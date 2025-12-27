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

class Main {

    private static class ByteArrayTests {
        public static byte[] init(byte[] arr) {
            for (byte i = 0; i < arr.length; i++) {
                arr[i] = (byte) i;
            }
            return arr;
        }

        public static void assertEquals(int expected, int actual) {
            if (expected != actual) {
                throw new Error("Expected " + expected + ", got " + actual);
            }
        }

        public static byte[] createArrayOfZeroes(int size) {
            byte[] arr = new byte[size];
            return arr;
        }

        public static byte[] createIncrementedArray(int size) {
            byte[] arr = new byte[size];
            init(arr);
            return arr;
        }

        public static void checkIncrementedArray(byte[] dst) {
            for (byte i = 0; i < dst.length; i++) {
                assertEquals(i, dst[i]);
            }
        }

        public static void $noinline$test1(byte[] src, byte[] dst) {
            System.arraycopy(src, 0, dst, 0, 15);
        }

        public static void $noinline$test2(byte[] src, byte[] dst) {
            System.arraycopy(src, 0, dst, 0, 127);
        }

        public static byte[] $noinline$test3() {
            byte size = 15;
            byte[] src = new byte[size];
            src = init(src);
            byte[] dst = new byte[size];

            System.arraycopy(src, 0, dst, 0, size);
            return dst;
        }

        public static byte[] $noinline$test4() {
            int size = 127;
            byte[] src = new byte[size];
            src = init(src);
            byte[] dst = new byte[size];

            System.arraycopy(src, 0, dst, 0, size);
            return dst;
        }

        public static byte[] $noinline$test5() {
            byte[] src = new byte[80];
            src = init(src);
            byte[] dst = new byte[80];

            System.arraycopy(src, 0, dst, 0, 80);
            return dst;
        }

        public static byte[] $noinline$test6() {
            byte[] src = new byte[127];
            src = init(src);
            byte[] dst = new byte[127];

            System.arraycopy(src, 0, dst, 100, 27);
            return dst;
        }
        public static void check6(byte[] dst) {
            for (byte i = 0; i < 100; i++) {
                assertEquals(0, dst[i]);
            }
            for (byte i = 100; i < 127; i++) {
                assertEquals(i - 100, dst[i]);
            }
        }

        public static byte[] $noinline$test7() {
            byte[] src = new byte[127];
            src = init(src);

            System.arraycopy(src, 0, src, 100, 27);
            return src;
        }
        public static void check7(byte[] dst) {
            for (byte i = 0; i < 100; i++) {
                assertEquals(i, dst[i]);
            }
            for (byte i = 100; i < 127; i++) {
                assertEquals(i - 100, dst[i]);
            }
        }

        public static byte[] $noinline$test8() {
            byte[] src = new byte[127];
            src = init(src);

            System.arraycopy(src, 100, src, 0, 27);
            return src;
        }
        public static void check8(byte[] dst) {
            for (byte i = 0; i < 27; i++) {
                assertEquals(100 + i, dst[i]);
            }
            for (byte i = 27; i < 127; i++) {
                assertEquals(i, dst[i]);
            }
        }

        public static void $noinline$test9(byte[] src, byte[] dst, byte i) {
            System.arraycopy(src, 0, dst, 0, i);
        }

        public static void runTests() {
            System.out.print("[ByteArrayTests]: ");

            byte[] src15 = createIncrementedArray(15);
            byte[] dst15 = createArrayOfZeroes(15);
            $noinline$test1(src15, dst15);
            checkIncrementedArray(dst15);

            byte[] src150 = createIncrementedArray(127);
            byte[] dst150 = createArrayOfZeroes(127);
            $noinline$test2(src150, dst150);
            checkIncrementedArray(dst150);

            checkIncrementedArray($noinline$test3());
            checkIncrementedArray($noinline$test4());
            checkIncrementedArray($noinline$test5());

            check6($noinline$test6());
            check7($noinline$test7());
            check8($noinline$test8());

            for (byte i = 1; i < 127; i++) {
                byte[] src = createIncrementedArray(i);
                byte[] dst = createArrayOfZeroes(i);
                $noinline$test9(src, dst, i);
                checkIncrementedArray(dst);
            }
            System.out.println("passed");
        }
    }

    private static class CharArrayTests {

        public static char[] init(char[] arr) {
            for (int i = 0; i < arr.length; i++) {
                arr[i] = (char) i;
            }
            return arr;
        }

        public static void assertEquals(int expected, int actual) {
            if (expected != actual) {
                throw new Error("Expected " + expected + ", got " + actual);
            }
        }

        public static char[] createArrayOfZeroes(int size) {
            char[] arr = new char[size];
            return arr;
        }

        public static char[] createIncrementedArray(int size) {
            char[] arr = new char[size];
            init(arr);
            return arr;
        }

        public static void checkIncrementedArray(char[] dst) {
            for (int i = 0; i < dst.length; i++) {
                assertEquals(i, dst[i]);
            }
        }

        public static void $noinline$test1(char[] src, char[] dst) {
            System.arraycopy(src, 0, dst, 0, 15);
        }

        public static void $noinline$test2(char[] src, char[] dst) {
            System.arraycopy(src, 0, dst, 0, 150);
        }

        public static char[] $noinline$test3() {
            int size = 15;
            char[] src = new char[size];
            src = init(src);
            char[] dst = new char[size];

            System.arraycopy(src, 0, dst, 0, size);
            return dst;
        }

        public static char[] $noinline$test4() {
            int size = 150;
            char[] src = new char[size];
            src = init(src);
            char[] dst = new char[size];

            System.arraycopy(src, 0, dst, 0, size);
            return dst;
        }

        public static char[] $noinline$test5() {
            char[] src = new char[80];
            src = init(src);
            char[] dst = new char[80];

            System.arraycopy(src, 0, dst, 0, 80);
            return dst;
        }

        public static char[] $noinline$test6() {
            char[] src = new char[150];
            src = init(src);
            char[] dst = new char[150];

            System.arraycopy(src, 0, dst, 100, 50);
            return dst;
        }
        public static void check6(char[] dst) {
            for (int i = 0; i < 100; i++) {
                assertEquals(0, dst[i]);
            }
            for (int i = 100; i < 150; i++) {
                assertEquals(i-100, dst[i]);
            }
        }

        public static char[] $noinline$test7() {
            char[] src = new char[150];
            src = init(src);

            System.arraycopy(src, 0, src, 100, 50);
            return src;
        }
        public static void check7(char[] dst) {
            for (int i = 0; i < 100; i++) {
                assertEquals(i, dst[i]);
            }
            for (int i = 100; i < 150; i++) {
                assertEquals(i - 100, dst[i]);
            }
        }

        public static char[] $noinline$test8() {
            char[] src = new char[150];
            src = init(src);

            System.arraycopy(src, 100, src, 0, 50);
            return src;
        }
        public static void check8(char[] dst) {
            for (int i = 0; i < 50; i++) {
                assertEquals(100 + i, dst[i]);
            }
            for (int i = 50; i < 150; i++) {
                assertEquals(i, dst[i]);
            }
        }

        public static void $noinline$test9(char[] src, char[] dst, int i) {
            System.arraycopy(src, 0, dst, 0, i);
        }

        public static void runTests() {
            System.out.print("[CharArrayTests]: ");

            char[] src15 = createIncrementedArray(15);
            char[] dst15 = createArrayOfZeroes(15);
            $noinline$test1(src15, dst15);
            checkIncrementedArray(dst15);

            char[] src150 = createIncrementedArray(150);
            char[] dst150 = createArrayOfZeroes(150);
            $noinline$test2(src150, dst150);
            checkIncrementedArray(dst150);

            checkIncrementedArray($noinline$test3());
            checkIncrementedArray($noinline$test4());
            checkIncrementedArray($noinline$test5());

            check6($noinline$test6());
            check7($noinline$test7());
            check8($noinline$test8());

            for (int i = 1; i < 256; i++) {
                char[] src = createIncrementedArray(i);
                char[] dst = createArrayOfZeroes(i);
                $noinline$test9(src, dst, i);
                checkIncrementedArray(dst);
            }

            System.out.println("passed");
        }
    }

    private static class IntArrayTests {
        public static int[] init(int[] arr) {
            for (int i = 0; i < arr.length; i++) {
                arr[i] = (int) i;
            }
            return arr;
        }

        public static void assertEquals(int expected, int actual) {
            if (expected != actual) {
                throw new Error("Expected " + expected + ", got " + actual);
            }
        }

        public static int[] createArrayOfZeroes(int size) {
            int[] arr = new int[size];
            return arr;
        }

        public static int[] createIncrementedArray(int size) {
            int[] arr = new int[size];
            init(arr);
            return arr;
        }

        public static void checkIncrementedArray(int[] dst) {
            for (int i = 0; i < dst.length; i++) {
                assertEquals(i, dst[i]);
            }
        }

        public static void $noinline$test1(int[] src, int[] dst) {
            System.arraycopy(src, 0, dst, 0, 15);
        }

        public static void $noinline$test2(int[] src, int[] dst) {
            System.arraycopy(src, 0, dst, 0, 150);
        }

        public static int[] $noinline$test3() {
            int size = 15;
            int[] src = new int[size];
            src = init(src);
            int[] dst = new int[size];

            System.arraycopy(src, 0, dst, 0, size);
            return dst;
        }

        public static int[] $noinline$test4() {
            int size = 150;
            int[] src = new int[size];
            src = init(src);
            int[] dst = new int[size];

            System.arraycopy(src, 0, dst, 0, size);
            return dst;
        }

        public static int[] $noinline$test5() {
            int[] src = new int[80];
            src = init(src);
            int[] dst = new int[80];

            System.arraycopy(src, 0, dst, 0, 80);
            return dst;
        }

        public static int[] $noinline$test6() {
            int[] src = new int[150];
            src = init(src);
            int[] dst = new int[150];

            System.arraycopy(src, 0, dst, 100, 50);
            return dst;
        }
        public static void check6(int[] dst) {
            for (int i = 0; i < 100; i++) {
                assertEquals(0, dst[i]);
            }
            for (int i = 100; i < 150; i++) {
                assertEquals(i-100, dst[i]);
            }
        }

        public static int[] $noinline$test7() {
            int[] src = new int[150];
            src = init(src);

            System.arraycopy(src, 0, src, 100, 50);
            return src;
        }
        public static void check7(int[] dst) {
            for (int i = 0; i < 100; i++) {
                assertEquals(i, dst[i]);
            }
            for (int i = 100; i < 150; i++) {
                assertEquals(i - 100, dst[i]);
            }
        }

        public static int[] $noinline$test8() {
            int[] src = new int[150];
            src = init(src);

            System.arraycopy(src, 100, src, 0, 50);
            return src;
        }
        public static void check8(int[] dst) {
            for (int i = 0; i < 50; i++) {
                assertEquals(100 + i, dst[i]);
            }
            for (int i = 50; i < 150; i++) {
                assertEquals(i, dst[i]);
            }
        }

        public static void $noinline$test9(int[] src, int[] dst, int i) {
            System.arraycopy(src, 0, dst, 0, i);
        }

        public static void runTests() {
            System.out.print("[IntArrayTests]: ");

            int[] src15 = createIncrementedArray(15);
            int[] dst15 = createArrayOfZeroes(15);
            $noinline$test1(src15, dst15);
            checkIncrementedArray(dst15);

            int[] src150 = createIncrementedArray(150);
            int[] dst150 = createArrayOfZeroes(150);
            $noinline$test2(src150, dst150);
            checkIncrementedArray(dst150);

            checkIncrementedArray($noinline$test3());
            checkIncrementedArray($noinline$test4());
            checkIncrementedArray($noinline$test5());

            check6($noinline$test6());
            check7($noinline$test7());
            check8($noinline$test8());

            for (int i = 1; i < 256; i++) {
                int[] src = createIncrementedArray(i);
                int[] dst = createArrayOfZeroes(i);
                $noinline$test9(src, dst, i);
                checkIncrementedArray(dst);
            }

            System.out.println("passed");
        }
    }

    public static void main(String[] args) {
        ByteArrayTests.runTests();
        CharArrayTests.runTests();
        IntArrayTests.runTests();
    }
}
