Hello-libs
=========
Hello-Libs is an Android sample that demos native lib management in Android Studio.

This sample uses the new [Gradle Experimental Android plugin](http://tools.android.com/tech-docs/new-build-system/gradle-experimental) with external lib support:  
*    Build JUST a native shared library[for distribution]
*    Use 3rd party shared library / Static library   

The key points are:
*    Inform Android Studio where lib/header is
*    Add lib dependency to application	
Details are inside projects' build.gradle files

Sample content:
*    Create 2 shared libs[on terminal, execute "gradlew :gmath:distributeLib :gperf:distributeLib"]
*    Copy built libs into distribution directory
*    Create one application to use libs from the given distribution directory

Pre-requisites
--------------
- Android Studio 2.0+ with [NDK](https://developer.android.com/ndk/) bundle.

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
