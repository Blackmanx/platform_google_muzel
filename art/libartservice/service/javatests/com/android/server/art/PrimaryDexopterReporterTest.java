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

import static com.android.dx.mockito.inline.extended.ExtendedMockito.verify;

import static org.junit.Assert.assertThrows;
import static org.mockito.ArgumentMatchers.any;
import static org.mockito.ArgumentMatchers.anyInt;
import static org.mockito.ArgumentMatchers.anyString;
import static org.mockito.ArgumentMatchers.eq;
import static org.mockito.Mockito.lenient;
import static org.mockito.Mockito.times;

import android.os.RemoteException;
import android.os.ServiceSpecificException;

import com.android.server.art.model.ArtFlags;
import com.android.server.art.model.DexMetadata;
import com.android.server.art.model.DexoptParams;
import com.android.server.art.testing.TestingUtils;

import org.junit.Before;
import org.junit.Test;
import org.mockito.ArgumentMatchers;

import java.nio.file.NoSuchFileException;
import java.util.function.Supplier;

public final class PrimaryDexopterReporterTest extends PrimaryDexopterTestBase {
    private static final String COMPILER_FILTER_SPEED_PROFILE = "speed-profile";
    private static final int COMPILER_FILTER_VERIFY_VALUE =
            ArtStatsLog.ART_DEX2_OAT_REPORTED__COMPILER_FILTER__ART_COMPILATION_FILTER_VERIFY;
    private static final int COMPILER_FILTER_NOOP_VALUE =
            ArtStatsLog.ART_DEX2_OAT_REPORTED__COMPILER_FILTER__ART_COMPILATION_FILTER_SKIP;
    private static final String COMPILER_REASON_INSTALL = "install";
    private static final int COMPILER_REASON_INSTALL_VALUE =
            ArtStatsLog
                    .ART_DATUM_DELTA_REPORTED__COMPILATION_REASON__ART_COMPILATION_REASON_INSTALL;
    private static final int ISA_ARM = ArtStatsLog.ART_DEX2_OAT_REPORTED__ISA__ART_ISA_ARM;
    private static final int ISA_ARM64 = ArtStatsLog.ART_DEX2_OAT_REPORTED__ISA__ART_ISA_ARM64;
    private static final long DEX2OAT_SIZE_BYTES_1 = 567L;
    private static final long DEX2OAT_SIZE_BYTES_2 = 678L;
    private static final long DEX2OAT_WALL_TIME_MS_1 = 234L;
    private static final long DEX2OAT_WALL_TIME_MS_2 = 345L;
    private static final DexoptParams DEXOPT_PARAMS_SKIP =
            new DexoptParams.Builder(COMPILER_REASON_INSTALL)
                    .setCompilerFilter(DexoptParams.COMPILER_FILTER_NOOP)
                    .setFlags(ArtFlags.FLAG_FOR_PRIMARY_DEX)
                    .build();
    private static final DexoptParams DEXOPT_PARAMS_SPEED_PROFILE =
            new DexoptParams.Builder(COMPILER_REASON_INSTALL)
                    .setCompilerFilter(COMPILER_FILTER_SPEED_PROFILE)
                    .setFlags(ArtFlags.FLAG_FOR_PRIMARY_DEX)
                    .build();
    private static final DexoptParams DEXOPT_PARAMS_ONLY_BASE_SPLIT =
            new DexoptParams.Builder(COMPILER_REASON_INSTALL)
                    .setCompilerFilter(COMPILER_FILTER_SPEED_PROFILE)
                    .setFlags(ArtFlags.FLAG_FOR_PRIMARY_DEX | ArtFlags.FLAG_FOR_SINGLE_SPLIT)
                    .setSplitName(null)
                    .build();
    public static final Supplier<String> ARM_ISA_MATCHER = () -> eq("arm");
    public static final Supplier<String> ARM64_ISA_MATCHER = () -> eq("arm64");

    private PrimaryDexopter mPrimaryDexopter;

    @Before
    public void setUp() throws Exception {
        super.setUp();

        lenient().when(mInjector.getReporterExecutor()).thenReturn(Runnable::run);

        // By default, none of the profiles are usable.
        lenient().when(mArtd.isProfileUsable(any(), anyString())).thenReturn(false);
        lenient()
                .when(mArtd.copyAndRewriteProfile(any(), any(), anyString()))
                .thenReturn(TestingUtils.createCopyAndRewriteProfileNoProfile());
        lenient()
                .when(mArtd.copyAndRewriteEmbeddedProfile(any(), anyString()))
                .thenReturn(TestingUtils.createCopyAndRewriteProfileNoProfile());

        // By default, no DM file exists.
        lenient()
                .when(mDexMetadataHelperInjector.openZipFile(anyString()))
                .thenThrow(NoSuchFileException.class);

        // Dexopt is by default needed and successful.
        lenient()
                .when(mArtd.getDexoptNeeded(
                        anyString(), anyString(), anyString(), anyString(), anyInt(), any()))
                .thenReturn(dexoptIsNeeded());

        mockPrimaryDexopter(DEXOPT_PARAMS_SPEED_PROFILE);
    }

    @Test
    public void testDex2OatResult_Success() throws Exception {
        mockArtDexoptReturns(ARM64_ISA_MATCHER,
                createArtdDexoptResult(false /* cancelled */, DEX2OAT_WALL_TIME_MS_1,
                        DEX2OAT_WALL_TIME_MS_1 + 10, DEX2OAT_SIZE_BYTES_1,
                        DEX2OAT_SIZE_BYTES_1 - 10));
        mockArtDexoptReturns(ARM_ISA_MATCHER,
                createArtdDexoptResult(false /* cancelled */, DEX2OAT_WALL_TIME_MS_2,
                        DEX2OAT_WALL_TIME_MS_2 + 20, DEX2OAT_SIZE_BYTES_2,
                        DEX2OAT_SIZE_BYTES_2 - 20));

        mPrimaryDexopter.dexopt();

        Dex2OatStatsReporter.Dex2OatResult expectedResult =
                Dex2OatStatsReporter.Dex2OatResult.exited(0);
        verifyNumReports(4);
        verifyReport(COMPILER_FILTER_VERIFY_VALUE, ISA_ARM64, expectedResult, DEX2OAT_SIZE_BYTES_1,
                DEX2OAT_WALL_TIME_MS_1, 2 /* numReports */);
        verifyReport(COMPILER_FILTER_VERIFY_VALUE, ISA_ARM, expectedResult, DEX2OAT_SIZE_BYTES_2,
                DEX2OAT_WALL_TIME_MS_2, 2 /* numReports */);
    }

    @Test
    public void testDex2OatResult_ExitedWithNonZeroCode() throws Exception {
        int status = 1, exitCode = 2, signal = 0;
        mockArtdDexoptResultFailure("dex2oat exited with non-zero code", status, exitCode, signal);

        mPrimaryDexopter.dexopt();

        Dex2OatStatsReporter.Dex2OatResult expectedResult =
                new Dex2OatStatsReporter.Dex2OatResult(status, exitCode, signal);
        verifyNumReports(4);
        verifyReport(COMPILER_FILTER_VERIFY_VALUE, ISA_ARM64, expectedResult, 0L /* sizeBytes */,
                0L /* wallTimeMs */, 2 /* numReports */);
        verifyReport(COMPILER_FILTER_VERIFY_VALUE, ISA_ARM, expectedResult, 0L /* sizeBytes */,
                0L /* wallTimeMs */, 2 /* numReports */);
    }

    @Test
    public void testDex2OatResult_Signaled() throws Exception {
        int status = 2, exitCode = -1, signal = 4;
        mockArtdDexoptResultFailure("dex2oat signaled", status, exitCode, signal);

        mPrimaryDexopter.dexopt();

        Dex2OatStatsReporter.Dex2OatResult expectedResult =
                new Dex2OatStatsReporter.Dex2OatResult(status, exitCode, signal);
        verifyNumReports(4);
        verifyReport(COMPILER_FILTER_VERIFY_VALUE, ISA_ARM64, expectedResult, 0L /* sizeBytes */,
                0L /* wallTimeMs */, 2 /* numReports */);
        verifyReport(COMPILER_FILTER_VERIFY_VALUE, ISA_ARM, expectedResult, 0L /* sizeBytes */,
                0L /* wallTimeMs */, 2 /* numReports */);
    }

    @Test
    public void testDex2OatResult_FailedToStart_ServiceSpecificException() throws Exception {
        mockArtdDexoptThrows(ArgumentMatchers::anyString,
                new ServiceSpecificException(-1, "Not your typical dex2oat error message"));

        mPrimaryDexopter.dexopt();

        Dex2OatStatsReporter.Dex2OatResult failedToStart =
                Dex2OatStatsReporter.Dex2OatResult.failedToStart();
        verifyNumReports(4);
        verifyReport(COMPILER_FILTER_VERIFY_VALUE, ISA_ARM64, failedToStart, 0L /* sizeBytes */,
                0L /* wallTimeMs */, 2 /* numReports */);
        verifyReport(COMPILER_FILTER_VERIFY_VALUE, ISA_ARM, failedToStart, 0L /* sizeBytes */,
                0L /* wallTimeMs */, 2 /* numReports */);
    }

    @Test
    public void testDex2OatResult_FailedToStart_RemoteException() throws Exception {
        Exception expectedException = new RemoteException();
        mockArtdDexoptThrows(ArgumentMatchers::anyString, expectedException);

        assertThrows(expectedException.getClass(), () -> mPrimaryDexopter.dexopt());

        Dex2OatStatsReporter.Dex2OatResult failedToStart =
                Dex2OatStatsReporter.Dex2OatResult.failedToStart();
        verifyNumReports(2);
        verifyReport(COMPILER_FILTER_VERIFY_VALUE, ISA_ARM64, failedToStart, 0L /* sizeBytes */,
                0L /* wallTimeMs */, 1 /* numReports */);
        verifyReport(COMPILER_FILTER_VERIFY_VALUE, ISA_ARM, failedToStart, 0L /* sizeBytes */,
                0L /* wallTimeMs */, 1 /* numReports */);
    }

    @Test
    public void testDex2OatResult_FailedToStart_RuntimeException() throws Exception {
        mockArtDexoptReturns(ARM64_ISA_MATCHER,
                createArtdDexoptResult(false /* cancelled */, DEX2OAT_WALL_TIME_MS_1,
                        DEX2OAT_WALL_TIME_MS_1 + 10, DEX2OAT_SIZE_BYTES_1,
                        DEX2OAT_SIZE_BYTES_1 - 10));

        Exception expectedException = new RuntimeException();
        mockArtdDexoptThrows(ARM_ISA_MATCHER, expectedException);

        assertThrows(expectedException.getClass(), () -> mPrimaryDexopter.dexopt());

        verifyNumReports(2);
        verifyReport(COMPILER_FILTER_VERIFY_VALUE, ISA_ARM64,
                Dex2OatStatsReporter.Dex2OatResult.exited(0), DEX2OAT_SIZE_BYTES_1,
                DEX2OAT_WALL_TIME_MS_1, 1 /* numReports */);
        verifyReport(COMPILER_FILTER_VERIFY_VALUE, ISA_ARM,
                Dex2OatStatsReporter.Dex2OatResult.failedToStart(), 0L /* sizeBytes */,
                0L /* wallTimeMs */, 1 /* numReports */);
    }

    @Test
    public void testDex2OatResult_NotRun() throws Exception {
        mockPrimaryDexopter(DEXOPT_PARAMS_SKIP);

        mPrimaryDexopter.dexopt();

        Dex2OatStatsReporter.Dex2OatResult notRun = Dex2OatStatsReporter.Dex2OatResult.notRun();
        verifyNumReports(4);
        verifyReport(COMPILER_FILTER_NOOP_VALUE, ISA_ARM64, notRun, 0L /* sizeBytes */,
                0L /* wallTimeMs */, 2 /* numReports */);
        verifyReport(COMPILER_FILTER_NOOP_VALUE, ISA_ARM, notRun, 0L /* sizeBytes */,
                0L /* wallTimeMs */, 2 /* numReports */);
    }

    @Test
    public void testDex2OatResult_Cancelled() throws Exception {
        mockArtDexoptReturns(
                ArgumentMatchers::anyString, createArtdDexoptResult(true /* cancelled */));

        mPrimaryDexopter.dexopt();

        Dex2OatStatsReporter.Dex2OatResult cancelled =
                Dex2OatStatsReporter.Dex2OatResult.cancelled();
        verifyNumReports(2);
        verifyReport(COMPILER_FILTER_VERIFY_VALUE, ISA_ARM64, cancelled, 0L /* sizeBytes */,
                0L /* wallTimeMs */, 1 /* numReports */);
        verifyReport(COMPILER_FILTER_VERIFY_VALUE, ISA_ARM, cancelled, 0L /* sizeBytes */,
                0L /* wallTimeMs */, 1 /* numReports */);
    }

    @Test
    public void testDex2OatResult_OnlyBaseSplit() throws Exception {
        mockArtDexoptReturns(ArgumentMatchers::anyString,
                createArtdDexoptResult(false /* cancelled */, DEX2OAT_WALL_TIME_MS_1,
                        DEX2OAT_WALL_TIME_MS_1 + 10, DEX2OAT_SIZE_BYTES_1,
                        DEX2OAT_SIZE_BYTES_1 - 10));
        mockPrimaryDexopter(DEXOPT_PARAMS_ONLY_BASE_SPLIT);

        mPrimaryDexopter.dexopt();

        // Artd is going to dexopt only the base split, so we will have 2 reports (one for each ISA)
        // instead of 4 as we have in testDex2OatResult_Success.
        verifyNumReports(2);
        verifyReport(COMPILER_FILTER_VERIFY_VALUE, ISA_ARM64,
                Dex2OatStatsReporter.Dex2OatResult.exited(0), DEX2OAT_SIZE_BYTES_1,
                DEX2OAT_WALL_TIME_MS_1, 1 /* numReports */);
        verifyReport(COMPILER_FILTER_VERIFY_VALUE, ISA_ARM,
                Dex2OatStatsReporter.Dex2OatResult.exited(0), DEX2OAT_SIZE_BYTES_1,
                DEX2OAT_WALL_TIME_MS_1, 1 /* numReports */);
    }

    private void mockPrimaryDexopter(DexoptParams params) {
        mPrimaryDexopter = new PrimaryDexopter(
                mInjector, mSnapshot, mPkgState, mPkg, params, mCancellationSignal);
    }

    private void mockArtDexoptReturns(
            Supplier<String> isaMatcher, ArtdDexoptResult artdDexoptResult) throws RemoteException {
        lenient()
                .when(mArtd.dexopt(any(), anyString(), isaMatcher.get(), anyString(), anyString(),
                        any(), any(), any(), anyInt(), any(), any(), any()))
                .thenReturn(artdDexoptResult);
    }

    private void mockArtdDexoptResultFailure(String message, int status, int exitCode, int signal)
            throws Exception {
        mockArtdDexoptThrows(ArgumentMatchers::anyString,
                new ServiceSpecificException(-1,
                        String.format(
                                "Failed to run dex2oat: %s [status=%d,exit_code=%d,signal=%d]",
                                message, status, exitCode, signal)));
    }

    private void mockArtdDexoptThrows(Supplier<String> isaMatcher, Exception exception)
            throws RemoteException {
        lenient()
                .when(mArtd.dexopt(any(), anyString(), isaMatcher.get(), anyString(), anyString(),
                        any(), any(), any(), anyInt(), any(), any(), any()))
                .thenThrow(exception);
    }

    private void verifyReport(int compilerFilter, int isa,
            Dex2OatStatsReporter.Dex2OatResult expectedResult, long sizeBytes, long wallTimeMs,
            int numReports) {
        verify(() -> {
            ArtStatsLog.write(eq(ArtStatsLog.ART_DEX2OAT_REPORTED), eq(UID), eq(compilerFilter),
                    eq(COMPILER_REASON_INSTALL_VALUE), eq(DexMetadata.TYPE_NONE), anyInt(), eq(isa),
                    eq(expectedResult.status()), eq(expectedResult.exitCode()),
                    eq(expectedResult.signal()), eq((int) sizeBytes / 1024), eq((int) wallTimeMs));
        }, times(numReports));
    }

    private void verifyNumReports(int numReports) {
        verify(() -> {
            ArtStatsLog.write(eq(ArtStatsLog.ART_DEX2OAT_REPORTED), anyInt(), anyInt(), anyInt(),
                    anyInt(), anyInt(), anyInt(), anyInt(), anyInt(), anyInt(), anyInt(), anyInt());
        }, times(numReports));
    }
}
