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
#ifndef GPU_ERROR_HEADER_H
#define GPU_ERROR_HEADER_H

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
// |	- Action command index in command buffer
// |    - Command resources index
// | 	- Error group (Id unique to the shader/instrumentation code that wrote the error)
// |	- subcode (maps to VUIDs)
// | --------------------------------
// | Error specific parameters
// \---------------------------------

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
// Each stage will contain different values in the next set of words of the
// record used to identify which instantiation of the shader generated the
// validation error.
const int kHeaderStageInfoOffset_0 = 4;
const int kHeaderStageInfoOffset_1 = 5;
const int kHeaderStageInfoOffset_2 = 6;

const int kHeaderActionIdOffset = 7;
const int kHeaderCommandResourceIdOffset = 8;

// This identifies the validation error
// We use groups to more easily manage the many int values not conflicting
const int kHeaderErrorGroupOffset = 9;
const int kHeaderErrorSubCodeOffset = 10;

const int kHeaderSize = 11;

// Error specific parameters offsets:
// ----------------------------------

// Bindless
// ---

const int kInstBindlessDescSetOffset = kHeaderSize;
const int kInstBindlessDescBindingOffset = kHeaderSize + 1;
const int kInstBindlessDescIndexOffset = kHeaderSize + 2;
const int kInstBindlessCustomOffset_0 = kHeaderSize + 3;
const int kInstBindlessCustomOffset_1 = kHeaderSize + 4;

// A bindless bounds error will output the index and the bound.
const int kInstBindlessBoundsDescSetOffset = kInstBindlessDescSetOffset;
const int kInstBindlessBoundsDescBindingOffset = kInstBindlessDescBindingOffset;
const int kInstBindlessBoundsDescIndexOffset = kInstBindlessDescIndexOffset;

// A descriptor uninitialized error will output the index.
const int kInstBindlessUninitDescSetOffset = kInstBindlessDescSetOffset;
const int kInstBindlessUninitBindingOffset = kInstBindlessDescBindingOffset;
const int kInstBindlessUninitDescIndexOffset = kInstBindlessDescIndexOffset;

// A buffer out-of-bounds error will output the descriptor
// index, the buffer offset and the buffer size
const int kInstBindlessBuffOOBDescSetOffset = kInstBindlessDescSetOffset;
const int kInstBindlessBuffOOBDescBindingOffset = kInstBindlessDescBindingOffset;
const int kInstBindlessBuffOOBDescIndexOffset = kInstBindlessDescIndexOffset;
const int kInstBindlessBuffOOBBuffOffOffset = kInstBindlessCustomOffset_0;
const int kInstBindlessBuffOOBBuffSizeOffset = kInstBindlessCustomOffset_1;

// Non-Bindless OOB
// ---

const int kInstNonBindlessOOBDescSetOffset = kHeaderSize;
const int kInstNonBindlessOOBDescBindingOffset = kHeaderSize + 1;
const int kInstNonBindlessOOBDescIndexOffset = kHeaderSize + 2;
const int kInstNonBindlessOOBParamOffset0 = kHeaderSize + 3;
const int kInstNonBindlessOOBParamOffset1 = kHeaderSize + 4;

// Buffer device addresses
// ---
// A buffer address unalloc error will output the 64-bit pointer in
// two 32-bit pieces, lower bits first.
const int kInstBuffAddrUnallocDescPtrLoOffset = kHeaderSize;
const int kInstBuffAddrUnallocDescPtrHiOffset = kHeaderSize + 1;
const int kInstBuffAddrAccessByteSizeOffset = kHeaderSize + 2;
const int kInstBuffAddrAccessInstructionOffset = kHeaderSize + 3;

// Ray query
// ---
const int kInstRayQueryParamOffset_0 = kHeaderSize;

// Pre shaders
// ---
const int kPreActionParamOffset_0 = kHeaderSize;
const int kPreActionParamOffset_1 = kHeaderSize + 1;

// Sizes/Counts
// -------------------
const int kErrorRecordSize = kHeaderSize + 5;
const int kErrorRecordCounts = 4096;  // Maximum number of errors a command buffer can hold. Arbitrary value
const int kErrorBufferByteSize = 4 * kErrorRecordSize * kErrorRecordCounts + 2 * 4;  // 2 * 4 bytes to store flags and errors count

#ifdef __cplusplus
}  // namespace glsl
#endif

// DebugPrintf
// ---
const int kDebugPrintfOutputBufferSize = 0;
const int kDebugPrintfOutputBufferData = 1;

#ifdef __cplusplus
}  // namespace gpuav
#endif
#endif
