# Teapots

Teapots is an collection of Android C++ samples that uses a Teapot rendering to
demonstrate Android NDK platform features:

- classic-teapot: Rendering classic teapot mesh using GLES 2.0 API and
  [NativeActivity](http://developer.android.com/reference/android/app/NativeActivity.html).
- more-teapots: Rendering multiple instances of Classic Teapot with GLES 3.0
  Instance Rendering
- choreographer-30fps: demonstrates multiple frame rate throttling techniques
  based on API level using Choreographer API and EGL Android presentation time
  extension.
- textured-teapot: Rendering classic teapot plus textures
- image-decoder: Same as textured-teapot, except that texture decoding is done
  with the ImageDecoder API introduced in Android 11 (Android NDK r21b)

## Screenshots

![screenshot](screenshot.png)
