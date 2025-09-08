cmake_minimum_required(VERSION 3.22.1)
project(HelloGl2 LANGUAGES CXX)

include(AppLibrary)

add_app_library(gl2jni SHARED gl_code.cpp)

target_link_libraries(gl2jni
    android
    log
    EGL
    GLESv2
)

