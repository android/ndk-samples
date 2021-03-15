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

#include "http.h"

#include <memory>
#include <optional>
#include <string>
#include <cassert>

#include "curl/curl.h"

using namespace std::string_literals;

namespace {

size_t write_fn(char* data, size_t size, size_t nmemb, void* user_data) {
  assert(user_data != nullptr);
  std::string* buffer = reinterpret_cast<std::string*>(user_data);
  buffer->append(data, size * nmemb);
  return size * nmemb;
}

}  // namespace

namespace curlssl {
namespace http {

Client::Client(const std::string& cacert_path) : cacert_path(cacert_path) {
  curl_global_init(CURL_GLOBAL_DEFAULT);
}

Client::~Client() { curl_global_cleanup(); }

std::optional<std::string> Client::get(const std::string& url,
                                       std::string* error) const {
  std::string placeholder;
  if (error == nullptr) {
    error = &placeholder;
  }

  std::unique_ptr<CURL, decltype(&curl_easy_cleanup)> curl(curl_easy_init(),
                                                           curl_easy_cleanup);
  if (curl == nullptr) {
    *error = "Failed to create CURL object";
    return std::nullopt;
  }

  CURLcode res = curl_easy_setopt(curl.get(), CURLOPT_URL, url.c_str());
  if (res != CURLE_OK) {
    *error = "CURLOPT_URL failed: "s + curl_easy_strerror(res);
    return std::nullopt;
  }

  res = curl_easy_setopt(curl.get(), CURLOPT_VERBOSE, 1L);
  if (res != CURLE_OK) {
    *error = "CURLOPT_VERBOSE failed: "s + curl_easy_strerror(res);
    return std::nullopt;
  }

  res = curl_easy_setopt(curl.get(), CURLOPT_CAINFO, cacert_path.c_str());
  if (res != CURLE_OK) {
    *error = "CURLOPT_VERBOSE failed: "s + curl_easy_strerror(res);
    return std::nullopt;
  }

  res = curl_easy_setopt(curl.get(), CURLOPT_WRITEFUNCTION, write_fn);
  if (res != CURLE_OK) {
    *error = "CURLOPT_WRITEFUNCTION failed: "s + curl_easy_strerror(res);
    return std::nullopt;
  }

  std::string buffer;
  res = curl_easy_setopt(curl.get(), CURLOPT_WRITEDATA,
                         reinterpret_cast<void*>(&buffer));
  if (res != CURLE_OK) {
    *error = "CURLOPT_WRITEDATA failed: "s + curl_easy_strerror(res);
    return std::nullopt;
  }

  res = curl_easy_setopt(curl.get(), CURLOPT_FOLLOWLOCATION, 1L);
  if (res != CURLE_OK) {
    *error = "CURLOPT_FOLLOWLOCATION failed: "s + curl_easy_strerror(res);
    return std::nullopt;
  }

  res = curl_easy_perform(curl.get());
  if (res != CURLE_OK) {
    *error = "easy_perform failed: "s + curl_easy_strerror(res);
    return std::nullopt;
  }

  return buffer;
}

}  // namespace http
}  // namespace curlssl
