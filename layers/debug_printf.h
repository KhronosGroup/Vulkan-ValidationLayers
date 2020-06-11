/* Copyright (c) 2020 The Khronos Group Inc.
 * Copyright (c) 2020 Valve Corporation
 * Copyright (c) 2020 LunarG, Inc.
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
 * Author: Tony Barbour <tony@lunarg.com>
 */

#pragma once

#include "chassis.h"
#include "vk_mem_alloc.h"
#include "state_tracker.h"
#include "gpu_utils.h"
#include <map>
class DebugPrintf;

struct DPFDeviceMemoryBlock {
    VkBuffer buffer;
    VmaAllocation allocation;
};

struct DPFBufferInfo {
    DPFDeviceMemoryBlock output_mem_block;
    VkDescriptorSet desc_set;
    VkDescriptorPool desc_pool;
    VkPipelineBindPoint pipeline_bind_point;
    DPFBufferInfo(DPFDeviceMemoryBlock output_mem_block, VkDescriptorSet desc_set, VkDescriptorPool desc_pool,
                  VkPipelineBindPoint pipeline_bind_point)
        : output_mem_block(output_mem_block), desc_set(desc_set), desc_pool(desc_pool), pipeline_bind_point(pipeline_bind_point){};
};

struct DPFShaderTracker {
    VkPipeline pipeline;
    VkShaderModule shader_module;
    std::vector<unsigned int> pgm;
};

enum vartype { varsigned, varunsigned, varfloat };
struct DPFSubstring {
    std::string string;
    bool needs_value;
    vartype type;
    uint64_t longval = 0;
};

struct DPFOutputRecord {
    uint32_t size;
    uint32_t shader_id;
    uint32_t instruction_position;
    uint32_t stage;
    uint32_t stage_word_1;
    uint32_t stage_word_2;
    uint32_t stage_word_3;
    uint32_t format_string_id;
    uint32_t values;
};

class DebugPrintf : public ValidationStateTracker {
    VkPhysicalDeviceFeatures supported_features;

    uint32_t unique_shader_module_id = 0;
    std::unordered_map<VkCommandBuffer, std::vector<DPFBufferInfo>> command_buffer_map;
    uint32_t output_buffer_size;

  public:
    DebugPrintf() { container_type = LayerObjectTypeDebugPrintf; }

    bool aborted = false;
    bool verbose = false;
    bool use_stdout = false;
    VkDevice device;
    VkPhysicalDevice physicalDevice;
    uint32_t adjusted_max_desc_sets;
    uint32_t desc_set_bind_index;
    VkDescriptorSetLayout debug_desc_layout = VK_NULL_HANDLE;
    VkDescriptorSetLayout dummy_desc_layout = VK_NULL_HANDLE;
    std::unique_ptr<UtilDescriptorSetManager> desc_set_manager;
    std::unordered_map<uint32_t, DPFShaderTracker> shader_map;
    PFN_vkSetDeviceLoaderData vkSetDeviceLoaderData;
    VmaAllocator vmaAllocator = {};
    std::map<VkQueue, UtilQueueBarrierCommandInfo> queue_barrier_command_infos;
    std::vector<DPFBufferInfo>& GetBufferInfo(const VkCommandBuffer command_buffer) {
        auto buffer_list = command_buffer_map.find(command_buffer);
        if (buffer_list == command_buffer_map.end()) {
            std::vector<DPFBufferInfo> new_list{};
            command_buffer_map[command_buffer] = new_list;
            return command_buffer_map[command_buffer];
        }
        return buffer_list->second;
    }

    template <typename T>
    void ReportSetupProblem(T object, const char* const specific_message) const;
    void PreCallRecordCreateDevice(VkPhysicalDevice gpu, const VkDeviceCreateInfo* pCreateInfo,
                                   const VkAllocationCallbacks* pAllocator, VkDevice* pDevice,
                                   safe_VkDeviceCreateInfo* modified_create_info);
    void PostCallRecordCreateDevice(VkPhysicalDevice gpu, const VkDeviceCreateInfo* pCreateInfo,
                                    const VkAllocationCallbacks* pAllocator, VkDevice* pDevice, VkResult result);
    void PreCallRecordDestroyDevice(VkDevice device, const VkAllocationCallbacks* pAllocator);
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
    void PreCallRecordCreateRayTracingPipelinesKHR(VkDevice device, VkPipelineCache pipelineCache, uint32_t count,
                                                   const VkRayTracingPipelineCreateInfoKHR* pCreateInfos,
                                                   const VkAllocationCallbacks* pAllocator, VkPipeline* pPipelines,
                                                   void* crtpl_state_data);
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
    std::vector<DPFSubstring> ParseFormatString(std::string format_string);
    std::string FindFormatString(std::vector<unsigned int> pgm, uint32_t string_id);
    void AnalyzeAndGenerateMessages(VkCommandBuffer command_buffer, VkQueue queue, VkPipelineBindPoint pipeline_bind_point,
                                    uint32_t operation_index, uint32_t* const debug_output_buffer);
    void PreCallRecordCmdDraw(VkCommandBuffer commandBuffer, uint32_t vertexCount, uint32_t instanceCount, uint32_t firstVertex,
                              uint32_t firstInstance);
    void PreCallRecordCmdDrawIndexed(VkCommandBuffer commandBuffer, uint32_t indexCount, uint32_t instanceCount,
                                     uint32_t firstIndex, int32_t vertexOffset, uint32_t firstInstance);
    void PreCallRecordCmdDrawIndirect(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset, uint32_t count,
                                      uint32_t stride);
    void PreCallRecordCmdDrawIndexedIndirect(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset, uint32_t count,
                                             uint32_t stride);
    void PreCallRecordCmdDrawIndirectCountKHR(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                              VkBuffer countBuffer, VkDeviceSize countBufferOffset, uint32_t maxDrawCount,
                                              uint32_t stride);
    void PreCallRecordCmdDrawIndirectCount(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                           VkBuffer countBuffer, VkDeviceSize countBufferOffset, uint32_t maxDrawCount,
                                           uint32_t stride);
    void PreCallRecordCmdDrawIndexedIndirectCountKHR(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                     VkBuffer countBuffer, VkDeviceSize countBufferOffset, uint32_t maxDrawCount,
                                                     uint32_t stride);
    void PreCallRecordCmdDrawIndexedIndirectCount(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                  VkBuffer countBuffer, VkDeviceSize countBufferOffset, uint32_t maxDrawCount,
                                                  uint32_t stride);
    void PreCallRecordCmdDrawMeshTasksNV(VkCommandBuffer commandBuffer, uint32_t taskCount, uint32_t firstTask);
    void PreCallRecordCmdDrawMeshTasksIndirectNV(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                 uint32_t drawCount, uint32_t stride);
    void PreCallRecordCmdDrawMeshTasksIndirectCountNV(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                      VkBuffer countBuffer, VkDeviceSize countBufferOffset, uint32_t maxDrawCount,
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
    void PreCallRecordCmdTraceRaysKHR(VkCommandBuffer commandBuffer, const VkStridedBufferRegionKHR* pRaygenShaderBindingTable,
                                      const VkStridedBufferRegionKHR* pMissShaderBindingTable,
                                      const VkStridedBufferRegionKHR* pHitShaderBindingTable,
                                      const VkStridedBufferRegionKHR* pCallableShaderBindingTable, uint32_t width, uint32_t height,
                                      uint32_t depth);
    void PostCallRecordCmdTraceRaysKHR(VkCommandBuffer commandBuffer, const VkStridedBufferRegionKHR* pRaygenShaderBindingTable,
                                       const VkStridedBufferRegionKHR* pMissShaderBindingTable,
                                       const VkStridedBufferRegionKHR* pHitShaderBindingTable,
                                       const VkStridedBufferRegionKHR* pCallableShaderBindingTable, uint32_t width, uint32_t height,
                                       uint32_t depth);
    void PreCallRecordCmdTraceRaysIndirectKHR(VkCommandBuffer commandBuffer,
                                              const VkStridedBufferRegionKHR* pRaygenShaderBindingTable,
                                              const VkStridedBufferRegionKHR* pMissShaderBindingTable,
                                              const VkStridedBufferRegionKHR* pHitShaderBindingTable,
                                              const VkStridedBufferRegionKHR* pCallableShaderBindingTable, VkBuffer buffer,
                                              VkDeviceSize offset);
    void PostCallRecordCmdTraceRaysIndirectKHR(VkCommandBuffer commandBuffer,
                                               const VkStridedBufferRegionKHR* pRaygenShaderBindingTable,
                                               const VkStridedBufferRegionKHR* pMissShaderBindingTable,
                                               const VkStridedBufferRegionKHR* pHitShaderBindingTable,
                                               const VkStridedBufferRegionKHR* pCallableShaderBindingTable, VkBuffer buffer,
                                               VkDeviceSize offset);
    void PostCallRecordQueueSubmit(VkQueue queue, uint32_t submitCount, const VkSubmitInfo* pSubmits, VkFence fence,
                                   VkResult result);
    void AllocateDebugPrintfResources(const VkCommandBuffer cmd_buffer, const VkPipelineBindPoint bind_point);
};
