add_library(ndk-helper STATIC
            gestureDetector.cpp
            gl3stub.cpp
            GLContext.cpp
            interpolator.cpp
            JNIHelper.cpp
            perfMonitor.cpp
            sensorManager.cpp
            shader.cpp
            tapCamera.cpp
            vecmath.cpp)

target_include_directories(ndk-helper PRIVATE
                           ${ANDROID_NDK}/sources/android/native_app_glue)
