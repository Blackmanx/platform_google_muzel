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

#ifndef AAA_PERIPHERALDRIVER_STROBE_STROBE_HAL_H_
#define AAA_PERIPHERALDRIVER_STROBE_STROBE_HAL_H_

#include <peripheraldriver/strobe/strobe_drv.h>
#include <camera_custom_flash_nvram.h>
#include <aaahal/include/IHalFlash.h>
#include <mutex>

/** strobe hal scenario */
typedef enum {
    STROBE_HAL_SCENARIO_TORCH,
    STROBE_HAL_SCENARIO_VIDEO_TORCH,
    STROBE_HAL_SCENARIO_AF_LAMP,
    STROBE_HAL_SCENARIO_PRE_FLASH,
    STROBE_HAL_SCENARIO_MAIN_FLASH,
    STROBE_HAL_SCENARIO_MAIN_FLASH_MAX,
    STROBE_HAL_SCENARIO_LOW_POWER,
    STROBE_HAL_SCENARIO_NUM,
} STROBE_HAL_SCENARIO_ENUM;

/** strobe on & off scenario */
typedef enum {
    STROBE_FLASH_OFF,
    STROBE_SOFTWARE_FLASH_ON,
    STROBE_HARDWARE_FLASH_ON,
} STROBE_ON_OFF_SCENARIO_ENUM;

/** flash hal info */
typedef struct StrobeHalInfo {
  int duty;
  int dutyLt;
  int timeout;
  int timeoutLt;
} StrobeHalInfo;

/** flash hal time info */
typedef struct StrobeHalTimeInfo {
  int mfStartTime;
  int mfEndTime;
  int mfTimeout;
  int mfTimeoutLt;
  int mfIsTimeout;
} StrobeHalTimeInfo;

typedef struct strobe_drv_cap {
  int32_t hasHw;
  int16_t ITabHt[FLASH_CUSTOM_MAX_DUTY_NUM_HT];
  int16_t ITabLt[FLASH_CUSTOM_MAX_DUTY_NUM_LT];
  int32_t maxDuty;
  int32_t maxDutyLt;
  int32_t partId;
} strobe_drv_cap;

typedef struct strobe_drv_info {
  int32_t isAvailable;
  int32_t isOn;
  int32_t inCharge;
  int32_t battVol;
  int32_t isLowPower;
  int32_t chargerStatus;
  int32_t driverFault;
  StrobeHalTimeInfo timeInfo;
} strobe_drv_info;

typedef struct strobe_duty_setting {
  int32_t duty;
  int32_t dutyLt;
} strobe_duty_setting;

class StrobeHal : public IHalFlash {
 public:
  /***************************************
   * Available before init()
   **************************************/
  explicit StrobeHal(int sensorDev);
  ~StrobeHal() = default;

  static StrobeHal* getInstance(int32_t const i4SensorOpenIdx);
  void destroyInstance();

  int init();
  int uninit();
  int start();
  int stop();

  int getStrobeCap(strobe_drv_cap* drv_cap);
  int getStrobeInfo(strobe_drv_info* drv_info);

  int setInfo(STROBE_HAL_SCENARIO_ENUM scenario, StrobeHalInfo info);
  int setInfoDuty(STROBE_HAL_SCENARIO_ENUM scenario, StrobeHalInfo* info);
  int setInitialDuty(strobe_duty_setting* setting);
  int getMaxDuty(int* duty, int* dutyLT);
  int getCurrentByDuty(int duty);
  int getCurrentTab(int16_t* ITabHt, int16_t* ITabLt);
  int getFlashMaxIDuty(int dutyNum, int dutyNumLt, int* duty, int* dutyLt);

  StrobeHalTimeInfo getTimeInfo();
  int getDriverFault();

  int hasHw(int* hasHw);
  int getPartId();

  /**
   * @brief get/set in charge
   *
   * For Dual camera, 2 sensor mapping to 1 flashlight.
   * Needs to maintain the control right.
   */
  int getInCharge();
  int setInCharge(int inCharge);

  /**
   * @brief is instance avalible for operation
   * @return available status
   */
  int isAvailable();

  /***************************************
   * Available since init()
   **************************************/
  void show();
  int getBattVol(int* battVol);
  int isLowPower(int* battStatus);
  int isNeedWaitCooling(int curMs, int* ms);

  /**
   * @brief Set charger
   *
   * Some flashlight driver IC (RT5081)
   * need time to switch fast-charge mode to normal mode.
   */
  int isChargerReady(int* chargerStatus);
  int setCharger(int ready);

  /**
   * @brief Check if flash is on/off.
   */
  int isFlashOn();
  int isAFLampOn();

  /**
   * @brief Set pre-on
   *
   * Some old style flashlight driver IC (MT6332)
   * need time to burst large currnet.
   */
  int setPreOn();

  /**
   * @brief Turn on/off flash.
   */
  int setFlashOn(StrobeHalInfo info);
  int setFlashOff();
  // int setOnOff(int enable, StrobeHalInfo info);
  int setOnOff(int enable, STROBE_HAL_SCENARIO_ENUM scenario);

  /**
   * @brief Turn on/off scenario flash.
   */
  int getTorchStatus();
  int setTorchOnOff(int32_t en);

  int setTorchDuty(int level);

  /************************************************************
   * Engineer mode related function
   ***********************************************************/
  int egGetDutyRange(int* start, int* end);
  int egGetStepRange(int* start, int* end);
  int egSetMfDutyStep(int duty, int step);

 private:
  mutable std::mutex mLock;

  int mSensorIdx;
  int mSensorFacing;
  int mTorchStatus;
  StrobeHalTimeInfo mStrobeHalTimeInfo;
  StrobeHalInfo mStrobeHalInfo[STROBE_HAL_SCENARIO_NUM];
  int mDriverFault;
  int mMaxDuty;
  int mMaxDutyLT;

  int mHasHw;
  int mInCharge;
  int mSetCharger;

  /** strobe handler */
  StrobeDrv* mpStrobe;
  StrobeDrv* mpStrobe2;
  STROBE_DEVICE_ENUM mStrobeDevice;
  STROBE_TYPE_ENUM mStrobeTypeId;
  int mStrobeCtNum;
  int mStrobePartId;
  int mTorchLevel;
};

#endif  // AAA_PERIPHERALDRIVER_STROBE_STROBE_HAL_H_
