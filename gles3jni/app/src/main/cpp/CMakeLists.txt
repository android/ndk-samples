cmake_minimum_required(VERSION 3.22.1)
project(Gles3Jni LANGUAGES CXX)

include(AppLibrary)
find_package(base CONFIG REQUIRED)

add_app_library(gles3jni SHARED
    ${GL3STUB_SRC}
    gles3jni.cpp
    RendererES2.cpp
    RendererES3.cpp
)

target_link_libraries(gles3jni
    base::base
    GLESv3
    android
    EGL
    log
    m
)
