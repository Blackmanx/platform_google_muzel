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

import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertNotNull;
import static org.junit.Assert.assertTrue;

import com.android.ahat.heapdump.AhatInstance;
import com.android.ahat.heapdump.AhatSnapshot;

import org.junit.Test;

import java.io.IOException;
import java.util.ArrayList;
import java.util.List;

public class StaticHandlerTest {
  @Test
  public void noCrashFound() throws IOException {
    AhatDataHandler handler = new StaticHandler("etc/style.css", "text/css");
    TestHandler.testNoCrash(handler, "http://localhost:7100/style.css");
  }

  @Test
  public void noCrashNotFound() throws IOException {
    AhatDataHandler handler = new StaticHandler("etc/noidonotexist.css", "text/css");
    TestHandler.testNoCrash(handler, "http://localhost:7100/noidonotexist.css");
  }
}
