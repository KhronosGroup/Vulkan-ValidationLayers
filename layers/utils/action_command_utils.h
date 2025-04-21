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

static inline bool IsCommandDrawMesh(vvl::Func command) {
    return
        command == vvl::Func::vkCmdDrawMeshTasksNV ||
        command == vvl::Func::vkCmdDrawMeshTasksIndirectNV ||
        command == vvl::Func::vkCmdDrawMeshTasksIndirectCountNV ||
        command == vvl::Func::vkCmdDrawMeshTasksEXT ||
        command == vvl::Func::vkCmdDrawMeshTasksIndirectEXT ||
        command == vvl::Func::vkCmdDrawMeshTasksIndirectCountEXT;
}

static inline bool IsCommandDrawVertexIndexed(vvl::Func command) {
    return
        command == vvl::Func::vkCmdDrawIndexed ||
        command == vvl::Func::vkCmdDrawMultiIndexedEXT ||
        command == vvl::Func::vkCmdDrawIndexedIndirect ||
        command == vvl::Func::vkCmdDrawIndexedIndirectCount ||
        command == vvl::Func::vkCmdDrawIndexedIndirectCountKHR;
}

static inline bool IsCommandDrawVertex(vvl::Func command) {
    return
        command == vvl::Func::vkCmdDraw ||
        command == vvl::Func::vkCmdDrawMultiEXT ||
        command == vvl::Func::vkCmdDrawIndexed ||
        command == vvl::Func::vkCmdDrawMultiIndexedEXT ||
        command == vvl::Func::vkCmdDrawIndirect ||
        command == vvl::Func::vkCmdDrawIndexedIndirect ||
        command == vvl::Func::vkCmdDrawIndirectCount ||
        command == vvl::Func::vkCmdDrawIndirectCountKHR ||
        command == vvl::Func::vkCmdDrawIndexedIndirectCount ||
        command == vvl::Func::vkCmdDrawIndexedIndirectCountKHR ||
        command == vvl::Func::vkCmdDrawIndirectByteCountEXT;
}

static inline bool IsCommandDispatch(vvl::Func command) {
    return
        command == vvl::Func::vkCmdDispatch ||
        command == vvl::Func::vkCmdDispatchIndirect ||
        command == vvl::Func::vkCmdDispatchBase ||
        command == vvl::Func::vkCmdDispatchBaseKHR ||
        command == vvl::Func::vkCmdDispatchGraphAMDX ||
        command == vvl::Func::vkCmdDispatchGraphIndirectAMDX ||
        command == vvl::Func::vkCmdDispatchGraphIndirectCountAMDX ||
        command == vvl::Func::vkCmdDispatchTileQCOM;
}

static inline bool IsCommandTraceRays(vvl::Func command) {
    return
        command == vvl::Func::vkCmdTraceRaysNV ||
        command == vvl::Func::vkCmdTraceRaysKHR ||
        command == vvl::Func::vkCmdTraceRaysIndirectKHR ||
        command == vvl::Func::vkCmdTraceRaysIndirect2KHR;
}

// clang-format on