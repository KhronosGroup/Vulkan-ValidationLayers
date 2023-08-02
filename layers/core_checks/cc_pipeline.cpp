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
                                         FormatHandle(last_bound.pipeline_state->pipeline()).c_str(),
                                         string_VkProvokingVertexModeEXT(last_bound_provoking_vertex_state_ci->provokingVertexMode),
                                         FormatHandle(pipeline).c_str());
                    } else if (!last_bound_provoking_vertex_state_ci && current_provoking_vertex_state_ci) {
                        const LogObjectList objlist(cb_state->commandBuffer(), pipeline);
                        skip |= LogError(objlist, "VUID-vkCmdBindPipeline-pipelineBindPoint-04881",
                                         " %s's provokingVertexMode is %s, but previous %s doesn't chain "
                                         "VkPipelineRasterizationProvokingVertexStateCreateInfoEXT.",
                                         FormatHandle(pipeline).c_str(),
                                         string_VkProvokingVertexModeEXT(current_provoking_vertex_state_ci->provokingVertexMode),
                                         FormatHandle(last_bound.pipeline_state->pipeline()).c_str());
                    } else if (last_bound_provoking_vertex_state_ci && current_provoking_vertex_state_ci &&
                               last_bound_provoking_vertex_state_ci->provokingVertexMode !=
                                   current_provoking_vertex_state_ci->provokingVertexMode) {
                        const LogObjectList objlist(cb_state->commandBuffer(), pipeline);
                        skip |=
                            LogError(objlist, "VUID-vkCmdBindPipeline-pipelineBindPoint-04881",
                                     "%s's provokingVertexMode is %s, but previous %s's provokingVertexMode is %s.",
                                     FormatHandle(pipeline).c_str(),
                                     string_VkProvokingVertexModeEXT(current_provoking_vertex_state_ci->provokingVertexMode),
                                     FormatHandle(last_bound.pipeline_state->pipeline()).c_str(),
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
                        LvlFindInChain<VkRenderPassSampleLocationsBeginInfoEXT>(cb_state->active_render_pass_begin_info.pNext);
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
                             FormatHandle(cb_state->commandBuffer()).c_str(),
                             FormatHandle(cb_state->createInfo.commandPool).c_str(), string_VkPipelineBindPoint(bind_point));
        }
    }
    return skip;
}

bool CoreChecks::ValidateShaderSubgroupSizeControl(const PIPELINE_STATE &pipeline, VkShaderStageFlagBits stage,
                                                   VkPipelineShaderStageCreateFlags flags) const {
    bool skip = false;

    if ((flags & VK_PIPELINE_SHADER_STAGE_CREATE_ALLOW_VARYING_SUBGROUP_SIZE_BIT_EXT) != 0 &&
        !enabled_features.core13.subgroupSizeControl) {
        skip |= LogError(
            device, "VUID-VkPipelineShaderStageCreateInfo-flags-02784",
            "%s(): pCreateInfos[%" PRIu32
            "] VkPipelineShaderStageCreateInfo flags contain VK_PIPELINE_SHADER_STAGE_CREATE_ALLOW_VARYING_SUBGROUP_SIZE_BIT_EXT, "
            "but the subgroupSizeControl feature is not enabled.",
            pipeline.GetCreateFunctionName(), pipeline.create_index);
    }

    if ((flags & VK_PIPELINE_SHADER_STAGE_CREATE_REQUIRE_FULL_SUBGROUPS_BIT_EXT) != 0) {
        if (!enabled_features.core13.computeFullSubgroups) {
            skip |= LogError(device, "VUID-VkPipelineShaderStageCreateInfo-flags-02785",
                             "%s(): pCreateInfos[%" PRIu32
                             "] VkPipelineShaderStageCreateInfo flags contain "
                             "VK_PIPELINE_SHADER_STAGE_CREATE_REQUIRE_FULL_SUBGROUPS_BIT_EXT, but the computeFullSubgroups feature "
                             "is not enabled",
                             pipeline.GetCreateFunctionName(), pipeline.create_index);
        } else if ((stage & (VK_SHADER_STAGE_MESH_BIT_EXT | VK_SHADER_STAGE_TASK_BIT_EXT | VK_SHADER_STAGE_COMPUTE_BIT)) == 0) {
            skip |= LogError(device, "VUID-VkPipelineShaderStageCreateInfo-flags-08988",
                             "%s(): pCreateInfos[%" PRIu32
                             "] VkPipelineShaderStageCreateInfo flags contain "
                             "VK_PIPELINE_SHADER_STAGE_CREATE_REQUIRE_FULL_SUBGROUPS_BIT_EXT, but the stage is %s.",
                             pipeline.GetCreateFunctionName(), pipeline.create_index, string_VkShaderStageFlagBits(stage));
        }
    }

    return skip;
}

// Validate that data for each specialization entry is fully contained within the buffer.
bool CoreChecks::ValidateSpecializations(const safe_VkSpecializationInfo *spec, const PIPELINE_STATE &pipeline) const {
    bool skip = false;
    if (spec) {
        for (auto i = 0u; i < spec->mapEntryCount; i++) {
            if (spec->pMapEntries[i].offset >= spec->dataSize) {
                skip |= LogError(device, "VUID-VkSpecializationInfo-offset-00773",
                                 "%s(): pCreateInfos[%" PRIu32
                                 "] Specialization entry %u (for constant id %u) references memory outside provided specialization "
                                 "data (bytes %u..%zu; %zu bytes provided).",
                                 pipeline.GetCreateFunctionName(), pipeline.create_index, i, spec->pMapEntries[i].constantID,
                                 spec->pMapEntries[i].offset, spec->pMapEntries[i].offset + spec->dataSize - 1, spec->dataSize);

                continue;
            }
            if (spec->pMapEntries[i].offset + spec->pMapEntries[i].size > spec->dataSize) {
                skip |= LogError(device, "VUID-VkSpecializationInfo-pMapEntries-00774",
                                 "%s(): pCreateInfos[%" PRIu32
                                 "] Specialization entry %u (for constant id %u) references memory outside provided specialization "
                                 "data (bytes %u..%zu; %zu bytes provided).",
                                 pipeline.GetCreateFunctionName(), pipeline.create_index, i, spec->pMapEntries[i].constantID,
                                 spec->pMapEntries[i].offset, spec->pMapEntries[i].offset + spec->pMapEntries[i].size - 1,
                                 spec->dataSize);
            }
            for (uint32_t j = i + 1; j < spec->mapEntryCount; ++j) {
                if (spec->pMapEntries[i].constantID == spec->pMapEntries[j].constantID) {
                    skip |=
                        LogError(device, "VUID-VkSpecializationInfo-constantID-04911",
                                 "%s(): pCreateInfos[%" PRIu32 "] Specialization entry %" PRIu32 " and %" PRIu32
                                 " have the same constantID (%" PRIu32 ").",
                                 pipeline.GetCreateFunctionName(), pipeline.create_index, i, j, spec->pMapEntries[i].constantID);
                }
            }
        }
    }

    return skip;
}

bool CoreChecks::ValidateShaderStageMaxResources(VkShaderStageFlagBits stage, const PIPELINE_STATE &pipeline) const {
    bool skip = false;
    uint32_t total_resources = 0;

    const auto &rp_state = pipeline.RenderPassState();
    if ((stage == VK_SHADER_STAGE_FRAGMENT_BIT) && rp_state) {
        if (rp_state->UsesDynamicRendering()) {
            total_resources += rp_state->dynamic_rendering_pipeline_create_info.colorAttachmentCount;
        } else {
            // "For the fragment shader stage the framebuffer color attachments also count against this limit"
            total_resources += rp_state->createInfo.pSubpasses[pipeline.Subpass()].colorAttachmentCount;
        }
    }

    // TODO: This reuses a lot of GetDescriptorCountMaxPerStage but currently would need to make it agnostic in a way to handle
    // input from CreatePipeline and CreatePipelineLayout level
    const auto &layout_state = pipeline.PipelineLayoutState();
    if (layout_state) {
        for (const auto &set_layout : layout_state->set_layouts) {
            if (!set_layout) {
                continue;
            }

            if ((set_layout->GetCreateFlags() & VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT) != 0) {
                continue;
            }

            for (uint32_t binding_idx = 0; binding_idx < set_layout->GetBindingCount(); binding_idx++) {
                const VkDescriptorSetLayoutBinding *binding = set_layout->GetDescriptorSetLayoutBindingPtrFromIndex(binding_idx);
                // Bindings with a descriptorCount of 0 are "reserved" and should be skipped
                if (((stage & binding->stageFlags) != 0) && (binding->descriptorCount > 0)) {
                    // Check only descriptor types listed in maxPerStageResources description in spec
                    switch (binding->descriptorType) {
                        case VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER:
                        case VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE:
                        case VK_DESCRIPTOR_TYPE_STORAGE_IMAGE:
                        case VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER:
                        case VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER:
                        case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER:
                        case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER:
                        case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC:
                        case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC:
                        case VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT:
                            total_resources += binding->descriptorCount;
                            break;
                        default:
                            break;
                    }
                }
            }
        }
    }

    if (total_resources > phys_dev_props.limits.maxPerStageResources) {
        const char *vuid = nullptr;
        if (stage == VK_SHADER_STAGE_COMPUTE_BIT) {
            vuid = "VUID-VkComputePipelineCreateInfo-layout-01687";
        } else if ((stage & VK_SHADER_STAGE_ALL_GRAPHICS) == 0) {
            vuid = "VUID-VkRayTracingPipelineCreateInfoKHR-layout-03428";
        } else {
            vuid = "VUID-VkGraphicsPipelineCreateInfo-layout-01688";
        }
        skip |= LogError(device, vuid,
                         "%s(): pCreateInfos[%" PRIu32
                         "] Shader Stage %s exceeds component limit "
                         "VkPhysicalDeviceLimits::maxPerStageResources (%" PRIu32 ")",
                         pipeline.GetCreateFunctionName(), pipeline.create_index, string_VkShaderStageFlagBits(stage),
                         phys_dev_props.limits.maxPerStageResources);
    }

    return skip;
}

bool CoreChecks::ValidateShaderModuleId(const PIPELINE_STATE &pipeline) const {
    bool skip = false;
    for (const auto &stage_ci : pipeline.shader_stages_ci) {
        const auto module_identifier = LvlFindInChain<VkPipelineShaderStageModuleIdentifierCreateInfoEXT>(stage_ci.pNext);
        const auto module_create_info = LvlFindInChain<VkShaderModuleCreateInfo>(stage_ci.pNext);
        if (module_identifier) {
            if (module_identifier->identifierSize > 0) {
                if (!(enabled_features.shader_module_identifier_features.shaderModuleIdentifier)) {
                    skip |= LogError(device, "VUID-VkPipelineShaderStageModuleIdentifierCreateInfoEXT-pNext-06850",
                                     "%s pCreateInfos[%" PRIu32
                                     "] module (stage %s) VkPipelineShaderStageCreateInfo has a "
                                     "VkPipelineShaderStageModuleIdentifierCreateInfoEXT "
                                     "struct in the pNext chain but the shaderModuleIdentifier feature is not enabled",
                                     pipeline.GetCreateFunctionName(), pipeline.create_index,
                                     string_VkShaderStageFlagBits(stage_ci.stage));
                }
                if (!(pipeline.create_flags & VK_PIPELINE_CREATE_FAIL_ON_PIPELINE_COMPILE_REQUIRED_BIT)) {
                    skip |= LogError(
                        device, "VUID-VkPipelineShaderStageModuleIdentifierCreateInfoEXT-pNext-06851",
                        "%s pCreateInfos[%" PRIu32
                        "] module (stage %s) VkPipelineShaderStageCreateInfo has a "
                        "VkPipelineShaderStageModuleIdentifierCreateInfoEXT "
                        "struct in the pNext chain whose identifierSize is > 0 (%" PRIu32
                        "), but the "
                        "VK_PIPELINE_CREATE_FAIL_ON_PIPELINE_COMPILE_REQUIRED_BIT bit is not set in the pipeline create flags",
                        pipeline.GetCreateFunctionName(), pipeline.create_index, string_VkShaderStageFlagBits(stage_ci.stage),
                        module_identifier->identifierSize);
                }
                if (module_identifier->identifierSize > VK_MAX_SHADER_MODULE_IDENTIFIER_SIZE_EXT) {
                    skip |= LogError(device, "VUID-VkPipelineShaderStageModuleIdentifierCreateInfoEXT-identifierSize-06852",
                                     "%s pCreateInfos[%" PRIu32
                                     "] module (stage %s) VkPipelineShaderStageCreateInfo has a "
                                     "VkPipelineShaderStageModuleIdentifierCreateInfoEXT "
                                     "struct in the pNext chain whose identifierSize (%" PRIu32
                                     ") is > VK_MAX_SHADER_MODULE_IDENTIFIER_SIZE_EXT (%" PRIu32 ")",
                                     pipeline.GetCreateFunctionName(), pipeline.create_index,
                                     string_VkShaderStageFlagBits(stage_ci.stage), module_identifier->identifierSize,
                                     VK_MAX_SHADER_MODULE_IDENTIFIER_SIZE_EXT);
                }
            }
            if (module_create_info) {
                skip |=
                    LogError(device, "VUID-VkPipelineShaderStageCreateInfo-stage-06844",
                             "%s pCreateInfos[%" PRIu32
                             "] module (stage %s) VkPipelineShaderStageCreateInfo has both a "
                             "VkPipelineShaderStageModuleIdentifierCreateInfoEXT "
                             "struct and a VkShaderModuleCreateInfo struct in the pNext chain",
                             pipeline.GetCreateFunctionName(), pipeline.create_index, string_VkShaderStageFlagBits(stage_ci.stage));
            }
            if (stage_ci.module != VK_NULL_HANDLE) {
                skip |= LogError(
                    device, "VUID-VkPipelineShaderStageCreateInfo-stage-06848",
                    "%s pCreateInfos[%" PRIu32
                    "] module (stage %s) VkPipelineShaderStageCreateInfo has a VkPipelineShaderStageModuleIdentifierCreateInfoEXT "
                    "struct in the pNext chain, and module is not VK_NULL_HANDLE",
                    pipeline.GetCreateFunctionName(), pipeline.create_index, string_VkShaderStageFlagBits(stage_ci.stage));
            }
        } else {
            if (enabled_features.graphics_pipeline_library_features.graphicsPipelineLibrary) {
                if (stage_ci.module == VK_NULL_HANDLE && !module_create_info) {
                    skip |= LogError(device, "VUID-VkPipelineShaderStageCreateInfo-stage-06845",
                                     "%s pCreateInfos[%" PRIu32
                                     "] module (stage %s) VkPipelineShaderStageCreateInfo has no "
                                     "VkPipelineShaderStageModuleIdentifierCreateInfoEXT "
                                     "struct and no VkShaderModuleCreateInfo struct in the pNext chain, and module is not a valid "
                                     "VkShaderModule",
                                     pipeline.GetCreateFunctionName(), pipeline.create_index,
                                     string_VkShaderStageFlagBits(stage_ci.stage));
                }
            } else if (stage_ci.module == VK_NULL_HANDLE && !enabled_features.maintenance5_features.maintenance5) {
                skip |= LogError(
                    device, "VUID-VkPipelineShaderStageCreateInfo-stage-08771",
                    "%s pCreateInfos[%" PRIu32
                    "] module (stage %s) VkPipelineShaderStageCreateInfo has no VkPipelineShaderStageModuleIdentifierCreateInfoEXT "
                    "struct in the pNext chain, the neither graphicsPipelineLibrary or maintenance5 feature are not enabled, and "
                    "module is not a valid VkShaderModule",
                    pipeline.GetCreateFunctionName(), pipeline.create_index, string_VkShaderStageFlagBits(stage_ci.stage));
            }
        }
    }
    return skip;
}
