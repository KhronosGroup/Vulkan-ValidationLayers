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
#include "state_tracker/queue_state.h"
#include "state_tracker/cmd_buffer_state.h"

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
    for (auto &cb_state : cbs) {
        cb_state->BeginUse();
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
    for (auto &cb_state : cbs) {
        cb_state->EndUse();
    }
    for (auto &signal : signal_semaphores) {
        signal.semaphore->EndUse();
    }
    if (fence) {
        fence->EndUse();
    }
}

uint64_t QUEUE_STATE::Submit(CB_SUBMISSION &&submission) {
    for (auto &cb_state : submission.cbs) {
        auto cb_guard = cb_state->WriteLock();
        for (auto *secondary_cmd_buffer : cb_state->linkedCommandBuffers) {
            auto secondary_guard = secondary_cmd_buffer->WriteLock();
            secondary_cmd_buffer->IncrementResources();
        }
        cb_state->IncrementResources();
        cb_state->Submit(submission.perf_submit_pass);
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
        signal.semaphore->EnqueueSignal(this, submission.seq, signal.payload);
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
            thread_ = std::make_unique<std::thread>(&QUEUE_STATE::ThreadFunc, this);
        }
    }
    return retire_early ? submission.seq : 0;
}

std::shared_future<void> QUEUE_STATE::Wait(uint64_t until_seq) {
    auto guard = Lock();
    if (until_seq == vvl::kU64Max) {
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
                           "Timeout waiting for queue state to update. This is most likely a validation bug."
                           " seq=%" PRIu64 " until=%" PRIu64,
                           seq_.load(), until_seq);
    }
}

uint64_t QUEUE_STATE::Notify(uint64_t until_seq) {
    auto guard = Lock();
    if (until_seq == vvl::kU64Max) {
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

CB_SUBMISSION *QUEUE_STATE::NextSubmission() {
    CB_SUBMISSION *result = nullptr;
    // Find if the next submission is ready so that the thread function doesn't need to worry
    // about locking.
    auto guard = Lock();
    while (!exit_thread_ && (submissions_.empty() || request_seq_ < submissions_.front().seq)) {
        // The queue thread must wait forever if nothing is happening, until we tell it to exit
        cond_.wait(guard);
    }
    if (!exit_thread_) {
        result = &submissions_.front();
        // NOTE: the submission must remain on the dequeue until we're done processing it so that
        // anyone waiting for it can find the correct waiter
    }
    return result;
}

void QUEUE_STATE::ThreadFunc() {
    CB_SUBMISSION *submission = nullptr;

    auto is_query_updated_after = [this](const QueryObject &query_object) {
        auto guard = this->Lock();
        bool first = true;
        for (const auto &submission : this->submissions_) {
            // The current submission is still on the deque, so skip it
            if (first) {
                first = false;
                continue;
            }
            for (const auto &next_cb_state : submission.cbs) {
                if (query_object.perf_pass != submission.perf_submit_pass) {
                    continue;
                }
                if (next_cb_state->UpdatesQuery(query_object)) {
                    return true;
                }
            }
        }
        return false;
    };

    // Roll this queue forward, one submission at a time.
    while (true) {
        submission = NextSubmission();
        if (submission == nullptr) {
            break;
        }

        submission->EndUse();
        for (auto &wait : submission->wait_semaphores) {
            wait.semaphore->Retire(this, wait.payload);
        }
        for (auto &cb_state : submission->cbs) {
            auto cb_guard = cb_state->WriteLock();
            for (auto *secondary_cmd_buffer : cb_state->linkedCommandBuffers) {
                auto secondary_guard = secondary_cmd_buffer->WriteLock();
                secondary_cmd_buffer->Retire(submission->perf_submit_pass, is_query_updated_after);
            }
            cb_state->Retire(submission->perf_submit_pass, is_query_updated_after);
        }
        for (auto &signal : submission->signal_semaphores) {
            signal.semaphore->Retire(this, signal.payload);
        }
        if (submission->fence) {
            submission->fence->Retire();
        }
        // wake up anyone waiting for this submission to be retired
        {
            auto guard = Lock();
            submission->completed.set_value();
            submissions_.pop_front();
        }
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
            if (queue_) {
                queue_->Notify(seq_);
                waiter = waiter_;
            } else {
                state_ = FENCE_RETIRED;
                completed_.set_value();
                queue_ = nullptr;
                seq_ = 0;
            }
        }
    }
    if (waiter.valid()) {
        auto result = waiter.wait_until(GetCondWaitTimeout());
        if (result != std::future_status::ready) {
            dev_data_.LogError(Handle(), "UNASSIGNED-VkFence-state-timeout",
                               "Timeout waiting for fence state to update. This is most likely a validation bug.");
        }
    }
}

// Retire from a queue operation
void FENCE_STATE::Retire() {
    auto guard = WriteLock();
    if (state_ == FENCE_INFLIGHT) {
        state_ = FENCE_RETIRED;
        completed_.set_value();
        queue_ = nullptr;
        seq_ = 0;
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
    state_ = FENCE_UNSIGNALED;
    completed_ = std::promise<void>();
    waiter_ = std::shared_future<void>(completed_.get_future());
}

void FENCE_STATE::Import(VkExternalFenceHandleTypeFlagBits handle_type, VkFenceImportFlags flags) {
    auto guard = WriteLock();
    if (scope_ != kSyncScopeExternalPermanent) {
        if (handle_type != VK_EXTERNAL_FENCE_HANDLE_TYPE_SYNC_FD_BIT && (flags & VK_FENCE_IMPORT_TEMPORARY_BIT) == 0) {
            scope_ = kSyncScopeExternalPermanent;
        } else if (scope_ == kSyncScopeInternal) {
            scope_ = kSyncScopeExternalTemporary;
        }
    }
}

void FENCE_STATE::Export(VkExternalFenceHandleTypeFlagBits handle_type) {
    auto guard = WriteLock();
    if (handle_type != VK_EXTERNAL_FENCE_HANDLE_TYPE_SYNC_FD_BIT) {
        // Export with reference transference becomes external
        scope_ = kSyncScopeExternalPermanent;
    } else {
        // Export with copy transference has a side effect of resetting the fence
        if (scope_ == kSyncScopeExternalTemporary) {
            scope_ = kSyncScopeInternal;
        }
        state_ = FENCE_UNSIGNALED;
        completed_ = std::promise<void>();
        waiter_ = std::shared_future<void>(completed_.get_future());
    }
}

void SEMAPHORE_STATE::EnqueueSignal(QUEUE_STATE *queue, uint64_t queue_seq, uint64_t &payload) {
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

void SEMAPHORE_STATE::EnqueueWait(QUEUE_STATE *queue, uint64_t queue_seq, uint64_t &payload) {
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

void SEMAPHORE_STATE::EnqueueAcquire(vvl::Func command) {
    auto guard = WriteLock();
    assert(type == VK_SEMAPHORE_TYPE_BINARY);
    auto payload = next_payload_++;
    SemOp acquire(kBinaryAcquire, nullptr, 0, payload, command);
    timeline_.emplace(payload, acquire);
}

std::optional<SemOp> SEMAPHORE_STATE::LastOp(const std::function<bool(const SemOp &, bool)> &filter) const {
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

bool SEMAPHORE_STATE::CanBeSignaled() const {
    if (type == VK_SEMAPHORE_TYPE_TIMELINE) {
        return true;
    }
    auto guard = ReadLock();
    if (timeline_.empty()) {
        return completed_.CanBeSignaled();
    }
    return timeline_.rbegin()->second.HasWaiters();
}

bool SEMAPHORE_STATE::CanBeWaited() const {
    if (type == VK_SEMAPHORE_TYPE_TIMELINE) {
        return true;
    }
    auto guard = ReadLock();
    if (timeline_.empty()) {
        return completed_.CanBeWaited();
    }
    return !timeline_.rbegin()->second.HasWaiters();
}

void SEMAPHORE_STATE::SemOp::Notify() const {
    if (queue) {
        queue->Notify(seq);
    }
}

void SEMAPHORE_STATE::TimePoint::Notify() const {
    if (signal_op) {
        signal_op->Notify();
    }
    for (auto &wait : wait_ops) {
        assert(wait.payload == wait_ops[0].payload);
        wait.Notify();
    }
}

void SEMAPHORE_STATE::Notify(uint64_t payload) {
    auto guard = ReadLock();
    auto pos = timeline_.find(payload);
    if (pos != timeline_.end()) {
        pos->second.Notify();
    }
}

void SEMAPHORE_STATE::Retire(QUEUE_STATE *current_queue, uint64_t payload) {
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
        if (scope_ != kSyncScopeInternal) {
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
        if (scope_ == kSyncScopeExternalTemporary) {
            scope_ = kSyncScopeInternal;
        }
    } else {
        // Wait for some other queue or a host operation to retire
        assert(timepoint.waiter.valid());
        // the current timepoint should get destroyed while we're waiting, so copy out the waiter.
        auto waiter = timepoint.waiter;
        guard.unlock();
        auto result = waiter.wait_until(GetCondWaitTimeout());
        if (result != std::future_status::ready) {
            dev_data_.LogError(Handle(), "UNASSIGNED-VkSemaphore-state-timeout",
                               "Timeout waiting for timeline semaphore state to update. This is most likely a validation bug."
                               " completed_.payload=%" PRIu64 " wait_payload=%" PRIu64,
                               completed_.payload, payload);
        }
        guard.lock();
    }
}

std::shared_future<void> SEMAPHORE_STATE::Wait(uint64_t payload) {
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

void SEMAPHORE_STATE::NotifyAndWait(uint64_t payload) {
    if (scope_ == kSyncScopeInternal) {
        Notify(payload);
        auto waiter = Wait(payload);
        dev_data_.BeginBlockingOperation();
        auto result = waiter.wait_until(GetCondWaitTimeout());
        dev_data_.EndBlockingOperation();
        if (result != std::future_status::ready) {
            dev_data_.LogError(Handle(), "UNASSIGNED-VkSemaphore-state-timeout",
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
        Retire(nullptr, payload);
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
    if (handle_type != VK_EXTERNAL_SEMAPHORE_HANDLE_TYPE_SYNC_FD_BIT) {
        // Cannot track semaphore state once it is exported, except for Sync FD handle types which have copy transference
        auto guard = WriteLock();
        scope_ = kSyncScopeExternalPermanent;
    } else {
        // Exporting a semaphore payload to a handle with copy transference has the same side effects on the source semaphore's
        // payload as executing a semaphore wait operation
        auto filter = [](const SEMAPHORE_STATE::SemOp &op, bool is_pending) { return is_pending && op.CanBeWaited(); };
        auto last_op = LastOp(filter);
        if (last_op) {
            EnqueueWait(last_op->queue, last_op->seq, last_op->payload);
        }
    }
}
