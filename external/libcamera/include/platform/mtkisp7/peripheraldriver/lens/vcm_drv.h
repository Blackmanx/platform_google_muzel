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
** $Log: lens_drv.h $
*
*/

#ifndef AAA_PERIPHERALDRIVER_LENS_VCM_DRV_H_
#define AAA_PERIPHERALDRIVER_LENS_VCM_DRV_H_

#include <stdio.h>
#include <thread>

struct VcmFocusInformation {
  int32_t focus_position;
  int32_t previous_focus_position;
  int64_t moving_timestamp;  // Unit : us
  int64_t previous_moving_timestamp;
};

class VCMDrv {
 public:
  explicit VCMDrv(int32_t const sensor_index);
  virtual ~VCMDrv() {}

  static VCMDrv* GetInstance(int32_t const sensor_index);

  int32_t Init(int32_t sensor_id, int32_t module_id);
  int32_t Uninit();
  int32_t SetFocusPosition(int32_t position);
  int32_t GetFocusInformation(struct VcmFocusInformation* p_focus_info);
  int32_t IsVcmSupported(int32_t sensor_id, int32_t module_id);

 private:
  int GetV4l2MediaDevice(char* media_name);
  int GetV4l2SubDevice(char* driver_name);
  int GetV4l2SubDevName(int major, int minor, char* subDevName);
  void* InitDrv(void);

 private:
  int32_t m_sensor_dev;
  int32_t m_sensor_index;

  int32_t m_user_count;
  int m_fd_driver;

  int32_t m_vcm_id;
  char m_vcm_drv_name[32];

  int32_t m_focus_position;
  int32_t m_previous_focus_position;
  int64_t m_moving_timestamp;           // Unit : us
  int64_t m_previous_moving_timestamp;  // Unit : us

  int64_t m_time1, m_time2;
  int32_t m_auto_test_enable;
  int32_t m_position_timestamp;

  int32_t m_init_flag;
  int32_t m_last_end_position;

  std::thread m_thread;
};

#endif  //  AAA_PERIPHERALDRIVER_LENS_VCM_DRV_H_
