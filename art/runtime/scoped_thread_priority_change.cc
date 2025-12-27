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

#include "scoped_thread_priority_change.h"

#include <sys/resource.h>

#include "base/mutex.h"
#include "com_android_libcore.h"
#include "thread-inl.h"

namespace art HIDDEN {

void ScopedPriorityChange::SetToNormalOrBetter() {
  static bool enabled = com::android::libcore::niceness_apis();
  if (!enabled) {
    return;
  }
  DCHECK(!priority_changed_);
  int cached_niceness = self_->GetCachedNiceness();
  if (cached_niceness > 0 /* excludes no Java peer case */) {
    int old_niceness = getpriority(PRIO_PROCESS, 0);
    if (old_niceness == cached_niceness) {
      // Reduce nicenesss to zero, increasing our priority.
      // Should succeed on Android; will probably fail elsewhere.
      int ret = setpriority(PRIO_PROCESS, 0, 0);
      if (ret == 0) {
        // Maintain old niceness in thread structure, so we can determine "real" niceness without
        // acquiring the mutator lock.
        self_->SetNicenessBeforeBoost(old_niceness);
        priority_changed_ = true;
      }
    } else if (old_niceness > 0) {
      // Priority increase is warranted, but cache is out of sync.
      LOG(WARNING) << "Cached niceness = " << cached_niceness
                   << "; actual niceness = " << old_niceness;
    }
  }
}

void ScopedPriorityChange::ResetInternal() {
  DCHECK(priority_changed_);
  DCHECK_GT(self_->GetNicenessBeforeBoost(), 0);
  DCHECK(com::android::libcore::niceness_apis());
  Locks::mutator_lock_->AssertSharedHeld(self_);
  self_->SetNicenessBeforeBoost(Thread::kNotBoosted);  // Do this even if getpriority() test
                                                       // fails.
  if (getpriority(PRIO_PROCESS, 0) == 0) {
    // We changed priority and nobody visibly altered it in the interim; change it to the current
    // cached value.
    int cached_niceness = self_->GetCachedNiceness();
    int old_cached_niceness;
    // We cannot acquire the Java monitor that normally protects the niceness field here, since that
    // would sometimes require us to suspend. We instead rely on the fact that any updates to the
    // niceness fields while the thread is running are followed by a setpriority() call with the
    // same niceness value.
    do {
      int ret = setpriority(PRIO_PROCESS, 0, cached_niceness);
      // Increasing our niceness should always work, especially since decreasing it did.
      CHECK_EQ(ret, 0) << strerror(errno);
      old_cached_niceness = cached_niceness;
      cached_niceness = self_->GetCachedNiceness();
      // If it changed in the interim, we may have restored an obsolete niceness value, and we
      // need to fix that. Otherwise any later updates will come with following setpriority()
      // calls, which will overwrite ours.
    } while (cached_niceness != old_cached_niceness);
  } else {
    LOG(WARNING) << "Priority change inside ScopedPriorityChange";
    // Do not restore priority, since somebody else changed it in the interim.
  }
}

}  // namespace art
