/*
 * Copyright (C) 2009 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
package com.example.greetingjni;

import android.support.test.runner.AndroidJUnit4;
import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;

import static org.junit.Assert.assertTrue;

/**
 * This test uses Instrumentation Tests
 * It loads the "greeting-jni" library to test
 * with System dependencies.
 */
@RunWith(AndroidJUnit4.class)
public class GreetingTest {

    private Greeter mGreeter;
    private String mName;
    private String mGreeting;

    @Before
    public void initGreeter() throws Exception {
        mGreeter = new Greeter();
    }

    @Test
    public void shouldContainSomeNameThenGreeterSendsName() {
        givenName("someName");
        whenGreeterSendNameToC();
        thenGreetingShouldContainName();
    }

    @Test(expected = java.lang.UnsatisfiedLinkError.class)
    public void shouldThrowExceptionForUnimplementedJniMethod() {
        whenGreeterCallUnimplementedJniMethod();
    }

    private void whenGreeterCallUnimplementedJniMethod() {
        mGreeter.callUnimplementedJniMethod();
    }

    private void givenName(String someName) {
        mName = someName;
    }

    private void whenGreeterSendNameToC() {
        mGreeting = mGreeter.sendName(mName);
    }

    private void thenGreetingShouldContainName() {
        assertTrue(mGreeting.contains(mName));
    }
}
