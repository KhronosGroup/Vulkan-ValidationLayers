/* Copyright (c) 2026 The Khronos Group Inc.
 * Copyright (c) 2026 Valve Corporation
 * Copyright (c) 2026 LunarG, Inc.
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

#include "sync_event.h"
#include "sync_access_context.h"
#include "sync_image.h"
#include "sync_validation.h"
#include "state_tracker/buffer_state.h"
#include "utils/image_utils.h"

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

// Need to restrict to only valid exec and access scope for this event
static SyncBarrier RestrictToEvent(const SyncBarrier& barrier, const SyncEventState& sync_event) {
    SyncBarrier result = barrier;
    result.src_exec_scope.exec_scope = sync_event.scope.exec_scope & barrier.src_exec_scope.exec_scope;
    result.src_access_scope = sync_event.scope.stage_mask_accesses & barrier.src_access_scope;
    return result;
}

void ApplyWaitEvents(CommandExecutionContext& exec_context, const std::vector<std::shared_ptr<const vvl::Event>>& events,
                     vvl::span<const BarrierSet> barrier_sets, ResourceUsageTag tag, vvl::Func command) {
    // Unlike PipelineBarrier, WaitEvent is *not* limited to accesses within the current subpass (if any) and thus needs to import
    // all accesses. Can instead import for all first_scopes, or a union of them, if this becomes a performance/memory issue,
    // but with no idea of the performance of the union, nor of whether it even matters... take the simplest approach here,

    AccessContext& access_context = exec_context.GetCurrentAccessContext();
    SyncEventsContext& events_context = exec_context.GetEventsContext();
    const QueueId queue_id = exec_context.GetQueueId();

    access_context.ResolveAllSubpassDependencies();

    assert(barrier_sets.size() == 1 || (barrier_sets.size() == events.size()));

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
    size_t barrier_set_incr = (barrier_sets.size() == 1) ? 0 : 1;
    for (auto& event_shared : events) {
        if (!event_shared) {
            continue;
        }
        auto* sync_event = events_context.GetFromShared(event_shared);
        if (!sync_event->first_scope) {
            continue;  // [core validation check]
        }

        sync_event->last_command = command;
        sync_event->last_command_tag = tag;

        const auto& barrier_set = barrier_sets[barrier_set_index];
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
    barrier_set_incr = (barrier_sets.size() == 1) ? 0 : 1;
    for (auto& event_shared : events) {
        if (!event_shared.get()) {
            continue;
        }
        auto* sync_event = events_context.GetFromShared(event_shared);
        if (!sync_event->first_scope) {
            continue;  // [core validation check]
        }

        const auto& barrier_set = barrier_sets[barrier_set_index];
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
    pending_barriers.Apply(tag);
}

}  // namespace syncval
