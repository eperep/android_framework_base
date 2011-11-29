LOCAL_PATH:= $(call my-dir)

include $(CLEAR_VARS)

LOCAL_SRC_FILES:=                 \
        AESParser.cpp             \

LOCAL_C_INCLUDES:= \
    $(JNI_H_INCLUDE) \
    $(TOP)/frameworks/base/include/media/stagefright/openmax \
    $(TOP)/frameworks/base/media/libstagefright \

LOCAL_MODULE:= libstagefright_es

ifeq ($(TARGET_ARCH),arm)
    LOCAL_CFLAGS += -Wno-psabi
endif

include $(BUILD_STATIC_LIBRARY)
