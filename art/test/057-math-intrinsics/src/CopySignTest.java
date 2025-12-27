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

public class CopySignTest {

  public static void main() {
    copySignFloat();
    copySignDouble();
  }

  private static float $opt$noinline$copySignFloat(float a, float b) {
    return Math.copySign(a, b);
  }

  private static void copySignFloat() {
    float testCases [][] = {
      { +0.0f,
         Float.MIN_VALUE,
         0x0.fffffcP-126f,
         0x0.fffffeP-126f,
         Float.MIN_NORMAL,
         1.0f,
         3.0f,
         0x1.fffffcP+127f,
         Float.MAX_VALUE,
         Float.POSITIVE_INFINITY,
         Float.NaN
      },
      {  Float.NEGATIVE_INFINITY,
        -Float.MAX_VALUE,
        -3.0f,
        -1.0f,
        -Float.MIN_NORMAL,
        -0x0.fffffcP-126f,
        -0x0.fffffeP-126f,
        -Float.MIN_VALUE,
        -0.0f,
         Float.intBitsToFloat(0xFFC00000),  // "negative" NaN
        -0x1.fffffcP+127f
      }
    };

    for (int i = 0; i < 2; i++) {
      for (int j = 0; j < 2; j++) {
        for (int m = 0; m < testCases[i].length; m++) {
          for (int n = 0; n < testCases[j].length; n++) {
            float expected = (j == 0 ? 1.0f : -1.0f) * Math.abs(testCases[i][m]);

            float calculated = Math.copySign(testCases[i][m], testCases[j][n]);
            if (Float.isNaN(testCases[i][m])) {
              assertEquals(calculated, Float.NaN);
            } else if (Float.isNaN(testCases[j][n])) {
              assertEquals(Math.abs(calculated), Math.abs(testCases[i][m]));
            } else {
              assertEquals(calculated, expected);
            }

            calculated = $opt$noinline$copySignFloat(testCases[i][m], testCases[j][n]);
            if (Float.isNaN(testCases[i][m])) {
              assertEquals(calculated, Float.NaN);
            } else if (Float.isNaN(testCases[j][n])) {
              assertEquals(Math.abs(calculated), Math.abs(testCases[i][m]));
            } else {
              assertEquals(calculated, expected);
            }
          }
        }
      }
    }
  }

  private static double $opt$noinline$copySignDouble(double a, double b) {
    return Math.copySign(a, b);
  }

  private static void copySignDouble() {
    double testCases [][] = {
      { +0.0d,
         Double.MIN_VALUE,
         0x0.ffffffffffffeP-1022,
         0x0.fffffffffffffP-1022,
         Double.MIN_NORMAL,
         1.0d,
         3.0d,
         0x1.ffffffffffffeP+1023,
         Double.MAX_VALUE,
         Double.POSITIVE_INFINITY,
         Double.NaN
      },
      {  Double.NEGATIVE_INFINITY,
        -Double.MAX_VALUE,
        -3.0d,
        -1.0d,
        -Double.MIN_NORMAL,
        -0x0.ffffffffffffeP-1022,
        -0x0.fffffffffffffP-1022,
        -Double.MIN_VALUE,
        -0.0d,
         Double.longBitsToDouble(0xfff8000000000000L),  // "negative" NaN
        -0x1.ffffffffffffeP+1023
      }
    };

    for (int i = 0; i < 2; i++) {
      for (int j = 0; j < 2; j++) {
        for (int m = 0; m < testCases[i].length; m++) {
          for (int n = 0; n < testCases[j].length; n++) {
            double expected = (j == 0 ? 1.0f : -1.0f) * Math.abs(testCases[i][m]);

            double calculated = Math.copySign(testCases[i][m], testCases[j][n]);
            if (Double.isNaN(testCases[i][m])) {
              assertEquals(calculated, Double.NaN);
            } else if (Double.isNaN(testCases[j][n])) {
              assertEquals(Math.abs(calculated), Math.abs(testCases[i][m]));
            } else {
              assertEquals(calculated, expected);
            }

            calculated = $opt$noinline$copySignDouble(testCases[i][m], testCases[j][n]);
            if (Double.isNaN(testCases[i][m])) {
              assertEquals(calculated, Double.NaN);
            } else if (Double.isNaN(testCases[j][n])) {
              assertEquals(Math.abs(calculated), Math.abs(testCases[i][m]));
            } else {
              assertEquals(calculated, expected);
            }
          }
        }
      }
    }
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
