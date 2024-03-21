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

using SemOp = vvl::Semaphore::SemOp;

void vvl::Semaphore::EnqueueSignal(vvl::Queue *queue, uint64_t queue_seq, uint64_t &payload) {
    auto guard = WriteLock();
    if (type == VK_SEMAPHORE_TYPE_BINARY) {
        payload = next_payload_++;
    }
    SemOp sig_op(queue, queue_seq);
    auto result = timeline_.emplace(payload, TimePoint(kSignal, sig_op));
    if (!result.second) {
        // timeline semaphore wait before signal
        result.first->second.signal_op.emplace(sig_op);
    }
}

void vvl::Semaphore::EnqueueWait(vvl::Queue *queue, uint64_t queue_seq, uint64_t &payload) {
    auto guard = WriteLock();
    SemOp wait_op(queue, queue_seq);
    if (type == VK_SEMAPHORE_TYPE_BINARY) {
        if (timeline_.empty()) {
            completed_ = SemOpTemp(wait_op, kWait, payload);
            return;
        }
        payload = timeline_.rbegin()->first;
    } else {
        if (payload <= completed_.payload) {
            return;
        }
    }
    auto result = timeline_.emplace(payload, TimePoint(kWait, wait_op));
    if (!result.second) {
        result.first->second.wait_ops.emplace_back(wait_op);
    }
}

void vvl::Semaphore::EnqueueAcquire(vvl::Func command) {
    auto guard = WriteLock();
    assert(type == VK_SEMAPHORE_TYPE_BINARY);
    auto payload = next_payload_++;
    timeline_.emplace(payload, TimePoint(command));
}

std::optional<vvl::Semaphore::SemOpTemp> vvl::Semaphore::LastOp(const std::function<bool(OpType, uint64_t, bool)> &filter) const {
    auto guard = ReadLock();
    std::optional<SemOpTemp> result;

    for (auto pos = timeline_.rbegin(); pos != timeline_.rend(); ++pos) {
        uint64_t payload = pos->first;
        auto &timepoint = pos->second;
        for (auto &op : timepoint.wait_ops) {
            if (!filter || filter(kWait, payload, true)) {
                result.emplace(SemOpTemp(op, kWait, payload));
                break;
            }
        }
        if (!result && timepoint.signal_op && (!filter || filter(kSignal, payload, true))) {
            result.emplace(SemOpTemp(*timepoint.signal_op, kSignal, payload));
            break;
        }
        if (!result && timepoint.acquire_command && (!filter || filter(kBinaryAcquire, payload, true))) {
            result.emplace(SemOpTemp(*timepoint.acquire_command, payload));
            break;
        }
    }
    if (!result && (!filter || filter(completed_.op_type, completed_.payload, false))) {
        result.emplace(completed_);
    }
    return result;
}

std::optional<vvl::SubmissionLocator> vvl::Semaphore::GetLastBinarySignalSubmission() const {
    assert(type == VK_SEMAPHORE_TYPE_BINARY);
    auto guard = ReadLock();
    if (timeline_.empty()) {
        return {};
    }
    const auto &timepoint = timeline_.rbegin()->second;
    const auto &signal_op = timepoint.signal_op;
    // Binary wait without a signal is not a valid semaphore state (part of binary semaphore validation).
    // Return an empty locator in this case.
    if (!signal_op.has_value()) {
        return {};
    }
    // Also skip signals that are not associated with a queue (e.g. swapchain acquire semaphore signaling).
    if (signal_op->queue == nullptr) {
        return {};
    }
    return vvl::SubmissionLocator{signal_op->queue, signal_op->seq};
}

bool vvl::Semaphore::CanBinaryBeSignaled() const {
    assert(type == VK_SEMAPHORE_TYPE_BINARY);
    auto guard = ReadLock();
    if (timeline_.empty()) {
        return CanSignalBinarySemaphoreAfterOperation(completed_.op_type);
    }
    return timeline_.rbegin()->second.HasWaiters();
}

bool vvl::Semaphore::CanBinaryBeWaited() const {
    assert(type == VK_SEMAPHORE_TYPE_BINARY);
    auto guard = ReadLock();
    if (timeline_.empty()) {
        return CanWaitBinarySemaphoreAfterOperation(completed_.op_type);
    }
    return !timeline_.rbegin()->second.HasWaiters();
}

void vvl::Semaphore::SemOp::Notify() const {
    if (queue) {
        queue->Notify(seq);
    }
}

void vvl::Semaphore::TimePoint::Notify() const {
    if (signal_op) {
        signal_op->Notify();
    }
    for (auto &wait : wait_ops) {
        wait.Notify();
    }
}

void vvl::Semaphore::Notify(uint64_t payload) {
    auto guard = ReadLock();
    auto pos = timeline_.find(payload);
    if (pos != timeline_.end()) {
        pos->second.Notify();
    }
}

void vvl::Semaphore::Retire(vvl::Queue *current_queue, const Location &loc, uint64_t payload) {
    auto guard = WriteLock();
    if (payload <= completed_.payload) {
        return;
    }
    auto pos = timeline_.find(payload);
    assert(pos != timeline_.end());
    auto &timepoint = pos->second;
    timepoint.Notify();

    bool retire_here = false;

    // Retire the operation if it occured on the current queue. Usually this means it is a signal.
    // Note that host operations occur on the null queue. Acquire operations are a special case because
    // the happen asynchronously but there isn't a queue associated with signalling them.
    if (timepoint.signal_op) {
        if (timepoint.signal_op->queue == current_queue) {
            retire_here = true;
        }
    } else if (timepoint.acquire_command) {
        retire_here = true;
    } else {
        // For external semaphores we might not have visibility to the signal op
        if (scope_ != kInternal) {
            retire_here = true;
        }
    }

    if (retire_here) {
        if (timepoint.signal_op) {
            completed_ = SemOpTemp(*timepoint.signal_op, kSignal, payload);
        }
        if (timepoint.acquire_command) {
            completed_ = SemOpTemp(*timepoint.acquire_command, payload);
        }
        for (auto &wait : timepoint.wait_ops) {
            completed_ = SemOpTemp(wait, kWait, payload);
        }
        timepoint.completed.set_value();
        timeline_.erase(timeline_.begin());
        if (scope_ == kExternalTemporary) {
            scope_ = kInternal;
            imported_handle_type_.reset();
        }
    } else {
        // Wait for some other queue or a host operation to retire
        assert(timepoint.waiter.valid());
        // the current timepoint should get destroyed while we're waiting, so copy out the waiter.
        auto waiter = timepoint.waiter;
        guard.unlock();
        auto result = waiter.wait_until(GetCondWaitTimeout());
        if (result != std::future_status::ready) {
            dev_data_.LogError("INTERNAL-ERROR-VkSemaphore-state-timeout", Handle(), loc,
                               "The Validation Layers hit a timeout waiting for timeline semaphore state to update (this is most "
                               "likely a validation bug)."
                               " completed_.payload=%" PRIu64 " wait_payload=%" PRIu64,
                               completed_.payload, payload);
        }
        guard.lock();
    }
}

std::shared_future<void> vvl::Semaphore::Wait(uint64_t payload) {
    auto guard = WriteLock();
    if (payload <= completed_.payload) {
        std::promise<void> already_done;
        auto result = already_done.get_future();
        already_done.set_value();
        return result;
    }
    SemOp wait_op(nullptr, 0);
    auto result = timeline_.emplace(payload, TimePoint(kWait, wait_op));
    auto &timepoint = result.first->second;
    if (!result.second) {
        timepoint.wait_ops.emplace_back(wait_op);
    }
    return timepoint.waiter;
}

void vvl::Semaphore::NotifyAndWait(const Location &loc, uint64_t payload) {
    if (scope_ == kInternal) {
        Notify(payload);
        auto waiter = Wait(payload);
        dev_data_.BeginBlockingOperation();
        auto result = waiter.wait_until(GetCondWaitTimeout());
        dev_data_.EndBlockingOperation();
        if (result != std::future_status::ready) {
            dev_data_.LogError("UNASSIGNED-VkSemaphore-state-timeout", Handle(), loc,
                               "Timeout waiting for timeline semaphore state to update. This is most likely a validation bug."
                               " completed_.payload=%" PRIu64 " wait_payload=%" PRIu64,
                               completed_.payload, payload);
        }
    } else {
        // For external timeline semaphores we should bump the completed payload to whatever the driver
        // tells us. That value may originate from an external process that imported the semaphore and
        // might not be signaled through the application queues.
        //
        // However, there is one exception. The current process can still signal the semaphore, even if
        // it was imported. The queue's semaphore signal should not be overwritten by a potentially
        // external signal. Otherwise, queue information (queue/seq) can be lost, which may prevent the
        // advancement of the queue simulation.
        const auto it = timeline_.find(payload);
        const bool already_signaled = it != timeline_.end() && it->second.signal_op.has_value();
        if (!already_signaled) {
            EnqueueSignal(nullptr, 0, payload);
        }
        Retire(nullptr, loc, payload);
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
            EnqueueWait(last_op->queue, last_op->seq, last_op->payload);
        }
    }
}

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
    // spec (for 003871 but all submit functions have a similar VUID):
    // "When a semaphore wait operation for a binary semaphore is **executed**,
    // as defined by the semaphore member of any element of the pWaitSemaphoreInfos
    // member of any element of pSubmits, there must be no other queues waiting on the same semaphore"
    //
    // For binary semaphores there can be only 1 wait per signal so we just need to check that the
    // last operation isn't a wait. Prior waits will have been removed by prior signals by the time
    // this wait executes.
    auto last_op = semaphore_state.LastOp();
    if (last_op && !CanWaitBinarySemaphoreAfterOperation(last_op->op_type) && last_op->queue &&
        last_op->queue->VkHandle() != queue) {
        return last_op->queue->VkHandle();
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
    other_queue = last_op->queue ? last_op->queue->VkHandle() : VK_NULL_HANDLE;
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
        if (pending->payload == semaphore_state.Completed().payload) {
            where = "current";
        } else {
            where = pending->op_type == vvl::Semaphore::OpType::kSignal ? "pending signal" : "pending wait";
        }
        bad_value = pending->payload;
        return true;
    }
    return false;
}