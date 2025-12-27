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

/**
 * @file IPeripheralController.h
 * @brief Declarations of Abstraction of control lens and flash driver
 */
#ifndef AAA_PERIPHERALCONTROLLER_INCLUDE_IPERIPHERALCONTROLLER_H_
#define AAA_PERIPHERALCONTROLLER_INCLUDE_IPERIPHERALCONTROLLER_H_

// std lib
#include <stdio.h>
#include <array>
#include <memory>
#include "PeripheralInfoDef.h"

namespace mtk {
namespace hal3a {

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
class IPeripheralController {
  static const std::array<SensorStaticInfo, kMaxSensorCnt>&
  GetSensorStaticInfoArray();
  friend class PeripheralController;

 public:
  enum MtkNotifyEventId {
    kBegin = 0,
    kFocusMove,
    kGetFocusInfo,
    kIsAfSupported,
    kGetOisInfo,
    kOisOnOff,
    kGetOisMv,
    kSetOisCfgData,
    kSetOisParams,
    kNotifyOisStreamOn,
    kNotifyOisStreamOff,
    kGetTofInfo,
    kFlashSupport,
    kFlashOnOff,
    kFlashInitialDuty,
    kFlashSetting,
    kGetFlashInfo,
    kGetFlashCapability,
    kGetCalData,
    kGetGyroMEBuf,
    kGetGyroData,
    kGetAcceData,
    kGetLightData,
    kGetALSDataAll,
    kGetFlickerData,
    kGetColorData,
    kIsIrisSupported,
    kSetIrisStep,
    kGetIrisData,
    kSetIrcutOnOff,
    kGetIrcutData,
    kIsIrcutSupported,
    kIsOpticalZoomSupported,
    kGetSensorStaticInfo,
    kGetSensorInitialDynamicInfo,
    kGetSensorConfigDynamicInfo,
    kGetSensorPerframeDynamicInfo,
    kSetSensorPerframeDynamicInfo,
    kDisableSensorProvider,
    kNum
  };

 public:
  /**
   * Create the latest IPeripheralController instance.
   *  @param[in] sensor_id Sensor device unique ID.
   *  @param[in] sensor index of the given |sensor_id|.
   *  @return The instance of IPeripheralController.
   */
  static std::shared_ptr<IPeripheralController> GetInstance(
      uint32_t sensor_idx);

  virtual ~IPeripheralController() = default;

 public:
  virtual void notifyPowerOn() = 0;
  virtual void notifyPowerOff() = 0;

  /**
   * @brief notify callback
   * @param [in] eId Notification message type
   * @param [in] i4Arg0 is one parameter
   * @param [in] i4Arg1 is one parameter
   * @param [in] i4Arg2 is second parameter
   * @param [in] i4Arg3 is third parameter
   */
  virtual void NotifyEvent(MtkNotifyEventId notify_event_id,
                           intptr_t arg0,
                           intptr_t arg1,
                           intptr_t arg2,
                           intptr_t arg3) = 0;
};

}       // namespace hal3a
}       // namespace mtk
#endif  // AAA_PERIPHERALCONTROLLER_INCLUDE_IPERIPHERALCONTROLLER_H_
