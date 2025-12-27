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

import java.io.File;
import java.io.IOException;

import dalvik.system.VMDebug;

public class Main {

    private static final String TEMP_FILE_NAME_PREFIX = "test";
    private static final String TEMP_FILE_NAME_SUFFIX = ".trace";

    private static File createTempFile() throws Exception {
        try {
            return  File.createTempFile(TEMP_FILE_NAME_PREFIX, TEMP_FILE_NAME_SUFFIX);
        } catch (IOException e) {
            System.setProperty("java.io.tmpdir", "/data/local/tmp");
            try {
                return File.createTempFile(TEMP_FILE_NAME_PREFIX, TEMP_FILE_NAME_SUFFIX);
            } catch (IOException e2) {
                System.setProperty("java.io.tmpdir", "/sdcard");
                return File.createTempFile(TEMP_FILE_NAME_PREFIX, TEMP_FILE_NAME_SUFFIX);
            }
        }
    }

  public static void main(String[] args) throws Exception {
    System.loadLibrary(args[0]);

    // In case we already run in tracing mode, disable it.
    if (VMDebug.getMethodTracingMode() != 0) {
        VMDebug.stopMethodTracing();
    }

    File tempFile = createTempFile();

    // Start method tracing so that native methods get the generic JNI stub.
    VMDebug.startMethodTracing(tempFile.getPath(), 0, 0, false, 0);

    // We need the caller of `throwsException` to be nterp or compiled, because the clinit check is
    // executed within the generic JNI stub. The switch interpreter does a clinit check before the
    // invoke, and that doesn't trigger the bug.
    Main.ensureJitCompiled(Runner.class, "run");

    // We want the `Test` class to be loaded after `startMethodTracing`. So we call it through
    // `Runner` to avoid the verification of the `Main` class preload `Test`.
    Runner.run();
  }

  public static native void ensureJitCompiled(Class<?> cls, String method);
}

class Runner {
  public static void run() {
    try {
        // Will call through the generic JNI stub and when returning from the native code will need
        // to walk the stack to throw the exception. We used to crash when walking the stack because
        // we did not expect to have a generic JNI PC with an entrypoint from a shared boot image
        // JNI stub.
        Test.throwsException();
    } catch (Test e) {
        if (!Test.ranInitializer) {
            throw new Error("Expected Test.ranInitializer to be true");
        }
        return;
    }
    throw new Error("Expected exception");
  }
}

class Test extends Exception {
  static {
      ranInitializer = true;
      // Will update the entrypoint of `Test.throwsException` to point to a JNI stub from the boot
      // image.
      VMDebug.stopMethodTracing();
  }

  static boolean ranInitializer;
  static native void throwsException() throws Test;
}
