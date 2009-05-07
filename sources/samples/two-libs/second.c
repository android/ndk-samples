#include "first.h"
#include <jni.h>

jint
Java_com_example_TwoLib_TwoLib_add( JNIEnv*  env,
                                    jobject  this,
                                    jint     x,
                                    jint     y )
{
    return first(x, y);
}
