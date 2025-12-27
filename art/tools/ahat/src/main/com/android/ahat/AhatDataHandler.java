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

import java.io.IOException;
import java.io.OutputStream;

/**
 * AhatDataHandler.
 *
 * Interface for an ahat data handler.
 */
interface AhatDataHandler {
  /**
   * Interface for sending one of three kinds of responses.
   * Exactly one of these methods should be called per response.
   */
  public interface Response {
    /**
     * Send an error response.
     */
    void error(String message) throws IOException;

    /**
     * Send a successful response with given content type. The content should
     * be written to the returned output stream.
     */
    OutputStream content(String contentType) throws IOException;

    /**
     * Send a successful response for download as an attachment with the given
     * name for the attachment. The content should be written to the returned
     * output stream.
     */
    OutputStream attachment(String contentType, String filename) throws IOException;
  }

  /**
   * Handle the given query, write out to the given response object.
   */
  void handle(Response response, Query query) throws IOException;
}
