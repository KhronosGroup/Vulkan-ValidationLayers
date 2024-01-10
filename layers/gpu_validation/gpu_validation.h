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

#include "gpu_validation/gpu_state_tracker.h"
#include "gpu_validation/gpu_error_message.h"
#include "gpu_validation/gpu_descriptor_set.h"
#include "gpu_validation/gpu_subclasses.h"
#include "state_tracker/pipeline_state.h"

typedef vvl::unordered_map<const vvl::Image*, std::optional<GlobalImageLayoutRangeMap>> GlobalImageLayoutMap;

namespace gpuav {
struct GpuVuid {
    const char* uniform_access_oob = kVUIDUndefined;
    const char* storage_access_oob = kVUIDUndefined;
    const char* invalid_descriptor = kVUIDUndefined;
    const char* count_exceeds_bufsize_1 = kVUIDUndefined;
    const char* count_exceeds_bufsize = kVUIDUndefined;
    const char* count_exceeds_device_limit = kVUIDUndefined;
    const char* first_instance_not_zero = kVUIDUndefined;
    const char* group_exceeds_device_limit_x = kVUIDUndefined;
    const char* group_exceeds_device_limit_y = kVUIDUndefined;
    const char* group_exceeds_device_limit_z = kVUIDUndefined;
    const char* mesh_group_count_exceeds_max_x = kVUIDUndefined;
    const char* mesh_group_count_exceeds_max_y = kVUIDUndefined;
    const char* mesh_group_count_exceeds_max_z = kVUIDUndefined;
    const char* mesh_group_count_exceeds_max_total = kVUIDUndefined;
    const char* task_group_count_exceeds_max_x = kVUIDUndefined;
    const char* task_group_count_exceeds_max_y = kVUIDUndefined;
    const char* task_group_count_exceeds_max_z = kVUIDUndefined;
    const char* task_group_count_exceeds_max_total = kVUIDUndefined;
    // vkCmdTraceRaysIndirectKHR
    const char* trace_rays_width_exceeds_device_limit = kVUIDUndefined;
    const char* trace_rays_height_exceeds_device_limit = kVUIDUndefined;
    const char* trace_rays_depth_exceeds_device_limit = kVUIDUndefined;
};
}  // namespace gpuav

VALSTATETRACK_DERIVED_STATE_OBJECT(VkAccelerationStructureKHR, gpuav::AccelerationStructureKHR, vvl::AccelerationStructureKHR)
VALSTATETRACK_DERIVED_STATE_OBJECT(VkAccelerationStructureNV, gpuav::AccelerationStructureNV, vvl::AccelerationStructureNV)
VALSTATETRACK_DERIVED_STATE_OBJECT(VkBuffer, gpuav::Buffer, vvl::Buffer)
VALSTATETRACK_DERIVED_STATE_OBJECT(VkBufferView, gpuav::BufferView, vvl::BufferView)
VALSTATETRACK_DERIVED_STATE_OBJECT(VkCommandBuffer, gpuav::CommandBuffer, vvl::CommandBuffer)
VALSTATETRACK_DERIVED_STATE_OBJECT(VkDescriptorSet, gpuav::DescriptorSet, vvl::DescriptorSet)
VALSTATETRACK_DERIVED_STATE_OBJECT(VkImageView, gpuav::ImageView, vvl::ImageView)
VALSTATETRACK_DERIVED_STATE_OBJECT(VkSampler, gpuav::Sampler, vvl::Sampler)

namespace gpuav {

class Validator : public gpu_tracker::Validator {
    using BaseClass = gpu_tracker::Validator;
    using Func = vvl::Func;
    using Struct = vvl::Struct;
    using Field = vvl::Field;
    using ImageBarrier = sync_utils::ImageBarrier;

  public:
    Validator() {
        setup_vuid = "UNASSIGNED-GPU-Assisted-Validation";
        container_type = LayerObjectTypeGpuAssisted;
        desired_features.vertexPipelineStoresAndAtomics = true;
        desired_features.fragmentStoresAndAtomics = true;
        desired_features.shaderInt64 = true;
        force_buffer_device_address = true;
    }

    // gpu_validation.cpp
    // ------------------
  public:
    VkDeviceAddress GetBufferDeviceAddress(VkBuffer buffer) const;
    bool CheckForDescriptorIndexing(DeviceFeatures enabled_features) const;
    bool InstrumentShader(const vvl::span<const uint32_t>& input, std::vector<uint32_t>& new_pgm, uint32_t unique_shader_id,
                          const Location& loc) override;
    bool CheckForCachedInstrumentedShader(const uint32_t shader_hash, create_shader_module_api_state* csm_state);
    bool CheckForCachedInstrumentedShader(const uint32_t index, const uint32_t shader_hash,
                                          create_shader_object_api_state* cso_state);
    void UpdateInstrumentationBuffer(CommandBuffer* cb_node);
    void UpdateBDABuffer(DeviceMemoryBlock buffer_device_addresses);

    void UpdateBoundDescriptors(VkCommandBuffer commandBuffer, VkPipelineBindPoint pipelineBindPoint);

    // Allocate per command validation resources
    [[nodiscard]] CommandResources AllocateCommandResources(const VkCommandBuffer cmd_buffer, const VkPipelineBindPoint bind_point,
                                                            Func command, const CmdIndirectState* indirect_state = nullptr);
    [[nodiscard]] std::unique_ptr<CommandResources> AllocatePreDrawIndirectValidationResources(
        vvl::Func command, VkCommandBuffer cmd_buffer, VkBuffer indirect_buffer, VkDeviceSize indirect_offset, uint32_t draw_count,
        VkBuffer count_buffer, VkDeviceSize count_buffer_offset, uint32_t stride);
    [[nodiscard]] std::unique_ptr<CommandResources> AllocatePreDispatchIndirectValidationResources(vvl::Func command,
                                                                                                   VkCommandBuffer cmd_buffer,
                                                                                                   VkBuffer indirect_buffer,
                                                                                                   VkDeviceSize indirect_offset);
    [[nodiscard]] std::unique_ptr<CommandResources> AllocatePreTraceRaysValidationResources(vvl::Func command,
                                                                                            VkCommandBuffer cmd_buffer,
                                                                                            VkDeviceAddress indirect_data_address);

  private:
    void AllocateSharedTraceRaysValidationResources();
    void AllocateSharedDrawIndirectValidationResources(bool use_shader_objects);
    void AllocateSharedDispatchIndirectValidationResources(bool use_shader_objects);
    void StoreCommandResources(const VkCommandBuffer cmd_buffer, std::unique_ptr<CommandResources> command_resources);

    // gpu_error_message.cpp
    // ---------------------
  public:
    // Return true iff a error has been found in the internal call to GenerateValidationMessage
    bool AnalyzeAndGenerateMessages(VkCommandBuffer cmd_buffer, VkQueue queue, CommandResources& cmd_resources,
                                    uint32_t operation_index, uint32_t* const debug_output_buffer,
                                    const std::vector<DescSetState>& descriptor_sets, const Location& loc);

  private:
    // Return true iff an error has been found in debug_record, among the list of errors this function manages
    bool GenerateValidationMessage(const uint32_t* debug_record, const CommandResources& cmd_resources,
                                   const std::vector<DescSetState>& descriptor_sets, std::string& out_error_msg,
                                   std::string& out_vuid_msg, bool& out_oob_access) const;

    // gpu_setup.cpp
    // -------------
  public:
    std::shared_ptr<vvl::Buffer> CreateBufferState(VkBuffer buf, const VkBufferCreateInfo* pCreateInfo) final;
    std::shared_ptr<vvl::BufferView> CreateBufferViewState(const std::shared_ptr<vvl::Buffer>& bf, VkBufferView bv,
                                                           const VkBufferViewCreateInfo* ci, VkFormatFeatureFlags2KHR buf_ff) final;
    std::shared_ptr<vvl::ImageView> CreateImageViewState(const std::shared_ptr<vvl::Image>& image_state, VkImageView iv,
                                                         const VkImageViewCreateInfo* ci, VkFormatFeatureFlags2KHR ff,
                                                         const VkFilterCubicImageViewImageFormatPropertiesEXT& cubic_props) final;
    std::shared_ptr<vvl::AccelerationStructureNV> CreateAccelerationStructureState(
        VkAccelerationStructureNV as, const VkAccelerationStructureCreateInfoNV* pCreateInfo) final;
    std::shared_ptr<vvl::AccelerationStructureKHR> CreateAccelerationStructureState(
        VkAccelerationStructureKHR as, const VkAccelerationStructureCreateInfoKHR* pCreateInfo,
        std::shared_ptr<vvl::Buffer>&& buf_state, VkDeviceAddress address) final;
    std::shared_ptr<vvl::Sampler> CreateSamplerState(VkSampler s, const VkSamplerCreateInfo* ci) final;
    std::shared_ptr<vvl::CommandBuffer> CreateCmdBufferState(VkCommandBuffer cb, const VkCommandBufferAllocateInfo* create_info,
                                                             const vvl::CommandPool* pool) final;
    std::shared_ptr<vvl::DescriptorSet> CreateDescriptorSet(VkDescriptorSet, vvl::DescriptorPool*,
                                                            const std::shared_ptr<vvl::DescriptorSetLayout const>& layout,
                                                            uint32_t variable_count) final;

    void CreateDevice(const VkDeviceCreateInfo* pCreateInfo) final;
    void CreateAccelerationStructureBuildValidationState(const VkDeviceCreateInfo* pCreateInfo);

    void Destroy(AccelerationStructureBuildValidationInfo& as_validation_info);

    // gpu_record.cpp
    // --------------
  public:
    void PreCallRecordDestroyDevice(VkDevice device, const VkAllocationCallbacks* pAllocator,
                                    const RecordObject& record_obj) override;
    void PostCallRecordBindAccelerationStructureMemoryNV(VkDevice device, uint32_t bindInfoCount,
                                                         const VkBindAccelerationStructureMemoryInfoNV* pBindInfos,
                                                         const RecordObject& record_obj) override;

    void PreCallRecordCreateBuffer(VkDevice device, const VkBufferCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator,
                                   VkBuffer* pBuffer, const RecordObject& record_obj, void* cb_state_data) override;

    void PreCallRecordCmdBuildAccelerationStructureNV(VkCommandBuffer commandBuffer, const VkAccelerationStructureInfoNV* pInfo,
                                                      VkBuffer instanceData, VkDeviceSize instanceOffset, VkBool32 update,
                                                      VkAccelerationStructureNV dst, VkAccelerationStructureNV src,
                                                      VkBuffer scratch, VkDeviceSize scratchOffset,
                                                      const RecordObject& record_obj) override;
    void PreCallRecordDestroyRenderPass(VkDevice device, VkRenderPass renderPass, const VkAllocationCallbacks* pAllocator,
                                        const RecordObject& record_obj) override;

    void PreCallRecordCreateShaderModule(VkDevice device, const VkShaderModuleCreateInfo* pCreateInfo,
                                         const VkAllocationCallbacks* pAllocator, VkShaderModule* pShaderModule,
                                         const RecordObject& record_obj, void* csm_state_data) override;
    void PreCallRecordCreateShadersEXT(VkDevice device, uint32_t createInfoCount, const VkShaderCreateInfoEXT* pCreateInfos,
                                       const VkAllocationCallbacks* pAllocator, VkShaderEXT* pShaders,
                                       const RecordObject& record_obj, void* csm_state_data) override;
    void RecordCmdBeginRenderPassLayouts(VkCommandBuffer commandBuffer, const VkRenderPassBeginInfo* pRenderPassBegin,
                                         const VkSubpassContents contents);
    void RecordCmdEndRenderPassLayouts(VkCommandBuffer commandBuffer);
    void PreCallRecordCmdBeginRenderPass(VkCommandBuffer commandBuffer, const VkRenderPassBeginInfo* pRenderPassBegin,
                                         VkSubpassContents contents, const RecordObject&) override;
    void PreCallRecordCmdBeginRenderPass2KHR(VkCommandBuffer commandBuffer, const VkRenderPassBeginInfo* pRenderPassBegin,
                                             const VkSubpassBeginInfo* pSubpassBeginInfo, const RecordObject&) override;
    void PreCallRecordCmdBeginRenderPass2(VkCommandBuffer commandBuffer, const VkRenderPassBeginInfo* pRenderPassBegin,
                                          const VkSubpassBeginInfo* pSubpassBeginInfo, const RecordObject&) override;

    void RecordCmdNextSubpassLayouts(VkCommandBuffer commandBuffer, VkSubpassContents contents);
    void PostCallRecordCmdNextSubpass(VkCommandBuffer commandBuffer, VkSubpassContents contents,
                                      const RecordObject& record_obj) override;
    void PostCallRecordCmdNextSubpass2KHR(VkCommandBuffer commandBuffer, const VkSubpassBeginInfo* pSubpassBeginInfo,
                                          const VkSubpassEndInfo* pSubpassEndInfo, const RecordObject& record_obj) override;
    void PostCallRecordCmdNextSubpass2(VkCommandBuffer commandBuffer, const VkSubpassBeginInfo* pSubpassBeginInfo,
                                       const VkSubpassEndInfo* pSubpassEndInfo, const RecordObject& record_obj) override;

    void PostCallRecordCmdEndRenderPass(VkCommandBuffer commandBuffer, const RecordObject& record_obj) override;
    void PostCallRecordCmdEndRenderPass2KHR(VkCommandBuffer commandBuffer, const VkSubpassEndInfo* pSubpassEndInfo,
                                            const RecordObject& record_obj) override;
    void PostCallRecordCmdEndRenderPass2(VkCommandBuffer commandBuffer, const VkSubpassEndInfo* pSubpassEndInfo,
                                         const RecordObject& record_obj) override;

    void PostCallRecordCmdBindDescriptorSets2KHR(VkCommandBuffer commandBuffer,
                                                 const VkBindDescriptorSetsInfoKHR* pBindDescriptorSetsInfo,
                                                 const RecordObject& record_obj) override;
    void PostCallRecordCmdBindDescriptorSets(VkCommandBuffer commandBuffer, VkPipelineBindPoint pipelineBindPoint,
                                             VkPipelineLayout layout, uint32_t firstSet, uint32_t descriptorSetCount,
                                             const VkDescriptorSet* pDescriptorSets, uint32_t dynamicOffsetCount,
                                             const uint32_t* pDynamicOffsets, const RecordObject& record_obj) override;
    void PreCallRecordCmdPushDescriptorSetKHR(VkCommandBuffer commandBuffer, VkPipelineBindPoint pipelineBindPoint,
                                              VkPipelineLayout layout, uint32_t set, uint32_t descriptorWriteCount,
                                              const VkWriteDescriptorSet* pDescriptorWrites, const RecordObject&) override;
    void PreCallRecordCmdPushDescriptorSet2KHR(VkCommandBuffer commandBuffer,
                                               const VkPushDescriptorSetInfoKHR* pPushDescriptorSetInfo,
                                               const RecordObject& record_obj) override;
    void PreCallRecordQueueSubmit(VkQueue queue, uint32_t submitCount, const VkSubmitInfo* pSubmits, VkFence fence,
                                  const RecordObject&) override;
    void PostCallRecordQueueSubmit(VkQueue queue, uint32_t submitCount, const VkSubmitInfo* pSubmits, VkFence fence,
                                   const RecordObject& record_obj) override;
    void PreCallRecordQueueSubmit2KHR(VkQueue queue, uint32_t submitCount, const VkSubmitInfo2KHR* pSubmits, VkFence fence,
                                      const RecordObject&) override;
    void PreCallRecordQueueSubmit2(VkQueue queue, uint32_t submitCount, const VkSubmitInfo2* pSubmits, VkFence fence,
                                   const RecordObject&) override;
    void PostCallRecordQueueSubmit2KHR(VkQueue queue, uint32_t submitCount, const VkSubmitInfo2KHR* pSubmits, VkFence fence,
                                       const RecordObject& record_obj) override;
    void PostCallRecordQueueSubmit2(VkQueue queue, uint32_t submitCount, const VkSubmitInfo2KHR* pSubmits, VkFence fence,
                                    const RecordObject& record_obj) override;
    void PreCallRecordCmdBindDescriptorBuffersEXT(VkCommandBuffer commandBuffer, uint32_t bufferCount,
                                                  const VkDescriptorBufferBindingInfoEXT* pBindingInfos,
                                                  const RecordObject& record_obj) override;
    void PreCallRecordCmdBindDescriptorBufferEmbeddedSamplersEXT(VkCommandBuffer commandBuffer,
                                                                 VkPipelineBindPoint pipelineBindPoint, VkPipelineLayout layout,
                                                                 uint32_t set, const RecordObject& record_obj) override;
    void PreCallRecordCmdBindDescriptorBufferEmbeddedSamplers2EXT(
        VkCommandBuffer commandBuffer,
        const VkBindDescriptorBufferEmbeddedSamplersInfoEXT* pBindDescriptorBufferEmbeddedSamplersInfo,
        const RecordObject& record_obj) override;
    void PreCallRecordCmdDraw(VkCommandBuffer commandBuffer, uint32_t vertexCount, uint32_t instanceCount, uint32_t firstVertex,
                              uint32_t firstInstance, const RecordObject& record_obj) override;
    void PreCallRecordCmdDrawMultiEXT(VkCommandBuffer commandBuffer, uint32_t drawCount, const VkMultiDrawInfoEXT* pVertexInfo,
                                      uint32_t instanceCount, uint32_t firstInstance, uint32_t stride,
                                      const RecordObject& record_obj) override;
    void PreCallRecordCmdDrawIndexed(VkCommandBuffer commandBuffer, uint32_t indexCount, uint32_t instanceCount,
                                     uint32_t firstIndex, int32_t vertexOffset, uint32_t firstInstance,
                                     const RecordObject& record_obj) override;
    void PreCallRecordCmdDrawMultiIndexedEXT(VkCommandBuffer commandBuffer, uint32_t drawCount,
                                             const VkMultiDrawIndexedInfoEXT* pIndexInfo, uint32_t instanceCount,
                                             uint32_t firstInstance, uint32_t stride, const int32_t* pVertexOffset,
                                             const RecordObject& record_obj) override;
    void PreCallRecordCmdDrawIndirect(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset, uint32_t count,
                                      uint32_t stride, const RecordObject& record_obj) override;
    void PreCallRecordCmdDrawIndexedIndirect(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset, uint32_t count,
                                             uint32_t stride, const RecordObject& record_obj) override;
    void PreCallRecordCmdDrawIndirectCountKHR(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                              VkBuffer countBuffer, VkDeviceSize countBufferOffset, uint32_t maxDrawCount,
                                              uint32_t stride, const RecordObject& record_obj) override;
    void PreCallRecordCmdDrawIndirectCount(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                           VkBuffer countBuffer, VkDeviceSize countBufferOffset, uint32_t maxDrawCount,
                                           uint32_t stride, const RecordObject& record_obj) override;
    void PreCallRecordCmdDrawIndirectByteCountEXT(VkCommandBuffer commandBuffer, uint32_t instanceCount, uint32_t firstInstance,
                                                  VkBuffer counterBuffer, VkDeviceSize counterBufferOffset, uint32_t counterOffset,
                                                  uint32_t vertexStride, const RecordObject& record_obj) override;
    void PreCallRecordCmdDrawIndexedIndirectCountKHR(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                     VkBuffer countBuffer, VkDeviceSize countBufferOffset, uint32_t maxDrawCount,
                                                     uint32_t stride, const RecordObject& record_obj) override;
    void PreCallRecordCmdDrawIndexedIndirectCount(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                  VkBuffer countBuffer, VkDeviceSize countBufferOffset, uint32_t maxDrawCount,
                                                  uint32_t stride, const RecordObject& record_obj) override;
    void PreCallRecordCmdDrawMeshTasksNV(VkCommandBuffer commandBuffer, uint32_t taskCount, uint32_t firstTask,
                                         const RecordObject& record_obj) override;
    void PreCallRecordCmdDrawMeshTasksIndirectNV(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                 uint32_t drawCount, uint32_t stride, const RecordObject& record_obj) override;
    void PreCallRecordCmdDrawMeshTasksIndirectCountNV(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                      VkBuffer countBuffer, VkDeviceSize countBufferOffset, uint32_t maxDrawCount,
                                                      uint32_t stride, const RecordObject& record_obj) override;
    void PreCallRecordCmdDrawMeshTasksEXT(VkCommandBuffer commandBuffer, uint32_t groupCountX, uint32_t groupCountY,
                                          uint32_t groupCountZ, const RecordObject& record_obj) override;
    void PreCallRecordCmdDrawMeshTasksIndirectEXT(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                  uint32_t drawCount, uint32_t stride, const RecordObject& record_obj) override;
    void PreCallRecordCmdDrawMeshTasksIndirectCountEXT(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                       VkBuffer countBuffer, VkDeviceSize countBufferOffset, uint32_t maxDrawCount,
                                                       uint32_t stride, const RecordObject& record_obj) override;
    void PreCallRecordCmdDispatch(VkCommandBuffer commandBuffer, uint32_t x, uint32_t y, uint32_t z,
                                  const RecordObject& record_obj) override;
    void PreCallRecordCmdDispatchIndirect(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                          const RecordObject& record_obj) override;
    void PreCallRecordCmdDispatchBase(VkCommandBuffer commandBuffer, uint32_t baseGroupX, uint32_t baseGroupY, uint32_t baseGroupZ,
                                      uint32_t groupCountX, uint32_t groupCountY, uint32_t groupCountZ,
                                      const RecordObject& record_obj) override;
    void PreCallRecordCmdDispatchBaseKHR(VkCommandBuffer commandBuffer, uint32_t baseGroupX, uint32_t baseGroupY,
                                         uint32_t baseGroupZ, uint32_t groupCountX, uint32_t groupCountY, uint32_t groupCountZ,
                                         const RecordObject& record_obj) override;
    void PreCallRecordCmdTraceRaysNV(VkCommandBuffer commandBuffer, VkBuffer raygenShaderBindingTableBuffer,
                                     VkDeviceSize raygenShaderBindingOffset, VkBuffer missShaderBindingTableBuffer,
                                     VkDeviceSize missShaderBindingOffset, VkDeviceSize missShaderBindingStride,
                                     VkBuffer hitShaderBindingTableBuffer, VkDeviceSize hitShaderBindingOffset,
                                     VkDeviceSize hitShaderBindingStride, VkBuffer callableShaderBindingTableBuffer,
                                     VkDeviceSize callableShaderBindingOffset, VkDeviceSize callableShaderBindingStride,
                                     uint32_t width, uint32_t height, uint32_t depth, const RecordObject& record_obj) override;
    void PreCallRecordCmdTraceRaysKHR(VkCommandBuffer commandBuffer,
                                      const VkStridedDeviceAddressRegionKHR* pRaygenShaderBindingTable,
                                      const VkStridedDeviceAddressRegionKHR* pMissShaderBindingTable,
                                      const VkStridedDeviceAddressRegionKHR* pHitShaderBindingTable,
                                      const VkStridedDeviceAddressRegionKHR* pCallableShaderBindingTable, uint32_t width,
                                      uint32_t height, uint32_t depth, const RecordObject& record_obj) override;
    void PreCallRecordCmdTraceRaysIndirectKHR(VkCommandBuffer commandBuffer,
                                              const VkStridedDeviceAddressRegionKHR* pRaygenShaderBindingTable,
                                              const VkStridedDeviceAddressRegionKHR* pMissShaderBindingTable,
                                              const VkStridedDeviceAddressRegionKHR* pHitShaderBindingTable,
                                              const VkStridedDeviceAddressRegionKHR* pCallableShaderBindingTable,
                                              VkDeviceAddress indirectDeviceAddress, const RecordObject& record_obj) override;
    void PreCallRecordCmdTraceRaysIndirect2KHR(VkCommandBuffer commandBuffer, VkDeviceAddress indirectDeviceAddress,
                                               const RecordObject& record_obj) override;

    void PostCallRecordGetPhysicalDeviceProperties(VkPhysicalDevice physicalDevice,
                                                   VkPhysicalDeviceProperties* pPhysicalDeviceProperties,
                                                   const RecordObject& record_obj) override;
    void PostCallRecordGetPhysicalDeviceProperties2(VkPhysicalDevice physicalDevice,
                                                    VkPhysicalDeviceProperties2* pPhysicalDeviceProperties2,
                                                    const RecordObject& record_obj) override;

    // gpu_image_layout.cpp
    // --------------------

    void TransitionAttachmentRefLayout(vvl::CommandBuffer* cb_state, const safe_VkAttachmentReference2& ref);

    void TransitionSubpassLayouts(vvl::CommandBuffer* cb_state, const vvl::RenderPass& render_pass_state, const int);
    void TransitionFinalSubpassLayouts(vvl::CommandBuffer* cb_state);

    void TransitionBeginRenderPassLayouts(vvl::CommandBuffer* cb_state, const vvl::RenderPass& render_pass_state);

    bool UpdateCommandBufferImageLayoutMap(const vvl::CommandBuffer* cb_state, const Location& image_loc,
                                           const ImageBarrier& img_barrier, const CommandBufferImageLayoutMap& current_map,
                                           CommandBufferImageLayoutMap& layout_updates) const;
    void PostCallRecordCreateImage(VkDevice device, const VkImageCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator,
                                   VkImage* pImage, const RecordObject& record_obj) override;
    void PreCallRecordDestroyImage(VkDevice device, VkImage image, const VkAllocationCallbacks* pAllocator,
                                   const RecordObject&) override;
    void PostCallRecordGetSwapchainImagesKHR(VkDevice device, VkSwapchainKHR swapchain, uint32_t* pSwapchainImageCount,
                                             VkImage* pSwapchainImages, const RecordObject& record_obj) override;

    void PreCallRecordCmdClearColorImage(VkCommandBuffer commandBuffer, VkImage image, VkImageLayout imageLayout,
                                         const VkClearColorValue* pColor, uint32_t rangeCount,
                                         const VkImageSubresourceRange* pRanges, const RecordObject&) override;
    void PreCallRecordCmdClearDepthStencilImage(VkCommandBuffer commandBuffer, VkImage image, VkImageLayout imageLayout,
                                                const VkClearDepthStencilValue* pDepthStencil, uint32_t rangeCount,
                                                const VkImageSubresourceRange* pRanges, const RecordObject&) override;
    void PreCallRecordCmdClearAttachments(VkCommandBuffer commandBuffer, uint32_t attachmentCount,
                                          const VkClearAttachment* pAttachments, uint32_t rectCount, const VkClearRect* pRects,
                                          const RecordObject&) override;
    void PostCallRecordTransitionImageLayoutEXT(VkDevice device, uint32_t transitionCount,
                                                const VkHostImageLayoutTransitionInfoEXT* pTransitions,
                                                const RecordObject& record_obj) override;
    void PreCallRecordCmdCopyImage(VkCommandBuffer commandBuffer, VkImage srcImage, VkImageLayout srcImageLayout, VkImage dstImage,
                                   VkImageLayout dstImageLayout, uint32_t regionCount, const VkImageCopy* pRegions,
                                   const RecordObject&) override;
    void PreCallRecordCmdCopyImage2KHR(VkCommandBuffer commandBuffer, const VkCopyImageInfo2KHR* pCopyImageInfo,
                                       const RecordObject&) override;
    void PreCallRecordCmdCopyImage2(VkCommandBuffer commandBuffer, const VkCopyImageInfo2* pCopyImageInfo,
                                    const RecordObject&) override;

    void PreCallRecordCmdCopyImageToBuffer(VkCommandBuffer commandBuffer, VkImage srcImage, VkImageLayout srcImageLayout,
                                           VkBuffer dstBuffer, uint32_t regionCount, const VkBufferImageCopy* pRegions,
                                           const RecordObject&) override;
    void PreCallRecordCmdCopyImageToBuffer2KHR(VkCommandBuffer commandBuffer,
                                               const VkCopyImageToBufferInfo2KHR* pCopyImageToBufferInfo,
                                               const RecordObject&) override;
    void PreCallRecordCmdCopyImageToBuffer2(VkCommandBuffer commandBuffer, const VkCopyImageToBufferInfo2* pCopyImageToBufferInfo,
                                            const RecordObject&) override;

    void PreCallRecordCmdCopyBufferToImage(VkCommandBuffer commandBuffer, VkBuffer srcBuffer, VkImage dstImage,
                                           VkImageLayout dstImageLayout, uint32_t regionCount, const VkBufferImageCopy* pRegions,
                                           const RecordObject&) override;
    void PreCallRecordCmdCopyBufferToImage2KHR(VkCommandBuffer commandBuffer,
                                               const VkCopyBufferToImageInfo2KHR* pCopyBufferToImageInfo2KHR,
                                               const RecordObject&) override;
    void PreCallRecordCmdCopyBufferToImage2(VkCommandBuffer commandBuffer, const VkCopyBufferToImageInfo2* pCopyBufferToImageInfo,
                                            const RecordObject&) override;

    template <typename RegionType>
    void RecordCmdBlitImage(VkCommandBuffer commandBuffer, VkImage srcImage, VkImageLayout srcImageLayout, VkImage dstImage,
                            VkImageLayout dstImageLayout, uint32_t regionCount, const RegionType* pRegions, VkFilter filter);
    void PreCallRecordCmdBlitImage(VkCommandBuffer commandBuffer, VkImage srcImage, VkImageLayout srcImageLayout, VkImage dstImage,
                                   VkImageLayout dstImageLayout, uint32_t regionCount, const VkImageBlit* pRegions, VkFilter filter,
                                   const RecordObject&) override;
    void PreCallRecordCmdBlitImage2KHR(VkCommandBuffer commandBuffer, const VkBlitImageInfo2KHR* pBlitImageInfo,
                                       const RecordObject&) override;
    void PreCallRecordCmdBlitImage2(VkCommandBuffer commandBuffer, const VkBlitImageInfo2KHR* pBlitImageInfo,
                                    const RecordObject&) override;

    void PostCallRecordBindImageMemory(VkDevice device, VkImage image, VkDeviceMemory memory, VkDeviceSize memoryOffset,
                                       const RecordObject& record_obj) override;
    void PostCallRecordBindImageMemory2(VkDevice device, uint32_t bindInfoCount, const VkBindImageMemoryInfo* pBindInfos,
                                        const RecordObject& record_obj) override;
    void PostCallRecordBindImageMemory2KHR(VkDevice device, uint32_t bindInfoCount, const VkBindImageMemoryInfo* pBindInfos,
                                           const RecordObject& record_obj) override;

    void PreCallRecordCmdWaitEvents(VkCommandBuffer commandBuffer, uint32_t eventCount, const VkEvent* pEvents,
                                    VkPipelineStageFlags sourceStageMask, VkPipelineStageFlags dstStageMask,
                                    uint32_t memoryBarrierCount, const VkMemoryBarrier* pMemoryBarriers,
                                    uint32_t bufferMemoryBarrierCount, const VkBufferMemoryBarrier* pBufferMemoryBarriers,
                                    uint32_t imageMemoryBarrierCount, const VkImageMemoryBarrier* pImageMemoryBarriers,
                                    const RecordObject&) override;
    void RecordCmdWaitEvents2(VkCommandBuffer commandBuffer, uint32_t eventCount, const VkEvent* pEvents,
                              const VkDependencyInfo* pDependencyInfos, Func command);
    void PreCallRecordCmdWaitEvents2KHR(VkCommandBuffer commandBuffer, uint32_t eventCount, const VkEvent* pEvents,
                                        const VkDependencyInfoKHR* pDependencyInfos, const RecordObject&) override;
    void PreCallRecordCmdWaitEvents2(VkCommandBuffer commandBuffer, uint32_t eventCount, const VkEvent* pEvents,
                                     const VkDependencyInfo* pDependencyInfos, const RecordObject&) override;

    void PreCallRecordCmdPipelineBarrier(VkCommandBuffer commandBuffer, VkPipelineStageFlags srcStageMask,
                                         VkPipelineStageFlags dstStageMask, VkDependencyFlags dependencyFlags,
                                         uint32_t memoryBarrierCount, const VkMemoryBarrier* pMemoryBarriers,
                                         uint32_t bufferMemoryBarrierCount, const VkBufferMemoryBarrier* pBufferMemoryBarriers,
                                         uint32_t imageMemoryBarrierCount, const VkImageMemoryBarrier* pImageMemoryBarriers,
                                         const RecordObject&) override;

    void PreCallRecordCmdPipelineBarrier2KHR(VkCommandBuffer commandBuffer, const VkDependencyInfoKHR* pDependencyInfo,
                                             const RecordObject&) override;
    void PreCallRecordCmdPipelineBarrier2(VkCommandBuffer commandBuffer, const VkDependencyInfo* pDependencyInfo,
                                          const RecordObject&) override;

    void UpdateCmdBufImageLayouts(const vvl::CommandBuffer& cb_state);
    void RecordTransitionImageLayout(vvl::CommandBuffer* cb_state, const ImageBarrier& mem_barrier);
    void TransitionImageLayouts(vvl::CommandBuffer* cb_state, uint32_t barrier_count, const VkImageMemoryBarrier2* image_barriers);
    void TransitionImageLayouts(vvl::CommandBuffer* cb_state, uint32_t barrier_count, const VkImageMemoryBarrier* image_barriers,
                                VkPipelineStageFlags src_stage_mask, VkPipelineStageFlags dst_stage_mask);

    bool ValidateProtectedImage(const vvl::CommandBuffer& cb_state, const vvl::Image& image_state, const Location& image_loc,
                                const char* vuid, const char* more_message = "") const override;
    bool ValidateUnprotectedImage(const vvl::CommandBuffer& cb_state, const vvl::Image& image_state, const Location& image_loc,
                                  const char* vuid, const char* more_message = "") const override;
    bool ValidateProtectedBuffer(const vvl::CommandBuffer& cb_state, const vvl::Buffer& buffer_state, const Location& buffer_loc,
                                 const char* vuid, const char* more_message = "") const override;
    bool ValidateUnprotectedBuffer(const vvl::CommandBuffer& cb_state, const vvl::Buffer& buffer_state, const Location& buffer_loc,
                                   const char* vuid, const char* more_message = "") const override;

    bool VerifyImageLayout(const vvl::CommandBuffer& cb_state, const vvl::ImageView& image_view_state,
                           VkImageLayout explicit_layout, const Location& image_loc, const char* mismatch_layout_vuid,
                           bool* error) const override;

  private:
    void PreRecordCommandBuffer(VkCommandBuffer command_buffer);
    VkPipeline GetDrawValidationPipeline(VkRenderPass render_pass);

    template <typename RangeFactory>
    bool VerifyImageLayoutRange(const vvl::CommandBuffer& cb_state, const vvl::Image& image_state, VkImageAspectFlags aspect_mask,
                                VkImageLayout explicit_layout, const RangeFactory& range_factory, const Location& loc,
                                const char* mismatch_layout_vuid, bool* error) const;

    VkBool32 shaderInt64 = false;
    bool validate_instrumented_shaders = false;
    std::string instrumented_shader_cache_path{};
    AccelerationStructureBuildValidationState acceleration_structure_validation_state{};
    CommonDrawResources common_draw_resources{};
    CommonDispatchResources common_dispatch_resources{};
    CommonTraceRaysResources common_trace_rays_resources{};
    DeviceMemoryBlock app_buffer_device_addresses{};
    size_t app_bda_buffer_size{};
    uint32_t gpuav_bda_buffer_version = 0;

    bool buffer_device_address_enabled = false;

    std::optional<DescriptorHeap> desc_heap{};  // optional only to defer construction
};

struct RestorablePipelineState {
    VkPipelineBindPoint pipeline_bind_point = VK_PIPELINE_BIND_POINT_MAX_ENUM;
    VkPipeline pipeline = VK_NULL_HANDLE;
    VkPipelineLayout pipeline_layout = VK_NULL_HANDLE;
    std::vector<std::pair<VkDescriptorSet, uint32_t>> descriptor_sets;
    std::vector<std::vector<uint32_t>> dynamic_offsets;
    uint32_t push_descriptor_set_index = 0;
    std::vector<safe_VkWriteDescriptorSet> push_descriptor_set_writes;
    std::vector<uint8_t> push_constants_data;
    PushConstantRangesId push_constants_ranges;

    RestorablePipelineState(vvl::CommandBuffer* cb_state, VkPipelineBindPoint bind_point) { Create(cb_state, bind_point); }

    void Create(vvl::CommandBuffer* cb_state, VkPipelineBindPoint bind_point);
    void Restore(VkCommandBuffer command_buffer) const;
};

}  // namespace gpuav
