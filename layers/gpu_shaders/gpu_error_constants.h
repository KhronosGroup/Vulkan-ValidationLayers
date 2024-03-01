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

#ifdef __cplusplus
namespace gpuav {
namespace glsl {
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

// Size of Common and Stage-specific Members
const int kInstStageOutCnt = kInstCommonOutCnt + 3;

// This identifies the validation error
// We use groups to more easily mangage the many int values not conflicting
const int kErrorGroup = kInstStageOutCnt;
const int kErrorSubCode = kErrorGroup + 1;

// Maximum Output Record Member Count
const int kInstMaxOutCnt = kErrorSubCode + 6;

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

// Error Group
//
// These will match one-for-one with the file found in gpu_shader folder
const int kErrorGroupInstBindlessDescriptor = 1;
const int kErrorGroupInstBufferDeviceAddress = 2;
const int kErrorGroupInstRayQuery = 3;
const int kErrorGroupGpuPreDraw = 4;
const int kErrorGroupGpuPreDispatch = 5;
const int kErrorGroupGpuPreTraceRays = 6;
const int kErrorGroupGpuCopyBufferToImage = 7;

// Bindless Descriptor
//
const int kErrorSubCodeBindlessDescriptorBounds = 1;
const int kErrorSubCodeBindlessDescriptorUninit = 2;
const int kErrorSubCodeBindlessDescriptorOOB = 3;
const int kErrorSubCodeBindlessDescriptorDestroyed = 4;

// A bindless bounds error will output the index and the bound.
const int kInstBindlessBoundsOutDescSet = kErrorSubCode + 1;
const int kInstBindlessBoundsOutDescBinding = kErrorSubCode + 2;
const int kInstBindlessBoundsOutDescIndex = kErrorSubCode + 3;
const int kInstBindlessBoundsOutDescBound = kErrorSubCode + 4;
const int kInstBindlessBoundsOutUnused = kErrorSubCode + 5;
const int kInstBindlessBoundsOutCnt = kErrorSubCode + 6;

// A descriptor uninitialized error will output the index.
const int kInstBindlessUninitOutDescSet = kErrorSubCode + 1;
const int kInstBindlessUninitOutBinding = kErrorSubCode + 2;
const int kInstBindlessUninitOutDescIndex = kErrorSubCode + 3;
const int kInstBindlessUninitOutUnused = kErrorSubCode + 4;
const int kInstBindlessUninitOutUnused2 = kErrorSubCode + 5;
const int kInstBindlessUninitOutCnt = kErrorSubCode + 6;

// A buffer out-of-bounds error will output the descriptor
// index, the buffer offset and the buffer size
const int kInstBindlessBuffOOBOutDescSet = kErrorSubCode + 1;
const int kInstBindlessBuffOOBOutDescBinding = kErrorSubCode + 2;
const int kInstBindlessBuffOOBOutDescIndex = kErrorSubCode + 3;
const int kInstBindlessBuffOOBOutBuffOff = kErrorSubCode + 4;
const int kInstBindlessBuffOOBOutBuffSize = kErrorSubCode + 5;
const int kInstBindlessBuffOOBOutCnt = kErrorSubCode + 6;

// Buffer Device Address
//
const int kErrorSubCodeBufferDeviceAddressUnallocRef = 1;

// A buffer address unalloc error will output the 64-bit pointer in
// two 32-bit pieces, lower bits first.
const int kInstBuffAddrUnallocOutDescPtrLo = kErrorSubCode + 1;
const int kInstBuffAddrUnallocOutDescPtrHi = kErrorSubCode + 2;
const int kInstBuffAddrUnallocOutCnt = kErrorSubCode + 3;

// Ray Query
//
const int kErrorSubCodeRayQueryNegativeMin = 1;
const int kErrorSubCodeRayQueryNegativeMax = 2;
const int kErrorSubCodeRayQueryBothSkip = 3;
const int kErrorSubCodeRayQuerySkipCull = 4;
const int kErrorSubCodeRayQueryOpaque = 5;
const int kErrorSubCodeRayQueryMinMax = 6;
const int kErrorSubCodeRayQueryMinNaN = 7;
const int kErrorSubCodeRayQueryMaxNaN = 8;
const int kErrorSubCodeRayQueryOriginNaN = 9;
const int kErrorSubCodeRayQueryDirectionNaN = 10;
const int kErrorSubCodeRayQueryOriginFinite = 11;
const int kErrorSubCodeRayQueryDirectionFinite = 12;

const int kInstRayQueryOutParam0 = kErrorSubCode + 1;

// Used by all "Pre" shaders
const int kPreActionOutParam0 = kErrorSubCode + 1;
const int kPreActionOutParam1 = kErrorSubCode + 2;

// Pre Draw
//
const int kErrorSubCodePreDrawBufferSize = 1;
const int kErrorSubCodePreDrawCountLimit = 2;
const int kErrorSubCodePreDrawFirstInstance = 3;
const int kErrorSubCodePreDrawGroupCountX = 4;
const int kErrorSubCodePreDrawGroupCountY = 5;
const int kErrorSubCodePreDrawGroupCountZ = 6;
const int kErrorSubCodePreDrawGroupCountTotal = 7;

const int kPreDrawSelectCountBuffer = 1;
const int kPreDrawSelectDrawBuffer = 2;
const int kPreDrawSelectMeshCountBuffer = 3;
const int kPreDrawSelectMeshNoCount = 4;

// Pre Dispatch
//
const int kErrorSubCodePreDispatchCountLimitX = 1;
const int kErrorSubCodePreDispatchCountLimitY = 2;
const int kErrorSubCodePreDispatchCountLimitZ = 3;

// Pre Tracy Rays
//
const int kErrorSubCodePreTraceRaysLimitWidth = 1;
const int kErrorSubCodePreTraceRaysLimitHeight = 2;
const int kErrorSubCodePreTraceRaysLimitDepth = 3;

// Pre Copy Buffer To Image
//
const int kErrorSubCodePreCopyBufferToImageBufferTexel = 1;

#ifdef __cplusplus
}  // namespace glsl
}  // namespace gpuav
#endif
#endif
