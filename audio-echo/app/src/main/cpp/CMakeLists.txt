cmake_minimum_required(VERSION 3.4.1)
project(echo LANGUAGES C CXX)

add_library(echo
  SHARED
    audio_main.cpp
    audio_player.cpp
    audio_recorder.cpp
    audio_effect.cpp
    audio_common.cpp
    debug_utils.cpp)

#include libraries needed for echo lib
target_link_libraries(echo
  PRIVATE
    OpenSLES
    android
    log
    atomic)

target_compile_options(echo
  PRIVATE
    -Wall -Werror)
