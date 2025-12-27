/*
 * Copyright (C) 2022 The Android Open Source Project
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

import static com.android.server.art.ArtManagerLocal.DexoptDoneCallback;
import static com.android.server.art.model.DexoptResult.DexContainerFileDexoptResult;
import static com.android.server.art.model.DexoptResult.DexoptResultStatus;
import static com.android.server.art.model.DexoptResult.PackageDexoptResult;
import static com.android.server.art.testing.TestDataHelper.newLibrary;
import static com.android.server.art.testing.TestDataHelper.newPackageState;
import static com.android.server.art.testing.TestDataHelper.newSplit;

import static com.google.common.truth.Truth.assertThat;

import static org.junit.Assert.assertThrows;
import static org.mockito.Mockito.any;
import static org.mockito.Mockito.eq;
import static org.mockito.Mockito.inOrder;
import static org.mockito.Mockito.lenient;
import static org.mockito.Mockito.mock;
import static org.mockito.Mockito.never;
import static org.mockito.Mockito.same;
import static org.mockito.Mockito.times;
import static org.mockito.Mockito.verify;
import static org.mockito.Mockito.when;

import android.apphibernation.AppHibernationManager;
import android.os.CancellationSignal;

import androidx.test.filters.SmallTest;
import androidx.test.runner.AndroidJUnit4;

import com.android.modules.utils.pm.PackageStateModulesUtils;
import com.android.server.art.model.ArtFlags;
import com.android.server.art.model.Config;
import com.android.server.art.model.DexoptParams;
import com.android.server.art.model.DexoptResult;
import com.android.server.art.model.OperationProgress;
import com.android.server.art.testing.StaticMockitoRule;
import com.android.server.art.testing.TestDataHelper.PackageStateBuilder;
import com.android.server.pm.PackageManagerLocal;
import com.android.server.pm.pkg.AndroidPackage;
import com.android.server.pm.pkg.AndroidPackageSplit;
import com.android.server.pm.pkg.PackageState;
import com.android.server.pm.pkg.SharedLibrary;

import org.junit.After;
import org.junit.Before;
import org.junit.Rule;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.mockito.InOrder;
import org.mockito.Mock;

import java.util.ArrayList;
import java.util.List;
import java.util.concurrent.Executor;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;
import java.util.concurrent.ForkJoinPool;
import java.util.concurrent.Future;
import java.util.concurrent.Semaphore;
import java.util.concurrent.TimeUnit;
import java.util.concurrent.atomic.AtomicBoolean;
import java.util.function.Consumer;
import java.util.stream.Stream;

@SmallTest
@RunWith(AndroidJUnit4.class)
public class DexoptHelperTest {
    private static final String PKG_NAME_FOO = "com.example.foo";
    private static final String PKG_NAME_BAR = "com.example.bar";
    private static final String PKG_NAME_LIB1 = "com.example.lib1";
    private static final String PKG_NAME_LIB2 = "com.example.lib2";
    private static final String PKG_NAME_LIB3 = "com.example.lib3";
    private static final String PKG_NAME_LIB4 = "com.example.lib4";
    private static final String PKG_NAME_LIBBAZ = "com.example.libbaz";
    private static final String PKG_NAME_SDK = "com.example.sdk";

    @Rule
    public StaticMockitoRule mockitoRule = new StaticMockitoRule(PackageStateModulesUtils.class);

    @Mock private DexoptHelper.Injector mInjector;
    @Mock private PrimaryDexopter mPrimaryDexopter;
    @Mock private SecondaryDexopter mSecondaryDexopter;
    @Mock private AppHibernationManager mAhm;
    @Mock private PackageManagerLocal.FilteredSnapshot mSnapshot;
    private PackageState mPkgStateFoo;
    private PackageState mPkgStateBar;
    private PackageState mPkgStateLib1;
    private PackageState mPkgStateLib2;
    private PackageState mPkgStateLib4;
    private PackageState mPkgStateLibbaz;
    private AndroidPackage mPkgFoo;
    private AndroidPackage mPkgBar;
    private AndroidPackage mPkgLib1;
    private AndroidPackage mPkgLib2;
    private AndroidPackage mPkgLib4;
    private AndroidPackage mPkgLibbaz;
    private CancellationSignal mCancellationSignal;
    private ExecutorService mExecutor;
    private List<DexContainerFileDexoptResult> mPrimaryResults;
    private List<DexContainerFileDexoptResult> mSecondaryResults;
    private Config mConfig;
    private DexoptParams mParams;
    private List<String> mRequestedPackages;
    private DexoptHelper mDexoptHelper;

    @Before
    public void setUp() throws Exception {
        lenient().when(mAhm.isHibernatingGlobally(any())).thenReturn(false);
        lenient().when(mAhm.isOatArtifactDeletionEnabled()).thenReturn(true);

        mCancellationSignal = new CancellationSignal();
        mExecutor = Executors.newSingleThreadExecutor();
        mConfig = new Config();

        preparePackagesAndLibraries();

        mPrimaryResults = createResults("/somewhere/app/foo/base.apk",
                DexoptResult.DEXOPT_PERFORMED /* status1 */,
                DexoptResult.DEXOPT_PERFORMED /* status2 */);
        mSecondaryResults = createResults("/data/user_de/0/foo/foo.apk",
                DexoptResult.DEXOPT_PERFORMED /* status1 */,
                DexoptResult.DEXOPT_PERFORMED /* status2 */);

        lenient()
                .when(mInjector.getPrimaryDexopter(any(), any(), any(), any(), any()))
                .thenReturn(mPrimaryDexopter);
        lenient().when(mPrimaryDexopter.dexopt()).thenReturn(mPrimaryResults);

        lenient()
                .when(mInjector.getSecondaryDexopter(any(), any(), any(), any()))
                .thenReturn(mSecondaryDexopter);
        lenient().when(mSecondaryDexopter.dexopt()).thenReturn(mSecondaryResults);

        mParams = new DexoptParams.Builder("install")
                          .setCompilerFilter("speed-profile")
                          .setFlags(ArtFlags.FLAG_FOR_SECONDARY_DEX
                                          | ArtFlags.FLAG_SHOULD_INCLUDE_DEPENDENCIES,
                                  ArtFlags.FLAG_FOR_SECONDARY_DEX
                                          | ArtFlags.FLAG_SHOULD_INCLUDE_DEPENDENCIES)
                          .build();

        lenient().when(mInjector.getAppHibernationManager()).thenReturn(mAhm);
        lenient().when(mInjector.getConfig()).thenReturn(mConfig);

        mDexoptHelper = new DexoptHelper(mInjector);
    }

    @After
    public void tearDown() {
        mExecutor.shutdown();
    }

    @Test
    public void testDexopt() throws Exception {
        // Only package libbaz fails.
        var failingPrimaryDexopter = mock(PrimaryDexopter.class);
        List<DexContainerFileDexoptResult> partialFailureResults = createResults(
                "/somewhere/app/foo/base.apk", DexoptResult.DEXOPT_PERFORMED /* status1 */,
                DexoptResult.DEXOPT_FAILED /* status2 */);
        lenient().when(failingPrimaryDexopter.dexopt()).thenReturn(partialFailureResults);
        when(mInjector.getPrimaryDexopter(any(), same(mPkgStateLibbaz), any(), any(), any()))
                .thenReturn(failingPrimaryDexopter);

        DexoptResult result = mDexoptHelper.dexopt(
                mSnapshot, mRequestedPackages, mParams, mCancellationSignal, mExecutor);

        assertThat(result.getRequestedCompilerFilter()).isEqualTo("speed-profile");
        assertThat(result.getReason()).isEqualTo("install");
        assertThat(result.getFinalStatus()).isEqualTo(DexoptResult.DEXOPT_FAILED);

        // The requested packages must come first.
        assertThat(result.getPackageDexoptResults()).hasSize(6);
        checkPackageResult(result, 0 /* index */, PKG_NAME_FOO, DexoptResult.DEXOPT_PERFORMED,
                List.of(mPrimaryResults, mSecondaryResults));
        checkPackageResult(result, 1 /* index */, PKG_NAME_BAR, DexoptResult.DEXOPT_PERFORMED,
                List.of(mPrimaryResults, mSecondaryResults));
        checkPackageResult(result, 2 /* index */, PKG_NAME_LIBBAZ, DexoptResult.DEXOPT_FAILED,
                List.of(partialFailureResults, mSecondaryResults));
        checkPackageResult(result, 3 /* index */, PKG_NAME_LIB1, DexoptResult.DEXOPT_PERFORMED,
                List.of(mPrimaryResults, mSecondaryResults));
        checkPackageResult(result, 4 /* index */, PKG_NAME_LIB2, DexoptResult.DEXOPT_PERFORMED,
                List.of(mPrimaryResults, mSecondaryResults));
        checkPackageResult(result, 5 /* index */, PKG_NAME_LIB4, DexoptResult.DEXOPT_PERFORMED,
                List.of(mPrimaryResults, mSecondaryResults));

        // The order matters. When running in a single thread, it should dexopt primary dex files
        // and the secondary dex files together for each package, and it should dexopt requested
        // packages, in the given order, and then dexopt dependencies.
        InOrder inOrder = inOrder(mInjector);
        inOrder.verify(mInjector).getPrimaryDexopter(
                any(), same(mPkgStateFoo), same(mPkgFoo), same(mParams), any());
        inOrder.verify(mInjector).getSecondaryDexopter(
                same(mPkgStateFoo), same(mPkgFoo), same(mParams), any());
        inOrder.verify(mInjector).getPrimaryDexopter(
                any(), same(mPkgStateBar), same(mPkgBar), same(mParams), any());
        inOrder.verify(mInjector).getSecondaryDexopter(
                same(mPkgStateBar), same(mPkgBar), same(mParams), any());
        inOrder.verify(mInjector).getPrimaryDexopter(
                any(), same(mPkgStateLibbaz), same(mPkgLibbaz), same(mParams), any());
        inOrder.verify(mInjector).getSecondaryDexopter(
                same(mPkgStateLibbaz), same(mPkgLibbaz), same(mParams), any());
        inOrder.verify(mInjector).getPrimaryDexopter(
                any(), same(mPkgStateLib1), same(mPkgLib1), same(mParams), any());
        inOrder.verify(mInjector).getSecondaryDexopter(
                same(mPkgStateLib1), same(mPkgLib1), same(mParams), any());
        inOrder.verify(mInjector).getPrimaryDexopter(
                any(), same(mPkgStateLib2), same(mPkgLib2), same(mParams), any());
        inOrder.verify(mInjector).getSecondaryDexopter(
                same(mPkgStateLib2), same(mPkgLib2), same(mParams), any());
        inOrder.verify(mInjector).getPrimaryDexopter(
                any(), same(mPkgStateLib4), same(mPkgLib4), same(mParams), any());
        inOrder.verify(mInjector).getSecondaryDexopter(
                same(mPkgStateLib4), same(mPkgLib4), same(mParams), any());

        verifyNoMoreDexopt(6 /* expectedPrimaryTimes */, 6 /* expectedSecondaryTimes */);
    }

    @Test
    public void testDexoptNoDependencies() throws Exception {
        mParams = new DexoptParams.Builder("install")
                          .setCompilerFilter("speed-profile")
                          .setFlags(ArtFlags.FLAG_FOR_SECONDARY_DEX,
                                  ArtFlags.FLAG_FOR_SECONDARY_DEX
                                          | ArtFlags.FLAG_SHOULD_INCLUDE_DEPENDENCIES)
                          .build();

        DexoptResult result = mDexoptHelper.dexopt(
                mSnapshot, mRequestedPackages, mParams, mCancellationSignal, mExecutor);

        assertThat(result.getPackageDexoptResults()).hasSize(3);
        checkPackageResult(result, 0 /* index */, PKG_NAME_FOO, DexoptResult.DEXOPT_PERFORMED,
                List.of(mPrimaryResults, mSecondaryResults));
        checkPackageResult(result, 1 /* index */, PKG_NAME_BAR, DexoptResult.DEXOPT_PERFORMED,
                List.of(mPrimaryResults, mSecondaryResults));
        checkPackageResult(result, 2 /* index */, PKG_NAME_LIBBAZ, DexoptResult.DEXOPT_PERFORMED,
                List.of(mPrimaryResults, mSecondaryResults));

        verifyNoMoreDexopt(3 /* expectedPrimaryTimes */, 3 /* expectedSecondaryTimes */);
    }

    @Test
    public void testDexoptPrimaryOnly() throws Exception {
        mParams = new DexoptParams.Builder("install")
                          .setCompilerFilter("speed-profile")
                          .setFlags(ArtFlags.FLAG_SHOULD_INCLUDE_DEPENDENCIES,
                                  ArtFlags.FLAG_FOR_SECONDARY_DEX
                                          | ArtFlags.FLAG_SHOULD_INCLUDE_DEPENDENCIES)
                          .build();

        DexoptResult result = mDexoptHelper.dexopt(
                mSnapshot, mRequestedPackages, mParams, mCancellationSignal, mExecutor);

        assertThat(result.getPackageDexoptResults()).hasSize(6);
        checkPackageResult(result, 0 /* index */, PKG_NAME_FOO, DexoptResult.DEXOPT_PERFORMED,
                List.of(mPrimaryResults));
        checkPackageResult(result, 1 /* index */, PKG_NAME_BAR, DexoptResult.DEXOPT_PERFORMED,
                List.of(mPrimaryResults));
        checkPackageResult(result, 2 /* index */, PKG_NAME_LIBBAZ, DexoptResult.DEXOPT_PERFORMED,
                List.of(mPrimaryResults));
        checkPackageResult(result, 3 /* index */, PKG_NAME_LIB1, DexoptResult.DEXOPT_PERFORMED,
                List.of(mPrimaryResults));
        checkPackageResult(result, 4 /* index */, PKG_NAME_LIB2, DexoptResult.DEXOPT_PERFORMED,
                List.of(mPrimaryResults));
        checkPackageResult(result, 5 /* index */, PKG_NAME_LIB4, DexoptResult.DEXOPT_PERFORMED,
                List.of(mPrimaryResults));

        verifyNoMoreDexopt(6 /* expectedPrimaryTimes */, 0 /* expectedSecondaryTimes */);
    }

    @Test
    public void testDexoptSdkPrimaryOnly() throws Exception {
        mParams = new DexoptParams.Builder("bg-dexopt")
                          .setCompilerFilter("speed-profile")
                          .setFlags(ArtFlags.FLAG_FOR_PRIMARY_DEX | ArtFlags.FLAG_FOR_SECONDARY_DEX)
                          .build();

        PackageState sdkPackageState =
                newPackageStateWithDefaults(PKG_NAME_SDK).setAppId(-1).build();
        lenient().when(mSnapshot.getPackageState(PKG_NAME_SDK)).thenReturn(sdkPackageState);
        mRequestedPackages = List.of(PKG_NAME_SDK);

        DexoptResult result = mDexoptHelper.dexopt(
                mSnapshot, mRequestedPackages, mParams, mCancellationSignal, mExecutor);

        assertThat(result.getPackageDexoptResults()).hasSize(1);
        checkPackageResult(result, 0 /* index */, PKG_NAME_SDK, DexoptResult.DEXOPT_PERFORMED,
                List.of(mPrimaryResults));

        verifyNoMoreDexopt(1 /* expectedPrimaryTimes */, 0 /* expectedSecondaryTimes */);
    }

    @Test
    public void testDexoptPrimaryOnlyNoDependencies() throws Exception {
        mParams = new DexoptParams.Builder("install")
                          .setCompilerFilter("speed-profile")
                          .setFlags(0,
                                  ArtFlags.FLAG_FOR_SECONDARY_DEX
                                          | ArtFlags.FLAG_SHOULD_INCLUDE_DEPENDENCIES)
                          .build();

        DexoptResult result = mDexoptHelper.dexopt(
                mSnapshot, mRequestedPackages, mParams, mCancellationSignal, mExecutor);

        assertThat(result.getPackageDexoptResults()).hasSize(3);
        checkPackageResult(result, 0 /* index */, PKG_NAME_FOO, DexoptResult.DEXOPT_PERFORMED,
                List.of(mPrimaryResults));
        checkPackageResult(result, 1 /* index */, PKG_NAME_BAR, DexoptResult.DEXOPT_PERFORMED,
                List.of(mPrimaryResults));
        checkPackageResult(result, 2 /* index */, PKG_NAME_LIBBAZ, DexoptResult.DEXOPT_PERFORMED,
                List.of(mPrimaryResults));

        verifyNoMoreDexopt(3 /* expectedPrimaryTimes */, 0 /* expectedSecondaryTimes */);
    }

    @Test
    public void testDexoptCancelledBetweenDex2oatInvocations() throws Exception {
        when(mPrimaryDexopter.dexopt()).thenAnswer(invocation -> {
            mCancellationSignal.cancel();
            return mPrimaryResults;
        });

        DexoptResult result = mDexoptHelper.dexopt(
                mSnapshot, mRequestedPackages, mParams, mCancellationSignal, mExecutor);

        assertThat(result.getFinalStatus()).isEqualTo(DexoptResult.DEXOPT_CANCELLED);

        assertThat(result.getPackageDexoptResults()).hasSize(6);
        checkPackageResult(result, 0 /* index */, PKG_NAME_FOO, DexoptResult.DEXOPT_CANCELLED,
                List.of(mPrimaryResults));
        checkPackageResult(
                result, 1 /* index */, PKG_NAME_BAR, DexoptResult.DEXOPT_CANCELLED, List.of());
        checkPackageResult(
                result, 2 /* index */, PKG_NAME_LIBBAZ, DexoptResult.DEXOPT_CANCELLED, List.of());
        checkPackageResult(
                result, 3 /* index */, PKG_NAME_LIB1, DexoptResult.DEXOPT_CANCELLED, List.of());
        checkPackageResult(
                result, 4 /* index */, PKG_NAME_LIB2, DexoptResult.DEXOPT_CANCELLED, List.of());
        checkPackageResult(
                result, 5 /* index */, PKG_NAME_LIB4, DexoptResult.DEXOPT_CANCELLED, List.of());

        verify(mInjector).getPrimaryDexopter(
                any(), same(mPkgStateFoo), same(mPkgFoo), same(mParams), any());

        verifyNoMoreDexopt(1 /* expectedPrimaryTimes */, 0 /* expectedSecondaryTimes */);
    }

    // This test verifies that every child thread can register its own listener on the cancellation
    // signal through `setOnCancelListener` (i.e., the listeners don't overwrite each other).
    @Test
    public void testDexoptCancelledDuringDex2oatInvocationsMultiThreaded() throws Exception {
        final int NUM_PACKAGES = 6;
        final long TIMEOUT_SEC = 10;
        var dexoptStarted = new Semaphore(0);
        var dexoptCancelled = new Semaphore(0);

        when(mInjector.getPrimaryDexopter(any(), any(), any(), any(), any())).thenAnswer(invocation -> {
            var cancellationSignal = invocation.<CancellationSignal>getArgument(4);
            var dexopter = mock(PrimaryDexopter.class);
            when(dexopter.dexopt()).thenAnswer(innerInvocation -> {
                // Simulate that the child thread registers its own listener.
                var isListenerCalled = new AtomicBoolean(false);
                cancellationSignal.setOnCancelListener(() -> isListenerCalled.set(true));

                dexoptStarted.release();
                assertThat(dexoptCancelled.tryAcquire(TIMEOUT_SEC, TimeUnit.SECONDS)).isTrue();

                // Verify that the listener is called.
                assertThat(isListenerCalled.get()).isTrue();

                return mPrimaryResults;
            });
            return dexopter;
        });

        ExecutorService dexoptExecutor = Executors.newFixedThreadPool(NUM_PACKAGES);
        Future<DexoptResult> future = ForkJoinPool.commonPool().submit(() -> {
            return mDexoptHelper.dexopt(
                    mSnapshot, mRequestedPackages, mParams, mCancellationSignal, dexoptExecutor);
        });

        try {
            // Wait for all dexopt operations to start.
            for (int i = 0; i < NUM_PACKAGES; i++) {
                assertThat(dexoptStarted.tryAcquire(TIMEOUT_SEC, TimeUnit.SECONDS)).isTrue();
            }

            mCancellationSignal.cancel();

            for (int i = 0; i < NUM_PACKAGES; i++) {
                dexoptCancelled.release();
            }
        } finally {
            dexoptExecutor.shutdown();
            Utils.getFuture(future);
        }
    }

    // This test verifies that dexopt operation on the current thread can be cancelled.
    @Test
    public void testDexoptCancelledDuringDex2oatInvocationsOnCurrentThread() throws Exception {
        final long TIMEOUT_SEC = 10;
        var dexoptStarted = new Semaphore(0);
        var dexoptCancelled = new Semaphore(0);

        when(mInjector.getPrimaryDexopter(any(), any(), any(), any(), any())).thenAnswer(invocation -> {
            var cancellationSignal = invocation.<CancellationSignal>getArgument(4);
            var dexopter = mock(PrimaryDexopter.class);
            when(dexopter.dexopt()).thenAnswer(innerInvocation -> {
                if (cancellationSignal.isCanceled()) {
                    return mPrimaryResults;
                }

                var isListenerCalled = new AtomicBoolean(false);
                cancellationSignal.setOnCancelListener(() -> isListenerCalled.set(true));

                dexoptStarted.release();
                assertThat(dexoptCancelled.tryAcquire(TIMEOUT_SEC, TimeUnit.SECONDS)).isTrue();

                // Verify that the listener is called.
                assertThat(isListenerCalled.get()).isTrue();

                return mPrimaryResults;
            });
            return dexopter;
        });

        // Use the current thread (the one in ForkJoinPool).
        Executor dexoptExecutor = Runnable::run;
        Future<DexoptResult> future = ForkJoinPool.commonPool().submit(() -> {
            return mDexoptHelper.dexopt(
                    mSnapshot, mRequestedPackages, mParams, mCancellationSignal, dexoptExecutor);
        });

        try {
            // Only one dexopt operation should start.
            assertThat(dexoptStarted.tryAcquire(TIMEOUT_SEC, TimeUnit.SECONDS)).isTrue();

            mCancellationSignal.cancel();

            dexoptCancelled.release();
        } finally {
            Utils.getFuture(future);
        }
    }

    @Test
    public void testDexoptNotDexoptable() throws Exception {
        when(PackageStateModulesUtils.isDexoptable(mPkgStateFoo)).thenReturn(false);

        mRequestedPackages = List.of(PKG_NAME_FOO);
        DexoptResult result = mDexoptHelper.dexopt(
                mSnapshot, mRequestedPackages, mParams, mCancellationSignal, mExecutor);

        assertThat(result.getFinalStatus()).isEqualTo(DexoptResult.DEXOPT_SKIPPED);
        assertThat(result.getPackageDexoptResults()).hasSize(1);
        checkPackageResult(
                result, 0 /* index */, PKG_NAME_FOO, DexoptResult.DEXOPT_SKIPPED, List.of());

        verifyNoDexopt();
    }

    @Test
    public void testDexoptLibraryNotDexoptable() throws Exception {
        when(PackageStateModulesUtils.isDexoptable(mPkgStateLib1)).thenReturn(false);

        mRequestedPackages = List.of(PKG_NAME_FOO);
        DexoptResult result = mDexoptHelper.dexopt(
                mSnapshot, mRequestedPackages, mParams, mCancellationSignal, mExecutor);

        assertThat(result.getFinalStatus()).isEqualTo(DexoptResult.DEXOPT_PERFORMED);
        assertThat(result.getPackageDexoptResults()).hasSize(1);
        checkPackageResult(result, 0 /* index */, PKG_NAME_FOO, DexoptResult.DEXOPT_PERFORMED,
                List.of(mPrimaryResults, mSecondaryResults));

        verifyNoMoreDexopt(1 /* expectedPrimaryTimes */, 1 /* expectedSecondaryTimes */);
    }

    @Test
    public void testDexoptIsHibernating() throws Exception {
        lenient().when(mAhm.isHibernatingGlobally(PKG_NAME_FOO)).thenReturn(true);

        mRequestedPackages = List.of(PKG_NAME_FOO);
        DexoptResult result = mDexoptHelper.dexopt(
                mSnapshot, mRequestedPackages, mParams, mCancellationSignal, mExecutor);

        assertThat(result.getFinalStatus()).isEqualTo(DexoptResult.DEXOPT_SKIPPED);
        checkPackageResult(
                result, 0 /* index */, PKG_NAME_FOO, DexoptResult.DEXOPT_SKIPPED, List.of());

        verifyNoDexopt();
    }

    @Test
    public void testDexoptIsHibernatingButOatArtifactDeletionDisabled() throws Exception {
        lenient().when(mAhm.isHibernatingGlobally(PKG_NAME_FOO)).thenReturn(true);
        lenient().when(mAhm.isOatArtifactDeletionEnabled()).thenReturn(false);

        DexoptResult result = mDexoptHelper.dexopt(
                mSnapshot, mRequestedPackages, mParams, mCancellationSignal, mExecutor);

        assertThat(result.getPackageDexoptResults()).hasSize(6);
        checkPackageResult(result, 0 /* index */, PKG_NAME_FOO, DexoptResult.DEXOPT_PERFORMED,
                List.of(mPrimaryResults, mSecondaryResults));
        checkPackageResult(result, 1 /* index */, PKG_NAME_BAR, DexoptResult.DEXOPT_PERFORMED,
                List.of(mPrimaryResults, mSecondaryResults));
        checkPackageResult(result, 2 /* index */, PKG_NAME_LIBBAZ, DexoptResult.DEXOPT_PERFORMED,
                List.of(mPrimaryResults, mSecondaryResults));
        checkPackageResult(result, 3 /* index */, PKG_NAME_LIB1, DexoptResult.DEXOPT_PERFORMED,
                List.of(mPrimaryResults, mSecondaryResults));
        checkPackageResult(result, 4 /* index */, PKG_NAME_LIB2, DexoptResult.DEXOPT_PERFORMED,
                List.of(mPrimaryResults, mSecondaryResults));
        checkPackageResult(result, 5 /* index */, PKG_NAME_LIB4, DexoptResult.DEXOPT_PERFORMED,
                List.of(mPrimaryResults, mSecondaryResults));
    }

    @Test(expected = IllegalArgumentException.class)
    public void testDexoptPackageNotFound() throws Exception {
        when(mSnapshot.getPackageState(any())).thenReturn(null);

        mDexoptHelper.dexopt(
                mSnapshot, mRequestedPackages, mParams, mCancellationSignal, mExecutor);

        verifyNoDexopt();
    }

    @Test(expected = IllegalArgumentException.class)
    public void testDexoptNoPackage() throws Exception {
        lenient().when(mPkgStateFoo.getAndroidPackage()).thenReturn(null);

        mDexoptHelper.dexopt(
                mSnapshot, mRequestedPackages, mParams, mCancellationSignal, mExecutor);

        verifyNoDexopt();
    }

    @Test
    public void testDexoptSplit() throws Exception {
        mRequestedPackages = List.of(PKG_NAME_FOO);
        mParams = new DexoptParams.Builder("install")
                          .setCompilerFilter("speed-profile")
                          .setFlags(ArtFlags.FLAG_FOR_PRIMARY_DEX | ArtFlags.FLAG_FOR_SINGLE_SPLIT)
                          .setSplitName("split_0")
                          .build();

        mDexoptHelper.dexopt(
                mSnapshot, mRequestedPackages, mParams, mCancellationSignal, mExecutor);
    }

    @Test
    public void testDexoptSplitNotFound() throws Exception {
        mRequestedPackages = List.of(PKG_NAME_FOO);
        mParams = new DexoptParams.Builder("install")
                          .setCompilerFilter("speed-profile")
                          .setFlags(ArtFlags.FLAG_FOR_PRIMARY_DEX | ArtFlags.FLAG_FOR_SINGLE_SPLIT)
                          .setSplitName("split_bogus")
                          .build();

        assertThrows(IllegalArgumentException.class, () -> {
            mDexoptHelper.dexopt(
                    mSnapshot, mRequestedPackages, mParams, mCancellationSignal, mExecutor);
        });
    }

    @Test
    public void testCallbacks() throws Exception {
        List<DexoptResult> list1 = new ArrayList<>();
        mConfig.addDexoptDoneCallback(
                false /* onlyIncludeUpdates */, Runnable::run, result -> list1.add(result));

        List<DexoptResult> list2 = new ArrayList<>();
        mConfig.addDexoptDoneCallback(
                false /* onlyIncludeUpdates */, Runnable::run, result -> list2.add(result));

        DexoptResult result = mDexoptHelper.dexopt(
                mSnapshot, mRequestedPackages, mParams, mCancellationSignal, mExecutor);

        assertThat(list1).containsExactly(result);
        assertThat(list2).containsExactly(result);
    }

    @Test
    public void testCallbackRemoved() throws Exception {
        List<DexoptResult> list1 = new ArrayList<>();
        DexoptDoneCallback callback1 = result -> list1.add(result);
        mConfig.addDexoptDoneCallback(false /* onlyIncludeUpdates */, Runnable::run, callback1);

        List<DexoptResult> list2 = new ArrayList<>();
        mConfig.addDexoptDoneCallback(
                false /* onlyIncludeUpdates */, Runnable::run, result -> list2.add(result));

        mConfig.removeDexoptDoneCallback(callback1);

        DexoptResult result = mDexoptHelper.dexopt(
                mSnapshot, mRequestedPackages, mParams, mCancellationSignal, mExecutor);

        assertThat(list1).isEmpty();
        assertThat(list2).containsExactly(result);
    }

    @Test(expected = IllegalStateException.class)
    public void testCallbackAlreadyAdded() throws Exception {
        List<DexoptResult> list = new ArrayList<>();
        DexoptDoneCallback callback = result -> list.add(result);
        mConfig.addDexoptDoneCallback(false /* onlyIncludeUpdates */, Runnable::run, callback);
        mConfig.addDexoptDoneCallback(false /* onlyIncludeUpdates */, Runnable::run, callback);
    }

    // Tests `addDexoptDoneCallback` with `onlyIncludeUpdates` being true and false.
    @Test
    public void testCallbackWithFailureResults() throws Exception {
        mParams = new DexoptParams.Builder("install")
                          .setCompilerFilter("speed-profile")
                          .setFlags(0,
                                  ArtFlags.FLAG_FOR_SECONDARY_DEX
                                          | ArtFlags.FLAG_SHOULD_INCLUDE_DEPENDENCIES)
                          .build();

        // This list should collect all results.
        List<DexoptResult> listAll = new ArrayList<>();
        mConfig.addDexoptDoneCallback(
                false /* onlyIncludeUpdates */, Runnable::run, result -> listAll.add(result));

        // This list should only collect results that have updates.
        List<DexoptResult> listOnlyIncludeUpdates = new ArrayList<>();
        mConfig.addDexoptDoneCallback(true /* onlyIncludeUpdates */, Runnable::run,
                result -> listOnlyIncludeUpdates.add(result));

        // Dexopt partially fails on package "foo".
        List<DexContainerFileDexoptResult> partialFailureResults = createResults(
                "/somewhere/app/foo/base.apk", DexoptResult.DEXOPT_PERFORMED /* status1 */,
                DexoptResult.DEXOPT_FAILED /* status2 */);
        var fooPrimaryDexopter = mock(PrimaryDexopter.class);
        when(mInjector.getPrimaryDexopter(any(), same(mPkgStateFoo), any(), any(), any()))
                .thenReturn(fooPrimaryDexopter);
        when(fooPrimaryDexopter.dexopt()).thenReturn(partialFailureResults);

        // Dexopt totally fails on package "bar".
        List<DexContainerFileDexoptResult> totalFailureResults = createResults(
                "/somewhere/app/bar/base.apk", DexoptResult.DEXOPT_FAILED /* status1 */,
                DexoptResult.DEXOPT_FAILED /* status2 */);
        var barPrimaryDexopter = mock(PrimaryDexopter.class);
        when(mInjector.getPrimaryDexopter(any(), same(mPkgStateBar), any(), any(), any()))
                .thenReturn(barPrimaryDexopter);
        when(barPrimaryDexopter.dexopt()).thenReturn(totalFailureResults);

        DexoptResult resultWithSomeUpdates = mDexoptHelper.dexopt(mSnapshot,
                List.of(PKG_NAME_FOO, PKG_NAME_BAR), mParams, mCancellationSignal, mExecutor);
        DexoptResult resultWithNoUpdates = mDexoptHelper.dexopt(
                mSnapshot, List.of(PKG_NAME_BAR), mParams, mCancellationSignal, mExecutor);

        assertThat(listAll).containsExactly(resultWithSomeUpdates, resultWithNoUpdates);

        assertThat(listOnlyIncludeUpdates).hasSize(1);
        assertThat(listOnlyIncludeUpdates.get(0)
                           .getPackageDexoptResults()
                           .stream()
                           .map(PackageDexoptResult::getPackageName)
                           .toList())
                .containsExactly(PKG_NAME_FOO);
    }

    @Test
    public void testProgressCallback() throws Exception {
        mParams = new DexoptParams.Builder("install")
                          .setCompilerFilter("speed-profile")
                          .setFlags(ArtFlags.FLAG_FOR_SECONDARY_DEX,
                                  ArtFlags.FLAG_FOR_SECONDARY_DEX
                                          | ArtFlags.FLAG_SHOULD_INCLUDE_DEPENDENCIES)
                          .build();

        // Delay the executor to verify that the commands passed to the executor are not bound to
        // changing variables.
        var progressCallbackExecutor = new DelayedExecutor();
        Consumer<OperationProgress> progressCallback = mock(Consumer.class);

        mDexoptHelper.dexopt(mSnapshot, mRequestedPackages, mParams, mCancellationSignal, mExecutor,
                progressCallbackExecutor, progressCallback);

        progressCallbackExecutor.runAll();

        List<DexContainerFileDexoptResult> fileResults =
                Stream.concat(mPrimaryResults.stream(), mSecondaryResults.stream()).toList();

        InOrder inOrder = inOrder(progressCallback);
        inOrder.verify(progressCallback)
                .accept(eq(OperationProgress.create(
                        0 /* current */, 3 /* total */, null /* packageDexoptResult */)));
        inOrder.verify(progressCallback)
                .accept(eq(OperationProgress.create(1 /* current */, 3 /* total */,
                        PackageDexoptResult.create(
                                PKG_NAME_FOO, fileResults, null /* packageLevelStatus */))));
        inOrder.verify(progressCallback)
                .accept(eq(OperationProgress.create(2 /* current */, 3 /* total */,
                        PackageDexoptResult.create(
                                PKG_NAME_BAR, fileResults, null /* packageLevelStatus */))));
        inOrder.verify(progressCallback)
                .accept(eq(OperationProgress.create(3 /* current */, 3 /* total */,
                        PackageDexoptResult.create(
                                PKG_NAME_LIBBAZ, fileResults, null /* packageLevelStatus */))));
    }

    private PackageStateBuilder newPackageStateWithDefaults(String packageName) {
        return newPackageState(packageName)
                .setAppId(12345)
                .addSplit(newSplit().build())
                .setDexoptable(true);
    }

    private void preparePackagesAndLibraries() {
        // Dependency graph:
        //                foo                bar
        //                 |                  |
        //            lib1a (lib1)       lib1b (lib1)       lib1c (lib1)
        //               /   \             /   \                  |
        //              /     \           /     \                 |
        //  libbaz (libbaz)    lib2 (lib2)    lib4 (lib4)    lib3 (lib3)
        //
        // "lib1a", "lib1b", and "lib1c" belong to the same package "lib1".

        mRequestedPackages = List.of(PKG_NAME_FOO, PKG_NAME_BAR, PKG_NAME_LIBBAZ);

        // The native library is not dexoptable.
        SharedLibrary libNative = newLibrary()
                                          .setName("libnative")
                                          .setPackageName("com.example.libnative")
                                          .setNative(true)
                                          .build();

        SharedLibrary libbaz =
                newLibrary().setName("libbaz").setPackageName(PKG_NAME_LIBBAZ).build();
        SharedLibrary lib4 = newLibrary().setName("lib4").setPackageName(PKG_NAME_LIB4).build();
        SharedLibrary lib3 = newLibrary().setName("lib3").setPackageName(PKG_NAME_LIB3).build();
        SharedLibrary lib2 = newLibrary().setName("lib2").setPackageName(PKG_NAME_LIB2).build();
        SharedLibrary lib1a = newLibrary()
                                      .setName("lib1a")
                                      .setPackageName(PKG_NAME_LIB1)
                                      .addDeps(libbaz, lib2)
                                      .build();
        SharedLibrary lib1b = newLibrary()
                                      .setName("lib1b")
                                      .setPackageName(PKG_NAME_LIB1)
                                      .addDeps(lib2, libNative, lib4)
                                      .build();
        SharedLibrary lib1c =
                newLibrary().setName("lib1c").setPackageName(PKG_NAME_LIB1).addDeps(lib3).build();

        mPkgStateFoo = newPackageStateWithDefaults(PKG_NAME_FOO)
                               .addSharedLibraryDeps(lib1a, libNative)
                               .addSplit(newSplit().setName("split_0").build())
                               .build();
        mPkgFoo = mPkgStateFoo.getAndroidPackage();
        lenient().when(mSnapshot.getPackageState(PKG_NAME_FOO)).thenReturn(mPkgStateFoo);

        mPkgStateBar =
                newPackageStateWithDefaults(PKG_NAME_BAR).addSharedLibraryDeps(lib1b).build();
        mPkgBar = mPkgStateBar.getAndroidPackage();
        lenient().when(mSnapshot.getPackageState(PKG_NAME_BAR)).thenReturn(mPkgStateBar);

        mPkgStateLib1 = newPackageStateWithDefaults(PKG_NAME_LIB1)
                                .addSharedLibraryDeps(libbaz, lib2, lib3, lib4)
                                .build();
        mPkgLib1 = mPkgStateLib1.getAndroidPackage();
        lenient().when(mSnapshot.getPackageState(PKG_NAME_LIB1)).thenReturn(mPkgStateLib1);

        mPkgStateLib2 = newPackageStateWithDefaults(PKG_NAME_LIB2).build();
        mPkgLib2 = mPkgStateLib2.getAndroidPackage();
        lenient().when(mSnapshot.getPackageState(PKG_NAME_LIB2)).thenReturn(mPkgStateLib2);

        // This should not be considered as a transitive dependency of any requested package, even
        // though it is a dependency of package "lib1".
        PackageState pkgStateLib3 = newPackageStateWithDefaults(PKG_NAME_LIB3).build();
        lenient().when(mSnapshot.getPackageState(PKG_NAME_LIB3)).thenReturn(pkgStateLib3);

        mPkgStateLib4 = newPackageStateWithDefaults(PKG_NAME_LIB4).build();
        mPkgLib4 = mPkgStateLib4.getAndroidPackage();
        lenient().when(mSnapshot.getPackageState(PKG_NAME_LIB4)).thenReturn(mPkgStateLib4);

        mPkgStateLibbaz = newPackageStateWithDefaults(PKG_NAME_LIBBAZ).build();
        mPkgLibbaz = mPkgStateLibbaz.getAndroidPackage();
        lenient().when(mSnapshot.getPackageState(PKG_NAME_LIBBAZ)).thenReturn(mPkgStateLibbaz);
    }

    private void verifyNoDexopt() {
        verify(mInjector, never()).getPrimaryDexopter(any(), any(), any(), any(), any());
        verify(mInjector, never()).getSecondaryDexopter(any(), any(), any(), any());
    }

    private void verifyNoMoreDexopt(int expectedPrimaryTimes, int expectedSecondaryTimes) {
        verify(mInjector, times(expectedPrimaryTimes))
                .getPrimaryDexopter(any(), any(), any(), any(), any());
        verify(mInjector, times(expectedSecondaryTimes))
                .getSecondaryDexopter(any(), any(), any(), any());
    }

    private List<DexContainerFileDexoptResult> createResults(
            String dexPath, @DexoptResultStatus int status1, @DexoptResultStatus int status2) {
        return List.of(DexContainerFileDexoptResult.create(
                               dexPath, true /* isPrimaryAbi */, "arm64-v8a", "verify", status1),
                DexContainerFileDexoptResult.create(
                        dexPath, false /* isPrimaryAbi */, "armeabi-v7a", "verify", status2));
    }

    private void checkPackageResult(DexoptResult result, int index, String packageName,
            @DexoptResult.DexoptResultStatus int status,
            List<List<DexContainerFileDexoptResult>> dexContainerFileDexoptResults) {
        PackageDexoptResult packageResult = result.getPackageDexoptResults().get(index);
        assertThat(packageResult.getPackageName()).isEqualTo(packageName);
        assertThat(packageResult.getStatus()).isEqualTo(status);
        assertThat(packageResult.getDexContainerFileDexoptResults())
                .containsExactlyElementsIn(
                        dexContainerFileDexoptResults.stream().flatMap(r -> r.stream()).toList());
    }

    /** An executor that delays execution until `runAll` is called. */
    private static class DelayedExecutor implements Executor {
        private List<Runnable> mCommands = new ArrayList<>();

        public void execute(Runnable command) {
            mCommands.add(command);
        }

        public void runAll() {
            for (Runnable command : mCommands) {
                command.run();
            }
            mCommands.clear();
        }
    }
}
