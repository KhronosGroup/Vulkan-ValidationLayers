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

#include <vulkan/vulkan_core.h>

namespace vvl {
class Queue;
struct QueueSubmission;
struct SubmissionReference;
struct SyncValInputData;
}  // namespace vvl

namespace gpuav {
class Validator;
class CommandBufferSubState;

namespace syncval {

bool IsEnabled(const Validator& gpuav);
void DrawCountBuffer(Validator& gpuav, CommandBufferSubState& cb_state, VkBuffer api_count_buffer,
                     VkDeviceSize api_count_buffer_offset);
void CopyDrawCounts(CommandBufferSubState& cb_state);

// In PreSubmit, GPU-AV creates SyncVal data slots for command buffers that produce such data.
// In its own PreSubmit, SyncVal stores references to those slots in the queue commands it prepares for replay
void PrepareData(CommandBufferSubState& cb_state, vvl::Queue& queue, vvl::QueueSubmission& submission, uint32_t cb_index);

// When the submission retires, GPU-AV fills the slot with the collected data.
// SyncVal then reads that data during deferred replay
void CollectData(const CommandBufferSubState& cb_state, const vvl::SubmissionReference& submission,
                 vvl::SyncValInputData& syncval_data);

}  // namespace syncval
}  // namespace gpuav
