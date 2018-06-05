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
#include "jni_interface.h"
#include "audio_recorder.h"
#include "audio_player.h"
#include "audio_effect.h"
#include "audio_common.h"
#include <jni.h>
#include <SLES/OpenSLES_Android.h>
#include <sys/types.h>
#include <cassert>
#include <cstring>

struct EchoAudioEngine {
  SLmilliHertz fastPathSampleRate_;
  uint32_t fastPathFramesPerBuf_;
  uint16_t sampleChannels_;
  uint16_t bitsPerSample_;

  SLObjectItf slEngineObj_;
  SLEngineItf slEngineItf_;

  AudioRecorder *recorder_;
  AudioPlayer *player_;
  AudioQueue *freeBufQueue_;  // Owner of the queue
  AudioQueue *recBufQueue_;   // Owner of the queue

  sample_buf *bufs_;
  uint32_t bufCount_;
  uint32_t frameCount_;
  int64_t echoDelay_;
  float echoDecay_;
  AudioDelay *delayEffect_;
};
static EchoAudioEngine engine;

bool EngineService(void *ctx, uint32_t msg, void *data);

JNIEXPORT void JNICALL Java_com_google_sample_echo_MainActivity_createSLEngine(
    JNIEnv *env, jclass type, jint sampleRate, jint framesPerBuf,
    jlong delayInMs, jfloat decay) {
  SLresult result;
  memset(&engine, 0, sizeof(engine));

  engine.fastPathSampleRate_ = static_cast<SLmilliHertz>(sampleRate) * 1000;
  engine.fastPathFramesPerBuf_ = static_cast<uint32_t>(framesPerBuf);
  engine.sampleChannels_ = AUDIO_SAMPLE_CHANNELS;
  engine.bitsPerSample_ = SL_PCMSAMPLEFORMAT_FIXED_16;

  result = slCreateEngine(&engine.slEngineObj_, 0, NULL, 0, NULL, NULL);
  SLASSERT(result);

  result =
      (*engine.slEngineObj_)->Realize(engine.slEngineObj_, SL_BOOLEAN_FALSE);
  SLASSERT(result);

  result = (*engine.slEngineObj_)
               ->GetInterface(engine.slEngineObj_, SL_IID_ENGINE,
                              &engine.slEngineItf_);
  SLASSERT(result);

  // compute the RECOMMENDED fast audio buffer size:
  //   the lower latency required
  //     *) the smaller the buffer should be (adjust it here) AND
  //     *) the less buffering should be before starting player AFTER
  //        receiving the recorder buffer
  //   Adjust the bufSize here to fit your bill [before it busts]
  uint32_t bufSize = engine.fastPathFramesPerBuf_ * engine.sampleChannels_ *
                     engine.bitsPerSample_;
  bufSize = (bufSize + 7) >> 3;  // bits --> byte
  engine.bufCount_ = BUF_COUNT;
  engine.bufs_ = allocateSampleBufs(engine.bufCount_, bufSize);
  assert(engine.bufs_);

  engine.freeBufQueue_ = new AudioQueue(engine.bufCount_);
  engine.recBufQueue_ = new AudioQueue(engine.bufCount_);
  assert(engine.freeBufQueue_ && engine.recBufQueue_);
  for (uint32_t i = 0; i < engine.bufCount_; i++) {
    engine.freeBufQueue_->push(&engine.bufs_[i]);
  }

  engine.echoDelay_ = delayInMs;
  engine.echoDecay_ = decay;
  engine.delayEffect_ = new AudioDelay(
      engine.fastPathSampleRate_, engine.sampleChannels_, engine.bitsPerSample_,
      engine.echoDelay_, engine.echoDecay_);
  assert(engine.delayEffect_);
}

JNIEXPORT jboolean JNICALL
Java_com_google_sample_echo_MainActivity_configureEcho(JNIEnv *env, jclass type,
                                                       jint delayInMs,
                                                       jfloat decay) {
  engine.echoDelay_ = delayInMs;
  engine.echoDecay_ = decay;

  engine.delayEffect_->setDelayTime(delayInMs);
  engine.delayEffect_->setDecayWeight(decay);
  return JNI_FALSE;
}

JNIEXPORT jboolean JNICALL
Java_com_google_sample_echo_MainActivity_createSLBufferQueueAudioPlayer(
    JNIEnv *env, jclass type) {
  SampleFormat sampleFormat;
  memset(&sampleFormat, 0, sizeof(sampleFormat));
  sampleFormat.pcmFormat_ = (uint16_t)engine.bitsPerSample_;
  sampleFormat.framesPerBuf_ = engine.fastPathFramesPerBuf_;

  // SampleFormat.representation_ = SL_ANDROID_PCM_REPRESENTATION_SIGNED_INT;
  sampleFormat.channels_ = (uint16_t)engine.sampleChannels_;
  sampleFormat.sampleRate_ = engine.fastPathSampleRate_;

  engine.player_ = new AudioPlayer(&sampleFormat, engine.slEngineItf_);
  assert(engine.player_);
  if (engine.player_ == nullptr) return JNI_FALSE;

  engine.player_->SetBufQueue(engine.recBufQueue_, engine.freeBufQueue_);
  engine.player_->RegisterCallback(EngineService, (void *)&engine);

  return JNI_TRUE;
}

JNIEXPORT void JNICALL
Java_com_google_sample_echo_MainActivity_deleteSLBufferQueueAudioPlayer(
    JNIEnv *env, jclass type) {
  if (engine.player_) {
    delete engine.player_;
    engine.player_ = nullptr;
  }
}

JNIEXPORT jboolean JNICALL
Java_com_google_sample_echo_MainActivity_createAudioRecorder(JNIEnv *env,
                                                             jclass type) {
  SampleFormat sampleFormat;
  memset(&sampleFormat, 0, sizeof(sampleFormat));
  sampleFormat.pcmFormat_ = static_cast<uint16_t>(engine.bitsPerSample_);

  // SampleFormat.representation_ = SL_ANDROID_PCM_REPRESENTATION_SIGNED_INT;
  sampleFormat.channels_ = engine.sampleChannels_;
  sampleFormat.sampleRate_ = engine.fastPathSampleRate_;
  sampleFormat.framesPerBuf_ = engine.fastPathFramesPerBuf_;
  engine.recorder_ = new AudioRecorder(&sampleFormat, engine.slEngineItf_);
  if (!engine.recorder_) {
    return JNI_FALSE;
  }
  engine.recorder_->SetBufQueues(engine.freeBufQueue_, engine.recBufQueue_);
  engine.recorder_->RegisterCallback(EngineService, (void *)&engine);
  return JNI_TRUE;
}

JNIEXPORT void JNICALL
Java_com_google_sample_echo_MainActivity_deleteAudioRecorder(JNIEnv *env,
                                                             jclass type) {
  if (engine.recorder_) delete engine.recorder_;

  engine.recorder_ = nullptr;
}

JNIEXPORT void JNICALL
Java_com_google_sample_echo_MainActivity_startPlay(JNIEnv *env, jclass type) {
  engine.frameCount_ = 0;
  /*
   * start player: make it into waitForData state
   */
  if (SL_BOOLEAN_FALSE == engine.player_->Start()) {
    LOGE("====%s failed", __FUNCTION__);
    return;
  }
  engine.recorder_->Start();
}

JNIEXPORT void JNICALL
Java_com_google_sample_echo_MainActivity_stopPlay(JNIEnv *env, jclass type) {
  engine.recorder_->Stop();
  engine.player_->Stop();

  delete engine.recorder_;
  delete engine.player_;
  engine.recorder_ = NULL;
  engine.player_ = NULL;
}

JNIEXPORT void JNICALL Java_com_google_sample_echo_MainActivity_deleteSLEngine(
    JNIEnv *env, jclass type) {
  delete engine.recBufQueue_;
  delete engine.freeBufQueue_;
  releaseSampleBufs(engine.bufs_, engine.bufCount_);
  if (engine.slEngineObj_ != NULL) {
    (*engine.slEngineObj_)->Destroy(engine.slEngineObj_);
    engine.slEngineObj_ = NULL;
    engine.slEngineItf_ = NULL;
  }

  if (engine.delayEffect_) {
    delete engine.delayEffect_;
    engine.delayEffect_ = nullptr;
  }
}

uint32_t dbgEngineGetBufCount(void) {
  uint32_t count = engine.player_->dbgGetDevBufCount();
  count += engine.recorder_->dbgGetDevBufCount();
  count += engine.freeBufQueue_->size();
  count += engine.recBufQueue_->size();

  LOGE(
      "Buf Disrtibutions: PlayerDev=%d, RecDev=%d, FreeQ=%d, "
      "RecQ=%d",
      engine.player_->dbgGetDevBufCount(),
      engine.recorder_->dbgGetDevBufCount(), engine.freeBufQueue_->size(),
      engine.recBufQueue_->size());
  if (count != engine.bufCount_) {
    LOGE("====Lost Bufs among the queue(supposed = %d, found = %d)", BUF_COUNT,
         count);
  }
  return count;
}

/*
 * simple message passing for player/recorder to communicate with engine
 */
bool EngineService(void *ctx, uint32_t msg, void *data) {
  assert(ctx == &engine);
  switch (msg) {
    case ENGINE_SERVICE_MSG_RETRIEVE_DUMP_BUFS: {
      *(static_cast<uint32_t *>(data)) = dbgEngineGetBufCount();
      break;
    }
    case ENGINE_SERVICE_MSG_RECORDED_AUDIO_AVAILABLE: {
      // adding audio delay effect
      sample_buf *buf = static_cast<sample_buf *>(data);
      assert(engine.fastPathFramesPerBuf_ ==
             buf->size_ / engine.sampleChannels_ / (engine.bitsPerSample_ / 8));
      engine.delayEffect_->process(reinterpret_cast<int16_t *>(buf->buf_),
                                   engine.fastPathFramesPerBuf_);
      break;
    }
    default:
      assert(false);
      return false;
  }

  return true;
}
