LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE    := MoreTeapotsNativeActivity
LOCAL_SRC_FILES := MoreTeapotsNativeActivity.cpp \
MoreTeapotsRenderer.cpp \

LOCAL_C_INCLUDES :=
LOCAL_CFLAGS :=

LOCAL_LDLIBS    := -llog -landroid -lEGL -lGLESv2
LOCAL_STATIC_LIBRARIES := cpufeatures android_native_app_glue ndk_helper

include $(BUILD_SHARED_LIBRARY)

$(call import-module,android/ndk_helper)
$(call import-module,android/native_app_glue)
$(call import-module,android/cpufeatures)
