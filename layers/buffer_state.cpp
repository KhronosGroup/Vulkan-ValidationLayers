/* Copyright (c) 2015-2022 The Khronos Group Inc.
 * Copyright (c) 2015-2022 Valve Corporation
 * Copyright (c) 2015-2022 LunarG, Inc.
 * Copyright (C) 2015-2022 Google Inc.
 * Modifications Copyright (C) 2020 Advanced Micro Devices, Inc. All rights reserved.
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
 * Author: Courtney Goeltzenleuchter <courtneygo@google.com>
 * Author: Tobin Ehlis <tobine@google.com>
 * Author: Chris Forbes <chrisf@ijw.co.nz>
 * Author: Mark Lobodzinski <mark@lunarg.com>
 * Author: Dave Houlton <daveh@lunarg.com>
 * Author: John Zulauf <jzulauf@lunarg.com>
 * Author: Tobias Hector <tobias.hector@amd.com>
 * Author: Jeremy Gebben <jeremyg@lunarg.com>
 */
#include "buffer_state.h"
#include "layer_chassis_dispatch.h"
#include "state_tracker.h"

static VkExternalMemoryHandleTypeFlags GetExternalHandleType(const VkBufferCreateInfo *create_info) {
    const auto *external_memory_info = LvlFindInChain<VkExternalMemoryBufferCreateInfo>(create_info->pNext);
    return external_memory_info ? external_memory_info->handleTypes : 0;
}

static VkMemoryRequirements GetMemoryRequirements(ValidationStateTracker *dev_data, VkBuffer buffer) {
    VkMemoryRequirements result{};
    DispatchGetBufferMemoryRequirements(dev_data->device, buffer, &result);
    return result;
}

BUFFER_STATE::BUFFER_STATE(ValidationStateTracker *dev_data, VkBuffer buff, const VkBufferCreateInfo *pCreateInfo)
    : BINDABLE(buff, kVulkanObjectTypeBuffer, (pCreateInfo->flags & VK_BUFFER_CREATE_SPARSE_BINDING_BIT) != 0,
               (pCreateInfo->flags & VK_BUFFER_CREATE_SPARSE_RESIDENCY_BIT) != 0,
               (pCreateInfo->flags & VK_BUFFER_CREATE_PROTECTED_BIT) == 0, GetExternalHandleType(pCreateInfo)),
      safe_create_info(pCreateInfo),
      createInfo(*safe_create_info.ptr()),
      deviceAddress(0),
      requirements(GetMemoryRequirements(dev_data, buff)),
      memory_requirements_checked(false) {
    resource_size = requirements.size;
}

bool BUFFER_STATE::DoesBoundMemoryOverlap(const sparse_container::range<VkDeviceSize> &src_region, const BUFFER_STATE *dst,
                                          const sparse_container::range<VkDeviceSize> &dst_region) const {
    if (Invalid() || dst->Invalid()) return false;

    auto src_ranges = ComputeMemoryRanges(src_region);
    auto dst_ranges = dst->ComputeMemoryRanges(dst_region);

    for (const auto &src_mem : src_ranges) {
        auto it = dst_ranges.find(src_mem.first);
        if (it != dst_ranges.end()) {
            for (const auto &dst_r : it->second) {
                auto collision = src_mem.second.bounds(dst_r.first);
                if (collision.begin != src_mem.second.end()) return true;
            }
        }
    }

    return false;
}

BUFFER_STATE::DevMemRanges BUFFER_STATE::ComputeMemoryRanges(const sparse_container::range<VkDeviceSize> &region) const {
    DevMemRanges mem_ranges;

    for (const auto &item : binding_map_) {
        if (item.second.device_memory != VK_NULL_HANDLE) {
            auto &range_map = mem_ranges[item.second.device_memory];

            // Transform range from resource space to memory space
            auto source_range = item.first;
            source_range.begin -= item.second.resource_offset;
            source_range.end -= item.second.resource_offset;
            source_range.begin += item.second.memory_offset;
            source_range.end += item.second.memory_offset;

            // Adjust range to check left for merge
            auto range = source_range;
            range.begin = range.begin > 1 ? range.begin - 1 : 0;

            auto intersecting_ranges = range_map.bounds(range);
            if (intersecting_ranges.begin != range_map.end()) {
                // See if we need to merge left
                auto left = intersecting_ranges.begin;
                if (left->first.includes(range.begin)) source_range.begin = left->first.begin;

                // See if we need to merge right
                auto right = intersecting_ranges.end;
                if (right == range_map.end()) --right;  // Happens when range.end is greater than map's rightest value
                if (right->first.includes(range.end)) source_range.end = right->first.end;
            }

            std::pair<sparse_container::range<VkDeviceSize>, unsigned> insert_value{source_range, 0u};
            range_map.overwrite_range(insert_value);
        }
    }

    return mem_ranges;
}
