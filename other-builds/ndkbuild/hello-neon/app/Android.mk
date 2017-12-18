abspath_wa = $(join $(filter %:,$(subst :,: ,$1)),$(abspath $(filter-out %:,$(subst :,: ,$1))))

LOCAL_PATH := $(call my-dir)
JNI_SRC_PATH := $(call abspath_wa, $(LOCAL_PATH)/../../../../hello-neon/app/src/main/cpp)

include $(CLEAR_VARS)

LOCAL_MODULE := hello-neon

LOCAL_SRC_FILES := $(JNI_SRC_PATH)/helloneon.c

ifeq ($(TARGET_ARCH_ABI),$(filter $(TARGET_ARCH_ABI), armeabi-v7a x86))
    LOCAL_CFLAGS := -DHAVE_NEON=1
ifeq ($(TARGET_ARCH_ABI),x86)
    LOCAL_CFLAGS += -mssse3
endif
    LOCAL_SRC_FILES += $(JNI_SRC_PATH)/helloneon-intrinsics.c.neon
endif

LOCAL_STATIC_LIBRARIES := cpufeatures

LOCAL_LDLIBS := -llog

include $(BUILD_SHARED_LIBRARY)

$(call import-add-path, $(NDK_ROOT)/sources/android)
$(call import-module,cpufeatures)
