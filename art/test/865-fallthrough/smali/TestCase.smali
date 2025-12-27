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
#

.class public final LTestCase;
.super Ljava/lang/Object;

.method public constructor <init>()V
  .registers 1
  invoke-direct {p0}, Ljava/lang/Object;-><init>()V
  return-void
.end method

.method public test(I)Ljava/lang/Object;
  .registers 3
  if-eqz p1, :call_gc
  move p0, p1
  const v0, 0
  return-object v0
  # We used to merge register masks in the fast compiler at this point
  # for the next instruction, which would erroneously mark p0 as a non-object.
:call_gc
  invoke-static {}, LMain;->invokeGc()V
  return-object p0
.end method
