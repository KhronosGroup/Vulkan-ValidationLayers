/* Copyright (c) 2015-2023 The Khronos Group Inc.
 * Copyright (c) 2015-2023 Valve Corporation
 * Copyright (c) 2015-2023 LunarG, Inc.
 * Copyright (C) 2015-2023 Google Inc.
 * Modifications Copyright (C) 2020-2022 Advanced Micro Devices, Inc. All rights reserved.
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

#include <string>
#include <vector>

#include "generated/vk_enum_string_helper.h"
#include "generated/chassis.h"
#include "core_validation.h"

bool CoreChecks::ValidatePipelineDerivatives(std::vector<std::shared_ptr<PIPELINE_STATE>> const &pipelines,
                                             uint32_t pipe_index) const {
    bool skip = false;
    const auto &pipeline = *pipelines[pipe_index].get();
    // If create derivative bit is set, check that we've specified a base
    // pipeline correctly, and that the base pipeline was created to allow
    // derivatives.
    if (pipeline.create_flags & VK_PIPELINE_CREATE_DERIVATIVE_BIT) {
        std::shared_ptr<const PIPELINE_STATE> base_pipeline;
        const VkPipeline base_handle = pipeline.BasePipeline<VkGraphicsPipelineCreateInfo>();
        const int32_t base_index = pipeline.BasePipelineIndex<VkGraphicsPipelineCreateInfo>();
        if (!((base_handle != VK_NULL_HANDLE) ^ (base_index != -1))) {
            skip |= LogError(device, "VUID-VkGraphicsPipelineCreateInfo-flags-07986",
                             "vkCreateGraphicsPipelines(): pCreateInfos[%" PRIu32
                             "]: exactly one of base pipeline index and handle must be specified",
                             pipeline.create_index);
        } else if (base_index != -1) {
            if (static_cast<uint32_t>(base_index) >= pipeline.create_index) {
                skip |= LogError(base_handle, "VUID-vkCreateGraphicsPipelines-flags-00720",
                                 "vkCreateGraphicsPipelines(): pCreateInfos[%" PRIu32
                                 "]: base pipeline must occur earlier in array than derivative pipeline.",
                                 pipeline.create_index);
            } else {
                base_pipeline = pipelines[base_index];
            }
        } else if (base_handle != VK_NULL_HANDLE) {
            base_pipeline = Get<PIPELINE_STATE>(base_handle);
        }

        if (base_pipeline && !(base_pipeline->create_flags & VK_PIPELINE_CREATE_ALLOW_DERIVATIVES_BIT)) {
            skip |= LogError(base_pipeline->pipeline(), "VUID-vkCreateGraphicsPipelines-flags-00721",
                             "vkCreateGraphicsPipelines(): pCreateInfos[%" PRIu32 "]: base pipeline does not allow derivatives.",
                             pipeline.create_index);
        }
    }
    return skip;
}

bool CoreChecks::ValidatePipelineCacheControlFlags(VkPipelineCreateFlags flags, uint32_t index, const char *caller_name,
                                                   const char *vuid) const {
    bool skip = false;
    if (enabled_features.core13.pipelineCreationCacheControl == VK_FALSE) {
        const VkPipelineCreateFlags invalid_flags =
            VK_PIPELINE_CREATE_FAIL_ON_PIPELINE_COMPILE_REQUIRED_BIT_EXT | VK_PIPELINE_CREATE_EARLY_RETURN_ON_FAILURE_BIT_EXT;
        if ((flags & invalid_flags) != 0) {
            skip |= LogError(device, vuid,
                             "%s(): pipelineCreationCacheControl is turned off but pCreateInfos[%" PRIu32
                             "]has VkPipelineCreateFlags "
                             "containing VK_PIPELINE_CREATE_FAIL_ON_PIPELINE_COMPILE_REQUIRED_BIT_EXT or "
                             "VK_PIPELINE_CREATE_EARLY_RETURN_ON_FAILURE_BIT_EXT",
                             caller_name, index);
        }
    }
    return skip;
}

bool CoreChecks::ValidatePipelineProtectedAccessFlags(VkPipelineCreateFlags flags, uint32_t index) const {
    bool skip = false;
    if (enabled_features.pipeline_protected_access_features.pipelineProtectedAccess == VK_FALSE) {
        const VkPipelineCreateFlags invalid_flags =
            VK_PIPELINE_CREATE_NO_PROTECTED_ACCESS_BIT_EXT | VK_PIPELINE_CREATE_PROTECTED_ACCESS_ONLY_BIT_EXT;
        if ((flags & invalid_flags) != 0) {
            skip |= LogError(
                device, "VUID-VkGraphicsPipelineCreateInfo-pipelineProtectedAccess-07368",
                "vkCreateGraphicsPipelines(): pipelineProtectedAccess is turned off but pCreateInfos[%" PRIu32
                "] has VkPipelineCreateFlags "
                "(%s) "
                "that contain VK_PIPELINE_CREATE_NO_PROTECTED_ACCESS_BIT_EXT or VK_PIPELINE_CREATE_PROTECTED_ACCESS_ONLY_BIT_EXT",
                index, string_VkPipelineCreateFlags(flags).c_str());
        }
    }
    if ((flags & VK_PIPELINE_CREATE_NO_PROTECTED_ACCESS_BIT_EXT) && (flags & VK_PIPELINE_CREATE_PROTECTED_ACCESS_ONLY_BIT_EXT)) {
        skip |= LogError(
            device, "VUID-VkGraphicsPipelineCreateInfo-flags-07369",
            "vkCreateGraphicsPipelines(): pCreateInfos[%" PRIu32
            "] has VkPipelineCreateFlags that "
            "contains both VK_PIPELINE_CREATE_NO_PROTECTED_ACCESS_BIT_EXT and VK_PIPELINE_CREATE_PROTECTED_ACCESS_ONLY_BIT_EXT",
            index);
    }
    return skip;
}

bool CoreChecks::PreCallValidateCreatePipelineCache(VkDevice device, const VkPipelineCacheCreateInfo *pCreateInfo,
                                                    const VkAllocationCallbacks *pAllocator,
                                                    VkPipelineCache *pPipelineCache) const {
    bool skip = false;
    if (enabled_features.core13.pipelineCreationCacheControl == VK_FALSE) {
        if ((pCreateInfo->flags & VK_PIPELINE_CACHE_CREATE_EXTERNALLY_SYNCHRONIZED_BIT_EXT) != 0) {
            skip |= LogError(device, "VUID-VkPipelineCacheCreateInfo-pipelineCreationCacheControl-02892",
                             "vkCreatePipelineCache(): pipelineCreationCacheControl is turned off but pCreateInfo::flags contains "
                             "VK_PIPELINE_CACHE_CREATE_EXTERNALLY_SYNCHRONIZED_BIT_EXT");
        }
    }
    return skip;
}

// This can be chained in the vkCreate*Pipelines() function or the VkPipelineShaderStageCreateInfo
bool CoreChecks::ValidatePipelineRobustnessCreateInfo(const PIPELINE_STATE &pipeline, const char *parameter_name,
                                                      const VkPipelineRobustnessCreateInfoEXT &create_info) const {
    bool skip = false;

    if (!enabled_features.pipeline_robustness_features.pipelineRobustness) {
        if (create_info.storageBuffers != VK_PIPELINE_ROBUSTNESS_BUFFER_BEHAVIOR_DEVICE_DEFAULT_EXT) {
            skip |= LogError(pipeline.pipeline(), "VUID-VkPipelineRobustnessCreateInfoEXT-pipelineRobustness-06926",
                             "%s "
                             "has VkPipelineRobustnessCreateInfoEXT::storageBuffers == %s "
                             "but the pipelineRobustness feature is not enabled.",
                             parameter_name, string_VkPipelineRobustnessBufferBehaviorEXT(create_info.storageBuffers));
        }
        if (create_info.uniformBuffers != VK_PIPELINE_ROBUSTNESS_BUFFER_BEHAVIOR_DEVICE_DEFAULT_EXT) {
            skip |= LogError(pipeline.pipeline(), "VUID-VkPipelineRobustnessCreateInfoEXT-pipelineRobustness-06927",
                             "%s "
                             "has VkPipelineRobustnessCreateInfoEXT::uniformBuffers == %s "
                             "but the pipelineRobustness feature is not enabled.",
                             parameter_name, string_VkPipelineRobustnessBufferBehaviorEXT(create_info.uniformBuffers));
        }
        if (create_info.vertexInputs != VK_PIPELINE_ROBUSTNESS_BUFFER_BEHAVIOR_DEVICE_DEFAULT_EXT) {
            skip |= LogError(pipeline.pipeline(), "VUID-VkPipelineRobustnessCreateInfoEXT-pipelineRobustness-06928",
                             "%s "
                             "has VkPipelineRobustnessCreateInfoEXT::vertexInputs == %s "
                             "but the pipelineRobustness feature is not enabled.",
                             parameter_name, string_VkPipelineRobustnessBufferBehaviorEXT(create_info.vertexInputs));
        }
        if (create_info.images != VK_PIPELINE_ROBUSTNESS_IMAGE_BEHAVIOR_DEVICE_DEFAULT_EXT) {
            skip |= LogError(pipeline.pipeline(), "VUID-VkPipelineRobustnessCreateInfoEXT-pipelineRobustness-06929",
                             "%s "
                             "has VkPipelineRobustnessCreateInfoEXT::images == %s "
                             "but the pipelineRobustness feature is not enabled.",
                             parameter_name, string_VkPipelineRobustnessImageBehaviorEXT(create_info.images));
        }
    }

    // These validation depend if the features are exposed (not just enabled)
    if (!has_robust_image_access && create_info.images == VK_PIPELINE_ROBUSTNESS_IMAGE_BEHAVIOR_ROBUST_IMAGE_ACCESS_EXT) {
        skip |= LogError(pipeline.pipeline(), "VUID-VkPipelineRobustnessCreateInfoEXT-robustImageAccess-06930",
                         "%s "
                         "has VkPipelineRobustnessCreateInfoEXT::images == "
                         "VK_PIPELINE_ROBUSTNESS_IMAGE_BEHAVIOR_ROBUST_IMAGE_ACCESS_EXT "
                         "but robustImageAccess2 is not supported.",
                         parameter_name);
    }
    return skip;
}

bool CoreChecks::PreCallValidateGetPipelineExecutablePropertiesKHR(VkDevice device, const VkPipelineInfoKHR *pPipelineInfo,
                                                                   uint32_t *pExecutableCount,
                                                                   VkPipelineExecutablePropertiesKHR *pProperties) const {
    bool skip = false;
    skip |= ValidatePipelineExecutableInfo(device, nullptr, "vkGetPipelineExecutablePropertiesKHR",
                                           "VUID-vkGetPipelineExecutablePropertiesKHR-pipelineExecutableInfo-03270");
    return skip;
}

bool CoreChecks::ValidatePipelineExecutableInfo(VkDevice device, const VkPipelineExecutableInfoKHR *pExecutableInfo,
                                                const char *caller_name, const char *feature_vuid) const {
    bool skip = false;

    if (!enabled_features.pipeline_exe_props_features.pipelineExecutableInfo) {
        skip |= LogError(device, feature_vuid, "%s(): called when pipelineExecutableInfo feature is not enabled.", caller_name);
    }

    // vkGetPipelineExecutablePropertiesKHR will not have struct to validate further
    if (pExecutableInfo) {
        auto pi = LvlInitStruct<VkPipelineInfoKHR>();
        pi.pipeline = pExecutableInfo->pipeline;

        // We could probably cache this instead of fetching it every time
        uint32_t executable_count = 0;
        DispatchGetPipelineExecutablePropertiesKHR(device, &pi, &executable_count, NULL);

        if (pExecutableInfo->executableIndex >= executable_count) {
            skip |= LogError(
                pExecutableInfo->pipeline, "VUID-VkPipelineExecutableInfoKHR-executableIndex-03275",
                "%s(): VkPipelineExecutableInfo::executableIndex (%1u) must be less than the number of executables associated with "
                "the pipeline (%1u) as returned by vkGetPipelineExecutablePropertiessKHR",
                caller_name, pExecutableInfo->executableIndex, executable_count);
        }
    }

    return skip;
}

bool CoreChecks::PreCallValidateGetPipelineExecutableStatisticsKHR(VkDevice device,
                                                                   const VkPipelineExecutableInfoKHR *pExecutableInfo,
                                                                   uint32_t *pStatisticCount,
                                                                   VkPipelineExecutableStatisticKHR *pStatistics) const {
    bool skip = false;
    skip |= ValidatePipelineExecutableInfo(device, pExecutableInfo, "vkGetPipelineExecutableStatisticsKHR",
                                           "VUID-vkGetPipelineExecutableStatisticsKHR-pipelineExecutableInfo-03272");

    auto pipeline_state = Get<PIPELINE_STATE>(pExecutableInfo->pipeline);
    if (!(pipeline_state->create_flags & VK_PIPELINE_CREATE_CAPTURE_STATISTICS_BIT_KHR)) {
        skip |= LogError(pExecutableInfo->pipeline, "VUID-vkGetPipelineExecutableStatisticsKHR-pipeline-03274",
                         "vkGetPipelineExecutableStatisticsKHR called on a pipeline created without the "
                         "VK_PIPELINE_CREATE_CAPTURE_STATISTICS_BIT_KHR flag set");
    }

    return skip;
}

bool CoreChecks::PreCallValidateGetPipelineExecutableInternalRepresentationsKHR(
    VkDevice device, const VkPipelineExecutableInfoKHR *pExecutableInfo, uint32_t *pInternalRepresentationCount,
    VkPipelineExecutableInternalRepresentationKHR *pStatistics) const {
    bool skip = false;
    skip |= ValidatePipelineExecutableInfo(device, pExecutableInfo, "vkGetPipelineExecutableInternalRepresentationsKHR",
                                           "VUID-vkGetPipelineExecutableInternalRepresentationsKHR-pipelineExecutableInfo-03276");

    auto pipeline_state = Get<PIPELINE_STATE>(pExecutableInfo->pipeline);
    if (!(pipeline_state->create_flags & VK_PIPELINE_CREATE_CAPTURE_INTERNAL_REPRESENTATIONS_BIT_KHR)) {
        skip |= LogError(pExecutableInfo->pipeline, "VUID-vkGetPipelineExecutableInternalRepresentationsKHR-pipeline-03278",
                         "vkGetPipelineExecutableInternalRepresentationsKHR called on a pipeline created without the "
                         "VK_PIPELINE_CREATE_CAPTURE_INTERNAL_REPRESENTATIONS_BIT_KHR flag set");
    }

    return skip;
}

bool CoreChecks::PreCallValidateDestroyPipeline(VkDevice device, VkPipeline pipeline,
                                                const VkAllocationCallbacks *pAllocator) const {
    auto pipeline_state = Get<PIPELINE_STATE>(pipeline);
    bool skip = false;
    if (pipeline_state) {
        skip |= ValidateObjectNotInUse(pipeline_state.get(), "vkDestroyPipeline", "VUID-vkDestroyPipeline-pipeline-00765");
    }
    return skip;
}

bool CoreChecks::PreCallValidateCmdBindPipeline(VkCommandBuffer commandBuffer, VkPipelineBindPoint pipelineBindPoint,
                                                VkPipeline pipeline) const {
    auto cb_state = GetRead<CMD_BUFFER_STATE>(commandBuffer);
    assert(cb_state);

    bool skip = false;
    skip |= ValidateCmd(*cb_state, CMD_BINDPIPELINE);
    static const std::map<VkPipelineBindPoint, std::string> bindpoint_errors = {
        std::make_pair(VK_PIPELINE_BIND_POINT_GRAPHICS, "VUID-vkCmdBindPipeline-pipelineBindPoint-00777"),
        std::make_pair(VK_PIPELINE_BIND_POINT_COMPUTE, "VUID-vkCmdBindPipeline-pipelineBindPoint-00778"),
        std::make_pair(VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR, "VUID-vkCmdBindPipeline-pipelineBindPoint-02391")};

    skip |= ValidatePipelineBindPoint(cb_state.get(), pipelineBindPoint, "vkCmdBindPipeline()", bindpoint_errors);

    auto pPipeline = Get<PIPELINE_STATE>(pipeline);
    assert(pPipeline);
    const PIPELINE_STATE &pipeline_state = *pPipeline;

    if (pipelineBindPoint != pipeline_state.pipeline_type) {
        if (pipelineBindPoint == VK_PIPELINE_BIND_POINT_GRAPHICS) {
            const LogObjectList objlist(cb_state->commandBuffer(), pipeline);
            skip |= LogError(objlist, "VUID-vkCmdBindPipeline-pipelineBindPoint-00779",
                             "Cannot bind a pipeline of type %s to the graphics pipeline bind point",
                             string_VkPipelineBindPoint(pipeline_state.pipeline_type));
        } else if (pipelineBindPoint == VK_PIPELINE_BIND_POINT_COMPUTE) {
            const LogObjectList objlist(cb_state->commandBuffer(), pipeline);
            skip |= LogError(objlist, "VUID-vkCmdBindPipeline-pipelineBindPoint-00780",
                             "Cannot bind a pipeline of type %s to the compute pipeline bind point",
                             string_VkPipelineBindPoint(pipeline_state.pipeline_type));
        } else if (pipelineBindPoint == VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR) {
            const LogObjectList objlist(cb_state->commandBuffer(), pipeline);
            skip |= LogError(objlist, "VUID-vkCmdBindPipeline-pipelineBindPoint-02392",
                             "Cannot bind a pipeline of type %s to the ray-tracing pipeline bind point",
                             string_VkPipelineBindPoint(pipeline_state.pipeline_type));
        }
    } else {
        if (pipelineBindPoint == VK_PIPELINE_BIND_POINT_GRAPHICS) {
            skip |= ValidateGraphicsPipelineBindPoint(cb_state.get(), pipeline_state);

            if (cb_state->activeRenderPass &&
                phys_dev_ext_props.provoking_vertex_props.provokingVertexModePerPipeline == VK_FALSE) {
                const auto lvl_bind_point = ConvertToLvlBindPoint(pipelineBindPoint);
                const auto &last_bound = cb_state->lastBound[lvl_bind_point];
                if (last_bound.pipeline_state) {
                    auto last_bound_provoking_vertex_state_ci =
                        LvlFindInChain<VkPipelineRasterizationProvokingVertexStateCreateInfoEXT>(
                            last_bound.pipeline_state->RasterizationState()->pNext);

                    auto current_provoking_vertex_state_ci =
                        LvlFindInChain<VkPipelineRasterizationProvokingVertexStateCreateInfoEXT>(
                            pipeline_state.RasterizationState()->pNext);

                    if (last_bound_provoking_vertex_state_ci && !current_provoking_vertex_state_ci) {
                        const LogObjectList objlist(cb_state->commandBuffer(), pipeline);
                        skip |= LogError(objlist, "VUID-vkCmdBindPipeline-pipelineBindPoint-04881",
                                         "Previous %s's provokingVertexMode is %s, but %s doesn't chain "
                                         "VkPipelineRasterizationProvokingVertexStateCreateInfoEXT.",
                                         report_data->FormatHandle(last_bound.pipeline_state->pipeline()).c_str(),
                                         string_VkProvokingVertexModeEXT(last_bound_provoking_vertex_state_ci->provokingVertexMode),
                                         report_data->FormatHandle(pipeline).c_str());
                    } else if (!last_bound_provoking_vertex_state_ci && current_provoking_vertex_state_ci) {
                        const LogObjectList objlist(cb_state->commandBuffer(), pipeline);
                        skip |= LogError(objlist, "VUID-vkCmdBindPipeline-pipelineBindPoint-04881",
                                         " %s's provokingVertexMode is %s, but previous %s doesn't chain "
                                         "VkPipelineRasterizationProvokingVertexStateCreateInfoEXT.",
                                         report_data->FormatHandle(pipeline).c_str(),
                                         string_VkProvokingVertexModeEXT(current_provoking_vertex_state_ci->provokingVertexMode),
                                         report_data->FormatHandle(last_bound.pipeline_state->pipeline()).c_str());
                    } else if (last_bound_provoking_vertex_state_ci && current_provoking_vertex_state_ci &&
                               last_bound_provoking_vertex_state_ci->provokingVertexMode !=
                                   current_provoking_vertex_state_ci->provokingVertexMode) {
                        const LogObjectList objlist(cb_state->commandBuffer(), pipeline);
                        skip |=
                            LogError(objlist, "VUID-vkCmdBindPipeline-pipelineBindPoint-04881",
                                     "%s's provokingVertexMode is %s, but previous %s's provokingVertexMode is %s.",
                                     report_data->FormatHandle(pipeline).c_str(),
                                     string_VkProvokingVertexModeEXT(current_provoking_vertex_state_ci->provokingVertexMode),
                                     report_data->FormatHandle(last_bound.pipeline_state->pipeline()).c_str(),
                                     string_VkProvokingVertexModeEXT(last_bound_provoking_vertex_state_ci->provokingVertexMode));
                    }
                }
            }

            if (cb_state->activeRenderPass && phys_dev_ext_props.sample_locations_props.variableSampleLocations == VK_FALSE) {
                const auto *multisample_state = pipeline_state.MultisampleState();
                const auto *sample_locations = LvlFindInChain<VkPipelineSampleLocationsStateCreateInfoEXT>(multisample_state);
                if (sample_locations && sample_locations->sampleLocationsEnable == VK_TRUE &&
                    !pipeline_state.IsDynamic(VK_DYNAMIC_STATE_SAMPLE_LOCATIONS_EXT)) {
                    const VkRenderPassSampleLocationsBeginInfoEXT *sample_locations_begin_info =
                        LvlFindInChain<VkRenderPassSampleLocationsBeginInfoEXT>(cb_state->activeRenderPassBeginInfo.pNext);
                    bool found = false;
                    if (sample_locations_begin_info) {
                        for (uint32_t i = 0; i < sample_locations_begin_info->postSubpassSampleLocationsCount; ++i) {
                            if (sample_locations_begin_info->pPostSubpassSampleLocations[i].subpassIndex ==
                                cb_state->GetActiveSubpass()) {
                                if (MatchSampleLocationsInfo(
                                        &sample_locations_begin_info->pPostSubpassSampleLocations[i].sampleLocationsInfo,
                                        &sample_locations->sampleLocationsInfo)) {
                                    found = true;
                                }
                            }
                        }
                    }
                    if (!found) {
                        const LogObjectList objlist(cb_state->commandBuffer(), pipeline);
                        skip |=
                            LogError(objlist, "VUID-vkCmdBindPipeline-variableSampleLocations-01525",
                                     "vkCmdBindPipeline(): VkPhysicalDeviceSampleLocationsPropertiesEXT::variableSampleLocations "
                                     "is false, pipeline is a graphics pipeline with "
                                     "VkPipelineSampleLocationsStateCreateInfoEXT::sampleLocationsEnable equal to true and without "
                                     "VK_DYNAMIC_STATE_SAMPLE_LOCATIONS_EXT, but the current render pass (%" PRIu32
                                     ") was not begun with any element of "
                                     "VkRenderPassSampleLocationsBeginInfoEXT::pPostSubpassSampleLocations subpassIndex "
                                     "matching the current subpass index and sampleLocationsInfo matching sampleLocationsInfo of "
                                     "VkPipelineSampleLocationsStateCreateInfoEXT the pipeline was created with.",
                                     cb_state->GetActiveSubpass());
                    }
                }
            }

            if (enabled_features.core.variableMultisampleRate == VK_FALSE) {
                if (const auto *multisample_state = pipeline_state.MultisampleState(); multisample_state) {
                    if (const auto &render_pass = cb_state->activeRenderPass; render_pass) {
                        const uint32_t subpass = cb_state->GetActiveSubpass();
                        // if render pass uses no attachment, verify that all bound pipelines referencing this subpass have the same
                        // pMultisampleState->rasterizationSamples.
                        if (!render_pass->UsesDynamicRendering() && !render_pass->UsesColorAttachment(subpass) &&
                            !render_pass->UsesDepthStencilAttachment(subpass)) {
                            // If execution ends up here, GetActiveSubpassRasterizationSampleCount() can still be empty if this is
                            // the first bound pipeline with the previous conditions holding. Rasterization samples count for the
                            // subpass will be updated in PostCallRecordCmdBindPipeline, if it is empty.
                            if (std::optional<VkSampleCountFlagBits> subpass_rasterization_samples =
                                    cb_state->GetActiveSubpassRasterizationSampleCount();
                                subpass_rasterization_samples &&
                                *subpass_rasterization_samples != multisample_state->rasterizationSamples) {
                                const LogObjectList objlist(device, render_pass->Handle(), pipeline_state.Handle());
                                skip |= LogError(
                                    objlist, "VUID-VkGraphicsPipelineCreateInfo-subpass-00758",
                                    "vkCreateGraphicsPipelines(): VkPhysicalDeviceFeatures::variableMultisampleRate is VK_FALSE "
                                    "and "
                                    "pipeline has pMultisampleState->rasterizationSamples equal to %s, while a previously bound "
                                    "pipeline in the current subpass (%" PRIu32
                                    ") used "
                                    "pMultisampleState->rasterizationSamples equal to %s.",
                                    string_VkSampleCountFlagBits(multisample_state->rasterizationSamples), subpass,
                                    string_VkSampleCountFlagBits(*subpass_rasterization_samples));
                            }
                        }
                    }
                }
            }
        }
        if (pipeline_state.create_flags & VK_PIPELINE_CREATE_LIBRARY_BIT_KHR) {
            const LogObjectList objlist(cb_state->commandBuffer(), pipeline);
            skip |= LogError(
                objlist, "VUID-vkCmdBindPipeline-pipeline-03382",
                "vkCmdBindPipeline(): Cannot bind a pipeline that was created with the VK_PIPELINE_CREATE_LIBRARY_BIT_KHR flag.");
        }
        if (cb_state->transform_feedback_active) {
            const LogObjectList objlist(cb_state->commandBuffer(), pipeline);
            skip |= LogError(objlist, "VUID-vkCmdBindPipeline-None-02323", "vkCmdBindPipeline(): transform feedback is active.");
        }
        if (enabled_features.pipeline_protected_access_features.pipelineProtectedAccess) {
            if (cb_state->unprotected) {
                const LogObjectList objlist(cb_state->commandBuffer(), pipeline);
                if (pipeline_state.create_flags & VK_PIPELINE_CREATE_PROTECTED_ACCESS_ONLY_BIT_EXT) {
                    skip |= LogError(objlist, "VUID-vkCmdBindPipeline-pipelineProtectedAccess-07409",
                                     "vkCmdBindPipeline(): Binding pipeline created with "
                                     "VK_PIPELINE_CREATE_PROTECTED_ACCESS_ONLY_BIT_EXT in an unprotected command buffer");
                }
            } else {
                const LogObjectList objlist(cb_state->commandBuffer(), pipeline);
                if (pipeline_state.create_flags & VK_PIPELINE_CREATE_NO_PROTECTED_ACCESS_BIT_EXT) {
                    skip |= LogError(objlist, "VUID-vkCmdBindPipeline-pipelineProtectedAccess-07408",
                                     "vkCmdBindPipeline(): Binding pipeline created with "
                                     "VK_PIPELINE_CREATE_NO_PROTECTED_ACCESS_BIT_EXT in a protected command buffer");
                }
            }
        }
    }

    return skip;
}

// Validates that the supplied bind point is supported for the command buffer (vis. the command pool)
// Takes array of error codes as some of the VUID's (e.g. vkCmdBindPipeline) are written per bindpoint
// TODO add vkCmdBindPipeline bind_point validation using this call.
bool CoreChecks::ValidatePipelineBindPoint(const CMD_BUFFER_STATE *cb_state, VkPipelineBindPoint bind_point, const char *func_name,
                                           const std::map<VkPipelineBindPoint, std::string> &bind_errors) const {
    bool skip = false;
    auto pool = cb_state->command_pool;
    if (pool) {  // The loss of a pool in a recording cmd is reported in DestroyCommandPool
        static const std::map<VkPipelineBindPoint, VkQueueFlags> flag_mask = {
            std::make_pair(VK_PIPELINE_BIND_POINT_GRAPHICS, static_cast<VkQueueFlags>(VK_QUEUE_GRAPHICS_BIT)),
            std::make_pair(VK_PIPELINE_BIND_POINT_COMPUTE, static_cast<VkQueueFlags>(VK_QUEUE_COMPUTE_BIT)),
            std::make_pair(VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR,
                           static_cast<VkQueueFlags>(VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_COMPUTE_BIT)),
        };
        const auto &qfp = physical_device_state->queue_family_properties[pool->queueFamilyIndex];
        if (0 == (qfp.queueFlags & flag_mask.at(bind_point))) {
            const std::string &error = bind_errors.at(bind_point);
            const LogObjectList objlist(cb_state->commandBuffer(), cb_state->createInfo.commandPool);
            skip |= LogError(objlist, error, "%s: %s was allocated from %s that does not support bindpoint %s.", func_name,
                             report_data->FormatHandle(cb_state->commandBuffer()).c_str(),
                             report_data->FormatHandle(cb_state->createInfo.commandPool).c_str(),
                             string_VkPipelineBindPoint(bind_point));
        }
    }
    return skip;
}
