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

VkBufferUsageFlags2 GetBufferCreateFlags(const VkBufferCreateInfo& create_info) {
    const auto extended = vku::FindStructInPNextChain<VkBufferUsageFlags2CreateInfo>(create_info.pNext);
    if (extended) {
        return extended->usage;
    }
    return static_cast<VkBufferUsageFlags2>(create_info.flags);
}

Location GetFlagsLocation(const VkBufferCreateInfo& create_info, const Location& loc) {
    const auto extended = vku::FindStructInPNextChain<VkBufferUsageFlags2CreateInfo>(create_info.pNext);
    if (extended) {
        return loc.pNext(vvl::Struct::VkBufferUsageFlags2CreateInfo).dot(vvl::Field::usage);
    }
    return loc.dot(vvl::Field::flags);
}

VkImageCreateFlags2KHR GetImageCreateFlags(const VkImageCreateInfo& create_info) {
    const auto extended = vku::FindStructInPNextChain<VkImageCreateFlags2CreateInfoKHR>(create_info.pNext);
    if (extended) {
        return extended->flags;
    }
    return static_cast<VkImageCreateFlags2KHR>(create_info.flags);
}

Location GetFlagsLocation(const VkImageCreateInfo& create_info, const Location& loc) {
    const auto extended = vku::FindStructInPNextChain<VkImageCreateFlags2CreateInfoKHR>(create_info.pNext);
    if (extended) {
        return loc.pNext(vvl::Struct::VkImageCreateFlags2CreateInfoKHR).dot(vvl::Field::flags);
    }
    return loc.dot(vvl::Field::flags);
}

VkImageUsageFlags2KHR GetImageUsageFlags(const VkImageCreateInfo& create_info) {
    const auto extended = vku::FindStructInPNextChain<VkImageUsageFlags2CreateInfoKHR>(create_info.pNext);
    if (extended) {
        return extended->usage;
    }
    return static_cast<VkImageUsageFlags2KHR>(create_info.usage);
}

Location GetUsageLocation(const VkImageCreateInfo& create_info, const Location& loc) {
    const auto extended = vku::FindStructInPNextChain<VkImageUsageFlags2CreateInfoKHR>(create_info.pNext);
    if (extended) {
        return loc.pNext(vvl::Struct::VkImageUsageFlags2CreateInfoKHR).dot(vvl::Field::usage);
    }
    return loc.dot(vvl::Field::usage);
}

VkBufferUsageFlags2 GetBufferViewCreateFlags(const VkBufferViewCreateInfo& create_info) {
    const auto extended = vku::FindStructInPNextChain<VkBufferUsageFlags2CreateInfo>(create_info.pNext);
    if (extended) {
        return extended->usage;
    }
    return static_cast<VkBufferUsageFlags2>(create_info.flags);
}

Location GetFlagsLocation(const VkBufferViewCreateInfo& create_info, const Location& loc) {
    const auto extended = vku::FindStructInPNextChain<VkBufferUsageFlags2CreateInfo>(create_info.pNext);
    if (extended) {
        return loc.pNext(vvl::Struct::VkBufferUsageFlags2CreateInfo).dot(vvl::Field::usage);
    }
    return loc.dot(vvl::Field::flags);
}

VkPipelineCreateFlags2 GetPipelineCreateFlags(const VkComputePipelineCreateInfo& create_info) {
    const auto extended = vku::FindStructInPNextChain<VkPipelineCreateFlags2CreateInfo>(create_info.pNext);
    if (extended) {
        return extended->flags;
    }
    return static_cast<VkPipelineCreateFlags2>(create_info.flags);
}

Location GetFlagsLocation(const VkComputePipelineCreateInfo& create_info, const Location& loc) {
    const auto extended = vku::FindStructInPNextChain<VkPipelineCreateFlags2CreateInfo>(create_info.pNext);
    if (extended) {
        return loc.pNext(vvl::Struct::VkPipelineCreateFlags2CreateInfo).dot(vvl::Field::flags);
    }
    return loc.dot(vvl::Field::flags);
}

VkPipelineCreateFlags2 GetPipelineCreateFlags(const VkGraphicsPipelineCreateInfo& create_info) {
    const auto extended = vku::FindStructInPNextChain<VkPipelineCreateFlags2CreateInfo>(create_info.pNext);
    if (extended) {
        return extended->flags;
    }
    return static_cast<VkPipelineCreateFlags2>(create_info.flags);
}

Location GetFlagsLocation(const VkGraphicsPipelineCreateInfo& create_info, const Location& loc) {
    const auto extended = vku::FindStructInPNextChain<VkPipelineCreateFlags2CreateInfo>(create_info.pNext);
    if (extended) {
        return loc.pNext(vvl::Struct::VkPipelineCreateFlags2CreateInfo).dot(vvl::Field::flags);
    }
    return loc.dot(vvl::Field::flags);
}

VkImageUsageFlags2KHR GetImageUsageFlags(const VkPhysicalDeviceImageFormatInfo2& create_info) {
    const auto extended = vku::FindStructInPNextChain<VkImageUsageFlags2CreateInfoKHR>(create_info.pNext);
    if (extended) {
        return extended->usage;
    }
    return static_cast<VkImageUsageFlags2KHR>(create_info.usage);
}

Location GetUsageLocation(const VkPhysicalDeviceImageFormatInfo2& create_info, const Location& loc) {
    const auto extended = vku::FindStructInPNextChain<VkImageUsageFlags2CreateInfoKHR>(create_info.pNext);
    if (extended) {
        return loc.pNext(vvl::Struct::VkImageUsageFlags2CreateInfoKHR).dot(vvl::Field::usage);
    }
    return loc.dot(vvl::Field::usage);
}

VkImageCreateFlags2KHR GetImageCreateFlags(const VkPhysicalDeviceImageFormatInfo2& create_info) {
    const auto extended = vku::FindStructInPNextChain<VkImageCreateFlags2CreateInfoKHR>(create_info.pNext);
    if (extended) {
        return extended->flags;
    }
    return static_cast<VkImageCreateFlags2KHR>(create_info.flags);
}

Location GetFlagsLocation(const VkPhysicalDeviceImageFormatInfo2& create_info, const Location& loc) {
    const auto extended = vku::FindStructInPNextChain<VkImageCreateFlags2CreateInfoKHR>(create_info.pNext);
    if (extended) {
        return loc.pNext(vvl::Struct::VkImageCreateFlags2CreateInfoKHR).dot(vvl::Field::flags);
    }
    return loc.dot(vvl::Field::flags);
}

VkImageUsageFlags2KHR GetImageUsageFlags(const VkPhysicalDeviceSparseImageFormatInfo2& create_info) {
    const auto extended = vku::FindStructInPNextChain<VkImageUsageFlags2CreateInfoKHR>(create_info.pNext);
    if (extended) {
        return extended->usage;
    }
    return static_cast<VkImageUsageFlags2KHR>(create_info.usage);
}

Location GetUsageLocation(const VkPhysicalDeviceSparseImageFormatInfo2& create_info, const Location& loc) {
    const auto extended = vku::FindStructInPNextChain<VkImageUsageFlags2CreateInfoKHR>(create_info.pNext);
    if (extended) {
        return loc.pNext(vvl::Struct::VkImageUsageFlags2CreateInfoKHR).dot(vvl::Field::usage);
    }
    return loc.dot(vvl::Field::usage);
}

VkBufferUsageFlags2 GetBufferUsageFlags(const VkPhysicalDeviceExternalBufferInfo& create_info) {
    const auto extended = vku::FindStructInPNextChain<VkBufferUsageFlags2CreateInfo>(create_info.pNext);
    if (extended) {
        return extended->usage;
    }
    return static_cast<VkBufferUsageFlags2>(create_info.usage);
}

Location GetUsageLocation(const VkPhysicalDeviceExternalBufferInfo& create_info, const Location& loc) {
    const auto extended = vku::FindStructInPNextChain<VkBufferUsageFlags2CreateInfo>(create_info.pNext);
    if (extended) {
        return loc.pNext(vvl::Struct::VkBufferUsageFlags2CreateInfo).dot(vvl::Field::usage);
    }
    return loc.dot(vvl::Field::usage);
}

VkImageCreateFlags2KHR GetImageCreateFlags(const VkFramebufferAttachmentImageInfo& create_info) {
    const auto extended = vku::FindStructInPNextChain<VkImageCreateFlags2CreateInfoKHR>(create_info.pNext);
    if (extended) {
        return extended->flags;
    }
    return static_cast<VkImageCreateFlags2KHR>(create_info.flags);
}

Location GetFlagsLocation(const VkFramebufferAttachmentImageInfo& create_info, const Location& loc) {
    const auto extended = vku::FindStructInPNextChain<VkImageCreateFlags2CreateInfoKHR>(create_info.pNext);
    if (extended) {
        return loc.pNext(vvl::Struct::VkImageCreateFlags2CreateInfoKHR).dot(vvl::Field::flags);
    }
    return loc.dot(vvl::Field::flags);
}

VkImageUsageFlags2KHR GetImageUsageFlags(const VkFramebufferAttachmentImageInfo& create_info) {
    const auto extended = vku::FindStructInPNextChain<VkImageUsageFlags2CreateInfoKHR>(create_info.pNext);
    if (extended) {
        return extended->usage;
    }
    return static_cast<VkImageUsageFlags2KHR>(create_info.usage);
}

Location GetUsageLocation(const VkFramebufferAttachmentImageInfo& create_info, const Location& loc) {
    const auto extended = vku::FindStructInPNextChain<VkImageUsageFlags2CreateInfoKHR>(create_info.pNext);
    if (extended) {
        return loc.pNext(vvl::Struct::VkImageUsageFlags2CreateInfoKHR).dot(vvl::Field::usage);
    }
    return loc.dot(vvl::Field::usage);
}

VkImageUsageFlags2KHR GetImageUsageFlags(const VkSwapchainCreateInfoKHR& create_info) {
    const auto extended = vku::FindStructInPNextChain<VkImageUsageFlags2CreateInfoKHR>(create_info.pNext);
    if (extended) {
        return extended->usage;
    }
    return static_cast<VkImageUsageFlags2KHR>(create_info.imageUsage);
}

Location GetImageUsageLocation(const VkSwapchainCreateInfoKHR& create_info, const Location& loc) {
    const auto extended = vku::FindStructInPNextChain<VkImageUsageFlags2CreateInfoKHR>(create_info.pNext);
    if (extended) {
        return loc.pNext(vvl::Struct::VkImageUsageFlags2CreateInfoKHR).dot(vvl::Field::usage);
    }
    return loc.dot(vvl::Field::imageUsage);
}

VkImageUsageFlags2KHR GetImageUsageFlags(const VkPhysicalDeviceVideoFormatInfoKHR& create_info) {
    const auto extended = vku::FindStructInPNextChain<VkImageUsageFlags2CreateInfoKHR>(create_info.pNext);
    if (extended) {
        return extended->usage;
    }
    return static_cast<VkImageUsageFlags2KHR>(create_info.imageUsage);
}

Location GetImageUsageLocation(const VkPhysicalDeviceVideoFormatInfoKHR& create_info, const Location& loc) {
    const auto extended = vku::FindStructInPNextChain<VkImageUsageFlags2CreateInfoKHR>(create_info.pNext);
    if (extended) {
        return loc.pNext(vvl::Struct::VkImageUsageFlags2CreateInfoKHR).dot(vvl::Field::usage);
    }
    return loc.dot(vvl::Field::imageUsage);
}

VkPipelineCreateFlags2 GetPipelineCreateFlags(const VkRayTracingPipelineCreateInfoNV& create_info) {
    const auto extended = vku::FindStructInPNextChain<VkPipelineCreateFlags2CreateInfo>(create_info.pNext);
    if (extended) {
        return extended->flags;
    }
    return static_cast<VkPipelineCreateFlags2>(create_info.flags);
}

Location GetFlagsLocation(const VkRayTracingPipelineCreateInfoNV& create_info, const Location& loc) {
    const auto extended = vku::FindStructInPNextChain<VkPipelineCreateFlags2CreateInfo>(create_info.pNext);
    if (extended) {
        return loc.pNext(vvl::Struct::VkPipelineCreateFlags2CreateInfo).dot(vvl::Field::flags);
    }
    return loc.dot(vvl::Field::flags);
}

VkBufferUsageFlags2 GetDeviceAddress(const VkDescriptorBufferBindingInfoEXT& create_info) {
    const auto extended = vku::FindStructInPNextChain<VkBufferUsageFlags2CreateInfo>(create_info.pNext);
    if (extended) {
        return extended->usage;
    }
    return static_cast<VkBufferUsageFlags2>(create_info.address);
}

Location GetAddressLocation(const VkDescriptorBufferBindingInfoEXT& create_info, const Location& loc) {
    const auto extended = vku::FindStructInPNextChain<VkBufferUsageFlags2CreateInfo>(create_info.pNext);
    if (extended) {
        return loc.pNext(vvl::Struct::VkBufferUsageFlags2CreateInfo).dot(vvl::Field::usage);
    }
    return loc.dot(vvl::Field::address);
}

VkPipelineCreateFlags2 GetPipelineCreateFlags(const VkRayTracingPipelineCreateInfoKHR& create_info) {
    const auto extended = vku::FindStructInPNextChain<VkPipelineCreateFlags2CreateInfo>(create_info.pNext);
    if (extended) {
        return extended->flags;
    }
    return static_cast<VkPipelineCreateFlags2>(create_info.flags);
}

Location GetFlagsLocation(const VkRayTracingPipelineCreateInfoKHR& create_info, const Location& loc) {
    const auto extended = vku::FindStructInPNextChain<VkPipelineCreateFlags2CreateInfo>(create_info.pNext);
    if (extended) {
        return loc.pNext(vvl::Struct::VkPipelineCreateFlags2CreateInfo).dot(vvl::Field::flags);
    }
    return loc.dot(vvl::Field::flags);
}

// NOLINTEND
