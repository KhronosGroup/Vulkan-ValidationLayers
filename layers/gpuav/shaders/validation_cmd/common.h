// Copyright (c) 2023-2026 The Khronos Group Inc.
// Copyright (c) 2023-2026 Valve Corporation
// Copyright (c) 2023-2026 LunarG, Inc.
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

#include "gpuav_error_header.h"
#include "gpuav_shaders_constants.h"

#ifdef __SLANG__

[vk::binding(kBindingDiagErrorBuffer, kDiagCommonDescriptorSet)] RWStructuredBuffer<uint> error_buffer;

[vk::binding(kBindingDiagActionIndex, kDiagCommonDescriptorSet)] StructuredBuffer<uint> action_index;

[vk::binding(kBindingDiagCmdResourceIndex, kDiagCommonDescriptorSet)] StructuredBuffer<uint> resource_index;

[vk::binding(kBindingDiagCmdErrorsCount, kDiagCommonDescriptorSet)] RWStructuredBuffer<uint> cmd_errors_count;

bool MaxCmdErrorsCountReached() {
    const uint cmd_id = resource_index[0];
    uint previous_errors_count = 0;
    InterlockedAdd(cmd_errors_count[cmd_id], 1, previous_errors_count);
    return previous_errors_count >= kMaxErrorsPerCmd;
}

void GpuavLogError5(uint error_group, uint error_sub_code, uint dword_0, uint dword_1, uint dword_2, uint dword_3, uint dword_4) {
    if (MaxCmdErrorsCountReached()) {
        return;
    }

    uint error_offset = 0;
    InterlockedAdd(error_buffer[error_buffer_used_size_member_offset], kErrorRecordDwordSize, error_offset);
    error_offset = error_offset + error_buffer_data_member_offset;

    const uint error_buffer_u32_size = error_buffer[error_buffer_u32_size_member_offset];
    const bool errors_buffer_filled = (error_offset + kErrorRecordDwordSize) > error_buffer_u32_size;
    if (errors_buffer_filled) {
        return;
    }

    error_buffer[error_offset + kHeader_ShaderIdErrorOffset] =
        (error_group << kErrorGroup_Shift) | (error_sub_code << kErrorSubCode_Shift);
    error_buffer[error_offset + kHeader_ErrorRecordSizeOffset] = kErrorRecordDwordSize;
    error_buffer[error_offset + kHeader_ActionIdErrorLoggerIdOffset] = (action_index[0] << kActionId_Shift) | resource_index[0];

    error_buffer[error_offset + kValCmd_ErrorPayloadDword_0] = dword_0;
    error_buffer[error_offset + kValCmd_ErrorPayloadDword_1] = dword_1;
    error_buffer[error_offset + kValCmd_ErrorPayloadDword_2] = dword_2;
    error_buffer[error_offset + kValCmd_ErrorPayloadDword_3] = dword_3;
    error_buffer[error_offset + kValCmd_ErrorPayloadDword_4] = dword_4;
}

#else

#extension GL_EXT_scalar_block_layout : require

layout(set = kDiagCommonDescriptorSet, binding = kBindingDiagErrorBuffer, scalar) buffer ErrorBuffer {
    uint flags;
    uint error_buffer_u32_size;
    uint errors_count;
    uint errors_buffer[];
};

layout(set = kDiagCommonDescriptorSet, binding = kBindingDiagActionIndex, scalar) readonly buffer ActionIndexBuffer {
    uint action_index[];
};

layout(set = kDiagCommonDescriptorSet, binding = kBindingDiagCmdResourceIndex, scalar) readonly buffer ResourceIndexBuffer {
    uint resource_index[];
};

layout(set = kDiagCommonDescriptorSet, binding = kBindingDiagCmdErrorsCount, scalar) buffer CmdErrorsCountBuffer {
    uint cmd_errors_count[];
};

bool MaxCmdErrorsCountReached() {
    const uint cmd_id = resource_index[0];
    const uint cmd_errors_count = atomicAdd(cmd_errors_count[cmd_id], 1);
    return cmd_errors_count >= kMaxErrorsPerCmd;
}

void GpuavLogError5(uint error_group, uint error_sub_code, uint dword_0, uint dword_1, uint dword_2, uint dword_3, uint dword_4) {
    if (MaxCmdErrorsCountReached()) {
        return;
    }

    uint error_offset = atomicAdd(errors_count, kErrorRecordDwordSize);
    const bool errors_buffer_filled =
        (error_offset + error_buffer_data_member_offset + kErrorRecordDwordSize) > error_buffer_u32_size;
    if (errors_buffer_filled) {
        return;
    }

    errors_buffer[error_offset + kHeader_ShaderIdErrorOffset] =
        (error_group << kErrorGroup_Shift) | (error_sub_code << kErrorSubCode_Shift);
    errors_buffer[error_offset + kHeader_ErrorRecordSizeOffset] = kErrorRecordDwordSize;
    errors_buffer[error_offset + kHeader_ActionIdErrorLoggerIdOffset] = (action_index[0] << kActionId_Shift) | resource_index[0];

    errors_buffer[error_offset + kValCmd_ErrorPayloadDword_0] = dword_0;
    errors_buffer[error_offset + kValCmd_ErrorPayloadDword_1] = dword_1;
    errors_buffer[error_offset + kValCmd_ErrorPayloadDword_2] = dword_2;
    errors_buffer[error_offset + kValCmd_ErrorPayloadDword_3] = dword_3;
    errors_buffer[error_offset + kValCmd_ErrorPayloadDword_4] = dword_4;
}

#endif

void GpuavLogError2(uint error_group, uint error_sub_code, uint dword_0, uint dword_1) {
    GpuavLogError5(error_group, error_sub_code, dword_0, dword_1, 0, 0, 0);
}

void GpuavLogError4(uint error_group, uint error_sub_code, uint dword_0, uint dword_1, uint dword_2, uint dword_3) {
    GpuavLogError5(error_group, error_sub_code, dword_0, dword_1, dword_2, dword_3, 0);
}
