#include <jni.h>

#include <string>

extern "C" JNIEXPORT jstring JNICALL
Java_com_example_sanitizers_MainActivity_stringFromJNI(JNIEnv* env,
                                                       jobject /* this */) {
  int* foo = new int;
  *foo = 3;
  delete foo;
  *foo = 4;
  std::string hello = "Hello from C++";
  return env->NewStringUTF(hello.c_str());
}