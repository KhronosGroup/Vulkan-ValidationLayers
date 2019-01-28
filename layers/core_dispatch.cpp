/* Copyright (c) 2015-2019 The Khronos Group Inc.
 * Copyright (c) 2015-2019 Valve Corporation
 * Copyright (c) 2015-2019 LunarG, Inc.
 * Copyright (C) 2015-2019 Google Inc.
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
 */

#include "core_validation.h"
#include "shader_validation.h"
#include "vk_layer_data.h"
#include "vk_dispatch_table_helper.h"
#include "vk_layer_extension_utils.h"
#include "buffer_validation.h"

#include <unordered_map>

namespace core_validation {

using std::unordered_map;
using mutex_t = std::mutex;
using lock_guard_t = std::lock_guard<mutex_t>;
using unique_lock_t = std::unique_lock<mutex_t>;

extern unordered_map<void *, layer_data *> layer_data_map;
extern unordered_map<void *, instance_layer_data *> instance_layer_data_map;
extern mutex_t global_lock;

static uint32_t loader_layer_if_version = CURRENT_LOADER_LAYER_INTERFACE_VERSION;

static const VkLayerProperties global_layer = {
    "VK_LAYER_LUNARG_core_validation",
    VK_LAYER_API_VERSION,
    1,
    "LunarG Validation Layer",
};

static const VkExtensionProperties device_extensions[] = {
    {VK_EXT_VALIDATION_CACHE_EXTENSION_NAME, VK_EXT_VALIDATION_CACHE_SPEC_VERSION},
};
static const VkExtensionProperties instance_extensions[] = {{VK_EXT_DEBUG_REPORT_EXTENSION_NAME, VK_EXT_DEBUG_REPORT_SPEC_VERSION}};

template <class TCreateInfo>
void ValidateLayerOrdering(const TCreateInfo &createInfo) {
    bool foundLayer = false;
    for (uint32_t i = 0; i < createInfo.enabledLayerCount; ++i) {
        if (!strcmp(createInfo.ppEnabledLayerNames[i], global_layer.layerName)) {
            foundLayer = true;
        }
        // This has to be logged to console as we don't have a callback at this point.
        if (!foundLayer && !strcmp(createInfo.ppEnabledLayerNames[0], "VK_LAYER_GOOGLE_unique_objects")) {
            LOGCONSOLE("Cannot activate layer VK_LAYER_GOOGLE_unique_objects prior to activating %s.", global_layer.layerName);
        }
    }
}

VKAPI_ATTR VkResult VKAPI_CALL CreateFence(VkDevice device, const VkFenceCreateInfo *pCreateInfo,
                                           const VkAllocationCallbacks *pAllocator, VkFence *pFence) {
    layer_data *dev_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    VkResult result = dev_data->dispatch_table.CreateFence(device, pCreateInfo, pAllocator, pFence);
    lock_guard_t lock(global_lock);
    PostCallRecordCreateFence(device, pCreateInfo, pAllocator, pFence, result);
    return result;
}

VKAPI_ATTR VkResult VKAPI_CALL CreateInstance(const VkInstanceCreateInfo *pCreateInfo, const VkAllocationCallbacks *pAllocator,
                                              VkInstance *pInstance) {
    VkLayerInstanceCreateInfo *chain_info = get_chain_info(pCreateInfo, VK_LAYER_LINK_INFO);

    assert(chain_info->u.pLayerInfo);
    PFN_vkGetInstanceProcAddr fpGetInstanceProcAddr = chain_info->u.pLayerInfo->pfnNextGetInstanceProcAddr;
    PFN_vkCreateInstance fpCreateInstance = (PFN_vkCreateInstance)fpGetInstanceProcAddr(NULL, "vkCreateInstance");
    if (fpCreateInstance == NULL) return VK_ERROR_INITIALIZATION_FAILED;

    chain_info->u.pLayerInfo = chain_info->u.pLayerInfo->pNext;

    VkResult result = fpCreateInstance(pCreateInfo, pAllocator, pInstance);
    if (result != VK_SUCCESS) return result;

    instance_layer_data *instance_data = GetLayerDataPtr(get_dispatch_key(*pInstance), instance_layer_data_map);
    instance_data->instance = *pInstance;
    layer_init_instance_dispatch_table(*pInstance, &instance_data->dispatch_table, fpGetInstanceProcAddr);
    instance_data->report_data = debug_utils_create_instance(
        &instance_data->dispatch_table, *pInstance, pCreateInfo->enabledExtensionCount, pCreateInfo->ppEnabledExtensionNames);

    instance_data->api_version = instance_data->extensions.InitFromInstanceCreateInfo(
        (pCreateInfo->pApplicationInfo ? pCreateInfo->pApplicationInfo->apiVersion : VK_API_VERSION_1_0), pCreateInfo);

    layer_debug_messenger_actions(instance_data->report_data, instance_data->logging_messenger, pAllocator,
                                  "lunarg_core_validation");

    ValidateLayerOrdering(*pCreateInfo);
    PostCallRecordCreateInstance(pCreateInfo, pAllocator, pInstance, result);

    return result;
}

VKAPI_ATTR void VKAPI_CALL DestroyInstance(VkInstance instance, const VkAllocationCallbacks *pAllocator) {
    dispatch_key key = get_dispatch_key(instance);
    instance_layer_data *instance_data = GetLayerDataPtr(key, instance_layer_data_map);
    instance_data->dispatch_table.DestroyInstance(instance, pAllocator);
    lock_guard_t lock(global_lock);
    while (instance_data->logging_messenger.size() > 0) {
        VkDebugUtilsMessengerEXT messenger = instance_data->logging_messenger.back();
        layer_destroy_messenger_callback(instance_data->report_data, messenger, pAllocator);
        instance_data->logging_messenger.pop_back();
    }
    while (instance_data->logging_callback.size() > 0) {
        VkDebugReportCallbackEXT callback = instance_data->logging_callback.back();
        layer_destroy_report_callback(instance_data->report_data, callback, pAllocator);
        instance_data->logging_callback.pop_back();
    }

    layer_debug_utils_destroy_instance(instance_data->report_data);
    FreeLayerDataPtr(key, instance_layer_data_map);
}

VKAPI_ATTR VkResult VKAPI_CALL CreateDevice(VkPhysicalDevice gpu, const VkDeviceCreateInfo *pCreateInfo,
                                            const VkAllocationCallbacks *pAllocator, VkDevice *pDevice) {
    instance_layer_data *instance_data = GetLayerDataPtr(get_dispatch_key(gpu), instance_layer_data_map);

    unique_lock_t lock(global_lock);
    bool skip = PreCallValidateCreateDevice(gpu, pCreateInfo, pAllocator, pDevice);

    if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;

    VkLayerDeviceCreateInfo *chain_info = get_chain_info(pCreateInfo, VK_LAYER_LINK_INFO);
    assert(chain_info->u.pLayerInfo);
    PFN_vkGetInstanceProcAddr fpGetInstanceProcAddr = chain_info->u.pLayerInfo->pfnNextGetInstanceProcAddr;
    PFN_vkGetDeviceProcAddr fpGetDeviceProcAddr = chain_info->u.pLayerInfo->pfnNextGetDeviceProcAddr;
    PFN_vkCreateDevice fpCreateDevice = (PFN_vkCreateDevice)fpGetInstanceProcAddr(instance_data->instance, "vkCreateDevice");
    if (fpCreateDevice == NULL) {
        return VK_ERROR_INITIALIZATION_FAILED;
    }

    // Advance the link info for the next element on the chain
    chain_info->u.pLayerInfo = chain_info->u.pLayerInfo->pNext;

    // GPU Validation can possibly turn on device features, so give it a chance to change the create info.
    std::unique_ptr<safe_VkDeviceCreateInfo> gpu_create_info;
    if (instance_data->enabled.gpu_validation) {
        VkPhysicalDeviceFeatures supported_features;
        instance_data->dispatch_table.GetPhysicalDeviceFeatures(gpu, &supported_features);
        gpu_create_info = GpuPreCallRecordCreateDevice(gpu, pCreateInfo, &supported_features);
        pCreateInfo = reinterpret_cast<VkDeviceCreateInfo *>(gpu_create_info.get());
    }
    lock.unlock();

    VkResult result = fpCreateDevice(gpu, pCreateInfo, pAllocator, pDevice);
    if (result != VK_SUCCESS) {
        return result;
    }

    lock.lock();
    layer_data *device_data = GetLayerDataPtr(get_dispatch_key(*pDevice), layer_data_map);
    device_data->instance_data = instance_data;
    // Setup device dispatch table
    layer_init_device_dispatch_table(*pDevice, &device_data->dispatch_table, fpGetDeviceProcAddr);
    device_data->device = *pDevice;
    // Save PhysicalDevice handle
    device_data->physical_device = gpu;
    device_data->report_data = layer_debug_utils_create_device(instance_data->report_data, *pDevice);
    // Get physical device limits for this device
    instance_data->dispatch_table.GetPhysicalDeviceProperties(gpu, &(device_data->phys_dev_properties.properties));
    // Setup the validation tables based on the application API version from the instance and the capabilities of the device driver.
    uint32_t effective_api_version = std::min(device_data->phys_dev_properties.properties.apiVersion, instance_data->api_version);
    device_data->api_version =
        device_data->extensions.InitFromDeviceCreateInfo(&instance_data->extensions, effective_api_version, pCreateInfo);
    PostCallRecordCreateDevice(gpu, pCreateInfo, pAllocator, pDevice, result);
    ValidateLayerOrdering(*pCreateInfo);
    lock.unlock();

    return result;
}

// TODO handle pipeline caches
VKAPI_ATTR VkResult VKAPI_CALL CreatePipelineCache(VkDevice device, const VkPipelineCacheCreateInfo *pCreateInfo,
                                                   const VkAllocationCallbacks *pAllocator, VkPipelineCache *pPipelineCache) {
    layer_data *dev_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    VkResult result = dev_data->dispatch_table.CreatePipelineCache(device, pCreateInfo, pAllocator, pPipelineCache);
    return result;
}

VKAPI_ATTR void VKAPI_CALL DestroyPipelineCache(VkDevice device, VkPipelineCache pipelineCache,
                                                const VkAllocationCallbacks *pAllocator) {
    layer_data *dev_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    // Pre-record to avoid Destroy/Create race (if/when implemented)
    dev_data->dispatch_table.DestroyPipelineCache(device, pipelineCache, pAllocator);
}

VKAPI_ATTR VkResult VKAPI_CALL GetPipelineCacheData(VkDevice device, VkPipelineCache pipelineCache, size_t *pDataSize,
                                                    void *pData) {
    layer_data *dev_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    VkResult result = dev_data->dispatch_table.GetPipelineCacheData(device, pipelineCache, pDataSize, pData);
    return result;
}

VKAPI_ATTR VkResult VKAPI_CALL MergePipelineCaches(VkDevice device, VkPipelineCache dstCache, uint32_t srcCacheCount,
                                                   const VkPipelineCache *pSrcCaches) {
    layer_data *dev_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    VkResult result = dev_data->dispatch_table.MergePipelineCaches(device, dstCache, srcCacheCount, pSrcCaches);
    return result;
}

VKAPI_ATTR VkResult VKAPI_CALL EnumerateInstanceLayerProperties(uint32_t *pCount, VkLayerProperties *pProperties) {
    return util_GetLayerProperties(1, &global_layer, pCount, pProperties);
}

VKAPI_ATTR VkResult VKAPI_CALL EnumerateDeviceLayerProperties(VkPhysicalDevice physicalDevice, uint32_t *pCount,
                                                              VkLayerProperties *pProperties) {
    return util_GetLayerProperties(1, &global_layer, pCount, pProperties);
}

VKAPI_ATTR VkResult VKAPI_CALL EnumerateInstanceExtensionProperties(const char *pLayerName, uint32_t *pCount,
                                                                    VkExtensionProperties *pProperties) {
    if (pLayerName && !strcmp(pLayerName, global_layer.layerName))
        return util_GetExtensionProperties(1, instance_extensions, pCount, pProperties);

    return VK_ERROR_LAYER_NOT_PRESENT;
}

VKAPI_ATTR VkResult VKAPI_CALL EnumerateDeviceExtensionProperties(VkPhysicalDevice physicalDevice, const char *pLayerName,
                                                                  uint32_t *pCount, VkExtensionProperties *pProperties) {
    if (pLayerName && !strcmp(pLayerName, global_layer.layerName))
        return util_GetExtensionProperties(1, device_extensions, pCount, pProperties);

    assert(physicalDevice);

    instance_layer_data *instance_data = GetLayerDataPtr(get_dispatch_key(physicalDevice), instance_layer_data_map);
    return instance_data->dispatch_table.EnumerateDeviceExtensionProperties(physicalDevice, NULL, pCount, pProperties);
}

VKAPI_ATTR void VKAPI_CALL CmdUpdateBuffer(VkCommandBuffer commandBuffer, VkBuffer dstBuffer, VkDeviceSize dstOffset,
                                           VkDeviceSize dataSize, const void *pData) {
    bool skip = false;
    layer_data *dev_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    unique_lock_t lock(global_lock);

    skip |= PreCallValidateCmdUpdateBuffer(commandBuffer, dstBuffer, dstOffset, dataSize, pData);
    lock.unlock();
    if (!skip) {
        dev_data->dispatch_table.CmdUpdateBuffer(commandBuffer, dstBuffer, dstOffset, dataSize, pData);
        lock.lock();
        PostCallRecordCmdUpdateBuffer(commandBuffer, dstBuffer, dstOffset, dataSize, pData);
        lock.unlock();
    }
}

VKAPI_ATTR void VKAPI_CALL GetDeviceQueue(VkDevice device, uint32_t queueFamilyIndex, uint32_t queueIndex, VkQueue *pQueue) {
    layer_data *dev_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    dev_data->dispatch_table.GetDeviceQueue(device, queueFamilyIndex, queueIndex, pQueue);
    unique_lock_t lock(global_lock);
    PostCallRecordGetDeviceQueue(device, queueFamilyIndex, queueIndex, pQueue);
}

VKAPI_ATTR void VKAPI_CALL GetDeviceQueue2(VkDevice device, const VkDeviceQueueInfo2 *pQueueInfo, VkQueue *pQueue) {
    layer_data *dev_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    dev_data->dispatch_table.GetDeviceQueue2(device, pQueueInfo, pQueue);
    unique_lock_t lock(global_lock);
    if (*pQueue != VK_NULL_HANDLE) {
        PostCallRecordGetDeviceQueue2(device, pQueueInfo, pQueue);
    }
}

VKAPI_ATTR VkResult VKAPI_CALL CreateSamplerYcbcrConversion(VkDevice device, const VkSamplerYcbcrConversionCreateInfo *pCreateInfo,
                                                            const VkAllocationCallbacks *pAllocator,
                                                            VkSamplerYcbcrConversion *pYcbcrConversion) {
    layer_data *dev_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    bool skip = PreCallValidateCreateSamplerYcbcrConversion(device, pCreateInfo, pAllocator, pYcbcrConversion);
    if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;

    VkResult result = dev_data->dispatch_table.CreateSamplerYcbcrConversion(device, pCreateInfo, pAllocator, pYcbcrConversion);
    unique_lock_t lock(global_lock);
    PostCallRecordCreateSamplerYcbcrConversion(device, pCreateInfo, pAllocator, pYcbcrConversion, result);
    lock.unlock();
    return result;
};

VKAPI_ATTR VkResult VKAPI_CALL CreateSamplerYcbcrConversionKHR(VkDevice device,
                                                               const VkSamplerYcbcrConversionCreateInfo *pCreateInfo,
                                                               const VkAllocationCallbacks *pAllocator,
                                                               VkSamplerYcbcrConversion *pYcbcrConversion) {
    layer_data *dev_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    bool skip = PreCallValidateCreateSamplerYcbcrConversion(device, pCreateInfo, pAllocator, pYcbcrConversion);
    if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;

    VkResult result = dev_data->dispatch_table.CreateSamplerYcbcrConversionKHR(device, pCreateInfo, pAllocator, pYcbcrConversion);
    unique_lock_t lock(global_lock);
    PostCallRecordCreateSamplerYcbcrConversion(device, pCreateInfo, pAllocator, pYcbcrConversion, result);
    lock.unlock();
    return result;
};

VKAPI_ATTR VkResult VKAPI_CALL DebugMarkerSetObjectTagEXT(VkDevice device, const VkDebugMarkerObjectTagInfoEXT *pTagInfo) {
    layer_data *device_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    VkResult result = device_data->dispatch_table.DebugMarkerSetObjectTagEXT(device, pTagInfo);
    return result;
}

VKAPI_ATTR void VKAPI_CALL CmdDebugMarkerBeginEXT(VkCommandBuffer commandBuffer, const VkDebugMarkerMarkerInfoEXT *pMarkerInfo) {
    layer_data *device_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    unique_lock_t lock(global_lock);
    bool skip = false;
    skip |= PreCallValidateCmdDebugMarkerBeginEXT(commandBuffer, pMarkerInfo);
    lock.unlock();
    if (!skip) {
        device_data->dispatch_table.CmdDebugMarkerBeginEXT(commandBuffer, pMarkerInfo);
    }
}

VKAPI_ATTR void VKAPI_CALL CmdDebugMarkerInsertEXT(VkCommandBuffer commandBuffer, const VkDebugMarkerMarkerInfoEXT *pMarkerInfo) {
    layer_data *device_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    device_data->dispatch_table.CmdDebugMarkerInsertEXT(commandBuffer, pMarkerInfo);
}

VKAPI_ATTR void VKAPI_CALL DestroyDevice(VkDevice device, const VkAllocationCallbacks *pAllocator) {
    dispatch_key key = get_dispatch_key(device);
    layer_data *dev_data = GetLayerDataPtr(key, layer_data_map);
    unique_lock_t lock(global_lock);
    PreCallRecordDestroyDevice(device, pAllocator);
    lock.unlock();

    dev_data->dispatch_table.DestroyDevice(device, pAllocator);

    lock.lock();
    FreeLayerDataPtr(key, layer_data_map);
}

VKAPI_ATTR VkResult VKAPI_CALL AllocateMemory(VkDevice device, const VkMemoryAllocateInfo *pAllocateInfo,
                                              const VkAllocationCallbacks *pAllocator, VkDeviceMemory *pMemory) {
    VkResult result = VK_ERROR_VALIDATION_FAILED_EXT;
    layer_data *dev_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    unique_lock_t lock(global_lock);
    bool skip = PreCallValidateAllocateMemory(device, pAllocateInfo, pAllocator, pMemory);
    lock.unlock();
    if (!skip) {
        result = dev_data->dispatch_table.AllocateMemory(device, pAllocateInfo, pAllocator, pMemory);
        lock.lock();
        PostCallRecordAllocateMemory(device, pAllocateInfo, pAllocator, pMemory, result);
        lock.unlock();
    }
    return result;
}

VKAPI_ATTR void VKAPI_CALL FreeMemory(VkDevice device, VkDeviceMemory mem, const VkAllocationCallbacks *pAllocator) {
    layer_data *dev_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    unique_lock_t lock(global_lock);
    bool skip = PreCallValidateFreeMemory(device, mem, pAllocator);
    lock.unlock();
    if (!skip) {
        if (mem != VK_NULL_HANDLE) {
            // Avoid free/alloc race by recording state change before dispatching
            lock.lock();
            PreCallRecordFreeMemory(device, mem, pAllocator);
            lock.unlock();
        }
        dev_data->dispatch_table.FreeMemory(device, mem, pAllocator);
    }
}

VKAPI_ATTR VkResult VKAPI_CALL WaitForFences(VkDevice device, uint32_t fenceCount, const VkFence *pFences, VkBool32 waitAll,
                                             uint64_t timeout) {
    layer_data *dev_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    // Verify fence status of submitted fences
    unique_lock_t lock(global_lock);
    bool skip = PreCallValidateWaitForFences(device, fenceCount, pFences, waitAll, timeout);
    lock.unlock();
    if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;

    VkResult result = dev_data->dispatch_table.WaitForFences(device, fenceCount, pFences, waitAll, timeout);
    lock.lock();
    PostCallRecordWaitForFences(device, fenceCount, pFences, waitAll, timeout, result);
    lock.unlock();
    return result;
}

VKAPI_ATTR VkResult VKAPI_CALL GetFenceStatus(VkDevice device, VkFence fence) {
    layer_data *dev_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    unique_lock_t lock(global_lock);
    bool skip = PreCallValidateGetFenceStatus(device, fence);
    lock.unlock();
    if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
    VkResult result = dev_data->dispatch_table.GetFenceStatus(device, fence);
    lock.lock();
    PostCallRecordGetFenceStatus(device, fence, result);
    lock.unlock();
    return result;
}

VKAPI_ATTR VkResult VKAPI_CALL QueueWaitIdle(VkQueue queue) {
    layer_data *dev_data = GetLayerDataPtr(get_dispatch_key(queue), layer_data_map);
    unique_lock_t lock(global_lock);
    bool skip = PreCallValidateQueueWaitIdle(queue);
    lock.unlock();
    if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
    VkResult result = dev_data->dispatch_table.QueueWaitIdle(queue);
    lock.lock();
    PostCallRecordQueueWaitIdle(queue, result);
    lock.unlock();
    return result;
}

VKAPI_ATTR VkResult VKAPI_CALL DeviceWaitIdle(VkDevice device) {
    layer_data *dev_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    unique_lock_t lock(global_lock);
    bool skip = PreCallValidateDeviceWaitIdle(device);
    lock.unlock();
    if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
    VkResult result = dev_data->dispatch_table.DeviceWaitIdle(device);
    lock.lock();
    PostCallRecordDeviceWaitIdle(device, result);
    lock.unlock();
    return result;
}

VKAPI_ATTR void VKAPI_CALL DestroyFence(VkDevice device, VkFence fence, const VkAllocationCallbacks *pAllocator) {
    layer_data *dev_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    unique_lock_t lock(global_lock);
    bool skip = PreCallValidateDestroyFence(device, fence, pAllocator);

    if (!skip) {
        // Pre-record to avoid Destroy/Create race
        PreCallRecordDestroyFence(device, fence, pAllocator);
        lock.unlock();
        dev_data->dispatch_table.DestroyFence(device, fence, pAllocator);
    }
}

VKAPI_ATTR void VKAPI_CALL DestroySemaphore(VkDevice device, VkSemaphore semaphore, const VkAllocationCallbacks *pAllocator) {
    layer_data *dev_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    unique_lock_t lock(global_lock);
    bool skip = PreCallValidateDestroySemaphore(device, semaphore, pAllocator);
    if (!skip) {
        // Pre-record to avoid Destroy/Create race
        PreCallRecordDestroySemaphore(device, semaphore, pAllocator);
        lock.unlock();
        dev_data->dispatch_table.DestroySemaphore(device, semaphore, pAllocator);
    }
}

VKAPI_ATTR void VKAPI_CALL DestroyEvent(VkDevice device, VkEvent event, const VkAllocationCallbacks *pAllocator) {
    layer_data *dev_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    unique_lock_t lock(global_lock);
    bool skip = PreCallValidateDestroyEvent(device, event, pAllocator);
    if (!skip) {
        PreCallRecordDestroyEvent(device, event, pAllocator);
        lock.unlock();
        dev_data->dispatch_table.DestroyEvent(device, event, pAllocator);
    }
}

VKAPI_ATTR void VKAPI_CALL DestroyQueryPool(VkDevice device, VkQueryPool queryPool, const VkAllocationCallbacks *pAllocator) {
    layer_data *dev_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    unique_lock_t lock(global_lock);
    bool skip = PreCallValidateDestroyQueryPool(device, queryPool, pAllocator);
    if (!skip) {
        PreCallRecordDestroyQueryPool(device, queryPool, pAllocator);
        lock.unlock();
        dev_data->dispatch_table.DestroyQueryPool(device, queryPool, pAllocator);
    }
}

VKAPI_ATTR VkResult VKAPI_CALL GetQueryPoolResults(VkDevice device, VkQueryPool queryPool, uint32_t firstQuery, uint32_t queryCount,
                                                   size_t dataSize, void *pData, VkDeviceSize stride, VkQueryResultFlags flags) {
    layer_data *dev_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    unique_lock_t lock(global_lock);
    bool skip = PreCallValidateGetQueryPoolResults(device, queryPool, firstQuery, queryCount, dataSize, pData, stride, flags);
    lock.unlock();
    if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
    VkResult result =
        dev_data->dispatch_table.GetQueryPoolResults(device, queryPool, firstQuery, queryCount, dataSize, pData, stride, flags);
    lock.lock();
    PostCallRecordGetQueryPoolResults(device, queryPool, firstQuery, queryCount, dataSize, pData, stride, flags, result);
    lock.unlock();
    return result;
}

VKAPI_ATTR void VKAPI_CALL DestroyBuffer(VkDevice device, VkBuffer buffer, const VkAllocationCallbacks *pAllocator) {
    layer_data *dev_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    unique_lock_t lock(global_lock);
    bool skip = PreCallValidateDestroyBuffer(device, buffer, pAllocator);
    if (!skip) {
        PreCallRecordDestroyBuffer(device, buffer, pAllocator);
        lock.unlock();
        dev_data->dispatch_table.DestroyBuffer(device, buffer, pAllocator);
    }
}

VKAPI_ATTR void VKAPI_CALL DestroyBufferView(VkDevice device, VkBufferView bufferView, const VkAllocationCallbacks *pAllocator) {
    layer_data *dev_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    unique_lock_t lock(global_lock);
    // Validate state before calling down chain, update common data if we'll be calling down chain
    bool skip = PreCallValidateDestroyBufferView(device, bufferView, pAllocator);
    if (!skip) {
        if (bufferView != VK_NULL_HANDLE) {
            // Pre-record to avoid Destroy/Create race
            PreCallRecordDestroyBufferView(device, bufferView, pAllocator);
        }
        lock.unlock();
        dev_data->dispatch_table.DestroyBufferView(device, bufferView, pAllocator);
    }
}

VKAPI_ATTR void VKAPI_CALL DestroyImage(VkDevice device, VkImage image, const VkAllocationCallbacks *pAllocator) {
    layer_data *dev_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    unique_lock_t lock(global_lock);
    bool skip = PreCallValidateDestroyImage(device, image, pAllocator);
    if (!skip) {
        if (image != VK_NULL_HANDLE) {
            // Pre-record to avoid Destroy/Create race
            PreCallRecordDestroyImage(device, image, pAllocator);
        }
        lock.unlock();
        dev_data->dispatch_table.DestroyImage(device, image, pAllocator);
    }
}

VKAPI_ATTR VkResult VKAPI_CALL BindBufferMemory(VkDevice device, VkBuffer buffer, VkDeviceMemory mem, VkDeviceSize memoryOffset) {
    layer_data *dev_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    VkResult result = VK_ERROR_VALIDATION_FAILED_EXT;
    unique_lock_t lock(global_lock);
    bool skip = PreCallValidateBindBufferMemory(device, buffer, mem, memoryOffset);
    lock.unlock();
    if (!skip) {
        result = dev_data->dispatch_table.BindBufferMemory(device, buffer, mem, memoryOffset);
        lock.lock();
        PostCallRecordBindBufferMemory(device, buffer, mem, memoryOffset, result);
        lock.unlock();
    }
    return result;
}

VKAPI_ATTR VkResult VKAPI_CALL BindBufferMemory2(VkDevice device, uint32_t bindInfoCount,
                                                 const VkBindBufferMemoryInfoKHR *pBindInfos) {
    layer_data *dev_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    VkResult result = VK_ERROR_VALIDATION_FAILED_EXT;
    bool skip = false;
    unique_lock_t lock(global_lock);
    skip = PreCallValidateBindBufferMemory2(device, bindInfoCount, pBindInfos);
    lock.unlock();
    if (!skip) {
        result = dev_data->dispatch_table.BindBufferMemory2(device, bindInfoCount, pBindInfos);
        lock.lock();
        PostCallRecordBindBufferMemory2(device, bindInfoCount, pBindInfos, result);
        lock.unlock();
    }
    return result;
}

VKAPI_ATTR VkResult VKAPI_CALL BindBufferMemory2KHR(VkDevice device, uint32_t bindInfoCount,
                                                    const VkBindBufferMemoryInfoKHR *pBindInfos) {
    layer_data *dev_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    VkResult result = VK_ERROR_VALIDATION_FAILED_EXT;
    bool skip = false;
    unique_lock_t lock(global_lock);
    skip = PreCallValidateBindBufferMemory2KHR(device, bindInfoCount, pBindInfos);
    lock.unlock();
    if (!skip) {
        result = dev_data->dispatch_table.BindBufferMemory2KHR(device, bindInfoCount, pBindInfos);
        lock.lock();
        PostCallRecordBindBufferMemory2KHR(device, bindInfoCount, pBindInfos, result);
        lock.unlock();
    }
    return result;
}

VKAPI_ATTR void VKAPI_CALL GetBufferMemoryRequirements(VkDevice device, VkBuffer buffer,
                                                       VkMemoryRequirements *pMemoryRequirements) {
    layer_data *dev_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    dev_data->dispatch_table.GetBufferMemoryRequirements(device, buffer, pMemoryRequirements);
    PostCallRecordGetBufferMemoryRequirements(device, buffer, pMemoryRequirements);
}

VKAPI_ATTR void VKAPI_CALL GetBufferMemoryRequirements2(VkDevice device, const VkBufferMemoryRequirementsInfo2KHR *pInfo,
                                                        VkMemoryRequirements2KHR *pMemoryRequirements) {
    layer_data *dev_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    dev_data->dispatch_table.GetBufferMemoryRequirements2(device, pInfo, pMemoryRequirements);
    PostCallRecordGetBufferMemoryRequirements2(device, pInfo, pMemoryRequirements);
}

VKAPI_ATTR void VKAPI_CALL GetBufferMemoryRequirements2KHR(VkDevice device, const VkBufferMemoryRequirementsInfo2KHR *pInfo,
                                                           VkMemoryRequirements2KHR *pMemoryRequirements) {
    layer_data *dev_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    dev_data->dispatch_table.GetBufferMemoryRequirements2KHR(device, pInfo, pMemoryRequirements);
    PostCallRecordGetBufferMemoryRequirements2KHR(device, pInfo, pMemoryRequirements);
}

VKAPI_ATTR void VKAPI_CALL GetImageMemoryRequirements(VkDevice device, VkImage image, VkMemoryRequirements *pMemoryRequirements) {
    layer_data *dev_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    dev_data->dispatch_table.GetImageMemoryRequirements(device, image, pMemoryRequirements);
    unique_lock_t lock(global_lock);
    PostCallRecordGetImageMemoryRequirements(device, image, pMemoryRequirements);
}

VKAPI_ATTR void VKAPI_CALL GetImageMemoryRequirements2(VkDevice device, const VkImageMemoryRequirementsInfo2 *pInfo,
                                                       VkMemoryRequirements2 *pMemoryRequirements) {
    layer_data *dev_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    unique_lock_t lock(global_lock);
    bool skip = PreCallValidateGetImageMemoryRequirements2(device, pInfo, pMemoryRequirements);
    if (skip) return;
    lock.unlock();
    dev_data->dispatch_table.GetImageMemoryRequirements2(device, pInfo, pMemoryRequirements);
    lock.lock();
    PostCallRecordGetImageMemoryRequirements2(device, pInfo, pMemoryRequirements);
}

VKAPI_ATTR void VKAPI_CALL GetImageMemoryRequirements2KHR(VkDevice device, const VkImageMemoryRequirementsInfo2 *pInfo,
                                                          VkMemoryRequirements2 *pMemoryRequirements) {
    layer_data *dev_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    unique_lock_t lock(global_lock);
    bool skip = PreCallValidateGetImageMemoryRequirements2(device, pInfo, pMemoryRequirements);
    if (skip) return;
    lock.unlock();
    dev_data->dispatch_table.GetImageMemoryRequirements2KHR(device, pInfo, pMemoryRequirements);
    lock.lock();
    PostCallRecordGetImageMemoryRequirements2KHR(device, pInfo, pMemoryRequirements);
}

VKAPI_ATTR void VKAPI_CALL GetImageSparseMemoryRequirements(VkDevice device, VkImage image, uint32_t *pSparseMemoryRequirementCount,
                                                            VkSparseImageMemoryRequirements *pSparseMemoryRequirements) {
    // TODO : Implement tracking here, just passthrough initially
    layer_data *dev_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    dev_data->dispatch_table.GetImageSparseMemoryRequirements(device, image, pSparseMemoryRequirementCount,
                                                              pSparseMemoryRequirements);
    unique_lock_t lock(global_lock);
    PostCallRecordGetImageSparseMemoryRequirements(device, image, pSparseMemoryRequirementCount, pSparseMemoryRequirements);
}

VKAPI_ATTR void VKAPI_CALL GetImageSparseMemoryRequirements2(VkDevice device, const VkImageSparseMemoryRequirementsInfo2KHR *pInfo,
                                                             uint32_t *pSparseMemoryRequirementCount,
                                                             VkSparseImageMemoryRequirements2KHR *pSparseMemoryRequirements) {
    // TODO : Implement tracking here, just passthrough initially
    layer_data *dev_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    dev_data->dispatch_table.GetImageSparseMemoryRequirements2(device, pInfo, pSparseMemoryRequirementCount,
                                                               pSparseMemoryRequirements);
    unique_lock_t lock(global_lock);
    PostCallRecordGetImageSparseMemoryRequirements2(device, pInfo, pSparseMemoryRequirementCount, pSparseMemoryRequirements);
}

VKAPI_ATTR void VKAPI_CALL GetImageSparseMemoryRequirements2KHR(VkDevice device,
                                                                const VkImageSparseMemoryRequirementsInfo2KHR *pInfo,
                                                                uint32_t *pSparseMemoryRequirementCount,
                                                                VkSparseImageMemoryRequirements2KHR *pSparseMemoryRequirements) {
    // TODO : Implement tracking here, just passthrough initially
    layer_data *dev_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    dev_data->dispatch_table.GetImageSparseMemoryRequirements2KHR(device, pInfo, pSparseMemoryRequirementCount,
                                                                  pSparseMemoryRequirements);
    unique_lock_t lock(global_lock);
    PostCallRecordGetImageSparseMemoryRequirements2(device, pInfo, pSparseMemoryRequirementCount, pSparseMemoryRequirements);
}

VKAPI_ATTR void VKAPI_CALL GetPhysicalDeviceSparseImageFormatProperties(VkPhysicalDevice physicalDevice, VkFormat format,
                                                                        VkImageType type, VkSampleCountFlagBits samples,
                                                                        VkImageUsageFlags usage, VkImageTiling tiling,
                                                                        uint32_t *pPropertyCount,
                                                                        VkSparseImageFormatProperties *pProperties) {
    // TODO : Implement this intercept, track sparse image format properties and make sure they are obeyed.
    instance_layer_data *instance_data = GetLayerDataPtr(get_dispatch_key(physicalDevice), instance_layer_data_map);
    instance_data->dispatch_table.GetPhysicalDeviceSparseImageFormatProperties(physicalDevice, format, type, samples, usage, tiling,
                                                                               pPropertyCount, pProperties);
}

VKAPI_ATTR VkResult VKAPI_CALL GetPhysicalDeviceImageFormatProperties2(VkPhysicalDevice physicalDevice,
                                                                       const VkPhysicalDeviceImageFormatInfo2 *pImageFormatInfo,
                                                                       VkImageFormatProperties2 *pImageFormatProperties) {
    instance_layer_data *instance_data = GetLayerDataPtr(get_dispatch_key(physicalDevice), instance_layer_data_map);
    unique_lock_t lock(global_lock);
    bool skip = PreCallValidateGetPhysicalDeviceImageFormatProperties2(physicalDevice, pImageFormatInfo, pImageFormatProperties);
    lock.unlock();
    if (skip) {
        return VK_ERROR_VALIDATION_FAILED_EXT;
    } else {
        return instance_data->dispatch_table.GetPhysicalDeviceImageFormatProperties2(physicalDevice, pImageFormatInfo,
                                                                                     pImageFormatProperties);
    }
}

VKAPI_ATTR VkResult VKAPI_CALL GetPhysicalDeviceImageFormatProperties2KHR(VkPhysicalDevice physicalDevice,
                                                                          const VkPhysicalDeviceImageFormatInfo2 *pImageFormatInfo,
                                                                          VkImageFormatProperties2 *pImageFormatProperties) {
    instance_layer_data *instance_data = GetLayerDataPtr(get_dispatch_key(physicalDevice), instance_layer_data_map);
    unique_lock_t lock(global_lock);
    bool skip = PreCallValidateGetPhysicalDeviceImageFormatProperties2KHR(physicalDevice, pImageFormatInfo, pImageFormatProperties);
    lock.unlock();
    if (skip) {
        return VK_ERROR_VALIDATION_FAILED_EXT;
    } else {
        return instance_data->dispatch_table.GetPhysicalDeviceImageFormatProperties2KHR(physicalDevice, pImageFormatInfo,
                                                                                        pImageFormatProperties);
    }
}

VKAPI_ATTR void VKAPI_CALL GetPhysicalDeviceSparseImageFormatProperties2(
    VkPhysicalDevice physicalDevice, const VkPhysicalDeviceSparseImageFormatInfo2KHR *pFormatInfo, uint32_t *pPropertyCount,
    VkSparseImageFormatProperties2KHR *pProperties) {
    // TODO : Implement this intercept, track sparse image format properties and make sure they are obeyed.
    instance_layer_data *instance_data = GetLayerDataPtr(get_dispatch_key(physicalDevice), instance_layer_data_map);
    instance_data->dispatch_table.GetPhysicalDeviceSparseImageFormatProperties2(physicalDevice, pFormatInfo, pPropertyCount,
                                                                                pProperties);
}

VKAPI_ATTR void VKAPI_CALL GetPhysicalDeviceSparseImageFormatProperties2KHR(
    VkPhysicalDevice physicalDevice, const VkPhysicalDeviceSparseImageFormatInfo2KHR *pFormatInfo, uint32_t *pPropertyCount,
    VkSparseImageFormatProperties2KHR *pProperties) {
    // TODO : Implement this intercept, track sparse image format properties and make sure they are obeyed.
    instance_layer_data *instance_data = GetLayerDataPtr(get_dispatch_key(physicalDevice), instance_layer_data_map);
    instance_data->dispatch_table.GetPhysicalDeviceSparseImageFormatProperties2KHR(physicalDevice, pFormatInfo, pPropertyCount,
                                                                                   pProperties);
}

VKAPI_ATTR void VKAPI_CALL DestroyImageView(VkDevice device, VkImageView imageView, const VkAllocationCallbacks *pAllocator) {
    layer_data *dev_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    unique_lock_t lock(global_lock);
    bool skip = PreCallValidateDestroyImageView(device, imageView, pAllocator);
    if (!skip) {
        PreCallRecordDestroyImageView(device, imageView, pAllocator);
        lock.unlock();
        dev_data->dispatch_table.DestroyImageView(device, imageView, pAllocator);
    }
}

VKAPI_ATTR void VKAPI_CALL DestroyShaderModule(VkDevice device, VkShaderModule shaderModule,
                                               const VkAllocationCallbacks *pAllocator) {
    layer_data *dev_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    unique_lock_t lock(global_lock);
    PreCallRecordDestroyShaderModule(device, shaderModule, pAllocator);
    lock.unlock();
    dev_data->dispatch_table.DestroyShaderModule(device, shaderModule, pAllocator);
}

VKAPI_ATTR void VKAPI_CALL DestroyPipeline(VkDevice device, VkPipeline pipeline, const VkAllocationCallbacks *pAllocator) {
    layer_data *dev_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    unique_lock_t lock(global_lock);
    bool skip = PreCallValidateDestroyPipeline(device, pipeline, pAllocator);
    if (!skip) {
        PreCallRecordDestroyPipeline(device, pipeline, pAllocator);
        lock.unlock();
        dev_data->dispatch_table.DestroyPipeline(device, pipeline, pAllocator);
    }
}

VKAPI_ATTR void VKAPI_CALL DestroyPipelineLayout(VkDevice device, VkPipelineLayout pipelineLayout,
                                                 const VkAllocationCallbacks *pAllocator) {
    layer_data *dev_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    unique_lock_t lock(global_lock);
    // Pre-record to avoid Destroy/Create race
    PreCallRecordDestroyPipelineLayout(device, pipelineLayout, pAllocator);
    lock.unlock();

    dev_data->dispatch_table.DestroyPipelineLayout(device, pipelineLayout, pAllocator);
}

VKAPI_ATTR void VKAPI_CALL DestroySampler(VkDevice device, VkSampler sampler, const VkAllocationCallbacks *pAllocator) {
    layer_data *dev_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    unique_lock_t lock(global_lock);
    bool skip = PreCallValidateDestroySampler(device, sampler, pAllocator);
    if (!skip) {
        PreCallRecordDestroySampler(device, sampler, pAllocator);
        lock.unlock();
        dev_data->dispatch_table.DestroySampler(device, sampler, pAllocator);
    }
}

VKAPI_ATTR void VKAPI_CALL DestroyDescriptorSetLayout(VkDevice device, VkDescriptorSetLayout descriptorSetLayout,
                                                      const VkAllocationCallbacks *pAllocator) {
    layer_data *dev_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    {
        lock_guard_t lock(global_lock);
        // Pre-record to avoid Destroy/Create race
        PreCallRecordDestroyDescriptorSetLayout(device, descriptorSetLayout, pAllocator);
    }
    dev_data->dispatch_table.DestroyDescriptorSetLayout(device, descriptorSetLayout, pAllocator);
}

VKAPI_ATTR void VKAPI_CALL DestroyDescriptorPool(VkDevice device, VkDescriptorPool descriptorPool,
                                                 const VkAllocationCallbacks *pAllocator) {
    layer_data *dev_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    unique_lock_t lock(global_lock);
    bool skip = PreCallValidateDestroyDescriptorPool(device, descriptorPool, pAllocator);
    if (!skip) {
        PreCallRecordDestroyDescriptorPool(device, descriptorPool, pAllocator);
        lock.unlock();
        dev_data->dispatch_table.DestroyDescriptorPool(device, descriptorPool, pAllocator);
    }
}

VKAPI_ATTR void VKAPI_CALL FreeCommandBuffers(VkDevice device, VkCommandPool commandPool, uint32_t commandBufferCount,
                                              const VkCommandBuffer *pCommandBuffers) {
    layer_data *dev_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    unique_lock_t lock(global_lock);

    bool skip = PreCallValidateFreeCommandBuffers(device, commandPool, commandBufferCount, pCommandBuffers);
    if (skip) return;
    PreCallRecordFreeCommandBuffers(device, commandPool, commandBufferCount, pCommandBuffers);
    lock.unlock();

    dev_data->dispatch_table.FreeCommandBuffers(device, commandPool, commandBufferCount, pCommandBuffers);
}

VKAPI_ATTR VkResult VKAPI_CALL CreateCommandPool(VkDevice device, const VkCommandPoolCreateInfo *pCreateInfo,
                                                 const VkAllocationCallbacks *pAllocator, VkCommandPool *pCommandPool) {
    layer_data *dev_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    VkResult result = dev_data->dispatch_table.CreateCommandPool(device, pCreateInfo, pAllocator, pCommandPool);
    lock_guard_t lock(global_lock);
    PostCallRecordCreateCommandPool(device, pCreateInfo, pAllocator, pCommandPool, result);
    return result;
}

VKAPI_ATTR VkResult VKAPI_CALL CreateQueryPool(VkDevice device, const VkQueryPoolCreateInfo *pCreateInfo,
                                               const VkAllocationCallbacks *pAllocator, VkQueryPool *pQueryPool) {
    layer_data *dev_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    unique_lock_t lock(global_lock);
    bool skip = PreCallValidateCreateQueryPool(device, pCreateInfo, pAllocator, pQueryPool);
    lock.unlock();

    VkResult result = VK_ERROR_VALIDATION_FAILED_EXT;
    if (!skip) {
        result = dev_data->dispatch_table.CreateQueryPool(device, pCreateInfo, pAllocator, pQueryPool);
    }
    if (result == VK_SUCCESS) {
        lock.lock();
        PostCallRecordCreateQueryPool(device, pCreateInfo, pAllocator, pQueryPool, result);
    }
    return result;
}

VKAPI_ATTR void VKAPI_CALL DestroyCommandPool(VkDevice device, VkCommandPool commandPool, const VkAllocationCallbacks *pAllocator) {
    layer_data *dev_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    unique_lock_t lock(global_lock);
    bool skip = PreCallValidateDestroyCommandPool(device, commandPool, pAllocator);
    if (!skip) {
        // Pre-record to avoid Destroy/Create race
        PreCallRecordDestroyCommandPool(device, commandPool, pAllocator);
        lock.unlock();
        dev_data->dispatch_table.DestroyCommandPool(device, commandPool, pAllocator);
    }
}

VKAPI_ATTR VkResult VKAPI_CALL ResetCommandPool(VkDevice device, VkCommandPool commandPool, VkCommandPoolResetFlags flags) {
    layer_data *dev_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    unique_lock_t lock(global_lock);
    bool skip = PreCallValidateResetCommandPool(device, commandPool, flags);
    lock.unlock();
    if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;

    VkResult result = dev_data->dispatch_table.ResetCommandPool(device, commandPool, flags);
    lock.lock();
    PostCallRecordResetCommandPool(device, commandPool, flags, result);
    lock.unlock();
    return result;
}

VKAPI_ATTR VkResult VKAPI_CALL ResetFences(VkDevice device, uint32_t fenceCount, const VkFence *pFences) {
    layer_data *dev_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    unique_lock_t lock(global_lock);
    bool skip = PreCallValidateResetFences(device, fenceCount, pFences);
    lock.unlock();
    if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;

    VkResult result = dev_data->dispatch_table.ResetFences(device, fenceCount, pFences);
    lock.lock();
    PostCallRecordResetFences(device, fenceCount, pFences, result);
    lock.unlock();
    return result;
}

VKAPI_ATTR void VKAPI_CALL DestroyFramebuffer(VkDevice device, VkFramebuffer framebuffer, const VkAllocationCallbacks *pAllocator) {
    layer_data *dev_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    unique_lock_t lock(global_lock);
    bool skip = PreCallValidateDestroyFramebuffer(device, framebuffer, pAllocator);
    if (!skip) {
        PreCallRecordDestroyFramebuffer(device, framebuffer, pAllocator);
        lock.unlock();
        dev_data->dispatch_table.DestroyFramebuffer(device, framebuffer, pAllocator);
    }
}

VKAPI_ATTR void VKAPI_CALL DestroyRenderPass(VkDevice device, VkRenderPass renderPass, const VkAllocationCallbacks *pAllocator) {
    layer_data *dev_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    unique_lock_t lock(global_lock);
    bool skip = PreCallValidateDestroyRenderPass(device, renderPass, pAllocator);
    if (!skip) {
        PreCallRecordDestroyRenderPass(device, renderPass, pAllocator);
        lock.unlock();
        dev_data->dispatch_table.DestroyRenderPass(device, renderPass, pAllocator);
    }
}

VKAPI_ATTR VkResult VKAPI_CALL CreateBuffer(VkDevice device, const VkBufferCreateInfo *pCreateInfo,
                                            const VkAllocationCallbacks *pAllocator, VkBuffer *pBuffer) {
    layer_data *dev_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    unique_lock_t lock(global_lock);
    bool skip = PreCallValidateCreateBuffer(device, pCreateInfo, pAllocator, pBuffer);
    lock.unlock();

    if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
    VkResult result = dev_data->dispatch_table.CreateBuffer(device, pCreateInfo, pAllocator, pBuffer);

    lock.lock();
    PostCallRecordCreateBuffer(device, pCreateInfo, pAllocator, pBuffer, result);
    lock.unlock();
    return result;
}

VKAPI_ATTR VkResult VKAPI_CALL CreateBufferView(VkDevice device, const VkBufferViewCreateInfo *pCreateInfo,
                                                const VkAllocationCallbacks *pAllocator, VkBufferView *pView) {
    layer_data *dev_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    unique_lock_t lock(global_lock);
    bool skip = PreCallValidateCreateBufferView(device, pCreateInfo, pAllocator, pView);
    lock.unlock();
    if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
    VkResult result = dev_data->dispatch_table.CreateBufferView(device, pCreateInfo, pAllocator, pView);
    lock.lock();
    PostCallRecordCreateBufferView(device, pCreateInfo, pAllocator, pView, result);
    lock.unlock();
    return result;
}

VKAPI_ATTR VkResult VKAPI_CALL CreateImage(VkDevice device, const VkImageCreateInfo *pCreateInfo,
                                           const VkAllocationCallbacks *pAllocator, VkImage *pImage) {
    VkResult result = VK_ERROR_VALIDATION_FAILED_EXT;
    layer_data *dev_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    unique_lock_t lock(global_lock);
    bool skip = PreCallValidateCreateImage(device, pCreateInfo, pAllocator, pImage);
    lock.unlock();
    if (!skip) {
        result = dev_data->dispatch_table.CreateImage(device, pCreateInfo, pAllocator, pImage);
    }
    lock.lock();
    PostCallRecordCreateImage(device, pCreateInfo, pAllocator, pImage, result);
    lock.unlock();
    return result;
}

VKAPI_ATTR VkResult VKAPI_CALL CreateImageView(VkDevice device, const VkImageViewCreateInfo *pCreateInfo,
                                               const VkAllocationCallbacks *pAllocator, VkImageView *pView) {
    layer_data *dev_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    unique_lock_t lock(global_lock);
    bool skip = PreCallValidateCreateImageView(device, pCreateInfo, pAllocator, pView);
    lock.unlock();
    if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
    VkResult result = dev_data->dispatch_table.CreateImageView(device, pCreateInfo, pAllocator, pView);
    lock.lock();
    PostCallRecordCreateImageView(device, pCreateInfo, pAllocator, pView, result);
    lock.unlock();
    return result;
}

VKAPI_ATTR VkResult VKAPI_CALL CreateValidationCacheEXT(VkDevice device, const VkValidationCacheCreateInfoEXT *pCreateInfo,
                                                        const VkAllocationCallbacks *pAllocator,
                                                        VkValidationCacheEXT *pValidationCache) {
    *pValidationCache = ValidationCache::Create(pCreateInfo);
    return *pValidationCache ? VK_SUCCESS : VK_ERROR_INITIALIZATION_FAILED;
}

VKAPI_ATTR void VKAPI_CALL DestroyValidationCacheEXT(VkDevice device, VkValidationCacheEXT validationCache,
                                                     const VkAllocationCallbacks *pAllocator) {
    delete (ValidationCache *)validationCache;
}

VKAPI_ATTR VkResult VKAPI_CALL GetValidationCacheDataEXT(VkDevice device, VkValidationCacheEXT validationCache, size_t *pDataSize,
                                                         void *pData) {
    size_t inSize = *pDataSize;
    ((ValidationCache *)validationCache)->Write(pDataSize, pData);
    return (pData && *pDataSize != inSize) ? VK_INCOMPLETE : VK_SUCCESS;
}

VKAPI_ATTR VkResult VKAPI_CALL MergeValidationCachesEXT(VkDevice device, VkValidationCacheEXT dstCache, uint32_t srcCacheCount,
                                                        const VkValidationCacheEXT *pSrcCaches) {
    layer_data *dev_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    bool skip = false;
    auto dst = (ValidationCache *)dstCache;
    auto src = (ValidationCache const *const *)pSrcCaches;
    VkResult result = VK_SUCCESS;
    for (uint32_t i = 0; i < srcCacheCount; i++) {
        if (src[i] == dst) {
            skip |= log_msg(dev_data->report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_VALIDATION_CACHE_EXT,
                            0, "VUID-vkMergeValidationCachesEXT-dstCache-01536",
                            "vkMergeValidationCachesEXT: dstCache (0x%" PRIx64 ") must not appear in pSrcCaches array.",
                            HandleToUint64(dstCache));
            result = VK_ERROR_VALIDATION_FAILED_EXT;
        }
        if (!skip) {
            dst->Merge(src[i]);
        }
    }

    return result;
}

#ifdef VK_USE_PLATFORM_ANDROID_KHR
VKAPI_ATTR VkResult VKAPI_CALL GetAndroidHardwareBufferPropertiesANDROID(VkDevice device, const struct AHardwareBuffer *buffer,
                                                                         VkAndroidHardwareBufferPropertiesANDROID *pProperties) {
    layer_data *dev_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    unique_lock_t lock(global_lock);
    bool skip = PreCallValidateGetAndroidHardwareBufferProperties(device, buffer, pProperties);
    if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
    lock.unlock();
    VkResult result = dev_data->dispatch_table.GetAndroidHardwareBufferPropertiesANDROID(device, buffer, pProperties);
    lock.lock();
    PostCallRecordGetAndroidHardwareBufferProperties(device, buffer, pProperties, result);
    return result;
}

VKAPI_ATTR VkResult VKAPI_CALL GetMemoryAndroidHardwareBufferANDROID(VkDevice device,
                                                                     const VkMemoryGetAndroidHardwareBufferInfoANDROID *pInfo,
                                                                     struct AHardwareBuffer **pBuffer) {
    layer_data *dev_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    unique_lock_t lock(global_lock);
    bool skip = PreCallValidateGetMemoryAndroidHardwareBuffer(device, pInfo, pBuffer);
    lock.unlock();
    if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
    return dev_data->dispatch_table.GetMemoryAndroidHardwareBufferANDROID(device, pInfo, pBuffer);
}
#endif  // VK_USE_PLATFORM_ANDROID_KHR

VKAPI_ATTR VkResult VKAPI_CALL CreateComputePipelines(VkDevice device, VkPipelineCache pipelineCache, uint32_t count,
                                                      const VkComputePipelineCreateInfo *pCreateInfos,
                                                      const VkAllocationCallbacks *pAllocator, VkPipeline *pPipelines) {
    std::vector<std::unique_ptr<PIPELINE_STATE>> pipe_state;
    layer_data *dev_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);

    unique_lock_t lock(global_lock);
    bool skip = PreCallValidateCreateComputePipelines(dev_data, &pipe_state, count, pCreateInfos);
    if (skip) {
        for (uint32_t i = 0; i < count; i++) {
            pPipelines[i] = VK_NULL_HANDLE;
        }
        return VK_ERROR_VALIDATION_FAILED_EXT;
    }
    lock.unlock();

    auto result =
        dev_data->dispatch_table.CreateComputePipelines(device, pipelineCache, count, pCreateInfos, pAllocator, pPipelines);

    lock.lock();
    PostCallRecordCreateComputePipelines(dev_data, &pipe_state, count, pPipelines);

    return result;
}

VKAPI_ATTR VkResult VKAPI_CALL CreateRayTracingPipelinesNV(VkDevice device, VkPipelineCache pipelineCache, uint32_t count,
                                                           const VkRayTracingPipelineCreateInfoNV *pCreateInfos,
                                                           const VkAllocationCallbacks *pAllocator, VkPipeline *pPipelines) {
    bool skip = false;
    std::vector<std::unique_ptr<PIPELINE_STATE>> pipe_state;
    pipe_state.reserve(count);
    layer_data *dev_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);

    unique_lock_t lock(global_lock);

    skip |= PreCallValidateCreateRayTracingPipelinesNV(dev_data, count, pCreateInfos, pipe_state);

    lock.unlock();

    if (skip) {
        for (uint32_t i = 0; i < count; i++) {
            pPipelines[i] = VK_NULL_HANDLE;
        }
        return VK_ERROR_VALIDATION_FAILED_EXT;
    }

    auto result =
        dev_data->dispatch_table.CreateRayTracingPipelinesNV(device, pipelineCache, count, pCreateInfos, pAllocator, pPipelines);
    lock.lock();

    PostCallRecordCreateRayTracingPipelinesNV(dev_data, count, pipe_state, pPipelines);

    return result;
}

VKAPI_ATTR VkResult VKAPI_CALL CreateSampler(VkDevice device, const VkSamplerCreateInfo *pCreateInfo,
                                             const VkAllocationCallbacks *pAllocator, VkSampler *pSampler) {
    layer_data *dev_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    VkResult result = dev_data->dispatch_table.CreateSampler(device, pCreateInfo, pAllocator, pSampler);
    lock_guard_t lock(global_lock);
    PostCallRecordCreateSampler(device, pCreateInfo, pAllocator, pSampler, result);
    return result;
}

VKAPI_ATTR VkResult VKAPI_CALL CreateDescriptorSetLayout(VkDevice device, const VkDescriptorSetLayoutCreateInfo *pCreateInfo,
                                                         const VkAllocationCallbacks *pAllocator,
                                                         VkDescriptorSetLayout *pSetLayout) {
    layer_data *dev_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    VkResult result = VK_ERROR_VALIDATION_FAILED_EXT;
    unique_lock_t lock(global_lock);
    bool skip = PreCallValidateCreateDescriptorSetLayout(device, pCreateInfo, pAllocator, pSetLayout);
    if (!skip) {
        lock.unlock();
        result = dev_data->dispatch_table.CreateDescriptorSetLayout(device, pCreateInfo, pAllocator, pSetLayout);
        lock.lock();
        PostCallRecordCreateDescriptorSetLayout(device, pCreateInfo, pAllocator, pSetLayout, result);
    }
    return result;
}

VKAPI_ATTR VkResult VKAPI_CALL CreateDescriptorPool(VkDevice device, const VkDescriptorPoolCreateInfo *pCreateInfo,
                                                    const VkAllocationCallbacks *pAllocator, VkDescriptorPool *pDescriptorPool) {
    layer_data *dev_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    VkResult result = dev_data->dispatch_table.CreateDescriptorPool(device, pCreateInfo, pAllocator, pDescriptorPool);
    lock_guard_t lock(global_lock);
    PostCallRecordCreateDescriptorPool(device, pCreateInfo, pAllocator, pDescriptorPool, result);
    return result;
}

VKAPI_ATTR VkResult VKAPI_CALL ResetDescriptorPool(VkDevice device, VkDescriptorPool descriptorPool,
                                                   VkDescriptorPoolResetFlags flags) {
    layer_data *dev_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);

    unique_lock_t lock(global_lock);
    bool skip = PreCallValidateResetDescriptorPool(device, descriptorPool, flags);
    lock.unlock();
    if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
    VkResult result = dev_data->dispatch_table.ResetDescriptorPool(device, descriptorPool, flags);
    lock.lock();
    PostCallRecordResetDescriptorPool(device, descriptorPool, flags, result);
    lock.unlock();
    return result;
}

// TODO: PostCallRecord routine is dependent on data generated in PreCallValidate -- needs to be moved out
VKAPI_ATTR VkResult VKAPI_CALL AllocateDescriptorSets(VkDevice device, const VkDescriptorSetAllocateInfo *pAllocateInfo,
                                                      VkDescriptorSet *pDescriptorSets) {
    layer_data *dev_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    unique_lock_t lock(global_lock);
    cvdescriptorset::AllocateDescriptorSetsData common_data(pAllocateInfo->descriptorSetCount);
    bool skip = PreCallValidateAllocateDescriptorSets(dev_data, pAllocateInfo, &common_data);
    lock.unlock();

    if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;

    VkResult result = dev_data->dispatch_table.AllocateDescriptorSets(device, pAllocateInfo, pDescriptorSets);

    if (VK_SUCCESS == result) {
        lock.lock();
        PostCallRecordAllocateDescriptorSets(dev_data, pAllocateInfo, pDescriptorSets, &common_data);
        lock.unlock();
    }
    return result;
}

VKAPI_ATTR VkResult VKAPI_CALL FreeDescriptorSets(VkDevice device, VkDescriptorPool descriptorPool, uint32_t count,
                                                  const VkDescriptorSet *pDescriptorSets) {
    layer_data *dev_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    unique_lock_t lock(global_lock);
    bool skip = PreCallValidateFreeDescriptorSets(device, descriptorPool, count, pDescriptorSets);

    VkResult result = VK_ERROR_VALIDATION_FAILED_EXT;
    if (!skip) {
        PreCallRecordFreeDescriptorSets(device, descriptorPool, count, pDescriptorSets);
        lock.unlock();
        result = dev_data->dispatch_table.FreeDescriptorSets(device, descriptorPool, count, pDescriptorSets);
    }
    return result;
}

VKAPI_ATTR void VKAPI_CALL UpdateDescriptorSets(VkDevice device, uint32_t descriptorWriteCount,
                                                const VkWriteDescriptorSet *pDescriptorWrites, uint32_t descriptorCopyCount,
                                                const VkCopyDescriptorSet *pDescriptorCopies) {
    // Only map look-up at top level is for device-level layer_data
    layer_data *dev_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    unique_lock_t lock(global_lock);
    bool skip = PreCallValidateUpdateDescriptorSets(device, descriptorWriteCount, pDescriptorWrites, descriptorCopyCount,
                                                    pDescriptorCopies);
    if (!skip) {
        PreCallRecordUpdateDescriptorSets(device, descriptorWriteCount, pDescriptorWrites, descriptorCopyCount, pDescriptorCopies);
        lock.unlock();
        dev_data->dispatch_table.UpdateDescriptorSets(device, descriptorWriteCount, pDescriptorWrites, descriptorCopyCount,
                                                      pDescriptorCopies);
    }
}

VKAPI_ATTR VkResult VKAPI_CALL AllocateCommandBuffers(VkDevice device, const VkCommandBufferAllocateInfo *pCreateInfo,
                                                      VkCommandBuffer *pCommandBuffer) {
    layer_data *dev_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    VkResult result = dev_data->dispatch_table.AllocateCommandBuffers(device, pCreateInfo, pCommandBuffer);
    unique_lock_t lock(global_lock);
    PostCallRecordAllocateCommandBuffers(device, pCreateInfo, pCommandBuffer, result);
    lock.unlock();
    return result;
}

VKAPI_ATTR VkResult VKAPI_CALL BeginCommandBuffer(VkCommandBuffer commandBuffer, const VkCommandBufferBeginInfo *pBeginInfo) {
    bool skip = false;
    layer_data *dev_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    unique_lock_t lock(global_lock);
    skip |= PreCallValidateBeginCommandBuffer(commandBuffer, pBeginInfo);
    PreCallRecordBeginCommandBuffer(commandBuffer, pBeginInfo);
    lock.unlock();
    if (skip) {
        return VK_ERROR_VALIDATION_FAILED_EXT;
    }
    VkResult result = dev_data->dispatch_table.BeginCommandBuffer(commandBuffer, pBeginInfo);

    return result;
}

VKAPI_ATTR VkResult VKAPI_CALL EndCommandBuffer(VkCommandBuffer commandBuffer) {
    bool skip = false;
    layer_data *dev_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    unique_lock_t lock(global_lock);
    VkResult result = VK_ERROR_VALIDATION_FAILED_EXT;
    skip |= PreCallValidateEndCommandBuffer(commandBuffer);
    lock.unlock();
    if (!skip) {
        result = dev_data->dispatch_table.EndCommandBuffer(commandBuffer);
        lock.lock();
        PostCallRecordEndCommandBuffer(commandBuffer, result);
        lock.unlock();
    }
    return result;
}

VKAPI_ATTR VkResult VKAPI_CALL ResetCommandBuffer(VkCommandBuffer commandBuffer, VkCommandBufferResetFlags flags) {
    layer_data *dev_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    VkResult result = VK_ERROR_VALIDATION_FAILED_EXT;
    unique_lock_t lock(global_lock);
    bool skip = PreCallValidateResetCommandBuffer(commandBuffer, flags);
    lock.unlock();

    if (!skip) result = dev_data->dispatch_table.ResetCommandBuffer(commandBuffer, flags);
    lock.lock();
    PostCallRecordResetCommandBuffer(commandBuffer, flags, result);
    lock.unlock();
    return result;
}

VKAPI_ATTR void VKAPI_CALL CmdSetViewport(VkCommandBuffer commandBuffer, uint32_t firstViewport, uint32_t viewportCount,
                                          const VkViewport *pViewports) {
    bool skip = false;
    layer_data *dev_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    unique_lock_t lock(global_lock);
    skip |= PreCallValidateCmdSetViewport(commandBuffer, firstViewport, viewportCount, pViewports);
    if (!skip) {
        PreCallRecordCmdSetViewport(commandBuffer, firstViewport, viewportCount, pViewports);
    }
    lock.unlock();
    if (!skip) dev_data->dispatch_table.CmdSetViewport(commandBuffer, firstViewport, viewportCount, pViewports);
}

VKAPI_ATTR void VKAPI_CALL CmdSetScissor(VkCommandBuffer commandBuffer, uint32_t firstScissor, uint32_t scissorCount,
                                         const VkRect2D *pScissors) {
    bool skip = false;
    layer_data *dev_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    unique_lock_t lock(global_lock);
    skip |= PreCallValidateCmdSetScissor(commandBuffer, firstScissor, scissorCount, pScissors);
    if (!skip) {
        PreCallRecordCmdSetScissor(commandBuffer, firstScissor, scissorCount, pScissors);
    }
    lock.unlock();
    if (!skip) dev_data->dispatch_table.CmdSetScissor(commandBuffer, firstScissor, scissorCount, pScissors);
}

VKAPI_ATTR void VKAPI_CALL CmdSetExclusiveScissorNV(VkCommandBuffer commandBuffer, uint32_t firstExclusiveScissor,
                                                    uint32_t exclusiveScissorCount, const VkRect2D *pExclusiveScissors) {
    bool skip = false;
    layer_data *dev_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    unique_lock_t lock(global_lock);
    skip |=
        PreCallValidateCmdSetExclusiveScissorNV(commandBuffer, firstExclusiveScissor, exclusiveScissorCount, pExclusiveScissors);
    if (!skip) {
        PreCallRecordCmdSetExclusiveScissorNV(commandBuffer, firstExclusiveScissor, exclusiveScissorCount, pExclusiveScissors);
    }
    lock.unlock();
    if (!skip)
        dev_data->dispatch_table.CmdSetExclusiveScissorNV(commandBuffer, firstExclusiveScissor, exclusiveScissorCount,
                                                          pExclusiveScissors);
}

VKAPI_ATTR void VKAPI_CALL CmdBindShadingRateImageNV(VkCommandBuffer commandBuffer, VkImageView imageView,
                                                     VkImageLayout imageLayout) {
    bool skip = false;
    layer_data *dev_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    unique_lock_t lock(global_lock);
    skip |= PreCallValidateCmdBindShadingRateImageNV(commandBuffer, imageView, imageLayout);
    if (!skip) {
        PreCallRecordCmdBindShadingRateImageNV(commandBuffer, imageView, imageLayout);
        lock.unlock();
        dev_data->dispatch_table.CmdBindShadingRateImageNV(commandBuffer, imageView, imageLayout);
    }
}

VKAPI_ATTR void VKAPI_CALL CmdSetViewportShadingRatePaletteNV(VkCommandBuffer commandBuffer, uint32_t firstViewport,
                                                              uint32_t viewportCount,
                                                              const VkShadingRatePaletteNV *pShadingRatePalettes) {
    bool skip = false;
    layer_data *dev_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    unique_lock_t lock(global_lock);
    skip |= PreCallValidateCmdSetViewportShadingRatePaletteNV(commandBuffer, firstViewport, viewportCount, pShadingRatePalettes);
    if (!skip) {
        PreCallRecordCmdSetViewportShadingRatePaletteNV(commandBuffer, firstViewport, viewportCount, pShadingRatePalettes);
    }
    lock.unlock();
    if (!skip)
        dev_data->dispatch_table.CmdSetViewportShadingRatePaletteNV(commandBuffer, firstViewport, viewportCount,
                                                                    pShadingRatePalettes);
}

VKAPI_ATTR void VKAPI_CALL CmdSetLineWidth(VkCommandBuffer commandBuffer, float lineWidth) {
    bool skip = false;
    layer_data *dev_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    unique_lock_t lock(global_lock);
    skip |= PreCallValidateCmdSetLineWidth(commandBuffer, lineWidth);
    if (!skip) {
        PreCallRecordCmdSetLineWidth(commandBuffer, lineWidth);
    }
    lock.unlock();
    if (!skip) dev_data->dispatch_table.CmdSetLineWidth(commandBuffer, lineWidth);
}

VKAPI_ATTR void VKAPI_CALL CmdSetDepthBias(VkCommandBuffer commandBuffer, float depthBiasConstantFactor, float depthBiasClamp,
                                           float depthBiasSlopeFactor) {
    bool skip = false;
    layer_data *dev_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    unique_lock_t lock(global_lock);
    skip |= PreCallValidateCmdSetDepthBias(commandBuffer, depthBiasConstantFactor, depthBiasClamp, depthBiasSlopeFactor);
    if (!skip) {
        PreCallRecordCmdSetDepthBias(commandBuffer, depthBiasConstantFactor, depthBiasClamp, depthBiasSlopeFactor);
    }
    lock.unlock();
    if (!skip) {
        dev_data->dispatch_table.CmdSetDepthBias(commandBuffer, depthBiasConstantFactor, depthBiasClamp, depthBiasSlopeFactor);
    }
}

VKAPI_ATTR void VKAPI_CALL CmdSetBlendConstants(VkCommandBuffer commandBuffer, const float blendConstants[4]) {
    bool skip = false;
    layer_data *dev_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    unique_lock_t lock(global_lock);
    skip |= PreCallValidateCmdSetBlendConstants(commandBuffer, blendConstants);
    if (!skip) {
        PreCallRecordCmdSetBlendConstants(commandBuffer, blendConstants);
    }
    lock.unlock();
    if (!skip) dev_data->dispatch_table.CmdSetBlendConstants(commandBuffer, blendConstants);
}

VKAPI_ATTR void VKAPI_CALL CmdSetDepthBounds(VkCommandBuffer commandBuffer, float minDepthBounds, float maxDepthBounds) {
    bool skip = false;
    layer_data *dev_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    unique_lock_t lock(global_lock);
    skip |= PreCallValidateCmdSetDepthBounds(commandBuffer, minDepthBounds, maxDepthBounds);
    if (!skip) {
        PreCallRecordCmdSetDepthBounds(commandBuffer, minDepthBounds, maxDepthBounds);
    }
    lock.unlock();
    if (!skip) dev_data->dispatch_table.CmdSetDepthBounds(commandBuffer, minDepthBounds, maxDepthBounds);
}

VKAPI_ATTR void VKAPI_CALL CmdSetStencilCompareMask(VkCommandBuffer commandBuffer, VkStencilFaceFlags faceMask,
                                                    uint32_t compareMask) {
    bool skip = false;
    layer_data *dev_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    unique_lock_t lock(global_lock);
    skip |= PreCallValidateCmdSetStencilCompareMask(commandBuffer, faceMask, compareMask);
    if (!skip) {
        PreCallRecordCmdSetStencilCompareMask(commandBuffer, faceMask, compareMask);
    }
    lock.unlock();
    if (!skip) dev_data->dispatch_table.CmdSetStencilCompareMask(commandBuffer, faceMask, compareMask);
}

VKAPI_ATTR void VKAPI_CALL CmdSetStencilWriteMask(VkCommandBuffer commandBuffer, VkStencilFaceFlags faceMask, uint32_t writeMask) {
    bool skip = false;
    layer_data *dev_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    unique_lock_t lock(global_lock);
    skip |= PreCallValidateCmdSetStencilWriteMask(commandBuffer, faceMask, writeMask);
    if (!skip) {
        PreCallRecordCmdSetStencilWriteMask(commandBuffer, faceMask, writeMask);
    }
    lock.unlock();
    if (!skip) dev_data->dispatch_table.CmdSetStencilWriteMask(commandBuffer, faceMask, writeMask);
}

VKAPI_ATTR void VKAPI_CALL CmdSetStencilReference(VkCommandBuffer commandBuffer, VkStencilFaceFlags faceMask, uint32_t reference) {
    bool skip = false;
    layer_data *dev_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    unique_lock_t lock(global_lock);
    skip |= PreCallValidateCmdSetStencilReference(commandBuffer, faceMask, reference);
    if (!skip) {
        PreCallRecordCmdSetStencilReference(commandBuffer, faceMask, reference);
    }
    lock.unlock();
    if (!skip) dev_data->dispatch_table.CmdSetStencilReference(commandBuffer, faceMask, reference);
}

VKAPI_ATTR void VKAPI_CALL CmdBindDescriptorSets(VkCommandBuffer commandBuffer, VkPipelineBindPoint pipelineBindPoint,
                                                 VkPipelineLayout layout, uint32_t firstSet, uint32_t setCount,
                                                 const VkDescriptorSet *pDescriptorSets, uint32_t dynamicOffsetCount,
                                                 const uint32_t *pDynamicOffsets) {
    bool skip = false;
    layer_data *device_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    unique_lock_t lock(global_lock);
    skip = PreCallValidateCmdBindDescriptorSets(commandBuffer, pipelineBindPoint, layout, firstSet, setCount, pDescriptorSets,
                                                dynamicOffsetCount, pDynamicOffsets);
    if (!skip) {
        PreCallRecordCmdBindDescriptorSets(commandBuffer, pipelineBindPoint, layout, firstSet, setCount, pDescriptorSets,
                                           dynamicOffsetCount, pDynamicOffsets);
        lock.unlock();
        device_data->dispatch_table.CmdBindDescriptorSets(commandBuffer, pipelineBindPoint, layout, firstSet, setCount,
                                                          pDescriptorSets, dynamicOffsetCount, pDynamicOffsets);
    }
}

VKAPI_ATTR void VKAPI_CALL CmdPushDescriptorSetKHR(VkCommandBuffer commandBuffer, VkPipelineBindPoint pipelineBindPoint,
                                                   VkPipelineLayout layout, uint32_t set, uint32_t descriptorWriteCount,
                                                   const VkWriteDescriptorSet *pDescriptorWrites) {
    layer_data *device_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    unique_lock_t lock(global_lock);
    bool skip = false;
    skip = PreCallValidateCmdPushDescriptorSetKHR(commandBuffer, pipelineBindPoint, layout, set, descriptorWriteCount,
                                                  pDescriptorWrites);
    if (!skip) {
        PreCallRecordCmdPushDescriptorSetKHR(commandBuffer, pipelineBindPoint, layout, set, descriptorWriteCount,
                                             pDescriptorWrites);
        lock.unlock();
        device_data->dispatch_table.CmdPushDescriptorSetKHR(commandBuffer, pipelineBindPoint, layout, set, descriptorWriteCount,
                                                            pDescriptorWrites);
    }
}

VKAPI_ATTR void VKAPI_CALL CmdBindIndexBuffer(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                              VkIndexType indexType) {
    bool skip = false;
    layer_data *dev_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    unique_lock_t lock(global_lock);

    skip = PreCallValidateCmdBindIndexBuffer(commandBuffer, buffer, offset, indexType);
    if (skip) return;
    PreCallRecordCmdBindIndexBuffer(commandBuffer, buffer, offset, indexType);
    lock.unlock();
    dev_data->dispatch_table.CmdBindIndexBuffer(commandBuffer, buffer, offset, indexType);
}

VKAPI_ATTR void VKAPI_CALL CmdBindVertexBuffers(VkCommandBuffer commandBuffer, uint32_t firstBinding, uint32_t bindingCount,
                                                const VkBuffer *pBuffers, const VkDeviceSize *pOffsets) {
    bool skip = false;
    layer_data *dev_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    unique_lock_t lock(global_lock);

    skip |= PreCallValidateCmdBindVertexBuffers(commandBuffer, firstBinding, bindingCount, pBuffers, pOffsets);
    if (skip) return;
    PreCallRecordCmdBindVertexBuffers(commandBuffer, firstBinding, bindingCount, pBuffers, pOffsets);
    lock.unlock();
    dev_data->dispatch_table.CmdBindVertexBuffers(commandBuffer, firstBinding, bindingCount, pBuffers, pOffsets);
}

VKAPI_ATTR void VKAPI_CALL CmdDraw(VkCommandBuffer commandBuffer, uint32_t vertexCount, uint32_t instanceCount,
                                   uint32_t firstVertex, uint32_t firstInstance) {
    layer_data *dev_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    unique_lock_t lock(global_lock);
    bool skip = PreCallValidateCmdDraw(commandBuffer, vertexCount, instanceCount, firstVertex, firstInstance);
    if (!skip) {
        PreCallRecordCmdDraw(commandBuffer, vertexCount, instanceCount, firstVertex, firstInstance);
        lock.unlock();
        dev_data->dispatch_table.CmdDraw(commandBuffer, vertexCount, instanceCount, firstVertex, firstInstance);
        lock.lock();
        PostCallRecordCmdDraw(commandBuffer, vertexCount, instanceCount, firstVertex, firstInstance);
        lock.unlock();
    }
}

VKAPI_ATTR void VKAPI_CALL CmdDrawIndexed(VkCommandBuffer commandBuffer, uint32_t indexCount, uint32_t instanceCount,
                                          uint32_t firstIndex, int32_t vertexOffset, uint32_t firstInstance) {
    layer_data *dev_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    unique_lock_t lock(global_lock);
    bool skip = PreCallValidateCmdDrawIndexed(commandBuffer, indexCount, instanceCount, firstIndex, vertexOffset, firstInstance);
    if (!skip) {
        PreCallRecordCmdDrawIndexed(commandBuffer, indexCount, instanceCount, firstIndex, vertexOffset, firstInstance);
        lock.unlock();
        dev_data->dispatch_table.CmdDrawIndexed(commandBuffer, indexCount, instanceCount, firstIndex, vertexOffset, firstInstance);
        lock.lock();
        PostCallRecordCmdDrawIndexed(commandBuffer, indexCount, instanceCount, firstIndex, vertexOffset, firstInstance);
        lock.unlock();
    }
}

VKAPI_ATTR void VKAPI_CALL CmdDrawIndexedIndirect(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                  uint32_t count, uint32_t stride) {
    layer_data *dev_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    unique_lock_t lock(global_lock);
    bool skip = PreCallValidateCmdDrawIndexedIndirect(commandBuffer, buffer, offset, count, stride);
    if (!skip) {
        PreCallRecordCmdDrawIndexedIndirect(commandBuffer, buffer, offset, count, stride);
        lock.unlock();
        dev_data->dispatch_table.CmdDrawIndexedIndirect(commandBuffer, buffer, offset, count, stride);
        lock.lock();
        PostCallRecordCmdDrawIndexedIndirect(commandBuffer, buffer, offset, count, stride);
        lock.unlock();
    }
}

VKAPI_ATTR void VKAPI_CALL CmdDispatch(VkCommandBuffer commandBuffer, uint32_t x, uint32_t y, uint32_t z) {
    layer_data *dev_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    unique_lock_t lock(global_lock);
    bool skip = PreCallValidateCmdDispatch(commandBuffer, x, y, z);
    if (!skip) {
        PreCallRecordCmdDispatch(commandBuffer, x, y, z);
        lock.unlock();
        dev_data->dispatch_table.CmdDispatch(commandBuffer, x, y, z);
        lock.lock();
        PostCallRecordCmdDispatch(commandBuffer, x, y, z);
        lock.unlock();
    }
}

VKAPI_ATTR void VKAPI_CALL CmdDispatchIndirect(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset) {
    layer_data *dev_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    unique_lock_t lock(global_lock);
    bool skip = PreCallValidateCmdDispatchIndirect(commandBuffer, buffer, offset);
    if (!skip) {
        PreCallValidateCmdDispatchIndirect(commandBuffer, buffer, offset);
        lock.unlock();
        dev_data->dispatch_table.CmdDispatchIndirect(commandBuffer, buffer, offset);
        lock.lock();
        PostCallRecordCmdDispatchIndirect(commandBuffer, buffer, offset);
        lock.unlock();
    }
}

VKAPI_ATTR void VKAPI_CALL CmdCopyBuffer(VkCommandBuffer commandBuffer, VkBuffer srcBuffer, VkBuffer dstBuffer,
                                         uint32_t regionCount, const VkBufferCopy *pRegions) {
    layer_data *device_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    unique_lock_t lock(global_lock);

    bool skip = PreCallValidateCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, regionCount, pRegions);
    if (!skip) {
        PreCallRecordCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, regionCount, pRegions);
        lock.unlock();
        device_data->dispatch_table.CmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, regionCount, pRegions);
    }
}

VKAPI_ATTR void VKAPI_CALL CmdCopyImage(VkCommandBuffer commandBuffer, VkImage srcImage, VkImageLayout srcImageLayout,
                                        VkImage dstImage, VkImageLayout dstImageLayout, uint32_t regionCount,
                                        const VkImageCopy *pRegions) {
    bool skip = false;
    layer_data *device_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    unique_lock_t lock(global_lock);
    skip = PreCallValidateCmdCopyImage(commandBuffer, srcImage, srcImageLayout, dstImage, dstImageLayout, regionCount, pRegions);
    if (!skip) {
        PreCallRecordCmdCopyImage(commandBuffer, srcImage, srcImageLayout, dstImage, dstImageLayout, regionCount, pRegions);
        lock.unlock();
        device_data->dispatch_table.CmdCopyImage(commandBuffer, srcImage, srcImageLayout, dstImage, dstImageLayout, regionCount,
                                                 pRegions);
    }
}

VKAPI_ATTR void VKAPI_CALL CmdDrawIndirect(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset, uint32_t count,
                                           uint32_t stride) {
    layer_data *dev_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    unique_lock_t lock(global_lock);
    bool skip = PreCallValidateCmdDrawIndirect(commandBuffer, buffer, offset, count, stride);
    if (!skip) {
        PreCallRecordCmdDrawIndirect(commandBuffer, buffer, offset, count, stride);
        lock.unlock();
        dev_data->dispatch_table.CmdDrawIndirect(commandBuffer, buffer, offset, count, stride);
        lock.lock();
        PostCallRecordCmdDrawIndirect(commandBuffer, buffer, offset, count, stride);
        lock.unlock();
    }
}

VKAPI_ATTR void VKAPI_CALL CmdBlitImage(VkCommandBuffer commandBuffer, VkImage srcImage, VkImageLayout srcImageLayout,
                                        VkImage dstImage, VkImageLayout dstImageLayout, uint32_t regionCount,
                                        const VkImageBlit *pRegions, VkFilter filter) {
    layer_data *dev_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    unique_lock_t lock(global_lock);

    bool skip = PreCallValidateCmdBlitImage(commandBuffer, srcImage, srcImageLayout, dstImage, dstImageLayout, regionCount,
                                            pRegions, filter);

    if (!skip) {
        PreCallRecordCmdBlitImage(commandBuffer, srcImage, srcImageLayout, dstImage, dstImageLayout, regionCount, pRegions, filter);
        lock.unlock();
        dev_data->dispatch_table.CmdBlitImage(commandBuffer, srcImage, srcImageLayout, dstImage, dstImageLayout, regionCount,
                                              pRegions, filter);
    }
}

VKAPI_ATTR void VKAPI_CALL CmdCopyBufferToImage(VkCommandBuffer commandBuffer, VkBuffer srcBuffer, VkImage dstImage,
                                                VkImageLayout dstImageLayout, uint32_t regionCount,
                                                const VkBufferImageCopy *pRegions) {
    layer_data *device_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    unique_lock_t lock(global_lock);
    bool skip = false;
    skip = PreCallValidateCmdCopyBufferToImage(commandBuffer, srcBuffer, dstImage, dstImageLayout, regionCount, pRegions);

    if (!skip) {
        PreCallRecordCmdCopyBufferToImage(commandBuffer, srcBuffer, dstImage, dstImageLayout, regionCount, pRegions);
        lock.unlock();
        device_data->dispatch_table.CmdCopyBufferToImage(commandBuffer, srcBuffer, dstImage, dstImageLayout, regionCount, pRegions);
    }
}

VKAPI_ATTR void VKAPI_CALL CmdCopyImageToBuffer(VkCommandBuffer commandBuffer, VkImage srcImage, VkImageLayout srcImageLayout,
                                                VkBuffer dstBuffer, uint32_t regionCount, const VkBufferImageCopy *pRegions) {
    bool skip = false;
    layer_data *device_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    unique_lock_t lock(global_lock);

    skip = PreCallValidateCmdCopyImageToBuffer(commandBuffer, srcImage, srcImageLayout, dstBuffer, regionCount, pRegions);
    if (!skip) {
        PreCallRecordCmdCopyImageToBuffer(commandBuffer, srcImage, srcImageLayout, dstBuffer, regionCount, pRegions);
        lock.unlock();
        device_data->dispatch_table.CmdCopyImageToBuffer(commandBuffer, srcImage, srcImageLayout, dstBuffer, regionCount, pRegions);
    }
}

VKAPI_ATTR void VKAPI_CALL CmdFillBuffer(VkCommandBuffer commandBuffer, VkBuffer dstBuffer, VkDeviceSize dstOffset,
                                         VkDeviceSize size, uint32_t data) {
    layer_data *device_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    unique_lock_t lock(global_lock);

    bool skip = PreCallValidateCmdFillBuffer(commandBuffer, dstBuffer, dstOffset, size, data);
    if (!skip) {
        PreCallRecordCmdFillBuffer(commandBuffer, dstBuffer, dstOffset, size, data);
        lock.unlock();
        device_data->dispatch_table.CmdFillBuffer(commandBuffer, dstBuffer, dstOffset, size, data);
    }
}

VKAPI_ATTR void VKAPI_CALL CmdClearAttachments(VkCommandBuffer commandBuffer, uint32_t attachmentCount,
                                               const VkClearAttachment *pAttachments, uint32_t rectCount,
                                               const VkClearRect *pRects) {
    bool skip = false;
    layer_data *dev_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    {
        lock_guard_t lock(global_lock);
        skip = PreCallValidateCmdClearAttachments(commandBuffer, attachmentCount, pAttachments, rectCount, pRects);
    }
    if (!skip) dev_data->dispatch_table.CmdClearAttachments(commandBuffer, attachmentCount, pAttachments, rectCount, pRects);
}

VKAPI_ATTR void VKAPI_CALL CmdClearColorImage(VkCommandBuffer commandBuffer, VkImage image, VkImageLayout imageLayout,
                                              const VkClearColorValue *pColor, uint32_t rangeCount,
                                              const VkImageSubresourceRange *pRanges) {
    layer_data *dev_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    unique_lock_t lock(global_lock);

    bool skip = PreCallValidateCmdClearColorImage(commandBuffer, image, imageLayout, pColor, rangeCount, pRanges);
    if (!skip) {
        PreCallRecordCmdClearColorImage(commandBuffer, image, imageLayout, pColor, rangeCount, pRanges);
        lock.unlock();
        dev_data->dispatch_table.CmdClearColorImage(commandBuffer, image, imageLayout, pColor, rangeCount, pRanges);
    }
}

VKAPI_ATTR void VKAPI_CALL CmdClearDepthStencilImage(VkCommandBuffer commandBuffer, VkImage image, VkImageLayout imageLayout,
                                                     const VkClearDepthStencilValue *pDepthStencil, uint32_t rangeCount,
                                                     const VkImageSubresourceRange *pRanges) {
    layer_data *dev_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    unique_lock_t lock(global_lock);

    bool skip = PreCallValidateCmdClearDepthStencilImage(commandBuffer, image, imageLayout, pDepthStencil, rangeCount, pRanges);
    if (!skip) {
        PreCallRecordCmdClearDepthStencilImage(commandBuffer, image, imageLayout, pDepthStencil, rangeCount, pRanges);
        lock.unlock();
        dev_data->dispatch_table.CmdClearDepthStencilImage(commandBuffer, image, imageLayout, pDepthStencil, rangeCount, pRanges);
    }
}

VKAPI_ATTR void VKAPI_CALL CmdResolveImage(VkCommandBuffer commandBuffer, VkImage srcImage, VkImageLayout srcImageLayout,
                                           VkImage dstImage, VkImageLayout dstImageLayout, uint32_t regionCount,
                                           const VkImageResolve *pRegions) {
    layer_data *dev_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    unique_lock_t lock(global_lock);

    bool skip =
        PreCallValidateCmdResolveImage(commandBuffer, srcImage, srcImageLayout, dstImage, dstImageLayout, regionCount, pRegions);

    if (!skip) {
        PreCallRecordCmdResolveImage(commandBuffer, srcImage, srcImageLayout, dstImage, dstImageLayout, regionCount, pRegions);
        lock.unlock();
        dev_data->dispatch_table.CmdResolveImage(commandBuffer, srcImage, srcImageLayout, dstImage, dstImageLayout, regionCount,
                                                 pRegions);
    }
}

VKAPI_ATTR void VKAPI_CALL GetImageSubresourceLayout(VkDevice device, VkImage image, const VkImageSubresource *pSubresource,
                                                     VkSubresourceLayout *pLayout) {
    layer_data *device_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    unique_lock_t lock(global_lock);

    bool skip = PreCallValidateGetImageSubresourceLayout(device, image, pSubresource, pLayout);
    if (!skip) {
        lock.unlock();
        device_data->dispatch_table.GetImageSubresourceLayout(device, image, pSubresource, pLayout);
    }
}

VKAPI_ATTR void VKAPI_CALL CmdSetEvent(VkCommandBuffer commandBuffer, VkEvent event, VkPipelineStageFlags stageMask) {
    bool skip = false;
    layer_data *dev_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    unique_lock_t lock(global_lock);
    skip |= PreCallValidateCmdSetEvent(commandBuffer, event, stageMask);
    if (!skip) {
        PreCallRecordCmdSetEvent(commandBuffer, event, stageMask);
        lock.unlock();
        dev_data->dispatch_table.CmdSetEvent(commandBuffer, event, stageMask);
    }
}

VKAPI_ATTR void VKAPI_CALL CmdResetEvent(VkCommandBuffer commandBuffer, VkEvent event, VkPipelineStageFlags stageMask) {
    bool skip = false;
    layer_data *dev_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    unique_lock_t lock(global_lock);
    skip |= PreCallValidateCmdResetEvent(commandBuffer, event, stageMask);
    if (!skip) {
        PreCallRecordCmdResetEvent(commandBuffer, event, stageMask);
        lock.unlock();
        dev_data->dispatch_table.CmdResetEvent(commandBuffer, event, stageMask);
    }
}

VKAPI_ATTR void VKAPI_CALL CmdWaitEvents(VkCommandBuffer commandBuffer, uint32_t eventCount, const VkEvent *pEvents,
                                         VkPipelineStageFlags sourceStageMask, VkPipelineStageFlags dstStageMask,
                                         uint32_t memoryBarrierCount, const VkMemoryBarrier *pMemoryBarriers,
                                         uint32_t bufferMemoryBarrierCount, const VkBufferMemoryBarrier *pBufferMemoryBarriers,
                                         uint32_t imageMemoryBarrierCount, const VkImageMemoryBarrier *pImageMemoryBarriers) {
    bool skip = false;
    layer_data *dev_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    unique_lock_t lock(global_lock);
    skip |= PreCallValidateCmdWaitEvents(commandBuffer, eventCount, pEvents, sourceStageMask, dstStageMask, memoryBarrierCount,
                                         pMemoryBarriers, bufferMemoryBarrierCount, pBufferMemoryBarriers, imageMemoryBarrierCount,
                                         pImageMemoryBarriers);
    if (!skip) {
        PreCallRecordCmdWaitEvents(commandBuffer, eventCount, pEvents, sourceStageMask, dstStageMask, memoryBarrierCount,
                                   pMemoryBarriers, bufferMemoryBarrierCount, pBufferMemoryBarriers, imageMemoryBarrierCount,
                                   pImageMemoryBarriers);
        lock.unlock();

        dev_data->dispatch_table.CmdWaitEvents(commandBuffer, eventCount, pEvents, sourceStageMask, dstStageMask,
                                               memoryBarrierCount, pMemoryBarriers, bufferMemoryBarrierCount, pBufferMemoryBarriers,
                                               imageMemoryBarrierCount, pImageMemoryBarriers);

        lock.lock();
        PostCallRecordCmdWaitEvents(commandBuffer, eventCount, pEvents, sourceStageMask, dstStageMask, memoryBarrierCount,
                                    pMemoryBarriers, bufferMemoryBarrierCount, pBufferMemoryBarriers, imageMemoryBarrierCount,
                                    pImageMemoryBarriers);
    }
}

VKAPI_ATTR void VKAPI_CALL CmdPipelineBarrier(VkCommandBuffer commandBuffer, VkPipelineStageFlags srcStageMask,
                                              VkPipelineStageFlags dstStageMask, VkDependencyFlags dependencyFlags,
                                              uint32_t memoryBarrierCount, const VkMemoryBarrier *pMemoryBarriers,
                                              uint32_t bufferMemoryBarrierCount, const VkBufferMemoryBarrier *pBufferMemoryBarriers,
                                              uint32_t imageMemoryBarrierCount, const VkImageMemoryBarrier *pImageMemoryBarriers) {
    bool skip = false;
    layer_data *device_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    unique_lock_t lock(global_lock);
    skip |= PreCallValidateCmdPipelineBarrier(commandBuffer, srcStageMask, dstStageMask, dependencyFlags, memoryBarrierCount,
                                              pMemoryBarriers, bufferMemoryBarrierCount, pBufferMemoryBarriers,
                                              imageMemoryBarrierCount, pImageMemoryBarriers);
    if (!skip) {
        PreCallRecordCmdPipelineBarrier(commandBuffer, srcStageMask, dstStageMask, dependencyFlags, memoryBarrierCount,
                                        pMemoryBarriers, bufferMemoryBarrierCount, pBufferMemoryBarriers, imageMemoryBarrierCount,
                                        pImageMemoryBarriers);
        lock.unlock();
        device_data->dispatch_table.CmdPipelineBarrier(commandBuffer, srcStageMask, dstStageMask, dependencyFlags,
                                                       memoryBarrierCount, pMemoryBarriers, bufferMemoryBarrierCount,
                                                       pBufferMemoryBarriers, imageMemoryBarrierCount, pImageMemoryBarriers);
    }
}

VKAPI_ATTR void VKAPI_CALL CmdBeginQuery(VkCommandBuffer commandBuffer, VkQueryPool queryPool, uint32_t slot, VkFlags flags) {
    bool skip = false;
    layer_data *dev_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    unique_lock_t lock(global_lock);
    PreCallValidateCmdBeginQuery(commandBuffer, queryPool, slot, flags);
    lock.unlock();
    if (skip) return;

    dev_data->dispatch_table.CmdBeginQuery(commandBuffer, queryPool, slot, flags);

    lock.lock();
    PostCallRecordCmdBeginQuery(commandBuffer, queryPool, slot, flags);
}

VKAPI_ATTR void VKAPI_CALL CmdEndQuery(VkCommandBuffer commandBuffer, VkQueryPool queryPool, uint32_t slot) {
    bool skip = false;
    layer_data *dev_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    unique_lock_t lock(global_lock);
    skip |= PreCallValidateCmdEndQuery(commandBuffer, queryPool, slot);
    lock.unlock();
    if (skip) return;

    dev_data->dispatch_table.CmdEndQuery(commandBuffer, queryPool, slot);
    lock.lock();
    PostCallRecordCmdEndQuery(commandBuffer, queryPool, slot);
}

VKAPI_ATTR void VKAPI_CALL CmdResetQueryPool(VkCommandBuffer commandBuffer, VkQueryPool queryPool, uint32_t firstQuery,
                                             uint32_t queryCount) {
    layer_data *dev_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    unique_lock_t lock(global_lock);
    bool skip = PreCallValidateCmdResetQueryPool(commandBuffer, queryPool, firstQuery, queryCount);
    lock.unlock();
    if (skip) return;

    dev_data->dispatch_table.CmdResetQueryPool(commandBuffer, queryPool, firstQuery, queryCount);
    lock.lock();
    PostCallRecordCmdResetQueryPool(commandBuffer, queryPool, firstQuery, queryCount);
}

VKAPI_ATTR void VKAPI_CALL CmdCopyQueryPoolResults(VkCommandBuffer commandBuffer, VkQueryPool queryPool, uint32_t firstQuery,
                                                   uint32_t queryCount, VkBuffer dstBuffer, VkDeviceSize dstOffset,
                                                   VkDeviceSize stride, VkQueryResultFlags flags) {
    bool skip = false;
    layer_data *dev_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    unique_lock_t lock(global_lock);

    skip |= PreCallValidateCmdCopyQueryPoolResults(commandBuffer, queryPool, firstQuery, queryCount, dstBuffer, dstOffset, stride,
                                                   flags);
    lock.unlock();
    if (skip) return;

    dev_data->dispatch_table.CmdCopyQueryPoolResults(commandBuffer, queryPool, firstQuery, queryCount, dstBuffer, dstOffset, stride,
                                                     flags);
    lock.lock();
    PostCallRecordCmdCopyQueryPoolResults(commandBuffer, queryPool, firstQuery, queryCount, dstBuffer, dstOffset, stride, flags);
}

VKAPI_ATTR void VKAPI_CALL CmdPushConstants(VkCommandBuffer commandBuffer, VkPipelineLayout layout, VkShaderStageFlags stageFlags,
                                            uint32_t offset, uint32_t size, const void *pValues) {
    bool skip = false;
    layer_data *dev_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    unique_lock_t lock(global_lock);
    skip |= PreCallValidateCmdPushConstants(commandBuffer, layout, stageFlags, offset, size, pValues);
    lock.unlock();
    if (!skip) dev_data->dispatch_table.CmdPushConstants(commandBuffer, layout, stageFlags, offset, size, pValues);
}

VKAPI_ATTR void VKAPI_CALL CmdWriteTimestamp(VkCommandBuffer commandBuffer, VkPipelineStageFlagBits pipelineStage,
                                             VkQueryPool queryPool, uint32_t slot) {
    bool skip = false;
    layer_data *dev_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    unique_lock_t lock(global_lock);
    skip |= PreCallValidateCmdWriteTimestamp(commandBuffer, pipelineStage, queryPool, slot);
    lock.unlock();
    if (skip) return;
    dev_data->dispatch_table.CmdWriteTimestamp(commandBuffer, pipelineStage, queryPool, slot);
    lock.lock();
    PostCallRecordCmdWriteTimestamp(commandBuffer, pipelineStage, queryPool, slot);
}

VKAPI_ATTR VkResult VKAPI_CALL CreateFramebuffer(VkDevice device, const VkFramebufferCreateInfo *pCreateInfo,
                                                 const VkAllocationCallbacks *pAllocator, VkFramebuffer *pFramebuffer) {
    layer_data *dev_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    unique_lock_t lock(global_lock);
    bool skip = PreCallValidateCreateFramebuffer(device, pCreateInfo, pAllocator, pFramebuffer);
    lock.unlock();
    if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;

    VkResult result = dev_data->dispatch_table.CreateFramebuffer(device, pCreateInfo, pAllocator, pFramebuffer);
    lock.lock();
    PostCallRecordCreateFramebuffer(device, pCreateInfo, pAllocator, pFramebuffer, result);
    return result;
}

VKAPI_ATTR VkResult VKAPI_CALL CreateRenderPass(VkDevice device, const VkRenderPassCreateInfo *pCreateInfo,
                                                const VkAllocationCallbacks *pAllocator, VkRenderPass *pRenderPass) {
    bool skip = false;

    layer_data *dev_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);

    unique_lock_t lock(global_lock);

    skip = PreCallValidateCreateRenderPass(device, pCreateInfo, pAllocator, pRenderPass);
    lock.unlock();

    if (skip) {
        return VK_ERROR_VALIDATION_FAILED_EXT;
    }

    VkResult result = dev_data->dispatch_table.CreateRenderPass(device, pCreateInfo, pAllocator, pRenderPass);

    if (VK_SUCCESS == result) {
        lock.lock();
        PostCallRecordCreateRenderPass(device, pCreateInfo, pAllocator, pRenderPass, result);
    }
    return result;
}

VKAPI_ATTR VkResult VKAPI_CALL CreateRenderPass2KHR(VkDevice device, const VkRenderPassCreateInfo2KHR *pCreateInfo,
                                                    const VkAllocationCallbacks *pAllocator, VkRenderPass *pRenderPass) {
    bool skip = false;
    layer_data *dev_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);

    unique_lock_t lock(global_lock);
    skip = PreCallValidateCreateRenderPass2KHR(device, pCreateInfo, pAllocator, pRenderPass);
    lock.unlock();

    if (skip) {
        return VK_ERROR_VALIDATION_FAILED_EXT;
    }
    VkResult result = dev_data->dispatch_table.CreateRenderPass2KHR(device, pCreateInfo, pAllocator, pRenderPass);
    lock.lock();
    PostCallRecordCreateRenderPass2KHR(device, pCreateInfo, pAllocator, pRenderPass, result);
    return result;
}

VKAPI_ATTR void VKAPI_CALL CmdBeginRenderPass(VkCommandBuffer commandBuffer, const VkRenderPassBeginInfo *pRenderPassBegin,
                                              VkSubpassContents contents) {
    bool skip = false;
    layer_data *dev_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    unique_lock_t lock(global_lock);
    skip |= PreCallValidateCmdBeginRenderPass(commandBuffer, pRenderPassBegin, contents);
    if (!skip) {
        PreCallRecordCmdBeginRenderPass(commandBuffer, pRenderPassBegin, contents);
        lock.unlock();
        dev_data->dispatch_table.CmdBeginRenderPass(commandBuffer, pRenderPassBegin, contents);
    }
}

VKAPI_ATTR void VKAPI_CALL CmdBeginRenderPass2KHR(VkCommandBuffer commandBuffer, const VkRenderPassBeginInfo *pRenderPassBegin,
                                                  const VkSubpassBeginInfoKHR *pSubpassBeginInfo) {
    bool skip = false;
    layer_data *dev_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    unique_lock_t lock(global_lock);
    skip |= PreCallValidateCmdBeginRenderPass2KHR(commandBuffer, pRenderPassBegin, pSubpassBeginInfo);
    if (!skip) {
        PreCallRecordCmdBeginRenderPass2KHR(commandBuffer, pRenderPassBegin, pSubpassBeginInfo);
        lock.unlock();
        dev_data->dispatch_table.CmdBeginRenderPass2KHR(commandBuffer, pRenderPassBegin, pSubpassBeginInfo);
    }
}

VKAPI_ATTR void VKAPI_CALL CmdNextSubpass(VkCommandBuffer commandBuffer, VkSubpassContents contents) {
    bool skip = false;
    layer_data *dev_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    unique_lock_t lock(global_lock);
    skip |= PreCallValidateCmdNextSubpass(commandBuffer, contents);
    lock.unlock();
    if (skip) return;
    dev_data->dispatch_table.CmdNextSubpass(commandBuffer, contents);
    lock.lock();
    PostCallRecordCmdNextSubpass(commandBuffer, contents);
}

VKAPI_ATTR void VKAPI_CALL CmdNextSubpass2KHR(VkCommandBuffer commandBuffer, const VkSubpassBeginInfoKHR *pSubpassBeginInfo,
                                              const VkSubpassEndInfoKHR *pSubpassEndInfo) {
    bool skip = false;
    layer_data *dev_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    unique_lock_t lock(global_lock);
    skip |= PreCallValidateCmdNextSubpass2KHR(commandBuffer, pSubpassBeginInfo, pSubpassEndInfo);
    lock.unlock();
    if (skip) return;
    dev_data->dispatch_table.CmdNextSubpass2KHR(commandBuffer, pSubpassBeginInfo, pSubpassEndInfo);
    PostCallRecordCmdNextSubpass2KHR(commandBuffer, pSubpassBeginInfo, pSubpassEndInfo);
}

VKAPI_ATTR void VKAPI_CALL CmdEndRenderPass(VkCommandBuffer commandBuffer) {
    bool skip = false;
    layer_data *dev_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    unique_lock_t lock(global_lock);
    skip |= PreCallValidateCmdEndRenderPass(commandBuffer);
    lock.unlock();
    if (skip) return;
    dev_data->dispatch_table.CmdEndRenderPass(commandBuffer);
    lock.lock();
    PostCallRecordCmdEndRenderPass(commandBuffer);
}

VKAPI_ATTR void VKAPI_CALL CmdEndRenderPass2KHR(VkCommandBuffer commandBuffer, const VkSubpassEndInfoKHR *pSubpassEndInfo) {
    bool skip = false;
    layer_data *dev_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    unique_lock_t lock(global_lock);
    skip |= PreCallValidateCmdEndRenderPass2KHR(commandBuffer, pSubpassEndInfo);
    lock.unlock();
    if (skip) return;
    dev_data->dispatch_table.CmdEndRenderPass2KHR(commandBuffer, pSubpassEndInfo);

    lock.lock();
    PostCallRecordCmdEndRenderPass2KHR(commandBuffer, pSubpassEndInfo);
}

VKAPI_ATTR void VKAPI_CALL CmdExecuteCommands(VkCommandBuffer commandBuffer, uint32_t commandBuffersCount,
                                              const VkCommandBuffer *pCommandBuffers) {
    bool skip = false;
    layer_data *dev_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    unique_lock_t lock(global_lock);
    skip |= PreCallValidateCmdExecuteCommands(commandBuffer, commandBuffersCount, pCommandBuffers);
    if (skip) return;
    PreCallRecordCmdExecuteCommands(commandBuffer, commandBuffersCount, pCommandBuffers);
    lock.unlock();
    dev_data->dispatch_table.CmdExecuteCommands(commandBuffer, commandBuffersCount, pCommandBuffers);
}

VKAPI_ATTR VkResult VKAPI_CALL MapMemory(VkDevice device, VkDeviceMemory mem, VkDeviceSize offset, VkDeviceSize size, VkFlags flags,
                                         void **ppData) {
    layer_data *dev_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    unique_lock_t lock(global_lock);
    bool skip = PreCallValidateMapMemory(device, mem, offset, size, flags, ppData);
    lock.unlock();
    VkResult result = VK_ERROR_VALIDATION_FAILED_EXT;
    if (!skip) {
        result = dev_data->dispatch_table.MapMemory(device, mem, offset, size, flags, ppData);
        lock.lock();
        PostCallRecordMapMemory(device, mem, offset, size, flags, ppData, result);
        lock.unlock();
    }
    return result;
}

VKAPI_ATTR void VKAPI_CALL UnmapMemory(VkDevice device, VkDeviceMemory mem) {
    layer_data *dev_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    bool skip = false;
    unique_lock_t lock(global_lock);
    skip |= PreCallValidateUnmapMemory(device, mem);
    PreCallRecordUnmapMemory(device, mem);
    lock.unlock();
    if (!skip) {
        dev_data->dispatch_table.UnmapMemory(device, mem);
    }
}

VKAPI_ATTR VkResult VKAPI_CALL FlushMappedMemoryRanges(VkDevice device, uint32_t memRangeCount,
                                                       const VkMappedMemoryRange *pMemRanges) {
    VkResult result = VK_ERROR_VALIDATION_FAILED_EXT;
    layer_data *dev_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    unique_lock_t lock(global_lock);
    bool skip = PreCallValidateFlushMappedMemoryRanges(device, memRangeCount, pMemRanges);
    lock.unlock();
    if (!skip) {
        result = dev_data->dispatch_table.FlushMappedMemoryRanges(device, memRangeCount, pMemRanges);
    }
    return result;
}

VKAPI_ATTR VkResult VKAPI_CALL InvalidateMappedMemoryRanges(VkDevice device, uint32_t memRangeCount,
                                                            const VkMappedMemoryRange *pMemRanges) {
    VkResult result = VK_ERROR_VALIDATION_FAILED_EXT;
    layer_data *dev_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    unique_lock_t lock(global_lock);
    bool skip = PreCallValidateInvalidateMappedMemoryRanges(device, memRangeCount, pMemRanges);
    lock.unlock();
    if (!skip) {
        result = dev_data->dispatch_table.InvalidateMappedMemoryRanges(device, memRangeCount, pMemRanges);
        lock.lock();
        PostCallRecordInvalidateMappedMemoryRanges(device, memRangeCount, pMemRanges, result);
        lock.unlock();
    }
    return result;
}

VKAPI_ATTR VkResult VKAPI_CALL BindImageMemory(VkDevice device, VkImage image, VkDeviceMemory mem, VkDeviceSize memoryOffset) {
    layer_data *dev_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    VkResult result = VK_ERROR_VALIDATION_FAILED_EXT;
    unique_lock_t lock(global_lock);
    bool skip = PreCallValidateBindImageMemory(device, image, mem, memoryOffset);
    lock.unlock();
    if (!skip) {
        result = dev_data->dispatch_table.BindImageMemory(device, image, mem, memoryOffset);
        lock.lock();
        PostCallRecordBindImageMemory(device, image, mem, memoryOffset, result);
        lock.unlock();
    }
    return result;
}

VKAPI_ATTR VkResult VKAPI_CALL BindImageMemory2(VkDevice device, uint32_t bindInfoCount,
                                                const VkBindImageMemoryInfoKHR *pBindInfos) {
    layer_data *dev_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    VkResult result = VK_ERROR_VALIDATION_FAILED_EXT;
    unique_lock_t lock(global_lock);
    bool skip = PreCallValidateBindImageMemory2(device, bindInfoCount, pBindInfos);
    lock.unlock();
    if (!skip) {
        result = dev_data->dispatch_table.BindImageMemory2(device, bindInfoCount, pBindInfos);
        lock.lock();
        PostCallRecordBindImageMemory2(device, bindInfoCount, pBindInfos, result);
        lock.unlock();
    }
    return result;
}

VKAPI_ATTR VkResult VKAPI_CALL BindImageMemory2KHR(VkDevice device, uint32_t bindInfoCount,
                                                   const VkBindImageMemoryInfoKHR *pBindInfos) {
    layer_data *dev_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    VkResult result = VK_ERROR_VALIDATION_FAILED_EXT;
    unique_lock_t lock(global_lock);
    bool skip = PreCallValidateBindImageMemory2KHR(device, bindInfoCount, pBindInfos);
    lock.unlock();
    if (!skip) {
        result = dev_data->dispatch_table.BindImageMemory2KHR(device, bindInfoCount, pBindInfos);
        lock.lock();
        PostCallRecordBindImageMemory2KHR(device, bindInfoCount, pBindInfos, result);
        lock.unlock();
    }
    return result;
}

VKAPI_ATTR VkResult VKAPI_CALL SetEvent(VkDevice device, VkEvent event) {
    layer_data *dev_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    unique_lock_t lock(global_lock);
    bool skip = PreCallValidateSetEvent(device, event);
    if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
    PreCallRecordSetEvent(device, event);
    lock.unlock();
    return dev_data->dispatch_table.SetEvent(device, event);
}

VKAPI_ATTR VkResult VKAPI_CALL QueueBindSparse(VkQueue queue, uint32_t bindInfoCount, const VkBindSparseInfo *pBindInfo,
                                               VkFence fence) {
    layer_data *dev_data = GetLayerDataPtr(get_dispatch_key(queue), layer_data_map);
    unique_lock_t lock(global_lock);
    bool skip = PreCallValidateQueueBindSparse(queue, bindInfoCount, pBindInfo, fence);
    lock.unlock();

    if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;

    VkResult result = dev_data->dispatch_table.QueueBindSparse(queue, bindInfoCount, pBindInfo, fence);

    lock.lock();
    PostCallRecordQueueBindSparse(queue, bindInfoCount, pBindInfo, fence, result);
    lock.unlock();
    return result;
}

VKAPI_ATTR VkResult VKAPI_CALL CreateSemaphore(VkDevice device, const VkSemaphoreCreateInfo *pCreateInfo,
                                               const VkAllocationCallbacks *pAllocator, VkSemaphore *pSemaphore) {
    layer_data *dev_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    VkResult result = dev_data->dispatch_table.CreateSemaphore(device, pCreateInfo, pAllocator, pSemaphore);
    lock_guard_t lock(global_lock);
    PostCallRecordCreateSemaphore(device, pCreateInfo, pAllocator, pSemaphore, result);
    return result;
}

#ifdef VK_USE_PLATFORM_WIN32_KHR
VKAPI_ATTR VkResult VKAPI_CALL
ImportSemaphoreWin32HandleKHR(VkDevice device, const VkImportSemaphoreWin32HandleInfoKHR *pImportSemaphoreWin32HandleInfo) {
    VkResult result = VK_ERROR_VALIDATION_FAILED_EXT;
    layer_data *dev_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    unique_lock_t lock(global_lock);
    bool skip = PreCallValidateImportSemaphoreWin32HandleKHR(device, pImportSemaphoreWin32HandleInfo);

    if (!skip) {
        lock.unlock();
        result = dev_data->dispatch_table.ImportSemaphoreWin32HandleKHR(device, pImportSemaphoreWin32HandleInfo);
        lock.lock();
        PostCallRecordImportSemaphoreWin32HandleKHR(device, pImportSemaphoreWin32HandleInfo, result);
    }
    return result;
}
#endif

VKAPI_ATTR VkResult VKAPI_CALL ImportSemaphoreFdKHR(VkDevice device, const VkImportSemaphoreFdInfoKHR *pImportSemaphoreFdInfo) {
    VkResult result = VK_ERROR_VALIDATION_FAILED_EXT;
    layer_data *dev_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    unique_lock_t lock(global_lock);
    bool skip = PreCallValidateImportSemaphoreFdKHR(device, pImportSemaphoreFdInfo);

    if (!skip) {
        lock.unlock();
        result = dev_data->dispatch_table.ImportSemaphoreFdKHR(device, pImportSemaphoreFdInfo);
        lock.lock();
        PostCallRecordImportSemaphoreFdKHR(device, pImportSemaphoreFdInfo, result);
    }
    return result;
}

#ifdef VK_USE_PLATFORM_WIN32_KHR
VKAPI_ATTR VkResult VKAPI_CALL GetSemaphoreWin32HandleKHR(VkDevice device,
                                                          const VkSemaphoreGetWin32HandleInfoKHR *pGetWin32HandleInfo,
                                                          HANDLE *pHandle) {
    layer_data *dev_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    VkResult result = dev_data->dispatch_table.GetSemaphoreWin32HandleKHR(device, pGetWin32HandleInfo, pHandle);
    unique_lock_t lock(global_lock);
    PostCallRecordGetSemaphoreWin32HandleKHR(device, pGetWin32HandleInfo, pHandle, result);
    return result;
}
#endif

VKAPI_ATTR VkResult VKAPI_CALL GetSemaphoreFdKHR(VkDevice device, const VkSemaphoreGetFdInfoKHR *pGetFdInfo, int *pFd) {
    layer_data *dev_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    VkResult result = dev_data->dispatch_table.GetSemaphoreFdKHR(device, pGetFdInfo, pFd);
    unique_lock_t lock(global_lock);
    PostCallRecordGetSemaphoreFdKHR(device, pGetFdInfo, pFd, result);
    return result;
}

#ifdef VK_USE_PLATFORM_WIN32_KHR
VKAPI_ATTR VkResult VKAPI_CALL ImportFenceWin32HandleKHR(VkDevice device,
                                                         const VkImportFenceWin32HandleInfoKHR *pImportFenceWin32HandleInfo) {
    VkResult result = VK_ERROR_VALIDATION_FAILED_EXT;
    layer_data *dev_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    unique_lock_t lock(global_lock);
    bool skip = PreCallValidateImportFenceWin32HandleKHR(device, pImportFenceWin32HandleInfo);

    if (!skip) {
        lock.unlock();
        result = dev_data->dispatch_table.ImportFenceWin32HandleKHR(device, pImportFenceWin32HandleInfo);
        lock.lock();
        PostCallRecordImportFenceWin32HandleKHR(device, pImportFenceWin32HandleInfo, result);
    }
    return result;
}
#endif  // VK_USE_PLATFORM_WIN32_KHR

VKAPI_ATTR VkResult VKAPI_CALL ImportFenceFdKHR(VkDevice device, const VkImportFenceFdInfoKHR *pImportFenceFdInfo) {
    VkResult result = VK_ERROR_VALIDATION_FAILED_EXT;
    layer_data *dev_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    unique_lock_t lock(global_lock);
    bool skip = PreCallValidateImportFenceFdKHR(device, pImportFenceFdInfo);

    if (!skip) {
        lock.unlock();
        result = dev_data->dispatch_table.ImportFenceFdKHR(device, pImportFenceFdInfo);
        lock.lock();
        PostCallRecordImportFenceFdKHR(device, pImportFenceFdInfo, result);
    }
    return result;
}

#ifdef VK_USE_PLATFORM_WIN32_KHR
VKAPI_ATTR VkResult VKAPI_CALL GetFenceWin32HandleKHR(VkDevice device, const VkFenceGetWin32HandleInfoKHR *pGetWin32HandleInfo,
                                                      HANDLE *pHandle) {
    layer_data *dev_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    VkResult result = dev_data->dispatch_table.GetFenceWin32HandleKHR(device, pGetWin32HandleInfo, pHandle);
    unique_lock_t lock(global_lock);
    PostCallRecordGetFenceWin32HandleKHR(device, pGetWin32HandleInfo, pHandle, result);
    return result;
}
#endif

VKAPI_ATTR VkResult VKAPI_CALL GetFenceFdKHR(VkDevice device, const VkFenceGetFdInfoKHR *pGetFdInfo, int *pFd) {
    layer_data *dev_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    VkResult result = dev_data->dispatch_table.GetFenceFdKHR(device, pGetFdInfo, pFd);

    if (result == VK_SUCCESS) {
        PostCallRecordGetFenceFdKHR(device, pGetFdInfo, pFd, result);
    }
    return result;
}

VKAPI_ATTR VkResult VKAPI_CALL CreateEvent(VkDevice device, const VkEventCreateInfo *pCreateInfo,
                                           const VkAllocationCallbacks *pAllocator, VkEvent *pEvent) {
    layer_data *dev_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    VkResult result = dev_data->dispatch_table.CreateEvent(device, pCreateInfo, pAllocator, pEvent);
    lock_guard_t lock(global_lock);
    PostCallRecordCreateEvent(device, pCreateInfo, pAllocator, pEvent, result);
    return result;
}

VKAPI_ATTR VkResult VKAPI_CALL CreateSwapchainKHR(VkDevice device, const VkSwapchainCreateInfoKHR *pCreateInfo,
                                                  const VkAllocationCallbacks *pAllocator, VkSwapchainKHR *pSwapchain) {
    layer_data *dev_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    bool skip = false;
    unique_lock_t lock(global_lock);
    skip = PreCallValidateCreateSwapchainKHR(device, pCreateInfo, pAllocator, pSwapchain);
    if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
    lock.unlock();
    VkResult result = dev_data->dispatch_table.CreateSwapchainKHR(device, pCreateInfo, pAllocator, pSwapchain);
    lock.lock();
    PostCallRecordCreateSwapchainKHR(device, pCreateInfo, pAllocator, pSwapchain, result);
    return result;
}

VKAPI_ATTR void VKAPI_CALL DestroySwapchainKHR(VkDevice device, VkSwapchainKHR swapchain, const VkAllocationCallbacks *pAllocator) {
    layer_data *dev_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    unique_lock_t lock(global_lock);
    // Pre-record to avoid Destroy/Create race
    PreCallRecordDestroySwapchainKHR(device, swapchain, pAllocator);
    lock.unlock();
    dev_data->dispatch_table.DestroySwapchainKHR(device, swapchain, pAllocator);
}

VKAPI_ATTR VkResult VKAPI_CALL GetSwapchainImagesKHR(VkDevice device, VkSwapchainKHR swapchain, uint32_t *pSwapchainImageCount,
                                                     VkImage *pSwapchainImages) {
    VkResult result = VK_ERROR_VALIDATION_FAILED_EXT;
    layer_data *device_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    unique_lock_t lock(global_lock);
    bool skip = PreCallValidateGetSwapchainImagesKHR(device, swapchain, pSwapchainImageCount, pSwapchainImages);
    lock.unlock();
    if (!skip) {
        result = device_data->dispatch_table.GetSwapchainImagesKHR(device, swapchain, pSwapchainImageCount, pSwapchainImages);
    }
    lock.lock();
    PostCallRecordGetSwapchainImagesKHR(device, swapchain, pSwapchainImageCount, pSwapchainImages, result);
    lock.unlock();
    return result;
}

VKAPI_ATTR VkResult VKAPI_CALL QueuePresentKHR(VkQueue queue, const VkPresentInfoKHR *pPresentInfo) {
    layer_data *dev_data = GetLayerDataPtr(get_dispatch_key(queue), layer_data_map);

    unique_lock_t lock(global_lock);
    bool skip = PreCallValidateQueuePresentKHR(queue, pPresentInfo);
    lock.unlock();

    if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
    VkResult result = dev_data->dispatch_table.QueuePresentKHR(queue, pPresentInfo);
    lock.lock();
    PostCallRecordQueuePresentKHR(queue, pPresentInfo, result);
    lock.unlock();
    return result;
}  // namespace core_validation

VKAPI_ATTR VkResult VKAPI_CALL CreateSharedSwapchainsKHR(VkDevice device, uint32_t swapchainCount,
                                                         const VkSwapchainCreateInfoKHR *pCreateInfos,
                                                         const VkAllocationCallbacks *pAllocator, VkSwapchainKHR *pSwapchains) {
    layer_data *dev_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    bool skip = false;
    unique_lock_t lock(global_lock);
    skip = PreCallValidateCreateSharedSwapchainsKHR(device, swapchainCount, pCreateInfos, pAllocator, pSwapchains);
    if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
    lock.unlock();
    VkResult result =
        dev_data->dispatch_table.CreateSharedSwapchainsKHR(device, swapchainCount, pCreateInfos, pAllocator, pSwapchains);
    lock.lock();
    PostCallRecordCreateSharedSwapchainsKHR(device, swapchainCount, pCreateInfos, pAllocator, pSwapchains, result);
    return result;
}

VKAPI_ATTR VkResult VKAPI_CALL AcquireNextImageKHR(VkDevice device, VkSwapchainKHR swapchain, uint64_t timeout,
                                                   VkSemaphore semaphore, VkFence fence, uint32_t *pImageIndex) {
    layer_data *dev_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    unique_lock_t lock(global_lock);
    bool skip = PreCallValidateAcquireNextImageKHR(device, swapchain, timeout, semaphore, fence, pImageIndex);
    lock.unlock();
    if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;

    VkResult result = dev_data->dispatch_table.AcquireNextImageKHR(device, swapchain, timeout, semaphore, fence, pImageIndex);
    lock.lock();
    PostCallRecordAcquireNextImageKHR(device, swapchain, timeout, semaphore, fence, pImageIndex, result);
    lock.unlock();

    return result;
}

VKAPI_ATTR VkResult VKAPI_CALL AcquireNextImage2KHR(VkDevice device, const VkAcquireNextImageInfoKHR *pAcquireInfo,
                                                    uint32_t *pImageIndex) {
    layer_data *dev_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    unique_lock_t lock(global_lock);
    bool skip = PreCallValidateAcquireNextImage2KHR(device, pAcquireInfo, pImageIndex);
    lock.unlock();

    if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
    VkResult result = dev_data->dispatch_table.AcquireNextImage2KHR(device, pAcquireInfo, pImageIndex);
    lock.lock();
    PostCallRecordAcquireNextImage2KHR(device, pAcquireInfo, pImageIndex, result);
    // TODO: consider physical device masks
    lock.unlock();
    return result;
}

VKAPI_ATTR VkResult VKAPI_CALL EnumeratePhysicalDevices(VkInstance instance, uint32_t *pPhysicalDeviceCount,
                                                        VkPhysicalDevice *pPhysicalDevices) {
    bool skip = false;
    instance_layer_data *instance_data = GetLayerDataPtr(get_dispatch_key(instance), instance_layer_data_map);
    assert(instance_data);

    unique_lock_t lock(global_lock);
    skip |= PreCallValidateEnumeratePhysicalDevices(instance, pPhysicalDeviceCount, pPhysicalDevices);
    if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
    PreCallRecordEnumeratePhysicalDevices(instance, pPhysicalDeviceCount, pPhysicalDevices);
    lock.unlock();
    VkResult result = instance_data->dispatch_table.EnumeratePhysicalDevices(instance, pPhysicalDeviceCount, pPhysicalDevices);
    lock.lock();
    PostCallRecordEnumeratePhysicalDevices(instance, pPhysicalDeviceCount, pPhysicalDevices, result);
    return result;
}

VKAPI_ATTR void VKAPI_CALL GetPhysicalDeviceProperties(VkPhysicalDevice physicalDevice,
                                                       VkPhysicalDeviceProperties *pPhysicalDeviceProperties) {
    instance_layer_data *instance_data = GetLayerDataPtr(get_dispatch_key(physicalDevice), instance_layer_data_map);
    instance_data->dispatch_table.GetPhysicalDeviceProperties(physicalDevice, pPhysicalDeviceProperties);
    if (instance_data->enabled.gpu_validation && instance_data->enabled.gpu_validation_reserve_binding_slot) {
        if (pPhysicalDeviceProperties->limits.maxBoundDescriptorSets > 1) {
            pPhysicalDeviceProperties->limits.maxBoundDescriptorSets -= 1;
        } else {
            log_msg(instance_data->report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_PHYSICAL_DEVICE_EXT,
                    HandleToUint64(physicalDevice), "UNASSIGNED-GPU-Assisted Validation Setup Error.",
                    "Unable to reserve descriptor binding slot on a device with only one slot.");
        }
    }
}

VKAPI_ATTR void VKAPI_CALL GetPhysicalDeviceQueueFamilyProperties(VkPhysicalDevice physicalDevice,
                                                                  uint32_t *pQueueFamilyPropertyCount,
                                                                  VkQueueFamilyProperties *pQueueFamilyProperties) {
    instance_layer_data *instance_data = GetLayerDataPtr(get_dispatch_key(physicalDevice), instance_layer_data_map);
    unique_lock_t lock(global_lock);
    bool skip =
        PreCallValidateGetPhysicalDeviceQueueFamilyProperties(physicalDevice, pQueueFamilyPropertyCount, pQueueFamilyProperties);
    lock.unlock();
    if (skip) return;
    instance_data->dispatch_table.GetPhysicalDeviceQueueFamilyProperties(physicalDevice, pQueueFamilyPropertyCount,
                                                                         pQueueFamilyProperties);
    lock.lock();
    PostCallRecordGetPhysicalDeviceQueueFamilyProperties(physicalDevice, pQueueFamilyPropertyCount, pQueueFamilyProperties);
}

VKAPI_ATTR void VKAPI_CALL GetPhysicalDeviceQueueFamilyProperties2(VkPhysicalDevice physicalDevice,
                                                                   uint32_t *pQueueFamilyPropertyCount,
                                                                   VkQueueFamilyProperties2KHR *pQueueFamilyProperties) {
    instance_layer_data *instance_data = GetLayerDataPtr(get_dispatch_key(physicalDevice), instance_layer_data_map);
    unique_lock_t lock(global_lock);
    bool skip =
        PreCallValidateGetPhysicalDeviceQueueFamilyProperties2(physicalDevice, pQueueFamilyPropertyCount, pQueueFamilyProperties);
    lock.unlock();
    if (skip) return;

    instance_data->dispatch_table.GetPhysicalDeviceQueueFamilyProperties2(physicalDevice, pQueueFamilyPropertyCount,
                                                                          pQueueFamilyProperties);
    lock.lock();
    PostCallRecordGetPhysicalDeviceQueueFamilyProperties2(physicalDevice, pQueueFamilyPropertyCount, pQueueFamilyProperties);
}

VKAPI_ATTR void VKAPI_CALL GetPhysicalDeviceQueueFamilyProperties2KHR(VkPhysicalDevice physicalDevice,
                                                                      uint32_t *pQueueFamilyPropertyCount,
                                                                      VkQueueFamilyProperties2KHR *pQueueFamilyProperties) {
    instance_layer_data *instance_data = GetLayerDataPtr(get_dispatch_key(physicalDevice), instance_layer_data_map);
    unique_lock_t lock(global_lock);
    bool skip = PreCallValidateGetPhysicalDeviceQueueFamilyProperties2KHR(physicalDevice, pQueueFamilyPropertyCount,
                                                                          pQueueFamilyProperties);
    lock.unlock();
    if (skip) return;

    instance_data->dispatch_table.GetPhysicalDeviceQueueFamilyProperties2(physicalDevice, pQueueFamilyPropertyCount,
                                                                          pQueueFamilyProperties);
    lock.lock();
    PostCallRecordGetPhysicalDeviceQueueFamilyProperties2KHR(physicalDevice, pQueueFamilyPropertyCount, pQueueFamilyProperties);
}

VKAPI_ATTR void VKAPI_CALL DestroySurfaceKHR(VkInstance instance, VkSurfaceKHR surface, const VkAllocationCallbacks *pAllocator) {
    instance_layer_data *instance_data = GetLayerDataPtr(get_dispatch_key(instance), instance_layer_data_map);
    unique_lock_t lock(global_lock);
    bool skip = PreCallValidateDestroySurfaceKHR(instance, surface, pAllocator);
    if (!skip) {
        // Pre-record to avoid Destroy/Create race
        PreCallRecordValidateDestroySurfaceKHR(instance, surface, pAllocator);
        lock.unlock();
        instance_data->dispatch_table.DestroySurfaceKHR(instance, surface, pAllocator);
    }
}

VKAPI_ATTR VkResult VKAPI_CALL CreateDisplayPlaneSurfaceKHR(VkInstance instance, const VkDisplaySurfaceCreateInfoKHR *pCreateInfo,
                                                            const VkAllocationCallbacks *pAllocator, VkSurfaceKHR *pSurface) {
    instance_layer_data *instance_data = GetLayerDataPtr(get_dispatch_key(instance), instance_layer_data_map);
    VkResult result = (instance_data->dispatch_table.CreateDisplayPlaneSurfaceKHR)(instance, pCreateInfo, pAllocator, pSurface);
    unique_lock_t lock(global_lock);
    PostCallRecordCreateDisplayPlaneSurfaceKHR(instance, pCreateInfo, pAllocator, pSurface, result);
    return result;
}

#ifdef VK_USE_PLATFORM_ANDROID_KHR
VKAPI_ATTR VkResult VKAPI_CALL CreateAndroidSurfaceKHR(VkInstance instance, const VkAndroidSurfaceCreateInfoKHR *pCreateInfo,
                                                       const VkAllocationCallbacks *pAllocator, VkSurfaceKHR *pSurface) {
    instance_layer_data *instance_data = GetLayerDataPtr(get_dispatch_key(instance), instance_layer_data_map);
    VkResult result = (instance_data->dispatch_table.CreateAndroidSurfaceKHR)(instance, pCreateInfo, pAllocator, pSurface);
    unique_lock_t lock(global_lock);
    PostCallRecordCreateAndroidSurfaceKHR(instance, pCreateInfo, pAllocator, pSurface, result);
    return result;
}
#endif  // VK_USE_PLATFORM_ANDROID_KHR

#ifdef VK_USE_PLATFORM_IOS_MVK
VKAPI_ATTR VkResult VKAPI_CALL CreateIOSSurfaceMVK(VkInstance instance, const VkIOSSurfaceCreateInfoMVK *pCreateInfo,
                                                   const VkAllocationCallbacks *pAllocator, VkSurfaceKHR *pSurface) {
    instance_layer_data *instance_data = GetLayerDataPtr(get_dispatch_key(instance), instance_layer_data_map);
    VkResult result = (instance_data->dispatch_table.CreateIOSSurfaceMVK)(instance, pCreateInfo, pAllocator, pSurface);
    unique_lock_t lock(global_lock);
    PostCallRecordCreateIOSSurfaceMVK(instance, pCreateInfo, pAllocator, pSurface, result);
    return result;
}
#endif  // VK_USE_PLATFORM_IOS_MVK

#ifdef VK_USE_PLATFORM_MACOS_MVK
VKAPI_ATTR VkResult VKAPI_CALL CreateMacOSSurfaceMVK(VkInstance instance, const VkMacOSSurfaceCreateInfoMVK *pCreateInfo,
                                                     const VkAllocationCallbacks *pAllocator, VkSurfaceKHR *pSurface) {
    instance_layer_data *instance_data = GetLayerDataPtr(get_dispatch_key(instance), instance_layer_data_map);
    VkResult result = (instance_data->dispatch_table.CreateMacOSSurfaceMVK)(instance, pCreateInfo, pAllocator, pSurface);
    if (result == VK_SUCCESS) {
        unique_lock_t lock(global_lock);
        PostCallRecordCreateMacOSSurfaceMVK(instance, pCreateInfo, pAllocator, pSurface, result);
    }
    return result;
}
#endif  // VK_USE_PLATFORM_MACOS_MVK

#ifdef VK_USE_PLATFORM_WAYLAND_KHR
VKAPI_ATTR VkResult VKAPI_CALL CreateWaylandSurfaceKHR(VkInstance instance, const VkWaylandSurfaceCreateInfoKHR *pCreateInfo,
                                                       const VkAllocationCallbacks *pAllocator, VkSurfaceKHR *pSurface) {
    instance_layer_data *instance_data = GetLayerDataPtr(get_dispatch_key(instance), instance_layer_data_map);
    VkResult result = (instance_data->dispatch_table.CreateWaylandSurfaceKHR)(instance, pCreateInfo, pAllocator, pSurface);
    unique_lock_t lock(global_lock);
    PostCallRecordCreateWaylandSurfaceKHR(instance, pCreateInfo, pAllocator, pSurface, result);
    return result;
}

VKAPI_ATTR VkBool32 VKAPI_CALL GetPhysicalDeviceWaylandPresentationSupportKHR(VkPhysicalDevice physicalDevice,
                                                                              uint32_t queueFamilyIndex,
                                                                              struct wl_display *display) {
    instance_layer_data *instance_data = GetLayerDataPtr(get_dispatch_key(physicalDevice), instance_layer_data_map);
    unique_lock_t lock(global_lock);
    bool skip = PreCallValidateGetPhysicalDeviceWaylandPresentationSupportKHR(physicalDevice, queueFamilyIndex, display);
    lock.unlock();
    if (skip) return VK_FALSE;
    VkBool32 result =
        instance_data->dispatch_table.GetPhysicalDeviceWaylandPresentationSupportKHR(physicalDevice, queueFamilyIndex, display);
    return result;
}
#endif
#ifdef VK_USE_PLATFORM_WIN32_KHR
VKAPI_ATTR VkResult VKAPI_CALL CreateWin32SurfaceKHR(VkInstance instance, const VkWin32SurfaceCreateInfoKHR *pCreateInfo,
                                                     const VkAllocationCallbacks *pAllocator, VkSurfaceKHR *pSurface) {
    instance_layer_data *instance_data = GetLayerDataPtr(get_dispatch_key(instance), instance_layer_data_map);
    VkResult result = (instance_data->dispatch_table.CreateWin32SurfaceKHR)(instance, pCreateInfo, pAllocator, pSurface);
    unique_lock_t lock(global_lock);
    PostCallRecordCreateWin32SurfaceKHR(instance, pCreateInfo, pAllocator, pSurface, result);
    return result;
}

VKAPI_ATTR VkBool32 VKAPI_CALL GetPhysicalDeviceWin32PresentationSupportKHR(VkPhysicalDevice physicalDevice,
                                                                            uint32_t queueFamilyIndex) {
    instance_layer_data *instance_data = GetLayerDataPtr(get_dispatch_key(physicalDevice), instance_layer_data_map);

    unique_lock_t lock(global_lock);
    bool skip = PreCallValidateGetPhysicalDeviceWin32PresentationSupportKHR(physicalDevice, queueFamilyIndex);
    lock.unlock();
    if (skip) return VK_FALSE;
    VkBool32 result = instance_data->dispatch_table.GetPhysicalDeviceWin32PresentationSupportKHR(physicalDevice, queueFamilyIndex);
    return result;
}
#endif  // VK_USE_PLATFORM_WIN32_KHR

#ifdef VK_USE_PLATFORM_XCB_KHR
VKAPI_ATTR VkResult VKAPI_CALL CreateXcbSurfaceKHR(VkInstance instance, const VkXcbSurfaceCreateInfoKHR *pCreateInfo,
                                                   const VkAllocationCallbacks *pAllocator, VkSurfaceKHR *pSurface) {
    instance_layer_data *instance_data = GetLayerDataPtr(get_dispatch_key(instance), instance_layer_data_map);
    VkResult result = (instance_data->dispatch_table.CreateXcbSurfaceKHR)(instance, pCreateInfo, pAllocator, pSurface);
    unique_lock_t lock(global_lock);
    PostCallRecordCreateXcbSurfaceKHR(instance, pCreateInfo, pAllocator, pSurface, result);
    return result;
}

VKAPI_ATTR VkBool32 VKAPI_CALL GetPhysicalDeviceXcbPresentationSupportKHR(VkPhysicalDevice physicalDevice,
                                                                          uint32_t queueFamilyIndex, xcb_connection_t *connection,
                                                                          xcb_visualid_t visual_id) {
    instance_layer_data *instance_data = GetLayerDataPtr(get_dispatch_key(physicalDevice), instance_layer_data_map);
    unique_lock_t lock(global_lock);
    bool skip = PreCallValidateGetPhysicalDeviceXcbPresentationSupportKHR(physicalDevice, queueFamilyIndex, connection, visual_id);
    lock.unlock();
    if (skip) return VK_FALSE;
    VkBool32 result = instance_data->dispatch_table.GetPhysicalDeviceXcbPresentationSupportKHR(physicalDevice, queueFamilyIndex,
                                                                                               connection, visual_id);
    return result;
}
#endif  // VK_USE_PLATFORM_XCB_KHR
#ifdef VK_USE_PLATFORM_XLIB_KHR
VKAPI_ATTR VkResult VKAPI_CALL CreateXlibSurfaceKHR(VkInstance instance, const VkXlibSurfaceCreateInfoKHR *pCreateInfo,
                                                    const VkAllocationCallbacks *pAllocator, VkSurfaceKHR *pSurface) {
    instance_layer_data *instance_data = GetLayerDataPtr(get_dispatch_key(instance), instance_layer_data_map);
    VkResult result = (instance_data->dispatch_table.CreateXlibSurfaceKHR)(instance, pCreateInfo, pAllocator, pSurface);
    unique_lock_t lock(global_lock);
    PostCallRecordCreateXlibSurfaceKHR(instance, pCreateInfo, pAllocator, pSurface, result);
    return result;
}

VKAPI_ATTR VkBool32 VKAPI_CALL GetPhysicalDeviceXlibPresentationSupportKHR(VkPhysicalDevice physicalDevice,
                                                                           uint32_t queueFamilyIndex, Display *dpy,
                                                                           VisualID visualID) {
    instance_layer_data *instance_data = GetLayerDataPtr(get_dispatch_key(physicalDevice), instance_layer_data_map);
    unique_lock_t lock(global_lock);
    bool skip = PreCallValidateGetPhysicalDeviceXlibPresentationSupportKHR(physicalDevice, queueFamilyIndex, dpy, visualID);
    lock.unlock();
    if (skip) return VK_FALSE;
    VkBool32 result =
        instance_data->dispatch_table.GetPhysicalDeviceXlibPresentationSupportKHR(physicalDevice, queueFamilyIndex, dpy, visualID);
    return result;
}
#endif  // VK_USE_PLATFORM_XLIB_KHR

VKAPI_ATTR VkResult VKAPI_CALL GetPhysicalDeviceSurfaceCapabilitiesKHR(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface,
                                                                       VkSurfaceCapabilitiesKHR *pSurfaceCapabilities) {
    auto instance_data = GetLayerDataPtr(get_dispatch_key(physicalDevice), instance_layer_data_map);
    auto result =
        instance_data->dispatch_table.GetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice, surface, pSurfaceCapabilities);
    unique_lock_t lock(global_lock);
    PostCallRecordGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice, surface, pSurfaceCapabilities, result);
    return result;
}

VKAPI_ATTR VkResult VKAPI_CALL GetPhysicalDeviceSurfaceCapabilities2KHR(VkPhysicalDevice physicalDevice,
                                                                        const VkPhysicalDeviceSurfaceInfo2KHR *pSurfaceInfo,
                                                                        VkSurfaceCapabilities2KHR *pSurfaceCapabilities) {
    auto instance_data = GetLayerDataPtr(get_dispatch_key(physicalDevice), instance_layer_data_map);
    VkResult result =
        instance_data->dispatch_table.GetPhysicalDeviceSurfaceCapabilities2KHR(physicalDevice, pSurfaceInfo, pSurfaceCapabilities);
    unique_lock_t lock(global_lock);
    PostCallRecordGetPhysicalDeviceSurfaceCapabilities2KHR(physicalDevice, pSurfaceInfo, pSurfaceCapabilities, result);
    return result;
}

VKAPI_ATTR VkResult VKAPI_CALL GetPhysicalDeviceSurfaceCapabilities2EXT(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface,
                                                                        VkSurfaceCapabilities2EXT *pSurfaceCapabilities) {
    auto instance_data = GetLayerDataPtr(get_dispatch_key(physicalDevice), instance_layer_data_map);
    VkResult result =
        instance_data->dispatch_table.GetPhysicalDeviceSurfaceCapabilities2EXT(physicalDevice, surface, pSurfaceCapabilities);
    unique_lock_t lock(global_lock);
    PostCallRecordGetPhysicalDeviceSurfaceCapabilities2EXT(physicalDevice, surface, pSurfaceCapabilities, result);
    return result;
}

VKAPI_ATTR VkResult VKAPI_CALL GetPhysicalDeviceSurfaceSupportKHR(VkPhysicalDevice physicalDevice, uint32_t queueFamilyIndex,
                                                                  VkSurfaceKHR surface, VkBool32 *pSupported) {
    bool skip = false;
    auto instance_data = GetLayerDataPtr(get_dispatch_key(physicalDevice), instance_layer_data_map);
    unique_lock_t lock(global_lock);
    skip |= PreCallValidateGetPhysicalDeviceSurfaceSupportKHR(physicalDevice, queueFamilyIndex, surface, pSupported);
    lock.unlock();
    if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
    VkResult result =
        instance_data->dispatch_table.GetPhysicalDeviceSurfaceSupportKHR(physicalDevice, queueFamilyIndex, surface, pSupported);

    lock.lock();
    PostCallRecordGetPhysicalDeviceSurfaceSupportKHR(physicalDevice, queueFamilyIndex, surface, pSupported, result);

    return result;
}

VKAPI_ATTR VkResult VKAPI_CALL GetPhysicalDeviceSurfacePresentModesKHR(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface,
                                                                       uint32_t *pPresentModeCount,
                                                                       VkPresentModeKHR *pPresentModes) {
    auto instance_data = GetLayerDataPtr(get_dispatch_key(physicalDevice), instance_layer_data_map);
    auto result = instance_data->dispatch_table.GetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface, pPresentModeCount,
                                                                                        pPresentModes);
    unique_lock_t lock(global_lock);
    PostCallRecordGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface, pPresentModeCount, pPresentModes, result);
    return result;
}

VKAPI_ATTR VkResult VKAPI_CALL GetPhysicalDeviceSurfaceFormatsKHR(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface,
                                                                  uint32_t *pSurfaceFormatCount,
                                                                  VkSurfaceFormatKHR *pSurfaceFormats) {
    bool skip = false;
    auto instance_data = GetLayerDataPtr(get_dispatch_key(physicalDevice), instance_layer_data_map);
    unique_lock_t lock(global_lock);
    skip |= PreCallValidateGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, pSurfaceFormatCount, pSurfaceFormats);
    lock.unlock();
    if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;

    auto result = instance_data->dispatch_table.GetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, pSurfaceFormatCount,
                                                                                   pSurfaceFormats);
    lock.lock();
    PostCallRecordGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, pSurfaceFormatCount, pSurfaceFormats, result);
    return result;
}

VKAPI_ATTR VkResult VKAPI_CALL GetPhysicalDeviceSurfaceFormats2KHR(VkPhysicalDevice physicalDevice,
                                                                   const VkPhysicalDeviceSurfaceInfo2KHR *pSurfaceInfo,
                                                                   uint32_t *pSurfaceFormatCount,
                                                                   VkSurfaceFormat2KHR *pSurfaceFormats) {
    auto instance_data = GetLayerDataPtr(get_dispatch_key(physicalDevice), instance_layer_data_map);
    auto result = instance_data->dispatch_table.GetPhysicalDeviceSurfaceFormats2KHR(physicalDevice, pSurfaceInfo,
                                                                                    pSurfaceFormatCount, pSurfaceFormats);
    unique_lock_t lock(global_lock);
    PostCallRecordGetPhysicalDeviceSurfaceFormats2KHR(physicalDevice, pSurfaceInfo, pSurfaceFormatCount, pSurfaceFormats, result);
    return result;
}

VKAPI_ATTR VkResult VKAPI_CALL SetDebugUtilsObjectNameEXT(VkDevice device, const VkDebugUtilsObjectNameInfoEXT *pNameInfo) {
    layer_data *dev_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    VkResult result = VK_SUCCESS;
    unique_lock_t lock(global_lock);
    PreCallRecordSetDebugUtilsObjectNameEXT(device, pNameInfo);
    lock.unlock();
    result = dev_data->dispatch_table.SetDebugUtilsObjectNameEXT(device, pNameInfo);
    return result;
}

VKAPI_ATTR VkResult VKAPI_CALL SetDebugUtilsObjectTagEXT(VkDevice device, const VkDebugUtilsObjectTagInfoEXT *pTagInfo) {
    layer_data *dev_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    VkResult result = VK_SUCCESS;
    if (nullptr != dev_data->dispatch_table.SetDebugUtilsObjectTagEXT) {
        result = dev_data->dispatch_table.SetDebugUtilsObjectTagEXT(device, pTagInfo);
    }
    return result;
}

VKAPI_ATTR void VKAPI_CALL QueueBeginDebugUtilsLabelEXT(VkQueue queue, const VkDebugUtilsLabelEXT *pLabelInfo) {
    layer_data *dev_data = GetLayerDataPtr(get_dispatch_key(queue), layer_data_map);
    std::unique_lock<std::mutex> lock(global_lock);
    PreCallRecordQueueBeginDebugUtilsLabelEXT(queue, pLabelInfo);
    lock.unlock();
    if (nullptr != dev_data->dispatch_table.QueueBeginDebugUtilsLabelEXT) {
        dev_data->dispatch_table.QueueBeginDebugUtilsLabelEXT(queue, pLabelInfo);
    }
}

VKAPI_ATTR void VKAPI_CALL QueueEndDebugUtilsLabelEXT(VkQueue queue) {
    layer_data *dev_data = GetLayerDataPtr(get_dispatch_key(queue), layer_data_map);
    if (nullptr != dev_data->dispatch_table.QueueEndDebugUtilsLabelEXT) {
        dev_data->dispatch_table.QueueEndDebugUtilsLabelEXT(queue);
    }
    lock_guard_t lock(global_lock);
    PostCallRecordQueueEndDebugUtilsLabelEXT(queue);
}

VKAPI_ATTR void VKAPI_CALL QueueInsertDebugUtilsLabelEXT(VkQueue queue, const VkDebugUtilsLabelEXT *pLabelInfo) {
    layer_data *dev_data = GetLayerDataPtr(get_dispatch_key(queue), layer_data_map);
    std::unique_lock<std::mutex> lock(global_lock);
    PreCallRecordQueueInsertDebugUtilsLabelEXT(queue, pLabelInfo);
    lock.unlock();
    if (nullptr != dev_data->dispatch_table.QueueInsertDebugUtilsLabelEXT) {
        dev_data->dispatch_table.QueueInsertDebugUtilsLabelEXT(queue, pLabelInfo);
    }
}

VKAPI_ATTR void VKAPI_CALL CmdBeginDebugUtilsLabelEXT(VkCommandBuffer commandBuffer, const VkDebugUtilsLabelEXT *pLabelInfo) {
    layer_data *dev_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    std::unique_lock<std::mutex> lock(global_lock);
    PreCallRecordCmdBeginDebugUtilsLabelEXT(commandBuffer, pLabelInfo);
    lock.unlock();
    if (nullptr != dev_data->dispatch_table.CmdBeginDebugUtilsLabelEXT) {
        dev_data->dispatch_table.CmdBeginDebugUtilsLabelEXT(commandBuffer, pLabelInfo);
    }
}

VKAPI_ATTR void VKAPI_CALL CmdEndDebugUtilsLabelEXT(VkCommandBuffer commandBuffer) {
    layer_data *dev_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    dev_data->dispatch_table.CmdEndDebugUtilsLabelEXT(commandBuffer);
    lock_guard_t lock(global_lock);
    PostCallRecordCmdEndDebugUtilsLabelEXT(commandBuffer);
}

VKAPI_ATTR void VKAPI_CALL CmdInsertDebugUtilsLabelEXT(VkCommandBuffer commandBuffer, const VkDebugUtilsLabelEXT *pLabelInfo) {
    layer_data *dev_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    std::unique_lock<std::mutex> lock(global_lock);
    PreCallRecordCmdInsertDebugUtilsLabelEXT(commandBuffer, pLabelInfo);
    lock.unlock();
    if (nullptr != dev_data->dispatch_table.CmdInsertDebugUtilsLabelEXT) {
        dev_data->dispatch_table.CmdInsertDebugUtilsLabelEXT(commandBuffer, pLabelInfo);
    }
}

VKAPI_ATTR VkResult VKAPI_CALL CreateDebugUtilsMessengerEXT(VkInstance instance,
                                                            const VkDebugUtilsMessengerCreateInfoEXT *pCreateInfo,
                                                            const VkAllocationCallbacks *pAllocator,
                                                            VkDebugUtilsMessengerEXT *pMessenger) {
    instance_layer_data *instance_data = GetLayerDataPtr(get_dispatch_key(instance), instance_layer_data_map);
    VkResult result = instance_data->dispatch_table.CreateDebugUtilsMessengerEXT(instance, pCreateInfo, pAllocator, pMessenger);
    std::unique_lock<std::mutex> lock(global_lock);
    PostCallRecordCreateDebugUtilsMessengerEXT(instance, pCreateInfo, pAllocator, pMessenger, result);
    return result;
}

VKAPI_ATTR void VKAPI_CALL DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT messenger,
                                                         const VkAllocationCallbacks *pAllocator) {
    instance_layer_data *instance_data = GetLayerDataPtr(get_dispatch_key(instance), instance_layer_data_map);
    instance_data->dispatch_table.DestroyDebugUtilsMessengerEXT(instance, messenger, pAllocator);
    std::unique_lock<std::mutex> lock(global_lock);
    PostCallRecordDestroyDebugUtilsMessengerEXT(instance, messenger, pAllocator);
    lock.unlock();
}

VKAPI_ATTR void VKAPI_CALL SubmitDebugUtilsMessageEXT(VkInstance instance, VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
                                                      VkDebugUtilsMessageTypeFlagsEXT messageTypes,
                                                      const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData) {
    instance_layer_data *instance_data = GetLayerDataPtr(get_dispatch_key(instance), instance_layer_data_map);
    instance_data->dispatch_table.SubmitDebugUtilsMessageEXT(instance, messageSeverity, messageTypes, pCallbackData);
}

VKAPI_ATTR VkResult VKAPI_CALL CreateDebugReportCallbackEXT(VkInstance instance,
                                                            const VkDebugReportCallbackCreateInfoEXT *pCreateInfo,
                                                            const VkAllocationCallbacks *pAllocator,
                                                            VkDebugReportCallbackEXT *pMsgCallback) {
    instance_layer_data *instance_data = GetLayerDataPtr(get_dispatch_key(instance), instance_layer_data_map);
    VkResult result = instance_data->dispatch_table.CreateDebugReportCallbackEXT(instance, pCreateInfo, pAllocator, pMsgCallback);
    lock_guard_t lock(global_lock);
    PostCallRecordCreateDebugReportCallbackEXT(instance, pCreateInfo, pAllocator, pMsgCallback, result);
    return result;
}

VKAPI_ATTR void VKAPI_CALL DestroyDebugReportCallbackEXT(VkInstance instance, VkDebugReportCallbackEXT msgCallback,
                                                         const VkAllocationCallbacks *pAllocator) {
    instance_layer_data *instance_data = GetLayerDataPtr(get_dispatch_key(instance), instance_layer_data_map);
    instance_data->dispatch_table.DestroyDebugReportCallbackEXT(instance, msgCallback, pAllocator);
    lock_guard_t lock(global_lock);
    PostCallDestroyDebugReportCallbackEXT(instance, msgCallback, pAllocator);
}

VKAPI_ATTR void VKAPI_CALL DebugReportMessageEXT(VkInstance instance, VkDebugReportFlagsEXT flags,
                                                 VkDebugReportObjectTypeEXT objType, uint64_t object, size_t location,
                                                 int32_t msgCode, const char *pLayerPrefix, const char *pMsg) {
    instance_layer_data *instance_data = GetLayerDataPtr(get_dispatch_key(instance), instance_layer_data_map);
    instance_data->dispatch_table.DebugReportMessageEXT(instance, flags, objType, object, location, msgCode, pLayerPrefix, pMsg);
}

VKAPI_ATTR VkResult VKAPI_CALL EnumeratePhysicalDeviceGroups(VkInstance instance, uint32_t *pPhysicalDeviceGroupCount,
                                                             VkPhysicalDeviceGroupPropertiesKHR *pPhysicalDeviceGroupProperties) {
    bool skip = false;
    instance_layer_data *instance_data = GetLayerDataPtr(get_dispatch_key(instance), instance_layer_data_map);
    unique_lock_t lock(global_lock);
    skip = PreCallValidateEnumeratePhysicalDeviceGroups(instance, pPhysicalDeviceGroupCount, pPhysicalDeviceGroupProperties);
    if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
    PreCallRecordEnumeratePhysicalDeviceGroups(instance, pPhysicalDeviceGroupCount, pPhysicalDeviceGroupProperties);
    lock.unlock();
    VkResult result = instance_data->dispatch_table.EnumeratePhysicalDeviceGroups(instance, pPhysicalDeviceGroupCount,
                                                                                  pPhysicalDeviceGroupProperties);
    lock.lock();
    PostCallRecordEnumeratePhysicalDeviceGroups(instance, pPhysicalDeviceGroupCount, pPhysicalDeviceGroupProperties, result);
    return result;
}

VKAPI_ATTR VkResult VKAPI_CALL EnumeratePhysicalDeviceGroupsKHR(
    VkInstance instance, uint32_t *pPhysicalDeviceGroupCount, VkPhysicalDeviceGroupPropertiesKHR *pPhysicalDeviceGroupProperties) {
    bool skip = false;
    instance_layer_data *instance_data = GetLayerDataPtr(get_dispatch_key(instance), instance_layer_data_map);
    unique_lock_t lock(global_lock);

    skip = PreCallValidateEnumeratePhysicalDeviceGroupsKHR(instance, pPhysicalDeviceGroupCount, pPhysicalDeviceGroupProperties);
    if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
    lock.unlock();
    PreCallRecordEnumeratePhysicalDeviceGroupsKHR(instance, pPhysicalDeviceGroupCount, pPhysicalDeviceGroupProperties);
    VkResult result = instance_data->dispatch_table.EnumeratePhysicalDeviceGroupsKHR(instance, pPhysicalDeviceGroupCount,
                                                                                     pPhysicalDeviceGroupProperties);
    lock.lock();
    PostCallRecordEnumeratePhysicalDeviceGroupsKHR(instance, pPhysicalDeviceGroupCount, pPhysicalDeviceGroupProperties, result);
    return result;
}

VKAPI_ATTR VkResult VKAPI_CALL CreateDescriptorUpdateTemplate(VkDevice device,
                                                              const VkDescriptorUpdateTemplateCreateInfoKHR *pCreateInfo,
                                                              const VkAllocationCallbacks *pAllocator,
                                                              VkDescriptorUpdateTemplateKHR *pDescriptorUpdateTemplate) {
    layer_data *device_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    unique_lock_t lock(global_lock);
    bool skip = PreCallValidateCreateDescriptorUpdateTemplate(device, pCreateInfo, pAllocator, pDescriptorUpdateTemplate);

    VkResult result = VK_ERROR_VALIDATION_FAILED_EXT;
    if (!skip) {
        lock.unlock();
        result =
            device_data->dispatch_table.CreateDescriptorUpdateTemplate(device, pCreateInfo, pAllocator, pDescriptorUpdateTemplate);
        lock.lock();
        PostCallRecordCreateDescriptorUpdateTemplate(device, pCreateInfo, pAllocator, pDescriptorUpdateTemplate, result);
    }
    return result;
}

VKAPI_ATTR VkResult VKAPI_CALL CreateDescriptorUpdateTemplateKHR(VkDevice device,
                                                                 const VkDescriptorUpdateTemplateCreateInfoKHR *pCreateInfo,
                                                                 const VkAllocationCallbacks *pAllocator,
                                                                 VkDescriptorUpdateTemplateKHR *pDescriptorUpdateTemplate) {
    layer_data *device_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    unique_lock_t lock(global_lock);
    bool skip = PreCallValidateCreateDescriptorUpdateTemplate(device, pCreateInfo, pAllocator, pDescriptorUpdateTemplate);

    VkResult result = VK_ERROR_VALIDATION_FAILED_EXT;
    if (!skip) {
        lock.unlock();
        result = device_data->dispatch_table.CreateDescriptorUpdateTemplateKHR(device, pCreateInfo, pAllocator,
                                                                               pDescriptorUpdateTemplate);
        lock.lock();
        PostCallRecordCreateDescriptorUpdateTemplate(device, pCreateInfo, pAllocator, pDescriptorUpdateTemplate, result);
    }
    return result;
}

VKAPI_ATTR void VKAPI_CALL DestroyDescriptorUpdateTemplate(VkDevice device, VkDescriptorUpdateTemplateKHR descriptorUpdateTemplate,
                                                           const VkAllocationCallbacks *pAllocator) {
    layer_data *device_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    unique_lock_t lock(global_lock);
    // Pre-record to avoid Destroy/Create race
    PreCallRecordDestroyDescriptorUpdateTemplate(device, descriptorUpdateTemplate, pAllocator);
    lock.unlock();
    device_data->dispatch_table.DestroyDescriptorUpdateTemplate(device, descriptorUpdateTemplate, pAllocator);
}

VKAPI_ATTR void VKAPI_CALL DestroyDescriptorUpdateTemplateKHR(VkDevice device,
                                                              VkDescriptorUpdateTemplateKHR descriptorUpdateTemplate,
                                                              const VkAllocationCallbacks *pAllocator) {
    layer_data *device_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    unique_lock_t lock(global_lock);
    // Pre-record to avoid Destroy/Create race
    PreCallRecordDestroyDescriptorUpdateTemplateKHR(device, descriptorUpdateTemplate, pAllocator);
    lock.unlock();
    device_data->dispatch_table.DestroyDescriptorUpdateTemplateKHR(device, descriptorUpdateTemplate, pAllocator);
}

VKAPI_ATTR void VKAPI_CALL UpdateDescriptorSetWithTemplate(VkDevice device, VkDescriptorSet descriptorSet,
                                                           VkDescriptorUpdateTemplateKHR descriptorUpdateTemplate,
                                                           const void *pData) {
    layer_data *device_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);

    unique_lock_t lock(global_lock);
    bool skip = false;
    skip = PreCallValidateUpdateDescriptorSetWithTemplate(device, descriptorSet, descriptorUpdateTemplate, pData);

    if (!skip) {
        PreCallRecordUpdateDescriptorSetWithTemplate(device, descriptorSet, descriptorUpdateTemplate, pData);
        lock.unlock();
        device_data->dispatch_table.UpdateDescriptorSetWithTemplate(device, descriptorSet, descriptorUpdateTemplate, pData);
    }
}

VKAPI_ATTR void VKAPI_CALL UpdateDescriptorSetWithTemplateKHR(VkDevice device, VkDescriptorSet descriptorSet,
                                                              VkDescriptorUpdateTemplateKHR descriptorUpdateTemplate,
                                                              const void *pData) {
    layer_data *device_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);

    unique_lock_t lock(global_lock);
    bool skip = false;
    skip = PreCallValidateUpdateDescriptorSetWithTemplate(device, descriptorSet, descriptorUpdateTemplate, pData);

    if (!skip) {
        PreCallRecordUpdateDescriptorSetWithTemplate(device, descriptorSet, descriptorUpdateTemplate, pData);
        lock.unlock();
        device_data->dispatch_table.UpdateDescriptorSetWithTemplateKHR(device, descriptorSet, descriptorUpdateTemplate, pData);
    }
}

VKAPI_ATTR void VKAPI_CALL CmdPushDescriptorSetWithTemplateKHR(VkCommandBuffer commandBuffer,
                                                               VkDescriptorUpdateTemplateKHR descriptorUpdateTemplate,
                                                               VkPipelineLayout layout, uint32_t set, const void *pData) {
    layer_data *dev_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    unique_lock_t lock(global_lock);
    bool skip = false;
    skip |= PreCallValidateCmdPushDescriptorSetWithTemplateKHR(commandBuffer, descriptorUpdateTemplate, layout, set, pData);
    if (!skip) {
        PreCallRecordCmdPushDescriptorSetWithTemplateKHR(commandBuffer, descriptorUpdateTemplate, layout, set, pData);
        lock.unlock();
        dev_data->dispatch_table.CmdPushDescriptorSetWithTemplateKHR(commandBuffer, descriptorUpdateTemplate, layout, set, pData);
    }
}

VKAPI_ATTR VkResult VKAPI_CALL GetPhysicalDeviceDisplayPlanePropertiesKHR(VkPhysicalDevice physicalDevice, uint32_t *pPropertyCount,
                                                                          VkDisplayPlanePropertiesKHR *pProperties) {
    VkResult result = VK_SUCCESS;
    instance_layer_data *instance_data = GetLayerDataPtr(get_dispatch_key(physicalDevice), instance_layer_data_map);
    result = instance_data->dispatch_table.GetPhysicalDeviceDisplayPlanePropertiesKHR(physicalDevice, pPropertyCount, pProperties);
    unique_lock_t lock(global_lock);
    PostCallRecordGetPhysicalDeviceDisplayPlanePropertiesKHR(physicalDevice, pPropertyCount, pProperties, result);
    return result;
}

VKAPI_ATTR VkResult VKAPI_CALL GetPhysicalDeviceDisplayPlaneProperties2KHR(VkPhysicalDevice physicalDevice,
                                                                           uint32_t *pPropertyCount,
                                                                           VkDisplayPlaneProperties2KHR *pProperties) {
    VkResult result = VK_SUCCESS;
    instance_layer_data *instance_data = GetLayerDataPtr(get_dispatch_key(physicalDevice), instance_layer_data_map);
    result = instance_data->dispatch_table.GetPhysicalDeviceDisplayPlaneProperties2KHR(physicalDevice, pPropertyCount, pProperties);
    unique_lock_t lock(global_lock);
    PostCallRecordGetPhysicalDeviceDisplayPlaneProperties2KHR(physicalDevice, pPropertyCount, pProperties, result);
    return result;
}

VKAPI_ATTR VkResult VKAPI_CALL GetDisplayPlaneSupportedDisplaysKHR(VkPhysicalDevice physicalDevice, uint32_t planeIndex,
                                                                   uint32_t *pDisplayCount, VkDisplayKHR *pDisplays) {
    VkResult result = VK_ERROR_VALIDATION_FAILED_EXT;
    instance_layer_data *instance_data = GetLayerDataPtr(get_dispatch_key(physicalDevice), instance_layer_data_map);
    unique_lock_t lock(global_lock);
    bool skip = PreCallValidateGetDisplayPlaneSupportedDisplaysKHR(physicalDevice, planeIndex, pDisplayCount, pDisplays);
    lock.unlock();
    if (!skip) {
        result =
            instance_data->dispatch_table.GetDisplayPlaneSupportedDisplaysKHR(physicalDevice, planeIndex, pDisplayCount, pDisplays);
    }
    return result;
}

VKAPI_ATTR VkResult VKAPI_CALL GetDisplayPlaneCapabilitiesKHR(VkPhysicalDevice physicalDevice, VkDisplayModeKHR mode,
                                                              uint32_t planeIndex, VkDisplayPlaneCapabilitiesKHR *pCapabilities) {
    VkResult result = VK_ERROR_VALIDATION_FAILED_EXT;
    instance_layer_data *instance_data = GetLayerDataPtr(get_dispatch_key(physicalDevice), instance_layer_data_map);
    unique_lock_t lock(global_lock);
    bool skip = PreCallValidateGetDisplayPlaneCapabilitiesKHR(physicalDevice, mode, planeIndex, pCapabilities);
    lock.unlock();
    if (!skip) {
        result = instance_data->dispatch_table.GetDisplayPlaneCapabilitiesKHR(physicalDevice, mode, planeIndex, pCapabilities);
    }
    return result;
}

VKAPI_ATTR VkResult VKAPI_CALL GetDisplayPlaneCapabilities2KHR(VkPhysicalDevice physicalDevice,
                                                               const VkDisplayPlaneInfo2KHR *pDisplayPlaneInfo,
                                                               VkDisplayPlaneCapabilities2KHR *pCapabilities) {
    VkResult result = VK_ERROR_VALIDATION_FAILED_EXT;
    instance_layer_data *instance_data = GetLayerDataPtr(get_dispatch_key(physicalDevice), instance_layer_data_map);
    unique_lock_t lock(global_lock);
    bool skip = PreCallValidateGetDisplayPlaneCapabilities2KHR(physicalDevice, pDisplayPlaneInfo, pCapabilities);
    lock.unlock();
    if (!skip) {
        result = instance_data->dispatch_table.GetDisplayPlaneCapabilities2KHR(physicalDevice, pDisplayPlaneInfo, pCapabilities);
    }
    return result;
}

VKAPI_ATTR VkResult VKAPI_CALL DebugMarkerSetObjectNameEXT(VkDevice device, const VkDebugMarkerObjectNameInfoEXT *pNameInfo) {
    layer_data *device_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    unique_lock_t lock(global_lock);
    PreCallRecordDebugMarkerSetObjectNameEXT(device, pNameInfo);
    lock.unlock();
    VkResult result = device_data->dispatch_table.DebugMarkerSetObjectNameEXT(device, pNameInfo);
    return result;
}

VKAPI_ATTR void VKAPI_CALL CmdDebugMarkerEndEXT(VkCommandBuffer commandBuffer) {
    layer_data *device_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    unique_lock_t lock(global_lock);
    bool skip = false;
    skip |= PreCallValidateCmdDebugMarkerEndEXT(commandBuffer);
    lock.unlock();
    if (!skip) {
        device_data->dispatch_table.CmdDebugMarkerEndEXT(commandBuffer);
    }
}

VKAPI_ATTR void VKAPI_CALL CmdSetDiscardRectangleEXT(VkCommandBuffer commandBuffer, uint32_t firstDiscardRectangle,
                                                     uint32_t discardRectangleCount, const VkRect2D *pDiscardRectangles) {
    layer_data *dev_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    unique_lock_t lock(global_lock);
    bool skip = false;
    skip |=
        PreCallValidateCmdSetDiscardRectangleEXT(commandBuffer, firstDiscardRectangle, discardRectangleCount, pDiscardRectangles);
    lock.unlock();

    if (!skip) {
        dev_data->dispatch_table.CmdSetDiscardRectangleEXT(commandBuffer, firstDiscardRectangle, discardRectangleCount,
                                                           pDiscardRectangles);
    }
}

VKAPI_ATTR void VKAPI_CALL CmdSetSampleLocationsEXT(VkCommandBuffer commandBuffer,
                                                    const VkSampleLocationsInfoEXT *pSampleLocationsInfo) {
    layer_data *dev_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    unique_lock_t lock(global_lock);
    bool skip = false;
    skip |= PreCallValidateCmdSetSampleLocationsEXT(commandBuffer, pSampleLocationsInfo);
    lock.unlock();

    if (!skip) {
        dev_data->dispatch_table.CmdSetSampleLocationsEXT(commandBuffer, pSampleLocationsInfo);
    }
}

VKAPI_ATTR void VKAPI_CALL CmdDrawIndirectCountKHR(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                   VkBuffer countBuffer, VkDeviceSize countBufferOffset, uint32_t maxDrawCount,
                                                   uint32_t stride) {
    layer_data *dev_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    unique_lock_t lock(global_lock);
    bool skip =
        PreCallValidateCmdDrawIndirectCountKHR(commandBuffer, buffer, offset, countBuffer, countBufferOffset, maxDrawCount, stride);
    if (!skip) {
        PreCallRecordCmdDrawIndirectCountKHR(commandBuffer, buffer, offset, countBuffer, countBufferOffset, maxDrawCount, stride);
        lock.unlock();
        dev_data->dispatch_table.CmdDrawIndirectCountKHR(commandBuffer, buffer, offset, countBuffer, countBufferOffset,
                                                         maxDrawCount, stride);
    }
}

VKAPI_ATTR void VKAPI_CALL CmdDrawIndexedIndirectCountKHR(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                          VkBuffer countBuffer, VkDeviceSize countBufferOffset,
                                                          uint32_t maxDrawCount, uint32_t stride) {
    layer_data *dev_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    unique_lock_t lock(global_lock);
    bool skip = PreCallValidateCmdDrawIndexedIndirectCountKHR(commandBuffer, buffer, offset, countBuffer, countBufferOffset,
                                                              maxDrawCount, stride);
    if (!skip) {
        PreCallRecordCmdDrawIndexedIndirectCountKHR(commandBuffer, buffer, offset, countBuffer, countBufferOffset, maxDrawCount,
                                                    stride);
        lock.unlock();
        dev_data->dispatch_table.CmdDrawIndexedIndirectCountKHR(commandBuffer, buffer, offset, countBuffer, countBufferOffset,
                                                                maxDrawCount, stride);
    }
}

VKAPI_ATTR void VKAPI_CALL CmdDrawMeshTasksNV(VkCommandBuffer commandBuffer, uint32_t taskCount, uint32_t firstTask) {
    layer_data *dev_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    bool skip = false;

    unique_lock_t lock(global_lock);
    skip |= PreCallValidateCmdDrawMeshTasksNV(commandBuffer, taskCount, firstTask);

    if (!skip) {
        PreCallRecordCmdDrawMeshTasksNV(commandBuffer, taskCount, firstTask);
        lock.unlock();
        dev_data->dispatch_table.CmdDrawMeshTasksNV(commandBuffer, taskCount, firstTask);
    }
}

VKAPI_ATTR void VKAPI_CALL CmdDrawMeshTasksIndirectNV(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                      uint32_t drawCount, uint32_t stride) {
    layer_data *dev_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    bool skip = false;

    unique_lock_t lock(global_lock);
    skip |= PreCallValidateCmdDrawMeshTasksIndirectNV(commandBuffer, buffer, offset, drawCount, stride);

    if (!skip) {
        PreCallRecordCmdDrawMeshTasksIndirectNV(commandBuffer, buffer, offset, drawCount, stride);
        lock.unlock();
        dev_data->dispatch_table.CmdDrawMeshTasksIndirectNV(commandBuffer, buffer, offset, drawCount, stride);
    }
}

VKAPI_ATTR void VKAPI_CALL CmdDrawMeshTasksIndirectCountNV(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                           VkBuffer countBuffer, VkDeviceSize countBufferOffset,
                                                           uint32_t maxDrawCount, uint32_t stride) {
    layer_data *dev_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    bool skip = false;

    unique_lock_t lock(global_lock);
    skip |= PreCallValidateCmdDrawMeshTasksIndirectCountNV(commandBuffer, buffer, offset, countBuffer, countBufferOffset,
                                                           maxDrawCount, stride);

    if (!skip) {
        PreCallRecordCmdDrawMeshTasksIndirectCountNV(commandBuffer, buffer, offset, countBuffer, countBufferOffset, maxDrawCount,
                                                     stride);
        lock.unlock();
        dev_data->dispatch_table.CmdDrawMeshTasksIndirectCountNV(commandBuffer, buffer, offset, countBuffer, countBufferOffset,
                                                                 maxDrawCount, stride);
    }
}

VKAPI_ATTR void VKAPI_CALL DestroySamplerYcbcrConversion(VkDevice device, VkSamplerYcbcrConversion ycbcrConversion,
                                                         const VkAllocationCallbacks *pAllocator) {
    layer_data *dev_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    dev_data->dispatch_table.DestroySamplerYcbcrConversion(device, ycbcrConversion, pAllocator);
    unique_lock_t lock(global_lock);
    PostCallRecordDestroySamplerYcbcrConversion(device, ycbcrConversion, pAllocator);
    lock.unlock();
};

VKAPI_ATTR void VKAPI_CALL DestroySamplerYcbcrConversionKHR(VkDevice device, VkSamplerYcbcrConversion ycbcrConversion,
                                                            const VkAllocationCallbacks *pAllocator) {
    layer_data *dev_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    dev_data->dispatch_table.DestroySamplerYcbcrConversionKHR(device, ycbcrConversion, pAllocator);
    unique_lock_t lock(global_lock);
    PostCallRecordDestroySamplerYcbcrConversionKHR(device, ycbcrConversion, pAllocator);
    lock.unlock();
};

VKAPI_ATTR VkResult VKAPI_CALL CreateGraphicsPipelines(VkDevice device, VkPipelineCache pipelineCache, uint32_t count,
                                                       const VkGraphicsPipelineCreateInfo *pCreateInfos,
                                                       const VkAllocationCallbacks *pAllocator, VkPipeline *pPipelines) {
    // The order of operations here is a little convoluted but gets the job done
    //  1. Pipeline create state is first shadowed into PIPELINE_STATE struct
    //  2. Create state is then validated (which uses flags setup during shadowing)
    //  3. If everything looks good, we'll then create the pipeline and add NODE to pipelineMap
    std::vector<std::unique_ptr<PIPELINE_STATE>> pipe_state;
    layer_data *dev_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    unique_lock_t lock(global_lock);

    bool skip = PreCallValidateCreateGraphicsPipelines(dev_data, &pipe_state, count, pCreateInfos);

    if (skip) {
        for (uint32_t i = 0; i < count; i++) {
            pPipelines[i] = VK_NULL_HANDLE;
        }
        return VK_ERROR_VALIDATION_FAILED_EXT;
    }

    // GPU Validation can possibly replace instrumented shaders with non-instrumented ones, so give it a chance to modify the create
    // infos.
    std::vector<safe_VkGraphicsPipelineCreateInfo> gpu_create_infos;
    if (GetEnables(dev_data)->gpu_validation) {
        gpu_create_infos = GpuPreCallRecordCreateGraphicsPipelines(dev_data, pipelineCache, count, pCreateInfos, pAllocator,
                                                                   pPipelines, pipe_state);
        pCreateInfos = reinterpret_cast<VkGraphicsPipelineCreateInfo *>(gpu_create_infos.data());
    }

    lock.unlock();

    auto result =
        dev_data->dispatch_table.CreateGraphicsPipelines(device, pipelineCache, count, pCreateInfos, pAllocator, pPipelines);

    lock.lock();
    PostCallRecordCreateGraphicsPipelines(dev_data, &pipe_state, count, pCreateInfos, pAllocator, pPipelines);

    return result;
}

VKAPI_ATTR VkResult VKAPI_CALL CreatePipelineLayout(VkDevice device, const VkPipelineLayoutCreateInfo *pCreateInfo,
                                                    const VkAllocationCallbacks *pAllocator, VkPipelineLayout *pPipelineLayout) {
    layer_data *dev_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    VkResult result;
    bool skip = PreCallValidateCreatePipelineLayout(dev_data, pCreateInfo);
    if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;

    if (GetEnables(dev_data)->gpu_validation) {
        unique_lock_t lock(global_lock);
        result = GpuOverrideDispatchCreatePipelineLayout(dev_data, pCreateInfo, pAllocator, pPipelineLayout);
    } else {
        result = dev_data->dispatch_table.CreatePipelineLayout(device, pCreateInfo, pAllocator, pPipelineLayout);
    }

    if (VK_SUCCESS == result) {
        PostCallRecordCreatePipelineLayout(dev_data, pCreateInfo, pPipelineLayout);
    }
    return result;
}

VKAPI_ATTR void VKAPI_CALL CmdBindPipeline(VkCommandBuffer commandBuffer, VkPipelineBindPoint pipelineBindPoint,
                                           VkPipeline pipeline) {
    bool skip = false;
    layer_data *dev_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    unique_lock_t lock(global_lock);
    skip |= PreCallValidateCmdBindPipeline(commandBuffer, pipelineBindPoint, pipeline);
    if (!skip) {
        PreCallRecordCmdBindPipeline(commandBuffer, pipelineBindPoint, pipeline);
        lock.unlock();
        dev_data->dispatch_table.CmdBindPipeline(commandBuffer, pipelineBindPoint, pipeline);
    }
}

VKAPI_ATTR VkResult VKAPI_CALL CreateShaderModule(VkDevice device, const VkShaderModuleCreateInfo *pCreateInfo,
                                                  const VkAllocationCallbacks *pAllocator, VkShaderModule *pShaderModule) {
    layer_data *dev_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    bool is_spirv;
    bool spirv_valid;
    VkResult result;
    uint32_t unique_shader_id = 0;

    if (PreCallValidateCreateShaderModule(dev_data, pCreateInfo, &is_spirv, &spirv_valid)) return VK_ERROR_VALIDATION_FAILED_EXT;

    if (GetEnables(dev_data)->gpu_validation) {
        result = GpuOverrideDispatchCreateShaderModule(dev_data, pCreateInfo, pAllocator, pShaderModule, &unique_shader_id);
    } else {
        result = dev_data->dispatch_table.CreateShaderModule(device, pCreateInfo, pAllocator, pShaderModule);
    }

    if (result == VK_SUCCESS) {
        lock_guard_t lock(global_lock);
        PostCallRecordCreateShaderModule(dev_data, is_spirv, pCreateInfo, pShaderModule, unique_shader_id);
    }
    return result;
}

VKAPI_ATTR VkDeviceAddress VKAPI_CALL GetBufferDeviceAddressEXT(VkDevice device, const VkBufferDeviceAddressInfoEXT *pInfo) {
    layer_data *dev_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    unique_lock_t lock(global_lock);
    bool skip = PreCallValidateGetBufferDeviceAddressEXT(dev_data, pInfo);
    if (!skip) {
        lock.unlock();
        return dev_data->dispatch_table.GetBufferDeviceAddressEXT(device, pInfo);
    }
    return 0;
}

VKAPI_ATTR VkResult VKAPI_CALL QueueSubmit(VkQueue queue, uint32_t submitCount, const VkSubmitInfo *pSubmits, VkFence fence) {
    layer_data *dev_data = GetLayerDataPtr(get_dispatch_key(queue), layer_data_map);
    unique_lock_t lock(global_lock);

    bool skip = PreCallValidateQueueSubmit(queue, submitCount, pSubmits, fence);
    lock.unlock();

    if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;

    VkResult result = dev_data->dispatch_table.QueueSubmit(queue, submitCount, pSubmits, fence);

    lock.lock();
    PostCallRecordQueueSubmit(queue, submitCount, pSubmits, fence, result);
    lock.unlock();
    return result;
}

// Map of all APIs to be intercepted by this layer
static const std::unordered_map<std::string, void *> name_to_funcptr_map = {
    {"vkGetInstanceProcAddr", (void *)GetInstanceProcAddr},
    {"vk_layerGetPhysicalDeviceProcAddr", (void *)GetPhysicalDeviceProcAddr},
    {"vkGetDeviceProcAddr", (void *)GetDeviceProcAddr},
    {"vkCreateInstance", (void *)CreateInstance},
    {"vkCreateDevice", (void *)CreateDevice},
    {"vkEnumeratePhysicalDevices", (void *)EnumeratePhysicalDevices},
    {"vkGetPhysicalDeviceProperties", (void *)GetPhysicalDeviceProperties},
    {"vkGetPhysicalDeviceQueueFamilyProperties", (void *)GetPhysicalDeviceQueueFamilyProperties},
    {"vkDestroyInstance", (void *)DestroyInstance},
    {"vkEnumerateInstanceLayerProperties", (void *)EnumerateInstanceLayerProperties},
    {"vkEnumerateDeviceLayerProperties", (void *)EnumerateDeviceLayerProperties},
    {"vkEnumerateInstanceExtensionProperties", (void *)EnumerateInstanceExtensionProperties},
    {"vkEnumerateDeviceExtensionProperties", (void *)EnumerateDeviceExtensionProperties},
    {"vkCreateDescriptorUpdateTemplate", (void *)CreateDescriptorUpdateTemplate},
    {"vkCreateDescriptorUpdateTemplateKHR", (void *)CreateDescriptorUpdateTemplateKHR},
    {"vkDestroyDescriptorUpdateTemplate", (void *)DestroyDescriptorUpdateTemplate},
    {"vkDestroyDescriptorUpdateTemplateKHR", (void *)DestroyDescriptorUpdateTemplateKHR},
    {"vkUpdateDescriptorSetWithTemplate", (void *)UpdateDescriptorSetWithTemplate},
    {"vkUpdateDescriptorSetWithTemplateKHR", (void *)UpdateDescriptorSetWithTemplateKHR},
    {"vkCmdPushDescriptorSetWithTemplateKHR", (void *)CmdPushDescriptorSetWithTemplateKHR},
    {"vkCmdPushDescriptorSetKHR", (void *)CmdPushDescriptorSetKHR},
    {"vkCreateSwapchainKHR", (void *)CreateSwapchainKHR},
    {"vkDestroySwapchainKHR", (void *)DestroySwapchainKHR},
    {"vkGetSwapchainImagesKHR", (void *)GetSwapchainImagesKHR},
    {"vkAcquireNextImageKHR", (void *)AcquireNextImageKHR},
    {"vkAcquireNextImage2KHR", (void *)AcquireNextImage2KHR},
    {"vkQueuePresentKHR", (void *)QueuePresentKHR},
    {"vkQueueSubmit", (void *)QueueSubmit},
    {"vkWaitForFences", (void *)WaitForFences},
    {"vkGetFenceStatus", (void *)GetFenceStatus},
    {"vkQueueWaitIdle", (void *)QueueWaitIdle},
    {"vkDeviceWaitIdle", (void *)DeviceWaitIdle},
    {"vkGetDeviceQueue", (void *)GetDeviceQueue},
    {"vkGetDeviceQueue2", (void *)GetDeviceQueue2},
    {"vkDestroyDevice", (void *)DestroyDevice},
    {"vkDestroyFence", (void *)DestroyFence},
    {"vkResetFences", (void *)ResetFences},
    {"vkDestroySemaphore", (void *)DestroySemaphore},
    {"vkDestroyEvent", (void *)DestroyEvent},
    {"vkDestroyQueryPool", (void *)DestroyQueryPool},
    {"vkDestroyBuffer", (void *)DestroyBuffer},
    {"vkDestroyBufferView", (void *)DestroyBufferView},
    {"vkDestroyImage", (void *)DestroyImage},
    {"vkDestroyImageView", (void *)DestroyImageView},
    {"vkDestroyShaderModule", (void *)DestroyShaderModule},
    {"vkDestroyPipeline", (void *)DestroyPipeline},
    {"vkDestroyPipelineLayout", (void *)DestroyPipelineLayout},
    {"vkDestroySampler", (void *)DestroySampler},
    {"vkDestroyDescriptorSetLayout", (void *)DestroyDescriptorSetLayout},
    {"vkDestroyDescriptorPool", (void *)DestroyDescriptorPool},
    {"vkDestroyFramebuffer", (void *)DestroyFramebuffer},
    {"vkDestroyRenderPass", (void *)DestroyRenderPass},
    {"vkCreateBuffer", (void *)CreateBuffer},
    {"vkCreateBufferView", (void *)CreateBufferView},
    {"vkCreateImage", (void *)CreateImage},
    {"vkCreateImageView", (void *)CreateImageView},
    {"vkCreateFence", (void *)CreateFence},
    {"vkCreatePipelineCache", (void *)CreatePipelineCache},
    {"vkDestroyPipelineCache", (void *)DestroyPipelineCache},
    {"vkGetPipelineCacheData", (void *)GetPipelineCacheData},
    {"vkMergePipelineCaches", (void *)MergePipelineCaches},
    {"vkCreateGraphicsPipelines", (void *)CreateGraphicsPipelines},
    {"vkCreateComputePipelines", (void *)CreateComputePipelines},
    {"vkCreateSampler", (void *)CreateSampler},
    {"vkCreateDescriptorSetLayout", (void *)CreateDescriptorSetLayout},
    {"vkCreatePipelineLayout", (void *)CreatePipelineLayout},
    {"vkCreateDescriptorPool", (void *)CreateDescriptorPool},
    {"vkResetDescriptorPool", (void *)ResetDescriptorPool},
    {"vkAllocateDescriptorSets", (void *)AllocateDescriptorSets},
    {"vkFreeDescriptorSets", (void *)FreeDescriptorSets},
    {"vkUpdateDescriptorSets", (void *)UpdateDescriptorSets},
    {"vkCreateCommandPool", (void *)CreateCommandPool},
    {"vkDestroyCommandPool", (void *)DestroyCommandPool},
    {"vkResetCommandPool", (void *)ResetCommandPool},
    {"vkCreateQueryPool", (void *)CreateQueryPool},
    {"vkAllocateCommandBuffers", (void *)AllocateCommandBuffers},
    {"vkFreeCommandBuffers", (void *)FreeCommandBuffers},
    {"vkBeginCommandBuffer", (void *)BeginCommandBuffer},
    {"vkEndCommandBuffer", (void *)EndCommandBuffer},
    {"vkResetCommandBuffer", (void *)ResetCommandBuffer},
    {"vkCmdBindPipeline", (void *)CmdBindPipeline},
    {"vkCmdSetViewport", (void *)CmdSetViewport},
    {"vkCmdSetScissor", (void *)CmdSetScissor},
    {"vkCmdSetLineWidth", (void *)CmdSetLineWidth},
    {"vkCmdSetDepthBias", (void *)CmdSetDepthBias},
    {"vkCmdSetBlendConstants", (void *)CmdSetBlendConstants},
    {"vkCmdSetDepthBounds", (void *)CmdSetDepthBounds},
    {"vkCmdSetStencilCompareMask", (void *)CmdSetStencilCompareMask},
    {"vkCmdSetStencilWriteMask", (void *)CmdSetStencilWriteMask},
    {"vkCmdSetStencilReference", (void *)CmdSetStencilReference},
    {"vkCmdBindDescriptorSets", (void *)CmdBindDescriptorSets},
    {"vkCmdBindVertexBuffers", (void *)CmdBindVertexBuffers},
    {"vkCmdBindIndexBuffer", (void *)CmdBindIndexBuffer},
    {"vkCmdDraw", (void *)CmdDraw},
    {"vkCmdDrawIndexed", (void *)CmdDrawIndexed},
    {"vkCmdDrawIndirect", (void *)CmdDrawIndirect},
    {"vkCmdDrawIndexedIndirect", (void *)CmdDrawIndexedIndirect},
    {"vkCmdDispatch", (void *)CmdDispatch},
    {"vkCmdDispatchIndirect", (void *)CmdDispatchIndirect},
    {"vkCmdCopyBuffer", (void *)CmdCopyBuffer},
    {"vkCmdCopyImage", (void *)CmdCopyImage},
    {"vkCmdBlitImage", (void *)CmdBlitImage},
    {"vkCmdCopyBufferToImage", (void *)CmdCopyBufferToImage},
    {"vkCmdCopyImageToBuffer", (void *)CmdCopyImageToBuffer},
    {"vkCmdUpdateBuffer", (void *)CmdUpdateBuffer},
    {"vkCmdFillBuffer", (void *)CmdFillBuffer},
    {"vkCmdClearColorImage", (void *)CmdClearColorImage},
    {"vkCmdClearDepthStencilImage", (void *)CmdClearDepthStencilImage},
    {"vkCmdClearAttachments", (void *)CmdClearAttachments},
    {"vkCmdResolveImage", (void *)CmdResolveImage},
    {"vkGetImageSubresourceLayout", (void *)GetImageSubresourceLayout},
    {"vkCmdSetEvent", (void *)CmdSetEvent},
    {"vkCmdResetEvent", (void *)CmdResetEvent},
    {"vkCmdWaitEvents", (void *)CmdWaitEvents},
    {"vkCmdPipelineBarrier", (void *)CmdPipelineBarrier},
    {"vkCmdBeginQuery", (void *)CmdBeginQuery},
    {"vkCmdEndQuery", (void *)CmdEndQuery},
    {"vkCmdResetQueryPool", (void *)CmdResetQueryPool},
    {"vkCmdCopyQueryPoolResults", (void *)CmdCopyQueryPoolResults},
    {"vkCmdPushConstants", (void *)CmdPushConstants},
    {"vkCmdWriteTimestamp", (void *)CmdWriteTimestamp},
    {"vkCreateFramebuffer", (void *)CreateFramebuffer},
    {"vkCreateShaderModule", (void *)CreateShaderModule},
    {"vkCreateRenderPass", (void *)CreateRenderPass},
    {"vkCmdBeginRenderPass", (void *)CmdBeginRenderPass},
    {"vkCmdNextSubpass", (void *)CmdNextSubpass},
    {"vkCmdEndRenderPass", (void *)CmdEndRenderPass},
    {"vkCmdExecuteCommands", (void *)CmdExecuteCommands},
    {"vkCmdDebugMarkerBeginEXT", (void *)CmdDebugMarkerBeginEXT},
    {"vkCmdDebugMarkerEndEXT", (void *)CmdDebugMarkerEndEXT},
    {"vkCmdDebugMarkerInsertEXT", (void *)CmdDebugMarkerInsertEXT},
    {"vkDebugMarkerSetObjectNameEXT", (void *)DebugMarkerSetObjectNameEXT},
    {"vkDebugMarkerSetObjectTagEXT", (void *)DebugMarkerSetObjectTagEXT},
    {"vkSetEvent", (void *)SetEvent},
    {"vkMapMemory", (void *)MapMemory},
    {"vkUnmapMemory", (void *)UnmapMemory},
    {"vkFlushMappedMemoryRanges", (void *)FlushMappedMemoryRanges},
    {"vkInvalidateMappedMemoryRanges", (void *)InvalidateMappedMemoryRanges},
    {"vkAllocateMemory", (void *)AllocateMemory},
    {"vkFreeMemory", (void *)FreeMemory},
    {"vkBindBufferMemory", (void *)BindBufferMemory},
    {"vkBindBufferMemory2", (void *)BindBufferMemory2},
    {"vkBindBufferMemory2KHR", (void *)BindBufferMemory2KHR},
    {"vkGetBufferMemoryRequirements", (void *)GetBufferMemoryRequirements},
    {"vkGetBufferMemoryRequirements2", (void *)GetBufferMemoryRequirements2},
    {"vkGetBufferMemoryRequirements2KHR", (void *)GetBufferMemoryRequirements2KHR},
    {"vkGetImageMemoryRequirements", (void *)GetImageMemoryRequirements},
    {"vkGetImageMemoryRequirements2", (void *)GetImageMemoryRequirements2},
    {"vkGetImageMemoryRequirements2KHR", (void *)GetImageMemoryRequirements2KHR},
    {"vkGetImageSparseMemoryRequirements", (void *)GetImageSparseMemoryRequirements},
    {"vkGetImageSparseMemoryRequirements2", (void *)GetImageSparseMemoryRequirements2},
    {"vkGetImageSparseMemoryRequirements2KHR", (void *)GetImageSparseMemoryRequirements2KHR},
    {"vkGetPhysicalDeviceImageFormatProperties2", (void *)GetPhysicalDeviceImageFormatProperties2},
    {"vkGetPhysicalDeviceImageFormatProperties2KHR", (void *)GetPhysicalDeviceImageFormatProperties2KHR},
    {"vkGetPhysicalDeviceSparseImageFormatProperties", (void *)GetPhysicalDeviceSparseImageFormatProperties},
    {"vkGetPhysicalDeviceSparseImageFormatProperties2", (void *)GetPhysicalDeviceSparseImageFormatProperties2},
    {"vkGetPhysicalDeviceSparseImageFormatProperties2KHR", (void *)GetPhysicalDeviceSparseImageFormatProperties2KHR},
    {"vkGetQueryPoolResults", (void *)GetQueryPoolResults},
    {"vkBindImageMemory", (void *)BindImageMemory},
    {"vkBindImageMemory2", (void *)BindImageMemory2},
    {"vkBindImageMemory2KHR", (void *)BindImageMemory2KHR},
    {"vkQueueBindSparse", (void *)QueueBindSparse},
    {"vkCreateSemaphore", (void *)CreateSemaphore},
    {"vkCreateEvent", (void *)CreateEvent},
    {"vkCreateSamplerYcbcrConversion", (void *)CreateSamplerYcbcrConversion},
    {"vkCreateSamplerYcbcrConversionKHR", (void *)CreateSamplerYcbcrConversionKHR},
    {"vkDestroySamplerYcbcrConversion", (void *)DestroySamplerYcbcrConversion},
    {"vkDestroySamplerYcbcrConversionKHR", (void *)DestroySamplerYcbcrConversionKHR},
#ifdef VK_USE_PLATFORM_ANDROID_KHR
    {"vkCreateAndroidSurfaceKHR", (void *)CreateAndroidSurfaceKHR},
    {"vkGetAndroidHardwareBufferPropertiesANDROID", (void *)GetAndroidHardwareBufferPropertiesANDROID},
    {"vkGetMemoryAndroidHardwareBufferANDROID", (void *)GetMemoryAndroidHardwareBufferANDROID},
#endif
#ifdef VK_USE_PLATFORM_WAYLAND_KHR
    {"vkCreateWaylandSurfaceKHR", (void *)CreateWaylandSurfaceKHR},
    {"vkGetPhysicalDeviceWaylandPresentationSupportKHR", (void *)GetPhysicalDeviceWaylandPresentationSupportKHR},
#endif
#ifdef VK_USE_PLATFORM_WIN32_KHR
    {"vkCreateWin32SurfaceKHR", (void *)CreateWin32SurfaceKHR},
    {"vkGetPhysicalDeviceWin32PresentationSupportKHR", (void *)GetPhysicalDeviceWin32PresentationSupportKHR},
    {"vkImportSemaphoreWin32HandleKHR", (void *)ImportSemaphoreWin32HandleKHR},
    {"vkGetSemaphoreWin32HandleKHR", (void *)GetSemaphoreWin32HandleKHR},
    {"vkImportFenceWin32HandleKHR", (void *)ImportFenceWin32HandleKHR},
    {"vkGetFenceWin32HandleKHR", (void *)GetFenceWin32HandleKHR},
#endif
#ifdef VK_USE_PLATFORM_XCB_KHR
    {"vkCreateXcbSurfaceKHR", (void *)CreateXcbSurfaceKHR},
    {"vkGetPhysicalDeviceXcbPresentationSupportKHR", (void *)GetPhysicalDeviceXcbPresentationSupportKHR},
#endif
#ifdef VK_USE_PLATFORM_XLIB_KHR
    {"vkCreateXlibSurfaceKHR", (void *)CreateXlibSurfaceKHR},
    {"vkGetPhysicalDeviceXlibPresentationSupportKHR", (void *)GetPhysicalDeviceXlibPresentationSupportKHR},
#endif
#ifdef VK_USE_PLATFORM_IOS_MVK
    {"vkCreateIOSSurfaceMVK", (void *)CreateIOSSurfaceMVK},
#endif
#ifdef VK_USE_PLATFORM_MACOS_MVK
    {"vkCreateMacOSSurfaceMVK", (void *)CreateMacOSSurfaceMVK},
#endif
    {"vkCreateDisplayPlaneSurfaceKHR", (void *)CreateDisplayPlaneSurfaceKHR},
    {"vkDestroySurfaceKHR", (void *)DestroySurfaceKHR},
    {"vkGetPhysicalDeviceSurfaceCapabilitiesKHR", (void *)GetPhysicalDeviceSurfaceCapabilitiesKHR},
    {"vkGetPhysicalDeviceSurfaceCapabilities2KHR", (void *)GetPhysicalDeviceSurfaceCapabilities2KHR},
    {"vkGetPhysicalDeviceSurfaceCapabilities2EXT", (void *)GetPhysicalDeviceSurfaceCapabilities2EXT},
    {"vkGetPhysicalDeviceSurfaceSupportKHR", (void *)GetPhysicalDeviceSurfaceSupportKHR},
    {"vkGetPhysicalDeviceSurfacePresentModesKHR", (void *)GetPhysicalDeviceSurfacePresentModesKHR},
    {"vkGetPhysicalDeviceSurfaceFormatsKHR", (void *)GetPhysicalDeviceSurfaceFormatsKHR},
    {"vkGetPhysicalDeviceSurfaceFormats2KHR", (void *)GetPhysicalDeviceSurfaceFormats2KHR},
    {"vkGetPhysicalDeviceQueueFamilyProperties2", (void *)GetPhysicalDeviceQueueFamilyProperties2},
    {"vkGetPhysicalDeviceQueueFamilyProperties2KHR", (void *)GetPhysicalDeviceQueueFamilyProperties2KHR},
    {"vkEnumeratePhysicalDeviceGroups", (void *)EnumeratePhysicalDeviceGroups},
    {"vkEnumeratePhysicalDeviceGroupsKHR", (void *)EnumeratePhysicalDeviceGroupsKHR},
    {"vkCreateDebugReportCallbackEXT", (void *)CreateDebugReportCallbackEXT},
    {"vkDestroyDebugReportCallbackEXT", (void *)DestroyDebugReportCallbackEXT},
    {"vkDebugReportMessageEXT", (void *)DebugReportMessageEXT},
    {"vkGetPhysicalDeviceDisplayPlanePropertiesKHR", (void *)GetPhysicalDeviceDisplayPlanePropertiesKHR},
    {"vkGetPhysicalDeviceDisplayPlaneProperties2KHR", (void *)GetPhysicalDeviceDisplayPlaneProperties2KHR},
    {"vkGetDisplayPlaneSupportedDisplaysKHR", (void *)GetDisplayPlaneSupportedDisplaysKHR},
    {"vkGetDisplayPlaneCapabilitiesKHR", (void *)GetDisplayPlaneCapabilitiesKHR},
    {"vkGetDisplayPlaneCapabilities2KHR", (void *)GetDisplayPlaneCapabilities2KHR},
    {"vkImportSemaphoreFdKHR", (void *)ImportSemaphoreFdKHR},
    {"vkGetSemaphoreFdKHR", (void *)GetSemaphoreFdKHR},
    {"vkImportFenceFdKHR", (void *)ImportFenceFdKHR},
    {"vkGetFenceFdKHR", (void *)GetFenceFdKHR},
    {"vkCreateValidationCacheEXT", (void *)CreateValidationCacheEXT},
    {"vkDestroyValidationCacheEXT", (void *)DestroyValidationCacheEXT},
    {"vkGetValidationCacheDataEXT", (void *)GetValidationCacheDataEXT},
    {"vkMergeValidationCachesEXT", (void *)MergeValidationCachesEXT},
    {"vkCmdSetDiscardRectangleEXT", (void *)CmdSetDiscardRectangleEXT},
    {"vkCmdSetSampleLocationsEXT", (void *)CmdSetSampleLocationsEXT},
    {"vkSetDebugUtilsObjectNameEXT", (void *)SetDebugUtilsObjectNameEXT},
    {"vkSetDebugUtilsObjectTagEXT", (void *)SetDebugUtilsObjectTagEXT},
    {"vkQueueBeginDebugUtilsLabelEXT", (void *)QueueBeginDebugUtilsLabelEXT},
    {"vkQueueEndDebugUtilsLabelEXT", (void *)QueueEndDebugUtilsLabelEXT},
    {"vkQueueInsertDebugUtilsLabelEXT", (void *)QueueInsertDebugUtilsLabelEXT},
    {"vkCmdBeginDebugUtilsLabelEXT", (void *)CmdBeginDebugUtilsLabelEXT},
    {"vkCmdEndDebugUtilsLabelEXT", (void *)CmdEndDebugUtilsLabelEXT},
    {"vkCmdInsertDebugUtilsLabelEXT", (void *)CmdInsertDebugUtilsLabelEXT},
    {"vkCreateDebugUtilsMessengerEXT", (void *)CreateDebugUtilsMessengerEXT},
    {"vkDestroyDebugUtilsMessengerEXT", (void *)DestroyDebugUtilsMessengerEXT},
    {"vkSubmitDebugUtilsMessageEXT", (void *)SubmitDebugUtilsMessageEXT},
    {"vkCmdDrawIndirectCountKHR", (void *)CmdDrawIndirectCountKHR},
    {"vkCmdDrawIndexedIndirectCountKHR", (void *)CmdDrawIndexedIndirectCountKHR},
    {"vkCmdSetExclusiveScissorNV", (void *)CmdSetExclusiveScissorNV},
    {"vkCmdBindShadingRateImageNV", (void *)CmdBindShadingRateImageNV},
    {"vkCmdSetViewportShadingRatePaletteNV", (void *)CmdSetViewportShadingRatePaletteNV},
    {"vkCmdDrawMeshTasksNV", (void *)CmdDrawMeshTasksNV},
    {"vkCmdDrawMeshTasksIndirectNV", (void *)CmdDrawMeshTasksIndirectNV},
    {"vkCmdDrawMeshTasksIndirectCountNV", (void *)CmdDrawMeshTasksIndirectCountNV},
    {"vkCreateRayTracingPipelinesNV", (void *)CreateRayTracingPipelinesNV},
    {"vkCreateRenderPass2KHR", (void *)CreateRenderPass2KHR},
    {"vkCmdBeginRenderPass2KHR", (void *)CmdBeginRenderPass2KHR},
    {"vkCmdNextSubpass2KHR", (void *)CmdNextSubpass2KHR},
    {"vkCmdEndRenderPass2KHR", (void *)CmdEndRenderPass2KHR},
    {"vkGetBufferDeviceAddressEXT", (void *)GetBufferDeviceAddressEXT},
};

VKAPI_ATTR PFN_vkVoidFunction VKAPI_CALL GetDeviceProcAddr(VkDevice device, const char *funcName) {
    assert(device);
    layer_data *device_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);

    if (!ApiParentExtensionEnabled(funcName, device_data->extensions.device_extension_set)) {
        return nullptr;
    }
    // Is API to be intercepted by this layer?
    const auto &item = name_to_funcptr_map.find(funcName);
    if (item != name_to_funcptr_map.end()) {
        return reinterpret_cast<PFN_vkVoidFunction>(item->second);
    }

    auto &table = device_data->dispatch_table;
    if (!table.GetDeviceProcAddr) return nullptr;
    return table.GetDeviceProcAddr(device, funcName);
}

VKAPI_ATTR PFN_vkVoidFunction VKAPI_CALL GetInstanceProcAddr(VkInstance instance, const char *funcName) {
    instance_layer_data *instance_data;
    // Is API to be intercepted by this layer?
    const auto &item = name_to_funcptr_map.find(funcName);
    if (item != name_to_funcptr_map.end()) {
        return reinterpret_cast<PFN_vkVoidFunction>(item->second);
    }

    instance_data = GetLayerDataPtr(get_dispatch_key(instance), instance_layer_data_map);
    auto &table = instance_data->dispatch_table;
    if (!table.GetInstanceProcAddr) return nullptr;
    return table.GetInstanceProcAddr(instance, funcName);
}

VKAPI_ATTR PFN_vkVoidFunction VKAPI_CALL GetPhysicalDeviceProcAddr(VkInstance instance, const char *funcName) {
    assert(instance);
    instance_layer_data *instance_data = GetLayerDataPtr(get_dispatch_key(instance), instance_layer_data_map);

    auto &table = instance_data->dispatch_table;
    if (!table.GetPhysicalDeviceProcAddr) return nullptr;
    return table.GetPhysicalDeviceProcAddr(instance, funcName);
}

}  // namespace core_validation

// loader-layer interface v0, just wrappers since there is only a layer

VK_LAYER_EXPORT VKAPI_ATTR VkResult VKAPI_CALL vkEnumerateInstanceExtensionProperties(const char *pLayerName, uint32_t *pCount,
                                                                                      VkExtensionProperties *pProperties) {
    return core_validation::EnumerateInstanceExtensionProperties(pLayerName, pCount, pProperties);
}

VK_LAYER_EXPORT VKAPI_ATTR VkResult VKAPI_CALL vkEnumerateInstanceLayerProperties(uint32_t *pCount,
                                                                                  VkLayerProperties *pProperties) {
    return core_validation::EnumerateInstanceLayerProperties(pCount, pProperties);
}

VK_LAYER_EXPORT VKAPI_ATTR VkResult VKAPI_CALL vkEnumerateDeviceLayerProperties(VkPhysicalDevice physicalDevice, uint32_t *pCount,
                                                                                VkLayerProperties *pProperties) {
    // the layer command handles VK_NULL_HANDLE just fine internally
    assert(physicalDevice == VK_NULL_HANDLE);
    return core_validation::EnumerateDeviceLayerProperties(VK_NULL_HANDLE, pCount, pProperties);
}

VK_LAYER_EXPORT VKAPI_ATTR VkResult VKAPI_CALL vkEnumerateDeviceExtensionProperties(VkPhysicalDevice physicalDevice,
                                                                                    const char *pLayerName, uint32_t *pCount,
                                                                                    VkExtensionProperties *pProperties) {
    // the layer command handles VK_NULL_HANDLE just fine internally
    assert(physicalDevice == VK_NULL_HANDLE);
    return core_validation::EnumerateDeviceExtensionProperties(VK_NULL_HANDLE, pLayerName, pCount, pProperties);
}

VK_LAYER_EXPORT VKAPI_ATTR PFN_vkVoidFunction VKAPI_CALL vkGetDeviceProcAddr(VkDevice dev, const char *funcName) {
    return core_validation::GetDeviceProcAddr(dev, funcName);
}

VK_LAYER_EXPORT VKAPI_ATTR PFN_vkVoidFunction VKAPI_CALL vkGetInstanceProcAddr(VkInstance instance, const char *funcName) {
    return core_validation::GetInstanceProcAddr(instance, funcName);
}

VK_LAYER_EXPORT VKAPI_ATTR PFN_vkVoidFunction VKAPI_CALL vk_layerGetPhysicalDeviceProcAddr(VkInstance instance,
                                                                                           const char *funcName) {
    return core_validation::GetPhysicalDeviceProcAddr(instance, funcName);
}

VK_LAYER_EXPORT VKAPI_ATTR VkResult VKAPI_CALL vkNegotiateLoaderLayerInterfaceVersion(VkNegotiateLayerInterface *pVersionStruct) {
    assert(pVersionStruct != NULL);
    assert(pVersionStruct->sType == LAYER_NEGOTIATE_INTERFACE_STRUCT);

    // Fill in the function pointers if our version is at least capable of having the structure contain them.
    if (pVersionStruct->loaderLayerInterfaceVersion >= 2) {
        pVersionStruct->pfnGetInstanceProcAddr = vkGetInstanceProcAddr;
        pVersionStruct->pfnGetDeviceProcAddr = vkGetDeviceProcAddr;
        pVersionStruct->pfnGetPhysicalDeviceProcAddr = vk_layerGetPhysicalDeviceProcAddr;
    }

    if (pVersionStruct->loaderLayerInterfaceVersion < CURRENT_LOADER_LAYER_INTERFACE_VERSION) {
        core_validation::loader_layer_if_version = pVersionStruct->loaderLayerInterfaceVersion;
    } else if (pVersionStruct->loaderLayerInterfaceVersion > CURRENT_LOADER_LAYER_INTERFACE_VERSION) {
        pVersionStruct->loaderLayerInterfaceVersion = CURRENT_LOADER_LAYER_INTERFACE_VERSION;
    }

    return VK_SUCCESS;
}
