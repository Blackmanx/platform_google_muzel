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

#ifndef HW_AIE_3_1_FDVTCOMMON_H_
#define HW_AIE_3_1_FDVTCOMMON_H_

#include "mtkcam-halif/def/BuiltinTypes.h"

#define DRV_TRACE_NAME_LENGTH 32
#define DRV_TRACE_CALL()
#define DRV_TRACE_NAME(name)
#define DRV_TRACE_INT(name, value)
#define DRV_TRACE_BEGIN(name)
#define DRV_TRACE_END()
#define DRV_TRACE_ASYNC_BEGIN(name, cookie)
#define DRV_TRACE_ASYNC_END(name, cookie)
#define DRV_TRACE_FMT_BEGIN(fmt, arg...)

#define DRV_TRACE_FMT_END() DRV_TRACE_END()
#define MAX_FACE_NUM 1024
#define RLT_NUM 48
// todo(yerlandinata): non-inclusive term
#define GENDER_OUT 32
#define PYRAMID_NUM 3

#ifndef FLD_FEATURE
#define FLD_FEATURE 1
#endif

#define KERNEL_SPACE_RLT 1

/******************************************************************************
 *
 ******************************************************************************/
namespace NSCam {
namespace NSIoPipe {

enum SUB_ENGINE_ID { eFDVT = 0, eENGINE_MAX = 1 };

/*FDVT_CTRL*/
/* API for MM */
/* num for IMB */
enum FDVTMODE { FDMODE = 0, ATTRIBUTEMODE = 1, FLDMODE = 2, POSEMODE = 3};

enum FLDROP { NORMAL = 0, RIGHT = 1, LEFT = 2 };
enum FLDRIP { FLD_0 = 0, FLD_1 = 1, FLD_2 = 2, FLD_3 = 3, FLD_4 = 4,
              FLD_5 = 5, FLD_6 = 6, FLD_7 = 7, FLD_8 = 8, FLD_9 = 9,
              FLD_10 = 10, FLD_11 = 11};

enum FDVTFORMAT {
  FMT_NA = 0,
  FMT_YUV_2P = 1,
  FMT_YVU_2P = 2,
  FMT_YUYV = 3,  // 1 plane
  FMT_YVYU = 4,  // 1 plane
  FMT_UYVY = 5,  // 1 plane
  FMT_VYUY = 6,  // 1 plane
  FMT_MONO = 7,  // AIE2.0
  FMT_YUV420_2P = 8,
  FMT_YUV420_1P = 9
};

enum FDVTINPUTDEGREE {
  DEGREE_0 = 0,
  DEGREE_90 = 1,
  DEGREE_270 = 2,
  DEGREE_180 = 3
};

struct EGNBufInfo {
  MINT32 memID;       //  memory ID
  MUINTPTR u4BufVA;   //  Vir Address of pool
  MUINTPTR u4BufPA;   //  Phy Address of pool
  MUINT32 u4BufSize;  //  Per buffer size
  MUINT32 u4Stride;   //  Buffer Stride
  MUINT32 width;
  MUINT32 height;
  EGNBufInfo()
      : memID(0),
        u4BufVA(0),
        u4BufPA(0),
        u4BufSize(0),
        u4Stride(0),
        width(0),
        height(0) {}
};

struct EGNInitInfo {
  MUINT16 MAX_SRC_IMG_WIDTH;
  MUINT16 MAX_SRC_IMG_HEIGHT;
  MINT16 feature_threshold;
  MUINT16 pyramid_width;
  MUINT16 pyramid_height;
  MUINT32 fd_version;
  MUINT32 attr_version;
  MBOOL IS_FDVT_SECURE;
  MUINT32 SEC_MEM_TYPE;
  MUINT32 fld_version;
  MUINT16 landmark_count;
  EGNInitInfo()
      : MAX_SRC_IMG_WIDTH(0),
        MAX_SRC_IMG_HEIGHT(0),
        feature_threshold(5),
        pyramid_width(480),
        pyramid_height(480),
        fd_version(0),
        attr_version(0),
        IS_FDVT_SECURE(0),
        SEC_MEM_TYPE(1),
        fld_version(0),
        landmark_count(0){}
};

struct FDRESULT {
  MUINT16 anchor_x0[MAX_FACE_NUM];
  MUINT16 anchor_x1[MAX_FACE_NUM];
  MUINT16 anchor_y0[MAX_FACE_NUM];
  MUINT16 anchor_y1[MAX_FACE_NUM];
  MINT16 rop_landmark_score0[MAX_FACE_NUM];
  MINT16 rop_landmark_score1[MAX_FACE_NUM];
  MINT16 rop_landmark_score2[MAX_FACE_NUM];
  MINT16 anchor_score[MAX_FACE_NUM];
  MINT16 rip_landmark_score0[MAX_FACE_NUM];
  MINT16 rip_landmark_score1[MAX_FACE_NUM];
  MINT16 rip_landmark_score2[MAX_FACE_NUM];
  MINT16 rip_landmark_score3[MAX_FACE_NUM];
  MINT16 rip_landmark_score4[MAX_FACE_NUM];
  MINT16 rip_landmark_score5[MAX_FACE_NUM];
  MINT16 rip_landmark_score6[MAX_FACE_NUM];
  MUINT16 face_result_index[MAX_FACE_NUM];
  MUINT16 anchor_index[MAX_FACE_NUM];
  MUINT32 fd_partial_result;
  FDRESULT()
      : anchor_x0{0},
        anchor_x1{0},
        anchor_y0{0},
        anchor_y1{0},
        rop_landmark_score0{0},
        rop_landmark_score1{0},
        rop_landmark_score2{0},
        anchor_score{0},
        rip_landmark_score0{0},
        rip_landmark_score1{0},
        rip_landmark_score2{0},
        rip_landmark_score3{0},
        rip_landmark_score4{0},
        rip_landmark_score5{0},
        rip_landmark_score6{0},
        face_result_index{0},
        anchor_index{0},
        fd_partial_result(0) {}
};

struct FD_RESULT {
  MUINT16 FD_PYRAMID0_NUM;
  MUINT16 FD_PYRAMID1_NUM;
  MUINT16 FD_PYRAMID2_NUM;
  MUINT16 FD_TOTAL_NUM;
  struct FDRESULT PYRAMID0_RESULT;
  struct FDRESULT PYRAMID1_RESULT;
  struct FDRESULT PYRAMID2_RESULT;
  FD_RESULT()
      : FD_PYRAMID0_NUM{0},
        FD_PYRAMID1_NUM{0},
        FD_PYRAMID2_NUM{0},
        FD_TOTAL_NUM(0) {}
};

// todo(yerlandinata): non-inclusive term
struct RACERESULT {
  MINT16 RESULT[4][64];  // RESULT[Channel][Feature]
  RACERESULT() : RESULT{{0,0,0,0}} {}
};

// todo(yerlandinata): non-inclusive term
struct GENDERRESULT {
  MINT16 RESULT[2][64];  // RESULT[Channel][Feature]
  GENDERRESULT() : RESULT{{0,0}} {}
};

struct RIPRESULT {
  MINT16 RESULT[7][64];  // RESULT[Channel][Feature]
  RIPRESULT() : RESULT{{0,0,0,0,0,0,0}} {}
};

struct ROPRESULT {
  MINT16 RESULT[3][64];  // RESULT[Channel][Feature]
  ROPRESULT() : RESULT{{0,0,0}} {}
};

// todo(yerlandinata): non-inclusive term
struct MERGED_RACERESULT {
  MINT16 RESULT[4];  // RESULT[Feature]
  MERGED_RACERESULT() : RESULT{0} {}
};

// todo(yerlandinata): non-inclusive term
struct MERGED_GENDERRESULT {
  MINT16 RESULT[2];  // RESULT[Feature]
  MERGED_GENDERRESULT() : RESULT{0} {}
};

// todo(yerlandinata): non-inclusive term
struct MERGED_AGERESULT {
  MINT16 RESULT[2];  // RESULT[Feature]
  MERGED_AGERESULT() : RESULT{0} {}
};

struct MERGED_IS_INDIANRESULT {
  MINT16 RESULT[2];  // RESULT[Feature]
  MERGED_IS_INDIANRESULT() : RESULT{0} {}
};

struct MERGED_RIPRESULT {
  MINT16 RESULT[7];  // RESULT[Feature]
  MERGED_RIPRESULT() : RESULT{0} {}
};

struct MERGED_ROPRESULT {
  MINT16 RESULT[3];  // RESULT[Feature]
  MERGED_ROPRESULT() : RESULT{0} {}
};

struct ATTRIBUTE_RESULT {
  // todo(yerlandinata): non-inclusive term
  struct GENDERRESULT GENDER_RESULT;
  struct RACERESULT RACE_RESULT;
  struct MERGED_AGERESULT MERGED_AGE_RESULT;
  struct MERGED_GENDERRESULT MERGED_GENDER_RESULT;
  struct MERGED_IS_INDIANRESULT MERGED_IS_INDIAN_RESULT;
  struct MERGED_RACERESULT MERGED_RACE_RESULT;
};

struct POSE_RESULT {
  struct RIPRESULT RIP_RESULT;
  struct ROPRESULT ROP_RESULT;
  struct MERGED_RIPRESULT MERGED_RIP_RESULT;
  struct MERGED_ROPRESULT MERGED_ROP_RESULT;
};

struct feedback {
  MUINT32 reg1;
  MUINT32 reg2;
  feedback() : reg1(0), reg2(0) {}
};

typedef struct FDVTROI {
  MUINT x1;
  MUINT y1;
  MUINT x2;
  MUINT y2;
  FDVTROI() : x1(0), y1(0), x2(0), y2(0) {}
} FDVTROI;

typedef struct FDVTPADDING {
  MUINT left;
  MUINT right;
  MUINT down;
  MUINT up;
  FDVTPADDING() : left(0), right(0), down(0), up(0) {}
} FDVTPADDING;

struct FLD_LANDMARK{
  MUINT16 x;
  MUINT16 y;
};

struct FLD_CROP_RIP_ROP{
  FDVTROI fld_in_crop;
  FLDRIP fld_in_rip;
  FLDROP fld_in_rop;
};

struct FLD_RESULT{
  struct FLD_LANDMARK *fld_landmark;
  MINT16 fld_out_rip;
  MINT16 fld_out_rop;
  MUINT16 confidence;
  MINT16 blinkscore;
};

struct FDVTConfig {
  FDVTMODE FD_MODE;
  FDVTFORMAT SRC_IMG_FMT;
  MUINT16 SRC_IMG_WIDTH;
  MUINT16 SRC_IMG_HEIGHT;
  MUINT16 SRC_IMG_STRIDE;
  MUINT16 pyramid_base_width;
  MUINT16 pyramid_base_height;
  MUINT16 number_of_pyramid;
  FDVTINPUTDEGREE INPUT_ROTATE_DEGREE;
  bool enROI;
  FDVTROI src_roi;
  bool enPadding;
  FDVTPADDING src_padding;
  unsigned int freq_level;
  MUINT64* source_img_address;
  MUINT64* source_img_address_UV;
  unsigned int Fd_version;
  unsigned int Attr_version;
  unsigned int Pose_version;
  struct FD_RESULT *pFDOUTPUT;
  struct ATTRIBUTE_RESULT *pATTRIBUTEOUTPUT;
  struct POSE_RESULT *pPOSEOUTPUT;
  feedback feedback_;
  MUINT64* pY2R_config;
  MUINT64* pRS_config;
  MUINT64* pFd_config;
  MUINT64* pFD_POSE_config;
  MINT32 FD_Y;
  MINT32 FD_UV;
  MUINT16 fld_num_face;
  struct FLD_CROP_RIP_ROP *fld_input;
#if CHECK_BITTRUE
  unsigned char *fld_raw_out;    // fld raw output buf
#endif
  struct FLD_RESULT *fld_result;
  FDVTConfig()
      : FD_MODE(FDMODE),
        SRC_IMG_FMT(FMT_NA),
        SRC_IMG_WIDTH(0),
        SRC_IMG_HEIGHT(0),
        SRC_IMG_STRIDE(0),
        pyramid_base_width(0),
        pyramid_base_height(0),
        number_of_pyramid(3),
        INPUT_ROTATE_DEGREE(DEGREE_0),
        enROI(false),
        enPadding(false),
        freq_level(0),
        source_img_address(nullptr),
        source_img_address_UV(nullptr),
        Fd_version(0),
        Attr_version(0),
        Pose_version(0),
        pFDOUTPUT(nullptr),
        pATTRIBUTEOUTPUT(nullptr),
        pPOSEOUTPUT(nullptr),
        pY2R_config(nullptr),
        pRS_config(nullptr),
        pFd_config(nullptr),
        pFD_POSE_config(nullptr),
        FD_Y(-1),
        FD_UV(-1),
        fld_num_face(0),
        fld_input(nullptr),
#if CHECK_BITTRUE
        fld_raw_out(nullptr),
#endif
        fld_result(nullptr){}
};

/******************************************************************************
 *
 ******************************************************************************/
}       // namespace NSIoPipe
}       // namespace NSCam
#endif  // HW_AIE_3_1_FDVTCOMMON_H_
