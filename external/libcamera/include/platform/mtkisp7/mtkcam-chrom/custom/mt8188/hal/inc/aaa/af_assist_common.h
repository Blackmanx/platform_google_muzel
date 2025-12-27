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


#ifndef _AF_ASSIST_COMMON_H_
#define _AF_ASSIST_COMMON_H_

#ifdef MTK_LOG_ENABLE
#undef MTK_LOG_ENABLE
#endif

#define MTK_LOG_ENABLE 1

#include <vector>

#include <aaa_types.h>
#include <aaa_error_code.h>

#include <camera_custom_af_nvram.h> /* For pd_param.h*/

#include <af_param.h>
#include <af_assist_param.h>

using namespace std;

typedef enum
{
    LASER_STATUS_RANGING_VALID             = 0x0,  // reference laser ranging distance
    LASER_STATUS_MOVE_DMAX                 = 0x1,  // Search range [DMAX  : infinity]
    LASER_STATUS_MOVE_MAX_RANGING_DIST     = 0x2,  // Search range [xx cm : infinity], according to the laser max ranging distance
    LASER_STATUS_NOT_REFERENCE             = 0x3
} LASER_STATUS_T;

/***************************************************
 *
 ***************************************************/
typedef struct
{
    unsigned long MajorVersion;
    unsigned long MinorVersion;

} SPDLibVersion_t;

/***************************************************
 * PDO stride should be 16 Byte align
 ***************************************************/
#define _PDO_STRIDE_ALIGN_(W) ({\
        MUINT32 _align = 0;\
        if(((W) % 16) != 0){\
            _align = ((W + 15)/16 * 16);\
        }\
        else{\
            _align = W;\
        }\
        _align;\
    })

/***************************************************
 * pd buffer manager type
 ***************************************************/
// category
#define MSK_CATEGORY_OPEN    0x10
#define MSK_CATEGORY_DUALPD  0x20
#define MSK_CATEGORY_QPD     0x40
#define MSK_CATEGORY         0xf0
// type
#define MSK_PDBUF_TYPE_VC          0x01
#define MSK_PDBUF_TYPE_RAW_LEGACY  0x02
#define MSK_PDBUF_TYPE_PDO         0x04
#define MSK_PDBUF_TYPE             0x0f
//
typedef enum
{
    EPDBUF_NOTDEF           = 0x00,
    EPDBUF_VC               = 0x01,
    EPDBUF_VC_OPEN          = 0x11,
    EPDBUF_RAW_LEGACY       = 0x02,
    EPDBUF_RAW_LEGACY_OPEN  = 0x12,
    EPDBUF_PDO              = 0x04,
    EPDBUF_PDO_OPEN         = 0x14,
    EPDBUF_DUALPD_VC        = 0x21,
    EPDBUF_DUALPD_RAW       = 0x24,
    EPDBUF_QPD_VC           = 0x41,

} EPDBUF_TYPE_t;

#define MAX_SIZE_OF_PD_SENSOR_LIST (32)
#define MAX_SIZE_OF_PD_ROI  (AF_PSUBWIN_NUM)

MUINT32 GetPDBuf_Type( unsigned int a_u4CurrSensorDev, unsigned int a_u4CurrSensorId);

/***************************************************
 * calcuation result
 ***************************************************/
typedef enum
{
    AF_ASSIST_TYPE_CV = 0,
    AF_ASSIST_TYPE_AI,
    AF_ASSIST_TYPE_NUM
} AF_ASSIST_TYPE_T;

typedef struct AF_ASSIST_RESULT_t
{
    unsigned int     CurLensPos;
    unsigned int     DesLensPos;
    unsigned char    Confidence;
    unsigned long    ConfidenceLevel;
    signed long      PhaseDifference;
    int              SaturationPercent;
    af_PdBlockRoi    sROIInfo;
    int              confidenceExt;
} AF_ASSIST_RESULT_T;

//
typedef struct AF_ASSIST_RESULT_PACKAGE_t
{
    MBOOL              updated;
    MUINT32            algoType;
    MUINT32            magicNumber;
    MUINT32            numROIs;
    AF_ASSIST_RESULT_T ROIRes[MAX_SIZE_OF_PD_ROI];
} AF_ASSIST_RESULT_PACKAGE_T;


#define AF_ASSIST_RESOURCE_BUF_SIZE 2
typedef struct AF_ASSIST_RESOURCE_t
{
    AF_ASSIST_RESULT_PACKAGE_T algo_result[AF_ASSIST_TYPE_NUM];
} AF_ASSIST_RESOURCE_T;

/***************************************************
 * Data structer of input argument for configuring isp_mgr_pdo.
 ***************************************************/
typedef struct SPDOHWINFO_t
{
    // pdo
    MUINT32  u4Pdo_xsz;
    MUINT32  u4Pdo_ysz;
    MUINT32  u4BitDepth;
    MUINT8   u1IsDualPD;
    // for shield pd
    MUINT32  u4Bpci_xsz;
    MUINT32  u4Bpci_ysz;
    MUINTPTR phyAddrBpci_tbl;
    MUINTPTR virAddrBpci_tbl;
    MINT32   i4memID;
    size_t   planeOffset;
    void*    imageBuffer;
    //for dualpd
    MUINT32  u4BinRatioX;
    MUINT32  u4BinRatioY;
    MUINT8   u1PBinType; /* [0]1x4, [1]4x4 */
    MUINT8   u1PBinStartLine; /* Can be [0], [1], [2], [3] */
    MUINT8   u1PdSeparateMode;

    SPDOHWINFO_t()
    {
        u4Pdo_xsz       = 0;
        u4Pdo_ysz       = 0;
        u4BitDepth      = 0;
        u1IsDualPD      = 0;
        u4Bpci_xsz      = 0;
        u4Bpci_ysz      = 0;
        phyAddrBpci_tbl = 0;
        virAddrBpci_tbl = 0;
        i4memID         = 0;
        planeOffset     = 0;
        imageBuffer     = NULL;
        u4BinRatioX     = 0;
        u4BinRatioY     = 0;
        u1PBinType      = 0;
        u1PBinStartLine = 0;
        u1PdSeparateMode = 0;
    }
    SPDOHWINFO_t operator =(const SPDOHWINFO_t &in)
    {
        u4Pdo_xsz       = in.u4Pdo_xsz;
        u4Pdo_ysz       = in.u4Pdo_ysz;
        u4BitDepth      = in.u4BitDepth;
        u1IsDualPD      = in.u1IsDualPD;
        u4Bpci_xsz      = in.u4Bpci_xsz;
        u4Bpci_ysz      = in.u4Bpci_ysz;
        phyAddrBpci_tbl = in.phyAddrBpci_tbl;
        virAddrBpci_tbl = in.virAddrBpci_tbl;
        i4memID         = in.i4memID;
        planeOffset     = in.planeOffset;
        imageBuffer     = in.imageBuffer;
        u4BinRatioX     = in.u4BinRatioX;
        u4BinRatioY     = in.u4BinRatioY;
        u1PBinType      = in.u1PBinType;
        u1PBinStartLine = in.u1PBinStartLine;
        u1PdSeparateMode = in.u1PdSeparateMode;
        return (*this);
    }

} SPDOHWINFO_T;

/***************************************************
 * Argument for PD mannager i/o.
 ***************************************************/
typedef struct
{
    EPDBUF_TYPE_t BufType;
    MINT32        i4CurrSensorId;
    MUINT32       u4IsZSD;
    MUINT32       uImgXsz;     /* current sensor output image size. */
    MUINT32       uImgYsz;
    MUINT32       uFullXsz;    /* Full size. */
    MUINT32       uFullYsz;
    MUINT32       u4PDAF_support_type;
    MUINT32       u4IsFrontalBinning;
    MBOOL         bEnablePBIN;
    MUINT8        uPdSeparateMode;
    MINT32        i4SensorMode;
    MINT32        AETargetMode;
    MUINT32       u4PDPipeCtrl;
    MBOOL         bSensorModeSupportPD;

    /* Output data from custom's pd_buf_mgr */
    MUINT32       u4IsIONBufferUsed;
    MUINT32       u4LRBufferSize;
} SPDProfile_t;

class PD_CALCULATION_INPUT
{
private :
    //default constructor is not allowed.
    PD_CALCULATION_INPUT() {}

public:
    PD_CALCULATION_INPUT( MUINT8 iNumROI, MUINT32 iBufSz, MUINT32 iBufStride, MUINT8 *iBufAddr) :
        magicNumber(0),
        frmNum(0),
        curLensPos(0),
        XSizeOfImage(0),
        YSizeOfImage(0),
        databuf_size(iBufSz),
        databuf_stride(iBufStride),
        databuf_virtAddr(iBufAddr),
        afeGain(0),
        rawBitDepth(0),
        iso(0),
        isAEStable(0)
    {
        numROI = iNumROI;
        ROI    = new af_PdBlockRoi [numROI];
    }

    ~PD_CALCULATION_INPUT()
    {
        delete []ROI;
        databuf_virtAddr = NULL;
    }

    MUINT32   magicNumber;
    MUINT32   frmNum;
    MUINT32   curLensPos;
    //ROI coordinate
    MUINT16   XSizeOfImage;
    MUINT16   YSizeOfImage;
    //PD analyze ROI
    MUINT8    numROI;
    af_PdBlockRoi *ROI;
    //PD data buffer
    MUINT32   databuf_size;
    MUINT32   databuf_stride;
    MUINT8   *databuf_virtAddr;
    MINT32    afeGain;
    MBOOL     rawBitDepth;
    MINT32    iso;
    MBOOL     isAEStable;

};

class AF_ASSIST_OUTPUT
{
private :
    //default constructor is not allowed.
    AF_ASSIST_OUTPUT() {}

public:
    AF_ASSIST_OUTPUT( MUINT8 iNumROI) :
        frmNum(0),
        algoType(0xFFFF)
    {
        numRes = iNumROI;
        Res = new AF_ASSIST_RESULT_T [numRes];
    }

    ~AF_ASSIST_OUTPUT()
    {
        delete []Res;
    }

    MUINT32             frmNum;
    MUINT32             algoType;
    MUINT8              numRes;
    AF_ASSIST_RESULT_T *Res;

};

/***************************************************
 * For 3rd party pd algorithm flow
 ***************************************************/
#define MAX_QUEUE_SIZE_OF_CONFIGURATION_FOR_3RD_PARTY_LIB 16

#define MAX_NUMBER_OF_PD_ROIS_FOR_3RD_PARTY_LIB 16

typedef struct S_CONFIG_DATA_OPEN_t
{
    typedef struct
    {
        MUINT8           cfgInfo;
        af_PdBlockRoi    sRoiInfo;

    } S_ROI_DATA_OPEN_T;

    /****************
     *  Linking list
     ****************/
    MUINT32 totalsz;
    S_CONFIG_DATA_OPEN_t *prv;
    S_CONFIG_DATA_OPEN_t *nxt;
    S_CONFIG_DATA_OPEN_t *head;
    S_CONFIG_DATA_OPEN_t *tail;

    /****************
     *      Data
     ****************/
    MUINT32 frmNum;
    MUINT32 cfgNum;
    MUINT32 numROIs;
    S_ROI_DATA_OPEN_T ROI[MAX_NUMBER_OF_PD_ROIS_FOR_3RD_PARTY_LIB];

    S_CONFIG_DATA_OPEN_t() :
        totalsz(0),
        prv(NULL),
        nxt(NULL),
        head(NULL),
        tail(NULL),
        frmNum(0),
        cfgNum(0),
        numROIs(0) {}

} S_CONFIG_DATA_OPEN_T;


typedef struct
{
    MUINT32          curLensPos;
    MUINT16          XSizeOfImage;
    MUINT16          YSizeOfImage;
    af_PdBlockRoi    ROI;
    MUINT8           cfgInfo;
    MINT32           afegain;
    MINT32           mode;

} SPDROIInput_T;


/***************************************************
 *
 ***************************************************/
typedef struct SDUALPDVCINFO_t
{
    MUINT32 u4VCBinningX;
    MUINT32 u4VCBinningY;
    MUINT32 u4VCBufFmt;

    SDUALPDVCINFO_t() :
        u4VCBinningX(0),
        u4VCBinningY(0),
        u4VCBufFmt(0)
    {}

    SDUALPDVCINFO_t operator =(const SDUALPDVCINFO_t &in)
    {
        u4VCBinningX = in.u4VCBinningX;
        u4VCBinningY = in.u4VCBinningY;
        u4VCBufFmt   = in.u4VCBufFmt;
        return (*this);
    }
} SDUALPDVCINFO_T;

// for dual PD
typedef enum
{
    DualPD_VCBuf_Normal = 0,
    DualPD_VCBuf_zHDR,
    DualPD_VCBuf_Type_Num
} DUALPD_VCBUF_TYPE_T;

typedef struct
{
    unsigned int i4BinningX;
    unsigned int i4BinningY;
    unsigned int i4BufFmt;
} DUALPD_VC_SETTING_T;

#endif // _AF_ASSIST_COMMON_H_
