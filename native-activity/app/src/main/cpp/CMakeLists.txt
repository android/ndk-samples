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

cmake_minimum_required(VERSION 3.22.1)
project(NativeActivity LANGUAGES C CXX)

# build native_app_glue as a static lib
set(${CMAKE_C_FLAGS}, "${CMAKE_C_FLAGS}")
add_library(native_app_glue STATIC
    ${ANDROID_NDK}/sources/android/native_app_glue/android_native_app_glue.c)

# now build app's shared lib
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Werror")

# Export ANativeActivity_onCreate(),
# Refer to: https://github.com/android-ndk/ndk/issues/381.
set(CMAKE_SHARED_LINKER_FLAGS
    "${CMAKE_SHARED_LINKER_FLAGS} -u ANativeActivity_onCreate")

add_library(native-activity SHARED main.cpp)

target_include_directories(native-activity PRIVATE
    ${ANDROID_NDK}/sources/android/native_app_glue)

# add lib dependencies
target_link_libraries(native-activity
    android
    native_app_glue
    EGL
    GLESv1_CM
    log)

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
