# Unit Test

This Android sample shows how to write unit tests for native code with
[googletest](https://github.com/google/googletest), and run them on-device.

## Writing unit tests

This app has a very simple library containing a function to add two numbers.

[adder.h](app/src/main/cpp/adder.h):

```C++
#pragma once

int add(int a, int b);
```

[adder.cpp](app/src/main/cpp/adder.cpp):

```C++
#include "adder.h"

int add(int a, int b) { return a + b; }
```

[adder_test.cpp](app/src/main/cpp/adder_test.cpp) contains a unit test for this
function:

```C++
#include "adder.h"

#include "gtest/gtest.h"

TEST(adder, adder) { EXPECT_EQ(3, add(1, 2)); }
```

## Building and running the tests

We need to add a library for the test to
[CMakeLists.txt](app/src/main/cpp/CMakeLists.txt), which depends on googletest
and junit-gtest:

```cmake
find_package(googletest REQUIRED CONFIG)
find_package(junit-gtest REQUIRED CONFIG)
```

```cmake
add_library(app_tests SHARED adder_test.cpp)
target_link_libraries(app_tests
  PRIVATE
    $<TARGET_OBJECTS:adder>
    googletest::gtest
    junit-gtest::junit-gtest
)
```

We need to add googletest and junit-gtest as dependencies in
[app/build.gradle](app/build.gradle). Googletest uses prefab, so we enable that.

```
    build_features {
    	prefab true
    }
```

```
dependencies {
    implementation 'androidx.test.ext:junit-gtest:1.0.0-alpha01'
    implementation 'com.android.ndk.thirdparty:googletest:1.11.0-beta-1'
}
```

Finally, we need a wrapper in androidTest,
[NativeTests.kt](app/src/androidTest/java/com/example/unittest/NativeTests.kt):

```kotlin
package com.example.unittest

import androidx.test.ext.junitgtest.GtestRunner
import androidx.test.ext.junitgtest.TargetLibrary
import org.junit.runner.RunWith

@RunWith(GtestRunner::class)
@TargetLibrary(libraryName = "app_tests")
class NativeTests
```

You can run the test on a phone or emulator as described in the
[Android developer documentation](https://developer.android.com/studio/test/test-in-android-studio).

Try deliberately breaking the test, like this:

```C++
EXPECT_EQ(4, add(1, 2));
```

You should see a failure message like this:

```
java.lang.AssertionError:
/path/to/ndk-samples/unit-test/app/src/main/cpp/adder_test.cpp:6
Expected equality of these values:
4
add(1,2)
Which is: 3
```

## Screenshot

![screenshot](screenshot.png)
