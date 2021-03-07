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
#ifndef HARDWARE_GOOGLE_PIXEL_POWERSTATS_UFSSTATERESIDENCYDATAPROVIDER_H
#define HARDWARE_GOOGLE_PIXEL_POWERSTATS_UFSSTATERESIDENCYDATAPROVIDER_H

#include <pixelpowerstats/PowerStats.h>

namespace android {
namespace hardware {
namespace google {
namespace pixel {
namespace powerstats {

class UfsStateResidencyDataProvider : public IStateResidencyDataProvider {
  public:
    /*
     * powerEntityId - id for the UFS power entity.
     */
    UfsStateResidencyDataProvider(uint32_t powerEntityId);
    ~UfsStateResidencyDataProvider() = default;

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
    uint64_t readStat(std::string path);

    uint32_t mPowerEntityId;
};

}  // namespace powerstats
}  // namespace pixel
}  // namespace google
}  // namespace hardware
}  // namespace android

#endif  // HARDWARE_GOOGLE_PIXEL_POWERSTATS_UFSSTATERESIDENCYDATAPROVIDER_H
