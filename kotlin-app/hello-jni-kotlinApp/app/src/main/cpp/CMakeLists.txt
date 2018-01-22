cmake_minimum_required(VERSION 3.4.1)

add_library(kotlin-jni SHARED
            kotlin-jni.c)

# Include libraries needed for kotlin-jni lib
target_link_libraries(kotlin-jni
            android
            log)
