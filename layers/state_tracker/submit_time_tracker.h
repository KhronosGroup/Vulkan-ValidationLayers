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

#include <vulkan/vulkan.h>
#include <memory>
#include <mutex>
#include <vector>

namespace vvl {

class DeviceProxy;
class CommandBuffer;

struct SubmissionBatch {
    std::vector<std::shared_ptr<CommandBuffer>> command_buffers;
};

// Tracks submission batches for submit time validation.
// Submit time validation is performed during queue submit calls
// such as QueueSubmit, QueuePresent, etc.
//
// Batches with no unresolved dependencies are validated immediately.
// Batches with pending timeline waits are stored until a subsequent
// queue operation specifies a resolving signal.
// vkSignalSemaphore that resolves a pending wait triggers validation.
//
// Validations that depend on actual completion of queue operations
// are not handled by this subsystem.
class SubmitTimeTracker {
  public:
    SubmitTimeTracker(DeviceProxy& validator) : validator_(validator) {}
    bool ProcessSubmissionBatch(const VkSubmitInfo& batch) const;

  private:
    DeviceProxy& validator_;

    // Submit time validation runs under mutex. This encompasses both validation and update.
    mutable std::mutex submit_time_mutex_;
};

}  // namespace vvl
