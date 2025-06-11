/*
 * Copyright (c) 2019-2025 Valve Corporation
 * Copyright (c) 2019-2025 LunarG, Inc.
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

#include "sync/sync_common.h"
#include "state_tracker/buffer_state.h"

extern const ResourceAccessRange kFullRange(std::numeric_limits<VkDeviceSize>::min(), std::numeric_limits<VkDeviceSize>::max());

template <typename Flags, typename Map>
SyncAccessFlags AccessScopeImpl(Flags flag_mask, const Map& map) {
    SyncAccessFlags scope;
    for (const auto& bit_scope : map) {
        if (flag_mask < bit_scope.first) break;

        if (flag_mask & bit_scope.first) {
            scope |= bit_scope.second;
        }
    }
    return scope;
}

static VkAccessFlags2 ExpandAccessFlags(VkAccessFlags2 access_mask) {
    VkAccessFlags2 expanded = access_mask;

    if (VK_ACCESS_2_SHADER_READ_BIT & access_mask) {
        expanded = expanded & ~VK_ACCESS_2_SHADER_READ_BIT;
        expanded |= kShaderReadExpandBits;
    }

    if (VK_ACCESS_2_SHADER_WRITE_BIT & access_mask) {
        expanded = expanded & ~VK_ACCESS_2_SHADER_WRITE_BIT;
        expanded |= kShaderWriteExpandBits;
    }

    return expanded;
}

SyncAccessFlags SyncStageAccess::AccessScopeByStage(VkPipelineStageFlags2 stages) {
    return AccessScopeImpl(stages, syncAccessMaskByStageBit());
}

SyncAccessFlags SyncStageAccess::AccessScopeByAccess(VkAccessFlags2 accesses) {
    SyncAccessFlags sync_accesses = AccessScopeImpl(ExpandAccessFlags(accesses), syncAccessMaskByAccessBit());

    // The above access expansion replaces SHADER_READ meta access with atomic accesses as defined by the specification.
    // ACCELERATION_STRUCTURE_BUILD and MICROMAP_BUILD stages are special in a way that they use SHADER_READ access directly.
    // It is an implementation detail of how SHADER_READ is used by the driver, and we cannot make assumption about specific
    // atomic accesses. If we make such assumption then it can be a problem when after applying synchronization we won't be
    // able to get full SHADER_READ access back, but only a subset of accesses, for example, only SHADER_STORAGE_READ.
    // It would mean we made (incorrect) assumption how the driver represents SHADER_READ in the context of AS build.
    //
    // Handle special cases that use non-expanded meta accesses.
    if (accesses & VK_ACCESS_2_SHADER_READ_BIT) {
        sync_accesses |= SYNC_ACCELERATION_STRUCTURE_BUILD_SHADER_READ_BIT;
        sync_accesses |= SYNC_MICROMAP_BUILD_EXT_SHADER_READ_BIT;
    }

    return sync_accesses;
}

SyncAccessFlags SyncStageAccess::AccessScope(const SyncAccessFlags& stage_scope, VkAccessFlags2 accesses) {
    SyncAccessFlags access_scope = stage_scope & AccessScopeByAccess(accesses);

    // Special case. AS copy operations (e.g., vkCmdCopyAccelerationStructureKHR) can be synchronized using
    // the ACCELERATION_STRUCTURE_COPY stage, but it's also valid to use ACCELERATION_STRUCTURE_BUILD stage.
    // Internally, AS copy accesses are represented via ACCELERATION_STRUCTURE_COPY stage. The logic below
    // ensures that a barrier using ACCELERATION_STRUCTURE_BUILD stage can also protect accesses on
    // ACCELERATION_STRUCTURE_COPY stage.
    if (access_scope[SYNC_ACCELERATION_STRUCTURE_BUILD_ACCELERATION_STRUCTURE_READ]) {
        access_scope.set(SYNC_ACCELERATION_STRUCTURE_COPY_ACCELERATION_STRUCTURE_READ);
    }
    if (access_scope[SYNC_ACCELERATION_STRUCTURE_BUILD_ACCELERATION_STRUCTURE_WRITE]) {
        access_scope.set(SYNC_ACCELERATION_STRUCTURE_COPY_ACCELERATION_STRUCTURE_WRITE);
    }
    return access_scope;
}

ResourceAccessRange MakeRange(VkDeviceSize start, VkDeviceSize size) { return ResourceAccessRange(start, (start + size)); }

ResourceAccessRange MakeRange(const vvl::Buffer& buffer, VkDeviceSize offset, VkDeviceSize size) {
    return MakeRange(offset, buffer.GetRegionSize(offset, size));
}

ResourceAccessRange MakeRange(const vvl::BufferView& buf_view_state) {
    return MakeRange(*buf_view_state.buffer_state.get(), buf_view_state.create_info.offset, buf_view_state.create_info.range);
}

ResourceAccessRange MakeRange(VkDeviceSize offset, uint32_t first_index, uint32_t count, uint32_t stride) {
    const VkDeviceSize range_start = offset + (first_index * stride);
    const VkDeviceSize range_size = count * stride;
    return MakeRange(range_start, range_size);
}
