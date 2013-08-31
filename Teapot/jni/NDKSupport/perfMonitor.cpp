/*
 * Copyright 2013 The Android Open Source Project
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

#include "perfMonitor.h"

perfMonitor::perfMonitor():_dLastTick(0.f), _tvLastSec(0), _tickindex(0), _ticksum(0)
{
    for(int32_t i = 0; i < NUM_SAMPLES; ++i )
        _ticklist[i] = 0;

}

perfMonitor::~perfMonitor() {
}

double perfMonitor::updateTick(double currentTick)
{
    _ticksum -= _ticklist[_tickindex];
    _ticksum += currentTick;
    _ticklist[_tickindex] = currentTick;
    _tickindex = (_tickindex+1)%NUM_SAMPLES;

    return((double)_ticksum/NUM_SAMPLES);
}

bool perfMonitor::update(float &fFPS)
{
    struct timeval Time;
    gettimeofday( &Time, NULL );

    double time = Time.tv_sec + Time.tv_usec * 1.0/1000000.0;
    double dTick = time - _dLastTick;
    double d = updateTick( dTick );
    _dLastTick = time;

    if( Time.tv_sec - _tvLastSec >= 1 )
    {
        double time = Time.tv_sec + Time.tv_usec * 1.0/1000000.0;
        _fCurrentFPS = 1.f / d;
        _tvLastSec = Time.tv_sec;
        fFPS = _fCurrentFPS;
        return true;
    }
    else
    {
        fFPS = _fCurrentFPS;
        return false;
    }
}
