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

import java.lang.reflect.Constructor;
import java.lang.reflect.Method;

public class Main {
  static final String DEX_FILE =
      System.getenv("DEX_LOCATION") + "/854-image-inlining-ex.jar";
  static final String LIBRARY_SEARCH_PATH = System.getProperty("java.library.path");

  public static void main(String[] args) throws Exception {
    // Create the secondary class loader.
    Class<?> pathClassLoader = Class.forName("dalvik.system.PathClassLoader");
    Constructor<?> constructor =
        pathClassLoader.getDeclaredConstructor(String.class, String.class, ClassLoader.class);
    ClassLoader loader = (ClassLoader) constructor.newInstance(
        DEX_FILE, LIBRARY_SEARCH_PATH, ClassLoader.getSystemClassLoader());

    // Invoke the test method to trigger the runtime crash.
    Method m = loader.loadClass("Test").getDeclaredMethod("callInstance");
    int result = ((Integer) m.invoke(null)).intValue();
    if (result != 42) {
      throw new Error("Expected 42, got " + result);
    }
  }
}
