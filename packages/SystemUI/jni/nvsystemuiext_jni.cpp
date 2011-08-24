/*
 * Copyright (c) 2011 NVIDIA Corporation.  All Rights Reserved.
 *
 * NVIDIA Corporation and its licensors retain all intellectual property and
 * proprietary rights in and to this software and related documentation.  Any
 * use, reproduction, disclosure or distribution of this software and related
 * documentation without an express license agreement from NVIDIA Corporation
 * is strictly prohibited.
 */

#define LOG_TAG "NvSystemUIExt_JNI"

#include <jni.h>
#include <string.h>
#include <stdio.h>
#include <cutils/log.h>
#include <cutils/properties.h>
#include <dlfcn.h>

jstring getProperty(JNIEnv * env, jobject jobj, jstring propname);
int register_natives(JNIEnv* env);

jstring getProperty(JNIEnv * env, jobject jobj, jstring propname)
{
    jboolean isCopy;
    const char * szPropName = env->GetStringUTFChars(propname, &isCopy);
    char prop_value[PROPERTY_VALUE_MAX];
    jclass cls;
    jmethodID mid;
    bool bSuccess = false;

    LOGD("getProperty(): START, get property %s", szPropName);

    if (property_get(szPropName, prop_value, NULL) > 0)
    {
        LOGD("getProperty(): Could GET the property, %s = %s", szPropName, prop_value);
        bSuccess = true;
    }
    else
    {
        LOGD("getProperty(): Could not GET the property");
        bSuccess = false;
    }

    env->ReleaseStringUTFChars(propname, szPropName);

    LOGD("getProperty(): FINISHED");

    return (bSuccess) ? env->NewStringUTF(prop_value) : env->NewStringUTF("");
}

static JNINativeMethod sMethods[] = {
     /* name, signature, funcPtr */
    { "getPropertyUsingJNI", "(Ljava/lang/String;)Ljava/lang/String;",  (void*)getProperty
    },
};

int register_natives(JNIEnv* env)
{
    // Map the methods for the StereoSepElement class
    static const char* const kClassNameSep = "com/android/systemui/statusbar/policy/StereoSepElement";
    jclass myClassSep = env->FindClass(kClassNameSep);

    LOGD("Register JNI Methods.\n");

    if (myClassSep == NULL) {
        LOGD("Cannot find classname %s!", kClassNameSep);
        return -1;
    }

    if (env->RegisterNatives(myClassSep, sMethods, sizeof(sMethods)/sizeof(sMethods[0])) != JNI_OK) {
        LOGD("Failed registering methods for %s \n", kClassNameSep);
        return -1;
    }

    return 0;
}

jint JNI_OnLoad(JavaVM* vm, void* reserved)
{
    JNIEnv* env = NULL;

    if (vm->GetEnv((void**) &env, JNI_VERSION_1_4) != JNI_OK) {
        LOGD("GetEnv Failed!");
        return -1;
    }
    if (env == 0) {
        LOGD("Could not retrieve the env!");
    }

    register_natives(env);

    return JNI_VERSION_1_4;
}
