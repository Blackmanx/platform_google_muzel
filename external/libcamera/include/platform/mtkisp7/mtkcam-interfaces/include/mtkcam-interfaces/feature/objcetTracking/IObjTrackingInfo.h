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

#ifndef INCLUDE_MTKCAM_INTERFACES_FEATURE_OBJCETTRACKING_IOBJTRACKINGINFO_H_
#define INCLUDE_MTKCAM_INTERFACES_FEATURE_OBJCETTRACKING_IOBJTRACKINGINFO_H_

namespace  OT_INFO_IF{
  typedef enum OBJ_CATEGORY_ENUM {
    OBJ_PERSON,
    OBJ_BICYCLE, OBJ_CAR, OBJ_MOTORBIKE, OBJ_AEROPLANE, OBJ_BUS, OBJ_TRAIN,
    OBJ_TRUCK, OBJ_BOAT,
    OBJ_TRAFFIC_LIGHT, OBJ_FIRE_HYDRANT, OBJ_STOP_SIGN, OBJ_PARKING_METER,
    OBJ_BENCH,
    OBJ_BIRD, OBJ_CAT, OBJ_DOG, OBJ_HORSE, OBJ_SHEEP, OBJ_COW, OBJ_ELEPHANT,
    OBJ_BEAR, OBJ_ZEBRA,
    OBJ_GIRAFFE,
    OBJ_BACKPACK, OBJ_UMBRELLA, OBJ_HANDBAG, OBJ_TIE, OBJ_SUITCASE,
    OBJ_FRISBEE, OBJ_SKIS, OBJ_SNOWBOARD, OBJ_SPORTS_BALL, OBJ_KITE,
    BASEBALL_BAT,
    OBJ_BASEBALL_GLOVE, OBJ_SKATEBOARD, OBJ_SURFBOARD,
    OBJ_TENNIS_RACKET,
    OBJ_BOTTLE, OBJ_WINE_GLASS, OBJ_CUP, OBJ_FORK, OBJ_KNIFE, OBJ_SPOON,
    OBJ_BOWL,
    OBJ_BANANA, OBJ_APPLE, OBJ_SANDWICH, OBJ_ORANGE, OBJ_BROCCOLI, OBJ_CARROT,
    OBJ_HOT_DOG,
    OBJ_PIZZA, OBJ_DONUT, OBJ_CAKE,
    OBJ_CHAIR, OBJ_SOFA, OBJ_POTTEDPLANT, OBJ_BED, OBJ_DININGTABLE, OBJ_TOILET,
    OBJ_TVMONITOR, OBJ_LAPTOP, OBJ_MOUSE, OBJ_REMOTE, OBJ_KEYBOARD,
    OBJ_CELL_PHONE,
    OBJ_MICROWAVE, OBJ_OVEN, OBJ_TOASTER, OBJ_SINK, OBJ_REFRIGERATOR,
    OBJ_BOOK, CLOCK, OBJ_VASE, OBJ_SCISSORS, OBJ_TEDDY_BEAR, OBJ_HAIR_DRIER,
    OBJ_TOOTHBRUSH,
    OBJ_ALL = 0xFF, OBJ_OTHERS = -1
  } OBJ_CATEGORY_ENUM;
}   // namespace  OT_INFO_IF

struct ObjectTrackingInfo {
  /*
  * tracking box [left, top, right, bottom]
  */
  int32_t i4TrackingBox[4];

  /*
  * tracking smooth box [left, top, right, bottom]
  */
  int32_t i4TrackingSmoothBox[4];

  /*
  * tracking object confidence value (low:0.0 ~ high:1.0)
  */
  float fTrustValue;

  /*
  * tracking object category
  */
  OT_INFO_IF::OBJ_CATEGORY_ENUM i4ObjType;

  /*
  * time stamp map to tracking object information
  */
  uint64_t u8image_timestamp;

  /*
  * Frame number map to tracking object information
  */
  int32_t i4FrameNum;
};


#endif  // INCLUDE_MTKCAM_INTERFACES_FEATURE_OBJCETTRACKING_IOBJTRACKINGINFO_H_
