#!/bin/bash

# Purpose:  Generate libgmath.a and libgperf.so
#           Copy them into distribution directories
#              distribution/gmath
#              distribution/gperf
#           works for Mac X. windows batch file is not implemented
# Execution in Terminal:
#          source gen_lib.sh  ( or ./gen_lib.sh )

# Build 2 libs in terminal with  ndk-build tool DIRECTLY
ndk-build APP_BUILD_SCRIPT=./gmath/Android.mk  NDK_PROJECT_PATH=. NDK_APPLICATION_MK=./gmath/Application.mk
ndk-build APP_BUILD_SCRIPT=./gperf/Android.mk  NDK_PROJECT_PATH=. NDK_APPLICATION_MK=./gperf/Application.mk

# Distribute the libs
DEST_DIR=../distribution
declare supported_ABIs=( \
  armeabi        \
  armeabi-v7a    \
  arm64-v8a      \
  mips           \
  mips64        \
  x86            \
  x86_64         \
)

for abi in "${supported_ABIs[@]}"; do
  mkdir -p ${DEST_DIR}/gmath/lib/${abi} ${DEST_DIR}/gperf/lib/${abi}
  cp -f ./obj/local/${abi}/libgmath.a  ${DEST_DIR}/gmath/lib/${abi}/
  cp -f ./obj/local/${abi}/libgperf.so ${DEST_DIR}/gperf/lib/${abi}/  
done

# or we could copy it from lib directly for shared lib
# cp -fr libs ${DEST_DIR}/gperf/lib

mkdir -p ${DEST_DIR}/gmath/include  ${DEST_DIR}/gperf/include
JNI_SRC_ROOT=../../../../hello-libs/gen-libs/src/main/cpp
cp -f ${JNI_SRC_ROOT}/gmath/src/gmath.h  ${DEST_DIR}/gmath/include/
cp -f ${JNI_SRC_ROOT}/gperf/src/gperf.h  ${DEST_DIR}/gperf/include/

