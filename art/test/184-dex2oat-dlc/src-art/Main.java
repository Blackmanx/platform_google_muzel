/*
 * Copyright (C) 2025 The Android Open Source Project
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

import dalvik.system.DelegateLastClassLoader;

// Note: The main part of this test happens during `dex2oat` compilation of the `*-ex.jar`.
// The verification of `SubClassOfDuplicateClassUser` in `*-ex.jar` triggers verification of
// `DuplicateClassUser` in the parent class loader and that loads `DuplicateClass` from the
// parent class loader. Previously, we would record image classes only by descriptor and
// therefore the `DuplicateClass` from parent class loader would be considered an image class.
// Hovever, when compiling the `*-ex.jar`, we cannot put classes from the parent class loader
// to the image and `dex2oat` used to fail a consistency `CHECK()` in `ImageWriter`.
public class Main {
    static final String TEST_NAME = "184-dex2oat-dlc";
    static final String EX_JAR_FILE = System.getenv("DEX_LOCATION") + "/" + TEST_NAME + "-ex.jar";

    public static void main(String[] args) throws Exception {
        System.loadLibrary(args[0]);

        assertTrue("Main app image is loaded", checkAppImageLoaded("184-dex2oat-dlc"));
        assertTrue("App image contains Main", checkAppImageContains(Main.class));

        ClassLoader mainClassLoader = Main.class.getClassLoader();
        Class<?> dc_main = mainClassLoader.loadClass("DuplicateClass");

        assertFalse("Secondary app image is not loaded", checkAppImageLoaded("184-dex2oat-dlc-ex"));
        ClassLoader delegateLast = new DelegateLastClassLoader(EX_JAR_FILE, mainClassLoader);
        assertTrue("Secondary app image is loaded", checkAppImageLoaded("184-dex2oat-dlc-ex"));

        Class<?> dc_ex = delegateLast.loadClass("DuplicateClass");

        System.out.println("dc_main: " + dc_main + "; in image: " + checkAppImageContains(dc_main));
        System.out.println("dc_ex: " + dc_ex + "; in image: " + checkAppImageContains(dc_ex));
        System.out.println("(dc_main == dc_ex): " + (dc_main == dc_ex));
    }

    private static void assertTrue(String message, boolean value) {
        if (value) {
            return;
        }
        throw new AssertionError(message);
    }

    private static void assertFalse(String message, boolean value) {
        if (!value) {
            return;
        }
        throw new AssertionError(message);
    }

    public static native boolean checkAppImageLoaded(String name);
    public static native boolean checkAppImageContains(Class<?> klass);
}
