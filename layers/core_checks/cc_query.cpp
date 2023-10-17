/* Copyright (c) 2015-2023 The Khronos Group Inc.
 * Copyright (c) 2015-2023 Valve Corporation
 * Copyright (c) 2015-2023 LunarG, Inc.
 * Copyright (C) 2015-2023 Google Inc.
 * Modifications Copyright (C) 2020-2022 Advanced Micro Devices, Inc. All rights reserved.
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

#include <assert.h>
#include <string>

#include <vulkan/vk_enum_string_helper.h>
#include "generated/chassis.h"
#include "core_validation.h"
#include "generated/enum_flag_bits.h"

static QueryState GetLocalQueryState(const QueryMap *localQueryToStateMap, VkQueryPool queryPool, uint32_t queryIndex,
                                     uint32_t perfPass) {
    QueryObject query = QueryObject(queryPool, queryIndex, perfPass);

    auto iter = localQueryToStateMap->find(query);
    if (iter != localQueryToStateMap->end()) return iter->second;

    return QUERYSTATE_UNKNOWN;
}

bool CoreChecks::PreCallValidateDestroyQueryPool(VkDevice device, VkQueryPool queryPool, const VkAllocationCallbacks *pAllocator,
                                                 const ErrorObject &error_obj) const {
    if (disabled[query_validation]) return false;
    if (queryPool == VK_NULL_HANDLE) return false;
    const auto query_pool_state = Get<QUERY_POOL_STATE>(queryPool);

    bool skip = false;
    bool completed_by_get_results = true;
    for (uint32_t i = 0; i < query_pool_state->createInfo.queryCount; ++i) {
        auto state = query_pool_state->GetQueryState(i, 0);
        if (state != QUERYSTATE_AVAILABLE) {
            completed_by_get_results = false;
            break;
        }
    }
    if (!completed_by_get_results) {
        skip |= ValidateObjectNotInUse(query_pool_state.get(), error_obj.location, "VUID-vkDestroyQueryPool-queryPool-00793");
    }
    return skip;
}

bool CoreChecks::ValidatePerformanceQueryResults(const QUERY_POOL_STATE &query_pool_state, uint32_t firstQuery, uint32_t queryCount,
                                                 VkQueryResultFlags flags, const Location &loc) const {
    bool skip = false;

    if (flags & (VK_QUERY_RESULT_WITH_AVAILABILITY_BIT | VK_QUERY_RESULT_WITH_STATUS_BIT_KHR | VK_QUERY_RESULT_PARTIAL_BIT |
                 VK_QUERY_RESULT_64_BIT)) {
        std::string invalid_flags_string;
        for (auto flag : {VK_QUERY_RESULT_WITH_AVAILABILITY_BIT, VK_QUERY_RESULT_WITH_STATUS_BIT_KHR, VK_QUERY_RESULT_PARTIAL_BIT,
                          VK_QUERY_RESULT_64_BIT}) {
            if (flag & flags) {
                if (invalid_flags_string.size()) {
                    invalid_flags_string += " and ";
                }
                invalid_flags_string += string_VkQueryResultFlagBits(flag);
            }
        }
        const char *vuid = loc.function == Func::vkGetQueryPoolResults ? "VUID-vkGetQueryPoolResults-queryType-03230"
                                                                       : "VUID-vkCmdCopyQueryPoolResults-queryType-03233";
        skip |= LogError(vuid, query_pool_state.pool(), loc.dot(Field::queryPool),
                         "(%s) was created with a queryType of"
                         "VK_QUERY_TYPE_PERFORMANCE_QUERY_KHR but flags contains %s.",
                         FormatHandle(query_pool_state).c_str(), invalid_flags_string.c_str());
    }

    for (uint32_t query_index = firstQuery; query_index < queryCount; query_index++) {
        uint32_t submitted = 0;
        for (uint32_t pass_index = 0; pass_index < query_pool_state.n_performance_passes; pass_index++) {
            auto state = query_pool_state.GetQueryState(query_index, pass_index);
            if (state == QUERYSTATE_AVAILABLE) {
                submitted++;
            }
        }
        if (submitted < query_pool_state.n_performance_passes) {
            const char *vuid = loc.function == Func::vkGetQueryPoolResults ? "VUID-vkGetQueryPoolResults-queryType-03231"
                                                                           : "VUID-vkCmdCopyQueryPoolResults-queryType-03234";
            skip |= LogError(vuid, query_pool_state.pool(), loc.dot(Field::queryPool),
                             "(%s) has %u performance query passes, but the query has only been "
                             "submitted for %u of the passes.",
                             FormatHandle(query_pool_state).c_str(), query_pool_state.n_performance_passes, submitted);
        }
    }

    return skip;
}

bool CoreChecks::ValidateQueryPoolWasReset(const QUERY_POOL_STATE &query_pool_state, uint32_t firstQuery, uint32_t queryCount,
                                           const Location &loc, QueryMap *localQueryToStateMap, uint32_t perfPass) const {
    bool skip = false;

    for (uint32_t i = firstQuery; i < firstQuery + queryCount; ++i) {
        if (localQueryToStateMap &&
            GetLocalQueryState(localQueryToStateMap, query_pool_state.pool(), i, perfPass) != QUERYSTATE_UNKNOWN) {
            continue;
        }
        if (query_pool_state.GetQueryState(i, 0u) == QUERYSTATE_UNKNOWN) {
            skip |= LogError(kVUID_Core_QueryPool_NotReset, query_pool_state.pool(), loc.dot(Field::queryPool),
                             "%s and query %" PRIu32
                             ": query not reset. After query pool creation, each query must be reset before it is used. Queries "
                             "must also be reset between uses.",
                             FormatHandle(query_pool_state.pool()).c_str(), i);
            break;
        }
    }
    return skip;
}

bool CoreChecks::PreCallValidateGetQueryPoolResults(VkDevice device, VkQueryPool queryPool, uint32_t firstQuery,
                                                    uint32_t queryCount, size_t dataSize, void *pData, VkDeviceSize stride,
                                                    VkQueryResultFlags flags, const ErrorObject &error_obj) const {
    if (disabled[query_validation]) return false;
    bool skip = false;
    const auto &query_pool_state = *Get<QUERY_POOL_STATE>(queryPool);
    skip |= ValidateQueryPoolIndex(query_pool_state, firstQuery, queryCount, error_obj.location,
                                   "VUID-vkGetQueryPoolResults-firstQuery-00813", "VUID-vkGetQueryPoolResults-firstQuery-00816");

    if (query_pool_state.createInfo.queryType != VK_QUERY_TYPE_PERFORMANCE_QUERY_KHR) {
        skip |= ValidateQueryPoolStride("VUID-vkGetQueryPoolResults-flags-02828", "VUID-vkGetQueryPoolResults-flags-00815", stride,
                                        "dataSize", dataSize, flags, error_obj.location);
    }
    if ((query_pool_state.createInfo.queryType == VK_QUERY_TYPE_TIMESTAMP) && (flags & VK_QUERY_RESULT_PARTIAL_BIT)) {
        skip |= LogError("VUID-vkGetQueryPoolResults-queryType-00818", queryPool, error_obj.location.dot(Field::flags),
                         "(%s) includes VK_QUERY_RESULT_PARTIAL_BIT, but queryPool (%s) was created with a queryType of "
                         "VK_QUERY_TYPE_TIMESTAMP.",
                         string_VkQueryResultFlags(flags).c_str(), FormatHandle(queryPool).c_str());
    }
    if (query_pool_state.createInfo.queryType == VK_QUERY_TYPE_RESULT_STATUS_ONLY_KHR &&
        (flags & VK_QUERY_RESULT_WITH_STATUS_BIT_KHR) == 0) {
        skip |= LogError("VUID-vkGetQueryPoolResults-queryType-04810", queryPool, error_obj.location.dot(Field::flags),
                         "(%s) doesn't have VK_QUERY_RESULT_WITH_STATUS_BIT_KHR, but queryPool %s was created with "
                         "VK_QUERY_TYPE_RESULT_STATUS_ONLY_KHR "
                         "queryType.",
                         string_VkQueryResultFlags(flags).c_str(), FormatHandle(queryPool).c_str());
    }

    if (skip) {
        return skip;
    }

    uint32_t query_avail_data = (flags & (VK_QUERY_RESULT_WITH_AVAILABILITY_BIT | VK_QUERY_RESULT_WITH_STATUS_BIT_KHR)) ? 1 : 0;
    uint32_t query_size_in_bytes = (flags & VK_QUERY_RESULT_64_BIT) ? sizeof(uint64_t) : sizeof(uint32_t);
    uint32_t query_items = 0;
    uint32_t query_size = 0;

    switch (query_pool_state.createInfo.queryType) {
        case VK_QUERY_TYPE_OCCLUSION:
            // Occlusion queries write one integer value - the number of samples passed.
            query_items = 1;
            query_size = query_size_in_bytes * (query_items + query_avail_data);
            break;

        case VK_QUERY_TYPE_PIPELINE_STATISTICS:
            // Pipeline statistics queries write one integer value for each bit that is enabled in the pipelineStatistics
            // when the pool is created
            {
                query_items = GetBitSetCount(query_pool_state.createInfo.pipelineStatistics);
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

        case VK_QUERY_TYPE_PERFORMANCE_QUERY_KHR:
            // Performance queries store results in a tightly packed array of VkPerformanceCounterResultsKHR
            query_items = query_pool_state.perf_counter_index_count;
            query_size = sizeof(VkPerformanceCounterResultKHR) * query_items;
            if (query_size > stride) {
                skip |= LogError("VUID-vkGetQueryPoolResults-queryType-04519", queryPool, error_obj.location.dot(Field::queryPool),
                                 "(%s) specified stride %" PRIu64
                                 " which must be at least counterIndexCount (%d) "
                                 "multiplied by sizeof(VkPerformanceCounterResultKHR) (%zu).",
                                 FormatHandle(queryPool).c_str(), stride, query_items, sizeof(VkPerformanceCounterResultKHR));
            }

            if ((((uintptr_t)pData) % sizeof(VkPerformanceCounterResultKHR)) != 0 ||
                (stride % sizeof(VkPerformanceCounterResultKHR)) != 0) {
                skip |= LogError("VUID-vkGetQueryPoolResults-queryType-03229", queryPool, error_obj.location.dot(Field::queryPool),
                                 "(%s) was created with a queryType of "
                                 "VK_QUERY_TYPE_PERFORMANCE_QUERY_KHR but pData & stride are not multiple of the "
                                 "size of VkPerformanceCounterResultKHR.",
                                 FormatHandle(queryPool).c_str());
            }
            skip |= ValidatePerformanceQueryResults(query_pool_state, firstQuery, queryCount, flags, error_obj.location);

            break;

        // These cases intentionally fall through to the default
        case VK_QUERY_TYPE_ACCELERATION_STRUCTURE_COMPACTED_SIZE_KHR:  // VK_QUERY_TYPE_ACCELERATION_STRUCTURE_COMPACTED_SIZE_NV
        case VK_QUERY_TYPE_ACCELERATION_STRUCTURE_SERIALIZATION_SIZE_KHR:
        case VK_QUERY_TYPE_PERFORMANCE_QUERY_INTEL:
        default:
            query_size = 0;
            break;
    }

    if (query_size && (((queryCount - 1) * stride + query_size) > dataSize)) {
        skip |= LogError("VUID-vkGetQueryPoolResults-dataSize-00817", queryPool, error_obj.location.dot(Field::queryPool),
                         "(%s) specified dataSize %zu which is "
                         "incompatible with the specified query type and options.",
                         FormatHandle(queryPool).c_str(), dataSize);
    }

    skip |= ValidateQueryPoolWasReset(query_pool_state, firstQuery, queryCount, error_obj.location, nullptr, 0u);

    return skip;
}

bool CoreChecks::PreCallValidateCreateQueryPool(VkDevice device, const VkQueryPoolCreateInfo *pCreateInfo,
                                                const VkAllocationCallbacks *pAllocator, VkQueryPool *pQueryPool,
                                                const ErrorObject &error_obj) const {
    if (disabled[query_validation]) return false;
    bool skip = false;
    const Location create_info_loc = error_obj.location.dot(Field::pCreateInfo);
    switch (pCreateInfo->queryType) {
        case VK_QUERY_TYPE_PIPELINE_STATISTICS: {
            if (!enabled_features.pipelineStatisticsQuery) {
                skip |= LogError("VUID-VkQueryPoolCreateInfo-queryType-00791", device, create_info_loc.dot(Field::queryType),
                                 "is VK_QUERY_TYPE_PIPELINE_STATISTICS but pipelineStatisticsQuery feature was not enabled.");
            } else if ((pCreateInfo->pipelineStatistics & (VK_QUERY_PIPELINE_STATISTIC_TASK_SHADER_INVOCATIONS_BIT_EXT |
                                                           VK_QUERY_PIPELINE_STATISTIC_MESH_SHADER_INVOCATIONS_BIT_EXT)) &&
                       !enabled_features.meshShaderQueries) {
                skip |= LogError("VUID-VkQueryPoolCreateInfo-meshShaderQueries-07069", device,
                                 create_info_loc.dot(Field::pipelineStatistics),
                                 "(%s) contains mesh/task shader bit, but "
                                 "meshShaderQueries feature was not enabled.",
                                 string_VkQueryPipelineStatisticFlags(pCreateInfo->pipelineStatistics).c_str());
            }
            break;
        }
        case VK_QUERY_TYPE_PERFORMANCE_QUERY_KHR: {
            if (!enabled_features.performanceCounterQueryPools) {
                skip |=
                    LogError("VUID-VkQueryPoolPerformanceCreateInfoKHR-performanceCounterQueryPools-03237", device,
                             create_info_loc.dot(Field::queryType),
                             "is VK_QUERY_TYPE_PERFORMANCE_QUERY_KHR but performanceCounterQueryPools feature was not enabled.");
            }

            auto perf_ci = vku::FindStructInPNextChain<VkQueryPoolPerformanceCreateInfoKHR>(pCreateInfo->pNext);
            if (!perf_ci) {
                skip |= LogError("VUID-VkQueryPoolCreateInfo-queryType-03222", device, create_info_loc.dot(Field::queryType),
                                 "is VK_QUERY_TYPE_PERFORMANCE_QUERY_KHR, but the pNext does not contain in instance of "
                                 "VkQueryPoolPerformanceCreateInfoKHR.");
            } else {
                const auto &perf_counter_iter = physical_device_state->perf_counters.find(perf_ci->queueFamilyIndex);
                if (perf_counter_iter == physical_device_state->perf_counters.end()) {
                    skip |= LogError("VUID-VkQueryPoolPerformanceCreateInfoKHR-queueFamilyIndex-03236", device,
                                     create_info_loc.pNext(Struct::VkQueryPoolPerformanceCreateInfoKHR, Field::queueFamilyIndex),
                                     "(%" PRIu32 ") is not a valid queue family index.", perf_ci->queueFamilyIndex);
                } else {
                    const QUEUE_FAMILY_PERF_COUNTERS *perf_counters = perf_counter_iter->second.get();
                    for (uint32_t idx = 0; idx < perf_ci->counterIndexCount; idx++) {
                        if (perf_ci->pCounterIndices[idx] >= perf_counters->counters.size()) {
                            skip |= LogError(
                                "VUID-VkQueryPoolPerformanceCreateInfoKHR-pCounterIndices-03321", device,
                                create_info_loc.pNext(Struct::VkQueryPoolPerformanceCreateInfoKHR, Field::pCounterIndices, idx),
                                "(%" PRIu32 ") is not a valid counter index.", perf_ci->pCounterIndices[idx]);
                        }
                    }
                }
            }
            break;
        }
        case VK_QUERY_TYPE_RESULT_STATUS_ONLY_KHR: {
            auto video_profile = vku::FindStructInPNextChain<VkVideoProfileInfoKHR>(pCreateInfo->pNext);
            if (video_profile) {
                skip |= ValidateVideoProfileInfo(video_profile, device, "vkCreateQueryPool",
                                                 "the VkVideoProfileInfoKHR structure included in the pCreateInfo->pNext chain");
            }
            break;
        }
        case VK_QUERY_TYPE_MESH_PRIMITIVES_GENERATED_EXT: {
            if (!enabled_features.meshShaderQueries) {
                skip |=
                    LogError("VUID-VkQueryPoolCreateInfo-meshShaderQueries-07068", device, create_info_loc.dot(Field::queryType),
                             "is VK_QUERY_TYPE_MESH_PRIMITIVES_GENERATED_EXT but meshShaderQueries feature was not enabled.");
            }
            break;
        }
        default:
            break;
    }

    return skip;
}

bool CoreChecks::ValidateCmdQueueFlags(const CMD_BUFFER_STATE &cb_state, const Location &loc, VkQueueFlags required_flags,
                                       const char *vuid) const {
    auto pool = cb_state.command_pool;
    if (pool) {
        const uint32_t queue_family_index = pool->queueFamilyIndex;
        const VkQueueFlags queue_flags = physical_device_state->queue_family_properties[queue_family_index].queueFlags;
        if (!(required_flags & queue_flags)) {
            std::string required_flags_string;
            for (const auto &flag : AllVkQueueFlags) {
                if (flag & required_flags) {
                    if (required_flags_string.size()) {
                        required_flags_string += " or ";
                    }
                    required_flags_string += string_VkQueueFlagBits(flag);
                }
            }
            return LogError(vuid, cb_state.commandBuffer(), loc,
                            "Called in command buffer %s which was allocated from the command pool %s which was created with "
                            "queueFamilyIndex %u which contains the capability flags %s (but requires %s).",
                            FormatHandle(cb_state).c_str(), FormatHandle(pool->commandPool()).c_str(), queue_family_index,
                            string_VkQueueFlags(queue_flags).c_str(), required_flags_string.c_str());
        }
    }
    return false;
}

bool CoreChecks::ValidateBeginQuery(const CMD_BUFFER_STATE &cb_state, const QueryObject &query_obj, VkQueryControlFlags flags,
                                    uint32_t index, const Location &loc, const ValidateBeginQueryVuids *vuids) const {
    bool skip = false;
    auto query_pool_state = Get<QUERY_POOL_STATE>(query_obj.pool);
    const auto &query_pool_ci = query_pool_state->createInfo;

    switch (query_pool_ci.queryType) {
        case VK_QUERY_TYPE_TIMESTAMP: {
            const LogObjectList objlist(cb_state.commandBuffer(), query_obj.pool);
            const char *vuid = loc.function == Func::vkCmdBeginQueryIndexedEXT ? "VUID-vkCmdBeginQueryIndexedEXT-queryType-02804"
                                                                               : "VUID-vkCmdBeginQuery-queryType-02804";
            skip |= LogError(vuid, objlist, loc.dot(Field::queryPool), "(%s) was created with VK_QUERY_TYPE_TIMESTAMP.",
                             FormatHandle(query_obj.pool).c_str());
            break;
        }
        case VK_QUERY_TYPE_TRANSFORM_FEEDBACK_STREAM_EXT: {
            // There are tighter queue constraints to test for certain query pools
            skip |= ValidateCmdQueueFlags(cb_state, loc, VK_QUEUE_GRAPHICS_BIT, vuids->vuid_queue_feedback);
            if (!phys_dev_ext_props.transform_feedback_props.transformFeedbackQueries) {
                const char *vuid = loc.function == Func::vkCmdBeginQueryIndexedEXT
                                       ? "VUID-vkCmdBeginQueryIndexedEXT-queryType-02341"
                                       : "VUID-vkCmdBeginQuery-queryType-02328";
                const LogObjectList objlist(cb_state.commandBuffer(), query_obj.pool);
                skip |= LogError(vuid, objlist, loc.dot(Field::queryPool),
                                 "(%s) was created with queryType VK_QUERY_TYPE_TRANSFORM_FEEDBACK_STREAM_EXT, but "
                                 "VkPhysicalDeviceTransformFeedbackPropertiesEXT::transformFeedbackQueries is not supported.",
                                 FormatHandle(query_obj.pool).c_str());
            }
            break;
        }
        case VK_QUERY_TYPE_OCCLUSION:
            skip |= ValidateCmdQueueFlags(cb_state, loc, VK_QUEUE_GRAPHICS_BIT, vuids->vuid_queue_occlusion);
            break;
        case VK_QUERY_TYPE_PERFORMANCE_QUERY_KHR: {
            if (!cb_state.performance_lock_acquired) {
                const LogObjectList objlist(cb_state.commandBuffer(), query_obj.pool);
                skip |= LogError(vuids->vuid_profile_lock, objlist, loc,
                                 "profiling lock must be held before vkBeginCommandBuffer is called on "
                                 "a command buffer where performance queries are recorded.");
            }

            if (query_pool_state->has_perf_scope_command_buffer && cb_state.command_count > 0) {
                const LogObjectList objlist(cb_state.commandBuffer(), query_obj.pool);
                skip |= LogError(vuids->vuid_scope_not_first, objlist, loc.dot(Field::queryPool),
                                 "(%s) was created with a counter of scope "
                                 "VK_QUERY_SCOPE_COMMAND_BUFFER_KHR but %s is not the first recorded "
                                 "command in the command buffer.",
                                 FormatHandle(query_obj.pool).c_str(), loc.StringFunc());
            }

            if (query_pool_state->has_perf_scope_render_pass && cb_state.activeRenderPass) {
                const LogObjectList objlist(cb_state.commandBuffer(), query_obj.pool);
                skip |= LogError(vuids->vuid_scope_in_rp, objlist, loc.dot(Field::queryPool),
                                 "(%s) was created with a counter of scope "
                                 "VK_QUERY_SCOPE_RENDER_PASS_KHR but %s is inside a render pass.",
                                 FormatHandle(query_obj.pool).c_str(), loc.StringFunc());
            }
        } break;
        case VK_QUERY_TYPE_ACCELERATION_STRUCTURE_COMPACTED_SIZE_KHR:
        case VK_QUERY_TYPE_ACCELERATION_STRUCTURE_SERIALIZATION_SIZE_KHR: {
            const char *vuid = loc.function == Func::vkCmdBeginQueryIndexedEXT ? "VUID-vkCmdBeginQueryIndexedEXT-queryType-04728"
                                                                               : "VUID-vkCmdBeginQuery-queryType-04728";
            const LogObjectList objlist(cb_state.commandBuffer(), query_obj.pool);
            skip |= LogError(vuid, objlist, loc.dot(Field::queryPool), "(%s) was created with queryType %s.",
                             FormatHandle(query_obj.pool).c_str(), string_VkQueryType(query_pool_ci.queryType));
            break;
        }
        case VK_QUERY_TYPE_ACCELERATION_STRUCTURE_COMPACTED_SIZE_NV: {
            const char *vuid = loc.function == Func::vkCmdBeginQueryIndexedEXT ? "VUID-vkCmdBeginQueryIndexedEXT-queryType-04729"
                                                                               : "VUID-vkCmdBeginQuery-queryType-04729";
            const LogObjectList objlist(cb_state.commandBuffer(), query_obj.pool);
            skip |= LogError(vuid, objlist, loc.dot(Field::queryPool), "(%s) was created with queryType %s.",
                             FormatHandle(query_obj.pool).c_str(), string_VkQueryType(query_pool_ci.queryType));
            break;
        }
        case VK_QUERY_TYPE_ACCELERATION_STRUCTURE_SIZE_KHR:
        case VK_QUERY_TYPE_ACCELERATION_STRUCTURE_SERIALIZATION_BOTTOM_LEVEL_POINTERS_KHR: {
            const char *vuid = loc.function == Func::vkCmdBeginQueryIndexedEXT ? "VUID-vkCmdBeginQueryIndexedEXT-queryType-06741"
                                                                               : "VUID-vkCmdBeginQuery-queryType-06741";
            const LogObjectList objlist(cb_state.commandBuffer(), query_obj.pool);
            skip |= LogError(vuid, objlist, loc.dot(Field::queryPool), "(%s) was created with queryType %s.",
                             FormatHandle(query_obj.pool).c_str(), string_VkQueryType(query_pool_ci.queryType));
            break;
        }
        case VK_QUERY_TYPE_PIPELINE_STATISTICS: {
            if ((cb_state.command_pool->queue_flags & VK_QUEUE_GRAPHICS_BIT) == 0) {
                if (query_pool_ci.pipelineStatistics &
                    (VK_QUERY_PIPELINE_STATISTIC_INPUT_ASSEMBLY_VERTICES_BIT |
                     VK_QUERY_PIPELINE_STATISTIC_INPUT_ASSEMBLY_PRIMITIVES_BIT |
                     VK_QUERY_PIPELINE_STATISTIC_VERTEX_SHADER_INVOCATIONS_BIT |
                     VK_QUERY_PIPELINE_STATISTIC_GEOMETRY_SHADER_INVOCATIONS_BIT |
                     VK_QUERY_PIPELINE_STATISTIC_GEOMETRY_SHADER_PRIMITIVES_BIT |
                     VK_QUERY_PIPELINE_STATISTIC_CLIPPING_INVOCATIONS_BIT | VK_QUERY_PIPELINE_STATISTIC_CLIPPING_PRIMITIVES_BIT |
                     VK_QUERY_PIPELINE_STATISTIC_FRAGMENT_SHADER_INVOCATIONS_BIT |
                     VK_QUERY_PIPELINE_STATISTIC_TESSELLATION_CONTROL_SHADER_PATCHES_BIT |
                     VK_QUERY_PIPELINE_STATISTIC_TESSELLATION_EVALUATION_SHADER_INVOCATIONS_BIT)) {
                    const LogObjectList objlist(cb_state.commandBuffer(), query_obj.pool);
                    skip |= LogError(
                        vuids->vuid_graphics_support, objlist, loc.dot(Field::queryPool),
                        "(%s) was created with queryType VK_QUERY_TYPE_PIPELINE_STATISTICS (%s) and indicates graphics operations, "
                        "but "
                        "the command pool the command buffer %s was allocated from does not support graphics operations (%s).",
                        FormatHandle(query_obj.pool).c_str(),
                        string_VkQueryPipelineStatisticFlags(query_pool_ci.pipelineStatistics).c_str(),
                        FormatHandle(cb_state).c_str(), string_VkQueueFlags(cb_state.command_pool->queue_flags).c_str());
                }
            }
            if ((cb_state.command_pool->queue_flags & VK_QUEUE_COMPUTE_BIT) == 0) {
                if (query_pool_ci.pipelineStatistics & VK_QUERY_PIPELINE_STATISTIC_COMPUTE_SHADER_INVOCATIONS_BIT) {
                    const LogObjectList objlist(cb_state.commandBuffer(), query_obj.pool);
                    skip |= LogError(
                        vuids->vuid_compute_support, objlist, loc.dot(Field::queryPool),
                        "(%s) was created with queryType VK_QUERY_TYPE_PIPELINE_STATISTICS (%s) and indicates compute operations, "
                        "but "
                        "the command pool the command buffer %s was allocated from does not support compute operations (%s).",
                        FormatHandle(query_obj.pool).c_str(),
                        string_VkQueryPipelineStatisticFlags(query_pool_ci.pipelineStatistics).c_str(),
                        FormatHandle(cb_state).c_str(), string_VkQueueFlags(cb_state.command_pool->queue_flags).c_str());
                }
            }
            break;
        }
        case VK_QUERY_TYPE_PRIMITIVES_GENERATED_EXT: {
            if ((cb_state.command_pool->queue_flags & VK_QUEUE_GRAPHICS_BIT) == 0) {
                const LogObjectList objlist(cb_state.commandBuffer(), query_obj.pool);
                skip |=
                    LogError(vuids->vuid_primitives_generated, objlist, loc.dot(Field::queryPool),
                             "(%s) was created with queryType VK_QUERY_TYPE_PRIMITIVES_GENERATED_EXT, but "
                             "the command pool the command buffer %s was allocated from does not support graphics operations (%s).",
                             FormatHandle(query_obj.pool).c_str(), FormatHandle(cb_state).c_str(),
                             string_VkQueueFlags(cb_state.command_pool->queue_flags).c_str());
            }
            break;
        }
        case VK_QUERY_TYPE_RESULT_STATUS_ONLY_KHR: {
            const auto &qf_ext_props = queue_family_ext_props[cb_state.command_pool->queueFamilyIndex];
            if (!qf_ext_props.query_result_status_props.queryResultStatusSupport) {
                const LogObjectList objlist(cb_state.commandBuffer(), query_obj.pool);
                skip |= LogError(vuids->vuid_result_status_support, objlist, loc,
                                 "the command pool's queue family (index %u) the command buffer %s was allocated "
                                 "from does not support result status queries.",
                                 cb_state.command_pool->queueFamilyIndex, FormatHandle(cb_state).c_str());
            }
            break;
        }
        case VK_QUERY_TYPE_MESH_PRIMITIVES_GENERATED_EXT: {
            if (loc.function == Func::vkCmdBeginQueryIndexedEXT) {
                const LogObjectList objlist(cb_state.commandBuffer(), query_obj.pool);
                skip |= LogError("VUID-vkCmdBeginQueryIndexedEXT-queryType-07071", objlist, loc.dot(Field::queryPool),
                                 "(%s) was created with queryType %s.", FormatHandle(query_obj.pool).c_str(),
                                 string_VkQueryType(query_pool_ci.queryType));
            } else {
                skip |= ValidateCmdQueueFlags(cb_state, loc, VK_QUEUE_GRAPHICS_BIT, "VUID-vkCmdBeginQuery-queryType-07070");
            }
            break;
        }
        default:
            break;
    }

    // Check for nested queries
    if (cb_state.activeQueries.size()) {
        for (const auto &active_query_obj : cb_state.activeQueries) {
            auto active_query_pool_state = Get<QUERY_POOL_STATE>(active_query_obj.pool);
            if (active_query_pool_state->createInfo.queryType == query_pool_ci.queryType && active_query_obj.index == index) {
                const LogObjectList objlist(cb_state.commandBuffer(), query_obj.pool, active_query_obj.pool);
                skip |= LogError(vuids->vuid_dup_query_type, objlist, loc,
                                 "Within the same command buffer %s, query %d from pool %s has same queryType as active query "
                                 "%d from pool %s.",
                                 FormatHandle(cb_state).c_str(), query_obj.index, FormatHandle(query_obj.pool).c_str(),
                                 active_query_obj.index, FormatHandle(active_query_obj.pool).c_str());
            }
        }
    }

    if (flags & VK_QUERY_CONTROL_PRECISE_BIT) {
        if (!enabled_features.occlusionQueryPrecise) {
            skip |= LogError(vuids->vuid_precise, cb_state.commandBuffer(), loc.dot(Field::flags),
                             "includes VK_QUERY_CONTROL_PRECISE_BIT, but occlusionQueryPrecise feature was not enabled.");
        }

        if (query_pool_ci.queryType != VK_QUERY_TYPE_OCCLUSION) {
            const LogObjectList objlist(cb_state.commandBuffer(), query_obj.pool);
            skip |= LogError(vuids->vuid_precise, objlist, loc.dot(Field::flags),
                             "includes VK_QUERY_CONTROL_PRECISE_BIT provided, but pool query type is not VK_QUERY_TYPE_OCCLUSION.");
        }
    }

    if (query_obj.slot >= query_pool_ci.queryCount) {
        const LogObjectList objlist(cb_state.commandBuffer(), query_obj.pool);
        skip |= LogError(vuids->vuid_query_count, objlist, loc,
                         "Query index %" PRIu32 " must be less than query count %" PRIu32 " of %s.", query_obj.slot,
                         query_pool_ci.queryCount, FormatHandle(query_obj.pool).c_str());
    }

    if (cb_state.unprotected == false) {
        skip |= LogError(vuids->vuid_protected_cb, cb_state.commandBuffer(), loc,
                         "command can't be used in protected command buffers.");
    }

    if (cb_state.activeRenderPass) {
        const auto *render_pass_info = cb_state.activeRenderPass->createInfo.ptr();
        if (!cb_state.activeRenderPass->UsesDynamicRendering()) {
            const auto *subpass_desc = &render_pass_info->pSubpasses[cb_state.GetActiveSubpass()];
            if (subpass_desc) {
                uint32_t bits = GetBitSetCount(subpass_desc->viewMask);
                if (query_obj.slot + bits > query_pool_state->createInfo.queryCount) {
                    const LogObjectList objlist(cb_state.commandBuffer(), query_obj.pool);
                    skip |= LogError(vuids->vuid_multiview_query, objlist, loc,
                                     "query (%" PRIu32 ") + bits set in current subpass view mask (%" PRIx32
                                     ") is greater than the number of queries in queryPool (%" PRIu32 ").",
                                     query_obj.slot, subpass_desc->viewMask, query_pool_state->createInfo.queryCount);
                }
            }
        }
    }

    if (cb_state.bound_video_session) {
        if (cb_state.activeQueries.size() > 0) {
            LogObjectList objlist(cb_state.commandBuffer());
            objlist.add(cb_state.bound_video_session->Handle());
            skip |= LogError(vuids->vuid_no_active_in_vc_scope, objlist, loc,
                             "cannot start another query while there is already an active query in a "
                             "video coding scope (%s is bound)",
                             FormatHandle(cb_state.bound_video_session->videoSession()).c_str());
        }

        if (cb_state.bound_video_session->profile != query_pool_state->supported_video_profile) {
            LogObjectList objlist(cb_state.commandBuffer());
            objlist.add(query_pool_state->pool());
            objlist.add(cb_state.bound_video_session->Handle());
            skip |= LogError(vuids->vuid_result_status_profile_in_vc_scope, objlist, loc,
                             "the video profile %s was created with does not match the video profile of %s",
                             FormatHandle(query_pool_state->pool()).c_str(),
                             FormatHandle(cb_state.bound_video_session->videoSession()).c_str());
        }

        if (query_pool_ci.queryType != VK_QUERY_TYPE_RESULT_STATUS_ONLY_KHR) {
            LogObjectList objlist(cb_state.commandBuffer());
            objlist.add(cb_state.bound_video_session->Handle());
            skip |= LogError(vuids->vuid_vc_scope_query_type, objlist, loc,
                             "invalid query type used in a video coding scope (%s is bound)",
                             FormatHandle(cb_state.bound_video_session->videoSession()).c_str());
        }
    }

    return skip;
}

bool CoreChecks::PreCallValidateCmdBeginQuery(VkCommandBuffer commandBuffer, VkQueryPool queryPool, uint32_t slot,
                                              VkQueryControlFlags flags, const ErrorObject &error_obj) const {
    if (disabled[query_validation]) return false;
    bool skip = false;
    auto cb_state = GetRead<CMD_BUFFER_STATE>(commandBuffer);
    assert(cb_state);
    QueryObject query_obj = {queryPool, slot};
    auto query_pool_state = Get<QUERY_POOL_STATE>(query_obj.pool);
    if (query_pool_state->createInfo.queryType == VK_QUERY_TYPE_PRIMITIVES_GENERATED_EXT) {
        if (!enabled_features.primitivesGeneratedQuery) {
            skip |= LogError("VUID-vkCmdBeginQuery-queryType-06688", device, error_obj.location.dot(Field::queryPool),
                             "was created with a queryType VK_QUERY_TYPE_PRIMITIVES_GENERATED_EXT, but "
                             "primitivesGeneratedQuery feature was not enabled.");
        }
    }
    struct BeginQueryVuids : ValidateBeginQueryVuids {
        BeginQueryVuids() : ValidateBeginQueryVuids() {
            vuid_queue_feedback = "VUID-vkCmdBeginQuery-queryType-02327";
            vuid_queue_occlusion = "VUID-vkCmdBeginQuery-queryType-00803";
            vuid_precise = "VUID-vkCmdBeginQuery-queryType-00800";
            vuid_query_count = "VUID-vkCmdBeginQuery-query-00802";
            vuid_profile_lock = "VUID-vkCmdBeginQuery-queryPool-03223";
            vuid_scope_not_first = "VUID-vkCmdBeginQuery-queryPool-03224";
            vuid_scope_in_rp = "VUID-vkCmdBeginQuery-queryPool-03225";
            vuid_dup_query_type = "VUID-vkCmdBeginQuery-queryPool-01922";
            vuid_protected_cb = "VUID-vkCmdBeginQuery-commandBuffer-01885";
            vuid_multiview_query = "VUID-vkCmdBeginQuery-query-00808";
            vuid_graphics_support = "VUID-vkCmdBeginQuery-queryType-00804";
            vuid_compute_support = "VUID-vkCmdBeginQuery-queryType-00805";
            vuid_primitives_generated = "VUID-vkCmdBeginQuery-queryType-06687";
            vuid_result_status_support = "VUID-vkCmdBeginQuery-queryType-07126";
            vuid_no_active_in_vc_scope = "VUID-vkCmdBeginQuery-None-07127";
            vuid_result_status_profile_in_vc_scope = "VUID-vkCmdBeginQuery-queryType-07128";
            vuid_vc_scope_query_type = "VUID-vkCmdBeginQuery-queryType-07131";
        }
    };
    BeginQueryVuids vuids;
    skip |= ValidateBeginQuery(*cb_state, query_obj, flags, 0, error_obj.location, &vuids);
    skip |= ValidateCmd(*cb_state, error_obj.location);
    return skip;
}

bool CoreChecks::VerifyQueryIsReset(const CMD_BUFFER_STATE &cb_state, const QueryObject &query_obj, Func command,
                                    VkQueryPool &firstPerfQueryPool, uint32_t perfPass, QueryMap *localQueryToStateMap) {
    bool skip = false;
    auto state_data = cb_state.dev_data;

    auto query_pool_state = state_data->Get<QUERY_POOL_STATE>(query_obj.pool);
    const auto &query_pool_ci = query_pool_state->createInfo;

    QueryState state = GetLocalQueryState(localQueryToStateMap, query_obj.pool, query_obj.slot, perfPass);
    // If reset was in another command buffer, check the global map
    if (state == QUERYSTATE_UNKNOWN) {
        state = query_pool_state->GetQueryState(query_obj.slot, perfPass);
    }
    // Performance queries have limitation upon when they can be
    // reset.
    if (query_pool_ci.queryType == VK_QUERY_TYPE_PERFORMANCE_QUERY_KHR && state == QUERYSTATE_UNKNOWN &&
        perfPass >= query_pool_state->n_performance_passes) {
        // If the pass is invalid, assume RESET state, another error
        // will be raised in ValidatePerformanceQuery().
        state = QUERYSTATE_RESET;
    }

    if (state != QUERYSTATE_RESET) {
        const LogObjectList objlist(cb_state.commandBuffer(), query_obj.pool);
        const Location loc(command);
        const char *vuid = (command == Func::vkCmdBeginQuery)             ? "VUID-vkCmdBeginQuery-None-00807"
                           : (command == Func::vkCmdBeginQueryIndexedEXT) ? "VUID-vkCmdBeginQueryIndexedEXT-None-00807"
                           : (command == Func::vkCmdWriteTimestamp)       ? "VUID-vkCmdWriteTimestamp-None-00830"
                                                                          : "VUID-vkCmdWriteTimestamp2-None-03864";
        skip |= state_data->LogError(vuid, objlist, loc,
                                     "%s and query %" PRIu32
                                     ": query not reset. "
                                     "After query pool creation, each query must be reset before it is used. "
                                     "Queries must also be reset between uses.",
                                     state_data->FormatHandle(query_obj.pool).c_str(), query_obj.slot);
    }

    return skip;
}

bool CoreChecks::ValidatePerformanceQuery(const CMD_BUFFER_STATE &cb_state, const QueryObject &query_obj, Func command,
                                          VkQueryPool &firstPerfQueryPool, uint32_t perfPass, QueryMap *localQueryToStateMap) {
    auto state_data = cb_state.dev_data;
    auto query_pool_state = state_data->Get<QUERY_POOL_STATE>(query_obj.pool);
    const auto &query_pool_ci = query_pool_state->createInfo;
    const Location loc(command);

    if (query_pool_ci.queryType != VK_QUERY_TYPE_PERFORMANCE_QUERY_KHR) return false;

    bool skip = false;

    if (perfPass >= query_pool_state->n_performance_passes) {
        const LogObjectList objlist(cb_state.commandBuffer(), query_obj.pool);
        skip |= state_data->LogError("VUID-VkPerformanceQuerySubmitInfoKHR-counterPassIndex-03221", objlist, loc,
                                     "Invalid counterPassIndex (%u, maximum allowed %u) value for query pool %s.", perfPass,
                                     query_pool_state->n_performance_passes, state_data->FormatHandle(query_obj.pool).c_str());
    }

    if (!cb_state.performance_lock_acquired || cb_state.performance_lock_released) {
        const LogObjectList objlist(cb_state.commandBuffer(), query_obj.pool);
        skip |= state_data->LogError("VUID-vkQueueSubmit-pCommandBuffers-03220", objlist, loc,
                                     "Commandbuffer %s was submitted and contains a performance query but the"
                                     "profiling lock was not held continuously throughout the recording of commands.",
                                     state_data->FormatHandle(cb_state).c_str());
    }

    QueryState command_buffer_state = GetLocalQueryState(localQueryToStateMap, query_obj.pool, query_obj.slot, perfPass);
    if (command_buffer_state == QUERYSTATE_RESET) {
        const LogObjectList objlist(cb_state.commandBuffer(), query_obj.pool);
        skip |= state_data->LogError(
            query_obj.indexed ? "VUID-vkCmdBeginQueryIndexedEXT-None-02863" : "VUID-vkCmdBeginQuery-None-02863", objlist, loc,
            "VkQuery begin command recorded in a command buffer that, either directly or "
            "through secondary command buffers, also contains a vkCmdResetQueryPool command "
            "affecting the same query.");
    }

    if (firstPerfQueryPool != VK_NULL_HANDLE) {
        if (firstPerfQueryPool != query_obj.pool && !state_data->enabled_features.performanceCounterMultipleQueryPools) {
            const LogObjectList objlist(cb_state.commandBuffer(), query_obj.pool);
            skip |= state_data->LogError(
                query_obj.indexed ? "VUID-vkCmdBeginQueryIndexedEXT-queryPool-03226" : "VUID-vkCmdBeginQuery-queryPool-03226",
                objlist, loc,
                "Commandbuffer %s contains more than one performance query pool but "
                "performanceCounterMultipleQueryPools is not enabled.",
                state_data->FormatHandle(cb_state).c_str());
        }
    } else {
        firstPerfQueryPool = query_obj.pool;
    }

    return skip;
}

void CoreChecks::EnqueueVerifyBeginQuery(VkCommandBuffer command_buffer, const QueryObject &query_obj, Func command) {
    auto cb_state = GetWrite<CMD_BUFFER_STATE>(command_buffer);

    // Enqueue the submit time validation here, ahead of the submit time state update in the StateTracker's PostCallRecord
    cb_state->queryUpdates.emplace_back([query_obj, command](CMD_BUFFER_STATE &cb_state_arg, bool do_validate,
                                                             VkQueryPool &firstPerfQueryPool, uint32_t perfPass,
                                                             QueryMap *localQueryToStateMap) {
        if (!do_validate) return false;
        bool skip = false;
        skip |= ValidatePerformanceQuery(cb_state_arg, query_obj, command, firstPerfQueryPool, perfPass, localQueryToStateMap);
        skip |= VerifyQueryIsReset(cb_state_arg, query_obj, command, firstPerfQueryPool, perfPass, localQueryToStateMap);
        return skip;
    });
}

void CoreChecks::PreCallRecordCmdBeginQuery(VkCommandBuffer commandBuffer, VkQueryPool queryPool, uint32_t slot,
                                            VkQueryControlFlags flags) {
    if (disabled[query_validation]) return;
    QueryObject query_obj = {queryPool, slot};
    EnqueueVerifyBeginQuery(commandBuffer, query_obj, Func::vkCmdBeginQuery);
}

void CoreChecks::EnqueueVerifyEndQuery(CMD_BUFFER_STATE &cb_state, const QueryObject &query_obj, Func command) {
    // Enqueue the submit time validation here, ahead of the submit time state update in the StateTracker's PostCallRecord
    cb_state.queryUpdates.emplace_back([this, query_obj, command](CMD_BUFFER_STATE &cb_state_arg, bool do_validate,
                                                                  VkQueryPool &firstPerfQueryPool, uint32_t perfPass,
                                                                  QueryMap *localQueryToStateMap) {
        if (!do_validate) return false;
        bool skip = false;
        // NOTE: dev_data == this, but the compiler "Visual Studio 16" complains Get is ambiguous if dev_data isn't used
        auto query_pool_state = cb_state_arg.dev_data->Get<QUERY_POOL_STATE>(query_obj.pool);
        if (query_pool_state->has_perf_scope_command_buffer && (cb_state_arg.command_count - 1) != query_obj.end_command_index) {
            const LogObjectList objlist(cb_state_arg.commandBuffer(), query_pool_state->pool());
            const Location loc(command);
            skip |= LogError("VUID-vkCmdEndQuery-queryPool-03227", objlist, loc,
                             "Query pool %s was created with a counter of scope "
                             "VK_QUERY_SCOPE_COMMAND_BUFFER_KHR but the end of the query is not the last "
                             "command in the command buffer %s.",
                             FormatHandle(query_obj.pool).c_str(), FormatHandle(cb_state_arg).c_str());
        }
        return skip;
    });
}

bool CoreChecks::ValidateCmdEndQuery(const CMD_BUFFER_STATE &cb_state, const QueryObject &query_obj, uint32_t index,
                                     const Location &loc, const ValidateEndQueryVuids *vuids) const {
    bool skip = false;
    if (!cb_state.activeQueries.count(query_obj)) {
        const LogObjectList objlist(cb_state.commandBuffer(), query_obj.pool);
        skip |= LogError(vuids->vuid_active_queries, objlist, loc, "Ending a query before it was started: %s, index %d.",
                         FormatHandle(query_obj.pool).c_str(), query_obj.slot);
    }
    auto query_pool_state = Get<QUERY_POOL_STATE>(query_obj.pool);
    const auto &query_pool_ci = query_pool_state->createInfo;
    if (query_pool_ci.queryType == VK_QUERY_TYPE_PERFORMANCE_QUERY_KHR) {
        if (query_pool_state->has_perf_scope_render_pass && cb_state.activeRenderPass) {
            const LogObjectList objlist(cb_state.commandBuffer(), query_obj.pool);
            skip |= LogError("VUID-vkCmdEndQuery-queryPool-03228", objlist, loc,
                             "Query pool %s was created with a counter of scope "
                             "VK_QUERY_SCOPE_RENDER_PASS_KHR but %s is inside a render pass.",
                             FormatHandle(query_obj.pool).c_str(), loc.StringFunc());
        }
    }

    if (cb_state.unprotected == false) {
        skip |= LogError(vuids->vuid_protected_cb, cb_state.commandBuffer(), loc,
                         "command can't be used in protected command buffers.");
    }
    if (cb_state.activeRenderPass) {
        const auto *render_pass_info = cb_state.activeRenderPass->createInfo.ptr();
        if (!cb_state.activeRenderPass->UsesDynamicRendering()) {
            const auto *subpass_desc = &render_pass_info->pSubpasses[cb_state.GetActiveSubpass()];
            if (subpass_desc) {
                const uint32_t bits = GetBitSetCount(subpass_desc->viewMask);
                if (query_obj.slot + bits > query_pool_state->createInfo.queryCount) {
                    const LogObjectList objlist(cb_state.commandBuffer(), query_obj.pool);
                    skip |= LogError(vuids->vuid_multiview_query, objlist, loc,
                                     "query (%" PRIu32 ") + bits set in current subpass view mask (%" PRIx32
                                     ") is greater than the number of queries in queryPool (%" PRIu32 ").",
                                     query_obj.slot, subpass_desc->viewMask, query_pool_state->createInfo.queryCount);
                }
            }
        }
    }
    return skip;
}

bool CoreChecks::PreCallValidateCmdEndQuery(VkCommandBuffer commandBuffer, VkQueryPool queryPool, uint32_t slot,
                                            const ErrorObject &error_obj) const {
    if (disabled[query_validation]) return false;
    bool skip = false;
    QueryObject query_obj = {queryPool, slot};
    auto cb_state = GetRead<CMD_BUFFER_STATE>(commandBuffer);
    assert(cb_state);

    const auto &query_pool_state = *Get<QUERY_POOL_STATE>(queryPool);
    const uint32_t available_query_count = query_pool_state.createInfo.queryCount;
    // Only continue validating if the slot is even within range
    if (slot >= available_query_count) {
        const LogObjectList objlist(commandBuffer, query_obj.pool);
        skip |= LogError("VUID-vkCmdEndQuery-query-00810", objlist, error_obj.location.dot(Field::query),
                         "(%u) is greater or equal to the queryPool size (%u).", slot, available_query_count);
    } else {
        struct EndQueryVuids : ValidateEndQueryVuids {
            EndQueryVuids() : ValidateEndQueryVuids() {
                vuid_active_queries = "VUID-vkCmdEndQuery-None-01923";
                vuid_protected_cb = "VUID-vkCmdEndQuery-commandBuffer-01886";
                vuid_multiview_query = "VUID-vkCmdEndQuery-query-00812";
            }
        };
        EndQueryVuids vuids;
        skip |= ValidateCmdEndQuery(*cb_state, query_obj, 0, error_obj.location, &vuids);
        skip |= ValidateCmd(*cb_state, error_obj.location);
    }
    return skip;
}

void CoreChecks::PreCallRecordCmdEndQuery(VkCommandBuffer commandBuffer, VkQueryPool queryPool, uint32_t slot) {
    if (disabled[query_validation]) return;
    auto cb_state = GetWrite<CMD_BUFFER_STATE>(commandBuffer);
    QueryObject query_obj = {queryPool, slot};
    query_obj.end_command_index = cb_state->command_count;  // off by one because cb_state hasn't recorded this yet
    EnqueueVerifyEndQuery(*cb_state, query_obj, Func::vkCmdEndQuery);
}

bool CoreChecks::ValidateQueryPoolIndex(const QUERY_POOL_STATE &query_pool_state, uint32_t firstQuery, uint32_t queryCount,
                                        const Location &loc, const char *first_vuid, const char *sum_vuid) const {
    bool skip = false;
    const uint32_t available_query_count = query_pool_state.createInfo.queryCount;
    if (firstQuery >= available_query_count) {
        skip |= LogError(first_vuid, query_pool_state.pool(), loc,
                         "In Query %s the firstQuery (%" PRIu32 ") is greater or equal to the queryPool size (%" PRIu32 ").",
                         FormatHandle(query_pool_state).c_str(), firstQuery, available_query_count);
    }
    if ((firstQuery + queryCount) > available_query_count) {
        skip |= LogError(sum_vuid, query_pool_state.pool(), loc,
                         "In Query %s the sum of firstQuery (%" PRIu32 ") + queryCount (%" PRIu32
                         ") is greater than the queryPool size (%" PRIu32 ").",
                         FormatHandle(query_pool_state).c_str(), firstQuery, queryCount, available_query_count);
    }
    return skip;
}

bool CoreChecks::ValidateQueriesNotActive(const CMD_BUFFER_STATE &cb_state, VkQueryPool queryPool, uint32_t firstQuery,
                                          uint32_t queryCount, const Location &loc, const char *vuid) const {
    bool skip = false;
    for (uint32_t i = 0; i < queryCount; i++) {
        const uint32_t slot = firstQuery + i;
        QueryObject query_obj = {queryPool, slot};
        if (cb_state.activeQueries.count(query_obj)) {
            const LogObjectList objlist(cb_state.commandBuffer(), queryPool);
            skip |= LogError(vuid, objlist, loc,
                             "Query index %" PRIu32 " is still active (firstQuery = %" PRIu32 ", queryCount = %" PRIu32 ").", slot,
                             firstQuery, queryCount);
        }
    }
    return skip;
}

bool CoreChecks::PreCallValidateCmdResetQueryPool(VkCommandBuffer commandBuffer, VkQueryPool queryPool, uint32_t firstQuery,
                                                  uint32_t queryCount, const ErrorObject &error_obj) const {
    if (disabled[query_validation]) return false;
    auto cb_state = GetRead<CMD_BUFFER_STATE>(commandBuffer);
    assert(cb_state);

    bool skip = false;
    skip |= ValidateCmd(*cb_state, error_obj.location);
    const auto &query_pool_state = *Get<QUERY_POOL_STATE>(queryPool);
    skip |= ValidateQueryPoolIndex(query_pool_state, firstQuery, queryCount, error_obj.location,
                                   "VUID-vkCmdResetQueryPool-firstQuery-00796", "VUID-vkCmdResetQueryPool-firstQuery-00797");
    skip |= ValidateQueriesNotActive(*cb_state, queryPool, firstQuery, queryCount, error_obj.location,
                                     "VUID-vkCmdResetQueryPool-None-02841");

    return skip;
}

void CoreChecks::PreCallRecordCmdResetQueryPool(VkCommandBuffer commandBuffer, VkQueryPool queryPool, uint32_t firstQuery,
                                                uint32_t queryCount) {
    if (disabled[query_validation]) return;
    auto cb_state = GetWrite<CMD_BUFFER_STATE>(commandBuffer);
    const auto &query_pool_state = *Get<QUERY_POOL_STATE>(queryPool);
    if (query_pool_state.createInfo.queryType == VK_QUERY_TYPE_PERFORMANCE_QUERY_KHR) {
        cb_state->queryUpdates.emplace_back([queryPool, firstQuery, queryCount](CMD_BUFFER_STATE &cb_state_arg, bool do_validate,
                                                                                VkQueryPool &firstPerfQueryPool, uint32_t perfPass,
                                                                                QueryMap *localQueryToStateMap) {
            if (!do_validate) return false;
            const auto state_data = cb_state_arg.dev_data;
            bool skip = false;
            for (uint32_t i = 0; i < queryCount; i++) {
                QueryState state = GetLocalQueryState(localQueryToStateMap, queryPool, firstQuery + i, perfPass);
                if (state == QUERYSTATE_ENDED) {
                    const LogObjectList objlist(cb_state_arg.commandBuffer(), queryPool);
                    const Location loc(Func::vkCmdResetQueryPool);
                    skip |= state_data->LogError("VUID-vkCmdResetQueryPool-firstQuery-02862", objlist, loc,
                                                 "Query index %" PRIu32 " was begun and reset in the same command buffer.",
                                                 firstQuery + i);
                    break;
                }
            }
            return skip;
        });
    }
}

static QueryResultType GetQueryResultType(QueryState state, VkQueryResultFlags flags) {
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

bool CoreChecks::PreCallValidateCmdCopyQueryPoolResults(VkCommandBuffer commandBuffer, VkQueryPool queryPool, uint32_t firstQuery,
                                                        uint32_t queryCount, VkBuffer dstBuffer, VkDeviceSize dstOffset,
                                                        VkDeviceSize stride, VkQueryResultFlags flags,
                                                        const ErrorObject &error_obj) const {
    if (disabled[query_validation]) return false;
    auto cb_state = GetRead<CMD_BUFFER_STATE>(commandBuffer);
    auto dst_buff_state = Get<BUFFER_STATE>(dstBuffer);
    assert(cb_state);
    assert(dst_buff_state);
    const LogObjectList buffer_objlist(commandBuffer, dstBuffer);
    bool skip = ValidateMemoryIsBoundToBuffer(commandBuffer, *dst_buff_state, error_obj.location.dot(Field::dstBuffer),
                                              "VUID-vkCmdCopyQueryPoolResults-dstBuffer-00826");
    skip |= ValidateQueryPoolStride("VUID-vkCmdCopyQueryPoolResults-flags-00822", "VUID-vkCmdCopyQueryPoolResults-flags-00823",
                                    stride, "dstOffset", dstOffset, flags, error_obj.location);
    // Validate that DST buffer has correct usage flags set
    skip |= ValidateBufferUsageFlags(buffer_objlist, *dst_buff_state, VK_BUFFER_USAGE_TRANSFER_DST_BIT, true,
                                     "VUID-vkCmdCopyQueryPoolResults-dstBuffer-00825", error_obj.location.dot(Field::dstBuffer));
    skip |= ValidateCmd(*cb_state, error_obj.location);

    if (dstOffset >= dst_buff_state->requirements.size) {
        skip |= LogError("VUID-vkCmdCopyQueryPoolResults-dstOffset-00819", buffer_objlist, error_obj.location.dot(Field::dstOffset),
                         "(%" PRIu64 ") is not less than the size (%" PRIu64 ") of buffer (%s).", dstOffset,
                         dst_buff_state->requirements.size, FormatHandle(dst_buff_state->buffer()).c_str());
    } else if (dstOffset + (queryCount * stride) > dst_buff_state->requirements.size) {
        skip |= LogError("VUID-vkCmdCopyQueryPoolResults-dstBuffer-00824", buffer_objlist, error_obj.location,
                         "storage required (%" PRIu64
                         ") equal to dstOffset + (queryCount * stride) is greater than the size (%" PRIu64 ") of buffer (%s).",
                         dstOffset + (queryCount * stride), dst_buff_state->requirements.size,
                         FormatHandle(dst_buff_state->buffer()).c_str());
    }

    if ((flags & VK_QUERY_RESULT_WITH_STATUS_BIT_KHR) && (flags & VK_QUERY_RESULT_WITH_AVAILABILITY_BIT)) {
        skip |= LogError("VUID-vkCmdCopyQueryPoolResults-flags-06902", commandBuffer, error_obj.location.dot(Field::flags),
                         "(%s) include both STATUS_BIT and AVAILABILITY_BIT.", string_VkQueryResultFlags(flags).c_str());
    }

    const auto &query_pool_state = *Get<QUERY_POOL_STATE>(queryPool);
    skip |= ValidateQueryPoolIndex(query_pool_state, firstQuery, queryCount, error_obj.location,
                                   "VUID-vkCmdCopyQueryPoolResults-firstQuery-00820",
                                   "VUID-vkCmdCopyQueryPoolResults-firstQuery-00821");
    skip |= ValidateQueriesNotActive(*cb_state, queryPool, firstQuery, queryCount, error_obj.location,
                                     "VUID-vkCmdCopyQueryPoolResults-None-07429");

    if (query_pool_state.createInfo.queryType == VK_QUERY_TYPE_PERFORMANCE_QUERY_KHR) {
        skip |= ValidatePerformanceQueryResults(query_pool_state, firstQuery, queryCount, flags, error_obj.location);
        if (!phys_dev_ext_props.performance_query_props.allowCommandBufferQueryCopies) {
            const LogObjectList objlist(commandBuffer, queryPool);
            skip |= LogError("VUID-vkCmdCopyQueryPoolResults-queryType-03232", objlist, error_obj.location.dot(Field::queryPool),
                             "(%s) was created with VK_QUERY_TYPE_PERFORMANCE_QUERY_KHR but "
                             "VkPhysicalDevicePerformanceQueryPropertiesKHR::allowCommandBufferQueryCopies is not supported.",
                             FormatHandle(queryPool).c_str());
        }
    }
    if ((query_pool_state.createInfo.queryType == VK_QUERY_TYPE_TIMESTAMP) && ((flags & VK_QUERY_RESULT_PARTIAL_BIT) != 0)) {
        const LogObjectList objlist(commandBuffer, queryPool);
        skip |= LogError("VUID-vkCmdCopyQueryPoolResults-queryType-00827", objlist, error_obj.location.dot(Field::flags),
                         "(%s) includes VK_QUERY_RESULT_PARTIAL_BIT, but %s was created with VK_QUERY_TYPE_TIMESTAMP.",
                         string_VkQueryResultFlags(flags).c_str(), FormatHandle(queryPool).c_str());
    }
    if (query_pool_state.createInfo.queryType == VK_QUERY_TYPE_PERFORMANCE_QUERY_INTEL) {
        const LogObjectList objlist(commandBuffer, queryPool);
        skip |= LogError("VUID-vkCmdCopyQueryPoolResults-queryType-02734", objlist, error_obj.location.dot(Field::queryPool),
                         "(%s) was created with queryType VK_QUERY_TYPE_PERFORMANCE_QUERY_INTEL.", FormatHandle(queryPool).c_str());
    }
    if (query_pool_state.createInfo.queryType == VK_QUERY_TYPE_RESULT_STATUS_ONLY_KHR &&
        (flags & VK_QUERY_RESULT_WITH_STATUS_BIT_KHR) == 0) {
        const LogObjectList objlist(commandBuffer, queryPool);
        skip |= LogError("VUID-vkCmdCopyQueryPoolResults-queryType-06901", objlist, error_obj.location.dot(Field::flags),
                         "(%s) does not include VK_QUERY_RESULT_WITH_STATUS_BIT_KHR, but %s was created with queryType "
                         "VK_QUERY_TYPE_RESULT_STATUS_ONLY_KHR.",
                         string_VkQueryResultFlags(flags).c_str(), FormatHandle(queryPool).c_str());
    }

    return skip;
}

void CoreChecks::PreCallRecordCmdCopyQueryPoolResults(VkCommandBuffer commandBuffer, VkQueryPool queryPool, uint32_t firstQuery,
                                                      uint32_t queryCount, VkBuffer dstBuffer, VkDeviceSize dstOffset,
                                                      VkDeviceSize stride, VkQueryResultFlags flags) {
    if (disabled[query_validation]) return;
    auto cb_state = GetWrite<CMD_BUFFER_STATE>(commandBuffer);
    cb_state->queryUpdates.emplace_back([queryPool, firstQuery, queryCount, flags, this](
                                            CMD_BUFFER_STATE &cb_state_arg, bool do_validate, VkQueryPool &firstPerfQueryPool,
                                            uint32_t perfPass, QueryMap *localQueryToStateMap) {
        if (!do_validate) return false;
        const auto state_data = cb_state_arg.dev_data;
        bool skip = false;
        const Location loc(Func::vkCmdCopyQueryPoolResults);
        for (uint32_t i = 0; i < queryCount; i++) {
            QueryState state = GetLocalQueryState(localQueryToStateMap, queryPool, firstQuery + i, perfPass);
            QueryResultType result_type = GetQueryResultType(state, flags);
            if (result_type != QUERYRESULT_SOME_DATA && result_type != QUERYRESULT_UNKNOWN) {
                const LogObjectList objlist(cb_state_arg.commandBuffer(), queryPool);
                skip |= state_data->LogError("VUID-vkCmdCopyQueryPoolResults-None-08752", objlist, loc,
                                             "Requesting a copy from query to buffer on %s query %" PRIu32 ": %s",
                                             state_data->FormatHandle(queryPool).c_str(), firstQuery + i,
                                             string_QueryResultType(result_type));
            }
        }

        // NOTE: dev_data == this, but the compiler "Visual Studio 16" complains Get is ambiguous if dev_data isn't used
        auto query_pool_state = cb_state_arg.dev_data->Get<QUERY_POOL_STATE>(queryPool);
        skip |= ValidateQueryPoolWasReset(*query_pool_state, firstQuery, queryCount, loc, localQueryToStateMap, perfPass);

        return skip;
    });
}

bool CoreChecks::ValidateCmdWriteTimestamp(const CMD_BUFFER_STATE &cb_state, VkQueryPool queryPool, uint32_t slot,
                                           const Location &loc) const {
    bool skip = false;
    skip |= ValidateCmd(cb_state, loc);
    const bool is_2 = loc.function == Func::vkCmdWriteTimestamp2 || loc.function == Func::vkCmdWriteTimestamp2KHR;

    const uint32_t timestamp_valid_bits =
        physical_device_state->queue_family_properties[cb_state.command_pool->queueFamilyIndex].timestampValidBits;
    if (timestamp_valid_bits == 0) {
        const char *vuid =
            is_2 ? "VUID-vkCmdWriteTimestamp2-timestampValidBits-03863" : "VUID-vkCmdWriteTimestamp-timestampValidBits-00829";
        const LogObjectList objlist(cb_state.commandBuffer(), queryPool);
        skip |= LogError(vuid, objlist, loc, "Query Pool %s has a timestampValidBits value of zero for queueFamilyIndex %u.",
                         FormatHandle(queryPool).c_str(), cb_state.command_pool->queueFamilyIndex);
    }

    const auto &query_pool_state = *Get<QUERY_POOL_STATE>(queryPool);
    if (query_pool_state.createInfo.queryType != VK_QUERY_TYPE_TIMESTAMP) {
        const char *vuid = is_2 ? "VUID-vkCmdWriteTimestamp2-queryPool-03861" : "VUID-vkCmdWriteTimestamp-queryPool-01416";
        const LogObjectList objlist(cb_state.commandBuffer(), queryPool);
        skip |= LogError(vuid, objlist, loc, "Query Pool %s was not created with VK_QUERY_TYPE_TIMESTAMP.",
                         FormatHandle(queryPool).c_str());
    }

    if (slot >= query_pool_state.createInfo.queryCount) {
        const char *vuid = is_2 ? "VUID-vkCmdWriteTimestamp2-query-04903" : "VUID-vkCmdWriteTimestamp-query-04904";
        const LogObjectList objlist(cb_state.commandBuffer(), queryPool);
        skip |= LogError(vuid, objlist, loc,
                         "query (%" PRIu32 ") is not lower than the number of queries (%" PRIu32 ") in Query pool %s.", slot,
                         query_pool_state.createInfo.queryCount, FormatHandle(queryPool).c_str());
    }

    return skip;
}

bool CoreChecks::PreCallValidateCmdWriteTimestamp(VkCommandBuffer commandBuffer, VkPipelineStageFlagBits pipelineStage,
                                                  VkQueryPool queryPool, uint32_t slot, const ErrorObject &error_obj) const {
    if (disabled[query_validation]) return false;
    auto cb_state = GetRead<CMD_BUFFER_STATE>(commandBuffer);
    assert(cb_state);
    bool skip = false;
    skip |= ValidateCmdWriteTimestamp(*cb_state, queryPool, slot, error_obj.location);

    const Location stage_loc = error_obj.location.dot(Field::pipelineStage);
    skip |= ValidatePipelineStage(LogObjectList(commandBuffer), stage_loc, cb_state->GetQueueFlags(), pipelineStage);
    return skip;
}

bool CoreChecks::PreCallValidateCmdWriteTimestamp2(VkCommandBuffer commandBuffer, VkPipelineStageFlags2 stage,
                                                   VkQueryPool queryPool, uint32_t slot, const ErrorObject &error_obj) const {
    if (disabled[query_validation]) return false;
    auto cb_state = GetRead<CMD_BUFFER_STATE>(commandBuffer);
    assert(cb_state);
    bool skip = false;
    skip |= ValidateCmdWriteTimestamp(*cb_state, queryPool, slot, error_obj.location);

    if (!enabled_features.synchronization2) {
        skip |= LogError("VUID-vkCmdWriteTimestamp2-synchronization2-03858", commandBuffer, error_obj.location,
                         "Synchronization2 feature is not enabled.");
    }

    const Location stage_loc = error_obj.location.dot(Field::stage);
    if ((stage & (stage - 1)) != 0) {
        skip |= LogError("VUID-vkCmdWriteTimestamp2-stage-03859", commandBuffer, stage_loc,
                         "(%s) must only set a single pipeline stage.", string_VkPipelineStageFlags2(stage).c_str());
    }
    skip |= ValidatePipelineStage(LogObjectList(commandBuffer), stage_loc, cb_state->GetQueueFlags(), stage);
    return skip;
}

bool CoreChecks::PreCallValidateCmdWriteTimestamp2KHR(VkCommandBuffer commandBuffer, VkPipelineStageFlags2KHR stage,
                                                      VkQueryPool queryPool, uint32_t query, const ErrorObject &error_obj) const {
    return PreCallValidateCmdWriteTimestamp2(commandBuffer, stage, queryPool, query, error_obj);
}

void CoreChecks::RecordCmdWriteTimestamp2(CMD_BUFFER_STATE &cb_state, VkQueryPool queryPool, uint32_t slot, Func command) const {
    if (disabled[query_validation]) return;
    // Enqueue the submit time validation check here, before the submit time state update in StateTracker::PostCall...
    QueryObject query_obj = {queryPool, slot};
    cb_state.queryUpdates.emplace_back([query_obj, command](CMD_BUFFER_STATE &cb_state_arg, bool do_validate,
                                                            VkQueryPool &firstPerfQueryPool, uint32_t perfPass,
                                                            QueryMap *localQueryToStateMap) {
        if (!do_validate) return false;
        return VerifyQueryIsReset(cb_state_arg, query_obj, command, firstPerfQueryPool, perfPass, localQueryToStateMap);
    });
}

void CoreChecks::PreCallRecordCmdWriteTimestamp(VkCommandBuffer commandBuffer, VkPipelineStageFlagBits pipelineStage,
                                                VkQueryPool queryPool, uint32_t slot) {
    auto cb_state = GetWrite<CMD_BUFFER_STATE>(commandBuffer);
    return RecordCmdWriteTimestamp2(*cb_state, queryPool, slot, Func::vkCmdWriteTimestamp);
}

void CoreChecks::PreCallRecordCmdWriteTimestamp2KHR(VkCommandBuffer commandBuffer, VkPipelineStageFlags2KHR stage,
                                                    VkQueryPool queryPool, uint32_t slot) {
    auto cb_state = GetWrite<CMD_BUFFER_STATE>(commandBuffer);
    return RecordCmdWriteTimestamp2(*cb_state, queryPool, slot, Func::vkCmdWriteTimestamp2KHR);
}

void CoreChecks::PreCallRecordCmdWriteTimestamp2(VkCommandBuffer commandBuffer, VkPipelineStageFlags2KHR stage,
                                                 VkQueryPool queryPool, uint32_t slot) {
    auto cb_state = GetWrite<CMD_BUFFER_STATE>(commandBuffer);
    return RecordCmdWriteTimestamp2(*cb_state, queryPool, slot, Func::vkCmdWriteTimestamp2);
}

bool CoreChecks::PreCallValidateCmdBeginQueryIndexedEXT(VkCommandBuffer commandBuffer, VkQueryPool queryPool, uint32_t slot,
                                                        VkQueryControlFlags flags, uint32_t index,
                                                        const ErrorObject &error_obj) const {
    if (disabled[query_validation]) return false;
    auto cb_state = GetRead<CMD_BUFFER_STATE>(commandBuffer);
    assert(cb_state);
    QueryObject query_obj = {queryPool, slot, flags, 0, true, index};
    struct BeginQueryIndexedVuids : ValidateBeginQueryVuids {
        BeginQueryIndexedVuids() : ValidateBeginQueryVuids() {
            vuid_queue_feedback = "VUID-vkCmdBeginQueryIndexedEXT-queryType-02338";
            vuid_queue_occlusion = "VUID-vkCmdBeginQueryIndexedEXT-queryType-00803";
            vuid_precise = "VUID-vkCmdBeginQueryIndexedEXT-queryType-00800";
            vuid_query_count = "VUID-vkCmdBeginQueryIndexedEXT-query-00802";
            vuid_profile_lock = "VUID-vkCmdBeginQueryIndexedEXT-queryPool-03223";
            vuid_scope_not_first = "VUID-vkCmdBeginQueryIndexedEXT-queryPool-03224";
            vuid_scope_in_rp = "VUID-vkCmdBeginQueryIndexedEXT-queryPool-03225";
            vuid_dup_query_type = "VUID-vkCmdBeginQueryIndexedEXT-queryPool-04753";
            vuid_protected_cb = "VUID-vkCmdBeginQueryIndexedEXT-commandBuffer-01885";
            vuid_multiview_query = "VUID-vkCmdBeginQueryIndexedEXT-query-00808";
            vuid_graphics_support = "VUID-vkCmdBeginQueryIndexedEXT-queryType-00804";
            vuid_compute_support = "VUID-vkCmdBeginQueryIndexedEXT-queryType-00805";
            vuid_primitives_generated = "VUID-vkCmdBeginQueryIndexedEXT-queryType-06689";
            vuid_result_status_support = "VUID-vkCmdBeginQueryIndexedEXT-queryType-07126";
            vuid_no_active_in_vc_scope = "VUID-vkCmdBeginQueryIndexedEXT-None-07127";
            vuid_result_status_profile_in_vc_scope = "VUID-vkCmdBeginQueryIndexedEXT-queryType-07128";
            vuid_vc_scope_query_type = "VUID-vkCmdBeginQueryIndexedEXT-queryType-07131";
        }
    };
    BeginQueryIndexedVuids vuids;
    bool skip = false;
    skip |= ValidateBeginQuery(*cb_state, query_obj, flags, index, error_obj.location, &vuids);
    skip |= ValidateCmd(*cb_state, error_obj.location);

    // Extension specific VU's
    const auto &query_pool_state = *Get<QUERY_POOL_STATE>(query_obj.pool);
    const auto &query_pool_ci = query_pool_state.createInfo;
    if (query_pool_ci.queryType == VK_QUERY_TYPE_PRIMITIVES_GENERATED_EXT) {
        if (!enabled_features.primitivesGeneratedQuery) {
            const LogObjectList objlist(commandBuffer, query_pool_state.pool());
            skip |= LogError("VUID-vkCmdBeginQueryIndexedEXT-queryType-06693", objlist, error_obj.location.dot(Field::queryPool),
                             "was created with queryType of VK_QUERY_TYPE_PRIMITIVES_GENERATED_EXT, "
                             "but the primitivesGeneratedQuery feature is not enabled.");
        }
        if (index >= phys_dev_ext_props.transform_feedback_props.maxTransformFeedbackStreams) {
            const LogObjectList objlist(commandBuffer, query_pool_state.pool());
            skip |= LogError("VUID-vkCmdBeginQueryIndexedEXT-queryType-06690", objlist, error_obj.location.dot(Field::queryPool),
                             "was created with queryType of VK_QUERY_TYPE_PRIMITIVES_GENERATED_EXT, but "
                             "index (%" PRIu32
                             ") is greater than or equal to "
                             "VkPhysicalDeviceTransformFeedbackPropertiesEXT::maxTransformFeedbackStreams (%" PRIu32 ")",
                             index, phys_dev_ext_props.transform_feedback_props.maxTransformFeedbackStreams);
        }
        if ((index != 0) && (!enabled_features.primitivesGeneratedQueryWithNonZeroStreams)) {
            const LogObjectList objlist(commandBuffer, query_pool_state.pool());
            skip |= LogError("VUID-vkCmdBeginQueryIndexedEXT-queryType-06691", objlist, error_obj.location.dot(Field::queryPool),
                             "was created with queryType of VK_QUERY_TYPE_PRIMITIVES_GENERATED_EXT, but "
                             "index (%" PRIu32
                             ") is not zero and the primitivesGeneratedQueryWithNonZeroStreams feature is not enabled",
                             index);
        }
    } else if (query_pool_ci.queryType == VK_QUERY_TYPE_TRANSFORM_FEEDBACK_STREAM_EXT) {
        if (IsExtEnabled(device_extensions.vk_ext_transform_feedback) &&
            (index >= phys_dev_ext_props.transform_feedback_props.maxTransformFeedbackStreams)) {
            skip |= LogError(
                "VUID-vkCmdBeginQueryIndexedEXT-queryType-02339", commandBuffer, error_obj.location.dot(Field::index),
                "(%" PRIu32
                ") must be less than VkPhysicalDeviceTransformFeedbackPropertiesEXT::maxTransformFeedbackStreams %" PRIu32 ".",
                index, phys_dev_ext_props.transform_feedback_props.maxTransformFeedbackStreams);
        }
    } else if (index != 0) {
        const LogObjectList objlist(commandBuffer, query_pool_state.pool());
        skip |= LogError("VUID-vkCmdBeginQueryIndexedEXT-queryType-06692", objlist, error_obj.location.dot(Field::index),
                         "(%" PRIu32
                         ") must be zero if %s was not created with type VK_QUERY_TYPE_TRANSFORM_FEEDBACK_STREAM_EXT or "
                         "VK_QUERY_TYPE_PRIMITIVES_GENERATED_EXT",
                         index, FormatHandle(queryPool).c_str());
    }
    return skip;
}

void CoreChecks::PreCallRecordCmdBeginQueryIndexedEXT(VkCommandBuffer commandBuffer, VkQueryPool queryPool, uint32_t slot,
                                                      VkQueryControlFlags flags, uint32_t index) {
    if (disabled[query_validation]) return;
    QueryObject query_obj = {queryPool, slot, flags, 0, true, index};
    EnqueueVerifyBeginQuery(commandBuffer, query_obj, Func::vkCmdBeginQueryIndexedEXT);
}

void CoreChecks::PreCallRecordCmdEndQueryIndexedEXT(VkCommandBuffer commandBuffer, VkQueryPool queryPool, uint32_t slot,
                                                    uint32_t index) {
    if (disabled[query_validation]) return;
    auto cb_state = GetWrite<CMD_BUFFER_STATE>(commandBuffer);
    QueryObject query_obj = {queryPool, slot, 0, 0, true, index};
    query_obj.end_command_index = cb_state->command_count;  // off by one because cb_state hasn't recorded this yet
    EnqueueVerifyEndQuery(*cb_state, query_obj, Func::vkCmdEndQueryIndexedEXT);
}

bool CoreChecks::PreCallValidateCmdEndQueryIndexedEXT(VkCommandBuffer commandBuffer, VkQueryPool queryPool, uint32_t slot,
                                                      uint32_t index, const ErrorObject &error_obj) const {
    if (disabled[query_validation]) return false;
    QueryObject query_obj = {queryPool, slot, 0, 0, true, index};
    auto cb_state = GetRead<CMD_BUFFER_STATE>(commandBuffer);
    assert(cb_state);
    struct EndQueryIndexedVuids : ValidateEndQueryVuids {
        EndQueryIndexedVuids() : ValidateEndQueryVuids() {
            vuid_active_queries = "VUID-vkCmdEndQueryIndexedEXT-None-02342";
            vuid_protected_cb = "VUID-vkCmdEndQueryIndexedEXT-commandBuffer-02344";
            vuid_multiview_query = "VUID-vkCmdEndQueryIndexedEXT-query-02345";
        }
    };
    EndQueryIndexedVuids vuids;
    bool skip = false;
    skip |= ValidateCmdEndQuery(*cb_state, query_obj, index, error_obj.location, &vuids);
    skip |= ValidateCmd(*cb_state, error_obj.location);

    const auto &query_pool_state = *Get<QUERY_POOL_STATE>(queryPool);
    const auto &query_pool_ci = query_pool_state.createInfo;
    const uint32_t available_query_count = query_pool_state.createInfo.queryCount;
    if (slot >= available_query_count) {
        const LogObjectList objlist(commandBuffer, query_pool_state.pool());
        skip |= LogError("VUID-vkCmdEndQueryIndexedEXT-query-02343", objlist, error_obj.location.dot(Field::index),
                         "(%" PRIu32 ") is greater or equal to the queryPool size (%" PRIu32 ").", index, available_query_count);
    }
    if (query_pool_ci.queryType == VK_QUERY_TYPE_TRANSFORM_FEEDBACK_STREAM_EXT ||
        query_pool_ci.queryType == VK_QUERY_TYPE_PRIMITIVES_GENERATED_EXT) {
        if (index >= phys_dev_ext_props.transform_feedback_props.maxTransformFeedbackStreams) {
            skip |= LogError("VUID-vkCmdEndQueryIndexedEXT-queryType-06694", commandBuffer, error_obj.location.dot(Field::index),
                             "(%" PRIu32
                             ") must be less than "
                             "VkPhysicalDeviceTransformFeedbackPropertiesEXT::maxTransformFeedbackStreams %" PRIu32 ".",
                             index, phys_dev_ext_props.transform_feedback_props.maxTransformFeedbackStreams);
        }
        for (const auto &query_object : cb_state->startedQueries) {
            if (query_object.pool == queryPool && query_object.slot == slot) {
                if (query_object.index != index) {
                    const LogObjectList objlist(commandBuffer, query_pool_state.pool());
                    skip |= LogError("VUID-vkCmdEndQueryIndexedEXT-queryType-06696", objlist, error_obj.location,
                                     "queryPool is of type %s, but "
                                     "index (%" PRIu32 ") is not equal to the index used to begin the query (%" PRIu32 ")",
                                     string_VkQueryType(query_pool_ci.queryType), index, query_object.index);
                }
                break;
            }
        }
    } else if (index != 0) {
        const LogObjectList objlist(commandBuffer, query_pool_state.pool());
        skip |= LogError("VUID-vkCmdEndQueryIndexedEXT-queryType-06695", objlist, error_obj.location.dot(Field::index),
                         "(%" PRIu32
                         ") must be zero if %s was not created with type VK_QUERY_TYPE_TRANSFORM_FEEDBACK_STREAM_EXT and not"
                         " VK_QUERY_TYPE_PRIMITIVES_GENERATED_EXT.",
                         index, FormatHandle(queryPool).c_str());
    }

    return skip;
}

bool CoreChecks::PreCallValidateResetQueryPool(VkDevice device, VkQueryPool queryPool, uint32_t firstQuery, uint32_t queryCount,
                                               const ErrorObject &error_obj) const {
    if (disabled[query_validation]) return false;

    bool skip = false;

    if (!enabled_features.hostQueryReset) {
        skip |= LogError("VUID-vkResetQueryPool-None-02665", device, error_obj.location, "hostQueryReset feature was not enabled.");
    }

    const auto &query_pool_state = *Get<QUERY_POOL_STATE>(queryPool);
    if (firstQuery >= query_pool_state.createInfo.queryCount) {
        skip |= LogError("VUID-vkResetQueryPool-firstQuery-02666", queryPool, error_obj.location.dot(Field::firstQuery),
                         "(%" PRIu32 ") is greater than or equal to query pool count (%" PRIu32 ") for %s.", firstQuery,
                         query_pool_state.createInfo.queryCount, FormatHandle(queryPool).c_str());
    }

    if ((firstQuery + queryCount) > query_pool_state.createInfo.queryCount) {
        skip |= LogError("VUID-vkResetQueryPool-firstQuery-02667", queryPool, error_obj.location,
                         "Query range [%" PRIu32 ", %" PRIu32 ") goes beyond query pool count (%" PRIu32 ") for %s.", firstQuery,
                         firstQuery + queryCount, query_pool_state.createInfo.queryCount, FormatHandle(queryPool).c_str());
    }

    return skip;
}

bool CoreChecks::PreCallValidateResetQueryPoolEXT(VkDevice device, VkQueryPool queryPool, uint32_t firstQuery, uint32_t queryCount,
                                                  const ErrorObject &error_obj) const {
    return PreCallValidateResetQueryPool(device, queryPool, firstQuery, queryCount, error_obj);
}

bool CoreChecks::ValidateQueryPoolStride(const std::string &vuid_not_64, const std::string &vuid_64, const VkDeviceSize stride,
                                         const char *parameter_name, const uint64_t parameter_value, const VkQueryResultFlags flags,
                                         const Location &loc) const {
    bool skip = false;
    if (flags & VK_QUERY_RESULT_64_BIT) {
        static const int condition_multiples = 0b0111;
        if ((stride & condition_multiples) || (parameter_value & condition_multiples)) {
            skip |= LogError(vuid_64, device, loc, "stride %" PRIu64 " or %s %" PRIu64 " is invalid.", stride, parameter_name,
                             parameter_value);
        }
    } else {
        static const int condition_multiples = 0b0011;
        if ((stride & condition_multiples) || (parameter_value & condition_multiples)) {
            skip |= LogError(vuid_not_64, device, loc, "stride %" PRIu64 " or %s %" PRIu64 " is invalid.", stride, parameter_name,
                             parameter_value);
        }
    }
    return skip;
}

void CoreChecks::PostCallRecordGetQueryPoolResults(VkDevice device, VkQueryPool queryPool, uint32_t firstQuery, uint32_t queryCount,
                                                   size_t dataSize, void *pData, VkDeviceSize stride, VkQueryResultFlags flags,
                                                   const RecordObject &record_obj) {
    if (record_obj.result != VK_SUCCESS) {
        return;
    }
    auto query_pool_state = Get<QUERY_POOL_STATE>(queryPool);
    if ((flags & VK_QUERY_RESULT_PARTIAL_BIT) == 0) {
        for (uint32_t i = firstQuery; i < queryCount; ++i) {
            query_pool_state->SetQueryState(i, 0, QUERYSTATE_AVAILABLE);
        }
    }
}

bool CoreChecks::PreCallValidateReleaseProfilingLockKHR(VkDevice device, const ErrorObject &error_obj) const {
    bool skip = false;

    if (!performance_lock_acquired) {
        skip |= LogError("VUID-vkReleaseProfilingLockKHR-device-03235", device, error_obj.location,
                         "The profiling lock of device must have been held via a previous successful "
                         "call to vkAcquireProfilingLockKHR.");
    }

    return skip;
}
