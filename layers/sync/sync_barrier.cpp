/* Copyright (c) 2025 The Khronos Group Inc.
 * Copyright (c) 2025 Valve Corporation
 * Copyright (c) 2025 LunarG, Inc.
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

#include "sync_barrier.h"
#include "utils/sync_utils.h"
#include <vulkan/utility/vk_struct_helper.hpp>

static VkAccessFlags2 ExpandAccessFlags(VkAccessFlags2 access_mask) {
    VkAccessFlags2 expanded = access_mask;
    if (access_mask & VK_ACCESS_2_SHADER_READ_BIT) {
        expanded = expanded & ~VK_ACCESS_2_SHADER_READ_BIT;
        expanded |= kShaderReadExpandBits;
    }
    if (access_mask & VK_ACCESS_2_SHADER_WRITE_BIT) {
        expanded = expanded & ~VK_ACCESS_2_SHADER_WRITE_BIT;
        expanded |= kShaderWriteExpandBits;
    }
    return expanded;
}

template <typename Flags, typename Map>
static SyncAccessFlags AccessScopeImpl(Flags flag_mask, const Map &map) {
    SyncAccessFlags scope;
    for (const auto &[flag_bits2, sync_access_flags] : map) {
        if (flag_mask < flag_bits2) {
            break;
        }
        if (flag_mask & flag_bits2) {
            scope |= sync_access_flags;
        }
    }
    return scope;
}

static SyncAccessFlags AccessScopeByStage(VkPipelineStageFlags2 stages) {
    return AccessScopeImpl(stages, syncAccessMaskByStageBit());
}

static SyncAccessFlags AccessScopeByAccess(VkAccessFlags2 accesses) {
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

static VkPipelineStageFlags2 RelatedPipelineStages(
    VkPipelineStageFlags2 stage_mask,
    const vvl::unordered_map<VkPipelineStageFlagBits2, VkPipelineStageFlags2> &earlier_or_later_stages) {
    VkPipelineStageFlags2 unscanned = stage_mask;
    VkPipelineStageFlags2 related = 0;
    for (const auto &[stage, related_stages] : earlier_or_later_stages) {
        if (stage & unscanned) {
            related |= related_stages;
            unscanned &= ~stage;
            if (!unscanned) {
                break;
            }
        }
    }
    return related;
}

static VkPipelineStageFlags2 WithEarlierPipelineStages(VkPipelineStageFlags2 stage_mask) {
    return stage_mask | RelatedPipelineStages(stage_mask, syncLogicallyEarlierStages());
}

static VkPipelineStageFlags2 WithLaterPipelineStages(VkPipelineStageFlags2 stage_mask) {
    return stage_mask | RelatedPipelineStages(stage_mask, syncLogicallyLaterStages());
}

static SyncAccessFlags AccessScope(const SyncAccessFlags &stage_scope, VkAccessFlags2 accesses) {
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

namespace syncval {

SyncExecScope SyncExecScope::MakeSrc(VkQueueFlags queue_flags, VkPipelineStageFlags2 mask_param,
                                     VkPipelineStageFlags2 disabled_feature_mask) {
    const VkPipelineStageFlags2 expanded_mask = sync_utils::ExpandPipelineStages(mask_param, queue_flags, disabled_feature_mask);

    SyncExecScope result;
    result.mask_param = mask_param;
    result.exec_scope = WithEarlierPipelineStages(expanded_mask);
    result.valid_accesses = AccessScopeByStage(expanded_mask);

    // ALL_COMMANDS stage includes all accesses performed by the gpu, not only accesses defined by the stages
    if (mask_param & VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT) {
        result.valid_accesses |= SYNC_IMAGE_LAYOUT_TRANSITION_BIT;
    }
    return result;
}

SyncExecScope SyncExecScope::MakeDst(VkQueueFlags queue_flags, VkPipelineStageFlags2 mask_param) {
    const VkPipelineStageFlags2 expanded_mask = sync_utils::ExpandPipelineStages(mask_param, queue_flags);

    SyncExecScope result;
    result.mask_param = mask_param;
    result.exec_scope = WithLaterPipelineStages(expanded_mask);
    result.valid_accesses = AccessScopeByStage(expanded_mask);

    // ALL_COMMANDS stage includes all accesses performed by the gpu, not only accesses defined by the stages
    if (mask_param & VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT) {
        result.valid_accesses |= SYNC_IMAGE_LAYOUT_TRANSITION_BIT;
    }
    return result;
}

bool SyncExecScope::operator==(const SyncExecScope &other) const {
    return mask_param == other.mask_param && exec_scope == other.exec_scope && valid_accesses == other.valid_accesses;
}

size_t SyncExecScope::Hash() const {
    hash_util::HashCombiner hc;
    hc << mask_param;
    hc << exec_scope;
    valid_accesses.HashCombine(hc);
    return hc.Value();
}

SyncBarrier::SyncBarrier(const SyncExecScope &src_exec, const SyncExecScope &dst_exec)
    : src_exec_scope(src_exec), dst_exec_scope(dst_exec) {}

SyncBarrier::SyncBarrier(const SyncExecScope &src_exec, const SyncExecScope &dst_exec, const SyncBarrier::AllAccess &)
    : src_exec_scope(src_exec),
      src_access_scope(src_exec.valid_accesses),
      dst_exec_scope(dst_exec),
      dst_access_scope(dst_exec.valid_accesses) {}

SyncBarrier::SyncBarrier(const SyncExecScope &src_exec, VkAccessFlags2 src_access_mask, const SyncExecScope &dst_exec,
                         VkAccessFlags2 dst_access_mask)
    : src_exec_scope(src_exec),
      src_access_scope(AccessScope(src_exec.valid_accesses, src_access_mask)),
      original_src_access(src_access_mask),
      dst_exec_scope(dst_exec),
      dst_access_scope(AccessScope(dst_exec.valid_accesses, dst_access_mask)),
      original_dst_access(dst_access_mask) {}

SyncBarrier::SyncBarrier(VkQueueFlags queue_flags, const VkSubpassDependency2 &subpass) {
    const auto barrier = vku::FindStructInPNextChain<VkMemoryBarrier2>(subpass.pNext);
    if (barrier) {
        auto src = SyncExecScope::MakeSrc(queue_flags, barrier->srcStageMask);
        src_exec_scope = src;
        src_access_scope = AccessScope(src.valid_accesses, barrier->srcAccessMask);
        original_src_access = barrier->srcAccessMask;

        auto dst = SyncExecScope::MakeDst(queue_flags, barrier->dstStageMask);
        dst_exec_scope = dst;
        dst_access_scope = AccessScope(dst.valid_accesses, barrier->dstAccessMask);
        original_dst_access = barrier->dstAccessMask;
    } else {
        auto src = SyncExecScope::MakeSrc(queue_flags, subpass.srcStageMask);
        src_exec_scope = src;
        src_access_scope = AccessScope(src.valid_accesses, subpass.srcAccessMask);
        original_src_access = subpass.srcAccessMask;

        auto dst = SyncExecScope::MakeDst(queue_flags, subpass.dstStageMask);
        dst_exec_scope = dst;
        dst_access_scope = AccessScope(dst.valid_accesses, subpass.dstAccessMask);
        original_dst_access = subpass.dstAccessMask;
    }
}

SyncBarrier::SyncBarrier(const std::vector<SyncBarrier> &barriers) {
    // Merge each barrier
    for (const SyncBarrier &barrier : barriers) {
        // Note that after merge, only the exec_scope and access_scope fields are fully valid
        // TODO: Do we need to update any of the other fields?  Merging has limited application.
        src_exec_scope.exec_scope |= barrier.src_exec_scope.exec_scope;
        src_access_scope |= barrier.src_access_scope;
        dst_exec_scope.exec_scope |= barrier.dst_exec_scope.exec_scope;
        dst_access_scope |= barrier.dst_access_scope;
    }
}

bool SyncBarrier::operator==(const SyncBarrier &other) const {
    return (src_exec_scope == other.src_exec_scope) && (src_access_scope == other.src_access_scope) &&
           (dst_exec_scope == other.dst_exec_scope) && (dst_access_scope == other.dst_access_scope);
}

size_t SyncBarrier::Hash() const {
    hash_util::HashCombiner hc;
    hc << src_exec_scope.Hash();
    src_access_scope.HashCombine(hc);
    hc << dst_exec_scope.Hash();
    dst_access_scope.HashCombine(hc);
    return hc.Value();
}

}  // namespace syncval
