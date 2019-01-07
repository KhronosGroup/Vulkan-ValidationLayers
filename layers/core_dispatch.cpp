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
    if (VK_SUCCESS == result) {
        lock_guard_t lock(global_lock);
        PostCallRecordCreateFence(dev_data, pCreateInfo, pFence);
    }
    return result;
}

VKAPI_ATTR VkResult VKAPI_CALL CreateInstance(const VkInstanceCreateInfo *pCreateInfo, const VkAllocationCallbacks *pAllocator,
                                              VkInstance *pInstance) {
    VkLayerInstanceCreateInfo *chain_info = get_chain_info(pCreateInfo, VK_LAYER_LINK_INFO);

    assert(chain_info->u.pLayerInfo);
    PFN_vkGetInstanceProcAddr fpGetInstanceProcAddr = chain_info->u.pLayerInfo->pfnNextGetInstanceProcAddr;
    PFN_vkCreateInstance fpCreateInstance = (PFN_vkCreateInstance)fpGetInstanceProcAddr(NULL, "vkCreateInstance");
    if (fpCreateInstance == NULL) return VK_ERROR_INITIALIZATION_FAILED;

    PreCallRecordCreateInstance(chain_info);

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
    PostCallRecordCreateInstance(instance_data, pCreateInfo);

    return result;
}

VKAPI_ATTR void VKAPI_CALL DestroyInstance(VkInstance instance, const VkAllocationCallbacks *pAllocator) {
    dispatch_key key = get_dispatch_key(instance);
    instance_layer_data *instance_data = GetLayerDataPtr(key, instance_layer_data_map);
    instance_data->dispatch_table.DestroyInstance(instance, pAllocator);
    lock_guard_t lock(global_lock);
    PostCallRecordDestroyInstance(instance_data, pAllocator, key);
}

VKAPI_ATTR VkResult VKAPI_CALL CreateDevice(VkPhysicalDevice gpu, const VkDeviceCreateInfo *pCreateInfo,
                                            const VkAllocationCallbacks *pAllocator, VkDevice *pDevice) {
    instance_layer_data *instance_data = GetLayerDataPtr(get_dispatch_key(gpu), instance_layer_data_map);

    unique_lock_t lock(global_lock);
    const VkPhysicalDeviceFeatures *enabled_features_found = pCreateInfo->pEnabledFeatures;

    bool skip = PreCallValidateCreateDevice(instance_data, &enabled_features_found, gpu, pCreateInfo);
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
    PostCallRecordCreateDevice(instance_data, enabled_features_found, fpGetDeviceProcAddr, gpu, pCreateInfo, pDevice);
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

    auto cb_state = GetCBNode(dev_data, commandBuffer);
    assert(cb_state);
    auto dst_buff_state = GetBufferState(dev_data, dstBuffer);
    assert(dst_buff_state);
    skip |= PreCallCmdUpdateBuffer(dev_data, cb_state, dst_buff_state);
    lock.unlock();
    if (!skip) {
        dev_data->dispatch_table.CmdUpdateBuffer(commandBuffer, dstBuffer, dstOffset, dataSize, pData);
        lock.lock();
        PostCallRecordCmdUpdateBuffer(dev_data, cb_state, dst_buff_state);
        lock.unlock();
    }
}

VKAPI_ATTR void VKAPI_CALL GetDeviceQueue(VkDevice device, uint32_t queueFamilyIndex, uint32_t queueIndex, VkQueue *pQueue) {
    layer_data *dev_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    dev_data->dispatch_table.GetDeviceQueue(device, queueFamilyIndex, queueIndex, pQueue);
    lock_guard_t lock(global_lock);

    PostCallRecordGetDeviceQueue(dev_data, queueFamilyIndex, *pQueue);
}

VKAPI_ATTR void VKAPI_CALL GetDeviceQueue2(VkDevice device, const VkDeviceQueueInfo2 *pQueueInfo, VkQueue *pQueue) {
    layer_data *dev_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    dev_data->dispatch_table.GetDeviceQueue2(device, pQueueInfo, pQueue);
    lock_guard_t lock(global_lock);

    if (*pQueue != VK_NULL_HANDLE) {
        PostCallRecordGetDeviceQueue(dev_data, pQueueInfo->queueFamilyIndex, *pQueue);
    }
}

VKAPI_ATTR VkResult VKAPI_CALL CreateSamplerYcbcrConversion(VkDevice device, const VkSamplerYcbcrConversionCreateInfo *pCreateInfo,
                                                            const VkAllocationCallbacks *pAllocator,
                                                            VkSamplerYcbcrConversion *pYcbcrConversion) {
    layer_data *dev_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    bool skip = PreCallValidateCreateSamplerYcbcrConversion(dev_data, pCreateInfo);
    if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;

    VkResult result = dev_data->dispatch_table.CreateSamplerYcbcrConversion(device, pCreateInfo, pAllocator, pYcbcrConversion);
    unique_lock_t lock(global_lock);
    PostCallRecordCreateSamplerYcbcrConversion(dev_data, pCreateInfo, *pYcbcrConversion);
    lock.unlock();
    return result;
};

VKAPI_ATTR VkResult VKAPI_CALL CreateSamplerYcbcrConversionKHR(VkDevice device,
                                                               const VkSamplerYcbcrConversionCreateInfo *pCreateInfo,
                                                               const VkAllocationCallbacks *pAllocator,
                                                               VkSamplerYcbcrConversion *pYcbcrConversion) {
    layer_data *dev_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    bool skip = PreCallValidateCreateSamplerYcbcrConversion(dev_data, pCreateInfo);
    if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;

    VkResult result = dev_data->dispatch_table.CreateSamplerYcbcrConversionKHR(device, pCreateInfo, pAllocator, pYcbcrConversion);
    unique_lock_t lock(global_lock);
    PostCallRecordCreateSamplerYcbcrConversion(dev_data, pCreateInfo, *pYcbcrConversion);
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
    GLOBAL_CB_NODE *cb_state = GetCBNode(device_data, commandBuffer);
    // Minimal validation for command buffer state
    if (cb_state) {
        skip |= PreCallValidateCmdDebugMarkerBeginEXT(device_data, cb_state);
    }
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
    PreCallRecordDestroyDevice(dev_data, device);
    lock.unlock();

    dev_data->dispatch_table.DestroyDevice(device, pAllocator);

    // Free all the memory
    lock.lock();
    FreeLayerDataPtr(key, layer_data_map);
}

VKAPI_ATTR VkResult VKAPI_CALL AllocateMemory(VkDevice device, const VkMemoryAllocateInfo *pAllocateInfo,
                                              const VkAllocationCallbacks *pAllocator, VkDeviceMemory *pMemory) {
    VkResult result = VK_ERROR_VALIDATION_FAILED_EXT;
    layer_data *dev_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    unique_lock_t lock(global_lock);
    bool skip = PreCallValidateAllocateMemory(dev_data, pAllocateInfo);
    if (!skip) {
        lock.unlock();
        result = dev_data->dispatch_table.AllocateMemory(device, pAllocateInfo, pAllocator, pMemory);
        lock.lock();
        if (VK_SUCCESS == result) {
            PostCallRecordAllocateMemory(dev_data, pAllocateInfo, pMemory);
        }
    }
    return result;
}

VKAPI_ATTR void VKAPI_CALL FreeMemory(VkDevice device, VkDeviceMemory mem, const VkAllocationCallbacks *pAllocator) {
    layer_data *dev_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    DEVICE_MEM_INFO *mem_info = nullptr;
    VK_OBJECT obj_struct;
    unique_lock_t lock(global_lock);
    bool skip = PreCallValidateFreeMemory(dev_data, mem, &mem_info, &obj_struct);
    if (!skip) {
        if (mem != VK_NULL_HANDLE) {
            // Avoid free/alloc race by recording state change before dispatching
            PreCallRecordFreeMemory(dev_data, mem, mem_info, obj_struct);
        }
        lock.unlock();
        dev_data->dispatch_table.FreeMemory(device, mem, pAllocator);
    }
}

VKAPI_ATTR VkResult VKAPI_CALL WaitForFences(VkDevice device, uint32_t fenceCount, const VkFence *pFences, VkBool32 waitAll,
                                             uint64_t timeout) {
    layer_data *dev_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    // Verify fence status of submitted fences
    unique_lock_t lock(global_lock);
    bool skip = PreCallValidateWaitForFences(dev_data, fenceCount, pFences);
    lock.unlock();
    if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;

    VkResult result = dev_data->dispatch_table.WaitForFences(device, fenceCount, pFences, waitAll, timeout);

    if (result == VK_SUCCESS) {
        lock.lock();
        PostCallRecordWaitForFences(dev_data, fenceCount, pFences, waitAll);
        lock.unlock();
    }
    return result;
}

VKAPI_ATTR VkResult VKAPI_CALL GetFenceStatus(VkDevice device, VkFence fence) {
    layer_data *dev_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    unique_lock_t lock(global_lock);
    bool skip = PreCallValidateGetFenceStatus(dev_data, fence);
    lock.unlock();
    if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;

    VkResult result = dev_data->dispatch_table.GetFenceStatus(device, fence);
    if (result == VK_SUCCESS) {
        lock.lock();
        PostCallRecordGetFenceStatus(dev_data, fence);
        lock.unlock();
    }
    return result;
}

VKAPI_ATTR VkResult VKAPI_CALL QueueWaitIdle(VkQueue queue) {
    layer_data *dev_data = GetLayerDataPtr(get_dispatch_key(queue), layer_data_map);
    QUEUE_STATE *queue_state = nullptr;
    unique_lock_t lock(global_lock);
    bool skip = PreCallValidateQueueWaitIdle(dev_data, queue, &queue_state);
    lock.unlock();
    if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
    VkResult result = dev_data->dispatch_table.QueueWaitIdle(queue);
    if (VK_SUCCESS == result) {
        lock.lock();
        PostCallRecordQueueWaitIdle(dev_data, queue_state);
        lock.unlock();
    }
    return result;
}

VKAPI_ATTR VkResult VKAPI_CALL DeviceWaitIdle(VkDevice device) {
    layer_data *dev_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    unique_lock_t lock(global_lock);
    bool skip = PreCallValidateDeviceWaitIdle(dev_data);
    lock.unlock();
    if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
    VkResult result = dev_data->dispatch_table.DeviceWaitIdle(device);
    if (VK_SUCCESS == result) {
        lock.lock();
        PostCallRecordDeviceWaitIdle(dev_data);
        lock.unlock();
    }
    return result;
}

VKAPI_ATTR void VKAPI_CALL DestroyFence(VkDevice device, VkFence fence, const VkAllocationCallbacks *pAllocator) {
    layer_data *dev_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    // Common data objects used pre & post call
    FENCE_NODE *fence_node = nullptr;
    VK_OBJECT obj_struct;
    unique_lock_t lock(global_lock);
    bool skip = PreCallValidateDestroyFence(dev_data, fence, &fence_node, &obj_struct);

    if (!skip) {
        // Pre-record to avoid Destroy/Create race
        PreCallRecordDestroyFence(dev_data, fence);
        lock.unlock();
        dev_data->dispatch_table.DestroyFence(device, fence, pAllocator);
    }
}

VKAPI_ATTR void VKAPI_CALL DestroySemaphore(VkDevice device, VkSemaphore semaphore, const VkAllocationCallbacks *pAllocator) {
    layer_data *dev_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    SEMAPHORE_NODE *sema_node;
    VK_OBJECT obj_struct;
    unique_lock_t lock(global_lock);
    bool skip = PreCallValidateDestroySemaphore(dev_data, semaphore, &sema_node, &obj_struct);
    if (!skip) {
        // Pre-record to avoid Destroy/Create race
        PreCallRecordDestroySemaphore(dev_data, semaphore);
        lock.unlock();
        dev_data->dispatch_table.DestroySemaphore(device, semaphore, pAllocator);
    }
}

VKAPI_ATTR void VKAPI_CALL DestroyEvent(VkDevice device, VkEvent event, const VkAllocationCallbacks *pAllocator) {
    layer_data *dev_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    EVENT_STATE *event_state = nullptr;
    VK_OBJECT obj_struct;
    unique_lock_t lock(global_lock);
    bool skip = PreCallValidateDestroyEvent(dev_data, event, &event_state, &obj_struct);
    if (!skip) {
        if (event != VK_NULL_HANDLE) {
            // Pre-record to avoid Destroy/Create race
            PreCallRecordDestroyEvent(dev_data, event, event_state, obj_struct);
        }
        lock.unlock();
        dev_data->dispatch_table.DestroyEvent(device, event, pAllocator);
    }
}

VKAPI_ATTR void VKAPI_CALL DestroyQueryPool(VkDevice device, VkQueryPool queryPool, const VkAllocationCallbacks *pAllocator) {
    layer_data *dev_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    QUERY_POOL_NODE *qp_state = nullptr;
    VK_OBJECT obj_struct;
    unique_lock_t lock(global_lock);
    bool skip = PreCallValidateDestroyQueryPool(dev_data, queryPool, &qp_state, &obj_struct);
    if (!skip) {
        if (queryPool != VK_NULL_HANDLE) {
            // Pre-record to avoid Destroy/Create race
            PreCallRecordDestroyQueryPool(dev_data, queryPool, qp_state, obj_struct);
        }
        lock.unlock();
        dev_data->dispatch_table.DestroyQueryPool(device, queryPool, pAllocator);
    }
}

VKAPI_ATTR VkResult VKAPI_CALL GetQueryPoolResults(VkDevice device, VkQueryPool queryPool, uint32_t firstQuery, uint32_t queryCount,
                                                   size_t dataSize, void *pData, VkDeviceSize stride, VkQueryResultFlags flags) {
    layer_data *dev_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    unordered_map<QueryObject, std::vector<VkCommandBuffer>> queries_in_flight;
    unique_lock_t lock(global_lock);
    bool skip = PreCallValidateGetQueryPoolResults(dev_data, queryPool, firstQuery, queryCount, flags, &queries_in_flight);
    lock.unlock();
    if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
    VkResult result =
        dev_data->dispatch_table.GetQueryPoolResults(device, queryPool, firstQuery, queryCount, dataSize, pData, stride, flags);
    lock.lock();
    PostCallRecordGetQueryPoolResults(dev_data, queryPool, firstQuery, queryCount, &queries_in_flight);
    lock.unlock();
    return result;
}

VKAPI_ATTR void VKAPI_CALL DestroyBuffer(VkDevice device, VkBuffer buffer, const VkAllocationCallbacks *pAllocator) {
    layer_data *dev_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    BUFFER_STATE *buffer_state = nullptr;
    VK_OBJECT obj_struct;
    unique_lock_t lock(global_lock);
    bool skip = PreCallValidateDestroyBuffer(dev_data, buffer, &buffer_state, &obj_struct);
    if (!skip) {
        if (buffer != VK_NULL_HANDLE) {
            // Pre-record to avoid Destroy/Create race
            PreCallRecordDestroyBuffer(dev_data, buffer, buffer_state, obj_struct);
        }
        lock.unlock();
        dev_data->dispatch_table.DestroyBuffer(device, buffer, pAllocator);
    }
}

VKAPI_ATTR void VKAPI_CALL DestroyBufferView(VkDevice device, VkBufferView bufferView, const VkAllocationCallbacks *pAllocator) {
    layer_data *dev_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    // Common data objects used pre & post call
    BUFFER_VIEW_STATE *buffer_view_state = nullptr;
    VK_OBJECT obj_struct;
    unique_lock_t lock(global_lock);
    // Validate state before calling down chain, update common data if we'll be calling down chain
    bool skip = PreCallValidateDestroyBufferView(dev_data, bufferView, &buffer_view_state, &obj_struct);
    if (!skip) {
        if (bufferView != VK_NULL_HANDLE) {
            // Pre-record to avoid Destroy/Create race
            PreCallRecordDestroyBufferView(dev_data, bufferView, buffer_view_state, obj_struct);
        }
        lock.unlock();
        dev_data->dispatch_table.DestroyBufferView(device, bufferView, pAllocator);
    }
}

VKAPI_ATTR void VKAPI_CALL DestroyImage(VkDevice device, VkImage image, const VkAllocationCallbacks *pAllocator) {
    layer_data *dev_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    IMAGE_STATE *image_state = nullptr;
    VK_OBJECT obj_struct;
    unique_lock_t lock(global_lock);
    bool skip = PreCallValidateDestroyImage(dev_data, image, &image_state, &obj_struct);
    if (!skip) {
        if (image != VK_NULL_HANDLE) {
            // Pre-record to avoid Destroy/Create race
            PreCallRecordDestroyImage(dev_data, image, image_state, obj_struct);
        }
        lock.unlock();
        dev_data->dispatch_table.DestroyImage(device, image, pAllocator);
    }
}

VKAPI_ATTR VkResult VKAPI_CALL BindBufferMemory(VkDevice device, VkBuffer buffer, VkDeviceMemory mem, VkDeviceSize memoryOffset) {
    layer_data *dev_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    VkResult result = VK_ERROR_VALIDATION_FAILED_EXT;
    BUFFER_STATE *buffer_state;
    {
        unique_lock_t lock(global_lock);
        buffer_state = GetBufferState(dev_data, buffer);
    }
    bool skip = PreCallValidateBindBufferMemory(dev_data, buffer, buffer_state, mem, memoryOffset, "vkBindBufferMemory()");
    if (!skip) {
        result = dev_data->dispatch_table.BindBufferMemory(device, buffer, mem, memoryOffset);
        if (result == VK_SUCCESS) {
            PostCallRecordBindBufferMemory(dev_data, buffer, buffer_state, mem, memoryOffset, "vkBindBufferMemory()");
        }
    }
    return result;
}

VKAPI_ATTR VkResult VKAPI_CALL BindBufferMemory2(VkDevice device, uint32_t bindInfoCount,
                                                 const VkBindBufferMemoryInfoKHR *pBindInfos) {
    layer_data *dev_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    VkResult result = VK_ERROR_VALIDATION_FAILED_EXT;
    std::vector<BUFFER_STATE *> buffer_state(bindInfoCount);
    if (!PreCallValidateBindBufferMemory2(dev_data, &buffer_state, bindInfoCount, pBindInfos)) {
        result = dev_data->dispatch_table.BindBufferMemory2(device, bindInfoCount, pBindInfos);
        if (result == VK_SUCCESS) {
            PostCallRecordBindBufferMemory2(dev_data, buffer_state, bindInfoCount, pBindInfos);
        }
    }
    return result;
}

VKAPI_ATTR VkResult VKAPI_CALL BindBufferMemory2KHR(VkDevice device, uint32_t bindInfoCount,
                                                    const VkBindBufferMemoryInfoKHR *pBindInfos) {
    layer_data *dev_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    VkResult result = VK_ERROR_VALIDATION_FAILED_EXT;
    std::vector<BUFFER_STATE *> buffer_state(bindInfoCount);
    if (!PreCallValidateBindBufferMemory2(dev_data, &buffer_state, bindInfoCount, pBindInfos)) {
        result = dev_data->dispatch_table.BindBufferMemory2KHR(device, bindInfoCount, pBindInfos);
        if (result == VK_SUCCESS) {
            PostCallRecordBindBufferMemory2(dev_data, buffer_state, bindInfoCount, pBindInfos);
        }
    }
    return result;
}

VKAPI_ATTR void VKAPI_CALL GetBufferMemoryRequirements(VkDevice device, VkBuffer buffer,
                                                       VkMemoryRequirements *pMemoryRequirements) {
    layer_data *dev_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    dev_data->dispatch_table.GetBufferMemoryRequirements(device, buffer, pMemoryRequirements);
    PostCallRecordGetBufferMemoryRequirements(dev_data, buffer, pMemoryRequirements);
}

VKAPI_ATTR void VKAPI_CALL GetBufferMemoryRequirements2(VkDevice device, const VkBufferMemoryRequirementsInfo2KHR *pInfo,
                                                        VkMemoryRequirements2KHR *pMemoryRequirements) {
    layer_data *dev_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    dev_data->dispatch_table.GetBufferMemoryRequirements2(device, pInfo, pMemoryRequirements);
    PostCallRecordGetBufferMemoryRequirements(dev_data, pInfo->buffer, &pMemoryRequirements->memoryRequirements);
}

VKAPI_ATTR void VKAPI_CALL GetBufferMemoryRequirements2KHR(VkDevice device, const VkBufferMemoryRequirementsInfo2KHR *pInfo,
                                                           VkMemoryRequirements2KHR *pMemoryRequirements) {
    layer_data *dev_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    dev_data->dispatch_table.GetBufferMemoryRequirements2KHR(device, pInfo, pMemoryRequirements);
    PostCallRecordGetBufferMemoryRequirements(dev_data, pInfo->buffer, &pMemoryRequirements->memoryRequirements);
}

VKAPI_ATTR void VKAPI_CALL GetImageMemoryRequirements(VkDevice device, VkImage image, VkMemoryRequirements *pMemoryRequirements) {
    layer_data *dev_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    dev_data->dispatch_table.GetImageMemoryRequirements(device, image, pMemoryRequirements);
    PostCallRecordGetImageMemoryRequirements(dev_data, image, pMemoryRequirements);
}

VKAPI_ATTR void VKAPI_CALL GetImageMemoryRequirements2(VkDevice device, const VkImageMemoryRequirementsInfo2 *pInfo,
                                                       VkMemoryRequirements2 *pMemoryRequirements) {
    layer_data *dev_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    bool skip = PreCallValidateGetImageMemoryRequirements2(dev_data, pInfo);
    if (skip) return;
    dev_data->dispatch_table.GetImageMemoryRequirements2(device, pInfo, pMemoryRequirements);
    PostCallRecordGetImageMemoryRequirements(dev_data, pInfo->image, &pMemoryRequirements->memoryRequirements);
}

VKAPI_ATTR void VKAPI_CALL GetImageMemoryRequirements2KHR(VkDevice device, const VkImageMemoryRequirementsInfo2 *pInfo,
                                                          VkMemoryRequirements2 *pMemoryRequirements) {
    layer_data *dev_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    bool skip = PreCallValidateGetImageMemoryRequirements2(dev_data, pInfo);
    if (skip) return;
    dev_data->dispatch_table.GetImageMemoryRequirements2KHR(device, pInfo, pMemoryRequirements);
    PostCallRecordGetImageMemoryRequirements(dev_data, pInfo->image, &pMemoryRequirements->memoryRequirements);
}

VKAPI_ATTR void VKAPI_CALL GetImageSparseMemoryRequirements(VkDevice device, VkImage image, uint32_t *pSparseMemoryRequirementCount,
                                                            VkSparseImageMemoryRequirements *pSparseMemoryRequirements) {
    // TODO : Implement tracking here, just passthrough initially
    layer_data *dev_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    dev_data->dispatch_table.GetImageSparseMemoryRequirements(device, image, pSparseMemoryRequirementCount,
                                                              pSparseMemoryRequirements);
    unique_lock_t lock(global_lock);
    auto image_state = GetImageState(dev_data, image);
    PostCallRecordGetImageSparseMemoryRequirements(image_state, *pSparseMemoryRequirementCount, pSparseMemoryRequirements);
}

VKAPI_ATTR void VKAPI_CALL GetImageSparseMemoryRequirements2(VkDevice device, const VkImageSparseMemoryRequirementsInfo2KHR *pInfo,
                                                             uint32_t *pSparseMemoryRequirementCount,
                                                             VkSparseImageMemoryRequirements2KHR *pSparseMemoryRequirements) {
    // TODO : Implement tracking here, just passthrough initially
    layer_data *dev_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    dev_data->dispatch_table.GetImageSparseMemoryRequirements2(device, pInfo, pSparseMemoryRequirementCount,
                                                               pSparseMemoryRequirements);
    unique_lock_t lock(global_lock);
    auto image_state = GetImageState(dev_data, pInfo->image);
    PostCallRecordGetImageSparseMemoryRequirements2(image_state, *pSparseMemoryRequirementCount, pSparseMemoryRequirements);
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
    auto image_state = GetImageState(dev_data, pInfo->image);
    PostCallRecordGetImageSparseMemoryRequirements2(image_state, *pSparseMemoryRequirementCount, pSparseMemoryRequirements);
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
    bool skip = PreCallValidateGetPhysicalDeviceImageFormatProperties2(instance_data->report_data, pImageFormatInfo,
                                                                       pImageFormatProperties);
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
    bool skip = PreCallValidateGetPhysicalDeviceImageFormatProperties2(instance_data->report_data, pImageFormatInfo,
                                                                       pImageFormatProperties);
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
    // Common data objects used pre & post call
    IMAGE_VIEW_STATE *image_view_state = nullptr;
    VK_OBJECT obj_struct;
    unique_lock_t lock(global_lock);
    bool skip = PreCallValidateDestroyImageView(dev_data, imageView, &image_view_state, &obj_struct);
    if (!skip) {
        if (imageView != VK_NULL_HANDLE) {
            // Pre-record to avoid Destroy/Create race
            PreCallRecordDestroyImageView(dev_data, imageView, image_view_state, obj_struct);
        }
        lock.unlock();
        dev_data->dispatch_table.DestroyImageView(device, imageView, pAllocator);
    }
}

VKAPI_ATTR void VKAPI_CALL DestroyShaderModule(VkDevice device, VkShaderModule shaderModule,
                                               const VkAllocationCallbacks *pAllocator) {
    layer_data *dev_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);

    unique_lock_t lock(global_lock);
    // Pre-record to avoid Destroy/Create race
    PreCallRecordDestroyShaderModule(dev_data, shaderModule);
    lock.unlock();

    dev_data->dispatch_table.DestroyShaderModule(device, shaderModule, pAllocator);
}

VKAPI_ATTR void VKAPI_CALL DestroyPipeline(VkDevice device, VkPipeline pipeline, const VkAllocationCallbacks *pAllocator) {
    layer_data *dev_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    PIPELINE_STATE *pipeline_state = nullptr;
    VK_OBJECT obj_struct;
    unique_lock_t lock(global_lock);
    bool skip = PreCallValidateDestroyPipeline(dev_data, pipeline, &pipeline_state, &obj_struct);
    if (!skip) {
        if (pipeline != VK_NULL_HANDLE) {
            // Pre-record to avoid Destroy/Create race
            PreCallRecordDestroyPipeline(dev_data, pipeline, pipeline_state, obj_struct);
        }
        lock.unlock();
        dev_data->dispatch_table.DestroyPipeline(device, pipeline, pAllocator);
    }
}

VKAPI_ATTR void VKAPI_CALL DestroyPipelineLayout(VkDevice device, VkPipelineLayout pipelineLayout,
                                                 const VkAllocationCallbacks *pAllocator) {
    layer_data *dev_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    unique_lock_t lock(global_lock);
    // Pre-record to avoid Destroy/Create race
    PreCallRecordDestroyPipelineLayout(dev_data, pipelineLayout);
    lock.unlock();

    dev_data->dispatch_table.DestroyPipelineLayout(device, pipelineLayout, pAllocator);
}

VKAPI_ATTR void VKAPI_CALL DestroySampler(VkDevice device, VkSampler sampler, const VkAllocationCallbacks *pAllocator) {
    layer_data *dev_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    SAMPLER_STATE *sampler_state = nullptr;
    VK_OBJECT obj_struct;
    unique_lock_t lock(global_lock);
    bool skip = PreCallValidateDestroySampler(dev_data, sampler, &sampler_state, &obj_struct);
    if (!skip) {
        if (sampler != VK_NULL_HANDLE) {
            // Pre-record to avoid Destroy/Create race
            PreCallRecordDestroySampler(dev_data, sampler, sampler_state, obj_struct);
        }
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
        PreCallRecordDestroyDescriptorSetLayout(dev_data, descriptorSetLayout);
    }
    dev_data->dispatch_table.DestroyDescriptorSetLayout(device, descriptorSetLayout, pAllocator);
}

VKAPI_ATTR void VKAPI_CALL DestroyDescriptorPool(VkDevice device, VkDescriptorPool descriptorPool,
                                                 const VkAllocationCallbacks *pAllocator) {
    layer_data *dev_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    DESCRIPTOR_POOL_STATE *desc_pool_state = nullptr;
    VK_OBJECT obj_struct;
    unique_lock_t lock(global_lock);
    bool skip = PreCallValidateDestroyDescriptorPool(dev_data, descriptorPool, &desc_pool_state, &obj_struct);
    if (!skip) {
        // Pre-record to avoid Destroy/Create race
        PreCallRecordDestroyDescriptorPool(dev_data, descriptorPool, desc_pool_state, obj_struct);
        lock.unlock();
        dev_data->dispatch_table.DestroyDescriptorPool(device, descriptorPool, pAllocator);
    }
}

VKAPI_ATTR void VKAPI_CALL FreeCommandBuffers(VkDevice device, VkCommandPool commandPool, uint32_t commandBufferCount,
                                              const VkCommandBuffer *pCommandBuffers) {
    layer_data *dev_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    unique_lock_t lock(global_lock);

    bool skip = PreCallValidateFreeCommandBuffers(dev_data, commandBufferCount, pCommandBuffers);
    if (skip) return;
    PreCallRecordFreeCommandBuffers(dev_data, commandPool, commandBufferCount, pCommandBuffers);
    lock.unlock();

    dev_data->dispatch_table.FreeCommandBuffers(device, commandPool, commandBufferCount, pCommandBuffers);
}

VKAPI_ATTR VkResult VKAPI_CALL CreateCommandPool(VkDevice device, const VkCommandPoolCreateInfo *pCreateInfo,
                                                 const VkAllocationCallbacks *pAllocator, VkCommandPool *pCommandPool) {
    layer_data *dev_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);

    VkResult result = dev_data->dispatch_table.CreateCommandPool(device, pCreateInfo, pAllocator, pCommandPool);

    if (VK_SUCCESS == result) {
        lock_guard_t lock(global_lock);
        PostCallRecordCreateCommandPool(dev_data, pCreateInfo, pCommandPool);
    }
    return result;
}

VKAPI_ATTR VkResult VKAPI_CALL CreateQueryPool(VkDevice device, const VkQueryPoolCreateInfo *pCreateInfo,
                                               const VkAllocationCallbacks *pAllocator, VkQueryPool *pQueryPool) {
    layer_data *dev_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    unique_lock_t lock(global_lock);
    bool skip = PreCallValidateCreateQueryPool(dev_data, pCreateInfo);
    lock.unlock();

    VkResult result = VK_ERROR_VALIDATION_FAILED_EXT;
    if (!skip) {
        result = dev_data->dispatch_table.CreateQueryPool(device, pCreateInfo, pAllocator, pQueryPool);
    }
    if (result == VK_SUCCESS) {
        lock.lock();
        PostCallRecordCreateQueryPool(dev_data, pCreateInfo, pQueryPool);
    }
    return result;
}

VKAPI_ATTR void VKAPI_CALL DestroyCommandPool(VkDevice device, VkCommandPool commandPool, const VkAllocationCallbacks *pAllocator) {
    layer_data *dev_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    unique_lock_t lock(global_lock);
    bool skip = PreCallValidateDestroyCommandPool(dev_data, commandPool);
    if (!skip) {
        // Pre-record to avoid Destroy/Create race
        PreCallRecordDestroyCommandPool(dev_data, commandPool);
        lock.unlock();
        dev_data->dispatch_table.DestroyCommandPool(device, commandPool, pAllocator);
    }
}

VKAPI_ATTR VkResult VKAPI_CALL ResetCommandPool(VkDevice device, VkCommandPool commandPool, VkCommandPoolResetFlags flags) {
    layer_data *dev_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    unique_lock_t lock(global_lock);
    auto pPool = GetCommandPoolNode(dev_data, commandPool);
    bool skip = PreCallValidateResetCommandPool(dev_data, pPool);
    lock.unlock();

    if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;

    VkResult result = dev_data->dispatch_table.ResetCommandPool(device, commandPool, flags);

    // Reset all of the CBs allocated from this pool
    if (VK_SUCCESS == result) {
        lock.lock();
        PostCallRecordResetCommandPool(dev_data, pPool);
        lock.unlock();
    }
    return result;
}

VKAPI_ATTR VkResult VKAPI_CALL ResetFences(VkDevice device, uint32_t fenceCount, const VkFence *pFences) {
    layer_data *dev_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    unique_lock_t lock(global_lock);
    bool skip = PreCallValidateResetFences(dev_data, fenceCount, pFences);
    lock.unlock();

    if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;

    VkResult result = dev_data->dispatch_table.ResetFences(device, fenceCount, pFences);

    if (result == VK_SUCCESS) {
        lock.lock();
        PostCallRecordResetFences(dev_data, fenceCount, pFences);
        lock.unlock();
    }

    return result;
}

VKAPI_ATTR void VKAPI_CALL DestroyFramebuffer(VkDevice device, VkFramebuffer framebuffer, const VkAllocationCallbacks *pAllocator) {
    layer_data *dev_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    FRAMEBUFFER_STATE *framebuffer_state = nullptr;
    VK_OBJECT obj_struct;
    unique_lock_t lock(global_lock);
    bool skip = PreCallValidateDestroyFramebuffer(dev_data, framebuffer, &framebuffer_state, &obj_struct);
    if (!skip) {
        if (framebuffer != VK_NULL_HANDLE) {
            // Pre-record to avoid Destroy/Create race
            PreCallRecordDestroyFramebuffer(dev_data, framebuffer, framebuffer_state, obj_struct);
        }
        lock.unlock();
        dev_data->dispatch_table.DestroyFramebuffer(device, framebuffer, pAllocator);
    }
}

VKAPI_ATTR void VKAPI_CALL DestroyRenderPass(VkDevice device, VkRenderPass renderPass, const VkAllocationCallbacks *pAllocator) {
    layer_data *dev_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    RENDER_PASS_STATE *rp_state = nullptr;
    VK_OBJECT obj_struct;
    unique_lock_t lock(global_lock);
    bool skip = PreCallValidateDestroyRenderPass(dev_data, renderPass, &rp_state, &obj_struct);
    if (!skip) {
        if (renderPass != VK_NULL_HANDLE) {
            // Pre-record to avoid Destroy/Create race
            PreCallRecordDestroyRenderPass(dev_data, renderPass, rp_state, obj_struct);
        }
        lock.unlock();
        dev_data->dispatch_table.DestroyRenderPass(device, renderPass, pAllocator);
    }
}

VKAPI_ATTR VkResult VKAPI_CALL CreateBuffer(VkDevice device, const VkBufferCreateInfo *pCreateInfo,
                                            const VkAllocationCallbacks *pAllocator, VkBuffer *pBuffer) {
    layer_data *dev_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    unique_lock_t lock(global_lock);
    bool skip = PreCallValidateCreateBuffer(dev_data, pCreateInfo);
    lock.unlock();

    if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
    VkResult result = dev_data->dispatch_table.CreateBuffer(device, pCreateInfo, pAllocator, pBuffer);

    if (VK_SUCCESS == result) {
        lock.lock();
        PostCallRecordCreateBuffer(dev_data, pCreateInfo, pBuffer);
        lock.unlock();
    }
    return result;
}

VKAPI_ATTR VkResult VKAPI_CALL CreateBufferView(VkDevice device, const VkBufferViewCreateInfo *pCreateInfo,
                                                const VkAllocationCallbacks *pAllocator, VkBufferView *pView) {
    layer_data *dev_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    unique_lock_t lock(global_lock);
    bool skip = PreCallValidateCreateBufferView(dev_data, pCreateInfo);
    lock.unlock();
    if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
    VkResult result = dev_data->dispatch_table.CreateBufferView(device, pCreateInfo, pAllocator, pView);
    if (VK_SUCCESS == result) {
        lock.lock();
        PostCallRecordCreateBufferView(dev_data, pCreateInfo, pView);
        lock.unlock();
    }
    return result;
}

VKAPI_ATTR VkResult VKAPI_CALL CreateImage(VkDevice device, const VkImageCreateInfo *pCreateInfo,
                                           const VkAllocationCallbacks *pAllocator, VkImage *pImage) {
    VkResult result = VK_ERROR_VALIDATION_FAILED_EXT;
    layer_data *dev_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    bool skip = PreCallValidateCreateImage(dev_data, pCreateInfo, pAllocator, pImage);
    if (!skip) {
        result = dev_data->dispatch_table.CreateImage(device, pCreateInfo, pAllocator, pImage);
    }
    if (VK_SUCCESS == result) {
        lock_guard_t lock(global_lock);
        PostCallRecordCreateImage(dev_data, pCreateInfo, pImage);
    }
    return result;
}

VKAPI_ATTR VkResult VKAPI_CALL CreateImageView(VkDevice device, const VkImageViewCreateInfo *pCreateInfo,
                                               const VkAllocationCallbacks *pAllocator, VkImageView *pView) {
    layer_data *dev_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    unique_lock_t lock(global_lock);
    bool skip = PreCallValidateCreateImageView(dev_data, pCreateInfo);
    lock.unlock();
    if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
    VkResult result = dev_data->dispatch_table.CreateImageView(device, pCreateInfo, pAllocator, pView);
    if (VK_SUCCESS == result) {
        lock.lock();
        PostCallRecordCreateImageView(dev_data, pCreateInfo, *pView);
        lock.unlock();
    }

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
    bool skip = PreCallValidateGetAndroidHardwareBufferProperties(dev_data, buffer);
    if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;

    VkResult res = dev_data->dispatch_table.GetAndroidHardwareBufferPropertiesANDROID(device, buffer, pProperties);
    if (VK_SUCCESS == res) {
        PostCallRecordGetAndroidHardwareBufferProperties(dev_data, pProperties);
    }
    return res;
}

VKAPI_ATTR VkResult VKAPI_CALL GetMemoryAndroidHardwareBufferANDROID(VkDevice device,
                                                                     const VkMemoryGetAndroidHardwareBufferInfoANDROID *pInfo,
                                                                     struct AHardwareBuffer **pBuffer) {
    layer_data *dev_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    bool skip = PreCallValidateGetMemoryAndroidHardwareBuffer(dev_data, pInfo);
    if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;

    return dev_data->dispatch_table.GetMemoryAndroidHardwareBufferANDROID(device, pInfo, pBuffer);
}
#endif  // VK_USE_PLATFORM_ANDROID_KHR

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
