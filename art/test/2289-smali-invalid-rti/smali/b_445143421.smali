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
.class public LB445143421;
.super Ljava/lang/Object;

.method public static main()V
    .registers 2
    const/4 v0, 0x0
    # This could have been a parameter or a real array. Let's use null for simplicity
    const/16 v1, 0x0

  :goto_311
    # v1 = v1[0]
    aget-object v1, v1, v0
    # v1[0] = 0
    aput v0, v1, v0
    goto :goto_311

    return-void
.end method
