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

public class StaticGet {

    private int instanceField;

    private static void unreachable() {
        throw new AssertionError("unreachable!");
    }

    @ConstantMethodHandle(
        kind = ConstantMethodHandle.STATIC_GET,
        owner = "StaticGet",
        fieldOrMethodName = "instanceField",
        descriptor = "I")
    private static MethodHandle forInstanceField() {
        unreachable();
        return null;
    }

    @ConstantMethodHandle(
        kind = ConstantMethodHandle.STATIC_GET,
        owner = "Main",
        fieldOrMethodName = "privateField",
        descriptor = "I")
    private static MethodHandle inaccessibleInstanceField() {
        unreachable();
        return null;
    }

    @ConstantMethodHandle(
        kind = ConstantMethodHandle.STATIC_GET,
        owner = "Main",
        fieldOrMethodName = "PRIVATE_STATIC_FIELD",
        descriptor = "I")
    private static MethodHandle inaccessibleStaticField() {
        unreachable();
        return null;
    }

    public static void runTests() {
        try {
            forInstanceField();
            unreachable();
        } catch (IncompatibleClassChangeError expected) {}

        try {
            inaccessibleInstanceField();
            unreachable();
        } catch (IncompatibleClassChangeError expected) {}

        try {
            inaccessibleStaticField();
            unreachable();
        } catch (IncompatibleClassChangeError expected) {}
    }

}
