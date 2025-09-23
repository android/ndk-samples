# Hello Oboe

Hello Oboe is an Android sample that uses the
[Oboe](https://github.com/google/oboe) library to play audio natively. Oboe
wraps both AAudio and OpenSLES to help developers play and record audio in
performance sensitive contexts across Android versions. The Oboe repository has
more information about how to fully take advantage of the Oboe API, as well as
more in-depth examples and documentation.

This sample uses the Studio Prefab feature to download pre-built Oboe library
from [Google Maven](https://maven.google.com/web/index.html), feel free to
checkout Prefab steps in

- gradle.properties
- app/build.gradle
- app/src/main/cpp/CMakeLists.txt

If you like to build your app with the Oboe library source code, refer to the
examples in the Oboe repository.

## Screenshot

![screenshot](screenshot.png)

## Using the App

Tap and hold the screen to play audio. The app will render a 440Hz sine wave
while the screen is being pressed.
