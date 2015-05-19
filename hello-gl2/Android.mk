LOCAL_PATH:= $(LOCAL_PATH)/jni

include $(CLEAR_VARS)

LOCAL_MODULE    := libgl2jni
LOCAL_CFLAGS    := -Werror
LOCAL_SRC_FILES := gl_code.cpp
LOCAL_LDLIBS    := -llog -lGLESv2

include $(BUILD_SHARED_LIBRARY)
