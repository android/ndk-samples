package com.raptrmatt.hellogl2kotlin

import android.content.Context
import android.graphics.PixelFormat
import android.opengl.GLSurfaceView
import android.util.Log
import javax.microedition.khronos.egl.EGL10
import javax.microedition.khronos.egl.EGLConfig
import javax.microedition.khronos.egl.EGLContext
import javax.microedition.khronos.egl.EGLDisplay
import javax.microedition.khronos.opengles.GL10


class GL2JNIView : GLSurfaceView{
    companion object {
        private val TAG: String = "GLView"
        private val DEBUG: Boolean = false

        private class ContextFactory : EGLContextFactory {
            companion object {
                private val EGL_CONTEXT_CLIENT_VERSION: Int = 0x3098
            }

            override fun createContext(egl: EGL10, display: EGLDisplay, eglConfig: EGLConfig): EGLContext {
                Log.w(TAG, "creating OpenGL ES 2.0 context")
                checkEglError("Before eglCreateContext", egl)
                val attribList: IntArray = intArrayOf(EGL_CONTEXT_CLIENT_VERSION, 2, EGL10.EGL_NONE)
                val context: EGLContext = egl.eglCreateContext(display, eglConfig, EGL10.EGL_NO_CONTEXT, attribList)
                checkEglError("after eglCreateContext", egl)
                return context
            }

            override fun destroyContext(egl: EGL10, display: EGLDisplay, context: EGLContext) {
                Log.w(TAG, "destroying OpenGL ES 2.0 context")
                egl.eglDestroyContext(display, context)
            }
        }

        private fun checkEglError(prompt: String, egl: EGL10) {
            var error: Int = egl.eglGetError()
            while(error != EGL10.EGL_SUCCESS) {
                Log.e(TAG, String.format("%s: EGL erro: 0x%x", prompt, error))
                error = egl.eglGetError()
            }
        }

        private class ConfigChooser(r: Int, g: Int, b: Int, a: Int, depth: Int, stencil: Int): EGLConfigChooser {
            protected val mRedSize: Int = r
            protected val mGreenSize: Int = g
            protected val mBlueSize: Int = b
            protected val mAlphaSize: Int = a
            protected val mDepthSize: Int = depth
            protected val mStencilSize: Int = stencil
            private val mValue: IntArray = intArrayOf(-1)

            companion object {
                private val EGL_OPEN_GL_ES2_BIT: Int = 4
                private val s_ConfigAttibs2: IntArray = intArrayOf(
                    EGL10.EGL_RED_SIZE, 4,
                    EGL10.EGL_GREEN_SIZE, 4,
                    EGL10.EGL_BLUE_SIZE, 4,
                    EGL10.EGL_RENDERABLE_TYPE, EGL_OPEN_GL_ES2_BIT,
                    EGL10.EGL_NONE
                )
            }

            override fun chooseConfig(egl: EGL10, display: EGLDisplay): EGLConfig? {
                Log.w(TAG, "choosing OpenGL ES 2.0 config")
                val num_config: IntArray = IntArray(1)
                egl.eglChooseConfig(display, s_ConfigAttibs2, null, 0, num_config)

                val numConfigs: Int = num_config[0]

                if (numConfigs <= 0) {
                    throw IllegalArgumentException("No configs match configSpec")
                }

                val configs: Array<EGLConfig?> = Array(numConfigs) { null }
                egl.eglChooseConfig(display, s_ConfigAttibs2, configs, numConfigs, num_config)
                val nonNullConfigs: Array<EGLConfig> = configs.requireNoNulls()

                if (DEBUG) {
                    printConfigs(egl, display, nonNullConfigs)
                }
                return chooseConfig(egl, display, nonNullConfigs)
            }

            fun chooseConfig(egl: EGL10, display: EGLDisplay, configs: Array<EGLConfig>): EGLConfig?{
                for (config: EGLConfig in configs) {
                    val d:Int = findConfigAttrib(egl, display, config, EGL10.EGL_DEPTH_SIZE, 0)
                    val s:Int = findConfigAttrib(egl, display, config, EGL10.EGL_STENCIL_SIZE, 0)

                    if (d < mDepthSize || s < mStencilSize) {
                        continue
                    }

                    val r:Int = findConfigAttrib(egl, display, config, EGL10.EGL_RED_SIZE, 0)
                    val g:Int = findConfigAttrib(egl, display, config, EGL10.EGL_GREEN_SIZE, 0)
                    val b:Int = findConfigAttrib(egl, display, config, EGL10.EGL_BLUE_SIZE, 0)
                    val a:Int = findConfigAttrib(egl, display, config, EGL10.EGL_ALPHA_SIZE, 0)

                    if (r == mRedSize && g == mGreenSize && b == mBlueSize && a == mAlphaSize) {
                        return config
                    }
                }
                return null
            }

            private fun findConfigAttrib(egl: EGL10, display: EGLDisplay, config: EGLConfig, attribute: Int, defaultValue: Int): Int {
                if (egl.eglGetConfigAttrib(display, config, attribute, mValue)) {
                    return mValue[0]
                }
                return defaultValue
            }

            private fun printConfigs(egl: EGL10, display: EGLDisplay, configs: Array<EGLConfig>) {
                val numConfigs: Int = configs.size
                Log.w(TAG, String.format("%d configurations", numConfigs))
                for(i in 0 until numConfigs) {
                    Log.w(TAG, String.format("Configuration %d:\n", i))
                    printConfig(egl, display, configs[i])
                }
            }

            private fun printConfig(egl: EGL10, display: EGLDisplay, config:EGLConfig) {
                val attributes = intArrayOf(
                    EGL10.EGL_BUFFER_SIZE,
                    EGL10.EGL_ALPHA_SIZE,
                    EGL10.EGL_BLUE_SIZE,
                    EGL10.EGL_GREEN_SIZE,
                    EGL10.EGL_RED_SIZE,
                    EGL10.EGL_DEPTH_SIZE,
                    EGL10.EGL_STENCIL_SIZE,
                    EGL10.EGL_CONFIG_CAVEAT,
                    EGL10.EGL_CONFIG_ID,
                    EGL10.EGL_LEVEL,
                    EGL10.EGL_MAX_PBUFFER_HEIGHT,
                    EGL10.EGL_MAX_PBUFFER_PIXELS,
                    EGL10.EGL_MAX_PBUFFER_WIDTH,
                    EGL10.EGL_NATIVE_RENDERABLE,
                    EGL10.EGL_NATIVE_VISUAL_ID,
                    EGL10.EGL_NATIVE_VISUAL_TYPE,
                    0x3030,  // EGL10.EGL_PRESERVED_RESOURCES,
                    EGL10.EGL_SAMPLES,
                    EGL10.EGL_SAMPLE_BUFFERS,
                    EGL10.EGL_SURFACE_TYPE,
                    EGL10.EGL_TRANSPARENT_TYPE,
                    EGL10.EGL_TRANSPARENT_RED_VALUE,
                    EGL10.EGL_TRANSPARENT_GREEN_VALUE,
                    EGL10.EGL_TRANSPARENT_BLUE_VALUE,
                    0x3039,  // EGL10.EGL_BIND_TO_TEXTURE_RGB,
                    0x303A,  // EGL10.EGL_BIND_TO_TEXTURE_RGBA,
                    0x303B,  // EGL10.EGL_MIN_SWAP_INTERVAL,
                    0x303C,  // EGL10.EGL_MAX_SWAP_INTERVAL,
                    EGL10.EGL_LUMINANCE_SIZE,
                    EGL10.EGL_ALPHA_MASK_SIZE,
                    EGL10.EGL_COLOR_BUFFER_TYPE,
                    EGL10.EGL_RENDERABLE_TYPE,
                    0x3042 // EGL10.EGL_CONFORMANT
                )
                val names = arrayOf(
                    "EGL_BUFFER_SIZE",
                    "EGL_ALPHA_SIZE",
                    "EGL_BLUE_SIZE",
                    "EGL_GREEN_SIZE",
                    "EGL_RED_SIZE",
                    "EGL_DEPTH_SIZE",
                    "EGL_STENCIL_SIZE",
                    "EGL_CONFIG_CAVEAT",
                    "EGL_CONFIG_ID",
                    "EGL_LEVEL",
                    "EGL_MAX_PBUFFER_HEIGHT",
                    "EGL_MAX_PBUFFER_PIXELS",
                    "EGL_MAX_PBUFFER_WIDTH",
                    "EGL_NATIVE_RENDERABLE",
                    "EGL_NATIVE_VISUAL_ID",
                    "EGL_NATIVE_VISUAL_TYPE",
                    "EGL_PRESERVED_RESOURCES",
                    "EGL_SAMPLES",
                    "EGL_SAMPLE_BUFFERS",
                    "EGL_SURFACE_TYPE",
                    "EGL_TRANSPARENT_TYPE",
                    "EGL_TRANSPARENT_RED_VALUE",
                    "EGL_TRANSPARENT_GREEN_VALUE",
                    "EGL_TRANSPARENT_BLUE_VALUE",
                    "EGL_BIND_TO_TEXTURE_RGB",
                    "EGL_BIND_TO_TEXTURE_RGBA",
                    "EGL_MIN_SWAP_INTERVAL",
                    "EGL_MAX_SWAP_INTERVAL",
                    "EGL_LUMINANCE_SIZE",
                    "EGL_ALPHA_MASK_SIZE",
                    "EGL_COLOR_BUFFER_TYPE",
                    "EGL_RENDERABLE_TYPE",
                    "EGL_CONFORMANT"
                )

                val value: IntArray = intArrayOf(-1)
                for (i in 0 until attributes.size) {
                    val attribute: Int = attributes[i]
                    val name: String = names[i]
                    if (egl.eglGetConfigAttrib(display, config, attribute, value)) {
                        Log.w(TAG, String.format("  %s: %d\n", name, value[0]))
                    } else {
                        Log.w(TAG, String.format(  "%s: failed\n", name))
                        while (egl.eglGetError() != EGL10.EGL_SUCCESS);
                    }
                }
            }

        }

        private class Renderer: GLSurfaceView.Renderer {
            override fun onDrawFrame(gl: GL10) {
                GL2JNILib.step()
            }

            override fun onSurfaceChanged(gl: GL10, width: Int, height: Int) {
                Log.e(TAG, "surface change $width : $height")
                GL2JNILib.init(width, height)
            }

            override fun onSurfaceCreated(gl: GL10, config: EGLConfig) {
                Log.e(TAG, "surface create")
                // No-op
            }
        }
    }

    constructor(context: Context) : super(context) {
        create(false, 0, 0)
    }

    constructor(context: Context, translucent: Boolean, depth: Int, stencil: Int) : super(context) {
        create(translucent, depth, stencil)
    }

    private fun create(translucent: Boolean, depth: Int, stencil: Int) {
        Log.e(TAG, "CREATE")
        if(translucent) {
            holder.setFormat(PixelFormat.TRANSLUCENT)
        }

        setEGLContextFactory(ContextFactory())

        if(translucent) {
            setEGLConfigChooser(ConfigChooser(8,8,8,8,depth, stencil))
        } else {
            setEGLConfigChooser(ConfigChooser(5,6,5,0,depth, stencil))
        }

        setRenderer(Renderer())
    }
}