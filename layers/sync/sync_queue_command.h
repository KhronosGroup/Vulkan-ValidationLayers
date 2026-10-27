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

#include <deque>
#include <variant>
#include <vulkan/vulkan.h>

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

struct QueueWaitIdleCommand {
    VkQueue queue;
    bool IsReady() const { return true; }
    bool Execute(SyncValidator& validator) const;
};

struct DeviceWaitIdleCommand {
    bool IsReady() const { return true; }
    bool Execute(SyncValidator& validator) const;
};

using QueueCommand = std::variant<WaitForFenceCommand, WaitForSemaphoreCommand, QueueWaitIdleCommand, DeviceWaitIdleCommand>;
using QueueCommandStorage = std::deque<QueueCommand>;

// Whether the command is ready to execute.
// Commands may need to wait for gpu results to be processed
bool IsQueueCommandReady(const QueueCommand& command);

// Returns the skip value
bool ExecuteQueueCommand(const QueueCommand& command, SyncValidator& validator);

}  // namespace syncval
