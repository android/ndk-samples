/*
 * Copyright (C) 2020 The Android Open Source Project
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

package com.example.prefabdependency

import android.os.Bundle
import androidx.appcompat.app.AppCompatActivity
import com.example.prefabdependency.databinding.ActivityJsonParseBinding

class MainActivity : AppCompatActivity() {

    private lateinit var binding: ActivityJsonParseBinding

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        binding = ActivityJsonParseBinding.inflate(layoutInflater)
        val view = binding.root
        setContentView(view)
        binding.resultTextview.text = getJsonValue(
            """
            {
                "cpp": "clang",
                "aar": "Prefab",
                "package": "cmake",
                "app": "gradle"
            }
            """.trimIndent(), "aar"
        )

    }

    private external fun getJsonValue(jsonString: String, key: String): String

    companion object {
        init {
            System.loadLibrary("app")
        }
    }
}
