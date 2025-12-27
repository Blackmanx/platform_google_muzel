/*
 * Copyright (C) 2023 The Android Open Source Project
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

#include "thread.h"

#include <sys/resource.h>

#include "android-base/logging.h"
#include "base/locks.h"
#include "base/mutex.h"
#include "common_runtime_test.h"
#include "mirror/object.h"
#include "scoped_thread_priority_change.h"
#include "thread-current-inl.h"
#include "thread-inl.h"
#include "well_known_classes-inl.h"

namespace art HIDDEN {

class ThreadTest : public CommonRuntimeTest {};

// Ensure that basic list operations on ThreadExitFlags work. These are rarely
// exercised in practice, since normally only one flag is registered at a time.

TEST_F(ThreadTest, ThreadExitFlagTest) {
  Thread* self = Thread::Current();
  ThreadExitFlag tefs[3];
  {
    MutexLock mu(self, *Locks::thread_list_lock_);
    self->NotifyOnThreadExit(&tefs[2]);
    ASSERT_TRUE(self->IsRegistered(&tefs[2]));
    ASSERT_FALSE(tefs[2].HasExited());
    ASSERT_FALSE(self->IsRegistered(&tefs[1]));
    self->NotifyOnThreadExit(&tefs[1]);
    self->NotifyOnThreadExit(&tefs[0]);
    ASSERT_TRUE(self->IsRegistered(&tefs[0]));
    ASSERT_TRUE(self->IsRegistered(&tefs[1]));
    ASSERT_TRUE(self->IsRegistered(&tefs[2]));
    self->UnregisterThreadExitFlag(&tefs[1]);
    ASSERT_TRUE(self->IsRegistered(&tefs[0]));
    ASSERT_FALSE(self->IsRegistered(&tefs[1]));
    ASSERT_TRUE(self->IsRegistered(&tefs[2]));
    self->UnregisterThreadExitFlag(&tefs[2]);
    ASSERT_TRUE(self->IsRegistered(&tefs[0]));
    ASSERT_FALSE(self->IsRegistered(&tefs[1]));
    ASSERT_FALSE(self->IsRegistered(&tefs[2]));
  }
  Thread::DCheckUnregisteredEverywhere(&tefs[1], &tefs[2]);
  {
    MutexLock mu(self, *Locks::thread_list_lock_);
    self->UnregisterThreadExitFlag(&tefs[0]);
    ASSERT_FALSE(self->IsRegistered(&tefs[0]));
    ASSERT_FALSE(self->IsRegistered(&tefs[1]));
    ASSERT_FALSE(self->IsRegistered(&tefs[2]));
  }
  Thread::DCheckUnregisteredEverywhere(&tefs[0], &tefs[2]);
}

TEST_F(ThreadTest, ThreadExitSignalTest) {
  Thread* self = Thread::Current();
  ThreadExitFlag tefs[3];
  {
    MutexLock mu(self, *Locks::thread_list_lock_);
    self->NotifyOnThreadExit(&tefs[2]);
    ASSERT_TRUE(self->IsRegistered(&tefs[2]));
    ASSERT_FALSE(self->IsRegistered(&tefs[1]));
    self->NotifyOnThreadExit(&tefs[1]);
    ASSERT_TRUE(self->IsRegistered(&tefs[1]));
    self->SignalExitFlags();
    ASSERT_TRUE(tefs[1].HasExited());
    ASSERT_TRUE(tefs[2].HasExited());
  }
  Thread::DCheckUnregisteredEverywhere(&tefs[1], &tefs[2]);
  {
    MutexLock mu(self, *Locks::thread_list_lock_);
    self->NotifyOnThreadExit(&tefs[0]);
    tefs[2].~ThreadExitFlag();  //  Destroy and reinitialize.
    new (&tefs[2]) ThreadExitFlag();
    self->NotifyOnThreadExit(&tefs[2]);
    ASSERT_FALSE(tefs[0].HasExited());
    ASSERT_TRUE(tefs[1].HasExited());
    ASSERT_FALSE(tefs[2].HasExited());
    self->SignalExitFlags();
    ASSERT_TRUE(tefs[0].HasExited());
    ASSERT_TRUE(tefs[1].HasExited());
    ASSERT_TRUE(tefs[2].HasExited());
  }
  Thread::DCheckUnregisteredEverywhere(&tefs[0], &tefs[2]);
}

// Ensure that ScopedPriorityChange works correctly and interacts properly with
// GetNicenessBeforeBoost().
TEST_F(ThreadTest, ScopedPriorityChangeTest) {
  // This is disabled on VM and SBC because they do not emulate Android's more generous
  // setpriority() handling.
  TEST_DISABLED_ON_VM();
  TEST_DISABLED_ON_SBC();
#if defined(ART_TARGET_ANDROID)
  Thread* self = Thread::Current();
  ASSERT_FALSE(self == nullptr);
  self->TransitionFromSuspendedToRunnable();  // Start() releases mutator lock.
  bool started = Runtime::Current()->Start();
  CHECK(started);
  ScopedObjectAccess soa(self);
  mirror::Object* peer = self->GetPeer();
  ASSERT_FALSE(peer == nullptr);
  int initial_niceness = self->GetCachedNiceness();
  // Set java.lang.Thread cached niceness and Linux niceness to match.
  // O.w. ScopedPriorityChage does nothing.
  WellKnownClasses::java_lang_Thread_niceness->SetInt<false>(peer, 5);
  int ret = setpriority(PRIO_PROCESS, 0, 5);
  ASSERT_EQ(ret, 0);
  ASSERT_EQ(getpriority(PRIO_PROCESS, 0), 5);
  ASSERT_EQ(self->GetCachedNiceness(), 5);
  ASSERT_EQ(self->GetNicenessBeforeBoost(), Thread::kNotBoosted);
  {
    ScopedPriorityChange spc(self);
    ASSERT_EQ(getpriority(PRIO_PROCESS, 0), 5);
    ASSERT_EQ(self->GetNicenessBeforeBoost(), Thread::kNotBoosted);
    spc.SetToNormalOrBetter();
    ASSERT_EQ(getpriority(PRIO_PROCESS, 0), 0);
    ASSERT_EQ(self->GetNicenessBeforeBoost(), 5);
    {
      // Nested invocations have no effect.
      ScopedPriorityChange spc2(self);
      spc2.SetToNormalOrBetter();
      ASSERT_EQ(getpriority(PRIO_PROCESS, 0), 0);
      ASSERT_EQ(self->GetNicenessBeforeBoost(), 5);
    }
    ASSERT_EQ(getpriority(PRIO_PROCESS, 0), 0);
    ASSERT_EQ(self->GetNicenessBeforeBoost(), 5);
  }
  ASSERT_EQ(getpriority(PRIO_PROCESS, 0), 5);
  ASSERT_EQ(self->GetNicenessBeforeBoost(), Thread::kNotBoosted);
  WellKnownClasses::java_lang_Thread_niceness->SetInt<false>(peer, initial_niceness);
  ret = setpriority(PRIO_PROCESS, 0, initial_niceness);
  ASSERT_EQ(ret, 0);
#endif  // ART_TARGET_ANDROID
  // Else we are on host where we don't have permission to decrease niceness,
  // and thus can't effectively test.
}

}  // namespace art
