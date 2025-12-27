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

#ifndef AAA_ISPHAL_INCLUDE_ISPHAL_IHALISPFACTORY_H_
#define AAA_ISPHAL_INCLUDE_ISPHAL_IHALISPFACTORY_H_

#include "utils/VersionInfo.h"  // mtk::isphal::Version

#include <string>  // std::string

namespace mtk {
namespace isphal {

/**
 * IHalIspFactory is a factory which provides static methods to query version
 * info, and declare some virtual functions that derived classes must implement
 * them.
 */
class IHalIspFactory {
 public:
    /**
     * Query supported version count.
     */
    static size_t querySupportedVersionCount();

    /**
     * Checks the given version was supported or not.
     *  @return True for supported, otherwise not supported.
     */
    static bool isSupported(mtk::isphal::Version ver);

 public:
    /**
     * Query the current ISP hal version, derived classes have to implement
     * this method.
     */
    virtual mtk::isphal::Version queryVersion() const = 0;

    /**
     * Query the major version number.
     *  @return Major version.
     */
    virtual int queryMajorVersion() const = 0;

    /**
     * Query the minor version number.
     *  @return Minor version.
     */
    virtual int queryMinorVersion() const = 0;

    /**
     * Query the patch version number.
     *  @return Patch version.
     */
    virtual int queryPatchVersion() const = 0;

    /**
     * Query the version information. The return version string would be
     * "MAJOR.MINOR.PATCH, for example: 1.10.5
     *  @return Ascii standard string contains version info.
     */
    virtual std::string queryVersionString() const = 0;

 public:
    virtual ~IHalIspFactory() = default;
};  // class IHalIspFactory

}  // namespace isphal
}  // namespace mtk
#endif  // AAA_ISPHAL_INCLUDE_ISPHAL_IHALISPFACTORY_H_

