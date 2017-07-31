/*
 * Copyright (C) 2017 The Android Open Source Project
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

package com.sample.textureview;

import android.Manifest;
import android.app.Activity;
import android.content.pm.PackageManager;
import android.graphics.SurfaceTexture;
import android.os.Bundle;
import android.support.annotation.NonNull;
import android.support.v4.app.ActivityCompat;
import android.util.Log;
import android.view.Display;
import android.view.Surface;
import android.view.TextureView;
import android.view.WindowManager;

import junit.framework.Assert;

public class ViewActivity extends Activity
		implements TextureView.SurfaceTextureListener,
		ActivityCompat.OnRequestPermissionsResultCallback {
	private  TextureView textureView_;
	private  int width_, height_;
	Surface  surface_ = null;
	private  int cameraWidth_;
	private  int cameraHeight_;

	@Override
	public void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
        Display display = getWindowManager().getDefaultDisplay();
		height_ = display.getMode().getPhysicalHeight();
		width_ = display.getMode().getPhysicalWidth();
		RequestCamera();
	}

	public void onSurfaceTextureAvailable(SurfaceTexture surface,
										  int width, int height) {
		surface.setDefaultBufferSize(cameraWidth_, cameraHeight_);
		surface_ = new Surface(surface);
		notifySurfaceTextureCreated(surface_);
	}

	public void onSurfaceTextureSizeChanged(SurfaceTexture surface,
											int width, int height) {
		// do nothing
	}

	public boolean onSurfaceTextureDestroyed(SurfaceTexture surface) {
		notifySurfaceTextureDestroyed(surface_);
		surface_ = null;
		return true;
	}

	public void onSurfaceTextureUpdated(SurfaceTexture surface) {
		// do nothing: display could consume YUV format
	}

	private static final int PERMISSION_REQUEST_CODE_CAMERA = 1;
	public void RequestCamera() {
		if (ActivityCompat.checkSelfPermission(this, Manifest.permission.CAMERA) !=
				PackageManager.PERMISSION_GRANTED) {
			ActivityCompat.requestPermissions(
					this,
					new String[] { Manifest.permission.CAMERA },
					PERMISSION_REQUEST_CODE_CAMERA);
			return;
		}
		CreatePreviewEngine();
	}

	@Override
	public void onRequestPermissionsResult(int requestCode,
										   @NonNull String[] permissions,
										   @NonNull int[] grantResults) {
        /*
         * if any permission failed, the sample could not play
         */
		if (PERMISSION_REQUEST_CODE_CAMERA != requestCode) {
			super.onRequestPermissionsResult(requestCode,
					permissions,
					grantResults);
			return;
		}

		Assert.assertEquals(grantResults.length, 1);
		if (grantResults[0] == PackageManager.PERMISSION_GRANTED) {
			CreatePreviewEngine();
		}
	}

	private void CreatePreviewEngine() {
		int rotation = 90 * ((WindowManager)(getSystemService(WINDOW_SERVICE)))
					.getDefaultDisplay()
					.getRotation();

		CreateCamera(width_, height_, rotation);
		cameraWidth_ = GetCameraCompatibleWidth();
		cameraHeight_ = GetCameraCompatibleHeight();

		// Now create TextureView
		textureView_ = new TextureView(this);
		textureView_.setSurfaceTextureListener(this);
		setContentView(textureView_);

	}

	private native void notifySurfaceTextureCreated(Surface surface);
	private native void notifySurfaceTextureDestroyed(Surface surface);
	/*
	 * Create a camera mgr, select backfacing camera, and find the best resolution
	 * for display mode. The returned type is Native side object
	 */
	private native long CreateCamera(int width, int height, int rotation);
	private native int  GetCameraCompatibleWidth();
	private native int  GetCameraCompatibleHeight();

	static {
		System.loadLibrary("camera_view");
	}
}