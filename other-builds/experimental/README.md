NDK Samples [![Build Status](https://travis-ci.org/googlesamples/android-ndk.svg?branch=master)](https://travis-ci.org/googlesamples/android-ndk) [![Build status](https://ci.appveyor.com/api/projects/status/48tbtqwg4heytmnq?svg=true)](https://ci.appveyor.com/project/proppy/android-ndk)
===========

This repository contains [Android NDK][0] samples with Android Studio [C++ integration](https://www.youtube.com/watch?v=f7ihSQ44WO0&feature=youtu.be).

There are 2 branches:
- master-cmake: uses the new [Gradle CMake support](http://tools.android.com/tech-docs/external-c-builds)
- master: uses the new [Gradle Experimental Android plugin](http://tools.android.com/tech-docs/new-build-system/gradle-experimental)

Additional Android Studio samples:    
- [Google Play Game Samples with Android Studio](https://github.com/playgameservices/cpp-android-basic-samples)
- [Google Android Vulkan Tutorials](https://github.com/googlesamples/android-vulkan-tutorials)
- [Android Vulkan API Basic Samples](https://github.com/googlesamples/vulkan-basic-samples)
- [Android High Performance Audio](https://github.com/googlesamples/android-audio-high-performance)	

Known Issues
- Some are documented at [Android Studio](http://tools.android.com/knownissues) page
- For native application, editing some gradle properties [like sdk version] from Android Studio UI is not supported

For samples using `Android.mk` build system with `ndk-build` see the [android-mk](https://github.com/googlesamples/android-ndk/tree/android-mk) branch.

Debugging
---------
- [hello-jni-codelab](https://codelabs.developers.google.com/codelabs/android-studio-jni/index.html?index=..%2F..%2Findex#0)
- [REFERENCE.md](REFERENCE.md) 

Support
-------

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
