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


#ifndef _MHAL_MTK_FACES_H_
#define _MHAL_MTK_FACES_H_

#include <cstdint>

typedef float float32_t;
// avoid redefine: typedef long int int64_t;
typedef long long int MTEE_INT64;
/******************************************************************************
 *  The information of a face from camera face detection.
 ******************************************************************************/
 struct MtkCameraFace {
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
    int32_t rect[4];

    /**
     * The confidence level of the face. The range is 1 to 100. 100 is the
     * highest confidence. This is supported by both hardware and software
     * face detection.
     */
    int32_t score;

    /**
     * An unique id per face while the face is visible to the tracker. If
     * the face leaves the field-of-view and comes back, it will get a new
     * id. If the value is 0, id is not supported.
     */
    int32_t id;

    /**
     * The coordinates of the center of the left eye. The range is -1000 to
     * 1000. -2000, -2000 if this is not supported.
     */
    int32_t left_eye[2];

    /**
     * The coordinates of the center of the right eye. The range is -1000 to
     * 1000. -2000, -2000 if this is not supported.
     */
    int32_t right_eye[2];

    /**
     * The coordinates of the center of the mouth. The range is -1000 to 1000.
     * -2000, -2000 if this is not supported.
     */
    int32_t mouth[2];
};


/******************************************************************************
 *   FD Pose Information: ROP & RIP
 *****************************************************************************/
struct MtkFaceInfo {

    int32_t rop_dir;
    int32_t rip_dir;
};

/******************************************************************************
 *   CNN FD Information
 *****************************************************************************/
struct MtkCNNFaceInfo {
    int32_t PortEnable;
    int32_t IsTrueFace;
    float32_t CnnResult0;
    float32_t CnnResult1;
};

/******************************************************************************
 *  The metadata of the frame data.
 *****************************************************************************/
struct MtkCameraFaceMetadata {
    /**
     * The number of detected faces in the frame.
     */
    int32_t number_of_faces;

    /**
     * An array of the detected faces. The length is number_of_faces.
     */
#if defined(_SIM_PC_) || defined(FDUT_RUN)
    MtkCameraFace   *faces;
    MtkFaceInfo     *posInfo;
#else
    MtkCameraFace   faces[15];
    MtkFaceInfo     posInfo[15];
#endif
    int32_t         faces_type[15];
    int32_t         motion[15][2];

    int32_t ImgWidth;
    int32_t ImgHeight;

    // GGM (before 6s)
    int32_t gmIndex;
    union{
    int32_t* gmData;
        MTEE_INT64 TEMP64_2;
    };
    int32_t gmSize;

    // TCY (after 70)
    int32_t tcy_index;
    uint32_t tcy_uv_gain;
    uint32_t tcy_y_curve[32];

    int32_t genderNum;
    int32_t poseNum;
    int32_t landmarkNum;
    int32_t leyex0[15];
    int32_t leyey0[15];
    int32_t leyex1[15];
    int32_t leyey1[15];
    int32_t reyex0[15];
    int32_t reyey0[15];
    int32_t reyex1[15];
    int32_t reyey1[15];
    int32_t nosex[15];
    int32_t nosey[15];
    int32_t mouthx0[15];
    int32_t mouthy0[15];
    int32_t mouthx1[15];
    int32_t mouthy1[15];
    int32_t leyeux[15];
    int32_t leyeuy[15];
    int32_t leyedx[15];
    int32_t leyedy[15];
    int32_t reyeux[15];
    int32_t reyeuy[15];
    int32_t reyedx[15];
    int32_t reyedy[15];
    int32_t fa_cv[15];
    int32_t fld_rip[15];
    int32_t fld_rop[15];
    int32_t YUVsts[15][5];
    uint8_t fld_GenderLabel[15];
    int32_t fld_GenderInfo[15];
    uint8_t GenderLabel[15];
    uint8_t oGenderLabel[15];
    uint8_t GenderCV[15];
    uint8_t RaceLabel[15];
    uint8_t RaceCV[15][4];
    uint8_t visible_percent[15]; //the visible percentage of each face in preview FOV: 0~100
    int16_t poseinfo[15][10];
	uint8_t NumofSkinMap;	// number of valid skip map
	uint8_t SkinMapWidth;	// after AIE3.1, will be 30
	uint8_t SkinMapHeight;	// after AIE3.1, will be 50
	uint8_t SkinMap[3][1500]; //maximal 3 skin maps: 30x50
	double  TfmtxMap2TG[3][6]; // transformation matrix from skin map to TG

    /**
     * Timestamp of source yuv frame
     */
    int64_t timestamp;
    int32_t magicNo;
    /**
     * for CNN face
     */
    MtkCNNFaceInfo CNNFaces;
    MtkCameraFaceMetadata(): number_of_faces(0), gmIndex(0), gmData(nullptr), gmSize(0)
    {
    }
};


#endif
