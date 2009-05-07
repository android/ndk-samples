#include "first.h"
#include <jni.h>

jint
Java_com_example_twolibs_TwoLibs_add( JNIEnv*  env,
                                      jobject  this,
                                      jint     x,
                                      jint     y )
{
    return first(x, y);
}
