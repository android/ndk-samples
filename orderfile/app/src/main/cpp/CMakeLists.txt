cmake_minimum_required(VERSION 3.18.1)
project(OrderfileDemo CXX)

set(GENERATE_PROFILES ON)
#set(USE_PROFILE "${CMAKE_SOURCE_DIR}/demo.orderfile")

add_library(orderfiledemo SHARED orderfile.cpp)
target_link_libraries(orderfiledemo log)

if(GENERATE_PROFILES)
    target_compile_options(orderfiledemo PRIVATE -forder-file-instrumentation -O1 -mllvm -orderfile-write-mapping=mapping.txt )
    target_link_options(orderfiledemo PRIVATE -forder-file-instrumentation )
    target_compile_definitions(orderfiledemo PRIVATE GENERATE_PROFILES)
elseif(USE_PROFILE)
    target_compile_options(orderfiledemo PRIVATE -Wl,--symbol-ordering-file=${USE_PROFILE} -Wl,--no-warn-symbol-ordering )
    target_link_options(orderfiledemo PRIVATE -Wl,--symbol-ordering-file=${USE_PROFILE} -Wl,--no-warn-symbol-ordering )
endif()