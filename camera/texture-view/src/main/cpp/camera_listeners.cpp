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
#include <camera/NdkCameraManager.h>

#include <cinttypes>
#include <queue>
#include <thread>
#include <utility>

#include "camera_manager.h"
#include "utils/camera_utils.h"
#include "utils/native_debug.h"

/*
 * Camera Manager Listener object
 */
void OnCameraAvailable(void* ctx, const char* id) {
  reinterpret_cast<NDKCamera*>(ctx)->OnCameraStatusChanged(id, true);
}
void OnCameraUnavailable(void* ctx, const char* id) {
  reinterpret_cast<NDKCamera*>(ctx)->OnCameraStatusChanged(id, false);
}

/**
 * OnCameraStatusChanged()
 *  handles Callback from ACameraManager
 */
void NDKCamera::OnCameraStatusChanged(const char* id, bool available) {
  if (valid_) {
    cameras_[std::string(id)].available_ = available ? true : false;
  }
}

/**
 * Construct a camera manager listener on the fly and return to caller
 *
 * @return ACameraManager_AvailabilityCallback
 */
ACameraManager_AvailabilityCallbacks* NDKCamera::GetManagerListener() {
  static ACameraManager_AvailabilityCallbacks cameraMgrListener = {
      .context = this,
      .onCameraAvailable = ::OnCameraAvailable,
      .onCameraUnavailable = ::OnCameraUnavailable,
  };
  return &cameraMgrListener;
}

/*
 * CameraDevice callbacks
 */
void OnDeviceStateChanges(void* ctx, ACameraDevice* dev) {
  reinterpret_cast<NDKCamera*>(ctx)->OnDeviceState(dev);
}

void OnDeviceErrorChanges(void* ctx, ACameraDevice* dev, int err) {
  reinterpret_cast<NDKCamera*>(ctx)->OnDeviceError(dev, err);
}
ACameraDevice_stateCallbacks* NDKCamera::GetDeviceListener() {
  static ACameraDevice_stateCallbacks cameraDeviceListener = {
      .context = this,
      .onDisconnected = ::OnDeviceStateChanges,
      .onError = ::OnDeviceErrorChanges,
  };
  return &cameraDeviceListener;
}
/**
 * Handle Camera DeviceStateChanges msg, notify device is disconnected
 * simply close the camera
 */
void NDKCamera::OnDeviceState(ACameraDevice* dev) {
  std::string id(ACameraDevice_getId(dev));
  LOGW("device %s is disconnected", id.c_str());

  cameras_[id].available_ = false;
  ACameraDevice_close(cameras_[id].device_);
  cameras_.erase(id);
}
/**
 * Handles Camera's deviceErrorChanges message, no action;
 * mainly debugging purpose
 *
 *
 */
void NDKCamera::OnDeviceError(ACameraDevice* dev, int err) {
  std::string id(ACameraDevice_getId(dev));

  LOGI("CameraDevice %s is in error %#x", id.c_str(), err);
  PrintCameraDeviceError(err);

  CameraId& cam = cameras_[id];

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

// CaptureSession state callbacks
void OnSessionClosed(void* ctx, ACameraCaptureSession* ses) {
  LOGW("session %p closed", ses);
  reinterpret_cast<NDKCamera*>(ctx)->OnSessionState(
      ses, CaptureSessionState::CLOSED);
}
void OnSessionReady(void* ctx, ACameraCaptureSession* ses) {
  LOGW("session %p ready", ses);
  reinterpret_cast<NDKCamera*>(ctx)->OnSessionState(ses,
                                                    CaptureSessionState::READY);
}
void OnSessionActive(void* ctx, ACameraCaptureSession* ses) {
  LOGW("session %p active", ses);
  reinterpret_cast<NDKCamera*>(ctx)->OnSessionState(
      ses, CaptureSessionState::ACTIVE);
}

ACameraCaptureSession_stateCallbacks* NDKCamera::GetSessionListener() {
  static ACameraCaptureSession_stateCallbacks sessionListener = {
      .context = this,
      .onClosed = ::OnSessionClosed,
      .onReady = ::OnSessionReady,
      .onActive = ::OnSessionActive,
  };
  return &sessionListener;
}

/**
 * Handles capture session state changes.
 *   Update into internal session state.
 */
void NDKCamera::OnSessionState(ACameraCaptureSession* ses,
                               CaptureSessionState state) {
  if (!ses || ses != captureSession_) {
    LOGW("CaptureSession is %s", (ses ? "NOT our session" : "NULL"));
    return;
  }

  ASSERT(state < CaptureSessionState::MAX_STATE, "Wrong state %d", state);

  captureSessionState_ = state;
}

// Capture callbacks, mostly information purpose
void SessionCaptureCallback_OnFailed(void* context,
                                     ACameraCaptureSession* session,
                                     ACaptureRequest* request,
                                     ACameraCaptureFailure* failure) {
  std::thread captureFailedThread(&NDKCamera::OnCaptureFailed,
                                  static_cast<NDKCamera*>(context), session,
                                  request, failure);
  captureFailedThread.detach();
}

void SessionCaptureCallback_OnSequenceEnd(void* context,
                                          ACameraCaptureSession* session,
                                          int sequenceId, int64_t frameNumber) {
  std::thread sequenceThread(&NDKCamera::OnCaptureSequenceEnd,
                             static_cast<NDKCamera*>(context), session,
                             sequenceId, frameNumber);
  sequenceThread.detach();
}
void SessionCaptureCallback_OnSequenceAborted(void* context,
                                              ACameraCaptureSession* session,
                                              int sequenceId) {
  std::thread sequenceThread(&NDKCamera::OnCaptureSequenceEnd,
                             static_cast<NDKCamera*>(context), session,
                             sequenceId, static_cast<int64_t>(-1));
  sequenceThread.detach();
}

ACameraCaptureSession_captureCallbacks* NDKCamera::GetCaptureCallback() {
  static ACameraCaptureSession_captureCallbacks captureListener{
      .context = this,
      .onCaptureStarted = nullptr,
      .onCaptureProgressed = nullptr,
      .onCaptureCompleted = nullptr,
      .onCaptureFailed = SessionCaptureCallback_OnFailed,
      .onCaptureSequenceCompleted = SessionCaptureCallback_OnSequenceEnd,
      .onCaptureSequenceAborted = SessionCaptureCallback_OnSequenceAborted,
      .onCaptureBufferLost = nullptr,
  };
  return &captureListener;
}

/**
 * Process JPG capture SessionCaptureCallback_OnFailed event
 * If this is current JPG capture session, simply resume preview
 * @param session the capture session that failed
 * @param request the capture request that failed
 * @param failure for additional fail info.
 */
void NDKCamera::OnCaptureFailed(ACameraCaptureSession* session,
                                ACaptureRequest* request,
                                ACameraCaptureFailure* failure) {
  if (valid_ && request == requests_[JPG_CAPTURE_REQUEST_IDX].request_) {
    ASSERT(failure->sequenceId ==
               requests_[JPG_CAPTURE_REQUEST_IDX].sessionSequenceId_,
           "Error jpg sequence id")
    StartPreview(true);
  }
}

/**
 * Process event from JPEG capture
 *    SessionCaptureCallback_OnSequenceEnd()
 *    SessionCaptureCallback_OnSequenceAborted()
 *
 * If this is jpg capture, turn back on preview after a catpure.
 */
void NDKCamera::OnCaptureSequenceEnd(ACameraCaptureSession* session,
                                     int sequenceId, int64_t frameNumber) {
  if (sequenceId != requests_[JPG_CAPTURE_REQUEST_IDX].sessionSequenceId_)
    return;

  // resume preview
  CALL_SESSION(setRepeatingRequest(captureSession_, nullptr, 1,
                                   &requests_[PREVIEW_REQUEST_IDX].request_,
                                   nullptr));
}
