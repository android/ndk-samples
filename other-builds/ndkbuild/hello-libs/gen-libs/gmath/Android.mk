LOCAL_PATH := $(abspath $(call my-dir))
include $(CLEAR_VARS)

JNI_SRC_PATH := $(LOCAL_PATH)/../../../../../hello-libs/gen-libs/src/main/cpp/gmath/src

LOCAL_MODULE := gmath
LOCAL_SRC_FILES := $(JNI_SRC_PATH)/gmath.c
LOCAL_C_INCLUDES := $(JNI_SRC_PATH)

include $(BUILD_STATIC_LIBRARY)

