/*
 * Copyright (C) 2011 The Android Open Source Project
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

package com.example.nativemedia;

import android.graphics.SurfaceTexture;
import android.util.Log;

import java.nio.ByteBuffer;
import java.nio.ByteOrder;
import java.nio.FloatBuffer;

import javax.microedition.khronos.egl.EGLConfig;
import javax.microedition.khronos.opengles.GL10;

import android.content.Context;

import android.opengl.GLES20;
import android.opengl.GLSurfaceView;
import android.opengl.Matrix;

import android.util.AttributeSet;

public class MyGLSurfaceView extends GLSurfaceView {

    MyRenderer mRenderer;

    public MyGLSurfaceView(Context context) {
        this(context, null);
    }

    public MyGLSurfaceView(Context context, AttributeSet attributeSet) {
        super(context, attributeSet);
        init();
    }

    private void init() {
        setEGLContextClientVersion(2);
        mRenderer = new MyRenderer();
        setRenderer(mRenderer);
    }

    @Override
    public void onPause() {
        super.onPause();
    }

    @Override
    public void onResume() {
        super.onResume();
    }

    public SurfaceTexture getSurfaceTexture() {
        return mRenderer.getSurfaceTexture();
    }
}

class MyRenderer implements GLSurfaceView.Renderer, SurfaceTexture.OnFrameAvailableListener {

    public MyRenderer() {
        mVertices = ByteBuffer.allocateDirect(mVerticesData.length
                * FLOAT_SIZE_BYTES).order(ByteOrder.nativeOrder()).asFloatBuffer();
        mVertices.put(mVerticesData).position(0);

        Matrix.setIdentityM(mSTMatrix, 0);
        Matrix.setIdentityM(mMMatrix, 0);
        Matrix.rotateM(mMMatrix, 0, 20, 0, 1, 0);
    }

    public void onDrawFrame(GL10 glUnused) {
        synchronized(this) {
            if (updateSurface) {
                mSurface.updateTexImage();

                mSurface.getTransformMatrix(mSTMatrix);
                updateSurface = false;
            }
        }

        // Ignore the passed-in GL10 interface, and use the GLES20
        // class's static methods instead.
        GLES20.glClear( GLES20.GL_DEPTH_BUFFER_BIT | GLES20.GL_COLOR_BUFFER_BIT);
        GLES20.glUseProgram(mProgram);
        checkGlError("glUseProgram");

        GLES20.glActiveTexture(GLES20.GL_TEXTURE0);
        GLES20.glBindTexture(GL_TEXTURE_EXTERNAL_OES, mTextureID);

        mVertices.position(VERTICES_DATA_POS_OFFSET);
        GLES20.glVertexAttribPointer(maPositionHandle, 3, GLES20.GL_FLOAT, false,
                VERTICES_DATA_STRIDE_BYTES, mVertices);
        checkGlError("glVertexAttribPointer maPosition");
        GLES20.glEnableVertexAttribArray(maPositionHandle);
        checkGlError("glEnableVertexAttribArray maPositionHandle");

        mVertices.position(VERTICES_DATA_UV_OFFSET);
        GLES20.glVertexAttribPointer(maTextureHandle, 3, GLES20.GL_FLOAT, false,
                VERTICES_DATA_STRIDE_BYTES, mVertices);
        checkGlError("glVertexAttribPointer maTextureHandle");
        GLES20.glEnableVertexAttribArray(maTextureHandle);
        checkGlError("glEnableVertexAttribArray maTextureHandle");

        Matrix.multiplyMM(mMVPMatrix, 0, mVMatrix, 0, mMMatrix, 0);
        Matrix.multiplyMM(mMVPMatrix, 0, mProjMatrix, 0, mMVPMatrix, 0);

        GLES20.glUniformMatrix4fv(muMVPMatrixHandle, 1, false, mMVPMatrix, 0);
        GLES20.glUniformMatrix4fv(muSTMatrixHandle, 1, false, mSTMatrix, 0);

        GLES20.glDrawArrays(GLES20.GL_TRIANGLE_STRIP, 0, 4);
        checkGlError("glDrawArrays");
    }

    public void onSurfaceChanged(GL10 glUnused, int width, int height) {
        // Ignore the passed-in GL10 interface, and use the GLES20
        // class's static methods instead.
        GLES20.glViewport(0, 0, width, height);
        mRatio = (float) width / height;
        Matrix.frustumM(mProjMatrix, 0, -mRatio, mRatio, -1, 1, 3, 7);
    }

    public void onSurfaceCreated(GL10 glUnused, EGLConfig config) {
        // Ignore the passed-in GL10 interface, and use the GLES20
        // class's static methods instead.

        /* Set up alpha blending and an Android background color */
        GLES20.glEnable(GLES20.GL_BLEND);
        GLES20.glBlendFunc(GLES20.GL_SRC_ALPHA, GLES20.GL_ONE_MINUS_SRC_ALPHA);
        GLES20.glClearColor(0.643f, 0.776f, 0.223f, 1.0f);

        /* Set up shaders and handles to their variables */
        mProgram = createProgram(mVertexShader, mFragmentShader);
        if (mProgram == 0) {
            return;
        }
        maPositionHandle = GLES20.glGetAttribLocation(mProgram, "aPosition");
        checkGlError("glGetAttribLocation aPosition");
        if (maPositionHandle == -1) {
            throw new RuntimeException("Could not get attrib location for aPosition");
        }
        maTextureHandle = GLES20.glGetAttribLocation(mProgram, "aTextureCoord");
        checkGlError("glGetAttribLocation aTextureCoord");
        if (maTextureHandle == -1) {
            throw new RuntimeException("Could not get attrib location for aTextureCoord");
        }

        muMVPMatrixHandle = GLES20.glGetUniformLocation(mProgram, "uMVPMatrix");
        checkGlError("glGetUniformLocation uMVPMatrix");
        if (muMVPMatrixHandle == -1) {
            throw new RuntimeException("Could not get attrib location for uMVPMatrix");
        }

        muSTMatrixHandle = GLES20.glGetUniformLocation(mProgram, "uSTMatrix");
        checkGlError("glGetUniformLocation uSTMatrix");
        if (muMVPMatrixHandle == -1) {
            throw new RuntimeException("Could not get attrib location for uSTMatrix");
        }

        checkGlError("glGetUniformLocation uCRatio");
        if (muMVPMatrixHandle == -1) {
            throw new RuntimeException("Could not get attrib location for uCRatio");
        }

        /*
         * Create our texture. This has to be done each time the
         * surface is created.
         */

        int[] textures = new int[1];
        GLES20.glGenTextures(1, textures, 0);

        mTextureID = textures[0];
        GLES20.glBindTexture(GL_TEXTURE_EXTERNAL_OES, mTextureID);
        checkGlError("glBindTexture mTextureID");

        // Can't do mipmapping with camera source
        GLES20.glTexParameterf(GL_TEXTURE_EXTERNAL_OES, GLES20.GL_TEXTURE_MIN_FILTER,
                GLES20.GL_NEAREST);
        GLES20.glTexParameterf(GL_TEXTURE_EXTERNAL_OES, GLES20.GL_TEXTURE_MAG_FILTER,
                GLES20.GL_LINEAR);
        // Clamp to edge is the only option
        GLES20.glTexParameteri(GL_TEXTURE_EXTERNAL_OES, GLES20.GL_TEXTURE_WRAP_S,
                GLES20.GL_CLAMP_TO_EDGE);
        GLES20.glTexParameteri(GL_TEXTURE_EXTERNAL_OES, GLES20.GL_TEXTURE_WRAP_T,
                GLES20.GL_CLAMP_TO_EDGE);
        checkGlError("glTexParameteri mTextureID");

        /*
         * Create the SurfaceTexture that will feed this textureID, and pass it to the camera
         */

        mSurface = new SurfaceTexture(mTextureID);
        mSurface.setOnFrameAvailableListener(this);

        Matrix.setLookAtM(mVMatrix, 0, 0, 0, 4f, 0f, 0f, 0f, 0f, 1.0f, 0.0f);

        synchronized(this) {
            updateSurface = false;
        }
    }

    synchronized public void onFrameAvailable(SurfaceTexture surface) {
        /* For simplicity, SurfaceTexture calls here when it has new
         * data available.  Call may come in from some random thread,
         * so let's be safe and use synchronize. No OpenGL calls can be done here.
         */
        updateSurface = true;
        //Log.v(TAG, "onFrameAvailable " + surface.getTimestamp());
    }

    private int loadShader(int shaderType, String source) {
        int shader = GLES20.glCreateShader(shaderType);
        if (shader != 0) {
            GLES20.glShaderSource(shader, source);
            GLES20.glCompileShader(shader);
            int[] compiled = new int[1];
            GLES20.glGetShaderiv(shader, GLES20.GL_COMPILE_STATUS, compiled, 0);
            if (compiled[0] == 0) {
                Log.e(TAG, "Could not compile shader " + shaderType + ":");
                Log.e(TAG, GLES20.glGetShaderInfoLog(shader));
                GLES20.glDeleteShader(shader);
                shader = 0;
            }
        }
        return shader;
    }

    private int createProgram(String vertexSource, String fragmentSource) {
        int vertexShader = loadShader(GLES20.GL_VERTEX_SHADER, vertexSource);
        if (vertexShader == 0) {
            return 0;
        }
        int pixelShader = loadShader(GLES20.GL_FRAGMENT_SHADER, fragmentSource);
        if (pixelShader == 0) {
            return 0;
        }

        int program = GLES20.glCreateProgram();
        if (program != 0) {
            GLES20.glAttachShader(program, vertexShader);
            checkGlError("glAttachShader");
            GLES20.glAttachShader(program, pixelShader);
            checkGlError("glAttachShader");
            GLES20.glLinkProgram(program);
            int[] linkStatus = new int[1];
            GLES20.glGetProgramiv(program, GLES20.GL_LINK_STATUS, linkStatus, 0);
            if (linkStatus[0] != GLES20.GL_TRUE) {
                Log.e(TAG, "Could not link program: ");
                Log.e(TAG, GLES20.glGetProgramInfoLog(program));
                GLES20.glDeleteProgram(program);
                program = 0;
            }
        }
        return program;
    }

    private void checkGlError(String op) {
        int error;
        while ((error = GLES20.glGetError()) != GLES20.GL_NO_ERROR) {
            Log.e(TAG, op + ": glError " + error);
            throw new RuntimeException(op + ": glError " + error);
        }
    }

    private static final int FLOAT_SIZE_BYTES = 4;
    private static final int VERTICES_DATA_STRIDE_BYTES = 5 * FLOAT_SIZE_BYTES;
    private static final int VERTICES_DATA_POS_OFFSET = 0;
    private static final int VERTICES_DATA_UV_OFFSET = 3;
    private final float[] mVerticesData = {
        // X, Y, Z, U, V
        -1.0f, -1.0f, 0, 0.f, 0.f,
        1.0f, -1.0f, 0, 1.f, 0.f,
        -1.0f,  1.0f, 0, 0.f, 1.f,
        1.0f,   1.0f, 0, 1.f, 1.f,
    };

    private FloatBuffer mVertices;

    private final String mVertexShader =
        "uniform mat4 uMVPMatrix;\n" +
        "uniform mat4 uSTMatrix;\n" +
        "attribute vec4 aPosition;\n" +
        "attribute vec4 aTextureCoord;\n" +
        "varying vec2 vTextureCoord;\n" +
        "void main() {\n" +
        "  gl_Position = uMVPMatrix * aPosition;\n" +
        "  vTextureCoord = (uSTMatrix * aTextureCoord).xy;\n" +
        "}\n";

    private final String mFragmentShader =
        "#extension GL_OES_EGL_image_external : require\n" +
        "precision mediump float;\n" +
        "varying vec2 vTextureCoord;\n" +
        "uniform samplerExternalOES sTexture;\n" +
        "void main() {\n" +
        "  gl_FragColor = texture2D(sTexture, vTextureCoord);\n" +
        "}\n";

    private float[] mMVPMatrix = new float[16];
    private float[] mProjMatrix = new float[16];
    private float[] mMMatrix = new float[16];
    private float[] mVMatrix = new float[16];
    private float[] mSTMatrix = new float[16];

    private int mProgram;
    private int mTextureID;
    private int muMVPMatrixHandle;
    private int muSTMatrixHandle;
    private int maPositionHandle;
    private int maTextureHandle;

    private float mRatio = 1.0f;
    private SurfaceTexture mSurface;
    private boolean updateSurface = false;

    private static final String TAG = "MyRenderer";

    // Magic key
    private static final int GL_TEXTURE_EXTERNAL_OES = 0x8D65;

    public SurfaceTexture getSurfaceTexture() {
        return mSurface;
    }
}
