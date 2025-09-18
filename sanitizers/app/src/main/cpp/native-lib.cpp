#include <base/macros.h>
#include <jni.h>

#include <string>

jstring StringFromJni(JNIEnv* env, jobject /* this */) {
  // Use-after-free error, caught by asan and hwasan.
  int* foo = new int;
  *foo = 3;
  delete foo;
  *foo = 4;

  // Signed integer overflow. Undefined behavior caught by ubsan.
  [[maybe_unused]] int k = 0x7fffffff;
  k += 1;

  std::string hello = "Hello from C++";
  return env->NewStringUTF(hello.c_str());
}

extern "C" JNIEXPORT jint JNICALL JNI_OnLoad(JavaVM* _Nonnull vm,
                                             void* _Nullable) {
  JNIEnv* env;
  if (vm->GetEnv(reinterpret_cast<void**>(&env), JNI_VERSION_1_6) != JNI_OK) {
    return JNI_ERR;
  }

  jclass c = env->FindClass("com/example/sanitizers/MainActivity");
  if (c == nullptr) return JNI_ERR;

  static const JNINativeMethod methods[] = {
      {"stringFromJNI", "()Ljava/lang/String;",
       reinterpret_cast<void*>(StringFromJni)},
  };
  int rc = env->RegisterNatives(c, methods, arraysize(methods));
  if (rc != JNI_OK) return rc;

  return JNI_VERSION_1_6;
}
