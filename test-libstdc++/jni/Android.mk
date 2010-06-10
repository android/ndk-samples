# A simple test for the minimal standard C++ library
#

LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)
LOCAL_MODULE := test-libstl
LOCAL_SRC_FILES := test-libstl.cpp
LOCAL_STATIC_LIBRARIES := libstdc++
include $(BUILD_EXECUTABLE)

include $(NDK_ROOT)/sources/cpp-stl/minimal/Android.mk
