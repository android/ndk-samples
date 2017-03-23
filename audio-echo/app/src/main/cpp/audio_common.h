/*
 * Copyright 2015 The Android Open Source Project
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


#ifndef NATIVE_AUDIO_AUDIO_COMMON_H
#define NATIVE_AUDIO_AUDIO_COMMON_H

#include <SLES/OpenSLES.h>
#include <SLES/OpenSLES_Android.h>

#include "android_debug.h"
#include "debug_utils.h"
#include "buf_manager.h"

/*
 * Audio Sample Controls...
 */
#define AUDIO_SAMPLE_CHANNELS               1

/*
 * Sample Buffer Controls...
 */
#define RECORD_DEVICE_KICKSTART_BUF_COUNT   2
#define PLAY_KICKSTART_BUFFER_COUNT         3
#define DEVICE_SHADOW_BUFFER_QUEUE_LEN      4
#define BUF_COUNT                           16


struct SampleFormat {
    uint32_t   sampleRate_;
    uint32_t   framesPerBuf_;
    uint16_t   channels_;
    uint16_t   pcmFormat_;          //8 bit, 16 bit, 24 bit ...
    uint32_t   representation_;     //android extensions
};
extern void ConvertToSLSampleFormat(SLAndroidDataFormat_PCM_EX *pFormat,
                                    SampleFormat* format);

/*
 * GetSystemTicks(void):  return the time in micro sec
 */
__inline__ uint64_t GetSystemTicks(void) {
    struct timeval Time;
    gettimeofday( &Time, NULL );

    return (static_cast<uint64_t>(1000000) * Time.tv_sec + Time.tv_usec);
}

#define SLASSERT(x)   do {\
    assert(SL_RESULT_SUCCESS == (x));\
    (void) (x);\
    } while (0)

/*
 * Interface for player and recorder to communicate with engine
 */
#define ENGINE_SERVICE_MSG_KICKSTART_PLAYER    1
#define ENGINE_SERVICE_MSG_RETRIEVE_DUMP_BUFS  2
typedef bool (*ENGINE_CALLBACK)(void* pCTX, uint32_t msg, void* pData);

/*
 * flag to enable file dumping
 */
//#define ENABLE_LOG  1

#endif //NATIVE_AUDIO_AUDIO_COMMON_H
