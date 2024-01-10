/* Copyright (c) 2015-2024 The Khronos Group Inc.
 * Copyright (c) 2015-2024 Valve Corporation
 * Copyright (c) 2015-2024 LunarG, Inc.
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
#pragma once
#include "state_tracker/state_object.h"
#include "state_tracker/fence_state.h"
#include "state_tracker/semaphore_state.h"
#include <condition_variable>
#include <deque>
#include <future>
#include <thread>
#include <vector>
#include "error_message/error_location.h"

class ValidationStateTracker;

namespace vvl {

class CommandBuffer;
class Queue;

struct QueueSubmission {
    struct SemaphoreInfo {
        SemaphoreInfo(std::shared_ptr<Semaphore> &&sem, uint64_t pl) : semaphore(std::move(sem)), payload(pl) {}
        std::shared_ptr<Semaphore> semaphore;
        uint64_t payload{0};
    };
    QueueSubmission(const Location &loc_) : loc(loc_), completed(), waiter(completed.get_future()) {}

    std::vector<std::shared_ptr<vvl::CommandBuffer>> cbs;
    std::vector<SemaphoreInfo> wait_semaphores;
    std::vector<SemaphoreInfo> signal_semaphores;
    std::shared_ptr<Fence> fence;
    LocationCapture loc;
    uint64_t seq{0};
    uint32_t perf_submit_pass{0};
    std::promise<void> completed;
    std::shared_future<void> waiter;

    void AddCommandBuffer(std::shared_ptr<vvl::CommandBuffer> &&cb_state) { cbs.emplace_back(std::move(cb_state)); }

    void AddSignalSemaphore(std::shared_ptr<Semaphore> &&semaphore_state, uint64_t value) {
        signal_semaphores.emplace_back(std::move(semaphore_state), value);
    }

    void AddWaitSemaphore(std::shared_ptr<Semaphore> &&semaphore_state, uint64_t value) {
        wait_semaphores.emplace_back(std::move(semaphore_state), value);
    }

    void AddFence(std::shared_ptr<Fence> &&fence_state) { fence = std::move(fence_state); }

    void EndUse();
    void BeginUse();
};

// This timeout is for all queue threads to update their state after we know
// (via being in a PostRecord call) that a fence, semaphore or wait for idle has
// completed. Hitting it is almost a certainly a bug in this code.
static inline std::chrono::time_point<std::chrono::steady_clock> GetCondWaitTimeout() {
    return std::chrono::steady_clock::now() + std::chrono::seconds(10);
}

class Queue: public StateObject {
  public:
    Queue(ValidationStateTracker &dev_data, VkQueue q, uint32_t index, VkDeviceQueueCreateFlags flags,
                const VkQueueFamilyProperties &queueFamilyProperties)
        : StateObject(q, kVulkanObjectTypeQueue),
          queueFamilyIndex(index),
          flags(flags),
          queueFamilyProperties(queueFamilyProperties),
          dev_data_(dev_data) {}

    ~Queue() { Destroy(); }
    void Destroy() override;

    VkQueue VkHandle() const { return handle_.Cast<VkQueue>(); }

    uint64_t Submit(QueueSubmission &&submission);

    // Tell the queue thread that submissions up to the submission with sequence number until_seq have finished
    uint64_t Notify(uint64_t until_seq = kU64Max);

    // Tell the queue and then wait for it to finish updating its state.
    // UINT64_MAX means to finish all submissions.
    void NotifyAndWait(const Location &loc, uint64_t until_seq = kU64Max);
    std::shared_future<void> Wait(uint64_t until_seq = kU64Max);

    const uint32_t queueFamilyIndex;
    const VkDeviceQueueCreateFlags flags;
    const VkQueueFamilyProperties queueFamilyProperties;

  private:
    using LockGuard = std::unique_lock<std::mutex>;
    void ThreadFunc();
    QueueSubmission *NextSubmission();
    LockGuard Lock() const { return LockGuard(lock_); }

    ValidationStateTracker &dev_data_;

    // state related to submitting to the queue, all data members must
    // be accessed with lock_ held
    std::unique_ptr<std::thread> thread_;
    std::deque<QueueSubmission> submissions_;
    std::atomic<uint64_t> seq_{0};
    uint64_t request_seq_{0};
    bool exit_thread_{false};
    mutable std::mutex lock_;
    // condition to wake up the queue's thread
    std::condition_variable cond_;
};
} // namespace vvl
