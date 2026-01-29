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
#ifndef GPU_ERROR_HEADER_H
#define GPU_ERROR_HEADER_H

#include "gpuav_error_codes.h"

#ifdef __cplusplus
#include <cstdint>
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
const int kHeader_ErrorRecordSizeOffset = 0;

// We compress the |shader id|, |error group| and |error sub code| in a single dword
// |shader id| is used to map back the VkShaderMoudle/Pipeline/ShaderObject
// |error group| maps to which instrumentation pass is from
// |error sub code| is how we can map things to a single VUID
const int kHeader_ShaderIdErrorOffset = 1;

// This is the ordinal position of the instruction within the SPIR-V shader
// which generated the validation error.
//
// This also is the stage which generated the validation error.
const int kHeader_StageInstructionIdOffset = 2;

// Each stage will contain different values in the next set of words of the
// record used to identify which instantiation of the shader generated the
// validation error.
const int kHeader_StageInfoOffset_0 = 3;
const int kHeader_StageInfoOffset_1 = 4;
const int kHeader_StageInfoOffset_2 = 5;

// Compressed dword to know where the error came from in the API
const int kHeader_ActionIdErrorLoggerIdOffset = 6;

const int kHeaderSize = 7;

const int kInst_LogError_ParameterOffset_0 = kHeaderSize;
const int kInst_LogError_ParameterOffset_1 = kHeaderSize + 1;
const int kInst_LogError_ParameterOffset_2 = kHeaderSize + 2;

// kHeader_ShaderIdErrorOffset
// ---
// This dword is split up as
// | 31 ..... 24 | 23 ......... 18 | 17 ................. 0 |
// | Error Group | Error Sub Group | Instrumented Shader Id |
//
// These limits are reasonable and will be awhile until we go past them
// When moved to slang from GLSL, can add staic asserts
const int kErrorSubCode_Shift = 18;
const int kErrorSubCode_Mask = 0x3F << kErrorSubCode_Shift;  // 32 slot
const int kErrorGroup_Shift = 24;
const int kErrorGroup_Mask = 0xFF << kErrorGroup_Shift;  // 256 slots
const uint kMaxInstrumentedShaders = 1u << kErrorSubCode_Shift;  // 256k
const uint kShaderIdMask = 0x3FFFF;                              // Used to get unique_shader_id

// kHeader_StageInstructionIdOffset
// ---
// This dword is split up as
// | 31 .. 27 | 26 ...... 0 |
// | Stage Id | Instruction Id |
// We control and know there are under 32 shader stages
// We can assume shader are under 128MB
const int kStageId_Shift = 27;
const int kStageId_Mask = 0x1F << kStageId_Shift;  // 32 slot
const int kInstructionId_Mask = 0x7FFFFFF;

// kHeader_ActionIdErrorLoggerIdOffset
// ---
// This dword is split up as
// | 31 ..... 16 | 15 ................. 0 |
// | Error Group | Instrumented Shader Id |
// Note we have a limit (GpuAVSettings::indices_count) but for simplicity, divide in half until find need to adjust.
const int kActionId_Shift = 16;
const int kActionId_Mask = 0xFFFF << kActionId_Shift;  // 64k slot
const int kErrorLoggerId_Mask = 0xFFFF;

// Error specific parameters offsets:
// ----------------------------------

// Descriptor Indexing
// ---
// This dword is split up as
// | 31 ........ 27 | 26 ............................................ 0 |
// | Descriptor Set | Global Descriptor index (with binding LUT offset) |
// We only allow for 32 sets (see kDebugInputBindlessMaxDescSets)
// We use a LUT per set that knows which binding the "global" Descriptor Index value lives
const int kInst_DescriptorIndexing_SetShift = 27;
const int kInst_DescriptorIndexing_SetMask = 0x1F << kInst_DescriptorIndexing_SetShift;  // 32 slot
const int kInst_DescriptorIndexing_IndexMask = 0x7FFFFFF;

// Buffer device addresses
// ---
// Payload contains 3 pieces of data, compressed into a single uint32_t
// We can be safe that assume alignment we don't need these upper 2 bytes
// Note - if needed, we could use log2(alignment) as it must be a Power-of-Two
// |    31    |     30    | 29 ........................ 0 |
// | is write | is struct | Alignment OR Access Byte Size |
const uint kInst_BuffAddrAccess_PayloadShiftIsWrite = 31;
const uint kInst_BuffAddrAccess_PayloadShiftIsStruct = 30;
const uint kInst_BuffAddrAccess_PayloadMaskAccessInfo = 0x3FFFFFFF;

// kErrorGroup_InstMeshShading
// ---
// This dword is split up as
// | 31 ........ 16 | 15 .............. 0 |
// | OutputVertices | OutputPrimitivesEXT |
// This is because the limits for both of these on all known devices is 1024
const int kInst_MeshShading_OutputVerticesShift = 16;
const int kInst_MeshShading_OutputPrimitivesMask = 0xFFFF;

// Validation commands shaders
// ---
const int kValCmd_ErrorPayloadDword_0 = kHeaderSize;
const int kValCmd_ErrorPayloadDword_1 = kHeaderSize + 1;
const int kValCmd_ErrorPayloadDword_2 = kHeaderSize + 2;
const int kValCmd_ErrorPayloadDword_3 = kHeaderSize + 3;
const int kValCmd_ErrorPayloadDword_4 = kHeaderSize + 4;

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
const int kDebugPrintf_OutputBuffer_DWordsCount = 0;
const int kDebugPrintf_OutputBuffer_Data = 1;

#ifdef __cplusplus

[[maybe_unused]] static inline uint32_t GetErrorGroup(const uint32_t* error_record) {
    return error_record[glsl::kHeader_ShaderIdErrorOffset] >> glsl::kErrorGroup_Shift;
}

[[maybe_unused]] static inline uint32_t GetSubError(const uint32_t* error_record) {
    return (error_record[glsl::kHeader_ShaderIdErrorOffset] & glsl::kErrorSubCode_Mask) >> glsl::kErrorSubCode_Shift;
}

}  // namespace gpuav
#endif
#endif
