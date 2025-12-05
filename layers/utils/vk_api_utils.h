/* Copyright (c) 2019-2025 The Khronos Group Inc.
 * Copyright (c) 2019-2025 Valve Corporation
 * Copyright (c) 2019-2025 LunarG, Inc.
 * Modifications Copyright (C) 2022 RasterGrid Kft.
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

// This file should not need to include anything else, the goal of this file is utils that "could" be in the Vulkan-Headers
#include <vulkan/vulkan_core.h>

// It is very rare to have more than 3 stages (really only geo/tess) and better to save memory/time for the 99% use cases
static const uint32_t kCommonMaxGraphicsShaderStages = 3;

static const VkShaderStageFlags kShaderStageAllGraphics =
    VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT | VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT |
    VK_SHADER_STAGE_GEOMETRY_BIT | VK_SHADER_STAGE_FRAGMENT_BIT | VK_SHADER_STAGE_TASK_BIT_EXT | VK_SHADER_STAGE_MESH_BIT_EXT;

static const VkShaderStageFlags kShaderStageAllRayTracing =
    VK_SHADER_STAGE_ANY_HIT_BIT_KHR | VK_SHADER_STAGE_CALLABLE_BIT_KHR | VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR |
    VK_SHADER_STAGE_INTERSECTION_BIT_KHR | VK_SHADER_STAGE_MISS_BIT_KHR | VK_SHADER_STAGE_RAYGEN_BIT_KHR;

static inline uint32_t GetIndexAlignment(VkIndexType indexType) {
    switch (indexType) {
        case VK_INDEX_TYPE_UINT16:
            return 2;
        case VK_INDEX_TYPE_UINT32:
            return 4;
        case VK_INDEX_TYPE_UINT8:
            return 1;
        case VK_INDEX_TYPE_NONE_KHR:  // alias VK_INDEX_TYPE_NONE_NV
            return 0;
        default:
            // Not a real index type. Express no alignment requirement here; we expect upper layer
            // to have already picked up on the enum being nonsense.
            return 1;
    }
}

inline constexpr uint32_t GetIndexBitsSize(VkIndexType indexType) {
    switch (indexType) {
        case VK_INDEX_TYPE_UINT16:
            return 16;
        case VK_INDEX_TYPE_UINT32:
            return 32;
        case VK_INDEX_TYPE_NONE_KHR:
            return 0;
        case VK_INDEX_TYPE_UINT8_KHR:
            return 8;
        case VK_INDEX_TYPE_MAX_ENUM:
            return 0;
    }
    return 0;
}

static bool inline IsStageInPipelineBindPoint(VkShaderStageFlags stages, VkPipelineBindPoint bind_point) {
    switch (bind_point) {
        case VK_PIPELINE_BIND_POINT_GRAPHICS:
            return (stages & kShaderStageAllGraphics) != 0;
        case VK_PIPELINE_BIND_POINT_COMPUTE:
            return (stages & VK_SHADER_STAGE_COMPUTE_BIT) != 0;
        case VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR:
            return (stages & kShaderStageAllRayTracing) != 0;
        default:
            return false;
    }
}

// all "advanced blend operation" found in spec
static inline bool IsAdvanceBlendOperation(const VkBlendOp blend_op) {
    return (static_cast<int>(blend_op) >= VK_BLEND_OP_ZERO_EXT) && (static_cast<int>(blend_op) <= VK_BLEND_OP_BLUE_EXT);
}

// Helper for Dual-Source Blending
static inline bool IsSecondaryColorInputBlendFactor(VkBlendFactor blend_factor) {
    return (blend_factor == VK_BLEND_FACTOR_SRC1_COLOR || blend_factor == VK_BLEND_FACTOR_ONE_MINUS_SRC1_COLOR ||
            blend_factor == VK_BLEND_FACTOR_SRC1_ALPHA || blend_factor == VK_BLEND_FACTOR_ONE_MINUS_SRC1_ALPHA);
}

static inline bool IsPointTopology(VkPrimitiveTopology topology) { return topology == VK_PRIMITIVE_TOPOLOGY_POINT_LIST; };

static inline bool IsLineTopology(VkPrimitiveTopology topology) {
    return (topology == VK_PRIMITIVE_TOPOLOGY_LINE_LIST || topology == VK_PRIMITIVE_TOPOLOGY_LINE_STRIP ||
            topology == VK_PRIMITIVE_TOPOLOGY_LINE_LIST_WITH_ADJACENCY ||
            topology == VK_PRIMITIVE_TOPOLOGY_LINE_STRIP_WITH_ADJACENCY);
};

static inline bool IsTriangleTopology(VkPrimitiveTopology topology) {
    return (topology == VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST || topology == VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP ||
            topology == VK_PRIMITIVE_TOPOLOGY_TRIANGLE_FAN || topology == VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST_WITH_ADJACENCY ||
            topology == VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP_WITH_ADJACENCY);
};

// from vkspec.html#drawing-primitive-topology-class
static inline bool IsSameTopologyClass(VkPrimitiveTopology a, VkPrimitiveTopology b) {
    if (IsPointTopology(a)) {
        return IsPointTopology(b);
    } else if (IsLineTopology(a)) {
        return IsLineTopology(b);
    } else if (IsTriangleTopology(a)) {
        return IsTriangleTopology(b);
    } else {
        return a == VK_PRIMITIVE_TOPOLOGY_PATCH_LIST && b == VK_PRIMITIVE_TOPOLOGY_PATCH_LIST;
    }
};

static inline VkPrimitiveTopology TriangleToLineTopology(VkPrimitiveTopology topology) {
    if (topology == VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST) {
        return VK_PRIMITIVE_TOPOLOGY_LINE_LIST;
    } else if (topology == VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST_WITH_ADJACENCY) {
        return VK_PRIMITIVE_TOPOLOGY_LINE_LIST_WITH_ADJACENCY;
    } else if (topology == VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP) {
        return VK_PRIMITIVE_TOPOLOGY_LINE_STRIP;
    } else if (topology == VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP_WITH_ADJACENCY) {
        return VK_PRIMITIVE_TOPOLOGY_LINE_STRIP_WITH_ADJACENCY;
    } else if (topology == VK_PRIMITIVE_TOPOLOGY_TRIANGLE_FAN) {
        return VK_PRIMITIVE_TOPOLOGY_LINE_STRIP;
    }
    return VK_PRIMITIVE_TOPOLOGY_MAX_ENUM;
}

static inline VkExtent3D CastTo3D(const VkExtent2D &d2) {
    VkExtent3D d3 = {d2.width, d2.height, 1};
    return d3;
}

static inline VkOffset3D CastTo3D(const VkOffset2D &d2) {
    VkOffset3D d3 = {d2.x, d2.y, 0};
    return d3;
}

// Returns true if sub_rect is entirely contained within rect
static inline bool ContainsRect(VkRect2D rect, VkRect2D sub_rect) {
    if ((sub_rect.offset.x < rect.offset.x) || (sub_rect.offset.x + sub_rect.extent.width > rect.offset.x + rect.extent.width) ||
        (sub_rect.offset.y < rect.offset.y) || (sub_rect.offset.y + sub_rect.extent.height > rect.offset.y + rect.extent.height)) {
        return false;
    }
    return true;
}

static constexpr VkPipelineStageFlags2 kFramebufferStagePipelineStageFlags =
    (VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT |
     VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT);

static constexpr VkAccessFlags2 kShaderTileImageAllowedAccessFlags =
    VK_ACCESS_2_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_READ_BIT |
    VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

static constexpr bool HasNonFramebufferStagePipelineStageFlags(VkPipelineStageFlags2 inflags) {
    return (inflags & ~kFramebufferStagePipelineStageFlags) != 0;
}

static constexpr bool HasFramebufferStagePipelineStageFlags(VkPipelineStageFlags2 inflags) {
    return (inflags & kFramebufferStagePipelineStageFlags) != 0;
}

static constexpr bool HasNonShaderTileImageAccessFlags(VkAccessFlags2 in_flags) {
    return ((in_flags & ~kShaderTileImageAllowedAccessFlags) != 0);
}
