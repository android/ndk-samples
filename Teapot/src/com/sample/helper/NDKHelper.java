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

import java.io.File;
import java.io.FileInputStream;

import javax.microedition.khronos.opengles.GL10;

import android.content.Context;
import android.content.pm.ApplicationInfo;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.graphics.Matrix;
import android.media.AudioManager;
import android.media.AudioTrack;
import android.opengl.GLUtils;
import android.util.Log;

public class NDKHelper
{
    private static Context context;

    public static void setContext(Context c)
    {
        Log.i("NDKHelper", "setContext:" + c);
        context = c;
    }

    //
    // Load Bitmap
    // Java helper is useful decoding PNG, TIFF etc rather than linking libPng
    // etc separately
    //
    private int nextPOT(int i)
    {
        int pot = 1;
        while (pot < i)
            pot <<= 1;
        return pot;
    }

    private Bitmap scaleBitmap(Bitmap bitmapToScale, float newWidth, float newHeight)
    {
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
        return Bitmap.createBitmap(bitmapToScale, 0, 0, bitmapToScale.getWidth(),
                bitmapToScale.getHeight(), matrix, true);
    }

    public boolean loadTexture(String path)
    {
        Bitmap bitmap = null;
        try
        {
            String str = path;
            if (!path.startsWith("/"))
            {
                str = "/" + path;
            }

            File file = new File(context.getExternalFilesDir(null), str);
            if (file.canRead())
            {
                bitmap = BitmapFactory.decodeStream(new FileInputStream(file));
            } else
            {
                bitmap = BitmapFactory.decodeStream(context.getResources().getAssets()
                        .open(path));
            }
            // Matrix matrix = new Matrix();
            // // resize the bit map
            // matrix.postScale(-1F, 1F);
            //
            // // recreate the new Bitmap and set it back
            // bitmap = Bitmap.createBitmap(bitmap, 0, 0, bitmap.getWidth(),
            // bitmap.getHeight(), matrix, true);

        } catch (Exception e)
        {
            Log.w("NDKHelper", "Coundn't load a file:" + path);
            return false;
        }

        if (bitmap != null)
        {
            GLUtils.texImage2D(GL10.GL_TEXTURE_2D, 0, bitmap, 0);
        }
        return true;

    }

    public Bitmap openBitmap(String path, boolean iScalePOT)
    {
        Bitmap bitmap = null;
        try
        {
            bitmap = BitmapFactory.decodeStream(context.getResources().getAssets()
                    .open(path));
            if (iScalePOT)
            {
                int originalWidth = getBitmapWidth(bitmap);
                int originalHeight = getBitmapHeight(bitmap);
                int width = nextPOT(originalWidth);
                int height = nextPOT(originalHeight);
                if (originalWidth != width || originalHeight != height)
                {
                    // Scale it
                    bitmap = scaleBitmap(bitmap, width, height);
                }
            }

        } catch (Exception e)
        {
            Log.w("NDKHelper", "Coundn't load a file:" + path);
        }

        return bitmap;
    }

    public int getBitmapWidth(Bitmap bmp)
    {
        return bmp.getWidth();
    }

    public int getBitmapHeight(Bitmap bmp)
    {
        return bmp.getHeight();
    }

    public void getBitmapPixels(Bitmap bmp, int[] pixels)
    {
        int w = bmp.getWidth();
        int h = bmp.getHeight();
        bmp.getPixels(pixels, 0, w, 0, 0, w, h);
    }

    public void closeBitmap(Bitmap bmp)
    {
        bmp.recycle();
    }

    public static String getNativeLibraryDirectory(Context appContext)
    {
        ApplicationInfo ai = context.getApplicationInfo();

        Log.w("NDKHelper", "ai.nativeLibraryDir:" + ai.nativeLibraryDir);

        if ((ai.flags & ApplicationInfo.FLAG_UPDATED_SYSTEM_APP) != 0
                || (ai.flags & ApplicationInfo.FLAG_SYSTEM) == 0)
        {
            return ai.nativeLibraryDir;
        }
        return "/system/lib/";
    }

    public int getNativeAudioBufferSize()
    {
        int SDK_INT = android.os.Build.VERSION.SDK_INT;
        if (SDK_INT >= 17)
        {
            AudioManager am = (AudioManager) context
                    .getSystemService(Context.AUDIO_SERVICE);
            String framesPerBuffer = am
                    .getProperty(AudioManager.PROPERTY_OUTPUT_FRAMES_PER_BUFFER);
            return Integer.parseInt(framesPerBuffer);
        } else
        {
            return 0;
        }
    }

    public int getNativeAudioSampleRate()
    {
        return AudioTrack.getNativeOutputSampleRate(AudioManager.STREAM_SYSTEM);

    }

}
