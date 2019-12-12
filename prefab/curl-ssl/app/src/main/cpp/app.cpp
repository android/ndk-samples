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

#include <jni.h>

#include <sstream>
#include <string>

#include "http.h"
#include "java_interop.h"
#include "json/json.h"
#include "logging.h"

namespace curlssl {
namespace {

std::vector<std::string> get_change_titles(const std::string& cacert_path) {
  std::string error;
  auto result = http::Client(cacert_path)
                    .get(
                        "http://android-review.googlesource.com/changes/"
                        "?q=status:open&n=10",
                        &error);
  if (!result) {
    return {error.c_str()};
  }

  // Strip XSSI defense prefix:
  // https://gerrit-review.googlesource.com/Documentation/rest-api.html#output
  const std::string payload = result.value().substr(5);

  Json::Value root;
  std::istringstream(payload) >> root;
  std::vector<std::string> titles;
  for (const auto& change : root) {
    titles.push_back(change["subject"].asString());
  }
  return titles;
}

}  // namespace

extern "C" JNIEXPORT jobjectArray JNICALL
Java_com_example_curlssl_MainActivity_getGerritChanges(JNIEnv* env,
                                                       jobject /* this */,
                                                       jstring cacert_java) {
  if (cacert_java == nullptr) {
    logging::FatalError(env, "cacert argument cannot be null");
  }

  const std::string cacert =
      curlssl::jni::Convert<std::string>::from(env, cacert_java);
  return jni::Convert<jobjectArray, jstring>::from(env,
                                                   get_change_titles(cacert));
}

}  // namespace curlssl
