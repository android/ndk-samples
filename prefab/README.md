Prefab Samples
==============

The samples here are to demontrate how to use [the Prefab feature] in Android Gradle Plugin.
Generally speaking, Prefab is a mechanism to make C/C++ libraries available to the gradle's
C/C++ build system, and the requirement is that the C/C++ libraries be organized in the
pre-defined format and packed inside AARs. Prefab's flow in Android Gradle Plugin, at high level,
could be viewed as:

1. Use Prefab to package C/C++ libraries (including the header files), called modules, into AAR
1. Distribute AARs with maven center or other mechanisms, but this is not Prefab specific
1. Add AAR dependency to your application, now you have access to the AAR ( but not prefab modules yet);
   but this is not Prefab specific either
1. Enable Prefab in the gradle scripts to access prefab module(s) inside AAR
1. Now you could use the Prefab modules in your app (in the source files and C/C++ build system)

Note again that the step 2 and 3 above are not C/C++ or prefab specific, please refer to
[the formal AAR documentations] for details. The rest of the steps are covered in the Prefab samples here:

* dependency-creation is for how to create Prefab dependency into an AAR
* dependency-integration is for how to use Prefab AARs hosted in Google Maven Center

curl-ssl and prefab-publishing directories are migrated into the samples named above.


[the Prefab feature]: https://developer.android.com/studio/build/native-dependencies?buildsystem=cmake&agpversion=4.0
[the formal AAR documentations]: https://developer.android.com/studio/projects/android-library

Prefab Availability
-------------------

Prefab has been added into Android Gradle Plugin since version 4:
* Using Prefab AARs: AGP 4.0+ (with update in 4.1+)
* Generating Prefab AARs: AGP 4.1+

The prefab AARs used by the samples here are hosted at [Google Maven], but you could
hosting your prefab AARs in anywhere accessible to gradle.

[Google Maven]: https://maven.google.com/web/index.html#com.android.ndk.thirdparty

Pre-requisites
--------------

* Android Gradle Plugin 4.0+/4.1+
* The [Android NDK](https://developer.android.com/ndk/)

Please check for the individial sample's README.md for anything specific to that sample.


Support
-------
If you've found an error in these samples, please [file an issue](https://github.com/android/ndk-samples/issues/new).

Patches are encouraged, and may be submitted by [forking this project](https://github.com/android/ndk-samples/fork) and
submitting a pull request through GitHub. Please see [CONTRIBUTING.md](../CONTRIBUTING.md) for more details.

- [Stack Overflow](http://stackoverflow.com/questions/tagged/android-ndk)
- [Android Tools Feedbacks](http://tools.android.com/feedback)

License
-------
Copyright 2020 Google, Inc.

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




