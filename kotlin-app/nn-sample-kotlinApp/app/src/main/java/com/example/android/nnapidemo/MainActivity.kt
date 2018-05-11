/**
 * Copyright 2017 The Android Open Source Project
 *
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

package com.example.android.nnapidemo

import android.app.Activity
import android.content.res.AssetManager
import android.os.AsyncTask
import android.os.Bundle
import android.util.Log
import android.view.View
import android.widget.Button
import android.widget.EditText
import android.widget.TextView
import android.widget.Toast

class MainActivity : Activity() {

    private val LOG_TAG = "NNAPI_DEMO"
    private var modelHandle = 0L

    external fun initModel(assetManager: AssetManager, assetName: String): Long

    external fun startCompute(modelHandle: Long, input1: Float, input2: Float): Float

    external fun destroyModel(modelHandle: Long)

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(R.layout.activity_main)

        InitModelTask().execute("model_data.bin")

        val compute = findViewById<View>(R.id.button) as Button
        compute.setOnClickListener {
            if (modelHandle != 0L) {
                val inputValue1 = findViewById<EditText>(R.id.inputValue1).text.toString()
                val inputValue2 = findViewById<EditText>(R.id.inputValue2).text.toString()
                if (inputValue1.isNotEmpty() && inputValue2.isNotEmpty()) {
                    Toast.makeText(applicationContext, "Computing",
                            Toast.LENGTH_SHORT).show()
                    ComputeTask().execute(inputValue1.toFloat(), inputValue2.toFloat())
                }
            } else {
                Toast.makeText(applicationContext, "Model initializing, please wait",
                        Toast.LENGTH_SHORT).show()
            }
        }
    }

    override fun onDestroy() {
        if (modelHandle != 0L) {
            destroyModel(modelHandle)
            modelHandle = 0
        }
        super.onDestroy()
    }

    private inner class InitModelTask : AsyncTask<String, Void, Long>() {
        override fun doInBackground(vararg modelName: String): Long? {
            if (modelName.size != 1) {
                Log.e(LOG_TAG, "Incorrect number of model files")
                return 0L
            }
            // Prepare the model in a separate thread.
            return initModel(assets, modelName[0])
        }

        override fun onPostExecute(result: Long?) {
            modelHandle = result!!
        }
    }

    private inner class ComputeTask : AsyncTask<Float, Void, Float>() {
        override fun doInBackground(vararg inputs: Float?): Float {
            if (inputs.size != 2) {
                Log.e(LOG_TAG, "Incorrect number of input values")
                return 0.0f
            }

            val first = inputs[0] ?: return 0.0f
            val second = inputs[1] ?: return 0.0f

            return startCompute(modelHandle, first, second)
        }

        override fun onPostExecute(result: Float?) {
            val tv = findViewById<TextView>(R.id.textView)
            tv.text = result.toString()
        }
    }
    companion object {
        // Used to load the 'nn_sample' library on application startup.
        init {
            System.loadLibrary("nn_sample")
        }
    }
}
