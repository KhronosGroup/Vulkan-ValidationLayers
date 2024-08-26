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

#include "gpu/descriptor_validation/gpuav_descriptor_set.h"
#include "gpu/resources/gpu_resources.h"
#include "gpu/instrumentation/gpu_shader_instrumentor.h"

#include <memory>

namespace chassis {
struct ShaderObject;
}  // namespace chassis

namespace gpuav {
class Buffer;
class BufferView;
class CommandBuffer;
class ImageView;
class Queue;
class Sampler;
class DescriptorSet;
struct DescSetState;
}  // namespace gpuav

VALSTATETRACK_DERIVED_STATE_OBJECT(VkBuffer, gpuav::Buffer, vvl::Buffer)
VALSTATETRACK_DERIVED_STATE_OBJECT(VkBufferView, gpuav::BufferView, vvl::BufferView)
VALSTATETRACK_DERIVED_STATE_OBJECT(VkCommandBuffer, gpuav::CommandBuffer, vvl::CommandBuffer)
VALSTATETRACK_DERIVED_STATE_OBJECT(VkDescriptorSet, gpuav::DescriptorSet, vvl::DescriptorSet)
VALSTATETRACK_DERIVED_STATE_OBJECT(VkImageView, gpuav::ImageView, vvl::ImageView)
VALSTATETRACK_DERIVED_STATE_OBJECT(VkSampler, gpuav::Sampler, vvl::Sampler)
VALSTATETRACK_DERIVED_STATE_OBJECT(VkQueue, gpuav::Queue, vvl::Queue)

namespace gpuav {

class Validator : public gpu::GpuShaderInstrumentor {
    using BaseClass = GpuShaderInstrumentor;
    using Func = vvl::Func;
    using Struct = vvl::Struct;
    using Field = vvl::Field;

  public:
    Validator() { container_type = LayerObjectTypeGpuAssisted; }

    // gpuav_setup.cpp
    // -------------
  public:
    std::shared_ptr<vvl::Buffer> CreateBufferState(VkBuffer handle, const VkBufferCreateInfo* create_info) final;
    std::shared_ptr<vvl::BufferView> CreateBufferViewState(const std::shared_ptr<vvl::Buffer>& buffer, VkBufferView handle,
                                                           const VkBufferViewCreateInfo* create_info,
                                                           VkFormatFeatureFlags2 format_features) final;
    std::shared_ptr<vvl::ImageView> CreateImageViewState(const std::shared_ptr<vvl::Image>& image_state, VkImageView handle,
                                                         const VkImageViewCreateInfo* create_info,
                                                         VkFormatFeatureFlags2 format_features,
                                                         const VkFilterCubicImageViewImageFormatPropertiesEXT& cubic_props) final;
    std::shared_ptr<vvl::Sampler> CreateSamplerState(VkSampler handle, const VkSamplerCreateInfo* create_info) final;
    std::shared_ptr<vvl::CommandBuffer> CreateCmdBufferState(VkCommandBuffer handle,
                                                             const VkCommandBufferAllocateInfo* allocate_info,
                                                             const vvl::CommandPool* pool) final;
    std::shared_ptr<vvl::DescriptorSet> CreateDescriptorSet(VkDescriptorSet handle, vvl::DescriptorPool* pool,
                                                            const std::shared_ptr<vvl::DescriptorSetLayout const>& layout,
                                                            u32 variable_count) final;

    void PreCallRecordCreateDevice(VkPhysicalDevice physicalDevice, const VkDeviceCreateInfo* pCreateInfo,
                                   const VkAllocationCallbacks* pAllocator, VkDevice* pDevice, const RecordObject& record_obj,
                                   vku::safe_VkDeviceCreateInfo* modified_create_info) final;
    void PostCreateDevice(const VkDeviceCreateInfo* pCreateInfo, const Location& loc) final;

  private:
    void InitSettings(const Location& loc);

    // gpuav_record.cpp
    // --------------
  public:
    void PreCallRecordDestroyDevice(VkDevice device, const VkAllocationCallbacks* pAllocator, const RecordObject& record_obj) final;
    void PreCallRecordCreateBuffer(VkDevice device, const VkBufferCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator,
                                   VkBuffer* pBuffer, const RecordObject& record_obj, chassis::CreateBuffer& chassis_state) final;
    void PreCallRecordDestroyRenderPass(VkDevice device, VkRenderPass renderPass, const VkAllocationCallbacks* pAllocator,
                                        const RecordObject& record_obj) final;

    void RecordCmdBeginRenderPassLayouts(VkCommandBuffer commandBuffer, const VkRenderPassBeginInfo* pRenderPassBegin,
                                         const VkSubpassContents contents);
    void RecordCmdEndRenderPassLayouts(VkCommandBuffer commandBuffer);
    void PreCallRecordCmdBeginRenderPass(VkCommandBuffer commandBuffer, const VkRenderPassBeginInfo* pRenderPassBegin,
                                         VkSubpassContents contents, const RecordObject&) final;
    void PreCallRecordCmdBeginRenderPass2KHR(VkCommandBuffer commandBuffer, const VkRenderPassBeginInfo* pRenderPassBegin,
                                             const VkSubpassBeginInfo* pSubpassBeginInfo, const RecordObject&) final;
    void PreCallRecordCmdBeginRenderPass2(VkCommandBuffer commandBuffer, const VkRenderPassBeginInfo* pRenderPassBegin,
                                          const VkSubpassBeginInfo* pSubpassBeginInfo, const RecordObject&) final;

    void RecordCmdNextSubpassLayouts(VkCommandBuffer commandBuffer, VkSubpassContents contents);
    void PostCallRecordCmdNextSubpass(VkCommandBuffer commandBuffer, VkSubpassContents contents,
                                      const RecordObject& record_obj) final;
    void PostCallRecordCmdNextSubpass2KHR(VkCommandBuffer commandBuffer, const VkSubpassBeginInfo* pSubpassBeginInfo,
                                          const VkSubpassEndInfo* pSubpassEndInfo, const RecordObject& record_obj) final;
    void PostCallRecordCmdNextSubpass2(VkCommandBuffer commandBuffer, const VkSubpassBeginInfo* pSubpassBeginInfo,
                                       const VkSubpassEndInfo* pSubpassEndInfo, const RecordObject& record_obj) final;

    void PostCallRecordCmdEndRenderPass(VkCommandBuffer commandBuffer, const RecordObject& record_obj) final;
    void PostCallRecordCmdEndRenderPass2KHR(VkCommandBuffer commandBuffer, const VkSubpassEndInfo* pSubpassEndInfo,
                                            const RecordObject& record_obj) final;
    void PostCallRecordCmdEndRenderPass2(VkCommandBuffer commandBuffer, const VkSubpassEndInfo* pSubpassEndInfo,
                                         const RecordObject& record_obj) final;

    void PostCallRecordCmdBindPipeline(VkCommandBuffer commandBuffer, VkPipelineBindPoint pipelineBindPoint, VkPipeline pipeline,
                                       const RecordObject& record_obj) final;
    void PostCallRecordCmdBindDescriptorSets2KHR(VkCommandBuffer commandBuffer,
                                                 const VkBindDescriptorSetsInfoKHR* pBindDescriptorSetsInfo,
                                                 const RecordObject& record_obj) final;
    void PostCallRecordCmdBindDescriptorSets(VkCommandBuffer commandBuffer, VkPipelineBindPoint pipelineBindPoint,
                                             VkPipelineLayout layout, u32 firstSet, u32 descriptorSetCount,
                                             const VkDescriptorSet* pDescriptorSets, u32 dynamicOffsetCount,
                                             const u32* pDynamicOffsets, const RecordObject& record_obj) final;
    void PreCallRecordCmdPushDescriptorSetKHR(VkCommandBuffer commandBuffer, VkPipelineBindPoint pipelineBindPoint,
                                              VkPipelineLayout layout, u32 set, u32 descriptorWriteCount,
                                              const VkWriteDescriptorSet* pDescriptorWrites, const RecordObject&) final;
    void PreCallRecordCmdPushDescriptorSet2KHR(VkCommandBuffer commandBuffer,
                                               const VkPushDescriptorSetInfoKHR* pPushDescriptorSetInfo,
                                               const RecordObject& record_obj) final;
    void PreCallRecordCmdBindDescriptorBuffersEXT(VkCommandBuffer commandBuffer, u32 bufferCount,
                                                  const VkDescriptorBufferBindingInfoEXT* pBindingInfos,
                                                  const RecordObject& record_obj) final;
    void PreCallRecordCmdBindDescriptorBufferEmbeddedSamplersEXT(VkCommandBuffer commandBuffer,
                                                                 VkPipelineBindPoint pipelineBindPoint, VkPipelineLayout layout,
                                                                 u32 set, const RecordObject& record_obj) final;
    void PreCallRecordCmdBindDescriptorBufferEmbeddedSamplers2EXT(
        VkCommandBuffer commandBuffer,
        const VkBindDescriptorBufferEmbeddedSamplersInfoEXT* pBindDescriptorBufferEmbeddedSamplersInfo,
        const RecordObject& record_obj) final;
    void PreCallRecordCmdDraw(VkCommandBuffer commandBuffer, u32 vertexCount, u32 instanceCount, u32 firstVertex, u32 firstInstance,
                              const RecordObject& record_obj) final;
    void PreCallRecordCmdDrawMultiEXT(VkCommandBuffer commandBuffer, u32 drawCount, const VkMultiDrawInfoEXT* pVertexInfo,
                                      u32 instanceCount, u32 firstInstance, u32 stride, const RecordObject& record_obj) final;
    void PreCallRecordCmdDrawIndexed(VkCommandBuffer commandBuffer, u32 indexCount, u32 instanceCount, u32 firstIndex,
                                     i32 vertexOffset, u32 firstInstance, const RecordObject& record_obj) final;
    void PreCallRecordCmdDrawMultiIndexedEXT(VkCommandBuffer commandBuffer, u32 drawCount,
                                             const VkMultiDrawIndexedInfoEXT* pIndexInfo, u32 instanceCount, u32 firstInstance,
                                             u32 stride, const i32* pVertexOffset, const RecordObject& record_obj) final;
    void PreCallRecordCmdDrawIndirect(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset, u32 count, u32 stride,
                                      const RecordObject& record_obj) final;
    void PreCallRecordCmdDrawIndexedIndirect(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset, u32 count,
                                             u32 stride, const RecordObject& record_obj) final;
    void PreCallRecordCmdDrawIndirectCountKHR(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                              VkBuffer countBuffer, VkDeviceSize countBufferOffset, u32 maxDrawCount, u32 stride,
                                              const RecordObject& record_obj) final;
    void PreCallRecordCmdDrawIndirectCount(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                           VkBuffer countBuffer, VkDeviceSize countBufferOffset, u32 maxDrawCount, u32 stride,
                                           const RecordObject& record_obj) final;
    void PreCallRecordCmdDrawIndirectByteCountEXT(VkCommandBuffer commandBuffer, u32 instanceCount, u32 firstInstance,
                                                  VkBuffer counterBuffer, VkDeviceSize counterBufferOffset, u32 counterOffset,
                                                  u32 vertexStride, const RecordObject& record_obj) final;
    void PreCallRecordCmdDrawIndexedIndirectCountKHR(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                     VkBuffer countBuffer, VkDeviceSize countBufferOffset, u32 maxDrawCount,
                                                     u32 stride, const RecordObject& record_obj) final;
    void PreCallRecordCmdDrawIndexedIndirectCount(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                  VkBuffer countBuffer, VkDeviceSize countBufferOffset, u32 maxDrawCount,
                                                  u32 stride, const RecordObject& record_obj) final;
    void PreCallRecordCmdDrawMeshTasksNV(VkCommandBuffer commandBuffer, u32 taskCount, u32 firstTask,
                                         const RecordObject& record_obj) final;
    void PreCallRecordCmdDrawMeshTasksIndirectNV(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset, u32 drawCount,
                                                 u32 stride, const RecordObject& record_obj) final;
    void PreCallRecordCmdDrawMeshTasksIndirectCountNV(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                      VkBuffer countBuffer, VkDeviceSize countBufferOffset, u32 maxDrawCount,
                                                      u32 stride, const RecordObject& record_obj) final;
    void PreCallRecordCmdDrawMeshTasksEXT(VkCommandBuffer commandBuffer, u32 groupCountX, u32 groupCountY, u32 groupCountZ,
                                          const RecordObject& record_obj) final;
    void PreCallRecordCmdDrawMeshTasksIndirectEXT(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                  u32 drawCount, u32 stride, const RecordObject& record_obj) final;
    void PreCallRecordCmdDrawMeshTasksIndirectCountEXT(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                       VkBuffer countBuffer, VkDeviceSize countBufferOffset, u32 maxDrawCount,
                                                       u32 stride, const RecordObject& record_obj) final;
    void PreCallRecordCmdDispatch(VkCommandBuffer commandBuffer, u32 x, u32 y, u32 z, const RecordObject& record_obj) final;
    void PreCallRecordCmdDispatchIndirect(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                          const RecordObject& record_obj) final;
    void PreCallRecordCmdDispatchBase(VkCommandBuffer commandBuffer, u32 baseGroupX, u32 baseGroupY, u32 baseGroupZ,
                                      u32 groupCountX, u32 groupCountY, u32 groupCountZ, const RecordObject& record_obj) final;
    void PreCallRecordCmdDispatchBaseKHR(VkCommandBuffer commandBuffer, u32 baseGroupX, u32 baseGroupY, u32 baseGroupZ,
                                         u32 groupCountX, u32 groupCountY, u32 groupCountZ, const RecordObject& record_obj) final;
    void PreCallRecordCmdTraceRaysNV(VkCommandBuffer commandBuffer, VkBuffer raygenShaderBindingTableBuffer,
                                     VkDeviceSize raygenShaderBindingOffset, VkBuffer missShaderBindingTableBuffer,
                                     VkDeviceSize missShaderBindingOffset, VkDeviceSize missShaderBindingStride,
                                     VkBuffer hitShaderBindingTableBuffer, VkDeviceSize hitShaderBindingOffset,
                                     VkDeviceSize hitShaderBindingStride, VkBuffer callableShaderBindingTableBuffer,
                                     VkDeviceSize callableShaderBindingOffset, VkDeviceSize callableShaderBindingStride, u32 width,
                                     u32 height, u32 depth, const RecordObject& record_obj) final;
    void PreCallRecordCmdTraceRaysKHR(VkCommandBuffer commandBuffer,
                                      const VkStridedDeviceAddressRegionKHR* pRaygenShaderBindingTable,
                                      const VkStridedDeviceAddressRegionKHR* pMissShaderBindingTable,
                                      const VkStridedDeviceAddressRegionKHR* pHitShaderBindingTable,
                                      const VkStridedDeviceAddressRegionKHR* pCallableShaderBindingTable, u32 width, u32 height,
                                      u32 depth, const RecordObject& record_obj) final;
    void PreCallRecordCmdTraceRaysIndirectKHR(VkCommandBuffer commandBuffer,
                                              const VkStridedDeviceAddressRegionKHR* pRaygenShaderBindingTable,
                                              const VkStridedDeviceAddressRegionKHR* pMissShaderBindingTable,
                                              const VkStridedDeviceAddressRegionKHR* pHitShaderBindingTable,
                                              const VkStridedDeviceAddressRegionKHR* pCallableShaderBindingTable,
                                              VkDeviceAddress indirectDeviceAddress, const RecordObject& record_obj) final;
    void PreCallRecordCmdTraceRaysIndirect2KHR(VkCommandBuffer commandBuffer, VkDeviceAddress indirectDeviceAddress,
                                               const RecordObject& record_obj) final;

    void PostCallRecordGetPhysicalDeviceProperties2(VkPhysicalDevice physicalDevice,
                                                    VkPhysicalDeviceProperties2* pPhysicalDeviceProperties2,
                                                    const RecordObject& record_obj) final;

    // gpuav_image_layout.cpp
    // --------------------

    void PostCallRecordCreateImage(VkDevice device, const VkImageCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator,
                                   VkImage* pImage, const RecordObject& record_obj) final;
    void PreCallRecordDestroyImage(VkDevice device, VkImage image, const VkAllocationCallbacks* pAllocator,
                                   const RecordObject&) final;

    void PreCallRecordCmdClearColorImage(VkCommandBuffer commandBuffer, VkImage image, VkImageLayout imageLayout,
                                         const VkClearColorValue* pColor, u32 rangeCount, const VkImageSubresourceRange* pRanges,
                                         const RecordObject&) final;
    void PreCallRecordCmdClearDepthStencilImage(VkCommandBuffer commandBuffer, VkImage image, VkImageLayout imageLayout,
                                                const VkClearDepthStencilValue* pDepthStencil, u32 rangeCount,
                                                const VkImageSubresourceRange* pRanges, const RecordObject&) final;
    void PreCallRecordCmdClearAttachments(VkCommandBuffer commandBuffer, u32 attachmentCount, const VkClearAttachment* pAttachments,
                                          u32 rectCount, const VkClearRect* pRects, const RecordObject&) final;
    void PostCallRecordTransitionImageLayoutEXT(VkDevice device, u32 transitionCount,
                                                const VkHostImageLayoutTransitionInfoEXT* pTransitions,
                                                const RecordObject& record_obj) final;
    void PreCallRecordCmdCopyImage(VkCommandBuffer commandBuffer, VkImage srcImage, VkImageLayout srcImageLayout, VkImage dstImage,
                                   VkImageLayout dstImageLayout, u32 regionCount, const VkImageCopy* pRegions,
                                   const RecordObject&) final;
    void PreCallRecordCmdCopyImage2KHR(VkCommandBuffer commandBuffer, const VkCopyImageInfo2KHR* pCopyImageInfo,
                                       const RecordObject&) final;
    void PreCallRecordCmdCopyImage2(VkCommandBuffer commandBuffer, const VkCopyImageInfo2* pCopyImageInfo,
                                    const RecordObject&) final;

    void PreCallRecordCmdCopyImageToBuffer(VkCommandBuffer commandBuffer, VkImage srcImage, VkImageLayout srcImageLayout,
                                           VkBuffer dstBuffer, u32 regionCount, const VkBufferImageCopy* pRegions,
                                           const RecordObject&) final;
    void PreCallRecordCmdCopyImageToBuffer2KHR(VkCommandBuffer commandBuffer,
                                               const VkCopyImageToBufferInfo2KHR* pCopyImageToBufferInfo,
                                               const RecordObject&) final;
    void PreCallRecordCmdCopyImageToBuffer2(VkCommandBuffer commandBuffer, const VkCopyImageToBufferInfo2* pCopyImageToBufferInfo,
                                            const RecordObject&) final;

    void PreCallRecordCmdCopyBufferToImage(VkCommandBuffer commandBuffer, VkBuffer srcBuffer, VkImage dstImage,
                                           VkImageLayout dstImageLayout, u32 regionCount, const VkBufferImageCopy* pRegions,
                                           const RecordObject&) final;
    void PreCallRecordCmdCopyBufferToImage2KHR(VkCommandBuffer commandBuffer,
                                               const VkCopyBufferToImageInfo2KHR* pCopyBufferToImageInfo2KHR,
                                               const RecordObject&) final;
    void PreCallRecordCmdCopyBufferToImage2(VkCommandBuffer commandBuffer, const VkCopyBufferToImageInfo2* pCopyBufferToImageInfo,
                                            const RecordObject&) final;

    void PreCallRecordCmdBlitImage(VkCommandBuffer commandBuffer, VkImage srcImage, VkImageLayout srcImageLayout, VkImage dstImage,
                                   VkImageLayout dstImageLayout, u32 regionCount, const VkImageBlit* pRegions, VkFilter filter,
                                   const RecordObject&) final;
    void PreCallRecordCmdBlitImage2KHR(VkCommandBuffer commandBuffer, const VkBlitImageInfo2KHR* pBlitImageInfo,
                                       const RecordObject&) final;
    void PreCallRecordCmdBlitImage2(VkCommandBuffer commandBuffer, const VkBlitImageInfo2KHR* pBlitImageInfo,
                                    const RecordObject&) final;

    void PostCallRecordBindImageMemory(VkDevice device, VkImage image, VkDeviceMemory memory, VkDeviceSize memoryOffset,
                                       const RecordObject& record_obj) final;
    void PostCallRecordBindImageMemory2(VkDevice device, u32 bindInfoCount, const VkBindImageMemoryInfo* pBindInfos,
                                        const RecordObject& record_obj) final;
    void PostCallRecordBindImageMemory2KHR(VkDevice device, u32 bindInfoCount, const VkBindImageMemoryInfo* pBindInfos,
                                           const RecordObject& record_obj) final;

    void PreCallRecordCmdWaitEvents(VkCommandBuffer commandBuffer, u32 eventCount, const VkEvent* pEvents,
                                    VkPipelineStageFlags sourceStageMask, VkPipelineStageFlags dstStageMask, u32 memoryBarrierCount,
                                    const VkMemoryBarrier* pMemoryBarriers, u32 bufferMemoryBarrierCount,
                                    const VkBufferMemoryBarrier* pBufferMemoryBarriers, u32 imageMemoryBarrierCount,
                                    const VkImageMemoryBarrier* pImageMemoryBarriers, const RecordObject&) final;
    void PreCallRecordCmdWaitEvents2KHR(VkCommandBuffer commandBuffer, u32 eventCount, const VkEvent* pEvents,
                                        const VkDependencyInfoKHR* pDependencyInfos, const RecordObject&) final;
    void PreCallRecordCmdWaitEvents2(VkCommandBuffer commandBuffer, u32 eventCount, const VkEvent* pEvents,
                                     const VkDependencyInfo* pDependencyInfos, const RecordObject&) final;

    void PreCallRecordCmdPipelineBarrier(VkCommandBuffer commandBuffer, VkPipelineStageFlags srcStageMask,
                                         VkPipelineStageFlags dstStageMask, VkDependencyFlags dependencyFlags,
                                         u32 memoryBarrierCount, const VkMemoryBarrier* pMemoryBarriers,
                                         u32 bufferMemoryBarrierCount, const VkBufferMemoryBarrier* pBufferMemoryBarriers,
                                         u32 imageMemoryBarrierCount, const VkImageMemoryBarrier* pImageMemoryBarriers,
                                         const RecordObject&) final;

    void PreCallRecordCmdPipelineBarrier2KHR(VkCommandBuffer commandBuffer, const VkDependencyInfoKHR* pDependencyInfo,
                                             const RecordObject&) final;
    void PreCallRecordCmdPipelineBarrier2(VkCommandBuffer commandBuffer, const VkDependencyInfo* pDependencyInfo,
                                          const RecordObject&) final;
    bool ValidateProtectedImage(const vvl::CommandBuffer& cb_state, const vvl::Image& image_state, const Location& image_loc,
                                const char* vuid, const char* more_message = "") const final;
    bool ValidateUnprotectedImage(const vvl::CommandBuffer& cb_state, const vvl::Image& image_state, const Location& image_loc,
                                  const char* vuid, const char* more_message = "") const final;
    bool ValidateProtectedBuffer(const vvl::CommandBuffer& cb_state, const vvl::Buffer& buffer_state, const Location& buffer_loc,
                                 const char* vuid, const char* more_message = "") const final;
    bool ValidateUnprotectedBuffer(const vvl::CommandBuffer& cb_state, const vvl::Buffer& buffer_state, const Location& buffer_loc,
                                   const char* vuid, const char* more_message = "") const final;

    bool VerifyImageLayout(const vvl::CommandBuffer& cb_state, const vvl::ImageView& image_view_state,
                           VkImageLayout explicit_layout, const Location& image_loc, const char* mismatch_layout_vuid,
                           bool* error) const final;

  public:
    std::optional<DescriptorHeap> desc_heap_{};  // optional only to defer construction
    gpu::SharedResourcesManager shared_resources_manager;

  private:
    std::string instrumented_shader_cache_path_{};
};

}  // namespace gpuav
