# workaround possible long file path issue on Windows host
NDK_VER=$(firstword $(subst ., ,$(lastword $(file < $(NDK_ROOT)/source.properties))))
ifeq ($(filter $(NDK_VER),21 22 23 24 25),)
  # For NDK 21 and before, need to workaround make's abspath issue
  abspath_wa = $(join $(filter %:,$(subst :,: ,$1)),$(abspath $(filter-out %:,$(subst :,: ,$1))))
else
  abspath_wa = $(abspath $1)
endif


LOCAL_PATH := $(call my-dir)
JNI_SRC_PATH := $(call abspath_wa, $(LOCAL_PATH)/../../../../hello-neon/app/src/main/cpp)

include $(CLEAR_VARS)

LOCAL_MODULE := hello-neon

LOCAL_SRC_FILES := $(JNI_SRC_PATH)/helloneon.c

ifeq ($(TARGET_ARCH_ABI),$(filter $(TARGET_ARCH_ABI), armeabi-v7a x86))
    LOCAL_CFLAGS := -DHAVE_NEON=1
ifeq ($(TARGET_ARCH_ABI),x86)
    LOCAL_CFLAGS += -DHAVE_NEON_X86=1 -mssse3
endif
    LOCAL_SRC_FILES += $(JNI_SRC_PATH)/helloneon-intrinsics.c.neon
endif

LOCAL_STATIC_LIBRARIES := cpufeatures

LOCAL_LDLIBS := -llog

include $(BUILD_SHARED_LIBRARY)

$(call import-add-path, $(NDK_ROOT)/sources/android)
$(call import-module,cpufeatures)
