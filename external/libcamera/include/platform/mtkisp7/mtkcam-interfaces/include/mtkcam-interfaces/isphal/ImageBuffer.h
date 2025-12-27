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

#ifndef INCLUDE_MTKCAM_INTERFACES_ISPHAL_IMAGEBUFFER_H_
#define INCLUDE_MTKCAM_INTERFACES_ISPHAL_IMAGEBUFFER_H_

#include <mtkcam-interfaces/isphal/Predefines.h>
#include <mtkcam-interfaces/isphal/Buffer.h>

#include <array>  // std::array

namespace mtk {
namespace isphal {

enum ImageDomain {
    kImageDomainUnknown,
    kImageDomainRayer,
    kImageDomainRGB,
    kImageDomainYUV
};

class ImageBuffer {
 public:
    /** Default constructor */
    ImageBuffer();

    /**
     * To construct a single plane image buffer.
     * If the given information is invalid, the ImageBuffer
     * would be an empty ImageBuffer.
     *  @param w    Width in pixel.
     *  @param h    Height in pixel.
     *  @param buf  The given Buffer instance.
     */
    explicit ImageBuffer(size_t w, size_t h,
        size_t bit_depth,
        ImageDomain domain,
        const Buffer& buf);

    /**
     * To construct a null image buffer.
     * with an empty ImageBuffer.
     *  @param w    Width in pixel.
     *  @param h    Height in pixel.
     */
    explicit ImageBuffer(size_t w, size_t h,
        size_t bit_depth,
        ImageDomain domain);

    virtual ~ImageBuffer() = default;

 public:  // copyable & movable
    ImageBuffer(const ImageBuffer&) = default;
    ImageBuffer(ImageBuffer&&) = default;
    ImageBuffer& operator=(const ImageBuffer&) = default;
    ImageBuffer& operator=(ImageBuffer&&) = default;

 public:
    Buffer getBuffer() const;
    Buffer& getRefBuffer();
    void set(size_t w, size_t h, size_t bit_depth, ImageDomain domain);
    int getVa() const;
    int getFd() const;
    size_t getBufferSize() const;
    size_t getWidth() const;
    size_t getHeight() const;
    size_t getBitDepth() const;
    ImageDomain getImageDomain() const;

    operator bool() const;

 public:
    bool operator==(const ImageBuffer& rhs) const;
    bool operator!=(const ImageBuffer& rhs) const;

 protected:
    Buffer m_buffer;  // buffers

    size_t m_width;  // unit: pixel
    size_t m_height;  // unit: pixel
    size_t m_bit_depth;
    ImageDomain m_image_domain;
};

}   // namespace isphal
}   // namespace mtk

#endif  // INCLUDE_MTKCAM_INTERFACES_ISPHAL_IMAGEBUFFER_H_

