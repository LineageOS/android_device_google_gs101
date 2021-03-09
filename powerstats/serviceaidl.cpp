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

#define LOG_TAG "android.hardware.power.stats-service.pixel"

#include <PowerStatsAidl.h>
#include "AocStateResidencyDataProviderAidl.h"
#include "DvfsStateResidencyDataProviderAidl.h"
#include <dataproviders/DisplayStateResidencyDataProvider.h>
#include <dataproviders/GenericStateResidencyDataProvider.h>
#include <dataproviders/IioEnergyMeterDataProvider.h>
#include <dataproviders/PowerStatsEnergyConsumer.h>
#include <dataproviders/PowerStatsEnergyAttribution.h>
#include <dataproviders/PixelStateResidencyDataProvider.h>

#include <android-base/logging.h>
#include <android-base/properties.h>
#include <android/binder_manager.h>
#include <android/binder_process.h>
#include <log/log.h>

using aidl::android::hardware::power::stats::AocStateResidencyDataProvider;
using aidl::android::hardware::power::stats::DisplayStateResidencyDataProvider;
using aidl::android::hardware::power::stats::DvfsStateResidencyDataProvider;
using aidl::android::hardware::power::stats::EnergyConsumerType;
using aidl::android::hardware::power::stats::GenericStateResidencyDataProvider;
using aidl::android::hardware::power::stats::IioEnergyMeterDataProvider;
using aidl::android::hardware::power::stats::PixelStateResidencyDataProvider;
using aidl::android::hardware::power::stats::PowerStats;
using aidl::android::hardware::power::stats::PowerStatsEnergyConsumer;

constexpr char kBootHwSoCRev[] = "ro.boot.hw.soc.rev";

void addAoC(std::shared_ptr<PowerStats> p) {
    std::string prefix = "/sys/devices/platform/19000000.aoc/control/";

    // Add AoC cores (a32, ff1, hf0, and hf1)
    std::vector<std::pair<std::string, std::string>> coreIds = {
            {"AoC-A32", prefix + "a32_"},
            {"AoC-FF1", prefix + "ff1_"},
            {"AoC-HF1", prefix + "hf1_"},
            {"AoC-HF0", prefix + "hf0_"},
    };
    std::vector<std::pair<std::string, std::string>> coreStates = {
            {"DWN", "off"}, {"RET", "retention"}, {"WFI", "wfi"}};
    p->addStateResidencyDataProvider(std::make_shared<AocStateResidencyDataProvider>(coreIds,
            coreStates));

    // Add AoC voltage stats
    std::vector<std::pair<std::string, std::string>> voltageIds = {
            {"AoC-Voltage", prefix + "voltage_"},
    };
    std::vector<std::pair<std::string, std::string>> voltageStates = {{"NOM", "nominal"},
                                                                      {"SUD", "super_underdrive"},
                                                                      {"UUD", "ultra_underdrive"},
                                                                      {"UD", "underdrive"}};
    p->addStateResidencyDataProvider(
            std::make_shared<AocStateResidencyDataProvider>(voltageIds, voltageStates));

    // Add AoC monitor mode
    std::vector<std::pair<std::string, std::string>> monitorIds = {
            {"AoC", prefix + "monitor_"},
    };
    std::vector<std::pair<std::string, std::string>> monitorStates = {
            {"MON", "mode"},
    };
    p->addStateResidencyDataProvider(
            std::make_shared<AocStateResidencyDataProvider>(monitorIds, monitorStates));
}

void addDvfsStats(std::shared_ptr<PowerStats> p) {
    // A constant to represent the number of nanoseconds in one millisecond
    const int NS_TO_MS = 1000000;

    std::vector<DvfsStateResidencyDataProvider::Config> cfgs;

    cfgs.push_back({"MIF", {
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
    }});

    cfgs.push_back({"CL0", {
        std::make_pair("2024KHz", "2024000"),
        std::make_pair("1950KHz", "1950000"),
        std::make_pair("1868KHz", "1868000"),
        std::make_pair("1803KHz", "1803000"),
        std::make_pair("1745KHz", "1745000"),
        std::make_pair("1704KHz", "1704000"),
        std::make_pair("1598KHz", "1598000"),
        std::make_pair("1459KHz", "1459000"),
        std::make_pair("1401KHz", "1401000"),
        std::make_pair("1328KHz", "1328000"),
        std::make_pair("1197KHz", "1197000"),
        std::make_pair("1098KHz", "1098000"),
        std::make_pair("930KHz", "930000"),
        std::make_pair("889KHz", "889000"),
        std::make_pair("738KHz", "738000"),
        std::make_pair("574KHz", "574000"),
        std::make_pair("300KHz", "300000"),
        std::make_pair("0KHz", "0"),
    }});

    cfgs.push_back({"CL1", {
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
    }});

    cfgs.push_back({"CL2", {
        std::make_pair("2630KHz", "2630000"),
        std::make_pair("2507KHz", "2507000"),
        std::make_pair("2401KHz", "2401000"),
        std::make_pair("2302KHz", "2302000"),
        std::make_pair("2252KHz", "2252000"),
        std::make_pair("2188KHz", "2188000"),
        std::make_pair("2048KHz", "2048000"),
        std::make_pair("1901KHz", "1901000"),
        std::make_pair("1826KHz", "1826000"),
        std::make_pair("1745KHz", "1745000"),
        std::make_pair("1582KHz", "1582000"),
        std::make_pair("1426KHz", "1426000"),
        std::make_pair("1277KHz", "1277000"),
        std::make_pair("1237KHz", "1237000"),
        std::make_pair("1106KHz", "1106000"),
        std::make_pair("984KHz", "984000"),
        std::make_pair("851KHz", "851000"),
        std::make_pair("848KHz", "848000"),
        std::make_pair("500KHz", "500000"),
        std::make_pair("0KHz", "0"),
    }});

    cfgs.push_back({"TPU", {
        std::make_pair("1066MHz", "1066000000"),
    }});

    p->addStateResidencyDataProvider(std::make_shared<DvfsStateResidencyDataProvider>(
            "/sys/devices/platform/1742048c.acpm_stats/fvp_stats", NS_TO_MS, cfgs));
}

void addSoC(std::shared_ptr<PowerStats> p) {
    // A constant to represent the number of nanoseconds in one millisecond.
    const int NS_TO_MS = 1000000;

    // ACPM stats are reported in nanoseconds. The transform function
    // converts nanoseconds to milliseconds.
    std::function<uint64_t(uint64_t)> acpmNsToMs = [](uint64_t a) { return a / NS_TO_MS; };
    const GenericStateResidencyDataProvider::StateResidencyConfig lpmStateConfig = {
            .entryCountSupported = true,
            .entryCountPrefix = "success_count:",
            .totalTimeSupported = true,
            .totalTimePrefix = "total_time_ns:",
            .totalTimeTransform = acpmNsToMs,
            .lastEntrySupported = true,
            .lastEntryPrefix = "last_entry_time_ns:",
            .lastEntryTransform = acpmNsToMs,
    };
    const GenericStateResidencyDataProvider::StateResidencyConfig downStateConfig = {
            .entryCountSupported = true,
            .entryCountPrefix = "down_count:",
            .totalTimeSupported = true,
            .totalTimePrefix = "total_down_time_ns:",
            .totalTimeTransform = acpmNsToMs,
            .lastEntrySupported = true,
            .lastEntryPrefix = "last_down_time_ns:",
            .lastEntryTransform = acpmNsToMs,
    };
    const GenericStateResidencyDataProvider::StateResidencyConfig reqStateConfig = {
            .entryCountSupported = true,
            .entryCountPrefix = "req_up_count:",
            .totalTimeSupported = true,
            .totalTimePrefix = "total_req_up_time_ns:",
            .totalTimeTransform = acpmNsToMs,
            .lastEntrySupported = true,
            .lastEntryPrefix = "last_req_up_time_ns:",
            .lastEntryTransform = acpmNsToMs,

    };
    const std::vector<std::pair<std::string, std::string>> powerStateHeaders = {
            std::make_pair("SICD", "SICD"),
            std::make_pair("SLEEP", "SLEEP"),
            std::make_pair("SLEEP_SLCMON", "SLEEP_SLCMON"),
            std::make_pair("STOP", "STOP"),
    };
    const std::vector<std::pair<std::string, std::string>> mifReqStateHeaders = {
            std::make_pair("AOC", "AOC"),
            std::make_pair("GSA", "GSA"),
    };
    const std::vector<std::pair<std::string, std::string>> slcReqStateHeaders = {
            std::make_pair("AOC", "AOC"),
    };

    std::vector<GenericStateResidencyDataProvider::PowerEntityConfig> cfgs;
    cfgs.emplace_back(generateGenericStateResidencyConfigs(lpmStateConfig, powerStateHeaders),
            "LPM", "LPM:");
    cfgs.emplace_back(generateGenericStateResidencyConfigs(downStateConfig, powerStateHeaders),
            "MIF", "MIF:");
    cfgs.emplace_back(generateGenericStateResidencyConfigs(reqStateConfig, mifReqStateHeaders),
            "MIF-REQ", "MIF_REQ:");
    cfgs.emplace_back(generateGenericStateResidencyConfigs(downStateConfig, powerStateHeaders),
            "SLC", "SLC:");
    cfgs.emplace_back(generateGenericStateResidencyConfigs(reqStateConfig, slcReqStateHeaders),
            "SLC-REQ", "SLC_REQ:");

    auto socSdp = std::make_shared<GenericStateResidencyDataProvider>(
            "/sys/devices/platform/1742048c.acpm_stats/soc_stats", cfgs);

    p->addStateResidencyDataProvider(socSdp);
}

void setEnergyMeter(std::shared_ptr<PowerStats> p) {
    std::vector<const std::string> deviceNames { "s2mpg10-odpm", "s2mpg11-odpm" };
    p->setEnergyMeterDataProvider(std::make_unique<IioEnergyMeterDataProvider>(deviceNames, true));
}

void addDisplay(std::shared_ptr<PowerStats> p) {
    // Add display residency stats

    /*
     * TODO(b/167216667): Add complete set of display states here. Must account
     * for ALL devices built using this source
     */
    std::vector<std::string> states = {
        "Off",
        "LP: 1440x3040@30",
        "On: 1440x3040@60",
        "On: 1440x3040@90",
        "HBM: 1440x3040@60",
        "HBM: 1440x3040@90"};

    auto displaySdp =
        std::make_shared<DisplayStateResidencyDataProvider>("Display",
            "/sys/class/backlight/panel0-backlight/state",
            states);
    p->addStateResidencyDataProvider(displaySdp);

    // Add display energy consumer
    /*
     * TODO(b/167216667): Add correct display power model here. Must read from display rail
     * and include proper coefficients for display states. Must account for ALL devices built
     * using this source.
     */
    auto displayConsumer = PowerStatsEnergyConsumer::createMeterAndEntityConsumer(p,
            EnergyConsumerType::DISPLAY, "display", {"PPVAR_VSYS_PWR_DISP"}, "Display",
            {{"LP: 1440x3040@30", 1},
             {"On: 1440x3040@60", 2},
             {"On: 1440x3040@90", 3}});

    p->addEnergyConsumer(displayConsumer);
}

void addCPUclusters(std::shared_ptr<PowerStats> p)
{
    p->addEnergyConsumer(PowerStatsEnergyConsumer::createMeterConsumer(p,
            EnergyConsumerType::CPU_CLUSTER, "CPUCL0", {"S4M_VDD_CPUCL0"}));
    p->addEnergyConsumer(PowerStatsEnergyConsumer::createMeterConsumer(p,
            EnergyConsumerType::CPU_CLUSTER, "CPUCL1", {"S3M_VDD_CPUCL1"}));
    p->addEnergyConsumer(PowerStatsEnergyConsumer::createMeterConsumer(p,
            EnergyConsumerType::CPU_CLUSTER, "CPUCL2", {"S2M_VDD_CPUCL2"}));
}

void addGPU(std::shared_ptr<PowerStats> p) {
    // Add gpu energy consumer
    std::map<std::string, int32_t> stateCoeffs;
    const int socRev = android::base::GetIntProperty(kBootHwSoCRev, 0);

    // B0/B1 chips have different GPU DVFS operating points than A0/A1 SoC
    if (socRev >= 2) {
        stateCoeffs = {
            {"151000",  10},
            {"202000",  20},
            {"251000",  30},
            {"302000",  40},
            {"351000",  50},
            {"400000",  60},
            {"471000",  70},
            {"510000",  80},
            {"572000",  90},
            {"701000", 100},
            {"762000", 110},
            {"848000", 120}};
    } else {
        stateCoeffs = {
            {"151000",  10},
            {"302000",  20},
            {"455000",  30},
            {"572000",  40},
            {"670000",  50}};
    }

    auto gpuConsumer = PowerStatsEnergyConsumer::createMeterAndAttrConsumer(p,
            EnergyConsumerType::OTHER, "GPU", {"S2S_VDD_G3D"},
            {{UID_TIME_IN_STATE, "/sys/devices/platform/1c500000.mali/uid_time_in_state"}},
            stateCoeffs);

    p->addEnergyConsumer(gpuConsumer);
}

void addMobileRadio(std::shared_ptr<PowerStats> p)
{
    // A constant to represent the number of microseconds in one millisecond.
    const int US_TO_MS = 1000;

    // modem power_stats are reported in microseconds. The transform function
    // converts microseconds to milliseconds.
    std::function<uint64_t(uint64_t)> modemUsToMs = [](uint64_t a) { return a / US_TO_MS; };
    const GenericStateResidencyDataProvider::StateResidencyConfig powerStateConfig = {
            .entryCountSupported = true,
            .entryCountPrefix = "count:",
            .totalTimeSupported = true,
            .totalTimePrefix = "duration_usec:",
            .totalTimeTransform = modemUsToMs,
            .lastEntrySupported = true,
            .lastEntryPrefix = "last_entry_timestamp_usec:",
            .lastEntryTransform = modemUsToMs,
    };
    const std::vector<std::pair<std::string, std::string>> powerStateHeaders = {
            std::make_pair("SLEEP", "SLEEP:"),
    };

    std::vector<GenericStateResidencyDataProvider::PowerEntityConfig> cfgs;
    cfgs.emplace_back(generateGenericStateResidencyConfigs(powerStateConfig, powerStateHeaders),
            "MODEM", "");

    p->addStateResidencyDataProvider(std::make_shared<GenericStateResidencyDataProvider>(
            "/sys/devices/platform/cpif/modem/power_stats", cfgs));

    p->addEnergyConsumer(PowerStatsEnergyConsumer::createMeterConsumer(p,
            EnergyConsumerType::MOBILE_RADIO, "MODEM", {"VSYS_PWR_MODEM", "VSYS_PWR_RFFE"}));
}

void addGNSS(std::shared_ptr<PowerStats> p)
{
    p->addEnergyConsumer(PowerStatsEnergyConsumer::createMeterConsumer(p,
            EnergyConsumerType::GNSS, "GPS", {"L9S_GNSS_CORE"}));
}
/**
 * Unlike other data providers, which source power entity state residency data from the kernel,
 * this data provider acts as a general-purpose channel for state residency data providers
 * that live in user space. Entities are defined here and user space clients of this provider's
 * vendor service register callbacks to provide state residency data for their given pwoer entity.
 */
void addPixelStateResidencyDataProvider(std::shared_ptr<PowerStats> p) {
    std::shared_ptr<PixelStateResidencyDataProvider> pixelSdp =
            ndk::SharedRefBase::make<PixelStateResidencyDataProvider>();

    pixelSdp->addEntity("Bluetooth", {{0, "Idle"}, {1, "Active"}, {2, "Tx"}, {3, "Rx"}});

    pixelSdp->start();
    p->addStateResidencyDataProvider(pixelSdp);
}

int main() {
    LOG(INFO) << "Pixel PowerStats HAL AIDL Service is starting.";

    // single thread
    ABinderProcess_setThreadPoolMaxThreadCount(0);

    std::shared_ptr<PowerStats> p = ndk::SharedRefBase::make<PowerStats>();

    setEnergyMeter(p);

    addPixelStateResidencyDataProvider(p);
    addAoC(p);
    addDisplay(p);
    addDvfsStats(p);
    addSoC(p);
    addCPUclusters(p);
    addGPU(p);
    addMobileRadio(p);
    addGNSS(p);

    const std::string instance = std::string() + PowerStats::descriptor + "/default";
    binder_status_t status = AServiceManager_addService(p->asBinder().get(), instance.c_str());
    LOG_ALWAYS_FATAL_IF(status != STATUS_OK);

    ABinderProcess_joinThreadPool();
    return EXIT_FAILURE;  // should not reach
}
