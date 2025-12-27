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

.class public LUnresolvedMergedArrayPutIntAsObject;
.super Ljava/lang/Object;

.field static intField:I

.method public static test()V
   .registers 3
   const/4 v0, 0x0
   sget v2, LUnresolvedMergedArrayPutIntAsShort;->intField:I
   if-eqz v2, :else1
   new-array v1, v0, [LUnresolved;
   goto :merge1
   :else1
   new-array v1, v0, [LMain;
   :merge1
   # We merge an unresolved array reference with a resolved array reference. Note that
   # merging two unresolved array references currently results in `j.l.Object` (non-array)
   # reference which would lead to rejecting the `aput-object` via a different path.

   # Hide the tested `aput-object` under another `if`. Otherwise, it would be processed first
   # on the plain unresolved array reference and rejected before the verifier creates the
   # merged unresolved array reference which we want to test.
   sget v2, LUnresolvedMergedArrayPutIntAsShort;->intField:I
   if-eqz v2, :else2
   # Nothing.
   goto :merge2
   :else2
   # Even with the array being unresolved merged array, storing `int` as object should be rejected.
   aput-object v2, v1, v0
   :merge2
   return-void
.end method
