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

#ifndef AAA_PERIPHERALDRIVER_STROBE_STROBE_DRV_H_
#define AAA_PERIPHERALDRIVER_STROBE_STROBE_DRV_H_

#include <strobe_param.h>
#include <stdint.h>

/* strobe driver */
class StrobeDrv {
 protected:
  virtual ~StrobeDrv() = 0;

 public:
  /***************************************
   * Available before init()
   **************************************/
  static StrobeDrv* getInstance(STROBE_DEVICE_ENUM device,
                                int typeId,
                                int ctId);

  /** part */
  virtual int getPartId(int* partId) = 0;

  /** has hw or not */
  virtual int hasFlashHw(int* hasHw) = 0;

  /** flash project parameter */
  virtual int setStrobeInfo(int dutyNum,
                            int tabNum,
                            int* tabId,
                            int* timeOutMs,
                            float* coolingTM) = 0;

  /** init/uninit */
  virtual int init() = 0;
  virtual int uninit() = 0;

  /***************************************
   * Available since init()
   **************************************/
  /** duty */
  virtual int setDuty(int duty) = 0;
  virtual int getDuty(int* duty) = 0;
  virtual int getMaxDuty(int* duty) = 0;
  virtual int getCurrentByDuty(int duty, int* current) = 0;
  virtual int getCurrentTable(int16_t* ITab) = 0;

  /** pre-enable */
  virtual int setPreOn() = 0;
  virtual int getPreOnTimeMs(int* ms) = 0;
  virtual int getPreOnTimeMsDuty(int duty, int* ms) = 0;

  /** battery */
  virtual int getBattVol(int* battVol) = 0;
  virtual int isLowPower(int* battStatus) = 0;
  virtual int lowPowerDetectStart(int lowPowerDuty) = 0;
  virtual int lowPowerDetectEnd() = 0;

  /** charger */
  virtual int isChargerReady(int* chargerStatus) = 0;
  virtual int setCharger(int ready) = 0;

  /** enable/disable */
  virtual int isOn(int* isOn) = 0;
  virtual int setOnOff(int isOn) = 0;
  virtual int setStrobe(int state) = 0;

  /** time */
  virtual int setTimeOutTime(int ms) = 0;
  virtual int getTimeOutTime(int duty, int* timeOut) = 0;
  virtual int getCoolTM(int duty, float* coolTM) = 0;

  /** fault */
  virtual int getHwFault(int* fault) = 0;
};

#endif  // AAA_PERIPHERALDRIVER_STROBE_STROBE_DRV_H_
