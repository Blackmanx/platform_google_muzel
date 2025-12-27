/*
 * Copyright (C) 2022 MediaTek Inc.
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

#ifndef INCLUDE_MTKCAM_INTERFACES_DEF_CAMERAINFO_H_
#define INCLUDE_MTKCAM_INTERFACES_DEF_CAMERAINFO_H_

#include <mtkcam-interfaces/def/SensorMap.h>
#include <mtkcam-interfaces/def/common.h>

#include <inttypes.h>
#include <stdint.h>
#include <map>
#include <sstream>
#include <string>
#include <vector>
/******************************************************************************
 *
 ******************************************************************************/
namespace NSCam {

/******************************************************************************
 * @brief CameraInfo is abstruct concept that can be
 *        used to descript for physical sensor.
 ******************************************************************************/
struct CameraInfo {
  /**
   * 0: front, 1: back
   */
  uint8_t sensorFacing;

  /**
   * Sensor raw type.
   * SENSOR_RAW_xxx in mtkcam/include/mtkcam/drv/IHalSensor.h
   */
  uint32_t sensorRawType;

  /**
   * refer to Transformation definitions in
   * mtkcam-core\include\mtkcam-core\def\ImageFormat.h
   */
  int32_t sensorTransform;

  /**
   * 0~3: x, y, w, h
   */
  int32_t activeArray[4];

  /**
   * store aperture.
   */
  float aperture;

  /**
   * store focal length.
   */
  float focalLength;

  /**
   * The physical dimensions of the full pixel array.
   * Units: Millimeters.
   */
  float sensorPhysicalSize[2];

  /**
   * Dimensions of the full pixel array,
   * possibly including black calibration pixels.
   * Units: Pixels.
   */
  int sensorPixelArraySize[2];
};

/*
 * Store physical sensor CameraInfo.
 */
typedef SensorMap<CameraInfo> CameraInfoMapT;  // <sensorId, CameraInfo>

static inline std::string toString(const CameraInfo& o) {
  std::ostringstream oss;
  oss << "{ .sensorFacing=" << static_cast<uint32_t>(o.sensorFacing);
  oss << " .sensorRawType=" << o.sensorRawType;
  oss << " .sensorTransform=" << o.sensorTransform;
  oss << " .activeArray(x,y,w,h)=(" << o.activeArray[0] << ','
      << o.activeArray[1] << ',' << o.activeArray[2] << ',' << o.activeArray[3]
      << ')';
  oss << " .aperture=(" << o.aperture << ")";
  oss << " .focalLength=( " << o.focalLength  << ")";
  oss << " .sensorPhysicalSize(w, h)= (" << o.sensorPhysicalSize[0] << ','
      << o.sensorPhysicalSize[1] << ") }";
  return oss.str();
}

}       // namespace NSCam
#endif  // INCLUDE_MTKCAM_INTERFACES_DEF_CAMERAINFO_H_
