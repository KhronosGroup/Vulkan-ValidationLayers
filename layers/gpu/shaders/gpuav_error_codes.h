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
#ifndef GPU_ERROR_CODES_H
#define GPU_ERROR_CODES_H

#ifdef __cplusplus
namespace gpuav {
namespace glsl {
#endif

// Error Groups
//
// These will match one-for-one with the file found in gpu_shader folder
const int kErrorGroupInstBindlessDescriptor = 1;
const int kErrorGroupInstBufferDeviceAddress = 2;
const int kErrorGroupInstRayQuery = 3;
const int kErrorGroupGpuPreDraw = 4;
const int kErrorGroupGpuPreDispatch = 5;
const int kErrorGroupGpuPreTraceRays = 6;
const int kErrorGroupGpuCopyBufferToImage = 7;
const int kErrorGroupInstNonBindlessOOB = 8;

// Used for MultiEntry and there is no single stage set
const int kHeaderStageIdMultiEntryPoint = 0x7fffffff;  // same as spv::ExecutionModelMax

// Bindless Descriptor
//
const int kErrorSubCodeBindlessDescriptorBounds = 1;
const int kErrorSubCodeBindlessDescriptorUninit = 2;
const int kErrorSubCodeBindlessDescriptorOOB = 3;
const int kErrorSubCodeBindlessDescriptorDestroyed = 4;

// Non-Bindless OOB
//
// Buffers
const int kErrorSubCodeNonBindlessOOBBufferArrays = 1;
const int kErrorSubCodeNonBindlessOOBBufferBounds = 2;
// Texel Buffers
const int kErrorSubCodeNonBindlessOOBTexelBufferArrays = 3;
const int kErrorSubCodeNonBindlessOOBTexelBufferBounds = 4;

// Buffer Device Address
//
const int kErrorSubCodeBufferDeviceAddressUnallocRef = 1;

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
