LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE    := native-plasma
LOCAL_SRC_FILES := plasma.c
LOCAL_LDLIBS    := -lthreaded_app -lm -llog -landroid

include $(BUILD_SHARED_LIBRARY)
