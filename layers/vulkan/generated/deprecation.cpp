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
#include "generated/dispatch_functions.h"

namespace deprecation {
void Device::FinishDeviceSetup(const VkDeviceCreateInfo* pCreateInfo, const Location& loc) {
    std::vector<VkExtensionProperties> ext_props{};
    uint32_t ext_count = 0;
    DispatchEnumerateDeviceExtensionProperties(physical_device, nullptr, &ext_count, nullptr);
    ext_props.resize(ext_count);
    DispatchEnumerateDeviceExtensionProperties(physical_device, nullptr, &ext_count, ext_props.data());
    for (const auto& prop : ext_props) {
        vvl::Extension extension = GetExtension(prop.extensionName);

        if (extension == vvl::Extension::_VK_KHR_dynamic_rendering_local_read) {
            supported_vk_khr_dynamic_rendering_local_read = true;
        }

        if (extension == vvl::Extension::_VK_KHR_create_renderpass2) {
            supported_vk_khr_create_renderpass2 = true;
        }
    }
}

bool Device::PreCallValidateCreateFramebuffer(VkDevice device, const VkFramebufferCreateInfo* pCreateInfo,
                                              const VkAllocationCallbacks* pAllocator, VkFramebuffer* pFramebuffer,
                                              const ErrorObject& error_obj) const {
    static bool reported = false;
    if (reported) return false;

    if (api_version >= VK_API_VERSION_1_4) {
        reported = true;
        LogWarning("WARNING-deprecation-dynamicrendering", device, error_obj.location,
                   "vkCreateFramebuffer is deprecated and this device supports VK_VERSION_1_4\nSee more information about this "
                   "deprecation in the specification: "
                   "https://docs.vulkan.org/spec/latest/appendices/deprecation.html#deprecation-dynamicrendering");
    } else if (supported_vk_khr_dynamic_rendering_local_read) {
        reported = true;
        LogWarning("WARNING-deprecation-dynamicrendering", device, error_obj.location,
                   "vkCreateFramebuffer is deprecated and this device supports VK_KHR_dynamic_rendering_local_read\nSee more "
                   "information about this deprecation in the specification: "
                   "https://docs.vulkan.org/spec/latest/appendices/deprecation.html#deprecation-dynamicrendering");
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
        LogWarning(
            "WARNING-deprecation-renderpass2", device, error_obj.location,
            "vkCreateRenderPass is deprecated and this device supports VK_VERSION_1_2\nSee more information about this deprecation "
            "in the specification: https://docs.vulkan.org/spec/latest/appendices/deprecation.html#deprecation-renderpass2");
    } else if (supported_vk_khr_create_renderpass2) {
        reported = true;
        LogWarning("WARNING-deprecation-renderpass2", device, error_obj.location,
                   "vkCreateRenderPass is deprecated and this device supports VK_KHR_create_renderpass2\nSee more information "
                   "about this deprecation in the specification: "
                   "https://docs.vulkan.org/spec/latest/appendices/deprecation.html#deprecation-renderpass2");
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
                   "vkGetRenderAreaGranularity is deprecated and this device supports VK_VERSION_1_4\nSee more information about "
                   "this deprecation in the specification: "
                   "https://docs.vulkan.org/spec/latest/appendices/deprecation.html#deprecation-dynamicrendering");
    } else if (supported_vk_khr_dynamic_rendering_local_read) {
        reported = true;
        LogWarning("WARNING-deprecation-dynamicrendering", device, error_obj.location,
                   "vkGetRenderAreaGranularity is deprecated and this device supports VK_KHR_dynamic_rendering_local_read\nSee "
                   "more information about this deprecation in the specification: "
                   "https://docs.vulkan.org/spec/latest/appendices/deprecation.html#deprecation-dynamicrendering");
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
                   "vkCmdBeginRenderPass is deprecated and this device supports VK_VERSION_1_2\nSee more information about this "
                   "deprecation in the specification: "
                   "https://docs.vulkan.org/spec/latest/appendices/deprecation.html#deprecation-renderpass2");
    } else if (supported_vk_khr_create_renderpass2) {
        reported = true;
        LogWarning("WARNING-deprecation-renderpass2", device, error_obj.location,
                   "vkCmdBeginRenderPass is deprecated and this device supports VK_KHR_create_renderpass2\nSee more information "
                   "about this deprecation in the specification: "
                   "https://docs.vulkan.org/spec/latest/appendices/deprecation.html#deprecation-renderpass2");
    }
    return false;
}

bool Device::PreCallValidateCmdNextSubpass(VkCommandBuffer commandBuffer, VkSubpassContents contents,
                                           const ErrorObject& error_obj) const {
    static bool reported = false;
    if (reported) return false;

    if (api_version >= VK_API_VERSION_1_2) {
        reported = true;
        LogWarning(
            "WARNING-deprecation-renderpass2", device, error_obj.location,
            "vkCmdNextSubpass is deprecated and this device supports VK_VERSION_1_2\nSee more information about this deprecation "
            "in the specification: https://docs.vulkan.org/spec/latest/appendices/deprecation.html#deprecation-renderpass2");
    } else if (supported_vk_khr_create_renderpass2) {
        reported = true;
        LogWarning("WARNING-deprecation-renderpass2", device, error_obj.location,
                   "vkCmdNextSubpass is deprecated and this device supports VK_KHR_create_renderpass2\nSee more information about "
                   "this deprecation in the specification: "
                   "https://docs.vulkan.org/spec/latest/appendices/deprecation.html#deprecation-renderpass2");
    }
    return false;
}

bool Device::PreCallValidateCmdEndRenderPass(VkCommandBuffer commandBuffer, const ErrorObject& error_obj) const {
    static bool reported = false;
    if (reported) return false;

    if (api_version >= VK_API_VERSION_1_2) {
        reported = true;
        LogWarning(
            "WARNING-deprecation-renderpass2", device, error_obj.location,
            "vkCmdEndRenderPass is deprecated and this device supports VK_VERSION_1_2\nSee more information about this deprecation "
            "in the specification: https://docs.vulkan.org/spec/latest/appendices/deprecation.html#deprecation-renderpass2");
    } else if (supported_vk_khr_create_renderpass2) {
        reported = true;
        LogWarning("WARNING-deprecation-renderpass2", device, error_obj.location,
                   "vkCmdEndRenderPass is deprecated and this device supports VK_KHR_create_renderpass2\nSee more information "
                   "about this deprecation in the specification: "
                   "https://docs.vulkan.org/spec/latest/appendices/deprecation.html#deprecation-renderpass2");
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
                   "vkCreateRenderPass2 is deprecated and this device supports VK_VERSION_1_4\nSee more information about this "
                   "deprecation in the specification: "
                   "https://docs.vulkan.org/spec/latest/appendices/deprecation.html#deprecation-dynamicrendering");
    } else if (supported_vk_khr_dynamic_rendering_local_read) {
        reported = true;
        LogWarning("WARNING-deprecation-dynamicrendering", device, error_obj.location,
                   "vkCreateRenderPass2 is deprecated and this device supports VK_KHR_dynamic_rendering_local_read\nSee more "
                   "information about this deprecation in the specification: "
                   "https://docs.vulkan.org/spec/latest/appendices/deprecation.html#deprecation-dynamicrendering");
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
                   "vkCmdBeginRenderPass2 is deprecated and this device supports VK_VERSION_1_4\nSee more information about this "
                   "deprecation in the specification: "
                   "https://docs.vulkan.org/spec/latest/appendices/deprecation.html#deprecation-dynamicrendering");
    } else if (supported_vk_khr_dynamic_rendering_local_read) {
        reported = true;
        LogWarning("WARNING-deprecation-dynamicrendering", device, error_obj.location,
                   "vkCmdBeginRenderPass2 is deprecated and this device supports VK_KHR_dynamic_rendering_local_read\nSee more "
                   "information about this deprecation in the specification: "
                   "https://docs.vulkan.org/spec/latest/appendices/deprecation.html#deprecation-dynamicrendering");
    }
    return false;
}

bool Device::PreCallValidateCmdNextSubpass2(VkCommandBuffer commandBuffer, const VkSubpassBeginInfo* pSubpassBeginInfo,
                                            const VkSubpassEndInfo* pSubpassEndInfo, const ErrorObject& error_obj) const {
    static bool reported = false;
    if (reported) return false;

    if (api_version >= VK_API_VERSION_1_4) {
        reported = true;
        LogWarning(
            "WARNING-deprecation-dynamicrendering", device, error_obj.location,
            "vkCmdNextSubpass2 is deprecated and this device supports VK_VERSION_1_4\nSee more information about this deprecation "
            "in the specification: https://docs.vulkan.org/spec/latest/appendices/deprecation.html#deprecation-dynamicrendering");
    } else if (supported_vk_khr_dynamic_rendering_local_read) {
        reported = true;
        LogWarning("WARNING-deprecation-dynamicrendering", device, error_obj.location,
                   "vkCmdNextSubpass2 is deprecated and this device supports VK_KHR_dynamic_rendering_local_read\nSee more "
                   "information about this deprecation in the specification: "
                   "https://docs.vulkan.org/spec/latest/appendices/deprecation.html#deprecation-dynamicrendering");
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
                   "vkCmdEndRenderPass2 is deprecated and this device supports VK_VERSION_1_4\nSee more information about this "
                   "deprecation in the specification: "
                   "https://docs.vulkan.org/spec/latest/appendices/deprecation.html#deprecation-dynamicrendering");
    } else if (supported_vk_khr_dynamic_rendering_local_read) {
        reported = true;
        LogWarning("WARNING-deprecation-dynamicrendering", device, error_obj.location,
                   "vkCmdEndRenderPass2 is deprecated and this device supports VK_KHR_dynamic_rendering_local_read\nSee more "
                   "information about this deprecation in the specification: "
                   "https://docs.vulkan.org/spec/latest/appendices/deprecation.html#deprecation-dynamicrendering");
    }
    return false;
}
}  // namespace deprecation
// NOLINTEND
