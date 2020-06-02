Teapots
======
Teapots is an collection of Android C++ samples that uses a Teapot rendering to demostrate Android NDK platform features:
- classic-teapot: Rendering classic teapot mesh using GLES 2.0 API and [NativeActivity](http://developer.android.com/reference/android/app/NativeActivity.html).
- more-teapots: Rendering multiple instances of Classic Teapot with GLES 3.0 Instance Rendering
- choreographer-30fps: demonstrates multiple frame rate throttoling techniques based on API level using Choreographer API and EGL Android presentation time extension.
- textured-teapot: Rendering classic teapot plus textures
- image-decoder: Same as textured-teapot, except that texture decoding is done
  with the ImageDecoder API introduced in Android 11 (Android NDK r21b)

This sample uses the new [Android Studio CMake plugin](http://tools.android.com/tech-docs/external-c-builds) with C++ support.

Pre-requisites
--------------
- Android Studio 4.0.0 with [NDK](https://developer.android.com/ndk/)
- Android [NDK 21.1+ installed](https://github.com/android/ndk-samples/wiki/Configure-NDK-Path)

Getting Started
---------------
1. [Download Android Studio](http://developer.android.com/sdk/index.html)
1. Launch Android Studio.
1. Open the sample directory.
1. Open *File/Project Structure...*
  - Click *Download* or *Select NDK location*.
1. Click *Tools/Android/Sync Project with Gradle Files*.
1. Click *Run/Run 'app'*.

Screenshots
-----------
![screenshot](screenshot.png)

Support
-------
If you've found an error in these samples, please [file an issue](https://github.com/android/ndk-samples/issues/new).

Patches are encouraged, and may be submitted by [forking this project](https://github.com/android/ndk-samples/fork) and
submitting a pull request through GitHub. Please see [CONTRIBUTING.md](../CONTRIBUTING.md) for more details.

- [Stack Overflow](http://stackoverflow.com/questions/tagged/android-ndk)
- [Android Tools Feedbacks](http://tools.android.com/feedback)

License
-------
Copyright 2020 Google LLC.

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
