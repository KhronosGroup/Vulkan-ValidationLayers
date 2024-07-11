/* Copyright (c) 2020-2024 The Khronos Group Inc.
 * Copyright (c) 2020-2024 Valve Corporation
 * Copyright (c) 2020-2024 LunarG, Inc.
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
#include "vma/vma.h"

struct Location;

namespace gpuav {
class CommandBuffer;
class Validator;

void UpdateBoundPipeline(Validator& gpuav, VkCommandBuffer cb, VkPipelineBindPoint pipeline_bind_point, VkPipeline pipeline,
                         const Location& loc);
void UpdateBoundDescriptors(Validator& gpuav, VkCommandBuffer cb, VkPipelineBindPoint pipeline_bind_point, const Location& loc);
[[nodiscard]] bool UpdateBindlessStateBuffer(Validator& gpuav, CommandBuffer& cb_state, VmaAllocator vma_allocator,
                                             const Location& loc);
}  // namespace gpuav
