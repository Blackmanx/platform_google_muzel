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


#ifndef INCLUDE_MTKCAM_INTERFACES_UTILS_SYS_SENSOR_TYPE_H_
#define INCLUDE_MTKCAM_INTERFACES_UTILS_SYS_SENSOR_TYPE_H_

#include <mtkcam-interfaces/def/BuiltinTypes.h>
#include <mtkcam-interfaces/def/UITypes.h>
#include <vector>

#define GYRO_MV_DATA_INTERVAL 5  // ms

namespace NSCam {
namespace Utils {

struct SensorData {
  SensorData() : acceleration{0}, gyro{0}, magnetic{0},
                 data{0}, light(0), als(0), rgbw{0},
                 oisGyro{0}, oisHall{0}, ois{0}, flicker{0},
                 timestamp(0) {}
  MFLOAT acceleration[3];
  MFLOAT gyro[3];
  MFLOAT magnetic[3];
  MFLOAT data[4];
  MFLOAT light;
  MFLOAT als;
  MFLOAT rgbw[10];
  MFLOAT oisGyro[2];
  MFLOAT oisHall[2];
  MFLOAT ois[2];
  MFLOAT flicker[8];
  MINT64 timestamp;
};

enum eSensorType {
  SENSOR_TYPE_GYRO = 0,
  SENSOR_TYPE_ACCELERATION,
  SENSOR_TYPE_LIGHT,
  SENSOR_TYPE_ALS,
  SENSOR_TYPE_FLICKER_DET,
  SENSOR_TYPE_RGBW,
  SENSOR_TYPE_MAGNETIC_FIELD,
  SENSOR_TYPE_GAME_ROTATION_VECTOR,
  SENSOR_TYPE_OIS_DATA,
  SENSOR_TYPE_ALS_FRONT,
  SENSOR_TYPE_FLICKER_FRONT,
  SENSOR_TYPE_RGBW_FRONT,
  SENSOR_TYPE_COUNT,
};

enum eSensorStatus {
  STATUS_UNINITIALIZED = 0,
  STATUS_INITIALIZED,
  STATUS_ERROR,
};

struct GyroMVInitData {
  MINT32 sensorIdx = 0;
  MINT32 sensorMode = 0;
  MINT32 sensor_Width = 0;
  MINT32 sensor_Height = 0;
  MINT32 rrz_crop_Width = 0;
  MINT32 rrz_crop_Height = 0;
  MINT32 rrz_crop_X = 0;
  MINT32 rrz_crop_Y = 0;
  MINT32 rrz_scale_Width = 0;
  MINT32 rrz_scale_Height = 0;
  MINT32 crz_crop_Width = 0;
  MINT32 crz_crop_Height = 0;
  MINT32 crz_crop_X = 0;
  MINT32 crz_crop_Y = 0;
  MINT32 mvWidth = 0;
  MINT32 mvHeight = 0;
  MINT32 mvResultSize = 0;
  MINT32 gyroNumSize = 0;
  MUINT64 sleep_t = 0;
};

struct GyroMVProcData {
  MINT64 frame_AE = 0;
  MINT64 frame_t = 0;
  std::vector<SensorData> gyroData;
  std::vector<SensorData> oisData;
};

struct GyroMVResult {
  MUINT64 frameTs = 0;
  MUINT8* mv = NULL;
  MUINT8* valid_gyro_num = NULL;
  MUINT32 mvSize = 0;
  MUINT32 gyroNumSize = 0;
  MUINT32 mvWidth = 0;
  MUINT32 mvHeight = 0;
  MUINT32 imgW = 0;
  MUINT32 imgH = 0;
};

struct OisMVResult {
  MUINT64 frameTs = 0;
  double *ois_positionX = NULL;  // OIS_VECTOR_SIZE * MAX_MV_V_NUM
  double *ois_positionY = NULL;  // OIS_VECTOR_SIZE * MAX_MV_V_NUM
};

struct RSCMEResult {
  MBOOL isValid = MFALSE;
  MINT32 sensorID = -1;
  MINT32 requestNo = -1;
  MUINT8* pMV = NULL;
  MUINT8* pBV = NULL;
  MSize rrzoSize;
  MSize rssoSize;
  MUINT32 mvSize = 0;
  MUINT32 bvSize = 0;
  MUINT32 staGMV = 0;
  MINT64 TS = 0;
};

struct ImageSensorInfo {
  MUINT32 used = 0;
  MUINT32 sensorDev = 0;
  MINT64 trsTime = 0;
  MUINT32 sensorPixelClock = 0;
  MUINT32 sensorLinePixel = 0;
  MUINT32 defWidth = 0;
  MUINT32 defHeight = 0;
  MUINT32 defCrop = 0;
  MDOUBLE recordParameter[6];

  ImageSensorInfo() { memset(recordParameter, 0, sizeof(recordParameter)); }
};

}  // namespace Utils
}  // namespace NSCam

#endif  // INCLUDE_MTKCAM_INTERFACES_UTILS_SYS_SENSOR_TYPE_H_
