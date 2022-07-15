/* Copyright (c) 2018-2022 The Khronos Group Inc.
 * Copyright (c) 2018-2022 Valve Corporation
 * Copyright (c) 2018-2022 LunarG, Inc.
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
 *
 * Author: Karl Schultz <karl@lunarg.com>
 * Author: Tony Barbour <tony@lunarg.com>
 */

#pragma once

#include "gpu_utils.h"

class GpuAssisted;

struct GpuAssistedDeviceMemoryBlock {
    VkBuffer buffer;
    VmaAllocation allocation;
    layer_data::unordered_map<uint32_t, const cvdescriptorset::DescriptorBinding*> update_at_submit;
};

struct GpuAssistedPreDrawResources {
    VkDescriptorPool desc_pool;
    VkDescriptorSet desc_set;
    VkBuffer buffer;
    VkDeviceSize offset;
    uint32_t stride;
    VkDeviceSize buf_size;
};

struct GpuAssistedBufferInfo {
    GpuAssistedDeviceMemoryBlock output_mem_block;
    GpuAssistedDeviceMemoryBlock di_input_mem_block;   // Descriptor Indexing input
    GpuAssistedDeviceMemoryBlock bda_input_mem_block;  // Buffer Device Address input
    GpuAssistedPreDrawResources pre_draw_resources;
    VkDescriptorSet desc_set;
    VkDescriptorPool desc_pool;
    VkPipelineBindPoint pipeline_bind_point;
    CMD_TYPE cmd_type;
    GpuAssistedBufferInfo(GpuAssistedDeviceMemoryBlock output_mem_block, GpuAssistedDeviceMemoryBlock di_input_mem_block,
                          GpuAssistedDeviceMemoryBlock bda_input_mem_block, GpuAssistedPreDrawResources pre_draw_resources,
                          VkDescriptorSet desc_set, VkDescriptorPool desc_pool, VkPipelineBindPoint pipeline_bind_point,
                          CMD_TYPE cmd_type)
        : output_mem_block(output_mem_block),
          di_input_mem_block(di_input_mem_block),
          bda_input_mem_block(bda_input_mem_block),
          pre_draw_resources(pre_draw_resources),
          desc_set(desc_set),
          desc_pool(desc_pool),
          pipeline_bind_point(pipeline_bind_point),
          cmd_type(cmd_type){};
};

struct GpuVuid {
    const char* uniform_access_oob = kVUIDUndefined;
    const char* storage_access_oob = kVUIDUndefined;
    const char* count_exceeds_bufsize_1 = kVUIDUndefined;
    const char* count_exceeds_bufsize = kVUIDUndefined;
    const char* count_exceeds_device_limit = kVUIDUndefined;
    const char* first_instance_not_zero = kVUIDUndefined;
};

struct GpuAssistedAccelerationStructureBuildValidationBufferInfo {
    // The acceleration structure that is being built.
    VkAccelerationStructureNV acceleration_structure = VK_NULL_HANDLE;

    // The descriptor pool and descriptor set being used to validate a given build.
    VkDescriptorPool descriptor_pool = VK_NULL_HANDLE;
    VkDescriptorSet descriptor_set = VK_NULL_HANDLE;

    // The storage buffer used by the validating compute shader whichcontains info about
    // the valid handles and which is written to communicate found invalid handles.
    VkBuffer validation_buffer = VK_NULL_HANDLE;
    VmaAllocation validation_buffer_allocation = VK_NULL_HANDLE;
};

struct GpuAssistedAccelerationStructureBuildValidationState {
    bool initialized = false;

    VkPipeline pipeline = VK_NULL_HANDLE;
    VkPipelineLayout pipeline_layout = VK_NULL_HANDLE;

    VkAccelerationStructureNV replacement_as = VK_NULL_HANDLE;
    VmaAllocation replacement_as_allocation = VK_NULL_HANDLE;
    uint64_t replacement_as_handle = 0;

};

struct GpuAssistedPreDrawValidationState {
    bool globals_created = false;
    VkShaderModule validation_shader_module = VK_NULL_HANDLE;
    VkDescriptorSetLayout validation_ds_layout = VK_NULL_HANDLE;
    VkPipelineLayout validation_pipeline_layout = VK_NULL_HANDLE;
    vl_concurrent_unordered_map <VkRenderPass, VkPipeline> renderpass_to_pipeline;
};

struct GpuAssistedCmdDrawIndirectState {
    VkBuffer buffer;
    VkDeviceSize offset;
    uint32_t drawCount;
    uint32_t stride;
    VkBuffer count_buffer;
    VkDeviceSize count_buffer_offset;
};

namespace gpuav_state {
class CommandBuffer : public gpu_utils_state::CommandBuffer {
  public:
    std::vector<GpuAssistedBufferInfo> gpuav_buffer_list;
    std::vector<GpuAssistedAccelerationStructureBuildValidationBufferInfo> as_validation_buffers;

    CommandBuffer(GpuAssisted* ga, VkCommandBuffer cb, const VkCommandBufferAllocateInfo* pCreateInfo,
                  const COMMAND_POOL_STATE* pool);

    bool NeedsProcessing() const final { return !gpuav_buffer_list.empty() || hasBuildAccelerationStructureCmd; }

    void Process(VkQueue queue) final;
    void Reset() final;

  private:
    void ProcessAccelerationStructure(VkQueue queue);
};
};  // namespace gpuav_state

VALSTATETRACK_DERIVED_STATE_OBJECT(VkCommandBuffer, gpuav_state::CommandBuffer, CMD_BUFFER_STATE);

class GpuAssisted : public GpuAssistedBase {
  public:
    GpuAssisted() {
        setup_vuid = "UNASSIGNED-GPU-Assisted-Validation";
        container_type = LayerObjectTypeGpuAssisted;
        desired_features.vertexPipelineStoresAndAtomics = true;
        desired_features.fragmentStoresAndAtomics = true;
        desired_features.shaderInt64 = true;
    }

    bool CheckForDescriptorIndexing(DeviceFeatures enabled_features) const;
    void CreateDevice(const VkDeviceCreateInfo* pCreateInfo) override;
    void PreCallRecordDestroyDevice(VkDevice device, const VkAllocationCallbacks* pAllocator) override;
    void PostCallRecordBindAccelerationStructureMemoryNV(VkDevice device, uint32_t bindInfoCount,
                                                         const VkBindAccelerationStructureMemoryInfoNV* pBindInfos,
                                                         VkResult result) override;
    bool PreCallValidateCmdWaitEvents(VkCommandBuffer commandBuffer, uint32_t eventCount, const VkEvent* pEvents,
                                      VkPipelineStageFlags srcStageMask, VkPipelineStageFlags dstStageMask,
                                      uint32_t memoryBarrierCount, const VkMemoryBarrier* pMemoryBarriers,
                                      uint32_t bufferMemoryBarrierCount, const VkBufferMemoryBarrier* pBufferMemoryBarriers,
                                      uint32_t imageMemoryBarrierCount, const VkImageMemoryBarrier* pImageMemoryBarriers) const override;
    bool PreCallValidateCmdWaitEvents2KHR(VkCommandBuffer commandBuffer, uint32_t eventCount, const VkEvent* pEvents,
                                          const VkDependencyInfoKHR* pDependencyInfos) const override;
    bool PreCallValidateCmdWaitEvents2(VkCommandBuffer commandBuffer, uint32_t eventCount, const VkEvent* pEvents,
                                       const VkDependencyInfo* pDependencyInfos) const override;
    void PreCallRecordCreateBuffer(VkDevice device, const VkBufferCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator,
                                   VkBuffer* pBuffer, void* cb_state_data) override;
    void CreateAccelerationStructureBuildValidationState();
    void DestroyAccelerationStructureBuildValidationState();
    void PreCallRecordCmdBuildAccelerationStructureNV(VkCommandBuffer commandBuffer, const VkAccelerationStructureInfoNV* pInfo,
                                                      VkBuffer instanceData, VkDeviceSize instanceOffset, VkBool32 update,
                                                      VkAccelerationStructureNV dst, VkAccelerationStructureNV src,
                                                      VkBuffer scratch, VkDeviceSize scratchOffset) override;
    void ProcessAccelerationStructureBuildValidationBuffer(VkQueue queue, gpuav_state::CommandBuffer* cb_node);
    void PreCallRecordDestroyRenderPass(VkDevice device, VkRenderPass renderPass, const VkAllocationCallbacks *pAllocator) override;
    bool InstrumentShader(const VkShaderModuleCreateInfo* pCreateInfo, std::vector<uint32_t>& new_pgm, uint32_t* unique_shader_id);
    void PreCallRecordCreateShaderModule(VkDevice device, const VkShaderModuleCreateInfo* pCreateInfo,
                                         const VkAllocationCallbacks* pAllocator, VkShaderModule* pShaderModule,
                                         void* csm_state_data) override;
    void AnalyzeAndGenerateMessages(VkCommandBuffer command_buffer, VkQueue queue, GpuAssistedBufferInfo &buffer_info,
        uint32_t operation_index, uint32_t* const debug_output_buffer);

    void SetBindingState(uint32_t* data, uint32_t index, const cvdescriptorset::DescriptorBinding* binding);
    void UpdateInstrumentationBuffer(gpuav_state::CommandBuffer* cb_node);
    const GpuVuid& GetGpuVuid(CMD_TYPE cmd_type) const;
    void PreCallRecordQueueSubmit(VkQueue queue, uint32_t submitCount, const VkSubmitInfo* pSubmits, VkFence fence) override;
    void PreCallRecordQueueSubmit2KHR(VkQueue queue, uint32_t submitCount, const VkSubmitInfo2KHR* pSubmits,
                                      VkFence fence) override;
    void PreCallRecordQueueSubmit2(VkQueue queue, uint32_t submitCount, const VkSubmitInfo2* pSubmits, VkFence fence) override;
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
    void PostCallRecordCmdTraceRaysNV(VkCommandBuffer commandBuffer, VkBuffer raygenShaderBindingTableBuffer,
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
    void PostCallRecordCmdTraceRaysKHR(VkCommandBuffer commandBuffer,
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
    void PostCallRecordCmdTraceRaysIndirectKHR(VkCommandBuffer commandBuffer,
                                               const VkStridedDeviceAddressRegionKHR* pRaygenShaderBindingTable,
                                               const VkStridedDeviceAddressRegionKHR* pMissShaderBindingTable,
                                               const VkStridedDeviceAddressRegionKHR* pHitShaderBindingTable,
                                               const VkStridedDeviceAddressRegionKHR* pCallableShaderBindingTable,
                                               VkDeviceAddress indirectDeviceAddress) override;
    void AllocateValidationResources(const VkCommandBuffer cmd_buffer, const VkPipelineBindPoint bind_point, CMD_TYPE cmd, const GpuAssistedCmdDrawIndirectState *cdic_state = nullptr);
    void AllocatePreDrawValidationResources(GpuAssistedDeviceMemoryBlock output_block, GpuAssistedPreDrawResources& resources,
                                            const LAST_BOUND_STATE& state, VkPipeline *pPipeline, const GpuAssistedCmdDrawIndirectState *cdic_state);
    void PostCallRecordGetPhysicalDeviceProperties(VkPhysicalDevice physicalDevice,
                                                   VkPhysicalDeviceProperties* pPhysicalDeviceProperties) override;
    void PostCallRecordGetPhysicalDeviceProperties2(VkPhysicalDevice physicalDevice,
                                                    VkPhysicalDeviceProperties2* pPhysicalDeviceProperties2) override;

    std::shared_ptr<CMD_BUFFER_STATE> CreateCmdBufferState(VkCommandBuffer cb, const VkCommandBufferAllocateInfo* create_info,
                                                           const COMMAND_POOL_STATE* pool) final;

    void DestroyBuffer(GpuAssistedBufferInfo& buffer_info);
    void DestroyBuffer(GpuAssistedAccelerationStructureBuildValidationBufferInfo& buffer_info);

  private:
    void PreRecordCommandBuffer(VkCommandBuffer command_buffer);
    VkPipeline GetValidationPipeline(VkRenderPass rp);

    VkBool32 shaderInt64;
    bool buffer_oob_enabled;
    bool validate_draw_indirect;
    VmaPool output_buffer_pool = VK_NULL_HANDLE;
    GpuAssistedAccelerationStructureBuildValidationState acceleration_structure_validation_state;
    GpuAssistedPreDrawValidationState pre_draw_validation_state;

    bool descriptor_indexing = false;
};
