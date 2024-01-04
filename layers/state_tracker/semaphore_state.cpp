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
    SemOp sig_op(kSignal, queue, queue_seq, payload);
    auto result = timeline_.emplace(payload, sig_op);
    if (!result.second) {
        // timeline semaphore wait before signal
        result.first->second.signal_op.emplace(sig_op);
    }
}

void vvl::Semaphore::EnqueueWait(vvl::Queue *queue, uint64_t queue_seq, uint64_t &payload) {
    auto guard = WriteLock();
    SemOp wait_op(kWait, queue, queue_seq, payload);
    if (type == VK_SEMAPHORE_TYPE_BINARY) {
        if (timeline_.empty()) {
            completed_ = wait_op;
            return;
        }
        payload = timeline_.rbegin()->first;
        wait_op.payload = payload;
    } else {
        if (payload <= completed_.payload) {
            return;
        }
    }
    auto result = timeline_.emplace(payload, TimePoint(wait_op));
    if (!result.second) {
        result.first->second.AddWaitOp(wait_op);
    }
}

void vvl::Semaphore::EnqueueAcquire(vvl::Func command) {
    auto guard = WriteLock();
    assert(type == VK_SEMAPHORE_TYPE_BINARY);
    auto payload = next_payload_++;
    SemOp acquire(kBinaryAcquire, nullptr, 0, payload, command);
    timeline_.emplace(payload, acquire);
}

std::optional<SemOp> vvl::Semaphore::LastOp(const std::function<bool(const SemOp &, bool)> &filter) const {
    auto guard = ReadLock();
    std::optional<SemOp> result;

    for (auto pos = timeline_.rbegin(); pos != timeline_.rend(); ++pos) {
        auto &timepoint = pos->second;
        for (auto &op : timepoint.wait_ops) {
            assert(op.payload == timepoint.wait_ops[0].payload);
            if (!filter || filter(op, true)) {
                result.emplace(op);
                break;
            }
        }
        if (!result && timepoint.signal_op && (!filter || filter(*timepoint.signal_op, true))) {
            result.emplace(*timepoint.signal_op);
            break;
        }
    }
    if (!result && (!filter || filter(completed_, false))) {
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
        assert(wait.payload == wait_ops[0].payload);
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
        if ((timepoint.signal_op->queue == current_queue || timepoint.signal_op->IsAcquire())) {
            retire_here = true;
        }
    } else {
        // For external semaphores we might not have visibility to the signal op
        if (scope_ != kInternal) {
            retire_here = true;
        }
    }

    if (retire_here) {
        if (timepoint.signal_op) {
            completed_ = *timepoint.signal_op;
        }
        for (auto &wait : timepoint.wait_ops) {
            assert(wait.payload == timepoint.wait_ops[0].payload);
            completed_ = wait;
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
    SemOp wait_op(kWait, nullptr, 0, payload);
    auto result = timeline_.emplace(payload, TimePoint(wait_op));
    auto &timepoint = result.first->second;
    if (!result.second) {
        timepoint.AddWaitOp(wait_op);
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
        auto filter = [](const Semaphore::SemOp &op, bool is_pending) {
            return is_pending && CanWaitBinarySemaphoreAfterOperation(op.op_type);
        };
        auto last_op = LastOp(filter);
        if (last_op) {
            EnqueueWait(last_op->queue, last_op->seq, last_op->payload);
        }
    }
}
