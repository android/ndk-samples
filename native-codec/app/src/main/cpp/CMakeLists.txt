cmake_minimum_required(VERSION 3.22.1)
project(NativeCodec LANGUAGES CXX)

include(AppLibrary)

add_app_library(native-codec-jni SHARED
    looper.cpp
    native-codec-jni.cpp
)

target_link_libraries(native-codec-jni
    android
    log
    mediandk
    OpenMAXAL
)
