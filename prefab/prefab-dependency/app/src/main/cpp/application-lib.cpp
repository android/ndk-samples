#include <jni.h>
#include <string>
#include "mylibrary/mylibrary.h"

extern "C" {
JNIEXPORT jstring JNICALL
Java_constantin_prefab_1dependency_MainActivity_stringFromJNI(
        JNIEnv* env,
        jobject /* this */) {
    // Here I use the 'mylibrary'  my_api() method for the string,
    // which comes from the prefab-publishing example
    //std::string hello = "Hello from C++";
    const std::string stringFromMyLibrary=my_api();
    return env->NewStringUTF(stringFromMyLibrary.c_str());
}
}