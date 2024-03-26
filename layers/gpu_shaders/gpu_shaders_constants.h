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
#ifndef GPU_SHADERS_CONSTANTS_H
#define GPU_SHADERS_CONSTANTS_H

#ifdef __cplusplus
namespace gpuav {
namespace glsl {
using uint = unsigned int;

// Upper bound for maxUpdateAfterBindDescriptorsInAllPools. This value needs to
// be small enough to allow for a table in memory, but some devices set it to 2^32.
// This value only matters for host code, but it is defined here so it can be used
// in unit tests.
const uint kDebugInputBindlessMaxDescriptors = 1024u*1024u*4u;

#endif

// Maximum errors a cmd is allowed to log
const uint kMaxErrorsPerCmd = 6;

// Instrumentation
// ---

// Instead of having to create a variable and pass it in each time for every function call made, we use these values to map
// constants in the GLSL to be updated with constant values known when we are doing the linking at GPU-AV runtime. (Similar to
// Specialization Constant)
const uint kLinkShaderId = 0x0DEAD001;

// This is just a placeholder, honestly could be anything, will be replaced when linking to the runtime descriptor set choosen
const int kInstDefaultDescriptorSet = 7;

// Inside the descriptor set used by instrumentation validation,
// binding #0 is reserved for the output, but each check that requires additional input
// must reserve its own binding slot
const int kBindingInstErrorBuffer = 0;
const int kBindingInstBindlessDescriptor = 1;
const int kBindingInstBufferDeviceAddress = 2;
const int kBindingInstActionIndex = 3;
const int kBindingInstCmdResourceIndex = 4;
const int kBindingInstCmdErrorsCount = 5;

// Diagnostic calls
// ---

const int kDiagCommonDescriptorSet = 0;
const int kDiagPerCmdDescriptorSet = 1;

// Diagnostic calls bindings in common descriptor set
const int kBindingDiagErrorBuffer = 0;
const int kBindingDiagActionIndex = 1;
const int kBindingDiagCmdResourceIndex = 2;
const int kBindingDiagCmdErrorsCount = 3;

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

#ifdef __cplusplus
}  // namespace glsl
}  // namespace gpuav
#endif
#endif
