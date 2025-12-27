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
.class public LB446169228;
.super Ljava/lang/Object;

.method public static pokeByte12345678(J)V
    .registers 3
    const v0, 0x12345678
    invoke-static {p0, p1, v0}, Llibcore/io/Memory;->pokeByte(JB)V
    return-void
.end method

.method public static pokeShort12345678(JZ)V
    .registers 4
    const v0, 0x12345678
    invoke-static {p0, p1, v0, p2}, Llibcore/io/Memory;->pokeShort(JSZ)V
    return-void
.end method
