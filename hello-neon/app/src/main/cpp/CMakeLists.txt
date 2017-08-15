cmake_minimum_required(VERSION 3.4.1)

# build cpufeatures as a static lib
add_library(cpufeatures STATIC
            ${ANDROID_NDK}/sources/android/cpufeatures/cpu-features.c)

# build app's shared lib

# set up neon build flag for file using intrinsics
# name: helloneon-intrinsics.c (It is named EXACTLY as this on disk,
#                              just like a normal source file)
# then set up neon flag for neon files
# [This example only build for armeabi-v7a, x86 could be done the same way]
#
if (${ANDROID_ABI} STREQUAL "armeabi-v7a")
  # make a list of neon files and add neon compiling flags to them
  set(neon_SRCS helloneon-intrinsics.c)

  set_property(SOURCE ${neon_SRCS}
               APPEND_STRING PROPERTY COMPILE_FLAGS " -mfpu=neon")
  add_definitions("-DHAVE_NEON=1")
elseif (${ANDROID_ABI} STREQUAL "x86")
    set(neon_SRCS helloneon-intrinsics.c)
    set_property(SOURCE ${neon_SRCS} APPEND_STRING PROPERTY COMPILE_FLAGS
        " -mssse3  -Wno-unknown-attributes \
                   -Wno-deprecated-declarations \
                   -Wno-constant-conversion \
                   -Wno-static-in-inline")
    add_definitions(-DHAVE_NEON_X86=1 -DHAVE_NEON=1)
else ()
  set(neon_SRCS)
endif ()

add_library(hello-neon SHARED helloneon.c ${neon_SRCS})
target_include_directories(hello-neon PRIVATE
    ${ANDROID_NDK}/sources/android/cpufeatures)

target_link_libraries(hello-neon android cpufeatures log)

