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

#include "gpu/core/gpu_state_tracker.h"
#include "gpu/instrumentation/gpu_shader_instrumentor.h"
#include "gpu/resources/gpu_resources.h"

namespace debug_printf {

class Validator;

struct BufferInfo {
    gpu::DeviceMemoryBlock output_mem_block;
    VkDescriptorSet desc_set;
    VkDescriptorPool desc_pool;
    VkPipelineBindPoint pipeline_bind_point;
    BufferInfo(gpu::DeviceMemoryBlock output_mem_block, VkDescriptorSet desc_set, VkDescriptorPool desc_pool,
               VkPipelineBindPoint pipeline_bind_point)
        : output_mem_block(output_mem_block), desc_set(desc_set), desc_pool(desc_pool), pipeline_bind_point(pipeline_bind_point){};
};

class CommandBuffer : public gpu_tracker::CommandBuffer {
  public:
    std::vector<BufferInfo> buffer_infos;

    CommandBuffer(Validator& dp, VkCommandBuffer handle, const VkCommandBufferAllocateInfo* create_info,
                  const vvl::CommandPool* pool);
    ~CommandBuffer();

    bool PreProcess(const Location& loc) final { return !buffer_infos.empty(); }
    void PostProcess(VkQueue queue, const Location& loc) final;

    void Destroy() final;
    void Reset(const Location& loc) final;

  private:
    void ResetCBState();
};
}  // namespace debug_printf

VALSTATETRACK_DERIVED_STATE_OBJECT(VkCommandBuffer, debug_printf::CommandBuffer, vvl::CommandBuffer)

namespace debug_printf {
class Validator : public gpu::GpuShaderInstrumentor {
  public:
    using BaseClass = gpu::GpuShaderInstrumentor;
    Validator() { container_type = LayerObjectTypeDebugPrintf; }

    void PreCallRecordCreateDevice(VkPhysicalDevice physicalDevice, const VkDeviceCreateInfo* pCreateInfo,
                                   const VkAllocationCallbacks* pAllocator, VkDevice* pDevice, const RecordObject& record_obj,
                                   vku::safe_VkDeviceCreateInfo* modified_create_info) final;
    void PostCreateDevice(const VkDeviceCreateInfo* pCreateInfo, const Location& loc) override;
    void AnalyzeAndGenerateMessage(VkCommandBuffer command_buffer, VkQueue queue, BufferInfo& buffer_info, u32 operation_index,
                                   u32* const debug_output_buffer, const Location& loc);
    void PreCallRecordCmdDraw(VkCommandBuffer commandBuffer, u32 vertexCount, u32 instanceCount, u32 firstVertex, u32 firstInstance,
                              const RecordObject& record_obj) override;
    void PreCallRecordCmdDrawMultiEXT(VkCommandBuffer commandBuffer, u32 drawCount, const VkMultiDrawInfoEXT* pVertexInfo,
                                      u32 instanceCount, u32 firstInstance, u32 stride, const RecordObject& record_obj) override;
    void PreCallRecordCmdDrawIndexed(VkCommandBuffer commandBuffer, u32 indexCount, u32 instanceCount, u32 firstIndex,
                                     i32 vertexOffset, u32 firstInstance, const RecordObject& record_obj) override;
    void PreCallRecordCmdDrawMultiIndexedEXT(VkCommandBuffer commandBuffer, u32 drawCount,
                                             const VkMultiDrawIndexedInfoEXT* pIndexInfo, u32 instanceCount, u32 firstInstance,
                                             u32 stride, const i32* pVertexOffset, const RecordObject& record_obj) override;
    void PreCallRecordCmdDrawIndirect(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset, u32 count, u32 stride,
                                      const RecordObject& record_obj) override;
    void PreCallRecordCmdDrawIndexedIndirect(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset, u32 count,
                                             u32 stride, const RecordObject& record_obj) override;
    void PreCallRecordCmdDrawIndirectCountKHR(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                              VkBuffer countBuffer, VkDeviceSize countBufferOffset, u32 maxDrawCount, u32 stride,
                                              const RecordObject& record_obj) override;
    void PreCallRecordCmdDrawIndirectCount(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                           VkBuffer countBuffer, VkDeviceSize countBufferOffset, u32 maxDrawCount, u32 stride,
                                           const RecordObject& record_obj) override;
    void PreCallRecordCmdDrawIndexedIndirectCountKHR(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                     VkBuffer countBuffer, VkDeviceSize countBufferOffset, u32 maxDrawCount,
                                                     u32 stride, const RecordObject& record_obj) override;
    void PreCallRecordCmdDrawIndexedIndirectCount(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                  VkBuffer countBuffer, VkDeviceSize countBufferOffset, u32 maxDrawCount,
                                                  u32 stride, const RecordObject& record_obj) override;
    void PreCallRecordCmdDrawIndirectByteCountEXT(VkCommandBuffer commandBuffer, u32 instanceCount, u32 firstInstance,
                                                  VkBuffer counterBuffer, VkDeviceSize counterBufferOffset, u32 counterOffset,
                                                  u32 vertexStride, const RecordObject& record_obj) override;
    void PreCallRecordCmdDrawMeshTasksNV(VkCommandBuffer commandBuffer, u32 taskCount, u32 firstTask,
                                         const RecordObject& record_obj) override;
    void PreCallRecordCmdDrawMeshTasksIndirectNV(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset, u32 drawCount,
                                                 u32 stride, const RecordObject& record_obj) override;
    void PreCallRecordCmdDrawMeshTasksIndirectCountNV(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                      VkBuffer countBuffer, VkDeviceSize countBufferOffset, u32 maxDrawCount,
                                                      u32 stride, const RecordObject& record_obj) override;
    void PreCallRecordCmdDrawMeshTasksEXT(VkCommandBuffer commandBuffer, u32 groupCountX, u32 groupCountY, u32 groupCountZ,
                                          const RecordObject& record_obj) override;
    void PreCallRecordCmdDrawMeshTasksIndirectEXT(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                  u32 drawCount, u32 stride, const RecordObject& record_obj) override;
    void PreCallRecordCmdDrawMeshTasksIndirectCountEXT(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                       VkBuffer countBuffer, VkDeviceSize countBufferOffset, u32 maxDrawCount,
                                                       u32 stride, const RecordObject& record_obj) override;
    void PreCallRecordCmdDispatch(VkCommandBuffer commandBuffer, u32 x, u32 y, u32 z, const RecordObject& record_obj) override;
    void PreCallRecordCmdDispatchIndirect(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                          const RecordObject& record_obj) override;
    void PreCallRecordCmdDispatchBase(VkCommandBuffer commandBuffer, u32 baseGroupX, u32 baseGroupY, u32 baseGroupZ,
                                      u32 groupCountX, u32 groupCountY, u32 groupCountZ, const RecordObject& record_obj) override;
    void PreCallRecordCmdDispatchBaseKHR(VkCommandBuffer commandBuffer, u32 baseGroupX, u32 baseGroupY, u32 baseGroupZ,
                                         u32 groupCountX, u32 groupCountY, u32 groupCountZ,
                                         const RecordObject& record_obj) override;
    void PreCallRecordCmdTraceRaysNV(VkCommandBuffer commandBuffer, VkBuffer raygenShaderBindingTableBuffer,
                                     VkDeviceSize raygenShaderBindingOffset, VkBuffer missShaderBindingTableBuffer,
                                     VkDeviceSize missShaderBindingOffset, VkDeviceSize missShaderBindingStride,
                                     VkBuffer hitShaderBindingTableBuffer, VkDeviceSize hitShaderBindingOffset,
                                     VkDeviceSize hitShaderBindingStride, VkBuffer callableShaderBindingTableBuffer,
                                     VkDeviceSize callableShaderBindingOffset, VkDeviceSize callableShaderBindingStride, u32 width,
                                     u32 height, u32 depth, const RecordObject& record_obj) override;
    void PreCallRecordCmdTraceRaysKHR(VkCommandBuffer commandBuffer,
                                      const VkStridedDeviceAddressRegionKHR* pRaygenShaderBindingTable,
                                      const VkStridedDeviceAddressRegionKHR* pMissShaderBindingTable,
                                      const VkStridedDeviceAddressRegionKHR* pHitShaderBindingTable,
                                      const VkStridedDeviceAddressRegionKHR* pCallableShaderBindingTable, u32 width, u32 height,
                                      u32 depth, const RecordObject& record_obj) override;
    void PreCallRecordCmdTraceRaysIndirectKHR(VkCommandBuffer commandBuffer,
                                              const VkStridedDeviceAddressRegionKHR* pRaygenShaderBindingTable,
                                              const VkStridedDeviceAddressRegionKHR* pMissShaderBindingTable,
                                              const VkStridedDeviceAddressRegionKHR* pHitShaderBindingTable,
                                              const VkStridedDeviceAddressRegionKHR* pCallableShaderBindingTable,
                                              VkDeviceAddress indirectDeviceAddress, const RecordObject& record_obj) override;
    void PreCallRecordCmdTraceRaysIndirect2KHR(VkCommandBuffer commandBuffer, VkDeviceAddress indirectDeviceAddress,
                                               const RecordObject& record_obj) override;
    void AllocateDebugPrintfResources(const VkCommandBuffer cmd_buffer, const VkPipelineBindPoint bind_point, const Location& loc);

    std::shared_ptr<vvl::CommandBuffer> CreateCmdBufferState(VkCommandBuffer cb, const VkCommandBufferAllocateInfo* create_info,
                                                             const vvl::CommandPool* pool) final;

    void DestroyBuffer(BufferInfo& buffer_info);

  private:
    bool verbose = false;
    bool use_stdout = false;
};
}  // namespace debug_printf
