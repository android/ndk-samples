#
# Copyright 2017 The Android Open Source Project
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

# Pick up a stable Oboe lib version to use:
# in the future, will point to a release branch rather than a commit hash
set(OBOE_VERSION bb066c330a201913bd18686901381ad820c41296)

### INCLUDE OBOE LIBRARY ###
get_filename_component(OBOE_SAMPLE_PROJ_HOME
        ${CMAKE_CURRENT_SOURCE_DIR}/.. ABSOLUTE)
set(OBOE_SRC_DIR ${OBOE_SAMPLE_PROJ_HOME}/oboe)

# clone the dependency repo.
# git submodule could also be used if this sample does not need
#     Android Studio's "Import Android code sample" option
if ((NOT EXISTS ${OBOE_SRC_DIR}) OR
(NOT EXISTS ${OBOE_SRC_DIR}/CMakeLists.txt))
    execute_process(COMMAND git clone https://github.com/google/oboe.git oboe
            WORKING_DIRECTORY ${OBOE_SAMPLE_PROJ_HOME}/)
    execute_process(COMMAND git checkout ${OBOE_VERSION}
            WORKING_DIRECTORY ${OBOE_SAMPLE_PROJ_HOME}/oboe)
endif()

# Add the Oboe library as a subproject. Since Oboe is an out-of-tree source library we must also
# specify a binary directory
add_subdirectory(${OBOE_SRC_DIR} ./oboe-bin)

# Include the Oboe headers
include_directories(${OBOE_SRC_DIR}/include)

### END OBOE INCLUDE SECTION ###

# Build the libhello-oboe library
add_library(hello-oboe SHARED
        hello-oboe.cpp
        )

# Specify the libraries needed for hello-oboe
target_link_libraries(hello-oboe android log oboe)

# Enable optimization flags: if having problems with source level debugging,
# disable -Ofast ( and debug ), re-enable after done debugging.
target_compile_options(hello-oboe PRIVATE -Wall -Werror "$<$<CONFIG:RELEASE>:-Ofast>")
