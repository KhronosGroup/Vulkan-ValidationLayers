// Copyright (c) 2024-2025 The Khronos Group Inc.
// Copyright (c) 2024-2025 Valve Corporation
// Copyright (c) 2024-2025 LunarG, Inc.
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

#ifndef ROOT_NODE_H
#define ROOT_NODE_H

#include "shader_defines.h"

#ifdef __cplusplus
using uint = unsigned int;

namespace gpuav {
namespace glsl {

#else

#extension GL_EXT_buffer_reference : require
#extension GL_EXT_buffer_reference2 : require
#extension GL_EXT_buffer_reference_uvec2 : require
#if defined(GL_ARB_gpu_shader_int64)
#extension GL_ARB_gpu_shader_int64 : require
#else
#error No extension available for 64-bit integers.
#endif
#extension GL_EXT_scalar_block_layout : require

#include "gpuav_error_header.h"
#include "gpuav_shaders_constants.h"

// Without a Spec Constant, GLSL (or any language) will be smart and constant fold for us
// When linking we can apply the constant fold for it
// (The constant_id doesn't matter, it easier to just hot swap the known default constant value)
layout(constant_id = 0) const uint SpecConstantLinkShaderId = kLinkShaderId;
#endif

BUFFER_ADDR_FWD_DECL(OutputBuffer)
BUFFER_ADDR_FWD_DECL(ActionIndexBuffer)
BUFFER_ADDR_FWD_DECL(ErrorLoggerIndexBuffer)
BUFFER_ADDR_FWD_DECL(CmdErrorsCountBuffer)

#ifndef __cplusplus
layout(buffer_reference, buffer_reference_align = 4, scalar) buffer OutputBuffer {
    uint size;
    uint written_count;
    uint data[];
};

layout(buffer_reference, buffer_reference_align = 4, scalar) buffer ActionIndexBuffer { uint index[]; };

layout(buffer_reference, buffer_reference_align = 4, scalar) buffer ErrorLoggerIndexBuffer { uint index[]; };

layout(buffer_reference, buffer_reference_align = 4, scalar) buffer CmdErrorsCountBuffer { uint errors_count[]; };
#endif

BUFFER_ADDR_FWD_DECL(DebugPrintfBuffer)
BUFFER_ADDR_FWD_DECL(VertexAttributeFetchLimits)
BUFFER_ADDR_FWD_DECL(BDAInputBuffer)
BUFFER_ADDR_FWD_DECL(PostProcessSSBO)
BUFFER_ADDR_FWD_DECL(BoundDescriptorSetsStateSSBO)

BUFFER_ADDR_STRUCT(RootNode, 8) {
    BUFFER_ADDR_DECL(DebugPrintfBuffer) debug_printf_buffer;
    BUFFER_ADDR_DECL(OutputBuffer) inst_errors_buffer;
    BUFFER_ADDR_DECL(ActionIndexBuffer) inst_action_index_buffer;
    BUFFER_ADDR_DECL(ErrorLoggerIndexBuffer) inst_error_logger_index_buffer;
    BUFFER_ADDR_DECL(CmdErrorsCountBuffer) inst_cmd_errors_count_buffer;

    BUFFER_ADDR_DECL(VertexAttributeFetchLimits) vertex_attribute_fetch_limits_buffer;
    BUFFER_ADDR_DECL(BDAInputBuffer) bda_input_buffer;
    BUFFER_ADDR_DECL(PostProcessSSBO) post_process_ssbo;  // #ARNO_TODO use _buffer everywhere
    BUFFER_ADDR_DECL(BoundDescriptorSetsStateSSBO) bound_desc_sets_state_ssbo;
};

#ifndef __cplusplus
layout(set = kInstDefaultDescriptorSet, binding = kBindingInstRootNode, scalar) buffer RootNodeBuffer { RootNode root_node; };
#endif

#ifdef __cplusplus
}  // namespace glsl
}  // namespace gpuav
#endif

#endif
