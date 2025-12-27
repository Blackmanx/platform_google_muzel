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

#ifndef INCLUDE_MTKCAM_CORE_NVRAM_INVBUFUTIL_H_
#define INCLUDE_MTKCAM_CORE_NVRAM_INVBUFUTIL_H_
//
#include <camera_custom_nvram_pub.h>
#include <string>

struct NVRAM_SENSOR_IDX_INFO {
  MUINT32 sensorDev;
  MUINT32 sensorId;
  MUINT32 facing;
  MUINT32 facingNum;
  MUINT32 moduleId;
  std::string sensorName;
};


class INvBufUtil {
 public:
  enum {
    e_SensorDevWrong = -1000,
    e_NvramIdWrong,
    e_NV_SensorDevWrong,
    e_NV_SensorIdNull,
  };

 public:
  virtual ~INvBufUtil() {}

  static INvBufUtil* getInstance();

  virtual int getBuf(CAMERA_DATA_TYPE_ENUM nvRamId,
                     int sensorDev,
                     void*& p) = 0;
  virtual int getBufAndRead(CAMERA_DATA_TYPE_ENUM nvRamId,
                            int sensorDev,
                            void*& p,
                            int bForceRead = 0) = 0;
  virtual int getBufAndReadNoDefault(CAMERA_DATA_TYPE_ENUM nvRamId,
                                     int sensorDev,
                                     void*& p,
                                     int bForceRead = 0) = 0;
  int getSensorIdAndModuleId(MINT32 sensorType,
                             MINT32 sensoridx,
                             MINT32& sensorId,
                             MUINT32& moduleId);

  virtual int write(CAMERA_DATA_TYPE_ENUM nvRamId, int sensorDev) = 0;

  // note: please provide memory to call the function.
  // For sync the buf data with NvRam data, the internal buf can't be used in
  // the function.
  virtual int readDefault(CAMERA_DATA_TYPE_ENUM nvRamId,
                          int sensorDev,
                          void* p) = 0;
  virtual int readDefault(CAMERA_DATA_TYPE_ENUM nvRamId, int sensorDev) = 0;
};

/******************************************************************************
 *
 ******************************************************************************/
#endif  // INCLUDE_MTKCAM_CORE_NVRAM_INVBUFUTIL_H_
