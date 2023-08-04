cmake_minimum_required(VERSION 3.22.1)
project(OrderfileDemo CXX)

# We have setup build variables that you can just comment or uncomment to use.
# Make sure to have only one build variable uncommented at a time.
# If you want to generate profiles and mapping file, make sure GENERATE_PROFILES is uncommented.
# If you want to use your generated order file to layout symbols, uncomment USE_PROFILE.


set(GENERATE_PROFILES ON)
#set(USE_PROFILE "${CMAKE_SOURCE_DIR}/demo.orderfile")

add_library(orderfiledemo SHARED orderfile.cpp)
target_link_libraries(orderfiledemo log)

if(GENERATE_PROFILES)
    # Generating profiles requires any optimization flag aside from -O0.
    # The mapping file will not generate and the profile instrumentation does not work without an optimization flag. 
    target_compile_options(orderfiledemo PRIVATE -forder-file-instrumentation -O1 -mllvm -orderfile-write-mapping=mapping.txt )
    target_link_options(orderfiledemo PRIVATE -forder-file-instrumentation )
    target_compile_definitions(orderfiledemo PRIVATE GENERATE_PROFILES)
elseif(USE_PROFILE)
    target_compile_options(orderfiledemo PRIVATE -Wl,--symbol-ordering-file=${USE_PROFILE} -Wl,--no-warn-symbol-ordering )
    target_link_options(orderfiledemo PRIVATE -Wl,--symbol-ordering-file=${USE_PROFILE} -Wl,--no-warn-symbol-ordering )
endif()