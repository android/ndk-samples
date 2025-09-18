#include <base/macros.h>
#include <jni.h>

#include <stdexcept>

#include "exception_helper.h"

void might_throw() { throw std::runtime_error("A C++ runtime_error"); }

void ThrowsException(JNIEnv* env, jobject /* this */) {
  try {
    might_throw();
  } catch (std::exception& e) {
    jniThrowRuntimeException(env, e.what());
  } catch (...) {
    // We don't want any C++ exceptions to cross the JNI boundary, so include a
    // catch-all.
    jniThrowRuntimeException(env, "Catch-all");
  }
}

extern "C" JNIEXPORT jint JNI_OnLoad(JavaVM* _Nonnull vm, void* _Nullable) {
  JNIEnv* env;
  if (vm->GetEnv(reinterpret_cast<void**>(&env), JNI_VERSION_1_6) != JNI_OK) {
    return JNI_ERR;
  }

  jclass c = env->FindClass("com/example/exceptions/MainActivity");
  if (c == nullptr) return JNI_ERR;

  static const JNINativeMethod methods[] = {
      {"throwsException", "()V", reinterpret_cast<void*>(ThrowsException)},
  };
  int rc = env->RegisterNatives(c, methods, arraysize(methods));
  if (rc != JNI_OK) return rc;

  return JNI_VERSION_1_6;
}
