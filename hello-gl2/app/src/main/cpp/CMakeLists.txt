cmake_minimum_required(VERSION 3.22.1)
project(HelloGl2 LANGUAGES CXX)

include(AppLibrary)
find_package(base CONFIG REQUIRED)

add_app_library(gl2jni SHARED gl_code.cpp)

target_link_libraries(gl2jni
    PRIVATE
    base::base
    android
    log
    EGL
    GLESv2
)

