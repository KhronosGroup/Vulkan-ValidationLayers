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
#include <map>
#include <mutex>
#include "containers/custom_containers.h"
#include "error_message/error_location.h"

class ValidationStateTracker;

namespace vvl {

class Queue;

class Semaphore : public RefcountedStateObject {
  public:
    enum OpType {
        kNone,
        kWait,
        kSignal,
        kBinaryAcquire,
    };
    enum Scope {
        kInternal,
        kExternalTemporary,
        kExternalPermanent,
    };

    struct SemOp {
        OpType op_type;
        uint64_t payload;
        SubmissionReference submit;
        std::optional<Func> acquire_command;

        SemOp(OpType op_type, const SubmissionReference &submit, uint64_t payload)
            : op_type(op_type), payload(payload), submit(submit) {}
        SemOp(Func acquire_command, uint64_t payload)
            : op_type(kBinaryAcquire), payload(payload), acquire_command(acquire_command) {}
    };

    struct TimePoint {
        std::optional<SubmissionReference> signal_submit;
        small_vector<SubmissionReference, 1, uint32_t> wait_submits;
        std::optional<Func> acquire_command;
        std::promise<void> completed;
        std::shared_future<void> waiter;

        TimePoint() : completed(), waiter(completed.get_future()) {}
        bool HasSignaler() const { return signal_submit.has_value() || acquire_command.has_value(); }
        bool HasWaiters() const { return !wait_submits.empty(); }
        void Notify() const;
    };

    Semaphore(ValidationStateTracker &dev, VkSemaphore handle, const VkSemaphoreCreateInfo *pCreateInfo)
        : Semaphore(dev, handle, vku::FindStructInPNextChain<VkSemaphoreTypeCreateInfo>(pCreateInfo->pNext), pCreateInfo) {}

    VkSemaphore VkHandle() const { return handle_.Cast<VkSemaphore>(); }
    enum Scope Scope() const;

    // Enqueue a semaphore operation. For binary semaphores, the payload value is generated and
    // returned, so that every semaphore operation has a unique value.
    void EnqueueSignal(const SubmissionReference &signal_submit, uint64_t &payload);
    void EnqueueWait(const SubmissionReference &wait_submit, uint64_t &payload);

    // Enqueue binary semaphore signal from swapchain image acquire command
    void EnqueueAcquire(Func acquire_command);

    // Helper for retiring timeline semaphores and then retiring all queues using the semaphore
    void NotifyAndWait(const Location &loc, uint64_t payload);

    // Remove completed operations and signal any waiters. This should only be called by Queue
    void Retire(Queue *current_queue, const Location &loc, uint64_t payload);

    // Look for most recent / highest payload operation that matches
    std::optional<SemOp> LastOp(
        const std::function<bool(OpType op_type, uint64_t payload, bool is_pending)> &filter = nullptr) const;

    // Returns pending queue submission that signals this binary semaphore.
    std::optional<SubmissionReference> GetPendingBinarySignalSubmission() const;

    // Returns pending queue submission that waits on this binary semaphore.
    std::optional<SubmissionReference> GetPendingBinaryWaitSubmission() const;

    // Current payload value.
    // If a queue submission command is pending execution, then the returned value may immediately be out of date.
    uint64_t CurrentPayload() const;

    bool CanBinaryBeSignaled() const;
    bool CanBinaryBeWaited() const;

    bool HasImportedHandleType() const;
    VkExternalSemaphoreHandleTypeFlagBits ImportedHandleType() const;
    void Import(VkExternalSemaphoreHandleTypeFlagBits handle_type, VkSemaphoreImportFlags flags);
    void Export(VkExternalSemaphoreHandleTypeFlagBits handle_type);

    const VkSemaphoreType type;
    const VkSemaphoreCreateFlags flags;
    const VkExternalSemaphoreHandleTypeFlags exportHandleTypes;

#ifdef VK_USE_PLATFORM_METAL_EXT
    static bool GetMetalExport(const VkSemaphoreCreateInfo *info);
    const bool metal_semaphore_export;
#endif  // VK_USE_PLATFORM_METAL_EXT

  private:
    Semaphore(ValidationStateTracker &dev, VkSemaphore handle, const VkSemaphoreTypeCreateInfo *type_create_info,
              const VkSemaphoreCreateInfo *pCreateInfo);
    VkExternalSemaphoreHandleTypeFlags GetExportHandleTypes(const VkSemaphoreCreateInfo *pCreateInfo);

    // Signal queue(s) that need to retire because a wait on this payload has finished
    void Notify(uint64_t payload);

    std::shared_future<void> Wait(uint64_t payload);

    ReadLockGuard ReadLock() const { return ReadLockGuard(lock_); }
    WriteLockGuard WriteLock() { return WriteLockGuard(lock_); }

  private:
    enum Scope scope_ { kInternal };
    std::optional<VkExternalSemaphoreHandleTypeFlagBits> imported_handle_type_;  // has value when scope is not kInternal

    // the most recently completed operation
    SemOp completed_;
    // next payload value for binary semaphore operations
    uint64_t next_payload_;

    // Set of pending operations ordered by payload.
    // Timeline operations can be added in any order and multiple wait operations
    // can use the same payload value.
    std::map<uint64_t, TimePoint> timeline_;
    mutable std::shared_mutex lock_;
    ValidationStateTracker &dev_data_;
};

// NOTE: Present semaphores are waited on by the implementation, not queue operations.
// We do not yet have a good way to figure out when this wait completes,
// so we must assume they are safe to re-use.
static inline bool CanSignalBinarySemaphoreAfterOperation(Semaphore::OpType op_type) {
    return op_type == Semaphore::kNone || op_type == Semaphore::kWait;
}
static inline bool CanWaitBinarySemaphoreAfterOperation(Semaphore::OpType op_type) {
    return op_type == Semaphore::kSignal || op_type == Semaphore::kBinaryAcquire;
}

}  // namespace vvl

class CoreChecks;
struct SemaphoreSubmitState {
    const CoreChecks &core;
    VkQueue queue;
    VkQueueFlags queue_flags;

    // This tracks how the payload of a binary semaphore changes **within the current submission**.
    // Before the first wait or signal no map entry for the semaphore is defined, which means that
    // semaphore's state is defined by the previous submissions on this queue or by the submissions on other queues.
    // After the first wait/signal the map starts tracking binary payload value: true - signaled, false - unsignaled.
    vvl::unordered_map<VkSemaphore, bool> binary_signaling_state;

    vvl::unordered_set<VkSemaphore> internal_semaphores;
    vvl::unordered_map<VkSemaphore, uint64_t> timeline_signals;
    vvl::unordered_map<VkSemaphore, uint64_t> timeline_waits;

    SemaphoreSubmitState(const CoreChecks &core_, VkQueue q_, VkQueueFlags queue_flags_)
        : core(core_), queue(q_), queue_flags(queue_flags_) {}

    bool CannotWaitBinary(const vvl::Semaphore &semaphore_state) const;

    VkQueue AnotherQueueWaits(const vvl::Semaphore &semaphore_state) const;

    bool ValidateBinaryWait(const Location &loc, VkQueue queue, const vvl::Semaphore &semaphore_state);
    bool ValidateWaitSemaphore(const Location &wait_semaphore_loc, VkSemaphore semaphore, uint64_t value);
    bool ValidateSignalSemaphore(const Location &signal_semaphore_loc, VkSemaphore semaphore, uint64_t value);

    bool CannotSignalBinary(const vvl::Semaphore &semaphore_state, VkQueue &other_queue, vvl::Func &other_command) const;

    bool CheckSemaphoreValue(const vvl::Semaphore &semaphore_state, std::string &where, uint64_t &bad_value,
                             std::function<bool(const vvl::Semaphore::OpType, uint64_t, bool is_pending)> compare_func);
};