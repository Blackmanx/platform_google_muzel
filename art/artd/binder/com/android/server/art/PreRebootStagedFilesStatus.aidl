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

package com.android.server.art;

/** @hide */
parcelable PreRebootStagedFilesStatus {
    /**
     * The staged files were created for the same platform build and APEXes as what the device
     * currently has.
     */
    boolean isCommittable;
    /**
     * A string describing why the staged files are not committable. Only applicable if
     * {@code isCommittable} is false.
     */
    @utf8InCpp String reason;
    /** When the staged files were created, in milliseconds. */
    long createdAtMillis;
}
