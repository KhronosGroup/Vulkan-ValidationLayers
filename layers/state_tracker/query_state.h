/* Copyright (c) 2015-2024 The Khronos Group Inc.
 * Copyright (c) 2015-2024 Valve Corporation
 * Copyright (c) 2015-2024 LunarG, Inc.
 * Copyright (C) 2015-2024 Google Inc.
 * Modifications Copyright (C) 2020 Advanced Micro Devices, Inc. All rights reserved.
 * Modifications Copyright (C) 2022 RasterGrid Kft.
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
#pragma once
#include "state_tracker/state_object.h"

enum QueryState {
    QUERYSTATE_UNKNOWN,    // Initial state.
    QUERYSTATE_RESET,      // After resetting.
    QUERYSTATE_RUNNING,    // Query running.
    QUERYSTATE_ENDED,      // Query ended but results may not be available.
    QUERYSTATE_AVAILABLE,  // Results available.
};

namespace vvl {

class VideoProfileDesc;

class QueryPool : public StateObject {
  public:
    QueryPool(VkQueryPool handle, const VkQueryPoolCreateInfo *pCreateInfo, u32 index_count, u32 perf_queue_family_index,
              u32 n_perf_pass, bool has_cb, bool has_rb, std::shared_ptr<const vvl::VideoProfileDesc> &&supp_video_profile,
              VkVideoEncodeFeedbackFlagsKHR enabled_video_encode_feedback_flags)
        : StateObject(handle, kVulkanObjectTypeQueryPool),
          safe_create_info(pCreateInfo),
          create_info(*safe_create_info.ptr()),
          has_perf_scope_command_buffer(has_cb),
          has_perf_scope_render_pass(has_rb),
          n_performance_passes(n_perf_pass),
          perf_counter_index_count(index_count),
          perf_counter_queue_family_index(perf_queue_family_index),
          supported_video_profile(std::move(supp_video_profile)),
          video_encode_feedback_flags(enabled_video_encode_feedback_flags),
          query_states_(pCreateInfo->queryCount) {
        for (u32 i = 0; i < pCreateInfo->queryCount; ++i) {
            auto perf_size = n_perf_pass > 0 ? n_perf_pass : 1;
            query_states_[i].reserve(perf_size);
            for (u32 p = 0; p < perf_size; p++) {
                query_states_[i].emplace_back(QUERYSTATE_UNKNOWN);
            }
        }
    }

    VkQueryPool VkHandle() const { return handle_.Cast<VkQueryPool>(); }

    void SetQueryState(u32 query, u32 perf_pass, QueryState state) {
        auto guard = WriteLock();
        assert(query < query_states_.size());
        assert((n_performance_passes == 0 && perf_pass == 0) || (perf_pass < n_performance_passes));
        if (state == QUERYSTATE_RESET) {
            for (auto &state : query_states_[query]) {
                state = QUERYSTATE_RESET;
            }
        } else {
            query_states_[query][perf_pass] = state;
        }
    }
    QueryState GetQueryState(u32 query, u32 perf_pass) const {
        auto guard = ReadLock();
        // this method can get called with invalid arguments during validation
        if (query < query_states_.size() && ((n_performance_passes == 0 && perf_pass == 0) || (perf_pass < n_performance_passes))) {
            return query_states_[query][perf_pass];
        }
        return QUERYSTATE_UNKNOWN;
    }

    const vku::safe_VkQueryPoolCreateInfo safe_create_info;
    const VkQueryPoolCreateInfo &create_info;

    const bool has_perf_scope_command_buffer;
    const bool has_perf_scope_render_pass;
    const u32 n_performance_passes;
    const u32 perf_counter_index_count;
    const u32 perf_counter_queue_family_index;

    std::shared_ptr<const vvl::VideoProfileDesc> supported_video_profile;
    VkVideoEncodeFeedbackFlagsKHR video_encode_feedback_flags;

  private:
    ReadLockGuard ReadLock() const { return ReadLockGuard(lock_); }
    WriteLockGuard WriteLock() { return WriteLockGuard(lock_); }

    std::vector<small_vector<QueryState, 1, u32>> query_states_;
    mutable std::shared_mutex lock_;
};
}  // namespace vvl

// Represents a single Query inside a QueryPool
struct QueryObject {
    VkQueryPool pool;
    u32 slot;  // use 'slot' as alias to 'query' parameter to help reduce confusing namespace
    u32 perf_pass;

    // These below fields are *not* used in hash or comparison, they are effectively a data payload
    VkQueryControlFlags control_flags;
    mutable u32 active_query_index;
    u32 last_activatable_query_index;
    u32 index;  // must be zero if !indexed
    bool indexed;
    // Command index in the command buffer where the end of the query was
    // recorded (equal to the number of commands in the command buffer before
    // the end of the query).
    u64 end_command_index = 0;
    bool inside_render_pass = false;
    u32 subpass = 0;

    QueryObject(VkQueryPool pool_, u32 slot_, VkQueryControlFlags control_flags_ = 0, u32 perf_pass_ = 0, bool indexed_ = false,
                u32 index_ = 0)
        : pool(pool_),
          slot(slot_),
          perf_pass(perf_pass_),
          control_flags(control_flags_),
          active_query_index(slot_),
          last_activatable_query_index(slot_),
          index(index_),
          indexed(indexed_) {}

    // This is needed because vvl::CommandBuffer::BeginQuery() and EndQuery() need to make a copy to update
    QueryObject(const QueryObject &obj, u32 perf_pass_)
        : pool(obj.pool),
          slot(obj.slot),
          perf_pass(perf_pass_),
          control_flags(obj.control_flags),
          active_query_index(obj.active_query_index),
          last_activatable_query_index(obj.last_activatable_query_index),
          index(obj.index),
          indexed(obj.indexed),
          end_command_index(obj.end_command_index),
          inside_render_pass(obj.inside_render_pass),
          subpass(obj.subpass) {}
};

inline bool operator==(const QueryObject &query1, const QueryObject &query2) {
    return ((query1.pool == query2.pool) && (query1.slot == query2.slot) && (query1.perf_pass == query2.perf_pass));
}

using QueryMap = vvl::unordered_map<QueryObject, QueryState>;

enum QueryResultType {
    QUERYRESULT_UNKNOWN,
    QUERYRESULT_NO_DATA,
    QUERYRESULT_SOME_DATA,
    QUERYRESULT_WAIT_ON_RESET,
    QUERYRESULT_WAIT_ON_RUNNING,
};

inline const char *string_QueryResultType(QueryResultType result_type) {
    switch (result_type) {
        case QUERYRESULT_UNKNOWN:
            return "query may be in an unknown state";
        case QUERYRESULT_NO_DATA:
            return "query may return no data";
        case QUERYRESULT_SOME_DATA:
            return "query will return some data or availability bit";
        case QUERYRESULT_WAIT_ON_RESET:
            return "waiting on a query that has been reset and not issued yet";
        case QUERYRESULT_WAIT_ON_RUNNING:
            return "waiting on a query that has not ended yet";
    }
    assert(false);
    return "UNKNOWN QUERY STATE";  // Unreachable.
}

namespace std {
template <>
struct hash<QueryObject> {
    size_t operator()(QueryObject query_obj) const throw() {
        return hash<u64>()((u64)(query_obj.pool)) ^
               hash<u64>()(static_cast<u64>(query_obj.slot) | (static_cast<u64>(query_obj.perf_pass) << 32));
    }
};

}  // namespace std
