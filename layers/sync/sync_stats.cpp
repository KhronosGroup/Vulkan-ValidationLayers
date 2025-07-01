/* Copyright (c) 2024-2025 The Khronos Group Inc.
 * Copyright (c) 2024-2025 Valve Corporation
 * Copyright (c) 2024-2025 LunarG, Inc.
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

#include "sync_stats.h"

#if VVL_ENABLE_SYNCVAL_STATS != 0
#include "sync_commandbuffer.h"

#include <iostream>

namespace vvl {
// Until C++ 26 std::atomic<T>::fetch_max arrives
// https://en.cppreference.com/w/cpp/atomic/atomic/fetch_max
// https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2024/p0493r5.pdf
template <typename T>
inline T atomic_fetch_max(std::atomic<T> &current_max, const T &value) noexcept {
    T t = current_max.load();
    while (!current_max.compare_exchange_weak(t, std::max(t, value)))
        ;
    return t;
}
}  // namespace vvl

namespace syncval_stats {

// NOTE: fetch_add/fetch_sub return value before increment/decrement.
// Our Add/Sub functions return new counter values, so they need to
// adjust result of the atomic function by adding/subtracting one.

void Value32::Update(uint32_t new_value) { u32.store(new_value); }
uint32_t Value32::Add(uint32_t n) { return u32.fetch_add(n) + 1; }
uint32_t Value32::Sub(uint32_t n) { return u32.fetch_sub(n) - 1; }

void Value64::Update(uint64_t new_value) { u64.store(new_value); }
uint64_t Value64::Add(uint64_t n) { return u64.fetch_add(n) + 1; }
uint64_t Value64::Sub(uint64_t n) { return u64.fetch_sub(n) - 1; }

void ValueMax32::Update(uint32_t new_value) {
    value.Update(new_value);
    vvl::atomic_fetch_max(max_value.u32, new_value);
}
void ValueMax32::Add(uint32_t n) {
    uint32_t new_value = value.Add(n);
    vvl::atomic_fetch_max(max_value.u32, new_value);
}
void ValueMax32::Sub(uint32_t n) { value.Sub(n); }

void ValueMax64::Update(uint64_t new_value) {
    value.Update(new_value);
    vvl::atomic_fetch_max(max_value.u64, new_value);
}
void ValueMax64::Add(uint64_t n) {
    uint64_t new_value = value.Add(n);
    vvl::atomic_fetch_max(max_value.u64, new_value);
}
void ValueMax64::Sub(uint64_t n) { value.Sub(n); }

Stats::~Stats() {
    if (report_on_destruction) {
        const std::string report = CreateReport();
        std::cout << report;
    }
}

void Stats::AddCommandBufferContext() { command_buffer_context_counter.Add(1); }
void Stats::RemoveCommandBufferContext() { command_buffer_context_counter.Sub(1); }

void Stats::AddQueueBatchContext() { queue_batch_context_counter.Add(1); }
void Stats::RemoveQueueBatchContext() { queue_batch_context_counter.Sub(1); }

void Stats::AddTimelineSignals(uint32_t count) { timeline_signal_counter.Add(count); }
void Stats::RemoveTimelineSignals(uint32_t count) { timeline_signal_counter.Sub(count); }

void Stats::AddUnresolvedBatch() { unresolved_batch_counter.Add(1); }
void Stats::RemoveUnresolvedBatch() { unresolved_batch_counter.Sub(1); }

void Stats::AddHandleRecord(uint32_t count) { handle_record_counter.Add(count); }
void Stats::RemoveHandleRecord(uint32_t count) { handle_record_counter.Sub(count); }

void Stats::UpdateMemoryStats() {
#if defined(USE_MIMALLOC_STATS)
    mi_stats_merge();
    mi_stats_get(sizeof(mi_stats), &mi_stats);

    const int64_t current_bytes = mi_stats.malloc_normal.current + mi_stats.malloc_huge.current;
    assert(current_bytes >= 0);
    total_allocated_memory.value.Update((uint64_t)current_bytes);

    const int64_t peak_bytes = mi_stats.malloc_normal.peak + mi_stats.malloc_huge.peak;
    assert(peak_bytes >= 0);
    total_allocated_memory.max_value.Update((uint64_t)peak_bytes);
#endif
}

void Stats::ReportOnDestruction() { report_on_destruction = true; }

std::string Stats::CreateReport() {
    UpdateMemoryStats();

    std::ostringstream str;
    {
        uint64_t allocated_bytes = total_allocated_memory.value.u64;
        uint64_t allocated_bytes_max = total_allocated_memory.max_value.u64;
        str << "Allocated memory:\n";
        str << "\tcurrent = " << allocated_bytes << " bytes\n";
        str << "\tmax = " << allocated_bytes_max << " bytes\n";
    }
    {
        uint32_t cb_contex = command_buffer_context_counter.value.u32;
        uint32_t cb_context_max = command_buffer_context_counter.max_value.u32;
        str << "CommandBufferAccessContext:\n";
        str << "\tcount = " << cb_contex << '\n';
        str << "\tmax_count = " << cb_context_max << '\n';
    }
    {
        uint32_t qbc_context = queue_batch_context_counter.value.u32;
        uint32_t qbc_context_max = queue_batch_context_counter.max_value.u32;
        str << "QueueBatchContext:\n";
        str << "\tcount = " << qbc_context << "\n";
        str << "\tmax_count = " << qbc_context_max << "\n";
    }
    {
        uint32_t signal = timeline_signal_counter.value.u32;
        uint32_t signal_max = timeline_signal_counter.max_value.u32;
        str << "Timeline signal:\n";
        str << "\tcount = " << signal << "\n";
        str << "\tmax_count = " << signal_max << "\n";
    }
    {
        uint32_t unresolved_batch = unresolved_batch_counter.value.u32;
        uint32_t unresolved_batch_max = unresolved_batch_counter.max_value.u32;
        str << "Unresolved batch:\n";
        str << "\tcount = " << unresolved_batch << "\n";
        str << "\tmax_count = " << unresolved_batch_max << "\n";
    }
    {
        uint32_t handle_record = handle_record_counter.value.u32;
        uint64_t handle_record_memory = handle_record * sizeof(HandleRecord);
        uint32_t handle_record_max = handle_record_counter.max_value.u32;
        uint64_t handle_record_max_memory = handle_record_max * sizeof(HandleRecord);
        str << "HandleRecord:\n";
        str << "\tcount = " << handle_record << '\n';
        str << "\tmemory = " << handle_record_memory << " bytes\n";
        str << "\tmax_count = " << handle_record_max << '\n';
        str << "\tmax_memory = " << handle_record_max_memory << " bytes\n";
    }
    return str.str();
}

}  // namespace syncval_stats
#endif  // VVL_ENABLE_SYNCVAL_STATS != 0
