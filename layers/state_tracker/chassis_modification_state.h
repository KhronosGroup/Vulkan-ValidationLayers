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
#include <unordered_map>
#include <vector>
#include "state_tracker/shader_module.h"

namespace vvl {
class Pipeline;
}  // namespace vvl

// These structure are here as a way to bridge information down the chassis.
// This allows the 4 different calls (PreCallValidate, PreCallRecord, Dispatch, PostCallRecord) to share information
namespace chassis {

struct CreateShaderModule {
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

// VkShaderEXT (VK_EXT_shader_object)
struct ShaderObject {
    // allows PreCallRecord to return a value like PreCallValidate
    bool skip = false;

    std::vector<std::shared_ptr<spirv::Module>> module_states;  // contains SPIR-V to validate
    std::vector<spirv::StatelessData> stateless_data;
    std::vector<uint32_t> unique_shader_ids;

    // Pass the instrumented SPIR-V info from PreCallRecord to Dispatch (so GPU-AV logic can run with it)
    VkShaderCreateInfoEXT* instrumented_create_info;
    std::vector<std::vector<uint32_t>> instrumented_spirv;

    std::vector<VkDescriptorSetLayout> new_layouts;

    ShaderObject(uint32_t createInfoCount, const VkShaderCreateInfoEXT* pCreateInfos) {
        instrumented_create_info = const_cast<VkShaderCreateInfoEXT*>(pCreateInfos);
        module_states.resize(createInfoCount);
        stateless_data.resize(createInfoCount);
        unique_shader_ids.resize(createInfoCount);
        instrumented_spirv.resize(createInfoCount);
    }
};

// We hold one slot for each potential graphics stage
// This is used to pass the unique_shader_id around for GPL
// Tried to use `vvl::unordered_map` but caused compiler errors
using ShaderModuleUniqueIds = std::unordered_map<VkShaderStageFlagBits, uint32_t>;

struct CreateGraphicsPipelines {
    std::vector<vku::safe_VkGraphicsPipelineCreateInfo> modified_create_infos;
    std::vector<ShaderModuleUniqueIds> shader_unique_id_maps;
    const VkGraphicsPipelineCreateInfo* pCreateInfos;
};

struct CreateComputePipelines {
    std::vector<vku::safe_VkComputePipelineCreateInfo> modified_create_infos;
    std::vector<ShaderModuleUniqueIds> shader_unique_id_maps;  // not used, here for template function
    const VkComputePipelineCreateInfo* pCreateInfos;
};

struct CreateRayTracingPipelinesNV {
    std::vector<vku::safe_VkRayTracingPipelineCreateInfoCommon> modified_create_infos;
    std::vector<ShaderModuleUniqueIds> shader_unique_id_maps;  // not used, here for template function
    const VkRayTracingPipelineCreateInfoNV* pCreateInfos;
};

struct CreateRayTracingPipelinesKHR {
    std::vector<vku::safe_VkRayTracingPipelineCreateInfoCommon> modified_create_infos;
    std::vector<ShaderModuleUniqueIds> shader_unique_id_maps;  // not used, here for template function
    const VkRayTracingPipelineCreateInfoKHR* pCreateInfos;
};

struct CreatePipelineLayout {
    // This currently only works because GPU-AV is the only layer who creates this state
    // If a 2nd layer starts to use it, can have conflicting values
    std::vector<VkDescriptorSetLayout> new_layouts;
    VkPipelineLayoutCreateInfo modified_create_info;
};

struct CreateBuffer {
    VkBufferCreateInfo modified_create_info;
};

}  // namespace chassis
