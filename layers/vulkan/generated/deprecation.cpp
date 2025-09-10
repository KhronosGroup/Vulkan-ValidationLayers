// *** THIS FILE IS GENERATED - DO NOT EDIT ***
// See deprecation_generator.py for modifications

/***************************************************************************
 *
 * Copyright (c) 2025 The Khronos Group Inc.
 * Copyright (c) 2025 Valve Corporation
 * Copyright (c) 2025 LunarG, Inc.
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

#include "deprecation.h"

namespace deprecation {

bool Instance::PreCallValidateGetPhysicalDeviceFeatures(VkPhysicalDevice physicalDevice, VkPhysicalDeviceFeatures* pFeatures,
                                                        const ErrorObject& error_obj) const {
    static bool reported = false;
    if (reported) return false;

    if (api_version >= VK_API_VERSION_1_1) {
        reported = true;
        LogWarning("WARNING-deprecation-gpdp2", physicalDevice, error_obj.location,
                   "vkGetPhysicalDeviceFeatures is deprecated and this VkInstance was created with VK_VERSION_1_1 which contains "
                   "vkGetPhysicalDeviceFeatures2 that can be used instead.\nSee more information about this deprecation in the "
                   "specification: https://docs.vulkan.org/spec/latest/appendices/deprecation.html#deprecation-gpdp2");
    } else if (IsExtEnabled(extensions.vk_khr_get_physical_device_properties2)) {
        reported = true;
        LogWarning(
            "WARNING-deprecation-gpdp2", physicalDevice, error_obj.location,
            "vkGetPhysicalDeviceFeatures is deprecated and this VkInstance enabled the VK_KHR_get_physical_device_properties2 "
            "extension which contains vkGetPhysicalDeviceFeatures2KHR that can be used instead.\nSee more information about this "
            "deprecation in the specification: https://docs.vulkan.org/spec/latest/appendices/deprecation.html#deprecation-gpdp2");
    }
    return false;
}

bool Instance::PreCallValidateGetPhysicalDeviceFormatProperties(VkPhysicalDevice physicalDevice, VkFormat format,
                                                                VkFormatProperties* pFormatProperties,
                                                                const ErrorObject& error_obj) const {
    static bool reported = false;
    if (reported) return false;

    if (api_version >= VK_API_VERSION_1_1) {
        reported = true;
        LogWarning(
            "WARNING-deprecation-gpdp2", physicalDevice, error_obj.location,
            "vkGetPhysicalDeviceFormatProperties is deprecated and this VkInstance was created with VK_VERSION_1_1 which contains "
            "vkGetPhysicalDeviceFormatProperties2 that can be used instead.\nSee more information about this deprecation in the "
            "specification: https://docs.vulkan.org/spec/latest/appendices/deprecation.html#deprecation-gpdp2");
    } else if (IsExtEnabled(extensions.vk_khr_get_physical_device_properties2)) {
        reported = true;
        LogWarning("WARNING-deprecation-gpdp2", physicalDevice, error_obj.location,
                   "vkGetPhysicalDeviceFormatProperties is deprecated and this VkInstance enabled the "
                   "VK_KHR_get_physical_device_properties2 extension which contains vkGetPhysicalDeviceFormatProperties2KHR that "
                   "can be used instead.\nSee more information about this deprecation in the specification: "
                   "https://docs.vulkan.org/spec/latest/appendices/deprecation.html#deprecation-gpdp2");
    }
    return false;
}

bool Instance::PreCallValidateGetPhysicalDeviceImageFormatProperties(VkPhysicalDevice physicalDevice, VkFormat format,
                                                                     VkImageType type, VkImageTiling tiling,
                                                                     VkImageUsageFlags usage, VkImageCreateFlags flags,
                                                                     VkImageFormatProperties* pImageFormatProperties,
                                                                     const ErrorObject& error_obj) const {
    static bool reported = false;
    if (reported) return false;

    if (api_version >= VK_API_VERSION_1_1) {
        reported = true;
        LogWarning(
            "WARNING-deprecation-gpdp2", physicalDevice, error_obj.location,
            "vkGetPhysicalDeviceImageFormatProperties is deprecated and this VkInstance was created with VK_VERSION_1_1 which "
            "contains vkGetPhysicalDeviceImageFormatProperties2 that can be used instead.\nSee more information about this "
            "deprecation in the specification: https://docs.vulkan.org/spec/latest/appendices/deprecation.html#deprecation-gpdp2");
    } else if (IsExtEnabled(extensions.vk_khr_get_physical_device_properties2)) {
        reported = true;
        LogWarning("WARNING-deprecation-gpdp2", physicalDevice, error_obj.location,
                   "vkGetPhysicalDeviceImageFormatProperties is deprecated and this VkInstance enabled the "
                   "VK_KHR_get_physical_device_properties2 extension which contains vkGetPhysicalDeviceImageFormatProperties2KHR "
                   "that can be used instead.\nSee more information about this deprecation in the specification: "
                   "https://docs.vulkan.org/spec/latest/appendices/deprecation.html#deprecation-gpdp2");
    }
    return false;
}

bool Instance::PreCallValidateGetPhysicalDeviceProperties(VkPhysicalDevice physicalDevice, VkPhysicalDeviceProperties* pProperties,
                                                          const ErrorObject& error_obj) const {
    static bool reported = false;
    if (reported) return false;

    if (api_version >= VK_API_VERSION_1_1) {
        reported = true;
        LogWarning("WARNING-deprecation-gpdp2", physicalDevice, error_obj.location,
                   "vkGetPhysicalDeviceProperties is deprecated and this VkInstance was created with VK_VERSION_1_1 which contains "
                   "vkGetPhysicalDeviceProperties2 that can be used instead.\nSee more information about this deprecation in the "
                   "specification: https://docs.vulkan.org/spec/latest/appendices/deprecation.html#deprecation-gpdp2");
    } else if (IsExtEnabled(extensions.vk_khr_get_physical_device_properties2)) {
        reported = true;
        LogWarning(
            "WARNING-deprecation-gpdp2", physicalDevice, error_obj.location,
            "vkGetPhysicalDeviceProperties is deprecated and this VkInstance enabled the VK_KHR_get_physical_device_properties2 "
            "extension which contains vkGetPhysicalDeviceProperties2KHR that can be used instead.\nSee more information about this "
            "deprecation in the specification: https://docs.vulkan.org/spec/latest/appendices/deprecation.html#deprecation-gpdp2");
    }
    return false;
}

bool Instance::PreCallValidateGetPhysicalDeviceQueueFamilyProperties(VkPhysicalDevice physicalDevice,
                                                                     uint32_t* pQueueFamilyPropertyCount,
                                                                     VkQueueFamilyProperties* pQueueFamilyProperties,
                                                                     const ErrorObject& error_obj) const {
    static bool reported = false;
    if (reported) return false;

    if (api_version >= VK_API_VERSION_1_1) {
        reported = true;
        LogWarning(
            "WARNING-deprecation-gpdp2", physicalDevice, error_obj.location,
            "vkGetPhysicalDeviceQueueFamilyProperties is deprecated and this VkInstance was created with VK_VERSION_1_1 which "
            "contains vkGetPhysicalDeviceQueueFamilyProperties2 that can be used instead.\nSee more information about this "
            "deprecation in the specification: https://docs.vulkan.org/spec/latest/appendices/deprecation.html#deprecation-gpdp2");
    } else if (IsExtEnabled(extensions.vk_khr_get_physical_device_properties2)) {
        reported = true;
        LogWarning("WARNING-deprecation-gpdp2", physicalDevice, error_obj.location,
                   "vkGetPhysicalDeviceQueueFamilyProperties is deprecated and this VkInstance enabled the "
                   "VK_KHR_get_physical_device_properties2 extension which contains vkGetPhysicalDeviceQueueFamilyProperties2KHR "
                   "that can be used instead.\nSee more information about this deprecation in the specification: "
                   "https://docs.vulkan.org/spec/latest/appendices/deprecation.html#deprecation-gpdp2");
    }
    return false;
}

bool Instance::PreCallValidateGetPhysicalDeviceMemoryProperties(VkPhysicalDevice physicalDevice,
                                                                VkPhysicalDeviceMemoryProperties* pMemoryProperties,
                                                                const ErrorObject& error_obj) const {
    static bool reported = false;
    if (reported) return false;

    if (api_version >= VK_API_VERSION_1_1) {
        reported = true;
        LogWarning(
            "WARNING-deprecation-gpdp2", physicalDevice, error_obj.location,
            "vkGetPhysicalDeviceMemoryProperties is deprecated and this VkInstance was created with VK_VERSION_1_1 which contains "
            "vkGetPhysicalDeviceMemoryProperties2 that can be used instead.\nSee more information about this deprecation in the "
            "specification: https://docs.vulkan.org/spec/latest/appendices/deprecation.html#deprecation-gpdp2");
    } else if (IsExtEnabled(extensions.vk_khr_get_physical_device_properties2)) {
        reported = true;
        LogWarning("WARNING-deprecation-gpdp2", physicalDevice, error_obj.location,
                   "vkGetPhysicalDeviceMemoryProperties is deprecated and this VkInstance enabled the "
                   "VK_KHR_get_physical_device_properties2 extension which contains vkGetPhysicalDeviceMemoryProperties2KHR that "
                   "can be used instead.\nSee more information about this deprecation in the specification: "
                   "https://docs.vulkan.org/spec/latest/appendices/deprecation.html#deprecation-gpdp2");
    }
    return false;
}

bool Instance::PreCallValidateEnumerateDeviceLayerProperties(VkPhysicalDevice physicalDevice, uint32_t* pPropertyCount,
                                                             VkLayerProperties* pProperties, const ErrorObject& error_obj) const {
    static bool reported = false;
    if (reported) return false;

    LogWarning("WARNING-deprecation-devicelayers", physicalDevice, error_obj.location,
               "vkEnumerateDeviceLayerProperties is deprecated.\nSee more information about this deprecation in the specification: "
               "https://docs.vulkan.org/spec/latest/appendices/deprecation.html#deprecation-devicelayers");

    return false;
}

bool Instance::PreCallValidateGetPhysicalDeviceSparseImageFormatProperties(VkPhysicalDevice physicalDevice, VkFormat format,
                                                                           VkImageType type, VkSampleCountFlagBits samples,
                                                                           VkImageUsageFlags usage, VkImageTiling tiling,
                                                                           uint32_t* pPropertyCount,
                                                                           VkSparseImageFormatProperties* pProperties,
                                                                           const ErrorObject& error_obj) const {
    static bool reported = false;
    if (reported) return false;

    if (api_version >= VK_API_VERSION_1_1) {
        reported = true;
        LogWarning("WARNING-deprecation-gpdp2", physicalDevice, error_obj.location,
                   "vkGetPhysicalDeviceSparseImageFormatProperties is deprecated and this VkInstance was created with "
                   "VK_VERSION_1_1 which contains vkGetPhysicalDeviceSparseImageFormatProperties2 that can be used instead.\nSee "
                   "more information about this deprecation in the specification: "
                   "https://docs.vulkan.org/spec/latest/appendices/deprecation.html#deprecation-gpdp2");
    } else if (IsExtEnabled(extensions.vk_khr_get_physical_device_properties2)) {
        reported = true;
        LogWarning(
            "WARNING-deprecation-gpdp2", physicalDevice, error_obj.location,
            "vkGetPhysicalDeviceSparseImageFormatProperties is deprecated and this VkInstance enabled the "
            "VK_KHR_get_physical_device_properties2 extension which contains vkGetPhysicalDeviceSparseImageFormatProperties2KHR "
            "that can be used instead.\nSee more information about this deprecation in the specification: "
            "https://docs.vulkan.org/spec/latest/appendices/deprecation.html#deprecation-gpdp2");
    }
    return false;
}

bool Device::PreCallValidateCreateFramebuffer(VkDevice device, const VkFramebufferCreateInfo* pCreateInfo,
                                              const VkAllocationCallbacks* pAllocator, VkFramebuffer* pFramebuffer,
                                              const ErrorObject& error_obj) const {
    static bool reported = false;
    if (reported) return false;

    if (api_version >= VK_API_VERSION_1_4) {
        reported = true;
        LogWarning("WARNING-deprecation-dynamicrendering", device, error_obj.location,
                   "vkCreateFramebuffer is deprecated and this VkDevice was created with VK_VERSION_1_4 which contains the new "
                   "feature to replace it.\nSee more information about this deprecation in the specification: "
                   "https://docs.vulkan.org/spec/latest/appendices/deprecation.html#deprecation-dynamicrendering");
    } else if (IsExtEnabled(extensions.vk_khr_dynamic_rendering_local_read)) {
        reported = true;
        LogWarning("WARNING-deprecation-dynamicrendering", device, error_obj.location,
                   "vkCreateFramebuffer is deprecated and this VkDevice enabled the VK_KHR_dynamic_rendering_local_read extension "
                   "which contains the new feature to replace it.\nSee more information about this deprecation in the "
                   "specification: https://docs.vulkan.org/spec/latest/appendices/deprecation.html#deprecation-dynamicrendering");
    }
    return false;
}

bool Device::PreCallValidateCreateRenderPass(VkDevice device, const VkRenderPassCreateInfo* pCreateInfo,
                                             const VkAllocationCallbacks* pAllocator, VkRenderPass* pRenderPass,
                                             const ErrorObject& error_obj) const {
    static bool reported = false;
    if (reported) return false;

    if (api_version >= VK_API_VERSION_1_2) {
        reported = true;
        LogWarning("WARNING-deprecation-renderpass2", device, error_obj.location,
                   "vkCreateRenderPass is deprecated and this VkDevice was created with VK_VERSION_1_2 which contains "
                   "vkCreateRenderPass2 that can be used instead.\nSee more information about this deprecation in the "
                   "specification: https://docs.vulkan.org/spec/latest/appendices/deprecation.html#deprecation-renderpass2");
    } else if (IsExtEnabled(extensions.vk_khr_create_renderpass2)) {
        reported = true;
        LogWarning("WARNING-deprecation-renderpass2", device, error_obj.location,
                   "vkCreateRenderPass is deprecated and this VkDevice enabled the VK_KHR_create_renderpass2 extension which "
                   "contains vkCreateRenderPass2KHR that can be used instead.\nSee more information about this deprecation in the "
                   "specification: https://docs.vulkan.org/spec/latest/appendices/deprecation.html#deprecation-renderpass2");
    }
    return false;
}

bool Device::PreCallValidateGetRenderAreaGranularity(VkDevice device, VkRenderPass renderPass, VkExtent2D* pGranularity,
                                                     const ErrorObject& error_obj) const {
    static bool reported = false;
    if (reported) return false;

    if (api_version >= VK_API_VERSION_1_4) {
        reported = true;
        LogWarning("WARNING-deprecation-dynamicrendering", device, error_obj.location,
                   "vkGetRenderAreaGranularity is deprecated and this VkDevice was created with VK_VERSION_1_4 which contains the "
                   "new feature to replace it.\nSee more information about this deprecation in the specification: "
                   "https://docs.vulkan.org/spec/latest/appendices/deprecation.html#deprecation-dynamicrendering");
    } else if (IsExtEnabled(extensions.vk_khr_dynamic_rendering_local_read)) {
        reported = true;
        LogWarning("WARNING-deprecation-dynamicrendering", device, error_obj.location,
                   "vkGetRenderAreaGranularity is deprecated and this VkDevice enabled the VK_KHR_dynamic_rendering_local_read "
                   "extension which contains the new feature to replace it.\nSee more information about this deprecation in the "
                   "specification: https://docs.vulkan.org/spec/latest/appendices/deprecation.html#deprecation-dynamicrendering");
    }
    return false;
}

bool Device::PreCallValidateCmdBeginRenderPass(VkCommandBuffer commandBuffer, const VkRenderPassBeginInfo* pRenderPassBegin,
                                               VkSubpassContents contents, const ErrorObject& error_obj) const {
    static bool reported = false;
    if (reported) return false;

    if (api_version >= VK_API_VERSION_1_2) {
        reported = true;
        LogWarning("WARNING-deprecation-renderpass2", device, error_obj.location,
                   "vkCmdBeginRenderPass is deprecated and this VkDevice was created with VK_VERSION_1_2 which contains "
                   "vkCmdBeginRenderPass2 that can be used instead.\nSee more information about this deprecation in the "
                   "specification: https://docs.vulkan.org/spec/latest/appendices/deprecation.html#deprecation-renderpass2");
    } else if (IsExtEnabled(extensions.vk_khr_create_renderpass2)) {
        reported = true;
        LogWarning("WARNING-deprecation-renderpass2", device, error_obj.location,
                   "vkCmdBeginRenderPass is deprecated and this VkDevice enabled the VK_KHR_create_renderpass2 extension which "
                   "contains vkCmdBeginRenderPass2KHR that can be used instead.\nSee more information about this deprecation in "
                   "the specification: https://docs.vulkan.org/spec/latest/appendices/deprecation.html#deprecation-renderpass2");
    }
    return false;
}

bool Device::PreCallValidateCmdNextSubpass(VkCommandBuffer commandBuffer, VkSubpassContents contents,
                                           const ErrorObject& error_obj) const {
    static bool reported = false;
    if (reported) return false;

    if (api_version >= VK_API_VERSION_1_2) {
        reported = true;
        LogWarning("WARNING-deprecation-renderpass2", device, error_obj.location,
                   "vkCmdNextSubpass is deprecated and this VkDevice was created with VK_VERSION_1_2 which contains "
                   "vkCmdNextSubpass2 that can be used instead.\nSee more information about this deprecation in the specification: "
                   "https://docs.vulkan.org/spec/latest/appendices/deprecation.html#deprecation-renderpass2");
    } else if (IsExtEnabled(extensions.vk_khr_create_renderpass2)) {
        reported = true;
        LogWarning("WARNING-deprecation-renderpass2", device, error_obj.location,
                   "vkCmdNextSubpass is deprecated and this VkDevice enabled the VK_KHR_create_renderpass2 extension which "
                   "contains vkCmdNextSubpass2KHR that can be used instead.\nSee more information about this deprecation in the "
                   "specification: https://docs.vulkan.org/spec/latest/appendices/deprecation.html#deprecation-renderpass2");
    }
    return false;
}

bool Device::PreCallValidateCmdEndRenderPass(VkCommandBuffer commandBuffer, const ErrorObject& error_obj) const {
    static bool reported = false;
    if (reported) return false;

    if (api_version >= VK_API_VERSION_1_2) {
        reported = true;
        LogWarning("WARNING-deprecation-renderpass2", device, error_obj.location,
                   "vkCmdEndRenderPass is deprecated and this VkDevice was created with VK_VERSION_1_2 which contains "
                   "vkCmdEndRenderPass2 that can be used instead.\nSee more information about this deprecation in the "
                   "specification: https://docs.vulkan.org/spec/latest/appendices/deprecation.html#deprecation-renderpass2");
    } else if (IsExtEnabled(extensions.vk_khr_create_renderpass2)) {
        reported = true;
        LogWarning("WARNING-deprecation-renderpass2", device, error_obj.location,
                   "vkCmdEndRenderPass is deprecated and this VkDevice enabled the VK_KHR_create_renderpass2 extension which "
                   "contains vkCmdEndRenderPass2KHR that can be used instead.\nSee more information about this deprecation in the "
                   "specification: https://docs.vulkan.org/spec/latest/appendices/deprecation.html#deprecation-renderpass2");
    }
    return false;
}

bool Device::PreCallValidateCreateRenderPass2(VkDevice device, const VkRenderPassCreateInfo2* pCreateInfo,
                                              const VkAllocationCallbacks* pAllocator, VkRenderPass* pRenderPass,
                                              const ErrorObject& error_obj) const {
    static bool reported = false;
    if (reported) return false;

    if (api_version >= VK_API_VERSION_1_4) {
        reported = true;
        LogWarning("WARNING-deprecation-dynamicrendering", device, error_obj.location,
                   "vkCreateRenderPass2 is deprecated and this VkDevice was created with VK_VERSION_1_4 which contains the new "
                   "feature to replace it.\nSee more information about this deprecation in the specification: "
                   "https://docs.vulkan.org/spec/latest/appendices/deprecation.html#deprecation-dynamicrendering");
    } else if (IsExtEnabled(extensions.vk_khr_dynamic_rendering_local_read)) {
        reported = true;
        LogWarning("WARNING-deprecation-dynamicrendering", device, error_obj.location,
                   "vkCreateRenderPass2 is deprecated and this VkDevice enabled the VK_KHR_dynamic_rendering_local_read extension "
                   "which contains the new feature to replace it.\nSee more information about this deprecation in the "
                   "specification: https://docs.vulkan.org/spec/latest/appendices/deprecation.html#deprecation-dynamicrendering");
    }
    return false;
}

bool Device::PreCallValidateCmdBeginRenderPass2(VkCommandBuffer commandBuffer, const VkRenderPassBeginInfo* pRenderPassBegin,
                                                const VkSubpassBeginInfo* pSubpassBeginInfo, const ErrorObject& error_obj) const {
    static bool reported = false;
    if (reported) return false;

    if (api_version >= VK_API_VERSION_1_4) {
        reported = true;
        LogWarning("WARNING-deprecation-dynamicrendering", device, error_obj.location,
                   "vkCmdBeginRenderPass2 is deprecated and this VkDevice was created with VK_VERSION_1_4 which contains the new "
                   "feature to replace it.\nSee more information about this deprecation in the specification: "
                   "https://docs.vulkan.org/spec/latest/appendices/deprecation.html#deprecation-dynamicrendering");
    } else if (IsExtEnabled(extensions.vk_khr_dynamic_rendering_local_read)) {
        reported = true;
        LogWarning("WARNING-deprecation-dynamicrendering", device, error_obj.location,
                   "vkCmdBeginRenderPass2 is deprecated and this VkDevice enabled the VK_KHR_dynamic_rendering_local_read "
                   "extension which contains the new feature to replace it.\nSee more information about this deprecation in the "
                   "specification: https://docs.vulkan.org/spec/latest/appendices/deprecation.html#deprecation-dynamicrendering");
    }
    return false;
}

bool Device::PreCallValidateCmdNextSubpass2(VkCommandBuffer commandBuffer, const VkSubpassBeginInfo* pSubpassBeginInfo,
                                            const VkSubpassEndInfo* pSubpassEndInfo, const ErrorObject& error_obj) const {
    static bool reported = false;
    if (reported) return false;

    if (api_version >= VK_API_VERSION_1_4) {
        reported = true;
        LogWarning("WARNING-deprecation-dynamicrendering", device, error_obj.location,
                   "vkCmdNextSubpass2 is deprecated and this VkDevice was created with VK_VERSION_1_4 which contains the new "
                   "feature to replace it.\nSee more information about this deprecation in the specification: "
                   "https://docs.vulkan.org/spec/latest/appendices/deprecation.html#deprecation-dynamicrendering");
    } else if (IsExtEnabled(extensions.vk_khr_dynamic_rendering_local_read)) {
        reported = true;
        LogWarning("WARNING-deprecation-dynamicrendering", device, error_obj.location,
                   "vkCmdNextSubpass2 is deprecated and this VkDevice enabled the VK_KHR_dynamic_rendering_local_read extension "
                   "which contains the new feature to replace it.\nSee more information about this deprecation in the "
                   "specification: https://docs.vulkan.org/spec/latest/appendices/deprecation.html#deprecation-dynamicrendering");
    }
    return false;
}

bool Device::PreCallValidateCmdEndRenderPass2(VkCommandBuffer commandBuffer, const VkSubpassEndInfo* pSubpassEndInfo,
                                              const ErrorObject& error_obj) const {
    static bool reported = false;
    if (reported) return false;

    if (api_version >= VK_API_VERSION_1_4) {
        reported = true;
        LogWarning("WARNING-deprecation-dynamicrendering", device, error_obj.location,
                   "vkCmdEndRenderPass2 is deprecated and this VkDevice was created with VK_VERSION_1_4 which contains the new "
                   "feature to replace it.\nSee more information about this deprecation in the specification: "
                   "https://docs.vulkan.org/spec/latest/appendices/deprecation.html#deprecation-dynamicrendering");
    } else if (IsExtEnabled(extensions.vk_khr_dynamic_rendering_local_read)) {
        reported = true;
        LogWarning("WARNING-deprecation-dynamicrendering", device, error_obj.location,
                   "vkCmdEndRenderPass2 is deprecated and this VkDevice enabled the VK_KHR_dynamic_rendering_local_read extension "
                   "which contains the new feature to replace it.\nSee more information about this deprecation in the "
                   "specification: https://docs.vulkan.org/spec/latest/appendices/deprecation.html#deprecation-dynamicrendering");
    }
    return false;
}
}  // namespace deprecation
// NOLINTEND
