# Copyright (C) 2011 The Android Open Source Project
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

include $(CLEAR_VARS)
JNI_SRC_PATH := $(SAMPLES_ROOT)/native-media/app/src/main/cpp

LOCAL_MODULE    := native-media-jni
LOCAL_SRC_FILES := $(JNI_SRC_PATH)/native-media-jni.c \
                   $(JNI_SRC_PATH)/android_fopen.c
# for native multimedia
LOCAL_LDLIBS    += -lOpenMAXAL
# for logging
LOCAL_LDLIBS    += -llog
# for native windows
LOCAL_LDLIBS    += -landroid

LOCAL_CFLAGS    += -UNDEBUG

include $(BUILD_SHARED_LIBRARY)
