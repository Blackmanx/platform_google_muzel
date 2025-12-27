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

.class public LNullArrayGetIPutS;
.super Ljava/lang/Object;

.method public static test()V
   .registers 3
   const/4 v0, 0x0
   const/4 v1, 0x0
   # Note that the array is null, so this shall throw at runtime.
   # However, for verification, null is valid as both `int[]` and `short[]`
   # and primitive aput allows storing wider types in narrow array elements.
   aget v2, v1, v0
   aput-short v2, v1, v0
   return-void
.end method
