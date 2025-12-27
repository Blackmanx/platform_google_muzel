/*
 * Copyright (C) 2024 The Android Open Source Project
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

package com.android.server.art;

import android.annotation.NonNull;
import android.annotation.Nullable;
import android.os.Build;
import android.os.ParcelFileDescriptor;
import android.system.ErrnoException;
import android.system.Os;
import android.util.Log;
import android.util.Slog;

import androidx.annotation.RequiresApi;

import java.io.InterruptedIOException;
import java.nio.ByteBuffer;
import java.nio.charset.StandardCharsets;

/**
 * A log wrapper that logs messages with the appropriate tag.
 *
 * @hide
 */
@RequiresApi(Build.VERSION_CODES.UPSIDE_DOWN_CAKE)
public class AsLog {
    private static final String TAG = "ArtService";
    private static final String PRE_REBOOT_TAG = "ArtServicePreReboot";

    @NonNull
    public static String getTag() {
        return GlobalInjector.getInstance().isPreReboot() ? PRE_REBOOT_TAG : TAG;
    }

    public static void v(@NonNull String msg) {
        Log.v(getTag(), msg);
    }

    public static void v(@NonNull String msg, @Nullable Throwable tr) {
        Log.v(getTag(), msg, tr);
    }

    public static void d(@NonNull String msg) {
        Log.d(getTag(), msg);
    }

    public static void d(@NonNull String msg, @Nullable Throwable tr) {
        Log.d(getTag(), msg, tr);
    }

    public static void i(@NonNull String msg) {
        Log.i(getTag(), msg);
    }

    public static void i(@NonNull String msg, @Nullable Throwable tr) {
        Log.i(getTag(), msg, tr);
    }

    public static void w(@NonNull String msg) {
        Log.w(getTag(), msg);
    }

    public static void w(@NonNull String msg, @Nullable Throwable tr) {
        Log.w(getTag(), msg, tr);
    }

    public static void e(@NonNull String msg) {
        Log.e(getTag(), msg);
    }

    public static void e(@NonNull String msg, @Nullable Throwable tr) {
        Log.e(getTag(), msg, tr);
    }

    public static void wtf(@NonNull String msg) {
        Slog.wtf(getTag(), msg);
    }

    public static void wtf(@NonNull String msg, @Nullable Throwable tr) {
        Slog.wtf(getTag(), msg, tr);
    }

    /**
     * A logger that redirects the logs to the given FD. If the FD is not set, it logs to logcat.
     */
    public static class Logger {
        private final @Nullable ParcelFileDescriptor mFd;

        public Logger(@Nullable ParcelFileDescriptor fd) {
            mFd = fd;
        }

        public void v(@NonNull String msg) {
            if (mFd != null) {
                write(msg, null /* tr */);
            } else {
                Log.v(getTag(), msg);
            }
        }

        public void v(@NonNull String msg, @Nullable Throwable tr) {
            if (mFd != null) {
                write(msg, tr);
            } else {
                Log.v(getTag(), msg, tr);
            }
        }

        public void d(@NonNull String msg) {
            if (mFd != null) {
                write(msg, null /* tr */);
            } else {
                Log.d(getTag(), msg);
            }
        }

        public void d(@NonNull String msg, @Nullable Throwable tr) {
            if (mFd != null) {
                write(msg, tr);
            } else {
                Log.d(getTag(), msg, tr);
            }
        }

        public void i(@NonNull String msg) {
            if (mFd != null) {
                write(msg, null /* tr */);
            } else {
                Log.i(getTag(), msg);
            }
        }

        public void i(@NonNull String msg, @Nullable Throwable tr) {
            if (mFd != null) {
                write(msg, tr);
            } else {
                Log.i(getTag(), msg, tr);
            }
        }

        public void w(@NonNull String msg) {
            if (mFd != null) {
                write(msg, null /* tr */);
            } else {
                Log.w(getTag(), msg);
            }
        }

        public void w(@NonNull String msg, @Nullable Throwable tr) {
            if (mFd != null) {
                write(msg, tr);
            } else {
                Log.w(getTag(), msg, tr);
            }
        }

        public void e(@NonNull String msg) {
            if (mFd != null) {
                write(msg, null /* tr */);
            } else {
                Log.e(getTag(), msg);
            }
        }

        public void e(@NonNull String msg, @Nullable Throwable tr) {
            if (mFd != null) {
                write(msg, tr);
            } else {
                Log.e(getTag(), msg, tr);
            }
        }

        // Intentionally omit `wtf` because wtf logs should go to `Slog.wtf` in order to be surfaced
        // on APC.

        private void write(@NonNull String msg, @Nullable Throwable tr) {
            try {
                Os.write(mFd.getFileDescriptor(),
                        ByteBuffer.wrap((msg + "\n").getBytes(StandardCharsets.UTF_8)));
                if (tr != null) {
                    String stackTrace = Log.getStackTraceString(tr);
                    Os.write(mFd.getFileDescriptor(),
                            ByteBuffer.wrap(stackTrace.getBytes(StandardCharsets.UTF_8)));
                }
            } catch (ErrnoException | InterruptedIOException e) {
                Log.e(getTag(),
                        "Failed to write log to %d: %s".formatted(mFd.getFd(), e.getMessage()));
                Log.e(getTag(), msg, tr);
            }
        }
    }
}
