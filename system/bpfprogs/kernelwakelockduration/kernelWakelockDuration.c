/*
 * kernelWakelockDuration eBPF program
 *
 * Copyright (C) 2025 Google
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License version
 * 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */

#include <android_bpf_defs.h>
#include <bpf_kernelwakelockduration.h>
#include <limits.h>
#include <stdbool.h>
#include <stdint.h>
#include <sys/cdefs.h>

#ifdef ENABLE_LIBBPF
#include <linux/bpf.h>
#include <private/android_filesystem_config.h>
#endif  // ENABLE_LIBBPF

// The cec is split in half between the in progress kernel wakelocks (lower part) and completed
// kernel wakelocks (upper part). The maximum value for the upper part is then equal to the maximum
// value for the in progress kernel wakelocks.
static const uint32_t kMaxCecUpper = MAX_IN_PROGRESS;

// Sentinel value used to signal that we processed enough events after initialization to consider it
// completed.
static const uint64_t kFullyInitialized = UINT64_MAX;

// Whether the standard processing should execute after initialization.
enum InitializationResult {
    RUN_PROCESSING,
    SKIP_PROCESSING,
};

const static struct kernel_wakelock_duration_program_state init_state = {};

DEFINE_BPF_MAP_GRO(program_state, ARRAY, uint32_t, struct kernel_wakelock_duration_program_state, 1,
                   AID_SYSTEM);

static __always_inline struct kernel_wakelock_duration_program_state* get_state_map(void) {
    uint32_t zero = 0;
    struct kernel_wakelock_duration_program_state* state_map = bpf_program_state_lookup_elem(&zero);

    if (state_map == NULL) {
        bpf_program_state_update_elem(&zero, &init_state, BPF_NOEXIST);
        state_map = bpf_program_state_lookup_elem(&zero);

        if (state_map == NULL) {
            // This should only happen if the previous update_elem call failed.
            return NULL;
        }
    }

    return state_map;
}

static __always_inline unsigned int get_state_from_event(struct bpf_raw_tracepoint_args* ctx) {
    // From common/include/trace/events/power.h in the kernel:
    // TP_PROTO(const char *name, unsigned int state)
    // TP_ARGS(name, state)
    return ctx->args[1];
}

// Initializes the state of the programs. It ensures that we do not process events fired after the
// event used to initialize.
static enum InitializationResult __always_inline
initialize(struct kernel_wakelock_duration_program_state* state_map, uint64_t cec, uint64_t now) {
    // This is already checked in get_state_map but the eBPF verifier needs it anyway.
    if (state_map == NULL) {
        return SKIP_PROCESSING;
    }

    uint64_t cec_upper = cec >> IN_PROGRESS_BITS;
    uint64_t cec_lower = cec & MAX_IN_PROGRESS;

    uint64_t previous = __sync_val_compare_and_swap(&state_map->program_init, 0, cec);

    // If the programs are fully initialized we continue with the normal event processing.
    if (previous == kFullyInitialized) {
        return RUN_PROCESSING;
    }

    // If this is the first event we set the timer if there are still active kernel wakelocks.
    if (previous == 0) {
        if (cec_lower > 0) {
            __sync_fetch_and_sub(&state_map->timer_state_ns, now);
        }
        return SKIP_PROCESSING;
    }

    // The following code is needed to avoid processing events that fired before the event that we
    // used to initialize the programs. This can happen in case of a race condition during
    // initialization. If we're running this function on an event that is older than the event we
    // used to initialize the programs, we skip it. An additional challenge is that the upper part
    // of cec is only 16 bits, which can easily wrap-around. For this reason we're calculating the
    // distance in modulo kMaxCecUpper.
    uint64_t previous_cec_upper = previous >> IN_PROGRESS_BITS;
    uint64_t previous_cec_lower = previous & MAX_IN_PROGRESS;
    uint64_t distance = (kMaxCecUpper + cec_upper - previous_cec_upper) % kMaxCecUpper;

    // The current event is older than the previous one if they have the same upper part but the
    // lower part is greater in the previous cec.
    //
    // For example:
    // current_upper = 100 \
    // current_lower = 75  \
    // previous_upper = 100 \
    // previous_lower = 76 \
    //
    // In this case the previous event must have been fired before the current one from the kernel.
    // Otherwise current_upper would have been 101 to indicate that a wakelock completed (in
    // progress wakelocks went from 76 to 75).
    //
    // The other case is when previous_cec_upper > cec_upper using the modulo logic.
    if (((distance == 0) && (previous_cec_lower > cec_lower)) || (distance > (kMaxCecUpper / 2))) {
        return SKIP_PROCESSING;
    }

    // If we processed enough events after the first one, we assume that we won't receive events
    // older than the first one anymore and we consider the programs initialized.
    if (distance > 64) {
        __sync_fetch_and_or(&state_map->program_init, kFullyInitialized);
    }

    return RUN_PROCESSING;
}

DEFINE_BPF_PROG("raw_tp/wakeup_source_activate", AID_ROOT, AID_SYSTEM,
                tracepoint_power_wakeup_source_activate)(struct bpf_raw_tracepoint_args* ctx) {
    uint64_t now = bpf_ktime_get_boot_ns();
    struct kernel_wakelock_duration_program_state* state_map = get_state_map();
    int cec = get_state_from_event(ctx);
    int cec_lower = cec & MAX_IN_PROGRESS;

    if (initialize(state_map, cec, now) == SKIP_PROCESSING) {
        return 0;
    }

    if (cec_lower == 1) {
        __sync_fetch_and_sub(&state_map->timer_state_ns, now);
    }

    return 0;
}

DEFINE_BPF_PROG("raw_tp/wakeup_source_deactivate", AID_ROOT, AID_SYSTEM,
                tracepoint_power_wakeup_source_deactivate)(struct bpf_raw_tracepoint_args* ctx) {
    uint64_t now = bpf_ktime_get_boot_ns();
    struct kernel_wakelock_duration_program_state* state_map = get_state_map();
    int cec = get_state_from_event(ctx);
    int cec_lower = cec & MAX_IN_PROGRESS;

    if (initialize(state_map, cec, now) == SKIP_PROCESSING) {
        return 0;
    }

    if (cec_lower == 0) {
        __sync_fetch_and_add(&state_map->timer_state_ns, now);
    }

    return 0;
}

LICENSE("GPL");
