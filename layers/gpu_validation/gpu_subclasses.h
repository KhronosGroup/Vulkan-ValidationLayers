/* Copyright (c) 2018-2023 The Khronos Group Inc.
 * Copyright (c) 2018-2023 Valve Corporation
 * Copyright (c) 2018-2023 LunarG, Inc.
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

#include <vector>
#include <mutex>

#include "gpu_validation/gpu_descriptor_set.h"
#include "gpu_validation/gpu_state_tracker.h"
#include "generated/vk_object_types.h"
#include "gpu_shaders/gpu_shaders_constants.h"

namespace gpuav {

class Validator;

struct DescSetState {
    uint32_t num;
    std::shared_ptr<DescriptorSet> state;
    BindingVariableMap binding_req;
    // State that will be used by the GPU-AV shader instrumentation
    // For update-after-bind, this will be set during queue submission
    // Otherwise it will be set when the DescriptorSet is bound.
    std::shared_ptr<DescriptorSet::State> gpu_state;
    std::shared_ptr<DescriptorSet::State> output_state;
};

struct DeviceMemoryBlock {
    VkBuffer buffer;
    VmaAllocation allocation;
};

struct DescBindingInfo {
    VkBuffer bindless_state_buffer;
    VmaAllocation bindless_state_buffer_allocation;
    std::vector<DescSetState> descriptor_set_buffers;
};

// State tracking needed to insert a "pre" draw call aimed at validating buffer data used to perform an indirect draw call
struct PreDrawResources {
    VkDescriptorPool desc_pool = VK_NULL_HANDLE;
    VkDescriptorSet desc_set = VK_NULL_HANDLE;
    VkBuffer indirect_buffer = VK_NULL_HANDLE;
    VkDeviceSize indirect_buffer_offset = 0;
    uint32_t indirect_buffer_stride = 0;
    VkDeviceSize indirect_buffer_size = 0;
    static constexpr uint32_t push_constant_words = 4;
};

// State tracking needed to insert a "pre" dispatch call aimed at validating buffer data used to perform an indirect dispatch call
struct PreDispatchResources {
    VkDescriptorPool desc_pool = VK_NULL_HANDLE;
    VkDescriptorSet desc_set = VK_NULL_HANDLE;
    VkBuffer indirect_buffer = VK_NULL_HANDLE;
    VkDeviceSize indirect_buffer_offset = 0;
    static constexpr uint32_t push_constant_words = 4;
};

// State tracking needed to insert a "pre" trace rays call aimed at validating buffer data used to perform an indirect trace rays
// call
struct PreTraceRaysResources {
    VkDescriptorPool desc_pool = VK_NULL_HANDLE;
    VkDescriptorSet desc_set = VK_NULL_HANDLE;
    VkDeviceAddress indirect_device_address = 0;
    static constexpr uint32_t push_constant_words = 5;
};

struct CommonDrawResources {
    // some resources can be used each time so only to need to create once
    bool initialized = false;

    VkShaderModule shader_module = VK_NULL_HANDLE;
    VkDescriptorSetLayout ds_layout = VK_NULL_HANDLE;
    VkPipelineLayout pipeline_layout = VK_NULL_HANDLE;
    vl_concurrent_unordered_map<VkRenderPass, VkPipeline> renderpass_to_pipeline;
    VkShaderEXT shader_object = VK_NULL_HANDLE;

    void Destroy(VkDevice device);
};

struct CommonDispatchResources {
    // some resources can be used each time so only to need to create once
    bool initialized = false;

    VkShaderModule shader_module = VK_NULL_HANDLE;
    VkDescriptorSetLayout ds_layout = VK_NULL_HANDLE;
    VkPipelineLayout pipeline_layout = VK_NULL_HANDLE;
    VkPipeline pipeline = VK_NULL_HANDLE;
    VkShaderEXT shader_object = VK_NULL_HANDLE;

    void Destroy(VkDevice device);
};

struct CommonTraceRaysResources {
    // some resources can be used each time so only to need to create once
    bool initialized = false;

    VkShaderModule shader_module = VK_NULL_HANDLE;
    VkDescriptorSetLayout ds_layout = VK_NULL_HANDLE;
    VkPipelineLayout pipeline_layout = VK_NULL_HANDLE;
    VkPipeline pipeline = VK_NULL_HANDLE;
    VmaPool sbt_pool = VK_NULL_HANDLE;
    VkBuffer sbt_buffer = VK_NULL_HANDLE;
    VmaAllocation sbt_allocation = {};
    VkDeviceAddress sbt_address = 0;
    uint32_t shader_group_handle_size_aligned = 0;

    void Destroy(VkDevice device, VmaAllocator &vmaAllocator);
};

struct AccelerationStructureBuildValidationState {
    // some resources can be used each time so only to need to create once
    bool initialized = false;

    VkPipeline pipeline = VK_NULL_HANDLE;
    VkPipelineLayout pipeline_layout = VK_NULL_HANDLE;

    VkAccelerationStructureNV replacement_as = VK_NULL_HANDLE;
    VmaAllocation replacement_as_allocation = VK_NULL_HANDLE;
    uint64_t replacement_as_handle = 0;

    void Destroy(VkDevice device, VmaAllocator &vmaAllocator);
};

// Used for draws/dispatch/traceRays indirect
struct CmdIndirectState {
    VkBuffer buffer;
    VkDeviceSize offset;
    uint32_t draw_count;
    uint32_t stride;
    VkBuffer count_buffer;
    VkDeviceSize count_buffer_offset;
    VkDeviceAddress indirectDeviceAddress;
};

struct CommandInfo {
    DeviceMemoryBlock output_mem_block;
    PreDrawResources draw_resources;
    PreDispatchResources dispatch_resources;
    PreTraceRaysResources trace_rays_resources;
    VkDescriptorSet desc_set;
    VkDescriptorPool desc_pool;
    VkPipelineBindPoint pipeline_bind_point;
    bool uses_robustness;
    vvl::Func command;
    uint32_t desc_binding_index;
    CommandInfo(DeviceMemoryBlock output_mem_block, PreDrawResources draw_resources, PreDispatchResources dispatch_resources,
                PreTraceRaysResources trace_rays_resources, VkDescriptorSet desc_set, VkDescriptorPool desc_pool,
                VkPipelineBindPoint pipeline_bind_point, bool uses_robustness, vvl::Func command, uint32_t desc_binding_index)
        : output_mem_block(output_mem_block),
          draw_resources(draw_resources),
          dispatch_resources(dispatch_resources),
          trace_rays_resources(trace_rays_resources),
          desc_set(desc_set),
          desc_pool(desc_pool),
          pipeline_bind_point(pipeline_bind_point),
          uses_robustness(uses_robustness),
          command(command),
          desc_binding_index(desc_binding_index){};
};

struct AccelerationStructureBuildValidationInfo {
    // The acceleration structure that is being built.
    VkAccelerationStructureNV acceleration_structure = VK_NULL_HANDLE;

    // The descriptor pool and descriptor set being used to validate a given build.
    VkDescriptorPool descriptor_pool = VK_NULL_HANDLE;
    VkDescriptorSet descriptor_set = VK_NULL_HANDLE;

    // The storage buffer used by the validating compute shader which contains info about
    // the valid handles and which is written to communicate found invalid handles.
    VkBuffer buffer = VK_NULL_HANDLE;
    VmaAllocation buffer_allocation = VK_NULL_HANDLE;
};

class CommandBuffer : public gpu_tracker::CommandBuffer {
  public:
    // per draw/dispatch command state
    std::vector<CommandInfo> per_draw_command_infos;
    // per vkCmdBindDescriptorSet() state
    std::vector<DescBindingInfo> di_input_buffer_list;
    std::vector<AccelerationStructureBuildValidationInfo> as_validation_buffers;
    VkBuffer current_bindless_buffer = VK_NULL_HANDLE;

    CommandBuffer(Validator *ga, VkCommandBuffer cb, const VkCommandBufferAllocateInfo *pCreateInfo,
                  const COMMAND_POOL_STATE *pool);
    ~CommandBuffer();

    bool NeedsProcessing() const final { return !per_draw_command_infos.empty() || has_build_as_cmd; }
    void Process(VkQueue queue, const Location &loc) final;

    void Destroy() final;
    void Reset() final;

  private:
    void ResetCBState();
    void ProcessAccelerationStructure(VkQueue queue);
};

class Buffer : public BUFFER_STATE {
  public:
    Buffer(ValidationStateTracker *dev_data, VkBuffer buff, const VkBufferCreateInfo *pCreateInfo, DescriptorHeap &desc_heap_);

    void Destroy() final;
    void NotifyInvalidate(const NodeList &invalid_nodes, bool unlink) final;

    DescriptorHeap &desc_heap;
    const DescriptorId id;
};

class BufferView : public BUFFER_VIEW_STATE {
  public:
    BufferView(const std::shared_ptr<BUFFER_STATE> &bf, VkBufferView bv, const VkBufferViewCreateInfo *ci,
               VkFormatFeatureFlags2KHR buf_ff, DescriptorHeap &desc_heap_);

    void Destroy() final;
    void NotifyInvalidate(const NodeList &invalid_nodes, bool unlink) final;

    DescriptorHeap &desc_heap;
    const DescriptorId id;
};

class ImageView : public IMAGE_VIEW_STATE {
  public:
    ImageView(const std::shared_ptr<IMAGE_STATE> &image_state, VkImageView iv, const VkImageViewCreateInfo *ci,
              VkFormatFeatureFlags2KHR ff, const VkFilterCubicImageViewImageFormatPropertiesEXT &cubic_props,
              DescriptorHeap &desc_heap_);

    void Destroy() final;
    void NotifyInvalidate(const NodeList &invalid_nodes, bool unlink) final;

    DescriptorHeap &desc_heap;
    const DescriptorId id;
};

class Sampler : public SAMPLER_STATE {
  public:
    Sampler(const VkSampler s, const VkSamplerCreateInfo *pci, DescriptorHeap &desc_heap_);

    void Destroy() final;
    void NotifyInvalidate(const NodeList &invalid_nodes, bool unlink) final;

    DescriptorHeap &desc_heap;
    const DescriptorId id;
};

class AccelerationStructureKHR : public ACCELERATION_STRUCTURE_STATE_KHR {
  public:
    AccelerationStructureKHR(VkAccelerationStructureKHR as, const VkAccelerationStructureCreateInfoKHR *ci,
                             std::shared_ptr<BUFFER_STATE> &&buf_state, VkDeviceAddress address, DescriptorHeap &desc_heap_);

    void Destroy() final;
    void NotifyInvalidate(const NodeList &invalid_nodes, bool unlink) final;

    DescriptorHeap &desc_heap;
    const DescriptorId id;
};

class AccelerationStructureNV : public ACCELERATION_STRUCTURE_STATE_NV {
  public:
    AccelerationStructureNV(VkDevice device, VkAccelerationStructureNV as, const VkAccelerationStructureCreateInfoNV *ci,
                            DescriptorHeap &desc_heap_);

    void Destroy() final;
    void NotifyInvalidate(const NodeList &invalid_nodes, bool unlink) final;

    DescriptorHeap &desc_heap;
    const DescriptorId id;
};

namespace glsl {

struct AccelerationStructureBuildValidationBuffer {
    uint32_t instances_to_validate;
    uint32_t replacement_handle_bits_0;
    uint32_t replacement_handle_bits_1;
    uint32_t invalid_handle_found;
    uint32_t invalid_handle_bits_0;
    uint32_t invalid_handle_bits_1;
    uint32_t valid_handles_count;
};

struct DescriptorSetRecord {
    VkDeviceAddress layout_data;
    VkDeviceAddress in_data;
    VkDeviceAddress out_data;
};

struct BindlessStateBuffer {
    VkDeviceAddress global_state;
    DescriptorSetRecord desc_sets[kDebugInputBindlessMaxDescSets];
};

}  // namespace glsl

}  // namespace gpuav
