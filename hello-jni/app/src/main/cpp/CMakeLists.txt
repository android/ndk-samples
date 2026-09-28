cmake_minimum_required(VERSION 4.1.0)
project("hello-jni" LANGUAGES CXX)

include(AppLibrary)
find_package(base CONFIG REQUIRED)

add_app_library(hello-jni SHARED
    hello-jni.cpp
)

target_link_libraries(hello-jni
    PRIVATE
    base::base
    android
    log
)
