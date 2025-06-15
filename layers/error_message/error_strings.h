/* Copyright (c) 2024-2025 The Khronos Group Inc.
 * Copyright (c) 2024-2025 Valve Corporation
 * Copyright (c) 2024-2025 LunarG, Inc.
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
 */
#pragma once

#include "error_message/logging.h"
#include <vulkan/vk_enum_string_helper.h>
#include <sstream>
#include <string>

[[maybe_unused]] static std::string string_Attachment(uint32_t attachment) {
    if (attachment == VK_ATTACHMENT_UNUSED) {
        return "VK_ATTACHMENT_UNUSED";
    } else {
        return std::to_string(attachment);
    }
}

[[maybe_unused]] static std::string string_VkExtent2D(VkExtent2D extent) {
    std::stringstream ss;
    ss << "width = " << extent.width << ", height = " << extent.height;
    return ss.str();
}

[[maybe_unused]] static std::string string_VkExtent3D(VkExtent3D extent) {
    std::stringstream ss;
    ss << "width = " << extent.width << ", height = " << extent.height << ", depth = " << extent.depth;
    return ss.str();
}

[[maybe_unused]] static std::string string_VkOffset2D(VkOffset2D offset) {
    std::stringstream ss;
    ss << "x = " << offset.x << ", y = " << offset.y;
    return ss.str();
}

[[maybe_unused]] static std::string string_VkOffset3D(VkOffset3D offset) {
    std::stringstream ss;
    ss << "x = " << offset.x << ", y = " << offset.y << ", z = " << offset.z;
    return ss.str();
}

[[maybe_unused]] static std::string string_VkRect2D(VkRect2D rect) {
    std::stringstream ss;
    ss << "offset = {" << rect.offset.x << ", " << rect.offset.y << "}, extent = {" << rect.extent.width << ", "
       << rect.extent.height << "}";
    return ss.str();
}

[[maybe_unused]] static std::string string_LevelCount(const VkImageCreateInfo &ci, VkImageSubresourceRange const &range) {
    std::stringstream ss;
    if (range.levelCount == VK_REMAINING_MIP_LEVELS) {
        const uint32_t level_count = ci.mipLevels - range.baseMipLevel;
        ss << "VK_REMAINING_MIP_LEVELS [mipLevels (" << ci.mipLevels << ") - baseMipLevel (" << range.baseMipLevel
           << ") = " << level_count << "]";
    } else {
        ss << range.levelCount;
    }
    return ss.str();
}

[[maybe_unused]] static std::string string_LayerCount(const VkImageCreateInfo &ci, VkImageSubresourceRange const &range) {
    std::stringstream ss;
    if (range.layerCount == VK_REMAINING_ARRAY_LAYERS) {
        const uint32_t layer_count = ci.arrayLayers - range.baseArrayLayer;
        ss << "VK_REMAINING_ARRAY_LAYERS [arrayLayers (" << ci.arrayLayers << ") - baseArrayLayer (" << range.baseArrayLayer
           << ") = " << layer_count << "]";
    } else {
        ss << range.layerCount;
    }
    return ss.str();
}

[[maybe_unused]] static std::string string_LayerCount(const VkImageCreateInfo &ci, VkImageSubresourceLayers const &resource) {
    std::stringstream ss;
    if (resource.layerCount == VK_REMAINING_ARRAY_LAYERS) {
        const uint32_t layer_count = ci.arrayLayers - resource.baseArrayLayer;
        ss << "VK_REMAINING_ARRAY_LAYERS [arrayLayers (" << ci.arrayLayers << ") - baseArrayLayer (" << resource.baseArrayLayer
           << ") = " << layer_count << "]";
    } else {
        ss << resource.layerCount;
    }
    return ss.str();
}

[[maybe_unused]] static std::string string_VkPushConstantRange(VkPushConstantRange range) {
    std::stringstream ss;
    ss << "range [" << range.offset << ", " << (range.offset + range.size) << ") for "
       << string_VkShaderStageFlags(range.stageFlags);
    return ss.str();
}

[[maybe_unused]] static std::string string_VkImageSubresource(VkImageSubresource subresource) {
    std::stringstream ss;
    ss << "aspectMask = " << string_VkImageAspectFlags(subresource.aspectMask) << ", mipLevel = " << subresource.mipLevel
       << ", arrayLayer = " << subresource.arrayLayer;
    return ss.str();
}

[[maybe_unused]] static std::string string_VkImageSubresourceLayers(VkImageSubresourceLayers subresource_layers) {
    std::stringstream ss;
    ss << "aspectMask = " << string_VkImageAspectFlags(subresource_layers.aspectMask)
       << ", mipLevel = " << subresource_layers.mipLevel << ", baseArrayLayer = " << subresource_layers.baseArrayLayer
       << ", layerCount = " << subresource_layers.layerCount;
    return ss.str();
}

[[maybe_unused]] static std::string string_VkImageSubresourceRange(VkImageSubresourceRange subresource_range) {
    std::stringstream ss;
    ss << "aspectMask = " << string_VkImageAspectFlags(subresource_range.aspectMask)
       << ", baseMipLevel = " << subresource_range.baseMipLevel << ", levelCount = " << subresource_range.levelCount
       << ", baseArrayLayer = " << subresource_range.baseArrayLayer << ", layerCount = " << subresource_range.layerCount;
    return ss.str();
}

[[maybe_unused]] static std::string string_VkComponentMapping(VkComponentMapping components) {
    std::stringstream ss;
    ss << "r swizzle = " << string_VkComponentSwizzle(components.r) << "\n";
    ss << "g swizzle = " << string_VkComponentSwizzle(components.g) << "\n";
    ss << "b swizzle = " << string_VkComponentSwizzle(components.b) << "\n";
    ss << "a swizzle = " << string_VkComponentSwizzle(components.a) << "\n";
    return ss.str();
}

[[maybe_unused]] static std::string string_VkBool32(VkBool32 value) { return value ? "VK_TRUE" : "VK_FALSE"; }

// Some VUs use the subset in VkPhysicalDeviceImageFormatInfo2 to refer to an VkImageCreateInfo
[[maybe_unused]] static std::string string_VkPhysicalDeviceImageFormatInfo2(VkPhysicalDeviceImageFormatInfo2 info) {
    std::stringstream ss;
    ss << "format (" << string_VkFormat(info.format) << ")\n";
    ss << "type (" << string_VkImageType(info.type) << ")\n";
    ss << "tiling (" << string_VkImageTiling(info.tiling) << ")\n";
    ss << "usage (" << string_VkImageUsageFlags(info.usage) << ")\n";
    ss << "flags (" << string_VkImageCreateFlags(info.flags) << ")\n";
    return ss.str();
}

// Same thing as VkPhysicalDeviceImageFormatInfo2 but given the actual VkImageCreateInfo
[[maybe_unused]] static std::string string_VkPhysicalDeviceImageFormatInfo2(VkImageCreateInfo info) {
    std::stringstream ss;
    ss << "format (" << string_VkFormat(info.format) << ")\n";
    ss << "type (" << string_VkImageType(info.imageType) << ")\n";
    ss << "tiling (" << string_VkImageTiling(info.tiling) << ")\n";
    ss << "usage (" << string_VkImageUsageFlags(info.usage) << ")\n";
    ss << "flags (" << string_VkImageCreateFlags(info.flags) << ")\n";
    return ss.str();
}

[[maybe_unused]] static std::string string_VkDependencyInfo(const Logger &logger, VkDependencyInfo set_dependency_info,
                                                            VkDependencyInfo dependency_info) {
    std::stringstream set;
    std::stringstream wait;
    if (set_dependency_info.dependencyFlags != dependency_info.dependencyFlags) {
        set << std::string(string_VkDependencyFlags(set_dependency_info.dependencyFlags));
        wait << std::string(string_VkDependencyFlags(dependency_info.dependencyFlags));
    } else if (set_dependency_info.memoryBarrierCount != dependency_info.memoryBarrierCount) {
        set << "memoryBarrierCount " << set_dependency_info.memoryBarrierCount;
        wait << "memoryBarrierCount " << dependency_info.memoryBarrierCount;
    } else if (set_dependency_info.bufferMemoryBarrierCount != dependency_info.bufferMemoryBarrierCount) {
        set << "bufferMemoryBarrierCount " << set_dependency_info.bufferMemoryBarrierCount;
        wait << "bufferMemoryBarrierCount " << dependency_info.bufferMemoryBarrierCount;
    } else if (set_dependency_info.imageMemoryBarrierCount != dependency_info.imageMemoryBarrierCount) {
        set << "imageMemoryBarrierCount " << set_dependency_info.imageMemoryBarrierCount;
        wait << "imageMemoryBarrierCount " << dependency_info.imageMemoryBarrierCount;
    } else {
        for (uint32_t i = 0; i < dependency_info.memoryBarrierCount; ++i) {
            bool found = true;
            if (dependency_info.pMemoryBarriers[i].srcStageMask != set_dependency_info.pMemoryBarriers[i].srcStageMask) {
                set << "pMemoryBarriers[" << i << "].srcStageMask "
                    << string_VkPipelineStageFlags2(set_dependency_info.pMemoryBarriers[i].srcStageMask);
                wait << "pMemoryBarriers[" << i << "].srcStageMask "
                     << string_VkPipelineStageFlags2(dependency_info.pMemoryBarriers[i].srcStageMask);
            } else if (dependency_info.pMemoryBarriers[i].srcAccessMask != set_dependency_info.pMemoryBarriers[i].srcAccessMask) {
                set << "pMemoryBarriers[" << i << "].srcAccessMask "
                    << string_VkAccessFlags2(set_dependency_info.pMemoryBarriers[i].srcAccessMask);
                wait << "pMemoryBarriers[" << i << "].srcAccessMask "
                     << string_VkAccessFlags2(dependency_info.pMemoryBarriers[i].srcAccessMask);
            } else if (dependency_info.pMemoryBarriers[i].dstStageMask != set_dependency_info.pMemoryBarriers[i].dstStageMask) {
                set << "pMemoryBarriers[" << i << "].dstStageMask "
                    << string_VkPipelineStageFlags2(set_dependency_info.pMemoryBarriers[i].dstStageMask);
                wait << "pMemoryBarriers[" << i << "].dstStageMask "
                     << string_VkPipelineStageFlags2(dependency_info.pMemoryBarriers[i].dstStageMask);
            } else if (dependency_info.pMemoryBarriers[i].dstAccessMask != set_dependency_info.pMemoryBarriers[i].dstAccessMask) {
                set << "pMemoryBarriers[" << i << "].dstAccessMask "
                    << string_VkAccessFlags2(set_dependency_info.pMemoryBarriers[i].dstAccessMask);
                wait << "pMemoryBarriers[" << i << "].dstAccessMask "
                     << string_VkAccessFlags2(dependency_info.pMemoryBarriers[i].dstAccessMask);
            } else {
                found = false;
            }
            if (found) {
                break;
            }
        }
        for (uint32_t i = 0; i < dependency_info.bufferMemoryBarrierCount; ++i) {
            bool found = true;
            if (dependency_info.pBufferMemoryBarriers[i].srcStageMask !=
                set_dependency_info.pBufferMemoryBarriers[i].srcStageMask) {
                set << "pBufferMemoryBarriers[" << i << "].srcStageMask "
                    << string_VkPipelineStageFlags2(set_dependency_info.pBufferMemoryBarriers[i].srcStageMask);
                wait << "pBufferMemoryBarriers[" << i << "].srcStageMask "
                     << string_VkPipelineStageFlags2(dependency_info.pBufferMemoryBarriers[i].srcStageMask);
            } else if (dependency_info.pBufferMemoryBarriers[i].srcAccessMask !=
                       set_dependency_info.pBufferMemoryBarriers[i].srcAccessMask) {
                set << "pBufferMemoryBarriers[" << i << "].srcAccessMask "
                    << string_VkAccessFlags2(set_dependency_info.pBufferMemoryBarriers[i].srcAccessMask);
                wait << "pBufferMemoryBarriers[" << i << "].srcAccessMask "
                     << string_VkAccessFlags2(dependency_info.pBufferMemoryBarriers[i].srcAccessMask);
            } else if (dependency_info.pBufferMemoryBarriers[i].dstStageMask !=
                       set_dependency_info.pBufferMemoryBarriers[i].dstStageMask) {
                set << "pBufferMemoryBarriers[" << i << "].dstStageMask "
                    << string_VkPipelineStageFlags2(set_dependency_info.pBufferMemoryBarriers[i].dstStageMask);
                wait << "pBufferMemoryBarriers[" << i << "].dstStageMask "
                     << string_VkPipelineStageFlags2(dependency_info.pBufferMemoryBarriers[i].dstStageMask);
            } else if (dependency_info.pBufferMemoryBarriers[i].dstAccessMask !=
                       set_dependency_info.pBufferMemoryBarriers[i].dstAccessMask) {
                set << "pBufferMemoryBarriers[" << i << "].dstAccessMask "
                    << string_VkAccessFlags2(set_dependency_info.pBufferMemoryBarriers[i].dstAccessMask);
                wait << "pBufferMemoryBarriers[" << i << "].dstAccessMask "
                     << string_VkAccessFlags2(dependency_info.pBufferMemoryBarriers[i].dstAccessMask);
            } else if (dependency_info.pBufferMemoryBarriers[i].srcQueueFamilyIndex !=
                       set_dependency_info.pBufferMemoryBarriers[i].srcQueueFamilyIndex) {
                set << "pBufferMemoryBarriers[" << i << "].srcQueueFamilyIndex "
                    << set_dependency_info.pBufferMemoryBarriers[i].srcQueueFamilyIndex;
                wait << "pBufferMemoryBarriers[" << i << "].srcQueueFamilyIndex "
                     << dependency_info.pBufferMemoryBarriers[i].srcQueueFamilyIndex;
            } else if (dependency_info.pBufferMemoryBarriers[i].dstQueueFamilyIndex !=
                       set_dependency_info.pBufferMemoryBarriers[i].dstQueueFamilyIndex) {
                set << "pBufferMemoryBarriers[" << i << "].dstQueueFamilyIndex "
                    << set_dependency_info.pBufferMemoryBarriers[i].dstQueueFamilyIndex;
                wait << "pBufferMemoryBarriers[" << i << "].dstQueueFamilyIndex "
                     << dependency_info.pBufferMemoryBarriers[i].dstQueueFamilyIndex;
            } else if (dependency_info.pBufferMemoryBarriers[i].buffer != set_dependency_info.pBufferMemoryBarriers[i].buffer) {
                set << "pBufferMemoryBarriers[" << i << "].buffer "
                    << logger.FormatHandle(set_dependency_info.pBufferMemoryBarriers[i].buffer);
                wait << "pBufferMemoryBarriers[" << i << "].buffer "
                     << logger.FormatHandle(dependency_info.pBufferMemoryBarriers[i].buffer);
            } else if (dependency_info.pBufferMemoryBarriers[i].offset != set_dependency_info.pBufferMemoryBarriers[i].offset) {
                set << "pBufferMemoryBarriers[" << i << "].offset " << set_dependency_info.pBufferMemoryBarriers[i].offset;
                wait << "pBufferMemoryBarriers[" << i << "].offset " << dependency_info.pBufferMemoryBarriers[i].offset;
            } else if (dependency_info.pBufferMemoryBarriers[i].size != set_dependency_info.pBufferMemoryBarriers[i].size) {
                set << "pBufferMemoryBarriers[" << i << "].size " << set_dependency_info.pBufferMemoryBarriers[i].size;
                wait << "pBufferMemoryBarriers[" << i << "].size " << dependency_info.pBufferMemoryBarriers[i].size;
            } else {
                found = false;
            }
            if (found) {
                break;
            }
        }
        for (uint32_t i = 0; i < dependency_info.imageMemoryBarrierCount; ++i) {
            bool found = true;
            if (dependency_info.pImageMemoryBarriers[i].srcStageMask != set_dependency_info.pImageMemoryBarriers[i].srcStageMask) {
                set << "pImageMemoryBarriers[" << i << "].srcStageMask "
                    << string_VkPipelineStageFlags2(set_dependency_info.pImageMemoryBarriers[i].srcStageMask);
                wait << "pImageMemoryBarriers[" << i << "].srcStageMask "
                     << string_VkPipelineStageFlags2(dependency_info.pImageMemoryBarriers[i].srcStageMask);
            } else if (dependency_info.pImageMemoryBarriers[i].srcAccessMask !=
                       set_dependency_info.pImageMemoryBarriers[i].srcAccessMask) {
                set << "pImageMemoryBarriers[" << i << "].srcAccessMask "
                    << string_VkAccessFlags2(set_dependency_info.pImageMemoryBarriers[i].srcAccessMask);
                wait << "pImageMemoryBarriers[" << i << "].srcAccessMask "
                     << string_VkAccessFlags2(dependency_info.pImageMemoryBarriers[i].srcAccessMask);
            } else if (dependency_info.pImageMemoryBarriers[i].dstStageMask !=
                       set_dependency_info.pImageMemoryBarriers[i].dstStageMask) {
                set << "pImageMemoryBarriers[" << i << "].dstStageMask "
                    << string_VkPipelineStageFlags2(set_dependency_info.pImageMemoryBarriers[i].dstStageMask);
                wait << "pImageMemoryBarriers[" << i << "].dstStageMask "
                     << string_VkPipelineStageFlags2(dependency_info.pImageMemoryBarriers[i].dstStageMask);
            } else if (dependency_info.pImageMemoryBarriers[i].dstAccessMask !=
                       set_dependency_info.pImageMemoryBarriers[i].dstAccessMask) {
                set << "pImageMemoryBarriers[" << i << "].dstAccessMask "
                    << string_VkAccessFlags2(set_dependency_info.pImageMemoryBarriers[i].dstAccessMask);
                wait << "pImageMemoryBarriers[" << i << "].dstAccessMask "
                     << string_VkAccessFlags2(dependency_info.pImageMemoryBarriers[i].dstAccessMask);
            } else if (dependency_info.pImageMemoryBarriers[i].oldLayout != set_dependency_info.pImageMemoryBarriers[i].oldLayout) {
                set << "pImageMemoryBarriers[" << i << "].oldLayout "
                    << string_VkAccessFlags2(set_dependency_info.pImageMemoryBarriers[i].oldLayout);
                wait << "pImageMemoryBarriers[" << i << "].oldLayout "
                     << string_VkAccessFlags2(dependency_info.pImageMemoryBarriers[i].oldLayout);
            } else if (dependency_info.pImageMemoryBarriers[i].newLayout != set_dependency_info.pImageMemoryBarriers[i].newLayout) {
                set << "pImageMemoryBarriers[" << i << "].newLayout "
                    << string_VkAccessFlags2(set_dependency_info.pImageMemoryBarriers[i].newLayout);
                wait << "pImageMemoryBarriers[" << i << "].newLayout "
                     << string_VkAccessFlags2(dependency_info.pImageMemoryBarriers[i].newLayout);
            } else if (dependency_info.pImageMemoryBarriers[i].srcQueueFamilyIndex !=
                       set_dependency_info.pImageMemoryBarriers[i].srcQueueFamilyIndex) {
                set << "pImageMemoryBarriers[" << i << "].srcQueueFamilyIndex "
                    << set_dependency_info.pImageMemoryBarriers[i].srcQueueFamilyIndex;
                wait << "pImageMemoryBarriers[" << i << "].srcQueueFamilyIndex "
                     << dependency_info.pImageMemoryBarriers[i].srcQueueFamilyIndex;
            } else if (dependency_info.pImageMemoryBarriers[i].dstQueueFamilyIndex !=
                       set_dependency_info.pImageMemoryBarriers[i].dstQueueFamilyIndex) {
                set << "pImageMemoryBarriers[" << i << "].dstQueueFamilyIndex "
                    << set_dependency_info.pImageMemoryBarriers[i].dstQueueFamilyIndex;
                wait << "pImageMemoryBarriers[" << i << "].dstQueueFamilyIndex "
                     << dependency_info.pImageMemoryBarriers[i].dstQueueFamilyIndex;
            } else if (dependency_info.pImageMemoryBarriers[i].image != set_dependency_info.pImageMemoryBarriers[i].image) {
                set << "pImageMemoryBarriers[" << i << "].image "
                    << logger.FormatHandle(set_dependency_info.pImageMemoryBarriers[i].image);
                wait << "pImageMemoryBarriers[" << i << "].image "
                     << logger.FormatHandle(dependency_info.pImageMemoryBarriers[i].image);
            } else if (dependency_info.pImageMemoryBarriers[i].subresourceRange.aspectMask !=
                       set_dependency_info.pImageMemoryBarriers[i].subresourceRange.aspectMask) {
                set << "pImageMemoryBarriers[" << i << "].subresourceRange.aspectMask "
                    << string_VkImageAspectFlags(set_dependency_info.pImageMemoryBarriers[i].subresourceRange.aspectMask);
                wait << "pImageMemoryBarriers[" << i << "].subresourceRange.aspectMask "
                     << string_VkImageAspectFlags(dependency_info.pImageMemoryBarriers[i].subresourceRange.aspectMask);
            } else if (dependency_info.pImageMemoryBarriers[i].subresourceRange.baseMipLevel !=
                       set_dependency_info.pImageMemoryBarriers[i].subresourceRange.baseMipLevel) {
                set << "pImageMemoryBarriers[" << i << "].subresourceRange.baseMipLevel "
                    << set_dependency_info.pImageMemoryBarriers[i].subresourceRange.baseMipLevel;
                wait << "pImageMemoryBarriers[" << i << "].subresourceRange.baseMipLevel "
                     << dependency_info.pImageMemoryBarriers[i].subresourceRange.baseMipLevel;
            } else if (dependency_info.pImageMemoryBarriers[i].subresourceRange.levelCount !=
                       set_dependency_info.pImageMemoryBarriers[i].subresourceRange.levelCount) {
                set << "pImageMemoryBarriers[" << i << "].subresourceRange.levelCount "
                    << set_dependency_info.pImageMemoryBarriers[i].subresourceRange.levelCount;
                wait << "pImageMemoryBarriers[" << i << "].subresourceRange.levelCount "
                     << dependency_info.pImageMemoryBarriers[i].subresourceRange.levelCount;
            } else if (dependency_info.pImageMemoryBarriers[i].subresourceRange.baseArrayLayer !=
                       set_dependency_info.pImageMemoryBarriers[i].subresourceRange.baseArrayLayer) {
                set << "pImageMemoryBarriers[" << i << "].subresourceRange.baseArrayLayer "
                    << set_dependency_info.pImageMemoryBarriers[i].subresourceRange.baseArrayLayer;
                wait << "pImageMemoryBarriers[" << i << "].subresourceRange.baseArrayLayer "
                     << dependency_info.pImageMemoryBarriers[i].subresourceRange.baseArrayLayer;
            } else if (dependency_info.pImageMemoryBarriers[i].subresourceRange.layerCount !=
                       set_dependency_info.pImageMemoryBarriers[i].subresourceRange.layerCount) {
                set << "pImageMemoryBarriers[" << i << "].subresourceRange.layerCount "
                    << set_dependency_info.pImageMemoryBarriers[i].subresourceRange.layerCount;
                wait << "pImageMemoryBarriers[" << i << "].subresourceRange.layerCount "
                     << dependency_info.pImageMemoryBarriers[i].subresourceRange.layerCount;
            } else {
                found = false;
            }
            if (found) {
                break;
            }
        }
    }
    return "event was set with " + set.str() + " and is being waited on with " + wait.str();
}
