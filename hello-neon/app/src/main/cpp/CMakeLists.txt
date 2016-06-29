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
# name: helloneon-intrinsics.c (It is named EXACTLY as this on disk,
#                              just like a normal source file)
# then set up neon flag for neon files
# [This example only build for armeabi-v7a, x86 could be done the same way]
#
if (${ANDROID_ABI} STREQUAL "armeabi-v7a")
  set(hello_neon_SRCS ${hello_neon_SRCS}
                      helloneon-intrinsics.c)
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DHAVE_NEON=1")

  # make a list of neon files and add neon compiling flags to them
  set_property(SOURCE helloneon-intrinsics.c
               APPEND_STRING PROPERTY COMPILE_FLAGS " -mfpu=neon ")
endif ()

add_library(hello-neon SHARED ${hello_neon_SRCS})
target_link_libraries(hello-neon android log cpufeatures)

