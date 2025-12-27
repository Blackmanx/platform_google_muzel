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
#ifndef _3DNR_CONFIG_H
#define _3DNR_CONFIG_H

#include "camera_custom_3dnr_base.h"

int get_3dnr_iso_enable_threshold_low(void);
int get_3dnr_iso_enable_threshold_high(void);
#if 0   // Obsolete
int get_3dnr_iso_enable_threshold_low_percentage(void);
int get_3dnr_iso_enable_threshold_high_percentage(void);
#endif  // Obsolete
int get_3dnr_max_iso_increase_percentage(void);
int get_3dnr_hw_power_off_threshold(void);
int get_3dnr_hw_power_reopen_delay(void);
int get_3dnr_gmv_threshold(int force3DNR);

MBOOL  is_vhdr_profile(MUINT8 ispProfile);

class NR3DCustom : public NR3DCustomBase
{
private:
    // DO NOT create instance
    NR3DCustom()
    {
    }

public:

    static MBOOL isSecure3DNRSupported();
    static MBOOL is3DNRSmvrBatchSupported();
    static MBOOL isUFBCSupported();

    static MBOOL isEnabled3DNR30()
    {
        return true;
    }

    static MBOOL isEnabled3DNR35()
    {
        return true;
    }

    static MBOOL isSupportRSC();
    static MBOOL isEnabledRSC(MUINT32 mask);

    static MINT32 get_3dnr_off_iso_threshold(MUINT8 ispProfile = 0, MBOOL useAdbValue = 0);
};

#endif /* _3DNR_CONFIG_H */


