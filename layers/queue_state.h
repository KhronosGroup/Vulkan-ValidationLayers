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
#pragma once
#include "base_node.h"
#include <deque>
#include <set>
#include <vector>
#include "vk_layer_utils.h"

class CMD_BUFFER_STATE;
class QUEUE_STATE;

enum SyncScope {
    kSyncScopeInternal,
    kSyncScopeExternalTemporary,
    kSyncScopeExternalPermanent,
};

enum FENCE_STATUS { FENCE_UNSIGNALED, FENCE_INFLIGHT, FENCE_RETIRED };

class FENCE_STATE : public REFCOUNTED_NODE {
  public:
    // Default constructor
    FENCE_STATE(VkFence f, const VkFenceCreateInfo *pCreateInfo)
        : REFCOUNTED_NODE(f, kVulkanObjectTypeFence),
          createInfo(*pCreateInfo),
          state_((pCreateInfo->flags & VK_FENCE_CREATE_SIGNALED_BIT) ? FENCE_RETIRED : FENCE_UNSIGNALED),
          scope_(kSyncScopeInternal) {}

    VkFence fence() const { return handle_.Cast<VkFence>(); }

    bool EnqueueSignal(QUEUE_STATE *queue_state, uint64_t next_seq);

    void Retire(bool notify_queue = true);

    void Reset();

    void Import(VkExternalFenceHandleTypeFlagBits handle_type, VkFenceImportFlags flags);

    void Export(VkExternalFenceHandleTypeFlagBits handle_type);

    const VkFenceCreateInfo createInfo;

    SyncScope Scope() const { return scope_; }
    FENCE_STATUS State() const { return state_; }
    QUEUE_STATE *Queue() const { return queue_; }
    uint64_t QueueSeq() const { return seq_; }

  private:
    ReadLockGuard ReadLock() const { return ReadLockGuard(lock_); }
    WriteLockGuard WriteLock() { return WriteLockGuard(lock_); }

    QUEUE_STATE *queue_{nullptr};
    uint64_t seq_{0};
    FENCE_STATUS state_;
    SyncScope scope_{kSyncScopeInternal};
    mutable ReadWriteLock lock_;
};

class SEMAPHORE_STATE : public REFCOUNTED_NODE {
  public:
    // possible payload values for binary semaphore
    enum OpType {
        kNone,
        kWait,
        kSignal,
        kBinaryAcquire,
        kBinaryPresent,
    };
    static inline const char *OpTypeName(OpType t) {
        switch (t) {
            case kWait:
                return "wait";
            case kSignal:
                return "signal";
            case kBinaryAcquire:
                return "acquire";
            case kBinaryPresent:
                return "present";
            case kNone:
            default:
                return "NONE";
        }
    }

    struct SemOp {
        // NOTE: c++11 doesn't allow aggregate initialization and default member
        // initializers in the same struct. This limitation is removed in c++14
        OpType op_type;
        QUEUE_STATE *queue;
        uint64_t seq;
        uint64_t payload;

        bool operator<(const SemOp &rhs) const { return payload < rhs.payload; }

        bool IsWait() const { return op_type == kWait || op_type == kBinaryPresent; }
        bool IsSignal() const { return op_type == kSignal; }

        // NOTE: Present semaphores are waited on by the implementation, not queue operations. We do not yet
        // have a good way to figure out when this wait completes, so we must assume they are safe to re-use
        bool CanBeSignaled() const { return op_type == kNone || op_type == kWait || op_type == kBinaryPresent; }
        bool CanBeWaited() const {  return op_type == kSignal || op_type == kBinaryAcquire; }
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

    SEMAPHORE_STATE(VkSemaphore sem, const VkSemaphoreTypeCreateInfo *type_create_info, const VkSemaphoreCreateInfo *pCreateInfo)
        : REFCOUNTED_NODE(sem, kVulkanObjectTypeSemaphore),
#ifdef VK_USE_PLATFORM_METAL_EXT
          metal_semaphore_export(GetMetalExport(pCreateInfo)),
#endif  // VK_USE_PLATFORM_METAL_EXT
          type(type_create_info ? type_create_info->semaphoreType : VK_SEMAPHORE_TYPE_BINARY),
          completed_{kNone, nullptr, 0, type_create_info ? type_create_info->initialValue : 0},
          next_payload_(completed_.payload + 1) {}

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
    bool EnqueueSignal(QUEUE_STATE *queue, uint64_t queue_seq, uint64_t &payload);
    void EnqueueWait(QUEUE_STATE *queue, uint64_t queue_seq, uint64_t &payload);

    // Binary only special cases enqueue functions
    void EnqueueAcquire();
    void EnqueuePresent(QUEUE_STATE *queue);

    // Remove completed operations and return highest sequence numbers for all affected queues
    using RetireResult = layer_data::unordered_map<QUEUE_STATE *, uint64_t>;
    RetireResult Retire(QUEUE_STATE *queue, uint64_t payload);

    // Helper for retiring timeline semaphores and then retiring all queues using the semaphore
    void RetireTimeline(uint64_t payload);

    // look for most recent / highest payload operation that matches
    layer_data::optional<SemOp> LastOp(std::function<bool(const SemOp &)> filter = nullptr) const;

    bool CanBeSignaled() const;
    bool CanBeWaited() const;
    VkQueue AnotherQueueWaitsBinary(VkQueue queue) const;
    bool HasPendingOps() const {
        auto guard = ReadLock();
        return !operations_.empty();
    }

    void Import(VkExternalSemaphoreHandleTypeFlagBits handle_type, VkSemaphoreImportFlags flags);
    void Export(VkExternalSemaphoreHandleTypeFlagBits handle_type);
#ifdef VK_USE_PLATFORM_METAL_EXT
    const bool metal_semaphore_export;
#endif  // VK_USE_PLATFORM_METAL_EXT
    const VkSemaphoreType type;

  private:
    ReadLockGuard ReadLock() const { return ReadLockGuard(lock_); }
    WriteLockGuard WriteLock() { return WriteLockGuard(lock_); }

    SyncScope scope_{kSyncScopeInternal};
    // the most recently completed operation
    SemOp completed_{};
    // next payload value for binary semaphore operations
    uint64_t next_payload_;

    std::vector<std::shared_ptr<std::function<void()>>> waiting_functions_;

    // Set of pending operations ordered by payload. This must be a multiset because
    // timeline operations can be added in any order and multiple operations
    // can use the same payload value.
    std::multiset<SemOp> operations_;
    mutable ReadWriteLock lock_;
};

struct CB_SUBMISSION {
    struct SemaphoreInfo {
        std::shared_ptr<SEMAPHORE_STATE> semaphore;
        uint64_t payload{0};
    };

    std::vector<std::shared_ptr<CMD_BUFFER_STATE>> cbs;
    std::vector<SemaphoreInfo> wait_semaphores;
    std::vector<SemaphoreInfo> signal_semaphores;
    std::shared_ptr<FENCE_STATE> fence;
    uint32_t perf_submit_pass{0};

    void AddCommandBuffer(std::shared_ptr<CMD_BUFFER_STATE> &&cb_node) { cbs.emplace_back(std::move(cb_node)); }

    void AddSignalSemaphore(std::shared_ptr<SEMAPHORE_STATE> &&semaphore_state, uint64_t value) {
        SemaphoreInfo signal;
        signal.semaphore = std::move(semaphore_state);
        signal.payload = value;
        signal_semaphores.emplace_back(std::move(signal));
    }

    void AddWaitSemaphore(std::shared_ptr<SEMAPHORE_STATE> &&semaphore_state, uint64_t value) {
        SemaphoreInfo wait;
        wait.semaphore = std::move(semaphore_state);
        wait.payload = value;
        wait_semaphores.emplace_back(std::move(wait));
    }

    void AddFence(std::shared_ptr<FENCE_STATE> &&fence_state) { fence = std::move(fence_state); }
};

class QUEUE_STATE : public BASE_NODE {
  public:
    QUEUE_STATE(VkQueue q, uint32_t index, VkDeviceQueueCreateFlags flags)
        : BASE_NODE(q, kVulkanObjectTypeQueue), queueFamilyIndex(index), flags(flags), seq_(0) {}

    VkQueue Queue() const { return handle_.Cast<VkQueue>(); }

    uint64_t Submit(CB_SUBMISSION &&submission);

    bool HasWait(VkSemaphore semaphore, VkFence fence) const;

    void Retire(uint64_t until_seq = UINT64_MAX);

    const uint32_t queueFamilyIndex;
    const VkDeviceQueueCreateFlags flags;

  private:
    layer_data::optional<CB_SUBMISSION> NextSubmission(uint64_t until_seq);
    ReadLockGuard ReadLock() const { return ReadLockGuard(lock_); }
    WriteLockGuard WriteLock() { return WriteLockGuard(lock_); }

    std::deque<CB_SUBMISSION> submissions_;
    uint64_t seq_;
    mutable ReadWriteLock lock_;
};
