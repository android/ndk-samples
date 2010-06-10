package com.example.TwoLib;

import android.test.ActivityInstrumentationTestCase;

/**
 * This is a simple framework for a test of an Application.  See
 * {@link android.test.ApplicationTestCase ApplicationTestCase} for more information on
 * how to write and extend Application tests.
 * <p/>
 * To run this test, you can type:
 * adb shell am instrument -w \
 * -e class com.example.TwoLib.TwoLibTest \
 * com.example.TwoLib.tests/android.test.InstrumentationTestRunner
 */
public class TwoLibTest extends ActivityInstrumentationTestCase<TwoLib> {

    public TwoLibTest() {
        super("com.example.TwoLib", TwoLib.class);
    }

}
