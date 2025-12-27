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

import static com.android.server.art.PreRebootDexoptJob.JOB_ID;

import static com.google.common.truth.Truth.assertThat;

import static org.mockito.Mockito.any;
import static org.mockito.Mockito.anyBoolean;
import static org.mockito.Mockito.anyInt;
import static org.mockito.Mockito.doAnswer;
import static org.mockito.Mockito.doThrow;
import static org.mockito.Mockito.eq;
import static org.mockito.Mockito.isNull;
import static org.mockito.Mockito.lenient;
import static org.mockito.Mockito.mock;
import static org.mockito.Mockito.never;
import static org.mockito.Mockito.verify;
import static org.mockito.Mockito.when;

import android.app.job.JobInfo;
import android.app.job.JobParameters;
import android.app.job.JobScheduler;
import android.os.CancellationSignal;
import android.os.ServiceSpecificException;
import android.os.SystemProperties;
import android.os.UpdateEngine;
import android.platform.test.annotations.DisableFlags;
import android.platform.test.annotations.EnableFlags;
import android.platform.test.flag.junit.SetFlagsRule;
import android.provider.DeviceConfig;

import androidx.test.filters.SmallTest;

import com.android.server.art.PreRebootDexoptJob.StagedFilesAge;
import com.android.server.art.model.ArtFlags;
import com.android.server.art.prereboot.PreRebootDriver;
import com.android.server.art.prereboot.PreRebootDriver.PreRebootResult;
import com.android.server.art.prereboot.PreRebootStatsReporter;
import com.android.server.art.proto.PreRebootStats.Status;
import com.android.server.art.testing.PreRebootStatsReporterHarness;
import com.android.server.art.testing.StaticMockitoRule;
import com.android.server.art.testing.TestingUtils;

import org.junit.Before;
import org.junit.Rule;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.mockito.Mock;
import org.mockito.junit.MockitoJUnitRunner;

import java.time.Duration;
import java.util.concurrent.CompletableFuture;
import java.util.concurrent.Semaphore;
import java.util.concurrent.TimeUnit;
import java.util.function.Supplier;

@SmallTest
@RunWith(MockitoJUnitRunner.StrictStubs.class)
public class PreRebootDexoptJobTest {
    private static final long TIMEOUT_SEC = 10;
    private static final long CURRENT_TIME_MS = 10000000000l;

    @Rule
    public StaticMockitoRule mockitoRule =
            new StaticMockitoRule(SystemProperties.class, BackgroundDexoptJobService.class);
    @Rule public final SetFlagsRule mSetFlagsRule = new SetFlagsRule();

    @Mock private PreRebootDexoptJob.Injector mInjector;
    @Mock private JobScheduler mJobScheduler;
    @Mock private PreRebootDriver mPreRebootDriver;
    @Mock private BackgroundDexoptJobService mJobService;
    @Mock private IArtd mArtd;
    @Mock private UpdateEngine mUpdateEngine;
    private PreRebootDexoptJob mPreRebootDexoptJob;
    private JobInfo mJobInfo;
    private JobParameters mJobParameters;
    private PreRebootStatsReporterHarness mPreRebootStatsReporterHarness;

    @Before
    public void setUp() throws Exception {
        mPreRebootStatsReporterHarness = new PreRebootStatsReporterHarness();

        // By default, the job is enabled by a build-time flag.
        lenient()
                .when(SystemProperties.getBoolean(eq("pm.dexopt.disable_bg_dexopt"), anyBoolean()))
                .thenReturn(false);
        lenient()
                .when(SystemProperties.getBoolean(eq("dalvik.vm.enable_pr_dexopt"), anyBoolean()))
                .thenReturn(true);
        lenient()
                .when(mInjector.getDeviceConfigBoolean(
                        eq(DeviceConfig.NAMESPACE_RUNTIME), eq("enable_pr_dexopt"), anyBoolean()))
                .thenReturn(false);

        lenient().when(mInjector.getJobScheduler()).thenReturn(mJobScheduler);
        lenient().when(mInjector.getPreRebootDriver()).thenReturn(mPreRebootDriver);
        lenient()
                .when(mInjector.getStatsReporter())
                .thenReturn(mPreRebootStatsReporterHarness.createStatsReporter());
        lenient().when(mInjector.getArtd()).thenReturn(mArtd);
        lenient().when(mInjector.getUpdateEngine()).thenReturn(mUpdateEngine);
        lenient().when(mInjector.getCurrentTimeMillis()).thenReturn(CURRENT_TIME_MS);

        lenient().when(mJobScheduler.schedule(any())).thenAnswer(invocation -> {
            mJobInfo = invocation.<JobInfo>getArgument(0);
            mJobParameters = mock(JobParameters.class);
            assertThat(mJobInfo.getId()).isEqualTo(JOB_ID);
            lenient().when(mJobParameters.getExtras()).thenReturn(mJobInfo.getExtras());
            return JobScheduler.RESULT_SUCCESS;
        });

        lenient()
                .doAnswer(invocation -> {
                    mJobInfo = null;
                    mJobParameters = null;
                    return null;
                })
                .when(mJobScheduler)
                .cancel(JOB_ID);

        lenient().when(mJobScheduler.getPendingJob(JOB_ID)).thenAnswer(invocation -> {
            return mJobInfo;
        });

        mPreRebootDexoptJob = new PreRebootDexoptJob(mInjector);
        lenient().when(BackgroundDexoptJobService.getJob(JOB_ID)).thenReturn(mPreRebootDexoptJob);

        lenient()
                .doAnswer(invocation -> {
                    CompletableFuture<?> unused = mPreRebootDexoptJob.notifyUpdateEngineReady();
                    return null;
                })
                .when(mUpdateEngine)
                .triggerPostinstall("system");
    }

    @Test
    public void testSchedule() throws Exception {
        assertThat(mPreRebootDexoptJob.onUpdateReadyImpl(null /* otaSlot */))
                .isEqualTo(ArtFlags.SCHEDULE_SUCCESS);

        assertThat(mJobInfo.isPeriodic()).isFalse();
        assertThat(mJobInfo.isRequireDeviceIdle()).isTrue();
        assertThat(mJobInfo.isRequireCharging()).isTrue();
        assertThat(mJobInfo.isRequireBatteryNotLow()).isTrue();

        mPreRebootStatsReporterHarness.recordFakeAfterRebootDataAndReport();
        mPreRebootStatsReporterHarness.verifyJobStats(Status.STATUS_SCHEDULED);
    }

    @Test
    public void testScheduleDisabled() {
        when(SystemProperties.getBoolean(eq("pm.dexopt.disable_bg_dexopt"), anyBoolean()))
                .thenReturn(true);

        assertThat(mPreRebootDexoptJob.onUpdateReadyImpl(null /* otaSlot */))
                .isEqualTo(ArtFlags.SCHEDULE_DISABLED_BY_SYSPROP);

        verify(mJobScheduler, never()).schedule(any());

        mPreRebootStatsReporterHarness.recordFakeAfterRebootDataAndReport();
        mPreRebootStatsReporterHarness.verifyJobStats(Status.STATUS_NOT_SCHEDULED_DISABLED);
    }

    @Test
    public void testSyncStartDisabled() throws Exception {
        when(SystemProperties.getBoolean(eq("pm.dexopt.disable_bg_dexopt"), anyBoolean()))
                .thenReturn(true);

        CompletableFuture<Void> future = mPreRebootDexoptJob.onUpdateReadyStartNow(
                null /* otaSlot */, true /* isUpdataEngineReady */);

        assertThat(future).isNull();
        verify(mPreRebootDriver, never()).run(any(), anyBoolean(), any());

        mPreRebootStatsReporterHarness.recordFakeAfterRebootDataAndReport();
        mPreRebootStatsReporterHarness.verifyJobStats(Status.STATUS_NOT_SCHEDULED_DISABLED);
    }

    @Test
    public void testScheduleNotEnabled() {
        when(SystemProperties.getBoolean(eq("dalvik.vm.enable_pr_dexopt"), anyBoolean()))
                .thenReturn(false);

        assertThat(mPreRebootDexoptJob.onUpdateReadyImpl(null /* otaSlot */))
                .isEqualTo(ArtFlags.SCHEDULE_DISABLED_BY_SYSPROP);

        verify(mJobScheduler, never()).schedule(any());

        mPreRebootStatsReporterHarness.recordFakeAfterRebootDataAndReport();
        mPreRebootStatsReporterHarness.verifyJobStats(Status.STATUS_NOT_SCHEDULED_DISABLED);
    }

    @Test
    public void testSyncStartNotEnabled() throws Exception {
        when(SystemProperties.getBoolean(eq("dalvik.vm.enable_pr_dexopt"), anyBoolean()))
                .thenReturn(false);

        CompletableFuture<Void> future = mPreRebootDexoptJob.onUpdateReadyStartNow(
                null /* otaSlot */, true /* isUpdataEngineReady */);

        assertThat(future).isNull();
        verify(mPreRebootDriver, never()).run(any(), anyBoolean(), any());

        mPreRebootStatsReporterHarness.recordFakeAfterRebootDataAndReport();
        mPreRebootStatsReporterHarness.verifyJobStats(Status.STATUS_NOT_SCHEDULED_DISABLED);
    }

    @Test
    public void testScheduleEnabledByPhenotypeFlag() {
        lenient()
                .when(SystemProperties.getBoolean(eq("dalvik.vm.enable_pr_dexopt"), anyBoolean()))
                .thenReturn(false);
        lenient()
                .when(mInjector.getDeviceConfigBoolean(
                        eq(DeviceConfig.NAMESPACE_RUNTIME), eq("enable_pr_dexopt"), anyBoolean()))
                .thenReturn(true);

        assertThat(mPreRebootDexoptJob.onUpdateReadyImpl(null /* otaSlot */))
                .isEqualTo(ArtFlags.SCHEDULE_SUCCESS);

        verify(mJobScheduler).schedule(any());

        mPreRebootStatsReporterHarness.recordFakeAfterRebootDataAndReport();
        mPreRebootStatsReporterHarness.verifyJobStats(Status.STATUS_SCHEDULED);
    }

    @Test
    public void testScheduleForceDisabledByPhenotypeFlag() {
        lenient()
                .when(SystemProperties.getBoolean(eq("dalvik.vm.enable_pr_dexopt"), anyBoolean()))
                .thenReturn(true);
        lenient()
                .when(mInjector.getDeviceConfigBoolean(
                        eq(DeviceConfig.NAMESPACE_RUNTIME), eq("enable_pr_dexopt"), anyBoolean()))
                .thenReturn(true);
        when(mInjector.getDeviceConfigBoolean(eq(DeviceConfig.NAMESPACE_RUNTIME),
                     eq("force_disable_pr_dexopt"), anyBoolean()))
                .thenReturn(true);

        assertThat(mPreRebootDexoptJob.onUpdateReadyImpl(null /* otaSlot */))
                .isEqualTo(ArtFlags.SCHEDULE_DISABLED_BY_SYSPROP);

        verify(mJobScheduler, never()).schedule(any());

        mPreRebootStatsReporterHarness.recordFakeAfterRebootDataAndReport();
        mPreRebootStatsReporterHarness.verifyJobStats(Status.STATUS_NOT_SCHEDULED_DISABLED);
    }

    @Test
    public void testUnschedule() {
        mPreRebootDexoptJob.onUpdateReadyImpl(null /* otaSlot */);
        verify(mJobScheduler).cancel(JOB_ID);
    }

    private void checkStart(String otaSlot, Supplier<Boolean> mapSnapshotsForOtaMatcher)
            throws Exception {
        var jobStarted = new Semaphore(0);
        when(mPreRebootDriver.run(eq(otaSlot), mapSnapshotsForOtaMatcher.get(), any()))
                .thenAnswer(invocation -> {
                    jobStarted.release();
                    return new PreRebootResult(Status.STATUS_FINISHED);
                });

        mPreRebootDexoptJob.onUpdateReadyImpl(otaSlot);
        mPreRebootDexoptJob.onStartJobImpl(mJobService, mJobParameters);
        assertThat(jobStarted.tryAcquire(TIMEOUT_SEC, TimeUnit.SECONDS)).isTrue();

        mPreRebootDexoptJob.waitForRunningJob();

        mPreRebootStatsReporterHarness.recordFakeAfterRebootDataAndReport();
        mPreRebootStatsReporterHarness.verifyJobStats(Status.STATUS_FINISHED);
    }

    @Test
    @EnableFlags({android.os.Flags.FLAG_UPDATE_ENGINE_API})
    public void testStartWithUpdateEngineApi() throws Exception {
        checkStart("_b" /* otaSlot */, () -> eq(false) /* mapSnapshotsForOtaMatcher */);
        verify(mUpdateEngine).triggerPostinstall("system");
    }

    @Test
    @DisableFlags({android.os.Flags.FLAG_UPDATE_ENGINE_API})
    public void testStartWithoutUpdateEngineApi() throws Exception {
        checkStart("_b" /* otaSlot */, () -> eq(true) /* mapSnapshotsForOtaMatcher */);
        verify(mUpdateEngine, never()).triggerPostinstall(any());
    }

    @Test
    public void testStartMainline() throws Exception {
        checkStart(null /* otaSlot */, () -> anyBoolean() /* mapSnapshotsForOtaMatcher */);
        verify(mUpdateEngine, never()).triggerPostinstall(any());
    }

    @Test
    @EnableFlags({android.os.Flags.FLAG_UPDATE_ENGINE_API})
    public void testStartWithUpdateEngineApiSkippedDueToUpdateGone() throws Exception {
        final int POSTINTALL_RUNNER_ERROR = 5;
        doThrow(new ServiceSpecificException(POSTINTALL_RUNNER_ERROR,
                        "Postinstall action did not run. OTA update must first reach the "
                                + "Postinstall phase(which verfies that all partitions can be "
                                + "mounted) before calling TriggerPostinstall"))
                .when(mUpdateEngine)
                .triggerPostinstall("system");

        mPreRebootDexoptJob.onUpdateReadyImpl("_b");
        mPreRebootDexoptJob.onStartJobImpl(mJobService, mJobParameters);

        mPreRebootDexoptJob.waitForRunningJob();

        mPreRebootStatsReporterHarness.recordFakeAfterRebootDataAndReport();
        mPreRebootStatsReporterHarness.verifyJobStats(Status.STATUS_FINISHED);
    }

    @Test
    @EnableFlags({android.os.Flags.FLAG_UPDATE_ENGINE_API})
    public void testStartWithUpdateEngineApiFailedDueToUnknownError() throws Exception {
        final int POSTINTALL_RUNNER_ERROR = 5;
        doThrow(new ServiceSpecificException(POSTINTALL_RUNNER_ERROR, "Some unknown error"))
                .when(mUpdateEngine)
                .triggerPostinstall("system");

        mPreRebootDexoptJob.onUpdateReadyImpl("_b");
        mPreRebootDexoptJob.onStartJobImpl(mJobService, mJobParameters);

        mPreRebootDexoptJob.waitForRunningJob();

        mPreRebootStatsReporterHarness.recordFakeAfterRebootDataAndReport();
        mPreRebootStatsReporterHarness.verifyJobStats(Status.STATUS_FAILED);
    }

    private void checkSyncStart(boolean isUpdateEngineReady, boolean expectedMapSnapshotsForOta)
            throws Exception {
        when(mPreRebootDriver.run(eq("_b"), eq(expectedMapSnapshotsForOta), any()))
                .thenReturn(new PreRebootResult(Status.STATUS_FINISHED));

        CompletableFuture<Void> future =
                mPreRebootDexoptJob.onUpdateReadyStartNow("_b" /* otaSlot */, isUpdateEngineReady);

        Utils.getFuture(future);

        mPreRebootStatsReporterHarness.recordFakeAfterRebootDataAndReport();
        mPreRebootStatsReporterHarness.verifyJobStats(Status.STATUS_FINISHED);
    }

    @Test
    @EnableFlags({android.os.Flags.FLAG_UPDATE_ENGINE_API})
    public void testSyncStartWithUpdateEngineApi() throws Exception {
        checkSyncStart(false /* isUpdataEngineReady */, false /* expectedMapSnapshotsForOta */);
        verify(mUpdateEngine).triggerPostinstall("system");
    }

    @Test
    @DisableFlags({android.os.Flags.FLAG_UPDATE_ENGINE_API})
    public void testSyncStartWithoutUpdateEngineApi() throws Exception {
        checkSyncStart(false /* isUpdataEngineReady */, true /* expectedMapSnapshotsForOta */);
        verify(mUpdateEngine, never()).triggerPostinstall(any());
    }

    @Test
    public void testSyncStartWithIsUpdateEngineReady() throws Exception {
        checkSyncStart(true /* isUpdataEngineReady */, false /* expectedMapSnapshotsForOta */);
        verify(mUpdateEngine, never()).triggerPostinstall(any());
    }

    @Test
    public void testCancel() {
        Semaphore dexoptCancelled = new Semaphore(0);
        Semaphore jobExited = new Semaphore(0);
        when(mPreRebootDriver.run(any(), anyBoolean(), any())).thenAnswer(invocation -> {
            var cancellationSignal = invocation.<CancellationSignal>getArgument(2);
            cancellationSignal.setOnCancelListener(() -> dexoptCancelled.release());
            assertThat(dexoptCancelled.tryAcquire(TIMEOUT_SEC, TimeUnit.SECONDS)).isTrue();
            jobExited.release();
            return new PreRebootResult(Status.STATUS_FINISHED);
        });

        mPreRebootDexoptJob.onUpdateReadyImpl(null /* otaSlot */);
        mPreRebootDexoptJob.onStartJobImpl(mJobService, mJobParameters);
        mPreRebootDexoptJob.onStopJobImpl(mJobParameters);

        // Check that `onStopJob` is really blocking. If it wasn't, the check below might still pass
        // due to a race, but we would have a flaky test.
        assertThat(jobExited.tryAcquire()).isTrue();

        mPreRebootStatsReporterHarness.recordFakeAfterRebootDataAndReport();
        mPreRebootStatsReporterHarness.verifyJobStats(Status.STATUS_FINISHED);
    }

    @Test
    public void testSyncCancel() throws Exception {
        Semaphore dexoptCancelled = new Semaphore(0);
        Semaphore jobExited = new Semaphore(0);
        when(mPreRebootDriver.run(any(), anyBoolean(), any())).thenAnswer(invocation -> {
            var cancellationSignal = invocation.<CancellationSignal>getArgument(2);
            cancellationSignal.setOnCancelListener(() -> dexoptCancelled.release());
            assertThat(dexoptCancelled.tryAcquire(TIMEOUT_SEC, TimeUnit.SECONDS)).isTrue();
            jobExited.release();
            return new PreRebootResult(Status.STATUS_FINISHED);
        });

        CompletableFuture<Void> future = mPreRebootDexoptJob.onUpdateReadyStartNow(
                null /* otaSlot */, true /* isUpdataEngineReady */);
        mPreRebootDexoptJob.cancelGiven(future, false /* expectInterrupt */);

        // Check that `cancelGiven` is really blocking. If it wasn't, the check below might still
        // pass due to a race, but we would have a flaky test.
        assertThat(jobExited.tryAcquire()).isTrue();

        mPreRebootStatsReporterHarness.recordFakeAfterRebootDataAndReport();
        mPreRebootStatsReporterHarness.verifyJobStats(Status.STATUS_FINISHED);
    }

    @Test
    public void testUpdateOtaSlotOtaThenMainline() {
        mPreRebootDexoptJob.onUpdateReadyImpl("_b" /* otaSlot */);
        mPreRebootDexoptJob.onUpdateReadyImpl(null /* otaSlot */);

        when(mPreRebootDriver.run(eq("_b"), anyBoolean(), any()))
                .thenReturn(new PreRebootResult(Status.STATUS_FINISHED));

        mPreRebootDexoptJob.onStartJobImpl(mJobService, mJobParameters);
        mPreRebootDexoptJob.waitForRunningJob();
    }

    @Test
    public void testUpdateOtaSlotMainlineThenOta() {
        mPreRebootDexoptJob.onUpdateReadyImpl(null /* otaSlot */);
        mPreRebootDexoptJob.onUpdateReadyImpl("_a" /* otaSlot */);

        when(mPreRebootDriver.run(eq("_a"), anyBoolean(), any()))
                .thenReturn(new PreRebootResult(Status.STATUS_FINISHED));

        mPreRebootDexoptJob.onStartJobImpl(mJobService, mJobParameters);
        mPreRebootDexoptJob.waitForRunningJob();
    }

    @Test
    public void testUpdateOtaSlotMainlineThenMainline() {
        mPreRebootDexoptJob.onUpdateReadyImpl(null /* otaSlot */);
        mPreRebootDexoptJob.onUpdateReadyImpl(null /* otaSlot */);

        when(mPreRebootDriver.run(isNull(), anyBoolean(), any()))
                .thenReturn(new PreRebootResult(Status.STATUS_FINISHED));

        mPreRebootDexoptJob.onStartJobImpl(mJobService, mJobParameters);
        mPreRebootDexoptJob.waitForRunningJob();
    }

    @Test
    public void testUpdateOtaSlotOtaThenOta() {
        mPreRebootDexoptJob.onUpdateReadyImpl("_b" /* otaSlot */);
        mPreRebootDexoptJob.onUpdateReadyImpl("_b" /* otaSlot */);

        when(mPreRebootDriver.run(eq("_b"), anyBoolean(), any()))
                .thenReturn(new PreRebootResult(Status.STATUS_FINISHED));

        mPreRebootDexoptJob.onStartJobImpl(mJobService, mJobParameters);
        mPreRebootDexoptJob.waitForRunningJob();
    }

    @Test(expected = IllegalStateException.class)
    public void testUpdateOtaSlotOtaThenOtaDifferentSlots() {
        mPreRebootDexoptJob.onUpdateReadyImpl("_b" /* otaSlot */);
        mPreRebootDexoptJob.onUpdateReadyImpl("_a" /* otaSlot */);
    }

    @Test(expected = IllegalStateException.class)
    public void testUpdateOtaSlotOtaBogusSlot() {
        mPreRebootDexoptJob.onUpdateReadyImpl("_bogus" /* otaSlot */);
    }

    @Test
    public void testStatsReportingForSuperseded() throws Exception {
        mPreRebootDexoptJob.onUpdateReadyImpl(null /* otaSlot */);

        when(mArtd.checkPreRebootStagedFilesStatus())
                .thenReturn(TestingUtils.createPreRebootStagedFilesStatus(
                        false /* isCommittable */, 200 /* createdAtMillis */));
        when(mInjector.getCurrentTimeMillis()).thenReturn(800l);

        mPreRebootDexoptJob.onUpdateReadyImpl("_a" /* otaSlot */);

        mPreRebootStatsReporterHarness.verifyArtifactsStats(
                PreRebootStatsReporter.END_STATUS_SUPERSEDED, 600 /* ageMillis */);
        mPreRebootStatsReporterHarness.verifyTimes(1);
    }

    /**
     * Verifies that `jobFinished` is not mistakenly called for an old job after a new job is
     * started.
     */
    @Test
    public void testRace1() throws Exception {
        var jobBlocker = new Semaphore(0);

        when(mPreRebootDriver.run(any(), anyBoolean(), any())).thenAnswer(invocation -> {
            // Simulate that the job takes a while to exit, no matter it's cancelled or not.
            assertThat(jobBlocker.tryAcquire(TIMEOUT_SEC, TimeUnit.SECONDS)).isTrue();
            return new PreRebootResult(Status.STATUS_FINISHED);
        });

        // An update arrives. A job is scheduled.
        mPreRebootDexoptJob.onUpdateReadyImpl(null /* otaSlot */);

        // The job scheduler starts the job.
        mPreRebootDexoptJob.onStartJobImpl(mJobService, mJobParameters);

        var jobFinishedCalledAfterNewJobStarted = new Semaphore(0);

        var thread = new Thread(() -> {
            // Another update arrives. A new job is scheduled, replacing the old job. The old job
            // doesn't exit immediately, so this call is blocked.
            JobParameters oldParameters = mJobParameters;
            mPreRebootDexoptJob.onUpdateReadyImpl(null /* otaSlot */);

            // The job scheduler tries to cancel the old job because of the new update. This call
            // doesn't matter because the job has already been cancelled by ourselves during the
            // `onUpdateReadyImpl` call above.
            mPreRebootDexoptJob.onStopJobImpl(oldParameters);

            // The job scheduler starts the new job.
            mPreRebootDexoptJob.onStartJobImpl(mJobService, mJobParameters);

            doAnswer(invocation -> {
                jobFinishedCalledAfterNewJobStarted.release();
                return null;
            })
                    .when(mJobService)
                    .jobFinished(any(), anyBoolean());
        });
        thread.start();

        // Wait a while for `thread` to block on waiting for the old job to exit.
        Utils.sleep(200);

        // The old job now exits, unblocking `thread`.
        jobBlocker.release();
        thread.join();

        // Give it 1s for `jobFinished` to be potentially called. Either `jobFinished` is called
        // before the new job is started, or it should not be called.
        assertThat(jobFinishedCalledAfterNewJobStarted.tryAcquire(1, TimeUnit.SECONDS)).isFalse();

        // The new job now exits.
        jobBlocker.release();

        // `jobFinished` is called for the new job.
        assertThat(jobFinishedCalledAfterNewJobStarted.tryAcquire(TIMEOUT_SEC, TimeUnit.SECONDS))
                .isTrue();
    }

    /** Verifies that `onStartJob` for an old job is ignored after the old job is unscheduled. */
    @Test
    public void testRace2() throws Exception {
        // An update arrives. A job is scheduled.
        mPreRebootDexoptJob.onUpdateReadyImpl(null /* otaSlot */);
        JobParameters oldParameters = mJobParameters;

        // The job scheduler starts the job. In the meantime, another update arrives. It's not
        // possible that `onStartJob` is called for the old job after `onUpdateReadyImpl` is called
        // because `onUpdateReadyImpl` unschedules the old job. However, since both calls acquire a
        // lock, the order of execution may be reversed. When this happens, the `onStartJob` request
        // should not succeed.
        mPreRebootDexoptJob.onUpdateReadyImpl(null /* otaSlot */);
        mPreRebootDexoptJob.onStartJobImpl(mJobService, oldParameters);
        assertThat(mPreRebootDexoptJob.hasRunningJob()).isFalse();

        // The job scheduler starts the new job. This request should succeed.
        mPreRebootDexoptJob.onStartJobImpl(mJobService, mJobParameters);
        assertThat(mPreRebootDexoptJob.hasRunningJob()).isTrue();
    }

    /**
     * Verifies that `onStopJob` for an old job is ignored after a new synchronous job is started.
     */
    @Test
    public void testRace3() throws Exception {
        Semaphore dexoptCancelled = new Semaphore(0);
        Semaphore jobExited = new Semaphore(0);
        when(mPreRebootDriver.run(any(), anyBoolean(), any())).thenAnswer(invocation -> {
            var cancellationSignal = invocation.<CancellationSignal>getArgument(2);
            cancellationSignal.setOnCancelListener(() -> dexoptCancelled.release());
            assertThat(dexoptCancelled.tryAcquire(TIMEOUT_SEC, TimeUnit.SECONDS)).isTrue();
            jobExited.release();
            return new PreRebootResult(Status.STATUS_FINISHED);
        });

        // An update arrives. A job is scheduled.
        mPreRebootDexoptJob.onUpdateReadyImpl(null /* otaSlot */);

        // The job scheduler starts the job.
        mPreRebootDexoptJob.onStartJobImpl(mJobService, mJobParameters);

        // Another update arrives, requesting a synchronous job run, replacing the old job. The new
        // job, which is synchronous, is started right after the old job is cancelled by
        // `onUpdateReadyStartNow`, before the job scheduler calls `onStartJob`.
        JobParameters oldParameters = mJobParameters;
        CompletableFuture<Void> future = mPreRebootDexoptJob.onUpdateReadyStartNow(
                null /* otaSlot */, true /* isUpdataEngineReady */);

        // The old job should be cancelled at this point.
        // This cannot be the new job having exited because jobs are serialized.
        assertThat(jobExited.tryAcquire()).isTrue();

        // The `onStopJob` call finally arrives. This call should be a no-op because the job has
        // already been cancelled by ourselves during the `onUpdateReadyStartNow` call above. It
        // should not cancel the new job.
        mPreRebootDexoptJob.onStopJobImpl(oldParameters);

        // The new job should not be cancelled.
        assertThat(jobExited.tryAcquire()).isFalse();

        // Now cancel the new job.
        mPreRebootDexoptJob.cancelGiven(future, false /* expectInterrupt */);

        // Now the new job should be cancelled.
        assertThat(jobExited.tryAcquire()).isTrue();
    }

    @Test
    public void testCheckStagedFilesAge() throws Exception {
        when(SystemProperties.getInt(eq("dalvik.vm.pr_dexopt_retention"), anyInt())).thenReturn(30);
        Duration createdAt = Duration.ofMillis(CURRENT_TIME_MS).minusDays(30).plusMillis(1);
        when(mArtd.checkPreRebootStagedFilesStatus())
                .thenReturn(TestingUtils.createPreRebootStagedFilesStatus(
                        false /* isCommittable */, createdAt.toMillis()));

        assertThat(mPreRebootDexoptJob.checkStagedFilesAge())
                .isEqualTo(new StagedFilesAge(
                        Duration.ofDays(30).minusMillis(1) /* age */, false /* isExpired */));
    }

    @Test
    public void testCheckStagedFilesAgeExpired() throws Exception {
        when(SystemProperties.getInt(eq("dalvik.vm.pr_dexopt_retention"), anyInt())).thenReturn(30);
        Duration createdAt = Duration.ofMillis(CURRENT_TIME_MS).minusDays(30);
        when(mArtd.checkPreRebootStagedFilesStatus())
                .thenReturn(TestingUtils.createPreRebootStagedFilesStatus(
                        false /* isCommittable */, createdAt.toMillis()));

        assertThat(mPreRebootDexoptJob.checkStagedFilesAge())
                .isEqualTo(new StagedFilesAge(Duration.ofDays(30) /* age */, true /* isExpired */));
    }

    @Test
    public void testCheckStagedFilesAgeMissing() throws Exception {
        lenient()
                .when(SystemProperties.getInt(eq("dalvik.vm.pr_dexopt_retention"), anyInt()))
                .thenReturn(30);
        when(mArtd.checkPreRebootStagedFilesStatus()).thenReturn(null);

        assertThat(mPreRebootDexoptJob.checkStagedFilesAge()).isEqualTo(null);
    }

    @Test
    public void testCheckStagedFilesAgeError() throws Exception {
        lenient()
                .when(SystemProperties.getInt(eq("dalvik.vm.pr_dexopt_retention"), anyInt()))
                .thenReturn(30);
        when(mArtd.checkPreRebootStagedFilesStatus()).thenThrow(ServiceSpecificException.class);

        assertThat(mPreRebootDexoptJob.checkStagedFilesAge()).isEqualTo(null);
    }
}
