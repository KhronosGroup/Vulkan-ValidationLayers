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
#include "sync_validation.h"
#include "state_tracker/state_tracker.h"

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

void AccessContextStats::UpdateMax(const AccessContextStats& cur_stats) {
#define UPDATE_MAX(field) field = std::max(field, cur_stats.field)
    UPDATE_MAX(access_contexts);
    UPDATE_MAX(access_states);
    UPDATE_MAX(read_states);
    UPDATE_MAX(write_states);
    UPDATE_MAX(access_states_with_multiple_reads);
    UPDATE_MAX(access_states_with_dynamic_allocations);
    UPDATE_MAX(access_states_dynamic_allocation_size);
#undef UPDATE_MAX
}

void AccessContextStats::Report(std::ostringstream& ss) {
    ss << "\taccess_contexts = " << access_contexts << '\n';
    ss << "\taccess_states = " << access_states << '\n';
    ss << "\tread_states = " << read_states << '\n';
    ss << "\twrite_states = " << write_states << '\n';
    ss << "\taccess_states_with_multiple_reads = " << access_states_with_multiple_reads << '\n';
    ss << "\taccess_states_with_dynamic_allocations = " << access_states_with_dynamic_allocations << '\n';
    ss << "\taccess_states_dynamic_allocation_size = " << access_states_dynamic_allocation_size << '\n';
}

void UpdateAccessMapStats(const ResourceAccessRangeMap& access_map, AccessContextStats& stats) {
    stats.access_states += (uint32_t)access_map.size();
    for (const auto& entry : access_map) {
        const ResourceAccessState& access_state = entry.second;
        access_state.UpdateStats(stats);
    }
}

void AccessStats::Update(SyncValidator& validator) {
    std::unique_lock<std::mutex> lock(access_stats_mutex);
    cb_access_stats = {};
    queue_access_stats = {};
    subpass_access_stats = {};

    validator.device_state->ForEachShared<vvl::CommandBuffer>([this](std::shared_ptr<vvl::CommandBuffer> cb) {
        const CommandBufferAccessContext* cb_access_context = syncval_state::AccessContext(*cb);
        cb_access_context->UpdateStats(*this);
    });
    for (const auto& batch : validator.GetAllQueueBatchContexts()) {
        const AccessContext& access_context = batch->GetAccessContext();
        UpdateAccessMapStats(access_context.GetAccessStateMap(), queue_access_stats);
    }

    max_cb_access_stats.UpdateMax(cb_access_stats);
    max_queue_access_stats.UpdateMax(queue_access_stats);
    max_subpass_access_stats.UpdateMax(subpass_access_stats);
}

void Stats::UpdateAccessStats(SyncValidator& validator) { access_stats.Update(validator); }

void Stats::UpdateMemoryStats() {
#if defined(USE_MIMALLOC_STATS)
    mi_stats_merge();
    {
        std::unique_lock<std::mutex> lock(mi_stats_mutex);
        mi_stats_get(sizeof(mi_stats), &mi_stats);
    }
#endif
}

void Stats::ReportOnDestruction() { report_on_destruction = true; }

std::string Stats::CreateReport() {
    std::ostringstream str;
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
    {
        str << "Access Context Stats (from most recent update):\n";
        str << "[cb access context]\n";
        access_stats.cb_access_stats.Report(str);
        str << "[queue access context]\n";
        access_stats.queue_access_stats.Report(str);
        str << "[subpass access context]\n";
        access_stats.subpass_access_stats.Report(str);

        str << "Access Context Stats (max values):\n";
        str << "[cb access context]\n";
        access_stats.max_cb_access_stats.Report(str);
        str << "[queue access context]\n";
        access_stats.max_queue_access_stats.Report(str);
        str << "[subpass access context]\n";
        access_stats.max_subpass_access_stats.Report(str);
    }

#if defined(USE_MIMALLOC_STATS)
    mi_stats_print_out([](const char* msg, void* arg) { *static_cast<std::ostringstream*>(arg) << msg; }, &str);
    // Print allocation counts (these are not reported by mi_stats_print_out)
    str << "malloc_normal_count: " << mi_stats.malloc_normal_count.total << "\n";
    str << "malloc_huge_count: " << mi_stats.malloc_huge_count.total << "\n";
#endif
    return str.str();
}

}  // namespace syncval_stats
#endif  // VVL_ENABLE_SYNCVAL_STATS != 0
