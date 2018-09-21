bool PreCallValidateGetDeviceProcAddr(VkDevice device, const char* funcName);
bool PreCallValidateGetInstanceProcAddr(VkInstance instance, const char* funcName);
bool PreCallValidateCreateInstance(const VkInstanceCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator,
                                   VkInstance* pInstance);
bool PreCallValidateDestroyInstance(VkInstance instance, const VkAllocationCallbacks* pAllocator);
bool PreCallValidateEnumeratePhysicalDevices(VkInstance instance, uint32_t* pPhysicalDeviceCount,
                                             VkPhysicalDevice* pPhysicalDevices);
bool PreCallValidateGetPhysicalDeviceFeatures(VkPhysicalDevice physicalDevice, VkPhysicalDeviceFeatures* pFeatures);
bool PreCallValidateGetPhysicalDeviceFormatProperties(VkPhysicalDevice physicalDevice, VkFormat format,
                                                      VkFormatProperties* pFormatProperties);
bool PreCallValidateGetPhysicalDeviceImageFormatProperties(VkPhysicalDevice physicalDevice, VkFormat format, VkImageType type,
                                                           VkImageTiling tiling, VkImageUsageFlags usage, VkImageCreateFlags flags,
                                                           VkImageFormatProperties* pImageFormatProperties);
bool PreCallValidateGetPhysicalDeviceProperties(VkPhysicalDevice physicalDevice, VkPhysicalDeviceProperties* pProperties);
bool PreCallValidateGetPhysicalDeviceQueueFamilyProperties(VkPhysicalDevice physicalDevice, uint32_t* pQueueFamilyPropertyCount,
                                                           VkQueueFamilyProperties* pQueueFamilyProperties);
bool PreCallValidateGetPhysicalDeviceMemoryProperties(VkPhysicalDevice physicalDevice,
                                                      VkPhysicalDeviceMemoryProperties* pMemoryProperties);
bool PreCallValidateCreateDevice(VkPhysicalDevice physicalDevice, const VkDeviceCreateInfo* pCreateInfo,
                                 const VkAllocationCallbacks* pAllocator, VkDevice* pDevice);
bool PreCallValidateDestroyDevice(VkDevice device, const VkAllocationCallbacks* pAllocator);
bool PreCallValidateEnumerateInstanceExtensionProperties(const char* pLayerName, uint32_t* pPropertyCount,
                                                         VkExtensionProperties* pProperties);
bool PreCallValidateEnumerateDeviceExtensionProperties(VkPhysicalDevice physicalDevice, const char* pLayerName,
                                                       uint32_t* pPropertyCount, VkExtensionProperties* pProperties);
bool PreCallValidateEnumerateInstanceLayerProperties(uint32_t* pPropertyCount, VkLayerProperties* pProperties);
bool PreCallValidateEnumerateDeviceLayerProperties(VkPhysicalDevice physicalDevice, uint32_t* pPropertyCount,
                                                   VkLayerProperties* pProperties);
bool PreCallValidateGetDeviceQueue(VkDevice device, uint32_t queueFamilyIndex, uint32_t queueIndex, VkQueue* pQueue);
bool PreCallValidateQueueSubmit(VkQueue queue, uint32_t submitCount, const VkSubmitInfo* pSubmits, VkFence fence);
bool PreCallValidateQueueWaitIdle(VkQueue queue);
bool PreCallValidateDeviceWaitIdle(VkDevice device);
bool PreCallValidateAllocateMemory(VkDevice device, const VkMemoryAllocateInfo* pAllocateInfo,
                                   const VkAllocationCallbacks* pAllocator, VkDeviceMemory* pMemory);
bool PreCallValidateFreeMemory(VkDevice device, VkDeviceMemory memory, const VkAllocationCallbacks* pAllocator);
bool PreCallValidateMapMemory(VkDevice device, VkDeviceMemory memory, VkDeviceSize offset, VkDeviceSize size,
                              VkMemoryMapFlags flags, void** ppData);
bool PreCallValidateUnmapMemory(VkDevice device, VkDeviceMemory memory);
bool PreCallValidateFlushMappedMemoryRanges(VkDevice device, uint32_t memoryRangeCount, const VkMappedMemoryRange* pMemoryRanges);
bool PreCallValidateInvalidateMappedMemoryRanges(VkDevice device, uint32_t memoryRangeCount,
                                                 const VkMappedMemoryRange* pMemoryRanges);
bool PreCallValidateGetDeviceMemoryCommitment(VkDevice device, VkDeviceMemory memory, VkDeviceSize* pCommittedMemoryInBytes);
bool PreCallValidateBindBufferMemory(VkDevice device, VkBuffer buffer, VkDeviceMemory memory, VkDeviceSize memoryOffset);
bool PreCallValidateBindImageMemory(VkDevice device, VkImage image, VkDeviceMemory memory, VkDeviceSize memoryOffset);
bool PreCallValidateGetBufferMemoryRequirements(VkDevice device, VkBuffer buffer, VkMemoryRequirements* pMemoryRequirements);
bool PreCallValidateGetImageMemoryRequirements(VkDevice device, VkImage image, VkMemoryRequirements* pMemoryRequirements);
bool PreCallValidateGetImageSparseMemoryRequirements(VkDevice device, VkImage image, uint32_t* pSparseMemoryRequirementCount,
                                                     VkSparseImageMemoryRequirements* pSparseMemoryRequirements);
bool PreCallValidateGetPhysicalDeviceSparseImageFormatProperties(VkPhysicalDevice physicalDevice, VkFormat format, VkImageType type,
                                                                 VkSampleCountFlagBits samples, VkImageUsageFlags usage,
                                                                 VkImageTiling tiling, uint32_t* pPropertyCount,
                                                                 VkSparseImageFormatProperties* pProperties);
bool PreCallValidateQueueBindSparse(VkQueue queue, uint32_t bindInfoCount, const VkBindSparseInfo* pBindInfo, VkFence fence);
bool PreCallValidateCreateFence(VkDevice device, const VkFenceCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator,
                                VkFence* pFence);
bool PreCallValidateDestroyFence(VkDevice device, VkFence fence, const VkAllocationCallbacks* pAllocator);
bool PreCallValidateResetFences(VkDevice device, uint32_t fenceCount, const VkFence* pFences);
bool PreCallValidateGetFenceStatus(VkDevice device, VkFence fence);
bool PreCallValidateWaitForFences(VkDevice device, uint32_t fenceCount, const VkFence* pFences, VkBool32 waitAll, uint64_t timeout);
bool PreCallValidateCreateSemaphore(VkDevice device, const VkSemaphoreCreateInfo* pCreateInfo,
                                    const VkAllocationCallbacks* pAllocator, VkSemaphore* pSemaphore);
bool PreCallValidateDestroySemaphore(VkDevice device, VkSemaphore semaphore, const VkAllocationCallbacks* pAllocator);
bool PreCallValidateCreateEvent(VkDevice device, const VkEventCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator,
                                VkEvent* pEvent);
bool PreCallValidateDestroyEvent(VkDevice device, VkEvent event, const VkAllocationCallbacks* pAllocator);
bool PreCallValidateGetEventStatus(VkDevice device, VkEvent event);
bool PreCallValidateSetEvent(VkDevice device, VkEvent event);
bool PreCallValidateResetEvent(VkDevice device, VkEvent event);
bool PreCallValidateCreateQueryPool(VkDevice device, const VkQueryPoolCreateInfo* pCreateInfo,
                                    const VkAllocationCallbacks* pAllocator, VkQueryPool* pQueryPool);
bool PreCallValidateDestroyQueryPool(VkDevice device, VkQueryPool queryPool, const VkAllocationCallbacks* pAllocator);
bool PreCallValidateGetQueryPoolResults(VkDevice device, VkQueryPool queryPool, uint32_t firstQuery, uint32_t queryCount,
                                        size_t dataSize, void* pData, VkDeviceSize stride, VkQueryResultFlags flags);
bool PreCallValidateCreateBuffer(VkDevice device, const VkBufferCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator,
                                 VkBuffer* pBuffer);
bool PreCallValidateDestroyBuffer(VkDevice device, VkBuffer buffer, const VkAllocationCallbacks* pAllocator);
bool PreCallValidateCreateBufferView(VkDevice device, const VkBufferViewCreateInfo* pCreateInfo,
                                     const VkAllocationCallbacks* pAllocator, VkBufferView* pView);
bool PreCallValidateDestroyBufferView(VkDevice device, VkBufferView bufferView, const VkAllocationCallbacks* pAllocator);
bool PreCallValidateCreateImage(VkDevice device, const VkImageCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator,
                                VkImage* pImage);
bool PreCallValidateDestroyImage(VkDevice device, VkImage image, const VkAllocationCallbacks* pAllocator);
bool PreCallValidateGetImageSubresourceLayout(VkDevice device, VkImage image, const VkImageSubresource* pSubresource,
                                              VkSubresourceLayout* pLayout);
bool PreCallValidateCreateImageView(VkDevice device, const VkImageViewCreateInfo* pCreateInfo,
                                    const VkAllocationCallbacks* pAllocator, VkImageView* pView);
bool PreCallValidateDestroyImageView(VkDevice device, VkImageView imageView, const VkAllocationCallbacks* pAllocator);
bool PreCallValidateCreateShaderModule(VkDevice device, const VkShaderModuleCreateInfo* pCreateInfo,
                                       const VkAllocationCallbacks* pAllocator, VkShaderModule* pShaderModule);
bool PreCallValidateDestroyShaderModule(VkDevice device, VkShaderModule shaderModule, const VkAllocationCallbacks* pAllocator);
bool PreCallValidateCreatePipelineCache(VkDevice device, const VkPipelineCacheCreateInfo* pCreateInfo,
                                        const VkAllocationCallbacks* pAllocator, VkPipelineCache* pPipelineCache);
bool PreCallValidateDestroyPipelineCache(VkDevice device, VkPipelineCache pipelineCache, const VkAllocationCallbacks* pAllocator);
bool PreCallValidateGetPipelineCacheData(VkDevice device, VkPipelineCache pipelineCache, size_t* pDataSize, void* pData);
bool PreCallValidateMergePipelineCaches(VkDevice device, VkPipelineCache dstCache, uint32_t srcCacheCount,
                                        const VkPipelineCache* pSrcCaches);
bool PreCallValidateCreateGraphicsPipelines(VkDevice device, VkPipelineCache pipelineCache, uint32_t createInfoCount,
                                            const VkGraphicsPipelineCreateInfo* pCreateInfos,
                                            const VkAllocationCallbacks* pAllocator, VkPipeline* pPipelines);
bool PreCallValidateCreateComputePipelines(VkDevice device, VkPipelineCache pipelineCache, uint32_t createInfoCount,
                                           const VkComputePipelineCreateInfo* pCreateInfos, const VkAllocationCallbacks* pAllocator,
                                           VkPipeline* pPipelines);
bool PreCallValidateDestroyPipeline(VkDevice device, VkPipeline pipeline, const VkAllocationCallbacks* pAllocator);
bool PreCallValidateCreatePipelineLayout(VkDevice device, const VkPipelineLayoutCreateInfo* pCreateInfo,
                                         const VkAllocationCallbacks* pAllocator, VkPipelineLayout* pPipelineLayout);
bool PreCallValidateDestroyPipelineLayout(VkDevice device, VkPipelineLayout pipelineLayout,
                                          const VkAllocationCallbacks* pAllocator);
bool PreCallValidateCreateSampler(VkDevice device, const VkSamplerCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator,
                                  VkSampler* pSampler);
bool PreCallValidateDestroySampler(VkDevice device, VkSampler sampler, const VkAllocationCallbacks* pAllocator);
bool PreCallValidateCreateDescriptorSetLayout(VkDevice device, const VkDescriptorSetLayoutCreateInfo* pCreateInfo,
                                              const VkAllocationCallbacks* pAllocator, VkDescriptorSetLayout* pSetLayout);
bool PreCallValidateDestroyDescriptorSetLayout(VkDevice device, VkDescriptorSetLayout descriptorSetLayout,
                                               const VkAllocationCallbacks* pAllocator);
bool PreCallValidateCreateDescriptorPool(VkDevice device, const VkDescriptorPoolCreateInfo* pCreateInfo,
                                         const VkAllocationCallbacks* pAllocator, VkDescriptorPool* pDescriptorPool);
bool PreCallValidateDestroyDescriptorPool(VkDevice device, VkDescriptorPool descriptorPool,
                                          const VkAllocationCallbacks* pAllocator);
bool PreCallValidateResetDescriptorPool(VkDevice device, VkDescriptorPool descriptorPool, VkDescriptorPoolResetFlags flags);
bool PreCallValidateAllocateDescriptorSets(VkDevice device, const VkDescriptorSetAllocateInfo* pAllocateInfo,
                                           VkDescriptorSet* pDescriptorSets);
bool PreCallValidateFreeDescriptorSets(VkDevice device, VkDescriptorPool descriptorPool, uint32_t descriptorSetCount,
                                       const VkDescriptorSet* pDescriptorSets);
bool PreCallValidateUpdateDescriptorSets(VkDevice device, uint32_t descriptorWriteCount,
                                         const VkWriteDescriptorSet* pDescriptorWrites, uint32_t descriptorCopyCount,
                                         const VkCopyDescriptorSet* pDescriptorCopies);
bool PreCallValidateCreateFramebuffer(VkDevice device, const VkFramebufferCreateInfo* pCreateInfo,
                                      const VkAllocationCallbacks* pAllocator, VkFramebuffer* pFramebuffer);
bool PreCallValidateDestroyFramebuffer(VkDevice device, VkFramebuffer framebuffer, const VkAllocationCallbacks* pAllocator);
bool PreCallValidateCreateRenderPass(VkDevice device, const VkRenderPassCreateInfo* pCreateInfo,
                                     const VkAllocationCallbacks* pAllocator, VkRenderPass* pRenderPass);
bool PreCallValidateDestroyRenderPass(VkDevice device, VkRenderPass renderPass, const VkAllocationCallbacks* pAllocator);
bool PreCallValidateGetRenderAreaGranularity(VkDevice device, VkRenderPass renderPass, VkExtent2D* pGranularity);
bool PreCallValidateCreateCommandPool(VkDevice device, const VkCommandPoolCreateInfo* pCreateInfo,
                                      const VkAllocationCallbacks* pAllocator, VkCommandPool* pCommandPool);
bool PreCallValidateDestroyCommandPool(VkDevice device, VkCommandPool commandPool, const VkAllocationCallbacks* pAllocator);
bool PreCallValidateResetCommandPool(VkDevice device, VkCommandPool commandPool, VkCommandPoolResetFlags flags);
bool PreCallValidateAllocateCommandBuffers(VkDevice device, const VkCommandBufferAllocateInfo* pAllocateInfo,
                                           VkCommandBuffer* pCommandBuffers);
bool PreCallValidateFreeCommandBuffers(VkDevice device, VkCommandPool commandPool, uint32_t commandBufferCount,
                                       const VkCommandBuffer* pCommandBuffers);
bool PreCallValidateBeginCommandBuffer(VkCommandBuffer commandBuffer, const VkCommandBufferBeginInfo* pBeginInfo);
bool PreCallValidateEndCommandBuffer(VkCommandBuffer commandBuffer);
bool PreCallValidateResetCommandBuffer(VkCommandBuffer commandBuffer, VkCommandBufferResetFlags flags);
bool PreCallValidateCmdBindPipeline(VkCommandBuffer commandBuffer, VkPipelineBindPoint pipelineBindPoint, VkPipeline pipeline);
bool PreCallValidateCmdSetViewport(VkCommandBuffer commandBuffer, uint32_t firstViewport, uint32_t viewportCount,
                                   const VkViewport* pViewports);
bool PreCallValidateCmdSetScissor(VkCommandBuffer commandBuffer, uint32_t firstScissor, uint32_t scissorCount,
                                  const VkRect2D* pScissors);
bool PreCallValidateCmdSetLineWidth(VkCommandBuffer commandBuffer, float lineWidth);
bool PreCallValidateCmdSetDepthBias(VkCommandBuffer commandBuffer, float depthBiasConstantFactor, float depthBiasClamp,
                                    float depthBiasSlopeFactor);
bool PreCallValidateCmdSetBlendConstants(VkCommandBuffer commandBuffer, const float blendConstants[4]);
bool PreCallValidateCmdSetDepthBounds(VkCommandBuffer commandBuffer, float minDepthBounds, float maxDepthBounds);
bool PreCallValidateCmdSetStencilCompareMask(VkCommandBuffer commandBuffer, VkStencilFaceFlags faceMask, uint32_t compareMask);
bool PreCallValidateCmdSetStencilWriteMask(VkCommandBuffer commandBuffer, VkStencilFaceFlags faceMask, uint32_t writeMask);
bool PreCallValidateCmdSetStencilReference(VkCommandBuffer commandBuffer, VkStencilFaceFlags faceMask, uint32_t reference);
bool PreCallValidateCmdBindDescriptorSets(VkCommandBuffer commandBuffer, VkPipelineBindPoint pipelineBindPoint,
                                          VkPipelineLayout layout, uint32_t firstSet, uint32_t descriptorSetCount,
                                          const VkDescriptorSet* pDescriptorSets, uint32_t dynamicOffsetCount,
                                          const uint32_t* pDynamicOffsets);
bool PreCallValidateCmdBindIndexBuffer(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset, VkIndexType indexType);
bool PreCallValidateCmdBindVertexBuffers(VkCommandBuffer commandBuffer, uint32_t firstBinding, uint32_t bindingCount,
                                         const VkBuffer* pBuffers, const VkDeviceSize* pOffsets);
bool PreCallValidateCmdDraw(VkCommandBuffer commandBuffer, uint32_t vertexCount, uint32_t instanceCount, uint32_t firstVertex,
                            uint32_t firstInstance);
bool PreCallValidateCmdDrawIndexed(VkCommandBuffer commandBuffer, uint32_t indexCount, uint32_t instanceCount, uint32_t firstIndex,
                                   int32_t vertexOffset, uint32_t firstInstance);
bool PreCallValidateCmdDrawIndirect(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset, uint32_t drawCount,
                                    uint32_t stride);
bool PreCallValidateCmdDrawIndexedIndirect(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset, uint32_t drawCount,
                                           uint32_t stride);
bool PreCallValidateCmdDispatch(VkCommandBuffer commandBuffer, uint32_t groupCountX, uint32_t groupCountY, uint32_t groupCountZ);
bool PreCallValidateCmdDispatchIndirect(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset);
bool PreCallValidateCmdCopyBuffer(VkCommandBuffer commandBuffer, VkBuffer srcBuffer, VkBuffer dstBuffer, uint32_t regionCount,
                                  const VkBufferCopy* pRegions);
bool PreCallValidateCmdCopyImage(VkCommandBuffer commandBuffer, VkImage srcImage, VkImageLayout srcImageLayout, VkImage dstImage,
                                 VkImageLayout dstImageLayout, uint32_t regionCount, const VkImageCopy* pRegions);
bool PreCallValidateCmdBlitImage(VkCommandBuffer commandBuffer, VkImage srcImage, VkImageLayout srcImageLayout, VkImage dstImage,
                                 VkImageLayout dstImageLayout, uint32_t regionCount, const VkImageBlit* pRegions, VkFilter filter);
bool PreCallValidateCmdCopyBufferToImage(VkCommandBuffer commandBuffer, VkBuffer srcBuffer, VkImage dstImage,
                                         VkImageLayout dstImageLayout, uint32_t regionCount, const VkBufferImageCopy* pRegions);
bool PreCallValidateCmdCopyImageToBuffer(VkCommandBuffer commandBuffer, VkImage srcImage, VkImageLayout srcImageLayout,
                                         VkBuffer dstBuffer, uint32_t regionCount, const VkBufferImageCopy* pRegions);
bool PreCallValidateCmdUpdateBuffer(VkCommandBuffer commandBuffer, VkBuffer dstBuffer, VkDeviceSize dstOffset,
                                    VkDeviceSize dataSize, const void* pData);
bool PreCallValidateCmdFillBuffer(VkCommandBuffer commandBuffer, VkBuffer dstBuffer, VkDeviceSize dstOffset, VkDeviceSize size,
                                  uint32_t data);
bool PreCallValidateCmdClearColorImage(VkCommandBuffer commandBuffer, VkImage image, VkImageLayout imageLayout,
                                       const VkClearColorValue* pColor, uint32_t rangeCount,
                                       const VkImageSubresourceRange* pRanges);
bool PreCallValidateCmdClearDepthStencilImage(VkCommandBuffer commandBuffer, VkImage image, VkImageLayout imageLayout,
                                              const VkClearDepthStencilValue* pDepthStencil, uint32_t rangeCount,
                                              const VkImageSubresourceRange* pRanges);
bool PreCallValidateCmdClearAttachments(VkCommandBuffer commandBuffer, uint32_t attachmentCount,
                                        const VkClearAttachment* pAttachments, uint32_t rectCount, const VkClearRect* pRects);
bool PreCallValidateCmdResolveImage(VkCommandBuffer commandBuffer, VkImage srcImage, VkImageLayout srcImageLayout, VkImage dstImage,
                                    VkImageLayout dstImageLayout, uint32_t regionCount, const VkImageResolve* pRegions);
bool PreCallValidateCmdSetEvent(VkCommandBuffer commandBuffer, VkEvent event, VkPipelineStageFlags stageMask);
bool PreCallValidateCmdResetEvent(VkCommandBuffer commandBuffer, VkEvent event, VkPipelineStageFlags stageMask);
bool PreCallValidateCmdWaitEvents(VkCommandBuffer commandBuffer, uint32_t eventCount, const VkEvent* pEvents,
                                  VkPipelineStageFlags srcStageMask, VkPipelineStageFlags dstStageMask, uint32_t memoryBarrierCount,
                                  const VkMemoryBarrier* pMemoryBarriers, uint32_t bufferMemoryBarrierCount,
                                  const VkBufferMemoryBarrier* pBufferMemoryBarriers, uint32_t imageMemoryBarrierCount,
                                  const VkImageMemoryBarrier* pImageMemoryBarriers);
bool PreCallValidateCmdPipelineBarrier(VkCommandBuffer commandBuffer, VkPipelineStageFlags srcStageMask,
                                       VkPipelineStageFlags dstStageMask, VkDependencyFlags dependencyFlags,
                                       uint32_t memoryBarrierCount, const VkMemoryBarrier* pMemoryBarriers,
                                       uint32_t bufferMemoryBarrierCount, const VkBufferMemoryBarrier* pBufferMemoryBarriers,
                                       uint32_t imageMemoryBarrierCount, const VkImageMemoryBarrier* pImageMemoryBarriers);
bool PreCallValidateCmdBeginQuery(VkCommandBuffer commandBuffer, VkQueryPool queryPool, uint32_t query, VkQueryControlFlags flags);
bool PreCallValidateCmdEndQuery(VkCommandBuffer commandBuffer, VkQueryPool queryPool, uint32_t query);
bool PreCallValidateCmdResetQueryPool(VkCommandBuffer commandBuffer, VkQueryPool queryPool, uint32_t firstQuery,
                                      uint32_t queryCount);
bool PreCallValidateCmdWriteTimestamp(VkCommandBuffer commandBuffer, VkPipelineStageFlagBits pipelineStage, VkQueryPool queryPool,
                                      uint32_t query);
bool PreCallValidateCmdCopyQueryPoolResults(VkCommandBuffer commandBuffer, VkQueryPool queryPool, uint32_t firstQuery,
                                            uint32_t queryCount, VkBuffer dstBuffer, VkDeviceSize dstOffset, VkDeviceSize stride,
                                            VkQueryResultFlags flags);
bool PreCallValidateCmdPushConstants(VkCommandBuffer commandBuffer, VkPipelineLayout layout, VkShaderStageFlags stageFlags,
                                     uint32_t offset, uint32_t size, const void* pValues);
bool PreCallValidateCmdBeginRenderPass(VkCommandBuffer commandBuffer, const VkRenderPassBeginInfo* pRenderPassBegin,
                                       VkSubpassContents contents);
bool PreCallValidateCmdBeginRenderPass2KHR(VkCommandBuffer commandBuffer, const VkRenderPassBeginInfo* pRenderPassBegin,
                                           const VkSubpassBeginInfoKHR* pSubpassBeginInfo);
bool PreCallValidateCmdNextSubpass(VkCommandBuffer commandBuffer, VkSubpassContents contents);
bool PreCallValidateCmdEndRenderPass(VkCommandBuffer commandBuffer);
bool PreCallValidateCmdEndRenderPass2KHR(VkCommandBuffer commandBuffer, const VkSubpassEndInfoKHR* pSubpassEndInfo);
bool PreCallValidateCmdExecuteCommands(VkCommandBuffer commandBuffer, uint32_t commandBufferCount,
                                       const VkCommandBuffer* pCommandBuffers);
bool PreCallValidateBindBufferMemory2(VkDevice device, uint32_t bindInfoCount, const VkBindBufferMemoryInfo* pBindInfos);
bool PreCallValidateBindImageMemory2(VkDevice device, uint32_t bindInfoCount, const VkBindImageMemoryInfo* pBindInfos);
bool PreCallValidateGetDeviceGroupPeerMemoryFeatures(VkDevice device, uint32_t heapIndex, uint32_t localDeviceIndex,
                                                     uint32_t remoteDeviceIndex, VkPeerMemoryFeatureFlags* pPeerMemoryFeatures);
bool PreCallValidateCmdSetDeviceMask(VkCommandBuffer commandBuffer, uint32_t deviceMask);
bool PreCallValidateCmdDispatchBase(VkCommandBuffer commandBuffer, uint32_t baseGroupX, uint32_t baseGroupY, uint32_t baseGroupZ,
                                    uint32_t groupCountX, uint32_t groupCountY, uint32_t groupCountZ);
bool PreCallValidateEnumeratePhysicalDeviceGroups(VkInstance instance, uint32_t* pPhysicalDeviceGroupCount,
                                                  VkPhysicalDeviceGroupProperties* pPhysicalDeviceGroupProperties);
bool PreCallValidateGetImageMemoryRequirements2(VkDevice device, const VkImageMemoryRequirementsInfo2* pInfo,
                                                VkMemoryRequirements2* pMemoryRequirements);
bool PreCallValidateGetBufferMemoryRequirements2(VkDevice device, const VkBufferMemoryRequirementsInfo2* pInfo,
                                                 VkMemoryRequirements2* pMemoryRequirements);
bool PreCallValidateGetImageSparseMemoryRequirements2(VkDevice device, const VkImageSparseMemoryRequirementsInfo2* pInfo,
                                                      uint32_t* pSparseMemoryRequirementCount,
                                                      VkSparseImageMemoryRequirements2* pSparseMemoryRequirements);
bool PreCallValidateGetPhysicalDeviceFeatures2(VkPhysicalDevice physicalDevice, VkPhysicalDeviceFeatures2* pFeatures);
bool PreCallValidateGetPhysicalDeviceProperties2(VkPhysicalDevice physicalDevice, VkPhysicalDeviceProperties2* pProperties);
bool PreCallValidateGetPhysicalDeviceFormatProperties2(VkPhysicalDevice physicalDevice, VkFormat format,
                                                       VkFormatProperties2* pFormatProperties);
bool PreCallValidateGetPhysicalDeviceImageFormatProperties2(VkPhysicalDevice physicalDevice,
                                                            const VkPhysicalDeviceImageFormatInfo2* pImageFormatInfo,
                                                            VkImageFormatProperties2* pImageFormatProperties);
bool PreCallValidateGetPhysicalDeviceQueueFamilyProperties2(VkPhysicalDevice physicalDevice, uint32_t* pQueueFamilyPropertyCount,
                                                            VkQueueFamilyProperties2* pQueueFamilyProperties);
bool PreCallValidateGetPhysicalDeviceMemoryProperties2(VkPhysicalDevice physicalDevice,
                                                       VkPhysicalDeviceMemoryProperties2* pMemoryProperties);
bool PreCallValidateGetPhysicalDeviceSparseImageFormatProperties2(VkPhysicalDevice physicalDevice,
                                                                  const VkPhysicalDeviceSparseImageFormatInfo2* pFormatInfo,
                                                                  uint32_t* pPropertyCount,
                                                                  VkSparseImageFormatProperties2* pProperties);
bool PreCallValidateTrimCommandPool(VkDevice device, VkCommandPool commandPool, VkCommandPoolTrimFlags flags);
bool PreCallValidateGetDeviceQueue2(VkDevice device, const VkDeviceQueueInfo2* pQueueInfo, VkQueue* pQueue);
bool PreCallValidateCreateSamplerYcbcrConversion(VkDevice device, const VkSamplerYcbcrConversionCreateInfo* pCreateInfo,
                                                 const VkAllocationCallbacks* pAllocator,
                                                 VkSamplerYcbcrConversion* pYcbcrConversion);
bool PreCallValidateDestroySamplerYcbcrConversion(VkDevice device, VkSamplerYcbcrConversion ycbcrConversion,
                                                  const VkAllocationCallbacks* pAllocator);
bool PreCallValidateCreateDescriptorUpdateTemplate(VkDevice device, const VkDescriptorUpdateTemplateCreateInfo* pCreateInfo,
                                                   const VkAllocationCallbacks* pAllocator,
                                                   VkDescriptorUpdateTemplate* pDescriptorUpdateTemplate);
bool PreCallValidateDestroyDescriptorUpdateTemplate(VkDevice device, VkDescriptorUpdateTemplate descriptorUpdateTemplate,
                                                    const VkAllocationCallbacks* pAllocator);
bool PreCallValidateUpdateDescriptorSetWithTemplate(VkDevice device, VkDescriptorSet descriptorSet,
                                                    VkDescriptorUpdateTemplate descriptorUpdateTemplate, const void* pData);
bool PreCallValidateGetPhysicalDeviceExternalBufferProperties(VkPhysicalDevice physicalDevice,
                                                              const VkPhysicalDeviceExternalBufferInfo* pExternalBufferInfo,
                                                              VkExternalBufferProperties* pExternalBufferProperties);
bool PreCallValidateGetPhysicalDeviceExternalFenceProperties(VkPhysicalDevice physicalDevice,
                                                             const VkPhysicalDeviceExternalFenceInfo* pExternalFenceInfo,
                                                             VkExternalFenceProperties* pExternalFenceProperties);
bool PreCallValidateGetPhysicalDeviceExternalSemaphoreProperties(
    VkPhysicalDevice physicalDevice, const VkPhysicalDeviceExternalSemaphoreInfo* pExternalSemaphoreInfo,
    VkExternalSemaphoreProperties* pExternalSemaphoreProperties);
bool PreCallValidateGetDescriptorSetLayoutSupport(VkDevice device, const VkDescriptorSetLayoutCreateInfo* pCreateInfo,
                                                  VkDescriptorSetLayoutSupport* pSupport);
bool PreCallValidateDestroySurfaceKHR(VkInstance instance, VkSurfaceKHR surface, const VkAllocationCallbacks* pAllocator);
bool PreCallValidateGetPhysicalDeviceSurfaceSupportKHR(VkPhysicalDevice physicalDevice, uint32_t queueFamilyIndex,
                                                       VkSurfaceKHR surface, VkBool32* pSupported);
bool PreCallValidateGetPhysicalDeviceSurfaceCapabilitiesKHR(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface,
                                                            VkSurfaceCapabilitiesKHR* pSurfaceCapabilities);
bool PreCallValidateGetPhysicalDeviceSurfaceFormatsKHR(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface,
                                                       uint32_t* pSurfaceFormatCount, VkSurfaceFormatKHR* pSurfaceFormats);
bool PreCallValidateGetPhysicalDeviceSurfacePresentModesKHR(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface,
                                                            uint32_t* pPresentModeCount, VkPresentModeKHR* pPresentModes);
bool PreCallValidateCreateSwapchainKHR(VkDevice device, const VkSwapchainCreateInfoKHR* pCreateInfo,
                                       const VkAllocationCallbacks* pAllocator, VkSwapchainKHR* pSwapchain);
bool PreCallValidateDestroySwapchainKHR(VkDevice device, VkSwapchainKHR swapchain, const VkAllocationCallbacks* pAllocator);
bool PreCallValidateGetSwapchainImagesKHR(VkDevice device, VkSwapchainKHR swapchain, uint32_t* pSwapchainImageCount,
                                          VkImage* pSwapchainImages);
bool PreCallValidateAcquireNextImageKHR(VkDevice device, VkSwapchainKHR swapchain, uint64_t timeout, VkSemaphore semaphore,
                                        VkFence fence, uint32_t* pImageIndex);
bool PreCallValidateQueuePresentKHR(VkQueue queue, const VkPresentInfoKHR* pPresentInfo);
bool PreCallValidateGetDeviceGroupPresentCapabilitiesKHR(VkDevice device,
                                                         VkDeviceGroupPresentCapabilitiesKHR* pDeviceGroupPresentCapabilities);
bool PreCallValidateGetDeviceGroupSurfacePresentModesKHR(VkDevice device, VkSurfaceKHR surface,
                                                         VkDeviceGroupPresentModeFlagsKHR* pModes);
bool PreCallValidateGetPhysicalDevicePresentRectanglesKHR(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface,
                                                          uint32_t* pRectCount, VkRect2D* pRects);
bool PreCallValidateAcquireNextImage2KHR(VkDevice device, const VkAcquireNextImageInfoKHR* pAcquireInfo, uint32_t* pImageIndex);
bool PreCallValidateGetPhysicalDeviceDisplayPropertiesKHR(VkPhysicalDevice physicalDevice, uint32_t* pPropertyCount,
                                                          VkDisplayPropertiesKHR* pProperties);
bool PreCallValidateGetPhysicalDeviceDisplayPlanePropertiesKHR(VkPhysicalDevice physicalDevice, uint32_t* pPropertyCount,
                                                               VkDisplayPlanePropertiesKHR* pProperties);
bool PreCallValidateGetDisplayPlaneSupportedDisplaysKHR(VkPhysicalDevice physicalDevice, uint32_t planeIndex,
                                                        uint32_t* pDisplayCount, VkDisplayKHR* pDisplays);
bool PreCallValidateGetDisplayModePropertiesKHR(VkPhysicalDevice physicalDevice, VkDisplayKHR display, uint32_t* pPropertyCount,
                                                VkDisplayModePropertiesKHR* pProperties);
bool PreCallValidateCreateDisplayModeKHR(VkPhysicalDevice physicalDevice, VkDisplayKHR display,
                                         const VkDisplayModeCreateInfoKHR* pCreateInfo, const VkAllocationCallbacks* pAllocator,
                                         VkDisplayModeKHR* pMode);
bool PreCallValidateGetDisplayPlaneCapabilitiesKHR(VkPhysicalDevice physicalDevice, VkDisplayModeKHR mode, uint32_t planeIndex,
                                                   VkDisplayPlaneCapabilitiesKHR* pCapabilities);
bool PreCallValidateCreateDisplayPlaneSurfaceKHR(VkInstance instance, const VkDisplaySurfaceCreateInfoKHR* pCreateInfo,
                                                 const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface);
bool PreCallValidateCreateSharedSwapchainsKHR(VkDevice device, uint32_t swapchainCount,
                                              const VkSwapchainCreateInfoKHR* pCreateInfos, const VkAllocationCallbacks* pAllocator,
                                              VkSwapchainKHR* pSwapchains);
#ifdef VK_USE_PLATFORM_XLIB_KHR
bool PreCallValidateCreateXlibSurfaceKHR(VkInstance instance, const VkXlibSurfaceCreateInfoKHR* pCreateInfo,
                                         const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface);
bool PreCallValidateGetPhysicalDeviceXlibPresentationSupportKHR(VkPhysicalDevice physicalDevice, uint32_t queueFamilyIndex,
                                                                Display* dpy, VisualID visualID);
void PreCallRecordCreateXlibSurfaceKHR(VkInstance instance, const VkXlibSurfaceCreateInfoKHR* pCreateInfo,
                                       const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface);
void PreCallRecordGetPhysicalDeviceXlibPresentationSupportKHR(VkPhysicalDevice physicalDevice, uint32_t queueFamilyIndex,
                                                              Display* dpy, VisualID visualID);
#endif
#ifdef VK_USE_PLATFORM_XLIB_XRANDR_EXT
bool PreCallValidateAcquireXlibDisplayEXT(VkPhysicalDevice physicalDevice, Display* dpy, VkDisplayKHR display);
void PreCallRecordAcquireXlibDisplayEXT(VkPhysicalDevice physicalDevice, Display* dpy, VkDisplayKHR display);
bool PreCallValidateGetRandROutputDisplayEXT(VkPhysicalDevice physicalDevice, Display* dpy, RROutput rrOutput,
                                             VkDisplayKHR* pDisplay);
void PreCallRecordGetRandROutputDisplayEXT(VkPhysicalDevice physicalDevice, Display* dpy, RROutput rrOutput,
                                           VkDisplayKHR* pDisplay);
#endif
#ifdef VK_USE_PLATFORM_XCB_KHR
bool PreCallValidateCreateXcbSurfaceKHR(VkInstance instance, const VkXcbSurfaceCreateInfoKHR* pCreateInfo,
                                        const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface);
bool PreCallValidateGetPhysicalDeviceXcbPresentationSupportKHR(VkPhysicalDevice physicalDevice, uint32_t queueFamilyIndex,
                                                               xcb_connection_t* connection, xcb_visualid_t visual_id);
void PreCallRecordCreateXcbSurfaceKHR(VkInstance instance, const VkXcbSurfaceCreateInfoKHR* pCreateInfo,
                                      const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface);
void PreCallRecordGetPhysicalDeviceXcbPresentationSupportKHR(VkPhysicalDevice physicalDevice, uint32_t queueFamilyIndex,
                                                             xcb_connection_t* connection, xcb_visualid_t visual_id);
#endif
#ifdef VK_USE_PLATFORM_WAYLAND_KHR
bool PreCallValidateCreateWaylandSurfaceKHR(VkInstance instance, const VkWaylandSurfaceCreateInfoKHR* pCreateInfo,
                                            const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface);
bool PreCallValidateGetPhysicalDeviceWaylandPresentationSupportKHR(VkPhysicalDevice physicalDevice, uint32_t queueFamilyIndex,
                                                                   struct wl_display* display);
void PreCallRecordCreateWaylandSurfaceKHR(VkInstance instance, const VkWaylandSurfaceCreateInfoKHR* pCreateInfo,
                                          const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface);
void PreCallRecordGetPhysicalDeviceWaylandPresentationSupportKHR(VkPhysicalDevice physicalDevice, uint32_t queueFamilyIndex,
                                                                 struct wl_display* display);
#endif
#ifdef VK_USE_PLATFORM_MIR_KHR
bool PreCallValidateCreateMirSurfaceKHR(VkInstance instance, const VkMirSurfaceCreateInfoKHR* pCreateInfo,
                                        const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface);
bool PreCallValidateGetPhysicalDeviceMirPresentationSupportKHR(VkPhysicalDevice physicalDevice, uint32_t queueFamilyIndex,
                                                               MirConnection* connection);
void PreCallRecordCreateMirSurfaceKHR(VkInstance instance, const VkMirSurfaceCreateInfoKHR* pCreateInfo,
                                      const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface);
void PreCallRecordGetPhysicalDeviceMirPresentationSupportKHR(VkPhysicalDevice physicalDevice, uint32_t queueFamilyIndex,
                                                             MirConnection* connection);
#endif
#ifdef VK_USE_PLATFORM_ANDROID_KHR
bool PreCallValidateCreateAndroidSurfaceKHR(VkInstance instance, const VkAndroidSurfaceCreateInfoKHR* pCreateInfo,
                                            const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface);
bool PreCallValidateGetAndroidHardwareBufferPropertiesANDROID(VkDevice device, const struct AHardwareBuffer* buffer,
                                                              VkAndroidHardwareBufferPropertiesANDROID* pProperties);
bool PreCallValidateGetMemoryAndroidHardwareBufferANDROID(VkDevice device, const VkMemoryGetAndroidHardwareBufferInfoANDROID* pInfo,
                                                          struct AHardwareBuffer** pBuffer);
void PreCallRecordGetAndroidHardwareBufferPropertiesANDROID(VkDevice device, const struct AHardwareBuffer* buffer,
                                                            VkAndroidHardwareBufferPropertiesANDROID* pProperties);
void PreCallRecordGetMemoryAndroidHardwareBufferANDROID(VkDevice device, const VkMemoryGetAndroidHardwareBufferInfoANDROID* pInfo,
                                                        struct AHardwareBuffer** pBuffer);
void PreCallRecordCreateAndroidSurfaceKHR(VkInstance instance, const VkAndroidSurfaceCreateInfoKHR* pCreateInfo,
                                          const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface);
#endif
#ifdef VK_USE_PLATFORM_WIN32_KHR
bool PreCallValidateCreateWin32SurfaceKHR(VkInstance instance, const VkWin32SurfaceCreateInfoKHR* pCreateInfo,
                                          const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface);
bool PreCallValidateGetPhysicalDeviceWin32PresentationSupportKHR(VkPhysicalDevice physicalDevice, uint32_t queueFamilyIndex);
bool PreCallValidateGetMemoryWin32HandleKHR(VkDevice device, const VkMemoryGetWin32HandleInfoKHR* pGetWin32HandleInfo,
                                            HANDLE* pHandle);
bool PreCallValidateGetMemoryWin32HandlePropertiesKHR(VkDevice device, VkExternalMemoryHandleTypeFlagBits handleType, HANDLE handle,
                                                      VkMemoryWin32HandlePropertiesKHR* pMemoryWin32HandleProperties);
bool PreCallValidateImportSemaphoreWin32HandleKHR(VkDevice device,
                                                  const VkImportSemaphoreWin32HandleInfoKHR* pImportSemaphoreWin32HandleInfo);
bool PreCallValidateGetSemaphoreWin32HandleKHR(VkDevice device, const VkSemaphoreGetWin32HandleInfoKHR* pGetWin32HandleInfo,
                                               HANDLE* pHandle);
bool PreCallValidateImportFenceWin32HandleKHR(VkDevice device, const VkImportFenceWin32HandleInfoKHR* pImportFenceWin32HandleInfo);
bool PreCallValidateGetFenceWin32HandleKHR(VkDevice device, const VkFenceGetWin32HandleInfoKHR* pGetWin32HandleInfo,
                                           HANDLE* pHandle);
bool PreCallValidateGetMemoryWin32HandleNV(VkDevice device, VkDeviceMemory memory, VkExternalMemoryHandleTypeFlagsNV handleType,
                                           HANDLE* pHandle);
void PreCallRecordCreateWin32SurfaceKHR(VkInstance instance, const VkWin32SurfaceCreateInfoKHR* pCreateInfo,
                                        const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface);
void PreCallRecordGetPhysicalDeviceWin32PresentationSupportKHR(VkPhysicalDevice physicalDevice, uint32_t queueFamilyIndex);
void PreCallRecordGetMemoryWin32HandleKHR(VkDevice device, const VkMemoryGetWin32HandleInfoKHR* pGetWin32HandleInfo,
                                          HANDLE* pHandle);
void PreCallRecordGetMemoryWin32HandlePropertiesKHR(VkDevice device, VkExternalMemoryHandleTypeFlagBits handleType, HANDLE handle,
                                                    VkMemoryWin32HandlePropertiesKHR* pMemoryWin32HandleProperties);
void PreCallRecordImportSemaphoreWin32HandleKHR(VkDevice device,
                                                const VkImportSemaphoreWin32HandleInfoKHR* pImportSemaphoreWin32HandleInfo);
void PreCallRecordGetSemaphoreWin32HandleKHR(VkDevice device, const VkSemaphoreGetWin32HandleInfoKHR* pGetWin32HandleInfo,
                                             HANDLE* pHandle);
void PreCallRecordImportFenceWin32HandleKHR(VkDevice device, const VkImportFenceWin32HandleInfoKHR* pImportFenceWin32HandleInfo);
void PreCallRecordGetFenceWin32HandleKHR(VkDevice device, const VkFenceGetWin32HandleInfoKHR* pGetWin32HandleInfo, HANDLE* pHandle);
void PreCallRecordGetMemoryWin32HandleNV(VkDevice device, VkDeviceMemory memory, VkExternalMemoryHandleTypeFlagsNV handleType,
                                         HANDLE* pHandle);
#endif
#ifdef VK_USE_PLATFORM_VI_NN
bool PreCallValidateCreateViSurfaceNN(VkInstance instance, const VkViSurfaceCreateInfoNN* pCreateInfo,
                                      const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface);
void PreCallRecordCreateViSurfaceNN(VkInstance instance, const VkViSurfaceCreateInfoNN* pCreateInfo,
                                    const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface);
#endif
#ifdef VK_USE_PLATFORM_IOS_MVK
bool PreCallValidateCreateIOSSurfaceMVK(VkInstance instance, const VkIOSSurfaceCreateInfoMVK* pCreateInfo,
                                        const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface);
void PreCallRecordCreateIOSSurfaceMVK(VkInstance instance, const VkIOSSurfaceCreateInfoMVK* pCreateInfo,
                                      const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface);
#endif
#ifdef VK_USE_PLATFORM_MACOS_MVK
bool PreCallValidateCreateMacOSSurfaceMVK(VkInstance instance, const VkMacOSSurfaceCreateInfoMVK* pCreateInfo,
                                          const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface);
void PreCallRecordCreateMacOSSurfaceMVK(VkInstance instance, const VkMacOSSurfaceCreateInfoMVK* pCreateInfo,
                                        const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface);
#endif
bool PreCallValidateGetPhysicalDeviceFeatures2KHR(VkPhysicalDevice physicalDevice, VkPhysicalDeviceFeatures2* pFeatures);
bool PreCallValidateGetPhysicalDeviceProperties2KHR(VkPhysicalDevice physicalDevice, VkPhysicalDeviceProperties2* pProperties);
bool PreCallValidateGetPhysicalDeviceFormatProperties2KHR(VkPhysicalDevice physicalDevice, VkFormat format,
                                                          VkFormatProperties2* pFormatProperties);
bool PreCallValidateGetPhysicalDeviceImageFormatProperties2KHR(VkPhysicalDevice physicalDevice,
                                                               const VkPhysicalDeviceImageFormatInfo2* pImageFormatInfo,
                                                               VkImageFormatProperties2* pImageFormatProperties);
bool PreCallValidateGetPhysicalDeviceQueueFamilyProperties2KHR(VkPhysicalDevice physicalDevice, uint32_t* pQueueFamilyPropertyCount,
                                                               VkQueueFamilyProperties2* pQueueFamilyProperties);
bool PreCallValidateGetPhysicalDeviceMemoryProperties2KHR(VkPhysicalDevice physicalDevice,
                                                          VkPhysicalDeviceMemoryProperties2* pMemoryProperties);
bool PreCallValidateGetPhysicalDeviceSparseImageFormatProperties2KHR(VkPhysicalDevice physicalDevice,
                                                                     const VkPhysicalDeviceSparseImageFormatInfo2* pFormatInfo,
                                                                     uint32_t* pPropertyCount,
                                                                     VkSparseImageFormatProperties2* pProperties);
bool PreCallValidateGetDeviceGroupPeerMemoryFeaturesKHR(VkDevice device, uint32_t heapIndex, uint32_t localDeviceIndex,
                                                        uint32_t remoteDeviceIndex, VkPeerMemoryFeatureFlags* pPeerMemoryFeatures);
bool PreCallValidateCmdSetDeviceMaskKHR(VkCommandBuffer commandBuffer, uint32_t deviceMask);
bool PreCallValidateCmdDispatchBaseKHR(VkCommandBuffer commandBuffer, uint32_t baseGroupX, uint32_t baseGroupY, uint32_t baseGroupZ,
                                       uint32_t groupCountX, uint32_t groupCountY, uint32_t groupCountZ);
bool PreCallValidateTrimCommandPoolKHR(VkDevice device, VkCommandPool commandPool, VkCommandPoolTrimFlags flags);
bool PreCallValidateEnumeratePhysicalDeviceGroupsKHR(VkInstance instance, uint32_t* pPhysicalDeviceGroupCount,
                                                     VkPhysicalDeviceGroupProperties* pPhysicalDeviceGroupProperties);
bool PreCallValidateGetPhysicalDeviceExternalBufferPropertiesKHR(VkPhysicalDevice physicalDevice,
                                                                 const VkPhysicalDeviceExternalBufferInfo* pExternalBufferInfo,
                                                                 VkExternalBufferProperties* pExternalBufferProperties);
bool PreCallValidateGetMemoryFdKHR(VkDevice device, const VkMemoryGetFdInfoKHR* pGetFdInfo, int* pFd);
bool PreCallValidateGetMemoryFdPropertiesKHR(VkDevice device, VkExternalMemoryHandleTypeFlagBits handleType, int fd,
                                             VkMemoryFdPropertiesKHR* pMemoryFdProperties);
bool PreCallValidateGetPhysicalDeviceExternalSemaphorePropertiesKHR(
    VkPhysicalDevice physicalDevice, const VkPhysicalDeviceExternalSemaphoreInfo* pExternalSemaphoreInfo,
    VkExternalSemaphoreProperties* pExternalSemaphoreProperties);
bool PreCallValidateImportSemaphoreFdKHR(VkDevice device, const VkImportSemaphoreFdInfoKHR* pImportSemaphoreFdInfo);
bool PreCallValidateGetSemaphoreFdKHR(VkDevice device, const VkSemaphoreGetFdInfoKHR* pGetFdInfo, int* pFd);
bool PreCallValidateCmdPushDescriptorSetKHR(VkCommandBuffer commandBuffer, VkPipelineBindPoint pipelineBindPoint,
                                            VkPipelineLayout layout, uint32_t set, uint32_t descriptorWriteCount,
                                            const VkWriteDescriptorSet* pDescriptorWrites);
bool PreCallValidateCmdPushDescriptorSetWithTemplateKHR(VkCommandBuffer commandBuffer,
                                                        VkDescriptorUpdateTemplate descriptorUpdateTemplate,
                                                        VkPipelineLayout layout, uint32_t set, const void* pData);
bool PreCallValidateCreateDescriptorUpdateTemplateKHR(VkDevice device, const VkDescriptorUpdateTemplateCreateInfo* pCreateInfo,
                                                      const VkAllocationCallbacks* pAllocator,
                                                      VkDescriptorUpdateTemplate* pDescriptorUpdateTemplate);
bool PreCallValidateDestroyDescriptorUpdateTemplateKHR(VkDevice device, VkDescriptorUpdateTemplate descriptorUpdateTemplate,
                                                       const VkAllocationCallbacks* pAllocator);
bool PreCallValidateUpdateDescriptorSetWithTemplateKHR(VkDevice device, VkDescriptorSet descriptorSet,
                                                       VkDescriptorUpdateTemplate descriptorUpdateTemplate, const void* pData);
bool PreCallValidateGetSwapchainStatusKHR(VkDevice device, VkSwapchainKHR swapchain);
bool PreCallValidateGetPhysicalDeviceExternalFencePropertiesKHR(VkPhysicalDevice physicalDevice,
                                                                const VkPhysicalDeviceExternalFenceInfo* pExternalFenceInfo,
                                                                VkExternalFenceProperties* pExternalFenceProperties);
bool PreCallValidateImportFenceFdKHR(VkDevice device, const VkImportFenceFdInfoKHR* pImportFenceFdInfo);
bool PreCallValidateGetFenceFdKHR(VkDevice device, const VkFenceGetFdInfoKHR* pGetFdInfo, int* pFd);
bool PreCallValidateGetPhysicalDeviceSurfaceCapabilities2KHR(VkPhysicalDevice physicalDevice,
                                                             const VkPhysicalDeviceSurfaceInfo2KHR* pSurfaceInfo,
                                                             VkSurfaceCapabilities2KHR* pSurfaceCapabilities);
bool PreCallValidateGetPhysicalDeviceSurfaceFormats2KHR(VkPhysicalDevice physicalDevice,
                                                        const VkPhysicalDeviceSurfaceInfo2KHR* pSurfaceInfo,
                                                        uint32_t* pSurfaceFormatCount, VkSurfaceFormat2KHR* pSurfaceFormats);
bool PreCallValidateGetPhysicalDeviceDisplayProperties2KHR(VkPhysicalDevice physicalDevice, uint32_t* pPropertyCount,
                                                           VkDisplayProperties2KHR* pProperties);
bool PreCallValidateGetPhysicalDeviceDisplayPlaneProperties2KHR(VkPhysicalDevice physicalDevice, uint32_t* pPropertyCount,
                                                                VkDisplayPlaneProperties2KHR* pProperties);
bool PreCallValidateGetDisplayModeProperties2KHR(VkPhysicalDevice physicalDevice, VkDisplayKHR display, uint32_t* pPropertyCount,
                                                 VkDisplayModeProperties2KHR* pProperties);
bool PreCallValidateGetDisplayPlaneCapabilities2KHR(VkPhysicalDevice physicalDevice,
                                                    const VkDisplayPlaneInfo2KHR* pDisplayPlaneInfo,
                                                    VkDisplayPlaneCapabilities2KHR* pCapabilities);
bool PreCallValidateGetImageMemoryRequirements2KHR(VkDevice device, const VkImageMemoryRequirementsInfo2* pInfo,
                                                   VkMemoryRequirements2* pMemoryRequirements);
bool PreCallValidateGetBufferMemoryRequirements2KHR(VkDevice device, const VkBufferMemoryRequirementsInfo2* pInfo,
                                                    VkMemoryRequirements2* pMemoryRequirements);
bool PreCallValidateGetImageSparseMemoryRequirements2KHR(VkDevice device, const VkImageSparseMemoryRequirementsInfo2* pInfo,
                                                         uint32_t* pSparseMemoryRequirementCount,
                                                         VkSparseImageMemoryRequirements2* pSparseMemoryRequirements);
bool PreCallValidateCreateSamplerYcbcrConversionKHR(VkDevice device, const VkSamplerYcbcrConversionCreateInfo* pCreateInfo,
                                                    const VkAllocationCallbacks* pAllocator,
                                                    VkSamplerYcbcrConversion* pYcbcrConversion);
bool PreCallValidateDestroySamplerYcbcrConversionKHR(VkDevice device, VkSamplerYcbcrConversion ycbcrConversion,
                                                     const VkAllocationCallbacks* pAllocator);
bool PreCallValidateBindBufferMemory2KHR(VkDevice device, uint32_t bindInfoCount, const VkBindBufferMemoryInfo* pBindInfos);
bool PreCallValidateBindImageMemory2KHR(VkDevice device, uint32_t bindInfoCount, const VkBindImageMemoryInfo* pBindInfos);
bool PreCallValidateGetDescriptorSetLayoutSupportKHR(VkDevice device, const VkDescriptorSetLayoutCreateInfo* pCreateInfo,
                                                     VkDescriptorSetLayoutSupport* pSupport);
bool PreCallValidateCmdDrawIndirectCountKHR(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                            VkBuffer countBuffer, VkDeviceSize countBufferOffset, uint32_t maxDrawCount,
                                            uint32_t stride);
bool PreCallValidateCmdDrawIndexedIndirectCountKHR(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                   VkBuffer countBuffer, VkDeviceSize countBufferOffset, uint32_t maxDrawCount,
                                                   uint32_t stride);
bool PreCallValidateCreateDebugReportCallbackEXT(VkInstance instance, const VkDebugReportCallbackCreateInfoEXT* pCreateInfo,
                                                 const VkAllocationCallbacks* pAllocator, VkDebugReportCallbackEXT* pCallback);
bool PreCallValidateDestroyDebugReportCallbackEXT(VkInstance instance, VkDebugReportCallbackEXT callback,
                                                  const VkAllocationCallbacks* pAllocator);
bool PreCallValidateDebugReportMessageEXT(VkInstance instance, VkDebugReportFlagsEXT flags, VkDebugReportObjectTypeEXT objectType,
                                          uint64_t object, size_t location, int32_t messageCode, const char* pLayerPrefix,
                                          const char* pMessage);
bool PreCallValidateDebugMarkerSetObjectTagEXT(VkDevice device, const VkDebugMarkerObjectTagInfoEXT* pTagInfo);
bool PreCallValidateDebugMarkerSetObjectNameEXT(VkDevice device, const VkDebugMarkerObjectNameInfoEXT* pNameInfo);
bool PreCallValidateCmdDebugMarkerBeginEXT(VkCommandBuffer commandBuffer, const VkDebugMarkerMarkerInfoEXT* pMarkerInfo);
bool PreCallValidateCmdDebugMarkerEndEXT(VkCommandBuffer commandBuffer);
bool PreCallValidateCmdDebugMarkerInsertEXT(VkCommandBuffer commandBuffer, const VkDebugMarkerMarkerInfoEXT* pMarkerInfo);
bool PreCallValidateCmdDrawIndirectCountAMD(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                            VkBuffer countBuffer, VkDeviceSize countBufferOffset, uint32_t maxDrawCount,
                                            uint32_t stride);
bool PreCallValidateCmdDrawIndexedIndirectCountAMD(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                   VkBuffer countBuffer, VkDeviceSize countBufferOffset, uint32_t maxDrawCount,
                                                   uint32_t stride);
bool PreCallValidateGetShaderInfoAMD(VkDevice device, VkPipeline pipeline, VkShaderStageFlagBits shaderStage,
                                     VkShaderInfoTypeAMD infoType, size_t* pInfoSize, void* pInfo);
bool PreCallValidateGetPhysicalDeviceExternalImageFormatPropertiesNV(
    VkPhysicalDevice physicalDevice, VkFormat format, VkImageType type, VkImageTiling tiling, VkImageUsageFlags usage,
    VkImageCreateFlags flags, VkExternalMemoryHandleTypeFlagsNV externalHandleType,
    VkExternalImageFormatPropertiesNV* pExternalImageFormatProperties);
bool PreCallValidateCmdProcessCommandsNVX(VkCommandBuffer commandBuffer, const VkCmdProcessCommandsInfoNVX* pProcessCommandsInfo);
bool PreCallValidateCmdReserveSpaceForCommandsNVX(VkCommandBuffer commandBuffer,
                                                  const VkCmdReserveSpaceForCommandsInfoNVX* pReserveSpaceInfo);
bool PreCallValidateCreateIndirectCommandsLayoutNVX(VkDevice device, const VkIndirectCommandsLayoutCreateInfoNVX* pCreateInfo,
                                                    const VkAllocationCallbacks* pAllocator,
                                                    VkIndirectCommandsLayoutNVX* pIndirectCommandsLayout);
bool PreCallValidateDestroyIndirectCommandsLayoutNVX(VkDevice device, VkIndirectCommandsLayoutNVX indirectCommandsLayout,
                                                     const VkAllocationCallbacks* pAllocator);
bool PreCallValidateCreateObjectTableNVX(VkDevice device, const VkObjectTableCreateInfoNVX* pCreateInfo,
                                         const VkAllocationCallbacks* pAllocator, VkObjectTableNVX* pObjectTable);
bool PreCallValidateDestroyObjectTableNVX(VkDevice device, VkObjectTableNVX objectTable, const VkAllocationCallbacks* pAllocator);
bool PreCallValidateRegisterObjectsNVX(VkDevice device, VkObjectTableNVX objectTable, uint32_t objectCount,
                                       const VkObjectTableEntryNVX* const* ppObjectTableEntries, const uint32_t* pObjectIndices);
bool PreCallValidateUnregisterObjectsNVX(VkDevice device, VkObjectTableNVX objectTable, uint32_t objectCount,
                                         const VkObjectEntryTypeNVX* pObjectEntryTypes, const uint32_t* pObjectIndices);
bool PreCallValidateGetPhysicalDeviceGeneratedCommandsPropertiesNVX(VkPhysicalDevice physicalDevice,
                                                                    VkDeviceGeneratedCommandsFeaturesNVX* pFeatures,
                                                                    VkDeviceGeneratedCommandsLimitsNVX* pLimits);
bool PreCallValidateCmdSetViewportWScalingNV(VkCommandBuffer commandBuffer, uint32_t firstViewport, uint32_t viewportCount,
                                             const VkViewportWScalingNV* pViewportWScalings);
bool PreCallValidateReleaseDisplayEXT(VkPhysicalDevice physicalDevice, VkDisplayKHR display);
bool PreCallValidateGetPhysicalDeviceSurfaceCapabilities2EXT(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface,
                                                             VkSurfaceCapabilities2EXT* pSurfaceCapabilities);
bool PreCallValidateDisplayPowerControlEXT(VkDevice device, VkDisplayKHR display, const VkDisplayPowerInfoEXT* pDisplayPowerInfo);
bool PreCallValidateRegisterDeviceEventEXT(VkDevice device, const VkDeviceEventInfoEXT* pDeviceEventInfo,
                                           const VkAllocationCallbacks* pAllocator, VkFence* pFence);
bool PreCallValidateRegisterDisplayEventEXT(VkDevice device, VkDisplayKHR display, const VkDisplayEventInfoEXT* pDisplayEventInfo,
                                            const VkAllocationCallbacks* pAllocator, VkFence* pFence);
bool PreCallValidateGetSwapchainCounterEXT(VkDevice device, VkSwapchainKHR swapchain, VkSurfaceCounterFlagBitsEXT counter,
                                           uint64_t* pCounterValue);
bool PreCallValidateGetRefreshCycleDurationGOOGLE(VkDevice device, VkSwapchainKHR swapchain,
                                                  VkRefreshCycleDurationGOOGLE* pDisplayTimingProperties);
bool PreCallValidateGetPastPresentationTimingGOOGLE(VkDevice device, VkSwapchainKHR swapchain, uint32_t* pPresentationTimingCount,
                                                    VkPastPresentationTimingGOOGLE* pPresentationTimings);
bool PreCallValidateCmdSetDiscardRectangleEXT(VkCommandBuffer commandBuffer, uint32_t firstDiscardRectangle,
                                              uint32_t discardRectangleCount, const VkRect2D* pDiscardRectangles);
bool PreCallValidateSetHdrMetadataEXT(VkDevice device, uint32_t swapchainCount, const VkSwapchainKHR* pSwapchains,
                                      const VkHdrMetadataEXT* pMetadata);
bool PreCallValidateSetDebugUtilsObjectNameEXT(VkDevice device, const VkDebugUtilsObjectNameInfoEXT* pNameInfo);
bool PreCallValidateSetDebugUtilsObjectTagEXT(VkDevice device, const VkDebugUtilsObjectTagInfoEXT* pTagInfo);
bool PreCallValidateQueueBeginDebugUtilsLabelEXT(VkQueue queue, const VkDebugUtilsLabelEXT* pLabelInfo);
bool PreCallValidateQueueEndDebugUtilsLabelEXT(VkQueue queue);
bool PreCallValidateQueueInsertDebugUtilsLabelEXT(VkQueue queue, const VkDebugUtilsLabelEXT* pLabelInfo);
bool PreCallValidateCmdBeginDebugUtilsLabelEXT(VkCommandBuffer commandBuffer, const VkDebugUtilsLabelEXT* pLabelInfo);
bool PreCallValidateCmdEndDebugUtilsLabelEXT(VkCommandBuffer commandBuffer);
bool PreCallValidateCmdInsertDebugUtilsLabelEXT(VkCommandBuffer commandBuffer, const VkDebugUtilsLabelEXT* pLabelInfo);
bool PreCallValidateCreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
                                                 const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pMessenger);
bool PreCallValidateDestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT messenger,
                                                  const VkAllocationCallbacks* pAllocator);
bool PreCallValidateSubmitDebugUtilsMessageEXT(VkInstance instance, VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
                                               VkDebugUtilsMessageTypeFlagsEXT messageTypes,
                                               const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData);
bool PreCallValidateCmdSetSampleLocationsEXT(VkCommandBuffer commandBuffer, const VkSampleLocationsInfoEXT* pSampleLocationsInfo);
bool PreCallValidateGetPhysicalDeviceMultisamplePropertiesEXT(VkPhysicalDevice physicalDevice, VkSampleCountFlagBits samples,
                                                              VkMultisamplePropertiesEXT* pMultisampleProperties);
bool PreCallValidateCreateValidationCacheEXT(VkDevice device, const VkValidationCacheCreateInfoEXT* pCreateInfo,
                                             const VkAllocationCallbacks* pAllocator, VkValidationCacheEXT* pValidationCache);
bool PreCallValidateDestroyValidationCacheEXT(VkDevice device, VkValidationCacheEXT validationCache,
                                              const VkAllocationCallbacks* pAllocator);
bool PreCallValidateMergeValidationCachesEXT(VkDevice device, VkValidationCacheEXT dstCache, uint32_t srcCacheCount,
                                             const VkValidationCacheEXT* pSrcCaches);
bool PreCallValidateGetValidationCacheDataEXT(VkDevice device, VkValidationCacheEXT validationCache, size_t* pDataSize,
                                              void* pData);
bool PreCallValidateGetMemoryHostPointerPropertiesEXT(VkDevice device, VkExternalMemoryHandleTypeFlagBits handleType,
                                                      const void* pHostPointer,
                                                      VkMemoryHostPointerPropertiesEXT* pMemoryHostPointerProperties);
bool PreCallValidateCmdWriteBufferMarkerAMD(VkCommandBuffer commandBuffer, VkPipelineStageFlagBits pipelineStage,
                                            VkBuffer dstBuffer, VkDeviceSize dstOffset, uint32_t marker);

void PreCallRecordCreateInstance(const VkInstanceCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator,
                                 VkInstance* pInstance);
void PreCallRecordDestroyInstance(VkInstance instance, const VkAllocationCallbacks* pAllocator);
void PreCallRecordEnumeratePhysicalDevices(VkInstance instance, uint32_t* pPhysicalDeviceCount, VkPhysicalDevice* pPhysicalDevices);
void PreCallRecordGetPhysicalDeviceFeatures(VkPhysicalDevice physicalDevice, VkPhysicalDeviceFeatures* pFeatures);
void PreCallRecordGetPhysicalDeviceFormatProperties(VkPhysicalDevice physicalDevice, VkFormat format,
                                                    VkFormatProperties* pFormatProperties);
void PreCallRecordGetPhysicalDeviceImageFormatProperties(VkPhysicalDevice physicalDevice, VkFormat format, VkImageType type,
                                                         VkImageTiling tiling, VkImageUsageFlags usage, VkImageCreateFlags flags,
                                                         VkImageFormatProperties* pImageFormatProperties);
void PreCallRecordGetPhysicalDeviceProperties(VkPhysicalDevice physicalDevice, VkPhysicalDeviceProperties* pProperties);
void PreCallRecordGetPhysicalDeviceQueueFamilyProperties(VkPhysicalDevice physicalDevice, uint32_t* pQueueFamilyPropertyCount,
                                                         VkQueueFamilyProperties* pQueueFamilyProperties);
void PreCallRecordGetPhysicalDeviceMemoryProperties(VkPhysicalDevice physicalDevice,
                                                    VkPhysicalDeviceMemoryProperties* pMemoryProperties);
void PreCallRecordCreateDevice(VkPhysicalDevice physicalDevice, const VkDeviceCreateInfo* pCreateInfo,
                               const VkAllocationCallbacks* pAllocator, VkDevice* pDevice);
void PreCallRecordDestroyDevice(VkDevice device, const VkAllocationCallbacks* pAllocator);
void PreCallRecordEnumerateInstanceExtensionProperties(const char* pLayerName, uint32_t* pPropertyCount,
                                                       VkExtensionProperties* pProperties);
void PreCallRecordEnumerateDeviceExtensionProperties(VkPhysicalDevice physicalDevice, const char* pLayerName,
                                                     uint32_t* pPropertyCount, VkExtensionProperties* pProperties);
void PreCallRecordEnumerateInstanceLayerProperties(uint32_t* pPropertyCount, VkLayerProperties* pProperties);
void PreCallRecordEnumerateDeviceLayerProperties(VkPhysicalDevice physicalDevice, uint32_t* pPropertyCount,
                                                 VkLayerProperties* pProperties);
void PreCallRecordGetDeviceQueue(VkDevice device, uint32_t queueFamilyIndex, uint32_t queueIndex, VkQueue* pQueue);
void PreCallRecordQueueSubmit(VkQueue queue, uint32_t submitCount, const VkSubmitInfo* pSubmits, VkFence fence);
void PreCallRecordQueueWaitIdle(VkQueue queue);
void PreCallRecordDeviceWaitIdle(VkDevice device);
void PreCallRecordAllocateMemory(VkDevice device, const VkMemoryAllocateInfo* pAllocateInfo,
                                 const VkAllocationCallbacks* pAllocator, VkDeviceMemory* pMemory);
void PreCallRecordFreeMemory(VkDevice device, VkDeviceMemory memory, const VkAllocationCallbacks* pAllocator);
void PreCallRecordMapMemory(VkDevice device, VkDeviceMemory memory, VkDeviceSize offset, VkDeviceSize size, VkMemoryMapFlags flags,
                            void** ppData);
void PreCallRecordUnmapMemory(VkDevice device, VkDeviceMemory memory);
void PreCallRecordFlushMappedMemoryRanges(VkDevice device, uint32_t memoryRangeCount, const VkMappedMemoryRange* pMemoryRanges);
void PreCallRecordInvalidateMappedMemoryRanges(VkDevice device, uint32_t memoryRangeCount,
                                               const VkMappedMemoryRange* pMemoryRanges);
void PreCallRecordGetDeviceMemoryCommitment(VkDevice device, VkDeviceMemory memory, VkDeviceSize* pCommittedMemoryInBytes);
void PreCallRecordBindBufferMemory(VkDevice device, VkBuffer buffer, VkDeviceMemory memory, VkDeviceSize memoryOffset);
void PreCallRecordBindImageMemory(VkDevice device, VkImage image, VkDeviceMemory memory, VkDeviceSize memoryOffset);
void PreCallRecordGetBufferMemoryRequirements(VkDevice device, VkBuffer buffer, VkMemoryRequirements* pMemoryRequirements);
void PreCallRecordGetImageMemoryRequirements(VkDevice device, VkImage image, VkMemoryRequirements* pMemoryRequirements);
void PreCallRecordGetImageSparseMemoryRequirements(VkDevice device, VkImage image, uint32_t* pSparseMemoryRequirementCount,
                                                   VkSparseImageMemoryRequirements* pSparseMemoryRequirements);
void PreCallRecordGetPhysicalDeviceSparseImageFormatProperties(VkPhysicalDevice physicalDevice, VkFormat format, VkImageType type,
                                                               VkSampleCountFlagBits samples, VkImageUsageFlags usage,
                                                               VkImageTiling tiling, uint32_t* pPropertyCount,
                                                               VkSparseImageFormatProperties* pProperties);
void PreCallRecordQueueBindSparse(VkQueue queue, uint32_t bindInfoCount, const VkBindSparseInfo* pBindInfo, VkFence fence);
void PreCallRecordCreateFence(VkDevice device, const VkFenceCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator,
                              VkFence* pFence);
void PreCallRecordDestroyFence(VkDevice device, VkFence fence, const VkAllocationCallbacks* pAllocator);
void PreCallRecordResetFences(VkDevice device, uint32_t fenceCount, const VkFence* pFences);
void PreCallRecordGetFenceStatus(VkDevice device, VkFence fence);
void PreCallRecordWaitForFences(VkDevice device, uint32_t fenceCount, const VkFence* pFences, VkBool32 waitAll, uint64_t timeout);
void PreCallRecordCreateSemaphore(VkDevice device, const VkSemaphoreCreateInfo* pCreateInfo,
                                  const VkAllocationCallbacks* pAllocator, VkSemaphore* pSemaphore);
void PreCallRecordDestroySemaphore(VkDevice device, VkSemaphore semaphore, const VkAllocationCallbacks* pAllocator);
void PreCallRecordCreateEvent(VkDevice device, const VkEventCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator,
                              VkEvent* pEvent);
void PreCallRecordDestroyEvent(VkDevice device, VkEvent event, const VkAllocationCallbacks* pAllocator);
void PreCallRecordGetEventStatus(VkDevice device, VkEvent event);
void PreCallRecordSetEvent(VkDevice device, VkEvent event);
void PreCallRecordResetEvent(VkDevice device, VkEvent event);
void PreCallRecordCreateQueryPool(VkDevice device, const VkQueryPoolCreateInfo* pCreateInfo,
                                  const VkAllocationCallbacks* pAllocator, VkQueryPool* pQueryPool);
void PreCallRecordDestroyQueryPool(VkDevice device, VkQueryPool queryPool, const VkAllocationCallbacks* pAllocator);
void PreCallRecordGetQueryPoolResults(VkDevice device, VkQueryPool queryPool, uint32_t firstQuery, uint32_t queryCount,
                                      size_t dataSize, void* pData, VkDeviceSize stride, VkQueryResultFlags flags);
void PreCallRecordCreateBuffer(VkDevice device, const VkBufferCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator,
                               VkBuffer* pBuffer);
void PreCallRecordDestroyBuffer(VkDevice device, VkBuffer buffer, const VkAllocationCallbacks* pAllocator);
void PreCallRecordCreateBufferView(VkDevice device, const VkBufferViewCreateInfo* pCreateInfo,
                                   const VkAllocationCallbacks* pAllocator, VkBufferView* pView);
void PreCallRecordDestroyBufferView(VkDevice device, VkBufferView bufferView, const VkAllocationCallbacks* pAllocator);
void PreCallRecordCreateImage(VkDevice device, const VkImageCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator,
                              VkImage* pImage);
void PreCallRecordDestroyImage(VkDevice device, VkImage image, const VkAllocationCallbacks* pAllocator);
void PreCallRecordGetImageSubresourceLayout(VkDevice device, VkImage image, const VkImageSubresource* pSubresource,
                                            VkSubresourceLayout* pLayout);
void PreCallRecordCreateImageView(VkDevice device, const VkImageViewCreateInfo* pCreateInfo,
                                  const VkAllocationCallbacks* pAllocator, VkImageView* pView);
void PreCallRecordDestroyImageView(VkDevice device, VkImageView imageView, const VkAllocationCallbacks* pAllocator);
void PreCallRecordCreateShaderModule(VkDevice device, const VkShaderModuleCreateInfo* pCreateInfo,
                                     const VkAllocationCallbacks* pAllocator, VkShaderModule* pShaderModule);
void PreCallRecordDestroyShaderModule(VkDevice device, VkShaderModule shaderModule, const VkAllocationCallbacks* pAllocator);
void PreCallRecordCreatePipelineCache(VkDevice device, const VkPipelineCacheCreateInfo* pCreateInfo,
                                      const VkAllocationCallbacks* pAllocator, VkPipelineCache* pPipelineCache);
void PreCallRecordDestroyPipelineCache(VkDevice device, VkPipelineCache pipelineCache, const VkAllocationCallbacks* pAllocator);
void PreCallRecordGetPipelineCacheData(VkDevice device, VkPipelineCache pipelineCache, size_t* pDataSize, void* pData);
void PreCallRecordMergePipelineCaches(VkDevice device, VkPipelineCache dstCache, uint32_t srcCacheCount,
                                      const VkPipelineCache* pSrcCaches);
void PreCallRecordCreateGraphicsPipelines(VkDevice device, VkPipelineCache pipelineCache, uint32_t createInfoCount,
                                          const VkGraphicsPipelineCreateInfo* pCreateInfos, const VkAllocationCallbacks* pAllocator,
                                          VkPipeline* pPipelines);
void PreCallRecordCreateComputePipelines(VkDevice device, VkPipelineCache pipelineCache, uint32_t createInfoCount,
                                         const VkComputePipelineCreateInfo* pCreateInfos, const VkAllocationCallbacks* pAllocator,
                                         VkPipeline* pPipelines);
void PreCallRecordDestroyPipeline(VkDevice device, VkPipeline pipeline, const VkAllocationCallbacks* pAllocator);
void PreCallRecordCreatePipelineLayout(VkDevice device, const VkPipelineLayoutCreateInfo* pCreateInfo,
                                       const VkAllocationCallbacks* pAllocator, VkPipelineLayout* pPipelineLayout);
void PreCallRecordDestroyPipelineLayout(VkDevice device, VkPipelineLayout pipelineLayout, const VkAllocationCallbacks* pAllocator);
void PreCallRecordCreateSampler(VkDevice device, const VkSamplerCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator,
                                VkSampler* pSampler);
void PreCallRecordDestroySampler(VkDevice device, VkSampler sampler, const VkAllocationCallbacks* pAllocator);
void PreCallRecordCreateDescriptorSetLayout(VkDevice device, const VkDescriptorSetLayoutCreateInfo* pCreateInfo,
                                            const VkAllocationCallbacks* pAllocator, VkDescriptorSetLayout* pSetLayout);
void PreCallRecordDestroyDescriptorSetLayout(VkDevice device, VkDescriptorSetLayout descriptorSetLayout,
                                             const VkAllocationCallbacks* pAllocator);
void PreCallRecordCreateDescriptorPool(VkDevice device, const VkDescriptorPoolCreateInfo* pCreateInfo,
                                       const VkAllocationCallbacks* pAllocator, VkDescriptorPool* pDescriptorPool);
void PreCallRecordDestroyDescriptorPool(VkDevice device, VkDescriptorPool descriptorPool, const VkAllocationCallbacks* pAllocator);
void PreCallRecordResetDescriptorPool(VkDevice device, VkDescriptorPool descriptorPool, VkDescriptorPoolResetFlags flags);
void PreCallRecordAllocateDescriptorSets(VkDevice device, const VkDescriptorSetAllocateInfo* pAllocateInfo,
                                         VkDescriptorSet* pDescriptorSets);
void PreCallRecordFreeDescriptorSets(VkDevice device, VkDescriptorPool descriptorPool, uint32_t descriptorSetCount,
                                     const VkDescriptorSet* pDescriptorSets);
void PreCallRecordUpdateDescriptorSets(VkDevice device, uint32_t descriptorWriteCount,
                                       const VkWriteDescriptorSet* pDescriptorWrites, uint32_t descriptorCopyCount,
                                       const VkCopyDescriptorSet* pDescriptorCopies);
void PreCallRecordCreateFramebuffer(VkDevice device, const VkFramebufferCreateInfo* pCreateInfo,
                                    const VkAllocationCallbacks* pAllocator, VkFramebuffer* pFramebuffer);
void PreCallRecordDestroyFramebuffer(VkDevice device, VkFramebuffer framebuffer, const VkAllocationCallbacks* pAllocator);
void PreCallRecordCreateRenderPass(VkDevice device, const VkRenderPassCreateInfo* pCreateInfo,
                                   const VkAllocationCallbacks* pAllocator, VkRenderPass* pRenderPass);
void PreCallRecordCreateRenderPass2KHR(VkDevice device, const VkRenderPassCreateInfo* pCreateInfo,
                                       const VkAllocationCallbacks* pAllocator, VkRenderPass* pRenderPass);
bool PreCallValidateCreateRenderPass2KHR(VkDevice device, const VkRenderPassCreateInfo2KHR* pCreateInfo,
                                         const VkAllocationCallbacks* pAllocator, VkRenderPass* pRenderPass);
void PreCallRecordDestroyRenderPass(VkDevice device, VkRenderPass renderPass, const VkAllocationCallbacks* pAllocator);
void PreCallRecordGetRenderAreaGranularity(VkDevice device, VkRenderPass renderPass, VkExtent2D* pGranularity);
void PreCallRecordCreateCommandPool(VkDevice device, const VkCommandPoolCreateInfo* pCreateInfo,
                                    const VkAllocationCallbacks* pAllocator, VkCommandPool* pCommandPool);
void PreCallRecordDestroyCommandPool(VkDevice device, VkCommandPool commandPool, const VkAllocationCallbacks* pAllocator);
void PreCallRecordResetCommandPool(VkDevice device, VkCommandPool commandPool, VkCommandPoolResetFlags flags);
void PreCallRecordAllocateCommandBuffers(VkDevice device, const VkCommandBufferAllocateInfo* pAllocateInfo,
                                         VkCommandBuffer* pCommandBuffers);
void PreCallRecordFreeCommandBuffers(VkDevice device, VkCommandPool commandPool, uint32_t commandBufferCount,
                                     const VkCommandBuffer* pCommandBuffers);
void PreCallRecordBeginCommandBuffer(VkCommandBuffer commandBuffer, const VkCommandBufferBeginInfo* pBeginInfo);
void PreCallRecordEndCommandBuffer(VkCommandBuffer commandBuffer);
void PreCallRecordResetCommandBuffer(VkCommandBuffer commandBuffer, VkCommandBufferResetFlags flags);
void PreCallRecordCmdBindPipeline(VkCommandBuffer commandBuffer, VkPipelineBindPoint pipelineBindPoint, VkPipeline pipeline);
void PreCallRecordCmdSetViewport(VkCommandBuffer commandBuffer, uint32_t firstViewport, uint32_t viewportCount,
                                 const VkViewport* pViewports);
void PreCallRecordCmdSetScissor(VkCommandBuffer commandBuffer, uint32_t firstScissor, uint32_t scissorCount,
                                const VkRect2D* pScissors);
void PreCallRecordCmdSetLineWidth(VkCommandBuffer commandBuffer, float lineWidth);
void PreCallRecordCmdSetDepthBias(VkCommandBuffer commandBuffer, float depthBiasConstantFactor, float depthBiasClamp,
                                  float depthBiasSlopeFactor);
void PreCallRecordCmdSetBlendConstants(VkCommandBuffer commandBuffer, const float blendConstants[4]);
void PreCallRecordCmdSetDepthBounds(VkCommandBuffer commandBuffer, float minDepthBounds, float maxDepthBounds);
void PreCallRecordCmdSetStencilCompareMask(VkCommandBuffer commandBuffer, VkStencilFaceFlags faceMask, uint32_t compareMask);
void PreCallRecordCmdSetStencilWriteMask(VkCommandBuffer commandBuffer, VkStencilFaceFlags faceMask, uint32_t writeMask);
void PreCallRecordCmdSetStencilReference(VkCommandBuffer commandBuffer, VkStencilFaceFlags faceMask, uint32_t reference);
void PreCallRecordCmdBindDescriptorSets(VkCommandBuffer commandBuffer, VkPipelineBindPoint pipelineBindPoint,
                                        VkPipelineLayout layout, uint32_t firstSet, uint32_t descriptorSetCount,
                                        const VkDescriptorSet* pDescriptorSets, uint32_t dynamicOffsetCount,
                                        const uint32_t* pDynamicOffsets);
void PreCallRecordCmdBindIndexBuffer(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset, VkIndexType indexType);
void PreCallRecordCmdBindVertexBuffers(VkCommandBuffer commandBuffer, uint32_t firstBinding, uint32_t bindingCount,
                                       const VkBuffer* pBuffers, const VkDeviceSize* pOffsets);
void PreCallRecordCmdDraw(VkCommandBuffer commandBuffer, uint32_t vertexCount, uint32_t instanceCount, uint32_t firstVertex,
                          uint32_t firstInstance);
void PreCallRecordCmdDrawIndexed(VkCommandBuffer commandBuffer, uint32_t indexCount, uint32_t instanceCount, uint32_t firstIndex,
                                 int32_t vertexOffset, uint32_t firstInstance);
void PreCallRecordCmdDrawIndirect(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset, uint32_t drawCount,
                                  uint32_t stride);
void PreCallRecordCmdDrawIndexedIndirect(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset, uint32_t drawCount,
                                         uint32_t stride);
void PreCallRecordCmdDispatch(VkCommandBuffer commandBuffer, uint32_t groupCountX, uint32_t groupCountY, uint32_t groupCountZ);
void PreCallRecordCmdDispatchIndirect(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset);
void PreCallRecordCmdCopyBuffer(VkCommandBuffer commandBuffer, VkBuffer srcBuffer, VkBuffer dstBuffer, uint32_t regionCount,
                                const VkBufferCopy* pRegions);
void PreCallRecordCmdCopyImage(VkCommandBuffer commandBuffer, VkImage srcImage, VkImageLayout srcImageLayout, VkImage dstImage,
                               VkImageLayout dstImageLayout, uint32_t regionCount, const VkImageCopy* pRegions);
void PreCallRecordCmdBlitImage(VkCommandBuffer commandBuffer, VkImage srcImage, VkImageLayout srcImageLayout, VkImage dstImage,
                               VkImageLayout dstImageLayout, uint32_t regionCount, const VkImageBlit* pRegions, VkFilter filter);
void PreCallRecordCmdCopyBufferToImage(VkCommandBuffer commandBuffer, VkBuffer srcBuffer, VkImage dstImage,
                                       VkImageLayout dstImageLayout, uint32_t regionCount, const VkBufferImageCopy* pRegions);
void PreCallRecordCmdCopyImageToBuffer(VkCommandBuffer commandBuffer, VkImage srcImage, VkImageLayout srcImageLayout,
                                       VkBuffer dstBuffer, uint32_t regionCount, const VkBufferImageCopy* pRegions);
void PreCallRecordCmdUpdateBuffer(VkCommandBuffer commandBuffer, VkBuffer dstBuffer, VkDeviceSize dstOffset, VkDeviceSize dataSize,
                                  const void* pData);
void PreCallRecordCmdFillBuffer(VkCommandBuffer commandBuffer, VkBuffer dstBuffer, VkDeviceSize dstOffset, VkDeviceSize size,
                                uint32_t data);
void PreCallRecordCmdClearColorImage(VkCommandBuffer commandBuffer, VkImage image, VkImageLayout imageLayout,
                                     const VkClearColorValue* pColor, uint32_t rangeCount, const VkImageSubresourceRange* pRanges);
void PreCallRecordCmdClearDepthStencilImage(VkCommandBuffer commandBuffer, VkImage image, VkImageLayout imageLayout,
                                            const VkClearDepthStencilValue* pDepthStencil, uint32_t rangeCount,
                                            const VkImageSubresourceRange* pRanges);
void PreCallRecordCmdClearAttachments(VkCommandBuffer commandBuffer, uint32_t attachmentCount,
                                      const VkClearAttachment* pAttachments, uint32_t rectCount, const VkClearRect* pRects);
void PreCallRecordCmdResolveImage(VkCommandBuffer commandBuffer, VkImage srcImage, VkImageLayout srcImageLayout, VkImage dstImage,
                                  VkImageLayout dstImageLayout, uint32_t regionCount, const VkImageResolve* pRegions);
void PreCallRecordCmdSetEvent(VkCommandBuffer commandBuffer, VkEvent event, VkPipelineStageFlags stageMask);
void PreCallRecordCmdResetEvent(VkCommandBuffer commandBuffer, VkEvent event, VkPipelineStageFlags stageMask);
void PreCallRecordCmdWaitEvents(VkCommandBuffer commandBuffer, uint32_t eventCount, const VkEvent* pEvents,
                                VkPipelineStageFlags srcStageMask, VkPipelineStageFlags dstStageMask, uint32_t memoryBarrierCount,
                                const VkMemoryBarrier* pMemoryBarriers, uint32_t bufferMemoryBarrierCount,
                                const VkBufferMemoryBarrier* pBufferMemoryBarriers, uint32_t imageMemoryBarrierCount,
                                const VkImageMemoryBarrier* pImageMemoryBarriers);
void PreCallRecordCmdPipelineBarrier(VkCommandBuffer commandBuffer, VkPipelineStageFlags srcStageMask,
                                     VkPipelineStageFlags dstStageMask, VkDependencyFlags dependencyFlags,
                                     uint32_t memoryBarrierCount, const VkMemoryBarrier* pMemoryBarriers,
                                     uint32_t bufferMemoryBarrierCount, const VkBufferMemoryBarrier* pBufferMemoryBarriers,
                                     uint32_t imageMemoryBarrierCount, const VkImageMemoryBarrier* pImageMemoryBarriers);
void PreCallRecordCmdBeginQuery(VkCommandBuffer commandBuffer, VkQueryPool queryPool, uint32_t query, VkQueryControlFlags flags);
void PreCallRecordCmdEndQuery(VkCommandBuffer commandBuffer, VkQueryPool queryPool, uint32_t query);
void PreCallRecordCmdResetQueryPool(VkCommandBuffer commandBuffer, VkQueryPool queryPool, uint32_t firstQuery, uint32_t queryCount);
void PreCallRecordCmdWriteTimestamp(VkCommandBuffer commandBuffer, VkPipelineStageFlagBits pipelineStage, VkQueryPool queryPool,
                                    uint32_t query);
void PreCallRecordCmdCopyQueryPoolResults(VkCommandBuffer commandBuffer, VkQueryPool queryPool, uint32_t firstQuery,
                                          uint32_t queryCount, VkBuffer dstBuffer, VkDeviceSize dstOffset, VkDeviceSize stride,
                                          VkQueryResultFlags flags);
void PreCallRecordCmdPushConstants(VkCommandBuffer commandBuffer, VkPipelineLayout layout, VkShaderStageFlags stageFlags,
                                   uint32_t offset, uint32_t size, const void* pValues);
void PreCallRecordCmdBeginRenderPass(VkCommandBuffer commandBuffer, const VkRenderPassBeginInfo* pRenderPassBegin,
                                     VkSubpassContents contents);
void PreCallRecordCmdNextSubpass(VkCommandBuffer commandBuffer, VkSubpassContents contents);
void PreCallRecordCmdNextSubpass2KHR(VkCommandBuffer commandBuffer, const VkSubpassBeginInfoKHR* pSubpassBeginInfo,
                                     const VkSubpassEndInfoKHR* pSubpassEndInfo);
bool PreCallValidateCmdNextSubpass2KHR(VkCommandBuffer commandBuffer, const VkSubpassBeginInfoKHR* pSubpassBeginInfo,
                                       const VkSubpassEndInfoKHR* pSubpassEndInfo);
void PreCallRecordCmdBeginConditionalRenderingEXT(VkCommandBuffer commandBuffer,
                                                  const VkConditionalRenderingBeginInfoEXT* pConditionalRenderingBegin);
bool PreCallValidateCmdBeginConditionalRenderingEXT(VkCommandBuffer commandBuffer,
                                                    const VkConditionalRenderingBeginInfoEXT* pConditionalRenderingBegin);
void PreCallRecordCmdEndConditionalRenderingEXT(VkCommandBuffer commandBuffer);
bool PreCallValidateCmdEndConditionalRenderingEXT(VkCommandBuffer commandBuffer);
void PreCallRecordCmdEndRenderPass(VkCommandBuffer commandBuffer);
void PreCallRecordCmdEndRenderPass2KHR(VkCommandBuffer commandBuffer, const VkSubpassEndInfoKHR* pSubpassEndInfo);
void PreCallRecordCmdExecuteCommands(VkCommandBuffer commandBuffer, uint32_t commandBufferCount,
                                     const VkCommandBuffer* pCommandBuffers);
void PreCallRecordBindBufferMemory2(VkDevice device, uint32_t bindInfoCount, const VkBindBufferMemoryInfo* pBindInfos);
void PreCallRecordBindImageMemory2(VkDevice device, uint32_t bindInfoCount, const VkBindImageMemoryInfo* pBindInfos);
void PreCallRecordGetDeviceGroupPeerMemoryFeatures(VkDevice device, uint32_t heapIndex, uint32_t localDeviceIndex,
                                                   uint32_t remoteDeviceIndex, VkPeerMemoryFeatureFlags* pPeerMemoryFeatures);
void PreCallRecordCmdSetDeviceMask(VkCommandBuffer commandBuffer, uint32_t deviceMask);
void PreCallRecordCmdDispatchBase(VkCommandBuffer commandBuffer, uint32_t baseGroupX, uint32_t baseGroupY, uint32_t baseGroupZ,
                                  uint32_t groupCountX, uint32_t groupCountY, uint32_t groupCountZ);
void PreCallRecordEnumeratePhysicalDeviceGroups(VkInstance instance, uint32_t* pPhysicalDeviceGroupCount,
                                                VkPhysicalDeviceGroupProperties* pPhysicalDeviceGroupProperties);
void PreCallRecordGetImageMemoryRequirements2(VkDevice device, const VkImageMemoryRequirementsInfo2* pInfo,
                                              VkMemoryRequirements2* pMemoryRequirements);
void PreCallRecordGetBufferMemoryRequirements2(VkDevice device, const VkBufferMemoryRequirementsInfo2* pInfo,
                                               VkMemoryRequirements2* pMemoryRequirements);
void PreCallRecordGetImageSparseMemoryRequirements2(VkDevice device, const VkImageSparseMemoryRequirementsInfo2* pInfo,
                                                    uint32_t* pSparseMemoryRequirementCount,
                                                    VkSparseImageMemoryRequirements2* pSparseMemoryRequirements);
void PreCallRecordGetPhysicalDeviceFeatures2(VkPhysicalDevice physicalDevice, VkPhysicalDeviceFeatures2* pFeatures);
void PreCallRecordGetPhysicalDeviceProperties2(VkPhysicalDevice physicalDevice, VkPhysicalDeviceProperties2* pProperties);
void PreCallRecordGetPhysicalDeviceFormatProperties2(VkPhysicalDevice physicalDevice, VkFormat format,
                                                     VkFormatProperties2* pFormatProperties);
void PreCallRecordGetPhysicalDeviceImageFormatProperties2(VkPhysicalDevice physicalDevice,
                                                          const VkPhysicalDeviceImageFormatInfo2* pImageFormatInfo,
                                                          VkImageFormatProperties2* pImageFormatProperties);
void PreCallRecordGetPhysicalDeviceQueueFamilyProperties2(VkPhysicalDevice physicalDevice, uint32_t* pQueueFamilyPropertyCount,
                                                          VkQueueFamilyProperties2* pQueueFamilyProperties);
void PreCallRecordGetPhysicalDeviceMemoryProperties2(VkPhysicalDevice physicalDevice,
                                                     VkPhysicalDeviceMemoryProperties2* pMemoryProperties);
void PreCallRecordGetPhysicalDeviceSparseImageFormatProperties2(VkPhysicalDevice physicalDevice,
                                                                const VkPhysicalDeviceSparseImageFormatInfo2* pFormatInfo,
                                                                uint32_t* pPropertyCount,
                                                                VkSparseImageFormatProperties2* pProperties);
void PreCallRecordTrimCommandPool(VkDevice device, VkCommandPool commandPool, VkCommandPoolTrimFlags flags);
void PreCallRecordGetDeviceQueue2(VkDevice device, const VkDeviceQueueInfo2* pQueueInfo, VkQueue* pQueue);
void PreCallRecordCreateSamplerYcbcrConversion(VkDevice device, const VkSamplerYcbcrConversionCreateInfo* pCreateInfo,
                                               const VkAllocationCallbacks* pAllocator, VkSamplerYcbcrConversion* pYcbcrConversion);
void PreCallRecordDestroySamplerYcbcrConversion(VkDevice device, VkSamplerYcbcrConversion ycbcrConversion,
                                                const VkAllocationCallbacks* pAllocator);
void PreCallRecordCreateDescriptorUpdateTemplate(VkDevice device, const VkDescriptorUpdateTemplateCreateInfo* pCreateInfo,
                                                 const VkAllocationCallbacks* pAllocator,
                                                 VkDescriptorUpdateTemplate* pDescriptorUpdateTemplate);
void PreCallRecordDestroyDescriptorUpdateTemplate(VkDevice device, VkDescriptorUpdateTemplate descriptorUpdateTemplate,
                                                  const VkAllocationCallbacks* pAllocator);
void PreCallRecordUpdateDescriptorSetWithTemplate(VkDevice device, VkDescriptorSet descriptorSet,
                                                  VkDescriptorUpdateTemplate descriptorUpdateTemplate, const void* pData);
void PreCallRecordGetPhysicalDeviceExternalBufferProperties(VkPhysicalDevice physicalDevice,
                                                            const VkPhysicalDeviceExternalBufferInfo* pExternalBufferInfo,
                                                            VkExternalBufferProperties* pExternalBufferProperties);
void PreCallRecordGetPhysicalDeviceExternalFenceProperties(VkPhysicalDevice physicalDevice,
                                                           const VkPhysicalDeviceExternalFenceInfo* pExternalFenceInfo,
                                                           VkExternalFenceProperties* pExternalFenceProperties);
void PreCallRecordGetPhysicalDeviceExternalSemaphoreProperties(VkPhysicalDevice physicalDevice,
                                                               const VkPhysicalDeviceExternalSemaphoreInfo* pExternalSemaphoreInfo,
                                                               VkExternalSemaphoreProperties* pExternalSemaphoreProperties);
void PreCallRecordGetDescriptorSetLayoutSupport(VkDevice device, const VkDescriptorSetLayoutCreateInfo* pCreateInfo,
                                                VkDescriptorSetLayoutSupport* pSupport);
void PreCallRecordDestroySurfaceKHR(VkInstance instance, VkSurfaceKHR surface, const VkAllocationCallbacks* pAllocator);
void PreCallRecordGetPhysicalDeviceSurfaceSupportKHR(VkPhysicalDevice physicalDevice, uint32_t queueFamilyIndex,
                                                     VkSurfaceKHR surface, VkBool32* pSupported);
void PreCallRecordGetPhysicalDeviceSurfaceCapabilitiesKHR(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface,
                                                          VkSurfaceCapabilitiesKHR* pSurfaceCapabilities);
void PreCallRecordGetPhysicalDeviceSurfaceFormatsKHR(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface,
                                                     uint32_t* pSurfaceFormatCount, VkSurfaceFormatKHR* pSurfaceFormats);
void PreCallRecordGetPhysicalDeviceSurfacePresentModesKHR(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface,
                                                          uint32_t* pPresentModeCount, VkPresentModeKHR* pPresentModes);
void PreCallRecordCreateSwapchainKHR(VkDevice device, const VkSwapchainCreateInfoKHR* pCreateInfo,
                                     const VkAllocationCallbacks* pAllocator, VkSwapchainKHR* pSwapchain);
void PreCallRecordDestroySwapchainKHR(VkDevice device, VkSwapchainKHR swapchain, const VkAllocationCallbacks* pAllocator);
void PreCallRecordGetSwapchainImagesKHR(VkDevice device, VkSwapchainKHR swapchain, uint32_t* pSwapchainImageCount,
                                        VkImage* pSwapchainImages);
void PreCallRecordAcquireNextImageKHR(VkDevice device, VkSwapchainKHR swapchain, uint64_t timeout, VkSemaphore semaphore,
                                      VkFence fence, uint32_t* pImageIndex);
void PreCallRecordQueuePresentKHR(VkQueue queue, const VkPresentInfoKHR* pPresentInfo);
void PreCallRecordGetDeviceGroupPresentCapabilitiesKHR(VkDevice device,
                                                       VkDeviceGroupPresentCapabilitiesKHR* pDeviceGroupPresentCapabilities);
void PreCallRecordGetDeviceGroupSurfacePresentModesKHR(VkDevice device, VkSurfaceKHR surface,
                                                       VkDeviceGroupPresentModeFlagsKHR* pModes);
void PreCallRecordGetPhysicalDevicePresentRectanglesKHR(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface, uint32_t* pRectCount,
                                                        VkRect2D* pRects);
void PreCallRecordAcquireNextImage2KHR(VkDevice device, const VkAcquireNextImageInfoKHR* pAcquireInfo, uint32_t* pImageIndex);
void PreCallRecordGetPhysicalDeviceDisplayPropertiesKHR(VkPhysicalDevice physicalDevice, uint32_t* pPropertyCount,
                                                        VkDisplayPropertiesKHR* pProperties);
void PreCallRecordGetPhysicalDeviceDisplayPlanePropertiesKHR(VkPhysicalDevice physicalDevice, uint32_t* pPropertyCount,
                                                             VkDisplayPlanePropertiesKHR* pProperties);
void PreCallRecordGetDisplayPlaneSupportedDisplaysKHR(VkPhysicalDevice physicalDevice, uint32_t planeIndex, uint32_t* pDisplayCount,
                                                      VkDisplayKHR* pDisplays);
void PreCallRecordGetDisplayModePropertiesKHR(VkPhysicalDevice physicalDevice, VkDisplayKHR display, uint32_t* pPropertyCount,
                                              VkDisplayModePropertiesKHR* pProperties);
void PreCallRecordCreateDisplayModeKHR(VkPhysicalDevice physicalDevice, VkDisplayKHR display,
                                       const VkDisplayModeCreateInfoKHR* pCreateInfo, const VkAllocationCallbacks* pAllocator,
                                       VkDisplayModeKHR* pMode);
void PreCallRecordGetDisplayPlaneCapabilitiesKHR(VkPhysicalDevice physicalDevice, VkDisplayModeKHR mode, uint32_t planeIndex,
                                                 VkDisplayPlaneCapabilitiesKHR* pCapabilities);
void PreCallRecordCreateDisplayPlaneSurfaceKHR(VkInstance instance, const VkDisplaySurfaceCreateInfoKHR* pCreateInfo,
                                               const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface);
void PreCallRecordCreateSharedSwapchainsKHR(VkDevice device, uint32_t swapchainCount, const VkSwapchainCreateInfoKHR* pCreateInfos,
                                            const VkAllocationCallbacks* pAllocator, VkSwapchainKHR* pSwapchains);
void PreCallRecordGetPhysicalDeviceFeatures2KHR(VkPhysicalDevice physicalDevice, VkPhysicalDeviceFeatures2* pFeatures);
void PreCallRecordGetPhysicalDeviceProperties2KHR(VkPhysicalDevice physicalDevice, VkPhysicalDeviceProperties2* pProperties);
void PreCallRecordGetPhysicalDeviceFormatProperties2KHR(VkPhysicalDevice physicalDevice, VkFormat format,
                                                        VkFormatProperties2* pFormatProperties);
void PreCallRecordGetPhysicalDeviceImageFormatProperties2KHR(VkPhysicalDevice physicalDevice,
                                                             const VkPhysicalDeviceImageFormatInfo2* pImageFormatInfo,
                                                             VkImageFormatProperties2* pImageFormatProperties);
void PreCallRecordGetPhysicalDeviceQueueFamilyProperties2KHR(VkPhysicalDevice physicalDevice, uint32_t* pQueueFamilyPropertyCount,
                                                             VkQueueFamilyProperties2* pQueueFamilyProperties);
void PreCallRecordGetPhysicalDeviceMemoryProperties2KHR(VkPhysicalDevice physicalDevice,
                                                        VkPhysicalDeviceMemoryProperties2* pMemoryProperties);
void PreCallRecordGetPhysicalDeviceSparseImageFormatProperties2KHR(VkPhysicalDevice physicalDevice,
                                                                   const VkPhysicalDeviceSparseImageFormatInfo2* pFormatInfo,
                                                                   uint32_t* pPropertyCount,
                                                                   VkSparseImageFormatProperties2* pProperties);
void PreCallRecordGetDeviceGroupPeerMemoryFeaturesKHR(VkDevice device, uint32_t heapIndex, uint32_t localDeviceIndex,
                                                      uint32_t remoteDeviceIndex, VkPeerMemoryFeatureFlags* pPeerMemoryFeatures);
void PreCallRecordCmdSetDeviceMaskKHR(VkCommandBuffer commandBuffer, uint32_t deviceMask);
void PreCallRecordCmdDispatchBaseKHR(VkCommandBuffer commandBuffer, uint32_t baseGroupX, uint32_t baseGroupY, uint32_t baseGroupZ,
                                     uint32_t groupCountX, uint32_t groupCountY, uint32_t groupCountZ);
void PreCallRecordTrimCommandPoolKHR(VkDevice device, VkCommandPool commandPool, VkCommandPoolTrimFlags flags);
void PreCallRecordEnumeratePhysicalDeviceGroupsKHR(VkInstance instance, uint32_t* pPhysicalDeviceGroupCount,
                                                   VkPhysicalDeviceGroupProperties* pPhysicalDeviceGroupProperties);
void PreCallRecordGetPhysicalDeviceExternalBufferPropertiesKHR(VkPhysicalDevice physicalDevice,
                                                               const VkPhysicalDeviceExternalBufferInfo* pExternalBufferInfo,
                                                               VkExternalBufferProperties* pExternalBufferProperties);
void PreCallRecordGetMemoryFdKHR(VkDevice device, const VkMemoryGetFdInfoKHR* pGetFdInfo, int* pFd);
void PreCallRecordGetMemoryFdPropertiesKHR(VkDevice device, VkExternalMemoryHandleTypeFlagBits handleType, int fd,
                                           VkMemoryFdPropertiesKHR* pMemoryFdProperties);
void PreCallRecordGetPhysicalDeviceExternalSemaphorePropertiesKHR(
    VkPhysicalDevice physicalDevice, const VkPhysicalDeviceExternalSemaphoreInfo* pExternalSemaphoreInfo,
    VkExternalSemaphoreProperties* pExternalSemaphoreProperties);
void PreCallRecordImportSemaphoreFdKHR(VkDevice device, const VkImportSemaphoreFdInfoKHR* pImportSemaphoreFdInfo);
void PreCallRecordGetSemaphoreFdKHR(VkDevice device, const VkSemaphoreGetFdInfoKHR* pGetFdInfo, int* pFd);
void PreCallRecordCmdPushDescriptorSetKHR(VkCommandBuffer commandBuffer, VkPipelineBindPoint pipelineBindPoint,
                                          VkPipelineLayout layout, uint32_t set, uint32_t descriptorWriteCount,
                                          const VkWriteDescriptorSet* pDescriptorWrites);
void PreCallRecordCmdPushDescriptorSetWithTemplateKHR(VkCommandBuffer commandBuffer,
                                                      VkDescriptorUpdateTemplate descriptorUpdateTemplate, VkPipelineLayout layout,
                                                      uint32_t set, const void* pData);
void PreCallRecordCreateDescriptorUpdateTemplateKHR(VkDevice device, const VkDescriptorUpdateTemplateCreateInfo* pCreateInfo,
                                                    const VkAllocationCallbacks* pAllocator,
                                                    VkDescriptorUpdateTemplate* pDescriptorUpdateTemplate);
void PreCallRecordDestroyDescriptorUpdateTemplateKHR(VkDevice device, VkDescriptorUpdateTemplate descriptorUpdateTemplate,
                                                     const VkAllocationCallbacks* pAllocator);
void PreCallRecordUpdateDescriptorSetWithTemplateKHR(VkDevice device, VkDescriptorSet descriptorSet,
                                                     VkDescriptorUpdateTemplate descriptorUpdateTemplate, const void* pData);
void PreCallRecordGetSwapchainStatusKHR(VkDevice device, VkSwapchainKHR swapchain);
void PreCallRecordGetPhysicalDeviceExternalFencePropertiesKHR(VkPhysicalDevice physicalDevice,
                                                              const VkPhysicalDeviceExternalFenceInfo* pExternalFenceInfo,
                                                              VkExternalFenceProperties* pExternalFenceProperties);
void PreCallRecordImportFenceFdKHR(VkDevice device, const VkImportFenceFdInfoKHR* pImportFenceFdInfo);
void PreCallRecordGetFenceFdKHR(VkDevice device, const VkFenceGetFdInfoKHR* pGetFdInfo, int* pFd);
void PreCallRecordGetPhysicalDeviceSurfaceCapabilities2KHR(VkPhysicalDevice physicalDevice,
                                                           const VkPhysicalDeviceSurfaceInfo2KHR* pSurfaceInfo,
                                                           VkSurfaceCapabilities2KHR* pSurfaceCapabilities);
void PreCallRecordGetPhysicalDeviceSurfaceFormats2KHR(VkPhysicalDevice physicalDevice,
                                                      const VkPhysicalDeviceSurfaceInfo2KHR* pSurfaceInfo,
                                                      uint32_t* pSurfaceFormatCount, VkSurfaceFormat2KHR* pSurfaceFormats);
void PreCallRecordGetPhysicalDeviceDisplayProperties2KHR(VkPhysicalDevice physicalDevice, uint32_t* pPropertyCount,
                                                         VkDisplayProperties2KHR* pProperties);
void PreCallRecordGetPhysicalDeviceDisplayPlaneProperties2KHR(VkPhysicalDevice physicalDevice, uint32_t* pPropertyCount,
                                                              VkDisplayPlaneProperties2KHR* pProperties);
void PreCallRecordGetDisplayModeProperties2KHR(VkPhysicalDevice physicalDevice, VkDisplayKHR display, uint32_t* pPropertyCount,
                                               VkDisplayModeProperties2KHR* pProperties);
void PreCallRecordGetDisplayPlaneCapabilities2KHR(VkPhysicalDevice physicalDevice, const VkDisplayPlaneInfo2KHR* pDisplayPlaneInfo,
                                                  VkDisplayPlaneCapabilities2KHR* pCapabilities);
void PreCallRecordGetImageMemoryRequirements2KHR(VkDevice device, const VkImageMemoryRequirementsInfo2* pInfo,
                                                 VkMemoryRequirements2* pMemoryRequirements);
void PreCallRecordGetBufferMemoryRequirements2KHR(VkDevice device, const VkBufferMemoryRequirementsInfo2* pInfo,
                                                  VkMemoryRequirements2* pMemoryRequirements);
void PreCallRecordGetImageSparseMemoryRequirements2KHR(VkDevice device, const VkImageSparseMemoryRequirementsInfo2* pInfo,
                                                       uint32_t* pSparseMemoryRequirementCount,
                                                       VkSparseImageMemoryRequirements2* pSparseMemoryRequirements);
void PreCallRecordCreateSamplerYcbcrConversionKHR(VkDevice device, const VkSamplerYcbcrConversionCreateInfo* pCreateInfo,
                                                  const VkAllocationCallbacks* pAllocator,
                                                  VkSamplerYcbcrConversion* pYcbcrConversion);
void PreCallRecordDestroySamplerYcbcrConversionKHR(VkDevice device, VkSamplerYcbcrConversion ycbcrConversion,
                                                   const VkAllocationCallbacks* pAllocator);
void PreCallRecordBindBufferMemory2KHR(VkDevice device, uint32_t bindInfoCount, const VkBindBufferMemoryInfo* pBindInfos);
void PreCallRecordBindImageMemory2KHR(VkDevice device, uint32_t bindInfoCount, const VkBindImageMemoryInfo* pBindInfos);
void PreCallRecordGetDescriptorSetLayoutSupportKHR(VkDevice device, const VkDescriptorSetLayoutCreateInfo* pCreateInfo,
                                                   VkDescriptorSetLayoutSupport* pSupport);
void PreCallRecordCmdDrawIndirectCountKHR(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset, VkBuffer countBuffer,
                                          VkDeviceSize countBufferOffset, uint32_t maxDrawCount, uint32_t stride);
void PreCallRecordCmdDrawIndexedIndirectCountKHR(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                 VkBuffer countBuffer, VkDeviceSize countBufferOffset, uint32_t maxDrawCount,
                                                 uint32_t stride);
void PreCallRecordCreateDebugReportCallbackEXT(VkInstance instance, const VkDebugReportCallbackCreateInfoEXT* pCreateInfo,
                                               const VkAllocationCallbacks* pAllocator, VkDebugReportCallbackEXT* pCallback);
void PreCallRecordDestroyDebugReportCallbackEXT(VkInstance instance, VkDebugReportCallbackEXT callback,
                                                const VkAllocationCallbacks* pAllocator);
void PreCallRecordDebugReportMessageEXT(VkInstance instance, VkDebugReportFlagsEXT flags, VkDebugReportObjectTypeEXT objectType,
                                        uint64_t object, size_t location, int32_t messageCode, const char* pLayerPrefix,
                                        const char* pMessage);
void PreCallRecordDebugMarkerSetObjectTagEXT(VkDevice device, const VkDebugMarkerObjectTagInfoEXT* pTagInfo);
void PreCallRecordDebugMarkerSetObjectNameEXT(VkDevice device, const VkDebugMarkerObjectNameInfoEXT* pNameInfo);
void PreCallRecordCmdDebugMarkerBeginEXT(VkCommandBuffer commandBuffer, const VkDebugMarkerMarkerInfoEXT* pMarkerInfo);
void PreCallRecordCmdDebugMarkerEndEXT(VkCommandBuffer commandBuffer);
void PreCallRecordCmdDebugMarkerInsertEXT(VkCommandBuffer commandBuffer, const VkDebugMarkerMarkerInfoEXT* pMarkerInfo);
void PreCallRecordCmdDrawIndirectCountAMD(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset, VkBuffer countBuffer,
                                          VkDeviceSize countBufferOffset, uint32_t maxDrawCount, uint32_t stride);
void PreCallRecordCmdDrawIndexedIndirectCountAMD(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                 VkBuffer countBuffer, VkDeviceSize countBufferOffset, uint32_t maxDrawCount,
                                                 uint32_t stride);
void PreCallRecordGetShaderInfoAMD(VkDevice device, VkPipeline pipeline, VkShaderStageFlagBits shaderStage,
                                   VkShaderInfoTypeAMD infoType, size_t* pInfoSize, void* pInfo);
void PreCallRecordGetPhysicalDeviceExternalImageFormatPropertiesNV(
    VkPhysicalDevice physicalDevice, VkFormat format, VkImageType type, VkImageTiling tiling, VkImageUsageFlags usage,
    VkImageCreateFlags flags, VkExternalMemoryHandleTypeFlagsNV externalHandleType,
    VkExternalImageFormatPropertiesNV* pExternalImageFormatProperties);
void PreCallRecordCmdProcessCommandsNVX(VkCommandBuffer commandBuffer, const VkCmdProcessCommandsInfoNVX* pProcessCommandsInfo);
void PreCallRecordCmdReserveSpaceForCommandsNVX(VkCommandBuffer commandBuffer,
                                                const VkCmdReserveSpaceForCommandsInfoNVX* pReserveSpaceInfo);
void PreCallRecordCreateIndirectCommandsLayoutNVX(VkDevice device, const VkIndirectCommandsLayoutCreateInfoNVX* pCreateInfo,
                                                  const VkAllocationCallbacks* pAllocator,
                                                  VkIndirectCommandsLayoutNVX* pIndirectCommandsLayout);
void PreCallRecordDestroyIndirectCommandsLayoutNVX(VkDevice device, VkIndirectCommandsLayoutNVX indirectCommandsLayout,
                                                   const VkAllocationCallbacks* pAllocator);
void PreCallRecordCreateObjectTableNVX(VkDevice device, const VkObjectTableCreateInfoNVX* pCreateInfo,
                                       const VkAllocationCallbacks* pAllocator, VkObjectTableNVX* pObjectTable);
void PreCallRecordDestroyObjectTableNVX(VkDevice device, VkObjectTableNVX objectTable, const VkAllocationCallbacks* pAllocator);
void PreCallRecordRegisterObjectsNVX(VkDevice device, VkObjectTableNVX objectTable, uint32_t objectCount,
                                     const VkObjectTableEntryNVX* const* ppObjectTableEntries, const uint32_t* pObjectIndices);
void PreCallRecordUnregisterObjectsNVX(VkDevice device, VkObjectTableNVX objectTable, uint32_t objectCount,
                                       const VkObjectEntryTypeNVX* pObjectEntryTypes, const uint32_t* pObjectIndices);
void PreCallRecordGetPhysicalDeviceGeneratedCommandsPropertiesNVX(VkPhysicalDevice physicalDevice,
                                                                  VkDeviceGeneratedCommandsFeaturesNVX* pFeatures,
                                                                  VkDeviceGeneratedCommandsLimitsNVX* pLimits);
void PreCallRecordCmdSetViewportWScalingNV(VkCommandBuffer commandBuffer, uint32_t firstViewport, uint32_t viewportCount,
                                           const VkViewportWScalingNV* pViewportWScalings);
void PreCallRecordReleaseDisplayEXT(VkPhysicalDevice physicalDevice, VkDisplayKHR display);
void PreCallRecordGetPhysicalDeviceSurfaceCapabilities2EXT(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface,
                                                           VkSurfaceCapabilities2EXT* pSurfaceCapabilities);
void PreCallRecordDisplayPowerControlEXT(VkDevice device, VkDisplayKHR display, const VkDisplayPowerInfoEXT* pDisplayPowerInfo);
void PreCallRecordRegisterDeviceEventEXT(VkDevice device, const VkDeviceEventInfoEXT* pDeviceEventInfo,
                                         const VkAllocationCallbacks* pAllocator, VkFence* pFence);
void PreCallRecordRegisterDisplayEventEXT(VkDevice device, VkDisplayKHR display, const VkDisplayEventInfoEXT* pDisplayEventInfo,
                                          const VkAllocationCallbacks* pAllocator, VkFence* pFence);
void PreCallRecordGetSwapchainCounterEXT(VkDevice device, VkSwapchainKHR swapchain, VkSurfaceCounterFlagBitsEXT counter,
                                         uint64_t* pCounterValue);
void PreCallRecordGetRefreshCycleDurationGOOGLE(VkDevice device, VkSwapchainKHR swapchain,
                                                VkRefreshCycleDurationGOOGLE* pDisplayTimingProperties);
void PreCallRecordGetPastPresentationTimingGOOGLE(VkDevice device, VkSwapchainKHR swapchain, uint32_t* pPresentationTimingCount,
                                                  VkPastPresentationTimingGOOGLE* pPresentationTimings);
void PreCallRecordCmdSetDiscardRectangleEXT(VkCommandBuffer commandBuffer, uint32_t firstDiscardRectangle,
                                            uint32_t discardRectangleCount, const VkRect2D* pDiscardRectangles);
void PreCallRecordSetHdrMetadataEXT(VkDevice device, uint32_t swapchainCount, const VkSwapchainKHR* pSwapchains,
                                    const VkHdrMetadataEXT* pMetadata);
void PreCallRecordSetDebugUtilsObjectNameEXT(VkDevice device, const VkDebugUtilsObjectNameInfoEXT* pNameInfo);
void PreCallRecordSetDebugUtilsObjectTagEXT(VkDevice device, const VkDebugUtilsObjectTagInfoEXT* pTagInfo);
void PreCallRecordQueueBeginDebugUtilsLabelEXT(VkQueue queue, const VkDebugUtilsLabelEXT* pLabelInfo);
void PreCallRecordQueueEndDebugUtilsLabelEXT(VkQueue queue);
void PreCallRecordQueueInsertDebugUtilsLabelEXT(VkQueue queue, const VkDebugUtilsLabelEXT* pLabelInfo);
void PreCallRecordCmdBeginDebugUtilsLabelEXT(VkCommandBuffer commandBuffer, const VkDebugUtilsLabelEXT* pLabelInfo);
void PreCallRecordCmdEndDebugUtilsLabelEXT(VkCommandBuffer commandBuffer);
void PreCallRecordCmdInsertDebugUtilsLabelEXT(VkCommandBuffer commandBuffer, const VkDebugUtilsLabelEXT* pLabelInfo);
void PreCallRecordCreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
                                               const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pMessenger);
void PreCallRecordDestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT messenger,
                                                const VkAllocationCallbacks* pAllocator);
void PreCallRecordSubmitDebugUtilsMessageEXT(VkInstance instance, VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
                                             VkDebugUtilsMessageTypeFlagsEXT messageTypes,
                                             const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData);
void PreCallRecordCmdSetSampleLocationsEXT(VkCommandBuffer commandBuffer, const VkSampleLocationsInfoEXT* pSampleLocationsInfo);
void PreCallRecordGetPhysicalDeviceMultisamplePropertiesEXT(VkPhysicalDevice physicalDevice, VkSampleCountFlagBits samples,
                                                            VkMultisamplePropertiesEXT* pMultisampleProperties);
void PreCallRecordCreateValidationCacheEXT(VkDevice device, const VkValidationCacheCreateInfoEXT* pCreateInfo,
                                           const VkAllocationCallbacks* pAllocator, VkValidationCacheEXT* pValidationCache);
void PreCallRecordDestroyValidationCacheEXT(VkDevice device, VkValidationCacheEXT validationCache,
                                            const VkAllocationCallbacks* pAllocator);
void PreCallRecordMergeValidationCachesEXT(VkDevice device, VkValidationCacheEXT dstCache, uint32_t srcCacheCount,
                                           const VkValidationCacheEXT* pSrcCaches);
void PreCallRecordGetValidationCacheDataEXT(VkDevice device, VkValidationCacheEXT validationCache, size_t* pDataSize, void* pData);
void PreCallRecordGetMemoryHostPointerPropertiesEXT(VkDevice device, VkExternalMemoryHandleTypeFlagBits handleType,
                                                    const void* pHostPointer,
                                                    VkMemoryHostPointerPropertiesEXT* pMemoryHostPointerProperties);
void PreCallRecordCmdWriteBufferMarkerAMD(VkCommandBuffer commandBuffer, VkPipelineStageFlagBits pipelineStage, VkBuffer dstBuffer,
                                          VkDeviceSize dstOffset, uint32_t marker);

bool PreCallValidateCmdBindShadingRateImageNV(VkCommandBuffer commandBuffer, VkImageView imageView, VkImageLayout imageLayout);
void PreCallRecordCmdBindShadingRateImageNV(VkCommandBuffer commandBuffer, VkImageView imageView, VkImageLayout imageLayout);
void PostCallRecordCmdBindShadingRateImageNV(VkCommandBuffer commandBuffer, VkImageView imageView, VkImageLayout imageLayout);
bool PreCallValidateCmdSetViewportShadingRatePaletteNV(VkCommandBuffer commandBuffer, uint32_t firstViewport,
                                                       uint32_t viewportCount, const VkShadingRatePaletteNV* pShadingRatePalettes);
void PreCallRecordCmdSetViewportShadingRatePaletteNV(VkCommandBuffer commandBuffer, uint32_t firstViewport, uint32_t viewportCount,
                                                     const VkShadingRatePaletteNV* pShadingRatePalettes);
void PostCallRecordCmdSetViewportShadingRatePaletteNV(VkCommandBuffer commandBuffer, uint32_t firstViewport, uint32_t viewportCount,
                                                      const VkShadingRatePaletteNV* pShadingRatePalettes);
bool PreCallValidateCmdSetCoarseSampleOrderNV(VkCommandBuffer commandBuffer, VkCoarseSampleOrderTypeNV sampleOrderType,
                                              uint32_t customSampleOrderCount,
                                              const VkCoarseSampleOrderCustomNV* pCustomSampleOrders);
void PreCallRecordCmdSetCoarseSampleOrderNV(VkCommandBuffer commandBuffer, VkCoarseSampleOrderTypeNV sampleOrderType,
                                            uint32_t customSampleOrderCount,
                                            const VkCoarseSampleOrderCustomNV* pCustomSampleOrders);
void PostCallRecordCmdSetCoarseSampleOrderNV(VkCommandBuffer commandBuffer, VkCoarseSampleOrderTypeNV sampleOrderType,
                                             uint32_t customSampleOrderCount,
                                             const VkCoarseSampleOrderCustomNV* pCustomSampleOrders);
bool PreCallValidateCreateAccelerationStructureNVX(VkDevice device, const VkAccelerationStructureCreateInfoNVX* pCreateInfo,
                                                   const VkAllocationCallbacks* pAllocator,
                                                   VkAccelerationStructureNVX* pAccelerationStructure);
void PreCallRecordCreateAccelerationStructureNVX(VkDevice device, const VkAccelerationStructureCreateInfoNVX* pCreateInfo,
                                                 const VkAllocationCallbacks* pAllocator,
                                                 VkAccelerationStructureNVX* pAccelerationStructure);
void PostCallRecordCreateAccelerationStructureNVX(VkDevice device, const VkAccelerationStructureCreateInfoNVX* pCreateInfo,
                                                  const VkAllocationCallbacks* pAllocator,
                                                  VkAccelerationStructureNVX* pAccelerationStructure);
bool PreCallValidateDestroyAccelerationStructureNVX(VkDevice device, VkAccelerationStructureNVX accelerationStructure,
                                                    const VkAllocationCallbacks* pAllocator);
void PreCallRecordDestroyAccelerationStructureNVX(VkDevice device, VkAccelerationStructureNVX accelerationStructure,
                                                  const VkAllocationCallbacks* pAllocator);
void PostCallRecordDestroyAccelerationStructureNVX(VkDevice device, VkAccelerationStructureNVX accelerationStructure,
                                                   const VkAllocationCallbacks* pAllocator);
bool PreCallValidateGetAccelerationStructureMemoryRequirementsNVX(VkDevice device,
                                                                  const VkAccelerationStructureMemoryRequirementsInfoNVX* pInfo,
                                                                  VkMemoryRequirements2KHR* pMemoryRequirements);
void PreCallRecordGetAccelerationStructureMemoryRequirementsNVX(VkDevice device,
                                                                const VkAccelerationStructureMemoryRequirementsInfoNVX* pInfo,
                                                                VkMemoryRequirements2KHR* pMemoryRequirements);
void PostCallRecordGetAccelerationStructureMemoryRequirementsNVX(VkDevice device,
                                                                 const VkAccelerationStructureMemoryRequirementsInfoNVX* pInfo,
                                                                 VkMemoryRequirements2KHR* pMemoryRequirements);
bool PreCallValidateGetAccelerationStructureScratchMemoryRequirementsNVX(
    VkDevice device, const VkAccelerationStructureMemoryRequirementsInfoNVX* pInfo, VkMemoryRequirements2KHR* pMemoryRequirements);
void PreCallRecordGetAccelerationStructureScratchMemoryRequirementsNVX(
    VkDevice device, const VkAccelerationStructureMemoryRequirementsInfoNVX* pInfo, VkMemoryRequirements2KHR* pMemoryRequirements);
void PostCallRecordGetAccelerationStructureScratchMemoryRequirementsNVX(
    VkDevice device, const VkAccelerationStructureMemoryRequirementsInfoNVX* pInfo, VkMemoryRequirements2KHR* pMemoryRequirements);
bool PreCallValidateBindAccelerationStructureMemoryNVX(VkDevice device, uint32_t bindInfoCount,
                                                       const VkBindAccelerationStructureMemoryInfoNVX* pBindInfos);
void PreCallRecordBindAccelerationStructureMemoryNVX(VkDevice device, uint32_t bindInfoCount,
                                                     const VkBindAccelerationStructureMemoryInfoNVX* pBindInfos);
void PostCallRecordBindAccelerationStructureMemoryNVX(VkDevice device, uint32_t bindInfoCount,
                                                      const VkBindAccelerationStructureMemoryInfoNVX* pBindInfos);
bool PreCallValidateCmdBuildAccelerationStructureNVX(VkCommandBuffer cmdBuf, VkAccelerationStructureTypeNVX type,
                                                     uint32_t instanceCount, VkBuffer instanceData, VkDeviceSize instanceOffset,
                                                     uint32_t geometryCount, const VkGeometryNVX* pGeometries,
                                                     VkBuildAccelerationStructureFlagsNVX flags, VkBool32 update,
                                                     VkAccelerationStructureNVX dst, VkAccelerationStructureNVX src,
                                                     VkBuffer scratch, VkDeviceSize scratchOffset);
void PreCallRecordCmdBuildAccelerationStructureNVX(VkCommandBuffer cmdBuf, VkAccelerationStructureTypeNVX type,
                                                   uint32_t instanceCount, VkBuffer instanceData, VkDeviceSize instanceOffset,
                                                   uint32_t geometryCount, const VkGeometryNVX* pGeometries,
                                                   VkBuildAccelerationStructureFlagsNVX flags, VkBool32 update,
                                                   VkAccelerationStructureNVX dst, VkAccelerationStructureNVX src, VkBuffer scratch,
                                                   VkDeviceSize scratchOffset);
void PostCallRecordCmdBuildAccelerationStructureNVX(VkCommandBuffer cmdBuf, VkAccelerationStructureTypeNVX type,
                                                    uint32_t instanceCount, VkBuffer instanceData, VkDeviceSize instanceOffset,
                                                    uint32_t geometryCount, const VkGeometryNVX* pGeometries,
                                                    VkBuildAccelerationStructureFlagsNVX flags, VkBool32 update,
                                                    VkAccelerationStructureNVX dst, VkAccelerationStructureNVX src,
                                                    VkBuffer scratch, VkDeviceSize scratchOffset);
bool PreCallValidateCmdCopyAccelerationStructureNVX(VkCommandBuffer cmdBuf, VkAccelerationStructureNVX dst,
                                                    VkAccelerationStructureNVX src, VkCopyAccelerationStructureModeNVX mode);
void PreCallRecordCmdCopyAccelerationStructureNVX(VkCommandBuffer cmdBuf, VkAccelerationStructureNVX dst,
                                                  VkAccelerationStructureNVX src, VkCopyAccelerationStructureModeNVX mode);
void PostCallRecordCmdCopyAccelerationStructureNVX(VkCommandBuffer cmdBuf, VkAccelerationStructureNVX dst,
                                                   VkAccelerationStructureNVX src, VkCopyAccelerationStructureModeNVX mode);
bool PreCallValidateCmdTraceRaysNVX(VkCommandBuffer cmdBuf, VkBuffer raygenShaderBindingTableBuffer,
                                    VkDeviceSize raygenShaderBindingOffset, VkBuffer missShaderBindingTableBuffer,
                                    VkDeviceSize missShaderBindingOffset, VkDeviceSize missShaderBindingStride,
                                    VkBuffer hitShaderBindingTableBuffer, VkDeviceSize hitShaderBindingOffset,
                                    VkDeviceSize hitShaderBindingStride, uint32_t width, uint32_t height);
void PreCallRecordCmdTraceRaysNVX(VkCommandBuffer cmdBuf, VkBuffer raygenShaderBindingTableBuffer,
                                  VkDeviceSize raygenShaderBindingOffset, VkBuffer missShaderBindingTableBuffer,
                                  VkDeviceSize missShaderBindingOffset, VkDeviceSize missShaderBindingStride,
                                  VkBuffer hitShaderBindingTableBuffer, VkDeviceSize hitShaderBindingOffset,
                                  VkDeviceSize hitShaderBindingStride, uint32_t width, uint32_t height);
void PostCallRecordCmdTraceRaysNVX(VkCommandBuffer cmdBuf, VkBuffer raygenShaderBindingTableBuffer,
                                   VkDeviceSize raygenShaderBindingOffset, VkBuffer missShaderBindingTableBuffer,
                                   VkDeviceSize missShaderBindingOffset, VkDeviceSize missShaderBindingStride,
                                   VkBuffer hitShaderBindingTableBuffer, VkDeviceSize hitShaderBindingOffset,
                                   VkDeviceSize hitShaderBindingStride, uint32_t width, uint32_t height);
bool PreCallValidateCreateRaytracingPipelinesNVX(VkDevice device, VkPipelineCache pipelineCache, uint32_t createInfoCount,
                                                 const VkRaytracingPipelineCreateInfoNVX* pCreateInfos,
                                                 const VkAllocationCallbacks* pAllocator, VkPipeline* pPipelines);
void PreCallRecordCreateRaytracingPipelinesNVX(VkDevice device, VkPipelineCache pipelineCache, uint32_t createInfoCount,
                                               const VkRaytracingPipelineCreateInfoNVX* pCreateInfos,
                                               const VkAllocationCallbacks* pAllocator, VkPipeline* pPipelines);
void PostCallRecordCreateRaytracingPipelinesNVX(VkDevice device, VkPipelineCache pipelineCache, uint32_t createInfoCount,
                                                const VkRaytracingPipelineCreateInfoNVX* pCreateInfos,
                                                const VkAllocationCallbacks* pAllocator, VkPipeline* pPipelines);
bool PreCallValidateGetRaytracingShaderHandlesNVX(VkDevice device, VkPipeline pipeline, uint32_t firstGroup, uint32_t groupCount,
                                                  size_t dataSize, void* pData);
void PreCallRecordGetRaytracingShaderHandlesNVX(VkDevice device, VkPipeline pipeline, uint32_t firstGroup, uint32_t groupCount,
                                                size_t dataSize, void* pData);
void PostCallRecordGetRaytracingShaderHandlesNVX(VkDevice device, VkPipeline pipeline, uint32_t firstGroup, uint32_t groupCount,
                                                 size_t dataSize, void* pData);
bool PreCallValidateGetAccelerationStructureHandleNVX(VkDevice device, VkAccelerationStructureNVX accelerationStructure,
                                                      size_t dataSize, void* pData);
void PreCallRecordGetAccelerationStructureHandleNVX(VkDevice device, VkAccelerationStructureNVX accelerationStructure,
                                                    size_t dataSize, void* pData);
void PostCallRecordGetAccelerationStructureHandleNVX(VkDevice device, VkAccelerationStructureNVX accelerationStructure,
                                                     size_t dataSize, void* pData);
bool PreCallValidateCmdWriteAccelerationStructurePropertiesNVX(VkCommandBuffer cmdBuf,
                                                               VkAccelerationStructureNVX accelerationStructure,
                                                               VkQueryType queryType, VkQueryPool queryPool, uint32_t query);
void PreCallRecordCmdWriteAccelerationStructurePropertiesNVX(VkCommandBuffer cmdBuf,
                                                             VkAccelerationStructureNVX accelerationStructure,
                                                             VkQueryType queryType, VkQueryPool queryPool, uint32_t query);
void PostCallRecordCmdWriteAccelerationStructurePropertiesNVX(VkCommandBuffer cmdBuf,
                                                              VkAccelerationStructureNVX accelerationStructure,
                                                              VkQueryType queryType, VkQueryPool queryPool, uint32_t query);
bool PreCallValidateCompileDeferredNVX(VkDevice device, VkPipeline pipeline, uint32_t shader);
void PreCallRecordCompileDeferredNVX(VkDevice device, VkPipeline pipeline, uint32_t shader);
void PostCallRecordCompileDeferredNVX(VkDevice device, VkPipeline pipeline, uint32_t shader);
void PostCallRecordCmdWriteBufferMarkerAMD(VkCommandBuffer commandBuffer, VkPipelineStageFlagBits pipelineStage, VkBuffer dstBuffer,
                                           VkDeviceSize dstOffset, uint32_t marker);
bool PreCallValidateCmdDrawMeshTasksNV(VkCommandBuffer commandBuffer, uint32_t taskCount, uint32_t firstTask);
void PreCallRecordCmdDrawMeshTasksNV(VkCommandBuffer commandBuffer, uint32_t taskCount, uint32_t firstTask);
void PostCallRecordCmdDrawMeshTasksNV(VkCommandBuffer commandBuffer, uint32_t taskCount, uint32_t firstTask);
bool PreCallValidateCmdDrawMeshTasksIndirectNV(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                               uint32_t drawCount, uint32_t stride);
void PreCallRecordCmdDrawMeshTasksIndirectNV(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                             uint32_t drawCount, uint32_t stride);
void PostCallRecordCmdDrawMeshTasksIndirectNV(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                              uint32_t drawCount, uint32_t stride);
bool PreCallValidateCmdDrawMeshTasksIndirectCountNV(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                    VkBuffer countBuffer, VkDeviceSize countBufferOffset, uint32_t maxDrawCount,
                                                    uint32_t stride);
void PreCallRecordCmdDrawMeshTasksIndirectCountNV(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                  VkBuffer countBuffer, VkDeviceSize countBufferOffset, uint32_t maxDrawCount,
                                                  uint32_t stride);
void PostCallRecordCmdDrawMeshTasksIndirectCountNV(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                   VkBuffer countBuffer, VkDeviceSize countBufferOffset, uint32_t maxDrawCount,
                                                   uint32_t stride);
bool PreCallValidateCmdSetExclusiveScissorNV(VkCommandBuffer commandBuffer, uint32_t firstExclusiveScissor,
                                             uint32_t exclusiveScissorCount, const VkRect2D* pExclusiveScissors);
void PreCallRecordCmdSetExclusiveScissorNV(VkCommandBuffer commandBuffer, uint32_t firstExclusiveScissor,
                                           uint32_t exclusiveScissorCount, const VkRect2D* pExclusiveScissors);
void PostCallRecordCmdSetExclusiveScissorNV(VkCommandBuffer commandBuffer, uint32_t firstExclusiveScissor,
                                            uint32_t exclusiveScissorCount, const VkRect2D* pExclusiveScissors);
bool PreCallValidateCmdSetCheckpointNV(VkCommandBuffer commandBuffer, const void* pCheckpointMarker);
void PreCallRecordCmdSetCheckpointNV(VkCommandBuffer commandBuffer, const void* pCheckpointMarker);
void PostCallRecordCmdSetCheckpointNV(VkCommandBuffer commandBuffer, const void* pCheckpointMarker);
bool PreCallValidateGetQueueCheckpointDataNV(VkQueue queue, uint32_t* pCheckpointDataCount, VkCheckpointDataNV* pCheckpointData);
void PreCallRecordGetQueueCheckpointDataNV(VkQueue queue, uint32_t* pCheckpointDataCount, VkCheckpointDataNV* pCheckpointData);
void PostCallRecordGetQueueCheckpointDataNV(VkQueue queue, uint32_t* pCheckpointDataCount, VkCheckpointDataNV* pCheckpointData);
