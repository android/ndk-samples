LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)
LOCAL_MODULE := gperf
LOCAL_SRC_FILES := gperf.c
include $(BUILD_SHARED_LIBRARY)


