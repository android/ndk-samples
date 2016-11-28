LOCAL_PATH:= $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE:= ndk_helper
LOCAL_SRC_FILES:= JNIHelper.cpp interpolator.cpp tapCamera.cpp gestureDetector.cpp perfMonitor.cpp vecmath.cpp GLContext.cpp shader.cpp gl3stub.c

LOCAL_CPPFLAGS +=-std=c++11
LOCAL_EXPORT_C_INCLUDES := $(LOCAL_PATH)
LOCAL_EXPORT_LDLIBS    := -llog -landroid -lEGL -lGLESv2

LOCAL_STATIC_LIBRARIES := cpufeatures android_native_app_glue

include $(BUILD_STATIC_LIBRARY)

#$(call import-module,android/native_app_glue)
#$(call import-module,android/cpufeatures)
