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
#ifndef endlesstunnel_sfxman_hpp
#define endlesstunnel_sfxman_hpp

#include <SLES/OpenSLES.h>
#include <SLES/OpenSLES_Android.h>

#include "engine.hpp"

/* Sound effect manager. This class is a singleton that manages sound effect
 * playback. Sound effects are defined by recipes (which are strings) that
 * indicate frequencies and durations. See the PlayTone() method for more info.
 * Note: our sfx generation code is very simplistic and lacks a mixer, so
 * only one sound can be playing at any given time. This is not a problematic
 * limitation for this simple sample but, needless to say, doesn't scale well
 * to a more complex game. */
class SfxMan {
    private:
        bool mInitOk;
        SLAndroidSimpleBufferQueueItf mPlayerBufferQueue;

    public:
        SfxMan();

        // Returns the (singleton) instance of SfxMan
        static SfxMan* GetInstance();

        /* Play a tone according to the given recipe. The recipe consists of one or more
         * tones. Tones are separated by periods ('.'):
         *
         *    "<tone1>.<tone2>.<tone3>."
         *
         * Also, there must be a period at the end of the string.
         *
         * Each tone consists of settings separated by spaces. The order of settings
         * is irrelevant. Possible settings are:
         *     f<freq>  set frequency to <freq> Hz. 0 means noise.
         *     d<dur>   set duration to <dur> milliseconds.
         *     a<amp>   set amplitude to <amp> percent (0-100)
         *
         * Example: "d100 f300. d50 f250. a0 d100. a100 d50 f0."
         * This will play a 300Hz tone for 100ms, followed by a 250Hz tone
         * for 50 milliseconds, followed by 100ms of silence, followed
         * by 50 milliseconds of loud random noise. */
        void PlayTone(const char *tone);

        // Returns whether or not the sound effect pipeline is idle (able to play
        // a tone right now).
        bool IsIdle();
};

#endif

