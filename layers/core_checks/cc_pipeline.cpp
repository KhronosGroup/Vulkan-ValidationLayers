/* Copyright (c) 2015-2024 The Khronos Group Inc.
 * Copyright (c) 2015-2024 Valve Corporation
 * Copyright (c) 2015-2024 LunarG, Inc.
 * Copyright (C) 2015-2024 Google Inc.
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

#include <vulkan/vk_enum_string_helper.h>
#include "generated/chassis.h"
#include "core_validation.h"
#include "state_tracker/device_state.h"
#include "state_tracker/descriptor_sets.h"
#include "state_tracker/render_pass_state.h"

bool CoreChecks::IsBeforeCtsVersion(uint32_t major, uint32_t minor, uint32_t subminor) const {
    // If VK_KHR_driver_properties is not enabled then conformance version will not be set
    if (phys_dev_props_core12.conformanceVersion.major == 0) {
        return false;
    }
    if (phys_dev_props_core12.conformanceVersion.major != major) {
        return phys_dev_props_core12.conformanceVersion.major < major;
    }
    if (phys_dev_props_core12.conformanceVersion.minor != minor) {
        return phys_dev_props_core12.conformanceVersion.minor < minor;
    }
    return phys_dev_props_core12.conformanceVersion.subminor < subminor;
}

bool CoreChecks::ValidatePipelineCacheControlFlags(VkPipelineCreateFlags2KHR flags, const Location &loc, const char *vuid) const {
    bool skip = false;
    if (enabled_features.pipelineCreationCacheControl == VK_FALSE) {
        const VkPipelineCreateFlags invalid_flags =
            VK_PIPELINE_CREATE_2_FAIL_ON_PIPELINE_COMPILE_REQUIRED_BIT_KHR | VK_PIPELINE_CREATE_2_EARLY_RETURN_ON_FAILURE_BIT_KHR;
        if ((flags & invalid_flags) != 0) {
            skip |= LogError(vuid, device, loc, "is %s but pipelineCreationCacheControl feature was not enabled.",
                             string_VkPipelineCreateFlags2KHR(flags).c_str());
        }
    }
    return skip;
}

bool CoreChecks::ValidatePipelineIndirectBindableFlags(VkPipelineCreateFlags2KHR flags, const Location &loc,
                                                       const char *vuid) const {
    bool skip = false;
    if (enabled_features.deviceGeneratedComputePipelines == VK_FALSE) {
        if ((flags & VK_PIPELINE_CREATE_2_INDIRECT_BINDABLE_BIT_NV) != 0) {
            skip |= LogError(vuid, device, loc, "is %s but deviceGeneratedComputePipelines feature was not enabled.",
                             string_VkPipelineCreateFlags2KHR(flags).c_str());
        }
    }
    return skip;
}

bool CoreChecks::ValidatePipelineProtectedAccessFlags(VkPipelineCreateFlags2KHR flags, const Location &loc) const {
    bool skip = false;
    if (enabled_features.pipelineProtectedAccess == VK_FALSE) {
        const VkPipelineCreateFlags invalid_flags =
            VK_PIPELINE_CREATE_2_NO_PROTECTED_ACCESS_BIT_EXT | VK_PIPELINE_CREATE_2_PROTECTED_ACCESS_ONLY_BIT_EXT;
        if ((flags & invalid_flags) != 0) {
            skip |= LogError("VUID-VkGraphicsPipelineCreateInfo-pipelineProtectedAccess-07368", device, loc,
                             "is %s, but pipelineProtectedAccess feature was not enabled.",
                             string_VkPipelineCreateFlags2KHR(flags).c_str());
        }
    }
    if ((flags & VK_PIPELINE_CREATE_2_NO_PROTECTED_ACCESS_BIT_EXT) && (flags & VK_PIPELINE_CREATE_2_PROTECTED_ACCESS_ONLY_BIT_EXT)) {
        skip |= LogError("VUID-VkGraphicsPipelineCreateInfo-flags-07369", device, loc,
                         "is %s (contains both NO_PROTECTED_ACCESS_BIT and PROTECTED_ACCESS_ONLY_BIT).",
                         string_VkPipelineCreateFlags2KHR(flags).c_str());
    }
    return skip;
}

bool CoreChecks::PreCallValidateCreatePipelineCache(VkDevice device, const VkPipelineCacheCreateInfo *pCreateInfo,
                                                    const VkAllocationCallbacks *pAllocator, VkPipelineCache *pPipelineCache,
                                                    const ErrorObject &error_obj) const {
    bool skip = false;
    if (enabled_features.pipelineCreationCacheControl == VK_FALSE) {
        if ((pCreateInfo->flags & VK_PIPELINE_CACHE_CREATE_EXTERNALLY_SYNCHRONIZED_BIT_EXT) != 0) {
            skip |= LogError("VUID-VkPipelineCacheCreateInfo-pipelineCreationCacheControl-02892", device,
                             error_obj.location.dot(Field::pCreateInfo).dot(Field::flags),
                             "includes VK_PIPELINE_CACHE_CREATE_EXTERNALLY_SYNCHRONIZED_BIT_EXT, but pipelineCreationCacheControl "
                             "feature was not enabled");
        }
    }
    return skip;
}

// This can be chained in the vkCreate*Pipelines() function or the VkPipelineShaderStageCreateInfo
bool CoreChecks::ValidatePipelineRobustnessCreateInfo(const vvl::Pipeline &pipeline,
                                                      const VkPipelineRobustnessCreateInfoEXT &pipeline_robustness_info,
                                                      const Location &loc) const {
    bool skip = false;

    if (!enabled_features.pipelineRobustness) {
        if (pipeline_robustness_info.storageBuffers != VK_PIPELINE_ROBUSTNESS_BUFFER_BEHAVIOR_DEVICE_DEFAULT_EXT) {
            skip |= LogError("VUID-VkPipelineRobustnessCreateInfoEXT-pipelineRobustness-06926", device,
                             loc.pNext(Struct::VkPipelineRobustnessCreateInfoEXT, Field::storageBuffers),
                             "is %s but the pipelineRobustness feature was not enabled.",
                             string_VkPipelineRobustnessBufferBehaviorEXT(pipeline_robustness_info.storageBuffers));
        }
        if (pipeline_robustness_info.uniformBuffers != VK_PIPELINE_ROBUSTNESS_BUFFER_BEHAVIOR_DEVICE_DEFAULT_EXT) {
            skip |= LogError("VUID-VkPipelineRobustnessCreateInfoEXT-pipelineRobustness-06927", device,
                             loc.pNext(Struct::VkPipelineRobustnessCreateInfoEXT, Field::uniformBuffers),
                             "is %s but the pipelineRobustness feature was not enabled.",
                             string_VkPipelineRobustnessBufferBehaviorEXT(pipeline_robustness_info.uniformBuffers));
        }
        if (pipeline_robustness_info.vertexInputs != VK_PIPELINE_ROBUSTNESS_BUFFER_BEHAVIOR_DEVICE_DEFAULT_EXT) {
            skip |= LogError("VUID-VkPipelineRobustnessCreateInfoEXT-pipelineRobustness-06928", device,
                             loc.pNext(Struct::VkPipelineRobustnessCreateInfoEXT, Field::vertexInputs),
                             "is %s but the pipelineRobustness feature was not enabled.",
                             string_VkPipelineRobustnessBufferBehaviorEXT(pipeline_robustness_info.vertexInputs));
        }
        if (pipeline_robustness_info.images != VK_PIPELINE_ROBUSTNESS_IMAGE_BEHAVIOR_DEVICE_DEFAULT_EXT) {
            skip |= LogError("VUID-VkPipelineRobustnessCreateInfoEXT-pipelineRobustness-06929", device,
                             loc.pNext(Struct::VkPipelineRobustnessCreateInfoEXT, Field::images),
                             "is %s but the pipelineRobustness feature was not enabled.",
                             string_VkPipelineRobustnessImageBehaviorEXT(pipeline_robustness_info.images));
        }
    }

    // These validation depend if the features are exposed (not just enabled)
    if (!has_robust_image_access &&
        pipeline_robustness_info.images == VK_PIPELINE_ROBUSTNESS_IMAGE_BEHAVIOR_ROBUST_IMAGE_ACCESS_EXT) {
        skip |= LogError("VUID-VkPipelineRobustnessCreateInfoEXT-robustImageAccess-06930", device,
                         loc.pNext(Struct::VkPipelineRobustnessCreateInfoEXT, Field::images),
                         "is VK_PIPELINE_ROBUSTNESS_IMAGE_BEHAVIOR_ROBUST_IMAGE_ACCESS_EXT "
                         "but robustImageAccess2 is not supported.");
    }
    return skip;
}

bool CoreChecks::PreCallValidateGetPipelineExecutablePropertiesKHR(VkDevice device, const VkPipelineInfoKHR *pPipelineInfo,
                                                                   uint32_t *pExecutableCount,
                                                                   VkPipelineExecutablePropertiesKHR *pProperties,
                                                                   const ErrorObject &error_obj) const {
    bool skip = false;
    skip |= ValidatePipelineExecutableInfo(device, nullptr, error_obj.location,
                                           "VUID-vkGetPipelineExecutablePropertiesKHR-pipelineExecutableInfo-03270");
    return skip;
}

bool CoreChecks::ValidatePipelineExecutableInfo(VkDevice device, const VkPipelineExecutableInfoKHR *pExecutableInfo,
                                                const Location &loc, const char *feature_vuid) const {
    bool skip = false;

    // If feature is not enabled, not allowed to call dispatch call below
    if (!enabled_features.pipelineExecutableInfo) {
        skip |= LogError(feature_vuid, device, loc, "called when pipelineExecutableInfo feature is not enabled.");
    } else if (pExecutableInfo) {
        // vkGetPipelineExecutablePropertiesKHR will not have struct to validate further
        VkPipelineInfoKHR pi = vku::InitStructHelper();
        pi.pipeline = pExecutableInfo->pipeline;

        // We could probably cache this instead of fetching it every time
        uint32_t executable_count = 0;
        DispatchGetPipelineExecutablePropertiesKHR(device, &pi, &executable_count, NULL);

        if (pExecutableInfo->executableIndex >= executable_count) {
            skip |= LogError("VUID-VkPipelineExecutableInfoKHR-executableIndex-03275", pExecutableInfo->pipeline,
                             loc.dot(Field::pExecutableInfo).dot(Field::executableIndex),
                             "(%" PRIu32
                             ") must be less than the number of executables associated with "
                             "the pipeline (%" PRIu32 ") as returned by vkGetPipelineExecutablePropertiessKHR.",
                             pExecutableInfo->executableIndex, executable_count);
        }
    }

    return skip;
}

bool CoreChecks::PreCallValidateGetPipelineExecutableStatisticsKHR(VkDevice device,
                                                                   const VkPipelineExecutableInfoKHR *pExecutableInfo,
                                                                   uint32_t *pStatisticCount,
                                                                   VkPipelineExecutableStatisticKHR *pStatistics,
                                                                   const ErrorObject &error_obj) const {
    bool skip = false;
    skip |= ValidatePipelineExecutableInfo(device, pExecutableInfo, error_obj.location,
                                           "VUID-vkGetPipelineExecutableStatisticsKHR-pipelineExecutableInfo-03272");

    auto pipeline_state = Get<vvl::Pipeline>(pExecutableInfo->pipeline);
    ASSERT_AND_RETURN_SKIP(pipeline_state);
    if (!(pipeline_state->create_flags & VK_PIPELINE_CREATE_CAPTURE_STATISTICS_BIT_KHR)) {
        skip |= LogError("VUID-vkGetPipelineExecutableStatisticsKHR-pipeline-03274", pExecutableInfo->pipeline, error_obj.location,
                         "called on a pipeline created without the "
                         "VK_PIPELINE_CREATE_CAPTURE_STATISTICS_BIT_KHR flag set.");
    }

    return skip;
}

bool CoreChecks::PreCallValidateGetPipelineExecutableInternalRepresentationsKHR(
    VkDevice device, const VkPipelineExecutableInfoKHR *pExecutableInfo, uint32_t *pInternalRepresentationCount,
    VkPipelineExecutableInternalRepresentationKHR *pStatistics, const ErrorObject &error_obj) const {
    bool skip = false;
    skip |= ValidatePipelineExecutableInfo(device, pExecutableInfo, error_obj.location,
                                           "VUID-vkGetPipelineExecutableInternalRepresentationsKHR-pipelineExecutableInfo-03276");

    auto pipeline_state = Get<vvl::Pipeline>(pExecutableInfo->pipeline);
    ASSERT_AND_RETURN_SKIP(pipeline_state);
    if (!(pipeline_state->create_flags & VK_PIPELINE_CREATE_CAPTURE_INTERNAL_REPRESENTATIONS_BIT_KHR)) {
        skip |= LogError("VUID-vkGetPipelineExecutableInternalRepresentationsKHR-pipeline-03278", pExecutableInfo->pipeline,
                         error_obj.location,
                         "called on a pipeline created without the "
                         "VK_PIPELINE_CREATE_CAPTURE_INTERNAL_REPRESENTATIONS_BIT_KHR flag set.");
    }

    return skip;
}

bool CoreChecks::PreCallValidateDestroyPipeline(VkDevice device, VkPipeline pipeline, const VkAllocationCallbacks *pAllocator,
                                                const ErrorObject &error_obj) const {
    bool skip = false;
    if (auto pipeline_state = Get<vvl::Pipeline>(pipeline)) {
        skip |= ValidateObjectNotInUse(pipeline_state.get(), error_obj.location, "VUID-vkDestroyPipeline-pipeline-00765");
    }
    return skip;
}

bool CoreChecks::PreCallValidateCmdBindPipeline(VkCommandBuffer commandBuffer, VkPipelineBindPoint pipelineBindPoint,
                                                VkPipeline pipeline, const ErrorObject &error_obj) const {
    auto cb_state = GetRead<vvl::CommandBuffer>(commandBuffer);

    bool skip = false;
    skip |= ValidateCmd(*cb_state, error_obj.location);
    skip |= ValidatePipelineBindPoint(*cb_state, pipelineBindPoint, error_obj.location);

    auto pipeline_ptr = Get<vvl::Pipeline>(pipeline);
    ASSERT_AND_RETURN_SKIP(pipeline_ptr);
    const vvl::Pipeline &pipeline_state = *pipeline_ptr;

    if (pipelineBindPoint != pipeline_state.pipeline_type) {
        if (pipelineBindPoint == VK_PIPELINE_BIND_POINT_GRAPHICS) {
            const LogObjectList objlist(cb_state->Handle(), pipeline);
            skip |= LogError("VUID-vkCmdBindPipeline-pipelineBindPoint-00779", objlist, error_obj.location,
                             "Cannot bind a pipeline of type %s to the graphics pipeline bind point",
                             string_VkPipelineBindPoint(pipeline_state.pipeline_type));
        } else if (pipelineBindPoint == VK_PIPELINE_BIND_POINT_COMPUTE) {
            const LogObjectList objlist(cb_state->Handle(), pipeline);
            skip |= LogError("VUID-vkCmdBindPipeline-pipelineBindPoint-00780", objlist, error_obj.location,
                             "Cannot bind a pipeline of type %s to the compute pipeline bind point",
                             string_VkPipelineBindPoint(pipeline_state.pipeline_type));
        } else if (pipelineBindPoint == VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR) {
            const LogObjectList objlist(cb_state->Handle(), pipeline);
            skip |= LogError("VUID-vkCmdBindPipeline-pipelineBindPoint-02392", objlist, error_obj.location,
                             "Cannot bind a pipeline of type %s to the ray-tracing pipeline bind point",
                             string_VkPipelineBindPoint(pipeline_state.pipeline_type));
        }
    } else {
        if (pipelineBindPoint == VK_PIPELINE_BIND_POINT_GRAPHICS) {
            skip |= ValidateGraphicsPipelineBindPoint(*cb_state, pipeline_state, error_obj.location);

            if (cb_state->activeRenderPass &&
                phys_dev_ext_props.provoking_vertex_props.provokingVertexModePerPipeline == VK_FALSE) {
                const auto lvl_bind_point = ConvertToLvlBindPoint(pipelineBindPoint);
                const auto &last_bound = cb_state->lastBound[lvl_bind_point];
                if (last_bound.pipeline_state) {
                    auto last_bound_provoking_vertex_state_ci =
                        vku::FindStructInPNextChain<VkPipelineRasterizationProvokingVertexStateCreateInfoEXT>(
                            last_bound.pipeline_state->RasterizationStatePNext());

                    auto current_provoking_vertex_state_ci =
                        vku::FindStructInPNextChain<VkPipelineRasterizationProvokingVertexStateCreateInfoEXT>(
                            pipeline_state.RasterizationStatePNext());

                    if (last_bound_provoking_vertex_state_ci && !current_provoking_vertex_state_ci) {
                        const LogObjectList objlist(cb_state->Handle(), pipeline);
                        skip |= LogError("VUID-vkCmdBindPipeline-pipelineBindPoint-04881", objlist, error_obj.location,
                                         "Previous %s's provokingVertexMode is %s, but %s doesn't chain "
                                         "VkPipelineRasterizationProvokingVertexStateCreateInfoEXT.",
                                         FormatHandle(last_bound.pipeline_state->Handle()).c_str(),
                                         string_VkProvokingVertexModeEXT(last_bound_provoking_vertex_state_ci->provokingVertexMode),
                                         FormatHandle(pipeline).c_str());
                    } else if (!last_bound_provoking_vertex_state_ci && current_provoking_vertex_state_ci) {
                        const LogObjectList objlist(cb_state->Handle(), pipeline);
                        skip |= LogError("VUID-vkCmdBindPipeline-pipelineBindPoint-04881", objlist, error_obj.location,
                                         "%s's provokingVertexMode is %s, but previous %s doesn't chain "
                                         "VkPipelineRasterizationProvokingVertexStateCreateInfoEXT.",
                                         FormatHandle(pipeline).c_str(),
                                         string_VkProvokingVertexModeEXT(current_provoking_vertex_state_ci->provokingVertexMode),
                                         FormatHandle(last_bound.pipeline_state->Handle()).c_str());
                    } else if (last_bound_provoking_vertex_state_ci && current_provoking_vertex_state_ci &&
                               last_bound_provoking_vertex_state_ci->provokingVertexMode !=
                                   current_provoking_vertex_state_ci->provokingVertexMode) {
                        const LogObjectList objlist(cb_state->Handle(), pipeline);
                        skip |=
                            LogError("VUID-vkCmdBindPipeline-pipelineBindPoint-04881", objlist, error_obj.location,
                                     "%s's provokingVertexMode is %s, but previous %s's provokingVertexMode is %s.",
                                     FormatHandle(pipeline).c_str(),
                                     string_VkProvokingVertexModeEXT(current_provoking_vertex_state_ci->provokingVertexMode),
                                     FormatHandle(last_bound.pipeline_state->Handle()).c_str(),
                                     string_VkProvokingVertexModeEXT(last_bound_provoking_vertex_state_ci->provokingVertexMode));
                    }
                }
            }

            if (cb_state->activeRenderPass && phys_dev_ext_props.sample_locations_props.variableSampleLocations == VK_FALSE) {
                const auto *multisample_state = pipeline_state.MultisampleState();
                const auto *sample_locations = vku::FindStructInPNextChain<VkPipelineSampleLocationsStateCreateInfoEXT>(multisample_state);
                if (sample_locations && sample_locations->sampleLocationsEnable == VK_TRUE &&
                    !pipeline_state.IsDynamic(CB_DYNAMIC_STATE_SAMPLE_LOCATIONS_EXT)) {
                    const auto *sample_locations_begin_info =
                        vku::FindStructInPNextChain<VkRenderPassSampleLocationsBeginInfoEXT>(cb_state->active_render_pass_begin_info.pNext);
                    bool found = false;
                    if (sample_locations_begin_info) {
                        for (uint32_t i = 0; i < sample_locations_begin_info->postSubpassSampleLocationsCount; ++i) {
                            if (sample_locations_begin_info->pPostSubpassSampleLocations[i].subpassIndex ==
                                cb_state->GetActiveSubpass()) {
                                if (MatchSampleLocationsInfo(
                                        sample_locations_begin_info->pPostSubpassSampleLocations[i].sampleLocationsInfo,
                                        sample_locations->sampleLocationsInfo)) {
                                    found = true;
                                }
                            }
                        }
                    }
                    if (!found) {
                        const LogObjectList objlist(cb_state->Handle(), pipeline, cb_state->activeRenderPass->Handle());
                        skip |= LogError("VUID-vkCmdBindPipeline-variableSampleLocations-01525", objlist, error_obj.location,
                                         "the current render pass was not begun with any element of "
                                         "pPostSubpassSampleLocations subpassIndex "
                                         "matching the current subpass index (%" PRIu32
                                         ") and sampleLocationsInfo from VkPipelineMultisampleStateCreateInfo of the pipeline.",
                                         cb_state->GetActiveSubpass());
                    }
                }
            }

            if (enabled_features.variableMultisampleRate == VK_FALSE) {
                if (const auto *multisample_state = pipeline_state.MultisampleState(); multisample_state) {
                    if (const auto &render_pass = cb_state->activeRenderPass; render_pass) {
                        const uint32_t subpass = cb_state->GetActiveSubpass();
                        // if render pass uses no attachment, verify that all bound pipelines referencing this subpass have the same
                        // pMultisampleState->rasterizationSamples.
                        if (render_pass->UsesNoAttachment(subpass)) {
                            // If execution ends up here, GetActiveSubpassRasterizationSampleCount() can still be empty if this is
                            // the first bound pipeline with the previous conditions holding. Rasterization samples count for the
                            // subpass will be updated in PostCallRecordCmdBindPipeline, if it is empty.
                            if (std::optional<VkSampleCountFlagBits> subpass_rasterization_samples =
                                    cb_state->GetActiveSubpassRasterizationSampleCount();
                                subpass_rasterization_samples &&
                                *subpass_rasterization_samples != multisample_state->rasterizationSamples) {
                                const LogObjectList objlist(device, render_pass->Handle(), pipeline_state.Handle());
                                skip |= LogError(
                                    "VUID-vkCmdBindPipeline-pipeline-00781", objlist, error_obj.location,
                                    "variableMultisampleRate is VK_FALSE "
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

            if (cb_state->GetCurrentPipeline(pipelineBindPoint) &&
                pipeline == cb_state->GetCurrentPipeline(pipelineBindPoint)->VkHandle() && cb_state->dirtyStaticState &&
                IsBeforeCtsVersion(1, 3, 8)) {
                const LogObjectList objlist(commandBuffer, pipeline);
                // This catches a bug in some drivers with conformance version lower than 1.3.8
                // https://gitlab.khronos.org/vulkan/vulkan/-/issues/3675
                // https://gitlab.khronos.org/Tracker/vk-gl-cts/-/issues/4642
                skip |= LogError(
                    "UNASSIGNED-vkCmdBindPipeline-Pipeline-Rebind", objlist, error_obj.location,
                    "The pipeline being bound (%s) is the same as the currently bound pipeline and between the calls, a "
                    "dynamic state was set which is static in this pipeline. This might not work correctly on drivers with "
                    "conformance version lower than 1.3.8.0.",
                    FormatHandle(pipeline).c_str());
            }
        } else if (pipelineBindPoint == VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR) {
            if (!cb_state->unprotected) {
                const LogObjectList objlist(cb_state->Handle(), pipeline);
                skip |= LogError("VUID-vkCmdBindPipeline-pipelineBindPoint-06721", objlist, error_obj.location,
                                 "Binding pipeline to VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR in a protected command buffer.");
            }
        }
        if (pipeline_state.create_flags & VK_PIPELINE_CREATE_LIBRARY_BIT_KHR) {
            const LogObjectList objlist(cb_state->Handle(), pipeline);
            skip |= LogError("VUID-vkCmdBindPipeline-pipeline-03382", objlist, error_obj.location,
                             "Cannot bind a pipeline that was created with the VK_PIPELINE_CREATE_LIBRARY_BIT_KHR flag.");
        }
        if (cb_state->transform_feedback_active) {
            const LogObjectList objlist(cb_state->Handle(), pipeline);
            skip |= LogError("VUID-vkCmdBindPipeline-None-02323", objlist, error_obj.location, "transform feedback is active.");
        }
        if (enabled_features.pipelineProtectedAccess) {
            if (cb_state->unprotected) {
                const LogObjectList objlist(cb_state->Handle(), pipeline);
                if (pipeline_state.create_flags & VK_PIPELINE_CREATE_PROTECTED_ACCESS_ONLY_BIT_EXT) {
                    skip |= LogError("VUID-vkCmdBindPipeline-pipelineProtectedAccess-07409", objlist, error_obj.location,
                                     "Binding pipeline created with "
                                     "VK_PIPELINE_CREATE_PROTECTED_ACCESS_ONLY_BIT_EXT in an unprotected command buffer.");
                }
            } else {
                const LogObjectList objlist(cb_state->Handle(), pipeline);
                if (pipeline_state.create_flags & VK_PIPELINE_CREATE_NO_PROTECTED_ACCESS_BIT_EXT) {
                    skip |= LogError("VUID-vkCmdBindPipeline-pipelineProtectedAccess-07408", objlist, error_obj.location,
                                     "Binding pipeline created with "
                                     "VK_PIPELINE_CREATE_NO_PROTECTED_ACCESS_BIT_EXT in a protected command buffer.");
                }
            }
        }
    }

    return skip;
}

// Validates that the supplied bind point is supported for the command buffer (vis. the command pool)
// Takes array of error codes as some of the VUID's (e.g. vkCmdBindPipeline) are written per bindpoint
// TODO add vkCmdBindPipeline bind_point validation using this call.
bool CoreChecks::ValidatePipelineBindPoint(const vvl::CommandBuffer &cb_state, VkPipelineBindPoint bind_point,
                                           const Location &loc) const {
    bool skip = false;
    const auto *pool = cb_state.command_pool;
    // The loss of a pool in a recording cmd is reported in DestroyCommandPool
    if (!pool) return skip;

    const VkQueueFlags required_mask = (VK_PIPELINE_BIND_POINT_GRAPHICS == bind_point)  ? VK_QUEUE_GRAPHICS_BIT
                                       : (VK_PIPELINE_BIND_POINT_COMPUTE == bind_point) ? VK_QUEUE_COMPUTE_BIT
                                       : (VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR == bind_point)
                                           ? (VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_COMPUTE_BIT)
                                           : VK_QUEUE_FLAG_BITS_MAX_ENUM;

    const auto &qfp = physical_device_state->queue_family_properties[pool->queueFamilyIndex];
    if (0 == (qfp.queueFlags & required_mask)) {
        const LogObjectList objlist(cb_state.Handle(), cb_state.allocate_info.commandPool);
        const char *vuid = kVUIDUndefined;
        switch (loc.function) {
            case Func::vkCmdBindDescriptorSets:
                vuid = "VUID-vkCmdBindDescriptorSets-pipelineBindPoint-00361";
                break;
            case Func::vkCmdBindDescriptorSets2KHR:
                vuid = "VUID-vkCmdBindDescriptorSets2KHR-pBindDescriptorSetsInfo-09467";
                break;
            case Func::vkCmdSetDescriptorBufferOffsetsEXT:
                vuid = "VUID-vkCmdSetDescriptorBufferOffsetsEXT-pipelineBindPoint-08067";
                break;
            case Func::vkCmdSetDescriptorBufferOffsets2EXT:
                vuid = "VUID-vkCmdSetDescriptorBufferOffsets2EXT-pSetDescriptorBufferOffsetsInfo-09471";
                break;
            case Func::vkCmdBindDescriptorBufferEmbeddedSamplersEXT:
                vuid = "VUID-vkCmdBindDescriptorBufferEmbeddedSamplersEXT-pipelineBindPoint-08069";
                break;
            case Func::vkCmdBindDescriptorBufferEmbeddedSamplers2EXT:
                vuid = "VUID-vkCmdBindDescriptorBufferEmbeddedSamplers2EXT-pBindDescriptorBufferEmbeddedSamplersInfo-09473";
                break;
            case Func::vkCmdPushDescriptorSetKHR:
                vuid = "VUID-vkCmdPushDescriptorSetKHR-pipelineBindPoint-00363";
                break;
            case Func::vkCmdPushDescriptorSet2KHR:
                vuid = "VUID-vkCmdPushDescriptorSet2KHR-pPushDescriptorSetInfo-09468";
                break;
            case Func::vkCmdPushDescriptorSetWithTemplateKHR:
                vuid = "VUID-vkCmdPushDescriptorSetWithTemplateKHR-commandBuffer-00366";
                break;
            case Func::vkCmdPushDescriptorSetWithTemplate2KHR:
                vuid = "VUID-VkPushDescriptorSetWithTemplateInfoKHR-commandBuffer-00366";
                break;
            case Func::vkCmdBindPipeline:
                if (VK_PIPELINE_BIND_POINT_GRAPHICS == bind_point) {
                    vuid = "VUID-vkCmdBindPipeline-pipelineBindPoint-00778";
                } else if (VK_PIPELINE_BIND_POINT_COMPUTE == bind_point) {
                    vuid = "VUID-vkCmdBindPipeline-pipelineBindPoint-00777";
                } else if (VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR == bind_point) {
                    vuid = "VUID-vkCmdBindPipeline-pipelineBindPoint-02391";
                }
                break;
            default:
                break;
        }
        skip |= LogError(vuid, objlist, loc, "%s was allocated from %s that does not support bindpoint %s.",
                         FormatHandle(cb_state.Handle()).c_str(), FormatHandle(cb_state.allocate_info.commandPool).c_str(),
                         string_VkPipelineBindPoint(bind_point));
    }
    return skip;
}

bool CoreChecks::ValidateShaderSubgroupSizeControl(VkShaderStageFlagBits stage, const ShaderStageState &stage_state,
                                                   const Location &loc) const {
    bool skip = false;

    if (stage_state.HasPipeline()) {
        const auto flags = stage_state.pipeline_create_info->flags;

        if ((flags & VK_PIPELINE_SHADER_STAGE_CREATE_ALLOW_VARYING_SUBGROUP_SIZE_BIT_EXT) != 0 &&
            !enabled_features.subgroupSizeControl) {
            skip |= LogError("VUID-VkPipelineShaderStageCreateInfo-flags-02784", device, loc.dot(Field::flags),
                             "includes "
                             "VK_PIPELINE_SHADER_STAGE_CREATE_ALLOW_VARYING_SUBGROUP_SIZE_BIT_EXT, "
                             "but the subgroupSizeControl feature was not enabled.");
        }

        if ((flags & VK_PIPELINE_SHADER_STAGE_CREATE_REQUIRE_FULL_SUBGROUPS_BIT_EXT) != 0) {
            if (!enabled_features.computeFullSubgroups) {
                skip |=
                    LogError("VUID-VkPipelineShaderStageCreateInfo-flags-02785", device, loc.dot(Field::flags),
                             "includes "
                             "VK_PIPELINE_SHADER_STAGE_CREATE_REQUIRE_FULL_SUBGROUPS_BIT_EXT, but the computeFullSubgroups feature "
                             "was not enabled");
            } else if ((stage & (VK_SHADER_STAGE_MESH_BIT_EXT | VK_SHADER_STAGE_TASK_BIT_EXT | VK_SHADER_STAGE_COMPUTE_BIT)) == 0) {
                skip |= LogError("VUID-VkPipelineShaderStageCreateInfo-flags-08988", device, loc.dot(Field::flags),
                                 "includes "
                                 "VK_PIPELINE_SHADER_STAGE_CREATE_REQUIRE_FULL_SUBGROUPS_BIT_EXT, but the stage is %s.",
                                 string_VkShaderStageFlagBits(stage));
            }
        }
    } else {
        const auto flags = stage_state.shader_object_create_info->flags;
        if ((flags & VK_SHADER_CREATE_REQUIRE_FULL_SUBGROUPS_BIT_EXT) != 0) {
            if ((stage & (VK_SHADER_STAGE_MESH_BIT_EXT | VK_SHADER_STAGE_TASK_BIT_EXT | VK_SHADER_STAGE_COMPUTE_BIT)) == 0) {
                skip |= LogError("VUID-VkShaderCreateInfoEXT-flags-08992", device, loc.dot(Field::flags),
                                 "includes VK_PIPELINE_SHADER_STAGE_CREATE_REQUIRE_FULL_SUBGROUPS_BIT_EXT, but the stage is %s.",
                                 string_VkShaderStageFlagBits(stage));
            }
        }
    }

    return skip;
}

// Validate that data for each specialization entry is fully contained within the buffer.
bool CoreChecks::ValidateSpecializations(const vku::safe_VkSpecializationInfo *spec, const Location &loc) const {
    bool skip = false;
    if (!spec) return skip;

    for (auto i = 0u; i < spec->mapEntryCount; i++) {
        const Location map_loc = loc.dot(Field::pMapEntries, i);
        const auto &map_entry = spec->pMapEntries[i];
        if (map_entry.offset >= spec->dataSize) {
            skip |= LogError("VUID-VkSpecializationInfo-offset-00773", device, map_loc.dot(Field::offset),
                             "is %" PRIu32 " but dataSize is %zu (for constantID %" PRIu32 ").", map_entry.offset, spec->dataSize,
                             map_entry.constantID);

            continue;
        }
        if (map_entry.offset + map_entry.size > spec->dataSize) {
            skip |= LogError("VUID-VkSpecializationInfo-pMapEntries-00774", device, map_loc.dot(Field::size),
                             "(%zu) plus offset (%" PRIu32 ") is greater than dataSize (%zu) (for constantID %" PRIu32 ").",
                             map_entry.size, map_entry.offset, spec->dataSize, map_entry.constantID);
        }
        for (uint32_t j = i + 1; j < spec->mapEntryCount; ++j) {
            if (map_entry.constantID == spec->pMapEntries[j].constantID) {
                skip |= LogError("VUID-VkSpecializationInfo-constantID-04911", device, map_loc,
                                 "and pMapEntries[%" PRIu32 "] both have constantID (%" PRIu32 ").", j, map_entry.constantID);
            }
        }
    }

    return skip;
}

bool CoreChecks::ValidateShaderStageMaxResources(VkShaderStageFlagBits stage, const vvl::Pipeline &pipeline,
                                                 const Location &loc) const {
    bool skip = false;
    uint32_t total_resources = 0;

    const auto &rp_state = pipeline.RenderPassState();
    if ((stage == VK_SHADER_STAGE_FRAGMENT_BIT) && rp_state) {
        if (rp_state->UsesDynamicRendering()) {
            total_resources += rp_state->dynamic_pipeline_rendering_create_info.colorAttachmentCount;
        } else {
            // "For the fragment shader stage the framebuffer color attachments also count against this limit"
            if (pipeline.Subpass() < rp_state->create_info.subpassCount) {
                total_resources += rp_state->create_info.pSubpasses[pipeline.Subpass()].colorAttachmentCount;
            }
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
        skip |= LogError(vuid, device, loc,
                         "%s exceeds component limit "
                         "VkPhysicalDeviceLimits::maxPerStageResources (%" PRIu32 ")",
                         string_VkShaderStageFlagBits(stage), phys_dev_props.limits.maxPerStageResources);
    }

    return skip;
}

bool CoreChecks::ValidatePipelineShaderStage(const vvl::Pipeline &pipeline,
                                             const vku::safe_VkPipelineShaderStageCreateInfo &stage_ci, const void *pipeline_ci_pnext,
                                             const Location &loc) const {
    bool skip = false;
    const auto binary_info = vku::FindStructInPNextChain<VkPipelineBinaryInfoKHR>(pipeline_ci_pnext);

    if (binary_info && binary_info->binaryCount != 0)
    {
        return skip;
    }

    const auto module_create_info = vku::FindStructInPNextChain<VkShaderModuleCreateInfo>(stage_ci.pNext);
    if (const auto module_identifier =
            vku::FindStructInPNextChain<VkPipelineShaderStageModuleIdentifierCreateInfoEXT>(stage_ci.pNext)) {
        if (module_identifier->identifierSize > 0) {
            if (!(enabled_features.shaderModuleIdentifier)) {
                skip |=
                    LogError("VUID-VkPipelineShaderStageModuleIdentifierCreateInfoEXT-pNext-06850", device, loc.dot(Field::pNext),
                             "has a "
                             "VkPipelineShaderStageModuleIdentifierCreateInfoEXT "
                             "struct in the pNext chain but the shaderModuleIdentifier feature was not enabled. (stage %s)",
                             string_VkShaderStageFlagBits(stage_ci.stage));
            }
            if (!(pipeline.create_flags & VK_PIPELINE_CREATE_2_FAIL_ON_PIPELINE_COMPILE_REQUIRED_BIT_KHR)) {
                skip |= LogError("VUID-VkPipelineShaderStageModuleIdentifierCreateInfoEXT-pNext-06851", pipeline.Handle(),
                                 loc.pNext(Struct::VkPipelineShaderStageModuleIdentifierCreateInfoEXT, Field::identifierSize),
                                 "(%" PRIu32 "), but the pipeline was created with %s. (stage %s)",
                                 module_identifier->identifierSize, string_VkPipelineCreateFlags2KHR(pipeline.create_flags).c_str(),
                                 string_VkShaderStageFlagBits(stage_ci.stage));
            }
            if (module_identifier->identifierSize > VK_MAX_SHADER_MODULE_IDENTIFIER_SIZE_EXT) {
                skip |= LogError("VUID-VkPipelineShaderStageModuleIdentifierCreateInfoEXT-identifierSize-06852", device,
                                 loc.pNext(Struct::VkPipelineShaderStageModuleIdentifierCreateInfoEXT, Field::identifierSize),
                                 "(%" PRIu32 ") is larger than VK_MAX_SHADER_MODULE_IDENTIFIER_SIZE_EXT (%" PRIu32 "). (stage %s).",
                                 module_identifier->identifierSize, VK_MAX_SHADER_MODULE_IDENTIFIER_SIZE_EXT,
                                 string_VkShaderStageFlagBits(stage_ci.stage));
            }
            if (stage_ci.module != VK_NULL_HANDLE) {
                skip |= LogError("VUID-VkPipelineShaderStageCreateInfo-stage-06848", device, loc.dot(Field::pNext),
                                 "has a VkPipelineShaderStageModuleIdentifierCreateInfoEXT "
                                 "struct in the pNext chain, but module is not VK_NULL_HANDLE. (stage %s).",
                                 string_VkShaderStageFlagBits(stage_ci.stage));
            }
        }
        if (module_create_info) {
            skip |= LogError("VUID-VkPipelineShaderStageCreateInfo-stage-06844", device, loc.dot(Field::pNext),
                             "has both a "
                             "VkPipelineShaderStageModuleIdentifierCreateInfoEXT "
                             "struct and a VkShaderModuleCreateInfo struct in the pNext chain. (stage %s).",
                             string_VkShaderStageFlagBits(stage_ci.stage));
        }
    } else if (stage_ci.module == VK_NULL_HANDLE) {
        if (!enabled_features.maintenance5 && !enabled_features.graphicsPipelineLibrary) {
            skip |= LogError("VUID-VkPipelineShaderStageCreateInfo-stage-08771", device, loc.dot(Field::module),
                             "is VK_NULL_HANDLE and both the graphicsPipelineLibrary and maintenance5 "
                             "features were not enabled. (stage %s).",
                             string_VkShaderStageFlagBits(stage_ci.stage));
        } else if (!module_create_info) {
            skip |= LogError("VUID-VkPipelineShaderStageCreateInfo-stage-06845", device, loc.dot(Field::module),
                             "is VK_NULL_HANDLE, but no "
                             "VkPipelineShaderStageModuleIdentifierCreateInfoEXT or VkShaderModuleCreateInfo found in the "
                             "pNext chain. (stage %s).",
                             string_VkShaderStageFlagBits(stage_ci.stage));
        } else {
            skip |= ValidateShaderModuleCreateInfo(*module_create_info, loc.pNext(Struct::VkShaderModuleCreateInfo));
        }
    }
    return skip;
}

bool CoreChecks::PreCallValidateGetPipelineKeyKHR(VkDevice device, const VkPipelineCreateInfoKHR *pPipelineCreateInfo,
                                                  VkPipelineBinaryKeyKHR *pPipelineKey, const ErrorObject &error_obj) const {
    bool skip = false;

    // Used when getting global key
    if (!pPipelineCreateInfo) return skip;

    const VkBaseOutStructure *pipeline_create_info = reinterpret_cast<const VkBaseOutStructure *>(pPipelineCreateInfo->pNext);
    if (pipeline_create_info) {
        if (pipeline_create_info->sType != VK_STRUCTURE_TYPE_EXECUTION_GRAPH_PIPELINE_CREATE_INFO_AMDX &&
            pipeline_create_info->sType != VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO &&
            pipeline_create_info->sType != VK_STRUCTURE_TYPE_RAY_TRACING_PIPELINE_CREATE_INFO_NV &&
            pipeline_create_info->sType != VK_STRUCTURE_TYPE_RAY_TRACING_PIPELINE_CREATE_INFO_KHR &&
            pipeline_create_info->sType != VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO) {
            skip |= LogError("VUID-VkPipelineCreateInfoKHR-pNext-09604", device,
                             error_obj.location.dot(Field::pPipelineCreateInfo).dot(Field::pNext),
                             "contains an invalid struct (%s).",
                             string_VkStructureType(pipeline_create_info->sType));
        }
    }

    const auto *binary_info = vku::FindStructInPNextChain<VkPipelineBinaryInfoKHR>(pPipelineCreateInfo->pNext);
    if (binary_info && (binary_info->binaryCount > 0)) {
        skip |=
            LogError("VUID-vkGetPipelineKeyKHR-pNext-09605", device,
                     error_obj.location.dot(Field::pPipelineCreateInfo).pNext(Struct::VkPipelineBinaryInfoKHR, Field::binaryCount),
                     "(%" PRIu32 ") is greater than zero", binary_info->binaryCount);
    }

    return skip;
}

bool CoreChecks::PreCallValidateReleaseCapturedPipelineDataKHR(VkDevice device, const VkReleaseCapturedPipelineDataInfoKHR *pInfo,
                                                               const VkAllocationCallbacks *pAllocator,
                                                               const ErrorObject &error_obj) const {
    auto pipeline_state = Get<vvl::Pipeline>(pInfo->pipeline);
    bool skip = false;

    ASSERT_AND_RETURN_SKIP(pipeline_state);

    if (!(pipeline_state->create_flags & VK_PIPELINE_CREATE_2_CAPTURE_DATA_BIT_KHR)) {
        skip |= LogError("VUID-VkReleaseCapturedPipelineDataInfoKHR-pipeline-09613", pInfo->pipeline, error_obj.location.dot(Field::pInfo).dot(Field::pipeline),
                         "called on a pipeline created without the "
                         "VK_PIPELINE_CREATE_2_CAPTURE_DATA_BIT_KHR flag set. (Make sure you set it with VkPipelineCreateFlags2CreateInfoKHR)");
    }

    if (pipeline_state->binary_data_released) {
        skip |= LogError("VUID-VkReleaseCapturedPipelineDataInfoKHR-pipeline-09618", pInfo->pipeline, error_obj.location.dot(Field::pInfo).dot(Field::pipeline),
                         "has been called multiple times.");
    }

    return skip;
}

void CoreChecks::PostCallRecordReleaseCapturedPipelineDataKHR(VkDevice device, const VkReleaseCapturedPipelineDataInfoKHR *pInfo,
                                                              const VkAllocationCallbacks *pAllocator,
                                                              const RecordObject &record_obj) {
    if (VK_SUCCESS != record_obj.result) return;

    StateTracker::PostCallRecordReleaseCapturedPipelineDataKHR(device, pInfo, pAllocator, record_obj);

    auto pipeline_state = Get<vvl::Pipeline>(pInfo->pipeline);
    if (pipeline_state) {
        pipeline_state->binary_data_released = true;
    }
}
