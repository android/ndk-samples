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

#ifndef NATIVE_AUDIO_AUDIO_RECORDER_H
#define NATIVE_AUDIO_AUDIO_RECORDER_H
#include <SLES/OpenSLES.h>
#include <SLES/OpenSLES_Android.h>
#include <sys/types.h>

#include "audio_common.h"
#include "buf_manager.h"
#include "debug_utils.h"

class AudioRecorder {
  SLObjectItf recObjectItf_;
  SLRecordItf recItf_;
  SLAndroidSimpleBufferQueueItf recBufQueueItf_;

  SampleFormat sampleInfo_;
  AudioQueue *freeQueue_;       // user
  AudioQueue *recQueue_;        // user
  AudioQueue *devShadowQueue_;  // owner
  uint32_t audioBufCount;

  ENGINE_CALLBACK callback_;
  void *ctx_;

 public:
  explicit AudioRecorder(SampleFormat *, SLEngineItf engineEngine);
  ~AudioRecorder();
  SLboolean Start(void);
  SLboolean Stop(void);
  void SetBufQueues(AudioQueue *freeQ, AudioQueue *recQ);
  void ProcessSLCallback(SLAndroidSimpleBufferQueueItf bq);
  void RegisterCallback(ENGINE_CALLBACK cb, void *ctx);
  int32_t dbgGetDevBufCount(void);

#ifdef ENABLE_LOG
  AndroidLog *recLog_;
#endif
};

#endif  // NATIVE_AUDIO_AUDIO_RECORDER_H
