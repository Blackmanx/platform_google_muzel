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

#ifndef INCLUDE_MTKCAM_HALIF_DEF_TYPEMANIP_H_
#define INCLUDE_MTKCAM_HALIF_DEF_TYPEMANIP_H_


namespace NSCam {

/**
 * Int2Type<v> converts an integral constant v into a unique type.
 *  @tparam v The given integer as a constant to `Int2Type::value`.
 */
template <int v>
struct Int2Type {
  /** Helper constant `value` as the given template argument `v`. */
  enum { value = v };
};

/**
 * Type2Type<T> converts a type T into the unique type.
 *  @tparam T The given type T as the unique type to `Type2Type::OriginalType`.
 */
template <typename T>
struct Type2Type {
  /**
   *Helper type `OriginalType` as the given type from template argument `T`.
   */
  typedef T OriginalType;
};

}       // namespace NSCam
#endif  // INCLUDE_MTKCAM_HALIF_DEF_TYPEMANIP_H_
