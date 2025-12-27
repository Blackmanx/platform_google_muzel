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

package art;

import java.lang.reflect.Executable;
import java.lang.reflect.Method;
import java.util.Arrays;

public class Test2282 {
    static int LAST_LINE_NUMBER = -1;
    static int CATCH_LINE_NUMBER = -1;
    static Method TEST_METHOD;
    static Method CATCH_HANDLER;
    static Method SINGLE_STEP_HANDLER;

    public static void notifySingleStep(Thread thr, Executable e, long loc) {
        if (!e.equals(TEST_METHOD) || LAST_LINE_NUMBER != -1) {
            // Ignore single stepping from methods like enableSingleStepping. We are only interested
            // in the first line from the test method. So ignore others. This would make the test
            // more robust if the implementation changes.
            return;
        }

        int cur_line = Breakpoint.locationToLine(e, loc);
        System.out.println("Single step: " + e + " @ line=" + cur_line);
        LAST_LINE_NUMBER = cur_line;
        if (LAST_LINE_NUMBER == CATCH_LINE_NUMBER) {
            System.out.println("Single stepping was enabled in catch handler. We should not break "
                               + "on catch line");
        }
    }

    public static void ExceptionCatchHandler(
            Thread thr, Executable method, long location, Throwable exception) {
        CATCH_LINE_NUMBER = Breakpoint.locationToLine(method, location);
        System.out.println(thr.getName() + ": " + method + " @ line = " + CATCH_LINE_NUMBER
                + " caught " + exception.getClass() + ": " + exception.getMessage());
        Exceptions.disableExceptionTracing(Thread.currentThread());
        Trace.enableSingleStepTracing(Test2282.class, SINGLE_STEP_HANDLER, Thread.currentThread());
    }

    public static class TestException extends Error {
        public TestException(String e) {
            super(e);
        }
    }

    public static void run() throws Exception {
        CATCH_HANDLER = Test2282.class.getDeclaredMethod("ExceptionCatchHandler", Thread.class,
                Executable.class, Long.TYPE, Throwable.class);
        SINGLE_STEP_HANDLER = Test2282.class.getDeclaredMethod(
                "notifySingleStep", Thread.class, Executable.class, Long.TYPE);
        TEST_METHOD = Test2282.class.getDeclaredMethod("run");
        // Enable exception catch event and setup a handler for the events.
        Exceptions.setupExceptionTracing(Test2282.class, TestException.class,
                /*ExceptionCaughtEventHandler=*/null, CATCH_HANDLER);
        Exceptions.enableExceptionCatchEvent(Thread.currentThread());

        try {
            throw new TestException("Test");
        } catch (TestException e) {
            // Single step should stop here.
            Trace.disableTracing(Thread.currentThread());
            System.out.println("Caught TestException");
        }
    }
}
