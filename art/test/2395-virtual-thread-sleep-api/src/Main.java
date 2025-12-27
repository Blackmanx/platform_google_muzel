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

import java.lang.invoke.MethodHandle;
import java.lang.invoke.MethodHandles;
import java.lang.invoke.MethodType;
import java.lang.reflect.Method;
import java.util.ArrayList;
import java.util.List;
import java.util.concurrent.atomic.AtomicInteger;

/**
 * Verify that {@link Thread#sleep(long)} API parks virtual threads.
 */
public class Main {

    public static void main(String[] args) throws InterruptedException {
        if (!com.android.art.flags.Flags.virtualThreadImplV1()) {
            return;
        }
        // Exit if the thread throws any exception.
        Thread.setDefaultUncaughtExceptionHandler(HANDLER);

        testSleep(2);
        testSleep(10);
        testSleep(100);
        testSleep(300);
        testSleepThreadPinning();

        testSleepViaReflection(1);
        testSleepViaReflection(10);
        testSleepViaMethodHandle(1, /*isInvokeExact*/ false);
        testSleepViaMethodHandle(10, /*isInvokeExact*/ false);
        testSleepViaMethodHandle(1, /*isInvokeExact*/ true);
        testSleepViaMethodHandle(10, /*isInvokeExact*/ true);
    }

    private static final Thread.UncaughtExceptionHandler HANDLER = (t, e) -> {
        System.err.println("thread: " + t.getName());
        e.printStackTrace(System.err);
        System.exit(1);
    };

    private static final long SLEEP_DURATION_MS = 10;

    /**
     * Start {@code numOfThreads} virtual threads sleeping for the given duration and wait until
     * all threads join or time out.
     * @param numOfThreads number of concurrent virtual threads sleeping
     */
    private static void testSleep(int numOfThreads)
            throws InterruptedException {
        Runnable task = () -> {
            try {
                Thread.sleep(SLEEP_DURATION_MS);
            } catch (InterruptedException e) {
                throw new RuntimeException(e);
            }
        };
        testSleeping(task, numOfThreads, UnmountListener.STATE_PARKED_AND_UNMOUNTED);
    }

    private static void testSleeping(Runnable runnable, int numOfThreads,
            int expectedUnmountState) throws InterruptedException {
        List<VirtualThread> threads = new ArrayList<>(numOfThreads);
        for (int i = 0; i < numOfThreads; i++) {
            VirtualThread vt = (VirtualThread) Thread.ofVirtual()
                    .uncaughtExceptionHandler(HANDLER)
                    .unstarted(runnable);
            vt.setJvmtiEventListener(new UnmountListener(vt));
            threads.add(vt);
            vt.start();
        }

        for (VirtualThread vt : threads) {
            vt.join();
        }

        for (VirtualThread vt : threads) {
            UnmountListener listener = (UnmountListener) vt.getJvmtiEventListener();
            int state = listener.state.get();
            if (state != expectedUnmountState) {
                throw new AssertionError("Expected state code " + expectedUnmountState
                        + ", but the state code was " + state);
            }
        }
    }

    private static void testSleepThreadPinning() throws InterruptedException {
        Runnable task = () -> {
            try {
                Thread t = Thread.currentThread();
                synchronized (t) {
                    Thread.sleep(SLEEP_DURATION_MS);
                }
            } catch (InterruptedException e) {
                throw new RuntimeException(e);
            }
        };
        testSleeping(task, 1, UnmountListener.STATE_UNMOUNTED);
    }

    private static void testSleepViaReflection(int numOfThreads) throws InterruptedException {
        Runnable task = () -> {
            try {
                Method m = Thread.class.getMethod("sleep", long.class);
                m.invoke(null, SLEEP_DURATION_MS);
            } catch (ReflectiveOperationException e) {
                throw new RuntimeException(e);
            }
        };

        // TODO: Unpin virtual thread from reflection frame.
        // When the virtual thread terminates, it's unmounted from the carrier thread.
        testSleeping(task, numOfThreads, UnmountListener.STATE_UNMOUNTED);
    }

    private static final MethodHandle METHOD_HANDLE_SLEEP;
    private static final MethodHandle METHOD_HANDLE_SLEEP_PLUS_ONE;

    static {
        try {
            METHOD_HANDLE_SLEEP = MethodHandles.lookup()
                    .findStatic(Thread.class, "sleep", MethodType.methodType(void.class, long.class));
            MethodHandle plusOne = MethodHandles.lookup()
                    .findStatic(Main.class, "plusOne", MethodType.methodType(long.class, long.class));
            METHOD_HANDLE_SLEEP_PLUS_ONE = MethodHandles.filterArguments(
                    METHOD_HANDLE_SLEEP, 0, plusOne);
        } catch (NoSuchMethodException | IllegalAccessException e) {
            throw new RuntimeException(e);
        }
    }

    private static long plusOne(long l) {
        return l + 1;
    }

    private static void testSleepViaMethodHandle(int numOfThreads, boolean isInvokeExact)
            throws InterruptedException {
        final Runnable taskSleep;
        final Runnable taskCombinator;
        if (isInvokeExact) {
            class InvokeExactTask implements Runnable {
                private final MethodHandle m;

                InvokeExactTask(MethodHandle m) {
                    this.m = m;
                }

                @Override
                public void run() {
                    try {
                        m.invokeExact(SLEEP_DURATION_MS);
                    } catch (Throwable e) {
                        throw new RuntimeException(e);
                    }
                }
            }
            taskSleep = new InvokeExactTask(METHOD_HANDLE_SLEEP); // Test Thread.sleep()
            taskCombinator = new InvokeExactTask(METHOD_HANDLE_SLEEP_PLUS_ONE); // Test combinator.
        } else {
            class InvokeTask implements Runnable {
                private final MethodHandle m;

                InvokeTask(MethodHandle m) {
                    this.m = m;
                }

                @Override
                public void run() {
                    try {
                        m.invokeExact(SLEEP_DURATION_MS);
                    } catch (Throwable e) {
                        throw new RuntimeException(e);
                    }
                }
            }
            taskSleep = new InvokeTask(METHOD_HANDLE_SLEEP); // Test Thread.sleep()
            taskCombinator = new InvokeTask(METHOD_HANDLE_SLEEP_PLUS_ONE); // Test combinator.
        }

        testSleeping(taskSleep, numOfThreads, UnmountListener.STATE_PARKED_AND_UNMOUNTED);
        // TODO: investigate why the thread is pinned.
        testSleeping(taskCombinator, numOfThreads, UnmountListener.STATE_UNMOUNTED);
    }


    private static class UnmountListener implements VirtualThread.JvmtiEventsListener {

        private static final int STATE_NOT_UNMOUNTED = 0;
        // Likely when the virtual thread terminates.
        private static final int STATE_UNMOUNTED = 1;
        // When the virtual thread is parked, not pinned.
        private static final int STATE_PARKED_AND_UNMOUNTED = 2;
        private final VirtualThread vt;
        // Indicate if the virtual thread is ever parked and unmounted.
        private final AtomicInteger state = new AtomicInteger(STATE_NOT_UNMOUNTED);

        public UnmountListener(VirtualThread vt) {
            this.vt = vt;
        }

        @Override
        public void onJvmtiUnmount(boolean hide) {
            if (!hide && state.get() != STATE_PARKED_AND_UNMOUNTED) {
                if (vt.getVirtualThreadContext().isUnmounted()) {
                    state.set(STATE_PARKED_AND_UNMOUNTED);
                } else {
                    state.set(STATE_UNMOUNTED);
                }
            }
        }
    }
}
