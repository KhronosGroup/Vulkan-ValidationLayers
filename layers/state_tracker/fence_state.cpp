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
#include "state_tracker/fence_state.h"
#include "state_tracker/queue_state.h"
#include "state_tracker/state_tracker.h"

static VkExternalFenceHandleTypeFlags GetExportHandleTypes(const VkFenceCreateInfo *info) {
    auto export_info = vku::FindStructInPNextChain<VkExportFenceCreateInfo>(info->pNext);
    return export_info ? export_info->handleTypes : 0;
}

vvl::Fence::Fence(Logger &logger, VkFence handle, const VkFenceCreateInfo *pCreateInfo)
    : RefcountedStateObject(handle, kVulkanObjectTypeFence),
      flags(pCreateInfo->flags),
      export_handle_types(GetExportHandleTypes(pCreateInfo)),
      state_((pCreateInfo->flags & VK_FENCE_CREATE_SIGNALED_BIT) ? kRetired : kUnsignaled),
      completed_(),
      waiter_(completed_.get_future()),
      logger_(logger) {}

const VulkanTypedHandle *vvl::Fence::InUse() const {
    auto guard = ReadLock();
    // Fence does not have a parent (in the sense of a VVL state object), and the value returned
    // by the base class InUse is not useful for reporting (it is the fence's own handle)
    const bool in_use = RefcountedStateObject::InUse() != nullptr;
    if (!in_use) {
        return nullptr;
    }
    // If the fence is in-use there should be a queue that uses it.
    // NOTE: in-use checks are always with regard to queue operations.
    assert(queue_ != nullptr && "Can't find queue that uses the fence");
    if (queue_) {
        return &queue_->Handle();
    }
    static const VulkanTypedHandle empty{};
    return &empty;
}

bool vvl::Fence::EnqueueSignal(vvl::Queue *queue_state, uint64_t next_seq) {
    auto guard = WriteLock();
    if (scope_ != kInternal) {
        return true;
    }
    // Mark fence in use
    state_ = kInflight;
    queue_ = queue_state;
    seq_ = next_seq;
    return false;
}

// Called from a non-queue operation, such as vkWaitForFences()|
void vvl::Fence::NotifyAndWait(const Location &loc) {
    std::shared_future<void> waiter;
    std::optional<SubmissionReference> present_submission_ref;
    {
        // Hold the lock only while updating members, but not
        // while waiting
        auto guard = WriteLock();
        if (state_ == kInflight) {
            if (queue_) {
                queue_->Notify(seq_);
                waiter = waiter_;
            } else {
                state_ = kRetired;
                completed_.set_value();
                queue_ = nullptr;
                seq_ = 0;
            }
            present_submission_ref = std::move(present_submission_ref_);
            present_submission_ref_.reset();
        }
        // Cleanup wait semaphores.
        // NOTE: Functions like QueueWaitIdle put fence in the retired state, still it can have
        // the list of present semaphores, which are not cleared by QueueWaitIdle when swapchain
        // maintenance extension is enabled. That's the reason this code is not under kInflight condition.
        for (auto &semaphore : present_wait_semaphores_) {
            semaphore->ClearSwapchainWaitInfo();
        }
        present_wait_semaphores_.clear();
    }
    if (waiter.valid()) {
        auto result = waiter.wait_until(GetCondWaitTimeout());
        if (result != std::future_status::ready) {
            logger_.LogError(
                "INTERNAL-ERROR-VkFence-state-timeout", Handle(), loc,
                "The Validation Layers hit a timeout waiting for fence state to update (this is most likely a validation bug).");
        }
    }
    if (present_submission_ref.has_value()) {
        present_submission_ref->queue->NotifyAndWait(loc, present_submission_ref->seq);
    }
}

// Retire from a queue operation
void vvl::Fence::Retire() {
    auto guard = WriteLock();
    if (state_ == kInflight) {
        state_ = kRetired;
        completed_.set_value();
        queue_ = nullptr;
        seq_ = 0;
    }
}

void vvl::Fence::Reset() {
    auto guard = WriteLock();
    queue_ = nullptr;
    seq_ = 0;
    // spec: If any member of pFences currently has its payload imported with temporary permanence,
    // that fence’s prior permanent payload is first restored. The remaining operations described
    // therefore operate on the restored payload.
    if (scope_ == kExternalTemporary) {
        scope_ = kInternal;
        imported_handle_type_.reset();
    }
    state_ = kUnsignaled;
    completed_ = std::promise<void>();
    waiter_ = std::shared_future<void>(completed_.get_future());
    present_submission_ref_.reset();

    // Do not reset swapchain-in-use state of each semaphore here, only stop the tracking.
    // In order to reset swapchain-in-use state we need to wait on the fence.
    present_wait_semaphores_.clear();
}

void vvl::Fence::Import(VkExternalFenceHandleTypeFlagBits handle_type, VkFenceImportFlags flags) {
    auto guard = WriteLock();
    if (scope_ != kExternalPermanent) {
        if (handle_type != VK_EXTERNAL_FENCE_HANDLE_TYPE_SYNC_FD_BIT && (flags & VK_FENCE_IMPORT_TEMPORARY_BIT) == 0) {
            scope_ = kExternalPermanent;
        } else if (scope_ == kInternal) {
            scope_ = kExternalTemporary;
        }
    }
    imported_handle_type_ = handle_type;
}

void vvl::Fence::Export(VkExternalFenceHandleTypeFlagBits handle_type) {
    auto guard = WriteLock();
    if (handle_type != VK_EXTERNAL_FENCE_HANDLE_TYPE_SYNC_FD_BIT) {
        // Export with reference transference becomes external
        scope_ = kExternalPermanent;
    } else {
        // Export with copy transference has a side effect of resetting the fence
        if (scope_ == kExternalTemporary) {
            scope_ = kInternal;
            imported_handle_type_.reset();
        }
        state_ = kUnsignaled;
        completed_ = std::promise<void>();
        waiter_ = std::shared_future<void>(completed_.get_future());
    }
}

std::optional<VkExternalFenceHandleTypeFlagBits> vvl::Fence::ImportedHandleType() const {
    auto guard = ReadLock();

    // Sanity check: fence imported -> scope is not internal
    assert(!imported_handle_type_.has_value() || scope_ != kInternal);

    return imported_handle_type_;
}

void vvl::Fence::SetPresentSubmissionRef(const SubmissionReference &present_submission_ref) {
    auto guard = WriteLock();
    // In an error-free scenario, "present_submission_ref_" member has no value (as optional).
    // If the fence is reused without being waited on/reset (which causes a validation error),
    // then we may find a stale ref value here. Simply overwrite it with a new ref.
    //
    // VVL INSIGHT: Interestingly, stale ref value can't happen in VVL tests because this function
    // (from the Record phase) is not called if validation fails. However, the Record phase is
    // always called for regular applications even in the case of validation errors.
    // This means we cannot assert that submission ref member is empty. Such an assert would be
    // valid for VVL tests but not for regular apps. It's not a big deal, but this observation
    // may be useful to get better understanding of VVL internals.
    //
    // assert(!present_submission_ref_.has_value()); - only valid for VVL tests

    assert(present_submission_ref.queue != nullptr);
    present_submission_ref_ = present_submission_ref;
}

void vvl::Fence::SetPresentWaitSemaphores(vvl::span<std::shared_ptr<vvl::Semaphore>> present_wait_semaphores) {
    present_wait_semaphores_.clear();
    for (const auto &semaphore : present_wait_semaphores) {
        present_wait_semaphores_.emplace_back(semaphore);
    }
}
