/*
 * Copyright (C) 2019 The Android Open Source Project
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


#ifndef HELLO_OBOE_OBOESINEPLAYER_H
#define HELLO_OBOE_OBOESINEPLAYER_H


#include <math.h>
#include <algorithm>

#include <oboe/Oboe.h>

class OboeSinePlayer: public oboe::AudioStreamCallback {
public:

    OboeSinePlayer() {
        oboe::AudioStreamBuilder builder;
        // The builder set methods can be chained for convenience.
        builder.setSharingMode(oboe::SharingMode::Exclusive)
                ->setPerformanceMode(oboe::PerformanceMode::LowLatency);
        builder.setFormat(oboe::AudioFormat::Float);
        builder.setCallback(this)->openManagedStream(outStream);
        // Typically, start the stream after querying some stream information, as well as some input from the user
        channelCount = outStream->getChannelCount();
        mPhaseIncrement = kFrequency * kTwoPi / outStream->getSampleRate();
        outStream->requestStart();
    }

    // This class will also be used for the callback
    // For more complicated callbacks create a separate class
    oboe::DataCallbackResult onAudioReady(oboe::AudioStream *oboeStream, void *audioData, int32_t numFrames) override {
        float *floatData = (float *) audioData;
        if (isOn) {
            for (int i = 0; i < numFrames; ++i) {
                float sampleValue = kAmplitude * sinf(mPhase);
                for (int j = 0; j < channelCount; j++) {
                    floatData[i * channelCount + j] = sampleValue;
                }
                mPhase += mPhaseIncrement;
                if (mPhase >= kTwoPi) mPhase -= kTwoPi;
            }
        } else {
            std::fill_n(floatData, numFrames * channelCount, 0);
        }
        return oboe::DataCallbackResult::Continue;
    }

    void enable(bool toEnable) {
        isOn.store(toEnable);
    }

private:
    oboe::ManagedStream outStream;

    std::atomic_bool isOn;
    int channelCount;
    double mPhaseIncrement;

    // Wave params, these could be instance variables in order to modify at runtime
    static float constexpr kAmplitude = 0.5f;
    static float constexpr kFrequency = 440;
    static double constexpr kTwoPi = M_PI * 2;

    // Keeps track of where the wave is
    float mPhase = 0.0;
};

#endif //HELLO_OBOE_OBOESINEPLAYER_H
