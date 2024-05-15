cmake_minimum_required(VERSION 3.22.1)
project(Vectorization LANGUAGES CXX)

add_compile_options(-Wall -Wextra -Werror)

find_package(base REQUIRED CONFIG)

add_library(app
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

target_link_options(app
    PRIVATE
    -flto
    -Wl,--version-script,${CMAKE_SOURCE_DIR}/libapp.map.txt
)

set_target_properties(app
    PROPERTIES
    LINK_DEPENDS ${CMAKE_SOURCE_DIR}/libapp.map.txt
)
