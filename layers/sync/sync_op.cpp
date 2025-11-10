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

#include "sync/sync_op.h"
#include "sync/sync_renderpass.h"
#include "sync/sync_access_context.h"
#include "sync/sync_commandbuffer.h"
#include "sync/sync_image.h"
#include "sync/sync_validation.h"

#include "state_tracker/buffer_state.h"
#include "state_tracker/cmd_buffer_state.h"
#include "state_tracker/render_pass_state.h"

#include "utils/image_utils.h"
#include "utils/sync_utils.h"

namespace syncval {

// Range generators for to allow event scope filtration to be limited to the top of the resource access traversal pipeline
//
// Note: there is no "begin/end" or reset facility.  These are each written as "one time through" generators.
//
// Usage:
//  Constructor() -- initializes the generator to point to the begin of the space declared.
//  *  -- the current range of the generator empty signfies end
//  ++ -- advance to the next non-empty range (or end)

// Generate the ranges that are the intersection of range and the entries in the RangeMap
class MapRangesRangeGenerator {
  public:
    // Default constructed is safe to dereference for "empty" test, but for no other operation.
    MapRangesRangeGenerator() {
        // Default construction *must* be empty range
        assert(current_.empty());
    }
    MapRangesRangeGenerator(const AccessMap &filter, const AccessRange &range)
        : range_(range), map_(&filter), map_pos_(), current_() {
        SeekBegin();
    }
    MapRangesRangeGenerator(const MapRangesRangeGenerator &from) = default;

    const AccessRange &operator*() const { return current_; }
    const AccessRange *operator->() const { return &current_; }
    MapRangesRangeGenerator &operator++() {
        ++map_pos_;
        UpdateCurrent();
        return *this;
    }

    bool operator==(const MapRangesRangeGenerator &other) const { return current_ == other.current_; }

  protected:
    void UpdateCurrent() {
        if (map_pos_ != map_->end()) {
            current_ = range_ & map_pos_->first;
        } else {
            current_ = {};
        }
    }
    void SeekBegin() {
        map_pos_ = map_->LowerBound(range_);
        UpdateCurrent();
    }

    // Adding this functionality here, to avoid gratuitous Base:: qualifiers in the derived class
    // Note: Not exposed in this classes public interface to encourage using a consistent ++/empty generator semantic
    template <typename Pred>
    MapRangesRangeGenerator &PredicatedIncrement(Pred &pred) {
        do {
            ++map_pos_;
        } while (map_pos_ != map_->end() && map_pos_->first.intersects(range_) && !pred(map_pos_));
        UpdateCurrent();
        return *this;
    }

    const AccessRange range_;
    const AccessMap *map_ = nullptr;
    AccessMap::const_iterator map_pos_;
    AccessRange current_;
};
using EventSimpleRangeGenerator = MapRangesRangeGenerator;

// Generate the ranges that are the intersection of the RangeGen ranges and the entries in the FilterMap
template <typename RangeGen>
class FilteredGeneratorGenerator {
  public:
    // Default constructed is safe to dereference for "empty" test, but for no other operation.
    FilteredGeneratorGenerator() : filter_(nullptr), gen_(), filter_pos_(), current_() {
        // Default construction for KeyType *must* be empty range
        assert(current_.empty());
    }
    FilteredGeneratorGenerator(const AccessMap &filter, RangeGen &gen) : filter_(&filter), gen_(gen), filter_pos_(), current_() {
        SeekBegin();
    }
    FilteredGeneratorGenerator(const FilteredGeneratorGenerator &from) = default;
    const AccessRange &operator*() const { return current_; }
    const AccessRange *operator->() const { return &current_; }
    FilteredGeneratorGenerator &operator++() {
        AccessRange gen_range = GenRange();
        AccessRange filter_range = FilterRange();
        current_ = {};
        while (gen_range.non_empty() && filter_range.non_empty() && current_.empty()) {
            if (gen_range.end > filter_range.end) {
                // if the generated range is beyond the filter_range, advance the filter range
                filter_range = AdvanceFilter();
            } else {
                gen_range = AdvanceGen();
            }
            current_ = gen_range & filter_range;
        }
        return *this;
    }

    bool operator==(const FilteredGeneratorGenerator &other) const { return current_ == other.current_; }

  private:
    AccessRange AdvanceFilter() {
        ++filter_pos_;
        auto filter_range = FilterRange();
        assert(filter_range.valid());
        if (filter_range.valid()) {
            FastForwardGen(filter_range);
        }
        return filter_range;
    }
    AccessRange AdvanceGen() {
        ++gen_;
        auto gen_range = GenRange();
        if (gen_range.valid()) {
            FastForwardFilter(gen_range);
        }
        return gen_range;
    }

    AccessRange FilterRange() const { return (filter_pos_ != filter_->end()) ? filter_pos_->first : AccessRange{}; }
    AccessRange GenRange() const { return *gen_; }

    AccessRange FastForwardFilter(const AccessRange &range) {
        auto filter_range = FilterRange();
        int retry_count = 0;
        const static int kRetryLimit = 2;  // TODO -- determine whether this limit is optimal
        while (!filter_range.empty() && (filter_range.end <= range.begin)) {
            if (retry_count < kRetryLimit) {
                ++filter_pos_;
                filter_range = FilterRange();
                retry_count++;
            } else {
                // Okay we've tried walking, do a seek.
                filter_pos_ = filter_->LowerBound(range);
                break;
            }
        }
        return FilterRange();
    }

    // TODO: Consider adding "seek" (or an absolute bound "get" to range generators to make this walk
    // faster.
    AccessRange FastForwardGen(const AccessRange &range) {
        auto gen_range = GenRange();
        while (!gen_range.empty() && (gen_range.end <= range.begin)) {
            ++gen_;
            gen_range = GenRange();
        }
        return gen_range;
    }

    void SeekBegin() {
        auto gen_range = GenRange();
        if (gen_range.empty()) {
            current_ = {};
            filter_pos_ = filter_->end();
        } else {
            filter_pos_ = filter_->LowerBound(gen_range);
            current_ = gen_range & FilterRange();
        }
    }

    const AccessMap *filter_ = nullptr;
    RangeGen gen_;
    AccessMap::const_iterator filter_pos_;
    AccessRange current_;
};

using EventImageRangeGenerator = FilteredGeneratorGenerator<subresource_adapter::ImageRangeGenerator>;

void BarrierSet::MakeMemoryBarriers(const SyncExecScope &src, const SyncExecScope &dst, uint32_t memory_barrier_count,
                                    const VkMemoryBarrier *barriers) {
    memory_barriers.reserve(std::max<uint32_t>(1, memory_barrier_count));
    for (const VkMemoryBarrier &barrier : vvl::make_span(barriers, memory_barrier_count)) {
        SyncBarrier sync_barrier(src, barrier.srcAccessMask, dst, barrier.dstAccessMask);
        memory_barriers.emplace_back(sync_barrier);
    }

    // Ensure we have a barrier that handles execution dependencies.
    // NOTE: the reason to have execution barrier is explained in details in the comment for Sync2
    // MakeMemoryBarriers overload. The Sync1 implementation is much simpler since execution scopes
    // are the same for all barriers.
    if (memory_barrier_count == 0) {
        memory_barriers.emplace_back(SyncBarrier(src, dst));
    }
    single_exec_scope = true;
}

void BarrierSet::MakeBufferMemoryBarriers(const SyncValidator &sync_state, const SyncExecScope &src, const SyncExecScope &dst,
                                          uint32_t barrier_count, const VkBufferMemoryBarrier *barriers) {
    buffer_memory_barriers.reserve(barrier_count);
    for (const VkBufferMemoryBarrier &barrier : vvl::make_span(barriers, barrier_count)) {
        if (auto buffer = sync_state.Get<vvl::Buffer>(barrier.buffer)) {
            const auto range = MakeRange(*buffer, barrier.offset, barrier.size);
            const SyncBarrier sync_barrier(src, barrier.srcAccessMask, dst, barrier.dstAccessMask);
            buffer_memory_barriers.emplace_back(buffer, sync_barrier, range);
        }
    }
}

void BarrierSet::MakeMemoryBarriers(VkQueueFlags queue_flags, const VkDependencyInfo &dep_info) {
    // Collect unique execution dependencies from buffer and image barriers.
    //
    // NOTE: the reason to collect execution dependencies in addition to original buffer/image
    // barriers is because syncval applies buffer/image barriers to the memory ranges defined
    // by the resource. But execution dependency can affect any resource/memory range, not
    // only the one specified by the barrier. For example, execution dependency synchronizes
    // all READ accesses that are in scope. To emulate this behavior we collect unique
    // execution dependencies and apply them to all memory accesses (don't specify access mask).
    small_vector<std::pair<VkPipelineStageFlags2, VkPipelineStageFlags2>, 4> buffer_image_barrier_exec_deps;
    for (const VkBufferMemoryBarrier2 &buffer_barrier :
         vvl::make_span(dep_info.pBufferMemoryBarriers, dep_info.bufferMemoryBarrierCount)) {
        const auto src_dst = std::make_pair(buffer_barrier.srcStageMask, buffer_barrier.dstStageMask);
        if (!buffer_image_barrier_exec_deps.Contains(src_dst)) {
            buffer_image_barrier_exec_deps.emplace_back(src_dst);
        }
    }
    for (const VkImageMemoryBarrier2 &image_barrier :
         vvl::make_span(dep_info.pImageMemoryBarriers, dep_info.imageMemoryBarrierCount)) {
        const auto src_dst = std::make_pair(image_barrier.srcStageMask, image_barrier.dstStageMask);
        if (!buffer_image_barrier_exec_deps.Contains(src_dst)) {
            buffer_image_barrier_exec_deps.emplace_back(src_dst);
        }
    }

    memory_barriers.reserve(dep_info.memoryBarrierCount + buffer_image_barrier_exec_deps.size());

    // Add global memory barriers specified in VkDependencyInfo
    for (const VkMemoryBarrier2 &barrier : vvl::make_span(dep_info.pMemoryBarriers, dep_info.memoryBarrierCount)) {
        auto src = SyncExecScope::MakeSrc(queue_flags, barrier.srcStageMask);
        auto dst = SyncExecScope::MakeDst(queue_flags, barrier.dstStageMask);
        memory_barriers.emplace_back(SyncBarrier(src, barrier.srcAccessMask, dst, barrier.dstAccessMask));
    }
    // Add execution dependencies from buffer and image barriers
    for (const auto &src_dst : buffer_image_barrier_exec_deps) {
        auto src = SyncExecScope::MakeSrc(queue_flags, src_dst.first);
        auto dst = SyncExecScope::MakeDst(queue_flags, src_dst.second);
        memory_barriers.emplace_back(SyncBarrier(src, dst));
    }
    single_exec_scope = false;
}

void BarrierSet::MakeBufferMemoryBarriers(const SyncValidator &sync_state, VkQueueFlags queue_flags, uint32_t barrier_count,
                                          const VkBufferMemoryBarrier2 *barriers) {
    buffer_memory_barriers.reserve(barrier_count);
    for (const VkBufferMemoryBarrier2 &barrier : vvl::make_span(barriers, barrier_count)) {
        auto src = SyncExecScope::MakeSrc(queue_flags, barrier.srcStageMask);
        auto dst = SyncExecScope::MakeDst(queue_flags, barrier.dstStageMask);
        if (auto buffer = sync_state.Get<vvl::Buffer>(barrier.buffer)) {
            const auto range = MakeRange(*buffer, barrier.offset, barrier.size);
            const SyncBarrier sync_barrier(src, barrier.srcAccessMask, dst, barrier.dstAccessMask);
            buffer_memory_barriers.emplace_back(buffer, sync_barrier, range);
        }
    }
}

void BarrierSet::MakeImageMemoryBarriers(const SyncValidator &sync_state, const SyncExecScope &src, const SyncExecScope &dst,
                                         uint32_t barrier_count, const VkImageMemoryBarrier *barriers,
                                         const DeviceExtensions &extensions) {
    image_memory_barriers.reserve(barrier_count);
    for (const auto [index, barrier] : vvl::enumerate(barriers, barrier_count)) {
        if (auto image = sync_state.Get<vvl::Image>(barrier.image)) {
            auto subresource_range = image->NormalizeSubresourceRange(barrier.subresourceRange);

            // VK_REMAINING_ARRAY_LAYERS for sliced 3d image in the context of layout transition means image's depth extent.
            if (barrier.subresourceRange.layerCount == VK_REMAINING_ARRAY_LAYERS &&
                CanTransitionDepthSlices(extensions, image->create_info)) {
                subresource_range.layerCount = image->create_info.extent.depth - subresource_range.baseArrayLayer;
            }

            const SyncBarrier sync_barrier(src, barrier.srcAccessMask, dst, barrier.dstAccessMask);
            const bool layout_transition = barrier.oldLayout != barrier.newLayout;
            image_memory_barriers.emplace_back(image, sync_barrier, subresource_range, layout_transition, index);
        }
    }
}

void BarrierSet::MakeImageMemoryBarriers(const SyncValidator &sync_state, VkQueueFlags queue_flags, uint32_t barrier_count,
                                         const VkImageMemoryBarrier2 *barriers, const DeviceExtensions &extensions) {
    image_memory_barriers.reserve(barrier_count);
    for (const auto [index, barrier] : vvl::enumerate(barriers, barrier_count)) {
        auto src = SyncExecScope::MakeSrc(queue_flags, barrier.srcStageMask);
        auto dst = SyncExecScope::MakeDst(queue_flags, barrier.dstStageMask);
        auto image = sync_state.Get<vvl::Image>(barrier.image);
        if (image) {
            auto subresource_range = image->NormalizeSubresourceRange(barrier.subresourceRange);

            // VK_REMAINING_ARRAY_LAYERS for sliced 3d image in the context of layout transition means image's depth extent.
            if (barrier.subresourceRange.layerCount == VK_REMAINING_ARRAY_LAYERS &&
                CanTransitionDepthSlices(extensions, image->create_info)) {
                subresource_range.layerCount = image->create_info.extent.depth - subresource_range.baseArrayLayer;
            }

            const SyncBarrier sync_barrier(src, barrier.srcAccessMask, dst, barrier.dstAccessMask);
            const bool layout_transition = barrier.oldLayout != barrier.newLayout;
            image_memory_barriers.emplace_back(image, sync_barrier, subresource_range, layout_transition, index);
        }
    }
}

SyncOpPipelineBarrier::SyncOpPipelineBarrier(vvl::Func command, const SyncValidator &sync_state, VkQueueFlags queue_flags,
                                             VkPipelineStageFlags srcStageMask, VkPipelineStageFlags dstStageMask,
                                             uint32_t memoryBarrierCount, const VkMemoryBarrier *pMemoryBarriers,
                                             uint32_t bufferMemoryBarrierCount, const VkBufferMemoryBarrier *pBufferMemoryBarriers,
                                             uint32_t imageMemoryBarrierCount, const VkImageMemoryBarrier *pImageMemoryBarriers)
    : SyncOpBase(command) {
    const auto src_exec_scope = SyncExecScope::MakeSrc(queue_flags, srcStageMask);
    const auto dst_exec_scope = SyncExecScope::MakeDst(queue_flags, dstStageMask);
    barrier_set_.src_exec_scope = src_exec_scope;
    barrier_set_.dst_exec_scope = dst_exec_scope;
    barrier_set_.MakeMemoryBarriers(src_exec_scope, dst_exec_scope, memoryBarrierCount, pMemoryBarriers);
    barrier_set_.MakeBufferMemoryBarriers(sync_state, src_exec_scope, dst_exec_scope, bufferMemoryBarrierCount,
                                          pBufferMemoryBarriers);
    barrier_set_.MakeImageMemoryBarriers(sync_state, src_exec_scope, dst_exec_scope, imageMemoryBarrierCount, pImageMemoryBarriers,
                                         sync_state.device_state->extensions);
}

SyncOpPipelineBarrier::SyncOpPipelineBarrier(vvl::Func command, const SyncValidator &sync_state, VkQueueFlags queue_flags,
                                             const VkDependencyInfo &dep_info)
    : SyncOpBase(command) {
    const ExecScopes stage_masks = sync_utils::GetExecScopes(dep_info);
    barrier_set_.src_exec_scope = SyncExecScope::MakeSrc(queue_flags, stage_masks.src);
    barrier_set_.dst_exec_scope = SyncExecScope::MakeDst(queue_flags, stage_masks.dst);
    barrier_set_.MakeMemoryBarriers(queue_flags, dep_info);

    barrier_set_.MakeBufferMemoryBarriers(sync_state, queue_flags, dep_info.bufferMemoryBarrierCount,
                                          dep_info.pBufferMemoryBarriers);
    barrier_set_.MakeImageMemoryBarriers(sync_state, queue_flags, dep_info.imageMemoryBarrierCount, dep_info.pImageMemoryBarriers,
                                         sync_state.device_state->extensions);
}

bool SyncOpPipelineBarrier::Validate(const CommandBufferAccessContext &cb_context) const {
    bool skip = false;
    const auto *context = cb_context.GetCurrentAccessContext();
    assert(context);
    if (!context) return skip;

    // Validate Image Layout transitions
    for (const auto &image_barrier : barrier_set_.image_memory_barriers) {
        if (!image_barrier.layout_transition) {
            continue;
        }
        const vvl::Image &image_state = *image_barrier.image;
        const bool can_transition_depth_slices =
            CanTransitionDepthSlices(cb_context.GetSyncState().extensions, image_state.create_info);
        const auto hazard = context->DetectImageBarrierHazard(
            image_state, image_barrier.barrier.src_exec_scope.exec_scope, image_barrier.barrier.src_access_scope,
            image_barrier.subresource_range, can_transition_depth_slices, AccessContext::kDetectAll);
        if (hazard.IsHazard()) {
            LogObjectList objlist(cb_context.GetCBState().Handle(), image_state.Handle());
            const Location loc(command_);
            const SyncValidator &sync_state = cb_context.GetSyncState();
            const std::string resource_description = sync_state.FormatHandle(image_state.Handle());
            const std::string error =
                sync_state.error_messages_.ImageBarrierError(hazard, cb_context, command_, resource_description, image_barrier);
            skip |= sync_state.SyncError(hazard.Hazard(), objlist, loc, error);
        }
    }
    return skip;
}

ResourceUsageTag SyncOpPipelineBarrier::Record(CommandBufferAccessContext *cb_context) {
    const auto tag = cb_context->NextCommandTag(command_);
    for (const auto &buffer_barrier : barrier_set_.buffer_memory_barriers) {
        cb_context->AddCommandHandle(tag, buffer_barrier.buffer->Handle());
    }
    for (auto &image_barrier : barrier_set_.image_memory_barriers) {
        if (image_barrier.layout_transition) {
            const auto tag_ex = cb_context->AddCommandHandle(tag, image_barrier.image->Handle());
            image_barrier.handle_index = tag_ex.handle_index;
        }
    }
    ReplayRecord(*cb_context, tag);
    return tag;
}

struct ApplyGlobalBarrierFunctor {
    ApplyGlobalBarrierFunctor(QueueId queue_id, const SyncBarrier &barrier) : barrier_scope(barrier, queue_id), barrier(barrier) {}

    using Iterator = AccessMap::iterator;
    Iterator Infill(AccessMap *accesses, const Iterator &pos_hint, const AccessRange &range) const { return pos_hint; }

    void operator()(const Iterator &pos) const {
        AccessState &access_state = pos->second;
        access_state.ApplyBarrier(barrier_scope, barrier);
    }

    const BarrierScope barrier_scope;
    SyncBarrier barrier;
};

// TODO: support more uses cases of a single barrier: if single barrier is an image/buffer
// barrier then we need consider three ranges - range of the resource where to apply
// full fledged barrier and also two ranges before and after the resource where it is
// enough to apply execution dependency.
void SyncOpPipelineBarrier::ApplySingleBarrier(CommandExecutionContext &exec_context) const {
    AccessContext *access_context = exec_context.GetCurrentAccessContext();
    const QueueId queue_id = exec_context.GetQueueId();

    for (const SyncBarrier &barrier : barrier_set_.memory_barriers) {
        ApplyGlobalBarrierFunctor apply_barrier(queue_id, barrier);
        access_context->UpdateMemoryAccessRangeState(apply_barrier, kFullRange);
    }
}

void SyncOpPipelineBarrier::ApplyMultipleBarriers(CommandExecutionContext &exec_context, const ResourceUsageTag exec_tag) const {
    AccessContext *access_context = exec_context.GetCurrentAccessContext();
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
    for (const SyncBufferMemoryBarrier &barrier : barrier_set_.buffer_memory_barriers) {
        if (SimpleBinding(*barrier.buffer)) {
            const VkDeviceSize base_address = ResourceBaseAddress(*barrier.buffer);
            const AccessRange range = barrier.range + base_address;
            ApplyMarkupFunctor markup_action(false);
            access_context->UpdateMemoryAccessRangeState(markup_action, range);
        }
    }
    for (const SyncImageMemoryBarrier &barrier : barrier_set_.image_memory_barriers) {
        const auto &sub_state = SubState(*barrier.image);
        const bool can_transition_depth_slices =
            CanTransitionDepthSlices(exec_context.GetSyncState().extensions, sub_state.base.create_info);
        auto range_gen = sub_state.MakeImageRangeGen(barrier.subresource_range, can_transition_depth_slices);
        // TODO: check if we need: barrier.layout_transition && (queue_id == kQueueIdInvalid)
        ApplyMarkupFunctor markup_action(barrier.layout_transition);
        access_context->UpdateMemoryAccessState(markup_action, range_gen);
    }

    // Apply barriers independently and store the result in the pending object.
    PendingBarriers pending_barriers;
    for (const SyncBufferMemoryBarrier &barrier : barrier_set_.buffer_memory_barriers) {
        if (SimpleBinding(*barrier.buffer)) {
            const BarrierScope barrier_scope(barrier.barrier, queue_id);
            CollectBarriersFunctor collect_barriers(barrier_scope, barrier.barrier, false, vvl::kNoIndex32, pending_barriers);

            const VkDeviceSize base_address = ResourceBaseAddress(*barrier.buffer);
            const AccessRange range = barrier.range + base_address;

            access_context->UpdateMemoryAccessRangeState(collect_barriers, range);
        }
    }
    for (const SyncImageMemoryBarrier &barrier : barrier_set_.image_memory_barriers) {
        const BarrierScope barrier_scope(barrier.barrier, queue_id);
        CollectBarriersFunctor collect_barriers(barrier_scope, barrier.barrier, barrier.layout_transition, barrier.handle_index,
                                                pending_barriers);

        const auto &sub_state = SubState(*barrier.image);
        const bool can_transition_depth_slices =
            CanTransitionDepthSlices(exec_context.GetSyncState().extensions, sub_state.base.create_info);
        auto range_gen = sub_state.MakeImageRangeGen(barrier.subresource_range, can_transition_depth_slices);

        access_context->UpdateMemoryAccessState(collect_barriers, range_gen);
    }
    for (const SyncBarrier &barrier : barrier_set_.memory_barriers) {
        const BarrierScope barrier_scope(barrier, queue_id);
        CollectBarriersFunctor collect_barriers(barrier_scope, barrier, false, vvl::kNoIndex32, pending_barriers);
        access_context->UpdateMemoryAccessRangeState(collect_barriers, kFullRange);
    }

    // Update access states with collected barriers
    pending_barriers.Apply(exec_tag);
}

void SyncOpPipelineBarrier::ReplayRecord(CommandExecutionContext &exec_context, const ResourceUsageTag exec_tag) const {
    if (!exec_context.ValidForSyncOps()) {
        return;
    }

    if (barrier_set_.memory_barriers.size() == 1 && barrier_set_.buffer_memory_barriers.empty() &&
        barrier_set_.image_memory_barriers.empty()) {
        ApplySingleBarrier(exec_context);
    } else {
        ApplyMultipleBarriers(exec_context, exec_tag);
    }

    SyncEventsContext *events_context = exec_context.GetCurrentEventsContext();
    if (barrier_set_.single_exec_scope) {
        events_context->ApplyBarrier(barrier_set_.src_exec_scope, barrier_set_.dst_exec_scope, exec_tag);
    } else {
        for (const auto &barrier : barrier_set_.memory_barriers) {
            events_context->ApplyBarrier(barrier.src_exec_scope, barrier.dst_exec_scope, exec_tag);
        }
    }
}

bool SyncOpPipelineBarrier::ReplayValidate(ReplayState &replay, ResourceUsageTag recorded_tag) const {
    // The layout transitions happen at the replay tag
    ResourceUsageRange first_use_range = {recorded_tag, recorded_tag + 1};
    return replay.DetectFirstUseHazard(first_use_range);
}

SyncOpWaitEvents::SyncOpWaitEvents(vvl::Func command, const SyncValidator &sync_state, VkQueueFlags queue_flags,
                                   uint32_t eventCount, const VkEvent *pEvents, VkPipelineStageFlags srcStageMask,
                                   VkPipelineStageFlags dstStageMask, uint32_t memoryBarrierCount,
                                   const VkMemoryBarrier *pMemoryBarriers, uint32_t bufferMemoryBarrierCount,
                                   const VkBufferMemoryBarrier *pBufferMemoryBarriers, uint32_t imageMemoryBarrierCount,
                                   const VkImageMemoryBarrier *pImageMemoryBarriers)
    : SyncOpBase(command), barrier_sets_(1) {
    auto &barrier_set = barrier_sets_[0];
    const auto src_exec_scope = SyncExecScope::MakeSrc(queue_flags, srcStageMask);
    const auto dst_exec_scope = SyncExecScope::MakeDst(queue_flags, dstStageMask);
    barrier_set.src_exec_scope = src_exec_scope;
    barrier_set.dst_exec_scope = dst_exec_scope;
    barrier_set.MakeMemoryBarriers(src_exec_scope, dst_exec_scope, memoryBarrierCount, pMemoryBarriers);
    barrier_set.MakeBufferMemoryBarriers(sync_state, src_exec_scope, dst_exec_scope, bufferMemoryBarrierCount,
                                         pBufferMemoryBarriers);
    barrier_set.MakeImageMemoryBarriers(sync_state, src_exec_scope, dst_exec_scope, imageMemoryBarrierCount, pImageMemoryBarriers,
                                        sync_state.device_state->extensions);
    MakeEventsList(sync_state, eventCount, pEvents);
}

SyncOpWaitEvents::SyncOpWaitEvents(vvl::Func command, const SyncValidator &sync_state, VkQueueFlags queue_flags,
                                   uint32_t eventCount, const VkEvent *pEvents, const VkDependencyInfo *pDependencyInfo)
    : SyncOpBase(command), barrier_sets_(eventCount) {
    for (uint32_t i = 0; i < eventCount; i++) {
        const auto &dep_info = pDependencyInfo[i];
        auto &barrier_set = barrier_sets_[i];
        auto stage_masks = sync_utils::GetExecScopes(dep_info);
        barrier_set.src_exec_scope = SyncExecScope::MakeSrc(queue_flags, stage_masks.src);
        barrier_set.dst_exec_scope = SyncExecScope::MakeDst(queue_flags, stage_masks.dst);
        barrier_set.MakeMemoryBarriers(queue_flags, dep_info);
        barrier_set.MakeBufferMemoryBarriers(sync_state, queue_flags, dep_info.bufferMemoryBarrierCount,
                                             dep_info.pBufferMemoryBarriers);
        barrier_set.MakeImageMemoryBarriers(sync_state, queue_flags, dep_info.imageMemoryBarrierCount,
                                            dep_info.pImageMemoryBarriers, sync_state.device_state->extensions);
    }
    MakeEventsList(sync_state, eventCount, pEvents);
}

const char *const SyncOpWaitEvents::kIgnored = "Wait operation is ignored for this event.";

bool SyncOpWaitEvents::Validate(const CommandBufferAccessContext &cb_context) const {
    bool skip = false;
    const auto &sync_state = cb_context.GetSyncState();
    const VkCommandBuffer command_buffer_handle = cb_context.GetCBState().VkHandle();

    // This is only interesting at record and not replay (Execute/Submit) time.
    for (size_t barrier_set_index = 0; barrier_set_index < barrier_sets_.size(); barrier_set_index++) {
        const auto &barrier_set = barrier_sets_[barrier_set_index];
        if (barrier_set.single_exec_scope) {
            const Location loc(command_);
            if (barrier_set.src_exec_scope.mask_param & VK_PIPELINE_STAGE_HOST_BIT) {
                const std::string vuid = std::string("SYNC-") + std::string(CmdName()) + std::string("-hostevent-unsupported");
                sync_state.LogInfo(vuid, command_buffer_handle, loc,
                                   "srcStageMask includes %s, unsupported by synchronization validation.",
                                   string_VkPipelineStageFlagBits(VK_PIPELINE_STAGE_HOST_BIT));
            } else {
                const auto &barriers = barrier_set.memory_barriers;
                for (size_t barrier_index = 0; barrier_index < barriers.size(); barrier_index++) {
                    const auto &barrier = barriers[barrier_index];
                    if (barrier.src_exec_scope.mask_param & VK_PIPELINE_STAGE_HOST_BIT) {
                        const std::string vuid =
                            std::string("SYNC-") + std::string(CmdName()) + std::string("-hostevent-unsupported");

                        sync_state.LogInfo(vuid, command_buffer_handle, loc,
                                           "srcStageMask %s of %s %zu, %s %zu, unsupported by synchronization validation.",
                                           string_VkPipelineStageFlagBits(VK_PIPELINE_STAGE_HOST_BIT), "pDependencyInfo",
                                           barrier_set_index, "pMemoryBarriers", barrier_index);
                    }
                }
            }
        }
    }

    // The rest is common to record time and replay time.
    skip |= DoValidate(cb_context, ResourceUsageRecord::kMaxIndex);
    return skip;
}

bool SyncOpWaitEvents::DoValidate(const CommandExecutionContext &exec_context, const ResourceUsageTag base_tag) const {
    bool skip = false;
    const auto &sync_state = exec_context.GetSyncState();
    const QueueId queue_id = exec_context.GetQueueId();

    VkPipelineStageFlags2 event_stage_masks = 0U;
    VkPipelineStageFlags2 barrier_mask_params = 0U;
    bool events_not_found = false;
    const auto *events_context = exec_context.GetCurrentEventsContext();
    assert(events_context);
    size_t barrier_set_index = 0;
    size_t barrier_set_incr = (barrier_sets_.size() == 1) ? 0 : 1;
    const Location loc(command_);
    for (const auto &event : events_) {
        const auto *sync_event = events_context->Get(event.get());
        const auto &barrier_set = barrier_sets_[barrier_set_index];
        if (!sync_event) {
            // NOTE PHASE2: This is where we'll need queue submit time validation to come back and check the srcStageMask bits
            //              or solve this with replay creating the SyncEventState in the queue context... also this will be a
            //              new validation error... wait without previously submitted set event...
            events_not_found = true;  // Demote "extra_stage_bits" error to warning, to avoid false positives at *record time*
            barrier_set_index += barrier_set_incr;
            continue;  // Core, Lifetimes, or Param check needs to catch invalid events.
        }

        // For replay calls, don't revalidate "same command buffer" events
        if (sync_event->last_command_tag >= base_tag) continue;

        const VkEvent event_handle = sync_event->event->VkHandle();
        // TODO add "destroyed" checks

        if (sync_event->first_scope) {
            // Only accumulate barrier and event stages if there is a pending set in the current context
            barrier_mask_params |= barrier_set.src_exec_scope.mask_param;
            event_stage_masks |= sync_event->scope.mask_param;
        }

        const auto &src_exec_scope = barrier_set.src_exec_scope;

        const auto ignore_reason = sync_event->IsIgnoredByWait(command_, src_exec_scope.mask_param);
        if (ignore_reason) {
            switch (ignore_reason) {
                case SyncEventState::ResetWaitRace:
                case SyncEventState::Reset2WaitRace: {
                    // Four permuations of Reset and Wait calls...
                    const char *vuid = (command_ == vvl::Func::vkCmdWaitEvents) ? "VUID-vkCmdResetEvent-event-03834"
                                                                                : "VUID-vkCmdResetEvent-event-03835";
                    if (ignore_reason == SyncEventState::Reset2WaitRace) {
                        vuid = (command_ == vvl::Func::vkCmdWaitEvents) ? "VUID-vkCmdResetEvent2-event-03831"
                                                                        : "VUID-vkCmdResetEvent2-event-03832";
                    }
                    const char *const message =
                        "%s %s operation following %s without intervening execution barrier, may cause race condition. %s";
                    skip |= sync_state.LogError(vuid, event_handle, loc, message, sync_state.FormatHandle(event_handle).c_str(),
                                                CmdName(), vvl::String(sync_event->last_command), kIgnored);
                    break;
                }
                case SyncEventState::SetRace: {
                    // Issue error message that Wait is waiting on an signal subject to race condition, and is thus ignored for
                    // this event
                    const char *const vuid = "SYNC-vkCmdWaitEvents-unsynchronized-setops";
                    const char *const message =
                        "%s Unsychronized %s calls result in race conditions w.r.t. event signalling, %s %s";
                    const char *const reason = "First synchronization scope is undefined.";
                    skip |= sync_state.LogError(vuid, event_handle, loc, message, sync_state.FormatHandle(event_handle).c_str(),
                                                vvl::String(sync_event->last_command), reason, kIgnored);
                    break;
                }
                case SyncEventState::MissingStageBits: {
                    const auto missing_bits = sync_event->scope.mask_param & ~src_exec_scope.mask_param;
                    // Issue error message that event waited for is not in wait events scope
                    const char *const vuid = "VUID-vkCmdWaitEvents-srcStageMask-01158";
                    const char *const message =
                        "%s stageMask %s includes stages not present in srcStageMask %s. Stages missing from srcStageMask: %s. %s";
                    skip |= sync_state.LogError(vuid, event_handle, loc, message, sync_state.FormatHandle(event_handle).c_str(),
                                                sync_utils::StringPipelineStageFlags(sync_event->scope.mask_param).c_str(),
                                                sync_utils::StringPipelineStageFlags(src_exec_scope.mask_param).c_str(),
                                                sync_utils::StringPipelineStageFlags(missing_bits).c_str(), kIgnored);
                    break;
                }
                case SyncEventState::SetVsWait2: {
                    skip |= sync_state.LogError(
                        "VUID-vkCmdWaitEvents2-pEvents-03837", event_handle, loc, "Follows set of %s by %s. Disallowed.",
                        sync_state.FormatHandle(event_handle).c_str(), vvl::String(sync_event->last_command));
                    break;
                }
                case SyncEventState::MissingSetEvent: {
                    // TODO: There are conditions at queue submit time where we can definitively say that
                    // a missing set event is an error.  Add those if not captured in CoreChecks
                    break;
                }
                default:
                    assert(ignore_reason == SyncEventState::NotIgnored);
            }
        } else if (barrier_set.image_memory_barriers.size()) {
            const auto &image_memory_barriers = barrier_set.image_memory_barriers;
            const auto *context = exec_context.GetCurrentAccessContext();
            assert(context);
            for (const auto &image_memory_barrier : image_memory_barriers) {
                if (!image_memory_barrier.layout_transition) continue;
                const auto *image_state = image_memory_barrier.image.get();
                if (!image_state) continue;
                const auto &subresource_range = image_memory_barrier.subresource_range;
                const auto &src_access_scope = image_memory_barrier.barrier.src_access_scope;
                const auto hazard = context->DetectImageBarrierHazard(
                    *image_state, subresource_range, sync_event->scope.exec_scope, src_access_scope, queue_id,
                    sync_event->FirstScope(), sync_event->first_scope_tag, AccessContext::DetectOptions::kDetectAll);
                if (hazard.IsHazard()) {
                    LogObjectList objlist(exec_context.Handle(), image_state->Handle());
                    const std::string resource_description = sync_state.FormatHandle(image_state->Handle());
                    const std::string error = sync_state.error_messages_.ImageBarrierError(
                        hazard, exec_context, command_, resource_description, image_memory_barrier);
                    skip |= sync_state.SyncError(hazard.Hazard(), image_state->Handle(), loc, error);
                    break;
                }
            }
        }
        // TODO:  Add infrastructure for checking pDependencyInfo's vs. CmdSetEvent2 VUID - vkCmdWaitEvents2KHR - pEvents -
        // 03839
        barrier_set_index += barrier_set_incr;
    }

    // Note that we can't check for HOST in pEvents as we don't track that set event type
    const auto extra_stage_bits = (barrier_mask_params & ~VK_PIPELINE_STAGE_2_HOST_BIT) & ~event_stage_masks;
    if (extra_stage_bits) {
        assert(vvl::Func::vkCmdWaitEvents == command_);
        // Issue error message that event waited for is not in wait events scope
        const char *const message =
            "srcStageMask 0x%" PRIx64 " contains stages not present in pEvents stageMask. Extra stages are %s.%s";
        const auto handle = exec_context.Handle();
        if (!events_not_found) {
            skip |= sync_state.LogError("VUID-vkCmdWaitEvents-srcStageMask-01158", handle, loc, message, barrier_mask_params,
                                        sync_utils::StringPipelineStageFlags(extra_stage_bits).c_str(), "");
        }
    }
    return skip;
}

ResourceUsageTag SyncOpWaitEvents::Record(CommandBufferAccessContext *cb_context) {
    const auto tag = cb_context->NextCommandTag(command_);

    ReplayRecord(*cb_context, tag);
    return tag;
}

// Need to restrict to only valid exec and access scope for this event
static SyncBarrier RestrictToEvent(const SyncBarrier &barrier, const SyncEventState &sync_event) {
    SyncBarrier result = barrier;
    result.src_exec_scope.exec_scope = sync_event.scope.exec_scope & barrier.src_exec_scope.exec_scope;
    result.src_access_scope = sync_event.scope.valid_accesses & barrier.src_access_scope;
    return result;
}

void SyncOpWaitEvents::ReplayRecord(CommandExecutionContext &exec_context, ResourceUsageTag exec_tag) const {
    // Unlike PipelineBarrier, WaitEvent is *not* limited to accesses within the current subpass (if any) and thus needs to import
    // all accesses. Can instead import for all first_scopes, or a union of them, if this becomes a performance/memory issue,
    // but with no idea of the performance of the union, nor of whether it even matters... take the simplest approach here,
    if (!exec_context.ValidForSyncOps()) return;
    AccessContext *access_context = exec_context.GetCurrentAccessContext();
    SyncEventsContext *events_context = exec_context.GetCurrentEventsContext();
    const QueueId queue_id = exec_context.GetQueueId();

    access_context->ResolvePreviousAccesses();

    assert(barrier_sets_.size() == 1 || (barrier_sets_.size() == events_.size()));

    // Apply markup action.
    // The markup action does not change any access state but it can trim the access map according to the
    // provided range and creates infill ranges if necessary (for layout transitions). The purpose of all
    // this is to ensure that after markup action the topology of access map ranges is finalized for the
    // duration of barrier application (so we can cache pointers to specific access states with a goal
    // to apply pending barriers in the end).
    //
    // NOTE: event's global barriers can split() access map because EventSimpleRangeGenerator filters kFullRange.
    // That's why, in contrast to SyncOpPipelineBarrier, we need apply markup action also to global barriers.
    // TODO: need a test that demonstrates this (when doing some work on syncval events)
    size_t barrier_set_index = 0;
    size_t barrier_set_incr = (barrier_sets_.size() == 1) ? 0 : 1;
    for (auto &event_shared : events_) {
        if (!event_shared.get()) continue;
        auto *sync_event = events_context->GetFromShared(event_shared);

        sync_event->last_command = command_;
        sync_event->last_command_tag = exec_tag;

        const auto &barrier_set = barrier_sets_[barrier_set_index];
        if (!sync_event->IsIgnoredByWait(command_, barrier_set.src_exec_scope.mask_param)) {
            for (const SyncBufferMemoryBarrier &barrier : barrier_set.buffer_memory_barriers) {
                if (SimpleBinding(*barrier.buffer)) {
                    const VkDeviceSize base_address = ResourceBaseAddress(*barrier.buffer);
                    const AccessRange range = barrier.range + base_address;
                    EventSimpleRangeGenerator filtered_range_gen(sync_event->FirstScope(), range);
                    ApplyMarkupFunctor markup_action(false);
                    access_context->UpdateMemoryAccessState(markup_action, filtered_range_gen);
                }
            }
            for (const SyncImageMemoryBarrier &barrier : barrier_set.image_memory_barriers) {
                const auto &sub_state = SubState(*barrier.image);
                const bool can_transition_depth_slices =
                    CanTransitionDepthSlices(exec_context.GetSyncState().extensions, sub_state.base.create_info);
                ImageRangeGen range_gen = sub_state.MakeImageRangeGen(barrier.subresource_range, can_transition_depth_slices);
                EventImageRangeGenerator filtered_range_gen(sync_event->FirstScope(), range_gen);
                ApplyMarkupFunctor markup_action(barrier.layout_transition);
                access_context->UpdateMemoryAccessState(markup_action, filtered_range_gen);
            }
            auto global_barriers_range_gen = EventSimpleRangeGenerator(sync_event->FirstScope(), kFullRange);
            ApplyMarkupFunctor markup_action(false);
            access_context->UpdateMemoryAccessState(markup_action, global_barriers_range_gen);
        }
        barrier_set_index += barrier_set_incr;
    }

    // Apply barriers independently and store the result in the pending object.
    PendingBarriers pending_barriers;
    barrier_set_index = 0;
    barrier_set_incr = (barrier_sets_.size() == 1) ? 0 : 1;
    for (auto &event_shared : events_) {
        if (!event_shared.get()) continue;
        auto *sync_event = events_context->GetFromShared(event_shared);

        const auto &barrier_set = barrier_sets_[barrier_set_index];
        const auto &dst = barrier_set.dst_exec_scope;
        if (!sync_event->IsIgnoredByWait(command_, barrier_set.src_exec_scope.mask_param)) {
            // These apply barriers one at a time as the are restricted to the resource ranges specified per each barrier,
            // but do not update the dependency chain information (but set the "pending" state) // s.t. the order independence
            // of the barriers is maintained.

            for (const SyncBufferMemoryBarrier &barrier : barrier_set.buffer_memory_barriers) {
                if (SimpleBinding(*barrier.buffer)) {
                    const SyncBarrier event_barrier = RestrictToEvent(barrier.barrier, *sync_event);
                    const BarrierScope barrier_scope(event_barrier, queue_id, sync_event->first_scope_tag);
                    CollectBarriersFunctor collect_barriers(barrier_scope, event_barrier, false, vvl::kNoIndex32, pending_barriers);

                    const VkDeviceSize base_address = ResourceBaseAddress(*barrier.buffer);
                    const AccessRange range = barrier.range + base_address;
                    EventSimpleRangeGenerator range_gen(sync_event->FirstScope(), range);

                    access_context->UpdateMemoryAccessState(collect_barriers, range_gen);
                }
            }
            for (const SyncImageMemoryBarrier &barrier : barrier_set.image_memory_barriers) {
                const SyncBarrier event_barrier = RestrictToEvent(barrier.barrier, *sync_event);
                const BarrierScope barrier_scope(event_barrier, queue_id, sync_event->first_scope_tag);
                CollectBarriersFunctor collect_barriers(barrier_scope, event_barrier, barrier.layout_transition,
                                                        barrier.handle_index, pending_barriers);

                const auto &sub_state = SubState(*barrier.image);
                const bool can_transition_depth_slices =
                    CanTransitionDepthSlices(exec_context.GetSyncState().extensions, sub_state.base.create_info);
                ImageRangeGen range_gen = sub_state.MakeImageRangeGen(barrier.subresource_range, can_transition_depth_slices);
                EventImageRangeGenerator filtered_range_gen(sync_event->FirstScope(), range_gen);

                access_context->UpdateMemoryAccessState(collect_barriers, filtered_range_gen);
            }
            // TODO: because each iteration applies functor to the same range, investigate if it is
            // beneficial for the functor to support multiple barriers, so we traverse access map once.
            auto global_range_gen = EventSimpleRangeGenerator(sync_event->FirstScope(), kFullRange);
            for (const auto &barrier : barrier_set.memory_barriers) {
                const SyncBarrier event_barrier = RestrictToEvent(barrier, *sync_event);
                const BarrierScope barrier_scope(event_barrier, queue_id, sync_event->first_scope_tag);
                CollectBarriersFunctor collect_barriers(barrier_scope, event_barrier, false, vvl::kNoIndex32, pending_barriers);

                auto range_gen = global_range_gen;  // intentional copy
                access_context->UpdateMemoryAccessState(collect_barriers, range_gen);
            }

            // Apply the global barrier to the event itself (for race condition tracking)
            // Events don't happen at a stage, so we need to store the unexpanded ALL_COMMANDS if set for inter-event-calls
            sync_event->barriers = dst.mask_param & VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;
            sync_event->barriers |= dst.exec_scope;
        } else {
            // We ignored this wait, so we don't have any effective synchronization barriers for it.
            sync_event->barriers = 0U;
        }
        barrier_set_index += barrier_set_incr;
    }

    // Update access states with collected barriers
    pending_barriers.Apply(exec_tag);
}

bool SyncOpWaitEvents::ReplayValidate(ReplayState &replay, ResourceUsageTag recorded_tag) const {
    return DoValidate(replay.GetExecutionContext(), replay.GetBaseTag() + recorded_tag);
}

void SyncOpWaitEvents::MakeEventsList(const SyncValidator &sync_state, uint32_t event_count, const VkEvent *events) {
    events_.reserve(event_count);
    for (uint32_t event_index = 0; event_index < event_count; event_index++) {
        events_.emplace_back(sync_state.Get<vvl::Event>(events[event_index]));
    }
}

SyncOpResetEvent::SyncOpResetEvent(vvl::Func command, const SyncValidator &sync_state, VkQueueFlags queue_flags, VkEvent event,
                                   VkPipelineStageFlags2 stageMask)
    : SyncOpBase(command), event_(sync_state.Get<vvl::Event>(event)), exec_scope_(SyncExecScope::MakeSrc(queue_flags, stageMask)) {}

bool SyncOpResetEvent::Validate(const CommandBufferAccessContext &cb_context) const {
    return DoValidate(cb_context, ResourceUsageRecord::kMaxIndex);
}

bool SyncOpResetEvent::DoValidate(const CommandExecutionContext &exec_context, const ResourceUsageTag base_tag) const {
    auto *events_context = exec_context.GetCurrentEventsContext();
    assert(events_context);
    bool skip = false;
    if (!events_context) return skip;

    const auto &sync_state = exec_context.GetSyncState();
    const auto *sync_event = events_context->Get(event_);
    if (!sync_event) return skip;  // Core, Lifetimes, or Param check needs to catch invalid events.

    if (sync_event->last_command_tag > base_tag) return skip;  // if we validated this in recording of the secondary, don't repeat

    const char *const set_wait =
        "%s %s operation following %s without intervening execution barrier, is a race condition and may result in data "
        "hazards.";
    const char *message = set_wait;  // Only one message this call.
    if (!sync_event->HasBarrier(exec_scope_.mask_param, exec_scope_.exec_scope)) {
        const char *vuid = nullptr;
        switch (sync_event->last_command) {
            case vvl::Func::vkCmdSetEvent:
            case vvl::Func::vkCmdSetEvent2KHR:
            case vvl::Func::vkCmdSetEvent2:
                // Needs a barrier between set and reset
                vuid = "SYNC-vkCmdResetEvent-missingbarrier-set";
                break;
            case vvl::Func::vkCmdWaitEvents:
            case vvl::Func::vkCmdWaitEvents2KHR:
            case vvl::Func::vkCmdWaitEvents2: {
                // Needs to be in the barriers chain (either because of a barrier, or because of dstStageMask
                vuid = "SYNC-vkCmdResetEvent-missingbarrier-wait";
                break;
            }
            case vvl::Func::Empty:
            case vvl::Func::vkCmdResetEvent:
            case vvl::Func::vkCmdResetEvent2KHR:
            case vvl::Func::vkCmdResetEvent2:
                break;  // Valid, but nothing to do
            default:
                assert(false);
                break;
        }
        if (vuid) {
            const Location loc(command_);
            skip |= sync_state.LogError(vuid, event_->Handle(), loc, message, sync_state.FormatHandle(event_->Handle()).c_str(),
                                        CmdName(), vvl::String(sync_event->last_command));
        }
    }
    return skip;
}

ResourceUsageTag SyncOpResetEvent::Record(CommandBufferAccessContext *cb_context) {
    const auto tag = cb_context->NextCommandTag(command_);
    ReplayRecord(*cb_context, tag);
    return tag;
}

bool SyncOpResetEvent::ReplayValidate(ReplayState &replay, ResourceUsageTag recorded_tag) const {
    return DoValidate(replay.GetExecutionContext(), replay.GetBaseTag() + recorded_tag);
}

void SyncOpResetEvent::ReplayRecord(CommandExecutionContext &exec_context, ResourceUsageTag exec_tag) const {
    if (!exec_context.ValidForSyncOps()) return;
    SyncEventsContext *events_context = exec_context.GetCurrentEventsContext();

    auto *sync_event = events_context->GetFromShared(event_);
    if (!sync_event) return;  // Core, Lifetimes, or Param check needs to catch invalid events.

    // Update the event state
    sync_event->last_command = command_;
    sync_event->last_command_tag = exec_tag;
    sync_event->unsynchronized_set = vvl::Func::Empty;
    sync_event->ResetFirstScope();
    sync_event->barriers = 0U;
}

SyncOpSetEvent::SyncOpSetEvent(vvl::Func command, const SyncValidator &sync_state, VkQueueFlags queue_flags, VkEvent event,
                               VkPipelineStageFlags2 stageMask, const AccessContext *access_context)
    : SyncOpBase(command),
      event_(sync_state.Get<vvl::Event>(event)),
      src_exec_scope_(SyncExecScope::MakeSrc(queue_flags, stageMask)),
      dep_info_() {
    // Snapshot the current access_context for later inspection at wait time.
    // NOTE: This appears brute force, but given that we only save a "first-last" model of access history, the current
    //       access context (include barrier state for chaining) won't necessarily contain the needed information at Wait
    //       or Submit time reference.
    if (access_context) {
        auto new_context = std::make_shared<AccessContext>();
        new_context->InitFrom(*access_context);
        recorded_context_ = new_context;
    }
}

SyncOpSetEvent::SyncOpSetEvent(vvl::Func command, const SyncValidator &sync_state, VkQueueFlags queue_flags, VkEvent event,
                               const VkDependencyInfo &dep_info, const AccessContext *access_context)
    : SyncOpBase(command),
      event_(sync_state.Get<vvl::Event>(event)),
      src_exec_scope_(SyncExecScope::MakeSrc(queue_flags, sync_utils::GetExecScopes(dep_info).src)),
      dep_info_(new vku::safe_VkDependencyInfo(&dep_info)) {
    if (access_context) {
        auto new_context = std::make_shared<AccessContext>();
        new_context->InitFrom(*access_context);
        recorded_context_ = new_context;
    }
}

bool SyncOpSetEvent::Validate(const CommandBufferAccessContext &cb_context) const {
    return DoValidate(cb_context, ResourceUsageRecord::kMaxIndex);
}
bool SyncOpSetEvent::ReplayValidate(ReplayState &replay, ResourceUsageTag recorded_tag) const {
    return DoValidate(replay.GetExecutionContext(), replay.GetBaseTag() + recorded_tag);
}

bool SyncOpSetEvent::DoValidate(const CommandExecutionContext &exec_context, const ResourceUsageTag base_tag) const {
    bool skip = false;

    const auto &sync_state = exec_context.GetSyncState();
    auto *events_context = exec_context.GetCurrentEventsContext();
    assert(events_context);
    if (!events_context) return skip;

    const auto *sync_event = events_context->Get(event_);
    if (!sync_event) return skip;  // Core, Lifetimes, or Param check needs to catch invalid events.

    if (sync_event->last_command_tag >= base_tag) return skip;  // for replay we don't want to revalidate internal "last commmand"

    const char *const reset_set =
        "%s %s operation following %s without intervening execution barrier, is a race condition and may result in data "
        "hazards.";
    const char *const wait =
        "%s %s operation following %s without intervening vkCmdResetEvent, may result in data hazard and is ignored.";

    if (!sync_event->HasBarrier(src_exec_scope_.mask_param, src_exec_scope_.exec_scope)) {
        const char *vuid_stem = nullptr;
        const char *message = nullptr;
        switch (sync_event->last_command) {
            case vvl::Func::vkCmdResetEvent:
            case vvl::Func::vkCmdResetEvent2KHR:
            case vvl::Func::vkCmdResetEvent2:
                // Needs a barrier between reset and set
                vuid_stem = "-missingbarrier-reset";
                message = reset_set;
                break;
            case vvl::Func::vkCmdSetEvent:
            case vvl::Func::vkCmdSetEvent2KHR:
            case vvl::Func::vkCmdSetEvent2:
                // Needs a barrier between set and set
                vuid_stem = "-missingbarrier-set";
                message = reset_set;
                break;
            case vvl::Func::vkCmdWaitEvents:
            case vvl::Func::vkCmdWaitEvents2KHR:
            case vvl::Func::vkCmdWaitEvents2:
                // Needs a barrier or is in second execution scope
                vuid_stem = "-missingbarrier-wait";
                message = wait;
                break;
            default:
                // The only other valid last command that wasn't one.
                assert(sync_event->last_command == vvl::Func::Empty);
                break;
        }
        if (vuid_stem) {
            assert(nullptr != message);
            const Location loc(command_);
            std::string vuid("SYNC-");
            vuid.append(CmdName()).append(vuid_stem);
            skip |=
                sync_state.LogError(vuid.c_str(), event_->Handle(), loc, message, sync_state.FormatHandle(event_->Handle()).c_str(),
                                    CmdName(), vvl::String(sync_event->last_command));
        }
    }

    return skip;
}

ResourceUsageTag SyncOpSetEvent::Record(CommandBufferAccessContext *cb_context) {
    const auto tag = cb_context->NextCommandTag(command_);
    auto *events_context = cb_context->GetCurrentEventsContext();
    const QueueId queue_id = cb_context->GetQueueId();
    assert(recorded_context_);
    if (recorded_context_ && events_context) {
        DoRecord(queue_id, tag, recorded_context_, events_context);
    }
    return tag;
}

void SyncOpSetEvent::ReplayRecord(CommandExecutionContext &exec_context, ResourceUsageTag exec_tag) const {
    // Create a copy of the current context, and merge in the state snapshot at record set event time
    // Note: we mustn't change the recorded context copy, as a given CB could be submitted more than once (in generaL)
    if (!exec_context.ValidForSyncOps()) return;
    SyncEventsContext *events_context = exec_context.GetCurrentEventsContext();
    AccessContext *access_context = exec_context.GetCurrentAccessContext();
    const QueueId queue_id = exec_context.GetQueueId();

    // Note: merged_context is a copy of the access_context, combined with the recorded context
    auto merged_context = std::make_shared<AccessContext>();
    merged_context->InitFrom(*access_context);
    merged_context->ResolveFromContext(QueueTagOffsetBarrierAction(queue_id, exec_tag), *recorded_context_);
    merged_context->TrimAndClearFirstAccess();  // Ensure the copy is minimal and normalized
    DoRecord(queue_id, exec_tag, merged_context, events_context);
}

void SyncOpSetEvent::DoRecord(QueueId queue_id, ResourceUsageTag tag, const std::shared_ptr<const AccessContext> &access_context,
                              SyncEventsContext *events_context) const {
    auto *sync_event = events_context->GetFromShared(event_);
    if (!sync_event) return;  // Core, Lifetimes, or Param check needs to catch invalid events.

    // NOTE: We're going to simply record the sync scope here, as anything else would be implementation defined/undefined
    //       and we're issuing errors re: missing barriers between event commands, which if the user fixes would fix
    //       any issues caused by naive scope setting here.

    // What happens with two SetEvent is that one cannot know what group of operations will be waited for.
    // Given:
    //     Stuff1; SetEvent; Stuff2; SetEvent; WaitEvents;
    // WaitEvents cannot know which of Stuff1, Stuff2, or both has completed execution.

    if (!sync_event->HasBarrier(src_exec_scope_.mask_param, src_exec_scope_.exec_scope)) {
        sync_event->unsynchronized_set = sync_event->last_command;
        sync_event->ResetFirstScope();
    } else if (!sync_event->first_scope) {
        // We only set the scope if there isn't one
        sync_event->scope = src_exec_scope_;

        // Save the shared_ptr to copy of the access_context present at set time (sent us by the caller)
        sync_event->first_scope = access_context;
        sync_event->unsynchronized_set = vvl::Func::Empty;
        sync_event->first_scope_tag = tag;
    }
    // TODO: Store dep_info_ shared ptr in sync_state for WaitEvents2 validation
    sync_event->last_command = command_;
    sync_event->last_command_tag = tag;
    sync_event->barriers = 0U;
}

SyncOpBeginRenderPass::SyncOpBeginRenderPass(vvl::Func command, const SyncValidator &sync_state,
                                             const VkRenderPassBeginInfo *pRenderPassBegin,
                                             const VkSubpassBeginInfo *pSubpassBeginInfo)
    : SyncOpBase(command), rp_context_(nullptr) {
    if (pRenderPassBegin) {
        rp_state_ = sync_state.Get<vvl::RenderPass>(pRenderPassBegin->renderPass);
        renderpass_begin_info_ = vku::safe_VkRenderPassBeginInfo(pRenderPassBegin);
        auto fb_state = sync_state.Get<vvl::Framebuffer>(pRenderPassBegin->framebuffer);
        if (fb_state) {
            shared_attachments_ = sync_state.device_state->GetAttachmentViews(*renderpass_begin_info_.ptr(), *fb_state);
            // TODO: Revisit this when all attachment validation is through SyncOps to see if we can discard the plain pointer copy
            // Note that this a safe to presist as long as shared_attachments is not cleared
            attachments_.reserve(shared_attachments_.size());
            for (const auto &attachment : shared_attachments_) {
                attachments_.emplace_back(attachment.get());
            }
        }
        if (pSubpassBeginInfo) {
            subpass_begin_info_ = vku::safe_VkSubpassBeginInfo(pSubpassBeginInfo);
        }
    }
}

bool SyncOpBeginRenderPass::Validate(const CommandBufferAccessContext &cb_context) const {
    // Check if any of the layout transitions are hazardous.... but we don't have the renderpass context to work with, so we
    bool skip = false;

    assert(rp_state_.get());
    if (nullptr == rp_state_.get()) return skip;
    auto &rp_state = *rp_state_.get();

    const uint32_t subpass = 0;

    // Construct the state to validate against (since validation is const and RecordCmdBeginRenderPass hasn't happened yet).
    // TODO: investigate if using nullptr in InitFrom is safe (this just follows the initial implementation - it assumes
    // that array of subpass dependencies won't be indexed, but it's not obvious).
    AccessContext temp_context;

    temp_context.InitFrom(subpass, cb_context.GetQueueFlags(), rp_state.subpass_dependencies, nullptr,
                          cb_context.GetCurrentAccessContext());

    // Validate attachment operations
    if (attachments_.empty()) return skip;
    const auto &render_area = renderpass_begin_info_.renderArea;

    // Since the isn't a valid RenderPassAccessContext until Record, needs to create the view/generator list... we could limit this
    // by predicating on whether subpass 0 uses the attachment if it is too expensive to create the full list redundantly here.
    // More broadly we could look at thread specific state shared between Validate and Record as is done for other heavyweight
    // operations (though it's currently a messy approach)
    AttachmentViewGenVector view_gens = RenderPassAccessContext::CreateAttachmentViewGen(render_area, attachments_);
    skip |= RenderPassAccessContext::ValidateLayoutTransitions(cb_context, temp_context, rp_state, render_area, subpass, view_gens,
                                                               command_);

    // Validate load operations if there were no layout transition hazards
    if (!skip) {
        RenderPassAccessContext::RecordLayoutTransitions(rp_state, subpass, view_gens, kInvalidTag, temp_context);
        skip |= RenderPassAccessContext::ValidateLoadOperation(cb_context, temp_context, rp_state, render_area, subpass, view_gens,
                                                               command_);
    }

    return skip;
}

ResourceUsageTag SyncOpBeginRenderPass::Record(CommandBufferAccessContext *cb_context) {
    assert(rp_state_.get());
    if (nullptr == rp_state_.get()) return cb_context->NextCommandTag(command_);
    const ResourceUsageTag begin_tag =
        cb_context->RecordBeginRenderPass(command_, *rp_state_.get(), renderpass_begin_info_.renderArea, attachments_);

    // Note: this state update must be after RecordBeginRenderPass as there is no current render pass until that function runs
    rp_context_ = cb_context->GetCurrentRenderPassContext();

    return begin_tag;
}

bool SyncOpBeginRenderPass::ReplayValidate(ReplayState &replay, ResourceUsageTag recorded_tag) const {
    CommandExecutionContext &exec_context = replay.GetExecutionContext();
    // this operation is not allowed in secondary command buffers
    assert(exec_context.Handle().type == kVulkanObjectTypeQueue);
    auto &batch_context = static_cast<QueueBatchContext &>(exec_context);
    batch_context.BeginRenderPassReplaySetup(replay, *this);

    // Only the layout transitions happen at the replay tag, loadOp's happen at a subsequent tag
    ResourceUsageRange first_use_range = {recorded_tag, recorded_tag + 1};
    return replay.DetectFirstUseHazard(first_use_range);
}

void SyncOpBeginRenderPass::ReplayRecord(CommandExecutionContext &exec_context, ResourceUsageTag exec_tag) const {
    // All the needed replay state changes (for the layout transition, and context update) have to happen in ReplayValidate
}

SyncOpNextSubpass::SyncOpNextSubpass(vvl::Func command, const SyncValidator &sync_state,
                                     const VkSubpassBeginInfo *pSubpassBeginInfo, const VkSubpassEndInfo *pSubpassEndInfo)
    : SyncOpBase(command) {
    if (pSubpassBeginInfo) {
        subpass_begin_info_.initialize(pSubpassBeginInfo);
    }
    if (pSubpassEndInfo) {
        subpass_end_info_.initialize(pSubpassEndInfo);
    }
}

bool SyncOpNextSubpass::Validate(const CommandBufferAccessContext &cb_context) const {
    bool skip = false;
    const auto *renderpass_context = cb_context.GetCurrentRenderPassContext();
    if (!renderpass_context) return skip;

    skip |= renderpass_context->ValidateNextSubpass(cb_context, command_);
    return skip;
}

ResourceUsageTag SyncOpNextSubpass::Record(CommandBufferAccessContext *cb_context) {
    return cb_context->RecordNextSubpass(command_);
}

bool SyncOpNextSubpass::ReplayValidate(ReplayState &replay, ResourceUsageTag recorded_tag) const {
    // Any store/resolve operations happen before the NextSubpass tag so we can advance to the next subpass state
    CommandExecutionContext &exec_context = replay.GetExecutionContext();
    // this operation is not allowed in secondary command buffers
    assert(exec_context.Handle().type == kVulkanObjectTypeQueue);
    auto &batch_context = static_cast<QueueBatchContext &>(exec_context);
    batch_context.NextSubpassReplaySetup(replay);

    // Only the layout transitions happen at the replay tag, loadOp's happen at a subsequent tag
    ResourceUsageRange first_use_range = {recorded_tag, recorded_tag + 1};
    return replay.DetectFirstUseHazard(first_use_range);
}

void SyncOpNextSubpass::ReplayRecord(CommandExecutionContext &exec_context, ResourceUsageTag exec_tag) const {
    // All the needed replay state changes (for the layout transition, and context update) have to happen in ReplayValidate
}
SyncOpEndRenderPass::SyncOpEndRenderPass(vvl::Func command, const SyncValidator &sync_state,
                                         const VkSubpassEndInfo *pSubpassEndInfo)
    : SyncOpBase(command) {
    if (pSubpassEndInfo) {
        subpass_end_info_.initialize(pSubpassEndInfo);
    }
}

bool SyncOpEndRenderPass::Validate(const CommandBufferAccessContext &cb_context) const {
    bool skip = false;
    const auto *renderpass_context = cb_context.GetCurrentRenderPassContext();

    if (!renderpass_context) return skip;
    skip |= renderpass_context->ValidateEndRenderPass(cb_context, command_);
    return skip;
}

ResourceUsageTag SyncOpEndRenderPass::Record(CommandBufferAccessContext *cb_context) {
    return cb_context->RecordEndRenderPass(command_);
}

bool SyncOpEndRenderPass::ReplayValidate(ReplayState &replay, ResourceUsageTag recorded_tag) const {
    // Any store/resolve operations happen before the EndRenderPass tag so we can ignore them
    // Only the layout transitions happen at the replay tag
    ResourceUsageRange first_use_range = {recorded_tag, recorded_tag + 1};
    bool skip = false;
    skip |= replay.DetectFirstUseHazard(first_use_range);

    // We can cleanup here as the recorded tag represents the final layout transition (which is the last operation or the RP)
    CommandExecutionContext &exec_context = replay.GetExecutionContext();
    // this operation is not allowed in secondary command buffers
    assert(exec_context.Handle().type == kVulkanObjectTypeQueue);
    auto &batch_context = static_cast<QueueBatchContext &>(exec_context);
    batch_context.EndRenderPassReplayCleanup(replay);

    return skip;
}

void SyncOpEndRenderPass::ReplayRecord(CommandExecutionContext &exec_context, ResourceUsageTag exec_tag) const {}

ReplayState::ReplayState(CommandExecutionContext &exec_context, const CommandBufferAccessContext &recorded_context,
                         const ErrorObject &error_obj, uint32_t index, ResourceUsageTag base_tag)
    : exec_context_(exec_context), recorded_context_(recorded_context), error_obj_(error_obj), index_(index), base_tag_(base_tag) {}

AccessContext *ReplayState::ReplayStateRenderPassBegin(VkQueueFlags queue_flags, const SyncOpBeginRenderPass &begin_op,
                                                       const AccessContext &external_context) {
    return rp_replay_.Begin(queue_flags, begin_op, external_context);
}

AccessContext *ReplayState::ReplayStateRenderPassNext() { return rp_replay_.Next(); }

void ReplayState::ReplayStateRenderPassEnd(AccessContext &external_context) { rp_replay_.End(external_context); }

const AccessContext *ReplayState::GetRecordedAccessContext() const {
    if (rp_replay_.begin_op) {
        return rp_replay_.replay_context;
    }
    return recorded_context_.GetCurrentAccessContext();
}

bool ReplayState::DetectFirstUseHazard(const ResourceUsageRange &first_use_range) const {
    bool skip = false;
    if (first_use_range.non_empty()) {
        // We're allowing for the Replay(Validate|Record) to modify the exec_context (e.g. for Renderpass operations), so
        // we need to fetch the current access context each time
        const AccessContext *access_context = GetRecordedAccessContext();

        const HazardResult hazard = access_context->DetectFirstUseHazard(exec_context_.GetQueueId(), first_use_range,
                                                                         *exec_context_.GetCurrentAccessContext());
        if (hazard.IsHazard()) {
            const SyncValidator &sync_state = exec_context_.GetSyncState();
            LogObjectList objlist(exec_context_.Handle(), recorded_context_.Handle());
            const std::string error = sync_state.error_messages_.FirstUseError(hazard, exec_context_, recorded_context_, index_);
            skip |= sync_state.SyncError(hazard.Hazard(), objlist, error_obj_.location, error);
        }
    }
    return skip;
}

bool ReplayState::ValidateFirstUse() {
    if (!exec_context_.ValidForSyncOps()) return false;

    bool skip = false;
    ResourceUsageRange first_use_range = {0, 0};

    for (const auto &sync_op : recorded_context_.GetSyncOps()) {
        // Set the range to cover all accesses until the next sync_op, and validate
        first_use_range.end = sync_op.tag;
        skip |= DetectFirstUseHazard(first_use_range);

        // Call to replay validate support for syncop with non-trivial replay
        skip |= sync_op.sync_op->ReplayValidate(*this, sync_op.tag);

        // Record the barrier into the proxy context.
        sync_op.sync_op->ReplayRecord(exec_context_, base_tag_ + sync_op.tag);
        first_use_range.begin = sync_op.tag + 1;
    }

    // and anything after the last syncop
    first_use_range.end = ResourceUsageRecord::kMaxIndex;
    skip |= DetectFirstUseHazard(first_use_range);

    return skip;
}
AccessContext *ReplayState::RenderPassReplayState::Begin(VkQueueFlags queue_flags, const SyncOpBeginRenderPass &begin_op_,
                                                         const AccessContext &external_context) {
    const RenderPassAccessContext *rp_context = begin_op_.GetRenderPassAccessContext();
    assert(rp_context);

    begin_op = &begin_op_;
    replay_context = &rp_context->GetSubpassContexts()[0];
    subpass = 0;
    subpass_contexts = InitSubpassContexts(queue_flags, *rp_context->GetRenderPassState(), &external_context);

    // Replace the Async contexts with the the async context of the "external" context
    // For replay we don't care about async subpasses, just async queue batches
    for (AccessContext &context : GetSubpassContexts()) {
        context.ClearAsyncContexts();
        context.ImportAsyncContexts(external_context);
    }

    return &subpass_contexts[0];
}

AccessContext *ReplayState::RenderPassReplayState::Next() {
    subpass++;

    const RenderPassAccessContext *rp_context = begin_op->GetRenderPassAccessContext();

    replay_context = &rp_context->GetSubpassContexts()[subpass];
    return &subpass_contexts[subpass];
}

void ReplayState::RenderPassReplayState::End(AccessContext &external_context) {
    external_context.ResolveChildContexts(GetSubpassContexts());
    *this = RenderPassReplayState{};
}

vvl::span<AccessContext> ReplayState::RenderPassReplayState::GetSubpassContexts() {
    return vvl::make_span(subpass_contexts.get(),
                          begin_op->GetRenderPassAccessContext()->GetRenderPassState()->create_info.subpassCount);
}

void SyncEventsContext::ApplyBarrier(const SyncExecScope &src, const SyncExecScope &dst, ResourceUsageTag tag) {
    const bool all_commands_bit = 0 != (src.mask_param & VK_PIPELINE_STAGE_ALL_COMMANDS_BIT);
    for (auto &event_pair : map_) {
        assert(event_pair.second);  // Shouldn't be storing empty
        auto &sync_event = *event_pair.second;
        // Events don't happen at a stage, so we need to check and store the unexpanded ALL_COMMANDS if set for inter-event-calls
        // But only if occuring before the tag
        if (((sync_event.barriers & src.exec_scope) || all_commands_bit) && (sync_event.last_command_tag <= tag)) {
            sync_event.barriers |= dst.exec_scope;
            sync_event.barriers |= dst.mask_param & VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;
        }
    }
}

void SyncEventsContext::ApplyTaggedWait(VkQueueFlags queue_flags, ResourceUsageTag tag) {
    const SyncExecScope src_scope =
        SyncExecScope::MakeSrc(queue_flags, VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT, VK_PIPELINE_STAGE_2_HOST_BIT);
    const SyncExecScope dst_scope = SyncExecScope::MakeDst(queue_flags, VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT);
    ApplyBarrier(src_scope, dst_scope, tag);
}

SyncEventsContext &SyncEventsContext::DeepCopy(const SyncEventsContext &from) {
    // We need a deep copy of the const context to update during validation phase
    for (const auto &event : from.map_) {
        map_.emplace(event.first, std::make_shared<SyncEventState>(*event.second));
    }
    return *this;
}

void SyncEventsContext::AddReferencedTags(ResourceUsageTagSet &referenced) const {
    for (const auto &event : map_) {
        const std::shared_ptr<const SyncEventState> &event_state = event.second;
        if (event_state) {
            event_state->AddReferencedTags(referenced);
        }
    }
}

SyncEventState::SyncEventState(const SyncEventState::EventPointer &event_state) : SyncEventState() {
    event = event_state;
    destroyed = (event.get() == nullptr) || event_state->Destroyed();
}

void SyncEventState::ResetFirstScope() {
    first_scope.reset();
    scope = SyncExecScope();
    first_scope_tag = 0;
}

// Keep the "ignore this event" logic in same place for ValidateWait and RecordWait to use
SyncEventState::IgnoreReason SyncEventState::IsIgnoredByWait(vvl::Func command, VkPipelineStageFlags2 srcStageMask) const {
    IgnoreReason reason = NotIgnored;

    if ((vvl::Func::vkCmdWaitEvents2KHR == command || vvl::Func::vkCmdWaitEvents2 == command) &&
        (vvl::Func::vkCmdSetEvent == last_command)) {
        reason = SetVsWait2;
    } else if ((last_command == vvl::Func::vkCmdResetEvent || last_command == vvl::Func::vkCmdResetEvent2KHR) &&
               !HasBarrier(0U, 0U)) {
        reason = (last_command == vvl::Func::vkCmdResetEvent) ? ResetWaitRace : Reset2WaitRace;
    } else if (unsynchronized_set != vvl::Func::Empty) {
        reason = SetRace;
    } else if (first_scope) {
        const VkPipelineStageFlags2 missing_bits = scope.mask_param & ~srcStageMask;
        // Note it is the "not missing bits" path that is the only "NotIgnored" path
        if (missing_bits) reason = MissingStageBits;
    } else {
        reason = MissingSetEvent;
    }

    return reason;
}

bool SyncEventState::HasBarrier(VkPipelineStageFlags2 stageMask, VkPipelineStageFlags2 exec_scope_arg) const {
    return (last_command == vvl::Func::Empty) || (stageMask & VK_PIPELINE_STAGE_ALL_COMMANDS_BIT) || (barriers & exec_scope_arg) ||
           (barriers & VK_PIPELINE_STAGE_ALL_COMMANDS_BIT);
}

void SyncEventState::AddReferencedTags(ResourceUsageTagSet &referenced) const {
    if (first_scope) {
        first_scope->AddReferencedTags(referenced);
    }
}

}  // namespace syncval
