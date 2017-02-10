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

#ifndef CAMERA_NATIVE_CAMERA_H
#define CAMERA_NATIVE_CAMERA_H
#include <string>
#include <map>
#include <camera/NdkCameraManager.h>
#include <camera/NdkCameraError.h>
#include <camera/NdkCameraDevice.h>
#include <camera/NdkCameraMetadataTags.h>
#include "image_reader.h"

enum class CaptureSessionState:int32_t {
    READY = 0,   // session is ready
    ACTIVE,      // session is busy
    CLOSED,      // session is closed(by itself or a new session evicts)
    MAX_STATE
};

class CameraId;
class NativeCamera {
  private:
    ACameraManager *cameraMgr_;
    std::map<std::string, CameraId> cameras_;
    std::string activeCameraId_;
    uint32_t cameraFacing_;
    uint32_t cameraOrientation_;

    ACameraManager_AvailabilityCallbacks mgrListener_;
    ACameraDevice_stateCallbacks cameraListener_;
    ACaptureSessionOutputContainer* outputContainer_;
    ACaptureSessionOutput* output_;
    ACameraCaptureSession_stateCallbacks sessionListener_;
    ACameraCaptureSession* captureSession_;
    CaptureSessionState captureSessionState_;
    ACaptureRequest *captureRequest_;
    ACameraOutputTarget* outputTarget_;

    ANativeWindow* outputWindow_;

  public:
    explicit NativeCamera(ANativeWindow *outputWindow);
    ~NativeCamera();
    void EnumerateCamera(void);
    bool FindSupportedCaptureResolution(ANativeWindow *display,
                                        ImageResolution* res);
    void CreateSession(ANativeWindow *outputWindow);
    bool GetSensorOrientation(int32_t* facing, int32_t* angle);
    void OnCameraStatusChanged(const char* id, bool available);
    void OnDeviceStateChanges(ACameraDevice* dev);
    void OnDeviceErrorChanges(ACameraDevice* dev, int err);
    void OnSessionStateChange(ACameraCaptureSession* ses,
                              CaptureSessionState state);
    void Animate(void);
};

// helper class
class CameraId {
public:
    ACameraDevice *device_;
    std::string id_;
    acamera_metadata_enum_android_lens_facing_t facing_;
    bool available_;        // free to use ( no other apps are using
    bool owner_;            // we are the owner of the camera
    explicit CameraId(const char* id) :
            device_(nullptr),
            facing_(ACAMERA_LENS_FACING_FRONT),
            available_(false),
            owner_(false) { id_ = id; }

    explicit CameraId(void) {
        CameraId("");
    }
};

#endif // CAMERA_NATIVE_CAMERA_H
