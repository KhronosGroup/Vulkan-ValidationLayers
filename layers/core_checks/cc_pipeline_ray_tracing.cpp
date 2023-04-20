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

#include "generated/vk_enum_string_helper.h"
#include "generated/chassis.h"
#include "core_validation.h"

bool CoreChecks::ValidateRayTracingPipeline(const PIPELINE_STATE &pipeline,
                                            const safe_VkRayTracingPipelineCreateInfoCommon &create_info,
                                            VkPipelineCreateFlags flags, bool isKHR) const {
    bool skip = false;

    if (isKHR) {
        if (create_info.maxPipelineRayRecursionDepth > phys_dev_ext_props.ray_tracing_props_khr.maxRayRecursionDepth) {
            skip |= LogError(device, "VUID-VkRayTracingPipelineCreateInfoKHR-maxPipelineRayRecursionDepth-03589",
                             "vkCreateRayTracingPipelinesKHR: pCreateInfos[%" PRIu32
                             "] maxPipelineRayRecursionDepth (%d ) must be less than or equal to "
                             "VkPhysicalDeviceRayTracingPipelinePropertiesKHR::maxRayRecursionDepth %d",
                             pipeline.create_index, create_info.maxPipelineRayRecursionDepth,
                             phys_dev_ext_props.ray_tracing_props_khr.maxRayRecursionDepth);
        }
        if (create_info.pLibraryInfo) {
            for (uint32_t i = 0; i < create_info.pLibraryInfo->libraryCount; ++i) {
                const auto library_pipelinestate = Get<PIPELINE_STATE>(create_info.pLibraryInfo->pLibraries[i]);
                const auto &library_create_info = library_pipelinestate->GetCreateInfo<VkRayTracingPipelineCreateInfoKHR>();
                if (library_create_info.maxPipelineRayRecursionDepth != create_info.maxPipelineRayRecursionDepth) {
                    skip |= LogError(
                        device, "VUID-VkRayTracingPipelineCreateInfoKHR-pLibraries-03591",
                        "vkCreateRayTracingPipelinesKHR: pCreateInfos[%" PRIu32 "] pLibraries[%" PRIu32
                        "] member of libraries must have been"
                        "created with the value of maxPipelineRayRecursionDepth (%d) equal to that in this pipeline (%d) .",
                        pipeline.create_index, i, library_create_info.maxPipelineRayRecursionDepth,
                        create_info.maxPipelineRayRecursionDepth);
                }
                if (library_create_info.pLibraryInfo && (library_create_info.pLibraryInterface->maxPipelineRayHitAttributeSize !=
                                                             create_info.pLibraryInterface->maxPipelineRayHitAttributeSize ||
                                                         library_create_info.pLibraryInterface->maxPipelineRayPayloadSize !=
                                                             create_info.pLibraryInterface->maxPipelineRayPayloadSize)) {
                    skip |= LogError(device, "VUID-VkRayTracingPipelineCreateInfoKHR-pLibraryInfo-03593",
                                     "vkCreateRayTracingPipelinesKHR: pCreateInfos[%" PRIu32 "] pLibraries[%" PRIu32
                                     "] must have been created with values of the maxPipelineRayPayloadSize and "
                                     "maxPipelineRayHitAttributeSize members of pLibraryInterface equal to those in this pipeline",
                                     pipeline.create_index, i);
                }
                if ((flags & VK_PIPELINE_CREATE_RAY_TRACING_SHADER_GROUP_HANDLE_CAPTURE_REPLAY_BIT_KHR) &&
                    !(library_create_info.flags & VK_PIPELINE_CREATE_RAY_TRACING_SHADER_GROUP_HANDLE_CAPTURE_REPLAY_BIT_KHR)) {
                    skip |=
                        LogError(device, "VUID-VkRayTracingPipelineCreateInfoKHR-flags-03594",
                                 "vkCreateRayTracingPipelinesKHR: pCreateInfos[%" PRIu32
                                 "] If flags includes "
                                 "VK_PIPELINE_CREATE_RAY_TRACING_SHADER_GROUP_HANDLE_CAPTURE_REPLAY_BIT_KHR, pLibraries[%" PRIu32
                                 "] must have been created with the "
                                 "VK_PIPELINE_CREATE_RAY_TRACING_SHADER_GROUP_HANDLE_CAPTURE_REPLAY_BIT_KHR bit set",
                                 pipeline.create_index, i);
                }
            }
        }
    } else {
        if (create_info.maxRecursionDepth > phys_dev_ext_props.ray_tracing_props_nv.maxRecursionDepth) {
            skip |= LogError(device, "VUID-VkRayTracingPipelineCreateInfoNV-maxRecursionDepth-03457",
                             "vkCreateRayTracingPipelinesNV: pCreateInfos[%" PRIu32
                             "] maxRecursionDepth (%d) must be less than or equal to "
                             "VkPhysicalDeviceRayTracingPropertiesNV::maxRecursionDepth (%d)",
                             pipeline.create_index, create_info.maxRecursionDepth,
                             phys_dev_ext_props.ray_tracing_props_nv.maxRecursionDepth);
        }
    }
    const auto *groups = create_info.ptr()->pGroups;

    for (auto &stage_state : pipeline.stage_states) {
        skip |= ValidatePipelineShaderStage(pipeline, stage_state);
    }

    if (const auto *pipeline_robustness_info = LvlFindInChain<VkPipelineRobustnessCreateInfoEXT>(create_info.pNext);
        pipeline_robustness_info) {
        std::stringstream parameter_name;
        parameter_name << "vkCreateRayTracingPipelinesKHR(): pCreateInfos[" << pipeline.create_index << "]";
        skip |= ValidatePipelineRobustnessCreateInfo(pipeline, parameter_name.str().c_str(), *pipeline_robustness_info);
    }

    if ((create_info.flags & VK_PIPELINE_CREATE_LIBRARY_BIT_KHR) == 0) {
        const uint32_t raygen_stages_count = CalcShaderStageCount(pipeline, VK_SHADER_STAGE_RAYGEN_BIT_KHR);
        if (raygen_stages_count == 0) {
            skip |= LogError(
                device,
                isKHR ? "VUID-VkRayTracingPipelineCreateInfoKHR-stage-03425" : "VUID-VkRayTracingPipelineCreateInfoNV-stage-06232",
                "vkCreateRayTracingPipelinesKHR : pCreateInfos[%" PRIu32
                "] The stage member of at least one element of pStages must be VK_SHADER_STAGE_RAYGEN_BIT_KHR.",
                pipeline.create_index);
        }
    }
    if ((flags & VK_PIPELINE_CREATE_RAY_TRACING_SKIP_TRIANGLES_BIT_KHR) != 0 &&
        (flags & VK_PIPELINE_CREATE_RAY_TRACING_SKIP_AABBS_BIT_KHR) != 0) {
        skip |= LogError(device, "VUID-VkRayTracingPipelineCreateInfoKHR-flags-06546",
                         "vkCreateRayTracingPipelinesKHR: pCreateInfos[%" PRIu32
                         "] flags (%s) contains both VK_PIPELINE_CREATE_RAY_TRACING_SKIP_TRIANGLES_BIT_KHR and "
                         "VK_PIPELINE_CREATE_RAY_TRACING_SKIP_AABBS_BIT_KHR bits.",
                         pipeline.create_index, string_VkPipelineCreateFlags(flags).c_str());
    }

    for (uint32_t group_index = 0; group_index < create_info.groupCount; group_index++) {
        const auto &group = groups[group_index];

        if (group.type == VK_RAY_TRACING_SHADER_GROUP_TYPE_GENERAL_NV) {
            if (!GroupHasValidIndex(
                    pipeline, group.generalShader,
                    VK_SHADER_STAGE_RAYGEN_BIT_NV | VK_SHADER_STAGE_MISS_BIT_NV | VK_SHADER_STAGE_CALLABLE_BIT_NV)) {
                skip |= LogError(device,
                                 isKHR ? "VUID-VkRayTracingShaderGroupCreateInfoKHR-type-03474"
                                       : "VUID-VkRayTracingShaderGroupCreateInfoNV-type-02413",
                                 "vkCreateRayTracingPipelinesKHR: pCreateInfos[%" PRIu32 "] pGroups[%d]", pipeline.create_index,
                                 group_index);
            }
            if (group.anyHitShader != VK_SHADER_UNUSED_NV || group.closestHitShader != VK_SHADER_UNUSED_NV ||
                group.intersectionShader != VK_SHADER_UNUSED_NV) {
                skip |= LogError(device,
                                 isKHR ? "VUID-VkRayTracingShaderGroupCreateInfoKHR-type-03475"
                                       : "VUID-VkRayTracingShaderGroupCreateInfoNV-type-02414",
                                 "vkCreateRayTracingPipelinesKHR: pCreateInfos[%" PRIu32 "] pGroups[%d]", pipeline.create_index,
                                 group_index);
            }
        } else if (group.type == VK_RAY_TRACING_SHADER_GROUP_TYPE_PROCEDURAL_HIT_GROUP_NV) {
            if (!GroupHasValidIndex(pipeline, group.intersectionShader, VK_SHADER_STAGE_INTERSECTION_BIT_NV)) {
                skip |= LogError(device,
                                 isKHR ? "VUID-VkRayTracingShaderGroupCreateInfoKHR-type-03476"
                                       : "VUID-VkRayTracingShaderGroupCreateInfoNV-type-02415",
                                 "vkCreateRayTracingPipelinesKHR: pCreateInfos[%" PRIu32 "] pGroups[%d]", pipeline.create_index,
                                 group_index);
            }
        } else if (group.type == VK_RAY_TRACING_SHADER_GROUP_TYPE_TRIANGLES_HIT_GROUP_NV) {
            if (group.intersectionShader != VK_SHADER_UNUSED_NV) {
                skip |= LogError(device,
                                 isKHR ? "VUID-VkRayTracingShaderGroupCreateInfoKHR-type-03477"
                                       : "VUID-VkRayTracingShaderGroupCreateInfoNV-type-02416",
                                 "vkCreateRayTracingPipelinesKHR: pCreateInfos[%" PRIu32 "] pGroups[%d]", pipeline.create_index,
                                 group_index);
            }
        }

        if (group.type == VK_RAY_TRACING_SHADER_GROUP_TYPE_PROCEDURAL_HIT_GROUP_NV ||
            group.type == VK_RAY_TRACING_SHADER_GROUP_TYPE_TRIANGLES_HIT_GROUP_NV) {
            if (!GroupHasValidIndex(pipeline, group.anyHitShader, VK_SHADER_STAGE_ANY_HIT_BIT_KHR)) {
                skip |= LogError(device,
                                 isKHR ? "VUID-VkRayTracingShaderGroupCreateInfoKHR-anyHitShader-03479"
                                       : "VUID-VkRayTracingShaderGroupCreateInfoNV-anyHitShader-02418",
                                 "vkCreateRayTracingPipelinesKHR: pCreateInfos[%" PRIu32 "] pGroups[%d]", pipeline.create_index,
                                 group_index);
            }
            if (!GroupHasValidIndex(pipeline, group.closestHitShader, VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR)) {
                skip |= LogError(device,
                                 isKHR ? "VUID-VkRayTracingShaderGroupCreateInfoKHR-closestHitShader-03478"
                                       : "VUID-VkRayTracingShaderGroupCreateInfoNV-closestHitShader-02417",
                                 "vkCreateRayTracingPipelinesKHR: pCreateInfos[%" PRIu32 "] pGroups[%d]", pipeline.create_index,
                                 group_index);
            }
        }
    }
    return skip;
}

bool CoreChecks::PreCallValidateCreateRayTracingPipelinesNV(VkDevice device, VkPipelineCache pipelineCache, uint32_t count,
                                                            const VkRayTracingPipelineCreateInfoNV *pCreateInfos,
                                                            const VkAllocationCallbacks *pAllocator, VkPipeline *pPipelines,
                                                            void *crtpl_state_data) const {
    bool skip = StateTracker::PreCallValidateCreateRayTracingPipelinesNV(device, pipelineCache, count, pCreateInfos, pAllocator,
                                                                         pPipelines, crtpl_state_data);

    auto *crtpl_state = reinterpret_cast<create_ray_tracing_pipeline_api_state *>(crtpl_state_data);
    for (uint32_t i = 0; i < count; i++) {
        const PIPELINE_STATE *pipeline = crtpl_state->pipe_state[i].get();
        if (!pipeline) {
            continue;
        }
        using CIType = vvl::base_type<decltype(pCreateInfos)>;
        if (pipeline->create_flags & VK_PIPELINE_CREATE_DERIVATIVE_BIT) {
            std::shared_ptr<const PIPELINE_STATE> base_pipeline;
            const auto bpi = pipeline->BasePipelineIndex<CIType>();
            const auto bph = pipeline->BasePipeline<CIType>();
            if (bpi != -1) {
                base_pipeline = crtpl_state->pipe_state[bpi];
            } else if (bph != VK_NULL_HANDLE) {
                base_pipeline = Get<PIPELINE_STATE>(bph);
            }
            if (!base_pipeline || !(base_pipeline->create_flags & VK_PIPELINE_CREATE_ALLOW_DERIVATIVES_BIT)) {
                skip |=
                    LogError(device, "VUID-vkCreateRayTracingPipelinesNV-flags-03416",
                             "vkCreateRayTracingPipelinesNV: pCreateInfos[%" PRIu32
                             "]  If the flags member of any element of pCreateInfos contains the "
                             "VK_PIPELINE_CREATE_DERIVATIVE_BIT flag,"
                             "the base pipeline must have been created with the VK_PIPELINE_CREATE_ALLOW_DERIVATIVES_BIT flag set.",
                             i);
            }
        }
        skip |= ValidateRayTracingPipeline(*pipeline, pipeline->GetCreateInfo<CIType>(), pCreateInfos[i].flags, /*isKHR*/ false);
        skip |= ValidateShaderModuleId(*pipeline);
        skip |= ValidatePipelineCacheControlFlags(pCreateInfos[i].flags, i, "vkCreateRayTracingPipelinesNV",
                                                  "VUID-VkRayTracingPipelineCreateInfoNV-pipelineCreationCacheControl-02905");
    }
    return skip;
}

bool CoreChecks::PreCallValidateCreateRayTracingPipelinesKHR(VkDevice device, VkDeferredOperationKHR deferredOperation,
                                                             VkPipelineCache pipelineCache, uint32_t count,
                                                             const VkRayTracingPipelineCreateInfoKHR *pCreateInfos,
                                                             const VkAllocationCallbacks *pAllocator, VkPipeline *pPipelines,
                                                             void *crtpl_state_data) const {
    bool skip = StateTracker::PreCallValidateCreateRayTracingPipelinesKHR(device, deferredOperation, pipelineCache, count,
                                                                          pCreateInfos, pAllocator, pPipelines, crtpl_state_data);

    auto *crtpl_state = reinterpret_cast<create_ray_tracing_pipeline_khr_api_state *>(crtpl_state_data);
    for (uint32_t i = 0; i < count; i++) {
        const PIPELINE_STATE *pipeline = crtpl_state->pipe_state[i].get();
        if (!pipeline) {
            continue;
        }
        using CIType = vvl::base_type<decltype(pCreateInfos)>;
        if (pipeline->create_flags & VK_PIPELINE_CREATE_DERIVATIVE_BIT) {
            std::shared_ptr<const PIPELINE_STATE> base_pipeline;
            const auto bpi = pipeline->BasePipelineIndex<CIType>();
            const auto bph = pipeline->BasePipeline<CIType>();
            if (bpi != -1) {
                base_pipeline = crtpl_state->pipe_state[bpi];
            } else if (bph != VK_NULL_HANDLE) {
                base_pipeline = Get<PIPELINE_STATE>(bph);
            }
            if (!base_pipeline || !(base_pipeline->create_flags & VK_PIPELINE_CREATE_ALLOW_DERIVATIVES_BIT)) {
                skip |=
                    LogError(device, "VUID-vkCreateRayTracingPipelinesKHR-flags-03416",
                             "vkCreateRayTracingPipelinesKHR: pCreateInfos[%" PRIu32
                             "]  If the flags member of any element of pCreateInfos contains the "
                             "VK_PIPELINE_CREATE_DERIVATIVE_BIT flag,"
                             "the base pipeline must have been created with the VK_PIPELINE_CREATE_ALLOW_DERIVATIVES_BIT flag set.",
                             i);
            }
        }
        skip |= ValidateRayTracingPipeline(*pipeline, pipeline->GetCreateInfo<CIType>(), pCreateInfos[i].flags, /*isKHR*/ true);
        skip |= ValidateShaderModuleId(*pipeline);
        skip |= ValidatePipelineCacheControlFlags(pCreateInfos[i].flags, i, "vkCreateRayTracingPipelinesKHR",
                                                  "VUID-VkRayTracingPipelineCreateInfoKHR-pipelineCreationCacheControl-02905");
        const auto create_info = pipeline->GetCreateInfo<VkRayTracingPipelineCreateInfoKHR>();
        if (create_info.pLibraryInfo) {
            constexpr std::array<std::pair<const char *, VkPipelineCreateFlags>, 7> vuid_map = {{
                {"VUID-VkRayTracingPipelineCreateInfoKHR-flags-04718", VK_PIPELINE_CREATE_RAY_TRACING_SKIP_AABBS_BIT_KHR},
                {"VUID-VkRayTracingPipelineCreateInfoKHR-flags-04719", VK_PIPELINE_CREATE_RAY_TRACING_SKIP_TRIANGLES_BIT_KHR},
                {"VUID-VkRayTracingPipelineCreateInfoKHR-flags-04720",
                 VK_PIPELINE_CREATE_RAY_TRACING_NO_NULL_ANY_HIT_SHADERS_BIT_KHR},
                {"VUID-VkRayTracingPipelineCreateInfoKHR-flags-04721",
                 VK_PIPELINE_CREATE_RAY_TRACING_NO_NULL_CLOSEST_HIT_SHADERS_BIT_KHR},
                {"VUID-VkRayTracingPipelineCreateInfoKHR-flags-04722",
                 VK_PIPELINE_CREATE_RAY_TRACING_NO_NULL_INTERSECTION_SHADERS_BIT_KHR},
                {"VUID-VkRayTracingPipelineCreateInfoKHR-flags-04723", VK_PIPELINE_CREATE_RAY_TRACING_NO_NULL_MISS_SHADERS_BIT_KHR},
                {"VUID-VkRayTracingPipelineCreateInfoKHR-flags-07403", VK_PIPELINE_CREATE_RAY_TRACING_OPACITY_MICROMAP_BIT_EXT},
            }};
            unsigned int descriptor_buffer_library_count = 0;
            for (uint32_t j = 0; j < create_info.pLibraryInfo->libraryCount; ++j) {
                const auto lib = Get<PIPELINE_STATE>(create_info.pLibraryInfo->pLibraries[j]);
                if ((lib->create_flags & VK_PIPELINE_CREATE_LIBRARY_BIT_KHR) == 0) {
                    skip |= LogError(device, "VUID-VkPipelineLibraryCreateInfoKHR-pLibraries-03381",
                                     "vkCreateRayTracingPipelinesKHR(): pCreateInfo[%" PRIu32 "].pLibraryInfo->pLibraries[%" PRIu32
                                     "] was not created with VK_PIPELINE_CREATE_LIBRARY_BIT_KHR.",
                                     i, j);
                }
                if (lib->descriptor_buffer_mode) {
                    ++descriptor_buffer_library_count;
                }
                for (const auto &pair : vuid_map) {
                    if (pipeline->create_flags & pair.second) {
                        if ((lib->create_flags & pair.second) == 0) {
                            skip |= LogError(device, pair.first,
                                             "vkCreateRayTracingPipelinesKHR(): pCreateInfo[%" PRIu32
                                             "].flags contains %s bit, but pCreateInfo[%" PRIu32
                                             "].pLibraryInfo->pLibraries[%" PRIu32 "] was created without it.",
                                             i, string_VkPipelineCreateFlags(pair.second).c_str(), i, j);
                        }
                    }
                }
            }
            if ((descriptor_buffer_library_count != 0) &&
                (create_info.pLibraryInfo->libraryCount != descriptor_buffer_library_count)) {
                skip |= LogError(device, "VUID-VkPipelineLibraryCreateInfoKHR-pLibraries-08096",
                                 "vkCreateRayTracingPipelinesKHR(): All or none of the elements of pCreateInfo[%" PRIu32
                                 "].pLibraryInfo->pLibraries must be created "
                                 "with VK_PIPELINE_CREATE_DESCRIPTOR_BUFFER_BIT_EXT.",
                                 i);
            }
        }
    }

    return skip;
}
