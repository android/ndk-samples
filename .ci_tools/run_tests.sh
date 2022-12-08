#!/bin/sh
set -e

VERBOSITY=--quiet
if [ -n "${RUNNER_DEBUG+1}" ] ; then
    VERBOSITY=--info
fi

pushd unit-test
TERM=dumb ./gradlew ${VERBOSITY} pixel2api30DebugAndroidTest
popd > /dev/null