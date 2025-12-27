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

import java.lang.reflect.Field;
import sun.misc.Unsafe;

public class Main {
  private static Unsafe getUnsafe() throws NoSuchFieldException, IllegalAccessException {
    Class<?> unsafeClass = Unsafe.class;
    Field f = unsafeClass.getDeclaredField("theUnsafe");
    f.setAccessible(true);
    return (Unsafe) f.get(null);
  }

  private static Unsafe unsafe;

  static void assertEquals(int expected, int actual) {
    if (expected != actual) {
      throw new Error("Expected " + expected + ", got " + actual);
    }
  }

  public static void main(String[] args) throws NoSuchFieldException, IllegalAccessException {
    unsafe = getUnsafe();
    testPutZero();
    testPutFixedOffset();
    assertEquals(0, testGet());
    assertEquals(42, testGetFar());
    testGetAndPutAbsoluteAddress();
  }

  /// CHECK-START-ARM64: void Main.testPutZero() disassembly (after)
  /// CHECK:                  str wzr, [x{{[0-9]+}}, #12]
  private static void testPutZero() {
    int[] object = new int[42];
    unsafe.putInt(object, unsafe.arrayBaseOffset(int[].class), 0);
  }

  /// CHECK-START-ARM64: void Main.testPutFixedOffset() disassembly (after)
  /// CHECK:                  stur w{{[0-9]+}}, [x{{[0-9]+}}, #38]
  private static void testPutFixedOffset() {
    int[] object = new int[42];
    unsafe.putInt(object, 38, 12);
  }

  /// CHECK-START-ARM64: int Main.testGet() disassembly (after)
  /// CHECK:                  ldur w{{[0-9]+}}, [x{{[0-9]+}}, #38]
  private static int testGet() {
    int[] object = new int[42];
    return unsafe.getInt(object, 38);
  }

  private static int testGetFar() {
    int offset = 32 * 1024;
    int arraySize = offset / 4;
    int[] object = new int[arraySize];
    unsafe.putInt(object, offset, 42);
    return unsafe.getInt(object, offset);
  }

  /// CHECK-START: int Main.testArrayBaseOffsetObject() instruction_simplifier (after)
  /// CHECK:                  IntConstant 12
  private static int testArrayBaseOffsetObject() {
    return unsafe.arrayBaseOffset(Object[].class);
  }

  /// CHECK-START: int Main.testArrayBaseOffsetInt() instruction_simplifier (after)
  /// CHECK:                  IntConstant 12
  private static int testArrayBaseOffsetInt() {
    return unsafe.arrayBaseOffset(int[].class);
  }

  /// CHECK-START: int Main.testArrayBaseOffsetDouble() instruction_simplifier (after)
  /// CHECK:                  IntConstant 16
  private static int testArrayBaseOffsetDouble() {
    return unsafe.arrayBaseOffset(double[].class);
  }

  private static void testGetAndPutAbsoluteAddress() {
    long address = 0;
    try {
      address = unsafe.allocateMemory(4);
      $noinline$unsafePutAbsoluteInt(address, 0xDEADBEEF);
      assertEquals(0xDEADBEEF, $noinline$unsafeGetAbsoluteInt(address));
    } finally {
      if (address != 0) {
        unsafe.freeMemory(address);
      }
    }
  }

  /// CHECK-START: void Main.$noinline$unsafePutAbsoluteInt(long, int) builder (after)
  /// CHECK:                  InvokeVirtual intrinsic:UnsafePutAbsolute
  private static void $noinline$unsafePutAbsoluteInt(long address, int value) {
    unsafe.putInt(address, value);
  }

  /// CHECK-START: int Main.$noinline$unsafeGetAbsoluteInt(long) builder (after)
  /// CHECK:                  InvokeVirtual intrinsic:UnsafeGetAbsolute
  private static int $noinline$unsafeGetAbsoluteInt(long address) {
    return unsafe.getInt(address);
  }
}
