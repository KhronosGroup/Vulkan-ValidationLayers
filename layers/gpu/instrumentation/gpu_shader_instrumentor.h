/* Copyright (c) 2020-2024 The Khronos Group Inc.
 * Copyright (c) 2020-2024 Valve Corporation
 * Copyright (c) 2020-2024 LunarG, Inc.
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

#include "containers/custom_containers.h"
#include "generated/chassis.h"
#include "gpu/core/gpu_state_tracker.h"
#include "gpu/resources/gpu_resources.h"
#include "gpu/spirv/instruction.h"
#include "vma/vma.h"

#include <vector>

namespace gpuav {
class Validator;
}

namespace chassis {
struct ShaderInstrumentationMetadata;
struct ShaderObjectInstrumentationData;
}

namespace gpu {

// There are 3 ways to have a null VkShaderModule
// 1. Use GPL for something like Vertex Input which won't have a shader
// 2. Use Shader Objects
// 3. Use VK_KHR_maintenance5 and inline your VkShaderModuleCreateInfo via VkPipelineShaderStageCreateInfo::pNext
//
// The first is handled because you have to link it in the end, but we need a way to differentiate 2 and 3
static const VkShaderModule kPipelineStageInfoHandle = CastFromUint64<VkShaderModule>(0xEEEEEEEEEEEEEEEE);

// GPU Info shows 99% of devices have a maxBoundDescriptorSets of 32 or less, but some are 2^30
// We set a reasonable max because we have to pad the pipeline layout with dummy descriptor set layouts.
static const uint32_t kMaxAdjustedBoundDescriptorSet = 33;

class SpirvCache {
  public:
    void Add(uint32_t hash, std::vector<uint32_t> spirv);
    std::vector<uint32_t> *Get(uint32_t spirv_hash);
    bool IsEmpty() { return spirv_shaders_.empty(); }

  private:
    friend class gpuav::Validator;
    vvl::unordered_map<uint32_t, std::vector<uint32_t>> spirv_shaders_{};
};

struct GpuAssistedShaderTracker {
    VkPipeline pipeline;
    VkShaderModule shader_module;
    VkShaderEXT shader_object;
    std::vector<uint32_t> instrumented_spirv;
};

// Interface common to both GPU-AV and DebugPrintF.
// Handles shader instrumentation (reserve a descriptor slot, create descriptor
// sets, pipeline layout, hook into pipeline creation, etc...)
class GpuShaderInstrumentor : public ValidationStateTracker {
  public:
    using BaseClass = ValidationStateTracker;

    ReadLockGuard ReadLock() const override;
    WriteLockGuard WriteLock() override;
    void PreCallRecordCreateDevice(VkPhysicalDevice physicalDevice, const VkDeviceCreateInfo *pCreateInfo,
                                   const VkAllocationCallbacks *pAllocator, VkDevice *pDevice, const RecordObject &record_obj,
                                   vku::safe_VkDeviceCreateInfo *modified_create_info) override;
    void PostCreateDevice(const VkDeviceCreateInfo *pCreateInfo, const Location &loc) override;
    void PreCallRecordDestroyDevice(VkDevice device, const VkAllocationCallbacks *pAllocator,
                                    const RecordObject &record_obj) override;

    void ReserveBindingSlot(VkPhysicalDevice physicalDevice, VkPhysicalDeviceLimits &limits, const Location &loc);
    void PostCallRecordGetPhysicalDeviceProperties(VkPhysicalDevice physicalDevice,
                                                   VkPhysicalDeviceProperties *pPhysicalDeviceProperties,
                                                   const RecordObject &record_obj) override;
    void PostCallRecordGetPhysicalDeviceProperties2(VkPhysicalDevice physicalDevice,
                                                    VkPhysicalDeviceProperties2 *pPhysicalDeviceProperties2,
                                                    const RecordObject &record_obj) override;

    bool ValidateCmdWaitEvents(VkCommandBuffer command_buffer, VkPipelineStageFlags2 src_stage_mask, const Location &loc) const;
    bool PreCallValidateCmdWaitEvents(VkCommandBuffer commandBuffer, uint32_t eventCount, const VkEvent *pEvents,
                                      VkPipelineStageFlags srcStageMask, VkPipelineStageFlags dstStageMask,
                                      uint32_t memoryBarrierCount, const VkMemoryBarrier *pMemoryBarriers,
                                      uint32_t bufferMemoryBarrierCount, const VkBufferMemoryBarrier *pBufferMemoryBarriers,
                                      uint32_t imageMemoryBarrierCount, const VkImageMemoryBarrier *pImageMemoryBarriers,
                                      const ErrorObject &error_obj) const override;
    bool PreCallValidateCmdWaitEvents2KHR(VkCommandBuffer commandBuffer, uint32_t eventCount, const VkEvent *pEvents,
                                          const VkDependencyInfoKHR *pDependencyInfos, const ErrorObject &error_obj) const override;
    bool PreCallValidateCmdWaitEvents2(VkCommandBuffer commandBuffer, uint32_t eventCount, const VkEvent *pEvents,
                                       const VkDependencyInfo *pDependencyInfos, const ErrorObject &error_obj) const override;
    void PreCallRecordCreatePipelineLayout(VkDevice device, const VkPipelineLayoutCreateInfo *pCreateInfo,
                                           const VkAllocationCallbacks *pAllocator, VkPipelineLayout *pPipelineLayout,
                                           const RecordObject &record_obj, chassis::CreatePipelineLayout &chassis_state) override;
    void PostCallRecordCreatePipelineLayout(VkDevice device, const VkPipelineLayoutCreateInfo *pCreateInfo,
                                            const VkAllocationCallbacks *pAllocator, VkPipelineLayout *pPipelineLayout,
                                            const RecordObject &record_obj) override;

    void PostCallRecordCreateShaderModule(VkDevice device, const VkShaderModuleCreateInfo *pCreateInfo,
                                          const VkAllocationCallbacks *pAllocator, VkShaderModule *pShaderModule,
                                          const RecordObject &record_obj, chassis::CreateShaderModule &chassis_state) override;
    void PreCallRecordShaderObjectInstrumentation(VkShaderCreateInfoEXT &create_info, const Location &create_info_loc,
                                                  chassis::ShaderObjectInstrumentationData &shader_instrumentation_data);
    void PreCallRecordCreateShadersEXT(VkDevice device, uint32_t createInfoCount, const VkShaderCreateInfoEXT *pCreateInfos,
                                       const VkAllocationCallbacks *pAllocator, VkShaderEXT *pShaders,
                                       const RecordObject &record_obj, chassis::ShaderObject &chassis_state) override;
    void PostCallRecordCreateShadersEXT(VkDevice device, uint32_t createInfoCount, const VkShaderCreateInfoEXT *pCreateInfos,
                                        const VkAllocationCallbacks *pAllocator, VkShaderEXT *pShaders,
                                        const RecordObject &record_obj, chassis::ShaderObject &chassis_state) override;
    void PreCallRecordDestroyShaderEXT(VkDevice device, VkShaderEXT shader, const VkAllocationCallbacks *pAllocator,
                                       const RecordObject &record_obj) override;

    void PreCallRecordCreateGraphicsPipelines(VkDevice device, VkPipelineCache pipelineCache, uint32_t count,
                                              const VkGraphicsPipelineCreateInfo *pCreateInfos,
                                              const VkAllocationCallbacks *pAllocator, VkPipeline *pPipelines,
                                              const RecordObject &record_obj, PipelineStates &pipeline_states,
                                              chassis::CreateGraphicsPipelines &chassis_state) override;
    void PreCallRecordCreateComputePipelines(VkDevice device, VkPipelineCache pipelineCache, uint32_t count,
                                             const VkComputePipelineCreateInfo *pCreateInfos,
                                             const VkAllocationCallbacks *pAllocator, VkPipeline *pPipelines,
                                             const RecordObject &record_obj, PipelineStates &pipeline_states,
                                             chassis::CreateComputePipelines &chassis_state) override;
    void PreCallRecordCreateRayTracingPipelinesNV(VkDevice device, VkPipelineCache pipelineCache, uint32_t count,
                                                  const VkRayTracingPipelineCreateInfoNV *pCreateInfos,
                                                  const VkAllocationCallbacks *pAllocator, VkPipeline *pPipelines,
                                                  const RecordObject &record_obj, PipelineStates &pipeline_states,
                                                  chassis::CreateRayTracingPipelinesNV &chassis_state) override;
    void PreCallRecordCreateRayTracingPipelinesKHR(VkDevice device, VkDeferredOperationKHR deferredOperation,
                                                   VkPipelineCache pipelineCache, uint32_t count,
                                                   const VkRayTracingPipelineCreateInfoKHR *pCreateInfos,
                                                   const VkAllocationCallbacks *pAllocator, VkPipeline *pPipelines,
                                                   const RecordObject &record_obj, PipelineStates &pipeline_states,
                                                   chassis::CreateRayTracingPipelinesKHR &chassis_state) override;
    void PostCallRecordCreateGraphicsPipelines(VkDevice device, VkPipelineCache pipelineCache, uint32_t count,
                                               const VkGraphicsPipelineCreateInfo *pCreateInfos,
                                               const VkAllocationCallbacks *pAllocator, VkPipeline *pPipelines,
                                               const RecordObject &record_obj, PipelineStates &pipeline_states,
                                               chassis::CreateGraphicsPipelines &chassis_state) override;
    void PostCallRecordCreateComputePipelines(VkDevice device, VkPipelineCache pipelineCache, uint32_t count,
                                              const VkComputePipelineCreateInfo *pCreateInfos,
                                              const VkAllocationCallbacks *pAllocator, VkPipeline *pPipelines,
                                              const RecordObject &record_obj, PipelineStates &pipeline_states,
                                              chassis::CreateComputePipelines &chassis_state) override;
    void PostCallRecordCreateRayTracingPipelinesNV(VkDevice device, VkPipelineCache pipelineCache, uint32_t count,
                                                   const VkRayTracingPipelineCreateInfoNV *pCreateInfos,
                                                   const VkAllocationCallbacks *pAllocator, VkPipeline *pPipelines,
                                                   const RecordObject &record_obj, PipelineStates &pipeline_states,
                                                   chassis::CreateRayTracingPipelinesNV &chassis_state) override;
    void PostCallRecordCreateRayTracingPipelinesKHR(VkDevice device, VkDeferredOperationKHR deferredOperation,
                                                    VkPipelineCache pipelineCache, uint32_t count,
                                                    const VkRayTracingPipelineCreateInfoKHR *pCreateInfos,
                                                    const VkAllocationCallbacks *pAllocator, VkPipeline *pPipelines,
                                                    const RecordObject &record_obj, PipelineStates &pipeline_states,
                                                    std::shared_ptr<chassis::CreateRayTracingPipelinesKHR> chassis_state) override;
    void PreCallRecordDestroyPipeline(VkDevice device, VkPipeline pipeline, const VkAllocationCallbacks *pAllocator,
                                      const RecordObject &record_obj) override;

    VkDeviceAddress GetBufferDeviceAddressHelper(VkBuffer buffer) const;

    void InternalError(LogObjectList objlist, const Location &loc, const char *const specific_message, bool vma_fail = false) const;
    void InternalWarning(LogObjectList objlist, const Location &loc, const char *const specific_message) const;

    bool IsSelectiveInstrumentationEnabled(const void *pNext);

    std::string GenerateDebugInfoMessage(VkCommandBuffer commandBuffer, const std::vector<spirv::Instruction> &instructions,
                                         uint32_t stage_id, uint32_t stage_info_0, uint32_t stage_info_1, uint32_t stage_info_2,
                                         uint32_t instruction_position, const gpu::GpuAssistedShaderTracker *tracker_info,
                                         uint32_t shader_id, VkPipelineBindPoint pipeline_bind_point,
                                         uint32_t operation_index) const;

  protected:
    std::shared_ptr<vvl::Queue> CreateQueue(VkQueue handle, uint32_t family_index, uint32_t queue_index,
                                            VkDeviceQueueCreateFlags flags,
                                            const VkQueueFamilyProperties &queueFamilyProperties) override;

    bool NeedPipelineCreationShaderInstrumentation(vvl::Pipeline &pipeline_state);
    bool HasBindlessDescriptors(vvl::Pipeline &pipeline_state);
    bool HasBindlessDescriptors(VkShaderCreateInfoEXT &create_info);

    template <typename SafeCreateInfo>
    void PreCallRecordPipelineCreationShaderInstrumentation(
        const VkAllocationCallbacks *pAllocator, vvl::Pipeline &pipeline_state, SafeCreateInfo &new_pipeline_ci,
        const Location &loc, std::vector<chassis::ShaderInstrumentationMetadata> &shader_instrumentation_metadata);
    void PostCallRecordPipelineCreationShaderInstrumentation(
        vvl::Pipeline &pipeline_state, std::vector<chassis::ShaderInstrumentationMetadata> &shader_instrumentation_metadata);

    // We have GPL variations for graphics as they defer instrumentation until linking
    void PreCallRecordPipelineCreationShaderInstrumentationGPL(
        const VkAllocationCallbacks *pAllocator, vvl::Pipeline &pipeline_state,
        vku::safe_VkGraphicsPipelineCreateInfo &new_pipeline_ci, const Location &loc,
        std::vector<chassis::ShaderInstrumentationMetadata> &shader_instrumentation_metadata);
    void PostCallRecordPipelineCreationShaderInstrumentationGPL(
        vvl::Pipeline &pipeline_state, const VkAllocationCallbacks *pAllocator,
        std::vector<chassis::ShaderInstrumentationMetadata> &shader_instrumentation_metadata);

    // GPU-AV and DebugPrint are using the same way to do the actual shader instrumentation logic
    // Returns if shader was instrumented successfully or not
    bool InstrumentShader(const vvl::span<const uint32_t> &input_spirv, uint32_t unique_shader_id, bool has_bindless_descriptors,
                          const Location &loc, std::vector<uint32_t> &out_instrumented_spirv);

  public:
    VkDescriptorSetLayout GetInstrumentationDescriptorSetLayout() { return instrumentation_desc_layout_; }
    VkPipelineLayout GetInstrumentationPipelineLayout() { return instrumentation_pipeline_layout_; }

    // When aborting we will disconnect all future chassis calls.
    // If we are deep into a call stack, we can use this to return up to the chassis call.
    // It should only be used after calls that might abort, not to be used for guarding a function (unless a case is found that make
    // sense too)
    mutable bool aborted_ = false;

    PFN_vkSetDeviceLoaderData vk_set_device_loader_data_;
    std::atomic<uint32_t> unique_shader_module_id_ = 1;  // zero represents no shader module found
    // The descriptor slot we will be injecting our error buffer into
    uint32_t instrumentation_desc_set_bind_index_ = 0;
    // This is a layout used to "pad" a pipeline layout to fill in any gaps to the selected bind index
    VkDescriptorSetLayout dummy_desc_layout_ = VK_NULL_HANDLE;
    VmaAllocator vma_allocator_ = {};
    VmaPool output_buffer_pool_ = VK_NULL_HANDLE;
    std::unique_ptr<DescriptorSetManager> desc_set_manager_;
    vvl::concurrent_unordered_map<uint32_t, GpuAssistedShaderTracker> shader_map_;
    std::vector<VkDescriptorSetLayoutBinding> instrumentation_bindings_;
    SpirvCache instrumented_shaders_cache_;
    DeviceMemoryBlock indices_buffer_{};
    unsigned int indices_buffer_alignment_ = 0;

    // DebugPrintf takes the first available slot in the set
    uint32_t debug_printf_binding_slot_ = 0;

  private:
    void Cleanup();
    // These are objects used to inject our descriptor set into the command buffer
    VkDescriptorSetLayout instrumentation_desc_layout_ = VK_NULL_HANDLE;
    VkPipelineLayout instrumentation_pipeline_layout_ = VK_NULL_HANDLE;
    // Make sure we call the right versions of any timeline semaphore functions.
    bool timeline_khr_{false};

    // Pass select_instrumented_shaders from vkCreateShaderModule to CreatePipeline time
    vvl::unordered_set<VkShaderModule> selected_instrumented_shaders;
};

}  // namespace gpu
