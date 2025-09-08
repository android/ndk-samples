cmake_minimum_required(VERSION 3.22.1)
project(HelloJniCallback LANGUAGES CXX)

add_library(hello-jnicallback SHARED hello-jnicallback.cpp)

# Include libraries needed for hello-jnicallback lib
target_link_libraries(hello-jnicallback
    android
    log
)
