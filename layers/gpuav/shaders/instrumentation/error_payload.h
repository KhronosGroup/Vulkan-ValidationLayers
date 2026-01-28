// Copyright (c) 2025-2026 The Khronos Group Inc.
// Copyright (c) 2025-2026 Valve Corporation
// Copyright (c) 2025-2026 LunarG, Inc.
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

// This is in a dedicated file instead of common_descriptor_sets.h until we get every shader to use this.
// (We don't want to link this in if not required)

// TODO - Would be nice to get this down to a only 16 bytes as per-thread memory is VERY precious for runtime perf
struct ErrorPayload {
    uint inst_offset;            // kHeader_StageInstructionIdOffset
    uint shader_error_encoding;  // kHeader_ShaderIdErrorOffset
    uint parameter_0;            // kInst_LogError_ParameterOffset_0
    uint parameter_1;            // kInst_LogError_ParameterOffset_1
    uint parameter_2;            // kInst_LogError_ParameterOffset_2
} error_payload;