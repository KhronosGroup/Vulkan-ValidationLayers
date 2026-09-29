/* Copyright (c) 2024-2026 LunarG, Inc.
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
#include "containers/span.h"

namespace gpuav {
class Validator;
class CommandBufferSubState;

void RegisterBufferDeviceAddressValidation(Validator& gpuav, CommandBufferSubState& cb);
void RegisterPostProcessingValidation(Validator& gpuav, CommandBufferSubState& cb);
void RegisterMeshShadingValidation(Validator& gpuav, CommandBufferSubState& cb);
void RegisterSanitizer(Validator& gpuav, CommandBufferSubState& cb);
void RegisterVertexAttributeFetchOobValidation(Validator& gpuav, CommandBufferSubState& cb);
void RegisterSharedMemoryDataRaceValidation(Validator& gpuav, CommandBufferSubState& cb);
void RegisterTraceRayValidation(Validator& gpuav, CommandBufferSubState& cb);

struct AccelerationStructureGpuStateUpdate {
    VkAccelerationStructureKHR dst = VK_NULL_HANDLE;
    // If not null, dst GPU state becomes a copy of src GPU state, at copy AS time
    VkAccelerationStructureKHR src = VK_NULL_HANDLE;
    // Ignored if src is not null.
    // Maybe TODO: use this as a way to track that AS is coming from a copy or deserialization
    VkBuildAccelerationStructureModeKHR mode = VK_BUILD_ACCELERATION_STRUCTURE_MODE_BUILD_KHR;
};
void UpdateAccelerationStructureGpuState(Validator& gpuav, CommandBufferSubState& cb, const Location& loc,
                                         vvl::span<const AccelerationStructureGpuStateUpdate> updates);

}  // namespace gpuav
