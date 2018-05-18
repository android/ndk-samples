#!/bin/bash
set +e

MISC_STATUS=0
# check that all Support section of the README are the same.
for f in */README.md; do
  sed -n '/Support/,/License/p' $f > /tmp/$(dirname $f).readme;
done && diff -u --from-file=/tmp/hello-jni.readme /tmp/*.readme
MISC_STATUS=$(($MISC_STATUS + $?))

# check that all targetSdkVersion are 26+
# test "$(grep -H targetSdkVersion */app/build.gradle | tee /dev/stderr | cut -d= -f 2 | xargs -n1 echo | sort | uniq | wc -l)" = "2"
# check that there is no tabs in AndroidManifest
(! grep -n $'\t' */*/src/main/AndroidManifest.xml) | cat -t;
MISC_STATUS=$(($MISC_STATUS + ${PIPESTATUS[0]}))

# check that there is no trailing spaces in AndroidManifest
(! grep -E '\s+$' */*/src/main/AndroidManifest.xml) | cat -e;
MISC_STATUS=$(($MISC_STATUS + ${PIPESTATUS[0]}))

## Fix the builder then enable it [TBD]
#(cd builder && ./gradlew test)
# print build failure summary
# pandoc builder/build/reports/tests/index.html -t plain | sed -n '/^Failed tests/,/default-package/p'
# print lint results details
# for f in */app/build/outputs/lint-results.html; do pandoc $f -t plain; done

# populate the error to final status
if [[ "$MISC_STATUS" -ne 0 ]]; then
    SAMPLE_CI_RESULT=$(($SAMPLE_CI_RESULT + 1))
fi
