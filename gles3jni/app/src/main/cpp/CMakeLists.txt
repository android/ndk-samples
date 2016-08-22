cmake_minimum_required(VERSION 3.4.1)
# set targetPlatform, will be passed in from gradle when this sample is completed
# openGL Supportability
# platform         status
#   (0 11)            ES2/ES3 not supported
#   [11, 18)          ES2 only, for ES3, app do dynamic load
#   [18, infinite)    ES2  & ES3
#   [24 ...)          ES2/ES3/Vulkan


set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -fno-rtti -fno-exceptions -Wall")
set(OPENGL_LIB  GLESv3)
set(gles3jni_SRCS gles3jni.cpp 
                  RendererES3.cpp
                  gl3stub.c.inc
                  RendererES2.cpp)

if (${ANDROID_PLATFORM_LEVEL} LESS 11)
  MESSAGE(ERROR " OpenGL 2 is not supported")
  return()
elseif (${ANDROID_PLATFORM_LEVEL} LESS 18)
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DDYNAMIC_ES3")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DDYNAMIC_ES3")
  set(gles3jni_SRCS ${gles3jni_SRCS} gl3stub_wrapper.c)
  set(OPENGL_LIB  GLESv2)
endif (${ANDROID_PLATFORM_LEVEL} LESS 11)

add_library(gles3jni SHARED ${gles3jni_SRCS})

# Include libraries needed for gles3jni lib
target_link_libraries(gles3jni android log m EGL ${OPENGL_LIB})
