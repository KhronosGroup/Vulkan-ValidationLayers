/*
 * Copyright (c) 2019-2024 Valve Corporation
 * Copyright (c) 2019-2024 LunarG, Inc.
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
#include "sync/sync_utils.h"
#include "sync/sync_access_state.h"

ResourceAccessState::OrderingBarriers ResourceAccessState::kOrderingRules = {
    {{VK_PIPELINE_STAGE_2_NONE_KHR, SyncStageAccessFlags()},
     {kColorAttachmentExecScope, kColorAttachmentAccessScope},
     {kDepthStencilAttachmentExecScope, kDepthStencilAttachmentAccessScope},
     {kRasterAttachmentExecScope, kRasterAttachmentAccessScope}}};

// Apply a list of barriers, without resolving pending state, useful for subpass layout transitions
void ResourceAccessState::ApplyBarriers(const std::vector<SyncBarrier> &barriers, bool layout_transition) {
    const UntaggedScopeOps scope;
    for (const auto &barrier : barriers) {
        ApplyBarrier(scope, barrier, layout_transition);
    }
}

// ApplyBarriers is design for *fully* inclusive barrier lists without layout tranistions.  Designed use was for
// inter-subpass barriers for lazy-evaluation of parent context memory ranges.  Subpass layout transistions are *not* done
// lazily, s.t. no previous access reports should need layout transitions.
void ResourceAccessState::ApplyBarriersImmediate(const std::vector<SyncBarrier> &barriers) {
    assert(!HasPendingState());  // This should never be call in the middle of another barrier application
    const UntaggedScopeOps scope;
    for (const auto &barrier : barriers) {
        ApplyBarrier(scope, barrier, false);
    }
    ApplyPendingBarriers(kInvalidTag);  // There can't be any need for this tag
}
HazardResult ResourceAccessState::DetectHazard(const SyncStageAccessInfoType &usage_info) const {
    HazardResult hazard;
    const auto &usage_stage = usage_info.stage_mask;
    if (IsRead(usage_info)) {
        if (IsRAWHazard(usage_info)) {
            hazard.Set(this, usage_info, READ_AFTER_WRITE, *last_write);
        }
    } else {
        // Write operation:
        // Check for read operations more recent than last_write (as setting last_write clears reads, that would be *any*
        // If reads exists -- test only against them because either:
        //     * the reads were hazards, and we've reported the hazard, so just test the current write vs. the read operations
        //     * the read weren't hazards, and thus if the write is safe w.r.t. the reads, no hazard vs. last_write is possible if
        //       the current write happens after the reads, so just test the write against the reades
        // Otherwise test against last_write
        //
        // Look for casus belli for WAR
        if (last_reads.size()) {
            for (const auto &read_access : last_reads) {
                if (IsReadHazard(usage_stage, read_access)) {
                    hazard.Set(this, usage_info, WRITE_AFTER_READ, read_access.access, read_access.tag);
                    break;
                }
            }
        } else if (last_write.has_value() && last_write->IsWriteHazard(usage_info)) {
            // Write-After-Write check -- if we have a previous write to test against
            hazard.Set(this, usage_info, WRITE_AFTER_WRITE, *last_write);
        }
    }
    return hazard;
}

HazardResult ResourceAccessState::DetectHazard(const SyncStageAccessInfoType &usage_info, const SyncOrdering ordering_rule,
                                               QueueId queue_id) const {
    const auto &ordering = GetOrderingRules(ordering_rule);
    return DetectHazard(usage_info, ordering, queue_id);
}

HazardResult ResourceAccessState::DetectHazard(const SyncStageAccessInfoType &usage_info, const OrderingBarrier &ordering,
                                               QueueId queue_id) const {
    // The ordering guarantees act as barriers to the last accesses, independent of synchronization operations
    HazardResult hazard;
    const auto &usage_bit = usage_info.stage_access_bit;
    const auto &usage_stage = usage_info.stage_mask;
    const auto &usage_index = usage_info.stage_access_index;
    const bool input_attachment_ordering = ordering.access_scope[SYNC_FRAGMENT_SHADER_INPUT_ATTACHMENT_READ];

    if (IsRead(usage_info)) {
        // Exclude RAW if no write, or write not most "most recent" operation w.r.t. usage;
        bool is_raw_hazard = IsRAWHazard(usage_info);
        if (is_raw_hazard) {
            // NOTE: we know last_write is non-zero
            // See if the ordering rules save us from the simple RAW check above
            // First check to see if the current usage is covered by the ordering rules
            const bool usage_is_input_attachment = (usage_index == SYNC_FRAGMENT_SHADER_INPUT_ATTACHMENT_READ);
            const bool usage_is_ordered =
                (input_attachment_ordering && usage_is_input_attachment) || (0 != (usage_stage & ordering.exec_scope));
            if (usage_is_ordered) {
                // Now see of the most recent write (or a subsequent read) are ordered
                const bool most_recent_is_ordered =
                    last_write->IsOrdered(ordering, queue_id) || (0 != GetOrderedStages(queue_id, ordering));
                is_raw_hazard = !most_recent_is_ordered;
            }
        }
        if (is_raw_hazard) {
            hazard.Set(this, usage_info, READ_AFTER_WRITE, *last_write);
        }
    } else if (usage_index == SyncStageAccessIndex::SYNC_IMAGE_LAYOUT_TRANSITION) {
        // For Image layout transitions, the barrier represents the first synchronization/access scope of the layout transition
        return DetectBarrierHazard(usage_info, queue_id, ordering.exec_scope, ordering.access_scope);
    } else {
        // Only check for WAW if there are no reads since last_write
        const bool usage_write_is_ordered = (usage_bit & ordering.access_scope).any();
        if (last_reads.size()) {
            // Look for any WAR hazards outside the ordered set of stages
            VkPipelineStageFlags2KHR ordered_stages = VK_PIPELINE_STAGE_2_NONE;
            if (usage_write_is_ordered) {
                // If the usage is ordered, we can ignore all ordered read stages w.r.t. WAR)
                ordered_stages = GetOrderedStages(queue_id, ordering);
            }
            // If we're tracking any reads that aren't ordered against the current write, got to check 'em all.
            if ((ordered_stages & last_read_stages) != last_read_stages) {
                for (const auto &read_access : last_reads) {
                    if (read_access.stage & ordered_stages) continue;  // but we can skip the ordered ones
                    if (IsReadHazard(usage_stage, read_access)) {
                        hazard.Set(this, usage_info, WRITE_AFTER_READ, read_access.access, read_access.tag);
                        break;
                    }
                }
            }
        } else if (last_write.has_value() && !(last_write->IsOrdered(ordering, queue_id) && usage_write_is_ordered)) {
            bool ilt_ilt_hazard = false;
            if ((usage_index == SYNC_IMAGE_LAYOUT_TRANSITION) && (last_write->IsIndex(SYNC_IMAGE_LAYOUT_TRANSITION))) {
                // ILT after ILT is a special case where we check the 2nd access scope of the first ILT against the first access
                // scope of the second ILT, which has been passed (smuggled?) in the ordering barrier
                ilt_ilt_hazard = !(last_write->Barriers() & ordering.access_scope).any();
            }
            if (ilt_ilt_hazard || last_write->IsWriteHazard(usage_info)) {
                hazard.Set(this, usage_info, WRITE_AFTER_WRITE, *last_write);
            }
        }
    }
    return hazard;
}

HazardResult ResourceAccessState::DetectHazard(const ResourceAccessState &recorded_use, QueueId queue_id,
                                               const ResourceUsageRange &tag_range) const {
    HazardResult hazard;
    using Size = FirstAccesses::size_type;
    const auto &recorded_accesses = recorded_use.first_accesses_;
    Size count = recorded_accesses.size();
    if (count) {
        // First access is only closed if the last is a write
        bool do_write_last = recorded_use.first_access_closed_;
        if (do_write_last) {
            // Note: We know count > 0 so this is alway safe.
            --count;
        }

        for (Size i = 0; i < count; ++i) {
            const auto &first = recorded_accesses[i];
            // Skip and quit logic
            if (first.tag < tag_range.begin) continue;
            if (first.tag >= tag_range.end) {
                do_write_last = false;  // ignore last since we know it can't be in tag_range
                break;
            }

            hazard = DetectHazard(*first.usage_info, first.ordering_rule, queue_id);
            if (hazard.IsHazard()) {
                hazard.AddRecordedAccess(first);
                break;
            }
        }

        if (do_write_last) {
            // Writes are a bit special... both for the "most recent" access logic, and layout transition specific logic
            const auto &last_access = recorded_accesses.back();
            if (tag_range.includes(last_access.tag)) {
                OrderingBarrier barrier = GetOrderingRules(last_access.ordering_rule);
                if (last_access.usage_info->stage_access_index == SyncStageAccessIndex::SYNC_IMAGE_LAYOUT_TRANSITION) {
                    // Or in the layout first access scope as a barrier... IFF the usage is an ILT
                    // this was saved off in the "apply barriers" logic to simplify ILT access checks as they straddle
                    // the barrier that applies them
                    barrier |= recorded_use.first_write_layout_ordering_;
                }
                // Any read stages present in the recorded context (this) are most recent to the write, and thus mask those stages
                // in the active context
                if (recorded_use.first_read_stages_) {
                    // we need to ignore the first use read stage in the active context (so we add them to the ordering rule),
                    // reads in the active context are not "most recent" as all recorded context operations are *after* them
                    // This supresses only RAW checks for stages present in the recorded context, but not those only present in the
                    // active context.
                    barrier.exec_scope |= recorded_use.first_read_stages_;
                    // if there are any first use reads, we suppress WAW by injecting the active context write in the ordering rule
                    barrier.access_scope |= last_access.usage_info->stage_access_bit;
                }
                hazard = DetectHazard(*last_access.usage_info, barrier, queue_id);
                if (hazard.IsHazard()) {
                    hazard.AddRecordedAccess(last_access);
                }
            }
        }
    }
    return hazard;
}

// Asynchronous Hazards occur between subpasses with no connection through the DAG
HazardResult ResourceAccessState::DetectAsyncHazard(const SyncStageAccessInfoType &usage_info,
                                                    const ResourceUsageTag start_tag) const {
    HazardResult hazard;
    // Async checks need to not go back further than the start of the subpass, as we only want to find hazards between the async
    // subpasses.  Anything older than that should have been checked at the start of each subpass, taking into account all of
    // the raster ordering rules.
    if (IsRead(usage_info)) {
        if (last_write.has_value() && (last_write->tag_ >= start_tag)) {
            hazard.Set(this, usage_info, READ_RACING_WRITE, *last_write);
        }
    } else {
        if (last_write.has_value() && (last_write->tag_ >= start_tag)) {
            hazard.Set(this, usage_info, WRITE_RACING_WRITE, *last_write);
        } else if (last_reads.size() > 0) {
            // Any reads during the other subpass will conflict with this write, so we need to check them all.
            for (const auto &read_access : last_reads) {
                if (read_access.tag >= start_tag) {
                    hazard.Set(this, usage_info, WRITE_RACING_READ, read_access.access, read_access.tag);
                    break;
                }
            }
        }
    }
    return hazard;
}

HazardResult ResourceAccessState::DetectAsyncHazard(const ResourceAccessState &recorded_use, const ResourceUsageRange &tag_range,
                                                    ResourceUsageTag start_tag) const {
    HazardResult hazard;
    for (const auto &first : recorded_use.first_accesses_) {
        // Skip and quit logic
        if (first.tag < tag_range.begin) continue;
        if (first.tag >= tag_range.end) break;

        hazard = DetectAsyncHazard(*first.usage_info, start_tag);
        if (hazard.IsHazard()) {
            hazard.AddRecordedAccess(first);
            break;
        }
    }
    return hazard;
}

HazardResult ResourceAccessState::DetectBarrierHazard(const SyncStageAccessInfoType &usage_info, QueueId queue_id,
                                                      VkPipelineStageFlags2KHR src_exec_scope,
                                                      const SyncStageAccessFlags &src_access_scope) const {
    // Only supporting image layout transitions for now
    assert(usage_info.stage_access_index == SyncStageAccessIndex::SYNC_IMAGE_LAYOUT_TRANSITION);
    HazardResult hazard;
    // only test for WAW if there no intervening read operations.
    // See DetectHazard(SyncStagetAccessIndex) above for more details.
    if (last_reads.size()) {
        // Look at the reads if any
        for (const auto &read_access : last_reads) {
            if (read_access.IsReadBarrierHazard(queue_id, src_exec_scope)) {
                hazard.Set(this, usage_info, WRITE_AFTER_READ, read_access.access, read_access.tag);
                break;
            }
        }
    } else if (last_write.has_value() && IsWriteBarrierHazard(queue_id, src_exec_scope, src_access_scope)) {
        hazard.Set(this, usage_info, WRITE_AFTER_WRITE, *last_write);
    }

    return hazard;
}

HazardResult ResourceAccessState::DetectBarrierHazard(const SyncStageAccessInfoType &usage_info,
                                                      const ResourceAccessState &scope_state,
                                                      VkPipelineStageFlags2KHR src_exec_scope,
                                                      const SyncStageAccessFlags &src_access_scope, QueueId event_queue,
                                                      ResourceUsageTag event_tag) const {
    // Only supporting image layout transitions for now
    assert(usage_info.stage_access_index == SyncStageAccessIndex::SYNC_IMAGE_LAYOUT_TRANSITION);
    HazardResult hazard;

    if (last_write.has_value() && (last_write->tag_ >= event_tag)) {
        // Any write after the event precludes the possibility of being in the first access scope for the layout transition
        hazard.Set(this, usage_info, WRITE_AFTER_WRITE, *last_write);
    } else {
        // only test for WAW if there no intervening read operations.
        // See DetectHazard(SyncStagetAccessIndex) above for more details.
        if (last_reads.size()) {
            // Look at the reads if any... if reads exist, they are either the reason the access is in the event
            // first scope, or they are a hazard.
            const ReadStates &scope_reads = scope_state.last_reads;
            const ReadStates::size_type scope_read_count = scope_reads.size();
            // Since the hasn't been a write:
            //  * The current read state is a superset of the scoped one
            //  * The stage order is the same.
            assert(last_reads.size() >= scope_read_count);
            for (ReadStates::size_type read_idx = 0; read_idx < scope_read_count; ++read_idx) {
                const ReadState &scope_read = scope_reads[read_idx];
                const ReadState &current_read = last_reads[read_idx];
                assert(scope_read.stage == current_read.stage);
                if (current_read.tag > event_tag) {
                    // The read is more recent than the set event scope, thus no barrier from the wait/ILT.
                    hazard.Set(this, usage_info, WRITE_AFTER_READ, current_read.access, current_read.tag);
                } else {
                    // The read is in the events first synchronization scope, so we use a barrier hazard check
                    // If the read stage is not in the src sync scope
                    // *AND* not execution chained with an existing sync barrier (that's the or)
                    // then the barrier access is unsafe (R/W after R)
                    if (scope_read.IsReadBarrierHazard(event_queue, src_exec_scope)) {
                        hazard.Set(this, usage_info, WRITE_AFTER_READ, scope_read.access, scope_read.tag);
                        break;
                    }
                }
            }
            if (!hazard.IsHazard() && (last_reads.size() > scope_read_count)) {
                const ReadState &current_read = last_reads[scope_read_count];
                hazard.Set(this, usage_info, WRITE_AFTER_READ, current_read.access, current_read.tag);
            }
        } else if (last_write.has_value()) {
            // if there are no reads, the write is either the reason the access is in the event scope... they are a hazard
            // The write is in the first sync scope of the event (sync their aren't any reads to be the reason)
            // So do a normal barrier hazard check
            if (scope_state.IsWriteBarrierHazard(event_queue, src_exec_scope, src_access_scope)) {
                hazard.Set(&scope_state, usage_info, WRITE_AFTER_WRITE, *scope_state.last_write);
            }
        }
    }

    return hazard;
}
void ResourceAccessState::MergePending(const ResourceAccessState &other) {
    pending_layout_transition |= other.pending_layout_transition;
}

void ResourceAccessState::MergeReads(const ResourceAccessState &other) {
    // Merge the read states
    const auto pre_merge_count = last_reads.size();
    const auto pre_merge_stages = last_read_stages;
    for (uint32_t other_read_index = 0; other_read_index < other.last_reads.size(); other_read_index++) {
        auto &other_read = other.last_reads[other_read_index];
        if (pre_merge_stages & other_read.stage) {
            // Merge in the barriers for read stages that exist in *both* this and other
            // TODO: This is N^2 with stages... perhaps the ReadStates should be sorted by stage index.
            //       but we should wait on profiling data for that.
            for (uint32_t my_read_index = 0; my_read_index < pre_merge_count; my_read_index++) {
                auto &my_read = last_reads[my_read_index];
                if (other_read.stage == my_read.stage) {
                    if (my_read.tag < other_read.tag) {
                        // Other is more recent, copy in the state
                        my_read.access = other_read.access;
                        my_read.tag = other_read.tag;
                        my_read.queue = other_read.queue;
                        my_read.pending_dep_chain = other_read.pending_dep_chain;
                        // TODO: Phase 2 -- review the state merge logic to avoid false positive from overwriting the barriers
                        //                  May require tracking more than one access per stage.
                        my_read.barriers = other_read.barriers;
                        my_read.sync_stages = other_read.sync_stages;
                        if (my_read.stage == VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT_KHR) {
                            // Since I'm overwriting the fragement stage read, also update the input attachment info
                            // as this is the only stage that affects it.
                            input_attachment_read = other.input_attachment_read;
                        }
                    } else if (other_read.tag == my_read.tag) {
                        // The read tags match so merge the barriers
                        my_read.barriers |= other_read.barriers;
                        my_read.sync_stages |= other_read.sync_stages;
                        my_read.pending_dep_chain |= other_read.pending_dep_chain;
                    }

                    break;
                }
            }
        } else {
            // The other read stage doesn't exist in this, so add it.
            last_reads.emplace_back(other_read);
            last_read_stages |= other_read.stage;
            if (other_read.stage == VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT_KHR) {
                input_attachment_read = other.input_attachment_read;
            }
        }
    }
    read_execution_barriers |= other.read_execution_barriers;
}

// The logic behind resolves is the same as update, we assume that earlier hazards have be reported, and that no
// tranistive hazard can exists with a hazard between the earlier operations.  Yes, an early hazard can mask that another
// exists, but if you fix *that* hazard it either fixes or unmasks the subsequent ones.
void ResourceAccessState::Resolve(const ResourceAccessState &other) {
    bool skip_first = false;
    if (last_write.has_value()) {
        if (other.last_write.has_value()) {
            if (last_write->Tag() < other.last_write->Tag()) {
                // NOTE: Both last and other have writes, and thus first access is "closed". We are selecting other's
                //       first_access state, but it and this can only differ if there are async hazards
                //       error state.
                //
                // If this is a later write, we've reported any exsiting hazard, and we can just overwrite as the more recent
                // operation
                *this = other;
                skip_first = true;
            } else if (last_write->Tag() == other.last_write->Tag()) {
                // In the *equals* case for write operations, we merged the write barriers and the read state (but without the
                // dependency chaining logic or any stage expansion)
                last_write->MergeBarriers(*other.last_write);
                MergePending(other);
                MergeReads(other);
            } else {
                // other write is before this write... in which case we keep this instead of other
                // and can skip the "first_access" merge, since first_access has been closed since other write tag or before
                skip_first = true;
            }
        } else {
            // this has a write and other doesn't -- at best async read in other, which have been reported, and will be dropped
            // Since this has a write first access is closed and shouldn't be updated by other
            skip_first = true;
        }
    } else if (other.last_write.has_value()) {  // && not this->last_write
        // Other has write and this doesn't, thus keep it, See first access NOTE above
        *this = other;
        skip_first = true;
    } else {  // not this->last_write OR other.last_write
        // Neither state has a write, just merge the reads
        MergePending(other);
        MergeReads(other);
    }

    // Merge first access information by making a copy of this first_access and reconstructing with a shuffle
    // of the copy and other into this using the update first logic.
    // NOTE: All sorts of additional cleverness could be put into short circuts.  (for example back is write and is before front
    //       of the other first_accesses... )
    if (!skip_first && !(first_accesses_ == other.first_accesses_) && !other.first_accesses_.empty()) {
        FirstAccesses firsts(std::move(first_accesses_));
        ClearFirstUse();
        auto a = firsts.begin();
        auto a_end = firsts.end();
        for (auto &b : other.first_accesses_) {
            // TODO: Determine whether some tag offset will be needed for PHASE II
            while ((a != a_end) && (a->tag < b.tag)) {
                UpdateFirst(a->tag, *a->usage_info, a->ordering_rule);
                ++a;
            }
            UpdateFirst(b.tag, *b.usage_info, b.ordering_rule);
        }
        for (; a != a_end; ++a) {
            UpdateFirst(a->tag, *a->usage_info, a->ordering_rule);
        }
    }
}

void ResourceAccessState::Update(const SyncStageAccessInfoType &usage_info, SyncOrdering ordering_rule,
                                 const ResourceUsageTag tag) {
    // Move this logic in the ResourceStateTracker as methods, thereof (or we'll repeat it for every flavor of resource...
    const auto &usage_bit = usage_info.stage_access_bit;
    const auto &usage_stage = usage_info.stage_mask;
    if (IsRead(usage_info)) {
        // Mulitple outstanding reads may be of interest and do dependency chains independently
        // However, for purposes of barrier tracking, only one read per pipeline stage matters
        if (usage_stage & last_read_stages) {
            const auto not_usage_stage = ~usage_stage;
            for (auto &read_access : last_reads) {
                if (read_access.stage == usage_stage) {
                    read_access.Set(usage_stage, usage_bit, 0, tag);
                } else if (read_access.barriers & usage_stage) {
                    // If the current access is barriered to this stage, mark it as "known to happen after"
                    read_access.sync_stages |= usage_stage;
                } else {
                    // If the current access is *NOT* barriered to this stage it needs to be cleared.
                    // Note: this is possible because semaphores can *clear* effective barriers, so the assumption
                    //       that sync_stages is a subset of barriers may not apply.
                    read_access.sync_stages &= not_usage_stage;
                }
            }
        } else {
            for (auto &read_access : last_reads) {
                if (read_access.barriers & usage_stage) {
                    read_access.sync_stages |= usage_stage;
                }
            }
            last_reads.emplace_back(usage_stage, usage_bit, 0, tag);
            last_read_stages |= usage_stage;
        }

        // Fragment shader reads come in two flavors, and we need to track if the one we're tracking is the special one.
        if (usage_stage == VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT_KHR) {
            // TODO Revisit re: multiple reads for a given stage
            input_attachment_read = (usage_bit == SYNC_FRAGMENT_SHADER_INPUT_ATTACHMENT_READ_BIT);
        }
    } else {
        // Assume write
        // TODO determine what to do with READ-WRITE operations if any
        SetWrite(usage_info, tag);
    }
    UpdateFirst(tag, usage_info, ordering_rule);
}

void HazardResult::Set(const ResourceAccessState *access_state_, const SyncStageAccessInfoType &usage_info_, SyncHazard hazard_,
                       const ResourceAccessWriteState &prior_write) {
    state_.emplace(access_state_, usage_info_, hazard_, prior_write.Access().stage_access_bit, prior_write.Tag());
}

void HazardResult::Set(const ResourceAccessState *access_state_, const SyncStageAccessInfoType &usage_info_, SyncHazard hazard_,
                       const SyncStageAccessFlags &prior_, ResourceUsageTag tag_) {
    state_.emplace(access_state_, usage_info_, hazard_, prior_, tag_);
}

void HazardResult::AddRecordedAccess(const ResourceFirstAccess &first_access) {
    assert(state_.has_value());
    state_->recorded_access = std::make_unique<const ResourceFirstAccess>(first_access);
}
bool HazardResult::IsWAWHazard() const {
    assert(state_.has_value());
    return (state_->hazard == WRITE_AFTER_WRITE) && (state_->prior_access[state_->usage_index]);
}

// Clobber last read and all barriers... because all we have is DANGER, DANGER, WILL ROBINSON!!!
// if the last_reads/last_write were unsafe, we've reported them, in either case the prior access is irrelevant.
// We can overwrite them as *this* write is now after them.
//
// Note: intentionally ignore pending barriers and chains (i.e. don't apply or clear them), let ApplyPendingBarriers handle them.
void ResourceAccessState::SetWrite(const SyncStageAccessInfoType &usage_info, const ResourceUsageTag tag) {
    ClearRead();
    if (last_write.has_value()) {
        last_write->Set(usage_info, tag);
    } else {
        last_write.emplace(usage_info, tag);
    }
}

void ResourceAccessState::ClearWrite() { last_write.reset(); }

void ResourceAccessState::ClearRead() {
    last_reads.clear();
    last_read_stages = VK_PIPELINE_STAGE_2_NONE;
    read_execution_barriers = VK_PIPELINE_STAGE_2_NONE;
    input_attachment_read = false;  // Denotes no outstanding input attachment read after the last write.
}

void ResourceAccessState::ClearPending() {
    pending_layout_transition = false;
    if (last_write.has_value()) last_write->ClearPending();
}

void ResourceAccessState::ClearFirstUse() {
    first_accesses_.clear();
    first_read_stages_ = VK_PIPELINE_STAGE_2_NONE;
    first_write_layout_ordering_ = OrderingBarrier();
    first_access_closed_ = false;
}

void ResourceAccessState::ApplyPendingBarriers(const ResourceUsageTag tag) {
    if (pending_layout_transition) {
        // SetWrite clobbers the last_reads array, and thus we don't have to clear the read_state out.
        const SyncStageAccessInfoType &layout_usage_info = UsageInfo(SYNC_IMAGE_LAYOUT_TRANSITION);
        SetWrite(layout_usage_info, tag);  // Side effect notes below
        UpdateFirst(tag, layout_usage_info, SyncOrdering::kNonAttachment);
        TouchupFirstForLayoutTransition(tag, last_write->GetPendingLayoutOrdering());

        last_write->ApplyPendingBarriers();
        last_write->ClearPending();
        pending_layout_transition = false;
    } else {
        // Apply the accumulate execution barriers (and thus update chaining information)
        // for layout transition, last_reads is reset by SetWrite, so this will be skipped.
        for (auto &read_access : last_reads) {
            read_execution_barriers |= read_access.ApplyPendingBarriers();
        }

        // We OR in the accumulated write chain and barriers even in the case of a layout transition as SetWrite zeros them.
        if (last_write.has_value()) {
            last_write->ApplyPendingBarriers();
            last_write->ClearPending();
        }
    }
}

// Assumes signal queue != wait queue
void ResourceAccessState::ApplySemaphore(const SemaphoreScope &signal, const SemaphoreScope wait) {
    // Semaphores only guarantee the first scope of the signal is before the second scope of the wait.
    // If any access isn't in the first scope, there are no guarantees, thus those barriers are cleared
    assert(signal.queue != wait.queue);
    for (auto &read_access : last_reads) {
        if (read_access.ReadInQueueScopeOrChain(signal.queue, signal.exec_scope)) {
            // Deflects WAR on wait queue
            read_access.barriers = wait.exec_scope;
        } else {
            // Leave sync stages alone. Update method will clear unsynchronized stages on subsequent reads as needed.
            read_access.barriers = VK_PIPELINE_STAGE_2_NONE;
        }
    }
    if (WriteInQueueSourceScopeOrChain(signal.queue, signal.exec_scope, signal.valid_accesses)) {
        assert(last_write.has_value());
        // Will deflect RAW wait queue, WAW needs a chained barrier on wait queue
        read_execution_barriers = wait.exec_scope;
        last_write->barriers_ = wait.valid_accesses;
    } else {
        read_execution_barriers = VK_PIPELINE_STAGE_2_NONE;
        if (last_write.has_value()) last_write->barriers_.reset();
    }
    if (last_write.has_value()) last_write->dependency_chain_ = read_execution_barriers;
}

// Read access predicate for queue wait
bool ResourceAccessState::WaitQueueTagPredicate::operator()(const ResourceAccessState::ReadState &read_access) const {
    return (read_access.queue == queue) && (read_access.tag <= tag) &&
           (read_access.stage != VK_PIPELINE_STAGE_2_PRESENT_ENGINE_BIT_SYNCVAL);
}
bool ResourceAccessState::WaitQueueTagPredicate::operator()(const ResourceAccessState &access) const {
    if (!access.last_write.has_value()) return false;
    const auto &write_state = *access.last_write;
    return write_state.IsQueue(queue) && (write_state.Tag() <= tag) &&
           !write_state.IsIndex(SYNC_PRESENT_ENGINE_SYNCVAL_PRESENT_PRESENTED_SYNCVAL);
}

// Read access predicate for queue wait
bool ResourceAccessState::WaitTagPredicate::operator()(const ResourceAccessState::ReadState &read_access) const {
    return (read_access.tag <= tag) && (read_access.stage != VK_PIPELINE_STAGE_2_PRESENT_ENGINE_BIT_SYNCVAL);
}
bool ResourceAccessState::WaitTagPredicate::operator()(const ResourceAccessState &access) const {
    if (!access.last_write.has_value()) return false;
    const auto &write_state = *access.last_write;
    return (write_state.Tag() <= tag) && !write_state.IsIndex(SYNC_PRESENT_ENGINE_SYNCVAL_PRESENT_PRESENTED_SYNCVAL);
}

// Present operations only matching only the *exactly* tagged present and acquire operations
bool ResourceAccessState::WaitAcquirePredicate::operator()(const ResourceAccessState::ReadState &read_access) const {
    return (read_access.tag == acquire_tag) && (read_access.stage == VK_PIPELINE_STAGE_2_PRESENT_ENGINE_BIT_SYNCVAL);
}
bool ResourceAccessState::WaitAcquirePredicate::operator()(const ResourceAccessState &access) const {
    if (!access.last_write.has_value()) return false;
    const auto &write_state = *access.last_write;
    return (write_state.Tag() == present_tag) && write_state.IsIndex(SYNC_PRESENT_ENGINE_SYNCVAL_PRESENT_PRESENTED_SYNCVAL);
}

bool ResourceAccessState::FirstAccessInTagRange(const ResourceUsageRange &tag_range) const {
    if (!first_accesses_.size()) return false;
    const ResourceUsageRange first_access_range = {first_accesses_.front().tag, first_accesses_.back().tag + 1};
    return tag_range.intersects(first_access_range);
}

void ResourceAccessState::OffsetTag(ResourceUsageTag offset) {
    if (last_write.has_value()) last_write->OffsetTag(offset);
    for (auto &read_access : last_reads) {
        read_access.tag += offset;
    }
    for (auto &first : first_accesses_) {
        first.tag += offset;
    }
}

static const SyncStageAccessFlags kAllSyncStageAccessBits = ~SyncStageAccessFlags(0);
ResourceAccessState::ResourceAccessState()
    : last_write(),
      last_read_stages(0),
      read_execution_barriers(VK_PIPELINE_STAGE_2_NONE),
      last_reads(),
      input_attachment_read(false),
      pending_layout_transition(false),
      first_accesses_(),
      first_read_stages_(VK_PIPELINE_STAGE_2_NONE),
      first_write_layout_ordering_(),
      first_access_closed_(false) {}

// This should be just Bits or Index, but we don't have an invalid state for Index
VkPipelineStageFlags2KHR ResourceAccessState::GetReadBarriers(const SyncStageAccessFlags &usage_bit) const {
    VkPipelineStageFlags2KHR barriers = VK_PIPELINE_STAGE_2_NONE;

    for (const auto &read_access : last_reads) {
        if ((read_access.access & usage_bit).any()) {
            barriers = read_access.barriers;
            break;
        }
    }

    return barriers;
}

void ResourceAccessState::SetQueueId(QueueId id) {
    for (auto &read_access : last_reads) {
        if (read_access.queue == kQueueIdInvalid) {
            read_access.queue = id;
        }
    }
    if (last_write.has_value()) last_write->SetQueueId(id);
}

bool ResourceAccessState::IsWriteBarrierHazard(QueueId queue_id, VkPipelineStageFlags2KHR src_exec_scope,
                                               const SyncStageAccessFlags &src_access_scope) const {
    return last_write.has_value() && last_write->IsWriteBarrierHazard(queue_id, src_exec_scope, src_access_scope);
}

bool ResourceAccessState::WriteInSourceScopeOrChain(VkPipelineStageFlags2KHR src_exec_scope,
                                                    SyncStageAccessFlags src_access_scope) const {
    return last_write.has_value() && last_write->WriteInSourceScopeOrChain(src_exec_scope, src_access_scope);
}

bool ResourceAccessState::WriteInQueueSourceScopeOrChain(QueueId queue, VkPipelineStageFlags2KHR src_exec_scope,
                                                         const SyncStageAccessFlags &src_access_scope) const {
    return last_write.has_value() && last_write->WriteInQueueSourceScopeOrChain(queue, src_exec_scope, src_access_scope);
}

bool ResourceAccessState::WriteInEventScope(VkPipelineStageFlags2KHR src_exec_scope, const SyncStageAccessFlags &src_access_scope,
                                            QueueId scope_queue, ResourceUsageTag scope_tag) const {
    return last_write.has_value() && last_write->WriteInEventScope(src_exec_scope, src_access_scope, scope_queue, scope_tag);
}

// As ReadStates must be unique by stage, this is as good a sort as needed
bool operator<(const ResourceAccessState::ReadState &lhs, const ResourceAccessState::ReadState &rhs) {
    return lhs.stage < rhs.stage;
}

void ResourceAccessState::Normalize() {
    std::sort(last_reads.begin(), last_reads.end());
    ClearFirstUse();
}

void ResourceAccessState::GatherReferencedTags(ResourceUsageTagSet &used) const {
    if (last_write.has_value()) {
        used.CachedInsert(last_write->Tag());
    }

    for (const auto &read_access : last_reads) {
        used.CachedInsert(read_access.tag);
    }
}

bool ResourceAccessState::IsRAWHazard(const SyncStageAccessInfoType &usage_info) const {
    assert(IsRead(usage_info));
    // Only RAW vs. last_write if it doesn't happen-after any other read because either:
    //    * the previous reads are not hazards, and thus last_write must be visible and available to
    //      any reads that happen after.
    //    * the previous reads *are* hazards to last_write, have been reported, and if that hazard is fixed
    //      the current read will be also not be a hazard, thus reporting a hazard here adds no needed information.
    return last_write.has_value() && (0 == (read_execution_barriers & usage_info.stage_mask)) &&
           last_write->IsWriteHazard(usage_info);
}

VkPipelineStageFlags2 ResourceAccessState::GetOrderedStages(QueueId queue_id, const OrderingBarrier &ordering) const {
    // At apply queue submission order limits on the effect of ordering
    VkPipelineStageFlags2 non_qso_stages = VK_PIPELINE_STAGE_2_NONE;
    if (queue_id != kQueueIdInvalid) {
        for (const auto &read_access : last_reads) {
            if (read_access.queue != queue_id) {
                non_qso_stages |= read_access.stage;
            }
        }
    }
    // Whether the stage are in the ordering scope only matters if the current write is ordered
    const VkPipelineStageFlags2 read_stages_in_qso = last_read_stages & ~non_qso_stages;
    VkPipelineStageFlags2 ordered_stages = read_stages_in_qso & ordering.exec_scope;
    // Special input attachment handling as always (not encoded in exec_scop)
    const bool input_attachment_ordering = ordering.access_scope[SYNC_FRAGMENT_SHADER_INPUT_ATTACHMENT_READ];
    if (input_attachment_ordering && input_attachment_read) {
        // If we have an input attachment in last_reads and input attachments are ordered we all that stage
        ordered_stages |= VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT_KHR;
    }

    return ordered_stages;
}

void ResourceAccessState::UpdateFirst(const ResourceUsageTag tag, const SyncStageAccessInfoType &usage_info,
                                      SyncOrdering ordering_rule) {
    // Only record until we record a write.
    if (!first_access_closed_) {
        const bool is_read = IsRead(usage_info);
        const VkPipelineStageFlags2KHR usage_stage = is_read ? usage_info.stage_mask : 0U;
        if (0 == (usage_stage & first_read_stages_)) {
            // If this is a read we haven't seen or a write, record.
            // We always need to know what stages were found prior to write
            first_read_stages_ |= usage_stage;
            if (0 == (read_execution_barriers & usage_stage)) {
                // If this stage isn't masked then we add it (since writes map to usage_stage 0, this also records writes)
                first_accesses_.emplace_back(tag, usage_info, ordering_rule);
                first_access_closed_ = !is_read;
            }
        }
    }
}

void ResourceAccessState::TouchupFirstForLayoutTransition(ResourceUsageTag tag, const OrderingBarrier &layout_ordering) {
    // Only call this after recording an image layout transition
    assert(first_accesses_.size());
    if (first_accesses_.back().tag == tag) {
        // If this layout transition is the the first write, add the additional ordering rules that guard the ILT
        assert(first_accesses_.back().usage_info->stage_access_index == SyncStageAccessIndex::SYNC_IMAGE_LAYOUT_TRANSITION);
        first_write_layout_ordering_ = layout_ordering;
    }
}

ResourceAccessState::ReadState::ReadState(VkPipelineStageFlags2KHR stage_, SyncStageAccessFlags access_,
                                          VkPipelineStageFlags2KHR barriers_, ResourceUsageTag tag_)
    : stage(stage_),
      access(access_),
      barriers(barriers_),
      sync_stages(VK_PIPELINE_STAGE_2_NONE),
      tag(tag_),
      queue(kQueueIdInvalid),
      pending_dep_chain(VK_PIPELINE_STAGE_2_NONE) {}

void ResourceAccessState::ReadState::Set(VkPipelineStageFlags2KHR stage_, const SyncStageAccessFlags &access_,
                                         VkPipelineStageFlags2KHR barriers_, ResourceUsageTag tag_) {
    stage = stage_;
    access = access_;
    barriers = barriers_;
    sync_stages = VK_PIPELINE_STAGE_2_NONE;
    tag = tag_;
    pending_dep_chain = VK_PIPELINE_STAGE_2_NONE;  // If this is a new read, we aren't applying a barrier set.
}

// Scope test including "queue submission order" effects.  Specifically, accesses from a different queue are not
// considered to be in "queue submission order" with barriers, events, or semaphore signalling, but any barriers
// that have bee applied (via semaphore) to those accesses can be chained off of.
bool ResourceAccessState::ReadState::ReadInQueueScopeOrChain(QueueId scope_queue, VkPipelineStageFlags2 exec_scope) const {
    VkPipelineStageFlags2 effective_stages = barriers | ((scope_queue == queue) ? stage : VK_PIPELINE_STAGE_2_NONE);
    return (exec_scope & effective_stages) != 0;
}

VkPipelineStageFlags2 ResourceAccessState::ReadState::ApplyPendingBarriers() {
    barriers |= pending_dep_chain;
    pending_dep_chain = VK_PIPELINE_STAGE_2_NONE;
    return barriers;
}

ResourceAccessWriteState::ResourceAccessWriteState(const SyncStageAccessInfoType &usage_info, ResourceUsageTag tag)
    : access_(&usage_info),
      barriers_(),
      tag_(tag),
      queue_(kQueueIdInvalid),
      dependency_chain_(VK_PIPELINE_STAGE_2_NONE_KHR),
      pending_layout_ordering_(),
      pending_dep_chain_(VK_PIPELINE_STAGE_2_NONE),
      pending_barriers_() {}

bool ResourceAccessWriteState::IsWriteHazard(const SyncStageAccessInfoType &usage_info) const {
    return !barriers_[usage_info.stage_access_index];
}

bool ResourceAccessWriteState::IsOrdered(const OrderingBarrier &ordering, QueueId queue_id) const {
    assert(access_);
    return (queue_ == queue_id) && ordering.access_scope[access_->stage_access_index];
}

bool ResourceAccessWriteState::IsWriteBarrierHazard(QueueId queue_id, VkPipelineStageFlags2KHR src_exec_scope,
                                                    const SyncStageAccessFlags &src_access_scope) const {
    // Special rules for sequential ILT's
    if (IsIndex(SYNC_IMAGE_LAYOUT_TRANSITION)) {
        if (queue_id == queue_) {
            // In queue, they are implicitly ordered
            return false;
        } else {
            // In dep chain means that the ILT is *available*
            return !WriteInChain(src_exec_scope);
        }
    }
    // In dep chain means that the write is *available*.
    // Available writes are automatically made visible and can't cause hazards during transition.
    if (WriteInChain(src_exec_scope)) {
        return false;
    }
    // The write is not in chain (previous call), so need only to check if the write is in access scope.
    return !WriteInScope(src_access_scope);
}

void ResourceAccessWriteState::Set(const SyncStageAccessInfoType &usage_info, ResourceUsageTag tag) {
    access_ = &usage_info;
    barriers_.reset();
    dependency_chain_ = VK_PIPELINE_STAGE_2_NONE;
    tag_ = tag;
    queue_ = kQueueIdInvalid;
}

void ResourceAccessWriteState::MergeBarriers(const ResourceAccessWriteState &other) {
    barriers_ |= other.barriers_;
    dependency_chain_ |= other.dependency_chain_;

    pending_barriers_ |= other.pending_barriers_;
    pending_dep_chain_ |= other.pending_dep_chain_;
    pending_layout_ordering_ |= other.pending_layout_ordering_;
}

void ResourceAccessWriteState::UpdatePendingBarriers(const SyncBarrier &barrier) {
    pending_barriers_ |= barrier.dst_access_scope;
    pending_dep_chain_ |= barrier.dst_exec_scope.exec_scope;
}

void ResourceAccessWriteState::ApplyPendingBarriers() {
    dependency_chain_ |= pending_dep_chain_;
    barriers_ |= pending_barriers_;
}

void ResourceAccessWriteState::UpdatePendingLayoutOrdering(const SyncBarrier &barrier) {
    pending_layout_ordering_ |= OrderingBarrier(barrier.src_exec_scope.exec_scope, barrier.src_access_scope);
}

void ResourceAccessWriteState::ClearPending() {
    pending_dep_chain_ = VK_PIPELINE_STAGE_2_NONE;
    pending_barriers_.reset();
    pending_layout_ordering_ = OrderingBarrier();
}

void ResourceAccessWriteState::SetQueueId(QueueId id) {
    if (queue_ == kQueueIdInvalid) {
        queue_ = id;
    }
}

bool ResourceAccessWriteState::WriteInChain(VkPipelineStageFlags2KHR src_exec_scope) const {
    return 0 != (dependency_chain_ & src_exec_scope);
}

bool ResourceAccessWriteState::WriteInScope(const SyncStageAccessFlags &src_access_scope) const {
    assert(access_);
    return src_access_scope[access_->stage_access_index];
}

bool ResourceAccessWriteState::WriteInSourceScopeOrChain(VkPipelineStageFlags2KHR src_exec_scope,
                                                         SyncStageAccessFlags src_access_scope) const {
    assert(access_);
    return WriteInChain(src_exec_scope) || WriteInScope(src_access_scope);
}

bool ResourceAccessWriteState::WriteInQueueSourceScopeOrChain(QueueId queue, VkPipelineStageFlags2KHR src_exec_scope,
                                                              const SyncStageAccessFlags &src_access_scope) const {
    assert(access_);
    return WriteInChain(src_exec_scope) || ((queue == queue_) && WriteInScope(src_access_scope));
}

bool ResourceAccessWriteState::WriteInEventScope(VkPipelineStageFlags2KHR src_exec_scope,
                                                 const SyncStageAccessFlags &src_access_scope, QueueId scope_queue,
                                                 ResourceUsageTag scope_tag) const {
    // The scope logic for events is, if we're asking, the resource usage was flagged as "in the first execution scope" at
    // the time of the SetEvent, thus all we need check is whether the access is the same one (i.e. before the scope tag
    // in order to know if it's in the excecution scope
    assert(access_);
    return (tag_ < scope_tag) && WriteInQueueSourceScopeOrChain(scope_queue, src_exec_scope, src_access_scope);
}

HazardResult::HazardState::HazardState(const ResourceAccessState *access_state_, const SyncStageAccessInfoType &usage_info_,
                                       SyncHazard hazard_, const SyncStageAccessFlags &prior_, ResourceUsageTag tag_)
    : access_state(std::make_unique<const ResourceAccessState>(*access_state_)),
      recorded_access(),
      usage_index(usage_info_.stage_access_index),
      prior_access(prior_),
      tag(tag_),
      hazard(hazard_) {
    // Touchup the hazard to reflect "present as release" semantics
    // NOTE: For implementing QFO release/acquire semantics... touch up here as well
    if (access_state->IsLastWriteOp(SYNC_PRESENT_ENGINE_SYNCVAL_PRESENT_PRESENTED_SYNCVAL)) {
        if (hazard == SyncHazard::READ_AFTER_WRITE) {
            hazard = SyncHazard::READ_AFTER_PRESENT;
        } else if (hazard == SyncHazard::WRITE_AFTER_WRITE) {
            hazard = SyncHazard::WRITE_AFTER_PRESENT;
        }
    } else if (usage_index == SYNC_PRESENT_ENGINE_SYNCVAL_PRESENT_PRESENTED_SYNCVAL) {
        if (hazard == SyncHazard::WRITE_AFTER_READ) {
            hazard = SyncHazard::PRESENT_AFTER_READ;
        } else if (hazard == SyncHazard::WRITE_AFTER_WRITE) {
            hazard = SyncHazard::PRESENT_AFTER_WRITE;
        }
    }
}

SyncExecScope SyncExecScope::MakeSrc(VkQueueFlags queue_flags, VkPipelineStageFlags2KHR mask_param,
                                     const VkPipelineStageFlags2KHR disabled_feature_mask) {
    SyncExecScope result;
    result.mask_param = mask_param;
    result.expanded_mask = sync_utils::ExpandPipelineStages(mask_param, queue_flags, disabled_feature_mask);
    result.exec_scope = sync_utils::WithEarlierPipelineStages(result.expanded_mask);
    result.valid_accesses = SyncStageAccess::AccessScopeByStage(result.expanded_mask);
    return result;
}

SyncExecScope SyncExecScope::MakeDst(VkQueueFlags queue_flags, VkPipelineStageFlags2KHR mask_param) {
    SyncExecScope result;
    result.mask_param = mask_param;
    result.expanded_mask = sync_utils::ExpandPipelineStages(mask_param, queue_flags);
    result.exec_scope = sync_utils::WithLaterPipelineStages(result.expanded_mask);
    result.valid_accesses = SyncStageAccess::AccessScopeByStage(result.expanded_mask);
    return result;
}

SyncBarrier::SyncBarrier(const SyncExecScope &src, const SyncExecScope &dst)
    : src_exec_scope(src), src_access_scope(0), dst_exec_scope(dst), dst_access_scope(0) {}

SyncBarrier::SyncBarrier(const SyncExecScope &src, const SyncExecScope &dst, const SyncBarrier::AllAccess &)
    : src_exec_scope(src), src_access_scope(src.valid_accesses), dst_exec_scope(dst), dst_access_scope(dst.valid_accesses) {}

SyncBarrier::SyncBarrier(VkQueueFlags queue_flags, const VkSubpassDependency2 &subpass) {
    const auto barrier = vku::FindStructInPNextChain<VkMemoryBarrier2KHR>(subpass.pNext);
    if (barrier) {
        auto src = SyncExecScope::MakeSrc(queue_flags, barrier->srcStageMask);
        src_exec_scope = src;
        src_access_scope = SyncStageAccess::AccessScope(src.valid_accesses, barrier->srcAccessMask);

        auto dst = SyncExecScope::MakeDst(queue_flags, barrier->dstStageMask);
        dst_exec_scope = dst;
        dst_access_scope = SyncStageAccess::AccessScope(dst.valid_accesses, barrier->dstAccessMask);

    } else {
        auto src = SyncExecScope::MakeSrc(queue_flags, subpass.srcStageMask);
        src_exec_scope = src;
        src_access_scope = SyncStageAccess::AccessScope(src.valid_accesses, subpass.srcAccessMask);

        auto dst = SyncExecScope::MakeDst(queue_flags, subpass.dstStageMask);
        dst_exec_scope = dst;
        dst_access_scope = SyncStageAccess::AccessScope(dst.valid_accesses, subpass.dstAccessMask);
    }
}

SyncBarrier::SyncBarrier(const std::vector<SyncBarrier> &barriers) : SyncBarrier() {
    for (const auto &barrier : barriers) {
        Merge(barrier);
    }
}
const char *string_SyncHazard(SyncHazard hazard) {
    switch (hazard) {
        case SyncHazard::NONE:
            return "NONE";
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
        case SyncHazard::READ_AFTER_PRESENT:
            return "READ_AFTER_PRESENT";
            break;
        case SyncHazard::WRITE_AFTER_PRESENT:
            return "WRITE_AFTER_PRESENT";
            break;
        case SyncHazard::PRESENT_AFTER_WRITE:
            return "PRESENT_AFTER_WRITE";
            break;
        case SyncHazard::PRESENT_AFTER_READ:
            return "PRESENT_AFTER_READ";
            break;
        default:
            assert(0);
    }
    return "INVALID HAZARD";
}

const char *string_SyncHazardVUID(SyncHazard hazard) {
    switch (hazard) {
        case SyncHazard::NONE:
            return "SYNC-HAZARD-NONE";
            break;
        case SyncHazard::READ_AFTER_WRITE:
            return "SYNC-HAZARD-READ-AFTER-WRITE";
            break;
        case SyncHazard::WRITE_AFTER_READ:
            return "SYNC-HAZARD-WRITE-AFTER-READ";
            break;
        case SyncHazard::WRITE_AFTER_WRITE:
            return "SYNC-HAZARD-WRITE-AFTER-WRITE";
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
        case SyncHazard::READ_AFTER_PRESENT:
            return "SYNC-HAZARD-READ-AFTER-PRESENT";
            break;
        case SyncHazard::WRITE_AFTER_PRESENT:
            return "SYNC-HAZARD-WRITE-AFTER-PRESENT";
            break;
        case SyncHazard::PRESENT_AFTER_WRITE:
            return "SYNC-HAZARD-PRESENT-AFTER-WRITE";
            break;
        case SyncHazard::PRESENT_AFTER_READ:
            return "SYNC-HAZARD-PRESENT-AFTER-READ";
            break;
        default:
            assert(0);
    }
    return "SYNC-HAZARD-INVALID";
}
