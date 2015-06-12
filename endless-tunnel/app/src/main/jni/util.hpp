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
#ifndef endlesstunnel_util_hpp
#define endlesstunnel_util_hpp

#include <ctime>
#include <cmath>

// Clean up a resource (delete and set to null).
template<class T> inline void CleanUp(T** pptr) {
    if (*pptr) {
        delete *pptr;
        *pptr = NULL;
    }
}

int Random(int uboundExclusive);
inline int Random(int lbound, int uboundExclusive) {
    return lbound + Random(uboundExclusive - lbound);
}

inline int Max(int a, int b) { return a > b ? a : b; }
inline int Min(int a, int b) { return a < b ? a : b; }
inline int Clamp(int v, int min, int max) {
    return (v < min) ? min : (v > max) ? max : v;
}
inline float Max(float a, float b) { return a > b ? a : b; }
inline float Min(float a, float b) { return a < b ? a : b; }
inline float Clamp(float v, float min, float max) {
    return (v < min) ? min : (v > max) ? max : v;
}

// Returns current wall clock time (seconds elapsed since an arbitrary fixed point in the past).
float Clock();

// Linear interpolation. If x < x1, returns y1. If x > x2, returns y2. If x1 <= x <= x2,
// then let f() be a linear function such that f(x1) = y1 and f(x2) = y2. Returns f(x).
inline float Interpolate(float x1, float y1, float x2, float y2, float x) {
    if (x2 < x1) return Interpolate(x2, y2, x1, y1, x);
    return (x < x1) ? y1 : (x > x2) ? y2 : y1 + ((x - x1) / (x2 - x1)) * (y2 - y1);
}

// If abs(orig - target) <= amount, returns target. Otherwise, returns
// either orig + amount or orig - amount, whichever one is closer to the target.
// Intuitively, this means we are at x=orig and are aiming at x=target, and can
// move up to <amount> units. For example, if orig is 50 and the target is 100,
// then moving it by 10 will yield 60. If the target were 0, it would yield
// 40. Now, if the target is within reach of the delta, the target is returned.
// So if we're at 50, the target is 60 and the delta amount is 20, this returns
// 60.
inline float Approach(float orig, float target, float amount) {
    float absDiff = orig - target;
    if (absDiff < 0.0f) absDiff = -absDiff;
    if (absDiff < amount) {
        return target;
    } else if (target > orig) {
        return orig + amount;
    } else {
        return orig - amount;
    }
}

inline float Abs(float f) {
    return f > 0.0f ? f : -f;
}

inline float SineWave(float min, float max, float period, float phase) {
    float ampl = max - min;
    return min + ampl * sin(((Clock() / period) + phase) * 2 * M_PI);
}

inline bool BlinkFunc(float period) {
    return (int)(Clock() / period) & 1;
}


/* A simple chronometer that computes elapsed time. */
class DeltaClock {
    private:
        float mLastTick;
        float mMaxDelta;
        bool mHasMax;
    public:
        inline DeltaClock() {
            mLastTick = Clock();
            mHasMax = false;
        }
        inline DeltaClock(float maxDelta) {
            mLastTick = Clock();
            mMaxDelta = maxDelta;
            mHasMax = true;
        }
        inline float ReadDelta() {
            float d = Clamp(Clock() - mLastTick, 0.0f, mMaxDelta);
            mLastTick = Clock();
            return d;
        }
        inline void SetMaxDelta(float m) {
            mMaxDelta = m;
        }
        inline void Reset() {
            mLastTick = Clock();
        }
};

#endif

