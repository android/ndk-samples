/*
 * Copyright 2017 The Android Open Source Project
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
package com.google.nnapi.simpleModel;

import android.app.Activity;
import android.os.Bundle;
import android.text.TextUtils;
import android.view.View;
import android.view.Window;
import android.view.WindowManager;
import android.widget.Button;
import android.widget.EditText;
import android.widget.TextView;

public class MainActivity extends Activity {
    // just to cap the value to avoid overflow from input
    final float kMaxValue = 1000.0f;
    int[] inputIds_ = { R.id.adder1Operand1,
                       R.id.adder1Operand2,
                       R.id.adder2Operand1,
                       R.id.adder2Operand2};
    EditText [] inputViews_;
    float[] inputValues_;

    TextView tvSampleResult_;
    boolean  initSuccess_;

    static {
        System.loadLibrary("simpleModel");
    }

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        this.requestWindowFeature(Window.FEATURE_NO_TITLE);

        this.getWindow().setFlags(WindowManager.LayoutParams.FLAG_FULLSCREEN,
                WindowManager.LayoutParams.FLAG_FULLSCREEN);

        setContentView(R.layout.activity_main);

        inputViews_ = new EditText[inputIds_.length];
        inputValues_ = new float[inputIds_.length];

        for (int idx = 0; idx < inputIds_.length; idx++) {
            inputViews_[idx] = findViewById(inputIds_[idx]);
        }
        Button btCompute = findViewById(R.id.compute);
        btCompute.setOnClickListener(new Button.OnClickListener() {
            public void onClick(View v) {
                onCompute();
            }
        });

        tvSampleResult_ = findViewById(R.id.sampleResult);

        DrawView drawView = findViewById(R.id.drawView);
        int operandLines[] = {
                R.id.adder1Operand1, R.id.adder1Op,
                R.id.adder1Operand2, R.id.adder1Op,
                R.id.adder2Operand1, R.id.adder2Op,
                R.id.adder2Operand2, R.id.adder2Op,
                R.id.adder1Op,       R.id.mulOp,
                R.id.adder2Op,       R.id.mulOp,
                R.id.mulOp,          R.id.sampleResult,
        };
        for (int idx = 0; idx < operandLines.length - 1; idx += 2) {
            drawView.AddLine(findViewById(operandLines[idx]),
                    findViewById(operandLines[idx+1]));
        }

        // init simpleModel in JNI and compute with default inputs
        initSuccess_ = basicModeInit();
        if(initSuccess_) {
            onCompute();
        } else {
            // init failed, could not compute
            btCompute.setEnabled(false);
            ((TextView)findViewById(R.id.prompt))
                    .setText("Error: BasicModel initialization failed");
        }
    }

    private void onCompute() {
        for (int idx = 0; idx < inputIds_.length; idx++) {
            String inputStr = inputViews_[idx].getText().toString();
            if (TextUtils.isEmpty(inputStr)) {
                inputValues_[idx]  = 0.0f;
            } else {
                inputValues_[idx] = Float.parseFloat(inputStr.trim());
                if(inputValues_[idx] > kMaxValue) {
                    inputValues_[idx] = kMaxValue;
                    inputViews_[idx].setText(String.valueOf(kMaxValue));
                }
            }
        }

        if(basicModelCompute(inputValues_) == false) {
            findViewById(R.id.compute).setEnabled(false);
        }
    }

    // called by jni code...
    public void UpdateResult(float res) {
        final float result = res;
        runOnUiThread(new Runnable() {
            @Override
            public void run() {
                tvSampleResult_.setText("Result: " + result);
            }
        });
    }
    @Override
    protected void onDestroy() {
        super.onDestroy();
        basicModelFinish();
    }

    public native boolean  basicModeInit();
    public native boolean basicModelCompute(float[] inputData);
    public native boolean basicModelFinish();
}
