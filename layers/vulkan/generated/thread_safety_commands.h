// *** THIS FILE IS GENERATED - DO NOT EDIT ***
// See thread_safety_generator.py for modifications

/***************************************************************************
 *
 * Copyright (c) 2015-2023 The Khronos Group Inc.
 * Copyright (c) 2015-2023 Valve Corporation
 * Copyright (c) 2015-2023 LunarG, Inc.
 * Copyright (c) 2015-2023 Google Inc.
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
void PreCallRecordCreateInstance(const VkInstanceCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator,
                                 VkInstance* pInstance) override;

void PostCallRecordCreateInstance(const VkInstanceCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator,
                                  VkInstance* pInstance, const RecordObject& record_obj) override;

void PreCallRecordDestroyInstance(VkInstance instance, const VkAllocationCallbacks* pAllocator) override;

void PostCallRecordDestroyInstance(VkInstance instance, const VkAllocationCallbacks* pAllocator,
                                   const RecordObject& record_obj) override;

void PreCallRecordEnumeratePhysicalDevices(VkInstance instance, uint32_t* pPhysicalDeviceCount,
                                           VkPhysicalDevice* pPhysicalDevices) override;

void PostCallRecordEnumeratePhysicalDevices(VkInstance instance, uint32_t* pPhysicalDeviceCount, VkPhysicalDevice* pPhysicalDevices,
                                            const RecordObject& record_obj) override;

void PreCallRecordGetInstanceProcAddr(VkInstance instance, const char* pName) override;

void PostCallRecordGetInstanceProcAddr(VkInstance instance, const char* pName, const RecordObject& record_obj) override;

void PreCallRecordGetDeviceProcAddr(VkDevice device, const char* pName) override;

void PostCallRecordGetDeviceProcAddr(VkDevice device, const char* pName, const RecordObject& record_obj) override;

void PreCallRecordCreateDevice(VkPhysicalDevice physicalDevice, const VkDeviceCreateInfo* pCreateInfo,
                               const VkAllocationCallbacks* pAllocator, VkDevice* pDevice) override;

void PostCallRecordCreateDevice(VkPhysicalDevice physicalDevice, const VkDeviceCreateInfo* pCreateInfo,
                                const VkAllocationCallbacks* pAllocator, VkDevice* pDevice,
                                const RecordObject& record_obj) override;

void PreCallRecordDestroyDevice(VkDevice device, const VkAllocationCallbacks* pAllocator) override;

void PostCallRecordDestroyDevice(VkDevice device, const VkAllocationCallbacks* pAllocator, const RecordObject& record_obj) override;

void PreCallRecordGetDeviceQueue(VkDevice device, uint32_t queueFamilyIndex, uint32_t queueIndex, VkQueue* pQueue) override;

void PostCallRecordGetDeviceQueue(VkDevice device, uint32_t queueFamilyIndex, uint32_t queueIndex, VkQueue* pQueue,
                                  const RecordObject& record_obj) override;

void PreCallRecordQueueSubmit(VkQueue queue, uint32_t submitCount, const VkSubmitInfo* pSubmits, VkFence fence) override;

void PostCallRecordQueueSubmit(VkQueue queue, uint32_t submitCount, const VkSubmitInfo* pSubmits, VkFence fence,
                               const RecordObject& record_obj) override;

void PreCallRecordQueueWaitIdle(VkQueue queue) override;

void PostCallRecordQueueWaitIdle(VkQueue queue, const RecordObject& record_obj) override;

void PreCallRecordDeviceWaitIdle(VkDevice device) override;

void PostCallRecordDeviceWaitIdle(VkDevice device, const RecordObject& record_obj) override;

void PreCallRecordAllocateMemory(VkDevice device, const VkMemoryAllocateInfo* pAllocateInfo,
                                 const VkAllocationCallbacks* pAllocator, VkDeviceMemory* pMemory) override;

void PostCallRecordAllocateMemory(VkDevice device, const VkMemoryAllocateInfo* pAllocateInfo,
                                  const VkAllocationCallbacks* pAllocator, VkDeviceMemory* pMemory,
                                  const RecordObject& record_obj) override;

void PreCallRecordFreeMemory(VkDevice device, VkDeviceMemory memory, const VkAllocationCallbacks* pAllocator) override;

void PostCallRecordFreeMemory(VkDevice device, VkDeviceMemory memory, const VkAllocationCallbacks* pAllocator,
                              const RecordObject& record_obj) override;

void PreCallRecordMapMemory(VkDevice device, VkDeviceMemory memory, VkDeviceSize offset, VkDeviceSize size, VkMemoryMapFlags flags,
                            void** ppData) override;

void PostCallRecordMapMemory(VkDevice device, VkDeviceMemory memory, VkDeviceSize offset, VkDeviceSize size, VkMemoryMapFlags flags,
                             void** ppData, const RecordObject& record_obj) override;

void PreCallRecordUnmapMemory(VkDevice device, VkDeviceMemory memory) override;

void PostCallRecordUnmapMemory(VkDevice device, VkDeviceMemory memory, const RecordObject& record_obj) override;

void PreCallRecordFlushMappedMemoryRanges(VkDevice device, uint32_t memoryRangeCount,
                                          const VkMappedMemoryRange* pMemoryRanges) override;

void PostCallRecordFlushMappedMemoryRanges(VkDevice device, uint32_t memoryRangeCount, const VkMappedMemoryRange* pMemoryRanges,
                                           const RecordObject& record_obj) override;

void PreCallRecordInvalidateMappedMemoryRanges(VkDevice device, uint32_t memoryRangeCount,
                                               const VkMappedMemoryRange* pMemoryRanges) override;

void PostCallRecordInvalidateMappedMemoryRanges(VkDevice device, uint32_t memoryRangeCount,
                                                const VkMappedMemoryRange* pMemoryRanges, const RecordObject& record_obj) override;

void PreCallRecordGetDeviceMemoryCommitment(VkDevice device, VkDeviceMemory memory, VkDeviceSize* pCommittedMemoryInBytes) override;

void PostCallRecordGetDeviceMemoryCommitment(VkDevice device, VkDeviceMemory memory, VkDeviceSize* pCommittedMemoryInBytes,
                                             const RecordObject& record_obj) override;

void PreCallRecordBindBufferMemory(VkDevice device, VkBuffer buffer, VkDeviceMemory memory, VkDeviceSize memoryOffset) override;

void PostCallRecordBindBufferMemory(VkDevice device, VkBuffer buffer, VkDeviceMemory memory, VkDeviceSize memoryOffset,
                                    const RecordObject& record_obj) override;

void PreCallRecordBindImageMemory(VkDevice device, VkImage image, VkDeviceMemory memory, VkDeviceSize memoryOffset) override;

void PostCallRecordBindImageMemory(VkDevice device, VkImage image, VkDeviceMemory memory, VkDeviceSize memoryOffset,
                                   const RecordObject& record_obj) override;

void PreCallRecordGetBufferMemoryRequirements(VkDevice device, VkBuffer buffer, VkMemoryRequirements* pMemoryRequirements) override;

void PostCallRecordGetBufferMemoryRequirements(VkDevice device, VkBuffer buffer, VkMemoryRequirements* pMemoryRequirements,
                                               const RecordObject& record_obj) override;

void PreCallRecordGetImageMemoryRequirements(VkDevice device, VkImage image, VkMemoryRequirements* pMemoryRequirements) override;

void PostCallRecordGetImageMemoryRequirements(VkDevice device, VkImage image, VkMemoryRequirements* pMemoryRequirements,
                                              const RecordObject& record_obj) override;

void PreCallRecordGetImageSparseMemoryRequirements(VkDevice device, VkImage image, uint32_t* pSparseMemoryRequirementCount,
                                                   VkSparseImageMemoryRequirements* pSparseMemoryRequirements) override;

void PostCallRecordGetImageSparseMemoryRequirements(VkDevice device, VkImage image, uint32_t* pSparseMemoryRequirementCount,
                                                    VkSparseImageMemoryRequirements* pSparseMemoryRequirements,
                                                    const RecordObject& record_obj) override;

void PreCallRecordQueueBindSparse(VkQueue queue, uint32_t bindInfoCount, const VkBindSparseInfo* pBindInfo, VkFence fence) override;

void PostCallRecordQueueBindSparse(VkQueue queue, uint32_t bindInfoCount, const VkBindSparseInfo* pBindInfo, VkFence fence,
                                   const RecordObject& record_obj) override;

void PreCallRecordCreateFence(VkDevice device, const VkFenceCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator,
                              VkFence* pFence) override;

void PostCallRecordCreateFence(VkDevice device, const VkFenceCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator,
                               VkFence* pFence, const RecordObject& record_obj) override;

void PreCallRecordDestroyFence(VkDevice device, VkFence fence, const VkAllocationCallbacks* pAllocator) override;

void PostCallRecordDestroyFence(VkDevice device, VkFence fence, const VkAllocationCallbacks* pAllocator,
                                const RecordObject& record_obj) override;

void PreCallRecordResetFences(VkDevice device, uint32_t fenceCount, const VkFence* pFences) override;

void PostCallRecordResetFences(VkDevice device, uint32_t fenceCount, const VkFence* pFences,
                               const RecordObject& record_obj) override;

void PreCallRecordGetFenceStatus(VkDevice device, VkFence fence) override;

void PostCallRecordGetFenceStatus(VkDevice device, VkFence fence, const RecordObject& record_obj) override;

void PreCallRecordWaitForFences(VkDevice device, uint32_t fenceCount, const VkFence* pFences, VkBool32 waitAll,
                                uint64_t timeout) override;

void PostCallRecordWaitForFences(VkDevice device, uint32_t fenceCount, const VkFence* pFences, VkBool32 waitAll, uint64_t timeout,
                                 const RecordObject& record_obj) override;

void PreCallRecordCreateSemaphore(VkDevice device, const VkSemaphoreCreateInfo* pCreateInfo,
                                  const VkAllocationCallbacks* pAllocator, VkSemaphore* pSemaphore) override;

void PostCallRecordCreateSemaphore(VkDevice device, const VkSemaphoreCreateInfo* pCreateInfo,
                                   const VkAllocationCallbacks* pAllocator, VkSemaphore* pSemaphore,
                                   const RecordObject& record_obj) override;

void PreCallRecordDestroySemaphore(VkDevice device, VkSemaphore semaphore, const VkAllocationCallbacks* pAllocator) override;

void PostCallRecordDestroySemaphore(VkDevice device, VkSemaphore semaphore, const VkAllocationCallbacks* pAllocator,
                                    const RecordObject& record_obj) override;

void PreCallRecordCreateEvent(VkDevice device, const VkEventCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator,
                              VkEvent* pEvent) override;

void PostCallRecordCreateEvent(VkDevice device, const VkEventCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator,
                               VkEvent* pEvent, const RecordObject& record_obj) override;

void PreCallRecordDestroyEvent(VkDevice device, VkEvent event, const VkAllocationCallbacks* pAllocator) override;

void PostCallRecordDestroyEvent(VkDevice device, VkEvent event, const VkAllocationCallbacks* pAllocator,
                                const RecordObject& record_obj) override;

void PreCallRecordGetEventStatus(VkDevice device, VkEvent event) override;

void PostCallRecordGetEventStatus(VkDevice device, VkEvent event, const RecordObject& record_obj) override;

void PreCallRecordSetEvent(VkDevice device, VkEvent event) override;

void PostCallRecordSetEvent(VkDevice device, VkEvent event, const RecordObject& record_obj) override;

void PreCallRecordResetEvent(VkDevice device, VkEvent event) override;

void PostCallRecordResetEvent(VkDevice device, VkEvent event, const RecordObject& record_obj) override;

void PreCallRecordCreateQueryPool(VkDevice device, const VkQueryPoolCreateInfo* pCreateInfo,
                                  const VkAllocationCallbacks* pAllocator, VkQueryPool* pQueryPool) override;

void PostCallRecordCreateQueryPool(VkDevice device, const VkQueryPoolCreateInfo* pCreateInfo,
                                   const VkAllocationCallbacks* pAllocator, VkQueryPool* pQueryPool,
                                   const RecordObject& record_obj) override;

void PreCallRecordDestroyQueryPool(VkDevice device, VkQueryPool queryPool, const VkAllocationCallbacks* pAllocator) override;

void PostCallRecordDestroyQueryPool(VkDevice device, VkQueryPool queryPool, const VkAllocationCallbacks* pAllocator,
                                    const RecordObject& record_obj) override;

void PreCallRecordGetQueryPoolResults(VkDevice device, VkQueryPool queryPool, uint32_t firstQuery, uint32_t queryCount,
                                      size_t dataSize, void* pData, VkDeviceSize stride, VkQueryResultFlags flags) override;

void PostCallRecordGetQueryPoolResults(VkDevice device, VkQueryPool queryPool, uint32_t firstQuery, uint32_t queryCount,
                                       size_t dataSize, void* pData, VkDeviceSize stride, VkQueryResultFlags flags,
                                       const RecordObject& record_obj) override;

void PreCallRecordCreateBuffer(VkDevice device, const VkBufferCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator,
                               VkBuffer* pBuffer) override;

void PostCallRecordCreateBuffer(VkDevice device, const VkBufferCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator,
                                VkBuffer* pBuffer, const RecordObject& record_obj) override;

void PreCallRecordDestroyBuffer(VkDevice device, VkBuffer buffer, const VkAllocationCallbacks* pAllocator) override;

void PostCallRecordDestroyBuffer(VkDevice device, VkBuffer buffer, const VkAllocationCallbacks* pAllocator,
                                 const RecordObject& record_obj) override;

void PreCallRecordCreateBufferView(VkDevice device, const VkBufferViewCreateInfo* pCreateInfo,
                                   const VkAllocationCallbacks* pAllocator, VkBufferView* pView) override;

void PostCallRecordCreateBufferView(VkDevice device, const VkBufferViewCreateInfo* pCreateInfo,
                                    const VkAllocationCallbacks* pAllocator, VkBufferView* pView,
                                    const RecordObject& record_obj) override;

void PreCallRecordDestroyBufferView(VkDevice device, VkBufferView bufferView, const VkAllocationCallbacks* pAllocator) override;

void PostCallRecordDestroyBufferView(VkDevice device, VkBufferView bufferView, const VkAllocationCallbacks* pAllocator,
                                     const RecordObject& record_obj) override;

void PreCallRecordCreateImage(VkDevice device, const VkImageCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator,
                              VkImage* pImage) override;

void PostCallRecordCreateImage(VkDevice device, const VkImageCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator,
                               VkImage* pImage, const RecordObject& record_obj) override;

void PreCallRecordDestroyImage(VkDevice device, VkImage image, const VkAllocationCallbacks* pAllocator) override;

void PostCallRecordDestroyImage(VkDevice device, VkImage image, const VkAllocationCallbacks* pAllocator,
                                const RecordObject& record_obj) override;

void PreCallRecordGetImageSubresourceLayout(VkDevice device, VkImage image, const VkImageSubresource* pSubresource,
                                            VkSubresourceLayout* pLayout) override;

void PostCallRecordGetImageSubresourceLayout(VkDevice device, VkImage image, const VkImageSubresource* pSubresource,
                                             VkSubresourceLayout* pLayout, const RecordObject& record_obj) override;

void PreCallRecordCreateImageView(VkDevice device, const VkImageViewCreateInfo* pCreateInfo,
                                  const VkAllocationCallbacks* pAllocator, VkImageView* pView) override;

void PostCallRecordCreateImageView(VkDevice device, const VkImageViewCreateInfo* pCreateInfo,
                                   const VkAllocationCallbacks* pAllocator, VkImageView* pView,
                                   const RecordObject& record_obj) override;

void PreCallRecordDestroyImageView(VkDevice device, VkImageView imageView, const VkAllocationCallbacks* pAllocator) override;

void PostCallRecordDestroyImageView(VkDevice device, VkImageView imageView, const VkAllocationCallbacks* pAllocator,
                                    const RecordObject& record_obj) override;

void PreCallRecordCreateShaderModule(VkDevice device, const VkShaderModuleCreateInfo* pCreateInfo,
                                     const VkAllocationCallbacks* pAllocator, VkShaderModule* pShaderModule) override;

void PostCallRecordCreateShaderModule(VkDevice device, const VkShaderModuleCreateInfo* pCreateInfo,
                                      const VkAllocationCallbacks* pAllocator, VkShaderModule* pShaderModule,
                                      const RecordObject& record_obj) override;

void PreCallRecordDestroyShaderModule(VkDevice device, VkShaderModule shaderModule,
                                      const VkAllocationCallbacks* pAllocator) override;

void PostCallRecordDestroyShaderModule(VkDevice device, VkShaderModule shaderModule, const VkAllocationCallbacks* pAllocator,
                                       const RecordObject& record_obj) override;

void PreCallRecordCreatePipelineCache(VkDevice device, const VkPipelineCacheCreateInfo* pCreateInfo,
                                      const VkAllocationCallbacks* pAllocator, VkPipelineCache* pPipelineCache) override;

void PostCallRecordCreatePipelineCache(VkDevice device, const VkPipelineCacheCreateInfo* pCreateInfo,
                                       const VkAllocationCallbacks* pAllocator, VkPipelineCache* pPipelineCache,
                                       const RecordObject& record_obj) override;

void PreCallRecordDestroyPipelineCache(VkDevice device, VkPipelineCache pipelineCache,
                                       const VkAllocationCallbacks* pAllocator) override;

void PostCallRecordDestroyPipelineCache(VkDevice device, VkPipelineCache pipelineCache, const VkAllocationCallbacks* pAllocator,
                                        const RecordObject& record_obj) override;

void PreCallRecordGetPipelineCacheData(VkDevice device, VkPipelineCache pipelineCache, size_t* pDataSize, void* pData) override;

void PostCallRecordGetPipelineCacheData(VkDevice device, VkPipelineCache pipelineCache, size_t* pDataSize, void* pData,
                                        const RecordObject& record_obj) override;

void PreCallRecordMergePipelineCaches(VkDevice device, VkPipelineCache dstCache, uint32_t srcCacheCount,
                                      const VkPipelineCache* pSrcCaches) override;

void PostCallRecordMergePipelineCaches(VkDevice device, VkPipelineCache dstCache, uint32_t srcCacheCount,
                                       const VkPipelineCache* pSrcCaches, const RecordObject& record_obj) override;

void PreCallRecordCreateGraphicsPipelines(VkDevice device, VkPipelineCache pipelineCache, uint32_t createInfoCount,
                                          const VkGraphicsPipelineCreateInfo* pCreateInfos, const VkAllocationCallbacks* pAllocator,
                                          VkPipeline* pPipelines) override;

void PostCallRecordCreateGraphicsPipelines(VkDevice device, VkPipelineCache pipelineCache, uint32_t createInfoCount,
                                           const VkGraphicsPipelineCreateInfo* pCreateInfos,
                                           const VkAllocationCallbacks* pAllocator, VkPipeline* pPipelines,
                                           const RecordObject& record_obj) override;

void PreCallRecordCreateComputePipelines(VkDevice device, VkPipelineCache pipelineCache, uint32_t createInfoCount,
                                         const VkComputePipelineCreateInfo* pCreateInfos, const VkAllocationCallbacks* pAllocator,
                                         VkPipeline* pPipelines) override;

void PostCallRecordCreateComputePipelines(VkDevice device, VkPipelineCache pipelineCache, uint32_t createInfoCount,
                                          const VkComputePipelineCreateInfo* pCreateInfos, const VkAllocationCallbacks* pAllocator,
                                          VkPipeline* pPipelines, const RecordObject& record_obj) override;

void PreCallRecordDestroyPipeline(VkDevice device, VkPipeline pipeline, const VkAllocationCallbacks* pAllocator) override;

void PostCallRecordDestroyPipeline(VkDevice device, VkPipeline pipeline, const VkAllocationCallbacks* pAllocator,
                                   const RecordObject& record_obj) override;

void PreCallRecordCreatePipelineLayout(VkDevice device, const VkPipelineLayoutCreateInfo* pCreateInfo,
                                       const VkAllocationCallbacks* pAllocator, VkPipelineLayout* pPipelineLayout) override;

void PostCallRecordCreatePipelineLayout(VkDevice device, const VkPipelineLayoutCreateInfo* pCreateInfo,
                                        const VkAllocationCallbacks* pAllocator, VkPipelineLayout* pPipelineLayout,
                                        const RecordObject& record_obj) override;

void PreCallRecordDestroyPipelineLayout(VkDevice device, VkPipelineLayout pipelineLayout,
                                        const VkAllocationCallbacks* pAllocator) override;

void PostCallRecordDestroyPipelineLayout(VkDevice device, VkPipelineLayout pipelineLayout, const VkAllocationCallbacks* pAllocator,
                                         const RecordObject& record_obj) override;

void PreCallRecordCreateSampler(VkDevice device, const VkSamplerCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator,
                                VkSampler* pSampler) override;

void PostCallRecordCreateSampler(VkDevice device, const VkSamplerCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator,
                                 VkSampler* pSampler, const RecordObject& record_obj) override;

void PreCallRecordDestroySampler(VkDevice device, VkSampler sampler, const VkAllocationCallbacks* pAllocator) override;

void PostCallRecordDestroySampler(VkDevice device, VkSampler sampler, const VkAllocationCallbacks* pAllocator,
                                  const RecordObject& record_obj) override;

void PreCallRecordCreateDescriptorSetLayout(VkDevice device, const VkDescriptorSetLayoutCreateInfo* pCreateInfo,
                                            const VkAllocationCallbacks* pAllocator, VkDescriptorSetLayout* pSetLayout) override;

void PostCallRecordCreateDescriptorSetLayout(VkDevice device, const VkDescriptorSetLayoutCreateInfo* pCreateInfo,
                                             const VkAllocationCallbacks* pAllocator, VkDescriptorSetLayout* pSetLayout,
                                             const RecordObject& record_obj) override;

void PreCallRecordDestroyDescriptorSetLayout(VkDevice device, VkDescriptorSetLayout descriptorSetLayout,
                                             const VkAllocationCallbacks* pAllocator) override;

void PostCallRecordDestroyDescriptorSetLayout(VkDevice device, VkDescriptorSetLayout descriptorSetLayout,
                                              const VkAllocationCallbacks* pAllocator, const RecordObject& record_obj) override;

void PreCallRecordCreateDescriptorPool(VkDevice device, const VkDescriptorPoolCreateInfo* pCreateInfo,
                                       const VkAllocationCallbacks* pAllocator, VkDescriptorPool* pDescriptorPool) override;

void PostCallRecordCreateDescriptorPool(VkDevice device, const VkDescriptorPoolCreateInfo* pCreateInfo,
                                        const VkAllocationCallbacks* pAllocator, VkDescriptorPool* pDescriptorPool,
                                        const RecordObject& record_obj) override;

void PreCallRecordDestroyDescriptorPool(VkDevice device, VkDescriptorPool descriptorPool,
                                        const VkAllocationCallbacks* pAllocator) override;

void PostCallRecordDestroyDescriptorPool(VkDevice device, VkDescriptorPool descriptorPool, const VkAllocationCallbacks* pAllocator,
                                         const RecordObject& record_obj) override;

void PreCallRecordResetDescriptorPool(VkDevice device, VkDescriptorPool descriptorPool, VkDescriptorPoolResetFlags flags) override;

void PostCallRecordResetDescriptorPool(VkDevice device, VkDescriptorPool descriptorPool, VkDescriptorPoolResetFlags flags,
                                       const RecordObject& record_obj) override;

void PreCallRecordAllocateDescriptorSets(VkDevice device, const VkDescriptorSetAllocateInfo* pAllocateInfo,
                                         VkDescriptorSet* pDescriptorSets) override;

void PostCallRecordAllocateDescriptorSets(VkDevice device, const VkDescriptorSetAllocateInfo* pAllocateInfo,
                                          VkDescriptorSet* pDescriptorSets, const RecordObject& record_obj) override;

void PreCallRecordFreeDescriptorSets(VkDevice device, VkDescriptorPool descriptorPool, uint32_t descriptorSetCount,
                                     const VkDescriptorSet* pDescriptorSets) override;

void PostCallRecordFreeDescriptorSets(VkDevice device, VkDescriptorPool descriptorPool, uint32_t descriptorSetCount,
                                      const VkDescriptorSet* pDescriptorSets, const RecordObject& record_obj) override;

void PreCallRecordUpdateDescriptorSets(VkDevice device, uint32_t descriptorWriteCount,
                                       const VkWriteDescriptorSet* pDescriptorWrites, uint32_t descriptorCopyCount,
                                       const VkCopyDescriptorSet* pDescriptorCopies) override;

void PostCallRecordUpdateDescriptorSets(VkDevice device, uint32_t descriptorWriteCount,
                                        const VkWriteDescriptorSet* pDescriptorWrites, uint32_t descriptorCopyCount,
                                        const VkCopyDescriptorSet* pDescriptorCopies, const RecordObject& record_obj) override;

void PreCallRecordCreateFramebuffer(VkDevice device, const VkFramebufferCreateInfo* pCreateInfo,
                                    const VkAllocationCallbacks* pAllocator, VkFramebuffer* pFramebuffer) override;

void PostCallRecordCreateFramebuffer(VkDevice device, const VkFramebufferCreateInfo* pCreateInfo,
                                     const VkAllocationCallbacks* pAllocator, VkFramebuffer* pFramebuffer,
                                     const RecordObject& record_obj) override;

void PreCallRecordDestroyFramebuffer(VkDevice device, VkFramebuffer framebuffer, const VkAllocationCallbacks* pAllocator) override;

void PostCallRecordDestroyFramebuffer(VkDevice device, VkFramebuffer framebuffer, const VkAllocationCallbacks* pAllocator,
                                      const RecordObject& record_obj) override;

void PreCallRecordCreateRenderPass(VkDevice device, const VkRenderPassCreateInfo* pCreateInfo,
                                   const VkAllocationCallbacks* pAllocator, VkRenderPass* pRenderPass) override;

void PostCallRecordCreateRenderPass(VkDevice device, const VkRenderPassCreateInfo* pCreateInfo,
                                    const VkAllocationCallbacks* pAllocator, VkRenderPass* pRenderPass,
                                    const RecordObject& record_obj) override;

void PreCallRecordDestroyRenderPass(VkDevice device, VkRenderPass renderPass, const VkAllocationCallbacks* pAllocator) override;

void PostCallRecordDestroyRenderPass(VkDevice device, VkRenderPass renderPass, const VkAllocationCallbacks* pAllocator,
                                     const RecordObject& record_obj) override;

void PreCallRecordGetRenderAreaGranularity(VkDevice device, VkRenderPass renderPass, VkExtent2D* pGranularity) override;

void PostCallRecordGetRenderAreaGranularity(VkDevice device, VkRenderPass renderPass, VkExtent2D* pGranularity,
                                            const RecordObject& record_obj) override;

void PreCallRecordCreateCommandPool(VkDevice device, const VkCommandPoolCreateInfo* pCreateInfo,
                                    const VkAllocationCallbacks* pAllocator, VkCommandPool* pCommandPool) override;

void PostCallRecordCreateCommandPool(VkDevice device, const VkCommandPoolCreateInfo* pCreateInfo,
                                     const VkAllocationCallbacks* pAllocator, VkCommandPool* pCommandPool,
                                     const RecordObject& record_obj) override;

void PreCallRecordDestroyCommandPool(VkDevice device, VkCommandPool commandPool, const VkAllocationCallbacks* pAllocator) override;

void PostCallRecordDestroyCommandPool(VkDevice device, VkCommandPool commandPool, const VkAllocationCallbacks* pAllocator,
                                      const RecordObject& record_obj) override;

void PreCallRecordResetCommandPool(VkDevice device, VkCommandPool commandPool, VkCommandPoolResetFlags flags) override;

void PostCallRecordResetCommandPool(VkDevice device, VkCommandPool commandPool, VkCommandPoolResetFlags flags,
                                    const RecordObject& record_obj) override;

void PreCallRecordAllocateCommandBuffers(VkDevice device, const VkCommandBufferAllocateInfo* pAllocateInfo,
                                         VkCommandBuffer* pCommandBuffers) override;

void PostCallRecordAllocateCommandBuffers(VkDevice device, const VkCommandBufferAllocateInfo* pAllocateInfo,
                                          VkCommandBuffer* pCommandBuffers, const RecordObject& record_obj) override;

void PreCallRecordFreeCommandBuffers(VkDevice device, VkCommandPool commandPool, uint32_t commandBufferCount,
                                     const VkCommandBuffer* pCommandBuffers) override;

void PostCallRecordFreeCommandBuffers(VkDevice device, VkCommandPool commandPool, uint32_t commandBufferCount,
                                      const VkCommandBuffer* pCommandBuffers, const RecordObject& record_obj) override;

void PreCallRecordBeginCommandBuffer(VkCommandBuffer commandBuffer, const VkCommandBufferBeginInfo* pBeginInfo) override;

void PostCallRecordBeginCommandBuffer(VkCommandBuffer commandBuffer, const VkCommandBufferBeginInfo* pBeginInfo,
                                      const RecordObject& record_obj) override;

void PreCallRecordEndCommandBuffer(VkCommandBuffer commandBuffer) override;

void PostCallRecordEndCommandBuffer(VkCommandBuffer commandBuffer, const RecordObject& record_obj) override;

void PreCallRecordResetCommandBuffer(VkCommandBuffer commandBuffer, VkCommandBufferResetFlags flags) override;

void PostCallRecordResetCommandBuffer(VkCommandBuffer commandBuffer, VkCommandBufferResetFlags flags,
                                      const RecordObject& record_obj) override;

void PreCallRecordCmdBindPipeline(VkCommandBuffer commandBuffer, VkPipelineBindPoint pipelineBindPoint,
                                  VkPipeline pipeline) override;

void PostCallRecordCmdBindPipeline(VkCommandBuffer commandBuffer, VkPipelineBindPoint pipelineBindPoint, VkPipeline pipeline,
                                   const RecordObject& record_obj) override;

void PreCallRecordCmdSetViewport(VkCommandBuffer commandBuffer, uint32_t firstViewport, uint32_t viewportCount,
                                 const VkViewport* pViewports) override;

void PostCallRecordCmdSetViewport(VkCommandBuffer commandBuffer, uint32_t firstViewport, uint32_t viewportCount,
                                  const VkViewport* pViewports, const RecordObject& record_obj) override;

void PreCallRecordCmdSetScissor(VkCommandBuffer commandBuffer, uint32_t firstScissor, uint32_t scissorCount,
                                const VkRect2D* pScissors) override;

void PostCallRecordCmdSetScissor(VkCommandBuffer commandBuffer, uint32_t firstScissor, uint32_t scissorCount,
                                 const VkRect2D* pScissors, const RecordObject& record_obj) override;

void PreCallRecordCmdSetLineWidth(VkCommandBuffer commandBuffer, float lineWidth) override;

void PostCallRecordCmdSetLineWidth(VkCommandBuffer commandBuffer, float lineWidth, const RecordObject& record_obj) override;

void PreCallRecordCmdSetDepthBias(VkCommandBuffer commandBuffer, float depthBiasConstantFactor, float depthBiasClamp,
                                  float depthBiasSlopeFactor) override;

void PostCallRecordCmdSetDepthBias(VkCommandBuffer commandBuffer, float depthBiasConstantFactor, float depthBiasClamp,
                                   float depthBiasSlopeFactor, const RecordObject& record_obj) override;

void PreCallRecordCmdSetBlendConstants(VkCommandBuffer commandBuffer, const float blendConstants[4]) override;

void PostCallRecordCmdSetBlendConstants(VkCommandBuffer commandBuffer, const float blendConstants[4],
                                        const RecordObject& record_obj) override;

void PreCallRecordCmdSetDepthBounds(VkCommandBuffer commandBuffer, float minDepthBounds, float maxDepthBounds) override;

void PostCallRecordCmdSetDepthBounds(VkCommandBuffer commandBuffer, float minDepthBounds, float maxDepthBounds,
                                     const RecordObject& record_obj) override;

void PreCallRecordCmdSetStencilCompareMask(VkCommandBuffer commandBuffer, VkStencilFaceFlags faceMask,
                                           uint32_t compareMask) override;

void PostCallRecordCmdSetStencilCompareMask(VkCommandBuffer commandBuffer, VkStencilFaceFlags faceMask, uint32_t compareMask,
                                            const RecordObject& record_obj) override;

void PreCallRecordCmdSetStencilWriteMask(VkCommandBuffer commandBuffer, VkStencilFaceFlags faceMask, uint32_t writeMask) override;

void PostCallRecordCmdSetStencilWriteMask(VkCommandBuffer commandBuffer, VkStencilFaceFlags faceMask, uint32_t writeMask,
                                          const RecordObject& record_obj) override;

void PreCallRecordCmdSetStencilReference(VkCommandBuffer commandBuffer, VkStencilFaceFlags faceMask, uint32_t reference) override;

void PostCallRecordCmdSetStencilReference(VkCommandBuffer commandBuffer, VkStencilFaceFlags faceMask, uint32_t reference,
                                          const RecordObject& record_obj) override;

void PreCallRecordCmdBindDescriptorSets(VkCommandBuffer commandBuffer, VkPipelineBindPoint pipelineBindPoint,
                                        VkPipelineLayout layout, uint32_t firstSet, uint32_t descriptorSetCount,
                                        const VkDescriptorSet* pDescriptorSets, uint32_t dynamicOffsetCount,
                                        const uint32_t* pDynamicOffsets) override;

void PostCallRecordCmdBindDescriptorSets(VkCommandBuffer commandBuffer, VkPipelineBindPoint pipelineBindPoint,
                                         VkPipelineLayout layout, uint32_t firstSet, uint32_t descriptorSetCount,
                                         const VkDescriptorSet* pDescriptorSets, uint32_t dynamicOffsetCount,
                                         const uint32_t* pDynamicOffsets, const RecordObject& record_obj) override;

void PreCallRecordCmdBindIndexBuffer(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                     VkIndexType indexType) override;

void PostCallRecordCmdBindIndexBuffer(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset, VkIndexType indexType,
                                      const RecordObject& record_obj) override;

void PreCallRecordCmdBindVertexBuffers(VkCommandBuffer commandBuffer, uint32_t firstBinding, uint32_t bindingCount,
                                       const VkBuffer* pBuffers, const VkDeviceSize* pOffsets) override;

void PostCallRecordCmdBindVertexBuffers(VkCommandBuffer commandBuffer, uint32_t firstBinding, uint32_t bindingCount,
                                        const VkBuffer* pBuffers, const VkDeviceSize* pOffsets,
                                        const RecordObject& record_obj) override;

void PreCallRecordCmdDraw(VkCommandBuffer commandBuffer, uint32_t vertexCount, uint32_t instanceCount, uint32_t firstVertex,
                          uint32_t firstInstance) override;

void PostCallRecordCmdDraw(VkCommandBuffer commandBuffer, uint32_t vertexCount, uint32_t instanceCount, uint32_t firstVertex,
                           uint32_t firstInstance, const RecordObject& record_obj) override;

void PreCallRecordCmdDrawIndexed(VkCommandBuffer commandBuffer, uint32_t indexCount, uint32_t instanceCount, uint32_t firstIndex,
                                 int32_t vertexOffset, uint32_t firstInstance) override;

void PostCallRecordCmdDrawIndexed(VkCommandBuffer commandBuffer, uint32_t indexCount, uint32_t instanceCount, uint32_t firstIndex,
                                  int32_t vertexOffset, uint32_t firstInstance, const RecordObject& record_obj) override;

void PreCallRecordCmdDrawIndirect(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset, uint32_t drawCount,
                                  uint32_t stride) override;

void PostCallRecordCmdDrawIndirect(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset, uint32_t drawCount,
                                   uint32_t stride, const RecordObject& record_obj) override;

void PreCallRecordCmdDrawIndexedIndirect(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset, uint32_t drawCount,
                                         uint32_t stride) override;

void PostCallRecordCmdDrawIndexedIndirect(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset, uint32_t drawCount,
                                          uint32_t stride, const RecordObject& record_obj) override;

void PreCallRecordCmdDispatch(VkCommandBuffer commandBuffer, uint32_t groupCountX, uint32_t groupCountY,
                              uint32_t groupCountZ) override;

void PostCallRecordCmdDispatch(VkCommandBuffer commandBuffer, uint32_t groupCountX, uint32_t groupCountY, uint32_t groupCountZ,
                               const RecordObject& record_obj) override;

void PreCallRecordCmdDispatchIndirect(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset) override;

void PostCallRecordCmdDispatchIndirect(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                       const RecordObject& record_obj) override;

void PreCallRecordCmdCopyBuffer(VkCommandBuffer commandBuffer, VkBuffer srcBuffer, VkBuffer dstBuffer, uint32_t regionCount,
                                const VkBufferCopy* pRegions) override;

void PostCallRecordCmdCopyBuffer(VkCommandBuffer commandBuffer, VkBuffer srcBuffer, VkBuffer dstBuffer, uint32_t regionCount,
                                 const VkBufferCopy* pRegions, const RecordObject& record_obj) override;

void PreCallRecordCmdCopyImage(VkCommandBuffer commandBuffer, VkImage srcImage, VkImageLayout srcImageLayout, VkImage dstImage,
                               VkImageLayout dstImageLayout, uint32_t regionCount, const VkImageCopy* pRegions) override;

void PostCallRecordCmdCopyImage(VkCommandBuffer commandBuffer, VkImage srcImage, VkImageLayout srcImageLayout, VkImage dstImage,
                                VkImageLayout dstImageLayout, uint32_t regionCount, const VkImageCopy* pRegions,
                                const RecordObject& record_obj) override;

void PreCallRecordCmdBlitImage(VkCommandBuffer commandBuffer, VkImage srcImage, VkImageLayout srcImageLayout, VkImage dstImage,
                               VkImageLayout dstImageLayout, uint32_t regionCount, const VkImageBlit* pRegions,
                               VkFilter filter) override;

void PostCallRecordCmdBlitImage(VkCommandBuffer commandBuffer, VkImage srcImage, VkImageLayout srcImageLayout, VkImage dstImage,
                                VkImageLayout dstImageLayout, uint32_t regionCount, const VkImageBlit* pRegions, VkFilter filter,
                                const RecordObject& record_obj) override;

void PreCallRecordCmdCopyBufferToImage(VkCommandBuffer commandBuffer, VkBuffer srcBuffer, VkImage dstImage,
                                       VkImageLayout dstImageLayout, uint32_t regionCount,
                                       const VkBufferImageCopy* pRegions) override;

void PostCallRecordCmdCopyBufferToImage(VkCommandBuffer commandBuffer, VkBuffer srcBuffer, VkImage dstImage,
                                        VkImageLayout dstImageLayout, uint32_t regionCount, const VkBufferImageCopy* pRegions,
                                        const RecordObject& record_obj) override;

void PreCallRecordCmdCopyImageToBuffer(VkCommandBuffer commandBuffer, VkImage srcImage, VkImageLayout srcImageLayout,
                                       VkBuffer dstBuffer, uint32_t regionCount, const VkBufferImageCopy* pRegions) override;

void PostCallRecordCmdCopyImageToBuffer(VkCommandBuffer commandBuffer, VkImage srcImage, VkImageLayout srcImageLayout,
                                        VkBuffer dstBuffer, uint32_t regionCount, const VkBufferImageCopy* pRegions,
                                        const RecordObject& record_obj) override;

void PreCallRecordCmdUpdateBuffer(VkCommandBuffer commandBuffer, VkBuffer dstBuffer, VkDeviceSize dstOffset, VkDeviceSize dataSize,
                                  const void* pData) override;

void PostCallRecordCmdUpdateBuffer(VkCommandBuffer commandBuffer, VkBuffer dstBuffer, VkDeviceSize dstOffset, VkDeviceSize dataSize,
                                   const void* pData, const RecordObject& record_obj) override;

void PreCallRecordCmdFillBuffer(VkCommandBuffer commandBuffer, VkBuffer dstBuffer, VkDeviceSize dstOffset, VkDeviceSize size,
                                uint32_t data) override;

void PostCallRecordCmdFillBuffer(VkCommandBuffer commandBuffer, VkBuffer dstBuffer, VkDeviceSize dstOffset, VkDeviceSize size,
                                 uint32_t data, const RecordObject& record_obj) override;

void PreCallRecordCmdClearColorImage(VkCommandBuffer commandBuffer, VkImage image, VkImageLayout imageLayout,
                                     const VkClearColorValue* pColor, uint32_t rangeCount,
                                     const VkImageSubresourceRange* pRanges) override;

void PostCallRecordCmdClearColorImage(VkCommandBuffer commandBuffer, VkImage image, VkImageLayout imageLayout,
                                      const VkClearColorValue* pColor, uint32_t rangeCount, const VkImageSubresourceRange* pRanges,
                                      const RecordObject& record_obj) override;

void PreCallRecordCmdClearDepthStencilImage(VkCommandBuffer commandBuffer, VkImage image, VkImageLayout imageLayout,
                                            const VkClearDepthStencilValue* pDepthStencil, uint32_t rangeCount,
                                            const VkImageSubresourceRange* pRanges) override;

void PostCallRecordCmdClearDepthStencilImage(VkCommandBuffer commandBuffer, VkImage image, VkImageLayout imageLayout,
                                             const VkClearDepthStencilValue* pDepthStencil, uint32_t rangeCount,
                                             const VkImageSubresourceRange* pRanges, const RecordObject& record_obj) override;

void PreCallRecordCmdClearAttachments(VkCommandBuffer commandBuffer, uint32_t attachmentCount,
                                      const VkClearAttachment* pAttachments, uint32_t rectCount,
                                      const VkClearRect* pRects) override;

void PostCallRecordCmdClearAttachments(VkCommandBuffer commandBuffer, uint32_t attachmentCount,
                                       const VkClearAttachment* pAttachments, uint32_t rectCount, const VkClearRect* pRects,
                                       const RecordObject& record_obj) override;

void PreCallRecordCmdResolveImage(VkCommandBuffer commandBuffer, VkImage srcImage, VkImageLayout srcImageLayout, VkImage dstImage,
                                  VkImageLayout dstImageLayout, uint32_t regionCount, const VkImageResolve* pRegions) override;

void PostCallRecordCmdResolveImage(VkCommandBuffer commandBuffer, VkImage srcImage, VkImageLayout srcImageLayout, VkImage dstImage,
                                   VkImageLayout dstImageLayout, uint32_t regionCount, const VkImageResolve* pRegions,
                                   const RecordObject& record_obj) override;

void PreCallRecordCmdSetEvent(VkCommandBuffer commandBuffer, VkEvent event, VkPipelineStageFlags stageMask) override;

void PostCallRecordCmdSetEvent(VkCommandBuffer commandBuffer, VkEvent event, VkPipelineStageFlags stageMask,
                               const RecordObject& record_obj) override;

void PreCallRecordCmdResetEvent(VkCommandBuffer commandBuffer, VkEvent event, VkPipelineStageFlags stageMask) override;

void PostCallRecordCmdResetEvent(VkCommandBuffer commandBuffer, VkEvent event, VkPipelineStageFlags stageMask,
                                 const RecordObject& record_obj) override;

void PreCallRecordCmdWaitEvents(VkCommandBuffer commandBuffer, uint32_t eventCount, const VkEvent* pEvents,
                                VkPipelineStageFlags srcStageMask, VkPipelineStageFlags dstStageMask, uint32_t memoryBarrierCount,
                                const VkMemoryBarrier* pMemoryBarriers, uint32_t bufferMemoryBarrierCount,
                                const VkBufferMemoryBarrier* pBufferMemoryBarriers, uint32_t imageMemoryBarrierCount,
                                const VkImageMemoryBarrier* pImageMemoryBarriers) override;

void PostCallRecordCmdWaitEvents(VkCommandBuffer commandBuffer, uint32_t eventCount, const VkEvent* pEvents,
                                 VkPipelineStageFlags srcStageMask, VkPipelineStageFlags dstStageMask, uint32_t memoryBarrierCount,
                                 const VkMemoryBarrier* pMemoryBarriers, uint32_t bufferMemoryBarrierCount,
                                 const VkBufferMemoryBarrier* pBufferMemoryBarriers, uint32_t imageMemoryBarrierCount,
                                 const VkImageMemoryBarrier* pImageMemoryBarriers, const RecordObject& record_obj) override;

void PreCallRecordCmdPipelineBarrier(VkCommandBuffer commandBuffer, VkPipelineStageFlags srcStageMask,
                                     VkPipelineStageFlags dstStageMask, VkDependencyFlags dependencyFlags,
                                     uint32_t memoryBarrierCount, const VkMemoryBarrier* pMemoryBarriers,
                                     uint32_t bufferMemoryBarrierCount, const VkBufferMemoryBarrier* pBufferMemoryBarriers,
                                     uint32_t imageMemoryBarrierCount, const VkImageMemoryBarrier* pImageMemoryBarriers) override;

void PostCallRecordCmdPipelineBarrier(VkCommandBuffer commandBuffer, VkPipelineStageFlags srcStageMask,
                                      VkPipelineStageFlags dstStageMask, VkDependencyFlags dependencyFlags,
                                      uint32_t memoryBarrierCount, const VkMemoryBarrier* pMemoryBarriers,
                                      uint32_t bufferMemoryBarrierCount, const VkBufferMemoryBarrier* pBufferMemoryBarriers,
                                      uint32_t imageMemoryBarrierCount, const VkImageMemoryBarrier* pImageMemoryBarriers,
                                      const RecordObject& record_obj) override;

void PreCallRecordCmdBeginQuery(VkCommandBuffer commandBuffer, VkQueryPool queryPool, uint32_t query,
                                VkQueryControlFlags flags) override;

void PostCallRecordCmdBeginQuery(VkCommandBuffer commandBuffer, VkQueryPool queryPool, uint32_t query, VkQueryControlFlags flags,
                                 const RecordObject& record_obj) override;

void PreCallRecordCmdEndQuery(VkCommandBuffer commandBuffer, VkQueryPool queryPool, uint32_t query) override;

void PostCallRecordCmdEndQuery(VkCommandBuffer commandBuffer, VkQueryPool queryPool, uint32_t query,
                               const RecordObject& record_obj) override;

void PreCallRecordCmdResetQueryPool(VkCommandBuffer commandBuffer, VkQueryPool queryPool, uint32_t firstQuery,
                                    uint32_t queryCount) override;

void PostCallRecordCmdResetQueryPool(VkCommandBuffer commandBuffer, VkQueryPool queryPool, uint32_t firstQuery, uint32_t queryCount,
                                     const RecordObject& record_obj) override;

void PreCallRecordCmdWriteTimestamp(VkCommandBuffer commandBuffer, VkPipelineStageFlagBits pipelineStage, VkQueryPool queryPool,
                                    uint32_t query) override;

void PostCallRecordCmdWriteTimestamp(VkCommandBuffer commandBuffer, VkPipelineStageFlagBits pipelineStage, VkQueryPool queryPool,
                                     uint32_t query, const RecordObject& record_obj) override;

void PreCallRecordCmdCopyQueryPoolResults(VkCommandBuffer commandBuffer, VkQueryPool queryPool, uint32_t firstQuery,
                                          uint32_t queryCount, VkBuffer dstBuffer, VkDeviceSize dstOffset, VkDeviceSize stride,
                                          VkQueryResultFlags flags) override;

void PostCallRecordCmdCopyQueryPoolResults(VkCommandBuffer commandBuffer, VkQueryPool queryPool, uint32_t firstQuery,
                                           uint32_t queryCount, VkBuffer dstBuffer, VkDeviceSize dstOffset, VkDeviceSize stride,
                                           VkQueryResultFlags flags, const RecordObject& record_obj) override;

void PreCallRecordCmdPushConstants(VkCommandBuffer commandBuffer, VkPipelineLayout layout, VkShaderStageFlags stageFlags,
                                   uint32_t offset, uint32_t size, const void* pValues) override;

void PostCallRecordCmdPushConstants(VkCommandBuffer commandBuffer, VkPipelineLayout layout, VkShaderStageFlags stageFlags,
                                    uint32_t offset, uint32_t size, const void* pValues, const RecordObject& record_obj) override;

void PreCallRecordCmdBeginRenderPass(VkCommandBuffer commandBuffer, const VkRenderPassBeginInfo* pRenderPassBegin,
                                     VkSubpassContents contents) override;

void PostCallRecordCmdBeginRenderPass(VkCommandBuffer commandBuffer, const VkRenderPassBeginInfo* pRenderPassBegin,
                                      VkSubpassContents contents, const RecordObject& record_obj) override;

void PreCallRecordCmdNextSubpass(VkCommandBuffer commandBuffer, VkSubpassContents contents) override;

void PostCallRecordCmdNextSubpass(VkCommandBuffer commandBuffer, VkSubpassContents contents,
                                  const RecordObject& record_obj) override;

void PreCallRecordCmdEndRenderPass(VkCommandBuffer commandBuffer) override;

void PostCallRecordCmdEndRenderPass(VkCommandBuffer commandBuffer, const RecordObject& record_obj) override;

void PreCallRecordCmdExecuteCommands(VkCommandBuffer commandBuffer, uint32_t commandBufferCount,
                                     const VkCommandBuffer* pCommandBuffers) override;

void PostCallRecordCmdExecuteCommands(VkCommandBuffer commandBuffer, uint32_t commandBufferCount,
                                      const VkCommandBuffer* pCommandBuffers, const RecordObject& record_obj) override;

void PreCallRecordBindBufferMemory2(VkDevice device, uint32_t bindInfoCount, const VkBindBufferMemoryInfo* pBindInfos) override;

void PostCallRecordBindBufferMemory2(VkDevice device, uint32_t bindInfoCount, const VkBindBufferMemoryInfo* pBindInfos,
                                     const RecordObject& record_obj) override;

void PreCallRecordBindImageMemory2(VkDevice device, uint32_t bindInfoCount, const VkBindImageMemoryInfo* pBindInfos) override;

void PostCallRecordBindImageMemory2(VkDevice device, uint32_t bindInfoCount, const VkBindImageMemoryInfo* pBindInfos,
                                    const RecordObject& record_obj) override;

void PreCallRecordGetDeviceGroupPeerMemoryFeatures(VkDevice device, uint32_t heapIndex, uint32_t localDeviceIndex,
                                                   uint32_t remoteDeviceIndex,
                                                   VkPeerMemoryFeatureFlags* pPeerMemoryFeatures) override;

void PostCallRecordGetDeviceGroupPeerMemoryFeatures(VkDevice device, uint32_t heapIndex, uint32_t localDeviceIndex,
                                                    uint32_t remoteDeviceIndex, VkPeerMemoryFeatureFlags* pPeerMemoryFeatures,
                                                    const RecordObject& record_obj) override;

void PreCallRecordCmdSetDeviceMask(VkCommandBuffer commandBuffer, uint32_t deviceMask) override;

void PostCallRecordCmdSetDeviceMask(VkCommandBuffer commandBuffer, uint32_t deviceMask, const RecordObject& record_obj) override;

void PreCallRecordCmdDispatchBase(VkCommandBuffer commandBuffer, uint32_t baseGroupX, uint32_t baseGroupY, uint32_t baseGroupZ,
                                  uint32_t groupCountX, uint32_t groupCountY, uint32_t groupCountZ) override;

void PostCallRecordCmdDispatchBase(VkCommandBuffer commandBuffer, uint32_t baseGroupX, uint32_t baseGroupY, uint32_t baseGroupZ,
                                   uint32_t groupCountX, uint32_t groupCountY, uint32_t groupCountZ,
                                   const RecordObject& record_obj) override;

void PreCallRecordEnumeratePhysicalDeviceGroups(VkInstance instance, uint32_t* pPhysicalDeviceGroupCount,
                                                VkPhysicalDeviceGroupProperties* pPhysicalDeviceGroupProperties) override;

void PostCallRecordEnumeratePhysicalDeviceGroups(VkInstance instance, uint32_t* pPhysicalDeviceGroupCount,
                                                 VkPhysicalDeviceGroupProperties* pPhysicalDeviceGroupProperties,
                                                 const RecordObject& record_obj) override;

void PreCallRecordGetImageMemoryRequirements2(VkDevice device, const VkImageMemoryRequirementsInfo2* pInfo,
                                              VkMemoryRequirements2* pMemoryRequirements) override;

void PostCallRecordGetImageMemoryRequirements2(VkDevice device, const VkImageMemoryRequirementsInfo2* pInfo,
                                               VkMemoryRequirements2* pMemoryRequirements, const RecordObject& record_obj) override;

void PreCallRecordGetBufferMemoryRequirements2(VkDevice device, const VkBufferMemoryRequirementsInfo2* pInfo,
                                               VkMemoryRequirements2* pMemoryRequirements) override;

void PostCallRecordGetBufferMemoryRequirements2(VkDevice device, const VkBufferMemoryRequirementsInfo2* pInfo,
                                                VkMemoryRequirements2* pMemoryRequirements,
                                                const RecordObject& record_obj) override;

void PreCallRecordGetImageSparseMemoryRequirements2(VkDevice device, const VkImageSparseMemoryRequirementsInfo2* pInfo,
                                                    uint32_t* pSparseMemoryRequirementCount,
                                                    VkSparseImageMemoryRequirements2* pSparseMemoryRequirements) override;

void PostCallRecordGetImageSparseMemoryRequirements2(VkDevice device, const VkImageSparseMemoryRequirementsInfo2* pInfo,
                                                     uint32_t* pSparseMemoryRequirementCount,
                                                     VkSparseImageMemoryRequirements2* pSparseMemoryRequirements,
                                                     const RecordObject& record_obj) override;

void PreCallRecordTrimCommandPool(VkDevice device, VkCommandPool commandPool, VkCommandPoolTrimFlags flags) override;

void PostCallRecordTrimCommandPool(VkDevice device, VkCommandPool commandPool, VkCommandPoolTrimFlags flags,
                                   const RecordObject& record_obj) override;

void PreCallRecordGetDeviceQueue2(VkDevice device, const VkDeviceQueueInfo2* pQueueInfo, VkQueue* pQueue) override;

void PostCallRecordGetDeviceQueue2(VkDevice device, const VkDeviceQueueInfo2* pQueueInfo, VkQueue* pQueue,
                                   const RecordObject& record_obj) override;

void PreCallRecordCreateSamplerYcbcrConversion(VkDevice device, const VkSamplerYcbcrConversionCreateInfo* pCreateInfo,
                                               const VkAllocationCallbacks* pAllocator,
                                               VkSamplerYcbcrConversion* pYcbcrConversion) override;

void PostCallRecordCreateSamplerYcbcrConversion(VkDevice device, const VkSamplerYcbcrConversionCreateInfo* pCreateInfo,
                                                const VkAllocationCallbacks* pAllocator, VkSamplerYcbcrConversion* pYcbcrConversion,
                                                const RecordObject& record_obj) override;

void PreCallRecordDestroySamplerYcbcrConversion(VkDevice device, VkSamplerYcbcrConversion ycbcrConversion,
                                                const VkAllocationCallbacks* pAllocator) override;

void PostCallRecordDestroySamplerYcbcrConversion(VkDevice device, VkSamplerYcbcrConversion ycbcrConversion,
                                                 const VkAllocationCallbacks* pAllocator, const RecordObject& record_obj) override;

void PreCallRecordCreateDescriptorUpdateTemplate(VkDevice device, const VkDescriptorUpdateTemplateCreateInfo* pCreateInfo,
                                                 const VkAllocationCallbacks* pAllocator,
                                                 VkDescriptorUpdateTemplate* pDescriptorUpdateTemplate) override;

void PostCallRecordCreateDescriptorUpdateTemplate(VkDevice device, const VkDescriptorUpdateTemplateCreateInfo* pCreateInfo,
                                                  const VkAllocationCallbacks* pAllocator,
                                                  VkDescriptorUpdateTemplate* pDescriptorUpdateTemplate,
                                                  const RecordObject& record_obj) override;

void PreCallRecordDestroyDescriptorUpdateTemplate(VkDevice device, VkDescriptorUpdateTemplate descriptorUpdateTemplate,
                                                  const VkAllocationCallbacks* pAllocator) override;

void PostCallRecordDestroyDescriptorUpdateTemplate(VkDevice device, VkDescriptorUpdateTemplate descriptorUpdateTemplate,
                                                   const VkAllocationCallbacks* pAllocator,
                                                   const RecordObject& record_obj) override;

void PreCallRecordUpdateDescriptorSetWithTemplate(VkDevice device, VkDescriptorSet descriptorSet,
                                                  VkDescriptorUpdateTemplate descriptorUpdateTemplate, const void* pData) override;

void PostCallRecordUpdateDescriptorSetWithTemplate(VkDevice device, VkDescriptorSet descriptorSet,
                                                   VkDescriptorUpdateTemplate descriptorUpdateTemplate, const void* pData,
                                                   const RecordObject& record_obj) override;

void PreCallRecordGetDescriptorSetLayoutSupport(VkDevice device, const VkDescriptorSetLayoutCreateInfo* pCreateInfo,
                                                VkDescriptorSetLayoutSupport* pSupport) override;

void PostCallRecordGetDescriptorSetLayoutSupport(VkDevice device, const VkDescriptorSetLayoutCreateInfo* pCreateInfo,
                                                 VkDescriptorSetLayoutSupport* pSupport, const RecordObject& record_obj) override;

void PreCallRecordCmdDrawIndirectCount(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset, VkBuffer countBuffer,
                                       VkDeviceSize countBufferOffset, uint32_t maxDrawCount, uint32_t stride) override;

void PostCallRecordCmdDrawIndirectCount(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset, VkBuffer countBuffer,
                                        VkDeviceSize countBufferOffset, uint32_t maxDrawCount, uint32_t stride,
                                        const RecordObject& record_obj) override;

void PreCallRecordCmdDrawIndexedIndirectCount(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                              VkBuffer countBuffer, VkDeviceSize countBufferOffset, uint32_t maxDrawCount,
                                              uint32_t stride) override;

void PostCallRecordCmdDrawIndexedIndirectCount(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                               VkBuffer countBuffer, VkDeviceSize countBufferOffset, uint32_t maxDrawCount,
                                               uint32_t stride, const RecordObject& record_obj) override;

void PreCallRecordCreateRenderPass2(VkDevice device, const VkRenderPassCreateInfo2* pCreateInfo,
                                    const VkAllocationCallbacks* pAllocator, VkRenderPass* pRenderPass) override;

void PostCallRecordCreateRenderPass2(VkDevice device, const VkRenderPassCreateInfo2* pCreateInfo,
                                     const VkAllocationCallbacks* pAllocator, VkRenderPass* pRenderPass,
                                     const RecordObject& record_obj) override;

void PreCallRecordCmdBeginRenderPass2(VkCommandBuffer commandBuffer, const VkRenderPassBeginInfo* pRenderPassBegin,
                                      const VkSubpassBeginInfo* pSubpassBeginInfo) override;

void PostCallRecordCmdBeginRenderPass2(VkCommandBuffer commandBuffer, const VkRenderPassBeginInfo* pRenderPassBegin,
                                       const VkSubpassBeginInfo* pSubpassBeginInfo, const RecordObject& record_obj) override;

void PreCallRecordCmdNextSubpass2(VkCommandBuffer commandBuffer, const VkSubpassBeginInfo* pSubpassBeginInfo,
                                  const VkSubpassEndInfo* pSubpassEndInfo) override;

void PostCallRecordCmdNextSubpass2(VkCommandBuffer commandBuffer, const VkSubpassBeginInfo* pSubpassBeginInfo,
                                   const VkSubpassEndInfo* pSubpassEndInfo, const RecordObject& record_obj) override;

void PreCallRecordCmdEndRenderPass2(VkCommandBuffer commandBuffer, const VkSubpassEndInfo* pSubpassEndInfo) override;

void PostCallRecordCmdEndRenderPass2(VkCommandBuffer commandBuffer, const VkSubpassEndInfo* pSubpassEndInfo,
                                     const RecordObject& record_obj) override;

void PreCallRecordResetQueryPool(VkDevice device, VkQueryPool queryPool, uint32_t firstQuery, uint32_t queryCount) override;

void PostCallRecordResetQueryPool(VkDevice device, VkQueryPool queryPool, uint32_t firstQuery, uint32_t queryCount,
                                  const RecordObject& record_obj) override;

void PreCallRecordGetSemaphoreCounterValue(VkDevice device, VkSemaphore semaphore, uint64_t* pValue) override;

void PostCallRecordGetSemaphoreCounterValue(VkDevice device, VkSemaphore semaphore, uint64_t* pValue,
                                            const RecordObject& record_obj) override;

void PreCallRecordWaitSemaphores(VkDevice device, const VkSemaphoreWaitInfo* pWaitInfo, uint64_t timeout) override;

void PostCallRecordWaitSemaphores(VkDevice device, const VkSemaphoreWaitInfo* pWaitInfo, uint64_t timeout,
                                  const RecordObject& record_obj) override;

void PreCallRecordSignalSemaphore(VkDevice device, const VkSemaphoreSignalInfo* pSignalInfo) override;

void PostCallRecordSignalSemaphore(VkDevice device, const VkSemaphoreSignalInfo* pSignalInfo,
                                   const RecordObject& record_obj) override;

void PreCallRecordGetBufferDeviceAddress(VkDevice device, const VkBufferDeviceAddressInfo* pInfo) override;

void PostCallRecordGetBufferDeviceAddress(VkDevice device, const VkBufferDeviceAddressInfo* pInfo,
                                          const RecordObject& record_obj) override;

void PreCallRecordGetBufferOpaqueCaptureAddress(VkDevice device, const VkBufferDeviceAddressInfo* pInfo) override;

void PostCallRecordGetBufferOpaqueCaptureAddress(VkDevice device, const VkBufferDeviceAddressInfo* pInfo,
                                                 const RecordObject& record_obj) override;

void PreCallRecordGetDeviceMemoryOpaqueCaptureAddress(VkDevice device,
                                                      const VkDeviceMemoryOpaqueCaptureAddressInfo* pInfo) override;

void PostCallRecordGetDeviceMemoryOpaqueCaptureAddress(VkDevice device, const VkDeviceMemoryOpaqueCaptureAddressInfo* pInfo,
                                                       const RecordObject& record_obj) override;

void PreCallRecordCreatePrivateDataSlot(VkDevice device, const VkPrivateDataSlotCreateInfo* pCreateInfo,
                                        const VkAllocationCallbacks* pAllocator, VkPrivateDataSlot* pPrivateDataSlot) override;

void PostCallRecordCreatePrivateDataSlot(VkDevice device, const VkPrivateDataSlotCreateInfo* pCreateInfo,
                                         const VkAllocationCallbacks* pAllocator, VkPrivateDataSlot* pPrivateDataSlot,
                                         const RecordObject& record_obj) override;

void PreCallRecordDestroyPrivateDataSlot(VkDevice device, VkPrivateDataSlot privateDataSlot,
                                         const VkAllocationCallbacks* pAllocator) override;

void PostCallRecordDestroyPrivateDataSlot(VkDevice device, VkPrivateDataSlot privateDataSlot,
                                          const VkAllocationCallbacks* pAllocator, const RecordObject& record_obj) override;

void PreCallRecordSetPrivateData(VkDevice device, VkObjectType objectType, uint64_t objectHandle, VkPrivateDataSlot privateDataSlot,
                                 uint64_t data) override;

void PostCallRecordSetPrivateData(VkDevice device, VkObjectType objectType, uint64_t objectHandle,
                                  VkPrivateDataSlot privateDataSlot, uint64_t data, const RecordObject& record_obj) override;

void PreCallRecordGetPrivateData(VkDevice device, VkObjectType objectType, uint64_t objectHandle, VkPrivateDataSlot privateDataSlot,
                                 uint64_t* pData) override;

void PostCallRecordGetPrivateData(VkDevice device, VkObjectType objectType, uint64_t objectHandle,
                                  VkPrivateDataSlot privateDataSlot, uint64_t* pData, const RecordObject& record_obj) override;

void PreCallRecordCmdSetEvent2(VkCommandBuffer commandBuffer, VkEvent event, const VkDependencyInfo* pDependencyInfo) override;

void PostCallRecordCmdSetEvent2(VkCommandBuffer commandBuffer, VkEvent event, const VkDependencyInfo* pDependencyInfo,
                                const RecordObject& record_obj) override;

void PreCallRecordCmdResetEvent2(VkCommandBuffer commandBuffer, VkEvent event, VkPipelineStageFlags2 stageMask) override;

void PostCallRecordCmdResetEvent2(VkCommandBuffer commandBuffer, VkEvent event, VkPipelineStageFlags2 stageMask,
                                  const RecordObject& record_obj) override;

void PreCallRecordCmdWaitEvents2(VkCommandBuffer commandBuffer, uint32_t eventCount, const VkEvent* pEvents,
                                 const VkDependencyInfo* pDependencyInfos) override;

void PostCallRecordCmdWaitEvents2(VkCommandBuffer commandBuffer, uint32_t eventCount, const VkEvent* pEvents,
                                  const VkDependencyInfo* pDependencyInfos, const RecordObject& record_obj) override;

void PreCallRecordCmdPipelineBarrier2(VkCommandBuffer commandBuffer, const VkDependencyInfo* pDependencyInfo) override;

void PostCallRecordCmdPipelineBarrier2(VkCommandBuffer commandBuffer, const VkDependencyInfo* pDependencyInfo,
                                       const RecordObject& record_obj) override;

void PreCallRecordCmdWriteTimestamp2(VkCommandBuffer commandBuffer, VkPipelineStageFlags2 stage, VkQueryPool queryPool,
                                     uint32_t query) override;

void PostCallRecordCmdWriteTimestamp2(VkCommandBuffer commandBuffer, VkPipelineStageFlags2 stage, VkQueryPool queryPool,
                                      uint32_t query, const RecordObject& record_obj) override;

void PreCallRecordQueueSubmit2(VkQueue queue, uint32_t submitCount, const VkSubmitInfo2* pSubmits, VkFence fence) override;

void PostCallRecordQueueSubmit2(VkQueue queue, uint32_t submitCount, const VkSubmitInfo2* pSubmits, VkFence fence,
                                const RecordObject& record_obj) override;

void PreCallRecordCmdCopyBuffer2(VkCommandBuffer commandBuffer, const VkCopyBufferInfo2* pCopyBufferInfo) override;

void PostCallRecordCmdCopyBuffer2(VkCommandBuffer commandBuffer, const VkCopyBufferInfo2* pCopyBufferInfo,
                                  const RecordObject& record_obj) override;

void PreCallRecordCmdCopyImage2(VkCommandBuffer commandBuffer, const VkCopyImageInfo2* pCopyImageInfo) override;

void PostCallRecordCmdCopyImage2(VkCommandBuffer commandBuffer, const VkCopyImageInfo2* pCopyImageInfo,
                                 const RecordObject& record_obj) override;

void PreCallRecordCmdCopyBufferToImage2(VkCommandBuffer commandBuffer,
                                        const VkCopyBufferToImageInfo2* pCopyBufferToImageInfo) override;

void PostCallRecordCmdCopyBufferToImage2(VkCommandBuffer commandBuffer, const VkCopyBufferToImageInfo2* pCopyBufferToImageInfo,
                                         const RecordObject& record_obj) override;

void PreCallRecordCmdCopyImageToBuffer2(VkCommandBuffer commandBuffer,
                                        const VkCopyImageToBufferInfo2* pCopyImageToBufferInfo) override;

void PostCallRecordCmdCopyImageToBuffer2(VkCommandBuffer commandBuffer, const VkCopyImageToBufferInfo2* pCopyImageToBufferInfo,
                                         const RecordObject& record_obj) override;

void PreCallRecordCmdBlitImage2(VkCommandBuffer commandBuffer, const VkBlitImageInfo2* pBlitImageInfo) override;

void PostCallRecordCmdBlitImage2(VkCommandBuffer commandBuffer, const VkBlitImageInfo2* pBlitImageInfo,
                                 const RecordObject& record_obj) override;

void PreCallRecordCmdResolveImage2(VkCommandBuffer commandBuffer, const VkResolveImageInfo2* pResolveImageInfo) override;

void PostCallRecordCmdResolveImage2(VkCommandBuffer commandBuffer, const VkResolveImageInfo2* pResolveImageInfo,
                                    const RecordObject& record_obj) override;

void PreCallRecordCmdBeginRendering(VkCommandBuffer commandBuffer, const VkRenderingInfo* pRenderingInfo) override;

void PostCallRecordCmdBeginRendering(VkCommandBuffer commandBuffer, const VkRenderingInfo* pRenderingInfo,
                                     const RecordObject& record_obj) override;

void PreCallRecordCmdEndRendering(VkCommandBuffer commandBuffer) override;

void PostCallRecordCmdEndRendering(VkCommandBuffer commandBuffer, const RecordObject& record_obj) override;

void PreCallRecordCmdSetCullMode(VkCommandBuffer commandBuffer, VkCullModeFlags cullMode) override;

void PostCallRecordCmdSetCullMode(VkCommandBuffer commandBuffer, VkCullModeFlags cullMode, const RecordObject& record_obj) override;

void PreCallRecordCmdSetFrontFace(VkCommandBuffer commandBuffer, VkFrontFace frontFace) override;

void PostCallRecordCmdSetFrontFace(VkCommandBuffer commandBuffer, VkFrontFace frontFace, const RecordObject& record_obj) override;

void PreCallRecordCmdSetPrimitiveTopology(VkCommandBuffer commandBuffer, VkPrimitiveTopology primitiveTopology) override;

void PostCallRecordCmdSetPrimitiveTopology(VkCommandBuffer commandBuffer, VkPrimitiveTopology primitiveTopology,
                                           const RecordObject& record_obj) override;

void PreCallRecordCmdSetViewportWithCount(VkCommandBuffer commandBuffer, uint32_t viewportCount,
                                          const VkViewport* pViewports) override;

void PostCallRecordCmdSetViewportWithCount(VkCommandBuffer commandBuffer, uint32_t viewportCount, const VkViewport* pViewports,
                                           const RecordObject& record_obj) override;

void PreCallRecordCmdSetScissorWithCount(VkCommandBuffer commandBuffer, uint32_t scissorCount, const VkRect2D* pScissors) override;

void PostCallRecordCmdSetScissorWithCount(VkCommandBuffer commandBuffer, uint32_t scissorCount, const VkRect2D* pScissors,
                                          const RecordObject& record_obj) override;

void PreCallRecordCmdBindVertexBuffers2(VkCommandBuffer commandBuffer, uint32_t firstBinding, uint32_t bindingCount,
                                        const VkBuffer* pBuffers, const VkDeviceSize* pOffsets, const VkDeviceSize* pSizes,
                                        const VkDeviceSize* pStrides) override;

void PostCallRecordCmdBindVertexBuffers2(VkCommandBuffer commandBuffer, uint32_t firstBinding, uint32_t bindingCount,
                                         const VkBuffer* pBuffers, const VkDeviceSize* pOffsets, const VkDeviceSize* pSizes,
                                         const VkDeviceSize* pStrides, const RecordObject& record_obj) override;

void PreCallRecordCmdSetDepthTestEnable(VkCommandBuffer commandBuffer, VkBool32 depthTestEnable) override;

void PostCallRecordCmdSetDepthTestEnable(VkCommandBuffer commandBuffer, VkBool32 depthTestEnable,
                                         const RecordObject& record_obj) override;

void PreCallRecordCmdSetDepthWriteEnable(VkCommandBuffer commandBuffer, VkBool32 depthWriteEnable) override;

void PostCallRecordCmdSetDepthWriteEnable(VkCommandBuffer commandBuffer, VkBool32 depthWriteEnable,
                                          const RecordObject& record_obj) override;

void PreCallRecordCmdSetDepthCompareOp(VkCommandBuffer commandBuffer, VkCompareOp depthCompareOp) override;

void PostCallRecordCmdSetDepthCompareOp(VkCommandBuffer commandBuffer, VkCompareOp depthCompareOp,
                                        const RecordObject& record_obj) override;

void PreCallRecordCmdSetDepthBoundsTestEnable(VkCommandBuffer commandBuffer, VkBool32 depthBoundsTestEnable) override;

void PostCallRecordCmdSetDepthBoundsTestEnable(VkCommandBuffer commandBuffer, VkBool32 depthBoundsTestEnable,
                                               const RecordObject& record_obj) override;

void PreCallRecordCmdSetStencilTestEnable(VkCommandBuffer commandBuffer, VkBool32 stencilTestEnable) override;

void PostCallRecordCmdSetStencilTestEnable(VkCommandBuffer commandBuffer, VkBool32 stencilTestEnable,
                                           const RecordObject& record_obj) override;

void PreCallRecordCmdSetStencilOp(VkCommandBuffer commandBuffer, VkStencilFaceFlags faceMask, VkStencilOp failOp,
                                  VkStencilOp passOp, VkStencilOp depthFailOp, VkCompareOp compareOp) override;

void PostCallRecordCmdSetStencilOp(VkCommandBuffer commandBuffer, VkStencilFaceFlags faceMask, VkStencilOp failOp,
                                   VkStencilOp passOp, VkStencilOp depthFailOp, VkCompareOp compareOp,
                                   const RecordObject& record_obj) override;

void PreCallRecordCmdSetRasterizerDiscardEnable(VkCommandBuffer commandBuffer, VkBool32 rasterizerDiscardEnable) override;

void PostCallRecordCmdSetRasterizerDiscardEnable(VkCommandBuffer commandBuffer, VkBool32 rasterizerDiscardEnable,
                                                 const RecordObject& record_obj) override;

void PreCallRecordCmdSetDepthBiasEnable(VkCommandBuffer commandBuffer, VkBool32 depthBiasEnable) override;

void PostCallRecordCmdSetDepthBiasEnable(VkCommandBuffer commandBuffer, VkBool32 depthBiasEnable,
                                         const RecordObject& record_obj) override;

void PreCallRecordCmdSetPrimitiveRestartEnable(VkCommandBuffer commandBuffer, VkBool32 primitiveRestartEnable) override;

void PostCallRecordCmdSetPrimitiveRestartEnable(VkCommandBuffer commandBuffer, VkBool32 primitiveRestartEnable,
                                                const RecordObject& record_obj) override;

void PreCallRecordGetDeviceBufferMemoryRequirements(VkDevice device, const VkDeviceBufferMemoryRequirements* pInfo,
                                                    VkMemoryRequirements2* pMemoryRequirements) override;

void PostCallRecordGetDeviceBufferMemoryRequirements(VkDevice device, const VkDeviceBufferMemoryRequirements* pInfo,
                                                     VkMemoryRequirements2* pMemoryRequirements,
                                                     const RecordObject& record_obj) override;

void PreCallRecordGetDeviceImageMemoryRequirements(VkDevice device, const VkDeviceImageMemoryRequirements* pInfo,
                                                   VkMemoryRequirements2* pMemoryRequirements) override;

void PostCallRecordGetDeviceImageMemoryRequirements(VkDevice device, const VkDeviceImageMemoryRequirements* pInfo,
                                                    VkMemoryRequirements2* pMemoryRequirements,
                                                    const RecordObject& record_obj) override;

void PreCallRecordGetDeviceImageSparseMemoryRequirements(VkDevice device, const VkDeviceImageMemoryRequirements* pInfo,
                                                         uint32_t* pSparseMemoryRequirementCount,
                                                         VkSparseImageMemoryRequirements2* pSparseMemoryRequirements) override;

void PostCallRecordGetDeviceImageSparseMemoryRequirements(VkDevice device, const VkDeviceImageMemoryRequirements* pInfo,
                                                          uint32_t* pSparseMemoryRequirementCount,
                                                          VkSparseImageMemoryRequirements2* pSparseMemoryRequirements,
                                                          const RecordObject& record_obj) override;

void PreCallRecordDestroySurfaceKHR(VkInstance instance, VkSurfaceKHR surface, const VkAllocationCallbacks* pAllocator) override;

void PostCallRecordDestroySurfaceKHR(VkInstance instance, VkSurfaceKHR surface, const VkAllocationCallbacks* pAllocator,
                                     const RecordObject& record_obj) override;

void PreCallRecordGetPhysicalDeviceSurfaceSupportKHR(VkPhysicalDevice physicalDevice, uint32_t queueFamilyIndex,
                                                     VkSurfaceKHR surface, VkBool32* pSupported) override;

void PostCallRecordGetPhysicalDeviceSurfaceSupportKHR(VkPhysicalDevice physicalDevice, uint32_t queueFamilyIndex,
                                                      VkSurfaceKHR surface, VkBool32* pSupported,
                                                      const RecordObject& record_obj) override;

void PreCallRecordGetPhysicalDeviceSurfaceCapabilitiesKHR(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface,
                                                          VkSurfaceCapabilitiesKHR* pSurfaceCapabilities) override;

void PostCallRecordGetPhysicalDeviceSurfaceCapabilitiesKHR(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface,
                                                           VkSurfaceCapabilitiesKHR* pSurfaceCapabilities,
                                                           const RecordObject& record_obj) override;

void PreCallRecordGetPhysicalDeviceSurfaceFormatsKHR(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface,
                                                     uint32_t* pSurfaceFormatCount, VkSurfaceFormatKHR* pSurfaceFormats) override;

void PostCallRecordGetPhysicalDeviceSurfaceFormatsKHR(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface,
                                                      uint32_t* pSurfaceFormatCount, VkSurfaceFormatKHR* pSurfaceFormats,
                                                      const RecordObject& record_obj) override;

void PreCallRecordGetPhysicalDeviceSurfacePresentModesKHR(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface,
                                                          uint32_t* pPresentModeCount, VkPresentModeKHR* pPresentModes) override;

void PostCallRecordGetPhysicalDeviceSurfacePresentModesKHR(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface,
                                                           uint32_t* pPresentModeCount, VkPresentModeKHR* pPresentModes,
                                                           const RecordObject& record_obj) override;

void PreCallRecordCreateSwapchainKHR(VkDevice device, const VkSwapchainCreateInfoKHR* pCreateInfo,
                                     const VkAllocationCallbacks* pAllocator, VkSwapchainKHR* pSwapchain) override;

void PostCallRecordCreateSwapchainKHR(VkDevice device, const VkSwapchainCreateInfoKHR* pCreateInfo,
                                      const VkAllocationCallbacks* pAllocator, VkSwapchainKHR* pSwapchain,
                                      const RecordObject& record_obj) override;

void PreCallRecordDestroySwapchainKHR(VkDevice device, VkSwapchainKHR swapchain, const VkAllocationCallbacks* pAllocator) override;

void PostCallRecordDestroySwapchainKHR(VkDevice device, VkSwapchainKHR swapchain, const VkAllocationCallbacks* pAllocator,
                                       const RecordObject& record_obj) override;

void PreCallRecordGetSwapchainImagesKHR(VkDevice device, VkSwapchainKHR swapchain, uint32_t* pSwapchainImageCount,
                                        VkImage* pSwapchainImages) override;

void PostCallRecordGetSwapchainImagesKHR(VkDevice device, VkSwapchainKHR swapchain, uint32_t* pSwapchainImageCount,
                                         VkImage* pSwapchainImages, const RecordObject& record_obj) override;

void PreCallRecordAcquireNextImageKHR(VkDevice device, VkSwapchainKHR swapchain, uint64_t timeout, VkSemaphore semaphore,
                                      VkFence fence, uint32_t* pImageIndex) override;

void PostCallRecordAcquireNextImageKHR(VkDevice device, VkSwapchainKHR swapchain, uint64_t timeout, VkSemaphore semaphore,
                                       VkFence fence, uint32_t* pImageIndex, const RecordObject& record_obj) override;

void PreCallRecordQueuePresentKHR(VkQueue queue, const VkPresentInfoKHR* pPresentInfo) override;

void PostCallRecordQueuePresentKHR(VkQueue queue, const VkPresentInfoKHR* pPresentInfo, const RecordObject& record_obj) override;

void PreCallRecordGetDeviceGroupPresentCapabilitiesKHR(
    VkDevice device, VkDeviceGroupPresentCapabilitiesKHR* pDeviceGroupPresentCapabilities) override;

void PostCallRecordGetDeviceGroupPresentCapabilitiesKHR(VkDevice device,
                                                        VkDeviceGroupPresentCapabilitiesKHR* pDeviceGroupPresentCapabilities,
                                                        const RecordObject& record_obj) override;

void PreCallRecordGetDeviceGroupSurfacePresentModesKHR(VkDevice device, VkSurfaceKHR surface,
                                                       VkDeviceGroupPresentModeFlagsKHR* pModes) override;

void PostCallRecordGetDeviceGroupSurfacePresentModesKHR(VkDevice device, VkSurfaceKHR surface,
                                                        VkDeviceGroupPresentModeFlagsKHR* pModes,
                                                        const RecordObject& record_obj) override;

void PreCallRecordGetPhysicalDevicePresentRectanglesKHR(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface, uint32_t* pRectCount,
                                                        VkRect2D* pRects) override;

void PostCallRecordGetPhysicalDevicePresentRectanglesKHR(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface,
                                                         uint32_t* pRectCount, VkRect2D* pRects,
                                                         const RecordObject& record_obj) override;

void PreCallRecordAcquireNextImage2KHR(VkDevice device, const VkAcquireNextImageInfoKHR* pAcquireInfo,
                                       uint32_t* pImageIndex) override;

void PostCallRecordAcquireNextImage2KHR(VkDevice device, const VkAcquireNextImageInfoKHR* pAcquireInfo, uint32_t* pImageIndex,
                                        const RecordObject& record_obj) override;

void PreCallRecordGetDisplayPlaneSupportedDisplaysKHR(VkPhysicalDevice physicalDevice, uint32_t planeIndex, uint32_t* pDisplayCount,
                                                      VkDisplayKHR* pDisplays) override;

void PostCallRecordGetDisplayPlaneSupportedDisplaysKHR(VkPhysicalDevice physicalDevice, uint32_t planeIndex,
                                                       uint32_t* pDisplayCount, VkDisplayKHR* pDisplays,
                                                       const RecordObject& record_obj) override;

void PreCallRecordGetDisplayModePropertiesKHR(VkPhysicalDevice physicalDevice, VkDisplayKHR display, uint32_t* pPropertyCount,
                                              VkDisplayModePropertiesKHR* pProperties) override;

void PostCallRecordGetDisplayModePropertiesKHR(VkPhysicalDevice physicalDevice, VkDisplayKHR display, uint32_t* pPropertyCount,
                                               VkDisplayModePropertiesKHR* pProperties, const RecordObject& record_obj) override;

void PreCallRecordCreateDisplayModeKHR(VkPhysicalDevice physicalDevice, VkDisplayKHR display,
                                       const VkDisplayModeCreateInfoKHR* pCreateInfo, const VkAllocationCallbacks* pAllocator,
                                       VkDisplayModeKHR* pMode) override;

void PostCallRecordCreateDisplayModeKHR(VkPhysicalDevice physicalDevice, VkDisplayKHR display,
                                        const VkDisplayModeCreateInfoKHR* pCreateInfo, const VkAllocationCallbacks* pAllocator,
                                        VkDisplayModeKHR* pMode, const RecordObject& record_obj) override;

void PreCallRecordGetDisplayPlaneCapabilitiesKHR(VkPhysicalDevice physicalDevice, VkDisplayModeKHR mode, uint32_t planeIndex,
                                                 VkDisplayPlaneCapabilitiesKHR* pCapabilities) override;

void PostCallRecordGetDisplayPlaneCapabilitiesKHR(VkPhysicalDevice physicalDevice, VkDisplayModeKHR mode, uint32_t planeIndex,
                                                  VkDisplayPlaneCapabilitiesKHR* pCapabilities,
                                                  const RecordObject& record_obj) override;

void PreCallRecordCreateDisplayPlaneSurfaceKHR(VkInstance instance, const VkDisplaySurfaceCreateInfoKHR* pCreateInfo,
                                               const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface) override;

void PostCallRecordCreateDisplayPlaneSurfaceKHR(VkInstance instance, const VkDisplaySurfaceCreateInfoKHR* pCreateInfo,
                                                const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface,
                                                const RecordObject& record_obj) override;

void PreCallRecordCreateSharedSwapchainsKHR(VkDevice device, uint32_t swapchainCount, const VkSwapchainCreateInfoKHR* pCreateInfos,
                                            const VkAllocationCallbacks* pAllocator, VkSwapchainKHR* pSwapchains) override;

void PostCallRecordCreateSharedSwapchainsKHR(VkDevice device, uint32_t swapchainCount, const VkSwapchainCreateInfoKHR* pCreateInfos,
                                             const VkAllocationCallbacks* pAllocator, VkSwapchainKHR* pSwapchains,
                                             const RecordObject& record_obj) override;

#ifdef VK_USE_PLATFORM_XLIB_KHR
void PreCallRecordCreateXlibSurfaceKHR(VkInstance instance, const VkXlibSurfaceCreateInfoKHR* pCreateInfo,
                                       const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface) override;

void PostCallRecordCreateXlibSurfaceKHR(VkInstance instance, const VkXlibSurfaceCreateInfoKHR* pCreateInfo,
                                        const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface,
                                        const RecordObject& record_obj) override;

#endif  // VK_USE_PLATFORM_XLIB_KHR
#ifdef VK_USE_PLATFORM_XCB_KHR
void PreCallRecordCreateXcbSurfaceKHR(VkInstance instance, const VkXcbSurfaceCreateInfoKHR* pCreateInfo,
                                      const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface) override;

void PostCallRecordCreateXcbSurfaceKHR(VkInstance instance, const VkXcbSurfaceCreateInfoKHR* pCreateInfo,
                                       const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface,
                                       const RecordObject& record_obj) override;

#endif  // VK_USE_PLATFORM_XCB_KHR
#ifdef VK_USE_PLATFORM_WAYLAND_KHR
void PreCallRecordCreateWaylandSurfaceKHR(VkInstance instance, const VkWaylandSurfaceCreateInfoKHR* pCreateInfo,
                                          const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface) override;

void PostCallRecordCreateWaylandSurfaceKHR(VkInstance instance, const VkWaylandSurfaceCreateInfoKHR* pCreateInfo,
                                           const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface,
                                           const RecordObject& record_obj) override;

#endif  // VK_USE_PLATFORM_WAYLAND_KHR
#ifdef VK_USE_PLATFORM_ANDROID_KHR
void PreCallRecordCreateAndroidSurfaceKHR(VkInstance instance, const VkAndroidSurfaceCreateInfoKHR* pCreateInfo,
                                          const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface) override;

void PostCallRecordCreateAndroidSurfaceKHR(VkInstance instance, const VkAndroidSurfaceCreateInfoKHR* pCreateInfo,
                                           const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface,
                                           const RecordObject& record_obj) override;

#endif  // VK_USE_PLATFORM_ANDROID_KHR
#ifdef VK_USE_PLATFORM_WIN32_KHR
void PreCallRecordCreateWin32SurfaceKHR(VkInstance instance, const VkWin32SurfaceCreateInfoKHR* pCreateInfo,
                                        const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface) override;

void PostCallRecordCreateWin32SurfaceKHR(VkInstance instance, const VkWin32SurfaceCreateInfoKHR* pCreateInfo,
                                         const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface,
                                         const RecordObject& record_obj) override;

#endif  // VK_USE_PLATFORM_WIN32_KHR
void PreCallRecordCreateVideoSessionKHR(VkDevice device, const VkVideoSessionCreateInfoKHR* pCreateInfo,
                                        const VkAllocationCallbacks* pAllocator, VkVideoSessionKHR* pVideoSession) override;

void PostCallRecordCreateVideoSessionKHR(VkDevice device, const VkVideoSessionCreateInfoKHR* pCreateInfo,
                                         const VkAllocationCallbacks* pAllocator, VkVideoSessionKHR* pVideoSession,
                                         const RecordObject& record_obj) override;

void PreCallRecordDestroyVideoSessionKHR(VkDevice device, VkVideoSessionKHR videoSession,
                                         const VkAllocationCallbacks* pAllocator) override;

void PostCallRecordDestroyVideoSessionKHR(VkDevice device, VkVideoSessionKHR videoSession, const VkAllocationCallbacks* pAllocator,
                                          const RecordObject& record_obj) override;

void PreCallRecordGetVideoSessionMemoryRequirementsKHR(VkDevice device, VkVideoSessionKHR videoSession,
                                                       uint32_t* pMemoryRequirementsCount,
                                                       VkVideoSessionMemoryRequirementsKHR* pMemoryRequirements) override;

void PostCallRecordGetVideoSessionMemoryRequirementsKHR(VkDevice device, VkVideoSessionKHR videoSession,
                                                        uint32_t* pMemoryRequirementsCount,
                                                        VkVideoSessionMemoryRequirementsKHR* pMemoryRequirements,
                                                        const RecordObject& record_obj) override;

void PreCallRecordBindVideoSessionMemoryKHR(VkDevice device, VkVideoSessionKHR videoSession, uint32_t bindSessionMemoryInfoCount,
                                            const VkBindVideoSessionMemoryInfoKHR* pBindSessionMemoryInfos) override;

void PostCallRecordBindVideoSessionMemoryKHR(VkDevice device, VkVideoSessionKHR videoSession, uint32_t bindSessionMemoryInfoCount,
                                             const VkBindVideoSessionMemoryInfoKHR* pBindSessionMemoryInfos,
                                             const RecordObject& record_obj) override;

void PreCallRecordCreateVideoSessionParametersKHR(VkDevice device, const VkVideoSessionParametersCreateInfoKHR* pCreateInfo,
                                                  const VkAllocationCallbacks* pAllocator,
                                                  VkVideoSessionParametersKHR* pVideoSessionParameters) override;

void PostCallRecordCreateVideoSessionParametersKHR(VkDevice device, const VkVideoSessionParametersCreateInfoKHR* pCreateInfo,
                                                   const VkAllocationCallbacks* pAllocator,
                                                   VkVideoSessionParametersKHR* pVideoSessionParameters,
                                                   const RecordObject& record_obj) override;

void PreCallRecordUpdateVideoSessionParametersKHR(VkDevice device, VkVideoSessionParametersKHR videoSessionParameters,
                                                  const VkVideoSessionParametersUpdateInfoKHR* pUpdateInfo) override;

void PostCallRecordUpdateVideoSessionParametersKHR(VkDevice device, VkVideoSessionParametersKHR videoSessionParameters,
                                                   const VkVideoSessionParametersUpdateInfoKHR* pUpdateInfo,
                                                   const RecordObject& record_obj) override;

void PreCallRecordDestroyVideoSessionParametersKHR(VkDevice device, VkVideoSessionParametersKHR videoSessionParameters,
                                                   const VkAllocationCallbacks* pAllocator) override;

void PostCallRecordDestroyVideoSessionParametersKHR(VkDevice device, VkVideoSessionParametersKHR videoSessionParameters,
                                                    const VkAllocationCallbacks* pAllocator,
                                                    const RecordObject& record_obj) override;

void PreCallRecordCmdBeginVideoCodingKHR(VkCommandBuffer commandBuffer, const VkVideoBeginCodingInfoKHR* pBeginInfo) override;

void PostCallRecordCmdBeginVideoCodingKHR(VkCommandBuffer commandBuffer, const VkVideoBeginCodingInfoKHR* pBeginInfo,
                                          const RecordObject& record_obj) override;

void PreCallRecordCmdEndVideoCodingKHR(VkCommandBuffer commandBuffer, const VkVideoEndCodingInfoKHR* pEndCodingInfo) override;

void PostCallRecordCmdEndVideoCodingKHR(VkCommandBuffer commandBuffer, const VkVideoEndCodingInfoKHR* pEndCodingInfo,
                                        const RecordObject& record_obj) override;

void PreCallRecordCmdControlVideoCodingKHR(VkCommandBuffer commandBuffer,
                                           const VkVideoCodingControlInfoKHR* pCodingControlInfo) override;

void PostCallRecordCmdControlVideoCodingKHR(VkCommandBuffer commandBuffer, const VkVideoCodingControlInfoKHR* pCodingControlInfo,
                                            const RecordObject& record_obj) override;

void PreCallRecordCmdDecodeVideoKHR(VkCommandBuffer commandBuffer, const VkVideoDecodeInfoKHR* pDecodeInfo) override;

void PostCallRecordCmdDecodeVideoKHR(VkCommandBuffer commandBuffer, const VkVideoDecodeInfoKHR* pDecodeInfo,
                                     const RecordObject& record_obj) override;

void PreCallRecordCmdBeginRenderingKHR(VkCommandBuffer commandBuffer, const VkRenderingInfo* pRenderingInfo) override;

void PostCallRecordCmdBeginRenderingKHR(VkCommandBuffer commandBuffer, const VkRenderingInfo* pRenderingInfo,
                                        const RecordObject& record_obj) override;

void PreCallRecordCmdEndRenderingKHR(VkCommandBuffer commandBuffer) override;

void PostCallRecordCmdEndRenderingKHR(VkCommandBuffer commandBuffer, const RecordObject& record_obj) override;

void PreCallRecordGetDeviceGroupPeerMemoryFeaturesKHR(VkDevice device, uint32_t heapIndex, uint32_t localDeviceIndex,
                                                      uint32_t remoteDeviceIndex,
                                                      VkPeerMemoryFeatureFlags* pPeerMemoryFeatures) override;

void PostCallRecordGetDeviceGroupPeerMemoryFeaturesKHR(VkDevice device, uint32_t heapIndex, uint32_t localDeviceIndex,
                                                       uint32_t remoteDeviceIndex, VkPeerMemoryFeatureFlags* pPeerMemoryFeatures,
                                                       const RecordObject& record_obj) override;

void PreCallRecordCmdSetDeviceMaskKHR(VkCommandBuffer commandBuffer, uint32_t deviceMask) override;

void PostCallRecordCmdSetDeviceMaskKHR(VkCommandBuffer commandBuffer, uint32_t deviceMask, const RecordObject& record_obj) override;

void PreCallRecordCmdDispatchBaseKHR(VkCommandBuffer commandBuffer, uint32_t baseGroupX, uint32_t baseGroupY, uint32_t baseGroupZ,
                                     uint32_t groupCountX, uint32_t groupCountY, uint32_t groupCountZ) override;

void PostCallRecordCmdDispatchBaseKHR(VkCommandBuffer commandBuffer, uint32_t baseGroupX, uint32_t baseGroupY, uint32_t baseGroupZ,
                                      uint32_t groupCountX, uint32_t groupCountY, uint32_t groupCountZ,
                                      const RecordObject& record_obj) override;

void PreCallRecordTrimCommandPoolKHR(VkDevice device, VkCommandPool commandPool, VkCommandPoolTrimFlags flags) override;

void PostCallRecordTrimCommandPoolKHR(VkDevice device, VkCommandPool commandPool, VkCommandPoolTrimFlags flags,
                                      const RecordObject& record_obj) override;

void PreCallRecordEnumeratePhysicalDeviceGroupsKHR(VkInstance instance, uint32_t* pPhysicalDeviceGroupCount,
                                                   VkPhysicalDeviceGroupProperties* pPhysicalDeviceGroupProperties) override;

void PostCallRecordEnumeratePhysicalDeviceGroupsKHR(VkInstance instance, uint32_t* pPhysicalDeviceGroupCount,
                                                    VkPhysicalDeviceGroupProperties* pPhysicalDeviceGroupProperties,
                                                    const RecordObject& record_obj) override;

#ifdef VK_USE_PLATFORM_WIN32_KHR
void PreCallRecordGetMemoryWin32HandleKHR(VkDevice device, const VkMemoryGetWin32HandleInfoKHR* pGetWin32HandleInfo,
                                          HANDLE* pHandle) override;

void PostCallRecordGetMemoryWin32HandleKHR(VkDevice device, const VkMemoryGetWin32HandleInfoKHR* pGetWin32HandleInfo,
                                           HANDLE* pHandle, const RecordObject& record_obj) override;

#endif  // VK_USE_PLATFORM_WIN32_KHR
#ifdef VK_USE_PLATFORM_WIN32_KHR
void PreCallRecordGetMemoryWin32HandlePropertiesKHR(VkDevice device, VkExternalMemoryHandleTypeFlagBits handleType, HANDLE handle,
                                                    VkMemoryWin32HandlePropertiesKHR* pMemoryWin32HandleProperties) override;

void PostCallRecordGetMemoryWin32HandlePropertiesKHR(VkDevice device, VkExternalMemoryHandleTypeFlagBits handleType, HANDLE handle,
                                                     VkMemoryWin32HandlePropertiesKHR* pMemoryWin32HandleProperties,
                                                     const RecordObject& record_obj) override;

#endif  // VK_USE_PLATFORM_WIN32_KHR
void PreCallRecordGetMemoryFdKHR(VkDevice device, const VkMemoryGetFdInfoKHR* pGetFdInfo, int* pFd) override;

void PostCallRecordGetMemoryFdKHR(VkDevice device, const VkMemoryGetFdInfoKHR* pGetFdInfo, int* pFd,
                                  const RecordObject& record_obj) override;

void PreCallRecordGetMemoryFdPropertiesKHR(VkDevice device, VkExternalMemoryHandleTypeFlagBits handleType, int fd,
                                           VkMemoryFdPropertiesKHR* pMemoryFdProperties) override;

void PostCallRecordGetMemoryFdPropertiesKHR(VkDevice device, VkExternalMemoryHandleTypeFlagBits handleType, int fd,
                                            VkMemoryFdPropertiesKHR* pMemoryFdProperties, const RecordObject& record_obj) override;

#ifdef VK_USE_PLATFORM_WIN32_KHR
void PreCallRecordImportSemaphoreWin32HandleKHR(
    VkDevice device, const VkImportSemaphoreWin32HandleInfoKHR* pImportSemaphoreWin32HandleInfo) override;

void PostCallRecordImportSemaphoreWin32HandleKHR(VkDevice device,
                                                 const VkImportSemaphoreWin32HandleInfoKHR* pImportSemaphoreWin32HandleInfo,
                                                 const RecordObject& record_obj) override;

#endif  // VK_USE_PLATFORM_WIN32_KHR
#ifdef VK_USE_PLATFORM_WIN32_KHR
void PreCallRecordGetSemaphoreWin32HandleKHR(VkDevice device, const VkSemaphoreGetWin32HandleInfoKHR* pGetWin32HandleInfo,
                                             HANDLE* pHandle) override;

void PostCallRecordGetSemaphoreWin32HandleKHR(VkDevice device, const VkSemaphoreGetWin32HandleInfoKHR* pGetWin32HandleInfo,
                                              HANDLE* pHandle, const RecordObject& record_obj) override;

#endif  // VK_USE_PLATFORM_WIN32_KHR
void PreCallRecordImportSemaphoreFdKHR(VkDevice device, const VkImportSemaphoreFdInfoKHR* pImportSemaphoreFdInfo) override;

void PostCallRecordImportSemaphoreFdKHR(VkDevice device, const VkImportSemaphoreFdInfoKHR* pImportSemaphoreFdInfo,
                                        const RecordObject& record_obj) override;

void PreCallRecordGetSemaphoreFdKHR(VkDevice device, const VkSemaphoreGetFdInfoKHR* pGetFdInfo, int* pFd) override;

void PostCallRecordGetSemaphoreFdKHR(VkDevice device, const VkSemaphoreGetFdInfoKHR* pGetFdInfo, int* pFd,
                                     const RecordObject& record_obj) override;

void PreCallRecordCmdPushDescriptorSetKHR(VkCommandBuffer commandBuffer, VkPipelineBindPoint pipelineBindPoint,
                                          VkPipelineLayout layout, uint32_t set, uint32_t descriptorWriteCount,
                                          const VkWriteDescriptorSet* pDescriptorWrites) override;

void PostCallRecordCmdPushDescriptorSetKHR(VkCommandBuffer commandBuffer, VkPipelineBindPoint pipelineBindPoint,
                                           VkPipelineLayout layout, uint32_t set, uint32_t descriptorWriteCount,
                                           const VkWriteDescriptorSet* pDescriptorWrites, const RecordObject& record_obj) override;

void PreCallRecordCmdPushDescriptorSetWithTemplateKHR(VkCommandBuffer commandBuffer,
                                                      VkDescriptorUpdateTemplate descriptorUpdateTemplate, VkPipelineLayout layout,
                                                      uint32_t set, const void* pData) override;

void PostCallRecordCmdPushDescriptorSetWithTemplateKHR(VkCommandBuffer commandBuffer,
                                                       VkDescriptorUpdateTemplate descriptorUpdateTemplate, VkPipelineLayout layout,
                                                       uint32_t set, const void* pData, const RecordObject& record_obj) override;

void PreCallRecordCreateDescriptorUpdateTemplateKHR(VkDevice device, const VkDescriptorUpdateTemplateCreateInfo* pCreateInfo,
                                                    const VkAllocationCallbacks* pAllocator,
                                                    VkDescriptorUpdateTemplate* pDescriptorUpdateTemplate) override;

void PostCallRecordCreateDescriptorUpdateTemplateKHR(VkDevice device, const VkDescriptorUpdateTemplateCreateInfo* pCreateInfo,
                                                     const VkAllocationCallbacks* pAllocator,
                                                     VkDescriptorUpdateTemplate* pDescriptorUpdateTemplate,
                                                     const RecordObject& record_obj) override;

void PreCallRecordDestroyDescriptorUpdateTemplateKHR(VkDevice device, VkDescriptorUpdateTemplate descriptorUpdateTemplate,
                                                     const VkAllocationCallbacks* pAllocator) override;

void PostCallRecordDestroyDescriptorUpdateTemplateKHR(VkDevice device, VkDescriptorUpdateTemplate descriptorUpdateTemplate,
                                                      const VkAllocationCallbacks* pAllocator,
                                                      const RecordObject& record_obj) override;

void PreCallRecordUpdateDescriptorSetWithTemplateKHR(VkDevice device, VkDescriptorSet descriptorSet,
                                                     VkDescriptorUpdateTemplate descriptorUpdateTemplate,
                                                     const void* pData) override;

void PostCallRecordUpdateDescriptorSetWithTemplateKHR(VkDevice device, VkDescriptorSet descriptorSet,
                                                      VkDescriptorUpdateTemplate descriptorUpdateTemplate, const void* pData,
                                                      const RecordObject& record_obj) override;

void PreCallRecordCreateRenderPass2KHR(VkDevice device, const VkRenderPassCreateInfo2* pCreateInfo,
                                       const VkAllocationCallbacks* pAllocator, VkRenderPass* pRenderPass) override;

void PostCallRecordCreateRenderPass2KHR(VkDevice device, const VkRenderPassCreateInfo2* pCreateInfo,
                                        const VkAllocationCallbacks* pAllocator, VkRenderPass* pRenderPass,
                                        const RecordObject& record_obj) override;

void PreCallRecordCmdBeginRenderPass2KHR(VkCommandBuffer commandBuffer, const VkRenderPassBeginInfo* pRenderPassBegin,
                                         const VkSubpassBeginInfo* pSubpassBeginInfo) override;

void PostCallRecordCmdBeginRenderPass2KHR(VkCommandBuffer commandBuffer, const VkRenderPassBeginInfo* pRenderPassBegin,
                                          const VkSubpassBeginInfo* pSubpassBeginInfo, const RecordObject& record_obj) override;

void PreCallRecordCmdNextSubpass2KHR(VkCommandBuffer commandBuffer, const VkSubpassBeginInfo* pSubpassBeginInfo,
                                     const VkSubpassEndInfo* pSubpassEndInfo) override;

void PostCallRecordCmdNextSubpass2KHR(VkCommandBuffer commandBuffer, const VkSubpassBeginInfo* pSubpassBeginInfo,
                                      const VkSubpassEndInfo* pSubpassEndInfo, const RecordObject& record_obj) override;

void PreCallRecordCmdEndRenderPass2KHR(VkCommandBuffer commandBuffer, const VkSubpassEndInfo* pSubpassEndInfo) override;

void PostCallRecordCmdEndRenderPass2KHR(VkCommandBuffer commandBuffer, const VkSubpassEndInfo* pSubpassEndInfo,
                                        const RecordObject& record_obj) override;

void PreCallRecordGetSwapchainStatusKHR(VkDevice device, VkSwapchainKHR swapchain) override;

void PostCallRecordGetSwapchainStatusKHR(VkDevice device, VkSwapchainKHR swapchain, const RecordObject& record_obj) override;

#ifdef VK_USE_PLATFORM_WIN32_KHR
void PreCallRecordImportFenceWin32HandleKHR(VkDevice device,
                                            const VkImportFenceWin32HandleInfoKHR* pImportFenceWin32HandleInfo) override;

void PostCallRecordImportFenceWin32HandleKHR(VkDevice device, const VkImportFenceWin32HandleInfoKHR* pImportFenceWin32HandleInfo,
                                             const RecordObject& record_obj) override;

#endif  // VK_USE_PLATFORM_WIN32_KHR
#ifdef VK_USE_PLATFORM_WIN32_KHR
void PreCallRecordGetFenceWin32HandleKHR(VkDevice device, const VkFenceGetWin32HandleInfoKHR* pGetWin32HandleInfo,
                                         HANDLE* pHandle) override;

void PostCallRecordGetFenceWin32HandleKHR(VkDevice device, const VkFenceGetWin32HandleInfoKHR* pGetWin32HandleInfo, HANDLE* pHandle,
                                          const RecordObject& record_obj) override;

#endif  // VK_USE_PLATFORM_WIN32_KHR
void PreCallRecordImportFenceFdKHR(VkDevice device, const VkImportFenceFdInfoKHR* pImportFenceFdInfo) override;

void PostCallRecordImportFenceFdKHR(VkDevice device, const VkImportFenceFdInfoKHR* pImportFenceFdInfo,
                                    const RecordObject& record_obj) override;

void PreCallRecordGetFenceFdKHR(VkDevice device, const VkFenceGetFdInfoKHR* pGetFdInfo, int* pFd) override;

void PostCallRecordGetFenceFdKHR(VkDevice device, const VkFenceGetFdInfoKHR* pGetFdInfo, int* pFd,
                                 const RecordObject& record_obj) override;

void PreCallRecordAcquireProfilingLockKHR(VkDevice device, const VkAcquireProfilingLockInfoKHR* pInfo) override;

void PostCallRecordAcquireProfilingLockKHR(VkDevice device, const VkAcquireProfilingLockInfoKHR* pInfo,
                                           const RecordObject& record_obj) override;

void PreCallRecordReleaseProfilingLockKHR(VkDevice device) override;

void PostCallRecordReleaseProfilingLockKHR(VkDevice device, const RecordObject& record_obj) override;

void PreCallRecordGetDisplayModeProperties2KHR(VkPhysicalDevice physicalDevice, VkDisplayKHR display, uint32_t* pPropertyCount,
                                               VkDisplayModeProperties2KHR* pProperties) override;

void PostCallRecordGetDisplayModeProperties2KHR(VkPhysicalDevice physicalDevice, VkDisplayKHR display, uint32_t* pPropertyCount,
                                                VkDisplayModeProperties2KHR* pProperties, const RecordObject& record_obj) override;

void PreCallRecordGetImageMemoryRequirements2KHR(VkDevice device, const VkImageMemoryRequirementsInfo2* pInfo,
                                                 VkMemoryRequirements2* pMemoryRequirements) override;

void PostCallRecordGetImageMemoryRequirements2KHR(VkDevice device, const VkImageMemoryRequirementsInfo2* pInfo,
                                                  VkMemoryRequirements2* pMemoryRequirements,
                                                  const RecordObject& record_obj) override;

void PreCallRecordGetBufferMemoryRequirements2KHR(VkDevice device, const VkBufferMemoryRequirementsInfo2* pInfo,
                                                  VkMemoryRequirements2* pMemoryRequirements) override;

void PostCallRecordGetBufferMemoryRequirements2KHR(VkDevice device, const VkBufferMemoryRequirementsInfo2* pInfo,
                                                   VkMemoryRequirements2* pMemoryRequirements,
                                                   const RecordObject& record_obj) override;

void PreCallRecordGetImageSparseMemoryRequirements2KHR(VkDevice device, const VkImageSparseMemoryRequirementsInfo2* pInfo,
                                                       uint32_t* pSparseMemoryRequirementCount,
                                                       VkSparseImageMemoryRequirements2* pSparseMemoryRequirements) override;

void PostCallRecordGetImageSparseMemoryRequirements2KHR(VkDevice device, const VkImageSparseMemoryRequirementsInfo2* pInfo,
                                                        uint32_t* pSparseMemoryRequirementCount,
                                                        VkSparseImageMemoryRequirements2* pSparseMemoryRequirements,
                                                        const RecordObject& record_obj) override;

void PreCallRecordCreateSamplerYcbcrConversionKHR(VkDevice device, const VkSamplerYcbcrConversionCreateInfo* pCreateInfo,
                                                  const VkAllocationCallbacks* pAllocator,
                                                  VkSamplerYcbcrConversion* pYcbcrConversion) override;

void PostCallRecordCreateSamplerYcbcrConversionKHR(VkDevice device, const VkSamplerYcbcrConversionCreateInfo* pCreateInfo,
                                                   const VkAllocationCallbacks* pAllocator,
                                                   VkSamplerYcbcrConversion* pYcbcrConversion,
                                                   const RecordObject& record_obj) override;

void PreCallRecordDestroySamplerYcbcrConversionKHR(VkDevice device, VkSamplerYcbcrConversion ycbcrConversion,
                                                   const VkAllocationCallbacks* pAllocator) override;

void PostCallRecordDestroySamplerYcbcrConversionKHR(VkDevice device, VkSamplerYcbcrConversion ycbcrConversion,
                                                    const VkAllocationCallbacks* pAllocator,
                                                    const RecordObject& record_obj) override;

void PreCallRecordBindBufferMemory2KHR(VkDevice device, uint32_t bindInfoCount, const VkBindBufferMemoryInfo* pBindInfos) override;

void PostCallRecordBindBufferMemory2KHR(VkDevice device, uint32_t bindInfoCount, const VkBindBufferMemoryInfo* pBindInfos,
                                        const RecordObject& record_obj) override;

void PreCallRecordBindImageMemory2KHR(VkDevice device, uint32_t bindInfoCount, const VkBindImageMemoryInfo* pBindInfos) override;

void PostCallRecordBindImageMemory2KHR(VkDevice device, uint32_t bindInfoCount, const VkBindImageMemoryInfo* pBindInfos,
                                       const RecordObject& record_obj) override;

void PreCallRecordGetDescriptorSetLayoutSupportKHR(VkDevice device, const VkDescriptorSetLayoutCreateInfo* pCreateInfo,
                                                   VkDescriptorSetLayoutSupport* pSupport) override;

void PostCallRecordGetDescriptorSetLayoutSupportKHR(VkDevice device, const VkDescriptorSetLayoutCreateInfo* pCreateInfo,
                                                    VkDescriptorSetLayoutSupport* pSupport,
                                                    const RecordObject& record_obj) override;

void PreCallRecordCmdDrawIndirectCountKHR(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset, VkBuffer countBuffer,
                                          VkDeviceSize countBufferOffset, uint32_t maxDrawCount, uint32_t stride) override;

void PostCallRecordCmdDrawIndirectCountKHR(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                           VkBuffer countBuffer, VkDeviceSize countBufferOffset, uint32_t maxDrawCount,
                                           uint32_t stride, const RecordObject& record_obj) override;

void PreCallRecordCmdDrawIndexedIndirectCountKHR(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                 VkBuffer countBuffer, VkDeviceSize countBufferOffset, uint32_t maxDrawCount,
                                                 uint32_t stride) override;

void PostCallRecordCmdDrawIndexedIndirectCountKHR(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                  VkBuffer countBuffer, VkDeviceSize countBufferOffset, uint32_t maxDrawCount,
                                                  uint32_t stride, const RecordObject& record_obj) override;

void PreCallRecordGetSemaphoreCounterValueKHR(VkDevice device, VkSemaphore semaphore, uint64_t* pValue) override;

void PostCallRecordGetSemaphoreCounterValueKHR(VkDevice device, VkSemaphore semaphore, uint64_t* pValue,
                                               const RecordObject& record_obj) override;

void PreCallRecordWaitSemaphoresKHR(VkDevice device, const VkSemaphoreWaitInfo* pWaitInfo, uint64_t timeout) override;

void PostCallRecordWaitSemaphoresKHR(VkDevice device, const VkSemaphoreWaitInfo* pWaitInfo, uint64_t timeout,
                                     const RecordObject& record_obj) override;

void PreCallRecordSignalSemaphoreKHR(VkDevice device, const VkSemaphoreSignalInfo* pSignalInfo) override;

void PostCallRecordSignalSemaphoreKHR(VkDevice device, const VkSemaphoreSignalInfo* pSignalInfo,
                                      const RecordObject& record_obj) override;

void PreCallRecordCmdSetFragmentShadingRateKHR(VkCommandBuffer commandBuffer, const VkExtent2D* pFragmentSize,
                                               const VkFragmentShadingRateCombinerOpKHR combinerOps[2]) override;

void PostCallRecordCmdSetFragmentShadingRateKHR(VkCommandBuffer commandBuffer, const VkExtent2D* pFragmentSize,
                                                const VkFragmentShadingRateCombinerOpKHR combinerOps[2],
                                                const RecordObject& record_obj) override;

void PreCallRecordWaitForPresentKHR(VkDevice device, VkSwapchainKHR swapchain, uint64_t presentId, uint64_t timeout) override;

void PostCallRecordWaitForPresentKHR(VkDevice device, VkSwapchainKHR swapchain, uint64_t presentId, uint64_t timeout,
                                     const RecordObject& record_obj) override;

void PreCallRecordGetBufferDeviceAddressKHR(VkDevice device, const VkBufferDeviceAddressInfo* pInfo) override;

void PostCallRecordGetBufferDeviceAddressKHR(VkDevice device, const VkBufferDeviceAddressInfo* pInfo,
                                             const RecordObject& record_obj) override;

void PreCallRecordGetBufferOpaqueCaptureAddressKHR(VkDevice device, const VkBufferDeviceAddressInfo* pInfo) override;

void PostCallRecordGetBufferOpaqueCaptureAddressKHR(VkDevice device, const VkBufferDeviceAddressInfo* pInfo,
                                                    const RecordObject& record_obj) override;

void PreCallRecordGetDeviceMemoryOpaqueCaptureAddressKHR(VkDevice device,
                                                         const VkDeviceMemoryOpaqueCaptureAddressInfo* pInfo) override;

void PostCallRecordGetDeviceMemoryOpaqueCaptureAddressKHR(VkDevice device, const VkDeviceMemoryOpaqueCaptureAddressInfo* pInfo,
                                                          const RecordObject& record_obj) override;

void PreCallRecordCreateDeferredOperationKHR(VkDevice device, const VkAllocationCallbacks* pAllocator,
                                             VkDeferredOperationKHR* pDeferredOperation) override;

void PostCallRecordCreateDeferredOperationKHR(VkDevice device, const VkAllocationCallbacks* pAllocator,
                                              VkDeferredOperationKHR* pDeferredOperation, const RecordObject& record_obj) override;

void PreCallRecordDestroyDeferredOperationKHR(VkDevice device, VkDeferredOperationKHR operation,
                                              const VkAllocationCallbacks* pAllocator) override;

void PostCallRecordDestroyDeferredOperationKHR(VkDevice device, VkDeferredOperationKHR operation,
                                               const VkAllocationCallbacks* pAllocator, const RecordObject& record_obj) override;

void PreCallRecordGetDeferredOperationMaxConcurrencyKHR(VkDevice device, VkDeferredOperationKHR operation) override;

void PostCallRecordGetDeferredOperationMaxConcurrencyKHR(VkDevice device, VkDeferredOperationKHR operation,
                                                         const RecordObject& record_obj) override;

void PreCallRecordGetDeferredOperationResultKHR(VkDevice device, VkDeferredOperationKHR operation) override;

void PostCallRecordGetDeferredOperationResultKHR(VkDevice device, VkDeferredOperationKHR operation,
                                                 const RecordObject& record_obj) override;

void PreCallRecordDeferredOperationJoinKHR(VkDevice device, VkDeferredOperationKHR operation) override;

void PostCallRecordDeferredOperationJoinKHR(VkDevice device, VkDeferredOperationKHR operation,
                                            const RecordObject& record_obj) override;

void PreCallRecordGetPipelineExecutablePropertiesKHR(VkDevice device, const VkPipelineInfoKHR* pPipelineInfo,
                                                     uint32_t* pExecutableCount,
                                                     VkPipelineExecutablePropertiesKHR* pProperties) override;

void PostCallRecordGetPipelineExecutablePropertiesKHR(VkDevice device, const VkPipelineInfoKHR* pPipelineInfo,
                                                      uint32_t* pExecutableCount, VkPipelineExecutablePropertiesKHR* pProperties,
                                                      const RecordObject& record_obj) override;

void PreCallRecordGetPipelineExecutableStatisticsKHR(VkDevice device, const VkPipelineExecutableInfoKHR* pExecutableInfo,
                                                     uint32_t* pStatisticCount,
                                                     VkPipelineExecutableStatisticKHR* pStatistics) override;

void PostCallRecordGetPipelineExecutableStatisticsKHR(VkDevice device, const VkPipelineExecutableInfoKHR* pExecutableInfo,
                                                      uint32_t* pStatisticCount, VkPipelineExecutableStatisticKHR* pStatistics,
                                                      const RecordObject& record_obj) override;

void PreCallRecordGetPipelineExecutableInternalRepresentationsKHR(
    VkDevice device, const VkPipelineExecutableInfoKHR* pExecutableInfo, uint32_t* pInternalRepresentationCount,
    VkPipelineExecutableInternalRepresentationKHR* pInternalRepresentations) override;

void PostCallRecordGetPipelineExecutableInternalRepresentationsKHR(
    VkDevice device, const VkPipelineExecutableInfoKHR* pExecutableInfo, uint32_t* pInternalRepresentationCount,
    VkPipelineExecutableInternalRepresentationKHR* pInternalRepresentations, const RecordObject& record_obj) override;

void PreCallRecordMapMemory2KHR(VkDevice device, const VkMemoryMapInfoKHR* pMemoryMapInfo, void** ppData) override;

void PostCallRecordMapMemory2KHR(VkDevice device, const VkMemoryMapInfoKHR* pMemoryMapInfo, void** ppData,
                                 const RecordObject& record_obj) override;

void PreCallRecordUnmapMemory2KHR(VkDevice device, const VkMemoryUnmapInfoKHR* pMemoryUnmapInfo) override;

void PostCallRecordUnmapMemory2KHR(VkDevice device, const VkMemoryUnmapInfoKHR* pMemoryUnmapInfo,
                                   const RecordObject& record_obj) override;

#ifdef VK_ENABLE_BETA_EXTENSIONS
void PreCallRecordGetEncodedVideoSessionParametersKHR(VkDevice device,
                                                      const VkVideoEncodeSessionParametersGetInfoKHR* pVideoSessionParametersInfo,
                                                      VkVideoEncodeSessionParametersFeedbackInfoKHR* pFeedbackInfo,
                                                      size_t* pDataSize, void* pData) override;

void PostCallRecordGetEncodedVideoSessionParametersKHR(VkDevice device,
                                                       const VkVideoEncodeSessionParametersGetInfoKHR* pVideoSessionParametersInfo,
                                                       VkVideoEncodeSessionParametersFeedbackInfoKHR* pFeedbackInfo,
                                                       size_t* pDataSize, void* pData, const RecordObject& record_obj) override;

#endif  // VK_ENABLE_BETA_EXTENSIONS
#ifdef VK_ENABLE_BETA_EXTENSIONS
void PreCallRecordCmdEncodeVideoKHR(VkCommandBuffer commandBuffer, const VkVideoEncodeInfoKHR* pEncodeInfo) override;

void PostCallRecordCmdEncodeVideoKHR(VkCommandBuffer commandBuffer, const VkVideoEncodeInfoKHR* pEncodeInfo,
                                     const RecordObject& record_obj) override;

#endif  // VK_ENABLE_BETA_EXTENSIONS
void PreCallRecordCmdSetEvent2KHR(VkCommandBuffer commandBuffer, VkEvent event, const VkDependencyInfo* pDependencyInfo) override;

void PostCallRecordCmdSetEvent2KHR(VkCommandBuffer commandBuffer, VkEvent event, const VkDependencyInfo* pDependencyInfo,
                                   const RecordObject& record_obj) override;

void PreCallRecordCmdResetEvent2KHR(VkCommandBuffer commandBuffer, VkEvent event, VkPipelineStageFlags2 stageMask) override;

void PostCallRecordCmdResetEvent2KHR(VkCommandBuffer commandBuffer, VkEvent event, VkPipelineStageFlags2 stageMask,
                                     const RecordObject& record_obj) override;

void PreCallRecordCmdWaitEvents2KHR(VkCommandBuffer commandBuffer, uint32_t eventCount, const VkEvent* pEvents,
                                    const VkDependencyInfo* pDependencyInfos) override;

void PostCallRecordCmdWaitEvents2KHR(VkCommandBuffer commandBuffer, uint32_t eventCount, const VkEvent* pEvents,
                                     const VkDependencyInfo* pDependencyInfos, const RecordObject& record_obj) override;

void PreCallRecordCmdPipelineBarrier2KHR(VkCommandBuffer commandBuffer, const VkDependencyInfo* pDependencyInfo) override;

void PostCallRecordCmdPipelineBarrier2KHR(VkCommandBuffer commandBuffer, const VkDependencyInfo* pDependencyInfo,
                                          const RecordObject& record_obj) override;

void PreCallRecordCmdWriteTimestamp2KHR(VkCommandBuffer commandBuffer, VkPipelineStageFlags2 stage, VkQueryPool queryPool,
                                        uint32_t query) override;

void PostCallRecordCmdWriteTimestamp2KHR(VkCommandBuffer commandBuffer, VkPipelineStageFlags2 stage, VkQueryPool queryPool,
                                         uint32_t query, const RecordObject& record_obj) override;

void PreCallRecordQueueSubmit2KHR(VkQueue queue, uint32_t submitCount, const VkSubmitInfo2* pSubmits, VkFence fence) override;

void PostCallRecordQueueSubmit2KHR(VkQueue queue, uint32_t submitCount, const VkSubmitInfo2* pSubmits, VkFence fence,
                                   const RecordObject& record_obj) override;

void PreCallRecordCmdWriteBufferMarker2AMD(VkCommandBuffer commandBuffer, VkPipelineStageFlags2 stage, VkBuffer dstBuffer,
                                           VkDeviceSize dstOffset, uint32_t marker) override;

void PostCallRecordCmdWriteBufferMarker2AMD(VkCommandBuffer commandBuffer, VkPipelineStageFlags2 stage, VkBuffer dstBuffer,
                                            VkDeviceSize dstOffset, uint32_t marker, const RecordObject& record_obj) override;

void PreCallRecordGetQueueCheckpointData2NV(VkQueue queue, uint32_t* pCheckpointDataCount,
                                            VkCheckpointData2NV* pCheckpointData) override;

void PostCallRecordGetQueueCheckpointData2NV(VkQueue queue, uint32_t* pCheckpointDataCount, VkCheckpointData2NV* pCheckpointData,
                                             const RecordObject& record_obj) override;

void PreCallRecordCmdCopyBuffer2KHR(VkCommandBuffer commandBuffer, const VkCopyBufferInfo2* pCopyBufferInfo) override;

void PostCallRecordCmdCopyBuffer2KHR(VkCommandBuffer commandBuffer, const VkCopyBufferInfo2* pCopyBufferInfo,
                                     const RecordObject& record_obj) override;

void PreCallRecordCmdCopyImage2KHR(VkCommandBuffer commandBuffer, const VkCopyImageInfo2* pCopyImageInfo) override;

void PostCallRecordCmdCopyImage2KHR(VkCommandBuffer commandBuffer, const VkCopyImageInfo2* pCopyImageInfo,
                                    const RecordObject& record_obj) override;

void PreCallRecordCmdCopyBufferToImage2KHR(VkCommandBuffer commandBuffer,
                                           const VkCopyBufferToImageInfo2* pCopyBufferToImageInfo) override;

void PostCallRecordCmdCopyBufferToImage2KHR(VkCommandBuffer commandBuffer, const VkCopyBufferToImageInfo2* pCopyBufferToImageInfo,
                                            const RecordObject& record_obj) override;

void PreCallRecordCmdCopyImageToBuffer2KHR(VkCommandBuffer commandBuffer,
                                           const VkCopyImageToBufferInfo2* pCopyImageToBufferInfo) override;

void PostCallRecordCmdCopyImageToBuffer2KHR(VkCommandBuffer commandBuffer, const VkCopyImageToBufferInfo2* pCopyImageToBufferInfo,
                                            const RecordObject& record_obj) override;

void PreCallRecordCmdBlitImage2KHR(VkCommandBuffer commandBuffer, const VkBlitImageInfo2* pBlitImageInfo) override;

void PostCallRecordCmdBlitImage2KHR(VkCommandBuffer commandBuffer, const VkBlitImageInfo2* pBlitImageInfo,
                                    const RecordObject& record_obj) override;

void PreCallRecordCmdResolveImage2KHR(VkCommandBuffer commandBuffer, const VkResolveImageInfo2* pResolveImageInfo) override;

void PostCallRecordCmdResolveImage2KHR(VkCommandBuffer commandBuffer, const VkResolveImageInfo2* pResolveImageInfo,
                                       const RecordObject& record_obj) override;

void PreCallRecordCmdTraceRaysIndirect2KHR(VkCommandBuffer commandBuffer, VkDeviceAddress indirectDeviceAddress) override;

void PostCallRecordCmdTraceRaysIndirect2KHR(VkCommandBuffer commandBuffer, VkDeviceAddress indirectDeviceAddress,
                                            const RecordObject& record_obj) override;

void PreCallRecordGetDeviceBufferMemoryRequirementsKHR(VkDevice device, const VkDeviceBufferMemoryRequirements* pInfo,
                                                       VkMemoryRequirements2* pMemoryRequirements) override;

void PostCallRecordGetDeviceBufferMemoryRequirementsKHR(VkDevice device, const VkDeviceBufferMemoryRequirements* pInfo,
                                                        VkMemoryRequirements2* pMemoryRequirements,
                                                        const RecordObject& record_obj) override;

void PreCallRecordGetDeviceImageMemoryRequirementsKHR(VkDevice device, const VkDeviceImageMemoryRequirements* pInfo,
                                                      VkMemoryRequirements2* pMemoryRequirements) override;

void PostCallRecordGetDeviceImageMemoryRequirementsKHR(VkDevice device, const VkDeviceImageMemoryRequirements* pInfo,
                                                       VkMemoryRequirements2* pMemoryRequirements,
                                                       const RecordObject& record_obj) override;

void PreCallRecordGetDeviceImageSparseMemoryRequirementsKHR(VkDevice device, const VkDeviceImageMemoryRequirements* pInfo,
                                                            uint32_t* pSparseMemoryRequirementCount,
                                                            VkSparseImageMemoryRequirements2* pSparseMemoryRequirements) override;

void PostCallRecordGetDeviceImageSparseMemoryRequirementsKHR(VkDevice device, const VkDeviceImageMemoryRequirements* pInfo,
                                                             uint32_t* pSparseMemoryRequirementCount,
                                                             VkSparseImageMemoryRequirements2* pSparseMemoryRequirements,
                                                             const RecordObject& record_obj) override;

void PreCallRecordCmdBindIndexBuffer2KHR(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset, VkDeviceSize size,
                                         VkIndexType indexType) override;

void PostCallRecordCmdBindIndexBuffer2KHR(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset, VkDeviceSize size,
                                          VkIndexType indexType, const RecordObject& record_obj) override;

void PreCallRecordGetRenderingAreaGranularityKHR(VkDevice device, const VkRenderingAreaInfoKHR* pRenderingAreaInfo,
                                                 VkExtent2D* pGranularity) override;

void PostCallRecordGetRenderingAreaGranularityKHR(VkDevice device, const VkRenderingAreaInfoKHR* pRenderingAreaInfo,
                                                  VkExtent2D* pGranularity, const RecordObject& record_obj) override;

void PreCallRecordGetDeviceImageSubresourceLayoutKHR(VkDevice device, const VkDeviceImageSubresourceInfoKHR* pInfo,
                                                     VkSubresourceLayout2KHR* pLayout) override;

void PostCallRecordGetDeviceImageSubresourceLayoutKHR(VkDevice device, const VkDeviceImageSubresourceInfoKHR* pInfo,
                                                      VkSubresourceLayout2KHR* pLayout, const RecordObject& record_obj) override;

void PreCallRecordGetImageSubresourceLayout2KHR(VkDevice device, VkImage image, const VkImageSubresource2KHR* pSubresource,
                                                VkSubresourceLayout2KHR* pLayout) override;

void PostCallRecordGetImageSubresourceLayout2KHR(VkDevice device, VkImage image, const VkImageSubresource2KHR* pSubresource,
                                                 VkSubresourceLayout2KHR* pLayout, const RecordObject& record_obj) override;

void PreCallRecordCreateDebugReportCallbackEXT(VkInstance instance, const VkDebugReportCallbackCreateInfoEXT* pCreateInfo,
                                               const VkAllocationCallbacks* pAllocator,
                                               VkDebugReportCallbackEXT* pCallback) override;

void PostCallRecordCreateDebugReportCallbackEXT(VkInstance instance, const VkDebugReportCallbackCreateInfoEXT* pCreateInfo,
                                                const VkAllocationCallbacks* pAllocator, VkDebugReportCallbackEXT* pCallback,
                                                const RecordObject& record_obj) override;

void PreCallRecordDestroyDebugReportCallbackEXT(VkInstance instance, VkDebugReportCallbackEXT callback,
                                                const VkAllocationCallbacks* pAllocator) override;

void PostCallRecordDestroyDebugReportCallbackEXT(VkInstance instance, VkDebugReportCallbackEXT callback,
                                                 const VkAllocationCallbacks* pAllocator, const RecordObject& record_obj) override;

void PreCallRecordDebugReportMessageEXT(VkInstance instance, VkDebugReportFlagsEXT flags, VkDebugReportObjectTypeEXT objectType,
                                        uint64_t object, size_t location, int32_t messageCode, const char* pLayerPrefix,
                                        const char* pMessage) override;

void PostCallRecordDebugReportMessageEXT(VkInstance instance, VkDebugReportFlagsEXT flags, VkDebugReportObjectTypeEXT objectType,
                                         uint64_t object, size_t location, int32_t messageCode, const char* pLayerPrefix,
                                         const char* pMessage, const RecordObject& record_obj) override;

void PreCallRecordCmdBindTransformFeedbackBuffersEXT(VkCommandBuffer commandBuffer, uint32_t firstBinding, uint32_t bindingCount,
                                                     const VkBuffer* pBuffers, const VkDeviceSize* pOffsets,
                                                     const VkDeviceSize* pSizes) override;

void PostCallRecordCmdBindTransformFeedbackBuffersEXT(VkCommandBuffer commandBuffer, uint32_t firstBinding, uint32_t bindingCount,
                                                      const VkBuffer* pBuffers, const VkDeviceSize* pOffsets,
                                                      const VkDeviceSize* pSizes, const RecordObject& record_obj) override;

void PreCallRecordCmdBeginTransformFeedbackEXT(VkCommandBuffer commandBuffer, uint32_t firstCounterBuffer,
                                               uint32_t counterBufferCount, const VkBuffer* pCounterBuffers,
                                               const VkDeviceSize* pCounterBufferOffsets) override;

void PostCallRecordCmdBeginTransformFeedbackEXT(VkCommandBuffer commandBuffer, uint32_t firstCounterBuffer,
                                                uint32_t counterBufferCount, const VkBuffer* pCounterBuffers,
                                                const VkDeviceSize* pCounterBufferOffsets, const RecordObject& record_obj) override;

void PreCallRecordCmdEndTransformFeedbackEXT(VkCommandBuffer commandBuffer, uint32_t firstCounterBuffer,
                                             uint32_t counterBufferCount, const VkBuffer* pCounterBuffers,
                                             const VkDeviceSize* pCounterBufferOffsets) override;

void PostCallRecordCmdEndTransformFeedbackEXT(VkCommandBuffer commandBuffer, uint32_t firstCounterBuffer,
                                              uint32_t counterBufferCount, const VkBuffer* pCounterBuffers,
                                              const VkDeviceSize* pCounterBufferOffsets, const RecordObject& record_obj) override;

void PreCallRecordCmdBeginQueryIndexedEXT(VkCommandBuffer commandBuffer, VkQueryPool queryPool, uint32_t query,
                                          VkQueryControlFlags flags, uint32_t index) override;

void PostCallRecordCmdBeginQueryIndexedEXT(VkCommandBuffer commandBuffer, VkQueryPool queryPool, uint32_t query,
                                           VkQueryControlFlags flags, uint32_t index, const RecordObject& record_obj) override;

void PreCallRecordCmdEndQueryIndexedEXT(VkCommandBuffer commandBuffer, VkQueryPool queryPool, uint32_t query,
                                        uint32_t index) override;

void PostCallRecordCmdEndQueryIndexedEXT(VkCommandBuffer commandBuffer, VkQueryPool queryPool, uint32_t query, uint32_t index,
                                         const RecordObject& record_obj) override;

void PreCallRecordCmdDrawIndirectByteCountEXT(VkCommandBuffer commandBuffer, uint32_t instanceCount, uint32_t firstInstance,
                                              VkBuffer counterBuffer, VkDeviceSize counterBufferOffset, uint32_t counterOffset,
                                              uint32_t vertexStride) override;

void PostCallRecordCmdDrawIndirectByteCountEXT(VkCommandBuffer commandBuffer, uint32_t instanceCount, uint32_t firstInstance,
                                               VkBuffer counterBuffer, VkDeviceSize counterBufferOffset, uint32_t counterOffset,
                                               uint32_t vertexStride, const RecordObject& record_obj) override;

void PreCallRecordCreateCuModuleNVX(VkDevice device, const VkCuModuleCreateInfoNVX* pCreateInfo,
                                    const VkAllocationCallbacks* pAllocator, VkCuModuleNVX* pModule) override;

void PostCallRecordCreateCuModuleNVX(VkDevice device, const VkCuModuleCreateInfoNVX* pCreateInfo,
                                     const VkAllocationCallbacks* pAllocator, VkCuModuleNVX* pModule,
                                     const RecordObject& record_obj) override;

void PreCallRecordCreateCuFunctionNVX(VkDevice device, const VkCuFunctionCreateInfoNVX* pCreateInfo,
                                      const VkAllocationCallbacks* pAllocator, VkCuFunctionNVX* pFunction) override;

void PostCallRecordCreateCuFunctionNVX(VkDevice device, const VkCuFunctionCreateInfoNVX* pCreateInfo,
                                       const VkAllocationCallbacks* pAllocator, VkCuFunctionNVX* pFunction,
                                       const RecordObject& record_obj) override;

void PreCallRecordDestroyCuModuleNVX(VkDevice device, VkCuModuleNVX module, const VkAllocationCallbacks* pAllocator) override;

void PostCallRecordDestroyCuModuleNVX(VkDevice device, VkCuModuleNVX module, const VkAllocationCallbacks* pAllocator,
                                      const RecordObject& record_obj) override;

void PreCallRecordDestroyCuFunctionNVX(VkDevice device, VkCuFunctionNVX function, const VkAllocationCallbacks* pAllocator) override;

void PostCallRecordDestroyCuFunctionNVX(VkDevice device, VkCuFunctionNVX function, const VkAllocationCallbacks* pAllocator,
                                        const RecordObject& record_obj) override;

void PreCallRecordCmdCuLaunchKernelNVX(VkCommandBuffer commandBuffer, const VkCuLaunchInfoNVX* pLaunchInfo) override;

void PostCallRecordCmdCuLaunchKernelNVX(VkCommandBuffer commandBuffer, const VkCuLaunchInfoNVX* pLaunchInfo,
                                        const RecordObject& record_obj) override;

void PreCallRecordGetImageViewHandleNVX(VkDevice device, const VkImageViewHandleInfoNVX* pInfo) override;

void PostCallRecordGetImageViewHandleNVX(VkDevice device, const VkImageViewHandleInfoNVX* pInfo,
                                         const RecordObject& record_obj) override;

void PreCallRecordGetImageViewAddressNVX(VkDevice device, VkImageView imageView,
                                         VkImageViewAddressPropertiesNVX* pProperties) override;

void PostCallRecordGetImageViewAddressNVX(VkDevice device, VkImageView imageView, VkImageViewAddressPropertiesNVX* pProperties,
                                          const RecordObject& record_obj) override;

void PreCallRecordCmdDrawIndirectCountAMD(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset, VkBuffer countBuffer,
                                          VkDeviceSize countBufferOffset, uint32_t maxDrawCount, uint32_t stride) override;

void PostCallRecordCmdDrawIndirectCountAMD(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                           VkBuffer countBuffer, VkDeviceSize countBufferOffset, uint32_t maxDrawCount,
                                           uint32_t stride, const RecordObject& record_obj) override;

void PreCallRecordCmdDrawIndexedIndirectCountAMD(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                 VkBuffer countBuffer, VkDeviceSize countBufferOffset, uint32_t maxDrawCount,
                                                 uint32_t stride) override;

void PostCallRecordCmdDrawIndexedIndirectCountAMD(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                  VkBuffer countBuffer, VkDeviceSize countBufferOffset, uint32_t maxDrawCount,
                                                  uint32_t stride, const RecordObject& record_obj) override;

void PreCallRecordGetShaderInfoAMD(VkDevice device, VkPipeline pipeline, VkShaderStageFlagBits shaderStage,
                                   VkShaderInfoTypeAMD infoType, size_t* pInfoSize, void* pInfo) override;

void PostCallRecordGetShaderInfoAMD(VkDevice device, VkPipeline pipeline, VkShaderStageFlagBits shaderStage,
                                    VkShaderInfoTypeAMD infoType, size_t* pInfoSize, void* pInfo,
                                    const RecordObject& record_obj) override;

#ifdef VK_USE_PLATFORM_GGP
void PreCallRecordCreateStreamDescriptorSurfaceGGP(VkInstance instance, const VkStreamDescriptorSurfaceCreateInfoGGP* pCreateInfo,
                                                   const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface) override;

void PostCallRecordCreateStreamDescriptorSurfaceGGP(VkInstance instance, const VkStreamDescriptorSurfaceCreateInfoGGP* pCreateInfo,
                                                    const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface,
                                                    const RecordObject& record_obj) override;

#endif  // VK_USE_PLATFORM_GGP
#ifdef VK_USE_PLATFORM_WIN32_KHR
void PreCallRecordGetMemoryWin32HandleNV(VkDevice device, VkDeviceMemory memory, VkExternalMemoryHandleTypeFlagsNV handleType,
                                         HANDLE* pHandle) override;

void PostCallRecordGetMemoryWin32HandleNV(VkDevice device, VkDeviceMemory memory, VkExternalMemoryHandleTypeFlagsNV handleType,
                                          HANDLE* pHandle, const RecordObject& record_obj) override;

#endif  // VK_USE_PLATFORM_WIN32_KHR
#ifdef VK_USE_PLATFORM_VI_NN
void PreCallRecordCreateViSurfaceNN(VkInstance instance, const VkViSurfaceCreateInfoNN* pCreateInfo,
                                    const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface) override;

void PostCallRecordCreateViSurfaceNN(VkInstance instance, const VkViSurfaceCreateInfoNN* pCreateInfo,
                                     const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface,
                                     const RecordObject& record_obj) override;

#endif  // VK_USE_PLATFORM_VI_NN
void PreCallRecordCmdBeginConditionalRenderingEXT(VkCommandBuffer commandBuffer,
                                                  const VkConditionalRenderingBeginInfoEXT* pConditionalRenderingBegin) override;

void PostCallRecordCmdBeginConditionalRenderingEXT(VkCommandBuffer commandBuffer,
                                                   const VkConditionalRenderingBeginInfoEXT* pConditionalRenderingBegin,
                                                   const RecordObject& record_obj) override;

void PreCallRecordCmdEndConditionalRenderingEXT(VkCommandBuffer commandBuffer) override;

void PostCallRecordCmdEndConditionalRenderingEXT(VkCommandBuffer commandBuffer, const RecordObject& record_obj) override;

void PreCallRecordCmdSetViewportWScalingNV(VkCommandBuffer commandBuffer, uint32_t firstViewport, uint32_t viewportCount,
                                           const VkViewportWScalingNV* pViewportWScalings) override;

void PostCallRecordCmdSetViewportWScalingNV(VkCommandBuffer commandBuffer, uint32_t firstViewport, uint32_t viewportCount,
                                            const VkViewportWScalingNV* pViewportWScalings,
                                            const RecordObject& record_obj) override;

void PreCallRecordReleaseDisplayEXT(VkPhysicalDevice physicalDevice, VkDisplayKHR display) override;

void PostCallRecordReleaseDisplayEXT(VkPhysicalDevice physicalDevice, VkDisplayKHR display,
                                     const RecordObject& record_obj) override;

#ifdef VK_USE_PLATFORM_XLIB_XRANDR_EXT
void PreCallRecordAcquireXlibDisplayEXT(VkPhysicalDevice physicalDevice, Display* dpy, VkDisplayKHR display) override;

void PostCallRecordAcquireXlibDisplayEXT(VkPhysicalDevice physicalDevice, Display* dpy, VkDisplayKHR display,
                                         const RecordObject& record_obj) override;

#endif  // VK_USE_PLATFORM_XLIB_XRANDR_EXT
void PreCallRecordGetPhysicalDeviceSurfaceCapabilities2EXT(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface,
                                                           VkSurfaceCapabilities2EXT* pSurfaceCapabilities) override;

void PostCallRecordGetPhysicalDeviceSurfaceCapabilities2EXT(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface,
                                                            VkSurfaceCapabilities2EXT* pSurfaceCapabilities,
                                                            const RecordObject& record_obj) override;

void PreCallRecordDisplayPowerControlEXT(VkDevice device, VkDisplayKHR display,
                                         const VkDisplayPowerInfoEXT* pDisplayPowerInfo) override;

void PostCallRecordDisplayPowerControlEXT(VkDevice device, VkDisplayKHR display, const VkDisplayPowerInfoEXT* pDisplayPowerInfo,
                                          const RecordObject& record_obj) override;

void PreCallRecordRegisterDeviceEventEXT(VkDevice device, const VkDeviceEventInfoEXT* pDeviceEventInfo,
                                         const VkAllocationCallbacks* pAllocator, VkFence* pFence) override;

void PostCallRecordRegisterDeviceEventEXT(VkDevice device, const VkDeviceEventInfoEXT* pDeviceEventInfo,
                                          const VkAllocationCallbacks* pAllocator, VkFence* pFence,
                                          const RecordObject& record_obj) override;

void PreCallRecordRegisterDisplayEventEXT(VkDevice device, VkDisplayKHR display, const VkDisplayEventInfoEXT* pDisplayEventInfo,
                                          const VkAllocationCallbacks* pAllocator, VkFence* pFence) override;

void PostCallRecordRegisterDisplayEventEXT(VkDevice device, VkDisplayKHR display, const VkDisplayEventInfoEXT* pDisplayEventInfo,
                                           const VkAllocationCallbacks* pAllocator, VkFence* pFence,
                                           const RecordObject& record_obj) override;

void PreCallRecordGetSwapchainCounterEXT(VkDevice device, VkSwapchainKHR swapchain, VkSurfaceCounterFlagBitsEXT counter,
                                         uint64_t* pCounterValue) override;

void PostCallRecordGetSwapchainCounterEXT(VkDevice device, VkSwapchainKHR swapchain, VkSurfaceCounterFlagBitsEXT counter,
                                          uint64_t* pCounterValue, const RecordObject& record_obj) override;

void PreCallRecordGetRefreshCycleDurationGOOGLE(VkDevice device, VkSwapchainKHR swapchain,
                                                VkRefreshCycleDurationGOOGLE* pDisplayTimingProperties) override;

void PostCallRecordGetRefreshCycleDurationGOOGLE(VkDevice device, VkSwapchainKHR swapchain,
                                                 VkRefreshCycleDurationGOOGLE* pDisplayTimingProperties,
                                                 const RecordObject& record_obj) override;

void PreCallRecordGetPastPresentationTimingGOOGLE(VkDevice device, VkSwapchainKHR swapchain, uint32_t* pPresentationTimingCount,
                                                  VkPastPresentationTimingGOOGLE* pPresentationTimings) override;

void PostCallRecordGetPastPresentationTimingGOOGLE(VkDevice device, VkSwapchainKHR swapchain, uint32_t* pPresentationTimingCount,
                                                   VkPastPresentationTimingGOOGLE* pPresentationTimings,
                                                   const RecordObject& record_obj) override;

void PreCallRecordCmdSetDiscardRectangleEXT(VkCommandBuffer commandBuffer, uint32_t firstDiscardRectangle,
                                            uint32_t discardRectangleCount, const VkRect2D* pDiscardRectangles) override;

void PostCallRecordCmdSetDiscardRectangleEXT(VkCommandBuffer commandBuffer, uint32_t firstDiscardRectangle,
                                             uint32_t discardRectangleCount, const VkRect2D* pDiscardRectangles,
                                             const RecordObject& record_obj) override;

void PreCallRecordCmdSetDiscardRectangleEnableEXT(VkCommandBuffer commandBuffer, VkBool32 discardRectangleEnable) override;

void PostCallRecordCmdSetDiscardRectangleEnableEXT(VkCommandBuffer commandBuffer, VkBool32 discardRectangleEnable,
                                                   const RecordObject& record_obj) override;

void PreCallRecordCmdSetDiscardRectangleModeEXT(VkCommandBuffer commandBuffer,
                                                VkDiscardRectangleModeEXT discardRectangleMode) override;

void PostCallRecordCmdSetDiscardRectangleModeEXT(VkCommandBuffer commandBuffer, VkDiscardRectangleModeEXT discardRectangleMode,
                                                 const RecordObject& record_obj) override;

void PreCallRecordSetHdrMetadataEXT(VkDevice device, uint32_t swapchainCount, const VkSwapchainKHR* pSwapchains,
                                    const VkHdrMetadataEXT* pMetadata) override;

void PostCallRecordSetHdrMetadataEXT(VkDevice device, uint32_t swapchainCount, const VkSwapchainKHR* pSwapchains,
                                     const VkHdrMetadataEXT* pMetadata, const RecordObject& record_obj) override;

#ifdef VK_USE_PLATFORM_IOS_MVK
void PreCallRecordCreateIOSSurfaceMVK(VkInstance instance, const VkIOSSurfaceCreateInfoMVK* pCreateInfo,
                                      const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface) override;

void PostCallRecordCreateIOSSurfaceMVK(VkInstance instance, const VkIOSSurfaceCreateInfoMVK* pCreateInfo,
                                       const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface,
                                       const RecordObject& record_obj) override;

#endif  // VK_USE_PLATFORM_IOS_MVK
#ifdef VK_USE_PLATFORM_MACOS_MVK
void PreCallRecordCreateMacOSSurfaceMVK(VkInstance instance, const VkMacOSSurfaceCreateInfoMVK* pCreateInfo,
                                        const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface) override;

void PostCallRecordCreateMacOSSurfaceMVK(VkInstance instance, const VkMacOSSurfaceCreateInfoMVK* pCreateInfo,
                                         const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface,
                                         const RecordObject& record_obj) override;

#endif  // VK_USE_PLATFORM_MACOS_MVK
void PreCallRecordQueueBeginDebugUtilsLabelEXT(VkQueue queue, const VkDebugUtilsLabelEXT* pLabelInfo) override;

void PostCallRecordQueueBeginDebugUtilsLabelEXT(VkQueue queue, const VkDebugUtilsLabelEXT* pLabelInfo,
                                                const RecordObject& record_obj) override;

void PreCallRecordQueueEndDebugUtilsLabelEXT(VkQueue queue) override;

void PostCallRecordQueueEndDebugUtilsLabelEXT(VkQueue queue, const RecordObject& record_obj) override;

void PreCallRecordQueueInsertDebugUtilsLabelEXT(VkQueue queue, const VkDebugUtilsLabelEXT* pLabelInfo) override;

void PostCallRecordQueueInsertDebugUtilsLabelEXT(VkQueue queue, const VkDebugUtilsLabelEXT* pLabelInfo,
                                                 const RecordObject& record_obj) override;

void PreCallRecordCmdBeginDebugUtilsLabelEXT(VkCommandBuffer commandBuffer, const VkDebugUtilsLabelEXT* pLabelInfo) override;

void PostCallRecordCmdBeginDebugUtilsLabelEXT(VkCommandBuffer commandBuffer, const VkDebugUtilsLabelEXT* pLabelInfo,
                                              const RecordObject& record_obj) override;

void PreCallRecordCmdEndDebugUtilsLabelEXT(VkCommandBuffer commandBuffer) override;

void PostCallRecordCmdEndDebugUtilsLabelEXT(VkCommandBuffer commandBuffer, const RecordObject& record_obj) override;

void PreCallRecordCmdInsertDebugUtilsLabelEXT(VkCommandBuffer commandBuffer, const VkDebugUtilsLabelEXT* pLabelInfo) override;

void PostCallRecordCmdInsertDebugUtilsLabelEXT(VkCommandBuffer commandBuffer, const VkDebugUtilsLabelEXT* pLabelInfo,
                                               const RecordObject& record_obj) override;

void PreCallRecordCreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
                                               const VkAllocationCallbacks* pAllocator,
                                               VkDebugUtilsMessengerEXT* pMessenger) override;

void PostCallRecordCreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
                                                const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pMessenger,
                                                const RecordObject& record_obj) override;

void PreCallRecordDestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT messenger,
                                                const VkAllocationCallbacks* pAllocator) override;

void PostCallRecordDestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT messenger,
                                                 const VkAllocationCallbacks* pAllocator, const RecordObject& record_obj) override;

void PreCallRecordSubmitDebugUtilsMessageEXT(VkInstance instance, VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
                                             VkDebugUtilsMessageTypeFlagsEXT messageTypes,
                                             const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData) override;

void PostCallRecordSubmitDebugUtilsMessageEXT(VkInstance instance, VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
                                              VkDebugUtilsMessageTypeFlagsEXT messageTypes,
                                              const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
                                              const RecordObject& record_obj) override;

#ifdef VK_USE_PLATFORM_ANDROID_KHR
void PreCallRecordGetAndroidHardwareBufferPropertiesANDROID(VkDevice device, const struct AHardwareBuffer* buffer,
                                                            VkAndroidHardwareBufferPropertiesANDROID* pProperties) override;

void PostCallRecordGetAndroidHardwareBufferPropertiesANDROID(VkDevice device, const struct AHardwareBuffer* buffer,
                                                             VkAndroidHardwareBufferPropertiesANDROID* pProperties,
                                                             const RecordObject& record_obj) override;

#endif  // VK_USE_PLATFORM_ANDROID_KHR
#ifdef VK_USE_PLATFORM_ANDROID_KHR
void PreCallRecordGetMemoryAndroidHardwareBufferANDROID(VkDevice device, const VkMemoryGetAndroidHardwareBufferInfoANDROID* pInfo,
                                                        struct AHardwareBuffer** pBuffer) override;

void PostCallRecordGetMemoryAndroidHardwareBufferANDROID(VkDevice device, const VkMemoryGetAndroidHardwareBufferInfoANDROID* pInfo,
                                                         struct AHardwareBuffer** pBuffer, const RecordObject& record_obj) override;

#endif  // VK_USE_PLATFORM_ANDROID_KHR
#ifdef VK_ENABLE_BETA_EXTENSIONS
void PreCallRecordCreateExecutionGraphPipelinesAMDX(VkDevice device, VkPipelineCache pipelineCache, uint32_t createInfoCount,
                                                    const VkExecutionGraphPipelineCreateInfoAMDX* pCreateInfos,
                                                    const VkAllocationCallbacks* pAllocator, VkPipeline* pPipelines) override;

void PostCallRecordCreateExecutionGraphPipelinesAMDX(VkDevice device, VkPipelineCache pipelineCache, uint32_t createInfoCount,
                                                     const VkExecutionGraphPipelineCreateInfoAMDX* pCreateInfos,
                                                     const VkAllocationCallbacks* pAllocator, VkPipeline* pPipelines,
                                                     const RecordObject& record_obj) override;

#endif  // VK_ENABLE_BETA_EXTENSIONS
#ifdef VK_ENABLE_BETA_EXTENSIONS
void PreCallRecordGetExecutionGraphPipelineScratchSizeAMDX(VkDevice device, VkPipeline executionGraph,
                                                           VkExecutionGraphPipelineScratchSizeAMDX* pSizeInfo) override;

void PostCallRecordGetExecutionGraphPipelineScratchSizeAMDX(VkDevice device, VkPipeline executionGraph,
                                                            VkExecutionGraphPipelineScratchSizeAMDX* pSizeInfo,
                                                            const RecordObject& record_obj) override;

#endif  // VK_ENABLE_BETA_EXTENSIONS
#ifdef VK_ENABLE_BETA_EXTENSIONS
void PreCallRecordGetExecutionGraphPipelineNodeIndexAMDX(VkDevice device, VkPipeline executionGraph,
                                                         const VkPipelineShaderStageNodeCreateInfoAMDX* pNodeInfo,
                                                         uint32_t* pNodeIndex) override;

void PostCallRecordGetExecutionGraphPipelineNodeIndexAMDX(VkDevice device, VkPipeline executionGraph,
                                                          const VkPipelineShaderStageNodeCreateInfoAMDX* pNodeInfo,
                                                          uint32_t* pNodeIndex, const RecordObject& record_obj) override;

#endif  // VK_ENABLE_BETA_EXTENSIONS
#ifdef VK_ENABLE_BETA_EXTENSIONS
void PreCallRecordCmdInitializeGraphScratchMemoryAMDX(VkCommandBuffer commandBuffer, VkDeviceAddress scratch) override;

void PostCallRecordCmdInitializeGraphScratchMemoryAMDX(VkCommandBuffer commandBuffer, VkDeviceAddress scratch,
                                                       const RecordObject& record_obj) override;

#endif  // VK_ENABLE_BETA_EXTENSIONS
#ifdef VK_ENABLE_BETA_EXTENSIONS
void PreCallRecordCmdDispatchGraphAMDX(VkCommandBuffer commandBuffer, VkDeviceAddress scratch,
                                       const VkDispatchGraphCountInfoAMDX* pCountInfo) override;

void PostCallRecordCmdDispatchGraphAMDX(VkCommandBuffer commandBuffer, VkDeviceAddress scratch,
                                        const VkDispatchGraphCountInfoAMDX* pCountInfo, const RecordObject& record_obj) override;

#endif  // VK_ENABLE_BETA_EXTENSIONS
#ifdef VK_ENABLE_BETA_EXTENSIONS
void PreCallRecordCmdDispatchGraphIndirectAMDX(VkCommandBuffer commandBuffer, VkDeviceAddress scratch,
                                               const VkDispatchGraphCountInfoAMDX* pCountInfo) override;

void PostCallRecordCmdDispatchGraphIndirectAMDX(VkCommandBuffer commandBuffer, VkDeviceAddress scratch,
                                                const VkDispatchGraphCountInfoAMDX* pCountInfo,
                                                const RecordObject& record_obj) override;

#endif  // VK_ENABLE_BETA_EXTENSIONS
#ifdef VK_ENABLE_BETA_EXTENSIONS
void PreCallRecordCmdDispatchGraphIndirectCountAMDX(VkCommandBuffer commandBuffer, VkDeviceAddress scratch,
                                                    VkDeviceAddress countInfo) override;

void PostCallRecordCmdDispatchGraphIndirectCountAMDX(VkCommandBuffer commandBuffer, VkDeviceAddress scratch,
                                                     VkDeviceAddress countInfo, const RecordObject& record_obj) override;

#endif  // VK_ENABLE_BETA_EXTENSIONS
void PreCallRecordCmdSetSampleLocationsEXT(VkCommandBuffer commandBuffer,
                                           const VkSampleLocationsInfoEXT* pSampleLocationsInfo) override;

void PostCallRecordCmdSetSampleLocationsEXT(VkCommandBuffer commandBuffer, const VkSampleLocationsInfoEXT* pSampleLocationsInfo,
                                            const RecordObject& record_obj) override;

void PreCallRecordGetImageDrmFormatModifierPropertiesEXT(VkDevice device, VkImage image,
                                                         VkImageDrmFormatModifierPropertiesEXT* pProperties) override;

void PostCallRecordGetImageDrmFormatModifierPropertiesEXT(VkDevice device, VkImage image,
                                                          VkImageDrmFormatModifierPropertiesEXT* pProperties,
                                                          const RecordObject& record_obj) override;

void PreCallRecordCreateValidationCacheEXT(VkDevice device, const VkValidationCacheCreateInfoEXT* pCreateInfo,
                                           const VkAllocationCallbacks* pAllocator, VkValidationCacheEXT* pValidationCache);

void PostCallRecordCreateValidationCacheEXT(VkDevice device, const VkValidationCacheCreateInfoEXT* pCreateInfo,
                                            const VkAllocationCallbacks* pAllocator, VkValidationCacheEXT* pValidationCache,
                                            const RecordObject& record_obj);

void PreCallRecordDestroyValidationCacheEXT(VkDevice device, VkValidationCacheEXT validationCache,
                                            const VkAllocationCallbacks* pAllocator);

void PostCallRecordDestroyValidationCacheEXT(VkDevice device, VkValidationCacheEXT validationCache,
                                             const VkAllocationCallbacks* pAllocator, const RecordObject& record_obj);

void PreCallRecordMergeValidationCachesEXT(VkDevice device, VkValidationCacheEXT dstCache, uint32_t srcCacheCount,
                                           const VkValidationCacheEXT* pSrcCaches);

void PostCallRecordMergeValidationCachesEXT(VkDevice device, VkValidationCacheEXT dstCache, uint32_t srcCacheCount,
                                            const VkValidationCacheEXT* pSrcCaches, const RecordObject& record_obj);

void PreCallRecordGetValidationCacheDataEXT(VkDevice device, VkValidationCacheEXT validationCache, size_t* pDataSize, void* pData);

void PostCallRecordGetValidationCacheDataEXT(VkDevice device, VkValidationCacheEXT validationCache, size_t* pDataSize, void* pData,
                                             const RecordObject& record_obj);

void PreCallRecordCmdBindShadingRateImageNV(VkCommandBuffer commandBuffer, VkImageView imageView,
                                            VkImageLayout imageLayout) override;

void PostCallRecordCmdBindShadingRateImageNV(VkCommandBuffer commandBuffer, VkImageView imageView, VkImageLayout imageLayout,
                                             const RecordObject& record_obj) override;

void PreCallRecordCmdSetViewportShadingRatePaletteNV(VkCommandBuffer commandBuffer, uint32_t firstViewport, uint32_t viewportCount,
                                                     const VkShadingRatePaletteNV* pShadingRatePalettes) override;

void PostCallRecordCmdSetViewportShadingRatePaletteNV(VkCommandBuffer commandBuffer, uint32_t firstViewport, uint32_t viewportCount,
                                                      const VkShadingRatePaletteNV* pShadingRatePalettes,
                                                      const RecordObject& record_obj) override;

void PreCallRecordCmdSetCoarseSampleOrderNV(VkCommandBuffer commandBuffer, VkCoarseSampleOrderTypeNV sampleOrderType,
                                            uint32_t customSampleOrderCount,
                                            const VkCoarseSampleOrderCustomNV* pCustomSampleOrders) override;

void PostCallRecordCmdSetCoarseSampleOrderNV(VkCommandBuffer commandBuffer, VkCoarseSampleOrderTypeNV sampleOrderType,
                                             uint32_t customSampleOrderCount,
                                             const VkCoarseSampleOrderCustomNV* pCustomSampleOrders,
                                             const RecordObject& record_obj) override;

void PreCallRecordCreateAccelerationStructureNV(VkDevice device, const VkAccelerationStructureCreateInfoNV* pCreateInfo,
                                                const VkAllocationCallbacks* pAllocator,
                                                VkAccelerationStructureNV* pAccelerationStructure) override;

void PostCallRecordCreateAccelerationStructureNV(VkDevice device, const VkAccelerationStructureCreateInfoNV* pCreateInfo,
                                                 const VkAllocationCallbacks* pAllocator,
                                                 VkAccelerationStructureNV* pAccelerationStructure,
                                                 const RecordObject& record_obj) override;

void PreCallRecordDestroyAccelerationStructureNV(VkDevice device, VkAccelerationStructureNV accelerationStructure,
                                                 const VkAllocationCallbacks* pAllocator) override;

void PostCallRecordDestroyAccelerationStructureNV(VkDevice device, VkAccelerationStructureNV accelerationStructure,
                                                  const VkAllocationCallbacks* pAllocator, const RecordObject& record_obj) override;

void PreCallRecordGetAccelerationStructureMemoryRequirementsNV(VkDevice device,
                                                               const VkAccelerationStructureMemoryRequirementsInfoNV* pInfo,
                                                               VkMemoryRequirements2KHR* pMemoryRequirements) override;

void PostCallRecordGetAccelerationStructureMemoryRequirementsNV(VkDevice device,
                                                                const VkAccelerationStructureMemoryRequirementsInfoNV* pInfo,
                                                                VkMemoryRequirements2KHR* pMemoryRequirements,
                                                                const RecordObject& record_obj) override;

void PreCallRecordBindAccelerationStructureMemoryNV(VkDevice device, uint32_t bindInfoCount,
                                                    const VkBindAccelerationStructureMemoryInfoNV* pBindInfos) override;

void PostCallRecordBindAccelerationStructureMemoryNV(VkDevice device, uint32_t bindInfoCount,
                                                     const VkBindAccelerationStructureMemoryInfoNV* pBindInfos,
                                                     const RecordObject& record_obj) override;

void PreCallRecordCmdBuildAccelerationStructureNV(VkCommandBuffer commandBuffer, const VkAccelerationStructureInfoNV* pInfo,
                                                  VkBuffer instanceData, VkDeviceSize instanceOffset, VkBool32 update,
                                                  VkAccelerationStructureNV dst, VkAccelerationStructureNV src, VkBuffer scratch,
                                                  VkDeviceSize scratchOffset) override;

void PostCallRecordCmdBuildAccelerationStructureNV(VkCommandBuffer commandBuffer, const VkAccelerationStructureInfoNV* pInfo,
                                                   VkBuffer instanceData, VkDeviceSize instanceOffset, VkBool32 update,
                                                   VkAccelerationStructureNV dst, VkAccelerationStructureNV src, VkBuffer scratch,
                                                   VkDeviceSize scratchOffset, const RecordObject& record_obj) override;

void PreCallRecordCmdCopyAccelerationStructureNV(VkCommandBuffer commandBuffer, VkAccelerationStructureNV dst,
                                                 VkAccelerationStructureNV src, VkCopyAccelerationStructureModeKHR mode) override;

void PostCallRecordCmdCopyAccelerationStructureNV(VkCommandBuffer commandBuffer, VkAccelerationStructureNV dst,
                                                  VkAccelerationStructureNV src, VkCopyAccelerationStructureModeKHR mode,
                                                  const RecordObject& record_obj) override;

void PreCallRecordCmdTraceRaysNV(VkCommandBuffer commandBuffer, VkBuffer raygenShaderBindingTableBuffer,
                                 VkDeviceSize raygenShaderBindingOffset, VkBuffer missShaderBindingTableBuffer,
                                 VkDeviceSize missShaderBindingOffset, VkDeviceSize missShaderBindingStride,
                                 VkBuffer hitShaderBindingTableBuffer, VkDeviceSize hitShaderBindingOffset,
                                 VkDeviceSize hitShaderBindingStride, VkBuffer callableShaderBindingTableBuffer,
                                 VkDeviceSize callableShaderBindingOffset, VkDeviceSize callableShaderBindingStride, uint32_t width,
                                 uint32_t height, uint32_t depth) override;

void PostCallRecordCmdTraceRaysNV(VkCommandBuffer commandBuffer, VkBuffer raygenShaderBindingTableBuffer,
                                  VkDeviceSize raygenShaderBindingOffset, VkBuffer missShaderBindingTableBuffer,
                                  VkDeviceSize missShaderBindingOffset, VkDeviceSize missShaderBindingStride,
                                  VkBuffer hitShaderBindingTableBuffer, VkDeviceSize hitShaderBindingOffset,
                                  VkDeviceSize hitShaderBindingStride, VkBuffer callableShaderBindingTableBuffer,
                                  VkDeviceSize callableShaderBindingOffset, VkDeviceSize callableShaderBindingStride,
                                  uint32_t width, uint32_t height, uint32_t depth, const RecordObject& record_obj) override;

void PreCallRecordCreateRayTracingPipelinesNV(VkDevice device, VkPipelineCache pipelineCache, uint32_t createInfoCount,
                                              const VkRayTracingPipelineCreateInfoNV* pCreateInfos,
                                              const VkAllocationCallbacks* pAllocator, VkPipeline* pPipelines) override;

void PostCallRecordCreateRayTracingPipelinesNV(VkDevice device, VkPipelineCache pipelineCache, uint32_t createInfoCount,
                                               const VkRayTracingPipelineCreateInfoNV* pCreateInfos,
                                               const VkAllocationCallbacks* pAllocator, VkPipeline* pPipelines,
                                               const RecordObject& record_obj) override;

void PreCallRecordGetRayTracingShaderGroupHandlesKHR(VkDevice device, VkPipeline pipeline, uint32_t firstGroup, uint32_t groupCount,
                                                     size_t dataSize, void* pData) override;

void PostCallRecordGetRayTracingShaderGroupHandlesKHR(VkDevice device, VkPipeline pipeline, uint32_t firstGroup,
                                                      uint32_t groupCount, size_t dataSize, void* pData,
                                                      const RecordObject& record_obj) override;

void PreCallRecordGetRayTracingShaderGroupHandlesNV(VkDevice device, VkPipeline pipeline, uint32_t firstGroup, uint32_t groupCount,
                                                    size_t dataSize, void* pData) override;

void PostCallRecordGetRayTracingShaderGroupHandlesNV(VkDevice device, VkPipeline pipeline, uint32_t firstGroup, uint32_t groupCount,
                                                     size_t dataSize, void* pData, const RecordObject& record_obj) override;

void PreCallRecordGetAccelerationStructureHandleNV(VkDevice device, VkAccelerationStructureNV accelerationStructure,
                                                   size_t dataSize, void* pData) override;

void PostCallRecordGetAccelerationStructureHandleNV(VkDevice device, VkAccelerationStructureNV accelerationStructure,
                                                    size_t dataSize, void* pData, const RecordObject& record_obj) override;

void PreCallRecordCmdWriteAccelerationStructuresPropertiesNV(VkCommandBuffer commandBuffer, uint32_t accelerationStructureCount,
                                                             const VkAccelerationStructureNV* pAccelerationStructures,
                                                             VkQueryType queryType, VkQueryPool queryPool,
                                                             uint32_t firstQuery) override;

void PostCallRecordCmdWriteAccelerationStructuresPropertiesNV(VkCommandBuffer commandBuffer, uint32_t accelerationStructureCount,
                                                              const VkAccelerationStructureNV* pAccelerationStructures,
                                                              VkQueryType queryType, VkQueryPool queryPool, uint32_t firstQuery,
                                                              const RecordObject& record_obj) override;

void PreCallRecordCompileDeferredNV(VkDevice device, VkPipeline pipeline, uint32_t shader) override;

void PostCallRecordCompileDeferredNV(VkDevice device, VkPipeline pipeline, uint32_t shader,
                                     const RecordObject& record_obj) override;

void PreCallRecordGetMemoryHostPointerPropertiesEXT(VkDevice device, VkExternalMemoryHandleTypeFlagBits handleType,
                                                    const void* pHostPointer,
                                                    VkMemoryHostPointerPropertiesEXT* pMemoryHostPointerProperties) override;

void PostCallRecordGetMemoryHostPointerPropertiesEXT(VkDevice device, VkExternalMemoryHandleTypeFlagBits handleType,
                                                     const void* pHostPointer,
                                                     VkMemoryHostPointerPropertiesEXT* pMemoryHostPointerProperties,
                                                     const RecordObject& record_obj) override;

void PreCallRecordCmdWriteBufferMarkerAMD(VkCommandBuffer commandBuffer, VkPipelineStageFlagBits pipelineStage, VkBuffer dstBuffer,
                                          VkDeviceSize dstOffset, uint32_t marker) override;

void PostCallRecordCmdWriteBufferMarkerAMD(VkCommandBuffer commandBuffer, VkPipelineStageFlagBits pipelineStage, VkBuffer dstBuffer,
                                           VkDeviceSize dstOffset, uint32_t marker, const RecordObject& record_obj) override;

void PreCallRecordGetCalibratedTimestampsEXT(VkDevice device, uint32_t timestampCount,
                                             const VkCalibratedTimestampInfoEXT* pTimestampInfos, uint64_t* pTimestamps,
                                             uint64_t* pMaxDeviation) override;

void PostCallRecordGetCalibratedTimestampsEXT(VkDevice device, uint32_t timestampCount,
                                              const VkCalibratedTimestampInfoEXT* pTimestampInfos, uint64_t* pTimestamps,
                                              uint64_t* pMaxDeviation, const RecordObject& record_obj) override;

void PreCallRecordCmdDrawMeshTasksNV(VkCommandBuffer commandBuffer, uint32_t taskCount, uint32_t firstTask) override;

void PostCallRecordCmdDrawMeshTasksNV(VkCommandBuffer commandBuffer, uint32_t taskCount, uint32_t firstTask,
                                      const RecordObject& record_obj) override;

void PreCallRecordCmdDrawMeshTasksIndirectNV(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                             uint32_t drawCount, uint32_t stride) override;

void PostCallRecordCmdDrawMeshTasksIndirectNV(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                              uint32_t drawCount, uint32_t stride, const RecordObject& record_obj) override;

void PreCallRecordCmdDrawMeshTasksIndirectCountNV(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                  VkBuffer countBuffer, VkDeviceSize countBufferOffset, uint32_t maxDrawCount,
                                                  uint32_t stride) override;

void PostCallRecordCmdDrawMeshTasksIndirectCountNV(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                   VkBuffer countBuffer, VkDeviceSize countBufferOffset, uint32_t maxDrawCount,
                                                   uint32_t stride, const RecordObject& record_obj) override;

void PreCallRecordCmdSetExclusiveScissorEnableNV(VkCommandBuffer commandBuffer, uint32_t firstExclusiveScissor,
                                                 uint32_t exclusiveScissorCount, const VkBool32* pExclusiveScissorEnables) override;

void PostCallRecordCmdSetExclusiveScissorEnableNV(VkCommandBuffer commandBuffer, uint32_t firstExclusiveScissor,
                                                  uint32_t exclusiveScissorCount, const VkBool32* pExclusiveScissorEnables,
                                                  const RecordObject& record_obj) override;

void PreCallRecordCmdSetExclusiveScissorNV(VkCommandBuffer commandBuffer, uint32_t firstExclusiveScissor,
                                           uint32_t exclusiveScissorCount, const VkRect2D* pExclusiveScissors) override;

void PostCallRecordCmdSetExclusiveScissorNV(VkCommandBuffer commandBuffer, uint32_t firstExclusiveScissor,
                                            uint32_t exclusiveScissorCount, const VkRect2D* pExclusiveScissors,
                                            const RecordObject& record_obj) override;

void PreCallRecordCmdSetCheckpointNV(VkCommandBuffer commandBuffer, const void* pCheckpointMarker) override;

void PostCallRecordCmdSetCheckpointNV(VkCommandBuffer commandBuffer, const void* pCheckpointMarker,
                                      const RecordObject& record_obj) override;

void PreCallRecordGetQueueCheckpointDataNV(VkQueue queue, uint32_t* pCheckpointDataCount,
                                           VkCheckpointDataNV* pCheckpointData) override;

void PostCallRecordGetQueueCheckpointDataNV(VkQueue queue, uint32_t* pCheckpointDataCount, VkCheckpointDataNV* pCheckpointData,
                                            const RecordObject& record_obj) override;

void PreCallRecordInitializePerformanceApiINTEL(VkDevice device,
                                                const VkInitializePerformanceApiInfoINTEL* pInitializeInfo) override;

void PostCallRecordInitializePerformanceApiINTEL(VkDevice device, const VkInitializePerformanceApiInfoINTEL* pInitializeInfo,
                                                 const RecordObject& record_obj) override;

void PreCallRecordUninitializePerformanceApiINTEL(VkDevice device) override;

void PostCallRecordUninitializePerformanceApiINTEL(VkDevice device, const RecordObject& record_obj) override;

void PreCallRecordCmdSetPerformanceMarkerINTEL(VkCommandBuffer commandBuffer,
                                               const VkPerformanceMarkerInfoINTEL* pMarkerInfo) override;

void PostCallRecordCmdSetPerformanceMarkerINTEL(VkCommandBuffer commandBuffer, const VkPerformanceMarkerInfoINTEL* pMarkerInfo,
                                                const RecordObject& record_obj) override;

void PreCallRecordCmdSetPerformanceStreamMarkerINTEL(VkCommandBuffer commandBuffer,
                                                     const VkPerformanceStreamMarkerInfoINTEL* pMarkerInfo) override;

void PostCallRecordCmdSetPerformanceStreamMarkerINTEL(VkCommandBuffer commandBuffer,
                                                      const VkPerformanceStreamMarkerInfoINTEL* pMarkerInfo,
                                                      const RecordObject& record_obj) override;

void PreCallRecordCmdSetPerformanceOverrideINTEL(VkCommandBuffer commandBuffer,
                                                 const VkPerformanceOverrideInfoINTEL* pOverrideInfo) override;

void PostCallRecordCmdSetPerformanceOverrideINTEL(VkCommandBuffer commandBuffer,
                                                  const VkPerformanceOverrideInfoINTEL* pOverrideInfo,
                                                  const RecordObject& record_obj) override;

void PreCallRecordAcquirePerformanceConfigurationINTEL(VkDevice device,
                                                       const VkPerformanceConfigurationAcquireInfoINTEL* pAcquireInfo,
                                                       VkPerformanceConfigurationINTEL* pConfiguration) override;

void PostCallRecordAcquirePerformanceConfigurationINTEL(VkDevice device,
                                                        const VkPerformanceConfigurationAcquireInfoINTEL* pAcquireInfo,
                                                        VkPerformanceConfigurationINTEL* pConfiguration,
                                                        const RecordObject& record_obj) override;

void PreCallRecordReleasePerformanceConfigurationINTEL(VkDevice device, VkPerformanceConfigurationINTEL configuration) override;

void PostCallRecordReleasePerformanceConfigurationINTEL(VkDevice device, VkPerformanceConfigurationINTEL configuration,
                                                        const RecordObject& record_obj) override;

void PreCallRecordQueueSetPerformanceConfigurationINTEL(VkQueue queue, VkPerformanceConfigurationINTEL configuration) override;

void PostCallRecordQueueSetPerformanceConfigurationINTEL(VkQueue queue, VkPerformanceConfigurationINTEL configuration,
                                                         const RecordObject& record_obj) override;

void PreCallRecordGetPerformanceParameterINTEL(VkDevice device, VkPerformanceParameterTypeINTEL parameter,
                                               VkPerformanceValueINTEL* pValue) override;

void PostCallRecordGetPerformanceParameterINTEL(VkDevice device, VkPerformanceParameterTypeINTEL parameter,
                                                VkPerformanceValueINTEL* pValue, const RecordObject& record_obj) override;

void PreCallRecordSetLocalDimmingAMD(VkDevice device, VkSwapchainKHR swapChain, VkBool32 localDimmingEnable) override;

void PostCallRecordSetLocalDimmingAMD(VkDevice device, VkSwapchainKHR swapChain, VkBool32 localDimmingEnable,
                                      const RecordObject& record_obj) override;

#ifdef VK_USE_PLATFORM_FUCHSIA
void PreCallRecordCreateImagePipeSurfaceFUCHSIA(VkInstance instance, const VkImagePipeSurfaceCreateInfoFUCHSIA* pCreateInfo,
                                                const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface) override;

void PostCallRecordCreateImagePipeSurfaceFUCHSIA(VkInstance instance, const VkImagePipeSurfaceCreateInfoFUCHSIA* pCreateInfo,
                                                 const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface,
                                                 const RecordObject& record_obj) override;

#endif  // VK_USE_PLATFORM_FUCHSIA
#ifdef VK_USE_PLATFORM_METAL_EXT
void PreCallRecordCreateMetalSurfaceEXT(VkInstance instance, const VkMetalSurfaceCreateInfoEXT* pCreateInfo,
                                        const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface) override;

void PostCallRecordCreateMetalSurfaceEXT(VkInstance instance, const VkMetalSurfaceCreateInfoEXT* pCreateInfo,
                                         const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface,
                                         const RecordObject& record_obj) override;

#endif  // VK_USE_PLATFORM_METAL_EXT
void PreCallRecordGetBufferDeviceAddressEXT(VkDevice device, const VkBufferDeviceAddressInfo* pInfo) override;

void PostCallRecordGetBufferDeviceAddressEXT(VkDevice device, const VkBufferDeviceAddressInfo* pInfo,
                                             const RecordObject& record_obj) override;

#ifdef VK_USE_PLATFORM_WIN32_KHR
void PreCallRecordAcquireFullScreenExclusiveModeEXT(VkDevice device, VkSwapchainKHR swapchain) override;

void PostCallRecordAcquireFullScreenExclusiveModeEXT(VkDevice device, VkSwapchainKHR swapchain,
                                                     const RecordObject& record_obj) override;

#endif  // VK_USE_PLATFORM_WIN32_KHR
#ifdef VK_USE_PLATFORM_WIN32_KHR
void PreCallRecordReleaseFullScreenExclusiveModeEXT(VkDevice device, VkSwapchainKHR swapchain) override;

void PostCallRecordReleaseFullScreenExclusiveModeEXT(VkDevice device, VkSwapchainKHR swapchain,
                                                     const RecordObject& record_obj) override;

#endif  // VK_USE_PLATFORM_WIN32_KHR
#ifdef VK_USE_PLATFORM_WIN32_KHR
void PreCallRecordGetDeviceGroupSurfacePresentModes2EXT(VkDevice device, const VkPhysicalDeviceSurfaceInfo2KHR* pSurfaceInfo,
                                                        VkDeviceGroupPresentModeFlagsKHR* pModes) override;

void PostCallRecordGetDeviceGroupSurfacePresentModes2EXT(VkDevice device, const VkPhysicalDeviceSurfaceInfo2KHR* pSurfaceInfo,
                                                         VkDeviceGroupPresentModeFlagsKHR* pModes,
                                                         const RecordObject& record_obj) override;

#endif  // VK_USE_PLATFORM_WIN32_KHR
void PreCallRecordCreateHeadlessSurfaceEXT(VkInstance instance, const VkHeadlessSurfaceCreateInfoEXT* pCreateInfo,
                                           const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface) override;

void PostCallRecordCreateHeadlessSurfaceEXT(VkInstance instance, const VkHeadlessSurfaceCreateInfoEXT* pCreateInfo,
                                            const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface,
                                            const RecordObject& record_obj) override;

void PreCallRecordCmdSetLineStippleEXT(VkCommandBuffer commandBuffer, uint32_t lineStippleFactor,
                                       uint16_t lineStipplePattern) override;

void PostCallRecordCmdSetLineStippleEXT(VkCommandBuffer commandBuffer, uint32_t lineStippleFactor, uint16_t lineStipplePattern,
                                        const RecordObject& record_obj) override;

void PreCallRecordResetQueryPoolEXT(VkDevice device, VkQueryPool queryPool, uint32_t firstQuery, uint32_t queryCount) override;

void PostCallRecordResetQueryPoolEXT(VkDevice device, VkQueryPool queryPool, uint32_t firstQuery, uint32_t queryCount,
                                     const RecordObject& record_obj) override;

void PreCallRecordCmdSetCullModeEXT(VkCommandBuffer commandBuffer, VkCullModeFlags cullMode) override;

void PostCallRecordCmdSetCullModeEXT(VkCommandBuffer commandBuffer, VkCullModeFlags cullMode,
                                     const RecordObject& record_obj) override;

void PreCallRecordCmdSetFrontFaceEXT(VkCommandBuffer commandBuffer, VkFrontFace frontFace) override;

void PostCallRecordCmdSetFrontFaceEXT(VkCommandBuffer commandBuffer, VkFrontFace frontFace,
                                      const RecordObject& record_obj) override;

void PreCallRecordCmdSetPrimitiveTopologyEXT(VkCommandBuffer commandBuffer, VkPrimitiveTopology primitiveTopology) override;

void PostCallRecordCmdSetPrimitiveTopologyEXT(VkCommandBuffer commandBuffer, VkPrimitiveTopology primitiveTopology,
                                              const RecordObject& record_obj) override;

void PreCallRecordCmdSetViewportWithCountEXT(VkCommandBuffer commandBuffer, uint32_t viewportCount,
                                             const VkViewport* pViewports) override;

void PostCallRecordCmdSetViewportWithCountEXT(VkCommandBuffer commandBuffer, uint32_t viewportCount, const VkViewport* pViewports,
                                              const RecordObject& record_obj) override;

void PreCallRecordCmdSetScissorWithCountEXT(VkCommandBuffer commandBuffer, uint32_t scissorCount,
                                            const VkRect2D* pScissors) override;

void PostCallRecordCmdSetScissorWithCountEXT(VkCommandBuffer commandBuffer, uint32_t scissorCount, const VkRect2D* pScissors,
                                             const RecordObject& record_obj) override;

void PreCallRecordCmdBindVertexBuffers2EXT(VkCommandBuffer commandBuffer, uint32_t firstBinding, uint32_t bindingCount,
                                           const VkBuffer* pBuffers, const VkDeviceSize* pOffsets, const VkDeviceSize* pSizes,
                                           const VkDeviceSize* pStrides) override;

void PostCallRecordCmdBindVertexBuffers2EXT(VkCommandBuffer commandBuffer, uint32_t firstBinding, uint32_t bindingCount,
                                            const VkBuffer* pBuffers, const VkDeviceSize* pOffsets, const VkDeviceSize* pSizes,
                                            const VkDeviceSize* pStrides, const RecordObject& record_obj) override;

void PreCallRecordCmdSetDepthTestEnableEXT(VkCommandBuffer commandBuffer, VkBool32 depthTestEnable) override;

void PostCallRecordCmdSetDepthTestEnableEXT(VkCommandBuffer commandBuffer, VkBool32 depthTestEnable,
                                            const RecordObject& record_obj) override;

void PreCallRecordCmdSetDepthWriteEnableEXT(VkCommandBuffer commandBuffer, VkBool32 depthWriteEnable) override;

void PostCallRecordCmdSetDepthWriteEnableEXT(VkCommandBuffer commandBuffer, VkBool32 depthWriteEnable,
                                             const RecordObject& record_obj) override;

void PreCallRecordCmdSetDepthCompareOpEXT(VkCommandBuffer commandBuffer, VkCompareOp depthCompareOp) override;

void PostCallRecordCmdSetDepthCompareOpEXT(VkCommandBuffer commandBuffer, VkCompareOp depthCompareOp,
                                           const RecordObject& record_obj) override;

void PreCallRecordCmdSetDepthBoundsTestEnableEXT(VkCommandBuffer commandBuffer, VkBool32 depthBoundsTestEnable) override;

void PostCallRecordCmdSetDepthBoundsTestEnableEXT(VkCommandBuffer commandBuffer, VkBool32 depthBoundsTestEnable,
                                                  const RecordObject& record_obj) override;

void PreCallRecordCmdSetStencilTestEnableEXT(VkCommandBuffer commandBuffer, VkBool32 stencilTestEnable) override;

void PostCallRecordCmdSetStencilTestEnableEXT(VkCommandBuffer commandBuffer, VkBool32 stencilTestEnable,
                                              const RecordObject& record_obj) override;

void PreCallRecordCmdSetStencilOpEXT(VkCommandBuffer commandBuffer, VkStencilFaceFlags faceMask, VkStencilOp failOp,
                                     VkStencilOp passOp, VkStencilOp depthFailOp, VkCompareOp compareOp) override;

void PostCallRecordCmdSetStencilOpEXT(VkCommandBuffer commandBuffer, VkStencilFaceFlags faceMask, VkStencilOp failOp,
                                      VkStencilOp passOp, VkStencilOp depthFailOp, VkCompareOp compareOp,
                                      const RecordObject& record_obj) override;

void PreCallRecordCopyMemoryToImageEXT(VkDevice device, const VkCopyMemoryToImageInfoEXT* pCopyMemoryToImageInfo) override;

void PostCallRecordCopyMemoryToImageEXT(VkDevice device, const VkCopyMemoryToImageInfoEXT* pCopyMemoryToImageInfo,
                                        const RecordObject& record_obj) override;

void PreCallRecordCopyImageToMemoryEXT(VkDevice device, const VkCopyImageToMemoryInfoEXT* pCopyImageToMemoryInfo) override;

void PostCallRecordCopyImageToMemoryEXT(VkDevice device, const VkCopyImageToMemoryInfoEXT* pCopyImageToMemoryInfo,
                                        const RecordObject& record_obj) override;

void PreCallRecordCopyImageToImageEXT(VkDevice device, const VkCopyImageToImageInfoEXT* pCopyImageToImageInfo) override;

void PostCallRecordCopyImageToImageEXT(VkDevice device, const VkCopyImageToImageInfoEXT* pCopyImageToImageInfo,
                                       const RecordObject& record_obj) override;

void PreCallRecordTransitionImageLayoutEXT(VkDevice device, uint32_t transitionCount,
                                           const VkHostImageLayoutTransitionInfoEXT* pTransitions) override;

void PostCallRecordTransitionImageLayoutEXT(VkDevice device, uint32_t transitionCount,
                                            const VkHostImageLayoutTransitionInfoEXT* pTransitions,
                                            const RecordObject& record_obj) override;

void PreCallRecordGetImageSubresourceLayout2EXT(VkDevice device, VkImage image, const VkImageSubresource2KHR* pSubresource,
                                                VkSubresourceLayout2KHR* pLayout) override;

void PostCallRecordGetImageSubresourceLayout2EXT(VkDevice device, VkImage image, const VkImageSubresource2KHR* pSubresource,
                                                 VkSubresourceLayout2KHR* pLayout, const RecordObject& record_obj) override;

void PreCallRecordReleaseSwapchainImagesEXT(VkDevice device, const VkReleaseSwapchainImagesInfoEXT* pReleaseInfo) override;

void PostCallRecordReleaseSwapchainImagesEXT(VkDevice device, const VkReleaseSwapchainImagesInfoEXT* pReleaseInfo,
                                             const RecordObject& record_obj) override;

void PreCallRecordGetGeneratedCommandsMemoryRequirementsNV(VkDevice device,
                                                           const VkGeneratedCommandsMemoryRequirementsInfoNV* pInfo,
                                                           VkMemoryRequirements2* pMemoryRequirements) override;

void PostCallRecordGetGeneratedCommandsMemoryRequirementsNV(VkDevice device,
                                                            const VkGeneratedCommandsMemoryRequirementsInfoNV* pInfo,
                                                            VkMemoryRequirements2* pMemoryRequirements,
                                                            const RecordObject& record_obj) override;

void PreCallRecordCmdPreprocessGeneratedCommandsNV(VkCommandBuffer commandBuffer,
                                                   const VkGeneratedCommandsInfoNV* pGeneratedCommandsInfo) override;

void PostCallRecordCmdPreprocessGeneratedCommandsNV(VkCommandBuffer commandBuffer,
                                                    const VkGeneratedCommandsInfoNV* pGeneratedCommandsInfo,
                                                    const RecordObject& record_obj) override;

void PreCallRecordCmdExecuteGeneratedCommandsNV(VkCommandBuffer commandBuffer, VkBool32 isPreprocessed,
                                                const VkGeneratedCommandsInfoNV* pGeneratedCommandsInfo) override;

void PostCallRecordCmdExecuteGeneratedCommandsNV(VkCommandBuffer commandBuffer, VkBool32 isPreprocessed,
                                                 const VkGeneratedCommandsInfoNV* pGeneratedCommandsInfo,
                                                 const RecordObject& record_obj) override;

void PreCallRecordCmdBindPipelineShaderGroupNV(VkCommandBuffer commandBuffer, VkPipelineBindPoint pipelineBindPoint,
                                               VkPipeline pipeline, uint32_t groupIndex) override;

void PostCallRecordCmdBindPipelineShaderGroupNV(VkCommandBuffer commandBuffer, VkPipelineBindPoint pipelineBindPoint,
                                                VkPipeline pipeline, uint32_t groupIndex, const RecordObject& record_obj) override;

void PreCallRecordCreateIndirectCommandsLayoutNV(VkDevice device, const VkIndirectCommandsLayoutCreateInfoNV* pCreateInfo,
                                                 const VkAllocationCallbacks* pAllocator,
                                                 VkIndirectCommandsLayoutNV* pIndirectCommandsLayout) override;

void PostCallRecordCreateIndirectCommandsLayoutNV(VkDevice device, const VkIndirectCommandsLayoutCreateInfoNV* pCreateInfo,
                                                  const VkAllocationCallbacks* pAllocator,
                                                  VkIndirectCommandsLayoutNV* pIndirectCommandsLayout,
                                                  const RecordObject& record_obj) override;

void PreCallRecordDestroyIndirectCommandsLayoutNV(VkDevice device, VkIndirectCommandsLayoutNV indirectCommandsLayout,
                                                  const VkAllocationCallbacks* pAllocator) override;

void PostCallRecordDestroyIndirectCommandsLayoutNV(VkDevice device, VkIndirectCommandsLayoutNV indirectCommandsLayout,
                                                   const VkAllocationCallbacks* pAllocator,
                                                   const RecordObject& record_obj) override;

void PreCallRecordCmdSetDepthBias2EXT(VkCommandBuffer commandBuffer, const VkDepthBiasInfoEXT* pDepthBiasInfo) override;

void PostCallRecordCmdSetDepthBias2EXT(VkCommandBuffer commandBuffer, const VkDepthBiasInfoEXT* pDepthBiasInfo,
                                       const RecordObject& record_obj) override;

void PreCallRecordAcquireDrmDisplayEXT(VkPhysicalDevice physicalDevice, int32_t drmFd, VkDisplayKHR display) override;

void PostCallRecordAcquireDrmDisplayEXT(VkPhysicalDevice physicalDevice, int32_t drmFd, VkDisplayKHR display,
                                        const RecordObject& record_obj) override;

void PreCallRecordCreatePrivateDataSlotEXT(VkDevice device, const VkPrivateDataSlotCreateInfo* pCreateInfo,
                                           const VkAllocationCallbacks* pAllocator, VkPrivateDataSlot* pPrivateDataSlot) override;

void PostCallRecordCreatePrivateDataSlotEXT(VkDevice device, const VkPrivateDataSlotCreateInfo* pCreateInfo,
                                            const VkAllocationCallbacks* pAllocator, VkPrivateDataSlot* pPrivateDataSlot,
                                            const RecordObject& record_obj) override;

void PreCallRecordDestroyPrivateDataSlotEXT(VkDevice device, VkPrivateDataSlot privateDataSlot,
                                            const VkAllocationCallbacks* pAllocator) override;

void PostCallRecordDestroyPrivateDataSlotEXT(VkDevice device, VkPrivateDataSlot privateDataSlot,
                                             const VkAllocationCallbacks* pAllocator, const RecordObject& record_obj) override;

void PreCallRecordSetPrivateDataEXT(VkDevice device, VkObjectType objectType, uint64_t objectHandle,
                                    VkPrivateDataSlot privateDataSlot, uint64_t data) override;

void PostCallRecordSetPrivateDataEXT(VkDevice device, VkObjectType objectType, uint64_t objectHandle,
                                     VkPrivateDataSlot privateDataSlot, uint64_t data, const RecordObject& record_obj) override;

void PreCallRecordGetPrivateDataEXT(VkDevice device, VkObjectType objectType, uint64_t objectHandle,
                                    VkPrivateDataSlot privateDataSlot, uint64_t* pData) override;

void PostCallRecordGetPrivateDataEXT(VkDevice device, VkObjectType objectType, uint64_t objectHandle,
                                     VkPrivateDataSlot privateDataSlot, uint64_t* pData, const RecordObject& record_obj) override;

#ifdef VK_USE_PLATFORM_METAL_EXT
void PreCallRecordExportMetalObjectsEXT(VkDevice device, VkExportMetalObjectsInfoEXT* pMetalObjectsInfo) override;

void PostCallRecordExportMetalObjectsEXT(VkDevice device, VkExportMetalObjectsInfoEXT* pMetalObjectsInfo,
                                         const RecordObject& record_obj) override;

#endif  // VK_USE_PLATFORM_METAL_EXT
void PreCallRecordGetDescriptorSetLayoutSizeEXT(VkDevice device, VkDescriptorSetLayout layout,
                                                VkDeviceSize* pLayoutSizeInBytes) override;

void PostCallRecordGetDescriptorSetLayoutSizeEXT(VkDevice device, VkDescriptorSetLayout layout, VkDeviceSize* pLayoutSizeInBytes,
                                                 const RecordObject& record_obj) override;

void PreCallRecordGetDescriptorSetLayoutBindingOffsetEXT(VkDevice device, VkDescriptorSetLayout layout, uint32_t binding,
                                                         VkDeviceSize* pOffset) override;

void PostCallRecordGetDescriptorSetLayoutBindingOffsetEXT(VkDevice device, VkDescriptorSetLayout layout, uint32_t binding,
                                                          VkDeviceSize* pOffset, const RecordObject& record_obj) override;

void PreCallRecordGetDescriptorEXT(VkDevice device, const VkDescriptorGetInfoEXT* pDescriptorInfo, size_t dataSize,
                                   void* pDescriptor) override;

void PostCallRecordGetDescriptorEXT(VkDevice device, const VkDescriptorGetInfoEXT* pDescriptorInfo, size_t dataSize,
                                    void* pDescriptor, const RecordObject& record_obj) override;

void PreCallRecordCmdBindDescriptorBuffersEXT(VkCommandBuffer commandBuffer, uint32_t bufferCount,
                                              const VkDescriptorBufferBindingInfoEXT* pBindingInfos) override;

void PostCallRecordCmdBindDescriptorBuffersEXT(VkCommandBuffer commandBuffer, uint32_t bufferCount,
                                               const VkDescriptorBufferBindingInfoEXT* pBindingInfos,
                                               const RecordObject& record_obj) override;

void PreCallRecordCmdSetDescriptorBufferOffsetsEXT(VkCommandBuffer commandBuffer, VkPipelineBindPoint pipelineBindPoint,
                                                   VkPipelineLayout layout, uint32_t firstSet, uint32_t setCount,
                                                   const uint32_t* pBufferIndices, const VkDeviceSize* pOffsets) override;

void PostCallRecordCmdSetDescriptorBufferOffsetsEXT(VkCommandBuffer commandBuffer, VkPipelineBindPoint pipelineBindPoint,
                                                    VkPipelineLayout layout, uint32_t firstSet, uint32_t setCount,
                                                    const uint32_t* pBufferIndices, const VkDeviceSize* pOffsets,
                                                    const RecordObject& record_obj) override;

void PreCallRecordCmdBindDescriptorBufferEmbeddedSamplersEXT(VkCommandBuffer commandBuffer, VkPipelineBindPoint pipelineBindPoint,
                                                             VkPipelineLayout layout, uint32_t set) override;

void PostCallRecordCmdBindDescriptorBufferEmbeddedSamplersEXT(VkCommandBuffer commandBuffer, VkPipelineBindPoint pipelineBindPoint,
                                                              VkPipelineLayout layout, uint32_t set,
                                                              const RecordObject& record_obj) override;

void PreCallRecordGetBufferOpaqueCaptureDescriptorDataEXT(VkDevice device, const VkBufferCaptureDescriptorDataInfoEXT* pInfo,
                                                          void* pData) override;

void PostCallRecordGetBufferOpaqueCaptureDescriptorDataEXT(VkDevice device, const VkBufferCaptureDescriptorDataInfoEXT* pInfo,
                                                           void* pData, const RecordObject& record_obj) override;

void PreCallRecordGetImageOpaqueCaptureDescriptorDataEXT(VkDevice device, const VkImageCaptureDescriptorDataInfoEXT* pInfo,
                                                         void* pData) override;

void PostCallRecordGetImageOpaqueCaptureDescriptorDataEXT(VkDevice device, const VkImageCaptureDescriptorDataInfoEXT* pInfo,
                                                          void* pData, const RecordObject& record_obj) override;

void PreCallRecordGetImageViewOpaqueCaptureDescriptorDataEXT(VkDevice device, const VkImageViewCaptureDescriptorDataInfoEXT* pInfo,
                                                             void* pData) override;

void PostCallRecordGetImageViewOpaqueCaptureDescriptorDataEXT(VkDevice device, const VkImageViewCaptureDescriptorDataInfoEXT* pInfo,
                                                              void* pData, const RecordObject& record_obj) override;

void PreCallRecordGetSamplerOpaqueCaptureDescriptorDataEXT(VkDevice device, const VkSamplerCaptureDescriptorDataInfoEXT* pInfo,
                                                           void* pData) override;

void PostCallRecordGetSamplerOpaqueCaptureDescriptorDataEXT(VkDevice device, const VkSamplerCaptureDescriptorDataInfoEXT* pInfo,
                                                            void* pData, const RecordObject& record_obj) override;

void PreCallRecordGetAccelerationStructureOpaqueCaptureDescriptorDataEXT(
    VkDevice device, const VkAccelerationStructureCaptureDescriptorDataInfoEXT* pInfo, void* pData) override;

void PostCallRecordGetAccelerationStructureOpaqueCaptureDescriptorDataEXT(
    VkDevice device, const VkAccelerationStructureCaptureDescriptorDataInfoEXT* pInfo, void* pData,
    const RecordObject& record_obj) override;

void PreCallRecordCmdSetFragmentShadingRateEnumNV(VkCommandBuffer commandBuffer, VkFragmentShadingRateNV shadingRate,
                                                  const VkFragmentShadingRateCombinerOpKHR combinerOps[2]) override;

void PostCallRecordCmdSetFragmentShadingRateEnumNV(VkCommandBuffer commandBuffer, VkFragmentShadingRateNV shadingRate,
                                                   const VkFragmentShadingRateCombinerOpKHR combinerOps[2],
                                                   const RecordObject& record_obj) override;

void PreCallRecordGetDeviceFaultInfoEXT(VkDevice device, VkDeviceFaultCountsEXT* pFaultCounts,
                                        VkDeviceFaultInfoEXT* pFaultInfo) override;

void PostCallRecordGetDeviceFaultInfoEXT(VkDevice device, VkDeviceFaultCountsEXT* pFaultCounts, VkDeviceFaultInfoEXT* pFaultInfo,
                                         const RecordObject& record_obj) override;

#ifdef VK_USE_PLATFORM_WIN32_KHR
void PreCallRecordAcquireWinrtDisplayNV(VkPhysicalDevice physicalDevice, VkDisplayKHR display) override;

void PostCallRecordAcquireWinrtDisplayNV(VkPhysicalDevice physicalDevice, VkDisplayKHR display,
                                         const RecordObject& record_obj) override;

#endif  // VK_USE_PLATFORM_WIN32_KHR
#ifdef VK_USE_PLATFORM_DIRECTFB_EXT
void PreCallRecordCreateDirectFBSurfaceEXT(VkInstance instance, const VkDirectFBSurfaceCreateInfoEXT* pCreateInfo,
                                           const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface) override;

void PostCallRecordCreateDirectFBSurfaceEXT(VkInstance instance, const VkDirectFBSurfaceCreateInfoEXT* pCreateInfo,
                                            const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface,
                                            const RecordObject& record_obj) override;

#endif  // VK_USE_PLATFORM_DIRECTFB_EXT
void PreCallRecordCmdSetVertexInputEXT(VkCommandBuffer commandBuffer, uint32_t vertexBindingDescriptionCount,
                                       const VkVertexInputBindingDescription2EXT* pVertexBindingDescriptions,
                                       uint32_t vertexAttributeDescriptionCount,
                                       const VkVertexInputAttributeDescription2EXT* pVertexAttributeDescriptions) override;

void PostCallRecordCmdSetVertexInputEXT(VkCommandBuffer commandBuffer, uint32_t vertexBindingDescriptionCount,
                                        const VkVertexInputBindingDescription2EXT* pVertexBindingDescriptions,
                                        uint32_t vertexAttributeDescriptionCount,
                                        const VkVertexInputAttributeDescription2EXT* pVertexAttributeDescriptions,
                                        const RecordObject& record_obj) override;

#ifdef VK_USE_PLATFORM_FUCHSIA
void PreCallRecordGetMemoryZirconHandleFUCHSIA(VkDevice device, const VkMemoryGetZirconHandleInfoFUCHSIA* pGetZirconHandleInfo,
                                               zx_handle_t* pZirconHandle) override;

void PostCallRecordGetMemoryZirconHandleFUCHSIA(VkDevice device, const VkMemoryGetZirconHandleInfoFUCHSIA* pGetZirconHandleInfo,
                                                zx_handle_t* pZirconHandle, const RecordObject& record_obj) override;

#endif  // VK_USE_PLATFORM_FUCHSIA
#ifdef VK_USE_PLATFORM_FUCHSIA
void PreCallRecordGetMemoryZirconHandlePropertiesFUCHSIA(
    VkDevice device, VkExternalMemoryHandleTypeFlagBits handleType, zx_handle_t zirconHandle,
    VkMemoryZirconHandlePropertiesFUCHSIA* pMemoryZirconHandleProperties) override;

void PostCallRecordGetMemoryZirconHandlePropertiesFUCHSIA(VkDevice device, VkExternalMemoryHandleTypeFlagBits handleType,
                                                          zx_handle_t zirconHandle,
                                                          VkMemoryZirconHandlePropertiesFUCHSIA* pMemoryZirconHandleProperties,
                                                          const RecordObject& record_obj) override;

#endif  // VK_USE_PLATFORM_FUCHSIA
#ifdef VK_USE_PLATFORM_FUCHSIA
void PreCallRecordImportSemaphoreZirconHandleFUCHSIA(
    VkDevice device, const VkImportSemaphoreZirconHandleInfoFUCHSIA* pImportSemaphoreZirconHandleInfo) override;

void PostCallRecordImportSemaphoreZirconHandleFUCHSIA(
    VkDevice device, const VkImportSemaphoreZirconHandleInfoFUCHSIA* pImportSemaphoreZirconHandleInfo,
    const RecordObject& record_obj) override;

#endif  // VK_USE_PLATFORM_FUCHSIA
#ifdef VK_USE_PLATFORM_FUCHSIA
void PreCallRecordGetSemaphoreZirconHandleFUCHSIA(VkDevice device,
                                                  const VkSemaphoreGetZirconHandleInfoFUCHSIA* pGetZirconHandleInfo,
                                                  zx_handle_t* pZirconHandle) override;

void PostCallRecordGetSemaphoreZirconHandleFUCHSIA(VkDevice device,
                                                   const VkSemaphoreGetZirconHandleInfoFUCHSIA* pGetZirconHandleInfo,
                                                   zx_handle_t* pZirconHandle, const RecordObject& record_obj) override;

#endif  // VK_USE_PLATFORM_FUCHSIA
#ifdef VK_USE_PLATFORM_FUCHSIA
void PreCallRecordCreateBufferCollectionFUCHSIA(VkDevice device, const VkBufferCollectionCreateInfoFUCHSIA* pCreateInfo,
                                                const VkAllocationCallbacks* pAllocator,
                                                VkBufferCollectionFUCHSIA* pCollection) override;

void PostCallRecordCreateBufferCollectionFUCHSIA(VkDevice device, const VkBufferCollectionCreateInfoFUCHSIA* pCreateInfo,
                                                 const VkAllocationCallbacks* pAllocator, VkBufferCollectionFUCHSIA* pCollection,
                                                 const RecordObject& record_obj) override;

#endif  // VK_USE_PLATFORM_FUCHSIA
#ifdef VK_USE_PLATFORM_FUCHSIA
void PreCallRecordSetBufferCollectionImageConstraintsFUCHSIA(VkDevice device, VkBufferCollectionFUCHSIA collection,
                                                             const VkImageConstraintsInfoFUCHSIA* pImageConstraintsInfo) override;

void PostCallRecordSetBufferCollectionImageConstraintsFUCHSIA(VkDevice device, VkBufferCollectionFUCHSIA collection,
                                                              const VkImageConstraintsInfoFUCHSIA* pImageConstraintsInfo,
                                                              const RecordObject& record_obj) override;

#endif  // VK_USE_PLATFORM_FUCHSIA
#ifdef VK_USE_PLATFORM_FUCHSIA
void PreCallRecordSetBufferCollectionBufferConstraintsFUCHSIA(
    VkDevice device, VkBufferCollectionFUCHSIA collection, const VkBufferConstraintsInfoFUCHSIA* pBufferConstraintsInfo) override;

void PostCallRecordSetBufferCollectionBufferConstraintsFUCHSIA(VkDevice device, VkBufferCollectionFUCHSIA collection,
                                                               const VkBufferConstraintsInfoFUCHSIA* pBufferConstraintsInfo,
                                                               const RecordObject& record_obj) override;

#endif  // VK_USE_PLATFORM_FUCHSIA
#ifdef VK_USE_PLATFORM_FUCHSIA
void PreCallRecordDestroyBufferCollectionFUCHSIA(VkDevice device, VkBufferCollectionFUCHSIA collection,
                                                 const VkAllocationCallbacks* pAllocator) override;

void PostCallRecordDestroyBufferCollectionFUCHSIA(VkDevice device, VkBufferCollectionFUCHSIA collection,
                                                  const VkAllocationCallbacks* pAllocator, const RecordObject& record_obj) override;

#endif  // VK_USE_PLATFORM_FUCHSIA
#ifdef VK_USE_PLATFORM_FUCHSIA
void PreCallRecordGetBufferCollectionPropertiesFUCHSIA(VkDevice device, VkBufferCollectionFUCHSIA collection,
                                                       VkBufferCollectionPropertiesFUCHSIA* pProperties) override;

void PostCallRecordGetBufferCollectionPropertiesFUCHSIA(VkDevice device, VkBufferCollectionFUCHSIA collection,
                                                        VkBufferCollectionPropertiesFUCHSIA* pProperties,
                                                        const RecordObject& record_obj) override;

#endif  // VK_USE_PLATFORM_FUCHSIA
void PreCallRecordGetDeviceSubpassShadingMaxWorkgroupSizeHUAWEI(VkDevice device, VkRenderPass renderpass,
                                                                VkExtent2D* pMaxWorkgroupSize) override;

void PostCallRecordGetDeviceSubpassShadingMaxWorkgroupSizeHUAWEI(VkDevice device, VkRenderPass renderpass,
                                                                 VkExtent2D* pMaxWorkgroupSize,
                                                                 const RecordObject& record_obj) override;

void PreCallRecordCmdSubpassShadingHUAWEI(VkCommandBuffer commandBuffer) override;

void PostCallRecordCmdSubpassShadingHUAWEI(VkCommandBuffer commandBuffer, const RecordObject& record_obj) override;

void PreCallRecordCmdBindInvocationMaskHUAWEI(VkCommandBuffer commandBuffer, VkImageView imageView,
                                              VkImageLayout imageLayout) override;

void PostCallRecordCmdBindInvocationMaskHUAWEI(VkCommandBuffer commandBuffer, VkImageView imageView, VkImageLayout imageLayout,
                                               const RecordObject& record_obj) override;

void PreCallRecordGetMemoryRemoteAddressNV(VkDevice device, const VkMemoryGetRemoteAddressInfoNV* pMemoryGetRemoteAddressInfo,
                                           VkRemoteAddressNV* pAddress) override;

void PostCallRecordGetMemoryRemoteAddressNV(VkDevice device, const VkMemoryGetRemoteAddressInfoNV* pMemoryGetRemoteAddressInfo,
                                            VkRemoteAddressNV* pAddress, const RecordObject& record_obj) override;

void PreCallRecordGetPipelinePropertiesEXT(VkDevice device, const VkPipelineInfoEXT* pPipelineInfo,
                                           VkBaseOutStructure* pPipelineProperties) override;

void PostCallRecordGetPipelinePropertiesEXT(VkDevice device, const VkPipelineInfoEXT* pPipelineInfo,
                                            VkBaseOutStructure* pPipelineProperties, const RecordObject& record_obj) override;

void PreCallRecordCmdSetPatchControlPointsEXT(VkCommandBuffer commandBuffer, uint32_t patchControlPoints) override;

void PostCallRecordCmdSetPatchControlPointsEXT(VkCommandBuffer commandBuffer, uint32_t patchControlPoints,
                                               const RecordObject& record_obj) override;

void PreCallRecordCmdSetRasterizerDiscardEnableEXT(VkCommandBuffer commandBuffer, VkBool32 rasterizerDiscardEnable) override;

void PostCallRecordCmdSetRasterizerDiscardEnableEXT(VkCommandBuffer commandBuffer, VkBool32 rasterizerDiscardEnable,
                                                    const RecordObject& record_obj) override;

void PreCallRecordCmdSetDepthBiasEnableEXT(VkCommandBuffer commandBuffer, VkBool32 depthBiasEnable) override;

void PostCallRecordCmdSetDepthBiasEnableEXT(VkCommandBuffer commandBuffer, VkBool32 depthBiasEnable,
                                            const RecordObject& record_obj) override;

void PreCallRecordCmdSetLogicOpEXT(VkCommandBuffer commandBuffer, VkLogicOp logicOp) override;

void PostCallRecordCmdSetLogicOpEXT(VkCommandBuffer commandBuffer, VkLogicOp logicOp, const RecordObject& record_obj) override;

void PreCallRecordCmdSetPrimitiveRestartEnableEXT(VkCommandBuffer commandBuffer, VkBool32 primitiveRestartEnable) override;

void PostCallRecordCmdSetPrimitiveRestartEnableEXT(VkCommandBuffer commandBuffer, VkBool32 primitiveRestartEnable,
                                                   const RecordObject& record_obj) override;

#ifdef VK_USE_PLATFORM_SCREEN_QNX
void PreCallRecordCreateScreenSurfaceQNX(VkInstance instance, const VkScreenSurfaceCreateInfoQNX* pCreateInfo,
                                         const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface) override;

void PostCallRecordCreateScreenSurfaceQNX(VkInstance instance, const VkScreenSurfaceCreateInfoQNX* pCreateInfo,
                                          const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface,
                                          const RecordObject& record_obj) override;

#endif  // VK_USE_PLATFORM_SCREEN_QNX
void PreCallRecordCmdSetColorWriteEnableEXT(VkCommandBuffer commandBuffer, uint32_t attachmentCount,
                                            const VkBool32* pColorWriteEnables) override;

void PostCallRecordCmdSetColorWriteEnableEXT(VkCommandBuffer commandBuffer, uint32_t attachmentCount,
                                             const VkBool32* pColorWriteEnables, const RecordObject& record_obj) override;

void PreCallRecordCmdDrawMultiEXT(VkCommandBuffer commandBuffer, uint32_t drawCount, const VkMultiDrawInfoEXT* pVertexInfo,
                                  uint32_t instanceCount, uint32_t firstInstance, uint32_t stride) override;

void PostCallRecordCmdDrawMultiEXT(VkCommandBuffer commandBuffer, uint32_t drawCount, const VkMultiDrawInfoEXT* pVertexInfo,
                                   uint32_t instanceCount, uint32_t firstInstance, uint32_t stride,
                                   const RecordObject& record_obj) override;

void PreCallRecordCmdDrawMultiIndexedEXT(VkCommandBuffer commandBuffer, uint32_t drawCount,
                                         const VkMultiDrawIndexedInfoEXT* pIndexInfo, uint32_t instanceCount,
                                         uint32_t firstInstance, uint32_t stride, const int32_t* pVertexOffset) override;

void PostCallRecordCmdDrawMultiIndexedEXT(VkCommandBuffer commandBuffer, uint32_t drawCount,
                                          const VkMultiDrawIndexedInfoEXT* pIndexInfo, uint32_t instanceCount,
                                          uint32_t firstInstance, uint32_t stride, const int32_t* pVertexOffset,
                                          const RecordObject& record_obj) override;

void PreCallRecordCreateMicromapEXT(VkDevice device, const VkMicromapCreateInfoEXT* pCreateInfo,
                                    const VkAllocationCallbacks* pAllocator, VkMicromapEXT* pMicromap) override;

void PostCallRecordCreateMicromapEXT(VkDevice device, const VkMicromapCreateInfoEXT* pCreateInfo,
                                     const VkAllocationCallbacks* pAllocator, VkMicromapEXT* pMicromap,
                                     const RecordObject& record_obj) override;

void PreCallRecordDestroyMicromapEXT(VkDevice device, VkMicromapEXT micromap, const VkAllocationCallbacks* pAllocator) override;

void PostCallRecordDestroyMicromapEXT(VkDevice device, VkMicromapEXT micromap, const VkAllocationCallbacks* pAllocator,
                                      const RecordObject& record_obj) override;

void PreCallRecordCmdBuildMicromapsEXT(VkCommandBuffer commandBuffer, uint32_t infoCount,
                                       const VkMicromapBuildInfoEXT* pInfos) override;

void PostCallRecordCmdBuildMicromapsEXT(VkCommandBuffer commandBuffer, uint32_t infoCount, const VkMicromapBuildInfoEXT* pInfos,
                                        const RecordObject& record_obj) override;

void PreCallRecordBuildMicromapsEXT(VkDevice device, VkDeferredOperationKHR deferredOperation, uint32_t infoCount,
                                    const VkMicromapBuildInfoEXT* pInfos) override;

void PostCallRecordBuildMicromapsEXT(VkDevice device, VkDeferredOperationKHR deferredOperation, uint32_t infoCount,
                                     const VkMicromapBuildInfoEXT* pInfos, const RecordObject& record_obj) override;

void PreCallRecordCopyMicromapEXT(VkDevice device, VkDeferredOperationKHR deferredOperation,
                                  const VkCopyMicromapInfoEXT* pInfo) override;

void PostCallRecordCopyMicromapEXT(VkDevice device, VkDeferredOperationKHR deferredOperation, const VkCopyMicromapInfoEXT* pInfo,
                                   const RecordObject& record_obj) override;

void PreCallRecordCopyMicromapToMemoryEXT(VkDevice device, VkDeferredOperationKHR deferredOperation,
                                          const VkCopyMicromapToMemoryInfoEXT* pInfo) override;

void PostCallRecordCopyMicromapToMemoryEXT(VkDevice device, VkDeferredOperationKHR deferredOperation,
                                           const VkCopyMicromapToMemoryInfoEXT* pInfo, const RecordObject& record_obj) override;

void PreCallRecordCopyMemoryToMicromapEXT(VkDevice device, VkDeferredOperationKHR deferredOperation,
                                          const VkCopyMemoryToMicromapInfoEXT* pInfo) override;

void PostCallRecordCopyMemoryToMicromapEXT(VkDevice device, VkDeferredOperationKHR deferredOperation,
                                           const VkCopyMemoryToMicromapInfoEXT* pInfo, const RecordObject& record_obj) override;

void PreCallRecordWriteMicromapsPropertiesEXT(VkDevice device, uint32_t micromapCount, const VkMicromapEXT* pMicromaps,
                                              VkQueryType queryType, size_t dataSize, void* pData, size_t stride) override;

void PostCallRecordWriteMicromapsPropertiesEXT(VkDevice device, uint32_t micromapCount, const VkMicromapEXT* pMicromaps,
                                               VkQueryType queryType, size_t dataSize, void* pData, size_t stride,
                                               const RecordObject& record_obj) override;

void PreCallRecordCmdCopyMicromapEXT(VkCommandBuffer commandBuffer, const VkCopyMicromapInfoEXT* pInfo) override;

void PostCallRecordCmdCopyMicromapEXT(VkCommandBuffer commandBuffer, const VkCopyMicromapInfoEXT* pInfo,
                                      const RecordObject& record_obj) override;

void PreCallRecordCmdCopyMicromapToMemoryEXT(VkCommandBuffer commandBuffer, const VkCopyMicromapToMemoryInfoEXT* pInfo) override;

void PostCallRecordCmdCopyMicromapToMemoryEXT(VkCommandBuffer commandBuffer, const VkCopyMicromapToMemoryInfoEXT* pInfo,
                                              const RecordObject& record_obj) override;

void PreCallRecordCmdCopyMemoryToMicromapEXT(VkCommandBuffer commandBuffer, const VkCopyMemoryToMicromapInfoEXT* pInfo) override;

void PostCallRecordCmdCopyMemoryToMicromapEXT(VkCommandBuffer commandBuffer, const VkCopyMemoryToMicromapInfoEXT* pInfo,
                                              const RecordObject& record_obj) override;

void PreCallRecordCmdWriteMicromapsPropertiesEXT(VkCommandBuffer commandBuffer, uint32_t micromapCount,
                                                 const VkMicromapEXT* pMicromaps, VkQueryType queryType, VkQueryPool queryPool,
                                                 uint32_t firstQuery) override;

void PostCallRecordCmdWriteMicromapsPropertiesEXT(VkCommandBuffer commandBuffer, uint32_t micromapCount,
                                                  const VkMicromapEXT* pMicromaps, VkQueryType queryType, VkQueryPool queryPool,
                                                  uint32_t firstQuery, const RecordObject& record_obj) override;

void PreCallRecordGetDeviceMicromapCompatibilityEXT(VkDevice device, const VkMicromapVersionInfoEXT* pVersionInfo,
                                                    VkAccelerationStructureCompatibilityKHR* pCompatibility) override;

void PostCallRecordGetDeviceMicromapCompatibilityEXT(VkDevice device, const VkMicromapVersionInfoEXT* pVersionInfo,
                                                     VkAccelerationStructureCompatibilityKHR* pCompatibility,
                                                     const RecordObject& record_obj) override;

void PreCallRecordGetMicromapBuildSizesEXT(VkDevice device, VkAccelerationStructureBuildTypeKHR buildType,
                                           const VkMicromapBuildInfoEXT* pBuildInfo,
                                           VkMicromapBuildSizesInfoEXT* pSizeInfo) override;

void PostCallRecordGetMicromapBuildSizesEXT(VkDevice device, VkAccelerationStructureBuildTypeKHR buildType,
                                            const VkMicromapBuildInfoEXT* pBuildInfo, VkMicromapBuildSizesInfoEXT* pSizeInfo,
                                            const RecordObject& record_obj) override;

void PreCallRecordCmdDrawClusterHUAWEI(VkCommandBuffer commandBuffer, uint32_t groupCountX, uint32_t groupCountY,
                                       uint32_t groupCountZ) override;

void PostCallRecordCmdDrawClusterHUAWEI(VkCommandBuffer commandBuffer, uint32_t groupCountX, uint32_t groupCountY,
                                        uint32_t groupCountZ, const RecordObject& record_obj) override;

void PreCallRecordCmdDrawClusterIndirectHUAWEI(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset) override;

void PostCallRecordCmdDrawClusterIndirectHUAWEI(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                const RecordObject& record_obj) override;

void PreCallRecordSetDeviceMemoryPriorityEXT(VkDevice device, VkDeviceMemory memory, float priority) override;

void PostCallRecordSetDeviceMemoryPriorityEXT(VkDevice device, VkDeviceMemory memory, float priority,
                                              const RecordObject& record_obj) override;

void PreCallRecordGetDescriptorSetLayoutHostMappingInfoVALVE(VkDevice device,
                                                             const VkDescriptorSetBindingReferenceVALVE* pBindingReference,
                                                             VkDescriptorSetLayoutHostMappingInfoVALVE* pHostMapping) override;

void PostCallRecordGetDescriptorSetLayoutHostMappingInfoVALVE(VkDevice device,
                                                              const VkDescriptorSetBindingReferenceVALVE* pBindingReference,
                                                              VkDescriptorSetLayoutHostMappingInfoVALVE* pHostMapping,
                                                              const RecordObject& record_obj) override;

void PreCallRecordGetDescriptorSetHostMappingVALVE(VkDevice device, VkDescriptorSet descriptorSet, void** ppData) override;

void PostCallRecordGetDescriptorSetHostMappingVALVE(VkDevice device, VkDescriptorSet descriptorSet, void** ppData,
                                                    const RecordObject& record_obj) override;

void PreCallRecordCmdCopyMemoryIndirectNV(VkCommandBuffer commandBuffer, VkDeviceAddress copyBufferAddress, uint32_t copyCount,
                                          uint32_t stride) override;

void PostCallRecordCmdCopyMemoryIndirectNV(VkCommandBuffer commandBuffer, VkDeviceAddress copyBufferAddress, uint32_t copyCount,
                                           uint32_t stride, const RecordObject& record_obj) override;

void PreCallRecordCmdCopyMemoryToImageIndirectNV(VkCommandBuffer commandBuffer, VkDeviceAddress copyBufferAddress,
                                                 uint32_t copyCount, uint32_t stride, VkImage dstImage,
                                                 VkImageLayout dstImageLayout,
                                                 const VkImageSubresourceLayers* pImageSubresources) override;

void PostCallRecordCmdCopyMemoryToImageIndirectNV(VkCommandBuffer commandBuffer, VkDeviceAddress copyBufferAddress,
                                                  uint32_t copyCount, uint32_t stride, VkImage dstImage,
                                                  VkImageLayout dstImageLayout, const VkImageSubresourceLayers* pImageSubresources,
                                                  const RecordObject& record_obj) override;

void PreCallRecordCmdDecompressMemoryNV(VkCommandBuffer commandBuffer, uint32_t decompressRegionCount,
                                        const VkDecompressMemoryRegionNV* pDecompressMemoryRegions) override;

void PostCallRecordCmdDecompressMemoryNV(VkCommandBuffer commandBuffer, uint32_t decompressRegionCount,
                                         const VkDecompressMemoryRegionNV* pDecompressMemoryRegions,
                                         const RecordObject& record_obj) override;

void PreCallRecordCmdDecompressMemoryIndirectCountNV(VkCommandBuffer commandBuffer, VkDeviceAddress indirectCommandsAddress,
                                                     VkDeviceAddress indirectCommandsCountAddress, uint32_t stride) override;

void PostCallRecordCmdDecompressMemoryIndirectCountNV(VkCommandBuffer commandBuffer, VkDeviceAddress indirectCommandsAddress,
                                                      VkDeviceAddress indirectCommandsCountAddress, uint32_t stride,
                                                      const RecordObject& record_obj) override;

void PreCallRecordGetPipelineIndirectMemoryRequirementsNV(VkDevice device, const VkComputePipelineCreateInfo* pCreateInfo,
                                                          VkMemoryRequirements2* pMemoryRequirements) override;

void PostCallRecordGetPipelineIndirectMemoryRequirementsNV(VkDevice device, const VkComputePipelineCreateInfo* pCreateInfo,
                                                           VkMemoryRequirements2* pMemoryRequirements,
                                                           const RecordObject& record_obj) override;

void PreCallRecordCmdUpdatePipelineIndirectBufferNV(VkCommandBuffer commandBuffer, VkPipelineBindPoint pipelineBindPoint,
                                                    VkPipeline pipeline) override;

void PostCallRecordCmdUpdatePipelineIndirectBufferNV(VkCommandBuffer commandBuffer, VkPipelineBindPoint pipelineBindPoint,
                                                     VkPipeline pipeline, const RecordObject& record_obj) override;

void PreCallRecordGetPipelineIndirectDeviceAddressNV(VkDevice device, const VkPipelineIndirectDeviceAddressInfoNV* pInfo) override;

void PostCallRecordGetPipelineIndirectDeviceAddressNV(VkDevice device, const VkPipelineIndirectDeviceAddressInfoNV* pInfo,
                                                      const RecordObject& record_obj) override;

void PreCallRecordCmdSetTessellationDomainOriginEXT(VkCommandBuffer commandBuffer,
                                                    VkTessellationDomainOrigin domainOrigin) override;

void PostCallRecordCmdSetTessellationDomainOriginEXT(VkCommandBuffer commandBuffer, VkTessellationDomainOrigin domainOrigin,
                                                     const RecordObject& record_obj) override;

void PreCallRecordCmdSetDepthClampEnableEXT(VkCommandBuffer commandBuffer, VkBool32 depthClampEnable) override;

void PostCallRecordCmdSetDepthClampEnableEXT(VkCommandBuffer commandBuffer, VkBool32 depthClampEnable,
                                             const RecordObject& record_obj) override;

void PreCallRecordCmdSetPolygonModeEXT(VkCommandBuffer commandBuffer, VkPolygonMode polygonMode) override;

void PostCallRecordCmdSetPolygonModeEXT(VkCommandBuffer commandBuffer, VkPolygonMode polygonMode,
                                        const RecordObject& record_obj) override;

void PreCallRecordCmdSetRasterizationSamplesEXT(VkCommandBuffer commandBuffer, VkSampleCountFlagBits rasterizationSamples) override;

void PostCallRecordCmdSetRasterizationSamplesEXT(VkCommandBuffer commandBuffer, VkSampleCountFlagBits rasterizationSamples,
                                                 const RecordObject& record_obj) override;

void PreCallRecordCmdSetSampleMaskEXT(VkCommandBuffer commandBuffer, VkSampleCountFlagBits samples,
                                      const VkSampleMask* pSampleMask) override;

void PostCallRecordCmdSetSampleMaskEXT(VkCommandBuffer commandBuffer, VkSampleCountFlagBits samples,
                                       const VkSampleMask* pSampleMask, const RecordObject& record_obj) override;

void PreCallRecordCmdSetAlphaToCoverageEnableEXT(VkCommandBuffer commandBuffer, VkBool32 alphaToCoverageEnable) override;

void PostCallRecordCmdSetAlphaToCoverageEnableEXT(VkCommandBuffer commandBuffer, VkBool32 alphaToCoverageEnable,
                                                  const RecordObject& record_obj) override;

void PreCallRecordCmdSetAlphaToOneEnableEXT(VkCommandBuffer commandBuffer, VkBool32 alphaToOneEnable) override;

void PostCallRecordCmdSetAlphaToOneEnableEXT(VkCommandBuffer commandBuffer, VkBool32 alphaToOneEnable,
                                             const RecordObject& record_obj) override;

void PreCallRecordCmdSetLogicOpEnableEXT(VkCommandBuffer commandBuffer, VkBool32 logicOpEnable) override;

void PostCallRecordCmdSetLogicOpEnableEXT(VkCommandBuffer commandBuffer, VkBool32 logicOpEnable,
                                          const RecordObject& record_obj) override;

void PreCallRecordCmdSetColorBlendEnableEXT(VkCommandBuffer commandBuffer, uint32_t firstAttachment, uint32_t attachmentCount,
                                            const VkBool32* pColorBlendEnables) override;

void PostCallRecordCmdSetColorBlendEnableEXT(VkCommandBuffer commandBuffer, uint32_t firstAttachment, uint32_t attachmentCount,
                                             const VkBool32* pColorBlendEnables, const RecordObject& record_obj) override;

void PreCallRecordCmdSetColorBlendEquationEXT(VkCommandBuffer commandBuffer, uint32_t firstAttachment, uint32_t attachmentCount,
                                              const VkColorBlendEquationEXT* pColorBlendEquations) override;

void PostCallRecordCmdSetColorBlendEquationEXT(VkCommandBuffer commandBuffer, uint32_t firstAttachment, uint32_t attachmentCount,
                                               const VkColorBlendEquationEXT* pColorBlendEquations,
                                               const RecordObject& record_obj) override;

void PreCallRecordCmdSetColorWriteMaskEXT(VkCommandBuffer commandBuffer, uint32_t firstAttachment, uint32_t attachmentCount,
                                          const VkColorComponentFlags* pColorWriteMasks) override;

void PostCallRecordCmdSetColorWriteMaskEXT(VkCommandBuffer commandBuffer, uint32_t firstAttachment, uint32_t attachmentCount,
                                           const VkColorComponentFlags* pColorWriteMasks, const RecordObject& record_obj) override;

void PreCallRecordCmdSetRasterizationStreamEXT(VkCommandBuffer commandBuffer, uint32_t rasterizationStream) override;

void PostCallRecordCmdSetRasterizationStreamEXT(VkCommandBuffer commandBuffer, uint32_t rasterizationStream,
                                                const RecordObject& record_obj) override;

void PreCallRecordCmdSetConservativeRasterizationModeEXT(VkCommandBuffer commandBuffer,
                                                         VkConservativeRasterizationModeEXT conservativeRasterizationMode) override;

void PostCallRecordCmdSetConservativeRasterizationModeEXT(VkCommandBuffer commandBuffer,
                                                          VkConservativeRasterizationModeEXT conservativeRasterizationMode,
                                                          const RecordObject& record_obj) override;

void PreCallRecordCmdSetExtraPrimitiveOverestimationSizeEXT(VkCommandBuffer commandBuffer,
                                                            float extraPrimitiveOverestimationSize) override;

void PostCallRecordCmdSetExtraPrimitiveOverestimationSizeEXT(VkCommandBuffer commandBuffer, float extraPrimitiveOverestimationSize,
                                                             const RecordObject& record_obj) override;

void PreCallRecordCmdSetDepthClipEnableEXT(VkCommandBuffer commandBuffer, VkBool32 depthClipEnable) override;

void PostCallRecordCmdSetDepthClipEnableEXT(VkCommandBuffer commandBuffer, VkBool32 depthClipEnable,
                                            const RecordObject& record_obj) override;

void PreCallRecordCmdSetSampleLocationsEnableEXT(VkCommandBuffer commandBuffer, VkBool32 sampleLocationsEnable) override;

void PostCallRecordCmdSetSampleLocationsEnableEXT(VkCommandBuffer commandBuffer, VkBool32 sampleLocationsEnable,
                                                  const RecordObject& record_obj) override;

void PreCallRecordCmdSetColorBlendAdvancedEXT(VkCommandBuffer commandBuffer, uint32_t firstAttachment, uint32_t attachmentCount,
                                              const VkColorBlendAdvancedEXT* pColorBlendAdvanced) override;

void PostCallRecordCmdSetColorBlendAdvancedEXT(VkCommandBuffer commandBuffer, uint32_t firstAttachment, uint32_t attachmentCount,
                                               const VkColorBlendAdvancedEXT* pColorBlendAdvanced,
                                               const RecordObject& record_obj) override;

void PreCallRecordCmdSetProvokingVertexModeEXT(VkCommandBuffer commandBuffer,
                                               VkProvokingVertexModeEXT provokingVertexMode) override;

void PostCallRecordCmdSetProvokingVertexModeEXT(VkCommandBuffer commandBuffer, VkProvokingVertexModeEXT provokingVertexMode,
                                                const RecordObject& record_obj) override;

void PreCallRecordCmdSetLineRasterizationModeEXT(VkCommandBuffer commandBuffer,
                                                 VkLineRasterizationModeEXT lineRasterizationMode) override;

void PostCallRecordCmdSetLineRasterizationModeEXT(VkCommandBuffer commandBuffer, VkLineRasterizationModeEXT lineRasterizationMode,
                                                  const RecordObject& record_obj) override;

void PreCallRecordCmdSetLineStippleEnableEXT(VkCommandBuffer commandBuffer, VkBool32 stippledLineEnable) override;

void PostCallRecordCmdSetLineStippleEnableEXT(VkCommandBuffer commandBuffer, VkBool32 stippledLineEnable,
                                              const RecordObject& record_obj) override;

void PreCallRecordCmdSetDepthClipNegativeOneToOneEXT(VkCommandBuffer commandBuffer, VkBool32 negativeOneToOne) override;

void PostCallRecordCmdSetDepthClipNegativeOneToOneEXT(VkCommandBuffer commandBuffer, VkBool32 negativeOneToOne,
                                                      const RecordObject& record_obj) override;

void PreCallRecordCmdSetViewportWScalingEnableNV(VkCommandBuffer commandBuffer, VkBool32 viewportWScalingEnable) override;

void PostCallRecordCmdSetViewportWScalingEnableNV(VkCommandBuffer commandBuffer, VkBool32 viewportWScalingEnable,
                                                  const RecordObject& record_obj) override;

void PreCallRecordCmdSetViewportSwizzleNV(VkCommandBuffer commandBuffer, uint32_t firstViewport, uint32_t viewportCount,
                                          const VkViewportSwizzleNV* pViewportSwizzles) override;

void PostCallRecordCmdSetViewportSwizzleNV(VkCommandBuffer commandBuffer, uint32_t firstViewport, uint32_t viewportCount,
                                           const VkViewportSwizzleNV* pViewportSwizzles, const RecordObject& record_obj) override;

void PreCallRecordCmdSetCoverageToColorEnableNV(VkCommandBuffer commandBuffer, VkBool32 coverageToColorEnable) override;

void PostCallRecordCmdSetCoverageToColorEnableNV(VkCommandBuffer commandBuffer, VkBool32 coverageToColorEnable,
                                                 const RecordObject& record_obj) override;

void PreCallRecordCmdSetCoverageToColorLocationNV(VkCommandBuffer commandBuffer, uint32_t coverageToColorLocation) override;

void PostCallRecordCmdSetCoverageToColorLocationNV(VkCommandBuffer commandBuffer, uint32_t coverageToColorLocation,
                                                   const RecordObject& record_obj) override;

void PreCallRecordCmdSetCoverageModulationModeNV(VkCommandBuffer commandBuffer,
                                                 VkCoverageModulationModeNV coverageModulationMode) override;

void PostCallRecordCmdSetCoverageModulationModeNV(VkCommandBuffer commandBuffer, VkCoverageModulationModeNV coverageModulationMode,
                                                  const RecordObject& record_obj) override;

void PreCallRecordCmdSetCoverageModulationTableEnableNV(VkCommandBuffer commandBuffer,
                                                        VkBool32 coverageModulationTableEnable) override;

void PostCallRecordCmdSetCoverageModulationTableEnableNV(VkCommandBuffer commandBuffer, VkBool32 coverageModulationTableEnable,
                                                         const RecordObject& record_obj) override;

void PreCallRecordCmdSetCoverageModulationTableNV(VkCommandBuffer commandBuffer, uint32_t coverageModulationTableCount,
                                                  const float* pCoverageModulationTable) override;

void PostCallRecordCmdSetCoverageModulationTableNV(VkCommandBuffer commandBuffer, uint32_t coverageModulationTableCount,
                                                   const float* pCoverageModulationTable, const RecordObject& record_obj) override;

void PreCallRecordCmdSetShadingRateImageEnableNV(VkCommandBuffer commandBuffer, VkBool32 shadingRateImageEnable) override;

void PostCallRecordCmdSetShadingRateImageEnableNV(VkCommandBuffer commandBuffer, VkBool32 shadingRateImageEnable,
                                                  const RecordObject& record_obj) override;

void PreCallRecordCmdSetRepresentativeFragmentTestEnableNV(VkCommandBuffer commandBuffer,
                                                           VkBool32 representativeFragmentTestEnable) override;

void PostCallRecordCmdSetRepresentativeFragmentTestEnableNV(VkCommandBuffer commandBuffer,
                                                            VkBool32 representativeFragmentTestEnable,
                                                            const RecordObject& record_obj) override;

void PreCallRecordCmdSetCoverageReductionModeNV(VkCommandBuffer commandBuffer,
                                                VkCoverageReductionModeNV coverageReductionMode) override;

void PostCallRecordCmdSetCoverageReductionModeNV(VkCommandBuffer commandBuffer, VkCoverageReductionModeNV coverageReductionMode,
                                                 const RecordObject& record_obj) override;

void PreCallRecordGetShaderModuleIdentifierEXT(VkDevice device, VkShaderModule shaderModule,
                                               VkShaderModuleIdentifierEXT* pIdentifier) override;

void PostCallRecordGetShaderModuleIdentifierEXT(VkDevice device, VkShaderModule shaderModule,
                                                VkShaderModuleIdentifierEXT* pIdentifier, const RecordObject& record_obj) override;

void PreCallRecordGetShaderModuleCreateInfoIdentifierEXT(VkDevice device, const VkShaderModuleCreateInfo* pCreateInfo,
                                                         VkShaderModuleIdentifierEXT* pIdentifier) override;

void PostCallRecordGetShaderModuleCreateInfoIdentifierEXT(VkDevice device, const VkShaderModuleCreateInfo* pCreateInfo,
                                                          VkShaderModuleIdentifierEXT* pIdentifier,
                                                          const RecordObject& record_obj) override;

void PreCallRecordCreateOpticalFlowSessionNV(VkDevice device, const VkOpticalFlowSessionCreateInfoNV* pCreateInfo,
                                             const VkAllocationCallbacks* pAllocator, VkOpticalFlowSessionNV* pSession) override;

void PostCallRecordCreateOpticalFlowSessionNV(VkDevice device, const VkOpticalFlowSessionCreateInfoNV* pCreateInfo,
                                              const VkAllocationCallbacks* pAllocator, VkOpticalFlowSessionNV* pSession,
                                              const RecordObject& record_obj) override;

void PreCallRecordDestroyOpticalFlowSessionNV(VkDevice device, VkOpticalFlowSessionNV session,
                                              const VkAllocationCallbacks* pAllocator) override;

void PostCallRecordDestroyOpticalFlowSessionNV(VkDevice device, VkOpticalFlowSessionNV session,
                                               const VkAllocationCallbacks* pAllocator, const RecordObject& record_obj) override;

void PreCallRecordBindOpticalFlowSessionImageNV(VkDevice device, VkOpticalFlowSessionNV session,
                                                VkOpticalFlowSessionBindingPointNV bindingPoint, VkImageView view,
                                                VkImageLayout layout) override;

void PostCallRecordBindOpticalFlowSessionImageNV(VkDevice device, VkOpticalFlowSessionNV session,
                                                 VkOpticalFlowSessionBindingPointNV bindingPoint, VkImageView view,
                                                 VkImageLayout layout, const RecordObject& record_obj) override;

void PreCallRecordCmdOpticalFlowExecuteNV(VkCommandBuffer commandBuffer, VkOpticalFlowSessionNV session,
                                          const VkOpticalFlowExecuteInfoNV* pExecuteInfo) override;

void PostCallRecordCmdOpticalFlowExecuteNV(VkCommandBuffer commandBuffer, VkOpticalFlowSessionNV session,
                                           const VkOpticalFlowExecuteInfoNV* pExecuteInfo, const RecordObject& record_obj) override;

void PreCallRecordCreateShadersEXT(VkDevice device, uint32_t createInfoCount, const VkShaderCreateInfoEXT* pCreateInfos,
                                   const VkAllocationCallbacks* pAllocator, VkShaderEXT* pShaders) override;

void PostCallRecordCreateShadersEXT(VkDevice device, uint32_t createInfoCount, const VkShaderCreateInfoEXT* pCreateInfos,
                                    const VkAllocationCallbacks* pAllocator, VkShaderEXT* pShaders,
                                    const RecordObject& record_obj) override;

void PreCallRecordDestroyShaderEXT(VkDevice device, VkShaderEXT shader, const VkAllocationCallbacks* pAllocator) override;

void PostCallRecordDestroyShaderEXT(VkDevice device, VkShaderEXT shader, const VkAllocationCallbacks* pAllocator,
                                    const RecordObject& record_obj) override;

void PreCallRecordGetShaderBinaryDataEXT(VkDevice device, VkShaderEXT shader, size_t* pDataSize, void* pData) override;

void PostCallRecordGetShaderBinaryDataEXT(VkDevice device, VkShaderEXT shader, size_t* pDataSize, void* pData,
                                          const RecordObject& record_obj) override;

void PreCallRecordCmdBindShadersEXT(VkCommandBuffer commandBuffer, uint32_t stageCount, const VkShaderStageFlagBits* pStages,
                                    const VkShaderEXT* pShaders) override;

void PostCallRecordCmdBindShadersEXT(VkCommandBuffer commandBuffer, uint32_t stageCount, const VkShaderStageFlagBits* pStages,
                                     const VkShaderEXT* pShaders, const RecordObject& record_obj) override;

void PreCallRecordGetFramebufferTilePropertiesQCOM(VkDevice device, VkFramebuffer framebuffer, uint32_t* pPropertiesCount,
                                                   VkTilePropertiesQCOM* pProperties) override;

void PostCallRecordGetFramebufferTilePropertiesQCOM(VkDevice device, VkFramebuffer framebuffer, uint32_t* pPropertiesCount,
                                                    VkTilePropertiesQCOM* pProperties, const RecordObject& record_obj) override;

void PreCallRecordGetDynamicRenderingTilePropertiesQCOM(VkDevice device, const VkRenderingInfo* pRenderingInfo,
                                                        VkTilePropertiesQCOM* pProperties) override;

void PostCallRecordGetDynamicRenderingTilePropertiesQCOM(VkDevice device, const VkRenderingInfo* pRenderingInfo,
                                                         VkTilePropertiesQCOM* pProperties,
                                                         const RecordObject& record_obj) override;

void PreCallRecordSetLatencySleepModeNV(VkDevice device, VkSwapchainKHR swapchain,
                                        const VkLatencySleepModeInfoNV* pSleepModeInfo) override;

void PostCallRecordSetLatencySleepModeNV(VkDevice device, VkSwapchainKHR swapchain, const VkLatencySleepModeInfoNV* pSleepModeInfo,
                                         const RecordObject& record_obj) override;

void PreCallRecordLatencySleepNV(VkDevice device, VkSwapchainKHR swapchain, const VkLatencySleepInfoNV* pSleepInfo) override;

void PostCallRecordLatencySleepNV(VkDevice device, VkSwapchainKHR swapchain, const VkLatencySleepInfoNV* pSleepInfo,
                                  const RecordObject& record_obj) override;

void PreCallRecordSetLatencyMarkerNV(VkDevice device, VkSwapchainKHR swapchain,
                                     const VkSetLatencyMarkerInfoNV* pLatencyMarkerInfo) override;

void PostCallRecordSetLatencyMarkerNV(VkDevice device, VkSwapchainKHR swapchain, const VkSetLatencyMarkerInfoNV* pLatencyMarkerInfo,
                                      const RecordObject& record_obj) override;

void PreCallRecordGetLatencyTimingsNV(VkDevice device, VkSwapchainKHR swapchain, uint32_t* pTimingCount,
                                      VkGetLatencyMarkerInfoNV* pLatencyMarkerInfo) override;

void PostCallRecordGetLatencyTimingsNV(VkDevice device, VkSwapchainKHR swapchain, uint32_t* pTimingCount,
                                       VkGetLatencyMarkerInfoNV* pLatencyMarkerInfo, const RecordObject& record_obj) override;

void PreCallRecordQueueNotifyOutOfBandNV(VkQueue queue, const VkOutOfBandQueueTypeInfoNV* pQueueTypeInfo) override;

void PostCallRecordQueueNotifyOutOfBandNV(VkQueue queue, const VkOutOfBandQueueTypeInfoNV* pQueueTypeInfo,
                                          const RecordObject& record_obj) override;

void PreCallRecordCmdSetAttachmentFeedbackLoopEnableEXT(VkCommandBuffer commandBuffer, VkImageAspectFlags aspectMask) override;

void PostCallRecordCmdSetAttachmentFeedbackLoopEnableEXT(VkCommandBuffer commandBuffer, VkImageAspectFlags aspectMask,
                                                         const RecordObject& record_obj) override;

#ifdef VK_USE_PLATFORM_SCREEN_QNX
void PreCallRecordGetScreenBufferPropertiesQNX(VkDevice device, const struct _screen_buffer* buffer,
                                               VkScreenBufferPropertiesQNX* pProperties) override;

void PostCallRecordGetScreenBufferPropertiesQNX(VkDevice device, const struct _screen_buffer* buffer,
                                                VkScreenBufferPropertiesQNX* pProperties, const RecordObject& record_obj) override;

#endif  // VK_USE_PLATFORM_SCREEN_QNX
void PreCallRecordCreateAccelerationStructureKHR(VkDevice device, const VkAccelerationStructureCreateInfoKHR* pCreateInfo,
                                                 const VkAllocationCallbacks* pAllocator,
                                                 VkAccelerationStructureKHR* pAccelerationStructure) override;

void PostCallRecordCreateAccelerationStructureKHR(VkDevice device, const VkAccelerationStructureCreateInfoKHR* pCreateInfo,
                                                  const VkAllocationCallbacks* pAllocator,
                                                  VkAccelerationStructureKHR* pAccelerationStructure,
                                                  const RecordObject& record_obj) override;

void PreCallRecordDestroyAccelerationStructureKHR(VkDevice device, VkAccelerationStructureKHR accelerationStructure,
                                                  const VkAllocationCallbacks* pAllocator) override;

void PostCallRecordDestroyAccelerationStructureKHR(VkDevice device, VkAccelerationStructureKHR accelerationStructure,
                                                   const VkAllocationCallbacks* pAllocator,
                                                   const RecordObject& record_obj) override;

void PreCallRecordCmdBuildAccelerationStructuresKHR(
    VkCommandBuffer commandBuffer, uint32_t infoCount, const VkAccelerationStructureBuildGeometryInfoKHR* pInfos,
    const VkAccelerationStructureBuildRangeInfoKHR* const* ppBuildRangeInfos) override;

void PostCallRecordCmdBuildAccelerationStructuresKHR(VkCommandBuffer commandBuffer, uint32_t infoCount,
                                                     const VkAccelerationStructureBuildGeometryInfoKHR* pInfos,
                                                     const VkAccelerationStructureBuildRangeInfoKHR* const* ppBuildRangeInfos,
                                                     const RecordObject& record_obj) override;

void PreCallRecordCmdBuildAccelerationStructuresIndirectKHR(VkCommandBuffer commandBuffer, uint32_t infoCount,
                                                            const VkAccelerationStructureBuildGeometryInfoKHR* pInfos,
                                                            const VkDeviceAddress* pIndirectDeviceAddresses,
                                                            const uint32_t* pIndirectStrides,
                                                            const uint32_t* const* ppMaxPrimitiveCounts) override;

void PostCallRecordCmdBuildAccelerationStructuresIndirectKHR(VkCommandBuffer commandBuffer, uint32_t infoCount,
                                                             const VkAccelerationStructureBuildGeometryInfoKHR* pInfos,
                                                             const VkDeviceAddress* pIndirectDeviceAddresses,
                                                             const uint32_t* pIndirectStrides,
                                                             const uint32_t* const* ppMaxPrimitiveCounts,
                                                             const RecordObject& record_obj) override;

void PreCallRecordBuildAccelerationStructuresKHR(VkDevice device, VkDeferredOperationKHR deferredOperation, uint32_t infoCount,
                                                 const VkAccelerationStructureBuildGeometryInfoKHR* pInfos,
                                                 const VkAccelerationStructureBuildRangeInfoKHR* const* ppBuildRangeInfos) override;

void PostCallRecordBuildAccelerationStructuresKHR(VkDevice device, VkDeferredOperationKHR deferredOperation, uint32_t infoCount,
                                                  const VkAccelerationStructureBuildGeometryInfoKHR* pInfos,
                                                  const VkAccelerationStructureBuildRangeInfoKHR* const* ppBuildRangeInfos,
                                                  const RecordObject& record_obj) override;

void PreCallRecordCopyAccelerationStructureKHR(VkDevice device, VkDeferredOperationKHR deferredOperation,
                                               const VkCopyAccelerationStructureInfoKHR* pInfo) override;

void PostCallRecordCopyAccelerationStructureKHR(VkDevice device, VkDeferredOperationKHR deferredOperation,
                                                const VkCopyAccelerationStructureInfoKHR* pInfo,
                                                const RecordObject& record_obj) override;

void PreCallRecordCopyAccelerationStructureToMemoryKHR(VkDevice device, VkDeferredOperationKHR deferredOperation,
                                                       const VkCopyAccelerationStructureToMemoryInfoKHR* pInfo) override;

void PostCallRecordCopyAccelerationStructureToMemoryKHR(VkDevice device, VkDeferredOperationKHR deferredOperation,
                                                        const VkCopyAccelerationStructureToMemoryInfoKHR* pInfo,
                                                        const RecordObject& record_obj) override;

void PreCallRecordCopyMemoryToAccelerationStructureKHR(VkDevice device, VkDeferredOperationKHR deferredOperation,
                                                       const VkCopyMemoryToAccelerationStructureInfoKHR* pInfo) override;

void PostCallRecordCopyMemoryToAccelerationStructureKHR(VkDevice device, VkDeferredOperationKHR deferredOperation,
                                                        const VkCopyMemoryToAccelerationStructureInfoKHR* pInfo,
                                                        const RecordObject& record_obj) override;

void PreCallRecordWriteAccelerationStructuresPropertiesKHR(VkDevice device, uint32_t accelerationStructureCount,
                                                           const VkAccelerationStructureKHR* pAccelerationStructures,
                                                           VkQueryType queryType, size_t dataSize, void* pData,
                                                           size_t stride) override;

void PostCallRecordWriteAccelerationStructuresPropertiesKHR(VkDevice device, uint32_t accelerationStructureCount,
                                                            const VkAccelerationStructureKHR* pAccelerationStructures,
                                                            VkQueryType queryType, size_t dataSize, void* pData, size_t stride,
                                                            const RecordObject& record_obj) override;

void PreCallRecordCmdCopyAccelerationStructureKHR(VkCommandBuffer commandBuffer,
                                                  const VkCopyAccelerationStructureInfoKHR* pInfo) override;

void PostCallRecordCmdCopyAccelerationStructureKHR(VkCommandBuffer commandBuffer, const VkCopyAccelerationStructureInfoKHR* pInfo,
                                                   const RecordObject& record_obj) override;

void PreCallRecordCmdCopyAccelerationStructureToMemoryKHR(VkCommandBuffer commandBuffer,
                                                          const VkCopyAccelerationStructureToMemoryInfoKHR* pInfo) override;

void PostCallRecordCmdCopyAccelerationStructureToMemoryKHR(VkCommandBuffer commandBuffer,
                                                           const VkCopyAccelerationStructureToMemoryInfoKHR* pInfo,
                                                           const RecordObject& record_obj) override;

void PreCallRecordCmdCopyMemoryToAccelerationStructureKHR(VkCommandBuffer commandBuffer,
                                                          const VkCopyMemoryToAccelerationStructureInfoKHR* pInfo) override;

void PostCallRecordCmdCopyMemoryToAccelerationStructureKHR(VkCommandBuffer commandBuffer,
                                                           const VkCopyMemoryToAccelerationStructureInfoKHR* pInfo,
                                                           const RecordObject& record_obj) override;

void PreCallRecordGetAccelerationStructureDeviceAddressKHR(VkDevice device,
                                                           const VkAccelerationStructureDeviceAddressInfoKHR* pInfo) override;

void PostCallRecordGetAccelerationStructureDeviceAddressKHR(VkDevice device,
                                                            const VkAccelerationStructureDeviceAddressInfoKHR* pInfo,
                                                            const RecordObject& record_obj) override;

void PreCallRecordCmdWriteAccelerationStructuresPropertiesKHR(VkCommandBuffer commandBuffer, uint32_t accelerationStructureCount,
                                                              const VkAccelerationStructureKHR* pAccelerationStructures,
                                                              VkQueryType queryType, VkQueryPool queryPool,
                                                              uint32_t firstQuery) override;

void PostCallRecordCmdWriteAccelerationStructuresPropertiesKHR(VkCommandBuffer commandBuffer, uint32_t accelerationStructureCount,
                                                               const VkAccelerationStructureKHR* pAccelerationStructures,
                                                               VkQueryType queryType, VkQueryPool queryPool, uint32_t firstQuery,
                                                               const RecordObject& record_obj) override;

void PreCallRecordGetDeviceAccelerationStructureCompatibilityKHR(VkDevice device,
                                                                 const VkAccelerationStructureVersionInfoKHR* pVersionInfo,
                                                                 VkAccelerationStructureCompatibilityKHR* pCompatibility) override;

void PostCallRecordGetDeviceAccelerationStructureCompatibilityKHR(VkDevice device,
                                                                  const VkAccelerationStructureVersionInfoKHR* pVersionInfo,
                                                                  VkAccelerationStructureCompatibilityKHR* pCompatibility,
                                                                  const RecordObject& record_obj) override;

void PreCallRecordGetAccelerationStructureBuildSizesKHR(VkDevice device, VkAccelerationStructureBuildTypeKHR buildType,
                                                        const VkAccelerationStructureBuildGeometryInfoKHR* pBuildInfo,
                                                        const uint32_t* pMaxPrimitiveCounts,
                                                        VkAccelerationStructureBuildSizesInfoKHR* pSizeInfo) override;

void PostCallRecordGetAccelerationStructureBuildSizesKHR(VkDevice device, VkAccelerationStructureBuildTypeKHR buildType,
                                                         const VkAccelerationStructureBuildGeometryInfoKHR* pBuildInfo,
                                                         const uint32_t* pMaxPrimitiveCounts,
                                                         VkAccelerationStructureBuildSizesInfoKHR* pSizeInfo,
                                                         const RecordObject& record_obj) override;

void PreCallRecordCmdTraceRaysKHR(VkCommandBuffer commandBuffer, const VkStridedDeviceAddressRegionKHR* pRaygenShaderBindingTable,
                                  const VkStridedDeviceAddressRegionKHR* pMissShaderBindingTable,
                                  const VkStridedDeviceAddressRegionKHR* pHitShaderBindingTable,
                                  const VkStridedDeviceAddressRegionKHR* pCallableShaderBindingTable, uint32_t width,
                                  uint32_t height, uint32_t depth) override;

void PostCallRecordCmdTraceRaysKHR(VkCommandBuffer commandBuffer, const VkStridedDeviceAddressRegionKHR* pRaygenShaderBindingTable,
                                   const VkStridedDeviceAddressRegionKHR* pMissShaderBindingTable,
                                   const VkStridedDeviceAddressRegionKHR* pHitShaderBindingTable,
                                   const VkStridedDeviceAddressRegionKHR* pCallableShaderBindingTable, uint32_t width,
                                   uint32_t height, uint32_t depth, const RecordObject& record_obj) override;

void PreCallRecordCreateRayTracingPipelinesKHR(VkDevice device, VkDeferredOperationKHR deferredOperation,
                                               VkPipelineCache pipelineCache, uint32_t createInfoCount,
                                               const VkRayTracingPipelineCreateInfoKHR* pCreateInfos,
                                               const VkAllocationCallbacks* pAllocator, VkPipeline* pPipelines) override;

void PostCallRecordCreateRayTracingPipelinesKHR(VkDevice device, VkDeferredOperationKHR deferredOperation,
                                                VkPipelineCache pipelineCache, uint32_t createInfoCount,
                                                const VkRayTracingPipelineCreateInfoKHR* pCreateInfos,
                                                const VkAllocationCallbacks* pAllocator, VkPipeline* pPipelines,
                                                const RecordObject& record_obj) override;

void PreCallRecordGetRayTracingCaptureReplayShaderGroupHandlesKHR(VkDevice device, VkPipeline pipeline, uint32_t firstGroup,
                                                                  uint32_t groupCount, size_t dataSize, void* pData) override;

void PostCallRecordGetRayTracingCaptureReplayShaderGroupHandlesKHR(VkDevice device, VkPipeline pipeline, uint32_t firstGroup,
                                                                   uint32_t groupCount, size_t dataSize, void* pData,
                                                                   const RecordObject& record_obj) override;

void PreCallRecordCmdTraceRaysIndirectKHR(VkCommandBuffer commandBuffer,
                                          const VkStridedDeviceAddressRegionKHR* pRaygenShaderBindingTable,
                                          const VkStridedDeviceAddressRegionKHR* pMissShaderBindingTable,
                                          const VkStridedDeviceAddressRegionKHR* pHitShaderBindingTable,
                                          const VkStridedDeviceAddressRegionKHR* pCallableShaderBindingTable,
                                          VkDeviceAddress indirectDeviceAddress) override;

void PostCallRecordCmdTraceRaysIndirectKHR(VkCommandBuffer commandBuffer,
                                           const VkStridedDeviceAddressRegionKHR* pRaygenShaderBindingTable,
                                           const VkStridedDeviceAddressRegionKHR* pMissShaderBindingTable,
                                           const VkStridedDeviceAddressRegionKHR* pHitShaderBindingTable,
                                           const VkStridedDeviceAddressRegionKHR* pCallableShaderBindingTable,
                                           VkDeviceAddress indirectDeviceAddress, const RecordObject& record_obj) override;

void PreCallRecordGetRayTracingShaderGroupStackSizeKHR(VkDevice device, VkPipeline pipeline, uint32_t group,
                                                       VkShaderGroupShaderKHR groupShader) override;

void PostCallRecordGetRayTracingShaderGroupStackSizeKHR(VkDevice device, VkPipeline pipeline, uint32_t group,
                                                        VkShaderGroupShaderKHR groupShader,
                                                        const RecordObject& record_obj) override;

void PreCallRecordCmdSetRayTracingPipelineStackSizeKHR(VkCommandBuffer commandBuffer, uint32_t pipelineStackSize) override;

void PostCallRecordCmdSetRayTracingPipelineStackSizeKHR(VkCommandBuffer commandBuffer, uint32_t pipelineStackSize,
                                                        const RecordObject& record_obj) override;

void PreCallRecordCmdDrawMeshTasksEXT(VkCommandBuffer commandBuffer, uint32_t groupCountX, uint32_t groupCountY,
                                      uint32_t groupCountZ) override;

void PostCallRecordCmdDrawMeshTasksEXT(VkCommandBuffer commandBuffer, uint32_t groupCountX, uint32_t groupCountY,
                                       uint32_t groupCountZ, const RecordObject& record_obj) override;

void PreCallRecordCmdDrawMeshTasksIndirectEXT(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                              uint32_t drawCount, uint32_t stride) override;

void PostCallRecordCmdDrawMeshTasksIndirectEXT(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                               uint32_t drawCount, uint32_t stride, const RecordObject& record_obj) override;

void PreCallRecordCmdDrawMeshTasksIndirectCountEXT(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                   VkBuffer countBuffer, VkDeviceSize countBufferOffset, uint32_t maxDrawCount,
                                                   uint32_t stride) override;

void PostCallRecordCmdDrawMeshTasksIndirectCountEXT(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                    VkBuffer countBuffer, VkDeviceSize countBufferOffset, uint32_t maxDrawCount,
                                                    uint32_t stride, const RecordObject& record_obj) override;

// NOLINTEND
