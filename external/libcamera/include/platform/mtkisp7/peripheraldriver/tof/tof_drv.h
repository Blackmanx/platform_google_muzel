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

/*
** $Log: ois_drv.h $
*
*/

#ifndef AAA_PERIPHERALDRIVER_TOF_TOF_DRV_H_
#define AAA_PERIPHERALDRIVER_TOF_TOF_DRV_H_

#include <stdio.h>

#define NUMBER_OF_MAX_TOFDRV_DATA 64
struct TofInfo {
  int32_t is_tof_supported;
  int32_t num_of_rows; /* Max : 8 */
  int32_t num_of_cols; /* Max : 8 */
  int32_t ranging_distance[NUMBER_OF_MAX_TOFDRV_DATA];
  int32_t dmax_distance[NUMBER_OF_MAX_TOFDRV_DATA];
  int32_t error_status[NUMBER_OF_MAX_TOFDRV_DATA];
  int32_t maximal_distance; /* Operating Range Distance */
  int64_t timestamp;
};

class TOFDrv {
 public:
  explicit TOFDrv(int32_t const sensor_index);
  virtual ~TOFDrv() {}

  static TOFDrv* GetInstance(int32_t const sensor_index);

  int32_t Init();
  int32_t Uninit();
  int32_t GetTofInfo(struct TofInfo* p_tof_info);

 private:
  int GetV4l2MediaDevice(char* media_name);
  int GetV4l2SubDevice(char* driver_name);
  int GetV4l2SubDevName(int major, int minor, char* subDevName);

 private:
  int32_t m_sensor_dev;
  int32_t m_sensor_index;
};

#endif  //  AAA_PERIPHERALDRIVER_TOF_TOF_DRV_H_
