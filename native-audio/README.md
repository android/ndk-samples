# Native Audio

Native Audio is an Android sample that plays and records sounds with the C++
OpenSLES API using JNI. The recorder / players created are not in fast audio
path.

This sample uses the new
[Android Studio CMake plugin](http://tools.android.com/tech-docs/external-c-builds)
with C++ support.

***Note that OpenSL ES is
[deprecated from Android 11](https://developer.android.com/preview/features#deprecate-opensl),
developers are recommended to use [Oboe](https://github.com/google/oboe) library
instead.***

## Screenshots

![screenshot](screenshot.png)

## Known Issues

- URI Player streaming is
  [broken](https://github.com/googlesamples/android-ndk/issues/229)
