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

enum SyncScope {
    kSyncScopeInternal,
    kSyncScopeExternalTemporary,
    kSyncScopeExternalPermanent,
};

enum FENCE_STATUS { FENCE_UNSIGNALED, FENCE_INFLIGHT, FENCE_RETIRED };

class FENCE_STATE : public REFCOUNTED_NODE {
  public:
    VkFenceCreateInfo createInfo;
    std::pair<VkQueue, uint64_t> signaler;
    FENCE_STATUS state;
    SyncScope scope;

    // Default constructor
    FENCE_STATE(VkFence f, const VkFenceCreateInfo* pCreateInfo)
        : REFCOUNTED_NODE(f, kVulkanObjectTypeFence),
          createInfo(*pCreateInfo),
          state((pCreateInfo->flags & VK_FENCE_CREATE_SIGNALED_BIT) ? FENCE_RETIRED : FENCE_UNSIGNALED),
          scope(kSyncScopeInternal) {}

    VkFence fence() const { return handle_.Cast<VkFence>(); }
};

class SEMAPHORE_STATE : public REFCOUNTED_NODE {
  public:
    std::pair<VkQueue, uint64_t> signaler;
    bool signaled;
    SyncScope scope;
    VkSemaphoreType type;
    uint64_t payload;

    SEMAPHORE_STATE(VkSemaphore sem, const VkSemaphoreTypeCreateInfo* type_create_info)
        : REFCOUNTED_NODE(sem, kVulkanObjectTypeSemaphore),
          signaler(VK_NULL_HANDLE, 0),
          signaled(false),
          scope(kSyncScopeInternal),
          type(type_create_info ? type_create_info->semaphoreType : VK_SEMAPHORE_TYPE_BINARY),
          payload(type_create_info ? type_create_info->initialValue : 0) {}

    VkSemaphore semaphore() const { return handle_.Cast<VkSemaphore>(); }
};

struct SEMAPHORE_WAIT {
    VkSemaphore semaphore;
    VkSemaphoreType type;
    VkQueue queue;
    uint64_t payload;
    uint64_t seq;
};

struct SEMAPHORE_SIGNAL {
    VkSemaphore semaphore;
    uint64_t payload;
    uint64_t seq;
};

struct CB_SUBMISSION {
    CB_SUBMISSION()
        : cbs(), waitSemaphores(), signalSemaphores(), externalSemaphores(), fence(VK_NULL_HANDLE), perf_submit_pass(0) {}

    std::vector<VkCommandBuffer> cbs;
    std::vector<SEMAPHORE_WAIT> waitSemaphores;
    std::vector<SEMAPHORE_SIGNAL> signalSemaphores;
    std::vector<VkSemaphore> externalSemaphores;
    VkFence fence;
    uint32_t perf_submit_pass;
};

class QUEUE_STATE {
  public:
    VkQueue queue;
    uint32_t queueFamilyIndex;

    uint64_t seq;
    std::deque<CB_SUBMISSION> submissions;

    QUEUE_STATE(VkQueue q, uint32_t index) : queue(q), queueFamilyIndex(index), seq(0) {}
};
