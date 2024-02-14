/* Copyright (c) 2015-2024 The Khronos Group Inc.
 * Copyright (c) 2015-2024 Valve Corporation
 * Copyright (c) 2015-2024 LunarG, Inc.
 * Copyright (C) 2015-2024 Google Inc.
 * Modifications Copyright (C) 2020 Advanced Micro Devices, Inc. All rights reserved.
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

#pragma once
#include <array>
#include <vector>
#include "state_tracker/shader_module.h"

namespace vvl {
class Pipeline;
}  // namespace vvl

// This structure is used modify and pass parameters for the CreateShaderModule down-chain API call
struct create_shader_module_api_state {
    // allows PreCallRecord to return a value like PreCallValidate
    bool skip = false;

    // We build a spirv::Module at PreCallRecord time were we can do basic validation of the SPIR-V (which can crash drivers
    // if passed in the Dispatch). It is then passed to PostCallRecord to save in state tracking so it can be used at Pipeline
    // creation time where the rest of the information is needed to do the remaining SPIR-V validation.
    std::shared_ptr<spirv::Module> module_state;  // contains SPIR-V to validate
    spirv::StatelessData stateless_data;

    uint32_t unique_shader_id = 0;

    // Pass the instrumented SPIR-V info from PreCallRecord to Dispatch (so GPU-AV logic can run with it)
    VkShaderModuleCreateInfo instrumented_create_info;
    std::vector<uint32_t> instrumented_spirv;
};

// same idea as create_shader_module_api_state but for VkShaderEXT (VK_EXT_shader_object)
struct create_shader_object_api_state {
    // allows PreCallRecord to return a value like PreCallValidate
    bool skip = false;

    std::vector<std::shared_ptr<spirv::Module>> module_states;  // contains SPIR-V to validate
    std::vector<spirv::StatelessData> stateless_data;
    std::vector<uint32_t> unique_shader_ids;

    // Pass the instrumented SPIR-V info from PreCallRecord to Dispatch (so GPU-AV logic can run with it)
    VkShaderCreateInfoEXT* instrumented_create_info;
    std::vector<std::vector<uint32_t>> instrumented_spirv;

    std::vector<VkDescriptorSetLayout> new_layouts;

    create_shader_object_api_state(uint32_t createInfoCount, const VkShaderCreateInfoEXT* pCreateInfos) {
        instrumented_create_info = const_cast<VkShaderCreateInfoEXT*>(pCreateInfos);
        module_states.resize(createInfoCount);
        stateless_data.resize(createInfoCount);
        unique_shader_ids.resize(createInfoCount);
        instrumented_spirv.resize(createInfoCount);
    }
};

// This structure is used to save data across the CreateGraphicsPipelines down-chain API call
// CreateShaderModuleStates[i] = ith shader state
using CreateShaderModuleStates = std::array<create_shader_module_api_state, 32>;
struct create_graphics_pipeline_api_state {
    std::vector<safe_VkGraphicsPipelineCreateInfo> modified_create_infos;
    std::vector<std::shared_ptr<vvl::Pipeline>> pipe_state;
    std::vector<CreateShaderModuleStates> shader_states;
    const VkGraphicsPipelineCreateInfo* pCreateInfos;
};

// This structure is used to save data across the CreateComputePipelines down-chain API call
struct create_compute_pipeline_api_state {
    std::vector<safe_VkComputePipelineCreateInfo> modified_create_infos;
    std::vector<std::shared_ptr<vvl::Pipeline>> pipe_state;
    std::vector<CreateShaderModuleStates> shader_states;
    const VkComputePipelineCreateInfo* pCreateInfos;
};

// This structure is used to save data across the CreateRayTracingPipelinesNV down-chain API call.
struct create_ray_tracing_pipeline_api_state {
    std::vector<safe_VkRayTracingPipelineCreateInfoCommon> modified_create_infos;
    std::vector<std::shared_ptr<vvl::Pipeline>> pipe_state;
    std::vector<CreateShaderModuleStates> shader_states;
    const VkRayTracingPipelineCreateInfoNV* pCreateInfos;
};

// This structure is used to save data across the CreateRayTracingPipelinesKHR down-chain API call.
struct create_ray_tracing_pipeline_khr_api_state {
    std::vector<safe_VkRayTracingPipelineCreateInfoCommon> modified_create_infos;
    std::vector<std::shared_ptr<vvl::Pipeline>> pipe_state;
    std::vector<CreateShaderModuleStates> shader_states;
    const VkRayTracingPipelineCreateInfoKHR* pCreateInfos;
};

// This structure is used modify parameters for the CreatePipelineLayout down-chain API call
struct create_pipeline_layout_api_state {
    std::vector<VkDescriptorSetLayout> new_layouts;
    VkPipelineLayoutCreateInfo modified_create_info;
};

// This structure is used modify parameters for the CreateBuffer down-chain API call
struct create_buffer_api_state {
    VkBufferCreateInfo modified_create_info;
};