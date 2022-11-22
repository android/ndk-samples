cmake_minimum_required(VERSION 3.4.1)
project(hello-vk)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)


#List the environment as CMAKE sees it
execute_process(
        COMMAND ${CMAKE_COMMAND} -E environment
)

# now build app's shared lib
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17 -Wall")

add_definitions(-DVK_USE_PLATFORM_ANDROID_KHR=1)
add_library(native_app_glue STATIC
        "${ANDROID_NDK}/sources/android/native_app_glue/android_native_app_glue.c")


add_library(hellovkjni SHARED
            vk_main.cpp
            )

set_property(TARGET hellovkjni APPEND_STRING PROPERTY LINK_FLAGS " -u ANativeActivity_onCreate")

target_include_directories(hellovkjni PRIVATE
        ${ANDROID_NDK}/sources/android/native_app_glue)

# add lib dependencies
target_link_libraries(hellovkjni
                      android
                      native_app_glue
                      vulkan
                      log)



