/*
 * Copyright (C) 2021 The Android Open Source Project
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
import java.util.concurrent.ThreadLocalRandom;
import jdk.internal.misc.Unsafe;

public class Main {
  private static void check(boolean actual, boolean expected, String msg) {
    if (actual != expected) {
      System.out.println(msg + " : " + actual + " != " + expected);
      System.exit(1);
    }
  }

  private static void check(byte actual, byte expected, String msg) {
    if (actual != expected) {
      System.out.println(msg + " : " + actual + " != " + expected);
      System.exit(1);
    }
  }

  private static void check(char actual, char expected, String msg) {
    if (actual != expected) {
      System.out.println(msg + " : " + actual + " != " + expected);
      System.exit(1);
    }
  }

  private static void check(short actual, short expected, String msg) {
    if (actual != expected) {
      System.out.println(msg + " : " + actual + " != " + expected);
      System.exit(1);
    }
  }

  private static void check(int actual, int expected, String msg) {
    if (actual != expected) {
      System.out.println(msg + " : " + actual + " != " + expected);
      System.exit(1);
    }
  }

  private static void check(long actual, long expected, String msg) {
    if (actual != expected) {
      System.out.println(msg + " : " + actual + " != " + expected);
      System.exit(1);
    }
  }

  private static void check(float actual, float expected, String msg) {
    if (actual != expected) {
      System.out.println(msg + " : " + actual + " != " + expected);
      System.exit(1);
    }
  }

  private static void check(double actual, double expected, String msg) {
    if (actual != expected) {
      System.out.println(msg + " : " + actual + " != " + expected);
      System.exit(1);
    }
  }

  private static void check(Object actual, Object expected, String msg) {
    if (actual != expected) {
      System.out.println(msg + " : " + actual + " != " + expected);
      System.exit(1);
    }
  }

  private static void expectThrow(Class<?> exceptionClass, String msg) {
    System.out.println(msg + " : expected " + exceptionClass.getName());
    System.exit(1);
  }

  private static Unsafe getUnsafe() throws NoSuchFieldException, IllegalAccessException {
    Class<?> unsafeClass = Unsafe.class;
    Field f = unsafeClass.getDeclaredField("theUnsafe");
    f.setAccessible(true);
    return (Unsafe) f.get(null);
  }

  public static void main(String[] args) throws NoSuchFieldException, IllegalAccessException {
    System.loadLibrary(args[0]);
    Unsafe unsafe = getUnsafe();

    testArrayBaseOffset(unsafe);
    testArrayIndexScale(unsafe);
    testGetAndPutAndCAS(unsafe);
    testCompareAndSet(unsafe);
    testGetAndPutVolatile(unsafe);
    testGetAcquireAndPutRelease(unsafe);
    testCopyMemory(unsafe);
    testNullBasedAccessors(unsafe);
    testConstants(unsafe);
  }

  private static void testArrayBaseOffset(Unsafe unsafe) {
    check(unsafe.arrayBaseOffset(boolean[].class), vmJdkArrayBaseOffset(boolean[].class),
        "Unsafe.arrayBaseOffset(boolean[])");
    check(unsafe.arrayBaseOffset(byte[].class), vmJdkArrayBaseOffset(byte[].class),
        "Unsafe.arrayBaseOffset(byte[])");
    check(unsafe.arrayBaseOffset(char[].class), vmJdkArrayBaseOffset(char[].class),
        "Unsafe.arrayBaseOffset(char[])");
    check(unsafe.arrayBaseOffset(double[].class), vmJdkArrayBaseOffset(double[].class),
        "Unsafe.arrayBaseOffset(double[])");
    check(unsafe.arrayBaseOffset(float[].class), vmJdkArrayBaseOffset(float[].class),
        "Unsafe.arrayBaseOffset(float[])");
    check(unsafe.arrayBaseOffset(int[].class), vmJdkArrayBaseOffset(int[].class),
        "Unsafe.arrayBaseOffset(int[])");
    check(unsafe.arrayBaseOffset(long[].class), vmJdkArrayBaseOffset(long[].class),
        "Unsafe.arrayBaseOffset(long[])");
    check(unsafe.arrayBaseOffset(Object[].class), vmJdkArrayBaseOffset(Object[].class),
        "Unsafe.arrayBaseOffset(Object[])");
  }

  private static void testArrayIndexScale(Unsafe unsafe) {
    check(unsafe.arrayIndexScale(boolean[].class), vmJdkArrayIndexScale(boolean[].class),
        "Unsafe.arrayIndexScale(boolean[])");
    check(unsafe.arrayIndexScale(byte[].class), vmJdkArrayIndexScale(byte[].class),
        "Unsafe.arrayIndexScale(byte[])");
    check(unsafe.arrayIndexScale(char[].class), vmJdkArrayIndexScale(char[].class),
        "Unsafe.arrayIndexScale(char[])");
    check(unsafe.arrayIndexScale(double[].class), vmJdkArrayIndexScale(double[].class),
        "Unsafe.arrayIndexScale(double[])");
    check(unsafe.arrayIndexScale(float[].class), vmJdkArrayIndexScale(float[].class),
        "Unsafe.arrayIndexScale(float[])");
    check(unsafe.arrayIndexScale(int[].class), vmJdkArrayIndexScale(int[].class),
        "Unsafe.arrayIndexScale(int[])");
    check(unsafe.arrayIndexScale(long[].class), vmJdkArrayIndexScale(long[].class),
        "Unsafe.arrayIndexScale(long[])");
    check(unsafe.arrayIndexScale(Object[].class), vmJdkArrayIndexScale(Object[].class),
        "Unsafe.arrayIndexScale(Object[])");
  }

  private static void testGetAndPutAndCAS(Unsafe unsafe) throws NoSuchFieldException {
    TestClass t = new TestClass();

    int intValue = 12345678;
    Field intField = TestClass.class.getDeclaredField("intVar");
    long intOffset = unsafe.objectFieldOffset(intField);
    check(unsafe.getInt(t, intOffset), 0, "Unsafe.getInt(Object, long) - initial");
    unsafe.putInt(t, intOffset, intValue);
    check(t.intVar, intValue, "Unsafe.putInt(Object, long, int)");
    check(unsafe.getInt(t, intOffset), intValue, "Unsafe.getInt(Object, long)");

    long longValue = 1234567887654321L;
    Field longField = TestClass.class.getDeclaredField("longVar");
    long longOffset = unsafe.objectFieldOffset(longField);
    check(unsafe.getLong(t, longOffset), 0, "Unsafe.getLong(Object, long) - initial");
    unsafe.putLong(t, longOffset, longValue);
    check(t.longVar, longValue, "Unsafe.putLong(Object, long, long)");
    check(unsafe.getLong(t, longOffset), longValue, "Unsafe.getLong(Object, long)");

    Object objectValue = new Object();
    Field objectField = TestClass.class.getDeclaredField("objectVar");
    long objectOffset = unsafe.objectFieldOffset(objectField);
    check(unsafe.getObject(t, objectOffset), null, "Unsafe.getObject(Object, long) - initial");
    unsafe.putObject(t, objectOffset, objectValue);
    check(t.objectVar, objectValue, "Unsafe.putObject(Object, long, Object)");
    check(unsafe.getObject(t, objectOffset), objectValue, "Unsafe.getObject(Object, long)");

    if (unsafe.compareAndSwapInt(t, intOffset, 0, 1)) {
      System.out.println("Unexpectedly succeeding compareAndSwapInt(t, intOffset, 0, 1)");
    }
    if (!unsafe.compareAndSwapInt(t, intOffset, intValue, 0)) {
      System.out.println(
          "Unexpectedly not succeeding compareAndSwapInt(t, intOffset, intValue, 0)");
    }
    if (!unsafe.compareAndSwapInt(t, intOffset, 0, 1)) {
      System.out.println("Unexpectedly not succeeding compareAndSwapInt(t, intOffset, 0, 1)");
    }
    // Exercise jdk.internal.misc.Unsafe.compareAndSwapInt using the same
    // integer (1) for the `expectedValue` and `newValue` arguments.
    if (!unsafe.compareAndSwapInt(t, intOffset, 1, 1)) {
      System.out.println("Unexpectedly not succeeding compareAndSwapInt(t, intOffset, 1, 1)");
    }

    if (unsafe.compareAndSwapLong(t, longOffset, 0, 1)) {
      System.out.println("Unexpectedly succeeding compareAndSwapLong(t, longOffset, 0, 1)");
    }
    if (!unsafe.compareAndSwapLong(t, longOffset, longValue, 0)) {
      System.out.println(
          "Unexpectedly not succeeding compareAndSwapLong(t, longOffset, longValue, 0)");
    }
    if (!unsafe.compareAndSwapLong(t, longOffset, 0, 1)) {
      System.out.println("Unexpectedly not succeeding compareAndSwapLong(t, longOffset, 0, 1)");
    }
    // Exercise jdk.internal.misc.Unsafe.compareAndSwapLong using the same
    // integer (1) for the `expectedValue` and `newValue` arguments.
    if (!unsafe.compareAndSwapLong(t, longOffset, 1, 1)) {
      System.out.println("Unexpectedly not succeeding compareAndSwapLong(t, longOffset, 1, 1)");
    }

    // We do not use `null` as argument to jdk.internal.misc.Unsafe.compareAndSwapObject
    // in those tests, as this value is not affected by heap poisoning
    // (which uses address negation to poison and unpoison heap object
    // references).  This way, when heap poisoning is enabled, we can
    // better exercise its implementation within that method.
    if (unsafe.compareAndSwapObject(t, objectOffset, new Object(), new Object())) {
      System.out.println("Unexpectedly succeeding " +
          "compareAndSwapObject(t, objectOffset, new Object(), new Object())");
    }
    Object objectValue2 = new Object();
    if (!unsafe.compareAndSwapObject(t, objectOffset, objectValue, objectValue2)) {
      System.out.println("Unexpectedly not succeeding " +
          "compareAndSwapObject(t, objectOffset, objectValue, objectValue2)");
    }
    Object objectValue3 = new Object();
    if (!unsafe.compareAndSwapObject(t, objectOffset, objectValue2, objectValue3)) {
      System.out.println("Unexpectedly not succeeding " +
          "compareAndSwapObject(t, objectOffset, objectValue2, objectValue3)");
    }
    // Exercise jdk.internal.misc.Unsafe.compareAndSwapObject using the same
    // object (`objectValue3`) for the `expectedValue` and `newValue` arguments.
    if (!unsafe.compareAndSwapObject(t, objectOffset, objectValue3, objectValue3)) {
      System.out.println("Unexpectedly not succeeding " +
          "compareAndSwapObject(t, objectOffset, objectValue3, objectValue3)");
    }
    // Exercise jdk.internal.misc.Unsafe.compareAndSwapObject using the same
    // object (`t`) for the `obj` and `newValue` arguments.
    if (!unsafe.compareAndSwapObject(t, objectOffset, objectValue3, t)) {
      System.out.println(
          "Unexpectedly not succeeding compareAndSwapObject(t, objectOffset, objectValue3, t)");
    }
    // Exercise jdk.internal.misc.Unsafe.compareAndSwapObject using the same
    // object (`t`) for the `obj`, `expectedValue` and `newValue` arguments.
    if (!unsafe.compareAndSwapObject(t, objectOffset, t, t)) {
      System.out.println("Unexpectedly not succeeding compareAndSwapObject(t, objectOffset, t, t)");
    }
    // Exercise jdk.internal.misc.Unsafe.compareAndSwapObject using the same
    // object (`t`) for the `obj` and `expectedValue` arguments.
    if (!unsafe.compareAndSwapObject(t, objectOffset, t, new Object())) {
      System.out.println(
          "Unexpectedly not succeeding compareAndSwapObject(t, objectOffset, t, new Object())");
    }
  }

  private static void testCompareAndSet(Unsafe unsafe) throws NoSuchFieldException {
    TestClass t = new TestClass();

    int intValue = 12345678;
    Field intField = TestClass.class.getDeclaredField("intVar");
    long intOffset = unsafe.objectFieldOffset(intField);
    unsafe.putInt(t, intOffset, intValue);

    long longValue = 1234567887654321L;
    Field longField = TestClass.class.getDeclaredField("longVar");
    long longOffset = unsafe.objectFieldOffset(longField);
    unsafe.putLong(t, longOffset, longValue);

    Object objectValue = new Object();
    Field objectField = TestClass.class.getDeclaredField("objectVar");
    long objectOffset = unsafe.objectFieldOffset(objectField);
    unsafe.putObject(t, objectOffset, objectValue);

    if (unsafe.compareAndSetInt(t, intOffset, 0, 1)) {
      System.out.println("Unexpectedly succeeding compareAndSetInt(t, intOffset, 0, 1)");
    }
    check(t.intVar, intValue, "Unsafe.compareAndSetInt(Object, long, int, int) - not set");
    if (!unsafe.compareAndSetInt(t, intOffset, intValue, 0)) {
      System.out.println(
          "Unexpectedly not succeeding compareAndSetInt(t, intOffset, intValue, 0)");
    }
    check(t.intVar, 0, "Unsafe.compareAndSetInt(Object, long, int, int) - gets set");
    if (!unsafe.compareAndSetInt(t, intOffset, 0, 1)) {
      System.out.println("Unexpectedly not succeeding compareAndSetInt(t, intOffset, 0, 1)");
    }
    check(t.intVar, 1, "Unsafe.compareAndSetInt(Object, long, int, int) - gets re-set");
    // Exercise jdk.internal.misc.Unsafe.compareAndSetInt using the same
    // integer (1) for the `expectedValue` and `newValue` arguments.
    if (!unsafe.compareAndSetInt(t, intOffset, 1, 1)) {
      System.out.println("Unexpectedly not succeeding compareAndSetInt(t, intOffset, 1, 1)");
    }
    check(t.intVar, 1, "Unsafe.compareAndSetInt(Object, long, int, int) - gets set to same");

    if (unsafe.compareAndSetLong(t, longOffset, 0, 1)) {
      System.out.println("Unexpectedly succeeding compareAndSetLong(t, longOffset, 0, 1)");
    }
    check(t.longVar, longValue, "Unsafe.compareAndSetLong(Object, long, long, long) - not set");
    if (!unsafe.compareAndSetLong(t, longOffset, longValue, 0)) {
      System.out.println(
          "Unexpectedly not succeeding compareAndSetLong(t, longOffset, longValue, 0)");
    }
    check(t.longVar, 0, "Unsafe.compareAndSetLong(Object, long, long, long) - gets set");
    if (!unsafe.compareAndSetLong(t, longOffset, 0, 1)) {
      System.out.println("Unexpectedly not succeeding compareAndSetLong(t, longOffset, 0, 1)");
    }
    check(t.longVar, 1, "Unsafe.compareAndSetLong(Object, long, long, long) - gets re-set");
    // Exercise jdk.internal.misc.Unsafe.compareAndSetLong using the same
    // integer (1) for the `expectedValue` and `newValue` arguments.
    if (!unsafe.compareAndSetLong(t, longOffset, 1, 1)) {
      System.out.println("Unexpectedly not succeeding compareAndSetLong(t, longOffset, 1, 1)");
    }
    check(t.longVar, 1, "Unsafe.compareAndSetLong(Object, long, long, long) - gets set to same");

    // We do not use `null` as argument to jdk.internal.misc.Unsafe.compareAndSwapObject
    // in those tests, as this value is not affected by heap poisoning
    // (which uses address negation to poison and unpoison heap object
    // references).  This way, when heap poisoning is enabled, we can
    // better exercise its implementation within that method.
    if (unsafe.compareAndSetObject(t, objectOffset, new Object(), new Object())) {
      System.out.println("Unexpectedly succeeding compareAndSetObject(t, objectOffset, 0, 1)");
    }
    check(t.objectVar, objectValue, "Unsafe.compareAndSetObject(Object, long, Object, Object) - not set");
    Object objectValue2 = new Object();
    if (!unsafe.compareAndSetObject(t, objectOffset, objectValue, objectValue2)) {
      System.out.println(
          "Unexpectedly not succeeding compareAndSetObject(t, objectOffset, objectValue, 0)");
    }
    check(t.objectVar, objectValue2, "Unsafe.compareAndSetObject(Object, long, Object, Object) - gets set");
    Object objectValue3 = new Object();
    if (!unsafe.compareAndSetObject(t, objectOffset, objectValue2, objectValue3)) {
      System.out.println("Unexpectedly not succeeding compareAndSetObject(t, objectOffset, 0, 1)");
    }
    check(t.objectVar, objectValue3, "Unsafe.compareAndSetObject(Object, long, Object, Object) - gets re-set");
    // Exercise jdk.internal.misc.Unsafe.compareAndSetObject using the same
    // object for the `expectedValue` and `newValue` arguments.
    if (!unsafe.compareAndSetObject(t, objectOffset, objectValue3, objectValue3)) {
      System.out.println("Unexpectedly not succeeding compareAndSetObject(t, objectOffset, 1, 1)");
    }
    check(t.objectVar, objectValue3, "Unsafe.compareAndSetObject(Object, long, Object, Object) - gets set to same");

    // Reset and now try with `compareAndSetReference` which replaced `compareAndSetObject`.
    unsafe.putObject(t, objectOffset, objectValue);

    if (unsafe.compareAndSetReference(t, objectOffset, new Object(), new Object())) {
      System.out.println("Unexpectedly succeeding compareAndSetReference(t, objectOffset, 0, 1)");
    }
    check(t.objectVar, objectValue, "Unsafe.compareAndSetReference(Object, long, Object, Object) - not set");
    objectValue2 = new Object();
    if (!unsafe.compareAndSetReference(t, objectOffset, objectValue, objectValue2)) {
      System.out.println(
          "Unexpectedly not succeeding compareAndSetReference(t, objectOffset, objectValue, 0)");
    }
    check(t.objectVar, objectValue2, "Unsafe.compareAndSetReference(Object, long, Object, Object) - gets set");
    objectValue3 = new Object();
    if (!unsafe.compareAndSetReference(t, objectOffset, objectValue2, objectValue3)) {
      System.out.println("Unexpectedly not succeeding compareAndSetReference(t, objectOffset, 0, 1)");
    }
    check(t.objectVar, objectValue3, "Unsafe.compareAndSetReference(Object, long, Object, Object) - gets re-set");
    // Exercise jdk.internal.misc.Unsafe.compareAndSetReference using the same
    // object for the `expectedValue` and `newValue` arguments.
    if (!unsafe.compareAndSetReference(t, objectOffset, objectValue3, objectValue3)) {
      System.out.println("Unexpectedly not succeeding compareAndSetReference(t, objectOffset, 1, 1)");
    }
    check(t.objectVar, objectValue3, "Unsafe.compareAndSetReference(Object, long, Object, Object) - gets set to same");
 }

  private static void testGetAndPutVolatile(Unsafe unsafe) throws NoSuchFieldException {
    TestVolatileClass tv = new TestVolatileClass();

    int intValue = 12345678;
    Field volatileIntField = TestVolatileClass.class.getDeclaredField("volatileIntVar");
    long volatileIntOffset = unsafe.objectFieldOffset(volatileIntField);
    check(unsafe.getIntVolatile(tv, volatileIntOffset),
          0,
          "Unsafe.getIntVolatile(Object, long) - initial");
    unsafe.putIntVolatile(tv, volatileIntOffset, intValue);
    check(tv.volatileIntVar, intValue, "Unsafe.putIntVolatile(Object, long, int)");
    check(unsafe.getIntVolatile(tv, volatileIntOffset),
          intValue,
          "Unsafe.getIntVolatile(Object, long)");

    boolean booleanValue = true;
    Field volatileBooleanField = TestVolatileClass.class.getDeclaredField("volatileBooleanVar");
    long volatileBooleanOffset = unsafe.objectFieldOffset(volatileBooleanField);
    check(unsafe.getBooleanVolatile(tv, volatileBooleanOffset),
          false,
          "Unsafe.getBooleanVolatile(Object, long) - initial");
    unsafe.putBooleanVolatile(tv, volatileBooleanOffset, booleanValue);
    check(tv.volatileBooleanVar, booleanValue, "Unsafe.putBooleanVolatile(Object, long, boolean)");
    check(unsafe.getBooleanVolatile(tv, volatileBooleanOffset),
          booleanValue,
          "Unsafe.getBooleanVolatile(Object, long)");

    byte byteValue = 125;
    Field volatileByteField = TestVolatileClass.class.getDeclaredField("volatileByteVar");
    long volatileByteOffset = unsafe.objectFieldOffset(volatileByteField);
    check(unsafe.getByteVolatile(tv, volatileByteOffset),
          0,
          "Unsafe.getByteVolatile(Object, long) - initial");
    unsafe.putByteVolatile(tv, volatileByteOffset, byteValue);
    check(tv.volatileByteVar, byteValue, "Unsafe.putByteVolatile(Object, long, byte)");
    check(unsafe.getByteVolatile(tv, volatileByteOffset),
          byteValue,
          "Unsafe.getByteVolatile(Object, long)");

    char charValue = 'X';
    Field volatileCharField = TestVolatileClass.class.getDeclaredField("volatileCharVar");
    long volatileCharOffset = unsafe.objectFieldOffset(volatileCharField);
    check(unsafe.getCharVolatile(tv, volatileCharOffset),
          '\0',
          "Unsafe.getCharVolatile(Object, long) - initial");
    unsafe.putCharVolatile(tv, volatileCharOffset, charValue);
    check(tv.volatileCharVar, charValue, "Unsafe.putCharVolatile(Object, long, char)");
    check(unsafe.getCharVolatile(tv, volatileCharOffset),
          charValue,
          "Unsafe.getCharVolatile(Object, long)");

    short shortValue = 32523;
    Field volatileShortField = TestVolatileClass.class.getDeclaredField("volatileShortVar");
    long volatileShortOffset = unsafe.objectFieldOffset(volatileShortField);
    check(unsafe.getShortVolatile(tv, volatileShortOffset),
          0,
          "Unsafe.getShortVolatile(Object, long) - initial");
    unsafe.putShortVolatile(tv, volatileShortOffset, shortValue);
    check(tv.volatileShortVar, shortValue, "Unsafe.putShortVolatile(Object, long, short)");
    check(unsafe.getShortVolatile(tv, volatileShortOffset),
          shortValue,
          "Unsafe.getShortVolatile(Object, long)");

    long longValue = 1234567887654321L;
    Field volatileLongField = TestVolatileClass.class.getDeclaredField("volatileLongVar");
    long volatileLongOffset = unsafe.objectFieldOffset(volatileLongField);
    check(unsafe.getLongVolatile(tv, volatileLongOffset),
          0,
          "Unsafe.getLongVolatile(Object, long) - initial");
    unsafe.putLongVolatile(tv, volatileLongOffset, longValue);
    check(tv.volatileLongVar, longValue, "Unsafe.putLongVolatile(Object, long, long)");
    check(unsafe.getLongVolatile(tv, volatileLongOffset),
          longValue,
          "Unsafe.getLongVolatile(Object, long)");

    float floatValue = 123456.7890123f;
    Field volatileFloatField = TestVolatileClass.class.getDeclaredField("volatileFloatVar");
    long volatileFloatOffset = unsafe.objectFieldOffset(volatileFloatField);
    check(unsafe.getFloatVolatile(tv, volatileFloatOffset),
          0.0f,
          "Unsafe.getFloatVolatile(Object, long) - initial");
    unsafe.putFloatVolatile(tv, volatileFloatOffset, floatValue);
    check(tv.volatileFloatVar, floatValue, "Unsafe.putFloatVolatile(Object, long, float)");
    check(unsafe.getFloatVolatile(tv, volatileFloatOffset),
          floatValue,
          "Unsafe.getFloatVolatile(Object, long)");

    double doubleValue = 654321.7890123d;
    Field volatileDoubleField = TestVolatileClass.class.getDeclaredField("volatileDoubleVar");
    long volatileDoubleOffset = unsafe.objectFieldOffset(volatileDoubleField);
    check(unsafe.getDoubleVolatile(tv, volatileDoubleOffset),
          0.0d,
          "Unsafe.getDoubleVolatile(Object, double) - initial");
    unsafe.putDoubleVolatile(tv, volatileDoubleOffset, doubleValue);
    check(tv.volatileDoubleVar, doubleValue, "Unsafe.putDoubleVolatile(Object, long, double)");
    check(unsafe.getDoubleVolatile(tv, volatileDoubleOffset),
          doubleValue,
          "Unsafe.getDoubleVolatile(Object, long)");

    Object objectValue = new Object();
    Field volatileObjectField = TestVolatileClass.class.getDeclaredField("volatileObjectVar");
    long volatileObjectOffset = unsafe.objectFieldOffset(volatileObjectField);
    check(unsafe.getReferenceVolatile(tv, volatileObjectOffset),
          null,
          "Unsafe.getReferenceVolatile(Object, long) - initial");
    unsafe.putReferenceVolatile(tv, volatileObjectOffset, objectValue);
    check(tv.volatileObjectVar, objectValue, "Unsafe.putReferenceVolatile(Object, long, Object)");
    check(unsafe.getReferenceVolatile(tv, volatileObjectOffset),
          objectValue,
          "Unsafe.getReferenceVolatile(Object, long)");
  }

  private static void testGetAcquireAndPutRelease(Unsafe unsafe) throws NoSuchFieldException {
    TestVolatileClass tv = new TestVolatileClass();

    int intValue = 12345678;
    Field volatileIntField = TestVolatileClass.class.getDeclaredField("volatileIntVar");
    long volatileIntOffset = unsafe.objectFieldOffset(volatileIntField);
    check(unsafe.getIntAcquire(tv, volatileIntOffset),
          0,
          "Unsafe.getIntAcquire(Object, long) - initial");
    unsafe.putIntRelease(tv, volatileIntOffset, intValue);
    check(tv.volatileIntVar, intValue, "Unsafe.putIntRelease(Object, long, int)");
    check(unsafe.getIntAcquire(tv, volatileIntOffset),
          intValue,
          "Unsafe.getIntAcquire(Object, long)");

    long longValue = 1234567887654321L;
    Field volatileLongField = TestVolatileClass.class.getDeclaredField("volatileLongVar");
    long volatileLongOffset = unsafe.objectFieldOffset(volatileLongField);
    check(unsafe.getLongAcquire(tv, volatileLongOffset),
          0,
          "Unsafe.getLongAcquire(Object, long) - initial");
    unsafe.putLongRelease(tv, volatileLongOffset, longValue);
    check(tv.volatileLongVar, longValue, "Unsafe.putLongRelease(Object, long, long)");
    check(unsafe.getLongAcquire(tv, volatileLongOffset),
          longValue,
          "Unsafe.getLongAcquire(Object, long)");

    Object objectValue = new Object();
    Field volatileObjectField = TestVolatileClass.class.getDeclaredField("volatileObjectVar");
    long volatileObjectOffset = unsafe.objectFieldOffset(volatileObjectField);
    check(unsafe.getObjectAcquire(tv, volatileObjectOffset),
          null,
          "Unsafe.getObjectAcquire(Object, long) - initial");
    unsafe.putObjectRelease(tv, volatileObjectOffset, objectValue);
    check(tv.volatileObjectVar, objectValue, "Unsafe.putObjectRelease(Object, long, Object)");
    check(unsafe.getObjectAcquire(tv, volatileObjectOffset),
          objectValue,
          "Unsafe.getObjectAcquire(Object, long)");
  }

  private static void testCopyMemory(Unsafe unsafe) {
    final int size = 4 * 1024;

    final int intSize = 4;
    int[] inputInts = new int[size / intSize];
    for (int i = 0; i != inputInts.length; ++i) {
      inputInts[i] = ((int)i) + 1;
    }
    int[] outputInts = new int[size / intSize];
    unsafe.copyMemory(inputInts, Unsafe.ARRAY_INT_BASE_OFFSET,
                      outputInts, Unsafe.ARRAY_INT_BASE_OFFSET,
                      size);
    for (int i = 0; i != inputInts.length; ++i) {
      check(inputInts[i], outputInts[i], "unsafe.copyMemory/int");
    }

    final int longSize = 8;
    long[] inputLongs = new long[size / longSize];
    for (int i = 0; i != inputLongs.length; ++i) {
      inputLongs[i] = ((long)i) + 1L;
    }
    long[] outputLongs = new long[size / longSize];
    unsafe.copyMemory(inputLongs, 0, outputLongs, 0, size);
    unsafe.copyMemory(inputLongs, Unsafe.ARRAY_LONG_BASE_OFFSET,
                      outputLongs, Unsafe.ARRAY_LONG_BASE_OFFSET,
                      size);
    for (int i = 0; i != inputLongs.length; ++i) {
      check(inputLongs[i], outputLongs[i], "unsafe.copyMemory/long");
    }

    final int floatSize = 4;
    float[] inputFloats = new float[size / floatSize];
    for (int i = 0; i != inputFloats.length; ++i) {
      inputFloats[i] = ((float)i) + 0.5f;
    }
    float[] outputFloats = new float[size / floatSize];
    unsafe.copyMemory(inputFloats, 0, outputFloats, 0, size);
    unsafe.copyMemory(inputFloats, Unsafe.ARRAY_FLOAT_BASE_OFFSET,
                      outputFloats, Unsafe.ARRAY_FLOAT_BASE_OFFSET,
                      size);
    for (int i = 0; i != inputFloats.length; ++i) {
      check(inputFloats[i], outputFloats[i], "unsafe.copyMemory/float");
    }

    final int doubleSize = 8;
    double[] inputDoubles = new double[size / doubleSize];
    for (int i = 0; i != inputDoubles.length; ++i) {
      inputDoubles[i] = ((double)i) + 0.5;
    }
    double[] outputDoubles = new double[size / doubleSize];
    unsafe.copyMemory(inputDoubles, Unsafe.ARRAY_DOUBLE_BASE_OFFSET,
                      outputDoubles, Unsafe.ARRAY_DOUBLE_BASE_OFFSET,
                      size);
    for (int i = 0; i != inputDoubles.length; ++i) {
      check(inputDoubles[i], outputDoubles[i], "unsafe.copyMemory/double");
    }

    // check the version that works with memory pointers
    try (TestMemoryPtr srcPtr = new TestMemoryPtr(size);
         TestMemoryPtr dstPtr = new TestMemoryPtr(size)) {
        // use the integer array to fill the source
        unsafe.copyMemory(inputInts, Unsafe.ARRAY_INT_BASE_OFFSET,
                          null, srcPtr.get(),
                          size);

        unsafe.copyMemory(srcPtr.get(), dstPtr.get(), size);
        for (int i = 0; i != size; ++i) {
          check(unsafe.getByte(srcPtr.get() + i),
                unsafe.getByte(dstPtr.get() + i),
                "unsafe.copyMemory/memoryAddress");
        }
    }

    try {
        TestClass srcObj = new TestClass();
        srcObj.intVar = 12345678;
        int[] dstArray = new int[1];
        unsafe.copyMemory(srcObj, unsafe.objectFieldOffset(TestClass.class, "intVar"),
                          dstArray, Unsafe.ARRAY_INT_BASE_OFFSET,
                          4);
        expectThrow(RuntimeException.class, "unsafe.copyMemory/non-array-src");
    } catch (RuntimeException expected) {
    }

    try {
        int[] srcArray = { 12345678 };
        TestClass dstObj = new TestClass();
        unsafe.copyMemory(srcArray, Unsafe.ARRAY_INT_BASE_OFFSET,
                          dstObj, unsafe.objectFieldOffset(TestClass.class, "intVar"),
                          4);
        expectThrow(RuntimeException.class, "unsafe.copyMemory/non-array-dst");
    } catch (RuntimeException expected) {
    }
  }

  private static void testNullBasedAccessors(Unsafe unsafe) {
    long ptr = unsafe.allocateMemory(16);
    int intV = ThreadLocalRandom.current().nextInt();

    unsafe.putInt(/*o=*/ null, ptr, intV);
    check(unsafe.getInt(/*o=*/ null, ptr), intV, "putInt/getInt");

    unsafe.putIntVolatile(/*o=*/ null, ptr, intV + 1);
    check(unsafe.getIntVolatile(/*o=*/ null, ptr), intV + 1, "getInt/putInt volatile");

    check(unsafe.compareAndSetInt(/*o=*/ null, ptr, intV + 1, intV), true, "CAS int");
    check(unsafe.compareAndSetInt(/*o=*/ null, ptr, intV + 1, intV), false, "CAS int repeat");

    long longV = ThreadLocalRandom.current().nextLong();

    unsafe.putLong(/*o=*/ null, ptr, longV);
    check(unsafe.getLong(/*o=*/ null, ptr), longV, "getLong/putLong");

    unsafe.putLongVolatile(/*o=*/ null, ptr, longV + 1);
    check(unsafe.getLongVolatile(/*o=*/ null, ptr), longV + 1, "getLong/putLong volatile");
    check(unsafe.compareAndSetLong(/*o=*/ null, ptr, longV + 1, longV), true, "CAS long");
    check(unsafe.compareAndSetLong(/*o=*/ null, ptr, longV + 1, longV), false, "CAS long repeat");

    check(unsafe.compareAndExchangeLong(/*o=*/ null, ptr, longV, longV + 1), longV, "CAE long");
    check(
        unsafe.compareAndExchangeLong(/*o=*/ null, ptr, longV, longV + 1),
        longV + 1,
        "CAE long repeat");

    boolean booleanV = ThreadLocalRandom.current().nextBoolean();

    unsafe.putBoolean(/*o=*/ null, ptr, booleanV);
    check(unsafe.getBoolean(/*o=*/ null, ptr), booleanV, "putBoolean/getBoolean");
    unsafe.putBooleanVolatile(/*o=*/ null, ptr, !booleanV);
    check(unsafe.getBooleanVolatile(/*o=*/ null, ptr), !booleanV, "putBoolean/getBoolean volatile");

    byte byteV = (byte) ThreadLocalRandom.current().nextInt();

    unsafe.putByte(/*o=*/ null, ptr, byteV);
    check(unsafe.getByte(/*o=*/ null, ptr), byteV, "putByte/getByte");

    byteV = (byte) ThreadLocalRandom.current().nextInt();
    unsafe.putByteVolatile(/*o=*/ null, ptr, byteV);
    check(unsafe.getByteVolatile(/*o=*/ null, ptr), byteV, "putByte/getByte volatile");

    char charV = (char) ThreadLocalRandom.current().nextInt();

    unsafe.putChar(/*o=*/ null, ptr, charV);
    check(unsafe.getChar(/*o=*/ null, ptr), charV, "putChar/getChar");

    charV = (char) ThreadLocalRandom.current().nextInt();
    unsafe.putCharVolatile(/*o=*/ null, ptr, charV);
    check(unsafe.getCharVolatile(/*o=*/ null, ptr), charV, "putChar/getChar volatile");

    short shortV = (short) ThreadLocalRandom.current().nextInt();

    unsafe.putShort(/*o=*/ null, ptr, shortV);
    check(unsafe.getShort(/*o=*/ null, ptr), shortV, "putShort/getShort");

    shortV = (short) ThreadLocalRandom.current().nextInt();
    unsafe.putShortVolatile(/*o=*/ null, ptr, shortV);
    check(unsafe.getShortVolatile(/*o=*/ null, ptr), shortV, "putShort/getShort volatile");

    float floatV = ThreadLocalRandom.current().nextFloat();

    unsafe.putFloat(/*o=*/ null, ptr, floatV);
    check(
        Float.floatToRawIntBits(unsafe.getFloat(/*o=*/ null, ptr)),
        Float.floatToRawIntBits(floatV),
        "putFloat/getFloat");

    floatV = ThreadLocalRandom.current().nextFloat();

    unsafe.putFloatVolatile(/*o=*/ null, ptr, floatV);
    check(
        Float.floatToRawIntBits(unsafe.getFloatVolatile(/*o=*/ null, ptr)),
        Float.floatToRawIntBits(floatV),
        "putFloat/getFloat volatile");

    double doubleV = ThreadLocalRandom.current().nextDouble();

    unsafe.putDouble(/*o=*/ null, ptr, doubleV);
    check(
        Double.doubleToRawLongBits(unsafe.getDouble(/*o=*/ null, ptr)),
        Double.doubleToRawLongBits(doubleV),
        "putDouble/getDouble");

    doubleV = ThreadLocalRandom.current().nextDouble();

    unsafe.putDoubleVolatile(/*o=*/ null, ptr, doubleV);
    check(
        Double.doubleToRawLongBits(unsafe.getDoubleVolatile(/*o=*/ null, ptr)),
        Double.doubleToRawLongBits(doubleV),
        "putDouble/getDouble volatile");
  }

  private static void testConstants(Unsafe unsafe) {
    check(Unsafe.ADDRESS_SIZE, unsafe.addressSize(), "ADDRESS_SIZE vs addressSize()");
    check(
        Unsafe.ARRAY_BOOLEAN_BASE_OFFSET,
        unsafe.arrayBaseOffset(boolean[].class),
        "boolean array offset");
    check(
        Unsafe.ARRAY_BYTE_BASE_OFFSET,
        unsafe.arrayBaseOffset(byte[].class),
        "byte array offset");
    check(
        Unsafe.ARRAY_SHORT_BASE_OFFSET,
        unsafe.arrayBaseOffset(short[].class),
        "short array offset");
    check(
        Unsafe.ARRAY_CHAR_BASE_OFFSET,
        unsafe.arrayBaseOffset(char[].class),
        "char array offset");
    check(
        Unsafe.ARRAY_INT_BASE_OFFSET,
        unsafe.arrayBaseOffset(int[].class),
        "int array offset");
    check(
        Unsafe.ARRAY_LONG_BASE_OFFSET,
        unsafe.arrayBaseOffset(long[].class),
        "long array offset");
    check(
        Unsafe.ARRAY_FLOAT_BASE_OFFSET,
        unsafe.arrayBaseOffset(float[].class),
        "float array offset");
    check(
        Unsafe.ARRAY_DOUBLE_BASE_OFFSET,
        unsafe.arrayBaseOffset(double[].class),
        "double array offset");
    check(
        Unsafe.ARRAY_OBJECT_BASE_OFFSET,
        unsafe.arrayBaseOffset(Object[].class),
        "Object array offset");
   }

  private static class TestClass {
    public int intVar = 0;
    public long longVar = 0;
    public Object objectVar = null;
  }

  private static class TestVolatileClass {
    public volatile int volatileIntVar = 0;
    public volatile boolean volatileBooleanVar = false;
    public volatile byte volatileByteVar = 0;
    public volatile short volatileShortVar = 0;
    public volatile char volatileCharVar = 0;
    public volatile long volatileLongVar = 0;
    public volatile float volatileFloatVar = 0.0f;
    public volatile double volatileDoubleVar = 0.0d;
    public volatile Object volatileObjectVar = null;
  }

  private static class TestMemoryPtr implements AutoCloseable {
      private long ptr = 0;

      public TestMemoryPtr(int size) {
          ptr = jdkUnsafeTestMalloc(size);
      }

      public long get() {
          return ptr;
      }

      @Override
      public void close() {
          if (ptr != 0) {
              jdkUnsafeTestFree(ptr);
              ptr = 0;
          }
      }
  }

  private static native int vmJdkArrayBaseOffset(Class<?> clazz);
  private static native int vmJdkArrayIndexScale(Class<?> clazz);
  private static native long jdkUnsafeTestMalloc(long size);
  private static native void jdkUnsafeTestFree(long memory);
}
