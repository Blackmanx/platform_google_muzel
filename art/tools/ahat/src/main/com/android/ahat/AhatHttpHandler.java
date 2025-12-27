/*
 * Copyright (C) 2015 The Android Open Source Project
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

import com.sun.net.httpserver.HttpExchange;
import com.sun.net.httpserver.HttpHandler;

import java.io.IOException;
import java.io.OutputStream;
import java.io.PrintStream;

/**
 * AhatHttpHandler.
 *
 * HttpHandler for AhatHandlers and AhatDataHandlers.
 */
class AhatHttpHandler implements HttpHandler {
  private AhatDataHandler mAhatDataHandler;

  public AhatHttpHandler(AhatHandler handler) {
    mAhatDataHandler = new DocHandler(handler);
  }

  public AhatHttpHandler(AhatDataHandler handler) {
    mAhatDataHandler = handler;
  }

  @Override
  public void handle(HttpExchange exchange) throws IOException {
    try {
      Response response = new Response(exchange);
      Query query = new Query(exchange.getRequestURI());
      mAhatDataHandler.handle(response, query);
    } catch (RuntimeException e) {
      // Print runtime exceptions to standard error for debugging purposes,
      // because otherwise they are swallowed and not reported.
      System.err.println("Exception when handling " + exchange.getRequestURI() + ": ");
      e.printStackTrace();
      throw e;
    }
  }

  /**
   * Implementation of Response interface for HttpHandler.
   */
  private static class Response implements AhatDataHandler.Response {
    private final HttpExchange mExchange;

    private Response(HttpExchange exchange) {
      mExchange = exchange;
    }

    @Override
    public void error(String message) throws IOException {
      mExchange.getResponseHeaders().add("Content-Type", "text/html");
      mExchange.sendResponseHeaders(404, 0);
      PrintStream ps = new PrintStream(mExchange.getResponseBody());
      HtmlDoc doc = new HtmlDoc(ps, DocString.text("ahat"), DocString.uri("style.css"));
      doc.big(DocString.text(message));
      doc.close();
      ps.close();
    }

    @Override
    public OutputStream content(String contentType) throws IOException {
      mExchange.getResponseHeaders().add("Content-Type", contentType);
      mExchange.sendResponseHeaders(200, 0);
      return mExchange.getResponseBody();
    }

    @Override
    public OutputStream attachment(String contentType, String filename) throws IOException {
      mExchange.getResponseHeaders().add("Content-Type", contentType);
      mExchange.getResponseHeaders().add(
          "Content-Disposition", "attachment; filename=\"" + filename + "\"");
      mExchange.sendResponseHeaders(200, 0);
      return mExchange.getResponseBody();
    }
  }

  /**
   * AhatDataHandler implementation of AhatHandler.
   */
  private static class DocHandler implements AhatDataHandler {
    private AhatHandler mAhatHandler;

    private DocHandler(AhatHandler handler) {
      mAhatHandler = handler;
    }

    @Override
    public void handle(Response response, Query query) throws IOException {
      OutputStream body = response.content("text/html;charset=utf-8");
      PrintStream ps = new PrintStream(body);
      HtmlDoc doc = new HtmlDoc(ps, DocString.text("ahat"), DocString.uri("style.css"));
      doc.menu(Menu.getMenu());
      mAhatHandler.handle(doc, query);
      doc.close();
      ps.close();
    }
  }
}
