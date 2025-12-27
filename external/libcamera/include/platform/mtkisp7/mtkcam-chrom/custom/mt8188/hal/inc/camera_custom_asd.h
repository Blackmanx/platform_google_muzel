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
#ifndef _ASD_CONFIG_H
#define _ASD_CONFIG_H

typedef struct
{
    unsigned char u1TimeWeightType;
    unsigned char u1TimeWeightRange;    
    unsigned char u1ScoreThrNight;
    unsigned char u1ScoreThrBacklit;
    unsigned char u1ScoreThrPortrait;
    unsigned char u1ScoreThrLandscape;
}ASD_Customize_PARA1;

typedef struct
{
	short int s2IdxWeightBlAe;
    short int s2IdxWeightBlScd;    
	short int s2IdxWeightLsAe;        
  	short int s2IdxWeightLsAwb;
    short int s2IdxWeightLsAf;    
    short int s2IdxWeightLsScd;
    short int s2EvLoThrNight;
    short int s2EvHiThrNight;
    short int s2EvLoThrOutdoor;
    short int s2EvHiThrOutdoor;
    bool boolBacklitLockEnable;
    short int s2BacklitLockEvDiff;  
}ASD_Customize_PARA2;


void get_asd_CustomizeData1(ASD_Customize_PARA1  *ASDDataOut1);
void get_asd_CustomizeData2(ASD_Customize_PARA2  *ASDDataOut2);
	
#endif /* _FD_CONFIG_H */

