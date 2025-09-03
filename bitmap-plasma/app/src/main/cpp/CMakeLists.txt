cmake_minimum_required(VERSION 3.22.1)
project(BitmapPlasma LANGUAGES CXX)

include(AppLibrary)

add_app_library(plasma SHARED plasma.cpp)

# Include libraries needed for plasma lib
target_link_libraries(plasma
    android
    jnigraphics
    log
    m
)
