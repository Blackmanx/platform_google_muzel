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

import annotations.ConstantMethodType;
import annotations.ConstantMethodHandle;

import java.lang.invoke.MethodHandle;
import java.lang.invoke.MethodType;

public class Worker {

    private static void unreachable() {
        throw new Error("unreachable!");
    }

    private static void assertSame(Object lhs, Object rhs) {
        if (lhs != rhs) {
            throw new AssertionError(lhs + " is not equal to " + rhs);
        }
    }

    private static void assertNonNull(Object object) {
        if (object == null) {
            throw new AssertionError("object is null");
        }
    }

    public static String workerToString(Worker worker) {
        return worker.toString();
    }

    @ConstantMethodType(
        returnType = String.class,
        parameterTypes = {Worker.class})
    private static MethodType methodType() {
        unreachable();
        return null;
    }

    @ConstantMethodType(
        returnType = String.class,
        parameterTypes = {})
    private static MethodType returnStringMethodType() {
        unreachable();
        return null;
    }

    @ConstantMethodHandle(
        kind = ConstantMethodHandle.INVOKE_STATIC,
        owner = "Worker",
        fieldOrMethodName = "workerToString",
        descriptor = "(LWorker;)Ljava/lang/String;")
    private static MethodHandle methodHandle() {
        unreachable();
        return null;
    }

    public static void doWork() {
        System.out.println("doWork()");
        MethodType methodType = methodType();
        MethodHandle methodHandle = methodHandle();

        assertNonNull(methodType);
        assertNonNull(methodHandle);
        assertSame(methodType, methodHandle.type());

        assertNonNull(returnStringMethodType());
    }
}
