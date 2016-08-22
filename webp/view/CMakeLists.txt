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

set(webp_src_dir ../libwebp)
SET(WEBP_ENABLE_SWAP_16BIT_CSP ON CACHE BOOL
    "Enable byte swap for 16 bit colorspaces." FORCE)
add_subdirectory(${webp_src_dir} ${webp_src_dir}/build/)

# build native_app_glue as a static lib
include_directories(${ANDROID_NDK}/sources/android/native_app_glue)

add_library(app-glue STATIC
            ${ANDROID_NDK}/sources/android/native_app_glue/android_native_app_glue.c)

# now build app's shared lib
include_directories(${webp_src_dir}/examples
                    ${webp_src_dir}/src)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=c99")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
add_library(webp-view SHARED src/main/cpp/webp_decode.cpp
                             src/main/cpp/webp_view.cpp)

# add lib dependencies
target_link_libraries(webp-view android log m app-glue webp)
