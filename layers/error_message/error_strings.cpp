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

#include "error_message/error_strings.h"
#include "error_message/logging.h"

#include "containers/span.h"
#include "state_tracker/buffer_state.h"
#include "state_tracker/state_tracker.h"

#include <sstream>

constexpr const char *indent = "    ";

std::string string_VkDependencyInfo(const Logger& logger, VkDependencyInfo set_dependency_info, VkDependencyInfo dependency_info) {
    std::ostringstream set;
    std::ostringstream wait;
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

static std::string BuffersFromAddressStr(const vvl::DeviceProxy &validator, VkDeviceAddress address) {
    std::string buffers_str;
    vvl::span<vvl::Buffer *const> buffers = validator.GetBuffersByAddress(address);
    for (vvl::Buffer *const buffer : buffers) {
        if (!buffers_str.empty()) {
            buffers_str += '\n';
        }
        buffers_str += indent;
        buffers_str += indent;
        buffers_str += buffer->Describe(validator);
    }
    return buffers_str;
}

std::string string_VkAccelerationStructureBuildGeometryInfoKHR(const Logger &logger,
                                                               const VkAccelerationStructureBuildGeometryInfoKHR &info) {
    std::stringstream ss;

    ss << indent << "type: " << string_VkAccelerationStructureTypeKHR(info.type) << '\n';
    ss << indent << "flags: " << string_VkBuildAccelerationStructureFlagsKHR(info.flags).c_str() << '\n';
    ss << indent << "mode: " << string_VkBuildAccelerationStructureModeKHR(info.mode) << '\n';
    ss << indent << "srcAccelerationStructure: " << logger.FormatHandle(info.srcAccelerationStructure).c_str() << '\n';
    ss << indent << "dstAccelerationStructure: " << logger.FormatHandle(info.dstAccelerationStructure).c_str() << '\n';
    ss << indent << "geometryCount: " << info.geometryCount << '\n';
    ss << indent << "pGeometries: " << info.pGeometries << '\n';
    ss << indent << "ppGeometries: " << info.ppGeometries << '\n';
    ss << indent << "scratchData: 0x" << std::hex << info.scratchData.deviceAddress << '\n';

    std::string ss_str = ss.str();
    return ss_str;
}

std::string string_VkAccelerationStructureGeometryTrianglesDataKHR(
    const vvl::DeviceProxy &validator, const VkAccelerationStructureGeometryTrianglesDataKHR &triangles) {
    std::string pertains = indent;
    pertains += indent;
    pertains += "Pertains to the following buffer(s):\n";

    std::stringstream ss;
    ss << indent << "vertexFormat: " << string_VkFormat(triangles.vertexFormat) << '\n';
    ss << indent << "vertexData: 0x" << std::hex << triangles.vertexData.deviceAddress << '\n';
    std::string vertex_buffers_list_str = BuffersFromAddressStr(validator, triangles.vertexData.deviceAddress);
    if (!vertex_buffers_list_str.empty()) {
        std::string vertex_buffers_str = pertains;
        vertex_buffers_str += vertex_buffers_list_str;
        ss << vertex_buffers_str << '\n';
    }
    ss << indent << "vertexStride: " << std::dec << triangles.vertexStride << '\n';
    ss << indent << "maxVertex: " << triangles.maxVertex << '\n';
    ss << indent << "indexType: " << string_VkIndexType(triangles.indexType) << '\n';
    ss << indent << "indexData: " << std::hex << triangles.indexData.deviceAddress << '\n';
    if (triangles.indexType != VK_INDEX_TYPE_NONE_KHR) {
        std::string index_buffers_list_str = BuffersFromAddressStr(validator, triangles.indexData.deviceAddress);
        if (!index_buffers_list_str.empty()) {
            std::string index_buffers_str = pertains;
            index_buffers_str += index_buffers_list_str;
            ss << index_buffers_str << '\n';
        }
    }
    ss << indent << "transformData: " << triangles.transformData.deviceAddress << '\n';
    if (triangles.transformData.deviceAddress != 0) {
        std::string transform_buffers_list_str = BuffersFromAddressStr(validator, triangles.transformData.deviceAddress);
        if (!transform_buffers_list_str.empty()) {
            std::string transform_buffers_str = pertains;
            transform_buffers_str += transform_buffers_list_str;
            ss << transform_buffers_str << '\n';
        }
    }

    std::string ss_str = ss.str();
    return ss_str;
}

std::string string_VkAccelerationStructureGeometryAabbsDataKHR(const vvl::DeviceProxy &validator,
                                                               const VkAccelerationStructureGeometryAabbsDataKHR aabb) {
    std::string pertains = indent;
    pertains += indent;
    pertains += "Pertains to the following buffer(s):\n";

    std::stringstream ss;
    ss << indent << "pNext: " << aabb.pNext << '\n';
    ss << indent << "data: 0x" << std::hex << aabb.data.deviceAddress << '\n';
    std::string aabb_buffers_list_str = BuffersFromAddressStr(validator, aabb.data.deviceAddress);
    if (!aabb_buffers_list_str.empty()) {
        std::string data_buffers_str = pertains;
        data_buffers_str += aabb_buffers_list_str;
        ss << data_buffers_str << '\n';
    }
    ss << indent << "stride: " << std::dec << aabb.stride << '\n';

    std::string ss_str = ss.str();
    return ss_str;
}

std::string string_VkAccelerationStructureBuildRangeInfoKHR(const VkAccelerationStructureBuildRangeInfoKHR &bri) {
    std::stringstream ss;
    ss << indent << "primitiveCount: " << bri.primitiveCount << '\n';
    ss << indent << "primitiveOffset: " << bri.primitiveOffset << '\n';
    ss << indent << "firstVertex: " << bri.firstVertex << '\n';
    ss << indent << "transformOffset: " << bri.transformOffset << '\n';

    std::string ss_str = ss.str();
    return ss_str;
}
