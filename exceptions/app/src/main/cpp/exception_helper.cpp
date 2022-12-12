/*
 * Copyright (C) 2022 The Android Open Source Project
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

/*
 * Helper functions for throwing Java exceptions from C++.
 *
 * Copied, with a few changes, from JNIHelp.{h,c} in the Android platform's
 * libnativehelper:
 *
 * https://android.googlesource.com/platform/libnativehelper/+/refs/heads/master/include/nativehelper/JNIHelp.h
 * https://android.googlesource.com/platform/libnativehelper/+/refs/heads/master/JNIHelp.c
 */

#include "exception_helper.h"

#include <android/log.h>

#include <string>

static jmethodID FindMethod(JNIEnv* env, const char* className,
                            const char* methodName, const char* descriptor) {
  jclass clazz = env->FindClass(className);
  jmethodID methodId = env->GetMethodID(clazz, methodName, descriptor);
  env->DeleteLocalRef(clazz);
  return methodId;
}

static bool AppendJString(JNIEnv* env, jstring text, std::string* dst) {
  const char* utfText = env->GetStringUTFChars(text, NULL);
  if (utfText == NULL) {
    return false;
  }
  dst->append(utfText);
  env->ReleaseStringUTFChars(text, utfText);
  return true;
}

/*
 * Returns a human-readable summary of an exception object.  The buffer will
 * be populated with the "binary" class name and, if present, the
 * exception message.
 */
static bool GetExceptionSummary(JNIEnv* env, jthrowable thrown,
                                std::string* dst) {
  // Summary is <exception_class_name> ": " <exception_message>
  jclass exceptionClass = env->GetObjectClass(thrown);  // Always succeeds
  jmethodID getName =
      FindMethod(env, "java/lang/Class", "getName", "()Ljava/lang/String;");
  jstring className = (jstring)env->CallObjectMethod(exceptionClass, getName);
  if (className == NULL) {
    *dst = "<error getting class name>";
    env->ExceptionClear();
    env->DeleteLocalRef(exceptionClass);
    return false;
  }
  env->DeleteLocalRef(exceptionClass);
  exceptionClass = NULL;
  if (!AppendJString(env, className, dst)) {
    *dst = "<error getting class name UTF-8>";
    env->ExceptionClear();
    env->DeleteLocalRef(className);
    return false;
  }
  env->DeleteLocalRef(className);
  className = NULL;
  jmethodID getMessage = FindMethod(env, "java/lang/Throwable", "getMessage",
                                    "()Ljava/lang/String;");
  jstring message = (jstring)env->CallObjectMethod(thrown, getMessage);
  if (message == NULL) {
    return true;
  }
  dst->append(": ");
  bool success = AppendJString(env, message, dst);
  if (!success) {
    // Two potential reasons for reaching here:
    //
    // 1. managed heap allocation failure (OOME).
    // 2. native heap allocation failure for the storage in |dst|.
    //
    // Attempt to append failure notification, okay to fail, |dst| contains the
    // class name of |thrown|.
    dst->append("<error getting message>");
    // Clear OOME if present.
    env->ExceptionClear();
  }
  env->DeleteLocalRef(message);
  message = NULL;
  return success;
}

static jstring CreateExceptionMsg(JNIEnv* env, const char* msg) {
  jstring detailMessage = env->NewStringUTF(msg);
  if (detailMessage == NULL) {
    /* Not really much we can do here. We're probably dead in the water,
       but let's try to stumble on... */
    env->ExceptionClear();
  }
  return detailMessage;
}

static void DiscardPendingException(JNIEnv* env, const char* className) {
  jthrowable exception = env->ExceptionOccurred();
  env->ExceptionClear();
  if (exception == NULL) {
    return;
  }
  std::string summary;
  GetExceptionSummary(env, exception, &summary);
  __android_log_print(ANDROID_LOG_WARN, "exception_helper::",
                      "Discarding pending exception (%s) to throw %s",
                      summary.c_str(), className);
  env->DeleteLocalRef(exception);
}

static int ThrowException(JNIEnv* env, const char* className,
                          const char* ctorSig, ...) {
  int status = -1;
  jclass exceptionClass = NULL;
  va_list args;
  va_start(args, ctorSig);
  DiscardPendingException(env, className);
  {
    /* We want to clean up local references before returning from this function,
     * so, regardless of return status, the end block must run. Have the work
     * done in a nested block to avoid using any uninitialized variables in the
     * end block. */
    exceptionClass = env->FindClass(className);
    if (exceptionClass == NULL) {
      __android_log_print(ANDROID_LOG_ERROR, "exception_helper::",
                          "Unable to find exception class %s", className);
      /* an exception, most likely ClassNotFoundException, will now be pending
       */
      goto end;
    }
    jmethodID init = env->GetMethodID(exceptionClass, "<init>", ctorSig);
    if (init == NULL) {
      __android_log_print(ANDROID_LOG_ERROR, "exception_helper::",
                          "Failed to find constructor for '%s' '%s'", className,
                          ctorSig);
      goto end;
    }
    jobject instance = env->NewObjectV(exceptionClass, init, args);
    if (instance == NULL) {
      __android_log_print(ANDROID_LOG_ERROR,
                          "exception_helper::", "Failed to construct '%s'",
                          className);
      goto end;
    }
    if (env->Throw((jthrowable)instance) != JNI_OK) {
      __android_log_print(ANDROID_LOG_ERROR,
                          "exception_helper::", "Failed to throw '%s'",
                          className);
      /* an exception, most likely OOM, will now be pending */
      goto end;
    }
    /* everything worked fine, just update status to success and clean up */
    status = 0;
  }
end:
  va_end(args);
  if (exceptionClass != NULL) {
    env->DeleteLocalRef(exceptionClass);
  }
  return status;
}

/*
 * Helper macro to deal with conversion of the exception message from a C string
 * to jstring.
 */
#define THROW_EXCEPTION_WITH_MESSAGE(env, className, ctorSig, msg, ...)   \
  ({                                                                      \
    jstring _detailMessage = CreateExceptionMsg(env, msg);                \
    int _status = ThrowException(env, className, ctorSig, _detailMessage, \
                                 ##__VA_ARGS__);                          \
    if (_detailMessage != NULL) {                                         \
      env->DeleteLocalRef(_detailMessage);                                \
    }                                                                     \
    _status;                                                              \
  })

int jniThrowException(JNIEnv* env, const char* className, const char* msg) {
  return THROW_EXCEPTION_WITH_MESSAGE(env, className, "(Ljava/lang/String;)V",
                                      msg);
}
