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

#include <inttypes.h>
#include <time.h>

#include <format>

#include "base/logging.h"

#define LOGGER_ENTRY_MAX_PAYLOAD 4068  // This constant is not in the NDK.

namespace ndksamples::base {

// This splits the message up line by line, by calling log_function with a
// pointer to the start of each line and the size up to the newline character.
// It sends size = -1 for the final line.
template <typename F, typename... Args>
static void SplitByLines(const char* msg, const F& log_function,
                         Args&&... args) {
  const char* newline = strchr(msg, '\n');
  while (newline != nullptr) {
    log_function(msg, newline - msg, args...);
    msg = newline + 1;
    newline = strchr(msg, '\n');
  }

  log_function(msg, -1, args...);
}

// This splits the message up into chunks that logs can process delimited by new
// lines.  It calls log_function with the exact null terminated message that
// should be sent to logd. Note, despite the loops and snprintf's, if severity
// is not fatal and there are no new lines, this function simply calls
// log_function with msg without any extra overhead.
template <typename F>
static void SplitByLogdChunks(LogId log_id, LogSeverity severity,
                              const char* tag, const char* file,
                              unsigned int line, const char* msg,
                              const F& log_function) {
  // The maximum size of a payload, after the log header that logd will accept
  // is LOGGER_ENTRY_MAX_PAYLOAD, so subtract the other elements in the payload
  // to find the size of the string that we can log in each pass. The protocol
  // is documented in liblog/README.protocol.md. Specifically we subtract a byte
  // for the priority, the length of the tag + its null terminator, and an
  // additional byte for the null terminator on the payload.  We subtract an
  // additional 32 bytes for slack, similar to java/android/util/Log.java.
  ptrdiff_t max_size = LOGGER_ENTRY_MAX_PAYLOAD - strlen(tag) - 35;
  if (max_size <= 0) {
    abort();
  }
  // If we're logging a fatal message, we'll append the file and line numbers.
  bool add_file =
      file != nullptr && (severity == FATAL || severity == FATAL_WITHOUT_ABORT);

  std::string file_header;
  if (add_file) {
    file_header = std::format("{}:{}]", file, line);
  }
  int file_header_size = file_header.size();

  __attribute__((uninitialized)) char logd_chunk[max_size + 1];
  ptrdiff_t chunk_position = 0;

  auto call_log_function = [&]() {
    log_function(log_id, severity, tag, logd_chunk);
    chunk_position = 0;
  };

  auto write_to_logd_chunk = [&](const char* message, int length) {
    int size_written = 0;
    const char* new_line = chunk_position > 0 ? "\n" : "";
    if (add_file) {
      size_written = snprintf(logd_chunk + chunk_position,
                              sizeof(logd_chunk) - chunk_position, "%s%s%.*s",
                              new_line, file_header.c_str(), length, message);
    } else {
      size_written = snprintf(logd_chunk + chunk_position,
                              sizeof(logd_chunk) - chunk_position, "%s%.*s",
                              new_line, length, message);
    }

    // This should never fail, if it does and we set size_written to 0, which
    // will skip this line and move to the next one.
    if (size_written < 0) {
      size_written = 0;
    }
    chunk_position += size_written;
  };

  const char* newline = strchr(msg, '\n');
  while (newline != nullptr) {
    // If we have data in the buffer and this next line doesn't fit, write the
    // buffer.
    if (chunk_position != 0 &&
        chunk_position + (newline - msg) + 1 + file_header_size > max_size) {
      call_log_function();
    }

    // Otherwise, either the next line fits or we have any empty buffer and too
    // large of a line to ever fit, in both cases, we add it to the buffer and
    // continue.
    write_to_logd_chunk(msg, newline - msg);

    msg = newline + 1;
    newline = strchr(msg, '\n');
  }

  // If we have left over data in the buffer and we can fit the rest of msg, add
  // it to the buffer then write the buffer.
  if (chunk_position != 0 &&
      chunk_position + static_cast<int>(strlen(msg)) + 1 + file_header_size <=
          max_size) {
    write_to_logd_chunk(msg, -1);
    call_log_function();
  } else {
    // If the buffer is not empty and we can't fit the rest of msg into it,
    // write its contents.
    if (chunk_position != 0) {
      call_log_function();
    }
    // Then write the rest of the msg.
    if (add_file) {
      snprintf(logd_chunk, sizeof(logd_chunk), "%s%s", file_header.c_str(),
               msg);
      log_function(log_id, severity, tag, logd_chunk);
    } else {
      log_function(log_id, severity, tag, msg);
    }
  }
}

}  // namespace ndksamples::base
