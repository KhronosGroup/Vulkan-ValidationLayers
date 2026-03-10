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

#include "containers/span.h"
#include "state_tracker/buffer_state.h"
#include "state_tracker/state_tracker.h"

#include <sstream>

constexpr const char *indent = "    ";

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
