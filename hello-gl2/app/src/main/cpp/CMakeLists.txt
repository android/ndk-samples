cmake_minimum_required(VERSION 3.4.1)

set(CMAKE_VERBOSE_MAKEFILE on)

# now build app's shared lib
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -Wall")
set(gl2jni_SRCS gl_code.cpp)
add_library(gl2jni SHARED ${gl2jni_SRCS})

# add lib dependencies
target_link_libraries(gl2jni android log EGL GLESv2)

