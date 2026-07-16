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

#pragma once

#include "sync/sync_common.h"

struct DeviceExtensions;

namespace syncval {

class SyncValidator;

struct SyncExecScope {
    // The xxxStageMask parameter passed by the caller
    VkPipelineStageFlags2 stage_mask = VK_PIPELINE_STAGE_2_NONE;

    // Stage mask with meta stages expanded, then extended with earlier/later stages.
    // Used for execution dependency checks
    VkPipelineStageFlags2 exec_scope = VK_PIPELINE_STAGE_2_NONE;

    // All accesses supported by stage_mask.
    // Used for pipeline barriers to intersect access masks
    SyncAccessFlags stage_mask_accesses;

    // All accesses supported by exec_scope.
    // Used for semaphore wait/signal access scopes
    SyncAccessFlags exec_scope_accesses;

    static SyncExecScope MakeSrc(VkQueueFlags queue_flags, VkPipelineStageFlags2 src_stage_mask,
                                 VkPipelineStageFlags2 disabled_feature_mask = 0);
    static SyncExecScope MakeDst(VkQueueFlags queue_flags, VkPipelineStageFlags2 src_stage_mask);

    bool operator==(const SyncExecScope& other) const;
    size_t Hash() const;
};

struct SyncBarrier {
    struct AllAccess {};
    SyncExecScope src_exec_scope;
    SyncAccessFlags src_access_scope;
    VkAccessFlags2 original_src_access = VK_ACCESS_2_NONE;

    SyncExecScope dst_exec_scope;
    SyncAccessFlags dst_access_scope;
    VkAccessFlags2 original_dst_access = VK_ACCESS_2_NONE;

    SyncBarrier() = default;
    SyncBarrier(const SyncExecScope& src_exec, const SyncExecScope& dst_exec);
    SyncBarrier(const SyncExecScope& src_exec, const SyncExecScope& dst_exec, const AllAccess&);
    SyncBarrier(const SyncExecScope& src_exec, VkAccessFlags2 src_access_mask, const SyncExecScope& dst_exec,
                VkAccessFlags2 dst_access_mask);
    SyncBarrier(VkQueueFlags queue_flags, const VkSubpassDependency2& barrier);
    SyncBarrier(const std::vector<SyncBarrier>& barriers);

    bool operator==(const SyncBarrier& other) const;
    size_t Hash() const;
};

struct SyncBufferBarrier {
    std::shared_ptr<const vvl::Buffer> buffer;
    SyncBarrier barrier;
    AccessRange range;

    SyncBufferBarrier(const std::shared_ptr<const vvl::Buffer>& buffer, const SyncBarrier& barrier, const AccessRange& range)
        : buffer(buffer), barrier(barrier), range(range) {}
};

struct SyncImageBarrier {
    std::shared_ptr<const vvl::Image> image;
    SyncBarrier barrier;
    VkImageSubresourceRange subresource_range;
    bool layout_transition;
    uint32_t barrier_index;
    uint32_t handle_index = vvl::kNoIndex32;

    SyncImageBarrier(const std::shared_ptr<const vvl::Image>& image, const SyncBarrier& barrier,
                     const VkImageSubresourceRange& subresource_range, bool layout_transition, uint32_t barrier_index)
        : image(image),
          barrier(barrier),
          subresource_range(subresource_range),
          layout_transition(layout_transition),
          barrier_index(barrier_index) {}
};

struct BarrierSet {
    SyncExecScope src_exec_scope;
    SyncExecScope dst_exec_scope;
    std::vector<SyncBarrier> memory_barriers;
    std::vector<SyncBufferBarrier> buffer_barriers;
    std::vector<SyncImageBarrier> image_barriers;

    bool single_exec_scope = false;

    // The numbers of additional global barriers introduced to track execution dependencies
    // defined by image and buffer barriers, or a single execution dependencies when a sync1
    // barrier command specifies no barriers (only exec scopes). Used for statistics tracking.
    uint32_t execution_dependency_barrier_count = 0;

    BarrierSet() = default;
    BarrierSet(const SyncValidator& sync_state, VkQueueFlags queue_flags, const VkDependencyInfo& dep_info);
    BarrierSet(const SyncValidator& sync_state, const SyncExecScope& src_exec_scope, const SyncExecScope& dst_exec_scope,
               uint32_t memory_barrier_count, const VkMemoryBarrier* memory_barriers, uint32_t buffer_barrier_count,
               const VkBufferMemoryBarrier* buffer_barriers, uint32_t image_barrier_count,
               const VkImageMemoryBarrier* image_barriers);

  private:
    void MakeMemoryBarriers(const SyncExecScope& src, const SyncExecScope& dst, uint32_t barrier_count,
                            const VkMemoryBarrier* barriers);
    void MakeMemoryBarriers(VkQueueFlags queue_flags, const VkDependencyInfo& dep_info);

    void MakeBufferMemoryBarriers(const SyncValidator& sync_state, const SyncExecScope& src, const SyncExecScope& dst,
                                  uint32_t barrier_count, const VkBufferMemoryBarrier* barriers);
    void MakeBufferMemoryBarriers(const SyncValidator& sync_state, VkQueueFlags queue_flags, uint32_t barrier_count,
                                  const VkBufferMemoryBarrier2* barriers);

    void MakeImageMemoryBarriers(const SyncValidator& sync_state, const SyncExecScope& src, const SyncExecScope& dst,
                                 uint32_t barrier_count, const VkImageMemoryBarrier* barriers, const DeviceExtensions& extensions);
    void MakeImageMemoryBarriers(const SyncValidator& sync_state, VkQueueFlags queue_flags, uint32_t barrier_count,
                                 const VkImageMemoryBarrier2* barriers, const DeviceExtensions& extensions);
};

// Defines the source scope of the barrier.
// ReadState and WriteState have InBarrierSourceScope() that checks if corresponding access is in the barrier source scope.
struct BarrierScope {
    VkPipelineStageFlagBits2 src_exec_scope;
    SyncAccessFlags src_access_scope;

    // Scope queue is used to include accesses only from the specific queue.
    // The check against queue scope is unified for all cases. During record time the scope queue
    // has default value (Invalid). This matches how the queue member of read/write accesses is
    // initialized during recording, so (access_queue == scope_queue) evaluates to true during record time.
    QueueId scope_queue = kQueueIdInvalid;

    // The tag is needed for the event scope logic. The scope tag is defined by the "set event" command.
    // The check against scope tag is unified for all cases. For non event code the scope tag is uint64-max
    // value, so (access_tag < scope_tag) evaluates to true for non event code.
    ResourceUsageTag scope_tag = kInvalidTag;

    BarrierScope(const SyncBarrier& barrier, QueueId scope_queue = kQueueIdInvalid, ResourceUsageTag scope_tag = kInvalidTag);
};

struct SemaphoreScope : SyncExecScope {
    SemaphoreScope(QueueId qid, const SyncExecScope& exec_scope) : SyncExecScope(exec_scope), queue(qid) {}
    SemaphoreScope() = default;
    QueueId queue;
};

}  // namespace syncval
