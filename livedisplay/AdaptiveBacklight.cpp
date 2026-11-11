/*
 * SPDX-FileCopyrightText: The LineageOS Project
 * SPDX-License-Identifier: Apache-2.0
 */

#define LOG_TAG "AdaptiveBacklightService"

#include <fstream>

#include <android-base/logging.h>
#include <android-base/properties.h>
#include <utils/Errors.h>
#include "AdaptiveBacklight.h"

namespace {

static constexpr const char* kCabcPath =
        "/sys/devices/platform/soc/soc:qcom,dsi-display-primary/cabc";

static constexpr const char* kCabcOff = "0";
static constexpr const char* kCabcOn = "1"; /* 1 = UI, 2 = Still, 3 = Movie */

bool isSupportedDevice() {
    std::string device = android::base::GetProperty("ro.product.device", "");
    return device == "vayu" || device == "nabu" || device == "bhima";
}

}  // anonymous namespace

namespace aidl {
namespace vendor {
namespace lineage {
namespace livedisplay {

bool AdaptiveBacklight::isSupported() {
    if (!isSupportedDevice()) {
        return false;
    }

    if (access(kCabcPath, R_OK | W_OK) != 0) {
        LOG(ERROR) << "Failed to access " << kCabcPath << ", error=" << errno
                   << " (" << strerror(errno) << ")";
        return false;
    }

    return true;
}

ndk::ScopedAStatus AdaptiveBacklight::getEnabled(bool* _aidl_return) {
    std::ifstream cabc_file(kCabcPath);
    int result = -1;
    cabc_file >> result;

    if (cabc_file.fail()) {
        LOG(ERROR) << "Failed to read current CABC state";
        return ndk::ScopedAStatus::fromExceptionCode(EX_UNSUPPORTED_OPERATION);
    }

    *_aidl_return = result > 0;
    return ndk::ScopedAStatus::ok();
}

ndk::ScopedAStatus AdaptiveBacklight::setEnabled(bool enabled) {
    std::ofstream cabc_file(kCabcPath);
    cabc_file << (enabled ? kCabcOn : kCabcOff) << std::flush;

    if (cabc_file.fail()) {
        LOG(ERROR) << "Not possible to write CABC state";
        return ndk::ScopedAStatus::fromExceptionCode(EX_UNSUPPORTED_OPERATION);
    }

    return ndk::ScopedAStatus::ok();
}

}  // namespace livedisplay
}  // namespace lineage
}  // namespace vendor
}  // namespace aidl