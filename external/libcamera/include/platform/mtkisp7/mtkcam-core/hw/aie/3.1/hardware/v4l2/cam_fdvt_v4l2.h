/*
 * Copyright (C) 2019 MediaTek Inc.
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

#ifndef HW_AIE_3_1_HARDWARE_V4L2_CAM_FDVT_V4L2_H_
#define HW_AIE_3_1_HARDWARE_V4L2_CAM_FDVT_V4L2_H_

#include <fcntl.h>
#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>
#include <linux/media.h>
#include <linux/v4l2-common.h>
#include <linux/v4l2-controls.h>
#include <linux/v4l2-mediabus.h>
#include <linux/v4l2-subdev.h>
#include <linux/videodev2.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <sys/time.h>
#include <sys/types.h>

#include "platform/mtkisp7/mtkcam-core/hw/aie/3.1/fdvtcommon.h"

typedef uint16_t MUINT16;
typedef int16_t MINT16;

#define MATCH_NAME_STR_SIZE_MAX 64
#define FLD_CUR_LANDMARK 11

// Detection error code
#define S_Detection_OK 0x0000
#define E_Init_Open_Fail -1
#define E_Init_Set_Fail -2
#define E_Init_Param_Fail -3

#define MAX_FACE_NUM 1024
#define RLT_NUM 48
#define GENDER_OUT 32

#define FLD_MAX_LANDMARK 11
#define FLD_MAX_FRAME 15
#define FLD_OUTPUT_SIZE 112

struct FdDrv_init_struct {
  unsigned int src_max_width;
  unsigned int src_max_height;
  unsigned int src_pyramid_width;
  unsigned int src_pyramid_height;
  unsigned int feature_threshold;
};

typedef struct FDVT_V_ROI {
  unsigned int x1;
  unsigned int y1;
  unsigned int x2;
  unsigned int y2;
} FDVT_V_ROI;

typedef struct FDVT_V_PADDING {
  unsigned int left;
  unsigned int right;
  unsigned int down;
  unsigned int up;
} FDVT_V_PADDING;

typedef struct FDVT_V_FLD_RIP_ROP {
  unsigned int fld_in_crop_x1;
  unsigned int fld_in_crop_y1;
  unsigned int fld_in_crop_x2;
  unsigned int fld_in_crop_y2;
  unsigned int fld_in_rip;
  unsigned int fld_in_rop;
} FDVT_V_FLD_RIP_ROP;

typedef struct {
  unsigned int fd_mode;
  unsigned int src_img_fmt;
  unsigned int src_img_width;
  unsigned int src_img_height;
  unsigned int src_img_stride;
  unsigned int pyramid_base_width;
  unsigned int pyramid_base_height;
  unsigned int number_of_pyramid;
  unsigned int input_rotate_degree;
  int en_roi;
  FDVT_V_ROI src_roi;
  int en_padding;
  FDVT_V_PADDING src_padding;
  unsigned int freq_level;
  unsigned int fld_face_num;
  FDVT_V_FLD_RIP_ROP fld_input[FLD_MAX_FRAME];
} FdDrv_input_struct;

#if KERNEL_SPACE_RLT
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
};

struct FD_V_RESULT {
  MUINT16 FD_PYRAMID0_NUM;
  MUINT16 FD_PYRAMID1_NUM;
  MUINT16 FD_PYRAMID2_NUM;
  MUINT16 FD_TOTAL_NUM;
  struct FDRESULT PYRAMID0_RESULT;
  struct FDRESULT PYRAMID1_RESULT;
  struct FDRESULT PYRAMID2_RESULT;
};
#else
struct FD_V_RESULT {
  MUINT16 FD_PYRAMID0_NUM;
  MUINT16 FD_PYRAMID1_NUM;
  MUINT16 FD_PYRAMID2_NUM;
  MUINT16 FD_TOTAL_NUM;
  unsigned char RPN31_RLT[MAX_FACE_NUM][RLT_NUM];
  unsigned char RPN63_RLT[MAX_FACE_NUM][RLT_NUM];
  unsigned char RPN95_RLT[MAX_FACE_NUM][RLT_NUM];
};
#endif

#if KERNEL_SPACE_RLT
struct ATTRIBUTE_V_RACERESULT {
  MINT16 RESULT[4][64];  // RESULT[Channel][Feature]
};

struct ATTRIBUTE_V_GENDERRESULT {
  MINT16 RESULT[2][64];  // RESULT[Channel][Feature]
};

struct ATTRIBUTE_V_RIPRESULT {
  MINT16 RESULT[7][64];  // RESULT[Channel][Feature]
};

struct ATTRIBUTE_V_ROPRESULT {
  MINT16 RESULT[3][64];  // RESULT[Channel][Feature]
};

struct ATTRIBUTE_V_MERGED_RACERESULT {
  MINT16 RESULT[4];  // RESULT[Feature]
};

struct ATTRIBUTE_V_MERGED_GENDERRESULT {
  MINT16 RESULT[2];  // RESULT[Feature]
};

struct ATTRIBUTE_V_MERGED_AGERESULT {
  MINT16 RESULT[2];  // RESULT[Feature]
};

struct ATTRIBUTE_V_MERGED_IS_INDIANRESULT {
  MINT16 RESULT[2];  // RESULT[Feature]
};

struct ATTRIBUTE_V_MERGED_RIPRESULT {
  MINT16 RESULT[7];  // RESULT[Feature]
};

struct ATTRIBUTE_V_MERGED_ROPRESULT {
  MINT16 RESULT[3];  // RESULT[Feature]
};

struct ATTRIBUTE_V_RESULT {
  struct ATTRIBUTE_V_GENDERRESULT GENDER_RESULT;
  struct ATTRIBUTE_V_RACERESULT RACE_RESULT;
  struct ATTRIBUTE_V_MERGED_AGERESULT MERGED_AGE_RESULT;
  struct ATTRIBUTE_V_MERGED_GENDERRESULT MERGED_GENDER_RESULT;
  struct ATTRIBUTE_V_MERGED_IS_INDIANRESULT MERGED_IS_INDIAN_RESULT;
  struct ATTRIBUTE_V_MERGED_RACERESULT MERGED_RACE_RESULT;
};
#else
struct ATTRIBUTE_V_RESULT {
  unsigned char RPN17_RLT[GENDER_OUT];
  unsigned char RPN20_RLT[GENDER_OUT];
  unsigned char RPN22_RLT[GENDER_OUT];
  unsigned char RPN25_RLT[GENDER_OUT];
};
#endif

#if KERNEL_SPACE_RLT
struct FLD_V_LANDMARK{
  unsigned short x;
  unsigned short y;
};

struct FLD_V_RESULT{
  struct FLD_V_LANDMARK fld_landmark[FLD_CUR_LANDMARK];
  signed short fld_out_rip;
  signed short fld_out_rop;
  unsigned short confidence;
  signed short blinkscore;
};
#else
struct FLD_V_RESULT {
  unsigned char FLD_OUTPUT_RLT[FLD_MAX_FRAME][FLD_OUTPUT_SIZE];
};
#endif

typedef struct {
  unsigned int FD_MODE;
  unsigned int SRC_IMG_FMT;
  unsigned int SRC_IMG_WIDTH;
  unsigned int SRC_IMG_HEIGHT;
  unsigned int SRC_IMG_STRIDE;
  unsigned int pyramid_base_width;
  unsigned int pyramid_base_height;
  unsigned int number_of_pyramid;
  unsigned int INPUT_ROTATE_DEGREE;
  int enROI;
  FDVT_V_ROI src_roi;
  int enPadding;
  FDVT_V_PADDING src_padding;
  unsigned int freq_level;
  unsigned int fld_face_num;
  FDVT_V_FLD_RIP_ROP fld_input[FLD_MAX_FRAME];
  unsigned int source_img_address;
  unsigned int source_img_address_UV;
  unsigned int Fd_version;
  unsigned int Attr_version;
  unsigned int Pose_version;
  struct FD_V_RESULT FDOUTPUT;
  struct ATTRIBUTE_V_RESULT ATTRIBUTEOUTPUT;
#if KERNEL_SPACE_RLT
  struct FLD_V_RESULT FLDOUTPUT[FLD_MAX_FRAME];
#else
  struct FLD_V_RESULT FLDOUTPUT;
#endif
  unsigned int irq_status;
} FdDrv_output_struct;

int FDVT_OpenDriver(FdDrv_init_struct* FdDrv_init, unsigned int* fd_version,
                    unsigned int* attr_version, unsigned int* fld_version,
                    uint16_t* landmark_count);
int FDVT_CloseDriver();
int FDVT_Enque(FdDrv_input_struct* FdDrv_input, int plane0Fd, int plane1Fd);
int FDVT_Deque(FdDrv_output_struct** FdDrv_output);
int FDVT_Poll();

#endif  // HW_AIE_3_1_HARDWARE_V4L2_CAM_FDVT_V4L2_H_
