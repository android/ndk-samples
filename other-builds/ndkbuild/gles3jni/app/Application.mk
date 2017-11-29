APP_ABI := armeabi-v7a arm64-v8a

NDK_TOOLCHAIN_VERSION := clang
APP_STL  := gnustl_static

# this number needs to be the same as minSDKVersion inside build.gradle for now
# to guarantee the same binary result from IDE and command line directly with ndk-build
APP_PLATFORM := android-18
