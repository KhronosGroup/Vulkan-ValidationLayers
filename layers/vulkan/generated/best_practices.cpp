// *** THIS FILE IS GENERATED - DO NOT EDIT ***
// See best_practices_generator.py for modifications

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

#include "chassis.h"
#include "best_practices/best_practices_validation.h"

void BestPractices::PostCallRecordCreateInstance(const VkInstanceCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator,
                                                 VkInstance* pInstance, const RecordObject& record_obj) {
    ValidationStateTracker::PostCallRecordCreateInstance(pCreateInfo, pAllocator, pInstance, record_obj);

    if (record_obj.result < VK_SUCCESS) {
        LogErrorCode(record_obj);
    }
}

void BestPractices::PostCallRecordEnumeratePhysicalDevices(VkInstance instance, uint32_t* pPhysicalDeviceCount,
                                                           VkPhysicalDevice* pPhysicalDevices, const RecordObject& record_obj) {
    ValidationStateTracker::PostCallRecordEnumeratePhysicalDevices(instance, pPhysicalDeviceCount, pPhysicalDevices, record_obj);

    if (record_obj.result > VK_SUCCESS) {
        LogPositiveSuccessCode(record_obj);
        return;
    }
    if (record_obj.result < VK_SUCCESS) {
        LogErrorCode(record_obj);
    }
}

void BestPractices::PostCallRecordGetPhysicalDeviceImageFormatProperties(VkPhysicalDevice physicalDevice, VkFormat format,
                                                                         VkImageType type, VkImageTiling tiling,
                                                                         VkImageUsageFlags usage, VkImageCreateFlags flags,
                                                                         VkImageFormatProperties* pImageFormatProperties,
                                                                         const RecordObject& record_obj) {
    ValidationStateTracker::PostCallRecordGetPhysicalDeviceImageFormatProperties(physicalDevice, format, type, tiling, usage, flags,
                                                                                 pImageFormatProperties, record_obj);

    if (record_obj.result < VK_SUCCESS) {
        LogErrorCode(record_obj);
    }
}

void BestPractices::PostCallRecordCreateDevice(VkPhysicalDevice physicalDevice, const VkDeviceCreateInfo* pCreateInfo,
                                               const VkAllocationCallbacks* pAllocator, VkDevice* pDevice,
                                               const RecordObject& record_obj) {
    ValidationStateTracker::PostCallRecordCreateDevice(physicalDevice, pCreateInfo, pAllocator, pDevice, record_obj);

    if (record_obj.result < VK_SUCCESS) {
        LogErrorCode(record_obj);
    }
}

void BestPractices::PostCallRecordEnumerateInstanceExtensionProperties(const char* pLayerName, uint32_t* pPropertyCount,
                                                                       VkExtensionProperties* pProperties,
                                                                       const RecordObject& record_obj) {
    ValidationStateTracker::PostCallRecordEnumerateInstanceExtensionProperties(pLayerName, pPropertyCount, pProperties, record_obj);

    if (record_obj.result > VK_SUCCESS) {
        LogPositiveSuccessCode(record_obj);
        return;
    }
    if (record_obj.result < VK_SUCCESS) {
        LogErrorCode(record_obj);
    }
}

void BestPractices::PostCallRecordEnumerateDeviceExtensionProperties(VkPhysicalDevice physicalDevice, const char* pLayerName,
                                                                     uint32_t* pPropertyCount, VkExtensionProperties* pProperties,
                                                                     const RecordObject& record_obj) {
    ValidationStateTracker::PostCallRecordEnumerateDeviceExtensionProperties(physicalDevice, pLayerName, pPropertyCount,
                                                                             pProperties, record_obj);

    if (record_obj.result > VK_SUCCESS) {
        LogPositiveSuccessCode(record_obj);
        return;
    }
    if (record_obj.result < VK_SUCCESS) {
        LogErrorCode(record_obj);
    }
}

void BestPractices::PostCallRecordEnumerateInstanceLayerProperties(uint32_t* pPropertyCount, VkLayerProperties* pProperties,
                                                                   const RecordObject& record_obj) {
    ValidationStateTracker::PostCallRecordEnumerateInstanceLayerProperties(pPropertyCount, pProperties, record_obj);

    if (record_obj.result > VK_SUCCESS) {
        LogPositiveSuccessCode(record_obj);
        return;
    }
    if (record_obj.result < VK_SUCCESS) {
        LogErrorCode(record_obj);
    }
}

void BestPractices::PostCallRecordEnumerateDeviceLayerProperties(VkPhysicalDevice physicalDevice, uint32_t* pPropertyCount,
                                                                 VkLayerProperties* pProperties, const RecordObject& record_obj) {
    ValidationStateTracker::PostCallRecordEnumerateDeviceLayerProperties(physicalDevice, pPropertyCount, pProperties, record_obj);

    if (record_obj.result > VK_SUCCESS) {
        LogPositiveSuccessCode(record_obj);
        return;
    }
    if (record_obj.result < VK_SUCCESS) {
        LogErrorCode(record_obj);
    }
}

void BestPractices::PostCallRecordQueueSubmit(VkQueue queue, uint32_t submitCount, const VkSubmitInfo* pSubmits, VkFence fence,
                                              const RecordObject& record_obj) {
    ValidationStateTracker::PostCallRecordQueueSubmit(queue, submitCount, pSubmits, fence, record_obj);
    ManualPostCallRecordQueueSubmit(queue, submitCount, pSubmits, fence, record_obj);

    if (record_obj.result < VK_SUCCESS) {
        LogErrorCode(record_obj);
    }
}

void BestPractices::PostCallRecordQueueWaitIdle(VkQueue queue, const RecordObject& record_obj) {
    ValidationStateTracker::PostCallRecordQueueWaitIdle(queue, record_obj);

    if (record_obj.result < VK_SUCCESS) {
        LogErrorCode(record_obj);
    }
}

void BestPractices::PostCallRecordDeviceWaitIdle(VkDevice device, const RecordObject& record_obj) {
    ValidationStateTracker::PostCallRecordDeviceWaitIdle(device, record_obj);

    if (record_obj.result < VK_SUCCESS) {
        LogErrorCode(record_obj);
    }
}

void BestPractices::PostCallRecordAllocateMemory(VkDevice device, const VkMemoryAllocateInfo* pAllocateInfo,
                                                 const VkAllocationCallbacks* pAllocator, VkDeviceMemory* pMemory,
                                                 const RecordObject& record_obj) {
    ValidationStateTracker::PostCallRecordAllocateMemory(device, pAllocateInfo, pAllocator, pMemory, record_obj);

    if (record_obj.result < VK_SUCCESS) {
        LogErrorCode(record_obj);
    }
}

void BestPractices::PostCallRecordMapMemory(VkDevice device, VkDeviceMemory memory, VkDeviceSize offset, VkDeviceSize size,
                                            VkMemoryMapFlags flags, void** ppData, const RecordObject& record_obj) {
    ValidationStateTracker::PostCallRecordMapMemory(device, memory, offset, size, flags, ppData, record_obj);

    if (record_obj.result < VK_SUCCESS) {
        LogErrorCode(record_obj);
    }
}

void BestPractices::PostCallRecordFlushMappedMemoryRanges(VkDevice device, uint32_t memoryRangeCount,
                                                          const VkMappedMemoryRange* pMemoryRanges,
                                                          const RecordObject& record_obj) {
    ValidationStateTracker::PostCallRecordFlushMappedMemoryRanges(device, memoryRangeCount, pMemoryRanges, record_obj);

    if (record_obj.result < VK_SUCCESS) {
        LogErrorCode(record_obj);
    }
}

void BestPractices::PostCallRecordInvalidateMappedMemoryRanges(VkDevice device, uint32_t memoryRangeCount,
                                                               const VkMappedMemoryRange* pMemoryRanges,
                                                               const RecordObject& record_obj) {
    ValidationStateTracker::PostCallRecordInvalidateMappedMemoryRanges(device, memoryRangeCount, pMemoryRanges, record_obj);

    if (record_obj.result < VK_SUCCESS) {
        LogErrorCode(record_obj);
    }
}

void BestPractices::PostCallRecordBindBufferMemory(VkDevice device, VkBuffer buffer, VkDeviceMemory memory,
                                                   VkDeviceSize memoryOffset, const RecordObject& record_obj) {
    ValidationStateTracker::PostCallRecordBindBufferMemory(device, buffer, memory, memoryOffset, record_obj);

    if (record_obj.result < VK_SUCCESS) {
        LogErrorCode(record_obj);
    }
}

void BestPractices::PostCallRecordBindImageMemory(VkDevice device, VkImage image, VkDeviceMemory memory, VkDeviceSize memoryOffset,
                                                  const RecordObject& record_obj) {
    ValidationStateTracker::PostCallRecordBindImageMemory(device, image, memory, memoryOffset, record_obj);

    if (record_obj.result < VK_SUCCESS) {
        LogErrorCode(record_obj);
    }
}

void BestPractices::PostCallRecordQueueBindSparse(VkQueue queue, uint32_t bindInfoCount, const VkBindSparseInfo* pBindInfo,
                                                  VkFence fence, const RecordObject& record_obj) {
    ValidationStateTracker::PostCallRecordQueueBindSparse(queue, bindInfoCount, pBindInfo, fence, record_obj);
    ManualPostCallRecordQueueBindSparse(queue, bindInfoCount, pBindInfo, fence, record_obj);

    if (record_obj.result < VK_SUCCESS) {
        LogErrorCode(record_obj);
    }
}

void BestPractices::PostCallRecordCreateFence(VkDevice device, const VkFenceCreateInfo* pCreateInfo,
                                              const VkAllocationCallbacks* pAllocator, VkFence* pFence,
                                              const RecordObject& record_obj) {
    ValidationStateTracker::PostCallRecordCreateFence(device, pCreateInfo, pAllocator, pFence, record_obj);

    if (record_obj.result < VK_SUCCESS) {
        LogErrorCode(record_obj);
    }
}

void BestPractices::PostCallRecordResetFences(VkDevice device, uint32_t fenceCount, const VkFence* pFences,
                                              const RecordObject& record_obj) {
    ValidationStateTracker::PostCallRecordResetFences(device, fenceCount, pFences, record_obj);

    if (record_obj.result < VK_SUCCESS) {
        LogErrorCode(record_obj);
    }
}

void BestPractices::PostCallRecordGetFenceStatus(VkDevice device, VkFence fence, const RecordObject& record_obj) {
    ValidationStateTracker::PostCallRecordGetFenceStatus(device, fence, record_obj);

    if (record_obj.result > VK_SUCCESS) {
        LogPositiveSuccessCode(record_obj);
        return;
    }
    if (record_obj.result < VK_SUCCESS) {
        LogErrorCode(record_obj);
    }
}

void BestPractices::PostCallRecordWaitForFences(VkDevice device, uint32_t fenceCount, const VkFence* pFences, VkBool32 waitAll,
                                                uint64_t timeout, const RecordObject& record_obj) {
    ValidationStateTracker::PostCallRecordWaitForFences(device, fenceCount, pFences, waitAll, timeout, record_obj);

    if (record_obj.result > VK_SUCCESS) {
        LogPositiveSuccessCode(record_obj);
        return;
    }
    if (record_obj.result < VK_SUCCESS) {
        LogErrorCode(record_obj);
    }
}

void BestPractices::PostCallRecordCreateSemaphore(VkDevice device, const VkSemaphoreCreateInfo* pCreateInfo,
                                                  const VkAllocationCallbacks* pAllocator, VkSemaphore* pSemaphore,
                                                  const RecordObject& record_obj) {
    ValidationStateTracker::PostCallRecordCreateSemaphore(device, pCreateInfo, pAllocator, pSemaphore, record_obj);

    if (record_obj.result < VK_SUCCESS) {
        LogErrorCode(record_obj);
    }
}

void BestPractices::PostCallRecordCreateEvent(VkDevice device, const VkEventCreateInfo* pCreateInfo,
                                              const VkAllocationCallbacks* pAllocator, VkEvent* pEvent,
                                              const RecordObject& record_obj) {
    ValidationStateTracker::PostCallRecordCreateEvent(device, pCreateInfo, pAllocator, pEvent, record_obj);

    if (record_obj.result < VK_SUCCESS) {
        LogErrorCode(record_obj);
    }
}

void BestPractices::PostCallRecordGetEventStatus(VkDevice device, VkEvent event, const RecordObject& record_obj) {
    ValidationStateTracker::PostCallRecordGetEventStatus(device, event, record_obj);

    if (record_obj.result > VK_SUCCESS) {
        LogPositiveSuccessCode(record_obj);
        return;
    }
    if (record_obj.result < VK_SUCCESS) {
        LogErrorCode(record_obj);
    }
}

void BestPractices::PostCallRecordSetEvent(VkDevice device, VkEvent event, const RecordObject& record_obj) {
    ValidationStateTracker::PostCallRecordSetEvent(device, event, record_obj);

    if (record_obj.result < VK_SUCCESS) {
        LogErrorCode(record_obj);
    }
}

void BestPractices::PostCallRecordResetEvent(VkDevice device, VkEvent event, const RecordObject& record_obj) {
    ValidationStateTracker::PostCallRecordResetEvent(device, event, record_obj);

    if (record_obj.result < VK_SUCCESS) {
        LogErrorCode(record_obj);
    }
}

void BestPractices::PostCallRecordCreateQueryPool(VkDevice device, const VkQueryPoolCreateInfo* pCreateInfo,
                                                  const VkAllocationCallbacks* pAllocator, VkQueryPool* pQueryPool,
                                                  const RecordObject& record_obj) {
    ValidationStateTracker::PostCallRecordCreateQueryPool(device, pCreateInfo, pAllocator, pQueryPool, record_obj);

    if (record_obj.result < VK_SUCCESS) {
        LogErrorCode(record_obj);
    }
}

void BestPractices::PostCallRecordGetQueryPoolResults(VkDevice device, VkQueryPool queryPool, uint32_t firstQuery,
                                                      uint32_t queryCount, size_t dataSize, void* pData, VkDeviceSize stride,
                                                      VkQueryResultFlags flags, const RecordObject& record_obj) {
    ValidationStateTracker::PostCallRecordGetQueryPoolResults(device, queryPool, firstQuery, queryCount, dataSize, pData, stride,
                                                              flags, record_obj);

    if (record_obj.result > VK_SUCCESS) {
        LogPositiveSuccessCode(record_obj);
        return;
    }
    if (record_obj.result < VK_SUCCESS) {
        LogErrorCode(record_obj);
    }
}

void BestPractices::PostCallRecordCreateBuffer(VkDevice device, const VkBufferCreateInfo* pCreateInfo,
                                               const VkAllocationCallbacks* pAllocator, VkBuffer* pBuffer,
                                               const RecordObject& record_obj) {
    ValidationStateTracker::PostCallRecordCreateBuffer(device, pCreateInfo, pAllocator, pBuffer, record_obj);

    if (record_obj.result < VK_SUCCESS) {
        LogErrorCode(record_obj);
    }
}

void BestPractices::PostCallRecordCreateBufferView(VkDevice device, const VkBufferViewCreateInfo* pCreateInfo,
                                                   const VkAllocationCallbacks* pAllocator, VkBufferView* pView,
                                                   const RecordObject& record_obj) {
    ValidationStateTracker::PostCallRecordCreateBufferView(device, pCreateInfo, pAllocator, pView, record_obj);

    if (record_obj.result < VK_SUCCESS) {
        LogErrorCode(record_obj);
    }
}

void BestPractices::PostCallRecordCreateImage(VkDevice device, const VkImageCreateInfo* pCreateInfo,
                                              const VkAllocationCallbacks* pAllocator, VkImage* pImage,
                                              const RecordObject& record_obj) {
    ValidationStateTracker::PostCallRecordCreateImage(device, pCreateInfo, pAllocator, pImage, record_obj);

    if (record_obj.result < VK_SUCCESS) {
        LogErrorCode(record_obj);
    }
}

void BestPractices::PostCallRecordCreateImageView(VkDevice device, const VkImageViewCreateInfo* pCreateInfo,
                                                  const VkAllocationCallbacks* pAllocator, VkImageView* pView,
                                                  const RecordObject& record_obj) {
    ValidationStateTracker::PostCallRecordCreateImageView(device, pCreateInfo, pAllocator, pView, record_obj);

    if (record_obj.result < VK_SUCCESS) {
        LogErrorCode(record_obj);
    }
}

void BestPractices::PostCallRecordCreateShaderModule(VkDevice device, const VkShaderModuleCreateInfo* pCreateInfo,
                                                     const VkAllocationCallbacks* pAllocator, VkShaderModule* pShaderModule,
                                                     const RecordObject& record_obj, void* state_data) {
    ValidationStateTracker::PostCallRecordCreateShaderModule(device, pCreateInfo, pAllocator, pShaderModule, record_obj,
                                                             state_data);

    if (record_obj.result < VK_SUCCESS) {
        LogErrorCode(record_obj);
    }
}

void BestPractices::PostCallRecordCreatePipelineCache(VkDevice device, const VkPipelineCacheCreateInfo* pCreateInfo,
                                                      const VkAllocationCallbacks* pAllocator, VkPipelineCache* pPipelineCache,
                                                      const RecordObject& record_obj) {
    ValidationStateTracker::PostCallRecordCreatePipelineCache(device, pCreateInfo, pAllocator, pPipelineCache, record_obj);

    if (record_obj.result < VK_SUCCESS) {
        LogErrorCode(record_obj);
    }
}

void BestPractices::PostCallRecordGetPipelineCacheData(VkDevice device, VkPipelineCache pipelineCache, size_t* pDataSize,
                                                       void* pData, const RecordObject& record_obj) {
    ValidationStateTracker::PostCallRecordGetPipelineCacheData(device, pipelineCache, pDataSize, pData, record_obj);

    if (record_obj.result > VK_SUCCESS) {
        LogPositiveSuccessCode(record_obj);
        return;
    }
    if (record_obj.result < VK_SUCCESS) {
        LogErrorCode(record_obj);
    }
}

void BestPractices::PostCallRecordMergePipelineCaches(VkDevice device, VkPipelineCache dstCache, uint32_t srcCacheCount,
                                                      const VkPipelineCache* pSrcCaches, const RecordObject& record_obj) {
    ValidationStateTracker::PostCallRecordMergePipelineCaches(device, dstCache, srcCacheCount, pSrcCaches, record_obj);

    if (record_obj.result < VK_SUCCESS) {
        LogErrorCode(record_obj);
    }
}

void BestPractices::PostCallRecordCreateGraphicsPipelines(VkDevice device, VkPipelineCache pipelineCache, uint32_t createInfoCount,
                                                          const VkGraphicsPipelineCreateInfo* pCreateInfos,
                                                          const VkAllocationCallbacks* pAllocator, VkPipeline* pPipelines,
                                                          const RecordObject& record_obj, void* state_data) {
    ValidationStateTracker::PostCallRecordCreateGraphicsPipelines(device, pipelineCache, createInfoCount, pCreateInfos, pAllocator,
                                                                  pPipelines, record_obj, state_data);
    ManualPostCallRecordCreateGraphicsPipelines(device, pipelineCache, createInfoCount, pCreateInfos, pAllocator, pPipelines,
                                                record_obj, state_data);

    if (record_obj.result > VK_SUCCESS) {
        LogPositiveSuccessCode(record_obj);
        return;
    }
    if (record_obj.result < VK_SUCCESS) {
        LogErrorCode(record_obj);
    }
}

void BestPractices::PostCallRecordCreateComputePipelines(VkDevice device, VkPipelineCache pipelineCache, uint32_t createInfoCount,
                                                         const VkComputePipelineCreateInfo* pCreateInfos,
                                                         const VkAllocationCallbacks* pAllocator, VkPipeline* pPipelines,
                                                         const RecordObject& record_obj, void* state_data) {
    ValidationStateTracker::PostCallRecordCreateComputePipelines(device, pipelineCache, createInfoCount, pCreateInfos, pAllocator,
                                                                 pPipelines, record_obj, state_data);
    ManualPostCallRecordCreateComputePipelines(device, pipelineCache, createInfoCount, pCreateInfos, pAllocator, pPipelines,
                                               record_obj, state_data);

    if (record_obj.result > VK_SUCCESS) {
        LogPositiveSuccessCode(record_obj);
        return;
    }
    if (record_obj.result < VK_SUCCESS) {
        LogErrorCode(record_obj);
    }
}

void BestPractices::PostCallRecordCreatePipelineLayout(VkDevice device, const VkPipelineLayoutCreateInfo* pCreateInfo,
                                                       const VkAllocationCallbacks* pAllocator, VkPipelineLayout* pPipelineLayout,
                                                       const RecordObject& record_obj) {
    ValidationStateTracker::PostCallRecordCreatePipelineLayout(device, pCreateInfo, pAllocator, pPipelineLayout, record_obj);

    if (record_obj.result < VK_SUCCESS) {
        LogErrorCode(record_obj);
    }
}

void BestPractices::PostCallRecordCreateSampler(VkDevice device, const VkSamplerCreateInfo* pCreateInfo,
                                                const VkAllocationCallbacks* pAllocator, VkSampler* pSampler,
                                                const RecordObject& record_obj) {
    ValidationStateTracker::PostCallRecordCreateSampler(device, pCreateInfo, pAllocator, pSampler, record_obj);

    if (record_obj.result < VK_SUCCESS) {
        LogErrorCode(record_obj);
    }
}

void BestPractices::PostCallRecordCreateDescriptorSetLayout(VkDevice device, const VkDescriptorSetLayoutCreateInfo* pCreateInfo,
                                                            const VkAllocationCallbacks* pAllocator,
                                                            VkDescriptorSetLayout* pSetLayout, const RecordObject& record_obj) {
    ValidationStateTracker::PostCallRecordCreateDescriptorSetLayout(device, pCreateInfo, pAllocator, pSetLayout, record_obj);

    if (record_obj.result < VK_SUCCESS) {
        LogErrorCode(record_obj);
    }
}

void BestPractices::PostCallRecordCreateDescriptorPool(VkDevice device, const VkDescriptorPoolCreateInfo* pCreateInfo,
                                                       const VkAllocationCallbacks* pAllocator, VkDescriptorPool* pDescriptorPool,
                                                       const RecordObject& record_obj) {
    ValidationStateTracker::PostCallRecordCreateDescriptorPool(device, pCreateInfo, pAllocator, pDescriptorPool, record_obj);

    if (record_obj.result < VK_SUCCESS) {
        LogErrorCode(record_obj);
    }
}

void BestPractices::PostCallRecordAllocateDescriptorSets(VkDevice device, const VkDescriptorSetAllocateInfo* pAllocateInfo,
                                                         VkDescriptorSet* pDescriptorSets, const RecordObject& record_obj,
                                                         void* state_data) {
    ValidationStateTracker::PostCallRecordAllocateDescriptorSets(device, pAllocateInfo, pDescriptorSets, record_obj, state_data);
    ManualPostCallRecordAllocateDescriptorSets(device, pAllocateInfo, pDescriptorSets, record_obj, state_data);

    if (record_obj.result < VK_SUCCESS) {
        LogErrorCode(record_obj);
    }
}

void BestPractices::PostCallRecordCreateFramebuffer(VkDevice device, const VkFramebufferCreateInfo* pCreateInfo,
                                                    const VkAllocationCallbacks* pAllocator, VkFramebuffer* pFramebuffer,
                                                    const RecordObject& record_obj) {
    ValidationStateTracker::PostCallRecordCreateFramebuffer(device, pCreateInfo, pAllocator, pFramebuffer, record_obj);

    if (record_obj.result < VK_SUCCESS) {
        LogErrorCode(record_obj);
    }
}

void BestPractices::PostCallRecordCreateRenderPass(VkDevice device, const VkRenderPassCreateInfo* pCreateInfo,
                                                   const VkAllocationCallbacks* pAllocator, VkRenderPass* pRenderPass,
                                                   const RecordObject& record_obj) {
    ValidationStateTracker::PostCallRecordCreateRenderPass(device, pCreateInfo, pAllocator, pRenderPass, record_obj);

    if (record_obj.result < VK_SUCCESS) {
        LogErrorCode(record_obj);
    }
}

void BestPractices::PostCallRecordCreateCommandPool(VkDevice device, const VkCommandPoolCreateInfo* pCreateInfo,
                                                    const VkAllocationCallbacks* pAllocator, VkCommandPool* pCommandPool,
                                                    const RecordObject& record_obj) {
    ValidationStateTracker::PostCallRecordCreateCommandPool(device, pCreateInfo, pAllocator, pCommandPool, record_obj);

    if (record_obj.result < VK_SUCCESS) {
        LogErrorCode(record_obj);
    }
}

void BestPractices::PostCallRecordResetCommandPool(VkDevice device, VkCommandPool commandPool, VkCommandPoolResetFlags flags,
                                                   const RecordObject& record_obj) {
    ValidationStateTracker::PostCallRecordResetCommandPool(device, commandPool, flags, record_obj);

    if (record_obj.result < VK_SUCCESS) {
        LogErrorCode(record_obj);
    }
}

void BestPractices::PostCallRecordAllocateCommandBuffers(VkDevice device, const VkCommandBufferAllocateInfo* pAllocateInfo,
                                                         VkCommandBuffer* pCommandBuffers, const RecordObject& record_obj) {
    ValidationStateTracker::PostCallRecordAllocateCommandBuffers(device, pAllocateInfo, pCommandBuffers, record_obj);

    if (record_obj.result < VK_SUCCESS) {
        LogErrorCode(record_obj);
    }
}

void BestPractices::PostCallRecordBeginCommandBuffer(VkCommandBuffer commandBuffer, const VkCommandBufferBeginInfo* pBeginInfo,
                                                     const RecordObject& record_obj) {
    ValidationStateTracker::PostCallRecordBeginCommandBuffer(commandBuffer, pBeginInfo, record_obj);

    if (record_obj.result < VK_SUCCESS) {
        LogErrorCode(record_obj);
    }
}

void BestPractices::PostCallRecordEndCommandBuffer(VkCommandBuffer commandBuffer, const RecordObject& record_obj) {
    ValidationStateTracker::PostCallRecordEndCommandBuffer(commandBuffer, record_obj);

    if (record_obj.result < VK_SUCCESS) {
        LogErrorCode(record_obj);
    }
}

void BestPractices::PostCallRecordResetCommandBuffer(VkCommandBuffer commandBuffer, VkCommandBufferResetFlags flags,
                                                     const RecordObject& record_obj) {
    ValidationStateTracker::PostCallRecordResetCommandBuffer(commandBuffer, flags, record_obj);

    if (record_obj.result < VK_SUCCESS) {
        LogErrorCode(record_obj);
    }
}

void BestPractices::PostCallRecordBindBufferMemory2(VkDevice device, uint32_t bindInfoCount,
                                                    const VkBindBufferMemoryInfo* pBindInfos, const RecordObject& record_obj) {
    ValidationStateTracker::PostCallRecordBindBufferMemory2(device, bindInfoCount, pBindInfos, record_obj);

    if (record_obj.result < VK_SUCCESS) {
        LogErrorCode(record_obj);
    }
}

void BestPractices::PostCallRecordBindImageMemory2(VkDevice device, uint32_t bindInfoCount, const VkBindImageMemoryInfo* pBindInfos,
                                                   const RecordObject& record_obj) {
    ValidationStateTracker::PostCallRecordBindImageMemory2(device, bindInfoCount, pBindInfos, record_obj);

    if (record_obj.result < VK_SUCCESS) {
        LogErrorCode(record_obj);
    }
}

void BestPractices::PostCallRecordEnumeratePhysicalDeviceGroups(VkInstance instance, uint32_t* pPhysicalDeviceGroupCount,
                                                                VkPhysicalDeviceGroupProperties* pPhysicalDeviceGroupProperties,
                                                                const RecordObject& record_obj) {
    ValidationStateTracker::PostCallRecordEnumeratePhysicalDeviceGroups(instance, pPhysicalDeviceGroupCount,
                                                                        pPhysicalDeviceGroupProperties, record_obj);

    if (record_obj.result > VK_SUCCESS) {
        LogPositiveSuccessCode(record_obj);
        return;
    }
    if (record_obj.result < VK_SUCCESS) {
        LogErrorCode(record_obj);
    }
}

void BestPractices::PostCallRecordGetPhysicalDeviceImageFormatProperties2(VkPhysicalDevice physicalDevice,
                                                                          const VkPhysicalDeviceImageFormatInfo2* pImageFormatInfo,
                                                                          VkImageFormatProperties2* pImageFormatProperties,
                                                                          const RecordObject& record_obj) {
    ValidationStateTracker::PostCallRecordGetPhysicalDeviceImageFormatProperties2(physicalDevice, pImageFormatInfo,
                                                                                  pImageFormatProperties, record_obj);

    if (record_obj.result < VK_SUCCESS) {
        LogErrorCode(record_obj);
    }
}

void BestPractices::PostCallRecordCreateSamplerYcbcrConversion(VkDevice device,
                                                               const VkSamplerYcbcrConversionCreateInfo* pCreateInfo,
                                                               const VkAllocationCallbacks* pAllocator,
                                                               VkSamplerYcbcrConversion* pYcbcrConversion,
                                                               const RecordObject& record_obj) {
    ValidationStateTracker::PostCallRecordCreateSamplerYcbcrConversion(device, pCreateInfo, pAllocator, pYcbcrConversion,
                                                                       record_obj);

    if (record_obj.result < VK_SUCCESS) {
        LogErrorCode(record_obj);
    }
}

void BestPractices::PostCallRecordCreateDescriptorUpdateTemplate(VkDevice device,
                                                                 const VkDescriptorUpdateTemplateCreateInfo* pCreateInfo,
                                                                 const VkAllocationCallbacks* pAllocator,
                                                                 VkDescriptorUpdateTemplate* pDescriptorUpdateTemplate,
                                                                 const RecordObject& record_obj) {
    ValidationStateTracker::PostCallRecordCreateDescriptorUpdateTemplate(device, pCreateInfo, pAllocator, pDescriptorUpdateTemplate,
                                                                         record_obj);

    if (record_obj.result < VK_SUCCESS) {
        LogErrorCode(record_obj);
    }
}

void BestPractices::PostCallRecordCreateRenderPass2(VkDevice device, const VkRenderPassCreateInfo2* pCreateInfo,
                                                    const VkAllocationCallbacks* pAllocator, VkRenderPass* pRenderPass,
                                                    const RecordObject& record_obj) {
    ValidationStateTracker::PostCallRecordCreateRenderPass2(device, pCreateInfo, pAllocator, pRenderPass, record_obj);

    if (record_obj.result < VK_SUCCESS) {
        LogErrorCode(record_obj);
    }
}

void BestPractices::PostCallRecordGetSemaphoreCounterValue(VkDevice device, VkSemaphore semaphore, uint64_t* pValue,
                                                           const RecordObject& record_obj) {
    ValidationStateTracker::PostCallRecordGetSemaphoreCounterValue(device, semaphore, pValue, record_obj);

    if (record_obj.result < VK_SUCCESS) {
        LogErrorCode(record_obj);
    }
}

void BestPractices::PostCallRecordWaitSemaphores(VkDevice device, const VkSemaphoreWaitInfo* pWaitInfo, uint64_t timeout,
                                                 const RecordObject& record_obj) {
    ValidationStateTracker::PostCallRecordWaitSemaphores(device, pWaitInfo, timeout, record_obj);

    if (record_obj.result > VK_SUCCESS) {
        LogPositiveSuccessCode(record_obj);
        return;
    }
    if (record_obj.result < VK_SUCCESS) {
        LogErrorCode(record_obj);
    }
}

void BestPractices::PostCallRecordSignalSemaphore(VkDevice device, const VkSemaphoreSignalInfo* pSignalInfo,
                                                  const RecordObject& record_obj) {
    ValidationStateTracker::PostCallRecordSignalSemaphore(device, pSignalInfo, record_obj);

    if (record_obj.result < VK_SUCCESS) {
        LogErrorCode(record_obj);
    }
}

void BestPractices::PostCallRecordGetPhysicalDeviceToolProperties(VkPhysicalDevice physicalDevice, uint32_t* pToolCount,
                                                                  VkPhysicalDeviceToolProperties* pToolProperties,
                                                                  const RecordObject& record_obj) {
    ValidationStateTracker::PostCallRecordGetPhysicalDeviceToolProperties(physicalDevice, pToolCount, pToolProperties, record_obj);

    if (record_obj.result > VK_SUCCESS) {
        LogPositiveSuccessCode(record_obj);
        return;
    }
    if (record_obj.result < VK_SUCCESS) {
        LogErrorCode(record_obj);
    }
}

void BestPractices::PostCallRecordCreatePrivateDataSlot(VkDevice device, const VkPrivateDataSlotCreateInfo* pCreateInfo,
                                                        const VkAllocationCallbacks* pAllocator,
                                                        VkPrivateDataSlot* pPrivateDataSlot, const RecordObject& record_obj) {
    ValidationStateTracker::PostCallRecordCreatePrivateDataSlot(device, pCreateInfo, pAllocator, pPrivateDataSlot, record_obj);

    if (record_obj.result < VK_SUCCESS) {
        LogErrorCode(record_obj);
    }
}

void BestPractices::PostCallRecordSetPrivateData(VkDevice device, VkObjectType objectType, uint64_t objectHandle,
                                                 VkPrivateDataSlot privateDataSlot, uint64_t data, const RecordObject& record_obj) {
    ValidationStateTracker::PostCallRecordSetPrivateData(device, objectType, objectHandle, privateDataSlot, data, record_obj);

    if (record_obj.result < VK_SUCCESS) {
        LogErrorCode(record_obj);
    }
}

void BestPractices::PostCallRecordQueueSubmit2(VkQueue queue, uint32_t submitCount, const VkSubmitInfo2* pSubmits, VkFence fence,
                                               const RecordObject& record_obj) {
    ValidationStateTracker::PostCallRecordQueueSubmit2(queue, submitCount, pSubmits, fence, record_obj);

    if (record_obj.result < VK_SUCCESS) {
        LogErrorCode(record_obj);
    }
}

void BestPractices::PostCallRecordGetPhysicalDeviceSurfaceSupportKHR(VkPhysicalDevice physicalDevice, uint32_t queueFamilyIndex,
                                                                     VkSurfaceKHR surface, VkBool32* pSupported,
                                                                     const RecordObject& record_obj) {
    ValidationStateTracker::PostCallRecordGetPhysicalDeviceSurfaceSupportKHR(physicalDevice, queueFamilyIndex, surface, pSupported,
                                                                             record_obj);

    if (record_obj.result < VK_SUCCESS) {
        LogErrorCode(record_obj);
    }
}

void BestPractices::PostCallRecordGetPhysicalDeviceSurfaceCapabilitiesKHR(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface,
                                                                          VkSurfaceCapabilitiesKHR* pSurfaceCapabilities,
                                                                          const RecordObject& record_obj) {
    ValidationStateTracker::PostCallRecordGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice, surface, pSurfaceCapabilities,
                                                                                  record_obj);
    ManualPostCallRecordGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice, surface, pSurfaceCapabilities, record_obj);

    if (record_obj.result < VK_SUCCESS) {
        LogErrorCode(record_obj);
    }
}

void BestPractices::PostCallRecordGetPhysicalDeviceSurfaceFormatsKHR(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface,
                                                                     uint32_t* pSurfaceFormatCount,
                                                                     VkSurfaceFormatKHR* pSurfaceFormats,
                                                                     const RecordObject& record_obj) {
    ValidationStateTracker::PostCallRecordGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, pSurfaceFormatCount,
                                                                             pSurfaceFormats, record_obj);
    ManualPostCallRecordGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, pSurfaceFormatCount, pSurfaceFormats,
                                                           record_obj);

    if (record_obj.result > VK_SUCCESS) {
        LogPositiveSuccessCode(record_obj);
        return;
    }
    if (record_obj.result < VK_SUCCESS) {
        LogErrorCode(record_obj);
    }
}

void BestPractices::PostCallRecordGetPhysicalDeviceSurfacePresentModesKHR(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface,
                                                                          uint32_t* pPresentModeCount,
                                                                          VkPresentModeKHR* pPresentModes,
                                                                          const RecordObject& record_obj) {
    ValidationStateTracker::PostCallRecordGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface, pPresentModeCount,
                                                                                  pPresentModes, record_obj);
    ManualPostCallRecordGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface, pPresentModeCount, pPresentModes,
                                                                record_obj);

    if (record_obj.result > VK_SUCCESS) {
        LogPositiveSuccessCode(record_obj);
        return;
    }
    if (record_obj.result < VK_SUCCESS) {
        LogErrorCode(record_obj);
    }
}

void BestPractices::PostCallRecordCreateSwapchainKHR(VkDevice device, const VkSwapchainCreateInfoKHR* pCreateInfo,
                                                     const VkAllocationCallbacks* pAllocator, VkSwapchainKHR* pSwapchain,
                                                     const RecordObject& record_obj) {
    ValidationStateTracker::PostCallRecordCreateSwapchainKHR(device, pCreateInfo, pAllocator, pSwapchain, record_obj);

    if (record_obj.result < VK_SUCCESS) {
        LogErrorCode(record_obj);
    }
}

void BestPractices::PostCallRecordGetSwapchainImagesKHR(VkDevice device, VkSwapchainKHR swapchain, uint32_t* pSwapchainImageCount,
                                                        VkImage* pSwapchainImages, const RecordObject& record_obj) {
    ValidationStateTracker::PostCallRecordGetSwapchainImagesKHR(device, swapchain, pSwapchainImageCount, pSwapchainImages,
                                                                record_obj);
    ManualPostCallRecordGetSwapchainImagesKHR(device, swapchain, pSwapchainImageCount, pSwapchainImages, record_obj);

    if (record_obj.result > VK_SUCCESS) {
        LogPositiveSuccessCode(record_obj);
        return;
    }
    if (record_obj.result < VK_SUCCESS) {
        LogErrorCode(record_obj);
    }
}

void BestPractices::PostCallRecordAcquireNextImageKHR(VkDevice device, VkSwapchainKHR swapchain, uint64_t timeout,
                                                      VkSemaphore semaphore, VkFence fence, uint32_t* pImageIndex,
                                                      const RecordObject& record_obj) {
    ValidationStateTracker::PostCallRecordAcquireNextImageKHR(device, swapchain, timeout, semaphore, fence, pImageIndex,
                                                              record_obj);

    if (record_obj.result > VK_SUCCESS) {
        LogPositiveSuccessCode(record_obj);
        return;
    }
    if (record_obj.result < VK_SUCCESS) {
        LogErrorCode(record_obj);
    }
}

void BestPractices::PostCallRecordQueuePresentKHR(VkQueue queue, const VkPresentInfoKHR* pPresentInfo,
                                                  const RecordObject& record_obj) {
    ValidationStateTracker::PostCallRecordQueuePresentKHR(queue, pPresentInfo, record_obj);
    ManualPostCallRecordQueuePresentKHR(queue, pPresentInfo, record_obj);

    if (record_obj.result > VK_SUCCESS) {
        LogPositiveSuccessCode(record_obj);
        return;
    }
    if (record_obj.result < VK_SUCCESS) {
        LogErrorCode(record_obj);
    }
}

void BestPractices::PostCallRecordGetDeviceGroupPresentCapabilitiesKHR(
    VkDevice device, VkDeviceGroupPresentCapabilitiesKHR* pDeviceGroupPresentCapabilities, const RecordObject& record_obj) {
    ValidationStateTracker::PostCallRecordGetDeviceGroupPresentCapabilitiesKHR(device, pDeviceGroupPresentCapabilities, record_obj);

    if (record_obj.result < VK_SUCCESS) {
        LogErrorCode(record_obj);
    }
}

void BestPractices::PostCallRecordGetDeviceGroupSurfacePresentModesKHR(VkDevice device, VkSurfaceKHR surface,
                                                                       VkDeviceGroupPresentModeFlagsKHR* pModes,
                                                                       const RecordObject& record_obj) {
    ValidationStateTracker::PostCallRecordGetDeviceGroupSurfacePresentModesKHR(device, surface, pModes, record_obj);

    if (record_obj.result < VK_SUCCESS) {
        LogErrorCode(record_obj);
    }
}

void BestPractices::PostCallRecordGetPhysicalDevicePresentRectanglesKHR(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface,
                                                                        uint32_t* pRectCount, VkRect2D* pRects,
                                                                        const RecordObject& record_obj) {
    ValidationStateTracker::PostCallRecordGetPhysicalDevicePresentRectanglesKHR(physicalDevice, surface, pRectCount, pRects,
                                                                                record_obj);

    if (record_obj.result > VK_SUCCESS) {
        LogPositiveSuccessCode(record_obj);
        return;
    }
    if (record_obj.result < VK_SUCCESS) {
        LogErrorCode(record_obj);
    }
}

void BestPractices::PostCallRecordAcquireNextImage2KHR(VkDevice device, const VkAcquireNextImageInfoKHR* pAcquireInfo,
                                                       uint32_t* pImageIndex, const RecordObject& record_obj) {
    ValidationStateTracker::PostCallRecordAcquireNextImage2KHR(device, pAcquireInfo, pImageIndex, record_obj);

    if (record_obj.result > VK_SUCCESS) {
        LogPositiveSuccessCode(record_obj);
        return;
    }
    if (record_obj.result < VK_SUCCESS) {
        LogErrorCode(record_obj);
    }
}

void BestPractices::PostCallRecordGetPhysicalDeviceDisplayPropertiesKHR(VkPhysicalDevice physicalDevice, uint32_t* pPropertyCount,
                                                                        VkDisplayPropertiesKHR* pProperties,
                                                                        const RecordObject& record_obj) {
    ValidationStateTracker::PostCallRecordGetPhysicalDeviceDisplayPropertiesKHR(physicalDevice, pPropertyCount, pProperties,
                                                                                record_obj);

    if (record_obj.result > VK_SUCCESS) {
        LogPositiveSuccessCode(record_obj);
        return;
    }
    if (record_obj.result < VK_SUCCESS) {
        LogErrorCode(record_obj);
    }
}

void BestPractices::PostCallRecordGetPhysicalDeviceDisplayPlanePropertiesKHR(VkPhysicalDevice physicalDevice,
                                                                             uint32_t* pPropertyCount,
                                                                             VkDisplayPlanePropertiesKHR* pProperties,
                                                                             const RecordObject& record_obj) {
    ValidationStateTracker::PostCallRecordGetPhysicalDeviceDisplayPlanePropertiesKHR(physicalDevice, pPropertyCount, pProperties,
                                                                                     record_obj);
    ManualPostCallRecordGetPhysicalDeviceDisplayPlanePropertiesKHR(physicalDevice, pPropertyCount, pProperties, record_obj);

    if (record_obj.result > VK_SUCCESS) {
        LogPositiveSuccessCode(record_obj);
        return;
    }
    if (record_obj.result < VK_SUCCESS) {
        LogErrorCode(record_obj);
    }
}

void BestPractices::PostCallRecordGetDisplayPlaneSupportedDisplaysKHR(VkPhysicalDevice physicalDevice, uint32_t planeIndex,
                                                                      uint32_t* pDisplayCount, VkDisplayKHR* pDisplays,
                                                                      const RecordObject& record_obj) {
    ValidationStateTracker::PostCallRecordGetDisplayPlaneSupportedDisplaysKHR(physicalDevice, planeIndex, pDisplayCount, pDisplays,
                                                                              record_obj);

    if (record_obj.result > VK_SUCCESS) {
        LogPositiveSuccessCode(record_obj);
        return;
    }
    if (record_obj.result < VK_SUCCESS) {
        LogErrorCode(record_obj);
    }
}

void BestPractices::PostCallRecordGetDisplayModePropertiesKHR(VkPhysicalDevice physicalDevice, VkDisplayKHR display,
                                                              uint32_t* pPropertyCount, VkDisplayModePropertiesKHR* pProperties,
                                                              const RecordObject& record_obj) {
    ValidationStateTracker::PostCallRecordGetDisplayModePropertiesKHR(physicalDevice, display, pPropertyCount, pProperties,
                                                                      record_obj);

    if (record_obj.result > VK_SUCCESS) {
        LogPositiveSuccessCode(record_obj);
        return;
    }
    if (record_obj.result < VK_SUCCESS) {
        LogErrorCode(record_obj);
    }
}

void BestPractices::PostCallRecordCreateDisplayModeKHR(VkPhysicalDevice physicalDevice, VkDisplayKHR display,
                                                       const VkDisplayModeCreateInfoKHR* pCreateInfo,
                                                       const VkAllocationCallbacks* pAllocator, VkDisplayModeKHR* pMode,
                                                       const RecordObject& record_obj) {
    ValidationStateTracker::PostCallRecordCreateDisplayModeKHR(physicalDevice, display, pCreateInfo, pAllocator, pMode, record_obj);

    if (record_obj.result < VK_SUCCESS) {
        LogErrorCode(record_obj);
    }
}

void BestPractices::PostCallRecordGetDisplayPlaneCapabilitiesKHR(VkPhysicalDevice physicalDevice, VkDisplayModeKHR mode,
                                                                 uint32_t planeIndex, VkDisplayPlaneCapabilitiesKHR* pCapabilities,
                                                                 const RecordObject& record_obj) {
    ValidationStateTracker::PostCallRecordGetDisplayPlaneCapabilitiesKHR(physicalDevice, mode, planeIndex, pCapabilities,
                                                                         record_obj);

    if (record_obj.result < VK_SUCCESS) {
        LogErrorCode(record_obj);
    }
}

void BestPractices::PostCallRecordCreateDisplayPlaneSurfaceKHR(VkInstance instance,
                                                               const VkDisplaySurfaceCreateInfoKHR* pCreateInfo,
                                                               const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface,
                                                               const RecordObject& record_obj) {
    ValidationStateTracker::PostCallRecordCreateDisplayPlaneSurfaceKHR(instance, pCreateInfo, pAllocator, pSurface, record_obj);

    if (record_obj.result < VK_SUCCESS) {
        LogErrorCode(record_obj);
    }
}

void BestPractices::PostCallRecordCreateSharedSwapchainsKHR(VkDevice device, uint32_t swapchainCount,
                                                            const VkSwapchainCreateInfoKHR* pCreateInfos,
                                                            const VkAllocationCallbacks* pAllocator, VkSwapchainKHR* pSwapchains,
                                                            const RecordObject& record_obj) {
    ValidationStateTracker::PostCallRecordCreateSharedSwapchainsKHR(device, swapchainCount, pCreateInfos, pAllocator, pSwapchains,
                                                                    record_obj);

    if (record_obj.result < VK_SUCCESS) {
        LogErrorCode(record_obj);
    }
}

#ifdef VK_USE_PLATFORM_XLIB_KHR
void BestPractices::PostCallRecordCreateXlibSurfaceKHR(VkInstance instance, const VkXlibSurfaceCreateInfoKHR* pCreateInfo,
                                                       const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface,
                                                       const RecordObject& record_obj) {
    ValidationStateTracker::PostCallRecordCreateXlibSurfaceKHR(instance, pCreateInfo, pAllocator, pSurface, record_obj);

    if (record_obj.result < VK_SUCCESS) {
        LogErrorCode(record_obj);
    }
}
#endif  // VK_USE_PLATFORM_XLIB_KHR

#ifdef VK_USE_PLATFORM_XCB_KHR
void BestPractices::PostCallRecordCreateXcbSurfaceKHR(VkInstance instance, const VkXcbSurfaceCreateInfoKHR* pCreateInfo,
                                                      const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface,
                                                      const RecordObject& record_obj) {
    ValidationStateTracker::PostCallRecordCreateXcbSurfaceKHR(instance, pCreateInfo, pAllocator, pSurface, record_obj);

    if (record_obj.result < VK_SUCCESS) {
        LogErrorCode(record_obj);
    }
}
#endif  // VK_USE_PLATFORM_XCB_KHR

#ifdef VK_USE_PLATFORM_WAYLAND_KHR
void BestPractices::PostCallRecordCreateWaylandSurfaceKHR(VkInstance instance, const VkWaylandSurfaceCreateInfoKHR* pCreateInfo,
                                                          const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface,
                                                          const RecordObject& record_obj) {
    ValidationStateTracker::PostCallRecordCreateWaylandSurfaceKHR(instance, pCreateInfo, pAllocator, pSurface, record_obj);

    if (record_obj.result < VK_SUCCESS) {
        LogErrorCode(record_obj);
    }
}
#endif  // VK_USE_PLATFORM_WAYLAND_KHR

#ifdef VK_USE_PLATFORM_ANDROID_KHR
void BestPractices::PostCallRecordCreateAndroidSurfaceKHR(VkInstance instance, const VkAndroidSurfaceCreateInfoKHR* pCreateInfo,
                                                          const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface,
                                                          const RecordObject& record_obj) {
    ValidationStateTracker::PostCallRecordCreateAndroidSurfaceKHR(instance, pCreateInfo, pAllocator, pSurface, record_obj);

    if (record_obj.result < VK_SUCCESS) {
        LogErrorCode(record_obj);
    }
}
#endif  // VK_USE_PLATFORM_ANDROID_KHR

#ifdef VK_USE_PLATFORM_WIN32_KHR
void BestPractices::PostCallRecordCreateWin32SurfaceKHR(VkInstance instance, const VkWin32SurfaceCreateInfoKHR* pCreateInfo,
                                                        const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface,
                                                        const RecordObject& record_obj) {
    ValidationStateTracker::PostCallRecordCreateWin32SurfaceKHR(instance, pCreateInfo, pAllocator, pSurface, record_obj);

    if (record_obj.result < VK_SUCCESS) {
        LogErrorCode(record_obj);
    }
}
#endif  // VK_USE_PLATFORM_WIN32_KHR

void BestPractices::PostCallRecordGetPhysicalDeviceVideoCapabilitiesKHR(VkPhysicalDevice physicalDevice,
                                                                        const VkVideoProfileInfoKHR* pVideoProfile,
                                                                        VkVideoCapabilitiesKHR* pCapabilities,
                                                                        const RecordObject& record_obj) {
    ValidationStateTracker::PostCallRecordGetPhysicalDeviceVideoCapabilitiesKHR(physicalDevice, pVideoProfile, pCapabilities,
                                                                                record_obj);

    if (record_obj.result < VK_SUCCESS) {
        LogErrorCode(record_obj);
    }
}

void BestPractices::PostCallRecordGetPhysicalDeviceVideoFormatPropertiesKHR(
    VkPhysicalDevice physicalDevice, const VkPhysicalDeviceVideoFormatInfoKHR* pVideoFormatInfo,
    uint32_t* pVideoFormatPropertyCount, VkVideoFormatPropertiesKHR* pVideoFormatProperties, const RecordObject& record_obj) {
    ValidationStateTracker::PostCallRecordGetPhysicalDeviceVideoFormatPropertiesKHR(
        physicalDevice, pVideoFormatInfo, pVideoFormatPropertyCount, pVideoFormatProperties, record_obj);

    if (record_obj.result > VK_SUCCESS) {
        LogPositiveSuccessCode(record_obj);
        return;
    }
    if (record_obj.result < VK_SUCCESS) {
        LogErrorCode(record_obj);
    }
}

void BestPractices::PostCallRecordCreateVideoSessionKHR(VkDevice device, const VkVideoSessionCreateInfoKHR* pCreateInfo,
                                                        const VkAllocationCallbacks* pAllocator, VkVideoSessionKHR* pVideoSession,
                                                        const RecordObject& record_obj) {
    ValidationStateTracker::PostCallRecordCreateVideoSessionKHR(device, pCreateInfo, pAllocator, pVideoSession, record_obj);

    if (record_obj.result < VK_SUCCESS) {
        LogErrorCode(record_obj);
    }
}

void BestPractices::PostCallRecordGetVideoSessionMemoryRequirementsKHR(VkDevice device, VkVideoSessionKHR videoSession,
                                                                       uint32_t* pMemoryRequirementsCount,
                                                                       VkVideoSessionMemoryRequirementsKHR* pMemoryRequirements,
                                                                       const RecordObject& record_obj) {
    ValidationStateTracker::PostCallRecordGetVideoSessionMemoryRequirementsKHR(device, videoSession, pMemoryRequirementsCount,
                                                                               pMemoryRequirements, record_obj);

    if (record_obj.result > VK_SUCCESS) {
        LogPositiveSuccessCode(record_obj);
        return;
    }
}

void BestPractices::PostCallRecordBindVideoSessionMemoryKHR(VkDevice device, VkVideoSessionKHR videoSession,
                                                            uint32_t bindSessionMemoryInfoCount,
                                                            const VkBindVideoSessionMemoryInfoKHR* pBindSessionMemoryInfos,
                                                            const RecordObject& record_obj) {
    ValidationStateTracker::PostCallRecordBindVideoSessionMemoryKHR(device, videoSession, bindSessionMemoryInfoCount,
                                                                    pBindSessionMemoryInfos, record_obj);

    if (record_obj.result < VK_SUCCESS) {
        LogErrorCode(record_obj);
    }
}

void BestPractices::PostCallRecordCreateVideoSessionParametersKHR(VkDevice device,
                                                                  const VkVideoSessionParametersCreateInfoKHR* pCreateInfo,
                                                                  const VkAllocationCallbacks* pAllocator,
                                                                  VkVideoSessionParametersKHR* pVideoSessionParameters,
                                                                  const RecordObject& record_obj) {
    ValidationStateTracker::PostCallRecordCreateVideoSessionParametersKHR(device, pCreateInfo, pAllocator, pVideoSessionParameters,
                                                                          record_obj);

    if (record_obj.result < VK_SUCCESS) {
        LogErrorCode(record_obj);
    }
}

void BestPractices::PostCallRecordUpdateVideoSessionParametersKHR(VkDevice device,
                                                                  VkVideoSessionParametersKHR videoSessionParameters,
                                                                  const VkVideoSessionParametersUpdateInfoKHR* pUpdateInfo,
                                                                  const RecordObject& record_obj) {
    ValidationStateTracker::PostCallRecordUpdateVideoSessionParametersKHR(device, videoSessionParameters, pUpdateInfo, record_obj);

    if (record_obj.result < VK_SUCCESS) {
        LogErrorCode(record_obj);
    }
}

void BestPractices::PostCallRecordGetPhysicalDeviceImageFormatProperties2KHR(
    VkPhysicalDevice physicalDevice, const VkPhysicalDeviceImageFormatInfo2* pImageFormatInfo,
    VkImageFormatProperties2* pImageFormatProperties, const RecordObject& record_obj) {
    ValidationStateTracker::PostCallRecordGetPhysicalDeviceImageFormatProperties2KHR(physicalDevice, pImageFormatInfo,
                                                                                     pImageFormatProperties, record_obj);

    if (record_obj.result < VK_SUCCESS) {
        LogErrorCode(record_obj);
    }
}

void BestPractices::PostCallRecordEnumeratePhysicalDeviceGroupsKHR(VkInstance instance, uint32_t* pPhysicalDeviceGroupCount,
                                                                   VkPhysicalDeviceGroupProperties* pPhysicalDeviceGroupProperties,
                                                                   const RecordObject& record_obj) {
    ValidationStateTracker::PostCallRecordEnumeratePhysicalDeviceGroupsKHR(instance, pPhysicalDeviceGroupCount,
                                                                           pPhysicalDeviceGroupProperties, record_obj);

    if (record_obj.result > VK_SUCCESS) {
        LogPositiveSuccessCode(record_obj);
        return;
    }
    if (record_obj.result < VK_SUCCESS) {
        LogErrorCode(record_obj);
    }
}

#ifdef VK_USE_PLATFORM_WIN32_KHR
void BestPractices::PostCallRecordGetMemoryWin32HandleKHR(VkDevice device, const VkMemoryGetWin32HandleInfoKHR* pGetWin32HandleInfo,
                                                          HANDLE* pHandle, const RecordObject& record_obj) {
    ValidationStateTracker::PostCallRecordGetMemoryWin32HandleKHR(device, pGetWin32HandleInfo, pHandle, record_obj);

    if (record_obj.result < VK_SUCCESS) {
        LogErrorCode(record_obj);
    }
}
#endif  // VK_USE_PLATFORM_WIN32_KHR

#ifdef VK_USE_PLATFORM_WIN32_KHR
void BestPractices::PostCallRecordGetMemoryWin32HandlePropertiesKHR(VkDevice device, VkExternalMemoryHandleTypeFlagBits handleType,
                                                                    HANDLE handle,
                                                                    VkMemoryWin32HandlePropertiesKHR* pMemoryWin32HandleProperties,
                                                                    const RecordObject& record_obj) {
    ValidationStateTracker::PostCallRecordGetMemoryWin32HandlePropertiesKHR(device, handleType, handle,
                                                                            pMemoryWin32HandleProperties, record_obj);

    if (record_obj.result < VK_SUCCESS) {
        LogErrorCode(record_obj);
    }
}
#endif  // VK_USE_PLATFORM_WIN32_KHR

void BestPractices::PostCallRecordGetMemoryFdKHR(VkDevice device, const VkMemoryGetFdInfoKHR* pGetFdInfo, int* pFd,
                                                 const RecordObject& record_obj) {
    ValidationStateTracker::PostCallRecordGetMemoryFdKHR(device, pGetFdInfo, pFd, record_obj);

    if (record_obj.result < VK_SUCCESS) {
        LogErrorCode(record_obj);
    }
}

void BestPractices::PostCallRecordGetMemoryFdPropertiesKHR(VkDevice device, VkExternalMemoryHandleTypeFlagBits handleType, int fd,
                                                           VkMemoryFdPropertiesKHR* pMemoryFdProperties,
                                                           const RecordObject& record_obj) {
    ValidationStateTracker::PostCallRecordGetMemoryFdPropertiesKHR(device, handleType, fd, pMemoryFdProperties, record_obj);

    if (record_obj.result < VK_SUCCESS) {
        LogErrorCode(record_obj);
    }
}

#ifdef VK_USE_PLATFORM_WIN32_KHR
void BestPractices::PostCallRecordImportSemaphoreWin32HandleKHR(
    VkDevice device, const VkImportSemaphoreWin32HandleInfoKHR* pImportSemaphoreWin32HandleInfo, const RecordObject& record_obj) {
    ValidationStateTracker::PostCallRecordImportSemaphoreWin32HandleKHR(device, pImportSemaphoreWin32HandleInfo, record_obj);

    if (record_obj.result < VK_SUCCESS) {
        LogErrorCode(record_obj);
    }
}
#endif  // VK_USE_PLATFORM_WIN32_KHR

#ifdef VK_USE_PLATFORM_WIN32_KHR
void BestPractices::PostCallRecordGetSemaphoreWin32HandleKHR(VkDevice device,
                                                             const VkSemaphoreGetWin32HandleInfoKHR* pGetWin32HandleInfo,
                                                             HANDLE* pHandle, const RecordObject& record_obj) {
    ValidationStateTracker::PostCallRecordGetSemaphoreWin32HandleKHR(device, pGetWin32HandleInfo, pHandle, record_obj);

    if (record_obj.result < VK_SUCCESS) {
        LogErrorCode(record_obj);
    }
}
#endif  // VK_USE_PLATFORM_WIN32_KHR

void BestPractices::PostCallRecordImportSemaphoreFdKHR(VkDevice device, const VkImportSemaphoreFdInfoKHR* pImportSemaphoreFdInfo,
                                                       const RecordObject& record_obj) {
    ValidationStateTracker::PostCallRecordImportSemaphoreFdKHR(device, pImportSemaphoreFdInfo, record_obj);

    if (record_obj.result < VK_SUCCESS) {
        LogErrorCode(record_obj);
    }
}

void BestPractices::PostCallRecordGetSemaphoreFdKHR(VkDevice device, const VkSemaphoreGetFdInfoKHR* pGetFdInfo, int* pFd,
                                                    const RecordObject& record_obj) {
    ValidationStateTracker::PostCallRecordGetSemaphoreFdKHR(device, pGetFdInfo, pFd, record_obj);

    if (record_obj.result < VK_SUCCESS) {
        LogErrorCode(record_obj);
    }
}

void BestPractices::PostCallRecordCreateDescriptorUpdateTemplateKHR(VkDevice device,
                                                                    const VkDescriptorUpdateTemplateCreateInfo* pCreateInfo,
                                                                    const VkAllocationCallbacks* pAllocator,
                                                                    VkDescriptorUpdateTemplate* pDescriptorUpdateTemplate,
                                                                    const RecordObject& record_obj) {
    ValidationStateTracker::PostCallRecordCreateDescriptorUpdateTemplateKHR(device, pCreateInfo, pAllocator,
                                                                            pDescriptorUpdateTemplate, record_obj);

    if (record_obj.result < VK_SUCCESS) {
        LogErrorCode(record_obj);
    }
}

void BestPractices::PostCallRecordCreateRenderPass2KHR(VkDevice device, const VkRenderPassCreateInfo2* pCreateInfo,
                                                       const VkAllocationCallbacks* pAllocator, VkRenderPass* pRenderPass,
                                                       const RecordObject& record_obj) {
    ValidationStateTracker::PostCallRecordCreateRenderPass2KHR(device, pCreateInfo, pAllocator, pRenderPass, record_obj);

    if (record_obj.result < VK_SUCCESS) {
        LogErrorCode(record_obj);
    }
}

void BestPractices::PostCallRecordGetSwapchainStatusKHR(VkDevice device, VkSwapchainKHR swapchain, const RecordObject& record_obj) {
    ValidationStateTracker::PostCallRecordGetSwapchainStatusKHR(device, swapchain, record_obj);

    if (record_obj.result > VK_SUCCESS) {
        LogPositiveSuccessCode(record_obj);
        return;
    }
    if (record_obj.result < VK_SUCCESS) {
        LogErrorCode(record_obj);
    }
}

#ifdef VK_USE_PLATFORM_WIN32_KHR
void BestPractices::PostCallRecordImportFenceWin32HandleKHR(VkDevice device,
                                                            const VkImportFenceWin32HandleInfoKHR* pImportFenceWin32HandleInfo,
                                                            const RecordObject& record_obj) {
    ValidationStateTracker::PostCallRecordImportFenceWin32HandleKHR(device, pImportFenceWin32HandleInfo, record_obj);

    if (record_obj.result < VK_SUCCESS) {
        LogErrorCode(record_obj);
    }
}
#endif  // VK_USE_PLATFORM_WIN32_KHR

#ifdef VK_USE_PLATFORM_WIN32_KHR
void BestPractices::PostCallRecordGetFenceWin32HandleKHR(VkDevice device, const VkFenceGetWin32HandleInfoKHR* pGetWin32HandleInfo,
                                                         HANDLE* pHandle, const RecordObject& record_obj) {
    ValidationStateTracker::PostCallRecordGetFenceWin32HandleKHR(device, pGetWin32HandleInfo, pHandle, record_obj);

    if (record_obj.result < VK_SUCCESS) {
        LogErrorCode(record_obj);
    }
}
#endif  // VK_USE_PLATFORM_WIN32_KHR

void BestPractices::PostCallRecordImportFenceFdKHR(VkDevice device, const VkImportFenceFdInfoKHR* pImportFenceFdInfo,
                                                   const RecordObject& record_obj) {
    ValidationStateTracker::PostCallRecordImportFenceFdKHR(device, pImportFenceFdInfo, record_obj);

    if (record_obj.result < VK_SUCCESS) {
        LogErrorCode(record_obj);
    }
}

void BestPractices::PostCallRecordGetFenceFdKHR(VkDevice device, const VkFenceGetFdInfoKHR* pGetFdInfo, int* pFd,
                                                const RecordObject& record_obj) {
    ValidationStateTracker::PostCallRecordGetFenceFdKHR(device, pGetFdInfo, pFd, record_obj);

    if (record_obj.result < VK_SUCCESS) {
        LogErrorCode(record_obj);
    }
}

void BestPractices::PostCallRecordEnumeratePhysicalDeviceQueueFamilyPerformanceQueryCountersKHR(
    VkPhysicalDevice physicalDevice, uint32_t queueFamilyIndex, uint32_t* pCounterCount, VkPerformanceCounterKHR* pCounters,
    VkPerformanceCounterDescriptionKHR* pCounterDescriptions, const RecordObject& record_obj) {
    ValidationStateTracker::PostCallRecordEnumeratePhysicalDeviceQueueFamilyPerformanceQueryCountersKHR(
        physicalDevice, queueFamilyIndex, pCounterCount, pCounters, pCounterDescriptions, record_obj);

    if (record_obj.result > VK_SUCCESS) {
        LogPositiveSuccessCode(record_obj);
        return;
    }
    if (record_obj.result < VK_SUCCESS) {
        LogErrorCode(record_obj);
    }
}

void BestPractices::PostCallRecordAcquireProfilingLockKHR(VkDevice device, const VkAcquireProfilingLockInfoKHR* pInfo,
                                                          const RecordObject& record_obj) {
    ValidationStateTracker::PostCallRecordAcquireProfilingLockKHR(device, pInfo, record_obj);

    if (record_obj.result < VK_SUCCESS) {
        LogErrorCode(record_obj);
    }
}

void BestPractices::PostCallRecordGetPhysicalDeviceSurfaceCapabilities2KHR(VkPhysicalDevice physicalDevice,
                                                                           const VkPhysicalDeviceSurfaceInfo2KHR* pSurfaceInfo,
                                                                           VkSurfaceCapabilities2KHR* pSurfaceCapabilities,
                                                                           const RecordObject& record_obj) {
    ValidationStateTracker::PostCallRecordGetPhysicalDeviceSurfaceCapabilities2KHR(physicalDevice, pSurfaceInfo,
                                                                                   pSurfaceCapabilities, record_obj);
    ManualPostCallRecordGetPhysicalDeviceSurfaceCapabilities2KHR(physicalDevice, pSurfaceInfo, pSurfaceCapabilities, record_obj);

    if (record_obj.result < VK_SUCCESS) {
        LogErrorCode(record_obj);
    }
}

void BestPractices::PostCallRecordGetPhysicalDeviceSurfaceFormats2KHR(VkPhysicalDevice physicalDevice,
                                                                      const VkPhysicalDeviceSurfaceInfo2KHR* pSurfaceInfo,
                                                                      uint32_t* pSurfaceFormatCount,
                                                                      VkSurfaceFormat2KHR* pSurfaceFormats,
                                                                      const RecordObject& record_obj) {
    ValidationStateTracker::PostCallRecordGetPhysicalDeviceSurfaceFormats2KHR(physicalDevice, pSurfaceInfo, pSurfaceFormatCount,
                                                                              pSurfaceFormats, record_obj);
    ManualPostCallRecordGetPhysicalDeviceSurfaceFormats2KHR(physicalDevice, pSurfaceInfo, pSurfaceFormatCount, pSurfaceFormats,
                                                            record_obj);

    if (record_obj.result > VK_SUCCESS) {
        LogPositiveSuccessCode(record_obj);
        return;
    }
    if (record_obj.result < VK_SUCCESS) {
        LogErrorCode(record_obj);
    }
}

void BestPractices::PostCallRecordGetPhysicalDeviceDisplayProperties2KHR(VkPhysicalDevice physicalDevice, uint32_t* pPropertyCount,
                                                                         VkDisplayProperties2KHR* pProperties,
                                                                         const RecordObject& record_obj) {
    ValidationStateTracker::PostCallRecordGetPhysicalDeviceDisplayProperties2KHR(physicalDevice, pPropertyCount, pProperties,
                                                                                 record_obj);

    if (record_obj.result > VK_SUCCESS) {
        LogPositiveSuccessCode(record_obj);
        return;
    }
    if (record_obj.result < VK_SUCCESS) {
        LogErrorCode(record_obj);
    }
}

void BestPractices::PostCallRecordGetPhysicalDeviceDisplayPlaneProperties2KHR(VkPhysicalDevice physicalDevice,
                                                                              uint32_t* pPropertyCount,
                                                                              VkDisplayPlaneProperties2KHR* pProperties,
                                                                              const RecordObject& record_obj) {
    ValidationStateTracker::PostCallRecordGetPhysicalDeviceDisplayPlaneProperties2KHR(physicalDevice, pPropertyCount, pProperties,
                                                                                      record_obj);

    if (record_obj.result > VK_SUCCESS) {
        LogPositiveSuccessCode(record_obj);
        return;
    }
    if (record_obj.result < VK_SUCCESS) {
        LogErrorCode(record_obj);
    }
}

void BestPractices::PostCallRecordGetDisplayModeProperties2KHR(VkPhysicalDevice physicalDevice, VkDisplayKHR display,
                                                               uint32_t* pPropertyCount, VkDisplayModeProperties2KHR* pProperties,
                                                               const RecordObject& record_obj) {
    ValidationStateTracker::PostCallRecordGetDisplayModeProperties2KHR(physicalDevice, display, pPropertyCount, pProperties,
                                                                       record_obj);

    if (record_obj.result > VK_SUCCESS) {
        LogPositiveSuccessCode(record_obj);
        return;
    }
    if (record_obj.result < VK_SUCCESS) {
        LogErrorCode(record_obj);
    }
}

void BestPractices::PostCallRecordGetDisplayPlaneCapabilities2KHR(VkPhysicalDevice physicalDevice,
                                                                  const VkDisplayPlaneInfo2KHR* pDisplayPlaneInfo,
                                                                  VkDisplayPlaneCapabilities2KHR* pCapabilities,
                                                                  const RecordObject& record_obj) {
    ValidationStateTracker::PostCallRecordGetDisplayPlaneCapabilities2KHR(physicalDevice, pDisplayPlaneInfo, pCapabilities,
                                                                          record_obj);

    if (record_obj.result < VK_SUCCESS) {
        LogErrorCode(record_obj);
    }
}

void BestPractices::PostCallRecordCreateSamplerYcbcrConversionKHR(VkDevice device,
                                                                  const VkSamplerYcbcrConversionCreateInfo* pCreateInfo,
                                                                  const VkAllocationCallbacks* pAllocator,
                                                                  VkSamplerYcbcrConversion* pYcbcrConversion,
                                                                  const RecordObject& record_obj) {
    ValidationStateTracker::PostCallRecordCreateSamplerYcbcrConversionKHR(device, pCreateInfo, pAllocator, pYcbcrConversion,
                                                                          record_obj);

    if (record_obj.result < VK_SUCCESS) {
        LogErrorCode(record_obj);
    }
}

void BestPractices::PostCallRecordBindBufferMemory2KHR(VkDevice device, uint32_t bindInfoCount,
                                                       const VkBindBufferMemoryInfo* pBindInfos, const RecordObject& record_obj) {
    ValidationStateTracker::PostCallRecordBindBufferMemory2KHR(device, bindInfoCount, pBindInfos, record_obj);

    if (record_obj.result < VK_SUCCESS) {
        LogErrorCode(record_obj);
    }
}

void BestPractices::PostCallRecordBindImageMemory2KHR(VkDevice device, uint32_t bindInfoCount,
                                                      const VkBindImageMemoryInfo* pBindInfos, const RecordObject& record_obj) {
    ValidationStateTracker::PostCallRecordBindImageMemory2KHR(device, bindInfoCount, pBindInfos, record_obj);

    if (record_obj.result < VK_SUCCESS) {
        LogErrorCode(record_obj);
    }
}

void BestPractices::PostCallRecordGetSemaphoreCounterValueKHR(VkDevice device, VkSemaphore semaphore, uint64_t* pValue,
                                                              const RecordObject& record_obj) {
    ValidationStateTracker::PostCallRecordGetSemaphoreCounterValueKHR(device, semaphore, pValue, record_obj);

    if (record_obj.result < VK_SUCCESS) {
        LogErrorCode(record_obj);
    }
}

void BestPractices::PostCallRecordWaitSemaphoresKHR(VkDevice device, const VkSemaphoreWaitInfo* pWaitInfo, uint64_t timeout,
                                                    const RecordObject& record_obj) {
    ValidationStateTracker::PostCallRecordWaitSemaphoresKHR(device, pWaitInfo, timeout, record_obj);

    if (record_obj.result > VK_SUCCESS) {
        LogPositiveSuccessCode(record_obj);
        return;
    }
    if (record_obj.result < VK_SUCCESS) {
        LogErrorCode(record_obj);
    }
}

void BestPractices::PostCallRecordSignalSemaphoreKHR(VkDevice device, const VkSemaphoreSignalInfo* pSignalInfo,
                                                     const RecordObject& record_obj) {
    ValidationStateTracker::PostCallRecordSignalSemaphoreKHR(device, pSignalInfo, record_obj);

    if (record_obj.result < VK_SUCCESS) {
        LogErrorCode(record_obj);
    }
}

void BestPractices::PostCallRecordGetPhysicalDeviceFragmentShadingRatesKHR(
    VkPhysicalDevice physicalDevice, uint32_t* pFragmentShadingRateCount,
    VkPhysicalDeviceFragmentShadingRateKHR* pFragmentShadingRates, const RecordObject& record_obj) {
    ValidationStateTracker::PostCallRecordGetPhysicalDeviceFragmentShadingRatesKHR(physicalDevice, pFragmentShadingRateCount,
                                                                                   pFragmentShadingRates, record_obj);

    if (record_obj.result > VK_SUCCESS) {
        LogPositiveSuccessCode(record_obj);
        return;
    }
    if (record_obj.result < VK_SUCCESS) {
        LogErrorCode(record_obj);
    }
}

void BestPractices::PostCallRecordWaitForPresentKHR(VkDevice device, VkSwapchainKHR swapchain, uint64_t presentId, uint64_t timeout,
                                                    const RecordObject& record_obj) {
    ValidationStateTracker::PostCallRecordWaitForPresentKHR(device, swapchain, presentId, timeout, record_obj);

    if (record_obj.result > VK_SUCCESS) {
        LogPositiveSuccessCode(record_obj);
        return;
    }
    if (record_obj.result < VK_SUCCESS) {
        LogErrorCode(record_obj);
    }
}

void BestPractices::PostCallRecordCreateDeferredOperationKHR(VkDevice device, const VkAllocationCallbacks* pAllocator,
                                                             VkDeferredOperationKHR* pDeferredOperation,
                                                             const RecordObject& record_obj) {
    ValidationStateTracker::PostCallRecordCreateDeferredOperationKHR(device, pAllocator, pDeferredOperation, record_obj);

    if (record_obj.result < VK_SUCCESS) {
        LogErrorCode(record_obj);
    }
}

void BestPractices::PostCallRecordGetDeferredOperationResultKHR(VkDevice device, VkDeferredOperationKHR operation,
                                                                const RecordObject& record_obj) {
    ValidationStateTracker::PostCallRecordGetDeferredOperationResultKHR(device, operation, record_obj);

    if (record_obj.result > VK_SUCCESS) {
        LogPositiveSuccessCode(record_obj);
        return;
    }
}

void BestPractices::PostCallRecordDeferredOperationJoinKHR(VkDevice device, VkDeferredOperationKHR operation,
                                                           const RecordObject& record_obj) {
    ValidationStateTracker::PostCallRecordDeferredOperationJoinKHR(device, operation, record_obj);

    if (record_obj.result > VK_SUCCESS) {
        LogPositiveSuccessCode(record_obj);
        return;
    }
    if (record_obj.result < VK_SUCCESS) {
        LogErrorCode(record_obj);
    }
}

void BestPractices::PostCallRecordGetPipelineExecutablePropertiesKHR(VkDevice device, const VkPipelineInfoKHR* pPipelineInfo,
                                                                     uint32_t* pExecutableCount,
                                                                     VkPipelineExecutablePropertiesKHR* pProperties,
                                                                     const RecordObject& record_obj) {
    ValidationStateTracker::PostCallRecordGetPipelineExecutablePropertiesKHR(device, pPipelineInfo, pExecutableCount, pProperties,
                                                                             record_obj);

    if (record_obj.result > VK_SUCCESS) {
        LogPositiveSuccessCode(record_obj);
        return;
    }
    if (record_obj.result < VK_SUCCESS) {
        LogErrorCode(record_obj);
    }
}

void BestPractices::PostCallRecordGetPipelineExecutableStatisticsKHR(VkDevice device,
                                                                     const VkPipelineExecutableInfoKHR* pExecutableInfo,
                                                                     uint32_t* pStatisticCount,
                                                                     VkPipelineExecutableStatisticKHR* pStatistics,
                                                                     const RecordObject& record_obj) {
    ValidationStateTracker::PostCallRecordGetPipelineExecutableStatisticsKHR(device, pExecutableInfo, pStatisticCount, pStatistics,
                                                                             record_obj);

    if (record_obj.result > VK_SUCCESS) {
        LogPositiveSuccessCode(record_obj);
        return;
    }
    if (record_obj.result < VK_SUCCESS) {
        LogErrorCode(record_obj);
    }
}

void BestPractices::PostCallRecordGetPipelineExecutableInternalRepresentationsKHR(
    VkDevice device, const VkPipelineExecutableInfoKHR* pExecutableInfo, uint32_t* pInternalRepresentationCount,
    VkPipelineExecutableInternalRepresentationKHR* pInternalRepresentations, const RecordObject& record_obj) {
    ValidationStateTracker::PostCallRecordGetPipelineExecutableInternalRepresentationsKHR(
        device, pExecutableInfo, pInternalRepresentationCount, pInternalRepresentations, record_obj);

    if (record_obj.result > VK_SUCCESS) {
        LogPositiveSuccessCode(record_obj);
        return;
    }
    if (record_obj.result < VK_SUCCESS) {
        LogErrorCode(record_obj);
    }
}

void BestPractices::PostCallRecordMapMemory2KHR(VkDevice device, const VkMemoryMapInfoKHR* pMemoryMapInfo, void** ppData,
                                                const RecordObject& record_obj) {
    ValidationStateTracker::PostCallRecordMapMemory2KHR(device, pMemoryMapInfo, ppData, record_obj);

    if (record_obj.result < VK_SUCCESS) {
        LogErrorCode(record_obj);
    }
}

#ifdef VK_ENABLE_BETA_EXTENSIONS
void BestPractices::PostCallRecordGetPhysicalDeviceVideoEncodeQualityLevelPropertiesKHR(
    VkPhysicalDevice physicalDevice, const VkPhysicalDeviceVideoEncodeQualityLevelInfoKHR* pQualityLevelInfo,
    VkVideoEncodeQualityLevelPropertiesKHR* pQualityLevelProperties, const RecordObject& record_obj) {
    ValidationStateTracker::PostCallRecordGetPhysicalDeviceVideoEncodeQualityLevelPropertiesKHR(
        physicalDevice, pQualityLevelInfo, pQualityLevelProperties, record_obj);

    if (record_obj.result < VK_SUCCESS) {
        LogErrorCode(record_obj);
    }
}
#endif  // VK_ENABLE_BETA_EXTENSIONS

#ifdef VK_ENABLE_BETA_EXTENSIONS
void BestPractices::PostCallRecordGetEncodedVideoSessionParametersKHR(
    VkDevice device, const VkVideoEncodeSessionParametersGetInfoKHR* pVideoSessionParametersInfo,
    VkVideoEncodeSessionParametersFeedbackInfoKHR* pFeedbackInfo, size_t* pDataSize, void* pData, const RecordObject& record_obj) {
    ValidationStateTracker::PostCallRecordGetEncodedVideoSessionParametersKHR(device, pVideoSessionParametersInfo, pFeedbackInfo,
                                                                              pDataSize, pData, record_obj);

    if (record_obj.result > VK_SUCCESS) {
        LogPositiveSuccessCode(record_obj);
        return;
    }
    if (record_obj.result < VK_SUCCESS) {
        LogErrorCode(record_obj);
    }
}
#endif  // VK_ENABLE_BETA_EXTENSIONS

void BestPractices::PostCallRecordQueueSubmit2KHR(VkQueue queue, uint32_t submitCount, const VkSubmitInfo2* pSubmits, VkFence fence,
                                                  const RecordObject& record_obj) {
    ValidationStateTracker::PostCallRecordQueueSubmit2KHR(queue, submitCount, pSubmits, fence, record_obj);

    if (record_obj.result < VK_SUCCESS) {
        LogErrorCode(record_obj);
    }
}

void BestPractices::PostCallRecordGetPhysicalDeviceCooperativeMatrixPropertiesKHR(VkPhysicalDevice physicalDevice,
                                                                                  uint32_t* pPropertyCount,
                                                                                  VkCooperativeMatrixPropertiesKHR* pProperties,
                                                                                  const RecordObject& record_obj) {
    ValidationStateTracker::PostCallRecordGetPhysicalDeviceCooperativeMatrixPropertiesKHR(physicalDevice, pPropertyCount,
                                                                                          pProperties, record_obj);

    if (record_obj.result > VK_SUCCESS) {
        LogPositiveSuccessCode(record_obj);
        return;
    }
    if (record_obj.result < VK_SUCCESS) {
        LogErrorCode(record_obj);
    }
}

void BestPractices::PostCallRecordCreateDebugReportCallbackEXT(VkInstance instance,
                                                               const VkDebugReportCallbackCreateInfoEXT* pCreateInfo,
                                                               const VkAllocationCallbacks* pAllocator,
                                                               VkDebugReportCallbackEXT* pCallback,
                                                               const RecordObject& record_obj) {
    ValidationStateTracker::PostCallRecordCreateDebugReportCallbackEXT(instance, pCreateInfo, pAllocator, pCallback, record_obj);

    if (record_obj.result < VK_SUCCESS) {
        LogErrorCode(record_obj);
    }
}

void BestPractices::PostCallRecordDebugMarkerSetObjectTagEXT(VkDevice device, const VkDebugMarkerObjectTagInfoEXT* pTagInfo,
                                                             const RecordObject& record_obj) {
    ValidationStateTracker::PostCallRecordDebugMarkerSetObjectTagEXT(device, pTagInfo, record_obj);

    if (record_obj.result < VK_SUCCESS) {
        LogErrorCode(record_obj);
    }
}

void BestPractices::PostCallRecordDebugMarkerSetObjectNameEXT(VkDevice device, const VkDebugMarkerObjectNameInfoEXT* pNameInfo,
                                                              const RecordObject& record_obj) {
    ValidationStateTracker::PostCallRecordDebugMarkerSetObjectNameEXT(device, pNameInfo, record_obj);

    if (record_obj.result < VK_SUCCESS) {
        LogErrorCode(record_obj);
    }
}

void BestPractices::PostCallRecordCreateCuModuleNVX(VkDevice device, const VkCuModuleCreateInfoNVX* pCreateInfo,
                                                    const VkAllocationCallbacks* pAllocator, VkCuModuleNVX* pModule,
                                                    const RecordObject& record_obj) {
    ValidationStateTracker::PostCallRecordCreateCuModuleNVX(device, pCreateInfo, pAllocator, pModule, record_obj);

    if (record_obj.result < VK_SUCCESS) {
        LogErrorCode(record_obj);
    }
}

void BestPractices::PostCallRecordCreateCuFunctionNVX(VkDevice device, const VkCuFunctionCreateInfoNVX* pCreateInfo,
                                                      const VkAllocationCallbacks* pAllocator, VkCuFunctionNVX* pFunction,
                                                      const RecordObject& record_obj) {
    ValidationStateTracker::PostCallRecordCreateCuFunctionNVX(device, pCreateInfo, pAllocator, pFunction, record_obj);

    if (record_obj.result < VK_SUCCESS) {
        LogErrorCode(record_obj);
    }
}

void BestPractices::PostCallRecordGetImageViewAddressNVX(VkDevice device, VkImageView imageView,
                                                         VkImageViewAddressPropertiesNVX* pProperties,
                                                         const RecordObject& record_obj) {
    ValidationStateTracker::PostCallRecordGetImageViewAddressNVX(device, imageView, pProperties, record_obj);

    if (record_obj.result < VK_SUCCESS) {
        LogErrorCode(record_obj);
    }
}

void BestPractices::PostCallRecordGetShaderInfoAMD(VkDevice device, VkPipeline pipeline, VkShaderStageFlagBits shaderStage,
                                                   VkShaderInfoTypeAMD infoType, size_t* pInfoSize, void* pInfo,
                                                   const RecordObject& record_obj) {
    ValidationStateTracker::PostCallRecordGetShaderInfoAMD(device, pipeline, shaderStage, infoType, pInfoSize, pInfo, record_obj);

    if (record_obj.result > VK_SUCCESS) {
        LogPositiveSuccessCode(record_obj);
        return;
    }
    if (record_obj.result < VK_SUCCESS) {
        LogErrorCode(record_obj);
    }
}

#ifdef VK_USE_PLATFORM_GGP
void BestPractices::PostCallRecordCreateStreamDescriptorSurfaceGGP(VkInstance instance,
                                                                   const VkStreamDescriptorSurfaceCreateInfoGGP* pCreateInfo,
                                                                   const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface,
                                                                   const RecordObject& record_obj) {
    ValidationStateTracker::PostCallRecordCreateStreamDescriptorSurfaceGGP(instance, pCreateInfo, pAllocator, pSurface, record_obj);

    if (record_obj.result < VK_SUCCESS) {
        LogErrorCode(record_obj);
    }
}
#endif  // VK_USE_PLATFORM_GGP

void BestPractices::PostCallRecordGetPhysicalDeviceExternalImageFormatPropertiesNV(
    VkPhysicalDevice physicalDevice, VkFormat format, VkImageType type, VkImageTiling tiling, VkImageUsageFlags usage,
    VkImageCreateFlags flags, VkExternalMemoryHandleTypeFlagsNV externalHandleType,
    VkExternalImageFormatPropertiesNV* pExternalImageFormatProperties, const RecordObject& record_obj) {
    ValidationStateTracker::PostCallRecordGetPhysicalDeviceExternalImageFormatPropertiesNV(
        physicalDevice, format, type, tiling, usage, flags, externalHandleType, pExternalImageFormatProperties, record_obj);

    if (record_obj.result < VK_SUCCESS) {
        LogErrorCode(record_obj);
    }
}

#ifdef VK_USE_PLATFORM_WIN32_KHR
void BestPractices::PostCallRecordGetMemoryWin32HandleNV(VkDevice device, VkDeviceMemory memory,
                                                         VkExternalMemoryHandleTypeFlagsNV handleType, HANDLE* pHandle,
                                                         const RecordObject& record_obj) {
    ValidationStateTracker::PostCallRecordGetMemoryWin32HandleNV(device, memory, handleType, pHandle, record_obj);

    if (record_obj.result < VK_SUCCESS) {
        LogErrorCode(record_obj);
    }
}
#endif  // VK_USE_PLATFORM_WIN32_KHR

#ifdef VK_USE_PLATFORM_VI_NN
void BestPractices::PostCallRecordCreateViSurfaceNN(VkInstance instance, const VkViSurfaceCreateInfoNN* pCreateInfo,
                                                    const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface,
                                                    const RecordObject& record_obj) {
    ValidationStateTracker::PostCallRecordCreateViSurfaceNN(instance, pCreateInfo, pAllocator, pSurface, record_obj);

    if (record_obj.result < VK_SUCCESS) {
        LogErrorCode(record_obj);
    }
}
#endif  // VK_USE_PLATFORM_VI_NN

#ifdef VK_USE_PLATFORM_XLIB_XRANDR_EXT
void BestPractices::PostCallRecordAcquireXlibDisplayEXT(VkPhysicalDevice physicalDevice, Display* dpy, VkDisplayKHR display,
                                                        const RecordObject& record_obj) {
    ValidationStateTracker::PostCallRecordAcquireXlibDisplayEXT(physicalDevice, dpy, display, record_obj);

    if (record_obj.result < VK_SUCCESS) {
        LogErrorCode(record_obj);
    }
}
#endif  // VK_USE_PLATFORM_XLIB_XRANDR_EXT

#ifdef VK_USE_PLATFORM_XLIB_XRANDR_EXT
void BestPractices::PostCallRecordGetRandROutputDisplayEXT(VkPhysicalDevice physicalDevice, Display* dpy, RROutput rrOutput,
                                                           VkDisplayKHR* pDisplay, const RecordObject& record_obj) {
    ValidationStateTracker::PostCallRecordGetRandROutputDisplayEXT(physicalDevice, dpy, rrOutput, pDisplay, record_obj);

    if (record_obj.result < VK_SUCCESS) {
        LogErrorCode(record_obj);
    }
}
#endif  // VK_USE_PLATFORM_XLIB_XRANDR_EXT

void BestPractices::PostCallRecordGetPhysicalDeviceSurfaceCapabilities2EXT(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface,
                                                                           VkSurfaceCapabilities2EXT* pSurfaceCapabilities,
                                                                           const RecordObject& record_obj) {
    ValidationStateTracker::PostCallRecordGetPhysicalDeviceSurfaceCapabilities2EXT(physicalDevice, surface, pSurfaceCapabilities,
                                                                                   record_obj);
    ManualPostCallRecordGetPhysicalDeviceSurfaceCapabilities2EXT(physicalDevice, surface, pSurfaceCapabilities, record_obj);

    if (record_obj.result < VK_SUCCESS) {
        LogErrorCode(record_obj);
    }
}

void BestPractices::PostCallRecordDisplayPowerControlEXT(VkDevice device, VkDisplayKHR display,
                                                         const VkDisplayPowerInfoEXT* pDisplayPowerInfo,
                                                         const RecordObject& record_obj) {
    ValidationStateTracker::PostCallRecordDisplayPowerControlEXT(device, display, pDisplayPowerInfo, record_obj);

    if (record_obj.result < VK_SUCCESS) {
        LogErrorCode(record_obj);
    }
}

void BestPractices::PostCallRecordRegisterDeviceEventEXT(VkDevice device, const VkDeviceEventInfoEXT* pDeviceEventInfo,
                                                         const VkAllocationCallbacks* pAllocator, VkFence* pFence,
                                                         const RecordObject& record_obj) {
    ValidationStateTracker::PostCallRecordRegisterDeviceEventEXT(device, pDeviceEventInfo, pAllocator, pFence, record_obj);

    if (record_obj.result < VK_SUCCESS) {
        LogErrorCode(record_obj);
    }
}

void BestPractices::PostCallRecordRegisterDisplayEventEXT(VkDevice device, VkDisplayKHR display,
                                                          const VkDisplayEventInfoEXT* pDisplayEventInfo,
                                                          const VkAllocationCallbacks* pAllocator, VkFence* pFence,
                                                          const RecordObject& record_obj) {
    ValidationStateTracker::PostCallRecordRegisterDisplayEventEXT(device, display, pDisplayEventInfo, pAllocator, pFence,
                                                                  record_obj);

    if (record_obj.result < VK_SUCCESS) {
        LogErrorCode(record_obj);
    }
}

void BestPractices::PostCallRecordGetSwapchainCounterEXT(VkDevice device, VkSwapchainKHR swapchain,
                                                         VkSurfaceCounterFlagBitsEXT counter, uint64_t* pCounterValue,
                                                         const RecordObject& record_obj) {
    ValidationStateTracker::PostCallRecordGetSwapchainCounterEXT(device, swapchain, counter, pCounterValue, record_obj);

    if (record_obj.result < VK_SUCCESS) {
        LogErrorCode(record_obj);
    }
}

void BestPractices::PostCallRecordGetRefreshCycleDurationGOOGLE(VkDevice device, VkSwapchainKHR swapchain,
                                                                VkRefreshCycleDurationGOOGLE* pDisplayTimingProperties,
                                                                const RecordObject& record_obj) {
    ValidationStateTracker::PostCallRecordGetRefreshCycleDurationGOOGLE(device, swapchain, pDisplayTimingProperties, record_obj);

    if (record_obj.result < VK_SUCCESS) {
        LogErrorCode(record_obj);
    }
}

void BestPractices::PostCallRecordGetPastPresentationTimingGOOGLE(VkDevice device, VkSwapchainKHR swapchain,
                                                                  uint32_t* pPresentationTimingCount,
                                                                  VkPastPresentationTimingGOOGLE* pPresentationTimings,
                                                                  const RecordObject& record_obj) {
    ValidationStateTracker::PostCallRecordGetPastPresentationTimingGOOGLE(device, swapchain, pPresentationTimingCount,
                                                                          pPresentationTimings, record_obj);

    if (record_obj.result > VK_SUCCESS) {
        LogPositiveSuccessCode(record_obj);
        return;
    }
    if (record_obj.result < VK_SUCCESS) {
        LogErrorCode(record_obj);
    }
}

#ifdef VK_USE_PLATFORM_IOS_MVK
void BestPractices::PostCallRecordCreateIOSSurfaceMVK(VkInstance instance, const VkIOSSurfaceCreateInfoMVK* pCreateInfo,
                                                      const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface,
                                                      const RecordObject& record_obj) {
    ValidationStateTracker::PostCallRecordCreateIOSSurfaceMVK(instance, pCreateInfo, pAllocator, pSurface, record_obj);

    if (record_obj.result < VK_SUCCESS) {
        LogErrorCode(record_obj);
    }
}
#endif  // VK_USE_PLATFORM_IOS_MVK

#ifdef VK_USE_PLATFORM_MACOS_MVK
void BestPractices::PostCallRecordCreateMacOSSurfaceMVK(VkInstance instance, const VkMacOSSurfaceCreateInfoMVK* pCreateInfo,
                                                        const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface,
                                                        const RecordObject& record_obj) {
    ValidationStateTracker::PostCallRecordCreateMacOSSurfaceMVK(instance, pCreateInfo, pAllocator, pSurface, record_obj);

    if (record_obj.result < VK_SUCCESS) {
        LogErrorCode(record_obj);
    }
}
#endif  // VK_USE_PLATFORM_MACOS_MVK

void BestPractices::PostCallRecordSetDebugUtilsObjectNameEXT(VkDevice device, const VkDebugUtilsObjectNameInfoEXT* pNameInfo,
                                                             const RecordObject& record_obj) {
    ValidationStateTracker::PostCallRecordSetDebugUtilsObjectNameEXT(device, pNameInfo, record_obj);

    if (record_obj.result < VK_SUCCESS) {
        LogErrorCode(record_obj);
    }
}

void BestPractices::PostCallRecordSetDebugUtilsObjectTagEXT(VkDevice device, const VkDebugUtilsObjectTagInfoEXT* pTagInfo,
                                                            const RecordObject& record_obj) {
    ValidationStateTracker::PostCallRecordSetDebugUtilsObjectTagEXT(device, pTagInfo, record_obj);

    if (record_obj.result < VK_SUCCESS) {
        LogErrorCode(record_obj);
    }
}

void BestPractices::PostCallRecordCreateDebugUtilsMessengerEXT(VkInstance instance,
                                                               const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
                                                               const VkAllocationCallbacks* pAllocator,
                                                               VkDebugUtilsMessengerEXT* pMessenger,
                                                               const RecordObject& record_obj) {
    ValidationStateTracker::PostCallRecordCreateDebugUtilsMessengerEXT(instance, pCreateInfo, pAllocator, pMessenger, record_obj);

    if (record_obj.result < VK_SUCCESS) {
        LogErrorCode(record_obj);
    }
}

#ifdef VK_USE_PLATFORM_ANDROID_KHR
void BestPractices::PostCallRecordGetAndroidHardwareBufferPropertiesANDROID(VkDevice device, const struct AHardwareBuffer* buffer,
                                                                            VkAndroidHardwareBufferPropertiesANDROID* pProperties,
                                                                            const RecordObject& record_obj) {
    ValidationStateTracker::PostCallRecordGetAndroidHardwareBufferPropertiesANDROID(device, buffer, pProperties, record_obj);

    if (record_obj.result < VK_SUCCESS) {
        LogErrorCode(record_obj);
    }
}
#endif  // VK_USE_PLATFORM_ANDROID_KHR

#ifdef VK_USE_PLATFORM_ANDROID_KHR
void BestPractices::PostCallRecordGetMemoryAndroidHardwareBufferANDROID(VkDevice device,
                                                                        const VkMemoryGetAndroidHardwareBufferInfoANDROID* pInfo,
                                                                        struct AHardwareBuffer** pBuffer,
                                                                        const RecordObject& record_obj) {
    ValidationStateTracker::PostCallRecordGetMemoryAndroidHardwareBufferANDROID(device, pInfo, pBuffer, record_obj);

    if (record_obj.result < VK_SUCCESS) {
        LogErrorCode(record_obj);
    }
}
#endif  // VK_USE_PLATFORM_ANDROID_KHR

#ifdef VK_ENABLE_BETA_EXTENSIONS
void BestPractices::PostCallRecordCreateExecutionGraphPipelinesAMDX(VkDevice device, VkPipelineCache pipelineCache,
                                                                    uint32_t createInfoCount,
                                                                    const VkExecutionGraphPipelineCreateInfoAMDX* pCreateInfos,
                                                                    const VkAllocationCallbacks* pAllocator, VkPipeline* pPipelines,
                                                                    const RecordObject& record_obj) {
    ValidationStateTracker::PostCallRecordCreateExecutionGraphPipelinesAMDX(device, pipelineCache, createInfoCount, pCreateInfos,
                                                                            pAllocator, pPipelines, record_obj);

    if (record_obj.result > VK_SUCCESS) {
        LogPositiveSuccessCode(record_obj);
        return;
    }
    if (record_obj.result < VK_SUCCESS) {
        LogErrorCode(record_obj);
    }
}
#endif  // VK_ENABLE_BETA_EXTENSIONS

#ifdef VK_ENABLE_BETA_EXTENSIONS
void BestPractices::PostCallRecordGetExecutionGraphPipelineScratchSizeAMDX(VkDevice device, VkPipeline executionGraph,
                                                                           VkExecutionGraphPipelineScratchSizeAMDX* pSizeInfo,
                                                                           const RecordObject& record_obj) {
    ValidationStateTracker::PostCallRecordGetExecutionGraphPipelineScratchSizeAMDX(device, executionGraph, pSizeInfo, record_obj);

    if (record_obj.result < VK_SUCCESS) {
        LogErrorCode(record_obj);
    }
}
#endif  // VK_ENABLE_BETA_EXTENSIONS

#ifdef VK_ENABLE_BETA_EXTENSIONS
void BestPractices::PostCallRecordGetExecutionGraphPipelineNodeIndexAMDX(VkDevice device, VkPipeline executionGraph,
                                                                         const VkPipelineShaderStageNodeCreateInfoAMDX* pNodeInfo,
                                                                         uint32_t* pNodeIndex, const RecordObject& record_obj) {
    ValidationStateTracker::PostCallRecordGetExecutionGraphPipelineNodeIndexAMDX(device, executionGraph, pNodeInfo, pNodeIndex,
                                                                                 record_obj);

    if (record_obj.result < VK_SUCCESS) {
        LogErrorCode(record_obj);
    }
}
#endif  // VK_ENABLE_BETA_EXTENSIONS

void BestPractices::PostCallRecordGetImageDrmFormatModifierPropertiesEXT(VkDevice device, VkImage image,
                                                                         VkImageDrmFormatModifierPropertiesEXT* pProperties,
                                                                         const RecordObject& record_obj) {
    ValidationStateTracker::PostCallRecordGetImageDrmFormatModifierPropertiesEXT(device, image, pProperties, record_obj);

    if (record_obj.result < VK_SUCCESS) {
        LogErrorCode(record_obj);
    }
}

void BestPractices::PostCallRecordCreateAccelerationStructureNV(VkDevice device,
                                                                const VkAccelerationStructureCreateInfoNV* pCreateInfo,
                                                                const VkAllocationCallbacks* pAllocator,
                                                                VkAccelerationStructureNV* pAccelerationStructure,
                                                                const RecordObject& record_obj) {
    ValidationStateTracker::PostCallRecordCreateAccelerationStructureNV(device, pCreateInfo, pAllocator, pAccelerationStructure,
                                                                        record_obj);

    if (record_obj.result < VK_SUCCESS) {
        LogErrorCode(record_obj);
    }
}

void BestPractices::PostCallRecordBindAccelerationStructureMemoryNV(VkDevice device, uint32_t bindInfoCount,
                                                                    const VkBindAccelerationStructureMemoryInfoNV* pBindInfos,
                                                                    const RecordObject& record_obj) {
    ValidationStateTracker::PostCallRecordBindAccelerationStructureMemoryNV(device, bindInfoCount, pBindInfos, record_obj);

    if (record_obj.result < VK_SUCCESS) {
        LogErrorCode(record_obj);
    }
}

void BestPractices::PostCallRecordCreateRayTracingPipelinesNV(VkDevice device, VkPipelineCache pipelineCache,
                                                              uint32_t createInfoCount,
                                                              const VkRayTracingPipelineCreateInfoNV* pCreateInfos,
                                                              const VkAllocationCallbacks* pAllocator, VkPipeline* pPipelines,
                                                              const RecordObject& record_obj, void* state_data) {
    ValidationStateTracker::PostCallRecordCreateRayTracingPipelinesNV(device, pipelineCache, createInfoCount, pCreateInfos,
                                                                      pAllocator, pPipelines, record_obj, state_data);

    if (record_obj.result > VK_SUCCESS) {
        LogPositiveSuccessCode(record_obj);
        return;
    }
    if (record_obj.result < VK_SUCCESS) {
        LogErrorCode(record_obj);
    }
}

void BestPractices::PostCallRecordGetRayTracingShaderGroupHandlesKHR(VkDevice device, VkPipeline pipeline, uint32_t firstGroup,
                                                                     uint32_t groupCount, size_t dataSize, void* pData,
                                                                     const RecordObject& record_obj) {
    ValidationStateTracker::PostCallRecordGetRayTracingShaderGroupHandlesKHR(device, pipeline, firstGroup, groupCount, dataSize,
                                                                             pData, record_obj);

    if (record_obj.result < VK_SUCCESS) {
        LogErrorCode(record_obj);
    }
}

void BestPractices::PostCallRecordGetRayTracingShaderGroupHandlesNV(VkDevice device, VkPipeline pipeline, uint32_t firstGroup,
                                                                    uint32_t groupCount, size_t dataSize, void* pData,
                                                                    const RecordObject& record_obj) {
    ValidationStateTracker::PostCallRecordGetRayTracingShaderGroupHandlesNV(device, pipeline, firstGroup, groupCount, dataSize,
                                                                            pData, record_obj);

    if (record_obj.result < VK_SUCCESS) {
        LogErrorCode(record_obj);
    }
}

void BestPractices::PostCallRecordGetAccelerationStructureHandleNV(VkDevice device, VkAccelerationStructureNV accelerationStructure,
                                                                   size_t dataSize, void* pData, const RecordObject& record_obj) {
    ValidationStateTracker::PostCallRecordGetAccelerationStructureHandleNV(device, accelerationStructure, dataSize, pData,
                                                                           record_obj);

    if (record_obj.result < VK_SUCCESS) {
        LogErrorCode(record_obj);
    }
}

void BestPractices::PostCallRecordCompileDeferredNV(VkDevice device, VkPipeline pipeline, uint32_t shader,
                                                    const RecordObject& record_obj) {
    ValidationStateTracker::PostCallRecordCompileDeferredNV(device, pipeline, shader, record_obj);

    if (record_obj.result < VK_SUCCESS) {
        LogErrorCode(record_obj);
    }
}

void BestPractices::PostCallRecordGetMemoryHostPointerPropertiesEXT(VkDevice device, VkExternalMemoryHandleTypeFlagBits handleType,
                                                                    const void* pHostPointer,
                                                                    VkMemoryHostPointerPropertiesEXT* pMemoryHostPointerProperties,
                                                                    const RecordObject& record_obj) {
    ValidationStateTracker::PostCallRecordGetMemoryHostPointerPropertiesEXT(device, handleType, pHostPointer,
                                                                            pMemoryHostPointerProperties, record_obj);

    if (record_obj.result < VK_SUCCESS) {
        LogErrorCode(record_obj);
    }
}

void BestPractices::PostCallRecordGetPhysicalDeviceCalibrateableTimeDomainsEXT(VkPhysicalDevice physicalDevice,
                                                                               uint32_t* pTimeDomainCount,
                                                                               VkTimeDomainEXT* pTimeDomains,
                                                                               const RecordObject& record_obj) {
    ValidationStateTracker::PostCallRecordGetPhysicalDeviceCalibrateableTimeDomainsEXT(physicalDevice, pTimeDomainCount,
                                                                                       pTimeDomains, record_obj);

    if (record_obj.result > VK_SUCCESS) {
        LogPositiveSuccessCode(record_obj);
        return;
    }
    if (record_obj.result < VK_SUCCESS) {
        LogErrorCode(record_obj);
    }
}

void BestPractices::PostCallRecordGetCalibratedTimestampsEXT(VkDevice device, uint32_t timestampCount,
                                                             const VkCalibratedTimestampInfoEXT* pTimestampInfos,
                                                             uint64_t* pTimestamps, uint64_t* pMaxDeviation,
                                                             const RecordObject& record_obj) {
    ValidationStateTracker::PostCallRecordGetCalibratedTimestampsEXT(device, timestampCount, pTimestampInfos, pTimestamps,
                                                                     pMaxDeviation, record_obj);

    if (record_obj.result < VK_SUCCESS) {
        LogErrorCode(record_obj);
    }
}

void BestPractices::PostCallRecordInitializePerformanceApiINTEL(VkDevice device,
                                                                const VkInitializePerformanceApiInfoINTEL* pInitializeInfo,
                                                                const RecordObject& record_obj) {
    ValidationStateTracker::PostCallRecordInitializePerformanceApiINTEL(device, pInitializeInfo, record_obj);

    if (record_obj.result < VK_SUCCESS) {
        LogErrorCode(record_obj);
    }
}

void BestPractices::PostCallRecordCmdSetPerformanceMarkerINTEL(VkCommandBuffer commandBuffer,
                                                               const VkPerformanceMarkerInfoINTEL* pMarkerInfo,
                                                               const RecordObject& record_obj) {
    ValidationStateTracker::PostCallRecordCmdSetPerformanceMarkerINTEL(commandBuffer, pMarkerInfo, record_obj);

    if (record_obj.result < VK_SUCCESS) {
        LogErrorCode(record_obj);
    }
}

void BestPractices::PostCallRecordCmdSetPerformanceStreamMarkerINTEL(VkCommandBuffer commandBuffer,
                                                                     const VkPerformanceStreamMarkerInfoINTEL* pMarkerInfo,
                                                                     const RecordObject& record_obj) {
    ValidationStateTracker::PostCallRecordCmdSetPerformanceStreamMarkerINTEL(commandBuffer, pMarkerInfo, record_obj);

    if (record_obj.result < VK_SUCCESS) {
        LogErrorCode(record_obj);
    }
}

void BestPractices::PostCallRecordCmdSetPerformanceOverrideINTEL(VkCommandBuffer commandBuffer,
                                                                 const VkPerformanceOverrideInfoINTEL* pOverrideInfo,
                                                                 const RecordObject& record_obj) {
    ValidationStateTracker::PostCallRecordCmdSetPerformanceOverrideINTEL(commandBuffer, pOverrideInfo, record_obj);

    if (record_obj.result < VK_SUCCESS) {
        LogErrorCode(record_obj);
    }
}

void BestPractices::PostCallRecordAcquirePerformanceConfigurationINTEL(
    VkDevice device, const VkPerformanceConfigurationAcquireInfoINTEL* pAcquireInfo,
    VkPerformanceConfigurationINTEL* pConfiguration, const RecordObject& record_obj) {
    ValidationStateTracker::PostCallRecordAcquirePerformanceConfigurationINTEL(device, pAcquireInfo, pConfiguration, record_obj);

    if (record_obj.result < VK_SUCCESS) {
        LogErrorCode(record_obj);
    }
}

void BestPractices::PostCallRecordReleasePerformanceConfigurationINTEL(VkDevice device,
                                                                       VkPerformanceConfigurationINTEL configuration,
                                                                       const RecordObject& record_obj) {
    ValidationStateTracker::PostCallRecordReleasePerformanceConfigurationINTEL(device, configuration, record_obj);

    if (record_obj.result < VK_SUCCESS) {
        LogErrorCode(record_obj);
    }
}

void BestPractices::PostCallRecordQueueSetPerformanceConfigurationINTEL(VkQueue queue,
                                                                        VkPerformanceConfigurationINTEL configuration,
                                                                        const RecordObject& record_obj) {
    ValidationStateTracker::PostCallRecordQueueSetPerformanceConfigurationINTEL(queue, configuration, record_obj);

    if (record_obj.result < VK_SUCCESS) {
        LogErrorCode(record_obj);
    }
}

void BestPractices::PostCallRecordGetPerformanceParameterINTEL(VkDevice device, VkPerformanceParameterTypeINTEL parameter,
                                                               VkPerformanceValueINTEL* pValue, const RecordObject& record_obj) {
    ValidationStateTracker::PostCallRecordGetPerformanceParameterINTEL(device, parameter, pValue, record_obj);

    if (record_obj.result < VK_SUCCESS) {
        LogErrorCode(record_obj);
    }
}

#ifdef VK_USE_PLATFORM_FUCHSIA
void BestPractices::PostCallRecordCreateImagePipeSurfaceFUCHSIA(VkInstance instance,
                                                                const VkImagePipeSurfaceCreateInfoFUCHSIA* pCreateInfo,
                                                                const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface,
                                                                const RecordObject& record_obj) {
    ValidationStateTracker::PostCallRecordCreateImagePipeSurfaceFUCHSIA(instance, pCreateInfo, pAllocator, pSurface, record_obj);

    if (record_obj.result < VK_SUCCESS) {
        LogErrorCode(record_obj);
    }
}
#endif  // VK_USE_PLATFORM_FUCHSIA

#ifdef VK_USE_PLATFORM_METAL_EXT
void BestPractices::PostCallRecordCreateMetalSurfaceEXT(VkInstance instance, const VkMetalSurfaceCreateInfoEXT* pCreateInfo,
                                                        const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface,
                                                        const RecordObject& record_obj) {
    ValidationStateTracker::PostCallRecordCreateMetalSurfaceEXT(instance, pCreateInfo, pAllocator, pSurface, record_obj);

    if (record_obj.result < VK_SUCCESS) {
        LogErrorCode(record_obj);
    }
}
#endif  // VK_USE_PLATFORM_METAL_EXT

void BestPractices::PostCallRecordGetPhysicalDeviceToolPropertiesEXT(VkPhysicalDevice physicalDevice, uint32_t* pToolCount,
                                                                     VkPhysicalDeviceToolProperties* pToolProperties,
                                                                     const RecordObject& record_obj) {
    ValidationStateTracker::PostCallRecordGetPhysicalDeviceToolPropertiesEXT(physicalDevice, pToolCount, pToolProperties,
                                                                             record_obj);

    if (record_obj.result > VK_SUCCESS) {
        LogPositiveSuccessCode(record_obj);
        return;
    }
    if (record_obj.result < VK_SUCCESS) {
        LogErrorCode(record_obj);
    }
}

void BestPractices::PostCallRecordGetPhysicalDeviceCooperativeMatrixPropertiesNV(VkPhysicalDevice physicalDevice,
                                                                                 uint32_t* pPropertyCount,
                                                                                 VkCooperativeMatrixPropertiesNV* pProperties,
                                                                                 const RecordObject& record_obj) {
    ValidationStateTracker::PostCallRecordGetPhysicalDeviceCooperativeMatrixPropertiesNV(physicalDevice, pPropertyCount,
                                                                                         pProperties, record_obj);

    if (record_obj.result > VK_SUCCESS) {
        LogPositiveSuccessCode(record_obj);
        return;
    }
    if (record_obj.result < VK_SUCCESS) {
        LogErrorCode(record_obj);
    }
}

void BestPractices::PostCallRecordGetPhysicalDeviceSupportedFramebufferMixedSamplesCombinationsNV(
    VkPhysicalDevice physicalDevice, uint32_t* pCombinationCount, VkFramebufferMixedSamplesCombinationNV* pCombinations,
    const RecordObject& record_obj) {
    ValidationStateTracker::PostCallRecordGetPhysicalDeviceSupportedFramebufferMixedSamplesCombinationsNV(
        physicalDevice, pCombinationCount, pCombinations, record_obj);

    if (record_obj.result > VK_SUCCESS) {
        LogPositiveSuccessCode(record_obj);
        return;
    }
    if (record_obj.result < VK_SUCCESS) {
        LogErrorCode(record_obj);
    }
}

#ifdef VK_USE_PLATFORM_WIN32_KHR
void BestPractices::PostCallRecordGetPhysicalDeviceSurfacePresentModes2EXT(VkPhysicalDevice physicalDevice,
                                                                           const VkPhysicalDeviceSurfaceInfo2KHR* pSurfaceInfo,
                                                                           uint32_t* pPresentModeCount,
                                                                           VkPresentModeKHR* pPresentModes,
                                                                           const RecordObject& record_obj) {
    ValidationStateTracker::PostCallRecordGetPhysicalDeviceSurfacePresentModes2EXT(physicalDevice, pSurfaceInfo, pPresentModeCount,
                                                                                   pPresentModes, record_obj);

    if (record_obj.result > VK_SUCCESS) {
        LogPositiveSuccessCode(record_obj);
        return;
    }
    if (record_obj.result < VK_SUCCESS) {
        LogErrorCode(record_obj);
    }
}
#endif  // VK_USE_PLATFORM_WIN32_KHR

#ifdef VK_USE_PLATFORM_WIN32_KHR
void BestPractices::PostCallRecordAcquireFullScreenExclusiveModeEXT(VkDevice device, VkSwapchainKHR swapchain,
                                                                    const RecordObject& record_obj) {
    ValidationStateTracker::PostCallRecordAcquireFullScreenExclusiveModeEXT(device, swapchain, record_obj);

    if (record_obj.result < VK_SUCCESS) {
        LogErrorCode(record_obj);
    }
}
#endif  // VK_USE_PLATFORM_WIN32_KHR

#ifdef VK_USE_PLATFORM_WIN32_KHR
void BestPractices::PostCallRecordReleaseFullScreenExclusiveModeEXT(VkDevice device, VkSwapchainKHR swapchain,
                                                                    const RecordObject& record_obj) {
    ValidationStateTracker::PostCallRecordReleaseFullScreenExclusiveModeEXT(device, swapchain, record_obj);

    if (record_obj.result < VK_SUCCESS) {
        LogErrorCode(record_obj);
    }
}
#endif  // VK_USE_PLATFORM_WIN32_KHR

#ifdef VK_USE_PLATFORM_WIN32_KHR
void BestPractices::PostCallRecordGetDeviceGroupSurfacePresentModes2EXT(VkDevice device,
                                                                        const VkPhysicalDeviceSurfaceInfo2KHR* pSurfaceInfo,
                                                                        VkDeviceGroupPresentModeFlagsKHR* pModes,
                                                                        const RecordObject& record_obj) {
    ValidationStateTracker::PostCallRecordGetDeviceGroupSurfacePresentModes2EXT(device, pSurfaceInfo, pModes, record_obj);

    if (record_obj.result < VK_SUCCESS) {
        LogErrorCode(record_obj);
    }
}
#endif  // VK_USE_PLATFORM_WIN32_KHR

void BestPractices::PostCallRecordCreateHeadlessSurfaceEXT(VkInstance instance, const VkHeadlessSurfaceCreateInfoEXT* pCreateInfo,
                                                           const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface,
                                                           const RecordObject& record_obj) {
    ValidationStateTracker::PostCallRecordCreateHeadlessSurfaceEXT(instance, pCreateInfo, pAllocator, pSurface, record_obj);

    if (record_obj.result < VK_SUCCESS) {
        LogErrorCode(record_obj);
    }
}

void BestPractices::PostCallRecordCopyMemoryToImageEXT(VkDevice device, const VkCopyMemoryToImageInfoEXT* pCopyMemoryToImageInfo,
                                                       const RecordObject& record_obj) {
    ValidationStateTracker::PostCallRecordCopyMemoryToImageEXT(device, pCopyMemoryToImageInfo, record_obj);

    if (record_obj.result < VK_SUCCESS) {
        LogErrorCode(record_obj);
    }
}

void BestPractices::PostCallRecordCopyImageToMemoryEXT(VkDevice device, const VkCopyImageToMemoryInfoEXT* pCopyImageToMemoryInfo,
                                                       const RecordObject& record_obj) {
    ValidationStateTracker::PostCallRecordCopyImageToMemoryEXT(device, pCopyImageToMemoryInfo, record_obj);

    if (record_obj.result < VK_SUCCESS) {
        LogErrorCode(record_obj);
    }
}

void BestPractices::PostCallRecordCopyImageToImageEXT(VkDevice device, const VkCopyImageToImageInfoEXT* pCopyImageToImageInfo,
                                                      const RecordObject& record_obj) {
    ValidationStateTracker::PostCallRecordCopyImageToImageEXT(device, pCopyImageToImageInfo, record_obj);

    if (record_obj.result < VK_SUCCESS) {
        LogErrorCode(record_obj);
    }
}

void BestPractices::PostCallRecordTransitionImageLayoutEXT(VkDevice device, uint32_t transitionCount,
                                                           const VkHostImageLayoutTransitionInfoEXT* pTransitions,
                                                           const RecordObject& record_obj) {
    ValidationStateTracker::PostCallRecordTransitionImageLayoutEXT(device, transitionCount, pTransitions, record_obj);

    if (record_obj.result < VK_SUCCESS) {
        LogErrorCode(record_obj);
    }
}

void BestPractices::PostCallRecordReleaseSwapchainImagesEXT(VkDevice device, const VkReleaseSwapchainImagesInfoEXT* pReleaseInfo,
                                                            const RecordObject& record_obj) {
    ValidationStateTracker::PostCallRecordReleaseSwapchainImagesEXT(device, pReleaseInfo, record_obj);

    if (record_obj.result < VK_SUCCESS) {
        LogErrorCode(record_obj);
    }
}

void BestPractices::PostCallRecordCreateIndirectCommandsLayoutNV(VkDevice device,
                                                                 const VkIndirectCommandsLayoutCreateInfoNV* pCreateInfo,
                                                                 const VkAllocationCallbacks* pAllocator,
                                                                 VkIndirectCommandsLayoutNV* pIndirectCommandsLayout,
                                                                 const RecordObject& record_obj) {
    ValidationStateTracker::PostCallRecordCreateIndirectCommandsLayoutNV(device, pCreateInfo, pAllocator, pIndirectCommandsLayout,
                                                                         record_obj);

    if (record_obj.result < VK_SUCCESS) {
        LogErrorCode(record_obj);
    }
}

void BestPractices::PostCallRecordAcquireDrmDisplayEXT(VkPhysicalDevice physicalDevice, int32_t drmFd, VkDisplayKHR display,
                                                       const RecordObject& record_obj) {
    ValidationStateTracker::PostCallRecordAcquireDrmDisplayEXT(physicalDevice, drmFd, display, record_obj);

    if (record_obj.result < VK_SUCCESS) {
        LogErrorCode(record_obj);
    }
}

void BestPractices::PostCallRecordGetDrmDisplayEXT(VkPhysicalDevice physicalDevice, int32_t drmFd, uint32_t connectorId,
                                                   VkDisplayKHR* display, const RecordObject& record_obj) {
    ValidationStateTracker::PostCallRecordGetDrmDisplayEXT(physicalDevice, drmFd, connectorId, display, record_obj);

    if (record_obj.result < VK_SUCCESS) {
        LogErrorCode(record_obj);
    }
}

void BestPractices::PostCallRecordCreatePrivateDataSlotEXT(VkDevice device, const VkPrivateDataSlotCreateInfo* pCreateInfo,
                                                           const VkAllocationCallbacks* pAllocator,
                                                           VkPrivateDataSlot* pPrivateDataSlot, const RecordObject& record_obj) {
    ValidationStateTracker::PostCallRecordCreatePrivateDataSlotEXT(device, pCreateInfo, pAllocator, pPrivateDataSlot, record_obj);

    if (record_obj.result < VK_SUCCESS) {
        LogErrorCode(record_obj);
    }
}

void BestPractices::PostCallRecordSetPrivateDataEXT(VkDevice device, VkObjectType objectType, uint64_t objectHandle,
                                                    VkPrivateDataSlot privateDataSlot, uint64_t data,
                                                    const RecordObject& record_obj) {
    ValidationStateTracker::PostCallRecordSetPrivateDataEXT(device, objectType, objectHandle, privateDataSlot, data, record_obj);

    if (record_obj.result < VK_SUCCESS) {
        LogErrorCode(record_obj);
    }
}

void BestPractices::PostCallRecordGetBufferOpaqueCaptureDescriptorDataEXT(VkDevice device,
                                                                          const VkBufferCaptureDescriptorDataInfoEXT* pInfo,
                                                                          void* pData, const RecordObject& record_obj) {
    ValidationStateTracker::PostCallRecordGetBufferOpaqueCaptureDescriptorDataEXT(device, pInfo, pData, record_obj);

    if (record_obj.result < VK_SUCCESS) {
        LogErrorCode(record_obj);
    }
}

void BestPractices::PostCallRecordGetImageOpaqueCaptureDescriptorDataEXT(VkDevice device,
                                                                         const VkImageCaptureDescriptorDataInfoEXT* pInfo,
                                                                         void* pData, const RecordObject& record_obj) {
    ValidationStateTracker::PostCallRecordGetImageOpaqueCaptureDescriptorDataEXT(device, pInfo, pData, record_obj);

    if (record_obj.result < VK_SUCCESS) {
        LogErrorCode(record_obj);
    }
}

void BestPractices::PostCallRecordGetImageViewOpaqueCaptureDescriptorDataEXT(VkDevice device,
                                                                             const VkImageViewCaptureDescriptorDataInfoEXT* pInfo,
                                                                             void* pData, const RecordObject& record_obj) {
    ValidationStateTracker::PostCallRecordGetImageViewOpaqueCaptureDescriptorDataEXT(device, pInfo, pData, record_obj);

    if (record_obj.result < VK_SUCCESS) {
        LogErrorCode(record_obj);
    }
}

void BestPractices::PostCallRecordGetSamplerOpaqueCaptureDescriptorDataEXT(VkDevice device,
                                                                           const VkSamplerCaptureDescriptorDataInfoEXT* pInfo,
                                                                           void* pData, const RecordObject& record_obj) {
    ValidationStateTracker::PostCallRecordGetSamplerOpaqueCaptureDescriptorDataEXT(device, pInfo, pData, record_obj);

    if (record_obj.result < VK_SUCCESS) {
        LogErrorCode(record_obj);
    }
}

void BestPractices::PostCallRecordGetAccelerationStructureOpaqueCaptureDescriptorDataEXT(
    VkDevice device, const VkAccelerationStructureCaptureDescriptorDataInfoEXT* pInfo, void* pData,
    const RecordObject& record_obj) {
    ValidationStateTracker::PostCallRecordGetAccelerationStructureOpaqueCaptureDescriptorDataEXT(device, pInfo, pData, record_obj);

    if (record_obj.result < VK_SUCCESS) {
        LogErrorCode(record_obj);
    }
}

void BestPractices::PostCallRecordGetDeviceFaultInfoEXT(VkDevice device, VkDeviceFaultCountsEXT* pFaultCounts,
                                                        VkDeviceFaultInfoEXT* pFaultInfo, const RecordObject& record_obj) {
    ValidationStateTracker::PostCallRecordGetDeviceFaultInfoEXT(device, pFaultCounts, pFaultInfo, record_obj);

    if (record_obj.result > VK_SUCCESS) {
        LogPositiveSuccessCode(record_obj);
        return;
    }
    if (record_obj.result < VK_SUCCESS) {
        LogErrorCode(record_obj);
    }
}

#ifdef VK_USE_PLATFORM_WIN32_KHR
void BestPractices::PostCallRecordAcquireWinrtDisplayNV(VkPhysicalDevice physicalDevice, VkDisplayKHR display,
                                                        const RecordObject& record_obj) {
    ValidationStateTracker::PostCallRecordAcquireWinrtDisplayNV(physicalDevice, display, record_obj);

    if (record_obj.result < VK_SUCCESS) {
        LogErrorCode(record_obj);
    }
}
#endif  // VK_USE_PLATFORM_WIN32_KHR

#ifdef VK_USE_PLATFORM_WIN32_KHR
void BestPractices::PostCallRecordGetWinrtDisplayNV(VkPhysicalDevice physicalDevice, uint32_t deviceRelativeId,
                                                    VkDisplayKHR* pDisplay, const RecordObject& record_obj) {
    ValidationStateTracker::PostCallRecordGetWinrtDisplayNV(physicalDevice, deviceRelativeId, pDisplay, record_obj);

    if (record_obj.result < VK_SUCCESS) {
        LogErrorCode(record_obj);
    }
}
#endif  // VK_USE_PLATFORM_WIN32_KHR

#ifdef VK_USE_PLATFORM_DIRECTFB_EXT
void BestPractices::PostCallRecordCreateDirectFBSurfaceEXT(VkInstance instance, const VkDirectFBSurfaceCreateInfoEXT* pCreateInfo,
                                                           const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface,
                                                           const RecordObject& record_obj) {
    ValidationStateTracker::PostCallRecordCreateDirectFBSurfaceEXT(instance, pCreateInfo, pAllocator, pSurface, record_obj);

    if (record_obj.result < VK_SUCCESS) {
        LogErrorCode(record_obj);
    }
}
#endif  // VK_USE_PLATFORM_DIRECTFB_EXT

#ifdef VK_USE_PLATFORM_FUCHSIA
void BestPractices::PostCallRecordGetMemoryZirconHandleFUCHSIA(VkDevice device,
                                                               const VkMemoryGetZirconHandleInfoFUCHSIA* pGetZirconHandleInfo,
                                                               zx_handle_t* pZirconHandle, const RecordObject& record_obj) {
    ValidationStateTracker::PostCallRecordGetMemoryZirconHandleFUCHSIA(device, pGetZirconHandleInfo, pZirconHandle, record_obj);

    if (record_obj.result < VK_SUCCESS) {
        LogErrorCode(record_obj);
    }
}
#endif  // VK_USE_PLATFORM_FUCHSIA

#ifdef VK_USE_PLATFORM_FUCHSIA
void BestPractices::PostCallRecordGetMemoryZirconHandlePropertiesFUCHSIA(
    VkDevice device, VkExternalMemoryHandleTypeFlagBits handleType, zx_handle_t zirconHandle,
    VkMemoryZirconHandlePropertiesFUCHSIA* pMemoryZirconHandleProperties, const RecordObject& record_obj) {
    ValidationStateTracker::PostCallRecordGetMemoryZirconHandlePropertiesFUCHSIA(device, handleType, zirconHandle,
                                                                                 pMemoryZirconHandleProperties, record_obj);

    if (record_obj.result < VK_SUCCESS) {
        LogErrorCode(record_obj);
    }
}
#endif  // VK_USE_PLATFORM_FUCHSIA

#ifdef VK_USE_PLATFORM_FUCHSIA
void BestPractices::PostCallRecordImportSemaphoreZirconHandleFUCHSIA(
    VkDevice device, const VkImportSemaphoreZirconHandleInfoFUCHSIA* pImportSemaphoreZirconHandleInfo,
    const RecordObject& record_obj) {
    ValidationStateTracker::PostCallRecordImportSemaphoreZirconHandleFUCHSIA(device, pImportSemaphoreZirconHandleInfo, record_obj);

    if (record_obj.result < VK_SUCCESS) {
        LogErrorCode(record_obj);
    }
}
#endif  // VK_USE_PLATFORM_FUCHSIA

#ifdef VK_USE_PLATFORM_FUCHSIA
void BestPractices::PostCallRecordGetSemaphoreZirconHandleFUCHSIA(VkDevice device,
                                                                  const VkSemaphoreGetZirconHandleInfoFUCHSIA* pGetZirconHandleInfo,
                                                                  zx_handle_t* pZirconHandle, const RecordObject& record_obj) {
    ValidationStateTracker::PostCallRecordGetSemaphoreZirconHandleFUCHSIA(device, pGetZirconHandleInfo, pZirconHandle, record_obj);

    if (record_obj.result < VK_SUCCESS) {
        LogErrorCode(record_obj);
    }
}
#endif  // VK_USE_PLATFORM_FUCHSIA

#ifdef VK_USE_PLATFORM_FUCHSIA
void BestPractices::PostCallRecordCreateBufferCollectionFUCHSIA(VkDevice device,
                                                                const VkBufferCollectionCreateInfoFUCHSIA* pCreateInfo,
                                                                const VkAllocationCallbacks* pAllocator,
                                                                VkBufferCollectionFUCHSIA* pCollection,
                                                                const RecordObject& record_obj) {
    ValidationStateTracker::PostCallRecordCreateBufferCollectionFUCHSIA(device, pCreateInfo, pAllocator, pCollection, record_obj);

    if (record_obj.result < VK_SUCCESS) {
        LogErrorCode(record_obj);
    }
}
#endif  // VK_USE_PLATFORM_FUCHSIA

#ifdef VK_USE_PLATFORM_FUCHSIA
void BestPractices::PostCallRecordSetBufferCollectionImageConstraintsFUCHSIA(
    VkDevice device, VkBufferCollectionFUCHSIA collection, const VkImageConstraintsInfoFUCHSIA* pImageConstraintsInfo,
    const RecordObject& record_obj) {
    ValidationStateTracker::PostCallRecordSetBufferCollectionImageConstraintsFUCHSIA(device, collection, pImageConstraintsInfo,
                                                                                     record_obj);

    if (record_obj.result < VK_SUCCESS) {
        LogErrorCode(record_obj);
    }
}
#endif  // VK_USE_PLATFORM_FUCHSIA

#ifdef VK_USE_PLATFORM_FUCHSIA
void BestPractices::PostCallRecordSetBufferCollectionBufferConstraintsFUCHSIA(
    VkDevice device, VkBufferCollectionFUCHSIA collection, const VkBufferConstraintsInfoFUCHSIA* pBufferConstraintsInfo,
    const RecordObject& record_obj) {
    ValidationStateTracker::PostCallRecordSetBufferCollectionBufferConstraintsFUCHSIA(device, collection, pBufferConstraintsInfo,
                                                                                      record_obj);

    if (record_obj.result < VK_SUCCESS) {
        LogErrorCode(record_obj);
    }
}
#endif  // VK_USE_PLATFORM_FUCHSIA

#ifdef VK_USE_PLATFORM_FUCHSIA
void BestPractices::PostCallRecordGetBufferCollectionPropertiesFUCHSIA(VkDevice device, VkBufferCollectionFUCHSIA collection,
                                                                       VkBufferCollectionPropertiesFUCHSIA* pProperties,
                                                                       const RecordObject& record_obj) {
    ValidationStateTracker::PostCallRecordGetBufferCollectionPropertiesFUCHSIA(device, collection, pProperties, record_obj);

    if (record_obj.result < VK_SUCCESS) {
        LogErrorCode(record_obj);
    }
}
#endif  // VK_USE_PLATFORM_FUCHSIA

void BestPractices::PostCallRecordGetDeviceSubpassShadingMaxWorkgroupSizeHUAWEI(VkDevice device, VkRenderPass renderpass,
                                                                                VkExtent2D* pMaxWorkgroupSize,
                                                                                const RecordObject& record_obj) {
    ValidationStateTracker::PostCallRecordGetDeviceSubpassShadingMaxWorkgroupSizeHUAWEI(device, renderpass, pMaxWorkgroupSize,
                                                                                        record_obj);

    if (record_obj.result > VK_SUCCESS) {
        LogPositiveSuccessCode(record_obj);
        return;
    }
    if (record_obj.result < VK_SUCCESS) {
        LogErrorCode(record_obj);
    }
}

void BestPractices::PostCallRecordGetMemoryRemoteAddressNV(VkDevice device,
                                                           const VkMemoryGetRemoteAddressInfoNV* pMemoryGetRemoteAddressInfo,
                                                           VkRemoteAddressNV* pAddress, const RecordObject& record_obj) {
    ValidationStateTracker::PostCallRecordGetMemoryRemoteAddressNV(device, pMemoryGetRemoteAddressInfo, pAddress, record_obj);

    if (record_obj.result < VK_SUCCESS) {
        LogErrorCode(record_obj);
    }
}

void BestPractices::PostCallRecordGetPipelinePropertiesEXT(VkDevice device, const VkPipelineInfoEXT* pPipelineInfo,
                                                           VkBaseOutStructure* pPipelineProperties,
                                                           const RecordObject& record_obj) {
    ValidationStateTracker::PostCallRecordGetPipelinePropertiesEXT(device, pPipelineInfo, pPipelineProperties, record_obj);

    if (record_obj.result < VK_SUCCESS) {
        LogErrorCode(record_obj);
    }
}

#ifdef VK_USE_PLATFORM_SCREEN_QNX
void BestPractices::PostCallRecordCreateScreenSurfaceQNX(VkInstance instance, const VkScreenSurfaceCreateInfoQNX* pCreateInfo,
                                                         const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface,
                                                         const RecordObject& record_obj) {
    ValidationStateTracker::PostCallRecordCreateScreenSurfaceQNX(instance, pCreateInfo, pAllocator, pSurface, record_obj);

    if (record_obj.result < VK_SUCCESS) {
        LogErrorCode(record_obj);
    }
}
#endif  // VK_USE_PLATFORM_SCREEN_QNX

void BestPractices::PostCallRecordCreateMicromapEXT(VkDevice device, const VkMicromapCreateInfoEXT* pCreateInfo,
                                                    const VkAllocationCallbacks* pAllocator, VkMicromapEXT* pMicromap,
                                                    const RecordObject& record_obj) {
    ValidationStateTracker::PostCallRecordCreateMicromapEXT(device, pCreateInfo, pAllocator, pMicromap, record_obj);

    if (record_obj.result < VK_SUCCESS) {
        LogErrorCode(record_obj);
    }
}

void BestPractices::PostCallRecordBuildMicromapsEXT(VkDevice device, VkDeferredOperationKHR deferredOperation, uint32_t infoCount,
                                                    const VkMicromapBuildInfoEXT* pInfos, const RecordObject& record_obj) {
    ValidationStateTracker::PostCallRecordBuildMicromapsEXT(device, deferredOperation, infoCount, pInfos, record_obj);

    if (record_obj.result > VK_SUCCESS) {
        LogPositiveSuccessCode(record_obj);
        return;
    }
    if (record_obj.result < VK_SUCCESS) {
        LogErrorCode(record_obj);
    }
}

void BestPractices::PostCallRecordCopyMicromapEXT(VkDevice device, VkDeferredOperationKHR deferredOperation,
                                                  const VkCopyMicromapInfoEXT* pInfo, const RecordObject& record_obj) {
    ValidationStateTracker::PostCallRecordCopyMicromapEXT(device, deferredOperation, pInfo, record_obj);

    if (record_obj.result > VK_SUCCESS) {
        LogPositiveSuccessCode(record_obj);
        return;
    }
    if (record_obj.result < VK_SUCCESS) {
        LogErrorCode(record_obj);
    }
}

void BestPractices::PostCallRecordCopyMicromapToMemoryEXT(VkDevice device, VkDeferredOperationKHR deferredOperation,
                                                          const VkCopyMicromapToMemoryInfoEXT* pInfo,
                                                          const RecordObject& record_obj) {
    ValidationStateTracker::PostCallRecordCopyMicromapToMemoryEXT(device, deferredOperation, pInfo, record_obj);

    if (record_obj.result > VK_SUCCESS) {
        LogPositiveSuccessCode(record_obj);
        return;
    }
    if (record_obj.result < VK_SUCCESS) {
        LogErrorCode(record_obj);
    }
}

void BestPractices::PostCallRecordCopyMemoryToMicromapEXT(VkDevice device, VkDeferredOperationKHR deferredOperation,
                                                          const VkCopyMemoryToMicromapInfoEXT* pInfo,
                                                          const RecordObject& record_obj) {
    ValidationStateTracker::PostCallRecordCopyMemoryToMicromapEXT(device, deferredOperation, pInfo, record_obj);

    if (record_obj.result > VK_SUCCESS) {
        LogPositiveSuccessCode(record_obj);
        return;
    }
    if (record_obj.result < VK_SUCCESS) {
        LogErrorCode(record_obj);
    }
}

void BestPractices::PostCallRecordWriteMicromapsPropertiesEXT(VkDevice device, uint32_t micromapCount,
                                                              const VkMicromapEXT* pMicromaps, VkQueryType queryType,
                                                              size_t dataSize, void* pData, size_t stride,
                                                              const RecordObject& record_obj) {
    ValidationStateTracker::PostCallRecordWriteMicromapsPropertiesEXT(device, micromapCount, pMicromaps, queryType, dataSize, pData,
                                                                      stride, record_obj);

    if (record_obj.result < VK_SUCCESS) {
        LogErrorCode(record_obj);
    }
}

void BestPractices::PostCallRecordGetPhysicalDeviceOpticalFlowImageFormatsNV(
    VkPhysicalDevice physicalDevice, const VkOpticalFlowImageFormatInfoNV* pOpticalFlowImageFormatInfo, uint32_t* pFormatCount,
    VkOpticalFlowImageFormatPropertiesNV* pImageFormatProperties, const RecordObject& record_obj) {
    ValidationStateTracker::PostCallRecordGetPhysicalDeviceOpticalFlowImageFormatsNV(
        physicalDevice, pOpticalFlowImageFormatInfo, pFormatCount, pImageFormatProperties, record_obj);

    if (record_obj.result > VK_SUCCESS) {
        LogPositiveSuccessCode(record_obj);
        return;
    }
    if (record_obj.result < VK_SUCCESS) {
        LogErrorCode(record_obj);
    }
}

void BestPractices::PostCallRecordCreateOpticalFlowSessionNV(VkDevice device, const VkOpticalFlowSessionCreateInfoNV* pCreateInfo,
                                                             const VkAllocationCallbacks* pAllocator,
                                                             VkOpticalFlowSessionNV* pSession, const RecordObject& record_obj) {
    ValidationStateTracker::PostCallRecordCreateOpticalFlowSessionNV(device, pCreateInfo, pAllocator, pSession, record_obj);

    if (record_obj.result < VK_SUCCESS) {
        LogErrorCode(record_obj);
    }
}

void BestPractices::PostCallRecordBindOpticalFlowSessionImageNV(VkDevice device, VkOpticalFlowSessionNV session,
                                                                VkOpticalFlowSessionBindingPointNV bindingPoint, VkImageView view,
                                                                VkImageLayout layout, const RecordObject& record_obj) {
    ValidationStateTracker::PostCallRecordBindOpticalFlowSessionImageNV(device, session, bindingPoint, view, layout, record_obj);

    if (record_obj.result < VK_SUCCESS) {
        LogErrorCode(record_obj);
    }
}

void BestPractices::PostCallRecordCreateShadersEXT(VkDevice device, uint32_t createInfoCount,
                                                   const VkShaderCreateInfoEXT* pCreateInfos,
                                                   const VkAllocationCallbacks* pAllocator, VkShaderEXT* pShaders,
                                                   const RecordObject& record_obj, void* state_data) {
    ValidationStateTracker::PostCallRecordCreateShadersEXT(device, createInfoCount, pCreateInfos, pAllocator, pShaders, record_obj,
                                                           state_data);

    if (record_obj.result < VK_SUCCESS) {
        LogErrorCode(record_obj);
    }
}

void BestPractices::PostCallRecordGetShaderBinaryDataEXT(VkDevice device, VkShaderEXT shader, size_t* pDataSize, void* pData,
                                                         const RecordObject& record_obj) {
    ValidationStateTracker::PostCallRecordGetShaderBinaryDataEXT(device, shader, pDataSize, pData, record_obj);

    if (record_obj.result > VK_SUCCESS) {
        LogPositiveSuccessCode(record_obj);
        return;
    }
    if (record_obj.result < VK_SUCCESS) {
        LogErrorCode(record_obj);
    }
}

void BestPractices::PostCallRecordGetFramebufferTilePropertiesQCOM(VkDevice device, VkFramebuffer framebuffer,
                                                                   uint32_t* pPropertiesCount, VkTilePropertiesQCOM* pProperties,
                                                                   const RecordObject& record_obj) {
    ValidationStateTracker::PostCallRecordGetFramebufferTilePropertiesQCOM(device, framebuffer, pPropertiesCount, pProperties,
                                                                           record_obj);

    if (record_obj.result > VK_SUCCESS) {
        LogPositiveSuccessCode(record_obj);
        return;
    }
}

void BestPractices::PostCallRecordSetLatencySleepModeNV(VkDevice device, VkSwapchainKHR swapchain,
                                                        const VkLatencySleepModeInfoNV* pSleepModeInfo,
                                                        const RecordObject& record_obj) {
    ValidationStateTracker::PostCallRecordSetLatencySleepModeNV(device, swapchain, pSleepModeInfo, record_obj);

    if (record_obj.result < VK_SUCCESS) {
        LogErrorCode(record_obj);
    }
}

void BestPractices::PostCallRecordLatencySleepNV(VkDevice device, VkSwapchainKHR swapchain, const VkLatencySleepInfoNV* pSleepInfo,
                                                 const RecordObject& record_obj) {
    ValidationStateTracker::PostCallRecordLatencySleepNV(device, swapchain, pSleepInfo, record_obj);

    if (record_obj.result < VK_SUCCESS) {
        LogErrorCode(record_obj);
    }
}

#ifdef VK_USE_PLATFORM_SCREEN_QNX
void BestPractices::PostCallRecordGetScreenBufferPropertiesQNX(VkDevice device, const struct _screen_buffer* buffer,
                                                               VkScreenBufferPropertiesQNX* pProperties,
                                                               const RecordObject& record_obj) {
    ValidationStateTracker::PostCallRecordGetScreenBufferPropertiesQNX(device, buffer, pProperties, record_obj);

    if (record_obj.result < VK_SUCCESS) {
        LogErrorCode(record_obj);
    }
}
#endif  // VK_USE_PLATFORM_SCREEN_QNX

void BestPractices::PostCallRecordCreateAccelerationStructureKHR(VkDevice device,
                                                                 const VkAccelerationStructureCreateInfoKHR* pCreateInfo,
                                                                 const VkAllocationCallbacks* pAllocator,
                                                                 VkAccelerationStructureKHR* pAccelerationStructure,
                                                                 const RecordObject& record_obj) {
    ValidationStateTracker::PostCallRecordCreateAccelerationStructureKHR(device, pCreateInfo, pAllocator, pAccelerationStructure,
                                                                         record_obj);

    if (record_obj.result < VK_SUCCESS) {
        LogErrorCode(record_obj);
    }
}

void BestPractices::PostCallRecordBuildAccelerationStructuresKHR(
    VkDevice device, VkDeferredOperationKHR deferredOperation, uint32_t infoCount,
    const VkAccelerationStructureBuildGeometryInfoKHR* pInfos,
    const VkAccelerationStructureBuildRangeInfoKHR* const* ppBuildRangeInfos, const RecordObject& record_obj) {
    ValidationStateTracker::PostCallRecordBuildAccelerationStructuresKHR(device, deferredOperation, infoCount, pInfos,
                                                                         ppBuildRangeInfos, record_obj);

    if (record_obj.result > VK_SUCCESS) {
        LogPositiveSuccessCode(record_obj);
        return;
    }
    if (record_obj.result < VK_SUCCESS) {
        LogErrorCode(record_obj);
    }
}

void BestPractices::PostCallRecordCopyAccelerationStructureKHR(VkDevice device, VkDeferredOperationKHR deferredOperation,
                                                               const VkCopyAccelerationStructureInfoKHR* pInfo,
                                                               const RecordObject& record_obj) {
    ValidationStateTracker::PostCallRecordCopyAccelerationStructureKHR(device, deferredOperation, pInfo, record_obj);

    if (record_obj.result > VK_SUCCESS) {
        LogPositiveSuccessCode(record_obj);
        return;
    }
    if (record_obj.result < VK_SUCCESS) {
        LogErrorCode(record_obj);
    }
}

void BestPractices::PostCallRecordCopyAccelerationStructureToMemoryKHR(VkDevice device, VkDeferredOperationKHR deferredOperation,
                                                                       const VkCopyAccelerationStructureToMemoryInfoKHR* pInfo,
                                                                       const RecordObject& record_obj) {
    ValidationStateTracker::PostCallRecordCopyAccelerationStructureToMemoryKHR(device, deferredOperation, pInfo, record_obj);

    if (record_obj.result > VK_SUCCESS) {
        LogPositiveSuccessCode(record_obj);
        return;
    }
    if (record_obj.result < VK_SUCCESS) {
        LogErrorCode(record_obj);
    }
}

void BestPractices::PostCallRecordCopyMemoryToAccelerationStructureKHR(VkDevice device, VkDeferredOperationKHR deferredOperation,
                                                                       const VkCopyMemoryToAccelerationStructureInfoKHR* pInfo,
                                                                       const RecordObject& record_obj) {
    ValidationStateTracker::PostCallRecordCopyMemoryToAccelerationStructureKHR(device, deferredOperation, pInfo, record_obj);

    if (record_obj.result > VK_SUCCESS) {
        LogPositiveSuccessCode(record_obj);
        return;
    }
    if (record_obj.result < VK_SUCCESS) {
        LogErrorCode(record_obj);
    }
}

void BestPractices::PostCallRecordWriteAccelerationStructuresPropertiesKHR(
    VkDevice device, uint32_t accelerationStructureCount, const VkAccelerationStructureKHR* pAccelerationStructures,
    VkQueryType queryType, size_t dataSize, void* pData, size_t stride, const RecordObject& record_obj) {
    ValidationStateTracker::PostCallRecordWriteAccelerationStructuresPropertiesKHR(
        device, accelerationStructureCount, pAccelerationStructures, queryType, dataSize, pData, stride, record_obj);

    if (record_obj.result < VK_SUCCESS) {
        LogErrorCode(record_obj);
    }
}

void BestPractices::PostCallRecordCreateRayTracingPipelinesKHR(VkDevice device, VkDeferredOperationKHR deferredOperation,
                                                               VkPipelineCache pipelineCache, uint32_t createInfoCount,
                                                               const VkRayTracingPipelineCreateInfoKHR* pCreateInfos,
                                                               const VkAllocationCallbacks* pAllocator, VkPipeline* pPipelines,
                                                               const RecordObject& record_obj, void* state_data) {
    ValidationStateTracker::PostCallRecordCreateRayTracingPipelinesKHR(
        device, deferredOperation, pipelineCache, createInfoCount, pCreateInfos, pAllocator, pPipelines, record_obj, state_data);

    if (record_obj.result > VK_SUCCESS) {
        LogPositiveSuccessCode(record_obj);
        return;
    }
    if (record_obj.result < VK_SUCCESS) {
        LogErrorCode(record_obj);
    }
}

void BestPractices::PostCallRecordGetRayTracingCaptureReplayShaderGroupHandlesKHR(VkDevice device, VkPipeline pipeline,
                                                                                  uint32_t firstGroup, uint32_t groupCount,
                                                                                  size_t dataSize, void* pData,
                                                                                  const RecordObject& record_obj) {
    ValidationStateTracker::PostCallRecordGetRayTracingCaptureReplayShaderGroupHandlesKHR(device, pipeline, firstGroup, groupCount,
                                                                                          dataSize, pData, record_obj);

    if (record_obj.result < VK_SUCCESS) {
        LogErrorCode(record_obj);
    }
}

// NOLINTEND
