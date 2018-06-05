/*
 * Copyright 2018 The Android Open Source Project
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
#include "audio_effect.h"
#include "audio_common.h"
#include <climits>
#include <cstring>

/*
 * Mixing Audio in integer domain to avoid FP calculation
 *   (FG * ( MixFactor * 16 ) + BG * ( (1.0f-MixFactor) * 16 )) / 16
 */
static const int32_t kFloatToIntMapFactor = 128;
static const uint32_t kMsPerSec = 1000;
/**
 * Constructor for AudioDelay
 * @param sampleRate
 * @param channelCount
 * @param format
 * @param delayTimeInMs
 */
AudioDelay::AudioDelay(int32_t sampleRate, int32_t channelCount,
                       SLuint32 format, size_t delayTimeInMs,
                       float decayWeight)
    : AudioFormat(sampleRate, channelCount, format),
      delayTime_(delayTimeInMs),
      decayWeight_(decayWeight) {
  feedbackFactor_ = static_cast<int32_t>(decayWeight_ * kFloatToIntMapFactor);
  liveAudioFactor_ = kFloatToIntMapFactor - feedbackFactor_;
  allocateBuffer();
}

/**
 * Destructor
 */
AudioDelay::~AudioDelay() {
  if (buffer_) delete static_cast<uint8_t*>(buffer_);
}

/**
 * Configure for delay time ( in miliseconds ), dynamically adjustable
 * @param delayTimeInMS in miliseconds
 * @return true if delay time is set successfully
 */
bool AudioDelay::setDelayTime(size_t delayTimeInMS) {
  if (delayTimeInMS == delayTime_) return true;

  std::lock_guard<std::mutex> lock(lock_);

  if (buffer_) {
    delete static_cast<uint8_t*>(buffer_);
    buffer_ = nullptr;
  }

  delayTime_ = delayTimeInMS;
  allocateBuffer();
  return buffer_ != nullptr;
}

/**
 * Internal helper function to allocate buffer for the delay
 *  - calculate the buffer size for the delay time
 *  - allocate and zero out buffer (0 means silent audio)
 *  - configure bufSize_ to be size of audioFrames
 */
void AudioDelay::allocateBuffer(void) {
  float floatDelayTime = (float)delayTime_ / kMsPerSec;
  float fNumFrames = floatDelayTime * (float)sampleRate_ / kMsPerSec;
  size_t sampleCount = static_cast<uint32_t>(fNumFrames + 0.5f) * channelCount_;

  uint32_t bytePerSample = format_ / 8;
  assert(bytePerSample <= 4 && bytePerSample);

  uint32_t bytePerFrame = channelCount_ * bytePerSample;

  // get bufCapacity in bytes
  bufCapacity_ = sampleCount * bytePerSample;
  bufCapacity_ =
      ((bufCapacity_ + bytePerFrame - 1) / bytePerFrame) * bytePerFrame;

  buffer_ = new uint8_t[bufCapacity_];
  assert(buffer_);

  memset(buffer_, 0, bufCapacity_);
  curPos_ = 0;

  // bufSize_ is in Frames ( not samples, not bytes )
  bufSize_ = bufCapacity_ / bytePerFrame;
}

size_t AudioDelay::getDelayTime(void) const { return delayTime_; }

/**
 * setDecayWeight(): set the decay factor
 * ratio: value of 0.0 -- 1.0f;
 *
 * the calculation is in integer ( not in float )
 * for performance purpose
 */
void AudioDelay::setDecayWeight(float weight) {
  if (weight > 0.0f && weight < 1.0f) {
    float feedback = (weight * kFloatToIntMapFactor + 0.5f);
    feedbackFactor_ = static_cast<int32_t>(feedback);
    liveAudioFactor_ = kFloatToIntMapFactor - feedbackFactor_;
  }
}

float AudioDelay::getDecayWeight(void) const { return decayWeight_; }

/**
 * process() filter live audio with "echo" effect:
 *   delay time is run-time adjustable
 *   decay time could also be adjustable, but not used
 *   in this sample, hardcoded to .5
 *
 * @param liveAudio is recorded audio stream
 * @param channelCount for liveAudio, must be 2 for stereo
 * @param numFrames is length of liveAudio in Frames ( not in byte )
 */
void AudioDelay::process(int16_t* liveAudio, int32_t numFrames) {
  if (feedbackFactor_ == 0 || bufSize_ < numFrames) {
    return;
  }

  if (!lock_.try_lock()) {
    return;
  }

  if (numFrames + curPos_ > bufSize_) {
    curPos_ = 0;
  }

  // process every sample
  int32_t sampleCount = channelCount_ * numFrames;
  int16_t* samples = &static_cast<int16_t*>(buffer_)[curPos_ * channelCount_];
  for (size_t idx = 0; idx < sampleCount; idx++) {
#if 1
    int32_t curSample =
        (samples[idx] * feedbackFactor_ + liveAudio[idx] * liveAudioFactor_) /
        kFloatToIntMapFactor;
    if (curSample > SHRT_MAX)
      curSample = SHRT_MAX;
    else if (curSample < SHRT_MIN)
      curSample = SHRT_MIN;
 
    liveAudio[idx] = samples[idx];
    samples[idx] = static_cast<int16_t>(curSample);
#else
    // Pure delay
    int16_t tmp = liveAudio[idx];
    liveAudio[idx] = samples[idx];
    samples[idx] = tmp;
#endif
  }

  curPos_ += numFrames;
  lock_.unlock();
}
