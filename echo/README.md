Echo
====
The sample demos how to create OpenSL ES to create a player and recorder, and connect them to loopback audio. On most android devices, there is a optimized audio path that is tuned up for low latency purpose. The sample create players to work in this highly optimized audio path(sometimes called native audio path, [low latency path](http://stackoverflow.com/questions/14842803/low-latency-audio-playback-on-android?rq=1), or fast audio path). The application is validated against the following configurations:
  *   Android L    AndroidOne
  *   Android M    Nexus 5, Nexus 9

Usage
-----
App will capture audio from android devices and playback on the same device; the playback on speaker will be captured immediately and played back...! So to verify it, it is recommended to "mute" the playback audio with a earphone/earbug so it does not get looped back.  Some device like Nexus 9, once you plug in a external headphone/headspeaker, it will not use onboard microphone AND speaker anymore -- in this case, you need turn on the microphone coming with your headphone.

Low Latency Verification
------------------------

1. execute adb shell dumpsys media.audio_flinger. Find a list of the running processes
    Name Active Client Type      Fmt Chn mask Session fCount S F SRate  L dB  R dB    Server Main buf  Aux Buf Flags UndFrmCnt
    F  2     no    704    1 00000001 00000003     562  13248 S 1 48000  -inf  -inf  000033C0 0xabab8480 0x0 0x600         0 
    F  5     no    597    1 00000001 00000003     257   6000 A 2 48000    -6    -6  00073B90 0xabab8480 0x0 0x600         0 
    F  1     no    597    1 00000001 00000003       9   6000 S 1 48000  -inf  -inf  00075300 0xabab8480 0x0 0x600         0 
    F  6    yes   9345    3 00000001 00000001     576    128 A 1 48000     0     0  0376AA00 0xabab8480 0x0 0x400       256 

2. execute adb shell ps  | grep echo; find the sample pid; with the pid, check up with result on step 1., if there is a "F" in the front of your echo pid, plaer is on fast audio path; otherwise, it is not; for fast audio capture[it is totally different story], if you do NOT see 
    com.example.nativeaudio W/AudioRecord﹕ AUDIO_INPUT_FLAG_FAST denied by client
you could assume you are on the fast path. If your system image build muted ALOGW, you will not see the above warning message.

Tune-ups
--------
A couple of knobs could be used for lower latency purpose, for example, audio buffer size, and how many audio buffers to cache before kicking start player. The lower you go with these 2 factors, the lower latency you will have; going with it is the audio processing efficiency -- to have a continously playing audio, all processing has to be finished within the capture/playback time for the same amount of the audio frames, minus other software layer overhead(audio driver, framework, and bufferqueue callbacks etc). Besides that, the irregularity of the buffer queue player/capture callback time -- the assumption of the regularity of evenly distributed callback time need to be highly verified on your interested platforms; the highly tuned up low latency audio path has better chance to "look" more regular than other audio paths. Taking iregularity into your consideration, you might have to increase the audio buffer size or caches more buffers between recorder and player. Finding the right trade-off between those factors could be a very interesting experience [alos high efficient code taking very little time to get work done] -- like what it is said "perfection is a journey". The knobs in the code are commented accordingly.

Credits
=======
  * the sample is greate inspired by native-audio sample
  * Don Turner @ Goolge for helping of low latency path
  * Ian Ni-Lewis @ Google for producer/consumer queue and many others

Pre-requisites
=============
If you need reference to how to use Android Studio for native samples, please refer to Sample Hello-Jni

Support
=======

If you've found an error in these samples, please [file an issue](https://github.com/googlesamples/android-ndk/issues/new).

Patches are encouraged, and may be submitted by [forking this project](https://github.com/googlesamples/android-ndk/fork) and
submitting a pull request through GitHub. Please see [CONTRIBUTING.md](CONTRIBUTING.md) for more details.

- [Stack Overflow](http://stackoverflow.com/questions/tagged/android-ndk)
- [Google+ Community](https://plus.google.com/communities/105153134372062985968)
- [Android Tools Feedbacks](http://tools.android.com/feedback)

License
-------

Copyright 2015 Google, Inc.

Licensed to the Apache Software Foundation (ASF) under one or more contributor
license agreements.  See the NOTICE file distributed with this work for
additional information regarding copyright ownership.  The ASF licenses this
file to you under the Apache License, Version 2.0 (the "License"); you may not
use this file except in compliance with the License.  You may obtain a copy of
the License at

  http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.  See the
License for the specific language governing permissions and limitations under
the License.
