/*
 * Copyright (C) 2024 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

import java.lang.invoke.MethodHandle;

import annotations.ConstantMethodHandle;

public class InvokeInstanceForConstructor {

    private static void unreachable() {
        throw new AssertionError("unreachable!");
    }

    @ConstantMethodHandle(
        kind = ConstantMethodHandle.INVOKE_VIRTUAL,
        owner = "InvokeInstanceForConstructor",
        fieldOrMethodName = "<init>",
        descriptor = "()V")
    private static MethodHandle forConstructor() {
        unreachable();
        return null;
    }

    public static void runTests() {
        forConstructor();
    }
}
