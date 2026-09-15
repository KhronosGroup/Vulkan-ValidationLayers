/* Copyright (c) 2018-2026 The Khronos Group Inc.
 * Copyright (c) 2018-2026 Valve Corporation
 * Copyright (c) 2018-2026 LunarG, Inc.
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

#include "containers/container_utils.h"
#include "containers/limits.h"
#include "state_tracker/cmd_buffer_state.h"
#include "state_tracker/descriptor_mode.h"
#include "state_tracker/push_constant_data.h"
#include "vulkan/vulkan_core.h"

#include <cstdint>
#include <optional>
#include <vector>
#include <vulkan/vulkan.h>
#include <vulkan/utility/vk_safe_struct.hpp>

struct Location;

namespace vvl {
struct ShaderObject;
class CommandBuffer;
}  // namespace vvl

namespace gpuav {

class Validator;
class CommandBufferSubState;

namespace valpipe {

struct BoundStorageBuffer {
    uint32_t binding = vvl::kNoIndex32;
    VkDescriptorBufferInfo info{VK_NULL_HANDLE, vvl::kU64Max, 0};
};

struct ErrorLogging {
    uint32_t cmd_i = vvl::kNoIndex32;
    uint32_t error_logger_i = vvl::kNoIndex32;
};

namespace internal {
void CreateDescSetComputePipelineHelper(Validator& gpuav, const Location& loc,
                                        const std::vector<VkDescriptorSetLayoutBinding>& specific_bindings,
                                        VkDescriptorSetLayout additional_desc_set_layout, uint32_t push_constants_byte_size,
                                        uint32_t spirv_size, const uint32_t* spirv, VkDevice& out_device,
                                        VkDescriptorSetLayout& out_specific_descriptor_set_layout,
                                        VkPipelineLayout& out_pipeline_layout, VkShaderModule& out_shader_module,
                                        VkPipeline& out_pipeline);

void CreateDescHeapComputePipelineHelper(Validator& gpuav, const Location& loc,
                                         const std::vector<VkDescriptorSetLayoutBinding>& specific_bindings,
                                         bool add_error_logging_descriptors, uint32_t push_data_byte_size, uint32_t spirv_size,
                                         const uint32_t* spirv, std::optional<VkShaderModule>& out_shader_module,
                                         std::optional<VkPipeline>& out_pipeline);

void DestroyComputePipelineHelper(VkDevice device, VkDescriptorSetLayout specific_descriptor_set_layout,
                                  VkPipelineLayout pipeline_layout, VkShaderModule shader_module, VkPipeline pipeline,
                                  std::optional<VkShaderModule> desc_heap_shader_module,
                                  std::optional<VkPipeline> desc_heap_pipeline);

vvl::DescriptorMode GetCbLastUsedDescriptorMode(CommandBufferSubState& cb_state);
VkDescriptorSet GetDescriptorSetHelper(CommandBufferSubState& cb_state, VkDescriptorSetLayout desc_set_layout);

void BindDescSetShaderResourcesHelper(Validator& gpuav, CommandBufferSubState& cb_state, VkPipelineLayout pipeline_layout,
                                      VkDescriptorSet desc_set, const std::vector<VkWriteDescriptorSet>& descriptor_writes,
                                      const uint32_t push_constants_byte_size, const void* push_constants,
                                      const std::optional<ErrorLogging>& error_logging);

void BindDescHeapShaderResourcesHelper(Validator& gpuav, CommandBufferSubState& cb_state,
                                       const std::vector<VkWriteDescriptorSet>& descriptor_writes,
                                       const uint32_t push_constants_byte_size, const void* push_constants,
                                       const std::optional<ErrorLogging>& error_logging);

void BindPushConstants(Validator& gpuav, CommandBufferSubState& cb_state, VkPipelineLayout pipeline_layout,
                       vvl::DescriptorMode descriptor_mode, const uint32_t push_constants_byte_size, const void* push_constants);

bool BindComputePipeline(Validator* gpuav, vvl::CommandBuffer& cb_state, VkPipeline pipeline);
}  // namespace internal

// ComputePipeline<> is an helper class to create compute pipeline used by GPU-AV to setup things for validation.
// The way descriptors are managed internally is *fixed* at construction time.
// When creating such compute pipelines, typically only the compute shader changes, the setup boilerplate is the same. This helper
// only asks for a description of the compute shader, and handles the boilerplate.
// The compute shader description is stored represented by the ShaderResources template argument.
// ComputePipeline<> handles a single descriptor set, its binding number is glsl::kValPipeDescSet.
// For example usage, valpipe::ComputePipeline<SetupDrawCountDispatchIndirectShader> is a good blueprint
template <typename ShaderResources>
class ComputePipeline {
  public:
    ComputePipeline(Validator& gpuav, const Location& loc, VkDescriptorSetLayout error_logging_desc_set = VK_NULL_HANDLE) {
        std::vector<VkDescriptorSetLayoutBinding> specific_bindings = ShaderResources::GetDescriptorSetLayoutBindings();

        internal::CreateDescSetComputePipelineHelper(
            gpuav, loc, specific_bindings, error_logging_desc_set, sizeof(ShaderResources::push_constants),
            uint32_t(ShaderResources::GetSpirvSize()), ShaderResources::GetSpirv(), device, specific_desc_set_layout,
            desc_set_pipeline_layout, desc_set_shader_module, desc_set_pipeline);

        internal::CreateDescHeapComputePipelineHelper(
            gpuav, loc, specific_bindings, error_logging_desc_set != VK_NULL_HANDLE, sizeof(ShaderResources::push_constants),
            uint32_t(ShaderResources::GetSpirvSize()), ShaderResources::GetSpirv(), desc_heap_shader_module, desc_heap_pipeline);
    }

    ~ComputePipeline() {
        internal::DestroyComputePipelineHelper(device, specific_desc_set_layout, desc_set_pipeline_layout, desc_set_shader_module,
                                               desc_set_pipeline, desc_heap_shader_module, desc_heap_pipeline);
    }

    bool Valid() const {
        return desc_set_pipeline != VK_NULL_HANDLE && (desc_heap_pipeline == std::nullopt || *desc_heap_pipeline != VK_NULL_HANDLE);
    }

    [[nodiscard]] bool BindShaderResources(Validator& gpuav, CommandBufferSubState& cb_state,
                                           const ShaderResources& shader_resources,
                                           const std::optional<ErrorLogging>& error_logging = std::nullopt) {
        const vvl::DescriptorMode descriptor_mode = internal::GetCbLastUsedDescriptorMode(cb_state);
        if (IsValueIn(descriptor_mode, {vvl::DescriptorModeClassic, vvl::DescriptorModeUnknown})) {
            std::vector<VkWriteDescriptorSet> desc_writes = shader_resources.GetDescriptorWrites();
            VkDescriptorSet desc_set = VK_NULL_HANDLE;
            if (!desc_writes.empty()) {
                desc_set = internal::GetDescriptorSetHelper(cb_state, specific_desc_set_layout);
                if (!desc_set) {
                    return false;
                }
            }
            for (VkWriteDescriptorSet& wds : desc_writes) {
                wds.dstSet = desc_set;
            }

            internal::BindDescSetShaderResourcesHelper(gpuav, cb_state, desc_set_pipeline_layout, desc_set, desc_writes,
                                                       sizeof(shader_resources.push_constants), &shader_resources.push_constants,
                                                       error_logging);
            return true;
        } else if (descriptor_mode == vvl::DescriptorModeHeap) {
            internal::BindDescHeapShaderResourcesHelper(gpuav, cb_state, shader_resources.GetDescriptorWrites(),
                                                        sizeof(shader_resources.push_constants), &shader_resources.push_constants,
                                                        error_logging);
            return true;
        }
        return false;
    }

    void BindPushConstants(Validator& gpuav, CommandBufferSubState& cb_state, vvl::DescriptorMode descriptor_mode,
                           const ShaderResources& shader_resources) {
        internal::BindPushConstants(gpuav, cb_state, desc_set_pipeline_layout, descriptor_mode,
                                    sizeof(shader_resources.push_constants), &shader_resources.push_constants);
    }

    [[nodiscard]] bool BindComputePipeline(Validator& gpuav, vvl::CommandBuffer& cb_state, vvl::DescriptorMode descriptor_mode) {
        const VkPipeline pipe_to_bind =
            IsValueIn(descriptor_mode, {vvl::DescriptorModeClassic, vvl::DescriptorModeUnknown}) ? desc_set_pipeline
            : (descriptor_mode == vvl::DescriptorModeHeap && desc_heap_pipeline.has_value())     ? *desc_heap_pipeline
                                                                                                 : VK_NULL_HANDLE;

        return internal::BindComputePipeline(&gpuav, cb_state, pipe_to_bind);
    }

    [[nodiscard]] bool BindComputePipeline(vvl::CommandBuffer& cb_state, vvl::DescriptorMode descriptor_mode) {
        const VkPipeline pipe_to_bind =
            IsValueIn(descriptor_mode, {vvl::DescriptorModeClassic, vvl::DescriptorModeUnknown}) ? desc_set_pipeline
            : (descriptor_mode == vvl::DescriptorModeHeap && desc_heap_pipeline.has_value())     ? *desc_heap_pipeline
                                                                                                 : VK_NULL_HANDLE;

        return internal::BindComputePipeline(nullptr, cb_state, pipe_to_bind);
    }

  private:
    VkDevice device = VK_NULL_HANDLE;
    VkDescriptorSetLayout specific_desc_set_layout = VK_NULL_HANDLE;
    VkPipelineLayout desc_set_pipeline_layout = VK_NULL_HANDLE;
    VkShaderModule desc_set_shader_module = VK_NULL_HANDLE;
    VkPipeline desc_set_pipeline = VK_NULL_HANDLE;
    std::optional<VkShaderModule> desc_heap_shader_module = std::nullopt;
    std::optional<VkPipeline> desc_heap_pipeline = std::nullopt;
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
    std::vector<PushConstantData> push_constants_data_;
    std::vector<uint8_t> push_data_;
    std::vector<vvl::ShaderObject*> shader_objects_;
};
}  // namespace valpipe
}  // namespace gpuav
