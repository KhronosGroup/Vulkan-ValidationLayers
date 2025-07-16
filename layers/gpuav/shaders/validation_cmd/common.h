// Copyright (c) 2023-2025 The Khronos Group Inc.
// Copyright (c) 2023-2025 Valve Corporation
// Copyright (c) 2023-2025 LunarG, Inc.
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
#include "root_node.h"

bool MaxCmdErrorsCountReached() {
    const uint cmd_id = root_node.inst_cmd_resource_index_buffer.index[0];
    const uint cmd_errors_count = atomicAdd(root_node.inst_cmd_errors_count_buffer.errors_count[cmd_id], 1);
    return cmd_errors_count >= kMaxErrorsPerCmd;
}

void GpuavLogError4(uint error_group, uint error_sub_code, uint param_0, uint param_1, uint param_2, uint param_3) {
    if (MaxCmdErrorsCountReached()) {
        return;
    }

    uint vo_idx = atomicAdd(root_node.inst_errors_buffer.written_count, kErrorRecordSize);
    const bool errors_buffer_filled = (vo_idx + kErrorRecordSize) > uint(root_node.inst_errors_buffer.size);
    if (errors_buffer_filled) {
        return;
    }

    root_node.inst_errors_buffer.data[vo_idx + kHeaderShaderIdErrorOffset] =
        (error_group << kErrorGroupShift) | (error_sub_code << kErrorSubCodeShift);
    root_node.inst_errors_buffer.data[vo_idx + kHeaderErrorRecordSizeOffset] = kErrorRecordSize;
    root_node.inst_errors_buffer.data[vo_idx + kHeaderActionIdOffset] =
        (root_node.inst_action_index_buffer.index[0] << kActionIdShift) | root_node.inst_cmd_resource_index_buffer.index[0];

    root_node.inst_errors_buffer.data[vo_idx + kPreActionParamOffset_0] = param_0;
    root_node.inst_errors_buffer.data[vo_idx + kPreActionParamOffset_1] = param_1;
    root_node.inst_errors_buffer.data[vo_idx + kPreActionParamOffset_2] = param_2;
    root_node.inst_errors_buffer.data[vo_idx + kPreActionParamOffset_3] = param_3;
}

void GpuavLogError2(uint error_group, uint error_sub_code, uint param_0, uint param_1) {
    GpuavLogError4(error_group, error_sub_code, param_0, param_1, 0, 0);
}
