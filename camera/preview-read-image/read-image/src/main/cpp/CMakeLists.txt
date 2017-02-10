#
# Copyright (C)  2017 The Android Open Source Project
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

cmake_minimum_required(VERSION 3.4.1)

set(CMAKE_VERBOSE_MAKEFILE on)
set(COMMON_SOURCE_DIR ${CMAKE_SOURCE_DIR}/../../../../common)

# build native_app_glue as a static lib
include_directories(${ANDROID_NDK}/sources/android/native_app_glue
                    ${COMMON_SOURCE_DIR}
                    ${COMMON_SOURCE_DIR}/utils)

add_library(app_glue STATIC
            ${ANDROID_NDK}/sources/android/native_app_glue/android_native_app_glue.c)

# now build app's shared lib
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=c99")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -Werror")
add_library(camera_view SHARED
            ${CMAKE_CURRENT_SOURCE_DIR}/android_main.cpp
            ${CMAKE_CURRENT_SOURCE_DIR}/camera_manager.cpp
            ${CMAKE_CURRENT_SOURCE_DIR}/image_reader.cpp
            ${COMMON_SOURCE_DIR}/utils/camera_utils.cpp)

# add lib dependencies
target_link_libraries(camera_view
                      android
                      log
                      m
                      app_glue
                      camera2ndk
                      mediandk)
