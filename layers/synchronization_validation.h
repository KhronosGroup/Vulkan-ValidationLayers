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

#include <map>
#include <memory>
#include <unordered_map>
#include <vulkan/vulkan.h>

#include "synchronization_validation_types.h"
#include "state_tracker.h"

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
    static SyncStageAccessFlagBits FlagBit(SyncStageAccessIndex stage_access) {
        return syncStageAccessInfoByStageAccessIndex[stage_access].stage_access_bit;
    }

    static bool IsRead(SyncStageAccessFlagBits stage_access_bit) { return 0 != (stage_access_bit & syncStageAccessReadMask); }
    static bool IsRead(SyncStageAccessIndex stage_access_index) { return IsRead(FlagBit(stage_access_index)); }

    static bool IsWrite(SyncStageAccessFlagBits stage_access_bit) { return 0 != (stage_access_bit & syncStageAccessWriteMask); }
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
    ResourceUsageTag &operator++() {
        index++;
        return *this;
    }
    bool IsBefore(const ResourceUsageTag &rhs) const { return index < rhs.index; }
    bool operator==(const ResourceUsageTag &rhs) const { return (index == rhs.index); }
    bool operator!=(const ResourceUsageTag &rhs) const { return !(*this == rhs); }
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
using SyncBarrierStack = std::vector<const SyncBarrier *>;

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

    static ResourceAccessState ApplyBarrierStack(const ResourceAccessState &that, const SyncBarrierStack &barrier_stack);

  public:
    HazardResult DetectHazard(SyncStageAccessIndex usage_index, SyncBarrierStack *barrier_stack) const;
    HazardResult DetectHazard(SyncStageAccessIndex usage_index) const;

    HazardResult DetectBarrierHazard(SyncStageAccessIndex usage_index, VkPipelineStageFlags src_exec_scope,
                                     SyncStageAccessFlags src_access_scope, SyncBarrierStack *barrier_stack) const;
    HazardResult DetectBarrierHazard(SyncStageAccessIndex usage_index, VkPipelineStageFlags src_stage_mask,
                                     SyncStageAccessFlags source_scope) const;

    HazardResult DetectAsyncHazard(SyncStageAccessIndex usage_index) const;

    void Update(SyncStageAccessIndex usage_index, const ResourceUsageTag &tag);
    void Resolve(const ResourceAccessState &other);
    void ApplyBarrier(const SyncBarrier &barrier);
    void ApplyExecutionBarrier(VkPipelineStageFlags srcStageMask, VkPipelineStageFlags dstStageMask);
    void ApplyMemoryAccessBarrier(VkPipelineStageFlags src_stage_mask, SyncStageAccessFlags src_scope,
                                  VkPipelineStageFlags dst_stage_mask, SyncStageAccessFlags dst_scope);

    ResourceAccessState()
        : write_barriers(~SyncStageAccessFlags(0)), write_dependency_chain(0), last_read_count(0), last_read_stages(0) {}

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

    std::array<ReadState, 8 * sizeof(VkPipelineStageFlags)> last_reads;
    SyncStageAccessFlagBits last_write;  // only the most recent write
};

using ResourceAccessRangeMap = sparse_container::range_map<uint64_t, ResourceAccessState>;
using ResourceAccessRange = typename ResourceAccessRangeMap::key_type;

class AccessTrackerContext;
// This class owns none of the objects pointed to.
class AccessTracker {
  public:
    AccessTracker(AccessTrackerContext *context) : accesses_() {}
    ResourceAccessRangeMap &GetCurrentAccessMap() { return accesses_; }
    const ResourceAccessRangeMap &GetCurrentAccessMap() const { return accesses_; }
    void UpdateAccessState(SyncStageAccessIndex current_usage, const ResourceAccessRange &range, const ResourceUsageTag &tag);
    void UpdateAccessState(const IMAGE_STATE &image, SyncStageAccessIndex current_usage,
                           const VkImageSubresourceLayers &subresource, const VkOffset3D &offset, const VkExtent3D &extent,
                           const ResourceUsageTag &tag);

  private:
    ResourceAccessRangeMap accesses_;
// TODO: Cache the track back tree to save on repeated map lookups
#if 0
    struct TrackBack {
        const VkSubpassDependency2 *barrier;
        AccessTracker *tracker;
    };
    std::vector<TrackBack> prev_;
    std::vector<AccessTracker *> async_;
    TrackBack external_;
    AccessTrackerContext *context_;
#endif
};

class AccessTrackerContext {
  protected:
    // TODO -- hide the details of the implementation..
    template <typename Map, typename Key>
    static typename Map::mapped_type *GetImpl(Map *map, Key key, AccessTrackerContext *context) {
        auto find_it = map->find(key);
        if (find_it == map->end()) {
            if (!context) return nullptr;
            auto insert_pair = map->insert(std::make_pair(key, typename Map::mapped_type(context)));
            find_it = insert_pair.first;
        }
        return &find_it->second;
    }

    template <typename Map, typename Key>
    static const typename Map::mapped_type *GetConstImpl(const Map *map, Key key) {
        auto find_it = map->find(key);
        if (find_it == map->cend()) {
            return nullptr;
        }
        return &find_it->second;
    }

  public:
    using AccessTrackerMap = std::unordered_map<VulkanTypedHandle, AccessTracker>;
    struct TrackBack {
        SyncBarrier barrier;
        AccessTrackerContext *context;
        TrackBack(AccessTrackerContext *context_, VkQueueFlags queue_flags_, const VkSubpassDependency2 &subpass_barrier_)
            : barrier(queue_flags_, subpass_barrier_), context(context_) {}
        TrackBack &operator=(const TrackBack &) = default;
        TrackBack() = default;
    };

    AccessTracker *GetAccessTracker(const VulkanTypedHandle &handle) { return GetImpl(&access_tracker_map_, handle, this); }
    AccessTracker *GetAccessTrackerNoInsert(const VulkanTypedHandle &handle) {
        return GetImpl(&access_tracker_map_, handle, nullptr);
    }
    const AccessTracker *GetAccessTracker(const VulkanTypedHandle &handle) const {
        return GetConstImpl(&access_tracker_map_, handle);
    }
    HazardResult DetectHazard(const VulkanTypedHandle &handle, SyncStageAccessIndex usage_index,
                              const ResourceAccessRange &range) const;
    HazardResult DetectBarrierHazard(const VulkanTypedHandle &handle, SyncStageAccessIndex current_usage,
                                     VkPipelineStageFlags src_exec_scope, SyncStageAccessFlags src_access_scope,
                                     const ResourceAccessRange &range) const;
    HazardResult DetectHazard(const IMAGE_STATE &image, SyncStageAccessIndex current_usage,
                              const VkImageSubresourceLayers &subresource, const VkOffset3D &offset,
                              const VkExtent3D &extent) const;

    const AccessTrackerMap &GetAccessTrackerMap() const { return access_tracker_map_; }
    AccessTrackerMap &GetAccessTrackerMap() { return access_tracker_map_; }

    const TrackBack &GetDstExternalTrackBack() const { return dst_external_; }
    void Reset() {
        access_tracker_map_.clear();
        prev_.clear();
        async_.clear();
        src_external_ = TrackBack();
    }
    // TODO: See if returning the lower_bound would be useful from a performance POV -- look at the lower_bound overhead
    // Would need to add a "hint" overload to parallel_iterator::invalidate_[AB] call, if so.
    void ResolvePreviousAccess(const VulkanTypedHandle &handle, const ResourceAccessRange &range,
                               ResourceAccessRangeMap *descent_map, const ResourceAccessState *infill_state) const;
    void ResolveTrackBack(const VulkanTypedHandle &handle, const ResourceAccessRange &range,
                          const AccessTrackerContext::TrackBack &track_back, ResourceAccessRangeMap *descent_map,
                          const ResourceAccessState *infill_state, bool recur_to_infill = true) const;
    void UpdateAccessState(const VulkanTypedHandle &handle, SyncStageAccessIndex current_usage, const ResourceAccessRange &range,
                           const ResourceUsageTag &tag);
    void UpdateAccessState(const IMAGE_STATE &image, SyncStageAccessIndex current_usage,
                           const VkImageSubresourceLayers &subresource, const VkOffset3D &offset, const VkExtent3D &extent,
                           const ResourceUsageTag &tag);

    AccessTrackerContext(uint32_t subpass, VkQueueFlags queue_flags, const std::vector<SubpassDependencyGraphNode> &dependencies,
                         const std::vector<AccessTrackerContext> &contexts, AccessTrackerContext *external_context);

    AccessTrackerContext() { Reset(); }

  private:
    template <typename Detector>
    HazardResult DetectHazard(const VulkanTypedHandle &handle, const Detector &detector, const ResourceAccessRange &range) const;
    template <typename Detector>
    HazardResult DetectAsyncHazard(const VulkanTypedHandle &handle, const Detector &detector,
                                   const ResourceAccessRange &range) const;
    template <typename Detector>
    HazardResult DetectPreviousHazard(const VulkanTypedHandle &handle, const Detector &detector,
                                      const ResourceAccessRange &range) const;

    AccessTrackerMap access_tracker_map_;

    std::vector<TrackBack> prev_;
    std::vector<AccessTrackerContext *> async_;
    TrackBack src_external_;
    TrackBack dst_external_;
};

struct RenderPassAccessContext {
    uint32_t current_subpass_;
    std::vector<AccessTrackerContext> subpass_contexts_;
    const std::vector<SubpassDependencyGraphNode> *dependencies_;
    RenderPassAccessContext(VkQueueFlags queue_flags, const std::vector<SubpassDependencyGraphNode> *dependencies,
                            AccessTrackerContext *external_context)
        : current_subpass_(0), dependencies_(dependencies) {
        if (dependencies_) {
            subpass_contexts_.emplace_back(0, queue_flags, *dependencies_, subpass_contexts_, external_context);
        }
    }
    void NextSubpass(VkQueueFlags queue_flags, AccessTrackerContext *external_context) {
        current_subpass_++;
        subpass_contexts_.emplace_back(current_subpass_, queue_flags, *dependencies_, subpass_contexts_, external_context);
        assert(subpass_contexts_.size() == (current_subpass_ + 1));
    }
    AccessTrackerContext &CurrentContext() { return subpass_contexts_[current_subpass_]; }
    const AccessTrackerContext &CurrentContext() const { return subpass_contexts_[current_subpass_]; }
};

class CommandBufferAccessContext {
  public:
    CommandBufferAccessContext()
        : render_pass_contexts_(),
          cb_tracker_context_(),
          current_context_(&cb_tracker_context_),
          current_renderpass_context_(),
          cb_state_(),
          queue_flags_() {}
    CommandBufferAccessContext(std::shared_ptr<CMD_BUFFER_STATE> &cb_state, VkQueueFlags queue_flags)
        : CommandBufferAccessContext() {
        cb_state_ = cb_state;
        queue_flags_ = queue_flags;
    }

    void Reset() {
        cb_tracker_context_.Reset();
        render_pass_contexts_.clear();
        current_context_ = &cb_tracker_context_;
        current_renderpass_context_ = nullptr;
    }

    AccessTrackerContext *GetCurrentAccessContext() { return current_context_; }
    const AccessTrackerContext *GetCurrentAccessContext() const { return current_context_; }
    void BeginRenderPass(const RENDER_PASS_STATE &render_pass);
    void NextRenderPass(const RENDER_PASS_STATE &render_pass);
    void EndRenderPass(const RENDER_PASS_STATE &render_pass);
    CMD_BUFFER_STATE *GetCommandBufferState() { return cb_state_.get(); }
    const CMD_BUFFER_STATE *GetCommandBufferState() const { return cb_state_.get(); }
    VkQueueFlags GetQueueFlags() const { return queue_flags_; }

  private:
    std::vector<RenderPassAccessContext> render_pass_contexts_;
    AccessTrackerContext cb_tracker_context_;
    AccessTrackerContext *current_context_;
    RenderPassAccessContext *current_renderpass_context_;
    std::shared_ptr<CMD_BUFFER_STATE> cb_state_;
    VkQueueFlags queue_flags_;
};

class SyncValidator : public ValidationStateTracker, public SyncStageAccess {
  public:
    SyncValidator() { container_type = LayerObjectTypeSyncValidation; }
    using StateTracker = ValidationStateTracker;

    using StateTracker::AccessorTraitsTypes;
    ResourceUsageTag tag;  // Find a better tagging scheme...
    std::unordered_map<VkCommandBuffer, std::unique_ptr<CommandBufferAccessContext>> cb_access_state;
    CommandBufferAccessContext *GetAccessContextImpl(VkCommandBuffer command_buffer, bool do_insert) {
        auto found_it = cb_access_state.find(command_buffer);
        if (found_it == cb_access_state.end()) {
            if (!do_insert) return nullptr;
            // If we don't have one, make it.
            auto cb_state = GetShared<CMD_BUFFER_STATE>(command_buffer);
            assert(cb_state.get());
            auto queue_flags = GetQueueFlags(*cb_state);
            std::unique_ptr<CommandBufferAccessContext> context(new CommandBufferAccessContext(cb_state, queue_flags));
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

    void ApplyGlobalBarriers(AccessTrackerContext *context, VkPipelineStageFlags srcStageMask, VkPipelineStageFlags dstStageMask,
                             SyncStageAccessFlags src_stage_scope, SyncStageAccessFlags dst_stage_scope,
                             uint32_t memoryBarrierCount, const VkMemoryBarrier *pMemoryBarriers);
    void ApplyBufferBarriers(AccessTrackerContext *context, VkPipelineStageFlags src_stage_mask,
                             SyncStageAccessFlags src_stage_scope, VkPipelineStageFlags dst_stage_mask,
                             SyncStageAccessFlags dst_stage_scope, uint32_t barrier_count, const VkBufferMemoryBarrier *barriers);
    void ApplyImageBarriers(AccessTrackerContext *context, VkPipelineStageFlags src_stage_mask,
                            SyncStageAccessFlags src_stage_scope, VkPipelineStageFlags dst_stage_mask,
                            SyncStageAccessFlags dst_stage_scope, uint32_t barrier_count, const VkImageMemoryBarrier *barriers);

    void ResetCommandBuffer(VkCommandBuffer command_buffer);
    void RecordCmdBeginRenderPass(VkCommandBuffer commandBuffer, const VkRenderPassBeginInfo *pRenderPassBegin,
                                  const VkSubpassBeginInfo *pSubpassBeginInfo);
    void RecordCmdNextSubpass(VkCommandBuffer commandBuffer, const VkSubpassBeginInfo *pSubpassBeginInfo,
                              const VkSubpassEndInfo *pSubpassEndInfo);
    void RecordCmdEndRenderPass(VkCommandBuffer commandBuffer, const VkSubpassEndInfo *pSubpassEndInfo);

    void PostCallRecordCreateDevice(VkPhysicalDevice gpu, const VkDeviceCreateInfo *pCreateInfo,
                                    const VkAllocationCallbacks *pAllocator, VkDevice *pDevice, VkResult result);

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

    void PostCallRecordCmdNextSubpass(VkCommandBuffer commandBuffer, VkSubpassContents contents);
    void PostCallRecordCmdNextSubpass2(VkCommandBuffer commandBuffer, const VkSubpassBeginInfo *pSubpassBeginInfo,
                                       const VkSubpassEndInfo *pSubpassEndInfo);
    void PostCallRecordCmdNextSubpass2KHR(VkCommandBuffer commandBuffer, const VkSubpassBeginInfo *pSubpassBeginInfo,
                                          const VkSubpassEndInfo *pSubpassEndInfo);

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
};
