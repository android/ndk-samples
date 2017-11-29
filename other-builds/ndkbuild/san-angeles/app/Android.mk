abspath_wa = $(join $(filter %:,$(subst :,: ,$1)),$(abspath $(filter-out %:,$(subst :,: ,$1))))

LOCAL_PATH := $(call my-dir)
JNI_SRC_PATH := $(call abspath_wa, $(LOCAL_PATH)/../../../../san-angeles/app/src/main/cpp)

include $(CLEAR_VARS)

LOCAL_MODULE := sanangeles

LOCAL_CFLAGS := -DANDROID_NDK \
                -DDISABLE_IMPORTGL

LOCAL_SRC_FILES := \
    $(JNI_SRC_PATH)/importgl.c \
    $(JNI_SRC_PATH)/demo.c \
    $(JNI_SRC_PATH)/app-android.c \

LOCAL_LDLIBS := -lGLESv1_CM -ldl -llog -lm -landroid

include $(BUILD_SHARED_LIBRARY)
