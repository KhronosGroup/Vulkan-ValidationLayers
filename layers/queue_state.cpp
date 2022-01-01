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

uint64_t QUEUE_STATE::Submit(CB_SUBMISSION &&submission) {
    for (auto &cb_node : submission.cbs) {
        for (auto *secondary_cmd_buffer : cb_node->linkedCommandBuffers) {
            secondary_cmd_buffer->IncrementResources();
        }
        cb_node->IncrementResources();
        // increment use count for all bound objects including secondary cbs
        cb_node->BeginUse();
        cb_node->Submit(submission.perf_submit_pass);
    }
    // Lock required for queue / semaphore operations, but not for command buffer
    // processing above.
    auto guard = WriteLock();
    const uint64_t next_seq = seq_ + submissions_.size() + 1;
    bool retire_early = false;
    for (auto &wait : submission.wait_semaphores) {
        wait.semaphore->EnqueueWait(this, next_seq, wait.payload);
        wait.semaphore->BeginUse();
    }

    for (auto &signal : submission.signal_semaphores) {
        if (signal.semaphore->EnqueueSignal(this, next_seq, signal.payload)) {
            retire_early = true;
        }
        signal.semaphore->BeginUse();
    }

    if (submission.fence) {
        if (submission.fence->EnqueueSignal(this, next_seq)) {
            retire_early = true;
        }
        submission.fence->BeginUse();
    }

    submissions_.emplace_back(std::move(submission));
    return retire_early ? next_seq : 0;
}

static void MergeResults(SEMAPHORE_STATE::RetireResult &results, const SEMAPHORE_STATE::RetireResult &sem_result) {
    for (auto &entry : sem_result) {
        auto &last_seq = results[entry.first];
        last_seq = std::max(last_seq, entry.second);
    }
}

layer_data::optional<CB_SUBMISSION> QUEUE_STATE::NextSubmission(uint64_t until_seq) {
    // Pop the next submission off of the queue so that Retire() doesn't need to worry
    // about locking.
    auto guard = WriteLock();
    layer_data::optional<CB_SUBMISSION> result;
    if (seq_ < until_seq && !submissions_.empty()) {
        result.emplace(std::move(submissions_.front()));
        submissions_.pop_front();
        seq_++;
    }
    return result;
}

void QUEUE_STATE::Retire(uint64_t until_seq) {
    SEMAPHORE_STATE::RetireResult other_queue_seqs;

    layer_data::optional<CB_SUBMISSION> submission;

    // Roll this queue forward, one submission at a time.
    while ((submission = NextSubmission(until_seq))) {
        for (auto &wait : submission->wait_semaphores) {
            auto result = wait.semaphore->Retire(this, wait.payload);
            MergeResults(other_queue_seqs, result);
            wait.semaphore->EndUse();
        }
        for (auto &signal : submission->signal_semaphores) {
            auto result = signal.semaphore->Retire(this, signal.payload);
            // in the case of timeline semaphores, signaling at payload == N
            // may unblock waiting queues for payload <= N so we need to
            // process them
            MergeResults(other_queue_seqs, result);
            signal.semaphore->EndUse();
        }
        // Handle updates to how far the current queue has progressed
        // without going recursive when we call Retire on other_queue_seqs
        // below.
        auto self_update = other_queue_seqs.find(this);
        if (self_update != other_queue_seqs.end()) {
            until_seq = std::max(until_seq, self_update->second);
            other_queue_seqs.erase(self_update);
        }

        for (auto &cb_node : submission->cbs) {
            for (auto *secondary_cmd_buffer : cb_node->linkedCommandBuffers) {
                secondary_cmd_buffer->Retire(submission->perf_submit_pass);
            }
            cb_node->Retire(submission->perf_submit_pass);
            cb_node->EndUse();
        }

        if (submission->fence) {
            submission->fence->Retire(false);
            submission->fence->EndUse();
        }
    }

    // Roll other queues forward to the highest seq we saw a wait for
    for (const auto &qs : other_queue_seqs) {
        qs.first->Retire(qs.second);
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

void FENCE_STATE::Retire(bool notify_queue) {
    QUEUE_STATE *q = nullptr;
    uint64_t seq = 0;
    {
        // Hold the lock only while updating members, but not
        // while calling QUEUE_STATE::Retire()
        auto guard = WriteLock();
        if (scope_ == kSyncScopeInternal) {
            q = queue_;
            seq = seq_;
        }
        queue_ = nullptr;
        seq_ = 0;
        state_ = FENCE_RETIRED;
    }
    if (q && notify_queue) {
        q->Retire(seq);
    }
}

void FENCE_STATE::Reset() {
    auto guard = WriteLock();
    if (scope_ == kSyncScopeInternal) {
        state_ = FENCE_UNSIGNALED;
    } else if (scope_ == kSyncScopeExternalTemporary) {
        scope_ = kSyncScopeInternal;
    }
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
    if (scope_ != kSyncScopeInternal) {
        return true;  // retire early
    }
    if (type == VK_SEMAPHORE_TYPE_BINARY) {
        payload = next_payload_++;
    }
    operations_.emplace(SemOp{kSignal, queue, queue_seq, payload});
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
    }
    operations_.emplace(SemOp{kWait, queue, queue_seq, payload});
}

void SEMAPHORE_STATE::EnqueueAcquire() {
    auto guard = WriteLock();
    assert(type == VK_SEMAPHORE_TYPE_BINARY);
    operations_.emplace(SemOp{kBinaryAcquire, nullptr, 0, next_payload_++});
}

void SEMAPHORE_STATE::EnqueuePresent(QUEUE_STATE *queue) {
    auto guard = WriteLock();
    assert(type == VK_SEMAPHORE_TYPE_BINARY);
    operations_.emplace(SemOp{kBinaryPresent, queue, 0, next_payload_++});
}

layer_data::optional<SemOp> SEMAPHORE_STATE::LastOp(std::function<bool(const SemOp &)> filter) const {
    auto guard = ReadLock();
    layer_data::optional<SemOp> result;

    for (auto pos = operations_.rbegin(); pos != operations_.rend(); ++pos) {
        if (!filter || filter(*pos)) {
            result.emplace(*pos);
            break;
        }
    }
    return result;
}

bool SEMAPHORE_STATE::CanBeSignaled() const {
    if (type == VK_SEMAPHORE_TYPE_TIMELINE) {
        return true;
    }
    // both LastOp() and Completed() lock, so no locking needed in this method.
    auto op = LastOp();
    if (op) {
        return op->CanBeSignaled();
    }
    auto comp = Completed();
    return comp.CanBeSignaled();
}

bool SEMAPHORE_STATE::CanBeWaited() const {
    if (type == VK_SEMAPHORE_TYPE_TIMELINE) {
        return true;
    }
    // both LastOp() and Completed() lock, so no locking needed in this method.
    auto op = LastOp();
    if (op) {
        return op->op_type == kSignal || op->op_type == kBinaryAcquire;
    }
    auto comp = Completed();
    return comp.op_type == kSignal || comp.op_type == kBinaryAcquire;
}

SEMAPHORE_STATE::RetireResult SEMAPHORE_STATE::Retire(QUEUE_STATE *queue, uint64_t payload) {
    auto guard = WriteLock();
    RetireResult result;

    while (!operations_.empty() && operations_.begin()->payload <= payload) {
        completed_ = *operations_.begin();
        operations_.erase(operations_.begin());
        // Note: even though presentation is directed to a queue, there is no direct ordering between QP and subsequent work,
        // so QP (and its semaphore waits) /never/ participate in any completion proof. Likewise, Acquire is not associated
        // with a queue.
        if (completed_.op_type != kBinaryAcquire && completed_.op_type != kBinaryPresent) {
            auto &last_seq = result[completed_.queue];
            last_seq = std::max(last_seq, completed_.seq);
        }
    }
    return result;
}

void SEMAPHORE_STATE::RetireTimeline(uint64_t payload) {
    if (type == VK_SEMAPHORE_TYPE_TIMELINE) {
        auto results = Retire(nullptr, payload);
        for (auto &entry : results) {
            entry.first->Retire(entry.second);
        }
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
