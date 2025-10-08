/* Copyright (c) 2025 The Khronos Group Inc.
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

#include <cassert>
#include "utils/vk_api_utils.h"

namespace vvl {
// Need to be values that can be used to access an array for each bind point
enum BindPoint {
    BindPointGraphics = 0,    // VK_PIPELINE_BIND_POINT_GRAPHICS
    BindPointCompute = 1,     // VK_PIPELINE_BIND_POINT_COMPUTE
    BindPointRayTracing = 2,  // VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR
    BindPointDataGraph = 3,   // VK_PIPELINE_BIND_POINT_DATA_GRAPH_ARM
    BindPointCount = 4,
};
}  // namespace vvl

static vvl::BindPoint inline ConvertToVvlBindPoint(VkPipelineBindPoint bind_point) {
    switch (bind_point) {
        case VK_PIPELINE_BIND_POINT_GRAPHICS:
            return vvl::BindPointGraphics;
        case VK_PIPELINE_BIND_POINT_COMPUTE:
            return vvl::BindPointCompute;
        case VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR:
            return vvl::BindPointRayTracing;
        case VK_PIPELINE_BIND_POINT_DATA_GRAPH_ARM:
            return vvl::BindPointDataGraph;
        default:
            break;
    }
    assert(false);
    return vvl::BindPointGraphics;
}

// Used for things like Device Generated Commands which supply multiple stages (but all need to be in a single bind point)
static VkPipelineBindPoint inline ConvertStageToBindPoint(VkShaderStageFlags stage) {
    // Assumes the call has checked stages have not been mixed
    if (stage & kShaderStageAllGraphics) {
        return VK_PIPELINE_BIND_POINT_GRAPHICS;
    } else if (stage & VK_SHADER_STAGE_COMPUTE_BIT) {
        return VK_PIPELINE_BIND_POINT_COMPUTE;
    } else if (stage & kShaderStageAllRayTracing) {
        return VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR;
    } else {
        assert(false);
        return VK_PIPELINE_BIND_POINT_GRAPHICS;
    }
}

// Used to get last bound for shader object which only has a single stage
static vvl::BindPoint inline ConvertStageToVvlBindPoint(VkShaderStageFlagBits stage) {
    switch (stage) {
        case VK_SHADER_STAGE_VERTEX_BIT:
        case VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT:
        case VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT:
        case VK_SHADER_STAGE_GEOMETRY_BIT:
        case VK_SHADER_STAGE_FRAGMENT_BIT:
        case VK_SHADER_STAGE_TASK_BIT_EXT:
        case VK_SHADER_STAGE_MESH_BIT_EXT:
            return vvl::BindPointGraphics;
        case VK_SHADER_STAGE_COMPUTE_BIT:
            return vvl::BindPointCompute;
        case VK_SHADER_STAGE_RAYGEN_BIT_KHR:
        case VK_SHADER_STAGE_ANY_HIT_BIT_KHR:
        case VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR:
        case VK_SHADER_STAGE_MISS_BIT_KHR:
        case VK_SHADER_STAGE_INTERSECTION_BIT_KHR:
        case VK_SHADER_STAGE_CALLABLE_BIT_KHR:
            return vvl::BindPointRayTracing;
        default:
            break;
    }
    assert(false);
    return vvl::BindPointGraphics;
}
