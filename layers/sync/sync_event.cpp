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

bool ValidateCmdSetEvent(const SyncEnvironment& env, const std::shared_ptr<const vvl::Event>& event,
                         const SyncExecScope& src_exec_scope, ResourceUsageTag base_tag, const Location& loc) {
    bool skip = false;

    const auto* sync_event = env.events_context.Get(event);
    if (!sync_event) {
        return skip;
    }
    if (sync_event->last_command_tag >= base_tag) {
        return skip;  // for replay we don't want to revalidate internal "last commmand"
    }
    if (!sync_event->HasBarrier(src_exec_scope.stage_mask, src_exec_scope.exec_scope)) {
        const std::string vuid_prefix = std::string("SYNC-") + vvl::String(loc.function);
        if (IsValueIn(sync_event->last_command,
                      {vvl::Func::vkCmdResetEvent, vvl::Func::vkCmdResetEvent2, vvl::Func::vkCmdResetEvent2KHR})) {
            skip |=
                env.validator.LogError(vuid_prefix + "-reset-race", event->Handle(), loc,
                                       "%s is set after %s without an intervening execution dependency. This is a race condition "
                                       "and may result in data hazards.",
                                       env.validator.FormatHandle(event->Handle()).c_str(), vvl::String(sync_event->last_command));
        } else if (IsValueIn(sync_event->last_command,
                             {vvl::Func::vkCmdSetEvent, vvl::Func::vkCmdSetEvent2, vvl::Func::vkCmdSetEvent2KHR})) {
            skip |=
                env.validator.LogError(vuid_prefix + "-set-race", event->Handle(), loc,
                                       "%s is set after a previous %s without an intervening execution dependency. This is a race "
                                       "condition and may result in data hazards.",
                                       env.validator.FormatHandle(event->Handle()).c_str(), vvl::String(sync_event->last_command));
        } else if (IsValueIn(sync_event->last_command,
                             {vvl::Func::vkCmdWaitEvents, vvl::Func::vkCmdWaitEvents2, vvl::Func::vkCmdWaitEvents2KHR})) {
            skip |=
                env.validator.LogError(vuid_prefix + "-wait", event->Handle(), loc,
                                       "%s is set after %s without intervening vkCmdResetEvent, may result in data hazard.",
                                       env.validator.FormatHandle(event->Handle()).c_str(), vvl::String(sync_event->last_command));
        }
    }
    return skip;
}

bool ValidateCmdResetEvent(const SyncEnvironment& env, const std::shared_ptr<const vvl::Event>& event,
                           const SyncExecScope& exec_scope, const ResourceUsageTag base_tag, const Location& loc) {
    bool skip = false;
    const auto* sync_event = env.events_context.Get(event);
    if (!sync_event) {
        return skip;
    }
    if (sync_event->last_command_tag > base_tag) {
        return skip;  // if we validated this in recording of the secondary, don't repeat
    }
    if (IsValueIn(sync_event->last_command, {vvl::Func::vkCmdSetEvent, vvl::Func::vkCmdSetEvent2, vvl::Func::vkCmdSetEvent2KHR}) &&
        !sync_event->HasBarrier(exec_scope.stage_mask, exec_scope.exec_scope)) {
        skip |= env.validator.LogError("SYNC-vkCmdResetEvent-set-race", event->Handle(), loc,
                                       "%s is reset after %s without an intervening execution dependency. This is a race condition "
                                       "and may result in data hazards.",
                                       env.validator.FormatHandle(event->Handle()).c_str(), vvl::String(sync_event->last_command));
    }
    return skip;
}

bool ValidateCmdWaitEvents(const SyncEnvironment& env, const AccessContext& access_context,
                           const std::vector<std::shared_ptr<const vvl::Event>>& events,
                           const vvl::span<const BarrierSet>& barrier_sets, const ResourceUsageTag base_tag, const Location& loc) {
    bool skip = false;

    // This is only interesting at record and not replay (Execute/Submit) time
    if (base_tag == ResourceUsageRecord::kMaxIndex) {
        for (size_t barrier_set_index = 0; barrier_set_index < barrier_sets.size(); barrier_set_index++) {
            const auto& barrier_set = barrier_sets[barrier_set_index];
            if (barrier_set.single_exec_scope) {
                if (barrier_set.src_exec_scope.stage_mask & VK_PIPELINE_STAGE_HOST_BIT) {
                    const std::string vuid =
                        std::string("SYNC-") + vvl::String(loc.function) + std::string("-hostevent-unsupported");
                    env.validator.LogInfo(vuid, env.handle, loc,
                                          "srcStageMask includes %s, unsupported by synchronization validation.",
                                          string_VkPipelineStageFlagBits(VK_PIPELINE_STAGE_HOST_BIT));
                } else {
                    const auto& barriers = barrier_set.memory_barriers;
                    for (size_t barrier_index = 0; barrier_index < barriers.size(); barrier_index++) {
                        const auto& barrier = barriers[barrier_index];
                        if (barrier.src_exec_scope.stage_mask & VK_PIPELINE_STAGE_HOST_BIT) {
                            const std::string vuid =
                                std::string("SYNC-") + vvl::String(loc.function) + std::string("-hostevent-unsupported");

                            env.validator.LogInfo(vuid, env.handle, loc,
                                                  "srcStageMask %s of %s %zu, %s %zu, unsupported by synchronization validation.",
                                                  string_VkPipelineStageFlagBits(VK_PIPELINE_STAGE_HOST_BIT), "pDependencyInfo",
                                                  barrier_set_index, "pMemoryBarriers", barrier_index);
                        }
                    }
                }
            }
        }
    }

    // The rest is common to record time and replay time
    size_t barrier_set_index = 0;
    size_t barrier_set_incr = (barrier_sets.size() == 1) ? 0 : 1;
    for (const auto& event : events) {
        const auto* sync_event = env.events_context.Get(event);
        const auto& barrier_set = barrier_sets[barrier_set_index];
        if (!sync_event || !sync_event->first_scope) {
            barrier_set_index += barrier_set_incr;
            continue;  // Core, Lifetimes, or Param check needs to catch invalid events.
        }

        // For replay calls, don't revalidate "same command buffer" events
        if (sync_event->last_command_tag >= base_tag) {
            continue;
        }

        const VkEvent event_handle = sync_event->event->VkHandle();

        // TODO: Cleanup this error message
        if (sync_event->unsynchronized_set != vvl::Func::Empty) {
            // Issue error message that Wait is waiting on an signal subject to race condition, and is thus ignored for
            // this event
            const char* const vuid = "SYNC-vkCmdWaitEvents-unsynchronized-setops";
            const char* const message = "%s Unsychronized %s calls result in race conditions w.r.t. event signalling, %s %s";
            const char* const reason = "First synchronization scope is undefined.";
            skip |=
                env.validator.LogError(vuid, event_handle, loc, message, env.validator.FormatHandle(event_handle).c_str(),
                                       vvl::String(sync_event->last_command), reason, "Wait operation is ignored for this event.");
        }
        if (barrier_set.image_barriers.size()) {
            const auto& image_memory_barriers = barrier_set.image_barriers;
            for (const auto& image_memory_barrier : image_memory_barriers) {
                if (!image_memory_barrier.layout_transition) continue;
                const auto* image_state = image_memory_barrier.image.get();
                if (!image_state) continue;
                const auto& subresource_range = image_memory_barrier.subresource_range;
                const auto& src_access_scope = image_memory_barrier.barrier.src_access_scope;
                const auto hazard = access_context.DetectImageBarrierHazard(
                    *image_state, subresource_range, sync_event->scope.exec_scope, src_access_scope, env.queue_id,
                    sync_event->FirstScope(), sync_event->first_scope_tag, AccessContext::DetectOptions::kDetectAll);
                if (hazard.IsHazard()) {
                    LogObjectList objlist(env.handle, image_state->Handle());
                    const std::string resource_description = env.validator.FormatHandle(image_state->Handle());
                    const std::string error = env.validator.error_messages_.ImageBarrierError(
                        env, hazard, loc.function, resource_description, image_memory_barrier);
                    skip |= env.validator.SyncError(hazard.Hazard(), image_state->Handle(), loc, error);
                    break;
                }
            }
        }
        barrier_set_index += barrier_set_incr;
    }
    return skip;
}

void ApplyCmdSetEvent(SyncEnvironment& env, const std::shared_ptr<const vvl::Event>& event, const SyncExecScope& src_exec_scope,
                      const std::shared_ptr<const AccessContext>& src_access_context, ResourceUsageTag tag, vvl::Func command) {
    SyncEventState* sync_event = env.events_context.GetFromShared(event);
    if (!sync_event) {
        return;
    }

    // What happens with two SetEvent is that one cannot know what group of operations will be waited for.
    // Given:
    //     Stuff1; SetEvent; Stuff2; SetEvent; WaitEvents;
    // WaitEvents cannot know which of Stuff1, Stuff2, or both has completed execution.

    if (!sync_event->HasBarrier(src_exec_scope.stage_mask, src_exec_scope.exec_scope)) {
        sync_event->unsynchronized_set = sync_event->last_command;
        sync_event->ResetFirstScope();
    } else if (!sync_event->first_scope) {
        // We only set the scope if there isn't one
        sync_event->scope = src_exec_scope;

        // Save the shared_ptr to copy of the access_context present at set time (sent us by the caller)
        sync_event->first_scope = src_access_context;
        sync_event->unsynchronized_set = vvl::Func::Empty;
        sync_event->first_scope_tag = tag;
    }
    sync_event->last_command = command;
    sync_event->last_command_tag = tag;
    sync_event->barriers = 0;
}

void ApplyCmdResetEvent(SyncEnvironment& env, const std::shared_ptr<const vvl::Event>& event, ResourceUsageTag tag,
                        vvl::Func command) {
    SyncEventState* sync_event = env.events_context.GetFromShared(event);
    if (!sync_event) {
        return;
    }
    sync_event->last_command = command;
    sync_event->last_command_tag = tag;
    sync_event->unsynchronized_set = vvl::Func::Empty;
    sync_event->ResetFirstScope();
    sync_event->barriers = 0;
}

void ApplyCmdWaitEvents(SyncEnvironment& env, AccessContext& access_context,
                        const std::vector<std::shared_ptr<const vvl::Event>>& events, vvl::span<const BarrierSet> barrier_sets,
                        ResourceUsageTag tag, vvl::Func command) {
    // Unlike PipelineBarrier, WaitEvent is *not* limited to accesses within the current subpass (if any) and thus needs to import
    // all accesses. Can instead import for all first_scopes, or a union of them, if this becomes a performance/memory issue,
    // but with no idea of the performance of the union, nor of whether it even matters... take the simplest approach here,

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
        auto* sync_event = env.events_context.GetFromShared(event_shared);
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
            const bool can_transition_depth_slices =
                CanTransitionDepthSlices(env.validator.extensions, sub_state.base.GetImageType(), sub_state.base.create_flags);
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
        auto* sync_event = env.events_context.GetFromShared(event_shared);
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
                const BarrierScope barrier_scope(event_barrier, env.queue_id, sync_event->first_scope_tag);
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
            const BarrierScope barrier_scope(event_barrier, env.queue_id, sync_event->first_scope_tag);
            CollectBarriersFunctor collect_barriers(access_context, barrier_scope, event_barrier, barrier.layout_transition,
                                                    barrier.handle_index, pending_barriers);

            const auto& sub_state = SubState(*barrier.image);
            const bool can_transition_depth_slices =
                CanTransitionDepthSlices(env.validator.extensions, sub_state.base.GetImageType(), sub_state.base.create_flags);
            ImageRangeGen range_gen = sub_state.MakeImageRangeGen(barrier.subresource_range, can_transition_depth_slices);
            EventImageRangeGenerator filtered_range_gen(sync_event->FirstScope(), range_gen);

            access_context.UpdateMemoryAccessState(collect_barriers, filtered_range_gen);
        }
        // TODO: because each iteration applies functor to the same range, investigate if it is
        // beneficial for the functor to support multiple barriers, so we traverse access map once.
        auto global_range_gen = EventSimpleRangeGenerator(sync_event->FirstScope(), kFullRange);
        for (const auto& barrier : barrier_set.memory_barriers) {
            const SyncBarrier event_barrier = RestrictToEvent(barrier, *sync_event);
            const BarrierScope barrier_scope(event_barrier, env.queue_id, sync_event->first_scope_tag);
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
