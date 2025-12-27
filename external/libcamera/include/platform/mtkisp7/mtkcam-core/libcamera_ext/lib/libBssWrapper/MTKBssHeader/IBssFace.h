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


#ifndef _I_BSS_FACES_H_
#define _I_BSS_FACES_H_

#include "IBssType.h"
//typedef int MINT32;
//typedef float float32_t;
/******************************************************************************
 *  The information of a face from camera face detection.
 ******************************************************************************/
 struct IBssFace {
    /**
     * Bounds of the face [left, top, right, bottom]. (-1000, -1000) represents
     * the top-left of the camera field of view, and (1000, 1000) represents the
     * bottom-right of the field of view. The width and height cannot be 0 or
     * negative. This is supported by both hardware and software face detection.
     *
     * The direction is relative to the sensor orientation, that is, what the
     * sensor sees. The direction is not affected by the rotation or mirroring
     * of CAMERA_CMD_SET_DISPLAY_ORIENTATION.
     */
    MINT32 rect[4];

    /**
     * The confidence level of the face. The range is 1 to 100. 100 is the
     * highest confidence. This is supported by both hardware and software
     * face detection.
     */
    MINT32 score;

    /**
     * An unique id per face while the face is visible to the tracker. If
     * the face leaves the field-of-view and comes back, it will get a new
     * id. If the value is 0, id is not supported.
     */
    MINT32 id;

    /**
     * The coordinates of the center of the left eye. The range is -1000 to
     * 1000. -2000, -2000 if this is not supported.
     */
    MINT32 left_eye[2];

    /**
     * The coordinates of the center of the right eye. The range is -1000 to
     * 1000. -2000, -2000 if this is not supported.
     */
    MINT32 right_eye[2];

    /**
     * The coordinates of the center of the mouth. The range is -1000 to 1000.
     * -2000, -2000 if this is not supported.
     */
    MINT32 mouth[2];
};


/******************************************************************************
 *   FD Pose Information: ROP & RIP
 *****************************************************************************/
struct IBssFaceInfo {

    MINT32 rop_dir;
    MINT32 rip_dir;
};

/******************************************************************************
 *   CNN FD Information
 *****************************************************************************/
struct IBssCNNFaceInfo {
    MINT32 PortEnable;
    MINT32 IsTrueFace;
    float CnnResult0;
    float CnnResult1;
};

/******************************************************************************
 *  The metadata of the frame data.
 *****************************************************************************/
//if add any member, should be notify other owner
 struct IBssFaceMetadata {
    /**
     * The number of detected faces in the frame.
     */
    MINT32 number_of_faces;

    /**
     * An array of the detected faces. The length is number_of_faces.
     */
    IBssFace   *faces;
    IBssFaceInfo     *posInfo;
    MINT32         faces_type[15];
    MINT32         motion[15][2];

    MINT32 ImgWidth;
    MINT32 ImgHeight;

    MINT32 gmIndex;
    MINT32* gmData;
    MINT32 gmSize;
    MINT32 genderNum;
    MINT32 poseNum;
    MINT32 landmarkNum;
    MINT32 leyex0[15];
    MINT32 leyey0[15];
    MINT32 leyex1[15];
    MINT32 leyey1[15];
    MINT32 reyex0[15];
    MINT32 reyey0[15];
    MINT32 reyex1[15];
    MINT32 reyey1[15];
    MINT32 nosex[15];
    MINT32 nosey[15];
    MINT32 mouthx0[15];
    MINT32 mouthy0[15];
    MINT32 mouthx1[15];
    MINT32 mouthy1[15];
    MINT32 leyeux[15];
    MINT32 leyeuy[15];
    MINT32 leyedx[15];
    MINT32 leyedy[15];
    MINT32 reyeux[15];
    MINT32 reyeuy[15];
    MINT32 reyedx[15];
    MINT32 reyedy[15];
    MINT32 fa_cv[15];
    MINT32 fld_rip[15];
    MINT32 fld_rop[15];
    MINT32 YUVsts[15][5];
    MUINT8 fld_GenderLabel[15];
    MINT32 fld_GenderInfo[15];
    MUINT8 GenderLabel[15];
    MUINT8 oGenderLabel[15];
    MUINT8 GenderCV[15];
    MUINT8 RaceLabel[15];
    MUINT8 RaceCV[15][4];

    int16_t poseinfo[15][10];
    /**
     * Timestamp of source yuv frame
     */
    MINT64 timestamp;
    /**
     * for CNN face
     */
    IBssCNNFaceInfo CNNFaces;
    IBssFaceMetadata(): number_of_faces(0), faces(0), posInfo(0), gmIndex(0), gmData(NULL), gmSize(0)
    {
    }
};


#endif

