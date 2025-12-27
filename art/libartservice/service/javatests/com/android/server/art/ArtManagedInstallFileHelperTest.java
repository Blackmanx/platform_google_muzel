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
 * limitations under the License
 */

package com.android.server.art;

import static com.android.server.art.model.ValidationResult.FILENAME_MISMATCH;
import static com.android.server.art.model.ValidationResult.INVALID_SDM_BAD_APK_SIGNATURE;
import static com.android.server.art.model.ValidationResult.INVALID_SDM_BAD_SDM_SIGNATURE;
import static com.android.server.art.model.ValidationResult.INVALID_SDM_INVALID_ISA;
import static com.android.server.art.model.ValidationResult.INVALID_SDM_SIGNATURE_MISMATCH;
import static com.android.server.art.model.ValidationResult.RESULT_ACCEPTED;
import static com.android.server.art.model.ValidationResult.RESULT_SHOULD_DELETE_AND_CONTINUE;
import static com.android.server.art.model.ValidationResult.RESULT_UNRECOGNIZED;
import static com.android.server.art.model.ValidationResult.UNRECOGNIZED_PATH;

import static com.google.common.truth.Truth.assertThat;

import static org.junit.Assert.assertThrows;
import static org.mockito.Mockito.any;
import static org.mockito.Mockito.anyInt;
import static org.mockito.Mockito.eq;
import static org.mockito.Mockito.lenient;
import static org.mockito.Mockito.when;

import android.content.pm.SigningInfo;
import android.content.pm.SigningInfoException;

import androidx.test.filters.SmallTest;
import androidx.test.runner.AndroidJUnit4;

import com.android.server.art.model.ValidationResult;
import com.android.server.art.testing.StaticMockitoRule;
import com.android.server.art.testing.TestingUtils;

import org.junit.Before;
import org.junit.Rule;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.mockito.Mock;

import java.util.List;

@SmallTest
@RunWith(AndroidJUnit4.class)
public class ArtManagedInstallFileHelperTest {
    @Rule public StaticMockitoRule mockitoRule = new StaticMockitoRule(Constants.class);

    @Mock private ArtManagedInstallFileHelper.Injector mInjector;
    @Mock private SigningInfo mSigningInfoA;
    @Mock private SigningInfo mSigningInfoB;
    @Mock private SigningInfoException mSigningInfoException;

    @Before
    public void setUp() throws Exception {
        lenient().when(Constants.getNative64BitAbi()).thenReturn("arm64-v8a");
        lenient().when(Constants.getNative32BitAbi()).thenReturn("armeabi-v7a");

        lenient().when(mSigningInfoA.signersMatchExactly(mSigningInfoA)).thenReturn(true);
        lenient().when(mSigningInfoA.signersMatchExactly(mSigningInfoB)).thenReturn(false);
        lenient().when(mSigningInfoB.signersMatchExactly(mSigningInfoB)).thenReturn(true);
        lenient().when(mSigningInfoB.signersMatchExactly(mSigningInfoA)).thenReturn(false);

        ArtManagedInstallFileHelper.sInjector = mInjector;
    }

    @Test
    public void testIsArtManaged() throws Exception {
        assertThat(ArtManagedInstallFileHelper.isArtManaged("/foo/bar.dm")).isTrue();
        assertThat(ArtManagedInstallFileHelper.isArtManaged("/foo/bar.prof")).isTrue();
        assertThat(ArtManagedInstallFileHelper.isArtManaged("/foo/bar.apk.prof")).isTrue();
        assertThat(ArtManagedInstallFileHelper.isArtManaged("/foo/bar.sdm")).isTrue();
        assertThat(ArtManagedInstallFileHelper.isArtManaged("/foo/bar.arm.sdm")).isTrue();
        assertThat(ArtManagedInstallFileHelper.isArtManaged("/foo/bar.arm64.sdm")).isTrue();
        assertThat(ArtManagedInstallFileHelper.isArtManaged("/foo/bar.abc")).isFalse();
    }

    @Test
    public void testFilterPathsForApk() throws Exception {
        assertThat(ArtManagedInstallFileHelper.filterPathsForApk(
                           List.of("/foo/bar.dm", "/foo/bar.prof", "/foo/bar.apk.prof",
                                   "/foo/bar.sdm", "/foo/bar.x86_64.sdm", "/foo/bar.arm.sdm",
                                   "/foo/bar.arm64.sdm", "/foo/bar.abc", "/foo/baz.dm"),
                           "/foo/bar.apk"))
                .containsExactly("/foo/bar.dm", "/foo/bar.apk.prof", "/foo/bar.arm.sdm",
                        "/foo/bar.arm64.sdm");

        // Filenames don't match.
        assertThat(ArtManagedInstallFileHelper.filterPathsForApk(
                           List.of("/foo/bar.dm", "/foo/bar.prof", "/foo/bar.apk.prof",
                                   "/foo/bar.arm64.sdm", "/foo/bar.abc", "/foo/baz.dm"),
                           "/foo/qux.apk"))
                .isEmpty();

        // Directories don't match.
        assertThat(ArtManagedInstallFileHelper.filterPathsForApk(
                           List.of("/foo/bar.dm", "/foo/bar.prof", "/foo/bar.apk.prof",
                                   "/foo/bar.arm64.sdm", "/foo/bar.abc", "/foo/baz.dm"),
                           "/quz/bar.apk"))
                .isEmpty();
    }

    @Test
    public void testGetTargetPathForApk() throws Exception {
        assertThat(ArtManagedInstallFileHelper.getTargetPathForApk(
                           "/foo/bar.dm", "/somewhere/base.apk"))
                .isEqualTo("/somewhere/base.dm");
        assertThat(ArtManagedInstallFileHelper.getTargetPathForApk(
                           "/foo/bar.prof", "/somewhere/base.apk"))
                .isEqualTo("/somewhere/base.apk.prof");
        assertThat(ArtManagedInstallFileHelper.getTargetPathForApk(
                           "/foo/bar.arm.sdm", "/somewhere/base.apk"))
                .isEqualTo("/somewhere/base.arm.sdm");
        assertThat(ArtManagedInstallFileHelper.getTargetPathForApk(
                           "/foo/bar.arm64.sdm", "/somewhere/base.apk"))
                .isEqualTo("/somewhere/base.arm64.sdm");

        // None or invalid ISA.
        assertThat(ArtManagedInstallFileHelper.getTargetPathForApk(
                           "/foo/bar.sdm", "/somewhere/base.apk"))
                .isEqualTo("/somewhere/bar.sdm");
        assertThat(ArtManagedInstallFileHelper.getTargetPathForApk(
                           "/foo/bar.x86_64.sdm", "/somewhere/base.apk"))
                .isEqualTo("/somewhere/bar.x86_64.sdm");
        assertThat(ArtManagedInstallFileHelper.getTargetPathForApk(
                           "/foo/bar.invalid-isa.sdm", "/somewhere/base.apk"))
                .isEqualTo("/somewhere/bar.invalid-isa.sdm");

        assertThrows(IllegalArgumentException.class, () -> {
            ArtManagedInstallFileHelper.getTargetPathForApk("/foo/bar.abc", "/somewhere/base.apk");
        });
    }

    @Test
    public void testValidateFilesOk() throws Exception {
        when(mInjector.exists(eq("/somewhere/app/bar/base.apk"))).thenReturn(true);

        lenient()
                .when(mInjector.getVerifiedSigningInfo(eq("/somewhere/app/bar/base.apk"), anyInt()))
                .thenReturn(mSigningInfoA);

        lenient()
                .when(mInjector.getVerifiedSigningInfo(
                        eq("/somewhere/app/bar/base.arm64.sdm"), anyInt()))
                .thenReturn(mSigningInfoA);

        assertThat(
                ArtManagedInstallFileHelper.validateFiles(List.of("/somewhere/app/bar/base.dm",
                        "/somewhere/app/bar/base.apk.prof", "/somewhere/app/bar/base.arm64.sdm")))
                .comparingElementsUsing(TestingUtils.<ValidationResult>deepEquality())
                .containsExactly(
                        new ValidationResult("/somewhere/app/bar/base.dm", RESULT_ACCEPTED),
                        new ValidationResult("/somewhere/app/bar/base.apk.prof", RESULT_ACCEPTED),
                        new ValidationResult("/somewhere/app/bar/base.arm64.sdm", RESULT_ACCEPTED));
    }

    @Test
    public void testValidateFilesUnrecognizedPath() throws Exception {
        assertThat(ArtManagedInstallFileHelper.validateFiles(
                           List.of("/somewhere/app/bar/base.apk", "/somewhere/app/bar/base.bogus")))
                .comparingElementsUsing(TestingUtils.<ValidationResult>deepEquality())
                .containsExactly(new ValidationResult("/somewhere/app/bar/base.apk",
                                         RESULT_UNRECOGNIZED, UNRECOGNIZED_PATH,
                                         "Path '/somewhere/app/bar/base.apk' does not represent an "
                                                 + "ART-managed install file"),
                        new ValidationResult("/somewhere/app/bar/base.bogus", RESULT_UNRECOGNIZED,
                                UNRECOGNIZED_PATH,
                                "Path '/somewhere/app/bar/base.bogus' does not represent an "
                                        + "ART-managed install file"));
    }

    @Test
    public void testValidateFilesFilenameMismatch() throws Exception {
        lenient().when(mInjector.exists(any())).thenReturn(false);
        lenient().when(mInjector.exists(eq("/somewhere/app/bar/base.apk"))).thenReturn(true);

        assertThat(ArtManagedInstallFileHelper.validateFiles(List.of("/somewhere/app/bar/bogus.dm",
                           "/somewhere/app/bar/base.prof", "/somewhere/app/bar/bogus.apk.prof",
                           "/somewhere/app/bar/bogus.arm64.sdm")))
                .comparingElementsUsing(TestingUtils.<ValidationResult>deepEquality())
                .containsExactly(new ValidationResult("/somewhere/app/bar/bogus.dm",
                                         RESULT_SHOULD_DELETE_AND_CONTINUE, FILENAME_MISMATCH,
                                         "DM filename 'bogus.dm' does not correspond to any APK"),
                        new ValidationResult("/somewhere/app/bar/base.prof",
                                RESULT_SHOULD_DELETE_AND_CONTINUE, FILENAME_MISMATCH,
                                "Profile filename 'base.prof' does not correspond to any APK"),
                        new ValidationResult("/somewhere/app/bar/bogus.apk.prof",
                                RESULT_SHOULD_DELETE_AND_CONTINUE, FILENAME_MISMATCH,
                                "Profile filename 'bogus.apk.prof' does not correspond to any APK"),
                        new ValidationResult("/somewhere/app/bar/bogus.arm64.sdm",
                                RESULT_SHOULD_DELETE_AND_CONTINUE, FILENAME_MISMATCH,
                                "SDM filename 'bogus.arm64.sdm' does not correspond to any APK"));
    }

    @Test
    public void testValidateFilesInvalidSdmInvalidIsa() throws Exception {
        assertThat(ArtManagedInstallFileHelper.validateFiles(
                           List.of("/somewhere/app/bar/base.x86_64.sdm")))
                .comparingElementsUsing(TestingUtils.<ValidationResult>deepEquality())
                .containsExactly(new ValidationResult("/somewhere/app/bar/base.x86_64.sdm",
                        RESULT_SHOULD_DELETE_AND_CONTINUE, INVALID_SDM_INVALID_ISA,
                        "Missing or invalid instruction set name in SDM filename "
                                + "'base.x86_64.sdm'"));
    }

    @Test
    public void testValidateFilesInvalidSdmBadApkSignature() throws Exception {
        when(mSigningInfoException.getMessage()).thenReturn("some message");

        lenient().when(mInjector.exists(eq("/somewhere/app/bar/base.apk"))).thenReturn(true);

        lenient()
                .when(mInjector.getVerifiedSigningInfo(eq("/somewhere/app/bar/base.apk"), anyInt()))
                .thenThrow(mSigningInfoException);

        assertThat(ArtManagedInstallFileHelper.validateFiles(
                           List.of("/somewhere/app/bar/base.arm64.sdm")))
                .comparingElementsUsing(TestingUtils.<ValidationResult>deepEquality())
                .containsExactly(new ValidationResult("/somewhere/app/bar/base.arm64.sdm",
                        RESULT_SHOULD_DELETE_AND_CONTINUE, INVALID_SDM_BAD_APK_SIGNATURE,
                        "Failed to verify APK signatures for 'base.apk': some message"));
    }

    @Test
    public void testValidateFilesInvalidSdmBadSdmSignature() throws Exception {
        when(mSigningInfoException.getMessage()).thenReturn("some message");

        lenient().when(mInjector.exists(eq("/somewhere/app/bar/base.apk"))).thenReturn(true);

        lenient()
                .when(mInjector.getVerifiedSigningInfo(eq("/somewhere/app/bar/base.apk"), anyInt()))
                .thenReturn(mSigningInfoA);

        lenient()
                .when(mInjector.getVerifiedSigningInfo(
                        eq("/somewhere/app/bar/base.arm64.sdm"), anyInt()))
                .thenThrow(mSigningInfoException);

        assertThat(ArtManagedInstallFileHelper.validateFiles(
                           List.of("/somewhere/app/bar/base.arm64.sdm")))
                .comparingElementsUsing(TestingUtils.<ValidationResult>deepEquality())
                .containsExactly(new ValidationResult("/somewhere/app/bar/base.arm64.sdm",
                        RESULT_SHOULD_DELETE_AND_CONTINUE, INVALID_SDM_BAD_SDM_SIGNATURE,
                        "Failed to verify SDM signatures for 'base.arm64.sdm': some message"));
    }

    @Test
    public void testValidateFilesInvalidSdmSignatureMismatch() throws Exception {
        lenient().when(mInjector.exists(eq("/somewhere/app/bar/base.apk"))).thenReturn(true);

        lenient()
                .when(mInjector.getVerifiedSigningInfo(eq("/somewhere/app/bar/base.apk"), anyInt()))
                .thenReturn(mSigningInfoA);

        lenient()
                .when(mInjector.getVerifiedSigningInfo(
                        eq("/somewhere/app/bar/base.arm64.sdm"), anyInt()))
                .thenReturn(mSigningInfoB);

        assertThat(ArtManagedInstallFileHelper.validateFiles(
                           List.of("/somewhere/app/bar/base.arm64.sdm")))
                .comparingElementsUsing(TestingUtils.<ValidationResult>deepEquality())
                .containsExactly(new ValidationResult("/somewhere/app/bar/base.arm64.sdm",
                        RESULT_SHOULD_DELETE_AND_CONTINUE, INVALID_SDM_SIGNATURE_MISMATCH,
                        "SDM signatures are inconsistent with APK (SDM filename: 'base.arm64.sdm', "
                                + "APK filename: 'base.apk')"));
    }
}
