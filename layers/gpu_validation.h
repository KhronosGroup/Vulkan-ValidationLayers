/* Copyright (c) 2018-2019 The Khronos Group Inc.
 * Copyright (c) 2018-2019 Valve Corporation
 * Copyright (c) 2018-2019 LunarG, Inc.
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

#include "chassis.h"
#include "state_tracker.h"
#include "vk_mem_alloc.h"
class GpuAssisted;

struct GpuAssistedDeviceMemoryBlock {
    VkBuffer buffer;
    VmaAllocation allocation;
    std::unordered_map<uint32_t, const cvdescriptorset::Descriptor*> update_at_submit;
};

struct GpuAssistedBufferInfo {
    GpuAssistedDeviceMemoryBlock output_mem_block;
    GpuAssistedDeviceMemoryBlock di_input_mem_block;   // Descriptor Indexing input
    GpuAssistedDeviceMemoryBlock bda_input_mem_block;  // Buffer Device Address input
    VkDescriptorSet desc_set;
    VkDescriptorPool desc_pool;
    VkPipelineBindPoint pipeline_bind_point;
    GpuAssistedBufferInfo(GpuAssistedDeviceMemoryBlock output_mem_block, GpuAssistedDeviceMemoryBlock di_input_mem_block,
                          GpuAssistedDeviceMemoryBlock bda_input_mem_block, VkDescriptorSet desc_set, VkDescriptorPool desc_pool,
                          VkPipelineBindPoint pipeline_bind_point)
        : output_mem_block(output_mem_block),
          di_input_mem_block(di_input_mem_block),
          bda_input_mem_block(bda_input_mem_block),
          desc_set(desc_set),
          desc_pool(desc_pool),
          pipeline_bind_point(pipeline_bind_point){};
};

struct GpuAssistedQueueBarrierCommandInfo {
    VkCommandPool barrier_command_pool = VK_NULL_HANDLE;
    VkCommandBuffer barrier_command_buffer = VK_NULL_HANDLE;
};

// Class to encapsulate Descriptor Set allocation.  This manager creates and destroys Descriptor Pools
// as needed to satisfy requests for descriptor sets.
class GpuAssistedDescriptorSetManager {
  public:
    GpuAssistedDescriptorSetManager(GpuAssisted* dev_data);
    ~GpuAssistedDescriptorSetManager();

    VkResult GetDescriptorSet(VkDescriptorPool* desc_pool, VkDescriptorSet* desc_sets);
    VkResult GetDescriptorSets(uint32_t count, VkDescriptorPool* pool, std::vector<VkDescriptorSet>* desc_sets);
    void PutBackDescriptorSet(VkDescriptorPool desc_pool, VkDescriptorSet desc_set);

  private:
    static const uint32_t kItemsPerChunk = 512;
    struct PoolTracker {
        uint32_t size;
        uint32_t used;
    };

    GpuAssisted* dev_data_;
    std::unordered_map<VkDescriptorPool, struct PoolTracker> desc_pool_map_;
};

struct GpuAssistedShaderTracker {
    VkPipeline pipeline;
    VkShaderModule shader_module;
    std::vector<unsigned int> pgm;
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

    std::unordered_map<VkCommandBuffer, std::vector<GpuAssistedAccelerationStructureBuildValidationBufferInfo>> validation_buffers;
};

class GpuAssisted : public ValidationStateTracker {
    bool aborted = false;
    VkBool32 shaderInt64;
    uint32_t adjusted_max_desc_sets;
    uint32_t desc_set_bind_index;
    uint32_t unique_shader_module_id = 0;
    std::unordered_map<uint32_t, GpuAssistedShaderTracker> shader_map;
    std::unique_ptr<GpuAssistedDescriptorSetManager> desc_set_manager;
    std::map<VkQueue, GpuAssistedQueueBarrierCommandInfo> queue_barrier_command_infos;
    std::unordered_map<VkCommandBuffer, std::vector<GpuAssistedBufferInfo>> command_buffer_map;  // gpu_buffer_list;
    uint32_t output_buffer_size;
    VmaAllocator vmaAllocator = {};
    PFN_vkSetDeviceLoaderData vkSetDeviceLoaderData;
    std::map<VkDeviceAddress, VkDeviceSize> buffer_map;
    GpuAssistedAccelerationStructureBuildValidationState acceleration_structure_validation_state;
    std::vector<GpuAssistedBufferInfo>& GetGpuAssistedBufferInfo(const VkCommandBuffer command_buffer) {
        auto buffer_list = command_buffer_map.find(command_buffer);
        if (buffer_list == command_buffer_map.end()) {
            std::vector<GpuAssistedBufferInfo> new_list{};
            command_buffer_map[command_buffer] = new_list;
            return command_buffer_map[command_buffer];
        }
        return buffer_list->second;
    }
    void ReportSetupProblem(VkDebugReportObjectTypeEXT object_type, uint64_t object_handle,
                            const char* const specific_message) const;

  public:
    VkDescriptorSetLayout debug_desc_layout;
    VkDescriptorSetLayout dummy_desc_layout;
    void PreCallRecordCreateDevice(VkPhysicalDevice gpu, const VkDeviceCreateInfo* pCreateInfo,
                                   const VkAllocationCallbacks* pAllocator, VkDevice* pDevice,
                                   safe_VkDeviceCreateInfo* modified_create_info);
    void PostCallRecordCreateDevice(VkPhysicalDevice gpu, const VkDeviceCreateInfo* pCreateInfo,
                                    const VkAllocationCallbacks* pAllocator, VkDevice* pDevice, VkResult result);
    void PostCallRecordGetBufferDeviceAddressEXT(VkDevice device, const VkBufferDeviceAddressInfoEXT* pInfo,
                                                 VkDeviceAddress address);
    void PreCallRecordDestroyBuffer(VkDevice device, VkBuffer buffer, const VkAllocationCallbacks* pAllocator);
    void PreCallRecordDestroyDevice(VkDevice device, const VkAllocationCallbacks* pAllocator);
    void PostCallRecordBindAccelerationStructureMemoryNV(VkDevice device, uint32_t bindInfoCount,
                                                         const VkBindAccelerationStructureMemoryInfoNV* pBindInfos,
                                                         VkResult result);
    void PreCallRecordCreatePipelineLayout(VkDevice device, const VkPipelineLayoutCreateInfo* pCreateInfo,
                                           const VkAllocationCallbacks* pAllocator, VkPipelineLayout* pPipelineLayout,
                                           void* cpl_state_data);
    void PostCallRecordCreatePipelineLayout(VkDevice device, const VkPipelineLayoutCreateInfo* pCreateInfo,
                                            const VkAllocationCallbacks* pAllocator, VkPipelineLayout* pPipelineLayout,
                                            VkResult result);
    void ResetCommandBuffer(VkCommandBuffer commandBuffer);
    bool PreCallValidateCmdWaitEvents(VkCommandBuffer commandBuffer, uint32_t eventCount, const VkEvent* pEvents,
                                      VkPipelineStageFlags srcStageMask, VkPipelineStageFlags dstStageMask,
                                      uint32_t memoryBarrierCount, const VkMemoryBarrier* pMemoryBarriers,
                                      uint32_t bufferMemoryBarrierCount, const VkBufferMemoryBarrier* pBufferMemoryBarriers,
                                      uint32_t imageMemoryBarrierCount, const VkImageMemoryBarrier* pImageMemoryBarriers) const;
    void PreCallRecordCreateBuffer(VkDevice device, const VkBufferCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator,
                                   VkBuffer* pBuffer, void* cb_state_data);
    void CreateAccelerationStructureBuildValidationState(GpuAssisted* device_GpuAssisted);
    void DestroyAccelerationStructureBuildValidationState();
    void PreCallRecordCmdBuildAccelerationStructureNV(VkCommandBuffer commandBuffer, const VkAccelerationStructureInfoNV* pInfo,
                                                      VkBuffer instanceData, VkDeviceSize instanceOffset, VkBool32 update,
                                                      VkAccelerationStructureNV dst, VkAccelerationStructureNV src,
                                                      VkBuffer scratch, VkDeviceSize scratchOffset);
    void ProcessAccelerationStructureBuildValidationBuffer(VkQueue queue, CMD_BUFFER_STATE* cb_node);
    void PreCallRecordCreateGraphicsPipelines(VkDevice device, VkPipelineCache pipelineCache, uint32_t count,
                                              const VkGraphicsPipelineCreateInfo* pCreateInfos,
                                              const VkAllocationCallbacks* pAllocator, VkPipeline* pPipelines,
                                              void* cgpl_state_data);
    void PreCallRecordCreateComputePipelines(VkDevice device, VkPipelineCache pipelineCache, uint32_t count,
                                             const VkComputePipelineCreateInfo* pCreateInfos,
                                             const VkAllocationCallbacks* pAllocator, VkPipeline* pPipelines,
                                             void* ccpl_state_data);
    void PreCallRecordCreateRayTracingPipelinesNV(VkDevice device, VkPipelineCache pipelineCache, uint32_t count,
                                                  const VkRayTracingPipelineCreateInfoNV* pCreateInfos,
                                                  const VkAllocationCallbacks* pAllocator, VkPipeline* pPipelines,
                                                  void* crtpl_state_data);
    template <typename CreateInfo, typename SafeCreateInfo>
    void PreCallRecordPipelineCreations(uint32_t count, const CreateInfo* pCreateInfos, const VkAllocationCallbacks* pAllocator,
                                        VkPipeline* pPipelines, std::vector<std::shared_ptr<PIPELINE_STATE>>& pipe_state,
                                        std::vector<SafeCreateInfo>* new_pipeline_create_infos,
                                        const VkPipelineBindPoint bind_point);
    template <typename CreateInfo>
    void PostCallRecordPipelineCreations(const uint32_t count, const CreateInfo* pCreateInfos,
                                         const VkAllocationCallbacks* pAllocator, VkPipeline* pPipelines,
                                         const VkPipelineBindPoint bind_point);
    void PostCallRecordCreateGraphicsPipelines(VkDevice device, VkPipelineCache pipelineCache, uint32_t count,
                                               const VkGraphicsPipelineCreateInfo* pCreateInfos,
                                               const VkAllocationCallbacks* pAllocator, VkPipeline* pPipelines, VkResult result,
                                               void* cgpl_state_data);
    void PostCallRecordCreateComputePipelines(VkDevice device, VkPipelineCache pipelineCache, uint32_t count,
                                              const VkComputePipelineCreateInfo* pCreateInfos,
                                              const VkAllocationCallbacks* pAllocator, VkPipeline* pPipelines, VkResult result,
                                              void* ccpl_state_data);
    void PostCallRecordCreateRayTracingPipelinesNV(VkDevice device, VkPipelineCache pipelineCache, uint32_t count,
                                                   const VkRayTracingPipelineCreateInfoNV* pCreateInfos,
                                                   const VkAllocationCallbacks* pAllocator, VkPipeline* pPipelines, VkResult result,
                                                   void* crtpl_state_data);
    void PreCallRecordDestroyPipeline(VkDevice device, VkPipeline pipeline, const VkAllocationCallbacks* pAllocator);
    bool InstrumentShader(const VkShaderModuleCreateInfo* pCreateInfo, std::vector<unsigned int>& new_pgm,
                          uint32_t* unique_shader_id);
    void PreCallRecordCreateShaderModule(VkDevice device, const VkShaderModuleCreateInfo* pCreateInfo,
                                         const VkAllocationCallbacks* pAllocator, VkShaderModule* pShaderModule,
                                         void* csm_state_data);
    void AnalyzeAndReportError(CMD_BUFFER_STATE* cb_node, VkQueue queue, VkPipelineBindPoint pipeline_bind_point,
                               uint32_t operation_index, uint32_t* const debug_output_buffer);
    void ProcessInstrumentationBuffer(VkQueue queue, CMD_BUFFER_STATE* cb_node);
    void UpdateInstrumentationBuffer(CMD_BUFFER_STATE* cb_node);
    void SubmitBarrier(VkQueue queue);
    void PreCallRecordQueueSubmit(VkQueue queue, uint32_t submitCount, const VkSubmitInfo* pSubmits, VkFence fence);
    void PostCallRecordQueueSubmit(VkQueue queue, uint32_t submitCount, const VkSubmitInfo* pSubmits, VkFence fence,
                                   VkResult result);
    void PreCallRecordCmdDraw(VkCommandBuffer commandBuffer, uint32_t vertexCount, uint32_t instanceCount, uint32_t firstVertex,
                              uint32_t firstInstance);
    void PreCallRecordCmdDrawIndexed(VkCommandBuffer commandBuffer, uint32_t indexCount, uint32_t instanceCount,
                                     uint32_t firstIndex, int32_t vertexOffset, uint32_t firstInstance);
    void PreCallRecordCmdDrawIndirect(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset, uint32_t count,
                                      uint32_t stride);
    void PreCallRecordCmdDrawIndexedIndirect(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset, uint32_t count,
                                             uint32_t stride);
    void PreCallRecordCmdDispatch(VkCommandBuffer commandBuffer, uint32_t x, uint32_t y, uint32_t z);
    void PreCallRecordCmdDispatchIndirect(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset);
    void PreCallRecordCmdTraceRaysNV(VkCommandBuffer commandBuffer, VkBuffer raygenShaderBindingTableBuffer,
                                     VkDeviceSize raygenShaderBindingOffset, VkBuffer missShaderBindingTableBuffer,
                                     VkDeviceSize missShaderBindingOffset, VkDeviceSize missShaderBindingStride,
                                     VkBuffer hitShaderBindingTableBuffer, VkDeviceSize hitShaderBindingOffset,
                                     VkDeviceSize hitShaderBindingStride, VkBuffer callableShaderBindingTableBuffer,
                                     VkDeviceSize callableShaderBindingOffset, VkDeviceSize callableShaderBindingStride,
                                     uint32_t width, uint32_t height, uint32_t depth);
    void PostCallRecordCmdTraceRaysNV(VkCommandBuffer commandBuffer, VkBuffer raygenShaderBindingTableBuffer,
                                      VkDeviceSize raygenShaderBindingOffset, VkBuffer missShaderBindingTableBuffer,
                                      VkDeviceSize missShaderBindingOffset, VkDeviceSize missShaderBindingStride,
                                      VkBuffer hitShaderBindingTableBuffer, VkDeviceSize hitShaderBindingOffset,
                                      VkDeviceSize hitShaderBindingStride, VkBuffer callableShaderBindingTableBuffer,
                                      VkDeviceSize callableShaderBindingOffset, VkDeviceSize callableShaderBindingStride,
                                      uint32_t width, uint32_t height, uint32_t depth);
    void AllocateValidationResources(const VkCommandBuffer cmd_buffer, const VkPipelineBindPoint bind_point);
    void PostCallRecordGetPhysicalDeviceProperties(VkPhysicalDevice physicalDevice,
                                                   VkPhysicalDeviceProperties* pPhysicalDeviceProperties);
    void PostCallRecordGetPhysicalDeviceProperties2(VkPhysicalDevice physicalDevice,
                                                    VkPhysicalDeviceProperties2* pPhysicalDeviceProperties2);
    VkResult InitializeVma(VkPhysicalDevice physicalDevice, VkDevice device, VmaAllocator* pAllocator);
};
