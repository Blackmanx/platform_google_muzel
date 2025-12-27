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

import java.lang.invoke.MethodType;
import java.lang.ref.Reference;
import java.lang.ref.WeakReference;
import java.lang.reflect.Constructor;
import java.lang.reflect.Method;

public class Main {
    private static final String DEX_FILE =
        System.getenv("DEX_LOCATION") + "/2276-const-method-type-gc-cleanup-ex.jar";
    private static final String LIBRARY_SEARCH_PATH = System.getProperty("java.library.path");

    public static void main(String[] args) throws Throwable {
        System.loadLibrary(args[0]);

        Class<?> pathClassLoader = Class.forName("dalvik.system.PathClassLoader");
        if (pathClassLoader == null) {
            throw new AssertionError("Couldn't find path class loader class");
        }
        Constructor<?> constructor =
            pathClassLoader.getDeclaredConstructor(String.class, String.class, ClassLoader.class);

        // Identical to Worker.returnStringMethodType() and is captured by JIT-ed code.
        MethodType returnStringMethodType = MethodType.methodType(String.class);

        for (int i = 0; i < 10; ++i) {
            callDoWork(constructor);
        }

        Reference.reachabilityFence(returnStringMethodType);
    }

    private static void callDoWork(Constructor constructor) throws Throwable {
        WeakReference loaderRef = $noinline$doRealWork(constructor);

        doUnload();

        if (loaderRef.refersTo(null)) {
            System.out.println("ClassLoader was unloaded");
        }
    }

    private static WeakReference $noinline$doRealWork(Constructor constructor) throws Throwable {
        ClassLoader loader = (ClassLoader) constructor.newInstance(
                DEX_FILE, LIBRARY_SEARCH_PATH, ClassLoader.getSystemClassLoader());

        Class workerClass = loader.loadClass("Worker");

        if (workerClass.getClassLoader() != loader) {
            throw new AssertionError("The class was loaded by a wrong ClassLoader");
        }

        ensureJitCompiled(workerClass, "doWork");

        Method m = workerClass.getDeclaredMethod("doWork");
        m.invoke(null);

        WeakReference loaderRef = new WeakReference(loader);

        m = null;
        workerClass = null;
        loader = null;

        return loaderRef;
    }

    private static void doUnload() {
        for (int i = 0; i < 3; ++i) {
            Runtime.getRuntime().gc();
            System.runFinalization();
        }
    }

    public static native void ensureJitCompiled(Class<?> cls, String methodName);
}
