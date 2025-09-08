cmake_minimum_required(VERSION 3.22.1)
project(HelloJniCallback LANGUAGES CXX)

include(AppLibrary)

add_app_library(hello-jnicallback SHARED hello-jnicallback.cpp)

target_link_libraries(hello-jnicallback
    android
    log
)
