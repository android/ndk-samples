Prefab Samples
==============

The samples here are to demontrate how to use [the Prefab feature] in Android Gradle Plugin.
Generally speaking, Prefab is a mechanism to make C/C++ libraries available to the gradle's
C/C++ build system, with the requirement that the C/C++ libraries are organized in the
[Prefab format] and packed inside AARs. In order to use the C/C++ dependency AAR, as AAR authors,
you need to do the following:

1. Use AGP to pack C/C++ libraries (including the header files), called modules, into AAR
1. Distribute AARs with [Google Maven] or other mechanisms, but this is not Prefab specific
1. Add AAR dependency to your application, now you have access to the AAR (but not prefab modules yet);
   again this is not Prefab specific either
1. Enable Prefab in the gradle scripts to access prefab module(s) inside AAR
1. Use Prefab modules in your native build system such as CMake or ndk-build

However as AAR users, you just need to do step 3 through step 5 above, i.e:
1. Add AAR dependency to your application, now you have access to the AAR (but not prefab modules yet);
   again this is not Prefab specific either
1. Enable Prefab in the gradle scripts to access prefab module(s) inside AAR
1. Use Prefab modules in your native build system such as CMake or ndk-build

Note that the step 2 and 3 above are not C/C++ or prefab specific, please refer to
[the AAR documentation] for details.

With that in mind, the samples here collectively demonstrate prefab usage with:

* prefab-publishing shows how to create an AAR for distributing native libraries
* prefab-dependency shows how to import native dependencies from [GMaven]
* curl-ssl: for shows to use 2 very specific and important AARs (curl and ssl)


[the Prefab feature]: https://developer.android.com/studio/build/native-dependencies?buildsystem=cmake&agpversion=4.0
[Prefab format]: https://google.github.io/prefab
[the formal AAR documentations]: https://developer.android.com/studio/projects/android-library
[GMaven]:(https://maven.google.com/web/index.html?q=ndk#com.android.ndk.thirdparty)

Prefab Availability
-------------------

Support for Prefab packages has been available Android Gradle Plugin since version 4.0:
* Consuming native dependencies requires AGP 4.0+
* Generating AARs that export native dependencies requires AGP 4.1+

The AARs used by the samples here are hosted at [Google Maven], but you can
host your AARs anywhere accessible to gradle.

[Google Maven]: https://maven.google.com/web/index.html#com.android.ndk.thirdparty

Pre-requisites
--------------

* Android Gradle Plugin 4.0+
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




