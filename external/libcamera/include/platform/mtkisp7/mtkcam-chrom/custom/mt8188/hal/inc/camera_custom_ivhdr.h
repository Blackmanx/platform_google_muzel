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
#ifndef _CAMERA_CUSTOM_IVHDR_H_
#define _CAMERA_CUSTOM_IVHDR_H_

#include "camera_custom_types.h"  // For MUINT*/MINT*/MVOID/MBOOL type definitions.

#define CUST_IVHDR_DEBUG          0   // Enable this will dump HDR Debug Information into SDCARD
#define IVHDR_DEBUG_OUTPUT_FOLDER   "/storage/sdcard1/" // For ALPS.JB.
/**************************************************************************
 *                      D E F I N E S / M A C R O S                       *
 **************************************************************************/

/**************************************************************************
 *     E N U M / S T R U C T / T Y P E D E F    D E C L A R A T I O N     *
 **************************************************************************/

/**************************************************************************
 *                 E X T E R N A L    R E F E R E N C E S                 *
 **************************************************************************/

/**************************************************************************
 *        P U B L I C    F U N C T I O N    D E C L A R A T I O N         *
 **************************************************************************/

/*******************************************************************************
* IVHDR exposure setting
*******************************************************************************/
typedef struct IVHDRExpSettingInputParam_S
{
    MBOOL bIs60HZ;                        // Flicker 50Hz:0, 60Hz:1
    MUINT32 u4ShutterTime;           // unit: us
    MUINT32 u4SensorGain;            // 1x=1024
    MUINT32 u4IspGain;
    MUINT32 u41xGainISO;             // ISO value for 1x gain
    MUINT32 u4SaturationGain;      // saturation gain for Sensor min gain.
} IVHDRExpSettingInputParam_T;

typedef struct IVHDRExpSettingOutputParam_S
{
    MBOOL bEnableWorkaround;    // MTRUE : enable, MFALSE : disable
    MUINT32 u4SEExpTimeInUS;     // unit: us short exposure
    MUINT32 u4SESensorGain;        // 1x=1204 sensor gain
    MUINT32 u4SEISPGain;              // 1x=1204 isp gain
    MUINT32 u4LEExpTimeInUS;     // unit: us long exposure
    MUINT32 u4LESensorGain;        // 1x=1204 sensor gain
    MUINT32 u4LEISPGain;              // 1x=1204 isp gain
    MUINT32 u4MEExpTimeInUS;     // unit: us middle exposure
    MUINT32 u4MESensorGain;        // 1x=1204 sensor gain
    MUINT32 u4MEISPGain;              // 1x=1204 isp gain
    MUINT32 u4VSEExpTimeInUS;     // unit: us very short exposure
    MUINT32 u4VSESensorGain;        // 1x=1204 sensor gain
    MUINT32 u4VSEISPGain;              // 1x=1204 isp gain
    MUINT32 u4LE_SERatio_x100;   // 100x
} IVHDRExpSettingOutputParam_T;

MVOID getIVHDRExpSetting(const IVHDRExpSettingInputParam_T& rInput, IVHDRExpSettingOutputParam_T& rOutput);

/**************************************************************************
 *                   C L A S S    D E C L A R A T I O N                   *
 **************************************************************************/

#endif // _CAMERA_CUSTOM_IVHDR_H_

