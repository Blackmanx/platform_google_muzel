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
import java.util.Optional;

public class Main {

  public static void main(String[] args) throws Throwable {
    $noinline$testMethodHandleFromOtherDex();

    new ConstMethodHandleTest().runAll();
    new JavaApiTest().runAll();
  }

  private static void $noinline$testMethodHandleFromOtherDex() throws Throwable {
    MethodHandle mh = Multi.$noinline$getMethodHandle();
    Optional<String> nonEmpty = Optional.<String>of("hello");
    Object returnedObject = mh.invokeExact(nonEmpty);
    assertEquals("hello", returnedObject);

    try {
      mh.invokeExact(nonEmpty);
      unreachable("mh.type() is (Optional)Object, but callsite is (Optional)V");
    } catch (WrongMethodTypeException expected) {}
  }

  private static void assertEquals(Object expected, Object actual) {
    if (!Objects.equals(expected, actual)) {
      throw new AssertionError("Expected: " + expected + ", got: " + actual);
    }
  }

  private static void unreachable(String msg) {
    throw new AssertionError("Unexpectedly reached this point, but shouldn't: " + msg);
  }
}
