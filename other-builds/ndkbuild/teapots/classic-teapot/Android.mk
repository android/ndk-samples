#
# Copyright (C) 2016 The Android Open Source Project
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#      http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#

LOCAL_PATH := $(call my-dir)

include $(LOCAL_PATH)/../../common.mk

JNI_SRC_PATH := $(SAMPLES_ROOT)/teapots/classic-teapot/src/main/cpp
NDK_HELPER_SRC :=$(SAMPLES_ROOT)/teapots/common/ndk_helper

include $(CLEAR_VARS)

LOCAL_MODULE    := TeapotNativeActivity
LOCAL_SRC_FILES := $(JNI_SRC_PATH)/TeapotNativeActivity.cpp \
                   $(JNI_SRC_PATH)/TeapotRenderer.cpp \
                   $(NDK_HELPER_SRC)/JNIHelper.cpp    \
                   $(NDK_HELPER_SRC)/interpolator.cpp \
                   $(NDK_HELPER_SRC)/sensorManager.cpp \
                   $(NDK_HELPER_SRC)/tapCamera.cpp    \
                   $(NDK_HELPER_SRC)/gestureDetector.cpp \
                   $(NDK_HELPER_SRC)/perfMonitor.cpp \
                   $(NDK_HELPER_SRC)/vecmath.cpp   \
                   $(NDK_HELPER_SRC)/GLContext.cpp \
                   $(NDK_HELPER_SRC)/shader.cpp \
                   $(NDK_HELPER_SRC)/gl3stub.c

LOCAL_C_INCLUDES := $(JNI_SRC_PATH) $(NDK_HELPER_SRC)
LOCAL_CPPFLAGS += -std=c++11

LOCAL_LDLIBS    := -llog -landroid -lEGL -lGLESv2 -latomic
LOCAL_STATIC_LIBRARIES := cpufeatures android_native_app_glue

# Force export ANativeActivity_onCreate(), 
# Refer to: https://github.com/android-ndk/ndk/issues/381.
# Only needed when building with NDK-R14 and older
LOCAL_LDFLAGS   := -u ANativeActivity_onCreate

include $(BUILD_SHARED_LIBRARY)

$(call import-module,android/native_app_glue)
$(call import-module,android/cpufeatures)
