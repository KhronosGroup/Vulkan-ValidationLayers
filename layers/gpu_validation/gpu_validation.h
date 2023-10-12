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

#include "gpu_validation/gpu_state_tracker.h"
#include "gpu_validation/gpu_error_message.h"
#include "gpu_validation/gpu_descriptor_set.h"
#include "gpu_validation/gpu_subclasses.h"
#include "state_tracker/pipeline_state.h"

class GpuAssisted;

struct GpuVuid {
    const char* uniform_access_oob = kVUIDUndefined;
    const char* storage_access_oob = kVUIDUndefined;
    const char* count_exceeds_bufsize_1 = kVUIDUndefined;
    const char* count_exceeds_bufsize = kVUIDUndefined;
    const char* count_exceeds_device_limit = kVUIDUndefined;
    const char* first_instance_not_zero = kVUIDUndefined;
    const char* group_exceeds_device_limit_x = kVUIDUndefined;
    const char* group_exceeds_device_limit_y = kVUIDUndefined;
    const char* group_exceeds_device_limit_z = kVUIDUndefined;
};

namespace gpuav_state {
struct AccelerationStructureBuildValidationState {
    // some resources can be used each time so only to need to create once
    bool initialized = false;

    VkPipeline pipeline = VK_NULL_HANDLE;
    VkPipelineLayout pipeline_layout = VK_NULL_HANDLE;

    VkAccelerationStructureNV replacement_as = VK_NULL_HANDLE;
    VmaAllocation replacement_as_allocation = VK_NULL_HANDLE;
    uint64_t replacement_as_handle = 0;

    void Destroy(VkDevice device, VmaAllocator& vmaAllocator);
};

struct PreDrawValidationState {
    // some resources can be used each time so only to need to create once
    bool initialized = false;

    VkShaderModule shader_module = VK_NULL_HANDLE;
    VkDescriptorSetLayout ds_layout = VK_NULL_HANDLE;
    VkPipelineLayout pipeline_layout = VK_NULL_HANDLE;
    vl_concurrent_unordered_map<VkRenderPass, VkPipeline> renderpass_to_pipeline;
    VkShaderEXT shader_object = VK_NULL_HANDLE;

    void Destroy(VkDevice device);
};

struct PreDispatchValidationState {
    // some resources can be used each time so only to need to create once
    bool initialized = false;

    VkShaderModule shader_module = VK_NULL_HANDLE;
    VkDescriptorSetLayout ds_layout = VK_NULL_HANDLE;
    VkPipelineLayout pipeline_layout = VK_NULL_HANDLE;
    VkPipeline pipeline = VK_NULL_HANDLE;
    VkShaderEXT shader_object = VK_NULL_HANDLE;

    void Destroy(VkDevice device);
};

// Used for draws/dispatch/traceRays indirect
struct CmdIndirectState {
    VkBuffer buffer;
    VkDeviceSize offset;
    uint32_t draw_count;
    uint32_t stride;
    VkBuffer count_buffer;
    VkDeviceSize count_buffer_offset;
};
}  // namespace gpuav_state

VALSTATETRACK_DERIVED_STATE_OBJECT(VkAccelerationStructureKHR, gpuav_state::AccelerationStructureKHR,
                                   ACCELERATION_STRUCTURE_STATE_KHR)
VALSTATETRACK_DERIVED_STATE_OBJECT(VkAccelerationStructureNV, gpuav_state::AccelerationStructureNV, ACCELERATION_STRUCTURE_STATE_NV)
VALSTATETRACK_DERIVED_STATE_OBJECT(VkBuffer, gpuav_state::Buffer, BUFFER_STATE)
VALSTATETRACK_DERIVED_STATE_OBJECT(VkBufferView, gpuav_state::BufferView, BUFFER_VIEW_STATE)
VALSTATETRACK_DERIVED_STATE_OBJECT(VkCommandBuffer, gpuav_state::CommandBuffer, CMD_BUFFER_STATE)
VALSTATETRACK_DERIVED_STATE_OBJECT(VkDescriptorSet, gpuav_state::DescriptorSet, cvdescriptorset::DescriptorSet)
VALSTATETRACK_DERIVED_STATE_OBJECT(VkImageView, gpuav_state::ImageView, IMAGE_VIEW_STATE)
VALSTATETRACK_DERIVED_STATE_OBJECT(VkSampler, gpuav_state::Sampler, SAMPLER_STATE)

class GpuAssisted : public GpuAssistedBase {
    using Func = vvl::Func;

  public:
    GpuAssisted() {
        setup_vuid = "UNASSIGNED-GPU-Assisted-Validation";
        container_type = LayerObjectTypeGpuAssisted;
        desired_features.vertexPipelineStoresAndAtomics = true;
        desired_features.fragmentStoresAndAtomics = true;
        desired_features.shaderInt64 = true;
        force_buffer_device_address = true;
    }

    bool CheckForDescriptorIndexing(DeviceFeatures enabled_features) const;
    void CreateDevice(const VkDeviceCreateInfo* pCreateInfo) override;
    void PreCallRecordDestroyDevice(VkDevice device, const VkAllocationCallbacks* pAllocator) override;
    void PostCallRecordBindAccelerationStructureMemoryNV(VkDevice device, uint32_t bindInfoCount,
                                                         const VkBindAccelerationStructureMemoryInfoNV* pBindInfos,
                                                         const RecordObject& record_obj) override;
    std::shared_ptr<BUFFER_STATE> CreateBufferState(VkBuffer buf, const VkBufferCreateInfo* pCreateInfo) override;
    void PreCallRecordCreateBuffer(VkDevice device, const VkBufferCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator,
                                   VkBuffer* pBuffer, void* cb_state_data) override;

    std::shared_ptr<BUFFER_VIEW_STATE> CreateBufferViewState(const std::shared_ptr<BUFFER_STATE>& bf, VkBufferView bv,
                                                             const VkBufferViewCreateInfo* ci,
                                                             VkFormatFeatureFlags2KHR buf_ff) override;
    std::shared_ptr<IMAGE_VIEW_STATE> CreateImageViewState(
        const std::shared_ptr<IMAGE_STATE>& image_state, VkImageView iv, const VkImageViewCreateInfo* ci,
        VkFormatFeatureFlags2KHR ff, const VkFilterCubicImageViewImageFormatPropertiesEXT& cubic_props) override;
    std::shared_ptr<ACCELERATION_STRUCTURE_STATE_NV> CreateAccelerationStructureState(
        VkAccelerationStructureNV as, const VkAccelerationStructureCreateInfoNV* pCreateInfo) override;
    std::shared_ptr<ACCELERATION_STRUCTURE_STATE_KHR> CreateAccelerationStructureState(
        VkAccelerationStructureKHR as, const VkAccelerationStructureCreateInfoKHR* pCreateInfo,
        std::shared_ptr<BUFFER_STATE>&& buf_state, VkDeviceAddress address) override;
    std::shared_ptr<SAMPLER_STATE> CreateSamplerState(VkSampler s, const VkSamplerCreateInfo* ci) override;

    void CreateAccelerationStructureBuildValidationState(const VkDeviceCreateInfo* pCreateInfo);
    void PreCallRecordCmdBuildAccelerationStructureNV(VkCommandBuffer commandBuffer, const VkAccelerationStructureInfoNV* pInfo,
                                                      VkBuffer instanceData, VkDeviceSize instanceOffset, VkBool32 update,
                                                      VkAccelerationStructureNV dst, VkAccelerationStructureNV src,
                                                      VkBuffer scratch, VkDeviceSize scratchOffset) override;
    void PreCallRecordDestroyRenderPass(VkDevice device, VkRenderPass renderPass, const VkAllocationCallbacks* pAllocator) override;
    bool CheckForCachedInstrumentedShader(const uint32_t shader_hash, create_shader_module_api_state* csm_state);
    bool CheckForCachedInstrumentedShader(const uint32_t index, const uint32_t shader_hash, create_shader_object_api_state* cso_state);
    bool InstrumentShader(const vvl::span<const uint32_t>& input, std::vector<uint32_t>& new_pgm,
                          uint32_t unique_shader_id) override;
    void PreCallRecordCreateShaderModule(VkDevice device, const VkShaderModuleCreateInfo* pCreateInfo,
                                         const VkAllocationCallbacks* pAllocator, VkShaderModule* pShaderModule,
                                         void* csm_state_data) override;
    void PreCallRecordCreateShadersEXT(VkDevice device, uint32_t createInfoCount, const VkShaderCreateInfoEXT* pCreateInfos,
                                       const VkAllocationCallbacks* pAllocator, VkShaderEXT* pShaders,
                                       void* csm_state_data) override;
    void AnalyzeAndGenerateMessages(VkCommandBuffer command_buffer, VkQueue queue, gpuav_state::BufferInfo& buffer_info,
                                    uint32_t operation_index, uint32_t* const debug_output_buffer,
                                    const std::vector<gpuav_state::DescSetState>& descriptor_sets);
    void UpdateInstrumentationBuffer(gpuav_state::CommandBuffer* cb_node);
    void UpdateBDABuffer(gpuav_state::DeviceMemoryBlock buffer_device_addresses);

    void UpdateBoundDescriptors(VkCommandBuffer commandBuffer, VkPipelineBindPoint pipelineBindPoint);

    void PostCallRecordCmdBindDescriptorSets(VkCommandBuffer commandBuffer, VkPipelineBindPoint pipelineBindPoint,
                                             VkPipelineLayout layout, uint32_t firstSet, uint32_t descriptorSetCount,
                                             const VkDescriptorSet* pDescriptorSets, uint32_t dynamicOffsetCount,
                                             const uint32_t* pDynamicOffsets, const RecordObject& record_obj) override;
    void PreCallRecordCmdPushDescriptorSetKHR(VkCommandBuffer commandBuffer, VkPipelineBindPoint pipelineBindPoint,
                                              VkPipelineLayout layout, uint32_t set, uint32_t descriptorWriteCount,
                                              const VkWriteDescriptorSet* pDescriptorWrites) override;
    void PreCallRecordQueueSubmit(VkQueue queue, uint32_t submitCount, const VkSubmitInfo* pSubmits, VkFence fence) override;
    void PreCallRecordQueueSubmit2KHR(VkQueue queue, uint32_t submitCount, const VkSubmitInfo2KHR* pSubmits,
                                      VkFence fence) override;
    void PreCallRecordQueueSubmit2(VkQueue queue, uint32_t submitCount, const VkSubmitInfo2* pSubmits, VkFence fence) override;
    void PreCallRecordCmdBindDescriptorBuffersEXT(VkCommandBuffer commandBuffer, uint32_t bufferCount,
                                                  const VkDescriptorBufferBindingInfoEXT* pBindingInfos) override;
    void PreCallRecordCmdBindDescriptorBufferEmbeddedSamplersEXT(VkCommandBuffer commandBuffer,
                                                                 VkPipelineBindPoint pipelineBindPoint, VkPipelineLayout layout,
                                                                 uint32_t set) override;
    void PreCallRecordCmdDraw(VkCommandBuffer commandBuffer, uint32_t vertexCount, uint32_t instanceCount, uint32_t firstVertex,
                              uint32_t firstInstance) override;
    void PreCallRecordCmdDrawMultiEXT(VkCommandBuffer commandBuffer, uint32_t drawCount, const VkMultiDrawInfoEXT* pVertexInfo,
                                      uint32_t instanceCount, uint32_t firstInstance, uint32_t stride) override;
    void PreCallRecordCmdDrawIndexed(VkCommandBuffer commandBuffer, uint32_t indexCount, uint32_t instanceCount,
                                     uint32_t firstIndex, int32_t vertexOffset, uint32_t firstInstance) override;
    void PreCallRecordCmdDrawMultiIndexedEXT(VkCommandBuffer commandBuffer, uint32_t drawCount,
                                             const VkMultiDrawIndexedInfoEXT* pIndexInfo, uint32_t instanceCount,
                                             uint32_t firstInstance, uint32_t stride, const int32_t* pVertexOffset) override;
    void PreCallRecordCmdDrawIndirect(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset, uint32_t count,
                                      uint32_t stride) override;
    void PreCallRecordCmdDrawIndexedIndirect(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset, uint32_t count,
                                             uint32_t stride) override;
    void PreCallRecordCmdDrawIndirectCountKHR(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                              VkBuffer countBuffer, VkDeviceSize countBufferOffset, uint32_t maxDrawCount,
                                              uint32_t stride) override;
    void PreCallRecordCmdDrawIndirectCount(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                           VkBuffer countBuffer, VkDeviceSize countBufferOffset, uint32_t maxDrawCount,
                                           uint32_t stride) override;
    void PreCallRecordCmdDrawIndirectByteCountEXT(VkCommandBuffer commandBuffer, uint32_t instanceCount, uint32_t firstInstance,
                                                  VkBuffer counterBuffer, VkDeviceSize counterBufferOffset, uint32_t counterOffset,
                                                  uint32_t vertexStride) override;
    void PreCallRecordCmdDrawIndexedIndirectCountKHR(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                     VkBuffer countBuffer, VkDeviceSize countBufferOffset, uint32_t maxDrawCount,
                                                     uint32_t stride) override;
    void PreCallRecordCmdDrawIndexedIndirectCount(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                  VkBuffer countBuffer, VkDeviceSize countBufferOffset, uint32_t maxDrawCount,
                                                  uint32_t stride) override;
    void PreCallRecordCmdDrawMeshTasksNV(VkCommandBuffer commandBuffer, uint32_t taskCount, uint32_t firstTask) override;
    void PreCallRecordCmdDrawMeshTasksIndirectNV(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                 uint32_t drawCount, uint32_t stride) override;
    void PreCallRecordCmdDrawMeshTasksIndirectCountNV(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                      VkBuffer countBuffer, VkDeviceSize countBufferOffset, uint32_t maxDrawCount,
                                                      uint32_t stride) override;
    void PreCallRecordCmdDrawMeshTasksEXT(VkCommandBuffer commandBuffer, uint32_t groupCountX, uint32_t groupCountY,
                                          uint32_t groupCountZ) override;
    void PreCallRecordCmdDrawMeshTasksIndirectEXT(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                  uint32_t drawCount, uint32_t stride) override;
    void PreCallRecordCmdDrawMeshTasksIndirectCountEXT(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                       VkBuffer countBuffer, VkDeviceSize countBufferOffset, uint32_t maxDrawCount,
                                                       uint32_t stride) override;
    void PreCallRecordCmdDispatch(VkCommandBuffer commandBuffer, uint32_t x, uint32_t y, uint32_t z) override;
    void PreCallRecordCmdDispatchIndirect(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset) override;
    void PreCallRecordCmdDispatchBase(VkCommandBuffer commandBuffer, uint32_t baseGroupX, uint32_t baseGroupY, uint32_t baseGroupZ,
                                      uint32_t groupCountX, uint32_t groupCountY, uint32_t groupCountZ) override;
    void PreCallRecordCmdDispatchBaseKHR(VkCommandBuffer commandBuffer, uint32_t baseGroupX, uint32_t baseGroupY,
                                         uint32_t baseGroupZ, uint32_t groupCountX, uint32_t groupCountY,
                                         uint32_t groupCountZ) override;
    void PreCallRecordCmdTraceRaysNV(VkCommandBuffer commandBuffer, VkBuffer raygenShaderBindingTableBuffer,
                                     VkDeviceSize raygenShaderBindingOffset, VkBuffer missShaderBindingTableBuffer,
                                     VkDeviceSize missShaderBindingOffset, VkDeviceSize missShaderBindingStride,
                                     VkBuffer hitShaderBindingTableBuffer, VkDeviceSize hitShaderBindingOffset,
                                     VkDeviceSize hitShaderBindingStride, VkBuffer callableShaderBindingTableBuffer,
                                     VkDeviceSize callableShaderBindingOffset, VkDeviceSize callableShaderBindingStride,
                                     uint32_t width, uint32_t height, uint32_t depth) override;
    void PreCallRecordCmdTraceRaysKHR(VkCommandBuffer commandBuffer,
                                      const VkStridedDeviceAddressRegionKHR* pRaygenShaderBindingTable,
                                      const VkStridedDeviceAddressRegionKHR* pMissShaderBindingTable,
                                      const VkStridedDeviceAddressRegionKHR* pHitShaderBindingTable,
                                      const VkStridedDeviceAddressRegionKHR* pCallableShaderBindingTable, uint32_t width,
                                      uint32_t height, uint32_t depth) override;
    void PreCallRecordCmdTraceRaysIndirectKHR(VkCommandBuffer commandBuffer,
                                              const VkStridedDeviceAddressRegionKHR* pRaygenShaderBindingTable,
                                              const VkStridedDeviceAddressRegionKHR* pMissShaderBindingTable,
                                              const VkStridedDeviceAddressRegionKHR* pHitShaderBindingTable,
                                              const VkStridedDeviceAddressRegionKHR* pCallableShaderBindingTable,
                                              VkDeviceAddress indirectDeviceAddress) override;
    void PreCallRecordCmdTraceRaysIndirect2KHR(VkCommandBuffer commandBuffer, VkDeviceAddress indirectDeviceAddress) override;
    void AllocateValidationResources(const VkCommandBuffer cmd_buffer, const VkPipelineBindPoint bind_point, Func command,
                                     const gpuav_state::CmdIndirectState* indirect_state = nullptr);
    void AllocatePreDrawValidationResources(const gpuav_state::DeviceMemoryBlock& output_block,
                                            gpuav_state::PreDrawResources& resources, const VkRenderPass render_pass,
                                            const bool use_shader_objects, VkPipeline* pPipeline, const gpuav_state::CmdIndirectState* indirect_state);
    void AllocatePreDispatchValidationResources(const gpuav_state::DeviceMemoryBlock& output_block,
                                                gpuav_state::PreDispatchResources& resources,
                                                const gpuav_state::CmdIndirectState* indirect_state, const bool use_shader_objects);
    void PostCallRecordGetPhysicalDeviceProperties(VkPhysicalDevice physicalDevice,
                                                   VkPhysicalDeviceProperties* pPhysicalDeviceProperties,
                                                   const RecordObject& record_obj) override;
    void PostCallRecordGetPhysicalDeviceProperties2(VkPhysicalDevice physicalDevice,
                                                    VkPhysicalDeviceProperties2* pPhysicalDeviceProperties2,
                                                    const RecordObject& record_obj) override;

    std::shared_ptr<CMD_BUFFER_STATE> CreateCmdBufferState(VkCommandBuffer cb, const VkCommandBufferAllocateInfo* create_info,
                                                           const COMMAND_POOL_STATE* pool) final;
    std::shared_ptr<cvdescriptorset::DescriptorSet> CreateDescriptorSet(
        VkDescriptorSet, DESCRIPTOR_POOL_STATE*, const std::shared_ptr<cvdescriptorset::DescriptorSetLayout const>& layout,
        uint32_t variable_count) final;

    void DestroyBuffer(gpuav_state::BufferInfo& buffer_info);
    void DestroyBuffer(gpuav_state::AccelerationStructureBuildValidationBufferInfo& buffer_info);

  private:
    void PreRecordCommandBuffer(VkCommandBuffer command_buffer);
    VkPipeline GetValidationPipeline(VkRenderPass render_pass);

    VkBool32 shaderInt64;
    bool validate_instrumented_shaders;
    std::string instrumented_shader_cache_path;
    gpuav_state::AccelerationStructureBuildValidationState acceleration_structure_validation_state;
    gpuav_state::PreDrawValidationState pre_draw_validation_state;
    gpuav_state::PreDispatchValidationState pre_dispatch_validation_state;
    gpuav_state::DeviceMemoryBlock app_buffer_device_addresses{};
    size_t app_bda_buffer_size{};
    uint32_t gpuav_bda_buffer_version = 0;

    bool buffer_device_address;

    std::optional<gpuav_state::DescriptorHeap> desc_heap; // optional only to defer construction
};
