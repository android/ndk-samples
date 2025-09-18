cmake_minimum_required(VERSION 3.22.1)
project(SensorGraph LANGUAGES CXX)

include(AppLibrary)
find_package(base CONFIG REQUIRED)

add_app_library(accelerometergraph SHARED sensorgraph.cpp)

target_link_libraries(accelerometergraph
    PRIVATE
    base::base
    android
    GLESv2
    log
)
