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
#define LOG_TAG "libpixelpowerstats"

#include "UfsStateResidencyDataProvider.h"

#include <android-base/logging.h>
#include <android-base/parseint.h>
#include <android-base/strings.h>

#include <string>
#include <utility>

using android::base::ParseUint;
using android::base::Split;
using android::base::StartsWith;
using android::base::Trim;

namespace android {
namespace hardware {
namespace google {
namespace pixel {
namespace powerstats {

const uint32_t HIBERNATE_STATE_ID = 0;

UfsStateResidencyDataProvider::UfsStateResidencyDataProvider(uint32_t powerEntityId)
    : mPowerEntityId(powerEntityId) {}

bool UfsStateResidencyDataProvider::getResults(
        std::unordered_map<uint32_t, PowerEntityStateResidencyResult> &results) {
    PowerEntityStateResidencyResult result = {
            .powerEntityId = mPowerEntityId,
            .stateResidencyData = {{.powerEntityStateId = HIBERNATE_STATE_ID}}};

    // The transform function converts microseconds to milliseconds.
    std::function<uint64_t(uint64_t)> usecToMs = [](uint64_t a) { return a / 1000; };

    std::string prefix = "/sys/bus/platform/devices/14700000.ufs/ufs_stats/";

    result.stateResidencyData[0].totalTimeInStateMs =
            usecToMs(readStat(prefix + "hibern8_total_us"));
    result.stateResidencyData[0].totalStateEntryCount = readStat(prefix + "hibern8_exit_cnt");
    result.stateResidencyData[0].lastEntryTimestampMs =
            usecToMs(readStat(prefix + "last_hibern8_enter_time"));
    results.insert(std::make_pair(mPowerEntityId, result));
    return true;
}

std::vector<PowerEntityStateSpace> UfsStateResidencyDataProvider::getStateSpaces() {
    return {{.powerEntityId = mPowerEntityId,
             .states = {{.powerEntityStateId = HIBERNATE_STATE_ID,
                         .powerEntityStateName = "HIBERN8"}}}};
}

uint64_t UfsStateResidencyDataProvider::readStat(std::string path) {
    std::unique_ptr<FILE, decltype(&fclose)> fp(fopen(path.c_str(), "r"), fclose);
    if (!fp) {
        PLOG(ERROR) << __func__ << ":Failed to open file " << path
                    << " Error = " << strerror(errno);
        return 0;
    }
    const size_t size = 20;
    char buf[size];
    (void)fread(&buf, sizeof(char), size, fp.get());
    uint64_t ret;
    if (!ParseUint(Trim(std::string(buf)), &ret)) {
        LOG(ERROR) << "Failed to parse uint64 from [" << std::string(buf) << "]";
    }
    return ret;
}

}  // namespace powerstats
}  // namespace pixel
}  // namespace google
}  // namespace hardware
}  // namespace android
