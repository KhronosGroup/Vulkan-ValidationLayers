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
#include "sync/sync_access_state.h"
#include "sync/sync_stats.h"
#include "utils/sync_utils.h"
#include <vulkan/utility/vk_struct_helper.hpp>

static bool IsRead(SyncAccessIndex access) { return syncAccessReadMask[access]; }

static const std::array<OrderingBarrier, static_cast<size_t>(SyncOrdering::kNumOrderings)> kOrderingRules = {
    {{VK_PIPELINE_STAGE_2_NONE, SyncAccessFlags()},
     {kColorAttachmentExecScope, kColorAttachmentAccessScope},
     {kDepthStencilAttachmentExecScope, kDepthStencilAttachmentAccessScope},
     {kRasterAttachmentExecScope, kRasterAttachmentAccessScope}}};

const OrderingBarrier &GetOrderingRules(SyncOrdering ordering_enum) { return kOrderingRules[static_cast<size_t>(ordering_enum)]; }

HazardResult ResourceAccessState::DetectHazard(const SyncAccessInfo &usage_info) const {
    const auto &usage_stage = usage_info.stage_mask;
    if (IsRead(usage_info.access_index)) {
        if (IsRAWHazard(usage_info)) {
            return HazardResult::HazardVsPriorWrite(this, usage_info, READ_AFTER_WRITE, *last_write);
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
        if (HasReads()) {
            for (const auto &read_access : GetReads()) {
                if (IsReadHazard(usage_stage, read_access)) {
                    return HazardResult::HazardVsPriorRead(this, usage_info, WRITE_AFTER_READ, read_access);
                }
            }
        } else if (last_write.has_value() && last_write->IsWriteHazard(usage_info)) {
            // Write-After-Write check -- if we have a previous write to test against
            return HazardResult::HazardVsPriorWrite(this, usage_info, WRITE_AFTER_WRITE, *last_write);
        }
    }
    return {};
}

HazardResult ResourceAccessState::DetectMarkerHazard() const {
    // Check for special case with two consecutive marker acceses.
    // Markers specify memory dependency betweem themselves, so this is not a hazard.
    if (!HasReads() && last_write.has_value() && (last_write->flags & SyncFlag::kMarker) != 0) {
        return {};
    }

    // Go back to regular hazard detection
    const SyncAccessInfo &marker_access_info = GetAccessInfo(SYNC_COPY_TRANSFER_WRITE);
    return DetectHazard(marker_access_info);
}

HazardResult ResourceAccessState::DetectHazard(const SyncAccessInfo &usage_info, const OrderingBarrier &ordering, SyncFlags flags,
                                               QueueId queue_id) const {
    // The ordering guarantees act as barriers to the last accesses, independent of synchronization operations
    const VkPipelineStageFlagBits2 usage_stage = usage_info.stage_mask;
    const SyncAccessIndex access_index = usage_info.access_index;
    const bool input_attachment_ordering = ordering.access_scope[SYNC_FRAGMENT_SHADER_INPUT_ATTACHMENT_READ];

    if (IsRead(usage_info.access_index)) {
        // Exclude RAW if no write, or write not most "most recent" operation w.r.t. usage;
        bool is_raw_hazard = IsRAWHazard(usage_info);
        if (is_raw_hazard) {
            // NOTE: we know last_write is non-zero
            // See if the ordering rules save us from the simple RAW check above
            // First check to see if the current usage is covered by the ordering rules
            const bool usage_is_input_attachment = (access_index == SYNC_FRAGMENT_SHADER_INPUT_ATTACHMENT_READ);
            const bool usage_is_ordered =
                (input_attachment_ordering && usage_is_input_attachment) || (0 != (usage_stage & ordering.exec_scope));
            if (usage_is_ordered) {
                // Check if the most recent write is ordered.
                // Input attachment is ordered against load op but not against regular draws (requires subpass barrier).
                bool most_recent_is_ordered =
                    last_write->IsOrdered(ordering, queue_id) && (!usage_is_input_attachment || last_write->IsLoadOp());

                // If most recent write is not ordered then check if subsequent read is ordered
                if (!most_recent_is_ordered) {
                    most_recent_is_ordered = (GetOrderedStages(queue_id, ordering, flags) != 0);
                }

                is_raw_hazard = !most_recent_is_ordered;
            }
        }
        if (is_raw_hazard) {
            return HazardResult::HazardVsPriorWrite(this, usage_info, READ_AFTER_WRITE, *last_write);
        }
        return {};
    }

    if (access_index == SyncAccessIndex::SYNC_IMAGE_LAYOUT_TRANSITION) {
        // For Image layout transitions, the barrier represents the first synchronization/access scope of the layout transition
        return DetectBarrierHazard(usage_info, queue_id, ordering.exec_scope, ordering.access_scope);
    }

    // Check WAR before WAW
    const bool usage_write_is_ordered = (usage_info.access_bit & ordering.access_scope).any();
    if (HasReads()) {
        // Look for any WAR hazards outside the ordered set of stages
        VkPipelineStageFlags2 ordered_stages = VK_PIPELINE_STAGE_2_NONE;
        if (usage_write_is_ordered) {
            // If the usage is ordered, we can ignore all ordered read stages w.r.t. WAR)
            ordered_stages = GetOrderedStages(queue_id, ordering, flags);
        }
        // If we're tracking any reads that aren't ordered against the current write, got to check 'em all.
        if ((ordered_stages & last_read_stages) != last_read_stages) {
            for (const auto &read_access : GetReads()) {
                if (read_access.stage & ordered_stages) continue;  // but we can skip the ordered ones
                if (IsReadHazard(usage_stage, read_access)) {
                    return HazardResult::HazardVsPriorRead(this, usage_info, WRITE_AFTER_READ, read_access);
                }
            }
        }
        return {};
    }

    // Only check for WAW if there are no reads since last_write
    if (last_write.has_value()) {
        if (last_write->IsOrdered(ordering, queue_id) && usage_write_is_ordered) {
            return {};
        }

        // Special case: marker accesses define memory dependency betweem themsevles
        if ((last_write->flags & SyncFlag::kMarker) != 0 && (flags & SyncFlag::kMarker) != 0) {
            return {};
        }

        // ILT after ILT is a special case where we check the 2nd access scope of the first ILT against the first access
        // scope of the second ILT, which has been passed (smuggled?) in the ordering barrier
        bool ilt_ilt_hazard = false;
        if (access_index == SYNC_IMAGE_LAYOUT_TRANSITION && last_write->access_index == SYNC_IMAGE_LAYOUT_TRANSITION) {
            ilt_ilt_hazard = !(last_write->barriers & ordering.access_scope).any();
        }

        if (ilt_ilt_hazard || last_write->IsWriteHazard(usage_info)) {
            return HazardResult::HazardVsPriorWrite(this, usage_info, WRITE_AFTER_WRITE, *last_write);
        }
    }
    return {};
}

HazardResult ResourceAccessState::DetectHazard(const ResourceAccessState &recorded_use, QueueId queue_id,
                                               const ResourceUsageRange &tag_range) const {
    HazardResult hazard;
    const auto &recorded_accesses = recorded_use.first_accesses_;
    uint32_t count = recorded_accesses.size();
    if (count) {
        // First access is only closed if the last is a write
        bool do_write_last = recorded_use.first_access_closed_;
        if (do_write_last) {
            // Note: We know count > 0 so this is alway safe.
            --count;
        }

        for (uint32_t i = 0; i < count; ++i) {
            const auto &first = recorded_accesses[i];
            // Skip and quit logic
            if (first.tag < tag_range.begin) continue;
            if (first.tag >= tag_range.end) {
                do_write_last = false;  // ignore last since we know it can't be in tag_range
                break;
            }

            const auto &first_ordering = GetOrderingRules(first.ordering_rule);
            hazard = DetectHazard(*first.usage_info, first_ordering, 0, queue_id);
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
                if (last_access.usage_info->access_index == SyncAccessIndex::SYNC_IMAGE_LAYOUT_TRANSITION) {
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
                    // This suppresses only RAW checks for stages present in the recorded context, but not those only present in the
                    // active context.
                    barrier.exec_scope |= recorded_use.first_read_stages_;
                    // if there are any first use reads, we suppress WAW by injecting the active context write in the ordering rule
                    barrier.access_scope |= last_access.usage_info->access_bit;
                }
                hazard = DetectHazard(*last_access.usage_info, barrier, last_access.flags, queue_id);
                if (hazard.IsHazard()) {
                    hazard.AddRecordedAccess(last_access);
                }
            }
        }
    }
    return hazard;
}

// Asynchronous Hazards occur between subpasses with no connection through the DAG
HazardResult ResourceAccessState::DetectAsyncHazard(const SyncAccessInfo &usage_info, const ResourceUsageTag start_tag,
                                                    QueueId queue_id) const {
    // Async checks need to not go back further than the start of the subpass, as we only want to find hazards between the async
    // subpasses.  Anything older than that should have been checked at the start of each subpass, taking into account all of
    // the raster ordering rules.
    if (IsRead(usage_info.access_index)) {
        if (last_write.has_value() && last_write->queue == queue_id && (last_write->tag >= start_tag)) {
            return HazardResult::HazardVsPriorWrite(this, usage_info, READ_RACING_WRITE, *last_write);
        }
    } else {
        if (last_write.has_value() && last_write->queue == queue_id && (last_write->tag >= start_tag)) {
            return HazardResult::HazardVsPriorWrite(this, usage_info, WRITE_RACING_WRITE, *last_write);
        } else if (HasReads()) {
            // Any reads during the other subpass will conflict with this write, so we need to check them all.
            for (const auto &read_access : GetReads()) {
                if (read_access.queue == queue_id && read_access.tag >= start_tag) {
                    return HazardResult::HazardVsPriorRead(this, usage_info, WRITE_RACING_READ, read_access);
                }
            }
        }
    }
    return {};
}

HazardResult ResourceAccessState::DetectAsyncHazard(const ResourceAccessState &recorded_use, const ResourceUsageRange &tag_range,
                                                    ResourceUsageTag start_tag, QueueId queue_id) const {
    for (const auto &first : recorded_use.first_accesses_) {
        // Skip and quit logic
        if (first.tag < tag_range.begin) continue;
        if (first.tag >= tag_range.end) break;

        HazardResult hazard = DetectAsyncHazard(*first.usage_info, start_tag, queue_id);
        if (hazard.IsHazard()) {
            hazard.AddRecordedAccess(first);
            return hazard;
        }
    }
    return {};
}

HazardResult ResourceAccessState::DetectBarrierHazard(const SyncAccessInfo &usage_info, QueueId queue_id,
                                                      VkPipelineStageFlags2 src_exec_scope,
                                                      const SyncAccessFlags &src_access_scope) const {
    // Only supporting image layout transitions for now
    assert(usage_info.access_index == SyncAccessIndex::SYNC_IMAGE_LAYOUT_TRANSITION);

    // only test for WAW if there no intervening read operations.
    // See DetectHazard(SyncStagetAccessIndex) above for more details.
    if (HasReads()) {
        // Look at the reads if any
        for (const auto &read_access : GetReads()) {
            if (read_access.IsReadBarrierHazard(queue_id, src_exec_scope, src_access_scope)) {
                return HazardResult::HazardVsPriorRead(this, usage_info, WRITE_AFTER_READ, read_access);
            }
        }
    } else if (last_write.has_value() && IsWriteBarrierHazard(queue_id, src_exec_scope, src_access_scope)) {
        return HazardResult::HazardVsPriorWrite(this, usage_info, WRITE_AFTER_WRITE, *last_write);
    }
    return {};
}

HazardResult ResourceAccessState::DetectBarrierHazard(const SyncAccessInfo &usage_info, const ResourceAccessState &scope_state,
                                                      VkPipelineStageFlags2 src_exec_scope, const SyncAccessFlags &src_access_scope,
                                                      QueueId event_queue, ResourceUsageTag event_tag) const {
    // Only supporting image layout transitions for now
    assert(usage_info.access_index == SyncAccessIndex::SYNC_IMAGE_LAYOUT_TRANSITION);

    if (last_write.has_value() && (last_write->tag >= event_tag)) {
        // Any write after the event precludes the possibility of being in the first access scope for the layout transition
        return HazardResult::HazardVsPriorWrite(this, usage_info, WRITE_AFTER_WRITE, *last_write);
    } else {
        // only test for WAW if there no intervening read operations.
        // See DetectHazard(SyncStagetAccessIndex) above for more details.
        if (HasReads()) {
            // Look at the reads if any... if reads exist, they are either the reason the access is in the event
            // first scope, or they are a hazard.
            const uint32_t scope_read_count = scope_state.last_read_count;
            // Since the hasn't been a write:
            //  * The current read state is a superset of the scoped one
            //  * The stage order is the same.
            assert(last_read_count >= scope_read_count);
            for (uint32_t read_idx = 0; read_idx < scope_read_count; ++read_idx) {
                const ReadState &scope_read = scope_state.last_reads[read_idx];
                const ReadState &current_read = last_reads[read_idx];
                assert(scope_read.stage == current_read.stage);
                if (current_read.tag > event_tag) {
                    // The read is more recent than the set event scope, thus no barrier from the wait/ILT.
                    return HazardResult::HazardVsPriorRead(this, usage_info, WRITE_AFTER_READ, current_read);
                } else {
                    // The read is in the events first synchronization scope, so we use a barrier hazard check
                    // If the read stage is not in the src sync scope
                    // *AND* not execution chained with an existing sync barrier (that's the or)
                    // then the barrier access is unsafe (R/W after R)
                    if (scope_read.IsReadBarrierHazard(event_queue, src_exec_scope, src_access_scope)) {
                        return HazardResult::HazardVsPriorRead(this, usage_info, WRITE_AFTER_READ, scope_read);
                    }
                }
            }
            if (last_read_count > scope_read_count) {
                const ReadState &current_read = last_reads[scope_read_count];
                return HazardResult::HazardVsPriorRead(this, usage_info, WRITE_AFTER_READ, current_read);
            }
        } else if (last_write.has_value()) {
            // if there are no reads, the write is either the reason the access is in the event scope... they are a hazard
            // The write is in the first sync scope of the event (sync their aren't any reads to be the reason)
            // So do a normal barrier hazard check
            if (scope_state.IsWriteBarrierHazard(event_queue, src_exec_scope, src_access_scope)) {
                return HazardResult::HazardVsPriorWrite(&scope_state, usage_info, WRITE_AFTER_WRITE, *scope_state.last_write);
            }
        }
    }
    return {};
}

void ResourceAccessState::AddRead(const ReadState &read) {
    if (last_read_count == 0) {
        single_last_read = read;
        last_reads = &single_last_read;
        last_read_count = 1;
    } else {  // last_read_count > 0
        auto new_reads = new ReadState[last_read_count + 1];
        std::memcpy(new_reads, last_reads, last_read_count * sizeof(ReadState));
        if (last_read_count > 1) {
            delete[] last_reads;
        }
        new_reads[last_read_count] = read;
        last_reads = new_reads;
        last_read_count++;
    }
}

void ResourceAccessState::MergeReads(const ResourceAccessState &other) {
    // Merge the read states
    const uint32_t pre_merge_count = last_read_count;
    const auto pre_merge_stages = last_read_stages;
    for (uint32_t other_read_index = 0; other_read_index < other.last_read_count; other_read_index++) {
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
                        my_read.access_index = other_read.access_index;
                        my_read.tag = other_read.tag;
                        my_read.handle_index = other_read.handle_index;
                        my_read.queue = other_read.queue;
                        // TODO: Phase 2 -- review the state merge logic to avoid false positive from overwriting the barriers
                        //                  May require tracking more than one access per stage.
                        my_read.barriers = other_read.barriers;
                        my_read.sync_stages = other_read.sync_stages;
                        if (my_read.stage == VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT) {
                            // Since I'm overwriting the fragement stage read, also update the input attachment info
                            // as this is the only stage that affects it.
                            input_attachment_read = other.input_attachment_read;
                        }
                    } else if (other_read.tag == my_read.tag) {
                        // The read tags match so merge the barriers
                        my_read.barriers |= other_read.barriers;
                        my_read.sync_stages |= other_read.sync_stages;
                    }

                    break;
                }
            }
        } else {
            // The other read stage doesn't exist in this, so add it.
            AddRead(other_read);
            last_read_stages |= other_read.stage;
            if (other_read.stage == VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT) {
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
            if (last_write->tag < other.last_write->tag) {
                // NOTE: Both last and other have writes, and thus first access is "closed". We are selecting other's
                //       first_access state, but it and this can only differ if there are async hazards
                //       error state.
                //
                // If this is a later write, we've reported any exsiting hazard, and we can just overwrite as the more recent
                // operation
                *this = other;
                skip_first = true;
            } else if (last_write->tag == other.last_write->tag) {
                // In the *equals* case for write operations, we merged the write barriers and the read state (but without the
                // dependency chaining logic or any stage expansion)
                last_write->MergeBarriers(*other.last_write);
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
        MergeReads(other);
    }

    // Merge first access information by merging this and other first accesses (similar to how merge sort works)
    if (!skip_first && !(first_accesses_ == other.first_accesses_) && !other.first_accesses_.empty()) {
        FirstAccesses firsts(std::move(first_accesses_));
        const OrderingBarrier this_first_write_layout_ordering = first_write_layout_ordering_;

        // Select layout transition barrier from the write that goes first (the later write will be
        // ignored since the first access gets closed after the first write).
        const bool resolve_to_this_layout_ordering =
            !other.first_access_closed_ || (first_access_closed_ && firsts.back().tag < other.first_accesses_.back().tag);

        ClearFirstUse();

        first_write_layout_ordering_ =
            resolve_to_this_layout_ordering ? this_first_write_layout_ordering : other.first_write_layout_ordering_;

        auto a = firsts.begin();
        auto a_end = firsts.end();
        for (auto &b : other.first_accesses_) {
            // TODO: Determine whether some tag offset will be needed for PHASE II
            while ((a != a_end) && (a->tag < b.tag)) {
                UpdateFirst(a->TagEx(), *a->usage_info, a->ordering_rule, a->flags);
                ++a;
            }
            UpdateFirst(b.TagEx(), *b.usage_info, b.ordering_rule, b.flags);
        }
        for (; a != a_end; ++a) {
            UpdateFirst(a->TagEx(), *a->usage_info, a->ordering_rule, a->flags);
        }
    }
}

void ResourceAccessState::Update(const SyncAccessInfo &usage_info, SyncOrdering ordering_rule, ResourceUsageTagEx tag_ex,
                                 SyncFlags flags) {
    const VkPipelineStageFlagBits2 usage_stage = usage_info.stage_mask;
    if (IsRead(usage_info.access_index)) {
        // Mulitple outstanding reads may be of interest and do dependency chains independently
        // However, for purposes of barrier tracking, only one read per pipeline stage matters
        if (usage_stage & last_read_stages) {
            const auto not_usage_stage = ~usage_stage;
            for (auto &read_access : GetReads()) {
                if (read_access.stage == usage_stage) {
                    read_access.Set(usage_stage, usage_info.access_index, tag_ex);
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
            for (auto &read_access : GetReads()) {
                if (read_access.barriers & usage_stage) {
                    read_access.sync_stages |= usage_stage;
                }
            }
            ReadState new_read_state;
            new_read_state.Set(usage_stage, usage_info.access_index, tag_ex);
            AddRead(new_read_state);
            last_read_stages |= usage_stage;
        }

        // Fragment shader reads come in two flavors, and we need to track if the one we're tracking is the special one.
        if (usage_stage == VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT) {
            // TODO Revisit re: multiple reads for a given stage
            input_attachment_read = (usage_info.access_index == SYNC_FRAGMENT_SHADER_INPUT_ATTACHMENT_READ);
        }
    } else {
        // Assume write
        // TODO determine what to do with READ-WRITE operations if any
        SetWrite(usage_info.access_index, tag_ex, flags);
    }
    UpdateFirst(tag_ex, usage_info, ordering_rule, flags);
}

HazardResult HazardResult::HazardVsPriorWrite(const ResourceAccessState *access_state, const SyncAccessInfo &usage_info,
                                              SyncHazard hazard, const WriteState &prior_write) {
    HazardResult result;
    result.state_.emplace(access_state, usage_info, hazard, prior_write.access_index, prior_write.TagEx());
    return result;
}

HazardResult HazardResult::HazardVsPriorRead(const ResourceAccessState *access_state, const SyncAccessInfo &usage_info,
                                             SyncHazard hazard, const ReadState &prior_read) {
    assert(prior_read.access_index != SYNC_ACCESS_INDEX_NONE);
    HazardResult result;
    result.state_.emplace(access_state, usage_info, hazard, prior_read.access_index, prior_read.TagEx());
    return result;
}

void HazardResult::AddRecordedAccess(const ResourceFirstAccess &first_access) {
    assert(state_.has_value());
    state_->recorded_access = std::make_unique<const ResourceFirstAccess>(first_access);
}
bool HazardResult::IsWAWHazard() const {
    assert(state_.has_value());
    assert(state_->prior_access_index != SYNC_ACCESS_INDEX_NONE);
    return (state_->hazard == WRITE_AFTER_WRITE) && (state_->prior_access_index == state_->access_index);
}

// Clobber last read and all barriers... because all we have is DANGER, DANGER, WILL ROBINSON!!!
// if the last_reads/last_write were unsafe, we've reported them, in either case the prior access is irrelevant.
// We can overwrite them as *this* write is now after them.
void ResourceAccessState::SetWrite(SyncAccessIndex access_index, ResourceUsageTagEx tag_ex, SyncFlags flags) {
    ClearRead();
    if (!last_write.has_value()) {
        last_write.emplace();
    }
    last_write->Set(access_index, tag_ex, flags);
}

void ResourceAccessState::ClearWrite() { last_write.reset(); }

void ResourceAccessState::ClearReadStates() {
    if (last_read_count > 1) {
        delete[] last_reads;
    }
    last_reads = nullptr;
    last_read_count = 0;
}

void ResourceAccessState::ClearRead() {
    ClearReadStates();
    last_read_stages = VK_PIPELINE_STAGE_2_NONE;
    read_execution_barriers = VK_PIPELINE_STAGE_2_NONE;
    input_attachment_read = false;  // Denotes no outstanding input attachment read after the last write.
}

void ResourceAccessState::ClearFirstUse() {
    first_accesses_.clear();
    first_read_stages_ = VK_PIPELINE_STAGE_2_NONE;
    first_write_layout_ordering_ = OrderingBarrier();
    first_access_closed_ = false;
}

void ResourceAccessState::ApplyBarrier(const BarrierScope &barrier_scope, const SyncBarrier &barrier, bool layout_transition,
                                       uint32_t layout_transition_handle_index, ResourceUsageTag layout_transition_tag) {
    // Dedicated layout transition barrier logic
    if (layout_transition) {
        const SyncAccessInfo &layout_transition_access_info = GetAccessInfo(SYNC_IMAGE_LAYOUT_TRANSITION);
        const ResourceUsageTagEx tag_ex = ResourceUsageTagEx{layout_transition_tag, layout_transition_handle_index};
        const OrderingBarrier layout_ordering(barrier.src_exec_scope.exec_scope, barrier.src_access_scope);

        // Register write access that models layout transition writes
        SetWrite(SYNC_IMAGE_LAYOUT_TRANSITION, tag_ex);
        UpdateFirst(tag_ex, layout_transition_access_info, SyncOrdering::kNonAttachment);
        TouchupFirstForLayoutTransition(layout_transition_tag, layout_ordering);

        last_write->barriers |= barrier.dst_access_scope;
        last_write->dependency_chain |= barrier.dst_exec_scope.exec_scope;
        return;
    }

    // Apply barriers over write access
    if (last_write.has_value() && last_write->InBarrierSourceScope(barrier_scope)) {
        last_write->barriers |= barrier.dst_access_scope;
        last_write->dependency_chain |= barrier.dst_exec_scope.exec_scope;
    }
    // Apply barriers over read accesses
    VkPipelineStageFlags2 stages_in_scope = VK_PIPELINE_STAGE_2_NONE;
    for (ReadState &read_access : GetReads()) {
        // The | implements the "dependency chain" logic for this access,
        // as the barriers field stores the second sync scope
        if (read_access.InBarrierSourceScope(barrier_scope)) {
            // We will apply the barrier in the next loop to have this in one place
            stages_in_scope |= read_access.stage;
        }
    }
    for (ReadState &read_access : GetReads()) {
        if ((read_access.stage | read_access.sync_stages) & stages_in_scope) {
            // If this stage, or any stage known to be synchronized after it are in scope, apply the barrier to this read.
            // NOTE: Forwarding barriers to known prior stages changes the sync_stages from shallow to deep, because the
            // barriers used to determine sync_stages have been propagated to all known earlier stages
            read_access.barriers |= barrier.dst_exec_scope.exec_scope;
            read_execution_barriers |= barrier.dst_exec_scope.exec_scope;
        }
    }
}

void ResourceAccessState::CollectPendingBarriers(const BarrierScope &barrier_scope, const SyncBarrier &barrier,
                                                 bool layout_transition, uint32_t layout_transition_handle_index,
                                                 PendingBarriers &pending_barriers) {
    if (layout_transition) {
        // Schedule layout transition first: layout transition creates WriteState if necessary
        pending_barriers.AddLayoutTransition(this, barrier, layout_transition_handle_index);
        // Apply barrier over layout trasition's write access
        pending_barriers.AddWriteBarrier(this, barrier);
        return;
    }

    // Collect barriers over write accesses
    if (last_write.has_value() && last_write->InBarrierSourceScope(barrier_scope)) {
        pending_barriers.AddWriteBarrier(this, barrier);
    }

    // Collect barriers over read accesses
    VkPipelineStageFlags2 stages_in_scope = VK_PIPELINE_STAGE_2_NONE;
    for (ReadState &read_access : GetReads()) {
        // The | implements the "dependency chain" logic for this access,
        // as the barriers field stores the second sync scope
        if (read_access.InBarrierSourceScope(barrier_scope)) {
            // We will apply the barrier in the next loop to have this in one place
            stages_in_scope |= read_access.stage;
        }
    }
    for (ReadState &read_access : GetReads()) {
        if ((read_access.stage | read_access.sync_stages) & stages_in_scope) {
            // If this stage, or any stage known to be synchronized after it are in scope, apply the barrier to this read.
            // NOTE: Forwarding barriers to known prior stages changes the sync_stages from shallow to deep, because the
            // barriers used to determine sync_stages have been propagated to all known earlier stages
            pending_barriers.AddReadBarrier(this, (uint32_t)(&read_access - last_reads), barrier);
        }
    }
}

void PendingBarriers::AddReadBarrier(ResourceAccessState *access_state, uint32_t last_reads_index, const SyncBarrier &barrier) {
    size_t barrier_index = 0;
    for (; barrier_index < read_barriers.size(); barrier_index++) {
        const PendingReadBarrier &pending = read_barriers[barrier_index];
        if (pending.barriers == barrier.dst_exec_scope.exec_scope && pending.last_reads_index == last_reads_index) {
            break;
        }
    }
    if (barrier_index == read_barriers.size()) {
        PendingReadBarrier &pending = read_barriers.emplace_back();
        pending.barriers = barrier.dst_exec_scope.exec_scope;
        pending.last_reads_index = last_reads_index;
    }
    PendingBarrierInfo &info = infos.emplace_back();
    info.type = PendingBarrierType::ReadAccessBarrier;
    info.index = (uint32_t)barrier_index;
    info.access_state = access_state;
}

void PendingBarriers::AddWriteBarrier(ResourceAccessState *access_state, const SyncBarrier &barrier) {
    size_t barrier_index = 0;
    for (; barrier_index < write_barriers.size(); barrier_index++) {
        const PendingWriteBarrier &pending = write_barriers[barrier_index];
        if (pending.barriers == barrier.dst_access_scope && pending.dependency_chain == barrier.dst_exec_scope.exec_scope) {
            break;
        }
    }
    if (barrier_index == write_barriers.size()) {
        PendingWriteBarrier &pending = write_barriers.emplace_back();
        pending.barriers = barrier.dst_access_scope;
        pending.dependency_chain = barrier.dst_exec_scope.exec_scope;
    }
    PendingBarrierInfo &info = infos.emplace_back();
    info.type = PendingBarrierType::WriteAccessBarrier;
    info.index = (uint32_t)barrier_index;
    info.access_state = access_state;
}

void PendingBarriers::AddLayoutTransition(ResourceAccessState *access_state, const SyncBarrier &barrier,
                                          uint32_t layout_transition_handle_index) {
    // NOTE: in contrast to read/write barriers, we don't do reuse search here,
    // mostly because we didn't see a beneficial use case yet.
    // Storing handle index can be a hint it would be harder to find duplicates.
    PendingBarrierInfo &info = infos.emplace_back();
    info.type = PendingBarrierType::LayoutTransition;
    info.index = (uint32_t)layout_transitions.size();
    info.access_state = access_state;

    PendingLayoutTransition &layout_transition = layout_transitions.emplace_back();
    layout_transition.ordering = OrderingBarrier(barrier.src_exec_scope.exec_scope, barrier.src_access_scope);
    layout_transition.handle_index = layout_transition_handle_index;
}

void PendingBarriers::Apply(const ResourceUsageTag exec_tag) {
    for (const PendingBarrierInfo &info : infos) {
        if (info.type == PendingBarrierType::ReadAccessBarrier) {
            const PendingReadBarrier &read_barrier = read_barriers[info.index];
            info.access_state->ApplyPendingReadBarrier(read_barrier, exec_tag);
        } else if (info.type == PendingBarrierType::WriteAccessBarrier) {
            const PendingWriteBarrier &write_barrier = write_barriers[info.index];
            info.access_state->ApplyPendingWriteBarrier(write_barrier);
        } else {
            assert(info.type == PendingBarrierType::LayoutTransition);
            const PendingLayoutTransition &layout_transition = layout_transitions[info.index];
            info.access_state->ApplyPendingLayoutTransition(layout_transition, exec_tag);
        }
    }
}

void ApplyBarriers(ResourceAccessState &access_state, const std::vector<SyncBarrier> &barriers, bool layout_transition,
                   ResourceUsageTag layout_transition_tag) {
    // The common case of a single barrier.
    // The pending barrier helper is unnecessary because there are no independent barriers to track.
    // The barrier can be applied directly to the access state.
    if (barriers.size() == 1) {
        access_state.ApplyBarrier(BarrierScope(barriers[0]), barriers[0], layout_transition, vvl::kNoIndex32,
                                  layout_transition_tag);
        return;
    }

    // There are multiple barriers. We can't apply them sequentially because they can form dependencies
    // between themselves (result of the previous barrier might affect application of the next barrier).
    // The APIs we are dealing require that the barriers in a set of barriers are applied independently.
    // That's the intended use case of PendingBarriers helper.
    PendingBarriers pending_barriers;
    for (const SyncBarrier &barrier : barriers) {
        access_state.CollectPendingBarriers(BarrierScope(barrier), barrier, layout_transition, vvl::kNoIndex32, pending_barriers);
    }
    pending_barriers.Apply(layout_transition_tag);
}

BarrierScope::BarrierScope(const SyncBarrier &barrier, QueueId scope_queue, ResourceUsageTag scope_tag)
    : src_exec_scope(barrier.src_exec_scope.exec_scope),
      src_access_scope(barrier.src_access_scope),
      scope_queue(scope_queue),
      scope_tag(scope_tag) {}

void ResourceAccessState::ApplyPendingReadBarrier(const PendingReadBarrier &read_barrier, ResourceUsageTag tag) {
    // Do not register read barriers if layout transition has been registered for the same barrier API command.
    // The layout transition resets the read state (if any) and sets a write instead. By definition of our
    // implementation the read barriers are the barriers we apply to read accesses, so without read accesses we
    // don't need read barriers.
    if (last_write.has_value() && last_write->tag == tag && last_write->access_index == SYNC_IMAGE_LAYOUT_TRANSITION) {
        return;
    }

    ReadState &read_state = last_reads[read_barrier.last_reads_index];
    read_state.barriers |= read_barrier.barriers;
    read_execution_barriers |= read_barrier.barriers;
}

void ResourceAccessState::ApplyPendingWriteBarrier(const PendingWriteBarrier &write_barrier) {
    if (last_write.has_value()) {
        last_write->dependency_chain |= write_barrier.dependency_chain;
        last_write->barriers |= write_barrier.barriers;
    }
}

void ResourceAccessState::ApplyPendingLayoutTransition(const PendingLayoutTransition &layout_transition, ResourceUsageTag tag) {
    const SyncAccessInfo &layout_usage_info = GetAccessInfo(SYNC_IMAGE_LAYOUT_TRANSITION);
    const ResourceUsageTagEx tag_ex = ResourceUsageTagEx{tag, layout_transition.handle_index};
    SetWrite(SYNC_IMAGE_LAYOUT_TRANSITION, tag_ex);
    UpdateFirst(tag_ex, layout_usage_info, SyncOrdering::kNonAttachment);
    TouchupFirstForLayoutTransition(tag, layout_transition.ordering);
}

// Assumes signal queue != wait queue
void ResourceAccessState::ApplySemaphore(const SemaphoreScope &signal, const SemaphoreScope wait) {
    // Semaphores only guarantee the first scope of the signal is before the second scope of the wait.
    // If any access isn't in the first scope, there are no guarantees, thus those barriers are cleared
    assert(signal.queue != wait.queue);
    for (auto &read_access : GetReads()) {
        if (read_access.ReadOrDependencyChainInSourceScope(signal.queue, signal.exec_scope)) {
            // Deflects WAR on wait queue
            read_access.barriers = wait.exec_scope;
        } else {
            // Leave sync stages alone. Update method will clear unsynchronized stages on subsequent reads as needed.
            read_access.barriers = VK_PIPELINE_STAGE_2_NONE;
        }
    }
    if (last_write.has_value() &&
        last_write->WriteOrDependencyChainInSourceScope(signal.queue, signal.exec_scope, signal.valid_accesses)) {
        // Will deflect RAW wait queue, WAW needs a chained barrier on wait queue
        read_execution_barriers = wait.exec_scope;
        last_write->barriers = wait.valid_accesses;
    } else {
        read_execution_barriers = VK_PIPELINE_STAGE_2_NONE;
        if (last_write.has_value()) last_write->barriers.reset();
    }
    if (last_write.has_value()) last_write->dependency_chain = read_execution_barriers;
}

// Read access predicate for queue wait
bool ResourceAccessState::WaitQueueTagPredicate::operator()(const ReadState &read_access) const {
    return (read_access.queue == queue) && (read_access.tag <= tag) &&
           (read_access.stage != VK_PIPELINE_STAGE_2_PRESENT_ENGINE_BIT_SYNCVAL);
}
bool ResourceAccessState::WaitQueueTagPredicate::operator()(const ResourceAccessState &access) const {
    if (!access.last_write.has_value()) return false;
    const auto &write_state = *access.last_write;
    return write_state.queue == queue && (write_state.tag <= tag) &&
           write_state.access_index != SYNC_PRESENT_ENGINE_SYNCVAL_PRESENT_PRESENTED_SYNCVAL;
}

// Read access predicate for queue wait
bool ResourceAccessState::WaitTagPredicate::operator()(const ReadState &read_access) const {
    return (read_access.tag <= tag) && (read_access.stage != VK_PIPELINE_STAGE_2_PRESENT_ENGINE_BIT_SYNCVAL);
}
bool ResourceAccessState::WaitTagPredicate::operator()(const ResourceAccessState &access) const {
    if (!access.last_write.has_value()) return false;
    const auto &write_state = *access.last_write;
    return (write_state.tag <= tag) && write_state.access_index != SYNC_PRESENT_ENGINE_SYNCVAL_PRESENT_PRESENTED_SYNCVAL;
}

// Present operations only matching only the *exactly* tagged present and acquire operations
bool ResourceAccessState::WaitAcquirePredicate::operator()(const ReadState &read_access) const {
    return (read_access.tag == acquire_tag) && (read_access.stage == VK_PIPELINE_STAGE_2_PRESENT_ENGINE_BIT_SYNCVAL);
}
bool ResourceAccessState::WaitAcquirePredicate::operator()(const ResourceAccessState &access) const {
    if (!access.last_write.has_value()) return false;
    const auto &write_state = *access.last_write;
    return (write_state.tag == present_tag) && write_state.access_index == SYNC_PRESENT_ENGINE_SYNCVAL_PRESENT_PRESENTED_SYNCVAL;
}

bool ResourceAccessState::FirstAccessInTagRange(const ResourceUsageRange &tag_range) const {
    if (!first_accesses_.size()) return false;
    const ResourceUsageRange first_access_range = {first_accesses_.front().tag, first_accesses_.back().tag + 1};
    return tag_range.intersects(first_access_range);
}

void ResourceAccessState::OffsetTag(ResourceUsageTag offset) {
    if (last_write.has_value()) {
        last_write->tag += offset;
    }
    for (auto &read_access : GetReads()) {
        read_access.tag += offset;
    }
    for (auto &first : first_accesses_) {
        first.tag += offset;
    }
}

// Copies everything except read states which need custom logic
void ResourceAccessState::CopySimpleMembers(const ResourceAccessState &other) {
    last_write = other.last_write;

    last_read_stages = other.last_read_stages;
    read_execution_barriers = other.read_execution_barriers;

    first_accesses_ = other.first_accesses_;
    first_read_stages_ = other.first_read_stages_;
    first_write_layout_ordering_ = other.first_write_layout_ordering_;
    first_access_closed_ = other.first_access_closed_;

    input_attachment_read = other.input_attachment_read;
}

ResourceAccessState::ResourceAccessState(const ResourceAccessState &other) { *this = other; }

ResourceAccessState &ResourceAccessState::operator=(const ResourceAccessState &other) {
    CopySimpleMembers(other);
    ClearReadStates();
    for (const ReadState &read : other.GetReads()) {
        AddRead(read);
    }
    return *this;
}

ResourceAccessState::ResourceAccessState(ResourceAccessState &&other) { *this = std::move(other); }

ResourceAccessState &ResourceAccessState::operator=(ResourceAccessState &&other) {
    CopySimpleMembers(other);

    last_read_count = other.last_read_count;
    single_last_read = other.single_last_read;

    if (other.last_read_count == 1) {
        last_reads = &single_last_read;
    } else {
        last_reads = other.last_reads;
    }
    other.last_reads = nullptr;
    other.last_read_count = 0;
    return *this;
}

ResourceAccessState ::~ResourceAccessState() {
    ClearReadStates();  // free allocated memory for multi-read access state
}

VkPipelineStageFlags2 ResourceAccessState::GetReadBarriers(SyncAccessIndex access_index) const {
    for (const auto &read_access : GetReads()) {
        if (read_access.access_index == access_index) {
            return read_access.barriers;
        }
    }
    return VK_PIPELINE_STAGE_2_NONE;
}

void ResourceAccessState::SetQueueId(QueueId id) {
    for (auto &read_access : GetReads()) {
        if (read_access.queue == kQueueIdInvalid) {
            read_access.queue = id;
        }
    }
    if (last_write.has_value()) {
        last_write->SetQueueId(id);
    }
}

bool ResourceAccessState::IsWriteBarrierHazard(QueueId queue_id, VkPipelineStageFlags2 src_exec_scope,
                                               const SyncAccessFlags &src_access_scope) const {
    return last_write.has_value() && last_write->IsWriteBarrierHazard(queue_id, src_exec_scope, src_access_scope);
}

// As ReadStates must be unique by stage, this is as good a sort as needed
bool operator<(const ReadState &lhs, const ReadState &rhs) { return lhs.stage < rhs.stage; }

void ResourceAccessState::Normalize() {
    std::sort(last_reads, last_reads + last_read_count);
    ClearFirstUse();
}

void ResourceAccessState::GatherReferencedTags(ResourceUsageTagSet &used) const {
    if (last_write.has_value()) {
        used.CachedInsert(last_write->tag);
    }

    for (const auto &read_access : GetReads()) {
        used.CachedInsert(read_access.tag);
    }
}

const WriteState &ResourceAccessState::LastWrite() const {
    assert(last_write.has_value());
    return *last_write;
}

void ResourceAccessState::UpdateStats(syncval_stats::AccessContextStats &stats) const {
#if VVL_ENABLE_SYNCVAL_STATS != 0
    stats.read_states += (uint32_t)last_reads.size();
    stats.write_states += last_write.has_value();
    stats.access_states_with_multiple_reads += (last_reads.size() > 1);

    bool is_dynamic_allocation = false;
    // check if last reads allocate
    if (last_reads.size() > last_reads.kSmallCapacity) {
        stats.access_states_dynamic_allocation_size += uint64_t(sizeof(ReadState) * last_reads.size());
        is_dynamic_allocation = true;
    }
    // check if first accesses allocate
    if (first_accesses_.size() > first_accesses_.kSmallCapacity) {
        stats.access_states_dynamic_allocation_size += uint64_t(sizeof(ResourceFirstAccess) * first_accesses_.size());
        is_dynamic_allocation = true;
    }
    stats.access_states_with_dynamic_allocations += is_dynamic_allocation;
    stats.max_first_accesses_size = std::max(stats.max_first_accesses_size, (uint32_t)first_accesses_.size());
#endif
}

bool ResourceAccessState::IsRAWHazard(const SyncAccessInfo &usage_info) const {
    assert(IsRead(usage_info.access_index));
    // Only RAW vs. last_write if it doesn't happen-after any other read because either:
    //    * the previous reads are not hazards, and thus last_write must be visible and available to
    //      any reads that happen after.
    //    * the previous reads *are* hazards to last_write, have been reported, and if that hazard is fixed
    //      the current read will be also not be a hazard, thus reporting a hazard here adds no needed information.
    return last_write.has_value() && (0 == (read_execution_barriers & usage_info.stage_mask)) &&
           last_write->IsWriteHazard(usage_info);
}

VkPipelineStageFlags2 ResourceAccessState::GetOrderedStages(QueueId queue_id, const OrderingBarrier &ordering, SyncFlags flags) const {
    // At apply queue submission order limits on the effect of ordering
    VkPipelineStageFlags2 non_qso_stages = VK_PIPELINE_STAGE_2_NONE;
    if (queue_id != kQueueIdInvalid) {
        for (const auto &read_access : GetReads()) {
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
    if (input_attachment_ordering && input_attachment_read && (flags & SyncFlag::kStoreOp) != 0) {
        // If we have an input attachment in last_reads and input attachments are ordered we all that stage
        ordered_stages |= VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT;
    }

    return ordered_stages;
}

void ResourceAccessState::UpdateFirst(const ResourceUsageTagEx tag_ex, const SyncAccessInfo &usage_info, SyncOrdering ordering_rule,
                                      SyncFlags flags) {
    // Only record until we record a write.
    if (!first_access_closed_) {
        const bool is_read = IsRead(usage_info.access_index);
        const VkPipelineStageFlags2 usage_stage = is_read ? usage_info.stage_mask : 0U;
        if (0 == (usage_stage & first_read_stages_)) {
            // If this is a read we haven't seen or a write, record.
            // We always need to know what stages were found prior to write
            first_read_stages_ |= usage_stage;
            if (0 == (read_execution_barriers & usage_stage)) {
                // If this stage isn't masked then we add it (since writes map to usage_stage 0, this also records writes)
                first_accesses_.emplace_back(usage_info, tag_ex, ordering_rule, flags);
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
        assert(first_accesses_.back().usage_info->access_index == SyncAccessIndex::SYNC_IMAGE_LAYOUT_TRANSITION);
        first_write_layout_ordering_ = layout_ordering;
    }
}

void ReadState::Set(VkPipelineStageFlagBits2 stage, SyncAccessIndex access_index, ResourceUsageTagEx tag_ex) {
    assert(access_index != SYNC_ACCESS_INDEX_NONE);
    this->stage = stage;
    this->access_index = access_index;
    barriers = VK_PIPELINE_STAGE_2_NONE;
    sync_stages = VK_PIPELINE_STAGE_2_NONE;
    tag = tag_ex.tag;
    handle_index = tag_ex.handle_index;
    queue = kQueueIdInvalid;
}

// Scope test including "queue submission order" effects.  Specifically, accesses from a different queue are not
// considered to be in "queue submission order" with barriers, events, or semaphore signalling, but any barriers
// that have bee applied (via semaphore) to those accesses can be chained off of.
bool ReadState::ReadOrDependencyChainInSourceScope(QueueId scope_queue, VkPipelineStageFlags2 src_exec_scope) const {
    VkPipelineStageFlags2 effective_stages = barriers | ((scope_queue == queue) ? stage : VK_PIPELINE_STAGE_2_NONE);

    // Special case. AS copy operations (e.g., vkCmdCopyAccelerationStructureKHR) can be synchronized using
    // the ACCELERATION_STRUCTURE_COPY stage, but it's also valid to use ACCELERATION_STRUCTURE_BUILD stage.
    // Internally, AS copy accesses are represented via ACCELERATION_STRUCTURE_COPY stage. The logic below
    // ensures that ACCELERATION_STRUCTURE_COPY accesses can be protected by the barrier that specifies the
    // ACCELERATION_STRUCTURE_BUILD state.
    if (access_index == SYNC_ACCELERATION_STRUCTURE_COPY_ACCELERATION_STRUCTURE_READ) {
        effective_stages |= VK_PIPELINE_STAGE_2_ACCELERATION_STRUCTURE_BUILD_BIT_KHR;
    }

    return (src_exec_scope & effective_stages) != 0;
}

bool ReadState::InBarrierSourceScope(const BarrierScope &barrier_scope) const {
    // TODO: the following comment is from the initial implementation. Check it during Event rework.
    // NOTE: That's not really correct... this read stage might *not* have been included in the SetEvent,
    // and the barriers representing the chain might have changed since then (that would be an odd usage),
    // so as a first approximation we'll assume the barriers *haven't* been changed since (if the tag hasn't),
    // and while this could be a false positive in the case of Set; SomeBarrier; Wait; we'll live with it
    // until we can add more state to the first scope capture (the specific write and read stages that
    // *were* in scope at the moment of SetEvents.
    if (tag > barrier_scope.scope_tag) {
        return false;
    }

    return ReadOrDependencyChainInSourceScope(barrier_scope.scope_queue, barrier_scope.src_exec_scope);
}

void WriteState::Set(SyncAccessIndex access_index, ResourceUsageTagEx tag_ex, SyncFlags flags) {
    this->access_index = access_index;
    this->flags = flags;
    barriers.reset();
    dependency_chain = VK_PIPELINE_STAGE_2_NONE;
    tag = tag_ex.tag;
    handle_index = tag_ex.handle_index;
    queue = kQueueIdInvalid;
}

void WriteState::SetQueueId(QueueId id) {
    // TODO: investigate if we need to check for invalid queue before assignment.
    // Currently no tests fail if we do uncoditional assignment, but in theory this might
    // be part of submit time logic that prevents already initialize ids from being overwritten.
    // If we don't need this check then SetQueueId can be removed.
    if (queue == kQueueIdInvalid) {
        queue = id;
    }
}

bool WriteState::operator==(const WriteState &rhs) const {
    return (access_index == rhs.access_index) && (barriers == rhs.barriers) && (tag == rhs.tag) && (queue == rhs.queue) &&
           (dependency_chain == rhs.dependency_chain);
}

bool WriteState::IsWriteHazard(const SyncAccessInfo &usage_info) const { return !barriers[usage_info.access_index]; }

bool WriteState::IsOrdered(const OrderingBarrier &ordering, QueueId queue_id) const {
    return (queue == queue_id) && ordering.access_scope[access_index];
}

bool WriteState::IsWriteBarrierHazard(QueueId queue_id, VkPipelineStageFlags2 src_exec_scope,
                                                    const SyncAccessFlags &src_access_scope) const {
    // Current implementation relies on TOP_OF_PIPE constant due to the fact that it's non-zero value
    // and AND-ing with it can create execution dependency when necessary. One example, it allows the
    // ALL_COMMANDS stage to guard all accesses even if NONE/TOP_OF_PIPE is used. When NONE constant is
    // used, which has numerical value of zero, then AND-ing with it always results in 0 which means
    // "no barrier", so it's not possible to use NONE internally in equivalent way to TOP_OF_PIPE.
    // Here we replace NONE with TOP_OF_PIPE in the scenarios where they are equivalent according to the spec.
    //
    // If we update implementation to get rid of deprecated TOP_OF_PIPE/BOTTOM_OF_PIPE then we must
    // invert the condition below and exchange TOP_OF_PIPE and NONE roles, so deprecated stages would
    // not propagate into implementation internals.
    if (src_exec_scope == VK_PIPELINE_STAGE_2_NONE && src_access_scope.none()) {
        src_exec_scope = VK_PIPELINE_STAGE_2_TOP_OF_PIPE_BIT;
    }

    // Special rules for sequential ILT's
    if (access_index == SYNC_IMAGE_LAYOUT_TRANSITION) {
        if (queue == queue_id) {
            // In queue, they are implicitly ordered
            return false;
        } else {
            // In dep chain means that the ILT is *available*
            return !DependencyChainInSourceScope(src_exec_scope);
        }
    }
    // In dep chain means that the write is *available*.
    // Available writes are automatically made visible and can't cause hazards during transition.
    if (DependencyChainInSourceScope(src_exec_scope)) {
        return false;
    }
    // The write is not in chain (previous call), so need only to check if the write is in access scope.
    return !WriteInSourceScope(src_access_scope);
}

void WriteState::MergeBarriers(const WriteState &other) {
    barriers |= other.barriers;
    dependency_chain |= other.dependency_chain;
}

bool WriteState::DependencyChainInSourceScope(VkPipelineStageFlags2 src_exec_scope) const {
    return (dependency_chain & src_exec_scope) != 0;
}

bool WriteState::WriteInSourceScope(const SyncAccessFlags &src_access_scope) const {
    return src_access_scope[access_index];
}

bool WriteState::WriteOrDependencyChainInSourceScope(QueueId queue_id, VkPipelineStageFlags2 src_exec_scope,
                                                     const SyncAccessFlags &src_access_scope) const {
    return DependencyChainInSourceScope(src_exec_scope) || (queue == queue_id && WriteInSourceScope(src_access_scope));
}

bool WriteState::InBarrierSourceScope(const BarrierScope &barrier_scope) const {
    if (tag > barrier_scope.scope_tag) {
        return false;
    }
    return WriteOrDependencyChainInSourceScope(barrier_scope.scope_queue, barrier_scope.src_exec_scope,
                                               barrier_scope.src_access_scope);
}

HazardResult::HazardState::HazardState(const ResourceAccessState *access_state_, const SyncAccessInfo &access_info_,
                                       SyncHazard hazard_, SyncAccessIndex prior_access_index, ResourceUsageTagEx tag_ex)
    : access_state(std::make_unique<const ResourceAccessState>(*access_state_)),
      recorded_access(),
      access_index(access_info_.access_index),
      prior_access_index(prior_access_index),
      tag(tag_ex.tag),
      handle_index(tag_ex.handle_index),
      hazard(hazard_) {
    assert(prior_access_index != SYNC_ACCESS_INDEX_NONE);
    // Touchup the hazard to reflect "present as release" semantics
    // NOTE: For implementing QFO release/acquire semantics... touch up here as well
    if (access_state->IsLastWriteOp(SYNC_PRESENT_ENGINE_SYNCVAL_PRESENT_PRESENTED_SYNCVAL)) {
        if (hazard == SyncHazard::READ_AFTER_WRITE) {
            hazard = SyncHazard::READ_AFTER_PRESENT;
        } else if (hazard == SyncHazard::WRITE_AFTER_WRITE) {
            hazard = SyncHazard::WRITE_AFTER_PRESENT;
        }
    } else if (access_index == SYNC_PRESENT_ENGINE_SYNCVAL_PRESENT_PRESENTED_SYNCVAL) {
        if (hazard == SyncHazard::WRITE_AFTER_READ) {
            hazard = SyncHazard::PRESENT_AFTER_READ;
        } else if (hazard == SyncHazard::WRITE_AFTER_WRITE) {
            hazard = SyncHazard::PRESENT_AFTER_WRITE;
        }
    }
}

static VkPipelineStageFlags2 RelatedPipelineStages(VkPipelineStageFlags2 stage_mask,
                                                   const vvl::unordered_map<VkPipelineStageFlags2, VkPipelineStageFlags2> &map) {
    VkPipelineStageFlags2 unscanned = stage_mask;
    VkPipelineStageFlags2 related = 0;
    for (const auto &entry : map) {
        const auto &stage = entry.first;
        if (stage & unscanned) {
            related = related | entry.second;
            unscanned = unscanned & ~stage;
            if (!unscanned) break;
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

template <typename Flags, typename Map>
static SyncAccessFlags AccessScopeImpl(Flags flag_mask, const Map &map) {
    SyncAccessFlags scope;
    for (const auto &bit_scope : map) {
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

SyncExecScope SyncExecScope::MakeSrc(VkQueueFlags queue_flags, VkPipelineStageFlags2 mask_param,
                                     const VkPipelineStageFlags2 disabled_feature_mask) {
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
      dst_exec_scope(dst_exec),
      dst_access_scope(AccessScope(dst_exec.valid_accesses, dst_access_mask)) {}

SyncBarrier::SyncBarrier(VkQueueFlags queue_flags, const VkSubpassDependency2 &subpass) {
    const auto barrier = vku::FindStructInPNextChain<VkMemoryBarrier2>(subpass.pNext);
    if (barrier) {
        auto src = SyncExecScope::MakeSrc(queue_flags, barrier->srcStageMask);
        src_exec_scope = src;
        src_access_scope = AccessScope(src.valid_accesses, barrier->srcAccessMask);

        auto dst = SyncExecScope::MakeDst(queue_flags, barrier->dstStageMask);
        dst_exec_scope = dst;
        dst_access_scope = AccessScope(dst.valid_accesses, barrier->dstAccessMask);
    } else {
        auto src = SyncExecScope::MakeSrc(queue_flags, subpass.srcStageMask);
        src_exec_scope = src;
        src_access_scope = AccessScope(src.valid_accesses, subpass.srcAccessMask);

        auto dst = SyncExecScope::MakeDst(queue_flags, subpass.dstStageMask);
        dst_exec_scope = dst;
        dst_access_scope = AccessScope(dst.valid_accesses, subpass.dstAccessMask);
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

SyncHazardInfo GetSyncHazardInfo(SyncHazard hazard) {
    switch (hazard) {
        case SyncHazard::NONE:
            return SyncHazardInfo{};
        case SyncHazard::READ_AFTER_WRITE:
            return SyncHazardInfo{false, true};
        case SyncHazard::WRITE_AFTER_READ:
            return SyncHazardInfo{true, false};
        case SyncHazard::WRITE_AFTER_WRITE:
            return SyncHazardInfo{true, true};
        case SyncHazard::READ_RACING_WRITE:
            return SyncHazardInfo{false, true, true};
        case SyncHazard::WRITE_RACING_WRITE:
            return SyncHazardInfo{true, true, true};
        case SyncHazard::WRITE_RACING_READ:
            return SyncHazardInfo{true, false, true};
        case SyncHazard::READ_AFTER_PRESENT:
            return SyncHazardInfo{false, true};
        case SyncHazard::WRITE_AFTER_PRESENT:
            return SyncHazardInfo{true, true};
        case SyncHazard::PRESENT_AFTER_WRITE:
            return SyncHazardInfo{true, true};
        case SyncHazard::PRESENT_AFTER_READ:
            return SyncHazardInfo{true, false};
        default:
            assert(false && "Unhandled SyncHazard value");
            return SyncHazardInfo{};
    }
}
