Hello-libs
=========
hello-cdep is an Android sample that demos [CDEP](https://github.com/google/cdep) native lib distribution system usage.
Refer to [CDEP documentation](https://github.com/google/cdep/tree/master/doc)
The lib binaries and header files are hosted in [temporary repo](https://github.com/ggfan/hello-libs), this sample use
[CDEP](https://github.com/google/cdep) to pull it into [cmake](https://cmake.org/documentation) script.

Introduction
------------
This sample uses the new [Android Studio CMake plugin](http://tools.android.com/tech-docs/external-c-builds) with external lib support.
* How to use external pre-build libs packed inside [CDEP](ttps://github.com/google/cdep) package

Refer to [CDEP documentation](https://github.com/google/cdep/tree/master/doc)
The lib binaries and header files are hosted in [temporary repo](https://github.com/ggfan/hello-libs), this sample use 
[CDEP](https://github.com/google/cdep) to pull it into Android Studio's [cmake](https://cmake.org/documentation) script.

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

Screenshots
-----------
![screenshot](screenshot.png)

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
