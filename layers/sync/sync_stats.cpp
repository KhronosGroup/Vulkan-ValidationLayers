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

void Stats::AddCommandBufferContext() { command_buffer_contexts.Add(1); }
void Stats::RemoveCommandBufferContext() { command_buffer_contexts.Sub(1); }

void Stats::AddQueueBatchContext() { queue_batch_contexts.Add(1); }
void Stats::RemoveQueueBatchContext() { queue_batch_contexts.Sub(1); }

void Stats::AddTimelineSignals(uint32_t count) { timeline_signals.Add(count); }
void Stats::RemoveTimelineSignals(uint32_t count) { timeline_signals.Sub(count); }

void Stats::AddUnresolvedBatch() { unresolved_batches.Add(1); }
void Stats::RemoveUnresolvedBatch() { unresolved_batches.Sub(1); }

void Stats::AddHandleRecord(uint32_t count) { handle_records.Add(count); }
void Stats::RemoveHandleRecord(uint32_t count) { handle_records.Sub(count); }

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

void UpdateAccessMapStats(const ResourceAccessRangeMap& access_map, AccessContextStats& stats) {
    stats.access_contexts += 1;
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
    std::ostringstream ss;
    ss << std::left;

    auto print_common_stats = [&ss](const char* field_name, const ValueMax32& stat) {
        ss << std::setw(32) << field_name;
        ss << std::setw(12) << stat.value.u32 << stat.max_value.u32;
        ss << "\n";
    };
    auto print_common_stats64 = [&ss](const char* field_name, uint64_t v1, uint64_t v2) {
        ss << std::setw(32) << field_name;
        ss << std::setw(12) << v1 << v2;
        ss << "\n";
    };
    auto print_access_state_stats = [&ss](const char* context_type, const AccessContextStats& stats) {
        ss << std::setw(15) << std::string(context_type) + "(" + std::to_string(stats.access_contexts) + ")";
        ss << std::setw(10) << stats.read_states;
        ss << std::setw(10) << stats.write_states;
        ss << std::setw(16) << stats.access_states;
        ss << std::setw(18) << stats.access_states_with_multiple_reads;
        ss << std::setw(14) << stats.access_states_with_dynamic_allocations;

        uint64_t access_state_objects_size = sizeof(ResourceAccessState) * stats.access_states;
        ss << std::setw(16) << access_state_objects_size;

        ss << std::setw(14) << stats.access_states_dynamic_allocation_size;
        ss << "\n";
    };

    ss << "-----------------------\n";
    ss << "Common stats                    count       max_count\n";
    ss << "-----------------------\n";
    print_common_stats("CommandBufferAccessContext", command_buffer_contexts);
    print_common_stats("QueueBatchContext", queue_batch_contexts);
    print_common_stats("Timeline signal", timeline_signals);
    print_common_stats("Unresolved batch", unresolved_batches);
    print_common_stats("HandleRecord", handle_records);

    uint64_t handle_record_memory = handle_records.value.u32 * sizeof(HandleRecord);
    uint64_t handle_record_max_memory = handle_records.max_value.u32 * sizeof(HandleRecord);
    print_common_stats64("HandleRecord bytes", handle_record_memory, handle_record_max_memory);

    const char* access_stats_header =
        "context        reads     writes    access_states   with_multi_read   with_allocs   size (bytes)    alloc_size (bytes)\n";

    ss << "\n";
    ss << "-----------------------\n";
    ss << "AccessState stats\n";
    ss << "-----------------------\n";
    ss << access_stats_header;
    print_access_state_stats("CB", access_stats.cb_access_stats);
    print_access_state_stats("Queue", access_stats.queue_access_stats);
    print_access_state_stats("Subpass", access_stats.subpass_access_stats);

    ss << "\n";
    ss << "-----------------------\n";
    ss << "MAX AccessState stats\n";
    ss << "-----------------------\n";
    ss << access_stats_header;
    print_access_state_stats("CB", access_stats.max_cb_access_stats);
    print_access_state_stats("Queue", access_stats.max_queue_access_stats);
    print_access_state_stats("Subpass", access_stats.max_subpass_access_stats);

    ss << "\n";
    ss << "Max first accesses";
    ss << ": CB: " << access_stats.cb_access_stats.max_first_accesses_size;
    ss << ", Queue: " << access_stats.queue_access_stats.max_first_accesses_size;
    ss << ", Subpass: " << access_stats.subpass_access_stats.max_first_accesses_size;
    ss << "\n";

#if defined(USE_MIMALLOC_STATS)
    // Print allocation counts (these are not reported by mi_stats_print_out)
    ss << "\n";
    ss << "malloc_normal_count: " << mi_stats.malloc_normal_count.total << "\n";
    ss << "malloc_huge_count: " << mi_stats.malloc_huge_count.total << "\n";
    ss << "\n";
    // Print main mimalloc stats
    mi_stats_print_out([](const char* msg, void* arg) { *static_cast<std::ostringstream*>(arg) << msg; }, &ss);
    ss << "\n";
#endif
    return ss.str();
}

}  // namespace syncval_stats
#endif  // VVL_ENABLE_SYNCVAL_STATS != 0
