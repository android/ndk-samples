# Copyright (C) 2010 The Android Open Source Project
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

JNI_SRC_PATH := $(SAMPLES_ROOT)/native-activity/app/src/main/cpp

include $(CLEAR_VARS)

LOCAL_MODULE    := native-activity
LOCAL_SRC_FILES := $(JNI_SRC_PATH)/main.cpp
LOCAL_CPPFLAGS  := -std=c++11
LOCAL_LDLIBS    := -llog -landroid -lEGL -lGLESv1_CM

# Force export ANativeActivity_onCreate(), 
# Refer to: https://github.com/android-ndk/ndk/issues/381.
# Only needed when building with NDK-R14 and older
LOCAL_LDFLAGS   := -u ANativeActivity_onCreate

LOCAL_STATIC_LIBRARIES := android_native_app_glue

include $(BUILD_SHARED_LIBRARY)

$(call import-module,android/native_app_glue)
