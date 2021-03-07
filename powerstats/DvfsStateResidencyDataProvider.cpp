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

#include "DvfsStateResidencyDataProvider.h"

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

DvfsStateResidencyDataProvider::DvfsStateResidencyDataProvider(std::string path, uint64_t clockRate)
    : mPath(std::move(path)), mClockRate(clockRate) {}

void DvfsStateResidencyDataProvider::addEntity(
        uint32_t id, std::string name, std::vector<std::pair<std::string, std::string>> states) {
    mPowerEntities.push_back({id, name, states});
}

int32_t DvfsStateResidencyDataProvider::matchEntity(char *line) {
    for (auto const &entity : mPowerEntities) {
        if (entity.powerEntityName == Trim(std::string(line))) {
            return entity.powerEntityId;
        }
    }
    return -1;
}

int32_t DvfsStateResidencyDataProvider::matchState(char *line, int32_t entityId) {
    uint32_t stateId = 0;
    for (auto const &entity : mPowerEntities) {
        if (entityId == entity.powerEntityId) {
            for (auto const &state : entity.states) {
                if (StartsWith(Trim(std::string(line)), state.second)) {
                    return stateId;
                }
                stateId++;
            }
            return -1;
        }
    }
    return -1;
}

bool DvfsStateResidencyDataProvider::parseState(char *line, uint64_t &duration, uint64_t &count) {
    std::vector<std::string> parts = Split(line, " ");
    if (parts.size() != 7) {
        return false;
    }
    if (!ParseUint(Trim(parts[3]), &count)) {
        return false;
    }
    if (!ParseUint(Trim(parts[6]), &duration)) {
        return false;
    }
    return true;
}

bool DvfsStateResidencyDataProvider::getResults(
        std::unordered_map<uint32_t, PowerEntityStateResidencyResult> &results) {
    std::unique_ptr<FILE, decltype(&fclose)> fp(fopen(mPath.c_str(), "r"), fclose);
    if (!fp) {
        PLOG(ERROR) << __func__ << ":Failed to open file " << mPath
                    << " Error = " << strerror(errno);
        return false;
    }

    for (auto const &stateSpace : getStateSpaces()) {
        PowerEntityStateResidencyResult result = {.powerEntityId = stateSpace.powerEntityId};
        result.stateResidencyData.resize(stateSpace.states.size());
        for (uint32_t i = 0; i < result.stateResidencyData.size(); i++) {
            result.stateResidencyData[i].powerEntityStateId =
                    stateSpace.states[i].powerEntityStateId;
        }
        results.insert(std::make_pair(stateSpace.powerEntityId, result));
    }

    size_t len = 0;
    char *line = nullptr;

    int32_t temp = -1, entityId = -1, stateId = -1;
    uint64_t duration, count;

    while (getline(&line, &len, fp.get()) != -1) {
        temp = matchEntity(line);
        // Assign entityId only when a new valid entity is encountered.
        if (temp >= 0) {
            entityId = temp;
        }
        if (entityId >= 0) {
            stateId = matchState(line, entityId);
            if (stateId >= 0) {
                if (parseState(line, duration, count)) {
                    results[entityId].stateResidencyData[stateId].totalTimeInStateMs =
                            duration / mClockRate;
                    results[entityId].stateResidencyData[stateId].totalStateEntryCount = count;
                } else {
                    LOG(ERROR) << "Failed to parse duration and count from [" << std::string(line)
                               << "]";
                    return false;
                }
            }
        }
    }

    free(line);

    return true;
}

std::vector<PowerEntityStateSpace> DvfsStateResidencyDataProvider::getStateSpaces() {
    std::vector<PowerEntityStateSpace> stateSpaces;
    stateSpaces.reserve(mPowerEntities.size());
    for (auto const &entity : mPowerEntities) {
        PowerEntityStateSpace s = {.powerEntityId = entity.powerEntityId};
        s.states.resize(entity.states.size());
        uint32_t stateId = 0;
        for (auto const &state : entity.states) {
            s.states[stateId] = {.powerEntityStateId = stateId,
                                 .powerEntityStateName = state.first};
            stateId++;
        }
        stateSpaces.emplace_back(s);
    }
    return stateSpaces;
}

}  // namespace powerstats
}  // namespace pixel
}  // namespace google
}  // namespace hardware
}  // namespace android
