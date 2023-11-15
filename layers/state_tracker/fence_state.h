/* Copyright (c) 2015-2023 The Khronos Group Inc.
 * Copyright (c) 2015-2023 Valve Corporation
 * Copyright (c) 2015-2023 LunarG, Inc.
 * Copyright (C) 2015-2023 Google Inc.
 * Modifications Copyright (C) 2020 Advanced Micro Devices, Inc. All rights reserved.
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

#include "state_tracker/base_node.h"
#include <future>
#include <mutex>

class ValidationStateTracker;

namespace vvl {

class Queue;

class Fence : public REFCOUNTED_NODE {
  public:
    enum State { kUnsignaled, kInflight, kRetired };
    enum Scope {
        kInternal,
        kExternalTemporary,
        kExternalPermanent,
    };
    // Default constructor
    Fence(ValidationStateTracker &dev, VkFence f, const VkFenceCreateInfo *pCreateInfo)
        : REFCOUNTED_NODE(f, kVulkanObjectTypeFence),
          flags(pCreateInfo->flags),
          exportHandleTypes(GetExportHandleTypes(pCreateInfo)),
          state_((pCreateInfo->flags & VK_FENCE_CREATE_SIGNALED_BIT) ? kRetired : kUnsignaled),
          completed_(),
          waiter_(completed_.get_future()),
          dev_data_(dev) {}

    VkFence VkHandle() const { return handle_.Cast<VkFence>(); }

    bool EnqueueSignal(Queue *queue_state, uint64_t next_seq);

    // Notify the queue that the fence has signalled and then wait for the queue
    // to update state.
    void NotifyAndWait(const Location &loc);

    // Update state of the completed fence. This should only be called by Queue.
    void Retire();

    void Reset();

    void Import(VkExternalFenceHandleTypeFlagBits handle_type, VkFenceImportFlags flags);

    void Export(VkExternalFenceHandleTypeFlagBits handle_type);

    const VkFenceCreateFlags flags;
    const VkExternalFenceHandleTypeFlags exportHandleTypes;

    enum Scope Scope() const { return scope_; }
    enum State State() const { return state_; }

  private:
    static VkExternalFenceHandleTypeFlags GetExportHandleTypes(const VkFenceCreateInfo *info) {
        auto export_info = vku::FindStructInPNextChain<VkExportFenceCreateInfo>(info->pNext);
        return export_info ? export_info->handleTypes : 0;
    }
    ReadLockGuard ReadLock() const { return ReadLockGuard(lock_); }
    WriteLockGuard WriteLock() { return WriteLockGuard(lock_); }

    Queue *queue_{nullptr};
    uint64_t seq_{0};
    enum State state_;
    enum Scope scope_{kInternal};
    mutable std::shared_mutex lock_;
    std::promise<void> completed_;
    std::shared_future<void> waiter_;
    ValidationStateTracker &dev_data_;
};

}  // namespace vvl
