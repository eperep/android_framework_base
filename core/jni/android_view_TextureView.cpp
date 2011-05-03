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

#include "jni.h"
#include <nativehelper/JNIHelp.h>
#include <android_runtime/AndroidRuntime.h>

#include <gui/SurfaceTexture.h>

namespace android {

// ----------------------------------------------------------------------------
// Native layer
// ----------------------------------------------------------------------------

static void android_view_TextureView_setDefaultBufferSize(JNIEnv* env, jobject,
    jint surfaceTexture, jint width, jint height) {

    sp<SurfaceTexture> surface = reinterpret_cast<SurfaceTexture*>(surfaceTexture);
    surface->setDefaultBufferSize(width, height);
}

// ----------------------------------------------------------------------------
// JNI Glue
// ----------------------------------------------------------------------------

const char* const kClassPathName = "android/view/TextureView";

static JNINativeMethod gMethods[] = {
    {   "nSetDefaultBufferSize", "(III)V", (void*) android_view_TextureView_setDefaultBufferSize }
};

int register_android_view_TextureView(JNIEnv* env) {
    return AndroidRuntime::registerNativeMethods(env, kClassPathName, gMethods, NELEM(gMethods));
}

};