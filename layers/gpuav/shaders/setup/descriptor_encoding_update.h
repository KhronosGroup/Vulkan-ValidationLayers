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

#ifndef DESCRIPTOR_ENCODING_UPDATE_H
#define DESCRIPTOR_ENCODING_UPDATE_H

#ifdef __cplusplus

#include <cstdint>

namespace gpuav {
namespace glsl {
using uint = uint32_t;
#else

#extension GL_ARB_gpu_shader_int64 : require
#extension GL_EXT_buffer_reference : require
#extension GL_EXT_buffer_reference2 : require
#extension GL_EXT_buffer_reference_uvec2 : require
#extension GL_EXT_scalar_block_layout : require
#endif

#ifdef __cplusplus
using DescriptorEncodingsPtr = uint64_t;
#else

layout(buffer_reference, scalar) buffer DescriptorEncodingsPtr { uvec2 array[]; };

#endif

struct DescriptorEncodingUpdateShaderPushData {
    DescriptorEncodingsPtr cb_desc_encodings_ptr;
    DescriptorEncodingsPtr staged_desc_encodings_ptr;
    uint start_binding;
};

#ifdef __cplusplus
}  // namespace glsl
}  // namespace gpuav
#endif

#endif
