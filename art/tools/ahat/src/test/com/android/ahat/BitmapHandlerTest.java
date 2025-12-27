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

package com.android.ahat;

import static org.junit.Assert.assertNotNull;

import com.android.ahat.heapdump.AhatInstance;

import org.junit.Test;

import java.io.IOException;

public class BitmapHandlerTest {
  @Test
  public void noCrashBitmap() throws IOException {
    TestDump dump = TestDump.getTestDump();

    AhatInstance object = dump.getDumpedAhatInstance("bitmapOne");
    assertNotNull(object);

    AhatDataHandler handler = new BitmapHandler(dump.getAhatSnapshot());
    TestHandler.testNoCrash(handler, "http://localhost:7100/bitmap?id=" + object.getId());
  }

  @Test
  public void noCrashNonBitmap() throws IOException {
    TestDump dump = TestDump.getTestDump();

    AhatInstance object = dump.getDumpedAhatInstance("anObject");
    assertNotNull(object);

    AhatDataHandler handler = new BitmapHandler(dump.getAhatSnapshot());
    TestHandler.testNoCrash(handler, "http://localhost:7100/bitmap?id=" + object.getId());
  }
}
