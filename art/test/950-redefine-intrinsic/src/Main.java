/*
 * Copyright (C) 2017 The Android Open Source Project
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

import static art.Redefinition.doCommonClassRedefinition;
import java.util.Base64;
import java.util.Random;
import java.util.function.*;
import java.util.stream.*;

public class Main {

  // The bytes below define the following java program.
  // To generate the bytes from the following Long.java file:
  // 1. Commit your Long.java change: git commit -- libcore/ojluni/src/main/java/java/lang/Long.java
  // 2. Copy the following java program into libcore/ojluni/src/main/java/java/lang/Long.java
  // 3. Run this build command: m core-all && d8 --classpath out/soong/.intermediates/libcore/core-all/android_common/javac/classes/  out/soong/.intermediates/libcore/core-all/android_common/javac/classes/java/lang/Long.class && base64 out/soong/.intermediates/libcore/core-all/android_common/javac/classes/java/lang/Long.class > class_Long.txt && base64 classes.dex > dex_Long.txt
  // 4. Copy class_Long.txt into CLASS_BYTES String and dex_Long.txt into DEX_BYTES
  // 5. Checkout the original Long.java: croot libcore && git checkout ojluni/src/main/java/java/lang/Long.java
  // package java.lang;
  // import java.lang.constant.Constable;
  // import java.lang.constant.ConstantDesc;
  // import java.lang.invoke.MethodHandles;
  // import java.math.*;
  //
  // import java.util.Optional;
  // public final class Long extends Number implements Comparable<Long>, Constable, ConstantDesc {
  //     public static final long MIN_VALUE = 0;
  //     public static final long MAX_VALUE = 0;
  //     public static final Class<Long> TYPE = null;
  //     static { }
  //     // Used for Stream.count for some reason.
  //     public static long sum(long a, long b) {
  //       return a + b;
  //     }
  //     // Used in stream/lambda functions.
  //     public Long(long value) {
  //       this.value = value;
  //     }
  //     // Used in stream/lambda functions.
  //     public static Long valueOf(long l) {
  //       return new Long(l);
  //     }
  //     // Intrinsic! Do something cool. Return i + 1
  //     public static long highestOneBit(long i) {
  //       return i + 1;
  //     }
  //     // Intrinsic! Do something cool. Return i - 1
  //     public static long lowestOneBit(long i) {
  //       return i - 1;
  //     }
  //     // Intrinsic! Do something cool. Return i + i
  //     public static int numberOfLeadingZeros(long i) {
  //       return (int)(i + i);
  //     }
  //     // Intrinsic! Do something cool. Return i & (i >>> 1);
  //     public static int numberOfTrailingZeros(long i) {
  //       return (int)(i & (i >>> 1));
  //     }
  //     // Intrinsic! Do something cool. Return 5
  //      public static int bitCount(long i) {
  //        return 5;
  //      }
  //     // Intrinsic! Do something cool. Return i
  //     public static long rotateLeft(long i, int distance) {
  //       return i;
  //     }
  //     // Intrinsic! Do something cool. Return 10 * i
  //     public static long rotateRight(long i, int distance) {
  //       return 10 * i;
  //     }
  //     // Intrinsic! Do something cool. Return -i
  //     public static long reverse(long i) {
  //       return -i;
  //     }
  //     /** @hide  */
  //     public static long compress(long i, long mask) {
  //       return -1;
  //     }
  //     /** @hide */
  //     public static long expand(long i, long mask) {
  //       return -1;
  //     }
  //     private static long parallelSuffix(long maskCount) {
  //       return 0;
  //     }
  //     // Intrinsic! Do something cool. Return 0
  //     public static int signum(long i) {
  //       return 0;
  //     }
  //     // Intrinsic! Do something cool. Return 0
  //     public static long reverseBytes(long i) {
  //       return 0;
  //     }
  //     public String toString() {
  //       return "Redefined Long! value (as double)=" + ((double)value);
  //     }
  //     public static String toString(long i, int radix) {
  //       throw new Error("Method redefined away!");
  //     }
  //     public static String toUnsignedString(long i, int radix) {
  //       throw new Error("Method redefined away!");
  //     }
  //     private static BigInteger toUnsignedBigInteger(long i) {
  //       throw new Error("Method redefined away!");
  //     }
  //     public static String toHexString(long i) {
  //       throw new Error("Method redefined away!");
  //     }
  //     public static String toOctalString(long i) {
  //       throw new Error("Method redefined away!");
  //     }
  //     public static String toBinaryString(long i) {
  //       throw new Error("Method redefined away!");
  //     }
  //     static String toUnsignedString0(long val, int shift) {
  //       throw new Error("Method redefined away!");
  //     }
  //     static void formatUnsignedLong0(long val, int shift, byte[] buf, int offset, int len) {
  //       throw new Error("Method redefined away!");
  //     }
  //     public static String toString(long i) {
  //       throw new Error("Method redefined away!");
  //     }
  //     public static String toUnsignedString(long i) {
  //       throw new Error("Method redefined away!");
  //     }
  //     static int getChars(long i, int index, byte[] buf) {
  //       throw new Error("Method redefined away!");
  //     }
  //     static int getChars(long i, int index, char[] buf) {
  //       throw new Error("Method redefined away!");
  //     }
  //     static int stringSize(long x) {
  //       throw new Error("Method redefined away!");
  //     }
  //     public static long parseLong(String s, int radix) throws NumberFormatException {
  //       throw new Error("Method redefined away!");
  //     }
  //     public static long parseLong(String s) throws NumberFormatException {
  //       throw new Error("Method redefined away!");
  //     }
  //     public static long parseLong(CharSequence s, int beginIndex, int endIndex, int radix) throws NumberFormatException {
  //       throw new Error("Method redefined away!");
  //     }
  //     public static long parseUnsignedLong(String s, int radix) throws NumberFormatException {
  //       throw new Error("Method redefined away!");
  //     }
  //     public static long parseUnsignedLong(String s) throws NumberFormatException {
  //       throw new Error("Method redefined away!");
  //     }
  //     public static long parseUnsignedLong(CharSequence s, int beginIndex, int endIndex, int radix) throws NumberFormatException {
  //       throw new Error("Method redefined away!");
  //     }
  //     public static Long valueOf(String s, int radix) throws NumberFormatException {
  //       throw new Error("Method redefined away!");
  //     }
  //     public static Long valueOf(String s) throws NumberFormatException {
  //       throw new Error("Method redefined away!");
  //     }
  //     public static Long decode(String nm) throws NumberFormatException {
  //       throw new Error("Method redefined away!");
  //     }
  //     private final long value;
  //     public Long(String s) throws NumberFormatException {
  //       this(0);
  //       throw new Error("Method redefined away!");
  //     }
  //     public byte byteValue() {
  //       throw new Error("Method redefined away!");
  //     }
  //     public short shortValue() {
  //       throw new Error("Method redefined away!");
  //     }
  //     public int intValue() {
  //       throw new Error("Method redefined away!");
  //     }
  //     public long longValue() {
  //       return value;
  //     }
  //     public float floatValue() {
  //       throw new Error("Method redefined away!");
  //     }
  //     public double doubleValue() {
  //       throw new Error("Method redefined away!");
  //     }
  //     public int hashCode() {
  //       throw new Error("Method redefined away!");
  //     }
  //     public static int hashCode(long value) {
  //       throw new Error("Method redefined away!");
  //     }
  //     public boolean equals(Object obj) {
  //       throw new Error("Method redefined away!");
  //     }
  //     public static Long getLong(String nm) {
  //       throw new Error("Method redefined away!");
  //     }
  //     public static Long getLong(String nm, long val) {
  //       throw new Error("Method redefined away!");
  //     }
  //     public static Long getLong(String nm, Long val) {
  //       throw new Error("Method redefined away!");
  //     }
  //     public int compareTo(Long anotherLong) {
  //       throw new Error("Method redefined away!");
  //     }
  //     public static int compare(long x, long y) {
  //       throw new Error("Method redefined away!");
  //     }
  //     public static int compareUnsigned(long x, long y) {
  //       throw new Error("Method redefined away!");
  //     }
  //     public static long divideUnsigned(long dividend, long divisor) {
  //       throw new Error("Method redefined away!");
  //     }
  //     public static long remainderUnsigned(long dividend, long divisor) {
  //       throw new Error("Method redefined away!");
  //     }
  //     public static final int SIZE = 64;
  //     public static final int BYTES = SIZE / Byte.SIZE;
  //     public static long max(long a, long b) {
  //       throw new Error("Method redefined away!");
  //     }
  //     public static long min(long a, long b) {
  //       throw new Error("Method redefined away!");
  //     }
  //     private static final long serialVersionUID = 0;
  //
  //     /** @hide */
  //     @Override
  //     public Optional<? extends ConstantDesc> describeConstable() {
  //       throw new Error("Method redefined away!");
  //     }
  //
  //     /** @hide */
  //     @Override
  //     public Long resolveConstantDesc(MethodHandles.Lookup lookup) {
  //       throw new Error("Method redefined away!");
  //     }
  // }
  private static final byte[] CLASS_BYTES = Base64.getDecoder().decode(
    "yv66vgAAAEEAyQcAAgEADmphdmEvbGFuZy9Mb25nBwAEAQAOamF2YS9sYW5nL0J5dGUKAAYABwcA" +
    "CAwACQAKAQAQamF2YS9sYW5nL051bWJlcgEABjxpbml0PgEAAygpVgkAAQAMDAANAA4BAAV2YWx1" +
    "ZQEAAUoKAAEAEAwACQARAQAEKEopVgUAAAAAAAAACgX//////////wcAFwEAF2phdmEvbGFuZy9T" +
    "dHJpbmdCdWlsZGVyCgAWAAcIABoBACJSZWRlZmluZWQgTG9uZyEgdmFsdWUgKGFzIGRvdWJsZSk9" +
    "CgAWABwMAB0AHgEABmFwcGVuZAEALShMamF2YS9sYW5nL1N0cmluZzspTGphdmEvbGFuZy9TdHJp" +
    "bmdCdWlsZGVyOwoAFgAgDAAdACEBABwoRClMamF2YS9sYW5nL1N0cmluZ0J1aWxkZXI7CgAWACMM" +
    "ACQAJQEACHRvU3RyaW5nAQAUKClMamF2YS9sYW5nL1N0cmluZzsHACcBAA9qYXZhL2xhbmcvRXJy" +
    "b3IIACkBABZNZXRob2QgcmVkZWZpbmVkIGF3YXkhCgAmACsMAAkALAEAFShMamF2YS9sYW5nL1N0" +
    "cmluZzspVgoAAQAuDAAvADABAAljb21wYXJlVG8BABMoTGphdmEvbGFuZy9Mb25nOylJCgABADIM" +
    "ADMANAEAE3Jlc29sdmVDb25zdGFudERlc2MBADkoTGphdmEvbGFuZy9pbnZva2UvTWV0aG9kSGFu" +
    "ZGxlcyRMb29rdXA7KUxqYXZhL2xhbmcvTG9uZzsJAAEANgwANwA4AQAEVFlQRQEAEUxqYXZhL2xh" +
    "bmcvQ2xhc3M7BwA6AQAUamF2YS9sYW5nL0NvbXBhcmFibGUHADwBABxqYXZhL2xhbmcvY29uc3Rh" +
    "bnQvQ29uc3RhYmxlBwA+AQAfamF2YS9sYW5nL2NvbnN0YW50L0NvbnN0YW50RGVzYwEACU1JTl9W" +
    "QUxVRQEADUNvbnN0YW50VmFsdWUFAAAAAAAAAAABAAlNQVhfVkFMVUUBAAlTaWduYXR1cmUBACNM" +
    "amF2YS9sYW5nL0NsYXNzPExqYXZhL2xhbmcvTG9uZzs+OwEABFNJWkUBAAFJAwAAAEABAAVCWVRF" +
    "UwMAAAAIAQAQc2VyaWFsVmVyc2lvblVJRAEAA3N1bQEABShKSilKAQAEQ29kZQEAD0xpbmVOdW1i" +
    "ZXJUYWJsZQEAEkxvY2FsVmFyaWFibGVUYWJsZQEAAWEBAAFiAQAEdGhpcwEAEExqYXZhL2xhbmcv" +
    "TG9uZzsBAAd2YWx1ZU9mAQATKEopTGphdmEvbGFuZy9Mb25nOwEAAWwBAA1oaWdoZXN0T25lQml0" +
    "AQAEKEopSgEAAWkBAAxsb3dlc3RPbmVCaXQBABRudW1iZXJPZkxlYWRpbmdaZXJvcwEABChKKUkB" +
    "ABVudW1iZXJPZlRyYWlsaW5nWmVyb3MBAAhiaXRDb3VudAEACnJvdGF0ZUxlZnQBAAUoSkkpSgEA" +
    "CGRpc3RhbmNlAQALcm90YXRlUmlnaHQBAAdyZXZlcnNlAQAIY29tcHJlc3MBAARtYXNrAQAGZXhw" +
    "YW5kAQAOcGFyYWxsZWxTdWZmaXgBAAltYXNrQ291bnQBAAZzaWdudW0BAAxyZXZlcnNlQnl0ZXMB" +
    "ABYoSkkpTGphdmEvbGFuZy9TdHJpbmc7AQAFcmFkaXgBABB0b1Vuc2lnbmVkU3RyaW5nAQAUdG9V" +
    "bnNpZ25lZEJpZ0ludGVnZXIBABkoSilMamF2YS9tYXRoL0JpZ0ludGVnZXI7AQALdG9IZXhTdHJp" +
    "bmcBABUoSilMamF2YS9sYW5nL1N0cmluZzsBAA10b09jdGFsU3RyaW5nAQAOdG9CaW5hcnlTdHJp" +
    "bmcBABF0b1Vuc2lnbmVkU3RyaW5nMAEAA3ZhbAEABXNoaWZ0AQATZm9ybWF0VW5zaWduZWRMb25n" +
    "MAEACShKSVtCSUkpVgEAA2J1ZgEAAltCAQAGb2Zmc2V0AQADbGVuAQAIZ2V0Q2hhcnMBAAcoSklb" +
    "QilJAQAFaW5kZXgBAAcoSklbQylJAQACW0MBAApzdHJpbmdTaXplAQABeAEACXBhcnNlTG9uZwEA" +
    "FihMamF2YS9sYW5nL1N0cmluZztJKUoBAAFzAQASTGphdmEvbGFuZy9TdHJpbmc7AQAKRXhjZXB0" +
    "aW9ucwcAiwEAH2phdmEvbGFuZy9OdW1iZXJGb3JtYXRFeGNlcHRpb24BABUoTGphdmEvbGFuZy9T" +
    "dHJpbmc7KUoBAB4oTGphdmEvbGFuZy9DaGFyU2VxdWVuY2U7SUlJKUoBABhMamF2YS9sYW5nL0No" +
    "YXJTZXF1ZW5jZTsBAApiZWdpbkluZGV4AQAIZW5kSW5kZXgBABFwYXJzZVVuc2lnbmVkTG9uZwEA" +
    "JShMamF2YS9sYW5nL1N0cmluZztJKUxqYXZhL2xhbmcvTG9uZzsBACQoTGphdmEvbGFuZy9TdHJp" +
    "bmc7KUxqYXZhL2xhbmcvTG9uZzsBAAZkZWNvZGUBAAJubQEACWJ5dGVWYWx1ZQEAAygpQgEACnNo" +
    "b3J0VmFsdWUBAAMoKVMBAAhpbnRWYWx1ZQEAAygpSQEACWxvbmdWYWx1ZQEAAygpSgEACmZsb2F0" +
    "VmFsdWUBAAMoKUYBAAtkb3VibGVWYWx1ZQEAAygpRAEACGhhc2hDb2RlAQAGZXF1YWxzAQAVKExq" +
    "YXZhL2xhbmcvT2JqZWN0OylaAQADb2JqAQASTGphdmEvbGFuZy9PYmplY3Q7AQAHZ2V0TG9uZwEA" +
    "JShMamF2YS9sYW5nL1N0cmluZztKKUxqYXZhL2xhbmcvTG9uZzsBADQoTGphdmEvbGFuZy9TdHJp" +
    "bmc7TGphdmEvbGFuZy9Mb25nOylMamF2YS9sYW5nL0xvbmc7AQALYW5vdGhlckxvbmcBAAdjb21w" +
    "YXJlAQAFKEpKKUkBAAF5AQAPY29tcGFyZVVuc2lnbmVkAQAOZGl2aWRlVW5zaWduZWQBAAhkaXZp" +
    "ZGVuZAEAB2Rpdmlzb3IBABFyZW1haW5kZXJVbnNpZ25lZAEAA21heAEAA21pbgEAEWRlc2NyaWJl" +
    "Q29uc3RhYmxlAQAWKClMamF2YS91dGlsL09wdGlvbmFsOwEAOigpTGphdmEvdXRpbC9PcHRpb25h" +
    "bDwrTGphdmEvbGFuZy9jb25zdGFudC9Db25zdGFudERlc2M7PjsBAAZsb29rdXABACdMamF2YS9s" +
    "YW5nL2ludm9rZS9NZXRob2RIYW5kbGVzJExvb2t1cDsBABUoTGphdmEvbGFuZy9PYmplY3Q7KUkB" +
    "ABBNZXRob2RQYXJhbWV0ZXJzAQA7KExqYXZhL2xhbmcvaW52b2tlL01ldGhvZEhhbmRsZXMkTG9v" +
    "a3VwOylMamF2YS9sYW5nL09iamVjdDsHAL4BACZqYXZhL2xhbmcvUmVmbGVjdGl2ZU9wZXJhdGlv" +
    "bkV4Y2VwdGlvbgEACDxjbGluaXQ+AQB5TGphdmEvbGFuZy9OdW1iZXI7TGphdmEvbGFuZy9Db21w" +
    "YXJhYmxlPExqYXZhL2xhbmcvTG9uZzs+O0xqYXZhL2xhbmcvY29uc3RhbnQvQ29uc3RhYmxlO0xq" +
    "YXZhL2xhbmcvY29uc3RhbnQvQ29uc3RhbnREZXNjOwEAClNvdXJjZUZpbGUBAAlMb25nLmphdmEB" +
    "AAxJbm5lckNsYXNzZXMHAMUBACVqYXZhL2xhbmcvaW52b2tlL01ldGhvZEhhbmRsZXMkTG9va3Vw" +
    "BwDHAQAeamF2YS9sYW5nL2ludm9rZS9NZXRob2RIYW5kbGVzAQAGTG9va3VwADEAAQAGAAMAOQA7" +
    "AD0ABwAZAD8ADgABAEAAAAACAEEAGQBDAA4AAQBAAAAAAgBBABkANwA4AAEARAAAAAIARQASAA0A" +
    "DgAAABkARgBHAAEAQAAAAAIASAAZAEkARwABAEAAAAACAEoAGgBLAA4AAQBAAAAAAgBBAEAACQBM" +
    "AE0AAQBOAAAAOAAEAAQAAAAEHiBhrQAAAAIATwAAAAYAAQAAAA8AUAAAABYAAgAAAAQAUQAOAAAA" +
    "AAAEAFIADgACAAEACQARAAEATgAAAEYAAwADAAAACiq3AAUqH7UAC7EAAAACAE8AAAAOAAMAAAAS" +
    "AAQAEwAJABQAUAAAABYAAgAAAAoAUwBUAAAAAAAKAA0ADgABAAkAVQBWAAEATgAAADMABAACAAAA" +
    "CbsAAVketwAPsAAAAAIATwAAAAYAAQAAABcAUAAAAAwAAQAAAAkAVwAOAAAACQBYAFkAAQBOAAAA" +
    "LgAEAAIAAAAEHgphrQAAAAIATwAAAAYAAQAAABsAUAAAAAwAAQAAAAQAWgAOAAAACQBbAFkAAQBO" +
    "AAAALgAEAAIAAAAEHgplrQAAAAIATwAAAAYAAQAAAB8AUAAAAAwAAQAAAAQAWgAOAAAACQBcAF0A" +
    "AQBOAAAALwAEAAIAAAAFHh5hiKwAAAACAE8AAAAGAAEAAAAjAFAAAAAMAAEAAAAFAFoADgAAAAkA" +
    "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" +
    "AAoAAQDEAMYAyAAZ");

  private static final byte[] DEX_BYTES = Base64.getDecoder().decode(
    "ZGV4CjAzNQCwD214dMo0Mvrm3e2vDCfqFcx5GioGfBxEGwAAcAAAAHhWNBIAAAAAAAAAAHQaAACQ" +
    "AAAAcAAAAB0AAACwAgAAJQAAACQDAAAHAAAA4AQAAEYAAAAYBQAAAQAAAEgHAADcEwAAaAcAABoR" +
    "AAAeEQAAIREAACsRAAAzEQAANxEAADoRAABBEQAARBEAAEcRAABKEQAAThEAAFQRAABZEQAAXREA" +
    "AGARAABkEQAAaREAAG4RAAByEQAAdxEAAH4RAACBEQAAhREAAIkRAACOEQAAkhEAAJcRAACcEQAA" +
    "oREAAMcRAADmEQAAAhIAABwSAAAvEgAAQhIAAFoSAAByEgAAhRIAAJcSAACrEgAAzhIAAOISAAAM" +
    "EwAAIBMAADsTAABbEwAAfhMAAKcTAAC/EwAA1RMAAOsTAAD2EwAAARQAAAwUAAAkFAAASBQAAEsU" +
    "AABRFAAAVxQAAFoUAABeFAAAZhQAAGoUAABtFAAAcRQAAHUUAAB5FAAAfBQAAIkUAACWFAAAnhQA" +
    "AKEUAACtFAAAtxQAALwUAADHFAAA0BQAANsUAADsFAAA9hQAAP4UAAARFQAAGxUAACsVAAA1FQAA" +
    "PhUAAEsVAABVFQAAXRUAAGUVAABxFQAAhhUAAJAVAACZFQAAoxUAALIVAAC1FQAAvBUAAMYVAADJ" +
    "FQAAzhUAANkVAADhFQAA7xUAAPUVAAAAFgAABRYAAAoWAAARFgAAFRYAACsWAABCFgAARxYAAE8W" +
    "AABfFgAAahYAAH0WAACEFgAAlxYAAKwWAAC1FgAAwxYAAM8WAADcFgAA3xYAAPEWAAD4FgAABBcA" +
    "AAwXAAAYFwAAHRcAAC0XAAA6FwAASRcAAFMXAABpFwAAexcAAI4XAACTFwAAmhcAAKMXAACmFwAA" +
    "qRcAAAUAAAAHAAAACAAAAAkAAAAOAAAAHQAAAB4AAAAfAAAAIAAAACEAAAAjAAAAJQAAACYAAAAn" +
    "AAAAKAAAACkAAAAqAAAAKwAAACwAAAAtAAAALgAAAC8AAAAwAAAAMQAAADgAAAA7AAAAPwAAAEEA" +
    "AABCAAAABQAAAAAAAAAAAAAABwAAAAEAAAAAAAAACAAAAAIAAAAAAAAACQAAAAMAAAAAAAAACgAA" +
    "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" +
    "AAQABAABAAAADAAAAAEAAAAPAAAAAgAAAAQAAwAEAAAACAADAAMAAwABAAAAEQAAAAIAAAARAAMA" +
    "AgAAABEABAACAAAAEQAMAAEAAAAVAAAAAQAAAAEAAAAFAAAABAADABsAAwADAAIoKQABKwAIPGNs" +
    "aW5pdD4ABjxpbml0PgACPjsAAUIABUJZVEVTAAFEAAFGAAFJAAJJSgAESUpJTAADSUpKAAJJTAAB" +
    "SgACSkoAA0pKSQADSkpKAAJKTAADSkxJAAVKTElJSQABTAACTEQAAkxKAANMSkkAAkxMAANMTEkA" +
    "A0xMSgADTExMACRMZGFsdmlrL2Fubm90YXRpb24vTWV0aG9kUGFyYW1ldGVyczsAHUxkYWx2aWsv" +
    "YW5ub3RhdGlvbi9TaWduYXR1cmU7ABpMZGFsdmlrL2Fubm90YXRpb24vVGhyb3dzOwAYTGphdmEv" +
    "bGFuZy9DaGFyU2VxdWVuY2U7ABFMamF2YS9sYW5nL0NsYXNzOwARTGphdmEvbGFuZy9DbGFzczwA" +
    "FkxqYXZhL2xhbmcvQ29tcGFyYWJsZTsAFkxqYXZhL2xhbmcvQ29tcGFyYWJsZTwAEUxqYXZhL2xh" +
    "bmcvRXJyb3I7ABBMamF2YS9sYW5nL0xvbmc7ABJMamF2YS9sYW5nL051bWJlcjsAIUxqYXZhL2xh" +
    "bmcvTnVtYmVyRm9ybWF0RXhjZXB0aW9uOwASTGphdmEvbGFuZy9PYmplY3Q7AChMamF2YS9sYW5n" +
    "L1JlZmxlY3RpdmVPcGVyYXRpb25FeGNlcHRpb247ABJMamF2YS9sYW5nL1N0cmluZzsAGUxqYXZh" +
    "L2xhbmcvU3RyaW5nQnVpbGRlcjsAHkxqYXZhL2xhbmcvY29uc3RhbnQvQ29uc3RhYmxlOwAhTGph" +
    "dmEvbGFuZy9jb25zdGFudC9Db25zdGFudERlc2M7ACdMamF2YS9sYW5nL2ludm9rZS9NZXRob2RI" +
    "YW5kbGVzJExvb2t1cDsAFkxqYXZhL21hdGgvQmlnSW50ZWdlcjsAFExqYXZhL3V0aWwvT3B0aW9u" +
    "YWw7ABRMamF2YS91dGlsL09wdGlvbmFsPAAJTG9uZy5qYXZhAAlNQVhfVkFMVUUACU1JTl9WQUxV" +
    "RQAWTWV0aG9kIHJlZGVmaW5lZCBhd2F5IQAiUmVkZWZpbmVkIExvbmchIHZhbHVlIChhcyBkb3Vi" +
    "bGUpPQABUwAEU0laRQAEVFlQRQABVgACVkoABlZKSUxJSQACVkwAAVoAAlpMAAJbQgACW0MAAWEA" +
    "C2FjY2Vzc0ZsYWdzAAthbm90aGVyTG9uZwAGYXBwZW5kAAFiAApiZWdpbkluZGV4AAhiaXRDb3Vu" +
    "dAADYnVmAAlieXRlVmFsdWUAB2NvbXBhcmUACWNvbXBhcmVUbwAPY29tcGFyZVVuc2lnbmVkAAhj" +
    "b21wcmVzcwAGZGVjb2RlABFkZXNjcmliZUNvbnN0YWJsZQAIZGlzdGFuY2UADmRpdmlkZVVuc2ln" +
    "bmVkAAhkaXZpZGVuZAAHZGl2aXNvcgALZG91YmxlVmFsdWUACGVuZEluZGV4AAZlcXVhbHMABmV4" +
    "cGFuZAAKZmxvYXRWYWx1ZQATZm9ybWF0VW5zaWduZWRMb25nMAAIZ2V0Q2hhcnMAB2dldExvbmcA" +
    "CGhhc2hDb2RlAA1oaWdoZXN0T25lQml0AAFpAAVpbmRleAAIaW50VmFsdWUAAWwAA2xlbgAJbG9u" +
    "Z1ZhbHVlAAZsb29rdXAADGxvd2VzdE9uZUJpdAAEbWFzawAJbWFza0NvdW50AANtYXgAA21pbgAF" +
    "bmFtZXMAAm5tABRudW1iZXJPZkxlYWRpbmdaZXJvcwAVbnVtYmVyT2ZUcmFpbGluZ1plcm9zAANv" +
    "YmoABm9mZnNldAAOcGFyYWxsZWxTdWZmaXgACXBhcnNlTG9uZwARcGFyc2VVbnNpZ25lZExvbmcA" +
    "BXJhZGl4ABFyZW1haW5kZXJVbnNpZ25lZAATcmVzb2x2ZUNvbnN0YW50RGVzYwAHcmV2ZXJzZQAM" +
    "cmV2ZXJzZUJ5dGVzAApyb3RhdGVMZWZ0AAtyb3RhdGVSaWdodAABcwAQc2VyaWFsVmVyc2lvblVJ" +
    "RAAFc2hpZnQACnNob3J0VmFsdWUABnNpZ251bQAKc3RyaW5nU2l6ZQADc3VtAA50b0JpbmFyeVN0" +
    "cmluZwALdG9IZXhTdHJpbmcADXRvT2N0YWxTdHJpbmcACHRvU3RyaW5nABR0b1Vuc2lnbmVkQmln" +
    "SW50ZWdlcgAQdG9VbnNpZ25lZFN0cmluZwARdG9VbnNpZ25lZFN0cmluZzAAA3ZhbAAFdmFsdWUA" +
    "B3ZhbHVlT2YAAXgAAXkAmwF+fkQ4eyJiYWNrZW5kIjoiZGV4IiwiY29tcGlsYXRpb24tbW9kZSI6" +
    "ImRlYnVnIiwiaGFzLWNoZWNrc3VtcyI6ZmFsc2UsIm1pbi1hcGkiOjEsInNoYS0xIjoiZWFlNDNh" +
    "MzAyODgyMjQ4ZmExYzU4NDUzZjJjZGViMjNiMGZkZDU1MCIsInZlcnNpb24iOiI4LjkuMS1kZXYi" +
    "fQACBwGLARwBGA4CBgGLARwFFwAXMhcBFy4XBAIFAkQcASQAEGwcAR4CBwGLARwBGBACBgGLARwD" +
    "FyIXJhcEAgYBiwEcBhcnFyQXJhcEFy0XLgYBMg4AGQEZARkBGQEZARoGEgGIgASEHQGBgATIHQGB" +
    "gAScHQEJ6A8CCfwPAwncEAEJnBgBCYQTAgm0GAMJ1BgCCOQdAQj8EAEInBEBCaQTAQnkEwEJxBMC" +
    "CdwRAQnsGAMJnBkBCbgZAQnYGQEJnBIBCbQSAQr4GQEJkBoBCbAaAQnQGgEJ8BoBCZAbAQmwGwEJ" +
    "0BsDCfAbAQmEHAEJnBwBCbAcAgnQEgEI5BIBCcwcAQmcFQEJvBUBCdwVAgm8FgEJ3BYBCtwXAQn8" +
    "FgEJnBcBCLwXAQnkFAEJpBQBCcQUBQGIDwIBnBABwSC8EAQB/BcCAagPAQHoDgIByA8HAbwRAwH8" +
    "EQEBhBkOAYQUAcEggBUFAeQcBwH8FQQECAYABgAEQAAAAAAAAAEAAABHGAAAAQAAAFAYAAABAAAA" +
    "YRgAAAIAAABhGAAAbhgAAAEAAAB3GAAAAQAAAIQYAADsGQAAAQAAAA0AAAAAAAAABAAAAOQZAAAD" +
    "AAAAwBkAAAgAAADQGQAACwAAAMAZAAAMAAAAyBkAACMAAADAGQAAJAAAAMAZAAAlAAAAwBkAACYA" +
    "AADAGQAAJwAAAMAZAAAoAAAAwBkAACsAAADYGQAAPwAAAMAZAABAAAAAwBkAABEAAAAAAAAAAQAA" +
    "AAAAAAABAAAAkAAAAHAAAAACAAAAHQAAALACAAADAAAAJQAAACQDAAAEAAAABwAAAOAEAAAFAAAA" +
    "RgAAABgFAAAGAAAAAQAAAEgHAAABIAAAQAAAAGgHAAADIAAAPwAAAAQPAAABEAAAEAAAAIQQAAAC" +
    "IAAAkAAAABoRAAAEIAAABgAAAEcYAAAAIAAAAQAAAJcYAAAFIAAAAQAAALEZAAADEAAABwAAALwZ" +
    "AAAGIAAAAQAAAPQZAAAAEAAAAQAAAHQaAAA=");

  static class FuncCmp implements LongPredicate {
    final String name;
    final LongPredicate p;
    public FuncCmp(String name, LongPredicate p) {
      this.name = name;
      this.p = p;
    }

    public boolean test(long l) {
      return p.test(l);
    }
  }
  static FuncCmp l2l(String name, final LongUnaryOperator a, final LongUnaryOperator b) {
    return new FuncCmp(name, (v) -> a.applyAsLong(v) == b.applyAsLong(v));
  }
  static FuncCmp l2i(String name, final LongToIntFunction a, final LongToIntFunction b) {
    return new FuncCmp(name, (v) -> a.applyAsInt(v) == b.applyAsInt(v));
  }

  /** Interface for a long, int -> long function. */
  static interface LI2IFunction { public long applyToLongInt(long a, int b); }

  static FuncCmp li2l(String name, final Random r, final LI2IFunction a, final LI2IFunction b) {
    return new FuncCmp(name, new LongPredicate() {
      public boolean test(long v) {
        int i = r.nextInt();
        return a.applyToLongInt(v, i) == b.applyToLongInt(v, i);
      }
    });
  }

  public static void main(String[] args) {
    doTest(10000);
  }

  public static void doTest(int iters) {
    // Just transform immediately.
    doCommonClassRedefinition(Long.class, CLASS_BYTES, DEX_BYTES);
    final Random r = new Random();
    FuncCmp[] comps = new FuncCmp[] {
      l2l("highestOneBit", Long::highestOneBit, RedefinedLongIntrinsics::highestOneBit),
      l2l("lowestOneBit", Long::lowestOneBit, RedefinedLongIntrinsics::lowestOneBit),
      l2i("numberOfLeadingZeros",
          Long::numberOfLeadingZeros,
          RedefinedLongIntrinsics::numberOfLeadingZeros),
      l2i("numberOfTrailingZeros",
          Long::numberOfTrailingZeros,
          RedefinedLongIntrinsics::numberOfTrailingZeros),
      l2i("bitCount", Long::bitCount, RedefinedLongIntrinsics::bitCount),
      li2l("rotateLeft", r, Long::rotateLeft, RedefinedLongIntrinsics::rotateLeft),
      li2l("rotateRight", r, Long::rotateRight, RedefinedLongIntrinsics::rotateRight),
      l2l("reverse", Long::reverse, RedefinedLongIntrinsics::reverse),
      l2i("signum", Long::signum, RedefinedLongIntrinsics::signum),
      l2l("reverseBytes", Long::reverseBytes, RedefinedLongIntrinsics::reverseBytes),
    };
    for (FuncCmp f : comps) {
      // Just actually use ints so we can cast them back int the tests to print them (since we
      // deleted a bunch of the Long methods needed for printing longs)!
      int failures = (int)r.ints(iters)
                           .mapToLong((v) -> (long)v)
                           .filter(f.negate()) // Get all the test cases that failed.
                           .count();
      if (failures != 0) {
        double percent = 100.0d*((double)failures/(double)iters);
        System.out.println("for intrinsic " + f.name + " " + failures + "/" + iters
            + " (" + Double.toString(percent) + "%) tests failed!");
      }
    }
    System.out.println("Finished!");
  }
}
