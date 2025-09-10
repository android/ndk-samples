#
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

cmake_minimum_required(VERSION 4.1.0)
project(NativeActivity LANGUAGES C CXX)

include(AppLibrary)

add_library(native_app_glue STATIC
    ${ANDROID_NDK}/sources/android/native_app_glue/android_native_app_glue.c
)

add_app_library(native-activity SHARED main.cpp)

target_include_directories(native-activity PRIVATE
    ${ANDROID_NDK}/sources/android/native_app_glue
)

target_link_libraries(native-activity
    android
    $<LINK_LIBRARY:WHOLE_ARCHIVE,native_app_glue>
    EGL
    GLESv1_CM
    log
)

if (ANDROID_ABI STREQUAL riscv64)
    # This sample uses Sensor Manager and Choreographer APIs which are
    # deprecated in modern API levels. Our minSdkVersion is 21, but we also
    # build for riscv64, which isn't a supported ABI yet and so that
    # configuration is built for the latest API level supported by the NDK.
    #
    # This sample should be tweaked to use the replacement APIs when they're
    # available at runtime and only use the older APIs on older devices. For
    # now, just disable the deprecation warnings so we can get the riscv64
    # samples building in CI, and we can come back to clean that up later.
    target_compile_options(native-activity PRIVATE -Wno-deprecated-declarations)
endif ()
