cmake_minimum_required(VERSION 4.1.0)
project(native_midi LANGUAGES CXX)

include(AppLibrary)
find_package(base CONFIG REQUIRED)

add_app_library(${PROJECT_NAME}
    SHARED
    AppMidiManager.cpp
    MainActivity.cpp
)

target_link_libraries(${PROJECT_NAME}
    PRIVATE
    base::base
    amidi
    OpenSLES
    android
    log
)
