/*
 * Copyright (c) 2019-2020 Valve Corporation
 * Copyright (c) 2019-2020 LunarG, Inc.
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
 *
 * Author: John Zulauf <jzulauf@lunarg.com>
 */

#pragma once

#include <limits>
#include <map>
#include <memory>
#include <unordered_map>
#include <vulkan/vulkan.h>

#include "synchronization_validation_types.h"
#include "state_tracker.h"

class SyncValidator;
class ResourceAccessState;

enum SyncHazard {
    NONE = 0,
    READ_AFTER_WRITE,
    WRITE_AFTER_READ,
    WRITE_AFTER_WRITE,
    READ_RACING_WRITE,
    WRITE_RACING_WRITE,
    WRITE_RACING_READ,
};

// Useful Utilites for manipulating StageAccess parameters, suitable as base class to save typing
struct SyncStageAccess {
    static inline SyncStageAccessFlagBits FlagBit(SyncStageAccessIndex stage_access) {
        return syncStageAccessInfoByStageAccessIndex[stage_access].stage_access_bit;
    }
    static inline SyncStageAccessFlags Flags(SyncStageAccessIndex stage_access) {
        return static_cast<SyncStageAccessFlags>(FlagBit(stage_access));
    }

    static bool IsRead(SyncStageAccessFlagBits stage_access_bit) { return 0 != (stage_access_bit & syncStageAccessReadMask); }
    static bool IsRead(SyncStageAccessIndex stage_access_index) { return IsRead(FlagBit(stage_access_index)); }

    static bool IsWrite(SyncStageAccessFlagBits stage_access_bit) { return 0 != (stage_access_bit & syncStageAccessWriteMask); }
    static bool HasWrite(SyncStageAccessFlags stage_access_mask) { return 0 != (stage_access_mask & syncStageAccessWriteMask); }
    static bool IsWrite(SyncStageAccessIndex stage_access_index) { return IsWrite(FlagBit(stage_access_index)); }
    static VkPipelineStageFlagBits PipelineStageBit(SyncStageAccessIndex stage_access_index) {
        return syncStageAccessInfoByStageAccessIndex[stage_access_index].stage_mask;
    }
    static SyncStageAccessFlags AccessScopeByStage(VkPipelineStageFlags stages);
    static SyncStageAccessFlags AccessScopeByAccess(VkAccessFlags access);
    static SyncStageAccessFlags AccessScope(VkPipelineStageFlags stages, VkAccessFlags access);
    static SyncStageAccessFlags AccessScope(SyncStageAccessFlags stage_scope, VkAccessFlags accesses) {
        return stage_scope & AccessScopeByAccess(accesses);
    }
};

struct ResourceUsageTag {
    uint64_t index;
    CMD_TYPE command;
    const static uint64_t kMaxIndex = std::numeric_limits<uint64_t>::max();
    ResourceUsageTag &operator++() {
        index++;
        return *this;
    }
    bool IsBefore(const ResourceUsageTag &rhs) const { return index < rhs.index; }
    bool operator==(const ResourceUsageTag &rhs) const { return (index == rhs.index); }
    bool operator!=(const ResourceUsageTag &rhs) const { return !(*this == rhs); }
    ResourceUsageTag() : index(0), command(CMD_NONE) {}
    ResourceUsageTag(uint64_t index_, CMD_TYPE command_) : index(index_), command(command_) {}
};

struct HazardResult {
    std::unique_ptr<const ResourceAccessState> access_state;
    SyncStageAccessIndex usage_index = std::numeric_limits<SyncStageAccessIndex>::max();
    SyncHazard hazard = NONE;
    SyncStageAccessFlags prior_access = 0U;  // TODO -- change to a NONE enum in ...Bits
    ResourceUsageTag tag = ResourceUsageTag();
    void Set(const ResourceAccessState *access_state_, SyncStageAccessIndex usage_index_, SyncHazard hazard_,
             SyncStageAccessFlags prior_, const ResourceUsageTag &tag_);
};

struct SyncBarrier {
    VkPipelineStageFlags src_exec_scope;
    SyncStageAccessFlags src_access_scope;
    VkPipelineStageFlags dst_exec_scope;
    SyncStageAccessFlags dst_access_scope;
    SyncBarrier() = default;
    SyncBarrier &operator=(const SyncBarrier &) = default;
    SyncBarrier(VkQueueFlags gueue_flags, const VkSubpassDependency2 &sub_pass_barrier);
    void Merge(const SyncBarrier &other) {
        src_exec_scope |= other.src_exec_scope;
        src_access_scope |= other.src_access_scope;
        dst_exec_scope |= other.dst_exec_scope;
        dst_access_scope |= other.dst_access_scope;
    }
    SyncBarrier(VkPipelineStageFlags src_exec_scope_, SyncStageAccessFlags src_access_scope_, VkPipelineStageFlags dst_exec_scope_,
                SyncStageAccessFlags dst_access_scope_)
        : src_exec_scope(src_exec_scope_),
          src_access_scope(src_access_scope_),
          dst_exec_scope(dst_exec_scope_),
          dst_access_scope(dst_access_scope_) {}
    SyncBarrier(const SyncBarrier &other) = default;
};

// To represent ordering guarantees such as rasterization and store
struct SyncOrderingBarrier {
    VkPipelineStageFlags exec_scope;
    SyncStageAccessFlags access_scope;
    SyncOrderingBarrier() = default;
    SyncOrderingBarrier &operator=(const SyncOrderingBarrier &) = default;
};

class ResourceAccessState : public SyncStageAccess {
  protected:
    // Mutliple read operations can be simlutaneously (and independently) synchronized,
    // given the only the second execution scope creates a dependency chain, we have to track each,
    // but only up to one per pipeline stage (as another read from the *same* stage become more recent,
    // and applicable one for hazard detection
    struct ReadState {
        VkPipelineStageFlagBits stage;  // The stage of this read
        SyncStageAccessFlags access;    // TODO: Change to FlagBits when we have a None bit enum
                                        // TODO: Revisit whether this needs to support multiple reads per stage
        VkPipelineStageFlags barriers;  // all applicable barriered stages
        ResourceUsageTag tag;
        VkPipelineStageFlags pending_dep_chain;  // Should be zero except during barrier application
                                                 // Excluded from comparison
        ReadState() = default;
        bool operator==(const ReadState &rhs) const {
            bool same = (stage == rhs.stage) && (access == rhs.access) && (barriers == rhs.barriers) && (tag == rhs.tag);
            return same;
        }
        bool operator!=(const ReadState &rhs) const { return !(*this == rhs); }
        inline void Set(VkPipelineStageFlagBits stage_, SyncStageAccessFlags access_, VkPipelineStageFlags barriers_,
                        const ResourceUsageTag &tag_) {
            stage = stage_;
            access = access_;
            barriers = barriers_;
            tag = tag_;
            pending_dep_chain = 0;  // If this is a new read, we aren't applying a barrier set.
        }
    };

  public:
    HazardResult DetectHazard(SyncStageAccessIndex usage_index) const;
    HazardResult DetectHazard(SyncStageAccessIndex usage_index, const SyncOrderingBarrier &ordering) const;

    HazardResult DetectBarrierHazard(SyncStageAccessIndex usage_index, VkPipelineStageFlags source_exec_scope,
                                     SyncStageAccessFlags source_access_scope) const;
    HazardResult DetectAsyncHazard(SyncStageAccessIndex usage_index) const;

    void Update(SyncStageAccessIndex usage_index, const ResourceUsageTag &tag);
    void SetWrite(SyncStageAccessFlagBits usage_bit, const ResourceUsageTag &tag);
    void Resolve(const ResourceAccessState &other);
    void ApplyBarriers(const std::vector<SyncBarrier> &barriers, bool layout_transition);
    void ApplyBarriers(const std::vector<SyncBarrier> &barriers, const ResourceUsageTag &tag);
    void ApplyBarrier(const SyncBarrier &barrier, bool layout_transition);
    void ApplyPendingBarriers(const ResourceUsageTag &tag);

    ResourceAccessState()
        : write_barriers(~SyncStageAccessFlags(0)),
          write_dependency_chain(0),
          write_tag(),
          last_write(0),
          input_attachment_read(false),
          last_read_count(0),
          last_read_stages(0),
          read_execution_barriers(0),
          pending_write_dep_chain(0),
          pending_layout_transition(false),
          pending_write_barriers(0) {}

    bool HasPendingState() const {
        return (0 != pending_layout_transition) || (0 != pending_write_barriers) || (0 != pending_write_dep_chain);
    }
    bool HasWriteOp() const { return last_write != 0; }
    bool operator==(const ResourceAccessState &rhs) const {
        bool same = (write_barriers == rhs.write_barriers) && (write_dependency_chain == rhs.write_dependency_chain) &&
                    (last_read_count == rhs.last_read_count) && (last_read_stages == rhs.last_read_stages) &&
                    (write_tag == rhs.write_tag) && (input_attachment_read == rhs.input_attachment_read) &&
                    (read_execution_barriers == rhs.read_execution_barriers);
        for (uint32_t i = 0; same && i < last_read_count; i++) {
            same &= last_reads[i] == rhs.last_reads[i];
        }
        return same;
    }
    bool operator!=(const ResourceAccessState &rhs) const { return !(*this == rhs); }
    VkPipelineStageFlags GetReadBarriers(SyncStageAccessFlags usage) const;
    SyncStageAccessFlags GetWriteBarriers() const { return write_barriers; }

  private:
    static constexpr VkPipelineStageFlags kInvalidAttachmentStage = ~VkPipelineStageFlags(0);
    bool IsWriteHazard(SyncStageAccessFlagBits usage) const { return 0 != (usage & ~write_barriers); }
    bool IsRAWHazard(VkPipelineStageFlagBits usage_stage, SyncStageAccessFlagBits usage) const;
    bool InSourceScopeOrChain(VkPipelineStageFlags src_exec_scope, SyncStageAccessFlags src_access_scope) const {
        return (src_access_scope & last_write) || (write_dependency_chain & src_exec_scope);
    }

    static bool IsReadHazard(VkPipelineStageFlagBits stage, const VkPipelineStageFlags barriers) {
        return 0 != (stage & ~barriers);
    }
    static bool IsReadHazard(VkPipelineStageFlags stage_mask, const VkPipelineStageFlags barriers) {
        return stage_mask != (stage_mask & barriers);
    }

    bool IsReadHazard(VkPipelineStageFlagBits stage, const ReadState &read_access) const {
        return IsReadHazard(stage, read_access.barriers);
    }
    bool IsReadHazard(VkPipelineStageFlags stage_mask, const ReadState &read_access) const {
        return IsReadHazard(stage_mask, read_access.barriers);
    }
    VkPipelineStageFlags GetOrderedStages(const SyncOrderingBarrier &ordering) const;
    ReadState *GetReadStateForStage(VkPipelineStageFlagBits stage, uint32_t search_limit = kStageCount);

    // TODO: Add a NONE (zero) enum to SyncStageAccessFlagBits for input_attachment_read and last_write

    // With reads, each must be "safe" relative to it's prior write, so we need only
    // save the most recent write operation (as anything *transitively* unsafe would arleady
    // be included
    SyncStageAccessFlags write_barriers;          // union of applicable barrier masks since last write
    VkPipelineStageFlags write_dependency_chain;  // intiially zero, but accumulating the dstStages of barriers if they chain.
    ResourceUsageTag write_tag;
    SyncStageAccessFlags last_write;  // only the most recent write

    // TODO Input Attachment cleanup for multiple reads in a given stage
    // Tracks whether the fragment shader read is input attachment read
    bool input_attachment_read;

    uint32_t last_read_count;
    VkPipelineStageFlags last_read_stages;
    VkPipelineStageFlags read_execution_barriers;
    static constexpr size_t kStageCount = 32;  // TODO: The manual count was 28 real stages. Add stage count to codegen
    std::array<ReadState, kStageCount> last_reads;

    // Pending execution state to support independent parallel barriers
    VkPipelineStageFlags pending_write_dep_chain;
    bool pending_layout_transition;
    SyncStageAccessFlags pending_write_barriers;
};

using ResourceAccessRangeMap = sparse_container::range_map<VkDeviceSize, ResourceAccessState>;
using ResourceAccessRange = typename ResourceAccessRangeMap::key_type;
using ResourceRangeMergeIterator = sparse_container::parallel_iterator<ResourceAccessRangeMap, const ResourceAccessRangeMap>;

class AccessContext {
  public:
    enum AddressType : int { kLinearAddress = 0, kIdealizedAddress = 1, kMaxAddressType = 1 };
    enum DetectOptions : uint32_t {
        kDetectPrevious = 1U << 0,
        kDetectAsync = 1U << 1,
        kDetectAll = (kDetectPrevious | kDetectAsync)
    };
    constexpr static int kAddressTypeCount = AddressType::kMaxAddressType + 1;
    static const std::array<AddressType, kAddressTypeCount> kAddressTypes;
    using MapArray = std::array<ResourceAccessRangeMap, kAddressTypeCount>;

    // WIP TODO WIP Multi-dep -- change track back to support barrier vector, not just last.
    struct TrackBack {
        std::vector<SyncBarrier> barriers;
        const AccessContext *context;
        TrackBack(const AccessContext *context_, VkQueueFlags queue_flags_,
                  const std::vector<const VkSubpassDependency2 *> &subpass_dependencies_)
            : barriers(), context(context_) {
            barriers.reserve(subpass_dependencies_.size());
            for (const VkSubpassDependency2 *dependency : subpass_dependencies_) {
                assert(dependency);
                barriers.emplace_back(queue_flags_, *dependency);
            }
        }

        TrackBack &operator=(const TrackBack &) = default;
        TrackBack() = default;
    };

    HazardResult DetectHazard(const BUFFER_STATE &buffer, SyncStageAccessIndex usage_index, const ResourceAccessRange &range) const;
    HazardResult DetectHazard(const IMAGE_STATE &image, SyncStageAccessIndex current_usage,
                              const VkImageSubresourceLayers &subresource, const VkOffset3D &offset,
                              const VkExtent3D &extent) const;
    template <typename Detector>
    HazardResult DetectHazard(Detector &detector, const IMAGE_STATE &image, const VkImageSubresourceRange &subresource_range,
                              const VkOffset3D &offset, const VkExtent3D &extent, DetectOptions options) const;
    HazardResult DetectHazard(const IMAGE_STATE &image, SyncStageAccessIndex current_usage,
                              const VkImageSubresourceRange &subresource_range, const VkOffset3D &offset,
                              const VkExtent3D &extent) const;
    HazardResult DetectHazard(const IMAGE_STATE &image, SyncStageAccessIndex current_usage,
                              const VkImageSubresourceRange &subresource_range, const SyncOrderingBarrier &ordering,
                              const VkOffset3D &offset, const VkExtent3D &extent) const;
    HazardResult DetectHazard(const IMAGE_VIEW_STATE *view, SyncStageAccessIndex current_usage, const SyncOrderingBarrier &ordering,
                              const VkOffset3D &offset, const VkExtent3D &extent, VkImageAspectFlags aspect_mask = 0U) const;
    HazardResult DetectImageBarrierHazard(const IMAGE_STATE &image, VkPipelineStageFlags src_exec_scope,
                                          SyncStageAccessFlags src_access_scope, const VkImageSubresourceRange &subresource_range,
                                          DetectOptions options) const;
    HazardResult DetectImageBarrierHazard(const IMAGE_STATE &image, VkPipelineStageFlags src_exec_scope,
                                          SyncStageAccessFlags src_stage_accesses, const VkImageMemoryBarrier &barrier) const;
    HazardResult DetectSubpassTransitionHazard(const TrackBack &track_back, const IMAGE_VIEW_STATE *attach_view) const;

    void RecordLayoutTransitions(const RENDER_PASS_STATE &rp_state, uint32_t subpass,
                                 const std::vector<const IMAGE_VIEW_STATE *> &attachment_views, const ResourceUsageTag &tag);

    const TrackBack &GetDstExternalTrackBack() const { return dst_external_; }
    void Reset() {
        prev_.clear();
        prev_by_subpass_.clear();
        async_.clear();
        src_external_ = TrackBack();
        dst_external_ = TrackBack();
        for (auto &map : access_state_maps_) {
            map.clear();
        }
    }

    // Follow the context previous to access the access state, supporting "lazy" import into the context. Not intended for
    // subpass layout transition, as the pending state handling is more complex
    // TODO: See if returning the lower_bound would be useful from a performance POV -- look at the lower_bound overhead
    // Would need to add a "hint" overload to parallel_iterator::invalidate_[AB] call, if so.
    void ResolvePreviousAccess(AddressType type, const ResourceAccessRange &range, ResourceAccessRangeMap *descent_map,
                               const ResourceAccessState *infill_state) const;
    template <typename BarrierAction>
    void ResolveAccessRange(const IMAGE_STATE &image_state, const VkImageSubresourceRange &subresource_range,
                            BarrierAction &barrier_action, AddressType address_type, ResourceAccessRangeMap *descent_map,
                            const ResourceAccessState *infill_state) const;
    template <typename BarrierAction>
    void ResolveAccessRange(AddressType type, const ResourceAccessRange &range, BarrierAction &barrier_action,
                            ResourceAccessRangeMap *resolve_map, const ResourceAccessState *infill_state,
                            bool recur_to_infill = true) const;

    void UpdateAccessState(const BUFFER_STATE &buffer, SyncStageAccessIndex current_usage, const ResourceAccessRange &range,
                           const ResourceUsageTag &tag);
    void UpdateAccessState(const IMAGE_STATE &image, SyncStageAccessIndex current_usage,
                           const VkImageSubresourceRange &subresource_range, const VkOffset3D &offset, const VkExtent3D &extent,
                           const ResourceUsageTag &tag);
    void UpdateAccessState(const IMAGE_VIEW_STATE *view, SyncStageAccessIndex current_usage, const VkOffset3D &offset,
                           const VkExtent3D &extent, VkImageAspectFlags aspect_mask, const ResourceUsageTag &tag);
    void UpdateAccessState(const IMAGE_STATE &image, SyncStageAccessIndex current_usage,
                           const VkImageSubresourceLayers &subresource, const VkOffset3D &offset, const VkExtent3D &extent,
                           const ResourceUsageTag &tag);
    void UpdateAttachmentResolveAccess(const RENDER_PASS_STATE &rp_state, const VkRect2D &render_area,
                                       const std::vector<const IMAGE_VIEW_STATE *> &attachment_views, uint32_t subpass,
                                       const ResourceUsageTag &tag);
    void UpdateAttachmentStoreAccess(const RENDER_PASS_STATE &rp_state, const VkRect2D &render_area,
                                     const std::vector<const IMAGE_VIEW_STATE *> &attachment_views, uint32_t subpass,
                                     const ResourceUsageTag &tag);

    void ResolveChildContexts(const std::vector<AccessContext> &contexts);

    template <typename Action>
    void UpdateResourceAccess(const BUFFER_STATE &buffer, const ResourceAccessRange &range, const Action action);
    template <typename Action>
    void UpdateResourceAccess(const IMAGE_STATE &image, const VkImageSubresourceRange &subresource_range, const Action action);

    template <typename Action>
    void ApplyGlobalBarriers(const Action &barrier_action);

    static AddressType ImageAddressType(const IMAGE_STATE &image);

    AccessContext(uint32_t subpass, VkQueueFlags queue_flags, const std::vector<SubpassDependencyGraphNode> &dependencies,
                  const std::vector<AccessContext> &contexts, const AccessContext *external_context);

    AccessContext() { Reset(); }
    AccessContext(const AccessContext &copy_from) = default;

    ResourceAccessRangeMap &GetAccessStateMap(AddressType type) { return access_state_maps_[type]; }
    const ResourceAccessRangeMap &GetAccessStateMap(AddressType type) const { return access_state_maps_[type]; }
    ResourceAccessRangeMap &GetLinearMap() { return GetAccessStateMap(AddressType::kLinearAddress); }
    const ResourceAccessRangeMap &GetLinearMap() const { return GetAccessStateMap(AddressType::kLinearAddress); }
    ResourceAccessRangeMap &GetIdealizedMap() { return GetAccessStateMap(AddressType::kIdealizedAddress); }
    const ResourceAccessRangeMap &GetIdealizedMap() const { return GetAccessStateMap(AddressType::kIdealizedAddress); }
    const TrackBack *GetTrackBackFromSubpass(uint32_t subpass) const {
        if (subpass == VK_SUBPASS_EXTERNAL) {
            return &src_external_;
        } else {
            assert(subpass < prev_by_subpass_.size());
            return prev_by_subpass_[subpass];
        }
    }

    bool ValidateLayoutTransitions(const SyncValidator &sync_state,

                                   const RENDER_PASS_STATE &rp_state,

                                   const VkRect2D &render_area,

                                   uint32_t subpass, const std::vector<const IMAGE_VIEW_STATE *> &attachment_views,
                                   const char *func_name) const;
    bool ValidateLoadOperation(const SyncValidator &sync_state, const RENDER_PASS_STATE &rp_state, const VkRect2D &render_area,
                               uint32_t subpass, const std::vector<const IMAGE_VIEW_STATE *> &attachment_views,
                               const char *func_name) const;
    bool ValidateStoreOperation(const SyncValidator &sync_state, const RENDER_PASS_STATE &rp_state, const VkRect2D &render_area,
                                uint32_t subpass, const std::vector<const IMAGE_VIEW_STATE *> &attachment_views,
                                const char *func_name) const;
    bool ValidateResolveOperations(const SyncValidator &sync_state, const RENDER_PASS_STATE &rp_state, const VkRect2D &render_area,
                                   const std::vector<const IMAGE_VIEW_STATE *> &attachment_views, const char *func_name,
                                   uint32_t subpass) const;

  private:
    HazardResult DetectHazard(AddressType type, SyncStageAccessIndex usage_index, const ResourceAccessRange &range) const;
    HazardResult DetectBarrierHazard(AddressType type, SyncStageAccessIndex current_usage, VkPipelineStageFlags src_exec_scope,
                                     SyncStageAccessFlags src_access_scope, const ResourceAccessRange &range,
                                     DetectOptions options) const;

    template <typename Detector>
    HazardResult DetectHazard(AddressType type, const Detector &detector, const ResourceAccessRange &range,
                              DetectOptions options) const;
    template <typename Detector>
    HazardResult DetectAsyncHazard(AddressType type, const Detector &detector, const ResourceAccessRange &range) const;
    template <typename Detector>
    HazardResult DetectPreviousHazard(AddressType type, const Detector &detector, const ResourceAccessRange &range) const;
    void UpdateAccessState(AddressType type, SyncStageAccessIndex current_usage, const ResourceAccessRange &range,
                           const ResourceUsageTag &tag);

    MapArray access_state_maps_;
    std::vector<TrackBack> prev_;
    std::vector<TrackBack *> prev_by_subpass_;
    std::vector<AccessContext *> async_;
    TrackBack src_external_;
    TrackBack dst_external_;
};

class RenderPassAccessContext {
  public:
    RenderPassAccessContext() : rp_state_(nullptr), current_subpass_(0) {}

    bool ValidateDrawSubpassAttachment(const SyncValidator &sync_state, const CMD_BUFFER_STATE &cmd, const VkRect2D &render_area,
                                       const char *func_name) const;
    void RecordDrawSubpassAttachment(const CMD_BUFFER_STATE &cmd, const VkRect2D &render_area, const ResourceUsageTag &tag);
    bool ValidateNextSubpass(const SyncValidator &sync_state, const VkRect2D &render_area, const char *command_name) const;
    bool ValidateEndRenderPass(const SyncValidator &sync_state, const VkRect2D &render_area, const char *func_name) const;
    bool ValidateFinalSubpassLayoutTransitions(const SyncValidator &sync_state, const VkRect2D &render_area,
                                               const char *func_name) const;

    void RecordLayoutTransitions(const ResourceUsageTag &tag);
    void RecordLoadOperations(const VkRect2D &render_area, const ResourceUsageTag &tag);
    void RecordBeginRenderPass(const SyncValidator &state, const CMD_BUFFER_STATE &cb_state, const AccessContext *external_context,
                               VkQueueFlags queue_flags, const ResourceUsageTag &tag);
    void RecordNextSubpass(const VkRect2D &render_area, const ResourceUsageTag &tag);
    void RecordEndRenderPass(AccessContext *external_context, const VkRect2D &render_area, const ResourceUsageTag &tag);

    AccessContext &CurrentContext() { return subpass_contexts_[current_subpass_]; }
    const AccessContext &CurrentContext() const { return subpass_contexts_[current_subpass_]; }
    const std::vector<AccessContext> &GetContexts() const { return subpass_contexts_; }
    uint32_t GetCurrentSubpass() const { return current_subpass_; }
    const RENDER_PASS_STATE *GetRenderPassState() const { return rp_state_; }
    AccessContext *CreateStoreResolveProxy(const VkRect2D &render_area) const;

  private:
    const RENDER_PASS_STATE *rp_state_;
    uint32_t current_subpass_;
    std::vector<AccessContext> subpass_contexts_;
    std::vector<const IMAGE_VIEW_STATE *> attachment_views_;
};

class CommandBufferAccessContext {
  public:
    CommandBufferAccessContext()
        : command_number_(0),
          reset_count_(0),
          render_pass_contexts_(),
          cb_access_context_(),
          current_context_(&cb_access_context_),
          current_renderpass_context_(),
          cb_state_(),
          queue_flags_() {}
    CommandBufferAccessContext(SyncValidator &sync_validator, std::shared_ptr<CMD_BUFFER_STATE> &cb_state, VkQueueFlags queue_flags)
        : CommandBufferAccessContext() {
        cb_state_ = cb_state;
        sync_state_ = &sync_validator;
        queue_flags_ = queue_flags;
    }

    void Reset() {
        command_number_ = 0;
        reset_count_++;
        cb_access_context_.Reset();
        render_pass_contexts_.clear();
        current_context_ = &cb_access_context_;
        current_renderpass_context_ = nullptr;
    }

    AccessContext *GetCurrentAccessContext() { return current_context_; }
    const AccessContext *GetCurrentAccessContext() const { return current_context_; }
    void RecordBeginRenderPass(const ResourceUsageTag &tag);
    bool ValidateBeginRenderPass(const RENDER_PASS_STATE &render_pass, const VkRenderPassBeginInfo *pRenderPassBegin,
                                 const VkSubpassBeginInfoKHR *pSubpassBeginInfo, const char *func_name) const;
    bool ValidateDispatchDrawDescriptorSet(VkPipelineBindPoint pipelineBindPoint, const char *func_name) const;
    void RecordDispatchDrawDescriptorSet(VkPipelineBindPoint pipelineBindPoint, const ResourceUsageTag &tag);
    bool ValidateDrawVertex(uint32_t vertexCount, uint32_t firstVertex, const char *func_name) const;
    void RecordDrawVertex(uint32_t vertexCount, uint32_t firstVertex, const ResourceUsageTag &tag);
    bool ValidateDrawVertexIndex(uint32_t indexCount, uint32_t firstIndex, const char *func_name) const;
    void RecordDrawVertexIndex(uint32_t indexCount, uint32_t firstIndex, const ResourceUsageTag &tag);
    bool ValidateDrawSubpassAttachment(const char *func_name) const;
    void RecordDrawSubpassAttachment(const ResourceUsageTag &tag);
    bool ValidateNextSubpass(const char *func_name) const;
    bool ValidateEndRenderpass(const char *func_name) const;
    void RecordNextSubpass(const RENDER_PASS_STATE &render_pass, const ResourceUsageTag &tag);
    void RecordEndRenderPass(const RENDER_PASS_STATE &render_pass, const ResourceUsageTag &tag);
    CMD_BUFFER_STATE *GetCommandBufferState() { return cb_state_.get(); }
    const CMD_BUFFER_STATE *GetCommandBufferState() const { return cb_state_.get(); }
    VkQueueFlags GetQueueFlags() const { return queue_flags_; }
    inline ResourceUsageTag NextCommandTag(CMD_TYPE command) {
        // TODO: add command encoding to ResourceUsageTag.
        // What else we what to include.  Do we want some sort of "parent" or global sequence number
        command_number_++;
        const auto index = (static_cast<uint64_t>(reset_count_) << 32) | command_number_;
        ResourceUsageTag next(index, command);
        return next;
    }

  private:
    uint32_t command_number_;
    uint32_t reset_count_;
    std::vector<RenderPassAccessContext> render_pass_contexts_;
    AccessContext cb_access_context_;
    AccessContext *current_context_;
    RenderPassAccessContext *current_renderpass_context_;
    std::shared_ptr<CMD_BUFFER_STATE> cb_state_;
    SyncValidator *sync_state_;

    VkQueueFlags queue_flags_;
};

class SyncValidator : public ValidationStateTracker, public SyncStageAccess {
  public:
    SyncValidator() { container_type = LayerObjectTypeSyncValidation; }
    using StateTracker = ValidationStateTracker;

    using StateTracker::AccessorTraitsTypes;
    std::unordered_map<VkCommandBuffer, std::unique_ptr<CommandBufferAccessContext>> cb_access_state;
    CommandBufferAccessContext *GetAccessContextImpl(VkCommandBuffer command_buffer, bool do_insert) {
        auto found_it = cb_access_state.find(command_buffer);
        if (found_it == cb_access_state.end()) {
            if (!do_insert) return nullptr;
            // If we don't have one, make it.
            auto cb_state = GetShared<CMD_BUFFER_STATE>(command_buffer);
            assert(cb_state.get());
            auto queue_flags = GetQueueFlags(*cb_state);
            std::unique_ptr<CommandBufferAccessContext> context(new CommandBufferAccessContext(*this, cb_state, queue_flags));
            auto insert_pair = cb_access_state.insert(std::make_pair(command_buffer, std::move(context)));
            found_it = insert_pair.first;
        }
        return found_it->second.get();
    }
    CommandBufferAccessContext *GetAccessContext(VkCommandBuffer command_buffer) {
        return GetAccessContextImpl(command_buffer, true);  // true -> do_insert on not found
    }
    CommandBufferAccessContext *GetAccessContextNoInsert(VkCommandBuffer command_buffer) {
        return GetAccessContextImpl(command_buffer, false);  // false -> don't do_insert on not found
    }

    const CommandBufferAccessContext *GetAccessContext(VkCommandBuffer command_buffer) const {
        const auto found_it = cb_access_state.find(command_buffer);
        if (found_it == cb_access_state.end()) {
            return nullptr;
        }
        return found_it->second.get();
    }

    void ApplyGlobalBarriers(AccessContext *context, VkPipelineStageFlags srcStageMask, VkPipelineStageFlags dstStageMask,
                             SyncStageAccessFlags src_stage_scope, SyncStageAccessFlags dst_stage_scope,
                             uint32_t memoryBarrierCount, const VkMemoryBarrier *pMemoryBarriers, const ResourceUsageTag &tag);
    void ApplyBufferBarriers(AccessContext *context, VkPipelineStageFlags src_stage_mask, SyncStageAccessFlags src_stage_scope,
                             VkPipelineStageFlags dst_stage_mask, SyncStageAccessFlags dst_stage_scope, uint32_t barrier_count,
                             const VkBufferMemoryBarrier *barriers);
    void ApplyImageBarriers(AccessContext *context, VkPipelineStageFlags src_stage_mask, SyncStageAccessFlags src_stage_scope,
                            VkPipelineStageFlags dst_stage_mask, SyncStageAccessFlags dst_stage_scope, uint32_t barrier_count,
                            const VkImageMemoryBarrier *barriers, const ResourceUsageTag &tag);

    void ResetCommandBufferCallback(VkCommandBuffer command_buffer);
    void FreeCommandBufferCallback(VkCommandBuffer command_buffer);
    void RecordCmdBeginRenderPass(VkCommandBuffer commandBuffer, const VkRenderPassBeginInfo *pRenderPassBegin,
                                  const VkSubpassBeginInfo *pSubpassBeginInfo, CMD_TYPE command);
    void RecordCmdNextSubpass(VkCommandBuffer commandBuffer,

                              const VkSubpassBeginInfo *pSubpassBeginInfo, const VkSubpassEndInfo *pSubpassEndInfo,
                              CMD_TYPE command);
    void RecordCmdEndRenderPass(VkCommandBuffer commandBuffer, const VkSubpassEndInfo *pSubpassEndInfo, CMD_TYPE command);
    bool SupressedBoundDescriptorWAW(const HazardResult &hazard) const;

    void PostCallRecordCreateDevice(VkPhysicalDevice gpu, const VkDeviceCreateInfo *pCreateInfo,
                                    const VkAllocationCallbacks *pAllocator, VkDevice *pDevice, VkResult result) override;

    bool ValidateBeginRenderPass(VkCommandBuffer commandBuffer, const VkRenderPassBeginInfo *pRenderPassBegin,
                                 const VkSubpassBeginInfoKHR *pSubpassBeginInfo, const char *func_name) const;

    bool PreCallValidateCmdBeginRenderPass(VkCommandBuffer commandBuffer, const VkRenderPassBeginInfo *pRenderPassBegin,
                                           VkSubpassContents contents) const override;

    bool PreCallValidateCmdBeginRenderPass2KHR(VkCommandBuffer commandBuffer, const VkRenderPassBeginInfo *pRenderPassBegin,
                                               const VkSubpassBeginInfoKHR *pSubpassBeginInfo) const override;

    bool PreCallValidateCmdBeginRenderPass2(VkCommandBuffer commandBuffer, const VkRenderPassBeginInfo *pRenderPassBegin,
                                            const VkSubpassBeginInfoKHR *pSubpassBeginInfo) const override;

    bool PreCallValidateCmdCopyBuffer(VkCommandBuffer commandBuffer, VkBuffer srcBuffer, VkBuffer dstBuffer, uint32_t regionCount,
                                      const VkBufferCopy *pRegions) const override;

    void PreCallRecordCmdCopyBuffer(VkCommandBuffer commandBuffer, VkBuffer srcBuffer, VkBuffer dstBuffer, uint32_t regionCount,
                                    const VkBufferCopy *pRegions) override;

    bool PreCallValidateCmdCopyBuffer2KHR(VkCommandBuffer commandBuffer, const VkCopyBufferInfo2KHR *pCopyBufferInfos) const override;

    void PreCallRecordCmdCopyBuffer2KHR(VkCommandBuffer commandBuffer, const VkCopyBufferInfo2KHR *pCopyBufferInfos) override;

    bool PreCallValidateCmdCopyImage(VkCommandBuffer commandBuffer, VkImage srcImage, VkImageLayout srcImageLayout,
                                     VkImage dstImage, VkImageLayout dstImageLayout, uint32_t regionCount,
                                     const VkImageCopy *pRegions) const override;

    void PreCallRecordCmdCopyImage(VkCommandBuffer commandBuffer, VkImage srcImage, VkImageLayout srcImageLayout, VkImage dstImage,
                                   VkImageLayout dstImageLayout, uint32_t regionCount, const VkImageCopy *pRegions) override;

    bool PreCallValidateCmdCopyImage2KHR(VkCommandBuffer commandBuffer, const VkCopyImageInfo2KHR *pCopyImageInfo) const override;

    void PreCallRecordCmdCopyImage2KHR(VkCommandBuffer commandBuffer, const VkCopyImageInfo2KHR *pCopyImageInfo) override;

    bool PreCallValidateCmdPipelineBarrier(VkCommandBuffer commandBuffer, VkPipelineStageFlags srcStageMask,
                                           VkPipelineStageFlags dstStageMask, VkDependencyFlags dependencyFlags,
                                           uint32_t memoryBarrierCount, const VkMemoryBarrier *pMemoryBarriers,
                                           uint32_t bufferMemoryBarrierCount, const VkBufferMemoryBarrier *pBufferMemoryBarriers,
                                           uint32_t imageMemoryBarrierCount,
                                           const VkImageMemoryBarrier *pImageMemoryBarriers) const override;

    void PreCallRecordCmdPipelineBarrier(VkCommandBuffer commandBuffer, VkPipelineStageFlags srcStageMask,
                                         VkPipelineStageFlags dstStageMask, VkDependencyFlags dependencyFlags,
                                         uint32_t memoryBarrierCount, const VkMemoryBarrier *pMemoryBarriers,
                                         uint32_t bufferMemoryBarrierCount, const VkBufferMemoryBarrier *pBufferMemoryBarriers,
                                         uint32_t imageMemoryBarrierCount, const VkImageMemoryBarrier *pImageMemoryBarriers) override;

    void PostCallRecordBeginCommandBuffer(VkCommandBuffer commandBuffer, const VkCommandBufferBeginInfo *pBeginInfo,
                                          VkResult result) override;

    void PostCallRecordCmdBeginRenderPass(VkCommandBuffer commandBuffer, const VkRenderPassBeginInfo *pRenderPassBegin,
                                          VkSubpassContents contents) override;
    void PostCallRecordCmdBeginRenderPass2(VkCommandBuffer commandBuffer, const VkRenderPassBeginInfo *pRenderPassBegin,
                                           const VkSubpassBeginInfo *pSubpassBeginInfo) override;
    void PostCallRecordCmdBeginRenderPass2KHR(VkCommandBuffer commandBuffer, const VkRenderPassBeginInfo *pRenderPassBegin,
                                              const VkSubpassBeginInfo *pSubpassBeginInfo) override;

    bool ValidateCmdNextSubpass(VkCommandBuffer commandBuffer, const VkSubpassBeginInfoKHR *pSubpassBeginInfo,
                                const VkSubpassEndInfoKHR *pSubpassEndInfo, const char *func_name) const;
    bool PreCallValidateCmdNextSubpass(VkCommandBuffer commandBuffer, VkSubpassContents contents) const override;
    bool PreCallValidateCmdNextSubpass2(VkCommandBuffer commandBuffer, const VkSubpassBeginInfoKHR *pSubpassBeginInfo,
                                        const VkSubpassEndInfoKHR *pSubpassEndInfo) const override;
    bool PreCallValidateCmdNextSubpass2KHR(VkCommandBuffer commandBuffer, const VkSubpassBeginInfoKHR *pSubpassBeginInfo,
                                           const VkSubpassEndInfoKHR *pSubpassEndInfo) const override;

    void PostCallRecordCmdNextSubpass(VkCommandBuffer commandBuffer, VkSubpassContents contents) override;
    void PostCallRecordCmdNextSubpass2(VkCommandBuffer commandBuffer, const VkSubpassBeginInfo *pSubpassBeginInfo,
                                       const VkSubpassEndInfo *pSubpassEndInfo) override;
    void PostCallRecordCmdNextSubpass2KHR(VkCommandBuffer commandBuffer, const VkSubpassBeginInfo *pSubpassBeginInfo,
                                          const VkSubpassEndInfo *pSubpassEndInfo) override;

    bool ValidateCmdEndRenderPass(VkCommandBuffer commandBuffer, const VkSubpassEndInfoKHR *pSubpassEndInfo,
                                  const char *func_name) const;
    bool PreCallValidateCmdEndRenderPass(VkCommandBuffer commandBuffer) const override;
    bool PreCallValidateCmdEndRenderPass2KHR(VkCommandBuffer commandBuffer, const VkSubpassEndInfoKHR *pSubpassEndInfo) const override;
    bool PreCallValidateCmdEndRenderPass2(VkCommandBuffer commandBuffer, const VkSubpassEndInfoKHR *pSubpassEndInfo) const override;

    void PostCallRecordCmdEndRenderPass(VkCommandBuffer commandBuffer) override;
    void PostCallRecordCmdEndRenderPass2(VkCommandBuffer commandBuffer, const VkSubpassEndInfo *pSubpassEndInfo) override;
    void PostCallRecordCmdEndRenderPass2KHR(VkCommandBuffer commandBuffer, const VkSubpassEndInfo *pSubpassEndInfo) override;

    template <typename BufferImageCopyRegionType>
    bool ValidateCmdCopyBufferToImage(VkCommandBuffer commandBuffer, VkBuffer srcBuffer, VkImage dstImage,
                                      VkImageLayout dstImageLayout, uint32_t regionCount, const BufferImageCopyRegionType *pRegions,
                                      CopyCommandVersion version) const;
    bool PreCallValidateCmdCopyBufferToImage(VkCommandBuffer commandBuffer, VkBuffer srcBuffer, VkImage dstImage,
                                             VkImageLayout dstImageLayout, uint32_t regionCount,
                                             const VkBufferImageCopy *pRegions) const override;
    bool PreCallValidateCmdCopyBufferToImage2KHR(VkCommandBuffer commandBuffer,
                                                 const VkCopyBufferToImageInfo2KHR *pCopyBufferToImageInfo) const override;

    template <typename BufferImageCopyRegionType>
    void RecordCmdCopyBufferToImage(VkCommandBuffer commandBuffer, VkBuffer srcBuffer, VkImage dstImage,
                                    VkImageLayout dstImageLayout, uint32_t regionCount, const BufferImageCopyRegionType *pRegions,
                                    CopyCommandVersion version);
    void PreCallRecordCmdCopyBufferToImage(VkCommandBuffer commandBuffer, VkBuffer srcBuffer, VkImage dstImage,
                                           VkImageLayout dstImageLayout, uint32_t regionCount, const VkBufferImageCopy *pRegions) override;
    void PreCallRecordCmdCopyBufferToImage2KHR(VkCommandBuffer commandBuffer,
                                               const VkCopyBufferToImageInfo2KHR *pCopyBufferToImageInfo) override;

    template <typename BufferImageCopyRegionType>
    bool ValidateCmdCopyImageToBuffer(VkCommandBuffer commandBuffer, VkImage srcImage, VkImageLayout srcImageLayout,
                                      VkBuffer dstBuffer, uint32_t regionCount, const BufferImageCopyRegionType *pRegions,
                                      CopyCommandVersion version) const;
    bool PreCallValidateCmdCopyImageToBuffer(VkCommandBuffer commandBuffer, VkImage srcImage, VkImageLayout srcImageLayout,
                                             VkBuffer dstBuffer, uint32_t regionCount, const VkBufferImageCopy *pRegions) const override;
    bool PreCallValidateCmdCopyImageToBuffer2KHR(VkCommandBuffer commandBuffer,
                                                 const VkCopyImageToBufferInfo2KHR *pCopyImageToBufferInfo) const override;

    template <typename BufferImageCopyRegionType>
    void RecordCmdCopyImageToBuffer(VkCommandBuffer commandBuffer, VkImage srcImage, VkImageLayout srcImageLayout,
                                    VkBuffer dstBuffer, uint32_t regionCount, const BufferImageCopyRegionType *pRegions,
                                    CopyCommandVersion version);
    void PreCallRecordCmdCopyImageToBuffer(VkCommandBuffer commandBuffer, VkImage srcImage, VkImageLayout srcImageLayout,
                                           VkBuffer dstBuffer, uint32_t regionCount, const VkBufferImageCopy *pRegions) override;
    void PreCallRecordCmdCopyImageToBuffer2KHR(VkCommandBuffer commandBuffer,
                                               const VkCopyImageToBufferInfo2KHR *pCopyImageToBufferInfo) override;

    template <typename RegionType>
    bool ValidateCmdBlitImage(VkCommandBuffer commandBuffer, VkImage srcImage, VkImageLayout srcImageLayout, VkImage dstImage,
                              VkImageLayout dstImageLayout, uint32_t regionCount, const RegionType *pRegions, VkFilter filter,
                              const char *apiName) const;

    bool PreCallValidateCmdBlitImage(VkCommandBuffer commandBuffer, VkImage srcImage, VkImageLayout srcImageLayout,
                                     VkImage dstImage, VkImageLayout dstImageLayout, uint32_t regionCount,
                                     const VkImageBlit *pRegions, VkFilter filter) const override;
    bool PreCallValidateCmdBlitImage2KHR(VkCommandBuffer commandBuffer, const VkBlitImageInfo2KHR *pBlitImageInfo) const override;

    template <typename RegionType>
    void RecordCmdBlitImage(VkCommandBuffer commandBuffer, VkImage srcImage, VkImageLayout srcImageLayout, VkImage dstImage,
                            VkImageLayout dstImageLayout, uint32_t regionCount, const RegionType *pRegions, VkFilter filter,
                            ResourceUsageTag tag);
    void PreCallRecordCmdBlitImage(VkCommandBuffer commandBuffer, VkImage srcImage, VkImageLayout srcImageLayout, VkImage dstImage,
                                   VkImageLayout dstImageLayout, uint32_t regionCount, const VkImageBlit *pRegions,
                                   VkFilter filter) override;
    void PreCallRecordCmdBlitImage2KHR(VkCommandBuffer commandBuffer, const VkBlitImageInfo2KHR *pBlitImageInfo) override;

    bool ValidateIndirectBuffer(const AccessContext &context, VkCommandBuffer commandBuffer, const VkDeviceSize struct_size,
                                const VkBuffer buffer, const VkDeviceSize offset, const uint32_t drawCount, const uint32_t stride,
                                const char *function) const;
    void RecordIndirectBuffer(AccessContext &context, const ResourceUsageTag &tag, const VkDeviceSize struct_size,
                              const VkBuffer buffer, const VkDeviceSize offset, const uint32_t drawCount, uint32_t stride);

    bool ValidateCountBuffer(const AccessContext &context, VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                             const char *function) const;
    void RecordCountBuffer(AccessContext &context, const ResourceUsageTag &tag, VkBuffer buffer, VkDeviceSize offset);

    bool PreCallValidateCmdDispatch(VkCommandBuffer commandBuffer, uint32_t x, uint32_t y, uint32_t z) const override;
    void PreCallRecordCmdDispatch(VkCommandBuffer commandBuffer, uint32_t x, uint32_t y, uint32_t z) override;

    bool PreCallValidateCmdDispatchIndirect(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset) const override;
    void PreCallRecordCmdDispatchIndirect(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset) override;

    bool PreCallValidateCmdDraw(VkCommandBuffer commandBuffer, uint32_t vertexCount, uint32_t instanceCount, uint32_t firstVertex,
                                uint32_t firstInstance) const override;
    void PreCallRecordCmdDraw(VkCommandBuffer commandBuffer, uint32_t vertexCount, uint32_t instanceCount, uint32_t firstVertex,
                              uint32_t firstInstance) override;

    bool PreCallValidateCmdDrawIndexed(VkCommandBuffer commandBuffer, uint32_t indexCount, uint32_t instanceCount,
                                       uint32_t firstIndex, int32_t vertexOffset, uint32_t firstInstance) const override;
    void PreCallRecordCmdDrawIndexed(VkCommandBuffer commandBuffer, uint32_t indexCount, uint32_t instanceCount,
                                     uint32_t firstIndex, int32_t vertexOffset, uint32_t firstInstance) override;

    bool PreCallValidateCmdDrawIndirect(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset, uint32_t drawCount,
                                        uint32_t stride) const override;
    void PreCallRecordCmdDrawIndirect(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset, uint32_t drawCount,
                                      uint32_t stride) override;

    bool PreCallValidateCmdDrawIndexedIndirect(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                               uint32_t drawCount, uint32_t stride) const override;
    void PreCallRecordCmdDrawIndexedIndirect(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                             uint32_t drawCount, uint32_t stride) override;

    bool ValidateCmdDrawIndirectCount(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset, VkBuffer countBuffer,
                                      VkDeviceSize countBufferOffset, uint32_t maxDrawCount, uint32_t stride,
                                      const char *function) const;
    bool PreCallValidateCmdDrawIndirectCount(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                             VkBuffer countBuffer, VkDeviceSize countBufferOffset, uint32_t maxDrawCount,
                                             uint32_t stride) const override;
    void PreCallRecordCmdDrawIndirectCount(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                           VkBuffer countBuffer, VkDeviceSize countBufferOffset, uint32_t maxDrawCount,
                                           uint32_t stride) override;
    bool PreCallValidateCmdDrawIndirectCountKHR(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                VkBuffer countBuffer, VkDeviceSize countBufferOffset, uint32_t maxDrawCount,
                                                uint32_t stride) const override;
    void PreCallRecordCmdDrawIndirectCountKHR(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                              VkBuffer countBuffer, VkDeviceSize countBufferOffset, uint32_t maxDrawCount,
                                              uint32_t stride) override;
    bool PreCallValidateCmdDrawIndirectCountAMD(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                VkBuffer countBuffer, VkDeviceSize countBufferOffset, uint32_t maxDrawCount,
                                                uint32_t stride) const override;
    void PreCallRecordCmdDrawIndirectCountAMD(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                              VkBuffer countBuffer, VkDeviceSize countBufferOffset, uint32_t maxDrawCount,
                                              uint32_t stride) override;

    bool ValidateCmdDrawIndexedIndirectCount(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                             VkBuffer countBuffer, VkDeviceSize countBufferOffset, uint32_t maxDrawCount,
                                             uint32_t stride, const char *function) const;
    bool PreCallValidateCmdDrawIndexedIndirectCount(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                    VkBuffer countBuffer, VkDeviceSize countBufferOffset, uint32_t maxDrawCount,
                                                    uint32_t stride) const override;
    void PreCallRecordCmdDrawIndexedIndirectCount(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                  VkBuffer countBuffer, VkDeviceSize countBufferOffset, uint32_t maxDrawCount,
                                                  uint32_t stride) override;
    bool PreCallValidateCmdDrawIndexedIndirectCountKHR(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                       VkBuffer countBuffer, VkDeviceSize countBufferOffset, uint32_t maxDrawCount,
                                                       uint32_t stride) const override;
    void PreCallRecordCmdDrawIndexedIndirectCountKHR(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                     VkBuffer countBuffer, VkDeviceSize countBufferOffset, uint32_t maxDrawCount,
                                                     uint32_t stride) override;
    bool PreCallValidateCmdDrawIndexedIndirectCountAMD(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                       VkBuffer countBuffer, VkDeviceSize countBufferOffset, uint32_t maxDrawCount,
                                                       uint32_t stride) const override;
    void PreCallRecordCmdDrawIndexedIndirectCountAMD(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                     VkBuffer countBuffer, VkDeviceSize countBufferOffset, uint32_t maxDrawCount,
                                                     uint32_t stride) override;

    bool PreCallValidateCmdClearColorImage(VkCommandBuffer commandBuffer, VkImage image, VkImageLayout imageLayout,
                                           const VkClearColorValue *pColor, uint32_t rangeCount,
                                           const VkImageSubresourceRange *pRanges) const override;
    void PreCallRecordCmdClearColorImage(VkCommandBuffer commandBuffer, VkImage image, VkImageLayout imageLayout,
                                         const VkClearColorValue *pColor, uint32_t rangeCount,
                                         const VkImageSubresourceRange *pRanges) override;

    bool PreCallValidateCmdClearDepthStencilImage(VkCommandBuffer commandBuffer, VkImage image, VkImageLayout imageLayout,
                                                  const VkClearDepthStencilValue *pDepthStencil, uint32_t rangeCount,
                                                  const VkImageSubresourceRange *pRanges) const override;
    void PreCallRecordCmdClearDepthStencilImage(VkCommandBuffer commandBuffer, VkImage image, VkImageLayout imageLayout,
                                                const VkClearDepthStencilValue *pDepthStencil, uint32_t rangeCount,
                                                const VkImageSubresourceRange *pRanges) override;

    bool PreCallValidateCmdCopyQueryPoolResults(VkCommandBuffer commandBuffer, VkQueryPool queryPool, uint32_t firstQuery,
                                                uint32_t queryCount, VkBuffer dstBuffer, VkDeviceSize dstOffset,
                                                VkDeviceSize stride, VkQueryResultFlags flags) const override;
    void PreCallRecordCmdCopyQueryPoolResults(VkCommandBuffer commandBuffer, VkQueryPool queryPool, uint32_t firstQuery,
                                              uint32_t queryCount, VkBuffer dstBuffer, VkDeviceSize dstOffset, VkDeviceSize stride,
                                              VkQueryResultFlags flags) override;

    bool PreCallValidateCmdFillBuffer(VkCommandBuffer commandBuffer, VkBuffer dstBuffer, VkDeviceSize dstOffset, VkDeviceSize size,
                                      uint32_t data) const override;
    void PreCallRecordCmdFillBuffer(VkCommandBuffer commandBuffer, VkBuffer dstBuffer, VkDeviceSize dstOffset, VkDeviceSize size,
                                    uint32_t data) override;

    bool PreCallValidateCmdResolveImage(VkCommandBuffer commandBuffer, VkImage srcImage, VkImageLayout srcImageLayout,
                                        VkImage dstImage, VkImageLayout dstImageLayout, uint32_t regionCount,
                                        const VkImageResolve *pRegions) const override;

    void PreCallRecordCmdResolveImage(VkCommandBuffer commandBuffer, VkImage srcImage, VkImageLayout srcImageLayout,
                                      VkImage dstImage, VkImageLayout dstImageLayout, uint32_t regionCount,
                                      const VkImageResolve *pRegions) override;

    bool PreCallValidateCmdResolveImage2KHR(VkCommandBuffer commandBuffer, const VkResolveImageInfo2KHR *pResolveImageInfo) const override;
    void PreCallRecordCmdResolveImage2KHR(VkCommandBuffer commandBuffer, const VkResolveImageInfo2KHR *pResolveImageInfo) override;

    bool PreCallValidateCmdUpdateBuffer(VkCommandBuffer commandBuffer, VkBuffer dstBuffer, VkDeviceSize dstOffset,
                                        VkDeviceSize dataSize, const void *pData) const override;
    void PreCallRecordCmdUpdateBuffer(VkCommandBuffer commandBuffer, VkBuffer dstBuffer, VkDeviceSize dstOffset,
                                      VkDeviceSize dataSize, const void *pData) override;

    bool PreCallValidateCmdWriteBufferMarkerAMD(VkCommandBuffer commandBuffer, VkPipelineStageFlagBits pipelineStage,
                                                VkBuffer dstBuffer, VkDeviceSize dstOffset, uint32_t marker) const override;
    void PreCallRecordCmdWriteBufferMarkerAMD(VkCommandBuffer commandBuffer, VkPipelineStageFlagBits pipelineStage,
                                              VkBuffer dstBuffer, VkDeviceSize dstOffset, uint32_t marker) override;
};
