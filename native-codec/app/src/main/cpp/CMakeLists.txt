cmake_minimum_required(VERSION 3.22.1)
project(NativeCodec LANGUAGES CXX)

include(AppLibrary)
find_package(base CONFIG REQUIRED)

add_app_library(native-codec-jni SHARED
    looper.cpp
    native-codec-jni.cpp
)

target_link_libraries(native-codec-jni
    PRIVATE
    base::base
    android
    log
    mediandk
    OpenMAXAL
)
