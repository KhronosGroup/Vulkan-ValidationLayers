/* Copyright (c) 2020-2023 The Khronos Group Inc.
 * Copyright (c) 2020-2023 Valve Corporation
 * Copyright (c) 2020-2023 LunarG, Inc.
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

namespace debug_printf_state {
class CommandBuffer : public gpu_utils_state::CommandBuffer {
  public:
    std::vector<DPFBufferInfo> buffer_infos;

    CommandBuffer(DebugPrintf* dp, VkCommandBuffer cb, const VkCommandBufferAllocateInfo* create_info,
                  const COMMAND_POOL_STATE* pool);
    ~CommandBuffer();

    bool NeedsProcessing() const final { return !buffer_infos.empty(); }
    void Process(VkQueue queue) final;

    void Destroy() final;
    void Reset() final;

  private:
    void ResetCBState();
};
}  // namespace debug_printf_state

VALSTATETRACK_DERIVED_STATE_OBJECT(VkCommandBuffer, debug_printf_state::CommandBuffer, CMD_BUFFER_STATE)

class DebugPrintf : public GpuAssistedBase {
  public:
    DebugPrintf() {
        setup_vuid = "UNASSIGNED-DEBUG-PRINTF";
        container_type = LayerObjectTypeDebugPrintf;
        desired_features.vertexPipelineStoresAndAtomics = true;
        desired_features.fragmentStoresAndAtomics = true;
    }

    void CreateDevice(const VkDeviceCreateInfo* pCreateInfo) override;
    bool InstrumentShader(const vvl::span<const uint32_t>& input, std::vector<uint32_t>& new_pgm,
                          uint32_t unique_shader_id) override;
    void PreCallRecordCreateShaderModule(VkDevice device, const VkShaderModuleCreateInfo* pCreateInfo,
                                         const VkAllocationCallbacks* pAllocator, VkShaderModule* pShaderModule,
                                         void* csm_state_data) override;
    void PreCallRecordCreateShadersEXT(VkDevice device, uint32_t createInfoCount, const VkShaderCreateInfoEXT* pCreateInfos,
                                       const VkAllocationCallbacks* pAllocator, VkShaderEXT* pShaders,
                                       void* csm_state_data) override;
    std::vector<DPFSubstring> ParseFormatString(const std::string& format_string);
    std::string FindFormatString(vvl::span<const uint32_t> pgm, uint32_t string_id);
    void AnalyzeAndGenerateMessages(VkCommandBuffer command_buffer, VkQueue queue, DPFBufferInfo& buffer_info,
                                    uint32_t operation_index, uint32_t* const debug_output_buffer);
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
    void PreCallRecordCmdDrawIndexedIndirectCountKHR(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                     VkBuffer countBuffer, VkDeviceSize countBufferOffset, uint32_t maxDrawCount,
                                                     uint32_t stride) override;
    void PreCallRecordCmdDrawIndexedIndirectCount(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                  VkBuffer countBuffer, VkDeviceSize countBufferOffset, uint32_t maxDrawCount,
                                                  uint32_t stride) override;
    void PreCallRecordCmdDrawIndirectByteCountEXT(VkCommandBuffer commandBuffer, uint32_t instanceCount, uint32_t firstInstance,
                                                  VkBuffer counterBuffer, VkDeviceSize counterBufferOffset, uint32_t counterOffset,
                                                  uint32_t vertexStride) override;
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
    void AllocateDebugPrintfResources(const VkCommandBuffer cmd_buffer, const VkPipelineBindPoint bind_point);

    std::shared_ptr<CMD_BUFFER_STATE> CreateCmdBufferState(VkCommandBuffer cb, const VkCommandBufferAllocateInfo* create_info,
                                                           const COMMAND_POOL_STATE* pool) final;

    void DestroyBuffer(DPFBufferInfo& buffer_info);

  private:
    bool verbose = false;
    bool use_stdout = false;
};
