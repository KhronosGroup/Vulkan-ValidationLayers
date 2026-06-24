/* Copyright (c) 2020-2026 The Khronos Group Inc.
 * Copyright (c) 2020-2026 Valve Corporation
 * Copyright (c) 2020-2026 LunarG, Inc.
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
#include <cstdint>

struct Location;
struct LastBound;

namespace gpuav {
class CommandBufferSubState;
class Validator;

namespace descriptor {
void UpdateBoundDescriptors(Validator& gpuav, CommandBufferSubState& cb_state, VkPipelineBindPoint pipeline_bind_point,
                            const Location& loc);

void UpdateBoundDescriptorHeap(Validator& gpuav, CommandBufferSubState& cb_state, bool is_sampler);
}  // namespace descriptor
}  // namespace gpuav
