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

vvl::Semaphore::TimePoint::TimePoint(OpType op_type, SubmissionReference &submit_ref)
    : signal_ref(), completed(), waiter(completed.get_future()) {
    if (op_type == kWait) {
        wait_refs.emplace_back(submit_ref);
    } else {
        assert(op_type == kSignal);
        signal_ref.emplace(submit_ref);
    }
}

void vvl::Semaphore::TimePoint::Notify() const {
    if (signal_ref && signal_ref->queue) {
        signal_ref->queue->Notify(signal_ref->seq);
    }
    for (auto &wait : wait_refs) {
        if (wait.queue) {
            wait.queue->Notify(wait.seq);
        }
    }
}

vvl::Semaphore::Semaphore(ValidationStateTracker &dev, VkSemaphore handle, const VkSemaphoreTypeCreateInfo *type_create_info,
                          const VkSemaphoreCreateInfo *pCreateInfo)
    : RefcountedStateObject(handle, kVulkanObjectTypeSemaphore),
      type(type_create_info ? type_create_info->semaphoreType : VK_SEMAPHORE_TYPE_BINARY),
      flags(pCreateInfo->flags),
      exportHandleTypes(GetExportHandleTypes(pCreateInfo)),
#ifdef VK_USE_PLATFORM_METAL_EXT
      metal_semaphore_export(GetMetalExport(pCreateInfo)),
#endif  // VK_USE_PLATFORM_METAL_EXT
      completed_{SubmissionReference{}, type == VK_SEMAPHORE_TYPE_TIMELINE ? kSignal : kNone,
                 type_create_info ? type_create_info->initialValue : 0},
      next_payload_(completed_.payload + 1),
      dev_data_(dev) {
}

enum vvl::Semaphore::Scope vvl::Semaphore::Scope() const {
    auto guard = ReadLock();
    return scope_;
}

void vvl::Semaphore::EnqueueSignal(vvl::Queue *queue, uint64_t queue_seq, uint64_t &payload) {
    auto guard = WriteLock();
    if (type == VK_SEMAPHORE_TYPE_BINARY) {
        payload = next_payload_++;
    }
    SubmissionReference signal_ref(queue, queue_seq);
    auto result = timeline_.emplace(payload, TimePoint(kSignal, signal_ref));
    if (!result.second) {
        // timeline semaphore wait before signal
        result.first->second.signal_ref.emplace(signal_ref);
    }
}

void vvl::Semaphore::EnqueueWait(vvl::Queue *queue, uint64_t queue_seq, uint64_t &payload) {
    auto guard = WriteLock();
    SubmissionReference wait_ref(queue, queue_seq);
    if (type == VK_SEMAPHORE_TYPE_BINARY) {
        if (timeline_.empty()) {
            completed_ = SemOp(wait_ref, kWait, payload);
            return;
        }
        payload = timeline_.rbegin()->first;
    } else {
        if (payload <= completed_.payload) {
            return;
        }
    }
    auto result = timeline_.emplace(payload, TimePoint(kWait, wait_ref));
    if (!result.second) {
        result.first->second.wait_refs.emplace_back(wait_ref);
    }
}

void vvl::Semaphore::EnqueueAcquire(vvl::Func command) {
    auto guard = WriteLock();
    assert(type == VK_SEMAPHORE_TYPE_BINARY);
    auto payload = next_payload_++;
    timeline_.emplace(payload, TimePoint(command));
}

std::optional<vvl::Semaphore::SemOp> vvl::Semaphore::LastOp(const std::function<bool(OpType, uint64_t, bool)> &filter) const {
    auto guard = ReadLock();
    std::optional<SemOp> result;

    for (auto pos = timeline_.rbegin(); pos != timeline_.rend(); ++pos) {
        uint64_t payload = pos->first;
        auto &timepoint = pos->second;
        for (auto &op : timepoint.wait_refs) {
            if (!filter || filter(kWait, payload, true)) {
                result.emplace(SemOp(op, kWait, payload));
                break;
            }
        }
        if (!result && timepoint.signal_ref && (!filter || filter(kSignal, payload, true))) {
            result.emplace(SemOp(*timepoint.signal_ref, kSignal, payload));
            break;
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

std::optional<vvl::SubmissionReference> vvl::Semaphore::GetLastBinarySignalSubmission() const {
    assert(type == VK_SEMAPHORE_TYPE_BINARY);
    auto guard = ReadLock();
    if (timeline_.empty()) {
        return {};
    }
    const auto &timepoint = timeline_.rbegin()->second;
    const auto &signal_ref = timepoint.signal_ref;
    // Binary wait without a signal is not a valid semaphore state (part of binary semaphore validation).
    // Return an empty locator in this case.
    if (!signal_ref.has_value()) {
        return {};
    }
    // Also skip signals that are not associated with a queue (e.g. swapchain acquire semaphore signaling).
    if (signal_ref->queue == nullptr) {
        return {};
    }
    return vvl::SubmissionReference{signal_ref->queue, signal_ref->seq};
}

vvl::Semaphore::SemOp vvl::Semaphore::Completed() const {
    auto guard = ReadLock();
    return completed_;
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
    if (timepoint.signal_ref) {
        if (timepoint.signal_ref->queue == current_queue) {
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
        if (timepoint.signal_ref) {
            completed_ = SemOp(*timepoint.signal_ref, kSignal, payload);
        }
        if (timepoint.acquire_command) {
            completed_ = SemOp(*timepoint.acquire_command, payload);
        }
        for (auto &wait : timepoint.wait_refs) {
            completed_ = SemOp(wait, kWait, payload);
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
    SubmissionReference wait_ref(nullptr, 0);
    auto result = timeline_.emplace(payload, TimePoint(kWait, wait_ref));
    auto &timepoint = result.first->second;
    if (!result.second) {
        timepoint.wait_refs.emplace_back(wait_ref);
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
        const bool already_signaled = it != timeline_.end() && it->second.signal_ref.has_value();
        if (!already_signaled) {
            EnqueueSignal(nullptr, 0, payload);
        }
        Retire(nullptr, loc, payload);
    }
}

VkExternalSemaphoreHandleTypeFlags vvl::Semaphore::GetExportHandleTypes(const VkSemaphoreCreateInfo *pCreateInfo) {
    auto export_info = vku::FindStructInPNextChain<VkExportSemaphoreCreateInfo>(pCreateInfo->pNext);
    return export_info ? export_info->handleTypes : 0;
}

bool vvl::Semaphore::HasImportedHandleType() const {
    auto guard = ReadLock();
    return imported_handle_type_.has_value();
}

VkExternalSemaphoreHandleTypeFlagBits vvl::Semaphore::ImportedHandleType() const {
    auto guard = ReadLock();
    assert(imported_handle_type_.has_value());
    return imported_handle_type_.value();
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