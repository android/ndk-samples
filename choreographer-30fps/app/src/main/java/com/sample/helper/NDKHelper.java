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

package com.sample.helper;

import java.io.ByteArrayOutputStream;
import java.io.File;
import java.io.FileInputStream;
import java.nio.ByteBuffer;

import javax.microedition.khronos.opengles.GL10;

import android.R.bool;
import android.opengl.GLES30;

import android.annotation.TargetApi;
import android.app.Activity;
import android.app.NativeActivity;
import android.content.Context;
import android.content.pm.ApplicationInfo;
import android.content.pm.PackageManager;
import android.content.pm.PackageManager.NameNotFoundException;
import android.graphics.Bitmap;
import android.graphics.Bitmap.CompressFormat;
import android.graphics.BitmapFactory;
import android.graphics.Matrix;
import android.media.AudioManager;
import android.media.AudioTrack;
import android.opengl.GLUtils;
import android.os.Build;
import android.util.Log;
import android.view.View;
import android.view.View.MeasureSpec;

@TargetApi(Build.VERSION_CODES.GINGERBREAD)
public class NDKHelper {

    public NDKHelper(NativeActivity act) {
        activity = act;
    }

    public void loadLibrary(String soname) {
        if (soname.isEmpty() == false) {
            System.loadLibrary(soname);
            loadedSO = true;
        }
    }

    public static Boolean checkSOLoaded() {
        if (loadedSO == false) {
            Log.e("NDKHelper",
                    "--------------------------------------------\n"
                            + ".so has not been loaded. To use JUI helper, please initialize with \n"
                            + "NDKHelper::Init( ANativeActivity* activity, const char* helper_class_name, const char* native_soname);\n"
                            + "--------------------------------------------\n");
            return false;
        } else
            return true;
    }

    private static boolean loadedSO = false;
    NativeActivity activity;



    //
    // Load Bitmap
    // Java helper is useful decoding PNG, TIFF etc rather than linking libPng
    // etc separately
    //
    private int nextPOT(int i) {
        int pot = 1;
        while (pot < i)
            pot <<= 1;
        return pot;
    }

    private Bitmap scaleBitmap(Bitmap bitmapToScale, float newWidth,
                               float newHeight) {
        if (bitmapToScale == null)
            return null;
        // get the original width and height
        int width = bitmapToScale.getWidth();
        int height = bitmapToScale.getHeight();
        // create a matrix for the manipulation
        Matrix matrix = new Matrix();

        // resize the bit map
        matrix.postScale(newWidth / width, newHeight / height);

        // recreate the new Bitmap and set it back
        return Bitmap.createBitmap(bitmapToScale, 0, 0,
                bitmapToScale.getWidth(), bitmapToScale.getHeight(), matrix,
                true);
    }

    public class TextureInformation {
        boolean ret;
        boolean alphaChannel;
        int originalWidth;
        int originalHeight;
        Object image;
    }

    public Object loadTexture(String path) {
        Bitmap bitmap = null;
        TextureInformation info = new TextureInformation();
        try {
            String str = path;
            if (!path.startsWith("/")) {
                str = "/" + path;
            }

            File file = new File(activity.getExternalFilesDir(null), str);
            if (file.canRead()) {
                bitmap = BitmapFactory.decodeStream(new FileInputStream(file));
            } else {
                bitmap = BitmapFactory.decodeStream(activity.getResources()
                        .getAssets().open(path));
            }
            // Matrix matrix = new Matrix();
            // // resize the bit map
            // matrix.postScale(-1F, 1F);
            //
            // // recreate the new Bitmap and set it back
            // bitmap = Bitmap.createBitmap(bitmap, 0, 0, bitmap.getWidth(),
            // bitmap.getHeight(), matrix, true);

        } catch (Exception e) {
            Log.w("NDKHelper", "Coundn't load a file:" + path);
            info.ret = false;
            return info;
        }

        if (bitmap != null) {
            GLUtils.texImage2D(GL10.GL_TEXTURE_2D, 0, bitmap, 0);
        }
        info.ret = true;
        info.alphaChannel = bitmap.hasAlpha();
        info.originalWidth = getBitmapWidth(bitmap);
        info.originalHeight = getBitmapHeight(bitmap);

        return info;
    }

    public Object loadCubemapTexture(String path, int face, int miplevel, boolean sRGB) {
        Bitmap bitmap = null;
        TextureInformation info = new TextureInformation();
        try {
            String str = path;
            if (!path.startsWith("/")) {
                str = "/" + path;
            }

            File file = new File(activity.getExternalFilesDir(null), str);
            if (file.canRead()) {
                bitmap = BitmapFactory.decodeStream(new FileInputStream(file));
            } else {
                bitmap = BitmapFactory.decodeStream(activity.getResources()
                        .getAssets().open(path));
            }
        } catch (Exception e) {
            Log.w("NDKHelper", "Coundn't load a file:" + path);
            info.ret = false;
            return info;
        }

        if (bitmap != null) {
            if (sRGB)
            {
//        		GLUtils.texImage2D(face, miplevel, bitmap, 0);
//        		GLUtils.texImage2D(face, miplevel,
//        				GLUtils.getInternalFormat(bitmap), bitmap, 0);
//        		int i = GLUtils.getInternalFormat(bitmap);
//        		if( i == GL10.GL_RGBA)
//        		{
//            		GLUtils.texImage2D(face, miplevel,
//            				GLES30.GL_SRGB, bitmap, 0);
//        		}
                //Leave them for now
                GLUtils.texImage2D(face, miplevel, bitmap, 0);
            }
            else
                GLUtils.texImage2D(face, miplevel, bitmap, 0);
        }
        info.ret = true;
        info.alphaChannel = bitmap.hasAlpha();
        info.originalWidth = getBitmapWidth(bitmap);
        info.originalHeight = getBitmapHeight(bitmap);

        return info;

    }

    public Object loadImage(String path) {
        Bitmap bitmap = null;
        TextureInformation info = new TextureInformation();
        try {
            String str = path;
            if (!path.startsWith("/")) {
                str = "/" + path;
            }

            File file = new File(activity.getExternalFilesDir(null), str);
            if (file.canRead()) {
                bitmap = BitmapFactory.decodeStream(new FileInputStream(file));
            } else {
                bitmap = BitmapFactory.decodeStream(activity.getResources()
                        .getAssets().open(path));
            }
        } catch (Exception e) {
            Log.w("NDKHelper", "Coundn't load a file:" + path);
            info.ret = false;
            return info;
        }

        if (bitmap != null) {
            GLUtils.texImage2D(GL10.GL_TEXTURE_2D, 0, bitmap, 0);
        }
        info.ret = true;
        info.alphaChannel = bitmap.hasAlpha();
        info.originalWidth = getBitmapWidth(bitmap);
        info.originalHeight = getBitmapHeight(bitmap);

        int iBytes = bitmap.getWidth() * bitmap.getHeight() * 4;
        ByteBuffer buffer = ByteBuffer.allocateDirect(iBytes);

        bitmap.copyPixelsToBuffer(buffer);
        info.image = buffer;
        return info;
    }

    public Bitmap openBitmap(String path, boolean iScalePOT) {
        Bitmap bitmap = null;
        try {
            bitmap = BitmapFactory.decodeStream(activity.getResources()
                    .getAssets().open(path));
            if (iScalePOT) {
                int originalWidth = getBitmapWidth(bitmap);
                int originalHeight = getBitmapHeight(bitmap);
                int width = nextPOT(originalWidth);
                int height = nextPOT(originalHeight);
                if (originalWidth != width || originalHeight != height) {
                    // Scale it
                    bitmap = scaleBitmap(bitmap, width, height);
                }
            }

        } catch (Exception e) {
            Log.w("NDKHelper", "Coundn't load a file:" + path);
        }

        return bitmap;
    }

    public int getBitmapWidth(Bitmap bmp) {
        return bmp.getWidth();
    }

    public int getBitmapHeight(Bitmap bmp) {
        return bmp.getHeight();
    }

    public void getBitmapPixels(Bitmap bmp, int[] pixels) {
        int w = bmp.getWidth();
        int h = bmp.getHeight();
        bmp.getPixels(pixels, 0, w, 0, 0, w, h);
    }

    public void closeBitmap(Bitmap bmp) {
        bmp.recycle();
    }

    public String getNativeLibraryDirectory(Context appContext) {
        ApplicationInfo ai = activity.getApplicationInfo();

        Log.w("NDKHelper", "ai.nativeLibraryDir:" + ai.nativeLibraryDir);

        if ((ai.flags & ApplicationInfo.FLAG_UPDATED_SYSTEM_APP) != 0
                || (ai.flags & ApplicationInfo.FLAG_SYSTEM) == 0) {
            return ai.nativeLibraryDir;
        }
        return "/system/lib/";
    }

    public String getApplicationName() {
        final PackageManager pm = activity.getPackageManager();
        ApplicationInfo ai;
        try {
            ai = pm.getApplicationInfo(activity.getPackageName(), 0);
        } catch (final NameNotFoundException e) {
            ai = null;
        }
        String applicationName = (String) (ai != null ? pm
                .getApplicationLabel(ai) : "(unknown)");
        return applicationName;
    }

    public String getStringResource(String resourceName)
    {
        int id = activity.getResources().getIdentifier(resourceName, "string", activity.getPackageName());
        String value = id == 0 ? "" : (String)activity.getResources().getText(id);
        return value;
    }

    //
    // Audio related helpers
    //
    @TargetApi(Build.VERSION_CODES.JELLY_BEAN_MR1)
    public int getNativeAudioBufferSize() {
        int SDK_INT = android.os.Build.VERSION.SDK_INT;
        if (SDK_INT >= 17) {
            AudioManager am = (AudioManager) activity
                    .getSystemService(Context.AUDIO_SERVICE);
            String framesPerBuffer = am
                    .getProperty(AudioManager.PROPERTY_OUTPUT_FRAMES_PER_BUFFER);
            return Integer.parseInt(framesPerBuffer);
        } else {
            return 0;
        }
    }

    public int getNativeAudioSampleRate() {
        return AudioTrack.getNativeOutputSampleRate(AudioManager.STREAM_SYSTEM);
    }

    /*
     * Helper to execute function in UIThread
     */
    public void runOnUIThread(final long p) {
        if (checkSOLoaded()) {
            activity.runOnUiThread(new Runnable() {
                @Override
                public void run() {
                    RunOnUiThreadHandler(p);
                }
            });
        }
        return;
    }

    /*
     * Native code helper for RunOnUiThread
     */
    native public void RunOnUiThreadHandler(long pointer);
}
