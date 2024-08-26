/* Copyright (c) 2015-2024 The Khronos Group Inc.
 * Copyright (c) 2015-2024 Valve Corporation
 * Copyright (c) 2015-2024 LunarG, Inc.
 * Copyright (C) 2015-2024 Google Inc.
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

#include "state_tracker/state_object.h"
#include "state_tracker/submission_reference.h"
#include <future>

class ValidationStateTracker;

namespace vvl {

class Queue;
class Swapchain;

// present-based sync is when a fence from AcquireNextImage is used to synchronize
// with submissions presented in one of the previous frames.
// More common scheme is to use QueueSubmit fence for frame synchronization.
struct PresentSync {
    // Queue submissions that will be notified when WaitForFences is called.
    small_vector<SubmissionReference, 2, uint32_t> submissions;

    // Swapchain associated with this PresentSync.
    std::shared_ptr<vvl::Swapchain> swapchain;
};

class Fence : public RefcountedStateObject {
  public:
    enum State { kUnsignaled, kInflight, kRetired };
    enum Scope {
        kInternal,
        kExternalTemporary,
        kExternalPermanent,
    };

    Fence(ValidationStateTracker &dev, VkFence handle, const VkFenceCreateInfo *pCreateInfo);

    VkFence VkHandle() const { return handle_.Cast<VkFence>(); }
    // TODO: apply ReadLock as Semaphore does, or consider reading enums without lock.
    // Consider if more high-level operation should be exposed, because
    // (State() == a && Scope() == b) can be racey, but this could be fine if the
    // goal is not to crash and validity is based on external synchronization.
    enum State State() const { return state_; }
    enum Scope Scope() const { return scope_; }

    bool EnqueueSignal(Queue *queue_state, uint64_t next_seq);

    // Notify the queue that the fence has signalled and then wait for the queue
    // to update state.
    void NotifyAndWait(const Location &loc);

    // Update state of the completed fence. This should only be called by Queue.
    void Retire();

    // vkResetFences
    void Reset();

    void Import(VkExternalFenceHandleTypeFlagBits handle_type, VkFenceImportFlags flags);
    void Export(VkExternalFenceHandleTypeFlagBits handle_type);
    std::optional<VkExternalFenceHandleTypeFlagBits> ImportedHandleType() const;

    void SetPresentSync(const PresentSync &present_sync);
    bool IsPresentSyncSwapchainChanged(const std::shared_ptr<vvl::Swapchain> &current_swapchain) const;

    const VkFenceCreateFlags flags;
    const VkExternalFenceHandleTypeFlags export_handle_types;

  private:
    ReadLockGuard ReadLock() const { return ReadLockGuard(lock_); }
    WriteLockGuard WriteLock() { return WriteLockGuard(lock_); }

    Queue *queue_{nullptr};
    uint64_t seq_{0};
    enum State state_;
    enum Scope scope_{kInternal};
    std::optional<VkExternalFenceHandleTypeFlagBits> imported_handle_type_;  // has value when scope is not kInternal
    mutable std::shared_mutex lock_;
    std::promise<void> completed_;
    std::shared_future<void> waiter_;
    PresentSync present_sync_;
    ValidationStateTracker &dev_data_;
};

}  // namespace vvl
