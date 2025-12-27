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

// Note that $opt$ is a marker for the optimizing compiler to test
// it does compile the method, and that $noinline$ is a marker to
// test that it does not inline it.

public class SignumTest {

  public static void main() {
    signumFloat();
    signumFloat_noinline();
    signumDouble();
    signumDouble_noinline();
  }

  private static void signumFloat() {
    assertEquals(Math.signum(123.4f), 1.0f);
    assertEquals(Math.signum(-56.7f), -1.0f);
    assertEquals(Math.signum(7e30f), 1.0f);
    assertEquals(Math.signum(-0.3e30f), -1.0f);
    assertEquals(Math.signum(Float.MAX_VALUE), 1.0f);
    assertEquals(Math.signum(-Float.MAX_VALUE), -1.0f);
    assertEquals(Math.signum(Float.MIN_VALUE), 1.0f);
    assertEquals(Math.signum(-Float.MIN_VALUE), -1.0f);
    assertEquals(Math.signum(0.0f), 0.0f);
    assertEquals(Math.signum(-0.0f), -0.0f);
    assertEquals(Math.signum(Float.POSITIVE_INFINITY), 1.0f);
    assertEquals(Math.signum(Float.NEGATIVE_INFINITY), -1.0f);
    assertEquals(Math.signum(Float.NaN), Float.NaN);
    assertEquals(Math.signum(Float.MIN_NORMAL), 1.0f);
    assertEquals(Math.signum(-Float.MIN_NORMAL), -1.0f);
    assertEquals(Math.signum(0x0.0002P-126f), 1.0f);
    assertEquals(Math.signum(-0x0.0002P-126f), -1.0f);
  }

  private static float $opt$noinline$signumFloat(float a) {
    return Math.signum(a);
  }

  private static void signumFloat_noinline() {
    assertEquals($opt$noinline$signumFloat(123.4f), 1.0f);
    assertEquals($opt$noinline$signumFloat(-56.7f), -1.0f);
    assertEquals($opt$noinline$signumFloat(7e30f), 1.0f);
    assertEquals($opt$noinline$signumFloat(-0.3e30f), -1.0f);
    assertEquals($opt$noinline$signumFloat(Float.MAX_VALUE), 1.0f);
    assertEquals($opt$noinline$signumFloat(-Float.MAX_VALUE), -1.0f);
    assertEquals($opt$noinline$signumFloat(Float.MIN_VALUE), 1.0f);
    assertEquals($opt$noinline$signumFloat(-Float.MIN_VALUE), -1.0f);
    assertEquals($opt$noinline$signumFloat(0.0f), 0.0f);
    assertEquals($opt$noinline$signumFloat(-0.0f), -0.0f);
    assertEquals($opt$noinline$signumFloat(Float.POSITIVE_INFINITY), 1.0f);
    assertEquals($opt$noinline$signumFloat(Float.NEGATIVE_INFINITY), -1.0f);
    assertEquals($opt$noinline$signumFloat(Float.NaN), Float.NaN);
    assertEquals($opt$noinline$signumFloat(Float.MIN_NORMAL), 1.0f);
    assertEquals($opt$noinline$signumFloat(-Float.MIN_NORMAL), -1.0f);
    assertEquals($opt$noinline$signumFloat(0x0.0002P-126f), 1.0f);
    assertEquals($opt$noinline$signumFloat(-0x0.0002P-126f), -1.0f);
  }

  private static void signumDouble() {
    assertEquals(Math.signum(123.4d), 1.0d);
    assertEquals(Math.signum(-56.7d), -1.0d);
    assertEquals(Math.signum(7e30d), 1.0d);
    assertEquals(Math.signum(-0.3e30d), -1.0d);
    assertEquals(Math.signum(Double.MAX_VALUE), 1.0d);
    assertEquals(Math.signum(-Double.MAX_VALUE), -1.0d);
    assertEquals(Math.signum(Double.MIN_VALUE), 1.0d);
    assertEquals(Math.signum(-Double.MIN_VALUE), -1.0d);
    assertEquals(Math.signum(0.0d), 0.0d);
    assertEquals(Math.signum(-0.0d), -0.0d);
    assertEquals(Math.signum(Double.POSITIVE_INFINITY), 1.0d);
    assertEquals(Math.signum(Double.NEGATIVE_INFINITY), -1.0d);
    assertEquals(Math.signum(Double.NaN), Double.NaN);
    assertEquals(Math.signum(Double.MIN_NORMAL), 1.0d);
    assertEquals(Math.signum(-Double.MIN_NORMAL), -1.0d);
    assertEquals(Math.signum(0x0.00000001P-1022), 1.0d);
    assertEquals(Math.signum(-0x0.00000001P-1022), -1.0d);
  }

  private static double $opt$noinline$signumDouble(double a) {
    return Math.signum(a);
  }

  private static void signumDouble_noinline() {
    assertEquals($opt$noinline$signumDouble(123.4d), 1.0d);
    assertEquals($opt$noinline$signumDouble(-56.7d), -1.0d);
    assertEquals($opt$noinline$signumDouble(7e30d), 1.0d);
    assertEquals($opt$noinline$signumDouble(-0.3e30d), -1.0d);
    assertEquals($opt$noinline$signumDouble(Double.MAX_VALUE), 1.0d);
    assertEquals($opt$noinline$signumDouble(-Double.MAX_VALUE), -1.0d);
    assertEquals($opt$noinline$signumDouble(Double.MIN_VALUE), 1.0d);
    assertEquals($opt$noinline$signumDouble(-Double.MIN_VALUE), -1.0d);
    assertEquals($opt$noinline$signumDouble(0.0d), 0.0d);
    assertEquals($opt$noinline$signumDouble(-0.0d), -0.0d);
    assertEquals($opt$noinline$signumDouble(Double.POSITIVE_INFINITY), 1.0d);
    assertEquals($opt$noinline$signumDouble(Double.NEGATIVE_INFINITY), -1.0d);
    assertEquals($opt$noinline$signumDouble(Double.NaN), Double.NaN);
    assertEquals($opt$noinline$signumDouble(Double.MIN_NORMAL), 1.0d);
    assertEquals($opt$noinline$signumDouble(-Double.MIN_NORMAL), -1.0d);
    assertEquals($opt$noinline$signumDouble(0x0.00000001P-1022), 1.0d);
    assertEquals($opt$noinline$signumDouble(-0x0.00000001P-1022), -1.0d);
  }

  private static void assertEquals(float calculated, float expected) {
    if (0 != Float.compare(calculated, expected)) {
      throw new Error("Expected: " + expected + ", found: " + calculated);
    }
  }

  private static void assertEquals(double calculated, double expected) {
    if (0 != Double.compare(calculated, expected)) {
      throw new Error("Expected: " + expected + ", found: " + calculated);
    }
  }

}
