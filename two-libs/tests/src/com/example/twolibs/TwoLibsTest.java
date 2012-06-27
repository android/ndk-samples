package com.example.twolibs;

import android.test.ActivityInstrumentationTestCase;

/**
 * This is a simple framework for a test of an Application.  See
 * {@link android.test.ApplicationTestCase ApplicationTestCase} for more information on
 * how to write and extend Application tests.
 * <p/>
 * To run this test, you can type:
 * adb shell am instrument -w \
 * -e class com.example.twolibs.TwoLibsTest \
 * com.example.twolibs.tests/android.test.InstrumentationTestRunner
 */
public class TwoLibsTest extends ActivityInstrumentationTestCase<TwoLibs> {

    public TwoLibsTest() {
        super("com.example.twolibs", TwoLibs.class);
    }

}
