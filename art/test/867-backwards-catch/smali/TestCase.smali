# Copyright (C) 2025 The Android Open Source Project
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#      http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

.class public LTestCase;
.super Ljava/lang/Object;

.method public constructor <init>()V
  .registers 1
  invoke-direct {p0}, Ljava/lang/Object;-><init>()V
  return-void
.end method


.method public test()LTestCase;
  .registers 1
  goto :try_start
  :entry
  invoke-static {}, LTestCase;->empty()V
  return-object p0
  :try_start
  return-object p0
  :try_end
  .catchall {:try_start .. :try_end} :entry
.end method

.method public static empty()V
  .registers 0
  return-void
.end method
