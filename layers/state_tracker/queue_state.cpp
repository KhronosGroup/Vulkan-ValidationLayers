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
#include "state_tracker/queue_state.h"
#include "state_tracker/cmd_buffer_state.h"

void vvl::QueueSubmission::BeginUse() {
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

void vvl::QueueSubmission::EndUse() {
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

vvl::PreSubmitResult vvl::Queue::PreSubmit(std::vector<vvl::QueueSubmission> &&submissions) {
    if (!submissions.empty()) {
        submissions.back().end_batch = true;
    }
    PreSubmitResult result;
    for (auto &submission : submissions) {
        for (auto &cb_state : submission.cbs) {
            auto cb_guard = cb_state->WriteLock();
            for (auto *secondary_cmd_buffer : cb_state->linkedCommandBuffers) {
                auto secondary_guard = secondary_cmd_buffer->WriteLock();
                secondary_cmd_buffer->IncrementResources();
            }
            cb_state->IncrementResources();
            cb_state->Submit(VkHandle(), submission.perf_submit_pass, submission.loc.Get());
        }
        // seq_ is atomic so we don't need a lock until updating the deque below.
        // Note that this relies on the external synchonization requirements for the
        // VkQueue
        submission.seq = ++seq_;
        submission.BeginUse();
        for (auto &wait : submission.wait_semaphores) {
            wait.semaphore->EnqueueWait(SubmissionReference(this, submission.seq), wait.payload);
        }

        for (auto &signal : submission.signal_semaphores) {
            signal.semaphore->EnqueueSignal(SubmissionReference(this, submission.seq), signal.payload);
        }

        if (submission.fence) {
            if (submission.fence->EnqueueSignal(this, submission.seq)) {
                result.has_external_fence = true;
                result.submission_with_external_fence_seq = submission.seq;
            }
        }
        {
            auto guard = Lock();
            submissions_.emplace_back(std::move(submission));
            if (!thread_) {
                thread_ = std::make_unique<std::thread>(&Queue::ThreadFunc, this);
            }
        }
    }
    return result;
}

void vvl::Queue::Notify(uint64_t until_seq) {
    auto guard = Lock();
    if (until_seq == kU64Max) {
        until_seq = seq_.load();
    }
    if (request_seq_ < until_seq) {
        request_seq_ = until_seq;
    }
    cond_.notify_one();
}

void vvl::Queue::Wait(const Location &loc, uint64_t until_seq) {
    std::shared_future<void> waiter;
    {
        auto guard = Lock();
        if (until_seq == kU64Max) {
            until_seq = seq_.load();
        }
        if (submissions_.empty() || until_seq < submissions_.begin()->seq) {
            return;
        }
        uint64_t index = until_seq - submissions_.begin()->seq;
        assert(index < submissions_.size());
        waiter = submissions_[static_cast<size_t>(index)].waiter;
    }
    auto wait_status = waiter.wait_until(GetCondWaitTimeout());
    if (wait_status != std::future_status::ready) {
        dev_data_.LogError(
            "INTERNAL-ERROR-VkQueue-state-timeout", Handle(), loc,
            "The Validation Layers hit a timeout waiting for queue state to update (this is most likely a validation bug)."
            " seq=%" PRIu64 " until=%" PRIu64,
            seq_.load(), until_seq);
    }
}

void vvl::Queue::NotifyAndWait(const Location &loc, uint64_t until_seq) {
    Notify(until_seq);
    Wait(loc, until_seq);
}

void vvl::Queue::Destroy() {
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
    StateObject::Destroy();
}

void vvl::Queue::PostSubmit() {
    auto guard = Lock();
    if (!submissions_.empty()) {
        PostSubmit(submissions_.back());
    }
}

vvl::QueueSubmission *vvl::Queue::NextSubmission() {
    QueueSubmission *result = nullptr;
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

void vvl::Queue::Retire(QueueSubmission &submission) {
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
    submission.EndUse();
    for (auto &wait : submission.wait_semaphores) {
        wait.semaphore->RetireWait(this, wait.payload, submission.loc.Get(), true);
    }
    for (auto &cb_state : submission.cbs) {
        auto cb_guard = cb_state->WriteLock();
        for (auto *secondary_cmd_buffer : cb_state->linkedCommandBuffers) {
            auto secondary_guard = secondary_cmd_buffer->WriteLock();
            secondary_cmd_buffer->Retire(submission.perf_submit_pass, is_query_updated_after);
        }
        cb_state->Retire(submission.perf_submit_pass, is_query_updated_after);
    }
    for (auto &signal : submission.signal_semaphores) {
        signal.semaphore->RetireSignal(signal.payload);
    }
    if (submission.fence) {
        submission.fence->Retire();
    }
}

void vvl::Queue::ThreadFunc() {
    QueueSubmission *submission = nullptr;

    // Roll this queue forward, one submission at a time.
    while (true) {
        submission = NextSubmission();
        if (submission == nullptr) {
            break;
        }
        Retire(*submission);
        // wake up anyone waiting for this submission to be retired
        {
            std::promise<void> completed;
            {
                auto guard = Lock();
                completed = std::move(submission->completed);
                submissions_.pop_front();
            }
            completed.set_value();
        }
    }
}
