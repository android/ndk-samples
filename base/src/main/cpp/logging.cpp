/*
 * Copyright (C) 2015 The Android Open Source Project
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

#include "base/logging.h"

#include <android/log.h>
#include <android/set_abort_message.h>
#include <fcntl.h>
#include <inttypes.h>
#include <libgen.h>
#include <stdlib.h>
#include <time.h>

#include <atomic>
#include <iostream>
#include <limits>
#include <mutex>
#include <optional>
#include <sstream>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include "base/macros.h"
#include "logging_splitters.h"

namespace ndksamples::base {

static const char* GetFileBasename(const char* file) {
  // We can't use basename(3) even on Unix because the Mac doesn't
  // have a non-modifying basename.
  const char* last_slash = strrchr(file, '/');
  if (last_slash != nullptr) {
    return last_slash + 1;
  }
  return file;
}

static int32_t LogIdTolog_id_t(LogId log_id) {
  switch (log_id) {
    case MAIN:
      return LOG_ID_MAIN;
    case SYSTEM:
      return LOG_ID_SYSTEM;
    case RADIO:
      return LOG_ID_RADIO;
    case CRASH:
      return LOG_ID_CRASH;
    case DEFAULT:
    default:
      return LOG_ID_DEFAULT;
  }
}

static int32_t LogSeverityToPriority(LogSeverity severity) {
  switch (severity) {
    case VERBOSE:
      return ANDROID_LOG_VERBOSE;
    case DEBUG:
      return ANDROID_LOG_DEBUG;
    case INFO:
      return ANDROID_LOG_INFO;
    case WARNING:
      return ANDROID_LOG_WARN;
    case ERROR:
      return ANDROID_LOG_ERROR;
    case FATAL_WITHOUT_ABORT:
    case FATAL:
    default:
      return ANDROID_LOG_FATAL;
  }
}

static LogFunction& Logger() {
  static auto& logger = *new LogFunction(LogdLogger());
  return logger;
}

static AbortFunction& Aborter() {
  static auto& aborter = *new AbortFunction(DefaultAborter);
  return aborter;
}

// Only used for Q fallback.
static std::recursive_mutex& TagLock() {
  static auto& tag_lock = *new std::recursive_mutex();
  return tag_lock;
}

static std::string* gDefaultTag;

void SetDefaultTag(const std::string_view tag) {
  std::lock_guard<std::recursive_mutex> lock(TagLock());
  if (gDefaultTag != nullptr) {
    delete gDefaultTag;
    gDefaultTag = nullptr;
  }
  if (!tag.empty()) {
    gDefaultTag = new std::string(tag);
  }
}

static bool gInitialized = false;

// Only used for Q fallback.
static LogSeverity gMinimumLogSeverity = INFO;

void DefaultAborter(const char* abort_message) {
  android_set_abort_message(abort_message);
  abort();
}

static void LogdLogChunk(LogId id, LogSeverity severity, const char* tag,
                         const char* message) {
  int32_t lg_id = LogIdTolog_id_t(id);
  int32_t priority = LogSeverityToPriority(severity);

  __android_log_buf_print(lg_id, priority, tag, "%s", message);
}

LogdLogger::LogdLogger(LogId default_log_id)
    : default_log_id_(default_log_id) {}

void LogdLogger::operator()(LogId id, LogSeverity severity, const char* tag,
                            const char* file, unsigned int line,
                            const char* message) {
  if (id == DEFAULT) {
    id = default_log_id_;
  }

  SplitByLogdChunks(id, severity, tag, file, line, message, LogdLogChunk);
}

void InitLogging(const std::optional<std::string_view> default_tag,
                 std::optional<LogSeverity> log_level, LogFunction&& logger,
                 AbortFunction&& aborter) {
  SetLogger(std::forward<LogFunction>(logger));
  SetAborter(std::forward<AbortFunction>(aborter));

  if (gInitialized) {
    return;
  }

  gInitialized = true;

  if (default_tag.has_value()) {
    SetDefaultTag(default_tag.value());
  }

  const char* tags = getenv("ANDROID_LOG_TAGS");
  if (tags == nullptr) {
    return;
  }

  if (log_level.has_value()) {
    SetMinimumLogSeverity(log_level.value());
  }
}

LogFunction SetLogger(LogFunction&& logger) {
  LogFunction old_logger = std::move(Logger());
  Logger() = std::move(logger);
  return old_logger;
}

AbortFunction SetAborter(AbortFunction&& aborter) {
  AbortFunction old_aborter = std::move(Aborter());
  Aborter() = std::move(aborter);
  return old_aborter;
}

// This indirection greatly reduces the stack impact of having lots of
// checks/logging in a function.
class LogMessageData {
 public:
  LogMessageData(const char* file, unsigned int line, LogSeverity severity,
                 const char* tag, int error)
      : file_(GetFileBasename(file)),
        line_number_(line),
        severity_(severity),
        tag_(tag),
        error_(error) {}

  DISALLOW_COPY_AND_ASSIGN(LogMessageData);

  const char* GetFile() const { return file_; }

  unsigned int GetLineNumber() const { return line_number_; }

  LogSeverity GetSeverity() const { return severity_; }

  const char* GetTag() const { return tag_; }

  int GetError() const { return error_; }

  std::ostream& GetBuffer() { return buffer_; }

  std::string ToString() const { return buffer_.str(); }

 private:
  std::ostringstream buffer_;
  const char* const file_;
  const unsigned int line_number_;
  const LogSeverity severity_;
  const char* const tag_;
  const int error_;
};

LogMessage::LogMessage(const char* file, unsigned int line, LogId,
                       LogSeverity severity, const char* tag, int error)
    : LogMessage(file, line, severity, tag, error) {}

LogMessage::LogMessage(const char* file, unsigned int line,
                       LogSeverity severity, const char* tag, int error)
    : data_(new LogMessageData(file, line, severity, tag, error)) {}

LogMessage::~LogMessage() {
  // Check severity again. This is duplicate work wrt/ LOG macros, but not
  // LOG_STREAM.
  if (!WOULD_LOG(data_->GetSeverity())) {
    return;
  }

  // Finish constructing the message.
  if (data_->GetError() != -1) {
    data_->GetBuffer() << ": " << strerror(data_->GetError());
  }
  std::string msg(data_->ToString());

  if (data_->GetSeverity() == FATAL) {
    // Set the bionic abort message early to avoid liblog doing it
    // with the individual lines, so that we get the whole message.
    android_set_abort_message(msg.c_str());
  }

  LogLine(data_->GetFile(), data_->GetLineNumber(), data_->GetSeverity(),
          data_->GetTag(), msg.c_str());

  // Abort if necessary.
  if (data_->GetSeverity() == FATAL) {
    Aborter()(msg.c_str());
  }
}

std::ostream& LogMessage::stream() { return data_->GetBuffer(); }

void LogMessage::LogLine(const char* file, unsigned int line,
                         LogSeverity severity, const char* tag,
                         const char* message) {
  if (tag == nullptr) {
    std::lock_guard<std::recursive_mutex> lock(TagLock());
    if (gDefaultTag == nullptr) {
      gDefaultTag = new std::string(getprogname());
    }

    Logger()(DEFAULT, severity, gDefaultTag->c_str(), file, line, message);
  } else {
    Logger()(DEFAULT, severity, tag, file, line, message);
  }
}

LogSeverity GetMinimumLogSeverity() { return gMinimumLogSeverity; }

bool ShouldLog(LogSeverity severity, const char*) {
  return severity >= gMinimumLogSeverity;
}

LogSeverity SetMinimumLogSeverity(LogSeverity new_severity) {
  LogSeverity old_severity = gMinimumLogSeverity;
  gMinimumLogSeverity = new_severity;
  return old_severity;
}

ScopedLogSeverity::ScopedLogSeverity(LogSeverity new_severity) {
  old_ = SetMinimumLogSeverity(new_severity);
}

ScopedLogSeverity::~ScopedLogSeverity() { SetMinimumLogSeverity(old_); }

}  // namespace ndksamples::base
