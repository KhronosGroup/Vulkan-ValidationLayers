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

#include <vulkan/vk_enum_string_helper.h>
#include "generated/chassis.h"
#include "core_validation.h"

bool CoreChecks::ValidateRayTracingPipeline(const PIPELINE_STATE &pipeline,
                                            const safe_VkRayTracingPipelineCreateInfoCommon &create_info,
                                            VkPipelineCreateFlags flags, const Location &create_info_loc) const {
    bool skip = false;
    bool isKHR = create_info_loc.function == Func::vkCreateRayTracingPipelinesKHR;

    if (isKHR) {
        if (create_info.maxPipelineRayRecursionDepth > phys_dev_ext_props.ray_tracing_props_khr.maxRayRecursionDepth) {
            skip |=
                LogError("VUID-VkRayTracingPipelineCreateInfoKHR-maxPipelineRayRecursionDepth-03589", device,
                         create_info_loc.dot(Field::maxPipelineRayRecursionDepth),
                         "(%" PRIu32
                         ") must be less than or equal to "
                         "maxRayRecursionDepth (%" PRIu32 ").",
                         create_info.maxPipelineRayRecursionDepth, phys_dev_ext_props.ray_tracing_props_khr.maxRayRecursionDepth);
        }
        if (create_info.pLibraryInfo) {
            for (uint32_t i = 0; i < create_info.pLibraryInfo->libraryCount; ++i) {
                const Location library_info_loc = create_info_loc.dot(Field::pLibraryInfo);
                const Location library_loc = library_info_loc.dot(Field::pLibraries, i);
                const auto library_pipelinestate = Get<PIPELINE_STATE>(create_info.pLibraryInfo->pLibraries[i]);
                const auto &library_create_info = library_pipelinestate->GetCreateInfo<VkRayTracingPipelineCreateInfoKHR>();
                if (library_create_info.maxPipelineRayRecursionDepth != create_info.maxPipelineRayRecursionDepth) {
                    skip |= LogError("VUID-VkRayTracingPipelineCreateInfoKHR-pLibraries-03591", device, library_loc,
                                     "was created with maxPipelineRayRecursionDepth (%" PRIu32 ") which is not equal %s (%" PRIu32
                                     ") .",
                                     library_create_info.maxPipelineRayRecursionDepth,
                                     create_info_loc.dot(Field::maxPipelineRayRecursionDepth).Fields().c_str(),
                                     create_info.maxPipelineRayRecursionDepth);
                }
                if (library_create_info.pLibraryInfo && (library_create_info.pLibraryInterface->maxPipelineRayHitAttributeSize !=
                                                             create_info.pLibraryInterface->maxPipelineRayHitAttributeSize ||
                                                         library_create_info.pLibraryInterface->maxPipelineRayPayloadSize !=
                                                             create_info.pLibraryInterface->maxPipelineRayPayloadSize)) {
                    skip |= LogError("VUID-VkRayTracingPipelineCreateInfoKHR-pLibraryInfo-03593", device, library_loc,
                                     "was created with maxPipelineRayPayloadSize (%" PRIu32
                                     ") and "
                                     "maxPipelineRayHitAttributeSize (%" PRIu32 ") which is not equal to %s values of (%" PRIu32
                                     ") and (%" PRIu32 ").",
                                     library_create_info.pLibraryInterface->maxPipelineRayPayloadSize,
                                     library_create_info.pLibraryInterface->maxPipelineRayHitAttributeSize,
                                     create_info_loc.dot(Field::pLibraryInterface).Fields().c_str(),
                                     create_info.pLibraryInterface->maxPipelineRayPayloadSize,
                                     create_info.pLibraryInterface->maxPipelineRayHitAttributeSize);
                }
                if ((flags & VK_PIPELINE_CREATE_RAY_TRACING_SHADER_GROUP_HANDLE_CAPTURE_REPLAY_BIT_KHR) &&
                    !(library_create_info.flags & VK_PIPELINE_CREATE_RAY_TRACING_SHADER_GROUP_HANDLE_CAPTURE_REPLAY_BIT_KHR)) {
                    skip |=
                        LogError("VUID-VkRayTracingPipelineCreateInfoKHR-flags-03594", device, library_loc,
                                 "was created with %s, which is missing "
                                 "VK_PIPELINE_CREATE_RAY_TRACING_SHADER_GROUP_HANDLE_CAPTURE_REPLAY_BIT_KHR included in %s (%s).",
                                 string_VkPipelineCreateFlags(flags).c_str(), create_info_loc.dot(Field::flags).Fields().c_str(),
                                 string_VkPipelineCreateFlags(library_create_info.flags).c_str());
                }
            }
        }
    } else {
        if (create_info.maxRecursionDepth > phys_dev_ext_props.ray_tracing_props_nv.maxRecursionDepth) {
            skip |= LogError("VUID-VkRayTracingPipelineCreateInfoNV-maxRecursionDepth-03457", device,
                             create_info_loc.dot(Field::maxRecursionDepth),
                             "(%" PRIu32
                             ") must be less than or equal to "
                             "maxRecursionDepth (%" PRIu32 ")",
                             create_info.maxRecursionDepth, phys_dev_ext_props.ray_tracing_props_nv.maxRecursionDepth);
        }
    }
    const auto *groups = create_info.ptr()->pGroups;

    for (uint32_t i = 0; i < pipeline.stage_states.size(); i++) {
        StageCreateInfo stage_create_info(&pipeline);
        skip |= ValidatePipelineShaderStage(stage_create_info, pipeline.stage_states[i], create_info_loc.dot(Field::pStages, i));
    }

    if (const auto *pipeline_robustness_info = vku::FindStructInPNextChain<VkPipelineRobustnessCreateInfoEXT>(create_info.pNext);
        pipeline_robustness_info) {
        skip |= ValidatePipelineRobustnessCreateInfo(pipeline, *pipeline_robustness_info, create_info_loc);
    }

    if ((create_info.flags & VK_PIPELINE_CREATE_LIBRARY_BIT_KHR) == 0) {
        const uint32_t raygen_stages_count = CalcShaderStageCount(pipeline, VK_SHADER_STAGE_RAYGEN_BIT_KHR);
        if (raygen_stages_count == 0) {
            skip |= LogError(
                isKHR ? "VUID-VkRayTracingPipelineCreateInfoKHR-stage-03425" : "VUID-VkRayTracingPipelineCreateInfoNV-stage-06232",
                device, create_info_loc,
                "The stage member of at least one element of pStages must be VK_SHADER_STAGE_RAYGEN_BIT_KHR.");
        }
    }
    if ((flags & VK_PIPELINE_CREATE_RAY_TRACING_SKIP_TRIANGLES_BIT_KHR) != 0 &&
        (flags & VK_PIPELINE_CREATE_RAY_TRACING_SKIP_AABBS_BIT_KHR) != 0) {
        skip |= LogError("VUID-VkRayTracingPipelineCreateInfoKHR-flags-06546", device, create_info_loc.dot(Field::flags),
                         "is %s (contains both SKIP_TRIANGLES and SKIP_AABBS).", string_VkPipelineCreateFlags(flags).c_str());
    }

    for (uint32_t group_index = 0; group_index < create_info.groupCount; group_index++) {
        const auto &group = groups[group_index];
        const Location &group_loc = create_info_loc.dot(Field::pGroups, group_index);

        if (group.type == VK_RAY_TRACING_SHADER_GROUP_TYPE_GENERAL_NV) {
            if (!GroupHasValidIndex(
                    pipeline, group.generalShader,
                    VK_SHADER_STAGE_RAYGEN_BIT_NV | VK_SHADER_STAGE_MISS_BIT_NV | VK_SHADER_STAGE_CALLABLE_BIT_NV)) {
                skip |= LogError(isKHR ? "VUID-VkRayTracingShaderGroupCreateInfoKHR-type-03474"
                                       : "VUID-VkRayTracingShaderGroupCreateInfoNV-type-02413",
                                 device, group_loc.dot(Field::generalShader), "is %" PRIu32 ".", group.generalShader);
            }
            if (group.anyHitShader != VK_SHADER_UNUSED_NV || group.closestHitShader != VK_SHADER_UNUSED_NV ||
                group.intersectionShader != VK_SHADER_UNUSED_NV) {
                skip |= LogError(isKHR ? "VUID-VkRayTracingShaderGroupCreateInfoKHR-type-03475"
                                       : "VUID-VkRayTracingShaderGroupCreateInfoNV-type-02414",
                                 device, group_loc,
                                 "anyHitShader is %" PRIu32 ", closestHitShader is %" PRIu32 ", intersectionShader is %" PRIu32 ".",
                                 group.anyHitShader, group.closestHitShader, group.intersectionShader);
            }
        } else if (group.type == VK_RAY_TRACING_SHADER_GROUP_TYPE_PROCEDURAL_HIT_GROUP_NV) {
            if (!GroupHasValidIndex(pipeline, group.intersectionShader, VK_SHADER_STAGE_INTERSECTION_BIT_NV)) {
                skip |= LogError(isKHR ? "VUID-VkRayTracingShaderGroupCreateInfoKHR-type-03476"
                                       : "VUID-VkRayTracingShaderGroupCreateInfoNV-type-02415",
                                 device, group_loc.dot(Field::intersectionShader), "is %" PRIu32 ".", group.intersectionShader);
            }
        } else if (group.type == VK_RAY_TRACING_SHADER_GROUP_TYPE_TRIANGLES_HIT_GROUP_NV) {
            if (group.intersectionShader != VK_SHADER_UNUSED_NV) {
                skip |= LogError(isKHR ? "VUID-VkRayTracingShaderGroupCreateInfoKHR-type-03477"
                                       : "VUID-VkRayTracingShaderGroupCreateInfoNV-type-02416",
                                 device, group_loc.dot(Field::intersectionShader), "is %" PRIu32 ".", group.intersectionShader);
            }
        }

        if (group.type == VK_RAY_TRACING_SHADER_GROUP_TYPE_PROCEDURAL_HIT_GROUP_NV ||
            group.type == VK_RAY_TRACING_SHADER_GROUP_TYPE_TRIANGLES_HIT_GROUP_NV) {
            if (!GroupHasValidIndex(pipeline, group.anyHitShader, VK_SHADER_STAGE_ANY_HIT_BIT_KHR)) {
                skip |= LogError(isKHR ? "VUID-VkRayTracingShaderGroupCreateInfoKHR-anyHitShader-03479"
                                       : "VUID-VkRayTracingShaderGroupCreateInfoNV-anyHitShader-02418",
                                 device, group_loc.dot(Field::anyHitShader), "is %" PRIu32 ".", group.anyHitShader);
            }
            if (!GroupHasValidIndex(pipeline, group.closestHitShader, VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR)) {
                skip |= LogError(isKHR ? "VUID-VkRayTracingShaderGroupCreateInfoKHR-closestHitShader-03478"
                                       : "VUID-VkRayTracingShaderGroupCreateInfoNV-closestHitShader-02417",
                                 device, group_loc.dot(Field::closestHitShader), "is %" PRIu32 ".", group.closestHitShader);
            }
        }
    }
    return skip;
}

bool CoreChecks::PreCallValidateCreateRayTracingPipelinesNV(VkDevice device, VkPipelineCache pipelineCache, uint32_t count,
                                                            const VkRayTracingPipelineCreateInfoNV *pCreateInfos,
                                                            const VkAllocationCallbacks *pAllocator, VkPipeline *pPipelines,
                                                            const ErrorObject &error_obj, void *crtpl_state_data) const {
    bool skip = StateTracker::PreCallValidateCreateRayTracingPipelinesNV(device, pipelineCache, count, pCreateInfos, pAllocator,
                                                                         pPipelines, error_obj, crtpl_state_data);

    auto *crtpl_state = reinterpret_cast<create_ray_tracing_pipeline_api_state *>(crtpl_state_data);
    for (uint32_t i = 0; i < count; i++) {
        const PIPELINE_STATE *pipeline = crtpl_state->pipe_state[i].get();
        if (!pipeline) {
            continue;
        }
        const Location create_info_loc = error_obj.location.dot(Field::pCreateInfos, i);
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
                skip |= LogError(
                    "VUID-vkCreateRayTracingPipelinesNV-flags-03416", device, create_info_loc,
                    "If the flags member of any element of pCreateInfos contains the "
                    "VK_PIPELINE_CREATE_DERIVATIVE_BIT flag,"
                    "the base pipeline must have been created with the VK_PIPELINE_CREATE_ALLOW_DERIVATIVES_BIT flag set.");
            }
        }
        skip |= ValidateRayTracingPipeline(*pipeline, pipeline->GetCreateInfo<CIType>(), pCreateInfos[i].flags, create_info_loc);
        skip |= ValidateShaderModuleId(*pipeline, create_info_loc);
        skip |= ValidatePipelineCacheControlFlags(pCreateInfos[i].flags, create_info_loc.dot(Field::flags),
                                                  "VUID-VkRayTracingPipelineCreateInfoNV-pipelineCreationCacheControl-02905");
    }
    return skip;
}

bool CoreChecks::PreCallValidateCreateRayTracingPipelinesKHR(VkDevice device, VkDeferredOperationKHR deferredOperation,
                                                             VkPipelineCache pipelineCache, uint32_t count,
                                                             const VkRayTracingPipelineCreateInfoKHR *pCreateInfos,
                                                             const VkAllocationCallbacks *pAllocator, VkPipeline *pPipelines,
                                                             const ErrorObject &error_obj, void *crtpl_state_data) const {
    bool skip = StateTracker::PreCallValidateCreateRayTracingPipelinesKHR(
        device, deferredOperation, pipelineCache, count, pCreateInfos, pAllocator, pPipelines, error_obj, crtpl_state_data);

    auto *crtpl_state = reinterpret_cast<create_ray_tracing_pipeline_khr_api_state *>(crtpl_state_data);
    for (uint32_t i = 0; i < count; i++) {
        const PIPELINE_STATE *pipeline = crtpl_state->pipe_state[i].get();
        if (!pipeline) {
            continue;
        }
        const Location create_info_loc = error_obj.location.dot(Field::pCreateInfos, i);
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
                skip |= LogError(
                    "VUID-vkCreateRayTracingPipelinesKHR-flags-03416", device, create_info_loc,
                    "If the flags member of any element of pCreateInfos contains the "
                    "VK_PIPELINE_CREATE_DERIVATIVE_BIT flag,"
                    "the base pipeline must have been created with the VK_PIPELINE_CREATE_ALLOW_DERIVATIVES_BIT flag set.");
            }
        }
        skip |= ValidateRayTracingPipeline(*pipeline, pipeline->GetCreateInfo<CIType>(), pCreateInfos[i].flags, create_info_loc);
        skip |= ValidateShaderModuleId(*pipeline, create_info_loc);
        skip |= ValidatePipelineCacheControlFlags(pCreateInfos[i].flags, create_info_loc.dot(Field::flags),
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
            bool uses_descriptor_buffer = false;
            for (uint32_t j = 0; j < create_info.pLibraryInfo->libraryCount; ++j) {
                const Location library_info_loc = create_info_loc.dot(Field::pLibraryInfo);
                const Location library_loc = library_info_loc.dot(Field::pLibraries, j);
                const auto lib = Get<PIPELINE_STATE>(create_info.pLibraryInfo->pLibraries[j]);
                if ((lib->create_flags & VK_PIPELINE_CREATE_LIBRARY_BIT_KHR) == 0) {
                    skip |= LogError("VUID-VkPipelineLibraryCreateInfoKHR-pLibraries-03381", device, library_loc,
                                     "was created with %s.", string_VkPipelineCreateFlags(lib->create_flags).c_str());
                }
                for (const auto &pair : vuid_map) {
                    if (pipeline->create_flags & pair.second) {
                        if ((lib->create_flags & pair.second) == 0) {
                            skip |= LogError(pair.first, device, library_loc,
                                             "was created with %s, which is missing %s included in %s (%s).",
                                             string_VkPipelineCreateFlags(lib->create_flags).c_str(),
                                             string_VkPipelineCreateFlags(pair.second).c_str(),
                                             create_info_loc.dot(Field::flags).Fields().c_str(),
                                             string_VkPipelineCreateFlags(pipeline->create_flags).c_str());
                        }
                    }
                }

                if (j == 0) {
                    uses_descriptor_buffer = lib->descriptor_buffer_mode;
                } else if (uses_descriptor_buffer != lib->descriptor_buffer_mode) {
                    skip |= LogError(
                        "VUID-VkPipelineLibraryCreateInfoKHR-pLibraries-08096", device, library_loc,
                        "%s created with VK_PIPELINE_CREATE_DESCRIPTOR_BUFFER_BIT_EXT which is opopposite of pLibraries[0].",
                        lib->descriptor_buffer_mode ? "was" : "was not");
                    break;  // no point keep checking as might have many of same error
                }
            }
        }
    }

    return skip;
}
