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

import android.app.Activity;
import android.view.View;
import android.widget.EditText;
import android.widget.TextView;
import android.os.Bundle;


public class GreetingJni extends Activity
{
    private EditText mEditText;
    private TextView mTextView;

    /** Called when the activity is first created. */
    @Override
    public void onCreate(Bundle savedInstanceState)
    {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.greeting);
        mEditText = (EditText) findViewById(R.id.editText);
        mTextView = (TextView) findViewById(R.id.greetingTextView);
    }

    public void sendName(View view) {
        /* The text is retrieved by calling a native
         * function and set into a TextView.
         */
        mTextView.setText(stringFromJNI(mEditText.getText().toString()));
    }

    /* A native method that is implemented by the
     * 'greeting-jni' native library, which is packaged
     * with this application.
     */
    public native String  stringFromJNI(String parameter);

    /* This is another native method declaration that is *not*
     * implemented by 'greeting-jni'. This is simply to show that
     * you can declare as many native methods in your Java code
     * as you want, their implementation is searched in the
     * currently loaded native libraries only the first time
     * you call them.
     *
     * Trying to call this function will result in a
     * java.lang.UnsatisfiedLinkError exception !
     */
    public native String  unimplementedStringFromJNI();

    /* this is used to load the 'greeting-jni' library on application
     * startup. The library has already been unpacked into
     * /data/data/com.example.greeting-jni/lib/libgreeting-jni.so at
     * installation time by the package manager.
     */
    static {
        System.loadLibrary("greeting-jni");
    }
}
