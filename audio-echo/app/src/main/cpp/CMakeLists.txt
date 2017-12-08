cmake_minimum_required(VERSION 3.4.1)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -Wall -Werror")

add_library(echo SHARED
            audio_common.cpp
            audio_main.cpp
            audio_player.cpp
            audio_recorder.cpp
            debug_utils.cpp)

# include libraries needed for hello-jni lib
target_link_libraries(echo
                      android
                      atomic
                      log
                      OpenSLES)

