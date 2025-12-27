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

import java.util.concurrent.TimeUnit;
import java.util.concurrent.atomic.AtomicBoolean;
import java.util.concurrent.locks.ReentrantLock;

import jdk.internal.access.SharedSecrets;

/**
 * Verify that {@link ReentrantLock} API running on Virtual Thread.
 */
public class Main {

    public static void main(String[] args) throws InterruptedException, ClassNotFoundException {
        if (!com.android.art.flags.Flags.virtualThreadImplV1()) {
            return;
        }
        // Exit if the thread throws any exception.
        Thread.setDefaultUncaughtExceptionHandler(HANDLER);

        verifyCurrentThreadApi();
        verifyReentrantLock();
    }

    private static final Thread.UncaughtExceptionHandler HANDLER = (t, e) -> {
        System.err.println("thread: " + t.getName());
        e.printStackTrace(System.err);
        System.exit(1);
    };

    private static void verifyReentrantLock() throws InterruptedException {
        ReentrantLock lock = new ReentrantLock();
        AtomicBoolean v = new AtomicBoolean(false);
        long timeoutThresholdNs = TimeUnit.SECONDS.toNanos(5);
        lock.lock();
        Thread vt = createDefaultVirtualThreadBuilder().start(() -> {
            lock.lock();
            v.set(true);
            lock.unlock();
        });
        long startTime = System.nanoTime();
        while (System.nanoTime() - startTime < timeoutThresholdNs) {
            if (vt.getVirtualThreadContext().isUnmounted()) {
                break;
            }
        }

        assertEquals(true, vt.getVirtualThreadContext().isUnmounted());
        assertEquals(false, v.get());

        lock.unlock();
        vt.join();

        assertEquals(true, v.get());
    }


    static void assertEquals(boolean expected, boolean value) {
        if (expected == value) {
            return;
        }
        throw new AssertionError("assertEquals expected: " + expected
                + ", value: " + value);
    }

    private static void verifyCurrentThreadApi() throws InterruptedException {
        Thread vt = createDefaultVirtualThreadBuilder().start(() -> {
            String threadClassName = Thread.currentThread().getClass().getName();
            if (!("java.lang.VirtualThread".equals(threadClassName))) {
                throw new AssertionError("Expect a VirtualThread instance");
            }
            if (Thread.currentThread().equals(getCarrierThread())) {
                throw new AssertionError("Thread.currentThread() shouldn't "
                        + "return a carrier thread.");
            }
        });
        vt.join();
    }

    private static Thread getCarrierThread() {
        return SharedSecrets.getJavaLangAccess().currentCarrierThread();
    }

    private static Thread.Builder.OfVirtual createDefaultVirtualThreadBuilder() {
        return Thread.ofVirtual().uncaughtExceptionHandler(HANDLER);
    }
}
