NDK Samples [![Build Status](https://travis-ci.org/googlesamples/android-ndk.svg?branch=master)](https://travis-ci.org/googlesamples/android-ndk) [![Build status](https://ci.appveyor.com/api/projects/status/48tbtqwg4heytmnq?svg=true)](https://ci.appveyor.com/project/proppy/android-ndk)
===========

This repository contains [Android NDK][0] samples with Android Studio [C++ integration](https://www.youtube.com/watch?v=f7ihSQ44WO0&feature=youtu.be).

These samples uses the new [CMake Android plugin](https://developer.android.com/studio/projects/add-native-code.html) with C++ support.

Samples could also be built with other build systems:
- for ndk-build with Android Studio, refer to directory [other-builds/ndkbuild](https://github.com/googlesamples/android-ndk/tree/master/other-builds/ndkbuild)
- for gradle-experimental plugin, refer to directory other-builds/experimental. Note that gradle-experimental does not work with unified headers yet: use NDK version up to r15 and Android Studio up to version 2.3

Additional Android Studio samples:    
- [Google Play Game Samples with Android Studio](https://github.com/playgameservices/cpp-android-basic-samples)
- [Google Android Vulkan Tutorials](https://github.com/googlesamples/android-vulkan-tutorials)
- [Android Vulkan API Basic Samples](https://github.com/googlesamples/vulkan-basic-samples)
- [Android High Performance Audio](https://github.com/googlesamples/android-audio-high-performance)	

Documentation
- [Add Native Code to Your Project](https://developer.android.com/studio/projects/add-native-code.html)
- [CMake for NDK](https://developer.android.com/ndk/guides/cmake.html)
- [Hello-CMake Codelab](https://codelabs.developers.google.com/codelabs/android-studio-cmake/index.html)

Known Issues
- Some are documented at [Android Studio](http://tools.android.com/knownissues) page

For samples using `Android.mk` build system with `ndk-build` see the [android-mk](https://github.com/googlesamples/android-ndk/tree/android-mk) branch.

Build Steps
----------
- With Android Studio: use "Import Project(Eclipse ADT, Gradle, etc)" or "File" > "Import Project" option
- On Command Line/Terminal:  make sure set up ANDROID_HOME and ANDROID_NDK_HOME to local installation of SDK and NDK, then go to individual sample dir, and use "gradlew assembleDebug"

Debugging
---------
- [References](REFERENCE.md)
 
Support
-------

- [Google+ Community](https://plus.google.com/communities/105153134372062985968)
- [Stack Overflow](http://stackoverflow.com/questions/tagged/android)

If you've found an error in these samples, please [file an issue](https://github.com/googlesamples/android-ndk/issues/new).

Patches and new samples are encouraged, and may be submitted by [forking this project](https://github.com/googlesamples/android-ndk/fork) and
submitting a pull request through GitHub. Please see [CONTRIBUTING.md](CONTRIBUTING.md) for more details.

License
-------

Copyright 2015 The Android Open Source Project, Inc.

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

[LICENSE](LICENSE)

[0]: https://developer.android.com/ndk
