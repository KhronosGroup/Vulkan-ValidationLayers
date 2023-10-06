// *** THIS FILE IS GENERATED - DO NOT EDIT ***
// See stateless_validation_helper_generator.py for modifications

/***************************************************************************
 *
 * Copyright (c) 2015-2023 The Khronos Group Inc.
 * Copyright (c) 2015-2023 Valve Corporation
 * Copyright (c) 2015-2023 LunarG, Inc.
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
 ****************************************************************************/

// NOLINTBEGIN
#pragma once

static inline bool IsDuplicatePnext(VkStructureType input_value) {
    switch (input_value) {
        case VK_STRUCTURE_TYPE_DEVICE_PRIVATE_DATA_CREATE_INFO:
        case VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT:
        case VK_STRUCTURE_TYPE_DEVICE_DEVICE_MEMORY_REPORT_CREATE_INFO_EXT:
        case VK_STRUCTURE_TYPE_EXPORT_METAL_OBJECT_CREATE_INFO_EXT:
        case VK_STRUCTURE_TYPE_EXPORT_METAL_COMMAND_QUEUE_INFO_EXT:
        case VK_STRUCTURE_TYPE_EXPORT_METAL_BUFFER_INFO_EXT:
        case VK_STRUCTURE_TYPE_EXPORT_METAL_TEXTURE_INFO_EXT:
        case VK_STRUCTURE_TYPE_IMPORT_METAL_TEXTURE_INFO_EXT:
        case VK_STRUCTURE_TYPE_EXPORT_METAL_IO_SURFACE_INFO_EXT:
        case VK_STRUCTURE_TYPE_EXPORT_METAL_SHARED_EVENT_INFO_EXT:
            return true;
        default:
            return false;
    }
}

bool PreCallValidateCreateInstance(const VkInstanceCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator,
                                   VkInstance* pInstance, const ErrorObject& error_obj) const override;
bool PreCallValidateDestroyInstance(VkInstance instance, const VkAllocationCallbacks* pAllocator,
                                    const ErrorObject& error_obj) const override;
bool PreCallValidateEnumeratePhysicalDevices(VkInstance instance, uint32_t* pPhysicalDeviceCount,
                                             VkPhysicalDevice* pPhysicalDevices, const ErrorObject& error_obj) const override;
bool PreCallValidateGetPhysicalDeviceFeatures(VkPhysicalDevice physicalDevice, VkPhysicalDeviceFeatures* pFeatures,
                                              const ErrorObject& error_obj) const override;
bool PreCallValidateGetPhysicalDeviceFormatProperties(VkPhysicalDevice physicalDevice, VkFormat format,
                                                      VkFormatProperties* pFormatProperties,
                                                      const ErrorObject& error_obj) const override;
bool PreCallValidateGetPhysicalDeviceImageFormatProperties(VkPhysicalDevice physicalDevice, VkFormat format, VkImageType type,
                                                           VkImageTiling tiling, VkImageUsageFlags usage, VkImageCreateFlags flags,
                                                           VkImageFormatProperties* pImageFormatProperties,
                                                           const ErrorObject& error_obj) const override;
bool PreCallValidateGetPhysicalDeviceProperties(VkPhysicalDevice physicalDevice, VkPhysicalDeviceProperties* pProperties,
                                                const ErrorObject& error_obj) const override;
bool PreCallValidateGetPhysicalDeviceQueueFamilyProperties(VkPhysicalDevice physicalDevice, uint32_t* pQueueFamilyPropertyCount,
                                                           VkQueueFamilyProperties* pQueueFamilyProperties,
                                                           const ErrorObject& error_obj) const override;
bool PreCallValidateGetPhysicalDeviceMemoryProperties(VkPhysicalDevice physicalDevice,
                                                      VkPhysicalDeviceMemoryProperties* pMemoryProperties,
                                                      const ErrorObject& error_obj) const override;
bool PreCallValidateCreateDevice(VkPhysicalDevice physicalDevice, const VkDeviceCreateInfo* pCreateInfo,
                                 const VkAllocationCallbacks* pAllocator, VkDevice* pDevice,
                                 const ErrorObject& error_obj) const override;
bool PreCallValidateDestroyDevice(VkDevice device, const VkAllocationCallbacks* pAllocator,
                                  const ErrorObject& error_obj) const override;
bool PreCallValidateGetDeviceQueue(VkDevice device, uint32_t queueFamilyIndex, uint32_t queueIndex, VkQueue* pQueue,
                                   const ErrorObject& error_obj) const override;
bool PreCallValidateQueueSubmit(VkQueue queue, uint32_t submitCount, const VkSubmitInfo* pSubmits, VkFence fence,
                                const ErrorObject& error_obj) const override;
bool PreCallValidateQueueWaitIdle(VkQueue queue, const ErrorObject& error_obj) const override;
bool PreCallValidateDeviceWaitIdle(VkDevice device, const ErrorObject& error_obj) const override;
bool PreCallValidateAllocateMemory(VkDevice device, const VkMemoryAllocateInfo* pAllocateInfo,
                                   const VkAllocationCallbacks* pAllocator, VkDeviceMemory* pMemory,
                                   const ErrorObject& error_obj) const override;
bool PreCallValidateFreeMemory(VkDevice device, VkDeviceMemory memory, const VkAllocationCallbacks* pAllocator,
                               const ErrorObject& error_obj) const override;
bool PreCallValidateMapMemory(VkDevice device, VkDeviceMemory memory, VkDeviceSize offset, VkDeviceSize size,
                              VkMemoryMapFlags flags, void** ppData, const ErrorObject& error_obj) const override;
bool PreCallValidateUnmapMemory(VkDevice device, VkDeviceMemory memory, const ErrorObject& error_obj) const override;
bool PreCallValidateFlushMappedMemoryRanges(VkDevice device, uint32_t memoryRangeCount, const VkMappedMemoryRange* pMemoryRanges,
                                            const ErrorObject& error_obj) const override;
bool PreCallValidateInvalidateMappedMemoryRanges(VkDevice device, uint32_t memoryRangeCount,
                                                 const VkMappedMemoryRange* pMemoryRanges,
                                                 const ErrorObject& error_obj) const override;
bool PreCallValidateGetDeviceMemoryCommitment(VkDevice device, VkDeviceMemory memory, VkDeviceSize* pCommittedMemoryInBytes,
                                              const ErrorObject& error_obj) const override;
bool PreCallValidateBindBufferMemory(VkDevice device, VkBuffer buffer, VkDeviceMemory memory, VkDeviceSize memoryOffset,
                                     const ErrorObject& error_obj) const override;
bool PreCallValidateBindImageMemory(VkDevice device, VkImage image, VkDeviceMemory memory, VkDeviceSize memoryOffset,
                                    const ErrorObject& error_obj) const override;
bool PreCallValidateGetBufferMemoryRequirements(VkDevice device, VkBuffer buffer, VkMemoryRequirements* pMemoryRequirements,
                                                const ErrorObject& error_obj) const override;
bool PreCallValidateGetImageMemoryRequirements(VkDevice device, VkImage image, VkMemoryRequirements* pMemoryRequirements,
                                               const ErrorObject& error_obj) const override;
bool PreCallValidateGetImageSparseMemoryRequirements(VkDevice device, VkImage image, uint32_t* pSparseMemoryRequirementCount,
                                                     VkSparseImageMemoryRequirements* pSparseMemoryRequirements,
                                                     const ErrorObject& error_obj) const override;
bool PreCallValidateGetPhysicalDeviceSparseImageFormatProperties(VkPhysicalDevice physicalDevice, VkFormat format, VkImageType type,
                                                                 VkSampleCountFlagBits samples, VkImageUsageFlags usage,
                                                                 VkImageTiling tiling, uint32_t* pPropertyCount,
                                                                 VkSparseImageFormatProperties* pProperties,
                                                                 const ErrorObject& error_obj) const override;
bool PreCallValidateQueueBindSparse(VkQueue queue, uint32_t bindInfoCount, const VkBindSparseInfo* pBindInfo, VkFence fence,
                                    const ErrorObject& error_obj) const override;
bool PreCallValidateCreateFence(VkDevice device, const VkFenceCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator,
                                VkFence* pFence, const ErrorObject& error_obj) const override;
bool PreCallValidateDestroyFence(VkDevice device, VkFence fence, const VkAllocationCallbacks* pAllocator,
                                 const ErrorObject& error_obj) const override;
bool PreCallValidateResetFences(VkDevice device, uint32_t fenceCount, const VkFence* pFences,
                                const ErrorObject& error_obj) const override;
bool PreCallValidateGetFenceStatus(VkDevice device, VkFence fence, const ErrorObject& error_obj) const override;
bool PreCallValidateWaitForFences(VkDevice device, uint32_t fenceCount, const VkFence* pFences, VkBool32 waitAll, uint64_t timeout,
                                  const ErrorObject& error_obj) const override;
bool PreCallValidateCreateSemaphore(VkDevice device, const VkSemaphoreCreateInfo* pCreateInfo,
                                    const VkAllocationCallbacks* pAllocator, VkSemaphore* pSemaphore,
                                    const ErrorObject& error_obj) const override;
bool PreCallValidateDestroySemaphore(VkDevice device, VkSemaphore semaphore, const VkAllocationCallbacks* pAllocator,
                                     const ErrorObject& error_obj) const override;
bool PreCallValidateCreateEvent(VkDevice device, const VkEventCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator,
                                VkEvent* pEvent, const ErrorObject& error_obj) const override;
bool PreCallValidateDestroyEvent(VkDevice device, VkEvent event, const VkAllocationCallbacks* pAllocator,
                                 const ErrorObject& error_obj) const override;
bool PreCallValidateGetEventStatus(VkDevice device, VkEvent event, const ErrorObject& error_obj) const override;
bool PreCallValidateSetEvent(VkDevice device, VkEvent event, const ErrorObject& error_obj) const override;
bool PreCallValidateResetEvent(VkDevice device, VkEvent event, const ErrorObject& error_obj) const override;
bool PreCallValidateCreateQueryPool(VkDevice device, const VkQueryPoolCreateInfo* pCreateInfo,
                                    const VkAllocationCallbacks* pAllocator, VkQueryPool* pQueryPool,
                                    const ErrorObject& error_obj) const override;
bool PreCallValidateDestroyQueryPool(VkDevice device, VkQueryPool queryPool, const VkAllocationCallbacks* pAllocator,
                                     const ErrorObject& error_obj) const override;
bool PreCallValidateGetQueryPoolResults(VkDevice device, VkQueryPool queryPool, uint32_t firstQuery, uint32_t queryCount,
                                        size_t dataSize, void* pData, VkDeviceSize stride, VkQueryResultFlags flags,
                                        const ErrorObject& error_obj) const override;
bool PreCallValidateCreateBuffer(VkDevice device, const VkBufferCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator,
                                 VkBuffer* pBuffer, const ErrorObject& error_obj) const override;
bool PreCallValidateDestroyBuffer(VkDevice device, VkBuffer buffer, const VkAllocationCallbacks* pAllocator,
                                  const ErrorObject& error_obj) const override;
bool PreCallValidateCreateBufferView(VkDevice device, const VkBufferViewCreateInfo* pCreateInfo,
                                     const VkAllocationCallbacks* pAllocator, VkBufferView* pView,
                                     const ErrorObject& error_obj) const override;
bool PreCallValidateDestroyBufferView(VkDevice device, VkBufferView bufferView, const VkAllocationCallbacks* pAllocator,
                                      const ErrorObject& error_obj) const override;
bool PreCallValidateCreateImage(VkDevice device, const VkImageCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator,
                                VkImage* pImage, const ErrorObject& error_obj) const override;
bool PreCallValidateDestroyImage(VkDevice device, VkImage image, const VkAllocationCallbacks* pAllocator,
                                 const ErrorObject& error_obj) const override;
bool PreCallValidateGetImageSubresourceLayout(VkDevice device, VkImage image, const VkImageSubresource* pSubresource,
                                              VkSubresourceLayout* pLayout, const ErrorObject& error_obj) const override;
bool PreCallValidateCreateImageView(VkDevice device, const VkImageViewCreateInfo* pCreateInfo,
                                    const VkAllocationCallbacks* pAllocator, VkImageView* pView,
                                    const ErrorObject& error_obj) const override;
bool PreCallValidateDestroyImageView(VkDevice device, VkImageView imageView, const VkAllocationCallbacks* pAllocator,
                                     const ErrorObject& error_obj) const override;
bool PreCallValidateCreateShaderModule(VkDevice device, const VkShaderModuleCreateInfo* pCreateInfo,
                                       const VkAllocationCallbacks* pAllocator, VkShaderModule* pShaderModule,
                                       const ErrorObject& error_obj) const override;
bool PreCallValidateDestroyShaderModule(VkDevice device, VkShaderModule shaderModule, const VkAllocationCallbacks* pAllocator,
                                        const ErrorObject& error_obj) const override;
bool PreCallValidateCreatePipelineCache(VkDevice device, const VkPipelineCacheCreateInfo* pCreateInfo,
                                        const VkAllocationCallbacks* pAllocator, VkPipelineCache* pPipelineCache,
                                        const ErrorObject& error_obj) const override;
bool PreCallValidateDestroyPipelineCache(VkDevice device, VkPipelineCache pipelineCache, const VkAllocationCallbacks* pAllocator,
                                         const ErrorObject& error_obj) const override;
bool PreCallValidateGetPipelineCacheData(VkDevice device, VkPipelineCache pipelineCache, size_t* pDataSize, void* pData,
                                         const ErrorObject& error_obj) const override;
bool PreCallValidateMergePipelineCaches(VkDevice device, VkPipelineCache dstCache, uint32_t srcCacheCount,
                                        const VkPipelineCache* pSrcCaches, const ErrorObject& error_obj) const override;
bool PreCallValidateCreateGraphicsPipelines(VkDevice device, VkPipelineCache pipelineCache, uint32_t createInfoCount,
                                            const VkGraphicsPipelineCreateInfo* pCreateInfos,
                                            const VkAllocationCallbacks* pAllocator, VkPipeline* pPipelines,
                                            const ErrorObject& error_obj) const override;
bool PreCallValidateCreateComputePipelines(VkDevice device, VkPipelineCache pipelineCache, uint32_t createInfoCount,
                                           const VkComputePipelineCreateInfo* pCreateInfos, const VkAllocationCallbacks* pAllocator,
                                           VkPipeline* pPipelines, const ErrorObject& error_obj) const override;
bool PreCallValidateDestroyPipeline(VkDevice device, VkPipeline pipeline, const VkAllocationCallbacks* pAllocator,
                                    const ErrorObject& error_obj) const override;
bool PreCallValidateCreatePipelineLayout(VkDevice device, const VkPipelineLayoutCreateInfo* pCreateInfo,
                                         const VkAllocationCallbacks* pAllocator, VkPipelineLayout* pPipelineLayout,
                                         const ErrorObject& error_obj) const override;
bool PreCallValidateDestroyPipelineLayout(VkDevice device, VkPipelineLayout pipelineLayout, const VkAllocationCallbacks* pAllocator,
                                          const ErrorObject& error_obj) const override;
bool PreCallValidateCreateSampler(VkDevice device, const VkSamplerCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator,
                                  VkSampler* pSampler, const ErrorObject& error_obj) const override;
bool PreCallValidateDestroySampler(VkDevice device, VkSampler sampler, const VkAllocationCallbacks* pAllocator,
                                   const ErrorObject& error_obj) const override;
bool PreCallValidateCreateDescriptorSetLayout(VkDevice device, const VkDescriptorSetLayoutCreateInfo* pCreateInfo,
                                              const VkAllocationCallbacks* pAllocator, VkDescriptorSetLayout* pSetLayout,
                                              const ErrorObject& error_obj) const override;
bool PreCallValidateDestroyDescriptorSetLayout(VkDevice device, VkDescriptorSetLayout descriptorSetLayout,
                                               const VkAllocationCallbacks* pAllocator,
                                               const ErrorObject& error_obj) const override;
bool PreCallValidateCreateDescriptorPool(VkDevice device, const VkDescriptorPoolCreateInfo* pCreateInfo,
                                         const VkAllocationCallbacks* pAllocator, VkDescriptorPool* pDescriptorPool,
                                         const ErrorObject& error_obj) const override;
bool PreCallValidateDestroyDescriptorPool(VkDevice device, VkDescriptorPool descriptorPool, const VkAllocationCallbacks* pAllocator,
                                          const ErrorObject& error_obj) const override;
bool PreCallValidateResetDescriptorPool(VkDevice device, VkDescriptorPool descriptorPool, VkDescriptorPoolResetFlags flags,
                                        const ErrorObject& error_obj) const override;
bool PreCallValidateAllocateDescriptorSets(VkDevice device, const VkDescriptorSetAllocateInfo* pAllocateInfo,
                                           VkDescriptorSet* pDescriptorSets, const ErrorObject& error_obj) const override;
bool PreCallValidateFreeDescriptorSets(VkDevice device, VkDescriptorPool descriptorPool, uint32_t descriptorSetCount,
                                       const VkDescriptorSet* pDescriptorSets, const ErrorObject& error_obj) const override;
bool PreCallValidateUpdateDescriptorSets(VkDevice device, uint32_t descriptorWriteCount,
                                         const VkWriteDescriptorSet* pDescriptorWrites, uint32_t descriptorCopyCount,
                                         const VkCopyDescriptorSet* pDescriptorCopies, const ErrorObject& error_obj) const override;
bool PreCallValidateCreateFramebuffer(VkDevice device, const VkFramebufferCreateInfo* pCreateInfo,
                                      const VkAllocationCallbacks* pAllocator, VkFramebuffer* pFramebuffer,
                                      const ErrorObject& error_obj) const override;
bool PreCallValidateDestroyFramebuffer(VkDevice device, VkFramebuffer framebuffer, const VkAllocationCallbacks* pAllocator,
                                       const ErrorObject& error_obj) const override;
bool PreCallValidateCreateRenderPass(VkDevice device, const VkRenderPassCreateInfo* pCreateInfo,
                                     const VkAllocationCallbacks* pAllocator, VkRenderPass* pRenderPass,
                                     const ErrorObject& error_obj) const override;
bool PreCallValidateDestroyRenderPass(VkDevice device, VkRenderPass renderPass, const VkAllocationCallbacks* pAllocator,
                                      const ErrorObject& error_obj) const override;
bool PreCallValidateGetRenderAreaGranularity(VkDevice device, VkRenderPass renderPass, VkExtent2D* pGranularity,
                                             const ErrorObject& error_obj) const override;
bool PreCallValidateCreateCommandPool(VkDevice device, const VkCommandPoolCreateInfo* pCreateInfo,
                                      const VkAllocationCallbacks* pAllocator, VkCommandPool* pCommandPool,
                                      const ErrorObject& error_obj) const override;
bool PreCallValidateDestroyCommandPool(VkDevice device, VkCommandPool commandPool, const VkAllocationCallbacks* pAllocator,
                                       const ErrorObject& error_obj) const override;
bool PreCallValidateResetCommandPool(VkDevice device, VkCommandPool commandPool, VkCommandPoolResetFlags flags,
                                     const ErrorObject& error_obj) const override;
bool PreCallValidateAllocateCommandBuffers(VkDevice device, const VkCommandBufferAllocateInfo* pAllocateInfo,
                                           VkCommandBuffer* pCommandBuffers, const ErrorObject& error_obj) const override;
bool PreCallValidateFreeCommandBuffers(VkDevice device, VkCommandPool commandPool, uint32_t commandBufferCount,
                                       const VkCommandBuffer* pCommandBuffers, const ErrorObject& error_obj) const override;
bool PreCallValidateBeginCommandBuffer(VkCommandBuffer commandBuffer, const VkCommandBufferBeginInfo* pBeginInfo,
                                       const ErrorObject& error_obj) const override;
bool PreCallValidateEndCommandBuffer(VkCommandBuffer commandBuffer, const ErrorObject& error_obj) const override;
bool PreCallValidateResetCommandBuffer(VkCommandBuffer commandBuffer, VkCommandBufferResetFlags flags,
                                       const ErrorObject& error_obj) const override;
bool PreCallValidateCmdBindPipeline(VkCommandBuffer commandBuffer, VkPipelineBindPoint pipelineBindPoint, VkPipeline pipeline,
                                    const ErrorObject& error_obj) const override;
bool PreCallValidateCmdSetViewport(VkCommandBuffer commandBuffer, uint32_t firstViewport, uint32_t viewportCount,
                                   const VkViewport* pViewports, const ErrorObject& error_obj) const override;
bool PreCallValidateCmdSetScissor(VkCommandBuffer commandBuffer, uint32_t firstScissor, uint32_t scissorCount,
                                  const VkRect2D* pScissors, const ErrorObject& error_obj) const override;
bool PreCallValidateCmdSetLineWidth(VkCommandBuffer commandBuffer, float lineWidth, const ErrorObject& error_obj) const override;
bool PreCallValidateCmdSetDepthBias(VkCommandBuffer commandBuffer, float depthBiasConstantFactor, float depthBiasClamp,
                                    float depthBiasSlopeFactor, const ErrorObject& error_obj) const override;
bool PreCallValidateCmdSetBlendConstants(VkCommandBuffer commandBuffer, const float blendConstants[4],
                                         const ErrorObject& error_obj) const override;
bool PreCallValidateCmdSetDepthBounds(VkCommandBuffer commandBuffer, float minDepthBounds, float maxDepthBounds,
                                      const ErrorObject& error_obj) const override;
bool PreCallValidateCmdSetStencilCompareMask(VkCommandBuffer commandBuffer, VkStencilFaceFlags faceMask, uint32_t compareMask,
                                             const ErrorObject& error_obj) const override;
bool PreCallValidateCmdSetStencilWriteMask(VkCommandBuffer commandBuffer, VkStencilFaceFlags faceMask, uint32_t writeMask,
                                           const ErrorObject& error_obj) const override;
bool PreCallValidateCmdSetStencilReference(VkCommandBuffer commandBuffer, VkStencilFaceFlags faceMask, uint32_t reference,
                                           const ErrorObject& error_obj) const override;
bool PreCallValidateCmdBindDescriptorSets(VkCommandBuffer commandBuffer, VkPipelineBindPoint pipelineBindPoint,
                                          VkPipelineLayout layout, uint32_t firstSet, uint32_t descriptorSetCount,
                                          const VkDescriptorSet* pDescriptorSets, uint32_t dynamicOffsetCount,
                                          const uint32_t* pDynamicOffsets, const ErrorObject& error_obj) const override;
bool PreCallValidateCmdBindIndexBuffer(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset, VkIndexType indexType,
                                       const ErrorObject& error_obj) const override;
bool PreCallValidateCmdBindVertexBuffers(VkCommandBuffer commandBuffer, uint32_t firstBinding, uint32_t bindingCount,
                                         const VkBuffer* pBuffers, const VkDeviceSize* pOffsets,
                                         const ErrorObject& error_obj) const override;
bool PreCallValidateCmdDraw(VkCommandBuffer commandBuffer, uint32_t vertexCount, uint32_t instanceCount, uint32_t firstVertex,
                            uint32_t firstInstance, const ErrorObject& error_obj) const override;
bool PreCallValidateCmdDrawIndexed(VkCommandBuffer commandBuffer, uint32_t indexCount, uint32_t instanceCount, uint32_t firstIndex,
                                   int32_t vertexOffset, uint32_t firstInstance, const ErrorObject& error_obj) const override;
bool PreCallValidateCmdDrawIndirect(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset, uint32_t drawCount,
                                    uint32_t stride, const ErrorObject& error_obj) const override;
bool PreCallValidateCmdDrawIndexedIndirect(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset, uint32_t drawCount,
                                           uint32_t stride, const ErrorObject& error_obj) const override;
bool PreCallValidateCmdDispatch(VkCommandBuffer commandBuffer, uint32_t groupCountX, uint32_t groupCountY, uint32_t groupCountZ,
                                const ErrorObject& error_obj) const override;
bool PreCallValidateCmdDispatchIndirect(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                        const ErrorObject& error_obj) const override;
bool PreCallValidateCmdCopyBuffer(VkCommandBuffer commandBuffer, VkBuffer srcBuffer, VkBuffer dstBuffer, uint32_t regionCount,
                                  const VkBufferCopy* pRegions, const ErrorObject& error_obj) const override;
bool PreCallValidateCmdCopyImage(VkCommandBuffer commandBuffer, VkImage srcImage, VkImageLayout srcImageLayout, VkImage dstImage,
                                 VkImageLayout dstImageLayout, uint32_t regionCount, const VkImageCopy* pRegions,
                                 const ErrorObject& error_obj) const override;
bool PreCallValidateCmdBlitImage(VkCommandBuffer commandBuffer, VkImage srcImage, VkImageLayout srcImageLayout, VkImage dstImage,
                                 VkImageLayout dstImageLayout, uint32_t regionCount, const VkImageBlit* pRegions, VkFilter filter,
                                 const ErrorObject& error_obj) const override;
bool PreCallValidateCmdCopyBufferToImage(VkCommandBuffer commandBuffer, VkBuffer srcBuffer, VkImage dstImage,
                                         VkImageLayout dstImageLayout, uint32_t regionCount, const VkBufferImageCopy* pRegions,
                                         const ErrorObject& error_obj) const override;
bool PreCallValidateCmdCopyImageToBuffer(VkCommandBuffer commandBuffer, VkImage srcImage, VkImageLayout srcImageLayout,
                                         VkBuffer dstBuffer, uint32_t regionCount, const VkBufferImageCopy* pRegions,
                                         const ErrorObject& error_obj) const override;
bool PreCallValidateCmdUpdateBuffer(VkCommandBuffer commandBuffer, VkBuffer dstBuffer, VkDeviceSize dstOffset,
                                    VkDeviceSize dataSize, const void* pData, const ErrorObject& error_obj) const override;
bool PreCallValidateCmdFillBuffer(VkCommandBuffer commandBuffer, VkBuffer dstBuffer, VkDeviceSize dstOffset, VkDeviceSize size,
                                  uint32_t data, const ErrorObject& error_obj) const override;
bool PreCallValidateCmdClearColorImage(VkCommandBuffer commandBuffer, VkImage image, VkImageLayout imageLayout,
                                       const VkClearColorValue* pColor, uint32_t rangeCount, const VkImageSubresourceRange* pRanges,
                                       const ErrorObject& error_obj) const override;
bool PreCallValidateCmdClearDepthStencilImage(VkCommandBuffer commandBuffer, VkImage image, VkImageLayout imageLayout,
                                              const VkClearDepthStencilValue* pDepthStencil, uint32_t rangeCount,
                                              const VkImageSubresourceRange* pRanges, const ErrorObject& error_obj) const override;
bool PreCallValidateCmdClearAttachments(VkCommandBuffer commandBuffer, uint32_t attachmentCount,
                                        const VkClearAttachment* pAttachments, uint32_t rectCount, const VkClearRect* pRects,
                                        const ErrorObject& error_obj) const override;
bool PreCallValidateCmdResolveImage(VkCommandBuffer commandBuffer, VkImage srcImage, VkImageLayout srcImageLayout, VkImage dstImage,
                                    VkImageLayout dstImageLayout, uint32_t regionCount, const VkImageResolve* pRegions,
                                    const ErrorObject& error_obj) const override;
bool PreCallValidateCmdSetEvent(VkCommandBuffer commandBuffer, VkEvent event, VkPipelineStageFlags stageMask,
                                const ErrorObject& error_obj) const override;
bool PreCallValidateCmdResetEvent(VkCommandBuffer commandBuffer, VkEvent event, VkPipelineStageFlags stageMask,
                                  const ErrorObject& error_obj) const override;
bool PreCallValidateCmdWaitEvents(VkCommandBuffer commandBuffer, uint32_t eventCount, const VkEvent* pEvents,
                                  VkPipelineStageFlags srcStageMask, VkPipelineStageFlags dstStageMask, uint32_t memoryBarrierCount,
                                  const VkMemoryBarrier* pMemoryBarriers, uint32_t bufferMemoryBarrierCount,
                                  const VkBufferMemoryBarrier* pBufferMemoryBarriers, uint32_t imageMemoryBarrierCount,
                                  const VkImageMemoryBarrier* pImageMemoryBarriers, const ErrorObject& error_obj) const override;
bool PreCallValidateCmdPipelineBarrier(VkCommandBuffer commandBuffer, VkPipelineStageFlags srcStageMask,
                                       VkPipelineStageFlags dstStageMask, VkDependencyFlags dependencyFlags,
                                       uint32_t memoryBarrierCount, const VkMemoryBarrier* pMemoryBarriers,
                                       uint32_t bufferMemoryBarrierCount, const VkBufferMemoryBarrier* pBufferMemoryBarriers,
                                       uint32_t imageMemoryBarrierCount, const VkImageMemoryBarrier* pImageMemoryBarriers,
                                       const ErrorObject& error_obj) const override;
bool PreCallValidateCmdBeginQuery(VkCommandBuffer commandBuffer, VkQueryPool queryPool, uint32_t query, VkQueryControlFlags flags,
                                  const ErrorObject& error_obj) const override;
bool PreCallValidateCmdEndQuery(VkCommandBuffer commandBuffer, VkQueryPool queryPool, uint32_t query,
                                const ErrorObject& error_obj) const override;
bool PreCallValidateCmdResetQueryPool(VkCommandBuffer commandBuffer, VkQueryPool queryPool, uint32_t firstQuery,
                                      uint32_t queryCount, const ErrorObject& error_obj) const override;
bool PreCallValidateCmdWriteTimestamp(VkCommandBuffer commandBuffer, VkPipelineStageFlagBits pipelineStage, VkQueryPool queryPool,
                                      uint32_t query, const ErrorObject& error_obj) const override;
bool PreCallValidateCmdCopyQueryPoolResults(VkCommandBuffer commandBuffer, VkQueryPool queryPool, uint32_t firstQuery,
                                            uint32_t queryCount, VkBuffer dstBuffer, VkDeviceSize dstOffset, VkDeviceSize stride,
                                            VkQueryResultFlags flags, const ErrorObject& error_obj) const override;
bool PreCallValidateCmdPushConstants(VkCommandBuffer commandBuffer, VkPipelineLayout layout, VkShaderStageFlags stageFlags,
                                     uint32_t offset, uint32_t size, const void* pValues,
                                     const ErrorObject& error_obj) const override;
bool PreCallValidateCmdBeginRenderPass(VkCommandBuffer commandBuffer, const VkRenderPassBeginInfo* pRenderPassBegin,
                                       VkSubpassContents contents, const ErrorObject& error_obj) const override;
bool PreCallValidateCmdNextSubpass(VkCommandBuffer commandBuffer, VkSubpassContents contents,
                                   const ErrorObject& error_obj) const override;
bool PreCallValidateCmdEndRenderPass(VkCommandBuffer commandBuffer, const ErrorObject& error_obj) const override;
bool PreCallValidateCmdExecuteCommands(VkCommandBuffer commandBuffer, uint32_t commandBufferCount,
                                       const VkCommandBuffer* pCommandBuffers, const ErrorObject& error_obj) const override;
bool PreCallValidateBindBufferMemory2(VkDevice device, uint32_t bindInfoCount, const VkBindBufferMemoryInfo* pBindInfos,
                                      const ErrorObject& error_obj) const override;
bool PreCallValidateBindImageMemory2(VkDevice device, uint32_t bindInfoCount, const VkBindImageMemoryInfo* pBindInfos,
                                     const ErrorObject& error_obj) const override;
bool PreCallValidateGetDeviceGroupPeerMemoryFeatures(VkDevice device, uint32_t heapIndex, uint32_t localDeviceIndex,
                                                     uint32_t remoteDeviceIndex, VkPeerMemoryFeatureFlags* pPeerMemoryFeatures,
                                                     const ErrorObject& error_obj) const override;
bool PreCallValidateCmdSetDeviceMask(VkCommandBuffer commandBuffer, uint32_t deviceMask,
                                     const ErrorObject& error_obj) const override;
bool PreCallValidateCmdDispatchBase(VkCommandBuffer commandBuffer, uint32_t baseGroupX, uint32_t baseGroupY, uint32_t baseGroupZ,
                                    uint32_t groupCountX, uint32_t groupCountY, uint32_t groupCountZ,
                                    const ErrorObject& error_obj) const override;
bool PreCallValidateEnumeratePhysicalDeviceGroups(VkInstance instance, uint32_t* pPhysicalDeviceGroupCount,
                                                  VkPhysicalDeviceGroupProperties* pPhysicalDeviceGroupProperties,
                                                  const ErrorObject& error_obj) const override;
bool PreCallValidateGetImageMemoryRequirements2(VkDevice device, const VkImageMemoryRequirementsInfo2* pInfo,
                                                VkMemoryRequirements2* pMemoryRequirements,
                                                const ErrorObject& error_obj) const override;
bool PreCallValidateGetBufferMemoryRequirements2(VkDevice device, const VkBufferMemoryRequirementsInfo2* pInfo,
                                                 VkMemoryRequirements2* pMemoryRequirements,
                                                 const ErrorObject& error_obj) const override;
bool PreCallValidateGetImageSparseMemoryRequirements2(VkDevice device, const VkImageSparseMemoryRequirementsInfo2* pInfo,
                                                      uint32_t* pSparseMemoryRequirementCount,
                                                      VkSparseImageMemoryRequirements2* pSparseMemoryRequirements,
                                                      const ErrorObject& error_obj) const override;
bool PreCallValidateGetPhysicalDeviceFeatures2(VkPhysicalDevice physicalDevice, VkPhysicalDeviceFeatures2* pFeatures,
                                               const ErrorObject& error_obj) const override;
bool PreCallValidateGetPhysicalDeviceProperties2(VkPhysicalDevice physicalDevice, VkPhysicalDeviceProperties2* pProperties,
                                                 const ErrorObject& error_obj) const override;
bool PreCallValidateGetPhysicalDeviceFormatProperties2(VkPhysicalDevice physicalDevice, VkFormat format,
                                                       VkFormatProperties2* pFormatProperties,
                                                       const ErrorObject& error_obj) const override;
bool PreCallValidateGetPhysicalDeviceImageFormatProperties2(VkPhysicalDevice physicalDevice,
                                                            const VkPhysicalDeviceImageFormatInfo2* pImageFormatInfo,
                                                            VkImageFormatProperties2* pImageFormatProperties,
                                                            const ErrorObject& error_obj) const override;
bool PreCallValidateGetPhysicalDeviceQueueFamilyProperties2(VkPhysicalDevice physicalDevice, uint32_t* pQueueFamilyPropertyCount,
                                                            VkQueueFamilyProperties2* pQueueFamilyProperties,
                                                            const ErrorObject& error_obj) const override;
bool PreCallValidateGetPhysicalDeviceMemoryProperties2(VkPhysicalDevice physicalDevice,
                                                       VkPhysicalDeviceMemoryProperties2* pMemoryProperties,
                                                       const ErrorObject& error_obj) const override;
bool PreCallValidateGetPhysicalDeviceSparseImageFormatProperties2(VkPhysicalDevice physicalDevice,
                                                                  const VkPhysicalDeviceSparseImageFormatInfo2* pFormatInfo,
                                                                  uint32_t* pPropertyCount,
                                                                  VkSparseImageFormatProperties2* pProperties,
                                                                  const ErrorObject& error_obj) const override;
bool PreCallValidateTrimCommandPool(VkDevice device, VkCommandPool commandPool, VkCommandPoolTrimFlags flags,
                                    const ErrorObject& error_obj) const override;
bool PreCallValidateGetDeviceQueue2(VkDevice device, const VkDeviceQueueInfo2* pQueueInfo, VkQueue* pQueue,
                                    const ErrorObject& error_obj) const override;
bool PreCallValidateCreateSamplerYcbcrConversion(VkDevice device, const VkSamplerYcbcrConversionCreateInfo* pCreateInfo,
                                                 const VkAllocationCallbacks* pAllocator,
                                                 VkSamplerYcbcrConversion* pYcbcrConversion,
                                                 const ErrorObject& error_obj) const override;
bool PreCallValidateDestroySamplerYcbcrConversion(VkDevice device, VkSamplerYcbcrConversion ycbcrConversion,
                                                  const VkAllocationCallbacks* pAllocator,
                                                  const ErrorObject& error_obj) const override;
bool PreCallValidateCreateDescriptorUpdateTemplate(VkDevice device, const VkDescriptorUpdateTemplateCreateInfo* pCreateInfo,
                                                   const VkAllocationCallbacks* pAllocator,
                                                   VkDescriptorUpdateTemplate* pDescriptorUpdateTemplate,
                                                   const ErrorObject& error_obj) const override;
bool PreCallValidateDestroyDescriptorUpdateTemplate(VkDevice device, VkDescriptorUpdateTemplate descriptorUpdateTemplate,
                                                    const VkAllocationCallbacks* pAllocator,
                                                    const ErrorObject& error_obj) const override;
bool PreCallValidateUpdateDescriptorSetWithTemplate(VkDevice device, VkDescriptorSet descriptorSet,
                                                    VkDescriptorUpdateTemplate descriptorUpdateTemplate, const void* pData,
                                                    const ErrorObject& error_obj) const override;
bool PreCallValidateGetPhysicalDeviceExternalBufferProperties(VkPhysicalDevice physicalDevice,
                                                              const VkPhysicalDeviceExternalBufferInfo* pExternalBufferInfo,
                                                              VkExternalBufferProperties* pExternalBufferProperties,
                                                              const ErrorObject& error_obj) const override;
bool PreCallValidateGetPhysicalDeviceExternalFenceProperties(VkPhysicalDevice physicalDevice,
                                                             const VkPhysicalDeviceExternalFenceInfo* pExternalFenceInfo,
                                                             VkExternalFenceProperties* pExternalFenceProperties,
                                                             const ErrorObject& error_obj) const override;
bool PreCallValidateGetPhysicalDeviceExternalSemaphoreProperties(
    VkPhysicalDevice physicalDevice, const VkPhysicalDeviceExternalSemaphoreInfo* pExternalSemaphoreInfo,
    VkExternalSemaphoreProperties* pExternalSemaphoreProperties, const ErrorObject& error_obj) const override;
bool PreCallValidateGetDescriptorSetLayoutSupport(VkDevice device, const VkDescriptorSetLayoutCreateInfo* pCreateInfo,
                                                  VkDescriptorSetLayoutSupport* pSupport,
                                                  const ErrorObject& error_obj) const override;
bool PreCallValidateCmdDrawIndirectCount(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset, VkBuffer countBuffer,
                                         VkDeviceSize countBufferOffset, uint32_t maxDrawCount, uint32_t stride,
                                         const ErrorObject& error_obj) const override;
bool PreCallValidateCmdDrawIndexedIndirectCount(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                VkBuffer countBuffer, VkDeviceSize countBufferOffset, uint32_t maxDrawCount,
                                                uint32_t stride, const ErrorObject& error_obj) const override;
bool PreCallValidateCreateRenderPass2(VkDevice device, const VkRenderPassCreateInfo2* pCreateInfo,
                                      const VkAllocationCallbacks* pAllocator, VkRenderPass* pRenderPass,
                                      const ErrorObject& error_obj) const override;
bool PreCallValidateCmdBeginRenderPass2(VkCommandBuffer commandBuffer, const VkRenderPassBeginInfo* pRenderPassBegin,
                                        const VkSubpassBeginInfo* pSubpassBeginInfo, const ErrorObject& error_obj) const override;
bool PreCallValidateCmdNextSubpass2(VkCommandBuffer commandBuffer, const VkSubpassBeginInfo* pSubpassBeginInfo,
                                    const VkSubpassEndInfo* pSubpassEndInfo, const ErrorObject& error_obj) const override;
bool PreCallValidateCmdEndRenderPass2(VkCommandBuffer commandBuffer, const VkSubpassEndInfo* pSubpassEndInfo,
                                      const ErrorObject& error_obj) const override;
bool PreCallValidateResetQueryPool(VkDevice device, VkQueryPool queryPool, uint32_t firstQuery, uint32_t queryCount,
                                   const ErrorObject& error_obj) const override;
bool PreCallValidateGetSemaphoreCounterValue(VkDevice device, VkSemaphore semaphore, uint64_t* pValue,
                                             const ErrorObject& error_obj) const override;
bool PreCallValidateWaitSemaphores(VkDevice device, const VkSemaphoreWaitInfo* pWaitInfo, uint64_t timeout,
                                   const ErrorObject& error_obj) const override;
bool PreCallValidateSignalSemaphore(VkDevice device, const VkSemaphoreSignalInfo* pSignalInfo,
                                    const ErrorObject& error_obj) const override;
bool PreCallValidateGetBufferDeviceAddress(VkDevice device, const VkBufferDeviceAddressInfo* pInfo,
                                           const ErrorObject& error_obj) const override;
bool PreCallValidateGetBufferOpaqueCaptureAddress(VkDevice device, const VkBufferDeviceAddressInfo* pInfo,
                                                  const ErrorObject& error_obj) const override;
bool PreCallValidateGetDeviceMemoryOpaqueCaptureAddress(VkDevice device, const VkDeviceMemoryOpaqueCaptureAddressInfo* pInfo,
                                                        const ErrorObject& error_obj) const override;
bool PreCallValidateGetPhysicalDeviceToolProperties(VkPhysicalDevice physicalDevice, uint32_t* pToolCount,
                                                    VkPhysicalDeviceToolProperties* pToolProperties,
                                                    const ErrorObject& error_obj) const override;
bool PreCallValidateCreatePrivateDataSlot(VkDevice device, const VkPrivateDataSlotCreateInfo* pCreateInfo,
                                          const VkAllocationCallbacks* pAllocator, VkPrivateDataSlot* pPrivateDataSlot,
                                          const ErrorObject& error_obj) const override;
bool PreCallValidateDestroyPrivateDataSlot(VkDevice device, VkPrivateDataSlot privateDataSlot,
                                           const VkAllocationCallbacks* pAllocator, const ErrorObject& error_obj) const override;
bool PreCallValidateSetPrivateData(VkDevice device, VkObjectType objectType, uint64_t objectHandle,
                                   VkPrivateDataSlot privateDataSlot, uint64_t data, const ErrorObject& error_obj) const override;
bool PreCallValidateGetPrivateData(VkDevice device, VkObjectType objectType, uint64_t objectHandle,
                                   VkPrivateDataSlot privateDataSlot, uint64_t* pData, const ErrorObject& error_obj) const override;
bool PreCallValidateCmdSetEvent2(VkCommandBuffer commandBuffer, VkEvent event, const VkDependencyInfo* pDependencyInfo,
                                 const ErrorObject& error_obj) const override;
bool PreCallValidateCmdResetEvent2(VkCommandBuffer commandBuffer, VkEvent event, VkPipelineStageFlags2 stageMask,
                                   const ErrorObject& error_obj) const override;
bool PreCallValidateCmdWaitEvents2(VkCommandBuffer commandBuffer, uint32_t eventCount, const VkEvent* pEvents,
                                   const VkDependencyInfo* pDependencyInfos, const ErrorObject& error_obj) const override;
bool PreCallValidateCmdPipelineBarrier2(VkCommandBuffer commandBuffer, const VkDependencyInfo* pDependencyInfo,
                                        const ErrorObject& error_obj) const override;
bool PreCallValidateCmdWriteTimestamp2(VkCommandBuffer commandBuffer, VkPipelineStageFlags2 stage, VkQueryPool queryPool,
                                       uint32_t query, const ErrorObject& error_obj) const override;
bool PreCallValidateQueueSubmit2(VkQueue queue, uint32_t submitCount, const VkSubmitInfo2* pSubmits, VkFence fence,
                                 const ErrorObject& error_obj) const override;
bool PreCallValidateCmdCopyBuffer2(VkCommandBuffer commandBuffer, const VkCopyBufferInfo2* pCopyBufferInfo,
                                   const ErrorObject& error_obj) const override;
bool PreCallValidateCmdCopyImage2(VkCommandBuffer commandBuffer, const VkCopyImageInfo2* pCopyImageInfo,
                                  const ErrorObject& error_obj) const override;
bool PreCallValidateCmdCopyBufferToImage2(VkCommandBuffer commandBuffer, const VkCopyBufferToImageInfo2* pCopyBufferToImageInfo,
                                          const ErrorObject& error_obj) const override;
bool PreCallValidateCmdCopyImageToBuffer2(VkCommandBuffer commandBuffer, const VkCopyImageToBufferInfo2* pCopyImageToBufferInfo,
                                          const ErrorObject& error_obj) const override;
bool PreCallValidateCmdBlitImage2(VkCommandBuffer commandBuffer, const VkBlitImageInfo2* pBlitImageInfo,
                                  const ErrorObject& error_obj) const override;
bool PreCallValidateCmdResolveImage2(VkCommandBuffer commandBuffer, const VkResolveImageInfo2* pResolveImageInfo,
                                     const ErrorObject& error_obj) const override;
bool PreCallValidateCmdBeginRendering(VkCommandBuffer commandBuffer, const VkRenderingInfo* pRenderingInfo,
                                      const ErrorObject& error_obj) const override;
bool PreCallValidateCmdEndRendering(VkCommandBuffer commandBuffer, const ErrorObject& error_obj) const override;
bool PreCallValidateCmdSetCullMode(VkCommandBuffer commandBuffer, VkCullModeFlags cullMode,
                                   const ErrorObject& error_obj) const override;
bool PreCallValidateCmdSetFrontFace(VkCommandBuffer commandBuffer, VkFrontFace frontFace,
                                    const ErrorObject& error_obj) const override;
bool PreCallValidateCmdSetPrimitiveTopology(VkCommandBuffer commandBuffer, VkPrimitiveTopology primitiveTopology,
                                            const ErrorObject& error_obj) const override;
bool PreCallValidateCmdSetViewportWithCount(VkCommandBuffer commandBuffer, uint32_t viewportCount, const VkViewport* pViewports,
                                            const ErrorObject& error_obj) const override;
bool PreCallValidateCmdSetScissorWithCount(VkCommandBuffer commandBuffer, uint32_t scissorCount, const VkRect2D* pScissors,
                                           const ErrorObject& error_obj) const override;
bool PreCallValidateCmdBindVertexBuffers2(VkCommandBuffer commandBuffer, uint32_t firstBinding, uint32_t bindingCount,
                                          const VkBuffer* pBuffers, const VkDeviceSize* pOffsets, const VkDeviceSize* pSizes,
                                          const VkDeviceSize* pStrides, const ErrorObject& error_obj) const override;
bool PreCallValidateCmdSetDepthTestEnable(VkCommandBuffer commandBuffer, VkBool32 depthTestEnable,
                                          const ErrorObject& error_obj) const override;
bool PreCallValidateCmdSetDepthWriteEnable(VkCommandBuffer commandBuffer, VkBool32 depthWriteEnable,
                                           const ErrorObject& error_obj) const override;
bool PreCallValidateCmdSetDepthCompareOp(VkCommandBuffer commandBuffer, VkCompareOp depthCompareOp,
                                         const ErrorObject& error_obj) const override;
bool PreCallValidateCmdSetDepthBoundsTestEnable(VkCommandBuffer commandBuffer, VkBool32 depthBoundsTestEnable,
                                                const ErrorObject& error_obj) const override;
bool PreCallValidateCmdSetStencilTestEnable(VkCommandBuffer commandBuffer, VkBool32 stencilTestEnable,
                                            const ErrorObject& error_obj) const override;
bool PreCallValidateCmdSetStencilOp(VkCommandBuffer commandBuffer, VkStencilFaceFlags faceMask, VkStencilOp failOp,
                                    VkStencilOp passOp, VkStencilOp depthFailOp, VkCompareOp compareOp,
                                    const ErrorObject& error_obj) const override;
bool PreCallValidateCmdSetRasterizerDiscardEnable(VkCommandBuffer commandBuffer, VkBool32 rasterizerDiscardEnable,
                                                  const ErrorObject& error_obj) const override;
bool PreCallValidateCmdSetDepthBiasEnable(VkCommandBuffer commandBuffer, VkBool32 depthBiasEnable,
                                          const ErrorObject& error_obj) const override;
bool PreCallValidateCmdSetPrimitiveRestartEnable(VkCommandBuffer commandBuffer, VkBool32 primitiveRestartEnable,
                                                 const ErrorObject& error_obj) const override;
bool PreCallValidateGetDeviceBufferMemoryRequirements(VkDevice device, const VkDeviceBufferMemoryRequirements* pInfo,
                                                      VkMemoryRequirements2* pMemoryRequirements,
                                                      const ErrorObject& error_obj) const override;
bool PreCallValidateGetDeviceImageMemoryRequirements(VkDevice device, const VkDeviceImageMemoryRequirements* pInfo,
                                                     VkMemoryRequirements2* pMemoryRequirements,
                                                     const ErrorObject& error_obj) const override;
bool PreCallValidateGetDeviceImageSparseMemoryRequirements(VkDevice device, const VkDeviceImageMemoryRequirements* pInfo,
                                                           uint32_t* pSparseMemoryRequirementCount,
                                                           VkSparseImageMemoryRequirements2* pSparseMemoryRequirements,
                                                           const ErrorObject& error_obj) const override;
bool PreCallValidateDestroySurfaceKHR(VkInstance instance, VkSurfaceKHR surface, const VkAllocationCallbacks* pAllocator,
                                      const ErrorObject& error_obj) const override;
bool PreCallValidateGetPhysicalDeviceSurfaceSupportKHR(VkPhysicalDevice physicalDevice, uint32_t queueFamilyIndex,
                                                       VkSurfaceKHR surface, VkBool32* pSupported,
                                                       const ErrorObject& error_obj) const override;
bool PreCallValidateGetPhysicalDeviceSurfaceCapabilitiesKHR(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface,
                                                            VkSurfaceCapabilitiesKHR* pSurfaceCapabilities,
                                                            const ErrorObject& error_obj) const override;
bool PreCallValidateGetPhysicalDeviceSurfaceFormatsKHR(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface,
                                                       uint32_t* pSurfaceFormatCount, VkSurfaceFormatKHR* pSurfaceFormats,
                                                       const ErrorObject& error_obj) const override;
bool PreCallValidateGetPhysicalDeviceSurfacePresentModesKHR(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface,
                                                            uint32_t* pPresentModeCount, VkPresentModeKHR* pPresentModes,
                                                            const ErrorObject& error_obj) const override;
bool PreCallValidateCreateSwapchainKHR(VkDevice device, const VkSwapchainCreateInfoKHR* pCreateInfo,
                                       const VkAllocationCallbacks* pAllocator, VkSwapchainKHR* pSwapchain,
                                       const ErrorObject& error_obj) const override;
bool PreCallValidateDestroySwapchainKHR(VkDevice device, VkSwapchainKHR swapchain, const VkAllocationCallbacks* pAllocator,
                                        const ErrorObject& error_obj) const override;
bool PreCallValidateGetSwapchainImagesKHR(VkDevice device, VkSwapchainKHR swapchain, uint32_t* pSwapchainImageCount,
                                          VkImage* pSwapchainImages, const ErrorObject& error_obj) const override;
bool PreCallValidateAcquireNextImageKHR(VkDevice device, VkSwapchainKHR swapchain, uint64_t timeout, VkSemaphore semaphore,
                                        VkFence fence, uint32_t* pImageIndex, const ErrorObject& error_obj) const override;
bool PreCallValidateQueuePresentKHR(VkQueue queue, const VkPresentInfoKHR* pPresentInfo,
                                    const ErrorObject& error_obj) const override;
bool PreCallValidateGetDeviceGroupPresentCapabilitiesKHR(VkDevice device,
                                                         VkDeviceGroupPresentCapabilitiesKHR* pDeviceGroupPresentCapabilities,
                                                         const ErrorObject& error_obj) const override;
bool PreCallValidateGetDeviceGroupSurfacePresentModesKHR(VkDevice device, VkSurfaceKHR surface,
                                                         VkDeviceGroupPresentModeFlagsKHR* pModes,
                                                         const ErrorObject& error_obj) const override;
bool PreCallValidateGetPhysicalDevicePresentRectanglesKHR(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface,
                                                          uint32_t* pRectCount, VkRect2D* pRects,
                                                          const ErrorObject& error_obj) const override;
bool PreCallValidateAcquireNextImage2KHR(VkDevice device, const VkAcquireNextImageInfoKHR* pAcquireInfo, uint32_t* pImageIndex,
                                         const ErrorObject& error_obj) const override;
bool PreCallValidateGetPhysicalDeviceDisplayPropertiesKHR(VkPhysicalDevice physicalDevice, uint32_t* pPropertyCount,
                                                          VkDisplayPropertiesKHR* pProperties,
                                                          const ErrorObject& error_obj) const override;
bool PreCallValidateGetPhysicalDeviceDisplayPlanePropertiesKHR(VkPhysicalDevice physicalDevice, uint32_t* pPropertyCount,
                                                               VkDisplayPlanePropertiesKHR* pProperties,
                                                               const ErrorObject& error_obj) const override;
bool PreCallValidateGetDisplayPlaneSupportedDisplaysKHR(VkPhysicalDevice physicalDevice, uint32_t planeIndex,
                                                        uint32_t* pDisplayCount, VkDisplayKHR* pDisplays,
                                                        const ErrorObject& error_obj) const override;
bool PreCallValidateGetDisplayModePropertiesKHR(VkPhysicalDevice physicalDevice, VkDisplayKHR display, uint32_t* pPropertyCount,
                                                VkDisplayModePropertiesKHR* pProperties,
                                                const ErrorObject& error_obj) const override;
bool PreCallValidateCreateDisplayModeKHR(VkPhysicalDevice physicalDevice, VkDisplayKHR display,
                                         const VkDisplayModeCreateInfoKHR* pCreateInfo, const VkAllocationCallbacks* pAllocator,
                                         VkDisplayModeKHR* pMode, const ErrorObject& error_obj) const override;
bool PreCallValidateGetDisplayPlaneCapabilitiesKHR(VkPhysicalDevice physicalDevice, VkDisplayModeKHR mode, uint32_t planeIndex,
                                                   VkDisplayPlaneCapabilitiesKHR* pCapabilities,
                                                   const ErrorObject& error_obj) const override;
bool PreCallValidateCreateDisplayPlaneSurfaceKHR(VkInstance instance, const VkDisplaySurfaceCreateInfoKHR* pCreateInfo,
                                                 const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface,
                                                 const ErrorObject& error_obj) const override;
bool PreCallValidateCreateSharedSwapchainsKHR(VkDevice device, uint32_t swapchainCount,
                                              const VkSwapchainCreateInfoKHR* pCreateInfos, const VkAllocationCallbacks* pAllocator,
                                              VkSwapchainKHR* pSwapchains, const ErrorObject& error_obj) const override;
#ifdef VK_USE_PLATFORM_XLIB_KHR
bool PreCallValidateCreateXlibSurfaceKHR(VkInstance instance, const VkXlibSurfaceCreateInfoKHR* pCreateInfo,
                                         const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface,
                                         const ErrorObject& error_obj) const override;
#endif  // VK_USE_PLATFORM_XLIB_KHR
#ifdef VK_USE_PLATFORM_XLIB_KHR
bool PreCallValidateGetPhysicalDeviceXlibPresentationSupportKHR(VkPhysicalDevice physicalDevice, uint32_t queueFamilyIndex,
                                                                Display* dpy, VisualID visualID,
                                                                const ErrorObject& error_obj) const override;
#endif  // VK_USE_PLATFORM_XLIB_KHR
#ifdef VK_USE_PLATFORM_XCB_KHR
bool PreCallValidateCreateXcbSurfaceKHR(VkInstance instance, const VkXcbSurfaceCreateInfoKHR* pCreateInfo,
                                        const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface,
                                        const ErrorObject& error_obj) const override;
#endif  // VK_USE_PLATFORM_XCB_KHR
#ifdef VK_USE_PLATFORM_XCB_KHR
bool PreCallValidateGetPhysicalDeviceXcbPresentationSupportKHR(VkPhysicalDevice physicalDevice, uint32_t queueFamilyIndex,
                                                               xcb_connection_t* connection, xcb_visualid_t visual_id,
                                                               const ErrorObject& error_obj) const override;
#endif  // VK_USE_PLATFORM_XCB_KHR
#ifdef VK_USE_PLATFORM_WAYLAND_KHR
bool PreCallValidateCreateWaylandSurfaceKHR(VkInstance instance, const VkWaylandSurfaceCreateInfoKHR* pCreateInfo,
                                            const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface,
                                            const ErrorObject& error_obj) const override;
#endif  // VK_USE_PLATFORM_WAYLAND_KHR
#ifdef VK_USE_PLATFORM_WAYLAND_KHR
bool PreCallValidateGetPhysicalDeviceWaylandPresentationSupportKHR(VkPhysicalDevice physicalDevice, uint32_t queueFamilyIndex,
                                                                   struct wl_display* display,
                                                                   const ErrorObject& error_obj) const override;
#endif  // VK_USE_PLATFORM_WAYLAND_KHR
#ifdef VK_USE_PLATFORM_ANDROID_KHR
bool PreCallValidateCreateAndroidSurfaceKHR(VkInstance instance, const VkAndroidSurfaceCreateInfoKHR* pCreateInfo,
                                            const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface,
                                            const ErrorObject& error_obj) const override;
#endif  // VK_USE_PLATFORM_ANDROID_KHR
#ifdef VK_USE_PLATFORM_WIN32_KHR
bool PreCallValidateCreateWin32SurfaceKHR(VkInstance instance, const VkWin32SurfaceCreateInfoKHR* pCreateInfo,
                                          const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface,
                                          const ErrorObject& error_obj) const override;
#endif  // VK_USE_PLATFORM_WIN32_KHR
#ifdef VK_USE_PLATFORM_WIN32_KHR
bool PreCallValidateGetPhysicalDeviceWin32PresentationSupportKHR(VkPhysicalDevice physicalDevice, uint32_t queueFamilyIndex,
                                                                 const ErrorObject& error_obj) const override;
#endif  // VK_USE_PLATFORM_WIN32_KHR
bool PreCallValidateGetPhysicalDeviceVideoCapabilitiesKHR(VkPhysicalDevice physicalDevice,
                                                          const VkVideoProfileInfoKHR* pVideoProfile,
                                                          VkVideoCapabilitiesKHR* pCapabilities,
                                                          const ErrorObject& error_obj) const override;
bool PreCallValidateGetPhysicalDeviceVideoFormatPropertiesKHR(VkPhysicalDevice physicalDevice,
                                                              const VkPhysicalDeviceVideoFormatInfoKHR* pVideoFormatInfo,
                                                              uint32_t* pVideoFormatPropertyCount,
                                                              VkVideoFormatPropertiesKHR* pVideoFormatProperties,
                                                              const ErrorObject& error_obj) const override;
bool PreCallValidateCreateVideoSessionKHR(VkDevice device, const VkVideoSessionCreateInfoKHR* pCreateInfo,
                                          const VkAllocationCallbacks* pAllocator, VkVideoSessionKHR* pVideoSession,
                                          const ErrorObject& error_obj) const override;
bool PreCallValidateDestroyVideoSessionKHR(VkDevice device, VkVideoSessionKHR videoSession, const VkAllocationCallbacks* pAllocator,
                                           const ErrorObject& error_obj) const override;
bool PreCallValidateGetVideoSessionMemoryRequirementsKHR(VkDevice device, VkVideoSessionKHR videoSession,
                                                         uint32_t* pMemoryRequirementsCount,
                                                         VkVideoSessionMemoryRequirementsKHR* pMemoryRequirements,
                                                         const ErrorObject& error_obj) const override;
bool PreCallValidateBindVideoSessionMemoryKHR(VkDevice device, VkVideoSessionKHR videoSession, uint32_t bindSessionMemoryInfoCount,
                                              const VkBindVideoSessionMemoryInfoKHR* pBindSessionMemoryInfos,
                                              const ErrorObject& error_obj) const override;
bool PreCallValidateCreateVideoSessionParametersKHR(VkDevice device, const VkVideoSessionParametersCreateInfoKHR* pCreateInfo,
                                                    const VkAllocationCallbacks* pAllocator,
                                                    VkVideoSessionParametersKHR* pVideoSessionParameters,
                                                    const ErrorObject& error_obj) const override;
bool PreCallValidateUpdateVideoSessionParametersKHR(VkDevice device, VkVideoSessionParametersKHR videoSessionParameters,
                                                    const VkVideoSessionParametersUpdateInfoKHR* pUpdateInfo,
                                                    const ErrorObject& error_obj) const override;
bool PreCallValidateDestroyVideoSessionParametersKHR(VkDevice device, VkVideoSessionParametersKHR videoSessionParameters,
                                                     const VkAllocationCallbacks* pAllocator,
                                                     const ErrorObject& error_obj) const override;
bool PreCallValidateCmdBeginVideoCodingKHR(VkCommandBuffer commandBuffer, const VkVideoBeginCodingInfoKHR* pBeginInfo,
                                           const ErrorObject& error_obj) const override;
bool PreCallValidateCmdEndVideoCodingKHR(VkCommandBuffer commandBuffer, const VkVideoEndCodingInfoKHR* pEndCodingInfo,
                                         const ErrorObject& error_obj) const override;
bool PreCallValidateCmdControlVideoCodingKHR(VkCommandBuffer commandBuffer, const VkVideoCodingControlInfoKHR* pCodingControlInfo,
                                             const ErrorObject& error_obj) const override;
bool PreCallValidateCmdDecodeVideoKHR(VkCommandBuffer commandBuffer, const VkVideoDecodeInfoKHR* pDecodeInfo,
                                      const ErrorObject& error_obj) const override;
bool PreCallValidateCmdBeginRenderingKHR(VkCommandBuffer commandBuffer, const VkRenderingInfo* pRenderingInfo,
                                         const ErrorObject& error_obj) const override;
bool PreCallValidateCmdEndRenderingKHR(VkCommandBuffer commandBuffer, const ErrorObject& error_obj) const override;
bool PreCallValidateGetPhysicalDeviceFeatures2KHR(VkPhysicalDevice physicalDevice, VkPhysicalDeviceFeatures2* pFeatures,
                                                  const ErrorObject& error_obj) const override;
bool PreCallValidateGetPhysicalDeviceProperties2KHR(VkPhysicalDevice physicalDevice, VkPhysicalDeviceProperties2* pProperties,
                                                    const ErrorObject& error_obj) const override;
bool PreCallValidateGetPhysicalDeviceFormatProperties2KHR(VkPhysicalDevice physicalDevice, VkFormat format,
                                                          VkFormatProperties2* pFormatProperties,
                                                          const ErrorObject& error_obj) const override;
bool PreCallValidateGetPhysicalDeviceImageFormatProperties2KHR(VkPhysicalDevice physicalDevice,
                                                               const VkPhysicalDeviceImageFormatInfo2* pImageFormatInfo,
                                                               VkImageFormatProperties2* pImageFormatProperties,
                                                               const ErrorObject& error_obj) const override;
bool PreCallValidateGetPhysicalDeviceQueueFamilyProperties2KHR(VkPhysicalDevice physicalDevice, uint32_t* pQueueFamilyPropertyCount,
                                                               VkQueueFamilyProperties2* pQueueFamilyProperties,
                                                               const ErrorObject& error_obj) const override;
bool PreCallValidateGetPhysicalDeviceMemoryProperties2KHR(VkPhysicalDevice physicalDevice,
                                                          VkPhysicalDeviceMemoryProperties2* pMemoryProperties,
                                                          const ErrorObject& error_obj) const override;
bool PreCallValidateGetPhysicalDeviceSparseImageFormatProperties2KHR(VkPhysicalDevice physicalDevice,
                                                                     const VkPhysicalDeviceSparseImageFormatInfo2* pFormatInfo,
                                                                     uint32_t* pPropertyCount,
                                                                     VkSparseImageFormatProperties2* pProperties,
                                                                     const ErrorObject& error_obj) const override;
bool PreCallValidateGetDeviceGroupPeerMemoryFeaturesKHR(VkDevice device, uint32_t heapIndex, uint32_t localDeviceIndex,
                                                        uint32_t remoteDeviceIndex, VkPeerMemoryFeatureFlags* pPeerMemoryFeatures,
                                                        const ErrorObject& error_obj) const override;
bool PreCallValidateCmdSetDeviceMaskKHR(VkCommandBuffer commandBuffer, uint32_t deviceMask,
                                        const ErrorObject& error_obj) const override;
bool PreCallValidateCmdDispatchBaseKHR(VkCommandBuffer commandBuffer, uint32_t baseGroupX, uint32_t baseGroupY, uint32_t baseGroupZ,
                                       uint32_t groupCountX, uint32_t groupCountY, uint32_t groupCountZ,
                                       const ErrorObject& error_obj) const override;
bool PreCallValidateTrimCommandPoolKHR(VkDevice device, VkCommandPool commandPool, VkCommandPoolTrimFlags flags,
                                       const ErrorObject& error_obj) const override;
bool PreCallValidateEnumeratePhysicalDeviceGroupsKHR(VkInstance instance, uint32_t* pPhysicalDeviceGroupCount,
                                                     VkPhysicalDeviceGroupProperties* pPhysicalDeviceGroupProperties,
                                                     const ErrorObject& error_obj) const override;
bool PreCallValidateGetPhysicalDeviceExternalBufferPropertiesKHR(VkPhysicalDevice physicalDevice,
                                                                 const VkPhysicalDeviceExternalBufferInfo* pExternalBufferInfo,
                                                                 VkExternalBufferProperties* pExternalBufferProperties,
                                                                 const ErrorObject& error_obj) const override;
#ifdef VK_USE_PLATFORM_WIN32_KHR
bool PreCallValidateGetMemoryWin32HandleKHR(VkDevice device, const VkMemoryGetWin32HandleInfoKHR* pGetWin32HandleInfo,
                                            HANDLE* pHandle, const ErrorObject& error_obj) const override;
#endif  // VK_USE_PLATFORM_WIN32_KHR
#ifdef VK_USE_PLATFORM_WIN32_KHR
bool PreCallValidateGetMemoryWin32HandlePropertiesKHR(VkDevice device, VkExternalMemoryHandleTypeFlagBits handleType, HANDLE handle,
                                                      VkMemoryWin32HandlePropertiesKHR* pMemoryWin32HandleProperties,
                                                      const ErrorObject& error_obj) const override;
#endif  // VK_USE_PLATFORM_WIN32_KHR
bool PreCallValidateGetMemoryFdKHR(VkDevice device, const VkMemoryGetFdInfoKHR* pGetFdInfo, int* pFd,
                                   const ErrorObject& error_obj) const override;
bool PreCallValidateGetMemoryFdPropertiesKHR(VkDevice device, VkExternalMemoryHandleTypeFlagBits handleType, int fd,
                                             VkMemoryFdPropertiesKHR* pMemoryFdProperties,
                                             const ErrorObject& error_obj) const override;
bool PreCallValidateGetPhysicalDeviceExternalSemaphorePropertiesKHR(
    VkPhysicalDevice physicalDevice, const VkPhysicalDeviceExternalSemaphoreInfo* pExternalSemaphoreInfo,
    VkExternalSemaphoreProperties* pExternalSemaphoreProperties, const ErrorObject& error_obj) const override;
#ifdef VK_USE_PLATFORM_WIN32_KHR
bool PreCallValidateImportSemaphoreWin32HandleKHR(VkDevice device,
                                                  const VkImportSemaphoreWin32HandleInfoKHR* pImportSemaphoreWin32HandleInfo,
                                                  const ErrorObject& error_obj) const override;
#endif  // VK_USE_PLATFORM_WIN32_KHR
#ifdef VK_USE_PLATFORM_WIN32_KHR
bool PreCallValidateGetSemaphoreWin32HandleKHR(VkDevice device, const VkSemaphoreGetWin32HandleInfoKHR* pGetWin32HandleInfo,
                                               HANDLE* pHandle, const ErrorObject& error_obj) const override;
#endif  // VK_USE_PLATFORM_WIN32_KHR
bool PreCallValidateImportSemaphoreFdKHR(VkDevice device, const VkImportSemaphoreFdInfoKHR* pImportSemaphoreFdInfo,
                                         const ErrorObject& error_obj) const override;
bool PreCallValidateGetSemaphoreFdKHR(VkDevice device, const VkSemaphoreGetFdInfoKHR* pGetFdInfo, int* pFd,
                                      const ErrorObject& error_obj) const override;
bool PreCallValidateCmdPushDescriptorSetKHR(VkCommandBuffer commandBuffer, VkPipelineBindPoint pipelineBindPoint,
                                            VkPipelineLayout layout, uint32_t set, uint32_t descriptorWriteCount,
                                            const VkWriteDescriptorSet* pDescriptorWrites,
                                            const ErrorObject& error_obj) const override;
bool PreCallValidateCmdPushDescriptorSetWithTemplateKHR(VkCommandBuffer commandBuffer,
                                                        VkDescriptorUpdateTemplate descriptorUpdateTemplate,
                                                        VkPipelineLayout layout, uint32_t set, const void* pData,
                                                        const ErrorObject& error_obj) const override;
bool PreCallValidateCreateDescriptorUpdateTemplateKHR(VkDevice device, const VkDescriptorUpdateTemplateCreateInfo* pCreateInfo,
                                                      const VkAllocationCallbacks* pAllocator,
                                                      VkDescriptorUpdateTemplate* pDescriptorUpdateTemplate,
                                                      const ErrorObject& error_obj) const override;
bool PreCallValidateDestroyDescriptorUpdateTemplateKHR(VkDevice device, VkDescriptorUpdateTemplate descriptorUpdateTemplate,
                                                       const VkAllocationCallbacks* pAllocator,
                                                       const ErrorObject& error_obj) const override;
bool PreCallValidateUpdateDescriptorSetWithTemplateKHR(VkDevice device, VkDescriptorSet descriptorSet,
                                                       VkDescriptorUpdateTemplate descriptorUpdateTemplate, const void* pData,
                                                       const ErrorObject& error_obj) const override;
bool PreCallValidateCreateRenderPass2KHR(VkDevice device, const VkRenderPassCreateInfo2* pCreateInfo,
                                         const VkAllocationCallbacks* pAllocator, VkRenderPass* pRenderPass,
                                         const ErrorObject& error_obj) const override;
bool PreCallValidateCmdBeginRenderPass2KHR(VkCommandBuffer commandBuffer, const VkRenderPassBeginInfo* pRenderPassBegin,
                                           const VkSubpassBeginInfo* pSubpassBeginInfo,
                                           const ErrorObject& error_obj) const override;
bool PreCallValidateCmdNextSubpass2KHR(VkCommandBuffer commandBuffer, const VkSubpassBeginInfo* pSubpassBeginInfo,
                                       const VkSubpassEndInfo* pSubpassEndInfo, const ErrorObject& error_obj) const override;
bool PreCallValidateCmdEndRenderPass2KHR(VkCommandBuffer commandBuffer, const VkSubpassEndInfo* pSubpassEndInfo,
                                         const ErrorObject& error_obj) const override;
bool PreCallValidateGetSwapchainStatusKHR(VkDevice device, VkSwapchainKHR swapchain, const ErrorObject& error_obj) const override;
bool PreCallValidateGetPhysicalDeviceExternalFencePropertiesKHR(VkPhysicalDevice physicalDevice,
                                                                const VkPhysicalDeviceExternalFenceInfo* pExternalFenceInfo,
                                                                VkExternalFenceProperties* pExternalFenceProperties,
                                                                const ErrorObject& error_obj) const override;
#ifdef VK_USE_PLATFORM_WIN32_KHR
bool PreCallValidateImportFenceWin32HandleKHR(VkDevice device, const VkImportFenceWin32HandleInfoKHR* pImportFenceWin32HandleInfo,
                                              const ErrorObject& error_obj) const override;
#endif  // VK_USE_PLATFORM_WIN32_KHR
#ifdef VK_USE_PLATFORM_WIN32_KHR
bool PreCallValidateGetFenceWin32HandleKHR(VkDevice device, const VkFenceGetWin32HandleInfoKHR* pGetWin32HandleInfo,
                                           HANDLE* pHandle, const ErrorObject& error_obj) const override;
#endif  // VK_USE_PLATFORM_WIN32_KHR
bool PreCallValidateImportFenceFdKHR(VkDevice device, const VkImportFenceFdInfoKHR* pImportFenceFdInfo,
                                     const ErrorObject& error_obj) const override;
bool PreCallValidateGetFenceFdKHR(VkDevice device, const VkFenceGetFdInfoKHR* pGetFdInfo, int* pFd,
                                  const ErrorObject& error_obj) const override;
bool PreCallValidateEnumeratePhysicalDeviceQueueFamilyPerformanceQueryCountersKHR(
    VkPhysicalDevice physicalDevice, uint32_t queueFamilyIndex, uint32_t* pCounterCount, VkPerformanceCounterKHR* pCounters,
    VkPerformanceCounterDescriptionKHR* pCounterDescriptions, const ErrorObject& error_obj) const override;
bool PreCallValidateGetPhysicalDeviceQueueFamilyPerformanceQueryPassesKHR(
    VkPhysicalDevice physicalDevice, const VkQueryPoolPerformanceCreateInfoKHR* pPerformanceQueryCreateInfo, uint32_t* pNumPasses,
    const ErrorObject& error_obj) const override;
bool PreCallValidateAcquireProfilingLockKHR(VkDevice device, const VkAcquireProfilingLockInfoKHR* pInfo,
                                            const ErrorObject& error_obj) const override;
bool PreCallValidateReleaseProfilingLockKHR(VkDevice device, const ErrorObject& error_obj) const override;
bool PreCallValidateGetPhysicalDeviceSurfaceCapabilities2KHR(VkPhysicalDevice physicalDevice,
                                                             const VkPhysicalDeviceSurfaceInfo2KHR* pSurfaceInfo,
                                                             VkSurfaceCapabilities2KHR* pSurfaceCapabilities,
                                                             const ErrorObject& error_obj) const override;
bool PreCallValidateGetPhysicalDeviceSurfaceFormats2KHR(VkPhysicalDevice physicalDevice,
                                                        const VkPhysicalDeviceSurfaceInfo2KHR* pSurfaceInfo,
                                                        uint32_t* pSurfaceFormatCount, VkSurfaceFormat2KHR* pSurfaceFormats,
                                                        const ErrorObject& error_obj) const override;
bool PreCallValidateGetPhysicalDeviceDisplayProperties2KHR(VkPhysicalDevice physicalDevice, uint32_t* pPropertyCount,
                                                           VkDisplayProperties2KHR* pProperties,
                                                           const ErrorObject& error_obj) const override;
bool PreCallValidateGetPhysicalDeviceDisplayPlaneProperties2KHR(VkPhysicalDevice physicalDevice, uint32_t* pPropertyCount,
                                                                VkDisplayPlaneProperties2KHR* pProperties,
                                                                const ErrorObject& error_obj) const override;
bool PreCallValidateGetDisplayModeProperties2KHR(VkPhysicalDevice physicalDevice, VkDisplayKHR display, uint32_t* pPropertyCount,
                                                 VkDisplayModeProperties2KHR* pProperties,
                                                 const ErrorObject& error_obj) const override;
bool PreCallValidateGetDisplayPlaneCapabilities2KHR(VkPhysicalDevice physicalDevice,
                                                    const VkDisplayPlaneInfo2KHR* pDisplayPlaneInfo,
                                                    VkDisplayPlaneCapabilities2KHR* pCapabilities,
                                                    const ErrorObject& error_obj) const override;
bool PreCallValidateGetImageMemoryRequirements2KHR(VkDevice device, const VkImageMemoryRequirementsInfo2* pInfo,
                                                   VkMemoryRequirements2* pMemoryRequirements,
                                                   const ErrorObject& error_obj) const override;
bool PreCallValidateGetBufferMemoryRequirements2KHR(VkDevice device, const VkBufferMemoryRequirementsInfo2* pInfo,
                                                    VkMemoryRequirements2* pMemoryRequirements,
                                                    const ErrorObject& error_obj) const override;
bool PreCallValidateGetImageSparseMemoryRequirements2KHR(VkDevice device, const VkImageSparseMemoryRequirementsInfo2* pInfo,
                                                         uint32_t* pSparseMemoryRequirementCount,
                                                         VkSparseImageMemoryRequirements2* pSparseMemoryRequirements,
                                                         const ErrorObject& error_obj) const override;
bool PreCallValidateCreateSamplerYcbcrConversionKHR(VkDevice device, const VkSamplerYcbcrConversionCreateInfo* pCreateInfo,
                                                    const VkAllocationCallbacks* pAllocator,
                                                    VkSamplerYcbcrConversion* pYcbcrConversion,
                                                    const ErrorObject& error_obj) const override;
bool PreCallValidateDestroySamplerYcbcrConversionKHR(VkDevice device, VkSamplerYcbcrConversion ycbcrConversion,
                                                     const VkAllocationCallbacks* pAllocator,
                                                     const ErrorObject& error_obj) const override;
bool PreCallValidateBindBufferMemory2KHR(VkDevice device, uint32_t bindInfoCount, const VkBindBufferMemoryInfo* pBindInfos,
                                         const ErrorObject& error_obj) const override;
bool PreCallValidateBindImageMemory2KHR(VkDevice device, uint32_t bindInfoCount, const VkBindImageMemoryInfo* pBindInfos,
                                        const ErrorObject& error_obj) const override;
bool PreCallValidateGetDescriptorSetLayoutSupportKHR(VkDevice device, const VkDescriptorSetLayoutCreateInfo* pCreateInfo,
                                                     VkDescriptorSetLayoutSupport* pSupport,
                                                     const ErrorObject& error_obj) const override;
bool PreCallValidateCmdDrawIndirectCountKHR(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                            VkBuffer countBuffer, VkDeviceSize countBufferOffset, uint32_t maxDrawCount,
                                            uint32_t stride, const ErrorObject& error_obj) const override;
bool PreCallValidateCmdDrawIndexedIndirectCountKHR(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                   VkBuffer countBuffer, VkDeviceSize countBufferOffset, uint32_t maxDrawCount,
                                                   uint32_t stride, const ErrorObject& error_obj) const override;
bool PreCallValidateGetSemaphoreCounterValueKHR(VkDevice device, VkSemaphore semaphore, uint64_t* pValue,
                                                const ErrorObject& error_obj) const override;
bool PreCallValidateWaitSemaphoresKHR(VkDevice device, const VkSemaphoreWaitInfo* pWaitInfo, uint64_t timeout,
                                      const ErrorObject& error_obj) const override;
bool PreCallValidateSignalSemaphoreKHR(VkDevice device, const VkSemaphoreSignalInfo* pSignalInfo,
                                       const ErrorObject& error_obj) const override;
bool PreCallValidateGetPhysicalDeviceFragmentShadingRatesKHR(VkPhysicalDevice physicalDevice, uint32_t* pFragmentShadingRateCount,
                                                             VkPhysicalDeviceFragmentShadingRateKHR* pFragmentShadingRates,
                                                             const ErrorObject& error_obj) const override;
bool PreCallValidateCmdSetFragmentShadingRateKHR(VkCommandBuffer commandBuffer, const VkExtent2D* pFragmentSize,
                                                 const VkFragmentShadingRateCombinerOpKHR combinerOps[2],
                                                 const ErrorObject& error_obj) const override;
bool PreCallValidateWaitForPresentKHR(VkDevice device, VkSwapchainKHR swapchain, uint64_t presentId, uint64_t timeout,
                                      const ErrorObject& error_obj) const override;
bool PreCallValidateGetBufferDeviceAddressKHR(VkDevice device, const VkBufferDeviceAddressInfo* pInfo,
                                              const ErrorObject& error_obj) const override;
bool PreCallValidateGetBufferOpaqueCaptureAddressKHR(VkDevice device, const VkBufferDeviceAddressInfo* pInfo,
                                                     const ErrorObject& error_obj) const override;
bool PreCallValidateGetDeviceMemoryOpaqueCaptureAddressKHR(VkDevice device, const VkDeviceMemoryOpaqueCaptureAddressInfo* pInfo,
                                                           const ErrorObject& error_obj) const override;
bool PreCallValidateCreateDeferredOperationKHR(VkDevice device, const VkAllocationCallbacks* pAllocator,
                                               VkDeferredOperationKHR* pDeferredOperation,
                                               const ErrorObject& error_obj) const override;
bool PreCallValidateDestroyDeferredOperationKHR(VkDevice device, VkDeferredOperationKHR operation,
                                                const VkAllocationCallbacks* pAllocator,
                                                const ErrorObject& error_obj) const override;
bool PreCallValidateGetDeferredOperationMaxConcurrencyKHR(VkDevice device, VkDeferredOperationKHR operation,
                                                          const ErrorObject& error_obj) const override;
bool PreCallValidateGetDeferredOperationResultKHR(VkDevice device, VkDeferredOperationKHR operation,
                                                  const ErrorObject& error_obj) const override;
bool PreCallValidateDeferredOperationJoinKHR(VkDevice device, VkDeferredOperationKHR operation,
                                             const ErrorObject& error_obj) const override;
bool PreCallValidateGetPipelineExecutablePropertiesKHR(VkDevice device, const VkPipelineInfoKHR* pPipelineInfo,
                                                       uint32_t* pExecutableCount, VkPipelineExecutablePropertiesKHR* pProperties,
                                                       const ErrorObject& error_obj) const override;
bool PreCallValidateGetPipelineExecutableStatisticsKHR(VkDevice device, const VkPipelineExecutableInfoKHR* pExecutableInfo,
                                                       uint32_t* pStatisticCount, VkPipelineExecutableStatisticKHR* pStatistics,
                                                       const ErrorObject& error_obj) const override;
bool PreCallValidateGetPipelineExecutableInternalRepresentationsKHR(
    VkDevice device, const VkPipelineExecutableInfoKHR* pExecutableInfo, uint32_t* pInternalRepresentationCount,
    VkPipelineExecutableInternalRepresentationKHR* pInternalRepresentations, const ErrorObject& error_obj) const override;
bool PreCallValidateMapMemory2KHR(VkDevice device, const VkMemoryMapInfoKHR* pMemoryMapInfo, void** ppData,
                                  const ErrorObject& error_obj) const override;
bool PreCallValidateUnmapMemory2KHR(VkDevice device, const VkMemoryUnmapInfoKHR* pMemoryUnmapInfo,
                                    const ErrorObject& error_obj) const override;
#ifdef VK_ENABLE_BETA_EXTENSIONS
bool PreCallValidateGetPhysicalDeviceVideoEncodeQualityLevelPropertiesKHR(
    VkPhysicalDevice physicalDevice, const VkPhysicalDeviceVideoEncodeQualityLevelInfoKHR* pQualityLevelInfo,
    VkVideoEncodeQualityLevelPropertiesKHR* pQualityLevelProperties, const ErrorObject& error_obj) const override;
#endif  // VK_ENABLE_BETA_EXTENSIONS
#ifdef VK_ENABLE_BETA_EXTENSIONS
bool PreCallValidateGetEncodedVideoSessionParametersKHR(VkDevice device,
                                                        const VkVideoEncodeSessionParametersGetInfoKHR* pVideoSessionParametersInfo,
                                                        VkVideoEncodeSessionParametersFeedbackInfoKHR* pFeedbackInfo,
                                                        size_t* pDataSize, void* pData,
                                                        const ErrorObject& error_obj) const override;
#endif  // VK_ENABLE_BETA_EXTENSIONS
#ifdef VK_ENABLE_BETA_EXTENSIONS
bool PreCallValidateCmdEncodeVideoKHR(VkCommandBuffer commandBuffer, const VkVideoEncodeInfoKHR* pEncodeInfo,
                                      const ErrorObject& error_obj) const override;
#endif  // VK_ENABLE_BETA_EXTENSIONS
bool PreCallValidateCmdSetEvent2KHR(VkCommandBuffer commandBuffer, VkEvent event, const VkDependencyInfo* pDependencyInfo,
                                    const ErrorObject& error_obj) const override;
bool PreCallValidateCmdResetEvent2KHR(VkCommandBuffer commandBuffer, VkEvent event, VkPipelineStageFlags2 stageMask,
                                      const ErrorObject& error_obj) const override;
bool PreCallValidateCmdWaitEvents2KHR(VkCommandBuffer commandBuffer, uint32_t eventCount, const VkEvent* pEvents,
                                      const VkDependencyInfo* pDependencyInfos, const ErrorObject& error_obj) const override;
bool PreCallValidateCmdPipelineBarrier2KHR(VkCommandBuffer commandBuffer, const VkDependencyInfo* pDependencyInfo,
                                           const ErrorObject& error_obj) const override;
bool PreCallValidateCmdWriteTimestamp2KHR(VkCommandBuffer commandBuffer, VkPipelineStageFlags2 stage, VkQueryPool queryPool,
                                          uint32_t query, const ErrorObject& error_obj) const override;
bool PreCallValidateQueueSubmit2KHR(VkQueue queue, uint32_t submitCount, const VkSubmitInfo2* pSubmits, VkFence fence,
                                    const ErrorObject& error_obj) const override;
bool PreCallValidateCmdWriteBufferMarker2AMD(VkCommandBuffer commandBuffer, VkPipelineStageFlags2 stage, VkBuffer dstBuffer,
                                             VkDeviceSize dstOffset, uint32_t marker, const ErrorObject& error_obj) const override;
bool PreCallValidateGetQueueCheckpointData2NV(VkQueue queue, uint32_t* pCheckpointDataCount, VkCheckpointData2NV* pCheckpointData,
                                              const ErrorObject& error_obj) const override;
bool PreCallValidateCmdCopyBuffer2KHR(VkCommandBuffer commandBuffer, const VkCopyBufferInfo2* pCopyBufferInfo,
                                      const ErrorObject& error_obj) const override;
bool PreCallValidateCmdCopyImage2KHR(VkCommandBuffer commandBuffer, const VkCopyImageInfo2* pCopyImageInfo,
                                     const ErrorObject& error_obj) const override;
bool PreCallValidateCmdCopyBufferToImage2KHR(VkCommandBuffer commandBuffer, const VkCopyBufferToImageInfo2* pCopyBufferToImageInfo,
                                             const ErrorObject& error_obj) const override;
bool PreCallValidateCmdCopyImageToBuffer2KHR(VkCommandBuffer commandBuffer, const VkCopyImageToBufferInfo2* pCopyImageToBufferInfo,
                                             const ErrorObject& error_obj) const override;
bool PreCallValidateCmdBlitImage2KHR(VkCommandBuffer commandBuffer, const VkBlitImageInfo2* pBlitImageInfo,
                                     const ErrorObject& error_obj) const override;
bool PreCallValidateCmdResolveImage2KHR(VkCommandBuffer commandBuffer, const VkResolveImageInfo2* pResolveImageInfo,
                                        const ErrorObject& error_obj) const override;
bool PreCallValidateCmdTraceRaysIndirect2KHR(VkCommandBuffer commandBuffer, VkDeviceAddress indirectDeviceAddress,
                                             const ErrorObject& error_obj) const override;
bool PreCallValidateGetDeviceBufferMemoryRequirementsKHR(VkDevice device, const VkDeviceBufferMemoryRequirements* pInfo,
                                                         VkMemoryRequirements2* pMemoryRequirements,
                                                         const ErrorObject& error_obj) const override;
bool PreCallValidateGetDeviceImageMemoryRequirementsKHR(VkDevice device, const VkDeviceImageMemoryRequirements* pInfo,
                                                        VkMemoryRequirements2* pMemoryRequirements,
                                                        const ErrorObject& error_obj) const override;
bool PreCallValidateGetDeviceImageSparseMemoryRequirementsKHR(VkDevice device, const VkDeviceImageMemoryRequirements* pInfo,
                                                              uint32_t* pSparseMemoryRequirementCount,
                                                              VkSparseImageMemoryRequirements2* pSparseMemoryRequirements,
                                                              const ErrorObject& error_obj) const override;
bool PreCallValidateCmdBindIndexBuffer2KHR(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset, VkDeviceSize size,
                                           VkIndexType indexType, const ErrorObject& error_obj) const override;
bool PreCallValidateGetRenderingAreaGranularityKHR(VkDevice device, const VkRenderingAreaInfoKHR* pRenderingAreaInfo,
                                                   VkExtent2D* pGranularity, const ErrorObject& error_obj) const override;
bool PreCallValidateGetDeviceImageSubresourceLayoutKHR(VkDevice device, const VkDeviceImageSubresourceInfoKHR* pInfo,
                                                       VkSubresourceLayout2KHR* pLayout,
                                                       const ErrorObject& error_obj) const override;
bool PreCallValidateGetImageSubresourceLayout2KHR(VkDevice device, VkImage image, const VkImageSubresource2KHR* pSubresource,
                                                  VkSubresourceLayout2KHR* pLayout, const ErrorObject& error_obj) const override;
bool PreCallValidateGetPhysicalDeviceCooperativeMatrixPropertiesKHR(VkPhysicalDevice physicalDevice, uint32_t* pPropertyCount,
                                                                    VkCooperativeMatrixPropertiesKHR* pProperties,
                                                                    const ErrorObject& error_obj) const override;
bool PreCallValidateCreateDebugReportCallbackEXT(VkInstance instance, const VkDebugReportCallbackCreateInfoEXT* pCreateInfo,
                                                 const VkAllocationCallbacks* pAllocator, VkDebugReportCallbackEXT* pCallback,
                                                 const ErrorObject& error_obj) const override;
bool PreCallValidateDestroyDebugReportCallbackEXT(VkInstance instance, VkDebugReportCallbackEXT callback,
                                                  const VkAllocationCallbacks* pAllocator,
                                                  const ErrorObject& error_obj) const override;
bool PreCallValidateDebugReportMessageEXT(VkInstance instance, VkDebugReportFlagsEXT flags, VkDebugReportObjectTypeEXT objectType,
                                          uint64_t object, size_t location, int32_t messageCode, const char* pLayerPrefix,
                                          const char* pMessage, const ErrorObject& error_obj) const override;
bool PreCallValidateDebugMarkerSetObjectTagEXT(VkDevice device, const VkDebugMarkerObjectTagInfoEXT* pTagInfo,
                                               const ErrorObject& error_obj) const override;
bool PreCallValidateDebugMarkerSetObjectNameEXT(VkDevice device, const VkDebugMarkerObjectNameInfoEXT* pNameInfo,
                                                const ErrorObject& error_obj) const override;
bool PreCallValidateCmdDebugMarkerBeginEXT(VkCommandBuffer commandBuffer, const VkDebugMarkerMarkerInfoEXT* pMarkerInfo,
                                           const ErrorObject& error_obj) const override;
bool PreCallValidateCmdDebugMarkerEndEXT(VkCommandBuffer commandBuffer, const ErrorObject& error_obj) const override;
bool PreCallValidateCmdDebugMarkerInsertEXT(VkCommandBuffer commandBuffer, const VkDebugMarkerMarkerInfoEXT* pMarkerInfo,
                                            const ErrorObject& error_obj) const override;
bool PreCallValidateCmdBindTransformFeedbackBuffersEXT(VkCommandBuffer commandBuffer, uint32_t firstBinding, uint32_t bindingCount,
                                                       const VkBuffer* pBuffers, const VkDeviceSize* pOffsets,
                                                       const VkDeviceSize* pSizes, const ErrorObject& error_obj) const override;
bool PreCallValidateCmdBeginTransformFeedbackEXT(VkCommandBuffer commandBuffer, uint32_t firstCounterBuffer,
                                                 uint32_t counterBufferCount, const VkBuffer* pCounterBuffers,
                                                 const VkDeviceSize* pCounterBufferOffsets,
                                                 const ErrorObject& error_obj) const override;
bool PreCallValidateCmdEndTransformFeedbackEXT(VkCommandBuffer commandBuffer, uint32_t firstCounterBuffer,
                                               uint32_t counterBufferCount, const VkBuffer* pCounterBuffers,
                                               const VkDeviceSize* pCounterBufferOffsets,
                                               const ErrorObject& error_obj) const override;
bool PreCallValidateCmdBeginQueryIndexedEXT(VkCommandBuffer commandBuffer, VkQueryPool queryPool, uint32_t query,
                                            VkQueryControlFlags flags, uint32_t index, const ErrorObject& error_obj) const override;
bool PreCallValidateCmdEndQueryIndexedEXT(VkCommandBuffer commandBuffer, VkQueryPool queryPool, uint32_t query, uint32_t index,
                                          const ErrorObject& error_obj) const override;
bool PreCallValidateCmdDrawIndirectByteCountEXT(VkCommandBuffer commandBuffer, uint32_t instanceCount, uint32_t firstInstance,
                                                VkBuffer counterBuffer, VkDeviceSize counterBufferOffset, uint32_t counterOffset,
                                                uint32_t vertexStride, const ErrorObject& error_obj) const override;
bool PreCallValidateCreateCuModuleNVX(VkDevice device, const VkCuModuleCreateInfoNVX* pCreateInfo,
                                      const VkAllocationCallbacks* pAllocator, VkCuModuleNVX* pModule,
                                      const ErrorObject& error_obj) const override;
bool PreCallValidateCreateCuFunctionNVX(VkDevice device, const VkCuFunctionCreateInfoNVX* pCreateInfo,
                                        const VkAllocationCallbacks* pAllocator, VkCuFunctionNVX* pFunction,
                                        const ErrorObject& error_obj) const override;
bool PreCallValidateDestroyCuModuleNVX(VkDevice device, VkCuModuleNVX module, const VkAllocationCallbacks* pAllocator,
                                       const ErrorObject& error_obj) const override;
bool PreCallValidateDestroyCuFunctionNVX(VkDevice device, VkCuFunctionNVX function, const VkAllocationCallbacks* pAllocator,
                                         const ErrorObject& error_obj) const override;
bool PreCallValidateCmdCuLaunchKernelNVX(VkCommandBuffer commandBuffer, const VkCuLaunchInfoNVX* pLaunchInfo,
                                         const ErrorObject& error_obj) const override;
bool PreCallValidateGetImageViewHandleNVX(VkDevice device, const VkImageViewHandleInfoNVX* pInfo,
                                          const ErrorObject& error_obj) const override;
bool PreCallValidateGetImageViewAddressNVX(VkDevice device, VkImageView imageView, VkImageViewAddressPropertiesNVX* pProperties,
                                           const ErrorObject& error_obj) const override;
bool PreCallValidateCmdDrawIndirectCountAMD(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                            VkBuffer countBuffer, VkDeviceSize countBufferOffset, uint32_t maxDrawCount,
                                            uint32_t stride, const ErrorObject& error_obj) const override;
bool PreCallValidateCmdDrawIndexedIndirectCountAMD(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                   VkBuffer countBuffer, VkDeviceSize countBufferOffset, uint32_t maxDrawCount,
                                                   uint32_t stride, const ErrorObject& error_obj) const override;
bool PreCallValidateGetShaderInfoAMD(VkDevice device, VkPipeline pipeline, VkShaderStageFlagBits shaderStage,
                                     VkShaderInfoTypeAMD infoType, size_t* pInfoSize, void* pInfo,
                                     const ErrorObject& error_obj) const override;
#ifdef VK_USE_PLATFORM_GGP
bool PreCallValidateCreateStreamDescriptorSurfaceGGP(VkInstance instance, const VkStreamDescriptorSurfaceCreateInfoGGP* pCreateInfo,
                                                     const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface,
                                                     const ErrorObject& error_obj) const override;
#endif  // VK_USE_PLATFORM_GGP
bool PreCallValidateGetPhysicalDeviceExternalImageFormatPropertiesNV(
    VkPhysicalDevice physicalDevice, VkFormat format, VkImageType type, VkImageTiling tiling, VkImageUsageFlags usage,
    VkImageCreateFlags flags, VkExternalMemoryHandleTypeFlagsNV externalHandleType,
    VkExternalImageFormatPropertiesNV* pExternalImageFormatProperties, const ErrorObject& error_obj) const override;
#ifdef VK_USE_PLATFORM_WIN32_KHR
bool PreCallValidateGetMemoryWin32HandleNV(VkDevice device, VkDeviceMemory memory, VkExternalMemoryHandleTypeFlagsNV handleType,
                                           HANDLE* pHandle, const ErrorObject& error_obj) const override;
#endif  // VK_USE_PLATFORM_WIN32_KHR
#ifdef VK_USE_PLATFORM_VI_NN
bool PreCallValidateCreateViSurfaceNN(VkInstance instance, const VkViSurfaceCreateInfoNN* pCreateInfo,
                                      const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface,
                                      const ErrorObject& error_obj) const override;
#endif  // VK_USE_PLATFORM_VI_NN
bool PreCallValidateCmdBeginConditionalRenderingEXT(VkCommandBuffer commandBuffer,
                                                    const VkConditionalRenderingBeginInfoEXT* pConditionalRenderingBegin,
                                                    const ErrorObject& error_obj) const override;
bool PreCallValidateCmdEndConditionalRenderingEXT(VkCommandBuffer commandBuffer, const ErrorObject& error_obj) const override;
bool PreCallValidateCmdSetViewportWScalingNV(VkCommandBuffer commandBuffer, uint32_t firstViewport, uint32_t viewportCount,
                                             const VkViewportWScalingNV* pViewportWScalings,
                                             const ErrorObject& error_obj) const override;
bool PreCallValidateReleaseDisplayEXT(VkPhysicalDevice physicalDevice, VkDisplayKHR display,
                                      const ErrorObject& error_obj) const override;
#ifdef VK_USE_PLATFORM_XLIB_XRANDR_EXT
bool PreCallValidateAcquireXlibDisplayEXT(VkPhysicalDevice physicalDevice, Display* dpy, VkDisplayKHR display,
                                          const ErrorObject& error_obj) const override;
#endif  // VK_USE_PLATFORM_XLIB_XRANDR_EXT
#ifdef VK_USE_PLATFORM_XLIB_XRANDR_EXT
bool PreCallValidateGetRandROutputDisplayEXT(VkPhysicalDevice physicalDevice, Display* dpy, RROutput rrOutput,
                                             VkDisplayKHR* pDisplay, const ErrorObject& error_obj) const override;
#endif  // VK_USE_PLATFORM_XLIB_XRANDR_EXT
bool PreCallValidateGetPhysicalDeviceSurfaceCapabilities2EXT(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface,
                                                             VkSurfaceCapabilities2EXT* pSurfaceCapabilities,
                                                             const ErrorObject& error_obj) const override;
bool PreCallValidateDisplayPowerControlEXT(VkDevice device, VkDisplayKHR display, const VkDisplayPowerInfoEXT* pDisplayPowerInfo,
                                           const ErrorObject& error_obj) const override;
bool PreCallValidateRegisterDeviceEventEXT(VkDevice device, const VkDeviceEventInfoEXT* pDeviceEventInfo,
                                           const VkAllocationCallbacks* pAllocator, VkFence* pFence,
                                           const ErrorObject& error_obj) const override;
bool PreCallValidateRegisterDisplayEventEXT(VkDevice device, VkDisplayKHR display, const VkDisplayEventInfoEXT* pDisplayEventInfo,
                                            const VkAllocationCallbacks* pAllocator, VkFence* pFence,
                                            const ErrorObject& error_obj) const override;
bool PreCallValidateGetSwapchainCounterEXT(VkDevice device, VkSwapchainKHR swapchain, VkSurfaceCounterFlagBitsEXT counter,
                                           uint64_t* pCounterValue, const ErrorObject& error_obj) const override;
bool PreCallValidateGetRefreshCycleDurationGOOGLE(VkDevice device, VkSwapchainKHR swapchain,
                                                  VkRefreshCycleDurationGOOGLE* pDisplayTimingProperties,
                                                  const ErrorObject& error_obj) const override;
bool PreCallValidateGetPastPresentationTimingGOOGLE(VkDevice device, VkSwapchainKHR swapchain, uint32_t* pPresentationTimingCount,
                                                    VkPastPresentationTimingGOOGLE* pPresentationTimings,
                                                    const ErrorObject& error_obj) const override;
bool PreCallValidateCmdSetDiscardRectangleEXT(VkCommandBuffer commandBuffer, uint32_t firstDiscardRectangle,
                                              uint32_t discardRectangleCount, const VkRect2D* pDiscardRectangles,
                                              const ErrorObject& error_obj) const override;
bool PreCallValidateCmdSetDiscardRectangleEnableEXT(VkCommandBuffer commandBuffer, VkBool32 discardRectangleEnable,
                                                    const ErrorObject& error_obj) const override;
bool PreCallValidateCmdSetDiscardRectangleModeEXT(VkCommandBuffer commandBuffer, VkDiscardRectangleModeEXT discardRectangleMode,
                                                  const ErrorObject& error_obj) const override;
bool PreCallValidateSetHdrMetadataEXT(VkDevice device, uint32_t swapchainCount, const VkSwapchainKHR* pSwapchains,
                                      const VkHdrMetadataEXT* pMetadata, const ErrorObject& error_obj) const override;
#ifdef VK_USE_PLATFORM_IOS_MVK
bool PreCallValidateCreateIOSSurfaceMVK(VkInstance instance, const VkIOSSurfaceCreateInfoMVK* pCreateInfo,
                                        const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface,
                                        const ErrorObject& error_obj) const override;
#endif  // VK_USE_PLATFORM_IOS_MVK
#ifdef VK_USE_PLATFORM_MACOS_MVK
bool PreCallValidateCreateMacOSSurfaceMVK(VkInstance instance, const VkMacOSSurfaceCreateInfoMVK* pCreateInfo,
                                          const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface,
                                          const ErrorObject& error_obj) const override;
#endif  // VK_USE_PLATFORM_MACOS_MVK
bool PreCallValidateSetDebugUtilsObjectNameEXT(VkDevice device, const VkDebugUtilsObjectNameInfoEXT* pNameInfo,
                                               const ErrorObject& error_obj) const override;
bool PreCallValidateSetDebugUtilsObjectTagEXT(VkDevice device, const VkDebugUtilsObjectTagInfoEXT* pTagInfo,
                                              const ErrorObject& error_obj) const override;
bool PreCallValidateQueueBeginDebugUtilsLabelEXT(VkQueue queue, const VkDebugUtilsLabelEXT* pLabelInfo,
                                                 const ErrorObject& error_obj) const override;
bool PreCallValidateQueueEndDebugUtilsLabelEXT(VkQueue queue, const ErrorObject& error_obj) const override;
bool PreCallValidateQueueInsertDebugUtilsLabelEXT(VkQueue queue, const VkDebugUtilsLabelEXT* pLabelInfo,
                                                  const ErrorObject& error_obj) const override;
bool PreCallValidateCmdBeginDebugUtilsLabelEXT(VkCommandBuffer commandBuffer, const VkDebugUtilsLabelEXT* pLabelInfo,
                                               const ErrorObject& error_obj) const override;
bool PreCallValidateCmdEndDebugUtilsLabelEXT(VkCommandBuffer commandBuffer, const ErrorObject& error_obj) const override;
bool PreCallValidateCmdInsertDebugUtilsLabelEXT(VkCommandBuffer commandBuffer, const VkDebugUtilsLabelEXT* pLabelInfo,
                                                const ErrorObject& error_obj) const override;
bool PreCallValidateCreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
                                                 const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pMessenger,
                                                 const ErrorObject& error_obj) const override;
bool PreCallValidateDestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT messenger,
                                                  const VkAllocationCallbacks* pAllocator,
                                                  const ErrorObject& error_obj) const override;
bool PreCallValidateSubmitDebugUtilsMessageEXT(VkInstance instance, VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
                                               VkDebugUtilsMessageTypeFlagsEXT messageTypes,
                                               const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
                                               const ErrorObject& error_obj) const override;
#ifdef VK_USE_PLATFORM_ANDROID_KHR
bool PreCallValidateGetAndroidHardwareBufferPropertiesANDROID(VkDevice device, const struct AHardwareBuffer* buffer,
                                                              VkAndroidHardwareBufferPropertiesANDROID* pProperties,
                                                              const ErrorObject& error_obj) const override;
#endif  // VK_USE_PLATFORM_ANDROID_KHR
#ifdef VK_USE_PLATFORM_ANDROID_KHR
bool PreCallValidateGetMemoryAndroidHardwareBufferANDROID(VkDevice device, const VkMemoryGetAndroidHardwareBufferInfoANDROID* pInfo,
                                                          struct AHardwareBuffer** pBuffer,
                                                          const ErrorObject& error_obj) const override;
#endif  // VK_USE_PLATFORM_ANDROID_KHR
#ifdef VK_ENABLE_BETA_EXTENSIONS
bool PreCallValidateCreateExecutionGraphPipelinesAMDX(VkDevice device, VkPipelineCache pipelineCache, uint32_t createInfoCount,
                                                      const VkExecutionGraphPipelineCreateInfoAMDX* pCreateInfos,
                                                      const VkAllocationCallbacks* pAllocator, VkPipeline* pPipelines,
                                                      const ErrorObject& error_obj) const override;
#endif  // VK_ENABLE_BETA_EXTENSIONS
#ifdef VK_ENABLE_BETA_EXTENSIONS
bool PreCallValidateGetExecutionGraphPipelineScratchSizeAMDX(VkDevice device, VkPipeline executionGraph,
                                                             VkExecutionGraphPipelineScratchSizeAMDX* pSizeInfo,
                                                             const ErrorObject& error_obj) const override;
#endif  // VK_ENABLE_BETA_EXTENSIONS
#ifdef VK_ENABLE_BETA_EXTENSIONS
bool PreCallValidateGetExecutionGraphPipelineNodeIndexAMDX(VkDevice device, VkPipeline executionGraph,
                                                           const VkPipelineShaderStageNodeCreateInfoAMDX* pNodeInfo,
                                                           uint32_t* pNodeIndex, const ErrorObject& error_obj) const override;
#endif  // VK_ENABLE_BETA_EXTENSIONS
#ifdef VK_ENABLE_BETA_EXTENSIONS
bool PreCallValidateCmdInitializeGraphScratchMemoryAMDX(VkCommandBuffer commandBuffer, VkDeviceAddress scratch,
                                                        const ErrorObject& error_obj) const override;
#endif  // VK_ENABLE_BETA_EXTENSIONS
#ifdef VK_ENABLE_BETA_EXTENSIONS
bool PreCallValidateCmdDispatchGraphAMDX(VkCommandBuffer commandBuffer, VkDeviceAddress scratch,
                                         const VkDispatchGraphCountInfoAMDX* pCountInfo,
                                         const ErrorObject& error_obj) const override;
#endif  // VK_ENABLE_BETA_EXTENSIONS
#ifdef VK_ENABLE_BETA_EXTENSIONS
bool PreCallValidateCmdDispatchGraphIndirectAMDX(VkCommandBuffer commandBuffer, VkDeviceAddress scratch,
                                                 const VkDispatchGraphCountInfoAMDX* pCountInfo,
                                                 const ErrorObject& error_obj) const override;
#endif  // VK_ENABLE_BETA_EXTENSIONS
#ifdef VK_ENABLE_BETA_EXTENSIONS
bool PreCallValidateCmdDispatchGraphIndirectCountAMDX(VkCommandBuffer commandBuffer, VkDeviceAddress scratch,
                                                      VkDeviceAddress countInfo, const ErrorObject& error_obj) const override;
#endif  // VK_ENABLE_BETA_EXTENSIONS
bool PreCallValidateCmdSetSampleLocationsEXT(VkCommandBuffer commandBuffer, const VkSampleLocationsInfoEXT* pSampleLocationsInfo,
                                             const ErrorObject& error_obj) const override;
bool PreCallValidateGetPhysicalDeviceMultisamplePropertiesEXT(VkPhysicalDevice physicalDevice, VkSampleCountFlagBits samples,
                                                              VkMultisamplePropertiesEXT* pMultisampleProperties,
                                                              const ErrorObject& error_obj) const override;
bool PreCallValidateGetImageDrmFormatModifierPropertiesEXT(VkDevice device, VkImage image,
                                                           VkImageDrmFormatModifierPropertiesEXT* pProperties,
                                                           const ErrorObject& error_obj) const override;
bool PreCallValidateCreateValidationCacheEXT(VkDevice device, const VkValidationCacheCreateInfoEXT* pCreateInfo,
                                             const VkAllocationCallbacks* pAllocator, VkValidationCacheEXT* pValidationCache,
                                             const ErrorObject& error_obj) const;
bool PreCallValidateDestroyValidationCacheEXT(VkDevice device, VkValidationCacheEXT validationCache,
                                              const VkAllocationCallbacks* pAllocator, const ErrorObject& error_obj) const;
bool PreCallValidateMergeValidationCachesEXT(VkDevice device, VkValidationCacheEXT dstCache, uint32_t srcCacheCount,
                                             const VkValidationCacheEXT* pSrcCaches, const ErrorObject& error_obj) const;
bool PreCallValidateGetValidationCacheDataEXT(VkDevice device, VkValidationCacheEXT validationCache, size_t* pDataSize, void* pData,
                                              const ErrorObject& error_obj) const;
bool PreCallValidateCmdBindShadingRateImageNV(VkCommandBuffer commandBuffer, VkImageView imageView, VkImageLayout imageLayout,
                                              const ErrorObject& error_obj) const override;
bool PreCallValidateCmdSetViewportShadingRatePaletteNV(VkCommandBuffer commandBuffer, uint32_t firstViewport,
                                                       uint32_t viewportCount, const VkShadingRatePaletteNV* pShadingRatePalettes,
                                                       const ErrorObject& error_obj) const override;
bool PreCallValidateCmdSetCoarseSampleOrderNV(VkCommandBuffer commandBuffer, VkCoarseSampleOrderTypeNV sampleOrderType,
                                              uint32_t customSampleOrderCount,
                                              const VkCoarseSampleOrderCustomNV* pCustomSampleOrders,
                                              const ErrorObject& error_obj) const override;
bool PreCallValidateCreateAccelerationStructureNV(VkDevice device, const VkAccelerationStructureCreateInfoNV* pCreateInfo,
                                                  const VkAllocationCallbacks* pAllocator,
                                                  VkAccelerationStructureNV* pAccelerationStructure,
                                                  const ErrorObject& error_obj) const override;
bool PreCallValidateDestroyAccelerationStructureNV(VkDevice device, VkAccelerationStructureNV accelerationStructure,
                                                   const VkAllocationCallbacks* pAllocator,
                                                   const ErrorObject& error_obj) const override;
bool PreCallValidateGetAccelerationStructureMemoryRequirementsNV(VkDevice device,
                                                                 const VkAccelerationStructureMemoryRequirementsInfoNV* pInfo,
                                                                 VkMemoryRequirements2KHR* pMemoryRequirements,
                                                                 const ErrorObject& error_obj) const override;
bool PreCallValidateBindAccelerationStructureMemoryNV(VkDevice device, uint32_t bindInfoCount,
                                                      const VkBindAccelerationStructureMemoryInfoNV* pBindInfos,
                                                      const ErrorObject& error_obj) const override;
bool PreCallValidateCmdBuildAccelerationStructureNV(VkCommandBuffer commandBuffer, const VkAccelerationStructureInfoNV* pInfo,
                                                    VkBuffer instanceData, VkDeviceSize instanceOffset, VkBool32 update,
                                                    VkAccelerationStructureNV dst, VkAccelerationStructureNV src, VkBuffer scratch,
                                                    VkDeviceSize scratchOffset, const ErrorObject& error_obj) const override;
bool PreCallValidateCmdCopyAccelerationStructureNV(VkCommandBuffer commandBuffer, VkAccelerationStructureNV dst,
                                                   VkAccelerationStructureNV src, VkCopyAccelerationStructureModeKHR mode,
                                                   const ErrorObject& error_obj) const override;
bool PreCallValidateCmdTraceRaysNV(VkCommandBuffer commandBuffer, VkBuffer raygenShaderBindingTableBuffer,
                                   VkDeviceSize raygenShaderBindingOffset, VkBuffer missShaderBindingTableBuffer,
                                   VkDeviceSize missShaderBindingOffset, VkDeviceSize missShaderBindingStride,
                                   VkBuffer hitShaderBindingTableBuffer, VkDeviceSize hitShaderBindingOffset,
                                   VkDeviceSize hitShaderBindingStride, VkBuffer callableShaderBindingTableBuffer,
                                   VkDeviceSize callableShaderBindingOffset, VkDeviceSize callableShaderBindingStride,
                                   uint32_t width, uint32_t height, uint32_t depth, const ErrorObject& error_obj) const override;
bool PreCallValidateCreateRayTracingPipelinesNV(VkDevice device, VkPipelineCache pipelineCache, uint32_t createInfoCount,
                                                const VkRayTracingPipelineCreateInfoNV* pCreateInfos,
                                                const VkAllocationCallbacks* pAllocator, VkPipeline* pPipelines,
                                                const ErrorObject& error_obj) const override;
bool PreCallValidateGetRayTracingShaderGroupHandlesKHR(VkDevice device, VkPipeline pipeline, uint32_t firstGroup,
                                                       uint32_t groupCount, size_t dataSize, void* pData,
                                                       const ErrorObject& error_obj) const override;
bool PreCallValidateGetRayTracingShaderGroupHandlesNV(VkDevice device, VkPipeline pipeline, uint32_t firstGroup,
                                                      uint32_t groupCount, size_t dataSize, void* pData,
                                                      const ErrorObject& error_obj) const override;
bool PreCallValidateGetAccelerationStructureHandleNV(VkDevice device, VkAccelerationStructureNV accelerationStructure,
                                                     size_t dataSize, void* pData, const ErrorObject& error_obj) const override;
bool PreCallValidateCmdWriteAccelerationStructuresPropertiesNV(VkCommandBuffer commandBuffer, uint32_t accelerationStructureCount,
                                                               const VkAccelerationStructureNV* pAccelerationStructures,
                                                               VkQueryType queryType, VkQueryPool queryPool, uint32_t firstQuery,
                                                               const ErrorObject& error_obj) const override;
bool PreCallValidateCompileDeferredNV(VkDevice device, VkPipeline pipeline, uint32_t shader,
                                      const ErrorObject& error_obj) const override;
bool PreCallValidateGetMemoryHostPointerPropertiesEXT(VkDevice device, VkExternalMemoryHandleTypeFlagBits handleType,
                                                      const void* pHostPointer,
                                                      VkMemoryHostPointerPropertiesEXT* pMemoryHostPointerProperties,
                                                      const ErrorObject& error_obj) const override;
bool PreCallValidateCmdWriteBufferMarkerAMD(VkCommandBuffer commandBuffer, VkPipelineStageFlagBits pipelineStage,
                                            VkBuffer dstBuffer, VkDeviceSize dstOffset, uint32_t marker,
                                            const ErrorObject& error_obj) const override;
bool PreCallValidateGetPhysicalDeviceCalibrateableTimeDomainsEXT(VkPhysicalDevice physicalDevice, uint32_t* pTimeDomainCount,
                                                                 VkTimeDomainEXT* pTimeDomains,
                                                                 const ErrorObject& error_obj) const override;
bool PreCallValidateGetCalibratedTimestampsEXT(VkDevice device, uint32_t timestampCount,
                                               const VkCalibratedTimestampInfoEXT* pTimestampInfos, uint64_t* pTimestamps,
                                               uint64_t* pMaxDeviation, const ErrorObject& error_obj) const override;
bool PreCallValidateCmdDrawMeshTasksNV(VkCommandBuffer commandBuffer, uint32_t taskCount, uint32_t firstTask,
                                       const ErrorObject& error_obj) const override;
bool PreCallValidateCmdDrawMeshTasksIndirectNV(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                               uint32_t drawCount, uint32_t stride, const ErrorObject& error_obj) const override;
bool PreCallValidateCmdDrawMeshTasksIndirectCountNV(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                    VkBuffer countBuffer, VkDeviceSize countBufferOffset, uint32_t maxDrawCount,
                                                    uint32_t stride, const ErrorObject& error_obj) const override;
bool PreCallValidateCmdSetExclusiveScissorEnableNV(VkCommandBuffer commandBuffer, uint32_t firstExclusiveScissor,
                                                   uint32_t exclusiveScissorCount, const VkBool32* pExclusiveScissorEnables,
                                                   const ErrorObject& error_obj) const override;
bool PreCallValidateCmdSetExclusiveScissorNV(VkCommandBuffer commandBuffer, uint32_t firstExclusiveScissor,
                                             uint32_t exclusiveScissorCount, const VkRect2D* pExclusiveScissors,
                                             const ErrorObject& error_obj) const override;
bool PreCallValidateCmdSetCheckpointNV(VkCommandBuffer commandBuffer, const void* pCheckpointMarker,
                                       const ErrorObject& error_obj) const override;
bool PreCallValidateGetQueueCheckpointDataNV(VkQueue queue, uint32_t* pCheckpointDataCount, VkCheckpointDataNV* pCheckpointData,
                                             const ErrorObject& error_obj) const override;
bool PreCallValidateInitializePerformanceApiINTEL(VkDevice device, const VkInitializePerformanceApiInfoINTEL* pInitializeInfo,
                                                  const ErrorObject& error_obj) const override;
bool PreCallValidateUninitializePerformanceApiINTEL(VkDevice device, const ErrorObject& error_obj) const override;
bool PreCallValidateCmdSetPerformanceMarkerINTEL(VkCommandBuffer commandBuffer, const VkPerformanceMarkerInfoINTEL* pMarkerInfo,
                                                 const ErrorObject& error_obj) const override;
bool PreCallValidateCmdSetPerformanceStreamMarkerINTEL(VkCommandBuffer commandBuffer,
                                                       const VkPerformanceStreamMarkerInfoINTEL* pMarkerInfo,
                                                       const ErrorObject& error_obj) const override;
bool PreCallValidateCmdSetPerformanceOverrideINTEL(VkCommandBuffer commandBuffer,
                                                   const VkPerformanceOverrideInfoINTEL* pOverrideInfo,
                                                   const ErrorObject& error_obj) const override;
bool PreCallValidateAcquirePerformanceConfigurationINTEL(VkDevice device,
                                                         const VkPerformanceConfigurationAcquireInfoINTEL* pAcquireInfo,
                                                         VkPerformanceConfigurationINTEL* pConfiguration,
                                                         const ErrorObject& error_obj) const override;
bool PreCallValidateReleasePerformanceConfigurationINTEL(VkDevice device, VkPerformanceConfigurationINTEL configuration,
                                                         const ErrorObject& error_obj) const override;
bool PreCallValidateQueueSetPerformanceConfigurationINTEL(VkQueue queue, VkPerformanceConfigurationINTEL configuration,
                                                          const ErrorObject& error_obj) const override;
bool PreCallValidateGetPerformanceParameterINTEL(VkDevice device, VkPerformanceParameterTypeINTEL parameter,
                                                 VkPerformanceValueINTEL* pValue, const ErrorObject& error_obj) const override;
bool PreCallValidateSetLocalDimmingAMD(VkDevice device, VkSwapchainKHR swapChain, VkBool32 localDimmingEnable,
                                       const ErrorObject& error_obj) const override;
#ifdef VK_USE_PLATFORM_FUCHSIA
bool PreCallValidateCreateImagePipeSurfaceFUCHSIA(VkInstance instance, const VkImagePipeSurfaceCreateInfoFUCHSIA* pCreateInfo,
                                                  const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface,
                                                  const ErrorObject& error_obj) const override;
#endif  // VK_USE_PLATFORM_FUCHSIA
#ifdef VK_USE_PLATFORM_METAL_EXT
bool PreCallValidateCreateMetalSurfaceEXT(VkInstance instance, const VkMetalSurfaceCreateInfoEXT* pCreateInfo,
                                          const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface,
                                          const ErrorObject& error_obj) const override;
#endif  // VK_USE_PLATFORM_METAL_EXT
bool PreCallValidateGetBufferDeviceAddressEXT(VkDevice device, const VkBufferDeviceAddressInfo* pInfo,
                                              const ErrorObject& error_obj) const override;
bool PreCallValidateGetPhysicalDeviceToolPropertiesEXT(VkPhysicalDevice physicalDevice, uint32_t* pToolCount,
                                                       VkPhysicalDeviceToolProperties* pToolProperties,
                                                       const ErrorObject& error_obj) const override;
bool PreCallValidateGetPhysicalDeviceCooperativeMatrixPropertiesNV(VkPhysicalDevice physicalDevice, uint32_t* pPropertyCount,
                                                                   VkCooperativeMatrixPropertiesNV* pProperties,
                                                                   const ErrorObject& error_obj) const override;
bool PreCallValidateGetPhysicalDeviceSupportedFramebufferMixedSamplesCombinationsNV(
    VkPhysicalDevice physicalDevice, uint32_t* pCombinationCount, VkFramebufferMixedSamplesCombinationNV* pCombinations,
    const ErrorObject& error_obj) const override;
#ifdef VK_USE_PLATFORM_WIN32_KHR
bool PreCallValidateGetPhysicalDeviceSurfacePresentModes2EXT(VkPhysicalDevice physicalDevice,
                                                             const VkPhysicalDeviceSurfaceInfo2KHR* pSurfaceInfo,
                                                             uint32_t* pPresentModeCount, VkPresentModeKHR* pPresentModes,
                                                             const ErrorObject& error_obj) const override;
#endif  // VK_USE_PLATFORM_WIN32_KHR
#ifdef VK_USE_PLATFORM_WIN32_KHR
bool PreCallValidateAcquireFullScreenExclusiveModeEXT(VkDevice device, VkSwapchainKHR swapchain,
                                                      const ErrorObject& error_obj) const override;
#endif  // VK_USE_PLATFORM_WIN32_KHR
#ifdef VK_USE_PLATFORM_WIN32_KHR
bool PreCallValidateReleaseFullScreenExclusiveModeEXT(VkDevice device, VkSwapchainKHR swapchain,
                                                      const ErrorObject& error_obj) const override;
#endif  // VK_USE_PLATFORM_WIN32_KHR
bool PreCallValidateCreateHeadlessSurfaceEXT(VkInstance instance, const VkHeadlessSurfaceCreateInfoEXT* pCreateInfo,
                                             const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface,
                                             const ErrorObject& error_obj) const override;
bool PreCallValidateCmdSetLineStippleEXT(VkCommandBuffer commandBuffer, uint32_t lineStippleFactor, uint16_t lineStipplePattern,
                                         const ErrorObject& error_obj) const override;
bool PreCallValidateResetQueryPoolEXT(VkDevice device, VkQueryPool queryPool, uint32_t firstQuery, uint32_t queryCount,
                                      const ErrorObject& error_obj) const override;
bool PreCallValidateCmdSetCullModeEXT(VkCommandBuffer commandBuffer, VkCullModeFlags cullMode,
                                      const ErrorObject& error_obj) const override;
bool PreCallValidateCmdSetFrontFaceEXT(VkCommandBuffer commandBuffer, VkFrontFace frontFace,
                                       const ErrorObject& error_obj) const override;
bool PreCallValidateCmdSetPrimitiveTopologyEXT(VkCommandBuffer commandBuffer, VkPrimitiveTopology primitiveTopology,
                                               const ErrorObject& error_obj) const override;
bool PreCallValidateCmdSetViewportWithCountEXT(VkCommandBuffer commandBuffer, uint32_t viewportCount, const VkViewport* pViewports,
                                               const ErrorObject& error_obj) const override;
bool PreCallValidateCmdSetScissorWithCountEXT(VkCommandBuffer commandBuffer, uint32_t scissorCount, const VkRect2D* pScissors,
                                              const ErrorObject& error_obj) const override;
bool PreCallValidateCmdBindVertexBuffers2EXT(VkCommandBuffer commandBuffer, uint32_t firstBinding, uint32_t bindingCount,
                                             const VkBuffer* pBuffers, const VkDeviceSize* pOffsets, const VkDeviceSize* pSizes,
                                             const VkDeviceSize* pStrides, const ErrorObject& error_obj) const override;
bool PreCallValidateCmdSetDepthTestEnableEXT(VkCommandBuffer commandBuffer, VkBool32 depthTestEnable,
                                             const ErrorObject& error_obj) const override;
bool PreCallValidateCmdSetDepthWriteEnableEXT(VkCommandBuffer commandBuffer, VkBool32 depthWriteEnable,
                                              const ErrorObject& error_obj) const override;
bool PreCallValidateCmdSetDepthCompareOpEXT(VkCommandBuffer commandBuffer, VkCompareOp depthCompareOp,
                                            const ErrorObject& error_obj) const override;
bool PreCallValidateCmdSetDepthBoundsTestEnableEXT(VkCommandBuffer commandBuffer, VkBool32 depthBoundsTestEnable,
                                                   const ErrorObject& error_obj) const override;
bool PreCallValidateCmdSetStencilTestEnableEXT(VkCommandBuffer commandBuffer, VkBool32 stencilTestEnable,
                                               const ErrorObject& error_obj) const override;
bool PreCallValidateCmdSetStencilOpEXT(VkCommandBuffer commandBuffer, VkStencilFaceFlags faceMask, VkStencilOp failOp,
                                       VkStencilOp passOp, VkStencilOp depthFailOp, VkCompareOp compareOp,
                                       const ErrorObject& error_obj) const override;
bool PreCallValidateCopyMemoryToImageEXT(VkDevice device, const VkCopyMemoryToImageInfoEXT* pCopyMemoryToImageInfo,
                                         const ErrorObject& error_obj) const override;
bool PreCallValidateCopyImageToMemoryEXT(VkDevice device, const VkCopyImageToMemoryInfoEXT* pCopyImageToMemoryInfo,
                                         const ErrorObject& error_obj) const override;
bool PreCallValidateCopyImageToImageEXT(VkDevice device, const VkCopyImageToImageInfoEXT* pCopyImageToImageInfo,
                                        const ErrorObject& error_obj) const override;
bool PreCallValidateTransitionImageLayoutEXT(VkDevice device, uint32_t transitionCount,
                                             const VkHostImageLayoutTransitionInfoEXT* pTransitions,
                                             const ErrorObject& error_obj) const override;
bool PreCallValidateGetImageSubresourceLayout2EXT(VkDevice device, VkImage image, const VkImageSubresource2KHR* pSubresource,
                                                  VkSubresourceLayout2KHR* pLayout, const ErrorObject& error_obj) const override;
bool PreCallValidateReleaseSwapchainImagesEXT(VkDevice device, const VkReleaseSwapchainImagesInfoEXT* pReleaseInfo,
                                              const ErrorObject& error_obj) const override;
bool PreCallValidateGetGeneratedCommandsMemoryRequirementsNV(VkDevice device,
                                                             const VkGeneratedCommandsMemoryRequirementsInfoNV* pInfo,
                                                             VkMemoryRequirements2* pMemoryRequirements,
                                                             const ErrorObject& error_obj) const override;
bool PreCallValidateCmdPreprocessGeneratedCommandsNV(VkCommandBuffer commandBuffer,
                                                     const VkGeneratedCommandsInfoNV* pGeneratedCommandsInfo,
                                                     const ErrorObject& error_obj) const override;
bool PreCallValidateCmdExecuteGeneratedCommandsNV(VkCommandBuffer commandBuffer, VkBool32 isPreprocessed,
                                                  const VkGeneratedCommandsInfoNV* pGeneratedCommandsInfo,
                                                  const ErrorObject& error_obj) const override;
bool PreCallValidateCmdBindPipelineShaderGroupNV(VkCommandBuffer commandBuffer, VkPipelineBindPoint pipelineBindPoint,
                                                 VkPipeline pipeline, uint32_t groupIndex,
                                                 const ErrorObject& error_obj) const override;
bool PreCallValidateCreateIndirectCommandsLayoutNV(VkDevice device, const VkIndirectCommandsLayoutCreateInfoNV* pCreateInfo,
                                                   const VkAllocationCallbacks* pAllocator,
                                                   VkIndirectCommandsLayoutNV* pIndirectCommandsLayout,
                                                   const ErrorObject& error_obj) const override;
bool PreCallValidateDestroyIndirectCommandsLayoutNV(VkDevice device, VkIndirectCommandsLayoutNV indirectCommandsLayout,
                                                    const VkAllocationCallbacks* pAllocator,
                                                    const ErrorObject& error_obj) const override;
bool PreCallValidateCmdSetDepthBias2EXT(VkCommandBuffer commandBuffer, const VkDepthBiasInfoEXT* pDepthBiasInfo,
                                        const ErrorObject& error_obj) const override;
bool PreCallValidateAcquireDrmDisplayEXT(VkPhysicalDevice physicalDevice, int32_t drmFd, VkDisplayKHR display,
                                         const ErrorObject& error_obj) const override;
bool PreCallValidateGetDrmDisplayEXT(VkPhysicalDevice physicalDevice, int32_t drmFd, uint32_t connectorId, VkDisplayKHR* display,
                                     const ErrorObject& error_obj) const override;
bool PreCallValidateCreatePrivateDataSlotEXT(VkDevice device, const VkPrivateDataSlotCreateInfo* pCreateInfo,
                                             const VkAllocationCallbacks* pAllocator, VkPrivateDataSlot* pPrivateDataSlot,
                                             const ErrorObject& error_obj) const override;
bool PreCallValidateDestroyPrivateDataSlotEXT(VkDevice device, VkPrivateDataSlot privateDataSlot,
                                              const VkAllocationCallbacks* pAllocator, const ErrorObject& error_obj) const override;
bool PreCallValidateSetPrivateDataEXT(VkDevice device, VkObjectType objectType, uint64_t objectHandle,
                                      VkPrivateDataSlot privateDataSlot, uint64_t data,
                                      const ErrorObject& error_obj) const override;
bool PreCallValidateGetPrivateDataEXT(VkDevice device, VkObjectType objectType, uint64_t objectHandle,
                                      VkPrivateDataSlot privateDataSlot, uint64_t* pData,
                                      const ErrorObject& error_obj) const override;
#ifdef VK_USE_PLATFORM_METAL_EXT
bool PreCallValidateExportMetalObjectsEXT(VkDevice device, VkExportMetalObjectsInfoEXT* pMetalObjectsInfo,
                                          const ErrorObject& error_obj) const override;
#endif  // VK_USE_PLATFORM_METAL_EXT
bool PreCallValidateGetDescriptorSetLayoutSizeEXT(VkDevice device, VkDescriptorSetLayout layout, VkDeviceSize* pLayoutSizeInBytes,
                                                  const ErrorObject& error_obj) const override;
bool PreCallValidateGetDescriptorSetLayoutBindingOffsetEXT(VkDevice device, VkDescriptorSetLayout layout, uint32_t binding,
                                                           VkDeviceSize* pOffset, const ErrorObject& error_obj) const override;
bool PreCallValidateGetDescriptorEXT(VkDevice device, const VkDescriptorGetInfoEXT* pDescriptorInfo, size_t dataSize,
                                     void* pDescriptor, const ErrorObject& error_obj) const override;
bool PreCallValidateCmdBindDescriptorBuffersEXT(VkCommandBuffer commandBuffer, uint32_t bufferCount,
                                                const VkDescriptorBufferBindingInfoEXT* pBindingInfos,
                                                const ErrorObject& error_obj) const override;
bool PreCallValidateCmdSetDescriptorBufferOffsetsEXT(VkCommandBuffer commandBuffer, VkPipelineBindPoint pipelineBindPoint,
                                                     VkPipelineLayout layout, uint32_t firstSet, uint32_t setCount,
                                                     const uint32_t* pBufferIndices, const VkDeviceSize* pOffsets,
                                                     const ErrorObject& error_obj) const override;
bool PreCallValidateCmdBindDescriptorBufferEmbeddedSamplersEXT(VkCommandBuffer commandBuffer, VkPipelineBindPoint pipelineBindPoint,
                                                               VkPipelineLayout layout, uint32_t set,
                                                               const ErrorObject& error_obj) const override;
bool PreCallValidateGetBufferOpaqueCaptureDescriptorDataEXT(VkDevice device, const VkBufferCaptureDescriptorDataInfoEXT* pInfo,
                                                            void* pData, const ErrorObject& error_obj) const override;
bool PreCallValidateGetImageOpaqueCaptureDescriptorDataEXT(VkDevice device, const VkImageCaptureDescriptorDataInfoEXT* pInfo,
                                                           void* pData, const ErrorObject& error_obj) const override;
bool PreCallValidateGetImageViewOpaqueCaptureDescriptorDataEXT(VkDevice device,
                                                               const VkImageViewCaptureDescriptorDataInfoEXT* pInfo, void* pData,
                                                               const ErrorObject& error_obj) const override;
bool PreCallValidateGetSamplerOpaqueCaptureDescriptorDataEXT(VkDevice device, const VkSamplerCaptureDescriptorDataInfoEXT* pInfo,
                                                             void* pData, const ErrorObject& error_obj) const override;
bool PreCallValidateGetAccelerationStructureOpaqueCaptureDescriptorDataEXT(
    VkDevice device, const VkAccelerationStructureCaptureDescriptorDataInfoEXT* pInfo, void* pData,
    const ErrorObject& error_obj) const override;
bool PreCallValidateCmdSetFragmentShadingRateEnumNV(VkCommandBuffer commandBuffer, VkFragmentShadingRateNV shadingRate,
                                                    const VkFragmentShadingRateCombinerOpKHR combinerOps[2],
                                                    const ErrorObject& error_obj) const override;
bool PreCallValidateGetDeviceFaultInfoEXT(VkDevice device, VkDeviceFaultCountsEXT* pFaultCounts, VkDeviceFaultInfoEXT* pFaultInfo,
                                          const ErrorObject& error_obj) const override;
#ifdef VK_USE_PLATFORM_WIN32_KHR
bool PreCallValidateAcquireWinrtDisplayNV(VkPhysicalDevice physicalDevice, VkDisplayKHR display,
                                          const ErrorObject& error_obj) const override;
#endif  // VK_USE_PLATFORM_WIN32_KHR
#ifdef VK_USE_PLATFORM_WIN32_KHR
bool PreCallValidateGetWinrtDisplayNV(VkPhysicalDevice physicalDevice, uint32_t deviceRelativeId, VkDisplayKHR* pDisplay,
                                      const ErrorObject& error_obj) const override;
#endif  // VK_USE_PLATFORM_WIN32_KHR
#ifdef VK_USE_PLATFORM_DIRECTFB_EXT
bool PreCallValidateCreateDirectFBSurfaceEXT(VkInstance instance, const VkDirectFBSurfaceCreateInfoEXT* pCreateInfo,
                                             const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface,
                                             const ErrorObject& error_obj) const override;
#endif  // VK_USE_PLATFORM_DIRECTFB_EXT
#ifdef VK_USE_PLATFORM_DIRECTFB_EXT
bool PreCallValidateGetPhysicalDeviceDirectFBPresentationSupportEXT(VkPhysicalDevice physicalDevice, uint32_t queueFamilyIndex,
                                                                    IDirectFB* dfb, const ErrorObject& error_obj) const override;
#endif  // VK_USE_PLATFORM_DIRECTFB_EXT
bool PreCallValidateCmdSetVertexInputEXT(VkCommandBuffer commandBuffer, uint32_t vertexBindingDescriptionCount,
                                         const VkVertexInputBindingDescription2EXT* pVertexBindingDescriptions,
                                         uint32_t vertexAttributeDescriptionCount,
                                         const VkVertexInputAttributeDescription2EXT* pVertexAttributeDescriptions,
                                         const ErrorObject& error_obj) const override;
#ifdef VK_USE_PLATFORM_FUCHSIA
bool PreCallValidateGetMemoryZirconHandleFUCHSIA(VkDevice device, const VkMemoryGetZirconHandleInfoFUCHSIA* pGetZirconHandleInfo,
                                                 zx_handle_t* pZirconHandle, const ErrorObject& error_obj) const override;
#endif  // VK_USE_PLATFORM_FUCHSIA
#ifdef VK_USE_PLATFORM_FUCHSIA
bool PreCallValidateGetMemoryZirconHandlePropertiesFUCHSIA(VkDevice device, VkExternalMemoryHandleTypeFlagBits handleType,
                                                           zx_handle_t zirconHandle,
                                                           VkMemoryZirconHandlePropertiesFUCHSIA* pMemoryZirconHandleProperties,
                                                           const ErrorObject& error_obj) const override;
#endif  // VK_USE_PLATFORM_FUCHSIA
#ifdef VK_USE_PLATFORM_FUCHSIA
bool PreCallValidateImportSemaphoreZirconHandleFUCHSIA(
    VkDevice device, const VkImportSemaphoreZirconHandleInfoFUCHSIA* pImportSemaphoreZirconHandleInfo,
    const ErrorObject& error_obj) const override;
#endif  // VK_USE_PLATFORM_FUCHSIA
#ifdef VK_USE_PLATFORM_FUCHSIA
bool PreCallValidateGetSemaphoreZirconHandleFUCHSIA(VkDevice device,
                                                    const VkSemaphoreGetZirconHandleInfoFUCHSIA* pGetZirconHandleInfo,
                                                    zx_handle_t* pZirconHandle, const ErrorObject& error_obj) const override;
#endif  // VK_USE_PLATFORM_FUCHSIA
#ifdef VK_USE_PLATFORM_FUCHSIA
bool PreCallValidateCreateBufferCollectionFUCHSIA(VkDevice device, const VkBufferCollectionCreateInfoFUCHSIA* pCreateInfo,
                                                  const VkAllocationCallbacks* pAllocator, VkBufferCollectionFUCHSIA* pCollection,
                                                  const ErrorObject& error_obj) const override;
#endif  // VK_USE_PLATFORM_FUCHSIA
#ifdef VK_USE_PLATFORM_FUCHSIA
bool PreCallValidateSetBufferCollectionImageConstraintsFUCHSIA(VkDevice device, VkBufferCollectionFUCHSIA collection,
                                                               const VkImageConstraintsInfoFUCHSIA* pImageConstraintsInfo,
                                                               const ErrorObject& error_obj) const override;
#endif  // VK_USE_PLATFORM_FUCHSIA
#ifdef VK_USE_PLATFORM_FUCHSIA
bool PreCallValidateSetBufferCollectionBufferConstraintsFUCHSIA(VkDevice device, VkBufferCollectionFUCHSIA collection,
                                                                const VkBufferConstraintsInfoFUCHSIA* pBufferConstraintsInfo,
                                                                const ErrorObject& error_obj) const override;
#endif  // VK_USE_PLATFORM_FUCHSIA
#ifdef VK_USE_PLATFORM_FUCHSIA
bool PreCallValidateDestroyBufferCollectionFUCHSIA(VkDevice device, VkBufferCollectionFUCHSIA collection,
                                                   const VkAllocationCallbacks* pAllocator,
                                                   const ErrorObject& error_obj) const override;
#endif  // VK_USE_PLATFORM_FUCHSIA
#ifdef VK_USE_PLATFORM_FUCHSIA
bool PreCallValidateGetBufferCollectionPropertiesFUCHSIA(VkDevice device, VkBufferCollectionFUCHSIA collection,
                                                         VkBufferCollectionPropertiesFUCHSIA* pProperties,
                                                         const ErrorObject& error_obj) const override;
#endif  // VK_USE_PLATFORM_FUCHSIA
bool PreCallValidateGetDeviceSubpassShadingMaxWorkgroupSizeHUAWEI(VkDevice device, VkRenderPass renderpass,
                                                                  VkExtent2D* pMaxWorkgroupSize,
                                                                  const ErrorObject& error_obj) const override;
bool PreCallValidateCmdSubpassShadingHUAWEI(VkCommandBuffer commandBuffer, const ErrorObject& error_obj) const override;
bool PreCallValidateCmdBindInvocationMaskHUAWEI(VkCommandBuffer commandBuffer, VkImageView imageView, VkImageLayout imageLayout,
                                                const ErrorObject& error_obj) const override;
bool PreCallValidateGetMemoryRemoteAddressNV(VkDevice device, const VkMemoryGetRemoteAddressInfoNV* pMemoryGetRemoteAddressInfo,
                                             VkRemoteAddressNV* pAddress, const ErrorObject& error_obj) const override;
bool PreCallValidateGetPipelinePropertiesEXT(VkDevice device, const VkPipelineInfoEXT* pPipelineInfo,
                                             VkBaseOutStructure* pPipelineProperties, const ErrorObject& error_obj) const override;
bool PreCallValidateCmdSetPatchControlPointsEXT(VkCommandBuffer commandBuffer, uint32_t patchControlPoints,
                                                const ErrorObject& error_obj) const override;
bool PreCallValidateCmdSetRasterizerDiscardEnableEXT(VkCommandBuffer commandBuffer, VkBool32 rasterizerDiscardEnable,
                                                     const ErrorObject& error_obj) const override;
bool PreCallValidateCmdSetDepthBiasEnableEXT(VkCommandBuffer commandBuffer, VkBool32 depthBiasEnable,
                                             const ErrorObject& error_obj) const override;
bool PreCallValidateCmdSetLogicOpEXT(VkCommandBuffer commandBuffer, VkLogicOp logicOp, const ErrorObject& error_obj) const override;
bool PreCallValidateCmdSetPrimitiveRestartEnableEXT(VkCommandBuffer commandBuffer, VkBool32 primitiveRestartEnable,
                                                    const ErrorObject& error_obj) const override;
#ifdef VK_USE_PLATFORM_SCREEN_QNX
bool PreCallValidateCreateScreenSurfaceQNX(VkInstance instance, const VkScreenSurfaceCreateInfoQNX* pCreateInfo,
                                           const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface,
                                           const ErrorObject& error_obj) const override;
#endif  // VK_USE_PLATFORM_SCREEN_QNX
#ifdef VK_USE_PLATFORM_SCREEN_QNX
bool PreCallValidateGetPhysicalDeviceScreenPresentationSupportQNX(VkPhysicalDevice physicalDevice, uint32_t queueFamilyIndex,
                                                                  struct _screen_window* window,
                                                                  const ErrorObject& error_obj) const override;
#endif  // VK_USE_PLATFORM_SCREEN_QNX
bool PreCallValidateCmdSetColorWriteEnableEXT(VkCommandBuffer commandBuffer, uint32_t attachmentCount,
                                              const VkBool32* pColorWriteEnables, const ErrorObject& error_obj) const override;
bool PreCallValidateCmdDrawMultiEXT(VkCommandBuffer commandBuffer, uint32_t drawCount, const VkMultiDrawInfoEXT* pVertexInfo,
                                    uint32_t instanceCount, uint32_t firstInstance, uint32_t stride,
                                    const ErrorObject& error_obj) const override;
bool PreCallValidateCmdDrawMultiIndexedEXT(VkCommandBuffer commandBuffer, uint32_t drawCount,
                                           const VkMultiDrawIndexedInfoEXT* pIndexInfo, uint32_t instanceCount,
                                           uint32_t firstInstance, uint32_t stride, const int32_t* pVertexOffset,
                                           const ErrorObject& error_obj) const override;
bool PreCallValidateCreateMicromapEXT(VkDevice device, const VkMicromapCreateInfoEXT* pCreateInfo,
                                      const VkAllocationCallbacks* pAllocator, VkMicromapEXT* pMicromap,
                                      const ErrorObject& error_obj) const override;
bool PreCallValidateDestroyMicromapEXT(VkDevice device, VkMicromapEXT micromap, const VkAllocationCallbacks* pAllocator,
                                       const ErrorObject& error_obj) const override;
bool PreCallValidateCmdBuildMicromapsEXT(VkCommandBuffer commandBuffer, uint32_t infoCount, const VkMicromapBuildInfoEXT* pInfos,
                                         const ErrorObject& error_obj) const override;
bool PreCallValidateBuildMicromapsEXT(VkDevice device, VkDeferredOperationKHR deferredOperation, uint32_t infoCount,
                                      const VkMicromapBuildInfoEXT* pInfos, const ErrorObject& error_obj) const override;
bool PreCallValidateCopyMicromapEXT(VkDevice device, VkDeferredOperationKHR deferredOperation, const VkCopyMicromapInfoEXT* pInfo,
                                    const ErrorObject& error_obj) const override;
bool PreCallValidateCopyMicromapToMemoryEXT(VkDevice device, VkDeferredOperationKHR deferredOperation,
                                            const VkCopyMicromapToMemoryInfoEXT* pInfo,
                                            const ErrorObject& error_obj) const override;
bool PreCallValidateCopyMemoryToMicromapEXT(VkDevice device, VkDeferredOperationKHR deferredOperation,
                                            const VkCopyMemoryToMicromapInfoEXT* pInfo,
                                            const ErrorObject& error_obj) const override;
bool PreCallValidateWriteMicromapsPropertiesEXT(VkDevice device, uint32_t micromapCount, const VkMicromapEXT* pMicromaps,
                                                VkQueryType queryType, size_t dataSize, void* pData, size_t stride,
                                                const ErrorObject& error_obj) const override;
bool PreCallValidateCmdCopyMicromapEXT(VkCommandBuffer commandBuffer, const VkCopyMicromapInfoEXT* pInfo,
                                       const ErrorObject& error_obj) const override;
bool PreCallValidateCmdCopyMicromapToMemoryEXT(VkCommandBuffer commandBuffer, const VkCopyMicromapToMemoryInfoEXT* pInfo,
                                               const ErrorObject& error_obj) const override;
bool PreCallValidateCmdCopyMemoryToMicromapEXT(VkCommandBuffer commandBuffer, const VkCopyMemoryToMicromapInfoEXT* pInfo,
                                               const ErrorObject& error_obj) const override;
bool PreCallValidateCmdWriteMicromapsPropertiesEXT(VkCommandBuffer commandBuffer, uint32_t micromapCount,
                                                   const VkMicromapEXT* pMicromaps, VkQueryType queryType, VkQueryPool queryPool,
                                                   uint32_t firstQuery, const ErrorObject& error_obj) const override;
bool PreCallValidateGetDeviceMicromapCompatibilityEXT(VkDevice device, const VkMicromapVersionInfoEXT* pVersionInfo,
                                                      VkAccelerationStructureCompatibilityKHR* pCompatibility,
                                                      const ErrorObject& error_obj) const override;
bool PreCallValidateGetMicromapBuildSizesEXT(VkDevice device, VkAccelerationStructureBuildTypeKHR buildType,
                                             const VkMicromapBuildInfoEXT* pBuildInfo, VkMicromapBuildSizesInfoEXT* pSizeInfo,
                                             const ErrorObject& error_obj) const override;
bool PreCallValidateCmdDrawClusterHUAWEI(VkCommandBuffer commandBuffer, uint32_t groupCountX, uint32_t groupCountY,
                                         uint32_t groupCountZ, const ErrorObject& error_obj) const override;
bool PreCallValidateCmdDrawClusterIndirectHUAWEI(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                 const ErrorObject& error_obj) const override;
bool PreCallValidateSetDeviceMemoryPriorityEXT(VkDevice device, VkDeviceMemory memory, float priority,
                                               const ErrorObject& error_obj) const override;
bool PreCallValidateGetDescriptorSetLayoutHostMappingInfoVALVE(VkDevice device,
                                                               const VkDescriptorSetBindingReferenceVALVE* pBindingReference,
                                                               VkDescriptorSetLayoutHostMappingInfoVALVE* pHostMapping,
                                                               const ErrorObject& error_obj) const override;
bool PreCallValidateGetDescriptorSetHostMappingVALVE(VkDevice device, VkDescriptorSet descriptorSet, void** ppData,
                                                     const ErrorObject& error_obj) const override;
bool PreCallValidateCmdCopyMemoryIndirectNV(VkCommandBuffer commandBuffer, VkDeviceAddress copyBufferAddress, uint32_t copyCount,
                                            uint32_t stride, const ErrorObject& error_obj) const override;
bool PreCallValidateCmdCopyMemoryToImageIndirectNV(VkCommandBuffer commandBuffer, VkDeviceAddress copyBufferAddress,
                                                   uint32_t copyCount, uint32_t stride, VkImage dstImage,
                                                   VkImageLayout dstImageLayout, const VkImageSubresourceLayers* pImageSubresources,
                                                   const ErrorObject& error_obj) const override;
bool PreCallValidateCmdDecompressMemoryNV(VkCommandBuffer commandBuffer, uint32_t decompressRegionCount,
                                          const VkDecompressMemoryRegionNV* pDecompressMemoryRegions,
                                          const ErrorObject& error_obj) const override;
bool PreCallValidateCmdDecompressMemoryIndirectCountNV(VkCommandBuffer commandBuffer, VkDeviceAddress indirectCommandsAddress,
                                                       VkDeviceAddress indirectCommandsCountAddress, uint32_t stride,
                                                       const ErrorObject& error_obj) const override;
bool PreCallValidateGetPipelineIndirectMemoryRequirementsNV(VkDevice device, const VkComputePipelineCreateInfo* pCreateInfo,
                                                            VkMemoryRequirements2* pMemoryRequirements,
                                                            const ErrorObject& error_obj) const override;
bool PreCallValidateCmdUpdatePipelineIndirectBufferNV(VkCommandBuffer commandBuffer, VkPipelineBindPoint pipelineBindPoint,
                                                      VkPipeline pipeline, const ErrorObject& error_obj) const override;
bool PreCallValidateGetPipelineIndirectDeviceAddressNV(VkDevice device, const VkPipelineIndirectDeviceAddressInfoNV* pInfo,
                                                       const ErrorObject& error_obj) const override;
bool PreCallValidateCmdSetTessellationDomainOriginEXT(VkCommandBuffer commandBuffer, VkTessellationDomainOrigin domainOrigin,
                                                      const ErrorObject& error_obj) const override;
bool PreCallValidateCmdSetDepthClampEnableEXT(VkCommandBuffer commandBuffer, VkBool32 depthClampEnable,
                                              const ErrorObject& error_obj) const override;
bool PreCallValidateCmdSetPolygonModeEXT(VkCommandBuffer commandBuffer, VkPolygonMode polygonMode,
                                         const ErrorObject& error_obj) const override;
bool PreCallValidateCmdSetRasterizationSamplesEXT(VkCommandBuffer commandBuffer, VkSampleCountFlagBits rasterizationSamples,
                                                  const ErrorObject& error_obj) const override;
bool PreCallValidateCmdSetSampleMaskEXT(VkCommandBuffer commandBuffer, VkSampleCountFlagBits samples,
                                        const VkSampleMask* pSampleMask, const ErrorObject& error_obj) const override;
bool PreCallValidateCmdSetAlphaToCoverageEnableEXT(VkCommandBuffer commandBuffer, VkBool32 alphaToCoverageEnable,
                                                   const ErrorObject& error_obj) const override;
bool PreCallValidateCmdSetAlphaToOneEnableEXT(VkCommandBuffer commandBuffer, VkBool32 alphaToOneEnable,
                                              const ErrorObject& error_obj) const override;
bool PreCallValidateCmdSetLogicOpEnableEXT(VkCommandBuffer commandBuffer, VkBool32 logicOpEnable,
                                           const ErrorObject& error_obj) const override;
bool PreCallValidateCmdSetColorBlendEnableEXT(VkCommandBuffer commandBuffer, uint32_t firstAttachment, uint32_t attachmentCount,
                                              const VkBool32* pColorBlendEnables, const ErrorObject& error_obj) const override;
bool PreCallValidateCmdSetColorBlendEquationEXT(VkCommandBuffer commandBuffer, uint32_t firstAttachment, uint32_t attachmentCount,
                                                const VkColorBlendEquationEXT* pColorBlendEquations,
                                                const ErrorObject& error_obj) const override;
bool PreCallValidateCmdSetColorWriteMaskEXT(VkCommandBuffer commandBuffer, uint32_t firstAttachment, uint32_t attachmentCount,
                                            const VkColorComponentFlags* pColorWriteMasks,
                                            const ErrorObject& error_obj) const override;
bool PreCallValidateCmdSetRasterizationStreamEXT(VkCommandBuffer commandBuffer, uint32_t rasterizationStream,
                                                 const ErrorObject& error_obj) const override;
bool PreCallValidateCmdSetConservativeRasterizationModeEXT(VkCommandBuffer commandBuffer,
                                                           VkConservativeRasterizationModeEXT conservativeRasterizationMode,
                                                           const ErrorObject& error_obj) const override;
bool PreCallValidateCmdSetExtraPrimitiveOverestimationSizeEXT(VkCommandBuffer commandBuffer, float extraPrimitiveOverestimationSize,
                                                              const ErrorObject& error_obj) const override;
bool PreCallValidateCmdSetDepthClipEnableEXT(VkCommandBuffer commandBuffer, VkBool32 depthClipEnable,
                                             const ErrorObject& error_obj) const override;
bool PreCallValidateCmdSetSampleLocationsEnableEXT(VkCommandBuffer commandBuffer, VkBool32 sampleLocationsEnable,
                                                   const ErrorObject& error_obj) const override;
bool PreCallValidateCmdSetColorBlendAdvancedEXT(VkCommandBuffer commandBuffer, uint32_t firstAttachment, uint32_t attachmentCount,
                                                const VkColorBlendAdvancedEXT* pColorBlendAdvanced,
                                                const ErrorObject& error_obj) const override;
bool PreCallValidateCmdSetProvokingVertexModeEXT(VkCommandBuffer commandBuffer, VkProvokingVertexModeEXT provokingVertexMode,
                                                 const ErrorObject& error_obj) const override;
bool PreCallValidateCmdSetLineRasterizationModeEXT(VkCommandBuffer commandBuffer, VkLineRasterizationModeEXT lineRasterizationMode,
                                                   const ErrorObject& error_obj) const override;
bool PreCallValidateCmdSetLineStippleEnableEXT(VkCommandBuffer commandBuffer, VkBool32 stippledLineEnable,
                                               const ErrorObject& error_obj) const override;
bool PreCallValidateCmdSetDepthClipNegativeOneToOneEXT(VkCommandBuffer commandBuffer, VkBool32 negativeOneToOne,
                                                       const ErrorObject& error_obj) const override;
bool PreCallValidateCmdSetViewportWScalingEnableNV(VkCommandBuffer commandBuffer, VkBool32 viewportWScalingEnable,
                                                   const ErrorObject& error_obj) const override;
bool PreCallValidateCmdSetViewportSwizzleNV(VkCommandBuffer commandBuffer, uint32_t firstViewport, uint32_t viewportCount,
                                            const VkViewportSwizzleNV* pViewportSwizzles,
                                            const ErrorObject& error_obj) const override;
bool PreCallValidateCmdSetCoverageToColorEnableNV(VkCommandBuffer commandBuffer, VkBool32 coverageToColorEnable,
                                                  const ErrorObject& error_obj) const override;
bool PreCallValidateCmdSetCoverageToColorLocationNV(VkCommandBuffer commandBuffer, uint32_t coverageToColorLocation,
                                                    const ErrorObject& error_obj) const override;
bool PreCallValidateCmdSetCoverageModulationModeNV(VkCommandBuffer commandBuffer, VkCoverageModulationModeNV coverageModulationMode,
                                                   const ErrorObject& error_obj) const override;
bool PreCallValidateCmdSetCoverageModulationTableEnableNV(VkCommandBuffer commandBuffer, VkBool32 coverageModulationTableEnable,
                                                          const ErrorObject& error_obj) const override;
bool PreCallValidateCmdSetCoverageModulationTableNV(VkCommandBuffer commandBuffer, uint32_t coverageModulationTableCount,
                                                    const float* pCoverageModulationTable,
                                                    const ErrorObject& error_obj) const override;
bool PreCallValidateCmdSetShadingRateImageEnableNV(VkCommandBuffer commandBuffer, VkBool32 shadingRateImageEnable,
                                                   const ErrorObject& error_obj) const override;
bool PreCallValidateCmdSetRepresentativeFragmentTestEnableNV(VkCommandBuffer commandBuffer,
                                                             VkBool32 representativeFragmentTestEnable,
                                                             const ErrorObject& error_obj) const override;
bool PreCallValidateCmdSetCoverageReductionModeNV(VkCommandBuffer commandBuffer, VkCoverageReductionModeNV coverageReductionMode,
                                                  const ErrorObject& error_obj) const override;
bool PreCallValidateGetShaderModuleIdentifierEXT(VkDevice device, VkShaderModule shaderModule,
                                                 VkShaderModuleIdentifierEXT* pIdentifier,
                                                 const ErrorObject& error_obj) const override;
bool PreCallValidateGetShaderModuleCreateInfoIdentifierEXT(VkDevice device, const VkShaderModuleCreateInfo* pCreateInfo,
                                                           VkShaderModuleIdentifierEXT* pIdentifier,
                                                           const ErrorObject& error_obj) const override;
bool PreCallValidateGetPhysicalDeviceOpticalFlowImageFormatsNV(VkPhysicalDevice physicalDevice,
                                                               const VkOpticalFlowImageFormatInfoNV* pOpticalFlowImageFormatInfo,
                                                               uint32_t* pFormatCount,
                                                               VkOpticalFlowImageFormatPropertiesNV* pImageFormatProperties,
                                                               const ErrorObject& error_obj) const override;
bool PreCallValidateCreateOpticalFlowSessionNV(VkDevice device, const VkOpticalFlowSessionCreateInfoNV* pCreateInfo,
                                               const VkAllocationCallbacks* pAllocator, VkOpticalFlowSessionNV* pSession,
                                               const ErrorObject& error_obj) const override;
bool PreCallValidateDestroyOpticalFlowSessionNV(VkDevice device, VkOpticalFlowSessionNV session,
                                                const VkAllocationCallbacks* pAllocator,
                                                const ErrorObject& error_obj) const override;
bool PreCallValidateBindOpticalFlowSessionImageNV(VkDevice device, VkOpticalFlowSessionNV session,
                                                  VkOpticalFlowSessionBindingPointNV bindingPoint, VkImageView view,
                                                  VkImageLayout layout, const ErrorObject& error_obj) const override;
bool PreCallValidateCmdOpticalFlowExecuteNV(VkCommandBuffer commandBuffer, VkOpticalFlowSessionNV session,
                                            const VkOpticalFlowExecuteInfoNV* pExecuteInfo,
                                            const ErrorObject& error_obj) const override;
bool PreCallValidateCreateShadersEXT(VkDevice device, uint32_t createInfoCount, const VkShaderCreateInfoEXT* pCreateInfos,
                                     const VkAllocationCallbacks* pAllocator, VkShaderEXT* pShaders,
                                     const ErrorObject& error_obj) const override;
bool PreCallValidateDestroyShaderEXT(VkDevice device, VkShaderEXT shader, const VkAllocationCallbacks* pAllocator,
                                     const ErrorObject& error_obj) const override;
bool PreCallValidateGetShaderBinaryDataEXT(VkDevice device, VkShaderEXT shader, size_t* pDataSize, void* pData,
                                           const ErrorObject& error_obj) const override;
bool PreCallValidateCmdBindShadersEXT(VkCommandBuffer commandBuffer, uint32_t stageCount, const VkShaderStageFlagBits* pStages,
                                      const VkShaderEXT* pShaders, const ErrorObject& error_obj) const override;
bool PreCallValidateGetFramebufferTilePropertiesQCOM(VkDevice device, VkFramebuffer framebuffer, uint32_t* pPropertiesCount,
                                                     VkTilePropertiesQCOM* pProperties,
                                                     const ErrorObject& error_obj) const override;
bool PreCallValidateGetDynamicRenderingTilePropertiesQCOM(VkDevice device, const VkRenderingInfo* pRenderingInfo,
                                                          VkTilePropertiesQCOM* pProperties,
                                                          const ErrorObject& error_obj) const override;
bool PreCallValidateSetLatencySleepModeNV(VkDevice device, VkSwapchainKHR swapchain, const VkLatencySleepModeInfoNV* pSleepModeInfo,
                                          const ErrorObject& error_obj) const override;
bool PreCallValidateLatencySleepNV(VkDevice device, VkSwapchainKHR swapchain, const VkLatencySleepInfoNV* pSleepInfo,
                                   const ErrorObject& error_obj) const override;
bool PreCallValidateSetLatencyMarkerNV(VkDevice device, VkSwapchainKHR swapchain,
                                       const VkSetLatencyMarkerInfoNV* pLatencyMarkerInfo,
                                       const ErrorObject& error_obj) const override;
bool PreCallValidateGetLatencyTimingsNV(VkDevice device, VkSwapchainKHR swapchain, uint32_t* pTimingCount,
                                        VkGetLatencyMarkerInfoNV* pLatencyMarkerInfo, const ErrorObject& error_obj) const override;
bool PreCallValidateQueueNotifyOutOfBandNV(VkQueue queue, const VkOutOfBandQueueTypeInfoNV* pQueueTypeInfo,
                                           const ErrorObject& error_obj) const override;
bool PreCallValidateCmdSetAttachmentFeedbackLoopEnableEXT(VkCommandBuffer commandBuffer, VkImageAspectFlags aspectMask,
                                                          const ErrorObject& error_obj) const override;
#ifdef VK_USE_PLATFORM_SCREEN_QNX
bool PreCallValidateGetScreenBufferPropertiesQNX(VkDevice device, const struct _screen_buffer* buffer,
                                                 VkScreenBufferPropertiesQNX* pProperties,
                                                 const ErrorObject& error_obj) const override;
#endif  // VK_USE_PLATFORM_SCREEN_QNX
bool PreCallValidateCreateAccelerationStructureKHR(VkDevice device, const VkAccelerationStructureCreateInfoKHR* pCreateInfo,
                                                   const VkAllocationCallbacks* pAllocator,
                                                   VkAccelerationStructureKHR* pAccelerationStructure,
                                                   const ErrorObject& error_obj) const override;
bool PreCallValidateDestroyAccelerationStructureKHR(VkDevice device, VkAccelerationStructureKHR accelerationStructure,
                                                    const VkAllocationCallbacks* pAllocator,
                                                    const ErrorObject& error_obj) const override;
bool PreCallValidateCmdBuildAccelerationStructuresKHR(VkCommandBuffer commandBuffer, uint32_t infoCount,
                                                      const VkAccelerationStructureBuildGeometryInfoKHR* pInfos,
                                                      const VkAccelerationStructureBuildRangeInfoKHR* const* ppBuildRangeInfos,
                                                      const ErrorObject& error_obj) const override;
bool PreCallValidateCmdBuildAccelerationStructuresIndirectKHR(VkCommandBuffer commandBuffer, uint32_t infoCount,
                                                              const VkAccelerationStructureBuildGeometryInfoKHR* pInfos,
                                                              const VkDeviceAddress* pIndirectDeviceAddresses,
                                                              const uint32_t* pIndirectStrides,
                                                              const uint32_t* const* ppMaxPrimitiveCounts,
                                                              const ErrorObject& error_obj) const override;
bool PreCallValidateBuildAccelerationStructuresKHR(VkDevice device, VkDeferredOperationKHR deferredOperation, uint32_t infoCount,
                                                   const VkAccelerationStructureBuildGeometryInfoKHR* pInfos,
                                                   const VkAccelerationStructureBuildRangeInfoKHR* const* ppBuildRangeInfos,
                                                   const ErrorObject& error_obj) const override;
bool PreCallValidateCopyAccelerationStructureKHR(VkDevice device, VkDeferredOperationKHR deferredOperation,
                                                 const VkCopyAccelerationStructureInfoKHR* pInfo,
                                                 const ErrorObject& error_obj) const override;
bool PreCallValidateCopyAccelerationStructureToMemoryKHR(VkDevice device, VkDeferredOperationKHR deferredOperation,
                                                         const VkCopyAccelerationStructureToMemoryInfoKHR* pInfo,
                                                         const ErrorObject& error_obj) const override;
bool PreCallValidateCopyMemoryToAccelerationStructureKHR(VkDevice device, VkDeferredOperationKHR deferredOperation,
                                                         const VkCopyMemoryToAccelerationStructureInfoKHR* pInfo,
                                                         const ErrorObject& error_obj) const override;
bool PreCallValidateWriteAccelerationStructuresPropertiesKHR(VkDevice device, uint32_t accelerationStructureCount,
                                                             const VkAccelerationStructureKHR* pAccelerationStructures,
                                                             VkQueryType queryType, size_t dataSize, void* pData, size_t stride,
                                                             const ErrorObject& error_obj) const override;
bool PreCallValidateCmdCopyAccelerationStructureKHR(VkCommandBuffer commandBuffer, const VkCopyAccelerationStructureInfoKHR* pInfo,
                                                    const ErrorObject& error_obj) const override;
bool PreCallValidateCmdCopyAccelerationStructureToMemoryKHR(VkCommandBuffer commandBuffer,
                                                            const VkCopyAccelerationStructureToMemoryInfoKHR* pInfo,
                                                            const ErrorObject& error_obj) const override;
bool PreCallValidateCmdCopyMemoryToAccelerationStructureKHR(VkCommandBuffer commandBuffer,
                                                            const VkCopyMemoryToAccelerationStructureInfoKHR* pInfo,
                                                            const ErrorObject& error_obj) const override;
bool PreCallValidateGetAccelerationStructureDeviceAddressKHR(VkDevice device,
                                                             const VkAccelerationStructureDeviceAddressInfoKHR* pInfo,
                                                             const ErrorObject& error_obj) const override;
bool PreCallValidateCmdWriteAccelerationStructuresPropertiesKHR(VkCommandBuffer commandBuffer, uint32_t accelerationStructureCount,
                                                                const VkAccelerationStructureKHR* pAccelerationStructures,
                                                                VkQueryType queryType, VkQueryPool queryPool, uint32_t firstQuery,
                                                                const ErrorObject& error_obj) const override;
bool PreCallValidateGetDeviceAccelerationStructureCompatibilityKHR(VkDevice device,
                                                                   const VkAccelerationStructureVersionInfoKHR* pVersionInfo,
                                                                   VkAccelerationStructureCompatibilityKHR* pCompatibility,
                                                                   const ErrorObject& error_obj) const override;
bool PreCallValidateGetAccelerationStructureBuildSizesKHR(VkDevice device, VkAccelerationStructureBuildTypeKHR buildType,
                                                          const VkAccelerationStructureBuildGeometryInfoKHR* pBuildInfo,
                                                          const uint32_t* pMaxPrimitiveCounts,
                                                          VkAccelerationStructureBuildSizesInfoKHR* pSizeInfo,
                                                          const ErrorObject& error_obj) const override;
bool PreCallValidateCmdTraceRaysKHR(VkCommandBuffer commandBuffer, const VkStridedDeviceAddressRegionKHR* pRaygenShaderBindingTable,
                                    const VkStridedDeviceAddressRegionKHR* pMissShaderBindingTable,
                                    const VkStridedDeviceAddressRegionKHR* pHitShaderBindingTable,
                                    const VkStridedDeviceAddressRegionKHR* pCallableShaderBindingTable, uint32_t width,
                                    uint32_t height, uint32_t depth, const ErrorObject& error_obj) const override;
bool PreCallValidateCreateRayTracingPipelinesKHR(VkDevice device, VkDeferredOperationKHR deferredOperation,
                                                 VkPipelineCache pipelineCache, uint32_t createInfoCount,
                                                 const VkRayTracingPipelineCreateInfoKHR* pCreateInfos,
                                                 const VkAllocationCallbacks* pAllocator, VkPipeline* pPipelines,
                                                 const ErrorObject& error_obj) const override;
bool PreCallValidateGetRayTracingCaptureReplayShaderGroupHandlesKHR(VkDevice device, VkPipeline pipeline, uint32_t firstGroup,
                                                                    uint32_t groupCount, size_t dataSize, void* pData,
                                                                    const ErrorObject& error_obj) const override;
bool PreCallValidateCmdTraceRaysIndirectKHR(VkCommandBuffer commandBuffer,
                                            const VkStridedDeviceAddressRegionKHR* pRaygenShaderBindingTable,
                                            const VkStridedDeviceAddressRegionKHR* pMissShaderBindingTable,
                                            const VkStridedDeviceAddressRegionKHR* pHitShaderBindingTable,
                                            const VkStridedDeviceAddressRegionKHR* pCallableShaderBindingTable,
                                            VkDeviceAddress indirectDeviceAddress, const ErrorObject& error_obj) const override;
bool PreCallValidateGetRayTracingShaderGroupStackSizeKHR(VkDevice device, VkPipeline pipeline, uint32_t group,
                                                         VkShaderGroupShaderKHR groupShader,
                                                         const ErrorObject& error_obj) const override;
bool PreCallValidateCmdSetRayTracingPipelineStackSizeKHR(VkCommandBuffer commandBuffer, uint32_t pipelineStackSize,
                                                         const ErrorObject& error_obj) const override;
bool PreCallValidateCmdDrawMeshTasksEXT(VkCommandBuffer commandBuffer, uint32_t groupCountX, uint32_t groupCountY,
                                        uint32_t groupCountZ, const ErrorObject& error_obj) const override;
bool PreCallValidateCmdDrawMeshTasksIndirectEXT(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                uint32_t drawCount, uint32_t stride, const ErrorObject& error_obj) const override;
bool PreCallValidateCmdDrawMeshTasksIndirectCountEXT(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                     VkBuffer countBuffer, VkDeviceSize countBufferOffset, uint32_t maxDrawCount,
                                                     uint32_t stride, const ErrorObject& error_obj) const override;

// NOLINTEND
