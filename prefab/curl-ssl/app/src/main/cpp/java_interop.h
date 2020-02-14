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

#pragma once

#include <jni.h>

#include <cstdlib>
#include <string>
#include <vector>

#include "logging.h"

namespace curlssl {
namespace jni {

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
struct Convert<jobjectArray, jstring> {
  static jobjectArray from(JNIEnv* env, const std::vector<std::string>& value) {
    const char stringClassName[] = "java/lang/String";
    jclass stringClass = env->FindClass(stringClassName);
    if (stringClass == nullptr) {
      logging::FatalError(env, "%s: FindClass(\"%s\") failed", __func__,
                          stringClassName);
    }

    jobjectArray array =
        env->NewObjectArray(value.size(), stringClass, nullptr);
    if (array == nullptr) {
      logging::FatalError(env, "%s: NewObjectArray failed", __func__);
    }
    for (size_t i = 0; i < value.size(); ++i) {
      jstring str = env->NewStringUTF(value[i].c_str());
      if (str == nullptr) {
        logging::FatalError(env, "%s: NewStringUTF(\"%s\") failed", __func__,
                            value[i].c_str());
      }
      env->SetObjectArrayElement(array, i, str);
    }
    return array;
  }
};

}  // namespace jni
}  // namespace curlssl
