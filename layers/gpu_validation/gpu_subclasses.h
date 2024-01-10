/* Copyright (c) 2018-2024 The Khronos Group Inc.
 * Copyright (c) 2018-2024 Valve Corporation
 * Copyright (c) 2018-2024 LunarG, Inc.
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

class Validator;

// Every recorded command need the validation resources listed in this function
// If adding validation for a new command reveals the need to allocate specific resources for it, create a new class that derives
// from this one
class CommandResources {
  public:
    virtual ~CommandResources() {}
    virtual void Destroy(gpuav::Validator &validator);
    CommandResources() = default;
    CommandResources(const CommandResources &) = default;
    CommandResources &operator=(const CommandResources &) = default;

    void LogErrorIfAny(gpuav::Validator &validator, VkQueue queue, VkCommandBuffer cmd_buffer, const uint32_t operation_index);
    // Return true iff an error has been logged
    virtual bool LogValidationMessage(gpuav::Validator &validator, VkQueue queue, VkCommandBuffer cmd_buffer,
                                      const uint32_t *debug_record, const uint32_t operation_index, const LogObjectList &objlist);

    DeviceMemoryBlock output_mem_block;

    VkDescriptorSet output_buffer_desc_set = VK_NULL_HANDLE;
    VkDescriptorPool output_buffer_desc_pool = VK_NULL_HANDLE;
    VkPipelineBindPoint pipeline_bind_point = VK_PIPELINE_BIND_POINT_MAX_ENUM;
    bool uses_robustness = false;  // Only used in AnalyseAndeGenerateMessages, to output using LogWarning instead of LogError. It needs to be removed
    vvl::Func command = vvl::Func::Empty;  // Should probably use Location instead
    uint32_t desc_binding_index = vvl::kU32Max;// desc_binding is only used to help generate an error message
    std::vector<DescBindingInfo> *desc_binding_list = nullptr;
};

class PreDrawResources : public CommandResources {
  public:
    ~PreDrawResources() {}

    VkDescriptorPool desc_pool = VK_NULL_HANDLE;
    // Store a descriptor for the indirect buffer or count buffer
    VkDescriptorSet buffer_desc_set = VK_NULL_HANDLE;
    VkBuffer indirect_buffer = VK_NULL_HANDLE;
    VkDeviceSize indirect_buffer_offset = 0;
    uint32_t indirect_buffer_stride = 0;
    VkDeviceSize indirect_buffer_size = 0;
    static constexpr uint32_t push_constant_words = 11;
    bool emit_task_error = false;  // Used to decide between mesh error and task error

    void Destroy(gpuav::Validator &validator) final;
    bool LogValidationMessage(gpuav::Validator &validator, VkQueue queue, VkCommandBuffer cmd_buffer, const uint32_t *debug_record,
                              const uint32_t operation_index, const LogObjectList &objlist);
};

class PreDispatchResources : public CommandResources {
  public:
    ~PreDispatchResources() {}

    VkDescriptorPool desc_pool = VK_NULL_HANDLE;
    VkDescriptorSet indirect_buffer_desc_set = VK_NULL_HANDLE;
    VkBuffer indirect_buffer = VK_NULL_HANDLE;
    VkDeviceSize indirect_buffer_offset = 0;
    static constexpr uint32_t push_constant_words = 4;

    void Destroy(gpuav::Validator &validator) final;
    bool LogValidationMessage(gpuav::Validator &validator, VkQueue queue, VkCommandBuffer cmd_buffer, const uint32_t *debug_record,
                              const uint32_t operation_index, const LogObjectList &objlist);
};

class PreTraceRaysResources : public CommandResources {
  public:
    ~PreTraceRaysResources() {}

    VkDescriptorPool desc_pool = VK_NULL_HANDLE;
    VkDescriptorSet desc_set = VK_NULL_HANDLE;
    VkDeviceAddress indirect_data_address = 0;
    static constexpr uint32_t push_constant_words = 5;

    void Destroy(gpuav::Validator &validator) final;
    bool LogValidationMessage(gpuav::Validator &validator, VkQueue queue, VkCommandBuffer cmd_buffer, const uint32_t *debug_record,
                              const uint32_t operation_index, const LogObjectList &objlist);
};

class CommandBuffer : public gpu_tracker::CommandBuffer {
  public:
    // per validated command state
    std::vector<std::unique_ptr<CommandResources>> per_command_resources;
    // per vkCmdBindDescriptorSet() state
    std::vector<DescBindingInfo> di_input_buffer_list;
    std::vector<AccelerationStructureBuildValidationInfo> as_validation_buffers;
    VkBuffer current_bindless_buffer = VK_NULL_HANDLE;

    CommandBuffer(Validator *ga, VkCommandBuffer cb, const VkCommandBufferAllocateInfo *pCreateInfo, const vvl::CommandPool *pool);
    ~CommandBuffer();

    bool NeedsProcessing() const final { return !per_command_resources.empty() || has_build_as_cmd; }
    void Process(VkQueue queue, const Location &loc) final;

    void Destroy() final;
    void Reset() final;

  private:
    void ResetCBState();
    void ProcessAccelerationStructure(VkQueue queue);
};

class Buffer : public vvl::Buffer {
  public:
    Buffer(ValidationStateTracker *dev_data, VkBuffer buff, const VkBufferCreateInfo *pCreateInfo, DescriptorHeap &desc_heap_);

    void Destroy() final;
    void NotifyInvalidate(const NodeList &invalid_nodes, bool unlink) final;

    DescriptorHeap &desc_heap;
    const DescriptorId id;
};

class BufferView : public vvl::BufferView {
  public:
    BufferView(const std::shared_ptr<vvl::Buffer> &bf, VkBufferView bv, const VkBufferViewCreateInfo *ci,
               VkFormatFeatureFlags2KHR buf_ff, DescriptorHeap &desc_heap_);

    void Destroy() final;
    void NotifyInvalidate(const NodeList &invalid_nodes, bool unlink) final;

    DescriptorHeap &desc_heap;
    const DescriptorId id;
};

class ImageView : public vvl::ImageView {
  public:
    ImageView(const std::shared_ptr<vvl::Image> &image_state, VkImageView iv, const VkImageViewCreateInfo *ci,
              VkFormatFeatureFlags2KHR ff, const VkFilterCubicImageViewImageFormatPropertiesEXT &cubic_props,
              DescriptorHeap &desc_heap_);

    void Destroy() final;
    void NotifyInvalidate(const NodeList &invalid_nodes, bool unlink) final;

    DescriptorHeap &desc_heap;
    const DescriptorId id;
};

class Sampler : public vvl::Sampler {
  public:
    Sampler(const VkSampler s, const VkSamplerCreateInfo *pci, DescriptorHeap &desc_heap_);

    void Destroy() final;
    void NotifyInvalidate(const NodeList &invalid_nodes, bool unlink) final;

    DescriptorHeap &desc_heap;
    const DescriptorId id;
};

class AccelerationStructureKHR : public vvl::AccelerationStructureKHR {
  public:
    AccelerationStructureKHR(VkAccelerationStructureKHR as, const VkAccelerationStructureCreateInfoKHR *ci,
                             std::shared_ptr<vvl::Buffer> &&buf_state, VkDeviceAddress address, DescriptorHeap &desc_heap_);

    void Destroy() final;
    void NotifyInvalidate(const NodeList &invalid_nodes, bool unlink) final;

    DescriptorHeap &desc_heap;
    const DescriptorId id;
};

class AccelerationStructureNV : public vvl::AccelerationStructureNV {
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
