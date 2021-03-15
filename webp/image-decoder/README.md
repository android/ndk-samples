# Image-Decoder
The sample demonstrates how to use the new AImageDecoder animated decoding API,
added in Android 12. The animated images such as webp, avif and other formats, are
decoded and copied into the native window to display them on the screen.

The goal of the sample is to collect feedback for new AImageDecoder API added in Android 12
and Android 11, please try it out and let us know your thoughts!

## Pre-requistites
- Android Gradle Plugin 4.1.2
- Platform SDK and NDK for Android 12
- An Android device running Android 12(API level 31)

## Getting Start
- build the sample and run it on an Android 12 device
- swap in new image files: drop them into src/main/assets/images, and add to imageFiles[] in image_viewer.cpp

## Support
Android 12, AImageDecoder API and this sample are at developer preview stage, you
may expect changes in the future Android 12 release cycles. If you find any error
or have any feedback, please [file an issue](https://github.com/googlesamples/android-ndk/issues/new).
Comments and patches are highly very welcome too!

## Copyright
Copyright 2021 Google, Inc.

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
