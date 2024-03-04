// Copyright (c) 2021-2024 The Khronos Group Inc.
// Copyright (c) 2021-2024 Valve Corporation
// Copyright (c) 2021-2024 LunarG, Inc.
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
#ifndef GPU_ERROR_CONSTANTS_H
#define GPU_ERROR_CONSTANTS_H

#include "gpu_error_codes.h"

#ifdef __cplusplus
namespace gpuav {
namespace glsl {
#endif

// GPU-AV Error record structure:
// ------------------------------

// /---------------------------------
// | Error header
// | 	- Record size
// |	- Shader Id
// |	- Instruction Id
// |	- Shader stage Id
// |	- Shader stage info (3 integers)
// | 	- Error group (Id unique to the shader/instrumentation code that wrote the error)
// |	- subcode (maps to VUIDs)
// | --------------------------------
// | Error specific parameters
// \---------------------------------
//
// The size of these parts depends on the validation being done.

// Error Header offsets:
// ---------------------
// The following are offsets to fields common to all records

// Each record first contains the size of the record in 32-bit words, including
// the size word.
const int kHeaderErrorRecordSizeOffset = 0;

// This is the shader id passed by the layer when the instrumentation pass is
// created.
const int kHeaderShaderIdOffset = 1;

// This is the ordinal position of the instruction within the SPIR-V shader
// which generated the validation error.
const int kHeaderInstructionIdOffset = 2;

// This is the stage which generated the validation error. This word is used
// to determine the contents of the next two words in the record.
const int kHeaderStageIdOffset = 3;  // Values come from SpvExecutionModel (See spirv.h):
const int kHeaderStageInfoOffset_0 = 4;
const int kHeaderStageInfoOffset_1 = 5;
const int kHeaderStageInfoOffset_2 = 6;

// Stage-specific Error record offsets
// ---
// Each stage will contain different values in the next set of words of the
// record used to identify which instantiation of the shader generated the
// validation error.
//
// Vertex Shader Output Record Offsets
const int kHeaderVertexIndexOffset = kHeaderStageInfoOffset_0;
const int kHeaderVertInstanceIndexOffset = kHeaderStageInfoOffset_1;

// Frag Shader Output Record Offsets
const int kHeaderFragCoordXOffset = kHeaderStageInfoOffset_0;
const int kHeaderFragCoordYOffset = kHeaderStageInfoOffset_1;

// Compute Shader Output Record Offsets
const int kHeaderInvocationIdXOffset = kHeaderStageInfoOffset_0;
const int kHeaderInvocationIdYOffset = kHeaderStageInfoOffset_1;
const int kHeaderInvocationIdZOffset = kHeaderStageInfoOffset_2;

// Tessellation Control Shader Output Record Offsets
const int kHeaderTessCltInvocationIdOffset = kHeaderStageInfoOffset_0;
const int kHeaderTessCtlPrimitiveIdOffset = kHeaderStageInfoOffset_1;

// Tessellation Eval Shader Output Record Offsets
const int kHeaderTessEvalPrimitiveIdOffset = kHeaderStageInfoOffset_0;
const int kHeaderTessEvalCoordUOffset = kHeaderStageInfoOffset_1;
const int kHeaderTessEvalCoordVOffset = kHeaderStageInfoOffset_2;

// Geometry Shader Output Record Offsets
const int kHeaderGeomPrimitiveIdOffset = kHeaderStageInfoOffset_0;
const int kHeaderGeomInvocationIdOffset = kHeaderStageInfoOffset_1;

// Ray Tracing Shader Output Record Offsets
const int kHeaderRayTracingLaunchIdXOffset = kHeaderStageInfoOffset_0;
const int kHeaderRayTracingLaunchIdYOffset = kHeaderStageInfoOffset_1;
const int kHeaderRayTracingLaunchIdZOffset = kHeaderStageInfoOffset_2;

// Mesh Shader Output Record Offsets
const int kHeaderMeshGlobalInvocationIdXOffset = kHeaderStageInfoOffset_0;
const int kHeaderMeshGlobalInvocationIdYOffset = kHeaderStageInfoOffset_1;
const int kHeaderMeshGlobalInvocationIdZOffset = kHeaderStageInfoOffset_2;

// Task Shader Output Record Offsets
const int kHeaderTaskGlobalInvocationIdXOffset = kHeaderStageInfoOffset_0;
const int kHeaderTaskGlobalInvocationIdYOffset = kHeaderStageInfoOffset_1;
const int kHeaderTaskGlobalInvocationIdZOffset = kHeaderStageInfoOffset_2;

// This identifies the validation error
// We use groups to more easily mangage the many int values not conflicting
const int kHeaderErrorGroupOffset = 7;
const int kHeaderErrorSubCodeOffset = 8;

const int kHeaderSize = 9;

// Error specific parameters offsets:
// ----------------------------------

// A bindless bounds error will output the index and the bound.
const int kInstBindlessBoundsDescSetOffset = kHeaderSize;
const int kInstBindlessBoundsDescBindingOffset = kHeaderSize + 1;
const int kInstBindlessBoundsDescIndexOffset = kHeaderSize + 2;
const int kInstBindlessBoundsDescBoundOffset = kHeaderSize + 3;
const int kInstBindlessBoundsUnusedOffset = kHeaderSize + 4;
const int kInstBindlessBoundsCntOffset = kHeaderSize + 5;

// A descriptor uninitialized error will output the index.
const int kInstBindlessUninitDescSetOffset = kHeaderSize;
const int kInstBindlessUninitBindingOffset = kHeaderSize + 1;
const int kInstBindlessUninitDescIndexOffset = kHeaderSize + 2;
const int kInstBindlessUninitUnusedOffset = kHeaderSize + 3;
const int kInstBindlessUninitOutUnused2 = kHeaderSize + 4;
const int kInstBindlessUninitCntOffset = kHeaderSize + 5;

// A buffer out-of-bounds error will output the descriptor
// index, the buffer offset and the buffer size
const int kInstBindlessBuffOOBDescSetOffset = kHeaderSize;
const int kInstBindlessBuffOOBDescBindingOffset = kHeaderSize + 1;
const int kInstBindlessBuffOOBDescIndexOffset = kHeaderSize + 2;
const int kInstBindlessBuffOOBBuffOffOffset = kHeaderSize + 3;
const int kInstBindlessBuffOOBBuffSizeOffset = kHeaderSize + 4;
const int kInstBindlessBuffOOBCntOffset = kHeaderSize + 5;

// A buffer address unalloc error will output the 64-bit pointer in
// two 32-bit pieces, lower bits first.
const int kInstBuffAddrUnallocDescPtrLoOffset = kHeaderSize;
const int kInstBuffAddrUnallocDescPtrHiOffset = kHeaderSize + 1;
const int kInstBuffAddrUnallocCntOffset = kHeaderSize + 2;

const int kInstRayQueryParamOffset_0 = kHeaderSize;

// Used by all "Pre" shaders
const int kPreActionParamOffset_0 = kHeaderSize;
const int kPreActionParamOffset_1 = kHeaderSize + 1;

// Maximum record size
const int kMaxErrorRecordSize = kHeaderSize + 7;

#ifdef __cplusplus
}  // namespace glsl
}  // namespace gpuav
#endif
#endif
