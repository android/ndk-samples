/*
 * Copyright (C) Google Inc.
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
#include <random>
#include "sfxman.hpp"

#define SAMPLES_PER_SEC 8000
#define BUF_SAMPLES_MAX SAMPLES_PER_SEC*5 // 5 seconds
#define DEFAULT_VOLUME 0.9f

static SfxMan *_instance = new SfxMan();
static short _sample_buf[BUF_SAMPLES_MAX];
static volatile bool _bufferActive = false;

SfxMan* SfxMan::GetInstance() {
    return _instance ? _instance : (_instance = new SfxMan());
}

static bool _checkError(SLresult r, const char *what) {
    if (r != SL_RESULT_SUCCESS) {
        LOGW("SfxMan: Error %s (result %lu)", what, (long unsigned int)r);
        LOGW("DISABLING SOUND!");
        return true;
    }
    return false;
}

static void _bqPlayerCallback(SLAndroidSimpleBufferQueueItf bq, void *context) {
    _bufferActive = false;
}


SfxMan::SfxMan() {
    // Note: this initialization code was mostly copied from the NDK audio sample.
    SLresult result;
    SLObjectItf engineObject = NULL;
    SLEngineItf engineEngine;
    SLObjectItf outputMixObject = NULL;
    SLEnvironmentalReverbItf outputMixEnvironmentalReverb = NULL;
    SLObjectItf bqPlayerObject = NULL;
    SLPlayItf bqPlayerPlay;
    SLEffectSendItf bqPlayerEffectSend;
    SLVolumeItf bqPlayerVolume;
    const SLEnvironmentalReverbSettings reverbSettings =
            SL_I3DL2_ENVIRONMENT_PRESET_STONECORRIDOR;

    LOGD("SfxMan: initializing.");
    mPlayerBufferQueue = NULL;

    // create engine
    result = slCreateEngine(&engineObject, 0, NULL, 0, NULL, NULL);
    if (_checkError(result, "creating engine")) return;

    // realize the engine
    result = (*engineObject)->Realize(engineObject, SL_BOOLEAN_FALSE);
    if (_checkError(result, "realizing engine")) return;

    // get the engine interface, which is needed in order to create other objects
    result = (*engineObject)->GetInterface(engineObject, SL_IID_ENGINE, &engineEngine);
    if (_checkError(result, "getting engine interface")) return;

    // create output mix, with einitializingnvironmental reverb specified as a non-required interface
    const SLInterfaceID ids[1] = {SL_IID_ENVIRONMENTALREVERB};
    const SLboolean req[1] = {SL_BOOLEAN_FALSE};
    result = (*engineEngine)->CreateOutputMix(engineEngine, &outputMixObject, 1, ids, req);
    if (_checkError(result, "creating output mix")) return;

    // realize the output mix
    result = (*outputMixObject)->Realize(outputMixObject, SL_BOOLEAN_FALSE);
    if (_checkError(result, "realizin goutput mix")) return;

    // get the environmental reverb interface
    // this could fail if the environmental reverb effect is not available,
    // either because the feature is not present, excessive CPU load, or
    // the required MODIFY_AUDIO_SETTINGS permission was not requested and granted
    result = (*outputMixObject)->GetInterface(outputMixObject, SL_IID_ENVIRONMENTALREVERB,
                &outputMixEnvironmentalReverb);
    if (SL_RESULT_SUCCESS == result) {
        result = (*outputMixEnvironmentalReverb)->SetEnvironmentalReverbProperties(
                    outputMixEnvironmentalReverb, &reverbSettings);
    }
    // ignore unsuccessful result codes for environmental reverb, as it is optional for this example

    // configure audio source
    SLDataLocator_AndroidSimpleBufferQueue loc_bufq = {SL_DATALOCATOR_ANDROIDSIMPLEBUFFERQUEUE, 2};
    SLDataFormat_PCM format_pcm = {SL_DATAFORMAT_PCM, 1, SL_SAMPLINGRATE_8,
        SL_PCMSAMPLEFORMAT_FIXED_16, SL_PCMSAMPLEFORMAT_FIXED_16,
        SL_SPEAKER_FRONT_CENTER, SL_BYTEORDER_LITTLEENDIAN};

    SLDataSource audioSrc = {&loc_bufq, &format_pcm};

    // configure audio sink
    SLDataLocator_OutputMix loc_outmix = {SL_DATALOCATOR_OUTPUTMIX, outputMixObject};
    SLDataSink audioSnk = {&loc_outmix, NULL};

    // create audio player
    const SLInterfaceID player_ids[3] = {SL_IID_BUFFERQUEUE, SL_IID_EFFECTSEND,
            /*SL_IID_MUTESOLO,*/ SL_IID_VOLUME};
    const SLboolean player_req[3] = {SL_BOOLEAN_TRUE, SL_BOOLEAN_TRUE,
            /*SL_BOOLEAN_TRUE,*/ SL_BOOLEAN_TRUE};
    result = (*engineEngine)->CreateAudioPlayer(engineEngine, &bqPlayerObject,
            &audioSrc, &audioSnk, 3, player_ids, player_req);
    if (_checkError(result, "creating audio player")) return;

    // realize the player
    result = (*bqPlayerObject)->Realize(bqPlayerObject, SL_BOOLEAN_FALSE);
    assert(SL_RESULT_SUCCESS == result);

    // get the play interface
    result = (*bqPlayerObject)->GetInterface(bqPlayerObject, SL_IID_PLAY, &bqPlayerPlay);
    if (_checkError(result, "realizing audio player")) return;

    // get the buffer queue interface
    result = (*bqPlayerObject)->GetInterface(bqPlayerObject, SL_IID_BUFFERQUEUE,
                &mPlayerBufferQueue);
    if (_checkError(result, "getting buffer queue interface")) return;

    // register callback on the buffer queue
    result = (*mPlayerBufferQueue)->RegisterCallback(mPlayerBufferQueue, _bqPlayerCallback, NULL);
    if (_checkError(result, "registering callback on buffer queue")) return;

    // get the effect send interface
    result = (*bqPlayerObject)->GetInterface(bqPlayerObject, SL_IID_EFFECTSEND,
                &bqPlayerEffectSend);
    if (_checkError(result, "getting effect send interface")) return;

    // get the volume interface
    result = (*bqPlayerObject)->GetInterface(bqPlayerObject, SL_IID_VOLUME, &bqPlayerVolume);
    if (_checkError(result, "getting volume interface")) return;

    // set the player's state to playing
    result = (*bqPlayerPlay)->SetPlayState(bqPlayerPlay, SL_PLAYSTATE_PLAYING);
    if (_checkError(result, "setting play state to playing")) return;

    LOGD("SfxMan: initialization complete.");
    mInitOk = true;
}

bool SfxMan::IsIdle() {
    return !_bufferActive;
}

static const char *_parseInt(const char *s, int *result) {
    *result = 0;
    while (*s >= '0' && *s <= '9') {
        *result = *result * 10 + (*s - '0');
        s++;
    }
    return s;
}

static int _synth(int frequency, int duration, float amplitude, short *sample_buf, int samples) {
    int i;

    for (i = 0; i < samples; i++) {
        float t = i / (float)SAMPLES_PER_SEC;
        float v;
        if (frequency > 0) {
            v = amplitude * sin(frequency * t * 2 * M_PI) +
                  (amplitude * 0.1f) * sin(frequency * 2 * t * 2 * M_PI);
        } else {
            int r = rand();
            r = r > 0 ? r : -r;
            v = amplitude * (-0.5f + (r % 1024) / 512.0f);
        }
        int value = (int)(v * 32768.0f);
        sample_buf[i] = value < -32767 ? -32767 : value > 32767 ? 32767 : value;

        if (i > 0 && sample_buf[i-1] < 0 && sample_buf[i] >= 0) {
            // start of new wave -- check if we have room for a full period of it
            int period_samples = (1.0f / frequency) * SAMPLES_PER_SEC;
            if (i + period_samples >= samples) break;
        }
    }

    return i;
}

static void _taper(short *sample_buf, int samples) {
    int i;
    const float TAPER_SAMPLES_FRACTION = 0.1f;
    int taper_samples = (int)(TAPER_SAMPLES_FRACTION * samples);
    for (i = 0; i < taper_samples && i < samples; i++) {
        float factor = i / (float)taper_samples;
        sample_buf[i] = (short)((float)sample_buf[i] * factor);
    }
    for (i = samples - taper_samples; i < samples; i++) {
        if (i < 0) continue;
        float factor = (samples - i)/ (float)taper_samples;
        sample_buf[i] = (short)((float)sample_buf[i] * factor);
    }
}

void SfxMan::PlayTone(const char *tone) {
    if (!mInitOk) {
        LOGW("SfxMan: not playing sound because initialization failed.");
        return;
    }
    if (_bufferActive) {
        // can't play -- the buffer is in use
        LOGW("SfxMan: can't play tone; buffer is active.");
        return;
    }

    // synth the tone
    int total_samples = 0;
    int num_samples;
    int frequency = 100;
    int duration = 50;
    int volume_int;
    float amplitude = DEFAULT_VOLUME;

    while (*tone) {
       switch (*tone) {
           case 'f':
               // set frequency
               tone = _parseInt(tone + 1, &frequency);
               break;
           case 'd':
               // set duration
               tone = _parseInt(tone + 1, &duration);
               break;
           case 'a':
               // set amplitude.
               tone = _parseInt(tone + 1, &volume_int);
               amplitude = volume_int / 100.0f;
               amplitude = amplitude < 0.0f ? 0.0f : amplitude > 1.0f ? 1.0f : amplitude;
               break;
           case '.':
               // synth
               num_samples = duration * SAMPLES_PER_SEC / 1000;
               if (num_samples > (BUF_SAMPLES_MAX - total_samples - 1)) {
                   num_samples = BUF_SAMPLES_MAX - total_samples - 1;
               }
               num_samples = _synth(frequency, duration, amplitude, _sample_buf + total_samples,
                       num_samples);
               total_samples += num_samples;
               tone++;
               break;
           default:
               // ignore and advance to next character
               tone++;
       }
    }

    SLresult result;
    int total_size = total_samples * sizeof(short);
    if (total_size <= 0) {
        LOGW("Tone is empty. Not playing.");
        return;
    }

    _taper(_sample_buf, total_samples);

    _bufferActive = true;
    result = (*mPlayerBufferQueue)->Enqueue(mPlayerBufferQueue, _sample_buf, total_size);
    if (result != SL_RESULT_SUCCESS) {
        LOGW("SfxMan: warning: failed to enqueue buffer: %lu", (unsigned long)result);
        return;
    }
}

