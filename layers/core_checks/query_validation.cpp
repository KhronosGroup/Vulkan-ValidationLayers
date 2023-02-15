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

#include "vk_enum_string_helper.h"
#include "chassis.h"
#include "core_checks/core_validation.h"

bool CoreChecks::PreCallValidateDestroyQueryPool(VkDevice device, VkQueryPool queryPool,
                                                 const VkAllocationCallbacks *pAllocator) const {
    if (disabled[query_validation]) return false;
    auto qp_state = Get<QUERY_POOL_STATE>(queryPool);
    bool skip = false;
    if (qp_state) {
        bool completed_by_get_results = true;
        for (uint32_t i = 0; i < qp_state->createInfo.queryCount; ++i) {
            auto state = qp_state->GetQueryState(i, 0);
            if (state != QUERYSTATE_AVAILABLE) {
                completed_by_get_results = false;
                break;
            }
        }
        if (!completed_by_get_results) {
            skip |= ValidateObjectNotInUse(qp_state.get(), "vkDestroyQueryPool", "VUID-vkDestroyQueryPool-queryPool-00793");
        }
    }
    return skip;
}

bool CoreChecks::ValidatePerformanceQueryResults(const char *cmd_name, const QUERY_POOL_STATE *query_pool_state,
                                                 uint32_t firstQuery, uint32_t queryCount, VkQueryResultFlags flags) const {
    bool skip = false;

    if (flags & (VK_QUERY_RESULT_WITH_AVAILABILITY_BIT | VK_QUERY_RESULT_PARTIAL_BIT | VK_QUERY_RESULT_64_BIT)) {
        std::string invalid_flags_string;
        for (auto flag : {VK_QUERY_RESULT_WITH_AVAILABILITY_BIT, VK_QUERY_RESULT_PARTIAL_BIT, VK_QUERY_RESULT_64_BIT}) {
            if (flag & flags) {
                if (invalid_flags_string.size()) {
                    invalid_flags_string += " and ";
                }
                invalid_flags_string += string_VkQueryResultFlagBits(flag);
            }
        }
        skip |= LogError(query_pool_state->pool(),
                         strcmp(cmd_name, "vkGetQueryPoolResults") == 0 ? "VUID-vkGetQueryPoolResults-queryType-03230"
                                                                        : "VUID-vkCmdCopyQueryPoolResults-queryType-03233",
                         "%s: QueryPool %s was created with a queryType of"
                         "VK_QUERY_TYPE_PERFORMANCE_QUERY_KHR but flags contains %s.",
                         cmd_name, report_data->FormatHandle(query_pool_state->pool()).c_str(), invalid_flags_string.c_str());
    }

    for (uint32_t query_index = firstQuery; query_index < queryCount; query_index++) {
        uint32_t submitted = 0;
        for (uint32_t pass_index = 0; pass_index < query_pool_state->n_performance_passes; pass_index++) {
            auto state = query_pool_state->GetQueryState(query_index, pass_index);
            if (state == QUERYSTATE_AVAILABLE) {
                submitted++;
            }
        }
        if (submitted < query_pool_state->n_performance_passes) {
            skip |= LogError(query_pool_state->pool(), "VUID-vkGetQueryPoolResults-queryType-03231",
                             "%s: QueryPool %s has %u performance query passes, but the query has only been "
                             "submitted for %u of the passes.",
                             cmd_name, report_data->FormatHandle(query_pool_state->pool()).c_str(),
                             query_pool_state->n_performance_passes, submitted);
        }
    }

    return skip;
}

bool CoreChecks::ValidateGetQueryPoolPerformanceResults(VkQueryPool queryPool, uint32_t firstQuery, uint32_t queryCount,
                                                        void *pData, VkDeviceSize stride, VkQueryResultFlags flags,
                                                        const char *apiName) const {
    bool skip = false;
    auto query_pool_state = Get<QUERY_POOL_STATE>(queryPool);

    if (!query_pool_state || query_pool_state->createInfo.queryType != VK_QUERY_TYPE_PERFORMANCE_QUERY_KHR) return skip;

    if (((((uintptr_t)pData) % sizeof(VkPerformanceCounterResultKHR)) != 0 ||
         (stride % sizeof(VkPerformanceCounterResultKHR)) != 0)) {
        skip |= LogError(queryPool, "VUID-vkGetQueryPoolResults-queryType-03229",
                         "%s(): QueryPool %s was created with a queryType of "
                         "VK_QUERY_TYPE_PERFORMANCE_QUERY_KHR but pData & stride are not multiple of the "
                         "size of VkPerformanceCounterResultKHR.",
                         apiName, report_data->FormatHandle(queryPool).c_str());
    }

    skip |= ValidatePerformanceQueryResults(apiName, query_pool_state.get(), firstQuery, queryCount, flags);

    return skip;
}

bool CoreChecks::PreCallValidateGetQueryPoolResults(VkDevice device, VkQueryPool queryPool, uint32_t firstQuery,
                                                    uint32_t queryCount, size_t dataSize, void *pData, VkDeviceSize stride,
                                                    VkQueryResultFlags flags) const {
    if (disabled[query_validation]) return false;
    bool skip = false;
    skip |= ValidateQueryPoolIndex(queryPool, firstQuery, queryCount, "vkGetQueryPoolResults()",
                                   "VUID-vkGetQueryPoolResults-firstQuery-00813", "VUID-vkGetQueryPoolResults-firstQuery-00816");
    skip |=
        ValidateGetQueryPoolPerformanceResults(queryPool, firstQuery, queryCount, pData, stride, flags, "vkGetQueryPoolResults");

    auto query_pool_state = Get<QUERY_POOL_STATE>(queryPool);
    if (query_pool_state) {
        if (query_pool_state->createInfo.queryType != VK_QUERY_TYPE_PERFORMANCE_QUERY_KHR) {
            const char *vuid = IsExtEnabled(device_extensions.vk_khr_performance_query) ? "VUID-vkGetQueryPoolResults-flags-02828"
                                                                                        : "VUID-vkGetQueryPoolResults-flags-02827";
            skip |= ValidateQueryPoolStride(vuid, "VUID-vkGetQueryPoolResults-flags-00815", stride, "dataSize", dataSize, flags);
        }
        if ((query_pool_state->createInfo.queryType == VK_QUERY_TYPE_TIMESTAMP) && (flags & VK_QUERY_RESULT_PARTIAL_BIT)) {
            skip |= LogError(
                queryPool, "VUID-vkGetQueryPoolResults-queryType-00818",
                "%s was created with a queryType of VK_QUERY_TYPE_TIMESTAMP but flags contains VK_QUERY_RESULT_PARTIAL_BIT.",
                report_data->FormatHandle(queryPool).c_str());
        }
        if (query_pool_state->createInfo.queryType == VK_QUERY_TYPE_RESULT_STATUS_ONLY_KHR &&
            (flags & VK_QUERY_RESULT_WITH_STATUS_BIT_KHR) == 0) {
            skip |= LogError(queryPool, "VUID-vkGetQueryPoolResults-queryType-04810",
                             "vkGetQueryPoolResults(): queryPool %s was created with VK_QUERY_TYPE_RESULT_STATUS_ONLY_KHR "
                             "queryType, but flags do not contain VK_QUERY_RESULT_WITH_STATUS_BIT_KHR bit",
                             report_data->FormatHandle(queryPool).c_str());
        }

        if (!skip) {
            uint32_t query_avail_data =
                (flags & (VK_QUERY_RESULT_WITH_AVAILABILITY_BIT | VK_QUERY_RESULT_WITH_STATUS_BIT_KHR)) ? 1 : 0;
            uint32_t query_size_in_bytes = (flags & VK_QUERY_RESULT_64_BIT) ? sizeof(uint64_t) : sizeof(uint32_t);
            uint32_t query_items = 0;
            uint32_t query_size = 0;

            switch (query_pool_state->createInfo.queryType) {
                case VK_QUERY_TYPE_OCCLUSION:
                    // Occlusion queries write one integer value - the number of samples passed.
                    query_items = 1;
                    query_size = query_size_in_bytes * (query_items + query_avail_data);
                    break;

                case VK_QUERY_TYPE_PIPELINE_STATISTICS:
                    // Pipeline statistics queries write one integer value for each bit that is enabled in the pipelineStatistics
                    // when the pool is created
                    {
                        query_items = GetBitSetCount(query_pool_state->createInfo.pipelineStatistics);
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
                    query_items = query_pool_state->perf_counter_index_count;
                    query_size = sizeof(VkPerformanceCounterResultKHR) * query_items;
                    if (query_size > stride) {
                        skip |= LogError(queryPool, "VUID-vkGetQueryPoolResults-queryType-04519",
                                         "vkGetQueryPoolResults() on querypool %s specified stride %" PRIu64
                                         " which must be at least counterIndexCount (%d) "
                                         "multiplied by sizeof(VkPerformanceCounterResultKHR) (%zu).",
                                         report_data->FormatHandle(queryPool).c_str(), stride, query_items,
                                         sizeof(VkPerformanceCounterResultKHR));
                    }
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
                skip |= LogError(queryPool, "VUID-vkGetQueryPoolResults-dataSize-00817",
                                 "vkGetQueryPoolResults() on querypool %s specified dataSize %zu which is "
                                 "incompatible with the specified query type and options.",
                                 report_data->FormatHandle(queryPool).c_str(), dataSize);
            }
        }
    }

    return skip;
}

bool CoreChecks::PreCallValidateCreateQueryPool(VkDevice device, const VkQueryPoolCreateInfo *pCreateInfo,
                                                const VkAllocationCallbacks *pAllocator, VkQueryPool *pQueryPool) const {
    if (disabled[query_validation]) return false;
    bool skip = false;
    if (pCreateInfo && pCreateInfo->queryType == VK_QUERY_TYPE_PIPELINE_STATISTICS) {
        if (!enabled_features.core.pipelineStatisticsQuery) {
            skip |= LogError(device, "VUID-VkQueryPoolCreateInfo-queryType-00791",
                             "vkCreateQueryPool(): Query pool with type VK_QUERY_TYPE_PIPELINE_STATISTICS created on a device with "
                             "VkDeviceCreateInfo.pEnabledFeatures.pipelineStatisticsQuery == VK_FALSE.");
        }
    }
    if (pCreateInfo && pCreateInfo->queryType == VK_QUERY_TYPE_PERFORMANCE_QUERY_KHR) {
        if (!enabled_features.performance_query_features.performanceCounterQueryPools) {
            skip |=
                LogError(device, "VUID-VkQueryPoolPerformanceCreateInfoKHR-performanceCounterQueryPools-03237",
                         "vkCreateQueryPool(): Query pool with type VK_QUERY_TYPE_PERFORMANCE_QUERY_KHR created on a device with "
                         "VkPhysicalDevicePerformanceQueryFeaturesKHR.performanceCounterQueryPools == VK_FALSE.");
        }

        auto perf_ci = LvlFindInChain<VkQueryPoolPerformanceCreateInfoKHR>(pCreateInfo->pNext);
        if (!perf_ci) {
            skip |= LogError(
                device, "VUID-VkQueryPoolCreateInfo-queryType-03222",
                "vkCreateQueryPool(): Query pool with type VK_QUERY_TYPE_PERFORMANCE_QUERY_KHR created but the pNext chain of "
                "pCreateInfo does not contain in instance of VkQueryPoolPerformanceCreateInfoKHR.");
        } else {
            const auto &perf_counter_iter = physical_device_state->perf_counters.find(perf_ci->queueFamilyIndex);
            if (perf_counter_iter == physical_device_state->perf_counters.end()) {
                skip |= LogError(
                    device, "VUID-VkQueryPoolPerformanceCreateInfoKHR-queueFamilyIndex-03236",
                    "vkCreateQueryPool(): VkQueryPerformanceCreateInfoKHR::queueFamilyIndex is not a valid queue family index.");
            } else {
                const QUEUE_FAMILY_PERF_COUNTERS *perf_counters = perf_counter_iter->second.get();
                for (uint32_t idx = 0; idx < perf_ci->counterIndexCount; idx++) {
                    if (perf_ci->pCounterIndices[idx] >= perf_counters->counters.size()) {
                        skip |= LogError(
                            device, "VUID-VkQueryPoolPerformanceCreateInfoKHR-pCounterIndices-03321",
                            "vkCreateQueryPool(): VkQueryPerformanceCreateInfoKHR::pCounterIndices[%u] = %u is not a valid "
                            "counter index.",
                            idx, perf_ci->pCounterIndices[idx]);
                    }
                }
            }
        }
    }
    if (pCreateInfo && pCreateInfo->queryType == VK_QUERY_TYPE_RESULT_STATUS_ONLY_KHR) {
        auto video_profile = LvlFindInChain<VkVideoProfileInfoKHR>(pCreateInfo->pNext);
        if (video_profile) {
            skip |= ValidateVideoProfileInfo(video_profile, device, "vkCreateQueryPool",
                                             "the VkVideoProfileInfoKHR structure included in the pCreateInfo->pNext chain");
        }
    }
    return skip;
}

bool CoreChecks::ValidateCmdQueueFlags(const CMD_BUFFER_STATE &cb_state, const char *caller_name, VkQueueFlags required_flags,
                                       const char *error_code) const {
    auto pool = cb_state.command_pool;
    if (pool) {
        const uint32_t queue_family_index = pool->queueFamilyIndex;
        const VkQueueFlags queue_flags = physical_device_state->queue_family_properties[queue_family_index].queueFlags;
        if (!(required_flags & queue_flags)) {
            std::string required_flags_string;
            for (auto flag : {VK_QUEUE_TRANSFER_BIT, VK_QUEUE_GRAPHICS_BIT, VK_QUEUE_COMPUTE_BIT, VK_QUEUE_SPARSE_BINDING_BIT,
                              VK_QUEUE_PROTECTED_BIT}) {
                if (flag & required_flags) {
                    if (required_flags_string.size()) {
                        required_flags_string += " or ";
                    }
                    required_flags_string += string_VkQueueFlagBits(flag);
                }
            }
            return LogError(cb_state.commandBuffer(), error_code,
                            "%s(): Called in command buffer %s which was allocated from the command pool %s which was created with "
                            "queueFamilyIndex %u which doesn't contain the required %s capability flags.",
                            caller_name, report_data->FormatHandle(cb_state.commandBuffer()).c_str(),
                            report_data->FormatHandle(pool->commandPool()).c_str(), queue_family_index,
                            required_flags_string.c_str());
        }
    }
    return false;
}

bool CoreChecks::ValidateBeginQuery(const CMD_BUFFER_STATE &cb_state, const QueryObject &query_obj, VkFlags flags, uint32_t index,
                                    CMD_TYPE cmd, const ValidateBeginQueryVuids *vuids) const {
    bool skip = false;
    auto query_pool_state = Get<QUERY_POOL_STATE>(query_obj.pool);
    const auto &query_pool_ci = query_pool_state->createInfo;
    const char *cmd_name = CommandTypeString(cmd);

    if (query_pool_ci.queryType == VK_QUERY_TYPE_TIMESTAMP) {
        skip |= LogError(cb_state.commandBuffer(), "VUID-vkCmdBeginQuery-queryType-02804",
                         "%s: The querypool's query type must not be VK_QUERY_TYPE_TIMESTAMP.", cmd_name);
    }

    // Check for nested queries
    if (cb_state.activeQueries.size()) {
        for (const auto &a_query : cb_state.activeQueries) {
            auto active_query_pool_state = Get<QUERY_POOL_STATE>(a_query.pool);
            if (active_query_pool_state->createInfo.queryType == query_pool_ci.queryType && a_query.index == index) {
                const LogObjectList objlist(cb_state.commandBuffer(), query_obj.pool, a_query.pool);
                skip |= LogError(objlist, vuids->vuid_dup_query_type,
                                 "%s: Within the same command buffer %s, query %d from pool %s has same queryType as active query "
                                 "%d from pool %s.",
                                 cmd_name, report_data->FormatHandle(cb_state.commandBuffer()).c_str(), query_obj.index,
                                 report_data->FormatHandle(query_obj.pool).c_str(), a_query.index,
                                 report_data->FormatHandle(a_query.pool).c_str());
            }
        }
    }

    // There are tighter queue constraints to test for certain query pools
    if (query_pool_ci.queryType == VK_QUERY_TYPE_TRANSFORM_FEEDBACK_STREAM_EXT) {
        skip |= ValidateCmdQueueFlags(cb_state, cmd_name, VK_QUEUE_GRAPHICS_BIT, vuids->vuid_queue_feedback);
        if (!phys_dev_ext_props.transform_feedback_props.transformFeedbackQueries) {
            const char *vuid = cmd == CMD_BEGINQUERYINDEXEDEXT ? "VUID-vkCmdBeginQueryIndexedEXT-queryType-02341"
                                                               : "VUID-vkCmdBeginQuery-queryType-02328";
            skip |= LogError(cb_state.commandBuffer(), vuid,
                             "%s: queryPool was created with queryType VK_QUERY_TYPE_TRANSFORM_FEEDBACK_STREAM_EXT, but "
                             "VkPhysicalDeviceTransformFeedbackPropertiesEXT::transformFeedbackQueries is not supported.",
                             cmd_name);
        }
    }
    if (query_pool_ci.queryType == VK_QUERY_TYPE_OCCLUSION) {
        skip |= ValidateCmdQueueFlags(cb_state, cmd_name, VK_QUEUE_GRAPHICS_BIT, vuids->vuid_queue_occlusion);
    }
    if (query_pool_ci.queryType == VK_QUERY_TYPE_PERFORMANCE_QUERY_KHR) {
        if (!cb_state.performance_lock_acquired) {
            skip |= LogError(cb_state.commandBuffer(), vuids->vuid_profile_lock,
                             "%s: profiling lock must be held before vkBeginCommandBuffer is called on "
                             "a command buffer where performance queries are recorded.",
                             cmd_name);
        }

        if (query_pool_state->has_perf_scope_command_buffer && cb_state.command_count > 0) {
            skip |= LogError(cb_state.commandBuffer(), vuids->vuid_scope_not_first,
                             "%s: Query pool %s was created with a counter of scope "
                             "VK_QUERY_SCOPE_COMMAND_BUFFER_KHR but %s is not the first recorded "
                             "command in the command buffer.",
                             cmd_name, report_data->FormatHandle(query_obj.pool).c_str(), cmd_name);
        }

        if (query_pool_state->has_perf_scope_render_pass && cb_state.activeRenderPass) {
            skip |= LogError(cb_state.commandBuffer(), vuids->vuid_scope_in_rp,
                             "%s: Query pool %s was created with a counter of scope "
                             "VK_QUERY_SCOPE_RENDER_PASS_KHR but %s is inside a render pass.",
                             cmd_name, report_data->FormatHandle(query_obj.pool).c_str(), cmd_name);
        }
    }
    if (query_pool_ci.queryType == VK_QUERY_TYPE_ACCELERATION_STRUCTURE_COMPACTED_SIZE_KHR ||
        query_pool_ci.queryType == VK_QUERY_TYPE_ACCELERATION_STRUCTURE_SERIALIZATION_SIZE_KHR) {
        const char *vuid = cmd == CMD_BEGINQUERYINDEXEDEXT ? "VUID-vkCmdBeginQueryIndexedEXT-queryType-04728"
                                                           : "VUID-vkCmdBeginQuery-queryType-04728";
        skip |= LogError(cb_state.commandBuffer(), vuid, "%s: QueryPool was created with queryType %s.", cmd_name,
                         string_VkQueryType(query_pool_ci.queryType));
    } else if (query_pool_ci.queryType == VK_QUERY_TYPE_ACCELERATION_STRUCTURE_COMPACTED_SIZE_NV) {
        const char *vuid = cmd == CMD_BEGINQUERYINDEXEDEXT ? "VUID-vkCmdBeginQueryIndexedEXT-queryType-04729"
                                                           : "VUID-vkCmdBeginQuery-queryType-04729";
        skip |= LogError(cb_state.commandBuffer(), vuid, "%s: QueryPool was created with queryType %s.", cmd_name,
                         string_VkQueryType(query_pool_ci.queryType));
    } else if (query_pool_ci.queryType == VK_QUERY_TYPE_ACCELERATION_STRUCTURE_SIZE_KHR ||
               query_pool_ci.queryType == VK_QUERY_TYPE_ACCELERATION_STRUCTURE_SERIALIZATION_BOTTOM_LEVEL_POINTERS_KHR) {
        const char *vuid = cmd == CMD_BEGINQUERYINDEXEDEXT ? "VUID-vkCmdBeginQueryIndexedEXT-queryType-06741"
                                                           : "VUID-vkCmdBeginQuery-queryType-06741";
        skip |= LogError(cb_state.commandBuffer(), vuid, "%s: QueryPool was created with queryType %s.", cmd_name,
                         string_VkQueryType(query_pool_ci.queryType));
    }
    if (query_pool_ci.queryType == VK_QUERY_TYPE_PIPELINE_STATISTICS) {
        if ((cb_state.command_pool->queue_flags & VK_QUEUE_GRAPHICS_BIT) == 0) {
            if (query_pool_ci.pipelineStatistics &
                (VK_QUERY_PIPELINE_STATISTIC_INPUT_ASSEMBLY_VERTICES_BIT |
                 VK_QUERY_PIPELINE_STATISTIC_INPUT_ASSEMBLY_PRIMITIVES_BIT |
                 VK_QUERY_PIPELINE_STATISTIC_VERTEX_SHADER_INVOCATIONS_BIT |
                 VK_QUERY_PIPELINE_STATISTIC_GEOMETRY_SHADER_INVOCATIONS_BIT |
                 VK_QUERY_PIPELINE_STATISTIC_GEOMETRY_SHADER_PRIMITIVES_BIT | VK_QUERY_PIPELINE_STATISTIC_CLIPPING_INVOCATIONS_BIT |
                 VK_QUERY_PIPELINE_STATISTIC_CLIPPING_PRIMITIVES_BIT | VK_QUERY_PIPELINE_STATISTIC_FRAGMENT_SHADER_INVOCATIONS_BIT |
                 VK_QUERY_PIPELINE_STATISTIC_TESSELLATION_CONTROL_SHADER_PATCHES_BIT |
                 VK_QUERY_PIPELINE_STATISTIC_TESSELLATION_EVALUATION_SHADER_INVOCATIONS_BIT)) {
                skip |= LogError(
                    cb_state.commandBuffer(), vuids->vuid_graphics_support,
                    "%s(): queryType of queryPool is VK_QUERY_TYPE_PIPELINE_STATISTICS (%s) and indicates graphics operations, but "
                    "the command pool the command buffer %s was allocated from does not support graphics operations (%s).",
                    cmd_name, string_VkQueryPipelineStatisticFlags(query_pool_ci.pipelineStatistics).c_str(),
                    report_data->FormatHandle(cb_state.commandBuffer()).c_str(),
                    string_VkQueueFlags(cb_state.command_pool->queue_flags).c_str());
            }
        }
        if ((cb_state.command_pool->queue_flags & VK_QUEUE_COMPUTE_BIT) == 0) {
            if (query_pool_ci.pipelineStatistics & VK_QUERY_PIPELINE_STATISTIC_COMPUTE_SHADER_INVOCATIONS_BIT) {
                skip |= LogError(
                    cb_state.commandBuffer(), vuids->vuid_compute_support,
                    "%s(): queryType of queryPool is VK_QUERY_TYPE_PIPELINE_STATISTICS (%s) and indicates compute operations, but "
                    "the command pool the command buffer %s was allocated from does not support compute operations (%s).",
                    cmd_name, string_VkQueryPipelineStatisticFlags(query_pool_ci.pipelineStatistics).c_str(),
                    report_data->FormatHandle(cb_state.commandBuffer()).c_str(),
                    string_VkQueueFlags(cb_state.command_pool->queue_flags).c_str());
            }
        }
    } else if (query_pool_ci.queryType == VK_QUERY_TYPE_PRIMITIVES_GENERATED_EXT) {
        if ((cb_state.command_pool->queue_flags & VK_QUEUE_GRAPHICS_BIT) == 0) {
            skip |= LogError(cb_state.commandBuffer(), vuids->vuid_primitives_generated,
                             "%s(): queryType of queryPool is VK_QUERY_TYPE_PRIMITIVES_GENERATED_EXT, but "
                             "the command pool the command buffer %s was allocated from does not support graphics operations (%s).",
                             cmd_name, report_data->FormatHandle(cb_state.commandBuffer()).c_str(),
                             string_VkQueueFlags(cb_state.command_pool->queue_flags).c_str());
        }
    }

    if (flags & VK_QUERY_CONTROL_PRECISE_BIT) {
        if (!enabled_features.core.occlusionQueryPrecise) {
            skip |= LogError(cb_state.commandBuffer(), vuids->vuid_precise,
                             "%s: VK_QUERY_CONTROL_PRECISE_BIT provided, but precise occlusion queries not enabled on the device.",
                             cmd_name);
        }

        if (query_pool_ci.queryType != VK_QUERY_TYPE_OCCLUSION) {
            skip |=
                LogError(cb_state.commandBuffer(), vuids->vuid_precise,
                         "%s: VK_QUERY_CONTROL_PRECISE_BIT provided, but pool query type is not VK_QUERY_TYPE_OCCLUSION", cmd_name);
        }
    }

    if (query_obj.query >= query_pool_ci.queryCount) {
        skip |= LogError(cb_state.commandBuffer(), vuids->vuid_query_count,
                         "%s: Query index %" PRIu32 " must be less than query count %" PRIu32 " of %s.", cmd_name, query_obj.query,
                         query_pool_ci.queryCount, report_data->FormatHandle(query_obj.pool).c_str());
    }

    if (cb_state.unprotected == false) {
        skip |= LogError(cb_state.commandBuffer(), vuids->vuid_protected_cb,
                         "%s: command can't be used in protected command buffers.", cmd_name);
    }

    if (cb_state.activeRenderPass) {
        const auto *render_pass_info = cb_state.activeRenderPass->createInfo.ptr();
        if (!cb_state.activeRenderPass->UsesDynamicRendering()) {
            const auto *subpass_desc = &render_pass_info->pSubpasses[cb_state.activeSubpass];
            if (subpass_desc) {
                uint32_t bits = GetBitSetCount(subpass_desc->viewMask);
                if (query_obj.query + bits > query_pool_state->createInfo.queryCount) {
                    skip |= LogError(cb_state.commandBuffer(), vuids->vuid_multiview_query,
                                     "%s: query (%" PRIu32 ") + bits set in current subpass view mask (%" PRIx32
                                     ") is greater than the number of queries in queryPool (%" PRIu32 ").",
                                     cmd_name, query_obj.query, subpass_desc->viewMask, query_pool_state->createInfo.queryCount);
                }
            }
        }
    }

    if (query_pool_ci.queryType == VK_QUERY_TYPE_RESULT_STATUS_ONLY_KHR) {
        const auto &qf_ext_props = queue_family_ext_props[cb_state.command_pool->queueFamilyIndex];
        if (!qf_ext_props.query_result_status_props.queryResultStatusSupport) {
            skip |= LogError(cb_state.commandBuffer(), vuids->vuid_result_status_support,
                             "%s: the command pool's queue family (index %u) the command buffer %s was allocated "
                             "from does not support result status queries",
                             cmd_name, cb_state.command_pool->queueFamilyIndex,
                             report_data->FormatHandle(cb_state.commandBuffer()).c_str());
        }
    }

    if (cb_state.bound_video_session) {
        if (cb_state.activeQueries.size() > 0) {
            LogObjectList objlist(cb_state.commandBuffer());
            objlist.add(cb_state.bound_video_session->Handle());
            skip |= LogError(objlist, vuids->vuid_no_active_in_vc_scope,
                             "%s: cannot start another query while there is already an active query in a "
                             "video coding scope (%s is bound)",
                             cmd_name, report_data->FormatHandle(cb_state.bound_video_session->videoSession()).c_str());
        }

        if (cb_state.bound_video_session->profile != query_pool_state->supported_video_profile) {
            LogObjectList objlist(cb_state.commandBuffer());
            objlist.add(query_pool_state->pool());
            objlist.add(cb_state.bound_video_session->Handle());
            skip |= LogError(objlist, vuids->vuid_result_status_profile_in_vc_scope,
                             "%s: the video profile %s was created with does not match the video profile of %s", cmd_name,
                             report_data->FormatHandle(query_pool_state->pool()).c_str(),
                             report_data->FormatHandle(cb_state.bound_video_session->videoSession()).c_str());
        }

        if (query_pool_ci.queryType != VK_QUERY_TYPE_RESULT_STATUS_ONLY_KHR) {
            LogObjectList objlist(cb_state.commandBuffer());
            objlist.add(cb_state.bound_video_session->Handle());
            skip |= LogError(objlist, vuids->vuid_vc_scope_query_type,
                             "%s: invalid query type used in a video coding scope (%s is bound)", cmd_name,
                             report_data->FormatHandle(cb_state.bound_video_session->videoSession()).c_str());
        }
    }

    skip |= ValidateCmd(cb_state, cmd);
    return skip;
}

bool CoreChecks::PreCallValidateCmdBeginQuery(VkCommandBuffer commandBuffer, VkQueryPool queryPool, uint32_t slot,
                                              VkFlags flags) const {
    if (disabled[query_validation]) return false;
    bool skip = false;
    auto cb_state = GetRead<CMD_BUFFER_STATE>(commandBuffer);
    assert(cb_state);
    QueryObject query_obj(queryPool, slot);
    auto query_pool_state = Get<QUERY_POOL_STATE>(query_obj.pool);
    if (query_pool_state->createInfo.queryType == VK_QUERY_TYPE_PRIMITIVES_GENERATED_EXT) {
        if (!enabled_features.primitives_generated_query_features.primitivesGeneratedQuery) {
            skip |= LogError(device, "VUID-vkCmdBeginQuery-queryType-06688",
                             "vkCreateQueryPool(): If pCreateInfo->queryType is VK_QUERY_TYPE_PRIMITIVES_GENERATED_EXT "
                             "primitivesGeneratedQuery feature must be enabled.");
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
            vuid_vc_scope_query_type = "VUID-vkCmdBeginQuery-queryType-07132";
        }
    };
    BeginQueryVuids vuids;
    skip |= ValidateBeginQuery(*cb_state, query_obj, flags, 0, CMD_BEGINQUERY, &vuids);
    return skip;
}

static QueryState GetLocalQueryState(const QueryMap *localQueryToStateMap, VkQueryPool queryPool, uint32_t queryIndex,
                                     uint32_t perfPass) {
    QueryObject query = QueryObject(QueryObject(queryPool, queryIndex), perfPass);

    auto iter = localQueryToStateMap->find(query);
    if (iter != localQueryToStateMap->end()) return iter->second;

    return QUERYSTATE_UNKNOWN;
}

bool CoreChecks::VerifyQueryIsReset(const CMD_BUFFER_STATE &cb_state, const QueryObject &query_obj, const CMD_TYPE cmd_type,
                                    VkQueryPool &firstPerfQueryPool, uint32_t perfPass, QueryMap *localQueryToStateMap) {
    bool skip = false;
    auto state_data = cb_state.dev_data;

    auto query_pool_state = state_data->Get<QUERY_POOL_STATE>(query_obj.pool);
    const auto &query_pool_ci = query_pool_state->createInfo;

    QueryState state = GetLocalQueryState(localQueryToStateMap, query_obj.pool, query_obj.query, perfPass);
    // If reset was in another command buffer, check the global map
    if (state == QUERYSTATE_UNKNOWN) {
        state = query_pool_state->GetQueryState(query_obj.query, perfPass);
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
        skip |= state_data->LogError(cb_state.Handle(), kVUID_Core_DrawState_QueryNotReset,
                                     "%s: %s and query %" PRIu32
                                     ": query not reset. "
                                     "After query pool creation, each query must be reset before it is used. "
                                     "Queries must also be reset between uses.",
                                     CommandTypeString(cmd_type), state_data->report_data->FormatHandle(query_obj.pool).c_str(),
                                     query_obj.query);
    }

    return skip;
}

bool CoreChecks::ValidatePerformanceQuery(const CMD_BUFFER_STATE &cb_state, const QueryObject &query_obj, const CMD_TYPE cmd_type,
                                          VkQueryPool &firstPerfQueryPool, uint32_t perfPass, QueryMap *localQueryToStateMap) {
    auto state_data = cb_state.dev_data;
    auto query_pool_state = state_data->Get<QUERY_POOL_STATE>(query_obj.pool);
    const auto &query_pool_ci = query_pool_state->createInfo;

    if (query_pool_ci.queryType != VK_QUERY_TYPE_PERFORMANCE_QUERY_KHR) return false;

    bool skip = false;

    if (perfPass >= query_pool_state->n_performance_passes) {
        skip |= state_data->LogError(cb_state.Handle(), "VUID-VkPerformanceQuerySubmitInfoKHR-counterPassIndex-03221",
                                     "%s: Invalid counterPassIndex (%u, maximum allowed %u) value for query pool %s.",
                                     CommandTypeString(cmd_type), perfPass, query_pool_state->n_performance_passes,
                                     state_data->report_data->FormatHandle(query_obj.pool).c_str());
    }

    if (!cb_state.performance_lock_acquired || cb_state.performance_lock_released) {
        skip |= state_data->LogError(cb_state.Handle(), "VUID-vkQueueSubmit-pCommandBuffers-03220",
                                     "%s: Commandbuffer %s was submitted and contains a performance query but the"
                                     "profiling lock was not held continuously throughout the recording of commands.",
                                     CommandTypeString(cmd_type), state_data->report_data->FormatHandle(cb_state.Handle()).c_str());
    }

    QueryState command_buffer_state = GetLocalQueryState(localQueryToStateMap, query_obj.pool, query_obj.query, perfPass);
    if (command_buffer_state == QUERYSTATE_RESET) {
        skip |= state_data->LogError(
            cb_state.Handle(), query_obj.indexed ? "VUID-vkCmdBeginQueryIndexedEXT-None-02863" : "VUID-vkCmdBeginQuery-None-02863",
            "%s: VkQuery begin command recorded in a command buffer that, either directly or "
            "through secondary command buffers, also contains a vkCmdResetQueryPool command "
            "affecting the same query.",
            CommandTypeString(cmd_type));
    }

    if (firstPerfQueryPool != VK_NULL_HANDLE) {
        if (firstPerfQueryPool != query_obj.pool &&
            !state_data->enabled_features.performance_query_features.performanceCounterMultipleQueryPools) {
            skip |= state_data->LogError(
                cb_state.Handle(),
                query_obj.indexed ? "VUID-vkCmdBeginQueryIndexedEXT-queryPool-03226" : "VUID-vkCmdBeginQuery-queryPool-03226",
                "%s: Commandbuffer %s contains more than one performance query pool but "
                "performanceCounterMultipleQueryPools is not enabled.",
                CommandTypeString(cmd_type), state_data->report_data->FormatHandle(cb_state.Handle()).c_str());
        }
    } else {
        firstPerfQueryPool = query_obj.pool;
    }

    return skip;
}

void CoreChecks::EnqueueVerifyBeginQuery(VkCommandBuffer command_buffer, const QueryObject &query_obj, const CMD_TYPE cmd_type) {
    auto cb_state = GetWrite<CMD_BUFFER_STATE>(command_buffer);

    // Enqueue the submit time validation here, ahead of the submit time state update in the StateTracker's PostCallRecord
    cb_state->queryUpdates.emplace_back([query_obj, cmd_type](CMD_BUFFER_STATE &cb_state_arg, bool do_validate,
                                                              VkQueryPool &firstPerfQueryPool, uint32_t perfPass,
                                                              QueryMap *localQueryToStateMap) {
        if (!do_validate) return false;
        bool skip = false;
        skip |= ValidatePerformanceQuery(cb_state_arg, query_obj, cmd_type, firstPerfQueryPool, perfPass, localQueryToStateMap);
        skip |= VerifyQueryIsReset(cb_state_arg, query_obj, cmd_type, firstPerfQueryPool, perfPass, localQueryToStateMap);
        return skip;
    });
}

void CoreChecks::PreCallRecordCmdBeginQuery(VkCommandBuffer commandBuffer, VkQueryPool queryPool, uint32_t slot, VkFlags flags) {
    if (disabled[query_validation]) return;
    QueryObject query_obj = {queryPool, slot};
    EnqueueVerifyBeginQuery(commandBuffer, query_obj, CMD_BEGINQUERY);
}

void CoreChecks::EnqueueVerifyEndQuery(CMD_BUFFER_STATE &cb_state, const QueryObject &query_obj) {
    // Enqueue the submit time validation here, ahead of the submit time state update in the StateTracker's PostCallRecord
    cb_state.queryUpdates.emplace_back([query_obj](CMD_BUFFER_STATE &cb_state_arg, bool do_validate,
                                                   VkQueryPool &firstPerfQueryPool, uint32_t perfPass,
                                                   QueryMap *localQueryToStateMap) {
        if (!do_validate) return false;
        bool skip = false;
        auto device_data = cb_state_arg.dev_data;
        auto query_pool_state = device_data->Get<QUERY_POOL_STATE>(query_obj.pool);
        if (query_pool_state->has_perf_scope_command_buffer && (cb_state_arg.command_count - 1) != query_obj.end_command_index) {
            skip |= device_data->LogError(cb_state_arg.Handle(), "VUID-vkCmdEndQuery-queryPool-03227",
                                          "vkCmdEndQuery: Query pool %s was created with a counter of scope "
                                          "VK_QUERY_SCOPE_COMMAND_BUFFER_KHR but the end of the query is not the last "
                                          "command in the command buffer %s.",
                                          device_data->report_data->FormatHandle(query_obj.pool).c_str(),
                                          device_data->report_data->FormatHandle(cb_state_arg.Handle()).c_str());
        }
        return skip;
    });
}

bool CoreChecks::ValidateCmdEndQuery(const CMD_BUFFER_STATE &cb_state, const QueryObject &query_obj, uint32_t index, CMD_TYPE cmd,
                                     const ValidateEndQueryVuids *vuids) const {
    bool skip = false;
    const char *cmd_name = CommandTypeString(cmd);
    if (!cb_state.activeQueries.count(query_obj)) {
        skip |= LogError(cb_state.commandBuffer(), vuids->vuid_active_queries,
                         "%s: Ending a query before it was started: %s, index %d.", cmd_name,
                         report_data->FormatHandle(query_obj.pool).c_str(), query_obj.query);
    }
    auto query_pool_state = Get<QUERY_POOL_STATE>(query_obj.pool);
    const auto &query_pool_ci = query_pool_state->createInfo;
    if (query_pool_ci.queryType == VK_QUERY_TYPE_PERFORMANCE_QUERY_KHR) {
        if (query_pool_state->has_perf_scope_render_pass && cb_state.activeRenderPass) {
            skip |= LogError(cb_state.commandBuffer(), "VUID-vkCmdEndQuery-queryPool-03228",
                             "%s: Query pool %s was created with a counter of scope "
                             "VK_QUERY_SCOPE_RENDER_PASS_KHR but %s is inside a render pass.",
                             cmd_name, report_data->FormatHandle(query_obj.pool).c_str(), cmd_name);
        }
    }
    skip |= ValidateCmd(cb_state, cmd);

    if (cb_state.unprotected == false) {
        skip |= LogError(cb_state.commandBuffer(), vuids->vuid_protected_cb,
                         "%s: command can't be used in protected command buffers.", cmd_name);
    }
    return skip;
}

bool CoreChecks::PreCallValidateCmdEndQuery(VkCommandBuffer commandBuffer, VkQueryPool queryPool, uint32_t slot) const {
    if (disabled[query_validation]) return false;
    bool skip = false;
    QueryObject query_obj = {queryPool, slot};
    auto cb_state = GetRead<CMD_BUFFER_STATE>(commandBuffer);
    assert(cb_state);

    auto query_pool_state = Get<QUERY_POOL_STATE>(queryPool);
    if (query_pool_state) {
        const uint32_t available_query_count = query_pool_state->createInfo.queryCount;
        // Only continue validating if the slot is even within range
        if (slot >= available_query_count) {
            skip |= LogError(cb_state->commandBuffer(), "VUID-vkCmdEndQuery-query-00810",
                             "vkCmdEndQuery(): query index (%u) is greater or equal to the queryPool size (%u).", slot,
                             available_query_count);
        } else {
            struct EndQueryVuids : ValidateEndQueryVuids {
                EndQueryVuids() : ValidateEndQueryVuids() {
                    vuid_active_queries = "VUID-vkCmdEndQuery-None-01923";
                    vuid_protected_cb = "VUID-vkCmdEndQuery-commandBuffer-01886";
                }
            };
            EndQueryVuids vuids;
            skip |= ValidateCmdEndQuery(*cb_state, query_obj, 0, CMD_ENDQUERY, &vuids);
        }
    }
    return skip;
}

void CoreChecks::PreCallRecordCmdEndQuery(VkCommandBuffer commandBuffer, VkQueryPool queryPool, uint32_t slot) {
    if (disabled[query_validation]) return;
    auto cb_state = GetWrite<CMD_BUFFER_STATE>(commandBuffer);
    QueryObject query_obj = {queryPool, slot};
    query_obj.end_command_index = cb_state->command_count;  // off by one because cb_state hasn't recorded this yet
    EnqueueVerifyEndQuery(*cb_state, query_obj);
}

bool CoreChecks::ValidateQueryPoolIndex(VkQueryPool queryPool, uint32_t firstQuery, uint32_t queryCount, const char *func_name,
                                        const char *first_vuid, const char *sum_vuid) const {
    bool skip = false;
    auto query_pool_state = Get<QUERY_POOL_STATE>(queryPool);
    if (query_pool_state) {
        const uint32_t available_query_count = query_pool_state->createInfo.queryCount;
        if (firstQuery >= available_query_count) {
            skip |= LogError(queryPool, first_vuid,
                             "%s: In Query %s the firstQuery (%u) is greater or equal to the queryPool size (%u).", func_name,
                             report_data->FormatHandle(queryPool).c_str(), firstQuery, available_query_count);
        }
        if ((firstQuery + queryCount) > available_query_count) {
            skip |=
                LogError(queryPool, sum_vuid,
                         "%s: In Query %s the sum of firstQuery (%u) + queryCount (%u) is greater than the queryPool size (%u).",
                         func_name, report_data->FormatHandle(queryPool).c_str(), firstQuery, queryCount, available_query_count);
        }
    }
    return skip;
}

bool CoreChecks::PreCallValidateCmdResetQueryPool(VkCommandBuffer commandBuffer, VkQueryPool queryPool, uint32_t firstQuery,
                                                  uint32_t queryCount) const {
    if (disabled[query_validation]) return false;
    auto cb_state = GetRead<CMD_BUFFER_STATE>(commandBuffer);
    assert(cb_state);

    bool skip = false;
    skip |= ValidateCmd(*cb_state, CMD_RESETQUERYPOOL);
    skip |= ValidateQueryPoolIndex(queryPool, firstQuery, queryCount, "VkCmdResetQueryPool()",
                                   "VUID-vkCmdResetQueryPool-firstQuery-00796", "VUID-vkCmdResetQueryPool-firstQuery-00797");

    return skip;
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

bool CoreChecks::ValidateCopyQueryPoolResults(const CMD_BUFFER_STATE &cb_state, VkQueryPool queryPool, uint32_t firstQuery,
                                              uint32_t queryCount, uint32_t perfPass, VkQueryResultFlags flags,
                                              QueryMap *localQueryToStateMap) {
    const auto state_data = cb_state.dev_data;
    bool skip = false;
    for (uint32_t i = 0; i < queryCount; i++) {
        QueryState state = GetLocalQueryState(localQueryToStateMap, queryPool, firstQuery + i, perfPass);
        QueryResultType result_type = GetQueryResultType(state, flags);
        if (result_type != QUERYRESULT_SOME_DATA && result_type != QUERYRESULT_UNKNOWN) {
            skip |= state_data->LogError(
                cb_state.Handle(), kVUID_Core_DrawState_InvalidQuery,
                "vkCmdCopyQueryPoolResults(): Requesting a copy from query to buffer on %s query %" PRIu32 ": %s",
                state_data->report_data->FormatHandle(queryPool).c_str(), firstQuery + i, string_QueryResultType(result_type));
        }
    }
    return skip;
}

bool CoreChecks::PreCallValidateCmdCopyQueryPoolResults(VkCommandBuffer commandBuffer, VkQueryPool queryPool, uint32_t firstQuery,
                                                        uint32_t queryCount, VkBuffer dstBuffer, VkDeviceSize dstOffset,
                                                        VkDeviceSize stride, VkQueryResultFlags flags) const {
    if (disabled[query_validation]) return false;
    auto cb_state = GetRead<CMD_BUFFER_STATE>(commandBuffer);
    auto dst_buff_state = Get<BUFFER_STATE>(dstBuffer);
    assert(cb_state);
    assert(dst_buff_state);
    bool skip = ValidateMemoryIsBoundToBuffer(commandBuffer, *dst_buff_state, "vkCmdCopyQueryPoolResults()",
                                              "VUID-vkCmdCopyQueryPoolResults-dstBuffer-00826");
    skip |= ValidateQueryPoolStride("VUID-vkCmdCopyQueryPoolResults-flags-00822", "VUID-vkCmdCopyQueryPoolResults-flags-00823",
                                    stride, "dstOffset", dstOffset, flags);
    // Validate that DST buffer has correct usage flags set
    skip |= ValidateBufferUsageFlags(commandBuffer, *dst_buff_state, VK_BUFFER_USAGE_TRANSFER_DST_BIT, true,
                                     "VUID-vkCmdCopyQueryPoolResults-dstBuffer-00825", "vkCmdCopyQueryPoolResults()",
                                     "VK_BUFFER_USAGE_TRANSFER_DST_BIT");
    skip |= ValidateCmd(*cb_state, CMD_COPYQUERYPOOLRESULTS);
    skip |= ValidateQueryPoolIndex(queryPool, firstQuery, queryCount, "vkCmdCopyQueryPoolResults()",
                                   "VUID-vkCmdCopyQueryPoolResults-firstQuery-00820",
                                   "VUID-vkCmdCopyQueryPoolResults-firstQuery-00821");

    if (dstOffset >= dst_buff_state->requirements.size) {
        skip |= LogError(commandBuffer, "VUID-vkCmdCopyQueryPoolResults-dstOffset-00819",
                         "vkCmdCopyQueryPoolResults() dstOffset (0x%" PRIxLEAST64 ") is not less than the size (0x%" PRIxLEAST64
                         ") of buffer (%s).",
                         dstOffset, dst_buff_state->requirements.size, report_data->FormatHandle(dst_buff_state->buffer()).c_str());
    } else if (dstOffset + (queryCount * stride) > dst_buff_state->requirements.size) {
        skip |=
            LogError(commandBuffer, "VUID-vkCmdCopyQueryPoolResults-dstBuffer-00824",
                     "vkCmdCopyQueryPoolResults() storage required (0x%" PRIxLEAST64
                     ") equal to dstOffset + (queryCount * stride) is greater than the size (0x%" PRIxLEAST64 ") of buffer (%s).",
                     dstOffset + (queryCount * stride), dst_buff_state->requirements.size,
                     report_data->FormatHandle(dst_buff_state->buffer()).c_str());
    }

    if ((flags & VK_QUERY_RESULT_WITH_STATUS_BIT_KHR) && (flags & VK_QUERY_RESULT_WITH_AVAILABILITY_BIT)) {
        skip |= LogError(device, "VUID-vkCmdCopyQueryPoolResults-flags-06902",
                         "vkCmdCopyQueryPoolResults(): flags include both VK_QUERY_RESULT_WITH_STATUS_BIT_KHR bit and "
                         "VK_QUERY_RESULT_WITH_AVAILABILITY_BIT bit");
    }

    auto query_pool_state = Get<QUERY_POOL_STATE>(queryPool);
    if (query_pool_state) {
        if (query_pool_state->createInfo.queryType == VK_QUERY_TYPE_PERFORMANCE_QUERY_KHR) {
            skip |=
                ValidatePerformanceQueryResults("vkCmdCopyQueryPoolResults", query_pool_state.get(), firstQuery, queryCount, flags);
            if (!phys_dev_ext_props.performance_query_props.allowCommandBufferQueryCopies) {
                skip |= LogError(commandBuffer, "VUID-vkCmdCopyQueryPoolResults-queryType-03232",
                                 "vkCmdCopyQueryPoolResults called with query pool %s but "
                                 "VkPhysicalDevicePerformanceQueryPropertiesKHR::allowCommandBufferQueryCopies "
                                 "is not set.",
                                 report_data->FormatHandle(queryPool).c_str());
            }
        }
        if ((query_pool_state->createInfo.queryType == VK_QUERY_TYPE_TIMESTAMP) && ((flags & VK_QUERY_RESULT_PARTIAL_BIT) != 0)) {
            skip |= LogError(commandBuffer, "VUID-vkCmdCopyQueryPoolResults-queryType-00827",
                             "vkCmdCopyQueryPoolResults() query pool %s was created with VK_QUERY_TYPE_TIMESTAMP so flags must not "
                             "contain VK_QUERY_RESULT_PARTIAL_BIT.",
                             report_data->FormatHandle(queryPool).c_str());
        }
        if (query_pool_state->createInfo.queryType == VK_QUERY_TYPE_PERFORMANCE_QUERY_INTEL) {
            skip |= LogError(queryPool, "VUID-vkCmdCopyQueryPoolResults-queryType-02734",
                             "vkCmdCopyQueryPoolResults() called but QueryPool %s was created with queryType "
                             "VK_QUERY_TYPE_PERFORMANCE_QUERY_INTEL.",
                             report_data->FormatHandle(queryPool).c_str());
        }
        if (query_pool_state->createInfo.queryType == VK_QUERY_TYPE_RESULT_STATUS_ONLY_KHR &&
            (flags & VK_QUERY_RESULT_WITH_STATUS_BIT_KHR) == 0) {
            skip |= LogError(queryPool, "VUID-vkCmdCopyQueryPoolResults-queryType-06901",
                             "vkCmdCopyQueryPoolResults(): %s was created with queryType VK_QUERY_TYPE_RESULT_STATUS_ONLY_KHR "
                             "but flags does not include VK_QUERY_RESULT_WITH_STATUS_BIT_KHR",
                             report_data->FormatHandle(queryPool).c_str());
        }
    }

    return skip;
}

void CoreChecks::PreCallRecordCmdCopyQueryPoolResults(VkCommandBuffer commandBuffer, VkQueryPool queryPool, uint32_t firstQuery,
                                                      uint32_t queryCount, VkBuffer dstBuffer, VkDeviceSize dstOffset,
                                                      VkDeviceSize stride, VkQueryResultFlags flags) {
    if (disabled[query_validation]) return;
    auto cb_state = GetWrite<CMD_BUFFER_STATE>(commandBuffer);
    cb_state->queryUpdates.emplace_back([queryPool, firstQuery, queryCount, flags](
                                            CMD_BUFFER_STATE &cb_state_arg, bool do_validate, VkQueryPool &firstPerfQueryPool,
                                            uint32_t perfPass, QueryMap *localQueryToStateMap) {
        if (!do_validate) return false;
        return ValidateCopyQueryPoolResults(cb_state_arg, queryPool, firstQuery, queryCount, perfPass, flags, localQueryToStateMap);
    });
}

bool CoreChecks::PreCallValidateCmdWriteTimestamp(VkCommandBuffer commandBuffer, VkPipelineStageFlagBits pipelineStage,
                                                  VkQueryPool queryPool, uint32_t slot) const {
    if (disabled[query_validation]) return false;
    auto cb_state = GetRead<CMD_BUFFER_STATE>(commandBuffer);
    assert(cb_state);
    bool skip = false;
    skip |= ValidateCmd(*cb_state, CMD_WRITETIMESTAMP);

    auto query_pool_state = Get<QUERY_POOL_STATE>(queryPool);
    if ((query_pool_state != nullptr) && (query_pool_state->createInfo.queryType != VK_QUERY_TYPE_TIMESTAMP)) {
        skip |= LogError(cb_state->commandBuffer(), "VUID-vkCmdWriteTimestamp-queryPool-01416",
                         "vkCmdWriteTimestamp(): Query Pool %s was not created with VK_QUERY_TYPE_TIMESTAMP.",
                         report_data->FormatHandle(queryPool).c_str());
    }

    const uint32_t timestamp_valid_bits =
        physical_device_state->queue_family_properties[cb_state->command_pool->queueFamilyIndex].timestampValidBits;
    if (timestamp_valid_bits == 0) {
        skip |= LogError(cb_state->commandBuffer(), "VUID-vkCmdWriteTimestamp-timestampValidBits-00829",
                         "vkCmdWriteTimestamp(): Query Pool %s has a timestampValidBits value of zero.",
                         report_data->FormatHandle(queryPool).c_str());
    }

    if ((query_pool_state != nullptr) && (slot >= query_pool_state->createInfo.queryCount)) {
        skip |= LogError(cb_state->commandBuffer(), "VUID-vkCmdWriteTimestamp-query-04904",
                         "vkCmdWriteTimestamp(): query (%" PRIu32 ") is not lower than the number of queries (%" PRIu32
                         ") in Query pool %s.",
                         slot, query_pool_state->createInfo.queryCount, report_data->FormatHandle(queryPool).c_str());
    }

    return skip;
}

bool CoreChecks::ValidateCmdWriteTimestamp2(VkCommandBuffer commandBuffer, VkPipelineStageFlags2 stage, VkQueryPool queryPool,
                                            uint32_t slot, CMD_TYPE cmd_type) const {
    if (disabled[query_validation]) return false;
    auto cb_state = GetRead<CMD_BUFFER_STATE>(commandBuffer);
    assert(cb_state);
    bool skip = false;

    const char *func_name = CommandTypeString(cmd_type);
    if (!enabled_features.core13.synchronization2) {
        skip |= LogError(commandBuffer, "VUID-vkCmdWriteTimestamp2-synchronization2-03858",
                         "%s(): Synchronization2 feature is not enabled", func_name);
    }
    skip |= ValidateCmd(*cb_state, cmd_type);

    Location loc(Func::vkCmdWriteTimestamp2, Field::stage);
    if ((stage & (stage - 1)) != 0) {
        skip |=
            LogError(cb_state->commandBuffer(), "VUID-vkCmdWriteTimestamp2-stage-03859",
                     "%s (%s) must only set a single pipeline stage.", func_name, string_VkPipelineStageFlags2KHR(stage).c_str());
    }
    skip |= ValidatePipelineStage(LogObjectList(cb_state->commandBuffer()), loc, cb_state->GetQueueFlags(), stage);

    loc.field = Field::queryPool;
    auto query_pool_state = Get<QUERY_POOL_STATE>(queryPool);

    if (query_pool_state) {
        if (query_pool_state->createInfo.queryType != VK_QUERY_TYPE_TIMESTAMP) {
            skip |= LogError(cb_state->commandBuffer(), "VUID-vkCmdWriteTimestamp2-queryPool-03861",
                             "%s Query Pool %s was not created with VK_QUERY_TYPE_TIMESTAMP.", func_name,
                             report_data->FormatHandle(queryPool).c_str());
        }

        if (slot >= query_pool_state->createInfo.queryCount) {
            skip |= LogError(cb_state->commandBuffer(), "VUID-vkCmdWriteTimestamp2-query-04903",
                             "vkCmdWriteTimestamp2KHR(): query (%" PRIu32 ") is not lower than the number of queries (%" PRIu32
                             ") in Query pool %s.",
                             slot, query_pool_state->createInfo.queryCount, report_data->FormatHandle(queryPool).c_str());
        }
    }

    const uint32_t timestampValidBits =
        physical_device_state->queue_family_properties[cb_state->command_pool->queueFamilyIndex].timestampValidBits;
    if (timestampValidBits == 0) {
        skip |= LogError(cb_state->commandBuffer(), "VUID-vkCmdWriteTimestamp2-timestampValidBits-03863",
                         "%s Query Pool %s has a timestampValidBits value of zero.", func_name,
                         report_data->FormatHandle(queryPool).c_str());
    }

    return skip;
}

bool CoreChecks::PreCallValidateCmdWriteTimestamp2KHR(VkCommandBuffer commandBuffer, VkPipelineStageFlags2KHR stage,
                                                      VkQueryPool queryPool, uint32_t query) const {
    return ValidateCmdWriteTimestamp2(commandBuffer, stage, queryPool, query, CMD_WRITETIMESTAMP2KHR);
}

bool CoreChecks::PreCallValidateCmdWriteTimestamp2(VkCommandBuffer commandBuffer, VkPipelineStageFlags2 stage,
                                                   VkQueryPool queryPool, uint32_t query) const {
    return ValidateCmdWriteTimestamp2(commandBuffer, stage, queryPool, query, CMD_WRITETIMESTAMP2);
}

void CoreChecks::PreCallRecordCmdWriteTimestamp(VkCommandBuffer commandBuffer, VkPipelineStageFlagBits pipelineStage,
                                                VkQueryPool queryPool, uint32_t slot) {
    if (disabled[query_validation]) return;
    // Enqueue the submit time validation check here, before the submit time state update in StateTracker::PostCall...
    auto cb_state = GetWrite<CMD_BUFFER_STATE>(commandBuffer);
    QueryObject query = {queryPool, slot};
    CMD_TYPE cmd_type = CMD_WRITETIMESTAMP;
    cb_state->queryUpdates.emplace_back([query, cmd_type](CMD_BUFFER_STATE &cb_state_arg, bool do_validate,
                                                          VkQueryPool &firstPerfQueryPool, uint32_t perfPass,
                                                          QueryMap *localQueryToStateMap) {
        if (!do_validate) return false;
        return VerifyQueryIsReset(cb_state_arg, query, cmd_type, firstPerfQueryPool, perfPass, localQueryToStateMap);
    });
}

void CoreChecks::PreCallRecordCmdWriteTimestamp2KHR(VkCommandBuffer commandBuffer, VkPipelineStageFlags2KHR pipelineStage,
                                                    VkQueryPool queryPool, uint32_t slot) {
    if (disabled[query_validation]) return;
    // Enqueue the submit time validation check here, before the submit time state update in StateTracker::PostCall...
    auto cb_state = GetWrite<CMD_BUFFER_STATE>(commandBuffer);
    QueryObject query = {queryPool, slot};
    CMD_TYPE cmd_type = CMD_WRITETIMESTAMP2KHR;
    cb_state->queryUpdates.emplace_back([query, cmd_type](CMD_BUFFER_STATE &cb_state_arg, bool do_validate,
                                                          VkQueryPool &firstPerfQueryPool, uint32_t perfPass,
                                                          QueryMap *localQueryToStateMap) {
        if (!do_validate) return false;
        return VerifyQueryIsReset(cb_state_arg, query, cmd_type, firstPerfQueryPool, perfPass, localQueryToStateMap);
    });
}

void CoreChecks::PreCallRecordCmdWriteTimestamp2(VkCommandBuffer commandBuffer, VkPipelineStageFlags2 pipelineStage,
                                                 VkQueryPool queryPool, uint32_t slot) {
    if (disabled[query_validation]) return;
    // Enqueue the submit time validation check here, before the submit time state update in StateTracker::PostCall...
    auto cb_state = GetWrite<CMD_BUFFER_STATE>(commandBuffer);
    QueryObject query = {queryPool, slot};
    CMD_TYPE cmd_type = CMD_WRITETIMESTAMP2;
    cb_state->queryUpdates.emplace_back([query, cmd_type](CMD_BUFFER_STATE &cb_state_arg, bool do_validate,
                                                          VkQueryPool &firstPerfQueryPool, uint32_t perfPass,
                                                          QueryMap *localQueryToStateMap) {
        if (!do_validate) return false;
        return VerifyQueryIsReset(cb_state_arg, query, cmd_type, firstPerfQueryPool, perfPass, localQueryToStateMap);
    });
}

bool CoreChecks::PreCallValidateCmdBeginQueryIndexedEXT(VkCommandBuffer commandBuffer, VkQueryPool queryPool, uint32_t query,
                                                        VkQueryControlFlags flags, uint32_t index) const {
    if (disabled[query_validation]) return false;
    auto cb_state = GetRead<CMD_BUFFER_STATE>(commandBuffer);
    assert(cb_state);
    QueryObject query_obj(queryPool, query, index);
    const char *cmd_name = "vkCmdBeginQueryIndexedEXT()";
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
            vuid_vc_scope_query_type = "VUID-vkCmdBeginQueryIndexedEXT-queryType-07132";
        }
    };
    BeginQueryIndexedVuids vuids;
    bool skip = ValidateBeginQuery(*cb_state, query_obj, flags, index, CMD_BEGINQUERYINDEXEDEXT, &vuids);

    // Extension specific VU's
    const auto query_pool_state = Get<QUERY_POOL_STATE>(query_obj.pool);
    const auto &query_pool_ci = query_pool_state->createInfo;
    if (IsExtEnabled(device_extensions.vk_ext_primitives_generated_query)) {
        if ((query_pool_ci.queryType != VK_QUERY_TYPE_TRANSFORM_FEEDBACK_STREAM_EXT) &&
            (query_pool_ci.queryType != VK_QUERY_TYPE_PRIMITIVES_GENERATED_EXT)) {
            if (index != 0) {
                skip |= LogError(cb_state->commandBuffer(), "VUID-vkCmdBeginQueryIndexedEXT-queryType-06692",
                                 "%s: index %" PRIu32
                                 " must be zero if %s was not created with type VK_QUERY_TYPE_TRANSFORM_FEEDBACK_STREAM_EXT or "
                                 "VK_QUERY_TYPE_PRIMITIVES_GENERATED_EXT",
                                 cmd_name, index, report_data->FormatHandle(queryPool).c_str());
            }
        } else if (query_pool_ci.queryType == VK_QUERY_TYPE_PRIMITIVES_GENERATED_EXT) {
            if (!enabled_features.primitives_generated_query_features.primitivesGeneratedQuery) {
                skip |= LogError(cb_state->commandBuffer(), "VUID-vkCmdBeginQueryIndexedEXT-queryType-06693",
                                 "%s(): queryType of queryPool is VK_QUERY_TYPE_PRIMITIVES_GENERATED_EXT, "
                                 "but the primitivesGeneratedQuery feature is not enabled.",
                                 cmd_name);
            }
            if (index >= phys_dev_ext_props.transform_feedback_props.maxTransformFeedbackStreams) {
                skip |= LogError(cb_state->commandBuffer(), "VUID-vkCmdBeginQueryIndexedEXT-queryType-06690",
                                 "%s(): queryType of queryPool is VK_QUERY_TYPE_PRIMITIVES_GENERATED_EXT, but "
                                 "index (%" PRIu32
                                 ") is greater than or equal to "
                                 "VkPhysicalDeviceTransformFeedbackPropertiesEXT::maxTransformFeedbackStreams (%" PRIu32 ")",
                                 cmd_name, index, phys_dev_ext_props.transform_feedback_props.maxTransformFeedbackStreams);
            }
            if ((index != 0) &&
                (!enabled_features.primitives_generated_query_features.primitivesGeneratedQueryWithNonZeroStreams)) {
                skip |= LogError(cb_state->commandBuffer(), "VUID-vkCmdBeginQueryIndexedEXT-queryType-06691",
                                 "%s(): queryType of queryPool is VK_QUERY_TYPE_PRIMITIVES_GENERATED_EXT, but "
                                 "index (%" PRIu32
                                 ") is not zero and the primitivesGeneratedQueryWithNonZeroStreams feature is not enabled",
                                 cmd_name, index);
            }
        }
    } else if (query_pool_ci.queryType == VK_QUERY_TYPE_TRANSFORM_FEEDBACK_STREAM_EXT) {
        if (IsExtEnabled(device_extensions.vk_ext_transform_feedback) &&
            (index >= phys_dev_ext_props.transform_feedback_props.maxTransformFeedbackStreams)) {
            skip |= LogError(
                cb_state->commandBuffer(), "VUID-vkCmdBeginQueryIndexedEXT-queryType-02339",
                "%s: index %" PRIu32
                " must be less than VkPhysicalDeviceTransformFeedbackPropertiesEXT::maxTransformFeedbackStreams %" PRIu32 ".",
                cmd_name, index, phys_dev_ext_props.transform_feedback_props.maxTransformFeedbackStreams);
        }
    } else if (index != 0) {
        skip |= LogError(cb_state->commandBuffer(), "VUID-vkCmdBeginQueryIndexedEXT-queryType-02340",
                         "%s: index %" PRIu32
                         " must be zero if %s was not created with type VK_QUERY_TYPE_TRANSFORM_FEEDBACK_STREAM_EXT.",
                         cmd_name, index, report_data->FormatHandle(queryPool).c_str());
    }
    return skip;
}

void CoreChecks::PreCallRecordCmdBeginQueryIndexedEXT(VkCommandBuffer commandBuffer, VkQueryPool queryPool, uint32_t query,
                                                      VkQueryControlFlags flags, uint32_t index) {
    if (disabled[query_validation]) return;
    QueryObject query_obj = {queryPool, query, index};
    EnqueueVerifyBeginQuery(commandBuffer, query_obj, CMD_BEGINQUERYINDEXEDEXT);
}

void CoreChecks::PreCallRecordCmdEndQueryIndexedEXT(VkCommandBuffer commandBuffer, VkQueryPool queryPool, uint32_t query,
                                                    uint32_t index) {
    if (disabled[query_validation]) return;
    auto cb_state = GetWrite<CMD_BUFFER_STATE>(commandBuffer);
    QueryObject query_obj = {queryPool, query, index};
    query_obj.end_command_index = cb_state->command_count;  // off by one because cb_state hasn't recorded this yet
    EnqueueVerifyEndQuery(*cb_state, query_obj);
}

bool CoreChecks::PreCallValidateCmdEndQueryIndexedEXT(VkCommandBuffer commandBuffer, VkQueryPool queryPool, uint32_t query,
                                                      uint32_t index) const {
    if (disabled[query_validation]) return false;
    QueryObject query_obj = {queryPool, query, index};
    auto cb_state = GetRead<CMD_BUFFER_STATE>(commandBuffer);
    assert(cb_state);
    struct EndQueryIndexedVuids : ValidateEndQueryVuids {
        EndQueryIndexedVuids() : ValidateEndQueryVuids() {
            vuid_active_queries = "VUID-vkCmdEndQueryIndexedEXT-None-02342";
            vuid_protected_cb = "VUID-vkCmdEndQueryIndexedEXT-commandBuffer-02344";
        }
    };
    EndQueryIndexedVuids vuids;
    bool skip = false;
    skip |= ValidateCmdEndQuery(*cb_state, query_obj, index, CMD_ENDQUERYINDEXEDEXT, &vuids);

    auto query_pool_state = Get<QUERY_POOL_STATE>(queryPool);
    if (query_pool_state) {
        const auto &query_pool_ci = query_pool_state->createInfo;
        const uint32_t available_query_count = query_pool_state->createInfo.queryCount;
        if (query >= available_query_count) {
            skip |= LogError(cb_state->commandBuffer(), "VUID-vkCmdEndQueryIndexedEXT-query-02343",
                             "vkCmdEndQueryIndexedEXT(): query index (%" PRIu32
                             ") is greater or equal to the queryPool size (%" PRIu32 ").",
                             index, available_query_count);
        }
        if (IsExtEnabled(device_extensions.vk_ext_primitives_generated_query)) {
            if (query_pool_ci.queryType == VK_QUERY_TYPE_TRANSFORM_FEEDBACK_STREAM_EXT ||
                query_pool_ci.queryType == VK_QUERY_TYPE_PRIMITIVES_GENERATED_EXT) {
                if (index >= phys_dev_ext_props.transform_feedback_props.maxTransformFeedbackStreams) {
                    skip |= LogError(cb_state->commandBuffer(), "VUID-vkCmdEndQueryIndexedEXT-queryType-06694",
                                     "vkCmdEndQueryIndexedEXT(): index %" PRIu32
                                     " must be less than "
                                     "VkPhysicalDeviceTransformFeedbackPropertiesEXT::maxTransformFeedbackStreams %" PRIu32 ".",
                                     index, phys_dev_ext_props.transform_feedback_props.maxTransformFeedbackStreams);
                }
                for (const auto &query_object : cb_state->startedQueries) {
                    if (query_object.pool == queryPool && query_object.query == query) {
                        if (query_object.index != index) {
                            skip |= LogError(cb_state->commandBuffer(), "VUID-vkCmdEndQueryIndexedEXT-queryType-06696",
                                             "vkCmdEndQueryIndexedEXT(): queryPool is of type %s, but "
                                             "index (%" PRIu32 ") is not equal to the index used to begin the query (%" PRIu32 ")",
                                             string_VkQueryType(query_pool_ci.queryType), index, query_object.index);
                        }
                        break;
                    }
                }
            } else if (index != 0) {
                skip |= LogError(cb_state->commandBuffer(), "VUID-vkCmdEndQueryIndexedEXT-queryType-06695",
                                 "vkCmdEndQueryIndexedEXT(): index %" PRIu32
                                 " must be zero if %s was not created with type VK_QUERY_TYPE_TRANSFORM_FEEDBACK_STREAM_EXT and not"
                                 " VK_QUERY_TYPE_PRIMITIVES_GENERATED_EXT.",
                                 index, report_data->FormatHandle(queryPool).c_str());
            }
        } else {
            if (query_pool_ci.queryType == VK_QUERY_TYPE_TRANSFORM_FEEDBACK_STREAM_EXT) {
                if (index >= phys_dev_ext_props.transform_feedback_props.maxTransformFeedbackStreams) {
                    skip |= LogError(
                        cb_state->commandBuffer(), "VUID-vkCmdEndQueryIndexedEXT-queryType-02346",
                        "vkCmdEndQueryIndexedEXT(): index %" PRIu32
                        " must be less than VkPhysicalDeviceTransformFeedbackPropertiesEXT::maxTransformFeedbackStreams %" PRIu32
                        ".",
                        index, phys_dev_ext_props.transform_feedback_props.maxTransformFeedbackStreams);
                }
            } else if (index != 0) {
                skip |= LogError(cb_state->commandBuffer(), "VUID-vkCmdEndQueryIndexedEXT-queryType-02347",
                                 "vkCmdEndQueryIndexedEXT(): index %" PRIu32
                                 " must be zero if %s was not created with type VK_QUERY_TYPE_TRANSFORM_FEEDBACK_STREAM_EXT.",
                                 index, report_data->FormatHandle(queryPool).c_str());
            }
        }
    }

    return skip;
}

bool CoreChecks::ValidateQueryRange(VkDevice device, VkQueryPool queryPool, uint32_t totalCount, uint32_t firstQuery,
                                    uint32_t queryCount, const char *vuid_badfirst, const char *vuid_badrange,
                                    const char *apiName) const {
    bool skip = false;

    if (firstQuery >= totalCount) {
        skip |= LogError(device, vuid_badfirst,
                         "%s(): firstQuery (%" PRIu32 ") greater than or equal to query pool count (%" PRIu32 ") for %s", apiName,
                         firstQuery, totalCount, report_data->FormatHandle(queryPool).c_str());
    }

    if ((firstQuery + queryCount) > totalCount) {
        skip |= LogError(device, vuid_badrange,
                         "%s(): Query range [%" PRIu32 ", %" PRIu32 ") goes beyond query pool count (%" PRIu32 ") for %s", apiName,
                         firstQuery, firstQuery + queryCount, totalCount, report_data->FormatHandle(queryPool).c_str());
    }

    return skip;
}

bool CoreChecks::ValidateResetQueryPool(VkDevice device, VkQueryPool queryPool, uint32_t firstQuery, uint32_t queryCount,
                                        const char *apiName) const {
    if (disabled[query_validation]) return false;

    bool skip = false;

    if (!enabled_features.core12.hostQueryReset) {
        skip |= LogError(device, "VUID-vkResetQueryPool-None-02665", "%s(): Host query reset not enabled for device", apiName);
    }

    auto query_pool_state = Get<QUERY_POOL_STATE>(queryPool);
    if (query_pool_state) {
        skip |= ValidateQueryRange(device, queryPool, query_pool_state->createInfo.queryCount, firstQuery, queryCount,
                                   "VUID-vkResetQueryPool-firstQuery-02666", "VUID-vkResetQueryPool-firstQuery-02667", apiName);
    }

    return skip;
}

bool CoreChecks::PreCallValidateResetQueryPoolEXT(VkDevice device, VkQueryPool queryPool, uint32_t firstQuery,
                                                  uint32_t queryCount) const {
    return ValidateResetQueryPool(device, queryPool, firstQuery, queryCount, "vkResetQueryPoolEXT");
}

bool CoreChecks::PreCallValidateResetQueryPool(VkDevice device, VkQueryPool queryPool, uint32_t firstQuery,
                                               uint32_t queryCount) const {
    return ValidateResetQueryPool(device, queryPool, firstQuery, queryCount, "vkResetQueryPool");
}

bool CoreChecks::ValidateQueryPoolStride(const std::string &vuid_not_64, const std::string &vuid_64, const VkDeviceSize stride,
                                         const char *parameter_name, const uint64_t parameter_value,
                                         const VkQueryResultFlags flags) const {
    bool skip = false;
    if (flags & VK_QUERY_RESULT_64_BIT) {
        static const int condition_multiples = 0b0111;
        if ((stride & condition_multiples) || (parameter_value & condition_multiples)) {
            skip |= LogError(device, vuid_64, "stride %" PRIx64 " or %s %" PRIx64 " is invalid.", stride, parameter_name,
                             parameter_value);
        }
    } else {
        static const int condition_multiples = 0b0011;
        if ((stride & condition_multiples) || (parameter_value & condition_multiples)) {
            skip |= LogError(device, vuid_not_64, "stride %" PRIx64 " or %s %" PRIx64 " is invalid.", stride, parameter_name,
                             parameter_value);
        }
    }
    return skip;
}

void CoreChecks::PostCallRecordGetQueryPoolResults(VkDevice device, VkQueryPool queryPool, uint32_t firstQuery, uint32_t queryCount,
                                                   size_t dataSize, void *pData, VkDeviceSize stride, VkQueryResultFlags flags,
                                                   VkResult result) {
    if (result != VK_SUCCESS) {
        return;
    }
    auto query_pool_state = Get<QUERY_POOL_STATE>(queryPool);
    if ((flags & VK_QUERY_RESULT_PARTIAL_BIT) == 0) {
        for (uint32_t i = firstQuery; i < queryCount; ++i) {
            query_pool_state->SetQueryState(i, 0, QUERYSTATE_AVAILABLE);
        }
    }
}

bool CoreChecks::PreCallValidateReleaseProfilingLockKHR(VkDevice device) const {
    bool skip = false;

    if (!performance_lock_acquired) {
        skip |= LogError(device, "VUID-vkReleaseProfilingLockKHR-device-03235",
                         "vkReleaseProfilingLockKHR(): The profiling lock of device must have been held via a previous successful "
                         "call to vkAcquireProfilingLockKHR.");
    }

    return skip;
}
