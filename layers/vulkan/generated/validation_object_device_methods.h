// *** THIS FILE IS GENERATED - DO NOT EDIT ***
// See layer_chassis_generator.py for modifications

/***************************************************************************
 *
 * Copyright (c) 2015-2025 The Khronos Group Inc.
 * Copyright (c) 2015-2025 Valve Corporation
 * Copyright (c) 2015-2025 LunarG, Inc.
 * Copyright (c) 2015-2024 Google Inc.
 * Copyright (c) 2023-2024 RasterGrid Kft.
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

// This file contains methods for class vvl::base::Device and it is designed to ONLY be
// included into validation_object.h.

virtual bool PreCallValidateGetDeviceProcAddr(VkDevice device, const char* pName, ErrorObject& error_obj) const { return false; }
virtual void PreCallRecordGetDeviceProcAddr(VkDevice device, const char* pName, const RecordObject& record_obj) {}
virtual void PostCallRecordGetDeviceProcAddr(VkDevice device, const char* pName, const RecordObject& record_obj) {}
virtual bool PreCallValidateDestroyDevice(VkDevice device, const VkAllocationCallbacks* pAllocator, ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordDestroyDevice(VkDevice device, const VkAllocationCallbacks* pAllocator, const RecordObject& record_obj) {}
virtual void PostCallRecordDestroyDevice(VkDevice device, const VkAllocationCallbacks* pAllocator, const RecordObject& record_obj) {
}
virtual bool PreCallValidateGetDeviceQueue(VkDevice device, uint32_t queueFamilyIndex, uint32_t queueIndex, VkQueue* pQueue,
                                           ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordGetDeviceQueue(VkDevice device, uint32_t queueFamilyIndex, uint32_t queueIndex, VkQueue* pQueue,
                                         const RecordObject& record_obj) {}
virtual void PostCallRecordGetDeviceQueue(VkDevice device, uint32_t queueFamilyIndex, uint32_t queueIndex, VkQueue* pQueue,
                                          const RecordObject& record_obj) {}
virtual bool PreCallValidateQueueSubmit(VkQueue queue, uint32_t submitCount, const VkSubmitInfo* pSubmits, VkFence fence,
                                        ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordQueueSubmit(VkQueue queue, uint32_t submitCount, const VkSubmitInfo* pSubmits, VkFence fence,
                                      const RecordObject& record_obj) {}
virtual void PostCallRecordQueueSubmit(VkQueue queue, uint32_t submitCount, const VkSubmitInfo* pSubmits, VkFence fence,
                                       const RecordObject& record_obj) {}
virtual bool PreCallValidateQueueWaitIdle(VkQueue queue, ErrorObject& error_obj) const { return false; }
virtual void PreCallRecordQueueWaitIdle(VkQueue queue, const RecordObject& record_obj) {}
virtual void PostCallRecordQueueWaitIdle(VkQueue queue, const RecordObject& record_obj) {}
virtual bool PreCallValidateDeviceWaitIdle(VkDevice device, ErrorObject& error_obj) const { return false; }
virtual void PreCallRecordDeviceWaitIdle(VkDevice device, const RecordObject& record_obj) {}
virtual void PostCallRecordDeviceWaitIdle(VkDevice device, const RecordObject& record_obj) {}
virtual bool PreCallValidateAllocateMemory(VkDevice device, const VkMemoryAllocateInfo* pAllocateInfo,
                                           const VkAllocationCallbacks* pAllocator, VkDeviceMemory* pMemory,
                                           ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordAllocateMemory(VkDevice device, const VkMemoryAllocateInfo* pAllocateInfo,
                                         const VkAllocationCallbacks* pAllocator, VkDeviceMemory* pMemory,
                                         const RecordObject& record_obj) {}
virtual void PostCallRecordAllocateMemory(VkDevice device, const VkMemoryAllocateInfo* pAllocateInfo,
                                          const VkAllocationCallbacks* pAllocator, VkDeviceMemory* pMemory,
                                          const RecordObject& record_obj) {}
virtual bool PreCallValidateFreeMemory(VkDevice device, VkDeviceMemory memory, const VkAllocationCallbacks* pAllocator,
                                       ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordFreeMemory(VkDevice device, VkDeviceMemory memory, const VkAllocationCallbacks* pAllocator,
                                     const RecordObject& record_obj) {}
virtual void PostCallRecordFreeMemory(VkDevice device, VkDeviceMemory memory, const VkAllocationCallbacks* pAllocator,
                                      const RecordObject& record_obj) {}
virtual bool PreCallValidateMapMemory(VkDevice device, VkDeviceMemory memory, VkDeviceSize offset, VkDeviceSize size,
                                      VkMemoryMapFlags flags, void** ppData, ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordMapMemory(VkDevice device, VkDeviceMemory memory, VkDeviceSize offset, VkDeviceSize size,
                                    VkMemoryMapFlags flags, void** ppData, const RecordObject& record_obj) {}
virtual void PostCallRecordMapMemory(VkDevice device, VkDeviceMemory memory, VkDeviceSize offset, VkDeviceSize size,
                                     VkMemoryMapFlags flags, void** ppData, const RecordObject& record_obj) {}
virtual bool PreCallValidateUnmapMemory(VkDevice device, VkDeviceMemory memory, ErrorObject& error_obj) const { return false; }
virtual void PreCallRecordUnmapMemory(VkDevice device, VkDeviceMemory memory, const RecordObject& record_obj) {}
virtual void PostCallRecordUnmapMemory(VkDevice device, VkDeviceMemory memory, const RecordObject& record_obj) {}
virtual bool PreCallValidateFlushMappedMemoryRanges(VkDevice device, uint32_t memoryRangeCount,
                                                    const VkMappedMemoryRange* pMemoryRanges, ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordFlushMappedMemoryRanges(VkDevice device, uint32_t memoryRangeCount,
                                                  const VkMappedMemoryRange* pMemoryRanges, const RecordObject& record_obj) {}
virtual void PostCallRecordFlushMappedMemoryRanges(VkDevice device, uint32_t memoryRangeCount,
                                                   const VkMappedMemoryRange* pMemoryRanges, const RecordObject& record_obj) {}
virtual bool PreCallValidateInvalidateMappedMemoryRanges(VkDevice device, uint32_t memoryRangeCount,
                                                         const VkMappedMemoryRange* pMemoryRanges, ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordInvalidateMappedMemoryRanges(VkDevice device, uint32_t memoryRangeCount,
                                                       const VkMappedMemoryRange* pMemoryRanges, const RecordObject& record_obj) {}
virtual void PostCallRecordInvalidateMappedMemoryRanges(VkDevice device, uint32_t memoryRangeCount,
                                                        const VkMappedMemoryRange* pMemoryRanges, const RecordObject& record_obj) {}
virtual bool PreCallValidateGetDeviceMemoryCommitment(VkDevice device, VkDeviceMemory memory, VkDeviceSize* pCommittedMemoryInBytes,
                                                      ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordGetDeviceMemoryCommitment(VkDevice device, VkDeviceMemory memory, VkDeviceSize* pCommittedMemoryInBytes,
                                                    const RecordObject& record_obj) {}
virtual void PostCallRecordGetDeviceMemoryCommitment(VkDevice device, VkDeviceMemory memory, VkDeviceSize* pCommittedMemoryInBytes,
                                                     const RecordObject& record_obj) {}
virtual bool PreCallValidateBindBufferMemory(VkDevice device, VkBuffer buffer, VkDeviceMemory memory, VkDeviceSize memoryOffset,
                                             ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordBindBufferMemory(VkDevice device, VkBuffer buffer, VkDeviceMemory memory, VkDeviceSize memoryOffset,
                                           const RecordObject& record_obj) {}
virtual void PostCallRecordBindBufferMemory(VkDevice device, VkBuffer buffer, VkDeviceMemory memory, VkDeviceSize memoryOffset,
                                            const RecordObject& record_obj) {}
virtual bool PreCallValidateBindImageMemory(VkDevice device, VkImage image, VkDeviceMemory memory, VkDeviceSize memoryOffset,
                                            ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordBindImageMemory(VkDevice device, VkImage image, VkDeviceMemory memory, VkDeviceSize memoryOffset,
                                          const RecordObject& record_obj) {}
virtual void PostCallRecordBindImageMemory(VkDevice device, VkImage image, VkDeviceMemory memory, VkDeviceSize memoryOffset,
                                           const RecordObject& record_obj) {}
virtual bool PreCallValidateGetBufferMemoryRequirements(VkDevice device, VkBuffer buffer, VkMemoryRequirements* pMemoryRequirements,
                                                        ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordGetBufferMemoryRequirements(VkDevice device, VkBuffer buffer, VkMemoryRequirements* pMemoryRequirements,
                                                      const RecordObject& record_obj) {}
virtual void PostCallRecordGetBufferMemoryRequirements(VkDevice device, VkBuffer buffer, VkMemoryRequirements* pMemoryRequirements,
                                                       const RecordObject& record_obj) {}
virtual bool PreCallValidateGetImageMemoryRequirements(VkDevice device, VkImage image, VkMemoryRequirements* pMemoryRequirements,
                                                       ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordGetImageMemoryRequirements(VkDevice device, VkImage image, VkMemoryRequirements* pMemoryRequirements,
                                                     const RecordObject& record_obj) {}
virtual void PostCallRecordGetImageMemoryRequirements(VkDevice device, VkImage image, VkMemoryRequirements* pMemoryRequirements,
                                                      const RecordObject& record_obj) {}
virtual bool PreCallValidateGetImageSparseMemoryRequirements(VkDevice device, VkImage image,
                                                             uint32_t* pSparseMemoryRequirementCount,
                                                             VkSparseImageMemoryRequirements* pSparseMemoryRequirements,
                                                             ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordGetImageSparseMemoryRequirements(VkDevice device, VkImage image, uint32_t* pSparseMemoryRequirementCount,
                                                           VkSparseImageMemoryRequirements* pSparseMemoryRequirements,
                                                           const RecordObject& record_obj) {}
virtual void PostCallRecordGetImageSparseMemoryRequirements(VkDevice device, VkImage image, uint32_t* pSparseMemoryRequirementCount,
                                                            VkSparseImageMemoryRequirements* pSparseMemoryRequirements,
                                                            const RecordObject& record_obj) {}
virtual bool PreCallValidateQueueBindSparse(VkQueue queue, uint32_t bindInfoCount, const VkBindSparseInfo* pBindInfo, VkFence fence,
                                            ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordQueueBindSparse(VkQueue queue, uint32_t bindInfoCount, const VkBindSparseInfo* pBindInfo, VkFence fence,
                                          const RecordObject& record_obj) {}
virtual void PostCallRecordQueueBindSparse(VkQueue queue, uint32_t bindInfoCount, const VkBindSparseInfo* pBindInfo, VkFence fence,
                                           const RecordObject& record_obj) {}
virtual bool PreCallValidateCreateFence(VkDevice device, const VkFenceCreateInfo* pCreateInfo,
                                        const VkAllocationCallbacks* pAllocator, VkFence* pFence, ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordCreateFence(VkDevice device, const VkFenceCreateInfo* pCreateInfo,
                                      const VkAllocationCallbacks* pAllocator, VkFence* pFence, const RecordObject& record_obj) {}
virtual void PostCallRecordCreateFence(VkDevice device, const VkFenceCreateInfo* pCreateInfo,
                                       const VkAllocationCallbacks* pAllocator, VkFence* pFence, const RecordObject& record_obj) {}
virtual bool PreCallValidateDestroyFence(VkDevice device, VkFence fence, const VkAllocationCallbacks* pAllocator,
                                         ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordDestroyFence(VkDevice device, VkFence fence, const VkAllocationCallbacks* pAllocator,
                                       const RecordObject& record_obj) {}
virtual void PostCallRecordDestroyFence(VkDevice device, VkFence fence, const VkAllocationCallbacks* pAllocator,
                                        const RecordObject& record_obj) {}
virtual bool PreCallValidateResetFences(VkDevice device, uint32_t fenceCount, const VkFence* pFences,
                                        ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordResetFences(VkDevice device, uint32_t fenceCount, const VkFence* pFences,
                                      const RecordObject& record_obj) {}
virtual void PostCallRecordResetFences(VkDevice device, uint32_t fenceCount, const VkFence* pFences,
                                       const RecordObject& record_obj) {}
virtual bool PreCallValidateGetFenceStatus(VkDevice device, VkFence fence, ErrorObject& error_obj) const { return false; }
virtual void PreCallRecordGetFenceStatus(VkDevice device, VkFence fence, const RecordObject& record_obj) {}
virtual void PostCallRecordGetFenceStatus(VkDevice device, VkFence fence, const RecordObject& record_obj) {}
virtual bool PreCallValidateWaitForFences(VkDevice device, uint32_t fenceCount, const VkFence* pFences, VkBool32 waitAll,
                                          uint64_t timeout, ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordWaitForFences(VkDevice device, uint32_t fenceCount, const VkFence* pFences, VkBool32 waitAll,
                                        uint64_t timeout, const RecordObject& record_obj) {}
virtual void PostCallRecordWaitForFences(VkDevice device, uint32_t fenceCount, const VkFence* pFences, VkBool32 waitAll,
                                         uint64_t timeout, const RecordObject& record_obj) {}
virtual bool PreCallValidateCreateSemaphore(VkDevice device, const VkSemaphoreCreateInfo* pCreateInfo,
                                            const VkAllocationCallbacks* pAllocator, VkSemaphore* pSemaphore,
                                            ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordCreateSemaphore(VkDevice device, const VkSemaphoreCreateInfo* pCreateInfo,
                                          const VkAllocationCallbacks* pAllocator, VkSemaphore* pSemaphore,
                                          const RecordObject& record_obj) {}
virtual void PostCallRecordCreateSemaphore(VkDevice device, const VkSemaphoreCreateInfo* pCreateInfo,
                                           const VkAllocationCallbacks* pAllocator, VkSemaphore* pSemaphore,
                                           const RecordObject& record_obj) {}
virtual bool PreCallValidateDestroySemaphore(VkDevice device, VkSemaphore semaphore, const VkAllocationCallbacks* pAllocator,
                                             ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordDestroySemaphore(VkDevice device, VkSemaphore semaphore, const VkAllocationCallbacks* pAllocator,
                                           const RecordObject& record_obj) {}
virtual void PostCallRecordDestroySemaphore(VkDevice device, VkSemaphore semaphore, const VkAllocationCallbacks* pAllocator,
                                            const RecordObject& record_obj) {}
virtual bool PreCallValidateCreateEvent(VkDevice device, const VkEventCreateInfo* pCreateInfo,
                                        const VkAllocationCallbacks* pAllocator, VkEvent* pEvent, ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordCreateEvent(VkDevice device, const VkEventCreateInfo* pCreateInfo,
                                      const VkAllocationCallbacks* pAllocator, VkEvent* pEvent, const RecordObject& record_obj) {}
virtual void PostCallRecordCreateEvent(VkDevice device, const VkEventCreateInfo* pCreateInfo,
                                       const VkAllocationCallbacks* pAllocator, VkEvent* pEvent, const RecordObject& record_obj) {}
virtual bool PreCallValidateDestroyEvent(VkDevice device, VkEvent event, const VkAllocationCallbacks* pAllocator,
                                         ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordDestroyEvent(VkDevice device, VkEvent event, const VkAllocationCallbacks* pAllocator,
                                       const RecordObject& record_obj) {}
virtual void PostCallRecordDestroyEvent(VkDevice device, VkEvent event, const VkAllocationCallbacks* pAllocator,
                                        const RecordObject& record_obj) {}
virtual bool PreCallValidateGetEventStatus(VkDevice device, VkEvent event, ErrorObject& error_obj) const { return false; }
virtual void PreCallRecordGetEventStatus(VkDevice device, VkEvent event, const RecordObject& record_obj) {}
virtual void PostCallRecordGetEventStatus(VkDevice device, VkEvent event, const RecordObject& record_obj) {}
virtual bool PreCallValidateSetEvent(VkDevice device, VkEvent event, ErrorObject& error_obj) const { return false; }
virtual void PreCallRecordSetEvent(VkDevice device, VkEvent event, const RecordObject& record_obj) {}
virtual void PostCallRecordSetEvent(VkDevice device, VkEvent event, const RecordObject& record_obj) {}
virtual bool PreCallValidateResetEvent(VkDevice device, VkEvent event, ErrorObject& error_obj) const { return false; }
virtual void PreCallRecordResetEvent(VkDevice device, VkEvent event, const RecordObject& record_obj) {}
virtual void PostCallRecordResetEvent(VkDevice device, VkEvent event, const RecordObject& record_obj) {}
virtual bool PreCallValidateCreateQueryPool(VkDevice device, const VkQueryPoolCreateInfo* pCreateInfo,
                                            const VkAllocationCallbacks* pAllocator, VkQueryPool* pQueryPool,
                                            ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordCreateQueryPool(VkDevice device, const VkQueryPoolCreateInfo* pCreateInfo,
                                          const VkAllocationCallbacks* pAllocator, VkQueryPool* pQueryPool,
                                          const RecordObject& record_obj) {}
virtual void PostCallRecordCreateQueryPool(VkDevice device, const VkQueryPoolCreateInfo* pCreateInfo,
                                           const VkAllocationCallbacks* pAllocator, VkQueryPool* pQueryPool,
                                           const RecordObject& record_obj) {}
virtual bool PreCallValidateDestroyQueryPool(VkDevice device, VkQueryPool queryPool, const VkAllocationCallbacks* pAllocator,
                                             ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordDestroyQueryPool(VkDevice device, VkQueryPool queryPool, const VkAllocationCallbacks* pAllocator,
                                           const RecordObject& record_obj) {}
virtual void PostCallRecordDestroyQueryPool(VkDevice device, VkQueryPool queryPool, const VkAllocationCallbacks* pAllocator,
                                            const RecordObject& record_obj) {}
virtual bool PreCallValidateGetQueryPoolResults(VkDevice device, VkQueryPool queryPool, uint32_t firstQuery, uint32_t queryCount,
                                                size_t dataSize, void* pData, VkDeviceSize stride, VkQueryResultFlags flags,
                                                ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordGetQueryPoolResults(VkDevice device, VkQueryPool queryPool, uint32_t firstQuery, uint32_t queryCount,
                                              size_t dataSize, void* pData, VkDeviceSize stride, VkQueryResultFlags flags,
                                              const RecordObject& record_obj) {}
virtual void PostCallRecordGetQueryPoolResults(VkDevice device, VkQueryPool queryPool, uint32_t firstQuery, uint32_t queryCount,
                                               size_t dataSize, void* pData, VkDeviceSize stride, VkQueryResultFlags flags,
                                               const RecordObject& record_obj) {}
virtual bool PreCallValidateCreateBuffer(VkDevice device, const VkBufferCreateInfo* pCreateInfo,
                                         const VkAllocationCallbacks* pAllocator, VkBuffer* pBuffer, ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordCreateBuffer(VkDevice device, const VkBufferCreateInfo* pCreateInfo,
                                       const VkAllocationCallbacks* pAllocator, VkBuffer* pBuffer, const RecordObject& record_obj) {
}
virtual void PostCallRecordCreateBuffer(VkDevice device, const VkBufferCreateInfo* pCreateInfo,
                                        const VkAllocationCallbacks* pAllocator, VkBuffer* pBuffer,
                                        const RecordObject& record_obj) {}
virtual bool PreCallValidateDestroyBuffer(VkDevice device, VkBuffer buffer, const VkAllocationCallbacks* pAllocator,
                                          ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordDestroyBuffer(VkDevice device, VkBuffer buffer, const VkAllocationCallbacks* pAllocator,
                                        const RecordObject& record_obj) {}
virtual void PostCallRecordDestroyBuffer(VkDevice device, VkBuffer buffer, const VkAllocationCallbacks* pAllocator,
                                         const RecordObject& record_obj) {}
virtual bool PreCallValidateCreateBufferView(VkDevice device, const VkBufferViewCreateInfo* pCreateInfo,
                                             const VkAllocationCallbacks* pAllocator, VkBufferView* pView,
                                             ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordCreateBufferView(VkDevice device, const VkBufferViewCreateInfo* pCreateInfo,
                                           const VkAllocationCallbacks* pAllocator, VkBufferView* pView,
                                           const RecordObject& record_obj) {}
virtual void PostCallRecordCreateBufferView(VkDevice device, const VkBufferViewCreateInfo* pCreateInfo,
                                            const VkAllocationCallbacks* pAllocator, VkBufferView* pView,
                                            const RecordObject& record_obj) {}
virtual bool PreCallValidateDestroyBufferView(VkDevice device, VkBufferView bufferView, const VkAllocationCallbacks* pAllocator,
                                              ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordDestroyBufferView(VkDevice device, VkBufferView bufferView, const VkAllocationCallbacks* pAllocator,
                                            const RecordObject& record_obj) {}
virtual void PostCallRecordDestroyBufferView(VkDevice device, VkBufferView bufferView, const VkAllocationCallbacks* pAllocator,
                                             const RecordObject& record_obj) {}
virtual bool PreCallValidateCreateImage(VkDevice device, const VkImageCreateInfo* pCreateInfo,
                                        const VkAllocationCallbacks* pAllocator, VkImage* pImage, ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordCreateImage(VkDevice device, const VkImageCreateInfo* pCreateInfo,
                                      const VkAllocationCallbacks* pAllocator, VkImage* pImage, const RecordObject& record_obj) {}
virtual void PostCallRecordCreateImage(VkDevice device, const VkImageCreateInfo* pCreateInfo,
                                       const VkAllocationCallbacks* pAllocator, VkImage* pImage, const RecordObject& record_obj) {}
virtual bool PreCallValidateDestroyImage(VkDevice device, VkImage image, const VkAllocationCallbacks* pAllocator,
                                         ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordDestroyImage(VkDevice device, VkImage image, const VkAllocationCallbacks* pAllocator,
                                       const RecordObject& record_obj) {}
virtual void PostCallRecordDestroyImage(VkDevice device, VkImage image, const VkAllocationCallbacks* pAllocator,
                                        const RecordObject& record_obj) {}
virtual bool PreCallValidateGetImageSubresourceLayout(VkDevice device, VkImage image, const VkImageSubresource* pSubresource,
                                                      VkSubresourceLayout* pLayout, ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordGetImageSubresourceLayout(VkDevice device, VkImage image, const VkImageSubresource* pSubresource,
                                                    VkSubresourceLayout* pLayout, const RecordObject& record_obj) {}
virtual void PostCallRecordGetImageSubresourceLayout(VkDevice device, VkImage image, const VkImageSubresource* pSubresource,
                                                     VkSubresourceLayout* pLayout, const RecordObject& record_obj) {}
virtual bool PreCallValidateCreateImageView(VkDevice device, const VkImageViewCreateInfo* pCreateInfo,
                                            const VkAllocationCallbacks* pAllocator, VkImageView* pView,
                                            ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordCreateImageView(VkDevice device, const VkImageViewCreateInfo* pCreateInfo,
                                          const VkAllocationCallbacks* pAllocator, VkImageView* pView,
                                          const RecordObject& record_obj) {}
virtual void PostCallRecordCreateImageView(VkDevice device, const VkImageViewCreateInfo* pCreateInfo,
                                           const VkAllocationCallbacks* pAllocator, VkImageView* pView,
                                           const RecordObject& record_obj) {}
virtual bool PreCallValidateDestroyImageView(VkDevice device, VkImageView imageView, const VkAllocationCallbacks* pAllocator,
                                             ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordDestroyImageView(VkDevice device, VkImageView imageView, const VkAllocationCallbacks* pAllocator,
                                           const RecordObject& record_obj) {}
virtual void PostCallRecordDestroyImageView(VkDevice device, VkImageView imageView, const VkAllocationCallbacks* pAllocator,
                                            const RecordObject& record_obj) {}
virtual bool PreCallValidateCreateShaderModule(VkDevice device, const VkShaderModuleCreateInfo* pCreateInfo,
                                               const VkAllocationCallbacks* pAllocator, VkShaderModule* pShaderModule,
                                               ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordCreateShaderModule(VkDevice device, const VkShaderModuleCreateInfo* pCreateInfo,
                                             const VkAllocationCallbacks* pAllocator, VkShaderModule* pShaderModule,
                                             const RecordObject& record_obj) {}
virtual void PostCallRecordCreateShaderModule(VkDevice device, const VkShaderModuleCreateInfo* pCreateInfo,
                                              const VkAllocationCallbacks* pAllocator, VkShaderModule* pShaderModule,
                                              const RecordObject& record_obj) {}
virtual bool PreCallValidateDestroyShaderModule(VkDevice device, VkShaderModule shaderModule,
                                                const VkAllocationCallbacks* pAllocator, ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordDestroyShaderModule(VkDevice device, VkShaderModule shaderModule, const VkAllocationCallbacks* pAllocator,
                                              const RecordObject& record_obj) {}
virtual void PostCallRecordDestroyShaderModule(VkDevice device, VkShaderModule shaderModule,
                                               const VkAllocationCallbacks* pAllocator, const RecordObject& record_obj) {}
virtual bool PreCallValidateCreatePipelineCache(VkDevice device, const VkPipelineCacheCreateInfo* pCreateInfo,
                                                const VkAllocationCallbacks* pAllocator, VkPipelineCache* pPipelineCache,
                                                ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordCreatePipelineCache(VkDevice device, const VkPipelineCacheCreateInfo* pCreateInfo,
                                              const VkAllocationCallbacks* pAllocator, VkPipelineCache* pPipelineCache,
                                              const RecordObject& record_obj) {}
virtual void PostCallRecordCreatePipelineCache(VkDevice device, const VkPipelineCacheCreateInfo* pCreateInfo,
                                               const VkAllocationCallbacks* pAllocator, VkPipelineCache* pPipelineCache,
                                               const RecordObject& record_obj) {}
virtual bool PreCallValidateDestroyPipelineCache(VkDevice device, VkPipelineCache pipelineCache,
                                                 const VkAllocationCallbacks* pAllocator, ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordDestroyPipelineCache(VkDevice device, VkPipelineCache pipelineCache,
                                               const VkAllocationCallbacks* pAllocator, const RecordObject& record_obj) {}
virtual void PostCallRecordDestroyPipelineCache(VkDevice device, VkPipelineCache pipelineCache,
                                                const VkAllocationCallbacks* pAllocator, const RecordObject& record_obj) {}
virtual bool PreCallValidateGetPipelineCacheData(VkDevice device, VkPipelineCache pipelineCache, size_t* pDataSize, void* pData,
                                                 ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordGetPipelineCacheData(VkDevice device, VkPipelineCache pipelineCache, size_t* pDataSize, void* pData,
                                               const RecordObject& record_obj) {}
virtual void PostCallRecordGetPipelineCacheData(VkDevice device, VkPipelineCache pipelineCache, size_t* pDataSize, void* pData,
                                                const RecordObject& record_obj) {}
virtual bool PreCallValidateMergePipelineCaches(VkDevice device, VkPipelineCache dstCache, uint32_t srcCacheCount,
                                                const VkPipelineCache* pSrcCaches, ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordMergePipelineCaches(VkDevice device, VkPipelineCache dstCache, uint32_t srcCacheCount,
                                              const VkPipelineCache* pSrcCaches, const RecordObject& record_obj) {}
virtual void PostCallRecordMergePipelineCaches(VkDevice device, VkPipelineCache dstCache, uint32_t srcCacheCount,
                                               const VkPipelineCache* pSrcCaches, const RecordObject& record_obj) {}
virtual bool PreCallValidateCreateGraphicsPipelines(VkDevice device, VkPipelineCache pipelineCache, uint32_t createInfoCount,
                                                    const VkGraphicsPipelineCreateInfo* pCreateInfos,
                                                    const VkAllocationCallbacks* pAllocator, VkPipeline* pPipelines,
                                                    ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordCreateGraphicsPipelines(VkDevice device, VkPipelineCache pipelineCache, uint32_t createInfoCount,
                                                  const VkGraphicsPipelineCreateInfo* pCreateInfos,
                                                  const VkAllocationCallbacks* pAllocator, VkPipeline* pPipelines,
                                                  const RecordObject& record_obj) {}
virtual void PostCallRecordCreateGraphicsPipelines(VkDevice device, VkPipelineCache pipelineCache, uint32_t createInfoCount,
                                                   const VkGraphicsPipelineCreateInfo* pCreateInfos,
                                                   const VkAllocationCallbacks* pAllocator, VkPipeline* pPipelines,
                                                   const RecordObject& record_obj) {}
virtual bool PreCallValidateCreateComputePipelines(VkDevice device, VkPipelineCache pipelineCache, uint32_t createInfoCount,
                                                   const VkComputePipelineCreateInfo* pCreateInfos,
                                                   const VkAllocationCallbacks* pAllocator, VkPipeline* pPipelines,
                                                   ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordCreateComputePipelines(VkDevice device, VkPipelineCache pipelineCache, uint32_t createInfoCount,
                                                 const VkComputePipelineCreateInfo* pCreateInfos,
                                                 const VkAllocationCallbacks* pAllocator, VkPipeline* pPipelines,
                                                 const RecordObject& record_obj) {}
virtual void PostCallRecordCreateComputePipelines(VkDevice device, VkPipelineCache pipelineCache, uint32_t createInfoCount,
                                                  const VkComputePipelineCreateInfo* pCreateInfos,
                                                  const VkAllocationCallbacks* pAllocator, VkPipeline* pPipelines,
                                                  const RecordObject& record_obj) {}
virtual bool PreCallValidateDestroyPipeline(VkDevice device, VkPipeline pipeline, const VkAllocationCallbacks* pAllocator,
                                            ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordDestroyPipeline(VkDevice device, VkPipeline pipeline, const VkAllocationCallbacks* pAllocator,
                                          const RecordObject& record_obj) {}
virtual void PostCallRecordDestroyPipeline(VkDevice device, VkPipeline pipeline, const VkAllocationCallbacks* pAllocator,
                                           const RecordObject& record_obj) {}
virtual bool PreCallValidateCreatePipelineLayout(VkDevice device, const VkPipelineLayoutCreateInfo* pCreateInfo,
                                                 const VkAllocationCallbacks* pAllocator, VkPipelineLayout* pPipelineLayout,
                                                 ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordCreatePipelineLayout(VkDevice device, const VkPipelineLayoutCreateInfo* pCreateInfo,
                                               const VkAllocationCallbacks* pAllocator, VkPipelineLayout* pPipelineLayout,
                                               const RecordObject& record_obj) {}
virtual void PostCallRecordCreatePipelineLayout(VkDevice device, const VkPipelineLayoutCreateInfo* pCreateInfo,
                                                const VkAllocationCallbacks* pAllocator, VkPipelineLayout* pPipelineLayout,
                                                const RecordObject& record_obj) {}
virtual bool PreCallValidateDestroyPipelineLayout(VkDevice device, VkPipelineLayout pipelineLayout,
                                                  const VkAllocationCallbacks* pAllocator, ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordDestroyPipelineLayout(VkDevice device, VkPipelineLayout pipelineLayout,
                                                const VkAllocationCallbacks* pAllocator, const RecordObject& record_obj) {}
virtual void PostCallRecordDestroyPipelineLayout(VkDevice device, VkPipelineLayout pipelineLayout,
                                                 const VkAllocationCallbacks* pAllocator, const RecordObject& record_obj) {}
virtual bool PreCallValidateCreateSampler(VkDevice device, const VkSamplerCreateInfo* pCreateInfo,
                                          const VkAllocationCallbacks* pAllocator, VkSampler* pSampler,
                                          ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordCreateSampler(VkDevice device, const VkSamplerCreateInfo* pCreateInfo,
                                        const VkAllocationCallbacks* pAllocator, VkSampler* pSampler,
                                        const RecordObject& record_obj) {}
virtual void PostCallRecordCreateSampler(VkDevice device, const VkSamplerCreateInfo* pCreateInfo,
                                         const VkAllocationCallbacks* pAllocator, VkSampler* pSampler,
                                         const RecordObject& record_obj) {}
virtual bool PreCallValidateDestroySampler(VkDevice device, VkSampler sampler, const VkAllocationCallbacks* pAllocator,
                                           ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordDestroySampler(VkDevice device, VkSampler sampler, const VkAllocationCallbacks* pAllocator,
                                         const RecordObject& record_obj) {}
virtual void PostCallRecordDestroySampler(VkDevice device, VkSampler sampler, const VkAllocationCallbacks* pAllocator,
                                          const RecordObject& record_obj) {}
virtual bool PreCallValidateCreateDescriptorSetLayout(VkDevice device, const VkDescriptorSetLayoutCreateInfo* pCreateInfo,
                                                      const VkAllocationCallbacks* pAllocator, VkDescriptorSetLayout* pSetLayout,
                                                      ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordCreateDescriptorSetLayout(VkDevice device, const VkDescriptorSetLayoutCreateInfo* pCreateInfo,
                                                    const VkAllocationCallbacks* pAllocator, VkDescriptorSetLayout* pSetLayout,
                                                    const RecordObject& record_obj) {}
virtual void PostCallRecordCreateDescriptorSetLayout(VkDevice device, const VkDescriptorSetLayoutCreateInfo* pCreateInfo,
                                                     const VkAllocationCallbacks* pAllocator, VkDescriptorSetLayout* pSetLayout,
                                                     const RecordObject& record_obj) {}
virtual bool PreCallValidateDestroyDescriptorSetLayout(VkDevice device, VkDescriptorSetLayout descriptorSetLayout,
                                                       const VkAllocationCallbacks* pAllocator, ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordDestroyDescriptorSetLayout(VkDevice device, VkDescriptorSetLayout descriptorSetLayout,
                                                     const VkAllocationCallbacks* pAllocator, const RecordObject& record_obj) {}
virtual void PostCallRecordDestroyDescriptorSetLayout(VkDevice device, VkDescriptorSetLayout descriptorSetLayout,
                                                      const VkAllocationCallbacks* pAllocator, const RecordObject& record_obj) {}
virtual bool PreCallValidateCreateDescriptorPool(VkDevice device, const VkDescriptorPoolCreateInfo* pCreateInfo,
                                                 const VkAllocationCallbacks* pAllocator, VkDescriptorPool* pDescriptorPool,
                                                 ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordCreateDescriptorPool(VkDevice device, const VkDescriptorPoolCreateInfo* pCreateInfo,
                                               const VkAllocationCallbacks* pAllocator, VkDescriptorPool* pDescriptorPool,
                                               const RecordObject& record_obj) {}
virtual void PostCallRecordCreateDescriptorPool(VkDevice device, const VkDescriptorPoolCreateInfo* pCreateInfo,
                                                const VkAllocationCallbacks* pAllocator, VkDescriptorPool* pDescriptorPool,
                                                const RecordObject& record_obj) {}
virtual bool PreCallValidateDestroyDescriptorPool(VkDevice device, VkDescriptorPool descriptorPool,
                                                  const VkAllocationCallbacks* pAllocator, ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordDestroyDescriptorPool(VkDevice device, VkDescriptorPool descriptorPool,
                                                const VkAllocationCallbacks* pAllocator, const RecordObject& record_obj) {}
virtual void PostCallRecordDestroyDescriptorPool(VkDevice device, VkDescriptorPool descriptorPool,
                                                 const VkAllocationCallbacks* pAllocator, const RecordObject& record_obj) {}
virtual bool PreCallValidateResetDescriptorPool(VkDevice device, VkDescriptorPool descriptorPool, VkDescriptorPoolResetFlags flags,
                                                ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordResetDescriptorPool(VkDevice device, VkDescriptorPool descriptorPool, VkDescriptorPoolResetFlags flags,
                                              const RecordObject& record_obj) {}
virtual void PostCallRecordResetDescriptorPool(VkDevice device, VkDescriptorPool descriptorPool, VkDescriptorPoolResetFlags flags,
                                               const RecordObject& record_obj) {}
virtual bool PreCallValidateAllocateDescriptorSets(VkDevice device, const VkDescriptorSetAllocateInfo* pAllocateInfo,
                                                   VkDescriptorSet* pDescriptorSets, ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordAllocateDescriptorSets(VkDevice device, const VkDescriptorSetAllocateInfo* pAllocateInfo,
                                                 VkDescriptorSet* pDescriptorSets, const RecordObject& record_obj) {}
virtual void PostCallRecordAllocateDescriptorSets(VkDevice device, const VkDescriptorSetAllocateInfo* pAllocateInfo,
                                                  VkDescriptorSet* pDescriptorSets, const RecordObject& record_obj) {}
virtual bool PreCallValidateFreeDescriptorSets(VkDevice device, VkDescriptorPool descriptorPool, uint32_t descriptorSetCount,
                                               const VkDescriptorSet* pDescriptorSets, ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordFreeDescriptorSets(VkDevice device, VkDescriptorPool descriptorPool, uint32_t descriptorSetCount,
                                             const VkDescriptorSet* pDescriptorSets, const RecordObject& record_obj) {}
virtual void PostCallRecordFreeDescriptorSets(VkDevice device, VkDescriptorPool descriptorPool, uint32_t descriptorSetCount,
                                              const VkDescriptorSet* pDescriptorSets, const RecordObject& record_obj) {}
virtual bool PreCallValidateUpdateDescriptorSets(VkDevice device, uint32_t descriptorWriteCount,
                                                 const VkWriteDescriptorSet* pDescriptorWrites, uint32_t descriptorCopyCount,
                                                 const VkCopyDescriptorSet* pDescriptorCopies, ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordUpdateDescriptorSets(VkDevice device, uint32_t descriptorWriteCount,
                                               const VkWriteDescriptorSet* pDescriptorWrites, uint32_t descriptorCopyCount,
                                               const VkCopyDescriptorSet* pDescriptorCopies, const RecordObject& record_obj) {}
virtual void PostCallRecordUpdateDescriptorSets(VkDevice device, uint32_t descriptorWriteCount,
                                                const VkWriteDescriptorSet* pDescriptorWrites, uint32_t descriptorCopyCount,
                                                const VkCopyDescriptorSet* pDescriptorCopies, const RecordObject& record_obj) {}
virtual bool PreCallValidateCreateFramebuffer(VkDevice device, const VkFramebufferCreateInfo* pCreateInfo,
                                              const VkAllocationCallbacks* pAllocator, VkFramebuffer* pFramebuffer,
                                              ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordCreateFramebuffer(VkDevice device, const VkFramebufferCreateInfo* pCreateInfo,
                                            const VkAllocationCallbacks* pAllocator, VkFramebuffer* pFramebuffer,
                                            const RecordObject& record_obj) {}
virtual void PostCallRecordCreateFramebuffer(VkDevice device, const VkFramebufferCreateInfo* pCreateInfo,
                                             const VkAllocationCallbacks* pAllocator, VkFramebuffer* pFramebuffer,
                                             const RecordObject& record_obj) {}
virtual bool PreCallValidateDestroyFramebuffer(VkDevice device, VkFramebuffer framebuffer, const VkAllocationCallbacks* pAllocator,
                                               ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordDestroyFramebuffer(VkDevice device, VkFramebuffer framebuffer, const VkAllocationCallbacks* pAllocator,
                                             const RecordObject& record_obj) {}
virtual void PostCallRecordDestroyFramebuffer(VkDevice device, VkFramebuffer framebuffer, const VkAllocationCallbacks* pAllocator,
                                              const RecordObject& record_obj) {}
virtual bool PreCallValidateCreateRenderPass(VkDevice device, const VkRenderPassCreateInfo* pCreateInfo,
                                             const VkAllocationCallbacks* pAllocator, VkRenderPass* pRenderPass,
                                             ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordCreateRenderPass(VkDevice device, const VkRenderPassCreateInfo* pCreateInfo,
                                           const VkAllocationCallbacks* pAllocator, VkRenderPass* pRenderPass,
                                           const RecordObject& record_obj) {}
virtual void PostCallRecordCreateRenderPass(VkDevice device, const VkRenderPassCreateInfo* pCreateInfo,
                                            const VkAllocationCallbacks* pAllocator, VkRenderPass* pRenderPass,
                                            const RecordObject& record_obj) {}
virtual bool PreCallValidateDestroyRenderPass(VkDevice device, VkRenderPass renderPass, const VkAllocationCallbacks* pAllocator,
                                              ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordDestroyRenderPass(VkDevice device, VkRenderPass renderPass, const VkAllocationCallbacks* pAllocator,
                                            const RecordObject& record_obj) {}
virtual void PostCallRecordDestroyRenderPass(VkDevice device, VkRenderPass renderPass, const VkAllocationCallbacks* pAllocator,
                                             const RecordObject& record_obj) {}
virtual bool PreCallValidateGetRenderAreaGranularity(VkDevice device, VkRenderPass renderPass, VkExtent2D* pGranularity,
                                                     ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordGetRenderAreaGranularity(VkDevice device, VkRenderPass renderPass, VkExtent2D* pGranularity,
                                                   const RecordObject& record_obj) {}
virtual void PostCallRecordGetRenderAreaGranularity(VkDevice device, VkRenderPass renderPass, VkExtent2D* pGranularity,
                                                    const RecordObject& record_obj) {}
virtual bool PreCallValidateCreateCommandPool(VkDevice device, const VkCommandPoolCreateInfo* pCreateInfo,
                                              const VkAllocationCallbacks* pAllocator, VkCommandPool* pCommandPool,
                                              ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordCreateCommandPool(VkDevice device, const VkCommandPoolCreateInfo* pCreateInfo,
                                            const VkAllocationCallbacks* pAllocator, VkCommandPool* pCommandPool,
                                            const RecordObject& record_obj) {}
virtual void PostCallRecordCreateCommandPool(VkDevice device, const VkCommandPoolCreateInfo* pCreateInfo,
                                             const VkAllocationCallbacks* pAllocator, VkCommandPool* pCommandPool,
                                             const RecordObject& record_obj) {}
virtual bool PreCallValidateDestroyCommandPool(VkDevice device, VkCommandPool commandPool, const VkAllocationCallbacks* pAllocator,
                                               ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordDestroyCommandPool(VkDevice device, VkCommandPool commandPool, const VkAllocationCallbacks* pAllocator,
                                             const RecordObject& record_obj) {}
virtual void PostCallRecordDestroyCommandPool(VkDevice device, VkCommandPool commandPool, const VkAllocationCallbacks* pAllocator,
                                              const RecordObject& record_obj) {}
virtual bool PreCallValidateResetCommandPool(VkDevice device, VkCommandPool commandPool, VkCommandPoolResetFlags flags,
                                             ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordResetCommandPool(VkDevice device, VkCommandPool commandPool, VkCommandPoolResetFlags flags,
                                           const RecordObject& record_obj) {}
virtual void PostCallRecordResetCommandPool(VkDevice device, VkCommandPool commandPool, VkCommandPoolResetFlags flags,
                                            const RecordObject& record_obj) {}
virtual bool PreCallValidateAllocateCommandBuffers(VkDevice device, const VkCommandBufferAllocateInfo* pAllocateInfo,
                                                   VkCommandBuffer* pCommandBuffers, ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordAllocateCommandBuffers(VkDevice device, const VkCommandBufferAllocateInfo* pAllocateInfo,
                                                 VkCommandBuffer* pCommandBuffers, const RecordObject& record_obj) {}
virtual void PostCallRecordAllocateCommandBuffers(VkDevice device, const VkCommandBufferAllocateInfo* pAllocateInfo,
                                                  VkCommandBuffer* pCommandBuffers, const RecordObject& record_obj) {}
virtual bool PreCallValidateFreeCommandBuffers(VkDevice device, VkCommandPool commandPool, uint32_t commandBufferCount,
                                               const VkCommandBuffer* pCommandBuffers, ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordFreeCommandBuffers(VkDevice device, VkCommandPool commandPool, uint32_t commandBufferCount,
                                             const VkCommandBuffer* pCommandBuffers, const RecordObject& record_obj) {}
virtual void PostCallRecordFreeCommandBuffers(VkDevice device, VkCommandPool commandPool, uint32_t commandBufferCount,
                                              const VkCommandBuffer* pCommandBuffers, const RecordObject& record_obj) {}
virtual bool PreCallValidateBeginCommandBuffer(VkCommandBuffer commandBuffer, const VkCommandBufferBeginInfo* pBeginInfo,
                                               ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordBeginCommandBuffer(VkCommandBuffer commandBuffer, const VkCommandBufferBeginInfo* pBeginInfo,
                                             const RecordObject& record_obj) {}
virtual void PostCallRecordBeginCommandBuffer(VkCommandBuffer commandBuffer, const VkCommandBufferBeginInfo* pBeginInfo,
                                              const RecordObject& record_obj) {}
virtual bool PreCallValidateEndCommandBuffer(VkCommandBuffer commandBuffer, ErrorObject& error_obj) const { return false; }
virtual void PreCallRecordEndCommandBuffer(VkCommandBuffer commandBuffer, const RecordObject& record_obj) {}
virtual void PostCallRecordEndCommandBuffer(VkCommandBuffer commandBuffer, const RecordObject& record_obj) {}
virtual bool PreCallValidateResetCommandBuffer(VkCommandBuffer commandBuffer, VkCommandBufferResetFlags flags,
                                               ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordResetCommandBuffer(VkCommandBuffer commandBuffer, VkCommandBufferResetFlags flags,
                                             const RecordObject& record_obj) {}
virtual void PostCallRecordResetCommandBuffer(VkCommandBuffer commandBuffer, VkCommandBufferResetFlags flags,
                                              const RecordObject& record_obj) {}
virtual bool PreCallValidateCmdBindPipeline(VkCommandBuffer commandBuffer, VkPipelineBindPoint pipelineBindPoint,
                                            VkPipeline pipeline, ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordCmdBindPipeline(VkCommandBuffer commandBuffer, VkPipelineBindPoint pipelineBindPoint, VkPipeline pipeline,
                                          const RecordObject& record_obj) {}
virtual void PostCallRecordCmdBindPipeline(VkCommandBuffer commandBuffer, VkPipelineBindPoint pipelineBindPoint,
                                           VkPipeline pipeline, const RecordObject& record_obj) {}
virtual bool PreCallValidateCmdSetViewport(VkCommandBuffer commandBuffer, uint32_t firstViewport, uint32_t viewportCount,
                                           const VkViewport* pViewports, ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordCmdSetViewport(VkCommandBuffer commandBuffer, uint32_t firstViewport, uint32_t viewportCount,
                                         const VkViewport* pViewports, const RecordObject& record_obj) {}
virtual void PostCallRecordCmdSetViewport(VkCommandBuffer commandBuffer, uint32_t firstViewport, uint32_t viewportCount,
                                          const VkViewport* pViewports, const RecordObject& record_obj) {}
virtual bool PreCallValidateCmdSetScissor(VkCommandBuffer commandBuffer, uint32_t firstScissor, uint32_t scissorCount,
                                          const VkRect2D* pScissors, ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordCmdSetScissor(VkCommandBuffer commandBuffer, uint32_t firstScissor, uint32_t scissorCount,
                                        const VkRect2D* pScissors, const RecordObject& record_obj) {}
virtual void PostCallRecordCmdSetScissor(VkCommandBuffer commandBuffer, uint32_t firstScissor, uint32_t scissorCount,
                                         const VkRect2D* pScissors, const RecordObject& record_obj) {}
virtual bool PreCallValidateCmdSetLineWidth(VkCommandBuffer commandBuffer, float lineWidth, ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordCmdSetLineWidth(VkCommandBuffer commandBuffer, float lineWidth, const RecordObject& record_obj) {}
virtual void PostCallRecordCmdSetLineWidth(VkCommandBuffer commandBuffer, float lineWidth, const RecordObject& record_obj) {}
virtual bool PreCallValidateCmdSetDepthBias(VkCommandBuffer commandBuffer, float depthBiasConstantFactor, float depthBiasClamp,
                                            float depthBiasSlopeFactor, ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordCmdSetDepthBias(VkCommandBuffer commandBuffer, float depthBiasConstantFactor, float depthBiasClamp,
                                          float depthBiasSlopeFactor, const RecordObject& record_obj) {}
virtual void PostCallRecordCmdSetDepthBias(VkCommandBuffer commandBuffer, float depthBiasConstantFactor, float depthBiasClamp,
                                           float depthBiasSlopeFactor, const RecordObject& record_obj) {}
virtual bool PreCallValidateCmdSetBlendConstants(VkCommandBuffer commandBuffer, const float blendConstants[4],
                                                 ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordCmdSetBlendConstants(VkCommandBuffer commandBuffer, const float blendConstants[4],
                                               const RecordObject& record_obj) {}
virtual void PostCallRecordCmdSetBlendConstants(VkCommandBuffer commandBuffer, const float blendConstants[4],
                                                const RecordObject& record_obj) {}
virtual bool PreCallValidateCmdSetDepthBounds(VkCommandBuffer commandBuffer, float minDepthBounds, float maxDepthBounds,
                                              ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordCmdSetDepthBounds(VkCommandBuffer commandBuffer, float minDepthBounds, float maxDepthBounds,
                                            const RecordObject& record_obj) {}
virtual void PostCallRecordCmdSetDepthBounds(VkCommandBuffer commandBuffer, float minDepthBounds, float maxDepthBounds,
                                             const RecordObject& record_obj) {}
virtual bool PreCallValidateCmdSetStencilCompareMask(VkCommandBuffer commandBuffer, VkStencilFaceFlags faceMask,
                                                     uint32_t compareMask, ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordCmdSetStencilCompareMask(VkCommandBuffer commandBuffer, VkStencilFaceFlags faceMask, uint32_t compareMask,
                                                   const RecordObject& record_obj) {}
virtual void PostCallRecordCmdSetStencilCompareMask(VkCommandBuffer commandBuffer, VkStencilFaceFlags faceMask,
                                                    uint32_t compareMask, const RecordObject& record_obj) {}
virtual bool PreCallValidateCmdSetStencilWriteMask(VkCommandBuffer commandBuffer, VkStencilFaceFlags faceMask, uint32_t writeMask,
                                                   ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordCmdSetStencilWriteMask(VkCommandBuffer commandBuffer, VkStencilFaceFlags faceMask, uint32_t writeMask,
                                                 const RecordObject& record_obj) {}
virtual void PostCallRecordCmdSetStencilWriteMask(VkCommandBuffer commandBuffer, VkStencilFaceFlags faceMask, uint32_t writeMask,
                                                  const RecordObject& record_obj) {}
virtual bool PreCallValidateCmdSetStencilReference(VkCommandBuffer commandBuffer, VkStencilFaceFlags faceMask, uint32_t reference,
                                                   ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordCmdSetStencilReference(VkCommandBuffer commandBuffer, VkStencilFaceFlags faceMask, uint32_t reference,
                                                 const RecordObject& record_obj) {}
virtual void PostCallRecordCmdSetStencilReference(VkCommandBuffer commandBuffer, VkStencilFaceFlags faceMask, uint32_t reference,
                                                  const RecordObject& record_obj) {}
virtual bool PreCallValidateCmdBindDescriptorSets(VkCommandBuffer commandBuffer, VkPipelineBindPoint pipelineBindPoint,
                                                  VkPipelineLayout layout, uint32_t firstSet, uint32_t descriptorSetCount,
                                                  const VkDescriptorSet* pDescriptorSets, uint32_t dynamicOffsetCount,
                                                  const uint32_t* pDynamicOffsets, ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordCmdBindDescriptorSets(VkCommandBuffer commandBuffer, VkPipelineBindPoint pipelineBindPoint,
                                                VkPipelineLayout layout, uint32_t firstSet, uint32_t descriptorSetCount,
                                                const VkDescriptorSet* pDescriptorSets, uint32_t dynamicOffsetCount,
                                                const uint32_t* pDynamicOffsets, const RecordObject& record_obj) {}
virtual void PostCallRecordCmdBindDescriptorSets(VkCommandBuffer commandBuffer, VkPipelineBindPoint pipelineBindPoint,
                                                 VkPipelineLayout layout, uint32_t firstSet, uint32_t descriptorSetCount,
                                                 const VkDescriptorSet* pDescriptorSets, uint32_t dynamicOffsetCount,
                                                 const uint32_t* pDynamicOffsets, const RecordObject& record_obj) {}
virtual bool PreCallValidateCmdBindIndexBuffer(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                               VkIndexType indexType, ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordCmdBindIndexBuffer(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                             VkIndexType indexType, const RecordObject& record_obj) {}
virtual void PostCallRecordCmdBindIndexBuffer(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                              VkIndexType indexType, const RecordObject& record_obj) {}
virtual bool PreCallValidateCmdBindVertexBuffers(VkCommandBuffer commandBuffer, uint32_t firstBinding, uint32_t bindingCount,
                                                 const VkBuffer* pBuffers, const VkDeviceSize* pOffsets,
                                                 ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordCmdBindVertexBuffers(VkCommandBuffer commandBuffer, uint32_t firstBinding, uint32_t bindingCount,
                                               const VkBuffer* pBuffers, const VkDeviceSize* pOffsets,
                                               const RecordObject& record_obj) {}
virtual void PostCallRecordCmdBindVertexBuffers(VkCommandBuffer commandBuffer, uint32_t firstBinding, uint32_t bindingCount,
                                                const VkBuffer* pBuffers, const VkDeviceSize* pOffsets,
                                                const RecordObject& record_obj) {}
virtual bool PreCallValidateCmdDraw(VkCommandBuffer commandBuffer, uint32_t vertexCount, uint32_t instanceCount,
                                    uint32_t firstVertex, uint32_t firstInstance, ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordCmdDraw(VkCommandBuffer commandBuffer, uint32_t vertexCount, uint32_t instanceCount, uint32_t firstVertex,
                                  uint32_t firstInstance, const RecordObject& record_obj) {}
virtual void PostCallRecordCmdDraw(VkCommandBuffer commandBuffer, uint32_t vertexCount, uint32_t instanceCount,
                                   uint32_t firstVertex, uint32_t firstInstance, const RecordObject& record_obj) {}
virtual bool PreCallValidateCmdDrawIndexed(VkCommandBuffer commandBuffer, uint32_t indexCount, uint32_t instanceCount,
                                           uint32_t firstIndex, int32_t vertexOffset, uint32_t firstInstance,
                                           ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordCmdDrawIndexed(VkCommandBuffer commandBuffer, uint32_t indexCount, uint32_t instanceCount,
                                         uint32_t firstIndex, int32_t vertexOffset, uint32_t firstInstance,
                                         const RecordObject& record_obj) {}
virtual void PostCallRecordCmdDrawIndexed(VkCommandBuffer commandBuffer, uint32_t indexCount, uint32_t instanceCount,
                                          uint32_t firstIndex, int32_t vertexOffset, uint32_t firstInstance,
                                          const RecordObject& record_obj) {}
virtual bool PreCallValidateCmdDrawIndirect(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset, uint32_t drawCount,
                                            uint32_t stride, ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordCmdDrawIndirect(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset, uint32_t drawCount,
                                          uint32_t stride, const RecordObject& record_obj) {}
virtual void PostCallRecordCmdDrawIndirect(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset, uint32_t drawCount,
                                           uint32_t stride, const RecordObject& record_obj) {}
virtual bool PreCallValidateCmdDrawIndexedIndirect(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                   uint32_t drawCount, uint32_t stride, ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordCmdDrawIndexedIndirect(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                 uint32_t drawCount, uint32_t stride, const RecordObject& record_obj) {}
virtual void PostCallRecordCmdDrawIndexedIndirect(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                  uint32_t drawCount, uint32_t stride, const RecordObject& record_obj) {}
virtual bool PreCallValidateCmdDispatch(VkCommandBuffer commandBuffer, uint32_t groupCountX, uint32_t groupCountY,
                                        uint32_t groupCountZ, ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordCmdDispatch(VkCommandBuffer commandBuffer, uint32_t groupCountX, uint32_t groupCountY,
                                      uint32_t groupCountZ, const RecordObject& record_obj) {}
virtual void PostCallRecordCmdDispatch(VkCommandBuffer commandBuffer, uint32_t groupCountX, uint32_t groupCountY,
                                       uint32_t groupCountZ, const RecordObject& record_obj) {}
virtual bool PreCallValidateCmdDispatchIndirect(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordCmdDispatchIndirect(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                              const RecordObject& record_obj) {}
virtual void PostCallRecordCmdDispatchIndirect(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                               const RecordObject& record_obj) {}
virtual bool PreCallValidateCmdCopyBuffer(VkCommandBuffer commandBuffer, VkBuffer srcBuffer, VkBuffer dstBuffer,
                                          uint32_t regionCount, const VkBufferCopy* pRegions, ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordCmdCopyBuffer(VkCommandBuffer commandBuffer, VkBuffer srcBuffer, VkBuffer dstBuffer, uint32_t regionCount,
                                        const VkBufferCopy* pRegions, const RecordObject& record_obj) {}
virtual void PostCallRecordCmdCopyBuffer(VkCommandBuffer commandBuffer, VkBuffer srcBuffer, VkBuffer dstBuffer,
                                         uint32_t regionCount, const VkBufferCopy* pRegions, const RecordObject& record_obj) {}
virtual bool PreCallValidateCmdCopyImage(VkCommandBuffer commandBuffer, VkImage srcImage, VkImageLayout srcImageLayout,
                                         VkImage dstImage, VkImageLayout dstImageLayout, uint32_t regionCount,
                                         const VkImageCopy* pRegions, ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordCmdCopyImage(VkCommandBuffer commandBuffer, VkImage srcImage, VkImageLayout srcImageLayout,
                                       VkImage dstImage, VkImageLayout dstImageLayout, uint32_t regionCount,
                                       const VkImageCopy* pRegions, const RecordObject& record_obj) {}
virtual void PostCallRecordCmdCopyImage(VkCommandBuffer commandBuffer, VkImage srcImage, VkImageLayout srcImageLayout,
                                        VkImage dstImage, VkImageLayout dstImageLayout, uint32_t regionCount,
                                        const VkImageCopy* pRegions, const RecordObject& record_obj) {}
virtual bool PreCallValidateCmdBlitImage(VkCommandBuffer commandBuffer, VkImage srcImage, VkImageLayout srcImageLayout,
                                         VkImage dstImage, VkImageLayout dstImageLayout, uint32_t regionCount,
                                         const VkImageBlit* pRegions, VkFilter filter, ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordCmdBlitImage(VkCommandBuffer commandBuffer, VkImage srcImage, VkImageLayout srcImageLayout,
                                       VkImage dstImage, VkImageLayout dstImageLayout, uint32_t regionCount,
                                       const VkImageBlit* pRegions, VkFilter filter, const RecordObject& record_obj) {}
virtual void PostCallRecordCmdBlitImage(VkCommandBuffer commandBuffer, VkImage srcImage, VkImageLayout srcImageLayout,
                                        VkImage dstImage, VkImageLayout dstImageLayout, uint32_t regionCount,
                                        const VkImageBlit* pRegions, VkFilter filter, const RecordObject& record_obj) {}
virtual bool PreCallValidateCmdCopyBufferToImage(VkCommandBuffer commandBuffer, VkBuffer srcBuffer, VkImage dstImage,
                                                 VkImageLayout dstImageLayout, uint32_t regionCount,
                                                 const VkBufferImageCopy* pRegions, ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordCmdCopyBufferToImage(VkCommandBuffer commandBuffer, VkBuffer srcBuffer, VkImage dstImage,
                                               VkImageLayout dstImageLayout, uint32_t regionCount,
                                               const VkBufferImageCopy* pRegions, const RecordObject& record_obj) {}
virtual void PostCallRecordCmdCopyBufferToImage(VkCommandBuffer commandBuffer, VkBuffer srcBuffer, VkImage dstImage,
                                                VkImageLayout dstImageLayout, uint32_t regionCount,
                                                const VkBufferImageCopy* pRegions, const RecordObject& record_obj) {}
virtual bool PreCallValidateCmdCopyImageToBuffer(VkCommandBuffer commandBuffer, VkImage srcImage, VkImageLayout srcImageLayout,
                                                 VkBuffer dstBuffer, uint32_t regionCount, const VkBufferImageCopy* pRegions,
                                                 ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordCmdCopyImageToBuffer(VkCommandBuffer commandBuffer, VkImage srcImage, VkImageLayout srcImageLayout,
                                               VkBuffer dstBuffer, uint32_t regionCount, const VkBufferImageCopy* pRegions,
                                               const RecordObject& record_obj) {}
virtual void PostCallRecordCmdCopyImageToBuffer(VkCommandBuffer commandBuffer, VkImage srcImage, VkImageLayout srcImageLayout,
                                                VkBuffer dstBuffer, uint32_t regionCount, const VkBufferImageCopy* pRegions,
                                                const RecordObject& record_obj) {}
virtual bool PreCallValidateCmdUpdateBuffer(VkCommandBuffer commandBuffer, VkBuffer dstBuffer, VkDeviceSize dstOffset,
                                            VkDeviceSize dataSize, const void* pData, ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordCmdUpdateBuffer(VkCommandBuffer commandBuffer, VkBuffer dstBuffer, VkDeviceSize dstOffset,
                                          VkDeviceSize dataSize, const void* pData, const RecordObject& record_obj) {}
virtual void PostCallRecordCmdUpdateBuffer(VkCommandBuffer commandBuffer, VkBuffer dstBuffer, VkDeviceSize dstOffset,
                                           VkDeviceSize dataSize, const void* pData, const RecordObject& record_obj) {}
virtual bool PreCallValidateCmdFillBuffer(VkCommandBuffer commandBuffer, VkBuffer dstBuffer, VkDeviceSize dstOffset,
                                          VkDeviceSize size, uint32_t data, ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordCmdFillBuffer(VkCommandBuffer commandBuffer, VkBuffer dstBuffer, VkDeviceSize dstOffset,
                                        VkDeviceSize size, uint32_t data, const RecordObject& record_obj) {}
virtual void PostCallRecordCmdFillBuffer(VkCommandBuffer commandBuffer, VkBuffer dstBuffer, VkDeviceSize dstOffset,
                                         VkDeviceSize size, uint32_t data, const RecordObject& record_obj) {}
virtual bool PreCallValidateCmdClearColorImage(VkCommandBuffer commandBuffer, VkImage image, VkImageLayout imageLayout,
                                               const VkClearColorValue* pColor, uint32_t rangeCount,
                                               const VkImageSubresourceRange* pRanges, ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordCmdClearColorImage(VkCommandBuffer commandBuffer, VkImage image, VkImageLayout imageLayout,
                                             const VkClearColorValue* pColor, uint32_t rangeCount,
                                             const VkImageSubresourceRange* pRanges, const RecordObject& record_obj) {}
virtual void PostCallRecordCmdClearColorImage(VkCommandBuffer commandBuffer, VkImage image, VkImageLayout imageLayout,
                                              const VkClearColorValue* pColor, uint32_t rangeCount,
                                              const VkImageSubresourceRange* pRanges, const RecordObject& record_obj) {}
virtual bool PreCallValidateCmdClearDepthStencilImage(VkCommandBuffer commandBuffer, VkImage image, VkImageLayout imageLayout,
                                                      const VkClearDepthStencilValue* pDepthStencil, uint32_t rangeCount,
                                                      const VkImageSubresourceRange* pRanges, ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordCmdClearDepthStencilImage(VkCommandBuffer commandBuffer, VkImage image, VkImageLayout imageLayout,
                                                    const VkClearDepthStencilValue* pDepthStencil, uint32_t rangeCount,
                                                    const VkImageSubresourceRange* pRanges, const RecordObject& record_obj) {}
virtual void PostCallRecordCmdClearDepthStencilImage(VkCommandBuffer commandBuffer, VkImage image, VkImageLayout imageLayout,
                                                     const VkClearDepthStencilValue* pDepthStencil, uint32_t rangeCount,
                                                     const VkImageSubresourceRange* pRanges, const RecordObject& record_obj) {}
virtual bool PreCallValidateCmdClearAttachments(VkCommandBuffer commandBuffer, uint32_t attachmentCount,
                                                const VkClearAttachment* pAttachments, uint32_t rectCount,
                                                const VkClearRect* pRects, ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordCmdClearAttachments(VkCommandBuffer commandBuffer, uint32_t attachmentCount,
                                              const VkClearAttachment* pAttachments, uint32_t rectCount, const VkClearRect* pRects,
                                              const RecordObject& record_obj) {}
virtual void PostCallRecordCmdClearAttachments(VkCommandBuffer commandBuffer, uint32_t attachmentCount,
                                               const VkClearAttachment* pAttachments, uint32_t rectCount, const VkClearRect* pRects,
                                               const RecordObject& record_obj) {}
virtual bool PreCallValidateCmdResolveImage(VkCommandBuffer commandBuffer, VkImage srcImage, VkImageLayout srcImageLayout,
                                            VkImage dstImage, VkImageLayout dstImageLayout, uint32_t regionCount,
                                            const VkImageResolve* pRegions, ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordCmdResolveImage(VkCommandBuffer commandBuffer, VkImage srcImage, VkImageLayout srcImageLayout,
                                          VkImage dstImage, VkImageLayout dstImageLayout, uint32_t regionCount,
                                          const VkImageResolve* pRegions, const RecordObject& record_obj) {}
virtual void PostCallRecordCmdResolveImage(VkCommandBuffer commandBuffer, VkImage srcImage, VkImageLayout srcImageLayout,
                                           VkImage dstImage, VkImageLayout dstImageLayout, uint32_t regionCount,
                                           const VkImageResolve* pRegions, const RecordObject& record_obj) {}
virtual bool PreCallValidateCmdSetEvent(VkCommandBuffer commandBuffer, VkEvent event, VkPipelineStageFlags stageMask,
                                        ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordCmdSetEvent(VkCommandBuffer commandBuffer, VkEvent event, VkPipelineStageFlags stageMask,
                                      const RecordObject& record_obj) {}
virtual void PostCallRecordCmdSetEvent(VkCommandBuffer commandBuffer, VkEvent event, VkPipelineStageFlags stageMask,
                                       const RecordObject& record_obj) {}
virtual bool PreCallValidateCmdResetEvent(VkCommandBuffer commandBuffer, VkEvent event, VkPipelineStageFlags stageMask,
                                          ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordCmdResetEvent(VkCommandBuffer commandBuffer, VkEvent event, VkPipelineStageFlags stageMask,
                                        const RecordObject& record_obj) {}
virtual void PostCallRecordCmdResetEvent(VkCommandBuffer commandBuffer, VkEvent event, VkPipelineStageFlags stageMask,
                                         const RecordObject& record_obj) {}
virtual bool PreCallValidateCmdWaitEvents(VkCommandBuffer commandBuffer, uint32_t eventCount, const VkEvent* pEvents,
                                          VkPipelineStageFlags srcStageMask, VkPipelineStageFlags dstStageMask,
                                          uint32_t memoryBarrierCount, const VkMemoryBarrier* pMemoryBarriers,
                                          uint32_t bufferMemoryBarrierCount, const VkBufferMemoryBarrier* pBufferMemoryBarriers,
                                          uint32_t imageMemoryBarrierCount, const VkImageMemoryBarrier* pImageMemoryBarriers,
                                          ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordCmdWaitEvents(VkCommandBuffer commandBuffer, uint32_t eventCount, const VkEvent* pEvents,
                                        VkPipelineStageFlags srcStageMask, VkPipelineStageFlags dstStageMask,
                                        uint32_t memoryBarrierCount, const VkMemoryBarrier* pMemoryBarriers,
                                        uint32_t bufferMemoryBarrierCount, const VkBufferMemoryBarrier* pBufferMemoryBarriers,
                                        uint32_t imageMemoryBarrierCount, const VkImageMemoryBarrier* pImageMemoryBarriers,
                                        const RecordObject& record_obj) {}
virtual void PostCallRecordCmdWaitEvents(VkCommandBuffer commandBuffer, uint32_t eventCount, const VkEvent* pEvents,
                                         VkPipelineStageFlags srcStageMask, VkPipelineStageFlags dstStageMask,
                                         uint32_t memoryBarrierCount, const VkMemoryBarrier* pMemoryBarriers,
                                         uint32_t bufferMemoryBarrierCount, const VkBufferMemoryBarrier* pBufferMemoryBarriers,
                                         uint32_t imageMemoryBarrierCount, const VkImageMemoryBarrier* pImageMemoryBarriers,
                                         const RecordObject& record_obj) {}
virtual bool PreCallValidateCmdPipelineBarrier(VkCommandBuffer commandBuffer, VkPipelineStageFlags srcStageMask,
                                               VkPipelineStageFlags dstStageMask, VkDependencyFlags dependencyFlags,
                                               uint32_t memoryBarrierCount, const VkMemoryBarrier* pMemoryBarriers,
                                               uint32_t bufferMemoryBarrierCount,
                                               const VkBufferMemoryBarrier* pBufferMemoryBarriers, uint32_t imageMemoryBarrierCount,
                                               const VkImageMemoryBarrier* pImageMemoryBarriers, ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordCmdPipelineBarrier(VkCommandBuffer commandBuffer, VkPipelineStageFlags srcStageMask,
                                             VkPipelineStageFlags dstStageMask, VkDependencyFlags dependencyFlags,
                                             uint32_t memoryBarrierCount, const VkMemoryBarrier* pMemoryBarriers,
                                             uint32_t bufferMemoryBarrierCount, const VkBufferMemoryBarrier* pBufferMemoryBarriers,
                                             uint32_t imageMemoryBarrierCount, const VkImageMemoryBarrier* pImageMemoryBarriers,
                                             const RecordObject& record_obj) {}
virtual void PostCallRecordCmdPipelineBarrier(VkCommandBuffer commandBuffer, VkPipelineStageFlags srcStageMask,
                                              VkPipelineStageFlags dstStageMask, VkDependencyFlags dependencyFlags,
                                              uint32_t memoryBarrierCount, const VkMemoryBarrier* pMemoryBarriers,
                                              uint32_t bufferMemoryBarrierCount, const VkBufferMemoryBarrier* pBufferMemoryBarriers,
                                              uint32_t imageMemoryBarrierCount, const VkImageMemoryBarrier* pImageMemoryBarriers,
                                              const RecordObject& record_obj) {}
virtual bool PreCallValidateCmdBeginQuery(VkCommandBuffer commandBuffer, VkQueryPool queryPool, uint32_t query,
                                          VkQueryControlFlags flags, ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordCmdBeginQuery(VkCommandBuffer commandBuffer, VkQueryPool queryPool, uint32_t query,
                                        VkQueryControlFlags flags, const RecordObject& record_obj) {}
virtual void PostCallRecordCmdBeginQuery(VkCommandBuffer commandBuffer, VkQueryPool queryPool, uint32_t query,
                                         VkQueryControlFlags flags, const RecordObject& record_obj) {}
virtual bool PreCallValidateCmdEndQuery(VkCommandBuffer commandBuffer, VkQueryPool queryPool, uint32_t query,
                                        ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordCmdEndQuery(VkCommandBuffer commandBuffer, VkQueryPool queryPool, uint32_t query,
                                      const RecordObject& record_obj) {}
virtual void PostCallRecordCmdEndQuery(VkCommandBuffer commandBuffer, VkQueryPool queryPool, uint32_t query,
                                       const RecordObject& record_obj) {}
virtual bool PreCallValidateCmdResetQueryPool(VkCommandBuffer commandBuffer, VkQueryPool queryPool, uint32_t firstQuery,
                                              uint32_t queryCount, ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordCmdResetQueryPool(VkCommandBuffer commandBuffer, VkQueryPool queryPool, uint32_t firstQuery,
                                            uint32_t queryCount, const RecordObject& record_obj) {}
virtual void PostCallRecordCmdResetQueryPool(VkCommandBuffer commandBuffer, VkQueryPool queryPool, uint32_t firstQuery,
                                             uint32_t queryCount, const RecordObject& record_obj) {}
virtual bool PreCallValidateCmdWriteTimestamp(VkCommandBuffer commandBuffer, VkPipelineStageFlagBits pipelineStage,
                                              VkQueryPool queryPool, uint32_t query, ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordCmdWriteTimestamp(VkCommandBuffer commandBuffer, VkPipelineStageFlagBits pipelineStage,
                                            VkQueryPool queryPool, uint32_t query, const RecordObject& record_obj) {}
virtual void PostCallRecordCmdWriteTimestamp(VkCommandBuffer commandBuffer, VkPipelineStageFlagBits pipelineStage,
                                             VkQueryPool queryPool, uint32_t query, const RecordObject& record_obj) {}
virtual bool PreCallValidateCmdCopyQueryPoolResults(VkCommandBuffer commandBuffer, VkQueryPool queryPool, uint32_t firstQuery,
                                                    uint32_t queryCount, VkBuffer dstBuffer, VkDeviceSize dstOffset,
                                                    VkDeviceSize stride, VkQueryResultFlags flags, ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordCmdCopyQueryPoolResults(VkCommandBuffer commandBuffer, VkQueryPool queryPool, uint32_t firstQuery,
                                                  uint32_t queryCount, VkBuffer dstBuffer, VkDeviceSize dstOffset,
                                                  VkDeviceSize stride, VkQueryResultFlags flags, const RecordObject& record_obj) {}
virtual void PostCallRecordCmdCopyQueryPoolResults(VkCommandBuffer commandBuffer, VkQueryPool queryPool, uint32_t firstQuery,
                                                   uint32_t queryCount, VkBuffer dstBuffer, VkDeviceSize dstOffset,
                                                   VkDeviceSize stride, VkQueryResultFlags flags, const RecordObject& record_obj) {}
virtual bool PreCallValidateCmdPushConstants(VkCommandBuffer commandBuffer, VkPipelineLayout layout, VkShaderStageFlags stageFlags,
                                             uint32_t offset, uint32_t size, const void* pValues, ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordCmdPushConstants(VkCommandBuffer commandBuffer, VkPipelineLayout layout, VkShaderStageFlags stageFlags,
                                           uint32_t offset, uint32_t size, const void* pValues, const RecordObject& record_obj) {}
virtual void PostCallRecordCmdPushConstants(VkCommandBuffer commandBuffer, VkPipelineLayout layout, VkShaderStageFlags stageFlags,
                                            uint32_t offset, uint32_t size, const void* pValues, const RecordObject& record_obj) {}
virtual bool PreCallValidateCmdBeginRenderPass(VkCommandBuffer commandBuffer, const VkRenderPassBeginInfo* pRenderPassBegin,
                                               VkSubpassContents contents, ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordCmdBeginRenderPass(VkCommandBuffer commandBuffer, const VkRenderPassBeginInfo* pRenderPassBegin,
                                             VkSubpassContents contents, const RecordObject& record_obj) {}
virtual void PostCallRecordCmdBeginRenderPass(VkCommandBuffer commandBuffer, const VkRenderPassBeginInfo* pRenderPassBegin,
                                              VkSubpassContents contents, const RecordObject& record_obj) {}
virtual bool PreCallValidateCmdNextSubpass(VkCommandBuffer commandBuffer, VkSubpassContents contents,
                                           ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordCmdNextSubpass(VkCommandBuffer commandBuffer, VkSubpassContents contents,
                                         const RecordObject& record_obj) {}
virtual void PostCallRecordCmdNextSubpass(VkCommandBuffer commandBuffer, VkSubpassContents contents,
                                          const RecordObject& record_obj) {}
virtual bool PreCallValidateCmdEndRenderPass(VkCommandBuffer commandBuffer, ErrorObject& error_obj) const { return false; }
virtual void PreCallRecordCmdEndRenderPass(VkCommandBuffer commandBuffer, const RecordObject& record_obj) {}
virtual void PostCallRecordCmdEndRenderPass(VkCommandBuffer commandBuffer, const RecordObject& record_obj) {}
virtual bool PreCallValidateCmdExecuteCommands(VkCommandBuffer commandBuffer, uint32_t commandBufferCount,
                                               const VkCommandBuffer* pCommandBuffers, ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordCmdExecuteCommands(VkCommandBuffer commandBuffer, uint32_t commandBufferCount,
                                             const VkCommandBuffer* pCommandBuffers, const RecordObject& record_obj) {}
virtual void PostCallRecordCmdExecuteCommands(VkCommandBuffer commandBuffer, uint32_t commandBufferCount,
                                              const VkCommandBuffer* pCommandBuffers, const RecordObject& record_obj) {}
virtual bool PreCallValidateBindBufferMemory2(VkDevice device, uint32_t bindInfoCount, const VkBindBufferMemoryInfo* pBindInfos,
                                              ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordBindBufferMemory2(VkDevice device, uint32_t bindInfoCount, const VkBindBufferMemoryInfo* pBindInfos,
                                            const RecordObject& record_obj) {}
virtual void PostCallRecordBindBufferMemory2(VkDevice device, uint32_t bindInfoCount, const VkBindBufferMemoryInfo* pBindInfos,
                                             const RecordObject& record_obj) {}
virtual bool PreCallValidateBindImageMemory2(VkDevice device, uint32_t bindInfoCount, const VkBindImageMemoryInfo* pBindInfos,
                                             ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordBindImageMemory2(VkDevice device, uint32_t bindInfoCount, const VkBindImageMemoryInfo* pBindInfos,
                                           const RecordObject& record_obj) {}
virtual void PostCallRecordBindImageMemory2(VkDevice device, uint32_t bindInfoCount, const VkBindImageMemoryInfo* pBindInfos,
                                            const RecordObject& record_obj) {}
virtual bool PreCallValidateGetDeviceGroupPeerMemoryFeatures(VkDevice device, uint32_t heapIndex, uint32_t localDeviceIndex,
                                                             uint32_t remoteDeviceIndex,
                                                             VkPeerMemoryFeatureFlags* pPeerMemoryFeatures,
                                                             ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordGetDeviceGroupPeerMemoryFeatures(VkDevice device, uint32_t heapIndex, uint32_t localDeviceIndex,
                                                           uint32_t remoteDeviceIndex,
                                                           VkPeerMemoryFeatureFlags* pPeerMemoryFeatures,
                                                           const RecordObject& record_obj) {}
virtual void PostCallRecordGetDeviceGroupPeerMemoryFeatures(VkDevice device, uint32_t heapIndex, uint32_t localDeviceIndex,
                                                            uint32_t remoteDeviceIndex,
                                                            VkPeerMemoryFeatureFlags* pPeerMemoryFeatures,
                                                            const RecordObject& record_obj) {}
virtual bool PreCallValidateCmdSetDeviceMask(VkCommandBuffer commandBuffer, uint32_t deviceMask, ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordCmdSetDeviceMask(VkCommandBuffer commandBuffer, uint32_t deviceMask, const RecordObject& record_obj) {}
virtual void PostCallRecordCmdSetDeviceMask(VkCommandBuffer commandBuffer, uint32_t deviceMask, const RecordObject& record_obj) {}
virtual bool PreCallValidateCmdDispatchBase(VkCommandBuffer commandBuffer, uint32_t baseGroupX, uint32_t baseGroupY,
                                            uint32_t baseGroupZ, uint32_t groupCountX, uint32_t groupCountY, uint32_t groupCountZ,
                                            ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordCmdDispatchBase(VkCommandBuffer commandBuffer, uint32_t baseGroupX, uint32_t baseGroupY,
                                          uint32_t baseGroupZ, uint32_t groupCountX, uint32_t groupCountY, uint32_t groupCountZ,
                                          const RecordObject& record_obj) {}
virtual void PostCallRecordCmdDispatchBase(VkCommandBuffer commandBuffer, uint32_t baseGroupX, uint32_t baseGroupY,
                                           uint32_t baseGroupZ, uint32_t groupCountX, uint32_t groupCountY, uint32_t groupCountZ,
                                           const RecordObject& record_obj) {}
virtual bool PreCallValidateGetImageMemoryRequirements2(VkDevice device, const VkImageMemoryRequirementsInfo2* pInfo,
                                                        VkMemoryRequirements2* pMemoryRequirements, ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordGetImageMemoryRequirements2(VkDevice device, const VkImageMemoryRequirementsInfo2* pInfo,
                                                      VkMemoryRequirements2* pMemoryRequirements, const RecordObject& record_obj) {}
virtual void PostCallRecordGetImageMemoryRequirements2(VkDevice device, const VkImageMemoryRequirementsInfo2* pInfo,
                                                       VkMemoryRequirements2* pMemoryRequirements, const RecordObject& record_obj) {
}
virtual bool PreCallValidateGetBufferMemoryRequirements2(VkDevice device, const VkBufferMemoryRequirementsInfo2* pInfo,
                                                         VkMemoryRequirements2* pMemoryRequirements, ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordGetBufferMemoryRequirements2(VkDevice device, const VkBufferMemoryRequirementsInfo2* pInfo,
                                                       VkMemoryRequirements2* pMemoryRequirements, const RecordObject& record_obj) {
}
virtual void PostCallRecordGetBufferMemoryRequirements2(VkDevice device, const VkBufferMemoryRequirementsInfo2* pInfo,
                                                        VkMemoryRequirements2* pMemoryRequirements,
                                                        const RecordObject& record_obj) {}
virtual bool PreCallValidateGetImageSparseMemoryRequirements2(VkDevice device, const VkImageSparseMemoryRequirementsInfo2* pInfo,
                                                              uint32_t* pSparseMemoryRequirementCount,
                                                              VkSparseImageMemoryRequirements2* pSparseMemoryRequirements,
                                                              ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordGetImageSparseMemoryRequirements2(VkDevice device, const VkImageSparseMemoryRequirementsInfo2* pInfo,
                                                            uint32_t* pSparseMemoryRequirementCount,
                                                            VkSparseImageMemoryRequirements2* pSparseMemoryRequirements,
                                                            const RecordObject& record_obj) {}
virtual void PostCallRecordGetImageSparseMemoryRequirements2(VkDevice device, const VkImageSparseMemoryRequirementsInfo2* pInfo,
                                                             uint32_t* pSparseMemoryRequirementCount,
                                                             VkSparseImageMemoryRequirements2* pSparseMemoryRequirements,
                                                             const RecordObject& record_obj) {}
virtual bool PreCallValidateTrimCommandPool(VkDevice device, VkCommandPool commandPool, VkCommandPoolTrimFlags flags,
                                            ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordTrimCommandPool(VkDevice device, VkCommandPool commandPool, VkCommandPoolTrimFlags flags,
                                          const RecordObject& record_obj) {}
virtual void PostCallRecordTrimCommandPool(VkDevice device, VkCommandPool commandPool, VkCommandPoolTrimFlags flags,
                                           const RecordObject& record_obj) {}
virtual bool PreCallValidateGetDeviceQueue2(VkDevice device, const VkDeviceQueueInfo2* pQueueInfo, VkQueue* pQueue,
                                            ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordGetDeviceQueue2(VkDevice device, const VkDeviceQueueInfo2* pQueueInfo, VkQueue* pQueue,
                                          const RecordObject& record_obj) {}
virtual void PostCallRecordGetDeviceQueue2(VkDevice device, const VkDeviceQueueInfo2* pQueueInfo, VkQueue* pQueue,
                                           const RecordObject& record_obj) {}
virtual bool PreCallValidateCreateSamplerYcbcrConversion(VkDevice device, const VkSamplerYcbcrConversionCreateInfo* pCreateInfo,
                                                         const VkAllocationCallbacks* pAllocator,
                                                         VkSamplerYcbcrConversion* pYcbcrConversion, ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordCreateSamplerYcbcrConversion(VkDevice device, const VkSamplerYcbcrConversionCreateInfo* pCreateInfo,
                                                       const VkAllocationCallbacks* pAllocator,
                                                       VkSamplerYcbcrConversion* pYcbcrConversion, const RecordObject& record_obj) {
}
virtual void PostCallRecordCreateSamplerYcbcrConversion(VkDevice device, const VkSamplerYcbcrConversionCreateInfo* pCreateInfo,
                                                        const VkAllocationCallbacks* pAllocator,
                                                        VkSamplerYcbcrConversion* pYcbcrConversion,
                                                        const RecordObject& record_obj) {}
virtual bool PreCallValidateDestroySamplerYcbcrConversion(VkDevice device, VkSamplerYcbcrConversion ycbcrConversion,
                                                          const VkAllocationCallbacks* pAllocator, ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordDestroySamplerYcbcrConversion(VkDevice device, VkSamplerYcbcrConversion ycbcrConversion,
                                                        const VkAllocationCallbacks* pAllocator, const RecordObject& record_obj) {}
virtual void PostCallRecordDestroySamplerYcbcrConversion(VkDevice device, VkSamplerYcbcrConversion ycbcrConversion,
                                                         const VkAllocationCallbacks* pAllocator, const RecordObject& record_obj) {}
virtual bool PreCallValidateCreateDescriptorUpdateTemplate(VkDevice device, const VkDescriptorUpdateTemplateCreateInfo* pCreateInfo,
                                                           const VkAllocationCallbacks* pAllocator,
                                                           VkDescriptorUpdateTemplate* pDescriptorUpdateTemplate,
                                                           ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordCreateDescriptorUpdateTemplate(VkDevice device, const VkDescriptorUpdateTemplateCreateInfo* pCreateInfo,
                                                         const VkAllocationCallbacks* pAllocator,
                                                         VkDescriptorUpdateTemplate* pDescriptorUpdateTemplate,
                                                         const RecordObject& record_obj) {}
virtual void PostCallRecordCreateDescriptorUpdateTemplate(VkDevice device, const VkDescriptorUpdateTemplateCreateInfo* pCreateInfo,
                                                          const VkAllocationCallbacks* pAllocator,
                                                          VkDescriptorUpdateTemplate* pDescriptorUpdateTemplate,
                                                          const RecordObject& record_obj) {}
virtual bool PreCallValidateDestroyDescriptorUpdateTemplate(VkDevice device, VkDescriptorUpdateTemplate descriptorUpdateTemplate,
                                                            const VkAllocationCallbacks* pAllocator, ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordDestroyDescriptorUpdateTemplate(VkDevice device, VkDescriptorUpdateTemplate descriptorUpdateTemplate,
                                                          const VkAllocationCallbacks* pAllocator, const RecordObject& record_obj) {
}
virtual void PostCallRecordDestroyDescriptorUpdateTemplate(VkDevice device, VkDescriptorUpdateTemplate descriptorUpdateTemplate,
                                                           const VkAllocationCallbacks* pAllocator,
                                                           const RecordObject& record_obj) {}
virtual bool PreCallValidateUpdateDescriptorSetWithTemplate(VkDevice device, VkDescriptorSet descriptorSet,
                                                            VkDescriptorUpdateTemplate descriptorUpdateTemplate, const void* pData,
                                                            ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordUpdateDescriptorSetWithTemplate(VkDevice device, VkDescriptorSet descriptorSet,
                                                          VkDescriptorUpdateTemplate descriptorUpdateTemplate, const void* pData,
                                                          const RecordObject& record_obj) {}
virtual void PostCallRecordUpdateDescriptorSetWithTemplate(VkDevice device, VkDescriptorSet descriptorSet,
                                                           VkDescriptorUpdateTemplate descriptorUpdateTemplate, const void* pData,
                                                           const RecordObject& record_obj) {}
virtual bool PreCallValidateGetDescriptorSetLayoutSupport(VkDevice device, const VkDescriptorSetLayoutCreateInfo* pCreateInfo,
                                                          VkDescriptorSetLayoutSupport* pSupport, ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordGetDescriptorSetLayoutSupport(VkDevice device, const VkDescriptorSetLayoutCreateInfo* pCreateInfo,
                                                        VkDescriptorSetLayoutSupport* pSupport, const RecordObject& record_obj) {}
virtual void PostCallRecordGetDescriptorSetLayoutSupport(VkDevice device, const VkDescriptorSetLayoutCreateInfo* pCreateInfo,
                                                         VkDescriptorSetLayoutSupport* pSupport, const RecordObject& record_obj) {}
virtual bool PreCallValidateCmdDrawIndirectCount(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                 VkBuffer countBuffer, VkDeviceSize countBufferOffset, uint32_t maxDrawCount,
                                                 uint32_t stride, ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordCmdDrawIndirectCount(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                               VkBuffer countBuffer, VkDeviceSize countBufferOffset, uint32_t maxDrawCount,
                                               uint32_t stride, const RecordObject& record_obj) {}
virtual void PostCallRecordCmdDrawIndirectCount(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                VkBuffer countBuffer, VkDeviceSize countBufferOffset, uint32_t maxDrawCount,
                                                uint32_t stride, const RecordObject& record_obj) {}
virtual bool PreCallValidateCmdDrawIndexedIndirectCount(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                        VkBuffer countBuffer, VkDeviceSize countBufferOffset, uint32_t maxDrawCount,
                                                        uint32_t stride, ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordCmdDrawIndexedIndirectCount(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                      VkBuffer countBuffer, VkDeviceSize countBufferOffset, uint32_t maxDrawCount,
                                                      uint32_t stride, const RecordObject& record_obj) {}
virtual void PostCallRecordCmdDrawIndexedIndirectCount(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                       VkBuffer countBuffer, VkDeviceSize countBufferOffset, uint32_t maxDrawCount,
                                                       uint32_t stride, const RecordObject& record_obj) {}
virtual bool PreCallValidateCreateRenderPass2(VkDevice device, const VkRenderPassCreateInfo2* pCreateInfo,
                                              const VkAllocationCallbacks* pAllocator, VkRenderPass* pRenderPass,
                                              ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordCreateRenderPass2(VkDevice device, const VkRenderPassCreateInfo2* pCreateInfo,
                                            const VkAllocationCallbacks* pAllocator, VkRenderPass* pRenderPass,
                                            const RecordObject& record_obj) {}
virtual void PostCallRecordCreateRenderPass2(VkDevice device, const VkRenderPassCreateInfo2* pCreateInfo,
                                             const VkAllocationCallbacks* pAllocator, VkRenderPass* pRenderPass,
                                             const RecordObject& record_obj) {}
virtual bool PreCallValidateCmdBeginRenderPass2(VkCommandBuffer commandBuffer, const VkRenderPassBeginInfo* pRenderPassBegin,
                                                const VkSubpassBeginInfo* pSubpassBeginInfo, ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordCmdBeginRenderPass2(VkCommandBuffer commandBuffer, const VkRenderPassBeginInfo* pRenderPassBegin,
                                              const VkSubpassBeginInfo* pSubpassBeginInfo, const RecordObject& record_obj) {}
virtual void PostCallRecordCmdBeginRenderPass2(VkCommandBuffer commandBuffer, const VkRenderPassBeginInfo* pRenderPassBegin,
                                               const VkSubpassBeginInfo* pSubpassBeginInfo, const RecordObject& record_obj) {}
virtual bool PreCallValidateCmdNextSubpass2(VkCommandBuffer commandBuffer, const VkSubpassBeginInfo* pSubpassBeginInfo,
                                            const VkSubpassEndInfo* pSubpassEndInfo, ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordCmdNextSubpass2(VkCommandBuffer commandBuffer, const VkSubpassBeginInfo* pSubpassBeginInfo,
                                          const VkSubpassEndInfo* pSubpassEndInfo, const RecordObject& record_obj) {}
virtual void PostCallRecordCmdNextSubpass2(VkCommandBuffer commandBuffer, const VkSubpassBeginInfo* pSubpassBeginInfo,
                                           const VkSubpassEndInfo* pSubpassEndInfo, const RecordObject& record_obj) {}
virtual bool PreCallValidateCmdEndRenderPass2(VkCommandBuffer commandBuffer, const VkSubpassEndInfo* pSubpassEndInfo,
                                              ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordCmdEndRenderPass2(VkCommandBuffer commandBuffer, const VkSubpassEndInfo* pSubpassEndInfo,
                                            const RecordObject& record_obj) {}
virtual void PostCallRecordCmdEndRenderPass2(VkCommandBuffer commandBuffer, const VkSubpassEndInfo* pSubpassEndInfo,
                                             const RecordObject& record_obj) {}
virtual bool PreCallValidateResetQueryPool(VkDevice device, VkQueryPool queryPool, uint32_t firstQuery, uint32_t queryCount,
                                           ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordResetQueryPool(VkDevice device, VkQueryPool queryPool, uint32_t firstQuery, uint32_t queryCount,
                                         const RecordObject& record_obj) {}
virtual void PostCallRecordResetQueryPool(VkDevice device, VkQueryPool queryPool, uint32_t firstQuery, uint32_t queryCount,
                                          const RecordObject& record_obj) {}
virtual bool PreCallValidateGetSemaphoreCounterValue(VkDevice device, VkSemaphore semaphore, uint64_t* pValue,
                                                     ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordGetSemaphoreCounterValue(VkDevice device, VkSemaphore semaphore, uint64_t* pValue,
                                                   const RecordObject& record_obj) {}
virtual void PostCallRecordGetSemaphoreCounterValue(VkDevice device, VkSemaphore semaphore, uint64_t* pValue,
                                                    const RecordObject& record_obj) {}
virtual bool PreCallValidateWaitSemaphores(VkDevice device, const VkSemaphoreWaitInfo* pWaitInfo, uint64_t timeout,
                                           ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordWaitSemaphores(VkDevice device, const VkSemaphoreWaitInfo* pWaitInfo, uint64_t timeout,
                                         const RecordObject& record_obj) {}
virtual void PostCallRecordWaitSemaphores(VkDevice device, const VkSemaphoreWaitInfo* pWaitInfo, uint64_t timeout,
                                          const RecordObject& record_obj) {}
virtual bool PreCallValidateSignalSemaphore(VkDevice device, const VkSemaphoreSignalInfo* pSignalInfo,
                                            ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordSignalSemaphore(VkDevice device, const VkSemaphoreSignalInfo* pSignalInfo,
                                          const RecordObject& record_obj) {}
virtual void PostCallRecordSignalSemaphore(VkDevice device, const VkSemaphoreSignalInfo* pSignalInfo,
                                           const RecordObject& record_obj) {}
virtual bool PreCallValidateGetBufferDeviceAddress(VkDevice device, const VkBufferDeviceAddressInfo* pInfo,
                                                   ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordGetBufferDeviceAddress(VkDevice device, const VkBufferDeviceAddressInfo* pInfo,
                                                 const RecordObject& record_obj) {}
virtual void PostCallRecordGetBufferDeviceAddress(VkDevice device, const VkBufferDeviceAddressInfo* pInfo,
                                                  const RecordObject& record_obj) {}
virtual bool PreCallValidateGetBufferOpaqueCaptureAddress(VkDevice device, const VkBufferDeviceAddressInfo* pInfo,
                                                          ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordGetBufferOpaqueCaptureAddress(VkDevice device, const VkBufferDeviceAddressInfo* pInfo,
                                                        const RecordObject& record_obj) {}
virtual void PostCallRecordGetBufferOpaqueCaptureAddress(VkDevice device, const VkBufferDeviceAddressInfo* pInfo,
                                                         const RecordObject& record_obj) {}
virtual bool PreCallValidateGetDeviceMemoryOpaqueCaptureAddress(VkDevice device,
                                                                const VkDeviceMemoryOpaqueCaptureAddressInfo* pInfo,
                                                                ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordGetDeviceMemoryOpaqueCaptureAddress(VkDevice device, const VkDeviceMemoryOpaqueCaptureAddressInfo* pInfo,
                                                              const RecordObject& record_obj) {}
virtual void PostCallRecordGetDeviceMemoryOpaqueCaptureAddress(VkDevice device, const VkDeviceMemoryOpaqueCaptureAddressInfo* pInfo,
                                                               const RecordObject& record_obj) {}
virtual bool PreCallValidateCreatePrivateDataSlot(VkDevice device, const VkPrivateDataSlotCreateInfo* pCreateInfo,
                                                  const VkAllocationCallbacks* pAllocator, VkPrivateDataSlot* pPrivateDataSlot,
                                                  ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordCreatePrivateDataSlot(VkDevice device, const VkPrivateDataSlotCreateInfo* pCreateInfo,
                                                const VkAllocationCallbacks* pAllocator, VkPrivateDataSlot* pPrivateDataSlot,
                                                const RecordObject& record_obj) {}
virtual void PostCallRecordCreatePrivateDataSlot(VkDevice device, const VkPrivateDataSlotCreateInfo* pCreateInfo,
                                                 const VkAllocationCallbacks* pAllocator, VkPrivateDataSlot* pPrivateDataSlot,
                                                 const RecordObject& record_obj) {}
virtual bool PreCallValidateDestroyPrivateDataSlot(VkDevice device, VkPrivateDataSlot privateDataSlot,
                                                   const VkAllocationCallbacks* pAllocator, ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordDestroyPrivateDataSlot(VkDevice device, VkPrivateDataSlot privateDataSlot,
                                                 const VkAllocationCallbacks* pAllocator, const RecordObject& record_obj) {}
virtual void PostCallRecordDestroyPrivateDataSlot(VkDevice device, VkPrivateDataSlot privateDataSlot,
                                                  const VkAllocationCallbacks* pAllocator, const RecordObject& record_obj) {}
virtual bool PreCallValidateSetPrivateData(VkDevice device, VkObjectType objectType, uint64_t objectHandle,
                                           VkPrivateDataSlot privateDataSlot, uint64_t data, ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordSetPrivateData(VkDevice device, VkObjectType objectType, uint64_t objectHandle,
                                         VkPrivateDataSlot privateDataSlot, uint64_t data, const RecordObject& record_obj) {}
virtual void PostCallRecordSetPrivateData(VkDevice device, VkObjectType objectType, uint64_t objectHandle,
                                          VkPrivateDataSlot privateDataSlot, uint64_t data, const RecordObject& record_obj) {}
virtual bool PreCallValidateGetPrivateData(VkDevice device, VkObjectType objectType, uint64_t objectHandle,
                                           VkPrivateDataSlot privateDataSlot, uint64_t* pData, ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordGetPrivateData(VkDevice device, VkObjectType objectType, uint64_t objectHandle,
                                         VkPrivateDataSlot privateDataSlot, uint64_t* pData, const RecordObject& record_obj) {}
virtual void PostCallRecordGetPrivateData(VkDevice device, VkObjectType objectType, uint64_t objectHandle,
                                          VkPrivateDataSlot privateDataSlot, uint64_t* pData, const RecordObject& record_obj) {}
virtual bool PreCallValidateCmdSetEvent2(VkCommandBuffer commandBuffer, VkEvent event, const VkDependencyInfo* pDependencyInfo,
                                         ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordCmdSetEvent2(VkCommandBuffer commandBuffer, VkEvent event, const VkDependencyInfo* pDependencyInfo,
                                       const RecordObject& record_obj) {}
virtual void PostCallRecordCmdSetEvent2(VkCommandBuffer commandBuffer, VkEvent event, const VkDependencyInfo* pDependencyInfo,
                                        const RecordObject& record_obj) {}
virtual bool PreCallValidateCmdResetEvent2(VkCommandBuffer commandBuffer, VkEvent event, VkPipelineStageFlags2 stageMask,
                                           ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordCmdResetEvent2(VkCommandBuffer commandBuffer, VkEvent event, VkPipelineStageFlags2 stageMask,
                                         const RecordObject& record_obj) {}
virtual void PostCallRecordCmdResetEvent2(VkCommandBuffer commandBuffer, VkEvent event, VkPipelineStageFlags2 stageMask,
                                          const RecordObject& record_obj) {}
virtual bool PreCallValidateCmdWaitEvents2(VkCommandBuffer commandBuffer, uint32_t eventCount, const VkEvent* pEvents,
                                           const VkDependencyInfo* pDependencyInfos, ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordCmdWaitEvents2(VkCommandBuffer commandBuffer, uint32_t eventCount, const VkEvent* pEvents,
                                         const VkDependencyInfo* pDependencyInfos, const RecordObject& record_obj) {}
virtual void PostCallRecordCmdWaitEvents2(VkCommandBuffer commandBuffer, uint32_t eventCount, const VkEvent* pEvents,
                                          const VkDependencyInfo* pDependencyInfos, const RecordObject& record_obj) {}
virtual bool PreCallValidateCmdPipelineBarrier2(VkCommandBuffer commandBuffer, const VkDependencyInfo* pDependencyInfo,
                                                ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordCmdPipelineBarrier2(VkCommandBuffer commandBuffer, const VkDependencyInfo* pDependencyInfo,
                                              const RecordObject& record_obj) {}
virtual void PostCallRecordCmdPipelineBarrier2(VkCommandBuffer commandBuffer, const VkDependencyInfo* pDependencyInfo,
                                               const RecordObject& record_obj) {}
virtual bool PreCallValidateCmdWriteTimestamp2(VkCommandBuffer commandBuffer, VkPipelineStageFlags2 stage, VkQueryPool queryPool,
                                               uint32_t query, ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordCmdWriteTimestamp2(VkCommandBuffer commandBuffer, VkPipelineStageFlags2 stage, VkQueryPool queryPool,
                                             uint32_t query, const RecordObject& record_obj) {}
virtual void PostCallRecordCmdWriteTimestamp2(VkCommandBuffer commandBuffer, VkPipelineStageFlags2 stage, VkQueryPool queryPool,
                                              uint32_t query, const RecordObject& record_obj) {}
virtual bool PreCallValidateQueueSubmit2(VkQueue queue, uint32_t submitCount, const VkSubmitInfo2* pSubmits, VkFence fence,
                                         ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordQueueSubmit2(VkQueue queue, uint32_t submitCount, const VkSubmitInfo2* pSubmits, VkFence fence,
                                       const RecordObject& record_obj) {}
virtual void PostCallRecordQueueSubmit2(VkQueue queue, uint32_t submitCount, const VkSubmitInfo2* pSubmits, VkFence fence,
                                        const RecordObject& record_obj) {}
virtual bool PreCallValidateCmdCopyBuffer2(VkCommandBuffer commandBuffer, const VkCopyBufferInfo2* pCopyBufferInfo,
                                           ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordCmdCopyBuffer2(VkCommandBuffer commandBuffer, const VkCopyBufferInfo2* pCopyBufferInfo,
                                         const RecordObject& record_obj) {}
virtual void PostCallRecordCmdCopyBuffer2(VkCommandBuffer commandBuffer, const VkCopyBufferInfo2* pCopyBufferInfo,
                                          const RecordObject& record_obj) {}
virtual bool PreCallValidateCmdCopyImage2(VkCommandBuffer commandBuffer, const VkCopyImageInfo2* pCopyImageInfo,
                                          ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordCmdCopyImage2(VkCommandBuffer commandBuffer, const VkCopyImageInfo2* pCopyImageInfo,
                                        const RecordObject& record_obj) {}
virtual void PostCallRecordCmdCopyImage2(VkCommandBuffer commandBuffer, const VkCopyImageInfo2* pCopyImageInfo,
                                         const RecordObject& record_obj) {}
virtual bool PreCallValidateCmdCopyBufferToImage2(VkCommandBuffer commandBuffer,
                                                  const VkCopyBufferToImageInfo2* pCopyBufferToImageInfo,
                                                  ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordCmdCopyBufferToImage2(VkCommandBuffer commandBuffer,
                                                const VkCopyBufferToImageInfo2* pCopyBufferToImageInfo,
                                                const RecordObject& record_obj) {}
virtual void PostCallRecordCmdCopyBufferToImage2(VkCommandBuffer commandBuffer,
                                                 const VkCopyBufferToImageInfo2* pCopyBufferToImageInfo,
                                                 const RecordObject& record_obj) {}
virtual bool PreCallValidateCmdCopyImageToBuffer2(VkCommandBuffer commandBuffer,
                                                  const VkCopyImageToBufferInfo2* pCopyImageToBufferInfo,
                                                  ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordCmdCopyImageToBuffer2(VkCommandBuffer commandBuffer,
                                                const VkCopyImageToBufferInfo2* pCopyImageToBufferInfo,
                                                const RecordObject& record_obj) {}
virtual void PostCallRecordCmdCopyImageToBuffer2(VkCommandBuffer commandBuffer,
                                                 const VkCopyImageToBufferInfo2* pCopyImageToBufferInfo,
                                                 const RecordObject& record_obj) {}
virtual bool PreCallValidateCmdBlitImage2(VkCommandBuffer commandBuffer, const VkBlitImageInfo2* pBlitImageInfo,
                                          ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordCmdBlitImage2(VkCommandBuffer commandBuffer, const VkBlitImageInfo2* pBlitImageInfo,
                                        const RecordObject& record_obj) {}
virtual void PostCallRecordCmdBlitImage2(VkCommandBuffer commandBuffer, const VkBlitImageInfo2* pBlitImageInfo,
                                         const RecordObject& record_obj) {}
virtual bool PreCallValidateCmdResolveImage2(VkCommandBuffer commandBuffer, const VkResolveImageInfo2* pResolveImageInfo,
                                             ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordCmdResolveImage2(VkCommandBuffer commandBuffer, const VkResolveImageInfo2* pResolveImageInfo,
                                           const RecordObject& record_obj) {}
virtual void PostCallRecordCmdResolveImage2(VkCommandBuffer commandBuffer, const VkResolveImageInfo2* pResolveImageInfo,
                                            const RecordObject& record_obj) {}
virtual bool PreCallValidateCmdBeginRendering(VkCommandBuffer commandBuffer, const VkRenderingInfo* pRenderingInfo,
                                              ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordCmdBeginRendering(VkCommandBuffer commandBuffer, const VkRenderingInfo* pRenderingInfo,
                                            const RecordObject& record_obj) {}
virtual void PostCallRecordCmdBeginRendering(VkCommandBuffer commandBuffer, const VkRenderingInfo* pRenderingInfo,
                                             const RecordObject& record_obj) {}
virtual bool PreCallValidateCmdEndRendering(VkCommandBuffer commandBuffer, ErrorObject& error_obj) const { return false; }
virtual void PreCallRecordCmdEndRendering(VkCommandBuffer commandBuffer, const RecordObject& record_obj) {}
virtual void PostCallRecordCmdEndRendering(VkCommandBuffer commandBuffer, const RecordObject& record_obj) {}
virtual bool PreCallValidateCmdSetCullMode(VkCommandBuffer commandBuffer, VkCullModeFlags cullMode, ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordCmdSetCullMode(VkCommandBuffer commandBuffer, VkCullModeFlags cullMode, const RecordObject& record_obj) {}
virtual void PostCallRecordCmdSetCullMode(VkCommandBuffer commandBuffer, VkCullModeFlags cullMode, const RecordObject& record_obj) {
}
virtual bool PreCallValidateCmdSetFrontFace(VkCommandBuffer commandBuffer, VkFrontFace frontFace, ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordCmdSetFrontFace(VkCommandBuffer commandBuffer, VkFrontFace frontFace, const RecordObject& record_obj) {}
virtual void PostCallRecordCmdSetFrontFace(VkCommandBuffer commandBuffer, VkFrontFace frontFace, const RecordObject& record_obj) {}
virtual bool PreCallValidateCmdSetPrimitiveTopology(VkCommandBuffer commandBuffer, VkPrimitiveTopology primitiveTopology,
                                                    ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordCmdSetPrimitiveTopology(VkCommandBuffer commandBuffer, VkPrimitiveTopology primitiveTopology,
                                                  const RecordObject& record_obj) {}
virtual void PostCallRecordCmdSetPrimitiveTopology(VkCommandBuffer commandBuffer, VkPrimitiveTopology primitiveTopology,
                                                   const RecordObject& record_obj) {}
virtual bool PreCallValidateCmdSetViewportWithCount(VkCommandBuffer commandBuffer, uint32_t viewportCount,
                                                    const VkViewport* pViewports, ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordCmdSetViewportWithCount(VkCommandBuffer commandBuffer, uint32_t viewportCount,
                                                  const VkViewport* pViewports, const RecordObject& record_obj) {}
virtual void PostCallRecordCmdSetViewportWithCount(VkCommandBuffer commandBuffer, uint32_t viewportCount,
                                                   const VkViewport* pViewports, const RecordObject& record_obj) {}
virtual bool PreCallValidateCmdSetScissorWithCount(VkCommandBuffer commandBuffer, uint32_t scissorCount, const VkRect2D* pScissors,
                                                   ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordCmdSetScissorWithCount(VkCommandBuffer commandBuffer, uint32_t scissorCount, const VkRect2D* pScissors,
                                                 const RecordObject& record_obj) {}
virtual void PostCallRecordCmdSetScissorWithCount(VkCommandBuffer commandBuffer, uint32_t scissorCount, const VkRect2D* pScissors,
                                                  const RecordObject& record_obj) {}
virtual bool PreCallValidateCmdBindVertexBuffers2(VkCommandBuffer commandBuffer, uint32_t firstBinding, uint32_t bindingCount,
                                                  const VkBuffer* pBuffers, const VkDeviceSize* pOffsets,
                                                  const VkDeviceSize* pSizes, const VkDeviceSize* pStrides,
                                                  ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordCmdBindVertexBuffers2(VkCommandBuffer commandBuffer, uint32_t firstBinding, uint32_t bindingCount,
                                                const VkBuffer* pBuffers, const VkDeviceSize* pOffsets, const VkDeviceSize* pSizes,
                                                const VkDeviceSize* pStrides, const RecordObject& record_obj) {}
virtual void PostCallRecordCmdBindVertexBuffers2(VkCommandBuffer commandBuffer, uint32_t firstBinding, uint32_t bindingCount,
                                                 const VkBuffer* pBuffers, const VkDeviceSize* pOffsets, const VkDeviceSize* pSizes,
                                                 const VkDeviceSize* pStrides, const RecordObject& record_obj) {}
virtual bool PreCallValidateCmdSetDepthTestEnable(VkCommandBuffer commandBuffer, VkBool32 depthTestEnable,
                                                  ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordCmdSetDepthTestEnable(VkCommandBuffer commandBuffer, VkBool32 depthTestEnable,
                                                const RecordObject& record_obj) {}
virtual void PostCallRecordCmdSetDepthTestEnable(VkCommandBuffer commandBuffer, VkBool32 depthTestEnable,
                                                 const RecordObject& record_obj) {}
virtual bool PreCallValidateCmdSetDepthWriteEnable(VkCommandBuffer commandBuffer, VkBool32 depthWriteEnable,
                                                   ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordCmdSetDepthWriteEnable(VkCommandBuffer commandBuffer, VkBool32 depthWriteEnable,
                                                 const RecordObject& record_obj) {}
virtual void PostCallRecordCmdSetDepthWriteEnable(VkCommandBuffer commandBuffer, VkBool32 depthWriteEnable,
                                                  const RecordObject& record_obj) {}
virtual bool PreCallValidateCmdSetDepthCompareOp(VkCommandBuffer commandBuffer, VkCompareOp depthCompareOp,
                                                 ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordCmdSetDepthCompareOp(VkCommandBuffer commandBuffer, VkCompareOp depthCompareOp,
                                               const RecordObject& record_obj) {}
virtual void PostCallRecordCmdSetDepthCompareOp(VkCommandBuffer commandBuffer, VkCompareOp depthCompareOp,
                                                const RecordObject& record_obj) {}
virtual bool PreCallValidateCmdSetDepthBoundsTestEnable(VkCommandBuffer commandBuffer, VkBool32 depthBoundsTestEnable,
                                                        ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordCmdSetDepthBoundsTestEnable(VkCommandBuffer commandBuffer, VkBool32 depthBoundsTestEnable,
                                                      const RecordObject& record_obj) {}
virtual void PostCallRecordCmdSetDepthBoundsTestEnable(VkCommandBuffer commandBuffer, VkBool32 depthBoundsTestEnable,
                                                       const RecordObject& record_obj) {}
virtual bool PreCallValidateCmdSetStencilTestEnable(VkCommandBuffer commandBuffer, VkBool32 stencilTestEnable,
                                                    ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordCmdSetStencilTestEnable(VkCommandBuffer commandBuffer, VkBool32 stencilTestEnable,
                                                  const RecordObject& record_obj) {}
virtual void PostCallRecordCmdSetStencilTestEnable(VkCommandBuffer commandBuffer, VkBool32 stencilTestEnable,
                                                   const RecordObject& record_obj) {}
virtual bool PreCallValidateCmdSetStencilOp(VkCommandBuffer commandBuffer, VkStencilFaceFlags faceMask, VkStencilOp failOp,
                                            VkStencilOp passOp, VkStencilOp depthFailOp, VkCompareOp compareOp,
                                            ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordCmdSetStencilOp(VkCommandBuffer commandBuffer, VkStencilFaceFlags faceMask, VkStencilOp failOp,
                                          VkStencilOp passOp, VkStencilOp depthFailOp, VkCompareOp compareOp,
                                          const RecordObject& record_obj) {}
virtual void PostCallRecordCmdSetStencilOp(VkCommandBuffer commandBuffer, VkStencilFaceFlags faceMask, VkStencilOp failOp,
                                           VkStencilOp passOp, VkStencilOp depthFailOp, VkCompareOp compareOp,
                                           const RecordObject& record_obj) {}
virtual bool PreCallValidateCmdSetRasterizerDiscardEnable(VkCommandBuffer commandBuffer, VkBool32 rasterizerDiscardEnable,
                                                          ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordCmdSetRasterizerDiscardEnable(VkCommandBuffer commandBuffer, VkBool32 rasterizerDiscardEnable,
                                                        const RecordObject& record_obj) {}
virtual void PostCallRecordCmdSetRasterizerDiscardEnable(VkCommandBuffer commandBuffer, VkBool32 rasterizerDiscardEnable,
                                                         const RecordObject& record_obj) {}
virtual bool PreCallValidateCmdSetDepthBiasEnable(VkCommandBuffer commandBuffer, VkBool32 depthBiasEnable,
                                                  ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordCmdSetDepthBiasEnable(VkCommandBuffer commandBuffer, VkBool32 depthBiasEnable,
                                                const RecordObject& record_obj) {}
virtual void PostCallRecordCmdSetDepthBiasEnable(VkCommandBuffer commandBuffer, VkBool32 depthBiasEnable,
                                                 const RecordObject& record_obj) {}
virtual bool PreCallValidateCmdSetPrimitiveRestartEnable(VkCommandBuffer commandBuffer, VkBool32 primitiveRestartEnable,
                                                         ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordCmdSetPrimitiveRestartEnable(VkCommandBuffer commandBuffer, VkBool32 primitiveRestartEnable,
                                                       const RecordObject& record_obj) {}
virtual void PostCallRecordCmdSetPrimitiveRestartEnable(VkCommandBuffer commandBuffer, VkBool32 primitiveRestartEnable,
                                                        const RecordObject& record_obj) {}
virtual bool PreCallValidateGetDeviceBufferMemoryRequirements(VkDevice device, const VkDeviceBufferMemoryRequirements* pInfo,
                                                              VkMemoryRequirements2* pMemoryRequirements,
                                                              ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordGetDeviceBufferMemoryRequirements(VkDevice device, const VkDeviceBufferMemoryRequirements* pInfo,
                                                            VkMemoryRequirements2* pMemoryRequirements,
                                                            const RecordObject& record_obj) {}
virtual void PostCallRecordGetDeviceBufferMemoryRequirements(VkDevice device, const VkDeviceBufferMemoryRequirements* pInfo,
                                                             VkMemoryRequirements2* pMemoryRequirements,
                                                             const RecordObject& record_obj) {}
virtual bool PreCallValidateGetDeviceImageMemoryRequirements(VkDevice device, const VkDeviceImageMemoryRequirements* pInfo,
                                                             VkMemoryRequirements2* pMemoryRequirements,
                                                             ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordGetDeviceImageMemoryRequirements(VkDevice device, const VkDeviceImageMemoryRequirements* pInfo,
                                                           VkMemoryRequirements2* pMemoryRequirements,
                                                           const RecordObject& record_obj) {}
virtual void PostCallRecordGetDeviceImageMemoryRequirements(VkDevice device, const VkDeviceImageMemoryRequirements* pInfo,
                                                            VkMemoryRequirements2* pMemoryRequirements,
                                                            const RecordObject& record_obj) {}
virtual bool PreCallValidateGetDeviceImageSparseMemoryRequirements(VkDevice device, const VkDeviceImageMemoryRequirements* pInfo,
                                                                   uint32_t* pSparseMemoryRequirementCount,
                                                                   VkSparseImageMemoryRequirements2* pSparseMemoryRequirements,
                                                                   ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordGetDeviceImageSparseMemoryRequirements(VkDevice device, const VkDeviceImageMemoryRequirements* pInfo,
                                                                 uint32_t* pSparseMemoryRequirementCount,
                                                                 VkSparseImageMemoryRequirements2* pSparseMemoryRequirements,
                                                                 const RecordObject& record_obj) {}
virtual void PostCallRecordGetDeviceImageSparseMemoryRequirements(VkDevice device, const VkDeviceImageMemoryRequirements* pInfo,
                                                                  uint32_t* pSparseMemoryRequirementCount,
                                                                  VkSparseImageMemoryRequirements2* pSparseMemoryRequirements,
                                                                  const RecordObject& record_obj) {}
virtual bool PreCallValidateCmdSetLineStipple(VkCommandBuffer commandBuffer, uint32_t lineStippleFactor,
                                              uint16_t lineStipplePattern, ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordCmdSetLineStipple(VkCommandBuffer commandBuffer, uint32_t lineStippleFactor, uint16_t lineStipplePattern,
                                            const RecordObject& record_obj) {}
virtual void PostCallRecordCmdSetLineStipple(VkCommandBuffer commandBuffer, uint32_t lineStippleFactor, uint16_t lineStipplePattern,
                                             const RecordObject& record_obj) {}
virtual bool PreCallValidateMapMemory2(VkDevice device, const VkMemoryMapInfo* pMemoryMapInfo, void** ppData,
                                       ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordMapMemory2(VkDevice device, const VkMemoryMapInfo* pMemoryMapInfo, void** ppData,
                                     const RecordObject& record_obj) {}
virtual void PostCallRecordMapMemory2(VkDevice device, const VkMemoryMapInfo* pMemoryMapInfo, void** ppData,
                                      const RecordObject& record_obj) {}
virtual bool PreCallValidateUnmapMemory2(VkDevice device, const VkMemoryUnmapInfo* pMemoryUnmapInfo, ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordUnmapMemory2(VkDevice device, const VkMemoryUnmapInfo* pMemoryUnmapInfo, const RecordObject& record_obj) {
}
virtual void PostCallRecordUnmapMemory2(VkDevice device, const VkMemoryUnmapInfo* pMemoryUnmapInfo,
                                        const RecordObject& record_obj) {}
virtual bool PreCallValidateCmdBindIndexBuffer2(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                VkDeviceSize size, VkIndexType indexType, ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordCmdBindIndexBuffer2(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                              VkDeviceSize size, VkIndexType indexType, const RecordObject& record_obj) {}
virtual void PostCallRecordCmdBindIndexBuffer2(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                               VkDeviceSize size, VkIndexType indexType, const RecordObject& record_obj) {}
virtual bool PreCallValidateGetRenderingAreaGranularity(VkDevice device, const VkRenderingAreaInfo* pRenderingAreaInfo,
                                                        VkExtent2D* pGranularity, ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordGetRenderingAreaGranularity(VkDevice device, const VkRenderingAreaInfo* pRenderingAreaInfo,
                                                      VkExtent2D* pGranularity, const RecordObject& record_obj) {}
virtual void PostCallRecordGetRenderingAreaGranularity(VkDevice device, const VkRenderingAreaInfo* pRenderingAreaInfo,
                                                       VkExtent2D* pGranularity, const RecordObject& record_obj) {}
virtual bool PreCallValidateGetDeviceImageSubresourceLayout(VkDevice device, const VkDeviceImageSubresourceInfo* pInfo,
                                                            VkSubresourceLayout2* pLayout, ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordGetDeviceImageSubresourceLayout(VkDevice device, const VkDeviceImageSubresourceInfo* pInfo,
                                                          VkSubresourceLayout2* pLayout, const RecordObject& record_obj) {}
virtual void PostCallRecordGetDeviceImageSubresourceLayout(VkDevice device, const VkDeviceImageSubresourceInfo* pInfo,
                                                           VkSubresourceLayout2* pLayout, const RecordObject& record_obj) {}
virtual bool PreCallValidateGetImageSubresourceLayout2(VkDevice device, VkImage image, const VkImageSubresource2* pSubresource,
                                                       VkSubresourceLayout2* pLayout, ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordGetImageSubresourceLayout2(VkDevice device, VkImage image, const VkImageSubresource2* pSubresource,
                                                     VkSubresourceLayout2* pLayout, const RecordObject& record_obj) {}
virtual void PostCallRecordGetImageSubresourceLayout2(VkDevice device, VkImage image, const VkImageSubresource2* pSubresource,
                                                      VkSubresourceLayout2* pLayout, const RecordObject& record_obj) {}
virtual bool PreCallValidateCmdPushDescriptorSet(VkCommandBuffer commandBuffer, VkPipelineBindPoint pipelineBindPoint,
                                                 VkPipelineLayout layout, uint32_t set, uint32_t descriptorWriteCount,
                                                 const VkWriteDescriptorSet* pDescriptorWrites, ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordCmdPushDescriptorSet(VkCommandBuffer commandBuffer, VkPipelineBindPoint pipelineBindPoint,
                                               VkPipelineLayout layout, uint32_t set, uint32_t descriptorWriteCount,
                                               const VkWriteDescriptorSet* pDescriptorWrites, const RecordObject& record_obj) {}
virtual void PostCallRecordCmdPushDescriptorSet(VkCommandBuffer commandBuffer, VkPipelineBindPoint pipelineBindPoint,
                                                VkPipelineLayout layout, uint32_t set, uint32_t descriptorWriteCount,
                                                const VkWriteDescriptorSet* pDescriptorWrites, const RecordObject& record_obj) {}
virtual bool PreCallValidateCmdPushDescriptorSetWithTemplate(VkCommandBuffer commandBuffer,
                                                             VkDescriptorUpdateTemplate descriptorUpdateTemplate,
                                                             VkPipelineLayout layout, uint32_t set, const void* pData,
                                                             ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordCmdPushDescriptorSetWithTemplate(VkCommandBuffer commandBuffer,
                                                           VkDescriptorUpdateTemplate descriptorUpdateTemplate,
                                                           VkPipelineLayout layout, uint32_t set, const void* pData,
                                                           const RecordObject& record_obj) {}
virtual void PostCallRecordCmdPushDescriptorSetWithTemplate(VkCommandBuffer commandBuffer,
                                                            VkDescriptorUpdateTemplate descriptorUpdateTemplate,
                                                            VkPipelineLayout layout, uint32_t set, const void* pData,
                                                            const RecordObject& record_obj) {}
virtual bool PreCallValidateCmdSetRenderingAttachmentLocations(VkCommandBuffer commandBuffer,
                                                               const VkRenderingAttachmentLocationInfo* pLocationInfo,
                                                               ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordCmdSetRenderingAttachmentLocations(VkCommandBuffer commandBuffer,
                                                             const VkRenderingAttachmentLocationInfo* pLocationInfo,
                                                             const RecordObject& record_obj) {}
virtual void PostCallRecordCmdSetRenderingAttachmentLocations(VkCommandBuffer commandBuffer,
                                                              const VkRenderingAttachmentLocationInfo* pLocationInfo,
                                                              const RecordObject& record_obj) {}
virtual bool PreCallValidateCmdSetRenderingInputAttachmentIndices(
    VkCommandBuffer commandBuffer, const VkRenderingInputAttachmentIndexInfo* pInputAttachmentIndexInfo,
    ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordCmdSetRenderingInputAttachmentIndices(
    VkCommandBuffer commandBuffer, const VkRenderingInputAttachmentIndexInfo* pInputAttachmentIndexInfo,
    const RecordObject& record_obj) {}
virtual void PostCallRecordCmdSetRenderingInputAttachmentIndices(
    VkCommandBuffer commandBuffer, const VkRenderingInputAttachmentIndexInfo* pInputAttachmentIndexInfo,
    const RecordObject& record_obj) {}
virtual bool PreCallValidateCmdBindDescriptorSets2(VkCommandBuffer commandBuffer,
                                                   const VkBindDescriptorSetsInfo* pBindDescriptorSetsInfo,
                                                   ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordCmdBindDescriptorSets2(VkCommandBuffer commandBuffer,
                                                 const VkBindDescriptorSetsInfo* pBindDescriptorSetsInfo,
                                                 const RecordObject& record_obj) {}
virtual void PostCallRecordCmdBindDescriptorSets2(VkCommandBuffer commandBuffer,
                                                  const VkBindDescriptorSetsInfo* pBindDescriptorSetsInfo,
                                                  const RecordObject& record_obj) {}
virtual bool PreCallValidateCmdPushConstants2(VkCommandBuffer commandBuffer, const VkPushConstantsInfo* pPushConstantsInfo,
                                              ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordCmdPushConstants2(VkCommandBuffer commandBuffer, const VkPushConstantsInfo* pPushConstantsInfo,
                                            const RecordObject& record_obj) {}
virtual void PostCallRecordCmdPushConstants2(VkCommandBuffer commandBuffer, const VkPushConstantsInfo* pPushConstantsInfo,
                                             const RecordObject& record_obj) {}
virtual bool PreCallValidateCmdPushDescriptorSet2(VkCommandBuffer commandBuffer,
                                                  const VkPushDescriptorSetInfo* pPushDescriptorSetInfo,
                                                  ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordCmdPushDescriptorSet2(VkCommandBuffer commandBuffer,
                                                const VkPushDescriptorSetInfo* pPushDescriptorSetInfo,
                                                const RecordObject& record_obj) {}
virtual void PostCallRecordCmdPushDescriptorSet2(VkCommandBuffer commandBuffer,
                                                 const VkPushDescriptorSetInfo* pPushDescriptorSetInfo,
                                                 const RecordObject& record_obj) {}
virtual bool PreCallValidateCmdPushDescriptorSetWithTemplate2(
    VkCommandBuffer commandBuffer, const VkPushDescriptorSetWithTemplateInfo* pPushDescriptorSetWithTemplateInfo,
    ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordCmdPushDescriptorSetWithTemplate2(
    VkCommandBuffer commandBuffer, const VkPushDescriptorSetWithTemplateInfo* pPushDescriptorSetWithTemplateInfo,
    const RecordObject& record_obj) {}
virtual void PostCallRecordCmdPushDescriptorSetWithTemplate2(
    VkCommandBuffer commandBuffer, const VkPushDescriptorSetWithTemplateInfo* pPushDescriptorSetWithTemplateInfo,
    const RecordObject& record_obj) {}
virtual bool PreCallValidateCopyMemoryToImage(VkDevice device, const VkCopyMemoryToImageInfo* pCopyMemoryToImageInfo,
                                              ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordCopyMemoryToImage(VkDevice device, const VkCopyMemoryToImageInfo* pCopyMemoryToImageInfo,
                                            const RecordObject& record_obj) {}
virtual void PostCallRecordCopyMemoryToImage(VkDevice device, const VkCopyMemoryToImageInfo* pCopyMemoryToImageInfo,
                                             const RecordObject& record_obj) {}
virtual bool PreCallValidateCopyImageToMemory(VkDevice device, const VkCopyImageToMemoryInfo* pCopyImageToMemoryInfo,
                                              ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordCopyImageToMemory(VkDevice device, const VkCopyImageToMemoryInfo* pCopyImageToMemoryInfo,
                                            const RecordObject& record_obj) {}
virtual void PostCallRecordCopyImageToMemory(VkDevice device, const VkCopyImageToMemoryInfo* pCopyImageToMemoryInfo,
                                             const RecordObject& record_obj) {}
virtual bool PreCallValidateCopyImageToImage(VkDevice device, const VkCopyImageToImageInfo* pCopyImageToImageInfo,
                                             ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordCopyImageToImage(VkDevice device, const VkCopyImageToImageInfo* pCopyImageToImageInfo,
                                           const RecordObject& record_obj) {}
virtual void PostCallRecordCopyImageToImage(VkDevice device, const VkCopyImageToImageInfo* pCopyImageToImageInfo,
                                            const RecordObject& record_obj) {}
virtual bool PreCallValidateTransitionImageLayout(VkDevice device, uint32_t transitionCount,
                                                  const VkHostImageLayoutTransitionInfo* pTransitions,
                                                  ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordTransitionImageLayout(VkDevice device, uint32_t transitionCount,
                                                const VkHostImageLayoutTransitionInfo* pTransitions,
                                                const RecordObject& record_obj) {}
virtual void PostCallRecordTransitionImageLayout(VkDevice device, uint32_t transitionCount,
                                                 const VkHostImageLayoutTransitionInfo* pTransitions,
                                                 const RecordObject& record_obj) {}
virtual bool PreCallValidateCreateSwapchainKHR(VkDevice device, const VkSwapchainCreateInfoKHR* pCreateInfo,
                                               const VkAllocationCallbacks* pAllocator, VkSwapchainKHR* pSwapchain,
                                               ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordCreateSwapchainKHR(VkDevice device, const VkSwapchainCreateInfoKHR* pCreateInfo,
                                             const VkAllocationCallbacks* pAllocator, VkSwapchainKHR* pSwapchain,
                                             const RecordObject& record_obj) {}
virtual void PostCallRecordCreateSwapchainKHR(VkDevice device, const VkSwapchainCreateInfoKHR* pCreateInfo,
                                              const VkAllocationCallbacks* pAllocator, VkSwapchainKHR* pSwapchain,
                                              const RecordObject& record_obj) {}
virtual bool PreCallValidateDestroySwapchainKHR(VkDevice device, VkSwapchainKHR swapchain, const VkAllocationCallbacks* pAllocator,
                                                ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordDestroySwapchainKHR(VkDevice device, VkSwapchainKHR swapchain, const VkAllocationCallbacks* pAllocator,
                                              const RecordObject& record_obj) {}
virtual void PostCallRecordDestroySwapchainKHR(VkDevice device, VkSwapchainKHR swapchain, const VkAllocationCallbacks* pAllocator,
                                               const RecordObject& record_obj) {}
virtual bool PreCallValidateGetSwapchainImagesKHR(VkDevice device, VkSwapchainKHR swapchain, uint32_t* pSwapchainImageCount,
                                                  VkImage* pSwapchainImages, ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordGetSwapchainImagesKHR(VkDevice device, VkSwapchainKHR swapchain, uint32_t* pSwapchainImageCount,
                                                VkImage* pSwapchainImages, const RecordObject& record_obj) {}
virtual void PostCallRecordGetSwapchainImagesKHR(VkDevice device, VkSwapchainKHR swapchain, uint32_t* pSwapchainImageCount,
                                                 VkImage* pSwapchainImages, const RecordObject& record_obj) {}
virtual bool PreCallValidateAcquireNextImageKHR(VkDevice device, VkSwapchainKHR swapchain, uint64_t timeout, VkSemaphore semaphore,
                                                VkFence fence, uint32_t* pImageIndex, ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordAcquireNextImageKHR(VkDevice device, VkSwapchainKHR swapchain, uint64_t timeout, VkSemaphore semaphore,
                                              VkFence fence, uint32_t* pImageIndex, const RecordObject& record_obj) {}
virtual void PostCallRecordAcquireNextImageKHR(VkDevice device, VkSwapchainKHR swapchain, uint64_t timeout, VkSemaphore semaphore,
                                               VkFence fence, uint32_t* pImageIndex, const RecordObject& record_obj) {}
virtual bool PreCallValidateQueuePresentKHR(VkQueue queue, const VkPresentInfoKHR* pPresentInfo, ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordQueuePresentKHR(VkQueue queue, const VkPresentInfoKHR* pPresentInfo, const RecordObject& record_obj) {}
virtual void PostCallRecordQueuePresentKHR(VkQueue queue, const VkPresentInfoKHR* pPresentInfo, const RecordObject& record_obj) {}
virtual bool PreCallValidateGetDeviceGroupPresentCapabilitiesKHR(
    VkDevice device, VkDeviceGroupPresentCapabilitiesKHR* pDeviceGroupPresentCapabilities, ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordGetDeviceGroupPresentCapabilitiesKHR(VkDevice device,
                                                               VkDeviceGroupPresentCapabilitiesKHR* pDeviceGroupPresentCapabilities,
                                                               const RecordObject& record_obj) {}
virtual void PostCallRecordGetDeviceGroupPresentCapabilitiesKHR(
    VkDevice device, VkDeviceGroupPresentCapabilitiesKHR* pDeviceGroupPresentCapabilities, const RecordObject& record_obj) {}
virtual bool PreCallValidateGetDeviceGroupSurfacePresentModesKHR(VkDevice device, VkSurfaceKHR surface,
                                                                 VkDeviceGroupPresentModeFlagsKHR* pModes,
                                                                 ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordGetDeviceGroupSurfacePresentModesKHR(VkDevice device, VkSurfaceKHR surface,
                                                               VkDeviceGroupPresentModeFlagsKHR* pModes,
                                                               const RecordObject& record_obj) {}
virtual void PostCallRecordGetDeviceGroupSurfacePresentModesKHR(VkDevice device, VkSurfaceKHR surface,
                                                                VkDeviceGroupPresentModeFlagsKHR* pModes,
                                                                const RecordObject& record_obj) {}
virtual bool PreCallValidateAcquireNextImage2KHR(VkDevice device, const VkAcquireNextImageInfoKHR* pAcquireInfo,
                                                 uint32_t* pImageIndex, ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordAcquireNextImage2KHR(VkDevice device, const VkAcquireNextImageInfoKHR* pAcquireInfo,
                                               uint32_t* pImageIndex, const RecordObject& record_obj) {}
virtual void PostCallRecordAcquireNextImage2KHR(VkDevice device, const VkAcquireNextImageInfoKHR* pAcquireInfo,
                                                uint32_t* pImageIndex, const RecordObject& record_obj) {}
virtual bool PreCallValidateCreateSharedSwapchainsKHR(VkDevice device, uint32_t swapchainCount,
                                                      const VkSwapchainCreateInfoKHR* pCreateInfos,
                                                      const VkAllocationCallbacks* pAllocator, VkSwapchainKHR* pSwapchains,
                                                      ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordCreateSharedSwapchainsKHR(VkDevice device, uint32_t swapchainCount,
                                                    const VkSwapchainCreateInfoKHR* pCreateInfos,
                                                    const VkAllocationCallbacks* pAllocator, VkSwapchainKHR* pSwapchains,
                                                    const RecordObject& record_obj) {}
virtual void PostCallRecordCreateSharedSwapchainsKHR(VkDevice device, uint32_t swapchainCount,
                                                     const VkSwapchainCreateInfoKHR* pCreateInfos,
                                                     const VkAllocationCallbacks* pAllocator, VkSwapchainKHR* pSwapchains,
                                                     const RecordObject& record_obj) {}
virtual bool PreCallValidateCreateVideoSessionKHR(VkDevice device, const VkVideoSessionCreateInfoKHR* pCreateInfo,
                                                  const VkAllocationCallbacks* pAllocator, VkVideoSessionKHR* pVideoSession,
                                                  ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordCreateVideoSessionKHR(VkDevice device, const VkVideoSessionCreateInfoKHR* pCreateInfo,
                                                const VkAllocationCallbacks* pAllocator, VkVideoSessionKHR* pVideoSession,
                                                const RecordObject& record_obj) {}
virtual void PostCallRecordCreateVideoSessionKHR(VkDevice device, const VkVideoSessionCreateInfoKHR* pCreateInfo,
                                                 const VkAllocationCallbacks* pAllocator, VkVideoSessionKHR* pVideoSession,
                                                 const RecordObject& record_obj) {}
virtual bool PreCallValidateDestroyVideoSessionKHR(VkDevice device, VkVideoSessionKHR videoSession,
                                                   const VkAllocationCallbacks* pAllocator, ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordDestroyVideoSessionKHR(VkDevice device, VkVideoSessionKHR videoSession,
                                                 const VkAllocationCallbacks* pAllocator, const RecordObject& record_obj) {}
virtual void PostCallRecordDestroyVideoSessionKHR(VkDevice device, VkVideoSessionKHR videoSession,
                                                  const VkAllocationCallbacks* pAllocator, const RecordObject& record_obj) {}
virtual bool PreCallValidateGetVideoSessionMemoryRequirementsKHR(VkDevice device, VkVideoSessionKHR videoSession,
                                                                 uint32_t* pMemoryRequirementsCount,
                                                                 VkVideoSessionMemoryRequirementsKHR* pMemoryRequirements,
                                                                 ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordGetVideoSessionMemoryRequirementsKHR(VkDevice device, VkVideoSessionKHR videoSession,
                                                               uint32_t* pMemoryRequirementsCount,
                                                               VkVideoSessionMemoryRequirementsKHR* pMemoryRequirements,
                                                               const RecordObject& record_obj) {}
virtual void PostCallRecordGetVideoSessionMemoryRequirementsKHR(VkDevice device, VkVideoSessionKHR videoSession,
                                                                uint32_t* pMemoryRequirementsCount,
                                                                VkVideoSessionMemoryRequirementsKHR* pMemoryRequirements,
                                                                const RecordObject& record_obj) {}
virtual bool PreCallValidateBindVideoSessionMemoryKHR(VkDevice device, VkVideoSessionKHR videoSession,
                                                      uint32_t bindSessionMemoryInfoCount,
                                                      const VkBindVideoSessionMemoryInfoKHR* pBindSessionMemoryInfos,
                                                      ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordBindVideoSessionMemoryKHR(VkDevice device, VkVideoSessionKHR videoSession,
                                                    uint32_t bindSessionMemoryInfoCount,
                                                    const VkBindVideoSessionMemoryInfoKHR* pBindSessionMemoryInfos,
                                                    const RecordObject& record_obj) {}
virtual void PostCallRecordBindVideoSessionMemoryKHR(VkDevice device, VkVideoSessionKHR videoSession,
                                                     uint32_t bindSessionMemoryInfoCount,
                                                     const VkBindVideoSessionMemoryInfoKHR* pBindSessionMemoryInfos,
                                                     const RecordObject& record_obj) {}
virtual bool PreCallValidateCreateVideoSessionParametersKHR(VkDevice device,
                                                            const VkVideoSessionParametersCreateInfoKHR* pCreateInfo,
                                                            const VkAllocationCallbacks* pAllocator,
                                                            VkVideoSessionParametersKHR* pVideoSessionParameters,
                                                            ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordCreateVideoSessionParametersKHR(VkDevice device, const VkVideoSessionParametersCreateInfoKHR* pCreateInfo,
                                                          const VkAllocationCallbacks* pAllocator,
                                                          VkVideoSessionParametersKHR* pVideoSessionParameters,
                                                          const RecordObject& record_obj) {}
virtual void PostCallRecordCreateVideoSessionParametersKHR(VkDevice device,
                                                           const VkVideoSessionParametersCreateInfoKHR* pCreateInfo,
                                                           const VkAllocationCallbacks* pAllocator,
                                                           VkVideoSessionParametersKHR* pVideoSessionParameters,
                                                           const RecordObject& record_obj) {}
virtual bool PreCallValidateUpdateVideoSessionParametersKHR(VkDevice device, VkVideoSessionParametersKHR videoSessionParameters,
                                                            const VkVideoSessionParametersUpdateInfoKHR* pUpdateInfo,
                                                            ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordUpdateVideoSessionParametersKHR(VkDevice device, VkVideoSessionParametersKHR videoSessionParameters,
                                                          const VkVideoSessionParametersUpdateInfoKHR* pUpdateInfo,
                                                          const RecordObject& record_obj) {}
virtual void PostCallRecordUpdateVideoSessionParametersKHR(VkDevice device, VkVideoSessionParametersKHR videoSessionParameters,
                                                           const VkVideoSessionParametersUpdateInfoKHR* pUpdateInfo,
                                                           const RecordObject& record_obj) {}
virtual bool PreCallValidateDestroyVideoSessionParametersKHR(VkDevice device, VkVideoSessionParametersKHR videoSessionParameters,
                                                             const VkAllocationCallbacks* pAllocator,
                                                             ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordDestroyVideoSessionParametersKHR(VkDevice device, VkVideoSessionParametersKHR videoSessionParameters,
                                                           const VkAllocationCallbacks* pAllocator,
                                                           const RecordObject& record_obj) {}
virtual void PostCallRecordDestroyVideoSessionParametersKHR(VkDevice device, VkVideoSessionParametersKHR videoSessionParameters,
                                                            const VkAllocationCallbacks* pAllocator,
                                                            const RecordObject& record_obj) {}
virtual bool PreCallValidateCmdBeginVideoCodingKHR(VkCommandBuffer commandBuffer, const VkVideoBeginCodingInfoKHR* pBeginInfo,
                                                   ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordCmdBeginVideoCodingKHR(VkCommandBuffer commandBuffer, const VkVideoBeginCodingInfoKHR* pBeginInfo,
                                                 const RecordObject& record_obj) {}
virtual void PostCallRecordCmdBeginVideoCodingKHR(VkCommandBuffer commandBuffer, const VkVideoBeginCodingInfoKHR* pBeginInfo,
                                                  const RecordObject& record_obj) {}
virtual bool PreCallValidateCmdEndVideoCodingKHR(VkCommandBuffer commandBuffer, const VkVideoEndCodingInfoKHR* pEndCodingInfo,
                                                 ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordCmdEndVideoCodingKHR(VkCommandBuffer commandBuffer, const VkVideoEndCodingInfoKHR* pEndCodingInfo,
                                               const RecordObject& record_obj) {}
virtual void PostCallRecordCmdEndVideoCodingKHR(VkCommandBuffer commandBuffer, const VkVideoEndCodingInfoKHR* pEndCodingInfo,
                                                const RecordObject& record_obj) {}
virtual bool PreCallValidateCmdControlVideoCodingKHR(VkCommandBuffer commandBuffer,
                                                     const VkVideoCodingControlInfoKHR* pCodingControlInfo,
                                                     ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordCmdControlVideoCodingKHR(VkCommandBuffer commandBuffer,
                                                   const VkVideoCodingControlInfoKHR* pCodingControlInfo,
                                                   const RecordObject& record_obj) {}
virtual void PostCallRecordCmdControlVideoCodingKHR(VkCommandBuffer commandBuffer,
                                                    const VkVideoCodingControlInfoKHR* pCodingControlInfo,
                                                    const RecordObject& record_obj) {}
virtual bool PreCallValidateCmdDecodeVideoKHR(VkCommandBuffer commandBuffer, const VkVideoDecodeInfoKHR* pDecodeInfo,
                                              ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordCmdDecodeVideoKHR(VkCommandBuffer commandBuffer, const VkVideoDecodeInfoKHR* pDecodeInfo,
                                            const RecordObject& record_obj) {}
virtual void PostCallRecordCmdDecodeVideoKHR(VkCommandBuffer commandBuffer, const VkVideoDecodeInfoKHR* pDecodeInfo,
                                             const RecordObject& record_obj) {}
virtual bool PreCallValidateCmdBeginRenderingKHR(VkCommandBuffer commandBuffer, const VkRenderingInfo* pRenderingInfo,
                                                 ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordCmdBeginRenderingKHR(VkCommandBuffer commandBuffer, const VkRenderingInfo* pRenderingInfo,
                                               const RecordObject& record_obj) {}
virtual void PostCallRecordCmdBeginRenderingKHR(VkCommandBuffer commandBuffer, const VkRenderingInfo* pRenderingInfo,
                                                const RecordObject& record_obj) {}
virtual bool PreCallValidateCmdEndRenderingKHR(VkCommandBuffer commandBuffer, ErrorObject& error_obj) const { return false; }
virtual void PreCallRecordCmdEndRenderingKHR(VkCommandBuffer commandBuffer, const RecordObject& record_obj) {}
virtual void PostCallRecordCmdEndRenderingKHR(VkCommandBuffer commandBuffer, const RecordObject& record_obj) {}
virtual bool PreCallValidateGetDeviceGroupPeerMemoryFeaturesKHR(VkDevice device, uint32_t heapIndex, uint32_t localDeviceIndex,
                                                                uint32_t remoteDeviceIndex,
                                                                VkPeerMemoryFeatureFlags* pPeerMemoryFeatures,
                                                                ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordGetDeviceGroupPeerMemoryFeaturesKHR(VkDevice device, uint32_t heapIndex, uint32_t localDeviceIndex,
                                                              uint32_t remoteDeviceIndex,
                                                              VkPeerMemoryFeatureFlags* pPeerMemoryFeatures,
                                                              const RecordObject& record_obj) {}
virtual void PostCallRecordGetDeviceGroupPeerMemoryFeaturesKHR(VkDevice device, uint32_t heapIndex, uint32_t localDeviceIndex,
                                                               uint32_t remoteDeviceIndex,
                                                               VkPeerMemoryFeatureFlags* pPeerMemoryFeatures,
                                                               const RecordObject& record_obj) {}
virtual bool PreCallValidateCmdSetDeviceMaskKHR(VkCommandBuffer commandBuffer, uint32_t deviceMask, ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordCmdSetDeviceMaskKHR(VkCommandBuffer commandBuffer, uint32_t deviceMask, const RecordObject& record_obj) {}
virtual void PostCallRecordCmdSetDeviceMaskKHR(VkCommandBuffer commandBuffer, uint32_t deviceMask, const RecordObject& record_obj) {
}
virtual bool PreCallValidateCmdDispatchBaseKHR(VkCommandBuffer commandBuffer, uint32_t baseGroupX, uint32_t baseGroupY,
                                               uint32_t baseGroupZ, uint32_t groupCountX, uint32_t groupCountY,
                                               uint32_t groupCountZ, ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordCmdDispatchBaseKHR(VkCommandBuffer commandBuffer, uint32_t baseGroupX, uint32_t baseGroupY,
                                             uint32_t baseGroupZ, uint32_t groupCountX, uint32_t groupCountY, uint32_t groupCountZ,
                                             const RecordObject& record_obj) {}
virtual void PostCallRecordCmdDispatchBaseKHR(VkCommandBuffer commandBuffer, uint32_t baseGroupX, uint32_t baseGroupY,
                                              uint32_t baseGroupZ, uint32_t groupCountX, uint32_t groupCountY, uint32_t groupCountZ,
                                              const RecordObject& record_obj) {}
virtual bool PreCallValidateTrimCommandPoolKHR(VkDevice device, VkCommandPool commandPool, VkCommandPoolTrimFlags flags,
                                               ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordTrimCommandPoolKHR(VkDevice device, VkCommandPool commandPool, VkCommandPoolTrimFlags flags,
                                             const RecordObject& record_obj) {}
virtual void PostCallRecordTrimCommandPoolKHR(VkDevice device, VkCommandPool commandPool, VkCommandPoolTrimFlags flags,
                                              const RecordObject& record_obj) {}
#ifdef VK_USE_PLATFORM_WIN32_KHR
virtual bool PreCallValidateGetMemoryWin32HandleKHR(VkDevice device, const VkMemoryGetWin32HandleInfoKHR* pGetWin32HandleInfo,
                                                    HANDLE* pHandle, ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordGetMemoryWin32HandleKHR(VkDevice device, const VkMemoryGetWin32HandleInfoKHR* pGetWin32HandleInfo,
                                                  HANDLE* pHandle, const RecordObject& record_obj) {}
virtual void PostCallRecordGetMemoryWin32HandleKHR(VkDevice device, const VkMemoryGetWin32HandleInfoKHR* pGetWin32HandleInfo,
                                                   HANDLE* pHandle, const RecordObject& record_obj) {}
virtual bool PreCallValidateGetMemoryWin32HandlePropertiesKHR(VkDevice device, VkExternalMemoryHandleTypeFlagBits handleType,
                                                              HANDLE handle,
                                                              VkMemoryWin32HandlePropertiesKHR* pMemoryWin32HandleProperties,
                                                              ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordGetMemoryWin32HandlePropertiesKHR(VkDevice device, VkExternalMemoryHandleTypeFlagBits handleType,
                                                            HANDLE handle,
                                                            VkMemoryWin32HandlePropertiesKHR* pMemoryWin32HandleProperties,
                                                            const RecordObject& record_obj) {}
virtual void PostCallRecordGetMemoryWin32HandlePropertiesKHR(VkDevice device, VkExternalMemoryHandleTypeFlagBits handleType,
                                                             HANDLE handle,
                                                             VkMemoryWin32HandlePropertiesKHR* pMemoryWin32HandleProperties,
                                                             const RecordObject& record_obj) {}
#endif  // VK_USE_PLATFORM_WIN32_KHR
virtual bool PreCallValidateGetMemoryFdKHR(VkDevice device, const VkMemoryGetFdInfoKHR* pGetFdInfo, int* pFd,
                                           ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordGetMemoryFdKHR(VkDevice device, const VkMemoryGetFdInfoKHR* pGetFdInfo, int* pFd,
                                         const RecordObject& record_obj) {}
virtual void PostCallRecordGetMemoryFdKHR(VkDevice device, const VkMemoryGetFdInfoKHR* pGetFdInfo, int* pFd,
                                          const RecordObject& record_obj) {}
virtual bool PreCallValidateGetMemoryFdPropertiesKHR(VkDevice device, VkExternalMemoryHandleTypeFlagBits handleType, int fd,
                                                     VkMemoryFdPropertiesKHR* pMemoryFdProperties, ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordGetMemoryFdPropertiesKHR(VkDevice device, VkExternalMemoryHandleTypeFlagBits handleType, int fd,
                                                   VkMemoryFdPropertiesKHR* pMemoryFdProperties, const RecordObject& record_obj) {}
virtual void PostCallRecordGetMemoryFdPropertiesKHR(VkDevice device, VkExternalMemoryHandleTypeFlagBits handleType, int fd,
                                                    VkMemoryFdPropertiesKHR* pMemoryFdProperties, const RecordObject& record_obj) {}
#ifdef VK_USE_PLATFORM_WIN32_KHR
virtual bool PreCallValidateImportSemaphoreWin32HandleKHR(
    VkDevice device, const VkImportSemaphoreWin32HandleInfoKHR* pImportSemaphoreWin32HandleInfo, ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordImportSemaphoreWin32HandleKHR(VkDevice device,
                                                        const VkImportSemaphoreWin32HandleInfoKHR* pImportSemaphoreWin32HandleInfo,
                                                        const RecordObject& record_obj) {}
virtual void PostCallRecordImportSemaphoreWin32HandleKHR(VkDevice device,
                                                         const VkImportSemaphoreWin32HandleInfoKHR* pImportSemaphoreWin32HandleInfo,
                                                         const RecordObject& record_obj) {}
virtual bool PreCallValidateGetSemaphoreWin32HandleKHR(VkDevice device, const VkSemaphoreGetWin32HandleInfoKHR* pGetWin32HandleInfo,
                                                       HANDLE* pHandle, ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordGetSemaphoreWin32HandleKHR(VkDevice device, const VkSemaphoreGetWin32HandleInfoKHR* pGetWin32HandleInfo,
                                                     HANDLE* pHandle, const RecordObject& record_obj) {}
virtual void PostCallRecordGetSemaphoreWin32HandleKHR(VkDevice device, const VkSemaphoreGetWin32HandleInfoKHR* pGetWin32HandleInfo,
                                                      HANDLE* pHandle, const RecordObject& record_obj) {}
#endif  // VK_USE_PLATFORM_WIN32_KHR
virtual bool PreCallValidateImportSemaphoreFdKHR(VkDevice device, const VkImportSemaphoreFdInfoKHR* pImportSemaphoreFdInfo,
                                                 ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordImportSemaphoreFdKHR(VkDevice device, const VkImportSemaphoreFdInfoKHR* pImportSemaphoreFdInfo,
                                               const RecordObject& record_obj) {}
virtual void PostCallRecordImportSemaphoreFdKHR(VkDevice device, const VkImportSemaphoreFdInfoKHR* pImportSemaphoreFdInfo,
                                                const RecordObject& record_obj) {}
virtual bool PreCallValidateGetSemaphoreFdKHR(VkDevice device, const VkSemaphoreGetFdInfoKHR* pGetFdInfo, int* pFd,
                                              ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordGetSemaphoreFdKHR(VkDevice device, const VkSemaphoreGetFdInfoKHR* pGetFdInfo, int* pFd,
                                            const RecordObject& record_obj) {}
virtual void PostCallRecordGetSemaphoreFdKHR(VkDevice device, const VkSemaphoreGetFdInfoKHR* pGetFdInfo, int* pFd,
                                             const RecordObject& record_obj) {}
virtual bool PreCallValidateCmdPushDescriptorSetKHR(VkCommandBuffer commandBuffer, VkPipelineBindPoint pipelineBindPoint,
                                                    VkPipelineLayout layout, uint32_t set, uint32_t descriptorWriteCount,
                                                    const VkWriteDescriptorSet* pDescriptorWrites, ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordCmdPushDescriptorSetKHR(VkCommandBuffer commandBuffer, VkPipelineBindPoint pipelineBindPoint,
                                                  VkPipelineLayout layout, uint32_t set, uint32_t descriptorWriteCount,
                                                  const VkWriteDescriptorSet* pDescriptorWrites, const RecordObject& record_obj) {}
virtual void PostCallRecordCmdPushDescriptorSetKHR(VkCommandBuffer commandBuffer, VkPipelineBindPoint pipelineBindPoint,
                                                   VkPipelineLayout layout, uint32_t set, uint32_t descriptorWriteCount,
                                                   const VkWriteDescriptorSet* pDescriptorWrites, const RecordObject& record_obj) {}
virtual bool PreCallValidateCmdPushDescriptorSetWithTemplateKHR(VkCommandBuffer commandBuffer,
                                                                VkDescriptorUpdateTemplate descriptorUpdateTemplate,
                                                                VkPipelineLayout layout, uint32_t set, const void* pData,
                                                                ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordCmdPushDescriptorSetWithTemplateKHR(VkCommandBuffer commandBuffer,
                                                              VkDescriptorUpdateTemplate descriptorUpdateTemplate,
                                                              VkPipelineLayout layout, uint32_t set, const void* pData,
                                                              const RecordObject& record_obj) {}
virtual void PostCallRecordCmdPushDescriptorSetWithTemplateKHR(VkCommandBuffer commandBuffer,
                                                               VkDescriptorUpdateTemplate descriptorUpdateTemplate,
                                                               VkPipelineLayout layout, uint32_t set, const void* pData,
                                                               const RecordObject& record_obj) {}
virtual bool PreCallValidateCreateDescriptorUpdateTemplateKHR(VkDevice device,
                                                              const VkDescriptorUpdateTemplateCreateInfo* pCreateInfo,
                                                              const VkAllocationCallbacks* pAllocator,
                                                              VkDescriptorUpdateTemplate* pDescriptorUpdateTemplate,
                                                              ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordCreateDescriptorUpdateTemplateKHR(VkDevice device,
                                                            const VkDescriptorUpdateTemplateCreateInfo* pCreateInfo,
                                                            const VkAllocationCallbacks* pAllocator,
                                                            VkDescriptorUpdateTemplate* pDescriptorUpdateTemplate,
                                                            const RecordObject& record_obj) {}
virtual void PostCallRecordCreateDescriptorUpdateTemplateKHR(VkDevice device,
                                                             const VkDescriptorUpdateTemplateCreateInfo* pCreateInfo,
                                                             const VkAllocationCallbacks* pAllocator,
                                                             VkDescriptorUpdateTemplate* pDescriptorUpdateTemplate,
                                                             const RecordObject& record_obj) {}
virtual bool PreCallValidateDestroyDescriptorUpdateTemplateKHR(VkDevice device, VkDescriptorUpdateTemplate descriptorUpdateTemplate,
                                                               const VkAllocationCallbacks* pAllocator,
                                                               ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordDestroyDescriptorUpdateTemplateKHR(VkDevice device, VkDescriptorUpdateTemplate descriptorUpdateTemplate,
                                                             const VkAllocationCallbacks* pAllocator,
                                                             const RecordObject& record_obj) {}
virtual void PostCallRecordDestroyDescriptorUpdateTemplateKHR(VkDevice device, VkDescriptorUpdateTemplate descriptorUpdateTemplate,
                                                              const VkAllocationCallbacks* pAllocator,
                                                              const RecordObject& record_obj) {}
virtual bool PreCallValidateUpdateDescriptorSetWithTemplateKHR(VkDevice device, VkDescriptorSet descriptorSet,
                                                               VkDescriptorUpdateTemplate descriptorUpdateTemplate,
                                                               const void* pData, ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordUpdateDescriptorSetWithTemplateKHR(VkDevice device, VkDescriptorSet descriptorSet,
                                                             VkDescriptorUpdateTemplate descriptorUpdateTemplate, const void* pData,
                                                             const RecordObject& record_obj) {}
virtual void PostCallRecordUpdateDescriptorSetWithTemplateKHR(VkDevice device, VkDescriptorSet descriptorSet,
                                                              VkDescriptorUpdateTemplate descriptorUpdateTemplate,
                                                              const void* pData, const RecordObject& record_obj) {}
virtual bool PreCallValidateCreateRenderPass2KHR(VkDevice device, const VkRenderPassCreateInfo2* pCreateInfo,
                                                 const VkAllocationCallbacks* pAllocator, VkRenderPass* pRenderPass,
                                                 ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordCreateRenderPass2KHR(VkDevice device, const VkRenderPassCreateInfo2* pCreateInfo,
                                               const VkAllocationCallbacks* pAllocator, VkRenderPass* pRenderPass,
                                               const RecordObject& record_obj) {}
virtual void PostCallRecordCreateRenderPass2KHR(VkDevice device, const VkRenderPassCreateInfo2* pCreateInfo,
                                                const VkAllocationCallbacks* pAllocator, VkRenderPass* pRenderPass,
                                                const RecordObject& record_obj) {}
virtual bool PreCallValidateCmdBeginRenderPass2KHR(VkCommandBuffer commandBuffer, const VkRenderPassBeginInfo* pRenderPassBegin,
                                                   const VkSubpassBeginInfo* pSubpassBeginInfo, ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordCmdBeginRenderPass2KHR(VkCommandBuffer commandBuffer, const VkRenderPassBeginInfo* pRenderPassBegin,
                                                 const VkSubpassBeginInfo* pSubpassBeginInfo, const RecordObject& record_obj) {}
virtual void PostCallRecordCmdBeginRenderPass2KHR(VkCommandBuffer commandBuffer, const VkRenderPassBeginInfo* pRenderPassBegin,
                                                  const VkSubpassBeginInfo* pSubpassBeginInfo, const RecordObject& record_obj) {}
virtual bool PreCallValidateCmdNextSubpass2KHR(VkCommandBuffer commandBuffer, const VkSubpassBeginInfo* pSubpassBeginInfo,
                                               const VkSubpassEndInfo* pSubpassEndInfo, ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordCmdNextSubpass2KHR(VkCommandBuffer commandBuffer, const VkSubpassBeginInfo* pSubpassBeginInfo,
                                             const VkSubpassEndInfo* pSubpassEndInfo, const RecordObject& record_obj) {}
virtual void PostCallRecordCmdNextSubpass2KHR(VkCommandBuffer commandBuffer, const VkSubpassBeginInfo* pSubpassBeginInfo,
                                              const VkSubpassEndInfo* pSubpassEndInfo, const RecordObject& record_obj) {}
virtual bool PreCallValidateCmdEndRenderPass2KHR(VkCommandBuffer commandBuffer, const VkSubpassEndInfo* pSubpassEndInfo,
                                                 ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordCmdEndRenderPass2KHR(VkCommandBuffer commandBuffer, const VkSubpassEndInfo* pSubpassEndInfo,
                                               const RecordObject& record_obj) {}
virtual void PostCallRecordCmdEndRenderPass2KHR(VkCommandBuffer commandBuffer, const VkSubpassEndInfo* pSubpassEndInfo,
                                                const RecordObject& record_obj) {}
virtual bool PreCallValidateGetSwapchainStatusKHR(VkDevice device, VkSwapchainKHR swapchain, ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordGetSwapchainStatusKHR(VkDevice device, VkSwapchainKHR swapchain, const RecordObject& record_obj) {}
virtual void PostCallRecordGetSwapchainStatusKHR(VkDevice device, VkSwapchainKHR swapchain, const RecordObject& record_obj) {}
#ifdef VK_USE_PLATFORM_WIN32_KHR
virtual bool PreCallValidateImportFenceWin32HandleKHR(VkDevice device,
                                                      const VkImportFenceWin32HandleInfoKHR* pImportFenceWin32HandleInfo,
                                                      ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordImportFenceWin32HandleKHR(VkDevice device,
                                                    const VkImportFenceWin32HandleInfoKHR* pImportFenceWin32HandleInfo,
                                                    const RecordObject& record_obj) {}
virtual void PostCallRecordImportFenceWin32HandleKHR(VkDevice device,
                                                     const VkImportFenceWin32HandleInfoKHR* pImportFenceWin32HandleInfo,
                                                     const RecordObject& record_obj) {}
virtual bool PreCallValidateGetFenceWin32HandleKHR(VkDevice device, const VkFenceGetWin32HandleInfoKHR* pGetWin32HandleInfo,
                                                   HANDLE* pHandle, ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordGetFenceWin32HandleKHR(VkDevice device, const VkFenceGetWin32HandleInfoKHR* pGetWin32HandleInfo,
                                                 HANDLE* pHandle, const RecordObject& record_obj) {}
virtual void PostCallRecordGetFenceWin32HandleKHR(VkDevice device, const VkFenceGetWin32HandleInfoKHR* pGetWin32HandleInfo,
                                                  HANDLE* pHandle, const RecordObject& record_obj) {}
#endif  // VK_USE_PLATFORM_WIN32_KHR
virtual bool PreCallValidateImportFenceFdKHR(VkDevice device, const VkImportFenceFdInfoKHR* pImportFenceFdInfo,
                                             ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordImportFenceFdKHR(VkDevice device, const VkImportFenceFdInfoKHR* pImportFenceFdInfo,
                                           const RecordObject& record_obj) {}
virtual void PostCallRecordImportFenceFdKHR(VkDevice device, const VkImportFenceFdInfoKHR* pImportFenceFdInfo,
                                            const RecordObject& record_obj) {}
virtual bool PreCallValidateGetFenceFdKHR(VkDevice device, const VkFenceGetFdInfoKHR* pGetFdInfo, int* pFd,
                                          ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordGetFenceFdKHR(VkDevice device, const VkFenceGetFdInfoKHR* pGetFdInfo, int* pFd,
                                        const RecordObject& record_obj) {}
virtual void PostCallRecordGetFenceFdKHR(VkDevice device, const VkFenceGetFdInfoKHR* pGetFdInfo, int* pFd,
                                         const RecordObject& record_obj) {}
virtual bool PreCallValidateAcquireProfilingLockKHR(VkDevice device, const VkAcquireProfilingLockInfoKHR* pInfo,
                                                    ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordAcquireProfilingLockKHR(VkDevice device, const VkAcquireProfilingLockInfoKHR* pInfo,
                                                  const RecordObject& record_obj) {}
virtual void PostCallRecordAcquireProfilingLockKHR(VkDevice device, const VkAcquireProfilingLockInfoKHR* pInfo,
                                                   const RecordObject& record_obj) {}
virtual bool PreCallValidateReleaseProfilingLockKHR(VkDevice device, ErrorObject& error_obj) const { return false; }
virtual void PreCallRecordReleaseProfilingLockKHR(VkDevice device, const RecordObject& record_obj) {}
virtual void PostCallRecordReleaseProfilingLockKHR(VkDevice device, const RecordObject& record_obj) {}
virtual bool PreCallValidateGetImageMemoryRequirements2KHR(VkDevice device, const VkImageMemoryRequirementsInfo2* pInfo,
                                                           VkMemoryRequirements2* pMemoryRequirements,
                                                           ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordGetImageMemoryRequirements2KHR(VkDevice device, const VkImageMemoryRequirementsInfo2* pInfo,
                                                         VkMemoryRequirements2* pMemoryRequirements,
                                                         const RecordObject& record_obj) {}
virtual void PostCallRecordGetImageMemoryRequirements2KHR(VkDevice device, const VkImageMemoryRequirementsInfo2* pInfo,
                                                          VkMemoryRequirements2* pMemoryRequirements,
                                                          const RecordObject& record_obj) {}
virtual bool PreCallValidateGetBufferMemoryRequirements2KHR(VkDevice device, const VkBufferMemoryRequirementsInfo2* pInfo,
                                                            VkMemoryRequirements2* pMemoryRequirements,
                                                            ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordGetBufferMemoryRequirements2KHR(VkDevice device, const VkBufferMemoryRequirementsInfo2* pInfo,
                                                          VkMemoryRequirements2* pMemoryRequirements,
                                                          const RecordObject& record_obj) {}
virtual void PostCallRecordGetBufferMemoryRequirements2KHR(VkDevice device, const VkBufferMemoryRequirementsInfo2* pInfo,
                                                           VkMemoryRequirements2* pMemoryRequirements,
                                                           const RecordObject& record_obj) {}
virtual bool PreCallValidateGetImageSparseMemoryRequirements2KHR(VkDevice device, const VkImageSparseMemoryRequirementsInfo2* pInfo,
                                                                 uint32_t* pSparseMemoryRequirementCount,
                                                                 VkSparseImageMemoryRequirements2* pSparseMemoryRequirements,
                                                                 ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordGetImageSparseMemoryRequirements2KHR(VkDevice device, const VkImageSparseMemoryRequirementsInfo2* pInfo,
                                                               uint32_t* pSparseMemoryRequirementCount,
                                                               VkSparseImageMemoryRequirements2* pSparseMemoryRequirements,
                                                               const RecordObject& record_obj) {}
virtual void PostCallRecordGetImageSparseMemoryRequirements2KHR(VkDevice device, const VkImageSparseMemoryRequirementsInfo2* pInfo,
                                                                uint32_t* pSparseMemoryRequirementCount,
                                                                VkSparseImageMemoryRequirements2* pSparseMemoryRequirements,
                                                                const RecordObject& record_obj) {}
virtual bool PreCallValidateCreateSamplerYcbcrConversionKHR(VkDevice device, const VkSamplerYcbcrConversionCreateInfo* pCreateInfo,
                                                            const VkAllocationCallbacks* pAllocator,
                                                            VkSamplerYcbcrConversion* pYcbcrConversion,
                                                            ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordCreateSamplerYcbcrConversionKHR(VkDevice device, const VkSamplerYcbcrConversionCreateInfo* pCreateInfo,
                                                          const VkAllocationCallbacks* pAllocator,
                                                          VkSamplerYcbcrConversion* pYcbcrConversion,
                                                          const RecordObject& record_obj) {}
virtual void PostCallRecordCreateSamplerYcbcrConversionKHR(VkDevice device, const VkSamplerYcbcrConversionCreateInfo* pCreateInfo,
                                                           const VkAllocationCallbacks* pAllocator,
                                                           VkSamplerYcbcrConversion* pYcbcrConversion,
                                                           const RecordObject& record_obj) {}
virtual bool PreCallValidateDestroySamplerYcbcrConversionKHR(VkDevice device, VkSamplerYcbcrConversion ycbcrConversion,
                                                             const VkAllocationCallbacks* pAllocator,
                                                             ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordDestroySamplerYcbcrConversionKHR(VkDevice device, VkSamplerYcbcrConversion ycbcrConversion,
                                                           const VkAllocationCallbacks* pAllocator,
                                                           const RecordObject& record_obj) {}
virtual void PostCallRecordDestroySamplerYcbcrConversionKHR(VkDevice device, VkSamplerYcbcrConversion ycbcrConversion,
                                                            const VkAllocationCallbacks* pAllocator,
                                                            const RecordObject& record_obj) {}
virtual bool PreCallValidateBindBufferMemory2KHR(VkDevice device, uint32_t bindInfoCount, const VkBindBufferMemoryInfo* pBindInfos,
                                                 ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordBindBufferMemory2KHR(VkDevice device, uint32_t bindInfoCount, const VkBindBufferMemoryInfo* pBindInfos,
                                               const RecordObject& record_obj) {}
virtual void PostCallRecordBindBufferMemory2KHR(VkDevice device, uint32_t bindInfoCount, const VkBindBufferMemoryInfo* pBindInfos,
                                                const RecordObject& record_obj) {}
virtual bool PreCallValidateBindImageMemory2KHR(VkDevice device, uint32_t bindInfoCount, const VkBindImageMemoryInfo* pBindInfos,
                                                ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordBindImageMemory2KHR(VkDevice device, uint32_t bindInfoCount, const VkBindImageMemoryInfo* pBindInfos,
                                              const RecordObject& record_obj) {}
virtual void PostCallRecordBindImageMemory2KHR(VkDevice device, uint32_t bindInfoCount, const VkBindImageMemoryInfo* pBindInfos,
                                               const RecordObject& record_obj) {}
virtual bool PreCallValidateGetDescriptorSetLayoutSupportKHR(VkDevice device, const VkDescriptorSetLayoutCreateInfo* pCreateInfo,
                                                             VkDescriptorSetLayoutSupport* pSupport, ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordGetDescriptorSetLayoutSupportKHR(VkDevice device, const VkDescriptorSetLayoutCreateInfo* pCreateInfo,
                                                           VkDescriptorSetLayoutSupport* pSupport, const RecordObject& record_obj) {
}
virtual void PostCallRecordGetDescriptorSetLayoutSupportKHR(VkDevice device, const VkDescriptorSetLayoutCreateInfo* pCreateInfo,
                                                            VkDescriptorSetLayoutSupport* pSupport,
                                                            const RecordObject& record_obj) {}
virtual bool PreCallValidateCmdDrawIndirectCountKHR(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                    VkBuffer countBuffer, VkDeviceSize countBufferOffset, uint32_t maxDrawCount,
                                                    uint32_t stride, ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordCmdDrawIndirectCountKHR(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                  VkBuffer countBuffer, VkDeviceSize countBufferOffset, uint32_t maxDrawCount,
                                                  uint32_t stride, const RecordObject& record_obj) {}
virtual void PostCallRecordCmdDrawIndirectCountKHR(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                   VkBuffer countBuffer, VkDeviceSize countBufferOffset, uint32_t maxDrawCount,
                                                   uint32_t stride, const RecordObject& record_obj) {}
virtual bool PreCallValidateCmdDrawIndexedIndirectCountKHR(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                           VkBuffer countBuffer, VkDeviceSize countBufferOffset,
                                                           uint32_t maxDrawCount, uint32_t stride, ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordCmdDrawIndexedIndirectCountKHR(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                         VkBuffer countBuffer, VkDeviceSize countBufferOffset,
                                                         uint32_t maxDrawCount, uint32_t stride, const RecordObject& record_obj) {}
virtual void PostCallRecordCmdDrawIndexedIndirectCountKHR(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                          VkBuffer countBuffer, VkDeviceSize countBufferOffset,
                                                          uint32_t maxDrawCount, uint32_t stride, const RecordObject& record_obj) {}
virtual bool PreCallValidateGetSemaphoreCounterValueKHR(VkDevice device, VkSemaphore semaphore, uint64_t* pValue,
                                                        ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordGetSemaphoreCounterValueKHR(VkDevice device, VkSemaphore semaphore, uint64_t* pValue,
                                                      const RecordObject& record_obj) {}
virtual void PostCallRecordGetSemaphoreCounterValueKHR(VkDevice device, VkSemaphore semaphore, uint64_t* pValue,
                                                       const RecordObject& record_obj) {}
virtual bool PreCallValidateWaitSemaphoresKHR(VkDevice device, const VkSemaphoreWaitInfo* pWaitInfo, uint64_t timeout,
                                              ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordWaitSemaphoresKHR(VkDevice device, const VkSemaphoreWaitInfo* pWaitInfo, uint64_t timeout,
                                            const RecordObject& record_obj) {}
virtual void PostCallRecordWaitSemaphoresKHR(VkDevice device, const VkSemaphoreWaitInfo* pWaitInfo, uint64_t timeout,
                                             const RecordObject& record_obj) {}
virtual bool PreCallValidateSignalSemaphoreKHR(VkDevice device, const VkSemaphoreSignalInfo* pSignalInfo,
                                               ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordSignalSemaphoreKHR(VkDevice device, const VkSemaphoreSignalInfo* pSignalInfo,
                                             const RecordObject& record_obj) {}
virtual void PostCallRecordSignalSemaphoreKHR(VkDevice device, const VkSemaphoreSignalInfo* pSignalInfo,
                                              const RecordObject& record_obj) {}
virtual bool PreCallValidateCmdSetFragmentShadingRateKHR(VkCommandBuffer commandBuffer, const VkExtent2D* pFragmentSize,
                                                         const VkFragmentShadingRateCombinerOpKHR combinerOps[2],
                                                         ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordCmdSetFragmentShadingRateKHR(VkCommandBuffer commandBuffer, const VkExtent2D* pFragmentSize,
                                                       const VkFragmentShadingRateCombinerOpKHR combinerOps[2],
                                                       const RecordObject& record_obj) {}
virtual void PostCallRecordCmdSetFragmentShadingRateKHR(VkCommandBuffer commandBuffer, const VkExtent2D* pFragmentSize,
                                                        const VkFragmentShadingRateCombinerOpKHR combinerOps[2],
                                                        const RecordObject& record_obj) {}
virtual bool PreCallValidateCmdSetRenderingAttachmentLocationsKHR(VkCommandBuffer commandBuffer,
                                                                  const VkRenderingAttachmentLocationInfo* pLocationInfo,
                                                                  ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordCmdSetRenderingAttachmentLocationsKHR(VkCommandBuffer commandBuffer,
                                                                const VkRenderingAttachmentLocationInfo* pLocationInfo,
                                                                const RecordObject& record_obj) {}
virtual void PostCallRecordCmdSetRenderingAttachmentLocationsKHR(VkCommandBuffer commandBuffer,
                                                                 const VkRenderingAttachmentLocationInfo* pLocationInfo,
                                                                 const RecordObject& record_obj) {}
virtual bool PreCallValidateCmdSetRenderingInputAttachmentIndicesKHR(
    VkCommandBuffer commandBuffer, const VkRenderingInputAttachmentIndexInfo* pInputAttachmentIndexInfo,
    ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordCmdSetRenderingInputAttachmentIndicesKHR(
    VkCommandBuffer commandBuffer, const VkRenderingInputAttachmentIndexInfo* pInputAttachmentIndexInfo,
    const RecordObject& record_obj) {}
virtual void PostCallRecordCmdSetRenderingInputAttachmentIndicesKHR(
    VkCommandBuffer commandBuffer, const VkRenderingInputAttachmentIndexInfo* pInputAttachmentIndexInfo,
    const RecordObject& record_obj) {}
virtual bool PreCallValidateWaitForPresentKHR(VkDevice device, VkSwapchainKHR swapchain, uint64_t presentId, uint64_t timeout,
                                              ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordWaitForPresentKHR(VkDevice device, VkSwapchainKHR swapchain, uint64_t presentId, uint64_t timeout,
                                            const RecordObject& record_obj) {}
virtual void PostCallRecordWaitForPresentKHR(VkDevice device, VkSwapchainKHR swapchain, uint64_t presentId, uint64_t timeout,
                                             const RecordObject& record_obj) {}
virtual bool PreCallValidateGetBufferDeviceAddressKHR(VkDevice device, const VkBufferDeviceAddressInfo* pInfo,
                                                      ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordGetBufferDeviceAddressKHR(VkDevice device, const VkBufferDeviceAddressInfo* pInfo,
                                                    const RecordObject& record_obj) {}
virtual void PostCallRecordGetBufferDeviceAddressKHR(VkDevice device, const VkBufferDeviceAddressInfo* pInfo,
                                                     const RecordObject& record_obj) {}
virtual bool PreCallValidateGetBufferOpaqueCaptureAddressKHR(VkDevice device, const VkBufferDeviceAddressInfo* pInfo,
                                                             ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordGetBufferOpaqueCaptureAddressKHR(VkDevice device, const VkBufferDeviceAddressInfo* pInfo,
                                                           const RecordObject& record_obj) {}
virtual void PostCallRecordGetBufferOpaqueCaptureAddressKHR(VkDevice device, const VkBufferDeviceAddressInfo* pInfo,
                                                            const RecordObject& record_obj) {}
virtual bool PreCallValidateGetDeviceMemoryOpaqueCaptureAddressKHR(VkDevice device,
                                                                   const VkDeviceMemoryOpaqueCaptureAddressInfo* pInfo,
                                                                   ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordGetDeviceMemoryOpaqueCaptureAddressKHR(VkDevice device,
                                                                 const VkDeviceMemoryOpaqueCaptureAddressInfo* pInfo,
                                                                 const RecordObject& record_obj) {}
virtual void PostCallRecordGetDeviceMemoryOpaqueCaptureAddressKHR(VkDevice device,
                                                                  const VkDeviceMemoryOpaqueCaptureAddressInfo* pInfo,
                                                                  const RecordObject& record_obj) {}
virtual bool PreCallValidateCreateDeferredOperationKHR(VkDevice device, const VkAllocationCallbacks* pAllocator,
                                                       VkDeferredOperationKHR* pDeferredOperation, ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordCreateDeferredOperationKHR(VkDevice device, const VkAllocationCallbacks* pAllocator,
                                                     VkDeferredOperationKHR* pDeferredOperation, const RecordObject& record_obj) {}
virtual void PostCallRecordCreateDeferredOperationKHR(VkDevice device, const VkAllocationCallbacks* pAllocator,
                                                      VkDeferredOperationKHR* pDeferredOperation, const RecordObject& record_obj) {}
virtual bool PreCallValidateDestroyDeferredOperationKHR(VkDevice device, VkDeferredOperationKHR operation,
                                                        const VkAllocationCallbacks* pAllocator, ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordDestroyDeferredOperationKHR(VkDevice device, VkDeferredOperationKHR operation,
                                                      const VkAllocationCallbacks* pAllocator, const RecordObject& record_obj) {}
virtual void PostCallRecordDestroyDeferredOperationKHR(VkDevice device, VkDeferredOperationKHR operation,
                                                       const VkAllocationCallbacks* pAllocator, const RecordObject& record_obj) {}
virtual bool PreCallValidateGetDeferredOperationMaxConcurrencyKHR(VkDevice device, VkDeferredOperationKHR operation,
                                                                  ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordGetDeferredOperationMaxConcurrencyKHR(VkDevice device, VkDeferredOperationKHR operation,
                                                                const RecordObject& record_obj) {}
virtual void PostCallRecordGetDeferredOperationMaxConcurrencyKHR(VkDevice device, VkDeferredOperationKHR operation,
                                                                 const RecordObject& record_obj) {}
virtual bool PreCallValidateGetDeferredOperationResultKHR(VkDevice device, VkDeferredOperationKHR operation,
                                                          ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordGetDeferredOperationResultKHR(VkDevice device, VkDeferredOperationKHR operation,
                                                        const RecordObject& record_obj) {}
virtual void PostCallRecordGetDeferredOperationResultKHR(VkDevice device, VkDeferredOperationKHR operation,
                                                         const RecordObject& record_obj) {}
virtual bool PreCallValidateDeferredOperationJoinKHR(VkDevice device, VkDeferredOperationKHR operation,
                                                     ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordDeferredOperationJoinKHR(VkDevice device, VkDeferredOperationKHR operation,
                                                   const RecordObject& record_obj) {}
virtual void PostCallRecordDeferredOperationJoinKHR(VkDevice device, VkDeferredOperationKHR operation,
                                                    const RecordObject& record_obj) {}
virtual bool PreCallValidateGetPipelineExecutablePropertiesKHR(VkDevice device, const VkPipelineInfoKHR* pPipelineInfo,
                                                               uint32_t* pExecutableCount,
                                                               VkPipelineExecutablePropertiesKHR* pProperties,
                                                               ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordGetPipelineExecutablePropertiesKHR(VkDevice device, const VkPipelineInfoKHR* pPipelineInfo,
                                                             uint32_t* pExecutableCount,
                                                             VkPipelineExecutablePropertiesKHR* pProperties,
                                                             const RecordObject& record_obj) {}
virtual void PostCallRecordGetPipelineExecutablePropertiesKHR(VkDevice device, const VkPipelineInfoKHR* pPipelineInfo,
                                                              uint32_t* pExecutableCount,
                                                              VkPipelineExecutablePropertiesKHR* pProperties,
                                                              const RecordObject& record_obj) {}
virtual bool PreCallValidateGetPipelineExecutableStatisticsKHR(VkDevice device, const VkPipelineExecutableInfoKHR* pExecutableInfo,
                                                               uint32_t* pStatisticCount,
                                                               VkPipelineExecutableStatisticKHR* pStatistics,
                                                               ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordGetPipelineExecutableStatisticsKHR(VkDevice device, const VkPipelineExecutableInfoKHR* pExecutableInfo,
                                                             uint32_t* pStatisticCount,
                                                             VkPipelineExecutableStatisticKHR* pStatistics,
                                                             const RecordObject& record_obj) {}
virtual void PostCallRecordGetPipelineExecutableStatisticsKHR(VkDevice device, const VkPipelineExecutableInfoKHR* pExecutableInfo,
                                                              uint32_t* pStatisticCount,
                                                              VkPipelineExecutableStatisticKHR* pStatistics,
                                                              const RecordObject& record_obj) {}
virtual bool PreCallValidateGetPipelineExecutableInternalRepresentationsKHR(
    VkDevice device, const VkPipelineExecutableInfoKHR* pExecutableInfo, uint32_t* pInternalRepresentationCount,
    VkPipelineExecutableInternalRepresentationKHR* pInternalRepresentations, ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordGetPipelineExecutableInternalRepresentationsKHR(
    VkDevice device, const VkPipelineExecutableInfoKHR* pExecutableInfo, uint32_t* pInternalRepresentationCount,
    VkPipelineExecutableInternalRepresentationKHR* pInternalRepresentations, const RecordObject& record_obj) {}
virtual void PostCallRecordGetPipelineExecutableInternalRepresentationsKHR(
    VkDevice device, const VkPipelineExecutableInfoKHR* pExecutableInfo, uint32_t* pInternalRepresentationCount,
    VkPipelineExecutableInternalRepresentationKHR* pInternalRepresentations, const RecordObject& record_obj) {}
virtual bool PreCallValidateMapMemory2KHR(VkDevice device, const VkMemoryMapInfo* pMemoryMapInfo, void** ppData,
                                          ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordMapMemory2KHR(VkDevice device, const VkMemoryMapInfo* pMemoryMapInfo, void** ppData,
                                        const RecordObject& record_obj) {}
virtual void PostCallRecordMapMemory2KHR(VkDevice device, const VkMemoryMapInfo* pMemoryMapInfo, void** ppData,
                                         const RecordObject& record_obj) {}
virtual bool PreCallValidateUnmapMemory2KHR(VkDevice device, const VkMemoryUnmapInfo* pMemoryUnmapInfo,
                                            ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordUnmapMemory2KHR(VkDevice device, const VkMemoryUnmapInfo* pMemoryUnmapInfo,
                                          const RecordObject& record_obj) {}
virtual void PostCallRecordUnmapMemory2KHR(VkDevice device, const VkMemoryUnmapInfo* pMemoryUnmapInfo,
                                           const RecordObject& record_obj) {}
virtual bool PreCallValidateGetEncodedVideoSessionParametersKHR(
    VkDevice device, const VkVideoEncodeSessionParametersGetInfoKHR* pVideoSessionParametersInfo,
    VkVideoEncodeSessionParametersFeedbackInfoKHR* pFeedbackInfo, size_t* pDataSize, void* pData, ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordGetEncodedVideoSessionParametersKHR(
    VkDevice device, const VkVideoEncodeSessionParametersGetInfoKHR* pVideoSessionParametersInfo,
    VkVideoEncodeSessionParametersFeedbackInfoKHR* pFeedbackInfo, size_t* pDataSize, void* pData, const RecordObject& record_obj) {}
virtual void PostCallRecordGetEncodedVideoSessionParametersKHR(
    VkDevice device, const VkVideoEncodeSessionParametersGetInfoKHR* pVideoSessionParametersInfo,
    VkVideoEncodeSessionParametersFeedbackInfoKHR* pFeedbackInfo, size_t* pDataSize, void* pData, const RecordObject& record_obj) {}
virtual bool PreCallValidateCmdEncodeVideoKHR(VkCommandBuffer commandBuffer, const VkVideoEncodeInfoKHR* pEncodeInfo,
                                              ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordCmdEncodeVideoKHR(VkCommandBuffer commandBuffer, const VkVideoEncodeInfoKHR* pEncodeInfo,
                                            const RecordObject& record_obj) {}
virtual void PostCallRecordCmdEncodeVideoKHR(VkCommandBuffer commandBuffer, const VkVideoEncodeInfoKHR* pEncodeInfo,
                                             const RecordObject& record_obj) {}
virtual bool PreCallValidateCmdSetEvent2KHR(VkCommandBuffer commandBuffer, VkEvent event, const VkDependencyInfo* pDependencyInfo,
                                            ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordCmdSetEvent2KHR(VkCommandBuffer commandBuffer, VkEvent event, const VkDependencyInfo* pDependencyInfo,
                                          const RecordObject& record_obj) {}
virtual void PostCallRecordCmdSetEvent2KHR(VkCommandBuffer commandBuffer, VkEvent event, const VkDependencyInfo* pDependencyInfo,
                                           const RecordObject& record_obj) {}
virtual bool PreCallValidateCmdResetEvent2KHR(VkCommandBuffer commandBuffer, VkEvent event, VkPipelineStageFlags2 stageMask,
                                              ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordCmdResetEvent2KHR(VkCommandBuffer commandBuffer, VkEvent event, VkPipelineStageFlags2 stageMask,
                                            const RecordObject& record_obj) {}
virtual void PostCallRecordCmdResetEvent2KHR(VkCommandBuffer commandBuffer, VkEvent event, VkPipelineStageFlags2 stageMask,
                                             const RecordObject& record_obj) {}
virtual bool PreCallValidateCmdWaitEvents2KHR(VkCommandBuffer commandBuffer, uint32_t eventCount, const VkEvent* pEvents,
                                              const VkDependencyInfo* pDependencyInfos, ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordCmdWaitEvents2KHR(VkCommandBuffer commandBuffer, uint32_t eventCount, const VkEvent* pEvents,
                                            const VkDependencyInfo* pDependencyInfos, const RecordObject& record_obj) {}
virtual void PostCallRecordCmdWaitEvents2KHR(VkCommandBuffer commandBuffer, uint32_t eventCount, const VkEvent* pEvents,
                                             const VkDependencyInfo* pDependencyInfos, const RecordObject& record_obj) {}
virtual bool PreCallValidateCmdPipelineBarrier2KHR(VkCommandBuffer commandBuffer, const VkDependencyInfo* pDependencyInfo,
                                                   ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordCmdPipelineBarrier2KHR(VkCommandBuffer commandBuffer, const VkDependencyInfo* pDependencyInfo,
                                                 const RecordObject& record_obj) {}
virtual void PostCallRecordCmdPipelineBarrier2KHR(VkCommandBuffer commandBuffer, const VkDependencyInfo* pDependencyInfo,
                                                  const RecordObject& record_obj) {}
virtual bool PreCallValidateCmdWriteTimestamp2KHR(VkCommandBuffer commandBuffer, VkPipelineStageFlags2 stage, VkQueryPool queryPool,
                                                  uint32_t query, ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordCmdWriteTimestamp2KHR(VkCommandBuffer commandBuffer, VkPipelineStageFlags2 stage, VkQueryPool queryPool,
                                                uint32_t query, const RecordObject& record_obj) {}
virtual void PostCallRecordCmdWriteTimestamp2KHR(VkCommandBuffer commandBuffer, VkPipelineStageFlags2 stage, VkQueryPool queryPool,
                                                 uint32_t query, const RecordObject& record_obj) {}
virtual bool PreCallValidateQueueSubmit2KHR(VkQueue queue, uint32_t submitCount, const VkSubmitInfo2* pSubmits, VkFence fence,
                                            ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordQueueSubmit2KHR(VkQueue queue, uint32_t submitCount, const VkSubmitInfo2* pSubmits, VkFence fence,
                                          const RecordObject& record_obj) {}
virtual void PostCallRecordQueueSubmit2KHR(VkQueue queue, uint32_t submitCount, const VkSubmitInfo2* pSubmits, VkFence fence,
                                           const RecordObject& record_obj) {}
virtual bool PreCallValidateCmdCopyBuffer2KHR(VkCommandBuffer commandBuffer, const VkCopyBufferInfo2* pCopyBufferInfo,
                                              ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordCmdCopyBuffer2KHR(VkCommandBuffer commandBuffer, const VkCopyBufferInfo2* pCopyBufferInfo,
                                            const RecordObject& record_obj) {}
virtual void PostCallRecordCmdCopyBuffer2KHR(VkCommandBuffer commandBuffer, const VkCopyBufferInfo2* pCopyBufferInfo,
                                             const RecordObject& record_obj) {}
virtual bool PreCallValidateCmdCopyImage2KHR(VkCommandBuffer commandBuffer, const VkCopyImageInfo2* pCopyImageInfo,
                                             ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordCmdCopyImage2KHR(VkCommandBuffer commandBuffer, const VkCopyImageInfo2* pCopyImageInfo,
                                           const RecordObject& record_obj) {}
virtual void PostCallRecordCmdCopyImage2KHR(VkCommandBuffer commandBuffer, const VkCopyImageInfo2* pCopyImageInfo,
                                            const RecordObject& record_obj) {}
virtual bool PreCallValidateCmdCopyBufferToImage2KHR(VkCommandBuffer commandBuffer,
                                                     const VkCopyBufferToImageInfo2* pCopyBufferToImageInfo,
                                                     ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordCmdCopyBufferToImage2KHR(VkCommandBuffer commandBuffer,
                                                   const VkCopyBufferToImageInfo2* pCopyBufferToImageInfo,
                                                   const RecordObject& record_obj) {}
virtual void PostCallRecordCmdCopyBufferToImage2KHR(VkCommandBuffer commandBuffer,
                                                    const VkCopyBufferToImageInfo2* pCopyBufferToImageInfo,
                                                    const RecordObject& record_obj) {}
virtual bool PreCallValidateCmdCopyImageToBuffer2KHR(VkCommandBuffer commandBuffer,
                                                     const VkCopyImageToBufferInfo2* pCopyImageToBufferInfo,
                                                     ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordCmdCopyImageToBuffer2KHR(VkCommandBuffer commandBuffer,
                                                   const VkCopyImageToBufferInfo2* pCopyImageToBufferInfo,
                                                   const RecordObject& record_obj) {}
virtual void PostCallRecordCmdCopyImageToBuffer2KHR(VkCommandBuffer commandBuffer,
                                                    const VkCopyImageToBufferInfo2* pCopyImageToBufferInfo,
                                                    const RecordObject& record_obj) {}
virtual bool PreCallValidateCmdBlitImage2KHR(VkCommandBuffer commandBuffer, const VkBlitImageInfo2* pBlitImageInfo,
                                             ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordCmdBlitImage2KHR(VkCommandBuffer commandBuffer, const VkBlitImageInfo2* pBlitImageInfo,
                                           const RecordObject& record_obj) {}
virtual void PostCallRecordCmdBlitImage2KHR(VkCommandBuffer commandBuffer, const VkBlitImageInfo2* pBlitImageInfo,
                                            const RecordObject& record_obj) {}
virtual bool PreCallValidateCmdResolveImage2KHR(VkCommandBuffer commandBuffer, const VkResolveImageInfo2* pResolveImageInfo,
                                                ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordCmdResolveImage2KHR(VkCommandBuffer commandBuffer, const VkResolveImageInfo2* pResolveImageInfo,
                                              const RecordObject& record_obj) {}
virtual void PostCallRecordCmdResolveImage2KHR(VkCommandBuffer commandBuffer, const VkResolveImageInfo2* pResolveImageInfo,
                                               const RecordObject& record_obj) {}
virtual bool PreCallValidateCmdTraceRaysIndirect2KHR(VkCommandBuffer commandBuffer, VkDeviceAddress indirectDeviceAddress,
                                                     ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordCmdTraceRaysIndirect2KHR(VkCommandBuffer commandBuffer, VkDeviceAddress indirectDeviceAddress,
                                                   const RecordObject& record_obj) {}
virtual void PostCallRecordCmdTraceRaysIndirect2KHR(VkCommandBuffer commandBuffer, VkDeviceAddress indirectDeviceAddress,
                                                    const RecordObject& record_obj) {}
virtual bool PreCallValidateGetDeviceBufferMemoryRequirementsKHR(VkDevice device, const VkDeviceBufferMemoryRequirements* pInfo,
                                                                 VkMemoryRequirements2* pMemoryRequirements,
                                                                 ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordGetDeviceBufferMemoryRequirementsKHR(VkDevice device, const VkDeviceBufferMemoryRequirements* pInfo,
                                                               VkMemoryRequirements2* pMemoryRequirements,
                                                               const RecordObject& record_obj) {}
virtual void PostCallRecordGetDeviceBufferMemoryRequirementsKHR(VkDevice device, const VkDeviceBufferMemoryRequirements* pInfo,
                                                                VkMemoryRequirements2* pMemoryRequirements,
                                                                const RecordObject& record_obj) {}
virtual bool PreCallValidateGetDeviceImageMemoryRequirementsKHR(VkDevice device, const VkDeviceImageMemoryRequirements* pInfo,
                                                                VkMemoryRequirements2* pMemoryRequirements,
                                                                ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordGetDeviceImageMemoryRequirementsKHR(VkDevice device, const VkDeviceImageMemoryRequirements* pInfo,
                                                              VkMemoryRequirements2* pMemoryRequirements,
                                                              const RecordObject& record_obj) {}
virtual void PostCallRecordGetDeviceImageMemoryRequirementsKHR(VkDevice device, const VkDeviceImageMemoryRequirements* pInfo,
                                                               VkMemoryRequirements2* pMemoryRequirements,
                                                               const RecordObject& record_obj) {}
virtual bool PreCallValidateGetDeviceImageSparseMemoryRequirementsKHR(VkDevice device, const VkDeviceImageMemoryRequirements* pInfo,
                                                                      uint32_t* pSparseMemoryRequirementCount,
                                                                      VkSparseImageMemoryRequirements2* pSparseMemoryRequirements,
                                                                      ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordGetDeviceImageSparseMemoryRequirementsKHR(VkDevice device, const VkDeviceImageMemoryRequirements* pInfo,
                                                                    uint32_t* pSparseMemoryRequirementCount,
                                                                    VkSparseImageMemoryRequirements2* pSparseMemoryRequirements,
                                                                    const RecordObject& record_obj) {}
virtual void PostCallRecordGetDeviceImageSparseMemoryRequirementsKHR(VkDevice device, const VkDeviceImageMemoryRequirements* pInfo,
                                                                     uint32_t* pSparseMemoryRequirementCount,
                                                                     VkSparseImageMemoryRequirements2* pSparseMemoryRequirements,
                                                                     const RecordObject& record_obj) {}
virtual bool PreCallValidateCmdBindIndexBuffer2KHR(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                   VkDeviceSize size, VkIndexType indexType, ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordCmdBindIndexBuffer2KHR(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                 VkDeviceSize size, VkIndexType indexType, const RecordObject& record_obj) {}
virtual void PostCallRecordCmdBindIndexBuffer2KHR(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                  VkDeviceSize size, VkIndexType indexType, const RecordObject& record_obj) {}
virtual bool PreCallValidateGetRenderingAreaGranularityKHR(VkDevice device, const VkRenderingAreaInfo* pRenderingAreaInfo,
                                                           VkExtent2D* pGranularity, ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordGetRenderingAreaGranularityKHR(VkDevice device, const VkRenderingAreaInfo* pRenderingAreaInfo,
                                                         VkExtent2D* pGranularity, const RecordObject& record_obj) {}
virtual void PostCallRecordGetRenderingAreaGranularityKHR(VkDevice device, const VkRenderingAreaInfo* pRenderingAreaInfo,
                                                          VkExtent2D* pGranularity, const RecordObject& record_obj) {}
virtual bool PreCallValidateGetDeviceImageSubresourceLayoutKHR(VkDevice device, const VkDeviceImageSubresourceInfo* pInfo,
                                                               VkSubresourceLayout2* pLayout, ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordGetDeviceImageSubresourceLayoutKHR(VkDevice device, const VkDeviceImageSubresourceInfo* pInfo,
                                                             VkSubresourceLayout2* pLayout, const RecordObject& record_obj) {}
virtual void PostCallRecordGetDeviceImageSubresourceLayoutKHR(VkDevice device, const VkDeviceImageSubresourceInfo* pInfo,
                                                              VkSubresourceLayout2* pLayout, const RecordObject& record_obj) {}
virtual bool PreCallValidateGetImageSubresourceLayout2KHR(VkDevice device, VkImage image, const VkImageSubresource2* pSubresource,
                                                          VkSubresourceLayout2* pLayout, ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordGetImageSubresourceLayout2KHR(VkDevice device, VkImage image, const VkImageSubresource2* pSubresource,
                                                        VkSubresourceLayout2* pLayout, const RecordObject& record_obj) {}
virtual void PostCallRecordGetImageSubresourceLayout2KHR(VkDevice device, VkImage image, const VkImageSubresource2* pSubresource,
                                                         VkSubresourceLayout2* pLayout, const RecordObject& record_obj) {}
virtual bool PreCallValidateCreatePipelineBinariesKHR(VkDevice device, const VkPipelineBinaryCreateInfoKHR* pCreateInfo,
                                                      const VkAllocationCallbacks* pAllocator,
                                                      VkPipelineBinaryHandlesInfoKHR* pBinaries, ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordCreatePipelineBinariesKHR(VkDevice device, const VkPipelineBinaryCreateInfoKHR* pCreateInfo,
                                                    const VkAllocationCallbacks* pAllocator,
                                                    VkPipelineBinaryHandlesInfoKHR* pBinaries, const RecordObject& record_obj) {}
virtual void PostCallRecordCreatePipelineBinariesKHR(VkDevice device, const VkPipelineBinaryCreateInfoKHR* pCreateInfo,
                                                     const VkAllocationCallbacks* pAllocator,
                                                     VkPipelineBinaryHandlesInfoKHR* pBinaries, const RecordObject& record_obj) {}
virtual bool PreCallValidateDestroyPipelineBinaryKHR(VkDevice device, VkPipelineBinaryKHR pipelineBinary,
                                                     const VkAllocationCallbacks* pAllocator, ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordDestroyPipelineBinaryKHR(VkDevice device, VkPipelineBinaryKHR pipelineBinary,
                                                   const VkAllocationCallbacks* pAllocator, const RecordObject& record_obj) {}
virtual void PostCallRecordDestroyPipelineBinaryKHR(VkDevice device, VkPipelineBinaryKHR pipelineBinary,
                                                    const VkAllocationCallbacks* pAllocator, const RecordObject& record_obj) {}
virtual bool PreCallValidateGetPipelineKeyKHR(VkDevice device, const VkPipelineCreateInfoKHR* pPipelineCreateInfo,
                                              VkPipelineBinaryKeyKHR* pPipelineKey, ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordGetPipelineKeyKHR(VkDevice device, const VkPipelineCreateInfoKHR* pPipelineCreateInfo,
                                            VkPipelineBinaryKeyKHR* pPipelineKey, const RecordObject& record_obj) {}
virtual void PostCallRecordGetPipelineKeyKHR(VkDevice device, const VkPipelineCreateInfoKHR* pPipelineCreateInfo,
                                             VkPipelineBinaryKeyKHR* pPipelineKey, const RecordObject& record_obj) {}
virtual bool PreCallValidateGetPipelineBinaryDataKHR(VkDevice device, const VkPipelineBinaryDataInfoKHR* pInfo,
                                                     VkPipelineBinaryKeyKHR* pPipelineBinaryKey, size_t* pPipelineBinaryDataSize,
                                                     void* pPipelineBinaryData, ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordGetPipelineBinaryDataKHR(VkDevice device, const VkPipelineBinaryDataInfoKHR* pInfo,
                                                   VkPipelineBinaryKeyKHR* pPipelineBinaryKey, size_t* pPipelineBinaryDataSize,
                                                   void* pPipelineBinaryData, const RecordObject& record_obj) {}
virtual void PostCallRecordGetPipelineBinaryDataKHR(VkDevice device, const VkPipelineBinaryDataInfoKHR* pInfo,
                                                    VkPipelineBinaryKeyKHR* pPipelineBinaryKey, size_t* pPipelineBinaryDataSize,
                                                    void* pPipelineBinaryData, const RecordObject& record_obj) {}
virtual bool PreCallValidateReleaseCapturedPipelineDataKHR(VkDevice device, const VkReleaseCapturedPipelineDataInfoKHR* pInfo,
                                                           const VkAllocationCallbacks* pAllocator, ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordReleaseCapturedPipelineDataKHR(VkDevice device, const VkReleaseCapturedPipelineDataInfoKHR* pInfo,
                                                         const VkAllocationCallbacks* pAllocator, const RecordObject& record_obj) {}
virtual void PostCallRecordReleaseCapturedPipelineDataKHR(VkDevice device, const VkReleaseCapturedPipelineDataInfoKHR* pInfo,
                                                          const VkAllocationCallbacks* pAllocator, const RecordObject& record_obj) {
}
virtual bool PreCallValidateCmdSetLineStippleKHR(VkCommandBuffer commandBuffer, uint32_t lineStippleFactor,
                                                 uint16_t lineStipplePattern, ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordCmdSetLineStippleKHR(VkCommandBuffer commandBuffer, uint32_t lineStippleFactor,
                                               uint16_t lineStipplePattern, const RecordObject& record_obj) {}
virtual void PostCallRecordCmdSetLineStippleKHR(VkCommandBuffer commandBuffer, uint32_t lineStippleFactor,
                                                uint16_t lineStipplePattern, const RecordObject& record_obj) {}
virtual bool PreCallValidateGetCalibratedTimestampsKHR(VkDevice device, uint32_t timestampCount,
                                                       const VkCalibratedTimestampInfoKHR* pTimestampInfos, uint64_t* pTimestamps,
                                                       uint64_t* pMaxDeviation, ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordGetCalibratedTimestampsKHR(VkDevice device, uint32_t timestampCount,
                                                     const VkCalibratedTimestampInfoKHR* pTimestampInfos, uint64_t* pTimestamps,
                                                     uint64_t* pMaxDeviation, const RecordObject& record_obj) {}
virtual void PostCallRecordGetCalibratedTimestampsKHR(VkDevice device, uint32_t timestampCount,
                                                      const VkCalibratedTimestampInfoKHR* pTimestampInfos, uint64_t* pTimestamps,
                                                      uint64_t* pMaxDeviation, const RecordObject& record_obj) {}
virtual bool PreCallValidateCmdBindDescriptorSets2KHR(VkCommandBuffer commandBuffer,
                                                      const VkBindDescriptorSetsInfo* pBindDescriptorSetsInfo,
                                                      ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordCmdBindDescriptorSets2KHR(VkCommandBuffer commandBuffer,
                                                    const VkBindDescriptorSetsInfo* pBindDescriptorSetsInfo,
                                                    const RecordObject& record_obj) {}
virtual void PostCallRecordCmdBindDescriptorSets2KHR(VkCommandBuffer commandBuffer,
                                                     const VkBindDescriptorSetsInfo* pBindDescriptorSetsInfo,
                                                     const RecordObject& record_obj) {}
virtual bool PreCallValidateCmdPushConstants2KHR(VkCommandBuffer commandBuffer, const VkPushConstantsInfo* pPushConstantsInfo,
                                                 ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordCmdPushConstants2KHR(VkCommandBuffer commandBuffer, const VkPushConstantsInfo* pPushConstantsInfo,
                                               const RecordObject& record_obj) {}
virtual void PostCallRecordCmdPushConstants2KHR(VkCommandBuffer commandBuffer, const VkPushConstantsInfo* pPushConstantsInfo,
                                                const RecordObject& record_obj) {}
virtual bool PreCallValidateCmdPushDescriptorSet2KHR(VkCommandBuffer commandBuffer,
                                                     const VkPushDescriptorSetInfo* pPushDescriptorSetInfo,
                                                     ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordCmdPushDescriptorSet2KHR(VkCommandBuffer commandBuffer,
                                                   const VkPushDescriptorSetInfo* pPushDescriptorSetInfo,
                                                   const RecordObject& record_obj) {}
virtual void PostCallRecordCmdPushDescriptorSet2KHR(VkCommandBuffer commandBuffer,
                                                    const VkPushDescriptorSetInfo* pPushDescriptorSetInfo,
                                                    const RecordObject& record_obj) {}
virtual bool PreCallValidateCmdPushDescriptorSetWithTemplate2KHR(
    VkCommandBuffer commandBuffer, const VkPushDescriptorSetWithTemplateInfo* pPushDescriptorSetWithTemplateInfo,
    ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordCmdPushDescriptorSetWithTemplate2KHR(
    VkCommandBuffer commandBuffer, const VkPushDescriptorSetWithTemplateInfo* pPushDescriptorSetWithTemplateInfo,
    const RecordObject& record_obj) {}
virtual void PostCallRecordCmdPushDescriptorSetWithTemplate2KHR(
    VkCommandBuffer commandBuffer, const VkPushDescriptorSetWithTemplateInfo* pPushDescriptorSetWithTemplateInfo,
    const RecordObject& record_obj) {}
virtual bool PreCallValidateCmdSetDescriptorBufferOffsets2EXT(
    VkCommandBuffer commandBuffer, const VkSetDescriptorBufferOffsetsInfoEXT* pSetDescriptorBufferOffsetsInfo,
    ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordCmdSetDescriptorBufferOffsets2EXT(
    VkCommandBuffer commandBuffer, const VkSetDescriptorBufferOffsetsInfoEXT* pSetDescriptorBufferOffsetsInfo,
    const RecordObject& record_obj) {}
virtual void PostCallRecordCmdSetDescriptorBufferOffsets2EXT(
    VkCommandBuffer commandBuffer, const VkSetDescriptorBufferOffsetsInfoEXT* pSetDescriptorBufferOffsetsInfo,
    const RecordObject& record_obj) {}
virtual bool PreCallValidateCmdBindDescriptorBufferEmbeddedSamplers2EXT(
    VkCommandBuffer commandBuffer, const VkBindDescriptorBufferEmbeddedSamplersInfoEXT* pBindDescriptorBufferEmbeddedSamplersInfo,
    ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordCmdBindDescriptorBufferEmbeddedSamplers2EXT(
    VkCommandBuffer commandBuffer, const VkBindDescriptorBufferEmbeddedSamplersInfoEXT* pBindDescriptorBufferEmbeddedSamplersInfo,
    const RecordObject& record_obj) {}
virtual void PostCallRecordCmdBindDescriptorBufferEmbeddedSamplers2EXT(
    VkCommandBuffer commandBuffer, const VkBindDescriptorBufferEmbeddedSamplersInfoEXT* pBindDescriptorBufferEmbeddedSamplersInfo,
    const RecordObject& record_obj) {}
virtual bool PreCallValidateDebugMarkerSetObjectTagEXT(VkDevice device, const VkDebugMarkerObjectTagInfoEXT* pTagInfo,
                                                       ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordDebugMarkerSetObjectTagEXT(VkDevice device, const VkDebugMarkerObjectTagInfoEXT* pTagInfo,
                                                     const RecordObject& record_obj) {}
virtual void PostCallRecordDebugMarkerSetObjectTagEXT(VkDevice device, const VkDebugMarkerObjectTagInfoEXT* pTagInfo,
                                                      const RecordObject& record_obj) {}
virtual bool PreCallValidateDebugMarkerSetObjectNameEXT(VkDevice device, const VkDebugMarkerObjectNameInfoEXT* pNameInfo,
                                                        ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordDebugMarkerSetObjectNameEXT(VkDevice device, const VkDebugMarkerObjectNameInfoEXT* pNameInfo,
                                                      const RecordObject& record_obj) {}
virtual void PostCallRecordDebugMarkerSetObjectNameEXT(VkDevice device, const VkDebugMarkerObjectNameInfoEXT* pNameInfo,
                                                       const RecordObject& record_obj) {}
virtual bool PreCallValidateCmdDebugMarkerBeginEXT(VkCommandBuffer commandBuffer, const VkDebugMarkerMarkerInfoEXT* pMarkerInfo,
                                                   ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordCmdDebugMarkerBeginEXT(VkCommandBuffer commandBuffer, const VkDebugMarkerMarkerInfoEXT* pMarkerInfo,
                                                 const RecordObject& record_obj) {}
virtual void PostCallRecordCmdDebugMarkerBeginEXT(VkCommandBuffer commandBuffer, const VkDebugMarkerMarkerInfoEXT* pMarkerInfo,
                                                  const RecordObject& record_obj) {}
virtual bool PreCallValidateCmdDebugMarkerEndEXT(VkCommandBuffer commandBuffer, ErrorObject& error_obj) const { return false; }
virtual void PreCallRecordCmdDebugMarkerEndEXT(VkCommandBuffer commandBuffer, const RecordObject& record_obj) {}
virtual void PostCallRecordCmdDebugMarkerEndEXT(VkCommandBuffer commandBuffer, const RecordObject& record_obj) {}
virtual bool PreCallValidateCmdDebugMarkerInsertEXT(VkCommandBuffer commandBuffer, const VkDebugMarkerMarkerInfoEXT* pMarkerInfo,
                                                    ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordCmdDebugMarkerInsertEXT(VkCommandBuffer commandBuffer, const VkDebugMarkerMarkerInfoEXT* pMarkerInfo,
                                                  const RecordObject& record_obj) {}
virtual void PostCallRecordCmdDebugMarkerInsertEXT(VkCommandBuffer commandBuffer, const VkDebugMarkerMarkerInfoEXT* pMarkerInfo,
                                                   const RecordObject& record_obj) {}
virtual bool PreCallValidateCmdBindTransformFeedbackBuffersEXT(VkCommandBuffer commandBuffer, uint32_t firstBinding,
                                                               uint32_t bindingCount, const VkBuffer* pBuffers,
                                                               const VkDeviceSize* pOffsets, const VkDeviceSize* pSizes,
                                                               ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordCmdBindTransformFeedbackBuffersEXT(VkCommandBuffer commandBuffer, uint32_t firstBinding,
                                                             uint32_t bindingCount, const VkBuffer* pBuffers,
                                                             const VkDeviceSize* pOffsets, const VkDeviceSize* pSizes,
                                                             const RecordObject& record_obj) {}
virtual void PostCallRecordCmdBindTransformFeedbackBuffersEXT(VkCommandBuffer commandBuffer, uint32_t firstBinding,
                                                              uint32_t bindingCount, const VkBuffer* pBuffers,
                                                              const VkDeviceSize* pOffsets, const VkDeviceSize* pSizes,
                                                              const RecordObject& record_obj) {}
virtual bool PreCallValidateCmdBeginTransformFeedbackEXT(VkCommandBuffer commandBuffer, uint32_t firstCounterBuffer,
                                                         uint32_t counterBufferCount, const VkBuffer* pCounterBuffers,
                                                         const VkDeviceSize* pCounterBufferOffsets, ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordCmdBeginTransformFeedbackEXT(VkCommandBuffer commandBuffer, uint32_t firstCounterBuffer,
                                                       uint32_t counterBufferCount, const VkBuffer* pCounterBuffers,
                                                       const VkDeviceSize* pCounterBufferOffsets, const RecordObject& record_obj) {}
virtual void PostCallRecordCmdBeginTransformFeedbackEXT(VkCommandBuffer commandBuffer, uint32_t firstCounterBuffer,
                                                        uint32_t counterBufferCount, const VkBuffer* pCounterBuffers,
                                                        const VkDeviceSize* pCounterBufferOffsets, const RecordObject& record_obj) {
}
virtual bool PreCallValidateCmdEndTransformFeedbackEXT(VkCommandBuffer commandBuffer, uint32_t firstCounterBuffer,
                                                       uint32_t counterBufferCount, const VkBuffer* pCounterBuffers,
                                                       const VkDeviceSize* pCounterBufferOffsets, ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordCmdEndTransformFeedbackEXT(VkCommandBuffer commandBuffer, uint32_t firstCounterBuffer,
                                                     uint32_t counterBufferCount, const VkBuffer* pCounterBuffers,
                                                     const VkDeviceSize* pCounterBufferOffsets, const RecordObject& record_obj) {}
virtual void PostCallRecordCmdEndTransformFeedbackEXT(VkCommandBuffer commandBuffer, uint32_t firstCounterBuffer,
                                                      uint32_t counterBufferCount, const VkBuffer* pCounterBuffers,
                                                      const VkDeviceSize* pCounterBufferOffsets, const RecordObject& record_obj) {}
virtual bool PreCallValidateCmdBeginQueryIndexedEXT(VkCommandBuffer commandBuffer, VkQueryPool queryPool, uint32_t query,
                                                    VkQueryControlFlags flags, uint32_t index, ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordCmdBeginQueryIndexedEXT(VkCommandBuffer commandBuffer, VkQueryPool queryPool, uint32_t query,
                                                  VkQueryControlFlags flags, uint32_t index, const RecordObject& record_obj) {}
virtual void PostCallRecordCmdBeginQueryIndexedEXT(VkCommandBuffer commandBuffer, VkQueryPool queryPool, uint32_t query,
                                                   VkQueryControlFlags flags, uint32_t index, const RecordObject& record_obj) {}
virtual bool PreCallValidateCmdEndQueryIndexedEXT(VkCommandBuffer commandBuffer, VkQueryPool queryPool, uint32_t query,
                                                  uint32_t index, ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordCmdEndQueryIndexedEXT(VkCommandBuffer commandBuffer, VkQueryPool queryPool, uint32_t query,
                                                uint32_t index, const RecordObject& record_obj) {}
virtual void PostCallRecordCmdEndQueryIndexedEXT(VkCommandBuffer commandBuffer, VkQueryPool queryPool, uint32_t query,
                                                 uint32_t index, const RecordObject& record_obj) {}
virtual bool PreCallValidateCmdDrawIndirectByteCountEXT(VkCommandBuffer commandBuffer, uint32_t instanceCount,
                                                        uint32_t firstInstance, VkBuffer counterBuffer,
                                                        VkDeviceSize counterBufferOffset, uint32_t counterOffset,
                                                        uint32_t vertexStride, ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordCmdDrawIndirectByteCountEXT(VkCommandBuffer commandBuffer, uint32_t instanceCount, uint32_t firstInstance,
                                                      VkBuffer counterBuffer, VkDeviceSize counterBufferOffset,
                                                      uint32_t counterOffset, uint32_t vertexStride,
                                                      const RecordObject& record_obj) {}
virtual void PostCallRecordCmdDrawIndirectByteCountEXT(VkCommandBuffer commandBuffer, uint32_t instanceCount,
                                                       uint32_t firstInstance, VkBuffer counterBuffer,
                                                       VkDeviceSize counterBufferOffset, uint32_t counterOffset,
                                                       uint32_t vertexStride, const RecordObject& record_obj) {}
virtual bool PreCallValidateCreateCuModuleNVX(VkDevice device, const VkCuModuleCreateInfoNVX* pCreateInfo,
                                              const VkAllocationCallbacks* pAllocator, VkCuModuleNVX* pModule,
                                              ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordCreateCuModuleNVX(VkDevice device, const VkCuModuleCreateInfoNVX* pCreateInfo,
                                            const VkAllocationCallbacks* pAllocator, VkCuModuleNVX* pModule,
                                            const RecordObject& record_obj) {}
virtual void PostCallRecordCreateCuModuleNVX(VkDevice device, const VkCuModuleCreateInfoNVX* pCreateInfo,
                                             const VkAllocationCallbacks* pAllocator, VkCuModuleNVX* pModule,
                                             const RecordObject& record_obj) {}
virtual bool PreCallValidateCreateCuFunctionNVX(VkDevice device, const VkCuFunctionCreateInfoNVX* pCreateInfo,
                                                const VkAllocationCallbacks* pAllocator, VkCuFunctionNVX* pFunction,
                                                ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordCreateCuFunctionNVX(VkDevice device, const VkCuFunctionCreateInfoNVX* pCreateInfo,
                                              const VkAllocationCallbacks* pAllocator, VkCuFunctionNVX* pFunction,
                                              const RecordObject& record_obj) {}
virtual void PostCallRecordCreateCuFunctionNVX(VkDevice device, const VkCuFunctionCreateInfoNVX* pCreateInfo,
                                               const VkAllocationCallbacks* pAllocator, VkCuFunctionNVX* pFunction,
                                               const RecordObject& record_obj) {}
virtual bool PreCallValidateDestroyCuModuleNVX(VkDevice device, VkCuModuleNVX module, const VkAllocationCallbacks* pAllocator,
                                               ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordDestroyCuModuleNVX(VkDevice device, VkCuModuleNVX module, const VkAllocationCallbacks* pAllocator,
                                             const RecordObject& record_obj) {}
virtual void PostCallRecordDestroyCuModuleNVX(VkDevice device, VkCuModuleNVX module, const VkAllocationCallbacks* pAllocator,
                                              const RecordObject& record_obj) {}
virtual bool PreCallValidateDestroyCuFunctionNVX(VkDevice device, VkCuFunctionNVX function, const VkAllocationCallbacks* pAllocator,
                                                 ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordDestroyCuFunctionNVX(VkDevice device, VkCuFunctionNVX function, const VkAllocationCallbacks* pAllocator,
                                               const RecordObject& record_obj) {}
virtual void PostCallRecordDestroyCuFunctionNVX(VkDevice device, VkCuFunctionNVX function, const VkAllocationCallbacks* pAllocator,
                                                const RecordObject& record_obj) {}
virtual bool PreCallValidateCmdCuLaunchKernelNVX(VkCommandBuffer commandBuffer, const VkCuLaunchInfoNVX* pLaunchInfo,
                                                 ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordCmdCuLaunchKernelNVX(VkCommandBuffer commandBuffer, const VkCuLaunchInfoNVX* pLaunchInfo,
                                               const RecordObject& record_obj) {}
virtual void PostCallRecordCmdCuLaunchKernelNVX(VkCommandBuffer commandBuffer, const VkCuLaunchInfoNVX* pLaunchInfo,
                                                const RecordObject& record_obj) {}
virtual bool PreCallValidateGetImageViewHandleNVX(VkDevice device, const VkImageViewHandleInfoNVX* pInfo,
                                                  ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordGetImageViewHandleNVX(VkDevice device, const VkImageViewHandleInfoNVX* pInfo,
                                                const RecordObject& record_obj) {}
virtual void PostCallRecordGetImageViewHandleNVX(VkDevice device, const VkImageViewHandleInfoNVX* pInfo,
                                                 const RecordObject& record_obj) {}
virtual bool PreCallValidateGetImageViewHandle64NVX(VkDevice device, const VkImageViewHandleInfoNVX* pInfo,
                                                    ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordGetImageViewHandle64NVX(VkDevice device, const VkImageViewHandleInfoNVX* pInfo,
                                                  const RecordObject& record_obj) {}
virtual void PostCallRecordGetImageViewHandle64NVX(VkDevice device, const VkImageViewHandleInfoNVX* pInfo,
                                                   const RecordObject& record_obj) {}
virtual bool PreCallValidateGetImageViewAddressNVX(VkDevice device, VkImageView imageView,
                                                   VkImageViewAddressPropertiesNVX* pProperties, ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordGetImageViewAddressNVX(VkDevice device, VkImageView imageView,
                                                 VkImageViewAddressPropertiesNVX* pProperties, const RecordObject& record_obj) {}
virtual void PostCallRecordGetImageViewAddressNVX(VkDevice device, VkImageView imageView,
                                                  VkImageViewAddressPropertiesNVX* pProperties, const RecordObject& record_obj) {}
virtual bool PreCallValidateCmdDrawIndirectCountAMD(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                    VkBuffer countBuffer, VkDeviceSize countBufferOffset, uint32_t maxDrawCount,
                                                    uint32_t stride, ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordCmdDrawIndirectCountAMD(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                  VkBuffer countBuffer, VkDeviceSize countBufferOffset, uint32_t maxDrawCount,
                                                  uint32_t stride, const RecordObject& record_obj) {}
virtual void PostCallRecordCmdDrawIndirectCountAMD(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                   VkBuffer countBuffer, VkDeviceSize countBufferOffset, uint32_t maxDrawCount,
                                                   uint32_t stride, const RecordObject& record_obj) {}
virtual bool PreCallValidateCmdDrawIndexedIndirectCountAMD(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                           VkBuffer countBuffer, VkDeviceSize countBufferOffset,
                                                           uint32_t maxDrawCount, uint32_t stride, ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordCmdDrawIndexedIndirectCountAMD(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                         VkBuffer countBuffer, VkDeviceSize countBufferOffset,
                                                         uint32_t maxDrawCount, uint32_t stride, const RecordObject& record_obj) {}
virtual void PostCallRecordCmdDrawIndexedIndirectCountAMD(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                          VkBuffer countBuffer, VkDeviceSize countBufferOffset,
                                                          uint32_t maxDrawCount, uint32_t stride, const RecordObject& record_obj) {}
virtual bool PreCallValidateGetShaderInfoAMD(VkDevice device, VkPipeline pipeline, VkShaderStageFlagBits shaderStage,
                                             VkShaderInfoTypeAMD infoType, size_t* pInfoSize, void* pInfo,
                                             ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordGetShaderInfoAMD(VkDevice device, VkPipeline pipeline, VkShaderStageFlagBits shaderStage,
                                           VkShaderInfoTypeAMD infoType, size_t* pInfoSize, void* pInfo,
                                           const RecordObject& record_obj) {}
virtual void PostCallRecordGetShaderInfoAMD(VkDevice device, VkPipeline pipeline, VkShaderStageFlagBits shaderStage,
                                            VkShaderInfoTypeAMD infoType, size_t* pInfoSize, void* pInfo,
                                            const RecordObject& record_obj) {}
#ifdef VK_USE_PLATFORM_WIN32_KHR
virtual bool PreCallValidateGetMemoryWin32HandleNV(VkDevice device, VkDeviceMemory memory,
                                                   VkExternalMemoryHandleTypeFlagsNV handleType, HANDLE* pHandle,
                                                   ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordGetMemoryWin32HandleNV(VkDevice device, VkDeviceMemory memory,
                                                 VkExternalMemoryHandleTypeFlagsNV handleType, HANDLE* pHandle,
                                                 const RecordObject& record_obj) {}
virtual void PostCallRecordGetMemoryWin32HandleNV(VkDevice device, VkDeviceMemory memory,
                                                  VkExternalMemoryHandleTypeFlagsNV handleType, HANDLE* pHandle,
                                                  const RecordObject& record_obj) {}
#endif  // VK_USE_PLATFORM_WIN32_KHR
virtual bool PreCallValidateCmdBeginConditionalRenderingEXT(VkCommandBuffer commandBuffer,
                                                            const VkConditionalRenderingBeginInfoEXT* pConditionalRenderingBegin,
                                                            ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordCmdBeginConditionalRenderingEXT(VkCommandBuffer commandBuffer,
                                                          const VkConditionalRenderingBeginInfoEXT* pConditionalRenderingBegin,
                                                          const RecordObject& record_obj) {}
virtual void PostCallRecordCmdBeginConditionalRenderingEXT(VkCommandBuffer commandBuffer,
                                                           const VkConditionalRenderingBeginInfoEXT* pConditionalRenderingBegin,
                                                           const RecordObject& record_obj) {}
virtual bool PreCallValidateCmdEndConditionalRenderingEXT(VkCommandBuffer commandBuffer, ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordCmdEndConditionalRenderingEXT(VkCommandBuffer commandBuffer, const RecordObject& record_obj) {}
virtual void PostCallRecordCmdEndConditionalRenderingEXT(VkCommandBuffer commandBuffer, const RecordObject& record_obj) {}
virtual bool PreCallValidateCmdSetViewportWScalingNV(VkCommandBuffer commandBuffer, uint32_t firstViewport, uint32_t viewportCount,
                                                     const VkViewportWScalingNV* pViewportWScalings, ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordCmdSetViewportWScalingNV(VkCommandBuffer commandBuffer, uint32_t firstViewport, uint32_t viewportCount,
                                                   const VkViewportWScalingNV* pViewportWScalings, const RecordObject& record_obj) {
}
virtual void PostCallRecordCmdSetViewportWScalingNV(VkCommandBuffer commandBuffer, uint32_t firstViewport, uint32_t viewportCount,
                                                    const VkViewportWScalingNV* pViewportWScalings,
                                                    const RecordObject& record_obj) {}
virtual bool PreCallValidateDisplayPowerControlEXT(VkDevice device, VkDisplayKHR display,
                                                   const VkDisplayPowerInfoEXT* pDisplayPowerInfo, ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordDisplayPowerControlEXT(VkDevice device, VkDisplayKHR display,
                                                 const VkDisplayPowerInfoEXT* pDisplayPowerInfo, const RecordObject& record_obj) {}
virtual void PostCallRecordDisplayPowerControlEXT(VkDevice device, VkDisplayKHR display,
                                                  const VkDisplayPowerInfoEXT* pDisplayPowerInfo, const RecordObject& record_obj) {}
virtual bool PreCallValidateRegisterDeviceEventEXT(VkDevice device, const VkDeviceEventInfoEXT* pDeviceEventInfo,
                                                   const VkAllocationCallbacks* pAllocator, VkFence* pFence,
                                                   ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordRegisterDeviceEventEXT(VkDevice device, const VkDeviceEventInfoEXT* pDeviceEventInfo,
                                                 const VkAllocationCallbacks* pAllocator, VkFence* pFence,
                                                 const RecordObject& record_obj) {}
virtual void PostCallRecordRegisterDeviceEventEXT(VkDevice device, const VkDeviceEventInfoEXT* pDeviceEventInfo,
                                                  const VkAllocationCallbacks* pAllocator, VkFence* pFence,
                                                  const RecordObject& record_obj) {}
virtual bool PreCallValidateRegisterDisplayEventEXT(VkDevice device, VkDisplayKHR display,
                                                    const VkDisplayEventInfoEXT* pDisplayEventInfo,
                                                    const VkAllocationCallbacks* pAllocator, VkFence* pFence,
                                                    ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordRegisterDisplayEventEXT(VkDevice device, VkDisplayKHR display,
                                                  const VkDisplayEventInfoEXT* pDisplayEventInfo,
                                                  const VkAllocationCallbacks* pAllocator, VkFence* pFence,
                                                  const RecordObject& record_obj) {}
virtual void PostCallRecordRegisterDisplayEventEXT(VkDevice device, VkDisplayKHR display,
                                                   const VkDisplayEventInfoEXT* pDisplayEventInfo,
                                                   const VkAllocationCallbacks* pAllocator, VkFence* pFence,
                                                   const RecordObject& record_obj) {}
virtual bool PreCallValidateGetSwapchainCounterEXT(VkDevice device, VkSwapchainKHR swapchain, VkSurfaceCounterFlagBitsEXT counter,
                                                   uint64_t* pCounterValue, ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordGetSwapchainCounterEXT(VkDevice device, VkSwapchainKHR swapchain, VkSurfaceCounterFlagBitsEXT counter,
                                                 uint64_t* pCounterValue, const RecordObject& record_obj) {}
virtual void PostCallRecordGetSwapchainCounterEXT(VkDevice device, VkSwapchainKHR swapchain, VkSurfaceCounterFlagBitsEXT counter,
                                                  uint64_t* pCounterValue, const RecordObject& record_obj) {}
virtual bool PreCallValidateGetRefreshCycleDurationGOOGLE(VkDevice device, VkSwapchainKHR swapchain,
                                                          VkRefreshCycleDurationGOOGLE* pDisplayTimingProperties,
                                                          ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordGetRefreshCycleDurationGOOGLE(VkDevice device, VkSwapchainKHR swapchain,
                                                        VkRefreshCycleDurationGOOGLE* pDisplayTimingProperties,
                                                        const RecordObject& record_obj) {}
virtual void PostCallRecordGetRefreshCycleDurationGOOGLE(VkDevice device, VkSwapchainKHR swapchain,
                                                         VkRefreshCycleDurationGOOGLE* pDisplayTimingProperties,
                                                         const RecordObject& record_obj) {}
virtual bool PreCallValidateGetPastPresentationTimingGOOGLE(VkDevice device, VkSwapchainKHR swapchain,
                                                            uint32_t* pPresentationTimingCount,
                                                            VkPastPresentationTimingGOOGLE* pPresentationTimings,
                                                            ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordGetPastPresentationTimingGOOGLE(VkDevice device, VkSwapchainKHR swapchain,
                                                          uint32_t* pPresentationTimingCount,
                                                          VkPastPresentationTimingGOOGLE* pPresentationTimings,
                                                          const RecordObject& record_obj) {}
virtual void PostCallRecordGetPastPresentationTimingGOOGLE(VkDevice device, VkSwapchainKHR swapchain,
                                                           uint32_t* pPresentationTimingCount,
                                                           VkPastPresentationTimingGOOGLE* pPresentationTimings,
                                                           const RecordObject& record_obj) {}
virtual bool PreCallValidateCmdSetDiscardRectangleEXT(VkCommandBuffer commandBuffer, uint32_t firstDiscardRectangle,
                                                      uint32_t discardRectangleCount, const VkRect2D* pDiscardRectangles,
                                                      ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordCmdSetDiscardRectangleEXT(VkCommandBuffer commandBuffer, uint32_t firstDiscardRectangle,
                                                    uint32_t discardRectangleCount, const VkRect2D* pDiscardRectangles,
                                                    const RecordObject& record_obj) {}
virtual void PostCallRecordCmdSetDiscardRectangleEXT(VkCommandBuffer commandBuffer, uint32_t firstDiscardRectangle,
                                                     uint32_t discardRectangleCount, const VkRect2D* pDiscardRectangles,
                                                     const RecordObject& record_obj) {}
virtual bool PreCallValidateCmdSetDiscardRectangleEnableEXT(VkCommandBuffer commandBuffer, VkBool32 discardRectangleEnable,
                                                            ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordCmdSetDiscardRectangleEnableEXT(VkCommandBuffer commandBuffer, VkBool32 discardRectangleEnable,
                                                          const RecordObject& record_obj) {}
virtual void PostCallRecordCmdSetDiscardRectangleEnableEXT(VkCommandBuffer commandBuffer, VkBool32 discardRectangleEnable,
                                                           const RecordObject& record_obj) {}
virtual bool PreCallValidateCmdSetDiscardRectangleModeEXT(VkCommandBuffer commandBuffer,
                                                          VkDiscardRectangleModeEXT discardRectangleMode,
                                                          ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordCmdSetDiscardRectangleModeEXT(VkCommandBuffer commandBuffer,
                                                        VkDiscardRectangleModeEXT discardRectangleMode,
                                                        const RecordObject& record_obj) {}
virtual void PostCallRecordCmdSetDiscardRectangleModeEXT(VkCommandBuffer commandBuffer,
                                                         VkDiscardRectangleModeEXT discardRectangleMode,
                                                         const RecordObject& record_obj) {}
virtual bool PreCallValidateSetHdrMetadataEXT(VkDevice device, uint32_t swapchainCount, const VkSwapchainKHR* pSwapchains,
                                              const VkHdrMetadataEXT* pMetadata, ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordSetHdrMetadataEXT(VkDevice device, uint32_t swapchainCount, const VkSwapchainKHR* pSwapchains,
                                            const VkHdrMetadataEXT* pMetadata, const RecordObject& record_obj) {}
virtual void PostCallRecordSetHdrMetadataEXT(VkDevice device, uint32_t swapchainCount, const VkSwapchainKHR* pSwapchains,
                                             const VkHdrMetadataEXT* pMetadata, const RecordObject& record_obj) {}
virtual bool PreCallValidateSetDebugUtilsObjectNameEXT(VkDevice device, const VkDebugUtilsObjectNameInfoEXT* pNameInfo,
                                                       ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordSetDebugUtilsObjectNameEXT(VkDevice device, const VkDebugUtilsObjectNameInfoEXT* pNameInfo,
                                                     const RecordObject& record_obj) {}
virtual void PostCallRecordSetDebugUtilsObjectNameEXT(VkDevice device, const VkDebugUtilsObjectNameInfoEXT* pNameInfo,
                                                      const RecordObject& record_obj) {}
virtual bool PreCallValidateSetDebugUtilsObjectTagEXT(VkDevice device, const VkDebugUtilsObjectTagInfoEXT* pTagInfo,
                                                      ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordSetDebugUtilsObjectTagEXT(VkDevice device, const VkDebugUtilsObjectTagInfoEXT* pTagInfo,
                                                    const RecordObject& record_obj) {}
virtual void PostCallRecordSetDebugUtilsObjectTagEXT(VkDevice device, const VkDebugUtilsObjectTagInfoEXT* pTagInfo,
                                                     const RecordObject& record_obj) {}
virtual bool PreCallValidateQueueBeginDebugUtilsLabelEXT(VkQueue queue, const VkDebugUtilsLabelEXT* pLabelInfo,
                                                         ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordQueueBeginDebugUtilsLabelEXT(VkQueue queue, const VkDebugUtilsLabelEXT* pLabelInfo,
                                                       const RecordObject& record_obj) {}
virtual void PostCallRecordQueueBeginDebugUtilsLabelEXT(VkQueue queue, const VkDebugUtilsLabelEXT* pLabelInfo,
                                                        const RecordObject& record_obj) {}
virtual bool PreCallValidateQueueEndDebugUtilsLabelEXT(VkQueue queue, ErrorObject& error_obj) const { return false; }
virtual void PreCallRecordQueueEndDebugUtilsLabelEXT(VkQueue queue, const RecordObject& record_obj) {}
virtual void PostCallRecordQueueEndDebugUtilsLabelEXT(VkQueue queue, const RecordObject& record_obj) {}
virtual bool PreCallValidateQueueInsertDebugUtilsLabelEXT(VkQueue queue, const VkDebugUtilsLabelEXT* pLabelInfo,
                                                          ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordQueueInsertDebugUtilsLabelEXT(VkQueue queue, const VkDebugUtilsLabelEXT* pLabelInfo,
                                                        const RecordObject& record_obj) {}
virtual void PostCallRecordQueueInsertDebugUtilsLabelEXT(VkQueue queue, const VkDebugUtilsLabelEXT* pLabelInfo,
                                                         const RecordObject& record_obj) {}
virtual bool PreCallValidateCmdBeginDebugUtilsLabelEXT(VkCommandBuffer commandBuffer, const VkDebugUtilsLabelEXT* pLabelInfo,
                                                       ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordCmdBeginDebugUtilsLabelEXT(VkCommandBuffer commandBuffer, const VkDebugUtilsLabelEXT* pLabelInfo,
                                                     const RecordObject& record_obj) {}
virtual void PostCallRecordCmdBeginDebugUtilsLabelEXT(VkCommandBuffer commandBuffer, const VkDebugUtilsLabelEXT* pLabelInfo,
                                                      const RecordObject& record_obj) {}
virtual bool PreCallValidateCmdEndDebugUtilsLabelEXT(VkCommandBuffer commandBuffer, ErrorObject& error_obj) const { return false; }
virtual void PreCallRecordCmdEndDebugUtilsLabelEXT(VkCommandBuffer commandBuffer, const RecordObject& record_obj) {}
virtual void PostCallRecordCmdEndDebugUtilsLabelEXT(VkCommandBuffer commandBuffer, const RecordObject& record_obj) {}
virtual bool PreCallValidateCmdInsertDebugUtilsLabelEXT(VkCommandBuffer commandBuffer, const VkDebugUtilsLabelEXT* pLabelInfo,
                                                        ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordCmdInsertDebugUtilsLabelEXT(VkCommandBuffer commandBuffer, const VkDebugUtilsLabelEXT* pLabelInfo,
                                                      const RecordObject& record_obj) {}
virtual void PostCallRecordCmdInsertDebugUtilsLabelEXT(VkCommandBuffer commandBuffer, const VkDebugUtilsLabelEXT* pLabelInfo,
                                                       const RecordObject& record_obj) {}
#ifdef VK_USE_PLATFORM_ANDROID_KHR
virtual bool PreCallValidateGetAndroidHardwareBufferPropertiesANDROID(VkDevice device, const struct AHardwareBuffer* buffer,
                                                                      VkAndroidHardwareBufferPropertiesANDROID* pProperties,
                                                                      ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordGetAndroidHardwareBufferPropertiesANDROID(VkDevice device, const struct AHardwareBuffer* buffer,
                                                                    VkAndroidHardwareBufferPropertiesANDROID* pProperties,
                                                                    const RecordObject& record_obj) {}
virtual void PostCallRecordGetAndroidHardwareBufferPropertiesANDROID(VkDevice device, const struct AHardwareBuffer* buffer,
                                                                     VkAndroidHardwareBufferPropertiesANDROID* pProperties,
                                                                     const RecordObject& record_obj) {}
virtual bool PreCallValidateGetMemoryAndroidHardwareBufferANDROID(VkDevice device,
                                                                  const VkMemoryGetAndroidHardwareBufferInfoANDROID* pInfo,
                                                                  struct AHardwareBuffer** pBuffer, ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordGetMemoryAndroidHardwareBufferANDROID(VkDevice device,
                                                                const VkMemoryGetAndroidHardwareBufferInfoANDROID* pInfo,
                                                                struct AHardwareBuffer** pBuffer, const RecordObject& record_obj) {}
virtual void PostCallRecordGetMemoryAndroidHardwareBufferANDROID(VkDevice device,
                                                                 const VkMemoryGetAndroidHardwareBufferInfoANDROID* pInfo,
                                                                 struct AHardwareBuffer** pBuffer, const RecordObject& record_obj) {
}
#endif  // VK_USE_PLATFORM_ANDROID_KHR
#ifdef VK_ENABLE_BETA_EXTENSIONS
virtual bool PreCallValidateCreateExecutionGraphPipelinesAMDX(VkDevice device, VkPipelineCache pipelineCache,
                                                              uint32_t createInfoCount,
                                                              const VkExecutionGraphPipelineCreateInfoAMDX* pCreateInfos,
                                                              const VkAllocationCallbacks* pAllocator, VkPipeline* pPipelines,
                                                              ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordCreateExecutionGraphPipelinesAMDX(VkDevice device, VkPipelineCache pipelineCache,
                                                            uint32_t createInfoCount,
                                                            const VkExecutionGraphPipelineCreateInfoAMDX* pCreateInfos,
                                                            const VkAllocationCallbacks* pAllocator, VkPipeline* pPipelines,
                                                            const RecordObject& record_obj) {}
virtual void PostCallRecordCreateExecutionGraphPipelinesAMDX(VkDevice device, VkPipelineCache pipelineCache,
                                                             uint32_t createInfoCount,
                                                             const VkExecutionGraphPipelineCreateInfoAMDX* pCreateInfos,
                                                             const VkAllocationCallbacks* pAllocator, VkPipeline* pPipelines,
                                                             const RecordObject& record_obj) {}
virtual bool PreCallValidateGetExecutionGraphPipelineScratchSizeAMDX(VkDevice device, VkPipeline executionGraph,
                                                                     VkExecutionGraphPipelineScratchSizeAMDX* pSizeInfo,
                                                                     ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordGetExecutionGraphPipelineScratchSizeAMDX(VkDevice device, VkPipeline executionGraph,
                                                                   VkExecutionGraphPipelineScratchSizeAMDX* pSizeInfo,
                                                                   const RecordObject& record_obj) {}
virtual void PostCallRecordGetExecutionGraphPipelineScratchSizeAMDX(VkDevice device, VkPipeline executionGraph,
                                                                    VkExecutionGraphPipelineScratchSizeAMDX* pSizeInfo,
                                                                    const RecordObject& record_obj) {}
virtual bool PreCallValidateGetExecutionGraphPipelineNodeIndexAMDX(VkDevice device, VkPipeline executionGraph,
                                                                   const VkPipelineShaderStageNodeCreateInfoAMDX* pNodeInfo,
                                                                   uint32_t* pNodeIndex, ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordGetExecutionGraphPipelineNodeIndexAMDX(VkDevice device, VkPipeline executionGraph,
                                                                 const VkPipelineShaderStageNodeCreateInfoAMDX* pNodeInfo,
                                                                 uint32_t* pNodeIndex, const RecordObject& record_obj) {}
virtual void PostCallRecordGetExecutionGraphPipelineNodeIndexAMDX(VkDevice device, VkPipeline executionGraph,
                                                                  const VkPipelineShaderStageNodeCreateInfoAMDX* pNodeInfo,
                                                                  uint32_t* pNodeIndex, const RecordObject& record_obj) {}
virtual bool PreCallValidateCmdInitializeGraphScratchMemoryAMDX(VkCommandBuffer commandBuffer, VkPipeline executionGraph,
                                                                VkDeviceAddress scratch, VkDeviceSize scratchSize,
                                                                ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordCmdInitializeGraphScratchMemoryAMDX(VkCommandBuffer commandBuffer, VkPipeline executionGraph,
                                                              VkDeviceAddress scratch, VkDeviceSize scratchSize,
                                                              const RecordObject& record_obj) {}
virtual void PostCallRecordCmdInitializeGraphScratchMemoryAMDX(VkCommandBuffer commandBuffer, VkPipeline executionGraph,
                                                               VkDeviceAddress scratch, VkDeviceSize scratchSize,
                                                               const RecordObject& record_obj) {}
virtual bool PreCallValidateCmdDispatchGraphAMDX(VkCommandBuffer commandBuffer, VkDeviceAddress scratch, VkDeviceSize scratchSize,
                                                 const VkDispatchGraphCountInfoAMDX* pCountInfo, ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordCmdDispatchGraphAMDX(VkCommandBuffer commandBuffer, VkDeviceAddress scratch, VkDeviceSize scratchSize,
                                               const VkDispatchGraphCountInfoAMDX* pCountInfo, const RecordObject& record_obj) {}
virtual void PostCallRecordCmdDispatchGraphAMDX(VkCommandBuffer commandBuffer, VkDeviceAddress scratch, VkDeviceSize scratchSize,
                                                const VkDispatchGraphCountInfoAMDX* pCountInfo, const RecordObject& record_obj) {}
virtual bool PreCallValidateCmdDispatchGraphIndirectAMDX(VkCommandBuffer commandBuffer, VkDeviceAddress scratch,
                                                         VkDeviceSize scratchSize, const VkDispatchGraphCountInfoAMDX* pCountInfo,
                                                         ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordCmdDispatchGraphIndirectAMDX(VkCommandBuffer commandBuffer, VkDeviceAddress scratch,
                                                       VkDeviceSize scratchSize, const VkDispatchGraphCountInfoAMDX* pCountInfo,
                                                       const RecordObject& record_obj) {}
virtual void PostCallRecordCmdDispatchGraphIndirectAMDX(VkCommandBuffer commandBuffer, VkDeviceAddress scratch,
                                                        VkDeviceSize scratchSize, const VkDispatchGraphCountInfoAMDX* pCountInfo,
                                                        const RecordObject& record_obj) {}
virtual bool PreCallValidateCmdDispatchGraphIndirectCountAMDX(VkCommandBuffer commandBuffer, VkDeviceAddress scratch,
                                                              VkDeviceSize scratchSize, VkDeviceAddress countInfo,
                                                              ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordCmdDispatchGraphIndirectCountAMDX(VkCommandBuffer commandBuffer, VkDeviceAddress scratch,
                                                            VkDeviceSize scratchSize, VkDeviceAddress countInfo,
                                                            const RecordObject& record_obj) {}
virtual void PostCallRecordCmdDispatchGraphIndirectCountAMDX(VkCommandBuffer commandBuffer, VkDeviceAddress scratch,
                                                             VkDeviceSize scratchSize, VkDeviceAddress countInfo,
                                                             const RecordObject& record_obj) {}
#endif  // VK_ENABLE_BETA_EXTENSIONS
virtual bool PreCallValidateCmdSetSampleLocationsEXT(VkCommandBuffer commandBuffer,
                                                     const VkSampleLocationsInfoEXT* pSampleLocationsInfo,
                                                     ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordCmdSetSampleLocationsEXT(VkCommandBuffer commandBuffer,
                                                   const VkSampleLocationsInfoEXT* pSampleLocationsInfo,
                                                   const RecordObject& record_obj) {}
virtual void PostCallRecordCmdSetSampleLocationsEXT(VkCommandBuffer commandBuffer,
                                                    const VkSampleLocationsInfoEXT* pSampleLocationsInfo,
                                                    const RecordObject& record_obj) {}
virtual bool PreCallValidateGetImageDrmFormatModifierPropertiesEXT(VkDevice device, VkImage image,
                                                                   VkImageDrmFormatModifierPropertiesEXT* pProperties,
                                                                   ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordGetImageDrmFormatModifierPropertiesEXT(VkDevice device, VkImage image,
                                                                 VkImageDrmFormatModifierPropertiesEXT* pProperties,
                                                                 const RecordObject& record_obj) {}
virtual void PostCallRecordGetImageDrmFormatModifierPropertiesEXT(VkDevice device, VkImage image,
                                                                  VkImageDrmFormatModifierPropertiesEXT* pProperties,
                                                                  const RecordObject& record_obj) {}
virtual bool PreCallValidateCmdBindShadingRateImageNV(VkCommandBuffer commandBuffer, VkImageView imageView,
                                                      VkImageLayout imageLayout, ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordCmdBindShadingRateImageNV(VkCommandBuffer commandBuffer, VkImageView imageView, VkImageLayout imageLayout,
                                                    const RecordObject& record_obj) {}
virtual void PostCallRecordCmdBindShadingRateImageNV(VkCommandBuffer commandBuffer, VkImageView imageView,
                                                     VkImageLayout imageLayout, const RecordObject& record_obj) {}
virtual bool PreCallValidateCmdSetViewportShadingRatePaletteNV(VkCommandBuffer commandBuffer, uint32_t firstViewport,
                                                               uint32_t viewportCount,
                                                               const VkShadingRatePaletteNV* pShadingRatePalettes,
                                                               ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordCmdSetViewportShadingRatePaletteNV(VkCommandBuffer commandBuffer, uint32_t firstViewport,
                                                             uint32_t viewportCount,
                                                             const VkShadingRatePaletteNV* pShadingRatePalettes,
                                                             const RecordObject& record_obj) {}
virtual void PostCallRecordCmdSetViewportShadingRatePaletteNV(VkCommandBuffer commandBuffer, uint32_t firstViewport,
                                                              uint32_t viewportCount,
                                                              const VkShadingRatePaletteNV* pShadingRatePalettes,
                                                              const RecordObject& record_obj) {}
virtual bool PreCallValidateCmdSetCoarseSampleOrderNV(VkCommandBuffer commandBuffer, VkCoarseSampleOrderTypeNV sampleOrderType,
                                                      uint32_t customSampleOrderCount,
                                                      const VkCoarseSampleOrderCustomNV* pCustomSampleOrders,
                                                      ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordCmdSetCoarseSampleOrderNV(VkCommandBuffer commandBuffer, VkCoarseSampleOrderTypeNV sampleOrderType,
                                                    uint32_t customSampleOrderCount,
                                                    const VkCoarseSampleOrderCustomNV* pCustomSampleOrders,
                                                    const RecordObject& record_obj) {}
virtual void PostCallRecordCmdSetCoarseSampleOrderNV(VkCommandBuffer commandBuffer, VkCoarseSampleOrderTypeNV sampleOrderType,
                                                     uint32_t customSampleOrderCount,
                                                     const VkCoarseSampleOrderCustomNV* pCustomSampleOrders,
                                                     const RecordObject& record_obj) {}
virtual bool PreCallValidateCreateAccelerationStructureNV(VkDevice device, const VkAccelerationStructureCreateInfoNV* pCreateInfo,
                                                          const VkAllocationCallbacks* pAllocator,
                                                          VkAccelerationStructureNV* pAccelerationStructure,
                                                          ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordCreateAccelerationStructureNV(VkDevice device, const VkAccelerationStructureCreateInfoNV* pCreateInfo,
                                                        const VkAllocationCallbacks* pAllocator,
                                                        VkAccelerationStructureNV* pAccelerationStructure,
                                                        const RecordObject& record_obj) {}
virtual void PostCallRecordCreateAccelerationStructureNV(VkDevice device, const VkAccelerationStructureCreateInfoNV* pCreateInfo,
                                                         const VkAllocationCallbacks* pAllocator,
                                                         VkAccelerationStructureNV* pAccelerationStructure,
                                                         const RecordObject& record_obj) {}
virtual bool PreCallValidateDestroyAccelerationStructureNV(VkDevice device, VkAccelerationStructureNV accelerationStructure,
                                                           const VkAllocationCallbacks* pAllocator, ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordDestroyAccelerationStructureNV(VkDevice device, VkAccelerationStructureNV accelerationStructure,
                                                         const VkAllocationCallbacks* pAllocator, const RecordObject& record_obj) {}
virtual void PostCallRecordDestroyAccelerationStructureNV(VkDevice device, VkAccelerationStructureNV accelerationStructure,
                                                          const VkAllocationCallbacks* pAllocator, const RecordObject& record_obj) {
}
virtual bool PreCallValidateGetAccelerationStructureMemoryRequirementsNV(
    VkDevice device, const VkAccelerationStructureMemoryRequirementsInfoNV* pInfo, VkMemoryRequirements2KHR* pMemoryRequirements,
    ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordGetAccelerationStructureMemoryRequirementsNV(VkDevice device,
                                                                       const VkAccelerationStructureMemoryRequirementsInfoNV* pInfo,
                                                                       VkMemoryRequirements2KHR* pMemoryRequirements,
                                                                       const RecordObject& record_obj) {}
virtual void PostCallRecordGetAccelerationStructureMemoryRequirementsNV(
    VkDevice device, const VkAccelerationStructureMemoryRequirementsInfoNV* pInfo, VkMemoryRequirements2KHR* pMemoryRequirements,
    const RecordObject& record_obj) {}
virtual bool PreCallValidateBindAccelerationStructureMemoryNV(VkDevice device, uint32_t bindInfoCount,
                                                              const VkBindAccelerationStructureMemoryInfoNV* pBindInfos,
                                                              ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordBindAccelerationStructureMemoryNV(VkDevice device, uint32_t bindInfoCount,
                                                            const VkBindAccelerationStructureMemoryInfoNV* pBindInfos,
                                                            const RecordObject& record_obj) {}
virtual void PostCallRecordBindAccelerationStructureMemoryNV(VkDevice device, uint32_t bindInfoCount,
                                                             const VkBindAccelerationStructureMemoryInfoNV* pBindInfos,
                                                             const RecordObject& record_obj) {}
virtual bool PreCallValidateCmdBuildAccelerationStructureNV(VkCommandBuffer commandBuffer,
                                                            const VkAccelerationStructureInfoNV* pInfo, VkBuffer instanceData,
                                                            VkDeviceSize instanceOffset, VkBool32 update,
                                                            VkAccelerationStructureNV dst, VkAccelerationStructureNV src,
                                                            VkBuffer scratch, VkDeviceSize scratchOffset,
                                                            ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordCmdBuildAccelerationStructureNV(VkCommandBuffer commandBuffer, const VkAccelerationStructureInfoNV* pInfo,
                                                          VkBuffer instanceData, VkDeviceSize instanceOffset, VkBool32 update,
                                                          VkAccelerationStructureNV dst, VkAccelerationStructureNV src,
                                                          VkBuffer scratch, VkDeviceSize scratchOffset,
                                                          const RecordObject& record_obj) {}
virtual void PostCallRecordCmdBuildAccelerationStructureNV(VkCommandBuffer commandBuffer,
                                                           const VkAccelerationStructureInfoNV* pInfo, VkBuffer instanceData,
                                                           VkDeviceSize instanceOffset, VkBool32 update,
                                                           VkAccelerationStructureNV dst, VkAccelerationStructureNV src,
                                                           VkBuffer scratch, VkDeviceSize scratchOffset,
                                                           const RecordObject& record_obj) {}
virtual bool PreCallValidateCmdCopyAccelerationStructureNV(VkCommandBuffer commandBuffer, VkAccelerationStructureNV dst,
                                                           VkAccelerationStructureNV src, VkCopyAccelerationStructureModeKHR mode,
                                                           ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordCmdCopyAccelerationStructureNV(VkCommandBuffer commandBuffer, VkAccelerationStructureNV dst,
                                                         VkAccelerationStructureNV src, VkCopyAccelerationStructureModeKHR mode,
                                                         const RecordObject& record_obj) {}
virtual void PostCallRecordCmdCopyAccelerationStructureNV(VkCommandBuffer commandBuffer, VkAccelerationStructureNV dst,
                                                          VkAccelerationStructureNV src, VkCopyAccelerationStructureModeKHR mode,
                                                          const RecordObject& record_obj) {}
virtual bool PreCallValidateCmdTraceRaysNV(VkCommandBuffer commandBuffer, VkBuffer raygenShaderBindingTableBuffer,
                                           VkDeviceSize raygenShaderBindingOffset, VkBuffer missShaderBindingTableBuffer,
                                           VkDeviceSize missShaderBindingOffset, VkDeviceSize missShaderBindingStride,
                                           VkBuffer hitShaderBindingTableBuffer, VkDeviceSize hitShaderBindingOffset,
                                           VkDeviceSize hitShaderBindingStride, VkBuffer callableShaderBindingTableBuffer,
                                           VkDeviceSize callableShaderBindingOffset, VkDeviceSize callableShaderBindingStride,
                                           uint32_t width, uint32_t height, uint32_t depth, ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordCmdTraceRaysNV(VkCommandBuffer commandBuffer, VkBuffer raygenShaderBindingTableBuffer,
                                         VkDeviceSize raygenShaderBindingOffset, VkBuffer missShaderBindingTableBuffer,
                                         VkDeviceSize missShaderBindingOffset, VkDeviceSize missShaderBindingStride,
                                         VkBuffer hitShaderBindingTableBuffer, VkDeviceSize hitShaderBindingOffset,
                                         VkDeviceSize hitShaderBindingStride, VkBuffer callableShaderBindingTableBuffer,
                                         VkDeviceSize callableShaderBindingOffset, VkDeviceSize callableShaderBindingStride,
                                         uint32_t width, uint32_t height, uint32_t depth, const RecordObject& record_obj) {}
virtual void PostCallRecordCmdTraceRaysNV(VkCommandBuffer commandBuffer, VkBuffer raygenShaderBindingTableBuffer,
                                          VkDeviceSize raygenShaderBindingOffset, VkBuffer missShaderBindingTableBuffer,
                                          VkDeviceSize missShaderBindingOffset, VkDeviceSize missShaderBindingStride,
                                          VkBuffer hitShaderBindingTableBuffer, VkDeviceSize hitShaderBindingOffset,
                                          VkDeviceSize hitShaderBindingStride, VkBuffer callableShaderBindingTableBuffer,
                                          VkDeviceSize callableShaderBindingOffset, VkDeviceSize callableShaderBindingStride,
                                          uint32_t width, uint32_t height, uint32_t depth, const RecordObject& record_obj) {}
virtual bool PreCallValidateCreateRayTracingPipelinesNV(VkDevice device, VkPipelineCache pipelineCache, uint32_t createInfoCount,
                                                        const VkRayTracingPipelineCreateInfoNV* pCreateInfos,
                                                        const VkAllocationCallbacks* pAllocator, VkPipeline* pPipelines,
                                                        ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordCreateRayTracingPipelinesNV(VkDevice device, VkPipelineCache pipelineCache, uint32_t createInfoCount,
                                                      const VkRayTracingPipelineCreateInfoNV* pCreateInfos,
                                                      const VkAllocationCallbacks* pAllocator, VkPipeline* pPipelines,
                                                      const RecordObject& record_obj) {}
virtual void PostCallRecordCreateRayTracingPipelinesNV(VkDevice device, VkPipelineCache pipelineCache, uint32_t createInfoCount,
                                                       const VkRayTracingPipelineCreateInfoNV* pCreateInfos,
                                                       const VkAllocationCallbacks* pAllocator, VkPipeline* pPipelines,
                                                       const RecordObject& record_obj) {}
virtual bool PreCallValidateGetRayTracingShaderGroupHandlesKHR(VkDevice device, VkPipeline pipeline, uint32_t firstGroup,
                                                               uint32_t groupCount, size_t dataSize, void* pData,
                                                               ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordGetRayTracingShaderGroupHandlesKHR(VkDevice device, VkPipeline pipeline, uint32_t firstGroup,
                                                             uint32_t groupCount, size_t dataSize, void* pData,
                                                             const RecordObject& record_obj) {}
virtual void PostCallRecordGetRayTracingShaderGroupHandlesKHR(VkDevice device, VkPipeline pipeline, uint32_t firstGroup,
                                                              uint32_t groupCount, size_t dataSize, void* pData,
                                                              const RecordObject& record_obj) {}
virtual bool PreCallValidateGetRayTracingShaderGroupHandlesNV(VkDevice device, VkPipeline pipeline, uint32_t firstGroup,
                                                              uint32_t groupCount, size_t dataSize, void* pData,
                                                              ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordGetRayTracingShaderGroupHandlesNV(VkDevice device, VkPipeline pipeline, uint32_t firstGroup,
                                                            uint32_t groupCount, size_t dataSize, void* pData,
                                                            const RecordObject& record_obj) {}
virtual void PostCallRecordGetRayTracingShaderGroupHandlesNV(VkDevice device, VkPipeline pipeline, uint32_t firstGroup,
                                                             uint32_t groupCount, size_t dataSize, void* pData,
                                                             const RecordObject& record_obj) {}
virtual bool PreCallValidateGetAccelerationStructureHandleNV(VkDevice device, VkAccelerationStructureNV accelerationStructure,
                                                             size_t dataSize, void* pData, ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordGetAccelerationStructureHandleNV(VkDevice device, VkAccelerationStructureNV accelerationStructure,
                                                           size_t dataSize, void* pData, const RecordObject& record_obj) {}
virtual void PostCallRecordGetAccelerationStructureHandleNV(VkDevice device, VkAccelerationStructureNV accelerationStructure,
                                                            size_t dataSize, void* pData, const RecordObject& record_obj) {}
virtual bool PreCallValidateCmdWriteAccelerationStructuresPropertiesNV(VkCommandBuffer commandBuffer,
                                                                       uint32_t accelerationStructureCount,
                                                                       const VkAccelerationStructureNV* pAccelerationStructures,
                                                                       VkQueryType queryType, VkQueryPool queryPool,
                                                                       uint32_t firstQuery, ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordCmdWriteAccelerationStructuresPropertiesNV(VkCommandBuffer commandBuffer,
                                                                     uint32_t accelerationStructureCount,
                                                                     const VkAccelerationStructureNV* pAccelerationStructures,
                                                                     VkQueryType queryType, VkQueryPool queryPool,
                                                                     uint32_t firstQuery, const RecordObject& record_obj) {}
virtual void PostCallRecordCmdWriteAccelerationStructuresPropertiesNV(VkCommandBuffer commandBuffer,
                                                                      uint32_t accelerationStructureCount,
                                                                      const VkAccelerationStructureNV* pAccelerationStructures,
                                                                      VkQueryType queryType, VkQueryPool queryPool,
                                                                      uint32_t firstQuery, const RecordObject& record_obj) {}
virtual bool PreCallValidateCompileDeferredNV(VkDevice device, VkPipeline pipeline, uint32_t shader, ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordCompileDeferredNV(VkDevice device, VkPipeline pipeline, uint32_t shader, const RecordObject& record_obj) {
}
virtual void PostCallRecordCompileDeferredNV(VkDevice device, VkPipeline pipeline, uint32_t shader,
                                             const RecordObject& record_obj) {}
virtual bool PreCallValidateGetMemoryHostPointerPropertiesEXT(VkDevice device, VkExternalMemoryHandleTypeFlagBits handleType,
                                                              const void* pHostPointer,
                                                              VkMemoryHostPointerPropertiesEXT* pMemoryHostPointerProperties,
                                                              ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordGetMemoryHostPointerPropertiesEXT(VkDevice device, VkExternalMemoryHandleTypeFlagBits handleType,
                                                            const void* pHostPointer,
                                                            VkMemoryHostPointerPropertiesEXT* pMemoryHostPointerProperties,
                                                            const RecordObject& record_obj) {}
virtual void PostCallRecordGetMemoryHostPointerPropertiesEXT(VkDevice device, VkExternalMemoryHandleTypeFlagBits handleType,
                                                             const void* pHostPointer,
                                                             VkMemoryHostPointerPropertiesEXT* pMemoryHostPointerProperties,
                                                             const RecordObject& record_obj) {}
virtual bool PreCallValidateCmdWriteBufferMarkerAMD(VkCommandBuffer commandBuffer, VkPipelineStageFlagBits pipelineStage,
                                                    VkBuffer dstBuffer, VkDeviceSize dstOffset, uint32_t marker,
                                                    ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordCmdWriteBufferMarkerAMD(VkCommandBuffer commandBuffer, VkPipelineStageFlagBits pipelineStage,
                                                  VkBuffer dstBuffer, VkDeviceSize dstOffset, uint32_t marker,
                                                  const RecordObject& record_obj) {}
virtual void PostCallRecordCmdWriteBufferMarkerAMD(VkCommandBuffer commandBuffer, VkPipelineStageFlagBits pipelineStage,
                                                   VkBuffer dstBuffer, VkDeviceSize dstOffset, uint32_t marker,
                                                   const RecordObject& record_obj) {}
virtual bool PreCallValidateCmdWriteBufferMarker2AMD(VkCommandBuffer commandBuffer, VkPipelineStageFlags2 stage, VkBuffer dstBuffer,
                                                     VkDeviceSize dstOffset, uint32_t marker, ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordCmdWriteBufferMarker2AMD(VkCommandBuffer commandBuffer, VkPipelineStageFlags2 stage, VkBuffer dstBuffer,
                                                   VkDeviceSize dstOffset, uint32_t marker, const RecordObject& record_obj) {}
virtual void PostCallRecordCmdWriteBufferMarker2AMD(VkCommandBuffer commandBuffer, VkPipelineStageFlags2 stage, VkBuffer dstBuffer,
                                                    VkDeviceSize dstOffset, uint32_t marker, const RecordObject& record_obj) {}
virtual bool PreCallValidateGetCalibratedTimestampsEXT(VkDevice device, uint32_t timestampCount,
                                                       const VkCalibratedTimestampInfoKHR* pTimestampInfos, uint64_t* pTimestamps,
                                                       uint64_t* pMaxDeviation, ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordGetCalibratedTimestampsEXT(VkDevice device, uint32_t timestampCount,
                                                     const VkCalibratedTimestampInfoKHR* pTimestampInfos, uint64_t* pTimestamps,
                                                     uint64_t* pMaxDeviation, const RecordObject& record_obj) {}
virtual void PostCallRecordGetCalibratedTimestampsEXT(VkDevice device, uint32_t timestampCount,
                                                      const VkCalibratedTimestampInfoKHR* pTimestampInfos, uint64_t* pTimestamps,
                                                      uint64_t* pMaxDeviation, const RecordObject& record_obj) {}
virtual bool PreCallValidateCmdDrawMeshTasksNV(VkCommandBuffer commandBuffer, uint32_t taskCount, uint32_t firstTask,
                                               ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordCmdDrawMeshTasksNV(VkCommandBuffer commandBuffer, uint32_t taskCount, uint32_t firstTask,
                                             const RecordObject& record_obj) {}
virtual void PostCallRecordCmdDrawMeshTasksNV(VkCommandBuffer commandBuffer, uint32_t taskCount, uint32_t firstTask,
                                              const RecordObject& record_obj) {}
virtual bool PreCallValidateCmdDrawMeshTasksIndirectNV(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                       uint32_t drawCount, uint32_t stride, ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordCmdDrawMeshTasksIndirectNV(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                     uint32_t drawCount, uint32_t stride, const RecordObject& record_obj) {}
virtual void PostCallRecordCmdDrawMeshTasksIndirectNV(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                      uint32_t drawCount, uint32_t stride, const RecordObject& record_obj) {}
virtual bool PreCallValidateCmdDrawMeshTasksIndirectCountNV(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                            VkBuffer countBuffer, VkDeviceSize countBufferOffset,
                                                            uint32_t maxDrawCount, uint32_t stride, ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordCmdDrawMeshTasksIndirectCountNV(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                          VkBuffer countBuffer, VkDeviceSize countBufferOffset,
                                                          uint32_t maxDrawCount, uint32_t stride, const RecordObject& record_obj) {}
virtual void PostCallRecordCmdDrawMeshTasksIndirectCountNV(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                           VkBuffer countBuffer, VkDeviceSize countBufferOffset,
                                                           uint32_t maxDrawCount, uint32_t stride, const RecordObject& record_obj) {
}
virtual bool PreCallValidateCmdSetExclusiveScissorEnableNV(VkCommandBuffer commandBuffer, uint32_t firstExclusiveScissor,
                                                           uint32_t exclusiveScissorCount, const VkBool32* pExclusiveScissorEnables,
                                                           ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordCmdSetExclusiveScissorEnableNV(VkCommandBuffer commandBuffer, uint32_t firstExclusiveScissor,
                                                         uint32_t exclusiveScissorCount, const VkBool32* pExclusiveScissorEnables,
                                                         const RecordObject& record_obj) {}
virtual void PostCallRecordCmdSetExclusiveScissorEnableNV(VkCommandBuffer commandBuffer, uint32_t firstExclusiveScissor,
                                                          uint32_t exclusiveScissorCount, const VkBool32* pExclusiveScissorEnables,
                                                          const RecordObject& record_obj) {}
virtual bool PreCallValidateCmdSetExclusiveScissorNV(VkCommandBuffer commandBuffer, uint32_t firstExclusiveScissor,
                                                     uint32_t exclusiveScissorCount, const VkRect2D* pExclusiveScissors,
                                                     ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordCmdSetExclusiveScissorNV(VkCommandBuffer commandBuffer, uint32_t firstExclusiveScissor,
                                                   uint32_t exclusiveScissorCount, const VkRect2D* pExclusiveScissors,
                                                   const RecordObject& record_obj) {}
virtual void PostCallRecordCmdSetExclusiveScissorNV(VkCommandBuffer commandBuffer, uint32_t firstExclusiveScissor,
                                                    uint32_t exclusiveScissorCount, const VkRect2D* pExclusiveScissors,
                                                    const RecordObject& record_obj) {}
virtual bool PreCallValidateCmdSetCheckpointNV(VkCommandBuffer commandBuffer, const void* pCheckpointMarker,
                                               ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordCmdSetCheckpointNV(VkCommandBuffer commandBuffer, const void* pCheckpointMarker,
                                             const RecordObject& record_obj) {}
virtual void PostCallRecordCmdSetCheckpointNV(VkCommandBuffer commandBuffer, const void* pCheckpointMarker,
                                              const RecordObject& record_obj) {}
virtual bool PreCallValidateGetQueueCheckpointDataNV(VkQueue queue, uint32_t* pCheckpointDataCount,
                                                     VkCheckpointDataNV* pCheckpointData, ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordGetQueueCheckpointDataNV(VkQueue queue, uint32_t* pCheckpointDataCount,
                                                   VkCheckpointDataNV* pCheckpointData, const RecordObject& record_obj) {}
virtual void PostCallRecordGetQueueCheckpointDataNV(VkQueue queue, uint32_t* pCheckpointDataCount,
                                                    VkCheckpointDataNV* pCheckpointData, const RecordObject& record_obj) {}
virtual bool PreCallValidateGetQueueCheckpointData2NV(VkQueue queue, uint32_t* pCheckpointDataCount,
                                                      VkCheckpointData2NV* pCheckpointData, ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordGetQueueCheckpointData2NV(VkQueue queue, uint32_t* pCheckpointDataCount,
                                                    VkCheckpointData2NV* pCheckpointData, const RecordObject& record_obj) {}
virtual void PostCallRecordGetQueueCheckpointData2NV(VkQueue queue, uint32_t* pCheckpointDataCount,
                                                     VkCheckpointData2NV* pCheckpointData, const RecordObject& record_obj) {}
virtual bool PreCallValidateInitializePerformanceApiINTEL(VkDevice device,
                                                          const VkInitializePerformanceApiInfoINTEL* pInitializeInfo,
                                                          ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordInitializePerformanceApiINTEL(VkDevice device, const VkInitializePerformanceApiInfoINTEL* pInitializeInfo,
                                                        const RecordObject& record_obj) {}
virtual void PostCallRecordInitializePerformanceApiINTEL(VkDevice device,
                                                         const VkInitializePerformanceApiInfoINTEL* pInitializeInfo,
                                                         const RecordObject& record_obj) {}
virtual bool PreCallValidateUninitializePerformanceApiINTEL(VkDevice device, ErrorObject& error_obj) const { return false; }
virtual void PreCallRecordUninitializePerformanceApiINTEL(VkDevice device, const RecordObject& record_obj) {}
virtual void PostCallRecordUninitializePerformanceApiINTEL(VkDevice device, const RecordObject& record_obj) {}
virtual bool PreCallValidateCmdSetPerformanceMarkerINTEL(VkCommandBuffer commandBuffer,
                                                         const VkPerformanceMarkerInfoINTEL* pMarkerInfo,
                                                         ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordCmdSetPerformanceMarkerINTEL(VkCommandBuffer commandBuffer,
                                                       const VkPerformanceMarkerInfoINTEL* pMarkerInfo,
                                                       const RecordObject& record_obj) {}
virtual void PostCallRecordCmdSetPerformanceMarkerINTEL(VkCommandBuffer commandBuffer,
                                                        const VkPerformanceMarkerInfoINTEL* pMarkerInfo,
                                                        const RecordObject& record_obj) {}
virtual bool PreCallValidateCmdSetPerformanceStreamMarkerINTEL(VkCommandBuffer commandBuffer,
                                                               const VkPerformanceStreamMarkerInfoINTEL* pMarkerInfo,
                                                               ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordCmdSetPerformanceStreamMarkerINTEL(VkCommandBuffer commandBuffer,
                                                             const VkPerformanceStreamMarkerInfoINTEL* pMarkerInfo,
                                                             const RecordObject& record_obj) {}
virtual void PostCallRecordCmdSetPerformanceStreamMarkerINTEL(VkCommandBuffer commandBuffer,
                                                              const VkPerformanceStreamMarkerInfoINTEL* pMarkerInfo,
                                                              const RecordObject& record_obj) {}
virtual bool PreCallValidateCmdSetPerformanceOverrideINTEL(VkCommandBuffer commandBuffer,
                                                           const VkPerformanceOverrideInfoINTEL* pOverrideInfo,
                                                           ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordCmdSetPerformanceOverrideINTEL(VkCommandBuffer commandBuffer,
                                                         const VkPerformanceOverrideInfoINTEL* pOverrideInfo,
                                                         const RecordObject& record_obj) {}
virtual void PostCallRecordCmdSetPerformanceOverrideINTEL(VkCommandBuffer commandBuffer,
                                                          const VkPerformanceOverrideInfoINTEL* pOverrideInfo,
                                                          const RecordObject& record_obj) {}
virtual bool PreCallValidateAcquirePerformanceConfigurationINTEL(VkDevice device,
                                                                 const VkPerformanceConfigurationAcquireInfoINTEL* pAcquireInfo,
                                                                 VkPerformanceConfigurationINTEL* pConfiguration,
                                                                 ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordAcquirePerformanceConfigurationINTEL(VkDevice device,
                                                               const VkPerformanceConfigurationAcquireInfoINTEL* pAcquireInfo,
                                                               VkPerformanceConfigurationINTEL* pConfiguration,
                                                               const RecordObject& record_obj) {}
virtual void PostCallRecordAcquirePerformanceConfigurationINTEL(VkDevice device,
                                                                const VkPerformanceConfigurationAcquireInfoINTEL* pAcquireInfo,
                                                                VkPerformanceConfigurationINTEL* pConfiguration,
                                                                const RecordObject& record_obj) {}
virtual bool PreCallValidateReleasePerformanceConfigurationINTEL(VkDevice device, VkPerformanceConfigurationINTEL configuration,
                                                                 ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordReleasePerformanceConfigurationINTEL(VkDevice device, VkPerformanceConfigurationINTEL configuration,
                                                               const RecordObject& record_obj) {}
virtual void PostCallRecordReleasePerformanceConfigurationINTEL(VkDevice device, VkPerformanceConfigurationINTEL configuration,
                                                                const RecordObject& record_obj) {}
virtual bool PreCallValidateQueueSetPerformanceConfigurationINTEL(VkQueue queue, VkPerformanceConfigurationINTEL configuration,
                                                                  ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordQueueSetPerformanceConfigurationINTEL(VkQueue queue, VkPerformanceConfigurationINTEL configuration,
                                                                const RecordObject& record_obj) {}
virtual void PostCallRecordQueueSetPerformanceConfigurationINTEL(VkQueue queue, VkPerformanceConfigurationINTEL configuration,
                                                                 const RecordObject& record_obj) {}
virtual bool PreCallValidateGetPerformanceParameterINTEL(VkDevice device, VkPerformanceParameterTypeINTEL parameter,
                                                         VkPerformanceValueINTEL* pValue, ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordGetPerformanceParameterINTEL(VkDevice device, VkPerformanceParameterTypeINTEL parameter,
                                                       VkPerformanceValueINTEL* pValue, const RecordObject& record_obj) {}
virtual void PostCallRecordGetPerformanceParameterINTEL(VkDevice device, VkPerformanceParameterTypeINTEL parameter,
                                                        VkPerformanceValueINTEL* pValue, const RecordObject& record_obj) {}
virtual bool PreCallValidateSetLocalDimmingAMD(VkDevice device, VkSwapchainKHR swapChain, VkBool32 localDimmingEnable,
                                               ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordSetLocalDimmingAMD(VkDevice device, VkSwapchainKHR swapChain, VkBool32 localDimmingEnable,
                                             const RecordObject& record_obj) {}
virtual void PostCallRecordSetLocalDimmingAMD(VkDevice device, VkSwapchainKHR swapChain, VkBool32 localDimmingEnable,
                                              const RecordObject& record_obj) {}
virtual bool PreCallValidateGetBufferDeviceAddressEXT(VkDevice device, const VkBufferDeviceAddressInfo* pInfo,
                                                      ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordGetBufferDeviceAddressEXT(VkDevice device, const VkBufferDeviceAddressInfo* pInfo,
                                                    const RecordObject& record_obj) {}
virtual void PostCallRecordGetBufferDeviceAddressEXT(VkDevice device, const VkBufferDeviceAddressInfo* pInfo,
                                                     const RecordObject& record_obj) {}
#ifdef VK_USE_PLATFORM_WIN32_KHR
virtual bool PreCallValidateAcquireFullScreenExclusiveModeEXT(VkDevice device, VkSwapchainKHR swapchain,
                                                              ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordAcquireFullScreenExclusiveModeEXT(VkDevice device, VkSwapchainKHR swapchain,
                                                            const RecordObject& record_obj) {}
virtual void PostCallRecordAcquireFullScreenExclusiveModeEXT(VkDevice device, VkSwapchainKHR swapchain,
                                                             const RecordObject& record_obj) {}
virtual bool PreCallValidateReleaseFullScreenExclusiveModeEXT(VkDevice device, VkSwapchainKHR swapchain,
                                                              ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordReleaseFullScreenExclusiveModeEXT(VkDevice device, VkSwapchainKHR swapchain,
                                                            const RecordObject& record_obj) {}
virtual void PostCallRecordReleaseFullScreenExclusiveModeEXT(VkDevice device, VkSwapchainKHR swapchain,
                                                             const RecordObject& record_obj) {}
virtual bool PreCallValidateGetDeviceGroupSurfacePresentModes2EXT(VkDevice device,
                                                                  const VkPhysicalDeviceSurfaceInfo2KHR* pSurfaceInfo,
                                                                  VkDeviceGroupPresentModeFlagsKHR* pModes,
                                                                  ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordGetDeviceGroupSurfacePresentModes2EXT(VkDevice device,
                                                                const VkPhysicalDeviceSurfaceInfo2KHR* pSurfaceInfo,
                                                                VkDeviceGroupPresentModeFlagsKHR* pModes,
                                                                const RecordObject& record_obj) {}
virtual void PostCallRecordGetDeviceGroupSurfacePresentModes2EXT(VkDevice device,
                                                                 const VkPhysicalDeviceSurfaceInfo2KHR* pSurfaceInfo,
                                                                 VkDeviceGroupPresentModeFlagsKHR* pModes,
                                                                 const RecordObject& record_obj) {}
#endif  // VK_USE_PLATFORM_WIN32_KHR
virtual bool PreCallValidateCmdSetLineStippleEXT(VkCommandBuffer commandBuffer, uint32_t lineStippleFactor,
                                                 uint16_t lineStipplePattern, ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordCmdSetLineStippleEXT(VkCommandBuffer commandBuffer, uint32_t lineStippleFactor,
                                               uint16_t lineStipplePattern, const RecordObject& record_obj) {}
virtual void PostCallRecordCmdSetLineStippleEXT(VkCommandBuffer commandBuffer, uint32_t lineStippleFactor,
                                                uint16_t lineStipplePattern, const RecordObject& record_obj) {}
virtual bool PreCallValidateResetQueryPoolEXT(VkDevice device, VkQueryPool queryPool, uint32_t firstQuery, uint32_t queryCount,
                                              ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordResetQueryPoolEXT(VkDevice device, VkQueryPool queryPool, uint32_t firstQuery, uint32_t queryCount,
                                            const RecordObject& record_obj) {}
virtual void PostCallRecordResetQueryPoolEXT(VkDevice device, VkQueryPool queryPool, uint32_t firstQuery, uint32_t queryCount,
                                             const RecordObject& record_obj) {}
virtual bool PreCallValidateCmdSetCullModeEXT(VkCommandBuffer commandBuffer, VkCullModeFlags cullMode,
                                              ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordCmdSetCullModeEXT(VkCommandBuffer commandBuffer, VkCullModeFlags cullMode,
                                            const RecordObject& record_obj) {}
virtual void PostCallRecordCmdSetCullModeEXT(VkCommandBuffer commandBuffer, VkCullModeFlags cullMode,
                                             const RecordObject& record_obj) {}
virtual bool PreCallValidateCmdSetFrontFaceEXT(VkCommandBuffer commandBuffer, VkFrontFace frontFace, ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordCmdSetFrontFaceEXT(VkCommandBuffer commandBuffer, VkFrontFace frontFace, const RecordObject& record_obj) {
}
virtual void PostCallRecordCmdSetFrontFaceEXT(VkCommandBuffer commandBuffer, VkFrontFace frontFace,
                                              const RecordObject& record_obj) {}
virtual bool PreCallValidateCmdSetPrimitiveTopologyEXT(VkCommandBuffer commandBuffer, VkPrimitiveTopology primitiveTopology,
                                                       ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordCmdSetPrimitiveTopologyEXT(VkCommandBuffer commandBuffer, VkPrimitiveTopology primitiveTopology,
                                                     const RecordObject& record_obj) {}
virtual void PostCallRecordCmdSetPrimitiveTopologyEXT(VkCommandBuffer commandBuffer, VkPrimitiveTopology primitiveTopology,
                                                      const RecordObject& record_obj) {}
virtual bool PreCallValidateCmdSetViewportWithCountEXT(VkCommandBuffer commandBuffer, uint32_t viewportCount,
                                                       const VkViewport* pViewports, ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordCmdSetViewportWithCountEXT(VkCommandBuffer commandBuffer, uint32_t viewportCount,
                                                     const VkViewport* pViewports, const RecordObject& record_obj) {}
virtual void PostCallRecordCmdSetViewportWithCountEXT(VkCommandBuffer commandBuffer, uint32_t viewportCount,
                                                      const VkViewport* pViewports, const RecordObject& record_obj) {}
virtual bool PreCallValidateCmdSetScissorWithCountEXT(VkCommandBuffer commandBuffer, uint32_t scissorCount,
                                                      const VkRect2D* pScissors, ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordCmdSetScissorWithCountEXT(VkCommandBuffer commandBuffer, uint32_t scissorCount, const VkRect2D* pScissors,
                                                    const RecordObject& record_obj) {}
virtual void PostCallRecordCmdSetScissorWithCountEXT(VkCommandBuffer commandBuffer, uint32_t scissorCount,
                                                     const VkRect2D* pScissors, const RecordObject& record_obj) {}
virtual bool PreCallValidateCmdBindVertexBuffers2EXT(VkCommandBuffer commandBuffer, uint32_t firstBinding, uint32_t bindingCount,
                                                     const VkBuffer* pBuffers, const VkDeviceSize* pOffsets,
                                                     const VkDeviceSize* pSizes, const VkDeviceSize* pStrides,
                                                     ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordCmdBindVertexBuffers2EXT(VkCommandBuffer commandBuffer, uint32_t firstBinding, uint32_t bindingCount,
                                                   const VkBuffer* pBuffers, const VkDeviceSize* pOffsets,
                                                   const VkDeviceSize* pSizes, const VkDeviceSize* pStrides,
                                                   const RecordObject& record_obj) {}
virtual void PostCallRecordCmdBindVertexBuffers2EXT(VkCommandBuffer commandBuffer, uint32_t firstBinding, uint32_t bindingCount,
                                                    const VkBuffer* pBuffers, const VkDeviceSize* pOffsets,
                                                    const VkDeviceSize* pSizes, const VkDeviceSize* pStrides,
                                                    const RecordObject& record_obj) {}
virtual bool PreCallValidateCmdSetDepthTestEnableEXT(VkCommandBuffer commandBuffer, VkBool32 depthTestEnable,
                                                     ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordCmdSetDepthTestEnableEXT(VkCommandBuffer commandBuffer, VkBool32 depthTestEnable,
                                                   const RecordObject& record_obj) {}
virtual void PostCallRecordCmdSetDepthTestEnableEXT(VkCommandBuffer commandBuffer, VkBool32 depthTestEnable,
                                                    const RecordObject& record_obj) {}
virtual bool PreCallValidateCmdSetDepthWriteEnableEXT(VkCommandBuffer commandBuffer, VkBool32 depthWriteEnable,
                                                      ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordCmdSetDepthWriteEnableEXT(VkCommandBuffer commandBuffer, VkBool32 depthWriteEnable,
                                                    const RecordObject& record_obj) {}
virtual void PostCallRecordCmdSetDepthWriteEnableEXT(VkCommandBuffer commandBuffer, VkBool32 depthWriteEnable,
                                                     const RecordObject& record_obj) {}
virtual bool PreCallValidateCmdSetDepthCompareOpEXT(VkCommandBuffer commandBuffer, VkCompareOp depthCompareOp,
                                                    ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordCmdSetDepthCompareOpEXT(VkCommandBuffer commandBuffer, VkCompareOp depthCompareOp,
                                                  const RecordObject& record_obj) {}
virtual void PostCallRecordCmdSetDepthCompareOpEXT(VkCommandBuffer commandBuffer, VkCompareOp depthCompareOp,
                                                   const RecordObject& record_obj) {}
virtual bool PreCallValidateCmdSetDepthBoundsTestEnableEXT(VkCommandBuffer commandBuffer, VkBool32 depthBoundsTestEnable,
                                                           ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordCmdSetDepthBoundsTestEnableEXT(VkCommandBuffer commandBuffer, VkBool32 depthBoundsTestEnable,
                                                         const RecordObject& record_obj) {}
virtual void PostCallRecordCmdSetDepthBoundsTestEnableEXT(VkCommandBuffer commandBuffer, VkBool32 depthBoundsTestEnable,
                                                          const RecordObject& record_obj) {}
virtual bool PreCallValidateCmdSetStencilTestEnableEXT(VkCommandBuffer commandBuffer, VkBool32 stencilTestEnable,
                                                       ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordCmdSetStencilTestEnableEXT(VkCommandBuffer commandBuffer, VkBool32 stencilTestEnable,
                                                     const RecordObject& record_obj) {}
virtual void PostCallRecordCmdSetStencilTestEnableEXT(VkCommandBuffer commandBuffer, VkBool32 stencilTestEnable,
                                                      const RecordObject& record_obj) {}
virtual bool PreCallValidateCmdSetStencilOpEXT(VkCommandBuffer commandBuffer, VkStencilFaceFlags faceMask, VkStencilOp failOp,
                                               VkStencilOp passOp, VkStencilOp depthFailOp, VkCompareOp compareOp,
                                               ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordCmdSetStencilOpEXT(VkCommandBuffer commandBuffer, VkStencilFaceFlags faceMask, VkStencilOp failOp,
                                             VkStencilOp passOp, VkStencilOp depthFailOp, VkCompareOp compareOp,
                                             const RecordObject& record_obj) {}
virtual void PostCallRecordCmdSetStencilOpEXT(VkCommandBuffer commandBuffer, VkStencilFaceFlags faceMask, VkStencilOp failOp,
                                              VkStencilOp passOp, VkStencilOp depthFailOp, VkCompareOp compareOp,
                                              const RecordObject& record_obj) {}
virtual bool PreCallValidateCopyMemoryToImageEXT(VkDevice device, const VkCopyMemoryToImageInfo* pCopyMemoryToImageInfo,
                                                 ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordCopyMemoryToImageEXT(VkDevice device, const VkCopyMemoryToImageInfo* pCopyMemoryToImageInfo,
                                               const RecordObject& record_obj) {}
virtual void PostCallRecordCopyMemoryToImageEXT(VkDevice device, const VkCopyMemoryToImageInfo* pCopyMemoryToImageInfo,
                                                const RecordObject& record_obj) {}
virtual bool PreCallValidateCopyImageToMemoryEXT(VkDevice device, const VkCopyImageToMemoryInfo* pCopyImageToMemoryInfo,
                                                 ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordCopyImageToMemoryEXT(VkDevice device, const VkCopyImageToMemoryInfo* pCopyImageToMemoryInfo,
                                               const RecordObject& record_obj) {}
virtual void PostCallRecordCopyImageToMemoryEXT(VkDevice device, const VkCopyImageToMemoryInfo* pCopyImageToMemoryInfo,
                                                const RecordObject& record_obj) {}
virtual bool PreCallValidateCopyImageToImageEXT(VkDevice device, const VkCopyImageToImageInfo* pCopyImageToImageInfo,
                                                ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordCopyImageToImageEXT(VkDevice device, const VkCopyImageToImageInfo* pCopyImageToImageInfo,
                                              const RecordObject& record_obj) {}
virtual void PostCallRecordCopyImageToImageEXT(VkDevice device, const VkCopyImageToImageInfo* pCopyImageToImageInfo,
                                               const RecordObject& record_obj) {}
virtual bool PreCallValidateTransitionImageLayoutEXT(VkDevice device, uint32_t transitionCount,
                                                     const VkHostImageLayoutTransitionInfo* pTransitions,
                                                     ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordTransitionImageLayoutEXT(VkDevice device, uint32_t transitionCount,
                                                   const VkHostImageLayoutTransitionInfo* pTransitions,
                                                   const RecordObject& record_obj) {}
virtual void PostCallRecordTransitionImageLayoutEXT(VkDevice device, uint32_t transitionCount,
                                                    const VkHostImageLayoutTransitionInfo* pTransitions,
                                                    const RecordObject& record_obj) {}
virtual bool PreCallValidateGetImageSubresourceLayout2EXT(VkDevice device, VkImage image, const VkImageSubresource2* pSubresource,
                                                          VkSubresourceLayout2* pLayout, ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordGetImageSubresourceLayout2EXT(VkDevice device, VkImage image, const VkImageSubresource2* pSubresource,
                                                        VkSubresourceLayout2* pLayout, const RecordObject& record_obj) {}
virtual void PostCallRecordGetImageSubresourceLayout2EXT(VkDevice device, VkImage image, const VkImageSubresource2* pSubresource,
                                                         VkSubresourceLayout2* pLayout, const RecordObject& record_obj) {}
virtual bool PreCallValidateReleaseSwapchainImagesEXT(VkDevice device, const VkReleaseSwapchainImagesInfoEXT* pReleaseInfo,
                                                      ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordReleaseSwapchainImagesEXT(VkDevice device, const VkReleaseSwapchainImagesInfoEXT* pReleaseInfo,
                                                    const RecordObject& record_obj) {}
virtual void PostCallRecordReleaseSwapchainImagesEXT(VkDevice device, const VkReleaseSwapchainImagesInfoEXT* pReleaseInfo,
                                                     const RecordObject& record_obj) {}
virtual bool PreCallValidateGetGeneratedCommandsMemoryRequirementsNV(VkDevice device,
                                                                     const VkGeneratedCommandsMemoryRequirementsInfoNV* pInfo,
                                                                     VkMemoryRequirements2* pMemoryRequirements,
                                                                     ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordGetGeneratedCommandsMemoryRequirementsNV(VkDevice device,
                                                                   const VkGeneratedCommandsMemoryRequirementsInfoNV* pInfo,
                                                                   VkMemoryRequirements2* pMemoryRequirements,
                                                                   const RecordObject& record_obj) {}
virtual void PostCallRecordGetGeneratedCommandsMemoryRequirementsNV(VkDevice device,
                                                                    const VkGeneratedCommandsMemoryRequirementsInfoNV* pInfo,
                                                                    VkMemoryRequirements2* pMemoryRequirements,
                                                                    const RecordObject& record_obj) {}
virtual bool PreCallValidateCmdPreprocessGeneratedCommandsNV(VkCommandBuffer commandBuffer,
                                                             const VkGeneratedCommandsInfoNV* pGeneratedCommandsInfo,
                                                             ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordCmdPreprocessGeneratedCommandsNV(VkCommandBuffer commandBuffer,
                                                           const VkGeneratedCommandsInfoNV* pGeneratedCommandsInfo,
                                                           const RecordObject& record_obj) {}
virtual void PostCallRecordCmdPreprocessGeneratedCommandsNV(VkCommandBuffer commandBuffer,
                                                            const VkGeneratedCommandsInfoNV* pGeneratedCommandsInfo,
                                                            const RecordObject& record_obj) {}
virtual bool PreCallValidateCmdExecuteGeneratedCommandsNV(VkCommandBuffer commandBuffer, VkBool32 isPreprocessed,
                                                          const VkGeneratedCommandsInfoNV* pGeneratedCommandsInfo,
                                                          ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordCmdExecuteGeneratedCommandsNV(VkCommandBuffer commandBuffer, VkBool32 isPreprocessed,
                                                        const VkGeneratedCommandsInfoNV* pGeneratedCommandsInfo,
                                                        const RecordObject& record_obj) {}
virtual void PostCallRecordCmdExecuteGeneratedCommandsNV(VkCommandBuffer commandBuffer, VkBool32 isPreprocessed,
                                                         const VkGeneratedCommandsInfoNV* pGeneratedCommandsInfo,
                                                         const RecordObject& record_obj) {}
virtual bool PreCallValidateCmdBindPipelineShaderGroupNV(VkCommandBuffer commandBuffer, VkPipelineBindPoint pipelineBindPoint,
                                                         VkPipeline pipeline, uint32_t groupIndex, ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordCmdBindPipelineShaderGroupNV(VkCommandBuffer commandBuffer, VkPipelineBindPoint pipelineBindPoint,
                                                       VkPipeline pipeline, uint32_t groupIndex, const RecordObject& record_obj) {}
virtual void PostCallRecordCmdBindPipelineShaderGroupNV(VkCommandBuffer commandBuffer, VkPipelineBindPoint pipelineBindPoint,
                                                        VkPipeline pipeline, uint32_t groupIndex, const RecordObject& record_obj) {}
virtual bool PreCallValidateCreateIndirectCommandsLayoutNV(VkDevice device, const VkIndirectCommandsLayoutCreateInfoNV* pCreateInfo,
                                                           const VkAllocationCallbacks* pAllocator,
                                                           VkIndirectCommandsLayoutNV* pIndirectCommandsLayout,
                                                           ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordCreateIndirectCommandsLayoutNV(VkDevice device, const VkIndirectCommandsLayoutCreateInfoNV* pCreateInfo,
                                                         const VkAllocationCallbacks* pAllocator,
                                                         VkIndirectCommandsLayoutNV* pIndirectCommandsLayout,
                                                         const RecordObject& record_obj) {}
virtual void PostCallRecordCreateIndirectCommandsLayoutNV(VkDevice device, const VkIndirectCommandsLayoutCreateInfoNV* pCreateInfo,
                                                          const VkAllocationCallbacks* pAllocator,
                                                          VkIndirectCommandsLayoutNV* pIndirectCommandsLayout,
                                                          const RecordObject& record_obj) {}
virtual bool PreCallValidateDestroyIndirectCommandsLayoutNV(VkDevice device, VkIndirectCommandsLayoutNV indirectCommandsLayout,
                                                            const VkAllocationCallbacks* pAllocator, ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordDestroyIndirectCommandsLayoutNV(VkDevice device, VkIndirectCommandsLayoutNV indirectCommandsLayout,
                                                          const VkAllocationCallbacks* pAllocator, const RecordObject& record_obj) {
}
virtual void PostCallRecordDestroyIndirectCommandsLayoutNV(VkDevice device, VkIndirectCommandsLayoutNV indirectCommandsLayout,
                                                           const VkAllocationCallbacks* pAllocator,
                                                           const RecordObject& record_obj) {}
virtual bool PreCallValidateCmdSetDepthBias2EXT(VkCommandBuffer commandBuffer, const VkDepthBiasInfoEXT* pDepthBiasInfo,
                                                ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordCmdSetDepthBias2EXT(VkCommandBuffer commandBuffer, const VkDepthBiasInfoEXT* pDepthBiasInfo,
                                              const RecordObject& record_obj) {}
virtual void PostCallRecordCmdSetDepthBias2EXT(VkCommandBuffer commandBuffer, const VkDepthBiasInfoEXT* pDepthBiasInfo,
                                               const RecordObject& record_obj) {}
virtual bool PreCallValidateCreatePrivateDataSlotEXT(VkDevice device, const VkPrivateDataSlotCreateInfo* pCreateInfo,
                                                     const VkAllocationCallbacks* pAllocator, VkPrivateDataSlot* pPrivateDataSlot,
                                                     ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordCreatePrivateDataSlotEXT(VkDevice device, const VkPrivateDataSlotCreateInfo* pCreateInfo,
                                                   const VkAllocationCallbacks* pAllocator, VkPrivateDataSlot* pPrivateDataSlot,
                                                   const RecordObject& record_obj) {}
virtual void PostCallRecordCreatePrivateDataSlotEXT(VkDevice device, const VkPrivateDataSlotCreateInfo* pCreateInfo,
                                                    const VkAllocationCallbacks* pAllocator, VkPrivateDataSlot* pPrivateDataSlot,
                                                    const RecordObject& record_obj) {}
virtual bool PreCallValidateDestroyPrivateDataSlotEXT(VkDevice device, VkPrivateDataSlot privateDataSlot,
                                                      const VkAllocationCallbacks* pAllocator, ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordDestroyPrivateDataSlotEXT(VkDevice device, VkPrivateDataSlot privateDataSlot,
                                                    const VkAllocationCallbacks* pAllocator, const RecordObject& record_obj) {}
virtual void PostCallRecordDestroyPrivateDataSlotEXT(VkDevice device, VkPrivateDataSlot privateDataSlot,
                                                     const VkAllocationCallbacks* pAllocator, const RecordObject& record_obj) {}
virtual bool PreCallValidateSetPrivateDataEXT(VkDevice device, VkObjectType objectType, uint64_t objectHandle,
                                              VkPrivateDataSlot privateDataSlot, uint64_t data, ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordSetPrivateDataEXT(VkDevice device, VkObjectType objectType, uint64_t objectHandle,
                                            VkPrivateDataSlot privateDataSlot, uint64_t data, const RecordObject& record_obj) {}
virtual void PostCallRecordSetPrivateDataEXT(VkDevice device, VkObjectType objectType, uint64_t objectHandle,
                                             VkPrivateDataSlot privateDataSlot, uint64_t data, const RecordObject& record_obj) {}
virtual bool PreCallValidateGetPrivateDataEXT(VkDevice device, VkObjectType objectType, uint64_t objectHandle,
                                              VkPrivateDataSlot privateDataSlot, uint64_t* pData, ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordGetPrivateDataEXT(VkDevice device, VkObjectType objectType, uint64_t objectHandle,
                                            VkPrivateDataSlot privateDataSlot, uint64_t* pData, const RecordObject& record_obj) {}
virtual void PostCallRecordGetPrivateDataEXT(VkDevice device, VkObjectType objectType, uint64_t objectHandle,
                                             VkPrivateDataSlot privateDataSlot, uint64_t* pData, const RecordObject& record_obj) {}
virtual bool PreCallValidateCreateCudaModuleNV(VkDevice device, const VkCudaModuleCreateInfoNV* pCreateInfo,
                                               const VkAllocationCallbacks* pAllocator, VkCudaModuleNV* pModule,
                                               ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordCreateCudaModuleNV(VkDevice device, const VkCudaModuleCreateInfoNV* pCreateInfo,
                                             const VkAllocationCallbacks* pAllocator, VkCudaModuleNV* pModule,
                                             const RecordObject& record_obj) {}
virtual void PostCallRecordCreateCudaModuleNV(VkDevice device, const VkCudaModuleCreateInfoNV* pCreateInfo,
                                              const VkAllocationCallbacks* pAllocator, VkCudaModuleNV* pModule,
                                              const RecordObject& record_obj) {}
virtual bool PreCallValidateGetCudaModuleCacheNV(VkDevice device, VkCudaModuleNV module, size_t* pCacheSize, void* pCacheData,
                                                 ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordGetCudaModuleCacheNV(VkDevice device, VkCudaModuleNV module, size_t* pCacheSize, void* pCacheData,
                                               const RecordObject& record_obj) {}
virtual void PostCallRecordGetCudaModuleCacheNV(VkDevice device, VkCudaModuleNV module, size_t* pCacheSize, void* pCacheData,
                                                const RecordObject& record_obj) {}
virtual bool PreCallValidateCreateCudaFunctionNV(VkDevice device, const VkCudaFunctionCreateInfoNV* pCreateInfo,
                                                 const VkAllocationCallbacks* pAllocator, VkCudaFunctionNV* pFunction,
                                                 ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordCreateCudaFunctionNV(VkDevice device, const VkCudaFunctionCreateInfoNV* pCreateInfo,
                                               const VkAllocationCallbacks* pAllocator, VkCudaFunctionNV* pFunction,
                                               const RecordObject& record_obj) {}
virtual void PostCallRecordCreateCudaFunctionNV(VkDevice device, const VkCudaFunctionCreateInfoNV* pCreateInfo,
                                                const VkAllocationCallbacks* pAllocator, VkCudaFunctionNV* pFunction,
                                                const RecordObject& record_obj) {}
virtual bool PreCallValidateDestroyCudaModuleNV(VkDevice device, VkCudaModuleNV module, const VkAllocationCallbacks* pAllocator,
                                                ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordDestroyCudaModuleNV(VkDevice device, VkCudaModuleNV module, const VkAllocationCallbacks* pAllocator,
                                              const RecordObject& record_obj) {}
virtual void PostCallRecordDestroyCudaModuleNV(VkDevice device, VkCudaModuleNV module, const VkAllocationCallbacks* pAllocator,
                                               const RecordObject& record_obj) {}
virtual bool PreCallValidateDestroyCudaFunctionNV(VkDevice device, VkCudaFunctionNV function,
                                                  const VkAllocationCallbacks* pAllocator, ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordDestroyCudaFunctionNV(VkDevice device, VkCudaFunctionNV function, const VkAllocationCallbacks* pAllocator,
                                                const RecordObject& record_obj) {}
virtual void PostCallRecordDestroyCudaFunctionNV(VkDevice device, VkCudaFunctionNV function,
                                                 const VkAllocationCallbacks* pAllocator, const RecordObject& record_obj) {}
virtual bool PreCallValidateCmdCudaLaunchKernelNV(VkCommandBuffer commandBuffer, const VkCudaLaunchInfoNV* pLaunchInfo,
                                                  ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordCmdCudaLaunchKernelNV(VkCommandBuffer commandBuffer, const VkCudaLaunchInfoNV* pLaunchInfo,
                                                const RecordObject& record_obj) {}
virtual void PostCallRecordCmdCudaLaunchKernelNV(VkCommandBuffer commandBuffer, const VkCudaLaunchInfoNV* pLaunchInfo,
                                                 const RecordObject& record_obj) {}
#ifdef VK_USE_PLATFORM_METAL_EXT
virtual bool PreCallValidateExportMetalObjectsEXT(VkDevice device, VkExportMetalObjectsInfoEXT* pMetalObjectsInfo,
                                                  ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordExportMetalObjectsEXT(VkDevice device, VkExportMetalObjectsInfoEXT* pMetalObjectsInfo,
                                                const RecordObject& record_obj) {}
virtual void PostCallRecordExportMetalObjectsEXT(VkDevice device, VkExportMetalObjectsInfoEXT* pMetalObjectsInfo,
                                                 const RecordObject& record_obj) {}
#endif  // VK_USE_PLATFORM_METAL_EXT
virtual bool PreCallValidateGetDescriptorSetLayoutSizeEXT(VkDevice device, VkDescriptorSetLayout layout,
                                                          VkDeviceSize* pLayoutSizeInBytes, ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordGetDescriptorSetLayoutSizeEXT(VkDevice device, VkDescriptorSetLayout layout,
                                                        VkDeviceSize* pLayoutSizeInBytes, const RecordObject& record_obj) {}
virtual void PostCallRecordGetDescriptorSetLayoutSizeEXT(VkDevice device, VkDescriptorSetLayout layout,
                                                         VkDeviceSize* pLayoutSizeInBytes, const RecordObject& record_obj) {}
virtual bool PreCallValidateGetDescriptorSetLayoutBindingOffsetEXT(VkDevice device, VkDescriptorSetLayout layout, uint32_t binding,
                                                                   VkDeviceSize* pOffset, ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordGetDescriptorSetLayoutBindingOffsetEXT(VkDevice device, VkDescriptorSetLayout layout, uint32_t binding,
                                                                 VkDeviceSize* pOffset, const RecordObject& record_obj) {}
virtual void PostCallRecordGetDescriptorSetLayoutBindingOffsetEXT(VkDevice device, VkDescriptorSetLayout layout, uint32_t binding,
                                                                  VkDeviceSize* pOffset, const RecordObject& record_obj) {}
virtual bool PreCallValidateGetDescriptorEXT(VkDevice device, const VkDescriptorGetInfoEXT* pDescriptorInfo, size_t dataSize,
                                             void* pDescriptor, ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordGetDescriptorEXT(VkDevice device, const VkDescriptorGetInfoEXT* pDescriptorInfo, size_t dataSize,
                                           void* pDescriptor, const RecordObject& record_obj) {}
virtual void PostCallRecordGetDescriptorEXT(VkDevice device, const VkDescriptorGetInfoEXT* pDescriptorInfo, size_t dataSize,
                                            void* pDescriptor, const RecordObject& record_obj) {}
virtual bool PreCallValidateCmdBindDescriptorBuffersEXT(VkCommandBuffer commandBuffer, uint32_t bufferCount,
                                                        const VkDescriptorBufferBindingInfoEXT* pBindingInfos,
                                                        ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordCmdBindDescriptorBuffersEXT(VkCommandBuffer commandBuffer, uint32_t bufferCount,
                                                      const VkDescriptorBufferBindingInfoEXT* pBindingInfos,
                                                      const RecordObject& record_obj) {}
virtual void PostCallRecordCmdBindDescriptorBuffersEXT(VkCommandBuffer commandBuffer, uint32_t bufferCount,
                                                       const VkDescriptorBufferBindingInfoEXT* pBindingInfos,
                                                       const RecordObject& record_obj) {}
virtual bool PreCallValidateCmdSetDescriptorBufferOffsetsEXT(VkCommandBuffer commandBuffer, VkPipelineBindPoint pipelineBindPoint,
                                                             VkPipelineLayout layout, uint32_t firstSet, uint32_t setCount,
                                                             const uint32_t* pBufferIndices, const VkDeviceSize* pOffsets,
                                                             ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordCmdSetDescriptorBufferOffsetsEXT(VkCommandBuffer commandBuffer, VkPipelineBindPoint pipelineBindPoint,
                                                           VkPipelineLayout layout, uint32_t firstSet, uint32_t setCount,
                                                           const uint32_t* pBufferIndices, const VkDeviceSize* pOffsets,
                                                           const RecordObject& record_obj) {}
virtual void PostCallRecordCmdSetDescriptorBufferOffsetsEXT(VkCommandBuffer commandBuffer, VkPipelineBindPoint pipelineBindPoint,
                                                            VkPipelineLayout layout, uint32_t firstSet, uint32_t setCount,
                                                            const uint32_t* pBufferIndices, const VkDeviceSize* pOffsets,
                                                            const RecordObject& record_obj) {}
virtual bool PreCallValidateCmdBindDescriptorBufferEmbeddedSamplersEXT(VkCommandBuffer commandBuffer,
                                                                       VkPipelineBindPoint pipelineBindPoint,
                                                                       VkPipelineLayout layout, uint32_t set,
                                                                       ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordCmdBindDescriptorBufferEmbeddedSamplersEXT(VkCommandBuffer commandBuffer,
                                                                     VkPipelineBindPoint pipelineBindPoint, VkPipelineLayout layout,
                                                                     uint32_t set, const RecordObject& record_obj) {}
virtual void PostCallRecordCmdBindDescriptorBufferEmbeddedSamplersEXT(VkCommandBuffer commandBuffer,
                                                                      VkPipelineBindPoint pipelineBindPoint,
                                                                      VkPipelineLayout layout, uint32_t set,
                                                                      const RecordObject& record_obj) {}
virtual bool PreCallValidateGetBufferOpaqueCaptureDescriptorDataEXT(VkDevice device,
                                                                    const VkBufferCaptureDescriptorDataInfoEXT* pInfo, void* pData,
                                                                    ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordGetBufferOpaqueCaptureDescriptorDataEXT(VkDevice device,
                                                                  const VkBufferCaptureDescriptorDataInfoEXT* pInfo, void* pData,
                                                                  const RecordObject& record_obj) {}
virtual void PostCallRecordGetBufferOpaqueCaptureDescriptorDataEXT(VkDevice device,
                                                                   const VkBufferCaptureDescriptorDataInfoEXT* pInfo, void* pData,
                                                                   const RecordObject& record_obj) {}
virtual bool PreCallValidateGetImageOpaqueCaptureDescriptorDataEXT(VkDevice device,
                                                                   const VkImageCaptureDescriptorDataInfoEXT* pInfo, void* pData,
                                                                   ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordGetImageOpaqueCaptureDescriptorDataEXT(VkDevice device, const VkImageCaptureDescriptorDataInfoEXT* pInfo,
                                                                 void* pData, const RecordObject& record_obj) {}
virtual void PostCallRecordGetImageOpaqueCaptureDescriptorDataEXT(VkDevice device, const VkImageCaptureDescriptorDataInfoEXT* pInfo,
                                                                  void* pData, const RecordObject& record_obj) {}
virtual bool PreCallValidateGetImageViewOpaqueCaptureDescriptorDataEXT(VkDevice device,
                                                                       const VkImageViewCaptureDescriptorDataInfoEXT* pInfo,
                                                                       void* pData, ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordGetImageViewOpaqueCaptureDescriptorDataEXT(VkDevice device,
                                                                     const VkImageViewCaptureDescriptorDataInfoEXT* pInfo,
                                                                     void* pData, const RecordObject& record_obj) {}
virtual void PostCallRecordGetImageViewOpaqueCaptureDescriptorDataEXT(VkDevice device,
                                                                      const VkImageViewCaptureDescriptorDataInfoEXT* pInfo,
                                                                      void* pData, const RecordObject& record_obj) {}
virtual bool PreCallValidateGetSamplerOpaqueCaptureDescriptorDataEXT(VkDevice device,
                                                                     const VkSamplerCaptureDescriptorDataInfoEXT* pInfo,
                                                                     void* pData, ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordGetSamplerOpaqueCaptureDescriptorDataEXT(VkDevice device,
                                                                   const VkSamplerCaptureDescriptorDataInfoEXT* pInfo, void* pData,
                                                                   const RecordObject& record_obj) {}
virtual void PostCallRecordGetSamplerOpaqueCaptureDescriptorDataEXT(VkDevice device,
                                                                    const VkSamplerCaptureDescriptorDataInfoEXT* pInfo, void* pData,
                                                                    const RecordObject& record_obj) {}
virtual bool PreCallValidateGetAccelerationStructureOpaqueCaptureDescriptorDataEXT(
    VkDevice device, const VkAccelerationStructureCaptureDescriptorDataInfoEXT* pInfo, void* pData, ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordGetAccelerationStructureOpaqueCaptureDescriptorDataEXT(
    VkDevice device, const VkAccelerationStructureCaptureDescriptorDataInfoEXT* pInfo, void* pData,
    const RecordObject& record_obj) {}
virtual void PostCallRecordGetAccelerationStructureOpaqueCaptureDescriptorDataEXT(
    VkDevice device, const VkAccelerationStructureCaptureDescriptorDataInfoEXT* pInfo, void* pData,
    const RecordObject& record_obj) {}
virtual bool PreCallValidateCmdSetFragmentShadingRateEnumNV(VkCommandBuffer commandBuffer, VkFragmentShadingRateNV shadingRate,
                                                            const VkFragmentShadingRateCombinerOpKHR combinerOps[2],
                                                            ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordCmdSetFragmentShadingRateEnumNV(VkCommandBuffer commandBuffer, VkFragmentShadingRateNV shadingRate,
                                                          const VkFragmentShadingRateCombinerOpKHR combinerOps[2],
                                                          const RecordObject& record_obj) {}
virtual void PostCallRecordCmdSetFragmentShadingRateEnumNV(VkCommandBuffer commandBuffer, VkFragmentShadingRateNV shadingRate,
                                                           const VkFragmentShadingRateCombinerOpKHR combinerOps[2],
                                                           const RecordObject& record_obj) {}
virtual bool PreCallValidateGetDeviceFaultInfoEXT(VkDevice device, VkDeviceFaultCountsEXT* pFaultCounts,
                                                  VkDeviceFaultInfoEXT* pFaultInfo, ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordGetDeviceFaultInfoEXT(VkDevice device, VkDeviceFaultCountsEXT* pFaultCounts,
                                                VkDeviceFaultInfoEXT* pFaultInfo, const RecordObject& record_obj) {}
virtual void PostCallRecordGetDeviceFaultInfoEXT(VkDevice device, VkDeviceFaultCountsEXT* pFaultCounts,
                                                 VkDeviceFaultInfoEXT* pFaultInfo, const RecordObject& record_obj) {}
virtual bool PreCallValidateCmdSetVertexInputEXT(VkCommandBuffer commandBuffer, uint32_t vertexBindingDescriptionCount,
                                                 const VkVertexInputBindingDescription2EXT* pVertexBindingDescriptions,
                                                 uint32_t vertexAttributeDescriptionCount,
                                                 const VkVertexInputAttributeDescription2EXT* pVertexAttributeDescriptions,
                                                 ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordCmdSetVertexInputEXT(VkCommandBuffer commandBuffer, uint32_t vertexBindingDescriptionCount,
                                               const VkVertexInputBindingDescription2EXT* pVertexBindingDescriptions,
                                               uint32_t vertexAttributeDescriptionCount,
                                               const VkVertexInputAttributeDescription2EXT* pVertexAttributeDescriptions,
                                               const RecordObject& record_obj) {}
virtual void PostCallRecordCmdSetVertexInputEXT(VkCommandBuffer commandBuffer, uint32_t vertexBindingDescriptionCount,
                                                const VkVertexInputBindingDescription2EXT* pVertexBindingDescriptions,
                                                uint32_t vertexAttributeDescriptionCount,
                                                const VkVertexInputAttributeDescription2EXT* pVertexAttributeDescriptions,
                                                const RecordObject& record_obj) {}
#ifdef VK_USE_PLATFORM_FUCHSIA
virtual bool PreCallValidateGetMemoryZirconHandleFUCHSIA(VkDevice device,
                                                         const VkMemoryGetZirconHandleInfoFUCHSIA* pGetZirconHandleInfo,
                                                         zx_handle_t* pZirconHandle, ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordGetMemoryZirconHandleFUCHSIA(VkDevice device,
                                                       const VkMemoryGetZirconHandleInfoFUCHSIA* pGetZirconHandleInfo,
                                                       zx_handle_t* pZirconHandle, const RecordObject& record_obj) {}
virtual void PostCallRecordGetMemoryZirconHandleFUCHSIA(VkDevice device,
                                                        const VkMemoryGetZirconHandleInfoFUCHSIA* pGetZirconHandleInfo,
                                                        zx_handle_t* pZirconHandle, const RecordObject& record_obj) {}
virtual bool PreCallValidateGetMemoryZirconHandlePropertiesFUCHSIA(
    VkDevice device, VkExternalMemoryHandleTypeFlagBits handleType, zx_handle_t zirconHandle,
    VkMemoryZirconHandlePropertiesFUCHSIA* pMemoryZirconHandleProperties, ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordGetMemoryZirconHandlePropertiesFUCHSIA(
    VkDevice device, VkExternalMemoryHandleTypeFlagBits handleType, zx_handle_t zirconHandle,
    VkMemoryZirconHandlePropertiesFUCHSIA* pMemoryZirconHandleProperties, const RecordObject& record_obj) {}
virtual void PostCallRecordGetMemoryZirconHandlePropertiesFUCHSIA(
    VkDevice device, VkExternalMemoryHandleTypeFlagBits handleType, zx_handle_t zirconHandle,
    VkMemoryZirconHandlePropertiesFUCHSIA* pMemoryZirconHandleProperties, const RecordObject& record_obj) {}
virtual bool PreCallValidateImportSemaphoreZirconHandleFUCHSIA(
    VkDevice device, const VkImportSemaphoreZirconHandleInfoFUCHSIA* pImportSemaphoreZirconHandleInfo,
    ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordImportSemaphoreZirconHandleFUCHSIA(
    VkDevice device, const VkImportSemaphoreZirconHandleInfoFUCHSIA* pImportSemaphoreZirconHandleInfo,
    const RecordObject& record_obj) {}
virtual void PostCallRecordImportSemaphoreZirconHandleFUCHSIA(
    VkDevice device, const VkImportSemaphoreZirconHandleInfoFUCHSIA* pImportSemaphoreZirconHandleInfo,
    const RecordObject& record_obj) {}
virtual bool PreCallValidateGetSemaphoreZirconHandleFUCHSIA(VkDevice device,
                                                            const VkSemaphoreGetZirconHandleInfoFUCHSIA* pGetZirconHandleInfo,
                                                            zx_handle_t* pZirconHandle, ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordGetSemaphoreZirconHandleFUCHSIA(VkDevice device,
                                                          const VkSemaphoreGetZirconHandleInfoFUCHSIA* pGetZirconHandleInfo,
                                                          zx_handle_t* pZirconHandle, const RecordObject& record_obj) {}
virtual void PostCallRecordGetSemaphoreZirconHandleFUCHSIA(VkDevice device,
                                                           const VkSemaphoreGetZirconHandleInfoFUCHSIA* pGetZirconHandleInfo,
                                                           zx_handle_t* pZirconHandle, const RecordObject& record_obj) {}
virtual bool PreCallValidateCreateBufferCollectionFUCHSIA(VkDevice device, const VkBufferCollectionCreateInfoFUCHSIA* pCreateInfo,
                                                          const VkAllocationCallbacks* pAllocator,
                                                          VkBufferCollectionFUCHSIA* pCollection, ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordCreateBufferCollectionFUCHSIA(VkDevice device, const VkBufferCollectionCreateInfoFUCHSIA* pCreateInfo,
                                                        const VkAllocationCallbacks* pAllocator,
                                                        VkBufferCollectionFUCHSIA* pCollection, const RecordObject& record_obj) {}
virtual void PostCallRecordCreateBufferCollectionFUCHSIA(VkDevice device, const VkBufferCollectionCreateInfoFUCHSIA* pCreateInfo,
                                                         const VkAllocationCallbacks* pAllocator,
                                                         VkBufferCollectionFUCHSIA* pCollection, const RecordObject& record_obj) {}
virtual bool PreCallValidateSetBufferCollectionImageConstraintsFUCHSIA(VkDevice device, VkBufferCollectionFUCHSIA collection,
                                                                       const VkImageConstraintsInfoFUCHSIA* pImageConstraintsInfo,
                                                                       ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordSetBufferCollectionImageConstraintsFUCHSIA(VkDevice device, VkBufferCollectionFUCHSIA collection,
                                                                     const VkImageConstraintsInfoFUCHSIA* pImageConstraintsInfo,
                                                                     const RecordObject& record_obj) {}
virtual void PostCallRecordSetBufferCollectionImageConstraintsFUCHSIA(VkDevice device, VkBufferCollectionFUCHSIA collection,
                                                                      const VkImageConstraintsInfoFUCHSIA* pImageConstraintsInfo,
                                                                      const RecordObject& record_obj) {}
virtual bool PreCallValidateSetBufferCollectionBufferConstraintsFUCHSIA(
    VkDevice device, VkBufferCollectionFUCHSIA collection, const VkBufferConstraintsInfoFUCHSIA* pBufferConstraintsInfo,
    ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordSetBufferCollectionBufferConstraintsFUCHSIA(VkDevice device, VkBufferCollectionFUCHSIA collection,
                                                                      const VkBufferConstraintsInfoFUCHSIA* pBufferConstraintsInfo,
                                                                      const RecordObject& record_obj) {}
virtual void PostCallRecordSetBufferCollectionBufferConstraintsFUCHSIA(VkDevice device, VkBufferCollectionFUCHSIA collection,
                                                                       const VkBufferConstraintsInfoFUCHSIA* pBufferConstraintsInfo,
                                                                       const RecordObject& record_obj) {}
virtual bool PreCallValidateDestroyBufferCollectionFUCHSIA(VkDevice device, VkBufferCollectionFUCHSIA collection,
                                                           const VkAllocationCallbacks* pAllocator, ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordDestroyBufferCollectionFUCHSIA(VkDevice device, VkBufferCollectionFUCHSIA collection,
                                                         const VkAllocationCallbacks* pAllocator, const RecordObject& record_obj) {}
virtual void PostCallRecordDestroyBufferCollectionFUCHSIA(VkDevice device, VkBufferCollectionFUCHSIA collection,
                                                          const VkAllocationCallbacks* pAllocator, const RecordObject& record_obj) {
}
virtual bool PreCallValidateGetBufferCollectionPropertiesFUCHSIA(VkDevice device, VkBufferCollectionFUCHSIA collection,
                                                                 VkBufferCollectionPropertiesFUCHSIA* pProperties,
                                                                 ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordGetBufferCollectionPropertiesFUCHSIA(VkDevice device, VkBufferCollectionFUCHSIA collection,
                                                               VkBufferCollectionPropertiesFUCHSIA* pProperties,
                                                               const RecordObject& record_obj) {}
virtual void PostCallRecordGetBufferCollectionPropertiesFUCHSIA(VkDevice device, VkBufferCollectionFUCHSIA collection,
                                                                VkBufferCollectionPropertiesFUCHSIA* pProperties,
                                                                const RecordObject& record_obj) {}
#endif  // VK_USE_PLATFORM_FUCHSIA
virtual bool PreCallValidateGetDeviceSubpassShadingMaxWorkgroupSizeHUAWEI(VkDevice device, VkRenderPass renderpass,
                                                                          VkExtent2D* pMaxWorkgroupSize,
                                                                          ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordGetDeviceSubpassShadingMaxWorkgroupSizeHUAWEI(VkDevice device, VkRenderPass renderpass,
                                                                        VkExtent2D* pMaxWorkgroupSize,
                                                                        const RecordObject& record_obj) {}
virtual void PostCallRecordGetDeviceSubpassShadingMaxWorkgroupSizeHUAWEI(VkDevice device, VkRenderPass renderpass,
                                                                         VkExtent2D* pMaxWorkgroupSize,
                                                                         const RecordObject& record_obj) {}
virtual bool PreCallValidateCmdSubpassShadingHUAWEI(VkCommandBuffer commandBuffer, ErrorObject& error_obj) const { return false; }
virtual void PreCallRecordCmdSubpassShadingHUAWEI(VkCommandBuffer commandBuffer, const RecordObject& record_obj) {}
virtual void PostCallRecordCmdSubpassShadingHUAWEI(VkCommandBuffer commandBuffer, const RecordObject& record_obj) {}
virtual bool PreCallValidateCmdBindInvocationMaskHUAWEI(VkCommandBuffer commandBuffer, VkImageView imageView,
                                                        VkImageLayout imageLayout, ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordCmdBindInvocationMaskHUAWEI(VkCommandBuffer commandBuffer, VkImageView imageView,
                                                      VkImageLayout imageLayout, const RecordObject& record_obj) {}
virtual void PostCallRecordCmdBindInvocationMaskHUAWEI(VkCommandBuffer commandBuffer, VkImageView imageView,
                                                       VkImageLayout imageLayout, const RecordObject& record_obj) {}
virtual bool PreCallValidateGetMemoryRemoteAddressNV(VkDevice device,
                                                     const VkMemoryGetRemoteAddressInfoNV* pMemoryGetRemoteAddressInfo,
                                                     VkRemoteAddressNV* pAddress, ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordGetMemoryRemoteAddressNV(VkDevice device,
                                                   const VkMemoryGetRemoteAddressInfoNV* pMemoryGetRemoteAddressInfo,
                                                   VkRemoteAddressNV* pAddress, const RecordObject& record_obj) {}
virtual void PostCallRecordGetMemoryRemoteAddressNV(VkDevice device,
                                                    const VkMemoryGetRemoteAddressInfoNV* pMemoryGetRemoteAddressInfo,
                                                    VkRemoteAddressNV* pAddress, const RecordObject& record_obj) {}
virtual bool PreCallValidateGetPipelinePropertiesEXT(VkDevice device, const VkPipelineInfoEXT* pPipelineInfo,
                                                     VkBaseOutStructure* pPipelineProperties, ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordGetPipelinePropertiesEXT(VkDevice device, const VkPipelineInfoEXT* pPipelineInfo,
                                                   VkBaseOutStructure* pPipelineProperties, const RecordObject& record_obj) {}
virtual void PostCallRecordGetPipelinePropertiesEXT(VkDevice device, const VkPipelineInfoEXT* pPipelineInfo,
                                                    VkBaseOutStructure* pPipelineProperties, const RecordObject& record_obj) {}
virtual bool PreCallValidateCmdSetPatchControlPointsEXT(VkCommandBuffer commandBuffer, uint32_t patchControlPoints,
                                                        ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordCmdSetPatchControlPointsEXT(VkCommandBuffer commandBuffer, uint32_t patchControlPoints,
                                                      const RecordObject& record_obj) {}
virtual void PostCallRecordCmdSetPatchControlPointsEXT(VkCommandBuffer commandBuffer, uint32_t patchControlPoints,
                                                       const RecordObject& record_obj) {}
virtual bool PreCallValidateCmdSetRasterizerDiscardEnableEXT(VkCommandBuffer commandBuffer, VkBool32 rasterizerDiscardEnable,
                                                             ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordCmdSetRasterizerDiscardEnableEXT(VkCommandBuffer commandBuffer, VkBool32 rasterizerDiscardEnable,
                                                           const RecordObject& record_obj) {}
virtual void PostCallRecordCmdSetRasterizerDiscardEnableEXT(VkCommandBuffer commandBuffer, VkBool32 rasterizerDiscardEnable,
                                                            const RecordObject& record_obj) {}
virtual bool PreCallValidateCmdSetDepthBiasEnableEXT(VkCommandBuffer commandBuffer, VkBool32 depthBiasEnable,
                                                     ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordCmdSetDepthBiasEnableEXT(VkCommandBuffer commandBuffer, VkBool32 depthBiasEnable,
                                                   const RecordObject& record_obj) {}
virtual void PostCallRecordCmdSetDepthBiasEnableEXT(VkCommandBuffer commandBuffer, VkBool32 depthBiasEnable,
                                                    const RecordObject& record_obj) {}
virtual bool PreCallValidateCmdSetLogicOpEXT(VkCommandBuffer commandBuffer, VkLogicOp logicOp, ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordCmdSetLogicOpEXT(VkCommandBuffer commandBuffer, VkLogicOp logicOp, const RecordObject& record_obj) {}
virtual void PostCallRecordCmdSetLogicOpEXT(VkCommandBuffer commandBuffer, VkLogicOp logicOp, const RecordObject& record_obj) {}
virtual bool PreCallValidateCmdSetPrimitiveRestartEnableEXT(VkCommandBuffer commandBuffer, VkBool32 primitiveRestartEnable,
                                                            ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordCmdSetPrimitiveRestartEnableEXT(VkCommandBuffer commandBuffer, VkBool32 primitiveRestartEnable,
                                                          const RecordObject& record_obj) {}
virtual void PostCallRecordCmdSetPrimitiveRestartEnableEXT(VkCommandBuffer commandBuffer, VkBool32 primitiveRestartEnable,
                                                           const RecordObject& record_obj) {}
virtual bool PreCallValidateCmdSetColorWriteEnableEXT(VkCommandBuffer commandBuffer, uint32_t attachmentCount,
                                                      const VkBool32* pColorWriteEnables, ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordCmdSetColorWriteEnableEXT(VkCommandBuffer commandBuffer, uint32_t attachmentCount,
                                                    const VkBool32* pColorWriteEnables, const RecordObject& record_obj) {}
virtual void PostCallRecordCmdSetColorWriteEnableEXT(VkCommandBuffer commandBuffer, uint32_t attachmentCount,
                                                     const VkBool32* pColorWriteEnables, const RecordObject& record_obj) {}
virtual bool PreCallValidateCmdDrawMultiEXT(VkCommandBuffer commandBuffer, uint32_t drawCount,
                                            const VkMultiDrawInfoEXT* pVertexInfo, uint32_t instanceCount, uint32_t firstInstance,
                                            uint32_t stride, ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordCmdDrawMultiEXT(VkCommandBuffer commandBuffer, uint32_t drawCount, const VkMultiDrawInfoEXT* pVertexInfo,
                                          uint32_t instanceCount, uint32_t firstInstance, uint32_t stride,
                                          const RecordObject& record_obj) {}
virtual void PostCallRecordCmdDrawMultiEXT(VkCommandBuffer commandBuffer, uint32_t drawCount, const VkMultiDrawInfoEXT* pVertexInfo,
                                           uint32_t instanceCount, uint32_t firstInstance, uint32_t stride,
                                           const RecordObject& record_obj) {}
virtual bool PreCallValidateCmdDrawMultiIndexedEXT(VkCommandBuffer commandBuffer, uint32_t drawCount,
                                                   const VkMultiDrawIndexedInfoEXT* pIndexInfo, uint32_t instanceCount,
                                                   uint32_t firstInstance, uint32_t stride, const int32_t* pVertexOffset,
                                                   ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordCmdDrawMultiIndexedEXT(VkCommandBuffer commandBuffer, uint32_t drawCount,
                                                 const VkMultiDrawIndexedInfoEXT* pIndexInfo, uint32_t instanceCount,
                                                 uint32_t firstInstance, uint32_t stride, const int32_t* pVertexOffset,
                                                 const RecordObject& record_obj) {}
virtual void PostCallRecordCmdDrawMultiIndexedEXT(VkCommandBuffer commandBuffer, uint32_t drawCount,
                                                  const VkMultiDrawIndexedInfoEXT* pIndexInfo, uint32_t instanceCount,
                                                  uint32_t firstInstance, uint32_t stride, const int32_t* pVertexOffset,
                                                  const RecordObject& record_obj) {}
virtual bool PreCallValidateCreateMicromapEXT(VkDevice device, const VkMicromapCreateInfoEXT* pCreateInfo,
                                              const VkAllocationCallbacks* pAllocator, VkMicromapEXT* pMicromap,
                                              ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordCreateMicromapEXT(VkDevice device, const VkMicromapCreateInfoEXT* pCreateInfo,
                                            const VkAllocationCallbacks* pAllocator, VkMicromapEXT* pMicromap,
                                            const RecordObject& record_obj) {}
virtual void PostCallRecordCreateMicromapEXT(VkDevice device, const VkMicromapCreateInfoEXT* pCreateInfo,
                                             const VkAllocationCallbacks* pAllocator, VkMicromapEXT* pMicromap,
                                             const RecordObject& record_obj) {}
virtual bool PreCallValidateDestroyMicromapEXT(VkDevice device, VkMicromapEXT micromap, const VkAllocationCallbacks* pAllocator,
                                               ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordDestroyMicromapEXT(VkDevice device, VkMicromapEXT micromap, const VkAllocationCallbacks* pAllocator,
                                             const RecordObject& record_obj) {}
virtual void PostCallRecordDestroyMicromapEXT(VkDevice device, VkMicromapEXT micromap, const VkAllocationCallbacks* pAllocator,
                                              const RecordObject& record_obj) {}
virtual bool PreCallValidateCmdBuildMicromapsEXT(VkCommandBuffer commandBuffer, uint32_t infoCount,
                                                 const VkMicromapBuildInfoEXT* pInfos, ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordCmdBuildMicromapsEXT(VkCommandBuffer commandBuffer, uint32_t infoCount,
                                               const VkMicromapBuildInfoEXT* pInfos, const RecordObject& record_obj) {}
virtual void PostCallRecordCmdBuildMicromapsEXT(VkCommandBuffer commandBuffer, uint32_t infoCount,
                                                const VkMicromapBuildInfoEXT* pInfos, const RecordObject& record_obj) {}
virtual bool PreCallValidateBuildMicromapsEXT(VkDevice device, VkDeferredOperationKHR deferredOperation, uint32_t infoCount,
                                              const VkMicromapBuildInfoEXT* pInfos, ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordBuildMicromapsEXT(VkDevice device, VkDeferredOperationKHR deferredOperation, uint32_t infoCount,
                                            const VkMicromapBuildInfoEXT* pInfos, const RecordObject& record_obj) {}
virtual void PostCallRecordBuildMicromapsEXT(VkDevice device, VkDeferredOperationKHR deferredOperation, uint32_t infoCount,
                                             const VkMicromapBuildInfoEXT* pInfos, const RecordObject& record_obj) {}
virtual bool PreCallValidateCopyMicromapEXT(VkDevice device, VkDeferredOperationKHR deferredOperation,
                                            const VkCopyMicromapInfoEXT* pInfo, ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordCopyMicromapEXT(VkDevice device, VkDeferredOperationKHR deferredOperation,
                                          const VkCopyMicromapInfoEXT* pInfo, const RecordObject& record_obj) {}
virtual void PostCallRecordCopyMicromapEXT(VkDevice device, VkDeferredOperationKHR deferredOperation,
                                           const VkCopyMicromapInfoEXT* pInfo, const RecordObject& record_obj) {}
virtual bool PreCallValidateCopyMicromapToMemoryEXT(VkDevice device, VkDeferredOperationKHR deferredOperation,
                                                    const VkCopyMicromapToMemoryInfoEXT* pInfo, ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordCopyMicromapToMemoryEXT(VkDevice device, VkDeferredOperationKHR deferredOperation,
                                                  const VkCopyMicromapToMemoryInfoEXT* pInfo, const RecordObject& record_obj) {}
virtual void PostCallRecordCopyMicromapToMemoryEXT(VkDevice device, VkDeferredOperationKHR deferredOperation,
                                                   const VkCopyMicromapToMemoryInfoEXT* pInfo, const RecordObject& record_obj) {}
virtual bool PreCallValidateCopyMemoryToMicromapEXT(VkDevice device, VkDeferredOperationKHR deferredOperation,
                                                    const VkCopyMemoryToMicromapInfoEXT* pInfo, ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordCopyMemoryToMicromapEXT(VkDevice device, VkDeferredOperationKHR deferredOperation,
                                                  const VkCopyMemoryToMicromapInfoEXT* pInfo, const RecordObject& record_obj) {}
virtual void PostCallRecordCopyMemoryToMicromapEXT(VkDevice device, VkDeferredOperationKHR deferredOperation,
                                                   const VkCopyMemoryToMicromapInfoEXT* pInfo, const RecordObject& record_obj) {}
virtual bool PreCallValidateWriteMicromapsPropertiesEXT(VkDevice device, uint32_t micromapCount, const VkMicromapEXT* pMicromaps,
                                                        VkQueryType queryType, size_t dataSize, void* pData, size_t stride,
                                                        ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordWriteMicromapsPropertiesEXT(VkDevice device, uint32_t micromapCount, const VkMicromapEXT* pMicromaps,
                                                      VkQueryType queryType, size_t dataSize, void* pData, size_t stride,
                                                      const RecordObject& record_obj) {}
virtual void PostCallRecordWriteMicromapsPropertiesEXT(VkDevice device, uint32_t micromapCount, const VkMicromapEXT* pMicromaps,
                                                       VkQueryType queryType, size_t dataSize, void* pData, size_t stride,
                                                       const RecordObject& record_obj) {}
virtual bool PreCallValidateCmdCopyMicromapEXT(VkCommandBuffer commandBuffer, const VkCopyMicromapInfoEXT* pInfo,
                                               ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordCmdCopyMicromapEXT(VkCommandBuffer commandBuffer, const VkCopyMicromapInfoEXT* pInfo,
                                             const RecordObject& record_obj) {}
virtual void PostCallRecordCmdCopyMicromapEXT(VkCommandBuffer commandBuffer, const VkCopyMicromapInfoEXT* pInfo,
                                              const RecordObject& record_obj) {}
virtual bool PreCallValidateCmdCopyMicromapToMemoryEXT(VkCommandBuffer commandBuffer, const VkCopyMicromapToMemoryInfoEXT* pInfo,
                                                       ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordCmdCopyMicromapToMemoryEXT(VkCommandBuffer commandBuffer, const VkCopyMicromapToMemoryInfoEXT* pInfo,
                                                     const RecordObject& record_obj) {}
virtual void PostCallRecordCmdCopyMicromapToMemoryEXT(VkCommandBuffer commandBuffer, const VkCopyMicromapToMemoryInfoEXT* pInfo,
                                                      const RecordObject& record_obj) {}
virtual bool PreCallValidateCmdCopyMemoryToMicromapEXT(VkCommandBuffer commandBuffer, const VkCopyMemoryToMicromapInfoEXT* pInfo,
                                                       ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordCmdCopyMemoryToMicromapEXT(VkCommandBuffer commandBuffer, const VkCopyMemoryToMicromapInfoEXT* pInfo,
                                                     const RecordObject& record_obj) {}
virtual void PostCallRecordCmdCopyMemoryToMicromapEXT(VkCommandBuffer commandBuffer, const VkCopyMemoryToMicromapInfoEXT* pInfo,
                                                      const RecordObject& record_obj) {}
virtual bool PreCallValidateCmdWriteMicromapsPropertiesEXT(VkCommandBuffer commandBuffer, uint32_t micromapCount,
                                                           const VkMicromapEXT* pMicromaps, VkQueryType queryType,
                                                           VkQueryPool queryPool, uint32_t firstQuery,
                                                           ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordCmdWriteMicromapsPropertiesEXT(VkCommandBuffer commandBuffer, uint32_t micromapCount,
                                                         const VkMicromapEXT* pMicromaps, VkQueryType queryType,
                                                         VkQueryPool queryPool, uint32_t firstQuery,
                                                         const RecordObject& record_obj) {}
virtual void PostCallRecordCmdWriteMicromapsPropertiesEXT(VkCommandBuffer commandBuffer, uint32_t micromapCount,
                                                          const VkMicromapEXT* pMicromaps, VkQueryType queryType,
                                                          VkQueryPool queryPool, uint32_t firstQuery,
                                                          const RecordObject& record_obj) {}
virtual bool PreCallValidateGetDeviceMicromapCompatibilityEXT(VkDevice device, const VkMicromapVersionInfoEXT* pVersionInfo,
                                                              VkAccelerationStructureCompatibilityKHR* pCompatibility,
                                                              ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordGetDeviceMicromapCompatibilityEXT(VkDevice device, const VkMicromapVersionInfoEXT* pVersionInfo,
                                                            VkAccelerationStructureCompatibilityKHR* pCompatibility,
                                                            const RecordObject& record_obj) {}
virtual void PostCallRecordGetDeviceMicromapCompatibilityEXT(VkDevice device, const VkMicromapVersionInfoEXT* pVersionInfo,
                                                             VkAccelerationStructureCompatibilityKHR* pCompatibility,
                                                             const RecordObject& record_obj) {}
virtual bool PreCallValidateGetMicromapBuildSizesEXT(VkDevice device, VkAccelerationStructureBuildTypeKHR buildType,
                                                     const VkMicromapBuildInfoEXT* pBuildInfo,
                                                     VkMicromapBuildSizesInfoEXT* pSizeInfo, ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordGetMicromapBuildSizesEXT(VkDevice device, VkAccelerationStructureBuildTypeKHR buildType,
                                                   const VkMicromapBuildInfoEXT* pBuildInfo, VkMicromapBuildSizesInfoEXT* pSizeInfo,
                                                   const RecordObject& record_obj) {}
virtual void PostCallRecordGetMicromapBuildSizesEXT(VkDevice device, VkAccelerationStructureBuildTypeKHR buildType,
                                                    const VkMicromapBuildInfoEXT* pBuildInfo,
                                                    VkMicromapBuildSizesInfoEXT* pSizeInfo, const RecordObject& record_obj) {}
virtual bool PreCallValidateCmdDrawClusterHUAWEI(VkCommandBuffer commandBuffer, uint32_t groupCountX, uint32_t groupCountY,
                                                 uint32_t groupCountZ, ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordCmdDrawClusterHUAWEI(VkCommandBuffer commandBuffer, uint32_t groupCountX, uint32_t groupCountY,
                                               uint32_t groupCountZ, const RecordObject& record_obj) {}
virtual void PostCallRecordCmdDrawClusterHUAWEI(VkCommandBuffer commandBuffer, uint32_t groupCountX, uint32_t groupCountY,
                                                uint32_t groupCountZ, const RecordObject& record_obj) {}
virtual bool PreCallValidateCmdDrawClusterIndirectHUAWEI(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                         ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordCmdDrawClusterIndirectHUAWEI(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                       const RecordObject& record_obj) {}
virtual void PostCallRecordCmdDrawClusterIndirectHUAWEI(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                        const RecordObject& record_obj) {}
virtual bool PreCallValidateSetDeviceMemoryPriorityEXT(VkDevice device, VkDeviceMemory memory, float priority,
                                                       ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordSetDeviceMemoryPriorityEXT(VkDevice device, VkDeviceMemory memory, float priority,
                                                     const RecordObject& record_obj) {}
virtual void PostCallRecordSetDeviceMemoryPriorityEXT(VkDevice device, VkDeviceMemory memory, float priority,
                                                      const RecordObject& record_obj) {}
virtual bool PreCallValidateGetDescriptorSetLayoutHostMappingInfoVALVE(
    VkDevice device, const VkDescriptorSetBindingReferenceVALVE* pBindingReference,
    VkDescriptorSetLayoutHostMappingInfoVALVE* pHostMapping, ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordGetDescriptorSetLayoutHostMappingInfoVALVE(VkDevice device,
                                                                     const VkDescriptorSetBindingReferenceVALVE* pBindingReference,
                                                                     VkDescriptorSetLayoutHostMappingInfoVALVE* pHostMapping,
                                                                     const RecordObject& record_obj) {}
virtual void PostCallRecordGetDescriptorSetLayoutHostMappingInfoVALVE(VkDevice device,
                                                                      const VkDescriptorSetBindingReferenceVALVE* pBindingReference,
                                                                      VkDescriptorSetLayoutHostMappingInfoVALVE* pHostMapping,
                                                                      const RecordObject& record_obj) {}
virtual bool PreCallValidateGetDescriptorSetHostMappingVALVE(VkDevice device, VkDescriptorSet descriptorSet, void** ppData,
                                                             ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordGetDescriptorSetHostMappingVALVE(VkDevice device, VkDescriptorSet descriptorSet, void** ppData,
                                                           const RecordObject& record_obj) {}
virtual void PostCallRecordGetDescriptorSetHostMappingVALVE(VkDevice device, VkDescriptorSet descriptorSet, void** ppData,
                                                            const RecordObject& record_obj) {}
virtual bool PreCallValidateCmdCopyMemoryIndirectNV(VkCommandBuffer commandBuffer, VkDeviceAddress copyBufferAddress,
                                                    uint32_t copyCount, uint32_t stride, ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordCmdCopyMemoryIndirectNV(VkCommandBuffer commandBuffer, VkDeviceAddress copyBufferAddress,
                                                  uint32_t copyCount, uint32_t stride, const RecordObject& record_obj) {}
virtual void PostCallRecordCmdCopyMemoryIndirectNV(VkCommandBuffer commandBuffer, VkDeviceAddress copyBufferAddress,
                                                   uint32_t copyCount, uint32_t stride, const RecordObject& record_obj) {}
virtual bool PreCallValidateCmdCopyMemoryToImageIndirectNV(VkCommandBuffer commandBuffer, VkDeviceAddress copyBufferAddress,
                                                           uint32_t copyCount, uint32_t stride, VkImage dstImage,
                                                           VkImageLayout dstImageLayout,
                                                           const VkImageSubresourceLayers* pImageSubresources,
                                                           ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordCmdCopyMemoryToImageIndirectNV(VkCommandBuffer commandBuffer, VkDeviceAddress copyBufferAddress,
                                                         uint32_t copyCount, uint32_t stride, VkImage dstImage,
                                                         VkImageLayout dstImageLayout,
                                                         const VkImageSubresourceLayers* pImageSubresources,
                                                         const RecordObject& record_obj) {}
virtual void PostCallRecordCmdCopyMemoryToImageIndirectNV(VkCommandBuffer commandBuffer, VkDeviceAddress copyBufferAddress,
                                                          uint32_t copyCount, uint32_t stride, VkImage dstImage,
                                                          VkImageLayout dstImageLayout,
                                                          const VkImageSubresourceLayers* pImageSubresources,
                                                          const RecordObject& record_obj) {}
virtual bool PreCallValidateCmdDecompressMemoryNV(VkCommandBuffer commandBuffer, uint32_t decompressRegionCount,
                                                  const VkDecompressMemoryRegionNV* pDecompressMemoryRegions,
                                                  ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordCmdDecompressMemoryNV(VkCommandBuffer commandBuffer, uint32_t decompressRegionCount,
                                                const VkDecompressMemoryRegionNV* pDecompressMemoryRegions,
                                                const RecordObject& record_obj) {}
virtual void PostCallRecordCmdDecompressMemoryNV(VkCommandBuffer commandBuffer, uint32_t decompressRegionCount,
                                                 const VkDecompressMemoryRegionNV* pDecompressMemoryRegions,
                                                 const RecordObject& record_obj) {}
virtual bool PreCallValidateCmdDecompressMemoryIndirectCountNV(VkCommandBuffer commandBuffer,
                                                               VkDeviceAddress indirectCommandsAddress,
                                                               VkDeviceAddress indirectCommandsCountAddress, uint32_t stride,
                                                               ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordCmdDecompressMemoryIndirectCountNV(VkCommandBuffer commandBuffer, VkDeviceAddress indirectCommandsAddress,
                                                             VkDeviceAddress indirectCommandsCountAddress, uint32_t stride,
                                                             const RecordObject& record_obj) {}
virtual void PostCallRecordCmdDecompressMemoryIndirectCountNV(VkCommandBuffer commandBuffer,
                                                              VkDeviceAddress indirectCommandsAddress,
                                                              VkDeviceAddress indirectCommandsCountAddress, uint32_t stride,
                                                              const RecordObject& record_obj) {}
virtual bool PreCallValidateGetPipelineIndirectMemoryRequirementsNV(VkDevice device, const VkComputePipelineCreateInfo* pCreateInfo,
                                                                    VkMemoryRequirements2* pMemoryRequirements,
                                                                    ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordGetPipelineIndirectMemoryRequirementsNV(VkDevice device, const VkComputePipelineCreateInfo* pCreateInfo,
                                                                  VkMemoryRequirements2* pMemoryRequirements,
                                                                  const RecordObject& record_obj) {}
virtual void PostCallRecordGetPipelineIndirectMemoryRequirementsNV(VkDevice device, const VkComputePipelineCreateInfo* pCreateInfo,
                                                                   VkMemoryRequirements2* pMemoryRequirements,
                                                                   const RecordObject& record_obj) {}
virtual bool PreCallValidateCmdUpdatePipelineIndirectBufferNV(VkCommandBuffer commandBuffer, VkPipelineBindPoint pipelineBindPoint,
                                                              VkPipeline pipeline, ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordCmdUpdatePipelineIndirectBufferNV(VkCommandBuffer commandBuffer, VkPipelineBindPoint pipelineBindPoint,
                                                            VkPipeline pipeline, const RecordObject& record_obj) {}
virtual void PostCallRecordCmdUpdatePipelineIndirectBufferNV(VkCommandBuffer commandBuffer, VkPipelineBindPoint pipelineBindPoint,
                                                             VkPipeline pipeline, const RecordObject& record_obj) {}
virtual bool PreCallValidateGetPipelineIndirectDeviceAddressNV(VkDevice device, const VkPipelineIndirectDeviceAddressInfoNV* pInfo,
                                                               ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordGetPipelineIndirectDeviceAddressNV(VkDevice device, const VkPipelineIndirectDeviceAddressInfoNV* pInfo,
                                                             const RecordObject& record_obj) {}
virtual void PostCallRecordGetPipelineIndirectDeviceAddressNV(VkDevice device, const VkPipelineIndirectDeviceAddressInfoNV* pInfo,
                                                              const RecordObject& record_obj) {}
virtual bool PreCallValidateCmdSetDepthClampEnableEXT(VkCommandBuffer commandBuffer, VkBool32 depthClampEnable,
                                                      ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordCmdSetDepthClampEnableEXT(VkCommandBuffer commandBuffer, VkBool32 depthClampEnable,
                                                    const RecordObject& record_obj) {}
virtual void PostCallRecordCmdSetDepthClampEnableEXT(VkCommandBuffer commandBuffer, VkBool32 depthClampEnable,
                                                     const RecordObject& record_obj) {}
virtual bool PreCallValidateCmdSetPolygonModeEXT(VkCommandBuffer commandBuffer, VkPolygonMode polygonMode,
                                                 ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordCmdSetPolygonModeEXT(VkCommandBuffer commandBuffer, VkPolygonMode polygonMode,
                                               const RecordObject& record_obj) {}
virtual void PostCallRecordCmdSetPolygonModeEXT(VkCommandBuffer commandBuffer, VkPolygonMode polygonMode,
                                                const RecordObject& record_obj) {}
virtual bool PreCallValidateCmdSetRasterizationSamplesEXT(VkCommandBuffer commandBuffer, VkSampleCountFlagBits rasterizationSamples,
                                                          ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordCmdSetRasterizationSamplesEXT(VkCommandBuffer commandBuffer, VkSampleCountFlagBits rasterizationSamples,
                                                        const RecordObject& record_obj) {}
virtual void PostCallRecordCmdSetRasterizationSamplesEXT(VkCommandBuffer commandBuffer, VkSampleCountFlagBits rasterizationSamples,
                                                         const RecordObject& record_obj) {}
virtual bool PreCallValidateCmdSetSampleMaskEXT(VkCommandBuffer commandBuffer, VkSampleCountFlagBits samples,
                                                const VkSampleMask* pSampleMask, ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordCmdSetSampleMaskEXT(VkCommandBuffer commandBuffer, VkSampleCountFlagBits samples,
                                              const VkSampleMask* pSampleMask, const RecordObject& record_obj) {}
virtual void PostCallRecordCmdSetSampleMaskEXT(VkCommandBuffer commandBuffer, VkSampleCountFlagBits samples,
                                               const VkSampleMask* pSampleMask, const RecordObject& record_obj) {}
virtual bool PreCallValidateCmdSetAlphaToCoverageEnableEXT(VkCommandBuffer commandBuffer, VkBool32 alphaToCoverageEnable,
                                                           ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordCmdSetAlphaToCoverageEnableEXT(VkCommandBuffer commandBuffer, VkBool32 alphaToCoverageEnable,
                                                         const RecordObject& record_obj) {}
virtual void PostCallRecordCmdSetAlphaToCoverageEnableEXT(VkCommandBuffer commandBuffer, VkBool32 alphaToCoverageEnable,
                                                          const RecordObject& record_obj) {}
virtual bool PreCallValidateCmdSetAlphaToOneEnableEXT(VkCommandBuffer commandBuffer, VkBool32 alphaToOneEnable,
                                                      ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordCmdSetAlphaToOneEnableEXT(VkCommandBuffer commandBuffer, VkBool32 alphaToOneEnable,
                                                    const RecordObject& record_obj) {}
virtual void PostCallRecordCmdSetAlphaToOneEnableEXT(VkCommandBuffer commandBuffer, VkBool32 alphaToOneEnable,
                                                     const RecordObject& record_obj) {}
virtual bool PreCallValidateCmdSetLogicOpEnableEXT(VkCommandBuffer commandBuffer, VkBool32 logicOpEnable,
                                                   ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordCmdSetLogicOpEnableEXT(VkCommandBuffer commandBuffer, VkBool32 logicOpEnable,
                                                 const RecordObject& record_obj) {}
virtual void PostCallRecordCmdSetLogicOpEnableEXT(VkCommandBuffer commandBuffer, VkBool32 logicOpEnable,
                                                  const RecordObject& record_obj) {}
virtual bool PreCallValidateCmdSetColorBlendEnableEXT(VkCommandBuffer commandBuffer, uint32_t firstAttachment,
                                                      uint32_t attachmentCount, const VkBool32* pColorBlendEnables,
                                                      ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordCmdSetColorBlendEnableEXT(VkCommandBuffer commandBuffer, uint32_t firstAttachment,
                                                    uint32_t attachmentCount, const VkBool32* pColorBlendEnables,
                                                    const RecordObject& record_obj) {}
virtual void PostCallRecordCmdSetColorBlendEnableEXT(VkCommandBuffer commandBuffer, uint32_t firstAttachment,
                                                     uint32_t attachmentCount, const VkBool32* pColorBlendEnables,
                                                     const RecordObject& record_obj) {}
virtual bool PreCallValidateCmdSetColorBlendEquationEXT(VkCommandBuffer commandBuffer, uint32_t firstAttachment,
                                                        uint32_t attachmentCount,
                                                        const VkColorBlendEquationEXT* pColorBlendEquations,
                                                        ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordCmdSetColorBlendEquationEXT(VkCommandBuffer commandBuffer, uint32_t firstAttachment,
                                                      uint32_t attachmentCount, const VkColorBlendEquationEXT* pColorBlendEquations,
                                                      const RecordObject& record_obj) {}
virtual void PostCallRecordCmdSetColorBlendEquationEXT(VkCommandBuffer commandBuffer, uint32_t firstAttachment,
                                                       uint32_t attachmentCount,
                                                       const VkColorBlendEquationEXT* pColorBlendEquations,
                                                       const RecordObject& record_obj) {}
virtual bool PreCallValidateCmdSetColorWriteMaskEXT(VkCommandBuffer commandBuffer, uint32_t firstAttachment,
                                                    uint32_t attachmentCount, const VkColorComponentFlags* pColorWriteMasks,
                                                    ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordCmdSetColorWriteMaskEXT(VkCommandBuffer commandBuffer, uint32_t firstAttachment, uint32_t attachmentCount,
                                                  const VkColorComponentFlags* pColorWriteMasks, const RecordObject& record_obj) {}
virtual void PostCallRecordCmdSetColorWriteMaskEXT(VkCommandBuffer commandBuffer, uint32_t firstAttachment,
                                                   uint32_t attachmentCount, const VkColorComponentFlags* pColorWriteMasks,
                                                   const RecordObject& record_obj) {}
virtual bool PreCallValidateCmdSetTessellationDomainOriginEXT(VkCommandBuffer commandBuffer,
                                                              VkTessellationDomainOrigin domainOrigin,
                                                              ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordCmdSetTessellationDomainOriginEXT(VkCommandBuffer commandBuffer, VkTessellationDomainOrigin domainOrigin,
                                                            const RecordObject& record_obj) {}
virtual void PostCallRecordCmdSetTessellationDomainOriginEXT(VkCommandBuffer commandBuffer, VkTessellationDomainOrigin domainOrigin,
                                                             const RecordObject& record_obj) {}
virtual bool PreCallValidateCmdSetRasterizationStreamEXT(VkCommandBuffer commandBuffer, uint32_t rasterizationStream,
                                                         ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordCmdSetRasterizationStreamEXT(VkCommandBuffer commandBuffer, uint32_t rasterizationStream,
                                                       const RecordObject& record_obj) {}
virtual void PostCallRecordCmdSetRasterizationStreamEXT(VkCommandBuffer commandBuffer, uint32_t rasterizationStream,
                                                        const RecordObject& record_obj) {}
virtual bool PreCallValidateCmdSetConservativeRasterizationModeEXT(VkCommandBuffer commandBuffer,
                                                                   VkConservativeRasterizationModeEXT conservativeRasterizationMode,
                                                                   ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordCmdSetConservativeRasterizationModeEXT(VkCommandBuffer commandBuffer,
                                                                 VkConservativeRasterizationModeEXT conservativeRasterizationMode,
                                                                 const RecordObject& record_obj) {}
virtual void PostCallRecordCmdSetConservativeRasterizationModeEXT(VkCommandBuffer commandBuffer,
                                                                  VkConservativeRasterizationModeEXT conservativeRasterizationMode,
                                                                  const RecordObject& record_obj) {}
virtual bool PreCallValidateCmdSetExtraPrimitiveOverestimationSizeEXT(VkCommandBuffer commandBuffer,
                                                                      float extraPrimitiveOverestimationSize,
                                                                      ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordCmdSetExtraPrimitiveOverestimationSizeEXT(VkCommandBuffer commandBuffer,
                                                                    float extraPrimitiveOverestimationSize,
                                                                    const RecordObject& record_obj) {}
virtual void PostCallRecordCmdSetExtraPrimitiveOverestimationSizeEXT(VkCommandBuffer commandBuffer,
                                                                     float extraPrimitiveOverestimationSize,
                                                                     const RecordObject& record_obj) {}
virtual bool PreCallValidateCmdSetDepthClipEnableEXT(VkCommandBuffer commandBuffer, VkBool32 depthClipEnable,
                                                     ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordCmdSetDepthClipEnableEXT(VkCommandBuffer commandBuffer, VkBool32 depthClipEnable,
                                                   const RecordObject& record_obj) {}
virtual void PostCallRecordCmdSetDepthClipEnableEXT(VkCommandBuffer commandBuffer, VkBool32 depthClipEnable,
                                                    const RecordObject& record_obj) {}
virtual bool PreCallValidateCmdSetSampleLocationsEnableEXT(VkCommandBuffer commandBuffer, VkBool32 sampleLocationsEnable,
                                                           ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordCmdSetSampleLocationsEnableEXT(VkCommandBuffer commandBuffer, VkBool32 sampleLocationsEnable,
                                                         const RecordObject& record_obj) {}
virtual void PostCallRecordCmdSetSampleLocationsEnableEXT(VkCommandBuffer commandBuffer, VkBool32 sampleLocationsEnable,
                                                          const RecordObject& record_obj) {}
virtual bool PreCallValidateCmdSetColorBlendAdvancedEXT(VkCommandBuffer commandBuffer, uint32_t firstAttachment,
                                                        uint32_t attachmentCount,
                                                        const VkColorBlendAdvancedEXT* pColorBlendAdvanced,
                                                        ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordCmdSetColorBlendAdvancedEXT(VkCommandBuffer commandBuffer, uint32_t firstAttachment,
                                                      uint32_t attachmentCount, const VkColorBlendAdvancedEXT* pColorBlendAdvanced,
                                                      const RecordObject& record_obj) {}
virtual void PostCallRecordCmdSetColorBlendAdvancedEXT(VkCommandBuffer commandBuffer, uint32_t firstAttachment,
                                                       uint32_t attachmentCount, const VkColorBlendAdvancedEXT* pColorBlendAdvanced,
                                                       const RecordObject& record_obj) {}
virtual bool PreCallValidateCmdSetProvokingVertexModeEXT(VkCommandBuffer commandBuffer,
                                                         VkProvokingVertexModeEXT provokingVertexMode,
                                                         ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordCmdSetProvokingVertexModeEXT(VkCommandBuffer commandBuffer, VkProvokingVertexModeEXT provokingVertexMode,
                                                       const RecordObject& record_obj) {}
virtual void PostCallRecordCmdSetProvokingVertexModeEXT(VkCommandBuffer commandBuffer, VkProvokingVertexModeEXT provokingVertexMode,
                                                        const RecordObject& record_obj) {}
virtual bool PreCallValidateCmdSetLineRasterizationModeEXT(VkCommandBuffer commandBuffer,
                                                           VkLineRasterizationModeEXT lineRasterizationMode,
                                                           ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordCmdSetLineRasterizationModeEXT(VkCommandBuffer commandBuffer,
                                                         VkLineRasterizationModeEXT lineRasterizationMode,
                                                         const RecordObject& record_obj) {}
virtual void PostCallRecordCmdSetLineRasterizationModeEXT(VkCommandBuffer commandBuffer,
                                                          VkLineRasterizationModeEXT lineRasterizationMode,
                                                          const RecordObject& record_obj) {}
virtual bool PreCallValidateCmdSetLineStippleEnableEXT(VkCommandBuffer commandBuffer, VkBool32 stippledLineEnable,
                                                       ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordCmdSetLineStippleEnableEXT(VkCommandBuffer commandBuffer, VkBool32 stippledLineEnable,
                                                     const RecordObject& record_obj) {}
virtual void PostCallRecordCmdSetLineStippleEnableEXT(VkCommandBuffer commandBuffer, VkBool32 stippledLineEnable,
                                                      const RecordObject& record_obj) {}
virtual bool PreCallValidateCmdSetDepthClipNegativeOneToOneEXT(VkCommandBuffer commandBuffer, VkBool32 negativeOneToOne,
                                                               ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordCmdSetDepthClipNegativeOneToOneEXT(VkCommandBuffer commandBuffer, VkBool32 negativeOneToOne,
                                                             const RecordObject& record_obj) {}
virtual void PostCallRecordCmdSetDepthClipNegativeOneToOneEXT(VkCommandBuffer commandBuffer, VkBool32 negativeOneToOne,
                                                              const RecordObject& record_obj) {}
virtual bool PreCallValidateCmdSetViewportWScalingEnableNV(VkCommandBuffer commandBuffer, VkBool32 viewportWScalingEnable,
                                                           ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordCmdSetViewportWScalingEnableNV(VkCommandBuffer commandBuffer, VkBool32 viewportWScalingEnable,
                                                         const RecordObject& record_obj) {}
virtual void PostCallRecordCmdSetViewportWScalingEnableNV(VkCommandBuffer commandBuffer, VkBool32 viewportWScalingEnable,
                                                          const RecordObject& record_obj) {}
virtual bool PreCallValidateCmdSetViewportSwizzleNV(VkCommandBuffer commandBuffer, uint32_t firstViewport, uint32_t viewportCount,
                                                    const VkViewportSwizzleNV* pViewportSwizzles, ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordCmdSetViewportSwizzleNV(VkCommandBuffer commandBuffer, uint32_t firstViewport, uint32_t viewportCount,
                                                  const VkViewportSwizzleNV* pViewportSwizzles, const RecordObject& record_obj) {}
virtual void PostCallRecordCmdSetViewportSwizzleNV(VkCommandBuffer commandBuffer, uint32_t firstViewport, uint32_t viewportCount,
                                                   const VkViewportSwizzleNV* pViewportSwizzles, const RecordObject& record_obj) {}
virtual bool PreCallValidateCmdSetCoverageToColorEnableNV(VkCommandBuffer commandBuffer, VkBool32 coverageToColorEnable,
                                                          ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordCmdSetCoverageToColorEnableNV(VkCommandBuffer commandBuffer, VkBool32 coverageToColorEnable,
                                                        const RecordObject& record_obj) {}
virtual void PostCallRecordCmdSetCoverageToColorEnableNV(VkCommandBuffer commandBuffer, VkBool32 coverageToColorEnable,
                                                         const RecordObject& record_obj) {}
virtual bool PreCallValidateCmdSetCoverageToColorLocationNV(VkCommandBuffer commandBuffer, uint32_t coverageToColorLocation,
                                                            ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordCmdSetCoverageToColorLocationNV(VkCommandBuffer commandBuffer, uint32_t coverageToColorLocation,
                                                          const RecordObject& record_obj) {}
virtual void PostCallRecordCmdSetCoverageToColorLocationNV(VkCommandBuffer commandBuffer, uint32_t coverageToColorLocation,
                                                           const RecordObject& record_obj) {}
virtual bool PreCallValidateCmdSetCoverageModulationModeNV(VkCommandBuffer commandBuffer,
                                                           VkCoverageModulationModeNV coverageModulationMode,
                                                           ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordCmdSetCoverageModulationModeNV(VkCommandBuffer commandBuffer,
                                                         VkCoverageModulationModeNV coverageModulationMode,
                                                         const RecordObject& record_obj) {}
virtual void PostCallRecordCmdSetCoverageModulationModeNV(VkCommandBuffer commandBuffer,
                                                          VkCoverageModulationModeNV coverageModulationMode,
                                                          const RecordObject& record_obj) {}
virtual bool PreCallValidateCmdSetCoverageModulationTableEnableNV(VkCommandBuffer commandBuffer,
                                                                  VkBool32 coverageModulationTableEnable,
                                                                  ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordCmdSetCoverageModulationTableEnableNV(VkCommandBuffer commandBuffer,
                                                                VkBool32 coverageModulationTableEnable,
                                                                const RecordObject& record_obj) {}
virtual void PostCallRecordCmdSetCoverageModulationTableEnableNV(VkCommandBuffer commandBuffer,
                                                                 VkBool32 coverageModulationTableEnable,
                                                                 const RecordObject& record_obj) {}
virtual bool PreCallValidateCmdSetCoverageModulationTableNV(VkCommandBuffer commandBuffer, uint32_t coverageModulationTableCount,
                                                            const float* pCoverageModulationTable, ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordCmdSetCoverageModulationTableNV(VkCommandBuffer commandBuffer, uint32_t coverageModulationTableCount,
                                                          const float* pCoverageModulationTable, const RecordObject& record_obj) {}
virtual void PostCallRecordCmdSetCoverageModulationTableNV(VkCommandBuffer commandBuffer, uint32_t coverageModulationTableCount,
                                                           const float* pCoverageModulationTable, const RecordObject& record_obj) {}
virtual bool PreCallValidateCmdSetShadingRateImageEnableNV(VkCommandBuffer commandBuffer, VkBool32 shadingRateImageEnable,
                                                           ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordCmdSetShadingRateImageEnableNV(VkCommandBuffer commandBuffer, VkBool32 shadingRateImageEnable,
                                                         const RecordObject& record_obj) {}
virtual void PostCallRecordCmdSetShadingRateImageEnableNV(VkCommandBuffer commandBuffer, VkBool32 shadingRateImageEnable,
                                                          const RecordObject& record_obj) {}
virtual bool PreCallValidateCmdSetRepresentativeFragmentTestEnableNV(VkCommandBuffer commandBuffer,
                                                                     VkBool32 representativeFragmentTestEnable,
                                                                     ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordCmdSetRepresentativeFragmentTestEnableNV(VkCommandBuffer commandBuffer,
                                                                   VkBool32 representativeFragmentTestEnable,
                                                                   const RecordObject& record_obj) {}
virtual void PostCallRecordCmdSetRepresentativeFragmentTestEnableNV(VkCommandBuffer commandBuffer,
                                                                    VkBool32 representativeFragmentTestEnable,
                                                                    const RecordObject& record_obj) {}
virtual bool PreCallValidateCmdSetCoverageReductionModeNV(VkCommandBuffer commandBuffer,
                                                          VkCoverageReductionModeNV coverageReductionMode,
                                                          ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordCmdSetCoverageReductionModeNV(VkCommandBuffer commandBuffer,
                                                        VkCoverageReductionModeNV coverageReductionMode,
                                                        const RecordObject& record_obj) {}
virtual void PostCallRecordCmdSetCoverageReductionModeNV(VkCommandBuffer commandBuffer,
                                                         VkCoverageReductionModeNV coverageReductionMode,
                                                         const RecordObject& record_obj) {}
virtual bool PreCallValidateGetShaderModuleIdentifierEXT(VkDevice device, VkShaderModule shaderModule,
                                                         VkShaderModuleIdentifierEXT* pIdentifier, ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordGetShaderModuleIdentifierEXT(VkDevice device, VkShaderModule shaderModule,
                                                       VkShaderModuleIdentifierEXT* pIdentifier, const RecordObject& record_obj) {}
virtual void PostCallRecordGetShaderModuleIdentifierEXT(VkDevice device, VkShaderModule shaderModule,
                                                        VkShaderModuleIdentifierEXT* pIdentifier, const RecordObject& record_obj) {}
virtual bool PreCallValidateGetShaderModuleCreateInfoIdentifierEXT(VkDevice device, const VkShaderModuleCreateInfo* pCreateInfo,
                                                                   VkShaderModuleIdentifierEXT* pIdentifier,
                                                                   ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordGetShaderModuleCreateInfoIdentifierEXT(VkDevice device, const VkShaderModuleCreateInfo* pCreateInfo,
                                                                 VkShaderModuleIdentifierEXT* pIdentifier,
                                                                 const RecordObject& record_obj) {}
virtual void PostCallRecordGetShaderModuleCreateInfoIdentifierEXT(VkDevice device, const VkShaderModuleCreateInfo* pCreateInfo,
                                                                  VkShaderModuleIdentifierEXT* pIdentifier,
                                                                  const RecordObject& record_obj) {}
virtual bool PreCallValidateCreateOpticalFlowSessionNV(VkDevice device, const VkOpticalFlowSessionCreateInfoNV* pCreateInfo,
                                                       const VkAllocationCallbacks* pAllocator, VkOpticalFlowSessionNV* pSession,
                                                       ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordCreateOpticalFlowSessionNV(VkDevice device, const VkOpticalFlowSessionCreateInfoNV* pCreateInfo,
                                                     const VkAllocationCallbacks* pAllocator, VkOpticalFlowSessionNV* pSession,
                                                     const RecordObject& record_obj) {}
virtual void PostCallRecordCreateOpticalFlowSessionNV(VkDevice device, const VkOpticalFlowSessionCreateInfoNV* pCreateInfo,
                                                      const VkAllocationCallbacks* pAllocator, VkOpticalFlowSessionNV* pSession,
                                                      const RecordObject& record_obj) {}
virtual bool PreCallValidateDestroyOpticalFlowSessionNV(VkDevice device, VkOpticalFlowSessionNV session,
                                                        const VkAllocationCallbacks* pAllocator, ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordDestroyOpticalFlowSessionNV(VkDevice device, VkOpticalFlowSessionNV session,
                                                      const VkAllocationCallbacks* pAllocator, const RecordObject& record_obj) {}
virtual void PostCallRecordDestroyOpticalFlowSessionNV(VkDevice device, VkOpticalFlowSessionNV session,
                                                       const VkAllocationCallbacks* pAllocator, const RecordObject& record_obj) {}
virtual bool PreCallValidateBindOpticalFlowSessionImageNV(VkDevice device, VkOpticalFlowSessionNV session,
                                                          VkOpticalFlowSessionBindingPointNV bindingPoint, VkImageView view,
                                                          VkImageLayout layout, ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordBindOpticalFlowSessionImageNV(VkDevice device, VkOpticalFlowSessionNV session,
                                                        VkOpticalFlowSessionBindingPointNV bindingPoint, VkImageView view,
                                                        VkImageLayout layout, const RecordObject& record_obj) {}
virtual void PostCallRecordBindOpticalFlowSessionImageNV(VkDevice device, VkOpticalFlowSessionNV session,
                                                         VkOpticalFlowSessionBindingPointNV bindingPoint, VkImageView view,
                                                         VkImageLayout layout, const RecordObject& record_obj) {}
virtual bool PreCallValidateCmdOpticalFlowExecuteNV(VkCommandBuffer commandBuffer, VkOpticalFlowSessionNV session,
                                                    const VkOpticalFlowExecuteInfoNV* pExecuteInfo, ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordCmdOpticalFlowExecuteNV(VkCommandBuffer commandBuffer, VkOpticalFlowSessionNV session,
                                                  const VkOpticalFlowExecuteInfoNV* pExecuteInfo, const RecordObject& record_obj) {}
virtual void PostCallRecordCmdOpticalFlowExecuteNV(VkCommandBuffer commandBuffer, VkOpticalFlowSessionNV session,
                                                   const VkOpticalFlowExecuteInfoNV* pExecuteInfo, const RecordObject& record_obj) {
}
virtual bool PreCallValidateAntiLagUpdateAMD(VkDevice device, const VkAntiLagDataAMD* pData, ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordAntiLagUpdateAMD(VkDevice device, const VkAntiLagDataAMD* pData, const RecordObject& record_obj) {}
virtual void PostCallRecordAntiLagUpdateAMD(VkDevice device, const VkAntiLagDataAMD* pData, const RecordObject& record_obj) {}
virtual bool PreCallValidateCreateShadersEXT(VkDevice device, uint32_t createInfoCount, const VkShaderCreateInfoEXT* pCreateInfos,
                                             const VkAllocationCallbacks* pAllocator, VkShaderEXT* pShaders,
                                             ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordCreateShadersEXT(VkDevice device, uint32_t createInfoCount, const VkShaderCreateInfoEXT* pCreateInfos,
                                           const VkAllocationCallbacks* pAllocator, VkShaderEXT* pShaders,
                                           const RecordObject& record_obj) {}
virtual void PostCallRecordCreateShadersEXT(VkDevice device, uint32_t createInfoCount, const VkShaderCreateInfoEXT* pCreateInfos,
                                            const VkAllocationCallbacks* pAllocator, VkShaderEXT* pShaders,
                                            const RecordObject& record_obj) {}
virtual bool PreCallValidateDestroyShaderEXT(VkDevice device, VkShaderEXT shader, const VkAllocationCallbacks* pAllocator,
                                             ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordDestroyShaderEXT(VkDevice device, VkShaderEXT shader, const VkAllocationCallbacks* pAllocator,
                                           const RecordObject& record_obj) {}
virtual void PostCallRecordDestroyShaderEXT(VkDevice device, VkShaderEXT shader, const VkAllocationCallbacks* pAllocator,
                                            const RecordObject& record_obj) {}
virtual bool PreCallValidateGetShaderBinaryDataEXT(VkDevice device, VkShaderEXT shader, size_t* pDataSize, void* pData,
                                                   ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordGetShaderBinaryDataEXT(VkDevice device, VkShaderEXT shader, size_t* pDataSize, void* pData,
                                                 const RecordObject& record_obj) {}
virtual void PostCallRecordGetShaderBinaryDataEXT(VkDevice device, VkShaderEXT shader, size_t* pDataSize, void* pData,
                                                  const RecordObject& record_obj) {}
virtual bool PreCallValidateCmdBindShadersEXT(VkCommandBuffer commandBuffer, uint32_t stageCount,
                                              const VkShaderStageFlagBits* pStages, const VkShaderEXT* pShaders,
                                              ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordCmdBindShadersEXT(VkCommandBuffer commandBuffer, uint32_t stageCount,
                                            const VkShaderStageFlagBits* pStages, const VkShaderEXT* pShaders,
                                            const RecordObject& record_obj) {}
virtual void PostCallRecordCmdBindShadersEXT(VkCommandBuffer commandBuffer, uint32_t stageCount,
                                             const VkShaderStageFlagBits* pStages, const VkShaderEXT* pShaders,
                                             const RecordObject& record_obj) {}
virtual bool PreCallValidateCmdSetDepthClampRangeEXT(VkCommandBuffer commandBuffer, VkDepthClampModeEXT depthClampMode,
                                                     const VkDepthClampRangeEXT* pDepthClampRange, ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordCmdSetDepthClampRangeEXT(VkCommandBuffer commandBuffer, VkDepthClampModeEXT depthClampMode,
                                                   const VkDepthClampRangeEXT* pDepthClampRange, const RecordObject& record_obj) {}
virtual void PostCallRecordCmdSetDepthClampRangeEXT(VkCommandBuffer commandBuffer, VkDepthClampModeEXT depthClampMode,
                                                    const VkDepthClampRangeEXT* pDepthClampRange, const RecordObject& record_obj) {}
virtual bool PreCallValidateGetFramebufferTilePropertiesQCOM(VkDevice device, VkFramebuffer framebuffer, uint32_t* pPropertiesCount,
                                                             VkTilePropertiesQCOM* pProperties, ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordGetFramebufferTilePropertiesQCOM(VkDevice device, VkFramebuffer framebuffer, uint32_t* pPropertiesCount,
                                                           VkTilePropertiesQCOM* pProperties, const RecordObject& record_obj) {}
virtual void PostCallRecordGetFramebufferTilePropertiesQCOM(VkDevice device, VkFramebuffer framebuffer, uint32_t* pPropertiesCount,
                                                            VkTilePropertiesQCOM* pProperties, const RecordObject& record_obj) {}
virtual bool PreCallValidateGetDynamicRenderingTilePropertiesQCOM(VkDevice device, const VkRenderingInfo* pRenderingInfo,
                                                                  VkTilePropertiesQCOM* pProperties, ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordGetDynamicRenderingTilePropertiesQCOM(VkDevice device, const VkRenderingInfo* pRenderingInfo,
                                                                VkTilePropertiesQCOM* pProperties, const RecordObject& record_obj) {
}
virtual void PostCallRecordGetDynamicRenderingTilePropertiesQCOM(VkDevice device, const VkRenderingInfo* pRenderingInfo,
                                                                 VkTilePropertiesQCOM* pProperties,
                                                                 const RecordObject& record_obj) {}
virtual bool PreCallValidateConvertCooperativeVectorMatrixNV(VkDevice device, const VkConvertCooperativeVectorMatrixInfoNV* pInfo,
                                                             ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordConvertCooperativeVectorMatrixNV(VkDevice device, const VkConvertCooperativeVectorMatrixInfoNV* pInfo,
                                                           const RecordObject& record_obj) {}
virtual void PostCallRecordConvertCooperativeVectorMatrixNV(VkDevice device, const VkConvertCooperativeVectorMatrixInfoNV* pInfo,
                                                            const RecordObject& record_obj) {}
virtual bool PreCallValidateCmdConvertCooperativeVectorMatrixNV(VkCommandBuffer commandBuffer, uint32_t infoCount,
                                                                const VkConvertCooperativeVectorMatrixInfoNV* pInfos,
                                                                ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordCmdConvertCooperativeVectorMatrixNV(VkCommandBuffer commandBuffer, uint32_t infoCount,
                                                              const VkConvertCooperativeVectorMatrixInfoNV* pInfos,
                                                              const RecordObject& record_obj) {}
virtual void PostCallRecordCmdConvertCooperativeVectorMatrixNV(VkCommandBuffer commandBuffer, uint32_t infoCount,
                                                               const VkConvertCooperativeVectorMatrixInfoNV* pInfos,
                                                               const RecordObject& record_obj) {}
virtual bool PreCallValidateSetLatencySleepModeNV(VkDevice device, VkSwapchainKHR swapchain,
                                                  const VkLatencySleepModeInfoNV* pSleepModeInfo, ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordSetLatencySleepModeNV(VkDevice device, VkSwapchainKHR swapchain,
                                                const VkLatencySleepModeInfoNV* pSleepModeInfo, const RecordObject& record_obj) {}
virtual void PostCallRecordSetLatencySleepModeNV(VkDevice device, VkSwapchainKHR swapchain,
                                                 const VkLatencySleepModeInfoNV* pSleepModeInfo, const RecordObject& record_obj) {}
virtual bool PreCallValidateLatencySleepNV(VkDevice device, VkSwapchainKHR swapchain, const VkLatencySleepInfoNV* pSleepInfo,
                                           ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordLatencySleepNV(VkDevice device, VkSwapchainKHR swapchain, const VkLatencySleepInfoNV* pSleepInfo,
                                         const RecordObject& record_obj) {}
virtual void PostCallRecordLatencySleepNV(VkDevice device, VkSwapchainKHR swapchain, const VkLatencySleepInfoNV* pSleepInfo,
                                          const RecordObject& record_obj) {}
virtual bool PreCallValidateSetLatencyMarkerNV(VkDevice device, VkSwapchainKHR swapchain,
                                               const VkSetLatencyMarkerInfoNV* pLatencyMarkerInfo, ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordSetLatencyMarkerNV(VkDevice device, VkSwapchainKHR swapchain,
                                             const VkSetLatencyMarkerInfoNV* pLatencyMarkerInfo, const RecordObject& record_obj) {}
virtual void PostCallRecordSetLatencyMarkerNV(VkDevice device, VkSwapchainKHR swapchain,
                                              const VkSetLatencyMarkerInfoNV* pLatencyMarkerInfo, const RecordObject& record_obj) {}
virtual bool PreCallValidateGetLatencyTimingsNV(VkDevice device, VkSwapchainKHR swapchain,
                                                VkGetLatencyMarkerInfoNV* pLatencyMarkerInfo, ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordGetLatencyTimingsNV(VkDevice device, VkSwapchainKHR swapchain,
                                              VkGetLatencyMarkerInfoNV* pLatencyMarkerInfo, const RecordObject& record_obj) {}
virtual void PostCallRecordGetLatencyTimingsNV(VkDevice device, VkSwapchainKHR swapchain,
                                               VkGetLatencyMarkerInfoNV* pLatencyMarkerInfo, const RecordObject& record_obj) {}
virtual bool PreCallValidateQueueNotifyOutOfBandNV(VkQueue queue, const VkOutOfBandQueueTypeInfoNV* pQueueTypeInfo,
                                                   ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordQueueNotifyOutOfBandNV(VkQueue queue, const VkOutOfBandQueueTypeInfoNV* pQueueTypeInfo,
                                                 const RecordObject& record_obj) {}
virtual void PostCallRecordQueueNotifyOutOfBandNV(VkQueue queue, const VkOutOfBandQueueTypeInfoNV* pQueueTypeInfo,
                                                  const RecordObject& record_obj) {}
virtual bool PreCallValidateCmdSetAttachmentFeedbackLoopEnableEXT(VkCommandBuffer commandBuffer, VkImageAspectFlags aspectMask,
                                                                  ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordCmdSetAttachmentFeedbackLoopEnableEXT(VkCommandBuffer commandBuffer, VkImageAspectFlags aspectMask,
                                                                const RecordObject& record_obj) {}
virtual void PostCallRecordCmdSetAttachmentFeedbackLoopEnableEXT(VkCommandBuffer commandBuffer, VkImageAspectFlags aspectMask,
                                                                 const RecordObject& record_obj) {}
#ifdef VK_USE_PLATFORM_SCREEN_QNX
virtual bool PreCallValidateGetScreenBufferPropertiesQNX(VkDevice device, const struct _screen_buffer* buffer,
                                                         VkScreenBufferPropertiesQNX* pProperties, ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordGetScreenBufferPropertiesQNX(VkDevice device, const struct _screen_buffer* buffer,
                                                       VkScreenBufferPropertiesQNX* pProperties, const RecordObject& record_obj) {}
virtual void PostCallRecordGetScreenBufferPropertiesQNX(VkDevice device, const struct _screen_buffer* buffer,
                                                        VkScreenBufferPropertiesQNX* pProperties, const RecordObject& record_obj) {}
#endif  // VK_USE_PLATFORM_SCREEN_QNX
virtual bool PreCallValidateGetClusterAccelerationStructureBuildSizesNV(VkDevice device,
                                                                        const VkClusterAccelerationStructureInputInfoNV* pInfo,
                                                                        VkAccelerationStructureBuildSizesInfoKHR* pSizeInfo,
                                                                        ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordGetClusterAccelerationStructureBuildSizesNV(VkDevice device,
                                                                      const VkClusterAccelerationStructureInputInfoNV* pInfo,
                                                                      VkAccelerationStructureBuildSizesInfoKHR* pSizeInfo,
                                                                      const RecordObject& record_obj) {}
virtual void PostCallRecordGetClusterAccelerationStructureBuildSizesNV(VkDevice device,
                                                                       const VkClusterAccelerationStructureInputInfoNV* pInfo,
                                                                       VkAccelerationStructureBuildSizesInfoKHR* pSizeInfo,
                                                                       const RecordObject& record_obj) {}
virtual bool PreCallValidateCmdBuildClusterAccelerationStructureIndirectNV(
    VkCommandBuffer commandBuffer, const VkClusterAccelerationStructureCommandsInfoNV* pCommandInfos,
    ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordCmdBuildClusterAccelerationStructureIndirectNV(
    VkCommandBuffer commandBuffer, const VkClusterAccelerationStructureCommandsInfoNV* pCommandInfos,
    const RecordObject& record_obj) {}
virtual void PostCallRecordCmdBuildClusterAccelerationStructureIndirectNV(
    VkCommandBuffer commandBuffer, const VkClusterAccelerationStructureCommandsInfoNV* pCommandInfos,
    const RecordObject& record_obj) {}
virtual bool PreCallValidateGetPartitionedAccelerationStructuresBuildSizesNV(
    VkDevice device, const VkPartitionedAccelerationStructureInstancesInputNV* pInfo,
    VkAccelerationStructureBuildSizesInfoKHR* pSizeInfo, ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordGetPartitionedAccelerationStructuresBuildSizesNV(
    VkDevice device, const VkPartitionedAccelerationStructureInstancesInputNV* pInfo,
    VkAccelerationStructureBuildSizesInfoKHR* pSizeInfo, const RecordObject& record_obj) {}
virtual void PostCallRecordGetPartitionedAccelerationStructuresBuildSizesNV(
    VkDevice device, const VkPartitionedAccelerationStructureInstancesInputNV* pInfo,
    VkAccelerationStructureBuildSizesInfoKHR* pSizeInfo, const RecordObject& record_obj) {}
virtual bool PreCallValidateCmdBuildPartitionedAccelerationStructuresNV(
    VkCommandBuffer commandBuffer, const VkBuildPartitionedAccelerationStructureInfoNV* pBuildInfo, ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordCmdBuildPartitionedAccelerationStructuresNV(
    VkCommandBuffer commandBuffer, const VkBuildPartitionedAccelerationStructureInfoNV* pBuildInfo,
    const RecordObject& record_obj) {}
virtual void PostCallRecordCmdBuildPartitionedAccelerationStructuresNV(
    VkCommandBuffer commandBuffer, const VkBuildPartitionedAccelerationStructureInfoNV* pBuildInfo,
    const RecordObject& record_obj) {}
virtual bool PreCallValidateGetGeneratedCommandsMemoryRequirementsEXT(VkDevice device,
                                                                      const VkGeneratedCommandsMemoryRequirementsInfoEXT* pInfo,
                                                                      VkMemoryRequirements2* pMemoryRequirements,
                                                                      ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordGetGeneratedCommandsMemoryRequirementsEXT(VkDevice device,
                                                                    const VkGeneratedCommandsMemoryRequirementsInfoEXT* pInfo,
                                                                    VkMemoryRequirements2* pMemoryRequirements,
                                                                    const RecordObject& record_obj) {}
virtual void PostCallRecordGetGeneratedCommandsMemoryRequirementsEXT(VkDevice device,
                                                                     const VkGeneratedCommandsMemoryRequirementsInfoEXT* pInfo,
                                                                     VkMemoryRequirements2* pMemoryRequirements,
                                                                     const RecordObject& record_obj) {}
virtual bool PreCallValidateCmdPreprocessGeneratedCommandsEXT(VkCommandBuffer commandBuffer,
                                                              const VkGeneratedCommandsInfoEXT* pGeneratedCommandsInfo,
                                                              VkCommandBuffer stateCommandBuffer, ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordCmdPreprocessGeneratedCommandsEXT(VkCommandBuffer commandBuffer,
                                                            const VkGeneratedCommandsInfoEXT* pGeneratedCommandsInfo,
                                                            VkCommandBuffer stateCommandBuffer, const RecordObject& record_obj) {}
virtual void PostCallRecordCmdPreprocessGeneratedCommandsEXT(VkCommandBuffer commandBuffer,
                                                             const VkGeneratedCommandsInfoEXT* pGeneratedCommandsInfo,
                                                             VkCommandBuffer stateCommandBuffer, const RecordObject& record_obj) {}
virtual bool PreCallValidateCmdExecuteGeneratedCommandsEXT(VkCommandBuffer commandBuffer, VkBool32 isPreprocessed,
                                                           const VkGeneratedCommandsInfoEXT* pGeneratedCommandsInfo,
                                                           ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordCmdExecuteGeneratedCommandsEXT(VkCommandBuffer commandBuffer, VkBool32 isPreprocessed,
                                                         const VkGeneratedCommandsInfoEXT* pGeneratedCommandsInfo,
                                                         const RecordObject& record_obj) {}
virtual void PostCallRecordCmdExecuteGeneratedCommandsEXT(VkCommandBuffer commandBuffer, VkBool32 isPreprocessed,
                                                          const VkGeneratedCommandsInfoEXT* pGeneratedCommandsInfo,
                                                          const RecordObject& record_obj) {}
virtual bool PreCallValidateCreateIndirectCommandsLayoutEXT(VkDevice device,
                                                            const VkIndirectCommandsLayoutCreateInfoEXT* pCreateInfo,
                                                            const VkAllocationCallbacks* pAllocator,
                                                            VkIndirectCommandsLayoutEXT* pIndirectCommandsLayout,
                                                            ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordCreateIndirectCommandsLayoutEXT(VkDevice device, const VkIndirectCommandsLayoutCreateInfoEXT* pCreateInfo,
                                                          const VkAllocationCallbacks* pAllocator,
                                                          VkIndirectCommandsLayoutEXT* pIndirectCommandsLayout,
                                                          const RecordObject& record_obj) {}
virtual void PostCallRecordCreateIndirectCommandsLayoutEXT(VkDevice device,
                                                           const VkIndirectCommandsLayoutCreateInfoEXT* pCreateInfo,
                                                           const VkAllocationCallbacks* pAllocator,
                                                           VkIndirectCommandsLayoutEXT* pIndirectCommandsLayout,
                                                           const RecordObject& record_obj) {}
virtual bool PreCallValidateDestroyIndirectCommandsLayoutEXT(VkDevice device, VkIndirectCommandsLayoutEXT indirectCommandsLayout,
                                                             const VkAllocationCallbacks* pAllocator,
                                                             ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordDestroyIndirectCommandsLayoutEXT(VkDevice device, VkIndirectCommandsLayoutEXT indirectCommandsLayout,
                                                           const VkAllocationCallbacks* pAllocator,
                                                           const RecordObject& record_obj) {}
virtual void PostCallRecordDestroyIndirectCommandsLayoutEXT(VkDevice device, VkIndirectCommandsLayoutEXT indirectCommandsLayout,
                                                            const VkAllocationCallbacks* pAllocator,
                                                            const RecordObject& record_obj) {}
virtual bool PreCallValidateCreateIndirectExecutionSetEXT(VkDevice device, const VkIndirectExecutionSetCreateInfoEXT* pCreateInfo,
                                                          const VkAllocationCallbacks* pAllocator,
                                                          VkIndirectExecutionSetEXT* pIndirectExecutionSet,
                                                          ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordCreateIndirectExecutionSetEXT(VkDevice device, const VkIndirectExecutionSetCreateInfoEXT* pCreateInfo,
                                                        const VkAllocationCallbacks* pAllocator,
                                                        VkIndirectExecutionSetEXT* pIndirectExecutionSet,
                                                        const RecordObject& record_obj) {}
virtual void PostCallRecordCreateIndirectExecutionSetEXT(VkDevice device, const VkIndirectExecutionSetCreateInfoEXT* pCreateInfo,
                                                         const VkAllocationCallbacks* pAllocator,
                                                         VkIndirectExecutionSetEXT* pIndirectExecutionSet,
                                                         const RecordObject& record_obj) {}
virtual bool PreCallValidateDestroyIndirectExecutionSetEXT(VkDevice device, VkIndirectExecutionSetEXT indirectExecutionSet,
                                                           const VkAllocationCallbacks* pAllocator, ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordDestroyIndirectExecutionSetEXT(VkDevice device, VkIndirectExecutionSetEXT indirectExecutionSet,
                                                         const VkAllocationCallbacks* pAllocator, const RecordObject& record_obj) {}
virtual void PostCallRecordDestroyIndirectExecutionSetEXT(VkDevice device, VkIndirectExecutionSetEXT indirectExecutionSet,
                                                          const VkAllocationCallbacks* pAllocator, const RecordObject& record_obj) {
}
virtual bool PreCallValidateUpdateIndirectExecutionSetPipelineEXT(VkDevice device, VkIndirectExecutionSetEXT indirectExecutionSet,
                                                                  uint32_t executionSetWriteCount,
                                                                  const VkWriteIndirectExecutionSetPipelineEXT* pExecutionSetWrites,
                                                                  ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordUpdateIndirectExecutionSetPipelineEXT(VkDevice device, VkIndirectExecutionSetEXT indirectExecutionSet,
                                                                uint32_t executionSetWriteCount,
                                                                const VkWriteIndirectExecutionSetPipelineEXT* pExecutionSetWrites,
                                                                const RecordObject& record_obj) {}
virtual void PostCallRecordUpdateIndirectExecutionSetPipelineEXT(VkDevice device, VkIndirectExecutionSetEXT indirectExecutionSet,
                                                                 uint32_t executionSetWriteCount,
                                                                 const VkWriteIndirectExecutionSetPipelineEXT* pExecutionSetWrites,
                                                                 const RecordObject& record_obj) {}
virtual bool PreCallValidateUpdateIndirectExecutionSetShaderEXT(VkDevice device, VkIndirectExecutionSetEXT indirectExecutionSet,
                                                                uint32_t executionSetWriteCount,
                                                                const VkWriteIndirectExecutionSetShaderEXT* pExecutionSetWrites,
                                                                ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordUpdateIndirectExecutionSetShaderEXT(VkDevice device, VkIndirectExecutionSetEXT indirectExecutionSet,
                                                              uint32_t executionSetWriteCount,
                                                              const VkWriteIndirectExecutionSetShaderEXT* pExecutionSetWrites,
                                                              const RecordObject& record_obj) {}
virtual void PostCallRecordUpdateIndirectExecutionSetShaderEXT(VkDevice device, VkIndirectExecutionSetEXT indirectExecutionSet,
                                                               uint32_t executionSetWriteCount,
                                                               const VkWriteIndirectExecutionSetShaderEXT* pExecutionSetWrites,
                                                               const RecordObject& record_obj) {}
#ifdef VK_USE_PLATFORM_METAL_EXT
virtual bool PreCallValidateGetMemoryMetalHandleEXT(VkDevice device, const VkMemoryGetMetalHandleInfoEXT* pGetMetalHandleInfo,
                                                    void** pHandle, ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordGetMemoryMetalHandleEXT(VkDevice device, const VkMemoryGetMetalHandleInfoEXT* pGetMetalHandleInfo,
                                                  void** pHandle, const RecordObject& record_obj) {}
virtual void PostCallRecordGetMemoryMetalHandleEXT(VkDevice device, const VkMemoryGetMetalHandleInfoEXT* pGetMetalHandleInfo,
                                                   void** pHandle, const RecordObject& record_obj) {}
virtual bool PreCallValidateGetMemoryMetalHandlePropertiesEXT(VkDevice device, VkExternalMemoryHandleTypeFlagBits handleType,
                                                              const void* pHandle,
                                                              VkMemoryMetalHandlePropertiesEXT* pMemoryMetalHandleProperties,
                                                              ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordGetMemoryMetalHandlePropertiesEXT(VkDevice device, VkExternalMemoryHandleTypeFlagBits handleType,
                                                            const void* pHandle,
                                                            VkMemoryMetalHandlePropertiesEXT* pMemoryMetalHandleProperties,
                                                            const RecordObject& record_obj) {}
virtual void PostCallRecordGetMemoryMetalHandlePropertiesEXT(VkDevice device, VkExternalMemoryHandleTypeFlagBits handleType,
                                                             const void* pHandle,
                                                             VkMemoryMetalHandlePropertiesEXT* pMemoryMetalHandleProperties,
                                                             const RecordObject& record_obj) {}
#endif  // VK_USE_PLATFORM_METAL_EXT
virtual bool PreCallValidateCreateAccelerationStructureKHR(VkDevice device, const VkAccelerationStructureCreateInfoKHR* pCreateInfo,
                                                           const VkAllocationCallbacks* pAllocator,
                                                           VkAccelerationStructureKHR* pAccelerationStructure,
                                                           ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordCreateAccelerationStructureKHR(VkDevice device, const VkAccelerationStructureCreateInfoKHR* pCreateInfo,
                                                         const VkAllocationCallbacks* pAllocator,
                                                         VkAccelerationStructureKHR* pAccelerationStructure,
                                                         const RecordObject& record_obj) {}
virtual void PostCallRecordCreateAccelerationStructureKHR(VkDevice device, const VkAccelerationStructureCreateInfoKHR* pCreateInfo,
                                                          const VkAllocationCallbacks* pAllocator,
                                                          VkAccelerationStructureKHR* pAccelerationStructure,
                                                          const RecordObject& record_obj) {}
virtual bool PreCallValidateDestroyAccelerationStructureKHR(VkDevice device, VkAccelerationStructureKHR accelerationStructure,
                                                            const VkAllocationCallbacks* pAllocator, ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordDestroyAccelerationStructureKHR(VkDevice device, VkAccelerationStructureKHR accelerationStructure,
                                                          const VkAllocationCallbacks* pAllocator, const RecordObject& record_obj) {
}
virtual void PostCallRecordDestroyAccelerationStructureKHR(VkDevice device, VkAccelerationStructureKHR accelerationStructure,
                                                           const VkAllocationCallbacks* pAllocator,
                                                           const RecordObject& record_obj) {}
virtual bool PreCallValidateCmdBuildAccelerationStructuresKHR(
    VkCommandBuffer commandBuffer, uint32_t infoCount, const VkAccelerationStructureBuildGeometryInfoKHR* pInfos,
    const VkAccelerationStructureBuildRangeInfoKHR* const* ppBuildRangeInfos, ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordCmdBuildAccelerationStructuresKHR(
    VkCommandBuffer commandBuffer, uint32_t infoCount, const VkAccelerationStructureBuildGeometryInfoKHR* pInfos,
    const VkAccelerationStructureBuildRangeInfoKHR* const* ppBuildRangeInfos, const RecordObject& record_obj) {}
virtual void PostCallRecordCmdBuildAccelerationStructuresKHR(
    VkCommandBuffer commandBuffer, uint32_t infoCount, const VkAccelerationStructureBuildGeometryInfoKHR* pInfos,
    const VkAccelerationStructureBuildRangeInfoKHR* const* ppBuildRangeInfos, const RecordObject& record_obj) {}
virtual bool PreCallValidateCmdBuildAccelerationStructuresIndirectKHR(VkCommandBuffer commandBuffer, uint32_t infoCount,
                                                                      const VkAccelerationStructureBuildGeometryInfoKHR* pInfos,
                                                                      const VkDeviceAddress* pIndirectDeviceAddresses,
                                                                      const uint32_t* pIndirectStrides,
                                                                      const uint32_t* const* ppMaxPrimitiveCounts,
                                                                      ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordCmdBuildAccelerationStructuresIndirectKHR(VkCommandBuffer commandBuffer, uint32_t infoCount,
                                                                    const VkAccelerationStructureBuildGeometryInfoKHR* pInfos,
                                                                    const VkDeviceAddress* pIndirectDeviceAddresses,
                                                                    const uint32_t* pIndirectStrides,
                                                                    const uint32_t* const* ppMaxPrimitiveCounts,
                                                                    const RecordObject& record_obj) {}
virtual void PostCallRecordCmdBuildAccelerationStructuresIndirectKHR(VkCommandBuffer commandBuffer, uint32_t infoCount,
                                                                     const VkAccelerationStructureBuildGeometryInfoKHR* pInfos,
                                                                     const VkDeviceAddress* pIndirectDeviceAddresses,
                                                                     const uint32_t* pIndirectStrides,
                                                                     const uint32_t* const* ppMaxPrimitiveCounts,
                                                                     const RecordObject& record_obj) {}
virtual bool PreCallValidateBuildAccelerationStructuresKHR(VkDevice device, VkDeferredOperationKHR deferredOperation,
                                                           uint32_t infoCount,
                                                           const VkAccelerationStructureBuildGeometryInfoKHR* pInfos,
                                                           const VkAccelerationStructureBuildRangeInfoKHR* const* ppBuildRangeInfos,
                                                           ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordBuildAccelerationStructuresKHR(VkDevice device, VkDeferredOperationKHR deferredOperation,
                                                         uint32_t infoCount,
                                                         const VkAccelerationStructureBuildGeometryInfoKHR* pInfos,
                                                         const VkAccelerationStructureBuildRangeInfoKHR* const* ppBuildRangeInfos,
                                                         const RecordObject& record_obj) {}
virtual void PostCallRecordBuildAccelerationStructuresKHR(VkDevice device, VkDeferredOperationKHR deferredOperation,
                                                          uint32_t infoCount,
                                                          const VkAccelerationStructureBuildGeometryInfoKHR* pInfos,
                                                          const VkAccelerationStructureBuildRangeInfoKHR* const* ppBuildRangeInfos,
                                                          const RecordObject& record_obj) {}
virtual bool PreCallValidateCopyAccelerationStructureKHR(VkDevice device, VkDeferredOperationKHR deferredOperation,
                                                         const VkCopyAccelerationStructureInfoKHR* pInfo,
                                                         ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordCopyAccelerationStructureKHR(VkDevice device, VkDeferredOperationKHR deferredOperation,
                                                       const VkCopyAccelerationStructureInfoKHR* pInfo,
                                                       const RecordObject& record_obj) {}
virtual void PostCallRecordCopyAccelerationStructureKHR(VkDevice device, VkDeferredOperationKHR deferredOperation,
                                                        const VkCopyAccelerationStructureInfoKHR* pInfo,
                                                        const RecordObject& record_obj) {}
virtual bool PreCallValidateCopyAccelerationStructureToMemoryKHR(VkDevice device, VkDeferredOperationKHR deferredOperation,
                                                                 const VkCopyAccelerationStructureToMemoryInfoKHR* pInfo,
                                                                 ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordCopyAccelerationStructureToMemoryKHR(VkDevice device, VkDeferredOperationKHR deferredOperation,
                                                               const VkCopyAccelerationStructureToMemoryInfoKHR* pInfo,
                                                               const RecordObject& record_obj) {}
virtual void PostCallRecordCopyAccelerationStructureToMemoryKHR(VkDevice device, VkDeferredOperationKHR deferredOperation,
                                                                const VkCopyAccelerationStructureToMemoryInfoKHR* pInfo,
                                                                const RecordObject& record_obj) {}
virtual bool PreCallValidateCopyMemoryToAccelerationStructureKHR(VkDevice device, VkDeferredOperationKHR deferredOperation,
                                                                 const VkCopyMemoryToAccelerationStructureInfoKHR* pInfo,
                                                                 ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordCopyMemoryToAccelerationStructureKHR(VkDevice device, VkDeferredOperationKHR deferredOperation,
                                                               const VkCopyMemoryToAccelerationStructureInfoKHR* pInfo,
                                                               const RecordObject& record_obj) {}
virtual void PostCallRecordCopyMemoryToAccelerationStructureKHR(VkDevice device, VkDeferredOperationKHR deferredOperation,
                                                                const VkCopyMemoryToAccelerationStructureInfoKHR* pInfo,
                                                                const RecordObject& record_obj) {}
virtual bool PreCallValidateWriteAccelerationStructuresPropertiesKHR(VkDevice device, uint32_t accelerationStructureCount,
                                                                     const VkAccelerationStructureKHR* pAccelerationStructures,
                                                                     VkQueryType queryType, size_t dataSize, void* pData,
                                                                     size_t stride, ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordWriteAccelerationStructuresPropertiesKHR(VkDevice device, uint32_t accelerationStructureCount,
                                                                   const VkAccelerationStructureKHR* pAccelerationStructures,
                                                                   VkQueryType queryType, size_t dataSize, void* pData,
                                                                   size_t stride, const RecordObject& record_obj) {}
virtual void PostCallRecordWriteAccelerationStructuresPropertiesKHR(VkDevice device, uint32_t accelerationStructureCount,
                                                                    const VkAccelerationStructureKHR* pAccelerationStructures,
                                                                    VkQueryType queryType, size_t dataSize, void* pData,
                                                                    size_t stride, const RecordObject& record_obj) {}
virtual bool PreCallValidateCmdCopyAccelerationStructureKHR(VkCommandBuffer commandBuffer,
                                                            const VkCopyAccelerationStructureInfoKHR* pInfo,
                                                            ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordCmdCopyAccelerationStructureKHR(VkCommandBuffer commandBuffer,
                                                          const VkCopyAccelerationStructureInfoKHR* pInfo,
                                                          const RecordObject& record_obj) {}
virtual void PostCallRecordCmdCopyAccelerationStructureKHR(VkCommandBuffer commandBuffer,
                                                           const VkCopyAccelerationStructureInfoKHR* pInfo,
                                                           const RecordObject& record_obj) {}
virtual bool PreCallValidateCmdCopyAccelerationStructureToMemoryKHR(VkCommandBuffer commandBuffer,
                                                                    const VkCopyAccelerationStructureToMemoryInfoKHR* pInfo,
                                                                    ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordCmdCopyAccelerationStructureToMemoryKHR(VkCommandBuffer commandBuffer,
                                                                  const VkCopyAccelerationStructureToMemoryInfoKHR* pInfo,
                                                                  const RecordObject& record_obj) {}
virtual void PostCallRecordCmdCopyAccelerationStructureToMemoryKHR(VkCommandBuffer commandBuffer,
                                                                   const VkCopyAccelerationStructureToMemoryInfoKHR* pInfo,
                                                                   const RecordObject& record_obj) {}
virtual bool PreCallValidateCmdCopyMemoryToAccelerationStructureKHR(VkCommandBuffer commandBuffer,
                                                                    const VkCopyMemoryToAccelerationStructureInfoKHR* pInfo,
                                                                    ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordCmdCopyMemoryToAccelerationStructureKHR(VkCommandBuffer commandBuffer,
                                                                  const VkCopyMemoryToAccelerationStructureInfoKHR* pInfo,
                                                                  const RecordObject& record_obj) {}
virtual void PostCallRecordCmdCopyMemoryToAccelerationStructureKHR(VkCommandBuffer commandBuffer,
                                                                   const VkCopyMemoryToAccelerationStructureInfoKHR* pInfo,
                                                                   const RecordObject& record_obj) {}
virtual bool PreCallValidateGetAccelerationStructureDeviceAddressKHR(VkDevice device,
                                                                     const VkAccelerationStructureDeviceAddressInfoKHR* pInfo,
                                                                     ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordGetAccelerationStructureDeviceAddressKHR(VkDevice device,
                                                                   const VkAccelerationStructureDeviceAddressInfoKHR* pInfo,
                                                                   const RecordObject& record_obj) {}
virtual void PostCallRecordGetAccelerationStructureDeviceAddressKHR(VkDevice device,
                                                                    const VkAccelerationStructureDeviceAddressInfoKHR* pInfo,
                                                                    const RecordObject& record_obj) {}
virtual bool PreCallValidateCmdWriteAccelerationStructuresPropertiesKHR(VkCommandBuffer commandBuffer,
                                                                        uint32_t accelerationStructureCount,
                                                                        const VkAccelerationStructureKHR* pAccelerationStructures,
                                                                        VkQueryType queryType, VkQueryPool queryPool,
                                                                        uint32_t firstQuery, ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordCmdWriteAccelerationStructuresPropertiesKHR(VkCommandBuffer commandBuffer,
                                                                      uint32_t accelerationStructureCount,
                                                                      const VkAccelerationStructureKHR* pAccelerationStructures,
                                                                      VkQueryType queryType, VkQueryPool queryPool,
                                                                      uint32_t firstQuery, const RecordObject& record_obj) {}
virtual void PostCallRecordCmdWriteAccelerationStructuresPropertiesKHR(VkCommandBuffer commandBuffer,
                                                                       uint32_t accelerationStructureCount,
                                                                       const VkAccelerationStructureKHR* pAccelerationStructures,
                                                                       VkQueryType queryType, VkQueryPool queryPool,
                                                                       uint32_t firstQuery, const RecordObject& record_obj) {}
virtual bool PreCallValidateGetDeviceAccelerationStructureCompatibilityKHR(
    VkDevice device, const VkAccelerationStructureVersionInfoKHR* pVersionInfo,
    VkAccelerationStructureCompatibilityKHR* pCompatibility, ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordGetDeviceAccelerationStructureCompatibilityKHR(VkDevice device,
                                                                         const VkAccelerationStructureVersionInfoKHR* pVersionInfo,
                                                                         VkAccelerationStructureCompatibilityKHR* pCompatibility,
                                                                         const RecordObject& record_obj) {}
virtual void PostCallRecordGetDeviceAccelerationStructureCompatibilityKHR(VkDevice device,
                                                                          const VkAccelerationStructureVersionInfoKHR* pVersionInfo,
                                                                          VkAccelerationStructureCompatibilityKHR* pCompatibility,
                                                                          const RecordObject& record_obj) {}
virtual bool PreCallValidateGetAccelerationStructureBuildSizesKHR(VkDevice device, VkAccelerationStructureBuildTypeKHR buildType,
                                                                  const VkAccelerationStructureBuildGeometryInfoKHR* pBuildInfo,
                                                                  const uint32_t* pMaxPrimitiveCounts,
                                                                  VkAccelerationStructureBuildSizesInfoKHR* pSizeInfo,
                                                                  ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordGetAccelerationStructureBuildSizesKHR(VkDevice device, VkAccelerationStructureBuildTypeKHR buildType,
                                                                const VkAccelerationStructureBuildGeometryInfoKHR* pBuildInfo,
                                                                const uint32_t* pMaxPrimitiveCounts,
                                                                VkAccelerationStructureBuildSizesInfoKHR* pSizeInfo,
                                                                const RecordObject& record_obj) {}
virtual void PostCallRecordGetAccelerationStructureBuildSizesKHR(VkDevice device, VkAccelerationStructureBuildTypeKHR buildType,
                                                                 const VkAccelerationStructureBuildGeometryInfoKHR* pBuildInfo,
                                                                 const uint32_t* pMaxPrimitiveCounts,
                                                                 VkAccelerationStructureBuildSizesInfoKHR* pSizeInfo,
                                                                 const RecordObject& record_obj) {}
virtual bool PreCallValidateCmdTraceRaysKHR(VkCommandBuffer commandBuffer,
                                            const VkStridedDeviceAddressRegionKHR* pRaygenShaderBindingTable,
                                            const VkStridedDeviceAddressRegionKHR* pMissShaderBindingTable,
                                            const VkStridedDeviceAddressRegionKHR* pHitShaderBindingTable,
                                            const VkStridedDeviceAddressRegionKHR* pCallableShaderBindingTable, uint32_t width,
                                            uint32_t height, uint32_t depth, ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordCmdTraceRaysKHR(VkCommandBuffer commandBuffer,
                                          const VkStridedDeviceAddressRegionKHR* pRaygenShaderBindingTable,
                                          const VkStridedDeviceAddressRegionKHR* pMissShaderBindingTable,
                                          const VkStridedDeviceAddressRegionKHR* pHitShaderBindingTable,
                                          const VkStridedDeviceAddressRegionKHR* pCallableShaderBindingTable, uint32_t width,
                                          uint32_t height, uint32_t depth, const RecordObject& record_obj) {}
virtual void PostCallRecordCmdTraceRaysKHR(VkCommandBuffer commandBuffer,
                                           const VkStridedDeviceAddressRegionKHR* pRaygenShaderBindingTable,
                                           const VkStridedDeviceAddressRegionKHR* pMissShaderBindingTable,
                                           const VkStridedDeviceAddressRegionKHR* pHitShaderBindingTable,
                                           const VkStridedDeviceAddressRegionKHR* pCallableShaderBindingTable, uint32_t width,
                                           uint32_t height, uint32_t depth, const RecordObject& record_obj) {}
virtual bool PreCallValidateCreateRayTracingPipelinesKHR(VkDevice device, VkDeferredOperationKHR deferredOperation,
                                                         VkPipelineCache pipelineCache, uint32_t createInfoCount,
                                                         const VkRayTracingPipelineCreateInfoKHR* pCreateInfos,
                                                         const VkAllocationCallbacks* pAllocator, VkPipeline* pPipelines,
                                                         ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordCreateRayTracingPipelinesKHR(VkDevice device, VkDeferredOperationKHR deferredOperation,
                                                       VkPipelineCache pipelineCache, uint32_t createInfoCount,
                                                       const VkRayTracingPipelineCreateInfoKHR* pCreateInfos,
                                                       const VkAllocationCallbacks* pAllocator, VkPipeline* pPipelines,
                                                       const RecordObject& record_obj) {}
virtual void PostCallRecordCreateRayTracingPipelinesKHR(VkDevice device, VkDeferredOperationKHR deferredOperation,
                                                        VkPipelineCache pipelineCache, uint32_t createInfoCount,
                                                        const VkRayTracingPipelineCreateInfoKHR* pCreateInfos,
                                                        const VkAllocationCallbacks* pAllocator, VkPipeline* pPipelines,
                                                        const RecordObject& record_obj) {}
virtual bool PreCallValidateGetRayTracingCaptureReplayShaderGroupHandlesKHR(VkDevice device, VkPipeline pipeline,
                                                                            uint32_t firstGroup, uint32_t groupCount,
                                                                            size_t dataSize, void* pData,
                                                                            ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordGetRayTracingCaptureReplayShaderGroupHandlesKHR(VkDevice device, VkPipeline pipeline, uint32_t firstGroup,
                                                                          uint32_t groupCount, size_t dataSize, void* pData,
                                                                          const RecordObject& record_obj) {}
virtual void PostCallRecordGetRayTracingCaptureReplayShaderGroupHandlesKHR(VkDevice device, VkPipeline pipeline,
                                                                           uint32_t firstGroup, uint32_t groupCount,
                                                                           size_t dataSize, void* pData,
                                                                           const RecordObject& record_obj) {}
virtual bool PreCallValidateCmdTraceRaysIndirectKHR(VkCommandBuffer commandBuffer,
                                                    const VkStridedDeviceAddressRegionKHR* pRaygenShaderBindingTable,
                                                    const VkStridedDeviceAddressRegionKHR* pMissShaderBindingTable,
                                                    const VkStridedDeviceAddressRegionKHR* pHitShaderBindingTable,
                                                    const VkStridedDeviceAddressRegionKHR* pCallableShaderBindingTable,
                                                    VkDeviceAddress indirectDeviceAddress, ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordCmdTraceRaysIndirectKHR(VkCommandBuffer commandBuffer,
                                                  const VkStridedDeviceAddressRegionKHR* pRaygenShaderBindingTable,
                                                  const VkStridedDeviceAddressRegionKHR* pMissShaderBindingTable,
                                                  const VkStridedDeviceAddressRegionKHR* pHitShaderBindingTable,
                                                  const VkStridedDeviceAddressRegionKHR* pCallableShaderBindingTable,
                                                  VkDeviceAddress indirectDeviceAddress, const RecordObject& record_obj) {}
virtual void PostCallRecordCmdTraceRaysIndirectKHR(VkCommandBuffer commandBuffer,
                                                   const VkStridedDeviceAddressRegionKHR* pRaygenShaderBindingTable,
                                                   const VkStridedDeviceAddressRegionKHR* pMissShaderBindingTable,
                                                   const VkStridedDeviceAddressRegionKHR* pHitShaderBindingTable,
                                                   const VkStridedDeviceAddressRegionKHR* pCallableShaderBindingTable,
                                                   VkDeviceAddress indirectDeviceAddress, const RecordObject& record_obj) {}
virtual bool PreCallValidateGetRayTracingShaderGroupStackSizeKHR(VkDevice device, VkPipeline pipeline, uint32_t group,
                                                                 VkShaderGroupShaderKHR groupShader, ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordGetRayTracingShaderGroupStackSizeKHR(VkDevice device, VkPipeline pipeline, uint32_t group,
                                                               VkShaderGroupShaderKHR groupShader, const RecordObject& record_obj) {
}
virtual void PostCallRecordGetRayTracingShaderGroupStackSizeKHR(VkDevice device, VkPipeline pipeline, uint32_t group,
                                                                VkShaderGroupShaderKHR groupShader,
                                                                const RecordObject& record_obj) {}
virtual bool PreCallValidateCmdSetRayTracingPipelineStackSizeKHR(VkCommandBuffer commandBuffer, uint32_t pipelineStackSize,
                                                                 ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordCmdSetRayTracingPipelineStackSizeKHR(VkCommandBuffer commandBuffer, uint32_t pipelineStackSize,
                                                               const RecordObject& record_obj) {}
virtual void PostCallRecordCmdSetRayTracingPipelineStackSizeKHR(VkCommandBuffer commandBuffer, uint32_t pipelineStackSize,
                                                                const RecordObject& record_obj) {}
virtual bool PreCallValidateCmdDrawMeshTasksEXT(VkCommandBuffer commandBuffer, uint32_t groupCountX, uint32_t groupCountY,
                                                uint32_t groupCountZ, ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordCmdDrawMeshTasksEXT(VkCommandBuffer commandBuffer, uint32_t groupCountX, uint32_t groupCountY,
                                              uint32_t groupCountZ, const RecordObject& record_obj) {}
virtual void PostCallRecordCmdDrawMeshTasksEXT(VkCommandBuffer commandBuffer, uint32_t groupCountX, uint32_t groupCountY,
                                               uint32_t groupCountZ, const RecordObject& record_obj) {}
virtual bool PreCallValidateCmdDrawMeshTasksIndirectEXT(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                        uint32_t drawCount, uint32_t stride, ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordCmdDrawMeshTasksIndirectEXT(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                      uint32_t drawCount, uint32_t stride, const RecordObject& record_obj) {}
virtual void PostCallRecordCmdDrawMeshTasksIndirectEXT(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                       uint32_t drawCount, uint32_t stride, const RecordObject& record_obj) {}
virtual bool PreCallValidateCmdDrawMeshTasksIndirectCountEXT(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                             VkBuffer countBuffer, VkDeviceSize countBufferOffset,
                                                             uint32_t maxDrawCount, uint32_t stride, ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordCmdDrawMeshTasksIndirectCountEXT(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                           VkBuffer countBuffer, VkDeviceSize countBufferOffset,
                                                           uint32_t maxDrawCount, uint32_t stride, const RecordObject& record_obj) {
}
virtual void PostCallRecordCmdDrawMeshTasksIndirectCountEXT(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                            VkBuffer countBuffer, VkDeviceSize countBufferOffset,
                                                            uint32_t maxDrawCount, uint32_t stride,
                                                            const RecordObject& record_obj) {}

// NOLINTEND
