/* Copyright (c) 2025 The Khronos Group Inc.
 * Copyright (c) 2025 Valve Corporation
 * Copyright (c) 2025 LunarG, Inc.
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

namespace syncval {

struct SyncExecScope {
    // The xxxStageMask parameter passed by the caller
    VkPipelineStageFlags2 mask_param = 0;

    // All earlier or later stages that would be affected by a barrier using this scope
    VkPipelineStageFlags2 exec_scope = 0;

    // All accesses that can be used with this scope
    SyncAccessFlags valid_accesses;

    static SyncExecScope MakeSrc(VkQueueFlags queue_flags, VkPipelineStageFlags2 src_stage_mask,
                                 VkPipelineStageFlags2 disabled_feature_mask = 0);
    static SyncExecScope MakeDst(VkQueueFlags queue_flags, VkPipelineStageFlags2 src_stage_mask);

    bool operator==(const SyncExecScope &other) const;
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
    SyncBarrier(const SyncExecScope &src_exec, const SyncExecScope &dst_exec);
    SyncBarrier(const SyncExecScope &src_exec, const SyncExecScope &dst_exec, const AllAccess &);
    SyncBarrier(const SyncExecScope &src_exec, VkAccessFlags2 src_access_mask, const SyncExecScope &dst_exec,
                VkAccessFlags2 dst_access_mask);
    SyncBarrier(VkQueueFlags queue_flags, const VkSubpassDependency2 &barrier);
    SyncBarrier(const std::vector<SyncBarrier> &barriers);

    bool operator==(const SyncBarrier &other) const;
    size_t Hash() const;
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

    BarrierScope(const SyncBarrier &barrier, QueueId scope_queue = kQueueIdInvalid, ResourceUsageTag scope_tag = kInvalidTag);
};

struct SemaphoreScope : SyncExecScope {
    SemaphoreScope(QueueId qid, const SyncExecScope &exec_scope) : SyncExecScope(exec_scope), queue(qid) {}
    SemaphoreScope() = default;
    QueueId queue;
};

}  // namespace syncval
