LOCAL_PATH:= $(call my-dir)
include $(NVIDIA_DEFAULTS)

# This is the target being built.
LOCAL_MODULE:= libnvsystemuiext_jni

# All of the source files that we will compile.
LOCAL_SRC_FILES:= \
    nvsystemuiext_jni.cpp

# All of the shared libraries we link against.
LOCAL_SHARED_LIBRARIES := \
    libandroid_runtime \
    liblog \
    libcutils \
    libnativehelper \
    libdl \

# No static libraries.
LOCAL_STATIC_LIBRARIES :=

# Also need the JNI headers.
LOCAL_C_INCLUDES += \
    $(JNI_H_INCLUDE) \

# Don't prelink this library.  For more efficient code, you may want
# to add this library to the prelink map and set this to true.
LOCAL_PRELINK_MODULE := false

include $(NVIDIA_SHARED_LIBRARY)
