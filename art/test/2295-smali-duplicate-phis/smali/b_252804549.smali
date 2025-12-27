#
# Copyright (C) 2025 The Android Open Source Project
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
.class public L454322107;
.super Ljava/lang/Object;

.method public static foo()V
   .registers 4
   const/4 v0, 0x0
   const/4 v1, 0x0
   :loop
   const/4 v2, 0x1
   if-lt v1, v2, :move_0
   move v3, v2
   goto :skip
   :move_0
   const/4 v3, 0x0
   :skip
   move v0, v3
   if-nez v3, :end
   mul-float/2addr v1, v0
   move v1, v0
   goto :loop
   :end
   return-void
.end method
