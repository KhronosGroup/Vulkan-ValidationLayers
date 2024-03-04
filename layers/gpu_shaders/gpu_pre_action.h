// Copyright (c) 2023-2024 The Khronos Group Inc.
// Copyright (c) 2023-2024 Valve Corporation
// Copyright (c) 2023-2024 LunarG, Inc.
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

#include "gpu_error_header.h"
#include "gpu_shaders_constants.h"

#define ERROR_RECORD_WORDS_COUNT kHeaderErrorGroupOffset + 4

layout(set = 0, binding = 0) buffer OutputBuffer {
    uint flags;
    uint output_buffer_count;
    uint output_buffer[];
};

void gpuavLogError(uint error_group, uint error_sub_code, uint param_0, uint param_1) {
    uint vo_idx = atomicAdd(output_buffer_count, ERROR_RECORD_WORDS_COUNT);
    if (vo_idx + ERROR_RECORD_WORDS_COUNT > output_buffer.length()) return;

    output_buffer[vo_idx + kHeaderErrorGroupOffset] = error_group;
    output_buffer[vo_idx + kHeaderErrorSubCodeOffset] = error_sub_code;
    output_buffer[vo_idx + kPreActionParamOffset_0] = param_0;
    output_buffer[vo_idx + kPreActionParamOffset_1] = param_1;
}
