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
#pragma once
#include "state_tracker/state_object.h"
#include <future>
#include <map>
#include <mutex>
#include "containers/custom_containers.h"
#include "error_message/error_location.h"

class ValidationStateTracker;

namespace vvl {

class Queue;
struct SubmissionLocator;

class Semaphore : public RefcountedStateObject {
  public:
    // possible payload values for binary semaphore
    enum OpType {
        kNone,
        kWait,
        kSignal,
        kBinaryAcquire,
    };
    enum Scope {
        kInternal,
        kExternalTemporary,
        kExternalPermanent,
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
        SemOp(OpType ot, Queue *q, uint64_t queue_seq, uint64_t timeline_payload, Func command = Func::Empty)
            : op_type(ot), command(command), queue(q), seq(queue_seq), payload(timeline_payload) {}

        OpType op_type;
        Func command;
        Queue *queue;
        uint64_t seq;
        uint64_t payload;

        bool operator<(const SemOp &rhs) const { return payload < rhs.payload; }

        bool IsWait() const { return op_type == kWait; }
        bool IsSignal() const { return op_type == kSignal; }
        bool IsAcquire() const { return op_type == kBinaryAcquire; }

        void Notify() const;
    };

    struct TimePoint {
        TimePoint(SemOp &op) : signal_op(), completed(), waiter(completed.get_future()) {
            if (op.op_type == kWait) {
                AddWaitOp(op);
            } else {
                signal_op.emplace(op);
            }
        }
        void AddWaitOp(const SemOp &op) {
            assert(op.op_type == kWait);
            assert(wait_ops.empty() || wait_ops[0].payload == op.payload);
            wait_ops.emplace_back(op);
        }
        std::optional<SemOp> signal_op;
        small_vector<SemOp, 1, uint32_t> wait_ops;
        std::promise<void> completed;
        std::shared_future<void> waiter;

        bool HasSignaler() const { return signal_op.has_value(); }
        bool HasWaiters() const { return !wait_ops.empty(); }
        void Notify() const;
    };

#ifdef VK_USE_PLATFORM_METAL_EXT
    static bool GetMetalExport(const VkSemaphoreCreateInfo *info) {
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
    VkExternalSemaphoreHandleTypeFlags GetExportHandleTypes(const VkSemaphoreCreateInfo *pCreateInfo) {
        auto export_info = vku::FindStructInPNextChain<VkExportSemaphoreCreateInfo>(pCreateInfo->pNext);
        return export_info ? export_info->handleTypes : 0;
    }

    Semaphore(ValidationStateTracker &dev, VkSemaphore sem, const VkSemaphoreCreateInfo *pCreateInfo)
        : Semaphore(dev, sem, vku::FindStructInPNextChain<VkSemaphoreTypeCreateInfo>(pCreateInfo->pNext), pCreateInfo) {}

    Semaphore(ValidationStateTracker &dev, VkSemaphore sem, const VkSemaphoreTypeCreateInfo *type_create_info,
                    const VkSemaphoreCreateInfo *pCreateInfo)
        : RefcountedStateObject(sem, kVulkanObjectTypeSemaphore),
#ifdef VK_USE_PLATFORM_METAL_EXT
          metal_semaphore_export(GetMetalExport(pCreateInfo)),
#endif  // VK_USE_PLATFORM_METAL_EXT
          type(type_create_info ? type_create_info->semaphoreType : VK_SEMAPHORE_TYPE_BINARY),
          flags(pCreateInfo->flags),
          exportHandleTypes(GetExportHandleTypes(pCreateInfo)),
          completed_{type == VK_SEMAPHORE_TYPE_TIMELINE ? kSignal : kNone, nullptr, 0,
                     type_create_info ? type_create_info->initialValue : 0},
          next_payload_(completed_.payload + 1),
          dev_data_(dev) {
    }

    VkSemaphore VkHandle() const { return handle_.Cast<VkSemaphore>(); }
    Scope Scope() const {
        auto guard = ReadLock();
        return scope_;
    }
    VkExternalSemaphoreHandleTypeFlagBits ImportedHandleType() const {
        auto guard = ReadLock();
        assert(imported_handle_type_.has_value());
        return imported_handle_type_.value();
    }
    // This is the most recently completed operation. It is returned by value so that the caller
    // has a correct copy even if something else is completing on this queue in a different thread.
    SemOp Completed() const {
        auto guard = ReadLock();
        return completed_;
    }

    // Enqueue a semaphore operation. For binary semaphores, the payload value is generated and
    // returned, so that every semaphore operation has a unique value.
    void EnqueueSignal(Queue *queue, uint64_t queue_seq, uint64_t &payload);
    void EnqueueWait(Queue *queue, uint64_t queue_seq, uint64_t &payload);

    // Binary only special cases enqueue functions
    void EnqueueAcquire(Func command);

    // Signal queue(s) that need to retire because a wait on this payload has finished
    void Notify(uint64_t payload);

    std::shared_future<void> Wait(uint64_t payload);

    // Helper for retiring timeline semaphores and then retiring all queues using the semaphore
    void NotifyAndWait(const Location &loc, uint64_t payload);

    // Remove completed operations and signal any waiters. This should only be called by Queue
    void Retire(Queue *current_queue, const Location &loc, uint64_t payload);

    // look for most recent / highest payload operation that matches
    std::optional<SemOp> LastOp(const std::function<bool(const SemOp &, bool is_pending)> &filter = nullptr) const;

    // Returns queue submission associated with the last binary signal.
    std::optional<SubmissionLocator> GetLastBinarySignalSubmission() const;

    bool CanBinaryBeSignaled() const;
    bool CanBinaryBeWaited() const;
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
    const VkSemaphoreCreateFlags flags;
    const VkExternalSemaphoreHandleTypeFlags exportHandleTypes;

  private:
    ReadLockGuard ReadLock() const { return ReadLockGuard(lock_); }
    WriteLockGuard WriteLock() { return WriteLockGuard(lock_); }

    enum Scope scope_ { kInternal };
    std::optional<VkExternalSemaphoreHandleTypeFlagBits> imported_handle_type_;  // has value when scope is not kInternal

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

// NOTE: Present semaphores are waited on by the implementation, not queue operations.
// We do not yet have a good way to figure out when this wait completes,
// so we must assume they are safe to re-use.
static inline bool CanSignalBinarySemaphoreAfterOperation(Semaphore::OpType op_type) {
    return op_type == Semaphore::kNone || op_type == Semaphore::kWait;
}
static inline bool CanWaitBinarySemaphoreAfterOperation(Semaphore::OpType op_type) {
    return op_type == Semaphore::kSignal || op_type == Semaphore::kBinaryAcquire;
}

}  // namespace vvl
