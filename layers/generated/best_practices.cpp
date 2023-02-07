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


#include "chassis.h"
#include "best_practices/best_practices_validation.h"
void BestPractices::PostCallRecordCreateInstance(
    const VkInstanceCreateInfo*                 pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkInstance*                                 pInstance,
    VkResult                                    result) {
    ValidationStateTracker::PostCallRecordCreateInstance(pCreateInfo, pAllocator, pInstance, result);
    if (result != VK_SUCCESS) {
        constexpr std::array error_codes = {VK_ERROR_OUT_OF_HOST_MEMORY,VK_ERROR_OUT_OF_DEVICE_MEMORY,VK_ERROR_INITIALIZATION_FAILED,VK_ERROR_LAYER_NOT_PRESENT,VK_ERROR_EXTENSION_NOT_PRESENT,VK_ERROR_INCOMPATIBLE_DRIVER};
        ValidateReturnCodes("vkCreateInstance", result, error_codes, {});
    }
}

void BestPractices::PostCallRecordEnumeratePhysicalDevices(
    VkInstance                                  instance,
    uint32_t*                                   pPhysicalDeviceCount,
    VkPhysicalDevice*                           pPhysicalDevices,
    VkResult                                    result) {
    ValidationStateTracker::PostCallRecordEnumeratePhysicalDevices(instance, pPhysicalDeviceCount, pPhysicalDevices, result);
    if (result != VK_SUCCESS) {
        constexpr std::array error_codes = {VK_ERROR_OUT_OF_HOST_MEMORY,VK_ERROR_OUT_OF_DEVICE_MEMORY,VK_ERROR_INITIALIZATION_FAILED};
        constexpr std::array success_codes = {VK_INCOMPLETE};
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
        constexpr std::array error_codes = {VK_ERROR_OUT_OF_HOST_MEMORY,VK_ERROR_OUT_OF_DEVICE_MEMORY,VK_ERROR_FORMAT_NOT_SUPPORTED};
        ValidateReturnCodes("vkGetPhysicalDeviceImageFormatProperties", result, error_codes, {});
    }
}

void BestPractices::PostCallRecordCreateDevice(
    VkPhysicalDevice                            physicalDevice,
    const VkDeviceCreateInfo*                   pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkDevice*                                   pDevice,
    VkResult                                    result) {
    ValidationStateTracker::PostCallRecordCreateDevice(physicalDevice, pCreateInfo, pAllocator, pDevice, result);
    if (result != VK_SUCCESS) {
        constexpr std::array error_codes = {VK_ERROR_OUT_OF_HOST_MEMORY,VK_ERROR_OUT_OF_DEVICE_MEMORY,VK_ERROR_INITIALIZATION_FAILED,VK_ERROR_EXTENSION_NOT_PRESENT,VK_ERROR_FEATURE_NOT_PRESENT,VK_ERROR_TOO_MANY_OBJECTS,VK_ERROR_DEVICE_LOST};
        ValidateReturnCodes("vkCreateDevice", result, error_codes, {});
    }
}

void BestPractices::PostCallRecordEnumerateInstanceExtensionProperties(
    const char*                                 pLayerName,
    uint32_t*                                   pPropertyCount,
    VkExtensionProperties*                      pProperties,
    VkResult                                    result) {
    ValidationStateTracker::PostCallRecordEnumerateInstanceExtensionProperties(pLayerName, pPropertyCount, pProperties, result);
    if (result != VK_SUCCESS) {
        constexpr std::array error_codes = {VK_ERROR_OUT_OF_HOST_MEMORY,VK_ERROR_OUT_OF_DEVICE_MEMORY,VK_ERROR_LAYER_NOT_PRESENT};
        constexpr std::array success_codes = {VK_INCOMPLETE};
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
        constexpr std::array error_codes = {VK_ERROR_OUT_OF_HOST_MEMORY,VK_ERROR_OUT_OF_DEVICE_MEMORY,VK_ERROR_LAYER_NOT_PRESENT};
        constexpr std::array success_codes = {VK_INCOMPLETE};
        ValidateReturnCodes("vkEnumerateDeviceExtensionProperties", result, error_codes, success_codes);
    }
}

void BestPractices::PostCallRecordEnumerateInstanceLayerProperties(
    uint32_t*                                   pPropertyCount,
    VkLayerProperties*                          pProperties,
    VkResult                                    result) {
    ValidationStateTracker::PostCallRecordEnumerateInstanceLayerProperties(pPropertyCount, pProperties, result);
    if (result != VK_SUCCESS) {
        constexpr std::array error_codes = {VK_ERROR_OUT_OF_HOST_MEMORY,VK_ERROR_OUT_OF_DEVICE_MEMORY};
        constexpr std::array success_codes = {VK_INCOMPLETE};
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
        constexpr std::array error_codes = {VK_ERROR_OUT_OF_HOST_MEMORY,VK_ERROR_OUT_OF_DEVICE_MEMORY};
        constexpr std::array success_codes = {VK_INCOMPLETE};
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
        constexpr std::array error_codes = {VK_ERROR_OUT_OF_HOST_MEMORY,VK_ERROR_OUT_OF_DEVICE_MEMORY,VK_ERROR_DEVICE_LOST};
        ValidateReturnCodes("vkQueueSubmit", result, error_codes, {});
    }
}

void BestPractices::PostCallRecordQueueWaitIdle(
    VkQueue                                     queue,
    VkResult                                    result) {
    ValidationStateTracker::PostCallRecordQueueWaitIdle(queue, result);
    if (result != VK_SUCCESS) {
        constexpr std::array error_codes = {VK_ERROR_OUT_OF_HOST_MEMORY,VK_ERROR_OUT_OF_DEVICE_MEMORY,VK_ERROR_DEVICE_LOST};
        ValidateReturnCodes("vkQueueWaitIdle", result, error_codes, {});
    }
}

void BestPractices::PostCallRecordDeviceWaitIdle(
    VkDevice                                    device,
    VkResult                                    result) {
    ValidationStateTracker::PostCallRecordDeviceWaitIdle(device, result);
    if (result != VK_SUCCESS) {
        constexpr std::array error_codes = {VK_ERROR_OUT_OF_HOST_MEMORY,VK_ERROR_OUT_OF_DEVICE_MEMORY,VK_ERROR_DEVICE_LOST};
        ValidateReturnCodes("vkDeviceWaitIdle", result, error_codes, {});
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
        constexpr std::array error_codes = {VK_ERROR_OUT_OF_HOST_MEMORY,VK_ERROR_OUT_OF_DEVICE_MEMORY,VK_ERROR_INVALID_EXTERNAL_HANDLE,VK_ERROR_INVALID_OPAQUE_CAPTURE_ADDRESS_KHR};
        ValidateReturnCodes("vkAllocateMemory", result, error_codes, {});
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
        constexpr std::array error_codes = {VK_ERROR_OUT_OF_HOST_MEMORY,VK_ERROR_OUT_OF_DEVICE_MEMORY,VK_ERROR_MEMORY_MAP_FAILED};
        ValidateReturnCodes("vkMapMemory", result, error_codes, {});
    }
}

void BestPractices::PostCallRecordFlushMappedMemoryRanges(
    VkDevice                                    device,
    uint32_t                                    memoryRangeCount,
    const VkMappedMemoryRange*                  pMemoryRanges,
    VkResult                                    result) {
    ValidationStateTracker::PostCallRecordFlushMappedMemoryRanges(device, memoryRangeCount, pMemoryRanges, result);
    if (result != VK_SUCCESS) {
        constexpr std::array error_codes = {VK_ERROR_OUT_OF_HOST_MEMORY,VK_ERROR_OUT_OF_DEVICE_MEMORY};
        ValidateReturnCodes("vkFlushMappedMemoryRanges", result, error_codes, {});
    }
}

void BestPractices::PostCallRecordInvalidateMappedMemoryRanges(
    VkDevice                                    device,
    uint32_t                                    memoryRangeCount,
    const VkMappedMemoryRange*                  pMemoryRanges,
    VkResult                                    result) {
    ValidationStateTracker::PostCallRecordInvalidateMappedMemoryRanges(device, memoryRangeCount, pMemoryRanges, result);
    if (result != VK_SUCCESS) {
        constexpr std::array error_codes = {VK_ERROR_OUT_OF_HOST_MEMORY,VK_ERROR_OUT_OF_DEVICE_MEMORY};
        ValidateReturnCodes("vkInvalidateMappedMemoryRanges", result, error_codes, {});
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
        constexpr std::array error_codes = {VK_ERROR_OUT_OF_HOST_MEMORY,VK_ERROR_OUT_OF_DEVICE_MEMORY,VK_ERROR_INVALID_OPAQUE_CAPTURE_ADDRESS_KHR};
        ValidateReturnCodes("vkBindBufferMemory", result, error_codes, {});
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
        constexpr std::array error_codes = {VK_ERROR_OUT_OF_HOST_MEMORY,VK_ERROR_OUT_OF_DEVICE_MEMORY};
        ValidateReturnCodes("vkBindImageMemory", result, error_codes, {});
    }
}

void BestPractices::PostCallRecordQueueBindSparse(
    VkQueue                                     queue,
    uint32_t                                    bindInfoCount,
    const VkBindSparseInfo*                     pBindInfo,
    VkFence                                     fence,
    VkResult                                    result) {
    ValidationStateTracker::PostCallRecordQueueBindSparse(queue, bindInfoCount, pBindInfo, fence, result);
    ManualPostCallRecordQueueBindSparse(queue, bindInfoCount, pBindInfo, fence, result);
    if (result != VK_SUCCESS) {
        constexpr std::array error_codes = {VK_ERROR_OUT_OF_HOST_MEMORY,VK_ERROR_OUT_OF_DEVICE_MEMORY,VK_ERROR_DEVICE_LOST};
        ValidateReturnCodes("vkQueueBindSparse", result, error_codes, {});
    }
}

void BestPractices::PostCallRecordCreateFence(
    VkDevice                                    device,
    const VkFenceCreateInfo*                    pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkFence*                                    pFence,
    VkResult                                    result) {
    ValidationStateTracker::PostCallRecordCreateFence(device, pCreateInfo, pAllocator, pFence, result);
    if (result != VK_SUCCESS) {
        constexpr std::array error_codes = {VK_ERROR_OUT_OF_HOST_MEMORY,VK_ERROR_OUT_OF_DEVICE_MEMORY};
        ValidateReturnCodes("vkCreateFence", result, error_codes, {});
    }
}

void BestPractices::PostCallRecordResetFences(
    VkDevice                                    device,
    uint32_t                                    fenceCount,
    const VkFence*                              pFences,
    VkResult                                    result) {
    ValidationStateTracker::PostCallRecordResetFences(device, fenceCount, pFences, result);
    if (result != VK_SUCCESS) {
        constexpr std::array error_codes = {VK_ERROR_OUT_OF_DEVICE_MEMORY};
        ValidateReturnCodes("vkResetFences", result, error_codes, {});
    }
}

void BestPractices::PostCallRecordGetFenceStatus(
    VkDevice                                    device,
    VkFence                                     fence,
    VkResult                                    result) {
    ValidationStateTracker::PostCallRecordGetFenceStatus(device, fence, result);
    if (result != VK_SUCCESS) {
        constexpr std::array error_codes = {VK_ERROR_OUT_OF_HOST_MEMORY,VK_ERROR_OUT_OF_DEVICE_MEMORY,VK_ERROR_DEVICE_LOST};
        constexpr std::array success_codes = {VK_NOT_READY};
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
        constexpr std::array error_codes = {VK_ERROR_OUT_OF_HOST_MEMORY,VK_ERROR_OUT_OF_DEVICE_MEMORY,VK_ERROR_DEVICE_LOST};
        constexpr std::array success_codes = {VK_TIMEOUT};
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
    if (result != VK_SUCCESS) {
        constexpr std::array error_codes = {VK_ERROR_OUT_OF_HOST_MEMORY,VK_ERROR_OUT_OF_DEVICE_MEMORY};
        ValidateReturnCodes("vkCreateSemaphore", result, error_codes, {});
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
        constexpr std::array error_codes = {VK_ERROR_OUT_OF_HOST_MEMORY,VK_ERROR_OUT_OF_DEVICE_MEMORY};
        ValidateReturnCodes("vkCreateEvent", result, error_codes, {});
    }
}

void BestPractices::PostCallRecordGetEventStatus(
    VkDevice                                    device,
    VkEvent                                     event,
    VkResult                                    result) {
    ValidationStateTracker::PostCallRecordGetEventStatus(device, event, result);
    if (result != VK_SUCCESS) {
        constexpr std::array error_codes = {VK_ERROR_OUT_OF_HOST_MEMORY,VK_ERROR_OUT_OF_DEVICE_MEMORY,VK_ERROR_DEVICE_LOST};
        constexpr std::array success_codes = {VK_EVENT_SET,VK_EVENT_RESET};
        ValidateReturnCodes("vkGetEventStatus", result, error_codes, success_codes);
    }
}

void BestPractices::PostCallRecordSetEvent(
    VkDevice                                    device,
    VkEvent                                     event,
    VkResult                                    result) {
    ValidationStateTracker::PostCallRecordSetEvent(device, event, result);
    if (result != VK_SUCCESS) {
        constexpr std::array error_codes = {VK_ERROR_OUT_OF_HOST_MEMORY,VK_ERROR_OUT_OF_DEVICE_MEMORY};
        ValidateReturnCodes("vkSetEvent", result, error_codes, {});
    }
}

void BestPractices::PostCallRecordResetEvent(
    VkDevice                                    device,
    VkEvent                                     event,
    VkResult                                    result) {
    ValidationStateTracker::PostCallRecordResetEvent(device, event, result);
    if (result != VK_SUCCESS) {
        constexpr std::array error_codes = {VK_ERROR_OUT_OF_DEVICE_MEMORY};
        ValidateReturnCodes("vkResetEvent", result, error_codes, {});
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
        constexpr std::array error_codes = {VK_ERROR_OUT_OF_HOST_MEMORY,VK_ERROR_OUT_OF_DEVICE_MEMORY};
        ValidateReturnCodes("vkCreateQueryPool", result, error_codes, {});
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
        constexpr std::array error_codes = {VK_ERROR_OUT_OF_HOST_MEMORY,VK_ERROR_OUT_OF_DEVICE_MEMORY,VK_ERROR_DEVICE_LOST};
        constexpr std::array success_codes = {VK_NOT_READY};
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
        constexpr std::array error_codes = {VK_ERROR_OUT_OF_HOST_MEMORY,VK_ERROR_OUT_OF_DEVICE_MEMORY,VK_ERROR_INVALID_OPAQUE_CAPTURE_ADDRESS_KHR};
        ValidateReturnCodes("vkCreateBuffer", result, error_codes, {});
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
        constexpr std::array error_codes = {VK_ERROR_OUT_OF_HOST_MEMORY,VK_ERROR_OUT_OF_DEVICE_MEMORY};
        ValidateReturnCodes("vkCreateBufferView", result, error_codes, {});
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
        constexpr std::array error_codes = {VK_ERROR_OUT_OF_HOST_MEMORY,VK_ERROR_OUT_OF_DEVICE_MEMORY,VK_ERROR_COMPRESSION_EXHAUSTED_EXT,VK_ERROR_INVALID_OPAQUE_CAPTURE_ADDRESS_KHR};
        ValidateReturnCodes("vkCreateImage", result, error_codes, {});
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
        constexpr std::array error_codes = {VK_ERROR_OUT_OF_HOST_MEMORY,VK_ERROR_OUT_OF_DEVICE_MEMORY,VK_ERROR_INVALID_OPAQUE_CAPTURE_ADDRESS_KHR};
        ValidateReturnCodes("vkCreateImageView", result, error_codes, {});
    }
}

void BestPractices::PostCallRecordCreateShaderModule(
    VkDevice                                    device,
    const VkShaderModuleCreateInfo*             pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkShaderModule*                             pShaderModule,
    VkResult                                    result,
    void*                                       state_data) {
    ValidationStateTracker::PostCallRecordCreateShaderModule(device, pCreateInfo, pAllocator, pShaderModule, result, state_data);
    if (result != VK_SUCCESS) {
        constexpr std::array error_codes = {VK_ERROR_OUT_OF_HOST_MEMORY,VK_ERROR_OUT_OF_DEVICE_MEMORY,VK_ERROR_INVALID_SHADER_NV};
        ValidateReturnCodes("vkCreateShaderModule", result, error_codes, {});
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
        constexpr std::array error_codes = {VK_ERROR_OUT_OF_HOST_MEMORY,VK_ERROR_OUT_OF_DEVICE_MEMORY};
        ValidateReturnCodes("vkCreatePipelineCache", result, error_codes, {});
    }
}

void BestPractices::PostCallRecordGetPipelineCacheData(
    VkDevice                                    device,
    VkPipelineCache                             pipelineCache,
    size_t*                                     pDataSize,
    void*                                       pData,
    VkResult                                    result) {
    ValidationStateTracker::PostCallRecordGetPipelineCacheData(device, pipelineCache, pDataSize, pData, result);
    if (result != VK_SUCCESS) {
        constexpr std::array error_codes = {VK_ERROR_OUT_OF_HOST_MEMORY,VK_ERROR_OUT_OF_DEVICE_MEMORY};
        constexpr std::array success_codes = {VK_INCOMPLETE};
        ValidateReturnCodes("vkGetPipelineCacheData", result, error_codes, success_codes);
    }
}

void BestPractices::PostCallRecordMergePipelineCaches(
    VkDevice                                    device,
    VkPipelineCache                             dstCache,
    uint32_t                                    srcCacheCount,
    const VkPipelineCache*                      pSrcCaches,
    VkResult                                    result) {
    ValidationStateTracker::PostCallRecordMergePipelineCaches(device, dstCache, srcCacheCount, pSrcCaches, result);
    if (result != VK_SUCCESS) {
        constexpr std::array error_codes = {VK_ERROR_OUT_OF_HOST_MEMORY,VK_ERROR_OUT_OF_DEVICE_MEMORY};
        ValidateReturnCodes("vkMergePipelineCaches", result, error_codes, {});
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
        constexpr std::array error_codes = {VK_ERROR_OUT_OF_HOST_MEMORY,VK_ERROR_OUT_OF_DEVICE_MEMORY,VK_ERROR_INVALID_SHADER_NV};
        constexpr std::array success_codes = {VK_PIPELINE_COMPILE_REQUIRED_EXT};
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
        constexpr std::array error_codes = {VK_ERROR_OUT_OF_HOST_MEMORY,VK_ERROR_OUT_OF_DEVICE_MEMORY,VK_ERROR_INVALID_SHADER_NV};
        constexpr std::array success_codes = {VK_PIPELINE_COMPILE_REQUIRED_EXT};
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
        constexpr std::array error_codes = {VK_ERROR_OUT_OF_HOST_MEMORY,VK_ERROR_OUT_OF_DEVICE_MEMORY};
        ValidateReturnCodes("vkCreatePipelineLayout", result, error_codes, {});
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
        constexpr std::array error_codes = {VK_ERROR_OUT_OF_HOST_MEMORY,VK_ERROR_OUT_OF_DEVICE_MEMORY,VK_ERROR_INVALID_OPAQUE_CAPTURE_ADDRESS_KHR};
        ValidateReturnCodes("vkCreateSampler", result, error_codes, {});
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
        constexpr std::array error_codes = {VK_ERROR_OUT_OF_HOST_MEMORY,VK_ERROR_OUT_OF_DEVICE_MEMORY};
        ValidateReturnCodes("vkCreateDescriptorSetLayout", result, error_codes, {});
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
        constexpr std::array error_codes = {VK_ERROR_OUT_OF_HOST_MEMORY,VK_ERROR_OUT_OF_DEVICE_MEMORY,VK_ERROR_FRAGMENTATION_EXT};
        ValidateReturnCodes("vkCreateDescriptorPool", result, error_codes, {});
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
        constexpr std::array error_codes = {VK_ERROR_OUT_OF_HOST_MEMORY,VK_ERROR_OUT_OF_DEVICE_MEMORY,VK_ERROR_FRAGMENTED_POOL,VK_ERROR_OUT_OF_POOL_MEMORY};
        ValidateReturnCodes("vkAllocateDescriptorSets", result, error_codes, {});
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
        constexpr std::array error_codes = {VK_ERROR_OUT_OF_HOST_MEMORY,VK_ERROR_OUT_OF_DEVICE_MEMORY};
        ValidateReturnCodes("vkCreateFramebuffer", result, error_codes, {});
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
        constexpr std::array error_codes = {VK_ERROR_OUT_OF_HOST_MEMORY,VK_ERROR_OUT_OF_DEVICE_MEMORY};
        ValidateReturnCodes("vkCreateRenderPass", result, error_codes, {});
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
        constexpr std::array error_codes = {VK_ERROR_OUT_OF_HOST_MEMORY,VK_ERROR_OUT_OF_DEVICE_MEMORY};
        ValidateReturnCodes("vkCreateCommandPool", result, error_codes, {});
    }
}

void BestPractices::PostCallRecordResetCommandPool(
    VkDevice                                    device,
    VkCommandPool                               commandPool,
    VkCommandPoolResetFlags                     flags,
    VkResult                                    result) {
    ValidationStateTracker::PostCallRecordResetCommandPool(device, commandPool, flags, result);
    if (result != VK_SUCCESS) {
        constexpr std::array error_codes = {VK_ERROR_OUT_OF_DEVICE_MEMORY};
        ValidateReturnCodes("vkResetCommandPool", result, error_codes, {});
    }
}

void BestPractices::PostCallRecordAllocateCommandBuffers(
    VkDevice                                    device,
    const VkCommandBufferAllocateInfo*          pAllocateInfo,
    VkCommandBuffer*                            pCommandBuffers,
    VkResult                                    result) {
    ValidationStateTracker::PostCallRecordAllocateCommandBuffers(device, pAllocateInfo, pCommandBuffers, result);
    if (result != VK_SUCCESS) {
        constexpr std::array error_codes = {VK_ERROR_OUT_OF_HOST_MEMORY,VK_ERROR_OUT_OF_DEVICE_MEMORY};
        ValidateReturnCodes("vkAllocateCommandBuffers", result, error_codes, {});
    }
}

void BestPractices::PostCallRecordBeginCommandBuffer(
    VkCommandBuffer                             commandBuffer,
    const VkCommandBufferBeginInfo*             pBeginInfo,
    VkResult                                    result) {
    ValidationStateTracker::PostCallRecordBeginCommandBuffer(commandBuffer, pBeginInfo, result);
    if (result != VK_SUCCESS) {
        constexpr std::array error_codes = {VK_ERROR_OUT_OF_HOST_MEMORY,VK_ERROR_OUT_OF_DEVICE_MEMORY};
        ValidateReturnCodes("vkBeginCommandBuffer", result, error_codes, {});
    }
}

void BestPractices::PostCallRecordEndCommandBuffer(
    VkCommandBuffer                             commandBuffer,
    VkResult                                    result) {
    ValidationStateTracker::PostCallRecordEndCommandBuffer(commandBuffer, result);
    if (result != VK_SUCCESS) {
        constexpr std::array error_codes = {VK_ERROR_OUT_OF_HOST_MEMORY,VK_ERROR_OUT_OF_DEVICE_MEMORY};
        ValidateReturnCodes("vkEndCommandBuffer", result, error_codes, {});
    }
}

void BestPractices::PostCallRecordResetCommandBuffer(
    VkCommandBuffer                             commandBuffer,
    VkCommandBufferResetFlags                   flags,
    VkResult                                    result) {
    ValidationStateTracker::PostCallRecordResetCommandBuffer(commandBuffer, flags, result);
    if (result != VK_SUCCESS) {
        constexpr std::array error_codes = {VK_ERROR_OUT_OF_DEVICE_MEMORY};
        ValidateReturnCodes("vkResetCommandBuffer", result, error_codes, {});
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
        constexpr std::array error_codes = {VK_ERROR_OUT_OF_HOST_MEMORY,VK_ERROR_OUT_OF_DEVICE_MEMORY,VK_ERROR_INVALID_OPAQUE_CAPTURE_ADDRESS_KHR};
        ValidateReturnCodes("vkBindBufferMemory2", result, error_codes, {});
    }
}

void BestPractices::PostCallRecordBindImageMemory2(
    VkDevice                                    device,
    uint32_t                                    bindInfoCount,
    const VkBindImageMemoryInfo*                pBindInfos,
    VkResult                                    result) {
    ValidationStateTracker::PostCallRecordBindImageMemory2(device, bindInfoCount, pBindInfos, result);
    if (result != VK_SUCCESS) {
        constexpr std::array error_codes = {VK_ERROR_OUT_OF_HOST_MEMORY,VK_ERROR_OUT_OF_DEVICE_MEMORY};
        ValidateReturnCodes("vkBindImageMemory2", result, error_codes, {});
    }
}

void BestPractices::PostCallRecordEnumeratePhysicalDeviceGroups(
    VkInstance                                  instance,
    uint32_t*                                   pPhysicalDeviceGroupCount,
    VkPhysicalDeviceGroupProperties*            pPhysicalDeviceGroupProperties,
    VkResult                                    result) {
    ValidationStateTracker::PostCallRecordEnumeratePhysicalDeviceGroups(instance, pPhysicalDeviceGroupCount, pPhysicalDeviceGroupProperties, result);
    if (result != VK_SUCCESS) {
        constexpr std::array error_codes = {VK_ERROR_OUT_OF_HOST_MEMORY,VK_ERROR_OUT_OF_DEVICE_MEMORY,VK_ERROR_INITIALIZATION_FAILED};
        constexpr std::array success_codes = {VK_INCOMPLETE};
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
        constexpr std::array error_codes = {VK_ERROR_OUT_OF_HOST_MEMORY,VK_ERROR_OUT_OF_DEVICE_MEMORY,VK_ERROR_FORMAT_NOT_SUPPORTED,VK_ERROR_IMAGE_USAGE_NOT_SUPPORTED_KHR,VK_ERROR_VIDEO_PROFILE_OPERATION_NOT_SUPPORTED_KHR,VK_ERROR_VIDEO_PROFILE_FORMAT_NOT_SUPPORTED_KHR,VK_ERROR_VIDEO_PICTURE_LAYOUT_NOT_SUPPORTED_KHR,VK_ERROR_VIDEO_PROFILE_CODEC_NOT_SUPPORTED_KHR};
        ValidateReturnCodes("vkGetPhysicalDeviceImageFormatProperties2", result, error_codes, {});
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
        constexpr std::array error_codes = {VK_ERROR_OUT_OF_HOST_MEMORY,VK_ERROR_OUT_OF_DEVICE_MEMORY};
        ValidateReturnCodes("vkCreateSamplerYcbcrConversion", result, error_codes, {});
    }
}

void BestPractices::PostCallRecordCreateDescriptorUpdateTemplate(
    VkDevice                                    device,
    const VkDescriptorUpdateTemplateCreateInfo* pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkDescriptorUpdateTemplate*                 pDescriptorUpdateTemplate,
    VkResult                                    result) {
    ValidationStateTracker::PostCallRecordCreateDescriptorUpdateTemplate(device, pCreateInfo, pAllocator, pDescriptorUpdateTemplate, result);
    if (result != VK_SUCCESS) {
        constexpr std::array error_codes = {VK_ERROR_OUT_OF_HOST_MEMORY,VK_ERROR_OUT_OF_DEVICE_MEMORY};
        ValidateReturnCodes("vkCreateDescriptorUpdateTemplate", result, error_codes, {});
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
        constexpr std::array error_codes = {VK_ERROR_OUT_OF_HOST_MEMORY,VK_ERROR_OUT_OF_DEVICE_MEMORY};
        ValidateReturnCodes("vkCreateRenderPass2", result, error_codes, {});
    }
}

void BestPractices::PostCallRecordGetSemaphoreCounterValue(
    VkDevice                                    device,
    VkSemaphore                                 semaphore,
    uint64_t*                                   pValue,
    VkResult                                    result) {
    ValidationStateTracker::PostCallRecordGetSemaphoreCounterValue(device, semaphore, pValue, result);
    if (result != VK_SUCCESS) {
        constexpr std::array error_codes = {VK_ERROR_OUT_OF_HOST_MEMORY,VK_ERROR_OUT_OF_DEVICE_MEMORY,VK_ERROR_DEVICE_LOST};
        ValidateReturnCodes("vkGetSemaphoreCounterValue", result, error_codes, {});
    }
}

void BestPractices::PostCallRecordWaitSemaphores(
    VkDevice                                    device,
    const VkSemaphoreWaitInfo*                  pWaitInfo,
    uint64_t                                    timeout,
    VkResult                                    result) {
    ValidationStateTracker::PostCallRecordWaitSemaphores(device, pWaitInfo, timeout, result);
    if (result != VK_SUCCESS) {
        constexpr std::array error_codes = {VK_ERROR_OUT_OF_HOST_MEMORY,VK_ERROR_OUT_OF_DEVICE_MEMORY,VK_ERROR_DEVICE_LOST};
        constexpr std::array success_codes = {VK_TIMEOUT};
        ValidateReturnCodes("vkWaitSemaphores", result, error_codes, success_codes);
    }
}

void BestPractices::PostCallRecordSignalSemaphore(
    VkDevice                                    device,
    const VkSemaphoreSignalInfo*                pSignalInfo,
    VkResult                                    result) {
    ValidationStateTracker::PostCallRecordSignalSemaphore(device, pSignalInfo, result);
    if (result != VK_SUCCESS) {
        constexpr std::array error_codes = {VK_ERROR_OUT_OF_HOST_MEMORY,VK_ERROR_OUT_OF_DEVICE_MEMORY};
        ValidateReturnCodes("vkSignalSemaphore", result, error_codes, {});
    }
}

void BestPractices::PostCallRecordGetPhysicalDeviceToolProperties(
    VkPhysicalDevice                            physicalDevice,
    uint32_t*                                   pToolCount,
    VkPhysicalDeviceToolProperties*             pToolProperties,
    VkResult                                    result) {
    ValidationStateTracker::PostCallRecordGetPhysicalDeviceToolProperties(physicalDevice, pToolCount, pToolProperties, result);
    if (result != VK_SUCCESS) {
        constexpr std::array error_codes = {VK_ERROR_OUT_OF_HOST_MEMORY};
        constexpr std::array success_codes = {VK_INCOMPLETE};
        ValidateReturnCodes("vkGetPhysicalDeviceToolProperties", result, error_codes, success_codes);
    }
}

void BestPractices::PostCallRecordCreatePrivateDataSlot(
    VkDevice                                    device,
    const VkPrivateDataSlotCreateInfo*          pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkPrivateDataSlot*                          pPrivateDataSlot,
    VkResult                                    result) {
    ValidationStateTracker::PostCallRecordCreatePrivateDataSlot(device, pCreateInfo, pAllocator, pPrivateDataSlot, result);
    if (result != VK_SUCCESS) {
        constexpr std::array error_codes = {VK_ERROR_OUT_OF_HOST_MEMORY};
        ValidateReturnCodes("vkCreatePrivateDataSlot", result, error_codes, {});
    }
}

void BestPractices::PostCallRecordSetPrivateData(
    VkDevice                                    device,
    VkObjectType                                objectType,
    uint64_t                                    objectHandle,
    VkPrivateDataSlot                           privateDataSlot,
    uint64_t                                    data,
    VkResult                                    result) {
    ValidationStateTracker::PostCallRecordSetPrivateData(device, objectType, objectHandle, privateDataSlot, data, result);
    if (result != VK_SUCCESS) {
        constexpr std::array error_codes = {VK_ERROR_OUT_OF_HOST_MEMORY};
        ValidateReturnCodes("vkSetPrivateData", result, error_codes, {});
    }
}

void BestPractices::PostCallRecordQueueSubmit2(
    VkQueue                                     queue,
    uint32_t                                    submitCount,
    const VkSubmitInfo2*                        pSubmits,
    VkFence                                     fence,
    VkResult                                    result) {
    ValidationStateTracker::PostCallRecordQueueSubmit2(queue, submitCount, pSubmits, fence, result);
    if (result != VK_SUCCESS) {
        constexpr std::array error_codes = {VK_ERROR_OUT_OF_HOST_MEMORY,VK_ERROR_OUT_OF_DEVICE_MEMORY,VK_ERROR_DEVICE_LOST};
        ValidateReturnCodes("vkQueueSubmit2", result, error_codes, {});
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
        constexpr std::array error_codes = {VK_ERROR_OUT_OF_HOST_MEMORY,VK_ERROR_OUT_OF_DEVICE_MEMORY,VK_ERROR_SURFACE_LOST_KHR};
        ValidateReturnCodes("vkGetPhysicalDeviceSurfaceSupportKHR", result, error_codes, {});
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
        constexpr std::array error_codes = {VK_ERROR_OUT_OF_HOST_MEMORY,VK_ERROR_OUT_OF_DEVICE_MEMORY,VK_ERROR_SURFACE_LOST_KHR};
        ValidateReturnCodes("vkGetPhysicalDeviceSurfaceCapabilitiesKHR", result, error_codes, {});
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
        constexpr std::array error_codes = {VK_ERROR_OUT_OF_HOST_MEMORY,VK_ERROR_OUT_OF_DEVICE_MEMORY,VK_ERROR_SURFACE_LOST_KHR};
        constexpr std::array success_codes = {VK_INCOMPLETE};
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
        constexpr std::array error_codes = {VK_ERROR_OUT_OF_HOST_MEMORY,VK_ERROR_OUT_OF_DEVICE_MEMORY,VK_ERROR_SURFACE_LOST_KHR};
        constexpr std::array success_codes = {VK_INCOMPLETE};
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
        constexpr std::array error_codes = {VK_ERROR_OUT_OF_HOST_MEMORY,VK_ERROR_OUT_OF_DEVICE_MEMORY,VK_ERROR_DEVICE_LOST,VK_ERROR_SURFACE_LOST_KHR,VK_ERROR_NATIVE_WINDOW_IN_USE_KHR,VK_ERROR_INITIALIZATION_FAILED,VK_ERROR_COMPRESSION_EXHAUSTED_EXT};
        ValidateReturnCodes("vkCreateSwapchainKHR", result, error_codes, {});
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
        constexpr std::array error_codes = {VK_ERROR_OUT_OF_HOST_MEMORY,VK_ERROR_OUT_OF_DEVICE_MEMORY};
        constexpr std::array success_codes = {VK_INCOMPLETE};
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
        constexpr std::array error_codes = {VK_ERROR_OUT_OF_HOST_MEMORY,VK_ERROR_OUT_OF_DEVICE_MEMORY,VK_ERROR_DEVICE_LOST,VK_ERROR_OUT_OF_DATE_KHR,VK_ERROR_SURFACE_LOST_KHR,VK_ERROR_FULL_SCREEN_EXCLUSIVE_MODE_LOST_EXT};
        constexpr std::array success_codes = {VK_TIMEOUT,VK_NOT_READY,VK_SUBOPTIMAL_KHR};
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
        constexpr std::array error_codes = {VK_ERROR_OUT_OF_HOST_MEMORY,VK_ERROR_OUT_OF_DEVICE_MEMORY,VK_ERROR_DEVICE_LOST,VK_ERROR_OUT_OF_DATE_KHR,VK_ERROR_SURFACE_LOST_KHR,VK_ERROR_FULL_SCREEN_EXCLUSIVE_MODE_LOST_EXT};
        constexpr std::array success_codes = {VK_SUBOPTIMAL_KHR};
        ValidateReturnCodes("vkQueuePresentKHR", result, error_codes, success_codes);
    }
}

void BestPractices::PostCallRecordGetDeviceGroupPresentCapabilitiesKHR(
    VkDevice                                    device,
    VkDeviceGroupPresentCapabilitiesKHR*        pDeviceGroupPresentCapabilities,
    VkResult                                    result) {
    ValidationStateTracker::PostCallRecordGetDeviceGroupPresentCapabilitiesKHR(device, pDeviceGroupPresentCapabilities, result);
    if (result != VK_SUCCESS) {
        constexpr std::array error_codes = {VK_ERROR_OUT_OF_HOST_MEMORY,VK_ERROR_OUT_OF_DEVICE_MEMORY};
        ValidateReturnCodes("vkGetDeviceGroupPresentCapabilitiesKHR", result, error_codes, {});
    }
}

void BestPractices::PostCallRecordGetDeviceGroupSurfacePresentModesKHR(
    VkDevice                                    device,
    VkSurfaceKHR                                surface,
    VkDeviceGroupPresentModeFlagsKHR*           pModes,
    VkResult                                    result) {
    ValidationStateTracker::PostCallRecordGetDeviceGroupSurfacePresentModesKHR(device, surface, pModes, result);
    if (result != VK_SUCCESS) {
        constexpr std::array error_codes = {VK_ERROR_OUT_OF_HOST_MEMORY,VK_ERROR_OUT_OF_DEVICE_MEMORY,VK_ERROR_SURFACE_LOST_KHR};
        ValidateReturnCodes("vkGetDeviceGroupSurfacePresentModesKHR", result, error_codes, {});
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
        constexpr std::array error_codes = {VK_ERROR_OUT_OF_HOST_MEMORY,VK_ERROR_OUT_OF_DEVICE_MEMORY};
        constexpr std::array success_codes = {VK_INCOMPLETE};
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
        constexpr std::array error_codes = {VK_ERROR_OUT_OF_HOST_MEMORY,VK_ERROR_OUT_OF_DEVICE_MEMORY,VK_ERROR_DEVICE_LOST,VK_ERROR_OUT_OF_DATE_KHR,VK_ERROR_SURFACE_LOST_KHR,VK_ERROR_FULL_SCREEN_EXCLUSIVE_MODE_LOST_EXT};
        constexpr std::array success_codes = {VK_TIMEOUT,VK_NOT_READY,VK_SUBOPTIMAL_KHR};
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
        constexpr std::array error_codes = {VK_ERROR_OUT_OF_HOST_MEMORY,VK_ERROR_OUT_OF_DEVICE_MEMORY};
        constexpr std::array success_codes = {VK_INCOMPLETE};
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
        constexpr std::array error_codes = {VK_ERROR_OUT_OF_HOST_MEMORY,VK_ERROR_OUT_OF_DEVICE_MEMORY};
        constexpr std::array success_codes = {VK_INCOMPLETE};
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
        constexpr std::array error_codes = {VK_ERROR_OUT_OF_HOST_MEMORY,VK_ERROR_OUT_OF_DEVICE_MEMORY};
        constexpr std::array success_codes = {VK_INCOMPLETE};
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
        constexpr std::array error_codes = {VK_ERROR_OUT_OF_HOST_MEMORY,VK_ERROR_OUT_OF_DEVICE_MEMORY};
        constexpr std::array success_codes = {VK_INCOMPLETE};
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
        constexpr std::array error_codes = {VK_ERROR_OUT_OF_HOST_MEMORY,VK_ERROR_OUT_OF_DEVICE_MEMORY,VK_ERROR_INITIALIZATION_FAILED};
        ValidateReturnCodes("vkCreateDisplayModeKHR", result, error_codes, {});
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
        constexpr std::array error_codes = {VK_ERROR_OUT_OF_HOST_MEMORY,VK_ERROR_OUT_OF_DEVICE_MEMORY};
        ValidateReturnCodes("vkGetDisplayPlaneCapabilitiesKHR", result, error_codes, {});
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
        constexpr std::array error_codes = {VK_ERROR_OUT_OF_HOST_MEMORY,VK_ERROR_OUT_OF_DEVICE_MEMORY};
        ValidateReturnCodes("vkCreateDisplayPlaneSurfaceKHR", result, error_codes, {});
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
        constexpr std::array error_codes = {VK_ERROR_OUT_OF_HOST_MEMORY,VK_ERROR_OUT_OF_DEVICE_MEMORY,VK_ERROR_INCOMPATIBLE_DISPLAY_KHR,VK_ERROR_DEVICE_LOST,VK_ERROR_SURFACE_LOST_KHR};
        ValidateReturnCodes("vkCreateSharedSwapchainsKHR", result, error_codes, {});
    }
}

#ifdef VK_USE_PLATFORM_XLIB_KHR

void BestPractices::PostCallRecordCreateXlibSurfaceKHR(
    VkInstance                                  instance,
    const VkXlibSurfaceCreateInfoKHR*           pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkSurfaceKHR*                               pSurface,
    VkResult                                    result) {
    ValidationStateTracker::PostCallRecordCreateXlibSurfaceKHR(instance, pCreateInfo, pAllocator, pSurface, result);
    if (result != VK_SUCCESS) {
        constexpr std::array error_codes = {VK_ERROR_OUT_OF_HOST_MEMORY,VK_ERROR_OUT_OF_DEVICE_MEMORY};
        ValidateReturnCodes("vkCreateXlibSurfaceKHR", result, error_codes, {});
    }
}

#endif // VK_USE_PLATFORM_XLIB_KHR

#ifdef VK_USE_PLATFORM_XCB_KHR

void BestPractices::PostCallRecordCreateXcbSurfaceKHR(
    VkInstance                                  instance,
    const VkXcbSurfaceCreateInfoKHR*            pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkSurfaceKHR*                               pSurface,
    VkResult                                    result) {
    ValidationStateTracker::PostCallRecordCreateXcbSurfaceKHR(instance, pCreateInfo, pAllocator, pSurface, result);
    if (result != VK_SUCCESS) {
        constexpr std::array error_codes = {VK_ERROR_OUT_OF_HOST_MEMORY,VK_ERROR_OUT_OF_DEVICE_MEMORY};
        ValidateReturnCodes("vkCreateXcbSurfaceKHR", result, error_codes, {});
    }
}

#endif // VK_USE_PLATFORM_XCB_KHR

#ifdef VK_USE_PLATFORM_WAYLAND_KHR

void BestPractices::PostCallRecordCreateWaylandSurfaceKHR(
    VkInstance                                  instance,
    const VkWaylandSurfaceCreateInfoKHR*        pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkSurfaceKHR*                               pSurface,
    VkResult                                    result) {
    ValidationStateTracker::PostCallRecordCreateWaylandSurfaceKHR(instance, pCreateInfo, pAllocator, pSurface, result);
    if (result != VK_SUCCESS) {
        constexpr std::array error_codes = {VK_ERROR_OUT_OF_HOST_MEMORY,VK_ERROR_OUT_OF_DEVICE_MEMORY};
        ValidateReturnCodes("vkCreateWaylandSurfaceKHR", result, error_codes, {});
    }
}

#endif // VK_USE_PLATFORM_WAYLAND_KHR

#ifdef VK_USE_PLATFORM_ANDROID_KHR

void BestPractices::PostCallRecordCreateAndroidSurfaceKHR(
    VkInstance                                  instance,
    const VkAndroidSurfaceCreateInfoKHR*        pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkSurfaceKHR*                               pSurface,
    VkResult                                    result) {
    ValidationStateTracker::PostCallRecordCreateAndroidSurfaceKHR(instance, pCreateInfo, pAllocator, pSurface, result);
    if (result != VK_SUCCESS) {
        constexpr std::array error_codes = {VK_ERROR_OUT_OF_HOST_MEMORY,VK_ERROR_OUT_OF_DEVICE_MEMORY,VK_ERROR_NATIVE_WINDOW_IN_USE_KHR};
        ValidateReturnCodes("vkCreateAndroidSurfaceKHR", result, error_codes, {});
    }
}

#endif // VK_USE_PLATFORM_ANDROID_KHR

#ifdef VK_USE_PLATFORM_WIN32_KHR

void BestPractices::PostCallRecordCreateWin32SurfaceKHR(
    VkInstance                                  instance,
    const VkWin32SurfaceCreateInfoKHR*          pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkSurfaceKHR*                               pSurface,
    VkResult                                    result) {
    ValidationStateTracker::PostCallRecordCreateWin32SurfaceKHR(instance, pCreateInfo, pAllocator, pSurface, result);
    if (result != VK_SUCCESS) {
        constexpr std::array error_codes = {VK_ERROR_OUT_OF_HOST_MEMORY,VK_ERROR_OUT_OF_DEVICE_MEMORY};
        ValidateReturnCodes("vkCreateWin32SurfaceKHR", result, error_codes, {});
    }
}

#endif // VK_USE_PLATFORM_WIN32_KHR

void BestPractices::PostCallRecordGetPhysicalDeviceVideoCapabilitiesKHR(
    VkPhysicalDevice                            physicalDevice,
    const VkVideoProfileInfoKHR*                pVideoProfile,
    VkVideoCapabilitiesKHR*                     pCapabilities,
    VkResult                                    result) {
    ValidationStateTracker::PostCallRecordGetPhysicalDeviceVideoCapabilitiesKHR(physicalDevice, pVideoProfile, pCapabilities, result);
    if (result != VK_SUCCESS) {
        constexpr std::array error_codes = {VK_ERROR_OUT_OF_HOST_MEMORY,VK_ERROR_OUT_OF_DEVICE_MEMORY,VK_ERROR_VIDEO_PROFILE_OPERATION_NOT_SUPPORTED_KHR,VK_ERROR_VIDEO_PROFILE_FORMAT_NOT_SUPPORTED_KHR,VK_ERROR_VIDEO_PICTURE_LAYOUT_NOT_SUPPORTED_KHR,VK_ERROR_VIDEO_PROFILE_CODEC_NOT_SUPPORTED_KHR};
        ValidateReturnCodes("vkGetPhysicalDeviceVideoCapabilitiesKHR", result, error_codes, {});
    }
}

void BestPractices::PostCallRecordGetPhysicalDeviceVideoFormatPropertiesKHR(
    VkPhysicalDevice                            physicalDevice,
    const VkPhysicalDeviceVideoFormatInfoKHR*   pVideoFormatInfo,
    uint32_t*                                   pVideoFormatPropertyCount,
    VkVideoFormatPropertiesKHR*                 pVideoFormatProperties,
    VkResult                                    result) {
    ValidationStateTracker::PostCallRecordGetPhysicalDeviceVideoFormatPropertiesKHR(physicalDevice, pVideoFormatInfo, pVideoFormatPropertyCount, pVideoFormatProperties, result);
    if (result != VK_SUCCESS) {
        constexpr std::array error_codes = {VK_ERROR_OUT_OF_HOST_MEMORY,VK_ERROR_OUT_OF_DEVICE_MEMORY,VK_ERROR_IMAGE_USAGE_NOT_SUPPORTED_KHR,VK_ERROR_VIDEO_PROFILE_OPERATION_NOT_SUPPORTED_KHR,VK_ERROR_VIDEO_PROFILE_FORMAT_NOT_SUPPORTED_KHR,VK_ERROR_VIDEO_PICTURE_LAYOUT_NOT_SUPPORTED_KHR,VK_ERROR_VIDEO_PROFILE_CODEC_NOT_SUPPORTED_KHR};
        constexpr std::array success_codes = {VK_INCOMPLETE};
        ValidateReturnCodes("vkGetPhysicalDeviceVideoFormatPropertiesKHR", result, error_codes, success_codes);
    }
}

void BestPractices::PostCallRecordCreateVideoSessionKHR(
    VkDevice                                    device,
    const VkVideoSessionCreateInfoKHR*          pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkVideoSessionKHR*                          pVideoSession,
    VkResult                                    result) {
    ValidationStateTracker::PostCallRecordCreateVideoSessionKHR(device, pCreateInfo, pAllocator, pVideoSession, result);
    if (result != VK_SUCCESS) {
        constexpr std::array error_codes = {VK_ERROR_OUT_OF_HOST_MEMORY,VK_ERROR_OUT_OF_DEVICE_MEMORY,VK_ERROR_INITIALIZATION_FAILED,VK_ERROR_VIDEO_STD_VERSION_NOT_SUPPORTED_KHR};
        ValidateReturnCodes("vkCreateVideoSessionKHR", result, error_codes, {});
    }
}

void BestPractices::PostCallRecordGetVideoSessionMemoryRequirementsKHR(
    VkDevice                                    device,
    VkVideoSessionKHR                           videoSession,
    uint32_t*                                   pMemoryRequirementsCount,
    VkVideoSessionMemoryRequirementsKHR*        pMemoryRequirements,
    VkResult                                    result) {
    ValidationStateTracker::PostCallRecordGetVideoSessionMemoryRequirementsKHR(device, videoSession, pMemoryRequirementsCount, pMemoryRequirements, result);
    if (result != VK_SUCCESS) {
        constexpr std::array success_codes = {VK_INCOMPLETE};
        ValidateReturnCodes("vkGetVideoSessionMemoryRequirementsKHR", result, {}, success_codes);
    }
}

void BestPractices::PostCallRecordBindVideoSessionMemoryKHR(
    VkDevice                                    device,
    VkVideoSessionKHR                           videoSession,
    uint32_t                                    bindSessionMemoryInfoCount,
    const VkBindVideoSessionMemoryInfoKHR*      pBindSessionMemoryInfos,
    VkResult                                    result) {
    ValidationStateTracker::PostCallRecordBindVideoSessionMemoryKHR(device, videoSession, bindSessionMemoryInfoCount, pBindSessionMemoryInfos, result);
    if (result != VK_SUCCESS) {
        constexpr std::array error_codes = {VK_ERROR_OUT_OF_HOST_MEMORY,VK_ERROR_OUT_OF_DEVICE_MEMORY};
        ValidateReturnCodes("vkBindVideoSessionMemoryKHR", result, error_codes, {});
    }
}

void BestPractices::PostCallRecordCreateVideoSessionParametersKHR(
    VkDevice                                    device,
    const VkVideoSessionParametersCreateInfoKHR* pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkVideoSessionParametersKHR*                pVideoSessionParameters,
    VkResult                                    result) {
    ValidationStateTracker::PostCallRecordCreateVideoSessionParametersKHR(device, pCreateInfo, pAllocator, pVideoSessionParameters, result);
    if (result != VK_SUCCESS) {
        constexpr std::array error_codes = {VK_ERROR_OUT_OF_HOST_MEMORY,VK_ERROR_OUT_OF_DEVICE_MEMORY,VK_ERROR_INITIALIZATION_FAILED};
        ValidateReturnCodes("vkCreateVideoSessionParametersKHR", result, error_codes, {});
    }
}

void BestPractices::PostCallRecordUpdateVideoSessionParametersKHR(
    VkDevice                                    device,
    VkVideoSessionParametersKHR                 videoSessionParameters,
    const VkVideoSessionParametersUpdateInfoKHR* pUpdateInfo,
    VkResult                                    result) {
    ValidationStateTracker::PostCallRecordUpdateVideoSessionParametersKHR(device, videoSessionParameters, pUpdateInfo, result);
    if (result != VK_SUCCESS) {
        constexpr std::array error_codes = {VK_ERROR_OUT_OF_HOST_MEMORY,VK_ERROR_OUT_OF_DEVICE_MEMORY};
        ValidateReturnCodes("vkUpdateVideoSessionParametersKHR", result, error_codes, {});
    }
}

void BestPractices::PostCallRecordGetPhysicalDeviceImageFormatProperties2KHR(
    VkPhysicalDevice                            physicalDevice,
    const VkPhysicalDeviceImageFormatInfo2*     pImageFormatInfo,
    VkImageFormatProperties2*                   pImageFormatProperties,
    VkResult                                    result) {
    ValidationStateTracker::PostCallRecordGetPhysicalDeviceImageFormatProperties2KHR(physicalDevice, pImageFormatInfo, pImageFormatProperties, result);
    if (result != VK_SUCCESS) {
        constexpr std::array error_codes = {VK_ERROR_OUT_OF_HOST_MEMORY,VK_ERROR_OUT_OF_DEVICE_MEMORY,VK_ERROR_FORMAT_NOT_SUPPORTED,VK_ERROR_IMAGE_USAGE_NOT_SUPPORTED_KHR,VK_ERROR_VIDEO_PROFILE_OPERATION_NOT_SUPPORTED_KHR,VK_ERROR_VIDEO_PROFILE_FORMAT_NOT_SUPPORTED_KHR,VK_ERROR_VIDEO_PICTURE_LAYOUT_NOT_SUPPORTED_KHR,VK_ERROR_VIDEO_PROFILE_CODEC_NOT_SUPPORTED_KHR};
        ValidateReturnCodes("vkGetPhysicalDeviceImageFormatProperties2KHR", result, error_codes, {});
    }
}

void BestPractices::PostCallRecordEnumeratePhysicalDeviceGroupsKHR(
    VkInstance                                  instance,
    uint32_t*                                   pPhysicalDeviceGroupCount,
    VkPhysicalDeviceGroupProperties*            pPhysicalDeviceGroupProperties,
    VkResult                                    result) {
    ValidationStateTracker::PostCallRecordEnumeratePhysicalDeviceGroupsKHR(instance, pPhysicalDeviceGroupCount, pPhysicalDeviceGroupProperties, result);
    if (result != VK_SUCCESS) {
        constexpr std::array error_codes = {VK_ERROR_OUT_OF_HOST_MEMORY,VK_ERROR_OUT_OF_DEVICE_MEMORY,VK_ERROR_INITIALIZATION_FAILED};
        constexpr std::array success_codes = {VK_INCOMPLETE};
        ValidateReturnCodes("vkEnumeratePhysicalDeviceGroupsKHR", result, error_codes, success_codes);
    }
}

#ifdef VK_USE_PLATFORM_WIN32_KHR

void BestPractices::PostCallRecordGetMemoryWin32HandleKHR(
    VkDevice                                    device,
    const VkMemoryGetWin32HandleInfoKHR*        pGetWin32HandleInfo,
    HANDLE*                                     pHandle,
    VkResult                                    result) {
    ValidationStateTracker::PostCallRecordGetMemoryWin32HandleKHR(device, pGetWin32HandleInfo, pHandle, result);
    if (result != VK_SUCCESS) {
        constexpr std::array error_codes = {VK_ERROR_TOO_MANY_OBJECTS,VK_ERROR_OUT_OF_HOST_MEMORY};
        ValidateReturnCodes("vkGetMemoryWin32HandleKHR", result, error_codes, {});
    }
}

#endif // VK_USE_PLATFORM_WIN32_KHR

#ifdef VK_USE_PLATFORM_WIN32_KHR

void BestPractices::PostCallRecordGetMemoryWin32HandlePropertiesKHR(
    VkDevice                                    device,
    VkExternalMemoryHandleTypeFlagBits          handleType,
    HANDLE                                      handle,
    VkMemoryWin32HandlePropertiesKHR*           pMemoryWin32HandleProperties,
    VkResult                                    result) {
    ValidationStateTracker::PostCallRecordGetMemoryWin32HandlePropertiesKHR(device, handleType, handle, pMemoryWin32HandleProperties, result);
    if (result != VK_SUCCESS) {
        constexpr std::array error_codes = {VK_ERROR_OUT_OF_HOST_MEMORY,VK_ERROR_INVALID_EXTERNAL_HANDLE};
        ValidateReturnCodes("vkGetMemoryWin32HandlePropertiesKHR", result, error_codes, {});
    }
}

#endif // VK_USE_PLATFORM_WIN32_KHR

void BestPractices::PostCallRecordGetMemoryFdKHR(
    VkDevice                                    device,
    const VkMemoryGetFdInfoKHR*                 pGetFdInfo,
    int*                                        pFd,
    VkResult                                    result) {
    ValidationStateTracker::PostCallRecordGetMemoryFdKHR(device, pGetFdInfo, pFd, result);
    if (result != VK_SUCCESS) {
        constexpr std::array error_codes = {VK_ERROR_TOO_MANY_OBJECTS,VK_ERROR_OUT_OF_HOST_MEMORY};
        ValidateReturnCodes("vkGetMemoryFdKHR", result, error_codes, {});
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
        constexpr std::array error_codes = {VK_ERROR_OUT_OF_HOST_MEMORY,VK_ERROR_INVALID_EXTERNAL_HANDLE};
        ValidateReturnCodes("vkGetMemoryFdPropertiesKHR", result, error_codes, {});
    }
}

#ifdef VK_USE_PLATFORM_WIN32_KHR

void BestPractices::PostCallRecordImportSemaphoreWin32HandleKHR(
    VkDevice                                    device,
    const VkImportSemaphoreWin32HandleInfoKHR*  pImportSemaphoreWin32HandleInfo,
    VkResult                                    result) {
    ValidationStateTracker::PostCallRecordImportSemaphoreWin32HandleKHR(device, pImportSemaphoreWin32HandleInfo, result);
    if (result != VK_SUCCESS) {
        constexpr std::array error_codes = {VK_ERROR_OUT_OF_HOST_MEMORY,VK_ERROR_INVALID_EXTERNAL_HANDLE};
        ValidateReturnCodes("vkImportSemaphoreWin32HandleKHR", result, error_codes, {});
    }
}

#endif // VK_USE_PLATFORM_WIN32_KHR

#ifdef VK_USE_PLATFORM_WIN32_KHR

void BestPractices::PostCallRecordGetSemaphoreWin32HandleKHR(
    VkDevice                                    device,
    const VkSemaphoreGetWin32HandleInfoKHR*     pGetWin32HandleInfo,
    HANDLE*                                     pHandle,
    VkResult                                    result) {
    ValidationStateTracker::PostCallRecordGetSemaphoreWin32HandleKHR(device, pGetWin32HandleInfo, pHandle, result);
    if (result != VK_SUCCESS) {
        constexpr std::array error_codes = {VK_ERROR_TOO_MANY_OBJECTS,VK_ERROR_OUT_OF_HOST_MEMORY};
        ValidateReturnCodes("vkGetSemaphoreWin32HandleKHR", result, error_codes, {});
    }
}

#endif // VK_USE_PLATFORM_WIN32_KHR

void BestPractices::PostCallRecordImportSemaphoreFdKHR(
    VkDevice                                    device,
    const VkImportSemaphoreFdInfoKHR*           pImportSemaphoreFdInfo,
    VkResult                                    result) {
    ValidationStateTracker::PostCallRecordImportSemaphoreFdKHR(device, pImportSemaphoreFdInfo, result);
    if (result != VK_SUCCESS) {
        constexpr std::array error_codes = {VK_ERROR_OUT_OF_HOST_MEMORY,VK_ERROR_INVALID_EXTERNAL_HANDLE};
        ValidateReturnCodes("vkImportSemaphoreFdKHR", result, error_codes, {});
    }
}

void BestPractices::PostCallRecordGetSemaphoreFdKHR(
    VkDevice                                    device,
    const VkSemaphoreGetFdInfoKHR*              pGetFdInfo,
    int*                                        pFd,
    VkResult                                    result) {
    ValidationStateTracker::PostCallRecordGetSemaphoreFdKHR(device, pGetFdInfo, pFd, result);
    if (result != VK_SUCCESS) {
        constexpr std::array error_codes = {VK_ERROR_TOO_MANY_OBJECTS,VK_ERROR_OUT_OF_HOST_MEMORY};
        ValidateReturnCodes("vkGetSemaphoreFdKHR", result, error_codes, {});
    }
}

void BestPractices::PostCallRecordCreateDescriptorUpdateTemplateKHR(
    VkDevice                                    device,
    const VkDescriptorUpdateTemplateCreateInfo* pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkDescriptorUpdateTemplate*                 pDescriptorUpdateTemplate,
    VkResult                                    result) {
    ValidationStateTracker::PostCallRecordCreateDescriptorUpdateTemplateKHR(device, pCreateInfo, pAllocator, pDescriptorUpdateTemplate, result);
    if (result != VK_SUCCESS) {
        constexpr std::array error_codes = {VK_ERROR_OUT_OF_HOST_MEMORY,VK_ERROR_OUT_OF_DEVICE_MEMORY};
        ValidateReturnCodes("vkCreateDescriptorUpdateTemplateKHR", result, error_codes, {});
    }
}

void BestPractices::PostCallRecordCreateRenderPass2KHR(
    VkDevice                                    device,
    const VkRenderPassCreateInfo2*              pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkRenderPass*                               pRenderPass,
    VkResult                                    result) {
    ValidationStateTracker::PostCallRecordCreateRenderPass2KHR(device, pCreateInfo, pAllocator, pRenderPass, result);
    if (result != VK_SUCCESS) {
        constexpr std::array error_codes = {VK_ERROR_OUT_OF_HOST_MEMORY,VK_ERROR_OUT_OF_DEVICE_MEMORY};
        ValidateReturnCodes("vkCreateRenderPass2KHR", result, error_codes, {});
    }
}

void BestPractices::PostCallRecordGetSwapchainStatusKHR(
    VkDevice                                    device,
    VkSwapchainKHR                              swapchain,
    VkResult                                    result) {
    ValidationStateTracker::PostCallRecordGetSwapchainStatusKHR(device, swapchain, result);
    if (result != VK_SUCCESS) {
        constexpr std::array error_codes = {VK_ERROR_OUT_OF_HOST_MEMORY,VK_ERROR_OUT_OF_DEVICE_MEMORY,VK_ERROR_DEVICE_LOST,VK_ERROR_OUT_OF_DATE_KHR,VK_ERROR_SURFACE_LOST_KHR,VK_ERROR_FULL_SCREEN_EXCLUSIVE_MODE_LOST_EXT};
        constexpr std::array success_codes = {VK_SUBOPTIMAL_KHR};
        ValidateReturnCodes("vkGetSwapchainStatusKHR", result, error_codes, success_codes);
    }
}

#ifdef VK_USE_PLATFORM_WIN32_KHR

void BestPractices::PostCallRecordImportFenceWin32HandleKHR(
    VkDevice                                    device,
    const VkImportFenceWin32HandleInfoKHR*      pImportFenceWin32HandleInfo,
    VkResult                                    result) {
    ValidationStateTracker::PostCallRecordImportFenceWin32HandleKHR(device, pImportFenceWin32HandleInfo, result);
    if (result != VK_SUCCESS) {
        constexpr std::array error_codes = {VK_ERROR_OUT_OF_HOST_MEMORY,VK_ERROR_INVALID_EXTERNAL_HANDLE};
        ValidateReturnCodes("vkImportFenceWin32HandleKHR", result, error_codes, {});
    }
}

#endif // VK_USE_PLATFORM_WIN32_KHR

#ifdef VK_USE_PLATFORM_WIN32_KHR

void BestPractices::PostCallRecordGetFenceWin32HandleKHR(
    VkDevice                                    device,
    const VkFenceGetWin32HandleInfoKHR*         pGetWin32HandleInfo,
    HANDLE*                                     pHandle,
    VkResult                                    result) {
    ValidationStateTracker::PostCallRecordGetFenceWin32HandleKHR(device, pGetWin32HandleInfo, pHandle, result);
    if (result != VK_SUCCESS) {
        constexpr std::array error_codes = {VK_ERROR_TOO_MANY_OBJECTS,VK_ERROR_OUT_OF_HOST_MEMORY};
        ValidateReturnCodes("vkGetFenceWin32HandleKHR", result, error_codes, {});
    }
}

#endif // VK_USE_PLATFORM_WIN32_KHR

void BestPractices::PostCallRecordImportFenceFdKHR(
    VkDevice                                    device,
    const VkImportFenceFdInfoKHR*               pImportFenceFdInfo,
    VkResult                                    result) {
    ValidationStateTracker::PostCallRecordImportFenceFdKHR(device, pImportFenceFdInfo, result);
    if (result != VK_SUCCESS) {
        constexpr std::array error_codes = {VK_ERROR_OUT_OF_HOST_MEMORY,VK_ERROR_INVALID_EXTERNAL_HANDLE};
        ValidateReturnCodes("vkImportFenceFdKHR", result, error_codes, {});
    }
}

void BestPractices::PostCallRecordGetFenceFdKHR(
    VkDevice                                    device,
    const VkFenceGetFdInfoKHR*                  pGetFdInfo,
    int*                                        pFd,
    VkResult                                    result) {
    ValidationStateTracker::PostCallRecordGetFenceFdKHR(device, pGetFdInfo, pFd, result);
    if (result != VK_SUCCESS) {
        constexpr std::array error_codes = {VK_ERROR_TOO_MANY_OBJECTS,VK_ERROR_OUT_OF_HOST_MEMORY};
        ValidateReturnCodes("vkGetFenceFdKHR", result, error_codes, {});
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
        constexpr std::array error_codes = {VK_ERROR_OUT_OF_HOST_MEMORY,VK_ERROR_OUT_OF_DEVICE_MEMORY,VK_ERROR_INITIALIZATION_FAILED};
        constexpr std::array success_codes = {VK_INCOMPLETE};
        ValidateReturnCodes("vkEnumeratePhysicalDeviceQueueFamilyPerformanceQueryCountersKHR", result, error_codes, success_codes);
    }
}

void BestPractices::PostCallRecordAcquireProfilingLockKHR(
    VkDevice                                    device,
    const VkAcquireProfilingLockInfoKHR*        pInfo,
    VkResult                                    result) {
    ValidationStateTracker::PostCallRecordAcquireProfilingLockKHR(device, pInfo, result);
    if (result != VK_SUCCESS) {
        constexpr std::array error_codes = {VK_ERROR_OUT_OF_HOST_MEMORY,VK_TIMEOUT};
        ValidateReturnCodes("vkAcquireProfilingLockKHR", result, error_codes, {});
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
        constexpr std::array error_codes = {VK_ERROR_OUT_OF_HOST_MEMORY,VK_ERROR_OUT_OF_DEVICE_MEMORY,VK_ERROR_SURFACE_LOST_KHR};
        ValidateReturnCodes("vkGetPhysicalDeviceSurfaceCapabilities2KHR", result, error_codes, {});
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
        constexpr std::array error_codes = {VK_ERROR_OUT_OF_HOST_MEMORY,VK_ERROR_OUT_OF_DEVICE_MEMORY,VK_ERROR_SURFACE_LOST_KHR};
        constexpr std::array success_codes = {VK_INCOMPLETE};
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
        constexpr std::array error_codes = {VK_ERROR_OUT_OF_HOST_MEMORY,VK_ERROR_OUT_OF_DEVICE_MEMORY};
        constexpr std::array success_codes = {VK_INCOMPLETE};
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
        constexpr std::array error_codes = {VK_ERROR_OUT_OF_HOST_MEMORY,VK_ERROR_OUT_OF_DEVICE_MEMORY};
        constexpr std::array success_codes = {VK_INCOMPLETE};
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
        constexpr std::array error_codes = {VK_ERROR_OUT_OF_HOST_MEMORY,VK_ERROR_OUT_OF_DEVICE_MEMORY};
        constexpr std::array success_codes = {VK_INCOMPLETE};
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
        constexpr std::array error_codes = {VK_ERROR_OUT_OF_HOST_MEMORY,VK_ERROR_OUT_OF_DEVICE_MEMORY};
        ValidateReturnCodes("vkGetDisplayPlaneCapabilities2KHR", result, error_codes, {});
    }
}

void BestPractices::PostCallRecordCreateSamplerYcbcrConversionKHR(
    VkDevice                                    device,
    const VkSamplerYcbcrConversionCreateInfo*   pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkSamplerYcbcrConversion*                   pYcbcrConversion,
    VkResult                                    result) {
    ValidationStateTracker::PostCallRecordCreateSamplerYcbcrConversionKHR(device, pCreateInfo, pAllocator, pYcbcrConversion, result);
    if (result != VK_SUCCESS) {
        constexpr std::array error_codes = {VK_ERROR_OUT_OF_HOST_MEMORY,VK_ERROR_OUT_OF_DEVICE_MEMORY};
        ValidateReturnCodes("vkCreateSamplerYcbcrConversionKHR", result, error_codes, {});
    }
}

void BestPractices::PostCallRecordBindBufferMemory2KHR(
    VkDevice                                    device,
    uint32_t                                    bindInfoCount,
    const VkBindBufferMemoryInfo*               pBindInfos,
    VkResult                                    result) {
    ValidationStateTracker::PostCallRecordBindBufferMemory2KHR(device, bindInfoCount, pBindInfos, result);
    if (result != VK_SUCCESS) {
        constexpr std::array error_codes = {VK_ERROR_OUT_OF_HOST_MEMORY,VK_ERROR_OUT_OF_DEVICE_MEMORY,VK_ERROR_INVALID_OPAQUE_CAPTURE_ADDRESS_KHR};
        ValidateReturnCodes("vkBindBufferMemory2KHR", result, error_codes, {});
    }
}

void BestPractices::PostCallRecordBindImageMemory2KHR(
    VkDevice                                    device,
    uint32_t                                    bindInfoCount,
    const VkBindImageMemoryInfo*                pBindInfos,
    VkResult                                    result) {
    ValidationStateTracker::PostCallRecordBindImageMemory2KHR(device, bindInfoCount, pBindInfos, result);
    if (result != VK_SUCCESS) {
        constexpr std::array error_codes = {VK_ERROR_OUT_OF_HOST_MEMORY,VK_ERROR_OUT_OF_DEVICE_MEMORY};
        ValidateReturnCodes("vkBindImageMemory2KHR", result, error_codes, {});
    }
}

void BestPractices::PostCallRecordGetSemaphoreCounterValueKHR(
    VkDevice                                    device,
    VkSemaphore                                 semaphore,
    uint64_t*                                   pValue,
    VkResult                                    result) {
    ValidationStateTracker::PostCallRecordGetSemaphoreCounterValueKHR(device, semaphore, pValue, result);
    if (result != VK_SUCCESS) {
        constexpr std::array error_codes = {VK_ERROR_OUT_OF_HOST_MEMORY,VK_ERROR_OUT_OF_DEVICE_MEMORY,VK_ERROR_DEVICE_LOST};
        ValidateReturnCodes("vkGetSemaphoreCounterValueKHR", result, error_codes, {});
    }
}

void BestPractices::PostCallRecordWaitSemaphoresKHR(
    VkDevice                                    device,
    const VkSemaphoreWaitInfo*                  pWaitInfo,
    uint64_t                                    timeout,
    VkResult                                    result) {
    ValidationStateTracker::PostCallRecordWaitSemaphoresKHR(device, pWaitInfo, timeout, result);
    if (result != VK_SUCCESS) {
        constexpr std::array error_codes = {VK_ERROR_OUT_OF_HOST_MEMORY,VK_ERROR_OUT_OF_DEVICE_MEMORY,VK_ERROR_DEVICE_LOST};
        constexpr std::array success_codes = {VK_TIMEOUT};
        ValidateReturnCodes("vkWaitSemaphoresKHR", result, error_codes, success_codes);
    }
}

void BestPractices::PostCallRecordSignalSemaphoreKHR(
    VkDevice                                    device,
    const VkSemaphoreSignalInfo*                pSignalInfo,
    VkResult                                    result) {
    ValidationStateTracker::PostCallRecordSignalSemaphoreKHR(device, pSignalInfo, result);
    if (result != VK_SUCCESS) {
        constexpr std::array error_codes = {VK_ERROR_OUT_OF_HOST_MEMORY,VK_ERROR_OUT_OF_DEVICE_MEMORY};
        ValidateReturnCodes("vkSignalSemaphoreKHR", result, error_codes, {});
    }
}

void BestPractices::PostCallRecordGetPhysicalDeviceFragmentShadingRatesKHR(
    VkPhysicalDevice                            physicalDevice,
    uint32_t*                                   pFragmentShadingRateCount,
    VkPhysicalDeviceFragmentShadingRateKHR*     pFragmentShadingRates,
    VkResult                                    result) {
    ValidationStateTracker::PostCallRecordGetPhysicalDeviceFragmentShadingRatesKHR(physicalDevice, pFragmentShadingRateCount, pFragmentShadingRates, result);
    if (result != VK_SUCCESS) {
        constexpr std::array error_codes = {VK_ERROR_OUT_OF_HOST_MEMORY};
        constexpr std::array success_codes = {VK_INCOMPLETE};
        ValidateReturnCodes("vkGetPhysicalDeviceFragmentShadingRatesKHR", result, error_codes, success_codes);
    }
}

void BestPractices::PostCallRecordWaitForPresentKHR(
    VkDevice                                    device,
    VkSwapchainKHR                              swapchain,
    uint64_t                                    presentId,
    uint64_t                                    timeout,
    VkResult                                    result) {
    ValidationStateTracker::PostCallRecordWaitForPresentKHR(device, swapchain, presentId, timeout, result);
    if (result != VK_SUCCESS) {
        constexpr std::array error_codes = {VK_ERROR_OUT_OF_HOST_MEMORY,VK_ERROR_OUT_OF_DEVICE_MEMORY,VK_ERROR_DEVICE_LOST,VK_ERROR_OUT_OF_DATE_KHR,VK_ERROR_SURFACE_LOST_KHR,VK_ERROR_FULL_SCREEN_EXCLUSIVE_MODE_LOST_EXT};
        constexpr std::array success_codes = {VK_TIMEOUT,VK_SUBOPTIMAL_KHR};
        ValidateReturnCodes("vkWaitForPresentKHR", result, error_codes, success_codes);
    }
}

void BestPractices::PostCallRecordCreateDeferredOperationKHR(
    VkDevice                                    device,
    const VkAllocationCallbacks*                pAllocator,
    VkDeferredOperationKHR*                     pDeferredOperation,
    VkResult                                    result) {
    ValidationStateTracker::PostCallRecordCreateDeferredOperationKHR(device, pAllocator, pDeferredOperation, result);
    if (result != VK_SUCCESS) {
        constexpr std::array error_codes = {VK_ERROR_OUT_OF_HOST_MEMORY};
        ValidateReturnCodes("vkCreateDeferredOperationKHR", result, error_codes, {});
    }
}

void BestPractices::PostCallRecordGetDeferredOperationResultKHR(
    VkDevice                                    device,
    VkDeferredOperationKHR                      operation,
    VkResult                                    result) {
    ValidationStateTracker::PostCallRecordGetDeferredOperationResultKHR(device, operation, result);
    if (result != VK_SUCCESS) {
        constexpr std::array success_codes = {VK_NOT_READY};
        ValidateReturnCodes("vkGetDeferredOperationResultKHR", result, {}, success_codes);
    }
}

void BestPractices::PostCallRecordDeferredOperationJoinKHR(
    VkDevice                                    device,
    VkDeferredOperationKHR                      operation,
    VkResult                                    result) {
    ValidationStateTracker::PostCallRecordDeferredOperationJoinKHR(device, operation, result);
    if (result != VK_SUCCESS) {
        constexpr std::array error_codes = {VK_ERROR_OUT_OF_HOST_MEMORY,VK_ERROR_OUT_OF_DEVICE_MEMORY};
        constexpr std::array success_codes = {VK_THREAD_DONE_KHR,VK_THREAD_IDLE_KHR};
        ValidateReturnCodes("vkDeferredOperationJoinKHR", result, error_codes, success_codes);
    }
}

void BestPractices::PostCallRecordGetPipelineExecutablePropertiesKHR(
    VkDevice                                    device,
    const VkPipelineInfoKHR*                    pPipelineInfo,
    uint32_t*                                   pExecutableCount,
    VkPipelineExecutablePropertiesKHR*          pProperties,
    VkResult                                    result) {
    ValidationStateTracker::PostCallRecordGetPipelineExecutablePropertiesKHR(device, pPipelineInfo, pExecutableCount, pProperties, result);
    if (result != VK_SUCCESS) {
        constexpr std::array error_codes = {VK_ERROR_OUT_OF_HOST_MEMORY,VK_ERROR_OUT_OF_DEVICE_MEMORY};
        constexpr std::array success_codes = {VK_INCOMPLETE};
        ValidateReturnCodes("vkGetPipelineExecutablePropertiesKHR", result, error_codes, success_codes);
    }
}

void BestPractices::PostCallRecordGetPipelineExecutableStatisticsKHR(
    VkDevice                                    device,
    const VkPipelineExecutableInfoKHR*          pExecutableInfo,
    uint32_t*                                   pStatisticCount,
    VkPipelineExecutableStatisticKHR*           pStatistics,
    VkResult                                    result) {
    ValidationStateTracker::PostCallRecordGetPipelineExecutableStatisticsKHR(device, pExecutableInfo, pStatisticCount, pStatistics, result);
    if (result != VK_SUCCESS) {
        constexpr std::array error_codes = {VK_ERROR_OUT_OF_HOST_MEMORY,VK_ERROR_OUT_OF_DEVICE_MEMORY};
        constexpr std::array success_codes = {VK_INCOMPLETE};
        ValidateReturnCodes("vkGetPipelineExecutableStatisticsKHR", result, error_codes, success_codes);
    }
}

void BestPractices::PostCallRecordGetPipelineExecutableInternalRepresentationsKHR(
    VkDevice                                    device,
    const VkPipelineExecutableInfoKHR*          pExecutableInfo,
    uint32_t*                                   pInternalRepresentationCount,
    VkPipelineExecutableInternalRepresentationKHR* pInternalRepresentations,
    VkResult                                    result) {
    ValidationStateTracker::PostCallRecordGetPipelineExecutableInternalRepresentationsKHR(device, pExecutableInfo, pInternalRepresentationCount, pInternalRepresentations, result);
    if (result != VK_SUCCESS) {
        constexpr std::array error_codes = {VK_ERROR_OUT_OF_HOST_MEMORY,VK_ERROR_OUT_OF_DEVICE_MEMORY};
        constexpr std::array success_codes = {VK_INCOMPLETE};
        ValidateReturnCodes("vkGetPipelineExecutableInternalRepresentationsKHR", result, error_codes, success_codes);
    }
}

void BestPractices::PostCallRecordQueueSubmit2KHR(
    VkQueue                                     queue,
    uint32_t                                    submitCount,
    const VkSubmitInfo2*                        pSubmits,
    VkFence                                     fence,
    VkResult                                    result) {
    ValidationStateTracker::PostCallRecordQueueSubmit2KHR(queue, submitCount, pSubmits, fence, result);
    if (result != VK_SUCCESS) {
        constexpr std::array error_codes = {VK_ERROR_OUT_OF_HOST_MEMORY,VK_ERROR_OUT_OF_DEVICE_MEMORY,VK_ERROR_DEVICE_LOST};
        ValidateReturnCodes("vkQueueSubmit2KHR", result, error_codes, {});
    }
}

void BestPractices::PostCallRecordCreateDebugReportCallbackEXT(
    VkInstance                                  instance,
    const VkDebugReportCallbackCreateInfoEXT*   pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkDebugReportCallbackEXT*                   pCallback,
    VkResult                                    result) {
    ValidationStateTracker::PostCallRecordCreateDebugReportCallbackEXT(instance, pCreateInfo, pAllocator, pCallback, result);
    if (result != VK_SUCCESS) {
        constexpr std::array error_codes = {VK_ERROR_OUT_OF_HOST_MEMORY};
        ValidateReturnCodes("vkCreateDebugReportCallbackEXT", result, error_codes, {});
    }
}

void BestPractices::PostCallRecordDebugMarkerSetObjectTagEXT(
    VkDevice                                    device,
    const VkDebugMarkerObjectTagInfoEXT*        pTagInfo,
    VkResult                                    result) {
    ValidationStateTracker::PostCallRecordDebugMarkerSetObjectTagEXT(device, pTagInfo, result);
    if (result != VK_SUCCESS) {
        constexpr std::array error_codes = {VK_ERROR_OUT_OF_HOST_MEMORY,VK_ERROR_OUT_OF_DEVICE_MEMORY};
        ValidateReturnCodes("vkDebugMarkerSetObjectTagEXT", result, error_codes, {});
    }
}

void BestPractices::PostCallRecordDebugMarkerSetObjectNameEXT(
    VkDevice                                    device,
    const VkDebugMarkerObjectNameInfoEXT*       pNameInfo,
    VkResult                                    result) {
    ValidationStateTracker::PostCallRecordDebugMarkerSetObjectNameEXT(device, pNameInfo, result);
    if (result != VK_SUCCESS) {
        constexpr std::array error_codes = {VK_ERROR_OUT_OF_HOST_MEMORY,VK_ERROR_OUT_OF_DEVICE_MEMORY};
        ValidateReturnCodes("vkDebugMarkerSetObjectNameEXT", result, error_codes, {});
    }
}

void BestPractices::PostCallRecordCreateCuModuleNVX(
    VkDevice                                    device,
    const VkCuModuleCreateInfoNVX*              pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkCuModuleNVX*                              pModule,
    VkResult                                    result) {
    ValidationStateTracker::PostCallRecordCreateCuModuleNVX(device, pCreateInfo, pAllocator, pModule, result);
    if (result != VK_SUCCESS) {
        constexpr std::array error_codes = {VK_ERROR_OUT_OF_HOST_MEMORY,VK_ERROR_INITIALIZATION_FAILED};
        ValidateReturnCodes("vkCreateCuModuleNVX", result, error_codes, {});
    }
}

void BestPractices::PostCallRecordCreateCuFunctionNVX(
    VkDevice                                    device,
    const VkCuFunctionCreateInfoNVX*            pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkCuFunctionNVX*                            pFunction,
    VkResult                                    result) {
    ValidationStateTracker::PostCallRecordCreateCuFunctionNVX(device, pCreateInfo, pAllocator, pFunction, result);
    if (result != VK_SUCCESS) {
        constexpr std::array error_codes = {VK_ERROR_OUT_OF_HOST_MEMORY,VK_ERROR_INITIALIZATION_FAILED};
        ValidateReturnCodes("vkCreateCuFunctionNVX", result, error_codes, {});
    }
}

void BestPractices::PostCallRecordGetImageViewAddressNVX(
    VkDevice                                    device,
    VkImageView                                 imageView,
    VkImageViewAddressPropertiesNVX*            pProperties,
    VkResult                                    result) {
    ValidationStateTracker::PostCallRecordGetImageViewAddressNVX(device, imageView, pProperties, result);
    if (result != VK_SUCCESS) {
        constexpr std::array error_codes = {VK_ERROR_OUT_OF_HOST_MEMORY,VK_ERROR_UNKNOWN};
        ValidateReturnCodes("vkGetImageViewAddressNVX", result, error_codes, {});
    }
}

void BestPractices::PostCallRecordGetShaderInfoAMD(
    VkDevice                                    device,
    VkPipeline                                  pipeline,
    VkShaderStageFlagBits                       shaderStage,
    VkShaderInfoTypeAMD                         infoType,
    size_t*                                     pInfoSize,
    void*                                       pInfo,
    VkResult                                    result) {
    ValidationStateTracker::PostCallRecordGetShaderInfoAMD(device, pipeline, shaderStage, infoType, pInfoSize, pInfo, result);
    if (result != VK_SUCCESS) {
        constexpr std::array error_codes = {VK_ERROR_FEATURE_NOT_PRESENT,VK_ERROR_OUT_OF_HOST_MEMORY};
        constexpr std::array success_codes = {VK_INCOMPLETE};
        ValidateReturnCodes("vkGetShaderInfoAMD", result, error_codes, success_codes);
    }
}

#ifdef VK_USE_PLATFORM_GGP

void BestPractices::PostCallRecordCreateStreamDescriptorSurfaceGGP(
    VkInstance                                  instance,
    const VkStreamDescriptorSurfaceCreateInfoGGP* pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkSurfaceKHR*                               pSurface,
    VkResult                                    result) {
    ValidationStateTracker::PostCallRecordCreateStreamDescriptorSurfaceGGP(instance, pCreateInfo, pAllocator, pSurface, result);
    if (result != VK_SUCCESS) {
        constexpr std::array error_codes = {VK_ERROR_OUT_OF_HOST_MEMORY,VK_ERROR_OUT_OF_DEVICE_MEMORY,VK_ERROR_NATIVE_WINDOW_IN_USE_KHR};
        ValidateReturnCodes("vkCreateStreamDescriptorSurfaceGGP", result, error_codes, {});
    }
}

#endif // VK_USE_PLATFORM_GGP

void BestPractices::PostCallRecordGetPhysicalDeviceExternalImageFormatPropertiesNV(
    VkPhysicalDevice                            physicalDevice,
    VkFormat                                    format,
    VkImageType                                 type,
    VkImageTiling                               tiling,
    VkImageUsageFlags                           usage,
    VkImageCreateFlags                          flags,
    VkExternalMemoryHandleTypeFlagsNV           externalHandleType,
    VkExternalImageFormatPropertiesNV*          pExternalImageFormatProperties,
    VkResult                                    result) {
    ValidationStateTracker::PostCallRecordGetPhysicalDeviceExternalImageFormatPropertiesNV(physicalDevice, format, type, tiling, usage, flags, externalHandleType, pExternalImageFormatProperties, result);
    if (result != VK_SUCCESS) {
        constexpr std::array error_codes = {VK_ERROR_OUT_OF_HOST_MEMORY,VK_ERROR_OUT_OF_DEVICE_MEMORY,VK_ERROR_FORMAT_NOT_SUPPORTED};
        ValidateReturnCodes("vkGetPhysicalDeviceExternalImageFormatPropertiesNV", result, error_codes, {});
    }
}

#ifdef VK_USE_PLATFORM_WIN32_KHR

void BestPractices::PostCallRecordGetMemoryWin32HandleNV(
    VkDevice                                    device,
    VkDeviceMemory                              memory,
    VkExternalMemoryHandleTypeFlagsNV           handleType,
    HANDLE*                                     pHandle,
    VkResult                                    result) {
    ValidationStateTracker::PostCallRecordGetMemoryWin32HandleNV(device, memory, handleType, pHandle, result);
    if (result != VK_SUCCESS) {
        constexpr std::array error_codes = {VK_ERROR_TOO_MANY_OBJECTS,VK_ERROR_OUT_OF_HOST_MEMORY};
        ValidateReturnCodes("vkGetMemoryWin32HandleNV", result, error_codes, {});
    }
}

#endif // VK_USE_PLATFORM_WIN32_KHR

#ifdef VK_USE_PLATFORM_VI_NN

void BestPractices::PostCallRecordCreateViSurfaceNN(
    VkInstance                                  instance,
    const VkViSurfaceCreateInfoNN*              pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkSurfaceKHR*                               pSurface,
    VkResult                                    result) {
    ValidationStateTracker::PostCallRecordCreateViSurfaceNN(instance, pCreateInfo, pAllocator, pSurface, result);
    if (result != VK_SUCCESS) {
        constexpr std::array error_codes = {VK_ERROR_OUT_OF_HOST_MEMORY,VK_ERROR_OUT_OF_DEVICE_MEMORY,VK_ERROR_NATIVE_WINDOW_IN_USE_KHR};
        ValidateReturnCodes("vkCreateViSurfaceNN", result, error_codes, {});
    }
}

#endif // VK_USE_PLATFORM_VI_NN

#ifdef VK_USE_PLATFORM_XLIB_XRANDR_EXT

void BestPractices::PostCallRecordAcquireXlibDisplayEXT(
    VkPhysicalDevice                            physicalDevice,
    Display*                                    dpy,
    VkDisplayKHR                                display,
    VkResult                                    result) {
    ValidationStateTracker::PostCallRecordAcquireXlibDisplayEXT(physicalDevice, dpy, display, result);
    if (result != VK_SUCCESS) {
        constexpr std::array error_codes = {VK_ERROR_OUT_OF_HOST_MEMORY,VK_ERROR_INITIALIZATION_FAILED};
        ValidateReturnCodes("vkAcquireXlibDisplayEXT", result, error_codes, {});
    }
}

#endif // VK_USE_PLATFORM_XLIB_XRANDR_EXT

#ifdef VK_USE_PLATFORM_XLIB_XRANDR_EXT

void BestPractices::PostCallRecordGetRandROutputDisplayEXT(
    VkPhysicalDevice                            physicalDevice,
    Display*                                    dpy,
    RROutput                                    rrOutput,
    VkDisplayKHR*                               pDisplay,
    VkResult                                    result) {
    ValidationStateTracker::PostCallRecordGetRandROutputDisplayEXT(physicalDevice, dpy, rrOutput, pDisplay, result);
    if (result != VK_SUCCESS) {
        constexpr std::array error_codes = {VK_ERROR_OUT_OF_HOST_MEMORY};
        ValidateReturnCodes("vkGetRandROutputDisplayEXT", result, error_codes, {});
    }
}

#endif // VK_USE_PLATFORM_XLIB_XRANDR_EXT

void BestPractices::PostCallRecordGetPhysicalDeviceSurfaceCapabilities2EXT(
    VkPhysicalDevice                            physicalDevice,
    VkSurfaceKHR                                surface,
    VkSurfaceCapabilities2EXT*                  pSurfaceCapabilities,
    VkResult                                    result) {
    ValidationStateTracker::PostCallRecordGetPhysicalDeviceSurfaceCapabilities2EXT(physicalDevice, surface, pSurfaceCapabilities, result);
    ManualPostCallRecordGetPhysicalDeviceSurfaceCapabilities2EXT(physicalDevice, surface, pSurfaceCapabilities, result);
    if (result != VK_SUCCESS) {
        constexpr std::array error_codes = {VK_ERROR_OUT_OF_HOST_MEMORY,VK_ERROR_OUT_OF_DEVICE_MEMORY,VK_ERROR_SURFACE_LOST_KHR};
        ValidateReturnCodes("vkGetPhysicalDeviceSurfaceCapabilities2EXT", result, error_codes, {});
    }
}

void BestPractices::PostCallRecordDisplayPowerControlEXT(
    VkDevice                                    device,
    VkDisplayKHR                                display,
    const VkDisplayPowerInfoEXT*                pDisplayPowerInfo,
    VkResult                                    result) {
    ValidationStateTracker::PostCallRecordDisplayPowerControlEXT(device, display, pDisplayPowerInfo, result);
    if (result != VK_SUCCESS) {
        constexpr std::array error_codes = {VK_ERROR_OUT_OF_HOST_MEMORY};
        ValidateReturnCodes("vkDisplayPowerControlEXT", result, error_codes, {});
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
        constexpr std::array error_codes = {VK_ERROR_OUT_OF_HOST_MEMORY};
        ValidateReturnCodes("vkRegisterDeviceEventEXT", result, error_codes, {});
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
        constexpr std::array error_codes = {VK_ERROR_OUT_OF_HOST_MEMORY};
        ValidateReturnCodes("vkRegisterDisplayEventEXT", result, error_codes, {});
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
        constexpr std::array error_codes = {VK_ERROR_OUT_OF_HOST_MEMORY,VK_ERROR_DEVICE_LOST,VK_ERROR_OUT_OF_DATE_KHR};
        ValidateReturnCodes("vkGetSwapchainCounterEXT", result, error_codes, {});
    }
}

void BestPractices::PostCallRecordGetRefreshCycleDurationGOOGLE(
    VkDevice                                    device,
    VkSwapchainKHR                              swapchain,
    VkRefreshCycleDurationGOOGLE*               pDisplayTimingProperties,
    VkResult                                    result) {
    ValidationStateTracker::PostCallRecordGetRefreshCycleDurationGOOGLE(device, swapchain, pDisplayTimingProperties, result);
    if (result != VK_SUCCESS) {
        constexpr std::array error_codes = {VK_ERROR_OUT_OF_HOST_MEMORY,VK_ERROR_DEVICE_LOST,VK_ERROR_SURFACE_LOST_KHR};
        ValidateReturnCodes("vkGetRefreshCycleDurationGOOGLE", result, error_codes, {});
    }
}

void BestPractices::PostCallRecordGetPastPresentationTimingGOOGLE(
    VkDevice                                    device,
    VkSwapchainKHR                              swapchain,
    uint32_t*                                   pPresentationTimingCount,
    VkPastPresentationTimingGOOGLE*             pPresentationTimings,
    VkResult                                    result) {
    ValidationStateTracker::PostCallRecordGetPastPresentationTimingGOOGLE(device, swapchain, pPresentationTimingCount, pPresentationTimings, result);
    if (result != VK_SUCCESS) {
        constexpr std::array error_codes = {VK_ERROR_OUT_OF_HOST_MEMORY,VK_ERROR_DEVICE_LOST,VK_ERROR_OUT_OF_DATE_KHR,VK_ERROR_SURFACE_LOST_KHR};
        constexpr std::array success_codes = {VK_INCOMPLETE};
        ValidateReturnCodes("vkGetPastPresentationTimingGOOGLE", result, error_codes, success_codes);
    }
}

#ifdef VK_USE_PLATFORM_IOS_MVK

void BestPractices::PostCallRecordCreateIOSSurfaceMVK(
    VkInstance                                  instance,
    const VkIOSSurfaceCreateInfoMVK*            pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkSurfaceKHR*                               pSurface,
    VkResult                                    result) {
    ValidationStateTracker::PostCallRecordCreateIOSSurfaceMVK(instance, pCreateInfo, pAllocator, pSurface, result);
    if (result != VK_SUCCESS) {
        constexpr std::array error_codes = {VK_ERROR_OUT_OF_HOST_MEMORY,VK_ERROR_OUT_OF_DEVICE_MEMORY,VK_ERROR_NATIVE_WINDOW_IN_USE_KHR};
        ValidateReturnCodes("vkCreateIOSSurfaceMVK", result, error_codes, {});
    }
}

#endif // VK_USE_PLATFORM_IOS_MVK

#ifdef VK_USE_PLATFORM_MACOS_MVK

void BestPractices::PostCallRecordCreateMacOSSurfaceMVK(
    VkInstance                                  instance,
    const VkMacOSSurfaceCreateInfoMVK*          pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkSurfaceKHR*                               pSurface,
    VkResult                                    result) {
    ValidationStateTracker::PostCallRecordCreateMacOSSurfaceMVK(instance, pCreateInfo, pAllocator, pSurface, result);
    if (result != VK_SUCCESS) {
        constexpr std::array error_codes = {VK_ERROR_OUT_OF_HOST_MEMORY,VK_ERROR_OUT_OF_DEVICE_MEMORY,VK_ERROR_NATIVE_WINDOW_IN_USE_KHR};
        ValidateReturnCodes("vkCreateMacOSSurfaceMVK", result, error_codes, {});
    }
}

#endif // VK_USE_PLATFORM_MACOS_MVK

void BestPractices::PostCallRecordSetDebugUtilsObjectNameEXT(
    VkDevice                                    device,
    const VkDebugUtilsObjectNameInfoEXT*        pNameInfo,
    VkResult                                    result) {
    ValidationStateTracker::PostCallRecordSetDebugUtilsObjectNameEXT(device, pNameInfo, result);
    if (result != VK_SUCCESS) {
        constexpr std::array error_codes = {VK_ERROR_OUT_OF_HOST_MEMORY,VK_ERROR_OUT_OF_DEVICE_MEMORY};
        ValidateReturnCodes("vkSetDebugUtilsObjectNameEXT", result, error_codes, {});
    }
}

void BestPractices::PostCallRecordSetDebugUtilsObjectTagEXT(
    VkDevice                                    device,
    const VkDebugUtilsObjectTagInfoEXT*         pTagInfo,
    VkResult                                    result) {
    ValidationStateTracker::PostCallRecordSetDebugUtilsObjectTagEXT(device, pTagInfo, result);
    if (result != VK_SUCCESS) {
        constexpr std::array error_codes = {VK_ERROR_OUT_OF_HOST_MEMORY,VK_ERROR_OUT_OF_DEVICE_MEMORY};
        ValidateReturnCodes("vkSetDebugUtilsObjectTagEXT", result, error_codes, {});
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
        constexpr std::array error_codes = {VK_ERROR_OUT_OF_HOST_MEMORY};
        ValidateReturnCodes("vkCreateDebugUtilsMessengerEXT", result, error_codes, {});
    }
}

#ifdef VK_USE_PLATFORM_ANDROID_KHR

void BestPractices::PostCallRecordGetAndroidHardwareBufferPropertiesANDROID(
    VkDevice                                    device,
    const struct AHardwareBuffer*               buffer,
    VkAndroidHardwareBufferPropertiesANDROID*   pProperties,
    VkResult                                    result) {
    ValidationStateTracker::PostCallRecordGetAndroidHardwareBufferPropertiesANDROID(device, buffer, pProperties, result);
    if (result != VK_SUCCESS) {
        constexpr std::array error_codes = {VK_ERROR_OUT_OF_HOST_MEMORY,VK_ERROR_INVALID_EXTERNAL_HANDLE_KHR};
        ValidateReturnCodes("vkGetAndroidHardwareBufferPropertiesANDROID", result, error_codes, {});
    }
}

#endif // VK_USE_PLATFORM_ANDROID_KHR

#ifdef VK_USE_PLATFORM_ANDROID_KHR

void BestPractices::PostCallRecordGetMemoryAndroidHardwareBufferANDROID(
    VkDevice                                    device,
    const VkMemoryGetAndroidHardwareBufferInfoANDROID* pInfo,
    struct AHardwareBuffer**                    pBuffer,
    VkResult                                    result) {
    ValidationStateTracker::PostCallRecordGetMemoryAndroidHardwareBufferANDROID(device, pInfo, pBuffer, result);
    if (result != VK_SUCCESS) {
        constexpr std::array error_codes = {VK_ERROR_TOO_MANY_OBJECTS,VK_ERROR_OUT_OF_HOST_MEMORY};
        ValidateReturnCodes("vkGetMemoryAndroidHardwareBufferANDROID", result, error_codes, {});
    }
}

#endif // VK_USE_PLATFORM_ANDROID_KHR

void BestPractices::PostCallRecordGetImageDrmFormatModifierPropertiesEXT(
    VkDevice                                    device,
    VkImage                                     image,
    VkImageDrmFormatModifierPropertiesEXT*      pProperties,
    VkResult                                    result) {
    ValidationStateTracker::PostCallRecordGetImageDrmFormatModifierPropertiesEXT(device, image, pProperties, result);
    if (result != VK_SUCCESS) {
        constexpr std::array error_codes = {VK_ERROR_OUT_OF_HOST_MEMORY};
        ValidateReturnCodes("vkGetImageDrmFormatModifierPropertiesEXT", result, error_codes, {});
    }
}

// Skipping vkCreateValidationCacheEXT for autogen as it has a manually created custom function or ignored.

// Skipping vkDestroyValidationCacheEXT for autogen as it has a manually created custom function or ignored.

// Skipping vkMergeValidationCachesEXT for autogen as it has a manually created custom function or ignored.

// Skipping vkGetValidationCacheDataEXT for autogen as it has a manually created custom function or ignored.

void BestPractices::PostCallRecordCreateAccelerationStructureNV(
    VkDevice                                    device,
    const VkAccelerationStructureCreateInfoNV*  pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkAccelerationStructureNV*                  pAccelerationStructure,
    VkResult                                    result) {
    ValidationStateTracker::PostCallRecordCreateAccelerationStructureNV(device, pCreateInfo, pAllocator, pAccelerationStructure, result);
    if (result != VK_SUCCESS) {
        constexpr std::array error_codes = {VK_ERROR_OUT_OF_HOST_MEMORY};
        ValidateReturnCodes("vkCreateAccelerationStructureNV", result, error_codes, {});
    }
}

void BestPractices::PostCallRecordBindAccelerationStructureMemoryNV(
    VkDevice                                    device,
    uint32_t                                    bindInfoCount,
    const VkBindAccelerationStructureMemoryInfoNV* pBindInfos,
    VkResult                                    result) {
    ValidationStateTracker::PostCallRecordBindAccelerationStructureMemoryNV(device, bindInfoCount, pBindInfos, result);
    if (result != VK_SUCCESS) {
        constexpr std::array error_codes = {VK_ERROR_OUT_OF_HOST_MEMORY,VK_ERROR_OUT_OF_DEVICE_MEMORY};
        ValidateReturnCodes("vkBindAccelerationStructureMemoryNV", result, error_codes, {});
    }
}

void BestPractices::PostCallRecordCreateRayTracingPipelinesNV(
    VkDevice                                    device,
    VkPipelineCache                             pipelineCache,
    uint32_t                                    createInfoCount,
    const VkRayTracingPipelineCreateInfoNV*     pCreateInfos,
    const VkAllocationCallbacks*                pAllocator,
    VkPipeline*                                 pPipelines,
    VkResult                                    result,
    void*                                       state_data) {
    ValidationStateTracker::PostCallRecordCreateRayTracingPipelinesNV(device, pipelineCache, createInfoCount, pCreateInfos, pAllocator, pPipelines, result, state_data);
    if (result != VK_SUCCESS) {
        constexpr std::array error_codes = {VK_ERROR_OUT_OF_HOST_MEMORY,VK_ERROR_OUT_OF_DEVICE_MEMORY,VK_ERROR_INVALID_SHADER_NV};
        constexpr std::array success_codes = {VK_PIPELINE_COMPILE_REQUIRED_EXT};
        ValidateReturnCodes("vkCreateRayTracingPipelinesNV", result, error_codes, success_codes);
    }
}

void BestPractices::PostCallRecordGetRayTracingShaderGroupHandlesKHR(
    VkDevice                                    device,
    VkPipeline                                  pipeline,
    uint32_t                                    firstGroup,
    uint32_t                                    groupCount,
    size_t                                      dataSize,
    void*                                       pData,
    VkResult                                    result) {
    ValidationStateTracker::PostCallRecordGetRayTracingShaderGroupHandlesKHR(device, pipeline, firstGroup, groupCount, dataSize, pData, result);
    if (result != VK_SUCCESS) {
        constexpr std::array error_codes = {VK_ERROR_OUT_OF_HOST_MEMORY,VK_ERROR_OUT_OF_DEVICE_MEMORY};
        ValidateReturnCodes("vkGetRayTracingShaderGroupHandlesKHR", result, error_codes, {});
    }
}

void BestPractices::PostCallRecordGetRayTracingShaderGroupHandlesNV(
    VkDevice                                    device,
    VkPipeline                                  pipeline,
    uint32_t                                    firstGroup,
    uint32_t                                    groupCount,
    size_t                                      dataSize,
    void*                                       pData,
    VkResult                                    result) {
    ValidationStateTracker::PostCallRecordGetRayTracingShaderGroupHandlesNV(device, pipeline, firstGroup, groupCount, dataSize, pData, result);
    if (result != VK_SUCCESS) {
        constexpr std::array error_codes = {VK_ERROR_OUT_OF_HOST_MEMORY,VK_ERROR_OUT_OF_DEVICE_MEMORY};
        ValidateReturnCodes("vkGetRayTracingShaderGroupHandlesNV", result, error_codes, {});
    }
}

void BestPractices::PostCallRecordGetAccelerationStructureHandleNV(
    VkDevice                                    device,
    VkAccelerationStructureNV                   accelerationStructure,
    size_t                                      dataSize,
    void*                                       pData,
    VkResult                                    result) {
    ValidationStateTracker::PostCallRecordGetAccelerationStructureHandleNV(device, accelerationStructure, dataSize, pData, result);
    if (result != VK_SUCCESS) {
        constexpr std::array error_codes = {VK_ERROR_OUT_OF_HOST_MEMORY,VK_ERROR_OUT_OF_DEVICE_MEMORY};
        ValidateReturnCodes("vkGetAccelerationStructureHandleNV", result, error_codes, {});
    }
}

void BestPractices::PostCallRecordCompileDeferredNV(
    VkDevice                                    device,
    VkPipeline                                  pipeline,
    uint32_t                                    shader,
    VkResult                                    result) {
    ValidationStateTracker::PostCallRecordCompileDeferredNV(device, pipeline, shader, result);
    if (result != VK_SUCCESS) {
        constexpr std::array error_codes = {VK_ERROR_OUT_OF_HOST_MEMORY,VK_ERROR_OUT_OF_DEVICE_MEMORY};
        ValidateReturnCodes("vkCompileDeferredNV", result, error_codes, {});
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
        constexpr std::array error_codes = {VK_ERROR_OUT_OF_HOST_MEMORY,VK_ERROR_INVALID_EXTERNAL_HANDLE};
        ValidateReturnCodes("vkGetMemoryHostPointerPropertiesEXT", result, error_codes, {});
    }
}

void BestPractices::PostCallRecordGetPhysicalDeviceCalibrateableTimeDomainsEXT(
    VkPhysicalDevice                            physicalDevice,
    uint32_t*                                   pTimeDomainCount,
    VkTimeDomainEXT*                            pTimeDomains,
    VkResult                                    result) {
    ValidationStateTracker::PostCallRecordGetPhysicalDeviceCalibrateableTimeDomainsEXT(physicalDevice, pTimeDomainCount, pTimeDomains, result);
    if (result != VK_SUCCESS) {
        constexpr std::array error_codes = {VK_ERROR_OUT_OF_HOST_MEMORY,VK_ERROR_OUT_OF_DEVICE_MEMORY};
        constexpr std::array success_codes = {VK_INCOMPLETE};
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
        constexpr std::array error_codes = {VK_ERROR_OUT_OF_HOST_MEMORY,VK_ERROR_OUT_OF_DEVICE_MEMORY};
        ValidateReturnCodes("vkGetCalibratedTimestampsEXT", result, error_codes, {});
    }
}

void BestPractices::PostCallRecordInitializePerformanceApiINTEL(
    VkDevice                                    device,
    const VkInitializePerformanceApiInfoINTEL*  pInitializeInfo,
    VkResult                                    result) {
    ValidationStateTracker::PostCallRecordInitializePerformanceApiINTEL(device, pInitializeInfo, result);
    if (result != VK_SUCCESS) {
        constexpr std::array error_codes = {VK_ERROR_TOO_MANY_OBJECTS,VK_ERROR_OUT_OF_HOST_MEMORY};
        ValidateReturnCodes("vkInitializePerformanceApiINTEL", result, error_codes, {});
    }
}

void BestPractices::PostCallRecordCmdSetPerformanceMarkerINTEL(
    VkCommandBuffer                             commandBuffer,
    const VkPerformanceMarkerInfoINTEL*         pMarkerInfo,
    VkResult                                    result) {
    ValidationStateTracker::PostCallRecordCmdSetPerformanceMarkerINTEL(commandBuffer, pMarkerInfo, result);
    if (result != VK_SUCCESS) {
        constexpr std::array error_codes = {VK_ERROR_TOO_MANY_OBJECTS,VK_ERROR_OUT_OF_HOST_MEMORY};
        ValidateReturnCodes("vkCmdSetPerformanceMarkerINTEL", result, error_codes, {});
    }
}

void BestPractices::PostCallRecordCmdSetPerformanceStreamMarkerINTEL(
    VkCommandBuffer                             commandBuffer,
    const VkPerformanceStreamMarkerInfoINTEL*   pMarkerInfo,
    VkResult                                    result) {
    ValidationStateTracker::PostCallRecordCmdSetPerformanceStreamMarkerINTEL(commandBuffer, pMarkerInfo, result);
    if (result != VK_SUCCESS) {
        constexpr std::array error_codes = {VK_ERROR_TOO_MANY_OBJECTS,VK_ERROR_OUT_OF_HOST_MEMORY};
        ValidateReturnCodes("vkCmdSetPerformanceStreamMarkerINTEL", result, error_codes, {});
    }
}

void BestPractices::PostCallRecordCmdSetPerformanceOverrideINTEL(
    VkCommandBuffer                             commandBuffer,
    const VkPerformanceOverrideInfoINTEL*       pOverrideInfo,
    VkResult                                    result) {
    ValidationStateTracker::PostCallRecordCmdSetPerformanceOverrideINTEL(commandBuffer, pOverrideInfo, result);
    if (result != VK_SUCCESS) {
        constexpr std::array error_codes = {VK_ERROR_TOO_MANY_OBJECTS,VK_ERROR_OUT_OF_HOST_MEMORY};
        ValidateReturnCodes("vkCmdSetPerformanceOverrideINTEL", result, error_codes, {});
    }
}

void BestPractices::PostCallRecordAcquirePerformanceConfigurationINTEL(
    VkDevice                                    device,
    const VkPerformanceConfigurationAcquireInfoINTEL* pAcquireInfo,
    VkPerformanceConfigurationINTEL*            pConfiguration,
    VkResult                                    result) {
    ValidationStateTracker::PostCallRecordAcquirePerformanceConfigurationINTEL(device, pAcquireInfo, pConfiguration, result);
    if (result != VK_SUCCESS) {
        constexpr std::array error_codes = {VK_ERROR_TOO_MANY_OBJECTS,VK_ERROR_OUT_OF_HOST_MEMORY};
        ValidateReturnCodes("vkAcquirePerformanceConfigurationINTEL", result, error_codes, {});
    }
}

void BestPractices::PostCallRecordReleasePerformanceConfigurationINTEL(
    VkDevice                                    device,
    VkPerformanceConfigurationINTEL             configuration,
    VkResult                                    result) {
    ValidationStateTracker::PostCallRecordReleasePerformanceConfigurationINTEL(device, configuration, result);
    if (result != VK_SUCCESS) {
        constexpr std::array error_codes = {VK_ERROR_TOO_MANY_OBJECTS,VK_ERROR_OUT_OF_HOST_MEMORY};
        ValidateReturnCodes("vkReleasePerformanceConfigurationINTEL", result, error_codes, {});
    }
}

void BestPractices::PostCallRecordQueueSetPerformanceConfigurationINTEL(
    VkQueue                                     queue,
    VkPerformanceConfigurationINTEL             configuration,
    VkResult                                    result) {
    ValidationStateTracker::PostCallRecordQueueSetPerformanceConfigurationINTEL(queue, configuration, result);
    if (result != VK_SUCCESS) {
        constexpr std::array error_codes = {VK_ERROR_TOO_MANY_OBJECTS,VK_ERROR_OUT_OF_HOST_MEMORY};
        ValidateReturnCodes("vkQueueSetPerformanceConfigurationINTEL", result, error_codes, {});
    }
}

void BestPractices::PostCallRecordGetPerformanceParameterINTEL(
    VkDevice                                    device,
    VkPerformanceParameterTypeINTEL             parameter,
    VkPerformanceValueINTEL*                    pValue,
    VkResult                                    result) {
    ValidationStateTracker::PostCallRecordGetPerformanceParameterINTEL(device, parameter, pValue, result);
    if (result != VK_SUCCESS) {
        constexpr std::array error_codes = {VK_ERROR_TOO_MANY_OBJECTS,VK_ERROR_OUT_OF_HOST_MEMORY};
        ValidateReturnCodes("vkGetPerformanceParameterINTEL", result, error_codes, {});
    }
}

#ifdef VK_USE_PLATFORM_FUCHSIA

void BestPractices::PostCallRecordCreateImagePipeSurfaceFUCHSIA(
    VkInstance                                  instance,
    const VkImagePipeSurfaceCreateInfoFUCHSIA*  pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkSurfaceKHR*                               pSurface,
    VkResult                                    result) {
    ValidationStateTracker::PostCallRecordCreateImagePipeSurfaceFUCHSIA(instance, pCreateInfo, pAllocator, pSurface, result);
    if (result != VK_SUCCESS) {
        constexpr std::array error_codes = {VK_ERROR_OUT_OF_HOST_MEMORY,VK_ERROR_OUT_OF_DEVICE_MEMORY};
        ValidateReturnCodes("vkCreateImagePipeSurfaceFUCHSIA", result, error_codes, {});
    }
}

#endif // VK_USE_PLATFORM_FUCHSIA

#ifdef VK_USE_PLATFORM_METAL_EXT

void BestPractices::PostCallRecordCreateMetalSurfaceEXT(
    VkInstance                                  instance,
    const VkMetalSurfaceCreateInfoEXT*          pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkSurfaceKHR*                               pSurface,
    VkResult                                    result) {
    ValidationStateTracker::PostCallRecordCreateMetalSurfaceEXT(instance, pCreateInfo, pAllocator, pSurface, result);
    if (result != VK_SUCCESS) {
        constexpr std::array error_codes = {VK_ERROR_OUT_OF_HOST_MEMORY,VK_ERROR_OUT_OF_DEVICE_MEMORY,VK_ERROR_NATIVE_WINDOW_IN_USE_KHR};
        ValidateReturnCodes("vkCreateMetalSurfaceEXT", result, error_codes, {});
    }
}

#endif // VK_USE_PLATFORM_METAL_EXT

void BestPractices::PostCallRecordGetPhysicalDeviceToolPropertiesEXT(
    VkPhysicalDevice                            physicalDevice,
    uint32_t*                                   pToolCount,
    VkPhysicalDeviceToolProperties*             pToolProperties,
    VkResult                                    result) {
    ValidationStateTracker::PostCallRecordGetPhysicalDeviceToolPropertiesEXT(physicalDevice, pToolCount, pToolProperties, result);
    if (result != VK_SUCCESS) {
        constexpr std::array error_codes = {VK_ERROR_OUT_OF_HOST_MEMORY};
        constexpr std::array success_codes = {VK_INCOMPLETE};
        ValidateReturnCodes("vkGetPhysicalDeviceToolPropertiesEXT", result, error_codes, success_codes);
    }
}

void BestPractices::PostCallRecordGetPhysicalDeviceCooperativeMatrixPropertiesNV(
    VkPhysicalDevice                            physicalDevice,
    uint32_t*                                   pPropertyCount,
    VkCooperativeMatrixPropertiesNV*            pProperties,
    VkResult                                    result) {
    ValidationStateTracker::PostCallRecordGetPhysicalDeviceCooperativeMatrixPropertiesNV(physicalDevice, pPropertyCount, pProperties, result);
    if (result != VK_SUCCESS) {
        constexpr std::array error_codes = {VK_ERROR_OUT_OF_HOST_MEMORY,VK_ERROR_OUT_OF_DEVICE_MEMORY};
        constexpr std::array success_codes = {VK_INCOMPLETE};
        ValidateReturnCodes("vkGetPhysicalDeviceCooperativeMatrixPropertiesNV", result, error_codes, success_codes);
    }
}

void BestPractices::PostCallRecordGetPhysicalDeviceSupportedFramebufferMixedSamplesCombinationsNV(
    VkPhysicalDevice                            physicalDevice,
    uint32_t*                                   pCombinationCount,
    VkFramebufferMixedSamplesCombinationNV*     pCombinations,
    VkResult                                    result) {
    ValidationStateTracker::PostCallRecordGetPhysicalDeviceSupportedFramebufferMixedSamplesCombinationsNV(physicalDevice, pCombinationCount, pCombinations, result);
    if (result != VK_SUCCESS) {
        constexpr std::array error_codes = {VK_ERROR_OUT_OF_HOST_MEMORY,VK_ERROR_OUT_OF_DEVICE_MEMORY};
        constexpr std::array success_codes = {VK_INCOMPLETE};
        ValidateReturnCodes("vkGetPhysicalDeviceSupportedFramebufferMixedSamplesCombinationsNV", result, error_codes, success_codes);
    }
}

#ifdef VK_USE_PLATFORM_WIN32_KHR

void BestPractices::PostCallRecordGetPhysicalDeviceSurfacePresentModes2EXT(
    VkPhysicalDevice                            physicalDevice,
    const VkPhysicalDeviceSurfaceInfo2KHR*      pSurfaceInfo,
    uint32_t*                                   pPresentModeCount,
    VkPresentModeKHR*                           pPresentModes,
    VkResult                                    result) {
    ValidationStateTracker::PostCallRecordGetPhysicalDeviceSurfacePresentModes2EXT(physicalDevice, pSurfaceInfo, pPresentModeCount, pPresentModes, result);
    if (result != VK_SUCCESS) {
        constexpr std::array error_codes = {VK_ERROR_OUT_OF_HOST_MEMORY,VK_ERROR_OUT_OF_DEVICE_MEMORY,VK_ERROR_SURFACE_LOST_KHR};
        constexpr std::array success_codes = {VK_INCOMPLETE};
        ValidateReturnCodes("vkGetPhysicalDeviceSurfacePresentModes2EXT", result, error_codes, success_codes);
    }
}

#endif // VK_USE_PLATFORM_WIN32_KHR

#ifdef VK_USE_PLATFORM_WIN32_KHR

void BestPractices::PostCallRecordAcquireFullScreenExclusiveModeEXT(
    VkDevice                                    device,
    VkSwapchainKHR                              swapchain,
    VkResult                                    result) {
    ValidationStateTracker::PostCallRecordAcquireFullScreenExclusiveModeEXT(device, swapchain, result);
    if (result != VK_SUCCESS) {
        constexpr std::array error_codes = {VK_ERROR_OUT_OF_HOST_MEMORY,VK_ERROR_OUT_OF_DEVICE_MEMORY,VK_ERROR_INITIALIZATION_FAILED,VK_ERROR_SURFACE_LOST_KHR};
        ValidateReturnCodes("vkAcquireFullScreenExclusiveModeEXT", result, error_codes, {});
    }
}

#endif // VK_USE_PLATFORM_WIN32_KHR

#ifdef VK_USE_PLATFORM_WIN32_KHR

void BestPractices::PostCallRecordReleaseFullScreenExclusiveModeEXT(
    VkDevice                                    device,
    VkSwapchainKHR                              swapchain,
    VkResult                                    result) {
    ValidationStateTracker::PostCallRecordReleaseFullScreenExclusiveModeEXT(device, swapchain, result);
    if (result != VK_SUCCESS) {
        constexpr std::array error_codes = {VK_ERROR_OUT_OF_HOST_MEMORY,VK_ERROR_OUT_OF_DEVICE_MEMORY,VK_ERROR_SURFACE_LOST_KHR};
        ValidateReturnCodes("vkReleaseFullScreenExclusiveModeEXT", result, error_codes, {});
    }
}

#endif // VK_USE_PLATFORM_WIN32_KHR

#ifdef VK_USE_PLATFORM_WIN32_KHR

void BestPractices::PostCallRecordGetDeviceGroupSurfacePresentModes2EXT(
    VkDevice                                    device,
    const VkPhysicalDeviceSurfaceInfo2KHR*      pSurfaceInfo,
    VkDeviceGroupPresentModeFlagsKHR*           pModes,
    VkResult                                    result) {
    ValidationStateTracker::PostCallRecordGetDeviceGroupSurfacePresentModes2EXT(device, pSurfaceInfo, pModes, result);
    if (result != VK_SUCCESS) {
        constexpr std::array error_codes = {VK_ERROR_OUT_OF_HOST_MEMORY,VK_ERROR_OUT_OF_DEVICE_MEMORY,VK_ERROR_SURFACE_LOST_KHR};
        ValidateReturnCodes("vkGetDeviceGroupSurfacePresentModes2EXT", result, error_codes, {});
    }
}

#endif // VK_USE_PLATFORM_WIN32_KHR

void BestPractices::PostCallRecordCreateHeadlessSurfaceEXT(
    VkInstance                                  instance,
    const VkHeadlessSurfaceCreateInfoEXT*       pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkSurfaceKHR*                               pSurface,
    VkResult                                    result) {
    ValidationStateTracker::PostCallRecordCreateHeadlessSurfaceEXT(instance, pCreateInfo, pAllocator, pSurface, result);
    if (result != VK_SUCCESS) {
        constexpr std::array error_codes = {VK_ERROR_OUT_OF_HOST_MEMORY,VK_ERROR_OUT_OF_DEVICE_MEMORY};
        ValidateReturnCodes("vkCreateHeadlessSurfaceEXT", result, error_codes, {});
    }
}

void BestPractices::PostCallRecordReleaseSwapchainImagesEXT(
    VkDevice                                    device,
    const VkReleaseSwapchainImagesInfoEXT*      pReleaseInfo,
    VkResult                                    result) {
    ValidationStateTracker::PostCallRecordReleaseSwapchainImagesEXT(device, pReleaseInfo, result);
    if (result != VK_SUCCESS) {
        constexpr std::array error_codes = {VK_ERROR_SURFACE_LOST_KHR};
        ValidateReturnCodes("vkReleaseSwapchainImagesEXT", result, error_codes, {});
    }
}

void BestPractices::PostCallRecordCreateIndirectCommandsLayoutNV(
    VkDevice                                    device,
    const VkIndirectCommandsLayoutCreateInfoNV* pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkIndirectCommandsLayoutNV*                 pIndirectCommandsLayout,
    VkResult                                    result) {
    ValidationStateTracker::PostCallRecordCreateIndirectCommandsLayoutNV(device, pCreateInfo, pAllocator, pIndirectCommandsLayout, result);
    if (result != VK_SUCCESS) {
        constexpr std::array error_codes = {VK_ERROR_OUT_OF_HOST_MEMORY,VK_ERROR_OUT_OF_DEVICE_MEMORY};
        ValidateReturnCodes("vkCreateIndirectCommandsLayoutNV", result, error_codes, {});
    }
}

void BestPractices::PostCallRecordAcquireDrmDisplayEXT(
    VkPhysicalDevice                            physicalDevice,
    int32_t                                     drmFd,
    VkDisplayKHR                                display,
    VkResult                                    result) {
    ValidationStateTracker::PostCallRecordAcquireDrmDisplayEXT(physicalDevice, drmFd, display, result);
    if (result != VK_SUCCESS) {
        constexpr std::array error_codes = {VK_ERROR_INITIALIZATION_FAILED};
        ValidateReturnCodes("vkAcquireDrmDisplayEXT", result, error_codes, {});
    }
}

void BestPractices::PostCallRecordGetDrmDisplayEXT(
    VkPhysicalDevice                            physicalDevice,
    int32_t                                     drmFd,
    uint32_t                                    connectorId,
    VkDisplayKHR*                               display,
    VkResult                                    result) {
    ValidationStateTracker::PostCallRecordGetDrmDisplayEXT(physicalDevice, drmFd, connectorId, display, result);
    if (result != VK_SUCCESS) {
        constexpr std::array error_codes = {VK_ERROR_INITIALIZATION_FAILED,VK_ERROR_OUT_OF_HOST_MEMORY};
        ValidateReturnCodes("vkGetDrmDisplayEXT", result, error_codes, {});
    }
}

void BestPractices::PostCallRecordCreatePrivateDataSlotEXT(
    VkDevice                                    device,
    const VkPrivateDataSlotCreateInfo*          pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkPrivateDataSlot*                          pPrivateDataSlot,
    VkResult                                    result) {
    ValidationStateTracker::PostCallRecordCreatePrivateDataSlotEXT(device, pCreateInfo, pAllocator, pPrivateDataSlot, result);
    if (result != VK_SUCCESS) {
        constexpr std::array error_codes = {VK_ERROR_OUT_OF_HOST_MEMORY};
        ValidateReturnCodes("vkCreatePrivateDataSlotEXT", result, error_codes, {});
    }
}

void BestPractices::PostCallRecordSetPrivateDataEXT(
    VkDevice                                    device,
    VkObjectType                                objectType,
    uint64_t                                    objectHandle,
    VkPrivateDataSlot                           privateDataSlot,
    uint64_t                                    data,
    VkResult                                    result) {
    ValidationStateTracker::PostCallRecordSetPrivateDataEXT(device, objectType, objectHandle, privateDataSlot, data, result);
    if (result != VK_SUCCESS) {
        constexpr std::array error_codes = {VK_ERROR_OUT_OF_HOST_MEMORY};
        ValidateReturnCodes("vkSetPrivateDataEXT", result, error_codes, {});
    }
}

void BestPractices::PostCallRecordGetBufferOpaqueCaptureDescriptorDataEXT(
    VkDevice                                    device,
    const VkBufferCaptureDescriptorDataInfoEXT* pInfo,
    void*                                       pData,
    VkResult                                    result) {
    ValidationStateTracker::PostCallRecordGetBufferOpaqueCaptureDescriptorDataEXT(device, pInfo, pData, result);
    if (result != VK_SUCCESS) {
        constexpr std::array error_codes = {VK_ERROR_OUT_OF_HOST_MEMORY,VK_ERROR_OUT_OF_DEVICE_MEMORY};
        ValidateReturnCodes("vkGetBufferOpaqueCaptureDescriptorDataEXT", result, error_codes, {});
    }
}

void BestPractices::PostCallRecordGetImageOpaqueCaptureDescriptorDataEXT(
    VkDevice                                    device,
    const VkImageCaptureDescriptorDataInfoEXT*  pInfo,
    void*                                       pData,
    VkResult                                    result) {
    ValidationStateTracker::PostCallRecordGetImageOpaqueCaptureDescriptorDataEXT(device, pInfo, pData, result);
    if (result != VK_SUCCESS) {
        constexpr std::array error_codes = {VK_ERROR_OUT_OF_HOST_MEMORY,VK_ERROR_OUT_OF_DEVICE_MEMORY};
        ValidateReturnCodes("vkGetImageOpaqueCaptureDescriptorDataEXT", result, error_codes, {});
    }
}

void BestPractices::PostCallRecordGetImageViewOpaqueCaptureDescriptorDataEXT(
    VkDevice                                    device,
    const VkImageViewCaptureDescriptorDataInfoEXT* pInfo,
    void*                                       pData,
    VkResult                                    result) {
    ValidationStateTracker::PostCallRecordGetImageViewOpaqueCaptureDescriptorDataEXT(device, pInfo, pData, result);
    if (result != VK_SUCCESS) {
        constexpr std::array error_codes = {VK_ERROR_OUT_OF_HOST_MEMORY,VK_ERROR_OUT_OF_DEVICE_MEMORY};
        ValidateReturnCodes("vkGetImageViewOpaqueCaptureDescriptorDataEXT", result, error_codes, {});
    }
}

void BestPractices::PostCallRecordGetSamplerOpaqueCaptureDescriptorDataEXT(
    VkDevice                                    device,
    const VkSamplerCaptureDescriptorDataInfoEXT* pInfo,
    void*                                       pData,
    VkResult                                    result) {
    ValidationStateTracker::PostCallRecordGetSamplerOpaqueCaptureDescriptorDataEXT(device, pInfo, pData, result);
    if (result != VK_SUCCESS) {
        constexpr std::array error_codes = {VK_ERROR_OUT_OF_HOST_MEMORY,VK_ERROR_OUT_OF_DEVICE_MEMORY};
        ValidateReturnCodes("vkGetSamplerOpaqueCaptureDescriptorDataEXT", result, error_codes, {});
    }
}

void BestPractices::PostCallRecordGetAccelerationStructureOpaqueCaptureDescriptorDataEXT(
    VkDevice                                    device,
    const VkAccelerationStructureCaptureDescriptorDataInfoEXT* pInfo,
    void*                                       pData,
    VkResult                                    result) {
    ValidationStateTracker::PostCallRecordGetAccelerationStructureOpaqueCaptureDescriptorDataEXT(device, pInfo, pData, result);
    if (result != VK_SUCCESS) {
        constexpr std::array error_codes = {VK_ERROR_OUT_OF_HOST_MEMORY,VK_ERROR_OUT_OF_DEVICE_MEMORY};
        ValidateReturnCodes("vkGetAccelerationStructureOpaqueCaptureDescriptorDataEXT", result, error_codes, {});
    }
}

void BestPractices::PostCallRecordGetDeviceFaultInfoEXT(
    VkDevice                                    device,
    VkDeviceFaultCountsEXT*                     pFaultCounts,
    VkDeviceFaultInfoEXT*                       pFaultInfo,
    VkResult                                    result) {
    ValidationStateTracker::PostCallRecordGetDeviceFaultInfoEXT(device, pFaultCounts, pFaultInfo, result);
    if (result != VK_SUCCESS) {
        constexpr std::array error_codes = {VK_ERROR_OUT_OF_HOST_MEMORY};
        constexpr std::array success_codes = {VK_INCOMPLETE};
        ValidateReturnCodes("vkGetDeviceFaultInfoEXT", result, error_codes, success_codes);
    }
}

#ifdef VK_USE_PLATFORM_WIN32_KHR

void BestPractices::PostCallRecordAcquireWinrtDisplayNV(
    VkPhysicalDevice                            physicalDevice,
    VkDisplayKHR                                display,
    VkResult                                    result) {
    ValidationStateTracker::PostCallRecordAcquireWinrtDisplayNV(physicalDevice, display, result);
    if (result != VK_SUCCESS) {
        constexpr std::array error_codes = {VK_ERROR_OUT_OF_HOST_MEMORY,VK_ERROR_DEVICE_LOST,VK_ERROR_INITIALIZATION_FAILED};
        ValidateReturnCodes("vkAcquireWinrtDisplayNV", result, error_codes, {});
    }
}

#endif // VK_USE_PLATFORM_WIN32_KHR

#ifdef VK_USE_PLATFORM_WIN32_KHR

void BestPractices::PostCallRecordGetWinrtDisplayNV(
    VkPhysicalDevice                            physicalDevice,
    uint32_t                                    deviceRelativeId,
    VkDisplayKHR*                               pDisplay,
    VkResult                                    result) {
    ValidationStateTracker::PostCallRecordGetWinrtDisplayNV(physicalDevice, deviceRelativeId, pDisplay, result);
    if (result != VK_SUCCESS) {
        constexpr std::array error_codes = {VK_ERROR_OUT_OF_HOST_MEMORY,VK_ERROR_DEVICE_LOST,VK_ERROR_INITIALIZATION_FAILED};
        ValidateReturnCodes("vkGetWinrtDisplayNV", result, error_codes, {});
    }
}

#endif // VK_USE_PLATFORM_WIN32_KHR

#ifdef VK_USE_PLATFORM_DIRECTFB_EXT

void BestPractices::PostCallRecordCreateDirectFBSurfaceEXT(
    VkInstance                                  instance,
    const VkDirectFBSurfaceCreateInfoEXT*       pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkSurfaceKHR*                               pSurface,
    VkResult                                    result) {
    ValidationStateTracker::PostCallRecordCreateDirectFBSurfaceEXT(instance, pCreateInfo, pAllocator, pSurface, result);
    if (result != VK_SUCCESS) {
        constexpr std::array error_codes = {VK_ERROR_OUT_OF_HOST_MEMORY,VK_ERROR_OUT_OF_DEVICE_MEMORY};
        ValidateReturnCodes("vkCreateDirectFBSurfaceEXT", result, error_codes, {});
    }
}

#endif // VK_USE_PLATFORM_DIRECTFB_EXT

#ifdef VK_USE_PLATFORM_FUCHSIA

void BestPractices::PostCallRecordGetMemoryZirconHandleFUCHSIA(
    VkDevice                                    device,
    const VkMemoryGetZirconHandleInfoFUCHSIA*   pGetZirconHandleInfo,
    zx_handle_t*                                pZirconHandle,
    VkResult                                    result) {
    ValidationStateTracker::PostCallRecordGetMemoryZirconHandleFUCHSIA(device, pGetZirconHandleInfo, pZirconHandle, result);
    if (result != VK_SUCCESS) {
        constexpr std::array error_codes = {VK_ERROR_TOO_MANY_OBJECTS,VK_ERROR_OUT_OF_HOST_MEMORY};
        ValidateReturnCodes("vkGetMemoryZirconHandleFUCHSIA", result, error_codes, {});
    }
}

#endif // VK_USE_PLATFORM_FUCHSIA

#ifdef VK_USE_PLATFORM_FUCHSIA

void BestPractices::PostCallRecordGetMemoryZirconHandlePropertiesFUCHSIA(
    VkDevice                                    device,
    VkExternalMemoryHandleTypeFlagBits          handleType,
    zx_handle_t                                 zirconHandle,
    VkMemoryZirconHandlePropertiesFUCHSIA*      pMemoryZirconHandleProperties,
    VkResult                                    result) {
    ValidationStateTracker::PostCallRecordGetMemoryZirconHandlePropertiesFUCHSIA(device, handleType, zirconHandle, pMemoryZirconHandleProperties, result);
    if (result != VK_SUCCESS) {
        constexpr std::array error_codes = {VK_ERROR_INVALID_EXTERNAL_HANDLE};
        ValidateReturnCodes("vkGetMemoryZirconHandlePropertiesFUCHSIA", result, error_codes, {});
    }
}

#endif // VK_USE_PLATFORM_FUCHSIA

#ifdef VK_USE_PLATFORM_FUCHSIA

void BestPractices::PostCallRecordImportSemaphoreZirconHandleFUCHSIA(
    VkDevice                                    device,
    const VkImportSemaphoreZirconHandleInfoFUCHSIA* pImportSemaphoreZirconHandleInfo,
    VkResult                                    result) {
    ValidationStateTracker::PostCallRecordImportSemaphoreZirconHandleFUCHSIA(device, pImportSemaphoreZirconHandleInfo, result);
    if (result != VK_SUCCESS) {
        constexpr std::array error_codes = {VK_ERROR_OUT_OF_HOST_MEMORY,VK_ERROR_INVALID_EXTERNAL_HANDLE};
        ValidateReturnCodes("vkImportSemaphoreZirconHandleFUCHSIA", result, error_codes, {});
    }
}

#endif // VK_USE_PLATFORM_FUCHSIA

#ifdef VK_USE_PLATFORM_FUCHSIA

void BestPractices::PostCallRecordGetSemaphoreZirconHandleFUCHSIA(
    VkDevice                                    device,
    const VkSemaphoreGetZirconHandleInfoFUCHSIA* pGetZirconHandleInfo,
    zx_handle_t*                                pZirconHandle,
    VkResult                                    result) {
    ValidationStateTracker::PostCallRecordGetSemaphoreZirconHandleFUCHSIA(device, pGetZirconHandleInfo, pZirconHandle, result);
    if (result != VK_SUCCESS) {
        constexpr std::array error_codes = {VK_ERROR_TOO_MANY_OBJECTS,VK_ERROR_OUT_OF_HOST_MEMORY};
        ValidateReturnCodes("vkGetSemaphoreZirconHandleFUCHSIA", result, error_codes, {});
    }
}

#endif // VK_USE_PLATFORM_FUCHSIA

#ifdef VK_USE_PLATFORM_FUCHSIA

void BestPractices::PostCallRecordCreateBufferCollectionFUCHSIA(
    VkDevice                                    device,
    const VkBufferCollectionCreateInfoFUCHSIA*  pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkBufferCollectionFUCHSIA*                  pCollection,
    VkResult                                    result) {
    ValidationStateTracker::PostCallRecordCreateBufferCollectionFUCHSIA(device, pCreateInfo, pAllocator, pCollection, result);
    if (result != VK_SUCCESS) {
        constexpr std::array error_codes = {VK_ERROR_OUT_OF_HOST_MEMORY,VK_ERROR_INVALID_EXTERNAL_HANDLE,VK_ERROR_INITIALIZATION_FAILED};
        ValidateReturnCodes("vkCreateBufferCollectionFUCHSIA", result, error_codes, {});
    }
}

#endif // VK_USE_PLATFORM_FUCHSIA

#ifdef VK_USE_PLATFORM_FUCHSIA

void BestPractices::PostCallRecordSetBufferCollectionImageConstraintsFUCHSIA(
    VkDevice                                    device,
    VkBufferCollectionFUCHSIA                   collection,
    const VkImageConstraintsInfoFUCHSIA*        pImageConstraintsInfo,
    VkResult                                    result) {
    ValidationStateTracker::PostCallRecordSetBufferCollectionImageConstraintsFUCHSIA(device, collection, pImageConstraintsInfo, result);
    if (result != VK_SUCCESS) {
        constexpr std::array error_codes = {VK_ERROR_INITIALIZATION_FAILED,VK_ERROR_OUT_OF_HOST_MEMORY,VK_ERROR_FORMAT_NOT_SUPPORTED};
        ValidateReturnCodes("vkSetBufferCollectionImageConstraintsFUCHSIA", result, error_codes, {});
    }
}

#endif // VK_USE_PLATFORM_FUCHSIA

#ifdef VK_USE_PLATFORM_FUCHSIA

void BestPractices::PostCallRecordSetBufferCollectionBufferConstraintsFUCHSIA(
    VkDevice                                    device,
    VkBufferCollectionFUCHSIA                   collection,
    const VkBufferConstraintsInfoFUCHSIA*       pBufferConstraintsInfo,
    VkResult                                    result) {
    ValidationStateTracker::PostCallRecordSetBufferCollectionBufferConstraintsFUCHSIA(device, collection, pBufferConstraintsInfo, result);
    if (result != VK_SUCCESS) {
        constexpr std::array error_codes = {VK_ERROR_INITIALIZATION_FAILED,VK_ERROR_OUT_OF_HOST_MEMORY,VK_ERROR_FORMAT_NOT_SUPPORTED};
        ValidateReturnCodes("vkSetBufferCollectionBufferConstraintsFUCHSIA", result, error_codes, {});
    }
}

#endif // VK_USE_PLATFORM_FUCHSIA

#ifdef VK_USE_PLATFORM_FUCHSIA

void BestPractices::PostCallRecordGetBufferCollectionPropertiesFUCHSIA(
    VkDevice                                    device,
    VkBufferCollectionFUCHSIA                   collection,
    VkBufferCollectionPropertiesFUCHSIA*        pProperties,
    VkResult                                    result) {
    ValidationStateTracker::PostCallRecordGetBufferCollectionPropertiesFUCHSIA(device, collection, pProperties, result);
    if (result != VK_SUCCESS) {
        constexpr std::array error_codes = {VK_ERROR_OUT_OF_HOST_MEMORY,VK_ERROR_INITIALIZATION_FAILED};
        ValidateReturnCodes("vkGetBufferCollectionPropertiesFUCHSIA", result, error_codes, {});
    }
}

#endif // VK_USE_PLATFORM_FUCHSIA

void BestPractices::PostCallRecordGetDeviceSubpassShadingMaxWorkgroupSizeHUAWEI(
    VkDevice                                    device,
    VkRenderPass                                renderpass,
    VkExtent2D*                                 pMaxWorkgroupSize,
    VkResult                                    result) {
    ValidationStateTracker::PostCallRecordGetDeviceSubpassShadingMaxWorkgroupSizeHUAWEI(device, renderpass, pMaxWorkgroupSize, result);
    if (result != VK_SUCCESS) {
        constexpr std::array error_codes = {VK_ERROR_OUT_OF_HOST_MEMORY,VK_ERROR_OUT_OF_DEVICE_MEMORY,VK_ERROR_SURFACE_LOST_KHR};
        constexpr std::array success_codes = {VK_INCOMPLETE};
        ValidateReturnCodes("vkGetDeviceSubpassShadingMaxWorkgroupSizeHUAWEI", result, error_codes, success_codes);
    }
}

void BestPractices::PostCallRecordGetMemoryRemoteAddressNV(
    VkDevice                                    device,
    const VkMemoryGetRemoteAddressInfoNV*       pMemoryGetRemoteAddressInfo,
    VkRemoteAddressNV*                          pAddress,
    VkResult                                    result) {
    ValidationStateTracker::PostCallRecordGetMemoryRemoteAddressNV(device, pMemoryGetRemoteAddressInfo, pAddress, result);
    if (result != VK_SUCCESS) {
        constexpr std::array error_codes = {VK_ERROR_INVALID_EXTERNAL_HANDLE};
        ValidateReturnCodes("vkGetMemoryRemoteAddressNV", result, error_codes, {});
    }
}

void BestPractices::PostCallRecordGetPipelinePropertiesEXT(
    VkDevice                                    device,
    const VkPipelineInfoEXT*                    pPipelineInfo,
    VkBaseOutStructure*                         pPipelineProperties,
    VkResult                                    result) {
    ValidationStateTracker::PostCallRecordGetPipelinePropertiesEXT(device, pPipelineInfo, pPipelineProperties, result);
    if (result != VK_SUCCESS) {
        constexpr std::array error_codes = {VK_ERROR_OUT_OF_HOST_MEMORY};
        ValidateReturnCodes("vkGetPipelinePropertiesEXT", result, error_codes, {});
    }
}

#ifdef VK_USE_PLATFORM_SCREEN_QNX

void BestPractices::PostCallRecordCreateScreenSurfaceQNX(
    VkInstance                                  instance,
    const VkScreenSurfaceCreateInfoQNX*         pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkSurfaceKHR*                               pSurface,
    VkResult                                    result) {
    ValidationStateTracker::PostCallRecordCreateScreenSurfaceQNX(instance, pCreateInfo, pAllocator, pSurface, result);
    if (result != VK_SUCCESS) {
        constexpr std::array error_codes = {VK_ERROR_OUT_OF_HOST_MEMORY,VK_ERROR_OUT_OF_DEVICE_MEMORY};
        ValidateReturnCodes("vkCreateScreenSurfaceQNX", result, error_codes, {});
    }
}

#endif // VK_USE_PLATFORM_SCREEN_QNX

void BestPractices::PostCallRecordCreateMicromapEXT(
    VkDevice                                    device,
    const VkMicromapCreateInfoEXT*              pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkMicromapEXT*                              pMicromap,
    VkResult                                    result) {
    ValidationStateTracker::PostCallRecordCreateMicromapEXT(device, pCreateInfo, pAllocator, pMicromap, result);
    if (result != VK_SUCCESS) {
        constexpr std::array error_codes = {VK_ERROR_OUT_OF_HOST_MEMORY,VK_ERROR_INVALID_OPAQUE_CAPTURE_ADDRESS_KHR};
        ValidateReturnCodes("vkCreateMicromapEXT", result, error_codes, {});
    }
}

void BestPractices::PostCallRecordBuildMicromapsEXT(
    VkDevice                                    device,
    VkDeferredOperationKHR                      deferredOperation,
    uint32_t                                    infoCount,
    const VkMicromapBuildInfoEXT*               pInfos,
    VkResult                                    result) {
    ValidationStateTracker::PostCallRecordBuildMicromapsEXT(device, deferredOperation, infoCount, pInfos, result);
    if (result != VK_SUCCESS) {
        constexpr std::array error_codes = {VK_ERROR_OUT_OF_HOST_MEMORY,VK_ERROR_OUT_OF_DEVICE_MEMORY};
        constexpr std::array success_codes = {VK_OPERATION_DEFERRED_KHR,VK_OPERATION_NOT_DEFERRED_KHR};
        ValidateReturnCodes("vkBuildMicromapsEXT", result, error_codes, success_codes);
    }
}

void BestPractices::PostCallRecordCopyMicromapEXT(
    VkDevice                                    device,
    VkDeferredOperationKHR                      deferredOperation,
    const VkCopyMicromapInfoEXT*                pInfo,
    VkResult                                    result) {
    ValidationStateTracker::PostCallRecordCopyMicromapEXT(device, deferredOperation, pInfo, result);
    if (result != VK_SUCCESS) {
        constexpr std::array error_codes = {VK_ERROR_OUT_OF_HOST_MEMORY,VK_ERROR_OUT_OF_DEVICE_MEMORY};
        constexpr std::array success_codes = {VK_OPERATION_DEFERRED_KHR,VK_OPERATION_NOT_DEFERRED_KHR};
        ValidateReturnCodes("vkCopyMicromapEXT", result, error_codes, success_codes);
    }
}

void BestPractices::PostCallRecordCopyMicromapToMemoryEXT(
    VkDevice                                    device,
    VkDeferredOperationKHR                      deferredOperation,
    const VkCopyMicromapToMemoryInfoEXT*        pInfo,
    VkResult                                    result) {
    ValidationStateTracker::PostCallRecordCopyMicromapToMemoryEXT(device, deferredOperation, pInfo, result);
    if (result != VK_SUCCESS) {
        constexpr std::array error_codes = {VK_ERROR_OUT_OF_HOST_MEMORY,VK_ERROR_OUT_OF_DEVICE_MEMORY};
        constexpr std::array success_codes = {VK_OPERATION_DEFERRED_KHR,VK_OPERATION_NOT_DEFERRED_KHR};
        ValidateReturnCodes("vkCopyMicromapToMemoryEXT", result, error_codes, success_codes);
    }
}

void BestPractices::PostCallRecordCopyMemoryToMicromapEXT(
    VkDevice                                    device,
    VkDeferredOperationKHR                      deferredOperation,
    const VkCopyMemoryToMicromapInfoEXT*        pInfo,
    VkResult                                    result) {
    ValidationStateTracker::PostCallRecordCopyMemoryToMicromapEXT(device, deferredOperation, pInfo, result);
    if (result != VK_SUCCESS) {
        constexpr std::array error_codes = {VK_ERROR_OUT_OF_HOST_MEMORY,VK_ERROR_OUT_OF_DEVICE_MEMORY};
        constexpr std::array success_codes = {VK_OPERATION_DEFERRED_KHR,VK_OPERATION_NOT_DEFERRED_KHR};
        ValidateReturnCodes("vkCopyMemoryToMicromapEXT", result, error_codes, success_codes);
    }
}

void BestPractices::PostCallRecordWriteMicromapsPropertiesEXT(
    VkDevice                                    device,
    uint32_t                                    micromapCount,
    const VkMicromapEXT*                        pMicromaps,
    VkQueryType                                 queryType,
    size_t                                      dataSize,
    void*                                       pData,
    size_t                                      stride,
    VkResult                                    result) {
    ValidationStateTracker::PostCallRecordWriteMicromapsPropertiesEXT(device, micromapCount, pMicromaps, queryType, dataSize, pData, stride, result);
    if (result != VK_SUCCESS) {
        constexpr std::array error_codes = {VK_ERROR_OUT_OF_HOST_MEMORY,VK_ERROR_OUT_OF_DEVICE_MEMORY};
        ValidateReturnCodes("vkWriteMicromapsPropertiesEXT", result, error_codes, {});
    }
}

void BestPractices::PostCallRecordGetPhysicalDeviceOpticalFlowImageFormatsNV(
    VkPhysicalDevice                            physicalDevice,
    const VkOpticalFlowImageFormatInfoNV*       pOpticalFlowImageFormatInfo,
    uint32_t*                                   pFormatCount,
    VkOpticalFlowImageFormatPropertiesNV*       pImageFormatProperties,
    VkResult                                    result) {
    ValidationStateTracker::PostCallRecordGetPhysicalDeviceOpticalFlowImageFormatsNV(physicalDevice, pOpticalFlowImageFormatInfo, pFormatCount, pImageFormatProperties, result);
    if (result != VK_SUCCESS) {
        constexpr std::array error_codes = {VK_ERROR_EXTENSION_NOT_PRESENT,VK_ERROR_INITIALIZATION_FAILED,VK_ERROR_FORMAT_NOT_SUPPORTED};
        constexpr std::array success_codes = {VK_INCOMPLETE};
        ValidateReturnCodes("vkGetPhysicalDeviceOpticalFlowImageFormatsNV", result, error_codes, success_codes);
    }
}

void BestPractices::PostCallRecordCreateOpticalFlowSessionNV(
    VkDevice                                    device,
    const VkOpticalFlowSessionCreateInfoNV*     pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkOpticalFlowSessionNV*                     pSession,
    VkResult                                    result) {
    ValidationStateTracker::PostCallRecordCreateOpticalFlowSessionNV(device, pCreateInfo, pAllocator, pSession, result);
    if (result != VK_SUCCESS) {
        constexpr std::array error_codes = {VK_ERROR_OUT_OF_HOST_MEMORY,VK_ERROR_INITIALIZATION_FAILED};
        ValidateReturnCodes("vkCreateOpticalFlowSessionNV", result, error_codes, {});
    }
}

void BestPractices::PostCallRecordBindOpticalFlowSessionImageNV(
    VkDevice                                    device,
    VkOpticalFlowSessionNV                      session,
    VkOpticalFlowSessionBindingPointNV          bindingPoint,
    VkImageView                                 view,
    VkImageLayout                               layout,
    VkResult                                    result) {
    ValidationStateTracker::PostCallRecordBindOpticalFlowSessionImageNV(device, session, bindingPoint, view, layout, result);
    if (result != VK_SUCCESS) {
        constexpr std::array error_codes = {VK_ERROR_OUT_OF_HOST_MEMORY,VK_ERROR_INITIALIZATION_FAILED};
        ValidateReturnCodes("vkBindOpticalFlowSessionImageNV", result, error_codes, {});
    }
}

void BestPractices::PostCallRecordGetFramebufferTilePropertiesQCOM(
    VkDevice                                    device,
    VkFramebuffer                               framebuffer,
    uint32_t*                                   pPropertiesCount,
    VkTilePropertiesQCOM*                       pProperties,
    VkResult                                    result) {
    ValidationStateTracker::PostCallRecordGetFramebufferTilePropertiesQCOM(device, framebuffer, pPropertiesCount, pProperties, result);
    if (result != VK_SUCCESS) {
        constexpr std::array success_codes = {VK_INCOMPLETE};
        ValidateReturnCodes("vkGetFramebufferTilePropertiesQCOM", result, {}, success_codes);
    }
}

void BestPractices::PostCallRecordCreateAccelerationStructureKHR(
    VkDevice                                    device,
    const VkAccelerationStructureCreateInfoKHR* pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkAccelerationStructureKHR*                 pAccelerationStructure,
    VkResult                                    result) {
    ValidationStateTracker::PostCallRecordCreateAccelerationStructureKHR(device, pCreateInfo, pAllocator, pAccelerationStructure, result);
    if (result != VK_SUCCESS) {
        constexpr std::array error_codes = {VK_ERROR_OUT_OF_HOST_MEMORY,VK_ERROR_INVALID_OPAQUE_CAPTURE_ADDRESS_KHR};
        ValidateReturnCodes("vkCreateAccelerationStructureKHR", result, error_codes, {});
    }
}

void BestPractices::PostCallRecordBuildAccelerationStructuresKHR(
    VkDevice                                    device,
    VkDeferredOperationKHR                      deferredOperation,
    uint32_t                                    infoCount,
    const VkAccelerationStructureBuildGeometryInfoKHR* pInfos,
    const VkAccelerationStructureBuildRangeInfoKHR* const* ppBuildRangeInfos,
    VkResult                                    result) {
    ValidationStateTracker::PostCallRecordBuildAccelerationStructuresKHR(device, deferredOperation, infoCount, pInfos, ppBuildRangeInfos, result);
    if (result != VK_SUCCESS) {
        constexpr std::array error_codes = {VK_ERROR_OUT_OF_HOST_MEMORY,VK_ERROR_OUT_OF_DEVICE_MEMORY};
        constexpr std::array success_codes = {VK_OPERATION_DEFERRED_KHR,VK_OPERATION_NOT_DEFERRED_KHR};
        ValidateReturnCodes("vkBuildAccelerationStructuresKHR", result, error_codes, success_codes);
    }
}

void BestPractices::PostCallRecordCopyAccelerationStructureKHR(
    VkDevice                                    device,
    VkDeferredOperationKHR                      deferredOperation,
    const VkCopyAccelerationStructureInfoKHR*   pInfo,
    VkResult                                    result) {
    ValidationStateTracker::PostCallRecordCopyAccelerationStructureKHR(device, deferredOperation, pInfo, result);
    if (result != VK_SUCCESS) {
        constexpr std::array error_codes = {VK_ERROR_OUT_OF_HOST_MEMORY,VK_ERROR_OUT_OF_DEVICE_MEMORY};
        constexpr std::array success_codes = {VK_OPERATION_DEFERRED_KHR,VK_OPERATION_NOT_DEFERRED_KHR};
        ValidateReturnCodes("vkCopyAccelerationStructureKHR", result, error_codes, success_codes);
    }
}

void BestPractices::PostCallRecordCopyAccelerationStructureToMemoryKHR(
    VkDevice                                    device,
    VkDeferredOperationKHR                      deferredOperation,
    const VkCopyAccelerationStructureToMemoryInfoKHR* pInfo,
    VkResult                                    result) {
    ValidationStateTracker::PostCallRecordCopyAccelerationStructureToMemoryKHR(device, deferredOperation, pInfo, result);
    if (result != VK_SUCCESS) {
        constexpr std::array error_codes = {VK_ERROR_OUT_OF_HOST_MEMORY,VK_ERROR_OUT_OF_DEVICE_MEMORY};
        constexpr std::array success_codes = {VK_OPERATION_DEFERRED_KHR,VK_OPERATION_NOT_DEFERRED_KHR};
        ValidateReturnCodes("vkCopyAccelerationStructureToMemoryKHR", result, error_codes, success_codes);
    }
}

void BestPractices::PostCallRecordCopyMemoryToAccelerationStructureKHR(
    VkDevice                                    device,
    VkDeferredOperationKHR                      deferredOperation,
    const VkCopyMemoryToAccelerationStructureInfoKHR* pInfo,
    VkResult                                    result) {
    ValidationStateTracker::PostCallRecordCopyMemoryToAccelerationStructureKHR(device, deferredOperation, pInfo, result);
    if (result != VK_SUCCESS) {
        constexpr std::array error_codes = {VK_ERROR_OUT_OF_HOST_MEMORY,VK_ERROR_OUT_OF_DEVICE_MEMORY};
        constexpr std::array success_codes = {VK_OPERATION_DEFERRED_KHR,VK_OPERATION_NOT_DEFERRED_KHR};
        ValidateReturnCodes("vkCopyMemoryToAccelerationStructureKHR", result, error_codes, success_codes);
    }
}

void BestPractices::PostCallRecordWriteAccelerationStructuresPropertiesKHR(
    VkDevice                                    device,
    uint32_t                                    accelerationStructureCount,
    const VkAccelerationStructureKHR*           pAccelerationStructures,
    VkQueryType                                 queryType,
    size_t                                      dataSize,
    void*                                       pData,
    size_t                                      stride,
    VkResult                                    result) {
    ValidationStateTracker::PostCallRecordWriteAccelerationStructuresPropertiesKHR(device, accelerationStructureCount, pAccelerationStructures, queryType, dataSize, pData, stride, result);
    if (result != VK_SUCCESS) {
        constexpr std::array error_codes = {VK_ERROR_OUT_OF_HOST_MEMORY,VK_ERROR_OUT_OF_DEVICE_MEMORY};
        ValidateReturnCodes("vkWriteAccelerationStructuresPropertiesKHR", result, error_codes, {});
    }
}

void BestPractices::PostCallRecordCreateRayTracingPipelinesKHR(
    VkDevice                                    device,
    VkDeferredOperationKHR                      deferredOperation,
    VkPipelineCache                             pipelineCache,
    uint32_t                                    createInfoCount,
    const VkRayTracingPipelineCreateInfoKHR*    pCreateInfos,
    const VkAllocationCallbacks*                pAllocator,
    VkPipeline*                                 pPipelines,
    VkResult                                    result,
    void*                                       state_data) {
    ValidationStateTracker::PostCallRecordCreateRayTracingPipelinesKHR(device, deferredOperation, pipelineCache, createInfoCount, pCreateInfos, pAllocator, pPipelines, result, state_data);
    if (result != VK_SUCCESS) {
        constexpr std::array error_codes = {VK_ERROR_OUT_OF_HOST_MEMORY,VK_ERROR_OUT_OF_DEVICE_MEMORY,VK_ERROR_INVALID_OPAQUE_CAPTURE_ADDRESS};
        constexpr std::array success_codes = {VK_OPERATION_DEFERRED_KHR,VK_OPERATION_NOT_DEFERRED_KHR,VK_PIPELINE_COMPILE_REQUIRED_EXT};
        ValidateReturnCodes("vkCreateRayTracingPipelinesKHR", result, error_codes, success_codes);
    }
}

void BestPractices::PostCallRecordGetRayTracingCaptureReplayShaderGroupHandlesKHR(
    VkDevice                                    device,
    VkPipeline                                  pipeline,
    uint32_t                                    firstGroup,
    uint32_t                                    groupCount,
    size_t                                      dataSize,
    void*                                       pData,
    VkResult                                    result) {
    ValidationStateTracker::PostCallRecordGetRayTracingCaptureReplayShaderGroupHandlesKHR(device, pipeline, firstGroup, groupCount, dataSize, pData, result);
    if (result != VK_SUCCESS) {
        constexpr std::array error_codes = {VK_ERROR_OUT_OF_HOST_MEMORY,VK_ERROR_OUT_OF_DEVICE_MEMORY};
        ValidateReturnCodes("vkGetRayTracingCaptureReplayShaderGroupHandlesKHR", result, error_codes, {});
    }
}





