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

package com.android.server.art.model;

import android.annotation.FlaggedApi;
import android.annotation.IntDef;
import android.annotation.NonNull;
import android.annotation.Nullable;
import android.annotation.SystemApi;

import com.android.art.flags.Flags;
import com.android.internal.annotations.VisibleForTesting;
import com.android.server.art.ArtManagedInstallFileHelper;

import java.lang.annotation.Retention;
import java.lang.annotation.RetentionPolicy;

/**
 * The result of {@link ArtManagedInstallFileHelper#validateFiles}.
 *
 * @hide
 */
@SystemApi(client = SystemApi.Client.SYSTEM_SERVER)
@FlaggedApi(Flags.FLAG_ART_MANAGED_INSTALL_FILES_VALIDATION_API)
public class ValidationResult {
    /** The file is accepted and should be installed. */
    @FlaggedApi(Flags.FLAG_ART_MANAGED_INSTALL_FILES_VALIDATION_API)
    public static final int RESULT_ACCEPTED = 0;

    /**
     * The specified <b>path</b> does not represent an <i>ART-managed install file</i>.
     *
     * This result is determined by a pure string operation on the path, without performing any file
     * I/O. It indicates that the caller has passed the wrong argument, and it should not occur
     * during normal usage.
     */
    @FlaggedApi(Flags.FLAG_ART_MANAGED_INSTALL_FILES_VALIDATION_API)
    public static final int RESULT_UNRECOGNIZED = 1;

    /**
     * The file is denied, but it shouldn’t fail the installation. Package Manager should delete
     * this file and continue.
     */
    @FlaggedApi(Flags.FLAG_ART_MANAGED_INSTALL_FILES_VALIDATION_API)
    public static final int RESULT_SHOULD_DELETE_AND_CONTINUE = 2;

    /** The file is denied and should fail the installation. */
    @FlaggedApi(Flags.FLAG_ART_MANAGED_INSTALL_FILES_VALIDATION_API)
    public static final int RESULT_SHOULD_FAIL = 3;

    /** @hide */
    // clang-format off
    @IntDef(prefix = "RESULT_", value = {
        RESULT_ACCEPTED,
        RESULT_UNRECOGNIZED,
        RESULT_SHOULD_DELETE_AND_CONTINUE,
        RESULT_SHOULD_FAIL,
    })
    // clang-format on
    @Retention(RetentionPolicy.SOURCE)
    public @interface ValidationResultCode {}

    /** @hide */
    @VisibleForTesting public static final String UNRECOGNIZED_PATH = "UNRECOGNIZED_PATH";

    /** @hide */
    @VisibleForTesting public static final String FILENAME_MISMATCH = "FILENAME_MISMATCH";

    /** @hide */
    @VisibleForTesting
    public static final String INVALID_SDM_INVALID_ISA = "INVALID_SDM_INVALID_ISA";

    /** @hide */
    @VisibleForTesting
    public static final String INVALID_SDM_BAD_APK_SIGNATURE = "INVALID_SDM_BAD_APK_SIGNATURE";

    /** @hide */
    @VisibleForTesting
    public static final String INVALID_SDM_BAD_SDM_SIGNATURE = "INVALID_SDM_BAD_SDM_SIGNATURE";

    /** @hide */
    @VisibleForTesting
    public static final String INVALID_SDM_SIGNATURE_MISMATCH = "INVALID_SDM_SIGNATURE_MISMATCH";

    private final @NonNull String mPath;
    private final @ValidationResultCode int mCode;
    private final @Nullable String mMessage;

    /** @hide */
    @VisibleForTesting
    public ValidationResult(@NonNull String path, @ValidationResultCode int code) {
        this(path, code, null /* errorCode */, null /* message */);
    }

    /** @hide */
    @VisibleForTesting
    public ValidationResult(@NonNull String path, @ValidationResultCode int code,
            @Nullable String errorCode, @Nullable String message) {
        mPath = path;
        mCode = code;
        mMessage = errorCode != null ? String.format("[%s] %s", errorCode, message) : null;
    }

    /** The path to the ART-managed install file. */
    @FlaggedApi(Flags.FLAG_ART_MANAGED_INSTALL_FILES_VALIDATION_API)
    public @NonNull String getPath() {
        return mPath;
    }

    /** The action that Package Manager should perform. */
    @FlaggedApi(Flags.FLAG_ART_MANAGED_INSTALL_FILES_VALIDATION_API)
    public @ValidationResultCode int getCode() {
        return mCode;
    }

    /**
     * A string indicating the reason of this result. Not applicable if {@link #getCode()} is
     * {@link #RESULT_ACCEPTED}.
     *
     * @return a string in the format of "[CODE] message", where CODE is a machine-readable
     *         string representing the error code.
     */
    @FlaggedApi(Flags.FLAG_ART_MANAGED_INSTALL_FILES_VALIDATION_API)
    public @Nullable String getMessage() {
        return mMessage;
    }
}
