#include <jni.h>

#include <stdexcept>

#include "exception_helper.h"

extern "C" JNIEXPORT void JNICALL
Java_com_example_exceptions_MainActivity_throwsException(JNIEnv* env,
                                                         jobject /* this */) {
  try {
    throw std::runtime_error("A C++ runtime_error");
  } catch (std::runtime_error e) {
    jniThrowRuntimeException(env, e.what());
  }
}