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

#ifndef INCLUDE_MTKCAM_CORE_HW_IMGSTREAM_EIGHTCC_H_
#define INCLUDE_MTKCAM_CORE_HW_IMGSTREAM_EIGHTCC_H_

#include <cstdint>

class EIGHTCC {
 public:
  EIGHTCC() {}

  template <unsigned int N>
  explicit EIGHTCC(const char (&s)[N]) {
    static_assert(N <= 8, "EIGHTCC arg exceeds 8 bytes");
    char* ptr = reinterpret_cast<char*>(&mData);
    for (unsigned int i = 0; i < N; ++i) {
      ptr[i] = s[i];
    }
    ptr[7] = '\0';
  }

  operator bool() const {
    return !!mData;
  }
  bool isValid() const {
    return !!mData;
  }

  const char* c_str() const { return (const char*)&mData; }

  uint64_t u64() const { return mData; }

 private:
  uint64_t mData = 0;
};

#endif  // INCLUDE_MTKCAM_CORE_HW_IMGSTREAM_EIGHTCC_H_
