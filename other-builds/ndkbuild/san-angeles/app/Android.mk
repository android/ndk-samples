# workaround possible long file path issue on Windows host
NDK_VER=$(firstword $(subst ., ,$(lastword $(file < $(NDK_ROOT)/source.properties))))
ifeq ($(filter $(NDK_VER),21 22 23 24 25),)
  # For NDK 21 and before, need to workaround make's abspath issue
  abspath_wa = $(join $(filter %:,$(subst :,: ,$1)),$(abspath $(filter-out %:,$(subst :,: ,$1))))
else
  abspath_wa = $(abspath $1)
endif


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
