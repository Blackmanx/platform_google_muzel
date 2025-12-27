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

import java.io.File;
import java.io.FileDescriptor;
import java.io.FileOutputStream;
import java.io.IOException;
import java.lang.reflect.Method;

public class Main {
    private static final String TEMP_FILE_NAME_PREFIX = "test";
    private static final String TEMP_FILE_NAME_SUFFIX = ".trace";
    private static File file;

    public static void main(String[] args) throws Exception {
        System.loadLibrary(args[0]);
        String name = System.getProperty("java.vm.name");
        if (!"Dalvik".equals(name)) {
            System.out.println("This test is not supported on " + name);
            return;
        }

        if (VMDebug.getMethodTracingMode() != 0) {
            VMDebug.$noinline$stopMethodTracing();
        }

        Main m = new Main();
        boolean isAot_main_before = isAotCompiled(Main.class, "main");
        boolean isAot_callOuter_before = isAotCompiled(Main.class, "callOuterFunction");

        file = createTempFile();
        FileOutputStream out_file = new FileOutputStream(file);
        VMDebug.startMethodTracing(file.getPath(), out_file.getFD(), /*buffer_size=*/0, /*flags=*/0,
                /*sampling=*/false, /*sampling_interval*/ 0, /*streaming=*/true);
        m.$noinline$doSomeWork();

        VMDebug.$noinline$stopMethodTracing();
        out_file.close();
        file.delete();

        boolean isAot_main_after = isAotCompiled(Main.class, "main");
        boolean isAot_callOuter_after = isAotCompiled(Main.class, "callOuterFunction");
        if (isAot_main_before != isAot_main_after) {
            throw new Exception("AOT code for main not restored after method tracing? "
                    + isAot_main_before + " " + isAot_main_after);
        }

        if (isAot_callOuter_before != isAot_callOuter_after) {
            throw new Exception("AOT code for callOuter not restored after method tracing? "
                    + isAot_callOuter_before + " " + isAot_callOuter_after);
        }
    }

    private static File createTempFile() throws Exception {
        try {
            return File.createTempFile(TEMP_FILE_NAME_PREFIX, TEMP_FILE_NAME_SUFFIX);
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

    public void callOuterFunction() {
        callLeafFunction();
    }

    public void callLeafFunction() {}

    public void $noinline$doSomeWork() {
        callOuterFunction();
        callLeafFunction();
    }

    private static class VMDebug {
        private static final Method startMethodTracingMethod;
        private static final Method stopMethodTracingMethod;
        private static final Method getMethodTracingModeMethod;
        static {
            try {
                Class<?> c = Class.forName("dalvik.system.VMDebug");
                startMethodTracingMethod = c.getDeclaredMethod("startMethodTracing", String.class,
                        FileDescriptor.class, Integer.TYPE, Integer.TYPE, Boolean.TYPE,
                        Integer.TYPE, Boolean.TYPE);
                stopMethodTracingMethod = c.getDeclaredMethod("stopMethodTracing");
                getMethodTracingModeMethod = c.getDeclaredMethod("getMethodTracingMode");
            } catch (Exception e) {
                throw new RuntimeException(e);
            }
        }

        public static void startMethodTracing(String filename, FileDescriptor fd, int bufferSize,
                int flags, boolean samplingEnabled, int intervalUs, boolean streaming)
                throws Exception {
            startMethodTracingMethod.invoke(
                    null, filename, fd, bufferSize, flags, samplingEnabled, intervalUs, streaming);
        }
        public static void $noinline$stopMethodTracing() throws Exception {
            stopMethodTracingMethod.invoke(null);
        }
        public static int getMethodTracingMode() throws Exception {
            return (int) getMethodTracingModeMethod.invoke(null);
        }
    }

    private native static boolean isAotCompiled(Class<?> cls, String methodName);
}
