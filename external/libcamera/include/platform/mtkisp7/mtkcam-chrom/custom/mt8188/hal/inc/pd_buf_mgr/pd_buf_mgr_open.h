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


#ifndef _PD_BUF_MGR_OPEN_H_
#define _PD_BUF_MGR_OPEN_H_

#ifdef MTK_LOG_ENABLE
#undef MTK_LOG_ENABLE
#endif
#define MTK_LOG_ENABLE 1

#include <utils/Log.h>
#include "pd_buf_common.h"


typedef enum
{
    /******************************************************
    *
    *       The input argument's type is define here.
    *
    *            !!! Please MUST follow it !!!
    *
    ******************************************************/

    PDBUFMGR_OPEN_CMD_GET_REG_SETTING_LIST     = 0x00, /* arg1:(MUINT32)    , arg2:(MUINT16 *),                  */
    PDBUFMGR_OPEN_CMD_GET_PD_WIN_REG_SETTING   = 0x01, /* arg1:(PDBUF_CFG_T), arg2:(MUINT32)  , arg3:(MUINT16 *) */
    PDBUFMGR_OPEN_CMD_GET_PD_WIN_MODE_SETTING  = 0x02, /* arg1:(MINT32)     ,                 ,                  */
    PDBUFMGR_OPEN_CMD_GET_CUR_BUF_SETTING_INFO = 0x03, /* arg1:(MINT32)     , arg2:(MUINT32)  ,                  */
    /* Add command before this line */
    PDBUFMGR_OPEN_CMD_NUM

} PDBUFMGR_OPEN_CMD_t;

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  PD buffer manager I/F : using 3rd party pd algo.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
class PDBufMgrOpen
{
private:

protected:
    //DMA output data which is got form host.
    MUINT8   *m_databuf;
    MUINT32   m_databuf_size;
    //Calibration data which is got form host.
    MUINT8   *m_calidatabuf;
    MUINT32   m_calidatabuf_size;

    //extract phase differenct data from m_databuf
    MUINT16  *m_phase_difference;
    //extract confidence level data from m_databuf
    MUINT16  *m_confidence_level;
    //extract calibration data from m_calidatabuf (convert format)
    MUINT16  *m_calibration_data;

    //for sync information
    MUINT32 m_frm_num;


    /**
    * @brief checking current sensor is supported or not.
    */
    virtual MBOOL IsSupport( SPDProfile_t &iPdProfile) = 0;

    /**
    * @brief get phase differnece data and confidence level data from data buffer.
    */
    virtual MBOOL ExtractPDCL() = 0;

    /**
    * @brief get calibration data from calibration data buffer.
    */
    virtual MBOOL ExtractCaliData() = 0;



public:
    PDBufMgrOpen();
    virtual ~PDBufMgrOpen();

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//                                     Interface
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    /**
    * @brief create instance
    */
    static PDBufMgrOpen* createInstance( SPDProfile_t &iPdProfile);


    /* Input */
    /**
    * @brief input data buffer which is got  from host.
    */
    MBOOL SetDataBuf( MUINT32  i4Size, MUINT8 *ptrBuf, MUINT32 &i4FrmCnt);
    /**
    * @brief input calibration data which is got  from host.
    */
    MBOOL SetCalibrationData( MUINT32  i4Size, MUINT8 *ptrcaldata);
    /**
    * @brief send command.
    */
    virtual MBOOL sendCommand( MUINT32  i4Cmd, MVOID* arg1=NULL, MVOID* arg2=NULL, MVOID* arg3=NULL, MVOID* arg4=NULL);


    /* Output */
    /**
    * @brief get PD calibration data size.
    */
    virtual MINT32 GetPDCalSz() = 0;
    /**
    * @brief PD information for hybrid af
    */
    virtual MBOOL GetPDInfo2HybridAF( MINT32 i4InArySz, MINT32 *i4OutAry) = 0;
    /**
    * @brief output 3rd party pd algorithm version.
    */
    virtual MRESULT GetVersionOfPdafLibrary( SPDLibVersion_t &tOutSWVer) = 0;
    /**
    * @brief output pd algorighm result.
    */
    virtual MBOOL GetDefocus( SPDROIInput_T &iPDInputData, AF_ASSIST_RESULT_T &oPdOutputData) = 0;
    /**
    * @brief output PDO information
    */
    virtual MBOOL GetPDOHWInfo( MINT32 i4CurSensorMode, SPDOHWINFO_T &oPDOHWInfo);

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

};

#endif // _PD_BUF_MGR_OPEN_H_
