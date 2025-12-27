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

import java.util.concurrent.atomic.AtomicInteger;
import java.util.concurrent.atomic.AtomicReference;
import java.util.concurrent.locks.ReentrantLock;

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

        verifyClassInitializerPinning();
    }

    private static final Thread.UncaughtExceptionHandler HANDLER = (t, e) -> {
        System.err.println("thread: " + t.getName());
        e.printStackTrace(System.err);
        System.exit(1);
    };

    private static class ParkingInInitializerClass {

        private static final String VALUE;

        static {
            try {
                Thread.sleep(10);
            } catch (InterruptedException e) {
                throw new RuntimeException(e);
            }
            VALUE = "42";
        }
        private ParkingInInitializerClass() {}
    }

    private static void verifyClassInitializerPinning() throws InterruptedException {
        AtomicReference<String> result = new AtomicReference<>();

        VirtualThread vt = (VirtualThread) Thread.ofVirtual()
                .uncaughtExceptionHandler(HANDLER)
                .unstarted(() -> result.set(ParkingInInitializerClass.VALUE));
        UnmountListener listener = new UnmountListener(vt);
        vt.setJvmtiEventListener(listener);
        vt.start();
        vt.join();
        if (!"42".equals(result.get())) {
            throw new AssertionError("Class is likely not initialized");
        }

        int state = listener.state.get();
        if (state != UnmountListener.STATE_UNMOUNTED) {
            throw new AssertionError("Expect Virtual Thread to be pinned: " + state);
        }
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
