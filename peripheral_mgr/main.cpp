/*
 * Minimal PeripheralManager stub for Android 16.
 *
 * The stock Qualcomm pm-service binary crashes on Android 16 due to binder
 * wire protocol incompatibility (VNDR vs SYST parcel headers). This stub
 * replaces it by registering "vendor.qcom.PeripheralManager" with
 * vndservicemanager and responding to all transactions with OK status.
 *
 * hal_gnss_default queries PeripheralManager for modem subsystem state.
 * This stub reports all peripherals as "ready", allowing GPS to function
 * without the proprietary pm-service binary.
 */
#define LOG_TAG "peripheral_mgr_stub"

#include <binder/IPCThreadState.h>
#include <binder/IServiceManager.h>
#include <binder/ProcessState.h>
#include <binder/Parcel.h>
#include <binder/Status.h>
#include <log/log.h>
#include <utils/Looper.h>

using namespace android;

class PeripheralManagerService : public BBinder {
public:
    status_t onTransact(uint32_t code, const Parcel& data,
                        Parcel* reply, uint32_t flags) override {
        (void)data;
        (void)flags;
        ALOGV("PeripheralManager transaction code=%u", code);
        if (reply) {
            reply->writeInt32(0); /* STATUS_OK */
        }
        return NO_ERROR;
    }
};

int main() {
    ALOGI("Starting PeripheralManager stub");

    sp<ProcessState> ps = ProcessState::initWithDriver("/dev/vndbinder");
    ps->startThreadPool();

    sp<IServiceManager> sm = defaultServiceManager();
    sp<PeripheralManagerService> service = new PeripheralManagerService();

    status_t ret = sm->addService(String16("vendor.qcom.PeripheralManager"), service);
    if (ret != NO_ERROR) {
        ALOGE("Failed to register PeripheralManager: %d", ret);
        return ret;
    }

    ALOGI("PeripheralManager stub registered successfully");
    IPCThreadState::self()->joinThreadPool();
    return 0;
}
