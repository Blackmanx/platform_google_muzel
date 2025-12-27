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

import annotations.ConstantMethodHandle;

import java.lang.invoke.MethodHandle;

public class ConstMethodHandleTest extends AbstractInvokeExactTest {

  @Override
  void $noinline$privateMethods() throws Throwable {
    // TODO(b/378051428): can't create const-method-handle targeting private methods of
    // inner classes.
  }

  @ConstantMethodHandle(
    kind = ConstantMethodHandle.INVOKE_VIRTUAL,
    owner = "java/util/Optional",
    fieldOrMethodName = "get",
    descriptor = "()Ljava/lang/Object;")
  private static MethodHandle constOptionalGet() {
    unreachable("should be replaced by const-method-handle");
    return null;
  }

  @Override
  public MethodHandle optionalGet() {
    return constOptionalGet();
  }

  @ConstantMethodHandle(
    kind = ConstantMethodHandle.INVOKE_VIRTUAL,
    owner = "A",
    fieldOrMethodName = "voidMethod",
    descriptor = "()V")
  private static MethodHandle constVoidMethod() {
    unreachable("should be replaced by const-method-handle");
    return null;
  }

  @Override
  public MethodHandle voidMethod() {
    return constVoidMethod();
  }

  @ConstantMethodHandle(
    kind = ConstantMethodHandle.INVOKE_VIRTUAL,
    owner = "A",
    fieldOrMethodName = "returnInt",
    descriptor = "()I")
  private static MethodHandle constReturnInt() {
    unreachable("should be replaced by const-method-handle");
    return null;
  }

  @Override
  public MethodHandle returnInt() {
    return constReturnInt();
  }

  @ConstantMethodHandle(
    kind = ConstantMethodHandle.INVOKE_VIRTUAL,
    owner = "A",
    fieldOrMethodName = "returnDouble",
    descriptor = "()D")
  private static MethodHandle constReturnDouble() {
    unreachable("should be replaced by const-method-handle");
    return null;
  }

  @Override
  public MethodHandle returnDouble() {
    return constReturnDouble();
  }

   @ConstantMethodHandle(
    kind = ConstantMethodHandle.INVOKE_INTERFACE,
    owner = "I",
    fieldOrMethodName = "defaultMethod",
    descriptor = "()V",
    ownerIsInterface = true)
  private static MethodHandle constInterfaceDefaultMethod() {
    unreachable("should be replaced by const-method-handle");
    return null;
  }

  @Override
  public MethodHandle interfaceDefaultMethod() {
    return constInterfaceDefaultMethod();
  }

  @ConstantMethodHandle(
    kind = ConstantMethodHandle.INVOKE_INTERFACE,
    owner = "I",
    fieldOrMethodName = "overrideMe",
    descriptor = "()V",
    ownerIsInterface = true)
  private static MethodHandle constOverwrittenInterfaceDefaultMethod() {
    unreachable("should be replaced by const-method-handle");
    return null;
  }

  @Override
  public MethodHandle overwrittenInterfaceDefaultMethod() {
    return constOverwrittenInterfaceDefaultMethod();
  }

  @ConstantMethodHandle(
    kind = ConstantMethodHandle.INVOKE_VIRTUAL,
    owner = "A",
    fieldOrMethodName = "throwException",
    descriptor = "()V")
  private static MethodHandle constExceptionThrowingMethod() {
    unreachable("should be replaced by const-method-handle");
    return null;
  }

  @Override
  public MethodHandle exceptionThrowingMethod() {
    return constExceptionThrowingMethod();
  }

  @ConstantMethodHandle(
    kind = ConstantMethodHandle.INVOKE_STATIC,
    owner = "A",
    fieldOrMethodName = "staticMethod",
    descriptor = "(LA;)Ljava/lang/String;")
  private static MethodHandle constStaticMethod() {
    unreachable("should be replaced by const-method-handle");
    return null;
  }

  @Override
  public MethodHandle staticMethod() {
    return constStaticMethod();
  }

  @ConstantMethodHandle(
    kind = ConstantMethodHandle.INVOKE_VIRTUAL,
    owner = "Sums",
    fieldOrMethodName = "sum",
    descriptor = "(I)I")
  private static MethodHandle constSumI() {
    unreachable("should be replaced by const-method-handle");
    return null;
  }

  @Override
  public MethodHandle sumI() {
    return constSumI();
  }

  @ConstantMethodHandle(
    kind = ConstantMethodHandle.INVOKE_VIRTUAL,
    owner = "Sums",
    fieldOrMethodName = "sum",
    descriptor = "(II)I")
  private static MethodHandle constSum2I() {
    unreachable("should be replaced by const-method-handle");
    return null;
  }

  @Override
  public MethodHandle sum2I() {
    return constSum2I();
  }

  @ConstantMethodHandle(
    kind = ConstantMethodHandle.INVOKE_VIRTUAL,
    owner = "Sums",
    fieldOrMethodName = "sum",
    descriptor = "(III)I")
  private static MethodHandle constSum3I() {
    unreachable("should be replaced by const-method-handle");
    return null;
  }

  @Override
  public MethodHandle sum3I() {
    return constSum3I();
  }

  @ConstantMethodHandle(
    kind = ConstantMethodHandle.INVOKE_VIRTUAL,
    owner = "Sums",
    fieldOrMethodName = "sum",
    descriptor = "(IIII)I")
  private static MethodHandle constSum4I() {
    unreachable("should be replaced by const-method-handle");
    return null;
  }

  @Override
  public MethodHandle sum4I() {
    return constSum4I();
  }

  @ConstantMethodHandle(
    kind = ConstantMethodHandle.INVOKE_VIRTUAL,
    owner = "Sums",
    fieldOrMethodName = "sum",
    descriptor = "(IIIII)I")
  private static MethodHandle constSum5I() {
    unreachable("should be replaced by const-method-handle");
    return null;
  }

  @Override
  public MethodHandle sum5I() {
    return constSum5I();
  }

  @ConstantMethodHandle(
    kind = ConstantMethodHandle.INVOKE_VIRTUAL,
    owner = "Sums",
    fieldOrMethodName = "sum",
    descriptor = "(IIIIII)I")
  private static MethodHandle constSum6I() {
    unreachable("should be replaced by const-method-handle");
    return null;
  }

  @Override
  public MethodHandle sum6I() {
    return constSum6I();
  }

  @ConstantMethodHandle(
    kind = ConstantMethodHandle.INVOKE_VIRTUAL,
    owner = "Sums",
    fieldOrMethodName = "sum",
    descriptor = "(IIIIIII)I")
  private static MethodHandle constSum7I() {
    unreachable("should be replaced by const-method-handle");
    return null;
  }

  @Override
  public MethodHandle sum7I() {
    return constSum7I();
  }

  @ConstantMethodHandle(
    kind = ConstantMethodHandle.INVOKE_VIRTUAL,
    owner = "Sums",
    fieldOrMethodName = "sum",
    descriptor = "(IIIIIIII)I")
  private static MethodHandle constSum8I() {
    unreachable("should be replaced by const-method-handle");
    return null;
  }

  @Override
  public MethodHandle sum8I() {
    return constSum8I();
  }

  @ConstantMethodHandle(
    kind = ConstantMethodHandle.INVOKE_VIRTUAL,
    owner = "Sums",
    fieldOrMethodName = "sum",
    descriptor = "(IIIIIIIII)I")
  private static MethodHandle constSum9I() {
    unreachable("should be replaced by const-method-handle");
    return null;
  }

  @Override
  public MethodHandle sum9I() {
    return constSum9I();
  }

  @ConstantMethodHandle(
    kind = ConstantMethodHandle.INVOKE_VIRTUAL,
    owner = "Sums",
    fieldOrMethodName = "sum",
    descriptor = "(IIIIIIIIII)I")
  private static MethodHandle constSum10I() {
    unreachable("should be replaced by const-method-handle");
    return null;
  }

  @Override
  public MethodHandle sum10I() {
    return constSum10I();
  }

  @ConstantMethodHandle(
    kind = ConstantMethodHandle.INVOKE_VIRTUAL,
    owner = "Sums",
    fieldOrMethodName = "sum",
    descriptor = "(IJ)J")
  private static MethodHandle constSumIJ() {
    unreachable("should be replaced by const-method-handle");
    return null;
  }

  @Override
  public MethodHandle sumIJ() {
    return constSumIJ();
  }

  @ConstantMethodHandle(
    kind = ConstantMethodHandle.INVOKE_VIRTUAL,
    owner = "Sums",
    fieldOrMethodName = "sum",
    descriptor = "(IJIJ)J")
  private static MethodHandle constSum2IJ() {
    unreachable("should be replaced by const-method-handle");
    return null;
  }

  @Override
  public MethodHandle sum2IJ() {
    return constSum2IJ();
  }

  @ConstantMethodHandle(
    kind = ConstantMethodHandle.INVOKE_VIRTUAL,
    owner = "Sums",
    fieldOrMethodName = "sum",
    descriptor = "(IJIJIJ)J")
  private static MethodHandle constSum3IJ() {
    unreachable("should be replaced by const-method-handle");
    return null;
  }

  @Override
  public MethodHandle sum3IJ() {
    return constSum3IJ();
  }

  @ConstantMethodHandle(
    kind = ConstantMethodHandle.INVOKE_VIRTUAL,
    owner = "Sums",
    fieldOrMethodName = "sum",
    descriptor = "(IJIJIJIJ)J")
  private static MethodHandle constSum4IJ() {
    unreachable("should be replaced by const-method-handle");
    return null;
  }

  @Override
  public MethodHandle sum4IJ() {
    return constSum4IJ();
  }


  @ConstantMethodHandle(
    kind = ConstantMethodHandle.INVOKE_VIRTUAL,
    owner = "Sums",
    fieldOrMethodName = "sum",
    descriptor = "(IJIJIJIJIJ)J")
  private static MethodHandle constSum5IJ() {
    unreachable("should be replaced by const-method-handle");
    return null;
  }

  @Override
  public MethodHandle sum5IJ() {
    return constSum5IJ();
  }

  @ConstantMethodHandle(
    kind = ConstantMethodHandle.INVOKE_INTERFACE,
    owner = "Foo",
    fieldOrMethodName = "nonDefault",
    descriptor = "()Ljava/lang/String;",
    ownerIsInterface = true)
  private static MethodHandle constFooNonDefault() {
    unreachable("should be replaced by const-method-handle");
    return null;
  }

  @Override
  public MethodHandle fooNonDefault() {
    return constFooNonDefault();
  }

  @ConstantMethodHandle(
    kind = ConstantMethodHandle.INVOKE_VIRTUAL,
    owner = "FooBarImpl",
    fieldOrMethodName = "nonDefault",
    descriptor = "()Ljava/lang/String;")
  private static MethodHandle constFooBarImplNonDefault() {
    unreachable("should be replaced by const-method-handle");
    return null;
  }

  @Override
  public MethodHandle fooBarImplNonDefault() {
    return constFooBarImplNonDefault();
  }

  @ConstantMethodHandle(
    kind = ConstantMethodHandle.INVOKE_INTERFACE,
    owner = "Bar",
    fieldOrMethodName = "defaultToOverride",
    descriptor = "()Ljava/lang/String;",
    ownerIsInterface = true)
  private static MethodHandle constBarDefault() {
    unreachable("should be replaced by const-method-handle");
    return null;
  }

  @Override
  public MethodHandle barDefault() {
    return constBarDefault();
  }

  @ConstantMethodHandle(
    kind = ConstantMethodHandle.INVOKE_INTERFACE,
    owner = "Foo",
    fieldOrMethodName = "defaultToOverride",
    descriptor = "()Ljava/lang/String;",
    ownerIsInterface = true)
  private static MethodHandle constFooDefault() {
    unreachable("should be replaced by const-method-handle");
    return null;
  }

  @Override
  public MethodHandle fooDefault() {
    return constFooDefault();
  }

  @ConstantMethodHandle(
    kind = ConstantMethodHandle.INVOKE_VIRTUAL,
    owner = "FooBarImpl",
    fieldOrMethodName = "defaultToOverride",
    descriptor = "()Ljava/lang/String;")
  private static MethodHandle constFooBarImplDefault() {
    unreachable("should be replaced by const-method-handle");
    return null;
  }

  @Override
  public MethodHandle fooBarImplDefault() {
    return constFooBarImplDefault();
  }

  @ConstantMethodHandle(
    kind = ConstantMethodHandle.INVOKE_INTERFACE,
    owner = "Foo",
    fieldOrMethodName = "nonOverriddenDefault",
    descriptor = "()Ljava/lang/String;",
    ownerIsInterface = true)
  private static MethodHandle constFooNonOverriddenDefault() {
    unreachable("should be replaced by const-method-handle");
    return null;
  }

  @Override
  public MethodHandle fooNonOverriddenDefault() {
    return constFooNonOverriddenDefault();
  }

  @ConstantMethodHandle(
    kind = ConstantMethodHandle.INVOKE_INTERFACE,
    owner = "Bar",
    fieldOrMethodName = "nonOverriddenDefault",
    descriptor = "()Ljava/lang/String;",
    ownerIsInterface = true)
  private static MethodHandle constBarNonOverriddenDefault() {
    unreachable("should be replaced by const-method-handle");
    return null;
  }

  @Override
  public MethodHandle barNonOverriddenDefault() {
    return constBarNonOverriddenDefault();
  }

  @ConstantMethodHandle(
    kind = ConstantMethodHandle.INVOKE_VIRTUAL,
    owner = "FooBar",
    fieldOrMethodName = "definedInAbstract",
    descriptor = "()Ljava/lang/String;")
  private static MethodHandle constFooBarDefinedInAbstract() {
    unreachable("should be replaced by const-method-handle");
    return null;
  }

  @Override
  public MethodHandle fooBarDefinedInAbstract() {
    return constFooBarDefinedInAbstract();
  }

  @ConstantMethodHandle(
    kind = ConstantMethodHandle.INVOKE_VIRTUAL,
    owner = "FooBarImpl",
    fieldOrMethodName = "definedInAbstract",
    descriptor = "()Ljava/lang/String;")
  private static MethodHandle constFooBarImplDefinedInAbstract() {
    unreachable("should be replaced by const-method-handle");
    return null;
  }

  @Override
  public MethodHandle fooBarImplDefinedInAbstract() {
    return constFooBarImplDefinedInAbstract();
  }

  @ConstantMethodHandle(
    kind = ConstantMethodHandle.INVOKE_VIRTUAL,
    owner = "FooBar",
    fieldOrMethodName = "nonDefault",
    descriptor = "()Ljava/lang/String;")
  private static MethodHandle constFooBarNonDefault() {
    unreachable("should be replaced by const-method-handle");
    return null;
  }

  @Override
  public MethodHandle fooBarNonDefault() {
    return constFooBarNonDefault();
  }

  @ConstantMethodHandle(
    kind = ConstantMethodHandle.INVOKE_INTERFACE,
    owner = "ToStringable",
    fieldOrMethodName = "toString",
    descriptor = "()Ljava/lang/String;",
    ownerIsInterface = true)
  private static MethodHandle constToStringDefinedInAnInterface() {
    unreachable("should be replaced by const-method-handle");
    return null;
  }

  @Override
  public MethodHandle toStringDefinedInAnInterface() {
    return constToStringDefinedInAnInterface();
  }

  @ConstantMethodHandle(
    kind = ConstantMethodHandle.INVOKE_INTERFACE,
    owner = "Interface1",
    fieldOrMethodName = "methodOne",
    descriptor = "()Ljava/lang/String;",
    ownerIsInterface = true)
  private static MethodHandle constInterfaceOneMethod() {
    unreachable("should be replaced by const-method-handle");
    return null;
  }

  @Override
  public MethodHandle interfaceOneMethod() {
    return constInterfaceOneMethod();
  }

  @ConstantMethodHandle(
    kind = ConstantMethodHandle.INVOKE_INTERFACE,
    owner = "Interface2",
    fieldOrMethodName = "methodTwo",
    descriptor = "()Ljava/lang/String;",
    ownerIsInterface = true)
  private static MethodHandle constInterfaceTwoMethod() {
    unreachable("should be replaced by const-method-handle");
    return null;
  }

  @Override
  public MethodHandle interfaceTwoMethod() {
    return constInterfaceTwoMethod();
  }

  @ConstantMethodHandle(
    kind = ConstantMethodHandle.INVOKE_INTERFACE,
    owner = "Interface3",
    fieldOrMethodName = "methodThree",
    descriptor = "()Ljava/lang/String;",
    ownerIsInterface = true)
  private static MethodHandle constInterfaceThreeMethod() {
    unreachable("should be replaced by const-method-handle");
    return null;
  }

  @Override
  public MethodHandle interfaceThreeMethod() {
    return constInterfaceThreeMethod();
  }

  @ConstantMethodHandle(
    kind = ConstantMethodHandle.INVOKE_INTERFACE,
    owner = "Interface4",
    fieldOrMethodName = "methodFour",
    descriptor = "()Ljava/lang/String;",
    ownerIsInterface = true)
  private static MethodHandle constInterfaceFourMethod() {
    unreachable("should be replaced by const-method-handle");
    return null;
  }

  @Override
  public MethodHandle interfaceFourMethod() {
    return constInterfaceFourMethod();
  }

  @ConstantMethodHandle(
    kind = ConstantMethodHandle.INVOKE_INTERFACE,
    owner = "FooAndFooConflict",
    fieldOrMethodName = "defaultToOverride",
    descriptor = "()Ljava/lang/String;",
    ownerIsInterface = true)
  private static MethodHandle constFooAndFooConflictDefault() {
    unreachable("should be replaced by const-method-handle");
    return null;
  }

  @Override
  public MethodHandle fooAndFooConflictDefault() {
    return constFooAndFooConflictDefault();
  }

  @ConstantMethodHandle(
    kind = ConstantMethodHandle.INVOKE_INTERFACE,
    owner = "BaseInterface",
    fieldOrMethodName = "method",
    descriptor = "()Ljava/lang/String;",
    ownerIsInterface = true)
  private static MethodHandle constBaseInterface() {
    unreachable("should be replaced by const-method-handle");
    return null;
  }

  @Override
  public MethodHandle baseInterface() {
    return constBaseInterface();
  }
}
