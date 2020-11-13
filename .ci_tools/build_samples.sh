#!/bin/bash

# - Configure SDK/NDK locations so we do not depends on local.properties, e.g
#   - export ANDROID_HOME=$HOME/dev/sdk
#   - export ANDROID_NDK_HOME=$ANDROID_HOME/ndk-bundle
# - executing in repo root directory

# Configurations:
#  temp file name to hold build result
BUILD_RESULT_FILE=build_result.txt

# Repo root directory
NDK_SAMPLE_REPO=.


declare projects=(
    audio-echo
    bitmap-plasma
    camera
    endless-tunnel
    gles3jni
    hello-gl2
    hello-jni
    hello-jniCallback
    hello-libs
    hello-neon
    native-activity
    native-audio
    native-codec
    native-media
    native-plasma
    nn-samples
    prefab/curl-ssl
    prefab/prefab-publishing
    san-angeles
    sensor-graph
#    webp
    teapots
##   ndk-build samples
    other-builds/ndkbuild/bitmap-plasma
    other-builds/ndkbuild/gles3jni
    other-builds/ndkbuild/hello-gl2
    other-builds/ndkbuild/hello-jni
    other-builds/ndkbuild/hello-libs
    other-builds/ndkbuild/hello-neon
    other-builds/ndkbuild/native-activity
    other-builds/ndkbuild/native-audio
    other-builds/ndkbuild/native-codec
    other-builds/ndkbuild/native-media
    other-builds/ndkbuild/native-plasma
    other-builds/ndkbuild/nn-samples
    other-builds/ndkbuild/san-angeles
    other-builds/ndkbuild/teapots
    )

for d in "${projects[@]}"; do 
    pushd ${NDK_SAMPLE_REPO}/${d} >/dev/null
    TERM=dumb ./gradlew  -q clean assembleDebug
    popd >/dev/null
done


# Check the apks that all get built fine
declare apks=(
  audio-echo/app/build/outputs/apk/debug/app-debug.apk
  bitmap-plasma/app/build/outputs/apk/debug/app-debug.apk
  camera/basic/build/outputs/apk/debug/basic-debug.apk
  camera/texture-view/build/outputs/apk/debug/texture-view-debug.apk
  endless-tunnel/app/build/outputs/apk/debug/app-debug.apk
  gles3jni/app/build/outputs/apk/debug/app-debug.apk
  hello-gl2/app/build/outputs/apk/debug/app-debug.apk
  hello-jni/app/build/outputs/apk/arm8/debug/app-arm8-debug.apk
  hello-jniCallback/app/build/outputs/apk/debug/app-debug.apk
  hello-libs/app/build/outputs/apk/debug/app-debug.apk
  hello-neon/app/build/outputs/apk/debug/app-debug.apk
  native-activity/app/build/outputs/apk/debug/app-debug.apk
  native-audio/app/build/outputs/apk/debug/app-debug.apk
  native-codec/app/build/outputs/apk/debug/app-debug.apk
  native-media/app/build/outputs/apk/debug/app-debug.apk
  native-plasma/app/build/outputs/apk/debug/app-debug.apk
  nn-samples/basic/build/outputs/apk/debug/basic-debug.apk
  nn-samples/sequence/build/outputs/apk/debug/sequence-debug.apk
  prefab/curl-ssl/app/build/outputs/apk/debug/app-debug.apk
  prefab/prefab-publishing/mylibrary/build/outputs/aar/mylibrary-debug.aar
  sensor-graph/accelerometer/build/outputs/apk/debug/accelerometer-debug.apk
  san-angeles/app/build/outputs/apk/debug/app-armeabi-v7a-debug.apk
  san-angeles/app/build/outputs/apk/debug/app-arm64-v8a-debug.apk
  san-angeles/app/build/outputs/apk/debug/app-x86-debug.apk
  teapots/classic-teapot/build/outputs/apk/debug/classic-teapot-debug.apk
  teapots/more-teapots/build/outputs/apk/debug/more-teapots-debug.apk
  teapots/choreographer-30fps/build/outputs/apk/debug/choreographer-30fps-debug.apk
  teapots/image-decoder/build/outputs/apk/debug/image-decoder-debug.apk
#    webp/view/build/outputs/apk/debug/view-arm7-debug.apk

## other-builds
  other-builds/ndkbuild/bitmap-plasma/app/build/outputs/apk/debug/app-debug.apk
  other-builds/ndkbuild/gles3jni/app/build/outputs/apk/debug/app-debug.apk
  other-builds/ndkbuild/hello-gl2/app/build/outputs/apk/debug/app-debug.apk
  other-builds/ndkbuild/hello-jni/app/build/outputs/apk/debug/app-debug.apk
  other-builds/ndkbuild/hello-libs/app/build/outputs/apk/debug/app-debug.apk
  other-builds/ndkbuild/hello-neon/app/build/outputs/apk/arm7/debug/app-arm7-debug.apk
  other-builds/ndkbuild/native-activity/app/build/outputs/apk/debug/app-debug.apk
  other-builds/ndkbuild/native-audio/app/build/outputs/apk/debug/app-debug.apk
  other-builds/ndkbuild/native-codec/app/build/outputs/apk/debug/app-debug.apk
  other-builds/ndkbuild/native-media/app/build/outputs/apk/debug/app-debug.apk
  other-builds/ndkbuild/native-plasma/app/build/outputs/apk/debug/app-debug.apk
  other-builds/ndkbuild/nn-samples/basic/build/outputs/apk/debug/basic-debug.apk
  other-builds/ndkbuild/san-angeles/app/build/outputs/apk/debug/app-armeabi-v7a-debug.apk
  other-builds/ndkbuild/san-angeles/app/build/outputs/apk/debug/app-arm64-v8a-debug.apk
  other-builds/ndkbuild/san-angeles/app/build/outputs/apk/debug/app-x86-debug.apk
  other-builds/ndkbuild/teapots/more-teapots/build/outputs/apk/debug/more-teapots-debug.apk
  other-builds/ndkbuild/teapots/classic-teapot/build/outputs/apk/debug/classic-teapot-debug.apk
)

rm -fr ${BUILD_RESULT_FILE}
for apk in "${apks[@]}"; do
  if [ ! -f ${NDK_SAMPLE_REPO}/${apk} ]; then
    export SAMPLE_CI_RESULT=1
    echo ${apk} does not build >> ${BUILD_RESULT_FILE}
  fi
done

if [ -f ${BUILD_RESULT_FILE} ]; then
   echo  "******* Failed Builds ********:"
   cat  ${BUILD_RESULT_FILE}
else
  echo "======= BUILD SUCCESS ======="
fi

rm -fr ${BUILD_RESULT_FILE}

