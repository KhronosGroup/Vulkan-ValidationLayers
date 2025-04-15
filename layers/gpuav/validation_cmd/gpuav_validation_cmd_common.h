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

#include <utility>
#include <vector>
#include <vulkan/utility/vk_safe_struct.hpp>
#include "containers/limits.h"
#include "generated/dispatch_functions.h"
#include "gpuav/core/gpuav.h"
#include "state_tracker/cmd_buffer_state.h"

namespace vvl {
struct ShaderObject;
}  // namespace vvl

namespace gpuav {
class Validator;
class CommandBufferSubState;

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

struct BoundStorageBuffer {
    uint32_t binding = vvl::kU32Max;
    VkDescriptorBufferInfo info{VK_NULL_HANDLE, vvl::kU64Max, 0};
};

VkDescriptorSet GetDescriptorSetHelper(CommandBufferSubState& cb_state, VkDescriptorSetLayout desc_set_layout);
void BindShaderResourcesHelper(Validator& gpuav, CommandBufferSubState& cb_state, uint32_t cmd_index, uint32_t error_logger_index,
                               VkPipelineLayout pipeline_layout, VkDescriptorSet desc_set, uint32_t desc_set_id,
                               const std::vector<VkWriteDescriptorSet>& descriptor_writes, const uint32_t push_constants_byte_size,
                               const void* push_constants);

template <typename ShaderResources>
struct ComputeValidationPipeline {
    ComputeValidationPipeline(Validator& gpuav, const Location& loc, VkDescriptorSetLayout error_output_desc_set_layout) {
        std::vector<VkDescriptorSetLayoutBinding> specific_bindings = ShaderResources::GetDescriptorSetLayoutBindings();

        VkPushConstantRange push_constant_range = {};
        push_constant_range.stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;
        push_constant_range.offset = 0;
        push_constant_range.size = sizeof(ShaderResources::push_constants);

        device = gpuav.device;
        VkDescriptorSetLayoutCreateInfo ds_layout_ci = vku::InitStructHelper();

        ds_layout_ci.bindingCount = static_cast<uint32_t>(specific_bindings.size());
        ds_layout_ci.pBindings = specific_bindings.data();
        VkResult result = DispatchCreateDescriptorSetLayout(device, &ds_layout_ci, nullptr, &specific_desc_set_layout);
        if (result != VK_SUCCESS) {
            gpuav.InternalError(device, loc, "Unable to create descriptor set layout for SharedDrawValidationResources.");
            return;
        }

        std::array<VkDescriptorSetLayout, 2> set_layouts = {{error_output_desc_set_layout, specific_desc_set_layout}};
        VkPipelineLayoutCreateInfo pipeline_layout_ci = vku::InitStructHelper();
        // Any push constants byte size below 4 is illegal. Can come from empty push constant struct
        if (push_constant_range.size >= 4) {
            pipeline_layout_ci.pushConstantRangeCount = 1;
            pipeline_layout_ci.pPushConstantRanges = &push_constant_range;
        }
        pipeline_layout_ci.setLayoutCount = static_cast<uint32_t>(set_layouts.size());
        pipeline_layout_ci.pSetLayouts = set_layouts.data();
        result = DispatchCreatePipelineLayout(device, &pipeline_layout_ci, nullptr, &pipeline_layout);
        if (result != VK_SUCCESS) {
            gpuav.InternalError(device, loc, "Unable to create pipeline layout for SharedDrawValidationResources.");
            return;
        }

        VkShaderModuleCreateInfo shader_module_ci = vku::InitStructHelper();
        shader_module_ci.codeSize = ShaderResources::GetSpirvSize();
        shader_module_ci.pCode = ShaderResources::GetSpirv();
        result = DispatchCreateShaderModule(device, &shader_module_ci, nullptr, &shader_module);
        if (result != VK_SUCCESS) {
            gpuav.InternalError(device, loc, "Unable to create shader module.");
            return;
        }

        VkComputePipelineCreateInfo compute_validation_pipeline_ci = vku::InitStructHelper();
        compute_validation_pipeline_ci.stage = vku::InitStructHelper();
        compute_validation_pipeline_ci.stage.stage = VK_SHADER_STAGE_COMPUTE_BIT;
        compute_validation_pipeline_ci.stage.module = shader_module;
        compute_validation_pipeline_ci.stage.pName = "main";
        compute_validation_pipeline_ci.layout = pipeline_layout;
        result = DispatchCreateComputePipelines(device, VK_NULL_HANDLE, 1, &compute_validation_pipeline_ci, nullptr, &pipeline);
        if (result != VK_SUCCESS) {
            gpuav.InternalError(device, loc, "Unable to create compute validation pipeline.");
            return;
        }

        valid = true;
    }

    ~ComputeValidationPipeline() {
        if (pipeline != VK_NULL_HANDLE) {
            DispatchDestroyPipeline(device, pipeline, nullptr);
        }

        if (shader_module != VK_NULL_HANDLE) {
            DispatchDestroyShaderModule(device, shader_module, nullptr);
        }

        if (specific_desc_set_layout != VK_NULL_HANDLE) {
            DispatchDestroyDescriptorSetLayout(device, specific_desc_set_layout, nullptr);
        }

        if (pipeline_layout != VK_NULL_HANDLE) {
            DispatchDestroyPipelineLayout(device, pipeline_layout, nullptr);
        }
    }

    [[nodiscard]] bool BindShaderResources(Validator& gpuav, CommandBufferSubState& cb_state, uint32_t cmd_index,
                                           uint32_t error_logger_index, const ShaderResources& shader_resources) {
        const VkDescriptorSet desc_set = GetDescriptorSetHelper(cb_state, specific_desc_set_layout);
        if (!desc_set) {
            return false;
        }
        const std::vector<VkWriteDescriptorSet> desc_writes = shader_resources.GetDescriptorWrites(desc_set);
        BindShaderResourcesHelper(gpuav, cb_state, cmd_index, error_logger_index, pipeline_layout, desc_set,
                                  shader_resources.desc_set_id, desc_writes, sizeof(shader_resources.push_constants),
                                  &shader_resources.push_constants);
        return true;
    }

    VkDevice device = VK_NULL_HANDLE;
    VkDescriptorSetLayout specific_desc_set_layout = VK_NULL_HANDLE;
    VkPipelineLayout pipeline_layout = VK_NULL_HANDLE;
    VkShaderModule shader_module = VK_NULL_HANDLE;
    VkPipeline pipeline = VK_NULL_HANDLE;
    bool valid = false;
};

}  // namespace gpuav
