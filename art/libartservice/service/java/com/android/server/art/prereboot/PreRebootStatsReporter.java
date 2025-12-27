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

package com.android.server.art.prereboot;

import android.annotation.NonNull;
import android.os.Build;

import androidx.annotation.RequiresApi;

import com.android.internal.annotations.VisibleForTesting;
import com.android.server.LocalManagerRegistry;
import com.android.server.art.ArtManagerLocal;
import com.android.server.art.ArtStatsLog;
import com.android.server.art.ArtdRefCache;
import com.android.server.art.AsLog;
import com.android.server.art.ReasonMapping;
import com.android.server.art.Utils;
import com.android.server.art.model.DexoptStatus;
import com.android.server.art.prereboot.PreRebootDriver.PreRebootResult;
import com.android.server.art.proto.PreRebootStats;
import com.android.server.art.proto.PreRebootStats.FailureReason;
import com.android.server.art.proto.PreRebootStats.JobRun;
import com.android.server.art.proto.PreRebootStats.JobType;
import com.android.server.art.proto.PreRebootStats.Status;
import com.android.server.pm.PackageManagerLocal;

import java.io.File;
import java.io.FileInputStream;
import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.nio.file.Files;
import java.nio.file.StandardCopyOption;
import java.util.HashSet;
import java.util.List;
import java.util.Objects;
import java.util.Set;
import java.util.concurrent.CompletableFuture;
import java.util.concurrent.Executor;
import java.util.concurrent.ForkJoinPool;
import java.util.function.Function;

/**
 * A helper class to report the Pre-reboot Dexopt metrics to StatsD.
 *
 * This class is not thread-safe.
 *
 * During Pre-reboot Dexopt, both the old version and the new version of this code is run. The old
 * version writes to disk first, and the new version writes to disk later. After reboot, the new
 * version loads from disk.
 *
 * @hide
 */
@RequiresApi(Build.VERSION_CODES.VANILLA_ICE_CREAM)
public class PreRebootStatsReporter {
    // Shorthands for those ultra long names in the stats proto generated code.
    public static final int END_STATUS_UNSPECIFIED =
            ArtStatsLog.PRE_REBOOT_DEXOPT_JOB_ENDED__ARTIFACTS_END_STATUS__END_STATUS_UNSPECIFIED;
    public static final int END_STATUS_COMMITTED =
            ArtStatsLog.PRE_REBOOT_DEXOPT_JOB_ENDED__ARTIFACTS_END_STATUS__END_STATUS_COMMITTED;
    public static final int END_STATUS_MISSING =
            ArtStatsLog.PRE_REBOOT_DEXOPT_JOB_ENDED__ARTIFACTS_END_STATUS__END_STATUS_MISSING;
    public static final int END_STATUS_ERROR =
            ArtStatsLog.PRE_REBOOT_DEXOPT_JOB_ENDED__ARTIFACTS_END_STATUS__END_STATUS_ERROR;
    public static final int END_STATUS_EXPIRED =
            ArtStatsLog.PRE_REBOOT_DEXOPT_JOB_ENDED__ARTIFACTS_END_STATUS__END_STATUS_EXPIRED;
    public static final int END_STATUS_OBSOLETE =
            ArtStatsLog.PRE_REBOOT_DEXOPT_JOB_ENDED__ARTIFACTS_END_STATUS__END_STATUS_OBSOLETE;
    public static final int END_STATUS_SUPERSEDED =
            ArtStatsLog.PRE_REBOOT_DEXOPT_JOB_ENDED__ARTIFACTS_END_STATUS__END_STATUS_SUPERSEDED;

    private static final String FILENAME = "/data/system/pre-reboot-stats.pb";

    @NonNull private final Injector mInjector;

    public PreRebootStatsReporter() {
        this(new Injector());
    }

    /** @hide */
    @VisibleForTesting
    public PreRebootStatsReporter(@NonNull Injector injector) {
        mInjector = injector;
    }

    public void recordJobScheduled(boolean isAsync, boolean isOtaUpdate) {
        PreRebootStats.Builder statsBuilder = PreRebootStats.newBuilder();
        statsBuilder.setStatus(Status.STATUS_SCHEDULED)
                .setJobType(isOtaUpdate ? JobType.JOB_TYPE_OTA : JobType.JOB_TYPE_MAINLINE);
        // Omit job_scheduled_timestamp_millis to indicate a synchronous job.
        if (isAsync) {
            statsBuilder.setJobScheduledTimestampMillis(mInjector.getCurrentTimeMillis());
        }
        save(statsBuilder);
    }

    public void recordJobNotScheduled(@NonNull Status reason, boolean isOtaUpdate) {
        Utils.check(reason == Status.STATUS_NOT_SCHEDULED_DISABLED
                || reason == Status.STATUS_NOT_SCHEDULED_JOB_SCHEDULER);
        PreRebootStats.Builder statsBuilder = PreRebootStats.newBuilder();
        statsBuilder.setStatus(reason).setJobType(
                isOtaUpdate ? JobType.JOB_TYPE_OTA : JobType.JOB_TYPE_MAINLINE);
        save(statsBuilder);
    }

    public void recordJobStarted() {
        PreRebootStats.Builder statsBuilder = load();
        if (statsBuilder.getStatus() == Status.STATUS_UNKNOWN) {
            // Failed to load, the error is already logged.
            return;
        }

        JobRun.Builder runBuilder =
                JobRun.newBuilder().setJobStartedTimestampMillis(mInjector.getCurrentTimeMillis());
        statsBuilder.setStatus(Status.STATUS_STARTED)
                .clearFailureReason()
                .addJobRuns(runBuilder)
                .setSkippedPackageCount(0)
                .setOptimizedPackageCount(0)
                .setFailedPackageCount(0)
                .setTotalPackageCount(0)
                // Some packages may have artifacts from a previously cancelled job, but we count
                // from scratch for simplicity.
                .setPackagesWithArtifactsBeforeRebootCount(0);
        save(statsBuilder);
    }

    public class ProgressSession {
        private @NonNull PreRebootStats.Builder mStatsBuilder = load();

        public void recordProgress(int skippedPackageCount, int optimizedPackageCount,
                int failedPackageCount, int totalPackageCount,
                int packagesWithArtifactsBeforeRebootCount) {
            if (mStatsBuilder.getStatus() == Status.STATUS_UNKNOWN) {
                // Failed to load, the error is already logged.
                return;
            }

            mStatsBuilder.setSkippedPackageCount(skippedPackageCount)
                    .setOptimizedPackageCount(optimizedPackageCount)
                    .setFailedPackageCount(failedPackageCount)
                    .setTotalPackageCount(totalPackageCount)
                    .setPackagesWithArtifactsBeforeRebootCount(
                            packagesWithArtifactsBeforeRebootCount);
            save(mStatsBuilder);
        }
    }

    public void recordJobEnded(@NonNull PreRebootResult result) {
        PreRebootStats.Builder statsBuilder = load();
        if (statsBuilder.getStatus() == Status.STATUS_UNKNOWN) {
            // Failed to load, the error is already logged.
            return;
        }

        List<JobRun> jobRuns = statsBuilder.getJobRunsList();
        Utils.check(jobRuns.size() > 0);
        JobRun lastRun = jobRuns.get(jobRuns.size() - 1);
        Utils.check(lastRun.getJobEndedTimestampMillis() == 0);

        JobRun.Builder runBuilder = JobRun.newBuilder(lastRun).setJobEndedTimestampMillis(
                mInjector.getCurrentTimeMillis());

        Utils.check(result.status() == Status.STATUS_FINISHED
                || result.status() == Status.STATUS_FAILED
                || result.status() == Status.STATUS_ABORTED_SYSTEM_REQUIREMENTS);

        statsBuilder.setStatus(result.status())
                .setFailureReason(result.status() == Status.STATUS_FAILED
                                ? result.failureReason()
                                : FailureReason.FAILURE_UNSPECIFIED)
                .setJobRuns(jobRuns.size() - 1, runBuilder);
        save(statsBuilder);
    }

    public class AfterRebootSession {
        private @NonNull Set<String> mPackagesWithArtifacts = new HashSet<>();
        private int mArtifactsEndStatus = END_STATUS_UNSPECIFIED;
        private long mArtifactsAgeMillis = 0;
        private boolean mExpectFound = true;

        public void recordPackageWithArtifacts(@NonNull String packageName) {
            mPackagesWithArtifacts.add(packageName);
        }

        public void recordArtifactsEndStatus(int status, long ageMillis) {
            mArtifactsEndStatus = status;
            mArtifactsAgeMillis = ageMillis;
        }

        public void setExpectFound(boolean value) {
            mExpectFound = value;
        }

        public void reportAsync() {
            new CompletableFuture()
                    .runAsync(this::report, mInjector.getExecutor())
                    .exceptionally(t -> {
                        AsLog.e("Failed to report stats", t);
                        return null;
                    });
        }

        public void report() {
            Utils.check(mArtifactsEndStatus != END_STATUS_UNSPECIFIED);

            PreRebootStats.Builder statsBuilder = load(mExpectFound);
            delete();

            if (statsBuilder.getStatus() == Status.STATUS_UNKNOWN) {
                // Job not scheduled, probably because Pre-reboot Dexopt is not enabled.
                return;
            }

            ArtManagerLocal artManagerLocal = mInjector.getArtManagerLocal();

            int packagesWithArtifactsUsableCount = 0;
            if (mArtifactsEndStatus == END_STATUS_COMMITTED) {
                // This takes some time (~3ms per package). It probably fine because we are running
                // asynchronously. Consider removing this in the future.
                try (var snapshot = mInjector.getPackageManagerLocal().withFilteredSnapshot();
                        var pin = mInjector.createArtdPin()) {
                    packagesWithArtifactsUsableCount =
                            (int) mPackagesWithArtifacts.stream()
                                    .map(packageName
                                            -> artManagerLocal.getDexoptStatus(
                                                    snapshot, packageName))
                                    .filter(status -> hasUsablePreRebootArtifacts(status))
                                    .count();
                }
            }

            List<JobRun> jobRuns = statsBuilder.getJobRunsList();
            // The total duration of all runs, or -1 if any run didn't end.
            long jobDurationMs = 0;
            for (JobRun run : jobRuns) {
                if (run.getJobEndedTimestampMillis() == 0) {
                    jobDurationMs = -1;
                    break;
                }
                jobDurationMs +=
                        run.getJobEndedTimestampMillis() - run.getJobStartedTimestampMillis();
            }
            if (jobRuns.size() == 0) {
                jobDurationMs = -1;
            }
            long jobLatencyMs =
                    (jobRuns.size() > 0 && statsBuilder.getJobScheduledTimestampMillis() > 0)
                    ? (jobRuns.get(0).getJobStartedTimestampMillis()
                              - statsBuilder.getJobScheduledTimestampMillis())
                    : -1;

            mInjector.writeStats(ArtStatsLog.PREREBOOT_DEXOPT_JOB_ENDED,
                    getStatusForStatsd(statsBuilder.getStatus()),
                    statsBuilder.getOptimizedPackageCount(), statsBuilder.getFailedPackageCount(),
                    statsBuilder.getSkippedPackageCount(), statsBuilder.getTotalPackageCount(),
                    jobDurationMs, jobLatencyMs, mPackagesWithArtifacts.size(),
                    packagesWithArtifactsUsableCount, jobRuns.size(),
                    statsBuilder.getPackagesWithArtifactsBeforeRebootCount(),
                    getJobTypeForStatsd(statsBuilder.getJobType()),
                    getFailureReasonForStatsd(statsBuilder.getFailureReason()), mArtifactsEndStatus,
                    mArtifactsAgeMillis);
        }
    }

    @VisibleForTesting
    public static int getStatusForStatsd(@NonNull Status status) {
        return switch (status) {
            case STATUS_UNKNOWN -> ArtStatsLog.PRE_REBOOT_DEXOPT_JOB_ENDED__STATUS__STATUS_UNKNOWN;
            case STATUS_SCHEDULED ->
                ArtStatsLog.PRE_REBOOT_DEXOPT_JOB_ENDED__STATUS__STATUS_SCHEDULED;
            case STATUS_STARTED -> ArtStatsLog.PRE_REBOOT_DEXOPT_JOB_ENDED__STATUS__STATUS_STARTED;
            case STATUS_FINISHED ->
                ArtStatsLog.PRE_REBOOT_DEXOPT_JOB_ENDED__STATUS__STATUS_FINISHED;
            case STATUS_FAILED -> ArtStatsLog.PRE_REBOOT_DEXOPT_JOB_ENDED__STATUS__STATUS_FAILED;
            case STATUS_CANCELLED ->
                ArtStatsLog.PRE_REBOOT_DEXOPT_JOB_ENDED__STATUS__STATUS_CANCELLED;
            case STATUS_ABORTED_SYSTEM_REQUIREMENTS ->
                ArtStatsLog.PRE_REBOOT_DEXOPT_JOB_ENDED__STATUS__STATUS_ABORTED_SYSTEM_REQUIREMENTS;
            case STATUS_NOT_SCHEDULED_DISABLED ->
                ArtStatsLog.PRE_REBOOT_DEXOPT_JOB_ENDED__STATUS__STATUS_NOT_SCHEDULED_DISABLED;
            case STATUS_NOT_SCHEDULED_JOB_SCHEDULER ->
                ArtStatsLog.PRE_REBOOT_DEXOPT_JOB_ENDED__STATUS__STATUS_NOT_SCHEDULED_JOB_SCHEDULER;
            default -> throw new IllegalStateException("Unknown status: " + status.getNumber());
        };
    }

    private static int getJobTypeForStatsd(@NonNull JobType jobType) {
        return switch (jobType) {
            case JOB_TYPE_UNKNOWN ->
                ArtStatsLog.PRE_REBOOT_DEXOPT_JOB_ENDED__JOB_TYPE__JOB_TYPE_UNKNOWN;
            case JOB_TYPE_OTA -> ArtStatsLog.PRE_REBOOT_DEXOPT_JOB_ENDED__JOB_TYPE__JOB_TYPE_OTA;
            case JOB_TYPE_MAINLINE ->
                ArtStatsLog.PRE_REBOOT_DEXOPT_JOB_ENDED__JOB_TYPE__JOB_TYPE_MAINLINE;
            default -> throw new IllegalStateException("Unknown job type: " + jobType.getNumber());
        };
    }

    private static int getFailureReasonForStatsd(@NonNull FailureReason failureReason) {
        return switch (failureReason) {
            case FAILURE_UNSPECIFIED ->
                ArtStatsLog.PRE_REBOOT_DEXOPT_JOB_ENDED__FAILURE_REASON__FAILURE_UNSPECIFIED;
            case FAILURE_UPDATE_ENGINE ->
                ArtStatsLog.PRE_REBOOT_DEXOPT_JOB_ENDED__FAILURE_REASON__FAILURE_UPDATE_ENGINE;
            case FAILURE_CHROOT_SETUP ->
                ArtStatsLog.PRE_REBOOT_DEXOPT_JOB_ENDED__FAILURE_REASON__FAILURE_CHROOT_SETUP;
            case FAILURE_CLASS_LOADER ->
                ArtStatsLog.PRE_REBOOT_DEXOPT_JOB_ENDED__FAILURE_REASON__FAILURE_CLASS_LOADER;
            default ->
                throw new IllegalStateException(
                        "Unknown failure reason: " + failureReason.getNumber());
        };
    }

    private static boolean hasUsablePreRebootArtifacts(@NonNull DexoptStatus status) {
        // For simplicity, we consider all artifacts of a package usable if we see at least one
        // `REASON_PRE_REBOOT_DEXOPT` because it's not easy to know which files are committed.
        return status.getDexContainerFileDexoptStatuses().stream().anyMatch(fileStatus
                -> fileStatus.getCompilationReason().equals(
                        ReasonMapping.REASON_PRE_REBOOT_DEXOPT));
    }

    @NonNull
    private PreRebootStats.Builder load() {
        return load(true /* expectFound */);
    }

    @NonNull
    private PreRebootStats.Builder load(boolean expectFound) {
        PreRebootStats.Builder statsBuilder = PreRebootStats.newBuilder();
        try (InputStream in = new FileInputStream(mInjector.getFilename())) {
            statsBuilder.mergeFrom(in);
        } catch (IOException e) {
            // Nothing else we can do but to start from scratch.
            if (expectFound || !(e instanceof FileNotFoundException)) {
                AsLog.e("Failed to load pre-reboot stats", e);
            }
        }
        return statsBuilder;
    }

    private void save(@NonNull PreRebootStats.Builder statsBuilder) {
        var file = new File(mInjector.getFilename());
        File tempFile = null;
        try {
            tempFile = File.createTempFile(file.getName(), null /* suffix */, file.getParentFile());
            try (OutputStream out = new FileOutputStream(tempFile.getPath())) {
                statsBuilder.build().writeTo(out);
            }
            Files.move(tempFile.toPath(), file.toPath(), StandardCopyOption.REPLACE_EXISTING,
                    StandardCopyOption.ATOMIC_MOVE);
        } catch (IOException e) {
            AsLog.e("Failed to save pre-reboot stats", e);
        } finally {
            Utils.deleteIfExistsSafe(tempFile);
        }
    }

    private void delete() {
        Utils.deleteIfExistsSafe(new File(mInjector.getFilename()));
    }

    /**
     * Injector pattern for testing purpose.
     *
     * @hide
     */
    @VisibleForTesting
    public static class Injector {
        @NonNull
        public String getFilename() {
            return FILENAME;
        }

        public long getCurrentTimeMillis() {
            return System.currentTimeMillis();
        }

        @NonNull
        public PackageManagerLocal getPackageManagerLocal() {
            return Objects.requireNonNull(
                    LocalManagerRegistry.getManager(PackageManagerLocal.class));
        }

        @NonNull
        public ArtManagerLocal getArtManagerLocal() {
            return Objects.requireNonNull(LocalManagerRegistry.getManager(ArtManagerLocal.class));
        }

        @NonNull
        public ArtdRefCache.Pin createArtdPin() {
            return ArtdRefCache.getInstance().new Pin();
        }

        // Wrap the static void method to make it easier to mock. There is no good way to mock a
        // method that is both void and static, due to the poor design of Mockito API.
        public void writeStats(int code, int status, int optimizedPackageCount,
                int failedPackageCount, int skippedPackageCount, int totalPackageCount,
                long jobDurationMillis, long jobLatencyMillis,
                int packagesWithArtifactsAfterRebootCount,
                int packagesWithArtifactsUsableAfterRebootCount, int jobRunCount,
                int packagesWithArtifactsBeforeRebootCount, int jobType, int failureReason,
                int artifactsEndStatus, long artifactsAgeMillis) {
            ArtStatsLog.write(code, status, optimizedPackageCount, failedPackageCount,
                    skippedPackageCount, totalPackageCount, jobDurationMillis, jobLatencyMillis,
                    packagesWithArtifactsAfterRebootCount,
                    packagesWithArtifactsUsableAfterRebootCount, jobRunCount,
                    packagesWithArtifactsBeforeRebootCount, jobType, failureReason,
                    artifactsEndStatus, artifactsAgeMillis);
        }

        @NonNull
        public Executor getExecutor() {
            return ForkJoinPool.commonPool();
        }
    }
}
