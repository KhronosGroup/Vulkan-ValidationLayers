/* Copyright (c) 2015-2021 The Khronos Group Inc.
 * Copyright (c) 2015-2021 Valve Corporation
 * Copyright (c) 2015-2021 LunarG, Inc.
 * Copyright (C) 2015-2021 Google Inc.
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
#include <vector>
#include <deque>

class CMD_BUFFER_STATE;
class QUEUE_STATE;

enum SyncScope {
    kSyncScopeInternal,
    kSyncScopeExternalTemporary,
    kSyncScopeExternalPermanent,
};

enum FENCE_STATUS { FENCE_UNSIGNALED, FENCE_INFLIGHT, FENCE_RETIRED };

struct QUEUE_SIGNALER {
    QUEUE_STATE *queue{nullptr};
    uint64_t seq{0};
};

class FENCE_STATE : public REFCOUNTED_NODE {
  public:
    const VkFenceCreateInfo createInfo;
    QUEUE_SIGNALER signaler;
    FENCE_STATUS state;
    SyncScope scope{kSyncScopeInternal};

    // Default constructor
    FENCE_STATE(VkFence f, const VkFenceCreateInfo *pCreateInfo)
        : REFCOUNTED_NODE(f, kVulkanObjectTypeFence),
          createInfo(*pCreateInfo),
          state((pCreateInfo->flags & VK_FENCE_CREATE_SIGNALED_BIT) ? FENCE_RETIRED : FENCE_UNSIGNALED),
          scope(kSyncScopeInternal) {}

    VkFence fence() const { return handle_.Cast<VkFence>(); }

    void Retire();
};

class SEMAPHORE_STATE : public REFCOUNTED_NODE {
  public:
    QUEUE_SIGNALER signaler;
    bool signaled{false};
    SyncScope scope{kSyncScopeInternal};
    const VkSemaphoreType type;
    uint64_t payload;

    SEMAPHORE_STATE(VkSemaphore sem, const VkSemaphoreTypeCreateInfo *type_create_info)
        : REFCOUNTED_NODE(sem, kVulkanObjectTypeSemaphore),
          type(type_create_info ? type_create_info->semaphoreType : VK_SEMAPHORE_TYPE_BINARY),
          payload(type_create_info ? type_create_info->initialValue : 0) {}

    VkSemaphore semaphore() const { return handle_.Cast<VkSemaphore>(); }

    void Retire(uint64_t next_seq);
};

struct SEMAPHORE_WAIT {
    std::shared_ptr<SEMAPHORE_STATE> semaphore;
    uint64_t payload{0};
    uint64_t seq{0};
};

struct SEMAPHORE_SIGNAL {
    std::shared_ptr<SEMAPHORE_STATE> semaphore;
    uint64_t payload{0};
    uint64_t seq{0};
};

struct CB_SUBMISSION {
    std::vector<std::shared_ptr<CMD_BUFFER_STATE>> cbs;
    std::vector<SEMAPHORE_WAIT> wait_semaphores;
    std::vector<SEMAPHORE_SIGNAL> signal_semaphores;
    std::shared_ptr<FENCE_STATE> fence;
    uint32_t perf_submit_pass{0};

    void AddCommandBuffer(std::shared_ptr<CMD_BUFFER_STATE> &&cb_node) { cbs.emplace_back(std::move(cb_node)); }

    void AddSignalSemaphore(std::shared_ptr<SEMAPHORE_STATE> &&semaphore_state, uint64_t value) {
        SEMAPHORE_SIGNAL signal;
        signal.semaphore = std::move(semaphore_state);
        signal.payload = value;
        signal.seq = 0;
        signal_semaphores.emplace_back(std::move(signal));
    }

    void AddWaitSemaphore(std::shared_ptr<SEMAPHORE_STATE> &&semaphore_state, uint64_t value) {
        SEMAPHORE_WAIT wait;
        wait.semaphore = std::move(semaphore_state);
        wait.payload = value;
        wait_semaphores.emplace_back(std::move(wait));
    }

    void AddFence(std::shared_ptr<FENCE_STATE> &&fence_state) { fence = std::move(fence_state); }
};

class QUEUE_STATE : public BASE_NODE {
  public:
    const uint32_t queueFamilyIndex;

    uint64_t seq;
    std::deque<CB_SUBMISSION> submissions;

    QUEUE_STATE(VkQueue q, uint32_t index) : BASE_NODE(q, kVulkanObjectTypeQueue), queueFamilyIndex(index), seq(0) {}

    VkQueue Queue() const { return handle_.Cast<VkQueue>(); }

    uint64_t Submit(CB_SUBMISSION &&submission);

    void Retire(uint64_t next_seq);
    void Retire() { Retire(seq + submissions.size()); }
};
