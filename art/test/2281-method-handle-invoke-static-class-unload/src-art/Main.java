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

import dalvik.system.InMemoryDexClassLoader;

import static java.lang.invoke.MethodType.methodType;

import java.lang.invoke.MethodHandle;
import java.lang.invoke.MethodHandles;
import java.lang.ref.WeakReference;
import java.lang.reflect.Constructor;
import java.lang.reflect.Method;
import java.nio.ByteBuffer;
import java.util.Base64;
import java.util.List;
import java.util.function.Consumer;

public class Main {
    // src-ex/ClassWithAMethod.java
    private static final String DEX_FILE =
        "ZGV4CjA0MQC4x9/g6TtIcd5240Zevdlr1D/wRBHV4Tv0BAAAeAAAAHhWNBIAAAAAAAAAADAEAAAW" +
        "AAAAeAAAAAcAAADQAAAABAAAAOwAAAAAAAAAAAAAAAcAAAAcAQAAAQAAAFQBAAAAAAAAAAAAAPQE" +
        "AAAAAAAAGgIAAB8CAAAnAgAAOQIAAFACAABTAgAAVgIAAGoCAACOAgAAqgIAAL4CAADfAgAA4gIA" +
        "AOYCAAD5AgAABQMAABgDAAAeAwAAJgMAADQDAAA7AwAASAMAAAQAAAAGAAAABwAAAAgAAAAJAAAA" +
        "CgAAAAsAAAAEAAAAAAAAAAAAAAAFAAAABQAAAAAAAAALAAAABgAAAAAAAAAMAAAABgAAABQCAAAB" +
        "AAIAAQAAAAEAAQANAAAAAQAAABEAAAABAAEAEgAAAAEAAgAUAAAAAwADAAEAAAAEAAIAAQAAAAEA" +
        "AAABAAAABAAAAAAAAAADAAAAGAQAAPYDAAAAAAAABAAAAAIAAAABAAAAAAAAAPwBAAACAAAAEhAP" +
        "AAEAAAAAAAAAAAIAAAMAAAD+AAAAEQAAAAEAAAAAAAAABAIAAAUAAABxAAQAAAASABEAAAABAAEA" +
        "AQAAAAkCAAAEAAAAcBAGAAAADgACAAAAAgAAAA0CAAAIAAAAIgADABoBFABwIAUAEAAnABoADgAs" +
        "AA4AJwAOPAAYAA4AHgAOAAAAAAEAAAAEAAMoKUkABjxpbml0PgAQQ2xhc3NXaXRoQU1ldGhvZAAV" +
        "Q2xhc3NXaXRoQU1ldGhvZC5qYXZhAAFJAAFMABJMQ2xhc3NXaXRoQU1ldGhvZDsAIkxhbm5vdGF0" +
        "aW9ucy9Db25zdGFudE1ldGhvZEhhbmRsZTsAGkxqYXZhL2xhbmcvQXNzZXJ0aW9uRXJyb3I7ABJM" +
        "amF2YS9sYW5nL09iamVjdDsAH0xqYXZhL2xhbmcvaW52b2tlL01ldGhvZEhhbmRsZTsAAVYAAlZM" +
        "ABFjb25zdE1ldGhvZEhhbmRsZQAKZGVzY3JpcHRvcgARZmllbGRPck1ldGhvZE5hbWUABGtpbmQA" +
        "Bm1ldGhvZAAMbWV0aG9kSGFuZGxlAAVvd25lcgALdW5yZWFjaGFibGUAnAF+fkQ4eyJiYWNrZW5k" +
        "IjoiZGV4IiwiY29tcGlsYXRpb24tbW9kZSI6ImRlYnVnIiwiaGFzLWNoZWNrc3VtcyI6ZmFsc2Us" +
        "Im1pbi1hcGkiOjI4LCJzaGEtMSI6IjdkZTM3N2E4MGI1MzQ1MGY1Y2ExYTNmYzUwZjZkYzk5M2U3" +
        "ZThhM2UiLCJ2ZXJzaW9uIjoiOC44LjQtZGV2In0AAQIEDhcADxcREAQEExcCAAAFAACBgATEAwEJ" +
        "kAMBCfwCAQqoAwEK3AMBAAAA5wMAAAAAAAAAAAAAAQAAAAAAAAADAAAAEAQAABAAAAAAAAAAAQAA" +
        "AAAAAAABAAAAFgAAAHgAAAACAAAABwAAANAAAAADAAAABAAAAOwAAAAFAAAABwAAABwBAAAGAAAA" +
        "AQAAAFQBAAAIAAAAAQAAAHQBAAABIAAABQAAAHwBAAADIAAABQAAAPwBAAABEAAAAQAAABQCAAAC" +
        "IAAAFgAAABoCAAAEIAAAAQAAAOcDAAAAIAAAAQAAAPYDAAADEAAAAQAAABAEAAAGIAAAAQAAABgE" +
        "AAAAEAAAAQAAADAEAAA=";

    private static final int ITERATIONS = 5;

    public static void main(String[] args) throws Throwable {
        verify($noinline$getJavaApiMethodHandle());
        verify($noinline$getConstMethodHandle());
    }

    private static void verify(MethodHandle mh) {
        int result = 0;
        for (int i = 0; i < ITERATIONS; ++i) {
            try {
                result += (int) mh.invokeWithArguments(List.of());
            } catch (Throwable t) {
                throw new RuntimeException(t);
            }
            Runtime.getRuntime().gc();
            System.runFinalization();
        }

        if (result != ITERATIONS) {
            throw new AssertionError();
        }
    }

    private static MethodHandle $noinline$getJavaApiMethodHandle() throws Throwable {
        ClassLoader loader = new InMemoryDexClassLoader(
            ByteBuffer.wrap(Base64.getDecoder().decode(DEX_FILE)),
            ClassLoader.getSystemClassLoader());
        Class<?> clazz = loader.loadClass("ClassWithAMethod");
        return MethodHandles.publicLookup().findStatic(clazz, "method", methodType(int.class));
    }

    private static MethodHandle $noinline$getConstMethodHandle() throws Throwable {
        ClassLoader loader = new InMemoryDexClassLoader(
            ByteBuffer.wrap(Base64.getDecoder().decode(DEX_FILE)),
            ClassLoader.getSystemClassLoader());
        Class<?> clazz = loader.loadClass("ClassWithAMethod");

        return (MethodHandle) clazz.getDeclaredMethod("constMethodHandle").invoke(null);
    }
}