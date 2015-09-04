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
    private Greeter mGreeter;

    /** Called when the activity is first created. */
    @Override
    public void onCreate(Bundle savedInstanceState)
    {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.greeting);
        mEditText = (EditText) findViewById(R.id.editText);
        mTextView = (TextView) findViewById(R.id.greetingTextView);
        mGreeter = new Greeter();
    }

    public void sendName(View view) {
        /* The text is retrieved by calling a native
         * function in Greeter class and set the result into a TextView.
         */
        final String greeting = mGreeter.sendName(mEditText.getText().toString());
        mTextView.setText(greeting);
    }

}
