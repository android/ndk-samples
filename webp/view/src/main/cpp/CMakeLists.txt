#
# Copyright (C)  The Android Open Source Project
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

get_filename_component(WEBP_SAMPLE_PROJ_DIR
                       ${CMAKE_CURRENT_SOURCE_DIR}/../../../.. ABSOLUTE)
set(WEBP_SRC_DIR ${WEBP_SAMPLE_PROJ_DIR}/libwebp)
# clone the dependency repo.
# git submodule could also be used if this sample does not need
#     Android Studio's "Import Android code sample" option
if ((NOT EXISTS ${WEBP_SRC_DIR}) OR
    (NOT EXISTS ${WEBP_SRC_DIR}/CMakeLists.txt))
    execute_process(COMMAND git clone -b 1.0.0
                            https://chromium.googlesource.com/webm/libwebp
                            libwebp
                    WORKING_DIRECTORY ${WEBP_SAMPLE_PROJ_DIR}/)
endif()

SET(WEBP_ENABLE_SWAP_16BIT_CSP ON CACHE BOOL
    "Enable byte swap for 16 bit colorspaces." FORCE)
add_subdirectory(${WEBP_SRC_DIR} ${WEBP_SRC_DIR}/build/)

# build native_app_glue as a static lib
include_directories(${ANDROID_NDK}/sources/android/native_app_glue)

add_library(native_app_glue STATIC
    ${ANDROID_NDK}/sources/android/native_app_glue/android_native_app_glue.c)

# now build app's shared lib
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=c99")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -Wall")

# Export ANativeActivity_onCreate(), 
# Refer to: https://github.com/android-ndk/ndk/issues/381.
set(CMAKE_SHARED_LINKER_FLAGS
    "${CMAKE_SHARED_LINKER_FLAGS} -u ANativeActivity_onCreate")

add_library(webp_view SHARED
    webp_decode.cpp
    webp_view.cpp)
target_include_directories(webp_view PRIVATE
    ${WEBP_SRC_DIR}/examples
    ${WEBP_SRC_DIR}/src)

# add lib dependencies
target_link_libraries(webp_view android log m native_app_glue webp)
