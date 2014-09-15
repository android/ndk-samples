/*
 * Copyright (C) 2010 The Android Open Source Project
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
 *
 */
#include <jni.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <cpu-features.h>
#include "helloneon-intrinsics.h"

#define DEBUG 0

#if DEBUG
#include <android/log.h>
#  define  D(x...)  __android_log_print(ANDROID_LOG_INFO,"helloneon",x)
#else
#  define  D(...)  do {} while (0)
#endif

/* return current time in milliseconds */
static double
now_ms(void)
{
    struct timespec res;
    clock_gettime(CLOCK_REALTIME, &res);
    return 1000.0*res.tv_sec + (double)res.tv_nsec/1e6;
}


/* this is a FIR filter implemented in C */
static void
fir_filter_c(short *output, const short* input, const short* kernel, int width, int kernelSize)
{
    int  offset = -kernelSize/2;
    int  nn;
    for (nn = 0; nn < width; nn++) {
        int sum = 0;
        int mm;
        for (mm = 0; mm < kernelSize; mm++) {
            sum += kernel[mm]*input[nn+offset+mm];
        }
        output[nn] = (short)((sum + 0x8000) >> 16);
    }
}

#define  FIR_KERNEL_SIZE   32
#define  FIR_OUTPUT_SIZE   2560
#define  FIR_INPUT_SIZE    (FIR_OUTPUT_SIZE + FIR_KERNEL_SIZE)
#define  FIR_ITERATIONS    600

static const short  fir_kernel[FIR_KERNEL_SIZE] = { 
    0x10, 0x20, 0x40, 0x70, 0x8c, 0xa2, 0xce, 0xf0, 0xe9, 0xce, 0xa2, 0x8c, 070, 0x40, 0x20, 0x10,
    0x10, 0x20, 0x40, 0x70, 0x8c, 0xa2, 0xce, 0xf0, 0xe9, 0xce, 0xa2, 0x8c, 070, 0x40, 0x20, 0x10 };

static short        fir_output[FIR_OUTPUT_SIZE];
static short        fir_input_0[FIR_INPUT_SIZE];
static const short* fir_input = fir_input_0 + (FIR_KERNEL_SIZE/2);
static short        fir_output_expected[FIR_OUTPUT_SIZE];

/* This is a trivial JNI example where we use a native method
 * to return a new VM String. See the corresponding Java source
 * file located at:
 *
 *   apps/samples/hello-neon/project/src/com/example/neon/HelloNeon.java
 */
jstring
Java_com_example_neon_HelloNeon_stringFromJNI( JNIEnv* env,
                                               jobject thiz )
{
    char*  str;
    AndroidCpuFamily family;
    uint64_t features;
    char buffer[512];
    char tryNeon = 0;
    double  t0, t1, time_c, time_neon;

    /* setup FIR input - whatever */
    {
        int  nn;
        for (nn = 0; nn < FIR_INPUT_SIZE; nn++) {
            fir_input_0[nn] = (5*nn) & 255;
        }
        fir_filter_c(fir_output_expected, fir_input, fir_kernel, FIR_OUTPUT_SIZE, FIR_KERNEL_SIZE);
    }

    /* Benchmark small FIR filter loop - C version */
    t0 = now_ms();
    {
        int  count = FIR_ITERATIONS;
        for (; count > 0; count--) {
            fir_filter_c(fir_output, fir_input, fir_kernel, FIR_OUTPUT_SIZE, FIR_KERNEL_SIZE);
        }
    }
    t1 = now_ms();
    time_c = t1 - t0;

    asprintf(&str, "FIR Filter benchmark:\nC version          : %g ms\n", time_c);
    strlcpy(buffer, str, sizeof buffer);
    free(str);

    strlcat(buffer, "Neon version   : ", sizeof buffer);

    family = android_getCpuFamily();
    if ((family != ANDROID_CPU_FAMILY_ARM) &&
        (family != ANDROID_CPU_FAMILY_X86))
    {
        strlcat(buffer, "Not an ARM and not an X86 CPU !\n", sizeof buffer);
        goto EXIT;
    }

    features = android_getCpuFeatures();
    if (((features & ANDROID_CPU_ARM_FEATURE_ARMv7) == 0) &&
        ((features & ANDROID_CPU_X86_FEATURE_SSSE3) == 0))
    {
        strlcat(buffer, "Not an ARMv7 and not an X86 SSSE3 CPU !\n", sizeof buffer);
        goto EXIT;
    }

    /* HAVE_NEON is defined in Android.mk ! */
#ifdef HAVE_NEON
    if (((features & ANDROID_CPU_ARM_FEATURE_NEON) == 0) &&
        ((features & ANDROID_CPU_X86_FEATURE_SSSE3) == 0))
    {
        strlcat(buffer, "CPU doesn't support NEON !\n", sizeof buffer);
        goto EXIT;
    }

    /* Benchmark small FIR filter loop - Neon version */
    t0 = now_ms();
    {
        int  count = FIR_ITERATIONS;
        for (; count > 0; count--) {
            fir_filter_neon_intrinsics(fir_output, fir_input, fir_kernel, FIR_OUTPUT_SIZE, FIR_KERNEL_SIZE);
        }
    }
    t1 = now_ms();
    time_neon = t1 - t0;
    asprintf(&str, "%g ms (x%g faster)\n", time_neon, time_c / (time_neon < 1e-6 ? 1. : time_neon));
    strlcat(buffer, str, sizeof buffer);
    free(str);

    /* check the result, just in case */
    {
        int  nn, fails = 0;
        for (nn = 0; nn < FIR_OUTPUT_SIZE; nn++) {
            if (fir_output[nn] != fir_output_expected[nn]) {
                if (++fails < 16)
                    D("neon[%d] = %d expected %d", nn, fir_output[nn], fir_output_expected[nn]);
            }
        }
        D("%d fails\n", fails);
    }
#else /* !HAVE_NEON */
    strlcat(buffer, "Program not compiled with ARMv7 support !\n", sizeof buffer);
#endif /* !HAVE_NEON */
EXIT:
    return (*env)->NewStringUTF(env, buffer);
}
