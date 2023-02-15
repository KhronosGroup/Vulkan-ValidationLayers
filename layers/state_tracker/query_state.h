/* Copyright (c) 2015-2023 The Khronos Group Inc.
 * Copyright (c) 2015-2023 Valve Corporation
 * Copyright (c) 2015-2023 LunarG, Inc.
 * Copyright (C) 2015-2023 Google Inc.
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
#include "state_tracker/base_node.h"
#include "hash_vk_types.h"
#include "vk_layer_utils.h"

class VideoProfileDesc;

enum QueryState {
    QUERYSTATE_UNKNOWN,    // Initial state.
    QUERYSTATE_RESET,      // After resetting.
    QUERYSTATE_RUNNING,    // Query running.
    QUERYSTATE_ENDED,      // Query ended but results may not be available.
    QUERYSTATE_AVAILABLE,  // Results available.
};

class QUERY_POOL_STATE : public BASE_NODE {
  public:
    QUERY_POOL_STATE(VkQueryPool qp, const VkQueryPoolCreateInfo *pCreateInfo, uint32_t index_count, uint32_t n_perf_pass,
                     bool has_cb, bool has_rb, std::shared_ptr<const VideoProfileDesc> &&supp_video_profile)
        : BASE_NODE(qp, kVulkanObjectTypeQueryPool),
          createInfo(*pCreateInfo),
          has_perf_scope_command_buffer(has_cb),
          has_perf_scope_render_pass(has_rb),
          n_performance_passes(n_perf_pass),
          perf_counter_index_count(index_count),
          supported_video_profile(std::move(supp_video_profile)),
          query_states_(pCreateInfo->queryCount) {
        for (uint32_t i = 0; i < pCreateInfo->queryCount; ++i) {
            auto perf_size = n_perf_pass > 0 ? n_perf_pass : 1;
            query_states_[i].reserve(perf_size);
            for (uint32_t p = 0; p < perf_size; p++) {
                query_states_[i].emplace_back(QUERYSTATE_UNKNOWN);
            }
        }
    }

    VkQueryPool pool() const { return handle_.Cast<VkQueryPool>(); }

    void SetQueryState(uint32_t query, uint32_t perf_pass, QueryState state) {
        auto guard = WriteLock();
        assert(query < query_states_.size());
        assert((n_performance_passes == 0 && perf_pass == 0) || (perf_pass < n_performance_passes));
        query_states_[query][perf_pass] = state;
    }
    QueryState GetQueryState(uint32_t query, uint32_t perf_pass) const {
        auto guard = ReadLock();
        // this method can get called with invalid arguments during validation
        if (query < query_states_.size() && ((n_performance_passes == 0 && perf_pass == 0) || (perf_pass < n_performance_passes))) {
            return query_states_[query][perf_pass];
        }
        return QUERYSTATE_UNKNOWN;
    }

    const VkQueryPoolCreateInfo createInfo;

    const bool has_perf_scope_command_buffer;
    const bool has_perf_scope_render_pass;
    const uint32_t n_performance_passes;
    const uint32_t perf_counter_index_count;

    std::shared_ptr<const VideoProfileDesc> supported_video_profile;

  private:
    ReadLockGuard ReadLock() const { return ReadLockGuard(lock_); }
    WriteLockGuard WriteLock() { return WriteLockGuard(lock_); }

    std::vector<small_vector<QueryState, 1, uint32_t>> query_states_;
    mutable std::shared_mutex lock_;
};

struct QueryObject {
    VkQueryPool pool;
    uint32_t query;
    uint32_t perf_pass;

    // These next five fields are *not* used in hash or comparison, they are effectively a data payload
    mutable uint32_t active_query_index;
    uint32_t last_activatable_query_index;
    uint32_t index;  // must be zero if !indexed
    bool indexed;
    // Command index in the command buffer where the end of the query was
    // recorded (equal to the number of commands in the command buffer before
    // the end of the query).
    uint64_t end_command_index;

    QueryObject(VkQueryPool pool_, uint32_t query_)
        : pool(pool_),
          query(query_),
          perf_pass(0),
          active_query_index(query_),
          last_activatable_query_index(query_),
          index(0),
          indexed(false),
          end_command_index(0) {}
    QueryObject(VkQueryPool pool_, uint32_t query_, uint32_t index_, uint32_t count_ = 1)
        : pool(pool_),
          query(query_),
          perf_pass(0),
          active_query_index(query_),
          last_activatable_query_index(query_ + count_ - 1),
          index(index_),
          indexed(true),
          end_command_index(0) {}
    QueryObject(const QueryObject &obj)
        : pool(obj.pool),
          query(obj.query),
          perf_pass(obj.perf_pass),
          active_query_index(obj.active_query_index),
          last_activatable_query_index(obj.last_activatable_query_index),
          index(obj.index),
          indexed(obj.indexed),
          end_command_index(obj.end_command_index) {}
    QueryObject(const QueryObject &obj, uint32_t perf_pass_)
        : pool(obj.pool),
          query(obj.query),
          perf_pass(perf_pass_),
          active_query_index(obj.active_query_index),
          last_activatable_query_index(obj.last_activatable_query_index),
          index(obj.index),
          indexed(obj.indexed),
          end_command_index(obj.end_command_index) {}
    bool operator<(const QueryObject &rhs) const {
        return (pool == rhs.pool) ? ((query == rhs.query) ? (perf_pass < rhs.perf_pass) : (query < rhs.query)) : pool < rhs.pool;
    }
};

inline bool operator==(const QueryObject &query1, const QueryObject &query2) {
    return ((query1.pool == query2.pool) && (query1.query == query2.query) && (query1.perf_pass == query2.perf_pass));
}

typedef std::map<QueryObject, QueryState> QueryMap;

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
    size_t operator()(QueryObject query) const throw() {
        return hash<uint64_t>()((uint64_t)(query.pool)) ^
               hash<uint64_t>()(static_cast<uint64_t>(query.query) | (static_cast<uint64_t>(query.perf_pass) << 32));
    }
};

}  // namespace std
