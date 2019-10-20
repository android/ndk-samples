# Copyright (C) The Android Open Source Project
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

# workaround possible long file path issue on Windows host
NDK_VER=$(firstword $(subst ., ,$(lastword $(file < $(NDK_ROOT)/source.properties))))
ifeq ($(filter $(NDK_VER),21 22 23 24 25),)
  # For NDK 21 and before, need to workaround make's abspath issue
  abspath_wa = $(join $(filter %:,$(subst :,: ,$1)),$(abspath $(filter-out %:,$(subst :,: ,$1))))
else
  abspath_wa = $(abspath $1)
endif

LOCAL_PATH:= $(call my-dir)

JNI_SRC_PATH := $(call abspath_wa, $(LOCAL_PATH)/../../../../hello-gl2/app/src/main/cpp)

include $(CLEAR_VARS)

LOCAL_MODULE    := libgl2jni
LOCAL_CFLAGS    := -Werror -std=c++11
LOCAL_SRC_FILES := $(JNI_SRC_PATH)/gl_code.cpp
LOCAL_LDLIBS    := -llog -lGLESv2

include $(BUILD_SHARED_LIBRARY)
