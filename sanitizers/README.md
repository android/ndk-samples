# Sanitizers

This Android sample shows how to use sanitizers to detect problems in your
native code, such as memory errors and undefined behavior.

## Build variants

Using a sanitizer requires building your app differently. Your native code must
be compiled differently, and you may need to include a runtime library and a
wrapper script.

To accomplish this, we use a different build variant for each sanitizer. You may
wish to review the
[documentation](https://developer.android.com/studio/build/build-variants) and
[codelab](https://developer.android.com/codelabs/build-variants) for build
variants.

The runtime libraries and wrapper script are included with the NDK distribution,
but it hasn't been well-documented how to use them. By using some tricks in
[CMakeLists.txt](app/src/main/cpp/CMakeLists.txt), we can automatically grab
what we need from the NDK and make it available to the build.

## Hardware-assisted Address Sanitizer (hwasan)

The recommended way to detect memory errors such as use-after-free in native
code is with hwasan. It uses special hardware features on 64-bit ARM devices. It
is preferable to asan (below) because it has much less RAM overhead, and is
actively supported.

For more information, including how to flash a build of Android that supports
hwasan, refer to the Android developer
[guide](https://developer.android.com/ndk/guides/hwasan) and the
[clang documentation](https://clang.llvm.org/docs/HardwareAssistedAddressSanitizerDesign.html).

[native-lib.cpp](app/src/main/cpp/native-lib.cpp) contains a simple
use-after-free error:

```C++
int* foo = new int;
*foo = 3;
delete foo;
*foo = 4;
```

When you build and run the hwasan build variant, the sample app should
immediately crash, and the
[logcat panel](https://developer.android.com/studio/debug/logcat) in Android
Studio should contain information about the problem:

![hwasan](hwasan.png)

## Address Sanitizer (asan)

If you want to test on a 32-bit device, or prefer not to flash a custom build,
you can use the asan build variant instead. As described in the
[developer guide](https://developer.android.com/ndk/guides/asan), this requires
a runtime library and wrapper script, which this sample copies automatically
from the NDK distribution.

As with hwasan, the sample app should immediately crash, and you can look at the
logcat panel for information about the problem:

![asan](asan.png)

## Undefined Behavior Sanitizer (ubsan)

The undefined behavior sanitizer detects undefined behavior in your native code.
For more information on the types of problems it can detect, refer to the
[clang documentation](https://clang.llvm.org/docs/UndefinedBehaviorSanitizer.html).
It requires a runtime library, which the sample app copies automatically from
the NDK distribution.

[native-lib.cpp](app/src/main/cpp/native-lib.cpp) contains a signed integer
overflow error, which is undefined behavior:

```C++
int k = 0x7fffffff;
k += 1;
```

This triggers a crash when you build and run the ubsan variant, and the logcat
panel tells you what's going on:

![ubsan](ubsan.png)
