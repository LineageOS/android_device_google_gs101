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
#ifndef HARDWARE_GOOGLE_PIXEL_POWERSTATS_DVFSSTATERESIDENCYDATAPROVIDER_H
#define HARDWARE_GOOGLE_PIXEL_POWERSTATS_DVFSSTATERESIDENCYDATAPROVIDER_H

#include <pixelpowerstats/PowerStats.h>

namespace android {
namespace hardware {
namespace google {
namespace pixel {
namespace powerstats {

class DvfsStateResidencyDataProvider : public IStateResidencyDataProvider {
  public:
    /*
     * path - path to dvfs sysfs node.
     * clockRate - clock rate in KHz.
     */
    DvfsStateResidencyDataProvider(std::string path, uint64_t clockRate);
    ~DvfsStateResidencyDataProvider() = default;

    /*
     * id - the power entity id
     * name - the power entity name to parse from sysfs node
     * frequencies - list of pairs (frequency display name, frequency in sysfs
     *               node).
     */
    void addEntity(uint32_t id, std::string name,
                   std::vector<std::pair<std::string, std::string>> frequencies);

    /*
     * See IStateResidencyDataProvider::getResults.
     */
    bool getResults(
            std::unordered_map<uint32_t, PowerEntityStateResidencyResult> &results) override;

    /*
     * See IStateResidencyDataProvider::getStateSpaces.
     */
    std::vector<PowerEntityStateSpace> getStateSpaces() override;

  private:
    int32_t matchEntity(char *line);
    int32_t matchState(char *line, int32_t entityId);
    bool parseState(char *line, uint64_t &duration, uint64_t &count);

    const std::string mPath;
    const uint64_t mClockRate;

    struct config {
        // Power entity id.
        uint32_t powerEntityId;

        // Power entity name to parse.
        std::string powerEntityName;

        // List of state pairs (name to display, name to parse).
        std::vector<std::pair<std::string, std::string>> states;
    };
    std::vector<config> mPowerEntities;
};

}  // namespace powerstats
}  // namespace pixel
}  // namespace google
}  // namespace hardware
}  // namespace android

#endif  // HARDWARE_GOOGLE_PIXEL_POWERSTATS_DVFSSTATERESIDENCYDATAPROVIDER_H
