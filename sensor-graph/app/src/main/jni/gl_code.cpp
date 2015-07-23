/*
 * Copyright (C) 2009 The Android Open Source Project
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

#include <jni.h>
#include <android/log.h>

#include <GLES2/gl2.h>

#include <cstdio>
#include <cstdlib>
#include <cassert>
#include <cmath>
#include <string>

#include <android/asset_manager_jni.h>
#include <android/asset_manager.h>

#define  LOG_TAG    "libgl2jni"
#define  LOGI(...)  __android_log_print(ANDROID_LOG_INFO,LOG_TAG,__VA_ARGS__)
#define  LOGE(...)  __android_log_print(ANDROID_LOG_ERROR,LOG_TAG,__VA_ARGS__)


const int SENSOR_HISTORY_LENGTH = 100;
GLfloat gSensorData[SENSOR_HISTORY_LENGTH*2];
int gSensorDataIndex = 0;
GLfloat gPositionX[SENSOR_HISTORY_LENGTH];
void initializePositionX() {
    for (auto i = 0; i < SENSOR_HISTORY_LENGTH; i++) {
        float t = float(i) / float(SENSOR_HISTORY_LENGTH - 1);
        gPositionX[i] = -1.f * (1.f - t) + 1.f * t;
    }
}

static void printGLString(const char *name, GLenum s) {
    const char *v = (const char *) glGetString(s);
    LOGI("GL %s = %s\n", name, v);
}

static void checkGlError(const char* op) {
    for (GLint error = glGetError(); error; error
            = glGetError()) {
        LOGI("after %s() glError (0x%x)\n", op, error);
    }
}

GLuint loadShader(GLenum shaderType, const std::string& pSource) {
    GLuint shader = glCreateShader(shaderType);
    if (shader) {
        const char *sourceBuf = pSource.c_str();
        glShaderSource(shader, 1, &sourceBuf, NULL);
        glCompileShader(shader);
        GLint compiled = 0;
        glGetShaderiv(shader, GL_COMPILE_STATUS, &compiled);
        if (!compiled) {
            GLint infoLen = 0;
            glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &infoLen);
            if (infoLen) {
                char* buf = (char*) malloc(infoLen);
                if (buf) {
                    glGetShaderInfoLog(shader, infoLen, NULL, buf);
                    LOGE("Could not compile shader %d:\n%s\n",
                            shaderType, buf);
                    free(buf);
                }
                glDeleteShader(shader);
                shader = 0;
            }
        }
    }
    return shader;
}

GLuint createProgram(const std::string& pVertexSource, const std::string& pFragmentSource) {
    GLuint vertexShader = loadShader(GL_VERTEX_SHADER, pVertexSource);
    if (!vertexShader) {
        return 0;
    }

    GLuint pixelShader = loadShader(GL_FRAGMENT_SHADER, pFragmentSource);
    if (!pixelShader) {
        return 0;
    }

    GLuint program = glCreateProgram();
    if (program) {
        glAttachShader(program, vertexShader);
        checkGlError("glAttachShader");
        glAttachShader(program, pixelShader);
        checkGlError("glAttachShader");
        glLinkProgram(program);
        GLint linkStatus = GL_FALSE;
        glGetProgramiv(program, GL_LINK_STATUS, &linkStatus);
        if (linkStatus != GL_TRUE) {
            GLint bufLength = 0;
            glGetProgramiv(program, GL_INFO_LOG_LENGTH, &bufLength);
            if (bufLength) {
                char* buf = (char*) malloc(bufLength);
                if (buf) {
                    glGetProgramInfoLog(program, bufLength, NULL, buf);
                    LOGE("Could not link program:\n%s\n", buf);
                    free(buf);
                }
            }
            glDeleteProgram(program);
            program = 0;
        }
    }
    return program;
}

GLuint gProgram;
GLuint gvPositionXHandle;
GLuint gvSensorValueHandle;


bool init(AAssetManager *assetManager, jint w, jint h) {
    printGLString("Version", GL_VERSION);
    printGLString("Vendor", GL_VENDOR);
    printGLString("Renderer", GL_RENDERER);
    printGLString("Extensions", GL_EXTENSIONS);

    LOGI("setupGraphics(%d, %d)", w, h);
    AAsset *vertexShader = AAssetManager_open(assetManager, "shader.glslv", AASSET_MODE_BUFFER);
    assert(vertexShader != NULL);
    const void *vertexShaderBuf = AAsset_getBuffer(vertexShader);
    assert(vertexShaderBuf != NULL);
    int vertexShaderLength = AAsset_getLength(vertexShader);
    std::string vertexShaderSource((const char*)vertexShaderBuf, vertexShaderLength);

    AAsset *fragmentShader = AAssetManager_open(assetManager, "shader.glslf", AASSET_MODE_BUFFER);
    assert(fragmentShader != NULL);
    const void *fragmentShaderBuf = AAsset_getBuffer(fragmentShader);
    assert(fragmentShaderBuf != NULL);
    int fragmentShaderLength = AAsset_getLength(fragmentShader);
    std::string fragmentShaderSource((const char*)fragmentShaderBuf, fragmentShaderLength);


    gProgram = createProgram(vertexShaderSource, fragmentShaderSource);
    if (!gProgram) {
        LOGE("Could not create program.");
        return false;
    }
    gvPositionXHandle = glGetAttribLocation(gProgram, "vPositionX");
    checkGlError("glGetAttribLocation");
    LOGI("glGetAttribLocation(\"vPositionX\") = %d\n",
            gvPositionXHandle);

    gvSensorValueHandle = glGetAttribLocation(gProgram, "vSensorValue");
    checkGlError("glGetAttribLocation");
    LOGI("glGetAttribLocation(\"vSensorValue\") = %d\n",
         gvSensorValueHandle);

    glViewport(0, 0, w, h);
    checkGlError("glViewport");

    initializePositionX();
    return true;
}

void update() {

    float value = cos(gSensorDataIndex);
    gSensorData[gSensorDataIndex] = value;
    gSensorData[SENSOR_HISTORY_LENGTH+gSensorDataIndex] = value;
    gSensorDataIndex = (gSensorDataIndex + 1) % SENSOR_HISTORY_LENGTH;
}

void render() {
    glClearColor(0.f, 0.f, 0.f, 1.0f);
    checkGlError("glClearColor");
    glClear( GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
    checkGlError("glClear");

    glUseProgram(gProgram);
    checkGlError("glUseProgram");

    glVertexAttribPointer(gvPositionXHandle, 1, GL_FLOAT, GL_FALSE, 0, gPositionX);
    checkGlError("glVertexAttribPointer");
    glEnableVertexAttribArray(gvPositionXHandle);
    checkGlError("glEnableVertexAttribArray");
    glVertexAttribPointer(gvSensorValueHandle, 1, GL_FLOAT, GL_FALSE, 0, gSensorData+gSensorDataIndex);
    checkGlError("glVertexAttribPointer");
    glEnableVertexAttribArray(gvSensorValueHandle);
    checkGlError("glEnableVertexAttribArray");
    glDrawArrays(GL_LINE_STRIP, 0, SENSOR_HISTORY_LENGTH);
    checkGlError("glDrawArrays");
}

extern "C" {
    JNIEXPORT void JNICALL
    Java_com_android_gl2jni_GL2JNILib_init(JNIEnv *env, jclass type, jobject assetManager, jint width,
                                           jint height) {
        AAssetManager *nativeAssetManager = AAssetManager_fromJava(env, assetManager);
        init(nativeAssetManager, width, height);
    }


    JNIEXPORT void JNICALL
    Java_com_android_gl2jni_GL2JNILib_step(JNIEnv *env, jclass type) {
        update();
        render();
    }
}