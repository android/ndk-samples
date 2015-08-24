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
#include <cstdlib>
#include "audio_player.h"

/*
 * Called by OpenSL SimpleBufferQueue for every audio buffer played
 * directly pass thru to our handler.
 * The regularity of this callback from openSL/Android System affects
 * playback continuity. If it does not callback in the regular time
 * slot, you are under big pressure for audio processing[here we do
 * not do any filtering/mixing]. Callback from fast audio path are
 * much more regular than other audio paths by my observation. If it
 * very regular, you could buffer much less audio samples between
 * recorder and player, hence lower latency.
 */
void bqPlayerCallback(SLAndroidSimpleBufferQueueItf bq, void *ctx) {
    (static_cast<AudioPlayer *>(ctx))->ProcessSLCallback(bq);
}
void AudioPlayer::ProcessSLCallback(SLAndroidSimpleBufferQueueItf bq) {
#ifdef ENABLE_LOG
    logFile_->logTime();
#endif

    // retrieve the finished device buf and put onto the free queue
    // so recorder could re-use it
    sample_buf *buf;
    if(!devShadowQueue_->front(&buf)) {
        /*
         * This should not happen: we got a callback,
         * but we have no buffer in deviceShadowedQueue
         * we lost buffers this way...(ERROR)
         */
        if(callback_) {
            uint32_t count;
            callback_(ctx_, ENGINE_SERVICE_MSG_RETRIEVE_DUMP_BUFS, &count);
        }
        return;
    }
    devShadowQueue_->pop();
    buf->size_ = 0;
    freeQueue_->push(buf);
    while(playQueue_->front(&buf) && devShadowQueue_->push(buf)) {
        (*bq)->Enqueue(bq, buf->buf_, buf->size_);
        playQueue_->pop();
    }
}

AudioPlayer::AudioPlayer(SampleFormat *sampleFormat, SLEngineItf slEngine) :
    playQueue_(nullptr),freeQueue_(nullptr), devShadowQueue_(nullptr),
    callback_(nullptr)
{
    SLresult result;
    assert(sampleFormat);
    sampleInfo_ = *sampleFormat;

    result = (*slEngine)->CreateOutputMix(slEngine, &outputMixObjectItf_,
                                          0, NULL, NULL);
    SLASSERT(result);

    // realize the output mix
    result = (*outputMixObjectItf_)->Realize(outputMixObjectItf_, SL_BOOLEAN_FALSE);
    SLASSERT(result);

    // configure audio source
    SLDataLocator_AndroidSimpleBufferQueue loc_bufq = {
            SL_DATALOCATOR_ANDROIDSIMPLEBUFFERQUEUE,
            DEVICE_SHADOW_BUFFER_QUEUE_LEN };

    SLAndroidDataFormat_PCM_EX format_pcm;
    ConvertToSLSampleFormat(&format_pcm, &sampleInfo_);
    SLDataSource audioSrc = {&loc_bufq, &format_pcm};

    // configure audio sink
    SLDataLocator_OutputMix loc_outmix = {SL_DATALOCATOR_OUTPUTMIX, outputMixObjectItf_};
    SLDataSink audioSnk = {&loc_outmix, NULL};
    /*
     * create fast path audio player: SL_IID_BUFFERQUEUE and SL_IID_VOLUME interfaces ok,
     * NO others!
     */
    SLInterfaceID  ids[2] = { SL_IID_BUFFERQUEUE, SL_IID_VOLUME};
    SLboolean      req[2] = {SL_BOOLEAN_TRUE, SL_BOOLEAN_TRUE};
    result = (*slEngine)->CreateAudioPlayer(slEngine, &playerObjectItf_, &audioSrc, &audioSnk,
                                            sizeof(ids)/sizeof(ids[0]), ids, req);
    SLASSERT(result);

    // realize the player
    result = (*playerObjectItf_)->Realize(playerObjectItf_, SL_BOOLEAN_FALSE);
    SLASSERT(result);

    // get the play interface
    result = (*playerObjectItf_)->GetInterface(playerObjectItf_, SL_IID_PLAY, &playItf_);
    SLASSERT(result);

    // get the buffer queue interface
    result = (*playerObjectItf_)->GetInterface(playerObjectItf_, SL_IID_BUFFERQUEUE,
                                             &playBufferQueueItf_);
    SLASSERT(result);

    // register callback on the buffer queue
    result = (*playBufferQueueItf_)->RegisterCallback(playBufferQueueItf_, bqPlayerCallback, this);
    SLASSERT(result);

    result = (*playItf_)->SetPlayState(playItf_, SL_PLAYSTATE_STOPPED);
    SLASSERT(result);

    // create an empty queue to track deviceQueue
    devShadowQueue_ = new AudioQueue(DEVICE_SHADOW_BUFFER_QUEUE_LEN);
    assert(devShadowQueue_);

#ifdef  ENABLE_LOG
    std::string name = "play";
    logFile_ = new AndroidLog(name);
#endif
}

AudioPlayer::~AudioPlayer() {

    // destroy buffer queue audio player object, and invalidate all associated interfaces
    if (playerObjectItf_ != NULL) {
        (*playerObjectItf_)->Destroy(playerObjectItf_);
    }
    if(devShadowQueue_) {
        delete devShadowQueue_;
    }

    // destroy output mix object, and invalidate all associated interfaces
    if (outputMixObjectItf_) {
        (*outputMixObjectItf_)->Destroy(outputMixObjectItf_);
    }
}

void AudioPlayer::SetBufQueue(AudioQueue *playQ, AudioQueue *freeQ) {
    playQueue_ = playQ;
    freeQueue_ = freeQ;
}

SLresult AudioPlayer::Start(void) {
    SLuint32   state;
    SLresult  result = (*playItf_)->GetPlayState(playItf_, &state);
    if (result != SL_RESULT_SUCCESS) {
        return SL_BOOLEAN_FALSE;
    }
    if(state == SL_PLAYSTATE_PLAYING) {
        return SL_BOOLEAN_TRUE;
    }

    result = (*playItf_)->SetPlayState(playItf_, SL_PLAYSTATE_STOPPED);
    SLASSERT(result);

    result = (*playItf_)->SetPlayState(playItf_, SL_PLAYSTATE_PLAYING);
    SLASSERT(result);

    // send pre-defined audio buffers to device
    int i = PLAY_KICKSTART_BUFFER_COUNT;
    while(i--) {
        sample_buf *buf;
        if(!playQueue_->front(&buf))    //we have buffers for sure
            break;
        if(SL_RESULT_SUCCESS !=
           (*playBufferQueueItf_)->Enqueue(playBufferQueueItf_, buf, buf->size_))
        {
            LOGE("====failed to enqueue (%d) in %s", i, __FUNCTION__);
            return SL_BOOLEAN_FALSE;
        } else {
            playQueue_->pop();
            devShadowQueue_->push(buf);
        }
    }
    return SL_BOOLEAN_TRUE;
}

void AudioPlayer::Stop(void) {
    SLuint32   state;

    SLresult   result = (*playItf_)->GetPlayState(playItf_, &state);
    SLASSERT(result);

    if(state == SL_PLAYSTATE_STOPPED)
        return;

    result = (*playItf_)->SetPlayState(playItf_, SL_PLAYSTATE_STOPPED);
    SLASSERT(result);

    // Consume all non-completed audio buffers
    sample_buf *buf = NULL;
    while(devShadowQueue_->front(&buf)) {
        buf->size_ = 0;
        devShadowQueue_->pop();
        freeQueue_->push(buf);
    }
    while(playQueue_->front(&buf)) {
        buf->size_ = 0;
        playQueue_->pop();
        freeQueue_->push(buf);
    }

#ifdef ENABLE_LOG
    if (logFile_) {
        delete logFile_;
        logFile_ = nullptr;
    }
#endif
}

void AudioPlayer::PlayAudioBuffers(int32_t count) {
    if(!count) {
        return;
    }

    while(count--) {
        sample_buf *buf = NULL;
        if(!playQueue_->front(&buf)) {
            uint32_t totalBufCount;
            callback_(ctx_, ENGINE_SERVICE_MSG_RETRIEVE_DUMP_BUFS,
                      &totalBufCount);
            LOGE("====Run out of buffers in %s @(count = %d), totalBuf =%d",
                 __FUNCTION__, count, totalBufCount);
            break;
        }
        if(!devShadowQueue_->push(buf)) {
            break;  // PlayerBufferQueue is full!!!
        }

        SLresult result = (*playBufferQueueItf_)->Enqueue(playBufferQueueItf_,
                                                  buf->buf_, buf->size_);
        if(result != SL_RESULT_SUCCESS) {
            if(callback_) {
                uint32_t totalBufCount;
                callback_(ctx_, ENGINE_SERVICE_MSG_RETRIEVE_DUMP_BUFS,
                          &totalBufCount);
            }
            LOGE("%s Error @( %p, %d ), result = %d", __FUNCTION__,
                 (void*)buf->buf_, buf->size_, result);
            /*
             * when this happens, a buffer is lost. Need to remove the buffer
             * from top of the devShadowQueue. Since I do not have it now,
             * just pop out the one that is being played right now. Afer a
             * cycle it will be normal.
             */
            devShadowQueue_->front(&buf), devShadowQueue_->pop();
            freeQueue_->push(buf);
            break;
        }
        playQueue_->pop();   // really pop out the buffer
    }
}

void AudioPlayer::RegisterCallback(ENGINE_CALLBACK cb, void *ctx) {
    callback_ = cb;
    ctx_ = ctx;
}

uint32_t  AudioPlayer::dbgGetDevBufCount(void) {
    return (devShadowQueue_->size());
}