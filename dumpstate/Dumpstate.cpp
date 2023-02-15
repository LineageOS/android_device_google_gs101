/*
 * Copyright 2016 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#define LOG_TAG "dumpstate_device"
#define ATRACE_TAG ATRACE_TAG_ALWAYS

#include <inttypes.h>

#include <android-base/file.h>
#include <android-base/stringprintf.h>
#include <android-base/properties.h>
#include <android-base/unique_fd.h>
#include <cutils/trace.h>
#include <log/log.h>
#include <pthread.h>
#include <sys/stat.h>

#include "Dumpstate.h"

#include "DumpstateUtil.h"

#define MODEM_LOG_DIRECTORY "/data/vendor/radio/logs/always-on"
#define MODEM_EXTENDED_LOG_DIRECTORY "/data/vendor/radio/extended_logs"
#define MODEM_LOG_HISTORY_DIRECTORY "/data/vendor/radio/logs/history"
#define RIL_LOG_DIRECTORY "/data/vendor/radio"
#define RIL_LOG_DIRECTORY_PROPERTY "persist.vendor.ril.log.base_dir"
#define RIL_LOG_NUMBER_PROPERTY "persist.vendor.ril.log.num_file"
#define MODEM_LOGGING_PERSIST_PROPERTY "persist.vendor.sys.modem.logging.enable"
#define MODEM_LOGGING_PROPERTY "vendor.sys.modem.logging.enable"
#define MODEM_LOGGING_STATUS_PROPERTY "vendor.sys.modem.logging.status"
#define MODEM_LOGGING_NUMBER_BUGREPORT_PROPERTY "persist.vendor.sys.modem.logging.br_num"
#define MODEM_LOGGING_PATH_PROPERTY "vendor.sys.modem.logging.log_path"
#define GPS_LOG_DIRECTORY "/data/vendor/gps/logs"
#define GPS_LOG_NUMBER_PROPERTY "persist.vendor.gps.aol.log_num"
#define GPS_LOGGING_STATUS_PROPERTY "vendor.gps.aol.enabled"

#define TCPDUMP_LOG_DIRECTORY "/data/vendor/tcpdump_logger/logs"
#define TCPDUMP_NUMBER_BUGREPORT "persist.vendor.tcpdump.log.br_num"
#define TCPDUMP_PERSIST_PROPERTY "persist.vendor.tcpdump.log.alwayson"

#define HW_REVISION "ro.boot.hardware.revision"

using android::os::dumpstate::CommandOptions;
using android::os::dumpstate::DumpFileToFd;
using android::os::dumpstate::PropertiesHelper;
using android::os::dumpstate::RunCommandToFd;

namespace aidl {
namespace android {
namespace hardware {
namespace dumpstate {

#define GPS_LOG_PREFIX "gl-"
#define GPS_MCU_LOG_PREFIX "esw-"
#define MODEM_LOG_PREFIX "sbuff_"
#define EXTENDED_LOG_PREFIX "extended_log_"
#define RIL_LOG_PREFIX "rild.log."
#define BUFSIZE 65536
#define TCPDUMP_LOG_PREFIX "tcpdump"

typedef std::chrono::time_point<std::chrono::steady_clock> timepoint_t;

const char kVerboseLoggingProperty[] = "persist.vendor.verbose_logging_enabled";

void copyFile(std::string srcFile, std::string destFile) {
    uint8_t buffer[BUFSIZE];
    ssize_t size;

    int fdSrc = open(srcFile.c_str(), O_RDONLY);
    if (fdSrc < 0) {
        ALOGD("Failed to open source file %s\n", srcFile.c_str());
        return;
    }

    int fdDest = open(destFile.c_str(), O_WRONLY | O_CREAT, 0666);
    if (fdDest < 0) {
        ALOGD("Failed to open destination file %s\n", destFile.c_str());
        close(fdSrc);
        return;
    }

    ALOGD("Copying %s to %s\n", srcFile.c_str(), destFile.c_str());
    while ((size = TEMP_FAILURE_RETRY(read(fdSrc, buffer, BUFSIZE))) > 0) {
        TEMP_FAILURE_RETRY(write(fdDest, buffer, size));
    }

    close(fdDest);
    close(fdSrc);
}

void dumpLogs(int fd, std::string srcDir, std::string destDir, int maxFileNum,
              const char *logPrefix) {
    (void) fd;
    struct dirent **dirent_list = NULL;
    int num_entries = scandir(srcDir.c_str(),
                              &dirent_list,
                              0,
                              (int (*)(const struct dirent **, const struct dirent **)) alphasort);
    if (!dirent_list) {
        return;
    } else if (num_entries <= 0) {
        return;
    }

    int copiedFiles = 0;

    for (int i = num_entries - 1; i >= 0; i--) {
        ALOGD("Found %s\n", dirent_list[i]->d_name);

        if (0 != strncmp(dirent_list[i]->d_name, logPrefix, strlen(logPrefix))) {
            continue;
        }

        if ((copiedFiles >= maxFileNum) && (maxFileNum != -1)) {
            ALOGD("Skipped %s\n", dirent_list[i]->d_name);
            continue;
        }

        copiedFiles++;

        std::string srcLogFile = srcDir + "/" + dirent_list[i]->d_name;
        std::string destLogFile = destDir + "/" + dirent_list[i]->d_name;
        copyFile(srcLogFile, destLogFile);

        ALOGD("Copying %s to %s\n", srcLogFile.c_str(), destLogFile.c_str());
    }

    while (num_entries--) {
        free(dirent_list[num_entries]);
    }

    free(dirent_list);
}

void dumpRilLogs(int fd, std::string destDir) {
    std::string rilLogDir =
            ::android::base::GetProperty(RIL_LOG_DIRECTORY_PROPERTY, RIL_LOG_DIRECTORY);

    int maxFileNum = ::android::base::GetIntProperty(RIL_LOG_NUMBER_PROPERTY, 50);

    const std::string currentLogDir = rilLogDir + "/cur";
    const std::string previousLogDir = rilLogDir + "/prev";
    const std::string currentDestDir = destDir + "/cur";
    const std::string previousDestDir = destDir + "/prev";

    RunCommandToFd(fd, "MKDIR RIL CUR LOG", {"/vendor/bin/mkdir", "-p", currentDestDir.c_str()},
                   CommandOptions::WithTimeout(2).Build());
    RunCommandToFd(fd, "MKDIR RIL PREV LOG", {"/vendor/bin/mkdir", "-p", previousDestDir.c_str()},
                   CommandOptions::WithTimeout(2).Build());

    dumpLogs(fd, currentLogDir, currentDestDir, maxFileNum, RIL_LOG_PREFIX);
    dumpLogs(fd, previousLogDir, previousDestDir, maxFileNum, RIL_LOG_PREFIX);
}

void dumpNetmgrLogs(std::string destDir) {
    const std::vector <std::string> netmgrLogs
        {
            "/data/vendor/radio/metrics_data",
            "/data/vendor/radio/omadm_logs.txt",
            "/data/vendor/radio/power_anomaly_data.txt",
        };
    for (const auto& logFile : netmgrLogs) {
        copyFile(logFile, destDir + "/" + basename(logFile.c_str()));
    }
}

/** Dumps last synced NV data into bugreports */
void dumpModemEFS(std::string destDir) {
    const std::string EFS_DIRECTORY = "/mnt/vendor/efs/";
    const std::vector <std::string> nv_files
        {
            EFS_DIRECTORY+"nv_normal.bin",
            EFS_DIRECTORY+"nv_protected.bin",
        };
    for (const auto& logFile : nv_files) {
        copyFile(logFile, destDir + "/" + basename(logFile.c_str()));
    }
}

void dumpGpsLogs(int fd, std::string destDir) {
    const std::string gpsLogDir = GPS_LOG_DIRECTORY;
    const std::string gpsTmpLogDir = gpsLogDir + "/.tmp";
    const std::string gpsDestDir = destDir + "/gps";
    int maxFileNum = ::android::base::GetIntProperty(GPS_LOG_NUMBER_PROPERTY, 20);

    RunCommandToFd(fd, "MKDIR GPS LOG", {"/vendor/bin/mkdir", "-p", gpsDestDir.c_str()},
                   CommandOptions::WithTimeout(2).Build());

    dumpLogs(fd, gpsTmpLogDir, gpsDestDir, 1, GPS_LOG_PREFIX);
    dumpLogs(fd, gpsLogDir, gpsDestDir, 3, GPS_MCU_LOG_PREFIX);
    dumpLogs(fd, gpsLogDir, gpsDestDir, maxFileNum, GPS_LOG_PREFIX);
}

void dumpCameraLogs(int fd, const std::string &destDir) {
    static const std::string kCameraLogDir = "/data/vendor/camera/profiler";
    const std::string cameraDestDir = destDir + "/camera";
    RunCommandToFd(fd, "MKDIR CAMERA LOG", {"/vendor/bin/mkdir", "-p", cameraDestDir.c_str()},
                   CommandOptions::WithTimeout(2).Build());
    // Attach multiple latest sessions (in case the user is running concurrent
    // sessions or starts a new session after the one with performance issues).
    dumpLogs(fd, kCameraLogDir, cameraDestDir, 10, "session-ended-");
    dumpLogs(fd, kCameraLogDir, cameraDestDir, 5, "high-drop-rate-");
    dumpLogs(fd, kCameraLogDir, cameraDestDir, 5, "watchdog-");
    dumpLogs(fd, kCameraLogDir, cameraDestDir, 5, "camera-ended-");
}

timepoint_t startSection(int fd, const std::string &sectionName) {
    ATRACE_BEGIN(sectionName.c_str());
    ::android::base::WriteStringToFd(
            "\n"
            "------ Section start: " + sectionName + " ------\n"
            "\n", fd);
    return std::chrono::steady_clock::now();
}

void endSection(int fd, const std::string &sectionName, timepoint_t startTime) {
    ATRACE_END();
    auto endTime = std::chrono::steady_clock::now();
    auto elapsedMsec = std::chrono::duration_cast<std::chrono::milliseconds>
            (endTime - startTime).count();

    ::android::base::WriteStringToFd(
            "\n"
            "------ Section end: " + sectionName + " ------\n"
            "Elapsed msec: " + std::to_string(elapsedMsec) + "\n"
            "\n", fd);
}

Dumpstate::Dumpstate()
  : mTextSections{
        { "memory", [this](int fd) { dumpMemorySection(fd); } },
        { "Devfreq", [this](int fd) { dumpDevfreqSection(fd); } },
        { "power", [this](int fd) { dumpPowerSection(fd); } },
        { "display", [this](int fd) { dumpDisplaySection(fd); } },
        { "camera", [this](int fd) { dumpCameraSection(fd); } },
    } {
}

// Dump data requested by an argument to the "dump" interface, or help info
// if the specified section is not supported.
void Dumpstate::dumpTextSection(int fd, const std::string &sectionName) {
    bool dumpAll = (sectionName == kAllSections);
    std::string dumpFiles;

    for (const auto &section : mTextSections) {
        if (dumpAll || sectionName == section.first) {
            auto startTime = startSection(fd, section.first);
            section.second(fd);
            endSection(fd, section.first, startTime);

            if (!dumpAll) {
                return;
            }
        }
    }

    // Execute all or designated programs under vendor/bin/dump/
    std::unique_ptr<DIR, decltype(&closedir)> dir(opendir("/vendor/bin/dump"), closedir);
    if (!dir) {
        ALOGE("Fail To Open Dir vendor/bin/dump/");
        ::android::base::WriteStringToFd("Fail To Open Dir vendor/bin/dump/\n", fd);
        return;
    }
    dirent *entry;
    while ((entry = readdir(dir.get())) != nullptr) {
        // Skip '.', '..'
        if (entry->d_name[0] == '.') {
            continue;
        }
        std::string bin(entry->d_name);
        dumpFiles = dumpFiles + " " + bin;
        if (dumpAll || sectionName == bin) {
            auto startTime = startSection(fd, bin);
            RunCommandToFd(fd, "/vendor/bin/dump/"+bin, {"/vendor/bin/dump/"+bin});
            endSection(fd, bin, startTime);
            if (!dumpAll) {
                return;
            }
        }
    }

    if (dumpAll) {
        RunCommandToFd(fd, "VENDOR PROPERTIES", {"/vendor/bin/getprop"});
        return;
    }

    // An unsupported section was requested on the command line
    ::android::base::WriteStringToFd("Unrecognized text section: " + sectionName + "\n", fd);
    ::android::base::WriteStringToFd("Try \"" + kAllSections + "\" or one of the following:", fd);
    for (const auto &section : mTextSections) {
        ::android::base::WriteStringToFd(" " + section.first, fd);
    }
    ::android::base::WriteStringToFd(dumpFiles, fd);
    ::android::base::WriteStringToFd("\nNote: sections with attachments (e.g. modem) are"
                                   "not avalable from the command line.\n", fd);
}

// Dump items related to power and battery
void Dumpstate::dumpPowerSection(int fd) {
    struct stat buffer;

    RunCommandToFd(fd, "Power Stats Times", {"/vendor/bin/sh", "-c",
                   "echo -n \"Boot: \" && /vendor/bin/uptime -s && "
                   "echo -n \"Now: \" && date"});

    RunCommandToFd(fd, "ACPM stats", {"/vendor/bin/sh", "-c",
                   "for f in /sys/devices/platform/acpm_stats/*_stats ; do "
                   "echo \"\\n\\n$f\" ; cat $f ; "
                   "done"});

    DumpFileToFd(fd, "CPU PM stats", "/sys/devices/system/cpu/cpupm/cpupm/time_in_state");

    DumpFileToFd(fd, "GENPD summary", "/d/pm_genpd/pm_genpd_summary");

    DumpFileToFd(fd, "Power supply property battery", "/sys/class/power_supply/battery/uevent");
    DumpFileToFd(fd, "Power supply property dc", "/sys/class/power_supply/dc/uevent");
    DumpFileToFd(fd, "Power supply property gcpm", "/sys/class/power_supply/gcpm/uevent");
    DumpFileToFd(fd, "Power supply property gcpm_pps", "/sys/class/power_supply/gcpm_pps/uevent");
    DumpFileToFd(fd, "Power supply property main-charger", "/sys/class/power_supply/main-charger/uevent");
    DumpFileToFd(fd, "Power supply property pca9486-mains", "/sys/class/power_supply/pca9468-mains/uevent");
    DumpFileToFd(fd, "Power supply property tcpm", "/sys/class/power_supply/tcpm-source-psy-i2c-max77759tcpc/uevent");
    DumpFileToFd(fd, "Power supply property usb", "/sys/class/power_supply/usb/uevent");
    DumpFileToFd(fd, "Power supply property wireless", "/sys/class/power_supply/wireless/uevent");
    if (!stat("/sys/class/power_supply/maxfg", &buffer)) {
        DumpFileToFd(fd, "Power supply property maxfg", "/sys/class/power_supply/maxfg/uevent");
        DumpFileToFd(fd, "m5_state", "/sys/class/power_supply/maxfg/m5_model_state");
        DumpFileToFd(fd, "maxfg", "/dev/logbuffer_maxfg");
        DumpFileToFd(fd, "maxfg", "/dev/logbuffer_maxfg_monitor");
    } else {
        DumpFileToFd(fd, "Power supply property maxfg_base", "/sys/class/power_supply/maxfg_base/uevent");
        DumpFileToFd(fd, "Power supply property maxfg_flip", "/sys/class/power_supply/maxfg_flip/uevent");
        DumpFileToFd(fd, "m5_state", "/sys/class/power_supply/maxfg_base/m5_model_state");
        DumpFileToFd(fd, "maxfg_base", "/dev/logbuffer_maxfg_base");
        DumpFileToFd(fd, "maxfg_flip", "/dev/logbuffer_maxfg_flip");
        DumpFileToFd(fd, "maxfg_base", "/dev/logbuffer_maxfg_base_monitor");
        DumpFileToFd(fd, "maxfg_flip", "/dev/logbuffer_maxfg_flip_monitor");
    }
    if (!stat("/sys/class/power_supply/dock", &buffer)) {
        DumpFileToFd(fd, "Power supply property dock", "/sys/class/power_supply/dock/uevent");
    }

    if (!stat("/dev/logbuffer_tcpm", &buffer)) {
        DumpFileToFd(fd, "Logbuffer TCPM", "/dev/logbuffer_tcpm");
    } else if (!PropertiesHelper::IsUserBuild()) {
        if (!stat("/sys/kernel/debug/tcpm", &buffer)) {
            RunCommandToFd(fd, "TCPM logs", {"/vendor/bin/sh", "-c", "cat /sys/kernel/debug/tcpm/*"});
        } else {
            RunCommandToFd(fd, "TCPM logs", {"/vendor/bin/sh", "-c", "cat /sys/kernel/debug/usb/tcpm*"});
        }
    }

    RunCommandToFd(fd, "TCPC", {"/vendor/bin/sh", "-c",
		       "for f in /sys/devices/platform/10d50000.hsi2c/i2c-*/i2c-max77759tcpc;"
		       "do echo \"registers:\"; cat $f/registers;"
		       "echo \"frs:\"; cat $f/frs;"
		       "echo \"auto_discharge:\"; cat $f/auto_discharge;"
		       "echo \"bc12_enabled:\"; cat $f/bc12_enabled;"
		       "echo \"cc_toggle_enable:\"; cat $f/cc_toggle_enable;"
		       "echo \"contaminant_detection:\"; cat $f/contaminant_detection;"
		       "echo \"contaminant_detection_status:\"; cat $f/contaminant_detection_status;  done"});

    DumpFileToFd(fd, "PD Engine", "/dev/logbuffer_usbpd");
    DumpFileToFd(fd, "POGO Transport", "/dev/logbuffer_pogo_transport");
    DumpFileToFd(fd, "PPS-google_cpm", "/dev/logbuffer_cpm");
    DumpFileToFd(fd, "PPS-dc", "/dev/logbuffer_pca9468");

    DumpFileToFd(fd, "Battery Health", "/sys/class/power_supply/battery/health_index_stats");
    DumpFileToFd(fd, "BMS", "/dev/logbuffer_ssoc");
    DumpFileToFd(fd, "TTF", "/dev/logbuffer_ttf");
    DumpFileToFd(fd, "TTF details", "/sys/class/power_supply/battery/ttf_details");
    DumpFileToFd(fd, "TTF stats", "/sys/class/power_supply/battery/ttf_stats");
    DumpFileToFd(fd, "maxq", "/dev/logbuffer_maxq");
    DumpFileToFd(fd, "aacr_state", "/sys/class/power_supply/battery/aacr_state");
    DumpFileToFd(fd, "TEMP/DOCK-DEFEND", "/dev/logbuffer_bd");

    RunCommandToFd(fd, "TRICKLE-DEFEND Config", {"/vendor/bin/sh", "-c",
                        " cd /sys/devices/platform/google,battery/power_supply/battery/;"
                        " for f in `ls bd_*` ; do echo \"$f: `cat $f`\" ; done"});

    RunCommandToFd(fd, "DWELL-DEFEND Config", {"/vendor/bin/sh", "-c",
                        " cd /sys/devices/platform/google,charger/;"
                        " for f in `ls charge_s*` ; do echo \"$f: `cat $f`\" ; done"});

    RunCommandToFd(fd, "TEMP-DEFEND Config", {"/vendor/bin/sh", "-c",
                        " cd /sys/devices/platform/google,charger/;"
                        " for f in `ls bd_*` ; do echo \"$f: `cat $f`\" ; done"});
    if (!PropertiesHelper::IsUserBuild()) {

        DumpFileToFd(fd, "DC_registers dump", "/sys/class/power_supply/pca9468-mains/device/registers_dump");
        DumpFileToFd(fd, "max77759_chg registers dump", "/d/max77759_chg/registers");
        DumpFileToFd(fd, "max77729_pmic registers dump", "/d/max77729_pmic/registers");
        DumpFileToFd(fd, "Charging table dump", "/d/google_battery/chg_raw_profile");


        RunCommandToFd(fd, "fg_model", {"/vendor/bin/sh", "-c",
                        "for f in /d/maxfg* ; do "
                        "regs=`cat $f/fg_model`; echo $f: ;"
                        "echo \"$regs\"; done"});

        RunCommandToFd(fd, "fg_alo_ver", {"/vendor/bin/sh", "-c",
                        "for f in /d/maxfg* ; do "
                        "regs=`cat $f/algo_ver`; echo $f: ;"
                        "echo \"$regs\"; done"});

        RunCommandToFd(fd, "fg_model_ok", {"/vendor/bin/sh", "-c",
                        "for f in /d/maxfg* ; do "
                        "regs=`cat $f/model_ok`; echo $f: ;"
                        "echo \"$regs\"; done"});


        /* FG Registers */
        RunCommandToFd(fd, "fg registers", {"/vendor/bin/sh", "-c",
                        "for f in /d/maxfg* ; do "
                        "regs=`cat $f/registers`; echo $f: ;"
                        "echo \"$regs\"; done"});
    }

    /* EEPROM State */
    if (!stat("/sys/devices/platform/10970000.hsi2c/i2c-4/4-0050/eeprom", &buffer)) {
        RunCommandToFd(fd, "Battery EEPROM", {"/vendor/bin/sh", "-c", "xxd /sys/devices/platform/10970000.hsi2c/i2c-4/4-0050/eeprom"});
    } else if(!stat("/sys/devices/platform/10970000.hsi2c/i2c-5/5-0050/eeprom", &buffer)) {
        RunCommandToFd(fd, "Battery EEPROM", {"/vendor/bin/sh", "-c", "xxd /sys/devices/platform/10970000.hsi2c/i2c-5/5-0050/eeprom"});
    } else if(!stat("/sys/devices/platform/10970000.hsi2c/i2c-6/6-0050/eeprom", &buffer)) {
        RunCommandToFd(fd, "Battery EEPROM", {"/vendor/bin/sh", "-c", "xxd /sys/devices/platform/10970000.hsi2c/i2c-6/6-0050/eeprom"});
    } else if(!stat("/sys/devices/platform/10970000.hsi2c/i2c-7/7-0050/eeprom", &buffer)) {
        RunCommandToFd(fd, "Battery EEPROM", {"/vendor/bin/sh", "-c", "xxd /sys/devices/platform/10970000.hsi2c/i2c-7/7-0050/eeprom"});
    } else {
        RunCommandToFd(fd, "Battery EEPROM", {"/vendor/bin/sh", "-c", "xxd /sys/devices/platform/10970000.hsi2c/i2c-8/8-0050/eeprom"});
    }

    DumpFileToFd(fd, "Charger Stats", "/sys/class/power_supply/battery/charge_details");
    if (!PropertiesHelper::IsUserBuild()) {
        RunCommandToFd(fd, "Google Charger", {"/vendor/bin/sh", "-c", "cd /sys/kernel/debug/google_charger/; "
                        "for f in `ls pps_*` ; do echo \"$f: `cat $f`\" ; done"});
        RunCommandToFd(fd, "Google Battery", {"/vendor/bin/sh", "-c", "cd /sys/kernel/debug/google_battery/; "
                        "for f in `ls ssoc_*` ; do echo \"$f: `cat $f`\" ; done"});
    }

    DumpFileToFd(fd, "WLC logs", "/dev/logbuffer_wireless");
    DumpFileToFd(fd, "WLC VER", "/sys/class/power_supply/wireless/device/version");
    DumpFileToFd(fd, "WLC STATUS", "/sys/class/power_supply/wireless/device/status");
    DumpFileToFd(fd, "WLC FW Version", "/sys/class/power_supply/wireless/device/fw_rev");
    DumpFileToFd(fd, "RTX", "/dev/logbuffer_rtx");

    if (!PropertiesHelper::IsUserBuild()) {
        RunCommandToFd(fd, "gvotables", {"/vendor/bin/sh", "-c", "cat /sys/kernel/debug/gvotables/*/status"});
    }
    RunCommandToFd(fd, "Mitigation Stats", {"/vendor/bin/sh", "-c", "echo \"Source\\t\\tCount\\tSOC\\tTime\\tVoltage\"; "
                        "for f in `ls /sys/devices/virtual/pmic/mitigation/last_triggered_count/*` ; "
                        "do count=`cat $f`; "
                        "a=${f/\\/sys\\/devices\\/virtual\\/pmic\\/mitigation\\/last_triggered_count\\//}; "
                        "b=${f/last_triggered_count/last_triggered_capacity}; "
                        "c=${f/last_triggered_count/last_triggered_timestamp/}; "
                        "d=${f/last_triggered_count/last_triggered_voltage/}; "
                        "cnt=`cat $f`; "
                        "cap=`cat ${b/count/cap}`; "
                        "ti=`cat ${c/count/time}`; "
                        "volt=`cat ${d/count/volt}`; "
                        "echo \"${a/_count/} "
                        "\\t$cnt\\t$cap\\t$ti\\t$volt\" ; done"});
    RunCommandToFd(fd, "Clock Divider Ratio", {"/vendor/bin/sh", "-c", "echo \"Source\\t\\tRatio\"; "
                        "for f in `ls /sys/devices/virtual/pmic/mitigation/clock_ratio/*` ; "
                        "do ratio=`cat $f`; "
                        "a=${f/\\/sys\\/devices\\/virtual\\/pmic\\/mitigation\\/clock_ratio\\//}; "
                        "echo \"${a/_ratio/} \\t$ratio\" ; done"});
    RunCommandToFd(fd, "Clock Stats", {"/vendor/bin/sh", "-c", "echo \"Source\\t\\tStats\"; "
                        "for f in `ls /sys/devices/virtual/pmic/mitigation/clock_stats/*` ; "
                        "do stats=`cat $f`; "
                        "a=${f/\\/sys\\/devices\\/virtual\\/pmic\\/mitigation\\/clock_stats\\//}; "
                        "echo \"${a/_stats/} \\t$stats\" ; done"});
    RunCommandToFd(fd, "Triggered Level", {"/vendor/bin/sh", "-c", "echo \"Source\\t\\tLevel\"; "
                        "for f in `ls /sys/devices/virtual/pmic/mitigation/triggered_lvl/*` ; "
                        "do lvl=`cat $f`; "
                        "a=${f/\\/sys\\/devices\\/virtual\\/pmic\\/mitigation\\/triggered_lvl\\//}; "
                        "echo \"${a/_lvl/} \\t$lvl\" ; done"});
    RunCommandToFd(fd, "Instruction", {"/vendor/bin/sh", "-c",
                        "for f in `ls /sys/devices/virtual/pmic/mitigation/instruction/*` ; "
                        "do val=`cat $f` ; "
                        "a=${f/\\/sys\\/devices\\/virtual\\/pmic\\/mitigation\\/instruction\\//}; "
                        "echo \"$a=$val\" ; done"});

}

// Dump items related to Devfreq & BTS
void Dumpstate::dumpDevfreqSection(int fd) {
    DumpFileToFd(fd, "MIF DVFS",
                 "/sys/devices/platform/17000010.devfreq_mif/devfreq/17000010.devfreq_mif/time_in_state");
    DumpFileToFd(fd, "INT DVFS",
                 "/sys/devices/platform/17000020.devfreq_int/devfreq/17000020.devfreq_int/time_in_state");
    DumpFileToFd(fd, "INTCAM DVFS",
                 "/sys/devices/platform/17000030.devfreq_intcam/devfreq/17000030.devfreq_intcam/time_in_state");
    DumpFileToFd(fd, "DISP DVFS",
                 "/sys/devices/platform/17000040.devfreq_disp/devfreq/17000040.devfreq_disp/time_in_state");
    DumpFileToFd(fd, "CAM DVFS",
                 "/sys/devices/platform/17000050.devfreq_cam/devfreq/17000050.devfreq_cam/time_in_state");
    DumpFileToFd(fd, "TNR DVFS",
                 "/sys/devices/platform/17000060.devfreq_tnr/devfreq/17000060.devfreq_tnr/time_in_state");
    DumpFileToFd(fd, "MFC DVFS",
                 "/sys/devices/platform/17000070.devfreq_mfc/devfreq/17000070.devfreq_mfc/time_in_state");
    DumpFileToFd(fd, "BO DVFS",
                 "/sys/devices/platform/17000080.devfreq_bo/devfreq/17000080.devfreq_bo/time_in_state");
    DumpFileToFd(fd, "BTS stats", "/sys/devices/platform/exynos-bts/bts_stats");
}

// Dump items related to memory
void Dumpstate::dumpMemorySection(int fd) {
    RunCommandToFd(fd, "Pixel CMA stat", {"/vendor/bin/sh", "-c",
                   "for d in $(ls -d /sys/kernel/pixel_stat/mm/cma/*); do "
                       "if [ -f $d ]; then "
                           "echo --- $d; cat $d; "
                       "else "
                           "for f in $(ls $d); do "
                               "echo --- $d/$f; cat $d/$f; "
                               "done; "
                        "fi; "
                        "done"});
}

// Dump items related to display
void Dumpstate::dumpDisplaySection(int fd) {
    // Dump counters for decon drivers
    const std::string decon_device_sysfs_path("/sys/class/drm/card0/device/");
    for(int i = 0; i <= 2; ++i){
        const std::string decon_num_str = std::to_string(i);
        const std::string decon_counter_path = decon_device_sysfs_path +
                                              "decon" + decon_num_str +
                                              "/counters";
        if (access(decon_counter_path.c_str(), R_OK) == 0){
            DumpFileToFd(fd, "DECON-" + decon_num_str + " counters",
                         decon_counter_path);
        }
        else{
            ::android::base::WriteStringToFd("No counters for DECON-" +
                decon_num_str + " found at path (" + decon_counter_path + ")\n",
                fd);
        }
    }
    DumpFileToFd(fd, "CRTC-0 event log", "/sys/kernel/debug/dri/0/crtc-0/event");
    DumpFileToFd(fd, "CRTC-1 event log", "/sys/kernel/debug/dri/0/crtc-1/event");
    RunCommandToFd(fd, "libdisplaycolor", {"/vendor/bin/dumpsys", "displaycolor", "-v"},
                   CommandOptions::WithTimeout(2).Build());
    DumpFileToFd(fd, "Primary panel extra info", "/sys/devices/platform/exynos-drm/primary-panel/panel_extinfo");
    DumpFileToFd(fd, "secondary panel extra info", "/sys/devices/platform/exynos-drm/secondary-panel/panel_extinfo");
    if (!PropertiesHelper::IsUserBuild()) {
        RunCommandToFd(fd, "HWC Fence States", {"/vendor/bin/sh", "-c",
                           "for f in $(ls /data/vendor/log/hwc/*_hwc_fence_state*.txt); do "
                           "echo $f ; cat $f ; done"},
                           CommandOptions::WithTimeout(2).Build());
        RunCommandToFd(fd, "HWC Error Logs", {"/vendor/bin/sh", "-c",
                           "for f in $(ls /data/vendor/log/hwc/*_hwc_error_log*.txt); do "
                           "echo $f ; cat $f ; done"},
                           CommandOptions::WithTimeout(2).Build());
        RunCommandToFd(fd, "HWC Debug Dumps", {"/vendor/bin/sh", "-c",
                           "for f in $(ls /data/vendor/log/hwc/*_hwc_debug*.dump); do "
                           "echo $f ; cat $f ; done"},
                           CommandOptions::WithTimeout(2).Build());
    }
}

// Dump essential camera debugging logs
void Dumpstate::dumpCameraSection(int fd) {
    RunCommandToFd(fd, "Camera HAL Graph State Dump", {"/vendor/bin/sh", "-c",
                       "for f in $(ls -t /data/vendor/camera/hal_graph_state*.txt |head -1); do "
                       "echo $f ; cat $f ; done"},
                       CommandOptions::WithTimeout(4).Build());
}

static void *dumpModemThread(void *data) {
    ATRACE_ASYNC_BEGIN("dumpModemThread", 0);
    std::string modemLogDir = MODEM_LOG_DIRECTORY;
    std::string extendedLogDir = MODEM_EXTENDED_LOG_DIRECTORY;
    std::string modemLogHistoryDir = MODEM_LOG_HISTORY_DIRECTORY;
    std::string tcpdumpLogDir = TCPDUMP_LOG_DIRECTORY;
    static const std::string sectionName = "modem";

    const std::string modemLogCombined = modemLogDir + "/modem_log_all.tar";
    const std::string modemLogAllDir = modemLogDir + "/modem_log";

    long fdModem = (long)data;

    ALOGD("dumpModemThread started\n");

    RunCommandToFd(STDOUT_FILENO, "MKDIR MODEM LOG", {"/vendor/bin/mkdir", "-p", modemLogAllDir.c_str()}, CommandOptions::WithTimeout(2).Build());

    bool modemLogEnabled = ::android::base::GetBoolProperty(MODEM_LOGGING_PERSIST_PROPERTY, false);
    if (modemLogEnabled && ::android::base::GetProperty(MODEM_LOGGING_PATH_PROPERTY, "") == MODEM_LOG_DIRECTORY) {
        bool modemLogStarted = ::android::base::GetBoolProperty(MODEM_LOGGING_STATUS_PROPERTY, false);
        int maxFileNum = ::android::base::GetIntProperty(MODEM_LOGGING_NUMBER_BUGREPORT_PROPERTY, 100);

        if (modemLogStarted) {
            ::android::base::SetProperty(MODEM_LOGGING_PROPERTY, "false");
            ALOGD("Stopping modem logging...\n");
        } else {
            ALOGD("modem logging is not running\n");
        }

        for (int i = 0; i < 15; i++) {
            if (!::android::base::GetBoolProperty(MODEM_LOGGING_STATUS_PROPERTY, false)) {
                ALOGD("modem logging stopped\n");
                sleep(1);
                break;
            }
            sleep(1);
        }

        dumpLogs(STDOUT_FILENO, modemLogDir, modemLogAllDir, maxFileNum, MODEM_LOG_PREFIX);

        if (modemLogStarted) {
            ALOGD("Restarting modem logging...\n");
            ::android::base::SetProperty(MODEM_LOGGING_PROPERTY, "true");
        }
    }

    if (!PropertiesHelper::IsUserBuild()) {
        bool gpsLogEnabled = ::android::base::GetBoolProperty(GPS_LOGGING_STATUS_PROPERTY, false);
        bool tcpdumpEnabled = ::android::base::GetBoolProperty(TCPDUMP_PERSIST_PROPERTY, false);
        bool cameraLogsEnabled = ::android::base::GetBoolProperty(
                "vendor.camera.debug.camera_performance_analyzer.attach_to_bugreport", true);

        if (tcpdumpEnabled) {
            dumpLogs(STDOUT_FILENO, tcpdumpLogDir, modemLogAllDir, ::android::base::GetIntProperty(TCPDUMP_NUMBER_BUGREPORT, 5), TCPDUMP_LOG_PREFIX);
        }

        if (gpsLogEnabled) {
            dumpGpsLogs(STDOUT_FILENO, modemLogAllDir);
        } else {
            ALOGD("gps logging is not running\n");
        }

        if (cameraLogsEnabled) {
            dumpCameraLogs(STDOUT_FILENO, modemLogAllDir);
        }

        dumpLogs(STDOUT_FILENO, extendedLogDir, modemLogAllDir, 50, EXTENDED_LOG_PREFIX);
        dumpLogs(STDOUT_FILENO, modemLogHistoryDir, modemLogAllDir, 2, "Logging");
        dumpRilLogs(STDOUT_FILENO, modemLogAllDir);
        dumpNetmgrLogs(modemLogAllDir);
        dumpModemEFS(modemLogAllDir);
    }

    ALOGD("Going to compress logs\n");

    RunCommandToFd(STDOUT_FILENO, "TAR LOG", {"/vendor/bin/tar", "cvf", modemLogCombined.c_str(), "-C", modemLogAllDir.c_str(), "."}, CommandOptions::WithTimeout(25).Build());
    RunCommandToFd(STDOUT_FILENO, "CHG PERM", {"/vendor/bin/chmod", "a+w", modemLogCombined.c_str()}, CommandOptions::WithTimeout(2).Build());

    std::vector<uint8_t> buffer(65536);
    ::android::base::unique_fd fdLog(TEMP_FAILURE_RETRY(open(modemLogCombined.c_str(), O_RDONLY | O_CLOEXEC | O_NONBLOCK)));

    ALOGD("Going to write to dumpstate board binary\n");
    if (fdLog >= 0) {
        while (1) {
            ssize_t bytes_read = TEMP_FAILURE_RETRY(read(fdLog, buffer.data(), buffer.size()));

            if (bytes_read == 0) {
                break;
            } else if (bytes_read < 0) {
                ALOGD("read(%s): %s\n", modemLogCombined.c_str(), strerror(errno));
                break;
            }

            ssize_t result = TEMP_FAILURE_RETRY(write(fdModem, buffer.data(), bytes_read));

            if (result != bytes_read) {
                ALOGD("Failed to write %ld bytes, actually written: %ld", bytes_read, result);
                break;
            }
        }
    }

    ALOGD("Going to remove logs\n");

    RunCommandToFd(STDOUT_FILENO, "RM MODEM DIR", { "/vendor/bin/rm", "-r", modemLogAllDir.c_str()}, CommandOptions::WithTimeout(2).Build());
    RunCommandToFd(STDOUT_FILENO, "RM LOG", { "/vendor/bin/rm", modemLogCombined.c_str()}, CommandOptions::WithTimeout(2).Build());

    ALOGD("dumpModemThread finished\n");

    ATRACE_ASYNC_END("dumpModemThread", 0);
    return NULL;
}

ndk::ScopedAStatus Dumpstate::dumpstateBoard(const std::vector<::ndk::ScopedFileDescriptor>& in_fds,
                                             IDumpstateDevice::DumpstateMode in_mode,
                                             int64_t in_timeoutMillis) {
    ATRACE_BEGIN("dumpstateBoard");
    // Unused arguments.
    (void) in_timeoutMillis;

    if (in_fds.size() < 1) {
        ALOGE("no FDs\n");
        return ndk::ScopedAStatus::fromExceptionCodeWithMessage(EX_ILLEGAL_ARGUMENT,
                                                                "No file descriptor");
    }

    int fd = in_fds[0].get();
    if (fd < 0) {
        ALOGE("invalid FD: %d\n", fd);
        return ndk::ScopedAStatus::fromExceptionCodeWithMessage(EX_ILLEGAL_ARGUMENT,
                                                                "Invalid file descriptor");
    }

    if (in_mode == IDumpstateDevice::DumpstateMode::WEAR) {
        // We aren't a Wear device.
        ALOGE("Unsupported mode: %d\n", in_mode);
        return ndk::ScopedAStatus::fromServiceSpecificErrorWithMessage(ERROR_UNSUPPORTED_MODE,
                                                                       "Unsupported mode");
    } else if (in_mode < IDumpstateDevice::DumpstateMode::FULL || in_mode > IDumpstateDevice::DumpstateMode::PROTO) {
        ALOGE("Invalid mode: %d\n", in_mode);
        return ndk::ScopedAStatus::fromExceptionCodeWithMessage(EX_ILLEGAL_ARGUMENT,
                                                                "Invalid mode");
    }

    // Create thread to collect modem related logs
    bool VerboseLogging;
    getVerboseLoggingEnabled(&VerboseLogging);
    pthread_t modemThreadHandle = 0;
    if (VerboseLogging && in_fds.size() > 1) {
        int fdModem = in_fds[1].get();
        if (pthread_create(&modemThreadHandle, NULL, dumpModemThread, (void *)((long)fdModem)) != 0) {
            ALOGE("could not create thread for dumpModem\n");
        }
    } else {
        ALOGD("Verbose logging is not enabled or no fd for modem.\n");
    }

    dumpTextSection(fd, kAllSections);

    if (modemThreadHandle) {
        pthread_join(modemThreadHandle, NULL);
    }

    ATRACE_END();
    return ndk::ScopedAStatus::ok();
}

ndk::ScopedAStatus Dumpstate::setVerboseLoggingEnabled(bool in_enable) {
    ::android::base::SetProperty(kVerboseLoggingProperty, in_enable ? "true" : "false");
    return ndk::ScopedAStatus::ok();
}

ndk::ScopedAStatus Dumpstate::getVerboseLoggingEnabled(bool* _aidl_return) {
    *_aidl_return = ::android::base::GetBoolProperty(kVerboseLoggingProperty, false);
    return ndk::ScopedAStatus::ok();
}

// Since AIDLs that support the dump() interface are automatically invoked during
// bugreport generation and we don't want to generate a second copy of the same
// data that will go into dumpstate_board.txt, this function will only do
// something if it is called with an option, e.g.
//   dumpsys android.hardware.dumpstate.IDumpstateDevice/default all
//
// Also, note that sections which generate attachments and/or binary data when
// included in a bugreport are not available through the dump() interface.
binder_status_t Dumpstate::dump(int fd, const char** args, uint32_t numArgs) {

    if (numArgs != 1) {
        return STATUS_OK;
    }

    dumpTextSection(fd, static_cast<std::string>(args[0]));

    fsync(fd);
    return STATUS_OK;
}

}  // namespace dumpstate
}  // namespace hardware
}  // namespace android
}  // namespace aidl
