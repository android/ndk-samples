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
#include <utility>
#include <queue>
#include <stdint.h>
#include <unistd.h>
#include <cinttypes>

#include <camera/NdkCameraManager.h>
#include <camera/NdkCameraMetadata.h>
#include <android_native_app_glue.h>
#include "camera_manager.h"
#include "utils/native_debug.h"
#include "camera_utils.h"

/*
 * CameraManager callbacks: this sample does not handle hotplug,
 * does nothing in callback
 */
void OnCameraAvailable(void* ctx, const char* id) {
    reinterpret_cast<NativeCamera*>(ctx)->OnCameraStatusChanged(id, true);
}
void OnCameraUnavailable(void* ctx, const char* id){
    reinterpret_cast<NativeCamera*>(ctx)->OnCameraStatusChanged(id, false);
}

/*
 * CameraDevice callbacks
 */
void OnDeviceStateChanges(void* ctx, ACameraDevice* dev) {
    reinterpret_cast<NativeCamera*>(ctx)->OnDeviceStateChanges(dev);
}
void OnDeviceErrorChanges(void* ctx, ACameraDevice* dev, int err) {
    reinterpret_cast<NativeCamera*>(ctx)->OnDeviceErrorChanges(dev, err);
}

// CaptureSession creation callbacks, pass through into mgr object
void OnSessionClosed(void* ctx, ACameraCaptureSession* ses) {
    reinterpret_cast<NativeCamera*>(ctx)->OnSessionStateChange(
            ses, CaptureSessionState::CLOSED);
}
void OnSessionReady(void* ctx, ACameraCaptureSession* ses) {
    reinterpret_cast<NativeCamera*>(ctx)->OnSessionStateChange(
            ses, CaptureSessionState::READY);
}
void OnSessionActive(void* ctx, ACameraCaptureSession *ses) {
    reinterpret_cast<NativeCamera*>(ctx)->OnSessionStateChange(
            ses, CaptureSessionState::ACTIVE);
}

NativeCamera::NativeCamera(ANativeWindow* window) :
                  outputWindow_(window),
                  cameraMgr_(nullptr),
                  activeCameraId_(""),
                  outputContainer_(nullptr),
                  captureSessionState_(CaptureSessionState::MAX_STATE),
                  cameraFacing_ (ACAMERA_LENS_FACING_BACK),
                  cameraOrientation_ (0) {
    cameras_.clear();
    cameraMgr_ = ACameraManager_create();
    ASSERT(cameraMgr_, "Failed to create cameraManager");

    // Pick up a back-facing camera to preview
    EnumerateCamera();
    ASSERT(activeCameraId_.size(), "Unknown ActiveCameraIdx");

    mgrListener_ = {
            .context = this,
            .onCameraAvailable = ::OnCameraAvailable,
            .onCameraUnavailable = ::OnCameraUnavailable,
    };
    CALL_MGR(registerAvailabilityCallback(cameraMgr_,
                                          &mgrListener_));

    // PrintCameras(cameraMgr_);

    // Create camera device for the selected Camera device
    cameraListener_ = {
            .context = this,
            .onDisconnected = ::OnDeviceStateChanges,
            .onError = ::OnDeviceErrorChanges,
    };
    CALL_MGR(openCamera(cameraMgr_, activeCameraId_.c_str(),
                        &cameraListener_, &cameras_[activeCameraId_].device_));

}

/*
 * 1) find the exactly the one to use, which is relating to the orientation of camera too
 * 2) find the one that matches ( half-size or the same aspect ratio )
 * 3) 640-480 as last resort
 * ACAMERA_SCALER_AVAILABLE_STREAM_CONFIGURATIONS
 */
class DisplaySize {
 public :
    DisplaySize(int32_t w, int32_t h) :w_(w), h_(h), portrait_(false) {
        if (h > w) {
            // make it landscape
            w_ = h;
            h_ = w;
            portrait_ = true;
        }
    }
    DisplaySize(const DisplaySize& other) {
        w_ = other.w_;
        h_ = other.h_;
        portrait_ = other.portrait_;
    }

    DisplaySize(void){
        w_ = 0;
        h_ = 0;
        portrait_ = false;
    }
    DisplaySize& operator= (const DisplaySize& other) {
        w_ = other.w_;
        h_ = other.h_;
        portrait_ = other.portrait_;

        return (*this);
    }

    bool IsSameRatio(DisplaySize& other) {
        return (w_ * other.h_ == h_ * other.w_);
    }
    bool operator >(DisplaySize& other) {
        return (w_ >= other.w_ & h_ >= other.h_);
    }
    bool operator ==(DisplaySize& other) {
        return (w_ == other.w_ && h_ == other.h_ &&
                portrait_ == other.portrait_);
    }
    DisplaySize operator -(DisplaySize &other) {
        DisplaySize delta(w_ - other.w_, h_ - other.h_);
        return delta;
    }
    void Flip(void) {
        portrait_ = !portrait_;
    }
    bool IsPortrait(void) { return  portrait_; }
    int32_t width(void) {return w_;}
    int32_t height(void) { return h_;}
    int32_t org_width(void) { return (portrait_ ? h_ : w_); }
    int32_t org_height(void) { return (portrait_ ? w_ : h_); }

 private:
    int32_t w_, h_;
    bool portrait_;
};

bool NativeCamera::FindSupportedCaptureResolution(ANativeWindow *display,
                                    ImageResolution* resRec) {

    int32_t dispWidth = ANativeWindow_getWidth(display);
    int32_t dispHeight = ANativeWindow_getHeight(display);

    DisplaySize disp(dispWidth, dispHeight);
    if (cameraOrientation_ == 90 || cameraOrientation_ == 270) {
        disp.Flip();
    }

    ACameraMetadata* metadata;
    CALL_MGR(getCameraCharacteristics(cameraMgr_, activeCameraId_.c_str(),
                                      &metadata));
    ACameraMetadata_const_entry entry;
    CALL_METADATA(getConstEntry(metadata,
                                ACAMERA_SCALER_AVAILABLE_STREAM_CONFIGURATIONS,
                                &entry));
    // format of the data: format, width, height, input?, type int32
    std::queue<DisplaySize>  modes;
    std::queue<bool>sameAspect;
    bool hasSameAspectRatio = false;
    bool hasBiggerRes = false;
    DisplaySize bigRes(0, 0), bigResDelta(disp.width(),disp.height());
    DisplaySize sameRatioRes(0, 0), ratioDelta(disp.width(),disp.height());

    for (int i = 0; i < entry.count; ++i) {
        int32_t input = entry.data.i32[i*4 + 3];
        int32_t format = entry.data.i32[i*4 + 0];
        if (input || format != AIMAGE_FORMAT_YUV_420_888)
            continue;

        DisplaySize res(entry.data.i32[i * 4 + 1], entry.data.i32[i * 4 + 2]);
        if (res == disp) {
            resRec->width = res.org_width();
            resRec->height = res.org_height();
            resRec->format = AIMAGE_FORMAT_YUV_420_888;

            // found it, no need to continue
            return true;
        }
        if (disp.IsSameRatio(res)) {
            sameAspect.push(true);
            hasSameAspectRatio = true;
            DisplaySize delta;
            if (disp > res ) {
                delta = disp - res;
            } else if (res > disp ) {
                delta = res - disp;
            }
            if (ratioDelta > delta) {
                ratioDelta = delta;
                sameRatioRes = res;
            }
        } else  {
            sameAspect.push(false);
        }

        if (res > disp) {
            hasBiggerRes = true;
            DisplaySize delta(0, 0);
            delta = res - disp;
            if (bigResDelta > delta) {
                bigResDelta = delta;
                bigRes = res;
            }
        }
        modes.push(res);
    }

    ASSERT(modes.size(), "NO output stream format found");

    if (hasSameAspectRatio && hasBiggerRes) {
        if (sameRatioRes > disp) {
            resRec->width = sameRatioRes.org_width();
            resRec->height = sameRatioRes.org_height();
        } else {
            resRec->width = bigRes.org_width();
            resRec->height = bigRes.org_height();
        }
    } else if (hasSameAspectRatio) {
        resRec->width = sameRatioRes.org_width();
        resRec->height = sameRatioRes.org_height();
    } else if (hasBiggerRes) {
        resRec->width = bigRes.org_width();
        resRec->height = bigRes.org_height();
    } else {
        resRec->width = 640;
        resRec->height = 480;
    }
    resRec->format = AIMAGE_FORMAT_YUV_420_888;
    return (!hasSameAspectRatio && !hasBiggerRes);
}

void NativeCamera::CreateSession(ANativeWindow* outputWindow) {
    // Create output from this app's ANativeWindow, and add into output container
    outputWindow_ = outputWindow;
    ANativeWindow_acquire(outputWindow_);
    CALL_CONTAINER(create(&outputContainer_));
    CALL_OUTPUT(create(outputWindow_, &output_));
    CALL_CONTAINER(create(&outputContainer_));
    CALL_CONTAINER(add(outputContainer_, output_));

    // Create output target from the same ANativeWindow, add it into captureRequest
    CALL_TARGET(create(outputWindow_, &outputTarget_));
    CALL_DEV(createCaptureRequest(cameras_[activeCameraId_].device_,
                                  TEMPLATE_PREVIEW, &captureRequest_));
    CALL_REQUEST(addTarget(captureRequest_, outputTarget_));

    {
        LOGI("====Avaliable Tags for this capture: ");
        PrintRequestMetadata(captureRequest_);
        LOGI("====Available Tags End");
    }
    // Create a capture session for the given preview request
    sessionListener_ = {
            .context = this,
            .onActive = ::OnSessionActive,
            .onReady = ::OnSessionReady,
            .onClosed = ::OnSessionClosed,
    };
    captureSessionState_ = CaptureSessionState::READY;
    CALL_DEV(createCaptureSession(cameras_[activeCameraId_].device_,
                                  outputContainer_, &sessionListener_,
                                  &captureSession_));
}

NativeCamera::~NativeCamera() {

    ACameraCaptureSession_close(captureSession_);
    CALL_REQUEST(removeTarget(captureRequest_, outputTarget_));
    ACaptureRequest_free(captureRequest_);
    ACameraOutputTarget_free(outputTarget_);

    CALL_CONTAINER(remove(outputContainer_, output_));
    ACaptureSessionOutputContainer_free(outputContainer_);
    ACaptureSessionOutput_free(output_);

    ANativeWindow_release(outputWindow_);

    for (auto &cam: cameras_) {
        if (cam.second.device_) {
            CALL_DEV(close(cam.second.device_));
        }
    }
    cameras_.clear();
    if (cameraMgr_) {
        CALL_MGR(unregisterAvailabilityCallback(cameraMgr_,
                                                &mgrListener_));
        ACameraManager_delete(cameraMgr_);
        cameraMgr_ = nullptr;
    }
}

/*
 * EnumerateCamera()
 *     Loop through cameras on the system, pick up
 *     1) back facing one if available
 *     2) otherwise pick the first one reported to us
 */
void NativeCamera::EnumerateCamera() {
    ACameraIdList*  cameraIds = nullptr;
    CALL_MGR(getCameraIdList(cameraMgr_, &cameraIds));

    for (int i = 0; i < cameraIds->numCameras; ++i) {
        const char* id = cameraIds->cameraIds[i];

        ACameraMetadata *metadataObj;
        CALL_MGR(getCameraCharacteristics(cameraMgr_, id, &metadataObj));

        int32_t count = 0;
        const uint32_t* tags = nullptr;
        ACameraMetadata_getAllTags(metadataObj, &count, &tags);
        for (int tagIdx=0; tagIdx < count; ++tagIdx) {
            if (ACAMERA_LENS_FACING == tags[tagIdx]) {
                ACameraMetadata_const_entry lensInfo = {0,};
                CALL_METADATA(getConstEntry(metadataObj,tags[tagIdx], &lensInfo));
                CameraId cam(id);
                cam.facing_ = static_cast<acamera_metadata_enum_android_lens_facing_t>(lensInfo.data.u8[0]);
                cam.owner_ = false;
                cam.device_ = nullptr;
                cameras_[cam.id_] = cam;
                if (cam.facing_ == ACAMERA_LENS_FACING_BACK) {
                    activeCameraId_ = cam.id_;
                }
                break;
            }
        }
        ACameraMetadata_free(metadataObj);
    }

    ASSERT(cameras_.size(), "No Camera Available on the device");
    if (activeCameraId_.length() == 0) {
        // if no back facing camera found, pick up the first one to use...
        activeCameraId_ = cameras_.begin()->second.id_;
    }
    ACameraManager_deleteCameraIdList(cameraIds);
}

/*
 * GetSensorOrientation()
 *     Retrieve current sensor orientation regarding to the phone device orientation
 *     SensorOrientation is NOT settable.
 */
bool NativeCamera::GetSensorOrientation(int32_t* facing, int32_t* angle) {

    if (!cameraMgr_) {
        return false;
    }

    ACameraMetadata *metadataObj;
    ACameraMetadata_const_entry face, orientation;
    CALL_MGR(getCameraCharacteristics(cameraMgr_, activeCameraId_.c_str(),
                                      &metadataObj));
    CALL_METADATA(getConstEntry(metadataObj, ACAMERA_LENS_FACING, &face));
    cameraFacing_ = static_cast<int32_t>(face.data.u8[0]);

    CALL_METADATA(getConstEntry(metadataObj, ACAMERA_SENSOR_ORIENTATION,
                                &orientation));

    LOGI("====Current SENSOR_ORIENTATION: %8d", orientation.data.i32[0]);

    ACameraMetadata_free(metadataObj);
    cameraOrientation_ = orientation.data.i32[0];

    if (facing )
        *facing = cameraFacing_;
    if (angle)
        *angle = cameraOrientation_;
    return true;
}
/*
 * OnCameraStatusChanged()
 *  handles Callback from ACameraManager
 */
void NativeCamera::OnCameraStatusChanged(const char* id, bool available) {

    cameras_[std::string(id)].available_ = available ? true : false;
}

/*
 * OnDeviceStateChanges()
 *   callback from CameraDevice called when device is disconnected
 */
void NativeCamera::OnDeviceStateChanges(ACameraDevice* dev) {
    std::string id(ACameraDevice_getId(dev));
    LOGI("device %s is disconnected", id.c_str());

    cameras_[id].available_ = false;
    ACameraDevice_close(cameras_[id].device_);
    cameras_.erase(id);
}

void NativeCamera::OnDeviceErrorChanges(ACameraDevice* dev, int err) {
    std::string id(ACameraDevice_getId(dev));
    CameraId *cameraInfo = nullptr;

    LOGI("CameraDevice %s is in error %#x", id.c_str(), err);
    PrintCameraDeviceError(err);

    CameraId &cam = cameras_[id];

    switch (err) {
        case ERROR_CAMERA_IN_USE:
            cam.available_ = false;
            cam.owner_ = false;
            break;
        case ERROR_CAMERA_SERVICE:
        case ERROR_CAMERA_DEVICE:
        case ERROR_CAMERA_DISABLED:
        case ERROR_MAX_CAMERAS_IN_USE:
            cam.available_ = false;
            cam.owner_ = false;
            break;
        default:
            LOGI("Unknown Camera Device Error: %#x", err);
    }
}
/*
 * OnSessionStateChange():
 *   Handles Capture Session callbacks
 */
void NativeCamera::OnSessionStateChange(ACameraCaptureSession* ses,
                                        CaptureSessionState state) {
    ASSERT(ses && ses == captureSession_, "CaptureSession from camera is null");
    if (state == captureSessionState_ &&
        captureSessionState_ != CaptureSessionState::MAX_STATE) {
        LOGW("Duplicate State (%d) for session (%p), ignored", state, ses);
        return;
    }

    captureSessionState_ = state;
    if (state == CaptureSessionState::READY) {
        LOGI("Capture Session is complete: Moving it to display queue");
        //TODO: move it into display, and sechedule another capture
        return;
    }
    if (state == CaptureSessionState::ACTIVE) {
        LOGI("Capture Session just started");
        return;
    }
    if (state == CaptureSessionState::CLOSED) {
        LOGI("Session is closed");
        // we have cleaned up everything externaly,
        // Nothing else to do
    }
}
/*
 * Animate()
 *   Toggle preview start/stop
 */
void NativeCamera::Animate(void) {
    if (captureSessionState_ == CaptureSessionState::READY) {
      CALL_SESSION(setRepeatingRequest(captureSession_,
                                       nullptr,
                                       1,
                                       &captureRequest_,
                                       nullptr));
    } else if (captureSessionState_ == CaptureSessionState::ACTIVE) {
        ACameraCaptureSession_stopRepeating(captureSession_);
    }
}
