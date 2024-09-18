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

#include "gpu/core/gpu_state_tracker.h"
#include "gpu/instrumentation/gpu_shader_instrumentor.h"

namespace gpu_tracker {

CommandBuffer::CommandBuffer(gpu::GpuShaderInstrumentor &shader_instrumentor, VkCommandBuffer handle,
                             const VkCommandBufferAllocateInfo *pCreateInfo, const vvl::CommandPool *pool)
    : vvl::CommandBuffer(shader_instrumentor, handle, pCreateInfo, pool) {}

}  // namespace gpu_tracker
