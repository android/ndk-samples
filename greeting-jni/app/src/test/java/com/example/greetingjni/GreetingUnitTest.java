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

import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.powermock.core.classloader.annotations.PrepareForTest;
import org.powermock.modules.junit4.PowerMockRunner;

import static org.junit.Assert.assertTrue;
import static org.mockito.Matchers.anyString;
import static org.mockito.Mockito.doReturn;
import static org.mockito.Mockito.spy;
import static org.powermock.api.support.membermodification.MemberMatcher.method;
import static org.powermock.api.support.membermodification.MemberModifier.suppress;

/**
 * The intention of this class is to be a Unit Test
 * we don't want to test the system so we mock the calls
 * to the "greeting-jni" library
 */
@RunWith(PowerMockRunner.class)
@PrepareForTest(Greeter.class)
public class GreetingUnitTest {

    private Greeter mGreeter;
    private String mName;
    private String mGreeting;

    @Before
    public void setUp() {
        suppress(method(Greeter.class, "initLibrary"));
        // Real instance to call and test the real methods
        mGreeter = new Greeter();
        // spy the instance to intercept calls to native methods
        mGreeter = spy(mGreeter);
    }

    @Test
    public void greetingTest() {
        givenName("someName");
        whenGreeterSendNameToC();
        thenGreetingShouldContainName();
    }

    private void thenGreetingShouldContainName() {
        assertTrue(mGreeting.contains(mName));
    }

    private void whenGreeterSendNameToC() {
        // returning the same name and not calling the native method
        doReturn(mName).when(mGreeter).stringFromJNI(anyString());
        mGreeting = mGreeter.sendName(mName);
    }

    private void givenName(String someName) {
        mName = someName;
    }
}
