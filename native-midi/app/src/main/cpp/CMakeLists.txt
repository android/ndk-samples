cmake_minimum_required(VERSION 3.22.1)
project(native_midi LANGUAGES C CXX)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Werror -O0")

add_library(${PROJECT_NAME}
  SHARED
    AppMidiManager.cpp
    MainActivity.cpp
)

target_link_libraries(${PROJECT_NAME} PRIVATE amidi OpenSLES android log)
