abspath_wa = $(join $(filter %:,$(subst :,: ,$1)),$(abspath $(filter-out %:,$(subst :,: ,$1))))

LOCAL_PATH := $(call my-dir)

JNI_SRC_PATH := $(call abspath_wa, $(LOCAL_PATH)/../../../../nn_sample/app/src/main/cpp)

include $(CLEAR_VARS)

LOCAL_MODULE    := nn_sample
LOCAL_SRC_FILES := $(JNI_SRC_PATH)/nn_sample.cpp \
                   $(JNI_SRC_PATH)/simple_model.cpp


LOCAL_C_INCLUDES := $(JNI_SRC_PATH)
LOCAL_CPPFLAGS += -std=c++14

LOCAL_LDLIBS    := -lneuralnetworks -llog -landroid
#LOCAL_STATIC_LIBRARIES := cpufeature

# Force export ANativeActivity_onCreate(), 
# Refer to: https://github.com/android-ndk/ndk/issues/381.
# Only needed when building with NDK-R14 and older
LOCAL_LDFLAGS   := -u ANativeActivity_onCreate

include $(BUILD_SHARED_LIBRARY)

# $(call import-module,android/native_app_glue)
# $(call import-module,android/cpufeatures)
