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

#ifndef _CAM_IDX_STRUCT_QUERY_H_
#define _CAM_IDX_STRUCT_QUERY_H_

#include <string>
#include <map>
#include <tuple>
#include <vector>
#include <tuning_mapping/cam_idx_struct_mapping_pub.h>

namespace NSIspTuning{

static const std::vector<std::tuple<EDim_T, std::string>> down_scale_cmd = {
    {EDim_Project, "SELECT db_ProjectID, db_ProjectOrder FROM db_Project"},
    {EDim_Action, "SELECT db_ActionID, db_ActionOrder FROM db_Action"},
    {EDim_Standard, "SELECT db_StandardID, db_StandardOrder FROM db_Standard"},
    {EDim_SensorMode, "SELECT db_SensorModeID, db_SensorModeOrder FROM db_SensorMode"},
    {EDim_Flash, "SELECT db_FlashID, db_FlashOrder FROM db_Flash"},
    {EDim_YUVSize, "SELECT db_YUVSizeID, db_YUVSizeOrder FROM db_YUVSize"},
    {EDim_Zoom, "SELECT db_ZoomID, db_ZoomOrder FROM db_Zoom"},
    {EDim_App, "SELECT db_AppID, db_AppOrder FROM db_App"},
    {EDim_SensorFeature, "SELECT db_SensorFeatureID, db_SensorFeatureOrder FROM db_SensorFeature"},
    {EDim_CustomFeature, "SELECT db_CustomFeatureID, db_CustomFeatureOrder FROM db_CustomFeature"},
    {EDim_Feature, "SELECT db_FeatureID, db_FeatureOrder FROM db_Feature"},
    {EDim_Custom, "SELECT db_CustomID, db_CustomOrder FROM db_Custom"},
    {EDim_ToneGain, "SELECT key_ToneGainID, key_ToneGainOrder FROM key_ToneGain"},
    {EDim_DR, "SELECT key_DRID, key_DROrder FROM key_DR"},
    {EDim_Tripod, "SELECT key_TripodID, key_TripodOrder FROM key_Tripod"},
    {EDim_Stage, "SELECT key_StageID, key_StageOrder FROM key_Stage"},
    {EDim_FaceDetection, "SELECT key_FaceDetectionID, key_FaceDetectionOrder FROM key_FaceDetection"},
    {EDim_Ratio, "SELECT key_RatioID, key_RatioOrder FROM key_Ratio"},
    {EDim_LV, "SELECT key_LVID, key_LVOrder FROM key_LV"},
    {EDim_CT, "SELECT key_CTID, key_CTOrder FROM key_CT"},
    {EDim_ISO, "SELECT key_ISOID, key_ISOOrder FROM key_ISO"},
};

typedef struct DOWN_SCALE_TBL_STRUCTURE
{
    std::map<int32_t, int32_t> project_down_scale_tbl;
    std::map<int32_t, int32_t> action_down_scale_tbl;
    std::map<int32_t, int32_t> standard_down_scale_tbl;
    std::map<int32_t, int32_t> sensormode_down_scale_tbl;
    std::map<int32_t, int32_t> flash_down_scale_tbl;
    std::map<int32_t, int32_t> yuvsize_down_scale_tbl;
    std::map<int32_t, int32_t> zoom_down_scale_tbl;
    std::map<int32_t, int32_t> app_down_scale_tbl;
    std::map<int32_t, int32_t> sensorfeature_down_scale_tbl;
    std::map<int32_t, int32_t> customfeature_down_scale_tbl;
    std::map<int32_t, int32_t> feature_down_scale_tbl;
    std::map<int32_t, int32_t> custom_down_scale_tbl;
    std::map<int32_t, int32_t> tonegain_down_scale_tbl;
    std::map<int32_t, int32_t> dr_down_scale_tbl;
    std::map<int32_t, int32_t> tripod_down_scale_tbl;
    std::map<int32_t, int32_t> stage_down_scale_tbl;
    std::map<int32_t, int32_t> facedetection_down_scale_tbl;
    std::map<int32_t, int32_t> ratio_down_scale_tbl;
    std::map<int32_t, int32_t> lv_down_scale_tbl;
    std::map<int32_t, int32_t> ct_down_scale_tbl;
    std::map<int32_t, int32_t> iso_down_scale_tbl;

    void insert(const EDim_T dim, const std::map<int32_t, int32_t> down_scale_tbl) {
        if (dim == EDim_Project) {
            project_down_scale_tbl = down_scale_tbl;
        }

        if (dim == EDim_Action) {
            action_down_scale_tbl = down_scale_tbl;
        }

        if (dim == EDim_Standard) {
            standard_down_scale_tbl = down_scale_tbl;
        }

        if (dim == EDim_SensorMode) {
            sensormode_down_scale_tbl = down_scale_tbl;
        }

        if (dim == EDim_Flash) {
            flash_down_scale_tbl = down_scale_tbl;
        }

        if (dim == EDim_YUVSize) {
            yuvsize_down_scale_tbl = down_scale_tbl;
        }

        if (dim == EDim_Zoom) {
            zoom_down_scale_tbl = down_scale_tbl;
        }

        if (dim == EDim_App) {
            app_down_scale_tbl = down_scale_tbl;
        }

        if (dim == EDim_SensorFeature) {
            sensorfeature_down_scale_tbl = down_scale_tbl;
        }

        if (dim == EDim_CustomFeature) {
            customfeature_down_scale_tbl = down_scale_tbl;
        }

        if (dim == EDim_Feature) {
            feature_down_scale_tbl = down_scale_tbl;
        }

        if (dim == EDim_Custom) {
            custom_down_scale_tbl = down_scale_tbl;
        }

        if (dim == EDim_ToneGain) {
            tonegain_down_scale_tbl = down_scale_tbl;
        }

        if (dim == EDim_DR) {
            dr_down_scale_tbl = down_scale_tbl;
        }

        if (dim == EDim_Tripod) {
            tripod_down_scale_tbl = down_scale_tbl;
        }

        if (dim == EDim_Stage) {
            stage_down_scale_tbl = down_scale_tbl;
        }

        if (dim == EDim_FaceDetection) {
            facedetection_down_scale_tbl = down_scale_tbl;
        }

        if (dim == EDim_Ratio) {
            ratio_down_scale_tbl = down_scale_tbl;
        }

        if (dim == EDim_LV) {
            lv_down_scale_tbl = down_scale_tbl;
        }

        if (dim == EDim_CT) {
            ct_down_scale_tbl = down_scale_tbl;
        }

        if (dim == EDim_ISO) {
            iso_down_scale_tbl = down_scale_tbl;
        }

    }

} DOWN_SCALE_TBL;

typedef union _TUNING_MAPPING_INFO
{
    struct {
        EProject_T eProject;
        EAction_T eAction;
        EStandard_T eStandard;
        ESensorMode_T eSensorMode;
        EFlash_T eFlash;
        EYUVSize_T eYUVSize;
        EZoom_T eZoom;
        EApp_T eApp;
        ESensorFeature_T eSensorFeature;
        ECustomFeature_T eCustomFeature;
        EFeature_T eFeature;
        ECustom_T eCustom;
        EToneGain_T eToneGain;
        EDR_T eDR;
        ETripod_T eTripod;
        EStage_T eStage;
        EFaceDetection_T eFaceDetection;
        ERatio_T eRatio;
        ELV_T eLV;
        ECT_T eCT;
        EISO_T eISO[NVRAM_ISP_REGS_ISO_GROUP_NUM];
    };
    MUINT32 query[EDim_NUM + NVRAM_ISP_REGS_ISO_GROUP_NUM - 1];

    _TUNING_MAPPING_INFO(const CAM_IDX_QRY_COMB_ISP7& qry) {
      memset(query, 0, sizeof(query));
        eProject = qry.eProject;
        eAction = qry.eAction;
        eStandard = qry.eStandard;
        eSensorMode = qry.eSensorMode;
        eFlash = qry.eFlash;
        eYUVSize = qry.eYUVSize;
        eZoom = qry.eZoom;
        eApp = qry.eApp;
        eSensorFeature = qry.eSensorFeature;
        eCustomFeature = qry.eCustomFeature;
        eFeature = qry.eFeature;
        eCustom = qry.eCustom;
        eToneGain = qry.eToneGain;
        eDR = qry.eDR;
        eTripod = qry.eTripod;
        eStage = qry.eStage;
        eFaceDetection = qry.eFaceDetection;
        eRatio = qry.eRatio;
        eLV = qry.eLV;
        eCT = qry.eCT;
        for (int i = 0; i < NVRAM_ISP_REGS_ISO_GROUP_NUM; i++)
            eISO[i] = qry.eISO[i];
    }

     _TUNING_MAPPING_INFO() {
      memset(query, 0, sizeof(query));
    }

    bool operator==(const _TUNING_MAPPING_INFO& other) const {
      return std::equal(std::begin(query), std::end(query), std::begin(other.query));
    }

    void reset_key_with_down_scale_tbl(DOWN_SCALE_TBL all_down_scale_tbl) {
        eProject = static_cast<EProject_T>(all_down_scale_tbl.project_down_scale_tbl[eProject]);
        eAction = static_cast<EAction_T>(all_down_scale_tbl.action_down_scale_tbl[eAction]);
        eStandard = static_cast<EStandard_T>(all_down_scale_tbl.standard_down_scale_tbl[eStandard]);
        eSensorMode = static_cast<ESensorMode_T>(all_down_scale_tbl.sensormode_down_scale_tbl[eSensorMode]);
        eFlash = static_cast<EFlash_T>(all_down_scale_tbl.flash_down_scale_tbl[eFlash]);
        eYUVSize = static_cast<EYUVSize_T>(all_down_scale_tbl.yuvsize_down_scale_tbl[eYUVSize]);
        eZoom = static_cast<EZoom_T>(all_down_scale_tbl.zoom_down_scale_tbl[eZoom]);
        eApp = static_cast<EApp_T>(all_down_scale_tbl.app_down_scale_tbl[eApp]);
        eSensorFeature = static_cast<ESensorFeature_T>(all_down_scale_tbl.sensorfeature_down_scale_tbl[eSensorFeature]);
        eCustomFeature = static_cast<ECustomFeature_T>(all_down_scale_tbl.customfeature_down_scale_tbl[eCustomFeature]);
        eFeature = static_cast<EFeature_T>(all_down_scale_tbl.feature_down_scale_tbl[eFeature]);
        eCustom = static_cast<ECustom_T>(all_down_scale_tbl.custom_down_scale_tbl[eCustom]);
        eToneGain = static_cast<EToneGain_T>(all_down_scale_tbl.tonegain_down_scale_tbl[eToneGain]);
        eDR = static_cast<EDR_T>(all_down_scale_tbl.dr_down_scale_tbl[eDR]);
        eTripod = static_cast<ETripod_T>(all_down_scale_tbl.tripod_down_scale_tbl[eTripod]);
        eStage = static_cast<EStage_T>(all_down_scale_tbl.stage_down_scale_tbl[eStage]);
        eFaceDetection = static_cast<EFaceDetection_T>(all_down_scale_tbl.facedetection_down_scale_tbl[eFaceDetection]);
        eRatio = static_cast<ERatio_T>(all_down_scale_tbl.ratio_down_scale_tbl[eRatio]);
        eLV = static_cast<ELV_T>(all_down_scale_tbl.lv_down_scale_tbl[eLV]);
        eCT = static_cast<ECT_T>(all_down_scale_tbl.ct_down_scale_tbl[eCT]);
        for (int i = 0; i < NVRAM_ISP_REGS_ISO_GROUP_NUM; i++)
            eISO[i] = static_cast<EISO_T>(all_down_scale_tbl.iso_down_scale_tbl[eISO[i]]);
    }

    std::string to_string() const {
        std::string out = "";
        out += " Project(" + std::to_string(eProject) + ")" + "(" + Project_str[(int)eProject] + ")";
        out += " Action(" + std::to_string(eAction) + ")" + "(" + Action_str[(int)eAction] + ")";
        out += " Standard(" + std::to_string(eStandard) + ")" + "(" + Standard_str[(int)eStandard] + ")";
        out += " SensorMode(" + std::to_string(eSensorMode) + ")" + "(" + SensorMode_str[(int)eSensorMode] + ")";
        out += " Flash(" + std::to_string(eFlash) + ")" + "(" + Flash_str[(int)eFlash] + ")";
        out += " YUVSize(" + std::to_string(eYUVSize) + ")" + "(" + YUVSize_str[(int)eYUVSize] + ")";
        out += " Zoom(" + std::to_string(eZoom) + ")" + "(" + Zoom_str[(int)eZoom] + ")";
        out += " App(" + std::to_string(eApp) + ")" + "(" + App_str[(int)eApp] + ")";
        out += " SensorFeature(" + std::to_string(eSensorFeature) + ")" + "(" + SensorFeature_str[(int)eSensorFeature] + ")";
        out += " CustomFeature(" + std::to_string(eCustomFeature) + ")" + "(" + CustomFeature_str[(int)eCustomFeature] + ")";
        out += " Feature(" + std::to_string(eFeature) + ")" + "(" + Feature_str[(int)eFeature] + ")";
        out += " Custom(" + std::to_string(eCustom) + ")" + "(" + Custom_str[(int)eCustom] + ")";
        out += " ToneGain(" + std::to_string(eToneGain) + ")" + "(" + ToneGain_str[(int)eToneGain] + ")";
        out += " DR(" + std::to_string(eDR) + ")" + "(" + DR_str[(int)eDR] + ")";
        out += " Tripod(" + std::to_string(eTripod) + ")" + "(" + Tripod_str[(int)eTripod] + ")";
        out += " Stage(" + std::to_string(eStage) + ")" + "(" + Stage_str[(int)eStage] + ")";
        out += " FaceDetection(" + std::to_string(eFaceDetection) + ")" + "(" + FaceDetection_str[(int)eFaceDetection] + ")";
        out += " Ratio(" + std::to_string(eRatio) + ")" + "(" + Ratio_str[(int)eRatio] + ")";
        out += " LV(" + std::to_string(eLV) + ")" + "(" + LV_str[(int)eLV] + ")";
        out += " CT(" + std::to_string(eCT) + ")" + "(" + CT_str[(int)eCT] + ")";
        out += " ISO(";
        for (int i = 0; i < NVRAM_ISP_REGS_ISO_GROUP_NUM; i++)
            out += std::to_string(eISO[i]) + "(" + ISO_str[(int)eISO[i]] + ")" + ", ";
        out += ")";
        return out;
    }
} TUNING_MAPPING_INFO;

static_assert(21 == EDim_NUM, "Number of dimensions mismatch");

}

typedef union ExpandResult {
    struct {
        uint32_t tonegain;
        uint32_t dr;
        uint32_t tripod;
        uint32_t stage;
        uint32_t facedetection;
        uint32_t ratio;
        uint32_t lv;
        uint32_t ct;
        uint32_t iso;
        uint32_t isValid;
    } info;
    uint32_t idx[10];

    int get_tuning_index(const TUNING_MAPPING_INFO& qry,
                         int idxBase,
                         int stage_for_exp,
                         int isoGroup) const {
        return
            idxBase +
            qry.eToneGain             * info.tonegain +
            qry.eDR                   * info.dr +
            qry.eTripod               * info.tripod +
            stage_for_exp             * info.stage +
            qry.eFaceDetection        * info.facedetection +
            qry.eRatio                * info.ratio +
            qry.eLV                   * info.lv +
            qry.eCT                   * info.ct +
            qry.eISO[isoGroup]        * info.iso;
    }

    std::string to_string() const {
        return
            std::to_string(info.tonegain) + ", " +
            std::to_string(info.dr) + ", " +
            std::to_string(info.tripod) + ", " +
            std::to_string(info.stage) + ", " +
            std::to_string(info.facedetection) + ", " +
            std::to_string(info.ratio) + ", " +
            std::to_string(info.lv) + ", " +
            std::to_string(info.ct) + ", " +
            std::to_string(info.iso) + ", ";
    }

} ExpandResult_T;

typedef struct ALL_ISP_INTERVAL_STRUCTURE {
#define SUPPORT_KEY_TONEGAIN
#define SUPPORT_KEY_DR
#define SUPPORT_KEY_TRIPOD
#define SUPPORT_KEY_STAGE
#define SUPPORT_KEY_FACEDETECTION
#define SUPPORT_KEY_RATIO
#define SUPPORT_KEY_LV
#define SUPPORT_KEY_CT
    std::vector<int> Tonegain_Env;
    std::vector<int> Dr_Env;
    std::vector<int> Tripod_Env;
    std::vector<int> Stage_Env;
    std::vector<int> Facedetection_Env;
    std::vector<int> Ratio_Env;
    std::vector<int> Lv_Env;
    std::vector<int> Ct_Env;
    std::vector<std::vector<int>> Iso_Env;
    std::vector<int> Project_Env;
    std::vector<int> Action_Env;
    std::vector<int> Standard_Env;
    std::vector<int> Sensormode_Env;
    std::vector<int> Flash_Env;
    std::vector<int> Yuvsize_Env;
    std::vector<int> Zoom_Env;
    std::vector<int> App_Env;
    std::vector<int> Sensorfeature_Env;
    std::vector<int> Customfeature_Env;
    std::vector<int> Feature_Env;
    std::vector<int> Custom_Env;

    void set_env_by_map_without_iso(std::map<std::string, std::vector<int>>& anchor_map) {

        if(anchor_map.find("ToneGain") != anchor_map.end()) {
            Tonegain_Env = anchor_map["ToneGain"];
        }

        if(anchor_map.find("DR") != anchor_map.end()) {
            Dr_Env = anchor_map["DR"];
        }

        if(anchor_map.find("Tripod") != anchor_map.end()) {
            Tripod_Env = anchor_map["Tripod"];
        }

        if(anchor_map.find("Stage") != anchor_map.end()) {
            Stage_Env = anchor_map["Stage"];
        }

        if(anchor_map.find("FaceDetection") != anchor_map.end()) {
            Facedetection_Env = anchor_map["FaceDetection"];
        }

        if(anchor_map.find("Ratio") != anchor_map.end()) {
            Ratio_Env = anchor_map["Ratio"];
        }

        if(anchor_map.find("LV") != anchor_map.end()) {
            Lv_Env = anchor_map["LV"];
        }

        if(anchor_map.find("CT") != anchor_map.end()) {
            Ct_Env = anchor_map["CT"];
        }

        if(anchor_map.find("Project") != anchor_map.end()) {
            Project_Env = anchor_map["Project"];
        }

        if(anchor_map.find("Action") != anchor_map.end()) {
            Action_Env = anchor_map["Action"];
        }

        if(anchor_map.find("Standard") != anchor_map.end()) {
            Standard_Env = anchor_map["Standard"];
        }

        if(anchor_map.find("SensorMode") != anchor_map.end()) {
            Sensormode_Env = anchor_map["SensorMode"];
        }

        if(anchor_map.find("Flash") != anchor_map.end()) {
            Flash_Env = anchor_map["Flash"];
        }

        if(anchor_map.find("YUVSize") != anchor_map.end()) {
            Yuvsize_Env = anchor_map["YUVSize"];
        }

        if(anchor_map.find("Zoom") != anchor_map.end()) {
            Zoom_Env = anchor_map["Zoom"];
        }

        if(anchor_map.find("App") != anchor_map.end()) {
            App_Env = anchor_map["App"];
        }

        if(anchor_map.find("SensorFeature") != anchor_map.end()) {
            Sensorfeature_Env = anchor_map["SensorFeature"];
        }

        if(anchor_map.find("CustomFeature") != anchor_map.end()) {
            Customfeature_Env = anchor_map["CustomFeature"];
        }

        if(anchor_map.find("Feature") != anchor_map.end()) {
            Feature_Env = anchor_map["Feature"];
        }

        if(anchor_map.find("Custom") != anchor_map.end()) {
            Custom_Env = anchor_map["Custom"];
        }

    }

    std::string to_string_without_iso() {
        std::string out = "";

        out += "Tonegain_Env size(" + std::to_string(Tonegain_Env.size()) + ") [";
        for (int i = 0; i < (int)Tonegain_Env.size(); ++i) {
            out += std::to_string(Tonegain_Env[i]) + " ";
        }
        out += "] ";

        out += "Dr_Env size(" + std::to_string(Dr_Env.size()) + ") [";
        for (int i = 0; i < (int)Dr_Env.size(); ++i) {
            out += std::to_string(Dr_Env[i]) + " ";
        }
        out += "] ";

        out += "Tripod_Env size(" + std::to_string(Tripod_Env.size()) + ") [";
        for (int i = 0; i < (int)Tripod_Env.size(); ++i) {
            out += std::to_string(Tripod_Env[i]) + " ";
        }
        out += "] ";

        out += "Stage_Env size(" + std::to_string(Stage_Env.size()) + ") [";
        for (int i = 0; i < (int)Stage_Env.size(); ++i) {
            out += std::to_string(Stage_Env[i]) + " ";
        }
        out += "] ";

        out += "Facedetection_Env size(" + std::to_string(Facedetection_Env.size()) + ") [";
        for (int i = 0; i < (int)Facedetection_Env.size(); ++i) {
            out += std::to_string(Facedetection_Env[i]) + " ";
        }
        out += "] ";

        out += "Ratio_Env size(" + std::to_string(Ratio_Env.size()) + ") [";
        for (int i = 0; i < (int)Ratio_Env.size(); ++i) {
            out += std::to_string(Ratio_Env[i]) + " ";
        }
        out += "] ";

        out += "Lv_Env size(" + std::to_string(Lv_Env.size()) + ") [";
        for (int i = 0; i < (int)Lv_Env.size(); ++i) {
            out += std::to_string(Lv_Env[i]) + " ";
        }
        out += "] ";

        out += "Ct_Env size(" + std::to_string(Ct_Env.size()) + ") [";
        for (int i = 0; i < (int)Ct_Env.size(); ++i) {
            out += std::to_string(Ct_Env[i]) + " ";
        }
        out += "] ";

        out += "Project_Env size(" + std::to_string(Project_Env.size()) + ") [";
        for (int i = 0; i < (int)Project_Env.size(); ++i) {
            out += std::to_string(Project_Env[i]) + " ";
        }
        out += "] ";

        out += "Action_Env size(" + std::to_string(Action_Env.size()) + ") [";
        for (int i = 0; i < (int)Action_Env.size(); ++i) {
            out += std::to_string(Action_Env[i]) + " ";
        }
        out += "] ";

        out += "Standard_Env size(" + std::to_string(Standard_Env.size()) + ") [";
        for (int i = 0; i < (int)Standard_Env.size(); ++i) {
            out += std::to_string(Standard_Env[i]) + " ";
        }
        out += "] ";

        out += "Sensormode_Env size(" + std::to_string(Sensormode_Env.size()) + ") [";
        for (int i = 0; i < (int)Sensormode_Env.size(); ++i) {
            out += std::to_string(Sensormode_Env[i]) + " ";
        }
        out += "] ";

        out += "Flash_Env size(" + std::to_string(Flash_Env.size()) + ") [";
        for (int i = 0; i < (int)Flash_Env.size(); ++i) {
            out += std::to_string(Flash_Env[i]) + " ";
        }
        out += "] ";

        out += "Yuvsize_Env size(" + std::to_string(Yuvsize_Env.size()) + ") [";
        for (int i = 0; i < (int)Yuvsize_Env.size(); ++i) {
            out += std::to_string(Yuvsize_Env[i]) + " ";
        }
        out += "] ";

        out += "Zoom_Env size(" + std::to_string(Zoom_Env.size()) + ") [";
        for (int i = 0; i < (int)Zoom_Env.size(); ++i) {
            out += std::to_string(Zoom_Env[i]) + " ";
        }
        out += "] ";

        out += "App_Env size(" + std::to_string(App_Env.size()) + ") [";
        for (int i = 0; i < (int)App_Env.size(); ++i) {
            out += std::to_string(App_Env[i]) + " ";
        }
        out += "] ";

        out += "Sensorfeature_Env size(" + std::to_string(Sensorfeature_Env.size()) + ") [";
        for (int i = 0; i < (int)Sensorfeature_Env.size(); ++i) {
            out += std::to_string(Sensorfeature_Env[i]) + " ";
        }
        out += "] ";

        out += "Customfeature_Env size(" + std::to_string(Customfeature_Env.size()) + ") [";
        for (int i = 0; i < (int)Customfeature_Env.size(); ++i) {
            out += std::to_string(Customfeature_Env[i]) + " ";
        }
        out += "] ";

        out += "Feature_Env size(" + std::to_string(Feature_Env.size()) + ") [";
        for (int i = 0; i < (int)Feature_Env.size(); ++i) {
            out += std::to_string(Feature_Env[i]) + " ";
        }
        out += "] ";

        out += "Custom_Env size(" + std::to_string(Custom_Env.size()) + ") [";
        for (int i = 0; i < (int)Custom_Env.size(); ++i) {
            out += std::to_string(Custom_Env[i]) + " ";
        }
        out += "] ";

        return out;
    }

    std::string to_string_for_iso() {
        std::string out = "";

        for (int i = 0; i < (int)Iso_Env.size(); i++) {
            out += "Iso_Env[" + std::to_string(i) + "] size(" + std::to_string(Iso_Env[i].size()) + ") [";
            for (int j = 0; j < (int)Iso_Env[i].size(); j++)
                out += std::to_string(Iso_Env[i][j]) + " ";

            out += "] ";
        }
    return out;
    }

    std::vector<uint64_t> get_expand_interval(const int stage_count) const {
        std::vector<uint64_t> expand_interval;
        expand_interval.reserve(DYNAMIC_KEY_DIM_NUM);

        expand_interval.push_back(Tonegain_Env.size());

        expand_interval.push_back(Dr_Env.size());

        expand_interval.push_back(Tripod_Env.size());

        expand_interval.push_back(stage_count);

        expand_interval.push_back(Facedetection_Env.size());

        expand_interval.push_back(Ratio_Env.size());

        expand_interval.push_back(Lv_Env.size());

        expand_interval.push_back(Ct_Env.size());

        uint64_t max_iso_length = 0;
        for (int i = 0; i < (int)Iso_Env.size(); i++) {
            if (Iso_Env[i].size() > max_iso_length)
                max_iso_length = Iso_Env[i].size();
        }
        expand_interval.push_back(max_iso_length);

        return expand_interval;
    }

} ALL_ISP_INTERVAL;

#endif // _CAM_IDX_STRUCT_QUERY_H_
