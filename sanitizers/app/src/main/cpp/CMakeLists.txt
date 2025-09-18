cmake_minimum_required(VERSION 3.22.1)
project(sanitizers LANGUAGES CXX)

include(AppLibrary)
find_package(base CONFIG REQUIRED)

add_app_library(sanitizers SHARED native-lib.cpp)
target_link_libraries(sanitizers PRIVATE base::base log)

if(SANITIZE)
    # For asan and ubsan, we need to copy some files from the NDK into our APK.
    get_filename_component(HINT_PATH ${ANDROID_C_COMPILER} DIRECTORY)
    set(ARCH_STR ${CMAKE_ANDROID_ARCH})
    if("${CMAKE_ANDROID_ARCH_ABI}" STREQUAL "arm64-v8a")
        set(ARCH_STR "aarch64")
    elseif("${CMAKE_ANDROID_ARCH_ABI}" STREQUAL "armeabi")
        set(ARCH_STR "arm")
    elseif("${CMAKE_ANDROID_ARCH_ABI}" STREQUAL "armeabi-v7a")
        set(ARCH_STR "arm")
    elseif("${CMAKE_ANDROID_ARCH_ABI}" STREQUAL "x86")
        set(ARCH_STR "i686")
    elseif("${CMAKE_ANDROID_ARCH_ABI}" STREQUAL "x86_64")
        set(ARCH_STR "x86_64")
    endif()

    if(${SANITIZE} STREQUAL "hwasan")
        target_compile_options(sanitizers PUBLIC -fsanitize=hwaddress -fno-omit-frame-pointer)
        target_link_options(sanitizers PUBLIC -fsanitize=hwaddress)
    elseif(${SANITIZE} STREQUAL "asan")
        target_compile_options(sanitizers PUBLIC -fsanitize=address -fno-omit-frame-pointer)
        target_link_options(sanitizers PUBLIC -fsanitize=address)

        # Grab libclang_rt.asan-${ARCH_STR}-android.so from the NDK.
        file(GLOB ASAN_GLOB "${HINT_PATH}/../lib/clang/*/lib/linux")
        find_file(ASAN
            NAMES libclang_rt.asan-${ARCH_STR}-android.so
            PATHS ${ASAN_GLOB})
        get_filename_component(ASAN_NAME ${ASAN} NAME)
        set(ASAN_NAME ${CMAKE_SOURCE_DIR}/../../asan/jniLibs/${CMAKE_ANDROID_ARCH_ABI}/${ASAN_NAME})
        add_custom_command(
            TARGET sanitizers PRE_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy ${ASAN} ${ASAN_NAME})

        # Grab the asan wrapper script from the NDK.
        find_file(WRAP
            NAMES asan.sh
            HINTS ${HINT_PATH}/../../../../../wrap.sh)
        set(WRAP_NAME ${CMAKE_SOURCE_DIR}/../../asan/resources/lib/${CMAKE_ANDROID_ARCH_ABI}/wrap.sh)
        add_custom_command(
            TARGET sanitizers PRE_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy ${WRAP} ${WRAP_NAME})
    elseif(${SANITIZE} STREQUAL "ubsan")
        target_compile_options(sanitizers PUBLIC -fsanitize=undefined -fno-sanitize-recover=undefined)
        target_link_options(sanitizers PUBLIC -fsanitize=undefined -fno-sanitize-recover=undefined)

        # Grab libclang_rt.ubsan_standalone-${ARCH_STR}-android.so from the NDK.
        file(GLOB UBSAN_GLOB "${HINT_PATH}/../lib/clang/*/lib/linux")
        find_file(UBSAN
            NAMES libclang_rt.ubsan_standalone-${ARCH_STR}-android.so
            PATHS ${UBSAN_GLOB})
        get_filename_component(UBSAN_NAME ${UBSAN} NAME)
        set(UBSAN_NAME ${CMAKE_SOURCE_DIR}/../../ubsan/jniLibs/${CMAKE_ANDROID_ARCH_ABI}/${UBSAN_NAME})
        add_custom_command(
            TARGET sanitizers PRE_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy ${UBSAN} ${UBSAN_NAME})
    endif()
endif()
