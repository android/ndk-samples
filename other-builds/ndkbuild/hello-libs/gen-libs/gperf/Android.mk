LOCAL_PATH := $(abspath $(call my-dir))

include $(CLEAR_VARS)
JNI_SRC_PATH := $(LOCAL_PATH)/../../../../../hello-libs/gen-libs/src/main/cpp/gperf/src

LOCAL_MODULE := gperf
LOCAL_SRC_FILES := $(JNI_SRC_PATH)/gperf.c
LOCAL_C_INCLUDES := $(JNI_SRC_PATH)

include $(BUILD_SHARED_LIBRARY)
