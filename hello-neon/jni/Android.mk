LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE := helloneon

LOCAL_SRC_FILES := helloneon.c

ifeq ($(TARGET_ARCH_ABI),armeabi-v7a)
    LOCAL_CFLAGS := -DHAVE_NEON=1
    LOCAL_SRC_FILES += helloneon-intrinsics.c.neon
endif

LOCAL_C_INCLUDES := $(NDK_ROOT)/sources/cpufeatures

LOCAL_STATIC_LIBRARIES := cpufeatures

LOCAL_LDLIBS := -llog

include $(BUILD_SHARED_LIBRARY)

include $(NDK_ROOT)/sources/cpufeatures/Android.mk
