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
#include "state_tracker/cmd_buffer_state.h"

namespace gpu {
class GpuShaderInstrumentor;
}

// TODO - Remove this file and namespace
// The only reason we have this is because DebugPrintf and GPU-AV have different command buffers
namespace gpu_tracker {

class CommandBuffer : public vvl::CommandBuffer {
  public:
    CommandBuffer(gpu::GpuShaderInstrumentor &shader_instrumentor_, VkCommandBuffer handle,
                  const VkCommandBufferAllocateInfo *pCreateInfo, const vvl::CommandPool *pool);

    virtual bool PreProcess(const Location &loc) = 0;
    virtual void PostProcess(VkQueue queue, const Location &loc) = 0;
};
}  // namespace gpu_tracker

VALSTATETRACK_DERIVED_STATE_OBJECT(VkCommandBuffer, gpu_tracker::CommandBuffer, vvl::CommandBuffer)
