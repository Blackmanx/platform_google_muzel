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

public class Main {
    public static void main(String[] args) {
        $noinline$testReturnValueOrDefault();
    }

    private class InnerObject {
        int value;
    }

    static InnerObject inner_obj;

    // GVN removes the second `inner_obj` get.
    /// CHECK-START: int Main.$noinline$returnValueOrDefault() GVN (before)
    /// CHECK: StaticFieldGet field_name:Main.inner_obj
    /// CHECK: StaticFieldGet field_name:Main.inner_obj

    /// CHECK-START: int Main.$noinline$returnValueOrDefault() GVN (after)
    /// CHECK:     StaticFieldGet field_name:Main.inner_obj
    /// CHECK-NOT: StaticFieldGet field_name:Main.inner_obj

    // Consistency check: No BoundTypes existed before.
    /// CHECK-START: int Main.$noinline$returnValueOrDefault() reference_type_propagation$after_gvn (before)
    /// CHECK-NOT: BoundType

    // Consistency check: Only one null check.
    /// CHECK-START: int Main.$noinline$returnValueOrDefault() reference_type_propagation$after_gvn (before)
    /// CHECK:     NullCheck
    /// CHECK-NOT: NullCheck

    // RTP will add a BoundType between the `StaticFieldGet` and `NullCheck`.

    /// CHECK-START: int Main.$noinline$returnValueOrDefault() reference_type_propagation$after_gvn (before)
    /// CHECK: <<SFG:l\d+>> StaticFieldGet field_name:Main.inner_obj
    /// CHECK:              NullCheck [<<SFG>>]

    /// CHECK-START: int Main.$noinline$returnValueOrDefault() reference_type_propagation$after_gvn (after)
    /// CHECK: <<SFG:l\d+>> StaticFieldGet field_name:Main.inner_obj
    /// CHECK: <<BT:l\d+>>  BoundType [<<SFG>>]
    /// CHECK:              NullCheck [<<BT>>]

    // Finally, InstructionSimplifier can remove the NullCheck

    /// CHECK-START: int Main.$noinline$returnValueOrDefault() instruction_simplifier$after_gvn (before)
    /// CHECK:     NullCheck
    /// CHECK-NOT: NullCheck

    /// CHECK-START: int Main.$noinline$returnValueOrDefault() instruction_simplifier$after_gvn (after)
    /// CHECK-NOT: NullCheck
    static int $noinline$returnValueOrDefault() {
        if (inner_obj != null) {
            return inner_obj.value;
        } else {
            return -123;
        }
    }

    static void $noinline$testReturnValueOrDefault() {
        inner_obj = null;
        $noinline$assertIntEquals(-123, $noinline$returnValueOrDefault());

        Main m = new Main();
        inner_obj = m.new InnerObject();
        inner_obj.value = 0;
        $noinline$assertIntEquals(0, $noinline$returnValueOrDefault());

        inner_obj.value = 1000;
        $noinline$assertIntEquals(1000, $noinline$returnValueOrDefault());
    }

    public static void $noinline$assertIntEquals(int expected, int result) {
        if (expected != result) {
            throw new Error("Expected: " + expected + ", found: " + result);
        }
    }
}
