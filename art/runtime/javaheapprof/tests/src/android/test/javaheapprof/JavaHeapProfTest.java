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
 * limitations under the License
 */

package android.test.javaheapprof;

import android.util.Log;

import androidx.test.filters.LargeTest;
import androidx.test.filters.MediumTest;
import androidx.test.filters.SmallTest;

import com.android.helpers.PerfettoHelper;

import org.junit.After;
import org.junit.AfterClass;
import org.junit.Assert;
import org.junit.Before;
import org.junit.BeforeClass;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.junit.runners.JUnit4;

import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStreamReader;
import java.util.HashMap;
import java.util.Map;

@RunWith(JUnit4.class)
public class JavaHeapProfTest {
    private static final String TAG = JavaHeapProfTest.class.getSimpleName();

    private static final int NUM_THREADS = 100;
    private static final int NUM_ITERS_PER_THREAD = 1000;
    private static final int NUM_BYTES_PER_ALLOC = 1024;
    private static final int SAMPLING_INTERVAL_BYTES = 4096;
    private static final int MULTIPLIER = 3;
    private static final int LARGE_MULTIPLIER = 128;
    private static final double STDDEV_ALLOWANCE = 3;
    private static final String TEST_DIR = "/data/local/android.test.javaheapprof";
    private static final String TRACE_CONFIG = "javaheapprof.pbtxt";
    private static final String TRACE_OUTPUT = TEST_DIR + "/trace.pftrace";
    private static final String TRACE_PROCESSOR = TEST_DIR + "/trace_processor_shell";
    private static final String TRACE_QUERY = TEST_DIR + "/query.sql";

    static class Allocator implements Runnable {
        Object mLastAllocated = null;

        void alloc(int multiplier) {
            // Arrays take up 12 bytes, so subtract that out.
            mLastAllocated = new byte[multiplier * NUM_BYTES_PER_ALLOC - 12];
        }

        void alloc() {
            alloc(1);
        }

        void B0() {
            alloc(LARGE_MULTIPLIER);
        };

        void S0() {
            alloc();
        };
        void S1() {
            alloc();
        };
        void S2() {
            alloc();
        };
        void S3() {
            alloc();
        };

        void M0() {
            alloc();
        };
        void M1() {
            alloc();
        };
        void M2() {
            alloc();
        };
        void M3() {
            alloc();
        };

        void N0() {
            alloc(MULTIPLIER);
        };
        void N1() {
            alloc(MULTIPLIER);
        };
        void N2() {
            alloc(MULTIPLIER);
        };
        void N3() {
            alloc(MULTIPLIER);
        };

        void E0() {
            alloc();
        };
        void E1() {
            alloc();
        };
        void E2() {
            alloc();
        };
        void E3() {
            alloc();
        };

        public void run() {
            // Allocate from a few different callsites at the start of the thread,
            // to make sure we don't bias towards the first allocation in the
            // thread.
            S0();
            S1();
            S2();
            S3();

            // Allocate from a few different callsites in a loop, to make sure we
            // don't bias towards some in a pattern and that we can distinguish call
            // sites with different allocation sizes.
            for (int i = 0; i < NUM_ITERS_PER_THREAD; ++i) {
                M0();
                M1();
                M2();
                M3();
            }

            // Do the same, but with a different size allocation to check we tell
            // the difference between different size allocations properly.
            for (int i = 0; i < NUM_ITERS_PER_THREAD; ++i) {
                N0();
                N1();
                N2();
                N3();
            }

            // Allocate from a large allocation to test tlab vs. non-tlab.
            B0();

            // Allocate from a few different callsites at the end of the thread,
            // to make sure we don't bias against the last allocations in the
            // thread.
            E0();
            E1();
            E2();
            E3();
        }
    }

    private static class Results {
        private final Map<String, Long> mResults;
        private final StringBuilder mMessage = new StringBuilder();
        private double mScaleFactor = 1.0;
        private boolean mFailed = false;

        private Results(Map<String, Long> results) {
            mResults = results;
            mMessage.append(String.format("%8s: %10s %10s %10s %12s %8s %8s\n", "<name>",
                    "expected", "actual", "normal", "Δ abs", "Δ rel", "Δ stddev"));
        }

        private void expect(String name, long expected, long actual) {
            // There are known issues with the absolute magnitude of
            // reporting. Apply the scale factor to normalize actual reported
            // values so we can separate the question of whether the reporting
            // is relatively or absolutely correct.
            long normal = (long) (mScaleFactor * actual);

            // In theory random sampling decides to sample each individual byte
            // randomly with probability 1 / SAMPLING_INTERVAL_BYTES. The sum of
            // bytes sampled is a binomial distribution. We want to check that
            // the sum is within a few standard deviations of what we expect.
            double p = 1.0 / (double) SAMPLING_INTERVAL_BYTES;
            double variance = (double) expected * p * (1.0 - p);
            double stddev = Math.sqrt(variance);

            // Try to be within 3 standard deviations for now, which ought to
            // cover 99.7% of cases.
            double sampleMargin = STDDEV_ALLOWANCE * stddev;

            // The random sampling should be artificially scaling up the number
            // of sampled bytes by SAMPLING_INTERVAL_BYTES. Scale up the margin
            // to match.
            long margin = (long) (sampleMargin * (double) SAMPLING_INTERVAL_BYTES);

            // Log the results to facilitate debug and analysis.
            long abs = normal - expected;
            double rel = (double) abs / (double) expected;
            double std = (double) abs * STDDEV_ALLOWANCE / (double) margin;
            mMessage.append(String.format("%8s: %10d %10d %10d %12d %8.2f %8.2f\n", name, expected,
                    actual, normal, abs, rel, std));

            if (Math.abs(abs) > margin) {
                mFailed = true;
            }
        }

        // Tell the results about the expected total number of allocations.
        // This should be called once before any calls to expectFrame.
        public void expectTotal(long expectedTotal) {
            long total = 0;
            for (Long value : mResults.values()) {
                total += value;
            }

            expect("total", expectedTotal, total);
            mScaleFactor = (double) expectedTotal / (double) total;
        }

        public void expectFrame(String key, long expected) {
            Long actual = mResults.get(key);
            Assert.assertNotNull(key + " not found", actual);
            expect(key, expected, actual);
        }

        public void assertOkay() {
            String msg = mMessage.toString();
            Log.i(TAG, msg);
            if (mFailed) {
                Assert.fail(
                        String.format("Expected results not within %.2f stddev of expected:\n %s",
                                STDDEV_ALLOWANCE, msg));
            }
        }

        public static Results query() throws IOException {
            Runtime runtime = Runtime.getRuntime();
            Process process =
                    runtime.exec(TRACE_PROCESSOR + " -q " + TRACE_QUERY + " " + TRACE_OUTPUT);

            Map<String, Long> results = new HashMap<>();
            BufferedReader reader =
                    new BufferedReader(new InputStreamReader(process.getInputStream()));
            final String prefix = "\"android.test.javaheapprof.JavaHeapProfTest$Allocator.";
            for (String line; (line = reader.readLine()) != null;) {
                if (line.startsWith(prefix)) {
                    String key =
                            line.substring(prefix.length(), line.indexOf('"', prefix.length()));
                    Long value = Long.parseLong(line.substring(line.indexOf(',') + 1));
                    results.put(key, value);
                }
            }
            return new Results(results);
        }
    }

    @Test
    public void testJavaHeapProf() throws InterruptedException, IOException {
        PerfettoHelper perfetto = new PerfettoHelper();
        perfetto.setPerfettoConfigRootDir("/data/misc/perfetto-configs/");
        perfetto.startCollectingFromConfigFile(TRACE_CONFIG, true);

        Allocator allocator = new Allocator();

        Thread[] threads = new Thread[NUM_THREADS];
        for (int i = 0; i < NUM_THREADS; ++i) {
            threads[i] = new Thread(allocator);
            threads[i].start();
        }

        for (int i = 0; i < NUM_THREADS; ++i) {
            threads[i].join();
        }

        perfetto.stopCollecting(0, TRACE_OUTPUT);
        Results results = Results.query();

        long perThread = NUM_THREADS * NUM_BYTES_PER_ALLOC;
        long perIter = NUM_THREADS * NUM_ITERS_PER_THREAD * NUM_BYTES_PER_ALLOC;
        long total = 4 * perThread + 4 * perIter + 4 * MULTIPLIER * perIter
                + 1 * LARGE_MULTIPLIER * perThread + 4 * perThread;

        results.expectTotal(total);
        results.expectFrame("S0", perThread);
        results.expectFrame("S1", perThread);
        results.expectFrame("S2", perThread);
        results.expectFrame("S3", perThread);
        results.expectFrame("M0", perIter);
        results.expectFrame("M1", perIter);
        results.expectFrame("M2", perIter);
        results.expectFrame("M3", perIter);
        results.expectFrame("N0", MULTIPLIER * perIter);
        results.expectFrame("N1", MULTIPLIER * perIter);
        results.expectFrame("N2", MULTIPLIER * perIter);
        results.expectFrame("N3", MULTIPLIER * perIter);
        results.expectFrame("B0", LARGE_MULTIPLIER * perThread);
        results.expectFrame("E0", perThread);
        results.expectFrame("E1", perThread);
        results.expectFrame("E2", perThread);
        results.expectFrame("E3", perThread);
        results.assertOkay();
    }
}
