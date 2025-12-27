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

#ifndef INCLUDE_MTKCAM_INTERFACES_ISPHAL_BUFFER_H_
#define INCLUDE_MTKCAM_INTERFACES_ISPHAL_BUFFER_H_

#include <mtkcam-interfaces/isphal/Predefines.h>
#include <mtkcam-halif/def/ImageFormat.h>

#include <memory>

namespace mtk {
namespace isphal {

/**
 * A basic buffer class to store a buffer information, including
 * VA (virtual address), PA (physical address), FD (file descriptor), data size,
 * and buffer size. This class has no ability to enlarge, reallocate or free
 * buffer, but just an interface or says information container of a buffer.
 *  @note: This class is reentrant but not thread-safe.
 *  @revision history:
 *      v1.0.0: First release version
 */
class Buffer {
  /* constructors / destructor */
 public:
  Buffer();
  explicit Buffer(intptr_t va, int fd, size_t offset, size_t buf_size,
    NSCam::EImageFormat format = NSCam::EImageFormat::eImgFmt_UNKNOWN);
  explicit Buffer(intptr_t va, int fd, size_t offset, size_t buf_size,
    NSCam::EImageFormat format, size_t width, size_t height,
    size_t stride_in_bytes = 0);
  virtual ~Buffer();

  /* methods */
 public:
  /**
   * Checks if the buffer contains no data.
   *  @return true for no data.
   */
  virtual bool isEmpty() const;

  /**
   * Get the virtual address.
   *  @return The stored virtual address.
   */
  virtual intptr_t getVa() const;

  /**
   * Get the file descriptor, for pure software buffers (such as an buffer
   * allocated by operator::new, or malloc), this value would be 0.
   *  @return The stored file descriptor.
   */
  virtual int getFd() const;

  /**
   * Get the buffer size, the buffer size indicates to the real buffer size,
   * maybe different to data size.
   *  @return The stored buffer size in bytes.
   */
  virtual size_t getBufferSize() const;

  /**
   * Get the buffer format, the buffer format in image buffer,
   * which enque to driver.
   *  @return The buffer format about this buffer.
   */
  virtual NSCam::EImageFormat getBufferFormat() const;
  /**
   * Get the buffer size, the buffer size indicates to the real buffer size,
   * maybe different to data size.
   *  @return The stored buffer size in bytes.
   */
  virtual size_t getWidth() const;
  /**
   * Get the buffer size, the buffer size indicates to the real buffer size,
   * maybe different to data size.
   *  @return The stored buffer size in bytes.
   */
  virtual size_t getHeight() const;

  /**
   * Get the buffer stride, the buffer size indicates to the real buffer sride,
   *  @return The stored buffer stride size in bytes.
   */
  virtual size_t getStride() const;

  /**
   * Get the buffer offset, the buffer offset indicates the offset from
   * the start of the memory for this buffer.
   *  @return The stored buffer offset in bytes.
   */
  virtual size_t getOffset() const;

  /**
   * Reset the Buffer to the given target, if the given target is nullptr,
   * clear the Buffer to an empty Buffer.
   *  @param p_target Target of Buffer to be reset (copy).
   */
  virtual void reset(const Buffer* p_target = nullptr);

  /* operators */
 public:
  virtual bool operator==(const Buffer& rhs) const;
  virtual bool operator!=(const Buffer& rhs) const;

  /**
   * Operator bool is to verify if theres any data in buffer.
   */
  virtual operator bool() const;

  /* attributes */
 protected:
  intptr_t m_va;
  int m_fd;
  size_t m_offset;
  size_t m_buffersize;
  NSCam::EImageFormat m_format;
  size_t m_width;
  size_t m_height;
  size_t m_stride_in_bytes;
  bool m_2dimension;
};

}   // namespace isphal
}   // namespace mtk

#endif  // INCLUDE_MTKCAM_INTERFACES_ISPHAL_BUFFER_H_
