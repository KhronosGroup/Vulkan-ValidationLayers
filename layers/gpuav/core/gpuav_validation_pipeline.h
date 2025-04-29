/* Copyright (c) 2018-2025 The Khronos Group Inc.
 * Copyright (c) 2018-2025 Valve Corporation
 * Copyright (c) 2018-2025 LunarG, Inc.
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

#include "containers/limits.h"
#include "state_tracker/cmd_buffer_state.h"

#include <vector>
#include <vulkan/vulkan.h>
#include <vulkan/utility/vk_safe_struct.hpp>

struct Location;

namespace vvl {
struct ShaderObject;
}  // namespace vvl

namespace gpuav {

class Validator;
class CommandBufferSubState;

namespace valpipe {

struct BoundStorageBuffer {
    uint32_t binding = vvl::kU32Max;
    VkDescriptorBufferInfo info{VK_NULL_HANDLE, vvl::kU64Max, 0};
};

namespace internal {
[[nodiscard]] bool CreateComputePipelineHelper(Validator& gpuav, const Location& loc,
                                               const std::vector<VkDescriptorSetLayoutBinding> specific_bindings,
                                               VkDescriptorSetLayout additional_desc_set_layout, uint32_t push_constants_byte_size,
                                               uint32_t spirv_size, const uint32_t* spirv, VkDevice& out_device,
                                               VkDescriptorSetLayout& out_specific_descriptor_set_layout,
                                               VkPipelineLayout& out_pipeline_layout, VkShaderModule& out_shader_module,
                                               VkPipeline& out_pipeline);
void DestroyComputePipelineHelper(VkDevice device, VkDescriptorSetLayout specific_descriptor_set_layout,
                                  VkPipelineLayout pipeline_layout, VkShaderModule shader_module, VkPipeline pipeline);

VkDescriptorSet GetDescriptorSetHelper(CommandBufferSubState& cb_state, VkDescriptorSetLayout desc_set_layout);

void BindShaderResourcesHelper(Validator& gpuav, CommandBufferSubState& cb_state, VkPipelineLayout pipeline_layout,
                               VkDescriptorSet desc_set, const std::vector<VkWriteDescriptorSet>& descriptor_writes,
                               const uint32_t push_constants_byte_size, const void* push_constants);
}  // namespace internal

// ComputePipeline<> is an helper class to create compute pipeline used by GPU-AV to setup things for validation.
// When creating such compute pipelines, typically only the compute shader changes, the setup boilerplate is the same. This helper
// only asks for a description of the compute shader, and handles the boilerplate.
// The compute shader description is stored represented by the ShaderResources template argument.
// ComputePipeline<> handles a single descriptor set, its binding number is glsl::kValPipeDescSet.
// For example usage, valpipe::ComputePipeline<SetupDrawCountDispatchIndirectShader> is a good blueprint
template <typename ShaderResources>
class ComputePipeline {
  public:
    ComputePipeline(Validator& gpuav, const Location& loc, VkDescriptorSetLayout additional_desc_set_layout = VK_NULL_HANDLE) {
        std::vector<VkDescriptorSetLayoutBinding> specific_bindings = ShaderResources::GetDescriptorSetLayoutBindings();
        valid = internal::CreateComputePipelineHelper(gpuav, loc, specific_bindings, additional_desc_set_layout,
                                                      sizeof(ShaderResources::push_constants),
                                                      uint32_t(ShaderResources::GetSpirvSize()), ShaderResources::GetSpirv(),
                                                      device, specific_desc_set_layout, pipeline_layout, shader_module, pipeline);
    }

    ~ComputePipeline() {
        internal::DestroyComputePipelineHelper(device, specific_desc_set_layout, pipeline_layout, shader_module, pipeline);
    }

    [[nodiscard]] bool BindShaderResources(Validator& gpuav, CommandBufferSubState& cb_state,
                                           const ShaderResources& shader_resources) {
        const VkDescriptorSet desc_set = internal::GetDescriptorSetHelper(cb_state, specific_desc_set_layout);
        if (!desc_set) {
            return false;
        }
        const std::vector<VkWriteDescriptorSet> desc_writes = shader_resources.GetDescriptorWrites(desc_set);
        internal::BindShaderResourcesHelper(gpuav, cb_state, pipeline_layout, desc_set, desc_writes,
                                            sizeof(shader_resources.push_constants), &shader_resources.push_constants);
        return true;
    }

    VkDevice device = VK_NULL_HANDLE;
    VkDescriptorSetLayout specific_desc_set_layout = VK_NULL_HANDLE;
    VkPipelineLayout pipeline_layout = VK_NULL_HANDLE;
    VkShaderModule shader_module = VK_NULL_HANDLE;
    VkPipeline pipeline = VK_NULL_HANDLE;
    bool valid = false;
};

class RestorablePipelineState {
  public:
    RestorablePipelineState(CommandBufferSubState& cb_state, VkPipelineBindPoint bind_point) : cb_state_(cb_state) {
        Create(cb_state, bind_point);
    }
    ~RestorablePipelineState() { Restore(); }

  private:
    void Create(CommandBufferSubState& cb_state, VkPipelineBindPoint bind_point);
    void Restore() const;

    CommandBufferSubState& cb_state_;
    const vku::safe_VkRenderingInfo* rendering_info_ = nullptr;
    VkPipelineBindPoint pipeline_bind_point_ = VK_PIPELINE_BIND_POINT_MAX_ENUM;
    VkPipeline pipeline_ = VK_NULL_HANDLE;
    VkPipelineLayout desc_set_pipeline_layout_ = VK_NULL_HANDLE;
    std::vector<std::pair<VkDescriptorSet, uint32_t>> descriptor_sets_;
    std::vector<std::vector<uint32_t>> dynamic_offsets_;
    uint32_t push_descriptor_set_index_ = 0;
    std::vector<vku::safe_VkWriteDescriptorSet> push_descriptor_set_writes_;
    std::vector<vvl::PushConstantData> push_constants_data_;
    std::vector<vvl::ShaderObject*> shader_objects_;
};
}  // namespace valpipe
}  // namespace gpuav
