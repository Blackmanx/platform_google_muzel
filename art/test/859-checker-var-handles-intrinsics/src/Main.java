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
import java.lang.invoke.MethodHandles;
import java.lang.invoke.VarHandle;

public class Main {
    public static void main(String[] args) {
        // Since the tests will share a value, we have the condition that the value starts
        // and ends as 0 before and after each test.

        // Int tests
        $noinline$testGetAndAdd_Int();
        $noinline$testGetAndSet_Int();
        $noinline$testGetAndBitwiseAnd_Int();
        $noinline$testGetAndBitwiseOr_Int();
        $noinline$testGetAndBitwiseXor_Int();

        // Long tests
        $noinline$testGetAndAdd_Long();
        $noinline$testGetAndSet_Long();
        $noinline$testGetAndBitwiseAnd_Long();
        $noinline$testGetAndBitwiseOr_Long();
        $noinline$testGetAndBitwiseXor_Long();

        // Float tests
        $noinline$testGetAndAdd_Float();
        $noinline$testGetAndSet_Float();

        // Double tests
        $noinline$testGetAndAdd_Double();
        $noinline$testGetAndSet_Double();
    }

    private static void $noinline$testGetAndAdd_Int() {
        Main m = new Main();
        // 0 + 100 = 100
        $noinline$assertIntEquals(0, (int) INT_VALUE.get(m));
        $noinline$assertIntEquals(0, m.$noinline$getAndAdd_Int(100));
        $noinline$assertIntEquals(100, (int) INT_VALUE.get(m));

        // 100 - 100 = 0
        $noinline$assertIntEquals(100, m.$noinline$getAndAdd_Int(-100));
        $noinline$assertIntEquals(0, (int) INT_VALUE.get(m));
    }

    private static void $noinline$testGetAndSet_Int() {
        Main m = new Main();
        $noinline$assertIntEquals(0, (int) INT_VALUE.get(m));
        $noinline$assertIntEquals(0, m.$noinline$getAndSet_Int(100));
        $noinline$assertIntEquals(100, (int) INT_VALUE.get(m));

        $noinline$assertIntEquals(100, m.$noinline$getAndSet_Int(-100));
        $noinline$assertIntEquals(-100, (int) INT_VALUE.get(m));

        $noinline$assertIntEquals(-100, m.$noinline$getAndSet_Int(0));
        $noinline$assertIntEquals(0, (int) INT_VALUE.get(m));
    }

    private static void $noinline$testGetAndBitwiseAnd_Int() {
        Main m = new Main();
        // 0 AND X = 0
        $noinline$assertIntEquals(0, (int) INT_VALUE.get(m));
        $noinline$assertIntEquals(0, m.$noinline$getAndBitwiseAnd_Int(100));
        $noinline$assertIntEquals(0, (int) INT_VALUE.get(m));

        // 10101010 AND
        // 11001100 =
        // 10001000
        m.$noinline$getAndSet_Int(0b10101010);
        $noinline$assertIntEquals(0b10101010, m.$noinline$getAndBitwiseAnd_Int(0b11001100));
        $noinline$assertIntEquals(0b10001000, (int) INT_VALUE.get(m));

        // 10001000 AND
        // 11111111 =
        // 10001000
        $noinline$assertIntEquals(0b10001000, m.$noinline$getAndBitwiseAnd_Int(0b11111111));
        $noinline$assertIntEquals(0b10001000, (int) INT_VALUE.get(m));

        // 10001000 AND
        // 01110111 =
        // 0
        $noinline$assertIntEquals(0b10001000, m.$noinline$getAndBitwiseAnd_Int(0b01110111));
        $noinline$assertIntEquals(0, (int) INT_VALUE.get(m));
    }

    private static void $noinline$testGetAndBitwiseOr_Int() {
        Main m = new Main();

        // 0 OR X = X
        $noinline$assertIntEquals(0, (int) INT_VALUE.get(m));
        $noinline$assertIntEquals(0, m.$noinline$getAndBitwiseOr_Int(0b10101010));
        $noinline$assertIntEquals(0b10101010, (int) INT_VALUE.get(m));

        // 10101010 OR
        // 01010101 =
        // 11111111
        $noinline$assertIntEquals(0b10101010, m.$noinline$getAndBitwiseOr_Int(0b01010101));
        $noinline$assertIntEquals(0b11111111, (int) INT_VALUE.get(m));

        // 11111111 OR
        // 0 =
        // 11111111
        $noinline$assertIntEquals(0b11111111, m.$noinline$getAndBitwiseOr_Int(0));
        $noinline$assertIntEquals(0b11111111, (int) INT_VALUE.get(m));

        // Set to 0 due to precondition. See comment in main.
        m.$noinline$getAndSet_Int(0);
        $noinline$assertIntEquals(0, (int) INT_VALUE.get(m));
    }

    private static void $noinline$testGetAndBitwiseXor_Int() {
        Main m = new Main();

        // 0 XOR X = X
        $noinline$assertIntEquals(0, (int) INT_VALUE.get(m));
        $noinline$assertIntEquals(0, m.$noinline$getAndBitwiseXor_Int(0b10101010));
        $noinline$assertIntEquals(0b10101010, (int) INT_VALUE.get(m));

        // 10101010 XOR
        // 01010101 =
        // 11111111
        $noinline$assertIntEquals(0b10101010, m.$noinline$getAndBitwiseXor_Int(0b01010101));
        $noinline$assertIntEquals(0b11111111, (int) INT_VALUE.get(m));

        // 11111111 XOR
        // 01010101 =
        // 10101010
        $noinline$assertIntEquals(0b11111111, m.$noinline$getAndBitwiseXor_Int(0b01010101));
        $noinline$assertIntEquals(0b10101010, (int) INT_VALUE.get(m));

        // X XOR X = 0
        $noinline$assertIntEquals(0b10101010, m.$noinline$getAndBitwiseXor_Int(0b10101010));
        $noinline$assertIntEquals(0, (int) INT_VALUE.get(m));
    }

    private static void $noinline$testGetAndAdd_Long() {
        Main m = new Main();
        // 0 + 100 = 100
        $noinline$assertLongEquals(0L, (long) LONG_VALUE.get(m));
        $noinline$assertLongEquals(0L, m.$noinline$getAndAdd_Long(100));
        $noinline$assertLongEquals(100L, (long) LONG_VALUE.get(m));

        // 100 - 100 = 0
        $noinline$assertLongEquals(100L, m.$noinline$getAndAdd_Long(-100));
        $noinline$assertLongEquals(0L, (long) LONG_VALUE.get(m));
    }

    private static void $noinline$testGetAndSet_Long() {
        Main m = new Main();
        $noinline$assertLongEquals(0L, (long) LONG_VALUE.get(m));
        $noinline$assertLongEquals(0L, m.$noinline$getAndSet_Long(100));
        $noinline$assertLongEquals(100L, (long) LONG_VALUE.get(m));

        $noinline$assertLongEquals(100L, m.$noinline$getAndSet_Long(-100));
        $noinline$assertLongEquals(-100L, (long) LONG_VALUE.get(m));

        $noinline$assertLongEquals(-100L, m.$noinline$getAndSet_Long(0));
        $noinline$assertLongEquals(0L, (long) LONG_VALUE.get(m));
    }

    private static void $noinline$testGetAndBitwiseAnd_Long() {
        Main m = new Main();
        // 0 AND X = 0
        $noinline$assertLongEquals(0L, (long) LONG_VALUE.get(m));
        $noinline$assertLongEquals(0L, m.$noinline$getAndBitwiseAnd_Long(100));
        $noinline$assertLongEquals(0L, (long) LONG_VALUE.get(m));

        // 10101010 AND
        // 11001100 =
        // 10001000
        m.$noinline$getAndSet_Long(0b10101010);
        $noinline$assertLongEquals(0b10101010L, m.$noinline$getAndBitwiseAnd_Long(0b11001100));
        $noinline$assertLongEquals(0b10001000L, (long) LONG_VALUE.get(m));

        // 10001000 AND
        // 11111111 =
        // 10001000
        $noinline$assertLongEquals(0b10001000L, m.$noinline$getAndBitwiseAnd_Long(0b11111111));
        $noinline$assertLongEquals(0b10001000L, (long) LONG_VALUE.get(m));

        // 10001000 AND
        // 01110111 =
        // 0
        $noinline$assertLongEquals(0b10001000L, m.$noinline$getAndBitwiseAnd_Long(0b01110111));
        $noinline$assertLongEquals(0L, (long) LONG_VALUE.get(m));
    }

    private static void $noinline$testGetAndBitwiseOr_Long() {
        Main m = new Main();

        // 0 OR X = X
        $noinline$assertLongEquals(0L, (long) LONG_VALUE.get(m));
        $noinline$assertLongEquals(0L, m.$noinline$getAndBitwiseOr_Long(0b10101010));
        $noinline$assertLongEquals(0b10101010L, (long) LONG_VALUE.get(m));

        // 10101010 OR
        // 01010101 =
        // 11111111
        $noinline$assertLongEquals(0b10101010L, m.$noinline$getAndBitwiseOr_Long(0b01010101));
        $noinline$assertLongEquals(0b11111111L, (long) LONG_VALUE.get(m));

        // 11111111 OR
        // 0 =
        // 11111111
        $noinline$assertLongEquals(0b11111111L, m.$noinline$getAndBitwiseOr_Long(0));
        $noinline$assertLongEquals(0b11111111L, (long) LONG_VALUE.get(m));

        // Set to 0 due to precondition. See comment in main.
        m.$noinline$getAndSet_Long(0);
        $noinline$assertLongEquals(0L, (long) LONG_VALUE.get(m));
    }

    private static void $noinline$testGetAndBitwiseXor_Long() {
        Main m = new Main();

        // 0 XOR X = X
        $noinline$assertLongEquals(0L, (long) LONG_VALUE.get(m));
        $noinline$assertLongEquals(0L, m.$noinline$getAndBitwiseXor_Long(0b10101010));
        $noinline$assertLongEquals(0b10101010L, (long) LONG_VALUE.get(m));

        // 10101010 XOR
        // 01010101 =
        // 11111111
        $noinline$assertLongEquals(0b10101010L, m.$noinline$getAndBitwiseXor_Long(0b01010101));
        $noinline$assertLongEquals(0b11111111L, (long) LONG_VALUE.get(m));

        // 11111111 XOR
        // 01010101 =
        // 10101010
        $noinline$assertLongEquals(0b11111111L, m.$noinline$getAndBitwiseXor_Long(0b01010101));
        $noinline$assertLongEquals(0b10101010L, (long) LONG_VALUE.get(m));

        // X XOR X = 0
        $noinline$assertLongEquals(0b10101010L, m.$noinline$getAndBitwiseXor_Long(0b10101010));
        $noinline$assertLongEquals(0L, (long) LONG_VALUE.get(m));
    }

    private static void $noinline$testGetAndAdd_Float() {
        Main m = new Main();
        // 0 + 100 = 100
        $noinline$assertFloatEquals(0.0f, (float) FLOAT_VALUE.get(m));
        $noinline$assertFloatEquals(0.0f, m.$noinline$getAndAdd_Float(100.0f));
        $noinline$assertFloatEquals(100.0f, (float) FLOAT_VALUE.get(m));

        // 100 - 100 = 0
        $noinline$assertFloatEquals(100.0f, m.$noinline$getAndAdd_Float(-100.0f));
        $noinline$assertFloatEquals(0.0f, (float) FLOAT_VALUE.get(m));
    }

    private static void $noinline$testGetAndSet_Float() {
        Main m = new Main();
        $noinline$assertFloatEquals(0.0f, (float) FLOAT_VALUE.get(m));
        $noinline$assertFloatEquals(0.0f, m.$noinline$getAndSet_Float(100.0f));
        $noinline$assertFloatEquals(100.0f, (float) FLOAT_VALUE.get(m));

        $noinline$assertFloatEquals(100.0f, m.$noinline$getAndSet_Float(-100.0f));
        $noinline$assertFloatEquals(-100.0f, (float) FLOAT_VALUE.get(m));

        $noinline$assertFloatEquals(-100.0f, m.$noinline$getAndSet_Float(0.0f));
        $noinline$assertFloatEquals(0.0f, (float) FLOAT_VALUE.get(m));
    }

    private static void $noinline$testGetAndAdd_Double() {
        Main m = new Main();
        // 0 + 100 = 100
        $noinline$assertDoubleEquals(0.0d, (double) DOUBLE_VALUE.get(m));
        $noinline$assertDoubleEquals(0.0d, m.$noinline$getAndAdd_Double(100.0d));
        $noinline$assertDoubleEquals(100.0d, (double) DOUBLE_VALUE.get(m));

        // 100 - 100 = 0
        $noinline$assertDoubleEquals(100.0d, m.$noinline$getAndAdd_Double(-100.0d));
        $noinline$assertDoubleEquals(0.0d, (double) DOUBLE_VALUE.get(m));
    }

    private static void $noinline$testGetAndSet_Double() {
        Main m = new Main();
        $noinline$assertDoubleEquals(0.0d, (double) DOUBLE_VALUE.get(m));
        $noinline$assertDoubleEquals(0.0d, m.$noinline$getAndSet_Double(100.0d));
        $noinline$assertDoubleEquals(100.0d, (double) DOUBLE_VALUE.get(m));

        $noinline$assertDoubleEquals(100.0d, m.$noinline$getAndSet_Double(-100.0d));
        $noinline$assertDoubleEquals(-100.0d, (double) DOUBLE_VALUE.get(m));

        $noinline$assertDoubleEquals(-100.0d, m.$noinline$getAndSet_Double(0.0d));
        $noinline$assertDoubleEquals(0.0d, (double) DOUBLE_VALUE.get(m));
    }

    // VarHandles
    private volatile int int_value = 0;
    private static final VarHandle INT_VALUE;
    static {
        try {
            MethodHandles.Lookup l = MethodHandles.lookup();
            INT_VALUE = l.findVarHandle(Main.class, "int_value", int.class);
        } catch (ReflectiveOperationException e) {
            throw new ExceptionInInitializerError(e);
        }
    }

    private volatile long long_value = 0L;
    private static final VarHandle LONG_VALUE;
    static {
        try {
            MethodHandles.Lookup l = MethodHandles.lookup();
            LONG_VALUE = l.findVarHandle(Main.class, "long_value", long.class);
        } catch (ReflectiveOperationException e) {
            throw new ExceptionInInitializerError(e);
        }
    }

    private volatile float float_value = 0.0f;
    private static final VarHandle FLOAT_VALUE;
    static {
        try {
            MethodHandles.Lookup l = MethodHandles.lookup();
            FLOAT_VALUE = l.findVarHandle(Main.class, "float_value", float.class);
        } catch (ReflectiveOperationException e) {
            throw new ExceptionInInitializerError(e);
        }
    }

    private volatile double double_value = 0.0d;
    private static final VarHandle DOUBLE_VALUE;
    static {
        try {
            MethodHandles.Lookup l = MethodHandles.lookup();
            DOUBLE_VALUE = l.findVarHandle(Main.class, "double_value", double.class);
        } catch (ReflectiveOperationException e) {
            throw new ExceptionInInitializerError(e);
        }
    }

    // Check that we successfully intrinsify intrinsics (e.g. getAndAdd) by checking that there's no
    // call to the runtime.

    /// CHECK-START-{X86,X86_64}: int Main.$noinline$getAndAdd_Int(int) disassembly (after)
    /// CHECK:     InvokePolymorphic intrinsic:VarHandleGetAndAdd
    /// CHECK-NOT: call
    /// CHECK:     Return

    /// CHECK-START-ARM64: int Main.$noinline$getAndAdd_Int(int) disassembly (after)
    /// CHECK:     InvokePolymorphic intrinsic:VarHandleGetAndAdd
    /// CHECK-NOT: blx
    /// CHECK:     Return

    /// CHECK-START-ARM64: int Main.$noinline$getAndAdd_Int(int) disassembly (after)
    /// CHECK:     InvokePolymorphic intrinsic:VarHandleGetAndAdd
    /// CHECK-NOT: blr
    /// CHECK:     Return

    /// CHECK-START-RISCV64: int Main.$noinline$getAndAdd_Int(int) disassembly (after)
    /// CHECK:     InvokePolymorphic intrinsic:VarHandleGetAndAdd
    /// CHECK-NOT: jalr
    /// CHECK:     Return
    private int $noinline$getAndAdd_Int(int value) {
        return (int) INT_VALUE.getAndAdd(this, value);
    }

    /// CHECK-START-{X86,X86_64}: int Main.$noinline$getAndSet_Int(int) disassembly (after)
    /// CHECK:     InvokePolymorphic intrinsic:VarHandleGetAndSet
    /// CHECK-NOT: call
    /// CHECK:     Return

    /// CHECK-START-ARM64: int Main.$noinline$getAndSet_Int(int) disassembly (after)
    /// CHECK:     InvokePolymorphic intrinsic:VarHandleGetAndSet
    /// CHECK-NOT: blx
    /// CHECK:     Return

    /// CHECK-START-ARM64: int Main.$noinline$getAndSet_Int(int) disassembly (after)
    /// CHECK:     InvokePolymorphic intrinsic:VarHandleGetAndSet
    /// CHECK-NOT: blr
    /// CHECK:     Return

    /// CHECK-START-RISCV64: int Main.$noinline$getAndSet_Int(int) disassembly (after)
    /// CHECK:     InvokePolymorphic intrinsic:VarHandleGetAndSet
    /// CHECK-NOT: jalr
    /// CHECK:     Return
    private int $noinline$getAndSet_Int(int value) {
        return (int) INT_VALUE.getAndSet(this, value);
    }

    /// CHECK-START-{X86,X86_64}: int Main.$noinline$getAndBitwiseAnd_Int(int) disassembly (after)
    /// CHECK:     InvokePolymorphic intrinsic:VarHandleGetAndBitwiseAnd
    /// CHECK-NOT: call
    /// CHECK:     Return

    /// CHECK-START-ARM64: int Main.$noinline$getAndBitwiseAnd_Int(int) disassembly (after)
    /// CHECK:     InvokePolymorphic intrinsic:VarHandleGetAndBitwiseAnd
    /// CHECK-NOT: blx
    /// CHECK:     Return

    /// CHECK-START-ARM64: int Main.$noinline$getAndBitwiseAnd_Int(int) disassembly (after)
    /// CHECK:     InvokePolymorphic intrinsic:VarHandleGetAndBitwiseAnd
    /// CHECK-NOT: blr
    /// CHECK:     Return

    /// CHECK-START-RISCV64: int Main.$noinline$getAndBitwiseAnd_Int(int) disassembly (after)
    /// CHECK:     InvokePolymorphic intrinsic:VarHandleGetAndBitwiseAnd
    /// CHECK-NOT: jalr
    /// CHECK:     Return
    private int $noinline$getAndBitwiseAnd_Int(int value) {
        return (int) INT_VALUE.getAndBitwiseAnd(this, value);
    }

    /// CHECK-START-{X86,X86_64}: int Main.$noinline$getAndBitwiseOr_Int(int) disassembly (after)
    /// CHECK:     InvokePolymorphic intrinsic:VarHandleGetAndBitwiseOr
    /// CHECK-NOT: call
    /// CHECK:     Return

    /// CHECK-START-ARM64: int Main.$noinline$getAndBitwiseOr_Int(int) disassembly (after)
    /// CHECK:     InvokePolymorphic intrinsic:VarHandleGetAndBitwiseOr
    /// CHECK-NOT: blx
    /// CHECK:     Return

    /// CHECK-START-ARM64: int Main.$noinline$getAndBitwiseOr_Int(int) disassembly (after)
    /// CHECK:     InvokePolymorphic intrinsic:VarHandleGetAndBitwiseOr
    /// CHECK-NOT: blr
    /// CHECK:     Return

    /// CHECK-START-RISCV64: int Main.$noinline$getAndBitwiseOr_Int(int) disassembly (after)
    /// CHECK:     InvokePolymorphic intrinsic:VarHandleGetAndBitwiseOr
    /// CHECK-NOT: jalr
    /// CHECK:     Return
    private int $noinline$getAndBitwiseOr_Int(int value) {
        return (int) INT_VALUE.getAndBitwiseOr(this, value);
    }

    /// CHECK-START-{X86,X86_64}: int Main.$noinline$getAndBitwiseXor_Int(int) disassembly (after)
    /// CHECK:     InvokePolymorphic intrinsic:VarHandleGetAndBitwiseXor
    /// CHECK-NOT: call
    /// CHECK:     Return

    /// CHECK-START-ARM64: int Main.$noinline$getAndBitwiseXor_Int(int) disassembly (after)
    /// CHECK:     InvokePolymorphic intrinsic:VarHandleGetAndBitwiseXor
    /// CHECK-NOT: blx
    /// CHECK:     Return

    /// CHECK-START-ARM64: int Main.$noinline$getAndBitwiseXor_Int(int) disassembly (after)
    /// CHECK:     InvokePolymorphic intrinsic:VarHandleGetAndBitwiseXor
    /// CHECK-NOT: blr
    /// CHECK:     Return

    /// CHECK-START-RISCV64: int Main.$noinline$getAndBitwiseXor_Int(int) disassembly (after)
    /// CHECK:     InvokePolymorphic intrinsic:VarHandleGetAndBitwiseXor
    /// CHECK-NOT: jalr
    /// CHECK:     Return
    private int $noinline$getAndBitwiseXor_Int(int value) {
        return (int) INT_VALUE.getAndBitwiseXor(this, value);
    }

    private static void $noinline$assertIntEquals(int expected, int result) {
        if (expected != result) {
            throw new Error("Expected: " + expected + ", found: " + result);
        }
    }

    // Note that the Long ones do a call for X86.
    // TODO(solanes): Add this support.

    /// CHECK-START-X86: long Main.$noinline$getAndAdd_Long(long) disassembly (after)
    /// CHECK:     InvokePolymorphic intrinsic:VarHandleGetAndAdd
    /// CHECK:     call
    /// CHECK:     Return

    /// CHECK-START-X86_64: long Main.$noinline$getAndAdd_Long(long) disassembly (after)
    /// CHECK:     InvokePolymorphic intrinsic:VarHandleGetAndAdd
    /// CHECK-NOT: call
    /// CHECK:     Return

    /// CHECK-START-ARM64: long Main.$noinline$getAndAdd_Long(long) disassembly (after)
    /// CHECK:     InvokePolymorphic intrinsic:VarHandleGetAndAdd
    /// CHECK-NOT: blx
    /// CHECK:     Return

    /// CHECK-START-ARM64: long Main.$noinline$getAndAdd_Long(long) disassembly (after)
    /// CHECK:     InvokePolymorphic intrinsic:VarHandleGetAndAdd
    /// CHECK-NOT: blr
    /// CHECK:     Return

    /// CHECK-START-RISCV64: long Main.$noinline$getAndAdd_Long(long) disassembly (after)
    /// CHECK:     InvokePolymorphic intrinsic:VarHandleGetAndAdd
    /// CHECK-NOT: jalr
    /// CHECK:     Return
    private long $noinline$getAndAdd_Long(long value) {
        return (long) LONG_VALUE.getAndAdd(this, value);
    }

    /// CHECK-START-X86: long Main.$noinline$getAndSet_Long(long) disassembly (after)
    /// CHECK:     InvokePolymorphic intrinsic:VarHandleGetAndSet
    /// CHECK:     call
    /// CHECK:     Return

    /// CHECK-START-X86_64: long Main.$noinline$getAndSet_Long(long) disassembly (after)
    /// CHECK:     InvokePolymorphic intrinsic:VarHandleGetAndSet
    /// CHECK-NOT: call
    /// CHECK:     Return

    /// CHECK-START-ARM64: long Main.$noinline$getAndSet_Long(long) disassembly (after)
    /// CHECK:     InvokePolymorphic intrinsic:VarHandleGetAndSet
    /// CHECK-NOT: blx
    /// CHECK:     Return

    /// CHECK-START-ARM64: long Main.$noinline$getAndSet_Long(long) disassembly (after)
    /// CHECK:     InvokePolymorphic intrinsic:VarHandleGetAndSet
    /// CHECK-NOT: blr
    /// CHECK:     Return

    /// CHECK-START-RISCV64: long Main.$noinline$getAndSet_Long(long) disassembly (after)
    /// CHECK:     InvokePolymorphic intrinsic:VarHandleGetAndSet
    /// CHECK-NOT: jalr
    /// CHECK:     Return
    private long $noinline$getAndSet_Long(long value) {
        return (long) LONG_VALUE.getAndSet(this, value);
    }

    /// CHECK-START-X86: long Main.$noinline$getAndBitwiseAnd_Long(long) disassembly (after)
    /// CHECK:     InvokePolymorphic intrinsic:VarHandleGetAndBitwiseAnd
    /// CHECK:     call
    /// CHECK:     Return

    /// CHECK-START-X86_64: long Main.$noinline$getAndBitwiseAnd_Long(long) disassembly (after)
    /// CHECK:     InvokePolymorphic intrinsic:VarHandleGetAndBitwiseAnd
    /// CHECK-NOT: call
    /// CHECK:     Return

    /// CHECK-START-ARM64: long Main.$noinline$getAndBitwiseAnd_Long(long) disassembly (after)
    /// CHECK:     InvokePolymorphic intrinsic:VarHandleGetAndBitwiseAnd
    /// CHECK-NOT: blx
    /// CHECK:     Return

    /// CHECK-START-ARM64: long Main.$noinline$getAndBitwiseAnd_Long(long) disassembly (after)
    /// CHECK:     InvokePolymorphic intrinsic:VarHandleGetAndBitwiseAnd
    /// CHECK-NOT: blr
    /// CHECK:     Return

    /// CHECK-START-RISCV64: long Main.$noinline$getAndBitwiseAnd_Long(long) disassembly (after)
    /// CHECK:     InvokePolymorphic intrinsic:VarHandleGetAndBitwiseAnd
    /// CHECK-NOT: jalr
    /// CHECK:     Return
    private long $noinline$getAndBitwiseAnd_Long(long value) {
        return (long) LONG_VALUE.getAndBitwiseAnd(this, value);
    }

    /// CHECK-START-X86: long Main.$noinline$getAndBitwiseOr_Long(long) disassembly (after)
    /// CHECK:     InvokePolymorphic intrinsic:VarHandleGetAndBitwiseOr
    /// CHECK:     call
    /// CHECK:     Return

    /// CHECK-START-X86_64: long Main.$noinline$getAndBitwiseOr_Long(long) disassembly (after)
    /// CHECK:     InvokePolymorphic intrinsic:VarHandleGetAndBitwiseOr
    /// CHECK-NOT: call
    /// CHECK:     Return

    /// CHECK-START-ARM64: long Main.$noinline$getAndBitwiseOr_Long(long) disassembly (after)
    /// CHECK:     InvokePolymorphic intrinsic:VarHandleGetAndBitwiseOr
    /// CHECK-NOT: blx
    /// CHECK:     Return

    /// CHECK-START-ARM64: long Main.$noinline$getAndBitwiseOr_Long(long) disassembly (after)
    /// CHECK:     InvokePolymorphic intrinsic:VarHandleGetAndBitwiseOr
    /// CHECK-NOT: blr
    /// CHECK:     Return

    /// CHECK-START-RISCV64: long Main.$noinline$getAndBitwiseOr_Long(long) disassembly (after)
    /// CHECK:     InvokePolymorphic intrinsic:VarHandleGetAndBitwiseOr
    /// CHECK-NOT: jalr
    /// CHECK:     Return
    private long $noinline$getAndBitwiseOr_Long(long value) {
        return (long) LONG_VALUE.getAndBitwiseOr(this, value);
    }

    /// CHECK-START-X86: long Main.$noinline$getAndBitwiseXor_Long(long) disassembly (after)
    /// CHECK:     InvokePolymorphic intrinsic:VarHandleGetAndBitwiseXor
    /// CHECK:     call
    /// CHECK:     Return

    /// CHECK-START-X86_64: long Main.$noinline$getAndBitwiseXor_Long(long) disassembly (after)
    /// CHECK:     InvokePolymorphic intrinsic:VarHandleGetAndBitwiseXor
    /// CHECK-NOT: call
    /// CHECK:     Return

    /// CHECK-START-ARM64: long Main.$noinline$getAndBitwiseXor_Long(long) disassembly (after)
    /// CHECK:     InvokePolymorphic intrinsic:VarHandleGetAndBitwiseXor
    /// CHECK-NOT: blx
    /// CHECK:     Return

    /// CHECK-START-ARM64: long Main.$noinline$getAndBitwiseXor_Long(long) disassembly (after)
    /// CHECK:     InvokePolymorphic intrinsic:VarHandleGetAndBitwiseXor
    /// CHECK-NOT: blr
    /// CHECK:     Return

    /// CHECK-START-RISCV64: long Main.$noinline$getAndBitwiseXor_Long(long) disassembly (after)
    /// CHECK:     InvokePolymorphic intrinsic:VarHandleGetAndBitwiseXor
    /// CHECK-NOT: jalr
    /// CHECK:     Return
    private long $noinline$getAndBitwiseXor_Long(long value) {
        return (long) LONG_VALUE.getAndBitwiseXor(this, value);
    }

    private static void $noinline$assertLongEquals(long expected, long result) {
        if (expected != result) {
            throw new Error("Expected: " + expected + ", found: " + result);
        }
    }

    /// CHECK-START-{X86,X86_64}: float Main.$noinline$getAndAdd_Float(float) disassembly (after)
    /// CHECK:     InvokePolymorphic intrinsic:VarHandleGetAndAdd
    /// CHECK-NOT: call
    /// CHECK:     Return

    /// CHECK-START-ARM64: float Main.$noinline$getAndAdd_Float(float) disassembly (after)
    /// CHECK:     InvokePolymorphic intrinsic:VarHandleGetAndAdd
    /// CHECK-NOT: blx
    /// CHECK:     Return

    /// CHECK-START-ARM64: float Main.$noinline$getAndAdd_Float(float) disassembly (after)
    /// CHECK:     InvokePolymorphic intrinsic:VarHandleGetAndAdd
    /// CHECK-NOT: blr
    /// CHECK:     Return

    /// CHECK-START-RISCV64: float Main.$noinline$getAndAdd_Float(float) disassembly (after)
    /// CHECK:     InvokePolymorphic intrinsic:VarHandleGetAndAdd
    /// CHECK-NOT: jalr
    /// CHECK:     Return
    private float $noinline$getAndAdd_Float(float value) {
        return (float) FLOAT_VALUE.getAndAdd(this, value);
    }

    /// CHECK-START-{X86,X86_64}: float Main.$noinline$getAndSet_Float(float) disassembly (after)
    /// CHECK:     InvokePolymorphic intrinsic:VarHandleGetAndSet
    /// CHECK-NOT: call
    /// CHECK:     Return

    /// CHECK-START-ARM64: float Main.$noinline$getAndSet_Float(float) disassembly (after)
    /// CHECK:     InvokePolymorphic intrinsic:VarHandleGetAndSet
    /// CHECK-NOT: blx
    /// CHECK:     Return

    /// CHECK-START-ARM64: float Main.$noinline$getAndSet_Float(float) disassembly (after)
    /// CHECK:     InvokePolymorphic intrinsic:VarHandleGetAndSet
    /// CHECK-NOT: blr
    /// CHECK:     Return

    /// CHECK-START-RISCV64: float Main.$noinline$getAndSet_Float(float) disassembly (after)
    /// CHECK:     InvokePolymorphic intrinsic:VarHandleGetAndSet
    /// CHECK-NOT: jalr
    /// CHECK:     Return
    private float $noinline$getAndSet_Float(float value) {
        return (float) FLOAT_VALUE.getAndSet(this, value);
    }

    private static void $noinline$assertFloatEquals(float expected, float result) {
        if (expected != result) {
            throw new Error("Expected: " + expected + ", found: " + result);
        }
    }

    // Note that the Double ones do a call for X86.
    // TODO(solanes): Add this support.

    /// CHECK-START-X86: double Main.$noinline$getAndAdd_Double(double) disassembly (after)
    /// CHECK:     InvokePolymorphic intrinsic:VarHandleGetAndAdd
    /// CHECK:     call
    /// CHECK:     Return

    /// CHECK-START-X86_64: double Main.$noinline$getAndAdd_Double(double) disassembly (after)
    /// CHECK:     InvokePolymorphic intrinsic:VarHandleGetAndAdd
    /// CHECK-NOT: call
    /// CHECK:     Return

    /// CHECK-START-ARM64: double Main.$noinline$getAndAdd_Double(double) disassembly (after)
    /// CHECK:     InvokePolymorphic intrinsic:VarHandleGetAndAdd
    /// CHECK-NOT: blx
    /// CHECK:     Return

    /// CHECK-START-ARM64: double Main.$noinline$getAndAdd_Double(double) disassembly (after)
    /// CHECK:     InvokePolymorphic intrinsic:VarHandleGetAndAdd
    /// CHECK-NOT: blr
    /// CHECK:     Return

    /// CHECK-START-RISCV64: double Main.$noinline$getAndAdd_Double(double) disassembly (after)
    /// CHECK:     InvokePolymorphic intrinsic:VarHandleGetAndAdd
    /// CHECK-NOT: jalr
    /// CHECK:     Return
    private double $noinline$getAndAdd_Double(double value) {
        return (double) DOUBLE_VALUE.getAndAdd(this, value);
    }

    /// CHECK-START-X86: double Main.$noinline$getAndSet_Double(double) disassembly (after)
    /// CHECK:     InvokePolymorphic intrinsic:VarHandleGetAndSet
    /// CHECK:     call
    /// CHECK:     Return

    /// CHECK-START-X86_64: double Main.$noinline$getAndSet_Double(double) disassembly (after)
    /// CHECK:     InvokePolymorphic intrinsic:VarHandleGetAndSet
    /// CHECK-NOT: call
    /// CHECK:     Return

    /// CHECK-START-ARM64: double Main.$noinline$getAndSet_Double(double) disassembly (after)
    /// CHECK:     InvokePolymorphic intrinsic:VarHandleGetAndSet
    /// CHECK-NOT: blx
    /// CHECK:     Return

    /// CHECK-START-ARM64: double Main.$noinline$getAndSet_Double(double) disassembly (after)
    /// CHECK:     InvokePolymorphic intrinsic:VarHandleGetAndSet
    /// CHECK-NOT: blr
    /// CHECK:     Return

    /// CHECK-START-RISCV64: double Main.$noinline$getAndSet_Double(double) disassembly (after)
    /// CHECK:     InvokePolymorphic intrinsic:VarHandleGetAndSet
    /// CHECK-NOT: jalr
    /// CHECK:     Return
    private double $noinline$getAndSet_Double(double value) {
        return (double) DOUBLE_VALUE.getAndSet(this, value);
    }

    private static void $noinline$assertDoubleEquals(double expected, double result) {
        if (expected != result) {
            throw new Error("Expected: " + expected + ", found: " + result);
        }
    }
}
