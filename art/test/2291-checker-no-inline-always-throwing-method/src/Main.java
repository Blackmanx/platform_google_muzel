/*
 * Copyright (C) 2025 The Android Open Source Project
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

public class Main {
    public static void main(String[] args) throws Error {
        try {
            $noinline$testDivideUnsignedLong();
            throw new Error("Was expecting an ArithmeticException");
        } catch (ArithmeticException expected) {
        }
    }

    // divideAndThrow shouldn't be inlined as it ends in an always-throwing instruction.

    /// CHECK-START: void Main.$noinline$testDivideUnsignedLong() inliner (before)
    /// CHECK: InvokeStaticOrDirect method_name:Main.divideAndThrow

    /// CHECK-START: void Main.$noinline$testDivideUnsignedLong() inliner (after)
    /// CHECK: InvokeStaticOrDirect method_name:Main.divideAndThrow
    private static void $noinline$testDivideUnsignedLong() {
        divideAndThrow();
    }

    /// CHECK-START: void Main.divideAndThrow() dead_code_elimination$initial (before)
    /// CHECK:     InvokeStaticOrDirect method_name:Main.assertLongEquals

    /// CHECK-START: void Main.divideAndThrow() dead_code_elimination$initial (after)
    /// CHECK-NOT: InvokeStaticOrDirect method_name:Main.assertLongEquals

    /// CHECK-START: void Main.divideAndThrow() inliner (after)
    /// CHECK:     InvokeStaticOrDirect method_name:java.lang.Long.divideUnsigned always_throws:true
    private static void divideAndThrow() {
        long dividend = 0x12345678L;
        long divisor = 0x0L;
        // Divide by zero!
        long result = Long.divideUnsigned(dividend, divisor);
        // The following lines will not be reached.
        assertLongEquals(0xdeadbeef, result);
    }

    private static void assertLongEquals(long expected, long result) {
        if (expected != result) {
            throw new Error("Expected: " + expected + ", found: " + result);
        }
    }
}
