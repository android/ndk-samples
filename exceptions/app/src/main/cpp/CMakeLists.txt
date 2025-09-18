cmake_minimum_required(VERSION 3.22.1)
project(exceptions LANGUAGES CXX)

include(AppLibrary)
find_package(base CONFIG REQUIRED)

add_app_library(
    exceptions
    SHARED
    native-lib.cpp
    exception_helper.cpp
)

target_link_libraries(
    exceptions
    PRIVATE
    base::base
    log
)