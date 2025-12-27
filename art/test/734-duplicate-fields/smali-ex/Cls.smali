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

.class public LCls;
.super Ljava/lang/Object;

.method public constructor <init>()V
.registers 2
    invoke-direct {p0}, Ljava/lang/Object;-><init>()V
    const/16 v0, 0x42
    iput v0, p0, LCls;->myField:I
    return-void
.end method

.field public myField:I
.field public static myField:I = 0x1
