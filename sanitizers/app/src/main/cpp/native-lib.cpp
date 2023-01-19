#include <jni.h>

#include <string>

extern "C" JNIEXPORT jstring JNICALL
Java_com_example_sanitizers_MainActivity_stringFromJNI(JNIEnv* env,
                                                       jobject /* this */) {
  // Use-after-free error, caught by asan and hwasan.
  int* foo = new int;
  *foo = 3;
  delete foo;
  *foo = 4;

  // Signed integer overflow. Undefined behavior caught by ubsan.
  int k = 0x7fffffff;
  k += 1;

  std::string hello = "Hello from C++";
  return env->NewStringUTF(hello.c_str());
}