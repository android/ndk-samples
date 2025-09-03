cmake_minimum_required(VERSION 3.22.1)
project(CameraCommon LANGUAGES CXX)

include(AppLibrary)

add_app_library(camera-utils
    STATIC
    camera_utils.cpp
)

target_include_directories(camera-utils PUBLIC include)
target_link_libraries(camera-utils PUBLIC log)
