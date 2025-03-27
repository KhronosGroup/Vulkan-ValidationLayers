/* Copyright (c) 2020-2025 The Khronos Group Inc.
 * Copyright (c) 2020-2025 Valve Corporation
 * Copyright (c) 2020-2025 LunarG, Inc.
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

#include "gpuav/instrumentation/gpuav_instrumentation_debug.h"

#include <cassert>
#include <sstream>
#include <fstream>

namespace gpuav {

thread_local InstrumentedShaderDebugInfo tl_instrumentation_debug_info;

std::string InstrumentedShaderDebugInfo::ToStringFileSuffix() const {
    std::stringstream ss;

    ss << "bda_" << bda << "_dbgpf_" << dbgpf << "_dcgb_" << dcgb << "_dctb_" << dctb << "_dioob_" << dioob << "_postp_" << postp
       << "_rayq_" << rayq << "_vaoob_" << vaoob;

    switch (shader_stage) {
        case VK_SHADER_STAGE_VERTEX_BIT:
            ss << "_vert";
            break;
        case VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT:
            ss << "_tess_ctrl";
            break;
        case VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT:
            ss << "_tess_eval";
            break;
        case VK_SHADER_STAGE_GEOMETRY_BIT:
            ss << "_geom";
            break;
        case VK_SHADER_STAGE_FRAGMENT_BIT:
            ss << "_frag";
            break;
        case VK_SHADER_STAGE_COMPUTE_BIT:
            ss << "_comp";
            break;
        case VK_SHADER_STAGE_RAYGEN_BIT_KHR:
            ss << "_rgen";
            break;
        case VK_SHADER_STAGE_ANY_HIT_BIT_KHR:
            ss << "_ahit";
            break;
        case VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR:
            ss << "_chit";
            break;
        case VK_SHADER_STAGE_MISS_BIT_KHR:
            ss << "_miss";
            break;
        case VK_SHADER_STAGE_INTERSECTION_BIT_KHR:
            ss << "_inter";
            break;
        case VK_SHADER_STAGE_CALLABLE_BIT_KHR:
            ss << "_call";
            break;
        case VK_SHADER_STAGE_TASK_BIT_EXT:
            ss << "_task";
            break;
        case VK_SHADER_STAGE_MESH_BIT_EXT:
            ss << "_mesh";
            break;
        default:
            assert(false);
            break;
    }

    const std::string ret = ss.str();
    return ret;
}

void DumpSpirvToFile(const std::filesystem::path &file_path, const char *spirv_data, size_t spirv_size) {
    std::ofstream debug_file(file_path, std::ios::out | std::ios::binary);
    debug_file.write(spirv_data, spirv_size);
}
}  // namespace gpuav
