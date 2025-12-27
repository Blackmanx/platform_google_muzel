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

import java.lang.invoke.MethodHandle;
import java.lang.invoke.WrongMethodTypeException;
import java.util.Objects;

public abstract class AbstractInvokeExactTest {

  public static String STATUS = "";

  public final void runAll() throws Throwable {
    STATUS = "";
    Multi.$noinline$testMHFromMain(optionalGet());
    $noinline$privateMethods();
    $noinline$testNoArgsCalls();
    $noinline$nullchecks();
    $noinline$testWithArgs();
    $noinline$interfaceChecks();
    $noinline$abstractClass();
  }

  // There is no privateLookupIn for const-method-handle.
  abstract void $noinline$privateMethods() throws Throwable;

  private void $noinline$testNoArgsCalls() throws Throwable {
    voidMethod().invokeExact(new A());
    assertEquals("A.voidMethod", STATUS);

    int returnedInt = (int) returnInt().invokeExact(new A());
    assertEquals(42, returnedInt);

    double returnedDouble = (double) returnDouble().invokeExact(new A());
    assertEquals(42.0d, returnedDouble);
    try {
      interfaceDefaultMethod().invokeExact(new A());
      unreachable("MethodHandle's type is (Main$I)V, but callsite is (Main$A)V");
    } catch (WrongMethodTypeException expected) {}

    interfaceDefaultMethod().invokeExact((I) new A());
    assertEquals("I.defaultMethod", STATUS);

    overwrittenInterfaceDefaultMethod().invokeExact((I) new A());
    assertEquals("A.overrideMe", STATUS);


    try {
      exceptionThrowingMethod().invokeExact(new A());
      unreachable("Target method always throws");
    } catch (MyRuntimeException expected) {
      assertEquals("A.throwException", STATUS);
    }

    try {
      returnInt().invokeExact(new A());
      unreachable("MethodHandle's type is (Main$A)I, but callsite type is (Main$A)V");
    } catch (WrongMethodTypeException expected) {}

    String returnedString = (String) staticMethod().invokeExact(new A());
    assertEquals("staticMethod", returnedString);
  }

  private void $noinline$nullchecks() throws Throwable {
    try {
      voidMethod().invokeExact((A) null);
      unreachable("Receiver is null, should throw NPE");
    } catch (NullPointerException expected) {}

    try {
      voidMethod().invokeExact((Main) null);
      unreachable("Should throw WMTE: input is of wrong type");
    } catch (WrongMethodTypeException expected) {}

    try {
      interfaceDefaultMethod().invokeExact((I) null);
      unreachable("Receiver is null, should throw NPE");
    } catch (NullPointerException expected) {}

    try {
      interfaceDefaultMethod().invokeExact((A) null);
      unreachable("Should throw WMTE: input is of wrong type");
    } catch (WrongMethodTypeException expected) {}

    try {
      MethodHandle mh = $noinline$nullMethodHandle();
      mh.invokeExact();
      unreachable("MethodHandle object is null, should throw NPE");
    } catch (NullPointerException expected) {}
  }

  private static MethodHandle $noinline$nullMethodHandle() {
    return null;
  }

  private void $noinline$testWithArgs() throws Throwable {
    int sum = (int) sumI().invokeExact(new Sums(), 1);
    assertEquals(1, sum);

    sum = (int) sum2I().invokeExact(new Sums(), 1, 2);
    assertEquals(3, sum);

    sum = (int) sum3I().invokeExact(new Sums(), 1, 2, 3);
    assertEquals(6, sum);

    sum = (int) sum4I().invokeExact(new Sums(), 1, 2, 3, 4);
    assertEquals(10, sum);

    sum = (int) sum5I().invokeExact(new Sums(), 1, 2, 3, 4, 5);
    assertEquals(15, sum);

    sum = (int) sum6I().invokeExact(new Sums(), 1, 2, 3, 4, 5, 6);
    assertEquals(21, sum);

    sum = (int) sum7I().invokeExact(new Sums(), 1, 2, 3, 4, 5, 6, 7);
    assertEquals(28, sum);

    sum = (int) sum8I().invokeExact(new Sums(), 1, 2, 3, 4, 5, 6, 7, 8);
    assertEquals(36, sum);

    sum = (int) sum9I().invokeExact(new Sums(), 1, 2, 3, 4, 5, 6, 7, 8, 9);
    assertEquals(45, sum);

    sum = (int) sum10I().invokeExact(new Sums(), 1, 2, 3, 4, 5, 6, 7, 8, 9, 10);
    assertEquals(55, sum);

    long lsum = (long) sumIJ().invokeExact(new Sums(), 1, 2L);
    assertEquals(3L, lsum);

    lsum = (long) sum2IJ().invokeExact(new Sums(), 1, 2L, 3, 4L);
    assertEquals(10L, lsum);

    lsum = (long) sum3IJ().invokeExact(new Sums(), 1, 2L, 3, 4L, 5, 6L);
    assertEquals(21L, lsum);

    lsum = (long) sum4IJ().invokeExact(new Sums(), 1, 2L, 3, 4L, 5, 6L, 7, 8L);
    assertEquals(36L, lsum);

    lsum = (long) sum5IJ().invokeExact(new Sums(), 1, 2L, 3, 4L, 5, 6L, 7, 8L, 9, 10L);
    assertEquals(55L, lsum);
  }

  private void $noinline$interfaceChecks() throws Throwable {
    FooBarImpl instance = new FooBarImpl();

    String result = null;
    result = (String) fooNonDefault().invokeExact((Foo) instance);
    assertEquals("FooBarImpl.nonDefault", result);

    result = (String) fooBarImplNonDefault().invokeExact(instance);
    assertEquals("FooBarImpl.nonDefault", result);

    result = (String) barDefault().invokeExact((Bar) instance);
    assertEquals("Bar.defaultToOverride", result);

    result = (String) fooDefault().invokeExact((Foo) instance);
    assertEquals("Bar.defaultToOverride", result);

    result = (String) fooBarImplDefault().invokeExact(instance);
    assertEquals("Bar.defaultToOverride", result);

    result = (String) fooNonOverriddenDefault().invokeExact((Foo) instance);
    assertEquals("Foo.nonOverriddenDefault", result);

    result = (String) barNonOverriddenDefault().invokeExact((Bar) instance);
    assertEquals("Foo.nonOverriddenDefault", result);

    ToStringable toStringable = new ToStringableImpl();
    result = (String) toStringDefinedInAnInterface().invokeExact(toStringable);
    assertEquals("ToStringableImpl", result);

    try {
      String ignored = (String) toStringDefinedInAnInterface().invokeExact(instance);
      unreachable("Should throw WMTE");
    } catch (WrongMethodTypeException expected) {}

    Impl1 impl1 = new Impl1();

    result = (String) interfaceOneMethod().invokeExact((Interface1) impl1);
    assertEquals("Impl1.methodOne", result);

    Impl2 impl2 = new Impl2();

    result = (String) interfaceTwoMethod().invokeExact((Interface2) impl2);
    assertEquals("Impl2.methodTwo", result);

    result = (String) interfaceOneMethod().invokeExact((Interface1) impl2);
    assertEquals("Impl2.methodOne", result);

    Impl3 impl3 = new Impl3();

    result = (String) interfaceThreeMethod().invokeExact((Interface3) impl3);
    assertEquals("Impl3.methodThree", result);

    result = (String) interfaceTwoMethod().invokeExact((Interface2) impl3);
    assertEquals("Impl3.methodTwo", result);

    result = (String) interfaceOneMethod().invokeExact((Interface1) impl3);
    assertEquals("Impl3.methodOne", result);

    Impl4 impl4 = new Impl4();

    result = (String) interfaceFourMethod().invokeExact((Interface4) impl4);
    assertEquals("Impl4.methodFour", result);

    result = (String) interfaceThreeMethod().invokeExact((Interface3) impl4);
    assertEquals("Impl4.methodThree", result);

    result = (String) interfaceTwoMethod().invokeExact((Interface2) impl4);
    assertEquals("Impl4.methodTwo", result);

    result = (String) interfaceOneMethod().invokeExact((Interface1) impl4);
    assertEquals("Impl4.methodOne", result);

    FooAndFooConflictImpl conflictImpl = new FooAndFooConflictImpl();

    result = (String) fooDefault().invokeExact((Foo) conflictImpl);
    assertEquals("DefaultMethodConflictImpl.defaultToOverride", result);

    FooAndFooConflict fooAndFooConflict = new FooAndFooConflict() {
      public String nonDefault() {
        throw new UnsupportedOperationException();
      }
    };

    // TODO(b/297147201): The RI throws AbsractMethodError in these cases, just like plain
    // fooAndFooConflict.defaultToOverride() call. So RI's invokeExact follows "equivalent of a
    // particular bytecode behavior". ART throws ICCE in both cases, so current implementation of
    // invokeExact does not break "equivalent ..." part of the javadoc.
    // Need to check what the spec says about the error thrown when a conflicting default method is
    // attempted to be called.
    try {
      String ignored = (String) fooAndFooConflictDefault().invokeExact(fooAndFooConflict);
      unreachable("Non-overridden default conflict method");
    } catch (IncompatibleClassChangeError expected) {}

    try {
      String ignored = (String) fooDefault().invokeExact((Foo) fooAndFooConflict);
      unreachable("Non-overridden default conflict method");
    } catch (IncompatibleClassChangeError expected) {}

    BaseInterface baseClassImpl = new BaseClassImpl();
    try {
      String ignored = (String) baseInterface().invokeExact(baseClassImpl);
      unreachable("Calling unimplemented interface method");
    } catch (AbstractMethodError expected) {}
  }

  private void $noinline$abstractClass() throws Throwable {
    FooBarImpl instance = new FooBarImpl();

    String result = null;
    result = (String) fooBarDefinedInAbstract().invokeExact((FooBar) instance);
    assertEquals("FooBar.definedInAbstract", result);

    result = (String) fooBarImplDefinedInAbstract().invokeExact(instance);
    assertEquals("FooBar.definedInAbstract", result);

    FooBar fooBar = new FooBar() {
      @Override
      public String nonDefault() {
        return "anonymous.nonDefault";
      }
    };

    result = (String) fooBarDefinedInAbstract().invokeExact(fooBar);
    assertEquals("FooBar.definedInAbstract", result);

    result = (String) fooBarNonDefault().invokeExact(fooBar);
    assertEquals("anonymous.nonDefault", result);
  }

  static void assertEquals(Object expected, Object actual) {
    if (!Objects.equals(expected, actual)) {
      throw new AssertionError("Expected: " + expected + ", got: " + actual);
    }
  }

  private static void assertEquals(int expected, int actual) {
    if (expected != actual) {
      throw new AssertionError("Expected: " + expected + ", got: " + actual);
    }
  }

  private static void assertEquals(long expected, long actual) {
    if (expected != actual) {
      throw new AssertionError("Expected: " + expected + ", got: " + actual);
    }
  }

  private static void assertEquals(double expected, double actual) {
    if (expected != actual) {
      throw new AssertionError("Expected: " + expected + ", got: " + actual);
    }
  }

  static void unreachable(String msg) {
    throw new AssertionError("Unexpectedly reached this point, but shouldn't: " + msg);
  }

  public abstract MethodHandle optionalGet();

  public abstract MethodHandle voidMethod();
  public abstract MethodHandle returnInt();
  public abstract MethodHandle returnDouble();
  public abstract MethodHandle interfaceDefaultMethod();
  public abstract MethodHandle overwrittenInterfaceDefaultMethod();
  public abstract MethodHandle exceptionThrowingMethod();
  public abstract MethodHandle staticMethod();

  public abstract MethodHandle sumI();
  public abstract MethodHandle sum2I();
  public abstract MethodHandle sum3I();
  public abstract MethodHandle sum4I();
  public abstract MethodHandle sum5I();
  public abstract MethodHandle sum6I();
  public abstract MethodHandle sum7I();
  public abstract MethodHandle sum8I();
  public abstract MethodHandle sum9I();
  public abstract MethodHandle sum10I();
  public abstract MethodHandle sumIJ();
  public abstract MethodHandle sum2IJ();
  public abstract MethodHandle sum3IJ();
  public abstract MethodHandle sum4IJ();
  public abstract MethodHandle sum5IJ();

  public abstract MethodHandle fooNonDefault();
  public abstract MethodHandle fooBarImplNonDefault();
  public abstract MethodHandle barDefault();
  public abstract MethodHandle fooDefault();
  public abstract MethodHandle fooBarImplDefault();
  public abstract MethodHandle fooNonOverriddenDefault();
  public abstract MethodHandle barNonOverriddenDefault();
  public abstract MethodHandle toStringDefinedInAnInterface();

  public abstract MethodHandle interfaceOneMethod();
  public abstract MethodHandle interfaceTwoMethod();
  public abstract MethodHandle interfaceThreeMethod();
  public abstract MethodHandle interfaceFourMethod();

  public abstract MethodHandle fooBarDefinedInAbstract();
  public abstract MethodHandle fooBarImplDefinedInAbstract();
  public abstract MethodHandle fooBarNonDefault();
  public abstract MethodHandle fooAndFooConflictDefault();
  public abstract MethodHandle baseInterface();
}
