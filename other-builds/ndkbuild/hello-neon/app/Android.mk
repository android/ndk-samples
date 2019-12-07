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
include $(CLEAR_VARS)

include $(LOCAL_PATH)/../../common.mk

PROJECT_DIR := hello-neon
JNI_SRC_PATH := $(SAMPLES_ROOT)/$(PROJECT_DIR)/app/src/main/cpp


LOCAL_MODULE := hello-neon

LOCAL_SRC_FILES := $(JNI_SRC_PATH)/helloneon.c

ifeq ($(TARGET_ARCH_ABI),$(filter $(TARGET_ARCH_ABI), armeabi-v7a x86))
    LOCAL_CFLAGS := -DHAVE_NEON=1
ifeq ($(TARGET_ARCH_ABI),x86)
    LOCAL_CFLAGS += -DHAVE_NEON_X86=1 -mssse3
endif
    LOCAL_SRC_FILES += $(JNI_SRC_PATH)/helloneon-intrinsics.c.neon
endif

LOCAL_STATIC_LIBRARIES := cpufeatures

LOCAL_LDLIBS := -llog

include $(BUILD_SHARED_LIBRARY)

$(call import-add-path, $(NDK_ROOT)/sources/android)
$(call import-module,cpufeatures)
