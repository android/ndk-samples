Audio-Echo
==========
The sample demos how to use OpenSL ES to create a player and recorder in Android Fast Audio Path, and connect them to loopback audio. On most android devices, there is a optimized audio path that is tuned up for low latency purpose. The sample creates player/recorder to work in this highly optimized audio path(sometimes called native audio path, [low latency path](http://stackoverflow.com/questions/14842803/low-latency-audio-playback-on-android?rq=1), or fast audio path). The application is validated against the following configurations:
  *   Android L    AndroidOne
  *   Android M    Nexus 5, Nexus 9
This sample uses the new Android Studio with CMake support, and shows how to use shared stl lib with android studio version 2.2.0, see CMakeLists.txt for details

Pre-requisites
--------------
- Android Studio 2.2+ with [NDK](https://developer.android.com/ndk/) bundle.

Getting Started
---------------
1. [Download Android Studio](http://developer.android.com/sdk/index.html)
1. Launch Android Studio.
1. Open the sample directory.
1. Open *File/Project Structure...*
  - Click *Download* or *Select NDK location*.
1. Click *Tools/Android/Sync Project with Gradle Files*.
1. Click *Run/Run 'app'*.

Usage
-----
App will capture audio from android devices and playback on the same device; the playback on speaker will be captured immediately and played back...! So to verify it, it is recommended to "mute" the playback audio with a earspeaker/earphone/earbug so it does not get looped back.  Some device like Nexus 9, once you plug in an external headphone/headspeaker, it stops to use onboard microphone AND speaker anymore -- in this case, you need turn on the microphone coming with your headphone. Another point, when switching between external headphone and internal one, the volume is sometimes very low/muted; recommend to increase the playback volume with volume buttons on the phone/pad after plugging external headphone.

Low Latency Verification
------------------------

1. execute "adb shell dumpsys media.audio_flinger". Find a list of the running processes

   Name Active Client Type      Fmt Chn mask Session fCount S F SRate  L dB  R dB    Server Main buf  Aux Buf Flags UndFrmCnt  
   F  2     no    704    1 00000001 00000003     562  13248 S 1 48000  -inf  -inf  000033C0 0xabab8480 0x0 0x600         0  
   F  6    yes   9345    3 00000001 00000001     576    128 A 1 48000     0     0  0376AA00 0xabab8480 0x0 0x400       256 

1. execute adb shell ps  | grep echo  

  * find the sample app pid  
  * check with result on step 1.  
   if there is one "F" in the front of your echo pid, **player** is on fast audio path  
   For fast audio capture [it is totally different story], if you do **NOT** see  
   com.example.nativeaudio W/AudioRecord﹕ AUDIO_INPUT_FLAG_FAST denied by client  
in your logcat output when you are creating audio recorder, you could "assume" you are on the fast path.  
If your system image was built with muted ALOGW, you will not be able to see the above warning message.

Tune-ups
--------
A couple of knobs in the code for lower latency purpose:
  * audio buffer size
  * number of audio buffers cached before kicking start player
The lower you go with them, the lower latency you get and also the lower budget for audio processing. All audio processing has to be completed in the time period they are captured / played back, plus extra time needed for:
  * audio driver
  * audio flinger framework,
  * bufferqueue callbacks etc
Besides those, the irregularity of the buffer queue player/capture callback time is another factor. The callback from openSL may not as regular as you assumed, the more irregularity it is, the more likely have choopy audio. To fight that, more buffering is needed, which defeats the low-latency purpose! The low latency path is highly tuned up so you have better chance to get more regular callbacks. You may experiment with your platform to find the best parameters for lower latency and continuously playback audio experience.
The app capture and playback on the same device [most of times the same chip], capture and playback clocks are assumed synchronized naturally [so we are not dealing with it]

Credits
-------
  * The sample is greatly inspired by native-audio sample
  * Don Turner @ Google for the helping of low latency path
  * Ian Ni-Lewis @ Google for producer/consumer queue and many others

Support
-------
If you've found an error in these samples, please [file an issue](https://github.com/googlesamples/android-ndk/issues/new).

Patches are encouraged, and may be submitted by [forking this project](https://github.com/googlesamples/android-ndk/fork) and
submitting a pull request through GitHub. Please see [CONTRIBUTING.md](../CONTRIBUTING.md) for more details.

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
