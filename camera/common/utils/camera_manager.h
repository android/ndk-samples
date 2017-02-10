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

enum class CaptureSessionState:int32_t {
    READY = 0,   // session is ready
    ACTIVE,      // session is busy
    CLOSED,      // session is closed(by itself or a new session evicts)
    MAX_STATE
};
struct rRect {
    int32_t w;
    int32_t h;
};

class CameraId;
class NativeCamera {
  public:
    /*
     * Ctor directly creates Camera and Preview Session
     */
    explicit NativeCamera(ANativeWindow* app);

    /*
     * Ctor Creates Camera Manager; need to use CreateSession()
     * to complete camera initialization
     */
    explicit NativeCamera(void);

    /*
     * Create a capture request with give ANativeWindow: preview images
     * go into this ANativeWindow; then create preview session
     */
    void CreateSession(ANativeWindow *outputWindow);

    /*
     * Retrieve selected camera facing direction and sensor orientation angle
     * this is needed for application in later stages to rotate captured images
     */
    bool GetSensorOrientation(int32_t* facing, int32_t* angle);

    /*
     * Given a display size, find a camera capture size that is:
     *     1) same aspect ration as give display size
     *     2) minimal possible image size
     * If not found a capturing iamge size meet the above requirement,
     * 640 x 480 (or 480x640, depends on rotation angle) will be used
     */
    rRect GetCompatibleSize( rRect size);

    /*
     * Animate()
     *    Toggle preview start/stop State.
     */
    void Animate(void);

    /*
     *  Callback functions used by created NativeCamera objects
     */
    void OnCameraStatusChanged(const char* id, bool available);
    void OnDeviceStateChanges(ACameraDevice* dev);
    void OnDeviceErrorChanges(ACameraDevice* dev, int err);
    void OnSessionStateChange(ACameraCaptureSession* ses,
                              CaptureSessionState state);

    /*
     * Dtor:
     *     Cleanup created camera objects
     */
    ~NativeCamera();

private:
    /*
     * Find a camera to use. Sample uses back facing camera
     */
    bool FindCamera(acamera_metadata_enum_android_lens_facing_t
                    faceOrientation = ACAMERA_LENS_FACING_BACK);

    /*
     * Camera output needs a native window object to for preview
     * Application supplies to camera objects
     */
    ANativeWindow* outputWindow_;

    /*
     * Internal properties
     */
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
};

/*
 * CameraId:
 *     A helper class to organize camera
 */
class CameraId {
public:
    ACameraDevice *device_;
    std::string id_;
    acamera_metadata_enum_android_lens_facing_t facing_;
    bool available_;
    bool owner_;
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
