/*
 * Copyright (C) 2025 The Android Open Source Project
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

#ifndef ART_RUNTIME_VIRTUAL_THREAD_COMMON_H_
#define ART_RUNTIME_VIRTUAL_THREAD_COMMON_H_

#include "base/macros.h"
#include "mirror/object.h"
#include "obj_ptr.h"

namespace art HIDDEN {

// LINT.IfChange
enum PinningReason {
  kNoReason = 0,
  kCriticalSection = 2,
  kNativeMethod = 3,
  kMonitor = 4,
  kUnsupportedFrame = 5,
};
// Update Continuation.Pinned and Continuation.pinnedReason(int)
// This lint check doesn't work across 2 git projects until b/154647410 is fixed.
// LINT.ThenChange(../../libcore/ojluni/src/main/java/jdk/internal/vm/Continuation.java)

/**
 * @return true if parking is successful. False if the thread is pinned, or fails to park.
 */
bool VirtualThreadPark(ObjPtr<mirror::Object> v_context,
                       ObjPtr<mirror::Object> parked_states,
                       ObjPtr<mirror::Throwable> vm_error,
                       bool is_continuation_api,
                       PinningReason& reason_) REQUIRES_SHARED(Locks::mutator_lock_);

}  // namespace art

#endif  // ART_RUNTIME_VIRTUAL_THREAD_COMMON_H_
