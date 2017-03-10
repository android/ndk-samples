cmake_minimum_required(VERSION 3.4.1)
# set targetPlatform, will be passed in from gradle when this sample is completed
# openGL Supportability
# platform         status
#   (0 12)           ES2/ES3 not supported
#   [12, 18)         ES2 only, for ES3, app do dynamic load
#   [18, 24)         ES2 & ES3
#   [24, infinite)   ES2 & ES3 & Vulkan


set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -fno-rtti -fno-exceptions -Wall")

if (${ANDROID_PLATFORM_LEVEL} LESS 12)
  message(FATAL_ERROR "OpenGL 2 is not supported before API level 11 (currently using ${ANDROID_PLATFORM_LEVEL}).")
  return()
elseif (${ANDROID_PLATFORM_LEVEL} LESS 18)
  add_definitions("-DDYNAMIC_ES3")
  set(OPENGL_LIB GLESv2)
else ()
  set(OPENGL_LIB GLESv3)
endif (${ANDROID_PLATFORM_LEVEL} LESS 11)

add_library(gles3jni SHARED
            gl3stub_wrapper.c
            gles3jni.cpp 
            RendererES2.cpp
            RendererES3.cpp)

# Include libraries needed for gles3jni lib
target_link_libraries(gles3jni
                      ${OPENGL_LIB}
                      android
                      EGL
                      log
                      m)
