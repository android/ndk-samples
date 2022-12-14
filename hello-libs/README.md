# Pre-compiled Libraries

This sample shows you how to use pre-compiled 3rd party C/C++ libraries in your
app.

## Introduction

This sample uses [CMake](https://developer.android.com/ndk/guides/cmake) with
external library support. It shows how to include pre-built static libraries
(gmath and gperf) in your app.

## Description

The sample includes 2 modules:

- **app** -- imports a shared library (libgperf.so) and a static library
  (libgmath.a) from the `distribution` folder.
- **gen-libs** -- contains the source code and CMake build script for the gmath
  and gperf example libraries. The resulting binaries are copied into the
  `distribution` folder. By default, the gen-libs module is disabled in
  settings.gradle and app/build.gradle, so it won't show up in Android Studio
  IDE. If re-generating lib is desirable, follow comments inside settings.gradle
  and app/build.gradle to enable this module, generate libs, then disable it
  again to avoid unnecessary confusion.

The main goal of the sample is to show how to use pre-built 3rd party libraries,
not to demonstrate how to build them. Thus, the pre-built libs are included in
the `distribution` folder.

When importing libraries into your app, include the following in your app's
`CMakeLists.txt` file (in the following order):

- Import libraries as static or shared (using `add_library`).
- Configure each library binary location (using `set_target_properties` and
  `${ANDROID_ABI}`).
- Configure each library headers location (using `target_include_directories`).

For example, from \[app/src/main/cpp/CMakeLists.txt\]:

```cmake
add_library(lib_gmath STATIC IMPORTED)
set_target_properties(lib_gmath PROPERTIES IMPORTED_LOCATION
    ${distribution_DIR}/gmath/lib/${ANDROID_ABI}/libgmath.a)

add_library(lib_gperf SHARED IMPORTED)
set_target_properties(lib_gperf PROPERTIES IMPORTED_LOCATION
    ${distribution_DIR}/gperf/lib/${ANDROID_ABI}/libgperf.so)

target_include_directories(hello-libs PRIVATE
                           ${distribution_DIR}/gmath/include
                           ${distribution_DIR}/gperf/include)
```

## Re-compiling the libraries

To regenerate the two libraries, follow these steps:

1. In \[settings.gradle\], enable the line `include :gen-libs`.
1. In \[app/build.gradle\], enable the line `api project(':gen-libs')`.
1. Build the project.
1. Undo the first 2 steps.
1. Clean up temporary build files with `rm -fr app/build app/.cxx`.

## Screenshots

![screenshot](screenshot.png)

## Support

If you've found an error in these samples, please
[file an issue](https://github.com/googlesamples/android-ndk/issues/new).

Patches are encouraged, and may be submitted by
[forking this project](https://github.com/googlesamples/android-ndk/fork) and
submitting a pull request through GitHub. Please see
[CONTRIBUTING.md](../CONTRIBUTING.md) for more details.

- [Stack Overflow](http://stackoverflow.com/questions/tagged/android-ndk)
- [Android Tools Feedbacks](http://tools.android.com/feedback)

## License

Copyright 2015 Google, Inc.

Licensed to the Apache Software Foundation (ASF) under one or more contributor
license agreements. See the NOTICE file distributed with this work for
additional information regarding copyright ownership. The ASF licenses this file
to you under the Apache License, Version 2.0 (the "License"); you may not use
this file except in compliance with the License. You may obtain a copy of the
License at

http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software distributed
under the License is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
CONDITIONS OF ANY KIND, either express or implied. See the License for the
specific language governing permissions and limitations under the License.
