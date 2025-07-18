/* Copyright (c) 2015-2025 The Khronos Group Inc.
 * Copyright (c) 2015-2025 Valve Corporation
 * Copyright (c) 2015-2025 LunarG, Inc.
 * Copyright (C) 2015-2025 Google Inc.
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
#include <vulkan/vulkan_core.h>
#include "core_validation.h"
#include "chassis/chassis_modification_state.h"
#include "state_tracker/pipeline_state.h"

bool CoreChecks::ValidateRayTracingPipeline(const vvl::Pipeline &pipeline,
                                            const vku::safe_VkRayTracingPipelineCreateInfoCommon &create_info,
                                            const Location &create_info_loc) const {
    bool skip = false;
    const bool isKHR = create_info_loc.function == Func::vkCreateRayTracingPipelinesKHR;

    const auto *groups = create_info.ptr()->pGroups;
    const VkPipelineCreateFlags2 create_flags = pipeline.create_flags;

    for (uint32_t i = 0; i < pipeline.stage_states.size(); i++) {
        skip |= ValidateShaderStage(pipeline.stage_states[i], &pipeline, create_info_loc.dot(Field::pStages, i));
    }

    if (const auto *pipeline_robustness_info = vku::FindStructInPNextChain<VkPipelineRobustnessCreateInfo>(create_info.pNext)) {
        skip |= ValidatePipelineRobustnessCreateInfo(pipeline, *pipeline_robustness_info, create_info_loc);
    }

    if ((create_flags & VK_PIPELINE_CREATE_LIBRARY_BIT_KHR) == 0) {
        const uint32_t raygen_stages_count = CalcShaderStageCount(pipeline, VK_SHADER_STAGE_RAYGEN_BIT_KHR);
        if (raygen_stages_count == 0) {
            skip |= LogError(
                isKHR ? "VUID-VkRayTracingPipelineCreateInfoKHR-stage-03425" : "VUID-VkRayTracingPipelineCreateInfoNV-stage-06232",
                device, create_info_loc,
                "The stage member of at least one element of pStages must be VK_SHADER_STAGE_RAYGEN_BIT_KHR.");
        }
    }
    if ((create_flags & VK_PIPELINE_CREATE_RAY_TRACING_SKIP_TRIANGLES_BIT_KHR) != 0 &&
        (create_flags & VK_PIPELINE_CREATE_RAY_TRACING_SKIP_AABBS_BIT_KHR) != 0) {
        skip |=
            LogError("VUID-VkRayTracingPipelineCreateInfoKHR-flags-06546", device, pipeline.GetCreateFlagsLoc(create_info_loc),
                     "is %s (contains both SKIP_TRIANGLES and SKIP_AABBS).", string_VkPipelineCreateFlags2(create_flags).c_str());
    }

    for (uint32_t group_index = 0; group_index < create_info.groupCount; group_index++) {
        const auto &group = groups[group_index];
        const Location &group_loc = create_info_loc.dot(Field::pGroups, group_index);

        if (group.type == VK_RAY_TRACING_SHADER_GROUP_TYPE_GENERAL_KHR) {
            if (group.generalShader == VK_SHADER_UNUSED_KHR ||
                !GroupHasValidIndex(
                    pipeline, group.generalShader,
                    VK_SHADER_STAGE_RAYGEN_BIT_KHR | VK_SHADER_STAGE_MISS_BIT_KHR | VK_SHADER_STAGE_CALLABLE_BIT_KHR)) {
                skip |= LogError(isKHR ? "VUID-VkRayTracingShaderGroupCreateInfoKHR-type-03474"
                                       : "VUID-VkRayTracingShaderGroupCreateInfoNV-type-02413",
                                 device, group_loc.dot(Field::generalShader), "is %" PRIu32 ".", group.generalShader);
            }
            if (group.anyHitShader != VK_SHADER_UNUSED_KHR || group.closestHitShader != VK_SHADER_UNUSED_KHR ||
                group.intersectionShader != VK_SHADER_UNUSED_KHR) {
                skip |= LogError(isKHR ? "VUID-VkRayTracingShaderGroupCreateInfoKHR-type-03475"
                                       : "VUID-VkRayTracingShaderGroupCreateInfoNV-type-02414",
                                 device, group_loc,
                                 "anyHitShader is %" PRIu32 ", closestHitShader is %" PRIu32 ", intersectionShader is %" PRIu32 ".",
                                 group.anyHitShader, group.closestHitShader, group.intersectionShader);
            }
        } else if (group.type == VK_RAY_TRACING_SHADER_GROUP_TYPE_PROCEDURAL_HIT_GROUP_KHR) {
            if (group.intersectionShader == VK_SHADER_UNUSED_KHR ||
                !GroupHasValidIndex(pipeline, group.intersectionShader, VK_SHADER_STAGE_INTERSECTION_BIT_KHR)) {
                skip |= LogError(isKHR ? "VUID-VkRayTracingShaderGroupCreateInfoKHR-type-03476"
                                       : "VUID-VkRayTracingShaderGroupCreateInfoNV-type-02415",
                                 device, group_loc.dot(Field::intersectionShader), "is %" PRIu32 ".", group.intersectionShader);
            }
        } else if (group.type == VK_RAY_TRACING_SHADER_GROUP_TYPE_TRIANGLES_HIT_GROUP_KHR) {
            if (group.intersectionShader != VK_SHADER_UNUSED_KHR) {
                skip |= LogError(isKHR ? "VUID-VkRayTracingShaderGroupCreateInfoKHR-type-03477"
                                       : "VUID-VkRayTracingShaderGroupCreateInfoNV-type-02416",
                                 device, group_loc.dot(Field::intersectionShader), "is %" PRIu32 ".", group.intersectionShader);
            }
        }

        if (group.type == VK_RAY_TRACING_SHADER_GROUP_TYPE_PROCEDURAL_HIT_GROUP_KHR ||
            group.type == VK_RAY_TRACING_SHADER_GROUP_TYPE_TRIANGLES_HIT_GROUP_KHR) {
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
                                                            const ErrorObject &error_obj, PipelineStates &pipeline_states) const {
    bool skip = false;

    skip |= ValidateDeviceQueueSupport(error_obj.location);
    for (uint32_t i = 0; i < count; i++) {
        const vvl::Pipeline *pipeline = pipeline_states[i].get();
        ASSERT_AND_CONTINUE(pipeline);

        const Location create_info_loc = error_obj.location.dot(Field::pCreateInfos, i);
        const auto &create_info = pipeline->RayTracingCreateInfo();
        if (pipeline->create_flags & VK_PIPELINE_CREATE_DERIVATIVE_BIT) {
            std::shared_ptr<const vvl::Pipeline> base_pipeline;
            const auto bpi = create_info.basePipelineIndex;
            const auto bph = create_info.basePipelineHandle;
            if (bpi != -1) {
                base_pipeline = pipeline_states[bpi];
            } else if (bph != VK_NULL_HANDLE) {
                base_pipeline = Get<vvl::Pipeline>(bph);
            }
            if (!base_pipeline || !(base_pipeline->create_flags & VK_PIPELINE_CREATE_ALLOW_DERIVATIVES_BIT)) {
                skip |= LogError(
                    "VUID-vkCreateRayTracingPipelinesNV-flags-03416", device, create_info_loc,
                    "If the flags member of any element of pCreateInfos contains the "
                    "VK_PIPELINE_CREATE_DERIVATIVE_BIT flag,"
                    "the base pipeline must have been created with the VK_PIPELINE_CREATE_ALLOW_DERIVATIVES_BIT flag set.");
            }
        }
        skip |= ValidateRayTracingPipeline(*pipeline, create_info, create_info_loc);
        uint32_t stage_index = 0;
        for (const auto &stage_ci : pipeline->shader_stages_ci) {
            skip |= ValidatePipelineShaderStage(*pipeline, stage_ci, pCreateInfos[i].pNext,
                                                create_info_loc.dot(Field::pStages, stage_index++));
        }
        const Location flag_loc = pipeline->GetCreateFlagsLoc(create_info_loc);
        skip |= ValidatePipelineCacheControlFlags(pipeline->create_flags, flag_loc,
                                                  "VUID-VkRayTracingPipelineCreateInfoNV-pipelineCreationCacheControl-02905");

        if (create_info.maxRecursionDepth > phys_dev_ext_props.ray_tracing_props_nv.maxRecursionDepth) {
            skip |= LogError("VUID-VkRayTracingPipelineCreateInfoNV-maxRecursionDepth-03457", device,
                             create_info_loc.dot(Field::maxRecursionDepth),
                             "(%" PRIu32
                             ") must be less than or equal to "
                             "maxRecursionDepth (%" PRIu32 ")",
                             create_info.maxRecursionDepth, phys_dev_ext_props.ray_tracing_props_nv.maxRecursionDepth);
        }
    }
    return skip;
}

bool CoreChecks::PreCallValidateCreateRayTracingPipelinesKHR(VkDevice device, VkDeferredOperationKHR deferredOperation,
                                                             VkPipelineCache pipelineCache, uint32_t count,
                                                             const VkRayTracingPipelineCreateInfoKHR *pCreateInfos,
                                                             const VkAllocationCallbacks *pAllocator, VkPipeline *pPipelines,
                                                             const ErrorObject &error_obj, PipelineStates &pipeline_states,
                                                             chassis::CreateRayTracingPipelinesKHR &) const {
    bool skip = false;

    skip |= ValidateDeviceQueueSupport(error_obj.location);
    skip |= ValidateDeferredOperation(device, deferredOperation, error_obj.location.dot(Field::deferredOperation),
                                      "VUID-vkCreateRayTracingPipelinesKHR-deferredOperation-03678");

    for (uint32_t i = 0; i < count; i++) {
        const vvl::Pipeline *pipeline = pipeline_states[i].get();
        ASSERT_AND_CONTINUE(pipeline);

        const Location create_info_loc = error_obj.location.dot(Field::pCreateInfos, i);
        const auto &create_info = pipeline->RayTracingCreateInfo();
        if (pipeline->create_flags & VK_PIPELINE_CREATE_DERIVATIVE_BIT) {
            std::shared_ptr<const vvl::Pipeline> base_pipeline;
            const auto bpi = create_info.basePipelineIndex;
            const auto bph = create_info.basePipelineHandle;
            if (bpi != -1) {
                base_pipeline = pipeline_states[bpi];
            } else if (bph != VK_NULL_HANDLE) {
                base_pipeline = Get<vvl::Pipeline>(bph);
            }
            if (!base_pipeline || !(base_pipeline->create_flags & VK_PIPELINE_CREATE_ALLOW_DERIVATIVES_BIT)) {
                skip |= LogError(
                    "VUID-vkCreateRayTracingPipelinesKHR-flags-03416", device, create_info_loc,
                    "If the flags member of any element of pCreateInfos contains the "
                    "VK_PIPELINE_CREATE_DERIVATIVE_BIT flag,"
                    "the base pipeline must have been created with the VK_PIPELINE_CREATE_ALLOW_DERIVATIVES_BIT flag set.");
            }
        }

        skip |= ValidateRayTracingPipeline(*pipeline, create_info, create_info_loc);
        uint32_t stage_index = 0;
        for (const auto &stage_ci : pipeline->shader_stages_ci) {
            skip |= ValidatePipelineShaderStage(*pipeline, stage_ci, pCreateInfos[i].pNext,
                                                create_info_loc.dot(Field::pStages, stage_index++));
        }
        const Location flags_loc = pipeline->GetCreateFlagsLoc(create_info_loc);
        skip |= ValidatePipelineCacheControlFlags(pipeline->create_flags, flags_loc,
                                                  "VUID-VkRayTracingPipelineCreateInfoKHR-pipelineCreationCacheControl-02905");

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
            skip |=
                ValidateRayTracingPipelineLibrary(*pipeline, pCreateInfos[i], *create_info.pLibraryInfo->ptr(), create_info_loc);
        }
    }

    return skip;
}

bool CoreChecks::ValidateRayTracingPipelineLibrary(const vvl::Pipeline &pipeline,
                                                   const VkRayTracingPipelineCreateInfoKHR &pipeline_create_info,
                                                   const VkPipelineLibraryCreateInfoKHR &library_create_info,
                                                   const Location &create_info_loc) const {
    bool skip = false;

    constexpr std::array<std::pair<const char *, VkPipelineCreateFlags>, 7> vuid_map = {{
        {"VUID-VkRayTracingPipelineCreateInfoKHR-flags-04718", VK_PIPELINE_CREATE_RAY_TRACING_SKIP_AABBS_BIT_KHR},
        {"VUID-VkRayTracingPipelineCreateInfoKHR-flags-04719", VK_PIPELINE_CREATE_RAY_TRACING_SKIP_TRIANGLES_BIT_KHR},
        {"VUID-VkRayTracingPipelineCreateInfoKHR-flags-04720", VK_PIPELINE_CREATE_RAY_TRACING_NO_NULL_ANY_HIT_SHADERS_BIT_KHR},
        {"VUID-VkRayTracingPipelineCreateInfoKHR-flags-04721", VK_PIPELINE_CREATE_RAY_TRACING_NO_NULL_CLOSEST_HIT_SHADERS_BIT_KHR},
        {"VUID-VkRayTracingPipelineCreateInfoKHR-flags-04722", VK_PIPELINE_CREATE_RAY_TRACING_NO_NULL_INTERSECTION_SHADERS_BIT_KHR},
        {"VUID-VkRayTracingPipelineCreateInfoKHR-flags-04723", VK_PIPELINE_CREATE_RAY_TRACING_NO_NULL_MISS_SHADERS_BIT_KHR},
        {"VUID-VkRayTracingPipelineCreateInfoKHR-flags-07403", VK_PIPELINE_CREATE_RAY_TRACING_OPACITY_MICROMAP_BIT_EXT},
    }};
    bool uses_descriptor_buffer = false;

    // Use "pipeline" to refer to the pipeline be created
    const VkPipelineCreateFlags2 pipeline_create_flags = pipeline.create_flags;
    const Location flags_loc = pipeline.GetCreateFlagsLoc(create_info_loc);
    const Location library_info_loc = create_info_loc.dot(Field::pLibraryInfo);

    for (uint32_t i = 0; i < library_create_info.libraryCount; i++) {
        const Location library_loc = library_info_loc.dot(Field::pLibraries, i);
        const auto lib = Get<vvl::Pipeline>(library_create_info.pLibraries[i]);
        if (!lib) continue;

        if ((lib->create_flags & VK_PIPELINE_CREATE_LIBRARY_BIT_KHR) == 0) {
            skip |= LogError("VUID-VkPipelineLibraryCreateInfoKHR-pLibraries-03381", device, library_loc, "was created with %s.",
                             string_VkPipelineCreateFlags2(lib->create_flags).c_str());
        }
        for (const auto &[vuid, flag] : vuid_map) {
            if (pipeline_create_flags & flag) {
                if ((lib->create_flags & flag) == 0) {
                    skip |= LogError(vuid, device, library_loc, "was created with %s, which is missing %s (%s is %s).",
                                     string_VkPipelineCreateFlags2(lib->create_flags).c_str(),
                                     string_VkPipelineCreateFlags2(flag).c_str(), flags_loc.Fields().c_str(),
                                     string_VkPipelineCreateFlags2(pipeline_create_flags).c_str());
                }
            }
        }

        if (i == 0) {
            uses_descriptor_buffer = lib->descriptor_buffer_mode;
        } else if (uses_descriptor_buffer != lib->descriptor_buffer_mode) {
            skip |= LogError("VUID-VkPipelineLibraryCreateInfoKHR-pLibraries-08096", device, library_loc,
                             "%s created with VK_PIPELINE_CREATE_DESCRIPTOR_BUFFER_BIT_EXT which is opposite of pLibraries[0].",
                             lib->descriptor_buffer_mode ? "was" : "was not");
            break;  // no point keep checking as might have many of same error
        }

        const auto &lib_create_info = lib->RayTracingCreateInfo();
        if (lib_create_info.maxPipelineRayRecursionDepth != pipeline_create_info.maxPipelineRayRecursionDepth) {
            skip |= LogError("VUID-VkRayTracingPipelineCreateInfoKHR-pLibraries-03591", device, library_loc,
                             "was created with maxPipelineRayRecursionDepth (%" PRIu32 ") which is not equal %s (%" PRIu32 ") .",
                             lib_create_info.maxPipelineRayRecursionDepth,
                             create_info_loc.dot(Field::maxPipelineRayRecursionDepth).Fields().c_str(),
                             pipeline_create_info.maxPipelineRayRecursionDepth);
        }
        if (lib_create_info.pLibraryInfo && pipeline_create_info.pLibraryInterface) {
            const auto &pipeline_interface = *pipeline_create_info.pLibraryInterface;
            const auto &lib_interface = *lib_create_info.pLibraryInterface;

            if (lib_interface.maxPipelineRayHitAttributeSize != pipeline_interface.maxPipelineRayHitAttributeSize ||
                lib_interface.maxPipelineRayPayloadSize != pipeline_interface.maxPipelineRayPayloadSize) {
                skip |= LogError("VUID-VkRayTracingPipelineCreateInfoKHR-pLibraryInfo-03593", device, library_loc,
                                 "was created with maxPipelineRayPayloadSize (%" PRIu32
                                 ") and "
                                 "maxPipelineRayHitAttributeSize (%" PRIu32 ") which is not equal to %s values of (%" PRIu32
                                 ") and (%" PRIu32 ").",
                                 lib_interface.maxPipelineRayPayloadSize, lib_interface.maxPipelineRayHitAttributeSize,
                                 create_info_loc.dot(Field::pLibraryInterface).Fields().c_str(),
                                 pipeline_interface.maxPipelineRayPayloadSize, pipeline_interface.maxPipelineRayHitAttributeSize);
            }
            if ((pipeline_create_flags & VK_PIPELINE_CREATE_RAY_TRACING_SHADER_GROUP_HANDLE_CAPTURE_REPLAY_BIT_KHR) &&
                !(lib->create_flags & VK_PIPELINE_CREATE_RAY_TRACING_SHADER_GROUP_HANDLE_CAPTURE_REPLAY_BIT_KHR)) {
                skip |= LogError("VUID-VkRayTracingPipelineCreateInfoKHR-flags-03594", device, library_loc,
                                 "was created with %s, which is missing "
                                 "VK_PIPELINE_CREATE_RAY_TRACING_SHADER_GROUP_HANDLE_CAPTURE_REPLAY_BIT_KHR included in %s (%s).",
                                 string_VkPipelineCreateFlags2(pipeline_create_flags).c_str(), flags_loc.Fields().c_str(),
                                 string_VkPipelineCreateFlags2(lib->create_flags).c_str());
            }
        }
    }

    return skip;
}