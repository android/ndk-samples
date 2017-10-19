cmake_minimum_required(VERSION 3.4.1)


set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11" )

get_filename_component(COMMON_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../../../../common ABSOLUTE)
add_library(simpleModel SHARED
    simple_model.cpp
    app_main.cpp
    ${COMMON_DIR}/wrapper/NeuralNetworksApi.c)

include_directories(${COMMON_DIR} ${COMMON_DIR}/utils)

target_link_libraries(simpleModel android log)
