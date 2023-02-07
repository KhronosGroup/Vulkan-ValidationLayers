// Copyright (c) 2021-2022 The Khronos Group Inc.
// Copyright (c) 2021-2023 Valve Corporation
// Copyright (c) 2021-2023 LunarG, Inc.
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

#ifndef GPU_SHADER_CONSTANTS
#define GPU_SHADER_CONSTANTS

// values match those found in SPIRV-Tools instrument.hpp file.
#define _kInstErrorMax 7
#define _kInstValidationOutError 7
// The values in instrument.hpp are for the spirv-opt pass but these values are for the
// internal gpu_shaders in the VVL. GLSL can't understand .hpp header file so these
// are defined internally here extending the max values
#define _kInstErrorPreDrawValidate _kInstErrorMax + 1
#define _kInstErrorPreDispatchValidate _kInstErrorMax + 2
#define _kPreValidateSubError _kInstValidationOutError + 1

// These values all share the byte at (_kPreValidateSubError + 1) location since only
// one will be used at a time. Also equivalent to (kInstStageOutCnt + 1)
// debug buffer is memset to 0 so need to start at index 1
#define pre_draw_count_exceeds_bufsize_error 1
#define pre_draw_count_exceeds_limit_error 2
#define pre_draw_first_instance_error 3
#define pre_dispatch_count_exceeds_limit_x_error 1
#define pre_dispatch_count_exceeds_limit_y_error 2
#define pre_dispatch_count_exceeds_limit_z_error 3

#endif
