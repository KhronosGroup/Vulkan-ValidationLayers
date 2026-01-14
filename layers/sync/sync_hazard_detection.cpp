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

#include "sync/sync_access_context.h"
#include "sync/sync_image.h"
#include "sync/sync_validation.h"
#include "state_tracker/buffer_state.h"

namespace syncval {

// Execute Action for each map entry in the generated ranges until it returns true
//
// Action is const w.r.t. map
// Action is allowed (assumed) to modify pos
// Action must not advance pos for ranges strictly < pos->first
// Action must handle range strictly less than pos->first correctly
// Action must handle pos == end correctly
// Action is assumed to only require invocation once per map entry
// Note: If Action invocations are heavyweight and inter-entry (gap) calls are not needed
//       add a template or function parameter to skip them. TBD.
template <typename Action>
bool ForEachEntryInRangesUntil(const AccessMap &map, ImageRangeGen &range_gen, Action &action) {
    using RangeType = ImageRangeGen::RangeType;
    using IndexType = RangeType::index_type;
    auto pos = map.LowerBound((*range_gen).begin);
    const auto end = map.end();
    IndexType skip_limit = 0;
    for (; range_gen->non_empty() && pos != end; ++range_gen) {
        RangeType range = *range_gen;
        // See if a prev pos has covered this range
        if (range.end <= skip_limit) {
            // Since the map is const, we needn't call action on the same pos again
            continue;
        }

        //  If the current range was *partially* covered be a previous pos, trim, such that Action is only
        //  called once for a given range (and pos)
        if (range.begin < skip_limit) {
            range.begin = skip_limit;
        }

        // Now advance pos as needed to match range
        if (pos->first.strictly_less(range)) {
            ++pos;
            if (pos == end) break;
            if (pos->first.strictly_less(range)) {
                pos = map.LowerBound(range.begin);
                if (pos == end) break;
            }
            assert(pos == map.LowerBound(range.begin));
        }

        // If the range intersects pos->first, consider Action performed for that map entry, and
        // make sure not to call Action for this pos for any subsequent ranges
        skip_limit = range.end > pos->first.begin ? pos->first.end : 0U;

        // Action is allowed to alter pos but shouldn't do so if range is strictly < pos->first
        if (action(range, end, pos)) return true;
    }

    // Action needs to handle the "at end " condition (and can be useful for recursive actions)
    for (; range_gen->non_empty(); ++range_gen) {
        if (action(*range_gen, end, pos)) return true;
    }

    return false;
}

template <typename DetectorRunner>
HazardResult DoDetect(const AccessContext &access_context, const AccessState &access_state, DetectorRunner detector_runner) {
    if (access_state.next_global_barrier_index < access_context.GetGlobalBarrierCount()) {
        AccessState new_access_state = AccessState::DefaultAccessState();
        new_access_state.Assign(access_state);
        access_context.ApplyGlobalBarriers(new_access_state);
        return detector_runner(new_access_state);
    } else {
        return detector_runner(access_state);
    }
}

class HazardDetector {
  public:
    HazardDetector(SyncAccessIndex access_index, const AccessContext &access_context)
        : access_info_(GetAccessInfo(access_index)), access_context_(access_context) {}

    HazardResult Detect(const AccessMap::const_iterator &pos) const {
        return DoDetect(access_context_, pos->second,
                        [this](const AccessState &access_state) { return access_state.DetectHazard(access_info_); });
    }

    HazardResult DetectAsync(const AccessMap::const_iterator &pos, ResourceUsageTag start_tag, QueueId queue_id) const {
        return DoDetect(access_context_, pos->second, [this, start_tag, queue_id](const AccessState &access_state) {
            return access_state.DetectAsyncHazard(access_info_, start_tag, queue_id);
        });
    }

  private:
    const SyncAccessInfo &access_info_;
    const AccessContext &access_context_;
};

class HazardDetectorWithOrdering {
  public:
    HazardDetectorWithOrdering(SyncAccessIndex access_index, SyncOrdering ordering, const AccessContext &access_context,
                               SyncFlags flags, bool detect_load_op_after_store_op_hazards)
        : access_info_(GetAccessInfo(access_index)),
          ordering_rule_(ordering),
          access_context_(access_context),
          flags_(flags),
          detect_load_op_after_store_op_hazards(detect_load_op_after_store_op_hazards) {}

    HazardResult Detect(const AccessMap::const_iterator &pos) const {
        const OrderingBarrier &ordering = GetOrderingRules(ordering_rule_);
        return DoDetect(access_context_, pos->second, [this, &ordering](const AccessState &access_state) {
            return access_state.DetectHazard(access_info_, ordering, flags_, kQueueIdInvalid,
                                             detect_load_op_after_store_op_hazards);
        });
    }

    HazardResult DetectAsync(const AccessMap::const_iterator &pos, ResourceUsageTag start_tag, QueueId queue_id) const {
        return DoDetect(access_context_, pos->second, [this, start_tag, queue_id](const AccessState &access_state) {
            return access_state.DetectAsyncHazard(access_info_, start_tag, queue_id);
        });
    }

  private:
    const SyncAccessInfo &access_info_;
    const SyncOrdering ordering_rule_;
    const AccessContext &access_context_;
    const SyncFlags flags_;
    const bool detect_load_op_after_store_op_hazards;
};

class HazardDetectFirstUse {
  public:
    HazardDetectFirstUse(const AccessState &recorded_use, QueueId queue_id, const ResourceUsageRange &tag_range,
                         const AccessContext &access_context, bool detect_load_op_after_store_op_hazards)
        : recorded_use_(recorded_use),
          queue_id_(queue_id),
          tag_range_(tag_range),
          access_context_(access_context),
          detect_load_op_after_store_op_hazards(detect_load_op_after_store_op_hazards) {}

    HazardResult Detect(const AccessMap::const_iterator &pos) const {
        return DoDetect(access_context_, pos->second, [this](const AccessState &access_state) {
            return access_state.DetectHazard(recorded_use_, queue_id_, tag_range_, detect_load_op_after_store_op_hazards);
        });
    }

    HazardResult DetectAsync(const AccessMap::const_iterator &pos, ResourceUsageTag start_tag, QueueId queue_id) const {
        return DoDetect(access_context_, pos->second, [this, start_tag, queue_id](const AccessState &access_state) {
            return access_state.DetectAsyncHazard(recorded_use_, tag_range_, start_tag, queue_id);
        });
    }

  private:
    const AccessState &recorded_use_;
    const QueueId queue_id_;
    const ResourceUsageRange &tag_range_;
    const AccessContext &access_context_;
    const bool detect_load_op_after_store_op_hazards;
};

struct HazardDetectorMarker {
    HazardDetectorMarker(const AccessContext &access_context) : access_context(access_context) {}

    HazardResult Detect(const AccessMap::const_iterator &pos) const {
        return DoDetect(access_context, pos->second,
                        [](const AccessState &access_state) { return access_state.DetectMarkerHazard(); });
    }

    HazardResult DetectAsync(const AccessMap::const_iterator &pos, ResourceUsageTag start_tag, QueueId queue_id) const {
        return DoDetect(access_context, pos->second, [start_tag, queue_id](const AccessState &access_state) {
            return access_state.DetectAsyncHazard(GetAccessInfo(SYNC_COPY_TRANSFER_WRITE), start_tag, queue_id);
        });
    }

    const AccessContext &access_context;
};

class BarrierHazardDetector {
  public:
    BarrierHazardDetector(const AccessContext &access_context, SyncAccessIndex access_index, VkPipelineStageFlags2 src_exec_scope,
                          SyncAccessFlags src_access_scope)
        : access_context_(access_context),
          access_info_(GetAccessInfo(access_index)),
          src_exec_scope_(src_exec_scope),
          src_access_scope_(src_access_scope) {}

    HazardResult Detect(const AccessMap::const_iterator &pos) const {
        return DoDetect(access_context_, pos->second, [this](const AccessState &access_state) {
            return access_state.DetectBarrierHazard(access_info_, kQueueIdInvalid, src_exec_scope_, src_access_scope_);
        });
    }

    HazardResult DetectAsync(const AccessMap::const_iterator &pos, ResourceUsageTag start_tag, QueueId queue_id) const {
        return DoDetect(access_context_, pos->second, [this, start_tag, queue_id](const AccessState &access_state) {
            return access_state.DetectAsyncHazard(access_info_, start_tag, queue_id);
        });
    }

  private:
    const AccessContext &access_context_;
    const SyncAccessInfo &access_info_;
    VkPipelineStageFlags2 src_exec_scope_;
    SyncAccessFlags src_access_scope_;
};

class EventBarrierHazardDetector {
  public:
    EventBarrierHazardDetector(SyncAccessIndex access_index, VkPipelineStageFlags2 src_exec_scope, SyncAccessFlags src_access_scope,
                               const AccessContext::ScopeMap &event_scope, QueueId queue_id, ResourceUsageTag scope_tag)
        : access_info_(GetAccessInfo(access_index)),
          src_exec_scope_(src_exec_scope),
          src_access_scope_(src_access_scope),
          event_scope_(event_scope),
          scope_queue_id_(queue_id),
          scope_tag_(scope_tag),
          scope_pos_(event_scope.begin()),
          scope_end_(event_scope.end()) {}

    HazardResult Detect(const AccessMap::const_iterator &pos) {
        // Need to piece together coverage of pos->first range:
        // Copy the range as we'll be chopping it up as needed
        AccessRange range = pos->first;
        const AccessState &access = pos->second;
        HazardResult hazard;

        bool in_scope = AdvanceScope(range);
        bool unscoped_tested = false;
        while (in_scope && !hazard.IsHazard()) {
            if (range.begin < ScopeBegin()) {
                if (!unscoped_tested) {
                    unscoped_tested = true;
                    hazard = access.DetectHazard(access_info_);
                }
                // Note: don't need to check for in_scope as AdvanceScope true means range and ScopeRange intersect.
                // Thus a [ ScopeBegin, range.end ) will be non-empty.
                range.begin = ScopeBegin();
            } else {  // in_scope implied that ScopeRange and range intersect
                hazard = access.DetectBarrierHazard(access_info_, ScopeState(), src_exec_scope_, src_access_scope_, scope_queue_id_,
                                                    scope_tag_);
                if (!hazard.IsHazard()) {
                    range.begin = ScopeEnd();
                    in_scope = AdvanceScope(range);  // contains a non_empty check
                }
            }
        }
        if (range.non_empty() && !hazard.IsHazard() && !unscoped_tested) {
            hazard = access.DetectHazard(access_info_);
        }
        return hazard;
    }

    HazardResult DetectAsync(const AccessMap::const_iterator &pos, ResourceUsageTag start_tag, QueueId queue_id) const {
        // Async barrier hazard detection can use the same path as the usage index is not IsRead, but is IsWrite
        return pos->second.DetectAsyncHazard(access_info_, start_tag, queue_id);
    }

  private:
    bool ScopeInvalid() const { return scope_pos_ == scope_end_; }
    bool ScopeValid() const { return !ScopeInvalid(); }
    void ScopeSeek(const AccessRange &range) { scope_pos_ = event_scope_.LowerBound(range.begin); }

    // Hiding away the std::pair grunge...
    ResourceAddress ScopeBegin() const { return scope_pos_->first.begin; }
    ResourceAddress ScopeEnd() const { return scope_pos_->first.end; }
    const AccessRange &ScopeRange() const { return scope_pos_->first; }
    const AccessState &ScopeState() const { return scope_pos_->second; }

    bool AdvanceScope(const AccessRange &range) {
        // Note: non_empty is (valid && !empty), so don't change !non_empty to empty...
        if (!range.non_empty()) return false;
        if (ScopeInvalid()) return false;

        if (ScopeRange().strictly_less(range)) {
            ScopeSeek(range);
        }

        return ScopeValid() && ScopeRange().intersects(range);
    }

    const SyncAccessInfo access_info_;
    VkPipelineStageFlags2 src_exec_scope_;
    SyncAccessFlags src_access_scope_;
    const AccessContext::ScopeMap &event_scope_;
    QueueId scope_queue_id_;
    const ResourceUsageTag scope_tag_;
    AccessContext::ScopeMap::const_iterator scope_pos_;
    AccessContext::ScopeMap::const_iterator scope_end_;
};

HazardResult AccessContext::DetectHazard(const vvl::Buffer &buffer, SyncAccessIndex access_index, const AccessRange &range) const {
    if (!SimpleBinding(buffer)) {
        return {};
    }
    const auto base_address = ResourceBaseAddress(buffer);
    HazardDetector detector(access_index, *this);
    return DetectHazardRange(detector, (range + base_address), DetectOptions::kDetectAll);
}

HazardResult AccessContext::DetectHazard(const vvl::Image &image, SyncAccessIndex current_usage,
                                         const VkImageSubresourceRange &subresource_range, bool is_depth_sliced) const {
    HazardDetector detector(current_usage, *this);
    ImageRangeGen range_gen = SubState(image).MakeImageRangeGen(subresource_range, is_depth_sliced);
    return DetectHazardGeneratedRangeGen(detector, range_gen, DetectOptions::kDetectAll);
}

HazardResult AccessContext::DetectHazard(const vvl::Image &image, const VkImageSubresourceRange &subresource_range,
                                         const VkOffset3D &offset, const VkExtent3D &extent, bool is_depth_sliced,
                                         SyncAccessIndex current_usage, SyncOrdering ordering_rule) const {
    ImageRangeGen range_gen = SubState(image).MakeImageRangeGen(subresource_range, offset, extent, is_depth_sliced);
    if (ordering_rule == SyncOrdering::kOrderingNone) {
        HazardDetector detector(current_usage, *this);
        return DetectHazardGeneratedRangeGen(detector, range_gen, DetectOptions::kDetectAll);
    } else {
        HazardDetectorWithOrdering detector(current_usage, ordering_rule, *this, 0,
                                            validator->syncval_settings.load_op_after_store_op_validation);
        return DetectHazardGeneratedRangeGen(detector, range_gen, DetectOptions::kDetectAll);
    }
}

HazardResult AccessContext::DetectHazard(const vvl::ImageView &image_view, SyncAccessIndex current_usage) const {
    HazardDetector detector(current_usage, *this);
    auto range_gen = MakeImageRangeGen(image_view);
    return DetectHazardGeneratedRangeGen(detector, range_gen, DetectOptions::kDetectAll);
}

HazardResult AccessContext::DetectHazard(const vvl::ImageView &image_view, const VkOffset3D &offset, const VkExtent3D &extent,
                                         SyncAccessIndex current_usage, SyncOrdering ordering_rule) const {
    HazardDetectorWithOrdering detector(current_usage, ordering_rule, *this, 0,
                                        validator->syncval_settings.load_op_after_store_op_validation);
    ImageRangeGen range_gen(MakeImageRangeGen(image_view, offset, extent));
    return DetectHazardGeneratedRangeGen(detector, range_gen, DetectOptions::kDetectAll);
}

HazardResult AccessContext::DetectHazard(const ImageRangeGen &const_range_gen, SyncAccessIndex current_usage,
                                         const SyncOrdering ordering_rule, SyncFlags flags) const {
    ImageRangeGen range_gen(const_range_gen);
    if (ordering_rule == SyncOrdering::kOrderingNone) {
        HazardDetector detector(current_usage, *this);
        return DetectHazardGeneratedRangeGen(detector, range_gen, DetectOptions::kDetectAll);
    } else {
        HazardDetectorWithOrdering detector(current_usage, ordering_rule, *this, flags,
                                            validator->syncval_settings.load_op_after_store_op_validation);
        return DetectHazardGeneratedRangeGen(detector, range_gen, DetectOptions::kDetectAll);
    }
}

HazardResult AccessContext::DetectHazard(const AttachmentViewGen &view_gen, AttachmentViewGen::Gen gen_type,
                                         SyncAccessIndex current_usage, SyncOrdering ordering_rule, SyncFlags flags) const {
    HazardDetectorWithOrdering detector(current_usage, ordering_rule, *this, flags,
                                        validator->syncval_settings.load_op_after_store_op_validation);
    const std::optional<ImageRangeGen> &attachment_gen = view_gen.GetRangeGen(gen_type);
    if (!attachment_gen) {
        return {};
    }
    ImageRangeGen range_gen(*attachment_gen);
    return DetectHazardGeneratedRangeGen(detector, range_gen, DetectOptions::kDetectAll);
}

HazardResult AccessContext::DetectHazard(const vvl::VideoSession &vs_state, const vvl::VideoPictureResource &resource,
                                         SyncAccessIndex current_usage) const {
    const auto image = static_cast<const vvl::Image *>(resource.image_state.get());
    const auto &sub_state = SubState(*image);
    const auto offset = resource.GetEffectiveImageOffset(vs_state);
    const auto extent = resource.GetEffectiveImageExtent(vs_state);
    ImageRangeGen range_gen(sub_state.MakeImageRangeGen(resource.range, offset, extent, false));
    HazardDetector detector(current_usage, *this);
    return DetectHazardGeneratedRangeGen(detector, range_gen, DetectOptions::kDetectAll);
}

HazardResult AccessContext::DetectImageBarrierHazard(const vvl::Image &image, const VkImageSubresourceRange &subresource_range,
                                                     VkPipelineStageFlags2 src_exec_scope, const SyncAccessFlags &src_access_scope,
                                                     QueueId queue_id, const ScopeMap &scope_map, const ResourceUsageTag scope_tag,
                                                     AccessContext::DetectOptions options) const {
    EventBarrierHazardDetector detector(SyncAccessIndex::SYNC_IMAGE_LAYOUT_TRANSITION, src_exec_scope, src_access_scope, scope_map,
                                        queue_id, scope_tag);
    ImageRangeGen range_gen = SubState(image).MakeImageRangeGen(subresource_range, false);
    return DetectHazardGeneratedRangeGen(detector, range_gen, options);
}

HazardResult AccessContext::DetectImageBarrierHazard(const vvl::Image &image, VkPipelineStageFlags2 src_exec_scope,
                                                     const SyncAccessFlags &src_access_scope,
                                                     const VkImageSubresourceRange &subresource_range, bool is_depth_sliced,
                                                     const DetectOptions options) const {
    BarrierHazardDetector detector(*this, SyncAccessIndex::SYNC_IMAGE_LAYOUT_TRANSITION, src_exec_scope, src_access_scope);
    ImageRangeGen range_gen = SubState(image).MakeImageRangeGen(subresource_range, is_depth_sliced);
    return DetectHazardGeneratedRangeGen(detector, range_gen, options);
}

HazardResult AccessContext::DetectImageBarrierHazard(const AttachmentViewGen &view_gen, const SyncBarrier &barrier,
                                                     DetectOptions options) const {
    BarrierHazardDetector detector(*this, SyncAccessIndex::SYNC_IMAGE_LAYOUT_TRANSITION, barrier.src_exec_scope.exec_scope,
                                   barrier.src_access_scope);
    const std::optional<ImageRangeGen> &attachment_gen = view_gen.GetRangeGen(AttachmentViewGen::Gen::kViewSubresource);
    subresource_adapter::ImageRangeGenerator range_gen(*attachment_gen);
    return DetectHazardGeneratedRangeGen(detector, range_gen, options);
}

HazardResult AccessContext::DetectSubpassTransitionHazard(const SubpassBarrierTrackback &track_back,
                                                          const AttachmentViewGen &attach_view) const {
    // We should never ask for a transition from a context we don't have
    assert(track_back.source_subpass);

    // Do the detection against the specific prior context independent of other contexts.  (Synchronous only)
    // Hazard detection for the transition can be against the merged of the barriers (it only uses src_...)
    const SyncBarrier merged_barrier(track_back.barriers);
    HazardResult hazard = track_back.source_subpass->DetectImageBarrierHazard(attach_view, merged_barrier, kDetectPrevious);
    if (!hazard.IsHazard()) {
        // The Async hazard check is against the current context's async set.
        SyncBarrier null_barrier = {};
        hazard = DetectImageBarrierHazard(attach_view, null_barrier, kDetectAsync);
    }

    return hazard;
}

// This is called with the *recorded* command buffers access context, with the
// *active* access context pass in, againsts which hazards will be detected
HazardResult AccessContext::DetectFirstUseHazard(QueueId queue_id, const ResourceUsageRange &tag_range,
                                                 const AccessContext &access_context) const {
    // If the context is finalized we have a fast path to find first accesses within a range
    if (finalized_) {
        for (const auto &single_tag : sorted_first_accesses_.IterateSingleTagFirstAccesses(tag_range)) {
            const AccessRange access_range = single_tag.p_key_value->first;
            const AccessState &access = single_tag.p_key_value->second;

            // For single tag first accesses we have exact search and can assert the find
            assert(access.FirstAccessInTagRange(tag_range));

            HazardDetectFirstUse detector(access, queue_id, tag_range, access_context,
                                          validator->syncval_settings.load_op_after_store_op_validation);
            HazardResult hazard = access_context.DetectHazardRange(detector, access_range, DetectOptions::kDetectAll);
            if (hazard.IsHazard()) {
                return hazard;
            }
        }
        for (const auto &multi_tag : sorted_first_accesses_.IterateMultiTagFirstAccesses(tag_range)) {
            const AccessRange access_range = multi_tag.p_key_value->first;
            const AccessState &access = multi_tag.p_key_value->second;

            // For multi tag first accesses the search is not exact, so we need to check for range inclusion
            // (on average multi tag search is faster than going over the entire access map)
            if (!access.FirstAccessInTagRange(tag_range)) {
                continue;
            }

            HazardDetectFirstUse detector(access, queue_id, tag_range, access_context,
                                          validator->syncval_settings.load_op_after_store_op_validation);
            HazardResult hazard = access_context.DetectHazardRange(detector, access_range, DetectOptions::kDetectAll);
            if (hazard.IsHazard()) {
                return hazard;
            }
        }
    }
    // The context is not finalized. We have to iterate over the entire access map
    else {
        for (const auto &recorded_access : access_state_map_) {
            // Cull any entries not in the current tag range
            if (!recorded_access.second.FirstAccessInTagRange(tag_range)) {
                continue;
            }
            HazardDetectFirstUse detector(recorded_access.second, queue_id, tag_range, access_context,
                                          validator->syncval_settings.load_op_after_store_op_validation);
            HazardResult hazard = access_context.DetectHazardRange(detector, recorded_access.first, DetectOptions::kDetectAll);
            if (hazard.IsHazard()) {
                return hazard;
            }
        }
    }
    return {};
}

HazardResult AccessContext::DetectMarkerHazard(const vvl::Buffer &buffer, const AccessRange &range) const {
    if (!SimpleBinding(buffer)) {
        return HazardResult();
    }
    const VkDeviceSize base_address = ResourceBaseAddress(buffer);
    HazardDetectorMarker detector(*this);
    return DetectHazardRange(detector, (range + base_address), DetectOptions::kDetectAll);
}

template <typename Detector>
HazardResult AccessContext::DetectHazardRange(Detector &detector, const AccessRange &range, DetectOptions options) const {
    if (!range.non_empty()) {
        return {};
    }

    HazardResult hazard;

    if (static_cast<uint32_t>(options) & DetectOptions::kDetectAsync) {
        // Async checks don't require recursive lookups, as the async lists are
        // exhaustive for the top-level context so we'll check these first
        for (const auto &async_ref : async_) {
            hazard = async_ref.Context().DetectAsyncHazard(detector, range, async_ref.StartTag(), async_ref.GetQueueId());
            if (hazard.IsHazard()) {
                return hazard;
            }
        }
    }
    const bool detect_prev = (options & DetectOptions::kDetectPrevious) != 0;
    auto pos = access_state_map_.LowerBound(range.begin);
    hazard = DetectHazardOneRange(detector, detect_prev, pos, access_state_map_.end(), range);
    return hazard;
}

template <typename Detector>
HazardResult AccessContext::DetectHazardGeneratedRangeGen(Detector &detector, ImageRangeGen &range_gen,
                                                          DetectOptions options) const {
    HazardResult hazard;

    if ((options & DetectOptions::kDetectAsync) != 0) {
        // Async checks don't require recursive lookups, as the async lists are
        // exhaustive for the top-level context so we'll check these first
        for (const auto &async_ref : async_) {
            ImageRangeGen range_gen_copy(range_gen);  // original range gen is needed later
            hazard = async_ref.Context().DetectAsyncHazard(detector, range_gen_copy, async_ref.StartTag(), async_ref.GetQueueId());
            if (hazard.IsHazard()) return hazard;
        }
    }

    const bool detect_prev = (options & DetectOptions::kDetectPrevious) != 0;
    using ConstIterator = AccessMap::const_iterator;
    auto do_detect_hazard_range = [this, &detector, &hazard, detect_prev](const ImageRangeGen::RangeType &range,
                                                                          const ConstIterator &end, ConstIterator &pos) {
        hazard = DetectHazardOneRange(detector, detect_prev, pos, end, range);
        return hazard.IsHazard();
    };
    ForEachEntryInRangesUntil(access_state_map_, range_gen, do_detect_hazard_range);
    return hazard;
}

template <typename Detector>
HazardResult AccessContext::DetectAsyncHazard(const Detector &detector, const AccessRange &range, ResourceUsageTag async_tag,
                                              QueueId async_queue_id) const {
    assert(range.non_empty());
    HazardResult hazard;
    auto pos = access_state_map_.LowerBound(range.begin);
    if (pos != access_state_map_.end() && pos->first.begin < range.end) {
        hazard = detector.DetectAsync(pos, async_tag, async_queue_id);
    }
    return hazard;
}

template <typename Detector>
HazardResult AccessContext::DetectAsyncHazard(const Detector &detector, ImageRangeGen &range_gen, ResourceUsageTag async_tag,
                                              QueueId async_queue_id) const {
    using ConstIterator = AccessMap::const_iterator;
    HazardResult hazard;

    auto do_async_hazard_check = [&detector, async_tag, async_queue_id, &hazard](const ImageRangeGen::RangeType &range,
                                                                                 const ConstIterator &end, ConstIterator &pos) {
        while (pos != end && pos->first.begin < range.end) {
            hazard = detector.DetectAsync(pos, async_tag, async_queue_id);
            if (hazard.IsHazard()) return true;
            ++pos;
        }
        return false;
    };
    ForEachEntryInRangesUntil(access_state_map_, range_gen, do_async_hazard_check);
    return hazard;
}

template <typename Detector>
HazardResult AccessContext::DetectHazardOneRange(Detector &detector, bool detect_prev, AccessMap::const_iterator &pos,
                                                 const AccessMap::const_iterator &the_end, const AccessRange &range) const {
    HazardResult hazard;
    AccessRange gap = {range.begin, range.begin};

    while (pos != the_end && pos->first.begin < range.end) {
        // Cover any leading gap, or gap between entries
        if (detect_prev) {
            // TODO: After profiling we may want to change the descent logic such that we don't recur per gap...
            // Cover any leading gap, or gap between entries
            gap.end = pos->first.begin;  // We know this begin is < range.end
            if (gap.non_empty()) {
                // Recur on all gaps
                hazard = DetectPreviousHazard(detector, gap);
                if (hazard.IsHazard()) return hazard;
            }
            // Set up for the next gap.  If pos..end is >= range.end, loop will exit, and trailing gap will be empty
            gap.begin = pos->first.end;
        }

        hazard = detector.Detect(pos);
        if (hazard.IsHazard()) return hazard;
        ++pos;
    }

    if (detect_prev) {
        // Detect in the trailing empty as needed
        gap.end = range.end;
        if (gap.non_empty()) {
            hazard = DetectPreviousHazard(detector, gap);
        }
    }

    return hazard;
}

template <typename Detector>
HazardResult AccessContext::DetectPreviousHazard(Detector &detector, const AccessRange &range) const {
    if (prev_.empty()) {
        return {};
    }
    AccessContext descent_context;
    for (const auto &prev_dep : prev_) {
        const ApplyTrackbackStackAction barrier_action(prev_dep.barriers, nullptr);
        prev_dep.source_subpass->ResolveAccessRangeRecursePrev(range, barrier_action, descent_context, false);
    }
    AccessMap &descent_map = descent_context.access_state_map_;
    for (auto prev = descent_map.begin(); prev != descent_map.end(); ++prev) {
        HazardResult hazard = detector.Detect(prev);
        if (hazard.IsHazard()) {
            return hazard;
        }
    }
    return {};
}

}  // namespace syncval
