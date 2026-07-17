/* Copyright (c) 2025-2026 The Khronos Group Inc.
 * Copyright (c) 2025-2026 Valve Corporation
 * Copyright (c) 2025-2026 LunarG, Inc.
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
#include "sync_image.h"
#include "sync_validation.h"
#include "state_tracker/buffer_state.h"
#include "state_tracker/image_state.h"
#include "utils/image_utils.h"
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
static SyncAccessFlags AccessScopeImpl(Flags flag_mask, const Map& map) {
    SyncAccessFlags scope;
    for (const auto& [flag_bits2, sync_access_flags] : map) {
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

static SyncAccessFlags AccessScope(const SyncAccessFlags& stage_scope, VkAccessFlags2 accesses) {
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

SyncExecScope SyncExecScope::MakeSrc(VkQueueFlags queue_flags, VkPipelineStageFlags2 stage_mask,
                                     VkPipelineStageFlags2 disabled_feature_mask) {
    const VkPipelineStageFlags2 expanded_mask = sync_utils::ExpandPipelineStages(stage_mask, queue_flags, disabled_feature_mask);

    SyncExecScope result;
    result.stage_mask = stage_mask;
    result.exec_scope = sync_utils::AddEarlierPipelineStages(expanded_mask);
    result.stage_mask_accesses = AccessScopeByStage(expanded_mask);
    result.exec_scope_accesses = AccessScopeByStage(result.exec_scope);

    // ALL_COMMANDS stage includes all operations performed by the gpu, not only operations that run on the stages.
    // BOTTOM_OF_PIPE has no accesses of its own, so does not add to stage_mask_accesses, but in the context of
    // semaphoer scopes it adds to exec_scope_accesses
    if (stage_mask & VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT) {
        result.stage_mask_accesses |= SYNC_IMAGE_LAYOUT_TRANSITION_BIT;
    }
    if (stage_mask & (VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT | VK_PIPELINE_STAGE_2_BOTTOM_OF_PIPE_BIT)) {
        result.exec_scope_accesses |= SYNC_IMAGE_LAYOUT_TRANSITION_BIT;
    }
    return result;
}

SyncExecScope SyncExecScope::MakeDst(VkQueueFlags queue_flags, VkPipelineStageFlags2 stage_mask) {
    const VkPipelineStageFlags2 expanded_mask = sync_utils::ExpandPipelineStages(stage_mask, queue_flags);

    SyncExecScope result;
    result.stage_mask = stage_mask;
    result.exec_scope = sync_utils::AddLaterPipelineStages(expanded_mask);
    result.stage_mask_accesses = AccessScopeByStage(expanded_mask);
    result.exec_scope_accesses = AccessScopeByStage(result.exec_scope);

    // ALL_COMMANDS stage includes all accesses performed by the gpu, not only accesses defined by the stages.
    // TOP_OF_PIPE has no accesses of its own, so does not add to stage_mask_accesses, but in the context of
    // semaphore scopes it adds to exec_scope_accesses
    if (stage_mask & VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT) {
        result.stage_mask_accesses |= SYNC_IMAGE_LAYOUT_TRANSITION_BIT;
    }
    if (stage_mask & (VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT | VK_PIPELINE_STAGE_2_TOP_OF_PIPE_BIT)) {
        result.exec_scope_accesses |= SYNC_IMAGE_LAYOUT_TRANSITION_BIT;
    }
    return result;
}

bool SyncExecScope::operator==(const SyncExecScope& other) const {
    // Check that the fields are packed without gaps so we can use fast memcmp.
    // If not true, switch to memberwise compare
    static_assert(sizeof(SyncExecScope) == 64, "Gap detected, use memberwise compare");
    return memcmp(this, &other, sizeof(SyncExecScope)) == 0;
}

size_t SyncExecScope::Hash() const {
    hash_util::HashCombiner hc;
    hc << stage_mask;
    hc << exec_scope;
    stage_mask_accesses.HashCombine(hc);
    exec_scope_accesses.HashCombine(hc);
    return hc.Value();
}

SyncBarrier::SyncBarrier(const SyncExecScope& src_exec, const SyncExecScope& dst_exec)
    : src_exec_scope(src_exec), dst_exec_scope(dst_exec) {}

SyncBarrier::SyncBarrier(const SyncExecScope& src_exec, const SyncExecScope& dst_exec, const SyncBarrier::AllAccess&)
    : src_exec_scope(src_exec),
      src_access_scope(src_exec.exec_scope_accesses),
      dst_exec_scope(dst_exec),
      dst_access_scope(dst_exec.exec_scope_accesses) {}

SyncBarrier::SyncBarrier(const SyncExecScope& src_exec, VkAccessFlags2 src_access_mask, const SyncExecScope& dst_exec,
                         VkAccessFlags2 dst_access_mask)
    : src_exec_scope(src_exec),
      src_access_scope(AccessScope(src_exec.stage_mask_accesses, src_access_mask)),
      original_src_access(src_access_mask),
      dst_exec_scope(dst_exec),
      dst_access_scope(AccessScope(dst_exec.stage_mask_accesses, dst_access_mask)),
      original_dst_access(dst_access_mask) {}

SyncBarrier::SyncBarrier(VkQueueFlags queue_flags, const VkSubpassDependency2& subpass) {
    const auto barrier = vku::FindStructInPNextChain<VkMemoryBarrier2>(subpass.pNext);
    if (barrier) {
        auto src = SyncExecScope::MakeSrc(queue_flags, barrier->srcStageMask);
        src_exec_scope = src;
        src_access_scope = AccessScope(src.stage_mask_accesses, barrier->srcAccessMask);
        original_src_access = barrier->srcAccessMask;

        auto dst = SyncExecScope::MakeDst(queue_flags, barrier->dstStageMask);
        dst_exec_scope = dst;
        dst_access_scope = AccessScope(dst.stage_mask_accesses, barrier->dstAccessMask);
        original_dst_access = barrier->dstAccessMask;
    } else {
        auto src = SyncExecScope::MakeSrc(queue_flags, subpass.srcStageMask);
        src_exec_scope = src;
        src_access_scope = AccessScope(src.stage_mask_accesses, subpass.srcAccessMask);
        original_src_access = subpass.srcAccessMask;

        auto dst = SyncExecScope::MakeDst(queue_flags, subpass.dstStageMask);
        dst_exec_scope = dst;
        dst_access_scope = AccessScope(dst.stage_mask_accesses, subpass.dstAccessMask);
        original_dst_access = subpass.dstAccessMask;
    }
}

SyncBarrier::SyncBarrier(const std::vector<SyncBarrier>& barriers) {
    // Merge each barrier
    for (const SyncBarrier& barrier : barriers) {
        // Note that after merge, only the exec_scope and access_scope fields are fully valid
        // TODO: Do we need to update any of the other fields?  Merging has limited application.
        src_exec_scope.exec_scope |= barrier.src_exec_scope.exec_scope;
        src_access_scope |= barrier.src_access_scope;
        dst_exec_scope.exec_scope |= barrier.dst_exec_scope.exec_scope;
        dst_access_scope |= barrier.dst_access_scope;
    }
}

bool SyncBarrier::operator==(const SyncBarrier& other) const {
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

BarrierSet::BarrierSet(const SyncValidator& sync_state, VkQueueFlags queue_flags, const VkDependencyInfo& dep_info) {
    const ExecScopes stage_masks = sync_utils::GetExecScopes(dep_info);
    src_exec_scope = SyncExecScope::MakeSrc(queue_flags, stage_masks.src);
    dst_exec_scope = SyncExecScope::MakeDst(queue_flags, stage_masks.dst);

    MakeMemoryBarriers(queue_flags, dep_info);
    MakeBufferMemoryBarriers(sync_state, queue_flags, dep_info.bufferMemoryBarrierCount, dep_info.pBufferMemoryBarriers);
    MakeImageMemoryBarriers(sync_state, queue_flags, dep_info.imageMemoryBarrierCount, dep_info.pImageMemoryBarriers,
                            sync_state.device_state->extensions);
}

BarrierSet::BarrierSet(const SyncValidator& sync_state, const SyncExecScope& src_exec_scope, const SyncExecScope& dst_exec_scope,
                       uint32_t memory_barrier_count, const VkMemoryBarrier* memory_barriers, uint32_t buffer_barrier_count,
                       const VkBufferMemoryBarrier* buffer_barriers, uint32_t image_barrier_count,
                       const VkImageMemoryBarrier* image_barriers)
    : src_exec_scope(src_exec_scope), dst_exec_scope(dst_exec_scope) {
    MakeMemoryBarriers(src_exec_scope, dst_exec_scope, memory_barrier_count, memory_barriers);
    MakeBufferMemoryBarriers(sync_state, src_exec_scope, dst_exec_scope, buffer_barrier_count, buffer_barriers);
    MakeImageMemoryBarriers(sync_state, src_exec_scope, dst_exec_scope, image_barrier_count, image_barriers,
                            sync_state.device_state->extensions);
}

void BarrierSet::MakeMemoryBarriers(const SyncExecScope& src, const SyncExecScope& dst, uint32_t barrier_count,
                                    const VkMemoryBarrier* barriers) {
    memory_barriers.reserve(std::max<uint32_t>(1, barrier_count));
    for (const VkMemoryBarrier& barrier : vvl::make_span(barriers, barrier_count)) {
        SyncBarrier sync_barrier(src, barrier.srcAccessMask, dst, barrier.dstAccessMask);
        memory_barriers.emplace_back(sync_barrier);
    }

    // Ensure we have a barrier that handles execution dependencies.
    // NOTE: the reason to have execution barrier is explained in details in the comment for Sync2
    // MakeMemoryBarriers overload. The Sync1 implementation is much simpler since execution scopes
    // are the same for all barriers.
    if (barrier_count == 0) {
        memory_barriers.emplace_back(SyncBarrier(src, dst));
    }
    single_exec_scope = true;
    execution_dependency_barrier_count = (barrier_count == 0) ? 1 : 0;
}

void BarrierSet::MakeMemoryBarriers(VkQueueFlags queue_flags, const VkDependencyInfo& dep_info) {
    // Collect unique execution dependencies from buffer and image barriers.
    //
    // NOTE: the reason to collect execution dependencies in addition to original buffer/image
    // barriers is because syncval applies buffer/image barriers to the memory ranges defined
    // by the resource. But execution dependency can affect any resource/memory range, not
    // only the one specified by the barrier. For example, execution dependency synchronizes
    // all READ accesses that are in scope. To emulate this behavior we collect unique
    // execution dependencies and apply them to all memory accesses (don't specify access mask).
    small_vector<std::pair<VkPipelineStageFlags2, VkPipelineStageFlags2>, 4> buffer_image_barrier_exec_deps;
    for (const VkBufferMemoryBarrier2& buffer_barrier :
         vvl::make_span(dep_info.pBufferMemoryBarriers, dep_info.bufferMemoryBarrierCount)) {
        const auto src_dst = std::make_pair(buffer_barrier.srcStageMask, buffer_barrier.dstStageMask);
        if (!buffer_image_barrier_exec_deps.Contains(src_dst)) {
            buffer_image_barrier_exec_deps.emplace_back(src_dst);
        }
    }
    for (const VkImageMemoryBarrier2& image_barrier :
         vvl::make_span(dep_info.pImageMemoryBarriers, dep_info.imageMemoryBarrierCount)) {
        const auto src_dst = std::make_pair(image_barrier.srcStageMask, image_barrier.dstStageMask);
        if (!buffer_image_barrier_exec_deps.Contains(src_dst)) {
            buffer_image_barrier_exec_deps.emplace_back(src_dst);
        }
    }

    memory_barriers.reserve(dep_info.memoryBarrierCount + buffer_image_barrier_exec_deps.size());

    // Add global memory barriers specified in VkDependencyInfo
    for (const VkMemoryBarrier2& barrier : vvl::make_span(dep_info.pMemoryBarriers, dep_info.memoryBarrierCount)) {
        auto src = SyncExecScope::MakeSrc(queue_flags, barrier.srcStageMask);
        auto dst = SyncExecScope::MakeDst(queue_flags, barrier.dstStageMask);
        memory_barriers.emplace_back(SyncBarrier(src, barrier.srcAccessMask, dst, barrier.dstAccessMask));
    }
    // Add execution dependencies from buffer and image barriers
    for (const auto& src_dst : buffer_image_barrier_exec_deps) {
        auto src = SyncExecScope::MakeSrc(queue_flags, src_dst.first);
        auto dst = SyncExecScope::MakeDst(queue_flags, src_dst.second);
        memory_barriers.emplace_back(SyncBarrier(src, dst));
    }
    single_exec_scope = false;
    execution_dependency_barrier_count = (uint32_t)buffer_image_barrier_exec_deps.size();
}

void BarrierSet::MakeBufferMemoryBarriers(const SyncValidator& sync_state, const SyncExecScope& src, const SyncExecScope& dst,
                                          uint32_t barrier_count, const VkBufferMemoryBarrier* barriers) {
    buffer_barriers.reserve(barrier_count);
    for (const VkBufferMemoryBarrier& barrier : vvl::make_span(barriers, barrier_count)) {
        if (auto buffer = sync_state.Get<vvl::Buffer>(barrier.buffer)) {
            const auto range = MakeRange(*buffer, barrier.offset, barrier.size);
            const SyncBarrier sync_barrier(src, barrier.srcAccessMask, dst, barrier.dstAccessMask);
            buffer_barriers.emplace_back(buffer, sync_barrier, range);
        }
    }
}

void BarrierSet::MakeBufferMemoryBarriers(const SyncValidator& sync_state, VkQueueFlags queue_flags, uint32_t barrier_count,
                                          const VkBufferMemoryBarrier2* barriers) {
    buffer_barriers.reserve(barrier_count);
    for (const VkBufferMemoryBarrier2& barrier : vvl::make_span(barriers, barrier_count)) {
        auto src = SyncExecScope::MakeSrc(queue_flags, barrier.srcStageMask);
        auto dst = SyncExecScope::MakeDst(queue_flags, barrier.dstStageMask);
        if (auto buffer = sync_state.Get<vvl::Buffer>(barrier.buffer)) {
            const auto range = MakeRange(*buffer, barrier.offset, barrier.size);
            const SyncBarrier sync_barrier(src, barrier.srcAccessMask, dst, barrier.dstAccessMask);
            buffer_barriers.emplace_back(buffer, sync_barrier, range);
        }
    }
}

void BarrierSet::MakeImageMemoryBarriers(const SyncValidator& sync_state, const SyncExecScope& src, const SyncExecScope& dst,
                                         uint32_t barrier_count, const VkImageMemoryBarrier* barriers,
                                         const DeviceExtensions& extensions) {
    image_barriers.reserve(barrier_count);
    for (const auto [index, barrier] : vvl::enumerate(barriers, barrier_count)) {
        if (auto image = sync_state.Get<vvl::Image>(barrier.image)) {
            auto subresource_range = image->NormalizeSubresourceRange(barrier.subresourceRange);

            // VK_REMAINING_ARRAY_LAYERS for sliced 3d image in the context of layout transition means image's depth extent.
            if (barrier.subresourceRange.layerCount == VK_REMAINING_ARRAY_LAYERS &&
                CanTransitionDepthSlices(extensions, image->GetImageType(), image->create_flags)) {
                subresource_range.layerCount = image->GetExtent().depth - subresource_range.baseArrayLayer;
            }

            const SyncBarrier sync_barrier(src, barrier.srcAccessMask, dst, barrier.dstAccessMask);
            const bool layout_transition = barrier.oldLayout != barrier.newLayout;
            image_barriers.emplace_back(image, sync_barrier, subresource_range, layout_transition, index);
        }
    }
}

void BarrierSet::MakeImageMemoryBarriers(const SyncValidator& sync_state, VkQueueFlags queue_flags, uint32_t barrier_count,
                                         const VkImageMemoryBarrier2* barriers, const DeviceExtensions& extensions) {
    image_barriers.reserve(barrier_count);
    for (const auto [index, barrier] : vvl::enumerate(barriers, barrier_count)) {
        auto src = SyncExecScope::MakeSrc(queue_flags, barrier.srcStageMask);
        auto dst = SyncExecScope::MakeDst(queue_flags, barrier.dstStageMask);
        auto image = sync_state.Get<vvl::Image>(barrier.image);
        if (image) {
            auto subresource_range = image->NormalizeSubresourceRange(barrier.subresourceRange);

            // VK_REMAINING_ARRAY_LAYERS for sliced 3d image in the context of layout transition means image's depth extent.
            if (barrier.subresourceRange.layerCount == VK_REMAINING_ARRAY_LAYERS &&
                CanTransitionDepthSlices(extensions, image->GetImageType(), image->create_flags)) {
                subresource_range.layerCount = image->GetExtent().depth - subresource_range.baseArrayLayer;
            }

            const SyncBarrier sync_barrier(src, barrier.srcAccessMask, dst, barrier.dstAccessMask);
            const bool layout_transition = barrier.oldLayout != barrier.newLayout;
            image_barriers.emplace_back(image, sync_barrier, subresource_range, layout_transition, index);
        }
    }
}

//
// A single barrier can be applied more efficently (immidiately) compared to multiple barrier.
// The latter are applied in two steps (collect and then apply)
//
static void ApplySingleBufferBarrier(CommandExecutionContext& exec_context, const SyncBufferBarrier& buffer_barrier,
                                     const SyncBarrier& exec_dep_barrier) {
    AccessContext& access_context = exec_context.GetCurrentAccessContext();
    const QueueId queue_id = exec_context.GetQueueId();

    if (SimpleBinding(*buffer_barrier.buffer)) {
        const BarrierScope barrier_scope(buffer_barrier.barrier, queue_id);
        ApplySingleBufferBarrierFunctor apply_barrier(access_context, barrier_scope, buffer_barrier.barrier);

        const VkDeviceSize base_address = ResourceBaseAddress(*buffer_barrier.buffer);
        const AccessRange range = buffer_barrier.range + base_address;

        access_context.UpdateMemoryAccessState(apply_barrier, range);
    }
    access_context.RegisterGlobalBarrier(exec_dep_barrier, queue_id);
}

static void ApplySingleImageBarrier(CommandExecutionContext& exec_context, const SyncImageBarrier& image_barrier,
                                    const SyncBarrier& exec_dep_barrier, ResourceUsageTag tag) {
    AccessContext& access_context = exec_context.GetCurrentAccessContext();
    const QueueId queue_id = exec_context.GetQueueId();

    const BarrierScope barrier_scope(image_barrier.barrier, queue_id);
    ApplySingleImageBarrierFunctor apply_barrier(access_context, barrier_scope, image_barrier.barrier,
                                                 image_barrier.layout_transition, image_barrier.handle_index, tag);

    const auto& sub_state = SubState(*image_barrier.image);
    const bool can_transition_depth_slices = CanTransitionDepthSlices(exec_context.GetSyncState().extensions,
                                                                      sub_state.base.GetImageType(), sub_state.base.create_flags);
    auto range_gen = sub_state.MakeImageRangeGen(image_barrier.subresource_range, can_transition_depth_slices);

    access_context.UpdateMemoryAccessState(apply_barrier, range_gen);
    access_context.RegisterGlobalBarrier(exec_dep_barrier, queue_id);
}

static void ApplySingleMemoryBarrier(CommandExecutionContext& exec_context, const SyncBarrier& memory_barrier) {
    AccessContext& access_context = exec_context.GetCurrentAccessContext();
    const QueueId queue_id = exec_context.GetQueueId();
    access_context.RegisterGlobalBarrier(memory_barrier, queue_id);
}

// This handles all configurations where barriers cannot be applied immidiately and need to use
// the PendingBarriers helper to ensure independent barrier application. All such configurations
// use more than one barrier.
static void ApplyMultipleBarriers(CommandExecutionContext& exec_context, const BarrierSet& barrier_set, ResourceUsageTag tag) {
    AccessContext& access_context = exec_context.GetCurrentAccessContext();
    const QueueId queue_id = exec_context.GetQueueId();

    // Apply markup action.
    // The markup action does not change any access state but it can trim the access map according to the
    // provided range and creates infill ranges if necessary (for layout transitions). The purpose of all
    // this is to ensure that after markup action the topology of access map ranges is finalized so we can
    // safely cache pointers to specific access states with a goal to apply pending barriers in the end.
    //
    // NOTE: it is enough to apply markup action to buffer and image barriers. The global barriers
    // do not use infill operations (no layout transitions) and also do not split access map ranges
    // because global barriers are applied to the full range.
    for (const SyncBufferBarrier& barrier : barrier_set.buffer_barriers) {
        if (SimpleBinding(*barrier.buffer)) {
            const VkDeviceSize base_address = ResourceBaseAddress(*barrier.buffer);
            const AccessRange range = barrier.range + base_address;
            ApplyMarkupFunctor markup_action(false);
            access_context.UpdateMemoryAccessState(markup_action, range);
        }
    }
    for (const SyncImageBarrier& barrier : barrier_set.image_barriers) {
        const auto& sub_state = SubState(*barrier.image);
        const bool can_transition_depth_slices = CanTransitionDepthSlices(
            exec_context.GetSyncState().extensions, sub_state.base.GetImageType(), sub_state.base.create_flags);
        auto range_gen = sub_state.MakeImageRangeGen(barrier.subresource_range, can_transition_depth_slices);
        // TODO: check if we need: barrier.layout_transition && (queue_id == kQueueIdInvalid)
        ApplyMarkupFunctor markup_action(barrier.layout_transition);
        access_context.UpdateMemoryAccessState(markup_action, range_gen);
    }

    // Use PendingBarriers to collect barriers that must be applied independently
    PendingBarriers pending_barriers;
    for (const SyncBufferBarrier& barrier : barrier_set.buffer_barriers) {
        if (SimpleBinding(*barrier.buffer)) {
            const BarrierScope barrier_scope(barrier.barrier, queue_id);
            CollectBarriersFunctor collect_barriers(access_context, barrier_scope, barrier.barrier, false, vvl::kNoIndex32,
                                                    pending_barriers);

            const VkDeviceSize base_address = ResourceBaseAddress(*barrier.buffer);
            const AccessRange range = barrier.range + base_address;

            access_context.UpdateMemoryAccessState(collect_barriers, range);
        }
    }
    for (const SyncImageBarrier& barrier : barrier_set.image_barriers) {
        const BarrierScope barrier_scope(barrier.barrier, queue_id);
        CollectBarriersFunctor collect_barriers(access_context, barrier_scope, barrier.barrier, barrier.layout_transition,
                                                barrier.handle_index, pending_barriers);

        const auto& sub_state = SubState(*barrier.image);
        const bool can_transition_depth_slices = CanTransitionDepthSlices(
            exec_context.GetSyncState().extensions, sub_state.base.GetImageType(), sub_state.base.create_flags);
        auto range_gen = sub_state.MakeImageRangeGen(barrier.subresource_range, can_transition_depth_slices);

        access_context.UpdateMemoryAccessState(collect_barriers, range_gen);
    }

    // Do kFullRange update only when there is multiple memory barriers.
    // For a single barrier we can use global barrier functionality.
    if (barrier_set.memory_barriers.size() > 1) {
        for (const SyncBarrier& barrier : barrier_set.memory_barriers) {
            const BarrierScope barrier_scope(barrier, queue_id);
            CollectBarriersFunctor collect_barriers(access_context, barrier_scope, barrier, false, vvl::kNoIndex32,
                                                    pending_barriers);
            access_context.UpdateMemoryAccessState(collect_barriers, kFullRange);
        }
    }

    // Apply collected barriers to access states
    pending_barriers.Apply(tag);

    // Register global barriers if we have the only memory barrier (likely execution dependency)
    if (barrier_set.memory_barriers.size() == 1) {
        access_context.RegisterGlobalBarrier(barrier_set.memory_barriers[0], queue_id);
    }
}

void ApplyBarrier(CommandExecutionContext& exec_context, const BarrierSet& barrier_set, ResourceUsageTag tag) {
    const bool has_buffer_barriers = !barrier_set.buffer_barriers.empty();
    const bool has_image_barriers = !barrier_set.image_barriers.empty();

    const bool single_buffer_barrier = barrier_set.buffer_barriers.size() == 1 &&
                                       barrier_set.memory_barriers.size() == 1 &&  // buffer barrier exec dependency
                                       !has_image_barriers;

    const bool single_image_barrier = barrier_set.image_barriers.size() == 1 &&
                                      barrier_set.memory_barriers.size() == 1 &&  // image barrier exec dependency
                                      !has_buffer_barriers;

    const bool single_memory_barrier = barrier_set.memory_barriers.size() == 1 && !has_buffer_barriers && !has_image_barriers;

    if (single_buffer_barrier) {
        ApplySingleBufferBarrier(exec_context, barrier_set.buffer_barriers[0], barrier_set.memory_barriers[0]);
    } else if (single_image_barrier) {
        ApplySingleImageBarrier(exec_context, barrier_set.image_barriers[0], barrier_set.memory_barriers[0], tag);
    } else if (single_memory_barrier) {
        ApplySingleMemoryBarrier(exec_context, barrier_set.memory_barriers[0]);
    } else {
        ApplyMultipleBarriers(exec_context, barrier_set, tag);
    }

    SyncEventsContext& events_context = exec_context.GetEventsContext();
    if (barrier_set.single_exec_scope) {
        events_context.ApplyBarrier(barrier_set.src_exec_scope, barrier_set.dst_exec_scope, tag);
    } else {
        for (const auto& barrier : barrier_set.memory_barriers) {
            events_context.ApplyBarrier(barrier.src_exec_scope, barrier.dst_exec_scope, tag);
        }
    }
}

}  // namespace syncval
