prefab-publishing
=================

This sample shows how to export native libraries to an AAR using the [Prefab] format.
It builds a trivial library called `mylibrary` (libmylibrary.so), and packs the library
into mylibrary.aar. From here you could distribute the AAR via Maven or however
you distribute AARs.

In order to export native libraries in an AAR with Android Gradle Plugin 4.1+,
make the following changes to your Android Library project's [build.gradle]:

* Enable the `prefabPublishing` build feature
```
    android.buildFeatures.prefabPublishing true
```

* Declare the libraries and headers you wish to export in the `android.prefab`
  block
```
    android.prefab.mylibrary {
        headers "src/main/cpp/include"
    }
```
* (Optional) If the exported native modules are **not** used by the Kotlin/Java modules in this AAR,
  you could avoid [the unnecessary duplication inside AAR]
```
    packagingOptions {
        exclude("**/libmylibrary.so")
        exclude("**/libc++_shared.so")
    }
```


To learn how to use the generated AAR, refer to [the prefab-dependency sample] in this repo;
for the detailed documentation about Prefab in AGP, go to [C/C++ dependencies] webpage.

[Prefab]:https://google.github.io/prefab/
[build.gradle]:https://github.com/android/ndk-samples/blob/master/prefab/prefab-publishing/mylibrary/build.gradle#L64
[the prefab-dependency sample]:https://github.com/android/ndk-samples/tree/main/prefab/prefab-dependency
[the unnecessary duplication inside AAR]:https://issuetracker.google.com/issues/168777344#comment5
[C/C++ dependencies]: https://developer.android.com/studio/build/native-dependencies?buildsystem=cmake&agpversion=4.1


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
