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
 * limitations under the License
 */

package com.android.server.art;

import static com.android.server.art.BackgroundDexoptJob.JobType.BG_DEXOPT;
import static com.android.server.art.BackgroundDexoptJob.JobType.POST_UNATTENDED_REBOOT;

import static com.google.common.truth.Truth.assertThat;

import static org.mockito.Mockito.any;
import static org.mockito.Mockito.anyBoolean;
import static org.mockito.Mockito.anyInt;
import static org.mockito.Mockito.eq;
import static org.mockito.Mockito.lenient;
import static org.mockito.Mockito.mock;
import static org.mockito.Mockito.never;
import static org.mockito.Mockito.same;
import static org.mockito.Mockito.times;
import static org.mockito.Mockito.verify;
import static org.mockito.Mockito.when;

import android.app.job.JobInfo;
import android.app.job.JobParameters;
import android.app.job.JobScheduler;
import android.os.CancellationSignal;
import android.os.SystemProperties;

import androidx.test.filters.SmallTest;
import androidx.test.runner.AndroidJUnit4;

import com.android.server.art.BackgroundDexoptJob.CompletedResult;
import com.android.server.art.BackgroundDexoptJob.FatalErrorResult;
import com.android.server.art.BackgroundDexoptJob.Result;
import com.android.server.art.model.ArtFlags;
import com.android.server.art.model.Config;
import com.android.server.art.model.Config.Callback;
import com.android.server.art.model.DexoptResult;
import com.android.server.art.model.DexoptResult.DexoptResultStatus;
import com.android.server.art.model.DexoptResult.PackageDexoptResult;
import com.android.server.art.testing.StaticMockitoRule;
import com.android.server.pm.PackageManagerLocal;

import org.junit.Before;
import org.junit.Rule;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.mockito.ArgumentCaptor;
import org.mockito.Mock;

import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.concurrent.Future;
import java.util.concurrent.Semaphore;
import java.util.concurrent.TimeUnit;

@SmallTest
@RunWith(AndroidJUnit4.class)
public class BackgroundDexoptJobTest {
    private static final long TIMEOUT_SEC = 10;

    @Rule
    public StaticMockitoRule mockitoRule =
            new StaticMockitoRule(SystemProperties.class, BackgroundDexoptJobService.class);

    @Mock private BackgroundDexoptJob.Injector mInjector;
    @Mock private ArtManagerLocal mArtManagerLocal;
    @Mock private PackageManagerLocal mPackageManagerLocal;
    @Mock private PackageManagerLocal.FilteredSnapshot mSnapshot1, mSnapshot2;
    @Mock private JobScheduler mJobScheduler;
    @Mock private BackgroundDexoptJobService mJobService;
    @Mock private JobParameters mJobParameters;
    private Config mConfig;
    private BackgroundDexoptJob mBackgroundDexoptJob;
    private Semaphore mJobFinishedCalled = new Semaphore(0);
    private Map<Integer, DexoptResult> mDexoptResultByPass;

    @Before
    public void setUp() throws Exception {
        lenient()
                .when(SystemProperties.getBoolean(eq("pm.dexopt.disable_bg_dexopt"), anyBoolean()))
                .thenReturn(false);

        mConfig = new Config();

        lenient().when(mInjector.getArtManagerLocal()).thenReturn(mArtManagerLocal);
        lenient().when(mInjector.getPackageManagerLocal()).thenReturn(mPackageManagerLocal);
        lenient().when(mInjector.getConfig()).thenReturn(mConfig);
        lenient().when(mInjector.getJobScheduler()).thenReturn(mJobScheduler);

        mBackgroundDexoptJob = new BackgroundDexoptJob(mInjector);
        lenient()
                .when(BackgroundDexoptJobService.getJob(BG_DEXOPT.getJobId()))
                .thenReturn(mBackgroundDexoptJob);
        lenient()
                .when(BackgroundDexoptJobService.getJob(POST_UNATTENDED_REBOOT.getJobId()))
                .thenReturn(mBackgroundDexoptJob);

        lenient()
                .doAnswer(invocation -> {
                    mJobFinishedCalled.release();
                    return null;
                })
                .when(mJobService)
                .jobFinished(any(), anyBoolean());

        lenient()
                .when(mJobParameters.getStopReason())
                .thenReturn(JobParameters.STOP_REASON_UNDEFINED);

        mDexoptResultByPass = new HashMap<>();
    }

    @Test
    public void testStartBgDexopt() {
        when(mPackageManagerLocal.withFilteredSnapshot())
                .thenReturn(mSnapshot1)
                .thenReturn(mSnapshot2);

        when(mArtManagerLocal.dexoptPackages(
                     same(mSnapshot1), eq(ReasonMapping.REASON_BG_DEXOPT), any(), any(), any()))
                .thenReturn(mDexoptResultByPass);

        Result result = Utils.getFuture(mBackgroundDexoptJob.start(BG_DEXOPT));
        assertThat(result).isInstanceOf(CompletedResult.class);
        assertThat(((CompletedResult) result).dexoptResultByPass()).isEqualTo(mDexoptResultByPass);

        verify(mArtManagerLocal).cleanup(same(mSnapshot2));
    }

    @Test
    public void testStartPostUr() {
        when(mPackageManagerLocal.withFilteredSnapshot()).thenReturn(mSnapshot1);

        when(mArtManagerLocal.dexoptPackages(same(mSnapshot1),
                     eq(ReasonMapping.REASON_POST_UNATTENDED_REBOOT), any(), any(), any()))
                .thenReturn(mDexoptResultByPass);

        Result result = Utils.getFuture(mBackgroundDexoptJob.start(POST_UNATTENDED_REBOOT));
        assertThat(result).isInstanceOf(CompletedResult.class);
        assertThat(((CompletedResult) result).dexoptResultByPass()).isEqualTo(mDexoptResultByPass);

        verify(mArtManagerLocal, never()).cleanup(any());
    }

    @Test
    public void testStartAlreadyRunning() {
        Semaphore dexoptDone = new Semaphore(0);
        when(mArtManagerLocal.dexoptPackages(any(), any(), any(), any(), any()))
                .thenAnswer(invocation -> {
                    assertThat(dexoptDone.tryAcquire(TIMEOUT_SEC, TimeUnit.SECONDS)).isTrue();
                    return mDexoptResultByPass;
                });

        Future<Result> future1 = mBackgroundDexoptJob.start(POST_UNATTENDED_REBOOT);
        Future<Result> future2 = mBackgroundDexoptJob.start(BG_DEXOPT);
        assertThat(future1).isSameInstanceAs(future2);

        dexoptDone.release();
        Utils.getFuture(future1);

        verify(mArtManagerLocal, times(1)).dexoptPackages(any(), any(), any(), any(), any());
    }

    @Test
    public void testStartAnother() {
        when(mArtManagerLocal.dexoptPackages(any(), any(), any(), any(), any()))
                .thenReturn(mDexoptResultByPass);

        Future<Result> future1 = mBackgroundDexoptJob.start(POST_UNATTENDED_REBOOT);
        Utils.getFuture(future1);
        Future<Result> future2 = mBackgroundDexoptJob.start(BG_DEXOPT);
        Utils.getFuture(future2);
        assertThat(future1).isNotSameInstanceAs(future2);
    }

    @Test
    public void testStartFatalError() {
        when(mArtManagerLocal.dexoptPackages(any(), any(), any(), any(), any()))
                .thenThrow(IllegalStateException.class);

        Result result = Utils.getFuture(mBackgroundDexoptJob.start(BG_DEXOPT));
        assertThat(result).isInstanceOf(FatalErrorResult.class);
    }

    @Test
    public void testStartIgnoreDisabled() {
        lenient()
                .when(SystemProperties.getBoolean(eq("pm.dexopt.disable_bg_dexopt"), anyBoolean()))
                .thenReturn(true);

        when(mArtManagerLocal.dexoptPackages(any(), any(), any(), any(), any()))
                .thenReturn(mDexoptResultByPass);

        // The `start` method should ignore the system property. The system property is for
        // `schedule`.
        Utils.getFuture(mBackgroundDexoptJob.start(BG_DEXOPT));
    }

    @Test
    public void testCancel() {
        Semaphore dexoptCancelled = new Semaphore(0);
        when(mArtManagerLocal.dexoptPackages(any(), any(), any(), any(), any()))
                .thenAnswer(invocation -> {
                    assertThat(dexoptCancelled.tryAcquire(TIMEOUT_SEC, TimeUnit.SECONDS)).isTrue();
                    var cancellationSignal = invocation.<CancellationSignal>getArgument(2);
                    assertThat(cancellationSignal.isCanceled()).isTrue();
                    return mDexoptResultByPass;
                });

        Future<Result> future = mBackgroundDexoptJob.start(BG_DEXOPT);
        mBackgroundDexoptJob.cancel();
        dexoptCancelled.release();
        Utils.getFuture(future);
    }

    @Test
    public void testScheduleBgDexopt() {
        var captor = ArgumentCaptor.forClass(JobInfo.class);
        when(mJobScheduler.schedule(captor.capture())).thenReturn(JobScheduler.RESULT_SUCCESS);

        assertThat(mBackgroundDexoptJob.schedule(BG_DEXOPT)).isEqualTo(ArtFlags.SCHEDULE_SUCCESS);

        JobInfo jobInfo = captor.getValue();
        assertThat(jobInfo.getId()).isEqualTo(BG_DEXOPT.getJobId());
        assertThat(jobInfo.getIntervalMillis()).isEqualTo(BackgroundDexoptJob.JOB_INTERVAL_MS);
        assertThat(jobInfo.isRequireDeviceIdle()).isTrue();
        assertThat(jobInfo.isRequireCharging()).isTrue();
        assertThat(jobInfo.isRequireBatteryNotLow()).isTrue();
        assertThat(jobInfo.isRequireStorageNotLow()).isFalse();
    }

    @Test
    public void testScheduleBgDexoptDisabled() {
        when(SystemProperties.getBoolean(eq("pm.dexopt.disable_bg_dexopt"), anyBoolean()))
                .thenReturn(true);

        assertThat(mBackgroundDexoptJob.schedule(BG_DEXOPT))
                .isEqualTo(ArtFlags.SCHEDULE_DISABLED_BY_SYSPROP);

        verify(mJobScheduler, never()).schedule(any());
    }

    @Test
    public void testScheduleBgDexoptOverride() {
        mConfig.setScheduleBackgroundDexoptJobCallback(Runnable::run, builder -> {
            builder.setRequiresBatteryNotLow(false);
            builder.setPriority(JobInfo.PRIORITY_LOW);
        });

        var captor = ArgumentCaptor.forClass(JobInfo.class);
        when(mJobScheduler.schedule(captor.capture())).thenReturn(JobScheduler.RESULT_SUCCESS);

        assertThat(mBackgroundDexoptJob.schedule(BG_DEXOPT)).isEqualTo(ArtFlags.SCHEDULE_SUCCESS);

        JobInfo jobInfo = captor.getValue();
        assertThat(jobInfo.getIntervalMillis()).isEqualTo(BackgroundDexoptJob.JOB_INTERVAL_MS);
        assertThat(jobInfo.isRequireDeviceIdle()).isTrue();
        assertThat(jobInfo.isRequireCharging()).isTrue();
        assertThat(jobInfo.isRequireBatteryNotLow()).isFalse();
        assertThat(jobInfo.getPriority()).isEqualTo(JobInfo.PRIORITY_LOW);
    }

    @Test
    public void testScheduleBgDexoptOverrideCleared() {
        mConfig.setScheduleBackgroundDexoptJobCallback(
                Runnable::run, builder -> { builder.setRequiresBatteryNotLow(false); });
        mConfig.clearScheduleBackgroundDexoptJobCallback();

        var captor = ArgumentCaptor.forClass(JobInfo.class);
        when(mJobScheduler.schedule(captor.capture())).thenReturn(JobScheduler.RESULT_SUCCESS);

        assertThat(mBackgroundDexoptJob.schedule(BG_DEXOPT)).isEqualTo(ArtFlags.SCHEDULE_SUCCESS);

        JobInfo jobInfo = captor.getValue();
        assertThat(jobInfo.isRequireBatteryNotLow()).isTrue();
    }

    @Test(expected = IllegalStateException.class)
    public void testScheduleBgDexoptOverrideStorageNotLow() {
        mConfig.setScheduleBackgroundDexoptJobCallback(
                Runnable::run, builder -> { builder.setRequiresStorageNotLow(true); });

        mBackgroundDexoptJob.schedule(BG_DEXOPT);
    }

    @Test
    public void testSchedulePostUr() {
        var captor = ArgumentCaptor.forClass(JobInfo.class);
        when(mJobScheduler.schedule(captor.capture())).thenReturn(JobScheduler.RESULT_SUCCESS);

        assertThat(mBackgroundDexoptJob.schedule(POST_UNATTENDED_REBOOT))
                .isEqualTo(ArtFlags.SCHEDULE_SUCCESS);

        JobInfo jobInfo = captor.getValue();
        assertThat(jobInfo.getId()).isEqualTo(POST_UNATTENDED_REBOOT.getJobId());
        assertThat(jobInfo.isPeriodic()).isFalse();
        assertThat(jobInfo.isRequireDeviceIdle()).isFalse();
        assertThat(jobInfo.isRequireCharging()).isTrue();
        assertThat(jobInfo.isRequireBatteryNotLow()).isTrue();
        assertThat(jobInfo.isRequireStorageNotLow()).isFalse();
    }

    @Test
    public void testSchedulePostUrDisabled() {
        when(SystemProperties.getBoolean(eq("pm.dexopt.disable_bg_dexopt"), anyBoolean()))
                .thenReturn(true);

        assertThat(mBackgroundDexoptJob.schedule(POST_UNATTENDED_REBOOT))
                .isEqualTo(ArtFlags.SCHEDULE_DISABLED_BY_SYSPROP);

        verify(mJobScheduler, never()).schedule(any());
    }

    @Test
    public void testSchedulePostUrNoOverride() {
        mConfig.setScheduleBackgroundDexoptJobCallback(
                Runnable::run, builder -> { builder.setRequiresBatteryNotLow(false); });

        var captor = ArgumentCaptor.forClass(JobInfo.class);
        when(mJobScheduler.schedule(captor.capture())).thenReturn(JobScheduler.RESULT_SUCCESS);

        assertThat(mBackgroundDexoptJob.schedule(POST_UNATTENDED_REBOOT))
                .isEqualTo(ArtFlags.SCHEDULE_SUCCESS);

        JobInfo jobInfo = captor.getValue();
        assertThat(jobInfo.isRequireBatteryNotLow()).isTrue();
    }

    @Test
    public void testUnscheduleBgDexopt() {
        mBackgroundDexoptJob.unschedule(BG_DEXOPT);
        verify(mJobScheduler).cancel(BG_DEXOPT.getJobId());
    }

    @Test
    public void testUnschedulePostUr() {
        mBackgroundDexoptJob.unschedule(POST_UNATTENDED_REBOOT);
        verify(mJobScheduler).cancel(POST_UNATTENDED_REBOOT.getJobId());
    }

    @Test
    public void testWantsRescheduleFalsePerformed() throws Exception {
        DexoptResult downgradeResult = createDexoptResultWithStatus(DexoptResult.DEXOPT_PERFORMED);
        mDexoptResultByPass.put(ArtFlags.PASS_DOWNGRADE, downgradeResult);
        DexoptResult mainResult = createDexoptResultWithStatus(DexoptResult.DEXOPT_PERFORMED);
        mDexoptResultByPass.put(ArtFlags.PASS_MAIN, mainResult);
        when(mArtManagerLocal.dexoptPackages(any(), any(), any(), any(), any()))
                .thenReturn(mDexoptResultByPass);

        when(mJobParameters.getJobId()).thenReturn(BG_DEXOPT.getJobId());
        mBackgroundDexoptJob.onStartJob(mJobService, mJobParameters);
        assertThat(mJobFinishedCalled.tryAcquire(TIMEOUT_SEC, TimeUnit.SECONDS)).isTrue();

        verify(mJobService).jobFinished(any(), eq(false) /* wantsReschedule */);
    }

    @Test
    public void testWantsRescheduleFalseFatalError() throws Exception {
        when(mArtManagerLocal.dexoptPackages(any(), any(), any(), any(), any()))
                .thenThrow(RuntimeException.class);

        when(mJobParameters.getJobId()).thenReturn(BG_DEXOPT.getJobId());
        mBackgroundDexoptJob.onStartJob(mJobService, mJobParameters);
        assertThat(mJobFinishedCalled.tryAcquire(TIMEOUT_SEC, TimeUnit.SECONDS)).isTrue();

        verify(mJobService).jobFinished(any(), eq(false) /* wantsReschedule */);
    }

    @Test
    public void testWantsRescheduleFalsePostUr() throws Exception {
        DexoptResult mainResult = createDexoptResultWithStatus(DexoptResult.DEXOPT_CANCELLED);
        mDexoptResultByPass.put(ArtFlags.PASS_MAIN, mainResult);
        when(mArtManagerLocal.dexoptPackages(any(), any(), any(), any(), any()))
                .thenReturn(mDexoptResultByPass);

        when(mJobParameters.getJobId()).thenReturn(POST_UNATTENDED_REBOOT.getJobId());
        mBackgroundDexoptJob.onStartJob(mJobService, mJobParameters);
        assertThat(mJobFinishedCalled.tryAcquire(TIMEOUT_SEC, TimeUnit.SECONDS)).isTrue();

        verify(mJobService).jobFinished(any(), eq(false) /* wantsReschedule */);
    }

    @Test
    public void testWantsRescheduleTrue() throws Exception {
        DexoptResult downgradeResult = createDexoptResultWithStatus(DexoptResult.DEXOPT_PERFORMED);
        mDexoptResultByPass.put(ArtFlags.PASS_DOWNGRADE, downgradeResult);
        DexoptResult mainResult = createDexoptResultWithStatus(DexoptResult.DEXOPT_CANCELLED);
        mDexoptResultByPass.put(ArtFlags.PASS_MAIN, mainResult);
        when(mArtManagerLocal.dexoptPackages(any(), any(), any(), any(), any()))
                .thenReturn(mDexoptResultByPass);

        when(mJobParameters.getJobId()).thenReturn(BG_DEXOPT.getJobId());
        mBackgroundDexoptJob.onStartJob(mJobService, mJobParameters);
        assertThat(mJobFinishedCalled.tryAcquire(TIMEOUT_SEC, TimeUnit.SECONDS)).isTrue();

        verify(mJobService).jobFinished(any(), eq(true) /* wantsReschedule */);
    }

    private DexoptResult createDexoptResultWithStatus(@DexoptResultStatus int status) {
        return DexoptResult.create("compiler-filter", "reason",
                List.of(PackageDexoptResult.create(
                        "package-name", List.of() /* dexContainerFileDexoptResults */, status)),
                null /* overallStatus */);
    }
}
