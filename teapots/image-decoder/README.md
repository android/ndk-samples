image-decoder
==============
This sample demonstrates the [ImageDecoder](https://developer.android.com/ndk/guides/image-decoder) functionality in Android 11:
- Texture files are decoded with AImageDecoder
- The rest of the code is the same as that of TexturedTeapot

Please note that the sample uses its own wrapper to access the AImageDecoder API introduced in Android 11 due to a known issue between the Android 11 Developer Preview SDK and NDK version r-21b. The sample will be updated to use headers and libraries exposed from the next NDK-r21b release
around the [time of the next Developer Preview build](https://developer.android.com/preview/overview#timeline).

NDK-r21b does include all necessary headers and libraries with full documentation for [AImageDecoder](https://developer.android.com/ndk/reference/group/image-decoder). You can preview its full functionality and even
build apps outside the Android Gradle Plugin environment.

Screenshots
-----------
![screenshot](screenshot.png)

