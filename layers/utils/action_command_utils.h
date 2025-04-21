/* Copyright (c) 2025 The Khronos Group Inc.
 * Copyright (c) 2025 Valve Corporation
 * Copyright (c) 2025 LunarG, Inc.
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
#pragma once
#include "generated/error_location_helper.h"

// clang-format off

namespace vvl {

static inline bool IsCommandDrawMesh(Func command) {
    return
        command == Func::vkCmdDrawMeshTasksNV ||
        command == Func::vkCmdDrawMeshTasksIndirectNV ||
        command == Func::vkCmdDrawMeshTasksIndirectCountNV ||
        command == Func::vkCmdDrawMeshTasksEXT ||
        command == Func::vkCmdDrawMeshTasksIndirectEXT ||
        command == Func::vkCmdDrawMeshTasksIndirectCountEXT;
}

static inline bool IsCommandDrawVertexIndexed(Func command) {
    return
        command == Func::vkCmdDrawIndexed ||
        command == Func::vkCmdDrawMultiIndexedEXT ||
        command == Func::vkCmdDrawIndexedIndirect ||
        command == Func::vkCmdDrawIndexedIndirectCount ||
        command == Func::vkCmdDrawIndexedIndirectCountKHR;
}

static inline bool IsCommandDrawVertex(Func command) {
    return
        command == Func::vkCmdDraw ||
        command == Func::vkCmdDrawMultiEXT ||
        command == Func::vkCmdDrawIndexed ||
        command == Func::vkCmdDrawMultiIndexedEXT ||
        command == Func::vkCmdDrawIndirect ||
        command == Func::vkCmdDrawIndexedIndirect ||
        command == Func::vkCmdDrawIndirectCount ||
        command == Func::vkCmdDrawIndirectCountKHR ||
        command == Func::vkCmdDrawIndexedIndirectCount ||
        command == Func::vkCmdDrawIndexedIndirectCountKHR ||
        command == Func::vkCmdDrawIndirectByteCountEXT;
}

static inline bool IsCommandDispatch(Func command) {
    return
        command == Func::vkCmdDispatch ||
        command == Func::vkCmdDispatchIndirect ||
        command == Func::vkCmdDispatchBase ||
        command == Func::vkCmdDispatchBaseKHR ||
        command == Func::vkCmdDispatchGraphAMDX ||
        command == Func::vkCmdDispatchGraphIndirectAMDX ||
        command == Func::vkCmdDispatchGraphIndirectCountAMDX ||
        command == Func::vkCmdDispatchTileQCOM;
}

static inline bool IsCommandTraceRays(Func command) {
    return
        command == Func::vkCmdTraceRaysNV ||
        command == Func::vkCmdTraceRaysKHR ||
        command == Func::vkCmdTraceRaysIndirectKHR ||
        command == Func::vkCmdTraceRaysIndirect2KHR;
}

} // namespace vvl

// clang-format on