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
#pragma once
#include "state_tracker/base_node.h"
#include <condition_variable>
#include <deque>
#include <future>
#include <set>
#include <thread>
#include <vector>
#include "vk_layer_utils.h"

class CMD_BUFFER_STATE;
class QUEUE_STATE;
class ValidationStateTracker;

enum SyncScope {
    kSyncScopeInternal,
    kSyncScopeExternalTemporary,
    kSyncScopeExternalPermanent,
};

enum FENCE_STATUS { FENCE_UNSIGNALED, FENCE_INFLIGHT, FENCE_RETIRED };

class FENCE_STATE : public REFCOUNTED_NODE {
  public:
    // Default constructor
    FENCE_STATE(ValidationStateTracker &dev, VkFence f, const VkFenceCreateInfo *pCreateInfo)
        : REFCOUNTED_NODE(f, kVulkanObjectTypeFence),
          flags(pCreateInfo->flags),
          exportHandleTypes(GetExportHandleTypes(pCreateInfo)),
          state_((pCreateInfo->flags & VK_FENCE_CREATE_SIGNALED_BIT) ? FENCE_RETIRED : FENCE_UNSIGNALED),
          completed_(),
          waiter_(completed_.get_future()),
          dev_data_(dev) {}

    VkFence fence() const { return handle_.Cast<VkFence>(); }

    bool EnqueueSignal(QUEUE_STATE *queue_state, uint64_t next_seq);

    // Notify the queue that the fence has signalled and then wait for the queue
    // to update state.
    void NotifyAndWait();

    // Update state of the completed fence. This should only be called by QUEUE_STATE.
    void Retire();

    void Reset();

    void Import(VkExternalFenceHandleTypeFlagBits handle_type, VkFenceImportFlags flags);

    void Export(VkExternalFenceHandleTypeFlagBits handle_type);

    const VkFenceCreateFlags flags;
    const VkExternalFenceHandleTypeFlags exportHandleTypes;

    SyncScope Scope() const { return scope_; }
    FENCE_STATUS State() const { return state_; }

  private:
    static VkExternalFenceHandleTypeFlags GetExportHandleTypes(const VkFenceCreateInfo *info) {
        auto export_info = LvlFindInChain<VkExportFenceCreateInfo>(info->pNext);
        return export_info ? export_info->handleTypes : 0;
    }
    ReadLockGuard ReadLock() const { return ReadLockGuard(lock_); }
    WriteLockGuard WriteLock() { return WriteLockGuard(lock_); }

    QUEUE_STATE *queue_{nullptr};
    uint64_t seq_{0};
    FENCE_STATUS state_;
    SyncScope scope_{kSyncScopeInternal};
    mutable std::shared_mutex lock_;
    std::promise<void> completed_;
    std::shared_future<void> waiter_;
    ValidationStateTracker &dev_data_;
};

class SEMAPHORE_STATE : public REFCOUNTED_NODE {
  public:
    // possible payload values for binary semaphore
    enum OpType {
        kNone,
        kWait,
        kSignal,
        kBinaryAcquire,
    };
    static inline const char *OpTypeName(OpType t) {
        switch (t) {
            case kWait:
                return "wait";
            case kSignal:
                return "signal";
            case kBinaryAcquire:
                return "acquire";
            case kNone:
            default:
                return "NONE";
        }
    }

    struct SemOp {
        SemOp(OpType ot, QUEUE_STATE *q, uint64_t queue_seq, uint64_t timeline_payload)
            : op_type(ot), queue(q), seq(queue_seq), payload(timeline_payload) {}

        OpType op_type;
        QUEUE_STATE *queue;
        uint64_t seq;
        uint64_t payload;

        bool operator<(const SemOp &rhs) const { return payload < rhs.payload; }

        bool IsWait() const { return op_type == kWait; }
        bool IsSignal() const { return op_type == kSignal; }
        bool IsAcquire() const { return op_type == kBinaryAcquire; }

        // NOTE: Present semaphores are waited on by the implementation, not queue operations. We do not yet
        // have a good way to figure out when this wait completes, so we must assume they are safe to re-use
        bool CanBeSignaled() const { return op_type == kNone || op_type == kWait; }
        bool CanBeWaited() const { return op_type == kSignal || op_type == kBinaryAcquire; }

        void Notify() const;
    };

    struct TimePoint {
        TimePoint(SemOp &op) : signal_op(), completed(), waiter(completed.get_future()) {
            if (op.op_type == kWait) {
                wait_ops.emplace(op);
            } else {
                signal_op.emplace(op);
            }
        }
        std::optional<SemOp> signal_op;
        std::set<SemOp> wait_ops;
        std::promise<void> completed;
        std::shared_future<void> waiter;

        bool HasSignaler() const { return signal_op.has_value(); }
        bool HasWaiters() const { return !wait_ops.empty(); }
        void Notify() const;
    };

#ifdef VK_USE_PLATFORM_METAL_EXT
    static bool GetMetalExport(const VkSemaphoreCreateInfo *info) {
        bool retval = false;
        auto export_metal_object_info = LvlFindInChain<VkExportMetalObjectCreateInfoEXT>(info->pNext);
        while (export_metal_object_info) {
            if (export_metal_object_info->exportObjectType == VK_EXPORT_METAL_OBJECT_TYPE_METAL_SHARED_EVENT_BIT_EXT) {
                retval = true;
                break;
            }
            export_metal_object_info = LvlFindInChain<VkExportMetalObjectCreateInfoEXT>(export_metal_object_info->pNext);
        }
        return retval;
    }
#endif  // VK_USE_PLATFORM_METAL_EXT
    VkExternalSemaphoreHandleTypeFlags GetExportHandleTypes(const VkSemaphoreCreateInfo *pCreateInfo) {
        auto export_info = LvlFindInChain<VkExportSemaphoreCreateInfo>(pCreateInfo->pNext);
        return export_info ? export_info->handleTypes : 0;
    }

    SEMAPHORE_STATE(ValidationStateTracker &dev, VkSemaphore sem, const VkSemaphoreCreateInfo *pCreateInfo)
        : SEMAPHORE_STATE(dev, sem, LvlFindInChain<VkSemaphoreTypeCreateInfo>(pCreateInfo->pNext), pCreateInfo) {}

    SEMAPHORE_STATE(ValidationStateTracker &dev, VkSemaphore sem, const VkSemaphoreTypeCreateInfo *type_create_info,
                    const VkSemaphoreCreateInfo *pCreateInfo)
        : REFCOUNTED_NODE(sem, kVulkanObjectTypeSemaphore),
#ifdef VK_USE_PLATFORM_METAL_EXT
          metal_semaphore_export(GetMetalExport(pCreateInfo)),
#endif  // VK_USE_PLATFORM_METAL_EXT
          type(type_create_info ? type_create_info->semaphoreType : VK_SEMAPHORE_TYPE_BINARY),
          exportHandleTypes(GetExportHandleTypes(pCreateInfo)),
          completed_{type == VK_SEMAPHORE_TYPE_TIMELINE ? kSignal : kNone, nullptr, 0,
                     type_create_info ? type_create_info->initialValue : 0},
          next_payload_(completed_.payload + 1),
          dev_data_(dev) {
    }

    VkSemaphore semaphore() const { return handle_.Cast<VkSemaphore>(); }
    SyncScope Scope() const {
        auto guard = ReadLock();
        return scope_;
    }
    // This is the most recently completed operation. It is returned by value so that the caller
    // has a correct copy even if something else is completing on this queue in a different thread.
    SemOp Completed() const {
        auto guard = ReadLock();
        return completed_;
    }

    // Enqueue a semaphore operation. For binary semaphores, the payload value is generated and
    // returned, so that every semaphore operation has a unique value.
    void EnqueueSignal(QUEUE_STATE *queue, uint64_t queue_seq, uint64_t &payload);
    void EnqueueWait(QUEUE_STATE *queue, uint64_t queue_seq, uint64_t &payload);

    // Binary only special cases enqueue functions
    void EnqueueAcquire();

    // Signal queue(s) that need to retire because a wait on this payload has finished
    void Notify(uint64_t payload);

    std::shared_future<void> Wait(uint64_t payload);

    // Helper for retiring timeline semaphores and then retiring all queues using the semaphore
    void NotifyAndWait(uint64_t payload);

    // Remove completed operations and signal any waiters. This should only be called by QUEUE_STATE
    void Retire(QUEUE_STATE *current_queue, uint64_t payload);

    // look for most recent / highest payload operation that matches
    std::optional<SemOp> LastOp(const std::function<bool(const SemOp &, bool is_pending)> &filter = nullptr) const;

    bool CanBeSignaled() const;
    bool CanBeWaited() const;
    bool HasPendingOps() const {
        auto guard = ReadLock();
        return !timeline_.empty();
    }

    void Import(VkExternalSemaphoreHandleTypeFlagBits handle_type, VkSemaphoreImportFlags flags);
    void Export(VkExternalSemaphoreHandleTypeFlagBits handle_type);
#ifdef VK_USE_PLATFORM_METAL_EXT
    const bool metal_semaphore_export;
#endif  // VK_USE_PLATFORM_METAL_EXT
    const VkSemaphoreType type;
    const VkExternalSemaphoreHandleTypeFlags exportHandleTypes;

  private:
    ReadLockGuard ReadLock() const { return ReadLockGuard(lock_); }
    WriteLockGuard WriteLock() { return WriteLockGuard(lock_); }

    SyncScope scope_{kSyncScopeInternal};
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

struct CB_SUBMISSION {
    struct SemaphoreInfo {
        SemaphoreInfo(std::shared_ptr<SEMAPHORE_STATE> &&sem, uint64_t pl) : semaphore(std::move(sem)), payload(pl) {}
        std::shared_ptr<SEMAPHORE_STATE> semaphore;
        uint64_t payload{0};
    };
    CB_SUBMISSION() : completed(), waiter(completed.get_future()) {}

    std::vector<std::shared_ptr<CMD_BUFFER_STATE>> cbs;
    std::vector<SemaphoreInfo> wait_semaphores;
    std::vector<SemaphoreInfo> signal_semaphores;
    std::shared_ptr<FENCE_STATE> fence;
    uint64_t seq{0};
    uint32_t perf_submit_pass{0};
    std::promise<void> completed;
    std::shared_future<void> waiter;

    void AddCommandBuffer(std::shared_ptr<CMD_BUFFER_STATE> &&cb_state) { cbs.emplace_back(std::move(cb_state)); }

    void AddSignalSemaphore(std::shared_ptr<SEMAPHORE_STATE> &&semaphore_state, uint64_t value) {
        signal_semaphores.emplace_back(std::move(semaphore_state), value);
    }

    void AddWaitSemaphore(std::shared_ptr<SEMAPHORE_STATE> &&semaphore_state, uint64_t value) {
        wait_semaphores.emplace_back(std::move(semaphore_state), value);
    }

    void AddFence(std::shared_ptr<FENCE_STATE> &&fence_state) { fence = std::move(fence_state); }

    void EndUse();
    void BeginUse();
};

class QUEUE_STATE : public BASE_NODE {
  public:
    QUEUE_STATE(ValidationStateTracker &dev_data, VkQueue q, uint32_t index, VkDeviceQueueCreateFlags flags,
                const VkQueueFamilyProperties &queueFamilyProperties)
        : BASE_NODE(q, kVulkanObjectTypeQueue),
          queueFamilyIndex(index),
          flags(flags),
          queueFamilyProperties(queueFamilyProperties),
          dev_data_(dev_data) {}

    ~QUEUE_STATE() { Destroy(); }
    void Destroy() override;

    VkQueue Queue() const { return handle_.Cast<VkQueue>(); }

    uint64_t Submit(CB_SUBMISSION &&submission);

    // Tell the queue thread that submissions up to the submission with sequence number until_seq have finished
    uint64_t Notify(uint64_t until_seq = vvl::kU64Max);

    // Tell the queue and then wait for it to finish updating its state.
    // UINT64_MAX means to finish all submissions.
    void NotifyAndWait(uint64_t until_seq = vvl::kU64Max);
    std::shared_future<void> Wait(uint64_t until_seq = vvl::kU64Max);

    const uint32_t queueFamilyIndex;
    const VkDeviceQueueCreateFlags flags;
    const VkQueueFamilyProperties queueFamilyProperties;

  private:
    using LockGuard = std::unique_lock<std::mutex>;
    void ThreadFunc();
    CB_SUBMISSION *NextSubmission();
    LockGuard Lock() const { return LockGuard(lock_); }

    ValidationStateTracker &dev_data_;

    // state related to submitting to the queue, all data members must
    // be accessed with lock_ held
    std::unique_ptr<std::thread> thread_;
    std::deque<CB_SUBMISSION> submissions_;
    std::atomic<uint64_t> seq_{0};
    uint64_t request_seq_{0};
    bool exit_thread_{false};
    mutable std::mutex lock_;
    // condition to wake up the queue's thread
    std::condition_variable cond_;
};
