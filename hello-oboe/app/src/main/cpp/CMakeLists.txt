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
project(hello-oboe LANGUAGES CXX)

# add oboe pre-release lib hosted at https://maven.google.com/web/index.html
# under com.google.oboe:oboe. For documentation about oboe pre-built lib, refer to
# https://github.com/google/oboe/blob/master/docs/GettingStarted.md#option-1-using-pre-built-binaries-and-headers
find_package(oboe REQUIRED CONFIG)

# build application with the oboe lib
add_library(${PROJECT_NAME} SHARED  hello-oboe.cpp)
target_link_libraries(${PROJECT_NAME} oboe::oboe android log)

# Enable optimization flags: if having problems with source level debugging,
# disable -Ofast ( and debug ), re-enable after done debugging.
target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Werror "$<$<CONFIG:RELEASE>:-Ofast>")
