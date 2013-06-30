package com.sample.teapot;

import javax.microedition.khronos.opengles.GL10;

import android.app.Application;
import android.content.Context;
import android.content.pm.ApplicationInfo;
import android.content.pm.PackageManager;
import android.content.pm.PackageManager.NameNotFoundException;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.graphics.Matrix;
import android.opengl.GLUtils;
import android.util.Log;
import android.widget.Toast;

public class TeapotApplication extends Application {
    private static Context context;
    public void onCreate(){
        context=getApplicationContext();
        Log.w("native-activity", "onCreate");

        final PackageManager pm = getApplicationContext().getPackageManager();
        ApplicationInfo ai;
        try {
            ai = pm.getApplicationInfo( this.getPackageName(), 0);
        } catch (final NameNotFoundException e) {
            ai = null;
        }
        final String applicationName = (String) (ai != null ? pm.getApplicationLabel(ai) : "(unknown)");
        Toast.makeText(this, applicationName, Toast.LENGTH_SHORT).show();
    }

    //Load Bitmap
    private int nextPOT(int i)
    {
        int pot = 1;
        while( pot < i ) pot <<= 1;
        return pot;
    }

    private Bitmap scaleBitmap(Bitmap bitmapToScale, float newWidth, float newHeight)
    {
        if(bitmapToScale == null)
            return null;
        //get the original width and height
        int width = bitmapToScale.getWidth();
        int height = bitmapToScale.getHeight();
        // create a matrix for the manipulation
        Matrix matrix = new Matrix();

        // resize the bit map
        matrix.postScale(newWidth / width, newHeight / height);

        // recreate the new Bitmap and set it back
        return Bitmap.createBitmap(bitmapToScale, 0, 0, bitmapToScale.getWidth(), bitmapToScale.getHeight(), matrix, true);
    }

    public void loadTexture(String path)
    {
        Bitmap bitmap = null;
        try
        {
            bitmap = BitmapFactory.decodeStream(context.getResources().getAssets().open(path));
        }
        catch (Exception e) {
            Log.w("native-activity", "Coundn't load a file:" + path);
        }

        GLUtils.texImage2D(GL10.GL_TEXTURE_2D, 0, bitmap, 0);
        return;

    }

    public Bitmap openBitmap(String path, boolean iScalePOT)
    {
        Bitmap bitmap = null;
        try
        {
            bitmap = BitmapFactory.decodeStream(context.getResources().getAssets().open(path));
            if( iScalePOT )
            {
                int originalWidth = getBitmapWidth(bitmap);
                int originalHeight = getBitmapHeight(bitmap);
                int width = nextPOT(originalWidth);
                int height = nextPOT(originalHeight);
                if( originalWidth != width || originalHeight != height )
                {
                    //Scale it
                    bitmap = scaleBitmap( bitmap, width, height );
                }
            }

        }
        catch (Exception e) {
            Log.w("native-activity", "Coundn't load a file:" + path);
        }

        return bitmap;
    }

    public int getBitmapWidth(Bitmap bmp) { return bmp.getWidth(); }
    public int getBitmapHeight(Bitmap bmp) { return bmp.getHeight(); }

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
}
