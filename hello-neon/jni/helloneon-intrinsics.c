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
#include "helloneon-intrinsics.h"
#include <arm_neon.h>

/* this source file should only be compiled by Android.mk when targeting
 * the armeabi-v7a ABI, and should be built in NEON mode
 */
void
fir_filter_neon_intrinsics(short *output, const short* input, const short* kernel, int width, int kernelSize)
{
#if 1
   int nn, offset = -kernelSize/2;

   for (nn = 0; nn < width; nn++)
   {
        int mm, sum = 0;
        int32x4_t sum_vec = vdupq_n_s32(0);
        for(mm = 0; mm < kernelSize/4; mm++)
        {
            int16x4_t  kernel_vec = vld1_s16(kernel + mm*4);
            int16x4_t  input_vec = vld1_s16(input + (nn+offset+mm*4));
            sum_vec = vmlal_s16(sum_vec, kernel_vec, input_vec);
        }

        sum += vgetq_lane_s32(sum_vec, 0);
        sum += vgetq_lane_s32(sum_vec, 1);
        sum += vgetq_lane_s32(sum_vec, 2);
        sum += vgetq_lane_s32(sum_vec, 3);

        if(kernelSize & 3)
        {
            for(mm = kernelSize - (kernelSize & 3); mm < kernelSize; mm++)
                sum += kernel[mm] * input[nn+offset+mm];
        }

        output[nn] = (short)((sum + 0x8000) >> 16);
    }
#else /* for comparison purposes only */
    int nn, offset = -kernelSize/2;
    for (nn = 0; nn < width; nn++) {
        int sum = 0;
        int mm;
        for (mm = 0; mm < kernelSize; mm++) {
            sum += kernel[mm]*input[nn+offset+mm];
        }
        output[n] = (short)((sum + 0x8000) >> 16);
    }
#endif
}
