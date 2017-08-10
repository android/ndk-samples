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
#ifndef __CAMERA_CAMERA_UTILS_H__
#define __CAMERA_CAMERA_UTILS_H__

#include <camera/NdkCameraManager.h>
#include <camera/NdkCameraError.h>

/*
 * A set of macros to call into Camera APIs. The API is grouped with a few
 * objects, with object name as the prefix of function names.
 */
#define CALL_CAMERA(func)                                             \
  {                                                                   \
    camera_status_t status = func;                                    \
    ASSERT(status == ACAMERA_OK, "%s call failed with code: %#x, %s", \
           __FUNCTION__, status, GetErrorStr(status));                \
  }
#define CALL_MGR(func) CALL_CAMERA(ACameraManager_##func)
#define CALL_DEV(func) CALL_CAMERA(ACameraDevice_##func)
#define CALL_METADATA(func) CALL_CAMERA(ACameraMetadata_##func)
#define CALL_CONTAINER(func) CALL_CAMERA(ACaptureSessionOutputContainer_##func)
#define CALL_OUTPUT(func) CALL_CAMERA(ACaptureSessionOutput_##func)
#define CALL_TARGET(func) CALL_CAMERA(ACameraOutputTarget_##func)
#define CALL_REQUEST(func) CALL_CAMERA(ACaptureRequest_##func)
#define CALL_SESSION(func) CALL_CAMERA(ACameraCaptureSession_##func)

/*
 * A few debugging functions for error code strings etc
 */
const char* GetErrorStr(camera_status_t err);
const char* GetTagStr(acamera_metadata_tag_t tag);
void PrintMetadataTags(int32_t entries, const uint32_t* pTags);
void PrintLensFacing(ACameraMetadata_const_entry& lensData);
void PrintCameras(ACameraManager* cameraMgr);
void PrintCameraDeviceError(int err);

void PrintRequestMetadata(ACaptureRequest* req);
#endif  // __CAMERA_CAMERA_UTILS_H__