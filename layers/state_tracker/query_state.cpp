/* Copyright (c) 2015-2026 The Khronos Group Inc.
 * Copyright (c) 2015-2026 Valve Corporation
 * Copyright (c) 2015-2026 LunarG, Inc.
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

uint32_t QueryPool::GetQuerySize(VkQueryResultFlags flags) const {
    uint32_t query_avail_data = (flags & (VK_QUERY_RESULT_WITH_AVAILABILITY_BIT | VK_QUERY_RESULT_WITH_STATUS_BIT_KHR)) ? 1 : 0;
    uint32_t query_size_in_bytes = (flags & VK_QUERY_RESULT_64_BIT) ? sizeof(uint64_t) : sizeof(uint32_t);
    uint32_t query_items = 0;
    uint32_t query_size = 0;

    switch (create_info.queryType) {
        case VK_QUERY_TYPE_OCCLUSION:
            // Occlusion queries write one integer value - the number of samples passed.
            query_items = 1;
            query_size = query_size_in_bytes * (query_items + query_avail_data);
            break;

        case VK_QUERY_TYPE_PIPELINE_STATISTICS:
            // Pipeline statistics queries write one integer value for each bit that is enabled in the pipelineStatistics
            // when the pool is created
            {
                query_items = CountSetBits(create_info.pipelineStatistics);
                query_size = query_size_in_bytes * (query_items + query_avail_data);
            }
            break;

        case VK_QUERY_TYPE_TIMESTAMP:
            // Timestamp queries write one integer
            query_items = 1;
            query_size = query_size_in_bytes * (query_items + query_avail_data);
            break;

        case VK_QUERY_TYPE_RESULT_STATUS_ONLY_KHR:
            // Result status only writes only status
            query_items = 0;
            query_size = query_size_in_bytes * (query_items + query_avail_data);
            break;

        case VK_QUERY_TYPE_TRANSFORM_FEEDBACK_STREAM_EXT:
            // Transform feedback queries write two integers
            query_items = 2;
            query_size = query_size_in_bytes * (query_items + query_avail_data);
            break;

        case VK_QUERY_TYPE_VIDEO_ENCODE_FEEDBACK_KHR:
            // Video encode feedback queries write one integer value for each bit that is enabled in
            // VkQueryPoolVideoEncodeFeedbackCreateInfoKHR::encodeFeedbackFlags when the pool is created
            query_items = CountSetBits(video_encode_feedback_flags);
            query_size = query_size_in_bytes * (query_items + query_avail_data);
            break;

        case VK_QUERY_TYPE_PERFORMANCE_QUERY_KHR:
            // Performance queries store results in a tightly packed array of VkPerformanceCounterResultsKHR
            query_items = perf_counter_index_count;
            query_size = sizeof(VkPerformanceCounterResultKHR) * query_items;
            break;

        // These cases intentionally fall through to the default
        case VK_QUERY_TYPE_ACCELERATION_STRUCTURE_COMPACTED_SIZE_KHR:  // VK_QUERY_TYPE_ACCELERATION_STRUCTURE_COMPACTED_SIZE_NV
        case VK_QUERY_TYPE_ACCELERATION_STRUCTURE_SERIALIZATION_SIZE_KHR:
        case VK_QUERY_TYPE_PERFORMANCE_QUERY_INTEL:
        default:
            query_size = 0;
            break;
    }

    return query_size;
}

}  // namespace vvl

QueryCount::QueryCount(vvl::CommandBuffer &cb_state) {
    count = 1;
    subpass = cb_state.GetActiveSubpass();
    inside_render_pass = cb_state.active_render_pass != nullptr;
    // If render pass instance has multiview enabled, query uses N consecutive query indices
    if (inside_render_pass) {
        const uint32_t bits = CountSetBits(cb_state.GetViewMask());
        count = std::max(count, bits);
    }
}