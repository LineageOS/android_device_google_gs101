/*
 * Copyright (C) 2018 The Android Open Source Project
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

#include "RailDataProvider.h"

#include <android-base/file.h>
#include <android-base/logging.h>
#include <android-base/properties.h>
#include <android-base/stringprintf.h>
#include <android-base/strings.h>
#include <inttypes.h>
#include <stdlib.h>

#include <algorithm>
#include <exception>
#include <thread>

namespace android {
namespace hardware {
namespace google {
namespace pixel {
namespace powerstats {

#define MAX_FILE_PATH_LEN 128
#define MAX_DEVICE_NAME_LEN 64
#define MAX_QUEUE_SIZE 8192

constexpr char kIioDirRoot[] = "/sys/bus/iio/devices/";
const char *const kDeviceNames[] = {"s2mpg10-odpm", "s2mpg11-odpm"};
constexpr char kDeviceType[] = "iio:device";
constexpr uint32_t MAX_SAMPLING_RATE = 10;
constexpr uint64_t WRITE_TIMEOUT_NS = 1000000000;

#define MAX_RAIL_NAME_LEN 50
#define STR(s) #s
#define XSTR(s) STR(s)

void RailDataProvider::findIioPowerMonitorNodes() {
    struct dirent *ent;
    int fd;
    char devName[MAX_DEVICE_NAME_LEN];
    char filePath[MAX_FILE_PATH_LEN];
    DIR *iioDir = opendir(kIioDirRoot);
    if (!iioDir) {
        ALOGE("Error opening directory: %s, error: %d", kIioDirRoot, errno);
        return;
    }
    while (ent = readdir(iioDir), ent) {
        if (strcmp(ent->d_name, ".") != 0 && strcmp(ent->d_name, "..") != 0 &&
                strlen(ent->d_name) > strlen(kDeviceType) &&
                strncmp(ent->d_name, kDeviceType, strlen(kDeviceType)) == 0) {
            snprintf(filePath, MAX_FILE_PATH_LEN, "%s/%s", ent->d_name, "name");
            fd = openat(dirfd(iioDir), filePath, O_RDONLY);
            if (fd < 0) {
                ALOGW("Failed to open directory: %s, error: %d", filePath, errno);
                continue;
            }
            if (read(fd, devName, MAX_DEVICE_NAME_LEN) < 0) {
                ALOGW("Failed to read device name from file: %s(%d)", filePath, fd);
                close(fd);
                continue;
            }

            for (const auto &kDeviceName : kDeviceNames) {
                if (strncmp(devName, kDeviceName, strlen(kDeviceName)) == 0) {
                    snprintf(filePath, MAX_FILE_PATH_LEN, "%s/%s", kIioDirRoot, ent->d_name);
                    mOdpm.devicePaths.push_back(filePath);
                }
            }
            close(fd);
        }
    }
    closedir(iioDir);
    return;
}

size_t RailDataProvider::parsePowerRails() {
    std::string data;
    std::string railFileName;
    std::string spsFileName;
    uint32_t index = 0;
    uint32_t samplingRate;
    for (const auto &path : mOdpm.devicePaths) {
        railFileName = path + "/enabled_rails";
        spsFileName = path + "/sampling_rate";
        if (!android::base::ReadFileToString(spsFileName, &data)) {
            ALOGW("Error reading file: %s", spsFileName.c_str());
            continue;
        }
        samplingRate = strtoul(data.c_str(), NULL, 10);
        if (!samplingRate || samplingRate == ULONG_MAX) {
            ALOGE("Error parsing: %s", spsFileName.c_str());
            break;
        }
        if (!android::base::ReadFileToString(railFileName, &data)) {
            ALOGW("Error reading file: %s", railFileName.c_str());
            continue;
        }
        std::istringstream railNames(data);
        std::string line;
        while (std::getline(railNames, line)) {
            std::vector<std::string> words = android::base::Split(line, ":][");
            if (words.size() == 4) {
                const std::string channelName = words[1];
                if (mOdpm.railsInfo.count(channelName) == 0) {
                    const std::string subsysName = words[3];
                    mOdpm.railsInfo.emplace(channelName, RailData{.devicePath = path,
                                                            .index = index,
                                                            .subsysName = subsysName,
                                                            .samplingRate = samplingRate});
                    index++;
                } else {
                    ALOGW("There exists rails with the same name (not supported): %s." \
                        " Only the last occurrence of rail energy will be provided.",
                        channelName.c_str());
                }
            } else {
                ALOGW("Unexpected format in file: %s", railFileName.c_str());
            }
        }
    }
    return index;
}

int RailDataProvider::parseIioEnergyNodeString(const std::string &contents, OnDeviceMmt &odpm) {
    std::istringstream energyData(contents);
    std::string line;

    int ret = 0;
    uint64_t timestamp = 0;
    bool timestampRead = false;

    while (std::getline(energyData, line)) {
        bool parseLineSuccess = false;

        if (timestampRead == false) {
            /* Read timestamp from boot (ms) */
            if (sscanf(line.c_str(), "t=%" PRIu64, &timestamp) == 1) {
                if (timestamp == 0 || timestamp == ULLONG_MAX) {
                    ALOGW("Potentially wrong timestamp: %" PRIu64, timestamp);
                }
                timestampRead = true;
                parseLineSuccess = true;
            }

        } else {
            /* Read rail energy */
            uint64_t energy = 0;
            char railNameRaw[MAX_RAIL_NAME_LEN + 1];
            if (sscanf(line.c_str(),
                       "CH%*d(T=%*" PRIu64 ")[%" XSTR(MAX_RAIL_NAME_LEN) "[^]]], %" PRIu64,
                       railNameRaw, &energy) == 2) {
                std::string railName(railNameRaw);

                /* If the count == 0, the rail may not be enabled */
                /* The count cannot be > 1; mChannelIds is a map */
                if (odpm.railsInfo.count(railName) == 1) {
                    size_t index = odpm.railsInfo[railName].index;
                    odpm.reading[index].index = index;
                    odpm.reading[index].timestamp = timestamp;
                    odpm.reading[index].energy = energy;
                    if (odpm.reading[index].energy == ULLONG_MAX) {
                        ALOGW("Potentially wrong energy value on rail: %s", railName.c_str());
                    }
                }
                parseLineSuccess = true;
            }
        }

        if (parseLineSuccess == false) {
            ret = -1;
            break;
        }
    }

    return ret;
}

int RailDataProvider::parseIioEnergyNode(std::string devName) {
    int ret;
    std::string data;
    std::string fileName = devName + "/energy_value";
    if (!android::base::ReadFileToString(fileName, &data)) {
        ALOGE("Error reading file: %s", fileName.c_str());
        return -1;
    }

    ret = parseIioEnergyNodeString(data, mOdpm);
    if (ret != 0) {
        ALOGW("Unexpected format in file: %s", fileName.c_str());
    }
    return ret;
}

Status RailDataProvider::parseIioEnergyNodes() {
    Status ret = Status::SUCCESS;
    if (mOdpm.hwEnabled == false) {
        return Status::NOT_SUPPORTED;
    }

    for (const auto &devicePath : mOdpm.devicePaths) {
        if (parseIioEnergyNode(devicePath) < 0) {
            ALOGE("Error in parsing power stats");
            ret = Status::FILESYSTEM_ERROR;
            break;
        }
    }
    return ret;
}

RailDataProvider::RailDataProvider() {
    findIioPowerMonitorNodes();
    size_t numRails = parsePowerRails();
    if (mOdpm.devicePaths.empty() || numRails == 0) {
        mOdpm.hwEnabled = false;
    } else {
        mOdpm.hwEnabled = true;
        mOdpm.reading.resize(numRails);
    }
}

Return<void> RailDataProvider::getRailInfo(IPowerStats::getRailInfo_cb _hidl_cb) {
    hidl_vec<RailInfo> rInfo;
    Status ret = Status::SUCCESS;
    size_t index;
    std::lock_guard<std::mutex> _lock(mOdpm.mLock);
    if (mOdpm.hwEnabled == false) {
        ALOGI("getRailInfo not supported");
        _hidl_cb(rInfo, Status::NOT_SUPPORTED);
        return Void();
    }
    rInfo.resize(mOdpm.railsInfo.size());
    for (const auto &railData : mOdpm.railsInfo) {
        index = railData.second.index;
        rInfo[index].railName = railData.first;
        rInfo[index].subsysName = railData.second.subsysName;
        rInfo[index].index = index;
        rInfo[index].samplingRate = railData.second.samplingRate;
    }
    _hidl_cb(rInfo, ret);
    return Void();
}

Return<void> RailDataProvider::getEnergyData(const hidl_vec<uint32_t> &railIndices,
                                             IPowerStats::getEnergyData_cb _hidl_cb) {
    hidl_vec<EnergyData> eVal;
    std::lock_guard<std::mutex> _lock(mOdpm.mLock);
    Status ret = parseIioEnergyNodes();

    if (ret != Status::SUCCESS) {
        ALOGE("Failed to getEnergyData");
        _hidl_cb(eVal, ret);
        return Void();
    }

    if (railIndices.size() == 0) {
        eVal.resize(mOdpm.railsInfo.size());
        memcpy(&eVal[0], &mOdpm.reading[0], mOdpm.reading.size() * sizeof(EnergyData));
    } else {
        eVal.resize(railIndices.size());
        int i = 0;
        for (const auto &railIndex : railIndices) {
            if (railIndex >= mOdpm.reading.size()) {
                ret = Status::INVALID_INPUT;
                eVal.resize(0);
                break;
            }
            memcpy(&eVal[i], &mOdpm.reading[railIndex], sizeof(EnergyData));
            i++;
        }
    }
    _hidl_cb(eVal, ret);
    return Void();
}

Return<void> RailDataProvider::streamEnergyData(uint32_t timeMs, uint32_t samplingRate,
                                                IPowerStats::streamEnergyData_cb _hidl_cb) {
    std::lock_guard<std::mutex> _lock(mOdpm.mLock);
    if (mOdpm.fmqSynchronized != nullptr) {
        _hidl_cb(MessageQueueSync::Descriptor(), 0, 0, Status::INSUFFICIENT_RESOURCES);
        return Void();
    }
    uint32_t sps = std::min(samplingRate, MAX_SAMPLING_RATE);
    uint32_t numSamples = timeMs * sps / 1000;
    mOdpm.fmqSynchronized.reset(new (std::nothrow) MessageQueueSync(MAX_QUEUE_SIZE, true));
    if (mOdpm.fmqSynchronized == nullptr || mOdpm.fmqSynchronized->isValid() == false) {
        mOdpm.fmqSynchronized = nullptr;
        _hidl_cb(MessageQueueSync::Descriptor(), 0, 0, Status::INSUFFICIENT_RESOURCES);
        return Void();
    }
    std::thread pollThread = std::thread([this, sps, numSamples]() {
        uint64_t sleepTimeUs = 1000000 / sps;
        uint32_t currSamples = 0;
        while (currSamples < numSamples) {
            mOdpm.mLock.lock();
            if (parseIioEnergyNodes() == Status::SUCCESS) {
                mOdpm.fmqSynchronized->writeBlocking(&mOdpm.reading[0], mOdpm.reading.size(),
                                                     WRITE_TIMEOUT_NS);
                mOdpm.mLock.unlock();
                currSamples++;
                if (usleep(sleepTimeUs) < 0) {
                    ALOGW("Sleep interrupted");
                    break;
                }
            } else {
                mOdpm.mLock.unlock();
                break;
            }
        }
        mOdpm.mLock.lock();
        mOdpm.fmqSynchronized = nullptr;
        mOdpm.mLock.unlock();
        return;
    });
    pollThread.detach();
    _hidl_cb(*(mOdpm.fmqSynchronized)->getDesc(), numSamples, mOdpm.reading.size(),
             Status::SUCCESS);
    return Void();
}

}  // namespace powerstats
}  // namespace pixel
}  // namespace google
}  // namespace hardware
}  // namespace android
