# Lopyright (C) The Android Open Source Project
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

abspath_wa = $(join $(filter %:,$(subst :,: ,$1)),$(abspath $(filter-out %:,$(subst :,: ,$1))))

LOCAL_PATH := $(call my-dir)
PROJECT_DIR :=bitmap-plasma
JNI_SRC_PATH := $(call abspath_wa, $(LOCAL_PATH)/../../../../$(PROJECT_DIR)/app/src/main/cpp)

include $(CLEAR_VARS)

LOCAL_MODULE    := plasma
LOCAL_SRC_FILES := $(JNI_SRC_PATH)/plasma.c
LOCAL_LDLIBS    := -lm -llog -ljnigraphics
LOCAL_CFLAGS    := -Wall -Werror -Wno-unused-function

NDK_TOOLCHAIN_VERSION := clang
include $(BUILD_SHARED_LIBRARY)
