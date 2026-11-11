/*
 * SPDX-FileCopyrightText: The LineageOS Project
 * SPDX-License-Identifier: Apache-2.0
 */

#define LOG_TAG "vendor.lineage.livedisplay-service.mi_sm8150"

#include <android-base/logging.h>
#include <android/binder_manager.h>
#include <android/binder_process.h>
#include <binder/ProcessState.h>
#include "AdaptiveBacklight.h"
#include "SunlightEnhancement.h"

using ::aidl::vendor::lineage::livedisplay::AdaptiveBacklight;
using ::aidl::vendor::lineage::livedisplay::SunlightEnhancement;

int main() {
    std::shared_ptr<AdaptiveBacklight> ab = ndk::SharedRefBase::make<AdaptiveBacklight>();
    std::string ab_instance = std::string() + AdaptiveBacklight::descriptor + "/default";
    std::shared_ptr<SunlightEnhancement> se = ndk::SharedRefBase::make<SunlightEnhancement>();
    std::string se_instance = std::string() + SunlightEnhancement::descriptor + "/default";
    binder_status_t status;

    android::ProcessState::self()->setThreadPoolMaxThreadCount(2);
    android::ProcessState::self()->startThreadPool();

    LOG(INFO) << "LiveDisplay HAL custom service is starting.";

    if (ab == nullptr || se == nullptr) {
        LOG(ERROR) << "Can not create instances of LiveDisplay HAL interfaces, exiting.";
        goto shutdown;
    }

    if (!ab->isSupported() && !se->isSupported()) {
        LOG(ERROR) << "AdaptiveBacklight and SunlightEnhancement Iface is not supported, gracefully bailing out.";
        return EXIT_SUCCESS;
    }

    if (ab->isSupported()) {
        status = AServiceManager_addService(ab->asBinder().get(), ab_instance.c_str());
        if (status != STATUS_OK) {
            LOG(ERROR) << "Could not register service for LiveDisplay HAL AdaptiveBacklight Iface ("
                       << status << ")";
            goto shutdown;
        }
    }

    if (se->isSupported()) {
        status = AServiceManager_addService(se->asBinder().get(), se_instance.c_str());
        if (status != STATUS_OK) {
            LOG(ERROR) << "Could not register service for LiveDisplay HAL SunlightEnhancement Iface ("
                        << status << ")";
            goto shutdown;
        }
    }

    LOG(INFO) << "LiveDisplay HAL custom service is ready.";
    ABinderProcess_joinThreadPool();
    // Should not pass this line

shutdown:
    // In normal operation, we don't expect the thread pool to shutdown
    LOG(ERROR) << "LiveDisplay HAL custom service is shutting down.";
    return EXIT_FAILURE;  // Should not reach
}