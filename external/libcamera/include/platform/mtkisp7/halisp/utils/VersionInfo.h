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

#ifndef AAA_ISPHAL_INCLUDE_ISPHAL_UTILS_VERSIONINFO_H_
#define AAA_ISPHAL_INCLUDE_ISPHAL_UTILS_VERSIONINFO_H_
/*
 * This file describes the version information of Mediatek ISP framework, the
 * version information is followed Semantic Versioning[1], and the detail
 * release information would be recorded here too.
 *
 * Reference:
 *   [1]: Semantic Versioning v2.0.0, https://semver.org/
 */

namespace mtk {
namespace isphal {

enum Version {
    ISPHAL_VERSION_UNDEF    = 0,
    ISPHAL_VERSION_1_0      = 0x00010000,
};


}  // namespace isphal
}  // namespace mtk

#endif  // AAA_ISPHAL_INCLUDE_ISPHAL_UTILS_VERSIONINFO_H_

