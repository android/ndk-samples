cmake_minimum_required(VERSION 3.4.1)

##########################################################
# build cpufeatures as a static lib
include_directories(./ndk_helper
                    ${ANDROID_NDK}/sources/android/cpufeatures)
add_library(cpufeatures  STATIC
            ${ANDROID_NDK}/sources/android/cpufeatures/cpu-features.c)

##########################################################
# build app's shared lib
set(hello_neon_SRCS helloneon.c)

# set up neon build flag for file using intrinsics
# name: helloneon-intrinsics.c.neon  (It is named EXACTLY as this on disk)
#                                    (you could name it otherwise)
# 1) Inform CMake this file is part of the C source file
# 2) Set up neon flag for neon file
# [This example only build for armeabi-v7a, x86 could be done the same way]
#
if (${ANDROID_ABI} STREQUAL "armeabi-v7a")
  set(hello_neon_SRCS ${hello_neon_SRCS}
                      helloneon-intrinsics.neon)
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DHAVE_NEON=1")

  file(GLOB_RECURSE NEON_TYPE *.neon)
  set_property(SOURCE ${NEON_TYPE} PROPERTY LANGUAGE C)
  set_property(SOURCE ${NEON_TYPE}
               APPEND_STRING PROPERTY COMPILE_FLAGS " -mfpu=neon -x c ")
endif ()

add_library(hello-neon SHARED ${hello_neon_SRCS})
target_link_libraries(hello-neon android log cpufeatures)

