# Android NDK Samples

This repository contains sample apps that use the [Android NDK].

For an explanation of the layout of this repository, see
[ARCHITECTURE.md](ARCHITECTURE.md).

## Build and run

[![build](https://github.com/android/ndk-samples/actions/workflows/build.yml/badge.svg)](https://github.com/android/ndk-samples/actions)

1. Clone the repository
2. Open the whole project in Android Studio
3. Install CMake 4.1.0 via the SDK Manager (must be done manually until
   https://issuetracker.google.com/443137057 is fixed).
4. Select the sample you want to run in the top bar (you may need to sync gradle
   first)
5. Click the play button to run the sample

You can also build the samples from the command line if you prefer. Use
`./gradlew build` to build everything (if you're on Windows, use `.\gradlew.bat`
instead of `./gradlew`). For individual tasks, see `./gradlew tasks`. To see the
tasks for an individual sample, run the `tasks` task for that directory. For
example, `./gradlew :camera:basic:tasks` will show the tasks for the
`camera/basic` app.

## I just want something to copy from as a starting point

The samples in this repository are generally not a good starting point for a
production quality app. They aim to demonstrate individual NDK APIs, but often
make sacrifices to be succinct that make them unsuitable for a production app.
This is gradually changing, but for now you should not do this.

[Now in Android](https://github.com/android/nowinandroid/) is an excellent
resource for production quality apps in general, but does not touch on NDK-
specific issues. https://github.com/DanAlbert/ndk-app-template can help some
with that until this repository is able to.

You're most likely best served by using the New Project wizard in Android Studio
to create a new app, then using those resources and the samples here as a
reference. Android Studio's "Native C++" template is a good starting point for
typical applications that need to use some C++ via JNI. The "Game Activity"
template is a good starting point for game-like apps (that is, apps that do not
use the Android UI, but instead render their own UI using OpenGL or Vulkan).

## Best practices shown here

There are a few best practices shown throughout this repository that should be
explained, but there's no central place to discuss those in the code, so we'll
discuss those here.

### `RegisterNatives`

We prefer using `RegisterNatives` via `JNI_OnLoad` over the name-based matching
that Studio's New Project Wizard will typically create, e.g. JNI functions that
follow the pattern `JNIEXPORT void JNICALL Java_com_etc_ClassName_methodName`.
That approach to matching C/C++ functions to their Java `native` function (or
Kotlin `external fun`) makes for a shorter demo when there are only a small
number of functions, but it has a number of disadvantages:

1. Mistakes are uncovered late. An incorrectly named function will not cause an
   error until called by Java, so it's easier to ship bugs if your testing does
   not cover all JNI methods.
2. All JNI symbols must be exported, leading to more relocations, larger
   binaries, and less effective link-time optimization (LTO).
3. The first call to a JNI function will be slower than subsequent calls, as the
   Java call will first need to find a matching implementation.
4. If you want to rename the function in Java, you have to also rename the C/C++
   function.
5. Android Studio cannot auto-fix typos in your native function names, although
   it can auto-generate new ones.

Instead, most apps should use `RegisterNatives` via `JNI_OnLoad`. This performs
up-front registration of native methods with an explicit mapping. It takes a
little bit more up-front-work, but doing so counters all of the disadvantages
above:

1. Mistakes are caught early. An incorrectly named function will cause an error
   during library load.
2. The only symbol that typically must be exported is `JNI_OnLoad`. Some use
   cases may additionally need `ANativeActivty_OnCreate` or similar.
3. No lookup required for the first call to each native function, so performance
   overhead is consistent.
4. The Java or C/C++ functions can be renamed at will, and only the map needs to
   be updated to match.
5. Android Studio can warn you and auto-fix common mistakes in class names and
   function signatures.

## Additional documentation

- [Add Native Code to Your Project](https://developer.android.com/studio/projects/add-native-code.html)
- [Configure NDK for Android Studio/Gradle Plugin](https://developer.android.com/studio/projects/configure-agp-ndk)
- [CMake for NDK](https://developer.android.com/ndk/guides/cmake.html)

## Support

If you've found an issue with a sample and you know how to fix it, please
[send us a PR!](CONTRIBUTING.md).

If you need to report a bug, where it needs to be filed depends on the type of
issue:

- Problems with the samples themselves:
  https://github.com/googlesamples/android-ndk/issues
- Problems with the OS APIs: http://b.android.com (usually the Framework
  component)
- Problems with NDK (that is, the compiler):
  https://github.com/android/ndk/issues

For questions about using the NDK or the platform APIs, you can ask on:

- [The NDK mailing list](https://groups.google.com/g/android-ndk) (best if
  you're not sure where else to ask)
- The [Discussions](https://github.com/android/ndk-samples/discussions) tab of
  this repo (best for questions about the samples themselves)
- The NDK's [Discussions](https://github.com/android/ndk/discussions) (best for
  questions about the NDK compilers and build systems)
- [Stack Overflow](https://stackoverflow.com/questions/tagged/android)

## Additional NDK samples:

- [Google Play Game Samples with Android Studio](https://github.com/playgameservices/cpp-android-basic-samples)
- [Google Android Vulkan Tutorials](https://github.com/googlesamples/android-vulkan-tutorials)
- [Android Vulkan API Basic Samples](https://github.com/googlesamples/vulkan-basic-samples)
- [Android High Performance Audio](https://github.com/googlesamples/android-audio-high-performance)

## License

Copyright 2018 The Android Open Source Project, Inc.

Licensed to the Apache Software Foundation (ASF) under one or more contributor
license agreements. See the NOTICE file distributed with this work for
additional information regarding copyright ownership. The ASF licenses this file
to you under the Apache License, Version 2.0 (the "License"); you may not use
this file except in compliance with the License. You may obtain a copy of the
License at

https://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software distributed
under the License is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
CONDITIONS OF ANY KIND, either express or implied. See the License for the
specific language governing permissions and limitations under the License.

[LICENSE](LICENSE)

[Android NDK]: https://developer.android.com/ndk
