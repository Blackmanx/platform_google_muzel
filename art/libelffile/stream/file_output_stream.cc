/*
 * Copyright (C) 2013 The Android Open Source Project
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

#include "file_output_stream.h"

#include <android-base/logging.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include "base/unix_file/fd_file.h"

namespace art {

FileOutputStream::FileOutputStream(File* file) : OutputStream(file->GetPath()), file_(file) {}

bool FileOutputStream::WriteFully(const void* buffer, size_t byte_count) {
  return file_->WriteFully(buffer, byte_count);
}

off_t FileOutputStream::Seek(off_t offset, Whence whence) {
  static const bool allow_sparse_files = unix_file::AllowSparseFiles();
  // If we are not allowed to generate sparse files, write zeros instead.
  if (UNLIKELY(!allow_sparse_files)) {
    // Check the current file size.
    int fd = file_->Fd();
    struct stat sb;
    if (fstat(fd, &sb) == -1) {
      return -1;
    }
    off_t file_size = sb.st_size;
    // Calculate new desired offset.
    switch (whence) {
      case kSeekSet:
        break;
      case kSeekCurrent: {
        off_t curr_offset = lseek(fd, 0, SEEK_CUR);
        if (curr_offset == -1) {
          return -1;
        }
        offset += curr_offset;
        whence = kSeekSet;
        break;
      }
      case kSeekEnd:
        offset += file_size;
        whence = kSeekSet;
        break;
      default:
        LOG(FATAL) << "Unsupported seek type: " << whence;
        UNREACHABLE();
    }
    // Write zeros if we are extending the file.
    if (offset > file_size) {
      off_t curr_offset = lseek(fd, 0, SEEK_END);
      if (curr_offset == -1) {
        return -1;
      }
      static const std::array<uint8_t, 1024> buffer{};
      while (curr_offset < offset) {
        size_t size = std::min<size_t>(offset - curr_offset, buffer.size());
        ssize_t bytes_written = write(fd, buffer.data(), size);
        if (bytes_written < 0) {
          return -1;
        }
        curr_offset += bytes_written;
      }
    }
  }
  return lseek(file_->Fd(), offset, static_cast<int>(whence));
}

bool FileOutputStream::Flush() {
  return file_->Flush() == 0;
}

}  // namespace art
