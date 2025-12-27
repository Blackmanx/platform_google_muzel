; Copyright (C) 2024 The Android Open Source Project
;
; Licensed under the Apache License, Version 2.0 (the "License");
; you may not use this file except in compliance with the License.
; You may obtain a copy of the License at
;
;      http://www.apache.org/licenses/LICENSE-2.0
;
; Unless required by applicable law or agreed to in writing, software
; distributed under the License is distributed on an "AS IS" BASIS,
; WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
; See the License for the specific language governing permissions and
; limitations under the License.

.class public GetInstanceFieldOnUninitializedThis
.super java/lang/Object

.field public intField I

.method public <init>()V
   .limit stack 2
   .limit locals 1
   aload_0
   getfield GetInstanceFieldOnUninitializedThis/intField I
   aload_0
   invokespecial java/lang/Object.<init>()V
   ; Avoid `getfield` elimination (DCE) by the dexer.
   aload_0
   swap
   putfield GetInstanceFieldOnUninitializedThis/intField I
   return
.end method

