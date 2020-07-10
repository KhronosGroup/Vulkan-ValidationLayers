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
    const static uint64_t kMaxIndex = std::numeric_limits<uint64_t>::max();
    ResourceUsageTag &operator++() {
        index++;
        return *this;
    }
    bool IsBefore(const ResourceUsageTag &rhs) const { return index < rhs.index; }
    bool operator==(const ResourceUsageTag &rhs) const { return (index == rhs.index); }
    bool operator!=(const ResourceUsageTag &rhs) const { return !(*this == rhs); }
    ResourceUsageTag() : index(0) {}
    ResourceUsageTag(uint64_t index_) : index(index_) {}
};

struct HazardResult {
    SyncHazard hazard = NONE;
    ResourceUsageTag tag = ResourceUsageTag();
    void Set(SyncHazard hazard_, const ResourceUsageTag &tag_) {
        hazard = hazard_;
        tag = tag_;
    }
};

struct SyncBarrier {
    VkPipelineStageFlags src_exec_scope;
    SyncStageAccessFlags src_access_scope;
    VkPipelineStageFlags dst_exec_scope;
    SyncStageAccessFlags dst_access_scope;
    SyncBarrier() = default;
    SyncBarrier &operator=(const SyncBarrier &) = default;
    SyncBarrier(VkQueueFlags gueue_flags, const VkSubpassDependency2 &sub_pass_barrier);
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
        VkPipelineStageFlags barriers;  // all applicable barriered stages
        ResourceUsageTag tag;
        bool operator==(const ReadState &rhs) const {
            bool same = (stage == rhs.stage) && (barriers == rhs.barriers) && (tag == rhs.tag);
            return same;
        }
        bool operator!=(const ReadState &rhs) const { return !(*this == rhs); }
    };

  public:
    HazardResult DetectHazard(SyncStageAccessIndex usage_index) const;
    HazardResult DetectHazard(SyncStageAccessIndex usage_index, const SyncOrderingBarrier &ordering) const;

    HazardResult DetectBarrierHazard(SyncStageAccessIndex usage_index, VkPipelineStageFlags source_exec_scope,
                                     SyncStageAccessFlags source_access_scope) const;
    HazardResult DetectAsyncHazard(SyncStageAccessIndex usage_index) const;

    void Update(SyncStageAccessIndex usage_index, const ResourceUsageTag &tag);
    void Resolve(const ResourceAccessState &other);
    void ApplyBarrier(const SyncBarrier &barrier);
    void ApplyExecutionBarrier(VkPipelineStageFlags srcStageMask, VkPipelineStageFlags dstStageMask);
    void ApplyMemoryAccessBarrier(VkPipelineStageFlags src_stage_mask, SyncStageAccessFlags src_scope,
                                  VkPipelineStageFlags dst_stage_mask, SyncStageAccessFlags dst_scope);

    ResourceAccessState()
        : write_barriers(~SyncStageAccessFlags(0)),
          write_dependency_chain(0),
          last_read_count(0),
          last_read_stages(0),
          write_tag(),
          last_write(0) {}

    bool HasWriteOp() const { return last_write != 0; }
    bool operator==(const ResourceAccessState &rhs) const {
        bool same = (write_barriers == rhs.write_barriers) && (write_dependency_chain == rhs.write_dependency_chain) &&
                    (last_read_count == rhs.last_read_count) && (last_read_stages == rhs.last_read_stages) &&
                    (write_tag == rhs.write_tag);
        for (uint32_t i = 0; same && i < last_read_count; i++) {
            same |= last_reads[i] == rhs.last_reads[i];
        }
        return same;
    }
    bool operator!=(const ResourceAccessState &rhs) const { return !(*this == rhs); }

  private:
    bool IsWriteHazard(SyncStageAccessFlagBits usage) const { return 0 != (usage & ~write_barriers); }
    bool IsReadHazard(VkPipelineStageFlagBits stage, const ReadState &read_access) const {
        return 0 != (stage & ~read_access.barriers);
    }
    bool IsReadHazard(VkPipelineStageFlags stage_mask, const ReadState &read_access) const {
        return stage_mask != (stage_mask & read_access.barriers);
    }
    // With reads, each must be "safe" relative to it's prior write, so we need only
    // save the most recent write operation (as anything *transitively* unsafe would arleady
    // be included
    SyncStageAccessFlags write_barriers;          // union of applicable barrier masks since last write
    VkPipelineStageFlags write_dependency_chain;  // intiially zero, but accumulating the dstStages of barriers if they chain.
    uint32_t last_read_count;
    VkPipelineStageFlags last_read_stages;

    ResourceUsageTag write_tag;
    // TODO: Add a NONE (zero) enum to SyncStageAccessFlagBits
    SyncStageAccessFlags last_write;  // only the most recent write

    std::array<ReadState, 8 * sizeof(VkPipelineStageFlags)> last_reads;
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

    struct TrackBack {
        SyncBarrier barrier;
        AccessContext *context;
        TrackBack(AccessContext *context_, VkQueueFlags queue_flags_, const VkSubpassDependency2 &subpass_barrier_)
            : barrier(queue_flags_, subpass_barrier_), context(context_) {}
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

    const TrackBack &GetDstExternalTrackBack() const { return dst_external_; }
    void Reset() {
        prev_.clear();
        prev_by_subpass_.clear();
        async_.clear();
        src_external_ = TrackBack();
        for (auto &map : access_state_maps_) {
            map.clear();
        }
    }
    // TODO: See if returning the lower_bound would be useful from a performance POV -- look at the lower_bound overhead
    // Would need to add a "hint" overload to parallel_iterator::invalidate_[AB] call, if so.
    void ResolvePreviousAccess(AddressType type, const ResourceAccessRange &range, ResourceAccessRangeMap *descent_map,
                               const ResourceAccessState *infill_state) const;
    void ResolvePreviousAccess(const IMAGE_STATE &image_state, const VkImageSubresourceRange &subresource_range,
                               AddressType address_type, ResourceAccessRangeMap *descent_map,
                               const ResourceAccessState *infill_state) const;
    void ResolveAccessRange(AddressType type, const ResourceAccessRange &range, const SyncBarrier *barrier,
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

    void ApplyImageBarrier(const IMAGE_STATE &image, VkPipelineStageFlags src_exec_scope, SyncStageAccessFlags src_access_scope,
                           VkPipelineStageFlags dst_exec_scope, SyncStageAccessFlags dst_accesse_scope,
                           const VkImageSubresourceRange &subresource_range);

    void ApplyImageBarrier(const IMAGE_STATE &image, VkPipelineStageFlags src_exec_scope, SyncStageAccessFlags src_access_scope,
                           VkPipelineStageFlags dst_exec_scope, SyncStageAccessFlags dst_access_scope,
                           const VkImageSubresourceRange &subresource_range, bool layout_transition, const ResourceUsageTag &tag);
    void ApplyImageBarrier(const IMAGE_STATE &image, const SyncBarrier &barrier, const VkImageSubresourceRange &subresource_range,
                           bool layout_transition, const ResourceUsageTag &tag);

    template <typename Action>
    void UpdateMemoryAccess(const BUFFER_STATE &buffer, const ResourceAccessRange &range, const Action action);
    template <typename Action>
    void UpdateMemoryAccess(const IMAGE_STATE &image, const VkImageSubresourceRange &subresource_range, const Action action);

    template <typename Action>
    void ApplyGlobalBarriers(const Action &barrier_action);

    static AddressType ImageAddressType(const IMAGE_STATE &image);
    static VkDeviceSize ResourceBaseAddress(const BINDABLE &bindable);

    AccessContext(uint32_t subpass, VkQueueFlags queue_flags, const std::vector<SubpassDependencyGraphNode> &dependencies,
                  const std::vector<AccessContext> &contexts, AccessContext *external_context);

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
    constexpr static int kAddressTypeCount = AddressType::kMaxAddressType + 1;
    static const std::array<AddressType, kAddressTypeCount> kAddressTypes;
    std::array<ResourceAccessRangeMap, kAddressTypeCount> access_state_maps_;
    std::vector<TrackBack> prev_;
    std::vector<TrackBack *> prev_by_subpass_;
    std::vector<AccessContext *> async_;
    TrackBack src_external_;
    TrackBack dst_external_;
};

class RenderPassAccessContext {
  public:
    RenderPassAccessContext(AccessContext *external_context)
        : external_context_(external_context), rp_state_(nullptr), current_subpass_(0) {}

    bool ValidateNextSubpass(const SyncValidator &sync_state, const VkRect2D &render_area, const char *command_name) const;
    bool ValidateEndRenderPass(const SyncValidator &sync_state, const VkRect2D &render_area, const char *func_name) const;
    bool ValidateFinalSubpassLayoutTransitions(const SyncValidator &sync_state, const VkRect2D &render_area,
                                               const char *func_name) const;

    void RecordLayoutTransitions(const ResourceUsageTag &tag);
    void RecordLoadOperations(const VkRect2D &render_area, const ResourceUsageTag &tag);
    void RecordBeginRenderPass(const SyncValidator &state, const CMD_BUFFER_STATE &cb_state, VkQueueFlags queue_flags,
                               const ResourceUsageTag &tag);
    void RecordNextSubpass(const VkRect2D &render_area, const ResourceUsageTag &tag);
    void RecordEndRenderPass(const VkRect2D &render_area, const ResourceUsageTag &tag);

    AccessContext &CurrentContext() { return subpass_contexts_[current_subpass_]; }
    const AccessContext &CurrentContext() const { return subpass_contexts_[current_subpass_]; }
    const std::vector<AccessContext> &GetContexts() const { return subpass_contexts_; }
    uint32_t GetCurrentSubpass() const { return current_subpass_; }
    const RENDER_PASS_STATE *GetRenderPassState() const { return rp_state_; }
    AccessContext *CreateStoreResolveProxy(const VkRect2D &render_area) const;

  private:
    AccessContext *external_context_;
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
        ResourceUsageTag next;
        next.index = (static_cast<uint64_t>(reset_count_) << 32) | command_number_;
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
                             uint32_t memoryBarrierCount, const VkMemoryBarrier *pMemoryBarriers);
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

    void PostCallRecordCreateDevice(VkPhysicalDevice gpu, const VkDeviceCreateInfo *pCreateInfo,
                                    const VkAllocationCallbacks *pAllocator, VkDevice *pDevice, VkResult result);

    bool ValidateBeginRenderPass(VkCommandBuffer commandBuffer, const VkRenderPassBeginInfo *pRenderPassBegin,
                                 const VkSubpassBeginInfoKHR *pSubpassBeginInfo, const char *func_name) const;

    bool PreCallValidateCmdBeginRenderPass(VkCommandBuffer commandBuffer, const VkRenderPassBeginInfo *pRenderPassBegin,
                                           VkSubpassContents contents) const;

    bool PreCallValidateCmdBeginRenderPass2KHR(VkCommandBuffer commandBuffer, const VkRenderPassBeginInfo *pRenderPassBegin,
                                               const VkSubpassBeginInfoKHR *pSubpassBeginInfo) const;

    bool PreCallValidateCmdBeginRenderPass2(VkCommandBuffer commandBuffer, const VkRenderPassBeginInfo *pRenderPassBegin,
                                            const VkSubpassBeginInfoKHR *pSubpassBeginInfo) const;

    bool PreCallValidateCmdCopyBuffer(VkCommandBuffer commandBuffer, VkBuffer srcBuffer, VkBuffer dstBuffer, uint32_t regionCount,
                                      const VkBufferCopy *pRegions) const;

    void PreCallRecordCmdCopyBuffer(VkCommandBuffer commandBuffer, VkBuffer srcBuffer, VkBuffer dstBuffer, uint32_t regionCount,
                                    const VkBufferCopy *pRegions);

    bool PreCallValidateCmdCopyImage(VkCommandBuffer commandBuffer, VkImage srcImage, VkImageLayout srcImageLayout,
                                     VkImage dstImage, VkImageLayout dstImageLayout, uint32_t regionCount,
                                     const VkImageCopy *pRegions) const;

    void PreCallRecordCmdCopyImage(VkCommandBuffer commandBuffer, VkImage srcImage, VkImageLayout srcImageLayout, VkImage dstImage,
                                   VkImageLayout dstImageLayout, uint32_t regionCount, const VkImageCopy *pRegions);

    bool PreCallValidateCmdPipelineBarrier(VkCommandBuffer commandBuffer, VkPipelineStageFlags srcStageMask,
                                           VkPipelineStageFlags dstStageMask, VkDependencyFlags dependencyFlags,
                                           uint32_t memoryBarrierCount, const VkMemoryBarrier *pMemoryBarriers,
                                           uint32_t bufferMemoryBarrierCount, const VkBufferMemoryBarrier *pBufferMemoryBarriers,
                                           uint32_t imageMemoryBarrierCount,
                                           const VkImageMemoryBarrier *pImageMemoryBarriers) const;

    void PreCallRecordCmdPipelineBarrier(VkCommandBuffer commandBuffer, VkPipelineStageFlags srcStageMask,
                                         VkPipelineStageFlags dstStageMask, VkDependencyFlags dependencyFlags,
                                         uint32_t memoryBarrierCount, const VkMemoryBarrier *pMemoryBarriers,
                                         uint32_t bufferMemoryBarrierCount, const VkBufferMemoryBarrier *pBufferMemoryBarriers,
                                         uint32_t imageMemoryBarrierCount, const VkImageMemoryBarrier *pImageMemoryBarriers);

    void PostCallRecordBeginCommandBuffer(VkCommandBuffer commandBuffer, const VkCommandBufferBeginInfo *pBeginInfo,
                                          VkResult result);

    void PostCallRecordCmdBeginRenderPass(VkCommandBuffer commandBuffer, const VkRenderPassBeginInfo *pRenderPassBegin,
                                          VkSubpassContents contents);
    void PostCallRecordCmdBeginRenderPass2(VkCommandBuffer commandBuffer, const VkRenderPassBeginInfo *pRenderPassBegin,
                                           const VkSubpassBeginInfo *pSubpassBeginInfo);
    void PostCallRecordCmdBeginRenderPass2KHR(VkCommandBuffer commandBuffer, const VkRenderPassBeginInfo *pRenderPassBegin,
                                              const VkSubpassBeginInfo *pSubpassBeginInfo);

    bool ValidateCmdNextSubpass(VkCommandBuffer commandBuffer, const VkSubpassBeginInfoKHR *pSubpassBeginInfo,
                                const VkSubpassEndInfoKHR *pSubpassEndInfo, const char *func_name) const;
    bool PreCallValidateCmdNextSubpass(VkCommandBuffer commandBuffer, VkSubpassContents contents) const;
    bool PreCallValidateCmdNextSubpass2(VkCommandBuffer commandBuffer, const VkSubpassBeginInfoKHR *pSubpassBeginInfo,
                                        const VkSubpassEndInfoKHR *pSubpassEndInfo) const;
    bool PreCallValidateCmdNextSubpass2KHR(VkCommandBuffer commandBuffer, const VkSubpassBeginInfoKHR *pSubpassBeginInfo,
                                           const VkSubpassEndInfoKHR *pSubpassEndInfo) const;

    void PostCallRecordCmdNextSubpass(VkCommandBuffer commandBuffer, VkSubpassContents contents);
    void PostCallRecordCmdNextSubpass2(VkCommandBuffer commandBuffer, const VkSubpassBeginInfo *pSubpassBeginInfo,
                                       const VkSubpassEndInfo *pSubpassEndInfo);
    void PostCallRecordCmdNextSubpass2KHR(VkCommandBuffer commandBuffer, const VkSubpassBeginInfo *pSubpassBeginInfo,
                                          const VkSubpassEndInfo *pSubpassEndInfo);

    bool ValidateCmdEndRenderPass(VkCommandBuffer commandBuffer, const VkSubpassEndInfoKHR *pSubpassEndInfo,
                                  const char *func_name) const;
    bool PreCallValidateCmdEndRenderPass(VkCommandBuffer commandBuffer) const;
    bool PreCallValidateCmdEndRenderPass2KHR(VkCommandBuffer commandBuffer, const VkSubpassEndInfoKHR *pSubpassEndInfo) const;
    bool PreCallValidateCmdEndRenderPass2(VkCommandBuffer commandBuffer, const VkSubpassEndInfoKHR *pSubpassEndInfo) const;

    void PostCallRecordCmdEndRenderPass(VkCommandBuffer commandBuffer);
    void PostCallRecordCmdEndRenderPass2(VkCommandBuffer commandBuffer, const VkSubpassEndInfo *pSubpassEndInfo);
    void PostCallRecordCmdEndRenderPass2KHR(VkCommandBuffer commandBuffer, const VkSubpassEndInfo *pSubpassEndInfo);
    bool PreCallValidateCmdCopyBufferToImage(VkCommandBuffer commandBuffer, VkBuffer srcBuffer, VkImage dstImage,
                                             VkImageLayout dstImageLayout, uint32_t regionCount,
                                             const VkBufferImageCopy *pRegions) const;

    void PreCallRecordCmdCopyBufferToImage(VkCommandBuffer commandBuffer, VkBuffer srcBuffer, VkImage dstImage,
                                           VkImageLayout dstImageLayout, uint32_t regionCount, const VkBufferImageCopy *pRegions);

    bool PreCallValidateCmdCopyImageToBuffer(VkCommandBuffer commandBuffer, VkImage srcImage, VkImageLayout srcImageLayout,
                                             VkBuffer dstBuffer, uint32_t regionCount, const VkBufferImageCopy *pRegions) const;

    void PreCallRecordCmdCopyImageToBuffer(VkCommandBuffer commandBuffer, VkImage srcImage, VkImageLayout srcImageLayout,
                                           VkBuffer dstBuffer, uint32_t regionCount, const VkBufferImageCopy *pRegions);

    bool PreCallValidateCmdBlitImage(VkCommandBuffer commandBuffer, VkImage srcImage, VkImageLayout srcImageLayout,
                                     VkImage dstImage, VkImageLayout dstImageLayout, uint32_t regionCount,
                                     const VkImageBlit *pRegions, VkFilter filter) const;

    void PreCallRecordCmdBlitImage(VkCommandBuffer commandBuffer, VkImage srcImage, VkImageLayout srcImageLayout, VkImage dstImage,
                                   VkImageLayout dstImageLayout, uint32_t regionCount, const VkImageBlit *pRegions,
                                   VkFilter filter);

    bool DetectDescriptorSetHazard(const CMD_BUFFER_STATE &cmd, VkPipelineBindPoint pipelineBindPoint, const char *function) const;
    void UpdateDescriptorSetAccessState(const CMD_BUFFER_STATE &cmd, CMD_TYPE command, VkPipelineBindPoint pipelineBindPoint);

    bool PreCallValidateCmdDispatch(VkCommandBuffer commandBuffer, uint32_t x, uint32_t y, uint32_t z) const;
    void PreCallRecordCmdDispatch(VkCommandBuffer commandBuffer, uint32_t x, uint32_t y, uint32_t z);
};
