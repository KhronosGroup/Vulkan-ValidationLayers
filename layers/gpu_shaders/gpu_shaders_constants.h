// Copyright (c) 2021-2022 The Khronos Group Inc.
// Copyright (c) 2021-2023 Valve Corporation
// Copyright (c) 2021-2023 LunarG, Inc.
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
// Values used between the GLSL shaders and the GPU-AV logic

// NOTE: This header is included by the instrumentation shaders and glslang doesn't support #pragma once
#ifndef GPU_SHADERS_CONSTANTS_H
#define GPU_SHADERS_CONSTANTS_H

#ifdef __cplusplus
namespace gpuav_glsl {
using uint = unsigned int;

// Upper bound for maxUpdateAfterBindDescriptorsInAllPools. This value needs to
// be small enough to allow for a table in memory, but some devices set it to 2^32.
// This value only matters for host code, but it is defined here so it can be used
// in unit tests.
const uint kDebugInputBindlessMaxDescriptors = 1024u*1024u;

#endif

// Common Stream Record Offsets
//
// The following are offsets to fields which are common to all records written
// to the output stream.
//
// Each record first contains the size of the record in 32-bit words, including
// the size word.
const int kInstCommonOutSize = 0;

// This is the shader id passed by the layer when the instrumentation pass is
// created.
const int kInstCommonOutShaderId = 1;

// This is the ordinal position of the instruction within the SPIR-V shader
// which generated the validation error.
const int kInstCommonOutInstructionIdx = 2;

// This is the stage which generated the validation error. This word is used
// to determine the contents of the next two words in the record.
// 0:Vert, 1:TessCtrl, 2:TessEval, 3:Geom, 4:Frag, 5:Compute
const int kInstCommonOutStageIdx = 3;
const int kInstCommonOutCnt = 4;

// Stage-specific Stream Record Offsets
//
// Each stage will contain different values in the next set of words of the
// record used to identify which instantiation of the shader generated the
// validation error.
//
// Vertex Shader Output Record Offsets
const int kInstVertOutVertexIndex = kInstCommonOutCnt;
const int kInstVertOutInstanceIndex = kInstCommonOutCnt + 1;
const int kInstVertOutUnused = kInstCommonOutCnt + 2;

// Frag Shader Output Record Offsets
const int kInstFragOutFragCoordX = kInstCommonOutCnt;
const int kInstFragOutFragCoordY = kInstCommonOutCnt + 1;
const int kInstFragOutUnused = kInstCommonOutCnt + 2;

// Compute Shader Output Record Offsets
const int kInstCompOutGlobalInvocationIdX = kInstCommonOutCnt;
const int kInstCompOutGlobalInvocationIdY = kInstCommonOutCnt + 1;
const int kInstCompOutGlobalInvocationIdZ = kInstCommonOutCnt + 2;

// Tessellation Control Shader Output Record Offsets
const int kInstTessCtlOutInvocationId = kInstCommonOutCnt;
const int kInstTessCtlOutPrimitiveId = kInstCommonOutCnt + 1;
const int kInstTessCtlOutUnused = kInstCommonOutCnt + 2;

// Tessellation Eval Shader Output Record Offsets
const int kInstTessEvalOutPrimitiveId = kInstCommonOutCnt;
const int kInstTessEvalOutTessCoordU = kInstCommonOutCnt + 1;
const int kInstTessEvalOutTessCoordV = kInstCommonOutCnt + 2;

// Geometry Shader Output Record Offsets
const int kInstGeomOutPrimitiveId = kInstCommonOutCnt;
const int kInstGeomOutInvocationId = kInstCommonOutCnt + 1;
const int kInstGeomOutUnused = kInstCommonOutCnt + 2;

// Ray Tracing Shader Output Record Offsets
const int kInstRayTracingOutLaunchIdX = kInstCommonOutCnt;
const int kInstRayTracingOutLaunchIdY = kInstCommonOutCnt + 1;
const int kInstRayTracingOutLaunchIdZ = kInstCommonOutCnt + 2;

// Mesh Shader Output Record Offsets
const int kInstMeshOutGlobalInvocationIdX = kInstCommonOutCnt;
const int kInstMeshOutGlobalInvocationIdY = kInstCommonOutCnt + 1;
const int kInstMeshOutGlobalInvocationIdZ = kInstCommonOutCnt + 2;

// Task Shader Output Record Offsets
const int kInstTaskOutGlobalInvocationIdX = kInstCommonOutCnt;
const int kInstTaskOutGlobalInvocationIdY = kInstCommonOutCnt + 1;
const int kInstTaskOutGlobalInvocationIdZ = kInstCommonOutCnt + 2;

// Size of Common and Stage-specific Members
const int kInstStageOutCnt = kInstCommonOutCnt + 3;

// Validation Error Code Offset
//
// This identifies the validation error. It also helps to identify
// how many words follow in the record and their meaning.
const int kInstValidationOutError = kInstStageOutCnt;

// Validation-specific Output Record Offsets
//
// Each different validation will generate a potentially different
// number of words at the end of the record giving more specifics
// about the validation error.
//
// A bindless bounds error will output the index and the bound.
const int kInstBindlessBoundsOutDescSet = kInstStageOutCnt + 1;
const int kInstBindlessBoundsOutDescBinding = kInstStageOutCnt + 2;
const int kInstBindlessBoundsOutDescIndex = kInstStageOutCnt + 3;
const int kInstBindlessBoundsOutDescBound = kInstStageOutCnt + 4;
const int kInstBindlessBoundsOutUnused = kInstStageOutCnt + 5;
const int kInstBindlessBoundsOutCnt = kInstStageOutCnt + 6;

// A descriptor uninitialized error will output the index.
const int kInstBindlessUninitOutDescSet = kInstStageOutCnt + 1;
const int kInstBindlessUninitOutBinding = kInstStageOutCnt + 2;
const int kInstBindlessUninitOutDescIndex = kInstStageOutCnt + 3;
const int kInstBindlessUninitOutUnused = kInstStageOutCnt + 4;
const int kInstBindlessUninitOutUnused2 = kInstStageOutCnt + 5;
const int kInstBindlessUninitOutCnt = kInstStageOutCnt + 6;

// A buffer out-of-bounds error will output the descriptor
// index, the buffer offset and the buffer size
const int kInstBindlessBuffOOBOutDescSet = kInstStageOutCnt + 1;
const int kInstBindlessBuffOOBOutDescBinding = kInstStageOutCnt + 2;
const int kInstBindlessBuffOOBOutDescIndex = kInstStageOutCnt + 3;
const int kInstBindlessBuffOOBOutBuffOff = kInstStageOutCnt + 4;
const int kInstBindlessBuffOOBOutBuffSize = kInstStageOutCnt + 5;
const int kInstBindlessBuffOOBOutCnt = kInstStageOutCnt + 6;

// A buffer address unalloc error will output the 64-bit pointer in
// two 32-bit pieces, lower bits first.
const int kInstBuffAddrUnallocOutDescPtrLo = kInstStageOutCnt + 1;
const int kInstBuffAddrUnallocOutDescPtrHi = kInstStageOutCnt + 2;
const int kInstBuffAddrUnallocOutCnt = kInstStageOutCnt + 3;

// Maximum Output Record Member Count
const int kInstMaxOutCnt = kInstStageOutCnt + 6;

const int kPreValidateSubError = kInstValidationOutError + 1;

// Validation Error Codes
//
// These are the possible validation error codes.
const int kInstErrorBindlessBounds = 1;
const int kInstErrorBindlessUninit = 2;
const int kInstErrorBuffAddrUnallocRef = 3;
const int kInstErrorOOB = 4;
const int kInstErrorPreDrawValidate = 5;
const int kInstErrorPreDispatchValidate = 6;
const int kInstErrorBindlessDestroyed = 7;
const int kInstErrorMax = 7;

// Direct Input Buffer Offsets
//
// The following values provide member offsets into the input buffers
// consumed by InstrumentPass::GenDebugDirectRead(). This method is utilized
// by InstBindlessCheckPass.
//
// The only object in an input buffer is a runtime array of unsigned
// integers. Each validation will have its own formatting of this array.
const int kDebugInputDataOffset = 0;

// The size of the inst_bindless_input_buffer array, regardless of how many
// descriptor sets the device supports.
const int kDebugInputBindlessMaxDescSets = 32;

// Special global descriptor id that skips checking.
const uint kDebugInputBindlessSkipId = 0x00ffffff;

// Top 8 bits of the descriptor id
const uint kDescBitShift = 24;
const uint kSamplerDesc = 1;
const uint kImageSamplerDesc = 2;
const uint kImageDesc = 3;
const uint kTexelDesc = 4;
const uint kBufferDesc = 5;
const uint kInlineUniformDesc = 6;
const uint kAccelDesc = 7;

// Buffer Device Address Input Buffer Format
//
// An input buffer for buffer device address validation consists of a single
// array of unsigned 64-bit integers we will call Data[]. This array is
// formatted as follows:
//
// At offset kDebugInputBuffAddrPtrOffset is a list of sorted valid buffer
// addresses. The list is terminated with the address 0xffffffffffffffff.
// If 0x0 is not a valid buffer address, this address is inserted at the
// start of the list.
//
const int kDebugInputBuffAddrPtrOffset = 1;
//
// At offset kDebugInputBuffAddrLengthOffset in Data[] is a single uint64 which
// gives an offset to the start of the buffer length data. More
// specifically, for a buffer whose pointer is located at input buffer offset
// i, the length is located at:
//
// Data[ i - kDebugInputBuffAddrPtrOffset
//         + Data[ kDebugInputBuffAddrLengthOffset ] ]
//
// The length associated with the 0xffffffffffffffff address is zero. If
// not a valid buffer, the length associated with the 0x0 address is zero.
const int kDebugInputBuffAddrLengthOffset = 0;

// These values all share the byte at (_kPreValidateSubError + 1) location since only
// one will be used at a time. Also equivalent to (kInstStageOutCnt + 1)
// debug buffer is memset to 0 so need to start at index 1
const int pre_draw_count_exceeds_bufsize_error = 1;
const int pre_draw_count_exceeds_limit_error = 2;
const int pre_draw_first_instance_error = 3;
const int pre_dispatch_count_exceeds_limit_x_error = 1;
const int pre_dispatch_count_exceeds_limit_y_error = 2;
const int pre_dispatch_count_exceeds_limit_z_error = 3;

#ifdef __cplusplus
} // namespace gpuav_glsl
#endif
#endif
