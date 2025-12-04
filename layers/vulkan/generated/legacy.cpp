// *** THIS FILE IS GENERATED - DO NOT EDIT ***
// See legacy_generator.py for modifications

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

#include "legacy.h"

namespace legacy {

bool Instance::PreCallValidateGetPhysicalDeviceFeatures(VkPhysicalDevice physicalDevice, VkPhysicalDeviceFeatures* pFeatures,
                                                        const ErrorObject& error_obj) const {
    static bool reported = false;
    if (reported) return false;

    if (api_version >= VK_API_VERSION_1_1) {
        reported = true;
        LogWarning("WARNING-legacy-gpdp2", physicalDevice, error_obj.location,
                   "vkGetPhysicalDeviceFeatures is a legacy command and this VkInstance was created with VK_VERSION_1_1 which "
                   "contains vkGetPhysicalDeviceFeatures2 that can be used instead.\nSee more information about this legacy in the "
                   "specification: https://docs.vulkan.org/spec/latest/appendices/legacy.html#legacy-gpdp2");
    } else if (IsExtEnabled(extensions.vk_khr_get_physical_device_properties2)) {
        reported = true;
        LogWarning("WARNING-legacy-gpdp2", physicalDevice, error_obj.location,
                   "vkGetPhysicalDeviceFeatures is a legacy command and this VkInstance enabled the "
                   "VK_KHR_get_physical_device_properties2 extension which contains vkGetPhysicalDeviceFeatures2KHR that can be "
                   "used instead.\nSee more information about this legacy in the specification: "
                   "https://docs.vulkan.org/spec/latest/appendices/legacy.html#legacy-gpdp2");
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
        LogWarning("WARNING-legacy-gpdp2", physicalDevice, error_obj.location,
                   "vkGetPhysicalDeviceFormatProperties is a legacy command and this VkInstance was created with VK_VERSION_1_1 "
                   "which contains vkGetPhysicalDeviceFormatProperties2 that can be used instead.\nSee more information about this "
                   "legacy in the specification: https://docs.vulkan.org/spec/latest/appendices/legacy.html#legacy-gpdp2");
    } else if (IsExtEnabled(extensions.vk_khr_get_physical_device_properties2)) {
        reported = true;
        LogWarning("WARNING-legacy-gpdp2", physicalDevice, error_obj.location,
                   "vkGetPhysicalDeviceFormatProperties is a legacy command and this VkInstance enabled the "
                   "VK_KHR_get_physical_device_properties2 extension which contains vkGetPhysicalDeviceFormatProperties2KHR that "
                   "can be used instead.\nSee more information about this legacy in the specification: "
                   "https://docs.vulkan.org/spec/latest/appendices/legacy.html#legacy-gpdp2");
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
            "WARNING-legacy-gpdp2", physicalDevice, error_obj.location,
            "vkGetPhysicalDeviceImageFormatProperties is a legacy command and this VkInstance was created with VK_VERSION_1_1 "
            "which contains vkGetPhysicalDeviceImageFormatProperties2 that can be used instead.\nSee more information about this "
            "legacy in the specification: https://docs.vulkan.org/spec/latest/appendices/legacy.html#legacy-gpdp2");
    } else if (IsExtEnabled(extensions.vk_khr_get_physical_device_properties2)) {
        reported = true;
        LogWarning("WARNING-legacy-gpdp2", physicalDevice, error_obj.location,
                   "vkGetPhysicalDeviceImageFormatProperties is a legacy command and this VkInstance enabled the "
                   "VK_KHR_get_physical_device_properties2 extension which contains vkGetPhysicalDeviceImageFormatProperties2KHR "
                   "that can be used instead.\nSee more information about this legacy in the specification: "
                   "https://docs.vulkan.org/spec/latest/appendices/legacy.html#legacy-gpdp2");
    }
    return false;
}

bool Instance::PreCallValidateGetPhysicalDeviceProperties(VkPhysicalDevice physicalDevice, VkPhysicalDeviceProperties* pProperties,
                                                          const ErrorObject& error_obj) const {
    static bool reported = false;
    if (reported) return false;

    if (api_version >= VK_API_VERSION_1_1) {
        reported = true;
        LogWarning("WARNING-legacy-gpdp2", physicalDevice, error_obj.location,
                   "vkGetPhysicalDeviceProperties is a legacy command and this VkInstance was created with VK_VERSION_1_1 which "
                   "contains vkGetPhysicalDeviceProperties2 that can be used instead.\nSee more information about this legacy in "
                   "the specification: https://docs.vulkan.org/spec/latest/appendices/legacy.html#legacy-gpdp2");
    } else if (IsExtEnabled(extensions.vk_khr_get_physical_device_properties2)) {
        reported = true;
        LogWarning("WARNING-legacy-gpdp2", physicalDevice, error_obj.location,
                   "vkGetPhysicalDeviceProperties is a legacy command and this VkInstance enabled the "
                   "VK_KHR_get_physical_device_properties2 extension which contains vkGetPhysicalDeviceProperties2KHR that can be "
                   "used instead.\nSee more information about this legacy in the specification: "
                   "https://docs.vulkan.org/spec/latest/appendices/legacy.html#legacy-gpdp2");
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
            "WARNING-legacy-gpdp2", physicalDevice, error_obj.location,
            "vkGetPhysicalDeviceQueueFamilyProperties is a legacy command and this VkInstance was created with VK_VERSION_1_1 "
            "which contains vkGetPhysicalDeviceQueueFamilyProperties2 that can be used instead.\nSee more information about this "
            "legacy in the specification: https://docs.vulkan.org/spec/latest/appendices/legacy.html#legacy-gpdp2");
    } else if (IsExtEnabled(extensions.vk_khr_get_physical_device_properties2)) {
        reported = true;
        LogWarning("WARNING-legacy-gpdp2", physicalDevice, error_obj.location,
                   "vkGetPhysicalDeviceQueueFamilyProperties is a legacy command and this VkInstance enabled the "
                   "VK_KHR_get_physical_device_properties2 extension which contains vkGetPhysicalDeviceQueueFamilyProperties2KHR "
                   "that can be used instead.\nSee more information about this legacy in the specification: "
                   "https://docs.vulkan.org/spec/latest/appendices/legacy.html#legacy-gpdp2");
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
        LogWarning("WARNING-legacy-gpdp2", physicalDevice, error_obj.location,
                   "vkGetPhysicalDeviceMemoryProperties is a legacy command and this VkInstance was created with VK_VERSION_1_1 "
                   "which contains vkGetPhysicalDeviceMemoryProperties2 that can be used instead.\nSee more information about this "
                   "legacy in the specification: https://docs.vulkan.org/spec/latest/appendices/legacy.html#legacy-gpdp2");
    } else if (IsExtEnabled(extensions.vk_khr_get_physical_device_properties2)) {
        reported = true;
        LogWarning("WARNING-legacy-gpdp2", physicalDevice, error_obj.location,
                   "vkGetPhysicalDeviceMemoryProperties is a legacy command and this VkInstance enabled the "
                   "VK_KHR_get_physical_device_properties2 extension which contains vkGetPhysicalDeviceMemoryProperties2KHR that "
                   "can be used instead.\nSee more information about this legacy in the specification: "
                   "https://docs.vulkan.org/spec/latest/appendices/legacy.html#legacy-gpdp2");
    }
    return false;
}

bool Instance::PreCallValidateEnumerateDeviceLayerProperties(VkPhysicalDevice physicalDevice, uint32_t* pPropertyCount,
                                                             VkLayerProperties* pProperties, const ErrorObject& error_obj) const {
    static bool reported = false;
    if (reported) return false;

    LogWarning("WARNING-legacy-devicelayers", physicalDevice, error_obj.location,
               "vkEnumerateDeviceLayerProperties is a legacy command.\nSee more information about this superseding in the "
               "specification: https://docs.vulkan.org/spec/latest/appendices/legacy.html#legacy-devicelayers");

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
        LogWarning("WARNING-legacy-gpdp2", physicalDevice, error_obj.location,
                   "vkGetPhysicalDeviceSparseImageFormatProperties is a legacy command and this VkInstance was created with "
                   "VK_VERSION_1_1 which contains vkGetPhysicalDeviceSparseImageFormatProperties2 that can be used instead.\nSee "
                   "more information about this legacy in the specification: "
                   "https://docs.vulkan.org/spec/latest/appendices/legacy.html#legacy-gpdp2");
    } else if (IsExtEnabled(extensions.vk_khr_get_physical_device_properties2)) {
        reported = true;
        LogWarning("WARNING-legacy-gpdp2", physicalDevice, error_obj.location,
                   "vkGetPhysicalDeviceSparseImageFormatProperties is a legacy command and this VkInstance enabled the "
                   "VK_KHR_get_physical_device_properties2 extension which contains "
                   "vkGetPhysicalDeviceSparseImageFormatProperties2KHR that can be used instead.\nSee more information about this "
                   "legacy in the specification: https://docs.vulkan.org/spec/latest/appendices/legacy.html#legacy-gpdp2");
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
        LogWarning("WARNING-legacy-dynamicrendering", device, error_obj.location,
                   "vkCreateFramebuffer is a legacy command and this VkDevice was created with VK_VERSION_1_4 which contains the "
                   "new feature to replace it.\nSee more information about this legacy in the specification: "
                   "https://docs.vulkan.org/spec/latest/appendices/legacy.html#legacy-dynamicrendering");
    } else if (IsExtEnabled(extensions.vk_khr_dynamic_rendering_local_read)) {
        reported = true;
        LogWarning("WARNING-legacy-dynamicrendering", device, error_obj.location,
                   "vkCreateFramebuffer is a legacy command and this VkDevice enabled the VK_KHR_dynamic_rendering_local_read "
                   "extension which contains the new feature to replace it.\nSee more information about this legacy in the "
                   "specification: https://docs.vulkan.org/spec/latest/appendices/legacy.html#legacy-dynamicrendering");
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
        LogWarning("WARNING-legacy-renderpass2", device, error_obj.location,
                   "vkCreateRenderPass is a legacy command and this VkDevice was created with VK_VERSION_1_2 which contains "
                   "vkCreateRenderPass2 that can be used instead.\nSee more information about this legacy in the specification: "
                   "https://docs.vulkan.org/spec/latest/appendices/legacy.html#legacy-renderpass2");
    } else if (IsExtEnabled(extensions.vk_khr_create_renderpass2)) {
        reported = true;
        LogWarning("WARNING-legacy-renderpass2", device, error_obj.location,
                   "vkCreateRenderPass is a legacy command and this VkDevice enabled the VK_KHR_create_renderpass2 extension which "
                   "contains vkCreateRenderPass2KHR that can be used instead.\nSee more information about this legacy in the "
                   "specification: https://docs.vulkan.org/spec/latest/appendices/legacy.html#legacy-renderpass2");
    }
    return false;
}

bool Device::PreCallValidateGetRenderAreaGranularity(VkDevice device, VkRenderPass renderPass, VkExtent2D* pGranularity,
                                                     const ErrorObject& error_obj) const {
    static bool reported = false;
    if (reported) return false;

    if (api_version >= VK_API_VERSION_1_4) {
        reported = true;
        LogWarning("WARNING-legacy-dynamicrendering", device, error_obj.location,
                   "vkGetRenderAreaGranularity is a legacy command and this VkDevice was created with VK_VERSION_1_4 which "
                   "contains the new feature to replace it.\nSee more information about this legacy in the specification: "
                   "https://docs.vulkan.org/spec/latest/appendices/legacy.html#legacy-dynamicrendering");
    } else if (IsExtEnabled(extensions.vk_khr_dynamic_rendering_local_read)) {
        reported = true;
        LogWarning(
            "WARNING-legacy-dynamicrendering", device, error_obj.location,
            "vkGetRenderAreaGranularity is a legacy command and this VkDevice enabled the VK_KHR_dynamic_rendering_local_read "
            "extension which contains the new feature to replace it.\nSee more information about this legacy in the specification: "
            "https://docs.vulkan.org/spec/latest/appendices/legacy.html#legacy-dynamicrendering");
    }
    return false;
}

bool Device::PreCallValidateCmdBeginRenderPass(VkCommandBuffer commandBuffer, const VkRenderPassBeginInfo* pRenderPassBegin,
                                               VkSubpassContents contents, const ErrorObject& error_obj) const {
    static bool reported = false;
    if (reported) return false;

    if (api_version >= VK_API_VERSION_1_2) {
        reported = true;
        LogWarning("WARNING-legacy-renderpass2", device, error_obj.location,
                   "vkCmdBeginRenderPass is a legacy command and this VkDevice was created with VK_VERSION_1_2 which contains "
                   "vkCmdBeginRenderPass2 that can be used instead.\nSee more information about this legacy in the specification: "
                   "https://docs.vulkan.org/spec/latest/appendices/legacy.html#legacy-renderpass2");
    } else if (IsExtEnabled(extensions.vk_khr_create_renderpass2)) {
        reported = true;
        LogWarning("WARNING-legacy-renderpass2", device, error_obj.location,
                   "vkCmdBeginRenderPass is a legacy command and this VkDevice enabled the VK_KHR_create_renderpass2 extension "
                   "which contains vkCmdBeginRenderPass2KHR that can be used instead.\nSee more information about this legacy in "
                   "the specification: https://docs.vulkan.org/spec/latest/appendices/legacy.html#legacy-renderpass2");
    }
    return false;
}

bool Device::PreCallValidateCmdNextSubpass(VkCommandBuffer commandBuffer, VkSubpassContents contents,
                                           const ErrorObject& error_obj) const {
    static bool reported = false;
    if (reported) return false;

    if (api_version >= VK_API_VERSION_1_2) {
        reported = true;
        LogWarning("WARNING-legacy-renderpass2", device, error_obj.location,
                   "vkCmdNextSubpass is a legacy command and this VkDevice was created with VK_VERSION_1_2 which contains "
                   "vkCmdNextSubpass2 that can be used instead.\nSee more information about this legacy in the specification: "
                   "https://docs.vulkan.org/spec/latest/appendices/legacy.html#legacy-renderpass2");
    } else if (IsExtEnabled(extensions.vk_khr_create_renderpass2)) {
        reported = true;
        LogWarning("WARNING-legacy-renderpass2", device, error_obj.location,
                   "vkCmdNextSubpass is a legacy command and this VkDevice enabled the VK_KHR_create_renderpass2 extension which "
                   "contains vkCmdNextSubpass2KHR that can be used instead.\nSee more information about this legacy in the "
                   "specification: https://docs.vulkan.org/spec/latest/appendices/legacy.html#legacy-renderpass2");
    }
    return false;
}

bool Device::PreCallValidateCmdEndRenderPass(VkCommandBuffer commandBuffer, const ErrorObject& error_obj) const {
    static bool reported = false;
    if (reported) return false;

    if (api_version >= VK_API_VERSION_1_2) {
        reported = true;
        LogWarning("WARNING-legacy-renderpass2", device, error_obj.location,
                   "vkCmdEndRenderPass is a legacy command and this VkDevice was created with VK_VERSION_1_2 which contains "
                   "vkCmdEndRenderPass2 that can be used instead.\nSee more information about this legacy in the specification: "
                   "https://docs.vulkan.org/spec/latest/appendices/legacy.html#legacy-renderpass2");
    } else if (IsExtEnabled(extensions.vk_khr_create_renderpass2)) {
        reported = true;
        LogWarning("WARNING-legacy-renderpass2", device, error_obj.location,
                   "vkCmdEndRenderPass is a legacy command and this VkDevice enabled the VK_KHR_create_renderpass2 extension which "
                   "contains vkCmdEndRenderPass2KHR that can be used instead.\nSee more information about this legacy in the "
                   "specification: https://docs.vulkan.org/spec/latest/appendices/legacy.html#legacy-renderpass2");
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
        LogWarning("WARNING-legacy-dynamicrendering", device, error_obj.location,
                   "vkCreateRenderPass2 is a legacy command and this VkDevice was created with VK_VERSION_1_4 which contains the "
                   "new feature to replace it.\nSee more information about this legacy in the specification: "
                   "https://docs.vulkan.org/spec/latest/appendices/legacy.html#legacy-dynamicrendering");
    } else if (IsExtEnabled(extensions.vk_khr_dynamic_rendering_local_read)) {
        reported = true;
        LogWarning("WARNING-legacy-dynamicrendering", device, error_obj.location,
                   "vkCreateRenderPass2 is a legacy command and this VkDevice enabled the VK_KHR_dynamic_rendering_local_read "
                   "extension which contains the new feature to replace it.\nSee more information about this legacy in the "
                   "specification: https://docs.vulkan.org/spec/latest/appendices/legacy.html#legacy-dynamicrendering");
    }
    return false;
}

bool Device::PreCallValidateCmdBeginRenderPass2(VkCommandBuffer commandBuffer, const VkRenderPassBeginInfo* pRenderPassBegin,
                                                const VkSubpassBeginInfo* pSubpassBeginInfo, const ErrorObject& error_obj) const {
    static bool reported = false;
    if (reported) return false;

    if (api_version >= VK_API_VERSION_1_4) {
        reported = true;
        LogWarning("WARNING-legacy-dynamicrendering", device, error_obj.location,
                   "vkCmdBeginRenderPass2 is a legacy command and this VkDevice was created with VK_VERSION_1_4 which contains the "
                   "new feature to replace it.\nSee more information about this legacy in the specification: "
                   "https://docs.vulkan.org/spec/latest/appendices/legacy.html#legacy-dynamicrendering");
    } else if (IsExtEnabled(extensions.vk_khr_dynamic_rendering_local_read)) {
        reported = true;
        LogWarning("WARNING-legacy-dynamicrendering", device, error_obj.location,
                   "vkCmdBeginRenderPass2 is a legacy command and this VkDevice enabled the VK_KHR_dynamic_rendering_local_read "
                   "extension which contains the new feature to replace it.\nSee more information about this legacy in the "
                   "specification: https://docs.vulkan.org/spec/latest/appendices/legacy.html#legacy-dynamicrendering");
    }
    return false;
}

bool Device::PreCallValidateCmdNextSubpass2(VkCommandBuffer commandBuffer, const VkSubpassBeginInfo* pSubpassBeginInfo,
                                            const VkSubpassEndInfo* pSubpassEndInfo, const ErrorObject& error_obj) const {
    static bool reported = false;
    if (reported) return false;

    if (api_version >= VK_API_VERSION_1_4) {
        reported = true;
        LogWarning("WARNING-legacy-dynamicrendering", device, error_obj.location,
                   "vkCmdNextSubpass2 is a legacy command and this VkDevice was created with VK_VERSION_1_4 which contains the new "
                   "feature to replace it.\nSee more information about this legacy in the specification: "
                   "https://docs.vulkan.org/spec/latest/appendices/legacy.html#legacy-dynamicrendering");
    } else if (IsExtEnabled(extensions.vk_khr_dynamic_rendering_local_read)) {
        reported = true;
        LogWarning("WARNING-legacy-dynamicrendering", device, error_obj.location,
                   "vkCmdNextSubpass2 is a legacy command and this VkDevice enabled the VK_KHR_dynamic_rendering_local_read "
                   "extension which contains the new feature to replace it.\nSee more information about this legacy in the "
                   "specification: https://docs.vulkan.org/spec/latest/appendices/legacy.html#legacy-dynamicrendering");
    }
    return false;
}

bool Device::PreCallValidateCmdEndRenderPass2(VkCommandBuffer commandBuffer, const VkSubpassEndInfo* pSubpassEndInfo,
                                              const ErrorObject& error_obj) const {
    static bool reported = false;
    if (reported) return false;

    if (api_version >= VK_API_VERSION_1_4) {
        reported = true;
        LogWarning("WARNING-legacy-dynamicrendering", device, error_obj.location,
                   "vkCmdEndRenderPass2 is a legacy command and this VkDevice was created with VK_VERSION_1_4 which contains the "
                   "new feature to replace it.\nSee more information about this legacy in the specification: "
                   "https://docs.vulkan.org/spec/latest/appendices/legacy.html#legacy-dynamicrendering");
    } else if (IsExtEnabled(extensions.vk_khr_dynamic_rendering_local_read)) {
        reported = true;
        LogWarning("WARNING-legacy-dynamicrendering", device, error_obj.location,
                   "vkCmdEndRenderPass2 is a legacy command and this VkDevice enabled the VK_KHR_dynamic_rendering_local_read "
                   "extension which contains the new feature to replace it.\nSee more information about this legacy in the "
                   "specification: https://docs.vulkan.org/spec/latest/appendices/legacy.html#legacy-dynamicrendering");
    }
    return false;
}

ExtensionData GetExtensionData(vvl::Extension extension_name) {
    static const ExtensionData empty_data{Reason::Empty, vvl::Extension::Empty};
    static const vvl::unordered_map<vvl::Extension, ExtensionData> legacy_extensions = {
        {vvl::Extension::_VK_KHR_sampler_mirror_clamp_to_edge, {Reason::Promoted, {vvl::Version::_VK_VERSION_1_2}}},
        {vvl::Extension::_VK_KHR_dynamic_rendering, {Reason::Promoted, {vvl::Version::_VK_VERSION_1_3}}},
        {vvl::Extension::_VK_KHR_multiview, {Reason::Promoted, {vvl::Version::_VK_VERSION_1_1}}},
        {vvl::Extension::_VK_KHR_get_physical_device_properties2, {Reason::Promoted, {vvl::Version::_VK_VERSION_1_1}}},
        {vvl::Extension::_VK_KHR_device_group, {Reason::Promoted, {vvl::Version::_VK_VERSION_1_1}}},
        {vvl::Extension::_VK_KHR_shader_draw_parameters, {Reason::Promoted, {vvl::Version::_VK_VERSION_1_1}}},
        {vvl::Extension::_VK_KHR_maintenance1, {Reason::Promoted, {vvl::Version::_VK_VERSION_1_1}}},
        {vvl::Extension::_VK_KHR_device_group_creation, {Reason::Promoted, {vvl::Version::_VK_VERSION_1_1}}},
        {vvl::Extension::_VK_KHR_external_memory_capabilities, {Reason::Promoted, {vvl::Version::_VK_VERSION_1_1}}},
        {vvl::Extension::_VK_KHR_external_memory, {Reason::Promoted, {vvl::Version::_VK_VERSION_1_1}}},
        {vvl::Extension::_VK_KHR_external_semaphore_capabilities, {Reason::Promoted, {vvl::Version::_VK_VERSION_1_1}}},
        {vvl::Extension::_VK_KHR_external_semaphore, {Reason::Promoted, {vvl::Version::_VK_VERSION_1_1}}},
        {vvl::Extension::_VK_KHR_push_descriptor, {Reason::Promoted, {vvl::Version::_VK_VERSION_1_4}}},
        {vvl::Extension::_VK_KHR_shader_float16_int8, {Reason::Promoted, {vvl::Version::_VK_VERSION_1_2}}},
        {vvl::Extension::_VK_KHR_16bit_storage, {Reason::Promoted, {vvl::Version::_VK_VERSION_1_1}}},
        {vvl::Extension::_VK_KHR_descriptor_update_template, {Reason::Promoted, {vvl::Version::_VK_VERSION_1_1}}},
        {vvl::Extension::_VK_KHR_imageless_framebuffer, {Reason::Promoted, {vvl::Version::_VK_VERSION_1_2}}},
        {vvl::Extension::_VK_KHR_create_renderpass2, {Reason::Promoted, {vvl::Version::_VK_VERSION_1_2}}},
        {vvl::Extension::_VK_KHR_external_fence_capabilities, {Reason::Promoted, {vvl::Version::_VK_VERSION_1_1}}},
        {vvl::Extension::_VK_KHR_external_fence, {Reason::Promoted, {vvl::Version::_VK_VERSION_1_1}}},
        {vvl::Extension::_VK_KHR_maintenance2, {Reason::Promoted, {vvl::Version::_VK_VERSION_1_1}}},
        {vvl::Extension::_VK_KHR_variable_pointers, {Reason::Promoted, {vvl::Version::_VK_VERSION_1_1}}},
        {vvl::Extension::_VK_KHR_dedicated_allocation, {Reason::Promoted, {vvl::Version::_VK_VERSION_1_1}}},
        {vvl::Extension::_VK_KHR_storage_buffer_storage_class, {Reason::Promoted, {vvl::Version::_VK_VERSION_1_1}}},
        {vvl::Extension::_VK_KHR_relaxed_block_layout, {Reason::Promoted, {vvl::Version::_VK_VERSION_1_1}}},
        {vvl::Extension::_VK_KHR_get_memory_requirements2, {Reason::Promoted, {vvl::Version::_VK_VERSION_1_1}}},
        {vvl::Extension::_VK_KHR_image_format_list, {Reason::Promoted, {vvl::Version::_VK_VERSION_1_2}}},
        {vvl::Extension::_VK_KHR_sampler_ycbcr_conversion, {Reason::Promoted, {vvl::Version::_VK_VERSION_1_1}}},
        {vvl::Extension::_VK_KHR_bind_memory2, {Reason::Promoted, {vvl::Version::_VK_VERSION_1_1}}},
        {vvl::Extension::_VK_KHR_maintenance3, {Reason::Promoted, {vvl::Version::_VK_VERSION_1_1}}},
        {vvl::Extension::_VK_KHR_draw_indirect_count, {Reason::Promoted, {vvl::Version::_VK_VERSION_1_2}}},
        {vvl::Extension::_VK_KHR_shader_subgroup_extended_types, {Reason::Promoted, {vvl::Version::_VK_VERSION_1_2}}},
        {vvl::Extension::_VK_KHR_8bit_storage, {Reason::Promoted, {vvl::Version::_VK_VERSION_1_2}}},
        {vvl::Extension::_VK_KHR_shader_atomic_int64, {Reason::Promoted, {vvl::Version::_VK_VERSION_1_2}}},
        {vvl::Extension::_VK_KHR_global_priority, {Reason::Promoted, {vvl::Version::_VK_VERSION_1_4}}},
        {vvl::Extension::_VK_KHR_driver_properties, {Reason::Promoted, {vvl::Version::_VK_VERSION_1_2}}},
        {vvl::Extension::_VK_KHR_shader_float_controls, {Reason::Promoted, {vvl::Version::_VK_VERSION_1_2}}},
        {vvl::Extension::_VK_KHR_depth_stencil_resolve, {Reason::Promoted, {vvl::Version::_VK_VERSION_1_2}}},
        {vvl::Extension::_VK_KHR_timeline_semaphore, {Reason::Promoted, {vvl::Version::_VK_VERSION_1_2}}},
        {vvl::Extension::_VK_KHR_vulkan_memory_model, {Reason::Promoted, {vvl::Version::_VK_VERSION_1_2}}},
        {vvl::Extension::_VK_KHR_shader_terminate_invocation, {Reason::Promoted, {vvl::Version::_VK_VERSION_1_3}}},
        {vvl::Extension::_VK_KHR_dynamic_rendering_local_read, {Reason::Promoted, {vvl::Version::_VK_VERSION_1_4}}},
        {vvl::Extension::_VK_KHR_spirv_1_4, {Reason::Promoted, {vvl::Version::_VK_VERSION_1_2}}},
        {vvl::Extension::_VK_KHR_separate_depth_stencil_layouts, {Reason::Promoted, {vvl::Version::_VK_VERSION_1_2}}},
        {vvl::Extension::_VK_KHR_uniform_buffer_standard_layout, {Reason::Promoted, {vvl::Version::_VK_VERSION_1_2}}},
        {vvl::Extension::_VK_KHR_buffer_device_address, {Reason::Promoted, {vvl::Version::_VK_VERSION_1_2}}},
        {vvl::Extension::_VK_KHR_map_memory2, {Reason::Promoted, {vvl::Version::_VK_VERSION_1_4}}},
        {vvl::Extension::_VK_KHR_shader_integer_dot_product, {Reason::Promoted, {vvl::Version::_VK_VERSION_1_3}}},
        {vvl::Extension::_VK_KHR_shader_non_semantic_info, {Reason::Promoted, {vvl::Version::_VK_VERSION_1_3}}},
        {vvl::Extension::_VK_KHR_synchronization2, {Reason::Promoted, {vvl::Version::_VK_VERSION_1_3}}},
        {vvl::Extension::_VK_KHR_zero_initialize_workgroup_memory, {Reason::Promoted, {vvl::Version::_VK_VERSION_1_3}}},
        {vvl::Extension::_VK_KHR_copy_commands2, {Reason::Promoted, {vvl::Version::_VK_VERSION_1_3}}},
        {vvl::Extension::_VK_KHR_format_feature_flags2, {Reason::Promoted, {vvl::Version::_VK_VERSION_1_3}}},
        {vvl::Extension::_VK_KHR_maintenance4, {Reason::Promoted, {vvl::Version::_VK_VERSION_1_3}}},
        {vvl::Extension::_VK_KHR_shader_subgroup_rotate, {Reason::Promoted, {vvl::Version::_VK_VERSION_1_4}}},
        {vvl::Extension::_VK_KHR_maintenance5, {Reason::Promoted, {vvl::Version::_VK_VERSION_1_4}}},
        {vvl::Extension::_VK_KHR_vertex_attribute_divisor, {Reason::Promoted, {vvl::Version::_VK_VERSION_1_4}}},
        {vvl::Extension::_VK_KHR_load_store_op_none, {Reason::Promoted, {vvl::Version::_VK_VERSION_1_4}}},
        {vvl::Extension::_VK_KHR_shader_float_controls2, {Reason::Promoted, {vvl::Version::_VK_VERSION_1_4}}},
        {vvl::Extension::_VK_KHR_index_type_uint8, {Reason::Promoted, {vvl::Version::_VK_VERSION_1_4}}},
        {vvl::Extension::_VK_KHR_line_rasterization, {Reason::Promoted, {vvl::Version::_VK_VERSION_1_4}}},
        {vvl::Extension::_VK_KHR_shader_expect_assume, {Reason::Promoted, {vvl::Version::_VK_VERSION_1_4}}},
        {vvl::Extension::_VK_KHR_maintenance6, {Reason::Promoted, {vvl::Version::_VK_VERSION_1_4}}},
        {vvl::Extension::_VK_EXT_debug_report, {Reason::Superseded, {vvl::Extension::_VK_EXT_debug_utils}}},
        {vvl::Extension::_VK_NV_glsl_shader, {Reason::Superseded, {vvl::Extension::Empty}}},
        {vvl::Extension::_VK_EXT_debug_marker, {Reason::Promoted, {vvl::Extension::_VK_EXT_debug_utils}}},
        {vvl::Extension::_VK_NV_dedicated_allocation, {Reason::Superseded, {vvl::Extension::_VK_KHR_dedicated_allocation}}},
        {vvl::Extension::_VK_AMD_draw_indirect_count, {Reason::Promoted, {vvl::Extension::_VK_KHR_draw_indirect_count}}},
        {vvl::Extension::_VK_AMD_negative_viewport_height, {Reason::Obsoleted, {vvl::Extension::_VK_KHR_maintenance1}}},
        {vvl::Extension::_VK_AMD_gpu_shader_half_float, {Reason::Superseded, {vvl::Extension::_VK_KHR_shader_float16_int8}}},
        {vvl::Extension::_VK_IMG_format_pvrtc, {Reason::Superseded, {vvl::Extension::Empty}}},
        {vvl::Extension::_VK_NV_external_memory_capabilities,
         {Reason::Superseded, {vvl::Extension::_VK_KHR_external_memory_capabilities}}},
        {vvl::Extension::_VK_NV_external_memory, {Reason::Superseded, {vvl::Extension::_VK_KHR_external_memory}}},
        {vvl::Extension::_VK_NV_external_memory_win32, {Reason::Superseded, {vvl::Extension::_VK_KHR_external_memory_win32}}},
        {vvl::Extension::_VK_NV_win32_keyed_mutex, {Reason::Promoted, {vvl::Extension::_VK_KHR_win32_keyed_mutex}}},
        {vvl::Extension::_VK_EXT_validation_flags, {Reason::Superseded, {vvl::Extension::_VK_EXT_layer_settings}}},
        {vvl::Extension::_VK_EXT_shader_subgroup_ballot, {Reason::Superseded, {vvl::Version::_VK_VERSION_1_2}}},
        {vvl::Extension::_VK_EXT_shader_subgroup_vote, {Reason::Superseded, {vvl::Version::_VK_VERSION_1_1}}},
        {vvl::Extension::_VK_EXT_texture_compression_astc_hdr, {Reason::Promoted, {vvl::Version::_VK_VERSION_1_3}}},
        {vvl::Extension::_VK_EXT_pipeline_robustness, {Reason::Promoted, {vvl::Version::_VK_VERSION_1_4}}},
        {vvl::Extension::_VK_MVK_ios_surface, {Reason::Superseded, {vvl::Extension::_VK_EXT_metal_surface}}},
        {vvl::Extension::_VK_MVK_macos_surface, {Reason::Superseded, {vvl::Extension::_VK_EXT_metal_surface}}},
        {vvl::Extension::_VK_EXT_sampler_filter_minmax, {Reason::Promoted, {vvl::Version::_VK_VERSION_1_2}}},
        {vvl::Extension::_VK_AMD_gpu_shader_int16, {Reason::Superseded, {vvl::Extension::_VK_KHR_shader_float16_int8}}},
        {vvl::Extension::_VK_EXT_inline_uniform_block, {Reason::Promoted, {vvl::Version::_VK_VERSION_1_3}}},
        {vvl::Extension::_VK_EXT_descriptor_indexing, {Reason::Promoted, {vvl::Version::_VK_VERSION_1_2}}},
        {vvl::Extension::_VK_EXT_shader_viewport_index_layer, {Reason::Promoted, {vvl::Version::_VK_VERSION_1_2}}},
        {vvl::Extension::_VK_NV_ray_tracing, {Reason::Superseded, {vvl::Extension::_VK_KHR_ray_tracing_pipeline}}},
        {vvl::Extension::_VK_QCOM_render_pass_shader_resolve, {Reason::Promoted, {vvl::Extension::_VK_EXT_custom_resolve}}},
        {vvl::Extension::_VK_EXT_global_priority, {Reason::Promoted, {vvl::Extension::_VK_KHR_global_priority}}},
        {vvl::Extension::_VK_EXT_calibrated_timestamps, {Reason::Promoted, {vvl::Extension::_VK_KHR_calibrated_timestamps}}},
        {vvl::Extension::_VK_EXT_vertex_attribute_divisor, {Reason::Promoted, {vvl::Extension::_VK_KHR_vertex_attribute_divisor}}},
        {vvl::Extension::_VK_EXT_pipeline_creation_feedback, {Reason::Promoted, {vvl::Version::_VK_VERSION_1_3}}},
        {vvl::Extension::_VK_NV_compute_shader_derivatives,
         {Reason::Promoted, {vvl::Extension::_VK_KHR_compute_shader_derivatives}}},
        {vvl::Extension::_VK_NV_fragment_shader_barycentric,
         {Reason::Promoted, {vvl::Extension::_VK_KHR_fragment_shader_barycentric}}},
        {vvl::Extension::_VK_EXT_scalar_block_layout, {Reason::Promoted, {vvl::Version::_VK_VERSION_1_2}}},
        {vvl::Extension::_VK_EXT_subgroup_size_control, {Reason::Promoted, {vvl::Version::_VK_VERSION_1_3}}},
        {vvl::Extension::_VK_EXT_buffer_device_address, {Reason::Superseded, {vvl::Extension::_VK_KHR_buffer_device_address}}},
        {vvl::Extension::_VK_EXT_tooling_info, {Reason::Promoted, {vvl::Version::_VK_VERSION_1_3}}},
        {vvl::Extension::_VK_EXT_separate_stencil_usage, {Reason::Promoted, {vvl::Version::_VK_VERSION_1_2}}},
        {vvl::Extension::_VK_EXT_validation_features, {Reason::Superseded, {vvl::Extension::_VK_EXT_layer_settings}}},
        {vvl::Extension::_VK_EXT_line_rasterization, {Reason::Promoted, {vvl::Extension::_VK_KHR_line_rasterization}}},
        {vvl::Extension::_VK_EXT_host_query_reset, {Reason::Promoted, {vvl::Version::_VK_VERSION_1_2}}},
        {vvl::Extension::_VK_EXT_index_type_uint8, {Reason::Promoted, {vvl::Extension::_VK_KHR_index_type_uint8}}},
        {vvl::Extension::_VK_EXT_extended_dynamic_state, {Reason::Promoted, {vvl::Version::_VK_VERSION_1_3}}},
        {vvl::Extension::_VK_EXT_host_image_copy, {Reason::Promoted, {vvl::Version::_VK_VERSION_1_4}}},
        {vvl::Extension::_VK_EXT_surface_maintenance1, {Reason::Promoted, {vvl::Extension::_VK_KHR_surface_maintenance1}}},
        {vvl::Extension::_VK_EXT_swapchain_maintenance1, {Reason::Promoted, {vvl::Extension::_VK_KHR_swapchain_maintenance1}}},
        {vvl::Extension::_VK_EXT_shader_demote_to_helper_invocation, {Reason::Promoted, {vvl::Version::_VK_VERSION_1_3}}},
        {vvl::Extension::_VK_EXT_texel_buffer_alignment, {Reason::Promoted, {vvl::Version::_VK_VERSION_1_3}}},
        {vvl::Extension::_VK_EXT_robustness2, {Reason::Promoted, {vvl::Extension::_VK_KHR_robustness2}}},
        {vvl::Extension::_VK_EXT_private_data, {Reason::Promoted, {vvl::Version::_VK_VERSION_1_3}}},
        {vvl::Extension::_VK_EXT_pipeline_creation_cache_control, {Reason::Promoted, {vvl::Version::_VK_VERSION_1_3}}},
        {vvl::Extension::_VK_EXT_ycbcr_2plane_444_formats, {Reason::Promoted, {vvl::Version::_VK_VERSION_1_3}}},
        {vvl::Extension::_VK_EXT_image_robustness, {Reason::Promoted, {vvl::Version::_VK_VERSION_1_3}}},
        {vvl::Extension::_VK_EXT_4444_formats, {Reason::Promoted, {vvl::Version::_VK_VERSION_1_3}}},
        {vvl::Extension::_VK_ARM_rasterization_order_attachment_access,
         {Reason::Promoted, {vvl::Extension::_VK_EXT_rasterization_order_attachment_access}}},
        {vvl::Extension::_VK_VALVE_mutable_descriptor_type, {Reason::Promoted, {vvl::Extension::_VK_EXT_mutable_descriptor_type}}},
        {vvl::Extension::_VK_EXT_present_mode_fifo_latest_ready,
         {Reason::Promoted, {vvl::Extension::_VK_KHR_present_mode_fifo_latest_ready}}},
        {vvl::Extension::_VK_EXT_extended_dynamic_state2, {Reason::Promoted, {vvl::Version::_VK_VERSION_1_3}}},
        {vvl::Extension::_VK_EXT_global_priority_query, {Reason::Promoted, {vvl::Extension::_VK_KHR_global_priority}}},
        {vvl::Extension::_VK_NV_displacement_micromap,
         {Reason::Superseded, {vvl::Extension::_VK_NV_cluster_acceleration_structure}}},
        {vvl::Extension::_VK_EXT_load_store_op_none, {Reason::Promoted, {vvl::Extension::_VK_KHR_load_store_op_none}}},
        {vvl::Extension::_VK_EXT_depth_clamp_zero_one, {Reason::Promoted, {vvl::Extension::_VK_KHR_depth_clamp_zero_one}}},
        {vvl::Extension::_VK_QCOM_fragment_density_map_offset,
         {Reason::Promoted, {vvl::Extension::_VK_EXT_fragment_density_map_offset}}},
        {vvl::Extension::_VK_NV_copy_memory_indirect, {Reason::Promoted, {vvl::Extension::_VK_KHR_copy_memory_indirect}}},
        {vvl::Extension::_VK_NV_memory_decompression, {Reason::Promoted, {vvl::Extension::_VK_EXT_memory_decompression}}},
        {vvl::Extension::_VK_EXT_pipeline_protected_access, {Reason::Promoted, {vvl::Version::_VK_VERSION_1_4}}},
        {vvl::Extension::_VK_NV_ray_tracing_invocation_reorder,
         {Reason::Promoted, {vvl::Extension::_VK_EXT_ray_tracing_invocation_reorder}}},
        {vvl::Extension::_VK_EXT_vertex_attribute_robustness, {Reason::Promoted, {vvl::Extension::_VK_KHR_maintenance9}}},
    };

    auto it = legacy_extensions.find(extension_name);
    return (it == legacy_extensions.end()) ? empty_data : it->second;
}
}  // namespace legacy
// NOLINTEND
