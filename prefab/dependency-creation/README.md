Creating Prefab Dependency AAR
===============================

This sample shows how to export native libraries to an AAR
using the [Prefab] format. Generally speaking, to export native libraries in an AAR 
with Android Gradle Plugin 4.1+, make the following changes to your Android Library project's [build.gradle]:

* Enable the `prefabPublishing` build feature
```
    android.buildFeatures.prefabPublising true
```

* Declare the libraries and headers you wish to export in the `android.prefab`
  block
```
    android.prefab.mylibrary {
        headers "src/main/cpp/include"
    }
```
* (Optional)If the exported native modules are NOT used by local Kotlin/Java modules in this AAR,
  you could temporarily avoid [the unnecessary duplication inside AAR]
```
    packagingOptions {
        exclude("**/libmylibrary.so")
        exclude("**/libc++_shared.so")
    }
```
This sample builds a trivial library called `mylibrary` (libmylibrary.so), and packs it into 
mylibrary.aar. From here you could distribute the AAR via Maven center or in other ways.
For how to use the generated dependency AAR, refer to [the dependency-integration sample] in this repo; for the official documentation about Prefab in AGP, go to [C/C++ dependencies] website.

[Prefab]:https://google.github.io/prefab/
[build.gradle]:https://github.com/android/ndk-samples/blob/master/prefab/prefab-publishing/mylibrary/build.gradle#L64
[the dependency-integration sample]:https://github.com/android/ndk-samples/tree/main/prefab/dependency-integration



**Note** that you could use any other ways to generate the prefab AAR, as long as the AAR contains a
legal prefab package.


[the official C/C++ dependency documentation]: https://developer.android.com/studio/build/native-dependencies?buildsystem=cmake&agpversion=4.1
[the unnecessary duplication inside AAR]:https://b.corp.google.com/issues/168777344#comment5

Pre-requisites
--------------

* Android Gradle Plugin 4.1+
* The [Android NDK](https://developer.android.com/ndk/).


Getting Started
---------------

The C++ code in this sample is built with CMake, but the concepts demonstrated
here work identically when using ndk-build.

To build:

1. Navigate to this directory in your terminal.
2. Run `./gradlew build` (or `gradlew.bat build` on Windows).

Note that there is no runnable application in this sample. The output is the
AAR.

Support
-------

If you've found an error in these samples, please [file an
issue](https://github.com/android/ndk-samples/issues/new).

Patches are encouraged, and may be submitted by submitting a pull request
through GitHub. Please see [CONTRIBUTING.md](../../CONTRIBUTING.md) for more
details.
