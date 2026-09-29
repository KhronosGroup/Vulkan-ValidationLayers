// Copyright (c) 2024-2026 The Khronos Group Inc.
// Copyright (c) 2024-2026 Valve Corporation
// Copyright (c) 2024-2026 LunarG, Inc.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "acceleration_structure_gpu_state_update.h"

[vk::push_constant]
ConstantBuffer<gpuav::shader::AccelerationStructureGpuStateUpdateShaderPushData> pc;

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 thread_id: SV_DispatchThreadID) {
    if (pc.src_gpu_state_ptr != nullptr) {
        *pc.dst_gpu_state_ptr = *pc.src_gpu_state_ptr;
    } else {
        *pc.dst_gpu_state_ptr = pc.state;
    }
}
