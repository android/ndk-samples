#!/bin/bash

# Installs Android SDK, NDK, and CMake required to build the samples.

if [ -f ~/.android/repositories.cfg ]; then
  touch ~/.android/repositories.cfg
fi

for version in $(find . -name build.gradle | xargs grep -h compileSdk | sort | uniq | grep -o '[0-9]\+ *$'); do
    echo y | sdkmanager "platforms;android-${version}"
done

for version in $(find . -name build.gradle | xargs grep -h ndkVersion | sort | uniq | cut -d \' -f 2); do
    echo y | sdkmanager "ndk;${version}"
done

echo y | sdkmanager "cmake;3.18.1"
echo y | sdkmanager "cmake;3.22.1"
