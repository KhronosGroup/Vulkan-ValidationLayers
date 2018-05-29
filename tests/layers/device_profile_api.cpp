/*
 *
 * Copyright (C) 2017-2018 Valve Corporation
 * Copyright (C) 2017-2018 LunarG, Inc.
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
 * Author: Arda Coskunses <arda@lunarg.com>
 * Author: Tony Barbour <tony@LunarG.com>
 * Author: Mark Lobodzinski <mark@lunarg.com>
 * Author: William Henning <whenning@google.com>
 */
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include <unordered_map>
#include <mutex>

#define VALIDATION_ERROR_MAP_IMPL

#include "vk_layer_data.h"
#include "vk_dispatch_table_helper.h"
#include "vk_layer_utils.h"
#include "device_profile_api.h"

namespace device_profile_api {

static std::mutex global_lock;

struct FormatHash {
    size_t operator()(const std::pair<VkFormat, VkImageTiling> key) const {
        return std::hash<int>{}(key.first) ^ std::hash<int>{}(key.second);
    }
};

struct PhysicalDeviceData {
    VkInstance instance;
    VkPhysicalDeviceProperties phy_device_props;
    std::unordered_map<VkFormat, VkFormatProperties, std::hash<int> > format_properties_map;
    std::unordered_map<std::pair<VkFormat, VkImageTiling>, std::pair<VkResult, VkImageFormatProperties>, FormatHash>
        image_format_properties_map;
};

struct DeviceData {
    std::unordered_map<VkImage, VkMemoryRequirements> memory_requirements_map;
};

struct InstanceLayerData {
    VkInstance instance;
    VkLayerInstanceDispatchTable dispatch_table;
};

struct DeviceLayerData {
    VkLayerDispatchTable dispatch_table;
};

static uint32_t loader_layer_if_version = CURRENT_LOADER_LAYER_INTERFACE_VERSION;

static std::unordered_map<VkPhysicalDevice, struct PhysicalDeviceData> physical_device_data;
static std::unordered_map<VkDevice, struct DeviceData> device_data;

static std::unordered_map<void *, InstanceLayerData *> instance_layer_data_map;
static std::unordered_map<void *, DeviceLayerData *> device_layer_data_map;

VKAPI_ATTR void VKAPI_CALL GetOriginalPhysicalDeviceLimitsEXT(VkPhysicalDevice physicalDevice, VkPhysicalDeviceLimits *orgLimits) {
    std::lock_guard<std::mutex> lock(global_lock);
    InstanceLayerData *instance_data =
        GetLayerDataPtr(get_dispatch_key(physical_device_data.at(physicalDevice).instance), instance_layer_data_map);

    VkPhysicalDeviceProperties props;
    instance_data->dispatch_table.GetPhysicalDeviceProperties(physicalDevice, &props);
    *orgLimits = props.limits;
}

VKAPI_ATTR void VKAPI_CALL SetPhysicalDeviceLimitsEXT(VkPhysicalDevice physicalDevice, const VkPhysicalDeviceLimits *newLimits) {
    std::lock_guard<std::mutex> lock(global_lock);
    physical_device_data.at(physicalDevice).phy_device_props.limits = *newLimits;
}

VKAPI_ATTR void VKAPI_CALL GetOriginalPhysicalDeviceFormatPropertiesEXT(VkPhysicalDevice physicalDevice, VkFormat format,
                                                                        VkFormatProperties *properties) {
    std::lock_guard<std::mutex> lock(global_lock);
    InstanceLayerData *instance_data =
        GetLayerDataPtr(get_dispatch_key(physical_device_data.at(physicalDevice).instance), instance_layer_data_map);
    instance_data->dispatch_table.GetPhysicalDeviceFormatProperties(physicalDevice, format, properties);
}

VKAPI_ATTR void VKAPI_CALL SetPhysicalDeviceFormatPropertiesEXT(VkPhysicalDevice physicalDevice, VkFormat format,
                                                                const VkFormatProperties newProperties) {
    std::lock_guard<std::mutex> lock(global_lock);
    physical_device_data.at(physicalDevice).format_properties_map[format] = newProperties;
}

VKAPI_ATTR VkResult VKAPI_CALL GetOriginalPhysicalDeviceImageFormatPropertiesEXT(VkPhysicalDevice physicalDevice, VkFormat format,
                                                                                 VkImageType type, VkImageTiling tiling,
                                                                                 VkImageUsageFlags usage, VkImageCreateFlags flags,
                                                                                 VkImageFormatProperties *pImageFormatProperties) {
    std::lock_guard<std::mutex> lock(global_lock);
    InstanceLayerData *instance_data =
        GetLayerDataPtr(get_dispatch_key(physical_device_data.at(physicalDevice).instance), instance_layer_data_map);
    return instance_data->dispatch_table.GetPhysicalDeviceImageFormatProperties(physicalDevice, format, type, tiling, usage, flags,
                                                                                 pImageFormatProperties);
}

VKAPI_ATTR void VKAPI_CALL SetPhysicalDeviceImageFormatPropertiesEXT(VkPhysicalDevice physicalDevice, VkFormat format,
                                                                     VkImageTiling tiling, const VkResult new_result,
                                                                     const VkImageFormatProperties new_properties) {
    std::lock_guard<std::mutex> lock(global_lock);
    physical_device_data.at(physicalDevice).image_format_properties_map[{format, tiling}] = {new_result, new_properties};
}

VKAPI_ATTR void VKAPI_CALL GetOriginalImageMemoryRequirementsEXT(VkDevice device, VkImage image,
                                                                 VkMemoryRequirements *pMemoryRequirements) {
    std::lock_guard<std::mutex> lock(global_lock);
    DeviceLayerData *device_layer_data = GetLayerDataPtr(get_dispatch_key(device), device_layer_data_map);
    device_layer_data->dispatch_table.GetImageMemoryRequirements(device, image, pMemoryRequirements);
}

VKAPI_ATTR void VKAPI_CALL SetImageMemoryRequirementsEXT(VkDevice device, VkImage image,
                                                         const VkMemoryRequirements new_requirements) {
    std::lock_guard<std::mutex> lock(global_lock);
    device_data.at(device).memory_requirements_map[image] = new_requirements;
}

VKAPI_ATTR VkResult VKAPI_CALL CreateInstance(const VkInstanceCreateInfo *pCreateInfo, const VkAllocationCallbacks *pAllocator,
                                              VkInstance *pInstance) {
    VkLayerInstanceCreateInfo *chain_info = get_chain_info(pCreateInfo, VK_LAYER_LINK_INFO);
    std::lock_guard<std::mutex> lock(global_lock);

    assert(chain_info->u.pLayerInfo);
    PFN_vkGetInstanceProcAddr fp_get_instance_proc_addr = chain_info->u.pLayerInfo->pfnNextGetInstanceProcAddr;
    PFN_vkCreateInstance fp_create_instance = (PFN_vkCreateInstance)fp_get_instance_proc_addr(NULL, "vkCreateInstance");
    if (fp_create_instance == NULL) return VK_ERROR_INITIALIZATION_FAILED;

    // Advance the link info for the next element on the chain
    chain_info->u.pLayerInfo = chain_info->u.pLayerInfo->pNext;

    VkResult result = fp_create_instance(pCreateInfo, pAllocator, pInstance);
    if (result != VK_SUCCESS) return result;

    InstanceLayerData *instance_layer_data = GetLayerDataPtr(get_dispatch_key(*pInstance), instance_layer_data_map);
    instance_layer_data->instance = *pInstance;
    layer_init_instance_dispatch_table(*pInstance, &instance_layer_data->dispatch_table, fp_get_instance_proc_addr);

    uint32_t physical_device_count = 0;
    instance_layer_data->dispatch_table.EnumeratePhysicalDevices(*pInstance, &physical_device_count, NULL);

    VkPhysicalDevice *physical_devices = (VkPhysicalDevice *)malloc(sizeof(physical_devices[0]) * physical_device_count);
    result = instance_layer_data->dispatch_table.EnumeratePhysicalDevices(*pInstance, &physical_device_count, physical_devices);

    for (uint8_t i = 0; i < physical_device_count; i++) {
        instance_layer_data->dispatch_table.GetPhysicalDeviceProperties(
            physical_devices[i], &physical_device_data[physical_devices[i]].phy_device_props);
        physical_device_data[physical_devices[i]].instance = *pInstance;
    }
    return result;
}

VKAPI_ATTR VkResult CreateDevice(VkPhysicalDevice physicalDevice, const VkDeviceCreateInfo *pCreateInfo,
                                 const VkAllocationCallbacks *pAllocator, VkDevice *pDevice) {
    InstanceLayerData *instance_layer_data =
        GetLayerDataPtr(get_dispatch_key(physical_device_data[physicalDevice].instance), instance_layer_data_map);

    std::unique_lock<std::mutex> lock(global_lock);

    VkLayerDeviceCreateInfo *chain_info = get_chain_info(pCreateInfo, VK_LAYER_LINK_INFO);

    assert(chain_info->u.pLayerInfo);
    PFN_vkGetInstanceProcAddr fpGetInstanceProcAddr = chain_info->u.pLayerInfo->pfnNextGetInstanceProcAddr;
    PFN_vkGetDeviceProcAddr fpGetDeviceProcAddr = chain_info->u.pLayerInfo->pfnNextGetDeviceProcAddr;
    PFN_vkCreateDevice fpCreateDevice = (PFN_vkCreateDevice)fpGetInstanceProcAddr(instance_layer_data->instance, "vkCreateDevice");
    if (fpCreateDevice == NULL) {
        return VK_ERROR_INITIALIZATION_FAILED;
    }

    // Advance the link info for the next element on the chain
    chain_info->u.pLayerInfo = chain_info->u.pLayerInfo->pNext;

    lock.unlock();

    VkResult result = fpCreateDevice(physicalDevice, pCreateInfo, pAllocator, pDevice);
    if (result != VK_SUCCESS) {
        return result;
    }

    lock.lock();

    DeviceLayerData *device_layer_data = GetLayerDataPtr(get_dispatch_key(*pDevice), device_layer_data_map);
    layer_init_device_dispatch_table(*pDevice, &device_layer_data->dispatch_table, fpGetDeviceProcAddr);

    device_data[*pDevice] = {};

    lock.unlock();
    return result;
}

VKAPI_ATTR void VKAPI_CALL GetPhysicalDeviceProperties(VkPhysicalDevice physicalDevice, VkPhysicalDeviceProperties *pProperties) {
    std::lock_guard<std::mutex> lock(global_lock);
    *pProperties = physical_device_data.at(physicalDevice).phy_device_props;
}

VKAPI_ATTR void VKAPI_CALL GetPhysicalDeviceFormatProperties(VkPhysicalDevice physicalDevice, VkFormat format,
                                                             VkFormatProperties *pProperties) {
    std::lock_guard<std::mutex> lock(global_lock);

    // If there are mock device format properties return them.
    if (physical_device_data.find(physicalDevice) != physical_device_data.end()) {
        const auto &format_properties_map = physical_device_data[physicalDevice].format_properties_map;
        if (format_properties_map.find(format) != format_properties_map.end()) {
            *pProperties = format_properties_map.at(format);
            return;
        }
    }

    // If not, pass the call to GetPhysicalDeviceFormatProperties down the dispatch chain.
    InstanceLayerData *device_profile_data =
        GetLayerDataPtr(get_dispatch_key(physical_device_data[physicalDevice].instance), instance_layer_data_map);
    device_profile_data->dispatch_table.GetPhysicalDeviceFormatProperties(physicalDevice, format, pProperties);
}

VKAPI_ATTR VkResult VKAPI_CALL GetPhysicalDeviceImageFormatProperties(VkPhysicalDevice physicalDevice, VkFormat format,
                                                                      VkImageType type, VkImageTiling tiling,
                                                                      VkImageUsageFlags usage, VkImageCreateFlags flags,
                                                                      VkImageFormatProperties *pImageFormatProperties) {
    std::lock_guard<std::mutex> lock(global_lock);

    // If there are mock device image format properties return them.
    if (physical_device_data.find(physicalDevice) != physical_device_data.end()) {
        const auto &image_format_properties_map = physical_device_data[physicalDevice].image_format_properties_map;
        if (image_format_properties_map.find({format, tiling}) != image_format_properties_map.end()) {
            const auto &image_format_support = image_format_properties_map.at({format, tiling});
            *pImageFormatProperties = image_format_support.second;
            return image_format_support.first;
        }
    }

    // If not, pass the call to GetPhysicalDeviceImageFormatProperties down the dispatch chain.
    InstanceLayerData *device_profile_data =
        GetLayerDataPtr(get_dispatch_key(physical_device_data[physicalDevice].instance), instance_layer_data_map);
    return device_profile_data->dispatch_table.GetPhysicalDeviceImageFormatProperties(
        physicalDevice, format, type, tiling, usage, flags, pImageFormatProperties);
}

VKAPI_ATTR void VKAPI_CALL GetImageMemoryRequirements(VkDevice device, VkImage image, VkMemoryRequirements *pMemoryRequirements) {
    std::lock_guard<std::mutex> lock(global_lock);

    // If there are mock image memory requirements return them.
    const auto &memory_requirements_map = device_data.at(device).memory_requirements_map;
    if (memory_requirements_map.find(image) != memory_requirements_map.end()) {
        *pMemoryRequirements = memory_requirements_map.at(image);
        return;
    }

    // If not, pass the call to GetImageMemoryRequirements down the dispatch chain.
    DeviceLayerData *device_layer_data = GetLayerDataPtr(get_dispatch_key(device), device_layer_data_map);
    device_layer_data->dispatch_table.GetImageMemoryRequirements(device, image, pMemoryRequirements);
}

static const VkLayerProperties device_profile_api_LayerProps = {
    "VK_LAYER_LUNARG_device_profile_api",
    VK_MAKE_VERSION(1, 0, VK_HEADER_VERSION),  // specVersion
    1,                                         // implementationVersion
    "LunarG device profile api Layer",
};

template <typename T>
VkResult EnumerateProperties(uint32_t src_count, const T *src_props, uint32_t *dst_count, T *dst_props) {
    if (!dst_props || !src_props) {
        *dst_count = src_count;
        return VK_SUCCESS;
    }

    uint32_t copy_count = (*dst_count < src_count) ? *dst_count : src_count;
    memcpy(dst_props, src_props, sizeof(T) * copy_count);
    *dst_count = copy_count;

    return (copy_count == src_count) ? VK_SUCCESS : VK_INCOMPLETE;
}

VKAPI_ATTR VkResult VKAPI_CALL EnumerateInstanceLayerProperties(uint32_t *pCount, VkLayerProperties *pProperties) {
    return EnumerateProperties(1, &device_profile_api_LayerProps, pCount, pProperties);
}

VKAPI_ATTR VkResult VKAPI_CALL EnumerateInstanceExtensionProperties(const char *pLayerName, uint32_t *pCount,
                                                                    VkExtensionProperties *pProperties) {
    if (pLayerName && !strcmp(pLayerName, device_profile_api_LayerProps.layerName))
        return EnumerateProperties<VkExtensionProperties>(0, NULL, pCount, pProperties);

    return VK_ERROR_LAYER_NOT_PRESENT;
}

VKAPI_ATTR PFN_vkVoidFunction VKAPI_CALL GetPhysicalDeviceProcAddr(VkInstance instance, const char *name) {
    if (!strcmp(name, "vkSetPhysicalDeviceLimitsEXT")) return (PFN_vkVoidFunction)SetPhysicalDeviceLimitsEXT;
    if (!strcmp(name, "vkGetOriginalPhysicalDeviceLimitsEXT")) return (PFN_vkVoidFunction)GetOriginalPhysicalDeviceLimitsEXT;
    if (!strcmp(name, "vkSetPhysicalDeviceFormatPropertiesEXT")) return (PFN_vkVoidFunction)SetPhysicalDeviceFormatPropertiesEXT;
    if (!strcmp(name, "vkGetOriginalPhysicalDeviceFormatPropertiesEXT"))
        return (PFN_vkVoidFunction)GetOriginalPhysicalDeviceFormatPropertiesEXT;
    if (!strcmp(name, "vkSetPhysicalDeviceImageFormatPropertiesEXT"))
        return (PFN_vkVoidFunction)SetPhysicalDeviceImageFormatPropertiesEXT;
    if (!strcmp(name, "vkGetOriginalPhysicalDeviceImageFormatPropertiesEXT"))
        return (PFN_vkVoidFunction)GetOriginalPhysicalDeviceImageFormatPropertiesEXT;

    InstanceLayerData *instance_layer_data = GetLayerDataPtr(get_dispatch_key(instance), instance_layer_data_map);
    return instance_layer_data->dispatch_table.GetPhysicalDeviceProcAddr(instance, name);
}

VKAPI_ATTR PFN_vkVoidFunction VKAPI_CALL GetDeviceProcAddr(VkDevice device, const char *name) {
    if (!strcmp(name, "vkGetImageMemoryRequirements")) return (PFN_vkVoidFunction)GetImageMemoryRequirements;
    if (!strcmp(name, "vkSetImageMemoryRequirementsEXT")) return (PFN_vkVoidFunction)SetImageMemoryRequirementsEXT;
    if (!strcmp(name, "vkGetOriginalImageMemoryRequirementsEXT")) return (PFN_vkVoidFunction)GetOriginalImageMemoryRequirementsEXT;

    DeviceLayerData *device_layer_data = GetLayerDataPtr(get_dispatch_key(device), device_layer_data_map);
    return device_layer_data->dispatch_table.GetDeviceProcAddr(device, name);
}

VKAPI_ATTR PFN_vkVoidFunction VKAPI_CALL GetInstanceProcAddr(VkInstance instance, const char *name) {
    if (!strcmp(name, "vkCreateInstance")) return (PFN_vkVoidFunction)CreateInstance;
    if (!strcmp(name, "vkCreateDevice")) return (PFN_vkVoidFunction)CreateDevice;
    if (!strcmp(name, "vkGetPhysicalDeviceProperties")) return (PFN_vkVoidFunction)GetPhysicalDeviceProperties;
    if (!strcmp(name, "vkGetPhysicalDeviceFormatProperties")) return (PFN_vkVoidFunction)GetPhysicalDeviceFormatProperties;
    if (!strcmp(name, "vkGetPhysicalDeviceImageFormatProperties"))
        return (PFN_vkVoidFunction)GetPhysicalDeviceImageFormatProperties;
    if (!strcmp(name, "vkGetImageMemoryRequirements")) return (PFN_vkVoidFunction)GetImageMemoryRequirements;
    if (!strcmp(name, "vkGetInstanceProcAddr")) return (PFN_vkVoidFunction)GetInstanceProcAddr;
    if (!strcmp(name, "vkEnumerateInstanceExtensionProperties")) return (PFN_vkVoidFunction)EnumerateInstanceExtensionProperties;
    if (!strcmp(name, "vkEnumerateInstanceLayerProperties")) return (PFN_vkVoidFunction)EnumerateInstanceLayerProperties;
    if (!strcmp(name, "vkSetPhysicalDeviceLimitsEXT")) return (PFN_vkVoidFunction)SetPhysicalDeviceLimitsEXT;
    if (!strcmp(name, "vkGetOriginalPhysicalDeviceLimitsEXT")) return (PFN_vkVoidFunction)GetOriginalPhysicalDeviceLimitsEXT;
    if (!strcmp(name, "vkSetPhysicalDeviceFormatPropertiesEXT")) return (PFN_vkVoidFunction)SetPhysicalDeviceFormatPropertiesEXT;
    if (!strcmp(name, "vkGetOriginalPhysicalDeviceFormatPropertiesEXT"))
        return (PFN_vkVoidFunction)GetOriginalPhysicalDeviceFormatPropertiesEXT;
    if (!strcmp(name, "vkSetPhysicalDeviceImageFormatPropertiesEXT"))
        return (PFN_vkVoidFunction)SetPhysicalDeviceImageFormatPropertiesEXT;
    if (!strcmp(name, "vkGetOriginalPhysicalDeviceImageFormatPropertiesEXT"))
        return (PFN_vkVoidFunction)GetOriginalPhysicalDeviceImageFormatPropertiesEXT;

    InstanceLayerData *instance_layer_data = GetLayerDataPtr(get_dispatch_key(instance), instance_layer_data_map);
    return instance_layer_data->dispatch_table.GetInstanceProcAddr(instance, name);
}

}  // namespace device_profile_api

VK_LAYER_EXPORT VKAPI_ATTR VkResult VKAPI_CALL vkEnumerateInstanceLayerProperties(uint32_t *pCount,
                                                                                  VkLayerProperties *pProperties) {
    return device_profile_api::EnumerateInstanceLayerProperties(pCount, pProperties);
}

VK_LAYER_EXPORT VKAPI_ATTR VkResult VKAPI_CALL vkEnumerateInstanceExtensionProperties(const char *pLayerName, uint32_t *pCount,
                                                                                      VkExtensionProperties *pProperties) {
    return device_profile_api::EnumerateInstanceExtensionProperties(pLayerName, pCount, pProperties);
}

VK_LAYER_EXPORT VKAPI_ATTR PFN_vkVoidFunction VKAPI_CALL vkGetInstanceProcAddr(VkInstance instance, const char *funcName) {
    return device_profile_api::GetInstanceProcAddr(instance, funcName);
}

VK_LAYER_EXPORT VKAPI_ATTR PFN_vkVoidFunction VKAPI_CALL vkGetDeviceProcAddr(VkDevice device, const char *funcName) {
    return device_profile_api::GetDeviceProcAddr(device, funcName);
}

VK_LAYER_EXPORT VKAPI_ATTR PFN_vkVoidFunction VKAPI_CALL vk_layerGetPhysicalDeviceProcAddr(VkInstance instance,
                                                                                           const char *funcName) {
    return device_profile_api::GetPhysicalDeviceProcAddr(instance, funcName);
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
        device_profile_api::loader_layer_if_version = pVersionStruct->loaderLayerInterfaceVersion;
    } else if (pVersionStruct->loaderLayerInterfaceVersion > CURRENT_LOADER_LAYER_INTERFACE_VERSION) {
        pVersionStruct->loaderLayerInterfaceVersion = CURRENT_LOADER_LAYER_INTERFACE_VERSION;
    }

    return VK_SUCCESS;
}
