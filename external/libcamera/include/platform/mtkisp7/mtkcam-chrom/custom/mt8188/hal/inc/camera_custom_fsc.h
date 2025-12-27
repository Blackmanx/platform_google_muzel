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
#ifndef _FSC_CONFIG_H
#define _FSC_CONFIG_H

#include "camera_custom_fsc_base.h"

class FSCCustom : public FSCCustomBase
{
private:
    // DO NOT create instance
    FSCCustom()
    {
    }

public:

    static MBOOL isSupportFSC();
    static MBOOL isEnabledFSC(MUINT32 mask);
    static MINT32 getDebugLevel();
    static MINT32 getMarcoToInfRatioOffset();
    static MINT32 getMaxCropRatio();
    static MINT32 getAfDampTimeOffset();
};

#endif /* _FSC_CONFIG_H */


