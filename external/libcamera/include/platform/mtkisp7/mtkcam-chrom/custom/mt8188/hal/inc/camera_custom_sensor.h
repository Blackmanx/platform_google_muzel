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


#ifndef _CAMERA_CUSTOM_SENSOR_H_
#define _CAMERA_CUSTOM_SENSOR_H_

#include "camera_custom_types.h"
#include "camera_custom_nvram_pub.h"
#include <camera_custom_isp_nvram_pub.h>

#define MTK000_SENSOR_ID  (0x00000000)
#define SENSOR_DRVNAME_MTK000_MIPI_RAW "mtk000_mipi_raw"

typedef MUINT32 (*fptrDefault)(CAMERA_DATA_TYPE_ENUM const CameraDataType, MVOID*const pDataBuf, MUINT32 const size);
typedef MUINT32 (*fptrFlicker)(MINT32 sensorMode, MINT32 binRatio, MVOID*const pDataBuf);

namespace NSFeature
{


struct FeatureInfoProvider;
class SensorInfoBase
{
public:     ////            Feature Type.
                            typedef enum
                            {
                                EType_RAW =   0,  //  RAW Sensor
                                EType_YUV,        //  YUV Sensor
                            }   EType;

                            typedef NSFeature::FeatureInfoProvider FeatureInfoProvider_T;

public:
    virtual                 ~SensorInfoBase(){}

public:     ////            Interface.
    virtual MBOOL           GetFeatureProvider(FeatureInfoProvider_T& rFInfoProvider) { return false; }
    virtual EType           GetType() const                                 = 0;
    virtual MUINT32         GetID()   const                                 = 0;
    virtual char const*     getDrvName() const                              = 0;
    virtual char const*     getDrvMacroName() const                         = 0;
};


template <SensorInfoBase::EType _sensor_type, MUINT32 _sensor_id, MUINT32 _module_index>
class SensorInfo : public SensorInfoBase
{
public:     ////
    typedef SensorInfo<_sensor_type, _sensor_id, _module_index>    SensorInfo_T;
public:     ////            Interface.
    virtual EType           GetType() const { return _sensor_type; }
    virtual MUINT32         GetID()   const { return _sensor_id;   }
                            //
    virtual char const*     getDrvName() const      { return mpszDrvName; }
    virtual char const*     getDrvMacroName() const { return mpszDrvMacroName; }
public:     ////            Implementation.
                            SensorInfo()
                                : mpszDrvName(0), mpszDrvMacroName(0)
                            {
                            }
                            ~SensorInfo() {}
protected:  ////            Data Members.
    char const*             mpszDrvName;
    char const*             mpszDrvMacroName;
};


template <MUINT32 _sensor_id, MUINT32 _module_index>
class YUVSensorInfo : public SensorInfo<SensorInfoBase::EType_YUV, _sensor_id, _module_index>
{
    typedef YUVSensorInfo<_sensor_id, _module_index>   SensorInfo_T;
public:     ////            Interface.
    static  SensorInfo_T*   createInstance(char const* pszDrvName = "", char const* pszDrvMacroName = "")
                            {
                                getInstance()->mpszDrvName      = pszDrvName;;
                                getInstance()->mpszDrvMacroName = pszDrvMacroName;
                                return  getInstance();
                            }
    static  SensorInfo_T*   getInstance() { static SensorInfo_T inst; return &inst; }
    static  MUINT32         getDefaultData(CAMERA_DATA_TYPE_ENUM const CameraDataType, MVOID*const pDataBuf, MUINT32 const size)
                            {
                                return  -1;
                            }
                            //
    typedef SensorInfoBase::FeatureInfoProvider_T FeatureInfoProvider_T;
    virtual MBOOL           GetFeatureProvider(FeatureInfoProvider_T& rFInfoProvider) { return false; }

    /*static  MUINT32			getNullFlickerPara(MINT32 sensorMode, MVOID*const pDataBuf)
    						{
    							return  -1;
    						}*/
    static  MUINT32			getNullFlickerPara(MINT32 /*sensorMode*/, MINT32 /*binRatio*/, MVOID* /*const pDataBuf*/)
    						{
    							return  -1;
    						}
protected:  ////            Implementation.
    virtual MUINT32         impGetDefaultData(CAMERA_DATA_TYPE_ENUM const CameraDataType, MVOID*const pDataBuf, MUINT32 const size) const { return  -1; }
    static  SensorInfoBase* GetInstance();
};


template <MUINT32 _sensor_id, MUINT32 _module_index>
class RAWSensorInfo : public SensorInfo<SensorInfoBase::EType_RAW, _sensor_id, _module_index>
{
    typedef RAWSensorInfo<_sensor_id, _module_index>   SensorInfo_T;
public:     ////            Interface.
    static  SensorInfo_T*   createInstance(char const* pszDrvName = "", char const* pszDrvMacroName = "")
                            {
                                getInstance()->mpszDrvName      = pszDrvName;;
                                getInstance()->mpszDrvMacroName = pszDrvMacroName;
                                return  getInstance();
                            }
    static  SensorInfo_T*   getInstance() { static SensorInfo_T inst; return &inst; }
    static  MUINT32         getDefaultData(CAMERA_DATA_TYPE_ENUM const CameraDataType, MVOID*const pDataBuf, MUINT32 const size)
                            {
                                return  getInstance()->impGetDefaultData(CameraDataType, pDataBuf, size);
                            }

	static  MUINT32         getFlickerPara(MINT32 sensorMode, MINT32 binRatio, MVOID*const pDataBuf)
                            {
                                return  getInstance()->impGetFlickerPara(sensorMode, binRatio, pDataBuf);
                            }
protected:  ////            Implementation.
    virtual MUINT32         impGetDefaultData(CAMERA_DATA_TYPE_ENUM const CameraDataType, MVOID*const pDataBuf, MUINT32 const size) const;
    virtual MUINT32         impGetFlickerPara(MINT32 sensorMode, MINT32 binRatio, MVOID*const pDataBuf) const;

};

template <MUINT32 _sensor_id, MUINT32 _module_index>
class AECustomTransform : public SensorInfo<SensorInfoBase::EType_RAW, _sensor_id, _module_index>
{
    typedef AECustomTransform<_sensor_id, _module_index>   SensorInfo_T;
public:     ////            Interface.
    static  SensorInfo_T*   createInstance(char const* pszDrvName = "", char const* pszDrvMacroName = "")
                            {
                                getInstance()->mpszDrvName      = pszDrvName;;
                                getInstance()->mpszDrvMacroName = pszDrvMacroName;
                                return  getInstance();
                            }
    static  SensorInfo_T*   getInstance() { static SensorInfo_T inst; return &inst; }
    static  MUINT32         getAECustomTransform(AE_CUSTOM_TRANSFORM_ENUM const AECusFuncType, MVOID*const pData)
                            {
                                return  getInstance()->impAECustomTransform(AECusFuncType, pData);
                            }

protected:  ////            Implementation.
    virtual MUINT32         impAECustomTransform(AE_CUSTOM_TRANSFORM_ENUM const AECusFuncType, MVOID*const pData) const;
};

};  //  NSFeature


typedef struct
{
    MUINT32 sensorType;
    MUINT32 SensorId;
    MUINT32 moduleIndex;
    MUINT64 moduleId;
    MUINT8  drvname[32];
    MUINT32 (*getAECustomTransform)(AE_CUSTOM_TRANSFORM_ENUM AECusFuncType, MVOID *pData);
    MUINT32 (*getCameraCalData)(UINT32* pGetSensorCalData);
} MSDK_SENSOR_INIT_FUNCTION_STRUCT, *PMSDK_SENSOR_INIT_FUNCTION_STRUCT;

MUINT32 GetSensorInitFuncList(MSDK_SENSOR_INIT_FUNCTION_STRUCT **ppSensorList, UINT32 *pSize,
                              MSDK_SENSOR_INIT_FUNCTION_STRUCT **ppSensorListDefault, UINT32 *pSizeDefault);


#endif  //  _CAMERA_CUSTOM_SENSOR_H_
