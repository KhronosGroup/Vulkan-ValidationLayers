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
#include "state_tracker/semaphore_state.h"
#include "state_tracker/queue_state.h"
#include "state_tracker/state_tracker.h"

static VkExternalSemaphoreHandleTypeFlags GetExportHandleTypes(const VkSemaphoreCreateInfo *pCreateInfo) {
    auto export_info = vku::FindStructInPNextChain<VkExportSemaphoreCreateInfo>(pCreateInfo->pNext);
    return export_info ? export_info->handleTypes : 0;
}

void vvl::Semaphore::TimePoint::Notify() const {
    assert(signal_submit.has_value() && signal_submit->queue);
    signal_submit->queue->Notify(signal_submit->seq);
}

vvl::Semaphore::Semaphore(ValidationStateTracker &dev, VkSemaphore handle, const VkSemaphoreTypeCreateInfo *type_create_info,
                          const VkSemaphoreCreateInfo *pCreateInfo)
    : RefcountedStateObject(handle, kVulkanObjectTypeSemaphore),
      type(type_create_info ? type_create_info->semaphoreType : VK_SEMAPHORE_TYPE_BINARY),
      flags(pCreateInfo->flags),
      export_handle_types(GetExportHandleTypes(pCreateInfo)),
      initial_value(type == VK_SEMAPHORE_TYPE_TIMELINE ? type_create_info->initialValue : 0),
#ifdef VK_USE_PLATFORM_METAL_EXT
      metal_semaphore_export(GetMetalExport(pCreateInfo)),
#endif  // VK_USE_PLATFORM_METAL_EXT
      completed_{type == VK_SEMAPHORE_TYPE_TIMELINE ? kSignal : kNone, SubmissionReference{},
                 type_create_info ? type_create_info->initialValue : 0},
      next_payload_(completed_.payload + 1),
      dev_data_(dev) {
}

enum vvl::Semaphore::Scope vvl::Semaphore::Scope() const {
    auto guard = ReadLock();
    return scope_;
}

void vvl::Semaphore::EnqueueSignal(const SubmissionReference &signal_submit, uint64_t &payload) {
    auto guard = WriteLock();
    if (type == VK_SEMAPHORE_TYPE_BINARY) {
        payload = next_payload_++;
    }
    // Check there is no existing signal, validation should enforce this
    assert(timeline_.find(payload) == timeline_.end() || !timeline_.find(payload)->second.signal_submit.has_value());

    timeline_[payload].signal_submit.emplace(signal_submit);
}

void vvl::Semaphore::EnqueueWait(const SubmissionReference &wait_submit, uint64_t &payload) {
    auto guard = WriteLock();
    if (type == VK_SEMAPHORE_TYPE_BINARY) {
        // Timeline can be empty for the binary wait operation if the semaphore was imported.
        // Otherwise timeline should contain a binary signal.
        if (timeline_.empty()) {
            assert(payload == 0);
            completed_ = SemOp(kWait, wait_submit, 0);
            return;
        }
        assert(timeline_.rbegin()->second.HasSignaler());
        payload = timeline_.rbegin()->first;
    } else {
        if (payload <= completed_.payload) {
            return;
        }
    }
    timeline_[payload].wait_submits.emplace_back(wait_submit);
}

void vvl::Semaphore::EnqueueAcquire(vvl::Func acquire_command) {
    assert(type == VK_SEMAPHORE_TYPE_BINARY);
    auto guard = WriteLock();
    auto payload = next_payload_++;
    assert(timeline_.find(payload) == timeline_.end());
    timeline_[payload].acquire_command.emplace(acquire_command);
}

std::optional<vvl::Semaphore::SemOp> vvl::Semaphore::LastOp(const std::function<bool(OpType, uint64_t, bool)> &filter) const {
    auto guard = ReadLock();
    std::optional<SemOp> result;

    for (auto pos = timeline_.rbegin(); pos != timeline_.rend(); ++pos) {
        uint64_t payload = pos->first;
        auto &timepoint = pos->second;
        for (auto &op : timepoint.wait_submits) {
            if (!filter || filter(kWait, payload, true)) {
                result.emplace(SemOp(kWait, op, payload));
                break;
            }
        }
        if (!result && timepoint.signal_submit) {
            // vkSemaphoreSignal can't be a pending operation, it signals immediately
            const bool pending = timepoint.signal_submit->queue != nullptr;

            if (!filter || filter(kSignal, payload, pending)) {
                result.emplace(SemOp(kSignal, *timepoint.signal_submit, payload));
                break;
            }
        }
        if (!result && timepoint.acquire_command && (!filter || filter(kBinaryAcquire, payload, true))) {
            result.emplace(SemOp(*timepoint.acquire_command, payload));
            break;
        }
    }
    if (!result && (!filter || filter(completed_.op_type, completed_.payload, false))) {
        result.emplace(completed_);
    }
    return result;
}

std::optional<vvl::SubmissionReference> vvl::Semaphore::GetPendingBinarySignalSubmission() const {
    assert(type == VK_SEMAPHORE_TYPE_BINARY);
    auto guard = ReadLock();
    if (timeline_.empty()) {
        return {};
    }
    const auto &timepoint = timeline_.rbegin()->second;
    const auto &signal_submit = timepoint.signal_submit;

    // Skip signals that are not associated with a queue
    if (signal_submit.has_value() && signal_submit->queue == nullptr) {
        return {};
    }
    return signal_submit;
}

std::optional<vvl::SubmissionReference> vvl::Semaphore::GetPendingBinaryWaitSubmission() const {
    assert(type == VK_SEMAPHORE_TYPE_BINARY);
    auto guard = ReadLock();
    if (timeline_.empty()) {
        return {};
    }
    const auto &timepoint = timeline_.rbegin()->second;
    assert(timepoint.wait_submits.empty() || timepoint.wait_submits.size() == 1);

    // No waits
    if (timepoint.wait_submits.empty()) {
        return {};
    }
    // Skip waits that are not associated with a queue
    if (timepoint.wait_submits[0].queue == nullptr) {
        return {};
    }
    return timepoint.wait_submits[0];
}

uint64_t vvl::Semaphore::CurrentPayload() const {
    auto guard = ReadLock();
    return completed_.payload;
}

bool vvl::Semaphore::CanBinaryBeSignaled() const {
    assert(type == VK_SEMAPHORE_TYPE_BINARY);
    auto guard = ReadLock();
    if (timeline_.empty()) {
        return CanSignalBinarySemaphoreAfterOperation(completed_.op_type);
    }
    // Every timeline slot of binary semaphore should contain at least a signal.
    // Wait before signal is not allowed.
    assert(timeline_.rbegin()->second.HasSignaler());

    return timeline_.rbegin()->second.HasWaiters();
}

bool vvl::Semaphore::CanBinaryBeWaited() const {
    assert(type == VK_SEMAPHORE_TYPE_BINARY);
    auto guard = ReadLock();
    if (timeline_.empty()) {
        return CanWaitBinarySemaphoreAfterOperation(completed_.op_type);
    }
    // Every timeline slot of binary semaphore should contain at least a signal.
    // Wait before signal is not allowed.
    assert(timeline_.rbegin()->second.HasSignaler());

    return !timeline_.rbegin()->second.HasWaiters();
}

bool vvl::Semaphore::CanRetireBinaryWait(TimePoint &timepoint) const {
    assert(type == VK_SEMAPHORE_TYPE_BINARY);
    // The only allowed configuration when binary semaphore wait does not have a signal
    // is external semaphore. Just retire the wait because there is no guarantee we can
    // track the signal.
    if (!timepoint.signal_submit.has_value()) {
        assert(scope_ != kInternal);
        return true;
    }

    // The resolving signal can only be on another queue (the earlier signals on the
    // current queue are already processed and corresponding timepoints are retired).
    // Initiate forward progress on signaling queue and ask the caller to wait.
    timepoint.Notify();
    return false;
}

bool vvl::Semaphore::CanRetireTimelineWait(const vvl::Queue *current_queue, uint64_t payload) const {
    assert(type == VK_SEMAPHORE_TYPE_TIMELINE);

    // In the correct program the resolving signal is the next signal on the timeline,
    // otherwise this violates the rule of strictly increasing signal values.
    auto it = timeline_.find(payload);
    assert(it != timeline_.end());
    for (; it != timeline_.end(); ++it) {
        const TimePoint &t = it->second;
        if (!t.signal_submit.has_value()) {
            continue;
        }
        // The only exception when the next timeline signal is not a resolving one, when it violates signaling order.
        // Skip such signals.
        //
        // TODO: report validation error (from the queue thread?), use ClosestSignalValueDoesNotFinishWait for testing.
        //
        // DETAILS: if the next signal is on the waiting queue, it can't be a resolving signal (blocked by wait).
        // During retirement phase we know that resolving signal exists -> it has larger value then the next timeline signal.
        // This scenario can't be handled by the core checks - they detect a subset related to vkSignalSemaphore signals,
        // but not other cases. Queue thread has more information about ordering.
        if (t.signal_submit->queue != nullptr && t.signal_submit->queue == current_queue) {
            continue;
        }
        // Found the resolving signal
        break;
    }

    // There is always a resolving signal when we reach a retirement phase (CPU successfully finished waiting on GPU).
    // For external semaphore we might not have visibility of this signal. Just retire the wait.
    if (it == timeline_.end()) {
        assert(scope_ != kInternal);
        return true;
    }

    // Found host signal that finishes this wait
    const TimePoint &t = it->second;
    if (t.signal_submit->queue == nullptr) {
        return true;
    }

    // Notify signaling queue and wait for its queue thread
    t.Notify();
    return false;
}

void vvl::Semaphore::RetireWait(vvl::Queue *current_queue, uint64_t payload, const Location &loc, bool queue_thread) {
    std::shared_future<void> waiter;
    {
        auto guard = WriteLock();
        if (payload <= completed_.payload) {
            return;
        }
        if (scope_ != kInternal) {
            if (!vvl::Find(timeline_, payload)) {
                // GetSemaphoreCounterValue for external semaphore might not have a registered timepoint.
                // Add timepoint so we can retire timeline up to that point.
                assert(type == VK_SEMAPHORE_TYPE_TIMELINE);
                timeline_[payload] = TimePoint{};
            }
            if (scope_ == kExternalTemporary) {
                scope_ = kInternal;
                imported_handle_type_.reset();
            }
        }
        TimePoint &timepoint = vvl::FindExisting(timeline_, payload);

        bool retire = false;
        if (timepoint.acquire_command) {
            retire = true;  // There is resolving acquire signal, timepoint can be retired
        } else if (type == VK_SEMAPHORE_TYPE_BINARY) {
            retire = CanRetireBinaryWait(timepoint);
        } else {
            retire = CanRetireTimelineWait(current_queue, payload);
        }
        if (retire) {
            // SemOp::submit is used only by the binary semaphores.
            // Binary semaphores can have at most one wait per timepoint.
            const auto submit_ref = (type == VK_SEMAPHORE_TYPE_BINARY) ? timepoint.wait_submits[0] : SubmissionReference{};

            RetireTimePoint(payload, kWait, submit_ref);
            return;
        }

        // Wait for some other queue or a host operation to retire
        assert(timepoint.waiter.valid());
        // the current timepoint should get destroyed while we're waiting, so copy out the waiter.
        waiter = timepoint.waiter;
    }
    WaitTimePoint(std::move(waiter), payload, !queue_thread, loc);
}

void vvl::Semaphore::RetireSignal(uint64_t payload) {
    auto guard = WriteLock();
    if (payload <= completed_.payload) {
        return;
    }
    TimePoint &timepoint = vvl::FindExisting(timeline_, payload);
    assert(timepoint.signal_submit.has_value());

    OpType completed_op = kSignal;
    SubmissionReference completed_submit = *timepoint.signal_submit;

    // If there is a wait operation then mark it as the last completed instead.
    // The reason to do this here instead on the waiter side (after it is unblocked)
    // is because signal can have larger (timeline) value than corresponding wait value.
    // In this case it's the signal that defines the last completed value.
    if (!timepoint.wait_submits.empty()) {
        completed_op = kWait;
        // SemOp::submit is used only for binary semaphores which can have only single wait
        completed_submit = timepoint.wait_submits[0];
    }

    RetireTimePoint(payload, completed_op, completed_submit);
}

void vvl::Semaphore::RetireTimePoint(uint64_t payload, OpType completed_op, SubmissionReference completed_submit) {
    auto it = timeline_.begin();
    while (it != timeline_.end() && it->first <= payload) {
        assert(it->first > completed_.payload);
        it->second.completed.set_value();
        ++it;
    }
    timeline_.erase(timeline_.begin(), it);
    completed_ = SemOp(completed_op, completed_submit, payload);
}

void vvl::Semaphore::WaitTimePoint(std::shared_future<void> &&waiter, uint64_t payload, bool unblock_validation_object,
                                   const Location &loc) {
    if (unblock_validation_object) {
        dev_data_.BeginBlockingOperation();
    }

    auto result = waiter.wait_until(GetCondWaitTimeout());

    if (unblock_validation_object) {
        dev_data_.EndBlockingOperation();
    }

    if (result != std::future_status::ready) {
        dev_data_.LogError("INTERNAL-ERROR-VkSemaphore-state-timeout", Handle(), loc,
                           "The Validation Layers hit a timeout waiting for timeline semaphore state to update (this is most "
                           "likely a validation bug). completed_.payload=%" PRIu64 " wait_payload=%" PRIu64,
                           completed_.payload, payload);
    }
}

void vvl::Semaphore::Import(VkExternalSemaphoreHandleTypeFlagBits handle_type, VkSemaphoreImportFlags flags) {
    auto guard = WriteLock();
    if (scope_ != kExternalPermanent) {
        if ((handle_type == VK_EXTERNAL_SEMAPHORE_HANDLE_TYPE_SYNC_FD_BIT || flags & VK_SEMAPHORE_IMPORT_TEMPORARY_BIT) &&
            scope_ == kInternal) {
            scope_ = kExternalTemporary;
        } else {
            scope_ = kExternalPermanent;
        }
    }
    imported_handle_type_ = handle_type;
}

void vvl::Semaphore::Export(VkExternalSemaphoreHandleTypeFlagBits handle_type) {
    if (handle_type != VK_EXTERNAL_SEMAPHORE_HANDLE_TYPE_SYNC_FD_BIT) {
        // Cannot track semaphore state once it is exported, except for Sync FD handle types which have copy transference
        auto guard = WriteLock();
        scope_ = kExternalPermanent;
    } else {
        assert(type == VK_SEMAPHORE_TYPE_BINARY);  // checked by validation phase
        // Exporting a semaphore payload to a handle with copy transference has the same side effects on the source semaphore's
        // payload as executing a semaphore wait operation
        auto filter = [](const Semaphore::OpType op_type, uint64_t payload, bool is_pending) {
            return is_pending && CanWaitBinarySemaphoreAfterOperation(op_type);
        };
        auto last_op = LastOp(filter);
        if (last_op) {
            EnqueueWait(last_op->submit, last_op->payload);
        }
    }
}

std::optional<VkExternalSemaphoreHandleTypeFlagBits> vvl::Semaphore::ImportedHandleType() const {
    auto guard = ReadLock();

    // Sanity check: semaphore imported -> scope is not internal
    assert(!imported_handle_type_.has_value() || scope_ != kInternal);

    return imported_handle_type_;
}

#ifdef VK_USE_PLATFORM_METAL_EXT
bool vvl::Semaphore::GetMetalExport(const VkSemaphoreCreateInfo *info) {
    bool retval = false;
    auto export_metal_object_info = vku::FindStructInPNextChain<VkExportMetalObjectCreateInfoEXT>(info->pNext);
    while (export_metal_object_info) {
        if (export_metal_object_info->exportObjectType == VK_EXPORT_METAL_OBJECT_TYPE_METAL_SHARED_EVENT_BIT_EXT) {
            retval = true;
            break;
        }
        export_metal_object_info = vku::FindStructInPNextChain<VkExportMetalObjectCreateInfoEXT>(export_metal_object_info->pNext);
    }
    return retval;
}
#endif  // VK_USE_PLATFORM_METAL_EXT

bool SemaphoreSubmitState::CannotWaitBinary(const vvl::Semaphore &semaphore_state) const {
    assert(semaphore_state.type == VK_SEMAPHORE_TYPE_BINARY);
    const auto semaphore = semaphore_state.VkHandle();

    // Check if this submission has signaled or unsignaled the semaphore
    if (const auto it = binary_signaling_state.find(semaphore); it != binary_signaling_state.end()) {
        const bool signaled = it->second;
        return !signaled;  // not signaled => can't wait
    }
    // If not, then query semaphore's payload set by other submissions.
    // This will return either the payload set by the previous submissions on the current queue,
    // or the payload that is currently being updated by the async running queues.
    return !semaphore_state.CanBinaryBeWaited();
}

VkQueue SemaphoreSubmitState::AnotherQueueWaits(const vvl::Semaphore &semaphore_state) const {
    // VUID-vkQueueSubmit-pWaitSemaphores-00068 (and similar VUs):
    // "When a semaphore wait operation referring to a binary semaphore defined
    //  by any element of the pWaitSemaphores member of any element of pSubmits
    //  executes on queue, there must be no other queues waiting on the same semaphore"
    auto pending_wait_submit = semaphore_state.GetPendingBinaryWaitSubmission();
    if (pending_wait_submit && pending_wait_submit->queue->VkHandle() != queue) {
        return pending_wait_submit->queue->VkHandle();
    }
    return VK_NULL_HANDLE;
}

bool SemaphoreSubmitState::CannotSignalBinary(const vvl::Semaphore &semaphore_state, VkQueue &other_queue,
                                              vvl::Func &other_command) const {
    assert(semaphore_state.type == VK_SEMAPHORE_TYPE_BINARY);
    const auto semaphore = semaphore_state.VkHandle();

    // Check if this submission has signaled or unsignaled the semaphore
    if (const auto it = binary_signaling_state.find(semaphore); it != binary_signaling_state.end()) {
        const bool signaled = it->second;
        if (!signaled) {
            return false;  // not signaled => can't wait
        }
        other_queue = queue;
        other_command = vvl::Func::Empty;
        return true;  // signaled => can wait
    }
    // If not, get signaling state from the semaphore's last op.
    // Last op was recorded either by the previous sumbissions on this queue,
    // or it's a hot state from async running queues (so can get outdated immediately after was read).
    const auto last_op = semaphore_state.LastOp();
    if (!last_op || CanSignalBinarySemaphoreAfterOperation(last_op->op_type)) {
        return false;
    }
    other_queue = last_op->submit.queue ? last_op->submit.queue->VkHandle() : VK_NULL_HANDLE;
    other_command = last_op->acquire_command ? *last_op->acquire_command : vvl::Func::Empty;
    return true;
}

bool SemaphoreSubmitState::CheckSemaphoreValue(
    const vvl::Semaphore &semaphore_state, std::string &where, uint64_t &bad_value,
    std::function<bool(const vvl::Semaphore::OpType, uint64_t, bool is_pending)> compare_func) {
    auto current_signal = timeline_signals.find(semaphore_state.VkHandle());
    // NOTE: for purposes of validation, duplicate operations in the same submission are not yet pending.
    if (current_signal != timeline_signals.end()) {
        if (compare_func(vvl::Semaphore::kSignal, current_signal->second, false)) {
            where = "current submit's signal";
            bad_value = current_signal->second;
            return true;
        }
    }
    auto current_wait = timeline_waits.find(semaphore_state.VkHandle());
    if (current_wait != timeline_waits.end()) {
        if (compare_func(vvl::Semaphore::kWait, current_wait->second, false)) {
            where = "current submit's wait";
            bad_value = current_wait->second;
            return true;
        }
    }
    auto pending = semaphore_state.LastOp(compare_func);
    if (pending) {
        if (pending->payload == semaphore_state.CurrentPayload()) {
            where = "current";
        } else {
            where = pending->op_type == vvl::Semaphore::OpType::kSignal ? "pending signal" : "pending wait";
        }
        bad_value = pending->payload;
        return true;
    }
    return false;
}