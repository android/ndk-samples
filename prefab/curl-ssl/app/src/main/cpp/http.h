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

#include <optional>
#include <string>

namespace curlssl {
namespace http {

/**
 * An HTTP client backed by curl.
 */
class Client {
 public:
  /**
   * Constructs an HTTP client.
   *
   * @param cacert_path Path to the cacert.pem file for use in verifying SSL
   * certifactes. See the project's README.md for more information.
   */
  explicit Client(const std::string& cacert_path);
  Client(const Client&) = delete;
  ~Client();

  void operator=(const Client&) = delete;

  /**
   * Performs an HTTP GET request.
   *
   * @param url The URL to GET.
   * @param error An out parameter for an error string, if one occurs.
   * @return A non-empty value containing the body of the response on success,
   * or an empty result on failure.
   */
  std::optional<std::string> get(const std::string& url,
                                 std::string* error) const;

 private:
  const std::string cacert_path;
};

}  // namespace http
}  // namespace curlssl
