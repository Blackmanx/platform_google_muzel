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

package android.art;

import android.os.Handler;
import android.os.HandlerThread;

import org.junit.BeforeClass;
import org.junit.Test;

public class HiddenApiSdk28AppTest {
    private static final String TAG = "HiddenApiSdk28AppTest";

    private static class NativeRunnable implements Runnable {
        @Override public native void run();
    }

    @BeforeClass
    public static void setUp() {
        System.loadLibrary("hidden_api_app_test_jni");
    }

    // Test getting a field and a method from JNI (see jni.cc) that would be blocked from the
    // platform domain but are allowed in an app with targetSdkVersion 28. Use a Handler to call
    // into the native NativeRunnable.run, to make the topmost Java frame look like the caller is
    // from android.os.Handler in the platform domain (which hiddenapi shouldn't pick up on).
    @Test
    public void testAppApiCallWithoutJavaFrames() throws Exception {
        HandlerThread handlerThread = new HandlerThread(TAG);
        handlerThread.start();
        Handler handler = new Handler(handlerThread.getLooper());
        NativeRunnable nativeRunnable = new NativeRunnable();
        handler.post(nativeRunnable);
        handlerThread.quitSafely();
        handlerThread.join();
    }
}
