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

#ifndef INCLUDE_MTKCAM_CORE_HW_IMGSTREAM_IIMGSTREAMDEF_H_
#define INCLUDE_MTKCAM_CORE_HW_IMGSTREAM_IIMGSTREAMDEF_H_

#include "mtkcam-halif/def/BuiltinTypes.h"
#include "eightcc.h"

#include <cstdint>
#include <list>
#include <memory>
#include <unordered_map>
#include <vector>
/******************************************************************************
 *
 ******************************************************************************/
namespace NSCam {
namespace NSImgStream {

// from ImgMeDef.h
enum ME_SCENARIO { EME_MODE_3PASS = 0, EME_MODE_1PASS, EME_MODE_BATCH };

enum ME_MODE { EME_MODE_0 = 0, EME_MODE_1, EME_MODE_2 };

// from ImgWpeDef.h
struct WPE_CrpInfo {
  MUINT32 x_start_point;
  MUINT32 x_end_point;
  MUINT32 y_start_point;
  MUINT32 y_end_point;
};

struct WPE_SzInfo {
  MUINT32 wd;
  MUINT32 ht;
};

struct WPE_CrpOfstInfo {
  MUINT32 x_start;  // tile align
  MUINT32 hr_int_ofst;
  MUINT32 hr_sub_ofst;
  MUINT32 y_start;  // tile align
  MUINT32 vt_int_ofst;
  MUINT32 vt_sub_ofst;
  MUINT32 wd;
  MUINT32 ht;
};

enum WPE_MODE {
  EWPE_HW_DEFAULT = 0,
  EWPE_MODE_3rdParty = (1UL << 0),
  EWPE_HW_EIS  = (1UL << 1),
  EWPE_HW_TNR  = (1UL << 2),
  EWPE_HW_LITE  = (1UL << 3),
  // isp7, no use
  EWPE_MODE_WPEO,
  EWPE_MODE_MDP,
  EWPE_MODE_ISP,
};

enum RGB_MODE {
  EWPE_RGB_ODD_LINE = 0,
  EWPE_RGB_EVEN_LINE,
  EWPE_TOTAL_RGB_MODE
};

enum EXTRA_FEATURE_INDEX {
  EWPE_NONE = 0,
  EWPE_PSP_BORDER_COLOR = (1UL << 0),  // user defined psp border / isp7.0 only
  EWPE_MVMAP = (1UL << 1),        // map to WPE is mv map
  EWPE_DEBUG_WPEO = (1UL << 2),   // dl, dump as dl content
  EWPE_IROI = (1UL << 3),         // input ROI
  EWPE_VGENIN_OFST = (1UL << 4),  // vgen_in ofst
  EWPE_TOTAL_INDEX
};

enum PSP_TABLE_SEL {
  PSP_TABLE_DEFAULT = 0,  // default value is TABLE 2 before ISP 7
  //
  PSP_TABLE_ISP_1,  // ISP generation based table1
  PSP_TABLE_ISP_2,  // ISP generation based table2
  PSP_TABLE_ISP_3,  // ISP generation based table3
  //
  PSP_TABLE_USER_DEFINE,  // no use
  PSP_TABLE_0,
  PSP_TABLE_1,
  PSP_TABLE_MAX
};

struct WpePspCoef {
  MUINT32 PSP_COEF0;
  MUINT32 PSP_COEF2;
  MUINT32 PSP_COEF4;
  MUINT32 PSP_COEF6;
  MUINT32 PSP_COEF8;
  MUINT32 PSP_COEF10;
  MUINT32 PSP_COEF12;
  MUINT32 PSP_COEF14;
  MUINT32 PSP_COEF16;
  MUINT32 PSP_COEF18;
  MUINT32 PSP_COEF20;
  MUINT32 PSP_COEF22;
  MUINT32 PSP_COEF24;
  MUINT32 PSP_COEF26;
  MUINT32 PSP_COEF28;
  MUINT32 PSP_COEF30;
  MUINT32 PSP_COEF32;
  MUINT32 PSP_COEF34;
  MUINT32 PSP_COEF36;
  MUINT32 PSP_COEF38;
  MUINT32 PSP_COEF40;
  MUINT32 PSP_COEF42;
  MUINT32 PSP_COEF44;
  MUINT32 PSP_COEF46;
  MUINT32 PSP_COEF48;
  MUINT32 PSP_COEF50;
  MUINT32 PSP_COEF52;
  MUINT32 PSP_COEF54;
  MUINT32 PSP_COEF56;
  MUINT32 PSP_COEF58;
  MUINT32 PSP_COEF60;
  MUINT32 PSP_COEF62;
  MUINT32 PSP_COEF64;
};


/******************************************************************************
 *
 * @struct IMG_PRIORITY
 * @brief package usage enum for Img
 * @details
 *
 ******************************************************************************/
enum IMG_PRIORITY {
  IMG_PRIORITY_PREVIEW = -19,  // High Prioirty
  IMG_PRIORITY_RECORD = -18,   // Medium Prioirty
  IMG_PRIORITY_AUX_1 = -17,    // Medium Prioirty
  IMG_PRIORITY_AUX_2 = -16,    // Low Prioirty
  IMG_PRIORITY_DEFAULT = 0,
};

/*******************************************************************************
 * @struct ImgInitParam
 ********************************************************************************/
struct ImgInitParam {
  MUINT32 mSecTag;    // Security Feature: Inform driver to allocate security
                      // working buffer
  MUINT32 mBatchNum;  // SMVR Feature: Inform driver the frame number intervals
  MUINT32 mMaxFps;    // Streaming Feature: support frame rate
  MUINT32 mMaxInOutWidth;   // Streaming Feature: this user will use maximum
                            // frame width by this instance
  MUINT32 mMaxInOutHeight;  // Streaming Feature: this user will use maximum
                            // frame height  by this instance
  IMG_PRIORITY mPriority;
  MBOOL mLowLatency;
  MBOOL mInitDLWKBuf;  // Inform driver whether use direct link working buffer.
  ImgInitParam()
      : mSecTag(0),
        mBatchNum(0),
        mMaxFps(0),
        mMaxInOutWidth(0),
        mMaxInOutHeight(0),
        mPriority(IMG_PRIORITY_DEFAULT),
        mLowLatency(MFALSE),
        mInitDLWKBuf(MFALSE) {}
};

/******************************************************************************
 * @enum IMG_SECURE_ENUM
 *
 * @brief IMG_SECURE_ENUM.
 ******************************************************************************/
enum IMG_SECURE_ENUM {
  IMG_SECURE_NONE = 0,  // normal memory
  IMG_SECURE_SECURE,    // secure memory
  IMG_SECURE_PROTECT,   // protect memory
  IMG_SECURE_MAX
};

/******************************************************************************
 * @enum ImgScenarioPath
 *
 * @brief IMGSYS hw have different hw path, you must choose proper scenario
 *dependent your feature flow.
 ******************************************************************************/
enum IMG_SCENARIO_PATH {
  IMG_SCENARIO_NORMAL = 0,
  IMG_SCENARIO_BOKEH,
  IMG_SCENARIO_FE,
  IMG_SCENARIO_FM,
  IMG_SCENARIO_WUV,
  IMG_SCENARIO_VSDOF_FE,
  IMG_SCENARIO_TOTAL
};

/******************************************************************************
 * @enum ImgResizeRatio
 *
 * @brief IMGSYS hw have many resizers. IMG_RESIZE_ANYRATIO also supports DOWN2
 * , DOWN4 and DOWN42. In practical, the resize vertical and horiziontal step
 *are not the same when you choose rather IMG_RESIZE_ANYRATIO to do DOWN2 than
 *IMG_RESIZE_DOWN2
 ******************************************************************************/
enum IMG_RESIZE_RATIO {
  IMG_RESIZE_ANYRATIO,
  IMG_RESIZE_DOWN4,
  IMG_RESIZE_DOWN2,
  IMG_RESIZE_DOWN42,
  IMG_RESIZE_MAX
};

/******************************************************************************
 * @struct ResizeInfo
 *
 * @brief Resizer Information
 *
 * @param[in] mResizeRatio:
 *
 * @param[in] mEnableAPL: enable average pixel level functionality
 *
 * @param[in] mF0FRMW:
*
* @param[in] mF0FRMH:
*
* @param[in] mMEL0FRMW:
*
* @param[in] mMEL0FRMH:
*
* @param[in] mFScaleIdx:

******************************************************************************/
struct ResizeInfo {
  IMG_RESIZE_RATIO mResizeRatio;
  ResizeInfo() : mResizeRatio(IMG_RESIZE_ANYRATIO) {}
};

/******************************************************************************
 * @struct CropInfo
 *
 * @brief Cropped Rectangle.
 *
 * @param[in] CropX: X integer start position for cropping
 *
 * @param[in] CropY: Y integer start position for crpping
 *
 * @param[in] CropW: width integer of cropped image
 *
 * @param[in] CropH: height integer of cropped image
 *
 * @param[in] CropFloatX: X float start position for cropping
 *
 * @param[in] CropFloatY: Y float start position for cropping
 *
 * @param[in] CropFloatW: width float of cropped image
 *
 * @param[in] CropFloatH: height float of cropped image
 *
 ******************************************************************************/
struct CropInfo {
  int CropX;       //! X integer start position for cropping
  int CropY;       //! Y integer start position for crpping
  int CropW;       //! width integer of cropped image
  int CropH;       //! height integer of cropped image
  int CropFloatX;  //! X float start position for cropping
  int CropFloatY;  //! Y float start position for cropping
  int CropFloatW;  //! width float of cropped image
  int CropFloatH;  //! height float of cropped image
  CropInfo()
      : CropX(0),
        CropY(0),
        CropW(0),
        CropH(0),
        CropFloatX(0),
        CropFloatY(0),
        CropFloatW(0),
        CropFloatH(0) {}
};

/******************************************************************************
 * @struct PortInfo
 *
 * @brief ImgStream Port parameters.
 *
 * @param[in] mPortIdx: The input port ID of the ImgStream
 *
 * @param[in] mPortUsage: we will choose different BT profile (FULL_BT601 or
 *BT601) according the Buffer Usage
 *
 * @param[in] mBuffer: A pointer to an image buffer.
 *            Callee must lock, unlock, and signal release-fence.
 *
 * @param[in] mSecureTag: represent this dma port is security buffer or not
 *
 * @param[in] mTransform: ROTATION CLOCKWISE is applied AFTER FLIP_{H|V}.
 *
 * @param[in] mSrcCrop: Source Crop applied BEFORE transforming and resizing.
 *
 ******************************************************************************/
class IImageBuffer;
struct PortInfo {
 public:
  MUINT32 mPortIdx;  // IMG_PORT_IDX
  IImageBuffer* mBuffer;
  IMG_SECURE_ENUM mSecureTag;  // memory type (normal, secure, protect)
  MINT32 mTransform;
  CropInfo mSrcCrop;
  ResizeInfo mResizeInfo;
  PortInfo()
      : mPortIdx(0), mBuffer(0), mSecureTag(IMG_SECURE_NONE), mTransform(0) {}
};

/******************************************************************************
 *
 * @struct IMG_SRZ_ID
 * @brief srz enum for Img
 * @details
 *
 ******************************************************************************/
enum IMG_SRZ_ID { IMG_SRZ_ID_NONE, IMG_SRZ_ID_FESRZ, IMG_SRZ_ID_CNRSRZ };

/******************************************************************************
 * @struct ImgExtraParam
 *
 * @brief ImgExtraParam.
 *
 * @param[in] mID: specific command index to correspond mData
 *
 * @param[in] mData: specific structure according to command index
 *
 ******************************************************************************/
enum IMG_EXTRA_PARAM_ID {
  IMG_EXTRA_PARAM_ID_DIP_MULTISCALE_INFO,  // type: structure => MultiScaleInfo
  IMG_EXTRA_PARAM_ID_DIP_MUTLIFRAME_INFO,  // type: structure => MultiFrameInfo
  IMG_EXTRA_PARAM_ID_P_IMG4O_CROP_INFO,    // type: structure => PImg4oCropInfo
  IMG_EXTRA_PARAM_ID_MVFRAME_INFO,         // type: structure => MVFrameInfo
  IMG_EXTRA_PARAM_ID_FE_INFO,              // type: structure => FEInfo
  IMG_EXTRA_PARAM_ID_FM_INFO,              // type: structure => FMInfo
  IMG_EXTRA_PARAM_ID_SRZ_INFO,             // type: structure => SRZInfo
  IMG_EXTRA_PARAM_ID_WPE_INFO,             // type: structure => EIS WPE Info
  IMG_EXTRA_PARAM_ID_WPE_TNR_INFO,         // type: structure => TNR WPE Info
  IMG_EXTRA_PARAM_ID_ME_INFO,              // type: structure => ME Info
  IMG_EXTRA_PARAM_ID_PQ_PORT_INFO,         // type: structure => PQPortInfo
  IMG_EXTRA_PARAM_ID_APL_INFO,             // type: structure => APLInfo
  IMG_EXTRA_PARAM_ID_COST_LEVEL_INFO,      // type: structure => WPECostLevel
  IMG_EXTRA_PARAM_ID_TOTAL,
};

struct APLInfo {
  MUINT32 mAplEnable;
};

struct FEInfo {
  MUINT32 mFEDSCR_SBIT;
  MUINT32 mFETH_C;
  MUINT32 mFETH_G;
  MUINT32 mFEFLT_EN;
  MUINT32 mFEPARAM;
  MUINT32 mFEMODE;
  MUINT32 mFEYIDX;
  MUINT32 mFEXIDX;
  MUINT32 mFESTART_X;
  MUINT32 mFESTART_Y;
  MUINT32 mFEIN_HT;
  MUINT32 mFEIN_WD;
};

struct FMInfo {
  MUINT32 mFMHEIGHT;
  MUINT32 mFMWIDTH;
  MUINT32 mFMSR_TYPE;
  MUINT32 mFMOFFSET_X;
  MUINT32 mFMOFFSET_Y;
  MUINT32 mFMRES_TH;
  MUINT32 mFMSAD_TH;
  MUINT32 mFMMIN_RATIO;
};

struct SRZInfo {
  MUINT32 srzId;  // please fill the enum ImgSRZId
  IMG_RESIZE_RATIO srzratio;
  MUINT32 in_w;
  MUINT32 in_h;
  MUINT32 out_w;
  MUINT32 out_h;
  MUINT32 crop_x;
  MUINT32 crop_y;
  MUINT32 crop_floatX;
  MUINT32 crop_floatY;
  MUINT32 crop_w;
  MUINT32 crop_h;
};

struct WPEInfo {
  WPE_MODE wpe_mode;
  WPE_CrpInfo vgen_out;  // Vgen out start and end point

  /* PSP, ISP7.0 only */
  PSP_TABLE_SEL tbl_sel_v;  // PSP table
  PSP_TABLE_SEL tbl_sel_h;  // PSP table

  /* Extra feature */
  unsigned int extra_feature_index;  // enum EXTRA_FEATURE_INDEX,default 0: off
  RGB_MODE rgb_mode;                 // 0: odd line fro RG; 1:even line for GB
  WPE_CrpOfstInfo vgen_in;           // EW_VGENIN_OFST

  unsigned int psp_border_color_y;   // border color y
  unsigned int psp_border_color_u;   // border color u
  unsigned int psp_border_color_v;   // border color v
};

enum COST_LEVEL {
    COST_LEVEL_ORI = 0,
    COST_LEVEL_MIN,
    COST_LEVEL_MED,
    COST_LEVEL_MAX
};

struct WPECostLevel {
  COST_LEVEL costlevel;
};

struct MEInfo {
  ME_MODE me_mode;
};

struct PQPortInfo {
  MUINT32 mWdmaoPQIdx;
  MUINT64 mWdmaoUserString;
  MUINT32 mWdmaoBypassCrop;  // 0: refine crop, 1: bypass crop
  MUINT32 mWrotoPQIdx;
  MUINT64 mWrotoUserString;
  MUINT32 mWrotoBypassCrop;  // 0: refine crop, 1: bypass crop
};

/******************************************************************************
 * @enum IMG_FRAME_SCALE_RATIO
 *
 * @brief
 ******************************************************************************/
enum IMG_MULTI_SCALE_RATIO {
  IMG_MULTI_SCALE_DOWN4,
  IMG_MULTI_SCALE_DOWN2,
  IMG_MULTI_SCALE_MAX
};

struct MultiScaleInfo {
  IMG_MULTI_SCALE_RATIO mScaleRatio;
  MUINT32 mScaleIdx;
  MUINT32 mScaleTotal;
};
struct MultiFrameInfo {
  MUINT32 mFrameIdx;
  MUINT32 mFrameTotal;
};

/******************************************************************************
 *
 * @param[in] mConfScaleRatio: Ratio between MEL0_W & Conf_W
 * E,g, MEL0_W =576, Conf_W = 144 => mConfScaleRatio = 4
 ******************************************************************************/
struct MVFrameInfo {
  MUINT32 mF0Width;
  MUINT32 mF0Height;
  MUINT32 mME0Width;
  MUINT32 mME0Height;
  MUINT32 mConfScaleRatio;
};
/******************************************************************************
 *
 * @param[in] tnrwo_scale_ratio: Ratio between IMGI & TNRWO size
 * E,g, IMGI =1920x1080, Ratio = 8, TNRWO = 1920/8 x 1080/8
 ******************************************************************************/
struct PImg4oCropInfo {
  MUINT32 p_img4o_crop_x;
  MUINT32 p_img4o_crop_y;
  MUINT32 p_img4o_crop_w;
  MUINT32 p_img4o_crop_h;
  MUINT32 tnrwo_scale_ratio;
};

union ExtraParamStruct {
  FEInfo mFEInfo;
  FMInfo mFMInfo;
  SRZInfo mSRZInfo;
  WPEInfo mWPEInfo;
  MEInfo mMEInfo;
  MultiScaleInfo mMutiScaleInfo;
  MultiFrameInfo mMultiFrameInfo;
  MVFrameInfo mMVFrameInfo;
  PImg4oCropInfo mPImg4oCropInfo;  // MCNR Zoom flow
  APLInfo mAPLInfo;
  PQPortInfo mPQPortInfo;
  WPECostLevel mCostLevel;
};

struct ImgExtraParam {
  IMG_EXTRA_PARAM_ID mID;  // Please IMG_EXTRA_PARAM_ID
  ExtraParamStruct mData;
  ImgExtraParam() : mID(IMG_EXTRA_PARAM_ID_TOTAL), mData() {}
};

struct DirectCoupleInfo {
  int64_t mtoken;
  DirectCoupleInfo() : mtoken(-1) {}
};
/******************************************************************************
 *
 * @struct FrameParams
 *
 * @brief Queuing parameters for the pipe.
 *      input cropping -> resizing ->
 *      output flip_{H|V} -> output rotation
 *
 * @param[in] mNeedDump: User decide this frame whether to save ISP bit true
 *files.
 *
 * @param[in] mDumpHint: isp bit true file save information
 *
 * @param[in] mSecureFra: to decide this frame is secrrity or not
 *
 * @param[in] mScenPath: to decide which hw scenario will be used
 *
 * @param[in] mpreemptable: This frame can be preemptable or not, MFALSE: this
 *frame can't be preemptable, MTRUE:this frame will share the HW when the HW is
 *avaiable
 *
 * @param[in] mvIn: a vector of input parameters.
 *
 * @param[in] mvOut: a vector of output parameters.
 *
 * @param[in] mvExtraParam: extra ImgExtraParam information in this frame
 *request.
 *
 * @param[in] mSyncTokenNotify: use this token to inform the waiting frame.
 * Sync Token from which frame in specific request
 *
 * @param[in] mSyncTokenWait: this frame will be notified by this token.
 * Sync Token to which frame in specific request
 *
 * @param[in] mSyncTokenNotifyList: use this token list to inform the waiting
 *frame
 *
 * @param[in] mSyncTokenWaitList: this frame will be notified by this token
 *list.
 *
 * @param[in] mpCookie: frame callback cookie; it shouldn't be modified by the
 *pipe.
 *
 * @param[in] mpfnCallback: optional, when the user fill the parameter, we will
 *call the callback function when the frame is finished.
 *
 ******************************************************************************/
struct FrameParams {
  MUINT32 mTimestamp;
  MBOOL mSecureFra;  // to decide this frame is secrrity or not
  MINT32 mScenPath;  // IMG_SCENARIO_PATH
  std::vector<PortInfo> mvIn;
  std::vector<PortInfo> mvOut;
  std::vector<ImgExtraParam> mvExtraParam;
  MBOOL mSyncPrevFrameParam;
  MBOOL mSyncNextFrameParam;
  MUINT32 mSyncTokenNotify;
  MUINT32 mSyncTokenWait;
  std::list<MUINT32> mSyncTokenNotifyList;
  std::list<MUINT32> mSyncTokenWaitList;
  MVOID* mpCookie;
  int mStage;
  DirectCoupleInfo mDCinfo;  // Direct couple APU Infomation
  EIGHTCC mFrameOwner;       // to find this frame is from which owner. MW have
                        // different source enque to the same ImgStream Instance
  std::unordered_map<uint32_t, uint32_t> mPortSel;
  typedef MVOID (*PFN_FRAMECB_T)(
      const FrameParams& rFrmParams);  // per frame callback
  PFN_FRAMECB_T mpfnCallback = nullptr;          // deque call back
  FrameParams()
      : mTimestamp(0),
        mSecureFra(0),
        mScenPath(-1),
        mvIn(),
        mvOut(),
        mvExtraParam(),
        mSyncPrevFrameParam(MFALSE),
        mSyncNextFrameParam(MFALSE),
        mSyncTokenNotify(0),
        mSyncTokenWait(0),
        mSyncTokenNotifyList(),
        mSyncTokenWaitList(),
        mpCookie(NULL),
        mStage(0),
        mFrameOwner(),
        mPortSel(),
        mpfnCallback(NULL) {}
};

/******************************************************************************
 *
 * @struct IMGParams
 *
 * @brief Queuing many frame parameters for ImgStream
 *
 * @param[in] mvFrameParams: a vector of frame parameters.
 *
 * @param[in] mpCookie: Package callback cookie; it shouldn't be modified by the
 *pipe.
 *
 * @param[in] mpfnCallback: it can't be NULL. We will call the callback function
 *when the package is finished.
 *
 ******************************************************************************/
struct ImgParams {
   typedef MVOID (*PFN_IMG_CALLBACK_T)(const ImgParams& rImgParams);

  typedef MVOID (*PFN_IMG_CBF_T)(
      const std::shared_ptr<const ImgParams>& pRarams);

  MBOOL mHWSharing;  // MFALSE: this Package can't be preemptable, MTRUE:this
                     // Package will share the HW when the HW is avaiable.
  MUINT32 mFps;      // This package exptected frame rate
  MINT32 mSyncID;
  MVOID* mpCookie;
  int mRequestNo;
  int mFrameNo;
  unsigned int mNumBatchRun;
   PFN_IMG_CALLBACK_T mpfnCallback;

  PFN_IMG_CBF_T mpfnCB;
  std::vector<FrameParams> mvFrameParams;
  ImgParams()
      : mHWSharing(MFALSE),
        mFps(30),
        mSyncID(-1),
        mpCookie(NULL),
        mRequestNo(0),
        mFrameNo(0),
        mNumBatchRun(0),
         mpfnCallback(NULL),
        mpfnCB(NULL),
        mvFrameParams() {}
};
/******************************************************************************
 *
 ******************************************************************************/
}      // namespace NSImgStream
}      // namespace NSCam
#endif  // INCLUDE_MTKCAM_CORE_HW_IMGSTREAM_IIMGSTREAMDEF_H_
