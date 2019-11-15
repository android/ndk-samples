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

include $(CLEAR_VARS)

JNI_SRC_PATH := $(SAMPLES_ROOT)/gles3jni/app/src/main/cpp

LOCAL_MODULE    := libgles3jni
LOCAL_CPPFLAGS    := -Werror -std=c++11 -fno-rtti -fno-exceptions -Wall
LOCAL_SRC_FILES := $(JNI_SRC_PATH)/gles3jni.cpp    \
				   $(JNI_SRC_PATH)/RendererES2.cpp \
				   $(JNI_SRC_PATH)/RendererES3.cpp

OPENGL_LIB := GLESv3
#  To build for openGL ES2 compatible platforms,
#  enable the following to use stub
#   OPENGL_LIB := GLESv2
#   LOCAL_SRC_FILES += $(JNI_SRC_PATH)/gl3stub.c
#   LOCAL_CFLAGS += -DDYNAMIC_ES3

LOCAL_LDLIBS    := -llog -l$(OPENGL_LIB) -lEGL

include $(BUILD_SHARED_LIBRARY)
