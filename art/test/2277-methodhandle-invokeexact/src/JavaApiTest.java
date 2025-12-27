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

import static java.lang.invoke.MethodType.methodType;

import java.lang.invoke.MethodHandle;
import java.lang.invoke.MethodHandles;
import java.util.Arrays;
import java.util.Optional;

public class JavaApiTest extends AbstractInvokeExactTest {

  private static final MethodHandle OPTIONAL_GET;

  private static final MethodHandle VOID_METHOD;
  private static final MethodHandle RETURN_INT;
  private static final MethodHandle RETURN_DOUBLE;
  private static final MethodHandle PRIVATE_INTERFACE_METHOD;
  private static final MethodHandle B_PRIVATE_RETURN_INT;
  private static final MethodHandle A_PRIVATE_RETURN_INT;
  private static final MethodHandle STATIC_METHOD;
  private static final MethodHandle EXCEPTION_THROWING_METHOD;
  private static final MethodHandle INTERFACE_DEFAULT_METHOD;
  private static final MethodHandle OVERWRITTEN_INTERFACE_DEFAULT_METHOD;

  private static final MethodHandle SUM_I;
  private static final MethodHandle SUM_2I;
  private static final MethodHandle SUM_3I;
  private static final MethodHandle SUM_4I;
  private static final MethodHandle SUM_5I;
  private static final MethodHandle SUM_6I;
  private static final MethodHandle SUM_7I;
  private static final MethodHandle SUM_8I;
  private static final MethodHandle SUM_9I;
  private static final MethodHandle SUM_10I;

  private static final MethodHandle SUM_IJ;
  private static final MethodHandle SUM_2IJ;
  private static final MethodHandle SUM_3IJ;
  private static final MethodHandle SUM_4IJ;
  private static final MethodHandle SUM_5IJ;

  private static final MethodHandle FOO_NONDEFAULT;
  private static final MethodHandle FOOBARIMPL_NONDEFAULT;
  private static final MethodHandle FOO_DEFAULT;
  private static final MethodHandle BAR_DEFAULT;
  private static final MethodHandle FOOBAR_DEFINEDINABSTRACT;
  private static final MethodHandle FOOBAR_NONDEFAULT;
  private static final MethodHandle FOOBARIMPL_DEFINEDINABSTRACT;
  private static final MethodHandle FOOBARIMPL_DEFAULT;
  private static final MethodHandle FOO_NONOVERRIDDEN_DEFAULT;
  private static final MethodHandle BAR_NONOVERRIDDEN_DEFAULT;
  private static final MethodHandle TO_STRING_DEFINED_IN_AN_INTERFACE;

  private static final MethodHandle INTERFACE_ONE_METHOD;
  private static final MethodHandle INTERFACE_TWO_METHOD;
  private static final MethodHandle INTERFACE_THREE_METHOD;
  private static final MethodHandle INTERFACE_FOUR_METHOD;
  private static final MethodHandle FOO_AND_FOO_CONFLICT_DEFAULT;
  private static final MethodHandle BASE_INTERFACE;

  static {
    try {
      OPTIONAL_GET = MethodHandles.lookup()
          .findVirtual(Optional.class, "get", methodType(Object.class));

      VOID_METHOD = MethodHandles.lookup()
          .findVirtual(A.class, "voidMethod", methodType(void.class));
      RETURN_DOUBLE = MethodHandles.lookup()
          .findVirtual(A.class, "returnDouble", methodType(double.class));
      RETURN_INT = MethodHandles.lookup()
          .findVirtual(A.class, "returnInt", methodType(int.class));
      PRIVATE_INTERFACE_METHOD = MethodHandles.privateLookupIn(I.class, MethodHandles.lookup())
          .findVirtual(I.class, "innerPrivateMethod", methodType(String.class));
      A_PRIVATE_RETURN_INT = MethodHandles.privateLookupIn(A.class, MethodHandles.lookup())
          .findVirtual(A.class, "privateReturnInt", methodType(int.class));
      B_PRIVATE_RETURN_INT = MethodHandles.privateLookupIn(B.class, MethodHandles.lookup())
          .findVirtual(B.class, "privateReturnInt", methodType(int.class));
      STATIC_METHOD = MethodHandles.lookup()
          .findStatic(A.class, "staticMethod", methodType(String.class, A.class));
      EXCEPTION_THROWING_METHOD = MethodHandles.lookup()
          .findVirtual(A.class, "throwException", methodType(void.class));
      INTERFACE_DEFAULT_METHOD = MethodHandles.lookup()
          .findVirtual(I.class, "defaultMethod", methodType(void.class));
      OVERWRITTEN_INTERFACE_DEFAULT_METHOD = MethodHandles.lookup()
          .findVirtual(I.class, "overrideMe", methodType(void.class));

      SUM_I  = MethodHandles.lookup()
          .findVirtual(Sums.class, "sum", methodType(int.class, repeat(1, int.class)));
      SUM_2I = MethodHandles.lookup()
          .findVirtual(Sums.class, "sum", methodType(int.class, repeat(2, int.class)));
      SUM_3I = MethodHandles.lookup()
          .findVirtual(Sums.class, "sum", methodType(int.class, repeat(3, int.class)));
      SUM_4I  = MethodHandles.lookup()
          .findVirtual(Sums.class, "sum", methodType(int.class, repeat(4, int.class)));
      SUM_5I  = MethodHandles.lookup()
          .findVirtual(Sums.class, "sum", methodType(int.class, repeat(5, int.class)));
      SUM_6I  = MethodHandles.lookup()
          .findVirtual(Sums.class, "sum", methodType(int.class, repeat(6, int.class)));
      SUM_7I  = MethodHandles.lookup()
          .findVirtual(Sums.class, "sum", methodType(int.class, repeat(7, int.class)));
      SUM_8I  = MethodHandles.lookup()
          .findVirtual(Sums.class, "sum", methodType(int.class, repeat(8, int.class)));
      SUM_9I  = MethodHandles.lookup()
          .findVirtual(Sums.class, "sum", methodType(int.class, repeat(9, int.class)));
      SUM_10I = MethodHandles.lookup()
          .findVirtual(Sums.class, "sum", methodType(int.class, repeat(10, int.class)));

      SUM_IJ = MethodHandles.lookup()
          .findVirtual(Sums.class, "sum", methodType(long.class, int.class, long.class));
      SUM_2IJ  = MethodHandles.lookup()
          .findVirtual(Sums.class,
                       "sum",
                       methodType(long.class, repeat(2, int.class, long.class)));
      SUM_3IJ = MethodHandles.lookup()
          .findVirtual(Sums.class,
                       "sum",
                       methodType(long.class, repeat(3, int.class, long.class)));
      SUM_4IJ = MethodHandles.lookup()
          .findVirtual(Sums.class,
                       "sum",
                       methodType(long.class, repeat(4, int.class, long.class)));
      SUM_5IJ = MethodHandles.lookup()
          .findVirtual(Sums.class,
                       "sum",
                       methodType(long.class, repeat(5, int.class, long.class)));

      FOO_NONDEFAULT = MethodHandles.lookup()
          .findVirtual(Foo.class, "nonDefault", methodType(String.class));
      FOOBARIMPL_NONDEFAULT = MethodHandles.lookup()
          .findVirtual(FooBarImpl.class, "nonDefault", methodType(String.class));
      FOO_DEFAULT = MethodHandles.lookup()
          .findVirtual(Foo.class, "defaultToOverride", methodType(String.class));
      BAR_DEFAULT = MethodHandles.lookup()
          .findVirtual(Bar.class, "defaultToOverride", methodType(String.class));
      FOOBAR_DEFINEDINABSTRACT = MethodHandles.lookup()
          .findVirtual(FooBar.class, "definedInAbstract", methodType(String.class));
      FOOBAR_NONDEFAULT = MethodHandles.lookup()
          .findVirtual(FooBar.class, "nonDefault", methodType(String.class));
      FOOBARIMPL_DEFINEDINABSTRACT = MethodHandles.lookup()
          .findVirtual(FooBarImpl.class, "definedInAbstract", methodType(String.class));
      FOOBARIMPL_DEFAULT = MethodHandles.lookup()
          .findVirtual(FooBarImpl.class, "defaultToOverride", methodType(String.class));
      FOO_NONOVERRIDDEN_DEFAULT = MethodHandles.lookup()
          .findVirtual(Foo.class, "nonOverriddenDefault", methodType(String.class));
      BAR_NONOVERRIDDEN_DEFAULT = MethodHandles.lookup()
          .findVirtual(Bar.class, "nonOverriddenDefault", methodType(String.class));
      TO_STRING_DEFINED_IN_AN_INTERFACE = MethodHandles.lookup()
          .findVirtual(ToStringable.class, "toString", methodType(String.class));

      INTERFACE_ONE_METHOD = MethodHandles.lookup()
          .findVirtual(Interface1.class, "methodOne", methodType(String.class));
      INTERFACE_TWO_METHOD = MethodHandles.lookup()
          .findVirtual(Interface2.class, "methodTwo", methodType(String.class));
      INTERFACE_THREE_METHOD = MethodHandles.lookup()
          .findVirtual(Interface3.class, "methodThree", methodType(String.class));
      INTERFACE_FOUR_METHOD = MethodHandles.lookup()
          .findVirtual(Interface4.class, "methodFour", methodType(String.class));
      FOO_AND_FOO_CONFLICT_DEFAULT = MethodHandles.lookup()
          .findVirtual(FooAndFooConflict.class, "defaultToOverride", methodType(String.class));
      BASE_INTERFACE = MethodHandles.lookup()
          .findVirtual(BaseInterface.class, "method", methodType(String.class));
    } catch (Exception e) {
      throw new RuntimeException(e);
    }
  }

  private static Class<?>[] repeat(int times, Class<?> clazz) {
    Class<?>[] classes = new Class<?>[times];
    Arrays.fill(classes, clazz);
    return classes;
  }

  private static Class<?>[] repeat(int times, Class<?> first, Class<?> second) {
    Class<?>[] classes = new Class<?>[times * 2];
    for (int i = 0; i < 2 * times;) {
      classes[i++] = first;
      classes[i++] = second;
    }
    return classes;
  }

  @Override
  void $noinline$privateMethods() throws Throwable {
    assertEquals("boo", (String) PRIVATE_INTERFACE_METHOD.invokeExact((I) new A()));

    int privateIntA = (int) A_PRIVATE_RETURN_INT.invokeExact(new A());
    assertEquals(1042, privateIntA);

    int privateIntB = (int) B_PRIVATE_RETURN_INT.invokeExact(new B());
    assertEquals(9999, privateIntB);

    privateIntB = (int) B_PRIVATE_RETURN_INT.invokeExact((B) new A());
    assertEquals(9999, privateIntB);
  }

  @Override
  public MethodHandle voidMethod() {
    return VOID_METHOD;
  }

  @Override
  public MethodHandle returnDouble() {
    return RETURN_DOUBLE;
  }

  @Override
  public MethodHandle returnInt() {
    return RETURN_INT;
  }

  @Override
  public MethodHandle interfaceDefaultMethod() {
    return INTERFACE_DEFAULT_METHOD;
  }

  @Override
  public MethodHandle overwrittenInterfaceDefaultMethod() {
    return OVERWRITTEN_INTERFACE_DEFAULT_METHOD;
  }

  @Override
  public MethodHandle exceptionThrowingMethod() {
    return EXCEPTION_THROWING_METHOD;
  }

  @Override
  public MethodHandle staticMethod() {
    return STATIC_METHOD;
  }

  @Override
  public MethodHandle sumI() {
    return SUM_I;
  }

  @Override
  public MethodHandle sum2I() {
    return SUM_2I;
  }

  @Override
  public MethodHandle sum3I() {
    return SUM_3I;
  }

  @Override
  public MethodHandle sum4I() {
    return SUM_4I;
  }

  @Override
  public MethodHandle sum5I() {
    return SUM_5I;
  }

  @Override
  public MethodHandle sum6I() {
    return SUM_6I;
  }

  @Override
  public MethodHandle sum7I() {
    return SUM_7I;
  }

  @Override
  public MethodHandle sum8I() {
    return SUM_8I;
  }

  @Override
  public MethodHandle sum9I() {
    return SUM_9I;
  }

  @Override
  public MethodHandle sum10I() {
    return SUM_10I;
  }

  @Override
  public MethodHandle sumIJ() {
    return SUM_IJ;
  }

  @Override
  public MethodHandle sum2IJ() {
    return SUM_2IJ;
  }

  @Override
  public MethodHandle sum3IJ() {
    return SUM_3IJ;
  }

  @Override
  public MethodHandle sum4IJ() {
    return SUM_4IJ;
  }

  @Override
  public MethodHandle sum5IJ() {
    return SUM_5IJ;
  }

  @Override
  public MethodHandle fooNonDefault() {
    return FOO_NONDEFAULT;
  }

  @Override
  public MethodHandle fooBarImplNonDefault() {
    return FOOBARIMPL_NONDEFAULT;
  }

  @Override
  public MethodHandle barDefault() {
    return BAR_DEFAULT;
  }

  @Override
  public MethodHandle fooDefault() {
    return FOO_DEFAULT;
  }

  @Override
  public MethodHandle fooBarImplDefault() {
    return FOOBARIMPL_DEFAULT;
  }

  @Override
  public MethodHandle fooNonOverriddenDefault() {
    return FOO_NONOVERRIDDEN_DEFAULT;
  }

  @Override
  public MethodHandle barNonOverriddenDefault() {
    return BAR_NONOVERRIDDEN_DEFAULT;
  }

  @Override
  public MethodHandle fooBarDefinedInAbstract() {
    return FOOBAR_DEFINEDINABSTRACT;
  }

  @Override
  public MethodHandle fooBarImplDefinedInAbstract() {
    return FOOBARIMPL_DEFINEDINABSTRACT;
  }

  @Override
  public MethodHandle fooBarNonDefault() {
    return FOOBAR_NONDEFAULT;
  }

  @Override
  public MethodHandle toStringDefinedInAnInterface() {
    return TO_STRING_DEFINED_IN_AN_INTERFACE;
  }

  @Override
  public MethodHandle optionalGet() {
    return OPTIONAL_GET;
  }

  @Override
  public MethodHandle interfaceOneMethod() {
    return INTERFACE_ONE_METHOD;
  }

  @Override
  public MethodHandle interfaceTwoMethod() {
    return INTERFACE_TWO_METHOD;
  }

  @Override
  public MethodHandle interfaceThreeMethod() {
    return INTERFACE_THREE_METHOD;
  }

  @Override
  public MethodHandle interfaceFourMethod() {
    return INTERFACE_FOUR_METHOD;
  }

  @Override
  public MethodHandle fooAndFooConflictDefault() {
    return FOO_AND_FOO_CONFLICT_DEFAULT;
  }

  @Override
  public MethodHandle baseInterface() {
    return BASE_INTERFACE;
  }
}
