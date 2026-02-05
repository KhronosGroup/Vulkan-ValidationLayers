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
// Values used between the GLSL shaders and the GPU-AV logic

// NOTE: This header is included by the instrumentation shaders and glslang doesn't support #pragma once
#ifndef GPU_ERROR_CODES_H
#define GPU_ERROR_CODES_H

#ifdef __cplusplus
namespace gpuav {
namespace glsl {
#endif

// Error Groups
//
// These will match one-for-one with the file found in gpu_shader folder
// Note - We currently have a max of 256 slots for error groups (see kHeader_ShaderIdErrorOffset)
const int kErrorGroup_InstDescriptorIndexingOOB = 1;
const int kErrorGroup_InstBufferDeviceAddress = 2;
const int kErrorGroup_InstRayQuery = 3;
const int kErrorGroup_GpuPreDraw = 4;
const int kErrorGroup_GpuPreDispatch = 5;
const int kErrorGroup_GpuPreTraceRays = 6;
const int kErrorGroup_GpuCopyBufferToImage = 7;
const int kErrorGroup_InstDescriptorClass = 8;
const int kErrorGroup_InstIndexedDraw = 9;
const int kErrorGroup_GpuCopyMemoryIndirect = 10;
const int kErrorGroup_InstSanitizer = 11;
const int kErrorGroup_GpuPreBuildAccelerationStructures = 12;
const int kErrorGroup_InstMeshShading = 13;
const int kErrorGroup_InstRayHitObject = 14;

// We just take ExecutionModel and normalize it so we only use 5 bits to store it
const int kExecutionModel_Vertex = 0;
const int kExecutionModel_TessellationControl = 1;
const int kExecutionModel_TessellationEvaluation = 2;
const int kExecutionModel_Geometry = 3;
const int kExecutionModel_Fragment = 4;
const int kExecutionModel_GLCompute = 5;
const int kExecutionModel_Kernel = 6;
const int kExecutionModel_TaskNV = 7;
const int kExecutionModel_MeshNV = 8;
const int kExecutionModel_RayGenerationKHR = 9;
const int kExecutionModel_IntersectionKHR = 10;
const int kExecutionModel_AnyHitKHR = 11;
const int kExecutionModel_ClosestHitKHR = 12;
const int kExecutionModel_MissKHR = 13;
const int kExecutionModel_CallableKHR = 14;
const int kExecutionModel_TaskEXT = 15;
const int kExecutionModel_MeshEXT = 16;
const int kExecutionModel_Unknown = 17;  // replace if new stage is added

// Descriptor Indexing
//
const int kErrorSubCode_DescriptorIndexing_Bounds = 1;
const int kErrorSubCode_DescriptorIndexing_Uninitialized = 2;
const int kErrorSubCode_DescriptorIndexing_Destroyed = 3;

// Descriptor Class specific errors
//
// Buffers
const int kErrorSubCode_DescriptorClass_GeneralBufferBounds = 1;
// Texel Buffers
const int kErrorSubCode_DescriptorClass_TexelBufferBounds = 2;
// Buffers, but with Cooperative Matrix
const int kErrorSubCode_DescriptorClass_GeneralBufferCoopMatBounds = 3;

// Buffer Device Address
//
const int kErrorSubCode_BufferDeviceAddress_UnallocRef = 1;
const int kErrorSubCode_BufferDeviceAddress_Alignment = 2;

// Ray Query
//
const int kErrorSubCode_RayQuery_NegativeMin = 1;
const int kErrorSubCode_RayQuery_NegativeMax = 2;
const int kErrorSubCode_RayQuery_BothSkip = 3;
const int kErrorSubCode_RayQuery_SkipCull = 4;
const int kErrorSubCode_RayQuery_Opaque = 5;
const int kErrorSubCode_RayQuery_MinMax = 6;
const int kErrorSubCode_RayQuery_MinNaN = 7;
const int kErrorSubCode_RayQuery_MaxNaN = 8;
const int kErrorSubCode_RayQuery_OriginNaN = 9;
const int kErrorSubCode_RayQuery_DirectionNaN = 10;
const int kErrorSubCode_RayQuery_OriginFinite = 11;
const int kErrorSubCode_RayQuery_DirectionFinite = 12;

// Ray Hit Object (VK_EXT_ray_tracing_invocation_reorder)
// OpHitObjectTraceRayEXT, OpHitObjectTraceReorderExecuteEXT, OpHitObjectTraceRayMotionEXT,
// OpHitObjectTraceMotionReorderExecuteEXT, OpHitObjectSetShaderBindingTableRecordIndexEXT
//
const int kErrorSubCode_RayHitObject_NegativeMin = 1;
const int kErrorSubCode_RayHitObject_NegativeMax = 2;
const int kErrorSubCode_RayHitObject_BothSkip = 3;
const int kErrorSubCode_RayHitObject_SkipCull = 4;
const int kErrorSubCode_RayHitObject_Opaque = 5;
const int kErrorSubCode_RayHitObject_MinMax = 6;
const int kErrorSubCode_RayHitObject_MinNaN = 7;
const int kErrorSubCode_RayHitObject_MaxNaN = 8;
const int kErrorSubCode_RayHitObject_OriginNaN = 9;
const int kErrorSubCode_RayHitObject_DirectionNaN = 10;
const int kErrorSubCode_RayHitObject_OriginFinite = 11;
const int kErrorSubCode_RayHitObject_DirectionFinite = 12;
const int kErrorSubCode_RayHitObject_SkipTrianglesWithPipelineSkipAABBs = 13;
const int kErrorSubCode_RayHitObject_SkipAABBsWithPipelineSkipTriangles = 14;
const int kErrorSubCode_RayHitObject_TimeOutOfRange = 15;
const int kErrorSubCode_RayHitObject_SBTIndexExceedsLimit = 16;

// MeshShading
//
const int kErrorSubCode_MeshShading_SetMeshOutputs = 1;

// Indexed Draw
//
const int kErrorSubCode_IndexedDraw_OOBVertexIndex = 1;
const int kErrorSubCode_IndexedDraw_OOBInstanceIndex = 2;

// Sanitizer
//
const int kErrorSubCode_Sanitizer_Empty = 0;  // reserved to mean no error was set
const int kErrorSubCode_Sanitizer_DivideZero = 1;
const int kErrorSubCode_Sanitizer_ImageGather = 2;
const int kErrorSubCode_Sanitizer_Pow = 3;
const int kErrorSubCode_Sanitizer_Atan2 = 4;
const int kErrorSubCode_Sanitizer_Fminmax = 5;
const int kErrorSubCode_Sanitizer_Count = 6;  // update when adding new item

// Pre Draw
//
// The draw count exceeded the draw buffer size
const int kErrorSubCode_PreDraw_DrawBufferSize = 1;
// The draw count exceeded the maxDrawCount parameter to the command
const int kErrorSubCode_PreDraw_DrawCountLimit = 2;
// A firstInstance field was non-zero
const int kErrorSubCode_PreDraw_FirstInstance = 3;
// Mesh limit checks
const int kErrorSubCode_PreDraw_GroupCountX = 4;
const int kErrorSubCode_PreDraw_GroupCountY = 5;
const int kErrorSubCode_PreDraw_GroupCountZ = 6;
const int kErrorSubCode_PreDraw_GroupCountTotal = 7;
// The index count exceeded the index buffer size
const int kErrorSubCode_OobIndexBuffer = 8;
// An index in the index buffer exceeded the vertex buffer size
const int kErrorSubCode_OobVertexBuffer = 9;

// Pre Dispatch
//
const int kErrorSubCode_PreDispatch_CountLimitX = 1;
const int kErrorSubCode_PreDispatch_CountLimitY = 2;
const int kErrorSubCode_PreDispatch_CountLimitZ = 3;

// Pre Trace Rays
//
const int kErrorSubCode_PreTraceRays_LimitWidth = 1;
const int kErrorSubCode_PreTraceRays_LimitHeight = 2;
const int kErrorSubCode_PreTraceRays_LimitDepth = 3;
const int kErrorSubCode_PreTraceRays_LimitVolume = 4;
// Pre Copy Buffer To Image
//
const int kErrorSubCodePreCopyBufferToImageBufferTexel = 1;

// Pre Copy Memory Indirect
//
const int kErrorSubCode_PreCopyMemoryIndirect_SrcAddressAligned = 1;
const int kErrorSubCode_PreCopyMemoryIndirect_DstAddressAligned = 2;
const int kErrorSubCode_PreCopyMemoryIndirect_SizeAligned = 3;
const int kErrorSubCode_PreCopyMemoryToImageIndirect_SrcAddressAligned = 4;
const int kErrorSubCode_PreCopyMemoryToImageIndirect_BufferRowLength = 5;
const int kErrorSubCode_PreCopyMemoryToImageIndirect_BufferImageHeight = 6;
const int kErrorSubCode_PreCopyMemoryIndirect_SrcAddressInvalid = 7;
const int kErrorSubCode_PreCopyMemoryIndirect_DstAddressInvalid = 8;
const int kErrorSubCode_PreCopyMemoryToImageIndirect_SrcAddressInvalid = 9;

// Pre Build Acceleration Structures
//
const int kErrorSubCode_PreBuildAccelerationStructures_BlasAddrAlignment = 1;
const int kErrorSubCode_PreBuildAccelerationStructures_InvalidAS = 2;
const int kErrorSubCode_PreBuildAccelerationStructures_DestroyedASBuffer = 3;
const int kErrorSubCode_PreBuildAccelerationStructures_InvalidASType = 4;
const int kErrorSubCode_PreBuildAccelerationStructures_DestroyedASMemory = 5;
const int kErrorSubCode_PreBuildAccelerationStructures_BlasMemoryOverlap = 6;
const int kErrorSubCode_PreBuildAccelerationStructures_MaxFetchedIndex = 7;
const int kErrorSubCode_PreBuildAccelerationStructures_MinMaxAabb_X = 8;
const int kErrorSubCode_PreBuildAccelerationStructures_MinMaxAabb_Y = 9;
const int kErrorSubCode_PreBuildAccelerationStructures_MinMaxAabb_Z = 10;
#ifdef __cplusplus
}  // namespace glsl
}  // namespace gpuav
#endif
#endif
