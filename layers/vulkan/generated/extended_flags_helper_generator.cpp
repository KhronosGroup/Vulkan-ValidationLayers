// *** THIS FILE IS GENERATED - DO NOT EDIT ***
// See extended_flags_helper_generator.py for modifications

/***************************************************************************
 *
 * Copyright (c) 2015-2026 The Khronos Group Inc.
 * Copyright (c) 2015-2026 Valve Corporation
 * Copyright (c) 2015-2026 LunarG, Inc.
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

#include "extended_flags_helper_generator.h"

#include <vulkan/utility/vk_struct_helper.hpp>
#include "error_message/error_location.h"

VkBufferUsageFlags GetBufferUsageFlags(const VkBufferCreateInfo& create_info) {
    return static_cast<VkBufferUsageFlags>(create_info.usage);
}

Location GetBufferCreateInfoUsageLocation(const void* pNext, const Location& loc) { return loc.dot(vvl::Field::usage); }

VkImageCreateFlags GetImageCreateFlags(const VkImageCreateInfo& create_info) {
    return static_cast<VkImageCreateFlags>(create_info.flags);
}

Location GetImageCreateInfoFlagsLocation(const void* pNext, const Location& loc) { return loc.dot(vvl::Field::flags); }

VkImageUsageFlags GetImageUsageFlags(const VkImageCreateInfo& create_info) {
    return static_cast<VkImageUsageFlags>(create_info.usage);
}

Location GetImageCreateInfoUsageLocation(const void* pNext, const Location& loc) { return loc.dot(vvl::Field::usage); }

VkPipelineCreateFlags GetPipelineCreateFlags(const VkComputePipelineCreateInfo& create_info) {
    return static_cast<VkPipelineCreateFlags>(create_info.flags);
}

Location GetComputePipelineCreateInfoFlagsLocation(const void* pNext, const Location& loc) { return loc.dot(vvl::Field::flags); }

VkPipelineCreateFlags GetPipelineCreateFlags(const VkGraphicsPipelineCreateInfo& create_info) {
    return static_cast<VkPipelineCreateFlags>(create_info.flags);
}

Location GetGraphicsPipelineCreateInfoFlagsLocation(const void* pNext, const Location& loc) { return loc.dot(vvl::Field::flags); }

VkImageUsageFlags GetImageUsageFlags(const VkPhysicalDeviceImageFormatInfo2& create_info) {
    return static_cast<VkImageUsageFlags>(create_info.usage);
}

Location GetPhysicalDeviceImageFormatInfo2UsageLocation(const void* pNext, const Location& loc) {
    return loc.dot(vvl::Field::usage);
}

VkImageCreateFlags GetImageCreateFlags(const VkPhysicalDeviceImageFormatInfo2& create_info) {
    return static_cast<VkImageCreateFlags>(create_info.flags);
}

Location GetPhysicalDeviceImageFormatInfo2FlagsLocation(const void* pNext, const Location& loc) {
    return loc.dot(vvl::Field::flags);
}

VkImageUsageFlags GetImageUsageFlags(const VkPhysicalDeviceSparseImageFormatInfo2& create_info) {
    return static_cast<VkImageUsageFlags>(create_info.usage);
}

Location GetPhysicalDeviceSparseImageFormatInfo2UsageLocation(const void* pNext, const Location& loc) {
    return loc.dot(vvl::Field::usage);
}

VkBufferUsageFlags GetBufferUsageFlags(const VkPhysicalDeviceExternalBufferInfo& create_info) {
    return static_cast<VkBufferUsageFlags>(create_info.usage);
}

Location GetPhysicalDeviceExternalBufferInfoUsageLocation(const void* pNext, const Location& loc) {
    return loc.dot(vvl::Field::usage);
}

VkImageCreateFlags GetImageCreateFlags(const VkFramebufferAttachmentImageInfo& create_info) {
    return static_cast<VkImageCreateFlags>(create_info.flags);
}

Location GetFramebufferAttachmentImageInfoFlagsLocation(const void* pNext, const Location& loc) {
    return loc.dot(vvl::Field::flags);
}

VkImageUsageFlags GetImageUsageFlags(const VkFramebufferAttachmentImageInfo& create_info) {
    return static_cast<VkImageUsageFlags>(create_info.usage);
}

Location GetFramebufferAttachmentImageInfoUsageLocation(const void* pNext, const Location& loc) {
    return loc.dot(vvl::Field::usage);
}

VkImageUsageFlags GetImageUsageFlags(const VkSwapchainCreateInfoKHR& create_info) {
    return static_cast<VkImageUsageFlags>(create_info.imageUsage);
}

Location GetSwapchainCreateInfoKHRImageUsageLocation(const void* pNext, const Location& loc) {
    return loc.dot(vvl::Field::imageUsage);
}

VkImageUsageFlags GetImageUsageFlags(const VkPhysicalDeviceVideoFormatInfoKHR& create_info) {
    return static_cast<VkImageUsageFlags>(create_info.imageUsage);
}

Location GetPhysicalDeviceVideoFormatInfoKHRImageUsageLocation(const void* pNext, const Location& loc) {
    return loc.dot(vvl::Field::imageUsage);
}

VkPipelineCreateFlags GetPipelineCreateFlags(const VkRayTracingPipelineCreateInfoNV& create_info) {
    return static_cast<VkPipelineCreateFlags>(create_info.flags);
}

Location GetRayTracingPipelineCreateInfoNVFlagsLocation(const void* pNext, const Location& loc) {
    return loc.dot(vvl::Field::flags);
}

VkDeviceAddress GetDeviceAddress(const VkDescriptorBufferBindingInfoEXT& create_info) {
    return static_cast<VkDeviceAddress>(create_info.address);
}

Location GetDescriptorBufferBindingInfoEXTAddressLocation(const void* pNext, const Location& loc) {
    return loc.dot(vvl::Field::address);
}

VkPipelineCreateFlags GetPipelineCreateFlags(const VkRayTracingPipelineCreateInfoKHR& create_info) {
    return static_cast<VkPipelineCreateFlags>(create_info.flags);
}

Location GetRayTracingPipelineCreateInfoKHRFlagsLocation(const void* pNext, const Location& loc) {
    return loc.dot(vvl::Field::flags);
}

// NOLINTEND
