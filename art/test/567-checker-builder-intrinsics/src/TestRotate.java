/*
 * Copyright (C) 2016 The Android Open Source Project
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

public class TestRotate {

  /// CHECK-START: int TestRotate.$inline$rotateLeftByte(byte, int) builder (after)
  /// CHECK:         <<ArgVal:b\d+>>  ParameterValue
  /// CHECK:         <<ArgDist:i\d+>> ParameterValue
  /// CHECK-DAG:     <<Result:i\d+>>  Rol [<<ArgVal>>,<<ArgDist>>]
  /// CHECK-DAG:                      Return [<<Result>>]

  /// CHECK-START: int TestRotate.$inline$rotateLeftByte(byte, int) builder (after)
  /// CHECK-NOT:                      InvokeStaticOrDirect

  private static int $inline$rotateLeftByte(byte value, int distance) {
    return Integer.rotateLeft(value, distance);
  }

  private static int $noinline$rotateLeftByte(byte value, int distance) {
    return Integer.rotateLeft(value, distance);
  }

  /// CHECK-START: int TestRotate.$inline$rotateLeftShort(short, int) builder (after)
  /// CHECK:         <<ArgVal:s\d+>>  ParameterValue
  /// CHECK:         <<ArgDist:i\d+>> ParameterValue
  /// CHECK-DAG:     <<Result:i\d+>>  Rol [<<ArgVal>>,<<ArgDist>>]
  /// CHECK-DAG:                      Return [<<Result>>]

  /// CHECK-START: int TestRotate.$inline$rotateLeftShort(short, int) builder (after)
  /// CHECK-NOT:                      InvokeStaticOrDirect

  private static int $inline$rotateLeftShort(short value, int distance) {
    return Integer.rotateLeft(value, distance);
  }

  private static int $noinline$rotateLeftShort(short value, int distance) {
    return Integer.rotateLeft(value, distance);
  }

  /// CHECK-START: int TestRotate.$inline$rotateLeftChar(char, int) builder (after)
  /// CHECK:         <<ArgVal:c\d+>>  ParameterValue
  /// CHECK:         <<ArgDist:i\d+>> ParameterValue
  /// CHECK-DAG:     <<Result:i\d+>>  Rol [<<ArgVal>>,<<ArgDist>>]
  /// CHECK-DAG:                      Return [<<Result>>]

  /// CHECK-START: int TestRotate.$inline$rotateLeftChar(char, int) builder (after)
  /// CHECK-NOT:                      InvokeStaticOrDirect

  private static int $inline$rotateLeftChar(char value, int distance) {
    return Integer.rotateLeft(value, distance);
  }

  private static int $noinline$rotateLeftChar(char value, int distance) {
    return Integer.rotateLeft(value, distance);
  }

  /// CHECK-START: int TestRotate.$inline$rotateLeftInt(int, int) builder (after)
  /// CHECK:         <<ArgVal:i\d+>>  ParameterValue
  /// CHECK:         <<ArgDist:i\d+>> ParameterValue
  /// CHECK-DAG:     <<Result:i\d+>>  Rol [<<ArgVal>>,<<ArgDist>>]
  /// CHECK-DAG:                      Return [<<Result>>]

  /// CHECK-START: int TestRotate.$inline$rotateLeftInt(int, int) builder (after)
  /// CHECK-NOT:                      InvokeStaticOrDirect

  private static int $inline$rotateLeftInt(int value, int distance) {
    return Integer.rotateLeft(value, distance);
  }

  /// CHECK-START-ARM64: int TestRotate.$noinline$rotateLeftInt(int, int) instruction_simplifier_arm64 (after)
  /// CHECK:         <<ArgVal:i\d+>>  ParameterValue
  /// CHECK:         <<ArgDist:i\d+>> ParameterValue
  /// CHECK-DAG:     <<neg:i\d+>>     Neg [<<ArgDist>>]
  /// CHECK-DAG:     <<Result:i\d+>>  Ror [<<ArgVal>>,<<neg>>]
  /// CHECK-DAG:                      Return [<<Result>>]

  /// CHECK-START-ARM64: int TestRotate.$noinline$rotateLeftInt(int, int) disassembly (after)
  /// CHECK:                           neg {{w\d+}}, {{w\d+}}
  /// CHECK:                           ror {{w\d+}}, {{w\d+}}, {{w\d+}}

  /// CHECK-START-RISCV64: int TestRotate.$noinline$rotateLeftInt(int, int) disassembly (after)
  /// CHECK:                           rolw {{a\d+}}, {{a\d+}}, {{a\d+}}

  private static int $noinline$rotateLeftInt(int value, int distance) {
    return Integer.rotateLeft(value, distance);
  }

  /// CHECK-START-ARM64: int TestRotate.$noinline$rotateLeftIntMulNegDistance(int, int) scheduler (before)
  /// CHECK:         <<ArgVal:i\d+>>  ParameterValue
  /// CHECK:         <<ArgDist:i\d+>> ParameterValue
  /// CHECK-DAG:     <<neg:i\d+>>     Neg [<<ArgDist>>]
  /// CHECK-DAG:     <<ror:i\d+>>     Ror [<<ArgVal>>,<<neg>>]
  /// CHECK-DAG:     <<Result:i\d+>>  Mul [<<neg>>,<<ror>>]
  /// CHECK-DAG:                      Return [<<Result>>]

  private static int $noinline$rotateLeftIntMulNegDistance(int value, int distance) {
    return Integer.rotateLeft(value, distance) * -distance;
  }

  /// CHECK-START: int TestRotate.$noinline$rotateLeftIntConstant(int) builder (after)
  /// CHECK:         <<ArgVal:i\d+>>   ParameterValue
  /// CHECK:         <<Constant:i\d+>> IntConstant 31
  /// CHECK-DAG:     <<Result:i\d+>>   Rol [<<ArgVal>>,<<Constant>>]
  /// CHECK-DAG:                       Return [<<Result>>]

  /// CHECK-START-ARM64: int TestRotate.$noinline$rotateLeftIntConstant(int) disassembly (after)
  /// CHECK:                           ror {{w\d+}}, {{w\d+}}, #1

  /// CHECK-START-RISCV64: int TestRotate.$noinline$rotateLeftIntConstant(int) disassembly (after)
  /// CHECK:                           roriw {{a\d+}}, {{a\d+}}, 1

  private static int $noinline$rotateLeftIntConstant(int value) {
    return Integer.rotateLeft(value, 31);
  }

  /// CHECK-START: long TestRotate.$inline$rotateLeftLong(long, int) builder (after)
  /// CHECK:         <<ArgVal:j\d+>>  ParameterValue
  /// CHECK:         <<ArgDist:i\d+>> ParameterValue
  /// CHECK-DAG:     <<Result:j\d+>>  Rol [<<ArgVal>>,<<ArgDist>>]
  /// CHECK-DAG:                      Return [<<Result>>]

  /// CHECK-START: long TestRotate.$inline$rotateLeftLong(long, int) builder (after)
  /// CHECK-NOT:                      InvokeStaticOrDirect

  private static long $inline$rotateLeftLong(long value, int distance) {
    return Long.rotateLeft(value, distance);
  }

  /// CHECK-START-ARM64: long TestRotate.$noinline$rotateLeftLong(long, int) instruction_simplifier_arm64 (after)
  /// CHECK:         <<ArgVal:j\d+>>  ParameterValue
  /// CHECK:         <<ArgDist:i\d+>> ParameterValue
  /// CHECK-DAG:     <<neg:i\d+>>     Neg [<<ArgDist>>]
  /// CHECK-DAG:     <<Result:j\d+>>  Ror [<<ArgVal>>,<<neg>>]
  /// CHECK-DAG:                      Return [<<Result>>]

  /// CHECK-START-ARM64: long TestRotate.$noinline$rotateLeftLong(long, int) disassembly (after)
  /// CHECK:                           neg {{w\d+}}, {{w\d+}}
  /// CHECK:                           ror {{x\d+}}, {{x\d+}}, {{x\d+}}

  /// CHECK-START-RISCV64: long TestRotate.$noinline$rotateLeftLong(long, int) disassembly (after)
  /// CHECK:                           rol {{a\d+}}, {{a\d+}}, {{a\d+}}

  private static long $noinline$rotateLeftLong(long value, int distance) {
    return Long.rotateLeft(value, distance);
  }

  /// CHECK-START: long TestRotate.$noinline$rotateLeftLongConstant(long) builder (after)
  /// CHECK:         <<ArgVal:j\d+>>   ParameterValue
  /// CHECK:         <<Constant:i\d+>> IntConstant 63
  /// CHECK-DAG:     <<Result:j\d+>>   Rol [<<ArgVal>>,<<Constant>>]
  /// CHECK-DAG:                       Return [<<Result>>]

  /// CHECK-START-ARM64: long TestRotate.$noinline$rotateLeftLongConstant(long) disassembly (after)
  /// CHECK:                           ror {{x\d+}}, {{x\d+}}, #1

  /// CHECK-START-RISCV64: long TestRotate.$noinline$rotateLeftLongConstant(long) disassembly (after)
  /// CHECK:                           rori {{a\d+}}, {{a\d+}}, 1

  private static long $noinline$rotateLeftLongConstant(long value) {
    return Long.rotateLeft(value, 63);
  }

  /// CHECK-START: int TestRotate.rotateRightByte(byte, int) builder (after)
  /// CHECK:         <<ArgVal:b\d+>>  ParameterValue
  /// CHECK:         <<ArgDist:i\d+>> ParameterValue
  /// CHECK-DAG:     <<Result:i\d+>>  Ror [<<ArgVal>>,<<ArgDist>>]
  /// CHECK-DAG:                      Return [<<Result>>]

  /// CHECK-START: int TestRotate.rotateRightByte(byte, int) builder (after)
  /// CHECK-NOT:                      InvokeStaticOrDirect

  private static int rotateRightByte(byte value, int distance) {
    return Integer.rotateRight(value, distance);
  }

  /// CHECK-START: int TestRotate.rotateRightShort(short, int) builder (after)
  /// CHECK:         <<ArgVal:s\d+>>  ParameterValue
  /// CHECK:         <<ArgDist:i\d+>> ParameterValue
  /// CHECK-DAG:     <<Result:i\d+>>  Ror [<<ArgVal>>,<<ArgDist>>]
  /// CHECK-DAG:                      Return [<<Result>>]

  /// CHECK-START: int TestRotate.rotateRightShort(short, int) builder (after)
  /// CHECK-NOT:                      InvokeStaticOrDirect

  private static int rotateRightShort(short value, int distance) {
    return Integer.rotateRight(value, distance);
  }

  /// CHECK-START: int TestRotate.rotateRightChar(char, int) builder (after)
  /// CHECK:         <<ArgVal:c\d+>>  ParameterValue
  /// CHECK:         <<ArgDist:i\d+>> ParameterValue
  /// CHECK-DAG:     <<Result:i\d+>>  Ror [<<ArgVal>>,<<ArgDist>>]
  /// CHECK-DAG:                      Return [<<Result>>]

  /// CHECK-START: int TestRotate.rotateRightChar(char, int) builder (after)
  /// CHECK-NOT:                      InvokeStaticOrDirect

  private static int rotateRightChar(char value, int distance) {
    return Integer.rotateRight(value, distance);
  }

  /// CHECK-START: int TestRotate.rotateRightInt(int, int) builder (after)
  /// CHECK:         <<ArgVal:i\d+>>  ParameterValue
  /// CHECK:         <<ArgDist:i\d+>> ParameterValue
  /// CHECK-DAG:     <<Result:i\d+>>  Ror [<<ArgVal>>,<<ArgDist>>]
  /// CHECK-DAG:                      Return [<<Result>>]

  /// CHECK-START: int TestRotate.rotateRightInt(int, int) builder (after)
  /// CHECK-NOT:                      InvokeStaticOrDirect

  private static int rotateRightInt(int value, int distance) {
    return Integer.rotateRight(value, distance);
  }

  /// CHECK-START: long TestRotate.rotateRightLong(long, int) builder (after)
  /// CHECK:         <<ArgVal:j\d+>>  ParameterValue
  /// CHECK:         <<ArgDist:i\d+>> ParameterValue
  /// CHECK-DAG:     <<Result:j\d+>>  Ror [<<ArgVal>>,<<ArgDist>>]
  /// CHECK-DAG:                      Return [<<Result>>]

  /// CHECK-START: long TestRotate.rotateRightLong(long, int) builder (after)
  /// CHECK-NOT:                      InvokeStaticOrDirect

  private static long rotateRightLong(long value, int distance) {
    return Long.rotateRight(value, distance);
  }


  /// CHECK-START: int TestRotate.$inline$rotateLeftIntWithByteDistance(int, byte) builder (after)
  /// CHECK:         <<ArgVal:i\d+>>  ParameterValue
  /// CHECK:         <<ArgDist:b\d+>> ParameterValue
  /// CHECK-DAG:     <<Result:i\d+>>  Rol [<<ArgVal>>,<<ArgDist>>]
  /// CHECK-DAG:                      Return [<<Result>>]

  /// CHECK-START: int TestRotate.$inline$rotateLeftIntWithByteDistance(int, byte) builder (after)
  /// CHECK-NOT:                      InvokeStaticOrDirect

  private static int $inline$rotateLeftIntWithByteDistance(int value, byte distance) {
    return Integer.rotateLeft(value, distance);
  }

  private static int $noinline$rotateLeftIntWithByteDistance(int value, byte distance) {
    return Integer.rotateLeft(value, distance);
  }

  /// CHECK-START: int TestRotate.rotateRightIntWithByteDistance(int, byte) builder (after)
  /// CHECK:         <<ArgVal:i\d+>>  ParameterValue
  /// CHECK:         <<ArgDist:b\d+>> ParameterValue
  /// CHECK-DAG:     <<Result:i\d+>>  Ror [<<ArgVal>>,<<ArgDist>>]
  /// CHECK-DAG:                      Return [<<Result>>]

  /// CHECK-START: int TestRotate.rotateRightIntWithByteDistance(int, byte) builder (after)
  /// CHECK-NOT:                      InvokeStaticOrDirect

  private static int rotateRightIntWithByteDistance(int value, byte distance) {
    return Integer.rotateRight(value, distance);
  }

  /// CHECK-START: int TestRotate.$inline$rotateLeftBoolean(boolean, int) builder (after)
  /// CHECK:         <<ArgVal:z\d+>>  ParameterValue
  /// CHECK:         <<ArgDist:i\d+>> ParameterValue
  /// CHECK-DAG:     <<Zero:i\d+>>    IntConstant 0
  /// CHECK-DAG:     <<One:i\d+>>     IntConstant 1
  /// CHECK-DAG:     <<Val:i\d+>>     Phi [<<One>>,<<Zero>>]
  /// CHECK-DAG:     <<Result:i\d+>>  Rol [<<Val>>,<<ArgDist>>]
  /// CHECK-DAG:                      Return [<<Result>>]

  /// CHECK-START: int TestRotate.$inline$rotateLeftBoolean(boolean, int) builder (after)
  /// CHECK-NOT:                      InvokeStaticOrDirect

  /// CHECK-START: int TestRotate.$inline$rotateLeftBoolean(boolean, int) control_flow_simplifier (after)
  /// CHECK:         <<ArgVal:z\d+>>  ParameterValue
  /// CHECK:         <<ArgDist:i\d+>> ParameterValue
  /// CHECK-DAG:     <<Zero:i\d+>>    IntConstant 0
  /// CHECK-DAG:     <<One:i\d+>>     IntConstant 1
  /// CHECK-DAG:     <<SelVal:i\d+>>  Select [<<Zero>>,<<One>>,<<ArgVal>>]
  /// CHECK-DAG:     <<Result:i\d+>>  Rol [<<SelVal>>,<<ArgDist>>]
  /// CHECK-DAG:                      Return [<<Result>>]

  /// CHECK-START: int TestRotate.$inline$rotateLeftBoolean(boolean, int) control_flow_simplifier (after)
  /// CHECK-NOT:                      Phi

  /// CHECK-START: int TestRotate.$inline$rotateLeftBoolean(boolean, int) instruction_simplifier$before_codegen (after)
  /// CHECK:         <<ArgVal:z\d+>>  ParameterValue
  /// CHECK:         <<ArgDist:i\d+>> ParameterValue
  /// CHECK-DAG:     <<Result:i\d+>>  Rol [<<ArgVal>>,<<ArgDist>>]
  /// CHECK-DAG:                      Return [<<Result>>]

  /// CHECK-START: int TestRotate.$inline$rotateLeftBoolean(boolean, int) instruction_simplifier$before_codegen (after)
  /// CHECK-NOT:                      Select

  private static int $inline$rotateLeftBoolean(boolean value, int distance) {
    // Note: D8 would replace the ternary expression `value ? 1 : 0` with `value`
    // but explicit `if` is preserved.
    int src;
    if (value) {
      src = 1;
    } else {
      src = 0;
    }
    return Integer.rotateLeft(src, distance);
  }

  private static int $noinline$rotateLeftBoolean(boolean value, int distance) {
    // Note: D8 would replace the ternary expression `value ? 1 : 0` with `value`
    // but explicit `if` is preserved.
    int src;
    if (value) {
      src = 1;
    } else {
      src = 0;
    }
    return Integer.rotateLeft(src, distance);
  }

  public static void testRotateLeftBoolean() {
    for (int i = 0; i < 40; i++) {  // overshoot a bit
      int j = i & 31;
      expectEqualsInt(0, $inline$rotateLeftBoolean(false, i));
      expectEqualsInt(1 << j, $inline$rotateLeftBoolean(true, i));

      expectEqualsInt(0, $noinline$rotateLeftBoolean(false, i));
      expectEqualsInt(1 << j, $noinline$rotateLeftBoolean(true, i));
    }
  }

  public static void testRotateLeftByte() {
    expectEqualsInt(0x00000001, $inline$rotateLeftByte((byte)0x01, 0));
    expectEqualsInt(0x00000002, $inline$rotateLeftByte((byte)0x01, 1));
    expectEqualsInt(0x80000000, $inline$rotateLeftByte((byte)0x01, 31));
    expectEqualsInt(0x00000001, $inline$rotateLeftByte((byte)0x01, 32));  // overshoot
    expectEqualsInt(0xFFFFFF03, $inline$rotateLeftByte((byte)0x81, 1));
    expectEqualsInt(0xFFFFFE07, $inline$rotateLeftByte((byte)0x81, 2));
    expectEqualsInt(0x00000120, $inline$rotateLeftByte((byte)0x12, 4));
    expectEqualsInt(0xFFFF9AFF, $inline$rotateLeftByte((byte)0x9A, 8));

    expectEqualsInt(0x00000001, $noinline$rotateLeftByte((byte)0x01, 0));
    expectEqualsInt(0x00000002, $noinline$rotateLeftByte((byte)0x01, 1));
    expectEqualsInt(0x80000000, $noinline$rotateLeftByte((byte)0x01, 31));
    expectEqualsInt(0x00000001, $noinline$rotateLeftByte((byte)0x01, 32));  // overshoot
    expectEqualsInt(0xFFFFFF03, $noinline$rotateLeftByte((byte)0x81, 1));
    expectEqualsInt(0xFFFFFE07, $noinline$rotateLeftByte((byte)0x81, 2));
    expectEqualsInt(0x00000120, $noinline$rotateLeftByte((byte)0x12, 4));
    expectEqualsInt(0xFFFF9AFF, $noinline$rotateLeftByte((byte)0x9A, 8));

    for (int i = 0; i < 40; i++) {  // overshoot a bit
      int j = i & 31;
      expectEqualsInt(0x00000000, $inline$rotateLeftByte((byte)0x0000, i));
      expectEqualsInt(0xFFFFFFFF, $inline$rotateLeftByte((byte)0xFFFF, i));
      expectEqualsInt((1 << j), $inline$rotateLeftByte((byte)0x0001, i));
      expectEqualsInt((0x12 << j) | (0x12 >>> -j), $inline$rotateLeftByte((byte)0x12, i));

      expectEqualsInt(0x00000000, $noinline$rotateLeftByte((byte)0x0000, i));
      expectEqualsInt(0xFFFFFFFF, $noinline$rotateLeftByte((byte)0xFFFF, i));
      expectEqualsInt((1 << j), $noinline$rotateLeftByte((byte)0x0001, i));
      expectEqualsInt((0x12 << j) | (0x12 >>> -j), $noinline$rotateLeftByte((byte)0x12, i));
    }
  }

  public static void testRotateLeftShort() {
    expectEqualsInt(0x00000001, $inline$rotateLeftShort((short)0x0001, 0));
    expectEqualsInt(0x00000002, $inline$rotateLeftShort((short)0x0001, 1));
    expectEqualsInt(0x80000000, $inline$rotateLeftShort((short)0x0001, 31));
    expectEqualsInt(0x00000001, $inline$rotateLeftShort((short)0x0001, 32));  // overshoot
    expectEqualsInt(0xFFFF0003, $inline$rotateLeftShort((short)0x8001, 1));
    expectEqualsInt(0xFFFE0007, $inline$rotateLeftShort((short)0x8001, 2));
    expectEqualsInt(0x00012340, $inline$rotateLeftShort((short)0x1234, 4));
    expectEqualsInt(0xFF9ABCFF, $inline$rotateLeftShort((short)0x9ABC, 8));

    expectEqualsInt(0x00000001, $noinline$rotateLeftShort((short)0x0001, 0));
    expectEqualsInt(0x00000002, $noinline$rotateLeftShort((short)0x0001, 1));
    expectEqualsInt(0x80000000, $noinline$rotateLeftShort((short)0x0001, 31));
    expectEqualsInt(0x00000001, $noinline$rotateLeftShort((short)0x0001, 32));  // overshoot
    expectEqualsInt(0xFFFF0003, $noinline$rotateLeftShort((short)0x8001, 1));
    expectEqualsInt(0xFFFE0007, $noinline$rotateLeftShort((short)0x8001, 2));
    expectEqualsInt(0x00012340, $noinline$rotateLeftShort((short)0x1234, 4));
    expectEqualsInt(0xFF9ABCFF, $noinline$rotateLeftShort((short)0x9ABC, 8));

    for (int i = 0; i < 40; i++) {  // overshoot a bit
      int j = i & 31;
      expectEqualsInt(0x00000000, $inline$rotateLeftShort((short)0x0000, i));
      expectEqualsInt(0xFFFFFFFF, $inline$rotateLeftShort((short)0xFFFF, i));
      expectEqualsInt((1 << j), $inline$rotateLeftShort((short)0x0001, i));
      expectEqualsInt((0x1234 << j) | (0x1234 >>> -j), $inline$rotateLeftShort((short)0x1234, i));

      expectEqualsInt(0x00000000, $noinline$rotateLeftShort((short)0x0000, i));
      expectEqualsInt(0xFFFFFFFF, $noinline$rotateLeftShort((short)0xFFFF, i));
      expectEqualsInt((1 << j), $noinline$rotateLeftShort((short)0x0001, i));
      expectEqualsInt((0x1234 << j) | (0x1234 >>> -j), $noinline$rotateLeftShort((short)0x1234, i));
    }
  }

  public static void testRotateLeftChar() {
    expectEqualsInt(0x00000001, $inline$rotateLeftChar((char)0x0001, 0));
    expectEqualsInt(0x00000002, $inline$rotateLeftChar((char)0x0001, 1));
    expectEqualsInt(0x80000000, $inline$rotateLeftChar((char)0x0001, 31));
    expectEqualsInt(0x00000001, $inline$rotateLeftChar((char)0x0001, 32));  // overshoot
    expectEqualsInt(0x00010002, $inline$rotateLeftChar((char)0x8001, 1));
    expectEqualsInt(0x00020004, $inline$rotateLeftChar((char)0x8001, 2));
    expectEqualsInt(0x00012340, $inline$rotateLeftChar((char)0x1234, 4));
    expectEqualsInt(0x009ABC00, $inline$rotateLeftChar((char)0x9ABC, 8));
    expectEqualsInt(0x00FF0000, $inline$rotateLeftChar((char)0xFF00, 8));

    expectEqualsInt(0x00000001, $noinline$rotateLeftChar((char)0x0001, 0));
    expectEqualsInt(0x00000002, $noinline$rotateLeftChar((char)0x0001, 1));
    expectEqualsInt(0x80000000, $noinline$rotateLeftChar((char)0x0001, 31));
    expectEqualsInt(0x00000001, $noinline$rotateLeftChar((char)0x0001, 32));  // overshoot
    expectEqualsInt(0x00010002, $noinline$rotateLeftChar((char)0x8001, 1));
    expectEqualsInt(0x00020004, $noinline$rotateLeftChar((char)0x8001, 2));
    expectEqualsInt(0x00012340, $noinline$rotateLeftChar((char)0x1234, 4));
    expectEqualsInt(0x009ABC00, $noinline$rotateLeftChar((char)0x9ABC, 8));
    expectEqualsInt(0x00FF0000, $noinline$rotateLeftChar((char)0xFF00, 8));

    for (int i = 0; i < 40; i++) {  // overshoot a bit
      int j = i & 31;
      expectEqualsInt(0x00000000, $inline$rotateLeftChar((char)0x0000, i));
      expectEqualsInt((1 << j), $inline$rotateLeftChar((char)0x0001, i));
      expectEqualsInt((0x1234 << j) | (0x1234 >>> -j), $inline$rotateLeftChar((char)0x1234, i));

      expectEqualsInt(0x00000000, $noinline$rotateLeftChar((char)0x0000, i));
      expectEqualsInt((1 << j), $noinline$rotateLeftChar((char)0x0001, i));
      expectEqualsInt((0x1234 << j) | (0x1234 >>> -j), $noinline$rotateLeftChar((char)0x1234, i));
    }
  }

  public static void testRotateLeftInt() {
    expectEqualsInt(0x00000001, $inline$rotateLeftInt(0x00000001, 0));
    expectEqualsInt(0x00000002, $inline$rotateLeftInt(0x00000001, 1));
    expectEqualsInt(0x80000000, $inline$rotateLeftInt(0x00000001, 31));
    expectEqualsInt(0x00000001, $inline$rotateLeftInt(0x00000001, 32));  // overshoot
    expectEqualsInt(0x00000003, $inline$rotateLeftInt(0x80000001, 1));
    expectEqualsInt(0x00000006, $inline$rotateLeftInt(0x80000001, 2));
    expectEqualsInt(0x23456781, $inline$rotateLeftInt(0x12345678, 4));
    expectEqualsInt(0xBCDEF09A, $inline$rotateLeftInt(0x9ABCDEF0, 8));

    expectEqualsInt(0x00000001, $noinline$rotateLeftInt(0x00000001, 0));
    expectEqualsInt(0x00000002, $noinline$rotateLeftInt(0x00000001, 1));
    expectEqualsInt(0x80000000, $noinline$rotateLeftInt(0x00000001, 31));
    expectEqualsInt(0x00000001, $noinline$rotateLeftInt(0x00000001, 32));  // overshoot
    expectEqualsInt(0x00000003, $noinline$rotateLeftInt(0x80000001, 1));
    expectEqualsInt(0x00000006, $noinline$rotateLeftInt(0x80000001, 2));
    expectEqualsInt(0x23456781, $noinline$rotateLeftInt(0x12345678, 4));
    expectEqualsInt(0xBCDEF09A, $noinline$rotateLeftInt(0x9ABCDEF0, 8));
    expectEqualsInt(0x80000000, $noinline$rotateLeftIntConstant(0x00000001));
    expectEqualsInt(Integer.MIN_VALUE, $noinline$rotateLeftIntMulNegDistance(1, -2));

    for (int i = 0; i < 40; i++) {  // overshoot a bit
      int j = i & 31;
      expectEqualsInt(0x00000000, $inline$rotateLeftInt(0x00000000, i));
      expectEqualsInt(0xFFFFFFFF, $inline$rotateLeftInt(0xFFFFFFFF, i));
      expectEqualsInt(1 << j, $inline$rotateLeftInt(0x00000001, i));
      expectEqualsInt((0x12345678 << j) | (0x12345678 >>> -j),
                      $inline$rotateLeftInt(0x12345678, i));

      expectEqualsInt(0x00000000, $noinline$rotateLeftInt(0x00000000, i));
      expectEqualsInt(0xFFFFFFFF, $noinline$rotateLeftInt(0xFFFFFFFF, i));
      expectEqualsInt(1 << j, $noinline$rotateLeftInt(0x00000001, i));
      expectEqualsInt((0x12345678 << j) | (0x12345678 >>> -j),
                      $noinline$rotateLeftInt(0x12345678, i));
    }
  }

  public static void testRotateLeftLong() {
    expectEqualsLong(0x0000000000000001L, $inline$rotateLeftLong(0x0000000000000001L, 0));
    expectEqualsLong(0x0000000000000002L, $inline$rotateLeftLong(0x0000000000000001L, 1));
    expectEqualsLong(0x8000000000000000L, $inline$rotateLeftLong(0x0000000000000001L, 63));
    expectEqualsLong(0x8000000000000000L, $noinline$rotateLeftLongConstant(0x0000000000000001L));
    expectEqualsLong(0x0000000000000001L,
                     $inline$rotateLeftLong(0x0000000000000001L, 64));  // overshoot
    expectEqualsLong(0x0000000000000003L, $inline$rotateLeftLong(0x8000000000000001L, 1));
    expectEqualsLong(0x0000000000000006L, $inline$rotateLeftLong(0x8000000000000001L, 2));
    expectEqualsLong(0x23456789ABCDEF01L, $inline$rotateLeftLong(0x123456789ABCDEF0L, 4));
    expectEqualsLong(0x3456789ABCDEF012L, $inline$rotateLeftLong(0x123456789ABCDEF0L, 8));

    expectEqualsLong(0x0000000000000001L, $noinline$rotateLeftLong(0x0000000000000001L, 0));
    expectEqualsLong(0x0000000000000002L, $noinline$rotateLeftLong(0x0000000000000001L, 1));
    expectEqualsLong(0x8000000000000000L, $noinline$rotateLeftLong(0x0000000000000001L, 63));
    expectEqualsLong(0x0000000000000001L,
                     $noinline$rotateLeftLong(0x0000000000000001L, 64));  // overshoot
    expectEqualsLong(0x0000000000000003L, $noinline$rotateLeftLong(0x8000000000000001L, 1));
    expectEqualsLong(0x0000000000000006L, $noinline$rotateLeftLong(0x8000000000000001L, 2));
    expectEqualsLong(0x23456789ABCDEF01L, $noinline$rotateLeftLong(0x123456789ABCDEF0L, 4));
    expectEqualsLong(0x3456789ABCDEF012L, $noinline$rotateLeftLong(0x123456789ABCDEF0L, 8));
    for (int i = 0; i < 70; i++) {  // overshoot a bit
      int j = i & 63;
      expectEqualsLong(0x0000000000000000L, $inline$rotateLeftLong(0x0000000000000000L, i));
      expectEqualsLong(0xFFFFFFFFFFFFFFFFL, $inline$rotateLeftLong(0xFFFFFFFFFFFFFFFFL, i));
      expectEqualsLong(1L << j, $inline$rotateLeftLong(0x0000000000000001, i));
      expectEqualsLong((0x123456789ABCDEF0L << j) | (0x123456789ABCDEF0L >>> -j),
                       $inline$rotateLeftLong(0x123456789ABCDEF0L, i));

      expectEqualsLong(0x0000000000000000L, $noinline$rotateLeftLong(0x0000000000000000L, i));
      expectEqualsLong(0xFFFFFFFFFFFFFFFFL, $noinline$rotateLeftLong(0xFFFFFFFFFFFFFFFFL, i));
      expectEqualsLong(1L << j, $noinline$rotateLeftLong(0x0000000000000001, i));
      expectEqualsLong((0x123456789ABCDEF0L << j) | (0x123456789ABCDEF0L >>> -j),
                       $noinline$rotateLeftLong(0x123456789ABCDEF0L, i));
    }
  }

  /// CHECK-START: int TestRotate.rotateRightBoolean(boolean, int) builder (after)
  /// CHECK:         <<ArgVal:z\d+>>  ParameterValue
  /// CHECK:         <<ArgDist:i\d+>> ParameterValue
  /// CHECK-DAG:     <<Zero:i\d+>>    IntConstant 0
  /// CHECK-DAG:     <<One:i\d+>>     IntConstant 1
  /// CHECK-DAG:     <<Val:i\d+>>     Phi [<<One>>,<<Zero>>]
  /// CHECK-DAG:     <<Result:i\d+>>  Ror [<<Val>>,<<ArgDist>>]
  /// CHECK-DAG:                      Return [<<Result>>]

  /// CHECK-START: int TestRotate.rotateRightBoolean(boolean, int) builder (after)
  /// CHECK-NOT:                      InvokeStaticOrDirect

  /// CHECK-START: int TestRotate.rotateRightBoolean(boolean, int) control_flow_simplifier (after)
  /// CHECK:         <<ArgVal:z\d+>>  ParameterValue
  /// CHECK:         <<ArgDist:i\d+>> ParameterValue
  /// CHECK-DAG:     <<Zero:i\d+>>    IntConstant 0
  /// CHECK-DAG:     <<One:i\d+>>     IntConstant 1
  /// CHECK-DAG:     <<SelVal:i\d+>>  Select [<<Zero>>,<<One>>,<<ArgVal>>]
  /// CHECK-DAG:     <<Result:i\d+>>  Ror [<<SelVal>>,<<ArgDist>>]
  /// CHECK-DAG:                      Return [<<Result>>]

  /// CHECK-START: int TestRotate.rotateRightBoolean(boolean, int) control_flow_simplifier (after)
  /// CHECK-NOT:                     Phi

  /// CHECK-START: int TestRotate.rotateRightBoolean(boolean, int) instruction_simplifier$before_codegen (after)
  /// CHECK:         <<ArgVal:z\d+>>  ParameterValue
  /// CHECK:         <<ArgDist:i\d+>> ParameterValue
  /// CHECK-DAG:     <<Result:i\d+>>  Ror [<<ArgVal>>,<<ArgDist>>]
  /// CHECK-DAG:                      Return [<<Result>>]

  /// CHECK-START: int TestRotate.rotateRightBoolean(boolean, int) instruction_simplifier$before_codegen (after)
  /// CHECK-NOT:                     Select

  private static int rotateRightBoolean(boolean value, int distance) {
    // Note: D8 would replace the ternary expression `value ? 1 : 0` with `value`
    // but explicit `if` is preserved.
    int src;
    if (value) {
      src = 1;
    } else {
      src = 0;
    }
    return Integer.rotateRight(src, distance);
  }

  public static void testRotateRightBoolean() {
    for (int i = 0; i < 40; i++) {  // overshoot a bit
      int j = (-i) & 31;
      expectEqualsInt(0, rotateRightBoolean(false, i));
      expectEqualsInt(1 << j, rotateRightBoolean(true, i));
    }
  }

  public static void testRotateRightByte() {
    expectEqualsInt(0xFFFFFF80, rotateRightByte((byte)0x80, 0));
    expectEqualsInt(0x7FFFFFC0, rotateRightByte((byte)0x80, 1));
    expectEqualsInt(0xFFFFFF01, rotateRightByte((byte)0x80, 31));
    expectEqualsInt(0xFFFFFF80, rotateRightByte((byte)0x80, 32));  // overshoot
    expectEqualsInt(0xFFFFFFC0, rotateRightByte((byte)0x81, 1));
    expectEqualsInt(0x7FFFFFE0, rotateRightByte((byte)0x81, 2));
    expectEqualsInt(0x20000001, rotateRightByte((byte)0x12, 4));
    expectEqualsInt(0x9AFFFFFF, rotateRightByte((byte)0x9A, 8));
    for (int i = 0; i < 40; i++) {  // overshoot a bit
      int j = i & 31;
      expectEqualsInt(0x00000000, rotateRightByte((byte)0x00, i));
      expectEqualsInt(0xFFFFFFFF, rotateRightByte((byte)0xFF, i));
      expectEqualsInt(1 << (32 - j), rotateRightByte((byte)0x01, i));
      expectEqualsInt((0x12 >>> j) | (0x12 << -j), rotateRightByte((byte)0x12, i));
    }
  }

  public static void testRotateRightShort() {
    expectEqualsInt(0xFFFF8000, rotateRightShort((short)0x8000, 0));
    expectEqualsInt(0x7FFFC000, rotateRightShort((short)0x8000, 1));
    expectEqualsInt(0xFFFF0001, rotateRightShort((short)0x8000, 31));
    expectEqualsInt(0xFFFF8000, rotateRightShort((short)0x8000, 32));  // overshoot
    expectEqualsInt(0xFFFFC000, rotateRightShort((short)0x8001, 1));
    expectEqualsInt(0x7FFFE000, rotateRightShort((short)0x8001, 2));
    expectEqualsInt(0x40000123, rotateRightShort((short)0x1234, 4));
    expectEqualsInt(0xBCFFFF9A, rotateRightShort((short)0x9ABC, 8));
    for (int i = 0; i < 40; i++) {  // overshoot a bit
      int j = i & 31;
      expectEqualsInt(0x00000000, rotateRightShort((short)0x0000, i));
      expectEqualsInt(0xFFFFFFFF, rotateRightShort((short)0xFFFF, i));
      expectEqualsInt(1 << (32 - j), rotateRightShort((short)0x0001, i));
      expectEqualsInt((0x1234 >>> j) | (0x1234 << -j), rotateRightShort((short)0x1234, i));
    }
  }

  public static void testRotateRightChar() {
    expectEqualsInt(0x00008000, rotateRightChar((char)0x8000, 0));
    expectEqualsInt(0x00004000, rotateRightChar((char)0x8000, 1));
    expectEqualsInt(0x00010000, rotateRightChar((char)0x8000, 31));
    expectEqualsInt(0x00008000, rotateRightChar((char)0x8000, 32));  // overshoot
    expectEqualsInt(0x80004000, rotateRightChar((char)0x8001, 1));
    expectEqualsInt(0x40002000, rotateRightChar((char)0x8001, 2));
    expectEqualsInt(0x40000123, rotateRightChar((char)0x1234, 4));
    expectEqualsInt(0xBC00009A, rotateRightChar((char)0x9ABC, 8));
    for (int i = 0; i < 40; i++) {  // overshoot a bit
      int j = i & 31;
      expectEqualsInt(0x00000000, rotateRightChar((char)0x0000, i));
      expectEqualsInt(1 << (32 - j), rotateRightChar((char)0x0001, i));
      expectEqualsInt((0x1234 >>> j) | (0x1234 << -j), rotateRightChar((char)0x1234, i));
    }
  }

  public static void testRotateRightInt() {
    expectEqualsInt(0x80000000, rotateRightInt(0x80000000, 0));
    expectEqualsInt(0x40000000, rotateRightInt(0x80000000, 1));
    expectEqualsInt(0x00000001, rotateRightInt(0x80000000, 31));
    expectEqualsInt(0x80000000, rotateRightInt(0x80000000, 32));  // overshoot
    expectEqualsInt(0xC0000000, rotateRightInt(0x80000001, 1));
    expectEqualsInt(0x60000000, rotateRightInt(0x80000001, 2));
    expectEqualsInt(0x81234567, rotateRightInt(0x12345678, 4));
    expectEqualsInt(0xF09ABCDE, rotateRightInt(0x9ABCDEF0, 8));
    for (int i = 0; i < 40; i++) {  // overshoot a bit
      int j = i & 31;
      expectEqualsInt(0x00000000, rotateRightInt(0x00000000, i));
      expectEqualsInt(0xFFFFFFFF, rotateRightInt(0xFFFFFFFF, i));
      expectEqualsInt(0x80000000 >>> j, rotateRightInt(0x80000000, i));
      expectEqualsInt((0x12345678 >>> j) | (0x12345678 << -j), rotateRightInt(0x12345678, i));
    }
  }

  public static void testRotateRightLong() {
    expectEqualsLong(0x8000000000000000L, rotateRightLong(0x8000000000000000L, 0));
    expectEqualsLong(0x4000000000000000L, rotateRightLong(0x8000000000000000L, 1));
    expectEqualsLong(0x0000000000000001L, rotateRightLong(0x8000000000000000L, 63));
    expectEqualsLong(0x8000000000000000L, rotateRightLong(0x8000000000000000L, 64));  // overshoot
    expectEqualsLong(0xC000000000000000L, rotateRightLong(0x8000000000000001L, 1));
    expectEqualsLong(0x6000000000000000L, rotateRightLong(0x8000000000000001L, 2));
    expectEqualsLong(0x0123456789ABCDEFL, rotateRightLong(0x123456789ABCDEF0L, 4));
    expectEqualsLong(0xF0123456789ABCDEL, rotateRightLong(0x123456789ABCDEF0L, 8));
    for (int i = 0; i < 70; i++) {  // overshoot a bit
      int j = i & 63;
      expectEqualsLong(0x0000000000000000L, rotateRightLong(0x0000000000000000L, i));
      expectEqualsLong(0xFFFFFFFFFFFFFFFFL, rotateRightLong(0xFFFFFFFFFFFFFFFFL, i));
      expectEqualsLong(0x8000000000000000L >>> j, rotateRightLong(0x8000000000000000L, i));
      expectEqualsLong((0x123456789ABCDEF0L >>> j) | (0x123456789ABCDEF0L << -j),
                       rotateRightLong(0x123456789ABCDEF0L, i));
    }
  }


  public static void testRotateLeftIntWithByteDistance() {
    expectEqualsInt(0x00000001, $inline$rotateLeftIntWithByteDistance(0x00000001, (byte)0));
    expectEqualsInt(0x00000002, $inline$rotateLeftIntWithByteDistance(0x00000001, (byte)1));
    expectEqualsInt(0x80000000, $inline$rotateLeftIntWithByteDistance(0x00000001, (byte)31));
    expectEqualsInt(0x00000001,
                    $inline$rotateLeftIntWithByteDistance(0x00000001, (byte)32));  // overshoot
    expectEqualsInt(0x00000003, $inline$rotateLeftIntWithByteDistance(0x80000001, (byte)1));
    expectEqualsInt(0x00000006, $inline$rotateLeftIntWithByteDistance(0x80000001, (byte)2));
    expectEqualsInt(0x23456781, $inline$rotateLeftIntWithByteDistance(0x12345678, (byte)4));
    expectEqualsInt(0xBCDEF09A, $inline$rotateLeftIntWithByteDistance(0x9ABCDEF0, (byte)8));

    expectEqualsInt(0x00000001, $noinline$rotateLeftIntWithByteDistance(0x00000001, (byte)0));
    expectEqualsInt(0x00000002, $noinline$rotateLeftIntWithByteDistance(0x00000001, (byte)1));
    expectEqualsInt(0x80000000, $noinline$rotateLeftIntWithByteDistance(0x00000001, (byte)31));
    expectEqualsInt(0x00000001,
                    $noinline$rotateLeftIntWithByteDistance(0x00000001, (byte)32));  // overshoot
    expectEqualsInt(0x00000003, $noinline$rotateLeftIntWithByteDistance(0x80000001, (byte)1));
    expectEqualsInt(0x00000006, $noinline$rotateLeftIntWithByteDistance(0x80000001, (byte)2));
    expectEqualsInt(0x23456781, $noinline$rotateLeftIntWithByteDistance(0x12345678, (byte)4));
    expectEqualsInt(0xBCDEF09A, $noinline$rotateLeftIntWithByteDistance(0x9ABCDEF0, (byte)8));

    for (byte i = 0; i < 40; i++) {  // overshoot a bit
      byte j = (byte)(i & 31);
      expectEqualsInt(0x00000000, $inline$rotateLeftIntWithByteDistance(0x00000000, i));
      expectEqualsInt(0xFFFFFFFF, $inline$rotateLeftIntWithByteDistance(0xFFFFFFFF, i));
      expectEqualsInt(1 << j, $inline$rotateLeftIntWithByteDistance(0x00000001, i));
      expectEqualsInt((0x12345678 << j) | (0x12345678 >>> -j),
                      $inline$rotateLeftIntWithByteDistance(0x12345678, i));

      expectEqualsInt(0x00000000, $noinline$rotateLeftIntWithByteDistance(0x00000000, i));
      expectEqualsInt(0xFFFFFFFF, $noinline$rotateLeftIntWithByteDistance(0xFFFFFFFF, i));
      expectEqualsInt(1 << j, $noinline$rotateLeftIntWithByteDistance(0x00000001, i));
      expectEqualsInt((0x12345678 << j) | (0x12345678 >>> -j),
                      $noinline$rotateLeftIntWithByteDistance(0x12345678, i));
    }
  }

  public static void testRotateRightIntWithByteDistance() {
    expectEqualsInt(0x80000000, rotateRightIntWithByteDistance(0x80000000, (byte)0));
    expectEqualsInt(0x40000000, rotateRightIntWithByteDistance(0x80000000, (byte)1));
    expectEqualsInt(0x00000001, rotateRightIntWithByteDistance(0x80000000, (byte)31));
    expectEqualsInt(0x80000000,
                    rotateRightIntWithByteDistance(0x80000000, (byte)32));  // overshoot
    expectEqualsInt(0xC0000000, rotateRightIntWithByteDistance(0x80000001, (byte)1));
    expectEqualsInt(0x60000000, rotateRightIntWithByteDistance(0x80000001, (byte)2));
    expectEqualsInt(0x81234567, rotateRightIntWithByteDistance(0x12345678, (byte)4));
    expectEqualsInt(0xF09ABCDE, rotateRightIntWithByteDistance(0x9ABCDEF0, (byte)8));
    for (byte i = 0; i < 40; i++) {  // overshoot a bit
      byte j = (byte)(i & 31);
      expectEqualsInt(0x00000000, rotateRightIntWithByteDistance(0x00000000, i));
      expectEqualsInt(0xFFFFFFFF, rotateRightIntWithByteDistance(0xFFFFFFFF, i));
      expectEqualsInt(0x80000000 >>> j, rotateRightIntWithByteDistance(0x80000000, i));
      expectEqualsInt((0x12345678 >>> j) | (0x12345678 << -j),
                      rotateRightIntWithByteDistance(0x12345678, i));
    }
  }


  public static void main() {
    testRotateLeftBoolean();
    testRotateLeftByte();
    testRotateLeftShort();
    testRotateLeftChar();
    testRotateLeftInt();
    testRotateLeftLong();

    testRotateRightBoolean();
    testRotateRightByte();
    testRotateRightShort();
    testRotateRightChar();
    testRotateRightInt();
    testRotateRightLong();

    // Also exercise distance values with types other than int.
    testRotateLeftIntWithByteDistance();
    testRotateRightIntWithByteDistance();

    System.out.println("TestRotate passed");
  }


  private static void expectEqualsInt(int expected, int result) {
    if (expected != result) {
      throw new Error("Expected: " + expected + ", found: " + result);
    }
  }

  private static void expectEqualsLong(long expected, long result) {
    if (expected != result) {
      throw new Error("Expected: " + expected + ", found: " + result);
    }
  }
}
