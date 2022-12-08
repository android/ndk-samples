#include <jni.h>

#include "adder.h"

extern "C" JNIEXPORT jint JNICALL Java_com_example_unittest_MainActivity_add(
    JNIEnv* env, jobject /* this */, jint a, jint b) {
  return add((int)a, (int)b);
}