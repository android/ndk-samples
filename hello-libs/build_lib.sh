#!/bin/bash
pushd /Users/gfan/proj/android-ndk/hello-libs

gradlew  :gmath:clean :gmath:assemble :gperf:clean :gperf:assemble

mkdir  -p distribution/gmath/lib
mkdir  -p distribution/gmath/include
cp -fr gmath/build/outputs/native/release/lib  distribution/gmath/
cp -f  gmath/src/main/jni/*.h   distribution/gmath/include/

mkdir  -p distribution/gperf/lib
mkdir  -p distribution/gperf/include
cp -fr gperf/build/outputs/native/release/lib distribution/gperf/
cp -fr gperf/src/main/jni/*.h  distribution/gperf/include/

popd

