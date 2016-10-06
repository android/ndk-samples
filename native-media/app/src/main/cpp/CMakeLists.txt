cmake_minimum_required(VERSION 3.4.1)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -UNDEBUG")

add_library(native-media-jni SHARED
            android_fopen.c
            native-media-jni.c)

# Include libraries needed for native-media-jni lib
target_link_libraries(native-media-jni
                      android
                      log
                      OpenMAXAL)

