/**
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

/** Description
 *   Demonstrate NDK Camera interface added to android-24
 */

#include <cstdio>
#include "camera_engine.h"
#include "utils/native_debug.h"


/**
 * Create a camera object for onboard BACK_FACING camera
 */
void CameraEngine::CreateCamera(void) {
  camera_ = new NDKCamera();
  ASSERT(camera_, "Failed to Create CameraObject");

  int32_t facing = 0, angle = 0, imageRotation = 0;
  if (camera_->GetSensorOrientation(&facing, &angle)) {
    if (facing == ACAMERA_LENS_FACING_FRONT) {
      imageRotation = (angle + rotation_) % 360;
      imageRotation = (360 - imageRotation) % 360;
    } else {
      imageRotation = (angle - rotation_ + 360) % 360;
    }
  }
  LOGI("Phone Rotation: %d, Present Rotation Angle: %d", rotation_,
       imageRotation);
  camera_->MatchCaptureSizeRequest(requestWidth_, requestHeight_, &compatibleCameraRes_);

  ASSERT(compatibleCameraRes_.width && compatibleCameraRes_.height, "Could not find supportable resolution");
}

void CameraEngine::DeleteCamera(void) {
  if (camera_) {
    delete camera_;
    camera_ = nullptr;
  }
}


int CameraEngine::GetCameraSensorOrientation(int32_t requestFacing) {
  ASSERT(requestFacing == ACAMERA_LENS_FACING_BACK, "Only support rear facing camera");
  int32_t facing = 0, angle = 0;
  if (camera_->GetSensorOrientation(&facing, &angle) ||
      facing == requestFacing) {
    return angle;
  }
  ASSERT(false, "Failed for GetSensorOrientation()");
  return 0;
}

/**
 *
 * @param start is true to start preview, false to stop preview
 * @return  true if preview started, false when error happened
 */
void CameraEngine::StartPreview(bool start) {
  camera_->StartPreview(start);
}
