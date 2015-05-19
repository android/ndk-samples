#include "foo.h"
#include <android/log.h>

/* FOO should be defined to '2' when building foo.c */
#ifndef FOO
#error FOO is not defined here !
#endif

#if FOO != 2
#error FOO is incorrectly defined here !
#endif

#define  LOG_TAG    "libfoo"
#define  LOGI(...)  __android_log_print(ANDROID_LOG_INFO,LOG_TAG,__VA_ARGS__)

int  foo(int  x)
{
    LOGI("foo(%d) called !", x);
    return x+1;
}
