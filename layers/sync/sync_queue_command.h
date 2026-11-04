/* Copyright (c) 2026 The Khronos Group Inc.
 * Copyright (c) 2026 Valve Corporation
 * Copyright (c) 2026 LunarG, Inc.
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

#include "sync/sync_common.h"
#include "containers/small_vector.h"
#include "containers/span.h"
#include <deque>
#include <variant>

namespace vvl {
class CommandBuffer;
class Swapchain;
enum class Func;
}  // namespace vvl

namespace syncval {

class SyncValidator;

struct WaitForFenceCommand {
    VkFence fence;
    bool IsReady() const { return true; }
    bool Execute(SyncValidator& validator) const;
};

struct WaitForSemaphoreCommand {
    VkSemaphore semaphore;
    uint64_t value;
    bool IsReady() const { return true; }
    bool Execute(SyncValidator& validator) const;
};

struct SignalSemaphoreCommand {
    VkSemaphore semaphore;
    uint64_t value;
    bool IsReady() const { return true; }
    bool Execute(SyncValidator& validator) const;
};

struct HostResetEventCommand {
    VkEvent event;
    bool IsReady() const { return true; }
    bool Execute(SyncValidator& validator) const;
};

struct QueueWaitIdleCommand {
    VkQueue queue;
    bool IsReady() const { return true; }
    bool Execute(SyncValidator& validator) const;
};

struct DeviceWaitIdleCommand {
    bool IsReady() const { return true; }
    bool Execute(SyncValidator& validator) const;
};

struct ResourceDestroyedCommand {
    small_vector<AccessRange, 1> ranges;
    bool IsReady() const { return true; }
    bool Execute(SyncValidator& validator) const;
};

struct SemaphoreDestroyedCommand {
    VkSemaphore semaphore;
    bool IsReady() const { return true; }
    bool Execute(SyncValidator& validator) const;
};

struct SwapchainDestroyedCommand {
    VkSwapchainKHR swapchain;
    bool IsReady() const { return true; }
    bool Execute(SyncValidator& validator) const;
};

struct QueuePresentCommand {
    struct ImageInfo {
        std::shared_ptr<vvl::Swapchain> swapchain;
        std::shared_ptr<const vvl::Image> image;
        uint32_t swapchain_index;
        uint32_t image_index;
    };
    QueueId queue_id;
    small_vector<VkSemaphore, 1> wait_semaphores;
    small_vector<ImageInfo, 1> images;  // per swapchain

    QueuePresentCommand(SyncValidator& validator, QueueId queue_id, const VkPresentInfoKHR& present_info);
    bool IsReady() const { return true; }
    bool Execute(SyncValidator& validator) const;
};

struct AcquireNextImageCommand {
    std::shared_ptr<vvl::Swapchain> swapchain;
    std::shared_ptr<const vvl::Image> image;
    uint32_t image_index;
    vvl::Func command;
    VkSemaphore semaphore;
    VkFence fence;

    bool IsReady() const { return true; }
    bool Execute(SyncValidator& validator) const;
};

struct QueueSubmitCommand {
    struct SubmitBatch {
        small_vector<VkSemaphoreSubmitInfo, 1> waits;
        small_vector<std::shared_ptr<const vvl::CommandBuffer>, 1> command_buffers;
        small_vector<VkSemaphoreSubmitInfo, 1> signals;
    };
    QueueId queue_id;
    VkFence fence;
    vvl::Func submit_func;
    std::vector<std::string> initial_label_stack;
    std::vector<SubmitBatch> batches;

    QueueSubmitCommand(SyncValidator& validator, QueueId queue_id, vvl::span<const VkSubmitInfo2> submits, VkFence fence,
                       vvl::Func submit_func);
    bool IsReady() const { return true; }
    bool Execute(SyncValidator& validator) const;
};

using QueueCommand = std::variant<WaitForFenceCommand, WaitForSemaphoreCommand, SignalSemaphoreCommand, HostResetEventCommand,
                                  QueueWaitIdleCommand, DeviceWaitIdleCommand, ResourceDestroyedCommand, SemaphoreDestroyedCommand,
                                  SwapchainDestroyedCommand, QueuePresentCommand, AcquireNextImageCommand, QueueSubmitCommand>;
using QueueCommandStorage = std::deque<QueueCommand>;

// Whether the command is ready to execute.
// Commands may need to wait for gpu results to be processed
bool IsQueueCommandReady(const QueueCommand& command);

// Returns the skip value
bool ExecuteQueueCommand(const QueueCommand& command, SyncValidator& validator);

}  // namespace syncval
