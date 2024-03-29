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

#include <vulkan/vk_enum_string_helper.h>
#include "generated/chassis.h"
#include "core_validation.h"
#include "chassis/chassis_modification_state.h"

bool CoreChecks::ValidateComputePipelineShaderState(const vvl::Pipeline &pipeline, const Location &create_info_loc) const {
    StageCreateInfo stage_create_info(&pipeline);
    return ValidatePipelineShaderStage(stage_create_info, pipeline.stage_states[0], create_info_loc.dot(Field::stage));
}

bool CoreChecks::PreCallValidateCreateComputePipelines(VkDevice device, VkPipelineCache pipelineCache, uint32_t count,
                                                       const VkComputePipelineCreateInfo *pCreateInfos,
                                                       const VkAllocationCallbacks *pAllocator, VkPipeline *pPipelines,
                                                       const ErrorObject &error_obj, PipelineStates &pipeline_states,
                                                       chassis::CreateComputePipelines &chassis_state) const {
    bool skip = StateTracker::PreCallValidateCreateComputePipelines(device, pipelineCache, count, pCreateInfos, pAllocator,
                                                                    pPipelines, error_obj, pipeline_states, chassis_state);
    for (uint32_t i = 0; i < count; i++) {
        const vvl::Pipeline *pipeline = pipeline_states[i].get();
        if (!pipeline) {
            continue;
        }
        const Location create_info_loc = error_obj.location.dot(Field::pCreateInfos, i);
        skip |= ValidateComputePipelineShaderState(*pipeline, create_info_loc);
        skip |= ValidateShaderModuleId(*pipeline, create_info_loc);
        skip |= ValidatePipelineCacheControlFlags(pipeline->create_flags, create_info_loc.dot(Field::flags),
                                                  "VUID-VkComputePipelineCreateInfo-pipelineCreationCacheControl-02875");
        skip |= ValidatePipelineIndirectBindableFlags(pipeline->create_flags, create_info_loc.dot(Field::flags),
                                                      "VUID-VkComputePipelineCreateInfo-flags-09007");

        if (const auto *pipeline_robustness_info =
                vku::FindStructInPNextChain<VkPipelineRobustnessCreateInfoEXT>(pCreateInfos[i].pNext);
            pipeline_robustness_info) {
            skip |= ValidatePipelineRobustnessCreateInfo(*pipeline, *pipeline_robustness_info, create_info_loc);
        }
    }
    return skip;
}
