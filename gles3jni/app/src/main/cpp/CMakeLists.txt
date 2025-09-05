cmake_minimum_required(VERSION 3.22.1)
project(Gles3Jni LANGUAGES CXX)

include(AppLibrary)

add_app_library(gles3jni SHARED
    ${GL3STUB_SRC}
    gles3jni.cpp
    RendererES2.cpp
    RendererES3.cpp
)

# Include libraries needed for gles3jni lib
target_link_libraries(gles3jni
    GLESv3
    android
    EGL
    log
    m
)
