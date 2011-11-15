/*
 * Copyright (C) 2011 The Android Open Source Project
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

#include <nvcpud/NvCpuClient.h>

#include <binder/IServiceManager.h>

#undef LOG_TAG
#define LOG_TAG "NvCpuClient"

namespace android {

NvCpuClient::NvCpuClient() : lastConnectionAttempt(0),
    lastPoke(0),
    failedConnectionAttempts(0) {
}

const nsecs_t NvCpuClient::MINIMUM_POKE_WAIT_NS = 40 * 1000000LL;
const nsecs_t NvCpuClient::MINIMUM_CONNECTION_DELAY_NS = 5000 * 1000000LL;
const int NvCpuClient::MAXIMUM_CONNECTION_ATTEMPTS = 100;

status_t NvCpuClient::tryBindToService() {
    sp<IServiceManager> sm = defaultServiceManager();
    if (sm == 0)
        return FAILED_TRANSACTION;
    sp<IBinder> binder = sm->checkService(String16("NvCpuService"));
    if (binder == 0)
        return NAME_NOT_FOUND;
    mInterface = INvCpuService::asInterface(binder);
    if (mInterface == 0)
        return BAD_VALUE;
    LOGI("Successfully bound to service");
    return NO_ERROR;
}

status_t NvCpuClient::bindToService() {
    sp<IServiceManager> sm = defaultServiceManager();
    if (sm == 0)
        return FAILED_TRANSACTION;
    sp<IBinder> binder = sm->getService(String16("NvCpuService"));
    if (binder == 0)
        return NAME_NOT_FOUND;
    mInterface = INvCpuService::asInterface(binder);
    if (mInterface == 0)
        return BAD_VALUE;
    LOGI("Successfully bound to service");
    return NO_ERROR;
}

void NvCpuClient::checkAndTryReconnect(nsecs_t now) {
    if (mInterface != 0)
        return;
    if (failedConnectionAttempts > MAXIMUM_CONNECTION_ATTEMPTS)
        return;
    if (now < lastConnectionAttempt + MINIMUM_CONNECTION_DELAY_NS)
        return;
    lastConnectionAttempt = now;
    status_t res = tryBindToService();
    if (res != NO_ERROR) {
        LOGW("Failed to bind to service");
        failedConnectionAttempts++;
    }
}

void NvCpuClient::pokeCPU(nsecs_t now) {
    if (now - lastPoke < MINIMUM_POKE_WAIT_NS)
        return;
    lastPoke = now;
    checkAndTryReconnect(now);
    if (mInterface == 0)
        return;
    status_t res = mInterface->pokeCPU();
    if (res != NO_ERROR)
        breakConnection();
}

void NvCpuClient::breakConnection() {
    LOGW("Lost connection to service");
    mInterface = NULL;
}

};
