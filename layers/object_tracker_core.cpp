/* Copyright (c) 2015-2018 The Khronos Group Inc.
 * Copyright (c) 2015-2018 Valve Corporation
 * Copyright (c) 2015-2018 LunarG, Inc.
 * Copyright (C) 2015-2018 Google Inc.
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
 * Author: Jon Ashburn <jon@lunarg.com>
 * Author: Tobin Ehlis <tobin@lunarg.com>
 */

#define VALIDATION_ERROR_MAP_IMPL

#include "object_tracker.h"
#include "object_lifetime_validation.h"


namespace object_tracker {

#include "precall.h"
#include "postcall.h"

std::unordered_map<void *, layer_data *> layer_data_map;
std::unordered_map<void *, instance_layer_data *> instance_layer_data_map;
std::mutex global_lock;
uint32_t loader_layer_if_version = CURRENT_LOADER_LAYER_INTERFACE_VERSION;

void InitObjectTracker(instance_layer_data *my_data, const VkAllocationCallbacks *pAllocator) {
    layer_debug_report_actions(my_data->report_data, my_data->logging_callback, pAllocator, "lunarg_object_tracker");
    layer_debug_messenger_actions(my_data->report_data, my_data->logging_messenger, pAllocator, "lunarg_object_tracker");
}


VKAPI_ATTR void VKAPI_CALL CmdPushDescriptorSetKHR(VkCommandBuffer commandBuffer, VkPipelineBindPoint pipelineBindPoint,
                                                   VkPipelineLayout layout, uint32_t set, uint32_t descriptorWriteCount,
                                                   const VkWriteDescriptorSet *pDescriptorWrites) {
    bool skip = false;
    {
        std::lock_guard<std::mutex> lock(global_lock);
        skip |= PreCallValidateCmdPushDescriptorSetKHR(commandBuffer, pipelineBindPoint, layout, set, descriptorWriteCount,
                                                       pDescriptorWrites);
        if (skip) return;
    }
    layer_data *device_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    device_data->device_dispatch_table.CmdPushDescriptorSetKHR(commandBuffer, pipelineBindPoint, layout, set, descriptorWriteCount,
                                                               pDescriptorWrites);
}

VKAPI_ATTR void VKAPI_CALL DestroyInstance(VkInstance instance, const VkAllocationCallbacks *pAllocator) {
    dispatch_key key = get_dispatch_key(instance);
    instance_layer_data *instance_data = GetLayerDataPtr(key, instance_layer_data_map);
    {
        std::lock_guard<std::mutex> lock(global_lock);
        bool skip = PreCallValidateDestroyInstance(instance, pAllocator);
        if (skip) return;
        PreCallRecordDestroyInstance(instance, pAllocator);
    }
    instance_data->instance_dispatch_table.DestroyInstance(instance, pAllocator);
    {
        std::lock_guard<std::mutex> lock(global_lock);
        PostCallRecordDestroyInstance(instance, pAllocator);
    }
}

VKAPI_ATTR void VKAPI_CALL DestroyDevice(VkDevice device, const VkAllocationCallbacks *pAllocator) {
    layer_data *device_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    {
        bool skip = false;
        std::lock_guard<std::mutex> lock(global_lock);
        skip = PreCallValidateDestroyDevice(device, pAllocator);
        // Skipping down-chain destroydevice calls will hose any upstream validation layers
        PreCallRecordDestroyDevice(device, pAllocator);
    }
    dispatch_key key = get_dispatch_key(device);
    device_data->device_dispatch_table.DestroyDevice(device, pAllocator);
    FreeLayerDataPtr(key, layer_data_map);
}

VKAPI_ATTR void VKAPI_CALL GetDeviceQueue(VkDevice device, uint32_t queueFamilyIndex, uint32_t queueIndex, VkQueue *pQueue) {
    {
        std::lock_guard<std::mutex> lock(global_lock);
        bool skip = PreCallValidateGetDeviceQueue(device, queueFamilyIndex, queueIndex, pQueue);
        if (skip) return;
    }
    layer_data *device_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    device_data->device_dispatch_table.GetDeviceQueue(device, queueFamilyIndex, queueIndex, pQueue);
    {
        if (*pQueue != VK_NULL_HANDLE) {
            std::lock_guard<std::mutex> lock(global_lock);
            PostCallRecordGetDeviceQueue(device, queueFamilyIndex, queueIndex, pQueue);
        }
    }
}

VKAPI_ATTR void VKAPI_CALL GetDeviceQueue2(VkDevice device, const VkDeviceQueueInfo2 *pQueueInfo, VkQueue *pQueue) {
    {
        std::lock_guard<std::mutex> lock(global_lock);
        bool skip = PreCallValidateGetDeviceQueue2(device, pQueueInfo, pQueue);
        if (skip) return;
    }
    layer_data *device_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    device_data->device_dispatch_table.GetDeviceQueue2(device, pQueueInfo, pQueue);
    {
        if (*pQueue != VK_NULL_HANDLE) {
            std::lock_guard<std::mutex> lock(global_lock);
            PostCallRecordGetDeviceQueue2(device, pQueueInfo, pQueue);
        }
    }
}

VKAPI_ATTR void VKAPI_CALL UpdateDescriptorSets(VkDevice device, uint32_t descriptorWriteCount,
                                                const VkWriteDescriptorSet *pDescriptorWrites, uint32_t descriptorCopyCount,
                                                const VkCopyDescriptorSet *pDescriptorCopies) {
    {
        std::lock_guard<std::mutex> lock(global_lock);
        bool skip = PreCallValidateUpdateDescriptorSets(device, descriptorWriteCount, pDescriptorWrites, descriptorCopyCount,
                                                        pDescriptorCopies);
        if (skip) return;
    }
    layer_data *device_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    device_data->device_dispatch_table.UpdateDescriptorSets(device, descriptorWriteCount, pDescriptorWrites, descriptorCopyCount,
                                                            pDescriptorCopies);
}

VKAPI_ATTR VkResult VKAPI_CALL CreateComputePipelines(VkDevice device, VkPipelineCache pipelineCache, uint32_t createInfoCount,
                                                      const VkComputePipelineCreateInfo *pCreateInfos,
                                                      const VkAllocationCallbacks *pAllocator, VkPipeline *pPipelines) {
    {
        std::lock_guard<std::mutex> lock(global_lock);
        bool skip =
            PreCallValidateCreateComputePipelines(device, pipelineCache, createInfoCount, pCreateInfos, pAllocator, pPipelines);
        if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
    }
    layer_data *device_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    VkResult result = device_data->device_dispatch_table.CreateComputePipelines(device, pipelineCache, createInfoCount,
                                                                                pCreateInfos, pAllocator, pPipelines);
    {
        std::lock_guard<std::mutex> lock(global_lock);
        PostCallRecordCreateComputePipelines(device, pipelineCache, createInfoCount, pCreateInfos, pAllocator, pPipelines);
    }
    return result;
}

VKAPI_ATTR VkResult VKAPI_CALL ResetDescriptorPool(VkDevice device, VkDescriptorPool descriptorPool,
                                                   VkDescriptorPoolResetFlags flags) {
    {
        std::lock_guard<std::mutex> lock(global_lock);
        bool skip = PreCallValidateResetDescriptorPool(device, descriptorPool, flags);
        if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
        PreCallRecordResetDescriptorPool(device, descriptorPool, flags);
    }
    layer_data *device_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    VkResult result = device_data->device_dispatch_table.ResetDescriptorPool(device, descriptorPool, flags);
    return result;
}

VKAPI_ATTR VkResult VKAPI_CALL BeginCommandBuffer(VkCommandBuffer command_buffer, const VkCommandBufferBeginInfo *begin_info) {
    {
        std::lock_guard<std::mutex> lock(global_lock);
        bool skip = PreCallValidateBeginCommandBuffer(command_buffer, begin_info);
        if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
    }
    layer_data *device_data = GetLayerDataPtr(get_dispatch_key(command_buffer), layer_data_map);
    VkResult result = device_data->device_dispatch_table.BeginCommandBuffer(command_buffer, begin_info);
    return result;
}

VKAPI_ATTR VkResult VKAPI_CALL CreateDebugReportCallbackEXT(VkInstance instance,
                                                            const VkDebugReportCallbackCreateInfoEXT *pCreateInfo,
                                                            const VkAllocationCallbacks *pAllocator,
                                                            VkDebugReportCallbackEXT *pCallback) {
    auto instance_data = GetLayerDataPtr(get_dispatch_key(instance), instance_layer_data_map);
    VkResult result =
        instance_data->instance_dispatch_table.CreateDebugReportCallbackEXT(instance, pCreateInfo, pAllocator, pCallback);
    if (VK_SUCCESS == result) {
        result = layer_create_report_callback(instance_data->report_data, false, pCreateInfo, pAllocator, pCallback);
        PostCallRecordCreateDebugReportCallbackEXT(instance, pCreateInfo, pAllocator, pCallback);
    }
    return result;
}

VKAPI_ATTR void VKAPI_CALL DestroyDebugReportCallbackEXT(VkInstance instance, VkDebugReportCallbackEXT msgCallback,
                                                         const VkAllocationCallbacks *pAllocator) {
    {
        std::lock_guard<std::mutex> lock(global_lock);
        bool skip = PreCallValidateDestroyDebugReportCallbackEXT(instance, msgCallback, pAllocator);
        if (skip) return;
        PreCallRecordDestroyDebugReportCallbackEXT(instance, msgCallback, pAllocator);
    }
    auto instance_data = GetLayerDataPtr(get_dispatch_key(instance), instance_layer_data_map);
    instance_data->instance_dispatch_table.DestroyDebugReportCallbackEXT(instance, msgCallback, pAllocator);
    layer_destroy_report_callback(instance_data->report_data, msgCallback, pAllocator);
}

VKAPI_ATTR void VKAPI_CALL DebugReportMessageEXT(VkInstance instance, VkDebugReportFlagsEXT flags,
                                                 VkDebugReportObjectTypeEXT objType, uint64_t object, size_t location,
                                                 int32_t msgCode, const char *pLayerPrefix, const char *pMsg) {
    auto instance_data = GetLayerDataPtr(get_dispatch_key(instance), instance_layer_data_map);
    instance_data->instance_dispatch_table.DebugReportMessageEXT(instance, flags, objType, object, location, msgCode, pLayerPrefix,
                                                                 pMsg);
}

// VK_EXT_debug_utils commands

VKAPI_ATTR VkResult VKAPI_CALL SetDebugUtilsObjectNameEXT(VkDevice device, const VkDebugUtilsObjectNameInfoEXT *pNameInfo) {
    {
        std::lock_guard<std::mutex> lock(global_lock);
        bool skip = PreCallValidateSetDebugUtilsObjectNameEXT(device, pNameInfo);
        if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
    }
    layer_data *dev_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    if (pNameInfo->pObjectName) {
        std::lock_guard<std::mutex> lock(global_lock);
        dev_data->report_data->debugUtilsObjectNameMap->insert(
            std::make_pair<uint64_t, std::string>((uint64_t &&) pNameInfo->objectHandle, pNameInfo->pObjectName));
    } else {
        std::lock_guard<std::mutex> lock(global_lock);
        dev_data->report_data->debugUtilsObjectNameMap->erase(pNameInfo->objectHandle);
    }
    VkResult result = dev_data->device_dispatch_table.SetDebugUtilsObjectNameEXT(device, pNameInfo);
    return result;
}

VKAPI_ATTR VkResult VKAPI_CALL SetDebugUtilsObjectTagEXT(VkDevice device, const VkDebugUtilsObjectTagInfoEXT *pTagInfo) {
    {
        std::lock_guard<std::mutex> lock(global_lock);
        bool skip = PreCallValidateSetDebugUtilsObjectTagEXT(device, pTagInfo);
        if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
    }
    layer_data *dev_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    VkResult result = dev_data->device_dispatch_table.SetDebugUtilsObjectTagEXT(device, pTagInfo);
    return result;
}

VKAPI_ATTR void VKAPI_CALL QueueBeginDebugUtilsLabelEXT(VkQueue queue, const VkDebugUtilsLabelEXT *pLabelInfo) {
    layer_data *dev_data = GetLayerDataPtr(get_dispatch_key(queue), layer_data_map);
    {
        std::lock_guard<std::mutex> lock(global_lock);
        bool skip = PreCallValidateQueueBeginDebugUtilsLabelEXT(queue, pLabelInfo);
        if (skip) return;
        BeginQueueDebugUtilsLabel(dev_data->report_data, queue, pLabelInfo);
    }
    dev_data->device_dispatch_table.QueueBeginDebugUtilsLabelEXT(queue, pLabelInfo);
}

VKAPI_ATTR void VKAPI_CALL QueueEndDebugUtilsLabelEXT(VkQueue queue) {
    {
        std::lock_guard<std::mutex> lock(global_lock);
        bool skip = PreCallValidateQueueEndDebugUtilsLabelEXT(queue);
        if (skip) return;
    }
    layer_data *dev_data = GetLayerDataPtr(get_dispatch_key(queue), layer_data_map);
    dev_data->device_dispatch_table.QueueEndDebugUtilsLabelEXT(queue);
    {
        std::lock_guard<std::mutex> lock(global_lock);
        EndQueueDebugUtilsLabel(dev_data->report_data, queue);
    }
}

VKAPI_ATTR void VKAPI_CALL QueueInsertDebugUtilsLabelEXT(VkQueue queue, const VkDebugUtilsLabelEXT *pLabelInfo) {
    {
        std::lock_guard<std::mutex> lock(global_lock);
        bool skip = PreCallValidateQueueInsertDebugUtilsLabelEXT(queue, pLabelInfo);
        if (skip) return;
    }
    layer_data *dev_data = GetLayerDataPtr(get_dispatch_key(queue), layer_data_map);
    {
        std::lock_guard<std::mutex> lock(global_lock);
        InsertQueueDebugUtilsLabel(dev_data->report_data, queue, pLabelInfo);
    }
    dev_data->device_dispatch_table.QueueInsertDebugUtilsLabelEXT(queue, pLabelInfo);
}

VKAPI_ATTR void VKAPI_CALL CmdBeginDebugUtilsLabelEXT(VkCommandBuffer commandBuffer, const VkDebugUtilsLabelEXT *pLabelInfo) {
    {
        std::lock_guard<std::mutex> lock(global_lock);
        bool skip = PreCallValidateCmdBeginDebugUtilsLabelEXT(commandBuffer, pLabelInfo);
        if (skip) return;
    }
    layer_data *dev_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    {
        std::lock_guard<std::mutex> lock(global_lock);
        BeginCmdDebugUtilsLabel(dev_data->report_data, commandBuffer, pLabelInfo);
    }
    dev_data->device_dispatch_table.CmdBeginDebugUtilsLabelEXT(commandBuffer, pLabelInfo);
}

VKAPI_ATTR void VKAPI_CALL CmdEndDebugUtilsLabelEXT(VkCommandBuffer commandBuffer) {
    {
        std::lock_guard<std::mutex> lock(global_lock);
        bool skip = PreCallValidateCmdEndDebugUtilsLabelEXT(commandBuffer);
        if (skip) return;
    }
    layer_data *dev_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    dev_data->device_dispatch_table.CmdEndDebugUtilsLabelEXT(commandBuffer);
    {
        std::lock_guard<std::mutex> lock(global_lock);
        EndCmdDebugUtilsLabel(dev_data->report_data, commandBuffer);
    }
}

VKAPI_ATTR void VKAPI_CALL CmdInsertDebugUtilsLabelEXT(VkCommandBuffer commandBuffer, const VkDebugUtilsLabelEXT *pLabelInfo) {
    {
        std::lock_guard<std::mutex> lock(global_lock);
        bool skip = PreCallValidateCmdInsertDebugUtilsLabelEXT(commandBuffer, pLabelInfo);
        if (skip) return;
    }
    layer_data *dev_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    {
        std::lock_guard<std::mutex> lock(global_lock);
        InsertCmdDebugUtilsLabel(dev_data->report_data, commandBuffer, pLabelInfo);
    }
    dev_data->device_dispatch_table.CmdInsertDebugUtilsLabelEXT(commandBuffer, pLabelInfo);
}

VKAPI_ATTR VkResult VKAPI_CALL CreateDebugUtilsMessengerEXT(VkInstance instance,
                                                            const VkDebugUtilsMessengerCreateInfoEXT *pCreateInfo,
                                                            const VkAllocationCallbacks *pAllocator,
                                                            VkDebugUtilsMessengerEXT *pMessenger) {
    {
        std::lock_guard<std::mutex> lock(global_lock);
        bool skip = PreCallValidateCreateDebugUtilsMessengerEXT(instance, pCreateInfo, pAllocator, pMessenger);
        if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
    }
    auto instance_data = GetLayerDataPtr(get_dispatch_key(instance), instance_layer_data_map);
    VkResult result =
        instance_data->instance_dispatch_table.CreateDebugUtilsMessengerEXT(instance, pCreateInfo, pAllocator, pMessenger);
    if (VK_SUCCESS == result) {
        result = layer_create_messenger_callback(instance_data->report_data, false, pCreateInfo, pAllocator, pMessenger);
        PostCallRecordCreateDebugUtilsMessengerEXT(instance, pCreateInfo, pAllocator, pMessenger);
    }
    return result;
}

VKAPI_ATTR void VKAPI_CALL DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT messenger,
                                                         const VkAllocationCallbacks *pAllocator) {
    {
        std::lock_guard<std::mutex> lock(global_lock);
        bool skip = PreCallValidateDestroyDebugUtilsMessengerEXT(instance, messenger, pAllocator);
        if (skip) return;
        PreCallRecordDestroyDebugUtilsMessengerEXT(instance, messenger, pAllocator);
    }
    auto instance_data = GetLayerDataPtr(get_dispatch_key(instance), instance_layer_data_map);
    instance_data->instance_dispatch_table.DestroyDebugUtilsMessengerEXT(instance, messenger, pAllocator);
    layer_destroy_messenger_callback(instance_data->report_data, messenger, pAllocator);
}

VKAPI_ATTR void VKAPI_CALL SubmitDebugUtilsMessageEXT(VkInstance instance, VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
                                                      VkDebugUtilsMessageTypeFlagsEXT messageTypes,
                                                      const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData) {
    {
        std::lock_guard<std::mutex> lock(global_lock);
        bool skip = PreCallValidateSubmitDebugUtilsMessageEXT(instance, messageSeverity, messageTypes, pCallbackData);
        if (skip) return;
    }

    auto instance_data = GetLayerDataPtr(get_dispatch_key(instance), instance_layer_data_map);
    instance_data->instance_dispatch_table.SubmitDebugUtilsMessageEXT(instance, messageSeverity, messageTypes, pCallbackData);
}

static const VkExtensionProperties instance_extensions[] = { {VK_EXT_DEBUG_REPORT_EXTENSION_NAME, VK_EXT_DEBUG_REPORT_SPEC_VERSION},
                                                            {VK_EXT_DEBUG_UTILS_EXTENSION_NAME, VK_EXT_DEBUG_UTILS_SPEC_VERSION} };

static const VkLayerProperties globalLayerProps = { "VK_LAYER_LUNARG_object_tracker",
                                                   VK_LAYER_API_VERSION,  // specVersion
                                                   1,                     // implementationVersion
                                                   "LunarG Validation Layer" };

VKAPI_ATTR VkResult VKAPI_CALL EnumerateInstanceLayerProperties(uint32_t *pCount, VkLayerProperties *pProperties) {
    return util_GetLayerProperties(1, &globalLayerProps, pCount, pProperties);
}

VKAPI_ATTR VkResult VKAPI_CALL EnumerateDeviceLayerProperties(VkPhysicalDevice physicalDevice, uint32_t *pCount,
                                                              VkLayerProperties *pProperties) {
    {
        std::lock_guard<std::mutex> lock(global_lock);
        bool skip = PreCallValidateEnumerateDeviceLayerProperties(physicalDevice, pCount, pProperties);
        if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
    }
    return util_GetLayerProperties(1, &globalLayerProps, pCount, pProperties);
}

VKAPI_ATTR VkResult VKAPI_CALL EnumerateInstanceExtensionProperties(const char *pLayerName, uint32_t *pCount,
                                                                    VkExtensionProperties *pProperties) {
    if (pLayerName && !strcmp(pLayerName, globalLayerProps.layerName))
        return util_GetExtensionProperties(1, instance_extensions, pCount, pProperties);

    return VK_ERROR_LAYER_NOT_PRESENT;
}

VKAPI_ATTR VkResult VKAPI_CALL EnumerateDeviceExtensionProperties(VkPhysicalDevice physicalDevice, const char *pLayerName,
                                                                  uint32_t *pCount, VkExtensionProperties *pProperties) {
    {
        std::lock_guard<std::mutex> lock(global_lock);
        bool skip = PreCallValidateEnumerateDeviceExtensionProperties(physicalDevice, pLayerName, pCount, pProperties);
        if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
    }
    if (pLayerName && !strcmp(pLayerName, globalLayerProps.layerName))
        return util_GetExtensionProperties(0, nullptr, pCount, pProperties);
    auto instance_data = GetLayerDataPtr(get_dispatch_key(physicalDevice), instance_layer_data_map);
    return instance_data->instance_dispatch_table.EnumerateDeviceExtensionProperties(physicalDevice, NULL, pCount, pProperties);
}

VKAPI_ATTR VkResult VKAPI_CALL CreateDevice(VkPhysicalDevice physicalDevice, const VkDeviceCreateInfo *pCreateInfo,
                                            const VkAllocationCallbacks *pAllocator, VkDevice *pDevice) {
    std::lock_guard<std::mutex> lock(global_lock);
    bool skip = PreCallValidateCreateDevice(physicalDevice, pCreateInfo, pAllocator, pDevice);
    if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;

    instance_layer_data *phy_dev_data = GetLayerDataPtr(get_dispatch_key(physicalDevice), instance_layer_data_map);
    VkLayerDeviceCreateInfo *chain_info = get_chain_info(pCreateInfo, VK_LAYER_LINK_INFO);

    assert(chain_info->u.pLayerInfo);
    PFN_vkGetInstanceProcAddr fpGetInstanceProcAddr = chain_info->u.pLayerInfo->pfnNextGetInstanceProcAddr;
    PFN_vkGetDeviceProcAddr fpGetDeviceProcAddr = chain_info->u.pLayerInfo->pfnNextGetDeviceProcAddr;
    PFN_vkCreateDevice fpCreateDevice = (PFN_vkCreateDevice)fpGetInstanceProcAddr(phy_dev_data->instance, "vkCreateDevice");
    if (fpCreateDevice == NULL) {
        return VK_ERROR_INITIALIZATION_FAILED;
    }

    // Advance the link info for the next element on the chain
    chain_info->u.pLayerInfo = chain_info->u.pLayerInfo->pNext;

    VkResult result = fpCreateDevice(physicalDevice, pCreateInfo, pAllocator, pDevice);
    if (result != VK_SUCCESS) {
        return result;
    }

    layer_data *device_data = GetLayerDataPtr(get_dispatch_key(*pDevice), layer_data_map);
    device_data->report_data = layer_debug_utils_create_device(phy_dev_data->report_data, *pDevice);
    layer_init_device_dispatch_table(*pDevice, &device_data->device_dispatch_table, fpGetDeviceProcAddr);

    // Save pCreateInfo device extension list for GetDeviceProcAddr()
    for (uint32_t extn = 0; extn < pCreateInfo->enabledExtensionCount; extn++) {
        device_data->device_extension_set.insert(pCreateInfo->ppEnabledExtensionNames[extn]);
    }

    // Add link back to physDev
    device_data->physical_device = physicalDevice;
    device_data->instance_data = phy_dev_data;

    PostCallRecordCreateDevice(physicalDevice, pCreateInfo, pAllocator, pDevice);

    return result;
}

VKAPI_ATTR VkResult VKAPI_CALL GetSwapchainImagesKHR(VkDevice device, VkSwapchainKHR swapchain, uint32_t *pSwapchainImageCount,
                                                     VkImage *pSwapchainImages) {
    {
        std::lock_guard<std::mutex> lock(global_lock);
        bool skip = PreCallValidateGetSwapchainImagesKHR(device, swapchain, pSwapchainImageCount, pSwapchainImages);
        if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
    }
    layer_data *device_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    VkResult result =
        device_data->device_dispatch_table.GetSwapchainImagesKHR(device, swapchain, pSwapchainImageCount, pSwapchainImages);
    {
        std::lock_guard<std::mutex> lock(global_lock);
        PostCallRecordGetSwapchainImagesKHR(device, swapchain, pSwapchainImageCount, pSwapchainImages);
    }
    return result;
}

VKAPI_ATTR VkResult VKAPI_CALL CreateDescriptorSetLayout(VkDevice device, const VkDescriptorSetLayoutCreateInfo *pCreateInfo,
                                                         const VkAllocationCallbacks *pAllocator,
                                                         VkDescriptorSetLayout *pSetLayout) {
    {
        std::lock_guard<std::mutex> lock(global_lock);
        bool skip = PreCallValidateCreateDescriptorSetLayout(device, pCreateInfo, pAllocator, pSetLayout);
        if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
    }
    layer_data *device_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    VkResult result = device_data->device_dispatch_table.CreateDescriptorSetLayout(device, pCreateInfo, pAllocator, pSetLayout);
    if (VK_SUCCESS == result) {
        std::lock_guard<std::mutex> lock(global_lock);
        PostCallRecordCreateDescriptorSetLayout(device, pCreateInfo, pAllocator, pSetLayout);
    }
    return result;
}

VKAPI_ATTR void VKAPI_CALL GetDescriptorSetLayoutSupport(VkDevice device, const VkDescriptorSetLayoutCreateInfo *pCreateInfo,
                                                         VkDescriptorSetLayoutSupport *pSupport) {
    bool skip = false;
    {
        std::lock_guard<std::mutex> lock(global_lock);
        skip = PreCallValidateGetDescriptorSetLayoutSupport(device, pCreateInfo, pSupport);
    }
    if (skip) return;
    GetLayerDataPtr(get_dispatch_key(device), layer_data_map)
        ->device_dispatch_table.GetDescriptorSetLayoutSupport(device, pCreateInfo, pSupport);
}

VKAPI_ATTR void VKAPI_CALL GetDescriptorSetLayoutSupportKHR(VkDevice device, const VkDescriptorSetLayoutCreateInfo *pCreateInfo,
                                                            VkDescriptorSetLayoutSupport *pSupport) {
    bool skip = false;
    {
        std::lock_guard<std::mutex> lock(global_lock);
        skip = PreCallValidateGetDescriptorSetLayoutSupport(device, pCreateInfo, pSupport);
    }
    if (skip) return;
    GetLayerDataPtr(get_dispatch_key(device), layer_data_map)
        ->device_dispatch_table.GetDescriptorSetLayoutSupportKHR(device, pCreateInfo, pSupport);
}

VKAPI_ATTR void VKAPI_CALL GetPhysicalDeviceQueueFamilyProperties(VkPhysicalDevice physicalDevice,
                                                                  uint32_t *pQueueFamilyPropertyCount,
                                                                  VkQueueFamilyProperties *pQueueFamilyProperties) {
    bool skip = false;
    {
        std::lock_guard<std::mutex> lock(global_lock);
        skip |= PreCallValidateGetPhysicalDeviceQueueFamilyProperties(physicalDevice, pQueueFamilyPropertyCount,
                                                                      pQueueFamilyProperties);
    }
    if (skip) return;

    auto instance_data = GetLayerDataPtr(get_dispatch_key(physicalDevice), instance_layer_data_map);
    instance_data->instance_dispatch_table.GetPhysicalDeviceQueueFamilyProperties(physicalDevice, pQueueFamilyPropertyCount,
                                                                                  pQueueFamilyProperties);
    std::lock_guard<std::mutex> lock(global_lock);
    PostCallRecordGetPhysicalDeviceQueueFamilyProperties(physicalDevice, pQueueFamilyPropertyCount, pQueueFamilyProperties);
}

VKAPI_ATTR VkResult VKAPI_CALL CreateInstance(const VkInstanceCreateInfo *pCreateInfo, const VkAllocationCallbacks *pAllocator,
                                              VkInstance *pInstance) {
    VkLayerInstanceCreateInfo *chain_info = get_chain_info(pCreateInfo, VK_LAYER_LINK_INFO);

    assert(chain_info->u.pLayerInfo);
    PFN_vkGetInstanceProcAddr fpGetInstanceProcAddr = chain_info->u.pLayerInfo->pfnNextGetInstanceProcAddr;
    PFN_vkCreateInstance fpCreateInstance = (PFN_vkCreateInstance)fpGetInstanceProcAddr(NULL, "vkCreateInstance");
    if (fpCreateInstance == NULL) {
        return VK_ERROR_INITIALIZATION_FAILED;
    }

    // Advance the link info for the next element on the chain
    chain_info->u.pLayerInfo = chain_info->u.pLayerInfo->pNext;

    VkResult result = fpCreateInstance(pCreateInfo, pAllocator, pInstance);
    if (result != VK_SUCCESS) {
        return result;
    }

    instance_layer_data *instance_data = GetLayerDataPtr(get_dispatch_key(*pInstance), instance_layer_data_map);
    instance_data->instance = *pInstance;
    layer_init_instance_dispatch_table(*pInstance, &instance_data->instance_dispatch_table, fpGetInstanceProcAddr);

    // Look for one or more debug report create info structures, and copy the
    // callback(s) for each one found (for use by vkDestroyInstance)
    layer_copy_tmp_debug_messengers(pCreateInfo->pNext, &instance_data->num_tmp_debug_messengers,
                                    &instance_data->tmp_messenger_create_infos, &instance_data->tmp_debug_messengers);
    layer_copy_tmp_report_callbacks(pCreateInfo->pNext, &instance_data->num_tmp_report_callbacks,
                                    &instance_data->tmp_report_create_infos, &instance_data->tmp_report_callbacks);

    instance_data->report_data =
        debug_utils_create_instance(&instance_data->instance_dispatch_table, *pInstance, pCreateInfo->enabledExtensionCount,
                                    pCreateInfo->ppEnabledExtensionNames);

    InitObjectTracker(instance_data, pAllocator);

    PostCallRecordCreateInstance(pCreateInfo, pAllocator, pInstance);

    return result;
}

VKAPI_ATTR VkResult VKAPI_CALL EnumeratePhysicalDevices(VkInstance instance, uint32_t *pPhysicalDeviceCount,
                                                        VkPhysicalDevice *pPhysicalDevices) {
    bool skip = VK_FALSE;
    {
        std::lock_guard<std::mutex> lock(global_lock);
        skip |= PreCallValidateEnumeratePhysicalDevices(instance, pPhysicalDeviceCount, pPhysicalDevices);
    }
    if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;

    instance_layer_data *instance_data = GetLayerDataPtr(get_dispatch_key(instance), instance_layer_data_map);
    VkResult result =
        instance_data->instance_dispatch_table.EnumeratePhysicalDevices(instance, pPhysicalDeviceCount, pPhysicalDevices);
    if (result == VK_SUCCESS) {
        std::lock_guard<std::mutex> lock(global_lock);
        PostCallRecordEnumeratePhysicalDevices(instance, pPhysicalDeviceCount, pPhysicalDevices);
    }
    return result;
}

VKAPI_ATTR VkResult VKAPI_CALL AllocateCommandBuffers(VkDevice device, const VkCommandBufferAllocateInfo *pAllocateInfo,
                                                      VkCommandBuffer *pCommandBuffers) {
    bool skip = VK_FALSE;
    {
        std::unique_lock<std::mutex> lock(global_lock);
        skip = PreCallValidateAllocateCommandBuffers(device, pAllocateInfo, pCommandBuffers);
    }
    if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;

    layer_data *device_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    VkResult result = device_data->device_dispatch_table.AllocateCommandBuffers(device, pAllocateInfo, pCommandBuffers);
    if (VK_SUCCESS == result) {
        std::unique_lock<std::mutex> lock(global_lock);
        PostCallRecordAllocateCommandBuffers(device, pAllocateInfo, pCommandBuffers);
    }

    return result;
}

VKAPI_ATTR VkResult VKAPI_CALL AllocateDescriptorSets(VkDevice device, const VkDescriptorSetAllocateInfo *pAllocateInfo,
                                                      VkDescriptorSet *pDescriptorSets) {
    bool skip = VK_FALSE;
    {
        std::unique_lock<std::mutex> lock(global_lock);
        skip = PreCallValidateAllocateDescriptorSets(device, pAllocateInfo, pDescriptorSets);
    }
    if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;

    layer_data *device_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    VkResult result = device_data->device_dispatch_table.AllocateDescriptorSets(device, pAllocateInfo, pDescriptorSets);

    if (VK_SUCCESS == result) {
        std::unique_lock<std::mutex> lock(global_lock);
        PostCallRecordAllocateDescriptorSets(device, pAllocateInfo, pDescriptorSets);
    }

    return result;
}

VKAPI_ATTR void VKAPI_CALL FreeCommandBuffers(VkDevice device, VkCommandPool commandPool, uint32_t commandBufferCount,
                                              const VkCommandBuffer *pCommandBuffers) {
    bool skip = false;
    {
        std::unique_lock<std::mutex> lock(global_lock);
        skip = PreCallValidateFreeCommandBuffers(device, commandPool, commandBufferCount, pCommandBuffers);
        if (skip) return;
        PreCallRecordFreeCommandBuffers(device, commandPool, commandBufferCount, pCommandBuffers);
    }
    layer_data *device_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    device_data->device_dispatch_table.FreeCommandBuffers(device, commandPool, commandBufferCount, pCommandBuffers);
}

VKAPI_ATTR void VKAPI_CALL DestroySwapchainKHR(VkDevice device, VkSwapchainKHR swapchain, const VkAllocationCallbacks *pAllocator) {
    bool skip = false;
    {
        std::unique_lock<std::mutex> lock(global_lock);
        skip = PreCallValidateDestroySwapchainKHR(device, swapchain, pAllocator);
        if (skip) return;
        PreCallRecordDestroySwapchainKHR(device, swapchain, pAllocator);
    }
    layer_data *device_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    device_data->device_dispatch_table.DestroySwapchainKHR(device, swapchain, pAllocator);
}

VKAPI_ATTR VkResult VKAPI_CALL FreeDescriptorSets(VkDevice device, VkDescriptorPool descriptorPool, uint32_t descriptorSetCount,
                                                  const VkDescriptorSet *pDescriptorSets) {
    bool skip = false;
    {
        std::unique_lock<std::mutex> lock(global_lock);
        skip = PreCallValidateFreeDescriptorSets(device, descriptorPool, descriptorSetCount, pDescriptorSets);
        if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
        PreCallRecordFreeDescriptorSets(device, descriptorPool, descriptorSetCount, pDescriptorSets);
    }
    layer_data *device_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    return device_data->device_dispatch_table.FreeDescriptorSets(device, descriptorPool, descriptorSetCount, pDescriptorSets);
}

VKAPI_ATTR void VKAPI_CALL DestroyDescriptorPool(VkDevice device, VkDescriptorPool descriptorPool,
                                                 const VkAllocationCallbacks *pAllocator) {
    bool skip = false;
    {
        std::unique_lock<std::mutex> lock(global_lock);
        skip = PreCallValidateDestroyDescriptorPool(device, descriptorPool, pAllocator);
        if (skip) return;
        PreCallRecordDestroyDescriptorPool(device, descriptorPool, pAllocator);
    }
    layer_data *device_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    device_data->device_dispatch_table.DestroyDescriptorPool(device, descriptorPool, pAllocator);
}

VKAPI_ATTR void VKAPI_CALL DestroyCommandPool(VkDevice device, VkCommandPool commandPool, const VkAllocationCallbacks *pAllocator) {
    bool skip = false;
    {
        std::unique_lock<std::mutex> lock(global_lock);
        skip = PreCallValidateDestroyCommandPool(device, commandPool, pAllocator);
        if (skip) return;
        PreCallRecordDestroyCommandPool(device, commandPool, pAllocator);
    }
    layer_data *device_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    device_data->device_dispatch_table.DestroyCommandPool(device, commandPool, pAllocator);
}

VKAPI_ATTR void VKAPI_CALL GetPhysicalDeviceQueueFamilyProperties2(VkPhysicalDevice physicalDevice,
                                                                   uint32_t *pQueueFamilyPropertyCount,
                                                                   VkQueueFamilyProperties2KHR *pQueueFamilyProperties) {
    bool skip = false;
    {
        std::lock_guard<std::mutex> lock(global_lock);
        skip |= PreCallValidateGetPhysicalDeviceQueueFamilyProperties2(physicalDevice, pQueueFamilyPropertyCount,
                                                                       pQueueFamilyProperties);
    }
    if (skip) return;
    instance_layer_data *instance_data = GetLayerDataPtr(get_dispatch_key(physicalDevice), instance_layer_data_map);
    instance_data->instance_dispatch_table.GetPhysicalDeviceQueueFamilyProperties2(physicalDevice, pQueueFamilyPropertyCount,
                                                                                   pQueueFamilyProperties);
    std::lock_guard<std::mutex> lock(global_lock);
    PostCallRecordGetPhysicalDeviceQueueFamilyProperties2(physicalDevice, pQueueFamilyPropertyCount, pQueueFamilyProperties);
}

VKAPI_ATTR void VKAPI_CALL GetPhysicalDeviceQueueFamilyProperties2KHR(VkPhysicalDevice physicalDevice,
                                                                      uint32_t *pQueueFamilyPropertyCount,
                                                                      VkQueueFamilyProperties2KHR *pQueueFamilyProperties) {

    bool skip = false;
    {
        std::lock_guard<std::mutex> lock(global_lock);
        skip |= PreCallValidateGetPhysicalDeviceQueueFamilyProperties2(physicalDevice, pQueueFamilyPropertyCount,
            pQueueFamilyProperties);
    }
    if (skip) return;
    instance_layer_data *instance_data = GetLayerDataPtr(get_dispatch_key(physicalDevice), instance_layer_data_map);
    instance_data->instance_dispatch_table.GetPhysicalDeviceQueueFamilyProperties2(physicalDevice, pQueueFamilyPropertyCount,
        pQueueFamilyProperties);
    std::lock_guard<std::mutex> lock(global_lock);
    PostCallRecordGetPhysicalDeviceQueueFamilyProperties2(physicalDevice, pQueueFamilyPropertyCount, pQueueFamilyProperties);
}

VKAPI_ATTR VkResult VKAPI_CALL GetPhysicalDeviceDisplayPropertiesKHR(VkPhysicalDevice physicalDevice, uint32_t *pPropertyCount,
                                                                     VkDisplayPropertiesKHR *pProperties) {
    bool skip = false;
    {
        std::lock_guard<std::mutex> lock(global_lock);
        skip |= PreCallValidateGetPhysicalDeviceDisplayPropertiesKHR(physicalDevice, pPropertyCount, pProperties);
    }
    if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;

    instance_layer_data *instance_data = GetLayerDataPtr(get_dispatch_key(physicalDevice), instance_layer_data_map);
    VkResult result =
        instance_data->instance_dispatch_table.GetPhysicalDeviceDisplayPropertiesKHR(physicalDevice, pPropertyCount, pProperties);

    if (result == VK_SUCCESS) {
        std::lock_guard<std::mutex> lock(global_lock);
        PostCallRecordGetPhysicalDeviceDisplayPropertiesKHR(physicalDevice, pPropertyCount, pProperties);
    }
    return result;
}

VKAPI_ATTR VkResult VKAPI_CALL GetDisplayModePropertiesKHR(VkPhysicalDevice physicalDevice, VkDisplayKHR display,
                                                           uint32_t *pPropertyCount, VkDisplayModePropertiesKHR *pProperties) {
    bool skip = false;
    {
        std::unique_lock<std::mutex> lock(global_lock);
        skip |= PreCallValidateGetDisplayModePropertiesKHR(physicalDevice, display, pPropertyCount, pProperties);
    }
    if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;

    instance_layer_data *instance_data = GetLayerDataPtr(get_dispatch_key(physicalDevice), instance_layer_data_map);
    VkResult result =
        instance_data->instance_dispatch_table.GetDisplayModePropertiesKHR(physicalDevice, display, pPropertyCount, pProperties);

    if (result == VK_SUCCESS) {
        if (pProperties) {
            std::unique_lock<std::mutex> lock(global_lock);
            PostCallRecordGetDisplayModePropertiesKHR(physicalDevice, display, pPropertyCount, pProperties);
        }
    }
    return result;
}

VKAPI_ATTR VkResult VKAPI_CALL DebugMarkerSetObjectNameEXT(VkDevice device, const VkDebugMarkerObjectNameInfoEXT *pNameInfo) {
    bool skip = VK_FALSE;
    layer_data *dev_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    {
        std::unique_lock<std::mutex> lock(global_lock);
        skip |= PreCallValidateDebugMarkerSetObjectNameEXT(device, pNameInfo);
        if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
        if (pNameInfo->pObjectName) {
            dev_data->report_data->debugObjectNameMap->insert(
                std::make_pair<uint64_t, std::string>((uint64_t &&) pNameInfo->object, pNameInfo->pObjectName));
        } else {
            dev_data->report_data->debugObjectNameMap->erase(pNameInfo->object);
        }
    }
    VkResult result = dev_data->device_dispatch_table.DebugMarkerSetObjectNameEXT(device, pNameInfo);
    return result;
}

// This is not a Vulkan API, but is part of the loader-layer interface.
VKAPI_ATTR PFN_vkVoidFunction VKAPI_CALL GetPhysicalDeviceProcAddr(VkInstance instance, const char *funcName) {
    instance_layer_data *instance_data = GetLayerDataPtr(get_dispatch_key(instance), instance_layer_data_map);
    if (instance_data->instance_dispatch_table.GetPhysicalDeviceProcAddr == NULL) {
        return NULL;
    }
    return instance_data->instance_dispatch_table.GetPhysicalDeviceProcAddr(instance, funcName);
}


VKAPI_ATTR PFN_vkVoidFunction VKAPI_CALL GetDeviceProcAddr(VkDevice device, const char *funcName) {
    {
        std::unique_lock<std::mutex> lock(global_lock);
        bool skip = PreCallValidateGetDeviceProcAddr(device, funcName);
        if (skip) return nullptr;
    }
    layer_data *device_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    if (!ApiParentExtensionEnabled(funcName, device_data->device_extension_set)) {
        return nullptr;
    }
    const auto item = name_to_funcptr_map.find(funcName);
    if (item != name_to_funcptr_map.end()) {
        return reinterpret_cast<PFN_vkVoidFunction>(item->second);
    }
    if (!device_data->device_dispatch_table.GetDeviceProcAddr) return NULL;
    return device_data->device_dispatch_table.GetDeviceProcAddr(device, funcName);
}

VKAPI_ATTR PFN_vkVoidFunction VKAPI_CALL GetInstanceProcAddr(VkInstance instance, const char *funcName) {
    if (instance != nullptr) {
        std::unique_lock<std::mutex> lock(global_lock);
        bool skip = PreCallValidateGetInstanceProcAddr(instance, funcName);
        if (skip) return nullptr;
    }
    const auto item = name_to_funcptr_map.find(funcName);
    if (item != name_to_funcptr_map.end()) {
        return reinterpret_cast<PFN_vkVoidFunction>(item->second);
    }
    instance_layer_data *instance_data = GetLayerDataPtr(get_dispatch_key(instance), instance_layer_data_map);
    if (!instance_data->instance_dispatch_table.GetInstanceProcAddr) return nullptr;
    return instance_data->instance_dispatch_table.GetInstanceProcAddr(instance, funcName);
}

VKAPI_ATTR VkResult VKAPI_CALL GetPhysicalDeviceDisplayProperties2KHR(VkPhysicalDevice physicalDevice, uint32_t *pPropertyCount,
                                                                      VkDisplayProperties2KHR *pProperties) {
    {
        std::lock_guard<std::mutex> lock(global_lock);
        bool skip = PreCallValidateGetPhysicalDeviceDisplayProperties2KHR(physicalDevice, pPropertyCount, pProperties);
        if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
    }
    instance_layer_data *instance_data = GetLayerDataPtr(get_dispatch_key(physicalDevice), instance_layer_data_map);
    VkResult result =
        instance_data->instance_dispatch_table.GetPhysicalDeviceDisplayProperties2KHR(physicalDevice, pPropertyCount, pProperties);
    if (pProperties && (VK_SUCCESS == result || VK_INCOMPLETE == result)) {
        std::lock_guard<std::mutex> lock(global_lock);
        PostCallRecordGetPhysicalDeviceDisplayProperties2KHR(physicalDevice, pPropertyCount, pProperties);
    }

    return result;
}

VKAPI_ATTR VkResult VKAPI_CALL GetDisplayPlaneSupportedDisplaysKHR(VkPhysicalDevice physicalDevice, uint32_t planeIndex,
                                                                   uint32_t *pDisplayCount, VkDisplayKHR *pDisplays) {
    {
        std::lock_guard<std::mutex> lock(global_lock);
        bool skip = PreCallValidateGetDisplayPlaneSupportedDisplaysKHR(physicalDevice, planeIndex, pDisplayCount, pDisplays);
        if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
    }
    instance_layer_data *instance_data = GetLayerDataPtr(get_dispatch_key(physicalDevice), instance_layer_data_map);
    VkResult result = instance_data->instance_dispatch_table.GetDisplayPlaneSupportedDisplaysKHR(physicalDevice, planeIndex,
                                                                                                 pDisplayCount, pDisplays);
    if (pDisplays && (VK_SUCCESS == result || VK_INCOMPLETE == result)) {
        std::lock_guard<std::mutex> lock(global_lock);
        PostCallRecordGetDisplayPlaneSupportedDisplaysKHR(physicalDevice, planeIndex, pDisplayCount, pDisplays);
    }
    return result;
}

VKAPI_ATTR VkResult VKAPI_CALL GetDisplayModeProperties2KHR(VkPhysicalDevice physicalDevice, VkDisplayKHR display,
                                                            uint32_t *pPropertyCount, VkDisplayModeProperties2KHR *pProperties) {
    {
        std::lock_guard<std::mutex> lock(global_lock);
        bool skip = PreCallValidateGetDisplayModeProperties2KHR(physicalDevice, display, pPropertyCount, pProperties);
        if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
    }
    instance_layer_data *instance_data = GetLayerDataPtr(get_dispatch_key(physicalDevice), instance_layer_data_map);
    VkResult result =
        instance_data->instance_dispatch_table.GetDisplayModeProperties2KHR(physicalDevice, display, pPropertyCount, pProperties);
    if (pProperties && (VK_SUCCESS == result || VK_INCOMPLETE == result)) {
        std::lock_guard<std::mutex> lock(global_lock);
        PostCallRecordGetDisplayModeProperties2KHR(physicalDevice, display, pPropertyCount, pProperties);
    }
    return result;
}

}  // namespace object_tracker

VK_LAYER_EXPORT VKAPI_ATTR VkResult VKAPI_CALL vkEnumerateInstanceExtensionProperties(const char *pLayerName, uint32_t *pCount,
                                                                                      VkExtensionProperties *pProperties) {
    return object_tracker::EnumerateInstanceExtensionProperties(pLayerName, pCount, pProperties);
}

VK_LAYER_EXPORT VKAPI_ATTR VkResult VKAPI_CALL vkEnumerateInstanceLayerProperties(uint32_t *pCount,
                                                                                  VkLayerProperties *pProperties) {
    return object_tracker::EnumerateInstanceLayerProperties(pCount, pProperties);
}

VK_LAYER_EXPORT VKAPI_ATTR VkResult VKAPI_CALL vkEnumerateDeviceLayerProperties(VkPhysicalDevice physicalDevice, uint32_t *pCount,
                                                                                VkLayerProperties *pProperties) {
    // The layer command handles VK_NULL_HANDLE just fine internally
    assert(physicalDevice == VK_NULL_HANDLE);
    return object_tracker::EnumerateDeviceLayerProperties(VK_NULL_HANDLE, pCount, pProperties);
}

VK_LAYER_EXPORT VKAPI_ATTR PFN_vkVoidFunction VKAPI_CALL vkGetDeviceProcAddr(VkDevice dev, const char *funcName) {
    return object_tracker::GetDeviceProcAddr(dev, funcName);
}

VK_LAYER_EXPORT VKAPI_ATTR PFN_vkVoidFunction VKAPI_CALL vkGetInstanceProcAddr(VkInstance instance, const char *funcName) {
    return object_tracker::GetInstanceProcAddr(instance, funcName);
}

VK_LAYER_EXPORT VKAPI_ATTR VkResult VKAPI_CALL vkEnumerateDeviceExtensionProperties(VkPhysicalDevice physicalDevice,
                                                                                    const char *pLayerName, uint32_t *pCount,
                                                                                    VkExtensionProperties *pProperties) {
    // The layer command handles VK_NULL_HANDLE just fine internally
    assert(physicalDevice == VK_NULL_HANDLE);
    return object_tracker::EnumerateDeviceExtensionProperties(VK_NULL_HANDLE, pLayerName, pCount, pProperties);
}

VK_LAYER_EXPORT VKAPI_ATTR PFN_vkVoidFunction VKAPI_CALL vk_layerGetPhysicalDeviceProcAddr(VkInstance instance,
                                                                                           const char *funcName) {
    return object_tracker::GetPhysicalDeviceProcAddr(instance, funcName);
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
        object_tracker::loader_layer_if_version = pVersionStruct->loaderLayerInterfaceVersion;
    } else if (pVersionStruct->loaderLayerInterfaceVersion > CURRENT_LOADER_LAYER_INTERFACE_VERSION) {
        pVersionStruct->loaderLayerInterfaceVersion = CURRENT_LOADER_LAYER_INTERFACE_VERSION;
    }

    return VK_SUCCESS;
}
