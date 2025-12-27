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
** $Log: ozoom_drv.h $
*
*/

#ifndef AAA_PERIPHERALDRIVER_OZOOM_OZOOM_DRV_H_
#define AAA_PERIPHERALDRIVER_OZOOM_OZOOM_DRV_H_

#include <stdio.h>

#include <mutex>

#define OZOOM_FILE_NAME_SIZE 32
#define OZOOM_PROFILE_BUF_NUM 32

// O-Zoom +++ -- lens driving type
typedef enum LensDrivingType {
  ENUM_LENS_DRV_MANUAL,
  ENUM_LENS_DRV_SCREW,
  ENUM_LENS_DRV_MOTORIZED,
  ENUM_LENS_DRV_ICS,
  ENUM_LENS_DRV_INVALID,
} LensDrivingType;

// O-Zoom +++ -- motor phase type
typedef enum StepperPhaseType {
  ENUM_STEPPING_NONE_PHASE,
  ENUM_STEPPING_2_2_PHASE,
  ENUM_STEPPING_1_2_PHASE,
  ENUM_STEPPING_TOTAL,
} StepperPhaseType;

// O-Zoom +++ -- lens profile from custom
typedef struct OpticalZoomProfile {
  int32_t driving_type; /* Manual/Screw/Motorized/ICS */
  int32_t phase_type;   /* (0):2-2phase, (1):1-2phase */
  int32_t
      cw_to_near_tele;   /* motor cw(forward) to increase focus/zoom position */
  int32_t pi_pos;        /* step position of PI */
  int32_t init_pos;      /* default pos while focus/zoom initial done */
  int32_t far_wide_opt;  /* optical position @ far/wide */
  int32_t near_tele_opt; /* optical position @ near/tele */
  int32_t far_wide_limit;    /* mechanical limit pos @ far/wide */
  int32_t near_tele_limit;   /* mechanical limit pos @ near/tele */
  int32_t defpps;            /* default driving PPS */
  int32_t minpps;            /* min driving PPS */
  int32_t maxpps;            /* max driving PPS */
  int32_t stroke_limit;      /* stroke between zoom and focus */
  int32_t pi_home_level;     /* IO level of PI while at home/close */
  int32_t pi_far_wide_level; /* IO level of PI while @ WIDE/FAR STOP position */
  int32_t reset_to_home;     /* Force reset to home when lunch camera */
  int32_t algo_pos_offset;   /* Optical wide/far offset */
} OpticalZoomProfile;

// O-Zoom +++ -- structure of motor init
typedef struct StepperMotorInit {
  int32_t group_id;
  int32_t reset_to_home;
} StepperMotorInit;

// O-Zoom +++ -- structure of motor set parameter
typedef struct MotorParameter {
  int32_t direction;
  int32_t steps;
  int32_t pps;
  int32_t excitation;
  int32_t blocking_wait;
} MotorParameter;

typedef struct StepperMotorParameter {
  int32_t group_id;
  MotorParameter focus;
  MotorParameter zoom;
} StepperMotorParameter;

// O-Zoom +++ -- structure of motor get info
typedef struct StepperMotorInformation {
  int32_t exc_status;
  int32_t exc_step;
} StepperMotorInformation;

// O-Zoom +++ -- structure of lens set parameter
typedef struct OpticalZoomParameter {
  int32_t focus_target;
  int32_t focus_exc;
  int32_t focus_speed;
  int32_t zoom_target;
  int32_t zoom_exc;
  int32_t zoom_speed;
  int32_t lens_reset;
} OpticalZoomParameter;

// O-Zoom +++ -- structure of lens get information
struct OpticalZoomInformation {
  int32_t focus_position;
  int32_t previous_focus_position;
  int32_t focus_status;
  int32_t focus_near_stop;
  int32_t focus_far_stop;
  int32_t zoom_position;
  int32_t zoom_status;
  int32_t previous_zoom_position;
  int64_t moving_timestamp;  // Unit : us
  int64_t previous_moving_timestamp;
};

class OzoomDrv {
 public:
  explicit OzoomDrv(int32_t const sensor_index);
  virtual ~OzoomDrv() {}

  static OzoomDrv* GetInstance(int32_t const sensor_index);
  int32_t Init(int32_t sensor_id, int32_t module_id);
  int32_t Uninit();
  int32_t SetOzoomParameter(OpticalZoomParameter* set_param);
  int32_t GetOzoomInformation(struct OpticalZoomInformation* get_info);
  int32_t IsOpticalZoomSupported(int32_t sensor_id, int32_t module_id);

 private:
  int GetV4l2MediaDevice(char* media_name);
  int GetV4l2SubDevice(char* driver_name);
  int GetV4l2SubDevName(int major, int minor, char* subDevName);
  void ResetOzoomInformation(void);
  void LoadOzoomLensProfile(void);
  int32_t ConvertLensMotorDirection(int32_t src,
                                  int32_t dest,
                                  OpticalZoomProfile* profile);
  int32_t ConvertLensMotorStep(int32_t src,
                             int32_t dest,
                             OpticalZoomProfile* profile);
  int32_t GetLensPosition(int32_t target_position,
                          OpticalZoomProfile* profile,
                          StepperMotorInformation* motor_info);
  int32_t LunchOzoomThread(void);

 private:
  int32_t m_sensor_dev;
  int32_t m_sensor_index;

  int32_t m_user_count;
  int32_t m_fd_driver;

  int32_t m_ozoom_id;
  char m_ozoom_drv_name[OZOOM_FILE_NAME_SIZE];

  int64_t m_entry_timestamp;           // Unit : us
  int64_t m_moving_timestamp;           // Unit : us
  int64_t m_previous_moving_timestamp;  // Unit : us

  // O-Zoom +++ -- declare init done flag
  int32_t ozoom_init_done;
  // O-Zoom +++ -- declare thread
  int32_t ozoom_thread_en;
  // O-Zoom +++ -- declare mutex lock
  std::mutex ozoom_lock;
  // O-Zoom +++ -- declare lens profile
  OpticalZoomProfile focus_profile;
  OpticalZoomProfile zoom_profile;
  // O-Zoom +++ -- declare motor param
  StepperMotorParameter stm_param;
  // O-Zoom +++ -- declare lens info
  int32_t focus_position;
  int32_t previous_focus_position;
  int32_t focus_status;
  int32_t zoom_position;
  int32_t previous_zoom_position;
  int32_t zoom_status;
  int32_t focus_target;
  int32_t focus_excitation;
  int32_t focus_speed;
  int32_t zoom_target;
  int32_t zoom_excitation;
  int32_t zoom_speed;
  // O-Zoom +++ -- declare lens profile buffer
  int32_t zoom_profile_buf[OZOOM_PROFILE_BUF_NUM];
  int32_t focus_profile_buf[OZOOM_PROFILE_BUF_NUM];
};

#endif  // AAA_PERIPHERALDRIVER_OZOOM_OZOOM_DRV_H_
