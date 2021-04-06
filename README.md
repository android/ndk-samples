NDK Samples [![build](https://github.com/android/ndk-samples/workflows/build/badge.svg)](https://github.com/android/ndk-samples/actions) 
===========

This repository contains [Android NDK][0] samples with Android Studio [C++ integration](https://www.youtube.com/watch?v=f7ihSQ44WO0&feature=youtu.be).

These samples use the new [CMake Android plugin](https://developer.android.com/studio/projects/add-native-code.html) with C++ support.

Samples could also be built with other build systems:
- for ndk-build with Android Studio, refer to directory [other-builds/ndkbuild](https://github.com/googlesamples/android-ndk/tree/master/other-builds/ndkbuild)
- for gradle-experimental plugin, refer to directory other-builds/experimental. Note that gradle-experimental does not work with unified headers yet: use NDK version up to r15 and Android Studio up to version 2.3. When starting new project, please use CMake or ndk-build plugin.

Additional Android Studio samples:    
- [Google Play Game Samples with Android Studio](https://github.com/playgameservices/cpp-android-basic-samples)
- [Google Android Vulkan Tutorials](https://github.com/googlesamples/android-vulkan-tutorials)
- [Android Vulkan API Basic Samples](https://github.com/googlesamples/vulkan-basic-samples)
- [Android High Performance Audio](https://github.com/googlesamples/android-audio-high-performance)	

Documentation
- [Add Native Code to Your Project](https://developer.android.com/studio/projects/add-native-code.html)
- [Configure NDK for Android Studio/Gradle Plugin](https://github.com/android/ndk-samples/wiki/Configure-NDK-Path)
- [CMake for NDK](https://developer.android.com/ndk/guides/cmake.html)

Known Issues
- For Studio related issues, refer to [Android Studio known issues](http://tools.android.com/knownissues) page
- For NDK issues, refer to [ndk issues](https://github.com/android/ndk/issues)

For samples using `Android.mk` build system with `ndk-build` see the [android-mk](https://github.com/googlesamples/android-ndk/tree/android-mk) branch.

Build Steps
----------
- With Android Studio: "Open An Existing Android Studio Project" or "File" > "Open", then navigate to & select project's build.gradle file.
- On Command Line: set up ANDROID_HOME and ANDROID_NDK_HOME to your SDK and NDK path, cd to individual sample dir, and do "gradlew assembleDebug"

Support
-------

For any issues you found in these samples, please
- submit patches with pull requests, see [CONTRIBUTING.md](CONTRIBUTING.md) for more details, or
- [create bugs](https://github.com/googlesamples/android-ndk/issues/new) here.

For Android NDK generic questions, please ask on [Stack Overflow](https://stackoverflow.com/questions/tagged/android), Android teams are periodically monitoring questions there.


License
-------

Copyright 2018 The Android Open Source Project, Inc.

Licensed to the Apache Software Foundation (ASF) under one or more contributor
license agreements.  See the NOTICE file distributed with this work for
additional information regarding copyright ownership.  The ASF licenses this
file to you under the Apache License, Version 2.0 (the "License"); you may not
use this file except in compliance with the License.  You may obtain a copy of
the License at

https://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.  See the
License for the specific language governing permissions and limitations under
the License.

[LICENSE](LICENSE)

[0]: https://developer.android.com/ndk
