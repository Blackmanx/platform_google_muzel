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


#ifndef _PD_BUF_MGR_H_
#define _PD_BUF_MGR_H_

#ifdef MTK_LOG_ENABLE
#undef MTK_LOG_ENABLE
#endif
#define MTK_LOG_ENABLE 1
#include <utils/Log.h>
#include "pd_buf_common.h"
#include "kd_imgsensor_define.h"

//for pdo general path
typedef enum
{
    R = 1,
    L = 2
} LR_T;

typedef struct
{
    unsigned int cx; //coordinate
    unsigned int cy;
    unsigned int bx; //block
    unsigned int by;
    LR_T lr;
} PDPIXEL_T;

typedef struct
{
    unsigned int cx;
    unsigned int cy;
    unsigned int row; //pdo buffer row
    LR_T lr;
} PDMAP_T;

#define MAX_PDO_BUF_ROW_NUM 16
#define MAX_LR_ROW 8
#define MAX_PAIR_NUM 16

typedef struct
{
    MINT32 i4RawWidth;
    MINT32 i4RawHeight;
} SCALE_FULL_RAW_SIZE_T;

typedef struct
{
    /* input arg */
    int i4AETargetMode;
    int i4VCFeature;

    /* output var */
    unsigned int i4BinningX;
    unsigned int i4BinningY;
    unsigned int i4BufFmt;
} DENSE_PD_VC_SETTING_T;

typedef enum
{
    /* Add command before this line */
    SET_CVT_ION_BUF_PTR = 0x0000,

    GET_SCALE_FULL_RAW_SIZE = 0x3001,

    GET_PD_CALIBRATION_MODE = 0x4001,
    GET_DENSE_PD_VC_SETTING = 0x4002,
    PDBUFMGR_CMD_NUM

} PDBUFMGR_CMD_t;

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  PD buffer manager I/F : using MTK pd algo.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
class PDBufMgr
{
private:

protected:
    SPDProfile_t        m_sPdProfile;
    SET_PD_BLOCK_INFO_T m_PDBlockInfo;
    SPDOHWINFO_T        m_sPDOHWInfo;

    //for pdo general separate method
    unsigned int m_subBlkNumX;
    unsigned int m_subBlkNumY;
    unsigned int m_rowNum;
    bool somePairedLRLedByL; //occurred when same paired LR at the same row and L comes first
    PDMAP_T pPdMap[MAX_PAIR_NUM * 2];
    PDPIXEL_T pdPixels[MAX_PAIR_NUM * 2];

    /**
    * @brief checking current setting is suport PDAF or not.
    */
    virtual MBOOL IsSupport( SPDProfile_t &iPdProfile) = 0;

public:
    PDBufMgr();
    virtual ~PDBufMgr();

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//                                      Interface
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    /**
    * @brief create pd buffer manager instance.
    */
    static PDBufMgr* createInstance( SPDProfile_t &iPdProfile);

    /* Input */
    /**
    * @brief set pd block information
    */
    virtual MBOOL SetPDInfo( SET_PD_BLOCK_INFO_T &iPDBlockInfo, SPDProfile_t &iPdProfile, SPDOHWINFO_T &iPDOHWInfo);
    /**
    * @brief send command.
    */
    virtual MBOOL sendCommand( MUINT32  i4Cmd, MVOID* arg=NULL) { CAM_LOGD("pd_buf_mgr : sendCommand is no use"); return 1; };

    /**
    * @brief convert PD data buffer format.
    */
    virtual MUINT16* ConvertPDBufFormat( MUINT32 i4Size, MUINT32 i4Stride, MUINT8 *ptrBufAddr, MUINT32 u4BitDepth=12, PD_AREA_T *ptrPDRegion=NULL) = 0;

    /**
    * @brief get m_PDXSz, m_PDYSz, and m_PDBufSz.
    * must called after ConvertPDBufFormat() calculated
    */
    virtual MBOOL GetLRBufferInfo(MUINT32 &PDXsz, MUINT32 &PDYsz, MUINT32 &PDBufSz);

    /**
    * @brief output DualPD VC information
    */
    virtual MBOOL GetDualPDVCInfo( MINT32 i4CurSensorMode, SDUALPDVCINFO_T &oDualPDVChwInfo, MINT32 i4AETargetMode);

    /**
    * @for pdo general separate method: separate LR by pd mapping
    */
    virtual MVOID separateLR( unsigned int stride, unsigned char *ptr, unsigned int pd_x_num, unsigned int pd_y_num, unsigned short *outBuf, unsigned int bitDepth, PD_AREA_T *ptrPDRegion=NULL);
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

};
#endif
