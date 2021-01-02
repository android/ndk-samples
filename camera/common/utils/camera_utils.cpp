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
#include <vector>
#include <string>
#include <utility>
#include <typeinfo>
#include <cinttypes>
#include <media/NdkImage.h>
#include <camera/NdkCameraManager.h>
#include "utils/native_debug.h"
#include "camera_utils.h"

#define UKNOWN_TAG "UNKNOW_TAG"
#define MAKE_PAIR(val) std::make_pair(val, #val)
template <typename T>
const char* GetPairStr(T key, std::vector<std::pair<T, const char*>>& store) {
  typedef typename std::vector<std::pair<T, const char*>>::iterator iterator;
  for (iterator it = store.begin(); it != store.end(); ++it) {
    if (it->first == key) {
      return it->second;
    }
  }
  LOGW("(%#08x) : UNKNOWN_TAG for %s", key, typeid(store[0].first).name());
  return UKNOWN_TAG;
}

/*
 * camera_status_t error translation
 */
using ERROR_PAIR = std::pair<camera_status_t, const char*>;
static std::vector<ERROR_PAIR> errorInfo{
    MAKE_PAIR(ACAMERA_OK),
    MAKE_PAIR(ACAMERA_ERROR_UNKNOWN),
    MAKE_PAIR(ACAMERA_ERROR_INVALID_PARAMETER),
    MAKE_PAIR(ACAMERA_ERROR_CAMERA_DISCONNECTED),
    MAKE_PAIR(ACAMERA_ERROR_NOT_ENOUGH_MEMORY),
    MAKE_PAIR(ACAMERA_ERROR_METADATA_NOT_FOUND),
    MAKE_PAIR(ACAMERA_ERROR_CAMERA_DEVICE),
    MAKE_PAIR(ACAMERA_ERROR_CAMERA_SERVICE),
    MAKE_PAIR(ACAMERA_ERROR_SESSION_CLOSED),
    MAKE_PAIR(ACAMERA_ERROR_INVALID_OPERATION),
    MAKE_PAIR(ACAMERA_ERROR_STREAM_CONFIGURE_FAIL),
    MAKE_PAIR(ACAMERA_ERROR_CAMERA_IN_USE),
    MAKE_PAIR(ACAMERA_ERROR_MAX_CAMERA_IN_USE),
    MAKE_PAIR(ACAMERA_ERROR_CAMERA_DISABLED),
    MAKE_PAIR(ACAMERA_ERROR_PERMISSION_DENIED),
};
const char* GetErrorStr(camera_status_t err) {
  return GetPairStr<camera_status_t>(err, errorInfo);
}

/*
 * camera_metadata_tag_t translation. Useful to look at available tags
 * on the underneath platform
 */
using TAG_PAIR = std::pair<acamera_metadata_tag_t, const char*>;
static std::vector<TAG_PAIR> tagInfo{
    MAKE_PAIR(ACAMERA_COLOR_CORRECTION_MODE),
    MAKE_PAIR(ACAMERA_COLOR_CORRECTION_TRANSFORM),
    MAKE_PAIR(ACAMERA_COLOR_CORRECTION_GAINS),
    MAKE_PAIR(ACAMERA_COLOR_CORRECTION_ABERRATION_MODE),
    MAKE_PAIR(ACAMERA_COLOR_CORRECTION_AVAILABLE_ABERRATION_MODES),
    MAKE_PAIR(ACAMERA_COLOR_CORRECTION_END),
    MAKE_PAIR(ACAMERA_CONTROL_AE_ANTIBANDING_MODE),
    MAKE_PAIR(ACAMERA_CONTROL_AE_EXPOSURE_COMPENSATION),
    MAKE_PAIR(ACAMERA_CONTROL_AE_LOCK),
    MAKE_PAIR(ACAMERA_CONTROL_AE_MODE),
    MAKE_PAIR(ACAMERA_CONTROL_AE_REGIONS),
    MAKE_PAIR(ACAMERA_CONTROL_AE_TARGET_FPS_RANGE),
    MAKE_PAIR(ACAMERA_CONTROL_AE_PRECAPTURE_TRIGGER),
    MAKE_PAIR(ACAMERA_CONTROL_AF_MODE),
    MAKE_PAIR(ACAMERA_CONTROL_AF_REGIONS),
    MAKE_PAIR(ACAMERA_CONTROL_AF_TRIGGER),
    MAKE_PAIR(ACAMERA_CONTROL_AWB_LOCK),
    MAKE_PAIR(ACAMERA_CONTROL_AWB_MODE),
    MAKE_PAIR(ACAMERA_CONTROL_AWB_REGIONS),
    MAKE_PAIR(ACAMERA_CONTROL_CAPTURE_INTENT),
    MAKE_PAIR(ACAMERA_CONTROL_EFFECT_MODE),
    MAKE_PAIR(ACAMERA_CONTROL_MODE),
    MAKE_PAIR(ACAMERA_CONTROL_SCENE_MODE),
    MAKE_PAIR(ACAMERA_CONTROL_VIDEO_STABILIZATION_MODE),
    MAKE_PAIR(ACAMERA_CONTROL_AE_AVAILABLE_ANTIBANDING_MODES),
    MAKE_PAIR(ACAMERA_CONTROL_AE_AVAILABLE_MODES),
    MAKE_PAIR(ACAMERA_CONTROL_AE_AVAILABLE_TARGET_FPS_RANGES),
    MAKE_PAIR(ACAMERA_CONTROL_AE_COMPENSATION_RANGE),
    MAKE_PAIR(ACAMERA_CONTROL_AE_COMPENSATION_STEP),
    MAKE_PAIR(ACAMERA_CONTROL_AF_AVAILABLE_MODES),
    MAKE_PAIR(ACAMERA_CONTROL_AVAILABLE_EFFECTS),
    MAKE_PAIR(ACAMERA_CONTROL_AVAILABLE_SCENE_MODES),
    MAKE_PAIR(ACAMERA_CONTROL_AVAILABLE_VIDEO_STABILIZATION_MODES),
    MAKE_PAIR(ACAMERA_CONTROL_AWB_AVAILABLE_MODES),
    MAKE_PAIR(ACAMERA_CONTROL_MAX_REGIONS),
    MAKE_PAIR(ACAMERA_CONTROL_AE_STATE),
    MAKE_PAIR(ACAMERA_CONTROL_AF_STATE),
    MAKE_PAIR(ACAMERA_CONTROL_AWB_STATE),
    MAKE_PAIR(ACAMERA_CONTROL_AE_LOCK_AVAILABLE),
    MAKE_PAIR(ACAMERA_CONTROL_AWB_LOCK_AVAILABLE),
    MAKE_PAIR(ACAMERA_CONTROL_AVAILABLE_MODES),
    MAKE_PAIR(ACAMERA_CONTROL_POST_RAW_SENSITIVITY_BOOST_RANGE),
    MAKE_PAIR(ACAMERA_CONTROL_POST_RAW_SENSITIVITY_BOOST),
    MAKE_PAIR(ACAMERA_CONTROL_END),
    MAKE_PAIR(ACAMERA_EDGE_MODE),
    MAKE_PAIR(ACAMERA_EDGE_AVAILABLE_EDGE_MODES),
    MAKE_PAIR(ACAMERA_EDGE_END),
    MAKE_PAIR(ACAMERA_FLASH_MODE),
    MAKE_PAIR(ACAMERA_FLASH_STATE),
    MAKE_PAIR(ACAMERA_FLASH_END),
    MAKE_PAIR(ACAMERA_FLASH_INFO_AVAILABLE),
    MAKE_PAIR(ACAMERA_FLASH_INFO_END),
    MAKE_PAIR(ACAMERA_HOT_PIXEL_MODE),
    MAKE_PAIR(ACAMERA_HOT_PIXEL_AVAILABLE_HOT_PIXEL_MODES),
    MAKE_PAIR(ACAMERA_HOT_PIXEL_END),
    MAKE_PAIR(ACAMERA_JPEG_GPS_COORDINATES),
    MAKE_PAIR(ACAMERA_JPEG_GPS_PROCESSING_METHOD),
    MAKE_PAIR(ACAMERA_JPEG_GPS_TIMESTAMP),
    MAKE_PAIR(ACAMERA_JPEG_ORIENTATION),
    MAKE_PAIR(ACAMERA_JPEG_QUALITY),
    MAKE_PAIR(ACAMERA_JPEG_THUMBNAIL_QUALITY),
    MAKE_PAIR(ACAMERA_JPEG_THUMBNAIL_SIZE),
    MAKE_PAIR(ACAMERA_JPEG_AVAILABLE_THUMBNAIL_SIZES),
    MAKE_PAIR(ACAMERA_JPEG_END),
    MAKE_PAIR(ACAMERA_LENS_APERTURE),
    MAKE_PAIR(ACAMERA_LENS_FILTER_DENSITY),
    MAKE_PAIR(ACAMERA_LENS_FOCAL_LENGTH),
    MAKE_PAIR(ACAMERA_LENS_FOCUS_DISTANCE),
    MAKE_PAIR(ACAMERA_LENS_OPTICAL_STABILIZATION_MODE),
    MAKE_PAIR(ACAMERA_LENS_FACING),
    MAKE_PAIR(ACAMERA_LENS_POSE_ROTATION),
    MAKE_PAIR(ACAMERA_LENS_POSE_TRANSLATION),
    MAKE_PAIR(ACAMERA_LENS_FOCUS_RANGE),
    MAKE_PAIR(ACAMERA_LENS_STATE),
    MAKE_PAIR(ACAMERA_LENS_INTRINSIC_CALIBRATION),
    MAKE_PAIR(ACAMERA_LENS_RADIAL_DISTORTION),
    MAKE_PAIR(ACAMERA_LENS_END),

    MAKE_PAIR(ACAMERA_LENS_INFO_AVAILABLE_APERTURES),
    MAKE_PAIR(ACAMERA_LENS_INFO_AVAILABLE_FILTER_DENSITIES),
    MAKE_PAIR(ACAMERA_LENS_INFO_AVAILABLE_FOCAL_LENGTHS),
    MAKE_PAIR(ACAMERA_LENS_INFO_AVAILABLE_OPTICAL_STABILIZATION),
    MAKE_PAIR(ACAMERA_LENS_INFO_HYPERFOCAL_DISTANCE),
    MAKE_PAIR(ACAMERA_LENS_INFO_MINIMUM_FOCUS_DISTANCE),
    MAKE_PAIR(ACAMERA_LENS_INFO_SHADING_MAP_SIZE),
    MAKE_PAIR(ACAMERA_LENS_INFO_FOCUS_DISTANCE_CALIBRATION),
    MAKE_PAIR(ACAMERA_LENS_INFO_END),

    MAKE_PAIR(ACAMERA_NOISE_REDUCTION_MODE),
    MAKE_PAIR(ACAMERA_NOISE_REDUCTION_AVAILABLE_NOISE_REDUCTION_MODES),
    MAKE_PAIR(ACAMERA_NOISE_REDUCTION_END),

    MAKE_PAIR(ACAMERA_REQUEST_MAX_NUM_OUTPUT_STREAMS),
    MAKE_PAIR(ACAMERA_REQUEST_PIPELINE_DEPTH),
    MAKE_PAIR(ACAMERA_REQUEST_PIPELINE_MAX_DEPTH),
    MAKE_PAIR(ACAMERA_REQUEST_PARTIAL_RESULT_COUNT),
    MAKE_PAIR(ACAMERA_REQUEST_AVAILABLE_CAPABILITIES),
    MAKE_PAIR(ACAMERA_REQUEST_AVAILABLE_REQUEST_KEYS),
    MAKE_PAIR(ACAMERA_REQUEST_AVAILABLE_RESULT_KEYS),
    MAKE_PAIR(ACAMERA_REQUEST_AVAILABLE_CHARACTERISTICS_KEYS),
    MAKE_PAIR(ACAMERA_REQUEST_END),

    MAKE_PAIR(ACAMERA_SCALER_CROP_REGION),
    MAKE_PAIR(ACAMERA_SCALER_AVAILABLE_MAX_DIGITAL_ZOOM),
    MAKE_PAIR(ACAMERA_SCALER_AVAILABLE_STREAM_CONFIGURATIONS),
    MAKE_PAIR(ACAMERA_SCALER_AVAILABLE_MIN_FRAME_DURATIONS),
    MAKE_PAIR(ACAMERA_SCALER_AVAILABLE_STALL_DURATIONS),
    MAKE_PAIR(ACAMERA_SCALER_CROPPING_TYPE),
    MAKE_PAIR(ACAMERA_SCALER_END),

    MAKE_PAIR(ACAMERA_SENSOR_EXPOSURE_TIME),
    MAKE_PAIR(ACAMERA_SENSOR_FRAME_DURATION),
    MAKE_PAIR(ACAMERA_SENSOR_SENSITIVITY),
    MAKE_PAIR(ACAMERA_SENSOR_REFERENCE_ILLUMINANT1),
    MAKE_PAIR(ACAMERA_SENSOR_REFERENCE_ILLUMINANT2),
    MAKE_PAIR(ACAMERA_SENSOR_CALIBRATION_TRANSFORM1),
    MAKE_PAIR(ACAMERA_SENSOR_CALIBRATION_TRANSFORM2),
    MAKE_PAIR(ACAMERA_SENSOR_COLOR_TRANSFORM1),
    MAKE_PAIR(ACAMERA_SENSOR_COLOR_TRANSFORM2),
    MAKE_PAIR(ACAMERA_SENSOR_FORWARD_MATRIX1),
    MAKE_PAIR(ACAMERA_SENSOR_FORWARD_MATRIX2),
    MAKE_PAIR(ACAMERA_SENSOR_BLACK_LEVEL_PATTERN),
    MAKE_PAIR(ACAMERA_SENSOR_MAX_ANALOG_SENSITIVITY),
    MAKE_PAIR(ACAMERA_SENSOR_ORIENTATION),
    MAKE_PAIR(ACAMERA_SENSOR_TIMESTAMP),
    MAKE_PAIR(ACAMERA_SENSOR_NEUTRAL_COLOR_POINT),
    MAKE_PAIR(ACAMERA_SENSOR_NOISE_PROFILE),
    MAKE_PAIR(ACAMERA_SENSOR_GREEN_SPLIT),
    MAKE_PAIR(ACAMERA_SENSOR_TEST_PATTERN_DATA),
    MAKE_PAIR(ACAMERA_SENSOR_TEST_PATTERN_MODE),
    MAKE_PAIR(ACAMERA_SENSOR_AVAILABLE_TEST_PATTERN_MODES),
    MAKE_PAIR(ACAMERA_SENSOR_ROLLING_SHUTTER_SKEW),
    MAKE_PAIR(ACAMERA_SENSOR_OPTICAL_BLACK_REGIONS),
    MAKE_PAIR(ACAMERA_SENSOR_DYNAMIC_BLACK_LEVEL),
    MAKE_PAIR(ACAMERA_SENSOR_DYNAMIC_WHITE_LEVEL),
    MAKE_PAIR(ACAMERA_SENSOR_END),

    MAKE_PAIR(ACAMERA_SENSOR_INFO_ACTIVE_ARRAY_SIZE),
    MAKE_PAIR(ACAMERA_SENSOR_INFO_SENSITIVITY_RANGE),
    MAKE_PAIR(ACAMERA_SENSOR_INFO_COLOR_FILTER_ARRANGEMENT),
    MAKE_PAIR(ACAMERA_SENSOR_INFO_EXPOSURE_TIME_RANGE),
    MAKE_PAIR(ACAMERA_SENSOR_INFO_MAX_FRAME_DURATION),
    MAKE_PAIR(ACAMERA_SENSOR_INFO_PHYSICAL_SIZE),
    MAKE_PAIR(ACAMERA_SENSOR_INFO_PIXEL_ARRAY_SIZE),
    MAKE_PAIR(ACAMERA_SENSOR_INFO_WHITE_LEVEL),
    MAKE_PAIR(ACAMERA_SENSOR_INFO_TIMESTAMP_SOURCE),
    MAKE_PAIR(ACAMERA_SENSOR_INFO_LENS_SHADING_APPLIED),
    MAKE_PAIR(ACAMERA_SENSOR_INFO_PRE_CORRECTION_ACTIVE_ARRAY_SIZE),
    MAKE_PAIR(ACAMERA_SENSOR_INFO_END),

    MAKE_PAIR(ACAMERA_SHADING_MODE),
    MAKE_PAIR(ACAMERA_SHADING_AVAILABLE_MODES),
    MAKE_PAIR(ACAMERA_SHADING_END),

    MAKE_PAIR(ACAMERA_STATISTICS_FACE_DETECT_MODE),
    MAKE_PAIR(ACAMERA_STATISTICS_HOT_PIXEL_MAP_MODE),
    MAKE_PAIR(ACAMERA_STATISTICS_FACE_IDS),
    MAKE_PAIR(ACAMERA_STATISTICS_FACE_LANDMARKS),
    MAKE_PAIR(ACAMERA_STATISTICS_FACE_RECTANGLES),
    MAKE_PAIR(ACAMERA_STATISTICS_FACE_SCORES),
    MAKE_PAIR(ACAMERA_STATISTICS_LENS_SHADING_MAP),
    MAKE_PAIR(ACAMERA_STATISTICS_SCENE_FLICKER),
    MAKE_PAIR(ACAMERA_STATISTICS_HOT_PIXEL_MAP),
    MAKE_PAIR(ACAMERA_STATISTICS_LENS_SHADING_MAP_MODE),
    MAKE_PAIR(ACAMERA_STATISTICS_END),

    MAKE_PAIR(ACAMERA_STATISTICS_INFO_AVAILABLE_FACE_DETECT_MODES),
    MAKE_PAIR(ACAMERA_STATISTICS_INFO_MAX_FACE_COUNT),
    MAKE_PAIR(ACAMERA_STATISTICS_INFO_AVAILABLE_HOT_PIXEL_MAP_MODES),
    MAKE_PAIR(ACAMERA_STATISTICS_INFO_AVAILABLE_LENS_SHADING_MAP_MODES),
    MAKE_PAIR(ACAMERA_STATISTICS_INFO_END),

    MAKE_PAIR(ACAMERA_TONEMAP_CURVE_BLUE),
    MAKE_PAIR(ACAMERA_TONEMAP_CURVE_GREEN),
    MAKE_PAIR(ACAMERA_TONEMAP_CURVE_RED),
    MAKE_PAIR(ACAMERA_TONEMAP_MODE),
    MAKE_PAIR(ACAMERA_TONEMAP_MAX_CURVE_POINTS),
    MAKE_PAIR(ACAMERA_TONEMAP_AVAILABLE_TONE_MAP_MODES),
    MAKE_PAIR(ACAMERA_TONEMAP_GAMMA),
    MAKE_PAIR(ACAMERA_TONEMAP_PRESET_CURVE),
    MAKE_PAIR(ACAMERA_TONEMAP_END),

    MAKE_PAIR(ACAMERA_INFO_SUPPORTED_HARDWARE_LEVEL),
    MAKE_PAIR(ACAMERA_INFO_END),
    MAKE_PAIR(ACAMERA_BLACK_LEVEL_LOCK),
    MAKE_PAIR(ACAMERA_BLACK_LEVEL_END),
    MAKE_PAIR(ACAMERA_SYNC_FRAME_NUMBER),
    MAKE_PAIR(ACAMERA_SYNC_MAX_LATENCY),
    MAKE_PAIR(ACAMERA_SYNC_END),
    MAKE_PAIR(ACAMERA_DEPTH_AVAILABLE_DEPTH_STREAM_CONFIGURATIONS),

    MAKE_PAIR(ACAMERA_DEPTH_AVAILABLE_DEPTH_MIN_FRAME_DURATIONS),
    MAKE_PAIR(ACAMERA_DEPTH_AVAILABLE_DEPTH_STALL_DURATIONS),
    MAKE_PAIR(ACAMERA_DEPTH_DEPTH_IS_EXCLUSIVE),
    MAKE_PAIR(ACAMERA_DEPTH_END),
};
const char* GetTagStr(acamera_metadata_tag_t tag) {
  return GetPairStr<acamera_metadata_tag_t>(tag, tagInfo);
}

using FORMAT_PAIR = std::pair<int, const char*>;
static std::vector<FORMAT_PAIR> formatInfo{
    MAKE_PAIR(AIMAGE_FORMAT_YUV_420_888),
    MAKE_PAIR(AIMAGE_FORMAT_JPEG),
    MAKE_PAIR(AIMAGE_FORMAT_RAW16),
    MAKE_PAIR(AIMAGE_FORMAT_RAW_PRIVATE),
    MAKE_PAIR(AIMAGE_FORMAT_RAW10),
    MAKE_PAIR(AIMAGE_FORMAT_RAW12),
    MAKE_PAIR(AIMAGE_FORMAT_DEPTH16),
    MAKE_PAIR(AIMAGE_FORMAT_DEPTH_POINT_CLOUD),
    MAKE_PAIR(AIMAGE_FORMAT_PRIVATE),
};
const char* GetFormatStr(int fmt) { return GetPairStr<int>(fmt, formatInfo); }

void PrintMetadataTags(int32_t entries, const uint32_t* pTags) {
  LOGI("MetadataTag (start):");
  for (int32_t idx = 0; idx < entries; ++idx) {
    const char* name =
        GetTagStr(static_cast<acamera_metadata_tag_t>(pTags[idx]));
    LOGI("(%#08x) : %s", pTags[idx], name);
  }
  LOGI("MetadataTag (end)");
}

void PrintLensFacing(ACameraMetadata_const_entry& lens) {
  ASSERT(lens.tag == ACAMERA_LENS_FACING, "Wrong tag(%#x) of %s to %s",
         lens.tag, GetTagStr((acamera_metadata_tag_t)lens.tag), __FUNCTION__);
  LOGI("LensFacing: tag(%#x), type(%d), count(%d), val(%#x)", lens.tag,
       lens.type, lens.count, lens.data.u8[0]);
}

/*
 * Stream_Configuration is in format of:
 *    format, width, height, input?
 *    ACAMERA_TYPE_INT32 type
 */
void PrintStreamConfigurations(ACameraMetadata_const_entry& val) {
#define MODE_LABLE "ModeInfo:"
  const char* tagName = GetTagStr(static_cast<acamera_metadata_tag_t>(val.tag));
  ASSERT(!(val.count & 0x3), "STREAM_CONFIGURATION (%d) should multiple of 4",
         val.count);
  ASSERT(val.type == ACAMERA_TYPE_INT32,
         "STREAM_CONFIGURATION TYPE(%d) is not ACAMERA_TYPE_INT32(1)",
         val.type);
  LOGI("%s -- %s:", tagName, MODE_LABLE);
  for (uint32_t i = 0; i < val.count; i += 4) {
    LOGI("%s: %08d x %08d  %s", GetFormatStr(val.data.i32[i]),
         val.data.i32[i + 1], val.data.i32[i + 2],
         val.data.i32[i + 3] ? "INPUT" : "OUTPUT");
  }
#undef MODE_LABLE
}
void PrintTagVal(const char* printLabel, ACameraMetadata_const_entry& val) {
  if (val.tag == ACAMERA_SCALER_AVAILABLE_STREAM_CONFIGURATIONS) {
    PrintStreamConfigurations(val);
    return;
  }
  const char* name = GetTagStr(static_cast<acamera_metadata_tag_t>(val.tag));
  for (uint32_t i = 0; i < val.count; ++i) {
    switch (val.type) {
      case ACAMERA_TYPE_INT32:
        LOGI("%s %s: %08d", printLabel, name, val.data.i32[i]);
        break;
      case ACAMERA_TYPE_BYTE:
        LOGI("%s %s: %#02x", printLabel, name, val.data.u8[i]);
        break;
      case ACAMERA_TYPE_INT64:
        LOGI("%s %s: %" PRIu64, printLabel, name, (int64_t)val.data.i64[i]);
        break;
      case ACAMERA_TYPE_FLOAT:
        LOGI("%s %s: %f", printLabel, name, val.data.f[i]);
        break;
      case ACAMERA_TYPE_DOUBLE:
        LOGI("%s %s: %" PRIx64, printLabel, name, val.data.i64[i]);
        break;
      case ACAMERA_TYPE_RATIONAL:
        LOGI("%s %s: %08x, %08x", printLabel, name, val.data.r[i].numerator,
             val.data.r[i].denominator);
        break;
      default:
        ASSERT(false, "Unknown tag value type: %d", val.type);
    }
  }
}

/*
 * PrintCamera():
 *   Enumerate existing camera and its metadata.
 */
void PrintCameras(ACameraManager* cmrMgr) {
  if (!cmrMgr) return;

  ACameraIdList* cameraIds = nullptr;
  camera_status_t status = ACameraManager_getCameraIdList(cmrMgr, &cameraIds);
  LOGI("camera Status = %d, %s", status, GetErrorStr(status));

  for (int i = 0; i < cameraIds->numCameras; ++i) {
    const char* id = cameraIds->cameraIds[i];
    LOGI("=====cameraId = %d, cameraName = %s=====", i, id);
    ACameraMetadata* metadataObj;
    CALL_MGR(getCameraCharacteristics(cmrMgr, id, &metadataObj));

    int32_t count = 0;
    const uint32_t* tags = nullptr;
    ACameraMetadata_getAllTags(metadataObj, &count, &tags);

    for (int tagIdx = 0; tagIdx < count; ++tagIdx) {
      ACameraMetadata_const_entry val = {
          0,
      };
      camera_status_t status =
          ACameraMetadata_getConstEntry(metadataObj, tags[tagIdx], &val);
      if (status != ACAMERA_OK) {
        LOGW("Unsupported Tag: %s",
             GetTagStr(static_cast<acamera_metadata_tag_t>(tags[tagIdx])));
        continue;
      }

      PrintTagVal("Camera Tag:", val);

      if (ACAMERA_LENS_FACING == tags[tagIdx]) {
        PrintLensFacing(val);
      }
    }
    ACameraMetadata_free(metadataObj);
  }

  ACameraManager_deleteCameraIdList(cameraIds);
}

void PrintRequestMetadata(ACaptureRequest* req) {
  if (!req) return;
  int32_t count;
  const uint32_t* tags;
  CALL_REQUEST(getAllTags(req, &count, &tags));
  for (int32_t idx = 0; idx < count; ++idx) {
    ACameraMetadata_const_entry val;
    CALL_REQUEST(getConstEntry(req, tags[idx], &val));
    const char* name =
        GetTagStr(static_cast<acamera_metadata_tag_t>(tags[idx]));

    for (uint32_t i = 0; i < val.count; ++i) {
      switch (val.type) {
        case ACAMERA_TYPE_INT32:
          LOGI("Capture Tag %s: %08d", name, val.data.i32[i]);
          break;
        case ACAMERA_TYPE_BYTE:
          LOGI("Capture Tag %s: %#08x", name, val.data.u8[i]);
          break;
        case ACAMERA_TYPE_INT64:
          LOGI("Capture Tag %s: %" PRIu64, name, (int64_t)val.data.i64[i]);
          break;
        case ACAMERA_TYPE_FLOAT:
          LOGI("Capture Tag %s: %f", name, val.data.f[i]);
          break;
        case ACAMERA_TYPE_DOUBLE:
          LOGI("Capture Tag %s: %" PRIx64, name, val.data.i64[i]);
          break;
        case ACAMERA_TYPE_RATIONAL:
          LOGI("Capture Tag %s: %08x, %08x", name, val.data.r[i].numerator,
               val.data.r[i].denominator);
          break;
        default:
          ASSERT(false, "Unknown tag value type: %d", val.type);
      }
    }
  }
}

/*
 * CameraDevice error state translation, used in
 *     ACameraDevice_ErrorStateCallback
 */
using DEV_ERROR_PAIR = std::pair<int, const char*>;
static std::vector<DEV_ERROR_PAIR> devErrors{
    MAKE_PAIR(ERROR_CAMERA_IN_USE),   MAKE_PAIR(ERROR_MAX_CAMERAS_IN_USE),
    MAKE_PAIR(ERROR_CAMERA_DISABLED), MAKE_PAIR(ERROR_CAMERA_DEVICE),
    MAKE_PAIR(ERROR_CAMERA_SERVICE),
};

const char* GetCameraDeviceErrorStr(int err) {
  return GetPairStr<int>(err, devErrors);
}
void PrintCameraDeviceError(int err) {
  LOGI("CameraDeviceError(%#x): %s", err, GetCameraDeviceErrorStr(err));
}
