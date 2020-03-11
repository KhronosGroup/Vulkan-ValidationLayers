/* Copyright (c) 2019 The Khronos Group Inc.
 * Copyright (c) 2019 Valve Corporation
 * Copyright (c) 2019 LunarG, Inc.
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
 * Author: John Zulauf <jzulauf@lunarg.com>
 */

#include <limits>
#include <vector>
#include "synchronization_validation.h"

static const char *string_SyncHazardVUID(SyncHazard hazard) {
    switch (hazard) {
        case SyncHazard::NONE:
            return "SYNC-HAZARD-NONE";
            break;
        case SyncHazard::READ_AFTER_WRITE:
            return "SYNC-HAZARD-READ_AFTER_WRITE";
            break;
        case SyncHazard::WRITE_AFTER_READ:
            return "SYNC-HAZARD-WRITE_AFTER_READ";
            break;
        case SyncHazard::WRITE_AFTER_WRITE:
            return "SYNC-HAZARD-WRITE_AFTER_WRITE";
            break;
        case SyncHazard::READ_RACING_WRITE:
            return "SYNC-HAZARD-READ-RACING-WRITE";
            break;
        case SyncHazard::WRITE_RACING_WRITE:
            return "SYNC-HAZARD-WRITE-RACING-WRITE";
            break;
        case SyncHazard::WRITE_RACING_READ:
            return "SYNC-HAZARD-WRITE-RACING-READ";
            break;
        default:
            assert(0);
    }
    return "SYNC-HAZARD-INVALID";
}

static const char *string_SyncHazard(SyncHazard hazard) {
    switch (hazard) {
        case SyncHazard::NONE:
            return "NONR";
            break;
        case SyncHazard::READ_AFTER_WRITE:
            return "READ_AFTER_WRITE";
            break;
        case SyncHazard::WRITE_AFTER_READ:
            return "WRITE_AFTER_READ";
            break;
        case SyncHazard::WRITE_AFTER_WRITE:
            return "WRITE_AFTER_WRITE";
            break;
        case SyncHazard::READ_RACING_WRITE:
            return "READ_RACING_WRITE";
            break;
        case SyncHazard::WRITE_RACING_WRITE:
            return "WRITE_RACING_WRITE";
            break;
        case SyncHazard::WRITE_RACING_READ:
            return "WRITE_RACING_READ";
            break;
        default:
            assert(0);
    }
    return "INVALID HAZARD";
}

// Expand the pipeline stage without regard to whether the are valid w.r.t. queue or extension
VkPipelineStageFlags ExpandPipelineStages(VkQueueFlags queue_flags, VkPipelineStageFlags stage_mask) {
    VkPipelineStageFlags expanded = stage_mask;
    if (VK_PIPELINE_STAGE_ALL_COMMANDS_BIT & stage_mask) {
        expanded = expanded & ~VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;
        for (const auto &all_commands : syncAllCommandStagesByQueueFlags) {
            if (all_commands.first & queue_flags) {
                expanded |= all_commands.second;
            }
        }
    }
    if (VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT & stage_mask) {
        expanded = expanded & ~VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT;
        expanded |= syncAllCommandStagesByQueueFlags.at(VK_QUEUE_GRAPHICS_BIT) & ~VK_PIPELINE_STAGE_HOST_BIT;
    }
    return expanded;
}

VkPipelineStageFlags RelatedPipelineStages(VkPipelineStageFlags stage_mask,
                                           std::map<VkPipelineStageFlagBits, VkPipelineStageFlags> &map) {
    VkPipelineStageFlags unscanned = stage_mask;
    VkPipelineStageFlags related = 0;
    for (const auto entry : map) {
        const auto stage = entry.first;
        if (stage & unscanned) {
            related = related | entry.second;
            unscanned = unscanned & ~stage;
            if (!unscanned) break;
        }
    }
    return related;
}

VkPipelineStageFlags WithEarlierPipelineStages(VkPipelineStageFlags stage_mask) {
    return stage_mask | RelatedPipelineStages(stage_mask, syncLogicallyEarlierStages);
}

VkPipelineStageFlags WithLaterPipelineStages(VkPipelineStageFlags stage_mask) {
    return stage_mask | RelatedPipelineStages(stage_mask, syncLogicallyLaterStages);
}

static const ResourceAccessRange full_range(std::numeric_limits<VkDeviceSize>::min(), std::numeric_limits<VkDeviceSize>::max());
static ResourceAccessRange MakeMemoryAccessRange(const BUFFER_STATE &buffer, VkDeviceSize offset, VkDeviceSize size) {
    assert(!buffer.sparse);
    const auto base = offset + buffer.binding.offset;
    return ResourceAccessRange(base, base + size);
}

AccessTrackerContext::AccessTrackerContext(uint32_t subpass, VkQueueFlags queue_flags,
                                           const std::vector<SubpassDependencyGraphNode> &dependencies,
                                           const std::vector<AccessTrackerContext> &contexts,
                                           AccessTrackerContext *external_context) {
    Reset();
    const auto &subpass_dep = dependencies[subpass];
    prev_.reserve(subpass_dep.prev.size());
    for (const auto &prev_dep : subpass_dep.prev) {
        assert(prev_dep.dependency);
        const auto dep = *prev_dep.dependency;
        prev_.emplace_back(const_cast<AccessTrackerContext *>(&contexts[dep.srcSubpass]), queue_flags, dep);
    }

    async_.reserve(subpass_dep.async.size());
    for (const auto async_subpass : subpass_dep.async) {
        async_.emplace_back(const_cast<AccessTrackerContext *>(&contexts[async_subpass]));
    }
    if (subpass_dep.barrier_from_external)
        external_ = TrackBack(external_context, queue_flags, *subpass_dep.barrier_from_external);
    else {
        external_ = TrackBack();
    }
}

// A recursive range walker for hazard detection, first for the current context and the (DetectHazardRecur) to walk
// the DAG of the contexts (for example subpasses)
template <typename Detector>
HazardResult AccessTrackerContext::DetectHazard(const VulkanTypedHandle &handle, const Detector &detector,
                                                const ResourceAccessRange &range, SyncBarrierStack *barrier_stack) const {
    const auto access_tracker = GetAccessTracker(handle);
    bool whole_range_gap = true;
    HazardResult hazard;
    if (access_tracker) {
        auto accesses = access_tracker->GetCurrentAccessMap();
        const auto from = accesses.lower_bound(range);
        if (from != accesses.end() && from->first.intersects(range)) {
            whole_range_gap = false;
            const auto to = accesses.upper_bound(range);
            ResourceAccessRange gap = {range.begin, range.begin};
            for (auto pos = from; pos != to; ++pos) {
                hazard = detector.Detect(pos, barrier_stack);
                if (hazard.hazard) return hazard;

                // Check for short circuiting recursion over pos->first
                // If we've just checked against a write we don't have to looke any further...
                // however even if the current usage is write and there is not hazard with a stored read, we still have to
                // recur to ensure it's safe against *all* prior/parallel accesses.
                auto upper_bound = std::min(range.end, pos->first.end);
                if (detector.ShortCircuit(pos, barrier_stack)) {
                    gap.end = pos->first.begin;
                } else {
                    // make sure we don't go past range
                    gap.end = upper_bound;
                }
                if (!gap.empty()) {
                    // Must recur on all gaps
                    hazard = DetectHazardRecur(handle, detector, gap, barrier_stack);
                    if (hazard.hazard) return hazard;
                }
                gap.begin = upper_bound;
            }
        }
    }

    if (whole_range_gap) {  // There's no accesses recorded for this range for this context and resource
        hazard = DetectHazardRecur(handle, detector, range, barrier_stack);
    }
    // We only check the async list for top level contexts (empty stack)
    // Async check are not done at recursive levels, as the async lists are exhaustive for the top-level context
    if (barrier_stack->size() == 0) {
        for (const auto &async_context : async_) {
            hazard = async_context->DetectAsyncHazard(handle, detector, range);
            if (hazard.hazard) return hazard;
        }
    }

    return hazard;
}

// A non recursive range walker for the asynchronous contexts (those we have no barriers with)
template <typename Detector>
HazardResult AccessTrackerContext::DetectAsyncHazard(const VulkanTypedHandle &handle, const Detector &detector,
                                                     const ResourceAccessRange &range) const {
    const auto access_tracker = GetAccessTracker(handle);
    HazardResult hazard;
    if (access_tracker) {
        auto accesses = access_tracker->GetCurrentAccessMap();
        const auto from = accesses.lower_bound(range);
        const auto to = accesses.upper_bound(range);
        for (auto pos = from; pos != to; ++pos) {
            hazard = detector.DetectAsync(pos);
            if (hazard.hazard) break;
        }
    }
    return hazard;
}

// Walk the contents or the context's predecessors, checking for hazards invoking the recursive and non-recursive range walkers
// as appropriate for the type of predecessor
template <typename Detector>
HazardResult AccessTrackerContext::DetectHazardRecur(const VulkanTypedHandle &handle, const Detector &detector,
                                                     const ResourceAccessRange &range, SyncBarrierStack *barrier_stack) const {
    HazardResult hazard;

    assert(barrier_stack);  // Barrier stack is a pointer only by convention (non-const objects to be passed by pointer)
    for (const auto &prev_dep : prev_) {
        barrier_stack->push_back(&prev_dep.barrier);
        hazard = prev_dep.context->DetectHazard(handle, detector, range, barrier_stack);
        barrier_stack->pop_back();
        if (hazard.hazard) return hazard;
    }
    if (external_.context) {
        barrier_stack->push_back(&external_.barrier);
        hazard = external_.context->DetectHazard(handle, detector, range, barrier_stack);
        barrier_stack->pop_back();
        if (hazard.hazard) return hazard;
    }

    return hazard;
}

template <typename Detector>
HazardResult AccessTrackerContext::DetectHazard(const VulkanTypedHandle &handle, const Detector &detector,
                                                const ResourceAccessRange &range) const {
    SyncBarrierStack barrier_stack;
    return DetectHazard(handle, detector, range, &barrier_stack);
}

class HazardDetector {
    SyncStageAccessIndex usage_index_;

  public:
    HazardResult Detect(const ResourceAccessRangeMap::const_iterator &pos, SyncBarrierStack *barrier_stack) const {
        return pos->second.DetectHazard(usage_index_, barrier_stack);
    }
    bool ShortCircuit(const ResourceAccessRangeMap::const_iterator &pos, SyncBarrierStack *barrier_stack) const {
        // Check for short circuiting recursion over pos->first
        // If we've just checked against a write we don't have to looke any further...
        // however even if the current usage is write and there is not hazard with a stored read, we still have to
        // recur to ensure it's safe against *all* prior/parallel accesses.
        return pos->second.HasWriteOp();
    }
    HazardResult DetectAsync(const ResourceAccessRangeMap::const_iterator &pos) const {
        return pos->second.DetectAsyncHazard(usage_index_);
    }
    HazardDetector(SyncStageAccessIndex usage) : usage_index_(usage) {}
};

HazardResult AccessTrackerContext::DetectHazard(const VulkanTypedHandle &handle, SyncStageAccessIndex usage_index,
                                                const ResourceAccessRange &range) const {
    HazardDetector detector(usage_index);
    return DetectHazard(handle, detector, range);
}

void CommandBufferAccessContext::BeginRenderPass(const RENDER_PASS_STATE &rp_state) {
    // Create an access context for the first subpass and add it to the command buffers collection
    render_pass_contexts_.emplace_back(queue_flags_, &rp_state.subpass_dependencies, &cb_tracker_context_);
    current_renderpass_context_ = &render_pass_contexts_.back();
    current_context_ = &current_renderpass_context_->CurrentContext();
}

void CommandBufferAccessContext::NextRenderPass(const RENDER_PASS_STATE &rp_state) {
    assert(current_renderpass_context_);
    current_renderpass_context_->NextSubpass(queue_flags_, &cb_tracker_context_);
    current_context_ = &current_renderpass_context_->CurrentContext();
}

HazardResult AccessTrackerContext::DetectHazard(const IMAGE_STATE &image, SyncStageAccessIndex current_usage,
                                                const VkImageSubresourceLayers &subresource, const VkOffset3D &offset,
                                                const VkExtent3D &extent) const {
    // TODO: replace the encoder/generator with offset3D/extent3D aware versions
    VkImageSubresourceRange subresource_range = {subresource.aspectMask, subresource.mipLevel, 1, subresource.baseArrayLayer,
                                                 subresource.layerCount};
    subresource_adapter::RangeGenerator range_gen(image.range_encoder, subresource_range);
    VulkanTypedHandle image_handle(image.image, kVulkanObjectTypeImage);
    for (; range_gen->non_empty(); ++range_gen) {
        HazardResult hazard = DetectHazard(image_handle, current_usage, *range_gen);
        if (hazard.hazard) return hazard;
    }
    return HazardResult();
}

class BarrierHazardDetector {
  public:
    BarrierHazardDetector(SyncStageAccessIndex usage_index, VkPipelineStageFlags src_exec_scope,
                          SyncStageAccessFlags src_access_scope)
        : usage_index_(usage_index), src_exec_scope_(src_exec_scope), src_access_scope_(src_access_scope) {}

    HazardResult Detect(const ResourceAccessRangeMap::const_iterator &pos, SyncBarrierStack *barrier_stack) const {
        return pos->second.DetectBarrierHazard(usage_index_, src_exec_scope_, src_access_scope_, barrier_stack);
    }
    bool ShortCircuit(const ResourceAccessRangeMap::const_iterator &pos, SyncBarrierStack *barrier_stack) const { return false; }
    HazardResult DetectAsync(const ResourceAccessRangeMap::const_iterator &pos) const {
        // Async barrier hazard detection can use the same path as the usage index is not IsRead, but is IsWrite
        return pos->second.DetectAsyncHazard(usage_index_);
    }

  private:
    SyncStageAccessIndex usage_index_;
    VkPipelineStageFlags src_exec_scope_;
    SyncStageAccessFlags src_access_scope_;
};

HazardResult AccessTrackerContext::DetectBarrierHazard(const VulkanTypedHandle &handle, SyncStageAccessIndex current_usage,
                                                       VkPipelineStageFlags src_exec_scope, SyncStageAccessFlags src_access_scope,
                                                       const ResourceAccessRange &range) const {
    BarrierHazardDetector detector(current_usage, src_exec_scope, src_access_scope);
    return DetectHazard(handle, detector, range);
}

HazardResult DetectImageBarrierHazard(const AccessTrackerContext &context, const IMAGE_STATE &image,
                                      VkPipelineStageFlags src_exec_scope, SyncStageAccessFlags src_stage_accesses,
                                      const VkImageMemoryBarrier &barrier) {
    auto subresource_range = NormalizeSubresourceRange(image.createInfo, barrier.subresourceRange);
    subresource_adapter::RangeGenerator range_gen(image.range_encoder, subresource_range);
    const VulkanTypedHandle image_handle(image.image, kVulkanObjectTypeImage);
    const auto src_access_scope = SyncStageAccess::AccessScope(src_stage_accesses, barrier.srcAccessMask);
    for (; range_gen->non_empty(); ++range_gen) {
        HazardResult hazard = context.DetectBarrierHazard(image_handle, SyncStageAccessIndex::SYNC_IMAGE_LAYOUT_TRANSITION,
                                                          src_exec_scope, src_access_scope, *range_gen);
        if (hazard.hazard) return hazard;
    }
    return HazardResult();
}

template <typename Flags, typename Map>
SyncStageAccessFlags AccessScopeImpl(Flags flag_mask, const Map &map) {
    SyncStageAccessFlags scope = 0;
    for (const auto &bit_scope : map) {
        if (flag_mask < bit_scope.first) break;

        if (flag_mask & bit_scope.first) {
            scope |= bit_scope.second;
        }
    }
    return scope;
}

SyncStageAccessFlags SyncStageAccess::AccessScopeByStage(VkPipelineStageFlags stages) {
    return AccessScopeImpl(stages, syncStageAccessMaskByStageBit);
}

SyncStageAccessFlags SyncStageAccess::AccessScopeByAccess(VkAccessFlags accesses) {
    return AccessScopeImpl(accesses, syncStageAccessMaskByAccessBit);
}

// Getting from stage mask and access mask to stage/acess masks is something we need to be good at...
SyncStageAccessFlags SyncStageAccess::AccessScope(VkPipelineStageFlags stages, VkAccessFlags accesses) {
    // The access scope is the intersection of all stage/access types possible for the enabled stages and the enables accesses
    // (after doing a couple factoring of common terms the union of stage/access intersections is the intersections of the
    // union of all stage/access types for all the stages and the same unions for the access mask...
    return AccessScopeByStage(stages) & AccessScopeByAccess(accesses);
}

template <typename Action>
void UpdateMemoryAccessState(ResourceAccessRangeMap *accesses, const ResourceAccessRange &range, const Action &action) {
    // TODO -- region/mem-range accuracte update
    auto pos = accesses->lower_bound(range);
    if (pos == accesses->end() || !pos->first.intersects(range)) {
        // The range is empty, fill it with a default value.
        pos = action.Infill(accesses, pos, range);
    } else if (range.begin < pos->first.begin) {
        // Leading empty space, infill
        pos = action.Infill(accesses, pos, ResourceAccessRange(range.begin, pos->first.begin));
    } else if (pos->first.begin < range.begin) {
        // Trim the beginning if needed
        pos = accesses->split(pos, range.begin, sparse_container::split_op_keep_both());
        ++pos;
    }

    const auto the_end = accesses->end();
    while ((pos != the_end) && pos->first.intersects(range)) {
        if (pos->first.end > range.end) {
            pos = accesses->split(pos, range.end, sparse_container::split_op_keep_both());
        }

        pos = action(accesses, pos);
        if (pos == the_end) break;

        auto next = pos;
        ++next;
        if ((pos->first.end < range.end) && (next != the_end) && !next->first.is_subsequent_to(pos->first)) {
            // Need to infill if next is disjoint
            VkDeviceSize limit = (next == the_end) ? range.end : std::min(range.end, next->first.begin);
            ResourceAccessRange new_range(pos->first.end, limit);
            next = action.Infill(accesses, next, new_range);
        }
        pos = next;
    }
}

struct UpdateMemoryAccessStateFunctor {
    using Iterator = ResourceAccessRangeMap::iterator;
    Iterator Infill(ResourceAccessRangeMap *accesses, Iterator pos, ResourceAccessRange range) const {
        return accesses->insert(pos, std::make_pair(range, ResourceAccessState()));
    }
    Iterator operator()(ResourceAccessRangeMap *accesses, Iterator pos) const {
        auto &access_state = pos->second;
        access_state.Update(usage, tag);
        return pos;
    }

    UpdateMemoryAccessStateFunctor(SyncStageAccessIndex usage_, const ResourceUsageTag &tag_) : usage(usage_), tag(tag_) {}
    SyncStageAccessIndex usage;
    const ResourceUsageTag &tag;
};

struct ApplyMemoryAccessBarrierFunctor {
    using Iterator = ResourceAccessRangeMap::iterator;
    inline Iterator Infill(ResourceAccessRangeMap *accesses, Iterator pos, ResourceAccessRange range) const { return pos; }

    Iterator operator()(ResourceAccessRangeMap *accesses, Iterator pos) const {
        auto &access_state = pos->second;
        access_state.ApplyMemoryAccessBarrier(src_exec_scope, src_access_scope, dst_exec_scope, dst_access_scope);
        return pos;
    }

    ApplyMemoryAccessBarrierFunctor(VkPipelineStageFlags src_exec_scope_, SyncStageAccessFlags src_access_scope_,
                                    VkPipelineStageFlags dst_exec_scope_, SyncStageAccessFlags dst_access_scope_)
        : src_exec_scope(src_exec_scope_),
          src_access_scope(src_access_scope_),
          dst_exec_scope(dst_exec_scope_),
          dst_access_scope(dst_access_scope_) {}

    VkPipelineStageFlags src_exec_scope;
    SyncStageAccessFlags src_access_scope;
    VkPipelineStageFlags dst_exec_scope;
    SyncStageAccessFlags dst_access_scope;
};

struct ApplyGlobalBarrierFunctor {
    using Iterator = ResourceAccessRangeMap::iterator;
    inline Iterator Infill(ResourceAccessRangeMap *accesses, Iterator pos, ResourceAccessRange range) const { return pos; }

    Iterator operator()(ResourceAccessRangeMap *accesses, Iterator pos) const {
        auto &access_state = pos->second;
        access_state.ApplyExecutionBarrier(src_exec_scope, dst_exec_scope);

        for (const auto &functor : barrier_functor) {
            functor(accesses, pos);
        }
        return pos;
    }

    ApplyGlobalBarrierFunctor(VkPipelineStageFlags src_exec_scope, VkPipelineStageFlags dst_exec_scope,
                              SyncStageAccessFlags src_stage_accesses, SyncStageAccessFlags dst_stage_accesses,
                              uint32_t memoryBarrierCount, const VkMemoryBarrier *pMemoryBarriers)
        : src_exec_scope(src_exec_scope), dst_exec_scope(dst_exec_scope) {
        // Don't want to create this per tracked item, but don't want to loop through all tracked items per barrier...
        barrier_functor.reserve(memoryBarrierCount);
        for (uint32_t barrier_index = 0; barrier_index < memoryBarrierCount; barrier_index++) {
            const auto &barrier = pMemoryBarriers[barrier_index];
            barrier_functor.emplace_back(src_exec_scope, SyncStageAccess::AccessScope(src_stage_accesses, barrier.srcAccessMask),
                                         dst_exec_scope, SyncStageAccess::AccessScope(dst_stage_accesses, barrier.dstAccessMask));
        }
    }

    const VkPipelineStageFlags src_exec_scope;
    const VkPipelineStageFlags dst_exec_scope;
    std::vector<ApplyMemoryAccessBarrierFunctor> barrier_functor;
};

void AccessTracker::UpdateAccessState(SyncStageAccessIndex current_usage, const ResourceAccessRange &range,
                                      const ResourceUsageTag &tag) {
    UpdateMemoryAccessStateFunctor action(current_usage, tag);
    UpdateMemoryAccessState(&accesses_, range, action);
}

void AccessTrackerContext::UpdateAccessState(const VulkanTypedHandle &handle, SyncStageAccessIndex current_usage,
                                             const ResourceAccessRange &range, const ResourceUsageTag &tag) {
    auto *tracker = GetAccessTracker(handle);
    assert(tracker);
    tracker->UpdateAccessState(current_usage, range, tag);
}

void AccessTracker::UpdateAccessState(const IMAGE_STATE &image, SyncStageAccessIndex current_usage,
                                      const VkImageSubresourceLayers &subresource, const VkOffset3D &offset,
                                      const VkExtent3D &extent, const ResourceUsageTag &tag) {
    // TODO: replace the encoder/generator with offset3D aware versions
    VkImageSubresourceRange subresource_range = {subresource.aspectMask, subresource.mipLevel, 1, subresource.baseArrayLayer,
                                                 subresource.layerCount};
    subresource_adapter::RangeGenerator range_gen(image.range_encoder, subresource_range);
    for (; range_gen->non_empty(); ++range_gen) {
        UpdateAccessState(current_usage, *range_gen, tag);
    }
}

void AccessTrackerContext::UpdateAccessState(const IMAGE_STATE &image, SyncStageAccessIndex current_usage,
                                             const VkImageSubresourceLayers &subresource, const VkOffset3D &offset,
                                             const VkExtent3D &extent, const ResourceUsageTag &tag) {
    const VulkanTypedHandle handle(image.image, kVulkanObjectTypeImage);
    auto *tracker = GetAccessTracker(handle);
    assert(tracker);
    tracker->UpdateAccessState(image, current_usage, subresource, offset, extent, tag);
}

SyncBarrier::SyncBarrier(VkQueueFlags queue_flags, const VkSubpassDependency2 &barrier) {
    const auto src_stage_mask = ExpandPipelineStages(queue_flags, barrier.srcStageMask);
    src_exec_scope = WithEarlierPipelineStages(src_stage_mask);
    src_access_scope = SyncStageAccess::AccessScope(src_stage_mask, barrier.srcAccessMask);
    const auto dst_stage_mask = ExpandPipelineStages(queue_flags, barrier.dstStageMask);
    dst_exec_scope = WithLaterPipelineStages(dst_stage_mask);
    dst_access_scope = SyncStageAccess::AccessScope(dst_stage_mask, barrier.dstAccessMask);
}

void ResourceAccessState::ApplyBarrier(const SyncBarrier &barrier) {
    ApplyExecutionBarrier(barrier.src_exec_scope, barrier.dst_exec_scope);
    ApplyMemoryAccessBarrier(barrier.src_exec_scope, barrier.src_access_scope, barrier.dst_exec_scope, barrier.dst_access_scope);
}

ResourceAccessState ResourceAccessState::ApplyBarrierStack(const ResourceAccessState &that, const SyncBarrierStack &barrier_stack) {
    ResourceAccessState copy = that;
    for (auto barrier = barrier_stack.begin(); barrier != barrier_stack.end(); ++barrier) {
        assert(*barrier);
        copy.ApplyBarrier(*(*barrier));
    }
    return copy;
}

HazardResult ResourceAccessState::DetectHazard(SyncStageAccessIndex usage_index, SyncBarrierStack *barrier_stack) const {
    if (barrier_stack) {
        return ApplyBarrierStack(*this, *barrier_stack).DetectHazard(usage_index);
    }
    return DetectHazard(usage_index);
}

HazardResult ResourceAccessState::DetectHazard(SyncStageAccessIndex usage_index) const {
    HazardResult hazard;
    auto usage = FlagBit(usage_index);
    if (IsRead(usage)) {
        if (IsWriteHazard(usage)) {
            hazard.Set(READ_AFTER_WRITE, write_tag);
        }
    } else {
        // Assume write
        // TODO determine what to do with READ-WRITE usage states if any
        // Write-After-Write check -- if we have a previous write to test against
        if (last_write && IsWriteHazard(usage)) {
            hazard.Set(WRITE_AFTER_WRITE, write_tag);
        } else {
            // Only look for casus belli for WAR
            const auto usage_stage = PipelineStageBit(usage_index);
            for (uint32_t read_index = 0; read_index < last_read_count; read_index++) {
                if (IsReadHazard(usage_stage, last_reads[read_index])) {
                    hazard.Set(WRITE_AFTER_READ, last_reads[read_index].tag);
                    break;
                }
            }
        }
    }
    return hazard;
}

// Asynchronous Hazards occur between subpasses with no connection through the DAG
HazardResult ResourceAccessState::DetectAsyncHazard(SyncStageAccessIndex usage_index) const {
    HazardResult hazard;
    auto usage = FlagBit(usage_index);
    if (IsRead(usage)) {
        if (last_write != 0) {
            hazard.Set(READ_RACING_WRITE, write_tag);
        }
    } else {
        if (last_write != 0) {
            hazard.Set(WRITE_RACING_WRITE, write_tag);
        } else if (last_read_count > 0) {
            hazard.Set(WRITE_RACING_READ, last_reads[0].tag);
        }
    }
    return hazard;
}

HazardResult ResourceAccessState::DetectBarrierHazard(SyncStageAccessIndex usage_index, VkPipelineStageFlags src_exec_scope,
                                                      SyncStageAccessFlags src_access_scope,
                                                      SyncBarrierStack *barrier_stack) const {
    if (barrier_stack) {
        return ApplyBarrierStack(*this, *barrier_stack).DetectBarrierHazard(usage_index, src_exec_scope, src_access_scope);
    }
    return DetectBarrierHazard(usage_index, src_exec_scope, src_access_scope);
}

HazardResult ResourceAccessState::DetectBarrierHazard(SyncStageAccessIndex usage_index, VkPipelineStageFlags src_exec_scope,
                                                      SyncStageAccessFlags src_access_scope) const {
    // Only supporting image layout transitions for now
    assert(usage_index == SyncStageAccessIndex::SYNC_IMAGE_LAYOUT_TRANSITION);
    HazardResult hazard;
    if (last_write) {
        // If the previous write is *not* in the 1st access scope
        // *AND* the current barrier is not in the dependency chain
        // *AND* the there is no prior memory barrier for the previous write in the dependency chain
        // then the barrier access is unsafe (R/W after W)
        if (((last_write & src_access_scope) == 0) && ((src_exec_scope & write_dependency_chain) == 0) && (write_barriers == 0)) {
            // TODO: Do we need a difference hazard name for this?
            hazard.Set(WRITE_AFTER_WRITE, write_tag);
        }
    } else {
        // Look at the reads
        for (uint32_t read_index = 0; read_index < last_read_count; read_index++) {
            const auto &read_access = last_reads[read_index];
            // If the read stage is not in the src sync sync
            // *AND* not execution chained with an existing sync barrier (that's the or)
            // then the barrier access is unsafe (R/W after R)
            if ((src_exec_scope & (read_access.stage | read_access.barriers)) == 0) {
                hazard.Set(WRITE_AFTER_READ, read_access.tag);
                break;
            }
        }
    }
    return hazard;
}

void ResourceAccessState::Update(SyncStageAccessIndex usage_index, const ResourceUsageTag &tag) {
    // Move this logic in the ResourceStateTracker as methods, thereof (or we'll repeat it for every flavor of resource...
    const auto usage_bit = FlagBit(usage_index);
    if (IsRead(usage_index)) {
        // Mulitple outstanding reads may be of interest and do dependency chains independently
        // However, for purposes of barrier tracking, only one read per pipeline stage matters
        const auto usage_stage = PipelineStageBit(usage_index);
        if (usage_stage & last_read_stages) {
            for (uint32_t read_index = 0; read_index < last_read_count; read_index++) {
                ReadState &access = last_reads[read_index];
                if (access.stage == usage_stage) {
                    access.barriers = 0;
                    access.tag = tag;
                    break;
                }
            }
        } else {
            // We don't have this stage in the list yet...
            assert(last_read_count < last_reads.size());
            ReadState &access = last_reads[last_read_count++];
            access.stage = usage_stage;
            access.barriers = 0;
            access.tag = tag;
            last_read_stages |= usage_stage;
        }
    } else {
        // Assume write
        // TODO determine what to do with READ-WRITE operations if any
        // Clobber last read and both sets of barriers... because all we have is DANGER, DANGER, WILL ROBINSON!!!
        // if the last_reads/last_write were unsafe, we've reported them,
        // in either case the prior access is irrelevant, we can overwrite them as *this* write is now after them
        last_read_count = 0;
        last_read_stages = 0;

        write_barriers = 0;
        write_dependency_chain = 0;
        write_tag = tag;
        last_write = usage_bit;
    }
}
void ResourceAccessState::ApplyExecutionBarrier(VkPipelineStageFlags srcStageMask, VkPipelineStageFlags dstStageMask) {
    // Execution Barriers only protect read operations
    for (uint32_t read_index = 0; read_index < last_read_count; read_index++) {
        ReadState &access = last_reads[read_index];
        // The | implements the "dependency chain" logic for this access, as the barriers field stores the second sync scope
        if (srcStageMask & (access.stage | access.barriers)) {
            access.barriers |= dstStageMask;
        }
    }
    if (write_dependency_chain & srcStageMask) write_dependency_chain |= dstStageMask;
}

void ResourceAccessState::ApplyMemoryAccessBarrier(VkPipelineStageFlags src_exec_scope, SyncStageAccessFlags src_access_scope,
                                                   VkPipelineStageFlags dst_exec_scope, SyncStageAccessFlags dst_access_scope) {
    // Assuming we've applied the execution side of this barrier, we update just the write
    // The || implements the "dependency chain" logic for this barrier
    if ((src_access_scope & last_write) || (write_dependency_chain & src_exec_scope)) {
        write_barriers |= dst_access_scope;
        write_dependency_chain |= dst_exec_scope;
    }
}

void SyncValidator::ResetCommandBuffer(VkCommandBuffer command_buffer) {
    auto *access_context = GetAccessContextNoInsert(command_buffer);
    if (access_context) {
        access_context->Reset();
    }
}

void SyncValidator::ApplyGlobalBarriers(AccessTrackerContext *context, VkPipelineStageFlags srcStageMask,
                                        VkPipelineStageFlags dstStageMask, SyncStageAccessFlags src_access_scope,
                                        SyncStageAccessFlags dst_access_scope, uint32_t memoryBarrierCount,
                                        const VkMemoryBarrier *pMemoryBarriers) {
    // TODO: Implement this better (maybe some delayed/on-demand integration).
    ApplyGlobalBarrierFunctor barriers_functor(srcStageMask, dstStageMask, src_access_scope, dst_access_scope, memoryBarrierCount,
                                               pMemoryBarriers);
    for (auto &handle_tracker_pair : context->GetAccessTrackerMap()) {
        UpdateMemoryAccessState(&handle_tracker_pair.second.GetCurrentAccessMap(), full_range, barriers_functor);
    }
}

void SyncValidator::ApplyBufferBarriers(AccessTrackerContext *context, VkPipelineStageFlags src_exec_scope,
                                        SyncStageAccessFlags src_stage_accesses, VkPipelineStageFlags dst_exec_scope,
                                        SyncStageAccessFlags dst_stage_accesses, uint32_t barrier_count,
                                        const VkBufferMemoryBarrier *barriers) {
    // TODO Implement this at subresource/memory_range accuracy
    for (uint32_t index = 0; index < barrier_count; index++) {
        const auto &barrier = barriers[index];
        const auto *buffer = Get<BUFFER_STATE>(barrier.buffer);
        if (!buffer) continue;
        auto *tracker = context->GetAccessTracker(VulkanTypedHandle(buffer->binding.mem_state->mem, kVulkanObjectTypeDeviceMemory));
        if (!tracker) continue;
        ResourceAccessRange range = MakeMemoryAccessRange(*buffer, barrier.offset, barrier.size);
        UpdateMemoryAccessState(
            &tracker->GetCurrentAccessMap(), range,
            ApplyMemoryAccessBarrierFunctor(src_exec_scope, AccessScope(src_stage_accesses, barrier.srcAccessMask), dst_exec_scope,
                                            AccessScope(dst_stage_accesses, barrier.dstAccessMask)));
    }
}

void SyncValidator::ApplyImageBarriers(AccessTrackerContext *context, VkPipelineStageFlags src_exec_scope,
                                       SyncStageAccessFlags src_stage_accesses, VkPipelineStageFlags dst_exec_scope,
                                       SyncStageAccessFlags dst_stage_accesses, uint32_t barrier_count,
                                       const VkImageMemoryBarrier *barriers) {
    for (uint32_t index = 0; index < barrier_count; index++) {
        const auto &barrier = barriers[index];
        const auto *image = Get<IMAGE_STATE>(barrier.image);
        if (!image) continue;
        auto tracker = context->GetAccessTrackerNoInsert(VulkanTypedHandle(barrier.image, kVulkanObjectTypeImage));
        if (!tracker) continue;
        auto *accesses = &tracker->GetCurrentAccessMap();

        auto subresource_range = NormalizeSubresourceRange(image->createInfo, barrier.subresourceRange);
        subresource_adapter::RangeGenerator range_gen(image->range_encoder, subresource_range);
        const ApplyMemoryAccessBarrierFunctor barrier_action(src_exec_scope, AccessScope(src_stage_accesses, barrier.srcAccessMask),
                                                             dst_exec_scope,
                                                             AccessScope(dst_stage_accesses, barrier.dstAccessMask));
        for (; range_gen->non_empty(); ++range_gen) {
            UpdateMemoryAccessState(accesses, *range_gen, barrier_action);
        }
    }
}

bool SyncValidator::PreCallValidateCmdCopyBuffer(VkCommandBuffer commandBuffer, VkBuffer srcBuffer, VkBuffer dstBuffer,
                                                 uint32_t regionCount, const VkBufferCopy *pRegions) const {
    bool skip = false;
    const auto *cb_context = GetAccessContext(commandBuffer);
    assert(cb_context);
    if (!cb_context) return skip;
    const auto *context = cb_context->GetCurrentAccessContext();

    // If we have no previous accesses, we have no hazards
    // TODO: make this sub-resource capable
    // TODO: make this general, and stuff it into templates/utility functions
    const auto *src_buffer = Get<BUFFER_STATE>(srcBuffer);
    const auto src_mem = (src_buffer && !src_buffer->sparse) ? src_buffer->binding.mem_state->mem : VK_NULL_HANDLE;
    const auto *dst_buffer = Get<BUFFER_STATE>(dstBuffer);
    const auto dst_mem = (dst_buffer && !dst_buffer->sparse) ? dst_buffer->binding.mem_state->mem : VK_NULL_HANDLE;

    for (uint32_t region = 0; region < regionCount; region++) {
        const auto &copy_region = pRegions[region];
        if (src_mem != VK_NULL_HANDLE) {
            ResourceAccessRange src_range = MakeMemoryAccessRange(*src_buffer, copy_region.srcOffset, copy_region.size);
            auto hazard = context->DetectHazard(VulkanTypedHandle(src_mem, kVulkanObjectTypeDeviceMemory),
                                                SYNC_TRANSFER_TRANSFER_READ, src_range);
            if (hazard.hazard) {
                // TODO -- add tag information to log msg when useful.
                skip |= LogError(srcBuffer, string_SyncHazardVUID(hazard.hazard), "Hazard %s for srcBuffer %s, region %" PRIu32,
                                 string_SyncHazard(hazard.hazard), report_data->FormatHandle(srcBuffer).c_str(), region);
            }
        }
        if ((dst_mem != VK_NULL_HANDLE) && !skip) {
            ResourceAccessRange dst_range = MakeMemoryAccessRange(*dst_buffer, copy_region.dstOffset, copy_region.size);
            auto hazard = context->DetectHazard(VulkanTypedHandle(dst_mem, kVulkanObjectTypeDeviceMemory),
                                                SYNC_TRANSFER_TRANSFER_WRITE, dst_range);
            if (hazard.hazard) {
                skip |= LogError(dstBuffer, string_SyncHazardVUID(hazard.hazard), "Hazard %s for dstBuffer %s, region %" PRIu32,
                                 string_SyncHazard(hazard.hazard), report_data->FormatHandle(dstBuffer).c_str(), region);
            }
        }
        if (skip) break;
    }
    return skip;
}

void SyncValidator::PreCallRecordCmdCopyBuffer(VkCommandBuffer commandBuffer, VkBuffer srcBuffer, VkBuffer dstBuffer,
                                               uint32_t regionCount, const VkBufferCopy *pRegions) {
    auto *cb_context = GetAccessContext(commandBuffer);
    assert(cb_context);
    auto *context = cb_context->GetCurrentAccessContext();

    const auto *src_buffer = Get<BUFFER_STATE>(srcBuffer);
    const auto src_mem = (src_buffer && !src_buffer->sparse) ? src_buffer->binding.mem_state->mem : VK_NULL_HANDLE;
    const VulkanTypedHandle src_handle(src_mem, kVulkanObjectTypeDeviceMemory);
    AccessTracker *src_tracker = src_mem ? context->GetAccessTracker(src_handle) : nullptr;

    const auto *dst_buffer = Get<BUFFER_STATE>(dstBuffer);
    const auto dst_mem = (dst_buffer && !dst_buffer->sparse) ? dst_buffer->binding.mem_state->mem : VK_NULL_HANDLE;
    const VulkanTypedHandle dst_handle(dst_mem, kVulkanObjectTypeDeviceMemory);
    AccessTracker *dst_tracker = dst_mem ? context->GetAccessTracker(dst_handle) : nullptr;

    for (uint32_t region = 0; region < regionCount; region++) {
        const auto &copy_region = pRegions[region];
        if (src_mem) {
            ResourceAccessRange src_range = MakeMemoryAccessRange(*src_buffer, copy_region.srcOffset, copy_region.size);
            src_tracker->UpdateAccessState(SYNC_TRANSFER_TRANSFER_READ, src_range, tag);
        }
        if (dst_mem) {
            ResourceAccessRange dst_range = MakeMemoryAccessRange(*dst_buffer, copy_region.dstOffset, copy_region.size);
            dst_tracker->UpdateAccessState(SYNC_TRANSFER_TRANSFER_WRITE, dst_range, tag);
        }
    }
}

bool SyncValidator::PreCallValidateCmdCopyImage(VkCommandBuffer commandBuffer, VkImage srcImage, VkImageLayout srcImageLayout,
                                                VkImage dstImage, VkImageLayout dstImageLayout, uint32_t regionCount,
                                                const VkImageCopy *pRegions) const {
    bool skip = false;
    const auto *cb_access_context = GetAccessContext(commandBuffer);
    assert(cb_access_context);
    if (!cb_access_context) return skip;

    const auto *context = cb_access_context->GetCurrentAccessContext();
    assert(context);
    if (!context) return skip;

    const auto *src_image = Get<IMAGE_STATE>(srcImage);
    const auto *dst_image = Get<IMAGE_STATE>(dstImage);

    for (uint32_t region = 0; region < regionCount; region++) {
        const auto &copy_region = pRegions[region];
        if (src_image) {
            auto hazard = context->DetectHazard(*src_image, SYNC_TRANSFER_TRANSFER_READ, copy_region.srcSubresource,
                                                copy_region.srcOffset, copy_region.extent);
            if (hazard.hazard) {
                skip |= LogError(srcImage, string_SyncHazardVUID(hazard.hazard), "Hazard %s for srcImage %s, region %" PRIu32,
                                 string_SyncHazard(hazard.hazard), report_data->FormatHandle(srcImage).c_str(), region);
            }
        }

        if (dst_image) {
            auto hazard = context->DetectHazard(*dst_image, SYNC_TRANSFER_TRANSFER_WRITE, copy_region.dstSubresource,
                                                copy_region.dstOffset, copy_region.extent);
            if (hazard.hazard) {
                skip |= LogError(dstImage, string_SyncHazardVUID(hazard.hazard), "Hazard %s for dstImage %s, region %" PRIu32,
                                 string_SyncHazard(hazard.hazard), report_data->FormatHandle(dstImage).c_str(), region);
            }
        }
    }

    return skip;
}

void SyncValidator::PreCallRecordCmdCopyImage(VkCommandBuffer commandBuffer, VkImage srcImage, VkImageLayout srcImageLayout,
                                              VkImage dstImage, VkImageLayout dstImageLayout, uint32_t regionCount,
                                              const VkImageCopy *pRegions) {
    auto *cb_access_context = GetAccessContext(commandBuffer);
    assert(cb_access_context);
    auto *context = cb_access_context->GetCurrentAccessContext();
    assert(context);

    auto *src_image = Get<IMAGE_STATE>(srcImage);
    auto *src_tracker = context->GetAccessTracker(VulkanTypedHandle(srcImage, kVulkanObjectTypeImage));
    auto *dst_image = Get<IMAGE_STATE>(dstImage);
    auto *dst_tracker = context->GetAccessTracker(VulkanTypedHandle(dstImage, kVulkanObjectTypeImage));

    for (uint32_t region = 0; region < regionCount; region++) {
        const auto &copy_region = pRegions[region];
        if (src_image) {
            src_tracker->UpdateAccessState(*src_image, SYNC_TRANSFER_TRANSFER_READ, copy_region.srcSubresource,
                                           copy_region.srcOffset, copy_region.extent, tag);
        }
        if (dst_image) {
            dst_tracker->UpdateAccessState(*dst_image, SYNC_TRANSFER_TRANSFER_WRITE, copy_region.dstSubresource,
                                           copy_region.dstOffset, copy_region.extent, tag);
        }
    }
}

bool SyncValidator::PreCallValidateCmdPipelineBarrier(VkCommandBuffer commandBuffer, VkPipelineStageFlags srcStageMask,
                                                      VkPipelineStageFlags dstStageMask, VkDependencyFlags dependencyFlags,
                                                      uint32_t memoryBarrierCount, const VkMemoryBarrier *pMemoryBarriers,
                                                      uint32_t bufferMemoryBarrierCount,
                                                      const VkBufferMemoryBarrier *pBufferMemoryBarriers,
                                                      uint32_t imageMemoryBarrierCount,
                                                      const VkImageMemoryBarrier *pImageMemoryBarriers) const {
    bool skip = false;
    const auto *cb_access_context = GetAccessContext(commandBuffer);
    assert(cb_access_context);
    if (!cb_access_context) return skip;

    const auto *context = cb_access_context->GetCurrentAccessContext();
    assert(context);
    if (!context) return skip;

    const auto src_stage_mask = ExpandPipelineStages(cb_access_context->GetQueueFlags(), srcStageMask);
    const auto src_exec_scope = WithEarlierPipelineStages(src_stage_mask);
    auto src_stage_accesses = AccessScopeByStage(src_stage_mask);
    // Validate Image Layout transitions
    for (uint32_t index = 0; index < imageMemoryBarrierCount; index++) {
        const auto &barrier = pImageMemoryBarriers[index];
        if (barrier.newLayout == barrier.oldLayout) continue;  // Only interested in layout transitions at this point.
        const auto *image_state = Get<IMAGE_STATE>(barrier.image);
        if (!image_state) continue;
        const auto hazard = DetectImageBarrierHazard(*context, *image_state, src_exec_scope, src_stage_accesses, barrier);
        if (hazard.hazard) {
            // TODO -- add tag information to log msg when useful.
            skip |= LogError(barrier.image, string_SyncHazardVUID(hazard.hazard), "Hazard %s for image barrier %" PRIu32 " %s",
                             string_SyncHazard(hazard.hazard), index, report_data->FormatHandle(barrier.image).c_str());
        }
    }

    return skip;
}

void SyncValidator::PreCallRecordCmdPipelineBarrier(VkCommandBuffer commandBuffer, VkPipelineStageFlags srcStageMask,
                                                    VkPipelineStageFlags dstStageMask, VkDependencyFlags dependencyFlags,
                                                    uint32_t memoryBarrierCount, const VkMemoryBarrier *pMemoryBarriers,
                                                    uint32_t bufferMemoryBarrierCount,
                                                    const VkBufferMemoryBarrier *pBufferMemoryBarriers,
                                                    uint32_t imageMemoryBarrierCount,
                                                    const VkImageMemoryBarrier *pImageMemoryBarriers) {
    auto *cb_access_context = GetAccessContext(commandBuffer);
    assert(cb_access_context);
    if (!cb_access_context) return;
    auto access_context = cb_access_context->GetCurrentAccessContext();
    assert(access_context);
    if (!access_context) return;

    const auto src_stage_mask = ExpandPipelineStages(cb_access_context->GetQueueFlags(), srcStageMask);
    auto src_stage_accesses = AccessScopeByStage(src_stage_mask);
    const auto dst_stage_mask = ExpandPipelineStages(cb_access_context->GetQueueFlags(), dstStageMask);
    auto dst_stage_accesses = AccessScopeByStage(dst_stage_mask);
    const auto src_exec_scope = WithEarlierPipelineStages(src_stage_mask);
    const auto dst_exec_scope = WithLaterPipelineStages(dst_stage_mask);
    ApplyBufferBarriers(access_context, src_exec_scope, src_stage_accesses, dst_exec_scope, dst_stage_accesses,
                        bufferMemoryBarrierCount, pBufferMemoryBarriers);
    ApplyImageBarriers(access_context, src_exec_scope, src_stage_accesses, dst_exec_scope, dst_stage_accesses,
                       imageMemoryBarrierCount, pImageMemoryBarriers);

    // Apply these last in-case there operation is a superset of the other two and would clean them up...
    ApplyGlobalBarriers(access_context, src_exec_scope, dst_exec_scope, src_stage_accesses, dst_stage_accesses, memoryBarrierCount,
                        pMemoryBarriers);
}

void SyncValidator::PostCallRecordCreateDevice(VkPhysicalDevice gpu, const VkDeviceCreateInfo *pCreateInfo,
                                               const VkAllocationCallbacks *pAllocator, VkDevice *pDevice, VkResult result) {
    // The state tracker sets up the device state
    StateTracker::PostCallRecordCreateDevice(gpu, pCreateInfo, pAllocator, pDevice, result);

    // Add the callback hooks for the functions that are either broadly or deeply used and that the ValidationStateTracker refactor
    // would be messier without.
    // TODO: Find a good way to do this hooklessly.
    ValidationObject *device_object = GetLayerDataPtr(get_dispatch_key(*pDevice), layer_data_map);
    ValidationObject *validation_data = GetValidationObject(device_object->object_dispatch, LayerObjectTypeSyncValidation);
    SyncValidator *sync_device_state = static_cast<SyncValidator *>(validation_data);

    sync_device_state->SetCommandBufferResetCallback(
        [sync_device_state](VkCommandBuffer command_buffer) -> void { sync_device_state->ResetCommandBuffer(command_buffer); });
}

void SyncValidator::PostCallRecordBeginCommandBuffer(VkCommandBuffer commandBuffer, const VkCommandBufferBeginInfo *pBeginInfo,
                                                     VkResult result) {
    // The state tracker sets up the command buffer state
    StateTracker::PostCallRecordBeginCommandBuffer(commandBuffer, pBeginInfo, result);

    // Create/initialize the structure that trackers accesses at the command buffer scope.
    auto cb_access_context = GetAccessContext(commandBuffer);
    assert(cb_access_context);
    cb_access_context->Reset();
}

void SyncValidator::RecordCmdBeginRenderPass(VkCommandBuffer commandBuffer, const VkRenderPassBeginInfo *pRenderPassBegin,
                                             const VkSubpassBeginInfo *pSubpassBeginInfo) {
    const auto rp_state = Get<RENDER_PASS_STATE>(pRenderPassBegin->renderPass);
    auto cb_context = GetAccessContext(commandBuffer);
    if (rp_state && cb_context) {
        cb_context->BeginRenderPass(*rp_state);
    }
}

void SyncValidator::PostCallRecordCmdBeginRenderPass(VkCommandBuffer commandBuffer, const VkRenderPassBeginInfo *pRenderPassBegin,
                                                     VkSubpassContents contents) {
    StateTracker::PostCallRecordCmdBeginRenderPass(commandBuffer, pRenderPassBegin, contents);
    auto subpass_begin_info = lvl_init_struct<VkSubpassBeginInfo>();
    subpass_begin_info.contents = contents;
    RecordCmdBeginRenderPass(commandBuffer, pRenderPassBegin, &subpass_begin_info);
}

void SyncValidator::PostCallRecordCmdBeginRenderPass2(VkCommandBuffer commandBuffer, const VkRenderPassBeginInfo *pRenderPassBegin,
                                                      const VkSubpassBeginInfo *pSubpassBeginInfo) {
    StateTracker::PostCallRecordCmdBeginRenderPass2(commandBuffer, pRenderPassBegin, pSubpassBeginInfo);
    RecordCmdBeginRenderPass(commandBuffer, pRenderPassBegin, pSubpassBeginInfo);
}

void SyncValidator::PostCallRecordCmdBeginRenderPass2KHR(VkCommandBuffer commandBuffer,
                                                         const VkRenderPassBeginInfo *pRenderPassBegin,
                                                         const VkSubpassBeginInfo *pSubpassBeginInfo) {
    StateTracker::PostCallRecordCmdBeginRenderPass2KHR(commandBuffer, pRenderPassBegin, pSubpassBeginInfo);
    RecordCmdBeginRenderPass(commandBuffer, pRenderPassBegin, pSubpassBeginInfo);
}

void SyncValidator::RecordCmdNextSubpass(VkCommandBuffer commandBuffer, const VkSubpassBeginInfo *pSubpassBeginInfo,
                                         const VkSubpassEndInfo *pSubpassEndInfo) {
    auto cb_context = GetAccessContext(commandBuffer);
    assert(cb_context);
    auto cb_state = cb_context->GetCommandBufferState();
    if (!cb_state) return;

    auto rp_state = cb_state->activeRenderPass;
    if (!rp_state) return;

    cb_context->NextRenderPass(*rp_state);
}

void SyncValidator::PostCallRecordCmdNextSubpass(VkCommandBuffer commandBuffer, VkSubpassContents contents) {
    StateTracker::PostCallRecordCmdNextSubpass(commandBuffer, contents);
    auto subpass_begin_info = lvl_init_struct<VkSubpassBeginInfo>();
    subpass_begin_info.contents = contents;
    RecordCmdNextSubpass(commandBuffer, &subpass_begin_info, nullptr);
}

void SyncValidator::PostCallRecordCmdNextSubpass2(VkCommandBuffer commandBuffer, const VkSubpassBeginInfo *pSubpassBeginInfo,
                                                  const VkSubpassEndInfo *pSubpassEndInfo) {
    StateTracker::PostCallRecordCmdNextSubpass2(commandBuffer, pSubpassBeginInfo, pSubpassEndInfo);
    RecordCmdNextSubpass(commandBuffer, pSubpassBeginInfo, pSubpassEndInfo);
}

void SyncValidator::PostCallRecordCmdNextSubpass2KHR(VkCommandBuffer commandBuffer, const VkSubpassBeginInfo *pSubpassBeginInfo,
                                                     const VkSubpassEndInfo *pSubpassEndInfo) {
    StateTracker::PostCallRecordCmdNextSubpass2KHR(commandBuffer, pSubpassBeginInfo, pSubpassEndInfo);
    RecordCmdNextSubpass(commandBuffer, pSubpassBeginInfo, pSubpassEndInfo);
}

void SyncValidator::RecordCmdEndRenderPass(VkCommandBuffer commandBuffer, const VkSubpassEndInfo *pSubpassEndInfo) {}

void SyncValidator::PostCallRecordCmdEndRenderPass(VkCommandBuffer commandBuffer) {
    StateTracker::PostCallRecordCmdEndRenderPass(commandBuffer);
    RecordCmdEndRenderPass(commandBuffer, nullptr);
}

void SyncValidator::PostCallRecordCmdEndRenderPass2(VkCommandBuffer commandBuffer, const VkSubpassEndInfo *pSubpassEndInfo) {
    StateTracker::PostCallRecordCmdEndRenderPass2(commandBuffer, pSubpassEndInfo);
    RecordCmdEndRenderPass(commandBuffer, pSubpassEndInfo);
}

void SyncValidator::PostCallRecordCmdEndRenderPass2KHR(VkCommandBuffer commandBuffer, const VkSubpassEndInfo *pSubpassEndInfo) {
    StateTracker::PostCallRecordCmdEndRenderPass2KHR(commandBuffer, pSubpassEndInfo);
    RecordCmdEndRenderPass(commandBuffer, pSubpassEndInfo);
}
