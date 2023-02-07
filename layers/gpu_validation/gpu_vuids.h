/* Copyright (c) 2021-2023 The Khronos Group Inc.
 * Copyright (c) 2021-2023 Valve Corporation
 * Copyright (c) 2021-2023 LunarG, Inc.
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

#include "gpu_validation/gpu_validation.h"

#pragma once
// clang-format off
struct GpuVuidsCmdDraw : GpuVuid {
    GpuVuidsCmdDraw() : GpuVuid() {
        uniform_access_oob = "VUID-vkCmdDraw-None-02705";
        storage_access_oob = "VUID-vkCmdDraw-None-02706";
    }
};

struct GpuVuidsCmdDrawMultiEXT : GpuVuid {
    GpuVuidsCmdDrawMultiEXT() : GpuVuid() {
        uniform_access_oob = "VUID-vkCmdDrawMultiEXT-None-02705";
        storage_access_oob = "VUID-vkCmdDrawMultiEXT-None-02706";
    }
};

struct GpuVuidsCmdDrawIndexed : GpuVuid {
    GpuVuidsCmdDrawIndexed() : GpuVuid() {
        uniform_access_oob = "VUID-vkCmdDrawIndexed-None-02705";
        storage_access_oob = "VUID-vkCmdDrawIndexed-None-02706";
    }
};

struct GpuVuidsCmdDrawMultiIndexedEXT : GpuVuid {
    GpuVuidsCmdDrawMultiIndexedEXT() : GpuVuid() {
        uniform_access_oob = "VUID-vkCmdDrawMultiIndexedEXT-None-02705";
        storage_access_oob = "VUID-vkCmdDrawMultiIndexedEXT-None-02706";
    }
};

struct GpuVuidsCmdDrawIndirect : GpuVuid {
    GpuVuidsCmdDrawIndirect() : GpuVuid() {
        uniform_access_oob = "VUID-vkCmdDrawIndirect-None-02705";
        storage_access_oob = "VUID-vkCmdDrawIndirect-None-02706";
        first_instance_not_zero = "VUID-VkDrawIndirectCommand-firstInstance-00501";
    }
};

struct GpuVuidsCmdDrawIndexedIndirect : GpuVuid {
    GpuVuidsCmdDrawIndexedIndirect() : GpuVuid() {
        uniform_access_oob = "VUID-vkCmdDrawIndexedIndirect-None-02705";
        storage_access_oob = "VUID-vkCmdDrawIndexedIndirect-None-02706";
        first_instance_not_zero = "VUID-VkDrawIndexedIndirectCommand-firstInstance-00554";
    }
};

struct GpuVuidsCmdDispatch : GpuVuid {
    GpuVuidsCmdDispatch() : GpuVuid() {
        uniform_access_oob = "VUID-vkCmdDispatch-None-02705";
        storage_access_oob = "VUID-vkCmdDispatch-None-02706";
    }
};

struct GpuVuidsCmdDispatchIndirect : GpuVuid {
    GpuVuidsCmdDispatchIndirect() : GpuVuid() {
        uniform_access_oob = "VUID-vkCmdDispatchIndirect-None-02705";
        storage_access_oob = "VUID-vkCmdDispatchIndirect-None-02706";
        group_exceeds_device_limit_x = "VUID-VkDispatchIndirectCommand-x-00417";
        group_exceeds_device_limit_y = "VUID-VkDispatchIndirectCommand-y-00418";
        group_exceeds_device_limit_z = "VUID-VkDispatchIndirectCommand-z-00419";

    }
};

struct GpuVuidsCmdDrawIndirectCount : GpuVuid {
    GpuVuidsCmdDrawIndirectCount() : GpuVuid() {
        uniform_access_oob = "VUID-vkCmdDrawIndirectCount-None-02705";
        storage_access_oob = "VUID-vkCmdDrawIndirectCount-None-02706";
        count_exceeds_bufsize_1 = "VUID-vkCmdDrawIndirectCount-countBuffer-03121";
        count_exceeds_bufsize = "VUID-vkCmdDrawIndirectCount-countBuffer-03122";
        count_exceeds_device_limit = "VUID-vkCmdDrawIndirectCount-countBuffer-02717";
    }
};

struct GpuVuidsCmdDrawIndexedIndirectCount : GpuVuid {
    GpuVuidsCmdDrawIndexedIndirectCount() : GpuVuid() {
        uniform_access_oob = "VUID-vkCmdDrawIndexedIndirectCount-None-02705";
        storage_access_oob = "VUID-vkCmdDrawIndexedIndirectCount-None-02706";
        count_exceeds_bufsize_1 = "VUID-vkCmdDrawIndexedIndirectCount-countBuffer-03153";
        count_exceeds_bufsize = "VUID-vkCmdDrawIndexedIndirectCount-countBuffer-03154";
        count_exceeds_device_limit = "VUID-vkCmdDrawIndexedIndirectCount-countBuffer-02717";
    }
};

struct GpuVuidsCmdTraceRaysNV : GpuVuid {
    GpuVuidsCmdTraceRaysNV() : GpuVuid() {
        uniform_access_oob = "VUID-vkCmdTraceRaysNV-None-02705";
        storage_access_oob = "VUID-vkCmdTraceRaysNV-None-02706";
    }
};

struct GpuVuidsCmdTraceRaysKHR : GpuVuid {
    GpuVuidsCmdTraceRaysKHR() : GpuVuid() {
        uniform_access_oob = "VUID-vkCmdTraceRaysKHR-None-02705";
        storage_access_oob = "VUID-vkCmdTraceRaysKHR-None-02706";
    }
};

struct GpuVuidsCmdTraceRaysIndirectKHR : GpuVuid {
    GpuVuidsCmdTraceRaysIndirectKHR() : GpuVuid() {
        uniform_access_oob = "VUID-vkCmdTraceRaysIndirectKHR-None-02705";
        storage_access_oob = "VUID-vkCmdTraceRaysIndirectKHR-None-02706";
    }
};

struct GpuVuidsCmdTraceRaysIndirect2KHR : GpuVuid {
    GpuVuidsCmdTraceRaysIndirect2KHR() : GpuVuid() {
        uniform_access_oob = "VUID-vkCmdTraceRaysIndirect2KHR-None-02705";
        storage_access_oob = "VUID-vkCmdTraceRaysIndirect2KHR-None-02706";
    }
};

struct GpuVuidsCmdDrawMeshTasksNV : GpuVuid {
    GpuVuidsCmdDrawMeshTasksNV() : GpuVuid() {
        uniform_access_oob = "VUID-vkCmdDrawMeshTasksNV-None-02705";
        storage_access_oob = "VUID-vkCmdDrawMeshTasksNV-None-02706";
    }
};

struct GpuVuidsCmdDrawMeshTasksIndirectNV : GpuVuid {
    GpuVuidsCmdDrawMeshTasksIndirectNV() : GpuVuid() {
        uniform_access_oob = "VUID-vkCmdDrawMeshTasksIndirectNV-None-02705";
        storage_access_oob = "VUID-vkCmdDrawMeshTasksIndirectNV-None-02706";
    }
};

struct GpuVuidsCmdDrawMeshTasksIndirectCountNV : GpuVuid {
    GpuVuidsCmdDrawMeshTasksIndirectCountNV() : GpuVuid() {
        uniform_access_oob = "VUID-vkCmdDrawMeshTasksIndirectCountNV-None-02705";
        storage_access_oob = "VUID-vkCmdDrawMeshTasksIndirectCountNV-None-02706";
    }
};

struct GpuVuidsCmdDrawIndirectByteCountEXT : GpuVuid {
    GpuVuidsCmdDrawIndirectByteCountEXT() : GpuVuid() {
        uniform_access_oob = "VUID-vkCmdDrawIndirectByteCountEXT-None-02705";
        storage_access_oob = "VUID-vkCmdDrawIndirectByteCountEXT-None-02706";
    }
};

struct GpuVuidsCmdDispatchBase : GpuVuid {
    GpuVuidsCmdDispatchBase() : GpuVuid() {
        uniform_access_oob = "VUID-vkCmdDispatchBase-None-02705";
        storage_access_oob = "VUID-vkCmdDispatchBase-None-02706";
    }
};

// This LUT is created to allow a static listing of each VUID that is covered by drawdispatch commands
static const std::map<CMD_TYPE, GpuVuid> gpu_vuid = {
    {CMD_DRAW, GpuVuidsCmdDraw()},
    {CMD_DRAWMULTIEXT, GpuVuidsCmdDrawMultiEXT()},
    {CMD_DRAWINDEXED, GpuVuidsCmdDrawIndexed()},
    {CMD_DRAWMULTIINDEXEDEXT, GpuVuidsCmdDrawMultiIndexedEXT()},
    {CMD_DRAWINDIRECT, GpuVuidsCmdDrawIndirect()},
    {CMD_DRAWINDEXEDINDIRECT, GpuVuidsCmdDrawIndexedIndirect()},
    {CMD_DISPATCH, GpuVuidsCmdDispatch()},
    {CMD_DISPATCHINDIRECT, GpuVuidsCmdDispatchIndirect()},
    {CMD_DRAWINDIRECTCOUNT, GpuVuidsCmdDrawIndirectCount()},
    {CMD_DRAWINDIRECTCOUNTKHR, GpuVuidsCmdDrawIndirectCount()},
    {CMD_DRAWINDEXEDINDIRECTCOUNT, GpuVuidsCmdDrawIndexedIndirectCount()},
    {CMD_DRAWINDEXEDINDIRECTCOUNTKHR, GpuVuidsCmdDrawIndexedIndirectCount()},
    {CMD_TRACERAYSNV, GpuVuidsCmdTraceRaysNV()},
    {CMD_TRACERAYSKHR, GpuVuidsCmdTraceRaysKHR()},
    {CMD_TRACERAYSINDIRECTKHR, GpuVuidsCmdTraceRaysIndirectKHR()},
    {CMD_TRACERAYSINDIRECT2KHR, GpuVuidsCmdTraceRaysIndirect2KHR()},
    {CMD_DRAWMESHTASKSNV, GpuVuidsCmdDrawMeshTasksNV()},
    {CMD_DRAWMESHTASKSINDIRECTNV, GpuVuidsCmdDrawMeshTasksIndirectNV()},
    {CMD_DRAWMESHTASKSINDIRECTCOUNTNV, GpuVuidsCmdDrawMeshTasksIndirectCountNV()},
    {CMD_DRAWINDIRECTBYTECOUNTEXT, GpuVuidsCmdDrawIndirectByteCountEXT()},
    {CMD_DISPATCHBASE, GpuVuidsCmdDispatchBase()},
    {CMD_DISPATCHBASEKHR, GpuVuidsCmdDispatchBase()},
    // Used if invalid cmd_type is used
    {CMD_NONE, GpuVuid()}
};

// Getter function to provide kVUIDUndefined in case an invalid cmd_type is passed in
const GpuVuid &GetGpuVuid(CMD_TYPE cmd_type) {
    if (gpu_vuid.find(cmd_type) != gpu_vuid.cend()) {
        return gpu_vuid.at(cmd_type);
    }
    else {
        return gpu_vuid.at(CMD_NONE);
    }
}
// clang-format on
