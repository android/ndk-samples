#!/bin/sh

# Guess ANDROID_HOME if it's not set, and also detect arch.
case $(uname -s) in
  Linux)
    ANDROID_HOME=${ANDROID_HOME:-${HOME}/Android/Sdk}
    ARCH_DIR=linux-x86_64
    ;;
  Darwin)
    ANDROID_HOME=${ANDROID_HOME:-${HOME}/Library/Android/sdk}
    ARCH_DIR=darwin-x86_64
esac

# Guess the latest version of the NDK we are using, and use that everywhere.
NDK_VERSION=$(find . -name build.gradle | xargs grep -h ndkVersion | sort -n | uniq | cut -d \' -f 2 | tail -1)

${ANDROID_HOME}/ndk/${NDK_VERSION}/toolchains/llvm/prebuilt/${ARCH_DIR}/bin/clang-format $@
