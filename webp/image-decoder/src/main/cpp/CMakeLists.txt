#[[
# Copyright (C) 2021 The Android Open Source Project
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
#]]
project("image_decoder")
cmake_minimum_required(VERSION 3.10.2)

set(CMAKE_VERBOSE_MAKEFILE on)


# Build native_app_glue as a static lib
include_directories(${ANDROID_NDK}/sources/android/native_app_glue)

add_library(native_app_glue STATIC
    ${ANDROID_NDK}/sources/android/native_app_glue/android_native_app_glue.c)

# Create app's shared lib
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=c99")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14 -Wall")

# Export ANativeActivity_onCreate(), 
# Refer to: https://github.com/android-ndk/ndk/issues/381.
set(CMAKE_SHARED_LINKER_FLAGS
    "${CMAKE_SHARED_LINKER_FLAGS} -u ANativeActivity_onCreate")

add_library(image_decoder SHARED
     android_main.cpp
     image_viewer.cpp
     image_decoder.cpp)

target_link_libraries(image_decoder native_app_glue jnigraphics android log m)
