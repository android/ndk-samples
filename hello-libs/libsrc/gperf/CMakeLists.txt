#
# Copyright (C) 2010 The Android Open Source Project
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

add_library(gperf SHARED src/gperf.c)

# copy out the lib binary and remove generated files
set(distribution_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../../distribution)
add_custom_command(TARGET gperf POST_BUILD
                   COMMAND "${CMAKE_COMMAND}" -E
                   copy "$<TARGET_FILE:gperf>"
                   "${distribution_DIR}/gperf/lib/${ANDROID_ABI}/$<TARGET_FILE_NAME:gperf>"
                   COMMAND "${CMAKE_COMMAND}" -E
                   copy "${CMAKE_CURRENT_SOURCE_DIR}/src/gperf.h"
                   "${distribution_DIR}/gperf/include/gperf.h"
                   COMMAND "${CMAKE_COMMAND}" -E
                   remove_directory "${CMAKE_CURRENT_BINARY_DIR}"
                   COMMENT "Copying gperf to output directory")

