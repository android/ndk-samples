cmake_minimum_required(VERSION 3.4.1)
add_library(native-lib SHARED native-lib.cpp)

if(ANDROID_ABI STREQUAL "arm64-v8a")
  target_compile_options(native-lib PUBLIC -fsanitize=hwaddress -fno-omit-frame-pointer)
  set_target_properties(native-lib PROPERTIES LINK_FLAGS -fsanitize=hwaddress)
endif()
