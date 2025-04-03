// Copyright (c) 2021-2025 The Khronos Group Inc.
// Copyright (c) 2021-2025 Valve Corporation
// Copyright (c) 2021-2025 LunarG, Inc.
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

#include "gpuav_error_codes.h"

#ifdef __cplusplus
namespace gpuav {
namespace glsl {
using uint = unsigned int;
#endif

// GPU-AV Error record structure:
// ------------------------------

// /---------------------------------
// | Error header
// |    - Record size
// |    - Shader Id
// |    - Instruction Id
// |    - Shader stage Id
// |    - Shader stage info (3 integers)
// |    - Action command index in command buffer
// |    - Command resources index
// |    - Error group (Id unique to the shader/instrumentation code that wrote the error)
// |    - subcode (maps to VUIDs)
// | --------------------------------
// | Error specific parameters
// \---------------------------------

// Error Header offsets:
// ---------------------
// The following are offsets to fields common to all records

// Each record first contains the size of the record in 32-bit words, including
// the size word.
const int kHeaderErrorRecordSizeOffset = 0;

// We compress the |shader id|, |error group| and |error sub code| in a single dword
// |shader id| is used to map back the VkShaderMoudle/Pipeline/ShaderObject
// |error group| maps to which instrumentation pass is from
// |error sub code| is how we can map things to a single VUID
const int kHeaderShaderIdErrorOffset = 1;

// This is the ordinal position of the instruction within the SPIR-V shader
// which generated the validation error.
//
// This also is the stage which generated the validation error.
const int kHeaderStageInstructionIdOffset = 2;

// Each stage will contain different values in the next set of words of the
// record used to identify which instantiation of the shader generated the
// validation error.
const int kHeaderStageInfoOffset_0 = 3;
const int kHeaderStageInfoOffset_1 = 4;
const int kHeaderStageInfoOffset_2 = 5;

// Compressed dword to know where the error came from in the API
const int kHeaderActionIdOffset = 6;

const int kHeaderSize = 7;

const int kInstLogErrorParameterOffset_0 = kHeaderSize;
const int kInstLogErrorParameterOffset_1 = kHeaderSize + 1;
const int kInstLogErrorParameterOffset_2 = kHeaderSize + 2;

// kHeaderShaderIdErrorOffset
// ---
// This dword is split up as
// | 31 ..... 24 | 23 ......... 18 | 17 ................. 0 |
// | Error Group | Error Sub Group | Instrumented Shader Id |
//
// These limits are reasonable and will be awhile until we go past them
// When moved to slang from GLSL, can add staic asserts
const int kErrorSubCodeShift = 18;
const int kErrorSubCodeMask = 0x3F << kErrorSubCodeShift;  // 32 slot
const int kErrorGroupShift = 24;
const int kErrorGroupMask = 0xFF << kErrorGroupShift;  // 256 slots

// kHeaderStageInstructionIdOffset
// ---
// This dword is split up as
// | 31 .. 27 | 26 ...... 0 |
// | Stage Id | Instruction Id |
// We control and know there are under 32 shader stages
// We can assume shader don't have 128 million lines of code in them
const int kStageIdShift = 27;
const int kStageIdMask = 0x1F << kStageIdShift;  // 32 slot
const int kInstructionIdMask = 0x7FFFFFF;

// kHeaderActionIdOffset
// ---
// This dword is split up as
// | 31 ..... 16 | 15 ................. 0 |
// | Error Group | Instrumented Shader Id |
// Note we have a limit (kMaxActionsPerCommandBuffer) but for simplicity, divide in half until find need to adjust.
const int kActionIdShift = 16;
const int kActionIdMask = 0xFFFF << kActionIdShift;  // 64k slot
const int kCommandResourceIdMask = 0xFFFF;

// Error specific parameters offsets:
// ----------------------------------

// Descriptor Indexing
// ---
const int kInstDescriptorIndexingSetAndIndexOffset = kHeaderSize;
const int kInstDescriptorIndexingParamOffset_0 = kHeaderSize + 1;
const int kInstDescriptorIndexingParamOffset_1 = kHeaderSize + 2;

// kInstDescriptorIndexingSetAndIndexOffset
// ---
// This dword is split up as
// | 31 ........ 27 | 26 ............................................ 0 |
// | Descriptor Set | Global Descriptor index (with binding LUT offset) |
// We only allow for 32 sets (see kDebugInputBindlessMaxDescSets)
// We use a LUT per set that knows which binding the "global" Descriptor Index value lives
const int kInstDescriptorIndexingSetShift = 27;
const int kInstDescriptorIndexingSetMask = 0x1F << kInstDescriptorIndexingSetShift;  // 32 slot
const int kInstDescriptorIndexingIndexMask = 0x7FFFFFF;

// Buffer device addresses
// ---
// Payload contains 3 pieces of data, compressed into a single uint32_t
// We can be safe that assume alignment we don't need these upper 2 bytes
// Note - if needed, we could use log2(alignment) as it must be a Power-of-Two
// |    31    |     30    | 29 ........................ 0 |
// | is write | is struct | Alignment OR Access Byte Size |
const uint kInstBuffAddrAccessPayloadShiftIsWrite = 31;
const uint kInstBuffAddrAccessPayloadShiftIsStruct = 30;
const uint kInstBuffAddrAccessPayloadMaskAccessInfo = 0x3FFFFFFF;

// Ray query
// ---
const int kInstRayQueryParamOffset_0 = kHeaderSize;

// Validation commands shaders
// ---
const int kPreActionParamOffset_0 = kHeaderSize;
const int kPreActionParamOffset_1 = kHeaderSize + 1;
const int kPreActionParamOffset_2 = kHeaderSize + 2;
const int kPreActionParamOffset_3 = kHeaderSize + 3;

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
const int kDebugPrintfOutputBufferDWordsCount = 0;
const int kDebugPrintfOutputBufferData = 1;

#ifdef __cplusplus
}  // namespace gpuav
#endif
#endif
