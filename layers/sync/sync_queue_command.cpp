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

#include "sync/sync_queue_command.h"
#include "sync/sync_validation.h"

namespace syncval {

bool WaitForFenceCommand::Execute(SyncValidator& validator) const {
    validator.WaitForFence(fence);
    return false;
}

bool IsQueueCommandReady(const QueueCommand& command) {
    return std::visit([](const auto& operation) { return operation.IsReady(); }, command);
}

bool ExecuteQueueCommand(const QueueCommand& command, SyncValidator& validator) {
    return std::visit([&validator](const auto& operation) { return operation.Execute(validator); }, command);
}

}  // namespace syncval
