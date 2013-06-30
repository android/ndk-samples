LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE    := NativeActivity
LOCAL_SRC_FILES := TeapotNativeActivity.cpp \
TeapotRenderer.cpp \
NDKSupport/JNIHelper.cpp \
NDKSupport/shader.cpp \
NDKSupport/vecmath.cpp \
NDKSupport/gestureDetector.cpp \
NDKSupport/perfMonitor.cpp \
NDKSupport/tapCamera.cpp \

LOCAL_C_INCLUDES := $(LOCAL_PATH)/NDKSupport

LOCAL_CFLAGS :=

LOCAL_LDLIBS    := -llog -landroid -lEGL -lGLESv2
LOCAL_STATIC_LIBRARIES := cpufeatures android_native_app_glue

include $(BUILD_SHARED_LIBRARY)

$(call import-module,android/native_app_glue)
$(call import-module,android/cpufeatures)