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
#ifndef _FD_CONFIG_H
#define _FD_CONFIG_H

typedef struct
{
    unsigned int FDThreadNum;
    unsigned int FDThreshold;
    unsigned int MajorFaceDecision;
    unsigned int OTRatio;
    unsigned int SmoothLevel;
    unsigned int Momentum;
    unsigned int MaxTrackCount;
    unsigned int FDSkipStep;
    unsigned int FDRectify;
    unsigned int FDRefresh;
    unsigned int SDThreshold;
    unsigned int SDMainFaceMust;
    unsigned int SDMaxSmileNum;
    unsigned int GSensor;
    unsigned char FDModel;
    unsigned char OTFlow;
    unsigned char UseCustomScale;
    float FDSizeRatio;
    unsigned int  SkipPartialFD;
    unsigned int  SkipAllFD;
}FD_Customize_PARA;


void get_fd_CustomizeData(FD_Customize_PARA  *FDDataOut);

#endif /* _FD_CONFIG_H */

