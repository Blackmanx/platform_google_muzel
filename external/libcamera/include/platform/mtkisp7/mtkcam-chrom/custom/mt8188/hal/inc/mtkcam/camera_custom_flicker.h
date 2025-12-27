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


#ifndef __CAMERA_CUSTOM_FLICKER_H__
#define __CAMERA_CUSTOM_FLICKER_H__


void cust_getFlickerHalPara(int* defaultHz, int* maxDetExpUs); //default: 50 (50hz), 70000 (70ms)
int cust_getFlickerDetectFrequency();
int cust_getMaxAttachNum();

/**
 * @brief adjust flicker parameter with specific sensor binning
 *
 * Only if the download time between two consecutive line before and after binning
 * are different needs to adjust the ratio, otherwise return 1 for normal case.
 *
 * @param[in] sensorID: specific sensor ID from kd_imgsensor.h
 * @param[in] sensorMode: specific sensor mode with binning needs to modify parameter
 * @return 1 for normal case, 2 for binning ratio 2 in the above case
 */
int cust_getSensorBinningRatio(int sensorID, int sensorMode);

#endif //#ifndef __CAMERA_CUSTOM_FLICKER_H__

