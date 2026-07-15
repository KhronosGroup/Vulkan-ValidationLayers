/*
 * Copyright (c) 2019-2026 Valve Corporation
 * Copyright (c) 2019-2026 LunarG, Inc.
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
#include "sync/sync_render_pass.h"
#include "sync/sync_access_context.h"
#include "sync/sync_command_buffer.h"
#include "sync/sync_image.h"
#include "sync/sync_validation.h"

#include "state_tracker/buffer_state.h"
#include "state_tracker/event_state.h"
#include "state_tracker/render_pass_state.h"

#include "utils/image_utils.h"
#include "utils/sync_utils.h"

using vvl::Func;

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
    MapRangesRangeGenerator(const AccessMap& filter, const AccessRange& range)
        : range_(range), map_(&filter), map_pos_(), current_() {
        SeekBegin();
    }
    MapRangesRangeGenerator(const MapRangesRangeGenerator& from) = default;

    const AccessRange& operator*() const { return current_; }
    const AccessRange* operator->() const { return &current_; }
    MapRangesRangeGenerator& operator++() {
        ++map_pos_;
        UpdateCurrent();
        return *this;
    }

    bool operator==(const MapRangesRangeGenerator& other) const { return current_ == other.current_; }

  protected:
    void UpdateCurrent() {
        if (map_pos_ != map_->end()) {
            current_ = range_ & map_pos_->first;
        } else {
            current_ = {};
        }
    }
    void SeekBegin() {
        map_pos_ = map_->LowerBound(range_.begin);
        UpdateCurrent();
    }

    // Adding this functionality here, to avoid gratuitous Base:: qualifiers in the derived class
    // Note: Not exposed in this classes public interface to encourage using a consistent ++/empty generator semantic
    template <typename Pred>
    MapRangesRangeGenerator& PredicatedIncrement(Pred& pred) {
        do {
            ++map_pos_;
        } while (map_pos_ != map_->end() && map_pos_->first.intersects(range_) && !pred(map_pos_));
        UpdateCurrent();
        return *this;
    }

    const AccessRange range_;
    const AccessMap* map_ = nullptr;
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
    FilteredGeneratorGenerator(const AccessMap& filter, RangeGen& gen) : filter_(&filter), gen_(gen), filter_pos_(), current_() {
        SeekBegin();
    }
    FilteredGeneratorGenerator(const FilteredGeneratorGenerator& from) = default;
    const AccessRange& operator*() const { return current_; }
    const AccessRange* operator->() const { return &current_; }
    FilteredGeneratorGenerator& operator++() {
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

    bool operator==(const FilteredGeneratorGenerator& other) const { return current_ == other.current_; }

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

    AccessRange FastForwardFilter(const AccessRange& range) {
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
                filter_pos_ = filter_->LowerBound(range.begin);
                break;
            }
        }
        return FilterRange();
    }

    // TODO: Consider adding "seek" (or an absolute bound "get" to range generators to make this walk
    // faster.
    AccessRange FastForwardGen(const AccessRange& range) {
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
            filter_pos_ = filter_->LowerBound(gen_range.begin);
            current_ = gen_range & FilterRange();
        }
    }

    const AccessMap* filter_ = nullptr;
    RangeGen gen_;
    AccessMap::const_iterator filter_pos_;
    AccessRange current_;
};

using EventImageRangeGenerator = FilteredGeneratorGenerator<subresource_adapter::ImageRangeGenerator>;

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

SyncOpPipelineBarrier::SyncOpPipelineBarrier(BarrierSet&& barrier_set) : barrier_set_(std::move(barrier_set)) {}

void SyncOpPipelineBarrier::ReplayRecord(CommandExecutionContext& exec_context, const ResourceUsageTag exec_tag) const {
    const SyncValidator& validator = exec_context.GetSyncState();
    validator.ApplyBarrier(exec_context, barrier_set_, exec_tag);
}

bool SyncOpPipelineBarrier::ReplayValidate(ReplayState& replay, ResourceUsageTag recorded_tag) const {
    // The layout transitions happen at the replay tag
    ResourceUsageRange first_use_range = {recorded_tag, recorded_tag + 1};
    return replay.DetectFirstUseHazard(first_use_range);
}

SyncOpWaitEvents::SyncOpWaitEvents(vvl::Func command, const SyncValidator& sync_state, VkQueueFlags queue_flags,
                                   uint32_t eventCount, const VkEvent* pEvents, VkPipelineStageFlags srcStageMask,
                                   VkPipelineStageFlags dstStageMask, uint32_t memoryBarrierCount,
                                   const VkMemoryBarrier* pMemoryBarriers, uint32_t bufferMemoryBarrierCount,
                                   const VkBufferMemoryBarrier* pBufferMemoryBarriers, uint32_t imageMemoryBarrierCount,
                                   const VkImageMemoryBarrier* pImageMemoryBarriers)
    : SyncOpBase(command), barrier_sets_(1) {
    const SyncExecScope src_exec_scope = SyncExecScope::MakeSrc(queue_flags, srcStageMask);
    const SyncExecScope dst_exec_scope = SyncExecScope::MakeDst(queue_flags, dstStageMask);
    barrier_sets_[0] = BarrierSet(sync_state, src_exec_scope, dst_exec_scope, memoryBarrierCount, pMemoryBarriers,
                                  bufferMemoryBarrierCount, pBufferMemoryBarriers, imageMemoryBarrierCount, pImageMemoryBarriers);
    MakeEventsList(sync_state, eventCount, pEvents);
}

SyncOpWaitEvents::SyncOpWaitEvents(vvl::Func command, const SyncValidator& sync_state, VkQueueFlags queue_flags,
                                   uint32_t eventCount, const VkEvent* pEvents, const VkDependencyInfo* pDependencyInfo)
    : SyncOpBase(command), barrier_sets_(eventCount) {
    for (uint32_t i = 0; i < eventCount; i++) {
        barrier_sets_[i] = BarrierSet(sync_state, queue_flags, pDependencyInfo[i]);
    }
    MakeEventsList(sync_state, eventCount, pEvents);
}

ResourceUsageTag SyncOpWaitEvents::Record(CommandBufferAccessContext& cb_context) {
    const ResourceUsageTag tag = cb_context.NextCommandTag(command_);
    ReplayRecord(cb_context, tag);
    return tag;
}

// Need to restrict to only valid exec and access scope for this event
static SyncBarrier RestrictToEvent(const SyncBarrier& barrier, const SyncEventState& sync_event) {
    SyncBarrier result = barrier;
    result.src_exec_scope.exec_scope = sync_event.scope.exec_scope & barrier.src_exec_scope.exec_scope;
    result.src_access_scope = sync_event.scope.stage_mask_accesses & barrier.src_access_scope;
    return result;
}

void SyncOpWaitEvents::ReplayRecord(CommandExecutionContext& exec_context, ResourceUsageTag exec_tag) const {
    // Unlike PipelineBarrier, WaitEvent is *not* limited to accesses within the current subpass (if any) and thus needs to import
    // all accesses. Can instead import for all first_scopes, or a union of them, if this becomes a performance/memory issue,
    // but with no idea of the performance of the union, nor of whether it even matters... take the simplest approach here,

    AccessContext& access_context = exec_context.GetCurrentAccessContext();
    SyncEventsContext& events_context = exec_context.GetEventsContext();
    const QueueId queue_id = exec_context.GetQueueId();

    access_context.ResolveAllSubpassDependencies();

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
    for (auto& event_shared : events_) {
        if (!event_shared) {
            continue;
        }
        auto* sync_event = events_context.GetFromShared(event_shared);
        if (!sync_event->first_scope) {
            continue;  // [core validation check]
        }

        sync_event->last_command = command_;
        sync_event->last_command_tag = exec_tag;

        const auto& barrier_set = barrier_sets_[barrier_set_index];
        for (const SyncBufferBarrier& barrier : barrier_set.buffer_barriers) {
            if (SimpleBinding(*barrier.buffer)) {
                const VkDeviceSize base_address = ResourceBaseAddress(*barrier.buffer);
                const AccessRange range = barrier.range + base_address;
                EventSimpleRangeGenerator filtered_range_gen(sync_event->FirstScope(), range);
                ApplyMarkupFunctor markup_action(false);
                access_context.UpdateMemoryAccessState(markup_action, filtered_range_gen);
            }
        }
        for (const SyncImageBarrier& barrier : barrier_set.image_barriers) {
            const auto& sub_state = SubState(*barrier.image);
            const bool can_transition_depth_slices = CanTransitionDepthSlices(
                exec_context.GetSyncState().extensions, sub_state.base.GetImageType(), sub_state.base.create_flags);
            ImageRangeGen range_gen = sub_state.MakeImageRangeGen(barrier.subresource_range, can_transition_depth_slices);
            EventImageRangeGenerator filtered_range_gen(sync_event->FirstScope(), range_gen);
            ApplyMarkupFunctor markup_action(barrier.layout_transition);
            access_context.UpdateMemoryAccessState(markup_action, filtered_range_gen);
        }
        auto global_barriers_range_gen = EventSimpleRangeGenerator(sync_event->FirstScope(), kFullRange);
        ApplyMarkupFunctor markup_action(false);
        access_context.UpdateMemoryAccessState(markup_action, global_barriers_range_gen);
        barrier_set_index += barrier_set_incr;
    }

    // Apply barriers independently and store the result in the pending object.
    PendingBarriers pending_barriers;
    barrier_set_index = 0;
    barrier_set_incr = (barrier_sets_.size() == 1) ? 0 : 1;
    for (auto& event_shared : events_) {
        if (!event_shared.get()) {
            continue;
        }
        auto* sync_event = events_context.GetFromShared(event_shared);
        if (!sync_event->first_scope) {
            continue;  // [core validation check]
        }

        const auto& barrier_set = barrier_sets_[barrier_set_index];
        const auto& dst = barrier_set.dst_exec_scope;

        // These apply barriers one at a time as the are restricted to the resource ranges specified per each barrier,
        // but do not update the dependency chain information (but set the "pending" state) // s.t. the order independence
        // of the barriers is maintained.

        for (const SyncBufferBarrier& barrier : barrier_set.buffer_barriers) {
            if (SimpleBinding(*barrier.buffer)) {
                const SyncBarrier event_barrier = RestrictToEvent(barrier.barrier, *sync_event);
                const BarrierScope barrier_scope(event_barrier, queue_id, sync_event->first_scope_tag);
                CollectBarriersFunctor collect_barriers(access_context, barrier_scope, event_barrier, false, vvl::kNoIndex32,
                                                        pending_barriers);

                const VkDeviceSize base_address = ResourceBaseAddress(*barrier.buffer);
                const AccessRange range = barrier.range + base_address;
                EventSimpleRangeGenerator range_gen(sync_event->FirstScope(), range);

                access_context.UpdateMemoryAccessState(collect_barriers, range_gen);
            }
        }
        for (const SyncImageBarrier& barrier : barrier_set.image_barriers) {
            const SyncBarrier event_barrier = RestrictToEvent(barrier.barrier, *sync_event);
            const BarrierScope barrier_scope(event_barrier, queue_id, sync_event->first_scope_tag);
            CollectBarriersFunctor collect_barriers(access_context, barrier_scope, event_barrier, barrier.layout_transition,
                                                    barrier.handle_index, pending_barriers);

            const auto& sub_state = SubState(*barrier.image);
            const bool can_transition_depth_slices = CanTransitionDepthSlices(
                exec_context.GetSyncState().extensions, sub_state.base.GetImageType(), sub_state.base.create_flags);
            ImageRangeGen range_gen = sub_state.MakeImageRangeGen(barrier.subresource_range, can_transition_depth_slices);
            EventImageRangeGenerator filtered_range_gen(sync_event->FirstScope(), range_gen);

            access_context.UpdateMemoryAccessState(collect_barriers, filtered_range_gen);
        }
        // TODO: because each iteration applies functor to the same range, investigate if it is
        // beneficial for the functor to support multiple barriers, so we traverse access map once.
        auto global_range_gen = EventSimpleRangeGenerator(sync_event->FirstScope(), kFullRange);
        for (const auto& barrier : barrier_set.memory_barriers) {
            const SyncBarrier event_barrier = RestrictToEvent(barrier, *sync_event);
            const BarrierScope barrier_scope(event_barrier, queue_id, sync_event->first_scope_tag);
            CollectBarriersFunctor collect_barriers(access_context, barrier_scope, event_barrier, false, vvl::kNoIndex32,
                                                    pending_barriers);

            auto range_gen = global_range_gen;  // intentional copy
            access_context.UpdateMemoryAccessState(collect_barriers, range_gen);
        }

        // Apply the global barrier to the event itself (for race condition tracking)
        // Events don't happen at a stage, so we need to store the unexpanded ALL_COMMANDS if set for inter-event-calls
        sync_event->barriers = dst.stage_mask & VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;
        sync_event->barriers |= dst.exec_scope;

        barrier_set_index += barrier_set_incr;
    }

    // Update access states with collected barriers
    pending_barriers.Apply(exec_tag);
}

bool SyncOpWaitEvents::ReplayValidate(ReplayState& replay, ResourceUsageTag recorded_tag) const {
    return replay.GetExecutionContext().GetSyncState().ValidateCmdWaitEvents(
        replay.GetExecutionContext(), events_, barrier_sets_, replay.GetBaseTag() + recorded_tag, Location(command_));
}

void SyncOpWaitEvents::MakeEventsList(const SyncValidator& sync_state, uint32_t event_count, const VkEvent* events) {
    events_.reserve(event_count);
    for (uint32_t event_index = 0; event_index < event_count; event_index++) {
        events_.emplace_back(sync_state.Get<vvl::Event>(events[event_index]));
    }
}

SyncOpResetEvent::SyncOpResetEvent(vvl::Func command, const SyncValidator& sync_state, VkQueueFlags queue_flags, VkEvent event,
                                   VkPipelineStageFlags2 stageMask)
    : SyncOpBase(command), event_(sync_state.Get<vvl::Event>(event)), exec_scope_(SyncExecScope::MakeSrc(queue_flags, stageMask)) {}


ResourceUsageTag SyncOpResetEvent::Record(CommandBufferAccessContext& cb_context) {
    const ResourceUsageTag tag = cb_context.NextCommandTag(command_);
    ReplayRecord(cb_context, tag);
    return tag;
}

bool SyncOpResetEvent::ReplayValidate(ReplayState& replay, ResourceUsageTag recorded_tag) const {
    return replay.GetExecutionContext().GetSyncState().ValidateCmdResetEvent(
        replay.GetExecutionContext(), event_, exec_scope_, replay.GetBaseTag() + recorded_tag, Location(command_));
}

void SyncOpResetEvent::ReplayRecord(CommandExecutionContext& exec_context, ResourceUsageTag exec_tag) const {
    SyncEventsContext& events_context = exec_context.GetEventsContext();

    auto* sync_event = events_context.GetFromShared(event_);
    if (!sync_event) {
        return;
    }

    // Update the event state
    sync_event->last_command = command_;
    sync_event->last_command_tag = exec_tag;
    sync_event->unsynchronized_set = vvl::Func::Empty;
    sync_event->ResetFirstScope();
    sync_event->barriers = 0U;
}

SyncOpSetEvent::SyncOpSetEvent(vvl::Func command, const SyncValidator& sync_state, VkQueueFlags queue_flags, VkEvent event,
                               VkPipelineStageFlags2 stageMask, const AccessContext* access_context)
    : SyncOpBase(command),
      event_(sync_state.Get<vvl::Event>(event)),
      src_exec_scope_(SyncExecScope::MakeSrc(queue_flags, stageMask)) {
    // Snapshot the current access_context for later inspection at wait time.
    // NOTE: This appears brute force, but given that we only save a "first-last" model of access history, the current
    //       access context (include barrier state for chaining) won't necessarily contain the needed information at Wait
    //       or Submit time reference.
    if (access_context) {
        auto new_context = std::make_shared<AccessContext>(sync_state);
        new_context->InitFrom(*access_context);
        recorded_context_ = new_context;
    }
}

SyncOpSetEvent::SyncOpSetEvent(vvl::Func command, const SyncValidator& sync_state, VkQueueFlags queue_flags, VkEvent event,
                               const VkDependencyInfo& dep_info, const AccessContext* access_context)
    : SyncOpBase(command),
      event_(sync_state.Get<vvl::Event>(event)),
      src_exec_scope_(SyncExecScope::MakeSrc(queue_flags, sync_utils::GetExecScopes(dep_info).src)) {
    if (access_context) {
        auto new_context = std::make_shared<AccessContext>(sync_state);
        new_context->InitFrom(*access_context);
        recorded_context_ = new_context;
    }
}

bool SyncOpSetEvent::ReplayValidate(ReplayState& replay, ResourceUsageTag recorded_tag) const {
    return replay.GetExecutionContext().GetSyncState().ValidateCmdSetEvent(replay.GetExecutionContext(), event_, src_exec_scope_,
                                                                           replay.GetBaseTag() + recorded_tag, Location(command_));
}

ResourceUsageTag SyncOpSetEvent::Record(CommandBufferAccessContext& cb_context) {
    const ResourceUsageTag tag = cb_context.NextCommandTag(command_);
    SyncEventsContext& events_context = cb_context.GetEventsContext();
    const QueueId queue_id = cb_context.GetQueueId();
    assert(recorded_context_);
    if (recorded_context_) {
        DoRecord(queue_id, tag, recorded_context_, events_context);
    }
    return tag;
}

void SyncOpSetEvent::ReplayRecord(CommandExecutionContext& exec_context, ResourceUsageTag exec_tag) const {
    // Create a copy of the current context, and merge in the state snapshot at record set event time
    // Note: we mustn't change the recorded context copy, as a given CB could be submitted more than once (in generaL)

    SyncEventsContext& events_context = exec_context.GetEventsContext();
    AccessContext& access_context = exec_context.GetCurrentAccessContext();
    const QueueId queue_id = exec_context.GetQueueId();

    // Note: merged_context is a copy of the access_context, combined with the recorded context
    auto merged_context = std::make_shared<AccessContext>(*access_context.validator);
    merged_context->InitFrom(access_context);
    merged_context->ResolveFromContext(QueueTagOffsetBarrierAction(queue_id, exec_tag), *recorded_context_);
    merged_context->TrimAndClearFirstAccess();  // Ensure the copy is minimal and normalized
    DoRecord(queue_id, exec_tag, merged_context, events_context);
}

void SyncOpSetEvent::DoRecord(QueueId queue_id, ResourceUsageTag tag, const std::shared_ptr<const AccessContext>& access_context,
                              SyncEventsContext& events_context) const {
    auto* sync_event = events_context.GetFromShared(event_);
    if (!sync_event) {
        return;
    }

    // What happens with two SetEvent is that one cannot know what group of operations will be waited for.
    // Given:
    //     Stuff1; SetEvent; Stuff2; SetEvent; WaitEvents;
    // WaitEvents cannot know which of Stuff1, Stuff2, or both has completed execution.

    if (!sync_event->HasBarrier(src_exec_scope_.stage_mask, src_exec_scope_.exec_scope)) {
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
    sync_event->last_command = command_;
    sync_event->last_command_tag = tag;
    sync_event->barriers = 0U;
}

SyncOpBeginRenderPass::SyncOpBeginRenderPass(vvl::Func command, const SyncValidator& sync_state,
                                             const VkRenderPassBeginInfo& render_pass_begin_info,
                                             const VkSubpassBeginInfo* p_subpass_begin_info)
    : SyncOpBase(command), rp_context_(nullptr) {
    rp_state_ = sync_state.Get<vvl::RenderPass>(render_pass_begin_info.renderPass);
    renderpass_begin_info_ = vku::safe_VkRenderPassBeginInfo(&render_pass_begin_info);
    auto fb_state = sync_state.Get<vvl::Framebuffer>(render_pass_begin_info.framebuffer);
    if (fb_state) {
        attachments_ = sync_state.device_state->GetAttachmentViews(*renderpass_begin_info_.ptr(), *fb_state);
    }
    if (p_subpass_begin_info) {
        subpass_begin_info_ = vku::safe_VkSubpassBeginInfo(p_subpass_begin_info);
    }
}

ResourceUsageTag SyncOpBeginRenderPass::Record(CommandBufferAccessContext& cb_context) {
    if (!rp_state_) {
        return cb_context.NextCommandTag(command_);
    }
    const ResourceUsageTag begin_tag =
        cb_context.RecordBeginRenderPass(command_, *rp_state_.get(), renderpass_begin_info_.renderArea, attachments_);

    // Note: this state update must be after RecordBeginRenderPass as there is no current render pass until that function runs
    rp_context_ = cb_context.GetCurrentRenderPassContext();

    return begin_tag;
}

bool SyncOpBeginRenderPass::ReplayValidate(ReplayState& replay, ResourceUsageTag recorded_tag) const {
    CommandExecutionContext& exec_context = replay.GetExecutionContext();
    // this operation is not allowed in secondary command buffers
    assert(exec_context.Handle().type == kVulkanObjectTypeQueue);
    auto& batch_context = static_cast<QueueBatchContext&>(exec_context);
    batch_context.BeginRenderPassReplaySetup(replay, *this);

    // Only the layout transitions happen at the replay tag, loadOp's happen at a subsequent tag
    ResourceUsageRange first_use_range = {recorded_tag, recorded_tag + 1};
    return replay.DetectFirstUseHazard(first_use_range);
}

void SyncOpBeginRenderPass::ReplayRecord(CommandExecutionContext& exec_context, ResourceUsageTag exec_tag) const {
    // All the needed replay state changes (for the layout transition, and context update) have to happen in ReplayValidate
}

SyncOpNextSubpass::SyncOpNextSubpass(vvl::Func command, const SyncValidator& sync_state,
                                     const VkSubpassBeginInfo* pSubpassBeginInfo, const VkSubpassEndInfo* pSubpassEndInfo)
    : SyncOpBase(command) {}

ResourceUsageTag SyncOpNextSubpass::Record(CommandBufferAccessContext& cb_context) {
    return cb_context.RecordNextSubpass(command_);
}

bool SyncOpNextSubpass::ReplayValidate(ReplayState& replay, ResourceUsageTag recorded_tag) const {
    // Any store/resolve operations happen before the NextSubpass tag so we can advance to the next subpass state
    CommandExecutionContext& exec_context = replay.GetExecutionContext();
    // this operation is not allowed in secondary command buffers
    assert(exec_context.Handle().type == kVulkanObjectTypeQueue);
    auto& batch_context = static_cast<QueueBatchContext&>(exec_context);
    batch_context.NextSubpassReplaySetup(replay);

    // Only the layout transitions happen at the replay tag, loadOp's happen at a subsequent tag
    ResourceUsageRange first_use_range = {recorded_tag, recorded_tag + 1};
    return replay.DetectFirstUseHazard(first_use_range);
}

void SyncOpNextSubpass::ReplayRecord(CommandExecutionContext& exec_context, ResourceUsageTag exec_tag) const {
    // All the needed replay state changes (for the layout transition, and context update) have to happen in ReplayValidate
}

SyncOpEndRenderPass::SyncOpEndRenderPass(vvl::Func command, const SyncValidator& sync_state,
                                         const VkSubpassEndInfo* pSubpassEndInfo)
    : SyncOpBase(command) {}

ResourceUsageTag SyncOpEndRenderPass::Record(CommandBufferAccessContext& cb_context) {
    return cb_context.RecordEndRenderPass(command_);
}

bool SyncOpEndRenderPass::ReplayValidate(ReplayState& replay, ResourceUsageTag recorded_tag) const {
    // The record_tag is the final layout transition. Any store/resolve operations happen before
    // the EndRenderPass tag so we can ignore them here.
    //
    // The final layout transition is recorded in command buffer context (not render pass context).
    // Do a render pass cleanup. This also switches replay to command buffer context where we can
    // validate layout transition.
    CommandExecutionContext& exec_context = replay.GetExecutionContext();
    assert(exec_context.Handle().type == kVulkanObjectTypeQueue);  // not allowed in secondary command buffers
    auto& batch_context = static_cast<QueueBatchContext&>(exec_context);
    batch_context.EndRenderPassReplayCleanup(replay);

    // Validate final layout transition
    ResourceUsageRange first_use_range = {recorded_tag, recorded_tag + 1};
    bool skip = false;
    skip |= replay.DetectFirstUseHazard(first_use_range);

    return skip;
}

void SyncOpEndRenderPass::ReplayRecord(CommandExecutionContext& exec_context, ResourceUsageTag exec_tag) const {}

ReplayState::ReplayState(CommandExecutionContext& exec_context, const CommandBufferAccessContext& recorded_context,
                         const ErrorObject& error_obj, uint32_t index, ResourceUsageTag base_tag)
    : exec_context_(exec_context), recorded_context_(recorded_context), error_obj_(error_obj), index_(index), base_tag_(base_tag) {}

AccessContext* ReplayState::ReplayStateRenderPassBegin(VkQueueFlags queue_flags, const SyncOpBeginRenderPass& begin_op,
                                                       const AccessContext& external_context) {
    return rp_replay_.Begin(queue_flags, begin_op, external_context);
}

AccessContext* ReplayState::ReplayStateRenderPassNext() { return rp_replay_.Next(); }

void ReplayState::ReplayStateRenderPassEnd(AccessContext& external_context) { rp_replay_.End(external_context); }

const AccessContext* ReplayState::GetRecordedAccessContext() const {
    if (rp_replay_.begin_op) {
        return rp_replay_.replay_context;
    }
    return &recorded_context_.GetCurrentAccessContext();
}

bool ReplayState::DetectFirstUseHazard(const ResourceUsageRange& first_use_range) const {
    bool skip = false;
    if (first_use_range.non_empty()) {
        // We're allowing for the Replay(Validate|Record) to modify the exec_context (e.g. for Renderpass operations), so
        // we need to fetch the current access context each time
        const AccessContext* access_context = GetRecordedAccessContext();

        const HazardResult hazard = access_context->DetectFirstUseHazard(exec_context_.GetQueueId(), first_use_range,
                                                                         exec_context_.GetCurrentAccessContext());
        if (hazard.IsHazard()) {
            const SyncValidator& sync_state = exec_context_.GetSyncState();
            LogObjectList objlist(exec_context_.Handle(), recorded_context_.Handle());
            const std::string error = sync_state.error_messages_.FirstUseError(hazard, exec_context_, recorded_context_, index_);
            skip |= sync_state.SyncError(hazard.Hazard(), objlist, error_obj_.location, error);
        }
    }
    return skip;
}

// Validate first-use hazards. The following describes how it works.
//
// The first access to a memory location can occur anywhere in the command buffer
// (not necessarily at the beginning), and first accesses to different resources
// may be interleaved with barriers. To validate each first access against accesses
// from previous submissions, we need to replay all barriers that occur before that
// specific first access.
//
// This defines the algorithm: replay barriers until we reach the next first access,
// validate that first access, then continue replaying barriers until the next first
// access, validate that one, and so on until we reach the end of the command buffer.
bool ReplayState::ValidateFirstUse() {
    bool skip = false;
    ResourceUsageRange first_use_range = {0, 0};

    for (const auto& sync_op : recorded_context_.GetSyncOps()) {
        // Validate all first accesses until the next sync_op
        first_use_range.end = sync_op.tag;
        skip |= DetectFirstUseHazard(first_use_range);

        // Validate and record sync_ops that make memory accesses (for example, image layout transition)
        skip |= sync_op.sync_op->ReplayValidate(*this, sync_op.tag);
        sync_op.sync_op->ReplayRecord(exec_context_, base_tag_ + sync_op.tag);

        // Advance past sync_op
        first_use_range.begin = sync_op.tag + 1;
    }

    // Validate first accesses after the last syncop
    first_use_range.end = ResourceUsageRecord::kMaxIndex;
    skip |= DetectFirstUseHazard(first_use_range);
    return skip;
}

AccessContext* ReplayState::RenderPassReplayState::Begin(VkQueueFlags queue_flags, const SyncOpBeginRenderPass& begin_op_,
                                                         const AccessContext& external_context) {
    const RenderPassAccessContext* rp_context = begin_op_.GetRenderPassAccessContext();
    assert(rp_context);

    begin_op = &begin_op_;
    replay_context = &rp_context->GetSubpassContexts()[0];
    subpass = 0;
    subpass_contexts = InitSubpassContexts(queue_flags, *rp_context->GetRenderPassState(), external_context);

    // Replace the Async contexts with the the async context of the "external" context
    // For replay we don't care about async subpasses, just async queue batches
    for (AccessContext& context : GetSubpassContexts()) {
        context.ClearAsyncContexts();
        context.ImportAsyncContexts(external_context);
    }

    return &subpass_contexts[0];
}

AccessContext* ReplayState::RenderPassReplayState::Next() {
    subpass++;

    const RenderPassAccessContext* rp_context = begin_op->GetRenderPassAccessContext();

    replay_context = &rp_context->GetSubpassContexts()[subpass];
    return &subpass_contexts[subpass];
}

void ReplayState::RenderPassReplayState::End(AccessContext& external_context) {
    external_context.ResolveChildContexts(GetSubpassContexts());
    *this = RenderPassReplayState{};
}

vvl::span<AccessContext> ReplayState::RenderPassReplayState::GetSubpassContexts() {
    return vvl::make_span(subpass_contexts.get(),
                          begin_op->GetRenderPassAccessContext()->GetRenderPassState()->create_info.subpassCount);
}

void SyncEventsContext::ApplyBarrier(const SyncExecScope& src, const SyncExecScope& dst, ResourceUsageTag tag) {
    const bool all_commands_bit = 0 != (src.stage_mask & VK_PIPELINE_STAGE_ALL_COMMANDS_BIT);
    for (auto& event_pair : map_) {
        assert(event_pair.second);  // Shouldn't be storing empty
        auto& sync_event = *event_pair.second;
        // Events don't happen at a stage, so we need to check and store the unexpanded ALL_COMMANDS if set for inter-event-calls
        // But only if occuring before the tag
        if (((sync_event.barriers & src.exec_scope) || all_commands_bit) && (sync_event.last_command_tag <= tag)) {
            sync_event.barriers |= dst.exec_scope;
            sync_event.barriers |= dst.stage_mask & VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;
        }
    }
}

void SyncEventsContext::ApplyTaggedWait(VkQueueFlags queue_flags, ResourceUsageTag tag) {
    const SyncExecScope src_scope =
        SyncExecScope::MakeSrc(queue_flags, VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT, VK_PIPELINE_STAGE_2_HOST_BIT);
    const SyncExecScope dst_scope = SyncExecScope::MakeDst(queue_flags, VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT);
    ApplyBarrier(src_scope, dst_scope, tag);
}

SyncEventsContext& SyncEventsContext::DeepCopy(const SyncEventsContext& from) {
    // We need a deep copy of the const context to update during validation phase
    for (const auto& event : from.map_) {
        map_.emplace(event.first, std::make_shared<SyncEventState>(*event.second));
    }
    return *this;
}

void SyncEventsContext::AddReferencedTags(ResourceUsageTagSet& referenced) const {
    for (const auto& event : map_) {
        const std::shared_ptr<const SyncEventState>& event_state = event.second;
        if (event_state) {
            event_state->AddReferencedTags(referenced);
        }
    }
}

SyncEventState::SyncEventState(const SyncEventState::EventPointer& event_state) : SyncEventState() {
    event = event_state;
}

void SyncEventState::ResetFirstScope() {
    first_scope.reset();
    scope = SyncExecScope();
    first_scope_tag = 0;
}

bool SyncEventState::HasBarrier(VkPipelineStageFlags2 stageMask, VkPipelineStageFlags2 exec_scope_arg) const {
    return (last_command == vvl::Func::Empty) || (stageMask & VK_PIPELINE_STAGE_ALL_COMMANDS_BIT) || (barriers & exec_scope_arg) ||
           (barriers & VK_PIPELINE_STAGE_ALL_COMMANDS_BIT);
}

void SyncEventState::AddReferencedTags(ResourceUsageTagSet& referenced) const {
    if (first_scope) {
        first_scope->AddReferencedTags(referenced);
    }
}

}  // namespace syncval
