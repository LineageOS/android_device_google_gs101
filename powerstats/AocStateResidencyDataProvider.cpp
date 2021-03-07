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

#include "AocStateResidencyDataProvider.h"

#include <android-base/logging.h>

#include <utility>

namespace android {
namespace hardware {
namespace google {
namespace pixel {
namespace powerstats {

AocStateResidencyDataProvider::AocStateResidencyDataProvider(
        std::vector<std::pair<uint32_t, std::string>> ids,
        std::vector<std::pair<std::string, std::string>> states) {
    // AoC stats are reported in ticks of 244.140625ns. The transform
    // function converts ticks to milliseconds.
    // 1000000 / 244.140625 = 4096.
    static const uint64_t AOC_CLK = 4096;
    std::function<uint64_t(uint64_t)> aocTickToMs = [](uint64_t a) { return a / AOC_CLK; };
    StateResidencyConfig config = {
            .entryCountSupported = true,
            .entryCountPrefix = "Counter:",
            .totalTimeSupported = true,
            .totalTimePrefix = "Cumulative time:",
            .totalTimeTransform = aocTickToMs,
            .lastEntrySupported = true,
            .lastEntryPrefix = "Time last entered:",
            .lastEntryTransform = aocTickToMs,
    };
    uint32_t state_id;
    for (auto &id : ids) {
        state_id = 1;
        for (auto &state : states) {
            std::vector<std::pair<std::string, std::string>> aocStateHeaders = {
                    std::make_pair(state.first, ""),
            };
            std::unique_ptr<GenericStateResidencyDataProvider> sdp(
                    new GenericStateResidencyDataProvider(id.second + state.second));
            sdp->addEntity(id.first, PowerEntityConfig(state_id++, "",
                                                       generateGenericStateResidencyConfigs(
                                                               config, aocStateHeaders)));
            mProviders.push_back(std::move(sdp));
        }
    }
}

bool AocStateResidencyDataProvider::getResults(
        std::unordered_map<uint32_t, PowerEntityStateResidencyResult> &results) {
    for (auto &provider : mProviders) {
        provider->getResults(results);
    }
    return true;
}

std::vector<PowerEntityStateSpace> AocStateResidencyDataProvider::getStateSpaces() {
    // Return state spaces based on all configured providers.
    // States from the same power entity are merged.
    std::map<uint32_t, PowerEntityStateSpace> stateSpaces;
    for (auto &provider : mProviders) {
        for (auto &stateSpace : provider->getStateSpaces()) {
            auto it = stateSpaces.find(stateSpace.powerEntityId);
            if (it != stateSpaces.end()) {
                auto &states = it->second.states;
                auto size = states.size();
                states.resize(size + stateSpace.states.size());
                for (uint32_t i = 0; i < stateSpace.states.size(); i++) {
                    states[size + i] = stateSpace.states[i];
                }
            } else {
                stateSpaces.insert(std::pair<uint32_t, PowerEntityStateSpace>(
                        stateSpace.powerEntityId, stateSpace));
            }
        }
    }

    std::vector<PowerEntityStateSpace> ret;
    for (auto &stateSpace : stateSpaces) {
        ret.push_back(stateSpace.second);
    }
    return ret;
}

}  // namespace powerstats
}  // namespace pixel
}  // namespace google
}  // namespace hardware
}  // namespace android
