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

#pragma once

#include <jni.h>

#include <cstdlib>
#include <string>
#include <vector>

#include "logging.h"

namespace jsonparse::jni {

template <typename... ToTypes>
struct Convert;

template <>
struct Convert<std::string> {
  static std::string from(JNIEnv* env, const jstring& value) {
    typedef std::unique_ptr<const char[], std::function<void(const char*)>>
        JniString;

    JniString cstr(env->GetStringUTFChars(value, nullptr), [=](const char* p) {
      env->ReleaseStringUTFChars(value, p);
    });

    if (cstr == nullptr) {
      logging::FatalError(env, "%s: GetStringUTFChars failed", __func__);
    }

    return cstr.get();
  }
};

template <>
struct Convert<jstring> {
  static jstring from(JNIEnv* env, const std::string& from) {
    return env->NewStringUTF(from.c_str());
  }
};

}  // namespace com.example.prefabdependency::jni
