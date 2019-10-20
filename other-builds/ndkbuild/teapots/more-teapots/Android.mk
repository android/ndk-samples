# workaround possible long file path issue on Windows host
NDK_VER=$(firstword $(subst ., ,$(lastword $(file < $(NDK_ROOT)/source.properties))))
ifeq ($(filter $(NDK_VER),21 22 23 24 25),)
  # For NDK 21 and before, need to workaround make's abspath issue
  abspath_wa = $(join $(filter %:,$(subst :,: ,$1)),$(abspath $(filter-out %:,$(subst :,: ,$1))))
else
  abspath_wa = $(abspath $1)
endif


LOCAL_PATH := $(call my-dir)

JNI_SRC_PATH := $(call abspath_wa, $(LOCAL_PATH)/../../../../teapots/more-teapots/src/main/cpp)
NDK_HELPER_SRC := $(call abspath_wa, $(LOCAL_PATH)/../../../../teapots/common/ndk_helper)

include $(CLEAR_VARS)

LOCAL_MODULE    := MoreTeapotsNativeActivity
LOCAL_SRC_FILES := $(JNI_SRC_PATH)/MoreTeapotsNativeActivity.cpp \
                   $(JNI_SRC_PATH)/MoreTeapotsRenderer.cpp \
                   $(NDK_HELPER_SRC)/JNIHelper.cpp    \
                   $(NDK_HELPER_SRC)/interpolator.cpp \
                   $(NDK_HELPER_SRC)/sensorManager.cpp \
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

# Force export ANativeActivity_onCreate(), 
# Refer to: https://github.com/android-ndk/ndk/issues/381.
# Only needed when building with NDK-R14 and older
LOCAL_LDFLAGS   := -u ANativeActivity_onCreate

include $(BUILD_SHARED_LIBRARY)

$(call import-module,android/native_app_glue)
$(call import-module,android/cpufeatures)
