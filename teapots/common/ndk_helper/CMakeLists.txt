# build native_app_glue as a static lib
cmake_minimum_required(VERSION 3.22.1)

include(AndroidNdkModules)
android_ndk_import_module_native_app_glue()

add_library(NdkHelper
  STATIC
    gestureDetector.cpp
    gl3stub.cpp
    GLContext.cpp
    interpolator.cpp
    JNIHelper.cpp
    perfMonitor.cpp
    sensorManager.cpp
    shader.cpp
    tapCamera.cpp
    vecmath.cpp
)
# TODO: Remove this
# This does not do what it looks like it does. It looks like it's redundant with
# the target_include_directories(... PUBLIC ...) below. What it's actually doing
# is re-exporting the headers for every library linked below. This is how all
# the teapots samples are getting their native_app_glue. They should depend on
# it explicitly.
set_target_properties(NdkHelper
  PROPERTIES
    INTERFACE_INCLUDE_DIRECTORIES $<TARGET_PROPERTY:NdkHelper,INCLUDE_DIRECTORIES>
)
target_include_directories(NdkHelper
  PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

# TODO: Audit, make private.
# Some of these are here because it was a common location to link the
# dependenices of the various teapot samples. Those samples should link their
# dependencies explicitly.
target_link_libraries(NdkHelper
  PUBLIC
    native_app_glue
    GLESv2
    EGL
    log
    android
    atomic
)
