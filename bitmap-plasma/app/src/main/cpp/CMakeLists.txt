cmake_minimum_required(VERSION 3.22.1)
project(BitmapPlasma LANGUAGES CXX)

include(AppLibrary)
find_package(base CONFIG REQUIRED)

add_app_library(plasma SHARED jni.cpp plasma.cpp)

target_link_libraries(plasma
    base::base
    android
    jnigraphics
    log
    m
)
