/* Copyright (c) 2015-2022 The Khronos Group Inc.
 * Copyright (c) 2015-2022 Valve Corporation
 * Copyright (c) 2015-2022 LunarG, Inc.
 * Copyright (C) 2015-2022 Google Inc.
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
 *
 * Author: Courtney Goeltzenleuchter <courtneygo@google.com>
 * Author: Tobin Ehlis <tobine@google.com>
 * Author: Chris Forbes <chrisf@ijw.co.nz>
 * Author: Mark Lobodzinski <mark@lunarg.com>
 * Author: Dave Houlton <daveh@lunarg.com>
 * Author: John Zulauf <jzulauf@lunarg.com>
 * Author: Tobias Hector <tobias.hector@amd.com>
 */
#include "queue_state.h"
#include "cmd_buffer_state.h"
#include "state_tracker.h"

using SemOp = SEMAPHORE_STATE::SemOp;

// This timeout is for all queue threads to update their state after we know
// (via being in a PostRecord call) that a fence, semaphore or wait for idle has
// completed. Hitting it is almost a certainly a bug in this code.
static std::chrono::time_point<std::chrono::steady_clock> GetCondWaitTimeout() {
    return std::chrono::steady_clock::now() + std::chrono::seconds(10);
}

void CB_SUBMISSION::BeginUse() {
    for (auto &wait : wait_semaphores) {
        wait.semaphore->BeginUse();
    }
    for (auto &cb_node : cbs) {
        cb_node->BeginUse();
    }
    for (auto &signal : signal_semaphores) {
        signal.semaphore->BeginUse();
    }
    if (fence) {
        fence->BeginUse();
    }
}

void CB_SUBMISSION::EndUse() {
    for (auto &wait : wait_semaphores) {
        wait.semaphore->EndUse();
    }
    for (auto &cb_node : cbs) {
        cb_node->EndUse();
    }
    for (auto &signal : signal_semaphores) {
        signal.semaphore->EndUse();
    }
    if (fence) {
        fence->EndUse();
    }
}

uint64_t QUEUE_STATE::Submit(CB_SUBMISSION &&submission) {
    for (auto &cb_node : submission.cbs) {
        auto cb_guard = cb_node->WriteLock();
        for (auto *secondary_cmd_buffer : cb_node->linkedCommandBuffers) {
            auto secondary_guard = secondary_cmd_buffer->WriteLock();
            secondary_cmd_buffer->IncrementResources();
        }
        cb_node->IncrementResources();
        cb_node->Submit(submission.perf_submit_pass);
    }
    // seq_ is atomic so we don't need a lock until updating the deque below.
    // Note that this relies on the external synchonization requirements for the
    // VkQueue
    submission.seq = ++seq_;
    submission.BeginUse();
    bool retire_early = false;
    for (auto &wait : submission.wait_semaphores) {
        wait.semaphore->EnqueueWait(this, submission.seq, wait.payload);
    }

    for (auto &signal : submission.signal_semaphores) {
        if (signal.semaphore->EnqueueSignal(this, submission.seq, signal.payload)) {
            retire_early = true;
        }
    }

    if (submission.fence) {
        if (submission.fence->EnqueueSignal(this, submission.seq)) {
            retire_early = true;
        }
    }
    {
        auto guard = Lock();
        submissions_.emplace_back(std::move(submission));
        if (!thread_) {
            thread_ = layer_data::make_unique<std::thread>(&QUEUE_STATE::ThreadFunc, this);
        }
    }
    return retire_early ? submission.seq : 0;
}

std::shared_future<void> QUEUE_STATE::Wait(uint64_t until_seq) {
    auto guard = Lock();
    if (until_seq == UINT64_MAX) {
        until_seq = seq_;
    }
    if (submissions_.empty() || until_seq < submissions_.begin()->seq) {
        std::promise<void> already_done;
        auto result = already_done.get_future();
        already_done.set_value();
        return result;
    }
    auto index = until_seq - submissions_.begin()->seq;
    assert(index < submissions_.size());
    // Make sure we don't overflow if size_t is 32 bit
    assert(index < std::numeric_limits<size_t>::max());
    return submissions_[static_cast<size_t>(index)].waiter;
}

void QUEUE_STATE::NotifyAndWait(uint64_t until_seq) {
    until_seq = Notify(until_seq);
    auto waiter = Wait(until_seq);
    auto result = waiter.wait_until(GetCondWaitTimeout());
    if (result != std::future_status::ready) {
        dev_data_.LogError(Handle(), "UNASSIGNED-VkQueue-state-timeout",
                           "Timeout waiting for queue state to update. seq=%" PRIu64,
                           until_seq);
    }
}

uint64_t QUEUE_STATE::Notify(uint64_t until_seq) {
    auto guard = Lock();
    if (until_seq == UINT64_MAX) {
        until_seq = seq_;
    }
    if (request_seq_ < until_seq) {
        request_seq_ = until_seq;
    }
    cond_.notify_one();
    return until_seq;
}

void QUEUE_STATE::Destroy() {
    std::unique_ptr<std::thread> dead_thread;
    {
        auto guard = Lock();
        exit_thread_ = true;
        cond_.notify_all();
        dead_thread = std::move(thread_);
    }
    if (dead_thread && dead_thread->joinable()) {
        dead_thread->join();
        dead_thread.reset();
    }
    BASE_NODE::Destroy();
}

layer_data::optional<CB_SUBMISSION> QUEUE_STATE::NextSubmission() {
    layer_data::optional<CB_SUBMISSION> result;
    // Pop the next submission off of the queue so that the thread function doesn't need to worry
    // about locking.
    auto guard = Lock();
    while (!exit_thread_ && (submissions_.empty() || request_seq_ < submissions_.front().seq)) {
        // The queue thread must wait forever if nothing is happening, until we tell it to exit
        cond_.wait(guard);
    }
    if (!exit_thread_) {
        result.emplace(std::move(submissions_.front()));
        submissions_.pop_front();
    }
    return result;
}

void QUEUE_STATE::ThreadFunc() {
    layer_data::optional<CB_SUBMISSION> submission;

    auto is_query_updated_after = [this](const QueryObject &query_object) {
        auto guard = this->Lock();
        for (const auto &submission : this->submissions_) {
            for (const auto &next_cb_node : submission.cbs) {
                if (query_object.perf_pass != submission.perf_submit_pass) {
                    continue;
                }
                if (next_cb_node->UpdatesQuery(query_object)) {
                    return true;
                }
            }
        }
        return false;
    };

    // Roll this queue forward, one submission at a time.
    while ((submission = NextSubmission())) {
        submission->EndUse();
        for (auto &wait : submission->wait_semaphores) {
            wait.semaphore->Retire(this, wait.payload);
        }
        for (auto &cb_node : submission->cbs) {
            auto cb_guard = cb_node->WriteLock();
            for (auto *secondary_cmd_buffer : cb_node->linkedCommandBuffers) {
                auto secondary_guard = secondary_cmd_buffer->WriteLock();
                secondary_cmd_buffer->Retire(submission->perf_submit_pass, is_query_updated_after);
            }
            cb_node->Retire(submission->perf_submit_pass, is_query_updated_after);
        }
        for (auto &signal : submission->signal_semaphores) {
            signal.semaphore->Retire(this, signal.payload);
        }
        if (submission->fence) {
            submission->fence->Retire();
        }
        // wake up anyone waiting for this submission to be retired
        submission->completed.set_value();
    }
}

bool FENCE_STATE::EnqueueSignal(QUEUE_STATE *queue_state, uint64_t next_seq) {
    auto guard = WriteLock();
    if (scope_ != kSyncScopeInternal) {
        return true;
    }
    // Mark fence in use
    state_ = FENCE_INFLIGHT;
    queue_ = queue_state;
    seq_ = next_seq;
    return false;
}

// Called from a non-queue operation, such as vkWaitForFences()
void FENCE_STATE::NotifyAndWait() {
    std::shared_future<void> waiter;
    {
        // Hold the lock only while updating members, but not
        // while waiting
        auto guard = WriteLock();
        if (state_ == FENCE_INFLIGHT) {
            if (scope_ == kSyncScopeInternal && queue_) {
                queue_->Notify(seq_);
            } else {
                state_ = FENCE_RETIRED;
                completed_.set_value();
            }
            waiter = waiter_;
        }
    }
    if (waiter.valid()) {
        auto result = waiter.wait_until(GetCondWaitTimeout());
        if (result != std::future_status::ready) {
            dev_data_.LogError(Handle(), "UNASSIGNED-VkFence-state-timeout",
                               "Timeout waiting for fence state to update.");
        }
    }
}

// Retire from a queue operation
void FENCE_STATE::Retire() {
    auto guard = WriteLock();
    if (state_ == FENCE_INFLIGHT) {
        state_ = FENCE_RETIRED;
        completed_.set_value();
    }
}

void FENCE_STATE::Reset() {
    auto guard = WriteLock();
    queue_ = nullptr;
    seq_ = 0;
    // spec: If any member of pFences currently has its payload imported with temporary permanence,
    // that fence’s prior permanent payload is first restored. The remaining operations described
    // therefore operate on the restored payload.
    if (scope_ == kSyncScopeExternalTemporary) {
        scope_ = kSyncScopeInternal;
    }
    if (scope_ == kSyncScopeInternal) {
        state_ = FENCE_UNSIGNALED;
    }
    completed_ = std::promise<void>();
    waiter_ = std::shared_future<void>(completed_.get_future());
}

void FENCE_STATE::Import(VkExternalFenceHandleTypeFlagBits handle_type, VkFenceImportFlags flags) {
    auto guard = WriteLock();
    if (scope_ != kSyncScopeExternalPermanent) {
        if ((handle_type == VK_EXTERNAL_FENCE_HANDLE_TYPE_SYNC_FD_BIT || flags & VK_FENCE_IMPORT_TEMPORARY_BIT) &&
            scope_ == kSyncScopeInternal) {
            scope_ = kSyncScopeExternalTemporary;
        } else {
            scope_ = kSyncScopeExternalPermanent;
        }
    }
}

void FENCE_STATE::Export(VkExternalFenceHandleTypeFlagBits handle_type) {
    auto guard = WriteLock();
    if (handle_type != VK_EXTERNAL_FENCE_HANDLE_TYPE_SYNC_FD_BIT) {
        // Export with reference transference becomes external
        scope_ = kSyncScopeExternalPermanent;
    } else if (scope_ == kSyncScopeInternal) {
        // Export with copy transference has a side effect of resetting the fence
        state_ = FENCE_UNSIGNALED;
    }
}

bool SEMAPHORE_STATE::EnqueueSignal(QUEUE_STATE *queue, uint64_t queue_seq, uint64_t &payload) {
    auto guard = WriteLock();
    if (type == VK_SEMAPHORE_TYPE_BINARY) {
        payload = next_payload_++;
    }
    operations_.emplace(payload, SemOpEntry(kSignal, queue, queue_seq, payload));
    return false;
}

void SEMAPHORE_STATE::EnqueueWait(QUEUE_STATE *queue, uint64_t queue_seq, uint64_t &payload) {
    auto guard = WriteLock();
    switch (scope_) {
        case kSyncScopeExternalTemporary:
            scope_ = kSyncScopeInternal;
            break;
        default:
            break;
    }
    if (type == VK_SEMAPHORE_TYPE_BINARY) {
        payload = next_payload_++;
    } else if (payload <= completed_.payload) {
        return;
    }
    operations_.emplace(payload, SemOpEntry(kWait, queue, queue_seq, payload));
}

void SEMAPHORE_STATE::EnqueueAcquire() {
    auto guard = WriteLock();
    assert(type == VK_SEMAPHORE_TYPE_BINARY);
    auto payload = next_payload_++;
    operations_.emplace(payload, SemOpEntry(kBinaryAcquire, nullptr, 0, payload));
}

layer_data::optional<SemOp> SEMAPHORE_STATE::LastOp(std::function<bool(const SemOp &)> filter) const {
    auto guard = ReadLock();
    layer_data::optional<SemOp> result;

    for (auto pos = operations_.rbegin(); pos != operations_.rend(); ++pos) {
        if (!filter || filter(pos->second)) {
            result.emplace(pos->second);
            break;
        }
    }
    return result;
}

bool SEMAPHORE_STATE::CanBeSignaled() const {
    if (type == VK_SEMAPHORE_TYPE_TIMELINE) {
        return true;
    }
    auto guard = ReadLock();
    if (operations_.empty()) {
        return completed_.CanBeSignaled();
    }
    return operations_.rbegin()->second.CanBeSignaled();
}

bool SEMAPHORE_STATE::CanBeWaited() const {
    if (type == VK_SEMAPHORE_TYPE_TIMELINE) {
        return true;
    }
    auto guard = ReadLock();
    if (operations_.empty()) {
        return completed_.CanBeWaited();
    }
    return operations_.rbegin()->second.CanBeWaited();
}

VkQueue SEMAPHORE_STATE::AnotherQueueWaitsBinary(VkQueue queue) const {
    if (type == VK_SEMAPHORE_TYPE_TIMELINE) {
        return VK_NULL_HANDLE;
    }
    auto guard = ReadLock();

    for (auto pos = operations_.rbegin(); pos != operations_.rend(); ++pos) {
        if (pos->second.op_type == kWait && pos->second.queue->Queue() != queue) {
            return pos->second.queue->Queue();
        }
    }
    return VK_NULL_HANDLE;
}

void SEMAPHORE_STATE::Notify(uint64_t payload) {
    auto guard = WriteLock();
    for (const auto &entry : operations_) {
        if (entry.first > payload) {
            break;
        }
        const auto &op = entry.second;
        if (op.queue) {
            op.queue->Notify(op.seq);
        }
    }
}

void SEMAPHORE_STATE::Retire(QUEUE_STATE *current_queue, uint64_t payload) {
    auto guard = WriteLock();
    // This loop tells all queues that use the semaphore that something has happened.
    // Operations that are on the current queue (or no queue), we clean them up
    // immediately. For other queues, we must notify them and then wait for them to
    // update state. This rather scary process ensures that all queues update their
    // state in the order that operations completed on the GPU.
    while (!operations_.empty() && operations_.begin()->second.payload <= payload) {
        auto &op = operations_.begin()->second;
        if (op.queue) {
            op.queue->Notify(op.seq);
        }
        if (op.queue == nullptr || op.queue == current_queue) {
            // make sure completed doesn't go backwards for timeline semaphores
            assert(completed_.payload <= op.payload);
            completed_ = op;
            op.completed.set_value();
            operations_.erase(operations_.begin());
        } else if (op.waiter.valid()) {
            // the current op should get destroyed while we're waiting, so copy out the waiter.
            auto waiter = op.waiter;
            guard.unlock();
            auto result = waiter.wait_until(GetCondWaitTimeout());
            if (result != std::future_status::ready) {
                dev_data_.LogError(Handle(), "UNASSIGNED-VkSemaphore-state-timeout",
                                   "Timeout waiting for semaphore state to update. completed_.payload=%" PRIu64
                                   " wait_payload=%" PRIu64,
                                   completed_.payload, payload);
            }
            guard.lock();
        }
    }
}

std::shared_future<void> SEMAPHORE_STATE::Wait(uint64_t payload) {
    auto guard = ReadLock();
    if (payload <= completed_.payload) {
        std::promise<void> already_done;
        auto result = already_done.get_future();
        already_done.set_value();
        return result;
    }
    auto entry = operations_.find(payload);
    if (entry == operations_.end()) {
        // Handle timeline semaphore wait before signal
        assert(type == VK_SEMAPHORE_TYPE_TIMELINE);
        entry = operations_.emplace(payload, SemOpEntry(kWait, nullptr, 0, payload));
    }
    return entry->second.waiter;
}

void SEMAPHORE_STATE::RetireTimeline(uint64_t payload) {
    assert(type == VK_SEMAPHORE_TYPE_TIMELINE);
    // For vkSignalSemaphores(), the signal operation is not associated with a queue but
    // we need to complete anyway.
    EnqueueSignal(nullptr, 0, payload);
    Retire(nullptr, payload);
}

void SEMAPHORE_STATE::NotifyAndWait(uint64_t payload) {
    if (scope_ == kSyncScopeInternal) {
        auto timeout = GetCondWaitTimeout();
        auto result = std::future_status::timeout;
        auto waiter = Wait(payload);

        // Handle a race condition where a vkWaitSemaphores() or vkSemaphoreSemaphoreCounterValue()
        // call completes before we've processed signal operations have been added to the semaphore
        // by vkQueueSubmit(). If that happens we need to keep poking the operations_ list to tell
        // new operations that they're done as soon as they show up.
        do {
            Notify(payload);
            result = waiter.wait_for(std::chrono::milliseconds(10));
        } while (result != std::future_status::ready && std::chrono::steady_clock::now() < timeout);

        if (result != std::future_status::ready) {
            dev_data_.LogError(Handle(), "UNASSIGNED-VkSemaphore-state-timeout",
                    "Timeout waiting for timeline semaphore state to update. completed_.payload=%" PRIu64
                    " wait_payload=%" PRIu64,
                    completed_.payload, payload);
        }
    } else {
        // For external timeline semaphores we should bump the completed payload to whatever the driver
        // tells us.
        RetireTimeline(payload);
    }
}

void SEMAPHORE_STATE::Import(VkExternalSemaphoreHandleTypeFlagBits handle_type, VkSemaphoreImportFlags flags) {
    auto guard = WriteLock();
    if (scope_ != kSyncScopeExternalPermanent) {
        if ((handle_type == VK_EXTERNAL_SEMAPHORE_HANDLE_TYPE_SYNC_FD_BIT || flags & VK_SEMAPHORE_IMPORT_TEMPORARY_BIT) &&
            scope_ == kSyncScopeInternal) {
            scope_ = kSyncScopeExternalTemporary;
        } else {
            scope_ = kSyncScopeExternalPermanent;
        }
    }
}

void SEMAPHORE_STATE::Export(VkExternalSemaphoreHandleTypeFlagBits handle_type) {
    auto guard = WriteLock();
    if (handle_type != VK_EXTERNAL_SEMAPHORE_HANDLE_TYPE_SYNC_FD_BIT) {
        // Cannot track semaphore state once it is exported, except for Sync FD handle types which have copy transference
        scope_ = kSyncScopeExternalPermanent;
    }
}
