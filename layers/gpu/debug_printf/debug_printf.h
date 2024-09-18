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

#include "gpu/core/gpu_state_tracker.h"
#include "gpu/instrumentation/gpu_shader_instrumentor.h"
#include "gpu/resources/gpu_resources.h"

namespace gpuav {
struct DebugPrintfBufferInfo;
class CommandBuffer;
class Validator;

namespace debug_printf {
void AllocateResources(Validator& gpuav, CommandBuffer& cb_state, const VkPipelineBindPoint bind_point, const Location& loc);
void AnalyzeAndGenerateMessage(Validator& gpuav, VkCommandBuffer command_buffer, VkQueue queue,
                               gpuav::DebugPrintfBufferInfo& buffer_info, uint32_t* const debug_output_buffer, const Location& loc);
}  // namespace debug_printf
}  // namespace gpuav

namespace debug_printf {
class Validator : public gpu::GpuShaderInstrumentor {
  public:
    using BaseClass = gpu::GpuShaderInstrumentor;
    Validator() { container_type = LayerObjectTypeDebugPrintf; }

    void PreCallRecordCreateDevice(VkPhysicalDevice physicalDevice, const VkDeviceCreateInfo* pCreateInfo,
                                   const VkAllocationCallbacks* pAllocator, VkDevice* pDevice, const RecordObject& record_obj,
                                   vku::safe_VkDeviceCreateInfo* modified_create_info) final;
    void PostCreateDevice(const VkDeviceCreateInfo* pCreateInfo, const Location& loc) override;
};
}  // namespace debug_printf
