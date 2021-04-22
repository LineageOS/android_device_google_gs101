/*
 * Copyright (C) 2021 The Android Open Source Project
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

#include <aidl/android/frameworks/stats/IStats.h>
#include <android/binder_manager.h>
#include <android/binder_process.h>
#include <android-base/logging.h>
#include <getopt.h>
#include <poll.h>
#include <trusty/metrics/metrics.h>
#include <vendor/google/libraries/pixelatoms/pixelatoms.pb.h>

#include <memory>
#include <string>

using aidl::android::frameworks::stats::IStats;
using aidl::android::frameworks::stats::VendorAtom;
using aidl::android::frameworks::stats::VendorAtomValue;
using android::hardware::google::pixel::vendor::PixelAtoms::Atom;
using android::hardware::google::pixel::vendor::PixelAtoms::ReverseDomainNames;

static void show_usage_and_exit(int code) {
    LOG(ERROR) << "usage: trusty_metricsd -d <trusty_dev>";
    exit(code);
}

static void parse_device_name(int argc, char* argv[], char*& device_name) {
    static const char* _sopts = "h:d:";
    static const struct option _lopts[] = {{"help", no_argument, NULL, 'h'},
                                           {"trusty_dev", required_argument, NULL, 'd'},
                                           {0, 0, 0, 0}};
    int opt;
    int oidx = 0;

    while ((opt = getopt_long(argc, argv, _sopts, _lopts, &oidx)) != -1) {
        switch (opt) {
            case 'd':
                device_name = strdup(optarg);
                break;

            default:
                LOG(ERROR) << "unrecognized option: " << opt;
                show_usage_and_exit(EXIT_FAILURE);
        }
    }

    if (device_name == nullptr) {
        LOG(ERROR) << "missing required argument(s)";
        show_usage_and_exit(EXIT_FAILURE);
    }

    LOG(INFO) << "starting trusty_metricsd";
    LOG(INFO) << "trusty dev: " << device_name;
}

namespace android {
namespace trusty {
namespace metrics {

class TrustyMetricsPixel : public TrustyMetrics {
  private:
    TrustyMetricsPixel(std::string tipc_dev, std::shared_ptr<IStats> stats_client)
      : TrustyMetrics(std::move(tipc_dev)), stats_client_(stats_client) {}

    std::shared_ptr<IStats> stats_client_;

  public:
    static std::unique_ptr<TrustyMetricsPixel> Create(std::string tipc_dev) {
        const std::string stats_service_name =
            std::string(IStats::descriptor).append("/default");
        std::shared_ptr<IStats> stats_client = IStats::fromBinder(ndk::SpAIBinder(
            AServiceManager_waitForService(stats_service_name.c_str())));
        if (!stats_client) {
            LOG(ERROR) << "failed to open IStats client";
            return nullptr;
        }

        std::unique_ptr<TrustyMetricsPixel> metrics(
            new TrustyMetricsPixel(std::move(tipc_dev), stats_client));
        if (!metrics) {
            LOG(ERROR) << "failed to allocate TrustyMetricsPixel";
            return nullptr;
        }

        auto ret = metrics->Open();
        if (!ret.ok()) {
          LOG(ERROR) << "failed to open TrustyMetricsPixel: " << ret.error();
          return nullptr;
        }

        return metrics;
    }

    virtual void HandleCrash(const std::string& app_id) override {
        LOG(ERROR) << "TA crashed: " << app_id;

        std::vector<VendorAtomValue> v(1);
        v[0].set<VendorAtomValue::stringValue>(app_id);

        VendorAtom atom{
            .reverseDomainName = ReverseDomainNames().pixel(),
            .atomId = Atom::kTrustyAppCrashed,
            .values = std::move(v),
        };

        auto ret = stats_client_->reportVendorAtom(atom);
        if (!ret.isOk()) {
            LOG(ERROR) << "failed to report Trusty app crash to IStats";
        }
    }

    virtual void HandleEventDrop() override {
        LOG(ERROR) << "Trusty metrics event dropped!";
    }
};

}  // namespace metrics
}  // namespace trusty
}  // namespace android

int main(int argc, char* argv[]) {
    char* device_name;
    parse_device_name(argc, argv, device_name);

    auto metrics = android::trusty::metrics::TrustyMetricsPixel::Create(device_name);
    if (!metrics) {
        LOG(ERROR) << "failed to open Trusty metrics client";
        return EXIT_FAILURE;
    }

    int binder_fd;
    binder_status_t status = ABinderProcess_setupPolling(&binder_fd);
    if (status != STATUS_OK || binder_fd < 0) {
        LOG(ERROR) << "failed to setup binder polling";
        return EXIT_FAILURE;
    }

    struct pollfd pfds[] = {
            {
                    .fd = binder_fd,
                    .events = POLLIN,
            },
            {
                    .fd = metrics->GetRawFd(),
                    .events = POLLIN,
            },
    };

    while (1) {
        int rc = poll(pfds, 2, -1 /* no timeout */);
        if (rc <= 0) {
            LOG(ERROR) << "failed poll()";
            continue;
        }

        short binder_revents = pfds[0].revents;
        short metrics_revents = pfds[1].revents;

        short error_mask = POLLHUP | POLLERR | POLLNVAL;
        if ((binder_revents & error_mask) || (metrics_revents & error_mask)) {
            LOG(ERROR) << "received an error event from poll()";
            /* irrecoverable error */
            break;
        }

        if (binder_revents & POLLIN) {
            ABinderProcess_handlePolledCommands();
        }

        if (metrics_revents & POLLIN) {
            metrics->HandleEvent();
        }
    }

    return EXIT_FAILURE;
}
