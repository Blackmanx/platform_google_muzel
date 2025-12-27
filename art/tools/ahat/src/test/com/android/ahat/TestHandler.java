/*
 * Copyright (C) 2016 The Android Open Source Project
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
import java.io.PrintStream;

/**
 * Provide common utilities for basic handler tests.
 */
public class TestHandler {
  /**
   * Test that the given handler doesn't crash on the given query.
   */
  public static void testNoCrash(AhatHandler handler, String uri) throws IOException {
    PrintStream ps = new PrintStream(OutputStream.nullOutputStream());
    HtmlDoc doc = new HtmlDoc(ps, DocString.text("noCrash test"), DocString.uri("style.css"));
    Query query = new Query(DocString.uri(uri));
    handler.handle(doc, query);
  }

  /**
   * Test that the given data handler doesn't crash on the given query.
   */
  public static void testNoCrash(AhatDataHandler handler, String uri) throws IOException {
    Response response = new Response();
    Query query = new Query(DocString.uri(uri));
    handler.handle(response, query);
  }

  /**
   * Response implementation for test purposes.
   */
  private static class Response implements AhatDataHandler.Response {
    @Override
    public void error(String message) throws IOException {}

    @Override
    public OutputStream content(String contentType) throws IOException {
      return OutputStream.nullOutputStream();
    }

    @Override
    public OutputStream attachment(String contentType, String filename) throws IOException {
      return OutputStream.nullOutputStream();
    }
  }
}
