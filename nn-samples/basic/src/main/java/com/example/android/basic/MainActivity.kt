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
package com.example.android.basic

import android.app.Activity
import android.content.res.AssetManager
import android.os.Bundle
import android.widget.Toast
import com.example.android.basic.databinding.ActivityMainBinding
import kotlinx.coroutines.*

/*
  MainActivity to take care of UI and user inputs
 */
class MainActivity : Activity() {
    private var modelHandle = 0L

    /*
       3 JNI functions managing NN models, refer to basic/README.md
       for model structure
     */
    private external fun initModel(assetManager: AssetManager?, assetName: String?): Long
    private external fun startCompute(modelHandle: Long, input1: Float, input2: Float): Float
    private external fun destroyModel(modelHandle: Long)

    private lateinit var binding: ActivityMainBinding
    private val activityJob = Job()

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        binding = ActivityMainBinding.inflate(layoutInflater)
        setContentView(binding.root)
        CoroutineScope(Dispatchers.IO + activityJob).async(Dispatchers.IO) {
            modelHandle = this@MainActivity.initModel(assets, "model_data.bin")
        }

        binding.computButton.setOnClickListener {
            if (modelHandle == 0L) {
                Toast.makeText(applicationContext, "Model initializing, please wait",
                        Toast.LENGTH_SHORT).show()
            }

            if (binding.tensorSeed0.text.isNotEmpty() && binding.tensorSeed2.text.isNotEmpty()) {
                Toast.makeText(applicationContext, "Computing", Toast.LENGTH_SHORT).show()
                binding.computeResult.text = runBlocking {
                    val operand0 = binding.tensorSeed0.text.toString().toFloat()
                    val operand2 = binding.tensorSeed2.text.toString().toFloat()
                    startCompute(modelHandle, operand0, operand2).toString()
                }.toString()
            }
        }
    }

    override fun onDestroy() {
        activityJob.cancel()
        if (modelHandle != 0L) {
            destroyModel(modelHandle)
            modelHandle = 0
        }
        super.onDestroy()
    }

    companion object {
        init {
            System.loadLibrary("basic")
        }
    }
}

