/*
 * Copyright (C) 2017 The Android Open Source Project
 * Copyright (C) 2026 The LineageOS Project
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#define LOG_TAG "android.hardware.vibrator-service.marlin"

#include "Vibrator.h"

#include <android-base/logging.h>
#include <android/binder_manager.h>
#include <android/binder_process.h>

#include <fstream>

using aidl::android::hardware::vibrator::Vibrator;

namespace {
constexpr const char* kEnablePath = "/sys/class/leds/vibrator/activate";
constexpr const char* kAmplitudePath = "/sys/class/leds/vibrator/vmax_mv";
constexpr const char* kDurationPath = "/sys/class/leds/vibrator/duration";
}  // namespace

int main() {
    std::ofstream enable{kEnablePath};
    if (!enable) {
        LOG(ERROR) << "Failed to open " << kEnablePath << ": " << strerror(errno);
        return errno ? errno : -1;
    }

    std::ofstream amplitude{kAmplitudePath};
    if (!amplitude) {
        LOG(ERROR) << "Failed to open " << kAmplitudePath << ": " << strerror(errno);
        return errno ? errno : -1;
    }

    std::ofstream duration{kDurationPath};
    if (!duration) {
        LOG(ERROR) << "Failed to open " << kDurationPath << ": " << strerror(errno);
        return errno ? errno : -1;
    }

    ABinderProcess_setThreadPoolMaxThreadCount(1);

    auto vibrator = ndk::SharedRefBase::make<Vibrator>(std::move(enable), std::move(amplitude),
                                                       std::move(duration));
    const std::string instance = std::string() + Vibrator::descriptor + "/default";
    binder_status_t status =
            AServiceManager_addService(vibrator->asBinder().get(), instance.c_str());
    if (status != STATUS_OK) {
        LOG(ERROR) << "Failed to register vibrator AIDL service: " << status;
        return -1;
    }

    ABinderProcess_joinThreadPool();
    return EXIT_FAILURE;  // should never get here
}
