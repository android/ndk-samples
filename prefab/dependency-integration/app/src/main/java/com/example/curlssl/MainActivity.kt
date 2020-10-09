/*
 * Copyright (C) 2019 The Android Open Source Project
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

package com.example.curlssl

import android.app.ListActivity
import android.os.Bundle
import android.widget.ArrayAdapter
import java.io.FileOutputStream

class MainActivity : ListActivity() {
    private val cacert by lazy {
        val path = cacheDir.resolve("cacert.pem")
        assets.open("cacert.pem").copyTo(FileOutputStream(path))
        path
    }

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)

        listAdapter = ArrayAdapter<String>(
            this,
            android.R.layout.simple_list_item_1,
            getGerritChanges(cacert.path)
        )
    }

    private external fun getGerritChanges(cacert: String): Array<String>

    companion object {
        init {
            System.loadLibrary("app")
        }
    }
}
