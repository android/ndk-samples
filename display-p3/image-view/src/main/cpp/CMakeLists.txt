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

get_filename_component(THIRD_PARTY_LIB_DIR
    ${CMAKE_CURRENT_SOURCE_DIR}/../../../../third_party
    ABSOLUTE)

# now build app's shared lib
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17 -Werror -Wall -Wno-unused-function")

add_library(native-activity SHARED
    ${ANDROID_NDK}/sources/android/native_app_glue/android_native_app_glue.c
    AssetUtil.cpp
    AndroidMain.cpp
    WideColorCtx.cpp
    ShaderProgram.cpp
    AppTexture.cpp
    AssetTexture.cpp
    ImageViewEngine.cpp
    gldebug.cpp
    ColorSpaceTransform.cpp
    InputEventHandler.cpp)

target_include_directories(native-activity PRIVATE
    ${ANDROID_NDK}/sources/android/native_app_glue
    ${THIRD_PARTY_LIB_DIR}
    ${THIRD_PARTY_LIB_DIR}/mathfu/include
    ${THIRD_PARTY_LIB_DIR}/mathfu/dependencies/vectorial/include)

# add lib dependencies
target_link_libraries(native-activity
    android
    log
    EGL
    GLESv3)
