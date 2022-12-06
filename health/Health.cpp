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
#define LOG_TAG "android.hardware.health@2.1-impl-gs101"
#include <android-base/logging.h>

#include <android-base/file.h>
#include <android-base/parseint.h>
#include <android-base/strings.h>
#include <android/hardware/health/translate-ndk.h>
#include <health-impl/Health.h>
#include <health/utils.h>

// Recovery doesn't have libpixelhealth and charger mode
#ifndef __ANDROID_RECOVERY__
#include <health-impl/ChargerUtils.h>
#include <pixelhealth/BatteryDefender.h>
#include <pixelhealth/BatteryMetricsLogger.h>
#include <pixelhealth/ChargerDetect.h>
#include <pixelhealth/DeviceHealth.h>
#include <pixelhealth/LowBatteryShutdownMetrics.h>
#endif // !__ANDROID_RECOVERY__

#include <chrono>
#include <fstream>
#include <iomanip>
#include <string>
#include <vector>

namespace {

using namespace std::literals;

using aidl::android::hardware::health::DiskStats;
using aidl::android::hardware::health::HalHealthLoop;
using aidl::android::hardware::health::HealthInfo;
using aidl::android::hardware::health::StorageInfo;
using android::hardware::health::InitHealthdConfig;

#ifndef __ANDROID_RECOVERY__
using aidl::android::hardware::health::charger::ChargerCallback;
using aidl::android::hardware::health::charger::ChargerModeMain;
using hardware::google::pixel::health::BatteryDefender;
using hardware::google::pixel::health::BatteryMetricsLogger;
using hardware::google::pixel::health::DeviceHealth;
using hardware::google::pixel::health::LowBatteryShutdownMetrics;
using hardware::google::pixel::health::ChargerDetect;

#define FG_DIR "/sys/class/power_supply/battery"
constexpr char kBatteryResistance[] {FG_DIR "/resistance"};
constexpr char kBatteryOCV[] {FG_DIR "/voltage_ocv"};
constexpr char kVoltageAvg[] {FG_DIR "/voltage_now"};

#define WLC_DIR "/sys/class/power_supply/wireless"

static BatteryDefender battDefender(WLC_DIR "/present",
    "/sys/devices/platform/google,charger/charge_start_level",
    "/sys/devices/platform/google,charger/charge_stop_level");
static BatteryMetricsLogger battMetricsLogger(kBatteryResistance, kBatteryOCV);
static LowBatteryShutdownMetrics shutdownMetrics(kVoltageAvg);
static DeviceHealth deviceHealth;
#endif // !__ANDROID_RECOVERY__

#define UFS_DIR "/dev/sys/block/bootdevice"
constexpr char kUfsHealthEol[]{UFS_DIR "/health_descriptor/eol_info"};
constexpr char kUfsHealthLifetimeA[]{UFS_DIR "/health_descriptor/life_time_estimation_a"};
constexpr char kUfsHealthLifetimeB[]{UFS_DIR "/health_descriptor/life_time_estimation_b"};
constexpr char kUfsVersion[]{UFS_DIR "/device_descriptor/specification_version"};
constexpr char kDiskStatsFile[]{"/sys/block/sda/stat"};

static std::string ufs_version;
static uint16_t eol;
static uint16_t lifetimeA;
static uint16_t lifetimeB;
static std::chrono::system_clock::time_point ufs_last_query_time;
constexpr auto kUfsQueryIntervalHours = std::chrono::hours{24};

#ifndef __ANDROID_RECOVERY__
static bool needs_wlc_updates = false;
constexpr char kWlcCapacity[]{WLC_DIR "/capacity"};
#endif // !__ANDROID_RECOVERY__

std::ifstream assert_open(const std::string &path) {
  std::ifstream stream(path);
  if (!stream.is_open()) {
    LOG(WARNING) << "Cannot read " << path;
  }
  return stream;
}

template <typename T>
void read_value_from_file(const std::string &path, T *field) {
  auto stream = assert_open(path);
  stream.unsetf(std::ios_base::basefield);
  stream >> *field;
}

void read_ufs_version(StorageInfo *info) {
  if (ufs_version.empty()) {
    uint64_t value;
    read_value_from_file(kUfsVersion, &value);
    std::stringstream ss;
    ss << "ufs " << std::hex << value;
    ufs_version = ss.str();
    LOG(INFO) << "ufs: " << ufs_version << " detected";
  }
  info->version = ufs_version;
}

#ifdef __ANDROID_RECOVERY__
void private_healthd_board_init(struct healthd_config *) {}
int private_healthd_board_battery_update(HealthInfo *) { return 0; }
#else // !__ANDROID__RECOVERY__
static bool FileExists(const std::string &filename) {
  struct stat buffer;

  return stat(filename.c_str(), &buffer) == 0;
}

void private_healthd_board_init(struct healthd_config *hc) {
  std::string tcpmPsyName;
  ChargerDetect::populateTcpmPsyName(&tcpmPsyName);
  hc->ignorePowerSupplyNames.push_back(android::String8(tcpmPsyName.c_str()));
  needs_wlc_updates = FileExists(kWlcCapacity);
  if (needs_wlc_updates == false) {
    battDefender.setWirelessNotSupported();
  }
}

int private_healthd_board_battery_update(HealthInfo *health_info) {
  deviceHealth.update(health_info);
  battMetricsLogger.logBatteryProperties(*health_info);
  shutdownMetrics.logShutdownVoltage(*health_info);
  // Allow BatteryDefender to override online properties
  ChargerDetect::onlineUpdate(health_info);
  battDefender.update(health_info);

  if (needs_wlc_updates &&
      !android::base::WriteStringToFile(std::to_string(health_info->batteryLevel), kWlcCapacity))
      LOG(INFO) << "Unable to write battery level to wireless capacity";

  return 0;
}
#endif // __ANDROID_RECOVERY__

void private_get_storage_info(std::vector<StorageInfo> *vec_storage_info) {
  vec_storage_info->resize(1);
  StorageInfo *storage_info = &vec_storage_info->at(0);

  read_ufs_version(storage_info);

  auto time_now = std::chrono::system_clock::now();
  auto time_delta = time_now - ufs_last_query_time;
  auto hoursElapsed = std::chrono::duration_cast<std::chrono::hours>(time_delta);
  if (hoursElapsed >= kUfsQueryIntervalHours) {
    ufs_last_query_time = time_now;
    read_value_from_file(kUfsHealthEol, &eol);
    read_value_from_file(kUfsHealthLifetimeA, &lifetimeA);
    read_value_from_file(kUfsHealthLifetimeB, &lifetimeB);
    LOG(INFO) << "ufs: eol=" << eol << " lifetimeA=" << lifetimeA << " lifetimeB=" << lifetimeB;
  }
  storage_info->eol = eol;
  storage_info->lifetimeA = lifetimeA;
  storage_info->lifetimeB = lifetimeB;

  return;
}

void private_get_disk_stats(std::vector<DiskStats> *vec_stats) {
  vec_stats->resize(1);
  DiskStats *stats = &vec_stats->at(0);

  auto stream = assert_open(kDiskStatsFile);
  // Regular diskstats entries
  stream >> stats->reads >> stats->readMerges >> stats->readSectors >>
      stats->readTicks >> stats->writes >> stats->writeMerges >>
      stats->writeSectors >> stats->writeTicks >> stats->ioInFlight >>
      stats->ioTicks >> stats->ioInQueue;
  return;
}
}  // anonymous namespace

namespace aidl::android::hardware::health::implementation {
class HealthImpl : public Health {
 public:
  HealthImpl(std::string_view instance_name, std::unique_ptr<healthd_config>&& config)
    : Health(std::move(instance_name), std::move(config)) {}

    ndk::ScopedAStatus getDiskStats(std::vector<DiskStats>* out) override;
    ndk::ScopedAStatus getStorageInfo(std::vector<StorageInfo>* out) override;

 protected:
  void UpdateHealthInfo(HealthInfo* health_info) override;

};

void HealthImpl::UpdateHealthInfo(HealthInfo* health_info) {
  private_healthd_board_battery_update(health_info);
}

ndk::ScopedAStatus HealthImpl::getStorageInfo(std::vector<StorageInfo>* out)
{
  private_get_storage_info(out);
  if (out->empty()) {
    return ndk::ScopedAStatus::fromExceptionCode(EX_UNSUPPORTED_OPERATION);
  }
  return ndk::ScopedAStatus::ok();
}

ndk::ScopedAStatus HealthImpl::getDiskStats(std::vector<DiskStats>* out)
{
  private_get_disk_stats(out);
  if (out->empty()) {
    return ndk::ScopedAStatus::fromExceptionCode(EX_UNSUPPORTED_OPERATION);
  }
  return ndk::ScopedAStatus::ok();
}

}  // namespace aidl::android::hardware::health::implementation

int main(int argc, char **argv) {
  using ::aidl::android::hardware::health::implementation::HealthImpl;

  // Use kernel logging in recovery
#ifdef __ANDROID_RECOVERY__
  android::base::InitLogging(argv, android::base::KernelLogger);
#endif

  auto config = std::make_unique<healthd_config>();
  InitHealthdConfig(config.get());

  private_healthd_board_init(config.get());

  auto binder =
      ndk::SharedRefBase::make<HealthImpl>("default"sv, std::move(config));

  if (argc >= 2 && argv[1] == "--charger"sv) {
    // In regular mode, start charger UI.
#ifndef __ANDROID_RECOVERY__
    LOG(INFO) << "Starting charger mode with UI.";
    return ChargerModeMain(binder, std::make_shared<ChargerCallback>(binder));
#endif
    // In recovery, ignore --charger arg.
    LOG(INFO) << "Starting charger mode without UI.";
  } else {
    LOG(INFO) << "Starting health HAL.";
  }

  auto hal_health_loop = std::make_shared<HalHealthLoop>(binder, binder);
  return hal_health_loop->StartLoop();
}
