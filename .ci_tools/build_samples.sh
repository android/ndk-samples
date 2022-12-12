#!/bin/bash

# - Configure SDK/NDK locations so we do not depends on local.properties, e.g
#   - export ANDROID_HOME=$HOME/dev/sdk
#   - export ANDROID_NDK_HOME=$ANDROID_HOME/ndk-bundle
# - executing in repo root directory

declare projects=(
    audio-echo
    bitmap-plasma
    camera
    display-p3
    endless-tunnel
    exceptions
    gles3jni
    hello-gl2
    hello-vulkan
    hello-jni
    hello-jniCallback
    hello-libs
    hello-neon
    hello-oboe
    native-activity
    native-audio
    native-codec
    native-media
    native-midi
    native-plasma
    nn-samples
    prefab/curl-ssl
    prefab/prefab-dependency
    prefab/prefab-publishing
    san-angeles
    sensor-graph
    teapots
    unit-test
    webp
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
    other-builds/ndkbuild/two-libs
    )

LINT_FAILURES=$(mktemp)
BUILD_FAILURES=$(mktemp)
APK_FAILURES=$(mktemp)

VERBOSITY=--quiet
if [ -n "${RUNNER_DEBUG+1}" ] ; then
    VERBOSITY=--info
fi
for d in "${projects[@]}"; do
  pushd ${d} >/dev/null
  echo "Building ${d}"
  if ! TERM=dumb ./gradlew ${VERBOSITY} clean lint ; then
    export SAMPLE_CI_RESULT=1
    echo ${d} >> ${LINT_FAILURES}
  fi
  if ! TERM=dumb ./gradlew ${VERBOSITY} assembleDebug ; then
    export SAMPLE_CI_RESULT=1
    echo ${d} >> ${BUILD_FAILURES}
  fi
  popd >/dev/null
done


# Check the apks that all get built fine
declare apks=(
  audio-echo/app/build/outputs/apk/debug/app-debug.apk
  bitmap-plasma/app/build/outputs/apk/debug/app-debug.apk
  camera/basic/build/outputs/apk/debug/basic-debug.apk
  camera/texture-view/build/outputs/apk/debug/texture-view-debug.apk
  display-p3/image-view/build/outputs/apk/debug/image-view-debug.apk
  endless-tunnel/app/build/outputs/apk/debug/app-debug.apk
  exceptions/app/build/outputs/apk/debug/app-debug.apk
  gles3jni/app/build/outputs/apk/debug/app-debug.apk
  hello-gl2/app/build/outputs/apk/debug/app-debug.apk
  hello-vulkan/app/build/outputs/apk/debug/app-debug.apk
  hello-jni/app/build/outputs/apk/debug/app-debug.apk
  hello-jniCallback/app/build/outputs/apk/debug/app-debug.apk
  hello-libs/app/build/outputs/apk/debug/app-debug.apk
  hello-neon/app/build/outputs/apk/debug/app-debug.apk
  hello-oboe/app/build/outputs/apk/debug/app-debug.apk
  native-activity/app/build/outputs/apk/debug/app-debug.apk
  native-audio/app/build/outputs/apk/debug/app-debug.apk
  native-codec/app/build/outputs/apk/debug/app-debug.apk
  native-media/app/build/outputs/apk/debug/app-debug.apk
  native-midi/app/build/outputs/apk/debug/app-debug.apk
  native-plasma/app/build/outputs/apk/debug/app-debug.apk
  nn-samples/basic/build/outputs/apk/debug/basic-debug.apk
  nn-samples/sequence/build/outputs/apk/debug/sequence-debug.apk
  prefab/curl-ssl/app/build/outputs/apk/debug/app-debug.apk
  prefab/prefab-publishing/mylibrary/build/outputs/aar/mylibrary-debug.aar
  prefab/prefab-dependency/app/build/outputs/apk/debug/app-debug.apk
  sensor-graph/accelerometer/build/outputs/apk/debug/accelerometer-debug.apk
  san-angeles/app/build/outputs/apk/debug/app-armeabi-v7a-debug.apk
  san-angeles/app/build/outputs/apk/debug/app-arm64-v8a-debug.apk
  san-angeles/app/build/outputs/apk/debug/app-x86-debug.apk
  teapots/classic-teapot/build/outputs/apk/debug/classic-teapot-debug.apk
  teapots/more-teapots/build/outputs/apk/debug/more-teapots-debug.apk
  teapots/choreographer-30fps/build/outputs/apk/debug/choreographer-30fps-debug.apk
  teapots/image-decoder/build/outputs/apk/debug/image-decoder-debug.apk
  unit-test/app/build/outputs/apk/debug/app-debug.apk
  webp/view/build/outputs/apk/debug/view-debug.apk

## other-builds
  other-builds/ndkbuild/bitmap-plasma/app/build/outputs/apk/debug/app-debug.apk
  other-builds/ndkbuild/gles3jni/app/build/outputs/apk/debug/app-debug.apk
  other-builds/ndkbuild/hello-gl2/app/build/outputs/apk/debug/app-debug.apk
  other-builds/ndkbuild/hello-jni/app/build/outputs/apk/debug/app-debug.apk
  other-builds/ndkbuild/hello-libs/app/build/outputs/apk/debug/app-debug.apk
  other-builds/ndkbuild/hello-neon/app/build/outputs/apk/debug/app-debug.apk
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
  other-builds/ndkbuild/two-libs/app/build/outputs/apk/debug/app-debug.apk
)

for apk in "${apks[@]}"; do
  if [ ! -f ${apk} ]; then
    export SAMPLE_CI_RESULT=1
    echo ${apk} >> ${APK_FAILURES}
  fi
done

if [ -s ${LINT_FAILURES} ]; then
  echo
  echo  "******* Lint failures ********"
  cat  ${LINT_FAILURES}
fi

if [ -s ${BUILD_FAILURES} ]; then
  echo
  echo  "******* Build failures ********"
  cat  ${BUILD_FAILURES}
fi

if [ -s ${APK_FAILURES} ]; then
  echo
  echo  "******* Missing APKs ********"
  cat  ${APK_FAILURES}
fi

if [ ! -s ${LINT_FAILURES} ] && [ ! -s ${BUILD_FAILURES} ] && [ ! -s ${APK_FAILURES} ]; then
  echo
  echo "======= BUILD SUCCESS ======="
fi

rm -f ${LINT_FAILURES} ${BUILD_FAILURES} ${APK_FAILURES}

