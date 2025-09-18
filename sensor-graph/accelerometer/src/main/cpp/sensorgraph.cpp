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

// OpenGL ES 2.0 code
#include <GLES2/gl2.h>
#include <android/asset_manager_jni.h>
#include <android/log.h>
#include <android/sensor.h>
#include <dlfcn.h>
#include <jni.h>

#include <cassert>
#include <cstdint>
#include <string>

#define LOG_TAG "accelerometergraph"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)

const int LOOPER_ID_USER = 3;
const int SENSOR_HISTORY_LENGTH = 100;
const int SENSOR_REFRESH_RATE_HZ = 100;
constexpr int32_t SENSOR_REFRESH_PERIOD_US =
    int32_t(1000000 / SENSOR_REFRESH_RATE_HZ);
const float SENSOR_FILTER_ALPHA = 0.1f;

/*
 * AcquireASensorManagerInstance(void)
 *    Workaround AsensorManager_getInstance() deprecation false alarm
 *    for Android-N and before, when compiling with NDK-r15
 */
#include <base/macros.h>
#include <dlfcn.h>
const char* kPackageName = "com.android.accelerometergraph";
ASensorManager* AcquireASensorManagerInstance(void) {
  typedef ASensorManager* (*PF_GETINSTANCEFORPACKAGE)(const char* name);
  void* androidHandle = dlopen("libandroid.so", RTLD_NOW);
  PF_GETINSTANCEFORPACKAGE getInstanceForPackageFunc =
      (PF_GETINSTANCEFORPACKAGE)dlsym(androidHandle,
                                      "ASensorManager_getInstanceForPackage");
  if (getInstanceForPackageFunc) {
    return getInstanceForPackageFunc(kPackageName);
  }

  typedef ASensorManager* (*PF_GETINSTANCE)();
  PF_GETINSTANCE getInstanceFunc =
      (PF_GETINSTANCE)dlsym(androidHandle, "ASensorManager_getInstance");
  // by all means at this point, ASensorManager_getInstance should be available
  assert(getInstanceFunc);
  return getInstanceFunc();
}

class sensorgraph {
  std::string vertexShaderSource;
  std::string fragmentShaderSource;
  ASensorManager* sensorManager;
  const ASensor* accelerometer;
  ASensorEventQueue* accelerometerEventQueue;
  ALooper* looper;

  GLuint shaderProgram;
  GLuint vPositionHandle;
  GLuint vSensorValueHandle;
  GLuint uFragColorHandle;
  GLfloat xPos[SENSOR_HISTORY_LENGTH];

  struct AccelerometerData {
    GLfloat x;
    GLfloat y;
    GLfloat z;
  };
  AccelerometerData sensorData[SENSOR_HISTORY_LENGTH * 2];
  AccelerometerData sensorDataFilter;
  int sensorDataIndex;

 public:
  sensorgraph() : sensorDataIndex(0) {}

  void init(AAssetManager* assetManager) {
    AAsset* vertexShaderAsset =
        AAssetManager_open(assetManager, "shader.glslv", AASSET_MODE_BUFFER);
    assert(vertexShaderAsset != NULL);
    const void* vertexShaderBuf = AAsset_getBuffer(vertexShaderAsset);
    assert(vertexShaderBuf != NULL);
    off_t vertexShaderLength = AAsset_getLength(vertexShaderAsset);
    vertexShaderSource =
        std::string((const char*)vertexShaderBuf, (size_t)vertexShaderLength);
    AAsset_close(vertexShaderAsset);

    AAsset* fragmentShaderAsset =
        AAssetManager_open(assetManager, "shader.glslf", AASSET_MODE_BUFFER);
    assert(fragmentShaderAsset != NULL);
    const void* fragmentShaderBuf = AAsset_getBuffer(fragmentShaderAsset);
    assert(fragmentShaderBuf != NULL);
    off_t fragmentShaderLength = AAsset_getLength(fragmentShaderAsset);
    fragmentShaderSource = std::string((const char*)fragmentShaderBuf,
                                       (size_t)fragmentShaderLength);
    AAsset_close(fragmentShaderAsset);

    sensorManager = AcquireASensorManagerInstance();
    assert(sensorManager != NULL);
    accelerometer = ASensorManager_getDefaultSensor(sensorManager,
                                                    ASENSOR_TYPE_ACCELEROMETER);
    assert(accelerometer != NULL);
    looper = ALooper_prepare(ALOOPER_PREPARE_ALLOW_NON_CALLBACKS);
    assert(looper != NULL);
    accelerometerEventQueue = ASensorManager_createEventQueue(
        sensorManager, looper, LOOPER_ID_USER, NULL, NULL);
    assert(accelerometerEventQueue != NULL);
    auto status =
        ASensorEventQueue_enableSensor(accelerometerEventQueue, accelerometer);
    assert(status >= 0);
    status = ASensorEventQueue_setEventRate(
        accelerometerEventQueue, accelerometer, SENSOR_REFRESH_PERIOD_US);
    assert(status >= 0);
    (void)status;  // to silent unused compiler warning

    generateXPos();
  }

  void surfaceCreated() {
    LOGI("GL_VERSION: %s", glGetString(GL_VERSION));
    LOGI("GL_VENDOR: %s", glGetString(GL_VENDOR));
    LOGI("GL_RENDERER: %s", glGetString(GL_RENDERER));
    LOGI("GL_EXTENSIONS: %s", glGetString(GL_EXTENSIONS));

    shaderProgram = createProgram(vertexShaderSource, fragmentShaderSource);
    assert(shaderProgram != 0);
    GLint getPositionLocationResult =
        glGetAttribLocation(shaderProgram, "vPosition");
    assert(getPositionLocationResult != -1);
    vPositionHandle = (GLuint)getPositionLocationResult;
    GLint getSensorValueLocationResult =
        glGetAttribLocation(shaderProgram, "vSensorValue");
    assert(getSensorValueLocationResult != -1);
    vSensorValueHandle = (GLuint)getSensorValueLocationResult;
    GLint getFragColorLocationResult =
        glGetUniformLocation(shaderProgram, "uFragColor");
    assert(getFragColorLocationResult != -1);
    uFragColorHandle = (GLuint)getFragColorLocationResult;
  }

  void surfaceChanged(int w, int h) { glViewport(0, 0, w, h); }

  void generateXPos() {
    for (auto i = 0; i < SENSOR_HISTORY_LENGTH; i++) {
      float t =
          static_cast<float>(i) / static_cast<float>(SENSOR_HISTORY_LENGTH - 1);
      xPos[i] = -1.f * (1.f - t) + 1.f * t;
    }
  }

  GLuint createProgram(const std::string& pVertexSource,
                       const std::string& pFragmentSource) {
    GLuint vertexShader = loadShader(GL_VERTEX_SHADER, pVertexSource);
    GLuint pixelShader = loadShader(GL_FRAGMENT_SHADER, pFragmentSource);
    GLuint program = glCreateProgram();
    assert(program != 0);
    glAttachShader(program, vertexShader);
    glAttachShader(program, pixelShader);
    glLinkProgram(program);
    GLint programLinked = 0;
    glGetProgramiv(program, GL_LINK_STATUS, &programLinked);
    assert(programLinked != 0);
    glDeleteShader(vertexShader);
    glDeleteShader(pixelShader);
    return program;
  }

  GLuint loadShader(GLenum shaderType, const std::string& pSource) {
    GLuint shader = glCreateShader(shaderType);
    assert(shader != 0);
    const char* sourceBuf = pSource.c_str();
    glShaderSource(shader, 1, &sourceBuf, NULL);
    glCompileShader(shader);
    GLint shaderCompiled = 0;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &shaderCompiled);
    assert(shaderCompiled != 0);
    return shader;
  }

  void update() {
    ALooper_pollOnce(0, NULL, NULL, NULL);
    ASensorEvent event;
    float a = SENSOR_FILTER_ALPHA;
    while (ASensorEventQueue_getEvents(accelerometerEventQueue, &event, 1) >
           0) {
      sensorDataFilter.x =
          a * event.acceleration.x + (1.0f - a) * sensorDataFilter.x;
      sensorDataFilter.y =
          a * event.acceleration.y + (1.0f - a) * sensorDataFilter.y;
      sensorDataFilter.z =
          a * event.acceleration.z + (1.0f - a) * sensorDataFilter.z;
    }
    sensorData[sensorDataIndex] = sensorDataFilter;
    sensorData[SENSOR_HISTORY_LENGTH + sensorDataIndex] = sensorDataFilter;
    sensorDataIndex = (sensorDataIndex + 1) % SENSOR_HISTORY_LENGTH;
  }

  void render() {
    glClearColor(0.f, 0.f, 0.f, 1.0f);
    glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

    glUseProgram(shaderProgram);

    glEnableVertexAttribArray(vPositionHandle);
    glVertexAttribPointer(vPositionHandle, 1, GL_FLOAT, GL_FALSE, 0, xPos);

    glEnableVertexAttribArray(vSensorValueHandle);
    glVertexAttribPointer(vSensorValueHandle, 1, GL_FLOAT, GL_FALSE,
                          sizeof(AccelerometerData),
                          &sensorData[sensorDataIndex].x);

    glUniform4f(uFragColorHandle, 1.0f, 1.0f, 0.0f, 1.0f);
    glDrawArrays(GL_LINE_STRIP, 0, SENSOR_HISTORY_LENGTH);

    glVertexAttribPointer(vSensorValueHandle, 1, GL_FLOAT, GL_FALSE,
                          sizeof(AccelerometerData),
                          &sensorData[sensorDataIndex].y);
    glUniform4f(uFragColorHandle, 1.0f, 0.0f, 1.0f, 1.0f);
    glDrawArrays(GL_LINE_STRIP, 0, SENSOR_HISTORY_LENGTH);

    glVertexAttribPointer(vSensorValueHandle, 1, GL_FLOAT, GL_FALSE,
                          sizeof(AccelerometerData),
                          &sensorData[sensorDataIndex].z);
    glUniform4f(uFragColorHandle, 0.0f, 1.0f, 1.0f, 1.0f);
    glDrawArrays(GL_LINE_STRIP, 0, SENSOR_HISTORY_LENGTH);
  }

  void pause() {
    ASensorEventQueue_disableSensor(accelerometerEventQueue, accelerometer);
  }

  void resume() {
    ASensorEventQueue_enableSensor(accelerometerEventQueue, accelerometer);
    [[maybe_unused]] auto status = ASensorEventQueue_setEventRate(
        accelerometerEventQueue, accelerometer, SENSOR_REFRESH_PERIOD_US);
    assert(status >= 0);
  }
};

sensorgraph gSensorGraph;

void Init(JNIEnv* env, jclass, jobject assetManager) {
  AAssetManager* nativeAssetManager = AAssetManager_fromJava(env, assetManager);
  gSensorGraph.init(nativeAssetManager);
}

void SurfaceCreated(JNIEnv*, jclass) { gSensorGraph.surfaceCreated(); }

void SurfaceChanged(JNIEnv*, jclass, jint width, jint height) {
  gSensorGraph.surfaceChanged(width, height);
}

void DrawFrame(JNIEnv*, jclass) {
  gSensorGraph.update();
  gSensorGraph.render();
}

void Pause(JNIEnv*, jclass) { gSensorGraph.pause(); }

void Resume(JNIEnv*, jclass) { gSensorGraph.resume(); }

extern "C" JNIEXPORT jint JNICALL JNI_OnLoad(JavaVM* _Nonnull vm,
                                             void* _Nullable) {
  JNIEnv* env;
  if (vm->GetEnv(reinterpret_cast<void**>(&env), JNI_VERSION_1_6) != JNI_OK) {
    return JNI_ERR;
  }

  jclass c =
      env->FindClass("com/android/accelerometergraph/AccelerometerGraphJNI");
  if (c == nullptr) return JNI_ERR;

  static const JNINativeMethod methods[] = {
      {"init", "(Landroid/content/res/AssetManager;)V",
       reinterpret_cast<void*>(Init)},
      {"surfaceCreated", "()V", reinterpret_cast<void*>(SurfaceCreated)},
      {"surfaceChanged", "(II)V", reinterpret_cast<void*>(SurfaceChanged)},
      {"drawFrame", "()V", reinterpret_cast<void*>(DrawFrame)},
      {"pause", "()V", reinterpret_cast<void*>(Pause)},
      {"resume", "()V", reinterpret_cast<void*>(Resume)},
  };
  int rc = env->RegisterNatives(c, methods, arraysize(methods));
  if (rc != JNI_OK) return rc;

  return JNI_VERSION_1_6;
}
