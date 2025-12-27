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

#ifndef INCLUDE_MTKCAM_HALIF_DEF_BASICTYPES_H_
#define INCLUDE_MTKCAM_HALIF_DEF_BASICTYPES_H_

#include "BuiltinTypes.h"

namespace NSCam {

/** Rational */
struct MRational {
  MINT32 numerator = 0;    ///< numberator
  MINT32 denominator = 1;  ///< denominator

  /** Constructs default rational 0/1 */
  MRational() = default;

  /**
   * Constructs a rational with numerator and denominator.
   *  @param _numerator Numberator.
   *  @param _denominator Denominator.
   */
  MRational(MINT32 _numerator, MINT32 _denominator)
      : numerator(_numerator), denominator(_denominator) {}
};

/**
 * Sensor type enumeration.
 */
namespace NSSensorType {
enum Type {
  eUNKNOWN = 0xFFFFFFFF, /*!< Unknown */
  eRAW = 0,              /*!< RAW */
  eYUV,                  /*!< YUV */
};
}

/**
 * Make a compile-time constant of 8-character code.
 *
 * For example:
 *  @code
 *    MAKE8CC<'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h'>::value
 *  @endcode
 */
template <char c0 = ' ', char c1 = ' ', char c2 = ' ', char c3 = ' ',
          char c4 = ' ', char c5 = ' ', char c6 = ' ', char c7 = ' '>
struct MAKE8CC {
  /** The compile-time constant (constexpr) of 8-character value. */
  static const uint64_t value =
      (static_cast<uint64_t>(c0) << 0)
    | (static_cast<uint64_t>(c1) << 8)
    | (static_cast<uint64_t>(c2) << 16)
    | (static_cast<uint64_t>(c3) << 24)
    | (static_cast<uint64_t>(c4) << 32)
    | (static_cast<uint64_t>(c5) << 40)
    | (static_cast<uint64_t>(c6) << 48)
    | (static_cast<uint64_t>(c7) << 56);
};


}       // namespace NSCam
#endif  // INCLUDE_MTKCAM_HALIF_DEF_BASICTYPES_H_
