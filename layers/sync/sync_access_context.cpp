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

#include <vulkan/utility/vk_format_utils.h>
#include "state_tracker/buffer_state.h"
#include "state_tracker/video_session_state.h"
#include "state_tracker/render_pass_state.h"
#include "sync/sync_access_context.h"
#include "sync/sync_image.h"

namespace syncval {

bool SimpleBinding(const vvl::Bindable &bindable) { return !bindable.sparse && bindable.Binding(); }
VkDeviceSize ResourceBaseAddress(const vvl::Buffer &buffer) { return buffer.GetFakeBaseAddress(); }

class HazardDetector {
    const SyncAccessInfo &access_info_;

  public:
    HazardResult Detect(const AccessMap::const_iterator &pos) const { return pos->second.DetectHazard(access_info_); }
    HazardResult DetectAsync(const AccessMap::const_iterator &pos, ResourceUsageTag start_tag, QueueId queue_id) const {
        return pos->second.DetectAsyncHazard(access_info_, start_tag, queue_id);
    }
    explicit HazardDetector(SyncAccessIndex access_index) : access_info_(GetAccessInfo(access_index)) {}
};

class HazardDetectorWithOrdering {
    const SyncAccessInfo &access_info_;
    const SyncOrdering ordering_rule_;
    const SyncFlags flags_;

  public:
    HazardResult Detect(const AccessMap::const_iterator &pos) const {
        const OrderingBarrier &ordering = GetOrderingRules(ordering_rule_);
        return pos->second.DetectHazard(access_info_, ordering, flags_, kQueueIdInvalid);
    }
    HazardResult DetectAsync(const AccessMap::const_iterator &pos, ResourceUsageTag start_tag, QueueId queue_id) const {
        return pos->second.DetectAsyncHazard(access_info_, start_tag, queue_id);
    }
    HazardDetectorWithOrdering(SyncAccessIndex access_index, SyncOrdering ordering, SyncFlags flags = 0)
        : access_info_(GetAccessInfo(access_index)), ordering_rule_(ordering), flags_(flags) {}
};

class HazardDetectFirstUse {
  public:
    HazardDetectFirstUse(const AccessState &recorded_use, QueueId queue_id, const ResourceUsageRange &tag_range)
        : recorded_use_(recorded_use), queue_id_(queue_id), tag_range_(tag_range) {}
    HazardResult Detect(const AccessMap::const_iterator &pos) const {
        return pos->second.DetectHazard(recorded_use_, queue_id_, tag_range_);
    }
    HazardResult DetectAsync(const AccessMap::const_iterator &pos, ResourceUsageTag start_tag, QueueId queue_id) const {
        return pos->second.DetectAsyncHazard(recorded_use_, tag_range_, start_tag, queue_id);
    }

  private:
    const AccessState &recorded_use_;
    const QueueId queue_id_;
    const ResourceUsageRange &tag_range_;
};

struct HazardDetectorMarker {
    HazardResult Detect(const AccessMap::const_iterator &pos) const { return pos->second.DetectMarkerHazard(); }
    HazardResult DetectAsync(const AccessMap::const_iterator &pos, ResourceUsageTag start_tag,
                             QueueId queue_id) const {
        return pos->second.DetectAsyncHazard(GetAccessInfo(SYNC_COPY_TRANSFER_WRITE), start_tag, queue_id);
    }
};

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

void AccessContext::InitFrom(const AccessContext &other) {
    access_state_map_ = other.access_state_map_;
    prev_ = other.prev_;
    prev_by_subpass_ = other.prev_by_subpass_;
    async_ = other.async_;
    src_external_ = other.src_external_;
    dst_external_ = other.dst_external_;
    start_tag_ = other.start_tag_;

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
    finalized_ = false;
    sorted_first_accesses_.Clear();
}

void AccessContext::Finalize() {
    assert(!finalized_);  // no need to finalize finalized
    sorted_first_accesses_.Init(access_state_map_);
    finalized_ = true;
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
    from.ResolveAccessRange(kFullRange, noop_action, &access_state_map_, false);
}

void AccessContext::ResolvePreviousAccess(const AccessRange &range, AccessMap *descent_map, bool infill,
                                          const AccessStateFunction *previous_barrier) const {
    if (prev_.empty()) {
        if (range.non_empty() && infill) {
            // Fill the empty poritions of descent_map with the default_state with the barrier function applied (iff present)
            AccessState access_state;
            if (previous_barrier) {
                (*previous_barrier)(&access_state);
            }
            UpdateRangeValue(*descent_map, range, access_state);
        }
    } else {
        // Look for something to fill the gap further along.
        for (const auto &prev_dep : prev_) {
            const ApplyTrackbackStackAction barrier_action(prev_dep.barriers, previous_barrier);
            prev_dep.source_subpass->ResolveAccessRange(range, barrier_action, descent_map, infill);
        }
    }
}

// Non-lazy import of all accesses, WaitEvents needs this.
void AccessContext::ResolvePreviousAccesses() {
    assert(!finalized_);
    if (!prev_.size()) {
        return;  // If no previous contexts, nothing to do
    }
    ResolvePreviousAccess(kFullRange, &access_state_map_, true);
}

void AccessContext::UpdateAccessState(const vvl::Buffer &buffer, SyncAccessIndex current_usage, SyncOrdering ordering_rule,
                                      const AccessRange &range, ResourceUsageTagEx tag_ex, SyncFlags flags) {
    if (current_usage == SYNC_ACCESS_INDEX_NONE) {
        return;
    }
    if (!SimpleBinding(buffer)) {
        return;
    }
    const auto base_address = ResourceBaseAddress(buffer);
    UpdateMemoryAccessStateFunctor action(*this, current_usage, ordering_rule, tag_ex, flags);
    UpdateMemoryAccessRangeState(action, range + base_address);
}

void AccessContext::UpdateAccessState(const vvl::Image &image, SyncAccessIndex current_usage, SyncOrdering ordering_rule,
                                      const VkImageSubresourceRange &subresource_range, const ResourceUsageTag &tag) {
    // range_gen is non-temporary to avoid an additional copy
    const auto &sub_state = SubState(image);
    ImageRangeGen range_gen = sub_state.MakeImageRangeGen(subresource_range, false);
    UpdateAccessState(range_gen, current_usage, ordering_rule, ResourceUsageTagEx{tag});
}
void AccessContext::UpdateAccessState(const vvl::Image &image, SyncAccessIndex current_usage, SyncOrdering ordering_rule,
                                      const VkImageSubresourceRange &subresource_range, const VkOffset3D &offset,
                                      const VkExtent3D &extent, const ResourceUsageTagEx tag_ex) {
    // range_gen is non-temporary to avoid an additional copy
    const auto &sub_state = SubState(image);
    ImageRangeGen range_gen = sub_state.MakeImageRangeGen(subresource_range, offset, extent, false);
    UpdateAccessState(range_gen, current_usage, ordering_rule, tag_ex);
}

void AccessContext::UpdateAccessState(const vvl::ImageView &image_view, SyncAccessIndex current_usage, SyncOrdering ordering_rule,
                                      const VkOffset3D &offset, const VkExtent3D &extent, const ResourceUsageTagEx tag_ex) {
    // range_gen is non-temporary to avoid an additional copy
    ImageRangeGen range_gen(MakeImageRangeGen(image_view, offset, extent));
    UpdateAccessState(range_gen, current_usage, ordering_rule, tag_ex);
}

void AccessContext::UpdateAccessState(const vvl::ImageView &image_view, SyncAccessIndex current_usage, SyncOrdering ordering_rule,
                                      ResourceUsageTagEx tag_ex) {
    auto range_gen = MakeImageRangeGen(image_view);
    UpdateAccessState(range_gen, current_usage, ordering_rule, tag_ex);
}

void AccessContext::UpdateAccessState(const AttachmentViewGen &view_gen, AttachmentViewGen::Gen gen_type,
                                      SyncAccessIndex current_usage, SyncOrdering ordering_rule, const ResourceUsageTag tag,
                                      SyncFlags flags) {
    const std::optional<ImageRangeGen> &attachment_gen = view_gen.GetRangeGen(gen_type);
    if (attachment_gen) {
        // Value of const optional is const, and will be copied in callee
        UpdateAccessState(*attachment_gen, current_usage, ordering_rule, ResourceUsageTagEx{tag}, flags);
    }
}

void AccessContext::UpdateAccessState(const vvl::VideoSession &vs_state, const vvl::VideoPictureResource &resource,
                                      SyncAccessIndex current_usage, ResourceUsageTag tag) {
    const auto image = static_cast<const vvl::Image *>(resource.image_state.get());
    const auto offset = resource.GetEffectiveImageOffset(vs_state);
    const auto extent = resource.GetEffectiveImageExtent(vs_state);
    const auto &sub_state = SubState(*image);
    ImageRangeGen range_gen(sub_state.MakeImageRangeGen(resource.range, offset, extent, false));
    UpdateAccessState(range_gen, current_usage, SyncOrdering::kNonAttachment, ResourceUsageTagEx{tag});
}

void AccessContext::UpdateAccessState(ImageRangeGen &range_gen, SyncAccessIndex current_usage, SyncOrdering ordering_rule,
                                      ResourceUsageTagEx tag_ex, SyncFlags flags) {
    if (current_usage == SYNC_ACCESS_INDEX_NONE) {
        return;
    }
    UpdateMemoryAccessStateFunctor action(*this, current_usage, ordering_rule, tag_ex, flags);
    UpdateMemoryAccessState(action, range_gen);
}

void AccessContext::UpdateAccessState(const ImageRangeGen &range_gen, SyncAccessIndex current_usage, SyncOrdering ordering_rule,
                                      ResourceUsageTagEx tag_ex, SyncFlags flags) {
    // range_gen is non-temporary to avoid infinite call recursion
    ImageRangeGen mutable_range_gen(range_gen);
    UpdateAccessState(mutable_range_gen, current_usage, ordering_rule, tag_ex, flags);
}

void AccessContext::ResolveChildContexts(vvl::span<AccessContext> subpass_contexts) {
    assert(!finalized_);
    for (AccessContext &context : subpass_contexts) {
        ApplyTrackbackStackAction barrier_action(context.GetDstExternalTrackBack().barriers);
        context.ResolveAccessRange(kFullRange, barrier_action, &access_state_map_, false, false);
    }
}

// Caller must ensure that lifespan of this is less than the lifespan of from
void AccessContext::ImportAsyncContexts(const AccessContext &from) {
    async_.insert(async_.end(), from.async_.begin(), from.async_.end());
}

// Suitable only for *subpass* access contexts
HazardResult AccessContext::DetectSubpassTransitionHazard(const SubpassBarrierTrackback &track_back,
                                                          const AttachmentViewGen &attach_view) const {
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

void AccessContext::AddAsyncContext(const AccessContext *context, ResourceUsageTag tag, QueueId queue_id) {
    if (context) {
        async_.emplace_back(*context, tag, queue_id);
    }
}

HazardResult AccessContext::DetectHazard(const vvl::Buffer &buffer, SyncAccessIndex access_index, const AccessRange &range) const {
    if (!SimpleBinding(buffer)) return HazardResult();
    const auto base_address = ResourceBaseAddress(buffer);
    HazardDetector detector(access_index);
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
HazardResult AccessContext::DetectHazard(Detector &detector, const vvl::Image &image,
                                         const VkImageSubresourceRange &subresource_range, const VkOffset3D &offset,
                                         const VkExtent3D &extent, bool is_depth_sliced, DetectOptions options) const {
    // range_gen is non-temporary to avoid additional copy
    const auto &sub_state = SubState(image);
    ImageRangeGen range_gen = sub_state.MakeImageRangeGen(subresource_range, offset, extent, is_depth_sliced);
    return DetectHazardGeneratedRanges(detector, range_gen, options);
}

template <typename Detector>
HazardResult AccessContext::DetectHazard(Detector &detector, const vvl::Image &image,
                                         const VkImageSubresourceRange &subresource_range, bool is_depth_sliced,
                                         DetectOptions options) const {
    // range_gen is non-temporary to avoid additional copy
    const auto &sub_state = SubState(image);
    ImageRangeGen range_gen = sub_state.MakeImageRangeGen(subresource_range, is_depth_sliced);
    return DetectHazardGeneratedRanges(detector, range_gen, options);
}

HazardResult AccessContext::DetectHazard(const vvl::Image &image, SyncAccessIndex current_usage,
                                         const VkImageSubresourceRange &subresource_range, bool is_depth_sliced) const {
    HazardDetector detector(current_usage);
    return DetectHazard(detector, image, subresource_range, is_depth_sliced, DetectOptions::kDetectAll);
}

HazardResult AccessContext::DetectHazard(const vvl::ImageView &image_view, SyncAccessIndex current_usage) const {
    // Get is const, but callee will copy
    HazardDetector detector(current_usage);
    auto range_gen = MakeImageRangeGen(image_view);
    return DetectHazardGeneratedRanges(detector, range_gen, DetectOptions::kDetectAll);
}

HazardResult AccessContext::DetectHazard(const ImageRangeGen &ref_range_gen, SyncAccessIndex current_usage,
                                         const SyncOrdering ordering_rule, SyncFlags flags) const {
    if (ordering_rule == SyncOrdering::kOrderingNone) {
        HazardDetector detector(current_usage);
        return DetectHazardGeneratedRanges(detector, ref_range_gen, DetectOptions::kDetectAll);
    }

    HazardDetectorWithOrdering detector(current_usage, ordering_rule, flags);
    return DetectHazardGeneratedRanges(detector, ref_range_gen, DetectOptions::kDetectAll);
}

HazardResult AccessContext::DetectHazard(const vvl::ImageView &image_view, const VkOffset3D &offset, const VkExtent3D &extent,
                                         SyncAccessIndex current_usage, SyncOrdering ordering_rule) const {
    // range_gen is non-temporary to avoid an additional copy
    ImageRangeGen range_gen(MakeImageRangeGen(image_view, offset, extent));
    HazardDetectorWithOrdering detector(current_usage, ordering_rule);
    return DetectHazardGeneratedRanges(detector, range_gen, DetectOptions::kDetectAll);
}

HazardResult AccessContext::DetectHazard(const AttachmentViewGen &view_gen, AttachmentViewGen::Gen gen_type,
                                         SyncAccessIndex current_usage, SyncOrdering ordering_rule, SyncFlags flags) const {
    HazardDetectorWithOrdering detector(current_usage, ordering_rule, flags);
    return DetectHazard(detector, view_gen, gen_type, DetectOptions::kDetectAll);
}

HazardResult AccessContext::DetectHazard(const vvl::VideoSession &vs_state, const vvl::VideoPictureResource &resource,
                                         SyncAccessIndex current_usage) const {
    const auto image = static_cast<const vvl::Image *>(resource.image_state.get());
    const auto &sub_state = SubState(*image);
    const auto offset = resource.GetEffectiveImageOffset(vs_state);
    const auto extent = resource.GetEffectiveImageExtent(vs_state);
    ImageRangeGen range_gen(sub_state.MakeImageRangeGen(resource.range, offset, extent, false));
    HazardDetector detector(current_usage);
    return DetectHazardGeneratedRanges(detector, range_gen, DetectOptions::kDetectAll);
}

HazardResult AccessContext::DetectHazard(const vvl::Image &image, const VkImageSubresourceRange &subresource_range,
                                         const VkOffset3D &offset, const VkExtent3D &extent, bool is_depth_sliced,
                                         SyncAccessIndex current_usage, SyncOrdering ordering_rule) const {
    if (ordering_rule == SyncOrdering::kOrderingNone) {
        HazardDetector detector(current_usage);
        return DetectHazard(detector, image, subresource_range, offset, extent, is_depth_sliced, DetectOptions::kDetectAll);
    }
    HazardDetectorWithOrdering detector(current_usage, ordering_rule);
    return DetectHazard(detector, image, subresource_range, offset, extent, is_depth_sliced, DetectOptions::kDetectAll);
}

class BarrierHazardDetector {
  public:
    BarrierHazardDetector(SyncAccessIndex access_index, VkPipelineStageFlags2 src_exec_scope, SyncAccessFlags src_access_scope)
        : access_info_(GetAccessInfo(access_index)), src_exec_scope_(src_exec_scope), src_access_scope_(src_access_scope) {}

    HazardResult Detect(const AccessMap::const_iterator &pos) const {
        return pos->second.DetectBarrierHazard(access_info_, kQueueIdInvalid, src_exec_scope_, src_access_scope_);
    }
    HazardResult DetectAsync(const AccessMap::const_iterator &pos, ResourceUsageTag start_tag, QueueId queue_id) const {
        // Async barrier hazard detection can use the same path as the usage index is not IsRead, but is IsWrite
        return pos->second.DetectAsyncHazard(access_info_, start_tag, queue_id);
    }

  private:
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
    void ScopeSeek(const AccessRange &range) { scope_pos_ = event_scope_.LowerBound(range); }

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

HazardResult AccessContext::DetectImageBarrierHazard(const vvl::Image &image, const VkImageSubresourceRange &subresource_range,
                                                     VkPipelineStageFlags2 src_exec_scope, const SyncAccessFlags &src_access_scope,
                                                     QueueId queue_id, const ScopeMap &scope_map, const ResourceUsageTag scope_tag,
                                                     AccessContext::DetectOptions options) const {
    EventBarrierHazardDetector detector(SyncAccessIndex::SYNC_IMAGE_LAYOUT_TRANSITION, src_exec_scope, src_access_scope, scope_map,
                                        queue_id, scope_tag);
    return DetectHazard(detector, image, subresource_range, false, options);
}

HazardResult AccessContext::DetectImageBarrierHazard(const AttachmentViewGen &view_gen, const SyncBarrier &barrier,
                                                     DetectOptions options) const {
    BarrierHazardDetector detector(SyncAccessIndex::SYNC_IMAGE_LAYOUT_TRANSITION, barrier.src_exec_scope.exec_scope,
                                   barrier.src_access_scope);
    return DetectHazard(detector, view_gen, AttachmentViewGen::Gen::kViewSubresource, options);
}

HazardResult AccessContext::DetectImageBarrierHazard(const vvl::Image &image, VkPipelineStageFlags2 src_exec_scope,
                                                     const SyncAccessFlags &src_access_scope,
                                                     const VkImageSubresourceRange &subresource_range, bool is_depth_sliced,
                                                     const DetectOptions options) const {
    BarrierHazardDetector detector(SyncAccessIndex::SYNC_IMAGE_LAYOUT_TRANSITION, src_exec_scope, src_access_scope);
    return DetectHazard(detector, image, subresource_range, is_depth_sliced, options);
}

AccessMap::iterator AccessContext::UpdateMemoryAccessStateFunctor::Infill(AccessMap *accesses, const Iterator &pos_hint,
                                                                          const AccessRange &range) const {
    // this is only called on gaps, and never returns a gap.
    context.ResolvePreviousAccess(range, accesses, true);
    return accesses->LowerBound(range);
}
void AccessContext::UpdateMemoryAccessStateFunctor::operator()(const AccessMap::iterator &pos) const {
    auto &access_state = pos->second;
    access_state.Update(usage_info, ordering_rule, tag_ex, flags);
}

// This is called with the *recorded* command buffers access context, with the *active* access context pass in, againsts which
// hazards will be detected
HazardResult AccessContext::DetectFirstUseHazard(QueueId queue_id, const ResourceUsageRange &tag_range,
                                                 const AccessContext &access_context) const {
    // If the context is finalized we have a fast path to find first accesses within a range
    if (finalized_) {
        for (const auto &single_tag : sorted_first_accesses_.IterateSingleTagFirstAccesses(tag_range)) {
            const AccessRange access_range = single_tag.p_key_value->first;
            const AccessState &access = single_tag.p_key_value->second;

            // For single tag first accesses we have exact search and can assert the find
            assert(access.FirstAccessInTagRange(tag_range));

            HazardDetectFirstUse detector(access, queue_id, tag_range);
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

            HazardDetectFirstUse detector(access, queue_id, tag_range);
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
            HazardDetectFirstUse detector(recorded_access.second, queue_id, tag_range);
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
    HazardDetectorMarker detector;
    return DetectHazardRange(detector, (range + base_address), DetectOptions::kDetectAll);
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
    assert(IsValid());
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
