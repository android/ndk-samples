
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

cmake_minimum_required(VERSION 3.4.1)

# build application's shared lib
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=gnu++11")

add_library(hello-cdep SHARED
            hello-cdep.cpp)

target_link_libraries(hello-cdep
                      android
                      log)
# Temporary workaround:
# android.toolchain.cmake does
#  set(CMAKE_SYSTEM_VERSION 1)
# CDep lib binary supports android-23 and above.
set(CMAKE_SYSTEM_VERSION 23)

find_package(cdep-dependencies REQUIRED)
add_all_cdep_dependencies(hello-cdep)
