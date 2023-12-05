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
#include "state_tracker/fence_state.h"
#include "state_tracker/queue_state.h"
#include "state_tracker/state_tracker.h"

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

// Called from a non-queue operation, such as vkWaitForFences()
void vvl::Fence::NotifyAndWait(const Location &loc) {
    std::shared_future<void> waiter;
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
        }
    }
    if (waiter.valid()) {
        auto result = waiter.wait_until(GetCondWaitTimeout());
        if (result != std::future_status::ready) {
            dev_data_.LogError(
                "INTERNAL-ERROR-VkFence-state-timeout", Handle(), loc,
                "The Validation Layers hit a timeout waiting for fence state to update (this is most likely a validation bug).");
        }
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
    }
    state_ = kUnsignaled;
    completed_ = std::promise<void>();
    waiter_ = std::shared_future<void>(completed_.get_future());
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
        }
        state_ = kUnsignaled;
        completed_ = std::promise<void>();
        waiter_ = std::shared_future<void>(completed_.get_future());
    }
}

