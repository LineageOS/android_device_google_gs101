/*
 * Copyright (C) 2020 The Android Open Source Project
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

#define LOG_TAG "android.hardware.power.stats@1.0-service.gs101"

#include <android/log.h>
#include <binder/IPCThreadState.h>
#include <binder/IServiceManager.h>
#include <binder/ProcessState.h>
#include <hidl/HidlTransportSupport.h>
#include <pixelpowerstats/AidlStateResidencyDataProvider.h>
#include <pixelpowerstats/GenericStateResidencyDataProvider.h>
#include <pixelpowerstats/PowerStats.h>

#include "AocStateResidencyDataProvider.h"
#include "DvfsStateResidencyDataProvider.h"
#include "RailDataProvider.h"
#include "UfsStateResidencyDataProvider.h"

using android::OK;
using android::sp;
using android::status_t;

// libhwbinder:
using android::hardware::configureRpcThreadpool;
using android::hardware::joinRpcThreadpool;

// Generated HIDL files
using android::hardware::power::stats::V1_0::IPowerStats;
using android::hardware::power::stats::V1_0::PowerEntityType;
using android::hardware::power::stats::V1_0::implementation::PowerStats;

// Pixel specific
using android::hardware::google::pixel::powerstats::AidlStateResidencyDataProvider;
using android::hardware::google::pixel::powerstats::AocStateResidencyDataProvider;
using android::hardware::google::pixel::powerstats::DvfsStateResidencyDataProvider;
using android::hardware::google::pixel::powerstats::generateGenericStateResidencyConfigs;
using android::hardware::google::pixel::powerstats::GenericStateResidencyDataProvider;
using android::hardware::google::pixel::powerstats::PowerEntityConfig;
using android::hardware::google::pixel::powerstats::RailDataProvider;
using android::hardware::google::pixel::powerstats::StateResidencyConfig;
using android::hardware::google::pixel::powerstats::UfsStateResidencyDataProvider;

// A constant to represnt the number of nanoseconds in one millisecond.
static const int NS_TO_MS = 1000000;

void addAoCStats(PowerStats *const service) {
    std::string prefix = "/sys/devices/platform/19000000.aoc/control/";

    // Add AoC cores (a32, ff1, hf0, and hf1)
    std::vector<std::pair<uint32_t, std::string>> coreIds = {
            {service->addPowerEntity("AoC-A32", PowerEntityType::SUBSYSTEM), prefix + "a32_"},
            {service->addPowerEntity("AoC-FF1", PowerEntityType::SUBSYSTEM), prefix + "ff1_"},
            {service->addPowerEntity("AoC-HF1", PowerEntityType::SUBSYSTEM), prefix + "hf1_"},
            {service->addPowerEntity("AoC-HF0", PowerEntityType::SUBSYSTEM), prefix + "hf0_"},
    };
    std::vector<std::pair<std::string, std::string>> coreStates = {
            {"DWN", "off"}, {"RET", "retention"}, {"WFI", "wfi"}};
    service->addStateResidencyDataProvider(new AocStateResidencyDataProvider(coreIds, coreStates));

    // Add AoC voltage stats
    std::vector<std::pair<uint32_t, std::string>> voltageIds = {
            {service->addPowerEntity("AoC-Voltage", PowerEntityType::SUBSYSTEM),
             prefix + "voltage_"},
    };
    std::vector<std::pair<std::string, std::string>> voltageStates = {{"NOM", "nominal"},
                                                                      {"SUD", "super_underdrive"},
                                                                      {"UUD", "ultra_underdrive"},
                                                                      {"UD", "underdrive"}};
    service->addStateResidencyDataProvider(
            new AocStateResidencyDataProvider(voltageIds, voltageStates));

    // Add AoC monitor mode
    std::vector<std::pair<uint32_t, std::string>> monitorIds = {
            {service->addPowerEntity("AoC", PowerEntityType::SUBSYSTEM), prefix + "monitor_"},
    };
    std::vector<std::pair<std::string, std::string>> monitorStates = {
            {"MON", "mode"},
    };
    service->addStateResidencyDataProvider(
            new AocStateResidencyDataProvider(monitorIds, monitorStates));
}

void addCpuCStateStats(PowerStats *const service) {
    // CPU stats are reported in nanoseconds. The transform function
    // converts nanoseconds to milliseconds.
    std::function<uint64_t(uint64_t)> cpuNsToMs = [](uint64_t a) { return a / NS_TO_MS; };
    StateResidencyConfig cStateConfig = {
            .entryCountSupported = true,
            .entryCountPrefix = "down_count:",
            .totalTimeSupported = true,
            .totalTimePrefix = "total_down_time_ns:",
            .totalTimeTransform = cpuNsToMs,
            .lastEntrySupported = true,
            .lastEntryPrefix = "last_down_time_ns:",
            .lastEntryTransform = cpuNsToMs,
    };

    sp<GenericStateResidencyDataProvider> cStateSdp = new GenericStateResidencyDataProvider(
            "/sys/devices/platform/1742048c.acpm_stats/core_stats");
    for (std::string state :
         {"CORE00", "CORE01", "CORE02", "CORE03", "CORE10", "CORE11", "CORE20", "CORE21"}) {
        cStateSdp->addEntity(service->addPowerEntity(state, PowerEntityType::SUBSYSTEM),
                             PowerEntityConfig("CORES:", generateGenericStateResidencyConfigs(
                                                                 cStateConfig,
                                                                 {std::make_pair("DOWN", state)})));
    }
    for (std::string state : {"CLUSTER0", "CLUSTER1", "CLUSTER2"}) {
        cStateSdp->addEntity(
                service->addPowerEntity(state, PowerEntityType::SUBSYSTEM),
                PowerEntityConfig("CLUSTERS:",
                                  generateGenericStateResidencyConfigs(
                                          cStateConfig, {std::make_pair("DOWN", state)})));
    }
    service->addStateResidencyDataProvider(cStateSdp);
}

void addDvfsStats(PowerStats *const service) {
    sp<DvfsStateResidencyDataProvider> dvfsSdp = new DvfsStateResidencyDataProvider(
            "/sys/devices/platform/1742048c.acpm_stats/fvp_stats", NS_TO_MS);

    dvfsSdp->addEntity(service->addPowerEntity("MIF-DVFS", PowerEntityType::SUBSYSTEM), "MIF",
                       {
                               std::make_pair("3172KHz", "3172000"),
                               std::make_pair("2730KHz", "2730000"),
                               std::make_pair("2535KHz", "2535000"),
                               std::make_pair("2288KHz", "2288000"),
                               std::make_pair("2028KHz", "2028000"),
                               std::make_pair("1716KHz", "1716000"),
                               std::make_pair("1539KHz", "1539000"),
                               std::make_pair("1352KHz", "1352000"),
                               std::make_pair("1014KHz", "1014000"),
                               std::make_pair("845KHz", "845000"),
                               std::make_pair("676KHz", "676000"),
                               std::make_pair("546KHz", "546000"),
                               std::make_pair("421KHz", "421000"),
                               std::make_pair("0KHz", "0"),
                       });

    dvfsSdp->addEntity(service->addPowerEntity("CL0-DVFS", PowerEntityType::SUBSYSTEM), "CL0",
                       {
                               std::make_pair("2196KHz", "2196000"),
                               std::make_pair("2098KHz", "2098000"),
                               std::make_pair("2024KHz", "2024000"),
                               std::make_pair("1950KHz", "1950000"),
                               std::make_pair("1868KHz", "1868000"),
                               std::make_pair("1745KHz", "1745000"),
                               std::make_pair("1598KHz", "1598000"),
                               std::make_pair("1459KHz", "1459000"),
                               std::make_pair("1328KHz", "1328000"),
                               std::make_pair("1197KHz", "1197000"),
                               std::make_pair("1098KHz", "1098000"),
                               std::make_pair("889KHz", "889000"),
                               std::make_pair("738KHz", "738000"),
                               std::make_pair("574KHz", "574000"),
                               std::make_pair("300KHz", "300000"),
                               std::make_pair("0KHz", "0"),
                       });

    dvfsSdp->addEntity(service->addPowerEntity("CL1-DVFS", PowerEntityType::SUBSYSTEM), "CL1",
                       {
                               std::make_pair("2466KHz", "2466000"),
                               std::make_pair("2393KHz", "2393000"),
                               std::make_pair("2348KHz", "2348000"),
                               std::make_pair("2253KHz", "2253000"),
                               std::make_pair("2130KHz", "2130000"),
                               std::make_pair("1999KHz", "1999000"),
                               std::make_pair("1836KHz", "1836000"),
                               std::make_pair("1663KHz", "1663000"),
                               std::make_pair("1491KHz", "1491000"),
                               std::make_pair("1328KHz", "1328000"),
                               std::make_pair("1197KHz", "1197000"),
                               std::make_pair("1024KHz", "1024000"),
                               std::make_pair("910KHz", "910000"),
                               std::make_pair("799KHz", "799000"),
                               std::make_pair("696KHz", "696000"),
                               std::make_pair("533KHz", "533000"),
                               std::make_pair("400KHz", "400000"),
                               std::make_pair("0KHz", "0"),
                       });

    dvfsSdp->addEntity(service->addPowerEntity("CL2-DVFS", PowerEntityType::SUBSYSTEM), "CL2",
                       {
                               std::make_pair("3195KHz", "3195000"),
                               std::make_pair("3097KHz", "3097000"),
                               std::make_pair("2999KHz", "2999000"),
                               std::make_pair("2900KHz", "2900000"),
                               std::make_pair("2802KHz", "2802000"),
                               std::make_pair("2704KHz", "2704000"),
                               std::make_pair("2630KHz", "2630000"),
                               std::make_pair("2507KHz", "2507000"),
                               std::make_pair("2401KHz", "2401000"),
                               std::make_pair("2302KHz", "2302000"),
                               std::make_pair("2188KHz", "2188000"),
                               std::make_pair("2048KHz", "2048000"),
                               std::make_pair("1901KHz", "1901000"),
                               std::make_pair("1745KHz", "1745000"),
                               std::make_pair("1582KHz", "1582000"),
                               std::make_pair("1426KHz", "1426000"),
                               std::make_pair("1237KHz", "1237000"),
                               std::make_pair("1106KHz", "1106000"),
                               std::make_pair("984KHz", "984000"),
                               std::make_pair("848KHz", "848000"),
                               std::make_pair("500KHz", "500000"),
                               std::make_pair("0KHz", "0"),
                       });

    // TODO(sujee): Complete frequency table for TPU.
    dvfsSdp->addEntity(service->addPowerEntity("TPU-DVFS", PowerEntityType::SUBSYSTEM), "TPU",
                       {
                               std::make_pair("1066MHz", "1066000000"),
                       });

    service->addStateResidencyDataProvider(dvfsSdp);
}

void addGPUStats(PowerStats *const service) {
    StateResidencyConfig gpuStateConfig = {
            .entryCountSupported = true,
            .entryCountPrefix = "count = ",
            .totalTimeSupported = true,
            .totalTimePrefix = "total_time = ",
            .lastEntrySupported = true,
            .lastEntryPrefix = "last_entry_time = ",
    };

    sp<GenericStateResidencyDataProvider> gpu = new GenericStateResidencyDataProvider(
            "/sys/devices/platform/1c500000.mali/power_stats");

    PowerEntityConfig onOffConfig("Summary stats: (times in ms)",
                                  generateGenericStateResidencyConfigs(
                                          gpuStateConfig, {{"DOWN", "OFF:"}, {"UP", "ON:"}}));
    gpu->addEntity(service->addPowerEntity("GPU", PowerEntityType::SUBSYSTEM), onOffConfig);

    PowerEntityConfig DVFSConfig(
            "DVFS stats: (times in ms)",
            generateGenericStateResidencyConfigs(gpuStateConfig, {{"996MHz", "996000:"},
                                                                  {"885MHz", "885000:"},
                                                                  {"750MHz", "750000:"},
                                                                  {"434MHz", "434000:"},
                                                                  {"302MHz", "302000:"},
                                                                  {"151MHz", "151000:"}}));
    gpu->addEntity(service->addPowerEntity("GPU-DVFS", PowerEntityType::SUBSYSTEM), DVFSConfig);
    service->addStateResidencyDataProvider(gpu);
}

void addNFCStats(PowerStats *const service) {
    StateResidencyConfig nfcStateConfig = {
            .entryCountSupported = true,
            .entryCountPrefix = "Cumulative count:",
            .totalTimeSupported = true,
            .totalTimePrefix = "Cumulative duration msec:",
            .lastEntrySupported = true,
            .lastEntryPrefix = "Last entry timestamp msec:",
    };
    std::vector<std::pair<std::string, std::string>> nfcStateHeaders = {
            std::make_pair("IDLE", "Idle mode:"),
            std::make_pair("ACTIVE", "Active mode:"),
            std::make_pair("ACTIVE-RW", "Active Reader/Writer mode:"),
    };

    sp<GenericStateResidencyDataProvider> nfcSdp = new GenericStateResidencyDataProvider(
            "/sys/devices/platform/10960000.hsi2c/i2c-4/4-0008/power_stats");
    nfcSdp->addEntity(service->addPowerEntity("NFC", PowerEntityType::SUBSYSTEM),
                      PowerEntityConfig("NFC subsystem", generateGenericStateResidencyConfigs(
                                                                 nfcStateConfig, nfcStateHeaders)));
    service->addStateResidencyDataProvider(nfcSdp);
}

void addPCIeStats(PowerStats *const service) {
    // Add PCIe power entities for Modem and WiFi
    StateResidencyConfig pcieStateConfig = {
            .entryCountSupported = true,
            .entryCountPrefix = "Cumulative count:",
            .totalTimeSupported = true,
            .totalTimePrefix = "Cumulative duration msec:",
            .lastEntrySupported = true,
            .lastEntryPrefix = "Last entry timestamp msec:",
    };
    std::vector<std::pair<std::string, std::string>> pcieStateHeaders = {
            std::make_pair("UP", "Link up:"),
            std::make_pair("DOWN", "Link down:"),
    };

    // Add PCIe - Modem
    sp<GenericStateResidencyDataProvider> pcieModemSdp = new GenericStateResidencyDataProvider(
            "/sys/devices/platform/11920000.pcie/power_stats");
    uint32_t pcieModemId = service->addPowerEntity("PCIe-Modem", PowerEntityType::SUBSYSTEM);
    pcieModemSdp->addEntity(
            pcieModemId,
            PowerEntityConfig("Version: 1", generateGenericStateResidencyConfigs(
                                                    pcieStateConfig, pcieStateHeaders)));
    service->addStateResidencyDataProvider(pcieModemSdp);

    // Add PCIe - WiFi
    sp<GenericStateResidencyDataProvider> pcieWifiSdp = new GenericStateResidencyDataProvider(
            "/sys/devices/platform/14520000.pcie/power_stats");
    uint32_t pcieWifiId = service->addPowerEntity("PCIe-WiFi", PowerEntityType::SUBSYSTEM);
    pcieWifiSdp->addEntity(
            pcieWifiId,
            PowerEntityConfig("Version: 1", generateGenericStateResidencyConfigs(
                                                    pcieStateConfig, pcieStateHeaders)));
    service->addStateResidencyDataProvider(pcieWifiSdp);
}

void addSocStats(PowerStats *const service) {
    // ACPM stats are reported in nanoseconds. The transform function
    // converts nanoseconds to milliseconds.
    std::function<uint64_t(uint64_t)> acpmNsToMs = [](uint64_t a) { return a / NS_TO_MS; };
    StateResidencyConfig lpmStateConfig = {
            .entryCountSupported = true,
            .entryCountPrefix = "success_count:",
            .totalTimeSupported = true,
            .totalTimePrefix = "total_time_ns:",
            .totalTimeTransform = acpmNsToMs,
            .lastEntrySupported = true,
            .lastEntryPrefix = "last_entry_time_ns:",
            .lastEntryTransform = acpmNsToMs,
    };
    StateResidencyConfig downStateConfig = {
            .entryCountSupported = true,
            .entryCountPrefix = "down_count:",
            .totalTimeSupported = true,
            .totalTimePrefix = "total_down_time_ns:",
            .totalTimeTransform = acpmNsToMs,
            .lastEntrySupported = true,
            .lastEntryPrefix = "last_down_time_ns:",
            .lastEntryTransform = acpmNsToMs,
    };
    StateResidencyConfig reqStateConfig = {
            .entryCountSupported = true,
            .entryCountPrefix = "req_up_count:",
            .totalTimeSupported = true,
            .totalTimePrefix = "total_req_up_time_ns:",
            .totalTimeTransform = acpmNsToMs,
            .lastEntrySupported = true,
            .lastEntryPrefix = "last_req_up_time_ns:",
            .lastEntryTransform = acpmNsToMs,

    };
    std::vector<std::pair<std::string, std::string>> powerStateHeaders = {
            std::make_pair("SICD", "SICD"),
            std::make_pair("SLEEP", "SLEEP"),
            std::make_pair("SLEEP_SLCMON", "SLEEP_SLCMON"),
            std::make_pair("STOP", "STOP"),
    };
    std::vector<std::pair<std::string, std::string>> mifReqStateHeaders = {
            std::make_pair("AOC", "AOC"),
            std::make_pair("GSA", "GSA"),
    };
    std::vector<std::pair<std::string, std::string>> slcReqStateHeaders = {
            std::make_pair("AOC", "AOC"),
    };

    sp<GenericStateResidencyDataProvider> socSdp = new GenericStateResidencyDataProvider(
            "/sys/devices/platform/1742048c.acpm_stats/soc_stats");

    socSdp->addEntity(service->addPowerEntity("LPM", PowerEntityType::SUBSYSTEM),
                      PowerEntityConfig("LPM:", generateGenericStateResidencyConfigs(
                                                        lpmStateConfig, powerStateHeaders)));
    socSdp->addEntity(service->addPowerEntity("MIF", PowerEntityType::SUBSYSTEM),
                      PowerEntityConfig("MIF:", generateGenericStateResidencyConfigs(
                                                        downStateConfig, powerStateHeaders)));
    socSdp->addEntity(service->addPowerEntity("MIF-REQ", PowerEntityType::SUBSYSTEM),
                      PowerEntityConfig("MIF_REQ:", generateGenericStateResidencyConfigs(
                                                            reqStateConfig, mifReqStateHeaders)));
    socSdp->addEntity(service->addPowerEntity("SLC", PowerEntityType::SUBSYSTEM),
                      PowerEntityConfig("SLC:", generateGenericStateResidencyConfigs(
                                                        downStateConfig, powerStateHeaders)));
    socSdp->addEntity(service->addPowerEntity("SLC-REQ", PowerEntityType::SUBSYSTEM),
                      PowerEntityConfig("SLC_REQ:", generateGenericStateResidencyConfigs(
                                                            reqStateConfig, slcReqStateHeaders)));

    service->addStateResidencyDataProvider(socSdp);
}

void addUfsStats(PowerStats *const service) {
    sp<UfsStateResidencyDataProvider> ufsSdp = new UfsStateResidencyDataProvider(
            service->addPowerEntity("UFS", PowerEntityType::SUBSYSTEM));
    service->addStateResidencyDataProvider(ufsSdp);
}

void addWifiStats(PowerStats *const service) {
    sp<GenericStateResidencyDataProvider> wifiSdp =
            new GenericStateResidencyDataProvider("/sys/wifi/power_stats");

    // The transform function converts microseconds to milliseconds.
    std::function<uint64_t(uint64_t)> usecToMs = [](uint64_t a) { return a / 1000; };
    StateResidencyConfig stateConfig = {
            .entryCountSupported = true,
            .entryCountPrefix = "count:",
            .totalTimeSupported = true,
            .totalTimePrefix = "duration_usec:",
            .totalTimeTransform = usecToMs,
            .lastEntrySupported = true,
            .lastEntryPrefix = "last_entry_timestamp_usec:",
            .lastEntryTransform = usecToMs,
    };
    StateResidencyConfig pcieStateConfig = {
            .entryCountSupported = true,
            .entryCountPrefix = "count:",
            .totalTimeSupported = true,
            .totalTimePrefix = "duration_usec:",
            .totalTimeTransform = usecToMs,
    };

    std::vector<std::pair<std::string, std::string>> stateHeaders = {
            std::make_pair("AWAKE", "AWAKE:"),
            std::make_pair("ASLEEP", "ASLEEP:"),
    };
    std::vector<std::pair<std::string, std::string>> pcieStateHeaders = {
            std::make_pair("L0", "L0:"),     std::make_pair("L1", "L1:"),
            std::make_pair("L1_1", "L1_1:"), std::make_pair("L1_2", "L1_2:"),
            std::make_pair("L2", "L2:"),
    };

    wifiSdp->addEntity(
            service->addPowerEntity("WIFI", PowerEntityType::SUBSYSTEM),
            PowerEntityConfig(generateGenericStateResidencyConfigs(stateConfig, stateHeaders)));
    wifiSdp->addEntity(service->addPowerEntity("WIFI-PCIE", PowerEntityType::SUBSYSTEM),
                       PowerEntityConfig(generateGenericStateResidencyConfigs(pcieStateConfig,
                                                                              pcieStateHeaders)));

    service->addStateResidencyDataProvider(wifiSdp);
}

int main(int /* argc */, char ** /* argv */) {
    ALOGI("power.stats service 1.0 is starting.");

    PowerStats *service = new PowerStats();

    // Add rail data provider
    service->setRailDataProvider(std::make_unique<RailDataProvider>());

    addAoCStats(service);
    addCpuCStateStats(service);
    addDvfsStats(service);
    addGPUStats(service);
    addNFCStats(service);
    addPCIeStats(service);
    addSocStats(service);
    addUfsStats(service);
    addWifiStats(service);

    // Add Power Entities that require the Aidl data provider
    sp<AidlStateResidencyDataProvider> aidlSdp = new AidlStateResidencyDataProvider();
    uint32_t bluetoothId = service->addPowerEntity("Bluetooth", PowerEntityType::SUBSYSTEM);
    aidlSdp->addEntity(bluetoothId, "Bluetooth", {"Idle", "Active", "Tx", "Rx"});
    uint32_t citadelId = service->addPowerEntity("Citadel", PowerEntityType::SUBSYSTEM);
    aidlSdp->addEntity(citadelId, "Citadel", {"Last-Reset", "Active", "Deep-Sleep"});
    service->addStateResidencyDataProvider(aidlSdp);

    auto serviceStatus = android::defaultServiceManager()->addService(
            android::String16("power.stats-vendor"), aidlSdp);
    if (serviceStatus != android::OK) {
        ALOGE("Unable to register power.stats-vendor service %d", serviceStatus);
        return 1;
    }

    sp<android::ProcessState> ps{android::ProcessState::self()};  // Create non-HW binder threadpool
    ps->startThreadPool();

    // Configure the threadpool
    configureRpcThreadpool(1, true /*callerWillJoin*/);

    status_t status = service->registerAsService();
    if (status != OK) {
        ALOGE("Could not register service for power.stats HAL Iface (%d), exiting.", status);
        return 1;
    }

    ALOGI("power.stats service is ready");
    joinRpcThreadpool();

    // In normal operation, we don't expect the thread pool to exit
    ALOGE("power.stats service is shutting down");
    return 1;
}
