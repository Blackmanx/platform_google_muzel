/*
 * Copyright (C) 2022 The Android Open Source Project
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

package libcore.java.lang;

import org.junit.Test;
import org.junit.runner.RunWith;
import org.junit.runners.JUnit4;

import java.io.IOException;
import java.io.InputStream;
import java.lang.StackWalker.Option;
import java.lang.StackWalker.StackFrame;
import java.lang.reflect.InvocationHandler;
import java.lang.reflect.Method;
import java.lang.reflect.Proxy;
import java.nio.ByteBuffer;
import java.util.Arrays;
import java.util.List;
import java.util.stream.Collectors;

import libcore.io.Streams;

import dalvik.system.InMemoryDexClassLoader;

import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertNotEquals;
import static org.junit.Assert.assertNotNull;
import static org.junit.Assert.assertSame;
import static org.junit.Assert.assertTrue;
import static org.junit.Assert.fail;

import static java.util.stream.Collectors.toList;

@RunWith(JUnit4.class)
public class StackWalkerTest {

    @Test
    public void testGetByteCodeIndex() throws Exception {
        ClassLoader classLoader = loadPrebuiltDexClassLoader();
        // See source at libcore/luni/src/test/dex_src/libcore/java/lang/stackwalker/Test1.java
        Class test1Class = classLoader.loadClass("libcore.java.lang.stackwalker.Test1");
        List<StackFrame> stackFrames = (List<StackFrame>) test1Class.getMethod("test")
                .invoke(null);
        assertTrue(stackFrames.size() >= 3);

        StackFrame f = stackFrames.get(0);
        assertEquals(test1Class.getName(), f.getClassName());
        assertEquals("invokeWalker", f.getMethodName());
        assertEquals(32, f.getLineNumber());
        // The getByteCodeIndex() result can be checked against
        // `dexdump -d libcore/luni/src/test/resources/prebuilt-dex-from-java.dex`
        assertEquals(9, f.getByteCodeIndex());

        f = stackFrames.get(1);
        assertEquals(test1Class.getName(), f.getClassName());
        assertEquals("test", f.getMethodName());
        assertEquals(26, f.getLineNumber());
        assertEquals(0, f.getByteCodeIndex());

        f = stackFrames.get(2);
        assertEquals(StackWalkerTest.class.getName(), f.getClassName());
        assertEquals("testGetByteCodeIndex", f.getMethodName());
    }


    private static ClassLoader loadPrebuiltDexClassLoader() throws IOException {
        // See "core-tests-prebuilt-dex-from-java" in JavaLibrary.bp to re-generate this dex file.
        try (InputStream is = ThreadTest.class.getClassLoader()
                .getResourceAsStream("prebuilt-dex-from-java.dex")) {
            byte[] data = Streams.readFullyNoClose(is);
            return new InMemoryDexClassLoader(ByteBuffer.wrap(data),
                    ThreadTest.class.getClassLoader());
        }
    }

    @Test
    public void testOptionValueOf() {
        assertSame(Option.RETAIN_CLASS_REFERENCE, Option.valueOf("RETAIN_CLASS_REFERENCE"));
        assertSame(Option.SHOW_REFLECT_FRAMES, Option.valueOf("SHOW_REFLECT_FRAMES"));
        assertSame(Option.SHOW_HIDDEN_FRAMES, Option.valueOf("SHOW_HIDDEN_FRAMES"));
    }

    @Test
    public void testOptionValues() {
        Option[] options = Option.values();
        Option[] expected = new Option[] {
                Option.RETAIN_CLASS_REFERENCE,
                Option.SHOW_REFLECT_FRAMES,
                Option.SHOW_HIDDEN_FRAMES,
        };
        for (Option e : expected) {
            assertHasOption(options, e);
        }
        assertEquals(expected.length, options.length);
    }

    private void assertHasOption(Option[] options, Option expected) {
        for (Option option : options) {
            if (option == expected) {
                return;
            }
        }
        fail("fail to find " + expected + " in " + Arrays.toString(options));
    }

    public interface ProxiedInterface {
        void run();
    }

    // regression test for b/404180956
    @Test
    public void testProxy_withTestInterface() {
        String[][] expectedFrame = new String[][] {
                new String[] {MyInvocationHandler.class.getName(), "invoke"},
                // The new RI doesn't have this java.lang.reflect.Proxy frame.
                new String[] {"java.lang.reflect.Proxy", "invoke"},
                // Ensure that the interface name isn't returned.
                new String[] {"~" + ProxiedInterface.class.getName(), "run"},
                new String[] {StackWalkerTest.class.getName(), "testProxy_withTestInterface"},

        };
        InvocationHandler handler = new MyInvocationHandler(expectedFrame);

        ProxiedInterface proxy = (ProxiedInterface) Proxy.newProxyInstance(
                ProxiedInterface.class.getClassLoader(),
                new Class[] {ProxiedInterface.class}, handler);
        proxy.run();
    }

    @Test
    public void testProxy_withRunnable() {
        String[][] expectedFrame = new String[][] {
                new String[] {MyInvocationHandler.class.getName(), "invoke"},
                // The new RI doesn't have this java.lang.reflect.Proxy frame.
                new String[] {"java.lang.reflect.Proxy", "invoke"},
                // Ensure that the interface name isn't returned.
                new String[] {"~" + Runnable.class.getName(), "run"},
                new String[] {StackWalkerTest.class.getName(), "testProxy_withRunnable"},

        };
        InvocationHandler handler = new MyInvocationHandler(expectedFrame);

        Runnable proxy = (Runnable) Proxy.newProxyInstance(Runnable.class.getClassLoader(),
                new Class[] {Runnable.class}, handler);
        proxy.run();
    }

    private static class MyInvocationHandler implements InvocationHandler {

        private final String[][] expectedTopClassAndMethodNames;

        public MyInvocationHandler(String[][] expectedTopClassAndMethodNames) {
            this.expectedTopClassAndMethodNames = expectedTopClassAndMethodNames;
        }

        @Override
        public Object invoke(Object proxy, Method method, Object[] args) throws Throwable {
            if (!"run".equals(method.getName())) {
                throw new AssertionError("Only run() method can be invoked.");
            }

            // The bottom frames are not verified, but reading all frames ensures not crashing.
            List<StackFrame> frames = StackWalker.getInstance().walk(
                    stackFrameStream -> stackFrameStream.collect(toList()));

            for (int i = 0; i < expectedTopClassAndMethodNames.length; i++) {
                String[] classAndMethodName = expectedTopClassAndMethodNames[i];
                assertTrue("stack size should be larger than " + i, frames.size() > i);
                StackFrame frame = frames.get(i);
                assertNotNull("The frame is null at index " + i, frame);
                String expectedClassName = classAndMethodName[0];
                if (expectedClassName.startsWith("~")) {
                    String unexpectedClassName = expectedClassName.substring(1);
                    assertNotEquals(unexpectedClassName, frame.getClassName());
                } else {
                    assertEquals(expectedClassName, frame.getClassName());
                }
                assertEquals(classAndMethodName[1], frame.getMethodName());
            }
            return null;
        }
    }

}