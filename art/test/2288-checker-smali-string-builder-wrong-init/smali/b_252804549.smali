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
.class public LB440704096;

.super Ljava/lang/Object;

## CHECK-START: java.lang.String B440704096.emptyString() prepare_for_register_allocation (after)
## CHECK-NOT: StringBuilderAppend
.method public static emptyString()Ljava/lang/String;
   .registers 2
   new-instance v0, Ljava/lang/StringBuilder;
   invoke-direct {v0}, Ljava/lang/Object;-><init>()V
   invoke-virtual {v0}, Ljava/lang/StringBuilder;->toString()Ljava/lang/String;
   move-result-object v0
   return-object v0
.end method

## CHECK-START: java.lang.String B440704096.appendSomething() prepare_for_register_allocation (after)
## CHECK-NOT: StringBuilderAppend
.method public static appendSomething()Ljava/lang/String;
   .registers 2
   new-instance v0, Ljava/lang/StringBuilder;
   invoke-direct {v0}, Ljava/lang/Object;-><init>()V
   const-string v1, "I am a string 12345"
   invoke-virtual {v0, v1}, Ljava/lang/StringBuilder;->append(Ljava/lang/String;)Ljava/lang/StringBuilder;
   move-result-object v0
   invoke-virtual {v0}, Ljava/lang/StringBuilder;->toString()Ljava/lang/String;
   move-result-object v0
   return-object v0
.end method
