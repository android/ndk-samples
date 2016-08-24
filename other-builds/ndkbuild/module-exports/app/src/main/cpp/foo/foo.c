/*
 * Copyright (C) 2015 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
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
