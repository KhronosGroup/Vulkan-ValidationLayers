/* Copyright (c) 2015-2025 The Khronos Group Inc.
 * Copyright (c) 2015-2025 Valve Corporation
 * Copyright (c) 2015-2025 LunarG, Inc.
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
#include "containers/span.h"
#include <optional>
#include <future>

class Logger;

namespace vvl {
class Semaphore;
class Queue;

class Fence : public RefcountedStateObject {
  public:
    enum State { kUnsignaled, kInflight, kRetired };
    enum Scope {
        kInternal,
        kExternalTemporary,
        kExternalPermanent,
    };

    Fence(Logger &logger, VkFence handle, const VkFenceCreateInfo *pCreateInfo);

    const VulkanTypedHandle *InUse() const override;
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

    // Called on AcquireNextImage fence
    void SetPresentSubmissionRef(const SubmissionReference &present_submission_ref);

    // Called on VkSwapchainPresentFenceInfoEXT fence
    void SetPresentWaitSemaphores(vvl::span<std::shared_ptr<vvl::Semaphore>> present_wait_semaphores);

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
    Logger &logger_;

    // The present queue submission is notified when WaitForFences waits for the image acquire fence.
    //
    // This implements a synchronization scheme when a fence from the AcquireNextImage is used to
    // synchronize with the last frame that used the same swapchain image. This is in contrast to
    // a more common approach when the fence from QueueSubmit is used for frame synchronization.
    //
    // The sequence that connects image acquire fence to one of the previous submissions (in chronologically backward order):
    //  a) the wait on the image acquire fence is finished ->
    //  b) the corresponding image is acquired ->
    //  c) the previous presentation of this image is finished (except for the first acquire) ->
    //  d) corresponding present request finished waiting on the queue submit semaphore ->
    //  e) corresponding queue batch finished execution and signaled that semaphore
    //
    // Acquire fence synchronization allows the use of the fence from step a) to wait on the
    // submit batch from step e) and also to ensure that the semaphore from step e) is no longer in use.
    //
    std::optional<SubmissionReference> present_submission_ref_;

    small_vector<std::shared_ptr<vvl::Semaphore>, 1> present_wait_semaphores_;
};

}  // namespace vvl
