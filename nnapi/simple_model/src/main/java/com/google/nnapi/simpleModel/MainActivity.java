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
    int[] inputIds_ = {
                       R.id.adder1Operand2,
                       R.id.adder2Operand2
                      };
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
        final Button btCompute = findViewById(R.id.compute);
        btCompute.setOnClickListener(new Button.OnClickListener() {
            public void onClick(View v) {
                onCompute();
            }
        });

        tvSampleResult_ = findViewById(R.id.sampleResult);

        InitDrawModelUI();

        // Init simpleModel in JNI and compute with default inputs
        btCompute.setEnabled(false);
        new Runnable() {
            @Override
            public void run() {
                initSuccess_ = basicModelInit();
                runOnUiThread(new Runnable() {
                    // init failed, could not compute
                    @Override
                    public void run() {
                        if (!initSuccess_) {
                            ((TextView)findViewById(R.id.prompt))
                                    .setText("Error: BasicModel initialization failed");
                            return;
                        }
                        findViewById(R.id.compute).setEnabled(true);
                        float adder1ConstantInputVal =  basicModelGetAdder1ConstantValue();
                        ((TextView)findViewById(R.id.adder1Operand1)).setText("" + adder1ConstantInputVal);
                        float adder2ConstantInputVal = basicModelGetAdder2ConstantValue();
                        ((TextView)findViewById(R.id.adder2Operand1)).setText("" + adder2ConstantInputVal);
                        onCompute();
                    };
                });
            }
        }.run();
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
        new Runnable() {
            @Override
            public void run() {
                final float result = basicModelCompute(inputValues_);
                runOnUiThread(new Runnable() {
                    @Override
                    public void run() {
                        tvSampleResult_.setText("Result: " + result);
                    }
                });
            }
        }.run();

    }

    private void InitDrawModelUI() {
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

        int modelBoundViews[] = {
                R.id.prompt,  R.id.adder1Operand1,  // top horizontal line
                R.id.adder2Op, R.id.compute,        // bottom horizontal line
                R.id.adder1Operand2, R.id.adder1Operand1, // left vertical line
                R.id.mulOp, R.id.sampleResult,            // right vertical line
        };
        for (int idx = 0; idx < modelBoundViews.length - 1; idx += 2) {
            drawView.AddBoundingViews(findViewById(modelBoundViews[idx]),
                             findViewById(modelBoundViews[idx+1]));
        }
    }
    @Override
    protected void onDestroy() {
        super.onDestroy();
        basicModelFinish();
    }

    public native boolean  basicModelInit();
    public native float basicModelGetAdder1ConstantValue();
    public native float basicModelGetAdder2ConstantValue();
    public native float basicModelCompute(float[] inputData);
    public native boolean basicModelFinish();
}
