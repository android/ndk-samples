cmake_minimum_required(VERSION 3.22.1)
project("hello-jni" LANGUAGES CXX)

include(AppLibrary)

add_app_library(hello-jni SHARED
    hello-jni.cpp
)

target_link_libraries(hello-jni
    android
    log
)
