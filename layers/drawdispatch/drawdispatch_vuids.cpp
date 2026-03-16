/* Copyright (c) 2015-2026 The Khronos Group Inc.
 * Copyright (c) 2015-2026 Valve Corporation
 * Copyright (c) 2015-2026 LunarG, Inc.
 * Copyright (C) 2015-2026 Google Inc.
 * Copyright (c) 2025 Arm Limited.
 * Modifications Copyright (C) 2020-2026 Advanced Micro Devices, Inc. All rights reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "drawdispatch_vuids.h"
#include "error_message/logging.h"

namespace vvl {
// clang-format off
struct DispatchVuidsCmdDraw : DrawDispatchVuid {
    DispatchVuidsCmdDraw() : DrawDispatchVuid(Func::vkCmdDraw) {
        pipeline_bound_08606                     = "VUID-vkCmdDraw-None-08606";
        compatible_pipeline_08600                = "VUID-vkCmdDraw-None-08600";
        vertex_binding_attribute_02721           = "VUID-vkCmdDraw-None-02721";
        unprotected_command_buffer_02707         = "VUID-vkCmdDraw-commandBuffer-02707";
        protected_command_buffer_02712           = "VUID-vkCmdDraw-commandBuffer-02712";
        descriptor_buffer_bit_set_08114          = "VUID-vkCmdDraw-None-08114";
        descriptor_buffer_bit_not_set_08115      = "VUID-vkCmdDraw-None-08115";
        descriptor_buffer_set_offset_missing_08117 = "VUID-vkCmdDraw-None-08117";
        tensorARM_pDescription_09900             = "VUID-vkCmdDraw-pDescription-09900";
        tensorARM_dimensionCount_09905           = "VUID-vkCmdDraw-dimensionCount-09905";
    }
};

struct DispatchVuidsCmdDrawMultiEXT : DrawDispatchVuid {
    DispatchVuidsCmdDrawMultiEXT() : DrawDispatchVuid(Func::vkCmdDrawMultiEXT) {
        pipeline_bound_08606                     = "VUID-vkCmdDrawMultiEXT-None-08606";
        compatible_pipeline_08600                = "VUID-vkCmdDrawMultiEXT-None-08600";
        vertex_binding_attribute_02721           = "VUID-vkCmdDrawMultiEXT-None-02721";
        unprotected_command_buffer_02707         = "VUID-vkCmdDrawMultiEXT-commandBuffer-02707";
        protected_command_buffer_02712           = "VUID-vkCmdDrawMultiEXT-commandBuffer-02712";
        descriptor_buffer_bit_set_08114          = "VUID-vkCmdDrawMultiEXT-None-08114";
        descriptor_buffer_bit_not_set_08115      = "VUID-vkCmdDrawMultiEXT-None-08115";
        descriptor_buffer_set_offset_missing_08117 = "VUID-vkCmdDrawMultiEXT-None-08117";
        tensorARM_pDescription_09900             = "VUID-vkCmdDrawMultiEXT-pDescription-09900";
        tensorARM_dimensionCount_09905           = "VUID-vkCmdDrawMultiEXT-dimensionCount-09905";
    }
};

struct DispatchVuidsCmdDrawIndexed : DrawDispatchVuid {
    DispatchVuidsCmdDrawIndexed() : DrawDispatchVuid(Func::vkCmdDrawIndexed) {
        pipeline_bound_08606                     = "VUID-vkCmdDrawIndexed-None-08606";
        index_binding_07312                      = "VUID-vkCmdDrawIndexed-None-07312";
        compatible_pipeline_08600                = "VUID-vkCmdDrawIndexed-None-08600";
        vertex_binding_attribute_02721           = "VUID-vkCmdDrawIndexed-None-02721";
        unprotected_command_buffer_02707         = "VUID-vkCmdDrawIndexed-commandBuffer-02707";
        protected_command_buffer_02712           = "VUID-vkCmdDrawIndexed-commandBuffer-02712";
        descriptor_buffer_bit_set_08114          = "VUID-vkCmdDrawIndexed-None-08114";
        descriptor_buffer_bit_not_set_08115      = "VUID-vkCmdDrawIndexed-None-08115";
        descriptor_buffer_set_offset_missing_08117 = "VUID-vkCmdDrawIndexed-None-08117";
        tensorARM_pDescription_09900             = "VUID-vkCmdDrawIndexed-pDescription-09900";
        tensorARM_dimensionCount_09905           = "VUID-vkCmdDrawIndexed-dimensionCount-09905";
    }
};

struct DispatchVuidsCmdDrawMultiIndexedEXT : DrawDispatchVuid {
    DispatchVuidsCmdDrawMultiIndexedEXT() : DrawDispatchVuid(Func::vkCmdDrawMultiIndexedEXT) {
        pipeline_bound_08606                     = "VUID-vkCmdDrawMultiIndexedEXT-None-08606";
        index_binding_07312                      = "VUID-vkCmdDrawMultiIndexedEXT-None-07312";
        compatible_pipeline_08600                = "VUID-vkCmdDrawMultiIndexedEXT-None-08600";
        vertex_binding_attribute_02721           = "VUID-vkCmdDrawMultiIndexedEXT-None-02721";
        unprotected_command_buffer_02707         = "VUID-vkCmdDrawMultiIndexedEXT-commandBuffer-02707";
        protected_command_buffer_02712           = "VUID-vkCmdDrawMultiIndexedEXT-commandBuffer-02712";
        descriptor_buffer_bit_set_08114          = "VUID-vkCmdDrawMultiIndexedEXT-None-08114";
        descriptor_buffer_bit_not_set_08115      = "VUID-vkCmdDrawMultiIndexedEXT-None-08115";
        descriptor_buffer_set_offset_missing_08117 = "VUID-vkCmdDrawMultiIndexedEXT-None-08117";
        tensorARM_pDescription_09900             = "VUID-vkCmdDrawMultiIndexedEXT-pDescription-09900";
        tensorARM_dimensionCount_09905           = "VUID-vkCmdDrawMultiIndexedEXT-dimensionCount-09905";
    }
};

struct DispatchVuidsCmdDrawIndirect : DrawDispatchVuid {
    DispatchVuidsCmdDrawIndirect() : DrawDispatchVuid(Func::vkCmdDrawIndirect) {
        pipeline_bound_08606                     = "VUID-vkCmdDrawIndirect-None-08606";
        compatible_pipeline_08600                = "VUID-vkCmdDrawIndirect-None-08600";
        indirect_protected_cb_02711              = "VUID-vkCmdDrawIndirect-commandBuffer-02711";
        indirect_contiguous_memory_02708         = "VUID-vkCmdDrawIndirect-buffer-02708";
        indirect_buffer_bit_02290                = "VUID-vkCmdDrawIndirect-buffer-02709";
        vertex_binding_attribute_02721           = "VUID-vkCmdDrawIndirect-None-02721";
        unprotected_command_buffer_02707         = "VUID-vkCmdDrawIndirect-commandBuffer-02707";
        descriptor_buffer_bit_set_08114          = "VUID-vkCmdDrawIndirect-None-08114";
        descriptor_buffer_bit_not_set_08115      = "VUID-vkCmdDrawIndirect-None-08115";
        descriptor_buffer_set_offset_missing_08117 = "VUID-vkCmdDrawIndirect-None-08117";
        tensorARM_pDescription_09900             = "VUID-vkCmdDrawIndirect-pDescription-09900";
        tensorARM_dimensionCount_09905           = "VUID-vkCmdDrawIndirect-dimensionCount-09905";
    }
};

struct DispatchVuidsCmdDrawIndexedIndirect : DrawDispatchVuid {
    DispatchVuidsCmdDrawIndexedIndirect() : DrawDispatchVuid(Func::vkCmdDrawIndexedIndirect) {
        pipeline_bound_08606                     = "VUID-vkCmdDrawIndexedIndirect-None-08606";
        index_binding_07312                      = "VUID-vkCmdDrawIndexedIndirect-None-07312";
        compatible_pipeline_08600                = "VUID-vkCmdDrawIndexedIndirect-None-08600";
        indirect_protected_cb_02711              = "VUID-vkCmdDrawIndexedIndirect-commandBuffer-02711";
        indirect_contiguous_memory_02708         = "VUID-vkCmdDrawIndexedIndirect-buffer-02708";
        indirect_buffer_bit_02290                = "VUID-vkCmdDrawIndexedIndirect-buffer-02709";
        vertex_binding_attribute_02721           = "VUID-vkCmdDrawIndexedIndirect-None-02721";
        unprotected_command_buffer_02707         = "VUID-vkCmdDrawIndexedIndirect-commandBuffer-02707";
        descriptor_buffer_bit_set_08114          = "VUID-vkCmdDrawIndexedIndirect-None-08114";
        descriptor_buffer_bit_not_set_08115      = "VUID-vkCmdDrawIndexedIndirect-None-08115";
        descriptor_buffer_set_offset_missing_08117 = "VUID-vkCmdDrawIndexedIndirect-None-08117";
        tensorARM_pDescription_09900             = "VUID-vkCmdDrawIndexedIndirect-pDescription-09900";
        tensorARM_dimensionCount_09905           = "VUID-vkCmdDrawIndexedIndirect-dimensionCount-09905";
    }
};

struct DispatchVuidsCmdDispatch : DrawDispatchVuid {
    DispatchVuidsCmdDispatch() : DrawDispatchVuid(Func::vkCmdDispatch) {
        pipeline_bound_08606                     = "VUID-vkCmdDispatch-None-08606";
        compatible_pipeline_08600                = "VUID-vkCmdDispatch-None-08600";
        unprotected_command_buffer_02707         = "VUID-vkCmdDispatch-commandBuffer-02707";
        protected_command_buffer_02712           = "VUID-vkCmdDispatch-commandBuffer-02712";
        descriptor_buffer_bit_set_08114          = "VUID-vkCmdDispatch-None-08114";
        descriptor_buffer_bit_not_set_08115      = "VUID-vkCmdDispatch-None-08115";
        descriptor_buffer_set_offset_missing_08117 = "VUID-vkCmdDispatch-None-08117";
        tensorARM_pDescription_09900             = "VUID-vkCmdDispatch-pDescription-09900";
        tensorARM_dimensionCount_09905           = "VUID-vkCmdDispatch-dimensionCount-09905";
    }
};

struct DispatchVuidsCmdDispatchIndirect : DrawDispatchVuid {
    DispatchVuidsCmdDispatchIndirect() : DrawDispatchVuid(Func::vkCmdDispatchIndirect) {
        pipeline_bound_08606                     = "VUID-vkCmdDispatchIndirect-None-08606";
        compatible_pipeline_08600                = "VUID-vkCmdDispatchIndirect-None-08600";
        indirect_protected_cb_02711              = "VUID-vkCmdDispatchIndirect-commandBuffer-02711";
        indirect_contiguous_memory_02708         = "VUID-vkCmdDispatchIndirect-buffer-02708";
        indirect_buffer_bit_02290                = "VUID-vkCmdDispatchIndirect-buffer-02709";
        unprotected_command_buffer_02707         = "VUID-vkCmdDispatchIndirect-commandBuffer-02707";
        descriptor_buffer_bit_set_08114          = "VUID-vkCmdDispatchIndirect-None-08114";
        descriptor_buffer_bit_not_set_08115      = "VUID-vkCmdDispatchIndirect-None-08115";
        descriptor_buffer_set_offset_missing_08117 = "VUID-vkCmdDispatchIndirect-None-08117";
        tensorARM_pDescription_09900             = "VUID-vkCmdDispatchIndirect-pDescription-09900";
        tensorARM_dimensionCount_09905           = "VUID-vkCmdDispatchIndirect-dimensionCount-09905";
    }
};

struct DispatchVuidsCmdDrawIndirectCount : DrawDispatchVuid {
    DispatchVuidsCmdDrawIndirectCount() : DrawDispatchVuid(Func::vkCmdDrawIndirectCount) {
        pipeline_bound_08606                     = "VUID-vkCmdDrawIndirectCount-None-08606";
        compatible_pipeline_08600                = "VUID-vkCmdDrawIndirectCount-None-08600";
        indirect_protected_cb_02711              = "VUID-vkCmdDrawIndirectCount-commandBuffer-02711";
        indirect_contiguous_memory_02708         = "VUID-vkCmdDrawIndirectCount-buffer-02708";
        indirect_count_contiguous_memory_02714   = "VUID-vkCmdDrawIndirectCount-countBuffer-02714";
        indirect_buffer_bit_02290                = "VUID-vkCmdDrawIndirectCount-buffer-02709";
        indirect_count_buffer_bit_02715          = "VUID-vkCmdDrawIndirectCount-countBuffer-02715";
        indirect_count_offset_04129              = "VUID-vkCmdDrawIndirectCount-countBufferOffset-04129";
        vertex_binding_attribute_02721           = "VUID-vkCmdDrawIndirectCount-None-02721";
        unprotected_command_buffer_02707         = "VUID-vkCmdDrawIndirectCount-commandBuffer-02707";
        descriptor_buffer_bit_set_08114          = "VUID-vkCmdDrawIndirectCount-None-08114";
        descriptor_buffer_bit_not_set_08115      = "VUID-vkCmdDrawIndirectCount-None-08115";
        descriptor_buffer_set_offset_missing_08117 = "VUID-vkCmdDrawIndirectCount-None-08117";
        tensorARM_pDescription_09900             = "VUID-vkCmdDrawIndirectCount-pDescription-09900";
        tensorARM_dimensionCount_09905           = "VUID-vkCmdDrawIndirectCount-dimensionCount-09905";
    }
};

struct DispatchVuidsCmdDrawIndexedIndirectCount : DrawDispatchVuid {
    DispatchVuidsCmdDrawIndexedIndirectCount() : DrawDispatchVuid(Func::vkCmdDrawIndexedIndirectCount) {
        pipeline_bound_08606                     = "VUID-vkCmdDrawIndexedIndirectCount-None-08606";
        index_binding_07312                      = "VUID-vkCmdDrawIndexedIndirectCount-None-07312";
        compatible_pipeline_08600                = "VUID-vkCmdDrawIndexedIndirectCount-None-08600";
        indirect_protected_cb_02711              = "VUID-vkCmdDrawIndexedIndirectCount-commandBuffer-02711";
        indirect_contiguous_memory_02708         = "VUID-vkCmdDrawIndexedIndirectCount-buffer-02708";
        indirect_count_contiguous_memory_02714   = "VUID-vkCmdDrawIndexedIndirectCount-countBuffer-02714";
        indirect_buffer_bit_02290                = "VUID-vkCmdDrawIndexedIndirectCount-buffer-02709";
        indirect_count_buffer_bit_02715          = "VUID-vkCmdDrawIndexedIndirectCount-countBuffer-02715";
        indirect_count_offset_04129              = "VUID-vkCmdDrawIndexedIndirectCount-countBufferOffset-04129";
        vertex_binding_attribute_02721           = "VUID-vkCmdDrawIndexedIndirectCount-None-02721";
        unprotected_command_buffer_02707         = "VUID-vkCmdDrawIndexedIndirectCount-commandBuffer-02707";
        descriptor_buffer_bit_set_08114          = "VUID-vkCmdDrawIndexedIndirectCount-None-08114";
        descriptor_buffer_bit_not_set_08115      = "VUID-vkCmdDrawIndexedIndirectCount-None-08115";
        descriptor_buffer_set_offset_missing_08117 = "VUID-vkCmdDrawIndexedIndirectCount-None-08117";
        tensorARM_pDescription_09900             = "VUID-vkCmdDrawIndexedIndirectCount-pDescription-09900";
        tensorARM_dimensionCount_09905           = "VUID-vkCmdDrawIndexedIndirectCount-dimensionCount-09905";
    }
};

struct DispatchVuidsCmdTraceRaysNV: DrawDispatchVuid {
    DispatchVuidsCmdTraceRaysNV() : DrawDispatchVuid(Func::vkCmdTraceRaysNV) {
        pipeline_bound_08606                     = "VUID-vkCmdTraceRaysNV-None-08606";
        compatible_pipeline_08600                = "VUID-vkCmdTraceRaysNV-None-08600";
        unprotected_command_buffer_02707         = "VUID-vkCmdTraceRaysNV-commandBuffer-02707";
        descriptor_buffer_bit_set_08114          = "VUID-vkCmdTraceRaysNV-None-08114";
        descriptor_buffer_bit_not_set_08115      = "VUID-vkCmdTraceRaysNV-None-08115";
        descriptor_buffer_set_offset_missing_08117 = "VUID-vkCmdTraceRaysNV-None-08117";
        tensorARM_pDescription_09900             = "VUID-vkCmdTraceRaysNV-pDescription-09900";
        tensorARM_dimensionCount_09905           = "VUID-vkCmdTraceRaysNV-dimensionCount-09905";
    }
};

struct DispatchVuidsCmdTraceRaysKHR: DrawDispatchVuid {
    DispatchVuidsCmdTraceRaysKHR() : DrawDispatchVuid(Func::vkCmdTraceRaysKHR) {
        pipeline_bound_08606                     = "VUID-vkCmdTraceRaysKHR-None-08606";
        compatible_pipeline_08600                = "VUID-vkCmdTraceRaysKHR-None-08600";
        unprotected_command_buffer_02707         = "VUID-vkCmdTraceRaysKHR-commandBuffer-02707";
        descriptor_buffer_bit_set_08114          = "VUID-vkCmdTraceRaysKHR-None-08114";
        descriptor_buffer_bit_not_set_08115      = "VUID-vkCmdTraceRaysKHR-None-08115";
        descriptor_buffer_set_offset_missing_08117 = "VUID-vkCmdTraceRaysKHR-None-08117";
        tensorARM_pDescription_09900             = "VUID-vkCmdTraceRaysKHR-pDescription-09900";
        tensorARM_dimensionCount_09905           = "VUID-vkCmdTraceRaysKHR-dimensionCount-09905";
    }
};

struct DispatchVuidsCmdTraceRaysIndirectKHR: DrawDispatchVuid {
    DispatchVuidsCmdTraceRaysIndirectKHR() : DrawDispatchVuid(Func::vkCmdTraceRaysIndirectKHR) {
        pipeline_bound_08606                     = "VUID-vkCmdTraceRaysIndirectKHR-None-08606";
        compatible_pipeline_08600                = "VUID-vkCmdTraceRaysIndirectKHR-None-08600";
        unprotected_command_buffer_02707         = "VUID-vkCmdTraceRaysIndirectKHR-commandBuffer-02707";
        descriptor_buffer_bit_set_08114          = "VUID-vkCmdTraceRaysIndirectKHR-None-08114";
        descriptor_buffer_bit_not_set_08115      = "VUID-vkCmdTraceRaysIndirectKHR-None-08115";
        descriptor_buffer_set_offset_missing_08117 = "VUID-vkCmdTraceRaysIndirectKHR-None-08117";
        tensorARM_pDescription_09900             = "VUID-vkCmdTraceRaysIndirectKHR-pDescription-09900";
        tensorARM_dimensionCount_09905           = "VUID-vkCmdTraceRaysIndirectKHR-dimensionCount-09905";
    }
};

struct DispatchVuidsCmdTraceRaysIndirect2KHR: DrawDispatchVuid {
    DispatchVuidsCmdTraceRaysIndirect2KHR() : DrawDispatchVuid(Func::vkCmdTraceRaysIndirect2KHR) {
        pipeline_bound_08606                     = "VUID-vkCmdTraceRaysIndirect2KHR-None-08606";
        compatible_pipeline_08600                = "VUID-vkCmdTraceRaysIndirect2KHR-None-08600";
        unprotected_command_buffer_02707         = "VUID-vkCmdTraceRaysIndirect2KHR-commandBuffer-02707";
        descriptor_buffer_bit_set_08114          = "VUID-vkCmdTraceRaysIndirect2KHR-None-08114";
        descriptor_buffer_bit_not_set_08115      = "VUID-vkCmdTraceRaysIndirect2KHR-None-08115";
        descriptor_buffer_set_offset_missing_08117 = "VUID-vkCmdTraceRaysIndirect2KHR-None-08117";
        tensorARM_pDescription_09900             = "VUID-vkCmdTraceRaysIndirect2KHR-pDescription-09900";
        tensorARM_dimensionCount_09905           = "VUID-vkCmdTraceRaysIndirect2KHR-dimensionCount-09905";
    }
};

struct DispatchVuidsCmdDrawMeshTasksNV: DrawDispatchVuid {
    DispatchVuidsCmdDrawMeshTasksNV() : DrawDispatchVuid(Func::vkCmdDrawMeshTasksNV) {
        pipeline_bound_08606                     = "VUID-vkCmdDrawMeshTasksNV-None-08606";
        compatible_pipeline_08600                = "VUID-vkCmdDrawMeshTasksNV-None-08600";
        unprotected_command_buffer_02707         = "VUID-vkCmdDrawMeshTasksNV-commandBuffer-02707";
        descriptor_buffer_bit_set_08114          = "VUID-vkCmdDrawMeshTasksNV-None-08114";
        descriptor_buffer_bit_not_set_08115      = "VUID-vkCmdDrawMeshTasksNV-None-08115";
        descriptor_buffer_set_offset_missing_08117 = "VUID-vkCmdDrawMeshTasksNV-None-08117";
        tensorARM_pDescription_09900             = "VUID-vkCmdDrawMeshTasksNV-pDescription-09900";
        tensorARM_dimensionCount_09905           = "VUID-vkCmdDrawMeshTasksNV-dimensionCount-09905";
    }
};

struct DispatchVuidsCmdDrawMeshTasksIndirectNV: DrawDispatchVuid {
    DispatchVuidsCmdDrawMeshTasksIndirectNV() : DrawDispatchVuid(Func::vkCmdDrawMeshTasksIndirectNV) {
        pipeline_bound_08606                     = "VUID-vkCmdDrawMeshTasksIndirectNV-None-08606";
        compatible_pipeline_08600                = "VUID-vkCmdDrawMeshTasksIndirectNV-None-08600";
        indirect_protected_cb_02711              = "VUID-vkCmdDrawMeshTasksIndirectNV-commandBuffer-02711";
        indirect_contiguous_memory_02708         = "VUID-vkCmdDrawMeshTasksIndirectNV-buffer-02708";
        indirect_buffer_bit_02290                = "VUID-vkCmdDrawMeshTasksIndirectNV-buffer-02709";
        unprotected_command_buffer_02707         = "VUID-vkCmdDrawMeshTasksIndirectNV-commandBuffer-02707";
        descriptor_buffer_bit_set_08114          = "VUID-vkCmdDrawMeshTasksIndirectNV-None-08114";
        descriptor_buffer_bit_not_set_08115      = "VUID-vkCmdDrawMeshTasksIndirectNV-None-08115";
        descriptor_buffer_set_offset_missing_08117 = "VUID-vkCmdDrawMeshTasksIndirectNV-None-08117";
        tensorARM_dimensionCount_09905           = "VUID-vkCmdDrawMeshTasksIndirectNV-dimensionCount-09905";
    }
};

struct DispatchVuidsCmdDrawMeshTasksIndirectCountNV : DrawDispatchVuid {
    DispatchVuidsCmdDrawMeshTasksIndirectCountNV() : DrawDispatchVuid(Func::vkCmdDrawMeshTasksIndirectCountNV) {
        pipeline_bound_08606                     = "VUID-vkCmdDrawMeshTasksIndirectCountNV-None-08606";
        compatible_pipeline_08600                = "VUID-vkCmdDrawMeshTasksIndirectCountNV-None-08600";
        indirect_protected_cb_02711              = "VUID-vkCmdDrawMeshTasksIndirectCountNV-commandBuffer-02711";
        indirect_contiguous_memory_02708         = "VUID-vkCmdDrawMeshTasksIndirectCountNV-buffer-02708";
        indirect_buffer_bit_02290                = "VUID-vkCmdDrawMeshTasksIndirectCountNV-buffer-02709";
        indirect_count_contiguous_memory_02714   = "VUID-vkCmdDrawMeshTasksIndirectCountNV-countBuffer-02714";
        indirect_count_buffer_bit_02715          = "VUID-vkCmdDrawMeshTasksIndirectCountNV-countBuffer-02715";
        indirect_count_offset_04129              = "VUID-vkCmdDrawMeshTasksIndirectCountNV-countBufferOffset-04129";
        unprotected_command_buffer_02707         = "VUID-vkCmdDrawMeshTasksIndirectCountNV-commandBuffer-02707";
        descriptor_buffer_bit_set_08114          = "VUID-vkCmdDrawMeshTasksIndirectCountNV-None-08114";
        descriptor_buffer_bit_not_set_08115      = "VUID-vkCmdDrawMeshTasksIndirectCountNV-None-08115";
        descriptor_buffer_set_offset_missing_08117 = "VUID-vkCmdDrawMeshTasksIndirectCountNV-None-08117";
        tensorARM_pDescription_09900             = "VUID-vkCmdDrawMeshTasksIndirectCountNV-pDescription-09900";
        tensorARM_dimensionCount_09905           = "VUID-vkCmdDrawMeshTasksIndirectCountNV-dimensionCount-09905";
    }
};

struct DispatchVuidsCmdDrawMeshTasksEXT: DrawDispatchVuid {
    DispatchVuidsCmdDrawMeshTasksEXT() : DrawDispatchVuid(Func::vkCmdDrawMeshTasksEXT) {
        pipeline_bound_08606                     = "VUID-vkCmdDrawMeshTasksEXT-None-08606";
        compatible_pipeline_08600                = "VUID-vkCmdDrawMeshTasksEXT-None-08600";
        unprotected_command_buffer_02707         = "VUID-vkCmdDrawMeshTasksEXT-commandBuffer-02707";
        descriptor_buffer_bit_set_08114          = "VUID-vkCmdDrawMeshTasksEXT-None-08114";
        descriptor_buffer_bit_not_set_08115      = "VUID-vkCmdDrawMeshTasksEXT-None-08115";
        descriptor_buffer_set_offset_missing_08117 = "VUID-vkCmdDrawMeshTasksEXT-None-08117";
        tensorARM_pDescription_09900             = "VUID-vkCmdDrawMeshTasksEXT-pDescription-09900";
        tensorARM_dimensionCount_09905           = "VUID-vkCmdDrawMeshTasksEXT-dimensionCount-09905";
    }
};

struct DispatchVuidsCmdDrawMeshTasksIndirectEXT: DrawDispatchVuid {
    DispatchVuidsCmdDrawMeshTasksIndirectEXT() : DrawDispatchVuid(Func::vkCmdDrawMeshTasksIndirectEXT) {
        pipeline_bound_08606                     = "VUID-vkCmdDrawMeshTasksIndirectEXT-None-08606";
        compatible_pipeline_08600                = "VUID-vkCmdDrawMeshTasksIndirectEXT-None-08600";
        indirect_protected_cb_02711              = "VUID-vkCmdDrawMeshTasksIndirectEXT-commandBuffer-02711";
        indirect_contiguous_memory_02708         = "VUID-vkCmdDrawMeshTasksIndirectEXT-buffer-02708";
        indirect_buffer_bit_02290                = "VUID-vkCmdDrawMeshTasksIndirectEXT-buffer-02709";
        unprotected_command_buffer_02707         = "VUID-vkCmdDrawMeshTasksIndirectEXT-commandBuffer-02707";
        descriptor_buffer_bit_set_08114          = "VUID-vkCmdDrawMeshTasksIndirectEXT-None-08114";
        descriptor_buffer_bit_not_set_08115      = "VUID-vkCmdDrawMeshTasksIndirectEXT-None-08115";
        descriptor_buffer_set_offset_missing_08117 = "VUID-vkCmdDrawMeshTasksIndirectEXT-None-08117";
        tensorARM_pDescription_09900             = "VUID-vkCmdDrawMeshTasksIndirectEXT-pDescription-09900";
        tensorARM_dimensionCount_09905           = "VUID-vkCmdDrawMeshTasksIndirectEXT-dimensionCount-09905";
    }
};

struct DispatchVuidsCmdDrawMeshTasksIndirectCountEXT : DrawDispatchVuid {
    DispatchVuidsCmdDrawMeshTasksIndirectCountEXT() : DrawDispatchVuid(Func::vkCmdDrawMeshTasksIndirectCountEXT) {
        pipeline_bound_08606                     = "VUID-vkCmdDrawMeshTasksIndirectCountEXT-None-08606";
        compatible_pipeline_08600                = "VUID-vkCmdDrawMeshTasksIndirectCountEXT-None-08600";
        indirect_protected_cb_02711              = "VUID-vkCmdDrawMeshTasksIndirectCountEXT-commandBuffer-02711";
        indirect_contiguous_memory_02708         = "VUID-vkCmdDrawMeshTasksIndirectCountEXT-buffer-02708";
        indirect_buffer_bit_02290                = "VUID-vkCmdDrawMeshTasksIndirectCountEXT-buffer-02709";
        indirect_count_contiguous_memory_02714   = "VUID-vkCmdDrawMeshTasksIndirectCountEXT-countBuffer-02714";
        indirect_count_buffer_bit_02715          = "VUID-vkCmdDrawMeshTasksIndirectCountEXT-countBuffer-02715";
        indirect_count_offset_04129              = "VUID-vkCmdDrawMeshTasksIndirectCountEXT-countBufferOffset-04129";
        unprotected_command_buffer_02707         = "VUID-vkCmdDrawMeshTasksIndirectCountEXT-commandBuffer-02707";
        descriptor_buffer_bit_set_08114          = "VUID-vkCmdDrawMeshTasksIndirectCountEXT-None-08114";
        descriptor_buffer_bit_not_set_08115      = "VUID-vkCmdDrawMeshTasksIndirectCountEXT-None-08115";
        descriptor_buffer_set_offset_missing_08117 = "VUID-vkCmdDrawMeshTasksIndirectCountEXT-None-08117";
        tensorARM_pDescription_09900             = "VUID-vkCmdDrawMeshTasksIndirectCountEXT-pDescription-09900";
        tensorARM_dimensionCount_09905           = "VUID-vkCmdDrawMeshTasksIndirectCountEXT-dimensionCount-09905";
    }
};

struct DispatchVuidsCmdDrawIndirectByteCountEXT: DrawDispatchVuid {
    DispatchVuidsCmdDrawIndirectByteCountEXT() : DrawDispatchVuid(Func::vkCmdDrawIndirectByteCountEXT) {
        pipeline_bound_08606                     = "VUID-vkCmdDrawIndirectByteCountEXT-None-08606";
        compatible_pipeline_08600                = "VUID-vkCmdDrawIndirectByteCountEXT-None-08600";
        indirect_protected_cb_02711              = "VUID-vkCmdDrawIndirectByteCountEXT-commandBuffer-02646";
        indirect_contiguous_memory_02708         = "VUID-vkCmdDrawIndirectByteCountEXT-counterBuffer-04567",
        indirect_buffer_bit_02290                = "VUID-vkCmdDrawIndirectByteCountEXT-counterBuffer-02290";
        vertex_binding_attribute_02721           = "VUID-vkCmdDrawIndirectByteCountEXT-None-02721";
        unprotected_command_buffer_02707         = "VUID-vkCmdDrawIndirectByteCountEXT-commandBuffer-02707";
        descriptor_buffer_bit_set_08114          = "VUID-vkCmdDrawIndirectByteCountEXT-None-08114";
        descriptor_buffer_bit_not_set_08115      = "VUID-vkCmdDrawIndirectByteCountEXT-None-08115";
        descriptor_buffer_set_offset_missing_08117 = "VUID-vkCmdDrawIndirectByteCountEXT-None-08117";
        tensorARM_pDescription_09900             = "VUID-vkCmdDrawIndirectByteCountEXT-pDescription-09900";
        tensorARM_dimensionCount_09905           = "VUID-vkCmdDrawIndirectByteCountEXT-dimensionCount-09905";
    }
};

struct DispatchVuidsCmdDrawIndirectByteCount2EXT: DrawDispatchVuid {
    DispatchVuidsCmdDrawIndirectByteCount2EXT() : DrawDispatchVuid(Func::vkCmdDrawIndirectByteCount2EXT) {
        pipeline_bound_08606                      = "VUID-vkCmdDrawIndirectByteCount2EXT-None-08606";
        compatible_pipeline_08600                 = "VUID-vkCmdDrawIndirectByteCount2EXT-None-08600";
        indirect_protected_cb_02711               = "VUID-vkCmdDrawIndirectByteCount2EXT-commandBuffer-02646";
        indirect_buffer_bit_02290                 = "UNASSIGNED-VkBindTransformFeedbackBuffer2InfoEXT-addressRange";
        primitive_topology_patch_list_10286       = "VUID-vkCmdDrawIndirectByteCount2EXT-primitiveTopology-10286";
        vertex_binding_attribute_02721            = "VUID-vkCmdDrawIndirectByteCount2EXT-None-02721";
        unprotected_command_buffer_02707          = "VUID-vkCmdDrawIndirectByteCount2EXT-commandBuffer-02707";
        descriptor_buffer_bit_set_08114           = "VUID-vkCmdDrawIndirectByteCount2EXT-None-08114";
        descriptor_buffer_bit_not_set_08115       = "VUID-vkCmdDrawIndirectByteCount2EXT-None-08115";
        descriptor_buffer_set_offset_missing_08117 = "VUID-vkCmdDrawIndirectByteCount2EXT-None-08117";
        tensorARM_pDescription_09900              = "VUID-vkCmdDrawIndirectByteCount2EXT-pDescription-09900";
        tensorARM_dimensionCount_09905            = "VUID-vkCmdDrawIndirectByteCount2EXT-dimensionCount-09905";
    }
};

struct DispatchVuidsCmdDispatchBase: DrawDispatchVuid {
    DispatchVuidsCmdDispatchBase() : DrawDispatchVuid(Func::vkCmdDispatchBase) {
        pipeline_bound_08606                     = "VUID-vkCmdDispatchBase-None-08606";
        compatible_pipeline_08600                = "VUID-vkCmdDispatchBase-None-08600";
        unprotected_command_buffer_02707         = "VUID-vkCmdDispatchBase-commandBuffer-02707";
        protected_command_buffer_02712           = "VUID-vkCmdDispatchBase-commandBuffer-02712";
        descriptor_buffer_bit_set_08114          = "VUID-vkCmdDispatchBase-None-08114";
        descriptor_buffer_bit_not_set_08115      = "VUID-vkCmdDispatchBase-None-08115";
        descriptor_buffer_set_offset_missing_08117 = "VUID-vkCmdDispatchBase-None-08117";
        tensorARM_pDescription_09900             = "VUID-vkCmdDispatchBase-pDescription-09900";
        tensorARM_dimensionCount_09905           = "VUID-vkCmdDispatchBase-dimensionCount-09905";
    }
};

struct DispatchVuidsCmdExecuteGeneratedCommandsEXT : DrawDispatchVuid {
    DispatchVuidsCmdExecuteGeneratedCommandsEXT() : DrawDispatchVuid(Func::vkCmdExecuteGeneratedCommandsEXT) {
        pipeline_bound_08606                     = "VUID-vkCmdExecuteGeneratedCommandsEXT-None-08606";
        compatible_pipeline_08600                = "VUID-vkCmdExecuteGeneratedCommandsEXT-None-08600";
        descriptor_buffer_bit_set_08114          = "VUID-vkCmdExecuteGeneratedCommandsEXT-None-08114";
        descriptor_buffer_bit_not_set_08115      = "VUID-vkCmdExecuteGeneratedCommandsEXT-None-08115";
        descriptor_buffer_set_offset_missing_08117 = "VUID-vkCmdExecuteGeneratedCommandsEXT-None-08117";
        tensorARM_pDescription_09900             = "VUID-vkCmdExecuteGeneratedCommandsEXT-pDescription-09900";
        tensorARM_dimensionCount_09905           = "VUID-vkCmdExecuteGeneratedCommandsEXT-dimensionCount-09905";
    }
};

struct DispatchVuidsCmdDrawIndirect2KHR : DrawDispatchVuid {
    DispatchVuidsCmdDrawIndirect2KHR() : DrawDispatchVuid(Func::vkCmdDrawIndirect2KHR) {
        pipeline_bound_08606                      = "VUID-vkCmdDrawIndirect2KHR-None-08606";
        compatible_pipeline_08600                 = "VUID-vkCmdDrawIndirect2KHR-None-08600";
        indirect_protected_cb_02711               = "VUID-vkCmdDrawIndirect2KHR-commandBuffer-13057";
        indirect_buffer_bit_02290                 = "VUID-VkDrawIndirect2InfoKHR-addressRange-13107";
        primitive_topology_patch_list_10286       = "VUID-vkCmdDrawIndirect2KHR-primitiveTopology-10286";
        vertex_binding_attribute_02721            = "VUID-vkCmdDrawIndirect2KHR-None-02721";
        unprotected_command_buffer_02707          = "VUID-vkCmdDrawIndirect2KHR-commandBuffer-02707";
        descriptor_buffer_bit_set_08114           = "VUID-vkCmdDrawIndirect2KHR-None-08114";
        descriptor_buffer_bit_not_set_08115       = "VUID-vkCmdDrawIndirect2KHR-None-08115";
        descriptor_buffer_set_offset_missing_08117 = "VUID-vkCmdDrawIndirect2KHR-None-08117";
        tensorARM_pDescription_09900              = "VUID-vkCmdDrawIndirect2KHR-pDescription-09900";
        tensorARM_dimensionCount_09905            = "VUID-vkCmdDrawIndirect2KHR-dimensionCount-09905";
    }
};

struct DispatchVuidsCmdDrawIndexedIndirect2KHR : DrawDispatchVuid {
    DispatchVuidsCmdDrawIndexedIndirect2KHR() : DrawDispatchVuid(Func::vkCmdDrawIndexedIndirect2KHR) {
        pipeline_bound_08606                      = "VUID-vkCmdDrawIndexedIndirect2KHR-None-08606";
        index_binding_07312                       = "VUID-vkCmdDrawIndexedIndirect2KHR-None-07312";
        compatible_pipeline_08600                 = "VUID-vkCmdDrawIndexedIndirect2KHR-None-08600";
        indirect_protected_cb_02711               = "VUID-vkCmdDrawIndexedIndirect2KHR-commandBuffer-13059";
        indirect_buffer_bit_02290                 = "VUID-VkDrawIndirect2InfoKHR-addressRange-13107";
        primitive_topology_patch_list_10286       = "VUID-vkCmdDrawIndexedIndirect2KHR-primitiveTopology-10286";
        vertex_binding_attribute_02721            = "VUID-vkCmdDrawIndexedIndirect2KHR-None-02721";
        unprotected_command_buffer_02707          = "VUID-vkCmdDrawIndexedIndirect2KHR-commandBuffer-02707";
        descriptor_buffer_bit_set_08114           = "VUID-vkCmdDrawIndexedIndirect2KHR-None-08114";
        descriptor_buffer_bit_not_set_08115       = "VUID-vkCmdDrawIndexedIndirect2KHR-None-08115";
        descriptor_buffer_set_offset_missing_08117 = "VUID-vkCmdDrawIndexedIndirect2KHR-None-08117";
        tensorARM_pDescription_09900              = "VUID-vkCmdDrawIndexedIndirect2KHR-pDescription-09900";
        tensorARM_dimensionCount_09905            = "VUID-vkCmdDrawIndexedIndirect2KHR-dimensionCount-09905";
    }
};

struct DispatchVuidsCmdDrawIndirectCount2KHR : DrawDispatchVuid {
    DispatchVuidsCmdDrawIndirectCount2KHR() : DrawDispatchVuid(Func::vkCmdDrawIndirectCount2KHR) {
        pipeline_bound_08606                      = "VUID-vkCmdDrawIndirectCount2KHR-None-08606";
        compatible_pipeline_08600                 = "VUID-vkCmdDrawIndirectCount2KHR-None-08600";
        indirect_protected_cb_02711               = "VUID-vkCmdDrawIndirectCount2KHR-commandBuffer-13058";
        indirect_buffer_bit_02290                 = "VUID-VkDrawIndirectCount2InfoKHR-addressRange-13107";
        indirect_count_buffer_bit_02715           = "VUID-VkDrawIndirectCount2InfoKHR-countAddressRange-13114";
        primitive_topology_patch_list_10286       = "VUID-vkCmdDrawIndirectCount2KHR-primitiveTopology-10286";
        vertex_binding_attribute_02721            = "VUID-vkCmdDrawIndirectCount2KHR-None-02721";
        unprotected_command_buffer_02707          = "VUID-vkCmdDrawIndirectCount2KHR-commandBuffer-02707";
        descriptor_buffer_bit_set_08114           = "VUID-vkCmdDrawIndirectCount2KHR-None-08114";
        descriptor_buffer_bit_not_set_08115       = "VUID-vkCmdDrawIndirectCount2KHR-None-08115";
        descriptor_buffer_set_offset_missing_08117 = "VUID-vkCmdDrawIndirectCount2KHR-None-08117";
        tensorARM_pDescription_09900              = "VUID-vkCmdDrawIndirectCount2KHR-pDescription-09900";
        tensorARM_dimensionCount_09905            = "VUID-vkCmdDrawIndirectCount2KHR-dimensionCount-09905";
    }
};

struct DispatchVuidsCmdDrawIndexedIndirectCount2KHR : DrawDispatchVuid {
    DispatchVuidsCmdDrawIndexedIndirectCount2KHR() : DrawDispatchVuid(Func::vkCmdDrawIndexedIndirectCount2KHR) {
        pipeline_bound_08606                      = "VUID-vkCmdDrawIndexedIndirectCount2KHR-None-08606";
        index_binding_07312                       = "VUID-vkCmdDrawIndexedIndirectCount2KHR-None-07312";
        compatible_pipeline_08600                 = "VUID-vkCmdDrawIndexedIndirectCount2KHR-None-08600";
        indirect_protected_cb_02711               = "VUID-vkCmdDrawIndexedIndirectCount2KHR-commandBuffer-13060";
        indirect_buffer_bit_02290                 = "VUID-VkDrawIndirectCount2InfoKHR-addressRange-13107";
        indirect_count_buffer_bit_02715           = "VUID-VkDrawIndirectCount2InfoKHR-countAddressRange-13114";
        primitive_topology_patch_list_10286       = "VUID-vkCmdDrawIndexedIndirectCount2KHR-primitiveTopology-10286";
        vertex_binding_attribute_02721            = "VUID-vkCmdDrawIndexedIndirectCount2KHR-None-02721";
        unprotected_command_buffer_02707          = "VUID-vkCmdDrawIndexedIndirectCount2KHR-commandBuffer-02707";
        descriptor_buffer_bit_set_08114           = "VUID-vkCmdDrawIndexedIndirectCount2KHR-None-08114";
        descriptor_buffer_bit_not_set_08115       = "VUID-vkCmdDrawIndexedIndirectCount2KHR-None-08115";
        descriptor_buffer_set_offset_missing_08117 = "VUID-vkCmdDrawIndexedIndirectCount2KHR-None-08117";
        tensorARM_pDescription_09900              = "VUID-vkCmdDrawIndexedIndirectCount2KHR-pDescription-09900";
        tensorARM_dimensionCount_09905            = "VUID-vkCmdDrawIndexedIndirectCount2KHR-dimensionCount-09905";
    }
};

struct DispatchVuidsCmdDispatchIndirect2KHR : DrawDispatchVuid {
    DispatchVuidsCmdDispatchIndirect2KHR() : DrawDispatchVuid(Func::vkCmdDispatchIndirect2KHR) {
        pipeline_bound_08606                      = "VUID-vkCmdDispatchIndirect2KHR-None-08606";
        compatible_pipeline_08600                 = "VUID-vkCmdDispatchIndirect2KHR-None-08600";
        indirect_protected_cb_02711               = "VUID-vkCmdDispatchIndirect2KHR-commandBuffer-13049";
        indirect_buffer_bit_02290                 = "VUID-VkDispatchIndirect2InfoKHR-addressRange-13107";
        unprotected_command_buffer_02707          = "VUID-vkCmdDispatchIndirect2KHR-commandBuffer-02707";
        descriptor_buffer_bit_set_08114           = "VUID-vkCmdDispatchIndirect2KHR-None-08114";
        descriptor_buffer_bit_not_set_08115       = "VUID-vkCmdDispatchIndirect2KHR-None-08115";
        descriptor_buffer_set_offset_missing_08117 = "VUID-vkCmdDispatchIndirect2KHR-None-08117";
        tensorARM_pDescription_09900              = "VUID-vkCmdDispatchIndirect2KHR-pDescription-09900";
        tensorARM_dimensionCount_09905            = "VUID-vkCmdDispatchIndirect2KHR-dimensionCount-09905";
    }
};

struct DispatchVuidsCmdDrawMeshTasksIndirect2EXT : DrawDispatchVuid {
    DispatchVuidsCmdDrawMeshTasksIndirect2EXT() : DrawDispatchVuid(Func::vkCmdDrawMeshTasksIndirect2EXT) {
        pipeline_bound_08606                      = "VUID-vkCmdDrawMeshTasksIndirect2EXT-None-08606";
        compatible_pipeline_08600                 = "VUID-vkCmdDrawMeshTasksIndirect2EXT-None-08600";
        indirect_protected_cb_02711               = "VUID-vkCmdDrawMeshTasksIndirect2EXT-commandBuffer-13067";
        indirect_buffer_bit_02290                 = "VUID-VkDrawIndirect2InfoKHR-addressRange-13107";
        unprotected_command_buffer_02707          = "VUID-vkCmdDrawMeshTasksIndirect2EXT-commandBuffer-02707";
        descriptor_buffer_bit_set_08114           = "VUID-vkCmdDrawMeshTasksIndirect2EXT-None-08114";
        descriptor_buffer_bit_not_set_08115       = "VUID-vkCmdDrawMeshTasksIndirect2EXT-None-08115";
        descriptor_buffer_set_offset_missing_08117 = "VUID-vkCmdDrawMeshTasksIndirect2EXT-None-08117";
        tensorARM_pDescription_09900              = "VUID-vkCmdDrawMeshTasksIndirect2EXT-pDescription-09900";
        tensorARM_dimensionCount_09905            = "VUID-vkCmdDrawMeshTasksIndirect2EXT-dimensionCount-09905";
    }
};

struct DispatchVuidsCmdDrawMeshTasksIndirectCount2EXT : DrawDispatchVuid {
    DispatchVuidsCmdDrawMeshTasksIndirectCount2EXT() : DrawDispatchVuid(Func::vkCmdDrawMeshTasksIndirectCount2EXT) {
        pipeline_bound_08606                      = "VUID-vkCmdDrawMeshTasksIndirectCount2EXT-None-08606";
        compatible_pipeline_08600                 = "VUID-vkCmdDrawMeshTasksIndirectCount2EXT-None-08600";
        indirect_protected_cb_02711               = "VUID-vkCmdDrawMeshTasksIndirectCount2EXT-commandBuffer-13068";
        indirect_buffer_bit_02290                 = "VUID-VkDrawIndirectCount2InfoKHR-addressRange-13107";
        unprotected_command_buffer_02707          = "VUID-vkCmdDrawMeshTasksIndirectCount2EXT-commandBuffer-02707";
        descriptor_buffer_bit_set_08114           = "VUID-vkCmdDrawMeshTasksIndirectCount2EXT-None-08114";
        descriptor_buffer_bit_not_set_08115       = "VUID-vkCmdDrawMeshTasksIndirectCount2EXT-None-08115";
        descriptor_buffer_set_offset_missing_08117 = "VUID-vkCmdDrawMeshTasksIndirectCount2EXT-None-08117";
        tensorARM_pDescription_09900              = "VUID-vkCmdDrawMeshTasksIndirectCount2EXT-pDescription-09900";
        tensorARM_dimensionCount_09905            = "VUID-vkCmdDrawMeshTasksIndirectCount2EXT-dimensionCount-09905";
    }
};

struct DispatchVuidsCmdDispatchDataGraphARM : DrawDispatchVuid {
    DispatchVuidsCmdDispatchDataGraphARM() : DrawDispatchVuid(Func::vkCmdDispatchDataGraphARM) {
        pipeline_bound_08606                     = "VUID-vkCmdDispatchDataGraphARM-None-09799";
        compatible_pipeline_08600                = "VUID-vkCmdDispatchDataGraphARM-None-09797";
        unprotected_command_buffer_02707         = "VUID-vkCmdDispatchDataGraphARM-commandBuffer-09800";
        protected_command_buffer_02712           = "VUID-vkCmdDispatchDataGraphARM-commandBuffer-09801";
        descriptor_buffer_bit_set_08114          = "VUID-vkCmdDispatchDataGraphARM-None-09935";
        descriptor_buffer_bit_not_set_08115      = "VUID-vkCmdDispatchDataGraphARM-None-09936";
        descriptor_buffer_set_offset_missing_08117 = "VUID-vkCmdDispatchDataGraphARM-None-09938";
        tensorARM_pDescription_09900             = "VUID-vkCmdDispatchDataGraphARM-pDescription-09930";
    }
};

using Func = vvl::Func;
// This LUT is created to allow a static listing of each VUID that is covered by drawdispatch commands
const auto &GetDrawDispatchVuid() {
static const std::pair<Func, DrawDispatchVuid> pairs[] = {
    {Func::vkCmdDraw, DispatchVuidsCmdDraw()},
    {Func::vkCmdDrawMultiEXT, DispatchVuidsCmdDrawMultiEXT()},
    {Func::vkCmdDrawIndexed, DispatchVuidsCmdDrawIndexed()},
    {Func::vkCmdDrawMultiIndexedEXT, DispatchVuidsCmdDrawMultiIndexedEXT()},
    {Func::vkCmdDrawIndirect, DispatchVuidsCmdDrawIndirect()},
    {Func::vkCmdDrawIndexedIndirect, DispatchVuidsCmdDrawIndexedIndirect()},
    {Func::vkCmdDispatch, DispatchVuidsCmdDispatch()},
    {Func::vkCmdDispatchIndirect, DispatchVuidsCmdDispatchIndirect()},
    {Func::vkCmdDrawIndirectCount, DispatchVuidsCmdDrawIndirectCount()},
    {Func::vkCmdDrawIndirectCountKHR , DispatchVuidsCmdDrawIndirectCount()},
    {Func::vkCmdDrawIndexedIndirectCount, DispatchVuidsCmdDrawIndexedIndirectCount()},
    {Func::vkCmdDrawIndexedIndirectCountKHR, DispatchVuidsCmdDrawIndexedIndirectCount()},
    {Func::vkCmdTraceRaysNV, DispatchVuidsCmdTraceRaysNV()},
    {Func::vkCmdTraceRaysKHR, DispatchVuidsCmdTraceRaysKHR()},
    {Func::vkCmdTraceRaysIndirectKHR, DispatchVuidsCmdTraceRaysIndirectKHR()},
    {Func::vkCmdTraceRaysIndirect2KHR, DispatchVuidsCmdTraceRaysIndirect2KHR()},
    {Func::vkCmdDrawMeshTasksNV, DispatchVuidsCmdDrawMeshTasksNV()},
    {Func::vkCmdDrawMeshTasksIndirectNV, DispatchVuidsCmdDrawMeshTasksIndirectNV()},
    {Func::vkCmdDrawMeshTasksIndirectCountNV, DispatchVuidsCmdDrawMeshTasksIndirectCountNV()},
    {Func::vkCmdDrawMeshTasksEXT, DispatchVuidsCmdDrawMeshTasksEXT()},
    {Func::vkCmdDrawMeshTasksIndirectEXT, DispatchVuidsCmdDrawMeshTasksIndirectEXT()},
    {Func::vkCmdDrawMeshTasksIndirectCountEXT, DispatchVuidsCmdDrawMeshTasksIndirectCountEXT()},
    {Func::vkCmdDrawIndirectByteCountEXT, DispatchVuidsCmdDrawIndirectByteCountEXT()},
    {Func::vkCmdDrawIndirectByteCount2EXT, DispatchVuidsCmdDrawIndirectByteCount2EXT()},
    {Func::vkCmdDispatchBase, DispatchVuidsCmdDispatchBase()},
    {Func::vkCmdDispatchBaseKHR, DispatchVuidsCmdDispatchBase()},
    {Func::vkCmdExecuteGeneratedCommandsEXT, DispatchVuidsCmdExecuteGeneratedCommandsEXT()},
    {Func::vkCmdDrawIndirect2KHR, DispatchVuidsCmdDrawIndirect2KHR()},
    {Func::vkCmdDrawIndexedIndirect2KHR, DispatchVuidsCmdDrawIndexedIndirect2KHR()},
    {Func::vkCmdDrawIndirectCount2KHR, DispatchVuidsCmdDrawIndirectCount2KHR()},
    {Func::vkCmdDrawIndexedIndirectCount2KHR, DispatchVuidsCmdDrawIndexedIndirectCount2KHR()},
    {Func::vkCmdDispatchIndirect2KHR, DispatchVuidsCmdDispatchIndirect2KHR()},
    {Func::vkCmdDrawMeshTasksIndirect2EXT, DispatchVuidsCmdDrawMeshTasksIndirect2EXT()},
    {Func::vkCmdDrawMeshTasksIndirectCount2EXT, DispatchVuidsCmdDrawMeshTasksIndirectCount2EXT()},
    {Func::vkCmdDispatchDataGraphARM, DispatchVuidsCmdDispatchDataGraphARM()},
    // Used if invalid function is used
    {Func::Empty, DrawDispatchVuid(Func::Empty)}
};
static const auto kDrawdispatchVuid = vvl::unordered_map<Func, DrawDispatchVuid>{std::begin(pairs), std::end(pairs)};

  return kDrawdispatchVuid;
}
// clang-format on

// Getter function to provide kVUIDUndefined in case an invalid function is passed in. Likely if new extension adds command and
// VUIDs are not added yet
const DrawDispatchVuid& GetDrawDispatchVuid(Func function) {
    const vvl::unordered_map<Func, DrawDispatchVuid>& vuids = GetDrawDispatchVuid();
    if (vuids.find(function) != vuids.cend()) {
        return vuids.at(function);
    } else {
        return vuids.at(Func::Empty);
    }
}

// The reason we keep strings of ALL the VUIDs is to make it easier to search and know what is covered or not.
// The reality is the "action" commands (draw/dispatch/traceRays) VUID list has got stupidly large with 30+ ways to invoke work,
// each having over 250 VUs, large enough that we are breaking stack limits from how many strings we have!!
// (https://github.com/KhronosGroup/Vulkan-ValidationLayers/issues/11779)
//
// ... So this is "THE EXCEPTION, NOT THE NORMAL" here that we just concatenate the VUID from a number/enum
// The scripts/vk_validation_stats.py is adjusted to handle this.
//
// TODO - This is an ongoing thing that will be too hard to do "all at once", so there might be some overlap between here and the
// original DrawDispatchVuid list for a while.
//
std::string CreateActionVuid(vvl::Func function, const ActionVUID id) {
    const char* suffix = nullptr;

    // clang-format off
    switch (id) {
        case ActionVUID::UNKNOWN: suffix = "UNKNOWN"; break;

        // TODO - This one is a lie, see ValidateCmdBufImageLayouts()
        // ### VUID-vkCmdDraw-None-09600
        case ActionVUID::IMAGE_LAYOUT_09600: suffix = "None-09600"; break;

        // We use the three # to reference one real example, for the scripts/vk_validation_stats.py to parse
        //
        // ### VUID-vkCmdDraw-None-04007
        case ActionVUID::VERTEX_BINDING_04007: suffix = "None-04007"; break;
        // ### VUID-vkCmdDraw-None-04008
        case ActionVUID::VERTEX_BINDING_04008: suffix = "None-04008"; break;

        // ### VUID-vkCmdDraw-subpass-02685
        case ActionVUID::SUBPASS_INDEX_02685: suffix = "subpass-02685"; break;

        // ### VUID-vkCmdDraw-sampleLocationsEnable-02689
        case ActionVUID::SAMPLE_LOCATION_02689: suffix = "sampleLocationsEnable-02689"; break;
        // ### VUID-vkCmdDraw-sampleLocationsEnable-07484
        case ActionVUID::SAMPLE_LOCATION_07484: suffix = "sampleLocationsEnable-07484"; break;
        // ### VUID-vkCmdDraw-sampleLocationsEnable-07485
        case ActionVUID::SAMPLE_LOCATION_07485: suffix = "sampleLocationsEnable-07485"; break;
        // ### VUID-vkCmdDraw-sampleLocationsEnable-07486
        case ActionVUID::SAMPLE_LOCATION_07486: suffix = "sampleLocationsEnable-07486"; break;
        // ### VUID-vkCmdDraw-sampleLocationsEnable-07487
        case ActionVUID::SAMPLE_LOCATION_07487: suffix = "sampleLocationsEnable-07487"; break;

        // ### VUID-vkCmdDraw-None-09211
        case ActionVUID::RASTERIZATION_SAMPLES_09211: suffix = "None-09211"; break;

        // ### VUID-vkCmdDraw-magFilter-04553
        case ActionVUID::LINEAR_FILTER_04553: suffix = "magFilter-04553"; break;
        // ### VUID-vkCmdDraw-magFilter-09598
        case ActionVUID::LINEAR_FILTER_09598: suffix = "magFilter-09598"; break;
        // ### VUID-vkCmdDraw-mipmapMode-04770
        case ActionVUID::LINEAR_MIPMAP_04770: suffix = "mipmapMode-04770"; break;
        // ### VUID-vkCmdDraw-mipmapMode-09599
        case ActionVUID::LINEAR_MIPMAP_09599: suffix = "mipmapMode-09599"; break;

        // ### VUID-vkCmdDraw-colorAttachmentCount-09362
        case ActionVUID::EXTERNAL_FORMAT_RESOLVE_09362: suffix = "colorAttachmentCount-09362"; break;
        // ### VUID-vkCmdDraw-None-09363
        case ActionVUID::EXTERNAL_FORMAT_RESOLVE_09363: suffix = "None-09363"; break;
        // ### VUID-vkCmdDraw-None-09364
        case ActionVUID::EXTERNAL_FORMAT_RESOLVE_09364: suffix = "None-09364"; break;
        // ### VUID-vkCmdDraw-None-09365
        case ActionVUID::EXTERNAL_FORMAT_RESOLVE_09365: suffix = "None-09365"; break;
        // ### VUID-vkCmdDraw-None-09368
        case ActionVUID::EXTERNAL_FORMAT_RESOLVE_09368: suffix = "None-09368"; break;
        // ### VUID-vkCmdDraw-None-09369
        case ActionVUID::EXTERNAL_FORMAT_RESOLVE_09369: suffix = "None-09369"; break;
        // ### VUID-vkCmdDraw-colorAttachmentCount-09372
        case ActionVUID::EXTERNAL_FORMAT_RESOLVE_09372: suffix = "colorAttachmentCount-09372"; break;

        // ### VUID-vkCmdDraw-flags-11521
        case ActionVUID::CUSTOM_RESOLVE_11521: suffix = "flags-11521"; break;
        // ### VUID-vkCmdDraw-None-11522
        case ActionVUID::CUSTOM_RESOLVE_11522: suffix = "None-11522"; break;
        // ### VUID-vkCmdDraw-None-11523
        case ActionVUID::CUSTOM_RESOLVE_11523: suffix = "None-11523"; break;
        // ### VUID-vkCmdDraw-customResolve-11524
        case ActionVUID::CUSTOM_RESOLVE_11524: suffix = "customResolve-11524"; break;
        // ### VUID-vkCmdDraw-customResolve-11525
        case ActionVUID::CUSTOM_RESOLVE_11525: suffix = "customResolve-11525"; break;
        // ### VUID-vkCmdDraw-customResolve-11529
        case ActionVUID::CUSTOM_RESOLVE_11529: suffix = "customResolve-11529"; break;
        // ### VUID-vkCmdDraw-customResolve-11530
        case ActionVUID::CUSTOM_RESOLVE_11530: suffix = "customResolve-11530"; break;
        // ### VUID-vkCmdDraw-pColorAttachments-11539
        case ActionVUID::CUSTOM_RESOLVE_11539: suffix = "pColorAttachments-11539"; break;
        // ### VUID-vkCmdDraw-pDepthAttachment-11540
        case ActionVUID::CUSTOM_RESOLVE_11540: suffix = "pDepthAttachment-11540"; break;
        // ### VUID-vkCmdDraw-pStencilAttachment-11860
        case ActionVUID::CUSTOM_RESOLVE_11860: suffix = "pStencilAttachment-11860"; break;
        // ### VUID-vkCmdDraw-None-11861
        case ActionVUID::CUSTOM_RESOLVE_11861: suffix = "None-11861"; break;
        // ### VUID-vkCmdDraw-None-11862
        case ActionVUID::CUSTOM_RESOLVE_11862: suffix = "None-11862"; break;
        // ### VUID-vkCmdDraw-None-11863
        case ActionVUID::CUSTOM_RESOLVE_11863: suffix = "None-11863"; break;
        // ### VUID-vkCmdDraw-dynamicRenderingUnusedAttachments-11864
        case ActionVUID::CUSTOM_RESOLVE_11864: suffix = "dynamicRenderingUnusedAttachments-11864"; break;
        // ### VUID-vkCmdDraw-None-11865
        case ActionVUID::CUSTOM_RESOLVE_11865: suffix = "None-11865"; break;
        // ### VUID-vkCmdDraw-None-11866
        case ActionVUID::CUSTOM_RESOLVE_11866: suffix = "None-11866"; break;
        // ### VUID-vkCmdDraw-dynamicRenderingUnusedAttachments-11867
        case ActionVUID::CUSTOM_RESOLVE_11867: suffix = "dynamicRenderingUnusedAttachments-11867"; break;
        // ### VUID-vkCmdDraw-None-11868
        case ActionVUID::CUSTOM_RESOLVE_11868: suffix = "None-11868"; break;
        // ### VUID-vkCmdDraw-None-11869
        case ActionVUID::CUSTOM_RESOLVE_11869: suffix = "None-11869"; break;
        // ### VUID-vkCmdDraw-dynamicRenderingUnusedAttachments-11870
        case ActionVUID::CUSTOM_RESOLVE_11870: suffix = "dynamicRenderingUnusedAttachments-11870"; break;

        // ### VUID-vkCmdDraw-stippledLineEnable-07495
        case ActionVUID::STIPPLED_RECTANGULAR_07495: suffix = "stippledLineEnable-07495"; break;
        // ### VUID-vkCmdDraw-stippledLineEnable-07496
        case ActionVUID::STIPPLED_BRESENHAM_07496: suffix = "stippledLineEnable-07496"; break;
        // ### VUID-vkCmdDraw-stippledLineEnable-07497
        case ActionVUID::STIPPLED_SMOOTH_07497: suffix = "stippledLineEnable-07497"; break;
        // ### VUID-vkCmdDraw-stippledLineEnable-07498
        case ActionVUID::STIPPLED_STRICT_07498: suffix = "stippledLineEnable-07498"; break;

        // ### VUID-vkCmdDraw-SampledType-04470
        case ActionVUID::IMAGE_VIEW_ACCESS_04470: suffix = "SampledType-04470"; break;
        // ### VUID-vkCmdDraw-SampledType-04471
        case ActionVUID::IMAGE_VIEW_ACCESS_04471: suffix = "SampledType-04471"; break;
        // ### VUID-vkCmdDraw-sparseImageInt64Atomics-04474
        case ActionVUID::IMAGE_VIEW_SPARSE_04474: suffix = "sparseImageInt64Atomics-04474"; break;
        // ### VUID-vkCmdDraw-SampledType-04472
        case ActionVUID::BUFFER_VIEW_ACCESS_04472: suffix = "SampledType-04472"; break;
        // ### VUID-vkCmdDraw-SampledType-04473
        case ActionVUID::BUFFER_VIEW_ACCESS_04473: suffix = "SampledType-04473"; break;
        // ### VUID-vkCmdDraw-OpTypeImage-07028
        case ActionVUID::STORAGE_IMAGE_FORMAT_07028: suffix = "OpTypeImage-07028"; break;
        // ### VUID-vkCmdDraw-OpTypeImage-07027
        case ActionVUID::STORAGE_IMAGE_FORMAT_07027: suffix = "OpTypeImage-07027"; break;
        // ### VUID-vkCmdDraw-OpTypeImage-07030
        case ActionVUID::STORAGE_TEXEL_FORMAT_07030: suffix = "OpTypeImage-07030"; break;
        // ### VUID-vkCmdDraw-OpTypeImage-07029
        case ActionVUID::STORAGE_TEXEL_FORMAT_07029: suffix = "OpTypeImage-07029"; break;
        // ### VUID-vkCmdDraw-OpImageWrite-08795
        case ActionVUID::STORAGE_IMAGE_TEXEL_08795: suffix = "OpImageWrite-08795"; break;
        // ### VUID-vkCmdDraw-OpImageWrite-08796
        case ActionVUID::STORAGE_IMAGE_TEXEL_08796: suffix = "OpImageWrite-08796"; break;
        // ### VUID-vkCmdDraw-OpImageWrite-04469
        case ActionVUID::STORAGE_TEXEL_04469: suffix = "OpImageWrite-04469"; break;
        // ### VUID-vkCmdDraw-None-02691
        case ActionVUID::IMAGE_VIEW_ATOMIC_02691: suffix = "None-02691"; break;
        // ### VUID-vkCmdDraw-None-07888
        case ActionVUID::BUFFER_VIEW_ATOMIC_07888: suffix = "None-07888"; break;
        // ### VUID-vkCmdDraw-viewType-07752
        case ActionVUID::IMAGE_VIEW_DIM_07752: suffix = "viewType-07752"; break;
        // ### VUID-vkCmdDraw-format-07753
        case ActionVUID::IMAGE_VIEW_NUMERIC_07753: suffix = "format-07753"; break;

        // ### VUID-vkCmdDraw-None-06537
        case ActionVUID::SUBRESOURCE_RP_WRTIE_06537: suffix = "None-06537"; break;
        // ### VUID-vkCmdDraw-None-12338
        case ActionVUID::SUBRESOURCE_SUBPASS_12338: suffix = "None-12338"; break;
        // ### VUID-vkCmdDraw-None-12339
        case ActionVUID::SUBRESOURCE_SUBPASS_12339: suffix = "None-12339"; break;
        // ### VUID-vkCmdDraw-None-12340
        case ActionVUID::SUBRESOURCE_SUBPASS_12340: suffix = "None-12340"; break;

        // ### VUID-vkCmdDraw-None-11308
        case ActionVUID::DESCRIPTOR_HEAP_11308: suffix = "None-11308"; break;
        // ### VUID-vkCmdDraw-pBindInfo-11375
        case ActionVUID::DESCRIPTOR_HEAP_11375: suffix = "pBindInfo-11375"; break;
        // ### VUID-vkCmdDraw-None-11376
        case ActionVUID::DESCRIPTOR_HEAP_11376: suffix = "None-11376"; break;

        // ### VUID-vkCmdDraw-None-08609
        case ActionVUID::SAMPLER_TYPE_08609: suffix = "None-08609"; break;
        // ### VUID-vkCmdDraw-None-08610
        case ActionVUID::SAMPLER_DREF_PROJ_08610: suffix = "None-08610"; break;
        // ### VUID-vkCmdDraw-None-08611
        case ActionVUID::SAMPLER_BIAS_OFFSET_08611: suffix = "None-08611"; break;
        // ### VUID-vkCmdDraw-None-02692
        case ActionVUID::SAMPLER_CUBIC_02692: suffix = "None-02692"; break;
        // ### VUID-vkCmdDraw-flags-02696
        case ActionVUID::SAMPLER_CORNER_02696: suffix = "flags-02696"; break;

        // ### VUID-vkCmdDraw-None-02693
        case ActionVUID::FILTER_CUBIC_02693: suffix = "None-02693"; break;
        // ### VUID-vkCmdDraw-filterCubic-02694
        case ActionVUID::FILTER_CUBIC_02694: suffix = "filterCubic-02694"; break;
        // ### VUID-vkCmdDraw-filterCubicMinmax-02695
        case ActionVUID::FILTER_CUBIC_02695: suffix = "filterCubicMinmax-02695"; break;

        // ### VUID-vkCmdDraw-imageLayout-00344
        case ActionVUID::IMAGE_LAYOUT_00344: suffix = "imageLayout-00344"; break;

        // ### VUID-vkCmdDraw-maintenance4-08602
        case ActionVUID::PUSH_CONSTANT_08602: suffix = "maintenance4-08602"; break;

        // ### VUID-vkCmdDraw-None-08608
        case ActionVUID::DYNAMIC_STATE_ALL_SET_08608: suffix = "None-08608"; break;

        // ### VUID-vkCmdDraw-None-07845
        case ActionVUID::DYNAMIC_DEPTH_COMPARE_OP_07845: suffix = "None-07845"; break;
        // ### VUID-vkCmdDraw-None-07834
        case ActionVUID::DYNAMIC_DEPTH_BIAS_07834: suffix = "None-07834"; break;
        // ### VUID-vkCmdDraw-None-07836
        case ActionVUID::DYNAMIC_DEPTH_BOUNDS_07836: suffix = "None-07836"; break;
        // ### VUID-vkCmdDraw-None-07840
        case ActionVUID::DYNAMIC_CULL_MODE_07840: suffix = "None-07840"; break;
        // ### VUID-vkCmdDraw-None-07843
        case ActionVUID::DYNAMIC_DEPTH_TEST_ENABLE_07843: suffix = "None-07843"; break;
        // ### VUID-vkCmdDraw-None-07844
        case ActionVUID::DYNAMIC_DEPTH_WRITE_ENABLE_07844: suffix = "None-07844"; break;
        // ### VUID-vkCmdDraw-None-07847
        case ActionVUID::DYNAMIC_STENCIL_TEST_ENABLE_07847: suffix = "None-07847"; break;
        // ### VUID-vkCmdDraw-None-04877
        case ActionVUID::DYNAMIC_DEPTH_BIAS_ENABLE_04877: suffix = "None-04877"; break;
        // ### VUID-vkCmdDraw-None-07846
        case ActionVUID::DYNAMIC_DEPTH_BOUND_TEST_ENABLE_07846: suffix = "None-07846"; break;
        // ### VUID-vkCmdDraw-None-07837
        case ActionVUID::DYNAMIC_STENCIL_COMPARE_MASK_07837: suffix = "None-07837"; break;
        // ### VUID-vkCmdDraw-None-07838
        case ActionVUID::DYNAMIC_STENCIL_WRITE_MASK_07838: suffix = "None-07838"; break;
        // ### VUID-vkCmdDraw-None-07839
        case ActionVUID::DYNAMIC_STENCIL_REFERENCE_07839: suffix = "None-07839"; break;
        // ### VUID-vkCmdDraw-None-07848
        case ActionVUID::DYNAMIC_STENCIL_OP_07848: suffix = "None-07848"; break;
        // ### VUID-vkCmdDraw-None-07842
        case ActionVUID::DYNAMIC_PRIMITIVE_TOPOLOGY_07842: suffix = "None-07842"; break;
        // ### VUID-vkCmdDraw-None-04879
        case ActionVUID::DYNAMIC_PRIMITIVE_RESTART_ENABLE_04879: suffix = "None-04879"; break;
        // ### VUID-vkCmdDraw-None-04914
        case ActionVUID::DYNAMIC_VERTEX_INPUT_04914: suffix = "None-04914"; break;
        // ### VUID-vkCmdDraw-pipelineFragmentShadingRate-09238
        case ActionVUID::DYNAMIC_SET_FRAGMENT_SHADING_RATE_09238: suffix = "pipelineFragmentShadingRate-09238"; break;
        // ### VUID-vkCmdDraw-logicOp-04878
        case ActionVUID::DYNAMIC_LOGIC_OP_04878: suffix = "logicOp-04878"; break;
        // ### VUID-vkCmdDraw-None-07621
        case ActionVUID::DYNAMIC_POLYGON_MODE_07621: suffix = "None-07621"; break;
        // ### VUID-vkCmdDraw-None-07622
        case ActionVUID::DYNAMIC_RASTERIZATION_SAMPLES_07622: suffix = "None-07622"; break;
        // ### VUID-vkCmdDraw-None-07623
        case ActionVUID::DYNAMIC_SAMPLE_MASK_07623: suffix = "None-07623"; break;
        // ### VUID-vkCmdDraw-None-07624
        case ActionVUID::DYNAMIC_ALPHA_TO_COVERAGE_ENABLE_07624: suffix = "None-07624"; break;
        // ### VUID-vkCmdDraw-None-07625
        case ActionVUID::DYNAMIC_ALPHA_TO_ONE_ENABLE_07625: suffix = "None-07625"; break;
        // ### VUID-vkCmdDraw-None-07626
        case ActionVUID::DYNAMIC_LOGIC_OP_ENABLE_07626: suffix = "None-07626"; break;
        // ### VUID-vkCmdDraw-None-04876
        case ActionVUID::DYNAMIC_RASTERIZER_DISCARD_ENABLE_04876: suffix = "None-04876"; break;
        // ### VUID-vkCmdDraw-None-07634
        case ActionVUID::DYNAMIC_SAMPLE_LOCATIONS_ENABLE_07634: suffix = "None-07634"; break;
        // ### VUID-vkCmdDraw-None-06666
        case ActionVUID::DYNAMIC_SAMPLE_LOCATIONS_06666: suffix = "None-06666"; break;
        // ### VUID-vkCmdDraw-None-08877
        case ActionVUID::DYNAMIC_ATTACHMENT_FEEDBACK_LOOP_08877: suffix = "None-08877"; break;
        // ### VUID-vkCmdDraw-None-07749
        case ActionVUID::DYNAMIC_COLOR_WRITE_ENABLE_07749: suffix = "None-07749"; break;
        // ### VUID-vkCmdDraw-None-07627
        case ActionVUID::DYNAMIC_COLOR_BLEND_ENABLE_07627: suffix = "None-07627"; break;
        // ### VUID-vkCmdDraw-None-07629
        case ActionVUID::DYNAMIC_COLOR_WRITE_MASK_07629: suffix = "None-07629"; break;
        // ### VUID-vkCmdDraw-None-07639
        case ActionVUID::DYNAMIC_DEPTH_CLIP_NEGATIVE_ONE_TO_ONE_07639: suffix = "None-07639"; break;
        // ### VUID-vkCmdDraw-None-09650
        case ActionVUID::DYNAMIC_DEPTH_CLAMP_CONTROL_09650: suffix = "None-09650"; break;
        // ### VUID-vkCmdDraw-None-07633
        case ActionVUID::DYNAMIC_DEPTH_CLIP_ENABLE_07633: suffix = "None-07633"; break;
        // ### VUID-vkCmdDraw-None-07620
        case ActionVUID::DYNAMIC_DEPTH_CLAMP_ENABLE_07620: suffix = "None-07620"; break;
        // ### VUID-vkCmdDraw-None-07878
        case ActionVUID::DYNAMIC_EXCLUSIVE_SCISSOR_ENABLE_07878: suffix = "None-07878"; break;
        // ### VUID-vkCmdDraw-None-07879
        case ActionVUID::DYNAMIC_EXCLUSIVE_SCISSOR_07879: suffix = "None-07879"; break;
        // ### VUID-vkCmdDraw-None-07619
        case ActionVUID::DYNAMIC_TESSELLATION_DOMAIN_ORIGIN_07619: suffix = "None-07619"; break;
        // ### VUID-vkCmdDraw-None-07630
        case ActionVUID::DYNAMIC_RASTERIZATION_STREAM_07630: suffix = "None-07630"; break;
        // ### VUID-vkCmdDraw-None-07631
        case ActionVUID::DYNAMIC_CONSERVATIVE_RASTERIZATION_MODE_07631: suffix = "None-07631"; break;
        // ### VUID-vkCmdDraw-None-07632
        case ActionVUID::DYNAMIC_EXTRA_PRIMITIVE_OVERESTIMATION_SIZE_07632: suffix = "None-07632"; break;
        // ### VUID-vkCmdDraw-None-07636
        case ActionVUID::DYNAMIC_PROVOKING_VERTEX_MODE_07636: suffix = "None-07636"; break;
        // ### VUID-vkCmdDraw-None-07640
        case ActionVUID::DYNAMIC_VIEWPORT_W_SCALING_ENABLE_07640: suffix = "None-07640"; break;
        // ### VUID-vkCmdDraw-None-07641
        case ActionVUID::DYNAMIC_VIEWPORT_SWIZZLE_07641: suffix = "None-07641"; break;
        // ### VUID-vkCmdDraw-None-07642
        case ActionVUID::DYNAMIC_COVERAGE_TO_COLOR_ENABLE_07642: suffix = "None-07642"; break;
        // ### VUID-vkCmdDraw-None-07643
        case ActionVUID::DYNAMIC_COVERAGE_TO_COLOR_LOCATION_07643: suffix = "None-07643"; break;
        // ### VUID-vkCmdDraw-None-07644
        case ActionVUID::DYNAMIC_COVERAGE_MODULATION_MODE_07644: suffix = "None-07644"; break;
        // ### VUID-vkCmdDraw-None-07645
        case ActionVUID::DYNAMIC_COVERAGE_MODULATION_TABLE_ENABLE_07645: suffix = "None-07645"; break;
        // ### VUID-vkCmdDraw-None-07646
        case ActionVUID::DYNAMIC_COVERAGE_MODULATION_TABLE_07646: suffix = "None-07646"; break;
        // ### VUID-vkCmdDraw-None-07647
        case ActionVUID::DYNAMIC_SHADING_RATE_IMAGE_ENABLE_07647: suffix = "None-07647"; break;
        // ### VUID-vkCmdDraw-None-07648
        case ActionVUID::DYNAMIC_REPRESENTATIVE_FRAGMENT_TEST_ENABLE_07648: suffix = "None-07648"; break;
        // ### VUID-vkCmdDraw-None-07649
        case ActionVUID::DYNAMIC_COVERAGE_REDUCTION_MODE_07649: suffix = "None-07649"; break;
        // ### VUID-vkCmdDraw-None-07841
        case ActionVUID::DYNAMIC_FRONT_FACE_07841: suffix = "None-07841"; break;
        // ### VUID-vkCmdDraw-viewportCount-03417
        case ActionVUID::DYNAMIC_VIEWPORT_COUNT_03417: suffix = "viewportCount-03417"; break;
        // ### VUID-vkCmdDraw-scissorCount-03418
        case ActionVUID::DYNAMIC_SCISSOR_COUNT_03418: suffix = "scissorCount-03418"; break;
        // ### VUID-vkCmdDraw-shadingRateImage-09233
        case ActionVUID::DYNAMIC_SET_VIEWPORT_COARSE_SAMPLE_ORDER_09233: suffix = "shadingRateImage-09233"; break;
        // ### VUID-vkCmdDraw-shadingRateImage-09234
        case ActionVUID::DYNAMIC_SET_VIEWPORT_SHADING_RATE_PALETTE_09234: suffix = "shadingRateImage-09234"; break;
        // ### VUID-vkCmdDraw-viewportCount-04138
        case ActionVUID::DYNAMIC_SET_CLIP_SPACE_W_SCALING_04138: suffix = "viewportCount-04138"; break;
        // ### VUID-vkCmdDraw-None-04875
        case ActionVUID::DYNAMIC_PATCH_CONTROL_POINTS_04875: suffix = "None-04875"; break;
        // ### VUID-vkCmdDraw-None-07880
        case ActionVUID::DYNAMIC_DISCARD_RECTANGLE_ENABLE_07880: suffix = "None-07880"; break;
        // ### VUID-vkCmdDraw-None-07881
        case ActionVUID::DYNAMIC_DISCARD_RECTANGLE_MODE_07881: suffix = "None-07881"; break;
        // ### VUID-vkCmdDraw-None-07849
        case ActionVUID::DYNAMIC_LINE_STIPPLE_EXT_07849: suffix = "None-07849"; break;
        // ### VUID-vkCmdDraw-None-08617
        case ActionVUID::DYNAMIC_SET_LINE_WIDTH_08617: suffix = "None-08617"; break;
        // ### VUID-vkCmdDraw-pStrides-04913
        case ActionVUID::DYNAMIC_VERTEX_INPUT_BINDING_STRIDE_04913: suffix = "pStrides-04913"; break;
        // ### VUID-vkCmdDraw-None-08666
        case ActionVUID::DYNAMIC_SET_LINE_RASTERIZATION_MODE_08666: suffix = "None-08666"; break;
        // ### VUID-vkCmdDraw-None-08669
        case ActionVUID::DYNAMIC_SET_LINE_STIPPLE_ENABLE_08669: suffix = "None-08669"; break;

        // ### VUID-vkCmdDraw-None-07751
        case ActionVUID::DISCARD_RECTANGLE_07751: suffix = "None-07751"; break;
        // ### VUID-vkCmdDraw-rasterizerDiscardEnable-09236
        case ActionVUID::DISCARD_RECTANGLE_09236: suffix = "rasterizerDiscardEnable-09236"; break;
        // ### VUID-vkCmdDraw-rasterizerDiscardEnable-09420
        case ActionVUID::COVERAGE_TO_COLOR_09420: suffix = "rasterizerDiscardEnable-09420"; break;
        // ### VUID-vkCmdDraw-coverageToColorEnable-07490
        case ActionVUID::COVERAGE_TO_COLOR_07490: suffix = "coverageToColorEnable-07490"; break;
        // ### VUID-vkCmdDraw-None-08644
        case ActionVUID::SET_RASTERIZATION_SAMPLES_08644: suffix = "None-08644"; break;
        // ### VUID-vkCmdDraw-attachmentCount-07750
        case ActionVUID::COLOR_WRITE_ENABLE_COUNT_07750: suffix = "attachmentCount-07750"; break;
        // ### VUID-vkCmdDraw-None-06479
        case ActionVUID::DEPTH_COMPARE_SAMPLE_06479: suffix = "None-06479"; break;
        // ### VUID-vkCmdDraw-None-06886
        case ActionVUID::DEPTH_READ_ONLY_06886: suffix = "None-06886"; break;
        // ### VUID-vkCmdDraw-None-06887
        case ActionVUID::STENCIL_READ_ONLY_06887: suffix = "None-06887"; break;
        // ### VUID-vkCmdDraw-alphaToCoverageEnable-08919
        case ActionVUID::ALPHA_TO_COVERAGE_COMPONENT_08919: suffix = "alphaToCoverageEnable-08919"; break;
        // ### VUID-vkCmdDraw-firstAttachment-07476
        case ActionVUID::COLOR_BLEND_ENABLE_07476: suffix = "firstAttachment-07476"; break;
        // ### VUID-vkCmdDraw-firstAttachment-07478
        case ActionVUID::COLOR_WRITE_MASK_07478: suffix = "firstAttachment-07478"; break;
        // ### VUID-vkCmdDraw-None-10862
        case ActionVUID::COLOR_BLEND_EQUATION_10862: suffix = "None-10862"; break;
        // ### VUID-vkCmdDraw-rasterizerDiscardEnable-10863
        case ActionVUID::COLOR_BLEND_EQUATION_10863: suffix = "rasterizerDiscardEnable-10863"; break;
        // ### VUID-vkCmdDraw-None-10864
        case ActionVUID::COLOR_BLEND_EQUATION_10864: suffix = "None-10864"; break;
        // ### VUID-vkCmdDraw-None-07831
        case ActionVUID::VIEWPORT_07831: suffix = "None-07831"; break;
        // ### VUID-vkCmdDraw-None-07832
        case ActionVUID::SCISSOR_07832: suffix = "None-07832"; break;
        // ### VUID-vkCmdDraw-None-07835
        case ActionVUID::BLEND_CONSTANTS_07835: suffix = "None-07835"; break;
        // ### VUID-vkCmdDraw-pDynamicStates-08715
        case ActionVUID::DEPTH_ENABLE_08715: suffix = "pDynamicStates-08715"; break;
        // ### VUID-vkCmdDraw-pDynamicStates-08716
        case ActionVUID::STENCIL_WRITE_MASK_08716: suffix = "pDynamicStates-08716"; break;
        // ### VUID-vkCmdDraw-None-07850
        case ActionVUID::STATE_INHERITED_07850: suffix = "None-07850"; break;
        // ### VUID-vkCmdDraw-None-09116
        case ActionVUID::COLOR_WRITE_MASK_09116: suffix = "None-09116"; break;
        // ### VUID-vkCmdDraw-None-10608
        case ActionVUID::LINE_RASTERIZATION_10608: suffix = "None-10608"; break;
        // ### VUID-vkCmdDraw-viewportCount-03419
        case ActionVUID::VIEWPORT_AND_SCISSOR_WITH_COUNT_03419: suffix = "viewportCount-03419"; break;
        // ### VUID-vkCmdDraw-None-08636
        case ActionVUID::VIEWPORT_W_SCALING_08636: suffix = "None-08636"; break;
        // ### VUID-vkCmdDraw-None-08637
        case ActionVUID::SHADING_RATE_PALETTE_08637: suffix = "None-08637"; break;
        // ### VUID-vkCmdDraw-viewportCount-09421
        case ActionVUID::SET_VIEWPORT_SWIZZLE_09421: suffix = "viewportCount-09421"; break;
        // ### VUID-vkCmdDraw-viewportCount-07493
        case ActionVUID::SET_VIEWPORT_SWIZZLE_07493: suffix = "viewportCount-07493"; break;
        // ### VUID-vkCmdDraw-alphaToCoverageEnable-08920
        case ActionVUID::ALPHA_COMPONENT_WORD_08920: suffix = "alphaToCoverageEnable-08920"; break;
        // ### VUID-vkCmdDraw-multiviewPerViewViewports-12262
        case ActionVUID::VIEWPORT_MULTIVIEW_12262: suffix = "multiviewPerViewViewports-12262"; break;
        // ### VUID-vkCmdDraw-multiviewPerViewViewports-12263
        case ActionVUID::SCISSOR_MULTIVIEW_12263: suffix = "multiviewPerViewViewports-12263"; break;
        // ### VUID-vkCmdDraw-conservativePointAndLineRasterization-07499
        case ActionVUID::CONVERVATIVE_RASTERIZATION_07499: suffix = "conservativePointAndLineRasterization-07499"; break;
        // ### VUID-vkCmdDraw-blendEnable-04727
        case ActionVUID::BLEND_ENABLE_04727: suffix = "blendEnable-04727"; break;
        // ### VUID-vkCmdDraw-maxFragmentDualSrcAttachments-09239
        case ActionVUID::BLEND_DUAL_SOURCE_09239: suffix = "maxFragmentDualSrcAttachments-09239"; break;
        // ### VUID-vkCmdDraw-advancedBlendMaxColorAttachments-07480
        case ActionVUID::BLEND_ADVANCED_07480: suffix = "advancedBlendMaxColorAttachments-07480"; break;
        // ### VUID-vkCmdDraw-primitivesGeneratedQueryWithNonZeroStreams-07481
        case ActionVUID::PRIMITIVES_GENERATED_QUERY_07481: suffix = "primitivesGeneratedQueryWithNonZeroStreams-07481"; break;
        // ### VUID-vkCmdDraw-sampleLocationsPerPixel-07482
        case ActionVUID::SAMPLE_LOCATIONS_07482: suffix = "sampleLocationsPerPixel-07482"; break;
        // ### VUID-vkCmdDraw-sampleLocationsPerPixel-07483
        case ActionVUID::SAMPLE_LOCATIONS_07483: suffix = "sampleLocationsPerPixel-07483"; break;
        // ### VUID-vkCmdDraw-rasterizationSamples-07471
        case ActionVUID::SAMPLE_LOCATIONS_07471: suffix = "rasterizationSamples-07471"; break;
        // ### VUID-vkCmdDraw-sampleLocationsEnable-07936
        case ActionVUID::SAMPLE_LOCATIONS_ENABLE_07936: suffix = "sampleLocationsEnable-07936"; break;
        // ### VUID-vkCmdDraw-sampleLocationsEnable-07937
        case ActionVUID::SAMPLE_LOCATIONS_ENABLE_07937: suffix = "sampleLocationsEnable-07937"; break;
        // ### VUID-vkCmdDraw-sampleLocationsEnable-07938
        case ActionVUID::SAMPLE_LOCATIONS_ENABLE_07938: suffix = "sampleLocationsEnable-07938"; break;
        // ### VUID-vkCmdDraw-samples-07472
        case ActionVUID::SAMPLE_MASK_07472: suffix = "samples-07472"; break;
        // ### VUID-vkCmdDraw-samples-07473
        case ActionVUID::SAMPLE_MASK_07473: suffix = "samples-07473"; break;
        // ### VUID-vkCmdDraw-dynamicPrimitiveTopologyUnrestricted-07500
        case ActionVUID::PRIMITIVE_TOPOLOGY_CLASS_07500: suffix = "dynamicPrimitiveTopologyUnrestricted-07500"; break;
        // ### VUID-vkCmdDraw-primitiveTopology-10286
        case ActionVUID::PRIMITIVE_TOPOLOGY_10286: suffix = "VUID-vkCmdDraw-primitiveTopology-10286"; break;
        // ### VUID-vkCmdDraw-primitiveTopology-10747
        case ActionVUID::PRIMITIVE_TOPOLOGY_10747: suffix = "VUID-vkCmdDraw-primitiveTopology-10747"; break;
        // ### VUID-vkCmdDraw-primitiveTopology-10748
        case ActionVUID::PRIMITIVE_TOPOLOGY_10748: suffix = "VUID-vkCmdDraw-primitiveTopology-10748"; break;
        // ### VUID-vkCmdDraw-None-09637
        case ActionVUID::PRIMITIVE_RESTART_09637: suffix = "VUID-vkCmdDraw-None-09637"; break;
        // ### VUID-vkCmdDraw-None-10909
        case ActionVUID::PRIMITIVE_RESTART_10909: suffix = "VUID-vkCmdDraw-None-10909"; break;
        // ### VUID-vkCmdDraw-Input-08734
        case ActionVUID::VERTEX_INPUT_08734: suffix = "VUID-vkCmdDraw-Input-08734"; break;
        // ### VUID-vkCmdDraw-format-08936
        case ActionVUID::VERTEX_INPUT_FORMAT_08936: suffix = "VUID-vkCmdDraw-format-08936"; break;
        // ### VUID-vkCmdDraw-format-08937
        case ActionVUID::VERTEX_INPUT_FORMAT_08937: suffix = "VUID-vkCmdDraw-format-08937"; break;
        // ### VUID-vkCmdDraw-None-09203
        case ActionVUID::VERTEX_INPUT_FORMAT_09203: suffix = "VUID-vkCmdDraw-None-09203"; break;
        // ### VUID-vkCmdDraw-Input-07939
        case ActionVUID::VERTEX_INPUT_FORMAT_07939: suffix = "VUID-vkCmdDraw-Input-07939"; break;
        // ### VUID-vkCmdDraw-commandBuffer-04617
        case ActionVUID::RAY_QUERY_04617: suffix = "commandBuffer-04617"; break;
        // ### VUID-vkCmdDraw-maxMultiviewInstanceIndex-02688
        case ActionVUID::MAX_MULTIVIEW_INSTANCE_INDEX_02688: suffix = "maxMultiviewInstanceIndex-02688"; break;
        // ### VUID-vkCmdDraw-primitiveFragmentShadingRateWithMultipleViewports-04552
        case ActionVUID::VIEWPORT_COUNT_PRIMITIVE_SHADING_RATE_04552: suffix = "primitiveFragmentShadingRateWithMultipleViewports-04552"; break;
        // ### VUID-vkCmdDraw-primitivesGeneratedQueryWithRasterizerDiscard-06708
        case ActionVUID::PRIMITIVES_GENERATED_06708: suffix = "primitivesGeneratedQueryWithRasterizerDiscard-06708"; break;
        // ### VUID-vkCmdDraw-primitivesGeneratedQueryWithNonZeroStreams-06709
        case ActionVUID::PRIMITIVES_GENERATED_STREAMS_06709: suffix = "primitivesGeneratedQueryWithNonZeroStreams-06709"; break;
        // ### VUID-vkCmdDraw-stage-06481
        case ActionVUID::INVALID_MESH_SHADER_STAGES_06481: suffix = "stage-06481"; break;
        // ### VUID-vkCmdDraw-None-10772
        case ActionVUID::SHADER_OBJECT_MULTIVIEW_10772: suffix = "None-10772"; break;
        // ### VUID-vkCmdDraw-nextStage-10745
        case ActionVUID::NEXT_STAGE_10745: suffix = "nextStage-10745"; break;
        // ### VUID-vkCmdDraw-None-08684
        case ActionVUID::VERTEX_SHADER_08684: suffix = "None-08684"; break;
        // ### VUID-vkCmdDraw-None-08685
        case ActionVUID::TESSELLATION_CONTROL_SHADER_08685: suffix = "None-08685"; break;
        // ### VUID-vkCmdDraw-None-08686
        case ActionVUID::TESSELLATION_EVALUATION_SHADER_08686: suffix = "None-08686"; break;
        // ### VUID-vkCmdDraw-None-08687
        case ActionVUID::GEOMETRY_SHADER_08687: suffix = "None-08687"; break;
        // ### VUID-vkCmdDraw-None-08688
        case ActionVUID::FRAGMENT_SHADER_08688: suffix = "None-08688"; break;
        // ### VUID-vkCmdDraw-None-08689
        case ActionVUID::TASK_SHADER_08689: suffix = "None-08689"; break;
        // ### VUID-vkCmdDraw-None-08690
        case ActionVUID::MESH_SHADER_08690: suffix = "None-08690"; break;
        // ### VUID-vkCmdDraw-None-08693
        case ActionVUID::VERT_MESH_SHADER_08693: suffix = "None-08693"; break;
        // ### VUID-vkCmdDraw-None-08696
        case ActionVUID::VERT_TASK_MESH_SHADER_08696: suffix = "None-08696"; break;
        // ### VUID-vkCmdDraw-None-08698
        case ActionVUID::LINKED_SHADERS_08698: suffix = "None-08698"; break;
        // ### VUID-vkCmdDraw-None-08699
        case ActionVUID::LINKED_SHADERS_08699: suffix = "None-08699"; break;
        // ### VUID-vkCmdDraw-None-08878
        case ActionVUID::SHADERS_PUSH_CONSTANTS_08878: suffix = "None-08878"; break;
        // ### VUID-vkCmdDraw-None-08879
        case ActionVUID::SHADERS_DESCRIPTOR_LAYOUTS_08879: suffix = "None-08879"; break;
        // ### VUID-vkCmdDraw-None-08885
        case ActionVUID::DRAW_SHADERS_NO_TASK_MESH_08885: suffix = "None-08885"; break;
        // ### VUID-vkCmdDraw-OpExecutionMode-12239
        case ActionVUID::TESSELLATION_SUBDIVISION_12239: suffix = "OpExecutionMode-12239"; break;
        // ### VUID-vkCmdDraw-OpExecutionMode-12240
        case ActionVUID::TESSELLATION_TRIANGLES_12240: suffix = "OpExecutionMode-12240"; break;
        // ### VUID-vkCmdDraw-OpExecutionMode-12241
        case ActionVUID::TESSELLATION_SEGMENT_12241: suffix = "OpExecutionMode-12241"; break;
        // ### VUID-vkCmdDraw-OpExecutionMode-12242
        case ActionVUID::TESSELLATION_PATCH_SIZE_12242: suffix = "OpExecutionMode-12242"; break;
        // ### VUID-vkCmdDraw-primitiveFragmentShadingRateWithMultipleViewports-08642
        case ActionVUID::SET_VIEWPORT_WITH_COUNT_08642: suffix = "primitiveFragmentShadingRateWithMultipleViewports-08642"; break;
        // ### VUID-vkCmdDraw-pNext-07935
        case ActionVUID::RASTERIZATION_SAMPLES_07935: suffix = "pNext-07935"; break;
        // ### VUID-vkCmdDraw-stage-07073
        case ActionVUID::MESH_SHADER_QUERIES_07073: suffix = "stage-07073"; break;
        // ### VUID-vkCmdDraw-layers-10831
        case ActionVUID::FDM_LAYERED_10831: suffix = "layers-10831"; break;
        // ### VUID-vkCmdDraw-pColorAttachments-08963
        case ActionVUID::COLOR_ATTACHMENT_08963: suffix = "pColorAttachments-08963"; break;
        // ### VUID-vkCmdDraw-pDepthAttachment-08964
        case ActionVUID::DEPTH_ATTACHMENT_08964: suffix = "pDepthAttachment-08964"; break;
        // ### VUID-vkCmdDraw-pStencilAttachment-08965
        case ActionVUID::STENCIL_ATTACHMENT_08965: suffix = "pStencilAttachment-08965"; break;
        // ### VUID-vkCmdDraw-pNext-09461
        case ActionVUID::VERTEX_INPUT_09461: suffix = "pNext-09461"; break;
        // ### VUID-vkCmdDraw-None-09462
        case ActionVUID::VERTEX_INPUT_09462: suffix = "None-09462"; break;
        // ### VUID-vkCmdDraw-flags-10582
        case ActionVUID::RENDERING_CONTENTS_10582: suffix = "flags-10582"; break;
        // ### VUID-vkCmdDraw-None-08876
        case ActionVUID::RENDER_PASS_BEGAN_08876: suffix = "None-08876"; break;
        // ### VUID-vkCmdDraw-unnormalizedCoordinates-09635
        case ActionVUID::UNNORMALIZED_COORDINATES_09635: suffix = "unnormalizedCoordinates-09635"; break;
        // ### VUID-vkCmdDraw-OpTypeTensorARM-09906
        case ActionVUID::SPIRV_OPTYPETENSORARM_09906: suffix = "OpTypeTensorARM-09906"; break;
        // ### VUID-vkCmdDraw-commandBuffer-10746
        case ActionVUID::TILE_MEMORY_HEAP_10746: suffix = "commandBuffer-10746"; break;
        // ### VUID-vkCmdDraw-protectedNoFault-13108
        case ActionVUID::PROTECTED_BUFFER_13108: suffix = "protectedNoFault-13108"; break;

        // ### VUID-vkCmdDraw-viewMask-06178
        case ActionVUID::DYNAMIC_RENDERING_VIEW_MASK_06178: suffix = "viewMask-06178"; break;
        // ### VUID-vkCmdDraw-colorAttachmentCount-06179
        case ActionVUID::DYNAMIC_RENDERING_COLOR_COUNT_06179: suffix = "colorAttachmentCount-06179"; break;
        // ### VUID-vkCmdDraw-dynamicRenderingUnusedAttachments-08910
        case ActionVUID::DYNAMIC_RENDERING_COLOR_FORMATS_08910: suffix = "dynamicRenderingUnusedAttachments-08910"; break;
        // ### VUID-vkCmdDraw-dynamicRenderingUnusedAttachments-08911
        case ActionVUID::DYNAMIC_RENDERING_UNUSED_ATTACHMENTS_08911: suffix = "dynamicRenderingUnusedAttachments-08911"; break;
        // ### VUID-vkCmdDraw-dynamicRenderingUnusedAttachments-08912
        case ActionVUID::DYNAMIC_RENDERING_UNDEFINED_COLOR_FORMATS_08912: suffix = "dynamicRenderingUnusedAttachments-08912"; break;
        // ### VUID-vkCmdDraw-dynamicRenderingUnusedAttachments-08913
        case ActionVUID::DYNAMIC_RENDERING_UNDEFINED_DEPTH_FORMAT_08913: suffix = "dynamicRenderingUnusedAttachments-08913"; break;
        // ### VUID-vkCmdDraw-dynamicRenderingUnusedAttachments-08916
        case ActionVUID::DYNAMIC_RENDERING_UNDEFINED_STENCIL_FORMAT_08916: suffix = "dynamicRenderingUnusedAttachments-08916"; break;
        // ### VUID-vkCmdDraw-dynamicRenderingUnusedAttachments-08914
        case ActionVUID::DYNAMIC_RENDERING_DEPTH_FORMAT_08914: suffix = "dynamicRenderingUnusedAttachments-08914"; break;
        // ### VUID-vkCmdDraw-dynamicRenderingUnusedAttachments-08915
        case ActionVUID::DYNAMIC_RENDERING_UNUSED_ATTACHMENTS_08915: suffix = "dynamicRenderingUnusedAttachments-08915"; break;
        // ### VUID-vkCmdDraw-dynamicRenderingUnusedAttachments-08917
        case ActionVUID::DYNAMIC_RENDERING_STENCIL_FORMAT_08917: suffix = "dynamicRenderingUnusedAttachments-08917"; break;
        // ### VUID-vkCmdDraw-dynamicRenderingUnusedAttachments-08918
        case ActionVUID::DYNAMIC_RENDERING_UNUSED_ATTACHMENTS_08918: suffix = "dynamicRenderingUnusedAttachments-08918"; break;
        // ### VUID-vkCmdDraw-imageView-06183
        case ActionVUID::DYNAMIC_RENDERING_FSR_06183: suffix = "imageView-06183"; break;
        // ### VUID-vkCmdDraw-imageView-06184
        case ActionVUID::DYNAMIC_RENDERING_FDM_06184: suffix = "imageView-06184"; break;
        // ### VUID-vkCmdDraw-colorAttachmentCount-06185
        case ActionVUID::DYNAMIC_RENDERING_COLOR_SAMPLE_06185: suffix = "colorAttachmentCount-06185"; break;
        // ### VUID-vkCmdDraw-pDepthAttachment-06186
        case ActionVUID::DYNAMIC_RENDERING_DEPTH_SAMPLE_06186: suffix = "pDepthAttachment-06186"; break;
        // ### VUID-vkCmdDraw-pStencilAttachment-06187
        case ActionVUID::DYNAMIC_RENDERING_STENCIL_SAMPLE_06187: suffix = "pStencilAttachment-06187"; break;
        // ### VUID-vkCmdDraw-renderPass-06198
        case ActionVUID::DYNAMIC_RENDERING_06198: suffix = "renderPass-06198"; break;
        // ### VUID-vkCmdDraw-multisampledRenderToSingleSampled-07285
        case ActionVUID::DYNAMIC_RENDERING_07285: suffix = "multisampledRenderToSingleSampled-07285"; break;
        // ### VUID-vkCmdDraw-multisampledRenderToSingleSampled-07286
        case ActionVUID::DYNAMIC_RENDERING_07286: suffix = "multisampledRenderToSingleSampled-07286"; break;
        // ### VUID-vkCmdDraw-multisampledRenderToSingleSampled-07287
        case ActionVUID::DYNAMIC_RENDERING_07287: suffix = "multisampledRenderToSingleSampled-07287"; break;
        // ### VUID-vkCmdDraw-None-09548
        case ActionVUID::DYNAMIC_RENDERING_LOCAL_LOCATION_09548: suffix = "None-09548"; break;
        // ### VUID-vkCmdDraw-None-09549
        case ActionVUID::DYNAMIC_RENDERING_LOCAL_INDEX_09549: suffix = "None-09549"; break;
        // ### VUID-vkCmdDraw-None-10927
        case ActionVUID::DYNAMIC_RENDERING_LOCAL_INDEX_10927: suffix = "None-10927"; break;
        // ### VUID-vkCmdDraw-None-10928
        case ActionVUID::DYNAMIC_RENDERING_LOCAL_INDEX_10928: suffix = "None-10928"; break;
        // ### VUID-vkCmdDraw-None-09642
        case ActionVUID::DYNAMIC_RENDERING_DITHERING_09642: suffix = "None-09642"; break;
        // ### VUID-vkCmdDraw-None-09643
        case ActionVUID::DYNAMIC_RENDERING_DITHERING_09643: suffix = "None-09643"; break;
        // ### VUID-vkCmdDraw-dynamicRenderingLocalRead-11797
        case ActionVUID::DYNAMIC_RENDERING_LOCAL_READ_11797: suffix = "dynamicRenderingLocalRead-11797"; break;

        // ### VUID-vkCmdDispatch-None-10743
        case ActionVUID::COMPUTE_NOT_BOUND_10743: suffix = "None-10743"; break;

        // ### VUID-vkCmdDrawMeshTasksEXT-stage-06480
        case ActionVUID::MESH_SHADER_STAGES_06480: suffix = "stage-06480"; break;
        // ### VUID-vkCmdDrawMeshTasksEXT-None-08694
        case ActionVUID::TASK_MESH_SHADER_08694: suffix = "None-08694"; break;
        // ### VUID-vkCmdDrawMeshTasksEXT-None-08695
        case ActionVUID::TASK_MESH_SHADER_08695: suffix = "None-08695"; break;
        // ### VUID-vkCmdDrawMeshTasksEXT-pStages-10680
        case ActionVUID::BOUND_NON_MESH_10680: suffix = "pStages-10680"; break;
        // ### VUID-vkCmdDrawMeshTasksEXT-None-07074
        case ActionVUID::XFB_QUERIES_07074: suffix = "None-07074"; break;
        // ### VUID-vkCmdDrawMeshTasksEXT-None-07075
        case ActionVUID::PG_QUERIES_07075: suffix = "None-07075"; break;
        // ### VUID-vkCmdDrawMeshTasksEXT-pipelineStatistics-07076
        case ActionVUID::PIPELINE_STATISTICS_QUERIES_07076: suffix = "pipelineStatistics-07076"; break;

        // ### VUID-vkCmdTraceRaysKHR-None-09458
        case ActionVUID::RTX_STACK_SIZE_09458: suffix = "None-09458"; break;
        // ### VUID-vkCmdTraceRaysKHR-commandBuffer-03635
        case ActionVUID::RAY_QUERY_PROTECT_03635: suffix = "commandBuffer-03635"; break;

    }
    // clang-format on

    if (!suffix) {
        return kVUIDUndefined;
    }
    // When c++20 is added, turn to std::format
    return std::string("VUID-") + String(function) + "-" + suffix;
}

// Helper to migrate from GetDrawDispatchVuid()
std::string CreateActionVuid(const DrawDispatchVuid& vuid, const ActionVUID id) { return CreateActionVuid(vuid.function, id); }

}  // namespace vvl
