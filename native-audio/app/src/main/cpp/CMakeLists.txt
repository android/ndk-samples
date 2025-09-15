cmake_minimum_required(VERSION 3.22.1)
project(NativeAudio LANGUAGES CXX)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall")

add_library(native-audio-jni SHARED
    native-audio-jni.cpp
)

# Include libraries needed for native-audio-jni lib
target_link_libraries(native-audio-jni
    android
    log
    OpenSLES
)
