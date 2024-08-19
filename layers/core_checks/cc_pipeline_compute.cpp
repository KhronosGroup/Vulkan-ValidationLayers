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

bool CoreChecks::PreCallValidateCreateComputePipelines(VkDevice device, VkPipelineCache pipelineCache, uint32_t count,
                                                       const VkComputePipelineCreateInfo *pCreateInfos,
                                                       const VkAllocationCallbacks *pAllocator, VkPipeline *pPipelines,
                                                       const ErrorObject &error_obj, PipelineStates &pipeline_states,
                                                       chassis::CreateComputePipelines &chassis_state) const {
    bool skip = StateTracker::PreCallValidateCreateComputePipelines(device, pipelineCache, count, pCreateInfos, pAllocator,
                                                                    pPipelines, error_obj, pipeline_states, chassis_state);

    skip |= ValidateDeviceQueueSupport(error_obj.location);
    for (uint32_t i = 0; i < count; i++) {
        const vvl::Pipeline *pipeline = pipeline_states[i].get();
        ASSERT_AND_CONTINUE(pipeline);

        const Location create_info_loc = error_obj.location.dot(Field::pCreateInfos, i);
        const Location stage_info = create_info_loc.dot(Field::stage);
        const auto &stage_state = pipeline->stage_states[0];
        skip |= ValidateShaderStage(stage_state, pipeline, stage_info);
        if (stage_state.pipeline_create_info) {
            skip |= ValidatePipelineShaderStage(*pipeline, *stage_state.pipeline_create_info, pCreateInfos[i].pNext, stage_info);
        }

        skip |= ValidatePipelineCacheControlFlags(pipeline->create_flags, create_info_loc.dot(Field::flags),
                                                  "VUID-VkComputePipelineCreateInfo-pipelineCreationCacheControl-02875");
        skip |= ValidatePipelineIndirectBindableFlags(pipeline->create_flags, create_info_loc.dot(Field::flags),
                                                      "VUID-VkComputePipelineCreateInfo-flags-09007");

        if (const auto *pipeline_robustness_info =
                vku::FindStructInPNextChain<VkPipelineRobustnessCreateInfoEXT>(pCreateInfos[i].pNext)) {
            skip |= ValidatePipelineRobustnessCreateInfo(*pipeline, *pipeline_robustness_info, create_info_loc);
        }

        // From dumping traces, we found almost all apps only create one pipeline at a time. To greatly simplify the logic, only
        // check the stateless validation in the pNext chain for the first pipeline. (The core issue is because we parse the SPIR-V
        // at state tracking time, and we state track pipelines first)
        if (i == 0 && chassis_state.stateless_data.pipeline_pnext_module) {
            skip |= ValidateSpirvStateless(*chassis_state.stateless_data.pipeline_pnext_module, chassis_state.stateless_data,
                                           create_info_loc.dot(Field::stage).pNext(Struct::VkShaderModuleCreateInfo, Field::pCode));
        }
    }
    return skip;
}
