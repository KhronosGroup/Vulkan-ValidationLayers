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

typedef vvl::unordered_map<const IMAGE_STATE*, std::optional<GlobalImageLayoutRangeMap>> GlobalImageLayoutMap;

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
    using Struct = vvl::Struct;
    using Field = vvl::Field;
    using ImageBarrier = sync_utils::ImageBarrier;

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
    void PreCallRecordDestroyDevice(VkDevice device, const VkAllocationCallbacks* pAllocator,
                                    const RecordObject& record_obj) override;
    void PostCallRecordBindAccelerationStructureMemoryNV(VkDevice device, uint32_t bindInfoCount,
                                                         const VkBindAccelerationStructureMemoryInfoNV* pBindInfos,
                                                         const RecordObject& record_obj) override;
    std::shared_ptr<BUFFER_STATE> CreateBufferState(VkBuffer buf, const VkBufferCreateInfo* pCreateInfo) override;
    void PreCallRecordCreateBuffer(VkDevice device, const VkBufferCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator,
                                   VkBuffer* pBuffer, const RecordObject& record_obj, void* cb_state_data) override;

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
                                                      VkBuffer scratch, VkDeviceSize scratchOffset,
                                                      const RecordObject& record_obj) override;
    void PreCallRecordDestroyRenderPass(VkDevice device, VkRenderPass renderPass, const VkAllocationCallbacks* pAllocator,
                                        const RecordObject& record_obj) override;
    bool CheckForCachedInstrumentedShader(const uint32_t shader_hash, create_shader_module_api_state* csm_state);
    bool CheckForCachedInstrumentedShader(const uint32_t index, const uint32_t shader_hash, create_shader_object_api_state* cso_state);
    bool InstrumentShader(const vvl::span<const uint32_t>& input, std::vector<uint32_t>& new_pgm, uint32_t unique_shader_id,
                          const Location& loc) override;
    void PreCallRecordCreateShaderModule(VkDevice device, const VkShaderModuleCreateInfo* pCreateInfo,
                                         const VkAllocationCallbacks* pAllocator, VkShaderModule* pShaderModule,
                                         const RecordObject& record_obj, void* csm_state_data) override;
    void PreCallRecordCreateShadersEXT(VkDevice device, uint32_t createInfoCount, const VkShaderCreateInfoEXT* pCreateInfos,
                                       const VkAllocationCallbacks* pAllocator, VkShaderEXT* pShaders,
                                       const RecordObject& record_obj, void* csm_state_data) override;
    void AnalyzeAndGenerateMessages(gpuav_state::CommandBuffer &command_buffer, VkQueue queue, gpuav_state::CommandInfo& cmd_info,
                                    uint32_t operation_index, uint32_t* const debug_output_buffer,
                                    const std::vector<gpuav_state::DescSetState>& descriptor_sets, const Location &loc);
    void UpdateInstrumentationBuffer(gpuav_state::CommandBuffer* cb_node);
    void UpdateBDABuffer(gpuav_state::DeviceMemoryBlock buffer_device_addresses);

    void UpdateBoundDescriptors(VkCommandBuffer commandBuffer, VkPipelineBindPoint pipelineBindPoint);

    void RecordCmdBeginRenderPassLayouts(VkCommandBuffer commandBuffer, const VkRenderPassBeginInfo* pRenderPassBegin,
                                         const VkSubpassContents contents);
    void RecordCmdEndRenderPassLayouts(VkCommandBuffer commandBuffer);
    void TransitionAttachmentRefLayout(CMD_BUFFER_STATE* cb_state, const safe_VkAttachmentReference2& ref);

    void TransitionSubpassLayouts(CMD_BUFFER_STATE* cb_state, const RENDER_PASS_STATE& render_pass_state, const int);
    void TransitionFinalSubpassLayouts(CMD_BUFFER_STATE* cb_state);

    void TransitionBeginRenderPassLayouts(CMD_BUFFER_STATE* cb_state, const RENDER_PASS_STATE& render_pass_state);

    bool UpdateCommandBufferImageLayoutMap(const CMD_BUFFER_STATE* cb_state, const Location& image_loc,
                                           const ImageBarrier& img_barrier, const CommandBufferImageLayoutMap& current_map,
                                           CommandBufferImageLayoutMap& layout_updates) const;

    void PreCallRecordCmdBeginRenderPass(VkCommandBuffer commandBuffer, const VkRenderPassBeginInfo* pRenderPassBegin,
                                         VkSubpassContents contents, const RecordObject& ) override;
    void PreCallRecordCmdBeginRenderPass2KHR(VkCommandBuffer commandBuffer, const VkRenderPassBeginInfo* pRenderPassBegin,
                                             const VkSubpassBeginInfo* pSubpassBeginInfo, const RecordObject& ) override;
    void PreCallRecordCmdBeginRenderPass2(VkCommandBuffer commandBuffer, const VkRenderPassBeginInfo* pRenderPassBegin,
                                             const VkSubpassBeginInfo* pSubpassBeginInfo, const RecordObject& ) override;

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

    void PostCallRecordCmdBindDescriptorSets(VkCommandBuffer commandBuffer, VkPipelineBindPoint pipelineBindPoint,
                                             VkPipelineLayout layout, uint32_t firstSet, uint32_t descriptorSetCount,
                                             const VkDescriptorSet* pDescriptorSets, uint32_t dynamicOffsetCount,
                                             const uint32_t* pDynamicOffsets, const RecordObject& record_obj) override;
    void PreCallRecordCmdPushDescriptorSetKHR(VkCommandBuffer commandBuffer, VkPipelineBindPoint pipelineBindPoint,
                                              VkPipelineLayout layout, uint32_t set, uint32_t descriptorWriteCount,
                                              const VkWriteDescriptorSet* pDescriptorWrites, const RecordObject& ) override;
    void PreCallRecordQueueSubmit(VkQueue queue, uint32_t submitCount, const VkSubmitInfo* pSubmits, VkFence fence, const RecordObject& ) override;
    void PostCallRecordQueueSubmit(VkQueue queue, uint32_t submitCount, const VkSubmitInfo *pSubmits, VkFence fence,
                                   const RecordObject &record_obj) override;
    void PreCallRecordQueueSubmit2KHR(VkQueue queue, uint32_t submitCount, const VkSubmitInfo2KHR* pSubmits,
                                      VkFence fence, const RecordObject& ) override;
    void PreCallRecordQueueSubmit2(VkQueue queue, uint32_t submitCount, const VkSubmitInfo2* pSubmits, VkFence fence, const RecordObject& ) override;
    void PostCallRecordQueueSubmit2KHR(VkQueue queue, uint32_t submitCount, const VkSubmitInfo2KHR *pSubmits, VkFence fence,
                                       const RecordObject &record_obj) override;
    void PostCallRecordQueueSubmit2(VkQueue queue, uint32_t submitCount, const VkSubmitInfo2KHR *pSubmits, VkFence fence,
                                    const RecordObject &record_obj) override;

    void PreCallRecordCmdBindDescriptorBuffersEXT(VkCommandBuffer commandBuffer, uint32_t bufferCount,
                                                  const VkDescriptorBufferBindingInfoEXT* pBindingInfos,
                                                  const RecordObject& record_obj) override;
    void PreCallRecordCmdBindDescriptorBufferEmbeddedSamplersEXT(VkCommandBuffer commandBuffer,
                                                                 VkPipelineBindPoint pipelineBindPoint, VkPipelineLayout layout,
                                                                 uint32_t set, const RecordObject& record_obj) override;
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

    void PostCallRecordCreateImage(VkDevice device, const VkImageCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator,
                                   VkImage* pImage, const RecordObject& record_obj) override;
    void PreCallRecordDestroyImage(VkDevice device, VkImage image, const VkAllocationCallbacks* pAllocator, const RecordObject& ) override;
    void PostCallRecordGetSwapchainImagesKHR(VkDevice device, VkSwapchainKHR swapchain, uint32_t *pSwapchainImageCount,
                                             VkImage *pSwapchainImages, const RecordObject &record_obj) override;

    void PreCallRecordCmdClearColorImage(VkCommandBuffer commandBuffer, VkImage image, VkImageLayout imageLayout,
                                         const VkClearColorValue* pColor, uint32_t rangeCount,
                                         const VkImageSubresourceRange* pRanges, const RecordObject& ) override;
    void PreCallRecordCmdClearDepthStencilImage(VkCommandBuffer commandBuffer, VkImage image, VkImageLayout imageLayout,
                                                const VkClearDepthStencilValue *pDepthStencil, uint32_t rangeCount,
                                                const VkImageSubresourceRange *pRanges, const RecordObject& ) override;
    void PreCallRecordCmdClearAttachments(VkCommandBuffer commandBuffer, uint32_t attachmentCount,
                                          const VkClearAttachment *pAttachments, uint32_t rectCount,
                                          const VkClearRect *pRects, const RecordObject& ) override;
    void PostCallRecordTransitionImageLayoutEXT(VkDevice device, uint32_t transitionCount,
                                                const VkHostImageLayoutTransitionInfoEXT *pTransitions,
                                                const RecordObject &record_obj) override;
    void PreCallRecordCmdCopyImage(VkCommandBuffer commandBuffer, VkImage srcImage, VkImageLayout srcImageLayout,
                                   VkImage dstImage, VkImageLayout dstImageLayout, uint32_t regionCount,
                                   const VkImageCopy *pRegions, const RecordObject& ) override;
    void PreCallRecordCmdCopyImage2KHR(VkCommandBuffer commandBuffer, const VkCopyImageInfo2KHR *pCopyImageInfo, const RecordObject& ) override;
    void PreCallRecordCmdCopyImage2(VkCommandBuffer commandBuffer, const VkCopyImageInfo2 *pCopyImageInfo, const RecordObject& ) override;

    void PreCallRecordCmdCopyImageToBuffer(VkCommandBuffer commandBuffer, VkImage srcImage, VkImageLayout srcImageLayout,
                                           VkBuffer dstBuffer, uint32_t regionCount, const VkBufferImageCopy *pRegions, const RecordObject& ) override;
    void PreCallRecordCmdCopyImageToBuffer2KHR(VkCommandBuffer commandBuffer,
                                               const VkCopyImageToBufferInfo2KHR *pCopyImageToBufferInfo, const RecordObject& ) override;
    void PreCallRecordCmdCopyImageToBuffer2(VkCommandBuffer commandBuffer,
                                            const VkCopyImageToBufferInfo2 *pCopyImageToBufferInfo, const RecordObject& ) override;

    void PreCallRecordCmdCopyBufferToImage(VkCommandBuffer commandBuffer, VkBuffer srcBuffer, VkImage dstImage,
                                           VkImageLayout dstImageLayout, uint32_t regionCount,
                                           const VkBufferImageCopy *pRegions, const RecordObject& ) override;
    void PreCallRecordCmdCopyBufferToImage2KHR(VkCommandBuffer commandBuffer,
                                               const VkCopyBufferToImageInfo2KHR *pCopyBufferToImageInfo2KHR, const RecordObject& ) override;
    void PreCallRecordCmdCopyBufferToImage2(VkCommandBuffer commandBuffer,
                                            const VkCopyBufferToImageInfo2 *pCopyBufferToImageInfo, const RecordObject& ) override;

    template <typename RegionType>
    void RecordCmdBlitImage(VkCommandBuffer commandBuffer, VkImage srcImage, VkImageLayout srcImageLayout, VkImage dstImage,
                            VkImageLayout dstImageLayout, uint32_t regionCount, const RegionType *pRegions,
                            VkFilter filter);
    void PreCallRecordCmdBlitImage(VkCommandBuffer commandBuffer, VkImage srcImage, VkImageLayout srcImageLayout,
                                   VkImage dstImage, VkImageLayout dstImageLayout, uint32_t regionCount,
                                   const VkImageBlit *pRegions, VkFilter filter, const RecordObject& ) override;
    void PreCallRecordCmdBlitImage2KHR(VkCommandBuffer commandBuffer, const VkBlitImageInfo2KHR *pBlitImageInfo, const RecordObject& ) override;
    void PreCallRecordCmdBlitImage2(VkCommandBuffer commandBuffer, const VkBlitImageInfo2KHR *pBlitImageInfo, const RecordObject& ) override;

    void PostCallRecordBindImageMemory(VkDevice device, VkImage image, VkDeviceMemory memory, VkDeviceSize memoryOffset,
                                       const RecordObject &record_obj) override;
    void PostCallRecordBindImageMemory2(VkDevice device, uint32_t bindInfoCount, const VkBindImageMemoryInfo *pBindInfos,
                                        const RecordObject &record_obj)  override;
    void PostCallRecordBindImageMemory2KHR(VkDevice device, uint32_t bindInfoCount, const VkBindImageMemoryInfo *pBindInfos,
                                           const RecordObject &record_obj) override;

    void PreCallRecordCmdWaitEvents(VkCommandBuffer commandBuffer, uint32_t eventCount, const VkEvent *pEvents,
                                    VkPipelineStageFlags sourceStageMask, VkPipelineStageFlags dstStageMask,
                                    uint32_t memoryBarrierCount, const VkMemoryBarrier *pMemoryBarriers,
                                    uint32_t bufferMemoryBarrierCount, const VkBufferMemoryBarrier *pBufferMemoryBarriers,
                                    uint32_t imageMemoryBarrierCount, const VkImageMemoryBarrier *pImageMemoryBarriers, const RecordObject& ) override;
    void RecordCmdWaitEvents2(VkCommandBuffer commandBuffer, uint32_t eventCount, const VkEvent *pEvents,
                              const VkDependencyInfo *pDependencyInfos, Func command);
    void PreCallRecordCmdWaitEvents2KHR(VkCommandBuffer commandBuffer, uint32_t eventCount, const VkEvent *pEvents,
                                        const VkDependencyInfoKHR *pDependencyInfos, const RecordObject&) override;
    void PreCallRecordCmdWaitEvents2(VkCommandBuffer commandBuffer, uint32_t eventCount, const VkEvent *pEvents,
                                     const VkDependencyInfo *pDependencyInfos, const RecordObject&) override;

    void PreCallRecordCmdPipelineBarrier(VkCommandBuffer commandBuffer, VkPipelineStageFlags srcStageMask,
                                         VkPipelineStageFlags dstStageMask, VkDependencyFlags dependencyFlags,
                                         uint32_t memoryBarrierCount, const VkMemoryBarrier *pMemoryBarriers,
                                         uint32_t bufferMemoryBarrierCount,
                                         const VkBufferMemoryBarrier *pBufferMemoryBarriers,
                                         uint32_t imageMemoryBarrierCount,
                                         const VkImageMemoryBarrier *pImageMemoryBarriers, const RecordObject& ) override;

    void PreCallRecordCmdPipelineBarrier2KHR(VkCommandBuffer commandBuffer, const VkDependencyInfoKHR *pDependencyInfo, const RecordObject& ) override;
    void PreCallRecordCmdPipelineBarrier2(VkCommandBuffer commandBuffer, const VkDependencyInfo *pDependencyInfo, const RecordObject& ) override;

    void UpdateCmdBufImageLayouts(const CMD_BUFFER_STATE& cb_state);
    void RecordTransitionImageLayout(CMD_BUFFER_STATE *cb_state, const ImageBarrier &mem_barrier);
    void TransitionImageLayouts(CMD_BUFFER_STATE* cb_state, uint32_t barrier_count, const VkImageMemoryBarrier2* image_barriers);
    void TransitionImageLayouts(CMD_BUFFER_STATE* cb_state, uint32_t barrier_count, const VkImageMemoryBarrier* image_barriers,
                                VkPipelineStageFlags src_stage_mask, VkPipelineStageFlags dst_stage_mask);

    std::shared_ptr<CMD_BUFFER_STATE> CreateCmdBufferState(VkCommandBuffer cb, const VkCommandBufferAllocateInfo* create_info,
                                                           const COMMAND_POOL_STATE* pool) final;
    std::shared_ptr<cvdescriptorset::DescriptorSet> CreateDescriptorSet(
        VkDescriptorSet, DESCRIPTOR_POOL_STATE*, const std::shared_ptr<cvdescriptorset::DescriptorSetLayout const>& layout,
        uint32_t variable_count) final;

    void DestroyBuffer(gpuav_state::CommandInfo& cmd_info);
    void DestroyBuffer(gpuav_state::AccelerationStructureBuildValidationBufferInfo& buffer_info);

    bool ValidateProtectedImage(const CMD_BUFFER_STATE& cb_state, const IMAGE_STATE& image_state, const Location& image_loc,
                                const char* vuid, const char* more_message = "") const override;
    bool ValidateUnprotectedImage(const CMD_BUFFER_STATE& cb_state, const IMAGE_STATE& image_state, const Location& image_loc,
                                  const char* vuid, const char* more_message = "") const override;
    bool ValidateProtectedBuffer(const CMD_BUFFER_STATE& cb_state, const BUFFER_STATE& buffer_state, const Location& buffer_loc,
                                 const char* vuid, const char* more_message = "") const override;
    bool ValidateUnprotectedBuffer(const CMD_BUFFER_STATE& cb_state, const BUFFER_STATE& buffer_state, const Location& buffer_loc,
                                   const char* vuid, const char* more_message = "") const override;

    bool VerifyImageLayout(const CMD_BUFFER_STATE& cb_state, const IMAGE_VIEW_STATE& image_view_state,
                           VkImageLayout explicit_layout, const Location& image_loc, const char* mismatch_layout_vuid,
                           bool* error) const override;
  private:
    void PreRecordCommandBuffer(VkCommandBuffer command_buffer);
    VkPipeline GetValidationPipeline(VkRenderPass render_pass);

    template <typename RangeFactory>
    bool VerifyImageLayoutRange(const CMD_BUFFER_STATE &cb_state, const IMAGE_STATE &image_state,
                                VkImageAspectFlags aspect_mask, VkImageLayout explicit_layout,
                                const RangeFactory &range_factory, const Location &loc, const char *mismatch_layout_vuid,
                                bool *error) const;

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
