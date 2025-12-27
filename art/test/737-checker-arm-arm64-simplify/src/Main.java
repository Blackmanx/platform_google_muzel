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
    public static void main(String args[]) {
        $noinline$regressionTestB434662092();
    }

    public static void $noinline$regressionTestB434662092() {
        int[] array = new int[1];
        $noinline$fillArray(array, 1, 42, 80, 7);
        assertEquals(3, array[0]);
        try {
            $noinline$fillArray(array, 2, 41, 80, 7);
        } catch (ArrayIndexOutOfBoundsException expected) {}
        assertEquals(5, array[0]);
    }

    /// CHECK-START: void Main.$noinline$fillArray(int[], int, int, int, int) BCE (after)
    /// CHECK:              Deoptimize

    /// CHECK-START-ARM: void Main.$noinline$fillArray(int[], int, int, int, int) instruction_simplifier_arm (after)
    /// CHECK: <<Shl:i\d+>> Shl
    /// CHECK:              Sub [<<Shl>>,{{i\d+}}]

    /// CHECK-START-ARM64: void Main.$noinline$fillArray(int[], int, int, int, int) instruction_simplifier_arm64 (after)
    /// CHECK: <<Shl:i\d+>> Shl
    /// CHECK:              Sub [<<Shl>>,{{i\d+}}]
    public static void $noinline$fillArray(int[] array, int n, int x, int y, int z) {
        int a = (x << 1) - y;
        for (int i = 0; i < n; ++i) {
            // The `b = z - a` was previously simplified as `z + (y - (x << 1))` to
            // allow using shifter operand. However, this left the environment use of
            // `a = Sub(x << 1, y)` in `Deoptimize` with exchanged operands, passing
            // wrong value to the deoptimization if `n > array.length`. b/434662092
            int b = z - a;
            array[i] = b;
        }
    }

    static void assertEquals(int expected, int actual) {
        if (expected != actual) {
            throw new AssertionError("Expected " + expected + " got " + actual);
        }
    }
}
