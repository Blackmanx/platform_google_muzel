/*
 * Copyright (C) 2025 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#include <bpf_kernelwakelockduration.h>

#include <BpfSyscallWrappers.h>
#include <android-base/unique_fd.h>
#include <android_bpfprogs_flags.h>
#include <gtest/gtest.h>
#include <libbpf.h>

#include <errno.h>
#include <stdint.h>
#include <filesystem>

static constexpr char kTestWakeupSourceName[] = "test";

// Test eBPF objects
static constexpr char kTestActivateProgPath[] =
        "/sys/fs/bpf/kernelwakelockduration/"
        "prog_kernelWakelockDurationTest_tracepoint_power_wakeup_source_activate";
static constexpr char kTestDectivateProgPath[] =
        "/sys/fs/bpf/kernelwakelockduration/"
        "prog_kernelWakelockDurationTest_tracepoint_power_wakeup_source_deactivate";
static constexpr char kTestProgramStateMapPath[] =
        "/sys/fs/bpf/kernelwakelockduration/map_kernelWakelockDurationTest_program_state";

const static struct kernel_wakelock_duration_program_state init_state = {};

namespace android::bpfprogs::kernel_wakelock_duration {
namespace {
using ::android::base::unique_fd;

unsigned int getCec(unsigned int completedWakelocksCount, unsigned int activeWakelocksCount) {
    return (completedWakelocksCount << IN_PROGRESS_BITS) | activeWakelocksCount;
}

void runProgram(int progFd, const char* wakeup_source_name, unsigned int cec) {
    uint64_t ctx[2];
    ctx[0] = (uint64_t)wakeup_source_name;
    ctx[1] = (uint64_t)cec;

    errno = 0;
    android::bpf::runProgram(progFd, nullptr, 0, ctx, sizeof(ctx));
    ASSERT_EQ(errno, 0) << "Unable to run the program.";
}

unique_fd getProgramStateMapFd() {
    unique_fd mProgramStateMapFd = unique_fd{bpf_obj_get(kTestProgramStateMapPath)};
    EXPECT_GE(mProgramStateMapFd, 0);

    return mProgramStateMapFd;
}

class KernelWakelockDurationTest : public ::testing::Test {
  protected:
    static void SetUpTestSuite() {
        if (!android::bpfprogs::flags::kernel_wakelock_duration()) {
            GTEST_SKIP() << "Feature flag not enabled.";
        }

        mActivateProgFd = unique_fd{android::bpf::retrieveProgram(kTestActivateProgPath)};
        ASSERT_GE(mActivateProgFd, 0) << "Activate program file should be found.";

        mDeactivateProgFd = unique_fd{android::bpf::retrieveProgram(kTestDectivateProgPath)};
        ASSERT_GE(mDeactivateProgFd, 0) << "Deactivate program file should be found.";

        mProgramStateMapFd = unique_fd{getProgramStateMapFd()};
    }

    void SetUp() override { resetProgramStateMap(); }

    void resetProgramStateMap() {
        uint32_t zero = 0;

        int ret =
                bpf_update_elem(mProgramStateMapFd.get(), &zero,
                                const_cast<void*>(static_cast<const void*>(&init_state)), BPF_ANY);

        ASSERT_EQ(ret, 0);
    }

    int64_t getTimerState() {
        uint32_t zero = 0;
        struct kernel_wakelock_duration_program_state value;

        int result = bpf_lookup_elem(mProgramStateMapFd.get(), &zero, &value);
        EXPECT_GE(result, 0);

        return value.timer_state_ns;
    }

    void runActivateProgram(int cec) {
        runProgram(mActivateProgFd.get(), kTestWakeupSourceName, cec);
    }

    void runDeactivateProgram(int cec) {
        runProgram(mDeactivateProgFd.get(), kTestWakeupSourceName, cec);
    }

    static unique_fd mActivateProgFd;
    static unique_fd mDeactivateProgFd;
    static unique_fd mProgramStateMapFd;
};

class KernelWakelockDurationDisabledFlag : public ::testing::Test {
  protected:
    static void SetUpTestSuite() {
        if (android::bpfprogs::flags::kernel_wakelock_duration()) {
            GTEST_SKIP() << "Feature flag enabled. Another test suite will execute.";
        }
    }
};
}  // namespace

unique_fd KernelWakelockDurationTest::mActivateProgFd = unique_fd{-1};
unique_fd KernelWakelockDurationTest::mDeactivateProgFd = unique_fd{-1};
unique_fd KernelWakelockDurationTest::mProgramStateMapFd = unique_fd{-1};

TEST_F(KernelWakelockDurationDisabledFlag, programs_are_not_loaded) {
    ASSERT_FALSE(std::filesystem::exists(kWakeupActivateProgPath));
    ASSERT_FALSE(std::filesystem::exists(kWakeupDeactivateProgPath));
    ASSERT_FALSE(std::filesystem::exists(kTestActivateProgPath));
    ASSERT_FALSE(std::filesystem::exists(kTestDectivateProgPath));
}

TEST_F(KernelWakelockDurationDisabledFlag, maps_are_not_loaded) {
    ASSERT_FALSE(std::filesystem::exists(kProgramStateMapPath));
    ASSERT_FALSE(std::filesystem::exists(kTestProgramStateMapPath));
}

TEST_F(KernelWakelockDurationTest, programs_are_loaded) {
    ASSERT_TRUE(std::filesystem::exists(kWakeupActivateProgPath));
    ASSERT_TRUE(std::filesystem::exists(kWakeupDeactivateProgPath));
    ASSERT_TRUE(std::filesystem::exists(kTestActivateProgPath));
    ASSERT_TRUE(std::filesystem::exists(kTestDectivateProgPath));
}

TEST_F(KernelWakelockDurationTest, maps_are_loaded) {
    ASSERT_TRUE(std::filesystem::exists(kProgramStateMapPath));
    ASSERT_TRUE(std::filesystem::exists(kTestProgramStateMapPath));
}

TEST_F(KernelWakelockDurationTest, timer_state_is_negative_after_activate) {
    int cec = getCec(0, 1);
    runActivateProgram(cec);

    ASSERT_LT(getTimerState(), 0)
            << "Timer state should be less than 0 while there are active wakelocks.";
}

TEST_F(KernelWakelockDurationTest,
       timer_state_is_negative_when_initializing_with_active_wakelocks) {
    int cec = getCec(3, 1);
    runDeactivateProgram(cec);

    ASSERT_LT(getTimerState(), 0) << "Timer state should be less than 0 if the programs are "
                                     "initialized while there are active wakelocks.";
}

TEST_F(KernelWakelockDurationTest, duration_is_calculated) {
    int activateCec = getCec(10, 1);
    int deactivateCec = getCec(11, 0);

    runActivateProgram(activateCec);
    runDeactivateProgram(deactivateCec);

    ASSERT_GT(getTimerState(), 0)
            << "Timer state should be greater than 0 while there are no active wakelocks.";
}

TEST_F(KernelWakelockDurationTest, skips_event_if_before_initialization) {
    int activateCec = getCec(10, 1);
    int deactivateCec = getCec(11, 0);

    runDeactivateProgram(deactivateCec);
    runActivateProgram(activateCec);

    ASSERT_EQ(getTimerState(), 0) << "Activate event shouldn't be taken into account as it "
                                     "happened before the event that initialized the programs.";
}
}  // namespace android::bpfprogs::kernel_wakelock_duration