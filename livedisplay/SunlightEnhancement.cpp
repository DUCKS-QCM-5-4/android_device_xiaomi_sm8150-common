/*
 * SPDX-FileCopyrightText: The LineageOS Project
 * SPDX-License-Identifier: Apache-2.0
 */

#define LOG_TAG "SunlightEnhancementService"

#include <fstream>

#include <android-base/logging.h>
#include <android-base/properties.h>
#include <utils/Errors.h>
#include "SunlightEnhancement.h"

namespace {

static constexpr const char* kHbmPath =
        "/sys/devices/platform/soc/soc:qcom,dsi-display-primary/hbm";

static constexpr const char* kHbmOff = "0";
static constexpr const char* kHbmOn = "2"; /* L1 = 1, L2 = max */

bool isSupportedDevice() {
    std::string device = android::base::GetProperty("ro.product.device", "");
    return device == "vayu" || device == "nabu" || device == "bhima";
}

}  // anonymous namespace

namespace aidl {
namespace vendor {
namespace lineage {
namespace livedisplay {

bool SunlightEnhancement::isSupported() {
    if (!isSupportedDevice()) {
        return false;
    }

    if (access(kHbmPath, R_OK | W_OK) != 0) {
        LOG(ERROR) << "Failed to access " << kHbmPath << ", error=" << errno
                   << " (" << strerror(errno) << ")";
        return false;
    }

    return true;
}

ndk::ScopedAStatus SunlightEnhancement::getEnabled(bool* _aidl_return) {
    std::ifstream hbm_file(kHbmPath);
    int result = -1;
    hbm_file >> result;

    if (hbm_file.fail()) {
        LOG(ERROR) << "Failed to read current HBM state";
        return ndk::ScopedAStatus::fromExceptionCode(EX_UNSUPPORTED_OPERATION);
    }

    *_aidl_return = result > 0;
    return ndk::ScopedAStatus::ok();
}

ndk::ScopedAStatus SunlightEnhancement::setEnabled(bool enabled) {
    std::ofstream hbm_file(kHbmPath);
    hbm_file << (enabled ? kHbmOn : kHbmOff) << std::flush;

    if (hbm_file.fail()) {
        LOG(ERROR) << "Not possible to write HBM state";
        return ndk::ScopedAStatus::fromExceptionCode(EX_UNSUPPORTED_OPERATION);
    }

    return ndk::ScopedAStatus::ok();
}

}  // namespace livedisplay
}  // namespace lineage
}  // namespace vendor
}  // namespace aidl