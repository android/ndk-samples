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

#include <jni.h>

#include <sstream>
#include <string>

#include "java_interop.h"
#include "json/json.h"

namespace jsonparse {

extern "C" JNIEXPORT jstring JNICALL
Java_com_example_prefabdependency_MainActivity_getJsonValue(JNIEnv *env,
                                                     jobject /* this */,
                                                     jstring jsonFromJava,
                                                     jstring keyFromJava) {
  if (jsonFromJava == nullptr) {
    logging::FatalError(env, "jsonFromJava argument cannot be null");
  }
  if (keyFromJava == nullptr) {
    logging::FatalError(env, "keyFromJava argument cannot be null");
  }

  const std::string json =
      jsonparse::jni::Convert<std::string>::from(env, jsonFromJava);
  const std::string key =
      jsonparse::jni::Convert<std::string>::from(env, keyFromJava);

  Json::Value root;
  std::istringstream(json) >> root;

  return jni::Convert<jstring>::from(env, root[key].asString());
}

}  // namespace com.example.prefabdependency
