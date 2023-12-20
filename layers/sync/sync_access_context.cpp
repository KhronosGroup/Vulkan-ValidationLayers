/*
 * Copyright (c) 2019-2023 Valve Corporation
 * Copyright (c) 2019-2023 LunarG, Inc.
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

#include <cinttypes>
#include "state_tracker/buffer_state.h"
#include "state_tracker/video_session_state.h"
#include "sync/sync_access_context.h"

bool SimpleBinding(const vvl::Bindable &bindable) { return !bindable.sparse && bindable.Binding(); }
VkDeviceSize ResourceBaseAddress(const vvl::Buffer &buffer) { return buffer.GetFakeBaseAddress(); }

class HazardDetector {
    const SyncStageAccessInfoType &usage_info_;

  public:
    HazardResult Detect(const ResourceAccessRangeMap::const_iterator &pos) const { return pos->second.DetectHazard(usage_info_); }
    HazardResult DetectAsync(const ResourceAccessRangeMap::const_iterator &pos, ResourceUsageTag start_tag) const {
        return pos->second.DetectAsyncHazard(usage_info_, start_tag);
    }
    explicit HazardDetector(SyncStageAccessIndex usage_index) : usage_info_(SyncStageAccess::UsageInfo(usage_index)) {}
};

class HazardDetectorWithOrdering {
    const SyncStageAccessInfoType &usage_info_;
    const SyncOrdering ordering_rule_;

  public:
    HazardResult Detect(const ResourceAccessRangeMap::const_iterator &pos) const {
        return pos->second.DetectHazard(usage_info_, ordering_rule_, kQueueIdInvalid);
    }
    HazardResult DetectAsync(const ResourceAccessRangeMap::const_iterator &pos, ResourceUsageTag start_tag) const {
        return pos->second.DetectAsyncHazard(usage_info_, start_tag);
    }
    HazardDetectorWithOrdering(SyncStageAccessIndex usage_index, SyncOrdering ordering)
        : usage_info_(SyncStageAccess::UsageInfo(usage_index)), ordering_rule_(ordering) {}
};

class HazardDetectFirstUse {
  public:
    HazardDetectFirstUse(const ResourceAccessState &recorded_use, QueueId queue_id, const ResourceUsageRange &tag_range)
        : recorded_use_(recorded_use), queue_id_(queue_id), tag_range_(tag_range) {}
    HazardResult Detect(const ResourceAccessRangeMap::const_iterator &pos) const {
        return pos->second.DetectHazard(recorded_use_, queue_id_, tag_range_);
    }
    HazardResult DetectAsync(const ResourceAccessRangeMap::const_iterator &pos, ResourceUsageTag start_tag) const {
        return pos->second.DetectAsyncHazard(recorded_use_, tag_range_, start_tag);
    }

  private:
    const ResourceAccessState &recorded_use_;
    const QueueId queue_id_;
    const ResourceUsageRange &tag_range_;
};

AccessContext::AccessContext(uint32_t subpass, VkQueueFlags queue_flags,
                             const std::vector<SubpassDependencyGraphNode> &dependencies,
                             const std::vector<AccessContext> &contexts, const AccessContext *external_context) {
    Reset();
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
        async_.emplace_back(contexts[async_subpass], kInvalidTag);
    }

    if (has_barrier_from_external) {
        // Store the barrier from external with the reat, but save pointer for "by subpass" lookups.
        prev_.emplace_back(external_context, queue_flags, subpass_dep.barrier_from_external);
        src_external_ = &prev_.back();
    }
    if (subpass_dep.barrier_to_external.size()) {
        dst_external_ = TrackBack(this, queue_flags, subpass_dep.barrier_to_external);
    }
}

template <typename NormalizeOp>
void AccessContext::Trim(NormalizeOp &&normalize) {
    ForAll(std::forward<NormalizeOp>(normalize));
    sparse_container::consolidate(access_state_map_);
}

void AccessContext::Trim() {
    auto normalize = [](ResourceAccessRangeMap::value_type &access) { access.second.Normalize(); };
    Trim(normalize);
}

void AccessContext::TrimAndClearFirstAccess() {
    auto normalize = [](ResourceAccessRangeMap::value_type &access) {
        access.second.Normalize();
        access.second.ClearFirstUse();
    };
    Trim(normalize);
}

void AccessContext::AddReferencedTags(ResourceUsageTagSet &used) const {
    auto gather = [&used](const ResourceAccessRangeMap::value_type &access) { access.second.GatherReferencedTags(used); };
    ConstForAll(gather);
}

template <typename Action>
void AccessContext::ForAll(Action &&action) {
    for (auto &access : access_state_map_) {
        action(access);
    }
}

template <typename Action>
void AccessContext::ConstForAll(Action &&action) const {
    for (auto &access : access_state_map_) {
        action(access);
    }
}

void AccessContext::ResolveFromContext(const AccessContext &from) {
    const NoopBarrierAction noop_barrier;
    from.ResolveAccessRange(kFullRange, noop_barrier, &access_state_map_, nullptr);
}

void AccessContext::ResolvePreviousAccess(const ResourceAccessRange &range, ResourceAccessRangeMap *descent_map,
                                          const ResourceAccessState *infill_state,
                                          const ResourceAccessStateFunction *previous_barrier) const {
    if (prev_.size() == 0) {
        if (range.non_empty() && infill_state) {
            // Fill the empty poritions of descent_map with the default_state with the barrier function applied (iff present)
            ResourceAccessState state_copy;
            if (previous_barrier) {
                assert(bool(*previous_barrier));
                state_copy = *infill_state;
                (*previous_barrier)(&state_copy);
                infill_state = &state_copy;
            }
            sparse_container::update_range_value(*descent_map, range, *infill_state,
                                                 sparse_container::value_precedence::prefer_dest);
        }
    } else {
        // Look for something to fill the gap further along.
        for (const auto &prev_dep : prev_) {
            const ApplyTrackbackStackAction barrier_action(prev_dep.barriers, previous_barrier);
            prev_dep.source_subpass->ResolveAccessRange(range, barrier_action, descent_map, infill_state);
        }
    }
}

// Non-lazy import of all accesses, WaitEvents needs this.
void AccessContext::ResolvePreviousAccesses() {
    ResourceAccessState default_state;
    if (!prev_.size()) return;  // If no previous contexts, nothing to do

    ResolvePreviousAccess(kFullRange, &access_state_map_, &default_state);
}

void AccessContext::UpdateAccessState(const vvl::Buffer &buffer, SyncStageAccessIndex current_usage, SyncOrdering ordering_rule,
                                      const ResourceAccessRange &range, const ResourceUsageTag tag) {
    if (!SimpleBinding(buffer)) return;
    const auto base_address = ResourceBaseAddress(buffer);
    UpdateMemoryAccessStateFunctor action(*this, current_usage, ordering_rule, tag);
    UpdateMemoryAccessRangeState(access_state_map_, action, range + base_address);
}

void AccessContext::UpdateAccessState(const ImageState &image, SyncStageAccessIndex current_usage, SyncOrdering ordering_rule,
                                      const VkImageSubresourceRange &subresource_range, const ResourceUsageTag &tag) {
    // range_gen is non-temporary to avoid an additional copy
    ImageRangeGen range_gen = image.MakeImageRangeGen(subresource_range, false);
    UpdateAccessState(range_gen, current_usage, ordering_rule, tag);
}
void AccessContext::UpdateAccessState(const ImageState &image, SyncStageAccessIndex current_usage, SyncOrdering ordering_rule,
                                      const VkImageSubresourceRange &subresource_range, const VkOffset3D &offset,
                                      const VkExtent3D &extent, const ResourceUsageTag tag) {
    // range_gen is non-temporary to avoid an additional copy
    ImageRangeGen range_gen = image.MakeImageRangeGen(subresource_range, offset, extent, false);
    UpdateAccessState(range_gen, current_usage, ordering_rule, tag);
}

void AccessContext::UpdateAccessState(const ImageViewState &image_view, SyncStageAccessIndex current_usage,
                                      SyncOrdering ordering_rule, const VkOffset3D &offset, const VkExtent3D &extent,
                                      const ResourceUsageTag tag) {
    // range_gen is non-temporary to avoid an additional copy
    ImageRangeGen range_gen(image_view.MakeImageRangeGen(offset, extent));
    UpdateAccessState(range_gen, current_usage, ordering_rule, tag);
}

void AccessContext::UpdateAccessState(const ImageViewState &image_view, SyncStageAccessIndex current_usage,
                                      SyncOrdering ordering_rule, ResourceUsageTag tag) {
    // Get is const, and will be copied in callee
    UpdateAccessState(image_view.GetFullViewImageRangeGen(), current_usage, ordering_rule, tag);
}

void AccessContext::UpdateAccessState(const AttachmentViewGen &view_gen, AttachmentViewGen::Gen gen_type,
                                      SyncStageAccessIndex current_usage, SyncOrdering ordering_rule, const ResourceUsageTag tag) {
    const std::optional<ImageRangeGen> &attachment_gen = view_gen.GetRangeGen(gen_type);
    if (attachment_gen) {
        // Value of const optional is const, and will be copied in callee
        UpdateAccessState(*attachment_gen, current_usage, ordering_rule, tag);
    }
}

void AccessContext::UpdateAccessState(const vvl::VideoSession &vs_state, const vvl::VideoPictureResource &resource,
                                      SyncStageAccessIndex current_usage, ResourceUsageTag tag) {
    const auto image = static_cast<const ImageState *>(resource.image_state.get());
    const auto offset = vs_state.profile->GetEffectiveImageOffset(resource.coded_offset);
    const auto extent = vs_state.profile->GetEffectiveImageExtent(resource.coded_extent);
    ImageRangeGen range_gen(image->MakeImageRangeGen(resource.range, offset, extent, false));
    UpdateAccessState(range_gen, current_usage, SyncOrdering::kNonAttachment, tag);
}

void AccessContext::UpdateAccessState(ImageRangeGen &range_gen, SyncStageAccessIndex current_usage, SyncOrdering ordering_rule,
                                      ResourceUsageTag tag) {
    UpdateMemoryAccessStateFunctor action(*this, current_usage, ordering_rule, tag);
    UpdateMemoryAccessState(action, range_gen);
}

void AccessContext::UpdateAccessState(const ImageRangeGen &range_gen, SyncStageAccessIndex current_usage,
                                      SyncOrdering ordering_rule, ResourceUsageTag tag) {
    // range_gen is non-temporary to avoid infinite call recursion
    ImageRangeGen mutable_range_gen(range_gen);
    UpdateAccessState(mutable_range_gen, current_usage, ordering_rule, tag);
}

void AccessContext::ResolveChildContexts(const std::vector<AccessContext> &contexts) {
    for (uint32_t subpass_index = 0; subpass_index < contexts.size(); subpass_index++) {
        auto &context = contexts[subpass_index];
        ApplyTrackbackStackAction barrier_action(context.GetDstExternalTrackBack().barriers);
        context.ResolveAccessRange(kFullRange, barrier_action, &access_state_map_, nullptr, false);
    }
}

// Caller must ensure that lifespan of this is less than the lifespan of from
void AccessContext::ImportAsyncContexts(const AccessContext &from) {
    async_.insert(async_.end(), from.async_.begin(), from.async_.end());
}

// Suitable only for *subpass* access contexts
HazardResult AccessContext::DetectSubpassTransitionHazard(const TrackBack &track_back, const AttachmentViewGen &attach_view) const {
    if (!attach_view.IsValid()) return HazardResult();

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

void AccessContext::AddAsyncContext(const AccessContext *context, ResourceUsageTag tag) {
    if (context) {
        async_.emplace_back(*context, tag);
    }
}

HazardResult AccessContext::DetectHazard(const vvl::Buffer &buffer, SyncStageAccessIndex usage_index,
                                         const ResourceAccessRange &range) const {
    if (!SimpleBinding(buffer)) return HazardResult();
    const auto base_address = ResourceBaseAddress(buffer);
    HazardDetector detector(usage_index);
    return DetectHazardRange(detector, (range + base_address), DetectOptions::kDetectAll);
}

template <typename Detector>
HazardResult AccessContext::DetectHazard(Detector &detector, const AttachmentViewGen &view_gen, AttachmentViewGen::Gen gen_type,
                                         DetectOptions options) const {
    const std::optional<ImageRangeGen> &attachment_gen = view_gen.GetRangeGen(gen_type);
    if (!attachment_gen) return HazardResult();

    subresource_adapter::ImageRangeGenerator range_gen(*attachment_gen);
    return DetectHazardGeneratedRanges(detector, range_gen, options);
}

template <typename Detector>
HazardResult AccessContext::DetectHazard(Detector &detector, const ImageState &image,
                                         const VkImageSubresourceRange &subresource_range, const VkOffset3D &offset,
                                         const VkExtent3D &extent, bool is_depth_sliced, DetectOptions options) const {
    // range_gen is non-temporary to avoid additional copy
    ImageRangeGen range_gen = image.MakeImageRangeGen(subresource_range, offset, extent, is_depth_sliced);
    return DetectHazardGeneratedRanges(detector, range_gen, options);
}

template <typename Detector>
HazardResult AccessContext::DetectHazard(Detector &detector, const ImageState &image,
                                         const VkImageSubresourceRange &subresource_range, bool is_depth_sliced,
                                         DetectOptions options) const {
    // range_gen is non-temporary to avoid additional copy
    ImageRangeGen range_gen = image.MakeImageRangeGen(subresource_range, is_depth_sliced);
    return DetectHazardGeneratedRanges(detector, range_gen, options);
}

HazardResult AccessContext::DetectHazard(const ImageState &image, SyncStageAccessIndex current_usage,
                                         const VkImageSubresourceRange &subresource_range, bool is_depth_sliced) const {
    HazardDetector detector(current_usage);
    return DetectHazard(detector, image, subresource_range, is_depth_sliced, DetectOptions::kDetectAll);
}

HazardResult AccessContext::DetectHazard(const ImageViewState &image_view, SyncStageAccessIndex current_usage) const {
    // Get is const, but callee will copy
    HazardDetector detector(current_usage);
    return DetectHazardGeneratedRanges(detector, image_view.GetFullViewImageRangeGen(), DetectOptions::kDetectAll);
}

HazardResult AccessContext::DetectHazard(const ImageRangeGen &ref_range_gen, SyncStageAccessIndex current_usage,
                                         const SyncOrdering ordering_rule) const {
    if (ordering_rule == SyncOrdering::kOrderingNone) {
        HazardDetector detector(current_usage);
        return DetectHazardGeneratedRanges(detector, ref_range_gen, DetectOptions::kDetectAll);
    }

    HazardDetectorWithOrdering detector(current_usage, ordering_rule);
    return DetectHazardGeneratedRanges(detector, ref_range_gen, DetectOptions::kDetectAll);
}

HazardResult AccessContext::DetectHazard(const ImageViewState &image_view, const VkOffset3D &offset, const VkExtent3D &extent,
                                         SyncStageAccessIndex current_usage, SyncOrdering ordering_rule) const {
    // range_gen is non-temporary to avoid an additional copy
    ImageRangeGen range_gen(image_view.MakeImageRangeGen(offset, extent));
    HazardDetectorWithOrdering detector(current_usage, ordering_rule);
    return DetectHazardGeneratedRanges(detector, range_gen, DetectOptions::kDetectAll);
}

HazardResult AccessContext::DetectHazard(const AttachmentViewGen &view_gen, AttachmentViewGen::Gen gen_type,
                                         SyncStageAccessIndex current_usage, SyncOrdering ordering_rule) const {
    HazardDetectorWithOrdering detector(current_usage, ordering_rule);
    return DetectHazard(detector, view_gen, gen_type, DetectOptions::kDetectAll);
}

HazardResult AccessContext::DetectHazard(const vvl::VideoSession &vs_state, const vvl::VideoPictureResource &resource,
                                         SyncStageAccessIndex current_usage) const {
    const auto image = static_cast<const ImageState *>(resource.image_state.get());
    const auto offset = vs_state.profile->GetEffectiveImageOffset(resource.coded_offset);
    const auto extent = vs_state.profile->GetEffectiveImageExtent(resource.coded_extent);
    ImageRangeGen range_gen(image->MakeImageRangeGen(resource.range, offset, extent, false));
    HazardDetector detector(current_usage);
    return DetectHazardGeneratedRanges(detector, range_gen, DetectOptions::kDetectAll);
}

HazardResult AccessContext::DetectHazard(const ImageState &image, const VkImageSubresourceRange &subresource_range,
                                         const VkOffset3D &offset, const VkExtent3D &extent, bool is_depth_sliced,
                                         SyncStageAccessIndex current_usage, SyncOrdering ordering_rule) const {
    if (ordering_rule == SyncOrdering::kOrderingNone) {
        HazardDetector detector(current_usage);
        return DetectHazard(detector, image, subresource_range, offset, extent, is_depth_sliced, DetectOptions::kDetectAll);
    }
    HazardDetectorWithOrdering detector(current_usage, ordering_rule);
    return DetectHazard(detector, image, subresource_range, offset, extent, is_depth_sliced, DetectOptions::kDetectAll);
}

class BarrierHazardDetector {
  public:
    BarrierHazardDetector(SyncStageAccessIndex usage_index, VkPipelineStageFlags2KHR src_exec_scope,
                          SyncStageAccessFlags src_access_scope)
        : usage_info_(SyncStageAccess::UsageInfo(usage_index)),
          src_exec_scope_(src_exec_scope),
          src_access_scope_(src_access_scope) {}

    HazardResult Detect(const ResourceAccessRangeMap::const_iterator &pos) const {
        return pos->second.DetectBarrierHazard(usage_info_, kQueueIdInvalid, src_exec_scope_, src_access_scope_);
    }
    HazardResult DetectAsync(const ResourceAccessRangeMap::const_iterator &pos, ResourceUsageTag start_tag) const {
        // Async barrier hazard detection can use the same path as the usage index is not IsRead, but is IsWrite
        return pos->second.DetectAsyncHazard(usage_info_, start_tag);
    }

  private:
    const SyncStageAccessInfoType &usage_info_;
    VkPipelineStageFlags2KHR src_exec_scope_;
    SyncStageAccessFlags src_access_scope_;
};

class EventBarrierHazardDetector {
  public:
    EventBarrierHazardDetector(SyncStageAccessIndex usage_index, VkPipelineStageFlags2KHR src_exec_scope,
                               SyncStageAccessFlags src_access_scope, const AccessContext::ScopeMap &event_scope, QueueId queue_id,
                               ResourceUsageTag scope_tag)
        : usage_info_(SyncStageAccess::UsageInfo(usage_index)),
          src_exec_scope_(src_exec_scope),
          src_access_scope_(src_access_scope),
          event_scope_(event_scope),
          scope_queue_id_(queue_id),
          scope_tag_(scope_tag),
          scope_pos_(event_scope.cbegin()),
          scope_end_(event_scope.cend()) {}

    HazardResult Detect(const ResourceAccessRangeMap::const_iterator &pos) {
        // Need to piece together coverage of pos->first range:
        // Copy the range as we'll be chopping it up as needed
        ResourceAccessRange range = pos->first;
        const ResourceAccessState &access = pos->second;
        HazardResult hazard;

        bool in_scope = AdvanceScope(range);
        bool unscoped_tested = false;
        while (in_scope && !hazard.IsHazard()) {
            if (range.begin < ScopeBegin()) {
                if (!unscoped_tested) {
                    unscoped_tested = true;
                    hazard = access.DetectHazard(usage_info_);
                }
                // Note: don't need to check for in_scope as AdvanceScope true means range and ScopeRange intersect.
                // Thus a [ ScopeBegin, range.end ) will be non-empty.
                range.begin = ScopeBegin();
            } else {  // in_scope implied that ScopeRange and range intersect
                hazard = access.DetectBarrierHazard(usage_info_, ScopeState(), src_exec_scope_, src_access_scope_, scope_queue_id_,
                                                    scope_tag_);
                if (!hazard.IsHazard()) {
                    range.begin = ScopeEnd();
                    in_scope = AdvanceScope(range);  // contains a non_empty check
                }
            }
        }
        if (range.non_empty() && !hazard.IsHazard() && !unscoped_tested) {
            hazard = access.DetectHazard(usage_info_);
        }
        return hazard;
    }

    HazardResult DetectAsync(const ResourceAccessRangeMap::const_iterator &pos, ResourceUsageTag start_tag) const {
        // Async barrier hazard detection can use the same path as the usage index is not IsRead, but is IsWrite
        return pos->second.DetectAsyncHazard(usage_info_, start_tag);
    }

  private:
    bool ScopeInvalid() const { return scope_pos_ == scope_end_; }
    bool ScopeValid() const { return !ScopeInvalid(); }
    void ScopeSeek(const ResourceAccessRange &range) { scope_pos_ = event_scope_.lower_bound(range); }

    // Hiding away the std::pair grunge...
    ResourceAddress ScopeBegin() const { return scope_pos_->first.begin; }
    ResourceAddress ScopeEnd() const { return scope_pos_->first.end; }
    const ResourceAccessRange &ScopeRange() const { return scope_pos_->first; }
    const ResourceAccessState &ScopeState() const { return scope_pos_->second; }

    bool AdvanceScope(const ResourceAccessRange &range) {
        // Note: non_empty is (valid && !empty), so don't change !non_empty to empty...
        if (!range.non_empty()) return false;
        if (ScopeInvalid()) return false;

        if (ScopeRange().strictly_less(range)) {
            ScopeSeek(range);
        }

        return ScopeValid() && ScopeRange().intersects(range);
    }

    const SyncStageAccessInfoType usage_info_;
    VkPipelineStageFlags2KHR src_exec_scope_;
    SyncStageAccessFlags src_access_scope_;
    const AccessContext::ScopeMap &event_scope_;
    QueueId scope_queue_id_;
    const ResourceUsageTag scope_tag_;
    AccessContext::ScopeMap::const_iterator scope_pos_;
    AccessContext::ScopeMap::const_iterator scope_end_;
};

HazardResult AccessContext::DetectImageBarrierHazard(const ImageState &image, const VkImageSubresourceRange &subresource_range,
                                                     VkPipelineStageFlags2KHR src_exec_scope,
                                                     const SyncStageAccessFlags &src_access_scope, QueueId queue_id,
                                                     const ScopeMap &scope_map, const ResourceUsageTag scope_tag,
                                                     AccessContext::DetectOptions options) const {
    EventBarrierHazardDetector detector(SyncStageAccessIndex::SYNC_IMAGE_LAYOUT_TRANSITION, src_exec_scope, src_access_scope,
                                        scope_map, queue_id, scope_tag);
    return DetectHazard(detector, image, subresource_range, false, options);
}

HazardResult AccessContext::DetectImageBarrierHazard(const AttachmentViewGen &view_gen, const SyncBarrier &barrier,
                                                     DetectOptions options) const {
    BarrierHazardDetector detector(SyncStageAccessIndex::SYNC_IMAGE_LAYOUT_TRANSITION, barrier.src_exec_scope.exec_scope,
                                   barrier.src_access_scope);
    return DetectHazard(detector, view_gen, AttachmentViewGen::Gen::kViewSubresource, options);
}

HazardResult AccessContext::DetectImageBarrierHazard(const ImageState &image, VkPipelineStageFlags2KHR src_exec_scope,
                                                     const SyncStageAccessFlags &src_access_scope,
                                                     const VkImageSubresourceRange &subresource_range,
                                                     const DetectOptions options) const {
    BarrierHazardDetector detector(SyncStageAccessIndex::SYNC_IMAGE_LAYOUT_TRANSITION, src_exec_scope, src_access_scope);
    return DetectHazard(detector, image, subresource_range, false, options);
}

ResourceAccessRangeMap::iterator AccessContext::UpdateMemoryAccessStateFunctor::Infill(ResourceAccessRangeMap *accesses,
                                                                                       const Iterator &pos,
                                                                                       const ResourceAccessRange &range) const {
    // this is only called on gaps, and never returns a gap.
    ResourceAccessState default_state;
    context.ResolvePreviousAccess(range, accesses, &default_state);
    return accesses->lower_bound(range);
}
void AccessContext::UpdateMemoryAccessStateFunctor::operator()(const ResourceAccessRangeMap::iterator &pos) const {
    auto &access_state = pos->second;
    access_state.Update(usage_info, ordering_rule, tag);
}

// This is called with the *recorded* command buffers access context, with the *active* access context pass in, againsts which
// hazards will be detected
HazardResult AccessContext::DetectFirstUseHazard(QueueId queue_id, const ResourceUsageRange &tag_range,
                                                 const AccessContext &access_context) const {
    HazardResult hazard;
    for (const auto &recorded_access : access_state_map_) {
        // Cull any entries not in the current tag range
        if (!recorded_access.second.FirstAccessInTagRange(tag_range)) continue;
        HazardDetectFirstUse detector(recorded_access.second, queue_id, tag_range);
        hazard = access_context.DetectHazardRange(detector, recorded_access.first, DetectOptions::kDetectAll);
        if (hazard.IsHazard()) break;
    }

    return hazard;
}

// For RenderPass time validation this is "start tag", for QueueSubmit, this is the earliest
// unsynchronized tag for the Queue being tested against (max synchrononous + 1, perhaps)
ResourceUsageTag AccessContext::AsyncReference::StartTag() const { return (tag_ == kInvalidTag) ? context_->StartTag() : tag_; }

AttachmentViewGen::AttachmentViewGen(const syncval_state::ImageViewState *image_view, const VkOffset3D &offset,
                                     const VkExtent3D &extent)
    : view_(image_view), view_mask_(image_view->normalized_subresource_range.aspectMask), gen_store_() {
    gen_store_[Gen::kViewSubresource].emplace(image_view->GetFullViewImageRangeGen());
    gen_store_[Gen::kRenderArea].emplace(image_view->MakeImageRangeGen(offset, extent));

    const auto depth = view_mask_ & VK_IMAGE_ASPECT_DEPTH_BIT;
    if (depth && (depth != view_mask_)) {
        gen_store_[Gen::kDepthOnlyRenderArea].emplace(image_view->MakeImageRangeGen(offset, extent, depth));
    }
    const auto stencil = view_mask_ & VK_IMAGE_ASPECT_STENCIL_BIT;
    if (stencil && (stencil != view_mask_)) {
        gen_store_[Gen::kStencilOnlyRenderArea].emplace(image_view->MakeImageRangeGen(offset, extent, stencil));
    }
}

const std::optional<ImageRangeGen> &AttachmentViewGen::GetRangeGen(AttachmentViewGen::Gen type) const {
    static_assert(Gen::kGenSize == 4, "Function written with this assumption");
    // If the view is a depth only view, then the depth only portion of the render area is simply the render area.
    // If the view is a depth stencil view, then the depth only portion of the render area will be a subset,
    // and thus needs the generator function that will produce the address ranges of that subset
    const bool depth_only = (type == kDepthOnlyRenderArea) && (view_mask_ == VK_IMAGE_ASPECT_DEPTH_BIT);
    const bool stencil_only = (type == kStencilOnlyRenderArea) && (view_mask_ == VK_IMAGE_ASPECT_STENCIL_BIT);
    if (depth_only || stencil_only) {
        type = Gen::kRenderArea;
    }
    return gen_store_[type];
}

AttachmentViewGen::Gen AttachmentViewGen::GetDepthStencilRenderAreaGenType(bool depth_op, bool stencil_op) const {
    assert(IsValid());
    assert(view_mask_ & (VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT));
    if (depth_op) {
        assert(view_mask_ & VK_IMAGE_ASPECT_DEPTH_BIT);
        if (stencil_op) {
            assert(view_mask_ & VK_IMAGE_ASPECT_STENCIL_BIT);
            return kRenderArea;
        }
        return kDepthOnlyRenderArea;
    }
    if (stencil_op) {
        assert(view_mask_ & VK_IMAGE_ASPECT_STENCIL_BIT);
        return kStencilOnlyRenderArea;
    }

    assert(depth_op || stencil_op);
    return kRenderArea;
}
