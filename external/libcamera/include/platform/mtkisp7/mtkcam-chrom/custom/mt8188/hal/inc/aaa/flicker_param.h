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

#ifndef _FLICKER_PARAM_H
#define _FLICKER_PARAM_H
typedef struct FLKWinCFG_S
{
    MINT32 m_uImageW;
    MINT32 m_uImageH;
    MINT32 m_u4OffsetX;
    MINT32 m_u4OffsetY;
    MINT32 m_u4NumX;
    MINT32 m_u4NumY;
    MINT32 m_u4SizeX;
    MINT32 m_u4SizeY;
    MINT32 m_u4DmaSize;
    MINT32 m_u4SGG3_PGN;
    MINT32 m_u4SGG3_GMR1;
    MINT32 m_u4SGG3_GMR2;
    MINT32 m_u4SGG3_GMR3;
    MINT32 m_u4SGG3_GMR4;
    MINT32 m_u4SGG3_GMR5;
    MINT32 m_u4SGG3_GMR6;
    MINT32 m_u4SGG3_GMR7;
    MINT32 m_u4INPUT_BIT_SEL;
    MINT32 m_u4ZHDR_NOISE_VAL;
    MINT32 m_u4SGG_OUT_MAX_VAL;
}FLKWinCFG_T;
#endif

