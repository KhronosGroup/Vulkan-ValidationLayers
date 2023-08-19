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
        uniform_access_oob = "VUID-vkCmdDraw-None-08612";
        storage_access_oob = "VUID-vkCmdDraw-None-08613";
    }
};

struct GpuVuidsCmdDrawMultiEXT : GpuVuid {
    GpuVuidsCmdDrawMultiEXT() : GpuVuid() {
        uniform_access_oob = "VUID-vkCmdDrawMultiEXT-None-08612";
        storage_access_oob = "VUID-vkCmdDrawMultiEXT-None-08613";
    }
};

struct GpuVuidsCmdDrawIndexed : GpuVuid {
    GpuVuidsCmdDrawIndexed() : GpuVuid() {
        uniform_access_oob = "VUID-vkCmdDrawIndexed-None-08612";
        storage_access_oob = "VUID-vkCmdDrawIndexed-None-08613";
    }
};

struct GpuVuidsCmdDrawMultiIndexedEXT : GpuVuid {
    GpuVuidsCmdDrawMultiIndexedEXT() : GpuVuid() {
        uniform_access_oob = "VUID-vkCmdDrawMultiIndexedEXT-None-08612";
        storage_access_oob = "VUID-vkCmdDrawMultiIndexedEXT-None-08613";
    }
};

struct GpuVuidsCmdDrawIndirect : GpuVuid {
    GpuVuidsCmdDrawIndirect() : GpuVuid() {
        uniform_access_oob = "VUID-vkCmdDrawIndirect-None-08612";
        storage_access_oob = "VUID-vkCmdDrawIndirect-None-08613";
        first_instance_not_zero = "VUID-VkDrawIndirectCommand-firstInstance-00501";
    }
};

struct GpuVuidsCmdDrawIndexedIndirect : GpuVuid {
    GpuVuidsCmdDrawIndexedIndirect() : GpuVuid() {
        uniform_access_oob = "VUID-vkCmdDrawIndexedIndirect-None-08612";
        storage_access_oob = "VUID-vkCmdDrawIndexedIndirect-None-08613";
        first_instance_not_zero = "VUID-VkDrawIndexedIndirectCommand-firstInstance-00554";
    }
};

struct GpuVuidsCmdDispatch : GpuVuid {
    GpuVuidsCmdDispatch() : GpuVuid() {
        uniform_access_oob = "VUID-vkCmdDispatch-None-08612";
        storage_access_oob = "VUID-vkCmdDispatch-None-08613";
    }
};

struct GpuVuidsCmdDispatchIndirect : GpuVuid {
    GpuVuidsCmdDispatchIndirect() : GpuVuid() {
        uniform_access_oob = "VUID-vkCmdDispatchIndirect-None-08612";
        storage_access_oob = "VUID-vkCmdDispatchIndirect-None-08613";
        group_exceeds_device_limit_x = "VUID-VkDispatchIndirectCommand-x-00417";
        group_exceeds_device_limit_y = "VUID-VkDispatchIndirectCommand-y-00418";
        group_exceeds_device_limit_z = "VUID-VkDispatchIndirectCommand-z-00419";

    }
};

struct GpuVuidsCmdDrawIndirectCount : GpuVuid {
    GpuVuidsCmdDrawIndirectCount() : GpuVuid() {
        uniform_access_oob = "VUID-vkCmdDrawIndirectCount-None-08612";
        storage_access_oob = "VUID-vkCmdDrawIndirectCount-None-08613";
        count_exceeds_bufsize_1 = "VUID-vkCmdDrawIndirectCount-countBuffer-03121";
        count_exceeds_bufsize = "VUID-vkCmdDrawIndirectCount-countBuffer-03122";
        count_exceeds_device_limit = "VUID-vkCmdDrawIndirectCount-countBuffer-02717";
    }
};

struct GpuVuidsCmdDrawIndexedIndirectCount : GpuVuid {
    GpuVuidsCmdDrawIndexedIndirectCount() : GpuVuid() {
        uniform_access_oob = "VUID-vkCmdDrawIndexedIndirectCount-None-08612";
        storage_access_oob = "VUID-vkCmdDrawIndexedIndirectCount-None-08613";
        count_exceeds_bufsize_1 = "VUID-vkCmdDrawIndexedIndirectCount-countBuffer-03153";
        count_exceeds_bufsize = "VUID-vkCmdDrawIndexedIndirectCount-countBuffer-03154";
        count_exceeds_device_limit = "VUID-vkCmdDrawIndexedIndirectCount-countBuffer-02717";
    }
};

struct GpuVuidsCmdTraceRaysNV : GpuVuid {
    GpuVuidsCmdTraceRaysNV() : GpuVuid() {
        uniform_access_oob = "VUID-vkCmdTraceRaysNV-None-08612";
        storage_access_oob = "VUID-vkCmdTraceRaysNV-None-08613";
    }
};

struct GpuVuidsCmdTraceRaysKHR : GpuVuid {
    GpuVuidsCmdTraceRaysKHR() : GpuVuid() {
        uniform_access_oob = "VUID-vkCmdTraceRaysKHR-None-08612";
        storage_access_oob = "VUID-vkCmdTraceRaysKHR-None-08613";
    }
};

struct GpuVuidsCmdTraceRaysIndirectKHR : GpuVuid {
    GpuVuidsCmdTraceRaysIndirectKHR() : GpuVuid() {
        uniform_access_oob = "VUID-vkCmdTraceRaysIndirectKHR-None-08612";
        storage_access_oob = "VUID-vkCmdTraceRaysIndirectKHR-None-08613";
    }
};

struct GpuVuidsCmdTraceRaysIndirect2KHR : GpuVuid {
    GpuVuidsCmdTraceRaysIndirect2KHR() : GpuVuid() {
        uniform_access_oob = "VUID-vkCmdTraceRaysIndirect2KHR-None-08612";
        storage_access_oob = "VUID-vkCmdTraceRaysIndirect2KHR-None-08613";
    }
};

struct GpuVuidsCmdDrawMeshTasksNV : GpuVuid {
    GpuVuidsCmdDrawMeshTasksNV() : GpuVuid() {
        uniform_access_oob = "VUID-vkCmdDrawMeshTasksNV-None-08612";
        storage_access_oob = "VUID-vkCmdDrawMeshTasksNV-None-08613";
    }
};

struct GpuVuidsCmdDrawMeshTasksIndirectNV : GpuVuid {
    GpuVuidsCmdDrawMeshTasksIndirectNV() : GpuVuid() {
        uniform_access_oob = "VUID-vkCmdDrawMeshTasksIndirectNV-None-08612";
        storage_access_oob = "VUID-vkCmdDrawMeshTasksIndirectNV-None-08613";
    }
};

struct GpuVuidsCmdDrawMeshTasksIndirectCountNV : GpuVuid {
    GpuVuidsCmdDrawMeshTasksIndirectCountNV() : GpuVuid() {
        uniform_access_oob = "VUID-vkCmdDrawMeshTasksIndirectCountNV-None-08612";
        storage_access_oob = "VUID-vkCmdDrawMeshTasksIndirectCountNV-None-08613";
    }
};

struct GpuVuidsCmdDrawIndirectByteCountEXT : GpuVuid {
    GpuVuidsCmdDrawIndirectByteCountEXT() : GpuVuid() {
        uniform_access_oob = "VUID-vkCmdDrawIndirectByteCountEXT-None-08612";
        storage_access_oob = "VUID-vkCmdDrawIndirectByteCountEXT-None-08613";
    }
};

struct GpuVuidsCmdDispatchBase : GpuVuid {
    GpuVuidsCmdDispatchBase() : GpuVuid() {
        uniform_access_oob = "VUID-vkCmdDispatchBase-None-08612";
        storage_access_oob = "VUID-vkCmdDispatchBase-None-08613";
    }
};

using Func = vvl::Func;
// This LUT is created to allow a static listing of each VUID that is covered by drawdispatch commands
static const std::map<Func, GpuVuid> gpu_vuid = {
    {Func::vkCmdDraw, GpuVuidsCmdDraw()},
    {Func::vkCmdDrawMultiEXT, GpuVuidsCmdDrawMultiEXT()},
    {Func::vkCmdDrawIndexed, GpuVuidsCmdDrawIndexed()},
    {Func::vkCmdDrawMultiIndexedEXT, GpuVuidsCmdDrawMultiIndexedEXT()},
    {Func::vkCmdDrawIndirect, GpuVuidsCmdDrawIndirect()},
    {Func::vkCmdDrawIndexedIndirect, GpuVuidsCmdDrawIndexedIndirect()},
    {Func::vkCmdDispatch, GpuVuidsCmdDispatch()},
    {Func::vkCmdDispatchIndirect, GpuVuidsCmdDispatchIndirect()},
    {Func::vkCmdDrawIndirectCount, GpuVuidsCmdDrawIndirectCount()},
    {Func::vkCmdDrawIndirectCountKHR, GpuVuidsCmdDrawIndirectCount()},
    {Func::vkCmdDrawIndexedIndirectCount, GpuVuidsCmdDrawIndexedIndirectCount()},
    {Func::vkCmdDrawIndexedIndirectCountKHR, GpuVuidsCmdDrawIndexedIndirectCount()},
    {Func::vkCmdTraceRaysNV, GpuVuidsCmdTraceRaysNV()},
    {Func::vkCmdTraceRaysKHR, GpuVuidsCmdTraceRaysKHR()},
    {Func::vkCmdTraceRaysIndirectKHR, GpuVuidsCmdTraceRaysIndirectKHR()},
    {Func::vkCmdTraceRaysIndirect2KHR, GpuVuidsCmdTraceRaysIndirect2KHR()},
    {Func::vkCmdDrawMeshTasksNV, GpuVuidsCmdDrawMeshTasksNV()},
    {Func::vkCmdDrawMeshTasksIndirectNV, GpuVuidsCmdDrawMeshTasksIndirectNV()},
    {Func::vkCmdDrawMeshTasksIndirectCountNV, GpuVuidsCmdDrawMeshTasksIndirectCountNV()},
    {Func::vkCmdDrawIndirectByteCountEXT, GpuVuidsCmdDrawIndirectByteCountEXT()},
    {Func::vkCmdDispatchBase, GpuVuidsCmdDispatchBase()},
    {Func::vkCmdDispatchBaseKHR, GpuVuidsCmdDispatchBase()},
    // Used if invalid function is used
    {Func::Empty, GpuVuid()}
};

// Getter function to provide kVUIDUndefined in case an invalid function is passed in
const GpuVuid &GetGpuVuid(Func command) {
    if (gpu_vuid.find(command) != gpu_vuid.cend()) {
        return gpu_vuid.at(command);
    }
    else {
        return gpu_vuid.at(Func::Empty);
    }
}
// clang-format on
