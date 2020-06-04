# Prefab AAR Publishing

The prefab-publishing sample shows how to export native libraries in an AAR
using the [Prefab] format. This sample build a trivial library `mylibrary` that
exports the `void my_api();` function to be used by projects that import this
AAR.

To export native libraries in an AAR with Android Gradle Plugin 4.1+, make the
following changes to your Android Library project's [build.gradle]:

* Enable the `prefabPublishing` build feature
* Declare the libraries and headers you wish to export in the `android.prefab`
  block

[Prefab]:https://google.github.io/prefab/
[build.gradle]:https://github.com/android/ndk-samples/blob/master/prefab/prefab-publishing/mylibrary/build.gradle#L64


## Pre-requisites

* Android Gradle Plugin 4.1+
* The [Android NDK](https://developer.android.com/ndk/).

## Getting Started

The C++ code in this sample is built with CMake, but the concepts demonstrated
here work identically when using ndk-build.

To build:

1. Navigate to this directory in your terminal.
2. Run `./gradlew build` (or `gradlew.bat build` on Windows).

Note that there is no runnable application in this sample. The output is the
AAR.

## Support

If you've found an error in these samples, please [file an
issue](https://github.com/android/ndk-samples/issues/new).

Patches are encouraged, and may be submitted by submitting a pull request
through GitHub. Please see [CONTRIBUTING.md](../../CONTRIBUTING.md) for more
details.
