/*
 * Copyright 2013 The Android Open Source Project
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

#include "JNIHelper.h"

#include <string.h>

#include <fstream>
#include <iostream>

#include <EGL/egl.h>
#include <GLES2/gl2.h>

namespace ndk_helper {

#define NATIVEACTIVITY_CLASS_NAME "android/app/NativeActivity"

//---------------------------------------------------------------------------
// JNI Helper functions
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
// Singleton
//---------------------------------------------------------------------------
JNIHelper* JNIHelper::GetInstance() {
  static JNIHelper helper;
  return &helper;
}

//---------------------------------------------------------------------------
// Ctor
//---------------------------------------------------------------------------
JNIHelper::JNIHelper() : activity_(NULL) {}

//---------------------------------------------------------------------------
// Dtor
//---------------------------------------------------------------------------
JNIHelper::~JNIHelper() {
  // Lock mutex
  std::lock_guard<std::mutex> lock(mutex_);

  JNIEnv* env = AttachCurrentThread();
  env->DeleteGlobalRef(jni_helper_java_ref_);
  env->DeleteGlobalRef(jni_helper_java_class_);

  DetachCurrentThread();
}

//---------------------------------------------------------------------------
// Init
//---------------------------------------------------------------------------
void JNIHelper::Init(ANativeActivity* activity, const char* helper_class_name) {
  JNIHelper& helper = *GetInstance();

  helper.activity_ = activity;

  // Lock mutex
  std::lock_guard<std::mutex> lock(helper.mutex_);

  JNIEnv* env = helper.AttachCurrentThread();

  // Retrieve app bundle id
  jclass android_content_Context = env->GetObjectClass(helper.activity_->clazz);
  jmethodID midGetPackageName = env->GetMethodID(
      android_content_Context, "getPackageName", "()Ljava/lang/String;");

  jstring packageName = (jstring)env->CallObjectMethod(helper.activity_->clazz,
                                                       midGetPackageName);
  const char* appname = env->GetStringUTFChars(packageName, NULL);
  helper.app_name_ = std::string(appname);

  jclass cls = helper.RetrieveClass(env, helper_class_name);
  helper.jni_helper_java_class_ = (jclass)env->NewGlobalRef(cls);

  jmethodID constructor =
      env->GetMethodID(helper.jni_helper_java_class_, "<init>",
                       "(Landroid/app/NativeActivity;)V");

  helper.jni_helper_java_ref_ = env->NewObject(helper.jni_helper_java_class_,
                                               constructor, activity->clazz);
  helper.jni_helper_java_ref_ = env->NewGlobalRef(helper.jni_helper_java_ref_);

  // Get app label
  jstring labelName = (jstring)helper.CallObjectMethod("getApplicationName",
                                                       "()Ljava/lang/String;");
  const char* label = env->GetStringUTFChars(labelName, NULL);
  helper.app_label_ = std::string(label);

  env->ReleaseStringUTFChars(packageName, appname);
  env->ReleaseStringUTFChars(labelName, label);
  env->DeleteLocalRef(packageName);
  env->DeleteLocalRef(labelName);
  env->DeleteLocalRef(cls);
}

void JNIHelper::Init(ANativeActivity* activity, const char* helper_class_name,
                     const char* native_soname) {
  Init(activity, helper_class_name);
  if (native_soname) {
    JNIHelper& helper = *GetInstance();
    // Lock mutex
    std::lock_guard<std::mutex> lock(helper.mutex_);

    JNIEnv* env = helper.AttachCurrentThread();

    // Setup soname
    jstring soname = env->NewStringUTF(native_soname);

    jmethodID mid = env->GetMethodID(helper.jni_helper_java_class_,
                                     "loadLibrary", "(Ljava/lang/String;)V");
    env->CallVoidMethod(helper.jni_helper_java_ref_, mid, soname);

    env->DeleteLocalRef(soname);
  }
}

//---------------------------------------------------------------------------
// readFile
//---------------------------------------------------------------------------
bool JNIHelper::ReadFile(const char* fileName,
                         std::vector<uint8_t>* buffer_ref) {
  if (activity_ == NULL) {
    LOGI(
        "JNIHelper has not been initialized.Call init() to initialize the "
        "helper");
    return false;
  }

  // Lock mutex
  std::lock_guard<std::mutex> lock(mutex_);

  // First, try reading from externalFileDir;
  JNIEnv* env = AttachCurrentThread();
  jstring str_path = GetExternalFilesDirJString(env);

  std::string s;
  if(str_path) {
    const char* path = env->GetStringUTFChars(str_path, NULL);
    s = std::string(path);
    if (fileName[0] != '/') {
      s.append("/");
    }
    s.append(fileName);
    env->ReleaseStringUTFChars(str_path, path);
    env->DeleteLocalRef(str_path);
  }
  std::ifstream f(s.c_str(), std::ios::binary);
  activity_->vm->DetachCurrentThread();
  if (f) {
    LOGI("reading:%s", s.c_str());
    f.seekg(0, std::ifstream::end);
    int32_t fileSize = f.tellg();
    f.seekg(0, std::ifstream::beg);
    buffer_ref->reserve(fileSize);
    buffer_ref->assign(std::istreambuf_iterator<char>(f),
                       std::istreambuf_iterator<char>());
    f.close();
    return true;
  } else {
    // Fallback to assetManager
    AAssetManager* assetManager = activity_->assetManager;
    AAsset* assetFile =
        AAssetManager_open(assetManager, fileName, AASSET_MODE_BUFFER);
    if (!assetFile) {
      return false;
    }
    uint8_t* data = (uint8_t*)AAsset_getBuffer(assetFile);
    int32_t size = AAsset_getLength(assetFile);
    if (data == NULL) {
      AAsset_close(assetFile);

      LOGI("Failed to load:%s", fileName);
      return false;
    }

    buffer_ref->reserve(size);
    buffer_ref->assign(data, data + size);

    AAsset_close(assetFile);
    return true;
  }
}

std::string JNIHelper::GetExternalFilesDir() {
  if (activity_ == NULL) {
    LOGI(
        "JNIHelper has not been initialized. Call init() to initialize the "
        "helper");
    return std::string("");
  }

  // Lock mutex
  std::lock_guard<std::mutex> lock(mutex_);

  // First, try reading from externalFileDir;
  JNIEnv* env = AttachCurrentThread();

  jstring strPath = GetExternalFilesDirJString(env);
  const char* path = env->GetStringUTFChars(strPath, NULL);
  std::string s(path);

  env->ReleaseStringUTFChars(strPath, path);
  env->DeleteLocalRef(strPath);
  return s;
}

uint32_t JNIHelper::LoadTexture(const char* file_name, int32_t* outWidth,
                                int32_t* outHeight, bool* hasAlpha) {
  if (activity_ == NULL) {
    LOGI(
        "JNIHelper has not been initialized. Call init() to initialize the "
        "helper");
    return 0;
  }

  // Lock mutex
  std::lock_guard<std::mutex> lock(mutex_);

  JNIEnv* env = AttachCurrentThread();
  jstring name = env->NewStringUTF(file_name);

  GLuint tex;
  glGenTextures(1, &tex);
  glBindTexture(GL_TEXTURE_2D, tex);

  glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
                  GL_LINEAR_MIPMAP_NEAREST);
  glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  jmethodID mid = env->GetMethodID(jni_helper_java_class_, "loadTexture",
                                   "(Ljava/lang/String;)Ljava/lang/Object;");

  jobject out = env->CallObjectMethod(jni_helper_java_ref_, mid, name);

  jclass javaCls =
      RetrieveClass(env, "com/sample/helper/NDKHelper$TextureInformation");
  jfieldID fidRet = env->GetFieldID(javaCls, "ret", "Z");
  jfieldID fidHasAlpha = env->GetFieldID(javaCls, "alphaChannel", "Z");
  jfieldID fidWidth = env->GetFieldID(javaCls, "originalWidth", "I");
  jfieldID fidHeight = env->GetFieldID(javaCls, "originalHeight", "I");
  bool ret = env->GetBooleanField(out, fidRet);
  bool alpha = env->GetBooleanField(out, fidHasAlpha);
  int32_t width = env->GetIntField(out, fidWidth);
  int32_t height = env->GetIntField(out, fidHeight);
  if (!ret) {
    glDeleteTextures(1, &tex);
    tex = -1;
    LOGI("Texture load failed %s", file_name);
  }
  LOGI("Loaded texture original size:%dx%d alpha:%d", width, height,
       (int32_t)alpha);
  if (outWidth != NULL) {
    *outWidth = width;
  }
  if (outHeight != NULL) {
    *outHeight = height;
  }
  if (hasAlpha != NULL) {
    *hasAlpha = alpha;
  }

  // Generate mipmap
  glGenerateMipmap(GL_TEXTURE_2D);

  env->DeleteLocalRef(name);
  DetachCurrentThread();

  return tex;
}

uint32_t JNIHelper::LoadCubemapTexture(const char* file_name,
                                       const int32_t face,
                                       const int32_t miplevel, const bool sRGB,
                                       int32_t* outWidth, int32_t* outHeight,
                                       bool* hasAlpha) {
  if (activity_ == NULL) {
    LOGI(
        "JNIHelper has not been initialized. Call init() to initialize the "
        "helper");
    return 0;
  }

  // Lock mutex
  std::lock_guard<std::mutex> lock(mutex_);

  JNIEnv* env = AttachCurrentThread();
  jstring name = env->NewStringUTF(file_name);

  jmethodID mid = env->GetMethodID(jni_helper_java_class_, "loadCubemapTexture",
                                   "(Ljava/lang/String;IIZ)Ljava/lang/Object;");

  jobject out =
      env->CallObjectMethod(jni_helper_java_ref_, mid, name, face, miplevel);

  jclass javaCls =
      RetrieveClass(env, "com/sample/helper/NDKHelper$TextureInformation");
  jfieldID fidRet = env->GetFieldID(javaCls, "ret", "Z");
  jfieldID fidHasAlpha = env->GetFieldID(javaCls, "alphaChannel", "Z");
  jfieldID fidWidth = env->GetFieldID(javaCls, "originalWidth", "I");
  jfieldID fidHeight = env->GetFieldID(javaCls, "originalHeight", "I");
  bool ret = env->GetBooleanField(out, fidRet);
  bool alpha = env->GetBooleanField(out, fidHasAlpha);
  int32_t width = env->GetIntField(out, fidWidth);
  int32_t height = env->GetIntField(out, fidHeight);
  if (!ret) {
    LOGI("Texture load failed %s", file_name);
  }
  LOGI("Loaded texture original size:%dx%d alpha:%d", width, height,
       (int32_t)alpha);
  if (outWidth != NULL) {
    *outWidth = width;
  }
  if (outHeight != NULL) {
    *outHeight = height;
  }
  if (hasAlpha != NULL) {
    *hasAlpha = alpha;
  }

  env->DeleteLocalRef(name);
  env->DeleteLocalRef(javaCls);

  return 0;
}

jobject JNIHelper::LoadImage(const char* file_name, int32_t* outWidth,
                             int32_t* outHeight, bool* hasAlpha) {
  if (activity_ == NULL) {
    LOGI(
        "JNIHelper has not been initialized. Call init() to initialize the "
        "helper");
    return 0;
  }

  // Lock mutex
  std::lock_guard<std::mutex> lock(mutex_);

  JNIEnv* env = AttachCurrentThread();
  jstring name = env->NewStringUTF(file_name);

  jmethodID mid = env->GetMethodID(jni_helper_java_class_, "loadImage",
                                   "(Ljava/lang/String;)Ljava/lang/Object;");

  jobject out = env->CallObjectMethod(jni_helper_java_ref_, mid, name);

  jclass javaCls =
      RetrieveClass(env, "com/sample/helper/NDKHelper$TextureInformation");
  jfieldID fidRet = env->GetFieldID(javaCls, "ret", "Z");
  jfieldID fidHasAlpha = env->GetFieldID(javaCls, "alphaChannel", "Z");
  jfieldID fidWidth = env->GetFieldID(javaCls, "originalWidth", "I");
  jfieldID fidHeight = env->GetFieldID(javaCls, "originalHeight", "I");
  bool ret = env->GetBooleanField(out, fidRet);
  bool alpha = env->GetBooleanField(out, fidHasAlpha);
  int32_t width = env->GetIntField(out, fidWidth);
  int32_t height = env->GetIntField(out, fidHeight);
  if (!ret) {
    LOGI("Texture load failed %s", file_name);
  }
  LOGI("Loaded texture original size:%dx%d alpha:%d", width, height,
       (int32_t)alpha);
  if (outWidth != NULL) {
    *outWidth = width;
  }
  if (outHeight != NULL) {
    *outHeight = height;
  }
  if (hasAlpha != NULL) {
    *hasAlpha = alpha;
  }

  jfieldID fidImage = env->GetFieldID(javaCls, "image", "Ljava/lang/Object;");
  jobject array = env->GetObjectField(out, fidImage);
  jobject objGlobal = env->NewGlobalRef(array);

  env->DeleteLocalRef(name);
  env->DeleteLocalRef(javaCls);

  return objGlobal;
}

std::string JNIHelper::ConvertString(const char* str, const char* encode) {
  if (activity_ == NULL) {
    LOGI(
        "JNIHelper has not been initialized. Call init() to initialize the "
        "helper");
    return std::string("");
  }

  // Lock mutex
  std::lock_guard<std::mutex> lock(mutex_);

  JNIEnv* env = AttachCurrentThread();
  env->PushLocalFrame(16);

  int32_t iLength = strlen((const char*)str);

  jbyteArray array = env->NewByteArray(iLength);
  env->SetByteArrayRegion(array, 0, iLength, (const signed char*)str);

  jstring strEncode = env->NewStringUTF(encode);

  jclass cls = env->FindClass("java/lang/String");
  jmethodID ctor = env->GetMethodID(cls, "<init>", "([BLjava/lang/String;)V");
  jstring object = (jstring)env->NewObject(cls, ctor, array, strEncode);

  const char* cparam = env->GetStringUTFChars(object, NULL);

  std::string s = std::string(cparam);

  env->ReleaseStringUTFChars(object, cparam);
  env->DeleteLocalRef(array);
  env->DeleteLocalRef(strEncode);
  env->DeleteLocalRef(object);
  env->DeleteLocalRef(cls);

  env->PopLocalFrame(NULL);

  return s;
}
/*
 * Retrieve string resource with a given name
 * arguments:
 *  in: resourceName, name of string resource to retrieve
 * return: string resource value, returns "" when there is no string resource
 * with given name
 */
std::string JNIHelper::GetStringResource(const std::string& resourceName) {
  if (activity_ == NULL) {
    LOGI(
        "JNIHelper has not been initialized. Call init() to initialize the "
        "helper");
    return std::string("");
  }

  // Lock mutex
  std::lock_guard<std::mutex> lock(mutex_);

  JNIEnv* env = AttachCurrentThread();
  jstring name = env->NewStringUTF(resourceName.c_str());

  jstring ret = (jstring)CallObjectMethod(
      "getStringResource", "(Ljava/lang/String;)Ljava/lang/String;", name);

  const char* resource = env->GetStringUTFChars(ret, NULL);
  std::string s = std::string(resource);

  env->ReleaseStringUTFChars(ret, resource);
  env->DeleteLocalRef(ret);
  env->DeleteLocalRef(name);

  return s;
}
/*
 * Audio helpers
 */
int32_t JNIHelper::GetNativeAudioBufferSize() {
  if (activity_ == NULL) {
    LOGI(
        "JNIHelper has not been initialized. Call init() to initialize the "
        "helper");
    return 0;
  }

  JNIEnv* env = AttachCurrentThread();
  jmethodID mid = env->GetMethodID(jni_helper_java_class_,
                                   "getNativeAudioBufferSize", "()I");
  int32_t i = env->CallIntMethod(jni_helper_java_ref_, mid);
  return i;
}

int32_t JNIHelper::GetNativeAudioSampleRate() {
  if (activity_ == NULL) {
    LOGI(
        "JNIHelper has not been initialized. Call init() to initialize the "
        "helper");
    return 0;
  }

  JNIEnv* env = AttachCurrentThread();
  jmethodID mid = env->GetMethodID(jni_helper_java_class_,
                                   "getNativeAudioSampleRate", "()I");
  int32_t i = env->CallIntMethod(jni_helper_java_ref_, mid);
  return i;
}

//---------------------------------------------------------------------------
// Misc implementations
//---------------------------------------------------------------------------
jclass JNIHelper::RetrieveClass(JNIEnv* jni, const char* class_name) {
  jclass activity_class = jni->FindClass(NATIVEACTIVITY_CLASS_NAME);
  jmethodID get_class_loader = jni->GetMethodID(
      activity_class, "getClassLoader", "()Ljava/lang/ClassLoader;");
  jobject cls = jni->CallObjectMethod(activity_->clazz, get_class_loader);
  jclass class_loader = jni->FindClass("java/lang/ClassLoader");
  jmethodID find_class = jni->GetMethodID(
      class_loader, "loadClass", "(Ljava/lang/String;)Ljava/lang/Class;");

  jstring str_class_name = jni->NewStringUTF(class_name);
  jclass class_retrieved =
      (jclass)jni->CallObjectMethod(cls, find_class, str_class_name);
  jni->DeleteLocalRef(str_class_name);
  jni->DeleteLocalRef(activity_class);
  jni->DeleteLocalRef(class_loader);
  return class_retrieved;
}

jstring JNIHelper::GetExternalFilesDirJString(JNIEnv* env) {
  if (activity_ == NULL) {
    LOGI(
        "JNIHelper has not been initialized. Call init() to initialize the "
        "helper");
    return NULL;
  }

  jstring obj_Path = nullptr;
  // Invoking getExternalFilesDir() java API
  jclass cls_Env = env->FindClass(NATIVEACTIVITY_CLASS_NAME);
  jmethodID mid = env->GetMethodID(cls_Env, "getExternalFilesDir",
                                   "(Ljava/lang/String;)Ljava/io/File;");
  jobject obj_File = env->CallObjectMethod(activity_->clazz, mid, NULL);
  if (obj_File) {
    jclass cls_File = env->FindClass("java/io/File");
    jmethodID mid_getPath =
        env->GetMethodID(cls_File, "getPath", "()Ljava/lang/String;");
    obj_Path = (jstring)env->CallObjectMethod(obj_File, mid_getPath);
  }
  return obj_Path;
}

void JNIHelper::DeleteObject(jobject obj) {
  if (obj == NULL) {
    LOGI("obj can not be NULL");
    return;
  }

  JNIEnv* env = AttachCurrentThread();
  env->DeleteGlobalRef(obj);
}

jobject JNIHelper::CallObjectMethod(const char* strMethodName,
                                    const char* strSignature, ...) {
  if (activity_ == NULL) {
    LOGI(
        "JNIHelper has not been initialized. Call init() to initialize the "
        "helper");
    return NULL;
  }

  JNIEnv* env = AttachCurrentThread();
  jmethodID mid =
      env->GetMethodID(jni_helper_java_class_, strMethodName, strSignature);
  if (mid == NULL) {
    LOGI("method ID %s, '%s' not found", strMethodName, strSignature);
    return NULL;
  }

  va_list args;
  va_start(args, strSignature);
  jobject obj = env->CallObjectMethodV(jni_helper_java_ref_, mid, args);
  va_end(args);

  return obj;
}

void JNIHelper::CallVoidMethod(const char* strMethodName,
                               const char* strSignature, ...) {
  if (activity_ == NULL) {
    LOGI(
        "JNIHelper has not been initialized. Call init() to initialize the "
        "helper");
    return;
  }

  JNIEnv* env = AttachCurrentThread();
  jmethodID mid =
      env->GetMethodID(jni_helper_java_class_, strMethodName, strSignature);
  if (mid == NULL) {
    LOGI("method ID %s, '%s' not found", strMethodName, strSignature);
    return;
  }
  va_list args;
  va_start(args, strSignature);
  env->CallVoidMethodV(jni_helper_java_ref_, mid, args);
  va_end(args);

  return;
}

jobject JNIHelper::CallObjectMethod(jobject object, const char* strMethodName,
                                    const char* strSignature, ...) {
  if (activity_ == NULL) {
    LOGI(
        "JNIHelper has not been initialized. Call init() to initialize the "
        "helper");
    return NULL;
  }

  JNIEnv* env = AttachCurrentThread();
  jclass cls = env->GetObjectClass(object);
  jmethodID mid = env->GetMethodID(cls, strMethodName, strSignature);
  if (mid == NULL) {
    LOGI("method ID %s, '%s' not found", strMethodName, strSignature);
    return NULL;
  }

  va_list args;
  va_start(args, strSignature);
  jobject obj = env->CallObjectMethodV(object, mid, args);
  va_end(args);

  env->DeleteLocalRef(cls);
  return obj;
}

void JNIHelper::CallVoidMethod(jobject object, const char* strMethodName,
                               const char* strSignature, ...) {
  if (activity_ == NULL) {
    LOGI(
        "JNIHelper has not been initialized. Call init() to initialize the "
        "helper");
    return;
  }

  JNIEnv* env = AttachCurrentThread();
  jclass cls = env->GetObjectClass(object);
  jmethodID mid = env->GetMethodID(cls, strMethodName, strSignature);
  if (mid == NULL) {
    LOGI("method ID %s, '%s' not found", strMethodName, strSignature);
    return;
  }

  va_list args;
  va_start(args, strSignature);
  env->CallVoidMethodV(object, mid, args);
  va_end(args);

  env->DeleteLocalRef(cls);
  return;
}

float JNIHelper::CallFloatMethod(jobject object, const char* strMethodName,
                                 const char* strSignature, ...) {
  float f = 0.f;
  if (activity_ == NULL) {
    LOGI(
        "JNIHelper has not been initialized. Call init() to initialize the "
        "helper");
    return f;
  }

  JNIEnv* env = AttachCurrentThread();
  jclass cls = env->GetObjectClass(object);
  jmethodID mid = env->GetMethodID(cls, strMethodName, strSignature);
  if (mid == NULL) {
    LOGI("method ID %s, '%s' not found", strMethodName, strSignature);
    return f;
  }
  va_list args;
  va_start(args, strSignature);
  f = env->CallFloatMethodV(object, mid, args);
  va_end(args);

  env->DeleteLocalRef(cls);
  return f;
}

int32_t JNIHelper::CallIntMethod(jobject object, const char* strMethodName,
                                 const char* strSignature, ...) {
  int32_t i = 0;
  if (activity_ == NULL) {
    LOGI(
        "JNIHelper has not been initialized. Call init() to initialize the "
        "helper");
    return i;
  }

  JNIEnv* env = AttachCurrentThread();
  jclass cls = env->GetObjectClass(object);
  jmethodID mid = env->GetMethodID(cls, strMethodName, strSignature);
  if (mid == NULL) {
    LOGI("method ID %s, '%s' not found", strMethodName, strSignature);
    return i;
  }
  va_list args;
  va_start(args, strSignature);
  i = env->CallIntMethodV(object, mid, args);
  va_end(args);

  env->DeleteLocalRef(cls);
  return i;
}

bool JNIHelper::CallBooleanMethod(jobject object, const char* strMethodName,
                                  const char* strSignature, ...) {
  bool b;
  if (activity_ == NULL) {
    LOGI(
        "JNIHelper has not been initialized. Call init() to initialize the "
        "helper");
    return false;
  }

  JNIEnv* env = AttachCurrentThread();
  jclass cls = env->GetObjectClass(object);
  jmethodID mid = env->GetMethodID(cls, strMethodName, strSignature);
  if (mid == NULL) {
    LOGI("method ID %s, '%s' not found", strMethodName, strSignature);
    return false;
  }
  va_list args;
  va_start(args, strSignature);
  b = env->CallBooleanMethodV(object, mid, args);
  va_end(args);

  env->DeleteLocalRef(cls);
  return b;
}

jobject JNIHelper::CreateObject(const char* class_name) {
  JNIEnv* env = AttachCurrentThread();

  jclass cls = env->FindClass(class_name);
  jmethodID constructor = env->GetMethodID(cls, "<init>", "()V");

  jobject obj = env->NewObject(cls, constructor);
  jobject objGlobal = env->NewGlobalRef(obj);
  env->DeleteLocalRef(obj);
  env->DeleteLocalRef(cls);
  return objGlobal;
}

void JNIHelper::RunOnUiThread(std::function<void()> callback) {
  // Lock mutex
  std::lock_guard<std::mutex> lock(mutex_);

  JNIEnv* env = AttachCurrentThread();
  static jmethodID mid = NULL;
  if (mid == NULL)
    mid = env->GetMethodID(jni_helper_java_class_, "runOnUIThread", "(J)V");

  // Allocate temporary function object to be passed around
  std::function<void()>* pCallback = new std::function<void()>(callback);
  env->CallVoidMethod(jni_helper_java_ref_, mid, (int64_t)pCallback);
}

// This JNI function is invoked from UIThread asynchronously
extern "C" {
JNIEXPORT void Java_com_sample_helper_NDKHelper_RunOnUiThreadHandler(
    JNIEnv* env, jobject thiz, int64_t pointer) {
  std::function<void()>* pCallback = (std::function<void()>*)pointer;
  (*pCallback)();

  // Deleting temporary object
  delete pCallback;
}
}

}  // namespace ndkHelper
