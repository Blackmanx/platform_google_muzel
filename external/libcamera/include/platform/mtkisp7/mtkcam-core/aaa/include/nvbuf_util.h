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

#ifndef AAA_INCLUDE_NVBUF_UTIL_H_
#define AAA_INCLUDE_NVBUF_UTIL_H_

#include <mtkcam-halif/def/BuiltinTypes.h>
#include <mtkcam-core/nvram/INvBufUtil.h>
#include <mutex>
#include <string>

#ifdef WIN32
#define DEF_CriticalSection_Win(cs) CRITICAL_SECTION cs;
#define DEF_CriticalSection_Linux(cs)
#define DEF_AutoLock(cs) AutoLock lock(cs)
#define DEF_InitCs(cs) InitializeCriticalSection(&cs)
#else

// #include <utils/threads.h>
#define DEF_CriticalSection_Win(cs)
#define DEF_CriticalSection_Linux(cs) mutable std::mutex cs;
#define DEF_AutoLock(cs) std::lock_guard<std::mutex> lock(cs)
#define DEF_InitCs(cs)
#endif
//
#define INVALID_MODULEID (99999)

template <typename T>
struct NVRAM_INST_T {
  std::mutex lock;
  T* instance;
  NVRAM_INST_T() : instance(nullptr) {}
};

typedef enum {
  NVRAM_BUFFER_GET,
  NVRAM_BUFFER_RELEASE,
} NVRAM_BUFFER_OPERATION;

class NvBufUtil : public INvBufUtil {
 public:
  static NvBufUtil& getInstance();
  static void initSensorInfo(uint32_t sensorIdx, NVRAM_SENSOR_IDX_INFO& input);

  virtual const char* getSensorNameWithSensorIdx(uint32_t sensorIdx);
  virtual MUINT32 getModuleIdWithSensorIdx(uint32_t sensorIdx);
  virtual MINT32 getSensorIdxWithDev(MINT32 sensorDev);
  virtual MINT32 getSensorIDWithDev(MINT32 sensorDev);
  virtual int getBuf(CAMERA_DATA_TYPE_ENUM nvRamId, int sensorDev, void*& p);
  virtual int getBufAndRead(CAMERA_DATA_TYPE_ENUM nvRamId,
                            int sensorDev,
                            void*& p,
                            int bForceRead = 0);
  virtual int getBufAndReadNoDefault(CAMERA_DATA_TYPE_ENUM nvRamId,
                                     int sensorDev,
                                     void*& p,
                                     int bForceRead = 0);
  virtual int releaseBuf(CAMERA_DATA_TYPE_ENUM nvRamId, int sensorDev);

  virtual int storeSensorModuleID(int sensorDev, MUINT32 moduleId);

  int getSensorIdAndModuleId(MINT32 sensorType,
                             MINT32 sensoridx,
                             MINT32& sensorId,
                             MUINT32& moduleId);

  virtual int write(CAMERA_DATA_TYPE_ENUM nvRamId, int sensorDev);

  // note: please provide memory to call the function.
  // For sync the buf data with NvRam data, the internal buf can't be used in
  // the function.
  virtual int readDefault(CAMERA_DATA_TYPE_ENUM nvRamId,
                          int sensorDev,
                          void* p);
  virtual int readDefault(CAMERA_DATA_TYPE_ENUM nvRamId, int sensorDev);

 private:
  DEF_CriticalSection_Linux(m_cs)
  NvBufUtil();
};

int nvbufUtil_getSensorId(int sensorDev, int& sensorId);

#endif  // AAA_INCLUDE_NVBUF_UTIL_H_

