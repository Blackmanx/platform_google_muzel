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

public class InvokeSpecial {

    private int instanceField;

    private static void unreachable() {
        throw new AssertionError("unreachable!");
    }

    public void method() {}

    @ConstantMethodHandle(
        kind = ConstantMethodHandle.INVOKE_SPECIAL,
        owner = "InvokeSpecial",
        fieldOrMethodName = "unreachable",
        descriptor = "()V")
    private static MethodHandle forStaticMethod() {
        unreachable();
        return null;
    }

    @ConstantMethodHandle(
        kind = ConstantMethodHandle.INVOKE_SPECIAL,
        owner = "java/util/List",
        fieldOrMethodName = "size",
        descriptor = "()I")
    private static MethodHandle forInterfaceMethod() {
        unreachable();
        return null;
    }

    @ConstantMethodHandle(
        kind = ConstantMethodHandle.INVOKE_SPECIAL,
        owner = "Main",
        fieldOrMethodName = "instanceMethod",
        descriptor = "()V")
    private static MethodHandle inaccessiblePrivateMethod() {
        unreachable();
        return null;
    }

    @ConstantMethodHandle(
        kind = ConstantMethodHandle.INVOKE_SPECIAL,
        owner = "Main",
        fieldOrMethodName = "staticMethod",
        descriptor = "()V")
    private static MethodHandle inaccessibleStaticMethod() {
        unreachable();
        return null;
    }

    public static void runTests() {
        try {
            forStaticMethod();
            unreachable();
        } catch (IncompatibleClassChangeError expected) {}

        // TODO(b/297147201): runtime crashes here.
        /*
        try {
            forInterfaceMethod();
            unreachable();
        } catch (IncompatibleClassChangeError expected) {}
        */

        // TODO(b/297147201): runtime does not throw exception here.
        /*
        try {
            InvokeSpecialForConstructor.runTests();
            unreachable();
        } catch (IncompatibleClassChangeError | ClassFormatError expected) {}
        */

        try {
            InvokeSpecialForClassInitializer.runTests();
            unreachable();
        } catch (IncompatibleClassChangeError | ClassFormatError expected) {}

        try {
            inaccessibleStaticMethod();
            unreachable();
        } catch (IncompatibleClassChangeError expected) {}
    }

}
