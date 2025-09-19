cmake_minimum_required(VERSION 3.22.1)
project(Vectorization LANGUAGES CXX)

include(AppLibrary)
find_package(base REQUIRED CONFIG)

add_app_library(app
    SHARED
    benchmark.cpp
    jni.cpp
)

target_compile_features(app PUBLIC cxx_std_23)
target_compile_options(app PUBLIC -fenable-matrix -fopenmp)

target_link_libraries(app
    PRIVATE
    base::base
    log
)
