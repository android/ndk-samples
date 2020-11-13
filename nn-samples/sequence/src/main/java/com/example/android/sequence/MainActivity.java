/**
 * Copyright 2020 The Android Open Source Project
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

package com.example.android.sequence;

import android.app.Activity;
import android.os.AsyncTask;
import android.os.Bundle;
import android.util.Log;
import android.view.View;
import android.widget.Button;
import android.widget.EditText;
import android.widget.TextView;
import android.widget.Toast;

public class MainActivity extends Activity {
    // Used to load the 'native-lib' library on application startup.
    static { System.loadLibrary("sequence"); }

    private final String LOG_TAG = "NNAPI_SEQUENCE";
    private long modelHandle = 0;

    public native long initModel(float ratio);

    public native float compute(float initialValue, int steps, long modelHandle);

    public native void destroyModel(long modelHandle);

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        Button resetButton = findViewById(R.id.reset_button);
        resetButton.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                EditText ratioInput = findViewById(R.id.ratio_input);
                String ratioStr = ratioInput.getText().toString();
                if (ratioStr.isEmpty()) {
                    Toast.makeText(getApplicationContext(), "Invalid ratio!", Toast.LENGTH_SHORT)
                            .show();
                    return;
                }

                if (modelHandle != 0) {
                    destroyModel(modelHandle);
                    modelHandle = 0;
                }
                TextView ratioText = findViewById(R.id.ratio_text);
                TextView resultText = findViewById(R.id.result_text);
                ratioText.setText(ratioStr);
                resultText.setText(R.string.none);
                new InitModelTask().execute(Float.valueOf(ratioStr));
            }
        });

        Button computeButton = findViewById(R.id.compute_button);
        computeButton.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                if (modelHandle != 0) {
                    EditText initialValueInput = findViewById(R.id.initial_value_input);
                    EditText stepsInput = findViewById(R.id.steps_input);
                    String initialValueStr = initialValueInput.getText().toString();
                    String stepsStr = stepsInput.getText().toString();
                    if (initialValueStr.isEmpty() || stepsStr.isEmpty()) {
                        Toast.makeText(getApplicationContext(), "Invalid initial value or steps!",
                                     Toast.LENGTH_SHORT)
                                .show();
                        return;
                    }
                    new ComputeTask().execute(initialValueStr, stepsStr);
                } else {
                    Toast.makeText(getApplicationContext(), "Model has not been initialized!",
                                 Toast.LENGTH_SHORT)
                            .show();
                }
            }
        });
    }

    @Override
    protected void onDestroy() {
        if (modelHandle != 0) {
            destroyModel(modelHandle);
            modelHandle = 0;
        }
        super.onDestroy();
    }

    private class InitModelTask extends AsyncTask<Float, Void, Long> {
        @Override
        protected Long doInBackground(Float... inputs) {
            if (inputs.length != 1) {
                Log.e(LOG_TAG, "Incorrect number of input values");
                return 0L;
            }
            // Prepare the model in a separate thread.
            return initModel(inputs[0]);
        }

        @Override
        protected void onPostExecute(Long result) {
            modelHandle = result;
        }
    }

    private class ComputeTask extends AsyncTask<String, Void, Float> {
        @Override
        protected Float doInBackground(String... inputs) {
            if (inputs.length != 2) {
                Log.e(LOG_TAG, "Incorrect number of input values");
                return 0.0f;
            }
            // Reusing the same prepared model with different inputs.
            return compute(Float.valueOf(inputs[0]), Integer.valueOf(inputs[1]), modelHandle);
        }

        @Override
        protected void onPostExecute(Float result) {
            TextView tv = findViewById(R.id.result_text);
            tv.setText(String.valueOf(result));
        }
    }
}
