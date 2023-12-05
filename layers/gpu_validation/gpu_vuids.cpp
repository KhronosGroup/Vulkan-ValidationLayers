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
#include "gpu_vuids.h"

// clang-format off
struct GpuVuidsCmdDraw : gpuav::GpuVuid {
    GpuVuidsCmdDraw() : GpuVuid() {
        uniform_access_oob = "VUID-vkCmdDraw-None-08612";
        storage_access_oob = "VUID-vkCmdDraw-None-08613";
        invalid_descriptor = "VUID-vkCmdDraw-None-08114";
    }
};

struct GpuVuidsCmdDrawMultiEXT : gpuav::GpuVuid {
    GpuVuidsCmdDrawMultiEXT() : GpuVuid() {
        uniform_access_oob = "VUID-vkCmdDrawMultiEXT-None-08612";
        storage_access_oob = "VUID-vkCmdDrawMultiEXT-None-08613";
        invalid_descriptor = "VUID-vkCmdDrawMultiEXT-None-08114";
    }
};

struct GpuVuidsCmdDrawIndexed : gpuav::GpuVuid {
    GpuVuidsCmdDrawIndexed() : GpuVuid() {
        uniform_access_oob = "VUID-vkCmdDrawIndexed-None-08612";
        storage_access_oob = "VUID-vkCmdDrawIndexed-None-08613";
        invalid_descriptor = "VUID-vkCmdDrawIndexed-None-08114";
    }
};

struct GpuVuidsCmdDrawMultiIndexedEXT : gpuav::GpuVuid {
    GpuVuidsCmdDrawMultiIndexedEXT() : GpuVuid() {
        uniform_access_oob = "VUID-vkCmdDrawMultiIndexedEXT-None-08612";
        storage_access_oob = "VUID-vkCmdDrawMultiIndexedEXT-None-08613";
        invalid_descriptor = "VUID-vkCmdDrawMultiIndexedEXT-None-08114";
    }
};

struct GpuVuidsCmdDrawIndirect : gpuav::GpuVuid {
    GpuVuidsCmdDrawIndirect() : GpuVuid() {
        uniform_access_oob = "VUID-vkCmdDrawIndirect-None-08612";
        storage_access_oob = "VUID-vkCmdDrawIndirect-None-08613";
        invalid_descriptor = "VUID-vkCmdDrawIndirect-None-08114";
        first_instance_not_zero = "VUID-VkDrawIndirectCommand-firstInstance-00501";
    }
};

struct GpuVuidsCmdDrawIndexedIndirect : gpuav::GpuVuid {
    GpuVuidsCmdDrawIndexedIndirect() : GpuVuid() {
        uniform_access_oob = "VUID-vkCmdDrawIndexedIndirect-None-08612";
        storage_access_oob = "VUID-vkCmdDrawIndexedIndirect-None-08613";
        invalid_descriptor = "VUID-vkCmdDrawIndexedIndirect-None-08114";
        first_instance_not_zero = "VUID-VkDrawIndexedIndirectCommand-firstInstance-00554";
    }
};

struct GpuVuidsCmdDispatch : gpuav::GpuVuid {
    GpuVuidsCmdDispatch() : GpuVuid() {
        uniform_access_oob = "VUID-vkCmdDispatch-None-08612";
        storage_access_oob = "VUID-vkCmdDispatch-None-08613";
        invalid_descriptor = "VUID-vkCmdDispatch-None-08114";
    }
};

struct GpuVuidsCmdDispatchIndirect : gpuav::GpuVuid {
    GpuVuidsCmdDispatchIndirect() : GpuVuid() {
        uniform_access_oob = "VUID-vkCmdDispatchIndirect-None-08612";
        storage_access_oob = "VUID-vkCmdDispatchIndirect-None-08613";
        invalid_descriptor = "VUID-vkCmdDispatchIndirect-None-08114";
        group_exceeds_device_limit_x = "VUID-VkDispatchIndirectCommand-x-00417";
        group_exceeds_device_limit_y = "VUID-VkDispatchIndirectCommand-y-00418";
        group_exceeds_device_limit_z = "VUID-VkDispatchIndirectCommand-z-00419";

    }
};

struct GpuVuidsCmdDrawIndirectCount : gpuav::GpuVuid {
    GpuVuidsCmdDrawIndirectCount() : GpuVuid() {
        uniform_access_oob = "VUID-vkCmdDrawIndirectCount-None-08612";
        storage_access_oob = "VUID-vkCmdDrawIndirectCount-None-08613";
        invalid_descriptor = "VUID-vkCmdDrawIndirectCount-None-08114";
        count_exceeds_bufsize_1 = "VUID-vkCmdDrawIndirectCount-countBuffer-03121";
        count_exceeds_bufsize = "VUID-vkCmdDrawIndirectCount-countBuffer-03122";
        count_exceeds_device_limit = "VUID-vkCmdDrawIndirectCount-countBuffer-02717";
    }
};

struct GpuVuidsCmdDrawIndexedIndirectCount : gpuav::GpuVuid {
    GpuVuidsCmdDrawIndexedIndirectCount() : GpuVuid() {
        uniform_access_oob = "VUID-vkCmdDrawIndexedIndirectCount-None-08612";
        storage_access_oob = "VUID-vkCmdDrawIndexedIndirectCount-None-08613";
        invalid_descriptor = "VUID-vkCmdDrawIndexedIndirectCount-None-08114";
        count_exceeds_bufsize_1 = "VUID-vkCmdDrawIndexedIndirectCount-countBuffer-03153";
        count_exceeds_bufsize = "VUID-vkCmdDrawIndexedIndirectCount-countBuffer-03154";
        count_exceeds_device_limit = "VUID-vkCmdDrawIndexedIndirectCount-countBuffer-02717";
    }
};

struct GpuVuidsCmdTraceRaysNV : gpuav::GpuVuid {
    GpuVuidsCmdTraceRaysNV() : GpuVuid() {
        uniform_access_oob = "VUID-vkCmdTraceRaysNV-None-08612";
        storage_access_oob = "VUID-vkCmdTraceRaysNV-None-08613";
        invalid_descriptor = "VUID-vkCmdTraceRaysNV-None-08114";
    }
};

struct GpuVuidsCmdTraceRaysKHR : gpuav::GpuVuid {
    GpuVuidsCmdTraceRaysKHR() : GpuVuid() {
        uniform_access_oob = "VUID-vkCmdTraceRaysKHR-None-08612";
        storage_access_oob = "VUID-vkCmdTraceRaysKHR-None-08613";
        invalid_descriptor = "VUID-vkCmdTraceRaysKHR-None-08114";
    }
};

struct GpuVuidsCmdTraceRaysIndirectKHR : gpuav::GpuVuid {
    GpuVuidsCmdTraceRaysIndirectKHR() : GpuVuid() {
        uniform_access_oob = "VUID-vkCmdTraceRaysIndirectKHR-None-08612";
        storage_access_oob = "VUID-vkCmdTraceRaysIndirectKHR-None-08613";
        invalid_descriptor = "VUID-vkCmdTraceRaysIndirectKHR-None-08114";
        trace_rays_width_exceeds_device_limit = "VUID-VkTraceRaysIndirectCommandKHR-width-03638";
        trace_rays_height_exceeds_device_limit = "VUID-VkTraceRaysIndirectCommandKHR-height-03639";
        trace_rays_depth_exceeds_device_limit = "VUID-VkTraceRaysIndirectCommandKHR-depth-03640";
    }
};

struct GpuVuidsCmdTraceRaysIndirect2KHR : gpuav::GpuVuid {
    GpuVuidsCmdTraceRaysIndirect2KHR() : GpuVuid() {
        uniform_access_oob = "VUID-vkCmdTraceRaysIndirect2KHR-None-08612";
        storage_access_oob = "VUID-vkCmdTraceRaysIndirect2KHR-None-08613";
        invalid_descriptor = "VUID-vkCmdTraceRaysIndirect2KHR-None-08114";
    }
};

struct GpuVuidsCmdDrawMeshTasksNV : gpuav::GpuVuid {
    GpuVuidsCmdDrawMeshTasksNV() : GpuVuid() {
        uniform_access_oob = "VUID-vkCmdDrawMeshTasksNV-None-08612";
        storage_access_oob = "VUID-vkCmdDrawMeshTasksNV-None-08613";
        invalid_descriptor = "VUID-vkCmdDrawMeshTasksNV-None-08114";
    }
};

struct GpuVuidsCmdDrawMeshTasksIndirectNV : gpuav::GpuVuid {
    GpuVuidsCmdDrawMeshTasksIndirectNV() : GpuVuid() {
        uniform_access_oob = "VUID-vkCmdDrawMeshTasksIndirectNV-None-08612";
        storage_access_oob = "VUID-vkCmdDrawMeshTasksIndirectNV-None-08613";
        invalid_descriptor = "VUID-vkCmdDrawMeshTasksIndirectNV-None-08114";
        task_group_count_exceeds_max_x = "VUID-VkDrawMeshTasksIndirectCommandEXT-TaskEXT-07322";
        task_group_count_exceeds_max_y = "VUID-VkDrawMeshTasksIndirectCommandEXT-TaskEXT-07323";
        task_group_count_exceeds_max_z = "VUID-VkDrawMeshTasksIndirectCommandEXT-TaskEXT-07324";
        task_group_count_exceeds_max_total = "VUID-VkDrawMeshTasksIndirectCommandEXT-TaskEXT-07325";
        mesh_group_count_exceeds_max_x = "VUID-VkDrawMeshTasksIndirectCommandEXT-TaskEXT-07326";
        mesh_group_count_exceeds_max_y = "VUID-VkDrawMeshTasksIndirectCommandEXT-TaskEXT-07327";
        mesh_group_count_exceeds_max_z = "VUID-VkDrawMeshTasksIndirectCommandEXT-TaskEXT-07328";
        mesh_group_count_exceeds_max_total = "VUID-VkDrawMeshTasksIndirectCommandEXT-TaskEXT-07329";
    }
};

struct GpuVuidsCmdDrawMeshTasksIndirectCountNV : gpuav::GpuVuid {
    GpuVuidsCmdDrawMeshTasksIndirectCountNV() : GpuVuid() {
        uniform_access_oob = "VUID-vkCmdDrawMeshTasksIndirectCountNV-None-08612";
        storage_access_oob = "VUID-vkCmdDrawMeshTasksIndirectCountNV-None-08613";
        invalid_descriptor = "VUID-vkCmdDrawMeshTasksIndirectCountNV-None-08114";
        count_exceeds_bufsize_1 = "VUID-vkCmdDrawMeshTasksIndirectCountNV-countBuffer-02191";
        count_exceeds_bufsize = "VUID-vkCmdDrawMeshTasksIndirectCountNV-countBuffer-02192";
        count_exceeds_device_limit = "VUID-vkCmdDrawMeshTasksIndirectCountNV-countBuffer-02717";
        task_group_count_exceeds_max_x = "VUID-VkDrawMeshTasksIndirectCommandEXT-TaskEXT-07322";
        task_group_count_exceeds_max_y = "VUID-VkDrawMeshTasksIndirectCommandEXT-TaskEXT-07323";
        task_group_count_exceeds_max_z = "VUID-VkDrawMeshTasksIndirectCommandEXT-TaskEXT-07324";
        task_group_count_exceeds_max_total = "VUID-VkDrawMeshTasksIndirectCommandEXT-TaskEXT-07325";
        mesh_group_count_exceeds_max_x = "VUID-VkDrawMeshTasksIndirectCommandEXT-TaskEXT-07326";
        mesh_group_count_exceeds_max_y = "VUID-VkDrawMeshTasksIndirectCommandEXT-TaskEXT-07327";
        mesh_group_count_exceeds_max_z = "VUID-VkDrawMeshTasksIndirectCommandEXT-TaskEXT-07328";
        mesh_group_count_exceeds_max_total = "VUID-VkDrawMeshTasksIndirectCommandEXT-TaskEXT-07329";
    }
};

struct GpuVuidsCmdDrawMeshTasksEXT : gpuav::GpuVuid {
    GpuVuidsCmdDrawMeshTasksEXT() : GpuVuid() {
        uniform_access_oob = "VUID-vkCmdDrawMeshTasksEXT-None-08612";
        storage_access_oob = "VUID-vkCmdDrawMeshTasksEXT-None-08613";
        invalid_descriptor = "VUID-vkCmdDrawMeshTasksEXT-None-08114";
    }
};

struct GpuVuidsCmdDrawMeshTasksIndirectEXT : gpuav::GpuVuid {
    GpuVuidsCmdDrawMeshTasksIndirectEXT() : GpuVuid() {
        uniform_access_oob = "VUID-vkCmdDrawMeshTasksIndirectEXT-None-08612";
        storage_access_oob = "VUID-vkCmdDrawMeshTasksIndirectEXT-None-08613";
        invalid_descriptor = "VUID-vkCmdDrawMeshTasksIndirectEXT-None-08114";
        task_group_count_exceeds_max_x = "VUID-VkDrawMeshTasksIndirectCommandEXT-TaskEXT-07322";
        task_group_count_exceeds_max_y = "VUID-VkDrawMeshTasksIndirectCommandEXT-TaskEXT-07323";
        task_group_count_exceeds_max_z = "VUID-VkDrawMeshTasksIndirectCommandEXT-TaskEXT-07324";
        task_group_count_exceeds_max_total = "VUID-VkDrawMeshTasksIndirectCommandEXT-TaskEXT-07325";
        mesh_group_count_exceeds_max_x = "VUID-VkDrawMeshTasksIndirectCommandEXT-TaskEXT-07326";
        mesh_group_count_exceeds_max_y = "VUID-VkDrawMeshTasksIndirectCommandEXT-TaskEXT-07327";
        mesh_group_count_exceeds_max_z = "VUID-VkDrawMeshTasksIndirectCommandEXT-TaskEXT-07328";
        mesh_group_count_exceeds_max_total = "VUID-VkDrawMeshTasksIndirectCommandEXT-TaskEXT-07329";
    }
};

struct GpuVuidsCmdDrawMeshTasksIndirectCountEXT : gpuav::GpuVuid {
    GpuVuidsCmdDrawMeshTasksIndirectCountEXT() : GpuVuid() {
        uniform_access_oob = "VUID-vkCmdDrawMeshTasksIndirectCountEXT-None-08612";
        storage_access_oob = "VUID-vkCmdDrawMeshTasksIndirectCountEXT-None-08613";
        invalid_descriptor = "VUID-vkCmdDrawMeshTasksIndirectCountEXT-None-08114";
        count_exceeds_bufsize_1 = "VUID-vkCmdDrawMeshTasksIndirectCountEXT-countBuffer-07098";
        count_exceeds_bufsize = "VUID-vkCmdDrawMeshTasksIndirectCountEXT-countBuffer-07099";
        count_exceeds_device_limit = "VUID-vkCmdDrawMeshTasksIndirectCountEXT-countBuffer-02717";
        task_group_count_exceeds_max_x = "VUID-VkDrawMeshTasksIndirectCommandEXT-TaskEXT-07322";
        task_group_count_exceeds_max_y = "VUID-VkDrawMeshTasksIndirectCommandEXT-TaskEXT-07323";
        task_group_count_exceeds_max_z = "VUID-VkDrawMeshTasksIndirectCommandEXT-TaskEXT-07324";
        task_group_count_exceeds_max_total = "VUID-VkDrawMeshTasksIndirectCommandEXT-TaskEXT-07325";
        mesh_group_count_exceeds_max_x = "VUID-VkDrawMeshTasksIndirectCommandEXT-TaskEXT-07326";
        mesh_group_count_exceeds_max_y = "VUID-VkDrawMeshTasksIndirectCommandEXT-TaskEXT-07327";
        mesh_group_count_exceeds_max_z = "VUID-VkDrawMeshTasksIndirectCommandEXT-TaskEXT-07328";
        mesh_group_count_exceeds_max_total = "VUID-VkDrawMeshTasksIndirectCommandEXT-TaskEXT-07329";
    }
};

struct GpuVuidsCmdDrawIndirectByteCountEXT : gpuav::GpuVuid {
    GpuVuidsCmdDrawIndirectByteCountEXT() : GpuVuid() {
        uniform_access_oob = "VUID-vkCmdDrawIndirectByteCountEXT-None-08612";
        storage_access_oob = "VUID-vkCmdDrawIndirectByteCountEXT-None-08613";
        invalid_descriptor = "VUID-vkCmdDrawIndirectByteCountEXT-None-08114";
    }
};

struct GpuVuidsCmdDispatchBase : gpuav::GpuVuid {
    GpuVuidsCmdDispatchBase() : GpuVuid() {
        uniform_access_oob = "VUID-vkCmdDispatchBase-None-08612";
        storage_access_oob = "VUID-vkCmdDispatchBase-None-08613";
        invalid_descriptor = "VUID-vkCmdDispatchBase-None-08114";
    }
};

using Func = vvl::Func;
// This LUT is created to allow a static listing of each VUID that is covered by drawdispatch commands
static const std::map<Func, gpuav::GpuVuid> gpu_vuid = {
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
    {Func::vkCmdDrawMeshTasksEXT, GpuVuidsCmdDrawMeshTasksEXT()},
    {Func::vkCmdDrawMeshTasksIndirectEXT, GpuVuidsCmdDrawMeshTasksIndirectEXT()},
    {Func::vkCmdDrawMeshTasksIndirectCountEXT, GpuVuidsCmdDrawMeshTasksIndirectCountEXT()},
    {Func::vkCmdDrawIndirectByteCountEXT, GpuVuidsCmdDrawIndirectByteCountEXT()},
    {Func::vkCmdDispatchBase, GpuVuidsCmdDispatchBase()},
    {Func::vkCmdDispatchBaseKHR, GpuVuidsCmdDispatchBase()},
    // Used if invalid function is used
    {Func::Empty, gpuav::GpuVuid()}
};

const gpuav::GpuVuid &gpuav::GetGpuVuid(Func command) {
    if (gpu_vuid.find(command) != gpu_vuid.cend()) {
        return gpu_vuid.at(command);
    }
    else {
        return gpu_vuid.at(Func::Empty);
    }
}
// clang-format on
