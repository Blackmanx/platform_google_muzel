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

#ifndef AAA_AAAHAL_INCLUDE_IHALFLASH_H_
#define AAA_AAAHAL_INCLUDE_IHALFLASH_H_

#include <mtkcam-interfaces/def/common.h>
#include <mtkcam-interfaces/utils/module/module.h>

/******************************************************************************
 *
 ******************************************************************************/
class IHalFlash {
  //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  //  Interfaces.
  //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 protected:     ////         Instantiation.
  virtual                 ~IHalFlash() {}

 public:     ////            Operations.
  static  IHalFlash*      getInstance(MINT32 i4SensorOpenIndex);
  virtual MVOID           destroyInstance()                           = 0;

  /**
  * Get current torch status
  */
  virtual MINT32          getTorchStatus()                            = 0;

  /**
  * Turn on/off torch
  */
  virtual MINT32          setTorchOnOff(MBOOL bEnable)              = 0;

 public:     ////            Operations.
  virtual MINT32          egGetDutyRange(int* st, int* ed)          = 0;
  virtual MINT32          egGetStepRange(int* st, int* ed)          = 0;
  virtual MINT32          egSetMfDutyStep(int duty, int step)       = 0;
};


/**
 * @brief The definition of the maker of HalFlash instance.
 */
typedef IHalFlash* (*HalFlash_FACTORY_T)(MINT32 i4SensorOpenIndex);


/******************************************************************************
 *
 ******************************************************************************/
#endif  // AAA_AAAHAL_INCLUDE_IHALFLASH_H_
