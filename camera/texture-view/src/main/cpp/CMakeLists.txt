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

cmake_minimum_required(VERSION 3.22.1)
project(CameraTextureView LANGUAGES CXX)

include(AppLibrary)
find_package(base REQUIRED CONFIG)
find_package(camera-utils REQUIRED CONFIG)

add_app_library(camera_textureview SHARED
    android_main.cpp
    camera_manager.cpp
    camera_engine.cpp
    camera_listeners.cpp
)

target_link_libraries(camera_textureview
    PRIVATE
    base::base
    camera-utils::camera-utils
    dl
    android
    log
    m
    camera2ndk
)
