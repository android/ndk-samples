cmake_minimum_required(VERSION 3.22.1)
project(NativeAudio LANGUAGES CXX)

include(AppLibrary)
find_package(base CONFIG REQUIRED)

add_app_library(native-audio-jni SHARED
    native-audio-jni.cpp
)

# Include libraries needed for native-audio-jni lib
target_link_libraries(native-audio-jni
    PRIVATE
    base::base
    android
    log
    OpenSLES
)

if(ANDROID_ABI STREQUAL riscv64)
    # This sample uses OpenSLES, which was deprecated in API 26. Our
    # minSdkVersion is 23, but we also build for riscv64, which isn't a
    # supported ABI yet and so that configuration is built for the latest API
    # level supported by the NDK.
    #
    # Longer term, this sample should migrate to Oboe.
    target_compile_options(native-audio-jni PRIVATE -Wno-deprecated-declarations)
endif()
