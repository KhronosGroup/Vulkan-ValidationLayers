/* Copyright (c) 2015-2025 The Khronos Group Inc.
 * Copyright (c) 2015-2025 Valve Corporation
 * Copyright (c) 2015-2025 LunarG, Inc.
 * Copyright (C) 2015-2025 Google Inc.
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

#include "state_tracker/query_state.h"
#include "state_tracker/cmd_buffer_state.h"
#include "state_tracker/render_pass_state.h"
#include "utils/math_utils.h"

namespace vvl {

QueryPool::QueryPool(VkQueryPool handle, const VkQueryPoolCreateInfo *pCreateInfo, uint32_t index_count,
                     uint32_t perf_queue_family_index, uint32_t n_perf_pass, bool has_cb, bool has_rb,
                     std::shared_ptr<const vvl::VideoProfileDesc> &&supp_video_profile,
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
    const QueryState initial_state =
        (pCreateInfo->flags & VK_QUERY_POOL_CREATE_RESET_BIT_KHR) ? QUERYSTATE_RESET : QUERYSTATE_UNKNOWN;
    for (uint32_t i = 0; i < pCreateInfo->queryCount; ++i) {
        auto perf_size = n_perf_pass > 0 ? n_perf_pass : 1;
        query_states_[i].reserve(perf_size);
        for (uint32_t p = 0; p < perf_size; p++) {
            query_states_[i].emplace_back(initial_state);
        }
    }
}

void QueryPool::SetQueryState(uint32_t query, uint32_t perf_pass, QueryState state) {
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
QueryState QueryPool::GetQueryState(uint32_t query, uint32_t perf_pass) const {
    auto guard = ReadLock();
    // this method can get called with invalid arguments during validation
    if (query < query_states_.size() && ((n_performance_passes == 0 && perf_pass == 0) || (perf_pass < n_performance_passes))) {
        return query_states_[query][perf_pass];
    }
    return QUERYSTATE_UNKNOWN;
}

QueryResultType QueryPool::GetQueryResultType(QueryState state, VkQueryResultFlags flags) {
    switch (state) {
        case QUERYSTATE_UNKNOWN:
            return QUERYRESULT_UNKNOWN;
        case QUERYSTATE_RESET:
        case QUERYSTATE_RUNNING:
            if (flags & VK_QUERY_RESULT_WAIT_BIT) {
                return ((state == QUERYSTATE_RESET) ? QUERYRESULT_WAIT_ON_RESET : QUERYRESULT_WAIT_ON_RUNNING);
            } else if ((flags & VK_QUERY_RESULT_PARTIAL_BIT) || (flags & VK_QUERY_RESULT_WITH_AVAILABILITY_BIT)) {
                return QUERYRESULT_SOME_DATA;
            } else {
                return QUERYRESULT_NO_DATA;
            }
        case QUERYSTATE_ENDED:
            if ((flags & VK_QUERY_RESULT_WAIT_BIT) || (flags & VK_QUERY_RESULT_PARTIAL_BIT) ||
                (flags & VK_QUERY_RESULT_WITH_AVAILABILITY_BIT)) {
                return QUERYRESULT_SOME_DATA;
            } else {
                return QUERYRESULT_UNKNOWN;
            }
        case QUERYSTATE_AVAILABLE:
            return QUERYRESULT_SOME_DATA;
    }
    assert(false);
    return QUERYRESULT_UNKNOWN;
}

}  // namespace vvl

QueryCount::QueryCount(vvl::CommandBuffer &cb_state) {
    count = 1;
    subpass = 0;
    inside_render_pass = cb_state.active_render_pass != nullptr;
    // If render pass instance has multiview enabled, query uses N consecutive query indices
    if (inside_render_pass) {
        const uint32_t bits = GetBitSetCount(cb_state.GetViewMask());
        count = std::max(count, bits);
    }
}