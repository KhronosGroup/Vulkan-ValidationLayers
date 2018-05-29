/*
 *
 * Copyright (c) 2016-2018 Valve Corporation
 * Copyright (c) 2016-2018 LunarG, Inc.
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
 * Author: William Henning <whenning@google.com>
 *
 */

#ifndef DEVICE_PROFILE_API_H_
#define DEVICE_PROFILE_API_H_

#include <unordered_map>
#include "vulkan/vulkan.h"
#include "vk_layer_config.h"

#ifdef __cplusplus
extern "C" {
#endif  // __cplusplus

// API functions
typedef void(VKAPI_PTR *PFN_vkGetOriginalPhysicalDeviceLimitsEXT)(VkPhysicalDevice physicalDevice,
                                                                  const VkPhysicalDeviceLimits *limits);
typedef void(VKAPI_PTR *PFN_vkSetPhysicalDeviceLimitsEXT)(VkPhysicalDevice physicalDevice, const VkPhysicalDeviceLimits *newLimits);
typedef void(VKAPI_PTR *PFN_vkGetOriginalPhysicalDeviceFormatPropertiesEXT)(VkPhysicalDevice physicalDevice, VkFormat format,
                                                                            VkFormatProperties *properties);
typedef void(VKAPI_PTR *PFN_vkSetPhysicalDeviceFormatPropertiesEXT)(VkPhysicalDevice physicalDevice, VkFormat format,
                                                                    const VkFormatProperties newProperties);
typedef VkResult(VKAPI_PTR *PFN_vkGetOriginalPhysicalDeviceImageFormatPropertiesEXT)(
    VkPhysicalDevice physicalDevice, VkFormat format, VkImageType type, VkImageTiling tiling, VkImageUsageFlags usage,
    VkImageCreateFlags flags, VkImageFormatProperties *pImageFormatProperties);
typedef void(VKAPI_PTR *PFN_vkSetPhysicalDeviceImageFormatPropertiesEXT)(VkPhysicalDevice physicalDevice, VkFormat format,
                                                                         VkImageTiling tiling, const VkResult new_result,
                                                                         const VkImageFormatProperties new_properties);
typedef void(VKAPI_PTR *PFN_vkGetOriginalImageMemoryRequirementsEXT)(VkDevice device, VkImage image,
                                                                     VkMemoryRequirements *pMemoryRequirements);
typedef void(VKAPI_PTR *PFN_vkSetImageMemoryRequirementsEXT)(VkDevice device, VkImage image,
                                                             const VkMemoryRequirements new_requirements);

#ifdef __cplusplus
}  // extern "C"
#endif  // __cplusplus

#endif  // DEVICE_PROFILE_API_H_
