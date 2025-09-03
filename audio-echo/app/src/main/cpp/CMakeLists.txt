cmake_minimum_required(VERSION 3.22.1)
project(echo LANGUAGES C CXX)

include(AppLibrary)

add_app_library(echo
    SHARED
    audio_main.cpp
    audio_player.cpp
    audio_recorder.cpp
    audio_effect.cpp
    audio_common.cpp
    debug_utils.cpp)

#include libraries needed for echo lib
target_link_libraries(echo
    PRIVATE
    OpenSLES
    android
    log
    atomic)

if (ANDROID_ABI STREQUAL riscv64)
    # This sample uses OpenSLES, which was deprecated in API 26. Our
    # minSdkVersion is 21, but we also build for riscv64, which isn't a
    # supported ABI yet and so that configuration is built for the latest API
    # level supported by the NDK.
    #
    # Longer term, this sample should probably be deleted. App developers
    # should be using Oboe rather than OpenSLES, and we already have a separate
    # sample for Oboe. There is some potentially useful audio latency
    # measurement in this sample that should be moved to the Oboe sample before
    # we delete it though.
    target_compile_options(echo PRIVATE -Wno-deprecated-declarations)
endif ()
