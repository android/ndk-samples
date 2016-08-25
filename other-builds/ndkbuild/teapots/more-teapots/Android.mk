LOCAL_PATH := $(call my-dir)

JNI_SRC_PATH := $(LOCAL_PATH)/../../../../teapots/more-teapots/src/main/cpp
NDK_HELPER_SRC := $(LOCAL_PATH)/../../../../teapots/common/ndk_helper

include $(CLEAR_VARS)

LOCAL_MODULE    := MoreTeapotsNativeActivity
LOCAL_SRC_FILES := $(JNI_SRC_PATH)/MoreTeapotsNativeActivity.cpp \
                   $(JNI_SRC_PATH)/MoreTeapotsRenderer.cpp \
                   $(NDK_HELPER_SRC)/JNIHelper.cpp    \
                   $(NDK_HELPER_SRC)/interpolator.cpp \
                   $(NDK_HELPER_SRC)/tapCamera.cpp    \
                   $(NDK_HELPER_SRC)/gestureDetector.cpp \
                   $(NDK_HELPER_SRC)/perfMonitor.cpp \
                   $(NDK_HELPER_SRC)/vecmath.cpp   \
                   $(NDK_HELPER_SRC)/GLContext.cpp \
                   $(NDK_HELPER_SRC)/shader.cpp \
                   $(NDK_HELPER_SRC)/gl3stub.c

LOCAL_C_INCLUDES := $(JNI_SRC_PATH) $(NDK_HELPER_SRC)
LOCAL_CPPFLAGS += -std=c++11

LOCAL_LDLIBS    := -llog -landroid -lEGL -lGLESv2 -latomic
LOCAL_STATIC_LIBRARIES := cpufeatures android_native_app_glue

include $(BUILD_SHARED_LIBRARY)

$(call import-module,android/native_app_glue)
$(call import-module,android/cpufeatures)
