#include <jni.h>

#include <stdexcept>

#include "exception_helper.h"

void might_throw() { throw std::runtime_error("A C++ runtime_error"); }

extern "C" JNIEXPORT void JNICALL
Java_com_example_exceptions_MainActivity_throwsException(JNIEnv* env,
                                                         jobject /* this */) {
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