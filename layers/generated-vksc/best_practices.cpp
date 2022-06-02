// *** THIS FILE IS GENERATED - DO NOT EDIT ***
// See best_practices_generator.py for modifications


/***************************************************************************
 *
 * Copyright (c) 2015-2021 The Khronos Group Inc.
 * Copyright (c) 2015-2021 Valve Corporation
 * Copyright (c) 2015-2021 LunarG, Inc.
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
 *
 * Author: Mark Lobodzinski <mark@lunarg.com>
 * Author: Nadav Geva <nadav.geva@amd.com>
 *
 ****************************************************************************/


#include "chassis.h"
#include "best_practices_validation.h"
void BestPractices::PostCallRecordCreateInstance(
    const VkInstanceCreateInfo*                 pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkInstance*                                 pInstance,
    VkResult                                    result) {
    ValidationStateTracker::PostCallRecordCreateInstance(pCreateInfo, pAllocator, pInstance, result);
    if (result != VK_SUCCESS) {
        static const std::vector<VkResult> error_codes = {VK_ERROR_OUT_OF_HOST_MEMORY,VK_ERROR_OUT_OF_DEVICE_MEMORY,VK_ERROR_INITIALIZATION_FAILED,VK_ERROR_LAYER_NOT_PRESENT,VK_ERROR_EXTENSION_NOT_PRESENT,VK_ERROR_INCOMPATIBLE_DRIVER};
        static const std::vector<VkResult> success_codes = {};
        ValidateReturnCodes("vkCreateInstance", result, error_codes, success_codes);
    }
}

void BestPractices::PostCallRecordEnumeratePhysicalDevices(
    VkInstance                                  instance,
    uint32_t*                                   pPhysicalDeviceCount,
    VkPhysicalDevice*                           pPhysicalDevices,
    VkResult                                    result) {
    ValidationStateTracker::PostCallRecordEnumeratePhysicalDevices(instance, pPhysicalDeviceCount, pPhysicalDevices, result);
    if (result != VK_SUCCESS) {
        static const std::vector<VkResult> error_codes = {VK_ERROR_OUT_OF_HOST_MEMORY,VK_ERROR_OUT_OF_DEVICE_MEMORY,VK_ERROR_INITIALIZATION_FAILED};
        static const std::vector<VkResult> success_codes = {VK_INCOMPLETE};
        ValidateReturnCodes("vkEnumeratePhysicalDevices", result, error_codes, success_codes);
    }
}

void BestPractices::PostCallRecordGetPhysicalDeviceImageFormatProperties(
    VkPhysicalDevice                            physicalDevice,
    VkFormat                                    format,
    VkImageType                                 type,
    VkImageTiling                               tiling,
    VkImageUsageFlags                           usage,
    VkImageCreateFlags                          flags,
    VkImageFormatProperties*                    pImageFormatProperties,
    VkResult                                    result) {
    ValidationStateTracker::PostCallRecordGetPhysicalDeviceImageFormatProperties(physicalDevice, format, type, tiling, usage, flags, pImageFormatProperties, result);
    if (result != VK_SUCCESS) {
        static const std::vector<VkResult> error_codes = {VK_ERROR_OUT_OF_HOST_MEMORY,VK_ERROR_OUT_OF_DEVICE_MEMORY,VK_ERROR_FORMAT_NOT_SUPPORTED};
        static const std::vector<VkResult> success_codes = {};
        ValidateReturnCodes("vkGetPhysicalDeviceImageFormatProperties", result, error_codes, success_codes);
    }
}

void BestPractices::PostCallRecordCreateDevice(
    VkPhysicalDevice                            physicalDevice,
    const VkDeviceCreateInfo*                   pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkDevice*                                   pDevice,
    VkResult                                    result) {
    ValidationStateTracker::PostCallRecordCreateDevice(physicalDevice, pCreateInfo, pAllocator, pDevice, result);
    ManualPostCallRecordCreateDevice(physicalDevice, pCreateInfo, pAllocator, pDevice, result);
    if (result != VK_SUCCESS) {
        static const std::vector<VkResult> error_codes = {VK_ERROR_OUT_OF_HOST_MEMORY,VK_ERROR_OUT_OF_DEVICE_MEMORY,VK_ERROR_INITIALIZATION_FAILED,VK_ERROR_EXTENSION_NOT_PRESENT,VK_ERROR_FEATURE_NOT_PRESENT,VK_ERROR_TOO_MANY_OBJECTS,VK_ERROR_DEVICE_LOST,VK_ERROR_INVALID_PIPELINE_CACHE_DATA};
        static const std::vector<VkResult> success_codes = {};
        ValidateReturnCodes("vkCreateDevice", result, error_codes, success_codes);
    }
}

void BestPractices::PostCallRecordEnumerateInstanceExtensionProperties(
    const char*                                 pLayerName,
    uint32_t*                                   pPropertyCount,
    VkExtensionProperties*                      pProperties,
    VkResult                                    result) {
    ValidationStateTracker::PostCallRecordEnumerateInstanceExtensionProperties(pLayerName, pPropertyCount, pProperties, result);
    if (result != VK_SUCCESS) {
        static const std::vector<VkResult> error_codes = {VK_ERROR_OUT_OF_HOST_MEMORY,VK_ERROR_OUT_OF_DEVICE_MEMORY,VK_ERROR_LAYER_NOT_PRESENT};
        static const std::vector<VkResult> success_codes = {VK_INCOMPLETE};
        ValidateReturnCodes("vkEnumerateInstanceExtensionProperties", result, error_codes, success_codes);
    }
}

void BestPractices::PostCallRecordEnumerateDeviceExtensionProperties(
    VkPhysicalDevice                            physicalDevice,
    const char*                                 pLayerName,
    uint32_t*                                   pPropertyCount,
    VkExtensionProperties*                      pProperties,
    VkResult                                    result) {
    ValidationStateTracker::PostCallRecordEnumerateDeviceExtensionProperties(physicalDevice, pLayerName, pPropertyCount, pProperties, result);
    if (result != VK_SUCCESS) {
        static const std::vector<VkResult> error_codes = {VK_ERROR_OUT_OF_HOST_MEMORY,VK_ERROR_OUT_OF_DEVICE_MEMORY,VK_ERROR_LAYER_NOT_PRESENT};
        static const std::vector<VkResult> success_codes = {VK_INCOMPLETE};
        ValidateReturnCodes("vkEnumerateDeviceExtensionProperties", result, error_codes, success_codes);
    }
}

void BestPractices::PostCallRecordEnumerateInstanceLayerProperties(
    uint32_t*                                   pPropertyCount,
    VkLayerProperties*                          pProperties,
    VkResult                                    result) {
    ValidationStateTracker::PostCallRecordEnumerateInstanceLayerProperties(pPropertyCount, pProperties, result);
    if (result != VK_SUCCESS) {
        static const std::vector<VkResult> error_codes = {VK_ERROR_OUT_OF_HOST_MEMORY,VK_ERROR_OUT_OF_DEVICE_MEMORY};
        static const std::vector<VkResult> success_codes = {VK_INCOMPLETE};
        ValidateReturnCodes("vkEnumerateInstanceLayerProperties", result, error_codes, success_codes);
    }
}

void BestPractices::PostCallRecordEnumerateDeviceLayerProperties(
    VkPhysicalDevice                            physicalDevice,
    uint32_t*                                   pPropertyCount,
    VkLayerProperties*                          pProperties,
    VkResult                                    result) {
    ValidationStateTracker::PostCallRecordEnumerateDeviceLayerProperties(physicalDevice, pPropertyCount, pProperties, result);
    if (result != VK_SUCCESS) {
        static const std::vector<VkResult> error_codes = {VK_ERROR_OUT_OF_HOST_MEMORY,VK_ERROR_OUT_OF_DEVICE_MEMORY};
        static const std::vector<VkResult> success_codes = {VK_INCOMPLETE};
        ValidateReturnCodes("vkEnumerateDeviceLayerProperties", result, error_codes, success_codes);
    }
}

void BestPractices::PostCallRecordQueueSubmit(
    VkQueue                                     queue,
    uint32_t                                    submitCount,
    const VkSubmitInfo*                         pSubmits,
    VkFence                                     fence,
    VkResult                                    result) {
    ValidationStateTracker::PostCallRecordQueueSubmit(queue, submitCount, pSubmits, fence, result);
    ManualPostCallRecordQueueSubmit(queue, submitCount, pSubmits, fence, result);
    if (result != VK_SUCCESS) {
        static const std::vector<VkResult> error_codes = {VK_ERROR_OUT_OF_HOST_MEMORY,VK_ERROR_OUT_OF_DEVICE_MEMORY,VK_ERROR_DEVICE_LOST};
        static const std::vector<VkResult> success_codes = {};
        ValidateReturnCodes("vkQueueSubmit", result, error_codes, success_codes);
    }
}

void BestPractices::PostCallRecordQueueWaitIdle(
    VkQueue                                     queue,
    VkResult                                    result) {
    ValidationStateTracker::PostCallRecordQueueWaitIdle(queue, result);
    if (result != VK_SUCCESS) {
        static const std::vector<VkResult> error_codes = {VK_ERROR_OUT_OF_HOST_MEMORY,VK_ERROR_OUT_OF_DEVICE_MEMORY,VK_ERROR_DEVICE_LOST};
        static const std::vector<VkResult> success_codes = {};
        ValidateReturnCodes("vkQueueWaitIdle", result, error_codes, success_codes);
    }
}

void BestPractices::PostCallRecordDeviceWaitIdle(
    VkDevice                                    device,
    VkResult                                    result) {
    ValidationStateTracker::PostCallRecordDeviceWaitIdle(device, result);
    if (result != VK_SUCCESS) {
        static const std::vector<VkResult> error_codes = {VK_ERROR_OUT_OF_HOST_MEMORY,VK_ERROR_OUT_OF_DEVICE_MEMORY,VK_ERROR_DEVICE_LOST};
        static const std::vector<VkResult> success_codes = {};
        ValidateReturnCodes("vkDeviceWaitIdle", result, error_codes, success_codes);
    }
}

void BestPractices::PostCallRecordAllocateMemory(
    VkDevice                                    device,
    const VkMemoryAllocateInfo*                 pAllocateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkDeviceMemory*                             pMemory,
    VkResult                                    result) {
    ValidationStateTracker::PostCallRecordAllocateMemory(device, pAllocateInfo, pAllocator, pMemory, result);
    ManualPostCallRecordAllocateMemory(device, pAllocateInfo, pAllocator, pMemory, result);
    if (result != VK_SUCCESS) {
        static const std::vector<VkResult> error_codes = {VK_ERROR_OUT_OF_HOST_MEMORY,VK_ERROR_OUT_OF_DEVICE_MEMORY,VK_ERROR_INVALID_EXTERNAL_HANDLE};
        static const std::vector<VkResult> success_codes = {};
        ValidateReturnCodes("vkAllocateMemory", result, error_codes, success_codes);
    }
}

void BestPractices::PostCallRecordMapMemory(
    VkDevice                                    device,
    VkDeviceMemory                              memory,
    VkDeviceSize                                offset,
    VkDeviceSize                                size,
    VkMemoryMapFlags                            flags,
    void**                                      ppData,
    VkResult                                    result) {
    ValidationStateTracker::PostCallRecordMapMemory(device, memory, offset, size, flags, ppData, result);
    if (result != VK_SUCCESS) {
        static const std::vector<VkResult> error_codes = {VK_ERROR_OUT_OF_HOST_MEMORY,VK_ERROR_OUT_OF_DEVICE_MEMORY,VK_ERROR_MEMORY_MAP_FAILED};
        static const std::vector<VkResult> success_codes = {};
        ValidateReturnCodes("vkMapMemory", result, error_codes, success_codes);
    }
}

void BestPractices::PostCallRecordFlushMappedMemoryRanges(
    VkDevice                                    device,
    uint32_t                                    memoryRangeCount,
    const VkMappedMemoryRange*                  pMemoryRanges,
    VkResult                                    result) {
    ValidationStateTracker::PostCallRecordFlushMappedMemoryRanges(device, memoryRangeCount, pMemoryRanges, result);
    if (result != VK_SUCCESS) {
        static const std::vector<VkResult> error_codes = {VK_ERROR_OUT_OF_HOST_MEMORY,VK_ERROR_OUT_OF_DEVICE_MEMORY};
        static const std::vector<VkResult> success_codes = {};
        ValidateReturnCodes("vkFlushMappedMemoryRanges", result, error_codes, success_codes);
    }
}

void BestPractices::PostCallRecordInvalidateMappedMemoryRanges(
    VkDevice                                    device,
    uint32_t                                    memoryRangeCount,
    const VkMappedMemoryRange*                  pMemoryRanges,
    VkResult                                    result) {
    ValidationStateTracker::PostCallRecordInvalidateMappedMemoryRanges(device, memoryRangeCount, pMemoryRanges, result);
    if (result != VK_SUCCESS) {
        static const std::vector<VkResult> error_codes = {VK_ERROR_OUT_OF_HOST_MEMORY,VK_ERROR_OUT_OF_DEVICE_MEMORY};
        static const std::vector<VkResult> success_codes = {};
        ValidateReturnCodes("vkInvalidateMappedMemoryRanges", result, error_codes, success_codes);
    }
}

void BestPractices::PostCallRecordBindBufferMemory(
    VkDevice                                    device,
    VkBuffer                                    buffer,
    VkDeviceMemory                              memory,
    VkDeviceSize                                memoryOffset,
    VkResult                                    result) {
    ValidationStateTracker::PostCallRecordBindBufferMemory(device, buffer, memory, memoryOffset, result);
    if (result != VK_SUCCESS) {
        static const std::vector<VkResult> error_codes = {VK_ERROR_OUT_OF_HOST_MEMORY,VK_ERROR_OUT_OF_DEVICE_MEMORY};
        static const std::vector<VkResult> success_codes = {};
        ValidateReturnCodes("vkBindBufferMemory", result, error_codes, success_codes);
    }
}

void BestPractices::PostCallRecordBindImageMemory(
    VkDevice                                    device,
    VkImage                                     image,
    VkDeviceMemory                              memory,
    VkDeviceSize                                memoryOffset,
    VkResult                                    result) {
    ValidationStateTracker::PostCallRecordBindImageMemory(device, image, memory, memoryOffset, result);
    if (result != VK_SUCCESS) {
        static const std::vector<VkResult> error_codes = {VK_ERROR_OUT_OF_HOST_MEMORY,VK_ERROR_OUT_OF_DEVICE_MEMORY};
        static const std::vector<VkResult> success_codes = {};
        ValidateReturnCodes("vkBindImageMemory", result, error_codes, success_codes);
    }
}

void BestPractices::PostCallRecordCreateFence(
    VkDevice                                    device,
    const VkFenceCreateInfo*                    pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkFence*                                    pFence,
    VkResult                                    result) {
    ValidationStateTracker::PostCallRecordCreateFence(device, pCreateInfo, pAllocator, pFence, result);
    ManualPostCallRecordCreateFence(device, pCreateInfo, pAllocator, pFence, result);
    if (result != VK_SUCCESS) {
        static const std::vector<VkResult> error_codes = {VK_ERROR_OUT_OF_HOST_MEMORY,VK_ERROR_OUT_OF_DEVICE_MEMORY};
        static const std::vector<VkResult> success_codes = {};
        ValidateReturnCodes("vkCreateFence", result, error_codes, success_codes);
    }
}

void BestPractices::PostCallRecordResetFences(
    VkDevice                                    device,
    uint32_t                                    fenceCount,
    const VkFence*                              pFences,
    VkResult                                    result) {
    ValidationStateTracker::PostCallRecordResetFences(device, fenceCount, pFences, result);
    if (result != VK_SUCCESS) {
        static const std::vector<VkResult> error_codes = {VK_ERROR_OUT_OF_DEVICE_MEMORY};
        static const std::vector<VkResult> success_codes = {};
        ValidateReturnCodes("vkResetFences", result, error_codes, success_codes);
    }
}

void BestPractices::PostCallRecordGetFenceStatus(
    VkDevice                                    device,
    VkFence                                     fence,
    VkResult                                    result) {
    ValidationStateTracker::PostCallRecordGetFenceStatus(device, fence, result);
    if (result != VK_SUCCESS) {
        static const std::vector<VkResult> error_codes = {VK_ERROR_OUT_OF_HOST_MEMORY,VK_ERROR_OUT_OF_DEVICE_MEMORY,VK_ERROR_DEVICE_LOST};
        static const std::vector<VkResult> success_codes = {VK_NOT_READY};
        ValidateReturnCodes("vkGetFenceStatus", result, error_codes, success_codes);
    }
}

void BestPractices::PostCallRecordWaitForFences(
    VkDevice                                    device,
    uint32_t                                    fenceCount,
    const VkFence*                              pFences,
    VkBool32                                    waitAll,
    uint64_t                                    timeout,
    VkResult                                    result) {
    ValidationStateTracker::PostCallRecordWaitForFences(device, fenceCount, pFences, waitAll, timeout, result);
    if (result != VK_SUCCESS) {
        static const std::vector<VkResult> error_codes = {VK_ERROR_OUT_OF_HOST_MEMORY,VK_ERROR_OUT_OF_DEVICE_MEMORY,VK_ERROR_DEVICE_LOST};
        static const std::vector<VkResult> success_codes = {VK_TIMEOUT};
        ValidateReturnCodes("vkWaitForFences", result, error_codes, success_codes);
    }
}

void BestPractices::PostCallRecordCreateSemaphore(
    VkDevice                                    device,
    const VkSemaphoreCreateInfo*                pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkSemaphore*                                pSemaphore,
    VkResult                                    result) {
    ValidationStateTracker::PostCallRecordCreateSemaphore(device, pCreateInfo, pAllocator, pSemaphore, result);
    ManualPostCallRecordCreateSemaphore(device, pCreateInfo, pAllocator, pSemaphore, result);
    if (result != VK_SUCCESS) {
        static const std::vector<VkResult> error_codes = {VK_ERROR_OUT_OF_HOST_MEMORY,VK_ERROR_OUT_OF_DEVICE_MEMORY};
        static const std::vector<VkResult> success_codes = {};
        ValidateReturnCodes("vkCreateSemaphore", result, error_codes, success_codes);
    }
}

void BestPractices::PostCallRecordCreateEvent(
    VkDevice                                    device,
    const VkEventCreateInfo*                    pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkEvent*                                    pEvent,
    VkResult                                    result) {
    ValidationStateTracker::PostCallRecordCreateEvent(device, pCreateInfo, pAllocator, pEvent, result);
    if (result != VK_SUCCESS) {
        static const std::vector<VkResult> error_codes = {VK_ERROR_OUT_OF_HOST_MEMORY,VK_ERROR_OUT_OF_DEVICE_MEMORY};
        static const std::vector<VkResult> success_codes = {};
        ValidateReturnCodes("vkCreateEvent", result, error_codes, success_codes);
    }
}

void BestPractices::PostCallRecordGetEventStatus(
    VkDevice                                    device,
    VkEvent                                     event,
    VkResult                                    result) {
    ValidationStateTracker::PostCallRecordGetEventStatus(device, event, result);
    if (result != VK_SUCCESS) {
        static const std::vector<VkResult> error_codes = {VK_ERROR_OUT_OF_HOST_MEMORY,VK_ERROR_OUT_OF_DEVICE_MEMORY,VK_ERROR_DEVICE_LOST};
        static const std::vector<VkResult> success_codes = {VK_EVENT_SET,VK_EVENT_RESET};
        ValidateReturnCodes("vkGetEventStatus", result, error_codes, success_codes);
    }
}

void BestPractices::PostCallRecordSetEvent(
    VkDevice                                    device,
    VkEvent                                     event,
    VkResult                                    result) {
    ValidationStateTracker::PostCallRecordSetEvent(device, event, result);
    if (result != VK_SUCCESS) {
        static const std::vector<VkResult> error_codes = {VK_ERROR_OUT_OF_HOST_MEMORY,VK_ERROR_OUT_OF_DEVICE_MEMORY};
        static const std::vector<VkResult> success_codes = {};
        ValidateReturnCodes("vkSetEvent", result, error_codes, success_codes);
    }
}

void BestPractices::PostCallRecordResetEvent(
    VkDevice                                    device,
    VkEvent                                     event,
    VkResult                                    result) {
    ValidationStateTracker::PostCallRecordResetEvent(device, event, result);
    if (result != VK_SUCCESS) {
        static const std::vector<VkResult> error_codes = {VK_ERROR_OUT_OF_DEVICE_MEMORY};
        static const std::vector<VkResult> success_codes = {};
        ValidateReturnCodes("vkResetEvent", result, error_codes, success_codes);
    }
}

void BestPractices::PostCallRecordCreateQueryPool(
    VkDevice                                    device,
    const VkQueryPoolCreateInfo*                pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkQueryPool*                                pQueryPool,
    VkResult                                    result) {
    ValidationStateTracker::PostCallRecordCreateQueryPool(device, pCreateInfo, pAllocator, pQueryPool, result);
    if (result != VK_SUCCESS) {
        static const std::vector<VkResult> error_codes = {VK_ERROR_OUT_OF_HOST_MEMORY,VK_ERROR_OUT_OF_DEVICE_MEMORY};
        static const std::vector<VkResult> success_codes = {};
        ValidateReturnCodes("vkCreateQueryPool", result, error_codes, success_codes);
    }
}

void BestPractices::PostCallRecordGetQueryPoolResults(
    VkDevice                                    device,
    VkQueryPool                                 queryPool,
    uint32_t                                    firstQuery,
    uint32_t                                    queryCount,
    size_t                                      dataSize,
    void*                                       pData,
    VkDeviceSize                                stride,
    VkQueryResultFlags                          flags,
    VkResult                                    result) {
    ValidationStateTracker::PostCallRecordGetQueryPoolResults(device, queryPool, firstQuery, queryCount, dataSize, pData, stride, flags, result);
    if (result != VK_SUCCESS) {
        static const std::vector<VkResult> error_codes = {VK_ERROR_OUT_OF_HOST_MEMORY,VK_ERROR_OUT_OF_DEVICE_MEMORY,VK_ERROR_DEVICE_LOST};
        static const std::vector<VkResult> success_codes = {VK_NOT_READY};
        ValidateReturnCodes("vkGetQueryPoolResults", result, error_codes, success_codes);
    }
}

void BestPractices::PostCallRecordCreateBuffer(
    VkDevice                                    device,
    const VkBufferCreateInfo*                   pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkBuffer*                                   pBuffer,
    VkResult                                    result) {
    ValidationStateTracker::PostCallRecordCreateBuffer(device, pCreateInfo, pAllocator, pBuffer, result);
    if (result != VK_SUCCESS) {
        static const std::vector<VkResult> error_codes = {VK_ERROR_OUT_OF_HOST_MEMORY,VK_ERROR_OUT_OF_DEVICE_MEMORY};
        static const std::vector<VkResult> success_codes = {};
        ValidateReturnCodes("vkCreateBuffer", result, error_codes, success_codes);
    }
}

void BestPractices::PostCallRecordCreateBufferView(
    VkDevice                                    device,
    const VkBufferViewCreateInfo*               pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkBufferView*                               pView,
    VkResult                                    result) {
    ValidationStateTracker::PostCallRecordCreateBufferView(device, pCreateInfo, pAllocator, pView, result);
    if (result != VK_SUCCESS) {
        static const std::vector<VkResult> error_codes = {VK_ERROR_OUT_OF_HOST_MEMORY,VK_ERROR_OUT_OF_DEVICE_MEMORY};
        static const std::vector<VkResult> success_codes = {};
        ValidateReturnCodes("vkCreateBufferView", result, error_codes, success_codes);
    }
}

void BestPractices::PostCallRecordCreateImage(
    VkDevice                                    device,
    const VkImageCreateInfo*                    pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkImage*                                    pImage,
    VkResult                                    result) {
    ValidationStateTracker::PostCallRecordCreateImage(device, pCreateInfo, pAllocator, pImage, result);
    if (result != VK_SUCCESS) {
        static const std::vector<VkResult> error_codes = {VK_ERROR_OUT_OF_HOST_MEMORY,VK_ERROR_OUT_OF_DEVICE_MEMORY};
        static const std::vector<VkResult> success_codes = {};
        ValidateReturnCodes("vkCreateImage", result, error_codes, success_codes);
    }
}

void BestPractices::PostCallRecordCreateImageView(
    VkDevice                                    device,
    const VkImageViewCreateInfo*                pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkImageView*                                pView,
    VkResult                                    result) {
    ValidationStateTracker::PostCallRecordCreateImageView(device, pCreateInfo, pAllocator, pView, result);
    if (result != VK_SUCCESS) {
        static const std::vector<VkResult> error_codes = {VK_ERROR_OUT_OF_HOST_MEMORY,VK_ERROR_OUT_OF_DEVICE_MEMORY};
        static const std::vector<VkResult> success_codes = {};
        ValidateReturnCodes("vkCreateImageView", result, error_codes, success_codes);
    }
}

void BestPractices::PostCallRecordCreatePipelineCache(
    VkDevice                                    device,
    const VkPipelineCacheCreateInfo*            pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkPipelineCache*                            pPipelineCache,
    VkResult                                    result) {
    ValidationStateTracker::PostCallRecordCreatePipelineCache(device, pCreateInfo, pAllocator, pPipelineCache, result);
    if (result != VK_SUCCESS) {
        static const std::vector<VkResult> error_codes = {VK_ERROR_OUT_OF_HOST_MEMORY,VK_ERROR_OUT_OF_DEVICE_MEMORY,VK_ERROR_INVALID_PIPELINE_CACHE_DATA};
        static const std::vector<VkResult> success_codes = {};
        ValidateReturnCodes("vkCreatePipelineCache", result, error_codes, success_codes);
    }
}

void BestPractices::PostCallRecordCreateGraphicsPipelines(
    VkDevice                                    device,
    VkPipelineCache                             pipelineCache,
    uint32_t                                    createInfoCount,
    const VkGraphicsPipelineCreateInfo*         pCreateInfos,
    const VkAllocationCallbacks*                pAllocator,
    VkPipeline*                                 pPipelines,
    VkResult                                    result,
    void*                                       state_data) {
    ValidationStateTracker::PostCallRecordCreateGraphicsPipelines(device, pipelineCache, createInfoCount, pCreateInfos, pAllocator, pPipelines, result, state_data);
    ManualPostCallRecordCreateGraphicsPipelines(device, pipelineCache, createInfoCount, pCreateInfos, pAllocator, pPipelines, result, state_data);
    if (result != VK_SUCCESS) {
        static const std::vector<VkResult> error_codes = {VK_ERROR_OUT_OF_HOST_MEMORY,VK_ERROR_OUT_OF_DEVICE_MEMORY,VK_ERROR_NO_PIPELINE_MATCH,VK_ERROR_OUT_OF_POOL_MEMORY};
        static const std::vector<VkResult> success_codes = {};
        ValidateReturnCodes("vkCreateGraphicsPipelines", result, error_codes, success_codes);
    }
}

void BestPractices::PostCallRecordCreateComputePipelines(
    VkDevice                                    device,
    VkPipelineCache                             pipelineCache,
    uint32_t                                    createInfoCount,
    const VkComputePipelineCreateInfo*          pCreateInfos,
    const VkAllocationCallbacks*                pAllocator,
    VkPipeline*                                 pPipelines,
    VkResult                                    result,
    void*                                       state_data) {
    ValidationStateTracker::PostCallRecordCreateComputePipelines(device, pipelineCache, createInfoCount, pCreateInfos, pAllocator, pPipelines, result, state_data);
    ManualPostCallRecordCreateComputePipelines(device, pipelineCache, createInfoCount, pCreateInfos, pAllocator, pPipelines, result, state_data);
    if (result != VK_SUCCESS) {
        static const std::vector<VkResult> error_codes = {VK_ERROR_OUT_OF_HOST_MEMORY,VK_ERROR_OUT_OF_DEVICE_MEMORY,VK_ERROR_NO_PIPELINE_MATCH,VK_ERROR_OUT_OF_POOL_MEMORY};
        static const std::vector<VkResult> success_codes = {};
        ValidateReturnCodes("vkCreateComputePipelines", result, error_codes, success_codes);
    }
}

void BestPractices::PostCallRecordCreatePipelineLayout(
    VkDevice                                    device,
    const VkPipelineLayoutCreateInfo*           pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkPipelineLayout*                           pPipelineLayout,
    VkResult                                    result) {
    ValidationStateTracker::PostCallRecordCreatePipelineLayout(device, pCreateInfo, pAllocator, pPipelineLayout, result);
    if (result != VK_SUCCESS) {
        static const std::vector<VkResult> error_codes = {VK_ERROR_OUT_OF_HOST_MEMORY,VK_ERROR_OUT_OF_DEVICE_MEMORY};
        static const std::vector<VkResult> success_codes = {};
        ValidateReturnCodes("vkCreatePipelineLayout", result, error_codes, success_codes);
    }
}

void BestPractices::PostCallRecordCreateSampler(
    VkDevice                                    device,
    const VkSamplerCreateInfo*                  pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkSampler*                                  pSampler,
    VkResult                                    result) {
    ValidationStateTracker::PostCallRecordCreateSampler(device, pCreateInfo, pAllocator, pSampler, result);
    if (result != VK_SUCCESS) {
        static const std::vector<VkResult> error_codes = {VK_ERROR_OUT_OF_HOST_MEMORY,VK_ERROR_OUT_OF_DEVICE_MEMORY};
        static const std::vector<VkResult> success_codes = {};
        ValidateReturnCodes("vkCreateSampler", result, error_codes, success_codes);
    }
}

void BestPractices::PostCallRecordCreateDescriptorSetLayout(
    VkDevice                                    device,
    const VkDescriptorSetLayoutCreateInfo*      pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkDescriptorSetLayout*                      pSetLayout,
    VkResult                                    result) {
    ValidationStateTracker::PostCallRecordCreateDescriptorSetLayout(device, pCreateInfo, pAllocator, pSetLayout, result);
    if (result != VK_SUCCESS) {
        static const std::vector<VkResult> error_codes = {VK_ERROR_OUT_OF_HOST_MEMORY,VK_ERROR_OUT_OF_DEVICE_MEMORY};
        static const std::vector<VkResult> success_codes = {};
        ValidateReturnCodes("vkCreateDescriptorSetLayout", result, error_codes, success_codes);
    }
}

void BestPractices::PostCallRecordCreateDescriptorPool(
    VkDevice                                    device,
    const VkDescriptorPoolCreateInfo*           pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkDescriptorPool*                           pDescriptorPool,
    VkResult                                    result) {
    ValidationStateTracker::PostCallRecordCreateDescriptorPool(device, pCreateInfo, pAllocator, pDescriptorPool, result);
    if (result != VK_SUCCESS) {
        static const std::vector<VkResult> error_codes = {VK_ERROR_OUT_OF_HOST_MEMORY,VK_ERROR_OUT_OF_DEVICE_MEMORY};
        static const std::vector<VkResult> success_codes = {};
        ValidateReturnCodes("vkCreateDescriptorPool", result, error_codes, success_codes);
    }
}

void BestPractices::PostCallRecordAllocateDescriptorSets(
    VkDevice                                    device,
    const VkDescriptorSetAllocateInfo*          pAllocateInfo,
    VkDescriptorSet*                            pDescriptorSets,
    VkResult                                    result,
    void*                                       state_data) {
    ValidationStateTracker::PostCallRecordAllocateDescriptorSets(device, pAllocateInfo, pDescriptorSets, result, state_data);
    ManualPostCallRecordAllocateDescriptorSets(device, pAllocateInfo, pDescriptorSets, result, state_data);
    if (result != VK_SUCCESS) {
        static const std::vector<VkResult> error_codes = {VK_ERROR_OUT_OF_HOST_MEMORY,VK_ERROR_OUT_OF_DEVICE_MEMORY,VK_ERROR_FRAGMENTED_POOL,VK_ERROR_OUT_OF_POOL_MEMORY};
        static const std::vector<VkResult> success_codes = {};
        ValidateReturnCodes("vkAllocateDescriptorSets", result, error_codes, success_codes);
    }
}

void BestPractices::PostCallRecordCreateFramebuffer(
    VkDevice                                    device,
    const VkFramebufferCreateInfo*              pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkFramebuffer*                              pFramebuffer,
    VkResult                                    result) {
    ValidationStateTracker::PostCallRecordCreateFramebuffer(device, pCreateInfo, pAllocator, pFramebuffer, result);
    if (result != VK_SUCCESS) {
        static const std::vector<VkResult> error_codes = {VK_ERROR_OUT_OF_HOST_MEMORY,VK_ERROR_OUT_OF_DEVICE_MEMORY};
        static const std::vector<VkResult> success_codes = {};
        ValidateReturnCodes("vkCreateFramebuffer", result, error_codes, success_codes);
    }
}

void BestPractices::PostCallRecordCreateRenderPass(
    VkDevice                                    device,
    const VkRenderPassCreateInfo*               pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkRenderPass*                               pRenderPass,
    VkResult                                    result) {
    ValidationStateTracker::PostCallRecordCreateRenderPass(device, pCreateInfo, pAllocator, pRenderPass, result);
    if (result != VK_SUCCESS) {
        static const std::vector<VkResult> error_codes = {VK_ERROR_OUT_OF_HOST_MEMORY,VK_ERROR_OUT_OF_DEVICE_MEMORY};
        static const std::vector<VkResult> success_codes = {};
        ValidateReturnCodes("vkCreateRenderPass", result, error_codes, success_codes);
    }
}

void BestPractices::PostCallRecordCreateCommandPool(
    VkDevice                                    device,
    const VkCommandPoolCreateInfo*              pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkCommandPool*                              pCommandPool,
    VkResult                                    result) {
    ValidationStateTracker::PostCallRecordCreateCommandPool(device, pCreateInfo, pAllocator, pCommandPool, result);
    if (result != VK_SUCCESS) {
        static const std::vector<VkResult> error_codes = {VK_ERROR_OUT_OF_HOST_MEMORY,VK_ERROR_OUT_OF_DEVICE_MEMORY};
        static const std::vector<VkResult> success_codes = {};
        ValidateReturnCodes("vkCreateCommandPool", result, error_codes, success_codes);
    }
}

void BestPractices::PostCallRecordResetCommandPool(
    VkDevice                                    device,
    VkCommandPool                               commandPool,
    VkCommandPoolResetFlags                     flags,
    VkResult                                    result) {
    ValidationStateTracker::PostCallRecordResetCommandPool(device, commandPool, flags, result);
    if (result != VK_SUCCESS) {
        static const std::vector<VkResult> error_codes = {VK_ERROR_OUT_OF_DEVICE_MEMORY};
        static const std::vector<VkResult> success_codes = {};
        ValidateReturnCodes("vkResetCommandPool", result, error_codes, success_codes);
    }
}

void BestPractices::PostCallRecordAllocateCommandBuffers(
    VkDevice                                    device,
    const VkCommandBufferAllocateInfo*          pAllocateInfo,
    VkCommandBuffer*                            pCommandBuffers,
    VkResult                                    result) {
    ValidationStateTracker::PostCallRecordAllocateCommandBuffers(device, pAllocateInfo, pCommandBuffers, result);
    if (result != VK_SUCCESS) {
        static const std::vector<VkResult> error_codes = {VK_ERROR_OUT_OF_HOST_MEMORY,VK_ERROR_OUT_OF_DEVICE_MEMORY};
        static const std::vector<VkResult> success_codes = {};
        ValidateReturnCodes("vkAllocateCommandBuffers", result, error_codes, success_codes);
    }
}

void BestPractices::PostCallRecordBeginCommandBuffer(
    VkCommandBuffer                             commandBuffer,
    const VkCommandBufferBeginInfo*             pBeginInfo,
    VkResult                                    result) {
    ValidationStateTracker::PostCallRecordBeginCommandBuffer(commandBuffer, pBeginInfo, result);
    if (result != VK_SUCCESS) {
        static const std::vector<VkResult> error_codes = {VK_ERROR_OUT_OF_HOST_MEMORY,VK_ERROR_OUT_OF_DEVICE_MEMORY};
        static const std::vector<VkResult> success_codes = {};
        ValidateReturnCodes("vkBeginCommandBuffer", result, error_codes, success_codes);
    }
}

void BestPractices::PostCallRecordEndCommandBuffer(
    VkCommandBuffer                             commandBuffer,
    VkResult                                    result) {
    ValidationStateTracker::PostCallRecordEndCommandBuffer(commandBuffer, result);
    if (result != VK_SUCCESS) {
        static const std::vector<VkResult> error_codes = {VK_ERROR_OUT_OF_HOST_MEMORY,VK_ERROR_OUT_OF_DEVICE_MEMORY};
        static const std::vector<VkResult> success_codes = {};
        ValidateReturnCodes("vkEndCommandBuffer", result, error_codes, success_codes);
    }
}

void BestPractices::PostCallRecordResetCommandBuffer(
    VkCommandBuffer                             commandBuffer,
    VkCommandBufferResetFlags                   flags,
    VkResult                                    result) {
    ValidationStateTracker::PostCallRecordResetCommandBuffer(commandBuffer, flags, result);
    if (result != VK_SUCCESS) {
        static const std::vector<VkResult> error_codes = {VK_ERROR_OUT_OF_DEVICE_MEMORY};
        static const std::vector<VkResult> success_codes = {};
        ValidateReturnCodes("vkResetCommandBuffer", result, error_codes, success_codes);
    }
}

// Skipping vkEnumerateInstanceVersion for autogen as it has a manually created custom function or ignored.

void BestPractices::PostCallRecordBindBufferMemory2(
    VkDevice                                    device,
    uint32_t                                    bindInfoCount,
    const VkBindBufferMemoryInfo*               pBindInfos,
    VkResult                                    result) {
    ValidationStateTracker::PostCallRecordBindBufferMemory2(device, bindInfoCount, pBindInfos, result);
    if (result != VK_SUCCESS) {
        static const std::vector<VkResult> error_codes = {VK_ERROR_OUT_OF_HOST_MEMORY,VK_ERROR_OUT_OF_DEVICE_MEMORY};
        static const std::vector<VkResult> success_codes = {};
        ValidateReturnCodes("vkBindBufferMemory2", result, error_codes, success_codes);
    }
}

void BestPractices::PostCallRecordBindImageMemory2(
    VkDevice                                    device,
    uint32_t                                    bindInfoCount,
    const VkBindImageMemoryInfo*                pBindInfos,
    VkResult                                    result) {
    ValidationStateTracker::PostCallRecordBindImageMemory2(device, bindInfoCount, pBindInfos, result);
    if (result != VK_SUCCESS) {
        static const std::vector<VkResult> error_codes = {VK_ERROR_OUT_OF_HOST_MEMORY,VK_ERROR_OUT_OF_DEVICE_MEMORY};
        static const std::vector<VkResult> success_codes = {};
        ValidateReturnCodes("vkBindImageMemory2", result, error_codes, success_codes);
    }
}

void BestPractices::PostCallRecordEnumeratePhysicalDeviceGroups(
    VkInstance                                  instance,
    uint32_t*                                   pPhysicalDeviceGroupCount,
    VkPhysicalDeviceGroupProperties*            pPhysicalDeviceGroupProperties,
    VkResult                                    result) {
    ValidationStateTracker::PostCallRecordEnumeratePhysicalDeviceGroups(instance, pPhysicalDeviceGroupCount, pPhysicalDeviceGroupProperties, result);
    if (result != VK_SUCCESS) {
        static const std::vector<VkResult> error_codes = {VK_ERROR_OUT_OF_HOST_MEMORY,VK_ERROR_OUT_OF_DEVICE_MEMORY,VK_ERROR_INITIALIZATION_FAILED};
        static const std::vector<VkResult> success_codes = {VK_INCOMPLETE};
        ValidateReturnCodes("vkEnumeratePhysicalDeviceGroups", result, error_codes, success_codes);
    }
}

void BestPractices::PostCallRecordGetPhysicalDeviceImageFormatProperties2(
    VkPhysicalDevice                            physicalDevice,
    const VkPhysicalDeviceImageFormatInfo2*     pImageFormatInfo,
    VkImageFormatProperties2*                   pImageFormatProperties,
    VkResult                                    result) {
    ValidationStateTracker::PostCallRecordGetPhysicalDeviceImageFormatProperties2(physicalDevice, pImageFormatInfo, pImageFormatProperties, result);
    if (result != VK_SUCCESS) {
        static const std::vector<VkResult> error_codes = {VK_ERROR_OUT_OF_HOST_MEMORY,VK_ERROR_OUT_OF_DEVICE_MEMORY,VK_ERROR_FORMAT_NOT_SUPPORTED};
        static const std::vector<VkResult> success_codes = {};
        ValidateReturnCodes("vkGetPhysicalDeviceImageFormatProperties2", result, error_codes, success_codes);
    }
}

void BestPractices::PostCallRecordCreateSamplerYcbcrConversion(
    VkDevice                                    device,
    const VkSamplerYcbcrConversionCreateInfo*   pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkSamplerYcbcrConversion*                   pYcbcrConversion,
    VkResult                                    result) {
    ValidationStateTracker::PostCallRecordCreateSamplerYcbcrConversion(device, pCreateInfo, pAllocator, pYcbcrConversion, result);
    if (result != VK_SUCCESS) {
        static const std::vector<VkResult> error_codes = {VK_ERROR_OUT_OF_HOST_MEMORY,VK_ERROR_OUT_OF_DEVICE_MEMORY};
        static const std::vector<VkResult> success_codes = {};
        ValidateReturnCodes("vkCreateSamplerYcbcrConversion", result, error_codes, success_codes);
    }
}

void BestPractices::PostCallRecordCreateRenderPass2(
    VkDevice                                    device,
    const VkRenderPassCreateInfo2*              pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkRenderPass*                               pRenderPass,
    VkResult                                    result) {
    ValidationStateTracker::PostCallRecordCreateRenderPass2(device, pCreateInfo, pAllocator, pRenderPass, result);
    if (result != VK_SUCCESS) {
        static const std::vector<VkResult> error_codes = {VK_ERROR_OUT_OF_HOST_MEMORY,VK_ERROR_OUT_OF_DEVICE_MEMORY};
        static const std::vector<VkResult> success_codes = {};
        ValidateReturnCodes("vkCreateRenderPass2", result, error_codes, success_codes);
    }
}

void BestPractices::PostCallRecordGetSemaphoreCounterValue(
    VkDevice                                    device,
    VkSemaphore                                 semaphore,
    uint64_t*                                   pValue,
    VkResult                                    result) {
    ValidationStateTracker::PostCallRecordGetSemaphoreCounterValue(device, semaphore, pValue, result);
    if (result != VK_SUCCESS) {
        static const std::vector<VkResult> error_codes = {VK_ERROR_OUT_OF_HOST_MEMORY,VK_ERROR_OUT_OF_DEVICE_MEMORY,VK_ERROR_DEVICE_LOST};
        static const std::vector<VkResult> success_codes = {};
        ValidateReturnCodes("vkGetSemaphoreCounterValue", result, error_codes, success_codes);
    }
}

void BestPractices::PostCallRecordWaitSemaphores(
    VkDevice                                    device,
    const VkSemaphoreWaitInfo*                  pWaitInfo,
    uint64_t                                    timeout,
    VkResult                                    result) {
    ValidationStateTracker::PostCallRecordWaitSemaphores(device, pWaitInfo, timeout, result);
    if (result != VK_SUCCESS) {
        static const std::vector<VkResult> error_codes = {VK_ERROR_OUT_OF_HOST_MEMORY,VK_ERROR_OUT_OF_DEVICE_MEMORY,VK_ERROR_DEVICE_LOST};
        static const std::vector<VkResult> success_codes = {VK_TIMEOUT};
        ValidateReturnCodes("vkWaitSemaphores", result, error_codes, success_codes);
    }
}

void BestPractices::PostCallRecordSignalSemaphore(
    VkDevice                                    device,
    const VkSemaphoreSignalInfo*                pSignalInfo,
    VkResult                                    result) {
    ValidationStateTracker::PostCallRecordSignalSemaphore(device, pSignalInfo, result);
    if (result != VK_SUCCESS) {
        static const std::vector<VkResult> error_codes = {VK_ERROR_OUT_OF_HOST_MEMORY,VK_ERROR_OUT_OF_DEVICE_MEMORY};
        static const std::vector<VkResult> success_codes = {};
        ValidateReturnCodes("vkSignalSemaphore", result, error_codes, success_codes);
    }
}

void BestPractices::PostCallRecordGetFaultData(
    VkDevice                                    device,
    VkFaultQueryBehavior                        faultQueryBehavior,
    VkBool32*                                   pUnrecordedFaults,
    uint32_t*                                   pFaultCount,
    VkFaultData*                                pFaults,
    VkResult                                    result) {
    ValidationStateTracker::PostCallRecordGetFaultData(device, faultQueryBehavior, pUnrecordedFaults, pFaultCount, pFaults, result);
    if (result != VK_SUCCESS) {
        static const std::vector<VkResult> error_codes = {VK_ERROR_OUT_OF_HOST_MEMORY,VK_ERROR_OUT_OF_DEVICE_MEMORY};
        static const std::vector<VkResult> success_codes = {VK_INCOMPLETE};
        ValidateReturnCodes("vkGetFaultData", result, error_codes, success_codes);
    }
}

void BestPractices::PostCallRecordGetPhysicalDeviceSurfaceSupportKHR(
    VkPhysicalDevice                            physicalDevice,
    uint32_t                                    queueFamilyIndex,
    VkSurfaceKHR                                surface,
    VkBool32*                                   pSupported,
    VkResult                                    result) {
    ValidationStateTracker::PostCallRecordGetPhysicalDeviceSurfaceSupportKHR(physicalDevice, queueFamilyIndex, surface, pSupported, result);
    if (result != VK_SUCCESS) {
        static const std::vector<VkResult> error_codes = {VK_ERROR_OUT_OF_HOST_MEMORY,VK_ERROR_OUT_OF_DEVICE_MEMORY,VK_ERROR_SURFACE_LOST_KHR};
        static const std::vector<VkResult> success_codes = {};
        ValidateReturnCodes("vkGetPhysicalDeviceSurfaceSupportKHR", result, error_codes, success_codes);
    }
}

void BestPractices::PostCallRecordGetPhysicalDeviceSurfaceCapabilitiesKHR(
    VkPhysicalDevice                            physicalDevice,
    VkSurfaceKHR                                surface,
    VkSurfaceCapabilitiesKHR*                   pSurfaceCapabilities,
    VkResult                                    result) {
    ValidationStateTracker::PostCallRecordGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice, surface, pSurfaceCapabilities, result);
    ManualPostCallRecordGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice, surface, pSurfaceCapabilities, result);
    if (result != VK_SUCCESS) {
        static const std::vector<VkResult> error_codes = {VK_ERROR_OUT_OF_HOST_MEMORY,VK_ERROR_OUT_OF_DEVICE_MEMORY,VK_ERROR_SURFACE_LOST_KHR};
        static const std::vector<VkResult> success_codes = {};
        ValidateReturnCodes("vkGetPhysicalDeviceSurfaceCapabilitiesKHR", result, error_codes, success_codes);
    }
}

void BestPractices::PostCallRecordGetPhysicalDeviceSurfaceFormatsKHR(
    VkPhysicalDevice                            physicalDevice,
    VkSurfaceKHR                                surface,
    uint32_t*                                   pSurfaceFormatCount,
    VkSurfaceFormatKHR*                         pSurfaceFormats,
    VkResult                                    result) {
    ValidationStateTracker::PostCallRecordGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, pSurfaceFormatCount, pSurfaceFormats, result);
    ManualPostCallRecordGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, pSurfaceFormatCount, pSurfaceFormats, result);
    if (result != VK_SUCCESS) {
        static const std::vector<VkResult> error_codes = {VK_ERROR_OUT_OF_HOST_MEMORY,VK_ERROR_OUT_OF_DEVICE_MEMORY,VK_ERROR_SURFACE_LOST_KHR};
        static const std::vector<VkResult> success_codes = {VK_INCOMPLETE};
        ValidateReturnCodes("vkGetPhysicalDeviceSurfaceFormatsKHR", result, error_codes, success_codes);
    }
}

void BestPractices::PostCallRecordGetPhysicalDeviceSurfacePresentModesKHR(
    VkPhysicalDevice                            physicalDevice,
    VkSurfaceKHR                                surface,
    uint32_t*                                   pPresentModeCount,
    VkPresentModeKHR*                           pPresentModes,
    VkResult                                    result) {
    ValidationStateTracker::PostCallRecordGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface, pPresentModeCount, pPresentModes, result);
    ManualPostCallRecordGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface, pPresentModeCount, pPresentModes, result);
    if (result != VK_SUCCESS) {
        static const std::vector<VkResult> error_codes = {VK_ERROR_OUT_OF_HOST_MEMORY,VK_ERROR_OUT_OF_DEVICE_MEMORY,VK_ERROR_SURFACE_LOST_KHR};
        static const std::vector<VkResult> success_codes = {VK_INCOMPLETE};
        ValidateReturnCodes("vkGetPhysicalDeviceSurfacePresentModesKHR", result, error_codes, success_codes);
    }
}

void BestPractices::PostCallRecordCreateSwapchainKHR(
    VkDevice                                    device,
    const VkSwapchainCreateInfoKHR*             pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkSwapchainKHR*                             pSwapchain,
    VkResult                                    result) {
    ValidationStateTracker::PostCallRecordCreateSwapchainKHR(device, pCreateInfo, pAllocator, pSwapchain, result);
    if (result != VK_SUCCESS) {
        static const std::vector<VkResult> error_codes = {VK_ERROR_OUT_OF_HOST_MEMORY,VK_ERROR_OUT_OF_DEVICE_MEMORY,VK_ERROR_DEVICE_LOST,VK_ERROR_SURFACE_LOST_KHR,VK_ERROR_NATIVE_WINDOW_IN_USE_KHR,VK_ERROR_INITIALIZATION_FAILED};
        static const std::vector<VkResult> success_codes = {};
        ValidateReturnCodes("vkCreateSwapchainKHR", result, error_codes, success_codes);
    }
}

void BestPractices::PostCallRecordGetSwapchainImagesKHR(
    VkDevice                                    device,
    VkSwapchainKHR                              swapchain,
    uint32_t*                                   pSwapchainImageCount,
    VkImage*                                    pSwapchainImages,
    VkResult                                    result) {
    ValidationStateTracker::PostCallRecordGetSwapchainImagesKHR(device, swapchain, pSwapchainImageCount, pSwapchainImages, result);
    ManualPostCallRecordGetSwapchainImagesKHR(device, swapchain, pSwapchainImageCount, pSwapchainImages, result);
    if (result != VK_SUCCESS) {
        static const std::vector<VkResult> error_codes = {VK_ERROR_OUT_OF_HOST_MEMORY,VK_ERROR_OUT_OF_DEVICE_MEMORY};
        static const std::vector<VkResult> success_codes = {VK_INCOMPLETE};
        ValidateReturnCodes("vkGetSwapchainImagesKHR", result, error_codes, success_codes);
    }
}

void BestPractices::PostCallRecordAcquireNextImageKHR(
    VkDevice                                    device,
    VkSwapchainKHR                              swapchain,
    uint64_t                                    timeout,
    VkSemaphore                                 semaphore,
    VkFence                                     fence,
    uint32_t*                                   pImageIndex,
    VkResult                                    result) {
    ValidationStateTracker::PostCallRecordAcquireNextImageKHR(device, swapchain, timeout, semaphore, fence, pImageIndex, result);
    if (result != VK_SUCCESS) {
        static const std::vector<VkResult> error_codes = {VK_ERROR_OUT_OF_HOST_MEMORY,VK_ERROR_OUT_OF_DEVICE_MEMORY,VK_ERROR_DEVICE_LOST,VK_ERROR_OUT_OF_DATE_KHR,VK_ERROR_SURFACE_LOST_KHR};
        static const std::vector<VkResult> success_codes = {VK_TIMEOUT,VK_NOT_READY,VK_SUBOPTIMAL_KHR};
        ValidateReturnCodes("vkAcquireNextImageKHR", result, error_codes, success_codes);
    }
}

void BestPractices::PostCallRecordQueuePresentKHR(
    VkQueue                                     queue,
    const VkPresentInfoKHR*                     pPresentInfo,
    VkResult                                    result) {
    ValidationStateTracker::PostCallRecordQueuePresentKHR(queue, pPresentInfo, result);
    ManualPostCallRecordQueuePresentKHR(queue, pPresentInfo, result);
    if (result != VK_SUCCESS) {
        static const std::vector<VkResult> error_codes = {VK_ERROR_OUT_OF_HOST_MEMORY,VK_ERROR_OUT_OF_DEVICE_MEMORY,VK_ERROR_DEVICE_LOST,VK_ERROR_OUT_OF_DATE_KHR,VK_ERROR_SURFACE_LOST_KHR};
        static const std::vector<VkResult> success_codes = {VK_SUBOPTIMAL_KHR};
        ValidateReturnCodes("vkQueuePresentKHR", result, error_codes, success_codes);
    }
}

void BestPractices::PostCallRecordGetDeviceGroupPresentCapabilitiesKHR(
    VkDevice                                    device,
    VkDeviceGroupPresentCapabilitiesKHR*        pDeviceGroupPresentCapabilities,
    VkResult                                    result) {
    ValidationStateTracker::PostCallRecordGetDeviceGroupPresentCapabilitiesKHR(device, pDeviceGroupPresentCapabilities, result);
    if (result != VK_SUCCESS) {
        static const std::vector<VkResult> error_codes = {VK_ERROR_OUT_OF_HOST_MEMORY,VK_ERROR_OUT_OF_DEVICE_MEMORY};
        static const std::vector<VkResult> success_codes = {};
        ValidateReturnCodes("vkGetDeviceGroupPresentCapabilitiesKHR", result, error_codes, success_codes);
    }
}

void BestPractices::PostCallRecordGetDeviceGroupSurfacePresentModesKHR(
    VkDevice                                    device,
    VkSurfaceKHR                                surface,
    VkDeviceGroupPresentModeFlagsKHR*           pModes,
    VkResult                                    result) {
    ValidationStateTracker::PostCallRecordGetDeviceGroupSurfacePresentModesKHR(device, surface, pModes, result);
    if (result != VK_SUCCESS) {
        static const std::vector<VkResult> error_codes = {VK_ERROR_OUT_OF_HOST_MEMORY,VK_ERROR_OUT_OF_DEVICE_MEMORY,VK_ERROR_SURFACE_LOST_KHR};
        static const std::vector<VkResult> success_codes = {};
        ValidateReturnCodes("vkGetDeviceGroupSurfacePresentModesKHR", result, error_codes, success_codes);
    }
}

void BestPractices::PostCallRecordGetPhysicalDevicePresentRectanglesKHR(
    VkPhysicalDevice                            physicalDevice,
    VkSurfaceKHR                                surface,
    uint32_t*                                   pRectCount,
    VkRect2D*                                   pRects,
    VkResult                                    result) {
    ValidationStateTracker::PostCallRecordGetPhysicalDevicePresentRectanglesKHR(physicalDevice, surface, pRectCount, pRects, result);
    if (result != VK_SUCCESS) {
        static const std::vector<VkResult> error_codes = {VK_ERROR_OUT_OF_HOST_MEMORY,VK_ERROR_OUT_OF_DEVICE_MEMORY};
        static const std::vector<VkResult> success_codes = {VK_INCOMPLETE};
        ValidateReturnCodes("vkGetPhysicalDevicePresentRectanglesKHR", result, error_codes, success_codes);
    }
}

void BestPractices::PostCallRecordAcquireNextImage2KHR(
    VkDevice                                    device,
    const VkAcquireNextImageInfoKHR*            pAcquireInfo,
    uint32_t*                                   pImageIndex,
    VkResult                                    result) {
    ValidationStateTracker::PostCallRecordAcquireNextImage2KHR(device, pAcquireInfo, pImageIndex, result);
    if (result != VK_SUCCESS) {
        static const std::vector<VkResult> error_codes = {VK_ERROR_OUT_OF_HOST_MEMORY,VK_ERROR_OUT_OF_DEVICE_MEMORY,VK_ERROR_DEVICE_LOST,VK_ERROR_OUT_OF_DATE_KHR,VK_ERROR_SURFACE_LOST_KHR};
        static const std::vector<VkResult> success_codes = {VK_TIMEOUT,VK_NOT_READY,VK_SUBOPTIMAL_KHR};
        ValidateReturnCodes("vkAcquireNextImage2KHR", result, error_codes, success_codes);
    }
}

void BestPractices::PostCallRecordGetPhysicalDeviceDisplayPropertiesKHR(
    VkPhysicalDevice                            physicalDevice,
    uint32_t*                                   pPropertyCount,
    VkDisplayPropertiesKHR*                     pProperties,
    VkResult                                    result) {
    ValidationStateTracker::PostCallRecordGetPhysicalDeviceDisplayPropertiesKHR(physicalDevice, pPropertyCount, pProperties, result);
    if (result != VK_SUCCESS) {
        static const std::vector<VkResult> error_codes = {VK_ERROR_OUT_OF_HOST_MEMORY,VK_ERROR_OUT_OF_DEVICE_MEMORY};
        static const std::vector<VkResult> success_codes = {VK_INCOMPLETE};
        ValidateReturnCodes("vkGetPhysicalDeviceDisplayPropertiesKHR", result, error_codes, success_codes);
    }
}

void BestPractices::PostCallRecordGetPhysicalDeviceDisplayPlanePropertiesKHR(
    VkPhysicalDevice                            physicalDevice,
    uint32_t*                                   pPropertyCount,
    VkDisplayPlanePropertiesKHR*                pProperties,
    VkResult                                    result) {
    ValidationStateTracker::PostCallRecordGetPhysicalDeviceDisplayPlanePropertiesKHR(physicalDevice, pPropertyCount, pProperties, result);
    ManualPostCallRecordGetPhysicalDeviceDisplayPlanePropertiesKHR(physicalDevice, pPropertyCount, pProperties, result);
    if (result != VK_SUCCESS) {
        static const std::vector<VkResult> error_codes = {VK_ERROR_OUT_OF_HOST_MEMORY,VK_ERROR_OUT_OF_DEVICE_MEMORY};
        static const std::vector<VkResult> success_codes = {VK_INCOMPLETE};
        ValidateReturnCodes("vkGetPhysicalDeviceDisplayPlanePropertiesKHR", result, error_codes, success_codes);
    }
}

void BestPractices::PostCallRecordGetDisplayPlaneSupportedDisplaysKHR(
    VkPhysicalDevice                            physicalDevice,
    uint32_t                                    planeIndex,
    uint32_t*                                   pDisplayCount,
    VkDisplayKHR*                               pDisplays,
    VkResult                                    result) {
    ValidationStateTracker::PostCallRecordGetDisplayPlaneSupportedDisplaysKHR(physicalDevice, planeIndex, pDisplayCount, pDisplays, result);
    if (result != VK_SUCCESS) {
        static const std::vector<VkResult> error_codes = {VK_ERROR_OUT_OF_HOST_MEMORY,VK_ERROR_OUT_OF_DEVICE_MEMORY};
        static const std::vector<VkResult> success_codes = {VK_INCOMPLETE};
        ValidateReturnCodes("vkGetDisplayPlaneSupportedDisplaysKHR", result, error_codes, success_codes);
    }
}

void BestPractices::PostCallRecordGetDisplayModePropertiesKHR(
    VkPhysicalDevice                            physicalDevice,
    VkDisplayKHR                                display,
    uint32_t*                                   pPropertyCount,
    VkDisplayModePropertiesKHR*                 pProperties,
    VkResult                                    result) {
    ValidationStateTracker::PostCallRecordGetDisplayModePropertiesKHR(physicalDevice, display, pPropertyCount, pProperties, result);
    if (result != VK_SUCCESS) {
        static const std::vector<VkResult> error_codes = {VK_ERROR_OUT_OF_HOST_MEMORY,VK_ERROR_OUT_OF_DEVICE_MEMORY};
        static const std::vector<VkResult> success_codes = {VK_INCOMPLETE};
        ValidateReturnCodes("vkGetDisplayModePropertiesKHR", result, error_codes, success_codes);
    }
}

void BestPractices::PostCallRecordCreateDisplayModeKHR(
    VkPhysicalDevice                            physicalDevice,
    VkDisplayKHR                                display,
    const VkDisplayModeCreateInfoKHR*           pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkDisplayModeKHR*                           pMode,
    VkResult                                    result) {
    ValidationStateTracker::PostCallRecordCreateDisplayModeKHR(physicalDevice, display, pCreateInfo, pAllocator, pMode, result);
    if (result != VK_SUCCESS) {
        static const std::vector<VkResult> error_codes = {VK_ERROR_OUT_OF_HOST_MEMORY,VK_ERROR_OUT_OF_DEVICE_MEMORY,VK_ERROR_INITIALIZATION_FAILED};
        static const std::vector<VkResult> success_codes = {};
        ValidateReturnCodes("vkCreateDisplayModeKHR", result, error_codes, success_codes);
    }
}

void BestPractices::PostCallRecordGetDisplayPlaneCapabilitiesKHR(
    VkPhysicalDevice                            physicalDevice,
    VkDisplayModeKHR                            mode,
    uint32_t                                    planeIndex,
    VkDisplayPlaneCapabilitiesKHR*              pCapabilities,
    VkResult                                    result) {
    ValidationStateTracker::PostCallRecordGetDisplayPlaneCapabilitiesKHR(physicalDevice, mode, planeIndex, pCapabilities, result);
    if (result != VK_SUCCESS) {
        static const std::vector<VkResult> error_codes = {VK_ERROR_OUT_OF_HOST_MEMORY,VK_ERROR_OUT_OF_DEVICE_MEMORY};
        static const std::vector<VkResult> success_codes = {};
        ValidateReturnCodes("vkGetDisplayPlaneCapabilitiesKHR", result, error_codes, success_codes);
    }
}

void BestPractices::PostCallRecordCreateDisplayPlaneSurfaceKHR(
    VkInstance                                  instance,
    const VkDisplaySurfaceCreateInfoKHR*        pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkSurfaceKHR*                               pSurface,
    VkResult                                    result) {
    ValidationStateTracker::PostCallRecordCreateDisplayPlaneSurfaceKHR(instance, pCreateInfo, pAllocator, pSurface, result);
    if (result != VK_SUCCESS) {
        static const std::vector<VkResult> error_codes = {VK_ERROR_OUT_OF_HOST_MEMORY,VK_ERROR_OUT_OF_DEVICE_MEMORY};
        static const std::vector<VkResult> success_codes = {};
        ValidateReturnCodes("vkCreateDisplayPlaneSurfaceKHR", result, error_codes, success_codes);
    }
}

void BestPractices::PostCallRecordCreateSharedSwapchainsKHR(
    VkDevice                                    device,
    uint32_t                                    swapchainCount,
    const VkSwapchainCreateInfoKHR*             pCreateInfos,
    const VkAllocationCallbacks*                pAllocator,
    VkSwapchainKHR*                             pSwapchains,
    VkResult                                    result) {
    ValidationStateTracker::PostCallRecordCreateSharedSwapchainsKHR(device, swapchainCount, pCreateInfos, pAllocator, pSwapchains, result);
    if (result != VK_SUCCESS) {
        static const std::vector<VkResult> error_codes = {VK_ERROR_OUT_OF_HOST_MEMORY,VK_ERROR_OUT_OF_DEVICE_MEMORY,VK_ERROR_INCOMPATIBLE_DISPLAY_KHR,VK_ERROR_DEVICE_LOST,VK_ERROR_SURFACE_LOST_KHR};
        static const std::vector<VkResult> success_codes = {};
        ValidateReturnCodes("vkCreateSharedSwapchainsKHR", result, error_codes, success_codes);
    }
}

void BestPractices::PostCallRecordGetMemoryFdKHR(
    VkDevice                                    device,
    const VkMemoryGetFdInfoKHR*                 pGetFdInfo,
    int*                                        pFd,
    VkResult                                    result) {
    ValidationStateTracker::PostCallRecordGetMemoryFdKHR(device, pGetFdInfo, pFd, result);
    if (result != VK_SUCCESS) {
        static const std::vector<VkResult> error_codes = {VK_ERROR_TOO_MANY_OBJECTS,VK_ERROR_OUT_OF_HOST_MEMORY};
        static const std::vector<VkResult> success_codes = {};
        ValidateReturnCodes("vkGetMemoryFdKHR", result, error_codes, success_codes);
    }
}

void BestPractices::PostCallRecordGetMemoryFdPropertiesKHR(
    VkDevice                                    device,
    VkExternalMemoryHandleTypeFlagBits          handleType,
    int                                         fd,
    VkMemoryFdPropertiesKHR*                    pMemoryFdProperties,
    VkResult                                    result) {
    ValidationStateTracker::PostCallRecordGetMemoryFdPropertiesKHR(device, handleType, fd, pMemoryFdProperties, result);
    if (result != VK_SUCCESS) {
        static const std::vector<VkResult> error_codes = {VK_ERROR_OUT_OF_HOST_MEMORY,VK_ERROR_INVALID_EXTERNAL_HANDLE};
        static const std::vector<VkResult> success_codes = {};
        ValidateReturnCodes("vkGetMemoryFdPropertiesKHR", result, error_codes, success_codes);
    }
}

void BestPractices::PostCallRecordImportSemaphoreFdKHR(
    VkDevice                                    device,
    const VkImportSemaphoreFdInfoKHR*           pImportSemaphoreFdInfo,
    VkResult                                    result) {
    ValidationStateTracker::PostCallRecordImportSemaphoreFdKHR(device, pImportSemaphoreFdInfo, result);
    if (result != VK_SUCCESS) {
        static const std::vector<VkResult> error_codes = {VK_ERROR_OUT_OF_HOST_MEMORY,VK_ERROR_INVALID_EXTERNAL_HANDLE};
        static const std::vector<VkResult> success_codes = {};
        ValidateReturnCodes("vkImportSemaphoreFdKHR", result, error_codes, success_codes);
    }
}

void BestPractices::PostCallRecordGetSemaphoreFdKHR(
    VkDevice                                    device,
    const VkSemaphoreGetFdInfoKHR*              pGetFdInfo,
    int*                                        pFd,
    VkResult                                    result) {
    ValidationStateTracker::PostCallRecordGetSemaphoreFdKHR(device, pGetFdInfo, pFd, result);
    if (result != VK_SUCCESS) {
        static const std::vector<VkResult> error_codes = {VK_ERROR_TOO_MANY_OBJECTS,VK_ERROR_OUT_OF_HOST_MEMORY};
        static const std::vector<VkResult> success_codes = {};
        ValidateReturnCodes("vkGetSemaphoreFdKHR", result, error_codes, success_codes);
    }
}

void BestPractices::PostCallRecordGetSwapchainStatusKHR(
    VkDevice                                    device,
    VkSwapchainKHR                              swapchain,
    VkResult                                    result) {
    ValidationStateTracker::PostCallRecordGetSwapchainStatusKHR(device, swapchain, result);
    if (result != VK_SUCCESS) {
        static const std::vector<VkResult> error_codes = {VK_ERROR_OUT_OF_HOST_MEMORY,VK_ERROR_OUT_OF_DEVICE_MEMORY,VK_ERROR_DEVICE_LOST,VK_ERROR_OUT_OF_DATE_KHR,VK_ERROR_SURFACE_LOST_KHR};
        static const std::vector<VkResult> success_codes = {VK_SUBOPTIMAL_KHR};
        ValidateReturnCodes("vkGetSwapchainStatusKHR", result, error_codes, success_codes);
    }
}

void BestPractices::PostCallRecordImportFenceFdKHR(
    VkDevice                                    device,
    const VkImportFenceFdInfoKHR*               pImportFenceFdInfo,
    VkResult                                    result) {
    ValidationStateTracker::PostCallRecordImportFenceFdKHR(device, pImportFenceFdInfo, result);
    if (result != VK_SUCCESS) {
        static const std::vector<VkResult> error_codes = {VK_ERROR_OUT_OF_HOST_MEMORY,VK_ERROR_INVALID_EXTERNAL_HANDLE};
        static const std::vector<VkResult> success_codes = {};
        ValidateReturnCodes("vkImportFenceFdKHR", result, error_codes, success_codes);
    }
}

void BestPractices::PostCallRecordGetFenceFdKHR(
    VkDevice                                    device,
    const VkFenceGetFdInfoKHR*                  pGetFdInfo,
    int*                                        pFd,
    VkResult                                    result) {
    ValidationStateTracker::PostCallRecordGetFenceFdKHR(device, pGetFdInfo, pFd, result);
    if (result != VK_SUCCESS) {
        static const std::vector<VkResult> error_codes = {VK_ERROR_TOO_MANY_OBJECTS,VK_ERROR_OUT_OF_HOST_MEMORY};
        static const std::vector<VkResult> success_codes = {};
        ValidateReturnCodes("vkGetFenceFdKHR", result, error_codes, success_codes);
    }
}

void BestPractices::PostCallRecordEnumeratePhysicalDeviceQueueFamilyPerformanceQueryCountersKHR(
    VkPhysicalDevice                            physicalDevice,
    uint32_t                                    queueFamilyIndex,
    uint32_t*                                   pCounterCount,
    VkPerformanceCounterKHR*                    pCounters,
    VkPerformanceCounterDescriptionKHR*         pCounterDescriptions,
    VkResult                                    result) {
    ValidationStateTracker::PostCallRecordEnumeratePhysicalDeviceQueueFamilyPerformanceQueryCountersKHR(physicalDevice, queueFamilyIndex, pCounterCount, pCounters, pCounterDescriptions, result);
    if (result != VK_SUCCESS) {
        static const std::vector<VkResult> error_codes = {VK_ERROR_OUT_OF_HOST_MEMORY,VK_ERROR_OUT_OF_DEVICE_MEMORY,VK_ERROR_INITIALIZATION_FAILED};
        static const std::vector<VkResult> success_codes = {VK_INCOMPLETE};
        ValidateReturnCodes("vkEnumeratePhysicalDeviceQueueFamilyPerformanceQueryCountersKHR", result, error_codes, success_codes);
    }
}

void BestPractices::PostCallRecordAcquireProfilingLockKHR(
    VkDevice                                    device,
    const VkAcquireProfilingLockInfoKHR*        pInfo,
    VkResult                                    result) {
    ValidationStateTracker::PostCallRecordAcquireProfilingLockKHR(device, pInfo, result);
    if (result != VK_SUCCESS) {
        static const std::vector<VkResult> error_codes = {VK_ERROR_OUT_OF_HOST_MEMORY,VK_TIMEOUT};
        static const std::vector<VkResult> success_codes = {};
        ValidateReturnCodes("vkAcquireProfilingLockKHR", result, error_codes, success_codes);
    }
}

void BestPractices::PostCallRecordGetPhysicalDeviceSurfaceCapabilities2KHR(
    VkPhysicalDevice                            physicalDevice,
    const VkPhysicalDeviceSurfaceInfo2KHR*      pSurfaceInfo,
    VkSurfaceCapabilities2KHR*                  pSurfaceCapabilities,
    VkResult                                    result) {
    ValidationStateTracker::PostCallRecordGetPhysicalDeviceSurfaceCapabilities2KHR(physicalDevice, pSurfaceInfo, pSurfaceCapabilities, result);
    ManualPostCallRecordGetPhysicalDeviceSurfaceCapabilities2KHR(physicalDevice, pSurfaceInfo, pSurfaceCapabilities, result);
    if (result != VK_SUCCESS) {
        static const std::vector<VkResult> error_codes = {VK_ERROR_OUT_OF_HOST_MEMORY,VK_ERROR_OUT_OF_DEVICE_MEMORY,VK_ERROR_SURFACE_LOST_KHR};
        static const std::vector<VkResult> success_codes = {};
        ValidateReturnCodes("vkGetPhysicalDeviceSurfaceCapabilities2KHR", result, error_codes, success_codes);
    }
}

void BestPractices::PostCallRecordGetPhysicalDeviceSurfaceFormats2KHR(
    VkPhysicalDevice                            physicalDevice,
    const VkPhysicalDeviceSurfaceInfo2KHR*      pSurfaceInfo,
    uint32_t*                                   pSurfaceFormatCount,
    VkSurfaceFormat2KHR*                        pSurfaceFormats,
    VkResult                                    result) {
    ValidationStateTracker::PostCallRecordGetPhysicalDeviceSurfaceFormats2KHR(physicalDevice, pSurfaceInfo, pSurfaceFormatCount, pSurfaceFormats, result);
    ManualPostCallRecordGetPhysicalDeviceSurfaceFormats2KHR(physicalDevice, pSurfaceInfo, pSurfaceFormatCount, pSurfaceFormats, result);
    if (result != VK_SUCCESS) {
        static const std::vector<VkResult> error_codes = {VK_ERROR_OUT_OF_HOST_MEMORY,VK_ERROR_OUT_OF_DEVICE_MEMORY,VK_ERROR_SURFACE_LOST_KHR};
        static const std::vector<VkResult> success_codes = {VK_INCOMPLETE};
        ValidateReturnCodes("vkGetPhysicalDeviceSurfaceFormats2KHR", result, error_codes, success_codes);
    }
}

void BestPractices::PostCallRecordGetPhysicalDeviceDisplayProperties2KHR(
    VkPhysicalDevice                            physicalDevice,
    uint32_t*                                   pPropertyCount,
    VkDisplayProperties2KHR*                    pProperties,
    VkResult                                    result) {
    ValidationStateTracker::PostCallRecordGetPhysicalDeviceDisplayProperties2KHR(physicalDevice, pPropertyCount, pProperties, result);
    if (result != VK_SUCCESS) {
        static const std::vector<VkResult> error_codes = {VK_ERROR_OUT_OF_HOST_MEMORY,VK_ERROR_OUT_OF_DEVICE_MEMORY};
        static const std::vector<VkResult> success_codes = {VK_INCOMPLETE};
        ValidateReturnCodes("vkGetPhysicalDeviceDisplayProperties2KHR", result, error_codes, success_codes);
    }
}

void BestPractices::PostCallRecordGetPhysicalDeviceDisplayPlaneProperties2KHR(
    VkPhysicalDevice                            physicalDevice,
    uint32_t*                                   pPropertyCount,
    VkDisplayPlaneProperties2KHR*               pProperties,
    VkResult                                    result) {
    ValidationStateTracker::PostCallRecordGetPhysicalDeviceDisplayPlaneProperties2KHR(physicalDevice, pPropertyCount, pProperties, result);
    if (result != VK_SUCCESS) {
        static const std::vector<VkResult> error_codes = {VK_ERROR_OUT_OF_HOST_MEMORY,VK_ERROR_OUT_OF_DEVICE_MEMORY};
        static const std::vector<VkResult> success_codes = {VK_INCOMPLETE};
        ValidateReturnCodes("vkGetPhysicalDeviceDisplayPlaneProperties2KHR", result, error_codes, success_codes);
    }
}

void BestPractices::PostCallRecordGetDisplayModeProperties2KHR(
    VkPhysicalDevice                            physicalDevice,
    VkDisplayKHR                                display,
    uint32_t*                                   pPropertyCount,
    VkDisplayModeProperties2KHR*                pProperties,
    VkResult                                    result) {
    ValidationStateTracker::PostCallRecordGetDisplayModeProperties2KHR(physicalDevice, display, pPropertyCount, pProperties, result);
    if (result != VK_SUCCESS) {
        static const std::vector<VkResult> error_codes = {VK_ERROR_OUT_OF_HOST_MEMORY,VK_ERROR_OUT_OF_DEVICE_MEMORY};
        static const std::vector<VkResult> success_codes = {VK_INCOMPLETE};
        ValidateReturnCodes("vkGetDisplayModeProperties2KHR", result, error_codes, success_codes);
    }
}

void BestPractices::PostCallRecordGetDisplayPlaneCapabilities2KHR(
    VkPhysicalDevice                            physicalDevice,
    const VkDisplayPlaneInfo2KHR*               pDisplayPlaneInfo,
    VkDisplayPlaneCapabilities2KHR*             pCapabilities,
    VkResult                                    result) {
    ValidationStateTracker::PostCallRecordGetDisplayPlaneCapabilities2KHR(physicalDevice, pDisplayPlaneInfo, pCapabilities, result);
    if (result != VK_SUCCESS) {
        static const std::vector<VkResult> error_codes = {VK_ERROR_OUT_OF_HOST_MEMORY,VK_ERROR_OUT_OF_DEVICE_MEMORY};
        static const std::vector<VkResult> success_codes = {};
        ValidateReturnCodes("vkGetDisplayPlaneCapabilities2KHR", result, error_codes, success_codes);
    }
}

void BestPractices::PostCallRecordGetPhysicalDeviceFragmentShadingRatesKHR(
    VkPhysicalDevice                            physicalDevice,
    uint32_t*                                   pFragmentShadingRateCount,
    VkPhysicalDeviceFragmentShadingRateKHR*     pFragmentShadingRates,
    VkResult                                    result) {
    ValidationStateTracker::PostCallRecordGetPhysicalDeviceFragmentShadingRatesKHR(physicalDevice, pFragmentShadingRateCount, pFragmentShadingRates, result);
    if (result != VK_SUCCESS) {
        static const std::vector<VkResult> error_codes = {VK_ERROR_OUT_OF_HOST_MEMORY};
        static const std::vector<VkResult> success_codes = {VK_INCOMPLETE};
        ValidateReturnCodes("vkGetPhysicalDeviceFragmentShadingRatesKHR", result, error_codes, success_codes);
    }
}

void BestPractices::PostCallRecordGetPhysicalDeviceRefreshableObjectTypesKHR(
    VkPhysicalDevice                            physicalDevice,
    uint32_t*                                   pRefreshableObjectTypeCount,
    VkObjectType*                               pRefreshableObjectTypes,
    VkResult                                    result) {
    ValidationStateTracker::PostCallRecordGetPhysicalDeviceRefreshableObjectTypesKHR(physicalDevice, pRefreshableObjectTypeCount, pRefreshableObjectTypes, result);
    if (result != VK_SUCCESS) {
        static const std::vector<VkResult> error_codes = {};
        static const std::vector<VkResult> success_codes = {VK_INCOMPLETE};
        ValidateReturnCodes("vkGetPhysicalDeviceRefreshableObjectTypesKHR", result, error_codes, success_codes);
    }
}

void BestPractices::PostCallRecordQueueSubmit2KHR(
    VkQueue                                     queue,
    uint32_t                                    submitCount,
    const VkSubmitInfo2KHR*                     pSubmits,
    VkFence                                     fence,
    VkResult                                    result) {
    ValidationStateTracker::PostCallRecordQueueSubmit2KHR(queue, submitCount, pSubmits, fence, result);
    if (result != VK_SUCCESS) {
        static const std::vector<VkResult> error_codes = {VK_ERROR_OUT_OF_HOST_MEMORY,VK_ERROR_OUT_OF_DEVICE_MEMORY,VK_ERROR_DEVICE_LOST};
        static const std::vector<VkResult> success_codes = {};
        ValidateReturnCodes("vkQueueSubmit2KHR", result, error_codes, success_codes);
    }
}

void BestPractices::PostCallRecordGetPhysicalDeviceSurfaceCapabilities2EXT(
    VkPhysicalDevice                            physicalDevice,
    VkSurfaceKHR                                surface,
    VkSurfaceCapabilities2EXT*                  pSurfaceCapabilities,
    VkResult                                    result) {
    ValidationStateTracker::PostCallRecordGetPhysicalDeviceSurfaceCapabilities2EXT(physicalDevice, surface, pSurfaceCapabilities, result);
    ManualPostCallRecordGetPhysicalDeviceSurfaceCapabilities2EXT(physicalDevice, surface, pSurfaceCapabilities, result);
    if (result != VK_SUCCESS) {
        static const std::vector<VkResult> error_codes = {VK_ERROR_OUT_OF_HOST_MEMORY,VK_ERROR_OUT_OF_DEVICE_MEMORY,VK_ERROR_SURFACE_LOST_KHR};
        static const std::vector<VkResult> success_codes = {};
        ValidateReturnCodes("vkGetPhysicalDeviceSurfaceCapabilities2EXT", result, error_codes, success_codes);
    }
}

void BestPractices::PostCallRecordDisplayPowerControlEXT(
    VkDevice                                    device,
    VkDisplayKHR                                display,
    const VkDisplayPowerInfoEXT*                pDisplayPowerInfo,
    VkResult                                    result) {
    ValidationStateTracker::PostCallRecordDisplayPowerControlEXT(device, display, pDisplayPowerInfo, result);
    if (result != VK_SUCCESS) {
        static const std::vector<VkResult> error_codes = {VK_ERROR_OUT_OF_HOST_MEMORY};
        static const std::vector<VkResult> success_codes = {};
        ValidateReturnCodes("vkDisplayPowerControlEXT", result, error_codes, success_codes);
    }
}

void BestPractices::PostCallRecordRegisterDeviceEventEXT(
    VkDevice                                    device,
    const VkDeviceEventInfoEXT*                 pDeviceEventInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkFence*                                    pFence,
    VkResult                                    result) {
    ValidationStateTracker::PostCallRecordRegisterDeviceEventEXT(device, pDeviceEventInfo, pAllocator, pFence, result);
    if (result != VK_SUCCESS) {
        static const std::vector<VkResult> error_codes = {VK_ERROR_OUT_OF_HOST_MEMORY};
        static const std::vector<VkResult> success_codes = {};
        ValidateReturnCodes("vkRegisterDeviceEventEXT", result, error_codes, success_codes);
    }
}

void BestPractices::PostCallRecordRegisterDisplayEventEXT(
    VkDevice                                    device,
    VkDisplayKHR                                display,
    const VkDisplayEventInfoEXT*                pDisplayEventInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkFence*                                    pFence,
    VkResult                                    result) {
    ValidationStateTracker::PostCallRecordRegisterDisplayEventEXT(device, display, pDisplayEventInfo, pAllocator, pFence, result);
    if (result != VK_SUCCESS) {
        static const std::vector<VkResult> error_codes = {VK_ERROR_OUT_OF_HOST_MEMORY};
        static const std::vector<VkResult> success_codes = {};
        ValidateReturnCodes("vkRegisterDisplayEventEXT", result, error_codes, success_codes);
    }
}

void BestPractices::PostCallRecordGetSwapchainCounterEXT(
    VkDevice                                    device,
    VkSwapchainKHR                              swapchain,
    VkSurfaceCounterFlagBitsEXT                 counter,
    uint64_t*                                   pCounterValue,
    VkResult                                    result) {
    ValidationStateTracker::PostCallRecordGetSwapchainCounterEXT(device, swapchain, counter, pCounterValue, result);
    if (result != VK_SUCCESS) {
        static const std::vector<VkResult> error_codes = {VK_ERROR_OUT_OF_HOST_MEMORY,VK_ERROR_DEVICE_LOST,VK_ERROR_OUT_OF_DATE_KHR};
        static const std::vector<VkResult> success_codes = {};
        ValidateReturnCodes("vkGetSwapchainCounterEXT", result, error_codes, success_codes);
    }
}

void BestPractices::PostCallRecordSetDebugUtilsObjectNameEXT(
    VkDevice                                    device,
    const VkDebugUtilsObjectNameInfoEXT*        pNameInfo,
    VkResult                                    result) {
    ValidationStateTracker::PostCallRecordSetDebugUtilsObjectNameEXT(device, pNameInfo, result);
    if (result != VK_SUCCESS) {
        static const std::vector<VkResult> error_codes = {VK_ERROR_OUT_OF_HOST_MEMORY,VK_ERROR_OUT_OF_DEVICE_MEMORY};
        static const std::vector<VkResult> success_codes = {};
        ValidateReturnCodes("vkSetDebugUtilsObjectNameEXT", result, error_codes, success_codes);
    }
}

void BestPractices::PostCallRecordSetDebugUtilsObjectTagEXT(
    VkDevice                                    device,
    const VkDebugUtilsObjectTagInfoEXT*         pTagInfo,
    VkResult                                    result) {
    ValidationStateTracker::PostCallRecordSetDebugUtilsObjectTagEXT(device, pTagInfo, result);
    if (result != VK_SUCCESS) {
        static const std::vector<VkResult> error_codes = {VK_ERROR_OUT_OF_HOST_MEMORY,VK_ERROR_OUT_OF_DEVICE_MEMORY};
        static const std::vector<VkResult> success_codes = {};
        ValidateReturnCodes("vkSetDebugUtilsObjectTagEXT", result, error_codes, success_codes);
    }
}

void BestPractices::PostCallRecordCreateDebugUtilsMessengerEXT(
    VkInstance                                  instance,
    const VkDebugUtilsMessengerCreateInfoEXT*   pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkDebugUtilsMessengerEXT*                   pMessenger,
    VkResult                                    result) {
    ValidationStateTracker::PostCallRecordCreateDebugUtilsMessengerEXT(instance, pCreateInfo, pAllocator, pMessenger, result);
    if (result != VK_SUCCESS) {
        static const std::vector<VkResult> error_codes = {VK_ERROR_OUT_OF_HOST_MEMORY};
        static const std::vector<VkResult> success_codes = {};
        ValidateReturnCodes("vkCreateDebugUtilsMessengerEXT", result, error_codes, success_codes);
    }
}

void BestPractices::PostCallRecordGetImageDrmFormatModifierPropertiesEXT(
    VkDevice                                    device,
    VkImage                                     image,
    VkImageDrmFormatModifierPropertiesEXT*      pProperties,
    VkResult                                    result) {
    ValidationStateTracker::PostCallRecordGetImageDrmFormatModifierPropertiesEXT(device, image, pProperties, result);
    if (result != VK_SUCCESS) {
        static const std::vector<VkResult> error_codes = {VK_ERROR_OUT_OF_HOST_MEMORY};
        static const std::vector<VkResult> success_codes = {};
        ValidateReturnCodes("vkGetImageDrmFormatModifierPropertiesEXT", result, error_codes, success_codes);
    }
}

void BestPractices::PostCallRecordGetMemoryHostPointerPropertiesEXT(
    VkDevice                                    device,
    VkExternalMemoryHandleTypeFlagBits          handleType,
    const void*                                 pHostPointer,
    VkMemoryHostPointerPropertiesEXT*           pMemoryHostPointerProperties,
    VkResult                                    result) {
    ValidationStateTracker::PostCallRecordGetMemoryHostPointerPropertiesEXT(device, handleType, pHostPointer, pMemoryHostPointerProperties, result);
    if (result != VK_SUCCESS) {
        static const std::vector<VkResult> error_codes = {VK_ERROR_OUT_OF_HOST_MEMORY,VK_ERROR_INVALID_EXTERNAL_HANDLE};
        static const std::vector<VkResult> success_codes = {};
        ValidateReturnCodes("vkGetMemoryHostPointerPropertiesEXT", result, error_codes, success_codes);
    }
}

void BestPractices::PostCallRecordGetPhysicalDeviceCalibrateableTimeDomainsEXT(
    VkPhysicalDevice                            physicalDevice,
    uint32_t*                                   pTimeDomainCount,
    VkTimeDomainEXT*                            pTimeDomains,
    VkResult                                    result) {
    ValidationStateTracker::PostCallRecordGetPhysicalDeviceCalibrateableTimeDomainsEXT(physicalDevice, pTimeDomainCount, pTimeDomains, result);
    if (result != VK_SUCCESS) {
        static const std::vector<VkResult> error_codes = {VK_ERROR_OUT_OF_HOST_MEMORY,VK_ERROR_OUT_OF_DEVICE_MEMORY};
        static const std::vector<VkResult> success_codes = {VK_INCOMPLETE};
        ValidateReturnCodes("vkGetPhysicalDeviceCalibrateableTimeDomainsEXT", result, error_codes, success_codes);
    }
}

void BestPractices::PostCallRecordGetCalibratedTimestampsEXT(
    VkDevice                                    device,
    uint32_t                                    timestampCount,
    const VkCalibratedTimestampInfoEXT*         pTimestampInfos,
    uint64_t*                                   pTimestamps,
    uint64_t*                                   pMaxDeviation,
    VkResult                                    result) {
    ValidationStateTracker::PostCallRecordGetCalibratedTimestampsEXT(device, timestampCount, pTimestampInfos, pTimestamps, pMaxDeviation, result);
    if (result != VK_SUCCESS) {
        static const std::vector<VkResult> error_codes = {VK_ERROR_OUT_OF_HOST_MEMORY,VK_ERROR_OUT_OF_DEVICE_MEMORY};
        static const std::vector<VkResult> success_codes = {};
        ValidateReturnCodes("vkGetCalibratedTimestampsEXT", result, error_codes, success_codes);
    }
}

void BestPractices::PostCallRecordCreateHeadlessSurfaceEXT(
    VkInstance                                  instance,
    const VkHeadlessSurfaceCreateInfoEXT*       pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkSurfaceKHR*                               pSurface,
    VkResult                                    result) {
    ValidationStateTracker::PostCallRecordCreateHeadlessSurfaceEXT(instance, pCreateInfo, pAllocator, pSurface, result);
    if (result != VK_SUCCESS) {
        static const std::vector<VkResult> error_codes = {VK_ERROR_OUT_OF_HOST_MEMORY,VK_ERROR_OUT_OF_DEVICE_MEMORY};
        static const std::vector<VkResult> success_codes = {};
        ValidateReturnCodes("vkCreateHeadlessSurfaceEXT", result, error_codes, success_codes);
    }
}

#ifdef VK_USE_PLATFORM_SCI

void BestPractices::PostCallRecordGetFenceSciSyncFenceNV(
    VkDevice                                    device,
    const VkFenceGetSciSyncInfoNV*              pGetSciSyncHandleInfo,
    void*                                       pHandle,
    VkResult                                    result) {
    ValidationStateTracker::PostCallRecordGetFenceSciSyncFenceNV(device, pGetSciSyncHandleInfo, pHandle, result);
    if (result != VK_SUCCESS) {
        static const std::vector<VkResult> error_codes = {VK_ERROR_INVALID_EXTERNAL_HANDLE,VK_ERROR_NOT_PERMITTED_EXT};
        static const std::vector<VkResult> success_codes = {};
        ValidateReturnCodes("vkGetFenceSciSyncFenceNV", result, error_codes, success_codes);
    }
}

#endif // VK_USE_PLATFORM_SCI

#ifdef VK_USE_PLATFORM_SCI

void BestPractices::PostCallRecordGetFenceSciSyncObjNV(
    VkDevice                                    device,
    const VkFenceGetSciSyncInfoNV*              pGetSciSyncHandleInfo,
    void*                                       pHandle,
    VkResult                                    result) {
    ValidationStateTracker::PostCallRecordGetFenceSciSyncObjNV(device, pGetSciSyncHandleInfo, pHandle, result);
    if (result != VK_SUCCESS) {
        static const std::vector<VkResult> error_codes = {VK_ERROR_INVALID_EXTERNAL_HANDLE,VK_ERROR_NOT_PERMITTED_EXT};
        static const std::vector<VkResult> success_codes = {};
        ValidateReturnCodes("vkGetFenceSciSyncObjNV", result, error_codes, success_codes);
    }
}

#endif // VK_USE_PLATFORM_SCI

#ifdef VK_USE_PLATFORM_SCI

void BestPractices::PostCallRecordImportFenceSciSyncFenceNV(
    VkDevice                                    device,
    const VkImportFenceSciSyncInfoNV*           pImportFenceSciSyncInfo,
    VkResult                                    result) {
    ValidationStateTracker::PostCallRecordImportFenceSciSyncFenceNV(device, pImportFenceSciSyncInfo, result);
    if (result != VK_SUCCESS) {
        static const std::vector<VkResult> error_codes = {VK_ERROR_INVALID_EXTERNAL_HANDLE,VK_ERROR_NOT_PERMITTED_EXT};
        static const std::vector<VkResult> success_codes = {};
        ValidateReturnCodes("vkImportFenceSciSyncFenceNV", result, error_codes, success_codes);
    }
}

#endif // VK_USE_PLATFORM_SCI

#ifdef VK_USE_PLATFORM_SCI

void BestPractices::PostCallRecordImportFenceSciSyncObjNV(
    VkDevice                                    device,
    const VkImportFenceSciSyncInfoNV*           pImportFenceSciSyncInfo,
    VkResult                                    result) {
    ValidationStateTracker::PostCallRecordImportFenceSciSyncObjNV(device, pImportFenceSciSyncInfo, result);
    if (result != VK_SUCCESS) {
        static const std::vector<VkResult> error_codes = {VK_ERROR_INVALID_EXTERNAL_HANDLE,VK_ERROR_NOT_PERMITTED_EXT};
        static const std::vector<VkResult> success_codes = {};
        ValidateReturnCodes("vkImportFenceSciSyncObjNV", result, error_codes, success_codes);
    }
}

#endif // VK_USE_PLATFORM_SCI

#ifdef VK_USE_PLATFORM_SCI

void BestPractices::PostCallRecordGetPhysicalDeviceSciSyncAttributesNV(
    VkPhysicalDevice                            physicalDevice,
    const VkSciSyncAttributesInfoNV*            pSciSyncAttributesInfo,
    NvSciSyncAttrList                           pAttributes,
    VkResult                                    result) {
    ValidationStateTracker::PostCallRecordGetPhysicalDeviceSciSyncAttributesNV(physicalDevice, pSciSyncAttributesInfo, pAttributes, result);
    if (result != VK_SUCCESS) {
        static const std::vector<VkResult> error_codes = {VK_ERROR_INITIALIZATION_FAILED};
        static const std::vector<VkResult> success_codes = {};
        ValidateReturnCodes("vkGetPhysicalDeviceSciSyncAttributesNV", result, error_codes, success_codes);
    }
}

#endif // VK_USE_PLATFORM_SCI

#ifdef VK_USE_PLATFORM_SCI

void BestPractices::PostCallRecordGetSemaphoreSciSyncObjNV(
    VkDevice                                    device,
    const VkSemaphoreGetSciSyncInfoNV*          pGetSciSyncInfo,
    void*                                       pHandle,
    VkResult                                    result) {
    ValidationStateTracker::PostCallRecordGetSemaphoreSciSyncObjNV(device, pGetSciSyncInfo, pHandle, result);
    if (result != VK_SUCCESS) {
        static const std::vector<VkResult> error_codes = {VK_ERROR_INVALID_EXTERNAL_HANDLE,VK_ERROR_NOT_PERMITTED_EXT};
        static const std::vector<VkResult> success_codes = {};
        ValidateReturnCodes("vkGetSemaphoreSciSyncObjNV", result, error_codes, success_codes);
    }
}

#endif // VK_USE_PLATFORM_SCI

#ifdef VK_USE_PLATFORM_SCI

void BestPractices::PostCallRecordImportSemaphoreSciSyncObjNV(
    VkDevice                                    device,
    const VkImportSemaphoreSciSyncInfoNV*       pImportSemaphoreSciSyncInfo,
    VkResult                                    result) {
    ValidationStateTracker::PostCallRecordImportSemaphoreSciSyncObjNV(device, pImportSemaphoreSciSyncInfo, result);
    if (result != VK_SUCCESS) {
        static const std::vector<VkResult> error_codes = {VK_ERROR_INVALID_EXTERNAL_HANDLE,VK_ERROR_NOT_PERMITTED_EXT,VK_ERROR_OUT_OF_HOST_MEMORY};
        static const std::vector<VkResult> success_codes = {};
        ValidateReturnCodes("vkImportSemaphoreSciSyncObjNV", result, error_codes, success_codes);
    }
}

#endif // VK_USE_PLATFORM_SCI

#ifdef VK_USE_PLATFORM_SCI

void BestPractices::PostCallRecordGetMemorySciBufNV(
    VkDevice                                    device,
    const VkMemoryGetSciBufInfoNV*              pGetSciBufInfo,
    NvSciBufObj*                                pHandle,
    VkResult                                    result) {
    ValidationStateTracker::PostCallRecordGetMemorySciBufNV(device, pGetSciBufInfo, pHandle, result);
    if (result != VK_SUCCESS) {
        static const std::vector<VkResult> error_codes = {VK_ERROR_INITIALIZATION_FAILED};
        static const std::vector<VkResult> success_codes = {};
        ValidateReturnCodes("vkGetMemorySciBufNV", result, error_codes, success_codes);
    }
}

#endif // VK_USE_PLATFORM_SCI

#ifdef VK_USE_PLATFORM_SCI

void BestPractices::PostCallRecordGetPhysicalDeviceExternalMemorySciBufPropertiesNV(
    VkPhysicalDevice                            physicalDevice,
    VkExternalMemoryHandleTypeFlagBits          handleType,
    NvSciBufObj                                 handle,
    VkMemorySciBufPropertiesNV*                 pMemorySciBufProperties,
    VkResult                                    result) {
    ValidationStateTracker::PostCallRecordGetPhysicalDeviceExternalMemorySciBufPropertiesNV(physicalDevice, handleType, handle, pMemorySciBufProperties, result);
    if (result != VK_SUCCESS) {
        static const std::vector<VkResult> error_codes = {VK_ERROR_INITIALIZATION_FAILED,VK_ERROR_INVALID_EXTERNAL_HANDLE};
        static const std::vector<VkResult> success_codes = {};
        ValidateReturnCodes("vkGetPhysicalDeviceExternalMemorySciBufPropertiesNV", result, error_codes, success_codes);
    }
}

#endif // VK_USE_PLATFORM_SCI

#ifdef VK_USE_PLATFORM_SCI

void BestPractices::PostCallRecordGetPhysicalDeviceSciBufAttributesNV(
    VkPhysicalDevice                            physicalDevice,
    NvSciBufAttrList                            pAttributes,
    VkResult                                    result) {
    ValidationStateTracker::PostCallRecordGetPhysicalDeviceSciBufAttributesNV(physicalDevice, pAttributes, result);
    if (result != VK_SUCCESS) {
        static const std::vector<VkResult> error_codes = {VK_ERROR_OUT_OF_HOST_MEMORY,VK_ERROR_INITIALIZATION_FAILED};
        static const std::vector<VkResult> success_codes = {};
        ValidateReturnCodes("vkGetPhysicalDeviceSciBufAttributesNV", result, error_codes, success_codes);
    }
}

#endif // VK_USE_PLATFORM_SCI





