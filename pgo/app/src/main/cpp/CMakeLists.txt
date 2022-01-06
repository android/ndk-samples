cmake_minimum_required(VERSION 3.18.1)
project(PGODemo CXX)

# PGO uses profile data from previous app runs to optimize the build. To generate profile data,
# follow https://developer.android.com/ndk/guides/pgo. Keep the configuration below when generating
# profile data. To use the profile data, comment out GENERATE_PROFILES and uncomment USE_PROFILE
# after correcting the path to point to your merged profile data.
#
# To pull the data from the device, you'll need to move it from an app-writable directory to a shell
# readable directory for adb pull. For example:
# adb shell "run-as com.example.pgodemo sh -c 'cat /data/user/0/com.example.pgodemo/cache/default.profraw' | cat > /data/local/tmp/default.profraw"
# adb pull /data/local/tmp/default.profraw
set(GENERATE_PROFILES ON)
# set(USE_PROFILE "${CMAKE_SOURCE_DIR}/pgo_profile.profdata")

add_library(pgodemo SHARED pgodemo.cpp)
target_link_libraries(pgodemo log)

if(GENERATE_PROFILES)
    target_compile_options(pgodemo PRIVATE -fprofile-generate)
    target_link_options(pgodemo PRIVATE -fprofile-generate)
    target_compile_definitions(pgodemo PRIVATE GENERATE_PROFILES)
elseif(USE_PROFILE)
    target_compile_options(pgodemo PRIVATE -fprofile-use=${USE_PROFILE})
    target_link_options(pgodemo PRIVATE -fprofile-use=${USE_PROFILE})
endif()