
// This file is ***GENERATED***.  Do Not Edit.
// See uber_layer_generator.py for modifications.

/* Copyright (c) 2015-2018 The Khronos Group Inc.
 * Copyright (c) 2015-2018 Valve Corporation
 * Copyright (c) 2015-2018 LunarG, Inc.
 * Copyright (c) 2015-2018 Google Inc.
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

#include <string.h>
#include <mutex>

#define VALIDATION_ERROR_MAP_IMPL

#include "uber_layer.h"

class uber_layer;

std::vector<uber_layer *> global_interceptor_list;
debug_report_data *report_data = VK_NULL_HANDLE;

std::unordered_map<void *, layer_data *> layer_data_map;
std::unordered_map<void *, instance_layer_data *> instance_layer_data_map;

// Create an object_lifetime object and add it to the global_interceptor_list
#include "interceptor_objects.h"
ObjectLifetimes object_tracker_object;

using mutex_t = std::mutex;
using lock_guard_t = std::lock_guard<mutex_t>;
using unique_lock_t = std::unique_lock<mutex_t>;

namespace vulkan_uber_layer {

using std::unordered_map;

static mutex_t global_lock;

static const VkLayerProperties global_layer = {
    "VK_LAYER_KHRONOS_uber_layer", VK_LAYER_API_VERSION, 1, "LunarG Ultra Layer",
};

static const VkExtensionProperties instance_extensions[] = {{VK_EXT_DEBUG_REPORT_EXTENSION_NAME, VK_EXT_DEBUG_REPORT_SPEC_VERSION}};

extern const std::unordered_map<std::string, void*> name_to_funcptr_map;


// Manually written functions

VKAPI_ATTR PFN_vkVoidFunction VKAPI_CALL GetDeviceProcAddr(VkDevice device, const char *funcName) {
    assert(device);
    layer_data *device_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
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
    instance_layer_data *instance_data = GetLayerDataPtr(get_dispatch_key(instance), instance_layer_data_map);
    auto &table = instance_data->dispatch_table;
    if (!table.GetPhysicalDeviceProcAddr) return nullptr;
    return table.GetPhysicalDeviceProcAddr(instance, funcName);
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
    if (pLayerName && !strcmp(pLayerName, global_layer.layerName)) return util_GetExtensionProperties(0, NULL, pCount, pProperties);

    assert(physicalDevice);

    instance_layer_data *instance_data = GetLayerDataPtr(get_dispatch_key(physicalDevice), instance_layer_data_map);
    return instance_data->dispatch_table.EnumerateDeviceExtensionProperties(physicalDevice, NULL, pCount, pProperties);
}

VKAPI_ATTR VkResult VKAPI_CALL CreateInstance(const VkInstanceCreateInfo *pCreateInfo, const VkAllocationCallbacks *pAllocator,
                                              VkInstance *pInstance) {
    VkLayerInstanceCreateInfo *chain_info = get_chain_info(pCreateInfo, VK_LAYER_LINK_INFO);

    assert(chain_info->u.pLayerInfo);
    PFN_vkGetInstanceProcAddr fpGetInstanceProcAddr = chain_info->u.pLayerInfo->pfnNextGetInstanceProcAddr;
    PFN_vkCreateInstance fpCreateInstance = (PFN_vkCreateInstance)fpGetInstanceProcAddr(NULL, "vkCreateInstance");
    if (fpCreateInstance == NULL) return VK_ERROR_INITIALIZATION_FAILED;
    chain_info->u.pLayerInfo = chain_info->u.pLayerInfo->pNext;

    // Init dispatch array and call registration functions
    for (auto intercept : global_interceptor_list) {
        intercept->PreCallValidateCreateInstance(pCreateInfo, pAllocator, pInstance);
    }
    for (auto intercept : global_interceptor_list) {
        intercept->PreCallRecordCreateInstance(pCreateInfo, pAllocator, pInstance);
    }

    VkResult result = fpCreateInstance(pCreateInfo, pAllocator, pInstance);

    instance_layer_data *instance_data = GetLayerDataPtr(get_dispatch_key(*pInstance), instance_layer_data_map);
    instance_data->instance = *pInstance;
    layer_init_instance_dispatch_table(*pInstance, &instance_data->dispatch_table, fpGetInstanceProcAddr);
    instance_data->report_data = debug_utils_create_instance(
        &instance_data->dispatch_table, *pInstance, pCreateInfo->enabledExtensionCount, pCreateInfo->ppEnabledExtensionNames);
    instance_data->extensions.InitFromInstanceCreateInfo((pCreateInfo->pApplicationInfo ? pCreateInfo->pApplicationInfo->apiVersion : VK_API_VERSION_1_0), pCreateInfo);
    layer_debug_report_actions(instance_data->report_data, instance_data->logging_callback, pAllocator, "lunarg_object_tracker");
    layer_debug_messenger_actions(instance_data->report_data, instance_data->logging_messenger, pAllocator, "lunarg_object_tracker");
    report_data = instance_data->report_data;

    for (auto intercept : global_interceptor_list) {
        intercept->PostCallRecordCreateInstance(pCreateInfo, pAllocator, pInstance);
    }

    return result;
}

VKAPI_ATTR void VKAPI_CALL DestroyInstance(VkInstance instance, const VkAllocationCallbacks *pAllocator) {
    dispatch_key key = get_dispatch_key(instance);
    instance_layer_data *instance_data = GetLayerDataPtr(key, instance_layer_data_map);
    for (auto intercept : global_interceptor_list) {
        intercept->PreCallValidateDestroyInstance(instance, pAllocator);
    }
    for (auto intercept : global_interceptor_list) {
        intercept->PreCallRecordDestroyInstance(instance, pAllocator);
    }

    instance_data->dispatch_table.DestroyInstance(instance, pAllocator);

    lock_guard_t lock(global_lock);
    for (auto intercept : global_interceptor_list) {
        intercept->PostCallRecordDestroyInstance(instance, pAllocator);
    }
    // Clean up logging callback, if any
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
    for (auto intercept : global_interceptor_list) {
        intercept->PostCallRecordDestroyInstance(instance, pAllocator);
    }
    layer_debug_utils_destroy_instance(instance_data->report_data);
    FreeLayerDataPtr(key, instance_layer_data_map);
}

VKAPI_ATTR VkResult VKAPI_CALL CreateDevice(VkPhysicalDevice gpu, const VkDeviceCreateInfo *pCreateInfo,
                                            const VkAllocationCallbacks *pAllocator, VkDevice *pDevice) {
    instance_layer_data *instance_data = GetLayerDataPtr(get_dispatch_key(gpu), instance_layer_data_map);

    unique_lock_t lock(global_lock);
    VkLayerDeviceCreateInfo *chain_info = get_chain_info(pCreateInfo, VK_LAYER_LINK_INFO);
    PFN_vkGetInstanceProcAddr fpGetInstanceProcAddr = chain_info->u.pLayerInfo->pfnNextGetInstanceProcAddr;
    PFN_vkGetDeviceProcAddr fpGetDeviceProcAddr = chain_info->u.pLayerInfo->pfnNextGetDeviceProcAddr;
    PFN_vkCreateDevice fpCreateDevice = (PFN_vkCreateDevice)fpGetInstanceProcAddr(instance_data->instance, "vkCreateDevice");
    chain_info->u.pLayerInfo = chain_info->u.pLayerInfo->pNext;

    for (auto intercept : global_interceptor_list) {
        intercept->PreCallValidateCreateDevice(gpu, pCreateInfo, pAllocator, pDevice);
    }
    for (auto intercept : global_interceptor_list) {
        intercept->PreCallRecordCreateDevice(gpu, pCreateInfo, pAllocator, pDevice);
    }
    lock.unlock();

    VkResult result = fpCreateDevice(gpu, pCreateInfo, pAllocator, pDevice);

    lock.lock();
    for (auto intercept : global_interceptor_list) {
        intercept->PostCallRecordCreateDevice(gpu, pCreateInfo, pAllocator, pDevice);
    }
    layer_data *device_data = GetLayerDataPtr(get_dispatch_key(*pDevice), layer_data_map);
    device_data->instance_data = instance_data;
    layer_init_device_dispatch_table(*pDevice, &device_data->dispatch_table, fpGetDeviceProcAddr);
    device_data->device = *pDevice;
    device_data->physical_device = gpu;
    device_data->report_data = layer_debug_utils_create_device(instance_data->report_data, *pDevice);
    VkPhysicalDeviceProperties physical_device_properties{};
    instance_data->dispatch_table.GetPhysicalDeviceProperties(gpu, &physical_device_properties);
    device_data->extensions.InitFromDeviceCreateInfo(&instance_data->extensions, physical_device_properties.apiVersion, pCreateInfo);
    lock.unlock();

    return result;
}

VKAPI_ATTR void VKAPI_CALL DestroyDevice(VkDevice device, const VkAllocationCallbacks *pAllocator) {
    dispatch_key key = get_dispatch_key(device);
    layer_data *device_data = GetLayerDataPtr(key, layer_data_map);

    unique_lock_t lock(global_lock);
    for (auto intercept : global_interceptor_list) {
        intercept->PreCallValidateDestroyDevice(device, pAllocator);
    }
    for (auto intercept : global_interceptor_list) {
        intercept->PreCallRecordDestroyDevice(device, pAllocator);
    }
    layer_debug_utils_destroy_device(device);
    lock.unlock();

    device_data->dispatch_table.DestroyDevice(device, pAllocator);

    lock.lock();
    for (auto intercept : global_interceptor_list) {
        intercept->PostCallRecordDestroyDevice(device, pAllocator);
    }

    FreeLayerDataPtr(key, layer_data_map);
}

VKAPI_ATTR VkResult VKAPI_CALL CreateDebugReportCallbackEXT(VkInstance instance,
                                                            const VkDebugReportCallbackCreateInfoEXT *pCreateInfo,
                                                            const VkAllocationCallbacks *pAllocator,
                                                            VkDebugReportCallbackEXT *pCallback) {
    instance_layer_data *instance_data = GetLayerDataPtr(get_dispatch_key(instance), instance_layer_data_map);
    for (auto intercept : global_interceptor_list) {
        intercept->PreCallValidateCreateDebugReportCallbackEXT(instance, pCreateInfo, pAllocator, pCallback);
    }
    for (auto intercept : global_interceptor_list) {
        intercept->PreCallRecordCreateDebugReportCallbackEXT(instance, pCreateInfo, pAllocator, pCallback);
    }
    VkResult result = instance_data->dispatch_table.CreateDebugReportCallbackEXT(instance, pCreateInfo, pAllocator, pCallback);
    result = layer_create_report_callback(instance_data->report_data, false, pCreateInfo, pAllocator, pCallback);
    for (auto intercept : global_interceptor_list) {
        intercept->PostCallRecordCreateDebugReportCallbackEXT(instance, pCreateInfo, pAllocator, pCallback);
    }
    return result;
}

VKAPI_ATTR void VKAPI_CALL DestroyDebugReportCallbackEXT(VkInstance instance, VkDebugReportCallbackEXT callback,
                                                         const VkAllocationCallbacks *pAllocator) {
    instance_layer_data *instance_data = GetLayerDataPtr(get_dispatch_key(instance), instance_layer_data_map);
    for (auto intercept : global_interceptor_list) {
        intercept->PreCallValidateDestroyDebugReportCallbackEXT(instance, callback, pAllocator);
    }
    for (auto intercept : global_interceptor_list) {
        intercept->PreCallRecordDestroyDebugReportCallbackEXT(instance, callback, pAllocator);
    }
    instance_data->dispatch_table.DestroyDebugReportCallbackEXT(instance, callback, pAllocator);
    layer_destroy_report_callback(instance_data->report_data, callback, pAllocator);
    for (auto intercept : global_interceptor_list) {
        intercept->PostCallRecordDestroyDebugReportCallbackEXT(instance, callback, pAllocator);
    }
}



VKAPI_ATTR VkResult VKAPI_CALL EnumeratePhysicalDevices(
    VkInstance                                  instance,
    uint32_t*                                   pPhysicalDeviceCount,
    VkPhysicalDevice*                           pPhysicalDevices) {
    instance_layer_data *instance_data = GetLayerDataPtr(get_dispatch_key(instance), instance_layer_data_map);
    {
        bool skip = false;
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            skip |= intercept->PreCallValidateEnumeratePhysicalDevices(instance, pPhysicalDeviceCount, pPhysicalDevices);
            if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
        }
        for (auto intercept : global_interceptor_list) {
            intercept->PreCallRecordEnumeratePhysicalDevices(instance, pPhysicalDeviceCount, pPhysicalDevices);
        }
    }
    VkResult result = instance_data->dispatch_table.EnumeratePhysicalDevices(instance, pPhysicalDeviceCount, pPhysicalDevices);
    if (VK_SUCCESS == result) {
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            intercept->PostCallRecordEnumeratePhysicalDevices(instance, pPhysicalDeviceCount, pPhysicalDevices);
        }
    }
    return result;
}

VKAPI_ATTR void VKAPI_CALL GetPhysicalDeviceFeatures(
    VkPhysicalDevice                            physicalDevice,
    VkPhysicalDeviceFeatures*                   pFeatures) {
    instance_layer_data *instance_data = GetLayerDataPtr(get_dispatch_key(physicalDevice), instance_layer_data_map);
    {
        bool skip = false;
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            skip |= intercept->PreCallValidateGetPhysicalDeviceFeatures(physicalDevice, pFeatures);
            if (skip) return;
        }
        for (auto intercept : global_interceptor_list) {
            intercept->PreCallRecordGetPhysicalDeviceFeatures(physicalDevice, pFeatures);
        }
    }
    instance_data->dispatch_table.GetPhysicalDeviceFeatures(physicalDevice, pFeatures);
    {
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            intercept->PostCallRecordGetPhysicalDeviceFeatures(physicalDevice, pFeatures);
        }
    }
}

VKAPI_ATTR void VKAPI_CALL GetPhysicalDeviceFormatProperties(
    VkPhysicalDevice                            physicalDevice,
    VkFormat                                    format,
    VkFormatProperties*                         pFormatProperties) {
    instance_layer_data *instance_data = GetLayerDataPtr(get_dispatch_key(physicalDevice), instance_layer_data_map);
    {
        bool skip = false;
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            skip |= intercept->PreCallValidateGetPhysicalDeviceFormatProperties(physicalDevice, format, pFormatProperties);
            if (skip) return;
        }
        for (auto intercept : global_interceptor_list) {
            intercept->PreCallRecordGetPhysicalDeviceFormatProperties(physicalDevice, format, pFormatProperties);
        }
    }
    instance_data->dispatch_table.GetPhysicalDeviceFormatProperties(physicalDevice, format, pFormatProperties);
    {
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            intercept->PostCallRecordGetPhysicalDeviceFormatProperties(physicalDevice, format, pFormatProperties);
        }
    }
}

VKAPI_ATTR VkResult VKAPI_CALL GetPhysicalDeviceImageFormatProperties(
    VkPhysicalDevice                            physicalDevice,
    VkFormat                                    format,
    VkImageType                                 type,
    VkImageTiling                               tiling,
    VkImageUsageFlags                           usage,
    VkImageCreateFlags                          flags,
    VkImageFormatProperties*                    pImageFormatProperties) {
    instance_layer_data *instance_data = GetLayerDataPtr(get_dispatch_key(physicalDevice), instance_layer_data_map);
    {
        bool skip = false;
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            skip |= intercept->PreCallValidateGetPhysicalDeviceImageFormatProperties(physicalDevice, format, type, tiling, usage, flags, pImageFormatProperties);
            if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
        }
        for (auto intercept : global_interceptor_list) {
            intercept->PreCallRecordGetPhysicalDeviceImageFormatProperties(physicalDevice, format, type, tiling, usage, flags, pImageFormatProperties);
        }
    }
    VkResult result = instance_data->dispatch_table.GetPhysicalDeviceImageFormatProperties(physicalDevice, format, type, tiling, usage, flags, pImageFormatProperties);
    if (VK_SUCCESS == result) {
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            intercept->PostCallRecordGetPhysicalDeviceImageFormatProperties(physicalDevice, format, type, tiling, usage, flags, pImageFormatProperties);
        }
    }
    return result;
}

VKAPI_ATTR void VKAPI_CALL GetPhysicalDeviceProperties(
    VkPhysicalDevice                            physicalDevice,
    VkPhysicalDeviceProperties*                 pProperties) {
    instance_layer_data *instance_data = GetLayerDataPtr(get_dispatch_key(physicalDevice), instance_layer_data_map);
    {
        bool skip = false;
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            skip |= intercept->PreCallValidateGetPhysicalDeviceProperties(physicalDevice, pProperties);
            if (skip) return;
        }
        for (auto intercept : global_interceptor_list) {
            intercept->PreCallRecordGetPhysicalDeviceProperties(physicalDevice, pProperties);
        }
    }
    instance_data->dispatch_table.GetPhysicalDeviceProperties(physicalDevice, pProperties);
    {
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            intercept->PostCallRecordGetPhysicalDeviceProperties(physicalDevice, pProperties);
        }
    }
}

VKAPI_ATTR void VKAPI_CALL GetPhysicalDeviceQueueFamilyProperties(
    VkPhysicalDevice                            physicalDevice,
    uint32_t*                                   pQueueFamilyPropertyCount,
    VkQueueFamilyProperties*                    pQueueFamilyProperties) {
    instance_layer_data *instance_data = GetLayerDataPtr(get_dispatch_key(physicalDevice), instance_layer_data_map);
    {
        bool skip = false;
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            skip |= intercept->PreCallValidateGetPhysicalDeviceQueueFamilyProperties(physicalDevice, pQueueFamilyPropertyCount, pQueueFamilyProperties);
            if (skip) return;
        }
        for (auto intercept : global_interceptor_list) {
            intercept->PreCallRecordGetPhysicalDeviceQueueFamilyProperties(physicalDevice, pQueueFamilyPropertyCount, pQueueFamilyProperties);
        }
    }
    instance_data->dispatch_table.GetPhysicalDeviceQueueFamilyProperties(physicalDevice, pQueueFamilyPropertyCount, pQueueFamilyProperties);
    {
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            intercept->PostCallRecordGetPhysicalDeviceQueueFamilyProperties(physicalDevice, pQueueFamilyPropertyCount, pQueueFamilyProperties);
        }
    }
}

VKAPI_ATTR void VKAPI_CALL GetPhysicalDeviceMemoryProperties(
    VkPhysicalDevice                            physicalDevice,
    VkPhysicalDeviceMemoryProperties*           pMemoryProperties) {
    instance_layer_data *instance_data = GetLayerDataPtr(get_dispatch_key(physicalDevice), instance_layer_data_map);
    {
        bool skip = false;
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            skip |= intercept->PreCallValidateGetPhysicalDeviceMemoryProperties(physicalDevice, pMemoryProperties);
            if (skip) return;
        }
        for (auto intercept : global_interceptor_list) {
            intercept->PreCallRecordGetPhysicalDeviceMemoryProperties(physicalDevice, pMemoryProperties);
        }
    }
    instance_data->dispatch_table.GetPhysicalDeviceMemoryProperties(physicalDevice, pMemoryProperties);
    {
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            intercept->PostCallRecordGetPhysicalDeviceMemoryProperties(physicalDevice, pMemoryProperties);
        }
    }
}

VKAPI_ATTR void VKAPI_CALL GetDeviceQueue(
    VkDevice                                    device,
    uint32_t                                    queueFamilyIndex,
    uint32_t                                    queueIndex,
    VkQueue*                                    pQueue) {
    layer_data *device_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    {
        bool skip = false;
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            skip |= intercept->PreCallValidateGetDeviceQueue(device, queueFamilyIndex, queueIndex, pQueue);
            if (skip) return;
        }
        for (auto intercept : global_interceptor_list) {
            intercept->PreCallRecordGetDeviceQueue(device, queueFamilyIndex, queueIndex, pQueue);
        }
    }
    device_data->dispatch_table.GetDeviceQueue(device, queueFamilyIndex, queueIndex, pQueue);
    {
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            intercept->PostCallRecordGetDeviceQueue(device, queueFamilyIndex, queueIndex, pQueue);
        }
    }
}

VKAPI_ATTR VkResult VKAPI_CALL QueueSubmit(
    VkQueue                                     queue,
    uint32_t                                    submitCount,
    const VkSubmitInfo*                         pSubmits,
    VkFence                                     fence) {
    layer_data *device_data = GetLayerDataPtr(get_dispatch_key(queue), layer_data_map);
    {
        bool skip = false;
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            skip |= intercept->PreCallValidateQueueSubmit(queue, submitCount, pSubmits, fence);
            if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
        }
        for (auto intercept : global_interceptor_list) {
            intercept->PreCallRecordQueueSubmit(queue, submitCount, pSubmits, fence);
        }
    }
    VkResult result = device_data->dispatch_table.QueueSubmit(queue, submitCount, pSubmits, fence);
    if (VK_SUCCESS == result) {
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            intercept->PostCallRecordQueueSubmit(queue, submitCount, pSubmits, fence);
        }
    }
    return result;
}

VKAPI_ATTR VkResult VKAPI_CALL QueueWaitIdle(
    VkQueue                                     queue) {
    layer_data *device_data = GetLayerDataPtr(get_dispatch_key(queue), layer_data_map);
    {
        bool skip = false;
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            skip |= intercept->PreCallValidateQueueWaitIdle(queue);
            if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
        }
        for (auto intercept : global_interceptor_list) {
            intercept->PreCallRecordQueueWaitIdle(queue);
        }
    }
    VkResult result = device_data->dispatch_table.QueueWaitIdle(queue);
    if (VK_SUCCESS == result) {
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            intercept->PostCallRecordQueueWaitIdle(queue);
        }
    }
    return result;
}

VKAPI_ATTR VkResult VKAPI_CALL DeviceWaitIdle(
    VkDevice                                    device) {
    layer_data *device_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    {
        bool skip = false;
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            skip |= intercept->PreCallValidateDeviceWaitIdle(device);
            if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
        }
        for (auto intercept : global_interceptor_list) {
            intercept->PreCallRecordDeviceWaitIdle(device);
        }
    }
    VkResult result = device_data->dispatch_table.DeviceWaitIdle(device);
    if (VK_SUCCESS == result) {
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            intercept->PostCallRecordDeviceWaitIdle(device);
        }
    }
    return result;
}

VKAPI_ATTR VkResult VKAPI_CALL AllocateMemory(
    VkDevice                                    device,
    const VkMemoryAllocateInfo*                 pAllocateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkDeviceMemory*                             pMemory) {
    layer_data *device_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    {
        bool skip = false;
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            skip |= intercept->PreCallValidateAllocateMemory(device, pAllocateInfo, pAllocator, pMemory);
            if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
        }
        for (auto intercept : global_interceptor_list) {
            intercept->PreCallRecordAllocateMemory(device, pAllocateInfo, pAllocator, pMemory);
        }
    }
    VkResult result = device_data->dispatch_table.AllocateMemory(device, pAllocateInfo, pAllocator, pMemory);
    if (VK_SUCCESS == result) {
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            intercept->PostCallRecordAllocateMemory(device, pAllocateInfo, pAllocator, pMemory);
        }
    }
    return result;
}

VKAPI_ATTR void VKAPI_CALL FreeMemory(
    VkDevice                                    device,
    VkDeviceMemory                              memory,
    const VkAllocationCallbacks*                pAllocator) {
    layer_data *device_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    {
        bool skip = false;
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            skip |= intercept->PreCallValidateFreeMemory(device, memory, pAllocator);
            if (skip) return;
        }
        for (auto intercept : global_interceptor_list) {
            intercept->PreCallRecordFreeMemory(device, memory, pAllocator);
        }
    }
    device_data->dispatch_table.FreeMemory(device, memory, pAllocator);
    {
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            intercept->PostCallRecordFreeMemory(device, memory, pAllocator);
        }
    }
}

VKAPI_ATTR VkResult VKAPI_CALL MapMemory(
    VkDevice                                    device,
    VkDeviceMemory                              memory,
    VkDeviceSize                                offset,
    VkDeviceSize                                size,
    VkMemoryMapFlags                            flags,
    void**                                      ppData) {
    layer_data *device_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    {
        bool skip = false;
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            skip |= intercept->PreCallValidateMapMemory(device, memory, offset, size, flags, ppData);
            if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
        }
        for (auto intercept : global_interceptor_list) {
            intercept->PreCallRecordMapMemory(device, memory, offset, size, flags, ppData);
        }
    }
    VkResult result = device_data->dispatch_table.MapMemory(device, memory, offset, size, flags, ppData);
    if (VK_SUCCESS == result) {
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            intercept->PostCallRecordMapMemory(device, memory, offset, size, flags, ppData);
        }
    }
    return result;
}

VKAPI_ATTR void VKAPI_CALL UnmapMemory(
    VkDevice                                    device,
    VkDeviceMemory                              memory) {
    layer_data *device_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    {
        bool skip = false;
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            skip |= intercept->PreCallValidateUnmapMemory(device, memory);
            if (skip) return;
        }
        for (auto intercept : global_interceptor_list) {
            intercept->PreCallRecordUnmapMemory(device, memory);
        }
    }
    device_data->dispatch_table.UnmapMemory(device, memory);
    {
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            intercept->PostCallRecordUnmapMemory(device, memory);
        }
    }
}

VKAPI_ATTR VkResult VKAPI_CALL FlushMappedMemoryRanges(
    VkDevice                                    device,
    uint32_t                                    memoryRangeCount,
    const VkMappedMemoryRange*                  pMemoryRanges) {
    layer_data *device_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    {
        bool skip = false;
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            skip |= intercept->PreCallValidateFlushMappedMemoryRanges(device, memoryRangeCount, pMemoryRanges);
            if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
        }
        for (auto intercept : global_interceptor_list) {
            intercept->PreCallRecordFlushMappedMemoryRanges(device, memoryRangeCount, pMemoryRanges);
        }
    }
    VkResult result = device_data->dispatch_table.FlushMappedMemoryRanges(device, memoryRangeCount, pMemoryRanges);
    if (VK_SUCCESS == result) {
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            intercept->PostCallRecordFlushMappedMemoryRanges(device, memoryRangeCount, pMemoryRanges);
        }
    }
    return result;
}

VKAPI_ATTR VkResult VKAPI_CALL InvalidateMappedMemoryRanges(
    VkDevice                                    device,
    uint32_t                                    memoryRangeCount,
    const VkMappedMemoryRange*                  pMemoryRanges) {
    layer_data *device_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    {
        bool skip = false;
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            skip |= intercept->PreCallValidateInvalidateMappedMemoryRanges(device, memoryRangeCount, pMemoryRanges);
            if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
        }
        for (auto intercept : global_interceptor_list) {
            intercept->PreCallRecordInvalidateMappedMemoryRanges(device, memoryRangeCount, pMemoryRanges);
        }
    }
    VkResult result = device_data->dispatch_table.InvalidateMappedMemoryRanges(device, memoryRangeCount, pMemoryRanges);
    if (VK_SUCCESS == result) {
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            intercept->PostCallRecordInvalidateMappedMemoryRanges(device, memoryRangeCount, pMemoryRanges);
        }
    }
    return result;
}

VKAPI_ATTR void VKAPI_CALL GetDeviceMemoryCommitment(
    VkDevice                                    device,
    VkDeviceMemory                              memory,
    VkDeviceSize*                               pCommittedMemoryInBytes) {
    layer_data *device_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    {
        bool skip = false;
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            skip |= intercept->PreCallValidateGetDeviceMemoryCommitment(device, memory, pCommittedMemoryInBytes);
            if (skip) return;
        }
        for (auto intercept : global_interceptor_list) {
            intercept->PreCallRecordGetDeviceMemoryCommitment(device, memory, pCommittedMemoryInBytes);
        }
    }
    device_data->dispatch_table.GetDeviceMemoryCommitment(device, memory, pCommittedMemoryInBytes);
    {
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            intercept->PostCallRecordGetDeviceMemoryCommitment(device, memory, pCommittedMemoryInBytes);
        }
    }
}

VKAPI_ATTR VkResult VKAPI_CALL BindBufferMemory(
    VkDevice                                    device,
    VkBuffer                                    buffer,
    VkDeviceMemory                              memory,
    VkDeviceSize                                memoryOffset) {
    layer_data *device_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    {
        bool skip = false;
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            skip |= intercept->PreCallValidateBindBufferMemory(device, buffer, memory, memoryOffset);
            if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
        }
        for (auto intercept : global_interceptor_list) {
            intercept->PreCallRecordBindBufferMemory(device, buffer, memory, memoryOffset);
        }
    }
    VkResult result = device_data->dispatch_table.BindBufferMemory(device, buffer, memory, memoryOffset);
    if (VK_SUCCESS == result) {
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            intercept->PostCallRecordBindBufferMemory(device, buffer, memory, memoryOffset);
        }
    }
    return result;
}

VKAPI_ATTR VkResult VKAPI_CALL BindImageMemory(
    VkDevice                                    device,
    VkImage                                     image,
    VkDeviceMemory                              memory,
    VkDeviceSize                                memoryOffset) {
    layer_data *device_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    {
        bool skip = false;
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            skip |= intercept->PreCallValidateBindImageMemory(device, image, memory, memoryOffset);
            if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
        }
        for (auto intercept : global_interceptor_list) {
            intercept->PreCallRecordBindImageMemory(device, image, memory, memoryOffset);
        }
    }
    VkResult result = device_data->dispatch_table.BindImageMemory(device, image, memory, memoryOffset);
    if (VK_SUCCESS == result) {
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            intercept->PostCallRecordBindImageMemory(device, image, memory, memoryOffset);
        }
    }
    return result;
}

VKAPI_ATTR void VKAPI_CALL GetBufferMemoryRequirements(
    VkDevice                                    device,
    VkBuffer                                    buffer,
    VkMemoryRequirements*                       pMemoryRequirements) {
    layer_data *device_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    {
        bool skip = false;
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            skip |= intercept->PreCallValidateGetBufferMemoryRequirements(device, buffer, pMemoryRequirements);
            if (skip) return;
        }
        for (auto intercept : global_interceptor_list) {
            intercept->PreCallRecordGetBufferMemoryRequirements(device, buffer, pMemoryRequirements);
        }
    }
    device_data->dispatch_table.GetBufferMemoryRequirements(device, buffer, pMemoryRequirements);
    {
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            intercept->PostCallRecordGetBufferMemoryRequirements(device, buffer, pMemoryRequirements);
        }
    }
}

VKAPI_ATTR void VKAPI_CALL GetImageMemoryRequirements(
    VkDevice                                    device,
    VkImage                                     image,
    VkMemoryRequirements*                       pMemoryRequirements) {
    layer_data *device_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    {
        bool skip = false;
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            skip |= intercept->PreCallValidateGetImageMemoryRequirements(device, image, pMemoryRequirements);
            if (skip) return;
        }
        for (auto intercept : global_interceptor_list) {
            intercept->PreCallRecordGetImageMemoryRequirements(device, image, pMemoryRequirements);
        }
    }
    device_data->dispatch_table.GetImageMemoryRequirements(device, image, pMemoryRequirements);
    {
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            intercept->PostCallRecordGetImageMemoryRequirements(device, image, pMemoryRequirements);
        }
    }
}

VKAPI_ATTR void VKAPI_CALL GetImageSparseMemoryRequirements(
    VkDevice                                    device,
    VkImage                                     image,
    uint32_t*                                   pSparseMemoryRequirementCount,
    VkSparseImageMemoryRequirements*            pSparseMemoryRequirements) {
    layer_data *device_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    {
        bool skip = false;
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            skip |= intercept->PreCallValidateGetImageSparseMemoryRequirements(device, image, pSparseMemoryRequirementCount, pSparseMemoryRequirements);
            if (skip) return;
        }
        for (auto intercept : global_interceptor_list) {
            intercept->PreCallRecordGetImageSparseMemoryRequirements(device, image, pSparseMemoryRequirementCount, pSparseMemoryRequirements);
        }
    }
    device_data->dispatch_table.GetImageSparseMemoryRequirements(device, image, pSparseMemoryRequirementCount, pSparseMemoryRequirements);
    {
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            intercept->PostCallRecordGetImageSparseMemoryRequirements(device, image, pSparseMemoryRequirementCount, pSparseMemoryRequirements);
        }
    }
}

VKAPI_ATTR void VKAPI_CALL GetPhysicalDeviceSparseImageFormatProperties(
    VkPhysicalDevice                            physicalDevice,
    VkFormat                                    format,
    VkImageType                                 type,
    VkSampleCountFlagBits                       samples,
    VkImageUsageFlags                           usage,
    VkImageTiling                               tiling,
    uint32_t*                                   pPropertyCount,
    VkSparseImageFormatProperties*              pProperties) {
    instance_layer_data *instance_data = GetLayerDataPtr(get_dispatch_key(physicalDevice), instance_layer_data_map);
    {
        bool skip = false;
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            skip |= intercept->PreCallValidateGetPhysicalDeviceSparseImageFormatProperties(physicalDevice, format, type, samples, usage, tiling, pPropertyCount, pProperties);
            if (skip) return;
        }
        for (auto intercept : global_interceptor_list) {
            intercept->PreCallRecordGetPhysicalDeviceSparseImageFormatProperties(physicalDevice, format, type, samples, usage, tiling, pPropertyCount, pProperties);
        }
    }
    instance_data->dispatch_table.GetPhysicalDeviceSparseImageFormatProperties(physicalDevice, format, type, samples, usage, tiling, pPropertyCount, pProperties);
    {
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            intercept->PostCallRecordGetPhysicalDeviceSparseImageFormatProperties(physicalDevice, format, type, samples, usage, tiling, pPropertyCount, pProperties);
        }
    }
}

VKAPI_ATTR VkResult VKAPI_CALL QueueBindSparse(
    VkQueue                                     queue,
    uint32_t                                    bindInfoCount,
    const VkBindSparseInfo*                     pBindInfo,
    VkFence                                     fence) {
    layer_data *device_data = GetLayerDataPtr(get_dispatch_key(queue), layer_data_map);
    {
        bool skip = false;
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            skip |= intercept->PreCallValidateQueueBindSparse(queue, bindInfoCount, pBindInfo, fence);
            if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
        }
        for (auto intercept : global_interceptor_list) {
            intercept->PreCallRecordQueueBindSparse(queue, bindInfoCount, pBindInfo, fence);
        }
    }
    VkResult result = device_data->dispatch_table.QueueBindSparse(queue, bindInfoCount, pBindInfo, fence);
    if (VK_SUCCESS == result) {
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            intercept->PostCallRecordQueueBindSparse(queue, bindInfoCount, pBindInfo, fence);
        }
    }
    return result;
}

VKAPI_ATTR VkResult VKAPI_CALL CreateFence(
    VkDevice                                    device,
    const VkFenceCreateInfo*                    pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkFence*                                    pFence) {
    layer_data *device_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    {
        bool skip = false;
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            skip |= intercept->PreCallValidateCreateFence(device, pCreateInfo, pAllocator, pFence);
            if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
        }
        for (auto intercept : global_interceptor_list) {
            intercept->PreCallRecordCreateFence(device, pCreateInfo, pAllocator, pFence);
        }
    }
    VkResult result = device_data->dispatch_table.CreateFence(device, pCreateInfo, pAllocator, pFence);
    if (VK_SUCCESS == result) {
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            intercept->PostCallRecordCreateFence(device, pCreateInfo, pAllocator, pFence);
        }
    }
    return result;
}

VKAPI_ATTR void VKAPI_CALL DestroyFence(
    VkDevice                                    device,
    VkFence                                     fence,
    const VkAllocationCallbacks*                pAllocator) {
    layer_data *device_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    {
        bool skip = false;
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            skip |= intercept->PreCallValidateDestroyFence(device, fence, pAllocator);
            if (skip) return;
        }
        for (auto intercept : global_interceptor_list) {
            intercept->PreCallRecordDestroyFence(device, fence, pAllocator);
        }
    }
    device_data->dispatch_table.DestroyFence(device, fence, pAllocator);
    {
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            intercept->PostCallRecordDestroyFence(device, fence, pAllocator);
        }
    }
}

VKAPI_ATTR VkResult VKAPI_CALL ResetFences(
    VkDevice                                    device,
    uint32_t                                    fenceCount,
    const VkFence*                              pFences) {
    layer_data *device_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    {
        bool skip = false;
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            skip |= intercept->PreCallValidateResetFences(device, fenceCount, pFences);
            if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
        }
        for (auto intercept : global_interceptor_list) {
            intercept->PreCallRecordResetFences(device, fenceCount, pFences);
        }
    }
    VkResult result = device_data->dispatch_table.ResetFences(device, fenceCount, pFences);
    if (VK_SUCCESS == result) {
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            intercept->PostCallRecordResetFences(device, fenceCount, pFences);
        }
    }
    return result;
}

VKAPI_ATTR VkResult VKAPI_CALL GetFenceStatus(
    VkDevice                                    device,
    VkFence                                     fence) {
    layer_data *device_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    {
        bool skip = false;
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            skip |= intercept->PreCallValidateGetFenceStatus(device, fence);
            if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
        }
        for (auto intercept : global_interceptor_list) {
            intercept->PreCallRecordGetFenceStatus(device, fence);
        }
    }
    VkResult result = device_data->dispatch_table.GetFenceStatus(device, fence);
    if (VK_SUCCESS == result) {
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            intercept->PostCallRecordGetFenceStatus(device, fence);
        }
    }
    return result;
}

VKAPI_ATTR VkResult VKAPI_CALL WaitForFences(
    VkDevice                                    device,
    uint32_t                                    fenceCount,
    const VkFence*                              pFences,
    VkBool32                                    waitAll,
    uint64_t                                    timeout) {
    layer_data *device_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    {
        bool skip = false;
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            skip |= intercept->PreCallValidateWaitForFences(device, fenceCount, pFences, waitAll, timeout);
            if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
        }
        for (auto intercept : global_interceptor_list) {
            intercept->PreCallRecordWaitForFences(device, fenceCount, pFences, waitAll, timeout);
        }
    }
    VkResult result = device_data->dispatch_table.WaitForFences(device, fenceCount, pFences, waitAll, timeout);
    if (VK_SUCCESS == result) {
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            intercept->PostCallRecordWaitForFences(device, fenceCount, pFences, waitAll, timeout);
        }
    }
    return result;
}

VKAPI_ATTR VkResult VKAPI_CALL CreateSemaphore(
    VkDevice                                    device,
    const VkSemaphoreCreateInfo*                pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkSemaphore*                                pSemaphore) {
    layer_data *device_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    {
        bool skip = false;
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            skip |= intercept->PreCallValidateCreateSemaphore(device, pCreateInfo, pAllocator, pSemaphore);
            if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
        }
        for (auto intercept : global_interceptor_list) {
            intercept->PreCallRecordCreateSemaphore(device, pCreateInfo, pAllocator, pSemaphore);
        }
    }
    VkResult result = device_data->dispatch_table.CreateSemaphore(device, pCreateInfo, pAllocator, pSemaphore);
    if (VK_SUCCESS == result) {
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            intercept->PostCallRecordCreateSemaphore(device, pCreateInfo, pAllocator, pSemaphore);
        }
    }
    return result;
}

VKAPI_ATTR void VKAPI_CALL DestroySemaphore(
    VkDevice                                    device,
    VkSemaphore                                 semaphore,
    const VkAllocationCallbacks*                pAllocator) {
    layer_data *device_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    {
        bool skip = false;
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            skip |= intercept->PreCallValidateDestroySemaphore(device, semaphore, pAllocator);
            if (skip) return;
        }
        for (auto intercept : global_interceptor_list) {
            intercept->PreCallRecordDestroySemaphore(device, semaphore, pAllocator);
        }
    }
    device_data->dispatch_table.DestroySemaphore(device, semaphore, pAllocator);
    {
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            intercept->PostCallRecordDestroySemaphore(device, semaphore, pAllocator);
        }
    }
}

VKAPI_ATTR VkResult VKAPI_CALL CreateEvent(
    VkDevice                                    device,
    const VkEventCreateInfo*                    pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkEvent*                                    pEvent) {
    layer_data *device_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    {
        bool skip = false;
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            skip |= intercept->PreCallValidateCreateEvent(device, pCreateInfo, pAllocator, pEvent);
            if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
        }
        for (auto intercept : global_interceptor_list) {
            intercept->PreCallRecordCreateEvent(device, pCreateInfo, pAllocator, pEvent);
        }
    }
    VkResult result = device_data->dispatch_table.CreateEvent(device, pCreateInfo, pAllocator, pEvent);
    if (VK_SUCCESS == result) {
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            intercept->PostCallRecordCreateEvent(device, pCreateInfo, pAllocator, pEvent);
        }
    }
    return result;
}

VKAPI_ATTR void VKAPI_CALL DestroyEvent(
    VkDevice                                    device,
    VkEvent                                     event,
    const VkAllocationCallbacks*                pAllocator) {
    layer_data *device_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    {
        bool skip = false;
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            skip |= intercept->PreCallValidateDestroyEvent(device, event, pAllocator);
            if (skip) return;
        }
        for (auto intercept : global_interceptor_list) {
            intercept->PreCallRecordDestroyEvent(device, event, pAllocator);
        }
    }
    device_data->dispatch_table.DestroyEvent(device, event, pAllocator);
    {
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            intercept->PostCallRecordDestroyEvent(device, event, pAllocator);
        }
    }
}

VKAPI_ATTR VkResult VKAPI_CALL GetEventStatus(
    VkDevice                                    device,
    VkEvent                                     event) {
    layer_data *device_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    {
        bool skip = false;
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            skip |= intercept->PreCallValidateGetEventStatus(device, event);
            if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
        }
        for (auto intercept : global_interceptor_list) {
            intercept->PreCallRecordGetEventStatus(device, event);
        }
    }
    VkResult result = device_data->dispatch_table.GetEventStatus(device, event);
    if (VK_SUCCESS == result) {
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            intercept->PostCallRecordGetEventStatus(device, event);
        }
    }
    return result;
}

VKAPI_ATTR VkResult VKAPI_CALL SetEvent(
    VkDevice                                    device,
    VkEvent                                     event) {
    layer_data *device_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    {
        bool skip = false;
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            skip |= intercept->PreCallValidateSetEvent(device, event);
            if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
        }
        for (auto intercept : global_interceptor_list) {
            intercept->PreCallRecordSetEvent(device, event);
        }
    }
    VkResult result = device_data->dispatch_table.SetEvent(device, event);
    if (VK_SUCCESS == result) {
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            intercept->PostCallRecordSetEvent(device, event);
        }
    }
    return result;
}

VKAPI_ATTR VkResult VKAPI_CALL ResetEvent(
    VkDevice                                    device,
    VkEvent                                     event) {
    layer_data *device_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    {
        bool skip = false;
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            skip |= intercept->PreCallValidateResetEvent(device, event);
            if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
        }
        for (auto intercept : global_interceptor_list) {
            intercept->PreCallRecordResetEvent(device, event);
        }
    }
    VkResult result = device_data->dispatch_table.ResetEvent(device, event);
    if (VK_SUCCESS == result) {
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            intercept->PostCallRecordResetEvent(device, event);
        }
    }
    return result;
}

VKAPI_ATTR VkResult VKAPI_CALL CreateQueryPool(
    VkDevice                                    device,
    const VkQueryPoolCreateInfo*                pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkQueryPool*                                pQueryPool) {
    layer_data *device_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    {
        bool skip = false;
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            skip |= intercept->PreCallValidateCreateQueryPool(device, pCreateInfo, pAllocator, pQueryPool);
            if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
        }
        for (auto intercept : global_interceptor_list) {
            intercept->PreCallRecordCreateQueryPool(device, pCreateInfo, pAllocator, pQueryPool);
        }
    }
    VkResult result = device_data->dispatch_table.CreateQueryPool(device, pCreateInfo, pAllocator, pQueryPool);
    if (VK_SUCCESS == result) {
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            intercept->PostCallRecordCreateQueryPool(device, pCreateInfo, pAllocator, pQueryPool);
        }
    }
    return result;
}

VKAPI_ATTR void VKAPI_CALL DestroyQueryPool(
    VkDevice                                    device,
    VkQueryPool                                 queryPool,
    const VkAllocationCallbacks*                pAllocator) {
    layer_data *device_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    {
        bool skip = false;
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            skip |= intercept->PreCallValidateDestroyQueryPool(device, queryPool, pAllocator);
            if (skip) return;
        }
        for (auto intercept : global_interceptor_list) {
            intercept->PreCallRecordDestroyQueryPool(device, queryPool, pAllocator);
        }
    }
    device_data->dispatch_table.DestroyQueryPool(device, queryPool, pAllocator);
    {
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            intercept->PostCallRecordDestroyQueryPool(device, queryPool, pAllocator);
        }
    }
}

VKAPI_ATTR VkResult VKAPI_CALL GetQueryPoolResults(
    VkDevice                                    device,
    VkQueryPool                                 queryPool,
    uint32_t                                    firstQuery,
    uint32_t                                    queryCount,
    size_t                                      dataSize,
    void*                                       pData,
    VkDeviceSize                                stride,
    VkQueryResultFlags                          flags) {
    layer_data *device_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    {
        bool skip = false;
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            skip |= intercept->PreCallValidateGetQueryPoolResults(device, queryPool, firstQuery, queryCount, dataSize, pData, stride, flags);
            if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
        }
        for (auto intercept : global_interceptor_list) {
            intercept->PreCallRecordGetQueryPoolResults(device, queryPool, firstQuery, queryCount, dataSize, pData, stride, flags);
        }
    }
    VkResult result = device_data->dispatch_table.GetQueryPoolResults(device, queryPool, firstQuery, queryCount, dataSize, pData, stride, flags);
    if (VK_SUCCESS == result) {
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            intercept->PostCallRecordGetQueryPoolResults(device, queryPool, firstQuery, queryCount, dataSize, pData, stride, flags);
        }
    }
    return result;
}

VKAPI_ATTR VkResult VKAPI_CALL CreateBuffer(
    VkDevice                                    device,
    const VkBufferCreateInfo*                   pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkBuffer*                                   pBuffer) {
    layer_data *device_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    {
        bool skip = false;
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            skip |= intercept->PreCallValidateCreateBuffer(device, pCreateInfo, pAllocator, pBuffer);
            if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
        }
        for (auto intercept : global_interceptor_list) {
            intercept->PreCallRecordCreateBuffer(device, pCreateInfo, pAllocator, pBuffer);
        }
    }
    VkResult result = device_data->dispatch_table.CreateBuffer(device, pCreateInfo, pAllocator, pBuffer);
    if (VK_SUCCESS == result) {
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            intercept->PostCallRecordCreateBuffer(device, pCreateInfo, pAllocator, pBuffer);
        }
    }
    return result;
}

VKAPI_ATTR void VKAPI_CALL DestroyBuffer(
    VkDevice                                    device,
    VkBuffer                                    buffer,
    const VkAllocationCallbacks*                pAllocator) {
    layer_data *device_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    {
        bool skip = false;
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            skip |= intercept->PreCallValidateDestroyBuffer(device, buffer, pAllocator);
            if (skip) return;
        }
        for (auto intercept : global_interceptor_list) {
            intercept->PreCallRecordDestroyBuffer(device, buffer, pAllocator);
        }
    }
    device_data->dispatch_table.DestroyBuffer(device, buffer, pAllocator);
    {
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            intercept->PostCallRecordDestroyBuffer(device, buffer, pAllocator);
        }
    }
}

VKAPI_ATTR VkResult VKAPI_CALL CreateBufferView(
    VkDevice                                    device,
    const VkBufferViewCreateInfo*               pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkBufferView*                               pView) {
    layer_data *device_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    {
        bool skip = false;
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            skip |= intercept->PreCallValidateCreateBufferView(device, pCreateInfo, pAllocator, pView);
            if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
        }
        for (auto intercept : global_interceptor_list) {
            intercept->PreCallRecordCreateBufferView(device, pCreateInfo, pAllocator, pView);
        }
    }
    VkResult result = device_data->dispatch_table.CreateBufferView(device, pCreateInfo, pAllocator, pView);
    if (VK_SUCCESS == result) {
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            intercept->PostCallRecordCreateBufferView(device, pCreateInfo, pAllocator, pView);
        }
    }
    return result;
}

VKAPI_ATTR void VKAPI_CALL DestroyBufferView(
    VkDevice                                    device,
    VkBufferView                                bufferView,
    const VkAllocationCallbacks*                pAllocator) {
    layer_data *device_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    {
        bool skip = false;
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            skip |= intercept->PreCallValidateDestroyBufferView(device, bufferView, pAllocator);
            if (skip) return;
        }
        for (auto intercept : global_interceptor_list) {
            intercept->PreCallRecordDestroyBufferView(device, bufferView, pAllocator);
        }
    }
    device_data->dispatch_table.DestroyBufferView(device, bufferView, pAllocator);
    {
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            intercept->PostCallRecordDestroyBufferView(device, bufferView, pAllocator);
        }
    }
}

VKAPI_ATTR VkResult VKAPI_CALL CreateImage(
    VkDevice                                    device,
    const VkImageCreateInfo*                    pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkImage*                                    pImage) {
    layer_data *device_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    {
        bool skip = false;
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            skip |= intercept->PreCallValidateCreateImage(device, pCreateInfo, pAllocator, pImage);
            if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
        }
        for (auto intercept : global_interceptor_list) {
            intercept->PreCallRecordCreateImage(device, pCreateInfo, pAllocator, pImage);
        }
    }
    VkResult result = device_data->dispatch_table.CreateImage(device, pCreateInfo, pAllocator, pImage);
    if (VK_SUCCESS == result) {
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            intercept->PostCallRecordCreateImage(device, pCreateInfo, pAllocator, pImage);
        }
    }
    return result;
}

VKAPI_ATTR void VKAPI_CALL DestroyImage(
    VkDevice                                    device,
    VkImage                                     image,
    const VkAllocationCallbacks*                pAllocator) {
    layer_data *device_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    {
        bool skip = false;
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            skip |= intercept->PreCallValidateDestroyImage(device, image, pAllocator);
            if (skip) return;
        }
        for (auto intercept : global_interceptor_list) {
            intercept->PreCallRecordDestroyImage(device, image, pAllocator);
        }
    }
    device_data->dispatch_table.DestroyImage(device, image, pAllocator);
    {
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            intercept->PostCallRecordDestroyImage(device, image, pAllocator);
        }
    }
}

VKAPI_ATTR void VKAPI_CALL GetImageSubresourceLayout(
    VkDevice                                    device,
    VkImage                                     image,
    const VkImageSubresource*                   pSubresource,
    VkSubresourceLayout*                        pLayout) {
    layer_data *device_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    {
        bool skip = false;
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            skip |= intercept->PreCallValidateGetImageSubresourceLayout(device, image, pSubresource, pLayout);
            if (skip) return;
        }
        for (auto intercept : global_interceptor_list) {
            intercept->PreCallRecordGetImageSubresourceLayout(device, image, pSubresource, pLayout);
        }
    }
    device_data->dispatch_table.GetImageSubresourceLayout(device, image, pSubresource, pLayout);
    {
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            intercept->PostCallRecordGetImageSubresourceLayout(device, image, pSubresource, pLayout);
        }
    }
}

VKAPI_ATTR VkResult VKAPI_CALL CreateImageView(
    VkDevice                                    device,
    const VkImageViewCreateInfo*                pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkImageView*                                pView) {
    layer_data *device_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    {
        bool skip = false;
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            skip |= intercept->PreCallValidateCreateImageView(device, pCreateInfo, pAllocator, pView);
            if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
        }
        for (auto intercept : global_interceptor_list) {
            intercept->PreCallRecordCreateImageView(device, pCreateInfo, pAllocator, pView);
        }
    }
    VkResult result = device_data->dispatch_table.CreateImageView(device, pCreateInfo, pAllocator, pView);
    if (VK_SUCCESS == result) {
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            intercept->PostCallRecordCreateImageView(device, pCreateInfo, pAllocator, pView);
        }
    }
    return result;
}

VKAPI_ATTR void VKAPI_CALL DestroyImageView(
    VkDevice                                    device,
    VkImageView                                 imageView,
    const VkAllocationCallbacks*                pAllocator) {
    layer_data *device_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    {
        bool skip = false;
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            skip |= intercept->PreCallValidateDestroyImageView(device, imageView, pAllocator);
            if (skip) return;
        }
        for (auto intercept : global_interceptor_list) {
            intercept->PreCallRecordDestroyImageView(device, imageView, pAllocator);
        }
    }
    device_data->dispatch_table.DestroyImageView(device, imageView, pAllocator);
    {
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            intercept->PostCallRecordDestroyImageView(device, imageView, pAllocator);
        }
    }
}

VKAPI_ATTR VkResult VKAPI_CALL CreateShaderModule(
    VkDevice                                    device,
    const VkShaderModuleCreateInfo*             pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkShaderModule*                             pShaderModule) {
    layer_data *device_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    {
        bool skip = false;
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            skip |= intercept->PreCallValidateCreateShaderModule(device, pCreateInfo, pAllocator, pShaderModule);
            if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
        }
        for (auto intercept : global_interceptor_list) {
            intercept->PreCallRecordCreateShaderModule(device, pCreateInfo, pAllocator, pShaderModule);
        }
    }
    VkResult result = device_data->dispatch_table.CreateShaderModule(device, pCreateInfo, pAllocator, pShaderModule);
    if (VK_SUCCESS == result) {
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            intercept->PostCallRecordCreateShaderModule(device, pCreateInfo, pAllocator, pShaderModule);
        }
    }
    return result;
}

VKAPI_ATTR void VKAPI_CALL DestroyShaderModule(
    VkDevice                                    device,
    VkShaderModule                              shaderModule,
    const VkAllocationCallbacks*                pAllocator) {
    layer_data *device_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    {
        bool skip = false;
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            skip |= intercept->PreCallValidateDestroyShaderModule(device, shaderModule, pAllocator);
            if (skip) return;
        }
        for (auto intercept : global_interceptor_list) {
            intercept->PreCallRecordDestroyShaderModule(device, shaderModule, pAllocator);
        }
    }
    device_data->dispatch_table.DestroyShaderModule(device, shaderModule, pAllocator);
    {
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            intercept->PostCallRecordDestroyShaderModule(device, shaderModule, pAllocator);
        }
    }
}

VKAPI_ATTR VkResult VKAPI_CALL CreatePipelineCache(
    VkDevice                                    device,
    const VkPipelineCacheCreateInfo*            pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkPipelineCache*                            pPipelineCache) {
    layer_data *device_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    {
        bool skip = false;
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            skip |= intercept->PreCallValidateCreatePipelineCache(device, pCreateInfo, pAllocator, pPipelineCache);
            if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
        }
        for (auto intercept : global_interceptor_list) {
            intercept->PreCallRecordCreatePipelineCache(device, pCreateInfo, pAllocator, pPipelineCache);
        }
    }
    VkResult result = device_data->dispatch_table.CreatePipelineCache(device, pCreateInfo, pAllocator, pPipelineCache);
    if (VK_SUCCESS == result) {
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            intercept->PostCallRecordCreatePipelineCache(device, pCreateInfo, pAllocator, pPipelineCache);
        }
    }
    return result;
}

VKAPI_ATTR void VKAPI_CALL DestroyPipelineCache(
    VkDevice                                    device,
    VkPipelineCache                             pipelineCache,
    const VkAllocationCallbacks*                pAllocator) {
    layer_data *device_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    {
        bool skip = false;
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            skip |= intercept->PreCallValidateDestroyPipelineCache(device, pipelineCache, pAllocator);
            if (skip) return;
        }
        for (auto intercept : global_interceptor_list) {
            intercept->PreCallRecordDestroyPipelineCache(device, pipelineCache, pAllocator);
        }
    }
    device_data->dispatch_table.DestroyPipelineCache(device, pipelineCache, pAllocator);
    {
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            intercept->PostCallRecordDestroyPipelineCache(device, pipelineCache, pAllocator);
        }
    }
}

VKAPI_ATTR VkResult VKAPI_CALL GetPipelineCacheData(
    VkDevice                                    device,
    VkPipelineCache                             pipelineCache,
    size_t*                                     pDataSize,
    void*                                       pData) {
    layer_data *device_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    {
        bool skip = false;
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            skip |= intercept->PreCallValidateGetPipelineCacheData(device, pipelineCache, pDataSize, pData);
            if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
        }
        for (auto intercept : global_interceptor_list) {
            intercept->PreCallRecordGetPipelineCacheData(device, pipelineCache, pDataSize, pData);
        }
    }
    VkResult result = device_data->dispatch_table.GetPipelineCacheData(device, pipelineCache, pDataSize, pData);
    if (VK_SUCCESS == result) {
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            intercept->PostCallRecordGetPipelineCacheData(device, pipelineCache, pDataSize, pData);
        }
    }
    return result;
}

VKAPI_ATTR VkResult VKAPI_CALL MergePipelineCaches(
    VkDevice                                    device,
    VkPipelineCache                             dstCache,
    uint32_t                                    srcCacheCount,
    const VkPipelineCache*                      pSrcCaches) {
    layer_data *device_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    {
        bool skip = false;
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            skip |= intercept->PreCallValidateMergePipelineCaches(device, dstCache, srcCacheCount, pSrcCaches);
            if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
        }
        for (auto intercept : global_interceptor_list) {
            intercept->PreCallRecordMergePipelineCaches(device, dstCache, srcCacheCount, pSrcCaches);
        }
    }
    VkResult result = device_data->dispatch_table.MergePipelineCaches(device, dstCache, srcCacheCount, pSrcCaches);
    if (VK_SUCCESS == result) {
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            intercept->PostCallRecordMergePipelineCaches(device, dstCache, srcCacheCount, pSrcCaches);
        }
    }
    return result;
}

VKAPI_ATTR VkResult VKAPI_CALL CreateGraphicsPipelines(
    VkDevice                                    device,
    VkPipelineCache                             pipelineCache,
    uint32_t                                    createInfoCount,
    const VkGraphicsPipelineCreateInfo*         pCreateInfos,
    const VkAllocationCallbacks*                pAllocator,
    VkPipeline*                                 pPipelines) {
    layer_data *device_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    {
        bool skip = false;
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            skip |= intercept->PreCallValidateCreateGraphicsPipelines(device, pipelineCache, createInfoCount, pCreateInfos, pAllocator, pPipelines);
            if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
        }
        for (auto intercept : global_interceptor_list) {
            intercept->PreCallRecordCreateGraphicsPipelines(device, pipelineCache, createInfoCount, pCreateInfos, pAllocator, pPipelines);
        }
    }
    VkResult result = device_data->dispatch_table.CreateGraphicsPipelines(device, pipelineCache, createInfoCount, pCreateInfos, pAllocator, pPipelines);
    if (VK_SUCCESS == result) {
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            intercept->PostCallRecordCreateGraphicsPipelines(device, pipelineCache, createInfoCount, pCreateInfos, pAllocator, pPipelines);
        }
    }
    return result;
}

VKAPI_ATTR VkResult VKAPI_CALL CreateComputePipelines(
    VkDevice                                    device,
    VkPipelineCache                             pipelineCache,
    uint32_t                                    createInfoCount,
    const VkComputePipelineCreateInfo*          pCreateInfos,
    const VkAllocationCallbacks*                pAllocator,
    VkPipeline*                                 pPipelines) {
    layer_data *device_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    {
        bool skip = false;
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            skip |= intercept->PreCallValidateCreateComputePipelines(device, pipelineCache, createInfoCount, pCreateInfos, pAllocator, pPipelines);
            if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
        }
        for (auto intercept : global_interceptor_list) {
            intercept->PreCallRecordCreateComputePipelines(device, pipelineCache, createInfoCount, pCreateInfos, pAllocator, pPipelines);
        }
    }
    VkResult result = device_data->dispatch_table.CreateComputePipelines(device, pipelineCache, createInfoCount, pCreateInfos, pAllocator, pPipelines);
    if (VK_SUCCESS == result) {
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            intercept->PostCallRecordCreateComputePipelines(device, pipelineCache, createInfoCount, pCreateInfos, pAllocator, pPipelines);
        }
    }
    return result;
}

VKAPI_ATTR void VKAPI_CALL DestroyPipeline(
    VkDevice                                    device,
    VkPipeline                                  pipeline,
    const VkAllocationCallbacks*                pAllocator) {
    layer_data *device_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    {
        bool skip = false;
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            skip |= intercept->PreCallValidateDestroyPipeline(device, pipeline, pAllocator);
            if (skip) return;
        }
        for (auto intercept : global_interceptor_list) {
            intercept->PreCallRecordDestroyPipeline(device, pipeline, pAllocator);
        }
    }
    device_data->dispatch_table.DestroyPipeline(device, pipeline, pAllocator);
    {
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            intercept->PostCallRecordDestroyPipeline(device, pipeline, pAllocator);
        }
    }
}

VKAPI_ATTR VkResult VKAPI_CALL CreatePipelineLayout(
    VkDevice                                    device,
    const VkPipelineLayoutCreateInfo*           pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkPipelineLayout*                           pPipelineLayout) {
    layer_data *device_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    {
        bool skip = false;
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            skip |= intercept->PreCallValidateCreatePipelineLayout(device, pCreateInfo, pAllocator, pPipelineLayout);
            if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
        }
        for (auto intercept : global_interceptor_list) {
            intercept->PreCallRecordCreatePipelineLayout(device, pCreateInfo, pAllocator, pPipelineLayout);
        }
    }
    VkResult result = device_data->dispatch_table.CreatePipelineLayout(device, pCreateInfo, pAllocator, pPipelineLayout);
    if (VK_SUCCESS == result) {
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            intercept->PostCallRecordCreatePipelineLayout(device, pCreateInfo, pAllocator, pPipelineLayout);
        }
    }
    return result;
}

VKAPI_ATTR void VKAPI_CALL DestroyPipelineLayout(
    VkDevice                                    device,
    VkPipelineLayout                            pipelineLayout,
    const VkAllocationCallbacks*                pAllocator) {
    layer_data *device_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    {
        bool skip = false;
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            skip |= intercept->PreCallValidateDestroyPipelineLayout(device, pipelineLayout, pAllocator);
            if (skip) return;
        }
        for (auto intercept : global_interceptor_list) {
            intercept->PreCallRecordDestroyPipelineLayout(device, pipelineLayout, pAllocator);
        }
    }
    device_data->dispatch_table.DestroyPipelineLayout(device, pipelineLayout, pAllocator);
    {
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            intercept->PostCallRecordDestroyPipelineLayout(device, pipelineLayout, pAllocator);
        }
    }
}

VKAPI_ATTR VkResult VKAPI_CALL CreateSampler(
    VkDevice                                    device,
    const VkSamplerCreateInfo*                  pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkSampler*                                  pSampler) {
    layer_data *device_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    {
        bool skip = false;
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            skip |= intercept->PreCallValidateCreateSampler(device, pCreateInfo, pAllocator, pSampler);
            if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
        }
        for (auto intercept : global_interceptor_list) {
            intercept->PreCallRecordCreateSampler(device, pCreateInfo, pAllocator, pSampler);
        }
    }
    VkResult result = device_data->dispatch_table.CreateSampler(device, pCreateInfo, pAllocator, pSampler);
    if (VK_SUCCESS == result) {
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            intercept->PostCallRecordCreateSampler(device, pCreateInfo, pAllocator, pSampler);
        }
    }
    return result;
}

VKAPI_ATTR void VKAPI_CALL DestroySampler(
    VkDevice                                    device,
    VkSampler                                   sampler,
    const VkAllocationCallbacks*                pAllocator) {
    layer_data *device_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    {
        bool skip = false;
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            skip |= intercept->PreCallValidateDestroySampler(device, sampler, pAllocator);
            if (skip) return;
        }
        for (auto intercept : global_interceptor_list) {
            intercept->PreCallRecordDestroySampler(device, sampler, pAllocator);
        }
    }
    device_data->dispatch_table.DestroySampler(device, sampler, pAllocator);
    {
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            intercept->PostCallRecordDestroySampler(device, sampler, pAllocator);
        }
    }
}

VKAPI_ATTR VkResult VKAPI_CALL CreateDescriptorSetLayout(
    VkDevice                                    device,
    const VkDescriptorSetLayoutCreateInfo*      pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkDescriptorSetLayout*                      pSetLayout) {
    layer_data *device_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    {
        bool skip = false;
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            skip |= intercept->PreCallValidateCreateDescriptorSetLayout(device, pCreateInfo, pAllocator, pSetLayout);
            if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
        }
        for (auto intercept : global_interceptor_list) {
            intercept->PreCallRecordCreateDescriptorSetLayout(device, pCreateInfo, pAllocator, pSetLayout);
        }
    }
    VkResult result = device_data->dispatch_table.CreateDescriptorSetLayout(device, pCreateInfo, pAllocator, pSetLayout);
    if (VK_SUCCESS == result) {
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            intercept->PostCallRecordCreateDescriptorSetLayout(device, pCreateInfo, pAllocator, pSetLayout);
        }
    }
    return result;
}

VKAPI_ATTR void VKAPI_CALL DestroyDescriptorSetLayout(
    VkDevice                                    device,
    VkDescriptorSetLayout                       descriptorSetLayout,
    const VkAllocationCallbacks*                pAllocator) {
    layer_data *device_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    {
        bool skip = false;
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            skip |= intercept->PreCallValidateDestroyDescriptorSetLayout(device, descriptorSetLayout, pAllocator);
            if (skip) return;
        }
        for (auto intercept : global_interceptor_list) {
            intercept->PreCallRecordDestroyDescriptorSetLayout(device, descriptorSetLayout, pAllocator);
        }
    }
    device_data->dispatch_table.DestroyDescriptorSetLayout(device, descriptorSetLayout, pAllocator);
    {
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            intercept->PostCallRecordDestroyDescriptorSetLayout(device, descriptorSetLayout, pAllocator);
        }
    }
}

VKAPI_ATTR VkResult VKAPI_CALL CreateDescriptorPool(
    VkDevice                                    device,
    const VkDescriptorPoolCreateInfo*           pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkDescriptorPool*                           pDescriptorPool) {
    layer_data *device_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    {
        bool skip = false;
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            skip |= intercept->PreCallValidateCreateDescriptorPool(device, pCreateInfo, pAllocator, pDescriptorPool);
            if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
        }
        for (auto intercept : global_interceptor_list) {
            intercept->PreCallRecordCreateDescriptorPool(device, pCreateInfo, pAllocator, pDescriptorPool);
        }
    }
    VkResult result = device_data->dispatch_table.CreateDescriptorPool(device, pCreateInfo, pAllocator, pDescriptorPool);
    if (VK_SUCCESS == result) {
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            intercept->PostCallRecordCreateDescriptorPool(device, pCreateInfo, pAllocator, pDescriptorPool);
        }
    }
    return result;
}

VKAPI_ATTR void VKAPI_CALL DestroyDescriptorPool(
    VkDevice                                    device,
    VkDescriptorPool                            descriptorPool,
    const VkAllocationCallbacks*                pAllocator) {
    layer_data *device_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    {
        bool skip = false;
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            skip |= intercept->PreCallValidateDestroyDescriptorPool(device, descriptorPool, pAllocator);
            if (skip) return;
        }
        for (auto intercept : global_interceptor_list) {
            intercept->PreCallRecordDestroyDescriptorPool(device, descriptorPool, pAllocator);
        }
    }
    device_data->dispatch_table.DestroyDescriptorPool(device, descriptorPool, pAllocator);
    {
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            intercept->PostCallRecordDestroyDescriptorPool(device, descriptorPool, pAllocator);
        }
    }
}

VKAPI_ATTR VkResult VKAPI_CALL ResetDescriptorPool(
    VkDevice                                    device,
    VkDescriptorPool                            descriptorPool,
    VkDescriptorPoolResetFlags                  flags) {
    layer_data *device_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    {
        bool skip = false;
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            skip |= intercept->PreCallValidateResetDescriptorPool(device, descriptorPool, flags);
            if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
        }
        for (auto intercept : global_interceptor_list) {
            intercept->PreCallRecordResetDescriptorPool(device, descriptorPool, flags);
        }
    }
    VkResult result = device_data->dispatch_table.ResetDescriptorPool(device, descriptorPool, flags);
    if (VK_SUCCESS == result) {
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            intercept->PostCallRecordResetDescriptorPool(device, descriptorPool, flags);
        }
    }
    return result;
}

VKAPI_ATTR VkResult VKAPI_CALL AllocateDescriptorSets(
    VkDevice                                    device,
    const VkDescriptorSetAllocateInfo*          pAllocateInfo,
    VkDescriptorSet*                            pDescriptorSets) {
    layer_data *device_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    {
        bool skip = false;
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            skip |= intercept->PreCallValidateAllocateDescriptorSets(device, pAllocateInfo, pDescriptorSets);
            if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
        }
        for (auto intercept : global_interceptor_list) {
            intercept->PreCallRecordAllocateDescriptorSets(device, pAllocateInfo, pDescriptorSets);
        }
    }
    VkResult result = device_data->dispatch_table.AllocateDescriptorSets(device, pAllocateInfo, pDescriptorSets);
    if (VK_SUCCESS == result) {
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            intercept->PostCallRecordAllocateDescriptorSets(device, pAllocateInfo, pDescriptorSets);
        }
    }
    return result;
}

VKAPI_ATTR VkResult VKAPI_CALL FreeDescriptorSets(
    VkDevice                                    device,
    VkDescriptorPool                            descriptorPool,
    uint32_t                                    descriptorSetCount,
    const VkDescriptorSet*                      pDescriptorSets) {
    layer_data *device_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    {
        bool skip = false;
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            skip |= intercept->PreCallValidateFreeDescriptorSets(device, descriptorPool, descriptorSetCount, pDescriptorSets);
            if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
        }
        for (auto intercept : global_interceptor_list) {
            intercept->PreCallRecordFreeDescriptorSets(device, descriptorPool, descriptorSetCount, pDescriptorSets);
        }
    }
    VkResult result = device_data->dispatch_table.FreeDescriptorSets(device, descriptorPool, descriptorSetCount, pDescriptorSets);
    if (VK_SUCCESS == result) {
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            intercept->PostCallRecordFreeDescriptorSets(device, descriptorPool, descriptorSetCount, pDescriptorSets);
        }
    }
    return result;
}

VKAPI_ATTR void VKAPI_CALL UpdateDescriptorSets(
    VkDevice                                    device,
    uint32_t                                    descriptorWriteCount,
    const VkWriteDescriptorSet*                 pDescriptorWrites,
    uint32_t                                    descriptorCopyCount,
    const VkCopyDescriptorSet*                  pDescriptorCopies) {
    layer_data *device_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    {
        bool skip = false;
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            skip |= intercept->PreCallValidateUpdateDescriptorSets(device, descriptorWriteCount, pDescriptorWrites, descriptorCopyCount, pDescriptorCopies);
            if (skip) return;
        }
        for (auto intercept : global_interceptor_list) {
            intercept->PreCallRecordUpdateDescriptorSets(device, descriptorWriteCount, pDescriptorWrites, descriptorCopyCount, pDescriptorCopies);
        }
    }
    device_data->dispatch_table.UpdateDescriptorSets(device, descriptorWriteCount, pDescriptorWrites, descriptorCopyCount, pDescriptorCopies);
    {
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            intercept->PostCallRecordUpdateDescriptorSets(device, descriptorWriteCount, pDescriptorWrites, descriptorCopyCount, pDescriptorCopies);
        }
    }
}

VKAPI_ATTR VkResult VKAPI_CALL CreateFramebuffer(
    VkDevice                                    device,
    const VkFramebufferCreateInfo*              pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkFramebuffer*                              pFramebuffer) {
    layer_data *device_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    {
        bool skip = false;
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            skip |= intercept->PreCallValidateCreateFramebuffer(device, pCreateInfo, pAllocator, pFramebuffer);
            if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
        }
        for (auto intercept : global_interceptor_list) {
            intercept->PreCallRecordCreateFramebuffer(device, pCreateInfo, pAllocator, pFramebuffer);
        }
    }
    VkResult result = device_data->dispatch_table.CreateFramebuffer(device, pCreateInfo, pAllocator, pFramebuffer);
    if (VK_SUCCESS == result) {
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            intercept->PostCallRecordCreateFramebuffer(device, pCreateInfo, pAllocator, pFramebuffer);
        }
    }
    return result;
}

VKAPI_ATTR void VKAPI_CALL DestroyFramebuffer(
    VkDevice                                    device,
    VkFramebuffer                               framebuffer,
    const VkAllocationCallbacks*                pAllocator) {
    layer_data *device_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    {
        bool skip = false;
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            skip |= intercept->PreCallValidateDestroyFramebuffer(device, framebuffer, pAllocator);
            if (skip) return;
        }
        for (auto intercept : global_interceptor_list) {
            intercept->PreCallRecordDestroyFramebuffer(device, framebuffer, pAllocator);
        }
    }
    device_data->dispatch_table.DestroyFramebuffer(device, framebuffer, pAllocator);
    {
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            intercept->PostCallRecordDestroyFramebuffer(device, framebuffer, pAllocator);
        }
    }
}

VKAPI_ATTR VkResult VKAPI_CALL CreateRenderPass(
    VkDevice                                    device,
    const VkRenderPassCreateInfo*               pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkRenderPass*                               pRenderPass) {
    layer_data *device_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    {
        bool skip = false;
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            skip |= intercept->PreCallValidateCreateRenderPass(device, pCreateInfo, pAllocator, pRenderPass);
            if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
        }
        for (auto intercept : global_interceptor_list) {
            intercept->PreCallRecordCreateRenderPass(device, pCreateInfo, pAllocator, pRenderPass);
        }
    }
    VkResult result = device_data->dispatch_table.CreateRenderPass(device, pCreateInfo, pAllocator, pRenderPass);
    if (VK_SUCCESS == result) {
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            intercept->PostCallRecordCreateRenderPass(device, pCreateInfo, pAllocator, pRenderPass);
        }
    }
    return result;
}

VKAPI_ATTR void VKAPI_CALL DestroyRenderPass(
    VkDevice                                    device,
    VkRenderPass                                renderPass,
    const VkAllocationCallbacks*                pAllocator) {
    layer_data *device_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    {
        bool skip = false;
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            skip |= intercept->PreCallValidateDestroyRenderPass(device, renderPass, pAllocator);
            if (skip) return;
        }
        for (auto intercept : global_interceptor_list) {
            intercept->PreCallRecordDestroyRenderPass(device, renderPass, pAllocator);
        }
    }
    device_data->dispatch_table.DestroyRenderPass(device, renderPass, pAllocator);
    {
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            intercept->PostCallRecordDestroyRenderPass(device, renderPass, pAllocator);
        }
    }
}

VKAPI_ATTR void VKAPI_CALL GetRenderAreaGranularity(
    VkDevice                                    device,
    VkRenderPass                                renderPass,
    VkExtent2D*                                 pGranularity) {
    layer_data *device_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    {
        bool skip = false;
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            skip |= intercept->PreCallValidateGetRenderAreaGranularity(device, renderPass, pGranularity);
            if (skip) return;
        }
        for (auto intercept : global_interceptor_list) {
            intercept->PreCallRecordGetRenderAreaGranularity(device, renderPass, pGranularity);
        }
    }
    device_data->dispatch_table.GetRenderAreaGranularity(device, renderPass, pGranularity);
    {
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            intercept->PostCallRecordGetRenderAreaGranularity(device, renderPass, pGranularity);
        }
    }
}

VKAPI_ATTR VkResult VKAPI_CALL CreateCommandPool(
    VkDevice                                    device,
    const VkCommandPoolCreateInfo*              pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkCommandPool*                              pCommandPool) {
    layer_data *device_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    {
        bool skip = false;
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            skip |= intercept->PreCallValidateCreateCommandPool(device, pCreateInfo, pAllocator, pCommandPool);
            if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
        }
        for (auto intercept : global_interceptor_list) {
            intercept->PreCallRecordCreateCommandPool(device, pCreateInfo, pAllocator, pCommandPool);
        }
    }
    VkResult result = device_data->dispatch_table.CreateCommandPool(device, pCreateInfo, pAllocator, pCommandPool);
    if (VK_SUCCESS == result) {
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            intercept->PostCallRecordCreateCommandPool(device, pCreateInfo, pAllocator, pCommandPool);
        }
    }
    return result;
}

VKAPI_ATTR void VKAPI_CALL DestroyCommandPool(
    VkDevice                                    device,
    VkCommandPool                               commandPool,
    const VkAllocationCallbacks*                pAllocator) {
    layer_data *device_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    {
        bool skip = false;
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            skip |= intercept->PreCallValidateDestroyCommandPool(device, commandPool, pAllocator);
            if (skip) return;
        }
        for (auto intercept : global_interceptor_list) {
            intercept->PreCallRecordDestroyCommandPool(device, commandPool, pAllocator);
        }
    }
    device_data->dispatch_table.DestroyCommandPool(device, commandPool, pAllocator);
    {
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            intercept->PostCallRecordDestroyCommandPool(device, commandPool, pAllocator);
        }
    }
}

VKAPI_ATTR VkResult VKAPI_CALL ResetCommandPool(
    VkDevice                                    device,
    VkCommandPool                               commandPool,
    VkCommandPoolResetFlags                     flags) {
    layer_data *device_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    {
        bool skip = false;
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            skip |= intercept->PreCallValidateResetCommandPool(device, commandPool, flags);
            if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
        }
        for (auto intercept : global_interceptor_list) {
            intercept->PreCallRecordResetCommandPool(device, commandPool, flags);
        }
    }
    VkResult result = device_data->dispatch_table.ResetCommandPool(device, commandPool, flags);
    if (VK_SUCCESS == result) {
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            intercept->PostCallRecordResetCommandPool(device, commandPool, flags);
        }
    }
    return result;
}

VKAPI_ATTR VkResult VKAPI_CALL AllocateCommandBuffers(
    VkDevice                                    device,
    const VkCommandBufferAllocateInfo*          pAllocateInfo,
    VkCommandBuffer*                            pCommandBuffers) {
    layer_data *device_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    {
        bool skip = false;
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            skip |= intercept->PreCallValidateAllocateCommandBuffers(device, pAllocateInfo, pCommandBuffers);
            if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
        }
        for (auto intercept : global_interceptor_list) {
            intercept->PreCallRecordAllocateCommandBuffers(device, pAllocateInfo, pCommandBuffers);
        }
    }
    VkResult result = device_data->dispatch_table.AllocateCommandBuffers(device, pAllocateInfo, pCommandBuffers);
    if (VK_SUCCESS == result) {
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            intercept->PostCallRecordAllocateCommandBuffers(device, pAllocateInfo, pCommandBuffers);
        }
    }
    return result;
}

VKAPI_ATTR void VKAPI_CALL FreeCommandBuffers(
    VkDevice                                    device,
    VkCommandPool                               commandPool,
    uint32_t                                    commandBufferCount,
    const VkCommandBuffer*                      pCommandBuffers) {
    layer_data *device_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    {
        bool skip = false;
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            skip |= intercept->PreCallValidateFreeCommandBuffers(device, commandPool, commandBufferCount, pCommandBuffers);
            if (skip) return;
        }
        for (auto intercept : global_interceptor_list) {
            intercept->PreCallRecordFreeCommandBuffers(device, commandPool, commandBufferCount, pCommandBuffers);
        }
    }
    device_data->dispatch_table.FreeCommandBuffers(device, commandPool, commandBufferCount, pCommandBuffers);
    {
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            intercept->PostCallRecordFreeCommandBuffers(device, commandPool, commandBufferCount, pCommandBuffers);
        }
    }
}

VKAPI_ATTR VkResult VKAPI_CALL BeginCommandBuffer(
    VkCommandBuffer                             commandBuffer,
    const VkCommandBufferBeginInfo*             pBeginInfo) {
    layer_data *device_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    {
        bool skip = false;
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            skip |= intercept->PreCallValidateBeginCommandBuffer(commandBuffer, pBeginInfo);
            if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
        }
        for (auto intercept : global_interceptor_list) {
            intercept->PreCallRecordBeginCommandBuffer(commandBuffer, pBeginInfo);
        }
    }
    VkResult result = device_data->dispatch_table.BeginCommandBuffer(commandBuffer, pBeginInfo);
    if (VK_SUCCESS == result) {
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            intercept->PostCallRecordBeginCommandBuffer(commandBuffer, pBeginInfo);
        }
    }
    return result;
}

VKAPI_ATTR VkResult VKAPI_CALL EndCommandBuffer(
    VkCommandBuffer                             commandBuffer) {
    layer_data *device_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    {
        bool skip = false;
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            skip |= intercept->PreCallValidateEndCommandBuffer(commandBuffer);
            if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
        }
        for (auto intercept : global_interceptor_list) {
            intercept->PreCallRecordEndCommandBuffer(commandBuffer);
        }
    }
    VkResult result = device_data->dispatch_table.EndCommandBuffer(commandBuffer);
    if (VK_SUCCESS == result) {
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            intercept->PostCallRecordEndCommandBuffer(commandBuffer);
        }
    }
    return result;
}

VKAPI_ATTR VkResult VKAPI_CALL ResetCommandBuffer(
    VkCommandBuffer                             commandBuffer,
    VkCommandBufferResetFlags                   flags) {
    layer_data *device_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    {
        bool skip = false;
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            skip |= intercept->PreCallValidateResetCommandBuffer(commandBuffer, flags);
            if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
        }
        for (auto intercept : global_interceptor_list) {
            intercept->PreCallRecordResetCommandBuffer(commandBuffer, flags);
        }
    }
    VkResult result = device_data->dispatch_table.ResetCommandBuffer(commandBuffer, flags);
    if (VK_SUCCESS == result) {
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            intercept->PostCallRecordResetCommandBuffer(commandBuffer, flags);
        }
    }
    return result;
}

VKAPI_ATTR void VKAPI_CALL CmdBindPipeline(
    VkCommandBuffer                             commandBuffer,
    VkPipelineBindPoint                         pipelineBindPoint,
    VkPipeline                                  pipeline) {
    layer_data *device_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    {
        bool skip = false;
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            skip |= intercept->PreCallValidateCmdBindPipeline(commandBuffer, pipelineBindPoint, pipeline);
            if (skip) return;
        }
        for (auto intercept : global_interceptor_list) {
            intercept->PreCallRecordCmdBindPipeline(commandBuffer, pipelineBindPoint, pipeline);
        }
    }
    device_data->dispatch_table.CmdBindPipeline(commandBuffer, pipelineBindPoint, pipeline);
    {
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            intercept->PostCallRecordCmdBindPipeline(commandBuffer, pipelineBindPoint, pipeline);
        }
    }
}

VKAPI_ATTR void VKAPI_CALL CmdSetViewport(
    VkCommandBuffer                             commandBuffer,
    uint32_t                                    firstViewport,
    uint32_t                                    viewportCount,
    const VkViewport*                           pViewports) {
    layer_data *device_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    {
        bool skip = false;
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            skip |= intercept->PreCallValidateCmdSetViewport(commandBuffer, firstViewport, viewportCount, pViewports);
            if (skip) return;
        }
        for (auto intercept : global_interceptor_list) {
            intercept->PreCallRecordCmdSetViewport(commandBuffer, firstViewport, viewportCount, pViewports);
        }
    }
    device_data->dispatch_table.CmdSetViewport(commandBuffer, firstViewport, viewportCount, pViewports);
    {
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            intercept->PostCallRecordCmdSetViewport(commandBuffer, firstViewport, viewportCount, pViewports);
        }
    }
}

VKAPI_ATTR void VKAPI_CALL CmdSetScissor(
    VkCommandBuffer                             commandBuffer,
    uint32_t                                    firstScissor,
    uint32_t                                    scissorCount,
    const VkRect2D*                             pScissors) {
    layer_data *device_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    {
        bool skip = false;
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            skip |= intercept->PreCallValidateCmdSetScissor(commandBuffer, firstScissor, scissorCount, pScissors);
            if (skip) return;
        }
        for (auto intercept : global_interceptor_list) {
            intercept->PreCallRecordCmdSetScissor(commandBuffer, firstScissor, scissorCount, pScissors);
        }
    }
    device_data->dispatch_table.CmdSetScissor(commandBuffer, firstScissor, scissorCount, pScissors);
    {
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            intercept->PostCallRecordCmdSetScissor(commandBuffer, firstScissor, scissorCount, pScissors);
        }
    }
}

VKAPI_ATTR void VKAPI_CALL CmdSetLineWidth(
    VkCommandBuffer                             commandBuffer,
    float                                       lineWidth) {
    layer_data *device_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    {
        bool skip = false;
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            skip |= intercept->PreCallValidateCmdSetLineWidth(commandBuffer, lineWidth);
            if (skip) return;
        }
        for (auto intercept : global_interceptor_list) {
            intercept->PreCallRecordCmdSetLineWidth(commandBuffer, lineWidth);
        }
    }
    device_data->dispatch_table.CmdSetLineWidth(commandBuffer, lineWidth);
    {
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            intercept->PostCallRecordCmdSetLineWidth(commandBuffer, lineWidth);
        }
    }
}

VKAPI_ATTR void VKAPI_CALL CmdSetDepthBias(
    VkCommandBuffer                             commandBuffer,
    float                                       depthBiasConstantFactor,
    float                                       depthBiasClamp,
    float                                       depthBiasSlopeFactor) {
    layer_data *device_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    {
        bool skip = false;
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            skip |= intercept->PreCallValidateCmdSetDepthBias(commandBuffer, depthBiasConstantFactor, depthBiasClamp, depthBiasSlopeFactor);
            if (skip) return;
        }
        for (auto intercept : global_interceptor_list) {
            intercept->PreCallRecordCmdSetDepthBias(commandBuffer, depthBiasConstantFactor, depthBiasClamp, depthBiasSlopeFactor);
        }
    }
    device_data->dispatch_table.CmdSetDepthBias(commandBuffer, depthBiasConstantFactor, depthBiasClamp, depthBiasSlopeFactor);
    {
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            intercept->PostCallRecordCmdSetDepthBias(commandBuffer, depthBiasConstantFactor, depthBiasClamp, depthBiasSlopeFactor);
        }
    }
}

VKAPI_ATTR void VKAPI_CALL CmdSetBlendConstants(
    VkCommandBuffer                             commandBuffer,
    const float                                 blendConstants[4]) {
    layer_data *device_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    {
        bool skip = false;
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            skip |= intercept->PreCallValidateCmdSetBlendConstants(commandBuffer, blendConstants);
            if (skip) return;
        }
        for (auto intercept : global_interceptor_list) {
            intercept->PreCallRecordCmdSetBlendConstants(commandBuffer, blendConstants);
        }
    }
    device_data->dispatch_table.CmdSetBlendConstants(commandBuffer, blendConstants);
    {
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            intercept->PostCallRecordCmdSetBlendConstants(commandBuffer, blendConstants);
        }
    }
}

VKAPI_ATTR void VKAPI_CALL CmdSetDepthBounds(
    VkCommandBuffer                             commandBuffer,
    float                                       minDepthBounds,
    float                                       maxDepthBounds) {
    layer_data *device_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    {
        bool skip = false;
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            skip |= intercept->PreCallValidateCmdSetDepthBounds(commandBuffer, minDepthBounds, maxDepthBounds);
            if (skip) return;
        }
        for (auto intercept : global_interceptor_list) {
            intercept->PreCallRecordCmdSetDepthBounds(commandBuffer, minDepthBounds, maxDepthBounds);
        }
    }
    device_data->dispatch_table.CmdSetDepthBounds(commandBuffer, minDepthBounds, maxDepthBounds);
    {
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            intercept->PostCallRecordCmdSetDepthBounds(commandBuffer, minDepthBounds, maxDepthBounds);
        }
    }
}

VKAPI_ATTR void VKAPI_CALL CmdSetStencilCompareMask(
    VkCommandBuffer                             commandBuffer,
    VkStencilFaceFlags                          faceMask,
    uint32_t                                    compareMask) {
    layer_data *device_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    {
        bool skip = false;
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            skip |= intercept->PreCallValidateCmdSetStencilCompareMask(commandBuffer, faceMask, compareMask);
            if (skip) return;
        }
        for (auto intercept : global_interceptor_list) {
            intercept->PreCallRecordCmdSetStencilCompareMask(commandBuffer, faceMask, compareMask);
        }
    }
    device_data->dispatch_table.CmdSetStencilCompareMask(commandBuffer, faceMask, compareMask);
    {
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            intercept->PostCallRecordCmdSetStencilCompareMask(commandBuffer, faceMask, compareMask);
        }
    }
}

VKAPI_ATTR void VKAPI_CALL CmdSetStencilWriteMask(
    VkCommandBuffer                             commandBuffer,
    VkStencilFaceFlags                          faceMask,
    uint32_t                                    writeMask) {
    layer_data *device_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    {
        bool skip = false;
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            skip |= intercept->PreCallValidateCmdSetStencilWriteMask(commandBuffer, faceMask, writeMask);
            if (skip) return;
        }
        for (auto intercept : global_interceptor_list) {
            intercept->PreCallRecordCmdSetStencilWriteMask(commandBuffer, faceMask, writeMask);
        }
    }
    device_data->dispatch_table.CmdSetStencilWriteMask(commandBuffer, faceMask, writeMask);
    {
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            intercept->PostCallRecordCmdSetStencilWriteMask(commandBuffer, faceMask, writeMask);
        }
    }
}

VKAPI_ATTR void VKAPI_CALL CmdSetStencilReference(
    VkCommandBuffer                             commandBuffer,
    VkStencilFaceFlags                          faceMask,
    uint32_t                                    reference) {
    layer_data *device_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    {
        bool skip = false;
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            skip |= intercept->PreCallValidateCmdSetStencilReference(commandBuffer, faceMask, reference);
            if (skip) return;
        }
        for (auto intercept : global_interceptor_list) {
            intercept->PreCallRecordCmdSetStencilReference(commandBuffer, faceMask, reference);
        }
    }
    device_data->dispatch_table.CmdSetStencilReference(commandBuffer, faceMask, reference);
    {
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            intercept->PostCallRecordCmdSetStencilReference(commandBuffer, faceMask, reference);
        }
    }
}

VKAPI_ATTR void VKAPI_CALL CmdBindDescriptorSets(
    VkCommandBuffer                             commandBuffer,
    VkPipelineBindPoint                         pipelineBindPoint,
    VkPipelineLayout                            layout,
    uint32_t                                    firstSet,
    uint32_t                                    descriptorSetCount,
    const VkDescriptorSet*                      pDescriptorSets,
    uint32_t                                    dynamicOffsetCount,
    const uint32_t*                             pDynamicOffsets) {
    layer_data *device_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    {
        bool skip = false;
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            skip |= intercept->PreCallValidateCmdBindDescriptorSets(commandBuffer, pipelineBindPoint, layout, firstSet, descriptorSetCount, pDescriptorSets, dynamicOffsetCount, pDynamicOffsets);
            if (skip) return;
        }
        for (auto intercept : global_interceptor_list) {
            intercept->PreCallRecordCmdBindDescriptorSets(commandBuffer, pipelineBindPoint, layout, firstSet, descriptorSetCount, pDescriptorSets, dynamicOffsetCount, pDynamicOffsets);
        }
    }
    device_data->dispatch_table.CmdBindDescriptorSets(commandBuffer, pipelineBindPoint, layout, firstSet, descriptorSetCount, pDescriptorSets, dynamicOffsetCount, pDynamicOffsets);
    {
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            intercept->PostCallRecordCmdBindDescriptorSets(commandBuffer, pipelineBindPoint, layout, firstSet, descriptorSetCount, pDescriptorSets, dynamicOffsetCount, pDynamicOffsets);
        }
    }
}

VKAPI_ATTR void VKAPI_CALL CmdBindIndexBuffer(
    VkCommandBuffer                             commandBuffer,
    VkBuffer                                    buffer,
    VkDeviceSize                                offset,
    VkIndexType                                 indexType) {
    layer_data *device_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    {
        bool skip = false;
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            skip |= intercept->PreCallValidateCmdBindIndexBuffer(commandBuffer, buffer, offset, indexType);
            if (skip) return;
        }
        for (auto intercept : global_interceptor_list) {
            intercept->PreCallRecordCmdBindIndexBuffer(commandBuffer, buffer, offset, indexType);
        }
    }
    device_data->dispatch_table.CmdBindIndexBuffer(commandBuffer, buffer, offset, indexType);
    {
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            intercept->PostCallRecordCmdBindIndexBuffer(commandBuffer, buffer, offset, indexType);
        }
    }
}

VKAPI_ATTR void VKAPI_CALL CmdBindVertexBuffers(
    VkCommandBuffer                             commandBuffer,
    uint32_t                                    firstBinding,
    uint32_t                                    bindingCount,
    const VkBuffer*                             pBuffers,
    const VkDeviceSize*                         pOffsets) {
    layer_data *device_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    {
        bool skip = false;
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            skip |= intercept->PreCallValidateCmdBindVertexBuffers(commandBuffer, firstBinding, bindingCount, pBuffers, pOffsets);
            if (skip) return;
        }
        for (auto intercept : global_interceptor_list) {
            intercept->PreCallRecordCmdBindVertexBuffers(commandBuffer, firstBinding, bindingCount, pBuffers, pOffsets);
        }
    }
    device_data->dispatch_table.CmdBindVertexBuffers(commandBuffer, firstBinding, bindingCount, pBuffers, pOffsets);
    {
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            intercept->PostCallRecordCmdBindVertexBuffers(commandBuffer, firstBinding, bindingCount, pBuffers, pOffsets);
        }
    }
}

VKAPI_ATTR void VKAPI_CALL CmdDraw(
    VkCommandBuffer                             commandBuffer,
    uint32_t                                    vertexCount,
    uint32_t                                    instanceCount,
    uint32_t                                    firstVertex,
    uint32_t                                    firstInstance) {
    layer_data *device_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    {
        bool skip = false;
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            skip |= intercept->PreCallValidateCmdDraw(commandBuffer, vertexCount, instanceCount, firstVertex, firstInstance);
            if (skip) return;
        }
        for (auto intercept : global_interceptor_list) {
            intercept->PreCallRecordCmdDraw(commandBuffer, vertexCount, instanceCount, firstVertex, firstInstance);
        }
    }
    device_data->dispatch_table.CmdDraw(commandBuffer, vertexCount, instanceCount, firstVertex, firstInstance);
    {
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            intercept->PostCallRecordCmdDraw(commandBuffer, vertexCount, instanceCount, firstVertex, firstInstance);
        }
    }
}

VKAPI_ATTR void VKAPI_CALL CmdDrawIndexed(
    VkCommandBuffer                             commandBuffer,
    uint32_t                                    indexCount,
    uint32_t                                    instanceCount,
    uint32_t                                    firstIndex,
    int32_t                                     vertexOffset,
    uint32_t                                    firstInstance) {
    layer_data *device_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    {
        bool skip = false;
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            skip |= intercept->PreCallValidateCmdDrawIndexed(commandBuffer, indexCount, instanceCount, firstIndex, vertexOffset, firstInstance);
            if (skip) return;
        }
        for (auto intercept : global_interceptor_list) {
            intercept->PreCallRecordCmdDrawIndexed(commandBuffer, indexCount, instanceCount, firstIndex, vertexOffset, firstInstance);
        }
    }
    device_data->dispatch_table.CmdDrawIndexed(commandBuffer, indexCount, instanceCount, firstIndex, vertexOffset, firstInstance);
    {
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            intercept->PostCallRecordCmdDrawIndexed(commandBuffer, indexCount, instanceCount, firstIndex, vertexOffset, firstInstance);
        }
    }
}

VKAPI_ATTR void VKAPI_CALL CmdDrawIndirect(
    VkCommandBuffer                             commandBuffer,
    VkBuffer                                    buffer,
    VkDeviceSize                                offset,
    uint32_t                                    drawCount,
    uint32_t                                    stride) {
    layer_data *device_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    {
        bool skip = false;
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            skip |= intercept->PreCallValidateCmdDrawIndirect(commandBuffer, buffer, offset, drawCount, stride);
            if (skip) return;
        }
        for (auto intercept : global_interceptor_list) {
            intercept->PreCallRecordCmdDrawIndirect(commandBuffer, buffer, offset, drawCount, stride);
        }
    }
    device_data->dispatch_table.CmdDrawIndirect(commandBuffer, buffer, offset, drawCount, stride);
    {
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            intercept->PostCallRecordCmdDrawIndirect(commandBuffer, buffer, offset, drawCount, stride);
        }
    }
}

VKAPI_ATTR void VKAPI_CALL CmdDrawIndexedIndirect(
    VkCommandBuffer                             commandBuffer,
    VkBuffer                                    buffer,
    VkDeviceSize                                offset,
    uint32_t                                    drawCount,
    uint32_t                                    stride) {
    layer_data *device_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    {
        bool skip = false;
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            skip |= intercept->PreCallValidateCmdDrawIndexedIndirect(commandBuffer, buffer, offset, drawCount, stride);
            if (skip) return;
        }
        for (auto intercept : global_interceptor_list) {
            intercept->PreCallRecordCmdDrawIndexedIndirect(commandBuffer, buffer, offset, drawCount, stride);
        }
    }
    device_data->dispatch_table.CmdDrawIndexedIndirect(commandBuffer, buffer, offset, drawCount, stride);
    {
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            intercept->PostCallRecordCmdDrawIndexedIndirect(commandBuffer, buffer, offset, drawCount, stride);
        }
    }
}

VKAPI_ATTR void VKAPI_CALL CmdDispatch(
    VkCommandBuffer                             commandBuffer,
    uint32_t                                    groupCountX,
    uint32_t                                    groupCountY,
    uint32_t                                    groupCountZ) {
    layer_data *device_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    {
        bool skip = false;
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            skip |= intercept->PreCallValidateCmdDispatch(commandBuffer, groupCountX, groupCountY, groupCountZ);
            if (skip) return;
        }
        for (auto intercept : global_interceptor_list) {
            intercept->PreCallRecordCmdDispatch(commandBuffer, groupCountX, groupCountY, groupCountZ);
        }
    }
    device_data->dispatch_table.CmdDispatch(commandBuffer, groupCountX, groupCountY, groupCountZ);
    {
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            intercept->PostCallRecordCmdDispatch(commandBuffer, groupCountX, groupCountY, groupCountZ);
        }
    }
}

VKAPI_ATTR void VKAPI_CALL CmdDispatchIndirect(
    VkCommandBuffer                             commandBuffer,
    VkBuffer                                    buffer,
    VkDeviceSize                                offset) {
    layer_data *device_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    {
        bool skip = false;
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            skip |= intercept->PreCallValidateCmdDispatchIndirect(commandBuffer, buffer, offset);
            if (skip) return;
        }
        for (auto intercept : global_interceptor_list) {
            intercept->PreCallRecordCmdDispatchIndirect(commandBuffer, buffer, offset);
        }
    }
    device_data->dispatch_table.CmdDispatchIndirect(commandBuffer, buffer, offset);
    {
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            intercept->PostCallRecordCmdDispatchIndirect(commandBuffer, buffer, offset);
        }
    }
}

VKAPI_ATTR void VKAPI_CALL CmdCopyBuffer(
    VkCommandBuffer                             commandBuffer,
    VkBuffer                                    srcBuffer,
    VkBuffer                                    dstBuffer,
    uint32_t                                    regionCount,
    const VkBufferCopy*                         pRegions) {
    layer_data *device_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    {
        bool skip = false;
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            skip |= intercept->PreCallValidateCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, regionCount, pRegions);
            if (skip) return;
        }
        for (auto intercept : global_interceptor_list) {
            intercept->PreCallRecordCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, regionCount, pRegions);
        }
    }
    device_data->dispatch_table.CmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, regionCount, pRegions);
    {
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            intercept->PostCallRecordCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, regionCount, pRegions);
        }
    }
}

VKAPI_ATTR void VKAPI_CALL CmdCopyImage(
    VkCommandBuffer                             commandBuffer,
    VkImage                                     srcImage,
    VkImageLayout                               srcImageLayout,
    VkImage                                     dstImage,
    VkImageLayout                               dstImageLayout,
    uint32_t                                    regionCount,
    const VkImageCopy*                          pRegions) {
    layer_data *device_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    {
        bool skip = false;
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            skip |= intercept->PreCallValidateCmdCopyImage(commandBuffer, srcImage, srcImageLayout, dstImage, dstImageLayout, regionCount, pRegions);
            if (skip) return;
        }
        for (auto intercept : global_interceptor_list) {
            intercept->PreCallRecordCmdCopyImage(commandBuffer, srcImage, srcImageLayout, dstImage, dstImageLayout, regionCount, pRegions);
        }
    }
    device_data->dispatch_table.CmdCopyImage(commandBuffer, srcImage, srcImageLayout, dstImage, dstImageLayout, regionCount, pRegions);
    {
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            intercept->PostCallRecordCmdCopyImage(commandBuffer, srcImage, srcImageLayout, dstImage, dstImageLayout, regionCount, pRegions);
        }
    }
}

VKAPI_ATTR void VKAPI_CALL CmdBlitImage(
    VkCommandBuffer                             commandBuffer,
    VkImage                                     srcImage,
    VkImageLayout                               srcImageLayout,
    VkImage                                     dstImage,
    VkImageLayout                               dstImageLayout,
    uint32_t                                    regionCount,
    const VkImageBlit*                          pRegions,
    VkFilter                                    filter) {
    layer_data *device_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    {
        bool skip = false;
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            skip |= intercept->PreCallValidateCmdBlitImage(commandBuffer, srcImage, srcImageLayout, dstImage, dstImageLayout, regionCount, pRegions, filter);
            if (skip) return;
        }
        for (auto intercept : global_interceptor_list) {
            intercept->PreCallRecordCmdBlitImage(commandBuffer, srcImage, srcImageLayout, dstImage, dstImageLayout, regionCount, pRegions, filter);
        }
    }
    device_data->dispatch_table.CmdBlitImage(commandBuffer, srcImage, srcImageLayout, dstImage, dstImageLayout, regionCount, pRegions, filter);
    {
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            intercept->PostCallRecordCmdBlitImage(commandBuffer, srcImage, srcImageLayout, dstImage, dstImageLayout, regionCount, pRegions, filter);
        }
    }
}

VKAPI_ATTR void VKAPI_CALL CmdCopyBufferToImage(
    VkCommandBuffer                             commandBuffer,
    VkBuffer                                    srcBuffer,
    VkImage                                     dstImage,
    VkImageLayout                               dstImageLayout,
    uint32_t                                    regionCount,
    const VkBufferImageCopy*                    pRegions) {
    layer_data *device_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    {
        bool skip = false;
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            skip |= intercept->PreCallValidateCmdCopyBufferToImage(commandBuffer, srcBuffer, dstImage, dstImageLayout, regionCount, pRegions);
            if (skip) return;
        }
        for (auto intercept : global_interceptor_list) {
            intercept->PreCallRecordCmdCopyBufferToImage(commandBuffer, srcBuffer, dstImage, dstImageLayout, regionCount, pRegions);
        }
    }
    device_data->dispatch_table.CmdCopyBufferToImage(commandBuffer, srcBuffer, dstImage, dstImageLayout, regionCount, pRegions);
    {
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            intercept->PostCallRecordCmdCopyBufferToImage(commandBuffer, srcBuffer, dstImage, dstImageLayout, regionCount, pRegions);
        }
    }
}

VKAPI_ATTR void VKAPI_CALL CmdCopyImageToBuffer(
    VkCommandBuffer                             commandBuffer,
    VkImage                                     srcImage,
    VkImageLayout                               srcImageLayout,
    VkBuffer                                    dstBuffer,
    uint32_t                                    regionCount,
    const VkBufferImageCopy*                    pRegions) {
    layer_data *device_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    {
        bool skip = false;
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            skip |= intercept->PreCallValidateCmdCopyImageToBuffer(commandBuffer, srcImage, srcImageLayout, dstBuffer, regionCount, pRegions);
            if (skip) return;
        }
        for (auto intercept : global_interceptor_list) {
            intercept->PreCallRecordCmdCopyImageToBuffer(commandBuffer, srcImage, srcImageLayout, dstBuffer, regionCount, pRegions);
        }
    }
    device_data->dispatch_table.CmdCopyImageToBuffer(commandBuffer, srcImage, srcImageLayout, dstBuffer, regionCount, pRegions);
    {
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            intercept->PostCallRecordCmdCopyImageToBuffer(commandBuffer, srcImage, srcImageLayout, dstBuffer, regionCount, pRegions);
        }
    }
}

VKAPI_ATTR void VKAPI_CALL CmdUpdateBuffer(
    VkCommandBuffer                             commandBuffer,
    VkBuffer                                    dstBuffer,
    VkDeviceSize                                dstOffset,
    VkDeviceSize                                dataSize,
    const void*                                 pData) {
    layer_data *device_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    {
        bool skip = false;
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            skip |= intercept->PreCallValidateCmdUpdateBuffer(commandBuffer, dstBuffer, dstOffset, dataSize, pData);
            if (skip) return;
        }
        for (auto intercept : global_interceptor_list) {
            intercept->PreCallRecordCmdUpdateBuffer(commandBuffer, dstBuffer, dstOffset, dataSize, pData);
        }
    }
    device_data->dispatch_table.CmdUpdateBuffer(commandBuffer, dstBuffer, dstOffset, dataSize, pData);
    {
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            intercept->PostCallRecordCmdUpdateBuffer(commandBuffer, dstBuffer, dstOffset, dataSize, pData);
        }
    }
}

VKAPI_ATTR void VKAPI_CALL CmdFillBuffer(
    VkCommandBuffer                             commandBuffer,
    VkBuffer                                    dstBuffer,
    VkDeviceSize                                dstOffset,
    VkDeviceSize                                size,
    uint32_t                                    data) {
    layer_data *device_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    {
        bool skip = false;
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            skip |= intercept->PreCallValidateCmdFillBuffer(commandBuffer, dstBuffer, dstOffset, size, data);
            if (skip) return;
        }
        for (auto intercept : global_interceptor_list) {
            intercept->PreCallRecordCmdFillBuffer(commandBuffer, dstBuffer, dstOffset, size, data);
        }
    }
    device_data->dispatch_table.CmdFillBuffer(commandBuffer, dstBuffer, dstOffset, size, data);
    {
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            intercept->PostCallRecordCmdFillBuffer(commandBuffer, dstBuffer, dstOffset, size, data);
        }
    }
}

VKAPI_ATTR void VKAPI_CALL CmdClearColorImage(
    VkCommandBuffer                             commandBuffer,
    VkImage                                     image,
    VkImageLayout                               imageLayout,
    const VkClearColorValue*                    pColor,
    uint32_t                                    rangeCount,
    const VkImageSubresourceRange*              pRanges) {
    layer_data *device_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    {
        bool skip = false;
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            skip |= intercept->PreCallValidateCmdClearColorImage(commandBuffer, image, imageLayout, pColor, rangeCount, pRanges);
            if (skip) return;
        }
        for (auto intercept : global_interceptor_list) {
            intercept->PreCallRecordCmdClearColorImage(commandBuffer, image, imageLayout, pColor, rangeCount, pRanges);
        }
    }
    device_data->dispatch_table.CmdClearColorImage(commandBuffer, image, imageLayout, pColor, rangeCount, pRanges);
    {
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            intercept->PostCallRecordCmdClearColorImage(commandBuffer, image, imageLayout, pColor, rangeCount, pRanges);
        }
    }
}

VKAPI_ATTR void VKAPI_CALL CmdClearDepthStencilImage(
    VkCommandBuffer                             commandBuffer,
    VkImage                                     image,
    VkImageLayout                               imageLayout,
    const VkClearDepthStencilValue*             pDepthStencil,
    uint32_t                                    rangeCount,
    const VkImageSubresourceRange*              pRanges) {
    layer_data *device_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    {
        bool skip = false;
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            skip |= intercept->PreCallValidateCmdClearDepthStencilImage(commandBuffer, image, imageLayout, pDepthStencil, rangeCount, pRanges);
            if (skip) return;
        }
        for (auto intercept : global_interceptor_list) {
            intercept->PreCallRecordCmdClearDepthStencilImage(commandBuffer, image, imageLayout, pDepthStencil, rangeCount, pRanges);
        }
    }
    device_data->dispatch_table.CmdClearDepthStencilImage(commandBuffer, image, imageLayout, pDepthStencil, rangeCount, pRanges);
    {
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            intercept->PostCallRecordCmdClearDepthStencilImage(commandBuffer, image, imageLayout, pDepthStencil, rangeCount, pRanges);
        }
    }
}

VKAPI_ATTR void VKAPI_CALL CmdClearAttachments(
    VkCommandBuffer                             commandBuffer,
    uint32_t                                    attachmentCount,
    const VkClearAttachment*                    pAttachments,
    uint32_t                                    rectCount,
    const VkClearRect*                          pRects) {
    layer_data *device_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    {
        bool skip = false;
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            skip |= intercept->PreCallValidateCmdClearAttachments(commandBuffer, attachmentCount, pAttachments, rectCount, pRects);
            if (skip) return;
        }
        for (auto intercept : global_interceptor_list) {
            intercept->PreCallRecordCmdClearAttachments(commandBuffer, attachmentCount, pAttachments, rectCount, pRects);
        }
    }
    device_data->dispatch_table.CmdClearAttachments(commandBuffer, attachmentCount, pAttachments, rectCount, pRects);
    {
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            intercept->PostCallRecordCmdClearAttachments(commandBuffer, attachmentCount, pAttachments, rectCount, pRects);
        }
    }
}

VKAPI_ATTR void VKAPI_CALL CmdResolveImage(
    VkCommandBuffer                             commandBuffer,
    VkImage                                     srcImage,
    VkImageLayout                               srcImageLayout,
    VkImage                                     dstImage,
    VkImageLayout                               dstImageLayout,
    uint32_t                                    regionCount,
    const VkImageResolve*                       pRegions) {
    layer_data *device_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    {
        bool skip = false;
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            skip |= intercept->PreCallValidateCmdResolveImage(commandBuffer, srcImage, srcImageLayout, dstImage, dstImageLayout, regionCount, pRegions);
            if (skip) return;
        }
        for (auto intercept : global_interceptor_list) {
            intercept->PreCallRecordCmdResolveImage(commandBuffer, srcImage, srcImageLayout, dstImage, dstImageLayout, regionCount, pRegions);
        }
    }
    device_data->dispatch_table.CmdResolveImage(commandBuffer, srcImage, srcImageLayout, dstImage, dstImageLayout, regionCount, pRegions);
    {
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            intercept->PostCallRecordCmdResolveImage(commandBuffer, srcImage, srcImageLayout, dstImage, dstImageLayout, regionCount, pRegions);
        }
    }
}

VKAPI_ATTR void VKAPI_CALL CmdSetEvent(
    VkCommandBuffer                             commandBuffer,
    VkEvent                                     event,
    VkPipelineStageFlags                        stageMask) {
    layer_data *device_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    {
        bool skip = false;
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            skip |= intercept->PreCallValidateCmdSetEvent(commandBuffer, event, stageMask);
            if (skip) return;
        }
        for (auto intercept : global_interceptor_list) {
            intercept->PreCallRecordCmdSetEvent(commandBuffer, event, stageMask);
        }
    }
    device_data->dispatch_table.CmdSetEvent(commandBuffer, event, stageMask);
    {
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            intercept->PostCallRecordCmdSetEvent(commandBuffer, event, stageMask);
        }
    }
}

VKAPI_ATTR void VKAPI_CALL CmdResetEvent(
    VkCommandBuffer                             commandBuffer,
    VkEvent                                     event,
    VkPipelineStageFlags                        stageMask) {
    layer_data *device_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    {
        bool skip = false;
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            skip |= intercept->PreCallValidateCmdResetEvent(commandBuffer, event, stageMask);
            if (skip) return;
        }
        for (auto intercept : global_interceptor_list) {
            intercept->PreCallRecordCmdResetEvent(commandBuffer, event, stageMask);
        }
    }
    device_data->dispatch_table.CmdResetEvent(commandBuffer, event, stageMask);
    {
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            intercept->PostCallRecordCmdResetEvent(commandBuffer, event, stageMask);
        }
    }
}

VKAPI_ATTR void VKAPI_CALL CmdWaitEvents(
    VkCommandBuffer                             commandBuffer,
    uint32_t                                    eventCount,
    const VkEvent*                              pEvents,
    VkPipelineStageFlags                        srcStageMask,
    VkPipelineStageFlags                        dstStageMask,
    uint32_t                                    memoryBarrierCount,
    const VkMemoryBarrier*                      pMemoryBarriers,
    uint32_t                                    bufferMemoryBarrierCount,
    const VkBufferMemoryBarrier*                pBufferMemoryBarriers,
    uint32_t                                    imageMemoryBarrierCount,
    const VkImageMemoryBarrier*                 pImageMemoryBarriers) {
    layer_data *device_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    {
        bool skip = false;
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            skip |= intercept->PreCallValidateCmdWaitEvents(commandBuffer, eventCount, pEvents, srcStageMask, dstStageMask, memoryBarrierCount, pMemoryBarriers, bufferMemoryBarrierCount, pBufferMemoryBarriers, imageMemoryBarrierCount, pImageMemoryBarriers);
            if (skip) return;
        }
        for (auto intercept : global_interceptor_list) {
            intercept->PreCallRecordCmdWaitEvents(commandBuffer, eventCount, pEvents, srcStageMask, dstStageMask, memoryBarrierCount, pMemoryBarriers, bufferMemoryBarrierCount, pBufferMemoryBarriers, imageMemoryBarrierCount, pImageMemoryBarriers);
        }
    }
    device_data->dispatch_table.CmdWaitEvents(commandBuffer, eventCount, pEvents, srcStageMask, dstStageMask, memoryBarrierCount, pMemoryBarriers, bufferMemoryBarrierCount, pBufferMemoryBarriers, imageMemoryBarrierCount, pImageMemoryBarriers);
    {
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            intercept->PostCallRecordCmdWaitEvents(commandBuffer, eventCount, pEvents, srcStageMask, dstStageMask, memoryBarrierCount, pMemoryBarriers, bufferMemoryBarrierCount, pBufferMemoryBarriers, imageMemoryBarrierCount, pImageMemoryBarriers);
        }
    }
}

VKAPI_ATTR void VKAPI_CALL CmdPipelineBarrier(
    VkCommandBuffer                             commandBuffer,
    VkPipelineStageFlags                        srcStageMask,
    VkPipelineStageFlags                        dstStageMask,
    VkDependencyFlags                           dependencyFlags,
    uint32_t                                    memoryBarrierCount,
    const VkMemoryBarrier*                      pMemoryBarriers,
    uint32_t                                    bufferMemoryBarrierCount,
    const VkBufferMemoryBarrier*                pBufferMemoryBarriers,
    uint32_t                                    imageMemoryBarrierCount,
    const VkImageMemoryBarrier*                 pImageMemoryBarriers) {
    layer_data *device_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    {
        bool skip = false;
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            skip |= intercept->PreCallValidateCmdPipelineBarrier(commandBuffer, srcStageMask, dstStageMask, dependencyFlags, memoryBarrierCount, pMemoryBarriers, bufferMemoryBarrierCount, pBufferMemoryBarriers, imageMemoryBarrierCount, pImageMemoryBarriers);
            if (skip) return;
        }
        for (auto intercept : global_interceptor_list) {
            intercept->PreCallRecordCmdPipelineBarrier(commandBuffer, srcStageMask, dstStageMask, dependencyFlags, memoryBarrierCount, pMemoryBarriers, bufferMemoryBarrierCount, pBufferMemoryBarriers, imageMemoryBarrierCount, pImageMemoryBarriers);
        }
    }
    device_data->dispatch_table.CmdPipelineBarrier(commandBuffer, srcStageMask, dstStageMask, dependencyFlags, memoryBarrierCount, pMemoryBarriers, bufferMemoryBarrierCount, pBufferMemoryBarriers, imageMemoryBarrierCount, pImageMemoryBarriers);
    {
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            intercept->PostCallRecordCmdPipelineBarrier(commandBuffer, srcStageMask, dstStageMask, dependencyFlags, memoryBarrierCount, pMemoryBarriers, bufferMemoryBarrierCount, pBufferMemoryBarriers, imageMemoryBarrierCount, pImageMemoryBarriers);
        }
    }
}

VKAPI_ATTR void VKAPI_CALL CmdBeginQuery(
    VkCommandBuffer                             commandBuffer,
    VkQueryPool                                 queryPool,
    uint32_t                                    query,
    VkQueryControlFlags                         flags) {
    layer_data *device_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    {
        bool skip = false;
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            skip |= intercept->PreCallValidateCmdBeginQuery(commandBuffer, queryPool, query, flags);
            if (skip) return;
        }
        for (auto intercept : global_interceptor_list) {
            intercept->PreCallRecordCmdBeginQuery(commandBuffer, queryPool, query, flags);
        }
    }
    device_data->dispatch_table.CmdBeginQuery(commandBuffer, queryPool, query, flags);
    {
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            intercept->PostCallRecordCmdBeginQuery(commandBuffer, queryPool, query, flags);
        }
    }
}

VKAPI_ATTR void VKAPI_CALL CmdEndQuery(
    VkCommandBuffer                             commandBuffer,
    VkQueryPool                                 queryPool,
    uint32_t                                    query) {
    layer_data *device_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    {
        bool skip = false;
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            skip |= intercept->PreCallValidateCmdEndQuery(commandBuffer, queryPool, query);
            if (skip) return;
        }
        for (auto intercept : global_interceptor_list) {
            intercept->PreCallRecordCmdEndQuery(commandBuffer, queryPool, query);
        }
    }
    device_data->dispatch_table.CmdEndQuery(commandBuffer, queryPool, query);
    {
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            intercept->PostCallRecordCmdEndQuery(commandBuffer, queryPool, query);
        }
    }
}

VKAPI_ATTR void VKAPI_CALL CmdResetQueryPool(
    VkCommandBuffer                             commandBuffer,
    VkQueryPool                                 queryPool,
    uint32_t                                    firstQuery,
    uint32_t                                    queryCount) {
    layer_data *device_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    {
        bool skip = false;
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            skip |= intercept->PreCallValidateCmdResetQueryPool(commandBuffer, queryPool, firstQuery, queryCount);
            if (skip) return;
        }
        for (auto intercept : global_interceptor_list) {
            intercept->PreCallRecordCmdResetQueryPool(commandBuffer, queryPool, firstQuery, queryCount);
        }
    }
    device_data->dispatch_table.CmdResetQueryPool(commandBuffer, queryPool, firstQuery, queryCount);
    {
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            intercept->PostCallRecordCmdResetQueryPool(commandBuffer, queryPool, firstQuery, queryCount);
        }
    }
}

VKAPI_ATTR void VKAPI_CALL CmdWriteTimestamp(
    VkCommandBuffer                             commandBuffer,
    VkPipelineStageFlagBits                     pipelineStage,
    VkQueryPool                                 queryPool,
    uint32_t                                    query) {
    layer_data *device_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    {
        bool skip = false;
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            skip |= intercept->PreCallValidateCmdWriteTimestamp(commandBuffer, pipelineStage, queryPool, query);
            if (skip) return;
        }
        for (auto intercept : global_interceptor_list) {
            intercept->PreCallRecordCmdWriteTimestamp(commandBuffer, pipelineStage, queryPool, query);
        }
    }
    device_data->dispatch_table.CmdWriteTimestamp(commandBuffer, pipelineStage, queryPool, query);
    {
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            intercept->PostCallRecordCmdWriteTimestamp(commandBuffer, pipelineStage, queryPool, query);
        }
    }
}

VKAPI_ATTR void VKAPI_CALL CmdCopyQueryPoolResults(
    VkCommandBuffer                             commandBuffer,
    VkQueryPool                                 queryPool,
    uint32_t                                    firstQuery,
    uint32_t                                    queryCount,
    VkBuffer                                    dstBuffer,
    VkDeviceSize                                dstOffset,
    VkDeviceSize                                stride,
    VkQueryResultFlags                          flags) {
    layer_data *device_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    {
        bool skip = false;
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            skip |= intercept->PreCallValidateCmdCopyQueryPoolResults(commandBuffer, queryPool, firstQuery, queryCount, dstBuffer, dstOffset, stride, flags);
            if (skip) return;
        }
        for (auto intercept : global_interceptor_list) {
            intercept->PreCallRecordCmdCopyQueryPoolResults(commandBuffer, queryPool, firstQuery, queryCount, dstBuffer, dstOffset, stride, flags);
        }
    }
    device_data->dispatch_table.CmdCopyQueryPoolResults(commandBuffer, queryPool, firstQuery, queryCount, dstBuffer, dstOffset, stride, flags);
    {
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            intercept->PostCallRecordCmdCopyQueryPoolResults(commandBuffer, queryPool, firstQuery, queryCount, dstBuffer, dstOffset, stride, flags);
        }
    }
}

VKAPI_ATTR void VKAPI_CALL CmdPushConstants(
    VkCommandBuffer                             commandBuffer,
    VkPipelineLayout                            layout,
    VkShaderStageFlags                          stageFlags,
    uint32_t                                    offset,
    uint32_t                                    size,
    const void*                                 pValues) {
    layer_data *device_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    {
        bool skip = false;
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            skip |= intercept->PreCallValidateCmdPushConstants(commandBuffer, layout, stageFlags, offset, size, pValues);
            if (skip) return;
        }
        for (auto intercept : global_interceptor_list) {
            intercept->PreCallRecordCmdPushConstants(commandBuffer, layout, stageFlags, offset, size, pValues);
        }
    }
    device_data->dispatch_table.CmdPushConstants(commandBuffer, layout, stageFlags, offset, size, pValues);
    {
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            intercept->PostCallRecordCmdPushConstants(commandBuffer, layout, stageFlags, offset, size, pValues);
        }
    }
}

VKAPI_ATTR void VKAPI_CALL CmdBeginRenderPass(
    VkCommandBuffer                             commandBuffer,
    const VkRenderPassBeginInfo*                pRenderPassBegin,
    VkSubpassContents                           contents) {
    layer_data *device_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    {
        bool skip = false;
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            skip |= intercept->PreCallValidateCmdBeginRenderPass(commandBuffer, pRenderPassBegin, contents);
            if (skip) return;
        }
        for (auto intercept : global_interceptor_list) {
            intercept->PreCallRecordCmdBeginRenderPass(commandBuffer, pRenderPassBegin, contents);
        }
    }
    device_data->dispatch_table.CmdBeginRenderPass(commandBuffer, pRenderPassBegin, contents);
    {
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            intercept->PostCallRecordCmdBeginRenderPass(commandBuffer, pRenderPassBegin, contents);
        }
    }
}

VKAPI_ATTR void VKAPI_CALL CmdNextSubpass(
    VkCommandBuffer                             commandBuffer,
    VkSubpassContents                           contents) {
    layer_data *device_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    {
        bool skip = false;
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            skip |= intercept->PreCallValidateCmdNextSubpass(commandBuffer, contents);
            if (skip) return;
        }
        for (auto intercept : global_interceptor_list) {
            intercept->PreCallRecordCmdNextSubpass(commandBuffer, contents);
        }
    }
    device_data->dispatch_table.CmdNextSubpass(commandBuffer, contents);
    {
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            intercept->PostCallRecordCmdNextSubpass(commandBuffer, contents);
        }
    }
}

VKAPI_ATTR void VKAPI_CALL CmdEndRenderPass(
    VkCommandBuffer                             commandBuffer) {
    layer_data *device_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    {
        bool skip = false;
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            skip |= intercept->PreCallValidateCmdEndRenderPass(commandBuffer);
            if (skip) return;
        }
        for (auto intercept : global_interceptor_list) {
            intercept->PreCallRecordCmdEndRenderPass(commandBuffer);
        }
    }
    device_data->dispatch_table.CmdEndRenderPass(commandBuffer);
    {
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            intercept->PostCallRecordCmdEndRenderPass(commandBuffer);
        }
    }
}

VKAPI_ATTR void VKAPI_CALL CmdExecuteCommands(
    VkCommandBuffer                             commandBuffer,
    uint32_t                                    commandBufferCount,
    const VkCommandBuffer*                      pCommandBuffers) {
    layer_data *device_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    {
        bool skip = false;
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            skip |= intercept->PreCallValidateCmdExecuteCommands(commandBuffer, commandBufferCount, pCommandBuffers);
            if (skip) return;
        }
        for (auto intercept : global_interceptor_list) {
            intercept->PreCallRecordCmdExecuteCommands(commandBuffer, commandBufferCount, pCommandBuffers);
        }
    }
    device_data->dispatch_table.CmdExecuteCommands(commandBuffer, commandBufferCount, pCommandBuffers);
    {
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            intercept->PostCallRecordCmdExecuteCommands(commandBuffer, commandBufferCount, pCommandBuffers);
        }
    }
}


VKAPI_ATTR VkResult VKAPI_CALL BindBufferMemory2(
    VkDevice                                    device,
    uint32_t                                    bindInfoCount,
    const VkBindBufferMemoryInfo*               pBindInfos) {
    layer_data *device_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    {
        bool skip = false;
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            skip |= intercept->PreCallValidateBindBufferMemory2(device, bindInfoCount, pBindInfos);
            if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
        }
        for (auto intercept : global_interceptor_list) {
            intercept->PreCallRecordBindBufferMemory2(device, bindInfoCount, pBindInfos);
        }
    }
    VkResult result = device_data->dispatch_table.BindBufferMemory2(device, bindInfoCount, pBindInfos);
    if (VK_SUCCESS == result) {
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            intercept->PostCallRecordBindBufferMemory2(device, bindInfoCount, pBindInfos);
        }
    }
    return result;
}

VKAPI_ATTR VkResult VKAPI_CALL BindImageMemory2(
    VkDevice                                    device,
    uint32_t                                    bindInfoCount,
    const VkBindImageMemoryInfo*                pBindInfos) {
    layer_data *device_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    {
        bool skip = false;
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            skip |= intercept->PreCallValidateBindImageMemory2(device, bindInfoCount, pBindInfos);
            if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
        }
        for (auto intercept : global_interceptor_list) {
            intercept->PreCallRecordBindImageMemory2(device, bindInfoCount, pBindInfos);
        }
    }
    VkResult result = device_data->dispatch_table.BindImageMemory2(device, bindInfoCount, pBindInfos);
    if (VK_SUCCESS == result) {
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            intercept->PostCallRecordBindImageMemory2(device, bindInfoCount, pBindInfos);
        }
    }
    return result;
}

VKAPI_ATTR void VKAPI_CALL GetDeviceGroupPeerMemoryFeatures(
    VkDevice                                    device,
    uint32_t                                    heapIndex,
    uint32_t                                    localDeviceIndex,
    uint32_t                                    remoteDeviceIndex,
    VkPeerMemoryFeatureFlags*                   pPeerMemoryFeatures) {
    layer_data *device_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    {
        bool skip = false;
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            skip |= intercept->PreCallValidateGetDeviceGroupPeerMemoryFeatures(device, heapIndex, localDeviceIndex, remoteDeviceIndex, pPeerMemoryFeatures);
            if (skip) return;
        }
        for (auto intercept : global_interceptor_list) {
            intercept->PreCallRecordGetDeviceGroupPeerMemoryFeatures(device, heapIndex, localDeviceIndex, remoteDeviceIndex, pPeerMemoryFeatures);
        }
    }
    device_data->dispatch_table.GetDeviceGroupPeerMemoryFeatures(device, heapIndex, localDeviceIndex, remoteDeviceIndex, pPeerMemoryFeatures);
    {
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            intercept->PostCallRecordGetDeviceGroupPeerMemoryFeatures(device, heapIndex, localDeviceIndex, remoteDeviceIndex, pPeerMemoryFeatures);
        }
    }
}

VKAPI_ATTR void VKAPI_CALL CmdSetDeviceMask(
    VkCommandBuffer                             commandBuffer,
    uint32_t                                    deviceMask) {
    layer_data *device_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    {
        bool skip = false;
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            skip |= intercept->PreCallValidateCmdSetDeviceMask(commandBuffer, deviceMask);
            if (skip) return;
        }
        for (auto intercept : global_interceptor_list) {
            intercept->PreCallRecordCmdSetDeviceMask(commandBuffer, deviceMask);
        }
    }
    device_data->dispatch_table.CmdSetDeviceMask(commandBuffer, deviceMask);
    {
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            intercept->PostCallRecordCmdSetDeviceMask(commandBuffer, deviceMask);
        }
    }
}

VKAPI_ATTR void VKAPI_CALL CmdDispatchBase(
    VkCommandBuffer                             commandBuffer,
    uint32_t                                    baseGroupX,
    uint32_t                                    baseGroupY,
    uint32_t                                    baseGroupZ,
    uint32_t                                    groupCountX,
    uint32_t                                    groupCountY,
    uint32_t                                    groupCountZ) {
    layer_data *device_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    {
        bool skip = false;
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            skip |= intercept->PreCallValidateCmdDispatchBase(commandBuffer, baseGroupX, baseGroupY, baseGroupZ, groupCountX, groupCountY, groupCountZ);
            if (skip) return;
        }
        for (auto intercept : global_interceptor_list) {
            intercept->PreCallRecordCmdDispatchBase(commandBuffer, baseGroupX, baseGroupY, baseGroupZ, groupCountX, groupCountY, groupCountZ);
        }
    }
    device_data->dispatch_table.CmdDispatchBase(commandBuffer, baseGroupX, baseGroupY, baseGroupZ, groupCountX, groupCountY, groupCountZ);
    {
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            intercept->PostCallRecordCmdDispatchBase(commandBuffer, baseGroupX, baseGroupY, baseGroupZ, groupCountX, groupCountY, groupCountZ);
        }
    }
}

VKAPI_ATTR VkResult VKAPI_CALL EnumeratePhysicalDeviceGroups(
    VkInstance                                  instance,
    uint32_t*                                   pPhysicalDeviceGroupCount,
    VkPhysicalDeviceGroupProperties*            pPhysicalDeviceGroupProperties) {
    instance_layer_data *instance_data = GetLayerDataPtr(get_dispatch_key(instance), instance_layer_data_map);
    {
        bool skip = false;
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            skip |= intercept->PreCallValidateEnumeratePhysicalDeviceGroups(instance, pPhysicalDeviceGroupCount, pPhysicalDeviceGroupProperties);
            if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
        }
        for (auto intercept : global_interceptor_list) {
            intercept->PreCallRecordEnumeratePhysicalDeviceGroups(instance, pPhysicalDeviceGroupCount, pPhysicalDeviceGroupProperties);
        }
    }
    VkResult result = instance_data->dispatch_table.EnumeratePhysicalDeviceGroups(instance, pPhysicalDeviceGroupCount, pPhysicalDeviceGroupProperties);
    if (VK_SUCCESS == result) {
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            intercept->PostCallRecordEnumeratePhysicalDeviceGroups(instance, pPhysicalDeviceGroupCount, pPhysicalDeviceGroupProperties);
        }
    }
    return result;
}

VKAPI_ATTR void VKAPI_CALL GetImageMemoryRequirements2(
    VkDevice                                    device,
    const VkImageMemoryRequirementsInfo2*       pInfo,
    VkMemoryRequirements2*                      pMemoryRequirements) {
    layer_data *device_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    {
        bool skip = false;
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            skip |= intercept->PreCallValidateGetImageMemoryRequirements2(device, pInfo, pMemoryRequirements);
            if (skip) return;
        }
        for (auto intercept : global_interceptor_list) {
            intercept->PreCallRecordGetImageMemoryRequirements2(device, pInfo, pMemoryRequirements);
        }
    }
    device_data->dispatch_table.GetImageMemoryRequirements2(device, pInfo, pMemoryRequirements);
    {
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            intercept->PostCallRecordGetImageMemoryRequirements2(device, pInfo, pMemoryRequirements);
        }
    }
}

VKAPI_ATTR void VKAPI_CALL GetBufferMemoryRequirements2(
    VkDevice                                    device,
    const VkBufferMemoryRequirementsInfo2*      pInfo,
    VkMemoryRequirements2*                      pMemoryRequirements) {
    layer_data *device_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    {
        bool skip = false;
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            skip |= intercept->PreCallValidateGetBufferMemoryRequirements2(device, pInfo, pMemoryRequirements);
            if (skip) return;
        }
        for (auto intercept : global_interceptor_list) {
            intercept->PreCallRecordGetBufferMemoryRequirements2(device, pInfo, pMemoryRequirements);
        }
    }
    device_data->dispatch_table.GetBufferMemoryRequirements2(device, pInfo, pMemoryRequirements);
    {
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            intercept->PostCallRecordGetBufferMemoryRequirements2(device, pInfo, pMemoryRequirements);
        }
    }
}

VKAPI_ATTR void VKAPI_CALL GetImageSparseMemoryRequirements2(
    VkDevice                                    device,
    const VkImageSparseMemoryRequirementsInfo2* pInfo,
    uint32_t*                                   pSparseMemoryRequirementCount,
    VkSparseImageMemoryRequirements2*           pSparseMemoryRequirements) {
    layer_data *device_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    {
        bool skip = false;
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            skip |= intercept->PreCallValidateGetImageSparseMemoryRequirements2(device, pInfo, pSparseMemoryRequirementCount, pSparseMemoryRequirements);
            if (skip) return;
        }
        for (auto intercept : global_interceptor_list) {
            intercept->PreCallRecordGetImageSparseMemoryRequirements2(device, pInfo, pSparseMemoryRequirementCount, pSparseMemoryRequirements);
        }
    }
    device_data->dispatch_table.GetImageSparseMemoryRequirements2(device, pInfo, pSparseMemoryRequirementCount, pSparseMemoryRequirements);
    {
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            intercept->PostCallRecordGetImageSparseMemoryRequirements2(device, pInfo, pSparseMemoryRequirementCount, pSparseMemoryRequirements);
        }
    }
}

VKAPI_ATTR void VKAPI_CALL GetPhysicalDeviceFeatures2(
    VkPhysicalDevice                            physicalDevice,
    VkPhysicalDeviceFeatures2*                  pFeatures) {
    instance_layer_data *instance_data = GetLayerDataPtr(get_dispatch_key(physicalDevice), instance_layer_data_map);
    {
        bool skip = false;
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            skip |= intercept->PreCallValidateGetPhysicalDeviceFeatures2(physicalDevice, pFeatures);
            if (skip) return;
        }
        for (auto intercept : global_interceptor_list) {
            intercept->PreCallRecordGetPhysicalDeviceFeatures2(physicalDevice, pFeatures);
        }
    }
    instance_data->dispatch_table.GetPhysicalDeviceFeatures2(physicalDevice, pFeatures);
    {
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            intercept->PostCallRecordGetPhysicalDeviceFeatures2(physicalDevice, pFeatures);
        }
    }
}

VKAPI_ATTR void VKAPI_CALL GetPhysicalDeviceProperties2(
    VkPhysicalDevice                            physicalDevice,
    VkPhysicalDeviceProperties2*                pProperties) {
    instance_layer_data *instance_data = GetLayerDataPtr(get_dispatch_key(physicalDevice), instance_layer_data_map);
    {
        bool skip = false;
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            skip |= intercept->PreCallValidateGetPhysicalDeviceProperties2(physicalDevice, pProperties);
            if (skip) return;
        }
        for (auto intercept : global_interceptor_list) {
            intercept->PreCallRecordGetPhysicalDeviceProperties2(physicalDevice, pProperties);
        }
    }
    instance_data->dispatch_table.GetPhysicalDeviceProperties2(physicalDevice, pProperties);
    {
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            intercept->PostCallRecordGetPhysicalDeviceProperties2(physicalDevice, pProperties);
        }
    }
}

VKAPI_ATTR void VKAPI_CALL GetPhysicalDeviceFormatProperties2(
    VkPhysicalDevice                            physicalDevice,
    VkFormat                                    format,
    VkFormatProperties2*                        pFormatProperties) {
    instance_layer_data *instance_data = GetLayerDataPtr(get_dispatch_key(physicalDevice), instance_layer_data_map);
    {
        bool skip = false;
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            skip |= intercept->PreCallValidateGetPhysicalDeviceFormatProperties2(physicalDevice, format, pFormatProperties);
            if (skip) return;
        }
        for (auto intercept : global_interceptor_list) {
            intercept->PreCallRecordGetPhysicalDeviceFormatProperties2(physicalDevice, format, pFormatProperties);
        }
    }
    instance_data->dispatch_table.GetPhysicalDeviceFormatProperties2(physicalDevice, format, pFormatProperties);
    {
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            intercept->PostCallRecordGetPhysicalDeviceFormatProperties2(physicalDevice, format, pFormatProperties);
        }
    }
}

VKAPI_ATTR VkResult VKAPI_CALL GetPhysicalDeviceImageFormatProperties2(
    VkPhysicalDevice                            physicalDevice,
    const VkPhysicalDeviceImageFormatInfo2*     pImageFormatInfo,
    VkImageFormatProperties2*                   pImageFormatProperties) {
    instance_layer_data *instance_data = GetLayerDataPtr(get_dispatch_key(physicalDevice), instance_layer_data_map);
    {
        bool skip = false;
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            skip |= intercept->PreCallValidateGetPhysicalDeviceImageFormatProperties2(physicalDevice, pImageFormatInfo, pImageFormatProperties);
            if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
        }
        for (auto intercept : global_interceptor_list) {
            intercept->PreCallRecordGetPhysicalDeviceImageFormatProperties2(physicalDevice, pImageFormatInfo, pImageFormatProperties);
        }
    }
    VkResult result = instance_data->dispatch_table.GetPhysicalDeviceImageFormatProperties2(physicalDevice, pImageFormatInfo, pImageFormatProperties);
    if (VK_SUCCESS == result) {
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            intercept->PostCallRecordGetPhysicalDeviceImageFormatProperties2(physicalDevice, pImageFormatInfo, pImageFormatProperties);
        }
    }
    return result;
}

VKAPI_ATTR void VKAPI_CALL GetPhysicalDeviceQueueFamilyProperties2(
    VkPhysicalDevice                            physicalDevice,
    uint32_t*                                   pQueueFamilyPropertyCount,
    VkQueueFamilyProperties2*                   pQueueFamilyProperties) {
    instance_layer_data *instance_data = GetLayerDataPtr(get_dispatch_key(physicalDevice), instance_layer_data_map);
    {
        bool skip = false;
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            skip |= intercept->PreCallValidateGetPhysicalDeviceQueueFamilyProperties2(physicalDevice, pQueueFamilyPropertyCount, pQueueFamilyProperties);
            if (skip) return;
        }
        for (auto intercept : global_interceptor_list) {
            intercept->PreCallRecordGetPhysicalDeviceQueueFamilyProperties2(physicalDevice, pQueueFamilyPropertyCount, pQueueFamilyProperties);
        }
    }
    instance_data->dispatch_table.GetPhysicalDeviceQueueFamilyProperties2(physicalDevice, pQueueFamilyPropertyCount, pQueueFamilyProperties);
    {
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            intercept->PostCallRecordGetPhysicalDeviceQueueFamilyProperties2(physicalDevice, pQueueFamilyPropertyCount, pQueueFamilyProperties);
        }
    }
}

VKAPI_ATTR void VKAPI_CALL GetPhysicalDeviceMemoryProperties2(
    VkPhysicalDevice                            physicalDevice,
    VkPhysicalDeviceMemoryProperties2*          pMemoryProperties) {
    instance_layer_data *instance_data = GetLayerDataPtr(get_dispatch_key(physicalDevice), instance_layer_data_map);
    {
        bool skip = false;
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            skip |= intercept->PreCallValidateGetPhysicalDeviceMemoryProperties2(physicalDevice, pMemoryProperties);
            if (skip) return;
        }
        for (auto intercept : global_interceptor_list) {
            intercept->PreCallRecordGetPhysicalDeviceMemoryProperties2(physicalDevice, pMemoryProperties);
        }
    }
    instance_data->dispatch_table.GetPhysicalDeviceMemoryProperties2(physicalDevice, pMemoryProperties);
    {
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            intercept->PostCallRecordGetPhysicalDeviceMemoryProperties2(physicalDevice, pMemoryProperties);
        }
    }
}

VKAPI_ATTR void VKAPI_CALL GetPhysicalDeviceSparseImageFormatProperties2(
    VkPhysicalDevice                            physicalDevice,
    const VkPhysicalDeviceSparseImageFormatInfo2* pFormatInfo,
    uint32_t*                                   pPropertyCount,
    VkSparseImageFormatProperties2*             pProperties) {
    instance_layer_data *instance_data = GetLayerDataPtr(get_dispatch_key(physicalDevice), instance_layer_data_map);
    {
        bool skip = false;
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            skip |= intercept->PreCallValidateGetPhysicalDeviceSparseImageFormatProperties2(physicalDevice, pFormatInfo, pPropertyCount, pProperties);
            if (skip) return;
        }
        for (auto intercept : global_interceptor_list) {
            intercept->PreCallRecordGetPhysicalDeviceSparseImageFormatProperties2(physicalDevice, pFormatInfo, pPropertyCount, pProperties);
        }
    }
    instance_data->dispatch_table.GetPhysicalDeviceSparseImageFormatProperties2(physicalDevice, pFormatInfo, pPropertyCount, pProperties);
    {
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            intercept->PostCallRecordGetPhysicalDeviceSparseImageFormatProperties2(physicalDevice, pFormatInfo, pPropertyCount, pProperties);
        }
    }
}

VKAPI_ATTR void VKAPI_CALL TrimCommandPool(
    VkDevice                                    device,
    VkCommandPool                               commandPool,
    VkCommandPoolTrimFlags                      flags) {
    layer_data *device_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    {
        bool skip = false;
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            skip |= intercept->PreCallValidateTrimCommandPool(device, commandPool, flags);
            if (skip) return;
        }
        for (auto intercept : global_interceptor_list) {
            intercept->PreCallRecordTrimCommandPool(device, commandPool, flags);
        }
    }
    device_data->dispatch_table.TrimCommandPool(device, commandPool, flags);
    {
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            intercept->PostCallRecordTrimCommandPool(device, commandPool, flags);
        }
    }
}

VKAPI_ATTR void VKAPI_CALL GetDeviceQueue2(
    VkDevice                                    device,
    const VkDeviceQueueInfo2*                   pQueueInfo,
    VkQueue*                                    pQueue) {
    layer_data *device_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    {
        bool skip = false;
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            skip |= intercept->PreCallValidateGetDeviceQueue2(device, pQueueInfo, pQueue);
            if (skip) return;
        }
        for (auto intercept : global_interceptor_list) {
            intercept->PreCallRecordGetDeviceQueue2(device, pQueueInfo, pQueue);
        }
    }
    device_data->dispatch_table.GetDeviceQueue2(device, pQueueInfo, pQueue);
    {
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            intercept->PostCallRecordGetDeviceQueue2(device, pQueueInfo, pQueue);
        }
    }
}

VKAPI_ATTR VkResult VKAPI_CALL CreateSamplerYcbcrConversion(
    VkDevice                                    device,
    const VkSamplerYcbcrConversionCreateInfo*   pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkSamplerYcbcrConversion*                   pYcbcrConversion) {
    layer_data *device_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    {
        bool skip = false;
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            skip |= intercept->PreCallValidateCreateSamplerYcbcrConversion(device, pCreateInfo, pAllocator, pYcbcrConversion);
            if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
        }
        for (auto intercept : global_interceptor_list) {
            intercept->PreCallRecordCreateSamplerYcbcrConversion(device, pCreateInfo, pAllocator, pYcbcrConversion);
        }
    }
    VkResult result = device_data->dispatch_table.CreateSamplerYcbcrConversion(device, pCreateInfo, pAllocator, pYcbcrConversion);
    if (VK_SUCCESS == result) {
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            intercept->PostCallRecordCreateSamplerYcbcrConversion(device, pCreateInfo, pAllocator, pYcbcrConversion);
        }
    }
    return result;
}

VKAPI_ATTR void VKAPI_CALL DestroySamplerYcbcrConversion(
    VkDevice                                    device,
    VkSamplerYcbcrConversion                    ycbcrConversion,
    const VkAllocationCallbacks*                pAllocator) {
    layer_data *device_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    {
        bool skip = false;
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            skip |= intercept->PreCallValidateDestroySamplerYcbcrConversion(device, ycbcrConversion, pAllocator);
            if (skip) return;
        }
        for (auto intercept : global_interceptor_list) {
            intercept->PreCallRecordDestroySamplerYcbcrConversion(device, ycbcrConversion, pAllocator);
        }
    }
    device_data->dispatch_table.DestroySamplerYcbcrConversion(device, ycbcrConversion, pAllocator);
    {
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            intercept->PostCallRecordDestroySamplerYcbcrConversion(device, ycbcrConversion, pAllocator);
        }
    }
}

VKAPI_ATTR VkResult VKAPI_CALL CreateDescriptorUpdateTemplate(
    VkDevice                                    device,
    const VkDescriptorUpdateTemplateCreateInfo* pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkDescriptorUpdateTemplate*                 pDescriptorUpdateTemplate) {
    layer_data *device_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    {
        bool skip = false;
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            skip |= intercept->PreCallValidateCreateDescriptorUpdateTemplate(device, pCreateInfo, pAllocator, pDescriptorUpdateTemplate);
            if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
        }
        for (auto intercept : global_interceptor_list) {
            intercept->PreCallRecordCreateDescriptorUpdateTemplate(device, pCreateInfo, pAllocator, pDescriptorUpdateTemplate);
        }
    }
    VkResult result = device_data->dispatch_table.CreateDescriptorUpdateTemplate(device, pCreateInfo, pAllocator, pDescriptorUpdateTemplate);
    if (VK_SUCCESS == result) {
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            intercept->PostCallRecordCreateDescriptorUpdateTemplate(device, pCreateInfo, pAllocator, pDescriptorUpdateTemplate);
        }
    }
    return result;
}

VKAPI_ATTR void VKAPI_CALL DestroyDescriptorUpdateTemplate(
    VkDevice                                    device,
    VkDescriptorUpdateTemplate                  descriptorUpdateTemplate,
    const VkAllocationCallbacks*                pAllocator) {
    layer_data *device_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    {
        bool skip = false;
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            skip |= intercept->PreCallValidateDestroyDescriptorUpdateTemplate(device, descriptorUpdateTemplate, pAllocator);
            if (skip) return;
        }
        for (auto intercept : global_interceptor_list) {
            intercept->PreCallRecordDestroyDescriptorUpdateTemplate(device, descriptorUpdateTemplate, pAllocator);
        }
    }
    device_data->dispatch_table.DestroyDescriptorUpdateTemplate(device, descriptorUpdateTemplate, pAllocator);
    {
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            intercept->PostCallRecordDestroyDescriptorUpdateTemplate(device, descriptorUpdateTemplate, pAllocator);
        }
    }
}

VKAPI_ATTR void VKAPI_CALL UpdateDescriptorSetWithTemplate(
    VkDevice                                    device,
    VkDescriptorSet                             descriptorSet,
    VkDescriptorUpdateTemplate                  descriptorUpdateTemplate,
    const void*                                 pData) {
    layer_data *device_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    {
        bool skip = false;
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            skip |= intercept->PreCallValidateUpdateDescriptorSetWithTemplate(device, descriptorSet, descriptorUpdateTemplate, pData);
            if (skip) return;
        }
        for (auto intercept : global_interceptor_list) {
            intercept->PreCallRecordUpdateDescriptorSetWithTemplate(device, descriptorSet, descriptorUpdateTemplate, pData);
        }
    }
    device_data->dispatch_table.UpdateDescriptorSetWithTemplate(device, descriptorSet, descriptorUpdateTemplate, pData);
    {
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            intercept->PostCallRecordUpdateDescriptorSetWithTemplate(device, descriptorSet, descriptorUpdateTemplate, pData);
        }
    }
}

VKAPI_ATTR void VKAPI_CALL GetPhysicalDeviceExternalBufferProperties(
    VkPhysicalDevice                            physicalDevice,
    const VkPhysicalDeviceExternalBufferInfo*   pExternalBufferInfo,
    VkExternalBufferProperties*                 pExternalBufferProperties) {
    instance_layer_data *instance_data = GetLayerDataPtr(get_dispatch_key(physicalDevice), instance_layer_data_map);
    {
        bool skip = false;
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            skip |= intercept->PreCallValidateGetPhysicalDeviceExternalBufferProperties(physicalDevice, pExternalBufferInfo, pExternalBufferProperties);
            if (skip) return;
        }
        for (auto intercept : global_interceptor_list) {
            intercept->PreCallRecordGetPhysicalDeviceExternalBufferProperties(physicalDevice, pExternalBufferInfo, pExternalBufferProperties);
        }
    }
    instance_data->dispatch_table.GetPhysicalDeviceExternalBufferProperties(physicalDevice, pExternalBufferInfo, pExternalBufferProperties);
    {
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            intercept->PostCallRecordGetPhysicalDeviceExternalBufferProperties(physicalDevice, pExternalBufferInfo, pExternalBufferProperties);
        }
    }
}

VKAPI_ATTR void VKAPI_CALL GetPhysicalDeviceExternalFenceProperties(
    VkPhysicalDevice                            physicalDevice,
    const VkPhysicalDeviceExternalFenceInfo*    pExternalFenceInfo,
    VkExternalFenceProperties*                  pExternalFenceProperties) {
    instance_layer_data *instance_data = GetLayerDataPtr(get_dispatch_key(physicalDevice), instance_layer_data_map);
    {
        bool skip = false;
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            skip |= intercept->PreCallValidateGetPhysicalDeviceExternalFenceProperties(physicalDevice, pExternalFenceInfo, pExternalFenceProperties);
            if (skip) return;
        }
        for (auto intercept : global_interceptor_list) {
            intercept->PreCallRecordGetPhysicalDeviceExternalFenceProperties(physicalDevice, pExternalFenceInfo, pExternalFenceProperties);
        }
    }
    instance_data->dispatch_table.GetPhysicalDeviceExternalFenceProperties(physicalDevice, pExternalFenceInfo, pExternalFenceProperties);
    {
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            intercept->PostCallRecordGetPhysicalDeviceExternalFenceProperties(physicalDevice, pExternalFenceInfo, pExternalFenceProperties);
        }
    }
}

VKAPI_ATTR void VKAPI_CALL GetPhysicalDeviceExternalSemaphoreProperties(
    VkPhysicalDevice                            physicalDevice,
    const VkPhysicalDeviceExternalSemaphoreInfo* pExternalSemaphoreInfo,
    VkExternalSemaphoreProperties*              pExternalSemaphoreProperties) {
    instance_layer_data *instance_data = GetLayerDataPtr(get_dispatch_key(physicalDevice), instance_layer_data_map);
    {
        bool skip = false;
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            skip |= intercept->PreCallValidateGetPhysicalDeviceExternalSemaphoreProperties(physicalDevice, pExternalSemaphoreInfo, pExternalSemaphoreProperties);
            if (skip) return;
        }
        for (auto intercept : global_interceptor_list) {
            intercept->PreCallRecordGetPhysicalDeviceExternalSemaphoreProperties(physicalDevice, pExternalSemaphoreInfo, pExternalSemaphoreProperties);
        }
    }
    instance_data->dispatch_table.GetPhysicalDeviceExternalSemaphoreProperties(physicalDevice, pExternalSemaphoreInfo, pExternalSemaphoreProperties);
    {
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            intercept->PostCallRecordGetPhysicalDeviceExternalSemaphoreProperties(physicalDevice, pExternalSemaphoreInfo, pExternalSemaphoreProperties);
        }
    }
}

VKAPI_ATTR void VKAPI_CALL GetDescriptorSetLayoutSupport(
    VkDevice                                    device,
    const VkDescriptorSetLayoutCreateInfo*      pCreateInfo,
    VkDescriptorSetLayoutSupport*               pSupport) {
    layer_data *device_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    {
        bool skip = false;
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            skip |= intercept->PreCallValidateGetDescriptorSetLayoutSupport(device, pCreateInfo, pSupport);
            if (skip) return;
        }
        for (auto intercept : global_interceptor_list) {
            intercept->PreCallRecordGetDescriptorSetLayoutSupport(device, pCreateInfo, pSupport);
        }
    }
    device_data->dispatch_table.GetDescriptorSetLayoutSupport(device, pCreateInfo, pSupport);
    {
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            intercept->PostCallRecordGetDescriptorSetLayoutSupport(device, pCreateInfo, pSupport);
        }
    }
}


VKAPI_ATTR void VKAPI_CALL DestroySurfaceKHR(
    VkInstance                                  instance,
    VkSurfaceKHR                                surface,
    const VkAllocationCallbacks*                pAllocator) {
    instance_layer_data *instance_data = GetLayerDataPtr(get_dispatch_key(instance), instance_layer_data_map);
    {
        bool skip = false;
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            skip |= intercept->PreCallValidateDestroySurfaceKHR(instance, surface, pAllocator);
            if (skip) return;
        }
        for (auto intercept : global_interceptor_list) {
            intercept->PreCallRecordDestroySurfaceKHR(instance, surface, pAllocator);
        }
    }
    instance_data->dispatch_table.DestroySurfaceKHR(instance, surface, pAllocator);
    {
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            intercept->PostCallRecordDestroySurfaceKHR(instance, surface, pAllocator);
        }
    }
}

VKAPI_ATTR VkResult VKAPI_CALL GetPhysicalDeviceSurfaceSupportKHR(
    VkPhysicalDevice                            physicalDevice,
    uint32_t                                    queueFamilyIndex,
    VkSurfaceKHR                                surface,
    VkBool32*                                   pSupported) {
    instance_layer_data *instance_data = GetLayerDataPtr(get_dispatch_key(physicalDevice), instance_layer_data_map);
    {
        bool skip = false;
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            skip |= intercept->PreCallValidateGetPhysicalDeviceSurfaceSupportKHR(physicalDevice, queueFamilyIndex, surface, pSupported);
            if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
        }
        for (auto intercept : global_interceptor_list) {
            intercept->PreCallRecordGetPhysicalDeviceSurfaceSupportKHR(physicalDevice, queueFamilyIndex, surface, pSupported);
        }
    }
    VkResult result = instance_data->dispatch_table.GetPhysicalDeviceSurfaceSupportKHR(physicalDevice, queueFamilyIndex, surface, pSupported);
    if (VK_SUCCESS == result) {
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            intercept->PostCallRecordGetPhysicalDeviceSurfaceSupportKHR(physicalDevice, queueFamilyIndex, surface, pSupported);
        }
    }
    return result;
}

VKAPI_ATTR VkResult VKAPI_CALL GetPhysicalDeviceSurfaceCapabilitiesKHR(
    VkPhysicalDevice                            physicalDevice,
    VkSurfaceKHR                                surface,
    VkSurfaceCapabilitiesKHR*                   pSurfaceCapabilities) {
    instance_layer_data *instance_data = GetLayerDataPtr(get_dispatch_key(physicalDevice), instance_layer_data_map);
    {
        bool skip = false;
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            skip |= intercept->PreCallValidateGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice, surface, pSurfaceCapabilities);
            if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
        }
        for (auto intercept : global_interceptor_list) {
            intercept->PreCallRecordGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice, surface, pSurfaceCapabilities);
        }
    }
    VkResult result = instance_data->dispatch_table.GetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice, surface, pSurfaceCapabilities);
    if (VK_SUCCESS == result) {
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            intercept->PostCallRecordGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice, surface, pSurfaceCapabilities);
        }
    }
    return result;
}

VKAPI_ATTR VkResult VKAPI_CALL GetPhysicalDeviceSurfaceFormatsKHR(
    VkPhysicalDevice                            physicalDevice,
    VkSurfaceKHR                                surface,
    uint32_t*                                   pSurfaceFormatCount,
    VkSurfaceFormatKHR*                         pSurfaceFormats) {
    instance_layer_data *instance_data = GetLayerDataPtr(get_dispatch_key(physicalDevice), instance_layer_data_map);
    {
        bool skip = false;
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            skip |= intercept->PreCallValidateGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, pSurfaceFormatCount, pSurfaceFormats);
            if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
        }
        for (auto intercept : global_interceptor_list) {
            intercept->PreCallRecordGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, pSurfaceFormatCount, pSurfaceFormats);
        }
    }
    VkResult result = instance_data->dispatch_table.GetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, pSurfaceFormatCount, pSurfaceFormats);
    if (VK_SUCCESS == result) {
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            intercept->PostCallRecordGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, pSurfaceFormatCount, pSurfaceFormats);
        }
    }
    return result;
}

VKAPI_ATTR VkResult VKAPI_CALL GetPhysicalDeviceSurfacePresentModesKHR(
    VkPhysicalDevice                            physicalDevice,
    VkSurfaceKHR                                surface,
    uint32_t*                                   pPresentModeCount,
    VkPresentModeKHR*                           pPresentModes) {
    instance_layer_data *instance_data = GetLayerDataPtr(get_dispatch_key(physicalDevice), instance_layer_data_map);
    {
        bool skip = false;
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            skip |= intercept->PreCallValidateGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface, pPresentModeCount, pPresentModes);
            if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
        }
        for (auto intercept : global_interceptor_list) {
            intercept->PreCallRecordGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface, pPresentModeCount, pPresentModes);
        }
    }
    VkResult result = instance_data->dispatch_table.GetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface, pPresentModeCount, pPresentModes);
    if (VK_SUCCESS == result) {
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            intercept->PostCallRecordGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface, pPresentModeCount, pPresentModes);
        }
    }
    return result;
}


VKAPI_ATTR VkResult VKAPI_CALL CreateSwapchainKHR(
    VkDevice                                    device,
    const VkSwapchainCreateInfoKHR*             pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkSwapchainKHR*                             pSwapchain) {
    layer_data *device_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    {
        bool skip = false;
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            skip |= intercept->PreCallValidateCreateSwapchainKHR(device, pCreateInfo, pAllocator, pSwapchain);
            if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
        }
        for (auto intercept : global_interceptor_list) {
            intercept->PreCallRecordCreateSwapchainKHR(device, pCreateInfo, pAllocator, pSwapchain);
        }
    }
    VkResult result = device_data->dispatch_table.CreateSwapchainKHR(device, pCreateInfo, pAllocator, pSwapchain);
    if (VK_SUCCESS == result) {
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            intercept->PostCallRecordCreateSwapchainKHR(device, pCreateInfo, pAllocator, pSwapchain);
        }
    }
    return result;
}

VKAPI_ATTR void VKAPI_CALL DestroySwapchainKHR(
    VkDevice                                    device,
    VkSwapchainKHR                              swapchain,
    const VkAllocationCallbacks*                pAllocator) {
    layer_data *device_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    {
        bool skip = false;
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            skip |= intercept->PreCallValidateDestroySwapchainKHR(device, swapchain, pAllocator);
            if (skip) return;
        }
        for (auto intercept : global_interceptor_list) {
            intercept->PreCallRecordDestroySwapchainKHR(device, swapchain, pAllocator);
        }
    }
    device_data->dispatch_table.DestroySwapchainKHR(device, swapchain, pAllocator);
    {
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            intercept->PostCallRecordDestroySwapchainKHR(device, swapchain, pAllocator);
        }
    }
}

VKAPI_ATTR VkResult VKAPI_CALL GetSwapchainImagesKHR(
    VkDevice                                    device,
    VkSwapchainKHR                              swapchain,
    uint32_t*                                   pSwapchainImageCount,
    VkImage*                                    pSwapchainImages) {
    layer_data *device_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    {
        bool skip = false;
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            skip |= intercept->PreCallValidateGetSwapchainImagesKHR(device, swapchain, pSwapchainImageCount, pSwapchainImages);
            if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
        }
        for (auto intercept : global_interceptor_list) {
            intercept->PreCallRecordGetSwapchainImagesKHR(device, swapchain, pSwapchainImageCount, pSwapchainImages);
        }
    }
    VkResult result = device_data->dispatch_table.GetSwapchainImagesKHR(device, swapchain, pSwapchainImageCount, pSwapchainImages);
    if (VK_SUCCESS == result) {
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            intercept->PostCallRecordGetSwapchainImagesKHR(device, swapchain, pSwapchainImageCount, pSwapchainImages);
        }
    }
    return result;
}

VKAPI_ATTR VkResult VKAPI_CALL AcquireNextImageKHR(
    VkDevice                                    device,
    VkSwapchainKHR                              swapchain,
    uint64_t                                    timeout,
    VkSemaphore                                 semaphore,
    VkFence                                     fence,
    uint32_t*                                   pImageIndex) {
    layer_data *device_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    {
        bool skip = false;
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            skip |= intercept->PreCallValidateAcquireNextImageKHR(device, swapchain, timeout, semaphore, fence, pImageIndex);
            if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
        }
        for (auto intercept : global_interceptor_list) {
            intercept->PreCallRecordAcquireNextImageKHR(device, swapchain, timeout, semaphore, fence, pImageIndex);
        }
    }
    VkResult result = device_data->dispatch_table.AcquireNextImageKHR(device, swapchain, timeout, semaphore, fence, pImageIndex);
    if (VK_SUCCESS == result) {
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            intercept->PostCallRecordAcquireNextImageKHR(device, swapchain, timeout, semaphore, fence, pImageIndex);
        }
    }
    return result;
}

VKAPI_ATTR VkResult VKAPI_CALL QueuePresentKHR(
    VkQueue                                     queue,
    const VkPresentInfoKHR*                     pPresentInfo) {
    layer_data *device_data = GetLayerDataPtr(get_dispatch_key(queue), layer_data_map);
    {
        bool skip = false;
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            skip |= intercept->PreCallValidateQueuePresentKHR(queue, pPresentInfo);
            if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
        }
        for (auto intercept : global_interceptor_list) {
            intercept->PreCallRecordQueuePresentKHR(queue, pPresentInfo);
        }
    }
    VkResult result = device_data->dispatch_table.QueuePresentKHR(queue, pPresentInfo);
    if (VK_SUCCESS == result) {
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            intercept->PostCallRecordQueuePresentKHR(queue, pPresentInfo);
        }
    }
    return result;
}

VKAPI_ATTR VkResult VKAPI_CALL GetDeviceGroupPresentCapabilitiesKHR(
    VkDevice                                    device,
    VkDeviceGroupPresentCapabilitiesKHR*        pDeviceGroupPresentCapabilities) {
    layer_data *device_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    {
        bool skip = false;
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            skip |= intercept->PreCallValidateGetDeviceGroupPresentCapabilitiesKHR(device, pDeviceGroupPresentCapabilities);
            if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
        }
        for (auto intercept : global_interceptor_list) {
            intercept->PreCallRecordGetDeviceGroupPresentCapabilitiesKHR(device, pDeviceGroupPresentCapabilities);
        }
    }
    VkResult result = device_data->dispatch_table.GetDeviceGroupPresentCapabilitiesKHR(device, pDeviceGroupPresentCapabilities);
    if (VK_SUCCESS == result) {
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            intercept->PostCallRecordGetDeviceGroupPresentCapabilitiesKHR(device, pDeviceGroupPresentCapabilities);
        }
    }
    return result;
}

VKAPI_ATTR VkResult VKAPI_CALL GetDeviceGroupSurfacePresentModesKHR(
    VkDevice                                    device,
    VkSurfaceKHR                                surface,
    VkDeviceGroupPresentModeFlagsKHR*           pModes) {
    layer_data *device_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    {
        bool skip = false;
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            skip |= intercept->PreCallValidateGetDeviceGroupSurfacePresentModesKHR(device, surface, pModes);
            if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
        }
        for (auto intercept : global_interceptor_list) {
            intercept->PreCallRecordGetDeviceGroupSurfacePresentModesKHR(device, surface, pModes);
        }
    }
    VkResult result = device_data->dispatch_table.GetDeviceGroupSurfacePresentModesKHR(device, surface, pModes);
    if (VK_SUCCESS == result) {
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            intercept->PostCallRecordGetDeviceGroupSurfacePresentModesKHR(device, surface, pModes);
        }
    }
    return result;
}

VKAPI_ATTR VkResult VKAPI_CALL GetPhysicalDevicePresentRectanglesKHR(
    VkPhysicalDevice                            physicalDevice,
    VkSurfaceKHR                                surface,
    uint32_t*                                   pRectCount,
    VkRect2D*                                   pRects) {
    instance_layer_data *instance_data = GetLayerDataPtr(get_dispatch_key(physicalDevice), instance_layer_data_map);
    {
        bool skip = false;
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            skip |= intercept->PreCallValidateGetPhysicalDevicePresentRectanglesKHR(physicalDevice, surface, pRectCount, pRects);
            if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
        }
        for (auto intercept : global_interceptor_list) {
            intercept->PreCallRecordGetPhysicalDevicePresentRectanglesKHR(physicalDevice, surface, pRectCount, pRects);
        }
    }
    VkResult result = instance_data->dispatch_table.GetPhysicalDevicePresentRectanglesKHR(physicalDevice, surface, pRectCount, pRects);
    if (VK_SUCCESS == result) {
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            intercept->PostCallRecordGetPhysicalDevicePresentRectanglesKHR(physicalDevice, surface, pRectCount, pRects);
        }
    }
    return result;
}

VKAPI_ATTR VkResult VKAPI_CALL AcquireNextImage2KHR(
    VkDevice                                    device,
    const VkAcquireNextImageInfoKHR*            pAcquireInfo,
    uint32_t*                                   pImageIndex) {
    layer_data *device_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    {
        bool skip = false;
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            skip |= intercept->PreCallValidateAcquireNextImage2KHR(device, pAcquireInfo, pImageIndex);
            if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
        }
        for (auto intercept : global_interceptor_list) {
            intercept->PreCallRecordAcquireNextImage2KHR(device, pAcquireInfo, pImageIndex);
        }
    }
    VkResult result = device_data->dispatch_table.AcquireNextImage2KHR(device, pAcquireInfo, pImageIndex);
    if (VK_SUCCESS == result) {
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            intercept->PostCallRecordAcquireNextImage2KHR(device, pAcquireInfo, pImageIndex);
        }
    }
    return result;
}


VKAPI_ATTR VkResult VKAPI_CALL GetPhysicalDeviceDisplayPropertiesKHR(
    VkPhysicalDevice                            physicalDevice,
    uint32_t*                                   pPropertyCount,
    VkDisplayPropertiesKHR*                     pProperties) {
    instance_layer_data *instance_data = GetLayerDataPtr(get_dispatch_key(physicalDevice), instance_layer_data_map);
    {
        bool skip = false;
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            skip |= intercept->PreCallValidateGetPhysicalDeviceDisplayPropertiesKHR(physicalDevice, pPropertyCount, pProperties);
            if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
        }
        for (auto intercept : global_interceptor_list) {
            intercept->PreCallRecordGetPhysicalDeviceDisplayPropertiesKHR(physicalDevice, pPropertyCount, pProperties);
        }
    }
    VkResult result = instance_data->dispatch_table.GetPhysicalDeviceDisplayPropertiesKHR(physicalDevice, pPropertyCount, pProperties);
    if (VK_SUCCESS == result) {
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            intercept->PostCallRecordGetPhysicalDeviceDisplayPropertiesKHR(physicalDevice, pPropertyCount, pProperties);
        }
    }
    return result;
}

VKAPI_ATTR VkResult VKAPI_CALL GetPhysicalDeviceDisplayPlanePropertiesKHR(
    VkPhysicalDevice                            physicalDevice,
    uint32_t*                                   pPropertyCount,
    VkDisplayPlanePropertiesKHR*                pProperties) {
    instance_layer_data *instance_data = GetLayerDataPtr(get_dispatch_key(physicalDevice), instance_layer_data_map);
    {
        bool skip = false;
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            skip |= intercept->PreCallValidateGetPhysicalDeviceDisplayPlanePropertiesKHR(physicalDevice, pPropertyCount, pProperties);
            if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
        }
        for (auto intercept : global_interceptor_list) {
            intercept->PreCallRecordGetPhysicalDeviceDisplayPlanePropertiesKHR(physicalDevice, pPropertyCount, pProperties);
        }
    }
    VkResult result = instance_data->dispatch_table.GetPhysicalDeviceDisplayPlanePropertiesKHR(physicalDevice, pPropertyCount, pProperties);
    if (VK_SUCCESS == result) {
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            intercept->PostCallRecordGetPhysicalDeviceDisplayPlanePropertiesKHR(physicalDevice, pPropertyCount, pProperties);
        }
    }
    return result;
}

VKAPI_ATTR VkResult VKAPI_CALL GetDisplayPlaneSupportedDisplaysKHR(
    VkPhysicalDevice                            physicalDevice,
    uint32_t                                    planeIndex,
    uint32_t*                                   pDisplayCount,
    VkDisplayKHR*                               pDisplays) {
    instance_layer_data *instance_data = GetLayerDataPtr(get_dispatch_key(physicalDevice), instance_layer_data_map);
    {
        bool skip = false;
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            skip |= intercept->PreCallValidateGetDisplayPlaneSupportedDisplaysKHR(physicalDevice, planeIndex, pDisplayCount, pDisplays);
            if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
        }
        for (auto intercept : global_interceptor_list) {
            intercept->PreCallRecordGetDisplayPlaneSupportedDisplaysKHR(physicalDevice, planeIndex, pDisplayCount, pDisplays);
        }
    }
    VkResult result = instance_data->dispatch_table.GetDisplayPlaneSupportedDisplaysKHR(physicalDevice, planeIndex, pDisplayCount, pDisplays);
    if (VK_SUCCESS == result) {
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            intercept->PostCallRecordGetDisplayPlaneSupportedDisplaysKHR(physicalDevice, planeIndex, pDisplayCount, pDisplays);
        }
    }
    return result;
}

VKAPI_ATTR VkResult VKAPI_CALL GetDisplayModePropertiesKHR(
    VkPhysicalDevice                            physicalDevice,
    VkDisplayKHR                                display,
    uint32_t*                                   pPropertyCount,
    VkDisplayModePropertiesKHR*                 pProperties) {
    instance_layer_data *instance_data = GetLayerDataPtr(get_dispatch_key(physicalDevice), instance_layer_data_map);
    {
        bool skip = false;
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            skip |= intercept->PreCallValidateGetDisplayModePropertiesKHR(physicalDevice, display, pPropertyCount, pProperties);
            if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
        }
        for (auto intercept : global_interceptor_list) {
            intercept->PreCallRecordGetDisplayModePropertiesKHR(physicalDevice, display, pPropertyCount, pProperties);
        }
    }
    VkResult result = instance_data->dispatch_table.GetDisplayModePropertiesKHR(physicalDevice, display, pPropertyCount, pProperties);
    if (VK_SUCCESS == result) {
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            intercept->PostCallRecordGetDisplayModePropertiesKHR(physicalDevice, display, pPropertyCount, pProperties);
        }
    }
    return result;
}

VKAPI_ATTR VkResult VKAPI_CALL CreateDisplayModeKHR(
    VkPhysicalDevice                            physicalDevice,
    VkDisplayKHR                                display,
    const VkDisplayModeCreateInfoKHR*           pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkDisplayModeKHR*                           pMode) {
    instance_layer_data *instance_data = GetLayerDataPtr(get_dispatch_key(physicalDevice), instance_layer_data_map);
    {
        bool skip = false;
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            skip |= intercept->PreCallValidateCreateDisplayModeKHR(physicalDevice, display, pCreateInfo, pAllocator, pMode);
            if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
        }
        for (auto intercept : global_interceptor_list) {
            intercept->PreCallRecordCreateDisplayModeKHR(physicalDevice, display, pCreateInfo, pAllocator, pMode);
        }
    }
    VkResult result = instance_data->dispatch_table.CreateDisplayModeKHR(physicalDevice, display, pCreateInfo, pAllocator, pMode);
    if (VK_SUCCESS == result) {
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            intercept->PostCallRecordCreateDisplayModeKHR(physicalDevice, display, pCreateInfo, pAllocator, pMode);
        }
    }
    return result;
}

VKAPI_ATTR VkResult VKAPI_CALL GetDisplayPlaneCapabilitiesKHR(
    VkPhysicalDevice                            physicalDevice,
    VkDisplayModeKHR                            mode,
    uint32_t                                    planeIndex,
    VkDisplayPlaneCapabilitiesKHR*              pCapabilities) {
    instance_layer_data *instance_data = GetLayerDataPtr(get_dispatch_key(physicalDevice), instance_layer_data_map);
    {
        bool skip = false;
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            skip |= intercept->PreCallValidateGetDisplayPlaneCapabilitiesKHR(physicalDevice, mode, planeIndex, pCapabilities);
            if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
        }
        for (auto intercept : global_interceptor_list) {
            intercept->PreCallRecordGetDisplayPlaneCapabilitiesKHR(physicalDevice, mode, planeIndex, pCapabilities);
        }
    }
    VkResult result = instance_data->dispatch_table.GetDisplayPlaneCapabilitiesKHR(physicalDevice, mode, planeIndex, pCapabilities);
    if (VK_SUCCESS == result) {
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            intercept->PostCallRecordGetDisplayPlaneCapabilitiesKHR(physicalDevice, mode, planeIndex, pCapabilities);
        }
    }
    return result;
}

VKAPI_ATTR VkResult VKAPI_CALL CreateDisplayPlaneSurfaceKHR(
    VkInstance                                  instance,
    const VkDisplaySurfaceCreateInfoKHR*        pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkSurfaceKHR*                               pSurface) {
    instance_layer_data *instance_data = GetLayerDataPtr(get_dispatch_key(instance), instance_layer_data_map);
    {
        bool skip = false;
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            skip |= intercept->PreCallValidateCreateDisplayPlaneSurfaceKHR(instance, pCreateInfo, pAllocator, pSurface);
            if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
        }
        for (auto intercept : global_interceptor_list) {
            intercept->PreCallRecordCreateDisplayPlaneSurfaceKHR(instance, pCreateInfo, pAllocator, pSurface);
        }
    }
    VkResult result = instance_data->dispatch_table.CreateDisplayPlaneSurfaceKHR(instance, pCreateInfo, pAllocator, pSurface);
    if (VK_SUCCESS == result) {
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            intercept->PostCallRecordCreateDisplayPlaneSurfaceKHR(instance, pCreateInfo, pAllocator, pSurface);
        }
    }
    return result;
}


VKAPI_ATTR VkResult VKAPI_CALL CreateSharedSwapchainsKHR(
    VkDevice                                    device,
    uint32_t                                    swapchainCount,
    const VkSwapchainCreateInfoKHR*             pCreateInfos,
    const VkAllocationCallbacks*                pAllocator,
    VkSwapchainKHR*                             pSwapchains) {
    layer_data *device_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    {
        bool skip = false;
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            skip |= intercept->PreCallValidateCreateSharedSwapchainsKHR(device, swapchainCount, pCreateInfos, pAllocator, pSwapchains);
            if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
        }
        for (auto intercept : global_interceptor_list) {
            intercept->PreCallRecordCreateSharedSwapchainsKHR(device, swapchainCount, pCreateInfos, pAllocator, pSwapchains);
        }
    }
    VkResult result = device_data->dispatch_table.CreateSharedSwapchainsKHR(device, swapchainCount, pCreateInfos, pAllocator, pSwapchains);
    if (VK_SUCCESS == result) {
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            intercept->PostCallRecordCreateSharedSwapchainsKHR(device, swapchainCount, pCreateInfos, pAllocator, pSwapchains);
        }
    }
    return result;
}

#ifdef VK_USE_PLATFORM_XLIB_KHR

VKAPI_ATTR VkResult VKAPI_CALL CreateXlibSurfaceKHR(
    VkInstance                                  instance,
    const VkXlibSurfaceCreateInfoKHR*           pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkSurfaceKHR*                               pSurface) {
    instance_layer_data *instance_data = GetLayerDataPtr(get_dispatch_key(instance), instance_layer_data_map);
    {
        bool skip = false;
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            skip |= intercept->PreCallValidateCreateXlibSurfaceKHR(instance, pCreateInfo, pAllocator, pSurface);
            if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
        }
        for (auto intercept : global_interceptor_list) {
            intercept->PreCallRecordCreateXlibSurfaceKHR(instance, pCreateInfo, pAllocator, pSurface);
        }
    }
    VkResult result = instance_data->dispatch_table.CreateXlibSurfaceKHR(instance, pCreateInfo, pAllocator, pSurface);
    if (VK_SUCCESS == result) {
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            intercept->PostCallRecordCreateXlibSurfaceKHR(instance, pCreateInfo, pAllocator, pSurface);
        }
    }
    return result;
}

VKAPI_ATTR VkBool32 VKAPI_CALL GetPhysicalDeviceXlibPresentationSupportKHR(
    VkPhysicalDevice                            physicalDevice,
    uint32_t                                    queueFamilyIndex,
    Display*                                    dpy,
    VisualID                                    visualID) {
    instance_layer_data *instance_data = GetLayerDataPtr(get_dispatch_key(physicalDevice), instance_layer_data_map);
    {
        bool skip = false;
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            skip |= intercept->PreCallValidateGetPhysicalDeviceXlibPresentationSupportKHR(physicalDevice, queueFamilyIndex, dpy, visualID);
            if (skip) return VK_FALSE;
        }
        for (auto intercept : global_interceptor_list) {
            intercept->PreCallRecordGetPhysicalDeviceXlibPresentationSupportKHR(physicalDevice, queueFamilyIndex, dpy, visualID);
        }
    }
    VkBool32 result = instance_data->dispatch_table.GetPhysicalDeviceXlibPresentationSupportKHR(physicalDevice, queueFamilyIndex, dpy, visualID);
    {
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            intercept->PostCallRecordGetPhysicalDeviceXlibPresentationSupportKHR(physicalDevice, queueFamilyIndex, dpy, visualID);
        }
    }
    return result;
}
#endif // VK_USE_PLATFORM_XLIB_KHR

#ifdef VK_USE_PLATFORM_XCB_KHR

VKAPI_ATTR VkResult VKAPI_CALL CreateXcbSurfaceKHR(
    VkInstance                                  instance,
    const VkXcbSurfaceCreateInfoKHR*            pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkSurfaceKHR*                               pSurface) {
    instance_layer_data *instance_data = GetLayerDataPtr(get_dispatch_key(instance), instance_layer_data_map);
    {
        bool skip = false;
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            skip |= intercept->PreCallValidateCreateXcbSurfaceKHR(instance, pCreateInfo, pAllocator, pSurface);
            if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
        }
        for (auto intercept : global_interceptor_list) {
            intercept->PreCallRecordCreateXcbSurfaceKHR(instance, pCreateInfo, pAllocator, pSurface);
        }
    }
    VkResult result = instance_data->dispatch_table.CreateXcbSurfaceKHR(instance, pCreateInfo, pAllocator, pSurface);
    if (VK_SUCCESS == result) {
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            intercept->PostCallRecordCreateXcbSurfaceKHR(instance, pCreateInfo, pAllocator, pSurface);
        }
    }
    return result;
}

VKAPI_ATTR VkBool32 VKAPI_CALL GetPhysicalDeviceXcbPresentationSupportKHR(
    VkPhysicalDevice                            physicalDevice,
    uint32_t                                    queueFamilyIndex,
    xcb_connection_t*                           connection,
    xcb_visualid_t                              visual_id) {
    instance_layer_data *instance_data = GetLayerDataPtr(get_dispatch_key(physicalDevice), instance_layer_data_map);
    {
        bool skip = false;
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            skip |= intercept->PreCallValidateGetPhysicalDeviceXcbPresentationSupportKHR(physicalDevice, queueFamilyIndex, connection, visual_id);
            if (skip) return VK_FALSE;
        }
        for (auto intercept : global_interceptor_list) {
            intercept->PreCallRecordGetPhysicalDeviceXcbPresentationSupportKHR(physicalDevice, queueFamilyIndex, connection, visual_id);
        }
    }
    VkBool32 result = instance_data->dispatch_table.GetPhysicalDeviceXcbPresentationSupportKHR(physicalDevice, queueFamilyIndex, connection, visual_id);
    {
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            intercept->PostCallRecordGetPhysicalDeviceXcbPresentationSupportKHR(physicalDevice, queueFamilyIndex, connection, visual_id);
        }
    }
    return result;
}
#endif // VK_USE_PLATFORM_XCB_KHR

#ifdef VK_USE_PLATFORM_WAYLAND_KHR

VKAPI_ATTR VkResult VKAPI_CALL CreateWaylandSurfaceKHR(
    VkInstance                                  instance,
    const VkWaylandSurfaceCreateInfoKHR*        pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkSurfaceKHR*                               pSurface) {
    instance_layer_data *instance_data = GetLayerDataPtr(get_dispatch_key(instance), instance_layer_data_map);
    {
        bool skip = false;
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            skip |= intercept->PreCallValidateCreateWaylandSurfaceKHR(instance, pCreateInfo, pAllocator, pSurface);
            if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
        }
        for (auto intercept : global_interceptor_list) {
            intercept->PreCallRecordCreateWaylandSurfaceKHR(instance, pCreateInfo, pAllocator, pSurface);
        }
    }
    VkResult result = instance_data->dispatch_table.CreateWaylandSurfaceKHR(instance, pCreateInfo, pAllocator, pSurface);
    if (VK_SUCCESS == result) {
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            intercept->PostCallRecordCreateWaylandSurfaceKHR(instance, pCreateInfo, pAllocator, pSurface);
        }
    }
    return result;
}

VKAPI_ATTR VkBool32 VKAPI_CALL GetPhysicalDeviceWaylandPresentationSupportKHR(
    VkPhysicalDevice                            physicalDevice,
    uint32_t                                    queueFamilyIndex,
    struct wl_display*                          display) {
    instance_layer_data *instance_data = GetLayerDataPtr(get_dispatch_key(physicalDevice), instance_layer_data_map);
    {
        bool skip = false;
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            skip |= intercept->PreCallValidateGetPhysicalDeviceWaylandPresentationSupportKHR(physicalDevice, queueFamilyIndex, display);
            if (skip) return VK_FALSE;
        }
        for (auto intercept : global_interceptor_list) {
            intercept->PreCallRecordGetPhysicalDeviceWaylandPresentationSupportKHR(physicalDevice, queueFamilyIndex, display);
        }
    }
    VkBool32 result = instance_data->dispatch_table.GetPhysicalDeviceWaylandPresentationSupportKHR(physicalDevice, queueFamilyIndex, display);
    {
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            intercept->PostCallRecordGetPhysicalDeviceWaylandPresentationSupportKHR(physicalDevice, queueFamilyIndex, display);
        }
    }
    return result;
}
#endif // VK_USE_PLATFORM_WAYLAND_KHR

#ifdef VK_USE_PLATFORM_MIR_KHR

VKAPI_ATTR VkResult VKAPI_CALL CreateMirSurfaceKHR(
    VkInstance                                  instance,
    const VkMirSurfaceCreateInfoKHR*            pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkSurfaceKHR*                               pSurface) {
    instance_layer_data *instance_data = GetLayerDataPtr(get_dispatch_key(instance), instance_layer_data_map);
    {
        bool skip = false;
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            skip |= intercept->PreCallValidateCreateMirSurfaceKHR(instance, pCreateInfo, pAllocator, pSurface);
            if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
        }
        for (auto intercept : global_interceptor_list) {
            intercept->PreCallRecordCreateMirSurfaceKHR(instance, pCreateInfo, pAllocator, pSurface);
        }
    }
    VkResult result = instance_data->dispatch_table.CreateMirSurfaceKHR(instance, pCreateInfo, pAllocator, pSurface);
    if (VK_SUCCESS == result) {
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            intercept->PostCallRecordCreateMirSurfaceKHR(instance, pCreateInfo, pAllocator, pSurface);
        }
    }
    return result;
}

VKAPI_ATTR VkBool32 VKAPI_CALL GetPhysicalDeviceMirPresentationSupportKHR(
    VkPhysicalDevice                            physicalDevice,
    uint32_t                                    queueFamilyIndex,
    MirConnection*                              connection) {
    instance_layer_data *instance_data = GetLayerDataPtr(get_dispatch_key(physicalDevice), instance_layer_data_map);
    {
        bool skip = false;
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            skip |= intercept->PreCallValidateGetPhysicalDeviceMirPresentationSupportKHR(physicalDevice, queueFamilyIndex, connection);
            if (skip) return VK_FALSE;
        }
        for (auto intercept : global_interceptor_list) {
            intercept->PreCallRecordGetPhysicalDeviceMirPresentationSupportKHR(physicalDevice, queueFamilyIndex, connection);
        }
    }
    VkBool32 result = instance_data->dispatch_table.GetPhysicalDeviceMirPresentationSupportKHR(physicalDevice, queueFamilyIndex, connection);
    {
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            intercept->PostCallRecordGetPhysicalDeviceMirPresentationSupportKHR(physicalDevice, queueFamilyIndex, connection);
        }
    }
    return result;
}
#endif // VK_USE_PLATFORM_MIR_KHR

#ifdef VK_USE_PLATFORM_ANDROID_KHR

VKAPI_ATTR VkResult VKAPI_CALL CreateAndroidSurfaceKHR(
    VkInstance                                  instance,
    const VkAndroidSurfaceCreateInfoKHR*        pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkSurfaceKHR*                               pSurface) {
    instance_layer_data *instance_data = GetLayerDataPtr(get_dispatch_key(instance), instance_layer_data_map);
    {
        bool skip = false;
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            skip |= intercept->PreCallValidateCreateAndroidSurfaceKHR(instance, pCreateInfo, pAllocator, pSurface);
            if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
        }
        for (auto intercept : global_interceptor_list) {
            intercept->PreCallRecordCreateAndroidSurfaceKHR(instance, pCreateInfo, pAllocator, pSurface);
        }
    }
    VkResult result = instance_data->dispatch_table.CreateAndroidSurfaceKHR(instance, pCreateInfo, pAllocator, pSurface);
    if (VK_SUCCESS == result) {
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            intercept->PostCallRecordCreateAndroidSurfaceKHR(instance, pCreateInfo, pAllocator, pSurface);
        }
    }
    return result;
}
#endif // VK_USE_PLATFORM_ANDROID_KHR

#ifdef VK_USE_PLATFORM_WIN32_KHR

VKAPI_ATTR VkResult VKAPI_CALL CreateWin32SurfaceKHR(
    VkInstance                                  instance,
    const VkWin32SurfaceCreateInfoKHR*          pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkSurfaceKHR*                               pSurface) {
    instance_layer_data *instance_data = GetLayerDataPtr(get_dispatch_key(instance), instance_layer_data_map);
    {
        bool skip = false;
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            skip |= intercept->PreCallValidateCreateWin32SurfaceKHR(instance, pCreateInfo, pAllocator, pSurface);
            if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
        }
        for (auto intercept : global_interceptor_list) {
            intercept->PreCallRecordCreateWin32SurfaceKHR(instance, pCreateInfo, pAllocator, pSurface);
        }
    }
    VkResult result = instance_data->dispatch_table.CreateWin32SurfaceKHR(instance, pCreateInfo, pAllocator, pSurface);
    if (VK_SUCCESS == result) {
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            intercept->PostCallRecordCreateWin32SurfaceKHR(instance, pCreateInfo, pAllocator, pSurface);
        }
    }
    return result;
}

VKAPI_ATTR VkBool32 VKAPI_CALL GetPhysicalDeviceWin32PresentationSupportKHR(
    VkPhysicalDevice                            physicalDevice,
    uint32_t                                    queueFamilyIndex) {
    instance_layer_data *instance_data = GetLayerDataPtr(get_dispatch_key(physicalDevice), instance_layer_data_map);
    {
        bool skip = false;
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            skip |= intercept->PreCallValidateGetPhysicalDeviceWin32PresentationSupportKHR(physicalDevice, queueFamilyIndex);
            if (skip) return VK_FALSE;
        }
        for (auto intercept : global_interceptor_list) {
            intercept->PreCallRecordGetPhysicalDeviceWin32PresentationSupportKHR(physicalDevice, queueFamilyIndex);
        }
    }
    VkBool32 result = instance_data->dispatch_table.GetPhysicalDeviceWin32PresentationSupportKHR(physicalDevice, queueFamilyIndex);
    {
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            intercept->PostCallRecordGetPhysicalDeviceWin32PresentationSupportKHR(physicalDevice, queueFamilyIndex);
        }
    }
    return result;
}
#endif // VK_USE_PLATFORM_WIN32_KHR




VKAPI_ATTR void VKAPI_CALL GetPhysicalDeviceFeatures2KHR(
    VkPhysicalDevice                            physicalDevice,
    VkPhysicalDeviceFeatures2*                  pFeatures) {
    instance_layer_data *instance_data = GetLayerDataPtr(get_dispatch_key(physicalDevice), instance_layer_data_map);
    {
        bool skip = false;
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            skip |= intercept->PreCallValidateGetPhysicalDeviceFeatures2KHR(physicalDevice, pFeatures);
            if (skip) return;
        }
        for (auto intercept : global_interceptor_list) {
            intercept->PreCallRecordGetPhysicalDeviceFeatures2KHR(physicalDevice, pFeatures);
        }
    }
    instance_data->dispatch_table.GetPhysicalDeviceFeatures2KHR(physicalDevice, pFeatures);
    {
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            intercept->PostCallRecordGetPhysicalDeviceFeatures2KHR(physicalDevice, pFeatures);
        }
    }
}

VKAPI_ATTR void VKAPI_CALL GetPhysicalDeviceProperties2KHR(
    VkPhysicalDevice                            physicalDevice,
    VkPhysicalDeviceProperties2*                pProperties) {
    instance_layer_data *instance_data = GetLayerDataPtr(get_dispatch_key(physicalDevice), instance_layer_data_map);
    {
        bool skip = false;
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            skip |= intercept->PreCallValidateGetPhysicalDeviceProperties2KHR(physicalDevice, pProperties);
            if (skip) return;
        }
        for (auto intercept : global_interceptor_list) {
            intercept->PreCallRecordGetPhysicalDeviceProperties2KHR(physicalDevice, pProperties);
        }
    }
    instance_data->dispatch_table.GetPhysicalDeviceProperties2KHR(physicalDevice, pProperties);
    {
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            intercept->PostCallRecordGetPhysicalDeviceProperties2KHR(physicalDevice, pProperties);
        }
    }
}

VKAPI_ATTR void VKAPI_CALL GetPhysicalDeviceFormatProperties2KHR(
    VkPhysicalDevice                            physicalDevice,
    VkFormat                                    format,
    VkFormatProperties2*                        pFormatProperties) {
    instance_layer_data *instance_data = GetLayerDataPtr(get_dispatch_key(physicalDevice), instance_layer_data_map);
    {
        bool skip = false;
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            skip |= intercept->PreCallValidateGetPhysicalDeviceFormatProperties2KHR(physicalDevice, format, pFormatProperties);
            if (skip) return;
        }
        for (auto intercept : global_interceptor_list) {
            intercept->PreCallRecordGetPhysicalDeviceFormatProperties2KHR(physicalDevice, format, pFormatProperties);
        }
    }
    instance_data->dispatch_table.GetPhysicalDeviceFormatProperties2KHR(physicalDevice, format, pFormatProperties);
    {
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            intercept->PostCallRecordGetPhysicalDeviceFormatProperties2KHR(physicalDevice, format, pFormatProperties);
        }
    }
}

VKAPI_ATTR VkResult VKAPI_CALL GetPhysicalDeviceImageFormatProperties2KHR(
    VkPhysicalDevice                            physicalDevice,
    const VkPhysicalDeviceImageFormatInfo2*     pImageFormatInfo,
    VkImageFormatProperties2*                   pImageFormatProperties) {
    instance_layer_data *instance_data = GetLayerDataPtr(get_dispatch_key(physicalDevice), instance_layer_data_map);
    {
        bool skip = false;
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            skip |= intercept->PreCallValidateGetPhysicalDeviceImageFormatProperties2KHR(physicalDevice, pImageFormatInfo, pImageFormatProperties);
            if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
        }
        for (auto intercept : global_interceptor_list) {
            intercept->PreCallRecordGetPhysicalDeviceImageFormatProperties2KHR(physicalDevice, pImageFormatInfo, pImageFormatProperties);
        }
    }
    VkResult result = instance_data->dispatch_table.GetPhysicalDeviceImageFormatProperties2KHR(physicalDevice, pImageFormatInfo, pImageFormatProperties);
    if (VK_SUCCESS == result) {
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            intercept->PostCallRecordGetPhysicalDeviceImageFormatProperties2KHR(physicalDevice, pImageFormatInfo, pImageFormatProperties);
        }
    }
    return result;
}

VKAPI_ATTR void VKAPI_CALL GetPhysicalDeviceQueueFamilyProperties2KHR(
    VkPhysicalDevice                            physicalDevice,
    uint32_t*                                   pQueueFamilyPropertyCount,
    VkQueueFamilyProperties2*                   pQueueFamilyProperties) {
    instance_layer_data *instance_data = GetLayerDataPtr(get_dispatch_key(physicalDevice), instance_layer_data_map);
    {
        bool skip = false;
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            skip |= intercept->PreCallValidateGetPhysicalDeviceQueueFamilyProperties2KHR(physicalDevice, pQueueFamilyPropertyCount, pQueueFamilyProperties);
            if (skip) return;
        }
        for (auto intercept : global_interceptor_list) {
            intercept->PreCallRecordGetPhysicalDeviceQueueFamilyProperties2KHR(physicalDevice, pQueueFamilyPropertyCount, pQueueFamilyProperties);
        }
    }
    instance_data->dispatch_table.GetPhysicalDeviceQueueFamilyProperties2KHR(physicalDevice, pQueueFamilyPropertyCount, pQueueFamilyProperties);
    {
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            intercept->PostCallRecordGetPhysicalDeviceQueueFamilyProperties2KHR(physicalDevice, pQueueFamilyPropertyCount, pQueueFamilyProperties);
        }
    }
}

VKAPI_ATTR void VKAPI_CALL GetPhysicalDeviceMemoryProperties2KHR(
    VkPhysicalDevice                            physicalDevice,
    VkPhysicalDeviceMemoryProperties2*          pMemoryProperties) {
    instance_layer_data *instance_data = GetLayerDataPtr(get_dispatch_key(physicalDevice), instance_layer_data_map);
    {
        bool skip = false;
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            skip |= intercept->PreCallValidateGetPhysicalDeviceMemoryProperties2KHR(physicalDevice, pMemoryProperties);
            if (skip) return;
        }
        for (auto intercept : global_interceptor_list) {
            intercept->PreCallRecordGetPhysicalDeviceMemoryProperties2KHR(physicalDevice, pMemoryProperties);
        }
    }
    instance_data->dispatch_table.GetPhysicalDeviceMemoryProperties2KHR(physicalDevice, pMemoryProperties);
    {
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            intercept->PostCallRecordGetPhysicalDeviceMemoryProperties2KHR(physicalDevice, pMemoryProperties);
        }
    }
}

VKAPI_ATTR void VKAPI_CALL GetPhysicalDeviceSparseImageFormatProperties2KHR(
    VkPhysicalDevice                            physicalDevice,
    const VkPhysicalDeviceSparseImageFormatInfo2* pFormatInfo,
    uint32_t*                                   pPropertyCount,
    VkSparseImageFormatProperties2*             pProperties) {
    instance_layer_data *instance_data = GetLayerDataPtr(get_dispatch_key(physicalDevice), instance_layer_data_map);
    {
        bool skip = false;
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            skip |= intercept->PreCallValidateGetPhysicalDeviceSparseImageFormatProperties2KHR(physicalDevice, pFormatInfo, pPropertyCount, pProperties);
            if (skip) return;
        }
        for (auto intercept : global_interceptor_list) {
            intercept->PreCallRecordGetPhysicalDeviceSparseImageFormatProperties2KHR(physicalDevice, pFormatInfo, pPropertyCount, pProperties);
        }
    }
    instance_data->dispatch_table.GetPhysicalDeviceSparseImageFormatProperties2KHR(physicalDevice, pFormatInfo, pPropertyCount, pProperties);
    {
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            intercept->PostCallRecordGetPhysicalDeviceSparseImageFormatProperties2KHR(physicalDevice, pFormatInfo, pPropertyCount, pProperties);
        }
    }
}


VKAPI_ATTR void VKAPI_CALL GetDeviceGroupPeerMemoryFeaturesKHR(
    VkDevice                                    device,
    uint32_t                                    heapIndex,
    uint32_t                                    localDeviceIndex,
    uint32_t                                    remoteDeviceIndex,
    VkPeerMemoryFeatureFlags*                   pPeerMemoryFeatures) {
    layer_data *device_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    {
        bool skip = false;
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            skip |= intercept->PreCallValidateGetDeviceGroupPeerMemoryFeaturesKHR(device, heapIndex, localDeviceIndex, remoteDeviceIndex, pPeerMemoryFeatures);
            if (skip) return;
        }
        for (auto intercept : global_interceptor_list) {
            intercept->PreCallRecordGetDeviceGroupPeerMemoryFeaturesKHR(device, heapIndex, localDeviceIndex, remoteDeviceIndex, pPeerMemoryFeatures);
        }
    }
    device_data->dispatch_table.GetDeviceGroupPeerMemoryFeaturesKHR(device, heapIndex, localDeviceIndex, remoteDeviceIndex, pPeerMemoryFeatures);
    {
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            intercept->PostCallRecordGetDeviceGroupPeerMemoryFeaturesKHR(device, heapIndex, localDeviceIndex, remoteDeviceIndex, pPeerMemoryFeatures);
        }
    }
}

VKAPI_ATTR void VKAPI_CALL CmdSetDeviceMaskKHR(
    VkCommandBuffer                             commandBuffer,
    uint32_t                                    deviceMask) {
    layer_data *device_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    {
        bool skip = false;
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            skip |= intercept->PreCallValidateCmdSetDeviceMaskKHR(commandBuffer, deviceMask);
            if (skip) return;
        }
        for (auto intercept : global_interceptor_list) {
            intercept->PreCallRecordCmdSetDeviceMaskKHR(commandBuffer, deviceMask);
        }
    }
    device_data->dispatch_table.CmdSetDeviceMaskKHR(commandBuffer, deviceMask);
    {
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            intercept->PostCallRecordCmdSetDeviceMaskKHR(commandBuffer, deviceMask);
        }
    }
}

VKAPI_ATTR void VKAPI_CALL CmdDispatchBaseKHR(
    VkCommandBuffer                             commandBuffer,
    uint32_t                                    baseGroupX,
    uint32_t                                    baseGroupY,
    uint32_t                                    baseGroupZ,
    uint32_t                                    groupCountX,
    uint32_t                                    groupCountY,
    uint32_t                                    groupCountZ) {
    layer_data *device_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    {
        bool skip = false;
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            skip |= intercept->PreCallValidateCmdDispatchBaseKHR(commandBuffer, baseGroupX, baseGroupY, baseGroupZ, groupCountX, groupCountY, groupCountZ);
            if (skip) return;
        }
        for (auto intercept : global_interceptor_list) {
            intercept->PreCallRecordCmdDispatchBaseKHR(commandBuffer, baseGroupX, baseGroupY, baseGroupZ, groupCountX, groupCountY, groupCountZ);
        }
    }
    device_data->dispatch_table.CmdDispatchBaseKHR(commandBuffer, baseGroupX, baseGroupY, baseGroupZ, groupCountX, groupCountY, groupCountZ);
    {
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            intercept->PostCallRecordCmdDispatchBaseKHR(commandBuffer, baseGroupX, baseGroupY, baseGroupZ, groupCountX, groupCountY, groupCountZ);
        }
    }
}



VKAPI_ATTR void VKAPI_CALL TrimCommandPoolKHR(
    VkDevice                                    device,
    VkCommandPool                               commandPool,
    VkCommandPoolTrimFlags                      flags) {
    layer_data *device_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    {
        bool skip = false;
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            skip |= intercept->PreCallValidateTrimCommandPoolKHR(device, commandPool, flags);
            if (skip) return;
        }
        for (auto intercept : global_interceptor_list) {
            intercept->PreCallRecordTrimCommandPoolKHR(device, commandPool, flags);
        }
    }
    device_data->dispatch_table.TrimCommandPoolKHR(device, commandPool, flags);
    {
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            intercept->PostCallRecordTrimCommandPoolKHR(device, commandPool, flags);
        }
    }
}


VKAPI_ATTR VkResult VKAPI_CALL EnumeratePhysicalDeviceGroupsKHR(
    VkInstance                                  instance,
    uint32_t*                                   pPhysicalDeviceGroupCount,
    VkPhysicalDeviceGroupProperties*            pPhysicalDeviceGroupProperties) {
    instance_layer_data *instance_data = GetLayerDataPtr(get_dispatch_key(instance), instance_layer_data_map);
    {
        bool skip = false;
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            skip |= intercept->PreCallValidateEnumeratePhysicalDeviceGroupsKHR(instance, pPhysicalDeviceGroupCount, pPhysicalDeviceGroupProperties);
            if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
        }
        for (auto intercept : global_interceptor_list) {
            intercept->PreCallRecordEnumeratePhysicalDeviceGroupsKHR(instance, pPhysicalDeviceGroupCount, pPhysicalDeviceGroupProperties);
        }
    }
    VkResult result = instance_data->dispatch_table.EnumeratePhysicalDeviceGroupsKHR(instance, pPhysicalDeviceGroupCount, pPhysicalDeviceGroupProperties);
    if (VK_SUCCESS == result) {
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            intercept->PostCallRecordEnumeratePhysicalDeviceGroupsKHR(instance, pPhysicalDeviceGroupCount, pPhysicalDeviceGroupProperties);
        }
    }
    return result;
}


VKAPI_ATTR void VKAPI_CALL GetPhysicalDeviceExternalBufferPropertiesKHR(
    VkPhysicalDevice                            physicalDevice,
    const VkPhysicalDeviceExternalBufferInfo*   pExternalBufferInfo,
    VkExternalBufferProperties*                 pExternalBufferProperties) {
    instance_layer_data *instance_data = GetLayerDataPtr(get_dispatch_key(physicalDevice), instance_layer_data_map);
    {
        bool skip = false;
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            skip |= intercept->PreCallValidateGetPhysicalDeviceExternalBufferPropertiesKHR(physicalDevice, pExternalBufferInfo, pExternalBufferProperties);
            if (skip) return;
        }
        for (auto intercept : global_interceptor_list) {
            intercept->PreCallRecordGetPhysicalDeviceExternalBufferPropertiesKHR(physicalDevice, pExternalBufferInfo, pExternalBufferProperties);
        }
    }
    instance_data->dispatch_table.GetPhysicalDeviceExternalBufferPropertiesKHR(physicalDevice, pExternalBufferInfo, pExternalBufferProperties);
    {
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            intercept->PostCallRecordGetPhysicalDeviceExternalBufferPropertiesKHR(physicalDevice, pExternalBufferInfo, pExternalBufferProperties);
        }
    }
}


#ifdef VK_USE_PLATFORM_WIN32_KHR

VKAPI_ATTR VkResult VKAPI_CALL GetMemoryWin32HandleKHR(
    VkDevice                                    device,
    const VkMemoryGetWin32HandleInfoKHR*        pGetWin32HandleInfo,
    HANDLE*                                     pHandle) {
    layer_data *device_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    {
        bool skip = false;
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            skip |= intercept->PreCallValidateGetMemoryWin32HandleKHR(device, pGetWin32HandleInfo, pHandle);
            if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
        }
        for (auto intercept : global_interceptor_list) {
            intercept->PreCallRecordGetMemoryWin32HandleKHR(device, pGetWin32HandleInfo, pHandle);
        }
    }
    VkResult result = device_data->dispatch_table.GetMemoryWin32HandleKHR(device, pGetWin32HandleInfo, pHandle);
    if (VK_SUCCESS == result) {
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            intercept->PostCallRecordGetMemoryWin32HandleKHR(device, pGetWin32HandleInfo, pHandle);
        }
    }
    return result;
}

VKAPI_ATTR VkResult VKAPI_CALL GetMemoryWin32HandlePropertiesKHR(
    VkDevice                                    device,
    VkExternalMemoryHandleTypeFlagBits          handleType,
    HANDLE                                      handle,
    VkMemoryWin32HandlePropertiesKHR*           pMemoryWin32HandleProperties) {
    layer_data *device_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    {
        bool skip = false;
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            skip |= intercept->PreCallValidateGetMemoryWin32HandlePropertiesKHR(device, handleType, handle, pMemoryWin32HandleProperties);
            if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
        }
        for (auto intercept : global_interceptor_list) {
            intercept->PreCallRecordGetMemoryWin32HandlePropertiesKHR(device, handleType, handle, pMemoryWin32HandleProperties);
        }
    }
    VkResult result = device_data->dispatch_table.GetMemoryWin32HandlePropertiesKHR(device, handleType, handle, pMemoryWin32HandleProperties);
    if (VK_SUCCESS == result) {
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            intercept->PostCallRecordGetMemoryWin32HandlePropertiesKHR(device, handleType, handle, pMemoryWin32HandleProperties);
        }
    }
    return result;
}
#endif // VK_USE_PLATFORM_WIN32_KHR


VKAPI_ATTR VkResult VKAPI_CALL GetMemoryFdKHR(
    VkDevice                                    device,
    const VkMemoryGetFdInfoKHR*                 pGetFdInfo,
    int*                                        pFd) {
    layer_data *device_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    {
        bool skip = false;
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            skip |= intercept->PreCallValidateGetMemoryFdKHR(device, pGetFdInfo, pFd);
            if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
        }
        for (auto intercept : global_interceptor_list) {
            intercept->PreCallRecordGetMemoryFdKHR(device, pGetFdInfo, pFd);
        }
    }
    VkResult result = device_data->dispatch_table.GetMemoryFdKHR(device, pGetFdInfo, pFd);
    if (VK_SUCCESS == result) {
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            intercept->PostCallRecordGetMemoryFdKHR(device, pGetFdInfo, pFd);
        }
    }
    return result;
}

VKAPI_ATTR VkResult VKAPI_CALL GetMemoryFdPropertiesKHR(
    VkDevice                                    device,
    VkExternalMemoryHandleTypeFlagBits          handleType,
    int                                         fd,
    VkMemoryFdPropertiesKHR*                    pMemoryFdProperties) {
    layer_data *device_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    {
        bool skip = false;
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            skip |= intercept->PreCallValidateGetMemoryFdPropertiesKHR(device, handleType, fd, pMemoryFdProperties);
            if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
        }
        for (auto intercept : global_interceptor_list) {
            intercept->PreCallRecordGetMemoryFdPropertiesKHR(device, handleType, fd, pMemoryFdProperties);
        }
    }
    VkResult result = device_data->dispatch_table.GetMemoryFdPropertiesKHR(device, handleType, fd, pMemoryFdProperties);
    if (VK_SUCCESS == result) {
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            intercept->PostCallRecordGetMemoryFdPropertiesKHR(device, handleType, fd, pMemoryFdProperties);
        }
    }
    return result;
}

#ifdef VK_USE_PLATFORM_WIN32_KHR
#endif // VK_USE_PLATFORM_WIN32_KHR


VKAPI_ATTR void VKAPI_CALL GetPhysicalDeviceExternalSemaphorePropertiesKHR(
    VkPhysicalDevice                            physicalDevice,
    const VkPhysicalDeviceExternalSemaphoreInfo* pExternalSemaphoreInfo,
    VkExternalSemaphoreProperties*              pExternalSemaphoreProperties) {
    instance_layer_data *instance_data = GetLayerDataPtr(get_dispatch_key(physicalDevice), instance_layer_data_map);
    {
        bool skip = false;
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            skip |= intercept->PreCallValidateGetPhysicalDeviceExternalSemaphorePropertiesKHR(physicalDevice, pExternalSemaphoreInfo, pExternalSemaphoreProperties);
            if (skip) return;
        }
        for (auto intercept : global_interceptor_list) {
            intercept->PreCallRecordGetPhysicalDeviceExternalSemaphorePropertiesKHR(physicalDevice, pExternalSemaphoreInfo, pExternalSemaphoreProperties);
        }
    }
    instance_data->dispatch_table.GetPhysicalDeviceExternalSemaphorePropertiesKHR(physicalDevice, pExternalSemaphoreInfo, pExternalSemaphoreProperties);
    {
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            intercept->PostCallRecordGetPhysicalDeviceExternalSemaphorePropertiesKHR(physicalDevice, pExternalSemaphoreInfo, pExternalSemaphoreProperties);
        }
    }
}


#ifdef VK_USE_PLATFORM_WIN32_KHR

VKAPI_ATTR VkResult VKAPI_CALL ImportSemaphoreWin32HandleKHR(
    VkDevice                                    device,
    const VkImportSemaphoreWin32HandleInfoKHR*  pImportSemaphoreWin32HandleInfo) {
    layer_data *device_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    {
        bool skip = false;
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            skip |= intercept->PreCallValidateImportSemaphoreWin32HandleKHR(device, pImportSemaphoreWin32HandleInfo);
            if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
        }
        for (auto intercept : global_interceptor_list) {
            intercept->PreCallRecordImportSemaphoreWin32HandleKHR(device, pImportSemaphoreWin32HandleInfo);
        }
    }
    VkResult result = device_data->dispatch_table.ImportSemaphoreWin32HandleKHR(device, pImportSemaphoreWin32HandleInfo);
    if (VK_SUCCESS == result) {
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            intercept->PostCallRecordImportSemaphoreWin32HandleKHR(device, pImportSemaphoreWin32HandleInfo);
        }
    }
    return result;
}

VKAPI_ATTR VkResult VKAPI_CALL GetSemaphoreWin32HandleKHR(
    VkDevice                                    device,
    const VkSemaphoreGetWin32HandleInfoKHR*     pGetWin32HandleInfo,
    HANDLE*                                     pHandle) {
    layer_data *device_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    {
        bool skip = false;
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            skip |= intercept->PreCallValidateGetSemaphoreWin32HandleKHR(device, pGetWin32HandleInfo, pHandle);
            if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
        }
        for (auto intercept : global_interceptor_list) {
            intercept->PreCallRecordGetSemaphoreWin32HandleKHR(device, pGetWin32HandleInfo, pHandle);
        }
    }
    VkResult result = device_data->dispatch_table.GetSemaphoreWin32HandleKHR(device, pGetWin32HandleInfo, pHandle);
    if (VK_SUCCESS == result) {
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            intercept->PostCallRecordGetSemaphoreWin32HandleKHR(device, pGetWin32HandleInfo, pHandle);
        }
    }
    return result;
}
#endif // VK_USE_PLATFORM_WIN32_KHR


VKAPI_ATTR VkResult VKAPI_CALL ImportSemaphoreFdKHR(
    VkDevice                                    device,
    const VkImportSemaphoreFdInfoKHR*           pImportSemaphoreFdInfo) {
    layer_data *device_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    {
        bool skip = false;
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            skip |= intercept->PreCallValidateImportSemaphoreFdKHR(device, pImportSemaphoreFdInfo);
            if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
        }
        for (auto intercept : global_interceptor_list) {
            intercept->PreCallRecordImportSemaphoreFdKHR(device, pImportSemaphoreFdInfo);
        }
    }
    VkResult result = device_data->dispatch_table.ImportSemaphoreFdKHR(device, pImportSemaphoreFdInfo);
    if (VK_SUCCESS == result) {
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            intercept->PostCallRecordImportSemaphoreFdKHR(device, pImportSemaphoreFdInfo);
        }
    }
    return result;
}

VKAPI_ATTR VkResult VKAPI_CALL GetSemaphoreFdKHR(
    VkDevice                                    device,
    const VkSemaphoreGetFdInfoKHR*              pGetFdInfo,
    int*                                        pFd) {
    layer_data *device_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    {
        bool skip = false;
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            skip |= intercept->PreCallValidateGetSemaphoreFdKHR(device, pGetFdInfo, pFd);
            if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
        }
        for (auto intercept : global_interceptor_list) {
            intercept->PreCallRecordGetSemaphoreFdKHR(device, pGetFdInfo, pFd);
        }
    }
    VkResult result = device_data->dispatch_table.GetSemaphoreFdKHR(device, pGetFdInfo, pFd);
    if (VK_SUCCESS == result) {
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            intercept->PostCallRecordGetSemaphoreFdKHR(device, pGetFdInfo, pFd);
        }
    }
    return result;
}


VKAPI_ATTR void VKAPI_CALL CmdPushDescriptorSetKHR(
    VkCommandBuffer                             commandBuffer,
    VkPipelineBindPoint                         pipelineBindPoint,
    VkPipelineLayout                            layout,
    uint32_t                                    set,
    uint32_t                                    descriptorWriteCount,
    const VkWriteDescriptorSet*                 pDescriptorWrites) {
    layer_data *device_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    {
        bool skip = false;
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            skip |= intercept->PreCallValidateCmdPushDescriptorSetKHR(commandBuffer, pipelineBindPoint, layout, set, descriptorWriteCount, pDescriptorWrites);
            if (skip) return;
        }
        for (auto intercept : global_interceptor_list) {
            intercept->PreCallRecordCmdPushDescriptorSetKHR(commandBuffer, pipelineBindPoint, layout, set, descriptorWriteCount, pDescriptorWrites);
        }
    }
    device_data->dispatch_table.CmdPushDescriptorSetKHR(commandBuffer, pipelineBindPoint, layout, set, descriptorWriteCount, pDescriptorWrites);
    {
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            intercept->PostCallRecordCmdPushDescriptorSetKHR(commandBuffer, pipelineBindPoint, layout, set, descriptorWriteCount, pDescriptorWrites);
        }
    }
}

VKAPI_ATTR void VKAPI_CALL CmdPushDescriptorSetWithTemplateKHR(
    VkCommandBuffer                             commandBuffer,
    VkDescriptorUpdateTemplate                  descriptorUpdateTemplate,
    VkPipelineLayout                            layout,
    uint32_t                                    set,
    const void*                                 pData) {
    layer_data *device_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    {
        bool skip = false;
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            skip |= intercept->PreCallValidateCmdPushDescriptorSetWithTemplateKHR(commandBuffer, descriptorUpdateTemplate, layout, set, pData);
            if (skip) return;
        }
        for (auto intercept : global_interceptor_list) {
            intercept->PreCallRecordCmdPushDescriptorSetWithTemplateKHR(commandBuffer, descriptorUpdateTemplate, layout, set, pData);
        }
    }
    device_data->dispatch_table.CmdPushDescriptorSetWithTemplateKHR(commandBuffer, descriptorUpdateTemplate, layout, set, pData);
    {
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            intercept->PostCallRecordCmdPushDescriptorSetWithTemplateKHR(commandBuffer, descriptorUpdateTemplate, layout, set, pData);
        }
    }
}




VKAPI_ATTR VkResult VKAPI_CALL CreateDescriptorUpdateTemplateKHR(
    VkDevice                                    device,
    const VkDescriptorUpdateTemplateCreateInfo* pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkDescriptorUpdateTemplate*                 pDescriptorUpdateTemplate) {
    layer_data *device_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    {
        bool skip = false;
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            skip |= intercept->PreCallValidateCreateDescriptorUpdateTemplateKHR(device, pCreateInfo, pAllocator, pDescriptorUpdateTemplate);
            if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
        }
        for (auto intercept : global_interceptor_list) {
            intercept->PreCallRecordCreateDescriptorUpdateTemplateKHR(device, pCreateInfo, pAllocator, pDescriptorUpdateTemplate);
        }
    }
    VkResult result = device_data->dispatch_table.CreateDescriptorUpdateTemplateKHR(device, pCreateInfo, pAllocator, pDescriptorUpdateTemplate);
    if (VK_SUCCESS == result) {
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            intercept->PostCallRecordCreateDescriptorUpdateTemplateKHR(device, pCreateInfo, pAllocator, pDescriptorUpdateTemplate);
        }
    }
    return result;
}

VKAPI_ATTR void VKAPI_CALL DestroyDescriptorUpdateTemplateKHR(
    VkDevice                                    device,
    VkDescriptorUpdateTemplate                  descriptorUpdateTemplate,
    const VkAllocationCallbacks*                pAllocator) {
    layer_data *device_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    {
        bool skip = false;
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            skip |= intercept->PreCallValidateDestroyDescriptorUpdateTemplateKHR(device, descriptorUpdateTemplate, pAllocator);
            if (skip) return;
        }
        for (auto intercept : global_interceptor_list) {
            intercept->PreCallRecordDestroyDescriptorUpdateTemplateKHR(device, descriptorUpdateTemplate, pAllocator);
        }
    }
    device_data->dispatch_table.DestroyDescriptorUpdateTemplateKHR(device, descriptorUpdateTemplate, pAllocator);
    {
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            intercept->PostCallRecordDestroyDescriptorUpdateTemplateKHR(device, descriptorUpdateTemplate, pAllocator);
        }
    }
}

VKAPI_ATTR void VKAPI_CALL UpdateDescriptorSetWithTemplateKHR(
    VkDevice                                    device,
    VkDescriptorSet                             descriptorSet,
    VkDescriptorUpdateTemplate                  descriptorUpdateTemplate,
    const void*                                 pData) {
    layer_data *device_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    {
        bool skip = false;
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            skip |= intercept->PreCallValidateUpdateDescriptorSetWithTemplateKHR(device, descriptorSet, descriptorUpdateTemplate, pData);
            if (skip) return;
        }
        for (auto intercept : global_interceptor_list) {
            intercept->PreCallRecordUpdateDescriptorSetWithTemplateKHR(device, descriptorSet, descriptorUpdateTemplate, pData);
        }
    }
    device_data->dispatch_table.UpdateDescriptorSetWithTemplateKHR(device, descriptorSet, descriptorUpdateTemplate, pData);
    {
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            intercept->PostCallRecordUpdateDescriptorSetWithTemplateKHR(device, descriptorSet, descriptorUpdateTemplate, pData);
        }
    }
}


VKAPI_ATTR VkResult VKAPI_CALL CreateRenderPass2KHR(
    VkDevice                                    device,
    const VkRenderPassCreateInfo2KHR*           pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkRenderPass*                               pRenderPass) {
    layer_data *device_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    {
        bool skip = false;
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            skip |= intercept->PreCallValidateCreateRenderPass2KHR(device, pCreateInfo, pAllocator, pRenderPass);
            if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
        }
        for (auto intercept : global_interceptor_list) {
            intercept->PreCallRecordCreateRenderPass2KHR(device, pCreateInfo, pAllocator, pRenderPass);
        }
    }
    VkResult result = device_data->dispatch_table.CreateRenderPass2KHR(device, pCreateInfo, pAllocator, pRenderPass);
    if (VK_SUCCESS == result) {
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            intercept->PostCallRecordCreateRenderPass2KHR(device, pCreateInfo, pAllocator, pRenderPass);
        }
    }
    return result;
}

VKAPI_ATTR void VKAPI_CALL CmdBeginRenderPass2KHR(
    VkCommandBuffer                             commandBuffer,
    const VkRenderPassBeginInfo*                pRenderPassBegin,
    const VkSubpassBeginInfoKHR*                pSubpassBeginInfo) {
    layer_data *device_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    {
        bool skip = false;
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            skip |= intercept->PreCallValidateCmdBeginRenderPass2KHR(commandBuffer, pRenderPassBegin, pSubpassBeginInfo);
            if (skip) return;
        }
        for (auto intercept : global_interceptor_list) {
            intercept->PreCallRecordCmdBeginRenderPass2KHR(commandBuffer, pRenderPassBegin, pSubpassBeginInfo);
        }
    }
    device_data->dispatch_table.CmdBeginRenderPass2KHR(commandBuffer, pRenderPassBegin, pSubpassBeginInfo);
    {
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            intercept->PostCallRecordCmdBeginRenderPass2KHR(commandBuffer, pRenderPassBegin, pSubpassBeginInfo);
        }
    }
}

VKAPI_ATTR void VKAPI_CALL CmdNextSubpass2KHR(
    VkCommandBuffer                             commandBuffer,
    const VkSubpassBeginInfoKHR*                pSubpassBeginInfo,
    const VkSubpassEndInfoKHR*                  pSubpassEndInfo) {
    layer_data *device_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    {
        bool skip = false;
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            skip |= intercept->PreCallValidateCmdNextSubpass2KHR(commandBuffer, pSubpassBeginInfo, pSubpassEndInfo);
            if (skip) return;
        }
        for (auto intercept : global_interceptor_list) {
            intercept->PreCallRecordCmdNextSubpass2KHR(commandBuffer, pSubpassBeginInfo, pSubpassEndInfo);
        }
    }
    device_data->dispatch_table.CmdNextSubpass2KHR(commandBuffer, pSubpassBeginInfo, pSubpassEndInfo);
    {
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            intercept->PostCallRecordCmdNextSubpass2KHR(commandBuffer, pSubpassBeginInfo, pSubpassEndInfo);
        }
    }
}

VKAPI_ATTR void VKAPI_CALL CmdEndRenderPass2KHR(
    VkCommandBuffer                             commandBuffer,
    const VkSubpassEndInfoKHR*                  pSubpassEndInfo) {
    layer_data *device_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    {
        bool skip = false;
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            skip |= intercept->PreCallValidateCmdEndRenderPass2KHR(commandBuffer, pSubpassEndInfo);
            if (skip) return;
        }
        for (auto intercept : global_interceptor_list) {
            intercept->PreCallRecordCmdEndRenderPass2KHR(commandBuffer, pSubpassEndInfo);
        }
    }
    device_data->dispatch_table.CmdEndRenderPass2KHR(commandBuffer, pSubpassEndInfo);
    {
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            intercept->PostCallRecordCmdEndRenderPass2KHR(commandBuffer, pSubpassEndInfo);
        }
    }
}


VKAPI_ATTR VkResult VKAPI_CALL GetSwapchainStatusKHR(
    VkDevice                                    device,
    VkSwapchainKHR                              swapchain) {
    layer_data *device_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    {
        bool skip = false;
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            skip |= intercept->PreCallValidateGetSwapchainStatusKHR(device, swapchain);
            if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
        }
        for (auto intercept : global_interceptor_list) {
            intercept->PreCallRecordGetSwapchainStatusKHR(device, swapchain);
        }
    }
    VkResult result = device_data->dispatch_table.GetSwapchainStatusKHR(device, swapchain);
    if (VK_SUCCESS == result) {
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            intercept->PostCallRecordGetSwapchainStatusKHR(device, swapchain);
        }
    }
    return result;
}


VKAPI_ATTR void VKAPI_CALL GetPhysicalDeviceExternalFencePropertiesKHR(
    VkPhysicalDevice                            physicalDevice,
    const VkPhysicalDeviceExternalFenceInfo*    pExternalFenceInfo,
    VkExternalFenceProperties*                  pExternalFenceProperties) {
    instance_layer_data *instance_data = GetLayerDataPtr(get_dispatch_key(physicalDevice), instance_layer_data_map);
    {
        bool skip = false;
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            skip |= intercept->PreCallValidateGetPhysicalDeviceExternalFencePropertiesKHR(physicalDevice, pExternalFenceInfo, pExternalFenceProperties);
            if (skip) return;
        }
        for (auto intercept : global_interceptor_list) {
            intercept->PreCallRecordGetPhysicalDeviceExternalFencePropertiesKHR(physicalDevice, pExternalFenceInfo, pExternalFenceProperties);
        }
    }
    instance_data->dispatch_table.GetPhysicalDeviceExternalFencePropertiesKHR(physicalDevice, pExternalFenceInfo, pExternalFenceProperties);
    {
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            intercept->PostCallRecordGetPhysicalDeviceExternalFencePropertiesKHR(physicalDevice, pExternalFenceInfo, pExternalFenceProperties);
        }
    }
}


#ifdef VK_USE_PLATFORM_WIN32_KHR

VKAPI_ATTR VkResult VKAPI_CALL ImportFenceWin32HandleKHR(
    VkDevice                                    device,
    const VkImportFenceWin32HandleInfoKHR*      pImportFenceWin32HandleInfo) {
    layer_data *device_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    {
        bool skip = false;
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            skip |= intercept->PreCallValidateImportFenceWin32HandleKHR(device, pImportFenceWin32HandleInfo);
            if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
        }
        for (auto intercept : global_interceptor_list) {
            intercept->PreCallRecordImportFenceWin32HandleKHR(device, pImportFenceWin32HandleInfo);
        }
    }
    VkResult result = device_data->dispatch_table.ImportFenceWin32HandleKHR(device, pImportFenceWin32HandleInfo);
    if (VK_SUCCESS == result) {
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            intercept->PostCallRecordImportFenceWin32HandleKHR(device, pImportFenceWin32HandleInfo);
        }
    }
    return result;
}

VKAPI_ATTR VkResult VKAPI_CALL GetFenceWin32HandleKHR(
    VkDevice                                    device,
    const VkFenceGetWin32HandleInfoKHR*         pGetWin32HandleInfo,
    HANDLE*                                     pHandle) {
    layer_data *device_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    {
        bool skip = false;
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            skip |= intercept->PreCallValidateGetFenceWin32HandleKHR(device, pGetWin32HandleInfo, pHandle);
            if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
        }
        for (auto intercept : global_interceptor_list) {
            intercept->PreCallRecordGetFenceWin32HandleKHR(device, pGetWin32HandleInfo, pHandle);
        }
    }
    VkResult result = device_data->dispatch_table.GetFenceWin32HandleKHR(device, pGetWin32HandleInfo, pHandle);
    if (VK_SUCCESS == result) {
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            intercept->PostCallRecordGetFenceWin32HandleKHR(device, pGetWin32HandleInfo, pHandle);
        }
    }
    return result;
}
#endif // VK_USE_PLATFORM_WIN32_KHR


VKAPI_ATTR VkResult VKAPI_CALL ImportFenceFdKHR(
    VkDevice                                    device,
    const VkImportFenceFdInfoKHR*               pImportFenceFdInfo) {
    layer_data *device_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    {
        bool skip = false;
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            skip |= intercept->PreCallValidateImportFenceFdKHR(device, pImportFenceFdInfo);
            if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
        }
        for (auto intercept : global_interceptor_list) {
            intercept->PreCallRecordImportFenceFdKHR(device, pImportFenceFdInfo);
        }
    }
    VkResult result = device_data->dispatch_table.ImportFenceFdKHR(device, pImportFenceFdInfo);
    if (VK_SUCCESS == result) {
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            intercept->PostCallRecordImportFenceFdKHR(device, pImportFenceFdInfo);
        }
    }
    return result;
}

VKAPI_ATTR VkResult VKAPI_CALL GetFenceFdKHR(
    VkDevice                                    device,
    const VkFenceGetFdInfoKHR*                  pGetFdInfo,
    int*                                        pFd) {
    layer_data *device_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    {
        bool skip = false;
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            skip |= intercept->PreCallValidateGetFenceFdKHR(device, pGetFdInfo, pFd);
            if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
        }
        for (auto intercept : global_interceptor_list) {
            intercept->PreCallRecordGetFenceFdKHR(device, pGetFdInfo, pFd);
        }
    }
    VkResult result = device_data->dispatch_table.GetFenceFdKHR(device, pGetFdInfo, pFd);
    if (VK_SUCCESS == result) {
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            intercept->PostCallRecordGetFenceFdKHR(device, pGetFdInfo, pFd);
        }
    }
    return result;
}



VKAPI_ATTR VkResult VKAPI_CALL GetPhysicalDeviceSurfaceCapabilities2KHR(
    VkPhysicalDevice                            physicalDevice,
    const VkPhysicalDeviceSurfaceInfo2KHR*      pSurfaceInfo,
    VkSurfaceCapabilities2KHR*                  pSurfaceCapabilities) {
    instance_layer_data *instance_data = GetLayerDataPtr(get_dispatch_key(physicalDevice), instance_layer_data_map);
    {
        bool skip = false;
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            skip |= intercept->PreCallValidateGetPhysicalDeviceSurfaceCapabilities2KHR(physicalDevice, pSurfaceInfo, pSurfaceCapabilities);
            if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
        }
        for (auto intercept : global_interceptor_list) {
            intercept->PreCallRecordGetPhysicalDeviceSurfaceCapabilities2KHR(physicalDevice, pSurfaceInfo, pSurfaceCapabilities);
        }
    }
    VkResult result = instance_data->dispatch_table.GetPhysicalDeviceSurfaceCapabilities2KHR(physicalDevice, pSurfaceInfo, pSurfaceCapabilities);
    if (VK_SUCCESS == result) {
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            intercept->PostCallRecordGetPhysicalDeviceSurfaceCapabilities2KHR(physicalDevice, pSurfaceInfo, pSurfaceCapabilities);
        }
    }
    return result;
}

VKAPI_ATTR VkResult VKAPI_CALL GetPhysicalDeviceSurfaceFormats2KHR(
    VkPhysicalDevice                            physicalDevice,
    const VkPhysicalDeviceSurfaceInfo2KHR*      pSurfaceInfo,
    uint32_t*                                   pSurfaceFormatCount,
    VkSurfaceFormat2KHR*                        pSurfaceFormats) {
    instance_layer_data *instance_data = GetLayerDataPtr(get_dispatch_key(physicalDevice), instance_layer_data_map);
    {
        bool skip = false;
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            skip |= intercept->PreCallValidateGetPhysicalDeviceSurfaceFormats2KHR(physicalDevice, pSurfaceInfo, pSurfaceFormatCount, pSurfaceFormats);
            if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
        }
        for (auto intercept : global_interceptor_list) {
            intercept->PreCallRecordGetPhysicalDeviceSurfaceFormats2KHR(physicalDevice, pSurfaceInfo, pSurfaceFormatCount, pSurfaceFormats);
        }
    }
    VkResult result = instance_data->dispatch_table.GetPhysicalDeviceSurfaceFormats2KHR(physicalDevice, pSurfaceInfo, pSurfaceFormatCount, pSurfaceFormats);
    if (VK_SUCCESS == result) {
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            intercept->PostCallRecordGetPhysicalDeviceSurfaceFormats2KHR(physicalDevice, pSurfaceInfo, pSurfaceFormatCount, pSurfaceFormats);
        }
    }
    return result;
}



VKAPI_ATTR VkResult VKAPI_CALL GetPhysicalDeviceDisplayProperties2KHR(
    VkPhysicalDevice                            physicalDevice,
    uint32_t*                                   pPropertyCount,
    VkDisplayProperties2KHR*                    pProperties) {
    instance_layer_data *instance_data = GetLayerDataPtr(get_dispatch_key(physicalDevice), instance_layer_data_map);
    {
        bool skip = false;
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            skip |= intercept->PreCallValidateGetPhysicalDeviceDisplayProperties2KHR(physicalDevice, pPropertyCount, pProperties);
            if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
        }
        for (auto intercept : global_interceptor_list) {
            intercept->PreCallRecordGetPhysicalDeviceDisplayProperties2KHR(physicalDevice, pPropertyCount, pProperties);
        }
    }
    VkResult result = instance_data->dispatch_table.GetPhysicalDeviceDisplayProperties2KHR(physicalDevice, pPropertyCount, pProperties);
    if (VK_SUCCESS == result) {
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            intercept->PostCallRecordGetPhysicalDeviceDisplayProperties2KHR(physicalDevice, pPropertyCount, pProperties);
        }
    }
    return result;
}

VKAPI_ATTR VkResult VKAPI_CALL GetPhysicalDeviceDisplayPlaneProperties2KHR(
    VkPhysicalDevice                            physicalDevice,
    uint32_t*                                   pPropertyCount,
    VkDisplayPlaneProperties2KHR*               pProperties) {
    instance_layer_data *instance_data = GetLayerDataPtr(get_dispatch_key(physicalDevice), instance_layer_data_map);
    {
        bool skip = false;
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            skip |= intercept->PreCallValidateGetPhysicalDeviceDisplayPlaneProperties2KHR(physicalDevice, pPropertyCount, pProperties);
            if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
        }
        for (auto intercept : global_interceptor_list) {
            intercept->PreCallRecordGetPhysicalDeviceDisplayPlaneProperties2KHR(physicalDevice, pPropertyCount, pProperties);
        }
    }
    VkResult result = instance_data->dispatch_table.GetPhysicalDeviceDisplayPlaneProperties2KHR(physicalDevice, pPropertyCount, pProperties);
    if (VK_SUCCESS == result) {
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            intercept->PostCallRecordGetPhysicalDeviceDisplayPlaneProperties2KHR(physicalDevice, pPropertyCount, pProperties);
        }
    }
    return result;
}

VKAPI_ATTR VkResult VKAPI_CALL GetDisplayModeProperties2KHR(
    VkPhysicalDevice                            physicalDevice,
    VkDisplayKHR                                display,
    uint32_t*                                   pPropertyCount,
    VkDisplayModeProperties2KHR*                pProperties) {
    instance_layer_data *instance_data = GetLayerDataPtr(get_dispatch_key(physicalDevice), instance_layer_data_map);
    {
        bool skip = false;
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            skip |= intercept->PreCallValidateGetDisplayModeProperties2KHR(physicalDevice, display, pPropertyCount, pProperties);
            if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
        }
        for (auto intercept : global_interceptor_list) {
            intercept->PreCallRecordGetDisplayModeProperties2KHR(physicalDevice, display, pPropertyCount, pProperties);
        }
    }
    VkResult result = instance_data->dispatch_table.GetDisplayModeProperties2KHR(physicalDevice, display, pPropertyCount, pProperties);
    if (VK_SUCCESS == result) {
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            intercept->PostCallRecordGetDisplayModeProperties2KHR(physicalDevice, display, pPropertyCount, pProperties);
        }
    }
    return result;
}

VKAPI_ATTR VkResult VKAPI_CALL GetDisplayPlaneCapabilities2KHR(
    VkPhysicalDevice                            physicalDevice,
    const VkDisplayPlaneInfo2KHR*               pDisplayPlaneInfo,
    VkDisplayPlaneCapabilities2KHR*             pCapabilities) {
    instance_layer_data *instance_data = GetLayerDataPtr(get_dispatch_key(physicalDevice), instance_layer_data_map);
    {
        bool skip = false;
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            skip |= intercept->PreCallValidateGetDisplayPlaneCapabilities2KHR(physicalDevice, pDisplayPlaneInfo, pCapabilities);
            if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
        }
        for (auto intercept : global_interceptor_list) {
            intercept->PreCallRecordGetDisplayPlaneCapabilities2KHR(physicalDevice, pDisplayPlaneInfo, pCapabilities);
        }
    }
    VkResult result = instance_data->dispatch_table.GetDisplayPlaneCapabilities2KHR(physicalDevice, pDisplayPlaneInfo, pCapabilities);
    if (VK_SUCCESS == result) {
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            intercept->PostCallRecordGetDisplayPlaneCapabilities2KHR(physicalDevice, pDisplayPlaneInfo, pCapabilities);
        }
    }
    return result;
}





VKAPI_ATTR void VKAPI_CALL GetImageMemoryRequirements2KHR(
    VkDevice                                    device,
    const VkImageMemoryRequirementsInfo2*       pInfo,
    VkMemoryRequirements2*                      pMemoryRequirements) {
    layer_data *device_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    {
        bool skip = false;
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            skip |= intercept->PreCallValidateGetImageMemoryRequirements2KHR(device, pInfo, pMemoryRequirements);
            if (skip) return;
        }
        for (auto intercept : global_interceptor_list) {
            intercept->PreCallRecordGetImageMemoryRequirements2KHR(device, pInfo, pMemoryRequirements);
        }
    }
    device_data->dispatch_table.GetImageMemoryRequirements2KHR(device, pInfo, pMemoryRequirements);
    {
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            intercept->PostCallRecordGetImageMemoryRequirements2KHR(device, pInfo, pMemoryRequirements);
        }
    }
}

VKAPI_ATTR void VKAPI_CALL GetBufferMemoryRequirements2KHR(
    VkDevice                                    device,
    const VkBufferMemoryRequirementsInfo2*      pInfo,
    VkMemoryRequirements2*                      pMemoryRequirements) {
    layer_data *device_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    {
        bool skip = false;
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            skip |= intercept->PreCallValidateGetBufferMemoryRequirements2KHR(device, pInfo, pMemoryRequirements);
            if (skip) return;
        }
        for (auto intercept : global_interceptor_list) {
            intercept->PreCallRecordGetBufferMemoryRequirements2KHR(device, pInfo, pMemoryRequirements);
        }
    }
    device_data->dispatch_table.GetBufferMemoryRequirements2KHR(device, pInfo, pMemoryRequirements);
    {
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            intercept->PostCallRecordGetBufferMemoryRequirements2KHR(device, pInfo, pMemoryRequirements);
        }
    }
}

VKAPI_ATTR void VKAPI_CALL GetImageSparseMemoryRequirements2KHR(
    VkDevice                                    device,
    const VkImageSparseMemoryRequirementsInfo2* pInfo,
    uint32_t*                                   pSparseMemoryRequirementCount,
    VkSparseImageMemoryRequirements2*           pSparseMemoryRequirements) {
    layer_data *device_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    {
        bool skip = false;
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            skip |= intercept->PreCallValidateGetImageSparseMemoryRequirements2KHR(device, pInfo, pSparseMemoryRequirementCount, pSparseMemoryRequirements);
            if (skip) return;
        }
        for (auto intercept : global_interceptor_list) {
            intercept->PreCallRecordGetImageSparseMemoryRequirements2KHR(device, pInfo, pSparseMemoryRequirementCount, pSparseMemoryRequirements);
        }
    }
    device_data->dispatch_table.GetImageSparseMemoryRequirements2KHR(device, pInfo, pSparseMemoryRequirementCount, pSparseMemoryRequirements);
    {
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            intercept->PostCallRecordGetImageSparseMemoryRequirements2KHR(device, pInfo, pSparseMemoryRequirementCount, pSparseMemoryRequirements);
        }
    }
}



VKAPI_ATTR VkResult VKAPI_CALL CreateSamplerYcbcrConversionKHR(
    VkDevice                                    device,
    const VkSamplerYcbcrConversionCreateInfo*   pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkSamplerYcbcrConversion*                   pYcbcrConversion) {
    layer_data *device_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    {
        bool skip = false;
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            skip |= intercept->PreCallValidateCreateSamplerYcbcrConversionKHR(device, pCreateInfo, pAllocator, pYcbcrConversion);
            if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
        }
        for (auto intercept : global_interceptor_list) {
            intercept->PreCallRecordCreateSamplerYcbcrConversionKHR(device, pCreateInfo, pAllocator, pYcbcrConversion);
        }
    }
    VkResult result = device_data->dispatch_table.CreateSamplerYcbcrConversionKHR(device, pCreateInfo, pAllocator, pYcbcrConversion);
    if (VK_SUCCESS == result) {
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            intercept->PostCallRecordCreateSamplerYcbcrConversionKHR(device, pCreateInfo, pAllocator, pYcbcrConversion);
        }
    }
    return result;
}

VKAPI_ATTR void VKAPI_CALL DestroySamplerYcbcrConversionKHR(
    VkDevice                                    device,
    VkSamplerYcbcrConversion                    ycbcrConversion,
    const VkAllocationCallbacks*                pAllocator) {
    layer_data *device_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    {
        bool skip = false;
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            skip |= intercept->PreCallValidateDestroySamplerYcbcrConversionKHR(device, ycbcrConversion, pAllocator);
            if (skip) return;
        }
        for (auto intercept : global_interceptor_list) {
            intercept->PreCallRecordDestroySamplerYcbcrConversionKHR(device, ycbcrConversion, pAllocator);
        }
    }
    device_data->dispatch_table.DestroySamplerYcbcrConversionKHR(device, ycbcrConversion, pAllocator);
    {
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            intercept->PostCallRecordDestroySamplerYcbcrConversionKHR(device, ycbcrConversion, pAllocator);
        }
    }
}


VKAPI_ATTR VkResult VKAPI_CALL BindBufferMemory2KHR(
    VkDevice                                    device,
    uint32_t                                    bindInfoCount,
    const VkBindBufferMemoryInfo*               pBindInfos) {
    layer_data *device_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    {
        bool skip = false;
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            skip |= intercept->PreCallValidateBindBufferMemory2KHR(device, bindInfoCount, pBindInfos);
            if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
        }
        for (auto intercept : global_interceptor_list) {
            intercept->PreCallRecordBindBufferMemory2KHR(device, bindInfoCount, pBindInfos);
        }
    }
    VkResult result = device_data->dispatch_table.BindBufferMemory2KHR(device, bindInfoCount, pBindInfos);
    if (VK_SUCCESS == result) {
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            intercept->PostCallRecordBindBufferMemory2KHR(device, bindInfoCount, pBindInfos);
        }
    }
    return result;
}

VKAPI_ATTR VkResult VKAPI_CALL BindImageMemory2KHR(
    VkDevice                                    device,
    uint32_t                                    bindInfoCount,
    const VkBindImageMemoryInfo*                pBindInfos) {
    layer_data *device_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    {
        bool skip = false;
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            skip |= intercept->PreCallValidateBindImageMemory2KHR(device, bindInfoCount, pBindInfos);
            if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
        }
        for (auto intercept : global_interceptor_list) {
            intercept->PreCallRecordBindImageMemory2KHR(device, bindInfoCount, pBindInfos);
        }
    }
    VkResult result = device_data->dispatch_table.BindImageMemory2KHR(device, bindInfoCount, pBindInfos);
    if (VK_SUCCESS == result) {
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            intercept->PostCallRecordBindImageMemory2KHR(device, bindInfoCount, pBindInfos);
        }
    }
    return result;
}


VKAPI_ATTR void VKAPI_CALL GetDescriptorSetLayoutSupportKHR(
    VkDevice                                    device,
    const VkDescriptorSetLayoutCreateInfo*      pCreateInfo,
    VkDescriptorSetLayoutSupport*               pSupport) {
    layer_data *device_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    {
        bool skip = false;
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            skip |= intercept->PreCallValidateGetDescriptorSetLayoutSupportKHR(device, pCreateInfo, pSupport);
            if (skip) return;
        }
        for (auto intercept : global_interceptor_list) {
            intercept->PreCallRecordGetDescriptorSetLayoutSupportKHR(device, pCreateInfo, pSupport);
        }
    }
    device_data->dispatch_table.GetDescriptorSetLayoutSupportKHR(device, pCreateInfo, pSupport);
    {
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            intercept->PostCallRecordGetDescriptorSetLayoutSupportKHR(device, pCreateInfo, pSupport);
        }
    }
}


VKAPI_ATTR void VKAPI_CALL CmdDrawIndirectCountKHR(
    VkCommandBuffer                             commandBuffer,
    VkBuffer                                    buffer,
    VkDeviceSize                                offset,
    VkBuffer                                    countBuffer,
    VkDeviceSize                                countBufferOffset,
    uint32_t                                    maxDrawCount,
    uint32_t                                    stride) {
    layer_data *device_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    {
        bool skip = false;
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            skip |= intercept->PreCallValidateCmdDrawIndirectCountKHR(commandBuffer, buffer, offset, countBuffer, countBufferOffset, maxDrawCount, stride);
            if (skip) return;
        }
        for (auto intercept : global_interceptor_list) {
            intercept->PreCallRecordCmdDrawIndirectCountKHR(commandBuffer, buffer, offset, countBuffer, countBufferOffset, maxDrawCount, stride);
        }
    }
    device_data->dispatch_table.CmdDrawIndirectCountKHR(commandBuffer, buffer, offset, countBuffer, countBufferOffset, maxDrawCount, stride);
    {
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            intercept->PostCallRecordCmdDrawIndirectCountKHR(commandBuffer, buffer, offset, countBuffer, countBufferOffset, maxDrawCount, stride);
        }
    }
}

VKAPI_ATTR void VKAPI_CALL CmdDrawIndexedIndirectCountKHR(
    VkCommandBuffer                             commandBuffer,
    VkBuffer                                    buffer,
    VkDeviceSize                                offset,
    VkBuffer                                    countBuffer,
    VkDeviceSize                                countBufferOffset,
    uint32_t                                    maxDrawCount,
    uint32_t                                    stride) {
    layer_data *device_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    {
        bool skip = false;
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            skip |= intercept->PreCallValidateCmdDrawIndexedIndirectCountKHR(commandBuffer, buffer, offset, countBuffer, countBufferOffset, maxDrawCount, stride);
            if (skip) return;
        }
        for (auto intercept : global_interceptor_list) {
            intercept->PreCallRecordCmdDrawIndexedIndirectCountKHR(commandBuffer, buffer, offset, countBuffer, countBufferOffset, maxDrawCount, stride);
        }
    }
    device_data->dispatch_table.CmdDrawIndexedIndirectCountKHR(commandBuffer, buffer, offset, countBuffer, countBufferOffset, maxDrawCount, stride);
    {
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            intercept->PostCallRecordCmdDrawIndexedIndirectCountKHR(commandBuffer, buffer, offset, countBuffer, countBufferOffset, maxDrawCount, stride);
        }
    }
}






VKAPI_ATTR void VKAPI_CALL DebugReportMessageEXT(
    VkInstance                                  instance,
    VkDebugReportFlagsEXT                       flags,
    VkDebugReportObjectTypeEXT                  objectType,
    uint64_t                                    object,
    size_t                                      location,
    int32_t                                     messageCode,
    const char*                                 pLayerPrefix,
    const char*                                 pMessage) {
    instance_layer_data *instance_data = GetLayerDataPtr(get_dispatch_key(instance), instance_layer_data_map);
    {
        bool skip = false;
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            skip |= intercept->PreCallValidateDebugReportMessageEXT(instance, flags, objectType, object, location, messageCode, pLayerPrefix, pMessage);
            if (skip) return;
        }
        for (auto intercept : global_interceptor_list) {
            intercept->PreCallRecordDebugReportMessageEXT(instance, flags, objectType, object, location, messageCode, pLayerPrefix, pMessage);
        }
    }
    instance_data->dispatch_table.DebugReportMessageEXT(instance, flags, objectType, object, location, messageCode, pLayerPrefix, pMessage);
    {
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            intercept->PostCallRecordDebugReportMessageEXT(instance, flags, objectType, object, location, messageCode, pLayerPrefix, pMessage);
        }
    }
}








VKAPI_ATTR VkResult VKAPI_CALL DebugMarkerSetObjectTagEXT(
    VkDevice                                    device,
    const VkDebugMarkerObjectTagInfoEXT*        pTagInfo) {
    layer_data *device_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    {
        bool skip = false;
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            skip |= intercept->PreCallValidateDebugMarkerSetObjectTagEXT(device, pTagInfo);
            if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
        }
        for (auto intercept : global_interceptor_list) {
            intercept->PreCallRecordDebugMarkerSetObjectTagEXT(device, pTagInfo);
        }
    }
    VkResult result = device_data->dispatch_table.DebugMarkerSetObjectTagEXT(device, pTagInfo);
    if (VK_SUCCESS == result) {
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            intercept->PostCallRecordDebugMarkerSetObjectTagEXT(device, pTagInfo);
        }
    }
    return result;
}

VKAPI_ATTR VkResult VKAPI_CALL DebugMarkerSetObjectNameEXT(
    VkDevice                                    device,
    const VkDebugMarkerObjectNameInfoEXT*       pNameInfo) {
    layer_data *device_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    {
        bool skip = false;
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            skip |= intercept->PreCallValidateDebugMarkerSetObjectNameEXT(device, pNameInfo);
            if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
        }
        for (auto intercept : global_interceptor_list) {
            intercept->PreCallRecordDebugMarkerSetObjectNameEXT(device, pNameInfo);
        }
    }
    VkResult result = device_data->dispatch_table.DebugMarkerSetObjectNameEXT(device, pNameInfo);
    if (VK_SUCCESS == result) {
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            intercept->PostCallRecordDebugMarkerSetObjectNameEXT(device, pNameInfo);
        }
    }
    return result;
}

VKAPI_ATTR void VKAPI_CALL CmdDebugMarkerBeginEXT(
    VkCommandBuffer                             commandBuffer,
    const VkDebugMarkerMarkerInfoEXT*           pMarkerInfo) {
    layer_data *device_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    {
        bool skip = false;
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            skip |= intercept->PreCallValidateCmdDebugMarkerBeginEXT(commandBuffer, pMarkerInfo);
            if (skip) return;
        }
        for (auto intercept : global_interceptor_list) {
            intercept->PreCallRecordCmdDebugMarkerBeginEXT(commandBuffer, pMarkerInfo);
        }
    }
    device_data->dispatch_table.CmdDebugMarkerBeginEXT(commandBuffer, pMarkerInfo);
    {
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            intercept->PostCallRecordCmdDebugMarkerBeginEXT(commandBuffer, pMarkerInfo);
        }
    }
}

VKAPI_ATTR void VKAPI_CALL CmdDebugMarkerEndEXT(
    VkCommandBuffer                             commandBuffer) {
    layer_data *device_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    {
        bool skip = false;
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            skip |= intercept->PreCallValidateCmdDebugMarkerEndEXT(commandBuffer);
            if (skip) return;
        }
        for (auto intercept : global_interceptor_list) {
            intercept->PreCallRecordCmdDebugMarkerEndEXT(commandBuffer);
        }
    }
    device_data->dispatch_table.CmdDebugMarkerEndEXT(commandBuffer);
    {
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            intercept->PostCallRecordCmdDebugMarkerEndEXT(commandBuffer);
        }
    }
}

VKAPI_ATTR void VKAPI_CALL CmdDebugMarkerInsertEXT(
    VkCommandBuffer                             commandBuffer,
    const VkDebugMarkerMarkerInfoEXT*           pMarkerInfo) {
    layer_data *device_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    {
        bool skip = false;
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            skip |= intercept->PreCallValidateCmdDebugMarkerInsertEXT(commandBuffer, pMarkerInfo);
            if (skip) return;
        }
        for (auto intercept : global_interceptor_list) {
            intercept->PreCallRecordCmdDebugMarkerInsertEXT(commandBuffer, pMarkerInfo);
        }
    }
    device_data->dispatch_table.CmdDebugMarkerInsertEXT(commandBuffer, pMarkerInfo);
    {
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            intercept->PostCallRecordCmdDebugMarkerInsertEXT(commandBuffer, pMarkerInfo);
        }
    }
}




VKAPI_ATTR void VKAPI_CALL CmdDrawIndirectCountAMD(
    VkCommandBuffer                             commandBuffer,
    VkBuffer                                    buffer,
    VkDeviceSize                                offset,
    VkBuffer                                    countBuffer,
    VkDeviceSize                                countBufferOffset,
    uint32_t                                    maxDrawCount,
    uint32_t                                    stride) {
    layer_data *device_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    {
        bool skip = false;
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            skip |= intercept->PreCallValidateCmdDrawIndirectCountAMD(commandBuffer, buffer, offset, countBuffer, countBufferOffset, maxDrawCount, stride);
            if (skip) return;
        }
        for (auto intercept : global_interceptor_list) {
            intercept->PreCallRecordCmdDrawIndirectCountAMD(commandBuffer, buffer, offset, countBuffer, countBufferOffset, maxDrawCount, stride);
        }
    }
    device_data->dispatch_table.CmdDrawIndirectCountAMD(commandBuffer, buffer, offset, countBuffer, countBufferOffset, maxDrawCount, stride);
    {
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            intercept->PostCallRecordCmdDrawIndirectCountAMD(commandBuffer, buffer, offset, countBuffer, countBufferOffset, maxDrawCount, stride);
        }
    }
}

VKAPI_ATTR void VKAPI_CALL CmdDrawIndexedIndirectCountAMD(
    VkCommandBuffer                             commandBuffer,
    VkBuffer                                    buffer,
    VkDeviceSize                                offset,
    VkBuffer                                    countBuffer,
    VkDeviceSize                                countBufferOffset,
    uint32_t                                    maxDrawCount,
    uint32_t                                    stride) {
    layer_data *device_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    {
        bool skip = false;
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            skip |= intercept->PreCallValidateCmdDrawIndexedIndirectCountAMD(commandBuffer, buffer, offset, countBuffer, countBufferOffset, maxDrawCount, stride);
            if (skip) return;
        }
        for (auto intercept : global_interceptor_list) {
            intercept->PreCallRecordCmdDrawIndexedIndirectCountAMD(commandBuffer, buffer, offset, countBuffer, countBufferOffset, maxDrawCount, stride);
        }
    }
    device_data->dispatch_table.CmdDrawIndexedIndirectCountAMD(commandBuffer, buffer, offset, countBuffer, countBufferOffset, maxDrawCount, stride);
    {
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            intercept->PostCallRecordCmdDrawIndexedIndirectCountAMD(commandBuffer, buffer, offset, countBuffer, countBufferOffset, maxDrawCount, stride);
        }
    }
}






VKAPI_ATTR VkResult VKAPI_CALL GetShaderInfoAMD(
    VkDevice                                    device,
    VkPipeline                                  pipeline,
    VkShaderStageFlagBits                       shaderStage,
    VkShaderInfoTypeAMD                         infoType,
    size_t*                                     pInfoSize,
    void*                                       pInfo) {
    layer_data *device_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    {
        bool skip = false;
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            skip |= intercept->PreCallValidateGetShaderInfoAMD(device, pipeline, shaderStage, infoType, pInfoSize, pInfo);
            if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
        }
        for (auto intercept : global_interceptor_list) {
            intercept->PreCallRecordGetShaderInfoAMD(device, pipeline, shaderStage, infoType, pInfoSize, pInfo);
        }
    }
    VkResult result = device_data->dispatch_table.GetShaderInfoAMD(device, pipeline, shaderStage, infoType, pInfoSize, pInfo);
    if (VK_SUCCESS == result) {
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            intercept->PostCallRecordGetShaderInfoAMD(device, pipeline, shaderStage, infoType, pInfoSize, pInfo);
        }
    }
    return result;
}





VKAPI_ATTR VkResult VKAPI_CALL GetPhysicalDeviceExternalImageFormatPropertiesNV(
    VkPhysicalDevice                            physicalDevice,
    VkFormat                                    format,
    VkImageType                                 type,
    VkImageTiling                               tiling,
    VkImageUsageFlags                           usage,
    VkImageCreateFlags                          flags,
    VkExternalMemoryHandleTypeFlagsNV           externalHandleType,
    VkExternalImageFormatPropertiesNV*          pExternalImageFormatProperties) {
    instance_layer_data *instance_data = GetLayerDataPtr(get_dispatch_key(physicalDevice), instance_layer_data_map);
    {
        bool skip = false;
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            skip |= intercept->PreCallValidateGetPhysicalDeviceExternalImageFormatPropertiesNV(physicalDevice, format, type, tiling, usage, flags, externalHandleType, pExternalImageFormatProperties);
            if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
        }
        for (auto intercept : global_interceptor_list) {
            intercept->PreCallRecordGetPhysicalDeviceExternalImageFormatPropertiesNV(physicalDevice, format, type, tiling, usage, flags, externalHandleType, pExternalImageFormatProperties);
        }
    }
    VkResult result = instance_data->dispatch_table.GetPhysicalDeviceExternalImageFormatPropertiesNV(physicalDevice, format, type, tiling, usage, flags, externalHandleType, pExternalImageFormatProperties);
    if (VK_SUCCESS == result) {
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            intercept->PostCallRecordGetPhysicalDeviceExternalImageFormatPropertiesNV(physicalDevice, format, type, tiling, usage, flags, externalHandleType, pExternalImageFormatProperties);
        }
    }
    return result;
}


#ifdef VK_USE_PLATFORM_WIN32_KHR

VKAPI_ATTR VkResult VKAPI_CALL GetMemoryWin32HandleNV(
    VkDevice                                    device,
    VkDeviceMemory                              memory,
    VkExternalMemoryHandleTypeFlagsNV           handleType,
    HANDLE*                                     pHandle) {
    layer_data *device_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    {
        bool skip = false;
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            skip |= intercept->PreCallValidateGetMemoryWin32HandleNV(device, memory, handleType, pHandle);
            if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
        }
        for (auto intercept : global_interceptor_list) {
            intercept->PreCallRecordGetMemoryWin32HandleNV(device, memory, handleType, pHandle);
        }
    }
    VkResult result = device_data->dispatch_table.GetMemoryWin32HandleNV(device, memory, handleType, pHandle);
    if (VK_SUCCESS == result) {
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            intercept->PostCallRecordGetMemoryWin32HandleNV(device, memory, handleType, pHandle);
        }
    }
    return result;
}
#endif // VK_USE_PLATFORM_WIN32_KHR

#ifdef VK_USE_PLATFORM_WIN32_KHR
#endif // VK_USE_PLATFORM_WIN32_KHR


#ifdef VK_USE_PLATFORM_VI_NN

VKAPI_ATTR VkResult VKAPI_CALL CreateViSurfaceNN(
    VkInstance                                  instance,
    const VkViSurfaceCreateInfoNN*              pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkSurfaceKHR*                               pSurface) {
    instance_layer_data *instance_data = GetLayerDataPtr(get_dispatch_key(instance), instance_layer_data_map);
    {
        bool skip = false;
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            skip |= intercept->PreCallValidateCreateViSurfaceNN(instance, pCreateInfo, pAllocator, pSurface);
            if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
        }
        for (auto intercept : global_interceptor_list) {
            intercept->PreCallRecordCreateViSurfaceNN(instance, pCreateInfo, pAllocator, pSurface);
        }
    }
    VkResult result = instance_data->dispatch_table.CreateViSurfaceNN(instance, pCreateInfo, pAllocator, pSurface);
    if (VK_SUCCESS == result) {
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            intercept->PostCallRecordCreateViSurfaceNN(instance, pCreateInfo, pAllocator, pSurface);
        }
    }
    return result;
}
#endif // VK_USE_PLATFORM_VI_NN





VKAPI_ATTR void VKAPI_CALL CmdBeginConditionalRenderingEXT(
    VkCommandBuffer                             commandBuffer,
    const VkConditionalRenderingBeginInfoEXT*   pConditionalRenderingBegin) {
    layer_data *device_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    {
        bool skip = false;
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            skip |= intercept->PreCallValidateCmdBeginConditionalRenderingEXT(commandBuffer, pConditionalRenderingBegin);
            if (skip) return;
        }
        for (auto intercept : global_interceptor_list) {
            intercept->PreCallRecordCmdBeginConditionalRenderingEXT(commandBuffer, pConditionalRenderingBegin);
        }
    }
    device_data->dispatch_table.CmdBeginConditionalRenderingEXT(commandBuffer, pConditionalRenderingBegin);
    {
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            intercept->PostCallRecordCmdBeginConditionalRenderingEXT(commandBuffer, pConditionalRenderingBegin);
        }
    }
}

VKAPI_ATTR void VKAPI_CALL CmdEndConditionalRenderingEXT(
    VkCommandBuffer                             commandBuffer) {
    layer_data *device_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    {
        bool skip = false;
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            skip |= intercept->PreCallValidateCmdEndConditionalRenderingEXT(commandBuffer);
            if (skip) return;
        }
        for (auto intercept : global_interceptor_list) {
            intercept->PreCallRecordCmdEndConditionalRenderingEXT(commandBuffer);
        }
    }
    device_data->dispatch_table.CmdEndConditionalRenderingEXT(commandBuffer);
    {
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            intercept->PostCallRecordCmdEndConditionalRenderingEXT(commandBuffer);
        }
    }
}


VKAPI_ATTR void VKAPI_CALL CmdProcessCommandsNVX(
    VkCommandBuffer                             commandBuffer,
    const VkCmdProcessCommandsInfoNVX*          pProcessCommandsInfo) {
    layer_data *device_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    {
        bool skip = false;
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            skip |= intercept->PreCallValidateCmdProcessCommandsNVX(commandBuffer, pProcessCommandsInfo);
            if (skip) return;
        }
        for (auto intercept : global_interceptor_list) {
            intercept->PreCallRecordCmdProcessCommandsNVX(commandBuffer, pProcessCommandsInfo);
        }
    }
    device_data->dispatch_table.CmdProcessCommandsNVX(commandBuffer, pProcessCommandsInfo);
    {
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            intercept->PostCallRecordCmdProcessCommandsNVX(commandBuffer, pProcessCommandsInfo);
        }
    }
}

VKAPI_ATTR void VKAPI_CALL CmdReserveSpaceForCommandsNVX(
    VkCommandBuffer                             commandBuffer,
    const VkCmdReserveSpaceForCommandsInfoNVX*  pReserveSpaceInfo) {
    layer_data *device_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    {
        bool skip = false;
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            skip |= intercept->PreCallValidateCmdReserveSpaceForCommandsNVX(commandBuffer, pReserveSpaceInfo);
            if (skip) return;
        }
        for (auto intercept : global_interceptor_list) {
            intercept->PreCallRecordCmdReserveSpaceForCommandsNVX(commandBuffer, pReserveSpaceInfo);
        }
    }
    device_data->dispatch_table.CmdReserveSpaceForCommandsNVX(commandBuffer, pReserveSpaceInfo);
    {
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            intercept->PostCallRecordCmdReserveSpaceForCommandsNVX(commandBuffer, pReserveSpaceInfo);
        }
    }
}

VKAPI_ATTR VkResult VKAPI_CALL CreateIndirectCommandsLayoutNVX(
    VkDevice                                    device,
    const VkIndirectCommandsLayoutCreateInfoNVX* pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkIndirectCommandsLayoutNVX*                pIndirectCommandsLayout) {
    layer_data *device_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    {
        bool skip = false;
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            skip |= intercept->PreCallValidateCreateIndirectCommandsLayoutNVX(device, pCreateInfo, pAllocator, pIndirectCommandsLayout);
            if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
        }
        for (auto intercept : global_interceptor_list) {
            intercept->PreCallRecordCreateIndirectCommandsLayoutNVX(device, pCreateInfo, pAllocator, pIndirectCommandsLayout);
        }
    }
    VkResult result = device_data->dispatch_table.CreateIndirectCommandsLayoutNVX(device, pCreateInfo, pAllocator, pIndirectCommandsLayout);
    if (VK_SUCCESS == result) {
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            intercept->PostCallRecordCreateIndirectCommandsLayoutNVX(device, pCreateInfo, pAllocator, pIndirectCommandsLayout);
        }
    }
    return result;
}

VKAPI_ATTR void VKAPI_CALL DestroyIndirectCommandsLayoutNVX(
    VkDevice                                    device,
    VkIndirectCommandsLayoutNVX                 indirectCommandsLayout,
    const VkAllocationCallbacks*                pAllocator) {
    layer_data *device_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    {
        bool skip = false;
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            skip |= intercept->PreCallValidateDestroyIndirectCommandsLayoutNVX(device, indirectCommandsLayout, pAllocator);
            if (skip) return;
        }
        for (auto intercept : global_interceptor_list) {
            intercept->PreCallRecordDestroyIndirectCommandsLayoutNVX(device, indirectCommandsLayout, pAllocator);
        }
    }
    device_data->dispatch_table.DestroyIndirectCommandsLayoutNVX(device, indirectCommandsLayout, pAllocator);
    {
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            intercept->PostCallRecordDestroyIndirectCommandsLayoutNVX(device, indirectCommandsLayout, pAllocator);
        }
    }
}

VKAPI_ATTR VkResult VKAPI_CALL CreateObjectTableNVX(
    VkDevice                                    device,
    const VkObjectTableCreateInfoNVX*           pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkObjectTableNVX*                           pObjectTable) {
    layer_data *device_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    {
        bool skip = false;
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            skip |= intercept->PreCallValidateCreateObjectTableNVX(device, pCreateInfo, pAllocator, pObjectTable);
            if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
        }
        for (auto intercept : global_interceptor_list) {
            intercept->PreCallRecordCreateObjectTableNVX(device, pCreateInfo, pAllocator, pObjectTable);
        }
    }
    VkResult result = device_data->dispatch_table.CreateObjectTableNVX(device, pCreateInfo, pAllocator, pObjectTable);
    if (VK_SUCCESS == result) {
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            intercept->PostCallRecordCreateObjectTableNVX(device, pCreateInfo, pAllocator, pObjectTable);
        }
    }
    return result;
}

VKAPI_ATTR void VKAPI_CALL DestroyObjectTableNVX(
    VkDevice                                    device,
    VkObjectTableNVX                            objectTable,
    const VkAllocationCallbacks*                pAllocator) {
    layer_data *device_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    {
        bool skip = false;
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            skip |= intercept->PreCallValidateDestroyObjectTableNVX(device, objectTable, pAllocator);
            if (skip) return;
        }
        for (auto intercept : global_interceptor_list) {
            intercept->PreCallRecordDestroyObjectTableNVX(device, objectTable, pAllocator);
        }
    }
    device_data->dispatch_table.DestroyObjectTableNVX(device, objectTable, pAllocator);
    {
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            intercept->PostCallRecordDestroyObjectTableNVX(device, objectTable, pAllocator);
        }
    }
}

VKAPI_ATTR VkResult VKAPI_CALL RegisterObjectsNVX(
    VkDevice                                    device,
    VkObjectTableNVX                            objectTable,
    uint32_t                                    objectCount,
    const VkObjectTableEntryNVX* const*         ppObjectTableEntries,
    const uint32_t*                             pObjectIndices) {
    layer_data *device_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    {
        bool skip = false;
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            skip |= intercept->PreCallValidateRegisterObjectsNVX(device, objectTable, objectCount, ppObjectTableEntries, pObjectIndices);
            if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
        }
        for (auto intercept : global_interceptor_list) {
            intercept->PreCallRecordRegisterObjectsNVX(device, objectTable, objectCount, ppObjectTableEntries, pObjectIndices);
        }
    }
    VkResult result = device_data->dispatch_table.RegisterObjectsNVX(device, objectTable, objectCount, ppObjectTableEntries, pObjectIndices);
    if (VK_SUCCESS == result) {
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            intercept->PostCallRecordRegisterObjectsNVX(device, objectTable, objectCount, ppObjectTableEntries, pObjectIndices);
        }
    }
    return result;
}

VKAPI_ATTR VkResult VKAPI_CALL UnregisterObjectsNVX(
    VkDevice                                    device,
    VkObjectTableNVX                            objectTable,
    uint32_t                                    objectCount,
    const VkObjectEntryTypeNVX*                 pObjectEntryTypes,
    const uint32_t*                             pObjectIndices) {
    layer_data *device_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    {
        bool skip = false;
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            skip |= intercept->PreCallValidateUnregisterObjectsNVX(device, objectTable, objectCount, pObjectEntryTypes, pObjectIndices);
            if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
        }
        for (auto intercept : global_interceptor_list) {
            intercept->PreCallRecordUnregisterObjectsNVX(device, objectTable, objectCount, pObjectEntryTypes, pObjectIndices);
        }
    }
    VkResult result = device_data->dispatch_table.UnregisterObjectsNVX(device, objectTable, objectCount, pObjectEntryTypes, pObjectIndices);
    if (VK_SUCCESS == result) {
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            intercept->PostCallRecordUnregisterObjectsNVX(device, objectTable, objectCount, pObjectEntryTypes, pObjectIndices);
        }
    }
    return result;
}

VKAPI_ATTR void VKAPI_CALL GetPhysicalDeviceGeneratedCommandsPropertiesNVX(
    VkPhysicalDevice                            physicalDevice,
    VkDeviceGeneratedCommandsFeaturesNVX*       pFeatures,
    VkDeviceGeneratedCommandsLimitsNVX*         pLimits) {
    instance_layer_data *instance_data = GetLayerDataPtr(get_dispatch_key(physicalDevice), instance_layer_data_map);
    {
        bool skip = false;
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            skip |= intercept->PreCallValidateGetPhysicalDeviceGeneratedCommandsPropertiesNVX(physicalDevice, pFeatures, pLimits);
            if (skip) return;
        }
        for (auto intercept : global_interceptor_list) {
            intercept->PreCallRecordGetPhysicalDeviceGeneratedCommandsPropertiesNVX(physicalDevice, pFeatures, pLimits);
        }
    }
    instance_data->dispatch_table.GetPhysicalDeviceGeneratedCommandsPropertiesNVX(physicalDevice, pFeatures, pLimits);
    {
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            intercept->PostCallRecordGetPhysicalDeviceGeneratedCommandsPropertiesNVX(physicalDevice, pFeatures, pLimits);
        }
    }
}


VKAPI_ATTR void VKAPI_CALL CmdSetViewportWScalingNV(
    VkCommandBuffer                             commandBuffer,
    uint32_t                                    firstViewport,
    uint32_t                                    viewportCount,
    const VkViewportWScalingNV*                 pViewportWScalings) {
    layer_data *device_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    {
        bool skip = false;
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            skip |= intercept->PreCallValidateCmdSetViewportWScalingNV(commandBuffer, firstViewport, viewportCount, pViewportWScalings);
            if (skip) return;
        }
        for (auto intercept : global_interceptor_list) {
            intercept->PreCallRecordCmdSetViewportWScalingNV(commandBuffer, firstViewport, viewportCount, pViewportWScalings);
        }
    }
    device_data->dispatch_table.CmdSetViewportWScalingNV(commandBuffer, firstViewport, viewportCount, pViewportWScalings);
    {
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            intercept->PostCallRecordCmdSetViewportWScalingNV(commandBuffer, firstViewport, viewportCount, pViewportWScalings);
        }
    }
}


VKAPI_ATTR VkResult VKAPI_CALL ReleaseDisplayEXT(
    VkPhysicalDevice                            physicalDevice,
    VkDisplayKHR                                display) {
    instance_layer_data *instance_data = GetLayerDataPtr(get_dispatch_key(physicalDevice), instance_layer_data_map);
    {
        bool skip = false;
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            skip |= intercept->PreCallValidateReleaseDisplayEXT(physicalDevice, display);
            if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
        }
        for (auto intercept : global_interceptor_list) {
            intercept->PreCallRecordReleaseDisplayEXT(physicalDevice, display);
        }
    }
    VkResult result = instance_data->dispatch_table.ReleaseDisplayEXT(physicalDevice, display);
    if (VK_SUCCESS == result) {
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            intercept->PostCallRecordReleaseDisplayEXT(physicalDevice, display);
        }
    }
    return result;
}

#ifdef VK_USE_PLATFORM_XLIB_XRANDR_EXT

VKAPI_ATTR VkResult VKAPI_CALL AcquireXlibDisplayEXT(
    VkPhysicalDevice                            physicalDevice,
    Display*                                    dpy,
    VkDisplayKHR                                display) {
    instance_layer_data *instance_data = GetLayerDataPtr(get_dispatch_key(physicalDevice), instance_layer_data_map);
    {
        bool skip = false;
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            skip |= intercept->PreCallValidateAcquireXlibDisplayEXT(physicalDevice, dpy, display);
            if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
        }
        for (auto intercept : global_interceptor_list) {
            intercept->PreCallRecordAcquireXlibDisplayEXT(physicalDevice, dpy, display);
        }
    }
    VkResult result = instance_data->dispatch_table.AcquireXlibDisplayEXT(physicalDevice, dpy, display);
    if (VK_SUCCESS == result) {
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            intercept->PostCallRecordAcquireXlibDisplayEXT(physicalDevice, dpy, display);
        }
    }
    return result;
}

VKAPI_ATTR VkResult VKAPI_CALL GetRandROutputDisplayEXT(
    VkPhysicalDevice                            physicalDevice,
    Display*                                    dpy,
    RROutput                                    rrOutput,
    VkDisplayKHR*                               pDisplay) {
    instance_layer_data *instance_data = GetLayerDataPtr(get_dispatch_key(physicalDevice), instance_layer_data_map);
    {
        bool skip = false;
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            skip |= intercept->PreCallValidateGetRandROutputDisplayEXT(physicalDevice, dpy, rrOutput, pDisplay);
            if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
        }
        for (auto intercept : global_interceptor_list) {
            intercept->PreCallRecordGetRandROutputDisplayEXT(physicalDevice, dpy, rrOutput, pDisplay);
        }
    }
    VkResult result = instance_data->dispatch_table.GetRandROutputDisplayEXT(physicalDevice, dpy, rrOutput, pDisplay);
    if (VK_SUCCESS == result) {
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            intercept->PostCallRecordGetRandROutputDisplayEXT(physicalDevice, dpy, rrOutput, pDisplay);
        }
    }
    return result;
}
#endif // VK_USE_PLATFORM_XLIB_XRANDR_EXT


VKAPI_ATTR VkResult VKAPI_CALL GetPhysicalDeviceSurfaceCapabilities2EXT(
    VkPhysicalDevice                            physicalDevice,
    VkSurfaceKHR                                surface,
    VkSurfaceCapabilities2EXT*                  pSurfaceCapabilities) {
    instance_layer_data *instance_data = GetLayerDataPtr(get_dispatch_key(physicalDevice), instance_layer_data_map);
    {
        bool skip = false;
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            skip |= intercept->PreCallValidateGetPhysicalDeviceSurfaceCapabilities2EXT(physicalDevice, surface, pSurfaceCapabilities);
            if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
        }
        for (auto intercept : global_interceptor_list) {
            intercept->PreCallRecordGetPhysicalDeviceSurfaceCapabilities2EXT(physicalDevice, surface, pSurfaceCapabilities);
        }
    }
    VkResult result = instance_data->dispatch_table.GetPhysicalDeviceSurfaceCapabilities2EXT(physicalDevice, surface, pSurfaceCapabilities);
    if (VK_SUCCESS == result) {
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            intercept->PostCallRecordGetPhysicalDeviceSurfaceCapabilities2EXT(physicalDevice, surface, pSurfaceCapabilities);
        }
    }
    return result;
}


VKAPI_ATTR VkResult VKAPI_CALL DisplayPowerControlEXT(
    VkDevice                                    device,
    VkDisplayKHR                                display,
    const VkDisplayPowerInfoEXT*                pDisplayPowerInfo) {
    layer_data *device_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    {
        bool skip = false;
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            skip |= intercept->PreCallValidateDisplayPowerControlEXT(device, display, pDisplayPowerInfo);
            if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
        }
        for (auto intercept : global_interceptor_list) {
            intercept->PreCallRecordDisplayPowerControlEXT(device, display, pDisplayPowerInfo);
        }
    }
    VkResult result = device_data->dispatch_table.DisplayPowerControlEXT(device, display, pDisplayPowerInfo);
    if (VK_SUCCESS == result) {
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            intercept->PostCallRecordDisplayPowerControlEXT(device, display, pDisplayPowerInfo);
        }
    }
    return result;
}

VKAPI_ATTR VkResult VKAPI_CALL RegisterDeviceEventEXT(
    VkDevice                                    device,
    const VkDeviceEventInfoEXT*                 pDeviceEventInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkFence*                                    pFence) {
    layer_data *device_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    {
        bool skip = false;
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            skip |= intercept->PreCallValidateRegisterDeviceEventEXT(device, pDeviceEventInfo, pAllocator, pFence);
            if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
        }
        for (auto intercept : global_interceptor_list) {
            intercept->PreCallRecordRegisterDeviceEventEXT(device, pDeviceEventInfo, pAllocator, pFence);
        }
    }
    VkResult result = device_data->dispatch_table.RegisterDeviceEventEXT(device, pDeviceEventInfo, pAllocator, pFence);
    if (VK_SUCCESS == result) {
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            intercept->PostCallRecordRegisterDeviceEventEXT(device, pDeviceEventInfo, pAllocator, pFence);
        }
    }
    return result;
}

VKAPI_ATTR VkResult VKAPI_CALL RegisterDisplayEventEXT(
    VkDevice                                    device,
    VkDisplayKHR                                display,
    const VkDisplayEventInfoEXT*                pDisplayEventInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkFence*                                    pFence) {
    layer_data *device_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    {
        bool skip = false;
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            skip |= intercept->PreCallValidateRegisterDisplayEventEXT(device, display, pDisplayEventInfo, pAllocator, pFence);
            if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
        }
        for (auto intercept : global_interceptor_list) {
            intercept->PreCallRecordRegisterDisplayEventEXT(device, display, pDisplayEventInfo, pAllocator, pFence);
        }
    }
    VkResult result = device_data->dispatch_table.RegisterDisplayEventEXT(device, display, pDisplayEventInfo, pAllocator, pFence);
    if (VK_SUCCESS == result) {
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            intercept->PostCallRecordRegisterDisplayEventEXT(device, display, pDisplayEventInfo, pAllocator, pFence);
        }
    }
    return result;
}

VKAPI_ATTR VkResult VKAPI_CALL GetSwapchainCounterEXT(
    VkDevice                                    device,
    VkSwapchainKHR                              swapchain,
    VkSurfaceCounterFlagBitsEXT                 counter,
    uint64_t*                                   pCounterValue) {
    layer_data *device_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    {
        bool skip = false;
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            skip |= intercept->PreCallValidateGetSwapchainCounterEXT(device, swapchain, counter, pCounterValue);
            if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
        }
        for (auto intercept : global_interceptor_list) {
            intercept->PreCallRecordGetSwapchainCounterEXT(device, swapchain, counter, pCounterValue);
        }
    }
    VkResult result = device_data->dispatch_table.GetSwapchainCounterEXT(device, swapchain, counter, pCounterValue);
    if (VK_SUCCESS == result) {
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            intercept->PostCallRecordGetSwapchainCounterEXT(device, swapchain, counter, pCounterValue);
        }
    }
    return result;
}


VKAPI_ATTR VkResult VKAPI_CALL GetRefreshCycleDurationGOOGLE(
    VkDevice                                    device,
    VkSwapchainKHR                              swapchain,
    VkRefreshCycleDurationGOOGLE*               pDisplayTimingProperties) {
    layer_data *device_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    {
        bool skip = false;
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            skip |= intercept->PreCallValidateGetRefreshCycleDurationGOOGLE(device, swapchain, pDisplayTimingProperties);
            if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
        }
        for (auto intercept : global_interceptor_list) {
            intercept->PreCallRecordGetRefreshCycleDurationGOOGLE(device, swapchain, pDisplayTimingProperties);
        }
    }
    VkResult result = device_data->dispatch_table.GetRefreshCycleDurationGOOGLE(device, swapchain, pDisplayTimingProperties);
    if (VK_SUCCESS == result) {
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            intercept->PostCallRecordGetRefreshCycleDurationGOOGLE(device, swapchain, pDisplayTimingProperties);
        }
    }
    return result;
}

VKAPI_ATTR VkResult VKAPI_CALL GetPastPresentationTimingGOOGLE(
    VkDevice                                    device,
    VkSwapchainKHR                              swapchain,
    uint32_t*                                   pPresentationTimingCount,
    VkPastPresentationTimingGOOGLE*             pPresentationTimings) {
    layer_data *device_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    {
        bool skip = false;
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            skip |= intercept->PreCallValidateGetPastPresentationTimingGOOGLE(device, swapchain, pPresentationTimingCount, pPresentationTimings);
            if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
        }
        for (auto intercept : global_interceptor_list) {
            intercept->PreCallRecordGetPastPresentationTimingGOOGLE(device, swapchain, pPresentationTimingCount, pPresentationTimings);
        }
    }
    VkResult result = device_data->dispatch_table.GetPastPresentationTimingGOOGLE(device, swapchain, pPresentationTimingCount, pPresentationTimings);
    if (VK_SUCCESS == result) {
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            intercept->PostCallRecordGetPastPresentationTimingGOOGLE(device, swapchain, pPresentationTimingCount, pPresentationTimings);
        }
    }
    return result;
}







VKAPI_ATTR void VKAPI_CALL CmdSetDiscardRectangleEXT(
    VkCommandBuffer                             commandBuffer,
    uint32_t                                    firstDiscardRectangle,
    uint32_t                                    discardRectangleCount,
    const VkRect2D*                             pDiscardRectangles) {
    layer_data *device_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    {
        bool skip = false;
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            skip |= intercept->PreCallValidateCmdSetDiscardRectangleEXT(commandBuffer, firstDiscardRectangle, discardRectangleCount, pDiscardRectangles);
            if (skip) return;
        }
        for (auto intercept : global_interceptor_list) {
            intercept->PreCallRecordCmdSetDiscardRectangleEXT(commandBuffer, firstDiscardRectangle, discardRectangleCount, pDiscardRectangles);
        }
    }
    device_data->dispatch_table.CmdSetDiscardRectangleEXT(commandBuffer, firstDiscardRectangle, discardRectangleCount, pDiscardRectangles);
    {
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            intercept->PostCallRecordCmdSetDiscardRectangleEXT(commandBuffer, firstDiscardRectangle, discardRectangleCount, pDiscardRectangles);
        }
    }
}




VKAPI_ATTR void VKAPI_CALL SetHdrMetadataEXT(
    VkDevice                                    device,
    uint32_t                                    swapchainCount,
    const VkSwapchainKHR*                       pSwapchains,
    const VkHdrMetadataEXT*                     pMetadata) {
    layer_data *device_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    {
        bool skip = false;
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            skip |= intercept->PreCallValidateSetHdrMetadataEXT(device, swapchainCount, pSwapchains, pMetadata);
            if (skip) return;
        }
        for (auto intercept : global_interceptor_list) {
            intercept->PreCallRecordSetHdrMetadataEXT(device, swapchainCount, pSwapchains, pMetadata);
        }
    }
    device_data->dispatch_table.SetHdrMetadataEXT(device, swapchainCount, pSwapchains, pMetadata);
    {
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            intercept->PostCallRecordSetHdrMetadataEXT(device, swapchainCount, pSwapchains, pMetadata);
        }
    }
}

#ifdef VK_USE_PLATFORM_IOS_MVK

VKAPI_ATTR VkResult VKAPI_CALL CreateIOSSurfaceMVK(
    VkInstance                                  instance,
    const VkIOSSurfaceCreateInfoMVK*            pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkSurfaceKHR*                               pSurface) {
    instance_layer_data *instance_data = GetLayerDataPtr(get_dispatch_key(instance), instance_layer_data_map);
    {
        bool skip = false;
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            skip |= intercept->PreCallValidateCreateIOSSurfaceMVK(instance, pCreateInfo, pAllocator, pSurface);
            if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
        }
        for (auto intercept : global_interceptor_list) {
            intercept->PreCallRecordCreateIOSSurfaceMVK(instance, pCreateInfo, pAllocator, pSurface);
        }
    }
    VkResult result = instance_data->dispatch_table.CreateIOSSurfaceMVK(instance, pCreateInfo, pAllocator, pSurface);
    if (VK_SUCCESS == result) {
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            intercept->PostCallRecordCreateIOSSurfaceMVK(instance, pCreateInfo, pAllocator, pSurface);
        }
    }
    return result;
}
#endif // VK_USE_PLATFORM_IOS_MVK

#ifdef VK_USE_PLATFORM_MACOS_MVK

VKAPI_ATTR VkResult VKAPI_CALL CreateMacOSSurfaceMVK(
    VkInstance                                  instance,
    const VkMacOSSurfaceCreateInfoMVK*          pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkSurfaceKHR*                               pSurface) {
    instance_layer_data *instance_data = GetLayerDataPtr(get_dispatch_key(instance), instance_layer_data_map);
    {
        bool skip = false;
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            skip |= intercept->PreCallValidateCreateMacOSSurfaceMVK(instance, pCreateInfo, pAllocator, pSurface);
            if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
        }
        for (auto intercept : global_interceptor_list) {
            intercept->PreCallRecordCreateMacOSSurfaceMVK(instance, pCreateInfo, pAllocator, pSurface);
        }
    }
    VkResult result = instance_data->dispatch_table.CreateMacOSSurfaceMVK(instance, pCreateInfo, pAllocator, pSurface);
    if (VK_SUCCESS == result) {
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            intercept->PostCallRecordCreateMacOSSurfaceMVK(instance, pCreateInfo, pAllocator, pSurface);
        }
    }
    return result;
}
#endif // VK_USE_PLATFORM_MACOS_MVK




VKAPI_ATTR VkResult VKAPI_CALL SetDebugUtilsObjectNameEXT(
    VkDevice                                    device,
    const VkDebugUtilsObjectNameInfoEXT*        pNameInfo) {
    layer_data *device_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    {
        bool skip = false;
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            skip |= intercept->PreCallValidateSetDebugUtilsObjectNameEXT(device, pNameInfo);
            if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
        }
        for (auto intercept : global_interceptor_list) {
            intercept->PreCallRecordSetDebugUtilsObjectNameEXT(device, pNameInfo);
        }
    }
    VkResult result = device_data->dispatch_table.SetDebugUtilsObjectNameEXT(device, pNameInfo);
    if (VK_SUCCESS == result) {
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            intercept->PostCallRecordSetDebugUtilsObjectNameEXT(device, pNameInfo);
        }
    }
    return result;
}

VKAPI_ATTR VkResult VKAPI_CALL SetDebugUtilsObjectTagEXT(
    VkDevice                                    device,
    const VkDebugUtilsObjectTagInfoEXT*         pTagInfo) {
    layer_data *device_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    {
        bool skip = false;
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            skip |= intercept->PreCallValidateSetDebugUtilsObjectTagEXT(device, pTagInfo);
            if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
        }
        for (auto intercept : global_interceptor_list) {
            intercept->PreCallRecordSetDebugUtilsObjectTagEXT(device, pTagInfo);
        }
    }
    VkResult result = device_data->dispatch_table.SetDebugUtilsObjectTagEXT(device, pTagInfo);
    if (VK_SUCCESS == result) {
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            intercept->PostCallRecordSetDebugUtilsObjectTagEXT(device, pTagInfo);
        }
    }
    return result;
}

VKAPI_ATTR void VKAPI_CALL QueueBeginDebugUtilsLabelEXT(
    VkQueue                                     queue,
    const VkDebugUtilsLabelEXT*                 pLabelInfo) {
    layer_data *device_data = GetLayerDataPtr(get_dispatch_key(queue), layer_data_map);
    {
        bool skip = false;
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            skip |= intercept->PreCallValidateQueueBeginDebugUtilsLabelEXT(queue, pLabelInfo);
            if (skip) return;
        }
        for (auto intercept : global_interceptor_list) {
            intercept->PreCallRecordQueueBeginDebugUtilsLabelEXT(queue, pLabelInfo);
        }
    }
    device_data->dispatch_table.QueueBeginDebugUtilsLabelEXT(queue, pLabelInfo);
    {
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            intercept->PostCallRecordQueueBeginDebugUtilsLabelEXT(queue, pLabelInfo);
        }
    }
}

VKAPI_ATTR void VKAPI_CALL QueueEndDebugUtilsLabelEXT(
    VkQueue                                     queue) {
    layer_data *device_data = GetLayerDataPtr(get_dispatch_key(queue), layer_data_map);
    {
        bool skip = false;
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            skip |= intercept->PreCallValidateQueueEndDebugUtilsLabelEXT(queue);
            if (skip) return;
        }
        for (auto intercept : global_interceptor_list) {
            intercept->PreCallRecordQueueEndDebugUtilsLabelEXT(queue);
        }
    }
    device_data->dispatch_table.QueueEndDebugUtilsLabelEXT(queue);
    {
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            intercept->PostCallRecordQueueEndDebugUtilsLabelEXT(queue);
        }
    }
}

VKAPI_ATTR void VKAPI_CALL QueueInsertDebugUtilsLabelEXT(
    VkQueue                                     queue,
    const VkDebugUtilsLabelEXT*                 pLabelInfo) {
    layer_data *device_data = GetLayerDataPtr(get_dispatch_key(queue), layer_data_map);
    {
        bool skip = false;
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            skip |= intercept->PreCallValidateQueueInsertDebugUtilsLabelEXT(queue, pLabelInfo);
            if (skip) return;
        }
        for (auto intercept : global_interceptor_list) {
            intercept->PreCallRecordQueueInsertDebugUtilsLabelEXT(queue, pLabelInfo);
        }
    }
    device_data->dispatch_table.QueueInsertDebugUtilsLabelEXT(queue, pLabelInfo);
    {
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            intercept->PostCallRecordQueueInsertDebugUtilsLabelEXT(queue, pLabelInfo);
        }
    }
}

VKAPI_ATTR void VKAPI_CALL CmdBeginDebugUtilsLabelEXT(
    VkCommandBuffer                             commandBuffer,
    const VkDebugUtilsLabelEXT*                 pLabelInfo) {
    layer_data *device_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    {
        bool skip = false;
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            skip |= intercept->PreCallValidateCmdBeginDebugUtilsLabelEXT(commandBuffer, pLabelInfo);
            if (skip) return;
        }
        for (auto intercept : global_interceptor_list) {
            intercept->PreCallRecordCmdBeginDebugUtilsLabelEXT(commandBuffer, pLabelInfo);
        }
    }
    device_data->dispatch_table.CmdBeginDebugUtilsLabelEXT(commandBuffer, pLabelInfo);
    {
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            intercept->PostCallRecordCmdBeginDebugUtilsLabelEXT(commandBuffer, pLabelInfo);
        }
    }
}

VKAPI_ATTR void VKAPI_CALL CmdEndDebugUtilsLabelEXT(
    VkCommandBuffer                             commandBuffer) {
    layer_data *device_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    {
        bool skip = false;
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            skip |= intercept->PreCallValidateCmdEndDebugUtilsLabelEXT(commandBuffer);
            if (skip) return;
        }
        for (auto intercept : global_interceptor_list) {
            intercept->PreCallRecordCmdEndDebugUtilsLabelEXT(commandBuffer);
        }
    }
    device_data->dispatch_table.CmdEndDebugUtilsLabelEXT(commandBuffer);
    {
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            intercept->PostCallRecordCmdEndDebugUtilsLabelEXT(commandBuffer);
        }
    }
}

VKAPI_ATTR void VKAPI_CALL CmdInsertDebugUtilsLabelEXT(
    VkCommandBuffer                             commandBuffer,
    const VkDebugUtilsLabelEXT*                 pLabelInfo) {
    layer_data *device_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    {
        bool skip = false;
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            skip |= intercept->PreCallValidateCmdInsertDebugUtilsLabelEXT(commandBuffer, pLabelInfo);
            if (skip) return;
        }
        for (auto intercept : global_interceptor_list) {
            intercept->PreCallRecordCmdInsertDebugUtilsLabelEXT(commandBuffer, pLabelInfo);
        }
    }
    device_data->dispatch_table.CmdInsertDebugUtilsLabelEXT(commandBuffer, pLabelInfo);
    {
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            intercept->PostCallRecordCmdInsertDebugUtilsLabelEXT(commandBuffer, pLabelInfo);
        }
    }
}

VKAPI_ATTR VkResult VKAPI_CALL CreateDebugUtilsMessengerEXT(
    VkInstance                                  instance,
    const VkDebugUtilsMessengerCreateInfoEXT*   pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkDebugUtilsMessengerEXT*                   pMessenger) {
    instance_layer_data *instance_data = GetLayerDataPtr(get_dispatch_key(instance), instance_layer_data_map);
    {
        bool skip = false;
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            skip |= intercept->PreCallValidateCreateDebugUtilsMessengerEXT(instance, pCreateInfo, pAllocator, pMessenger);
            if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
        }
        for (auto intercept : global_interceptor_list) {
            intercept->PreCallRecordCreateDebugUtilsMessengerEXT(instance, pCreateInfo, pAllocator, pMessenger);
        }
    }
    VkResult result = instance_data->dispatch_table.CreateDebugUtilsMessengerEXT(instance, pCreateInfo, pAllocator, pMessenger);
    if (VK_SUCCESS == result) {
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            intercept->PostCallRecordCreateDebugUtilsMessengerEXT(instance, pCreateInfo, pAllocator, pMessenger);
        }
    }
    return result;
}

VKAPI_ATTR void VKAPI_CALL DestroyDebugUtilsMessengerEXT(
    VkInstance                                  instance,
    VkDebugUtilsMessengerEXT                    messenger,
    const VkAllocationCallbacks*                pAllocator) {
    instance_layer_data *instance_data = GetLayerDataPtr(get_dispatch_key(instance), instance_layer_data_map);
    {
        bool skip = false;
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            skip |= intercept->PreCallValidateDestroyDebugUtilsMessengerEXT(instance, messenger, pAllocator);
            if (skip) return;
        }
        for (auto intercept : global_interceptor_list) {
            intercept->PreCallRecordDestroyDebugUtilsMessengerEXT(instance, messenger, pAllocator);
        }
    }
    instance_data->dispatch_table.DestroyDebugUtilsMessengerEXT(instance, messenger, pAllocator);
    {
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            intercept->PostCallRecordDestroyDebugUtilsMessengerEXT(instance, messenger, pAllocator);
        }
    }
}

VKAPI_ATTR void VKAPI_CALL SubmitDebugUtilsMessageEXT(
    VkInstance                                  instance,
    VkDebugUtilsMessageSeverityFlagBitsEXT      messageSeverity,
    VkDebugUtilsMessageTypeFlagsEXT             messageTypes,
    const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData) {
    instance_layer_data *instance_data = GetLayerDataPtr(get_dispatch_key(instance), instance_layer_data_map);
    {
        bool skip = false;
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            skip |= intercept->PreCallValidateSubmitDebugUtilsMessageEXT(instance, messageSeverity, messageTypes, pCallbackData);
            if (skip) return;
        }
        for (auto intercept : global_interceptor_list) {
            intercept->PreCallRecordSubmitDebugUtilsMessageEXT(instance, messageSeverity, messageTypes, pCallbackData);
        }
    }
    instance_data->dispatch_table.SubmitDebugUtilsMessageEXT(instance, messageSeverity, messageTypes, pCallbackData);
    {
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            intercept->PostCallRecordSubmitDebugUtilsMessageEXT(instance, messageSeverity, messageTypes, pCallbackData);
        }
    }
}

#ifdef VK_USE_PLATFORM_ANDROID_KHR

VKAPI_ATTR VkResult VKAPI_CALL GetAndroidHardwareBufferPropertiesANDROID(
    VkDevice                                    device,
    const struct AHardwareBuffer*               buffer,
    VkAndroidHardwareBufferPropertiesANDROID*   pProperties) {
    layer_data *device_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    {
        bool skip = false;
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            skip |= intercept->PreCallValidateGetAndroidHardwareBufferPropertiesANDROID(device, buffer, pProperties);
            if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
        }
        for (auto intercept : global_interceptor_list) {
            intercept->PreCallRecordGetAndroidHardwareBufferPropertiesANDROID(device, buffer, pProperties);
        }
    }
    VkResult result = device_data->dispatch_table.GetAndroidHardwareBufferPropertiesANDROID(device, buffer, pProperties);
    if (VK_SUCCESS == result) {
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            intercept->PostCallRecordGetAndroidHardwareBufferPropertiesANDROID(device, buffer, pProperties);
        }
    }
    return result;
}

VKAPI_ATTR VkResult VKAPI_CALL GetMemoryAndroidHardwareBufferANDROID(
    VkDevice                                    device,
    const VkMemoryGetAndroidHardwareBufferInfoANDROID* pInfo,
    struct AHardwareBuffer**                    pBuffer) {
    layer_data *device_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    {
        bool skip = false;
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            skip |= intercept->PreCallValidateGetMemoryAndroidHardwareBufferANDROID(device, pInfo, pBuffer);
            if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
        }
        for (auto intercept : global_interceptor_list) {
            intercept->PreCallRecordGetMemoryAndroidHardwareBufferANDROID(device, pInfo, pBuffer);
        }
    }
    VkResult result = device_data->dispatch_table.GetMemoryAndroidHardwareBufferANDROID(device, pInfo, pBuffer);
    if (VK_SUCCESS == result) {
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            intercept->PostCallRecordGetMemoryAndroidHardwareBufferANDROID(device, pInfo, pBuffer);
        }
    }
    return result;
}
#endif // VK_USE_PLATFORM_ANDROID_KHR








VKAPI_ATTR void VKAPI_CALL CmdSetSampleLocationsEXT(
    VkCommandBuffer                             commandBuffer,
    const VkSampleLocationsInfoEXT*             pSampleLocationsInfo) {
    layer_data *device_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    {
        bool skip = false;
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            skip |= intercept->PreCallValidateCmdSetSampleLocationsEXT(commandBuffer, pSampleLocationsInfo);
            if (skip) return;
        }
        for (auto intercept : global_interceptor_list) {
            intercept->PreCallRecordCmdSetSampleLocationsEXT(commandBuffer, pSampleLocationsInfo);
        }
    }
    device_data->dispatch_table.CmdSetSampleLocationsEXT(commandBuffer, pSampleLocationsInfo);
    {
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            intercept->PostCallRecordCmdSetSampleLocationsEXT(commandBuffer, pSampleLocationsInfo);
        }
    }
}

VKAPI_ATTR void VKAPI_CALL GetPhysicalDeviceMultisamplePropertiesEXT(
    VkPhysicalDevice                            physicalDevice,
    VkSampleCountFlagBits                       samples,
    VkMultisamplePropertiesEXT*                 pMultisampleProperties) {
    instance_layer_data *instance_data = GetLayerDataPtr(get_dispatch_key(physicalDevice), instance_layer_data_map);
    {
        bool skip = false;
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            skip |= intercept->PreCallValidateGetPhysicalDeviceMultisamplePropertiesEXT(physicalDevice, samples, pMultisampleProperties);
            if (skip) return;
        }
        for (auto intercept : global_interceptor_list) {
            intercept->PreCallRecordGetPhysicalDeviceMultisamplePropertiesEXT(physicalDevice, samples, pMultisampleProperties);
        }
    }
    instance_data->dispatch_table.GetPhysicalDeviceMultisamplePropertiesEXT(physicalDevice, samples, pMultisampleProperties);
    {
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            intercept->PostCallRecordGetPhysicalDeviceMultisamplePropertiesEXT(physicalDevice, samples, pMultisampleProperties);
        }
    }
}







VKAPI_ATTR VkResult VKAPI_CALL CreateValidationCacheEXT(
    VkDevice                                    device,
    const VkValidationCacheCreateInfoEXT*       pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkValidationCacheEXT*                       pValidationCache) {
    layer_data *device_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    {
        bool skip = false;
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            skip |= intercept->PreCallValidateCreateValidationCacheEXT(device, pCreateInfo, pAllocator, pValidationCache);
            if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
        }
        for (auto intercept : global_interceptor_list) {
            intercept->PreCallRecordCreateValidationCacheEXT(device, pCreateInfo, pAllocator, pValidationCache);
        }
    }
    VkResult result = device_data->dispatch_table.CreateValidationCacheEXT(device, pCreateInfo, pAllocator, pValidationCache);
    if (VK_SUCCESS == result) {
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            intercept->PostCallRecordCreateValidationCacheEXT(device, pCreateInfo, pAllocator, pValidationCache);
        }
    }
    return result;
}

VKAPI_ATTR void VKAPI_CALL DestroyValidationCacheEXT(
    VkDevice                                    device,
    VkValidationCacheEXT                        validationCache,
    const VkAllocationCallbacks*                pAllocator) {
    layer_data *device_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    {
        bool skip = false;
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            skip |= intercept->PreCallValidateDestroyValidationCacheEXT(device, validationCache, pAllocator);
            if (skip) return;
        }
        for (auto intercept : global_interceptor_list) {
            intercept->PreCallRecordDestroyValidationCacheEXT(device, validationCache, pAllocator);
        }
    }
    device_data->dispatch_table.DestroyValidationCacheEXT(device, validationCache, pAllocator);
    {
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            intercept->PostCallRecordDestroyValidationCacheEXT(device, validationCache, pAllocator);
        }
    }
}

VKAPI_ATTR VkResult VKAPI_CALL MergeValidationCachesEXT(
    VkDevice                                    device,
    VkValidationCacheEXT                        dstCache,
    uint32_t                                    srcCacheCount,
    const VkValidationCacheEXT*                 pSrcCaches) {
    layer_data *device_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    {
        bool skip = false;
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            skip |= intercept->PreCallValidateMergeValidationCachesEXT(device, dstCache, srcCacheCount, pSrcCaches);
            if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
        }
        for (auto intercept : global_interceptor_list) {
            intercept->PreCallRecordMergeValidationCachesEXT(device, dstCache, srcCacheCount, pSrcCaches);
        }
    }
    VkResult result = device_data->dispatch_table.MergeValidationCachesEXT(device, dstCache, srcCacheCount, pSrcCaches);
    if (VK_SUCCESS == result) {
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            intercept->PostCallRecordMergeValidationCachesEXT(device, dstCache, srcCacheCount, pSrcCaches);
        }
    }
    return result;
}

VKAPI_ATTR VkResult VKAPI_CALL GetValidationCacheDataEXT(
    VkDevice                                    device,
    VkValidationCacheEXT                        validationCache,
    size_t*                                     pDataSize,
    void*                                       pData) {
    layer_data *device_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    {
        bool skip = false;
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            skip |= intercept->PreCallValidateGetValidationCacheDataEXT(device, validationCache, pDataSize, pData);
            if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
        }
        for (auto intercept : global_interceptor_list) {
            intercept->PreCallRecordGetValidationCacheDataEXT(device, validationCache, pDataSize, pData);
        }
    }
    VkResult result = device_data->dispatch_table.GetValidationCacheDataEXT(device, validationCache, pDataSize, pData);
    if (VK_SUCCESS == result) {
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            intercept->PostCallRecordGetValidationCacheDataEXT(device, validationCache, pDataSize, pData);
        }
    }
    return result;
}




VKAPI_ATTR void VKAPI_CALL CmdBindShadingRateImageNV(
    VkCommandBuffer                             commandBuffer,
    VkImageView                                 imageView,
    VkImageLayout                               imageLayout) {
    layer_data *device_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    {
        bool skip = false;
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            skip |= intercept->PreCallValidateCmdBindShadingRateImageNV(commandBuffer, imageView, imageLayout);
            if (skip) return;
        }
        for (auto intercept : global_interceptor_list) {
            intercept->PreCallRecordCmdBindShadingRateImageNV(commandBuffer, imageView, imageLayout);
        }
    }
    device_data->dispatch_table.CmdBindShadingRateImageNV(commandBuffer, imageView, imageLayout);
    {
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            intercept->PostCallRecordCmdBindShadingRateImageNV(commandBuffer, imageView, imageLayout);
        }
    }
}

VKAPI_ATTR void VKAPI_CALL CmdSetViewportShadingRatePaletteNV(
    VkCommandBuffer                             commandBuffer,
    uint32_t                                    firstViewport,
    uint32_t                                    viewportCount,
    const VkShadingRatePaletteNV*               pShadingRatePalettes) {
    layer_data *device_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    {
        bool skip = false;
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            skip |= intercept->PreCallValidateCmdSetViewportShadingRatePaletteNV(commandBuffer, firstViewport, viewportCount, pShadingRatePalettes);
            if (skip) return;
        }
        for (auto intercept : global_interceptor_list) {
            intercept->PreCallRecordCmdSetViewportShadingRatePaletteNV(commandBuffer, firstViewport, viewportCount, pShadingRatePalettes);
        }
    }
    device_data->dispatch_table.CmdSetViewportShadingRatePaletteNV(commandBuffer, firstViewport, viewportCount, pShadingRatePalettes);
    {
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            intercept->PostCallRecordCmdSetViewportShadingRatePaletteNV(commandBuffer, firstViewport, viewportCount, pShadingRatePalettes);
        }
    }
}

VKAPI_ATTR void VKAPI_CALL CmdSetCoarseSampleOrderNV(
    VkCommandBuffer                             commandBuffer,
    VkCoarseSampleOrderTypeNV                   sampleOrderType,
    uint32_t                                    customSampleOrderCount,
    const VkCoarseSampleOrderCustomNV*          pCustomSampleOrders) {
    layer_data *device_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    {
        bool skip = false;
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            skip |= intercept->PreCallValidateCmdSetCoarseSampleOrderNV(commandBuffer, sampleOrderType, customSampleOrderCount, pCustomSampleOrders);
            if (skip) return;
        }
        for (auto intercept : global_interceptor_list) {
            intercept->PreCallRecordCmdSetCoarseSampleOrderNV(commandBuffer, sampleOrderType, customSampleOrderCount, pCustomSampleOrders);
        }
    }
    device_data->dispatch_table.CmdSetCoarseSampleOrderNV(commandBuffer, sampleOrderType, customSampleOrderCount, pCustomSampleOrders);
    {
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            intercept->PostCallRecordCmdSetCoarseSampleOrderNV(commandBuffer, sampleOrderType, customSampleOrderCount, pCustomSampleOrders);
        }
    }
}


VKAPI_ATTR VkResult VKAPI_CALL CreateAccelerationStructureNVX(
    VkDevice                                    device,
    const VkAccelerationStructureCreateInfoNVX* pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkAccelerationStructureNVX*                 pAccelerationStructure) {
    layer_data *device_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    {
        bool skip = false;
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            skip |= intercept->PreCallValidateCreateAccelerationStructureNVX(device, pCreateInfo, pAllocator, pAccelerationStructure);
            if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
        }
        for (auto intercept : global_interceptor_list) {
            intercept->PreCallRecordCreateAccelerationStructureNVX(device, pCreateInfo, pAllocator, pAccelerationStructure);
        }
    }
    VkResult result = device_data->dispatch_table.CreateAccelerationStructureNVX(device, pCreateInfo, pAllocator, pAccelerationStructure);
    if (VK_SUCCESS == result) {
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            intercept->PostCallRecordCreateAccelerationStructureNVX(device, pCreateInfo, pAllocator, pAccelerationStructure);
        }
    }
    return result;
}

VKAPI_ATTR void VKAPI_CALL DestroyAccelerationStructureNVX(
    VkDevice                                    device,
    VkAccelerationStructureNVX                  accelerationStructure,
    const VkAllocationCallbacks*                pAllocator) {
    layer_data *device_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    {
        bool skip = false;
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            skip |= intercept->PreCallValidateDestroyAccelerationStructureNVX(device, accelerationStructure, pAllocator);
            if (skip) return;
        }
        for (auto intercept : global_interceptor_list) {
            intercept->PreCallRecordDestroyAccelerationStructureNVX(device, accelerationStructure, pAllocator);
        }
    }
    device_data->dispatch_table.DestroyAccelerationStructureNVX(device, accelerationStructure, pAllocator);
    {
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            intercept->PostCallRecordDestroyAccelerationStructureNVX(device, accelerationStructure, pAllocator);
        }
    }
}

VKAPI_ATTR void VKAPI_CALL GetAccelerationStructureMemoryRequirementsNVX(
    VkDevice                                    device,
    const VkAccelerationStructureMemoryRequirementsInfoNVX* pInfo,
    VkMemoryRequirements2KHR*                   pMemoryRequirements) {
    layer_data *device_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    {
        bool skip = false;
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            skip |= intercept->PreCallValidateGetAccelerationStructureMemoryRequirementsNVX(device, pInfo, pMemoryRequirements);
            if (skip) return;
        }
        for (auto intercept : global_interceptor_list) {
            intercept->PreCallRecordGetAccelerationStructureMemoryRequirementsNVX(device, pInfo, pMemoryRequirements);
        }
    }
    device_data->dispatch_table.GetAccelerationStructureMemoryRequirementsNVX(device, pInfo, pMemoryRequirements);
    {
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            intercept->PostCallRecordGetAccelerationStructureMemoryRequirementsNVX(device, pInfo, pMemoryRequirements);
        }
    }
}

VKAPI_ATTR void VKAPI_CALL GetAccelerationStructureScratchMemoryRequirementsNVX(
    VkDevice                                    device,
    const VkAccelerationStructureMemoryRequirementsInfoNVX* pInfo,
    VkMemoryRequirements2KHR*                   pMemoryRequirements) {
    layer_data *device_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    {
        bool skip = false;
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            skip |= intercept->PreCallValidateGetAccelerationStructureScratchMemoryRequirementsNVX(device, pInfo, pMemoryRequirements);
            if (skip) return;
        }
        for (auto intercept : global_interceptor_list) {
            intercept->PreCallRecordGetAccelerationStructureScratchMemoryRequirementsNVX(device, pInfo, pMemoryRequirements);
        }
    }
    device_data->dispatch_table.GetAccelerationStructureScratchMemoryRequirementsNVX(device, pInfo, pMemoryRequirements);
    {
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            intercept->PostCallRecordGetAccelerationStructureScratchMemoryRequirementsNVX(device, pInfo, pMemoryRequirements);
        }
    }
}

VKAPI_ATTR VkResult VKAPI_CALL BindAccelerationStructureMemoryNVX(
    VkDevice                                    device,
    uint32_t                                    bindInfoCount,
    const VkBindAccelerationStructureMemoryInfoNVX* pBindInfos) {
    layer_data *device_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    {
        bool skip = false;
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            skip |= intercept->PreCallValidateBindAccelerationStructureMemoryNVX(device, bindInfoCount, pBindInfos);
            if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
        }
        for (auto intercept : global_interceptor_list) {
            intercept->PreCallRecordBindAccelerationStructureMemoryNVX(device, bindInfoCount, pBindInfos);
        }
    }
    VkResult result = device_data->dispatch_table.BindAccelerationStructureMemoryNVX(device, bindInfoCount, pBindInfos);
    if (VK_SUCCESS == result) {
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            intercept->PostCallRecordBindAccelerationStructureMemoryNVX(device, bindInfoCount, pBindInfos);
        }
    }
    return result;
}

VKAPI_ATTR void VKAPI_CALL CmdBuildAccelerationStructureNVX(
    VkCommandBuffer                             commandBuffer,
    VkAccelerationStructureTypeNVX              type,
    uint32_t                                    instanceCount,
    VkBuffer                                    instanceData,
    VkDeviceSize                                instanceOffset,
    uint32_t                                    geometryCount,
    const VkGeometryNVX*                        pGeometries,
    VkBuildAccelerationStructureFlagsNVX        flags,
    VkBool32                                    update,
    VkAccelerationStructureNVX                  dst,
    VkAccelerationStructureNVX                  src,
    VkBuffer                                    scratch,
    VkDeviceSize                                scratchOffset) {
    layer_data *device_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    {
        bool skip = false;
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            skip |= intercept->PreCallValidateCmdBuildAccelerationStructureNVX(commandBuffer, type, instanceCount, instanceData, instanceOffset, geometryCount, pGeometries, flags, update, dst, src, scratch, scratchOffset);
            if (skip) return;
        }
        for (auto intercept : global_interceptor_list) {
            intercept->PreCallRecordCmdBuildAccelerationStructureNVX(commandBuffer, type, instanceCount, instanceData, instanceOffset, geometryCount, pGeometries, flags, update, dst, src, scratch, scratchOffset);
        }
    }
    device_data->dispatch_table.CmdBuildAccelerationStructureNVX(commandBuffer, type, instanceCount, instanceData, instanceOffset, geometryCount, pGeometries, flags, update, dst, src, scratch, scratchOffset);
    {
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            intercept->PostCallRecordCmdBuildAccelerationStructureNVX(commandBuffer, type, instanceCount, instanceData, instanceOffset, geometryCount, pGeometries, flags, update, dst, src, scratch, scratchOffset);
        }
    }
}

VKAPI_ATTR void VKAPI_CALL CmdCopyAccelerationStructureNVX(
    VkCommandBuffer                             commandBuffer,
    VkAccelerationStructureNVX                  dst,
    VkAccelerationStructureNVX                  src,
    VkCopyAccelerationStructureModeNVX          mode) {
    layer_data *device_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    {
        bool skip = false;
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            skip |= intercept->PreCallValidateCmdCopyAccelerationStructureNVX(commandBuffer, dst, src, mode);
            if (skip) return;
        }
        for (auto intercept : global_interceptor_list) {
            intercept->PreCallRecordCmdCopyAccelerationStructureNVX(commandBuffer, dst, src, mode);
        }
    }
    device_data->dispatch_table.CmdCopyAccelerationStructureNVX(commandBuffer, dst, src, mode);
    {
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            intercept->PostCallRecordCmdCopyAccelerationStructureNVX(commandBuffer, dst, src, mode);
        }
    }
}

VKAPI_ATTR void VKAPI_CALL CmdTraceRaysNVX(
    VkCommandBuffer                             commandBuffer,
    VkBuffer                                    raygenShaderBindingTableBuffer,
    VkDeviceSize                                raygenShaderBindingOffset,
    VkBuffer                                    missShaderBindingTableBuffer,
    VkDeviceSize                                missShaderBindingOffset,
    VkDeviceSize                                missShaderBindingStride,
    VkBuffer                                    hitShaderBindingTableBuffer,
    VkDeviceSize                                hitShaderBindingOffset,
    VkDeviceSize                                hitShaderBindingStride,
    uint32_t                                    width,
    uint32_t                                    height) {
    layer_data *device_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    {
        bool skip = false;
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            skip |= intercept->PreCallValidateCmdTraceRaysNVX(commandBuffer, raygenShaderBindingTableBuffer, raygenShaderBindingOffset, missShaderBindingTableBuffer, missShaderBindingOffset, missShaderBindingStride, hitShaderBindingTableBuffer, hitShaderBindingOffset, hitShaderBindingStride, width, height);
            if (skip) return;
        }
        for (auto intercept : global_interceptor_list) {
            intercept->PreCallRecordCmdTraceRaysNVX(commandBuffer, raygenShaderBindingTableBuffer, raygenShaderBindingOffset, missShaderBindingTableBuffer, missShaderBindingOffset, missShaderBindingStride, hitShaderBindingTableBuffer, hitShaderBindingOffset, hitShaderBindingStride, width, height);
        }
    }
    device_data->dispatch_table.CmdTraceRaysNVX(commandBuffer, raygenShaderBindingTableBuffer, raygenShaderBindingOffset, missShaderBindingTableBuffer, missShaderBindingOffset, missShaderBindingStride, hitShaderBindingTableBuffer, hitShaderBindingOffset, hitShaderBindingStride, width, height);
    {
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            intercept->PostCallRecordCmdTraceRaysNVX(commandBuffer, raygenShaderBindingTableBuffer, raygenShaderBindingOffset, missShaderBindingTableBuffer, missShaderBindingOffset, missShaderBindingStride, hitShaderBindingTableBuffer, hitShaderBindingOffset, hitShaderBindingStride, width, height);
        }
    }
}

VKAPI_ATTR VkResult VKAPI_CALL CreateRaytracingPipelinesNVX(
    VkDevice                                    device,
    VkPipelineCache                             pipelineCache,
    uint32_t                                    createInfoCount,
    const VkRaytracingPipelineCreateInfoNVX*    pCreateInfos,
    const VkAllocationCallbacks*                pAllocator,
    VkPipeline*                                 pPipelines) {
    layer_data *device_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    {
        bool skip = false;
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            skip |= intercept->PreCallValidateCreateRaytracingPipelinesNVX(device, pipelineCache, createInfoCount, pCreateInfos, pAllocator, pPipelines);
            if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
        }
        for (auto intercept : global_interceptor_list) {
            intercept->PreCallRecordCreateRaytracingPipelinesNVX(device, pipelineCache, createInfoCount, pCreateInfos, pAllocator, pPipelines);
        }
    }
    VkResult result = device_data->dispatch_table.CreateRaytracingPipelinesNVX(device, pipelineCache, createInfoCount, pCreateInfos, pAllocator, pPipelines);
    if (VK_SUCCESS == result) {
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            intercept->PostCallRecordCreateRaytracingPipelinesNVX(device, pipelineCache, createInfoCount, pCreateInfos, pAllocator, pPipelines);
        }
    }
    return result;
}

VKAPI_ATTR VkResult VKAPI_CALL GetRaytracingShaderHandlesNVX(
    VkDevice                                    device,
    VkPipeline                                  pipeline,
    uint32_t                                    firstGroup,
    uint32_t                                    groupCount,
    size_t                                      dataSize,
    void*                                       pData) {
    layer_data *device_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    {
        bool skip = false;
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            skip |= intercept->PreCallValidateGetRaytracingShaderHandlesNVX(device, pipeline, firstGroup, groupCount, dataSize, pData);
            if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
        }
        for (auto intercept : global_interceptor_list) {
            intercept->PreCallRecordGetRaytracingShaderHandlesNVX(device, pipeline, firstGroup, groupCount, dataSize, pData);
        }
    }
    VkResult result = device_data->dispatch_table.GetRaytracingShaderHandlesNVX(device, pipeline, firstGroup, groupCount, dataSize, pData);
    if (VK_SUCCESS == result) {
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            intercept->PostCallRecordGetRaytracingShaderHandlesNVX(device, pipeline, firstGroup, groupCount, dataSize, pData);
        }
    }
    return result;
}

VKAPI_ATTR VkResult VKAPI_CALL GetAccelerationStructureHandleNVX(
    VkDevice                                    device,
    VkAccelerationStructureNVX                  accelerationStructure,
    size_t                                      dataSize,
    void*                                       pData) {
    layer_data *device_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    {
        bool skip = false;
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            skip |= intercept->PreCallValidateGetAccelerationStructureHandleNVX(device, accelerationStructure, dataSize, pData);
            if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
        }
        for (auto intercept : global_interceptor_list) {
            intercept->PreCallRecordGetAccelerationStructureHandleNVX(device, accelerationStructure, dataSize, pData);
        }
    }
    VkResult result = device_data->dispatch_table.GetAccelerationStructureHandleNVX(device, accelerationStructure, dataSize, pData);
    if (VK_SUCCESS == result) {
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            intercept->PostCallRecordGetAccelerationStructureHandleNVX(device, accelerationStructure, dataSize, pData);
        }
    }
    return result;
}

VKAPI_ATTR void VKAPI_CALL CmdWriteAccelerationStructurePropertiesNVX(
    VkCommandBuffer                             commandBuffer,
    VkAccelerationStructureNVX                  accelerationStructure,
    VkQueryType                                 queryType,
    VkQueryPool                                 queryPool,
    uint32_t                                    query) {
    layer_data *device_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    {
        bool skip = false;
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            skip |= intercept->PreCallValidateCmdWriteAccelerationStructurePropertiesNVX(commandBuffer, accelerationStructure, queryType, queryPool, query);
            if (skip) return;
        }
        for (auto intercept : global_interceptor_list) {
            intercept->PreCallRecordCmdWriteAccelerationStructurePropertiesNVX(commandBuffer, accelerationStructure, queryType, queryPool, query);
        }
    }
    device_data->dispatch_table.CmdWriteAccelerationStructurePropertiesNVX(commandBuffer, accelerationStructure, queryType, queryPool, query);
    {
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            intercept->PostCallRecordCmdWriteAccelerationStructurePropertiesNVX(commandBuffer, accelerationStructure, queryType, queryPool, query);
        }
    }
}

VKAPI_ATTR VkResult VKAPI_CALL CompileDeferredNVX(
    VkDevice                                    device,
    VkPipeline                                  pipeline,
    uint32_t                                    shader) {
    layer_data *device_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    {
        bool skip = false;
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            skip |= intercept->PreCallValidateCompileDeferredNVX(device, pipeline, shader);
            if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
        }
        for (auto intercept : global_interceptor_list) {
            intercept->PreCallRecordCompileDeferredNVX(device, pipeline, shader);
        }
    }
    VkResult result = device_data->dispatch_table.CompileDeferredNVX(device, pipeline, shader);
    if (VK_SUCCESS == result) {
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            intercept->PostCallRecordCompileDeferredNVX(device, pipeline, shader);
        }
    }
    return result;
}




VKAPI_ATTR VkResult VKAPI_CALL GetMemoryHostPointerPropertiesEXT(
    VkDevice                                    device,
    VkExternalMemoryHandleTypeFlagBits          handleType,
    const void*                                 pHostPointer,
    VkMemoryHostPointerPropertiesEXT*           pMemoryHostPointerProperties) {
    layer_data *device_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    {
        bool skip = false;
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            skip |= intercept->PreCallValidateGetMemoryHostPointerPropertiesEXT(device, handleType, pHostPointer, pMemoryHostPointerProperties);
            if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
        }
        for (auto intercept : global_interceptor_list) {
            intercept->PreCallRecordGetMemoryHostPointerPropertiesEXT(device, handleType, pHostPointer, pMemoryHostPointerProperties);
        }
    }
    VkResult result = device_data->dispatch_table.GetMemoryHostPointerPropertiesEXT(device, handleType, pHostPointer, pMemoryHostPointerProperties);
    if (VK_SUCCESS == result) {
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            intercept->PostCallRecordGetMemoryHostPointerPropertiesEXT(device, handleType, pHostPointer, pMemoryHostPointerProperties);
        }
    }
    return result;
}


VKAPI_ATTR void VKAPI_CALL CmdWriteBufferMarkerAMD(
    VkCommandBuffer                             commandBuffer,
    VkPipelineStageFlagBits                     pipelineStage,
    VkBuffer                                    dstBuffer,
    VkDeviceSize                                dstOffset,
    uint32_t                                    marker) {
    layer_data *device_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    {
        bool skip = false;
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            skip |= intercept->PreCallValidateCmdWriteBufferMarkerAMD(commandBuffer, pipelineStage, dstBuffer, dstOffset, marker);
            if (skip) return;
        }
        for (auto intercept : global_interceptor_list) {
            intercept->PreCallRecordCmdWriteBufferMarkerAMD(commandBuffer, pipelineStage, dstBuffer, dstOffset, marker);
        }
    }
    device_data->dispatch_table.CmdWriteBufferMarkerAMD(commandBuffer, pipelineStage, dstBuffer, dstOffset, marker);
    {
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            intercept->PostCallRecordCmdWriteBufferMarkerAMD(commandBuffer, pipelineStage, dstBuffer, dstOffset, marker);
        }
    }
}






VKAPI_ATTR void VKAPI_CALL CmdDrawMeshTasksNV(
    VkCommandBuffer                             commandBuffer,
    uint32_t                                    taskCount,
    uint32_t                                    firstTask) {
    layer_data *device_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    {
        bool skip = false;
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            skip |= intercept->PreCallValidateCmdDrawMeshTasksNV(commandBuffer, taskCount, firstTask);
            if (skip) return;
        }
        for (auto intercept : global_interceptor_list) {
            intercept->PreCallRecordCmdDrawMeshTasksNV(commandBuffer, taskCount, firstTask);
        }
    }
    device_data->dispatch_table.CmdDrawMeshTasksNV(commandBuffer, taskCount, firstTask);
    {
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            intercept->PostCallRecordCmdDrawMeshTasksNV(commandBuffer, taskCount, firstTask);
        }
    }
}

VKAPI_ATTR void VKAPI_CALL CmdDrawMeshTasksIndirectNV(
    VkCommandBuffer                             commandBuffer,
    VkBuffer                                    buffer,
    VkDeviceSize                                offset,
    uint32_t                                    drawCount,
    uint32_t                                    stride) {
    layer_data *device_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    {
        bool skip = false;
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            skip |= intercept->PreCallValidateCmdDrawMeshTasksIndirectNV(commandBuffer, buffer, offset, drawCount, stride);
            if (skip) return;
        }
        for (auto intercept : global_interceptor_list) {
            intercept->PreCallRecordCmdDrawMeshTasksIndirectNV(commandBuffer, buffer, offset, drawCount, stride);
        }
    }
    device_data->dispatch_table.CmdDrawMeshTasksIndirectNV(commandBuffer, buffer, offset, drawCount, stride);
    {
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            intercept->PostCallRecordCmdDrawMeshTasksIndirectNV(commandBuffer, buffer, offset, drawCount, stride);
        }
    }
}

VKAPI_ATTR void VKAPI_CALL CmdDrawMeshTasksIndirectCountNV(
    VkCommandBuffer                             commandBuffer,
    VkBuffer                                    buffer,
    VkDeviceSize                                offset,
    VkBuffer                                    countBuffer,
    VkDeviceSize                                countBufferOffset,
    uint32_t                                    maxDrawCount,
    uint32_t                                    stride) {
    layer_data *device_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    {
        bool skip = false;
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            skip |= intercept->PreCallValidateCmdDrawMeshTasksIndirectCountNV(commandBuffer, buffer, offset, countBuffer, countBufferOffset, maxDrawCount, stride);
            if (skip) return;
        }
        for (auto intercept : global_interceptor_list) {
            intercept->PreCallRecordCmdDrawMeshTasksIndirectCountNV(commandBuffer, buffer, offset, countBuffer, countBufferOffset, maxDrawCount, stride);
        }
    }
    device_data->dispatch_table.CmdDrawMeshTasksIndirectCountNV(commandBuffer, buffer, offset, countBuffer, countBufferOffset, maxDrawCount, stride);
    {
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            intercept->PostCallRecordCmdDrawMeshTasksIndirectCountNV(commandBuffer, buffer, offset, countBuffer, countBufferOffset, maxDrawCount, stride);
        }
    }
}




VKAPI_ATTR void VKAPI_CALL CmdSetExclusiveScissorNV(
    VkCommandBuffer                             commandBuffer,
    uint32_t                                    firstExclusiveScissor,
    uint32_t                                    exclusiveScissorCount,
    const VkRect2D*                             pExclusiveScissors) {
    layer_data *device_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    {
        bool skip = false;
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            skip |= intercept->PreCallValidateCmdSetExclusiveScissorNV(commandBuffer, firstExclusiveScissor, exclusiveScissorCount, pExclusiveScissors);
            if (skip) return;
        }
        for (auto intercept : global_interceptor_list) {
            intercept->PreCallRecordCmdSetExclusiveScissorNV(commandBuffer, firstExclusiveScissor, exclusiveScissorCount, pExclusiveScissors);
        }
    }
    device_data->dispatch_table.CmdSetExclusiveScissorNV(commandBuffer, firstExclusiveScissor, exclusiveScissorCount, pExclusiveScissors);
    {
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            intercept->PostCallRecordCmdSetExclusiveScissorNV(commandBuffer, firstExclusiveScissor, exclusiveScissorCount, pExclusiveScissors);
        }
    }
}


VKAPI_ATTR void VKAPI_CALL CmdSetCheckpointNV(
    VkCommandBuffer                             commandBuffer,
    const void*                                 pCheckpointMarker) {
    layer_data *device_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    {
        bool skip = false;
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            skip |= intercept->PreCallValidateCmdSetCheckpointNV(commandBuffer, pCheckpointMarker);
            if (skip) return;
        }
        for (auto intercept : global_interceptor_list) {
            intercept->PreCallRecordCmdSetCheckpointNV(commandBuffer, pCheckpointMarker);
        }
    }
    device_data->dispatch_table.CmdSetCheckpointNV(commandBuffer, pCheckpointMarker);
    {
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            intercept->PostCallRecordCmdSetCheckpointNV(commandBuffer, pCheckpointMarker);
        }
    }
}

VKAPI_ATTR void VKAPI_CALL GetQueueCheckpointDataNV(
    VkQueue                                     queue,
    uint32_t*                                   pCheckpointDataCount,
    VkCheckpointDataNV*                         pCheckpointData) {
    layer_data *device_data = GetLayerDataPtr(get_dispatch_key(queue), layer_data_map);
    {
        bool skip = false;
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            skip |= intercept->PreCallValidateGetQueueCheckpointDataNV(queue, pCheckpointDataCount, pCheckpointData);
            if (skip) return;
        }
        for (auto intercept : global_interceptor_list) {
            intercept->PreCallRecordGetQueueCheckpointDataNV(queue, pCheckpointDataCount, pCheckpointData);
        }
    }
    device_data->dispatch_table.GetQueueCheckpointDataNV(queue, pCheckpointDataCount, pCheckpointData);
    {
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            intercept->PostCallRecordGetQueueCheckpointDataNV(queue, pCheckpointDataCount, pCheckpointData);
        }
    }
}

#ifdef VK_USE_PLATFORM_FUCHSIA

VKAPI_ATTR VkResult VKAPI_CALL CreateImagePipeSurfaceFUCHSIA(
    VkInstance                                  instance,
    const VkImagePipeSurfaceCreateInfoFUCHSIA*  pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkSurfaceKHR*                               pSurface) {
    instance_layer_data *instance_data = GetLayerDataPtr(get_dispatch_key(instance), instance_layer_data_map);
    {
        bool skip = false;
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            skip |= intercept->PreCallValidateCreateImagePipeSurfaceFUCHSIA(instance, pCreateInfo, pAllocator, pSurface);
            if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
        }
        for (auto intercept : global_interceptor_list) {
            intercept->PreCallRecordCreateImagePipeSurfaceFUCHSIA(instance, pCreateInfo, pAllocator, pSurface);
        }
    }
    VkResult result = instance_data->dispatch_table.CreateImagePipeSurfaceFUCHSIA(instance, pCreateInfo, pAllocator, pSurface);
    if (VK_SUCCESS == result) {
        std::lock_guard<std::mutex> lock(global_lock);
        for (auto intercept : global_interceptor_list) {
            intercept->PostCallRecordCreateImagePipeSurfaceFUCHSIA(instance, pCreateInfo, pAllocator, pSurface);
        }
    }
    return result;
}
#endif // VK_USE_PLATFORM_FUCHSIA

// Map of all APIs to be intercepted by this layer
const std::unordered_map<std::string, void*> name_to_funcptr_map = {
    {"vkCreateInstance", (void*)CreateInstance},
    {"vkDestroyInstance", (void*)DestroyInstance},
    {"vkEnumeratePhysicalDevices", (void*)EnumeratePhysicalDevices},
    {"vkGetPhysicalDeviceFeatures", (void*)GetPhysicalDeviceFeatures},
    {"vkGetPhysicalDeviceFormatProperties", (void*)GetPhysicalDeviceFormatProperties},
    {"vkGetPhysicalDeviceImageFormatProperties", (void*)GetPhysicalDeviceImageFormatProperties},
    {"vkGetPhysicalDeviceProperties", (void*)GetPhysicalDeviceProperties},
    {"vkGetPhysicalDeviceQueueFamilyProperties", (void*)GetPhysicalDeviceQueueFamilyProperties},
    {"vkGetPhysicalDeviceMemoryProperties", (void*)GetPhysicalDeviceMemoryProperties},
    {"vkGetInstanceProcAddr", (void*)GetInstanceProcAddr},
    {"vkGetDeviceProcAddr", (void*)GetDeviceProcAddr},
    {"vkCreateDevice", (void*)CreateDevice},
    {"vkDestroyDevice", (void*)DestroyDevice},
    {"vkEnumerateInstanceExtensionProperties", (void*)EnumerateInstanceExtensionProperties},
    {"vkEnumerateDeviceExtensionProperties", (void*)EnumerateDeviceExtensionProperties},
    {"vkEnumerateInstanceLayerProperties", (void*)EnumerateInstanceLayerProperties},
    {"vkEnumerateDeviceLayerProperties", (void*)EnumerateDeviceLayerProperties},
    {"vkGetDeviceQueue", (void*)GetDeviceQueue},
    {"vkQueueSubmit", (void*)QueueSubmit},
    {"vkQueueWaitIdle", (void*)QueueWaitIdle},
    {"vkDeviceWaitIdle", (void*)DeviceWaitIdle},
    {"vkAllocateMemory", (void*)AllocateMemory},
    {"vkFreeMemory", (void*)FreeMemory},
    {"vkMapMemory", (void*)MapMemory},
    {"vkUnmapMemory", (void*)UnmapMemory},
    {"vkFlushMappedMemoryRanges", (void*)FlushMappedMemoryRanges},
    {"vkInvalidateMappedMemoryRanges", (void*)InvalidateMappedMemoryRanges},
    {"vkGetDeviceMemoryCommitment", (void*)GetDeviceMemoryCommitment},
    {"vkBindBufferMemory", (void*)BindBufferMemory},
    {"vkBindImageMemory", (void*)BindImageMemory},
    {"vkGetBufferMemoryRequirements", (void*)GetBufferMemoryRequirements},
    {"vkGetImageMemoryRequirements", (void*)GetImageMemoryRequirements},
    {"vkGetImageSparseMemoryRequirements", (void*)GetImageSparseMemoryRequirements},
    {"vkGetPhysicalDeviceSparseImageFormatProperties", (void*)GetPhysicalDeviceSparseImageFormatProperties},
    {"vkQueueBindSparse", (void*)QueueBindSparse},
    {"vkCreateFence", (void*)CreateFence},
    {"vkDestroyFence", (void*)DestroyFence},
    {"vkResetFences", (void*)ResetFences},
    {"vkGetFenceStatus", (void*)GetFenceStatus},
    {"vkWaitForFences", (void*)WaitForFences},
    {"vkCreateSemaphore", (void*)CreateSemaphore},
    {"vkDestroySemaphore", (void*)DestroySemaphore},
    {"vkCreateEvent", (void*)CreateEvent},
    {"vkDestroyEvent", (void*)DestroyEvent},
    {"vkGetEventStatus", (void*)GetEventStatus},
    {"vkSetEvent", (void*)SetEvent},
    {"vkResetEvent", (void*)ResetEvent},
    {"vkCreateQueryPool", (void*)CreateQueryPool},
    {"vkDestroyQueryPool", (void*)DestroyQueryPool},
    {"vkGetQueryPoolResults", (void*)GetQueryPoolResults},
    {"vkCreateBuffer", (void*)CreateBuffer},
    {"vkDestroyBuffer", (void*)DestroyBuffer},
    {"vkCreateBufferView", (void*)CreateBufferView},
    {"vkDestroyBufferView", (void*)DestroyBufferView},
    {"vkCreateImage", (void*)CreateImage},
    {"vkDestroyImage", (void*)DestroyImage},
    {"vkGetImageSubresourceLayout", (void*)GetImageSubresourceLayout},
    {"vkCreateImageView", (void*)CreateImageView},
    {"vkDestroyImageView", (void*)DestroyImageView},
    {"vkCreateShaderModule", (void*)CreateShaderModule},
    {"vkDestroyShaderModule", (void*)DestroyShaderModule},
    {"vkCreatePipelineCache", (void*)CreatePipelineCache},
    {"vkDestroyPipelineCache", (void*)DestroyPipelineCache},
    {"vkGetPipelineCacheData", (void*)GetPipelineCacheData},
    {"vkMergePipelineCaches", (void*)MergePipelineCaches},
    {"vkCreateGraphicsPipelines", (void*)CreateGraphicsPipelines},
    {"vkCreateComputePipelines", (void*)CreateComputePipelines},
    {"vkDestroyPipeline", (void*)DestroyPipeline},
    {"vkCreatePipelineLayout", (void*)CreatePipelineLayout},
    {"vkDestroyPipelineLayout", (void*)DestroyPipelineLayout},
    {"vkCreateSampler", (void*)CreateSampler},
    {"vkDestroySampler", (void*)DestroySampler},
    {"vkCreateDescriptorSetLayout", (void*)CreateDescriptorSetLayout},
    {"vkDestroyDescriptorSetLayout", (void*)DestroyDescriptorSetLayout},
    {"vkCreateDescriptorPool", (void*)CreateDescriptorPool},
    {"vkDestroyDescriptorPool", (void*)DestroyDescriptorPool},
    {"vkResetDescriptorPool", (void*)ResetDescriptorPool},
    {"vkAllocateDescriptorSets", (void*)AllocateDescriptorSets},
    {"vkFreeDescriptorSets", (void*)FreeDescriptorSets},
    {"vkUpdateDescriptorSets", (void*)UpdateDescriptorSets},
    {"vkCreateFramebuffer", (void*)CreateFramebuffer},
    {"vkDestroyFramebuffer", (void*)DestroyFramebuffer},
    {"vkCreateRenderPass", (void*)CreateRenderPass},
    {"vkDestroyRenderPass", (void*)DestroyRenderPass},
    {"vkGetRenderAreaGranularity", (void*)GetRenderAreaGranularity},
    {"vkCreateCommandPool", (void*)CreateCommandPool},
    {"vkDestroyCommandPool", (void*)DestroyCommandPool},
    {"vkResetCommandPool", (void*)ResetCommandPool},
    {"vkAllocateCommandBuffers", (void*)AllocateCommandBuffers},
    {"vkFreeCommandBuffers", (void*)FreeCommandBuffers},
    {"vkBeginCommandBuffer", (void*)BeginCommandBuffer},
    {"vkEndCommandBuffer", (void*)EndCommandBuffer},
    {"vkResetCommandBuffer", (void*)ResetCommandBuffer},
    {"vkCmdBindPipeline", (void*)CmdBindPipeline},
    {"vkCmdSetViewport", (void*)CmdSetViewport},
    {"vkCmdSetScissor", (void*)CmdSetScissor},
    {"vkCmdSetLineWidth", (void*)CmdSetLineWidth},
    {"vkCmdSetDepthBias", (void*)CmdSetDepthBias},
    {"vkCmdSetBlendConstants", (void*)CmdSetBlendConstants},
    {"vkCmdSetDepthBounds", (void*)CmdSetDepthBounds},
    {"vkCmdSetStencilCompareMask", (void*)CmdSetStencilCompareMask},
    {"vkCmdSetStencilWriteMask", (void*)CmdSetStencilWriteMask},
    {"vkCmdSetStencilReference", (void*)CmdSetStencilReference},
    {"vkCmdBindDescriptorSets", (void*)CmdBindDescriptorSets},
    {"vkCmdBindIndexBuffer", (void*)CmdBindIndexBuffer},
    {"vkCmdBindVertexBuffers", (void*)CmdBindVertexBuffers},
    {"vkCmdDraw", (void*)CmdDraw},
    {"vkCmdDrawIndexed", (void*)CmdDrawIndexed},
    {"vkCmdDrawIndirect", (void*)CmdDrawIndirect},
    {"vkCmdDrawIndexedIndirect", (void*)CmdDrawIndexedIndirect},
    {"vkCmdDispatch", (void*)CmdDispatch},
    {"vkCmdDispatchIndirect", (void*)CmdDispatchIndirect},
    {"vkCmdCopyBuffer", (void*)CmdCopyBuffer},
    {"vkCmdCopyImage", (void*)CmdCopyImage},
    {"vkCmdBlitImage", (void*)CmdBlitImage},
    {"vkCmdCopyBufferToImage", (void*)CmdCopyBufferToImage},
    {"vkCmdCopyImageToBuffer", (void*)CmdCopyImageToBuffer},
    {"vkCmdUpdateBuffer", (void*)CmdUpdateBuffer},
    {"vkCmdFillBuffer", (void*)CmdFillBuffer},
    {"vkCmdClearColorImage", (void*)CmdClearColorImage},
    {"vkCmdClearDepthStencilImage", (void*)CmdClearDepthStencilImage},
    {"vkCmdClearAttachments", (void*)CmdClearAttachments},
    {"vkCmdResolveImage", (void*)CmdResolveImage},
    {"vkCmdSetEvent", (void*)CmdSetEvent},
    {"vkCmdResetEvent", (void*)CmdResetEvent},
    {"vkCmdWaitEvents", (void*)CmdWaitEvents},
    {"vkCmdPipelineBarrier", (void*)CmdPipelineBarrier},
    {"vkCmdBeginQuery", (void*)CmdBeginQuery},
    {"vkCmdEndQuery", (void*)CmdEndQuery},
    {"vkCmdResetQueryPool", (void*)CmdResetQueryPool},
    {"vkCmdWriteTimestamp", (void*)CmdWriteTimestamp},
    {"vkCmdCopyQueryPoolResults", (void*)CmdCopyQueryPoolResults},
    {"vkCmdPushConstants", (void*)CmdPushConstants},
    {"vkCmdBeginRenderPass", (void*)CmdBeginRenderPass},
    {"vkCmdNextSubpass", (void*)CmdNextSubpass},
    {"vkCmdEndRenderPass", (void*)CmdEndRenderPass},
    {"vkCmdExecuteCommands", (void*)CmdExecuteCommands},
    {"vkBindBufferMemory2", (void*)BindBufferMemory2},
    {"vkBindImageMemory2", (void*)BindImageMemory2},
    {"vkGetDeviceGroupPeerMemoryFeatures", (void*)GetDeviceGroupPeerMemoryFeatures},
    {"vkCmdSetDeviceMask", (void*)CmdSetDeviceMask},
    {"vkCmdDispatchBase", (void*)CmdDispatchBase},
    {"vkEnumeratePhysicalDeviceGroups", (void*)EnumeratePhysicalDeviceGroups},
    {"vkGetImageMemoryRequirements2", (void*)GetImageMemoryRequirements2},
    {"vkGetBufferMemoryRequirements2", (void*)GetBufferMemoryRequirements2},
    {"vkGetImageSparseMemoryRequirements2", (void*)GetImageSparseMemoryRequirements2},
    {"vkGetPhysicalDeviceFeatures2", (void*)GetPhysicalDeviceFeatures2},
    {"vkGetPhysicalDeviceProperties2", (void*)GetPhysicalDeviceProperties2},
    {"vkGetPhysicalDeviceFormatProperties2", (void*)GetPhysicalDeviceFormatProperties2},
    {"vkGetPhysicalDeviceImageFormatProperties2", (void*)GetPhysicalDeviceImageFormatProperties2},
    {"vkGetPhysicalDeviceQueueFamilyProperties2", (void*)GetPhysicalDeviceQueueFamilyProperties2},
    {"vkGetPhysicalDeviceMemoryProperties2", (void*)GetPhysicalDeviceMemoryProperties2},
    {"vkGetPhysicalDeviceSparseImageFormatProperties2", (void*)GetPhysicalDeviceSparseImageFormatProperties2},
    {"vkTrimCommandPool", (void*)TrimCommandPool},
    {"vkGetDeviceQueue2", (void*)GetDeviceQueue2},
    {"vkCreateSamplerYcbcrConversion", (void*)CreateSamplerYcbcrConversion},
    {"vkDestroySamplerYcbcrConversion", (void*)DestroySamplerYcbcrConversion},
    {"vkCreateDescriptorUpdateTemplate", (void*)CreateDescriptorUpdateTemplate},
    {"vkDestroyDescriptorUpdateTemplate", (void*)DestroyDescriptorUpdateTemplate},
    {"vkUpdateDescriptorSetWithTemplate", (void*)UpdateDescriptorSetWithTemplate},
    {"vkGetPhysicalDeviceExternalBufferProperties", (void*)GetPhysicalDeviceExternalBufferProperties},
    {"vkGetPhysicalDeviceExternalFenceProperties", (void*)GetPhysicalDeviceExternalFenceProperties},
    {"vkGetPhysicalDeviceExternalSemaphoreProperties", (void*)GetPhysicalDeviceExternalSemaphoreProperties},
    {"vkGetDescriptorSetLayoutSupport", (void*)GetDescriptorSetLayoutSupport},
    {"vkDestroySurfaceKHR", (void*)DestroySurfaceKHR},
    {"vkGetPhysicalDeviceSurfaceSupportKHR", (void*)GetPhysicalDeviceSurfaceSupportKHR},
    {"vkGetPhysicalDeviceSurfaceCapabilitiesKHR", (void*)GetPhysicalDeviceSurfaceCapabilitiesKHR},
    {"vkGetPhysicalDeviceSurfaceFormatsKHR", (void*)GetPhysicalDeviceSurfaceFormatsKHR},
    {"vkGetPhysicalDeviceSurfacePresentModesKHR", (void*)GetPhysicalDeviceSurfacePresentModesKHR},
    {"vkCreateSwapchainKHR", (void*)CreateSwapchainKHR},
    {"vkDestroySwapchainKHR", (void*)DestroySwapchainKHR},
    {"vkGetSwapchainImagesKHR", (void*)GetSwapchainImagesKHR},
    {"vkAcquireNextImageKHR", (void*)AcquireNextImageKHR},
    {"vkQueuePresentKHR", (void*)QueuePresentKHR},
    {"vkGetDeviceGroupPresentCapabilitiesKHR", (void*)GetDeviceGroupPresentCapabilitiesKHR},
    {"vkGetDeviceGroupSurfacePresentModesKHR", (void*)GetDeviceGroupSurfacePresentModesKHR},
    {"vkGetPhysicalDevicePresentRectanglesKHR", (void*)GetPhysicalDevicePresentRectanglesKHR},
    {"vkAcquireNextImage2KHR", (void*)AcquireNextImage2KHR},
    {"vkGetPhysicalDeviceDisplayPropertiesKHR", (void*)GetPhysicalDeviceDisplayPropertiesKHR},
    {"vkGetPhysicalDeviceDisplayPlanePropertiesKHR", (void*)GetPhysicalDeviceDisplayPlanePropertiesKHR},
    {"vkGetDisplayPlaneSupportedDisplaysKHR", (void*)GetDisplayPlaneSupportedDisplaysKHR},
    {"vkGetDisplayModePropertiesKHR", (void*)GetDisplayModePropertiesKHR},
    {"vkCreateDisplayModeKHR", (void*)CreateDisplayModeKHR},
    {"vkGetDisplayPlaneCapabilitiesKHR", (void*)GetDisplayPlaneCapabilitiesKHR},
    {"vkCreateDisplayPlaneSurfaceKHR", (void*)CreateDisplayPlaneSurfaceKHR},
    {"vkCreateSharedSwapchainsKHR", (void*)CreateSharedSwapchainsKHR},
#ifdef VK_USE_PLATFORM_XLIB_KHR
    {"vkCreateXlibSurfaceKHR", (void*)CreateXlibSurfaceKHR},
#endif
#ifdef VK_USE_PLATFORM_XLIB_KHR
    {"vkGetPhysicalDeviceXlibPresentationSupportKHR", (void*)GetPhysicalDeviceXlibPresentationSupportKHR},
#endif
#ifdef VK_USE_PLATFORM_XCB_KHR
    {"vkCreateXcbSurfaceKHR", (void*)CreateXcbSurfaceKHR},
#endif
#ifdef VK_USE_PLATFORM_XCB_KHR
    {"vkGetPhysicalDeviceXcbPresentationSupportKHR", (void*)GetPhysicalDeviceXcbPresentationSupportKHR},
#endif
#ifdef VK_USE_PLATFORM_WAYLAND_KHR
    {"vkCreateWaylandSurfaceKHR", (void*)CreateWaylandSurfaceKHR},
#endif
#ifdef VK_USE_PLATFORM_WAYLAND_KHR
    {"vkGetPhysicalDeviceWaylandPresentationSupportKHR", (void*)GetPhysicalDeviceWaylandPresentationSupportKHR},
#endif
#ifdef VK_USE_PLATFORM_MIR_KHR
    {"vkCreateMirSurfaceKHR", (void*)CreateMirSurfaceKHR},
#endif
#ifdef VK_USE_PLATFORM_MIR_KHR
    {"vkGetPhysicalDeviceMirPresentationSupportKHR", (void*)GetPhysicalDeviceMirPresentationSupportKHR},
#endif
#ifdef VK_USE_PLATFORM_ANDROID_KHR
    {"vkCreateAndroidSurfaceKHR", (void*)CreateAndroidSurfaceKHR},
#endif
#ifdef VK_USE_PLATFORM_WIN32_KHR
    {"vkCreateWin32SurfaceKHR", (void*)CreateWin32SurfaceKHR},
#endif
#ifdef VK_USE_PLATFORM_WIN32_KHR
    {"vkGetPhysicalDeviceWin32PresentationSupportKHR", (void*)GetPhysicalDeviceWin32PresentationSupportKHR},
#endif
    {"vkGetPhysicalDeviceFeatures2KHR", (void*)GetPhysicalDeviceFeatures2KHR},
    {"vkGetPhysicalDeviceProperties2KHR", (void*)GetPhysicalDeviceProperties2KHR},
    {"vkGetPhysicalDeviceFormatProperties2KHR", (void*)GetPhysicalDeviceFormatProperties2KHR},
    {"vkGetPhysicalDeviceImageFormatProperties2KHR", (void*)GetPhysicalDeviceImageFormatProperties2KHR},
    {"vkGetPhysicalDeviceQueueFamilyProperties2KHR", (void*)GetPhysicalDeviceQueueFamilyProperties2KHR},
    {"vkGetPhysicalDeviceMemoryProperties2KHR", (void*)GetPhysicalDeviceMemoryProperties2KHR},
    {"vkGetPhysicalDeviceSparseImageFormatProperties2KHR", (void*)GetPhysicalDeviceSparseImageFormatProperties2KHR},
    {"vkGetDeviceGroupPeerMemoryFeaturesKHR", (void*)GetDeviceGroupPeerMemoryFeaturesKHR},
    {"vkCmdSetDeviceMaskKHR", (void*)CmdSetDeviceMaskKHR},
    {"vkCmdDispatchBaseKHR", (void*)CmdDispatchBaseKHR},
    {"vkTrimCommandPoolKHR", (void*)TrimCommandPoolKHR},
    {"vkEnumeratePhysicalDeviceGroupsKHR", (void*)EnumeratePhysicalDeviceGroupsKHR},
    {"vkGetPhysicalDeviceExternalBufferPropertiesKHR", (void*)GetPhysicalDeviceExternalBufferPropertiesKHR},
#ifdef VK_USE_PLATFORM_WIN32_KHR
    {"vkGetMemoryWin32HandleKHR", (void*)GetMemoryWin32HandleKHR},
#endif
#ifdef VK_USE_PLATFORM_WIN32_KHR
    {"vkGetMemoryWin32HandlePropertiesKHR", (void*)GetMemoryWin32HandlePropertiesKHR},
#endif
    {"vkGetMemoryFdKHR", (void*)GetMemoryFdKHR},
    {"vkGetMemoryFdPropertiesKHR", (void*)GetMemoryFdPropertiesKHR},
    {"vkGetPhysicalDeviceExternalSemaphorePropertiesKHR", (void*)GetPhysicalDeviceExternalSemaphorePropertiesKHR},
#ifdef VK_USE_PLATFORM_WIN32_KHR
    {"vkImportSemaphoreWin32HandleKHR", (void*)ImportSemaphoreWin32HandleKHR},
#endif
#ifdef VK_USE_PLATFORM_WIN32_KHR
    {"vkGetSemaphoreWin32HandleKHR", (void*)GetSemaphoreWin32HandleKHR},
#endif
    {"vkImportSemaphoreFdKHR", (void*)ImportSemaphoreFdKHR},
    {"vkGetSemaphoreFdKHR", (void*)GetSemaphoreFdKHR},
    {"vkCmdPushDescriptorSetKHR", (void*)CmdPushDescriptorSetKHR},
    {"vkCmdPushDescriptorSetWithTemplateKHR", (void*)CmdPushDescriptorSetWithTemplateKHR},
    {"vkCreateDescriptorUpdateTemplateKHR", (void*)CreateDescriptorUpdateTemplateKHR},
    {"vkDestroyDescriptorUpdateTemplateKHR", (void*)DestroyDescriptorUpdateTemplateKHR},
    {"vkUpdateDescriptorSetWithTemplateKHR", (void*)UpdateDescriptorSetWithTemplateKHR},
    {"vkCreateRenderPass2KHR", (void*)CreateRenderPass2KHR},
    {"vkCmdBeginRenderPass2KHR", (void*)CmdBeginRenderPass2KHR},
    {"vkCmdNextSubpass2KHR", (void*)CmdNextSubpass2KHR},
    {"vkCmdEndRenderPass2KHR", (void*)CmdEndRenderPass2KHR},
    {"vkGetSwapchainStatusKHR", (void*)GetSwapchainStatusKHR},
    {"vkGetPhysicalDeviceExternalFencePropertiesKHR", (void*)GetPhysicalDeviceExternalFencePropertiesKHR},
#ifdef VK_USE_PLATFORM_WIN32_KHR
    {"vkImportFenceWin32HandleKHR", (void*)ImportFenceWin32HandleKHR},
#endif
#ifdef VK_USE_PLATFORM_WIN32_KHR
    {"vkGetFenceWin32HandleKHR", (void*)GetFenceWin32HandleKHR},
#endif
    {"vkImportFenceFdKHR", (void*)ImportFenceFdKHR},
    {"vkGetFenceFdKHR", (void*)GetFenceFdKHR},
    {"vkGetPhysicalDeviceSurfaceCapabilities2KHR", (void*)GetPhysicalDeviceSurfaceCapabilities2KHR},
    {"vkGetPhysicalDeviceSurfaceFormats2KHR", (void*)GetPhysicalDeviceSurfaceFormats2KHR},
    {"vkGetPhysicalDeviceDisplayProperties2KHR", (void*)GetPhysicalDeviceDisplayProperties2KHR},
    {"vkGetPhysicalDeviceDisplayPlaneProperties2KHR", (void*)GetPhysicalDeviceDisplayPlaneProperties2KHR},
    {"vkGetDisplayModeProperties2KHR", (void*)GetDisplayModeProperties2KHR},
    {"vkGetDisplayPlaneCapabilities2KHR", (void*)GetDisplayPlaneCapabilities2KHR},
    {"vkGetImageMemoryRequirements2KHR", (void*)GetImageMemoryRequirements2KHR},
    {"vkGetBufferMemoryRequirements2KHR", (void*)GetBufferMemoryRequirements2KHR},
    {"vkGetImageSparseMemoryRequirements2KHR", (void*)GetImageSparseMemoryRequirements2KHR},
    {"vkCreateSamplerYcbcrConversionKHR", (void*)CreateSamplerYcbcrConversionKHR},
    {"vkDestroySamplerYcbcrConversionKHR", (void*)DestroySamplerYcbcrConversionKHR},
    {"vkBindBufferMemory2KHR", (void*)BindBufferMemory2KHR},
    {"vkBindImageMemory2KHR", (void*)BindImageMemory2KHR},
    {"vkGetDescriptorSetLayoutSupportKHR", (void*)GetDescriptorSetLayoutSupportKHR},
    {"vkCmdDrawIndirectCountKHR", (void*)CmdDrawIndirectCountKHR},
    {"vkCmdDrawIndexedIndirectCountKHR", (void*)CmdDrawIndexedIndirectCountKHR},
    {"vkCreateDebugReportCallbackEXT", (void*)CreateDebugReportCallbackEXT},
    {"vkDestroyDebugReportCallbackEXT", (void*)DestroyDebugReportCallbackEXT},
    {"vkDebugReportMessageEXT", (void*)DebugReportMessageEXT},
    {"vkDebugMarkerSetObjectTagEXT", (void*)DebugMarkerSetObjectTagEXT},
    {"vkDebugMarkerSetObjectNameEXT", (void*)DebugMarkerSetObjectNameEXT},
    {"vkCmdDebugMarkerBeginEXT", (void*)CmdDebugMarkerBeginEXT},
    {"vkCmdDebugMarkerEndEXT", (void*)CmdDebugMarkerEndEXT},
    {"vkCmdDebugMarkerInsertEXT", (void*)CmdDebugMarkerInsertEXT},
    {"vkCmdDrawIndirectCountAMD", (void*)CmdDrawIndirectCountAMD},
    {"vkCmdDrawIndexedIndirectCountAMD", (void*)CmdDrawIndexedIndirectCountAMD},
    {"vkGetShaderInfoAMD", (void*)GetShaderInfoAMD},
    {"vkGetPhysicalDeviceExternalImageFormatPropertiesNV", (void*)GetPhysicalDeviceExternalImageFormatPropertiesNV},
#ifdef VK_USE_PLATFORM_WIN32_KHR
    {"vkGetMemoryWin32HandleNV", (void*)GetMemoryWin32HandleNV},
#endif
#ifdef VK_USE_PLATFORM_VI_NN
    {"vkCreateViSurfaceNN", (void*)CreateViSurfaceNN},
#endif
    {"vkCmdBeginConditionalRenderingEXT", (void*)CmdBeginConditionalRenderingEXT},
    {"vkCmdEndConditionalRenderingEXT", (void*)CmdEndConditionalRenderingEXT},
    {"vkCmdProcessCommandsNVX", (void*)CmdProcessCommandsNVX},
    {"vkCmdReserveSpaceForCommandsNVX", (void*)CmdReserveSpaceForCommandsNVX},
    {"vkCreateIndirectCommandsLayoutNVX", (void*)CreateIndirectCommandsLayoutNVX},
    {"vkDestroyIndirectCommandsLayoutNVX", (void*)DestroyIndirectCommandsLayoutNVX},
    {"vkCreateObjectTableNVX", (void*)CreateObjectTableNVX},
    {"vkDestroyObjectTableNVX", (void*)DestroyObjectTableNVX},
    {"vkRegisterObjectsNVX", (void*)RegisterObjectsNVX},
    {"vkUnregisterObjectsNVX", (void*)UnregisterObjectsNVX},
    {"vkGetPhysicalDeviceGeneratedCommandsPropertiesNVX", (void*)GetPhysicalDeviceGeneratedCommandsPropertiesNVX},
    {"vkCmdSetViewportWScalingNV", (void*)CmdSetViewportWScalingNV},
    {"vkReleaseDisplayEXT", (void*)ReleaseDisplayEXT},
#ifdef VK_USE_PLATFORM_XLIB_XRANDR_EXT
    {"vkAcquireXlibDisplayEXT", (void*)AcquireXlibDisplayEXT},
#endif
#ifdef VK_USE_PLATFORM_XLIB_XRANDR_EXT
    {"vkGetRandROutputDisplayEXT", (void*)GetRandROutputDisplayEXT},
#endif
    {"vkGetPhysicalDeviceSurfaceCapabilities2EXT", (void*)GetPhysicalDeviceSurfaceCapabilities2EXT},
    {"vkDisplayPowerControlEXT", (void*)DisplayPowerControlEXT},
    {"vkRegisterDeviceEventEXT", (void*)RegisterDeviceEventEXT},
    {"vkRegisterDisplayEventEXT", (void*)RegisterDisplayEventEXT},
    {"vkGetSwapchainCounterEXT", (void*)GetSwapchainCounterEXT},
    {"vkGetRefreshCycleDurationGOOGLE", (void*)GetRefreshCycleDurationGOOGLE},
    {"vkGetPastPresentationTimingGOOGLE", (void*)GetPastPresentationTimingGOOGLE},
    {"vkCmdSetDiscardRectangleEXT", (void*)CmdSetDiscardRectangleEXT},
    {"vkSetHdrMetadataEXT", (void*)SetHdrMetadataEXT},
#ifdef VK_USE_PLATFORM_IOS_MVK
    {"vkCreateIOSSurfaceMVK", (void*)CreateIOSSurfaceMVK},
#endif
#ifdef VK_USE_PLATFORM_MACOS_MVK
    {"vkCreateMacOSSurfaceMVK", (void*)CreateMacOSSurfaceMVK},
#endif
    {"vkSetDebugUtilsObjectNameEXT", (void*)SetDebugUtilsObjectNameEXT},
    {"vkSetDebugUtilsObjectTagEXT", (void*)SetDebugUtilsObjectTagEXT},
    {"vkQueueBeginDebugUtilsLabelEXT", (void*)QueueBeginDebugUtilsLabelEXT},
    {"vkQueueEndDebugUtilsLabelEXT", (void*)QueueEndDebugUtilsLabelEXT},
    {"vkQueueInsertDebugUtilsLabelEXT", (void*)QueueInsertDebugUtilsLabelEXT},
    {"vkCmdBeginDebugUtilsLabelEXT", (void*)CmdBeginDebugUtilsLabelEXT},
    {"vkCmdEndDebugUtilsLabelEXT", (void*)CmdEndDebugUtilsLabelEXT},
    {"vkCmdInsertDebugUtilsLabelEXT", (void*)CmdInsertDebugUtilsLabelEXT},
    {"vkCreateDebugUtilsMessengerEXT", (void*)CreateDebugUtilsMessengerEXT},
    {"vkDestroyDebugUtilsMessengerEXT", (void*)DestroyDebugUtilsMessengerEXT},
    {"vkSubmitDebugUtilsMessageEXT", (void*)SubmitDebugUtilsMessageEXT},
#ifdef VK_USE_PLATFORM_ANDROID_KHR
    {"vkGetAndroidHardwareBufferPropertiesANDROID", (void*)GetAndroidHardwareBufferPropertiesANDROID},
#endif
#ifdef VK_USE_PLATFORM_ANDROID_KHR
    {"vkGetMemoryAndroidHardwareBufferANDROID", (void*)GetMemoryAndroidHardwareBufferANDROID},
#endif
    {"vkCmdSetSampleLocationsEXT", (void*)CmdSetSampleLocationsEXT},
    {"vkGetPhysicalDeviceMultisamplePropertiesEXT", (void*)GetPhysicalDeviceMultisamplePropertiesEXT},
    {"vkCreateValidationCacheEXT", (void*)CreateValidationCacheEXT},
    {"vkDestroyValidationCacheEXT", (void*)DestroyValidationCacheEXT},
    {"vkMergeValidationCachesEXT", (void*)MergeValidationCachesEXT},
    {"vkGetValidationCacheDataEXT", (void*)GetValidationCacheDataEXT},
    {"vkCmdBindShadingRateImageNV", (void*)CmdBindShadingRateImageNV},
    {"vkCmdSetViewportShadingRatePaletteNV", (void*)CmdSetViewportShadingRatePaletteNV},
    {"vkCmdSetCoarseSampleOrderNV", (void*)CmdSetCoarseSampleOrderNV},
    {"vkCreateAccelerationStructureNVX", (void*)CreateAccelerationStructureNVX},
    {"vkDestroyAccelerationStructureNVX", (void*)DestroyAccelerationStructureNVX},
    {"vkGetAccelerationStructureMemoryRequirementsNVX", (void*)GetAccelerationStructureMemoryRequirementsNVX},
    {"vkGetAccelerationStructureScratchMemoryRequirementsNVX", (void*)GetAccelerationStructureScratchMemoryRequirementsNVX},
    {"vkBindAccelerationStructureMemoryNVX", (void*)BindAccelerationStructureMemoryNVX},
    {"vkCmdBuildAccelerationStructureNVX", (void*)CmdBuildAccelerationStructureNVX},
    {"vkCmdCopyAccelerationStructureNVX", (void*)CmdCopyAccelerationStructureNVX},
    {"vkCmdTraceRaysNVX", (void*)CmdTraceRaysNVX},
    {"vkCreateRaytracingPipelinesNVX", (void*)CreateRaytracingPipelinesNVX},
    {"vkGetRaytracingShaderHandlesNVX", (void*)GetRaytracingShaderHandlesNVX},
    {"vkGetAccelerationStructureHandleNVX", (void*)GetAccelerationStructureHandleNVX},
    {"vkCmdWriteAccelerationStructurePropertiesNVX", (void*)CmdWriteAccelerationStructurePropertiesNVX},
    {"vkCompileDeferredNVX", (void*)CompileDeferredNVX},
    {"vkGetMemoryHostPointerPropertiesEXT", (void*)GetMemoryHostPointerPropertiesEXT},
    {"vkCmdWriteBufferMarkerAMD", (void*)CmdWriteBufferMarkerAMD},
    {"vkCmdDrawMeshTasksNV", (void*)CmdDrawMeshTasksNV},
    {"vkCmdDrawMeshTasksIndirectNV", (void*)CmdDrawMeshTasksIndirectNV},
    {"vkCmdDrawMeshTasksIndirectCountNV", (void*)CmdDrawMeshTasksIndirectCountNV},
    {"vkCmdSetExclusiveScissorNV", (void*)CmdSetExclusiveScissorNV},
    {"vkCmdSetCheckpointNV", (void*)CmdSetCheckpointNV},
    {"vkGetQueueCheckpointDataNV", (void*)GetQueueCheckpointDataNV},
#ifdef VK_USE_PLATFORM_FUCHSIA
    {"vkCreateImagePipeSurfaceFUCHSIA", (void*)CreateImagePipeSurfaceFUCHSIA},
#endif
};


} // namespace vulkan_uber_layer

// loader-layer interface v0, just wrappers since there is only a layer

VK_LAYER_EXPORT VKAPI_ATTR VkResult VKAPI_CALL vkEnumerateInstanceExtensionProperties(const char *pLayerName, uint32_t *pCount,
                                                                                      VkExtensionProperties *pProperties) {
    return vulkan_uber_layer::EnumerateInstanceExtensionProperties(pLayerName, pCount, pProperties);
}

VK_LAYER_EXPORT VKAPI_ATTR VkResult VKAPI_CALL vkEnumerateInstanceLayerProperties(uint32_t *pCount,
                                                                                  VkLayerProperties *pProperties) {
    return vulkan_uber_layer::EnumerateInstanceLayerProperties(pCount, pProperties);
}

VK_LAYER_EXPORT VKAPI_ATTR VkResult VKAPI_CALL vkEnumerateDeviceLayerProperties(VkPhysicalDevice physicalDevice, uint32_t *pCount,
                                                                                VkLayerProperties *pProperties) {
    // the layer command handles VK_NULL_HANDLE just fine internally
    assert(physicalDevice == VK_NULL_HANDLE);
    return vulkan_uber_layer::EnumerateDeviceLayerProperties(VK_NULL_HANDLE, pCount, pProperties);
}

VK_LAYER_EXPORT VKAPI_ATTR VkResult VKAPI_CALL vkEnumerateDeviceExtensionProperties(VkPhysicalDevice physicalDevice,
                                                                                    const char *pLayerName, uint32_t *pCount,
                                                                                    VkExtensionProperties *pProperties) {
    // the layer command handles VK_NULL_HANDLE just fine internally
    assert(physicalDevice == VK_NULL_HANDLE);
    return vulkan_uber_layer::EnumerateDeviceExtensionProperties(VK_NULL_HANDLE, pLayerName, pCount, pProperties);
}

VK_LAYER_EXPORT VKAPI_ATTR PFN_vkVoidFunction VKAPI_CALL vkGetDeviceProcAddr(VkDevice dev, const char *funcName) {
    return vulkan_uber_layer::GetDeviceProcAddr(dev, funcName);
}

VK_LAYER_EXPORT VKAPI_ATTR PFN_vkVoidFunction VKAPI_CALL vkGetInstanceProcAddr(VkInstance instance, const char *funcName) {
    return vulkan_uber_layer::GetInstanceProcAddr(instance, funcName);
}

VK_LAYER_EXPORT VKAPI_ATTR PFN_vkVoidFunction VKAPI_CALL vk_layerGetPhysicalDeviceProcAddr(VkInstance instance,
                                                                                           const char *funcName) {
    return vulkan_uber_layer::GetPhysicalDeviceProcAddr(instance, funcName);
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

    return VK_SUCCESS;
}
