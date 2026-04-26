/* Copyright (c) 2024-2026 The Khronos Group Inc.
 * Copyright (c) 2024-2026 Valve Corporation
 * Copyright (c) 2024-2026 LunarG, Inc.
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

#include <vulkan/vk_enum_string_helper.h>
#include <sstream>
#include <string>

class Logger;
namespace vvl {
class DeviceState;
}

[[maybe_unused]] static std::string string_Attachment(uint32_t attachment) {
    if (attachment == VK_ATTACHMENT_UNUSED) {
        return "VK_ATTACHMENT_UNUSED";
    } else {
        return std::to_string(attachment);
    }
}

[[maybe_unused]] static std::string string_AttachmentPointer(const uint32_t *attachment) {
    if (!attachment) {
        return "NULL";
    }
    return string_Attachment(*attachment);
}

[[maybe_unused]] static std::string string_VkExtent2D(VkExtent2D extent) {
    std::ostringstream ss;
    ss << "width = " << extent.width << ", height = " << extent.height;
    return ss.str();
}

[[maybe_unused]] static std::string string_VkExtent3D(VkExtent3D extent) {
    std::ostringstream ss;
    ss << "width = " << extent.width << ", height = " << extent.height << ", depth = " << extent.depth;
    return ss.str();
}

[[maybe_unused]] static std::string string_VkExtentDimensions(VkExtent2D extent) {
    std::ostringstream ss;
    ss << extent.width << "x" << extent.height;
    return ss.str();
}

[[maybe_unused]] static std::string string_VkExtentDimensions(VkExtent3D extent) {
    std::ostringstream ss;
    ss << extent.width << "x" << extent.height << "x" << extent.depth;
    return ss.str();
}

[[maybe_unused]] static std::string string_VkOffset2D(VkOffset2D offset) {
    std::ostringstream ss;
    ss << "x = " << offset.x << ", y = " << offset.y;
    return ss.str();
}

[[maybe_unused]] static std::string string_VkOffset3D(VkOffset3D offset) {
    std::ostringstream ss;
    ss << "x = " << offset.x << ", y = " << offset.y << ", z = " << offset.z;
    return ss.str();
}

[[maybe_unused]] static std::string string_VkRect2D(VkRect2D rect) {
    std::ostringstream ss;
    ss << "offset = {" << rect.offset.x << ", " << rect.offset.y << "}, extent = {" << rect.extent.width << ", "
       << rect.extent.height << "}";
    return ss.str();
}

[[maybe_unused]] static std::string string_LevelCount(uint32_t mipLevels, VkImageSubresourceRange const &range) {
    std::ostringstream ss;
    if (range.levelCount == VK_REMAINING_MIP_LEVELS) {
        const uint32_t level_count = mipLevels - range.baseMipLevel;
        ss << "VK_REMAINING_MIP_LEVELS [mipLevels (" << mipLevels << ") - baseMipLevel (" << range.baseMipLevel
           << ") = " << level_count << "]";
    } else {
        ss << range.levelCount;
    }
    return ss.str();
}

[[maybe_unused]] static std::string string_LayerCount(uint32_t arrayLayers, VkImageSubresourceRange const &range) {
    std::ostringstream ss;
    if (range.layerCount == VK_REMAINING_ARRAY_LAYERS) {
        const uint32_t layer_count = arrayLayers - range.baseArrayLayer;
        ss << "VK_REMAINING_ARRAY_LAYERS [arrayLayers (" << arrayLayers << ") - baseArrayLayer (" << range.baseArrayLayer
           << ") = " << layer_count << "]";
    } else {
        ss << range.layerCount;
    }
    return ss.str();
}

[[maybe_unused]] static std::string string_LayerCount(uint32_t arrayLayers, VkImageSubresourceLayers const &resource) {
    std::ostringstream ss;
    if (resource.layerCount == VK_REMAINING_ARRAY_LAYERS) {
        const uint32_t layer_count = arrayLayers - resource.baseArrayLayer;
        ss << "VK_REMAINING_ARRAY_LAYERS [arrayLayers (" << arrayLayers << ") - baseArrayLayer (" << resource.baseArrayLayer
           << ") = " << layer_count << "]";
    } else {
        ss << resource.layerCount;
    }
    return ss.str();
}

[[maybe_unused]] static std::string string_VkPushConstantRange(VkPushConstantRange range) {
    std::ostringstream ss;
    ss << "range [" << range.offset << ", " << (range.offset + range.size) << "] for "
       << string_VkShaderStageFlags(range.stageFlags);
    return ss.str();
}

[[maybe_unused]] static std::string string_VkImageSubresource(VkImageSubresource subresource) {
    std::ostringstream ss;
    ss << "aspectMask = " << string_VkImageAspectFlags(subresource.aspectMask) << ", mipLevel = " << subresource.mipLevel
       << ", arrayLayer = " << subresource.arrayLayer;
    return ss.str();
}

[[maybe_unused]] static std::string string_VkImageSubresourceLayers(VkImageSubresourceLayers subresource_layers) {
    std::ostringstream ss;
    ss << "aspectMask = " << string_VkImageAspectFlags(subresource_layers.aspectMask)
       << ", mipLevel = " << subresource_layers.mipLevel << ", baseArrayLayer = " << subresource_layers.baseArrayLayer
       << ", layerCount = " << subresource_layers.layerCount;
    return ss.str();
}

[[maybe_unused]] static std::string string_VkImageSubresourceRange(VkImageSubresourceRange subresource_range) {
    std::ostringstream ss;
    ss << "aspectMask = " << string_VkImageAspectFlags(subresource_range.aspectMask)
       << ", baseMipLevel = " << subresource_range.baseMipLevel << ", levelCount = " << subresource_range.levelCount
       << ", baseArrayLayer = " << subresource_range.baseArrayLayer << ", layerCount = " << subresource_range.layerCount;
    return ss.str();
}

[[maybe_unused]] static std::string string_VkComponentMapping(VkComponentMapping components) {
    std::ostringstream ss;
    ss << "r swizzle = " << string_VkComponentSwizzle(components.r) << "\n";
    ss << "g swizzle = " << string_VkComponentSwizzle(components.g) << "\n";
    ss << "b swizzle = " << string_VkComponentSwizzle(components.b) << "\n";
    ss << "a swizzle = " << string_VkComponentSwizzle(components.a) << "\n";
    return ss.str();
}

[[maybe_unused]] static std::string string_VkBool32(VkBool32 value) { return value ? "VK_TRUE" : "VK_FALSE"; }

// Some VUs use the subset in VkPhysicalDeviceImageFormatInfo2 to refer to an VkImageCreateInfo
std::string string_VkPhysicalDeviceImageFormatInfo2(VkPhysicalDeviceImageFormatInfo2 info);

// Same thing as VkPhysicalDeviceImageFormatInfo2 but given the actual VkImageCreateInfo
[[maybe_unused]] static std::string string_VkPhysicalDeviceImageFormatInfo2(VkImageCreateFlags flags, VkImageUsageFlags usage,
                                                                            VkFormat format, VkImageType imageType,
                                                                            VkImageTiling tiling) {
    std::ostringstream ss;
    ss << "format (" << string_VkFormat(format) << ")\n";
    ss << "type (" << string_VkImageType(imageType) << ")\n";
    ss << "tiling (" << string_VkImageTiling(tiling) << ")\n";
    ss << "usage (" << string_VkImageUsageFlags(usage) << ")\n";
    ss << "flags (" << string_VkImageCreateFlags(flags) << ")\n";
    return ss.str();
}

[[maybe_unused]] static std::string string_VkStencilOpState(VkStencilOpState state) {
    std::ostringstream ss;
    ss << " failOp (" << string_VkStencilOp(state.failOp) << ")\n";
    ss << " passOp (" << string_VkStencilOp(state.passOp) << ")\n";
    ss << " depthFailOp (" << string_VkStencilOp(state.depthFailOp) << ")\n";
    ss << " compareOp (" << string_VkCompareOp(state.compareOp) << ")\n";
    ss << " compareMask (" << state.compareMask << ")\n";
    ss << " writeMask (" << state.writeMask << ")\n";
    ss << " reference (" << state.reference << ")\n";
    return ss.str();
}

std::string string_VkDependencyInfo(const Logger& logger, VkDependencyInfo set_dependency_info, VkDependencyInfo dependency_info);

[[maybe_unused]] static std::string string_VkDataGraphPipelineResourceInfoARM(VkDataGraphPipelineResourceInfoARM resource) {
    std::ostringstream ss;
    ss << "[descriptorSet " << resource.descriptorSet << ", ";
    ss << "binding " << resource.binding << ", ";
    ss << "arrayElement " << resource.arrayElement << "]";
    return ss.str();
}

[[maybe_unused]] static std::string string_VkBindHeapInfoEXT(VkBindHeapInfoEXT info) {
    std::stringstream ss;
    ss << "heapRange = { address = " << info.heapRange.address << ", size = " << info.heapRange.size << " }, ";
    ss << "reservedRangeOffset = " << info.reservedRangeOffset << ", ";
    ss << "reservedRangeSize = " << info.reservedRangeSize << "";
    return ss.str();
}

[[maybe_unused]] static std::string string_VkPhysicalDeviceDataGraphProcessingEngineARM(
    const VkPhysicalDeviceDataGraphProcessingEngineARM& engine) {
    std::stringstream ss;
    ss << "{ type: " << string_VkPhysicalDeviceDataGraphProcessingEngineTypeARM(engine.type)
       << ", isForeign: " << string_VkBool32(engine.isForeign) << " }";
    return ss.str();
}

[[maybe_unused]] static std::string string_VkPhysicalDeviceDataGraphOperationSupportARM(
    const VkPhysicalDeviceDataGraphOperationSupportARM& operation) {
    std::stringstream ss;
    ss << "{ type: " << string_VkPhysicalDeviceDataGraphOperationTypeARM(operation.operationType) << ", name: \"" << operation.name
       << "\", version: " << operation.version << "}";
    return ss.str();
}

[[maybe_unused]] static std::string string_VkQueueFamilyDataGraphPropertiesARM(
    const VkQueueFamilyDataGraphPropertiesARM& property) {
    std::stringstream ss;
    ss << "{\n  engine: " << string_VkPhysicalDeviceDataGraphProcessingEngineARM(property.engine)
       << ",\n  operation: " << string_VkPhysicalDeviceDataGraphOperationSupportARM(property.operation) << "\n}\n";
    return ss.str();
}

std::string string_BuffersFromAddress(const vvl::DeviceState &device, VkDeviceAddress address);

std::string string_VkAccelerationStructureBuildGeometryInfoKHR(const Logger &logger,
                                                               const VkAccelerationStructureBuildGeometryInfoKHR &info);
std::string string_VkAccelerationStructureGeometryTrianglesDataKHR(
    const vvl::DeviceState &device_state, const VkAccelerationStructureGeometryTrianglesDataKHR &triangles);
std::string string_VkAccelerationStructureGeometryAabbsDataKHR(const vvl::DeviceState &device_state,
                                                               const VkAccelerationStructureGeometryAabbsDataKHR aabb);
std::string string_VkAccelerationStructureBuildRangeInfoKHR(const VkAccelerationStructureBuildRangeInfoKHR &bri);
