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

package com.android.server.art.testing;

import static org.mockito.Mockito.mock;

import android.os.Binder;
import android.os.ParcelFileDescriptor;

import com.android.modules.utils.BasicShellCommandHandler;
import com.android.server.art.Utils;

import java.io.BufferedReader;
import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.io.InputStreamReader;
import java.io.PrintWriter;
import java.util.concurrent.CompletableFuture;

/** A harness to test a {@link BasicShellCommandHandler}. */
public class CommandExecution implements AutoCloseable {
    private ParcelFileDescriptor[] mStdinPipe = null;
    private ParcelFileDescriptor[] mStdoutPipe = null;
    private ParcelFileDescriptor[] mStderrPipe = null;
    private PrintWriter mStdinWriter = null;
    private BufferedReader mStdoutReader = null;
    private BufferedReader mStderrReader = null;
    private CompletableFuture<Integer> mFuture = null;

    public CommandExecution(BasicShellCommandHandler commandHandler, String... args)
            throws Exception {
        try {
            mStdinPipe = ParcelFileDescriptor.createPipe();
            mStdoutPipe = ParcelFileDescriptor.createPipe();
            mStderrPipe = ParcelFileDescriptor.createPipe();
            mStdinWriter = new PrintWriter(new FileOutputStream(mStdinPipe[1].getFileDescriptor()));
            mStdoutReader = new BufferedReader(
                    new InputStreamReader(new FileInputStream(mStdoutPipe[0].getFileDescriptor())));
            mStderrReader = new BufferedReader(
                    new InputStreamReader(new FileInputStream(mStderrPipe[0].getFileDescriptor())));
            mFuture = CompletableFuture.supplyAsync(() -> exec(commandHandler, args));
        } catch (Exception e) {
            close();
            throw e;
        }
    }

    private int exec(BasicShellCommandHandler commandHandler, String... args) {
        int exitCode = commandHandler.exec(mock(Binder.class), mStdinPipe[0].getFileDescriptor(),
                mStdoutPipe[1].getFileDescriptor(), mStderrPipe[1].getFileDescriptor(), args);
        try {
            mStdinPipe[0].close();
            mStdoutPipe[1].close();
            mStderrPipe[1].close();
        } catch (Exception e) {
            throw new RuntimeException(e);
        }
        return exitCode;
    }

    public int waitAndGetExitCode() {
        return Utils.getFuture(mFuture);
    }

    public PrintWriter getStdin() {
        return mStdinWriter;
    }

    public void closeStdin() throws Exception {
        mStdinWriter.close();
        mStdinPipe[1].close();
    }

    public BufferedReader getStdout() {
        return mStdoutReader;
    }

    public BufferedReader getStderr() {
        return mStderrReader;
    }

    @Override
    public void close() throws Exception {
        if (mStdinWriter != null) {
            mStdinWriter.close();
        }
        if (mStdoutReader != null) {
            mStdoutReader.close();
        }
        if (mStderrReader != null) {
            mStderrReader.close();
        }
        // Note that we still need to close the FDs after closing the streams. See the
        // Android-specific warning at
        // https://developer.android.com/reference/java/io/FileInputStream#FileInputStream(java.io.FileDescriptor)
        if (mStdinPipe != null) {
            mStdinPipe[0].close();
            mStdinPipe[1].close();
        }
        if (mStdoutPipe != null) {
            mStdoutPipe[0].close();
            mStdoutPipe[1].close();
        }
        if (mStderrPipe != null) {
            mStderrPipe[0].close();
            mStderrPipe[1].close();
        }
    }
}
