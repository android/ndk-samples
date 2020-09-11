#include <jni.h>
#include <string>

extern "C" {
JNIEXPORT jstring JNICALL
Java_constantin_prefab_1dependency_MainActivity_stringFromJNI(
        JNIEnv* env,
        jobject /* this */) {
    // Here I want to use the 'mylibrary'  my_api() method for the string
    std::string hello = "Hello from C++";
    return env->NewStringUTF(hello.c_str());
}
}