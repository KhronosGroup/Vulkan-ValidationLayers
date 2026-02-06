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

#include "sync/sync_access_context.h"
#include "sync/sync_image.h"
#include "sync/sync_validation.h"
#include "state_tracker/buffer_state.h"
#include "state_tracker/render_pass_state.h"
#include "state_tracker/video_session_state.h"
#include <vulkan/utility/vk_format_utils.h>

namespace syncval {

bool SimpleBinding(const vvl::Bindable &bindable) { return !bindable.sparse && bindable.Binding(); }
VkDeviceSize ResourceBaseAddress(const vvl::Buffer &buffer) { return buffer.GetFakeBaseAddress(); }

void AccessContext::InitFrom(uint32_t subpass, VkQueueFlags queue_flags,
                             const std::vector<SubpassDependencyGraphNode> &dependencies, const AccessContext *contexts,
                             const AccessContext *external_context) {
    const auto &subpass_dep = dependencies[subpass];
    const bool has_barrier_from_external = subpass_dep.barrier_from_external.size() > 0U;
    prev_.reserve(subpass_dep.prev.size() + (has_barrier_from_external ? 1U : 0U));
    prev_by_subpass_.resize(subpass, nullptr);  // Can't be more prevs than the subpass we're on
    for (const auto &prev_dep : subpass_dep.prev) {
        const auto prev_pass = prev_dep.first->pass;
        const auto &prev_barriers = prev_dep.second;
        assert(prev_dep.second.size());
        prev_.emplace_back(&contexts[prev_pass], queue_flags, prev_barriers);
        prev_by_subpass_[prev_pass] = &prev_.back();
    }

    async_.reserve(subpass_dep.async.size());
    for (const auto async_subpass : subpass_dep.async) {
        // Start tags are not known at creation time (as it's done at BeginRenderpass)
        async_.emplace_back(contexts[async_subpass], kInvalidTag, kQueueIdInvalid);
    }

    if (has_barrier_from_external) {
        // Store the barrier from external with the reat, but save pointer for "by subpass" lookups.
        prev_.emplace_back(external_context, queue_flags, subpass_dep.barrier_from_external);
        src_external_ = &prev_.back();
    }
    if (subpass_dep.barrier_to_external.size()) {
        dst_external_ = SubpassBarrierTrackback(this, queue_flags, subpass_dep.barrier_to_external);
    }
}

ApplySingleBufferBarrierFunctor::ApplySingleBufferBarrierFunctor(const AccessContext &access_context,
                                                                 const BarrierScope &barrier_scope, const SyncBarrier &barrier)
    : access_context(access_context), barrier_scope(barrier_scope), barrier(barrier) {}

AccessMap::iterator ApplySingleBufferBarrierFunctor::Infill(AccessMap *accesses, const Iterator &pos_hint,
                                                            const AccessRange &range) const {
    // The buffer barrier does not need to fill the gaps because barrier
    // application to a range without accesses is a no-op.
    // Return the pos iterator unchanged to indicate that no entry was created.
    return pos_hint;
}

void ApplySingleBufferBarrierFunctor::operator()(const Iterator &pos) const {
    AccessState &access_state = pos->second;
    access_context.ApplyGlobalBarriers(access_state);
    access_state.ApplyBarrier(barrier_scope, barrier);
}

ApplySingleImageBarrierFunctor::ApplySingleImageBarrierFunctor(const AccessContext &access_context,
                                                               const BarrierScope &barrier_scope, const SyncBarrier &barrier,
                                                               bool layout_transition, uint32_t layout_transition_handle_index,
                                                               ResourceUsageTag exec_tag)
    : access_context(access_context),
      barrier_scope(barrier_scope),
      barrier(barrier),
      exec_tag(exec_tag),
      layout_transition(layout_transition),
      layout_transition_handle_index(layout_transition_handle_index) {
    // Suppress layout transition during submit time application.
    // It adds write access but this is necessary only during recording.
    if (barrier_scope.scope_queue != kQueueIdInvalid) {
        this->layout_transition = false;
        this->layout_transition_handle_index = vvl::kNoIndex32;
    }
}

AccessMap::iterator ApplySingleImageBarrierFunctor::Infill(AccessMap *accesses, const Iterator &pos_hint,
                                                           const AccessRange &range) const {
    if (!layout_transition) {
        // Do not create a new range if this is not a layout transition
        return pos_hint;
    }
    // Create a new range for layout transition write access
    auto inserted = accesses->Insert(pos_hint, range, AccessState::DefaultAccessState());
    return inserted;
}

void ApplySingleImageBarrierFunctor::operator()(const Iterator &pos) const {
    AccessState &access_state = pos->second;
    access_context.ApplyGlobalBarriers(access_state);
    access_state.ApplyBarrier(barrier_scope, barrier, layout_transition, layout_transition_handle_index, exec_tag);
}

void CollectBarriersFunctor::operator()(const Iterator &pos) const {
    AccessState &access_state = pos->second;
    access_context.ApplyGlobalBarriers(access_state);
    access_state.CollectPendingBarriers(barrier_scope, barrier, layout_transition, layout_transition_handle_index,
                                        pending_barriers);
}

void AccessContext::InitFrom(const AccessContext &other) {
    access_state_map_.Assign(other.access_state_map_);
    prev_ = other.prev_;
    prev_by_subpass_ = other.prev_by_subpass_;
    async_ = other.async_;
    src_external_ = other.src_external_;
    dst_external_ = other.dst_external_;
    start_tag_ = other.start_tag_;

    global_barriers_queue_ = other.global_barriers_queue_;
    for (uint32_t i = 0; i < other.global_barrier_def_count_; i++) {
        global_barrier_defs_[i] = other.global_barrier_defs_[i];
    }
    global_barrier_def_count_ = other.global_barrier_def_count_;
    global_barriers_ = other.global_barriers_;

    // Even though the "other" context may be finalized, we might still need to update "this" copy.
    // Therefore, the copied context cannot be marked as finalized yet.
    finalized_ = false;

    sorted_first_accesses_.Clear();
}

void AccessContext::Reset() {
    access_state_map_.Clear();
    prev_.clear();
    prev_by_subpass_.clear();
    async_.clear();
    src_external_ = nullptr;
    dst_external_ = {};
    start_tag_ = {};
    ResetGlobalBarriers();
    finalized_ = false;
    sorted_first_accesses_.Clear();
}

void AccessContext::Finalize() {
    assert(!finalized_);  // no need to finalize finalized
    sorted_first_accesses_.Init(access_state_map_);
    finalized_ = true;
}

void AccessContext::RegisterGlobalBarrier(const SyncBarrier &barrier, QueueId queue_id) {
    assert(global_barriers_.empty() || global_barriers_queue_ == queue_id);

    // Search for existing def
    uint32_t def_index = 0;
    for (; def_index < global_barrier_def_count_; def_index++) {
        if (global_barrier_defs_[def_index].barrier == barrier) {
            break;
        }
    }
    // Register a new def if this barrier is encountered for the first time
    if (def_index == global_barrier_def_count_) {
        // Flush global barriers if all def slots are in use
        if (global_barrier_def_count_ == kMaxGlobaBarrierDefCount) {
            for (auto &[_, access] : access_state_map_) {
                ApplyGlobalBarriers(access);
                access.next_global_barrier_index = 0;  // to match state after reset
            }
            ResetGlobalBarriers();
            def_index = 0;
        }

        GlobalBarrierDef &new_def = global_barrier_defs_[global_barrier_def_count_++];
        new_def.barrier = barrier;
        new_def.chain_mask = 0;

        // Update chain masks
        for (uint32_t i = 0; i < global_barrier_def_count_ - 1; i++) {
            GlobalBarrierDef &def = global_barrier_defs_[i];
            if ((new_def.barrier.src_exec_scope.exec_scope & def.barrier.dst_exec_scope.exec_scope) != 0) {
                new_def.chain_mask |= 1u << i;
            }
            if ((def.barrier.src_exec_scope.exec_scope & new_def.barrier.dst_exec_scope.exec_scope) != 0) {
                def.chain_mask |= 1u << (global_barrier_def_count_ - 1);
            }
        }
    }
    // A global barrier is just a reference to its def
    global_barriers_.push_back(def_index);
    global_barriers_queue_ = queue_id;
}

void AccessContext::ApplyGlobalBarriers(AccessState &access_state) const {
    const uint32_t global_barrier_count = GetGlobalBarrierCount();
    assert(access_state.next_global_barrier_index <= global_barrier_count);
    if (access_state.next_global_barrier_index == global_barrier_count) {
        return;  // access state is up-to-date
    }
    uint32_t applied_barrier_mask = 0;  // used to skip already applied barriers
    uint32_t applied_count = 0;         // used for early exit when all unique barriers are applied
    uint32_t failed_mask = 0;           // used to quickly test barriers that failed the first application attempt

    for (size_t i = access_state.next_global_barrier_index; i < global_barrier_count; i++) {
        const uint32_t def_index = global_barriers_[i];
        const uint32_t def_mask = 1u << def_index;
        assert(def_index < global_barrier_def_count_);

        const GlobalBarrierDef &def = global_barrier_defs_[def_index];

        // Skip barriers that were already applied
        if ((def_mask & applied_barrier_mask) != 0) {
            continue;
        }

        // If this barrier failed to apply initially, it can only be applied
        // again if it can chain with one of the newly applied barriers
        if ((def_mask & failed_mask) != 0) {
            if ((def.chain_mask & applied_barrier_mask) == 0) {
                continue;
            }
        }

        // TODO: for requests with multiple barriers we need to register them in groups
        // and use PendingBarriers helper here.
        const BarrierScope barrier_scope(def.barrier, global_barriers_queue_);
        const bool is_barrier_applied = access_state.ApplyBarrier(barrier_scope, def.barrier);
        if (is_barrier_applied) {
            applied_barrier_mask |= def_mask;
            applied_count++;
            if (applied_count == global_barrier_def_count_) {
                break;  // no barriers left that can add new information
            }
        } else {
            failed_mask |= def_mask;
        }
    }
    access_state.next_global_barrier_index = global_barrier_count;
}

void AccessContext::ResetGlobalBarriers() {
    global_barriers_queue_ = kQueueIdInvalid;
    global_barrier_def_count_ = 0;
    global_barriers_.clear();
}

void AccessContext::TrimAndClearFirstAccess() {
    assert(!finalized_);
    for (auto &[range, access] : access_state_map_) {
        access.Normalize();
    }
    Consolidate(access_state_map_);
}

void AccessContext::AddReferencedTags(ResourceUsageTagSet &used) const {
    assert(!finalized_);
    for (const auto &[range, access] : access_state_map_) {
        access.GatherReferencedTags(used);
    }
}

void AccessContext::ResolveFromContext(const AccessContext &from) {
    assert(!finalized_);
    auto noop_action = [](AccessState *access) {};
    from.ResolveAccessRangeRecursePrev(kFullRange, noop_action, *this, false);
}

void AccessContext::ResolvePreviousAccesses() {
    assert(!finalized_);
    if (!prev_.empty()) {
        for (const auto &prev_dep : prev_) {
            const ApplyTrackbackStackAction barrier_action(prev_dep.barriers, nullptr);
            prev_dep.source_subpass->ResolveAccessRangeRecursePrev(kFullRange, barrier_action, *this, true);
        }
    }
}

void AccessContext::ResolveFromSubpassContext(const ApplySubpassTransitionBarriersAction &subpass_transition_action,
                                              const AccessContext &from_context,
                                              subresource_adapter::ImageRangeGenerator attachment_range_gen) {
    assert(!finalized_);
    for (; attachment_range_gen->non_empty(); ++attachment_range_gen) {
        from_context.ResolveAccessRangeRecursePrev(*attachment_range_gen, subpass_transition_action, *this, true);
    }
}

void AccessContext::ResolveAccessRange(const AccessRange &range, const AccessStateFunction &barrier_action,
                                       AccessContext &resolve_context) const {
    if (!range.non_empty()) {
        return;
    }
    AccessMap &resolve_map = resolve_context.access_state_map_;

    ParallelIterator current(resolve_map, access_state_map_, range.begin);
    while (current.range.non_empty() && range.includes(current.range.begin)) {
        const auto current_range = current.range & range;
        if (current.pos_B.inside_lower_bound_range) {
            const auto &src_pos = current.pos_B.lower_bound;

            // Create a copy of the source access state (source is this context, destination is the resolve context).
            // Then do the following steps:
            //  a) apply not yet applied global barriers
            //  b) update global barrier index to ensure global barriers from the resolve context are not applied
            //  c) apply barrier action
            AccessState src_access = src_pos->second;
            ApplyGlobalBarriers(src_access);                                                 // a
            src_access.next_global_barrier_index = resolve_context.GetGlobalBarrierCount();  // b
            barrier_action(&src_access);                                                     // c

            if (current.pos_A.inside_lower_bound_range) {
                const auto trimmed = Split(current.pos_A.lower_bound, resolve_map, current_range);
                AccessState &dst_state = trimmed->second;
                resolve_context.ApplyGlobalBarriers(dst_state);
                dst_state.Resolve(src_access);
                current.OnCurrentRangeModified(trimmed);
            } else {
                auto inserted = resolve_map.Insert(current.pos_A.lower_bound, current_range, src_access);
                current.OnCurrentRangeModified(inserted);
            }
        }
        if (current.range.non_empty()) {
            current.NextRange();
        }
    }
}

void AccessContext::ResolveAccessRangeRecursePrev(const AccessRange &range, const AccessStateFunction &barrier_action,
                                                  AccessContext &resolve_context, bool infill) const {
    if (!range.non_empty()) {
        return;
    }
    AccessMap &resolve_map = resolve_context.access_state_map_;

    ParallelIterator current(resolve_map, access_state_map_, range.begin);
    while (current.range.non_empty() && range.includes(current.range.begin)) {
        const auto current_range = current.range & range;
        if (current.pos_B.inside_lower_bound_range) {
            const auto &src_pos = current.pos_B.lower_bound;

            // Create a copy of the source access state (source is this context, destination is the resolve context).
            // Then do the following steps:
            //  a) apply not yet applied global barriers
            //  b) update global barrier index to ensure global barriers from the resolve context are not applied
            //  c) apply barrier action
            AccessState src_access = src_pos->second;
            ApplyGlobalBarriers(src_access);                                                 // a
            src_access.next_global_barrier_index = resolve_context.GetGlobalBarrierCount();  // b
            barrier_action(&src_access);                                                     // c

            if (current.pos_A.inside_lower_bound_range) {
                const auto trimmed = Split(current.pos_A.lower_bound, resolve_map, current_range);
                AccessState &dst_state = trimmed->second;
                resolve_context.ApplyGlobalBarriers(dst_state);
                dst_state.Resolve(src_access);
                current.OnCurrentRangeModified(trimmed);
            } else {
                auto inserted = resolve_map.Insert(current.pos_A.lower_bound, current_range, src_access);
                current.OnCurrentRangeModified(inserted);
            }
        } else {  // Descend to fill this gap
            AccessRange recurrence_range = current_range;
            // The current context is empty for the current_range, so recur to fill the gap.
            // Since we will be recurring back up the DAG, expand the gap descent to cover the
            // full range for which B is not valid, to minimize that recurrence
            if (current.pos_B.lower_bound == access_state_map_.end()) {
                recurrence_range.end = range.end;
            } else {
                recurrence_range.end = std::min(range.end, current.pos_B.lower_bound->first.begin);
            }

            // Note that resolve_context over the recurrence_range may contain both empty and
            // non-empty entries; only the current context has a continuous empty entry over
            // this range. Therefore, the next call must iterate over potentially multiple
            // ranges in resolve_context that cross the recurrence_range and fill the empty ones.
            ResolveGapsRecursePrev(recurrence_range, resolve_context, infill, barrier_action);

            // recurrence_range is already processed and it can be larger than the current_range.
            // The NextRange might move to the range that is still inside recurrence_range, but we
            // need the range that goes after recurrence_range. Seek to the end of recurrence_range,
            // so NextRange will get the expected range.
            // TODO: it might be simpler to seek directly to recurrence_range.end without calling NextRange().
            assert(recurrence_range.non_empty());
            const auto seek_to = recurrence_range.end - 1;
            current.SeekAfterModification(seek_to);
        }
        if (current.range.non_empty()) {
            current.NextRange();
        }
    }

    // Infill the remainder, which is empty for both the current and resolve contexts
    if (current.range.end < range.end) {
        AccessRange trailing_fill_range = {current.range.end, range.end};
        ResolveGapsRecursePrev(trailing_fill_range, resolve_context, infill, barrier_action);
    }
}

void AccessContext::ResolveGapsRecursePrev(const AccessRange &range, AccessContext &descent_context, bool infill,
                                           const AccessStateFunction &previous_barrier_action) const {
    assert(range.non_empty());
    AccessMap &descent_map = descent_context.access_state_map_;
    if (prev_.empty()) {
        if (infill) {
            AccessState access_state = AccessState::DefaultAccessState();

            // The following is not needed for correctness but is rather an optimization. We are going to fill
            // the gaps and the application of the global barriers to an empty state is noop (nothing is in the
            // barrier's source scope). Update the index to skip application of the registered global barriers.
            access_state.next_global_barrier_index = descent_context.GetGlobalBarrierCount();

            previous_barrier_action(&access_state);
            descent_map.InfillGaps(range, access_state);
        }
    } else {
        for (const auto &prev_dep : prev_) {
            const ApplyTrackbackStackAction barrier_action(prev_dep.barriers, &previous_barrier_action);
            prev_dep.source_subpass->ResolveAccessRangeRecursePrev(range, barrier_action, descent_context, infill);
        }
    }
}

AccessMap::iterator AccessContext::ResolveGapRecursePrev(const AccessRange &gap_range, AccessMap::iterator pos_hint) {
    assert(gap_range.non_empty());
    if (prev_.empty()) {
        AccessState access_state = AccessState::DefaultAccessState();

        // The following is not needed for correctness but is rather an optimization. We are going to fill
        // the gaps and the application of the global barriers to an empty state is noop (nothing is in the
        // barrier's source scope). Update the index to skip application of the registered global barriers.
        access_state.next_global_barrier_index = GetGlobalBarrierCount();

        return access_state_map_.InfillGap(pos_hint, gap_range, access_state);
    } else {
        for (const auto &prev_dep : prev_) {
            const ApplyTrackbackStackAction barrier_action(prev_dep.barriers, nullptr);
            prev_dep.source_subpass->ResolveAccessRangeRecursePrev(gap_range, barrier_action, *this, true);
        }
        return access_state_map_.LowerBound(gap_range.begin);
    }
}

// Update memory access state over the given range.
// This inserts new accesses for empty regions and updates existing accesses.
// The passed pos must either be a lower bound (can be the end iterator) or be strictly less than the range.
// Map entries that intersect range.begin or range.end are split at the intersection point.
AccessMap::iterator AccessContext::DoUpdateAccessState(AccessMap::iterator pos, const AccessRange &range,
                                                       SyncAccessIndex access_index, SyncOrdering ordering_rule,
                                                       ResourceUsageTagEx tag_ex, SyncFlags flags) {
    assert(range.non_empty());
    const SyncAccessInfo &access_info = GetAccessInfo(access_index);

    const auto end = access_state_map_.end();
    assert(pos == access_state_map_.LowerBound(range.begin) || pos->first.strictly_less(range));

    if (pos != end && pos->first.strictly_less(range)) {
        // pos is not a lower bound for the range (pos < range), but if the range is
        // monotonically increasing, the next map entry may be the lower bound
        ++pos;

        // If the new pos is not a lower bound, run the full search
        if (pos != end && pos->first.strictly_less(range)) {
            pos = access_state_map_.LowerBound(range.begin);
        }
    }
    assert(pos == access_state_map_.LowerBound(range.begin));

    if (pos != end && range.begin > pos->first.begin) {
        // Lower bound starts before the range.
        // Split the entry so that a new entry starts exactly at the range.begin
        pos = access_state_map_.Split(pos, range.begin);
        ++pos;
    }

    AccessMap::index_type current_begin = range.begin;
    while (pos != end && current_begin < range.end) {
        if (current_begin < pos->first.begin) {  // infill the gap
            // Infill the gap with an empty access state or, if the previous contexts
            // exists (subpass case), derive the infill state from them
            const AccessRange gap_range(current_begin, std::min(range.end, pos->first.begin));
            AccessMap::iterator infilled_it = ResolveGapRecursePrev(gap_range, pos);

            // Update
            AccessState &new_access_state = infilled_it->second;
            ApplyGlobalBarriers(new_access_state);
            new_access_state.Update(access_info, ordering_rule, tag_ex, flags);

            // Advance current location.
            // Do not advance pos, as it's the next map entry to visit
            current_begin = pos->first.begin;
        } else {  // update existing entry
            assert(current_begin == pos->first.begin);

            // Split the current map entry if it goes beyond range.end.
            // This ensures the update is restricted to the given range.
            if (pos->first.end > range.end) {
                pos = access_state_map_.Split(pos, range.end);
            }

            // Update
            AccessState &access_state = pos->second;
            ApplyGlobalBarriers(access_state);
            access_state.Update(access_info, ordering_rule, tag_ex, flags);

            // Advance both current location and map entry
            current_begin = pos->first.end;
            ++pos;
        }
    }

    // Fill to the end if needed
    if (current_begin < range.end) {
        const AccessRange gap_range(current_begin, range.end);
        AccessMap::iterator infilled_it = ResolveGapRecursePrev(gap_range, pos);

        // Update
        AccessState &new_access_state = infilled_it->second;
        ApplyGlobalBarriers(new_access_state);
        new_access_state.Update(access_info, ordering_rule, tag_ex, flags);
    }
    return pos;
}

void AccessContext::UpdateAccessState(const vvl::Buffer &buffer, SyncAccessIndex current_usage, const AccessRange &range,
                                      ResourceUsageTagEx tag_ex, SyncFlags flags) {
    assert(range.valid());
    assert(!finalized_);

    if (current_usage == SYNC_ACCESS_INDEX_NONE) {
        return;
    }
    if (!SimpleBinding(buffer)) {
        return;
    }
    if (range.empty()) {
        return;
    }

    const VkDeviceSize base_address = ResourceBaseAddress(buffer);
    const AccessRange buffer_range = range + base_address;

    auto pos = access_state_map_.LowerBound(buffer_range.begin);
    DoUpdateAccessState(pos, buffer_range, current_usage, SyncOrdering::kOrderingNone, tag_ex, flags);
}

void AccessContext::UpdateAccessState(ImageRangeGen &range_gen, SyncAccessIndex current_usage, SyncOrdering ordering_rule,
                                      ResourceUsageTagEx tag_ex, SyncFlags flags) {
    assert(!finalized_);

    if (current_usage == SYNC_ACCESS_INDEX_NONE) {
        return;
    }

    auto pos = access_state_map_.LowerBound(range_gen->begin);
    for (; range_gen->non_empty(); ++range_gen) {
        pos = DoUpdateAccessState(pos, *range_gen, current_usage, ordering_rule, tag_ex, flags);
    }
}

void AccessContext::ResolveChildContexts(vvl::span<AccessContext> subpass_contexts) {
    assert(!finalized_);

    for (AccessContext &context : subpass_contexts) {
        ApplyTrackbackStackAction barrier_action(context.GetDstExternalTrackBack().barriers);
        context.ResolveAccessRange(kFullRange, barrier_action, *this);
    }
}

// Caller must ensure that lifespan of this is less than the lifespan of from
void AccessContext::ImportAsyncContexts(const AccessContext &from) {
    async_.insert(async_.end(), from.async_.begin(), from.async_.end());
}

void AccessContext::AddAsyncContext(const AccessContext *context, ResourceUsageTag tag, QueueId queue_id) {
    if (context) {
        async_.emplace_back(*context, tag, queue_id);
    }
}

void SortedFirstAccesses::Init(const AccessMap &finalized_access_map) {
    for (const auto &entry : finalized_access_map) {
        const AccessState &access = entry.second;
        const ResourceUsageRange range = access.GetFirstAccessRange();
        if (range.empty()) {
            continue;
        }
        // Access map is not going to be updated (finalized) and we can store references to map entries
        if (range.size() == 1) {
            sorted_single_tags.emplace_back(SingleTag{range.begin, &entry});
        } else {
            sorted_multi_tags.emplace_back(MultiTag{range, &entry});
        }
    }
    std::sort(sorted_single_tags.begin(), sorted_single_tags.end(),
              [](const SingleTag &a, const SingleTag &b) { return a.tag < b.tag; });
    std::sort(sorted_multi_tags.begin(), sorted_multi_tags.end(),
              [](const auto &a, const auto &b) { return a.range.begin < b.range.begin; });
}

void SortedFirstAccesses::Clear() {
    sorted_single_tags.clear();
    sorted_multi_tags.clear();
}

std::vector<SortedFirstAccesses::SingleTag>::const_iterator SortedFirstAccesses::SingleTagRange::begin() {
    return std::lower_bound(sorted_single_tags.begin(), sorted_single_tags.end(), tag_range.begin,
                            [](const SingleTag &single_tag, ResourceUsageTag tag) { return single_tag.tag < tag; });
}

std::vector<SortedFirstAccesses::SingleTag>::const_iterator SortedFirstAccesses::SingleTagRange::end() {
    return std::lower_bound(sorted_single_tags.begin(), sorted_single_tags.end(), tag_range.end,
                            [](const SingleTag &single_tag, ResourceUsageTag tag) { return single_tag.tag < tag; });
}

SortedFirstAccesses::SingleTagRange SortedFirstAccesses::IterateSingleTagFirstAccesses(const ResourceUsageRange &tag_range) const {
    return SingleTagRange{this->sorted_single_tags, tag_range};
}

std::vector<SortedFirstAccesses::MultiTag>::const_iterator SortedFirstAccesses::MultiTagRange::begin() {
    return sorted_multi_tags.begin();
}

std::vector<SortedFirstAccesses::MultiTag>::const_iterator SortedFirstAccesses::MultiTagRange::end() {
    return std::lower_bound(sorted_multi_tags.begin(), sorted_multi_tags.end(), tag_range.end,
                            [](const MultiTag &multi_tag, ResourceUsageTag tag) { return multi_tag.range.begin < tag; });
}

SortedFirstAccesses::MultiTagRange SortedFirstAccesses::IterateMultiTagFirstAccesses(const ResourceUsageRange &tag_range) const {
    return MultiTagRange{this->sorted_multi_tags, tag_range};
}

// For RenderPass time validation this is "start tag", for QueueSubmit, this is the earliest
// unsynchronized tag for the Queue being tested against (max synchrononous + 1, perhaps)
ResourceUsageTag AccessContext::AsyncReference::StartTag() const { return (tag_ == kInvalidTag) ? context_->StartTag() : tag_; }

AttachmentViewGen::AttachmentViewGen(const vvl::ImageView *image_view, const VkOffset3D &offset, const VkExtent3D &extent)
    : view_(image_view) {
    gen_store_[Gen::kViewSubresource].emplace(MakeImageRangeGen(*image_view));

    const bool has_depth = vkuFormatHasDepth(image_view->create_info.format);
    const bool has_stencil = vkuFormatHasStencil(image_view->create_info.format);

    // For depth-stencil attachment, the view's aspect flags are ignored according to the spec.
    // MakeImageRangeGen works with the aspect flags. Derive aspect from format.
    VkImageAspectFlags override_aspect_flags = 0;
    if (has_depth || has_stencil) {
        override_aspect_flags |= has_depth ? VK_IMAGE_ASPECT_DEPTH_BIT : 0;
        override_aspect_flags |= has_stencil ? VK_IMAGE_ASPECT_STENCIL_BIT : 0;
    }

    // Range gen for attachment's render area
    gen_store_[Gen::kRenderArea].emplace(MakeImageRangeGen(*image_view, offset, extent, override_aspect_flags));

    // If attachment has both depth and stencil aspects then add range gens to represent each aspect separately.
    if (has_depth && has_stencil) {
        gen_store_[Gen::kDepthOnlyRenderArea].emplace(MakeImageRangeGen(*image_view, offset, extent, VK_IMAGE_ASPECT_DEPTH_BIT));
        gen_store_[Gen::kStencilOnlyRenderArea].emplace(
            MakeImageRangeGen(*image_view, offset, extent, VK_IMAGE_ASPECT_STENCIL_BIT));
    }
}

const std::optional<ImageRangeGen> &AttachmentViewGen::GetRangeGen(AttachmentViewGen::Gen type) const {
    static_assert(Gen::kGenSize == 4, "Function written with this assumption");
    // If the view is a depth only view, then the depth only portion of the render area is simply the render area.
    // If the view is a depth stencil view, then the depth only portion of the render area will be a subset,
    // and thus needs the generator function that will produce the address ranges of that subset
    const bool depth_only = (type == kDepthOnlyRenderArea) && vkuFormatIsDepthOnly(view_->create_info.format);
    const bool stencil_only = (type == kStencilOnlyRenderArea) && vkuFormatIsStencilOnly(view_->create_info.format);
    if (depth_only || stencil_only) {
        type = Gen::kRenderArea;
    }
    return gen_store_[type];
}

AttachmentViewGen::Gen AttachmentViewGen::GetDepthStencilRenderAreaGenType(bool depth_op, bool stencil_op) const {
    assert(vkuFormatIsDepthOrStencil(view_->create_info.format));
    if (depth_op) {
        assert(vkuFormatHasDepth(view_->create_info.format));
        if (stencil_op) {
            assert(vkuFormatHasStencil(view_->create_info.format));
            return kRenderArea;
        }
        return kDepthOnlyRenderArea;
    }
    if (stencil_op) {
        assert(vkuFormatHasStencil(view_->create_info.format));
        return kStencilOnlyRenderArea;
    }
    assert(depth_op || stencil_op);
    return kRenderArea;
}

}  // namespace syncval
