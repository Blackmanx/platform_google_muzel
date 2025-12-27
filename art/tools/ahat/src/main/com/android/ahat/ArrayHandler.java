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

import com.android.ahat.heapdump.AhatInstance;
import com.android.ahat.heapdump.AhatSnapshot;

import java.io.IOException;
import java.io.OutputStream;

class ArrayHandler implements AhatDataHandler {
  private AhatSnapshot mSnapshot;

  public ArrayHandler(AhatSnapshot snapshot) {
    mSnapshot = snapshot;
  }

  @Override
  public void handle(Response response, Query query) throws IOException {
    long id = query.getLong("id", 0);
    AhatInstance inst = mSnapshot.findInstance(id);
    byte[] bytes = inst.asByteArray();

    if (bytes == null) {
      response.error("No byte[] found for the given request.");
      return;
    }

    OutputStream os =
        response.attachment("application/octet-stream", String.format("array-0x%08x.bin", id));
    os.write(bytes);
    os.close();
  }
}
