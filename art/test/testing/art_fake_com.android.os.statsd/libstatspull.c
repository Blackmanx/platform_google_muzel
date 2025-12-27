/*
 * Copyright (C) 2024 The Android Open Source Project
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

#include <stdlib.h>

#include "android/set_abort_message.h"

#define FAKE(method_name)               \
  void method_name() {                  \
    android_set_abort_message("Fake!"); \
    abort();                            \
  }

FAKE(AStatsManager_PullAtomMetadata_obtain);
FAKE(AStatsManager_PullAtomMetadata_release);
FAKE(AStatsManager_PullAtomMetadata_setCoolDownMillis);
FAKE(AStatsManager_PullAtomMetadata_getCoolDownMillis);
FAKE(AStatsManager_PullAtomMetadata_setTimeoutMillis);
FAKE(AStatsManager_PullAtomMetadata_getTimeoutMillis);
FAKE(AStatsManager_PullAtomMetadata_setAdditiveFields);
FAKE(AStatsManager_PullAtomMetadata_getNumAdditiveFields);
FAKE(AStatsManager_PullAtomMetadata_getAdditiveFields);
FAKE(AStatsEventList_addStatsEvent);
FAKE(AStatsManager_setPullAtomCallback);
FAKE(AStatsManager_clearPullAtomCallback);
FAKE(AStatsManager_addSubscription);
FAKE(AStatsManager_removeSubscription);
FAKE(AStatsManager_flushSubscription);
