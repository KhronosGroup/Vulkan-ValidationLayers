// Copyright (c) 2021-2026 The Khronos Group Inc.
// Copyright (c) 2021-2026 Valve Corporation
// Copyright (c) 2021-2026 LunarG, Inc.
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
// Values used between the Slang shaders and the GPU-AV logic

#pragma once

#include "build_acceleration_structures.h"

namespace gpuav {
namespace shader {

#define COPY_GEOMETRIES_X_COMPONENT_SHADER_WG_SIZE_X 64
struct CopyGeometriesXComponentShaderPushData {
    float* geometries_x_component_copy;
    // Elements are stride sized, only the bytes can be typed
    uint64_t build_cmd_vertex;
    uint32_t count;
    uint32_t stride;
};

}  // namespace shader
}  // namespace gpuav
