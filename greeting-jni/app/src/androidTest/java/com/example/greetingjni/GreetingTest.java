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

import android.support.test.rule.ActivityTestRule;
import android.support.test.runner.AndroidJUnit4;

import org.junit.Before;
import org.junit.Rule;
import org.junit.Test;
import org.junit.runner.RunWith;

import static android.support.test.espresso.Espresso.onView;
import static android.support.test.espresso.action.ViewActions.click;
import static android.support.test.espresso.action.ViewActions.closeSoftKeyboard;
import static android.support.test.espresso.action.ViewActions.typeText;
import static android.support.test.espresso.assertion.ViewAssertions.matches;
import static android.support.test.espresso.matcher.ViewMatchers.withId;
import static android.support.test.espresso.matcher.ViewMatchers.withText;
import static org.hamcrest.CoreMatchers.containsString;
import static org.junit.Assert.assertTrue;

/**
 * Basic tests showcasing simple view matchers and actions like {@link ViewMatchers#withId},
 * {@link ViewActions#click} and {@link ViewActions#typeText}.
 * <p>
 * Note that there is no need to tell Espresso that a view is in a different {@link Activity}.
 *
 * It loads the "greeting-jni" library to test,
 * and it is based on the ui/espresso/BasicSample that can be found here:
 * https://github.com/googlesamples/android-testing
 *
 */
@RunWith(AndroidJUnit4.class)
public class GreetingTest {

    public static final String STRING_TO_BE_TYPED = "Espresso";

    /**
     * A JUnit {@link Rule @Rule} to launch your activity under test. This is a replacement
     * for {@link ActivityInstrumentationTestCase2}.
     * <p>
     * Rules are interceptors which are executed for each test method and will run before
     * any of your setup code in the {@link Before @Before} method.
     * <p>
     * {@link ActivityTestRule} will create and launch of the activity for you and also expose
     * the activity under test. To get a reference to the activity you can use
     * the {@link ActivityTestRule#getActivity()} method.
     */
    @Rule
    public ActivityTestRule<GreetingJni> mActivityRule = new ActivityTestRule<>(
            GreetingJni.class);

    private Greeter mGreeter;
    private String mName;
    private String mGreeting;

    @Before
    public void initGreeter() throws Exception {
        mGreeter = new Greeter();
    }

    @Test
    public void sendGreeting_sameActivity() {
        // Type text and then press the button.
        onView(withId(R.id.editTextUserInput))
                .perform(typeText(STRING_TO_BE_TYPED), closeSoftKeyboard());
        onView(withId(R.id.sendNameToJniButton)).perform(click());

        // Check that the text was changed.
        onView(withId(R.id.greetingTextView)).check(matches(withText(containsString(STRING_TO_BE_TYPED))));
    }

    @Test
    public void shouldContainSomeNameWhenGreeterSendsName() {
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
