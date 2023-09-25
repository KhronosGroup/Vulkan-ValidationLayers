/* Copyright (c) 2023 Valve Corporation
 * Copyright (c) 2023 LunarG, Inc.
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

#include "ray_tracing_utils.h"

#include <cassert>
#include <vector>

#include "generated/layer_chassis_dispatch.h"
#include <vulkan/utility/vk_struct_helper.hpp>

namespace rt {
VkDeviceSize ComputeScratchSize(const VkDevice device, const VkAccelerationStructureBuildGeometryInfoKHR &build_info,
                                const VkAccelerationStructureBuildRangeInfoKHR *range_infos) {
    std::vector<uint32_t> primitive_counts(build_info.geometryCount);
    primitive_counts.reserve(build_info.geometryCount);
    for (uint32_t build_range_i = 0; build_range_i < build_info.geometryCount; build_range_i++) {
        primitive_counts[build_range_i] = range_infos[build_range_i].primitiveCount;
    }
    VkAccelerationStructureBuildSizesInfoKHR size_info = vku::InitStructHelper();
    DispatchGetAccelerationStructureBuildSizesKHR(device, VK_ACCELERATION_STRUCTURE_BUILD_TYPE_DEVICE_KHR, &build_info,
                                                  primitive_counts.data(), &size_info);
    switch (build_info.mode) {
        case VK_BUILD_ACCELERATION_STRUCTURE_MODE_BUILD_KHR:
            return size_info.buildScratchSize;
        case VK_BUILD_ACCELERATION_STRUCTURE_MODE_UPDATE_KHR:
            return size_info.updateScratchSize;
        default:
            assert(false);
            return static_cast<VkDeviceSize>(0);
            break;
    }
}
}  // namespace rt
