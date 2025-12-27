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

#ifndef INCLUDE_MTKCAM_INTERFACES_DEF_COMMON_H_
#define INCLUDE_MTKCAM_INTERFACES_DEF_COMMON_H_
/******************************************************************************
 *
 ******************************************************************************/
//
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <mutex>
//
#include "Errors.h"
#include "BuiltinTypes.h"
#include "BasicTypes.h"
#include "UITypes.h"
#include "TypeManip.h"
//
#include "ImageFormat.h"
#include "Modes.h"
#include "CameraInfo.h"
#include "mtkcam-halif/utils/metadata/1.x/IMetadata.h"
//
/******************************************************************************
 *
 ******************************************************************************/
class MeteringROIProvider {
    private:
        MeteringROIProvider() {}

    public :
        static MeteringROIProvider* getInstance() {
            static MeteringROIProvider instance;
            return &instance;
        }
    private :
        MINT32 mMeteringMode = 0;
        MINT32 mCropEnable = 0;
        NSCam::MRect mP1Crop;
        std::vector<MINT32> mAeRegion;
        std::mutex mlock;
    public :
        inline void setCrop(std::shared_ptr<NSCam::MRect> p1Crop) {
            std::unique_lock<std::mutex> _l(mlock);
            if (p1Crop) {
                mCropEnable = 1;
                mP1Crop = *p1Crop;
            } else {
                mCropEnable = 0;
            }
        }
        inline void getCrop(NSCam::MRect& crop) {
            std::unique_lock<std::mutex> _l(mlock);
            if (mCropEnable)
                crop = mP1Crop;
        }
        inline void setMeteringRoi(NSCam::IMetadata::IEntry const& entry) {
            std::unique_lock<std::mutex> _l(mlock);
            if (!entry.isEmpty()) {
                mMeteringMode = 1;
                mAeRegion.clear();
                auto count = entry.count();
                for (MUINT i = 0; i < count; i++) {
                    mAeRegion.push_back(entry.itemAt(i, NSCam::Type2Type<MINT32>()));
                }
            } else {
                mMeteringMode = 0;
            }
        }
        inline void getMeteringRoi(std::vector<MINT32>& roi) {
            std::unique_lock<std::mutex> _l(mlock);
            if (mMeteringMode != 0) {
                auto size = mAeRegion.size();
                for (size_t i = 0; i < size; i++) {
                    roi.push_back(mAeRegion[i]);
                }
            }
        }
};
#endif  // INCLUDE_MTKCAM_INTERFACES_DEF_COMMON_H_
