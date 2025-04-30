/* Copyright (c) 2020-2025 The Khronos Group Inc.
 * Copyright (c) 2020-2025 Valve Corporation
 * Copyright (c) 2020-2025 LunarG, Inc.
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

struct Location;
struct LastBound;

namespace gpuav {
class CommandBufferSubState;
class Validator;
struct DescriptorBindingCommand;

namespace descriptor {
void UpdateBoundDescriptorsPostProcess(Validator& gpuav, CommandBufferSubState& cb_state, const LastBound& last_bound,
                                       DescriptorBindingCommand& descriptor_binding_cmd);
void UpdateBoundDescriptorsDescriptorChecks(Validator& gpuav, CommandBufferSubState& cb_state, const LastBound& last_bound,
                                            DescriptorBindingCommand& descriptor_binding_cmd, const Location& loc);
void UpdateBoundDescriptors(Validator& gpuav, CommandBufferSubState& cb_state, VkPipelineBindPoint pipeline_bind_point,
                            const Location& loc);
[[nodiscard]] bool UpdateDescriptorStateSSBO(Validator& gpuav, CommandBufferSubState& cb_state, const Location& loc);
}  // namespace descriptor
}  // namespace gpuav
