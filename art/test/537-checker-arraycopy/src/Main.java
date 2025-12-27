/*
 * Copyright (C) 2015 The Android Open Source Project
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
  private char[] arrChar;
  private byte[] arrByte;
  private int[] arrInt;

  public static void main(String[] args) {
    arraycopy();
    try {
      arraycopy(new Object());
      throw new Error("Should not be here");
    } catch (ArrayStoreException ase) {
      // Ignore.
    }
    try {
      arraycopy(null);
      throw new Error("Should not be here");
    } catch (NullPointerException npe) {
      // Ignore.
    }

    try {
      arraycopy(new Object[1]);
      throw new Error("Should not be here");
    } catch (ArrayIndexOutOfBoundsException aiooe) {
      // Ignore.
    }

    arraycopy(new Object[2]);
    arraycopy(new Object[2], 0);

    try {
      arraycopy(new Object[1], 1);
      throw new Error("Should not be here");
    } catch (ArrayIndexOutOfBoundsException aiooe) {
      // Ignore.
    }
  }

  /// CHECK-START-X86_64: void Main.arraycopy() disassembly (after)
  /// CHECK:          InvokeStaticOrDirect intrinsic:SystemArrayCopy
  /// CHECK-NOT:      test {{^[^\[].*}}, {{^[^\[].*}}
  /// CHECK-NOT:      call
  /// CHECK:          ReturnVoid
  // Checks that the call is intrinsified and that there is no register test instruction
  // when we know the source and destination are not null.
  public static void arraycopy() {
    Object[] obj = new Object[4];
    System.arraycopy(obj, 1, obj, 0, 1);
  }

  public static void arraycopy(Object obj) {
    System.arraycopy(obj, 1, obj, 0, 1);
  }

  // Test case for having enough registers on x86 for the arraycopy intrinsic.
  /// CHECK-START-X86: void Main.arraycopy(java.lang.Object[], int) disassembly (after)
  /// CHECK:          InvokeStaticOrDirect intrinsic:SystemArrayCopy
  /// CHECK-NOT:      mov {{[a-z]+}}, [esp + {{[0-9]+}}]
  /// CHECK:          ReturnVoid
  public static void arraycopy(Object[] obj, int pos) {
    System.arraycopy(obj, pos, obj, 0, obj.length);
  }

  // Test case for having enough registers on x86 for the arraycopy intrinsic
  // when an input is passed twice.
  /// CHECK-START-X86: int Main.arraycopy2(java.lang.Object[], int) disassembly (after)
  /// CHECK:          InvokeStaticOrDirect intrinsic:SystemArrayCopy
  /// CHECK-NOT:      mov {{[a-z]+}}, [esp + {{[0-9]+}}]
  /// CHECK:          Return
  public static int arraycopy2(Object[] obj, int pos) {
    System.arraycopy(obj, pos, obj, pos - 1, obj.length);
    return pos;
  }

  // Test case for not having enough registers on x86. The arraycopy intrinsic
  // will ask for length to be in stack and load it.
  /// CHECK-START-X86: int Main.arraycopy3(java.lang.Object[], java.lang.Object[], int, int, int) disassembly (after)
  /// CHECK:          InvokeStaticOrDirect intrinsic:SystemArrayCopy
  /// CHECK:          mov {{[a-z]+}}, [esp + {{[0-9]+}}]
  /// CHECK:          Return
  public static int arraycopy3(Object[] obj1, Object[] obj2, int input1, int input3, int input4) {
    System.arraycopy(obj1, input1, obj2, input3, input4);
    System.out.println(obj1);
    System.out.println(obj2);
    return input1 + input3 + input4;
  }

  // Test case for Char specialization when destination array is non-null.
  //
  /// CHECK-START-RISCV64: void Main.arraycopyCharDstNonNull() disassembly (after)
  /// CHECK:          InvokeStaticOrDirect intrinsic:SystemArrayCopyChar
  /// CHECK-NEXT:     beq <<reg1:t\d+>>, <<reg2:a\d+>>,
  /// CHECK-NEXT:     beqz <<reg1>>,
  /// CHECK-NOT:      beqz <<reg2>>,
  /// CHECK:          ReturnVoid
  //
  /// CHECK-START-ARM64: void Main.arraycopyCharDstNonNull() disassembly (after)
  /// CHECK:          InvokeStaticOrDirect intrinsic:SystemArrayCopyChar
  /// CHECK-NEXT:     cmp <<reg1:\w+>>, <<reg2:\w+>>
  /// CHECK-NEXT:     b.eq
  /// CHECK-NEXT:     cbz <<reg1>>,
  /// CHECK-NOT:      cbz <<reg2>>,
  /// CHECK:          ReturnVoid
  //
  /// CHECK-START-ARM: void Main.arraycopyCharDstNonNull() disassembly (after)
  /// CHECK:          InvokeStaticOrDirect intrinsic:SystemArrayCopyChar
  /// CHECK-NEXT:     cmp <<reg1:\w+>>, <<reg2:\w+>>
  /// CHECK-NEXT:     beq{{\.w?}}
  /// CHECK-NEXT:     cmp <<reg1>>, #0
  /// CHECK-NEXT:     bne{{\.w?}}
  /// CHECK-NOT:      cmp <<reg2>>, #0
  /// CHECK:          ReturnVoid
  //
  /// CHECK-START-{X86,X86_64}: void Main.arraycopyCharDstNonNull() disassembly (after)
  /// CHECK:          InvokeStaticOrDirect intrinsic:SystemArrayCopyChar
  /// CHECK-NEXT:     cmp <<reg1:\w+>>, <<reg2:\w+>>
  /// CHECK-NEXT:     jz/eq
  /// CHECK-NEXT:     test <<reg1>>, <<reg1>>
  /// CHECK-NEXT:     jz/eq
  /// CHECK-NOT:      test <<reg2>>, <<reg2>>
  /// CHECK:          ReturnVoid
  public void arraycopyCharDstNonNull() {
    char[] arrChar2 = new char[1];
    System.arraycopy(arrChar, 0, arrChar2, 0, 1);
  }

  // Test case for Char specialization when source and destination arrays are the same.
  //
  /// CHECK-START-RISCV64: void Main.arraycopyCharSameSrcDst() disassembly (after)
  /// CHECK:          InvokeStaticOrDirect intrinsic:SystemArrayCopyChar
  /// CHECK-NEXT:     auipc a0, {{\d+}}
  /// CHECK-NEXT:     {{lwu|ld}} a0, {{\d+}}(a0)
  /// CHECK-NEXT:     ld ra, {{\d+}}(a0)
  /// CHECK-NEXT:     c.jalr ra
  /// CHECK:          ReturnVoid
  //
  /// CHECK-START-ARM64: void Main.arraycopyCharSameSrcDst() disassembly (after)
  /// CHECK:          InvokeStaticOrDirect intrinsic:SystemArrayCopyChar
  /// CHECK-NEXT:     adrp
  /// CHECK-NEXT:     ldr
  /// CHECK-NEXT:     ldr lr,
  /// CHECK-NEXT:     blr lr
  /// CHECK:          ReturnVoid
  //
  /// CHECK-START-ARM: void Main.arraycopyCharSameSrcDst() disassembly (after)
  /// CHECK:          InvokeStaticOrDirect intrinsic:SystemArrayCopyChar
  /// CHECK:          add <<reg:\w+>>, pc
  /// CHECK-NEXT:     ldr <<reg>>, [<<reg>>]
  /// CHECK-NEXT:     ldr lr, [<<reg>>, #{{\d+}}]
  /// CHECK-NEXT:     blx lr
  /// CHECK:          ReturnVoid
  //
  /// CHECK-START-{X86,X86_64}: void Main.arraycopyCharSameSrcDst() disassembly (after)
  /// CHECK:          InvokeStaticOrDirect intrinsic:SystemArrayCopyChar
  /// CHECK-NEXT:     mov{{q?}} {{\w+}}, [{{RIP|ebp}} + {{\w+}}]
  /// CHECK-NEXT:     call [{{\w+}} + {{\d+}}]
  /// CHECK:          ReturnVoid
  public void arraycopyCharSameSrcDst() {
    System.arraycopy(arrChar, 1, arrChar, 0, 1);
  }

  // Test case for Byte specialization when destination array is non-null.
  //
  /// CHECK-START-RISCV64: void Main.arraycopyByteDstNonNull() disassembly (after)
  /// CHECK:          InvokeStaticOrDirect intrinsic:SystemArrayCopyByte
  /// CHECK-NEXT:     beq <<reg1:t\d+>>, <<reg2:a\d+>>,
  /// CHECK-NEXT:     beqz <<reg1>>,
  /// CHECK-NOT:      beqz <<reg2>>,
  /// CHECK:          ReturnVoid
  //
  /// CHECK-START-ARM64: void Main.arraycopyByteDstNonNull() disassembly (after)
  /// CHECK:          InvokeStaticOrDirect intrinsic:SystemArrayCopyByte
  /// CHECK-NEXT:     cmp <<reg1:\w+>>, <<reg2:\w+>>
  /// CHECK-NEXT:     b.eq
  /// CHECK-NEXT:     cbz <<reg1>>,
  /// CHECK-NOT:      cbz <<reg2>>,
  /// CHECK:          ReturnVoid
  //
  /// CHECK-START-ARM: void Main.arraycopyByteDstNonNull() disassembly (after)
  /// CHECK:          InvokeStaticOrDirect intrinsic:SystemArrayCopyByte
  /// CHECK-NEXT:     cmp <<reg1:\w+>>, <<reg2:\w+>>
  /// CHECK-NEXT:     beq{{\.w?}}
  /// CHECK-NEXT:     cmp <<reg1>>, #0
  /// CHECK-NEXT:     bne{{\.w?}}
  /// CHECK-NOT:      cmp <<reg2>>, #0
  /// CHECK:          ReturnVoid
  //
  /// CHECK-START-{X86,X86_64}: void Main.arraycopyByteDstNonNull() disassembly (after)
  /// CHECK:          InvokeStaticOrDirect intrinsic:SystemArrayCopyByte
  /// CHECK-NEXT:     cmp <<reg1:\w+>>, <<reg2:\w+>>
  /// CHECK-NEXT:     jz/eq
  /// CHECK-NEXT:     test <<reg1>>, <<reg1>>
  /// CHECK-NEXT:     jz/eq
  /// CHECK-NOT:      test <<reg2>>, <<reg2>>
  /// CHECK:          ReturnVoid
  public void arraycopyByteDstNonNull() {
    byte[] arrByte2 = new byte[1];
    System.arraycopy(arrByte, 0, arrByte2, 0, 1);
  }

  // Test case for Byte specialization when source and destination arrays are the same.
  /// CHECK-START-RISCV64: void Main.arraycopyByteSameSrcDst() disassembly (after)
  /// CHECK:          InvokeStaticOrDirect intrinsic:SystemArrayCopyByte
  /// CHECK-NEXT:     auipc a0, {{\d+}}
  /// CHECK-NEXT:     {{lwu|ld}} a0, {{\d+}}(a0)
  /// CHECK-NEXT:     ld ra, {{\d+}}(a0)
  /// CHECK-NEXT:     c.jalr ra
  /// CHECK:          ReturnVoid
  //
  /// CHECK-START-ARM64: void Main.arraycopyByteSameSrcDst() disassembly (after)
  /// CHECK:          InvokeStaticOrDirect intrinsic:SystemArrayCopyByte
  /// CHECK-NEXT:     adrp
  /// CHECK-NEXT:     ldr
  /// CHECK-NEXT:     ldr lr,
  /// CHECK-NEXT:     blr lr
  /// CHECK:          ReturnVoid
  //
  /// CHECK-START-ARM: void Main.arraycopyByteSameSrcDst() disassembly (after)
  /// CHECK:          InvokeStaticOrDirect intrinsic:SystemArrayCopyByte
  /// CHECK:          add <<reg:\w+>>, pc
  /// CHECK-NEXT:     ldr <<reg>>, [<<reg>>]
  /// CHECK-NEXT:     ldr lr, [<<reg>>, #{{\d+}}]
  /// CHECK-NEXT:     blx lr
  /// CHECK:          ReturnVoid
  //
  /// CHECK-START-{X86,X86_64}: void Main.arraycopyByteSameSrcDst() disassembly (after)
  /// CHECK:          InvokeStaticOrDirect intrinsic:SystemArrayCopyByte
  /// CHECK-NEXT:     mov{{q?}} {{\w+}}, [{{RIP|ebp}} + {{\w+}}]
  /// CHECK-NEXT:     call [{{\w+}} + {{\d+}}]
  /// CHECK:          ReturnVoid
  public void arraycopyByteSameSrcDst() {
    System.arraycopy(arrByte, 1, arrByte, 0, 1);
  }

  // Test case for Int specialization when destination array is non-null.
  //
  /// CHECK-START-RISCV64: void Main.arraycopyIntDstNonNull() disassembly (after)
  /// CHECK:          InvokeStaticOrDirect intrinsic:SystemArrayCopyInt
  /// CHECK-NEXT:     beq <<reg1:t\d+>>, <<reg2:a\d+>>,
  /// CHECK-NEXT:     beqz <<reg1>>,
  /// CHECK-NOT:      beqz <<reg2>>,
  /// CHECK:          ReturnVoid
  //
  /// CHECK-START-ARM64: void Main.arraycopyIntDstNonNull() disassembly (after)
  /// CHECK:          InvokeStaticOrDirect intrinsic:SystemArrayCopyInt
  /// CHECK-NEXT:     cmp <<reg1:\w+>>, <<reg2:\w+>>
  /// CHECK-NEXT:     b.eq
  /// CHECK-NEXT:     cbz <<reg1>>,
  /// CHECK-NOT:      cbz <<reg2>>,
  /// CHECK:          ReturnVoid
  //
  /// CHECK-START-ARM: void Main.arraycopyIntDstNonNull() disassembly (after)
  /// CHECK:          InvokeStaticOrDirect intrinsic:SystemArrayCopyInt
  /// CHECK-NEXT:     cmp <<reg1:\w+>>, <<reg2:\w+>>
  /// CHECK-NEXT:     beq{{\.w?}}
  /// CHECK-NEXT:     cmp <<reg1>>, #0
  /// CHECK-NEXT:     bne{{\.w?}}
  /// CHECK-NOT:      cmp <<reg2>>, #0
  /// CHECK:          ReturnVoid
  //
  /// CHECK-START-{X86,X86_64}: void Main.arraycopyIntDstNonNull() disassembly (after)
  /// CHECK:          InvokeStaticOrDirect intrinsic:SystemArrayCopyInt
  /// CHECK-NEXT:     cmp <<reg1:\w+>>, <<reg2:\w+>>
  /// CHECK-NEXT:     jz/eq
  /// CHECK-NEXT:     test <<reg1>>, <<reg1>>
  /// CHECK-NEXT:     jz/eq
  /// CHECK-NOT:      test <<reg2>>, <<reg2>>
  /// CHECK:          ReturnVoid
  public void arraycopyIntDstNonNull() {
    int[] arrInt2 = new int[1];
    System.arraycopy(arrInt, 0, arrInt2, 0, 1);
  }

  // Test case for Int specialization when source and destination arrays are the same.
  /// CHECK-START-RISCV64: void Main.arraycopyIntSameSrcDst() disassembly (after)
  /// CHECK:          InvokeStaticOrDirect intrinsic:SystemArrayCopyInt
  /// CHECK-NEXT:     auipc a0, {{\d+}}
  /// CHECK-NEXT:     {{lwu|ld}} a0, {{\d+}}(a0)
  /// CHECK-NEXT:     ld ra, {{\d+}}(a0)
  /// CHECK-NEXT:     c.jalr ra
  /// CHECK:          ReturnVoid
  //
  /// CHECK-START-ARM64: void Main.arraycopyIntSameSrcDst() disassembly (after)
  /// CHECK:          InvokeStaticOrDirect intrinsic:SystemArrayCopyInt
  /// CHECK-NEXT:     adrp
  /// CHECK-NEXT:     ldr
  /// CHECK-NEXT:     ldr lr,
  /// CHECK-NEXT:     blr lr
  /// CHECK:          ReturnVoid
  //
  /// CHECK-START-ARM: void Main.arraycopyIntSameSrcDst() disassembly (after)
  /// CHECK:          InvokeStaticOrDirect intrinsic:SystemArrayCopyInt
  /// CHECK:          add <<reg:\w+>>, pc
  /// CHECK-NEXT:     ldr <<reg>>, [<<reg>>]
  /// CHECK-NEXT:     ldr lr, [<<reg>>, #{{\d+}}]
  /// CHECK-NEXT:     blx lr
  /// CHECK:          ReturnVoid
  //
  /// CHECK-START-{X86,X86_64}: void Main.arraycopyIntSameSrcDst() disassembly (after)
  /// CHECK:          InvokeStaticOrDirect intrinsic:SystemArrayCopyInt
  /// CHECK-NEXT:     mov{{q?}} {{\w+}}, [{{RIP|ebp}} + {{\w+}}]
  /// CHECK-NEXT:     call [{{\w+}} + {{\d+}}]
  /// CHECK:          ReturnVoid
  public void arraycopyIntSameSrcDst() {
    System.arraycopy(arrInt, 1, arrInt, 0, 1);
  }
}
