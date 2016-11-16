Hello-libs
=========
Hello-Libs is an Android sample that demos 3rd party native lib management with Android Studio

Introduction
------------
This sample uses the new [Android Studio CMake plugin](http://tools.android.com/tech-docs/external-c-builds) with external lib support.
* how external pre-build static lib (gmath) could be used in app
* how external pre-built shared lib (gperf) could be used in app

Description
-----------
The sample includes 2 modules but only uses app module:
*    app -- uses one shared lib and one static lib from $project/distribution/
*    gen-libs -- generates one shared and one static lib, and copy them into $project/distribution
For this demo purpose, you do not need to build libs: binaries are included in the project -- the 
debug library binaries are saved inside distribution folder. If you want, you could build your own with
gen-libs source, just follow comment in setting.gradle and app/build.gradle  -- do it once, then
comment them out again so you are not affected by lib building

The key point is to inform app's CMakeLists.txt
*    where lib/header are
*    where lib binaries are and import them as static or shared

Note: for shared lib, with android plugin 2.2.0-alpha3+, once declared as SHARED IMPORTED, Android
Studio will automatically pack them into apk too! So lib just need tell Android Studio once, it
will be used both on Host and on Target

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
