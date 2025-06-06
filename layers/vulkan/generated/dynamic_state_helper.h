// *** THIS FILE IS GENERATED - DO NOT EDIT ***
// See dynamic_state_generator.py for modifications

/***************************************************************************
 *
 * Copyright (c) 2023-2025 Valve Corporation
 * Copyright (c) 2023-2025 LunarG, Inc.
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
 ****************************************************************************/

// NOLINTBEGIN

#pragma once
#include <bitset>

namespace vvl {
class Pipeline;
}  // namespace vvl

// Reorders VkDynamicState so it can be a bitset
typedef enum CBDynamicState {
    CB_DYNAMIC_STATE_VIEWPORT = 1,
    CB_DYNAMIC_STATE_SCISSOR = 2,
    CB_DYNAMIC_STATE_LINE_WIDTH = 3,
    CB_DYNAMIC_STATE_DEPTH_BIAS = 4,
    CB_DYNAMIC_STATE_BLEND_CONSTANTS = 5,
    CB_DYNAMIC_STATE_DEPTH_BOUNDS = 6,
    CB_DYNAMIC_STATE_STENCIL_COMPARE_MASK = 7,
    CB_DYNAMIC_STATE_STENCIL_WRITE_MASK = 8,
    CB_DYNAMIC_STATE_STENCIL_REFERENCE = 9,
    CB_DYNAMIC_STATE_CULL_MODE = 10,
    CB_DYNAMIC_STATE_FRONT_FACE = 11,
    CB_DYNAMIC_STATE_PRIMITIVE_TOPOLOGY = 12,
    CB_DYNAMIC_STATE_VIEWPORT_WITH_COUNT = 13,
    CB_DYNAMIC_STATE_SCISSOR_WITH_COUNT = 14,
    CB_DYNAMIC_STATE_VERTEX_INPUT_BINDING_STRIDE = 15,
    CB_DYNAMIC_STATE_DEPTH_TEST_ENABLE = 16,
    CB_DYNAMIC_STATE_DEPTH_WRITE_ENABLE = 17,
    CB_DYNAMIC_STATE_DEPTH_COMPARE_OP = 18,
    CB_DYNAMIC_STATE_DEPTH_BOUNDS_TEST_ENABLE = 19,
    CB_DYNAMIC_STATE_STENCIL_TEST_ENABLE = 20,
    CB_DYNAMIC_STATE_STENCIL_OP = 21,
    CB_DYNAMIC_STATE_RASTERIZER_DISCARD_ENABLE = 22,
    CB_DYNAMIC_STATE_DEPTH_BIAS_ENABLE = 23,
    CB_DYNAMIC_STATE_PRIMITIVE_RESTART_ENABLE = 24,
    CB_DYNAMIC_STATE_LINE_STIPPLE = 25,
    CB_DYNAMIC_STATE_VIEWPORT_W_SCALING_NV = 26,
    CB_DYNAMIC_STATE_DISCARD_RECTANGLE_EXT = 27,
    CB_DYNAMIC_STATE_DISCARD_RECTANGLE_ENABLE_EXT = 28,
    CB_DYNAMIC_STATE_DISCARD_RECTANGLE_MODE_EXT = 29,
    CB_DYNAMIC_STATE_SAMPLE_LOCATIONS_EXT = 30,
    CB_DYNAMIC_STATE_RAY_TRACING_PIPELINE_STACK_SIZE_KHR = 31,
    CB_DYNAMIC_STATE_VIEWPORT_SHADING_RATE_PALETTE_NV = 32,
    CB_DYNAMIC_STATE_VIEWPORT_COARSE_SAMPLE_ORDER_NV = 33,
    CB_DYNAMIC_STATE_EXCLUSIVE_SCISSOR_ENABLE_NV = 34,
    CB_DYNAMIC_STATE_EXCLUSIVE_SCISSOR_NV = 35,
    CB_DYNAMIC_STATE_FRAGMENT_SHADING_RATE_KHR = 36,
    CB_DYNAMIC_STATE_VERTEX_INPUT_EXT = 37,
    CB_DYNAMIC_STATE_PATCH_CONTROL_POINTS_EXT = 38,
    CB_DYNAMIC_STATE_LOGIC_OP_EXT = 39,
    CB_DYNAMIC_STATE_COLOR_WRITE_ENABLE_EXT = 40,
    CB_DYNAMIC_STATE_DEPTH_CLAMP_ENABLE_EXT = 41,
    CB_DYNAMIC_STATE_POLYGON_MODE_EXT = 42,
    CB_DYNAMIC_STATE_RASTERIZATION_SAMPLES_EXT = 43,
    CB_DYNAMIC_STATE_SAMPLE_MASK_EXT = 44,
    CB_DYNAMIC_STATE_ALPHA_TO_COVERAGE_ENABLE_EXT = 45,
    CB_DYNAMIC_STATE_ALPHA_TO_ONE_ENABLE_EXT = 46,
    CB_DYNAMIC_STATE_LOGIC_OP_ENABLE_EXT = 47,
    CB_DYNAMIC_STATE_COLOR_BLEND_ENABLE_EXT = 48,
    CB_DYNAMIC_STATE_COLOR_BLEND_EQUATION_EXT = 49,
    CB_DYNAMIC_STATE_COLOR_WRITE_MASK_EXT = 50,
    CB_DYNAMIC_STATE_TESSELLATION_DOMAIN_ORIGIN_EXT = 51,
    CB_DYNAMIC_STATE_RASTERIZATION_STREAM_EXT = 52,
    CB_DYNAMIC_STATE_CONSERVATIVE_RASTERIZATION_MODE_EXT = 53,
    CB_DYNAMIC_STATE_EXTRA_PRIMITIVE_OVERESTIMATION_SIZE_EXT = 54,
    CB_DYNAMIC_STATE_DEPTH_CLIP_ENABLE_EXT = 55,
    CB_DYNAMIC_STATE_SAMPLE_LOCATIONS_ENABLE_EXT = 56,
    CB_DYNAMIC_STATE_COLOR_BLEND_ADVANCED_EXT = 57,
    CB_DYNAMIC_STATE_PROVOKING_VERTEX_MODE_EXT = 58,
    CB_DYNAMIC_STATE_LINE_RASTERIZATION_MODE_EXT = 59,
    CB_DYNAMIC_STATE_LINE_STIPPLE_ENABLE_EXT = 60,
    CB_DYNAMIC_STATE_DEPTH_CLIP_NEGATIVE_ONE_TO_ONE_EXT = 61,
    CB_DYNAMIC_STATE_VIEWPORT_W_SCALING_ENABLE_NV = 62,
    CB_DYNAMIC_STATE_VIEWPORT_SWIZZLE_NV = 63,
    CB_DYNAMIC_STATE_COVERAGE_TO_COLOR_ENABLE_NV = 64,
    CB_DYNAMIC_STATE_COVERAGE_TO_COLOR_LOCATION_NV = 65,
    CB_DYNAMIC_STATE_COVERAGE_MODULATION_MODE_NV = 66,
    CB_DYNAMIC_STATE_COVERAGE_MODULATION_TABLE_ENABLE_NV = 67,
    CB_DYNAMIC_STATE_COVERAGE_MODULATION_TABLE_NV = 68,
    CB_DYNAMIC_STATE_SHADING_RATE_IMAGE_ENABLE_NV = 69,
    CB_DYNAMIC_STATE_REPRESENTATIVE_FRAGMENT_TEST_ENABLE_NV = 70,
    CB_DYNAMIC_STATE_COVERAGE_REDUCTION_MODE_NV = 71,
    CB_DYNAMIC_STATE_ATTACHMENT_FEEDBACK_LOOP_ENABLE_EXT = 72,
    CB_DYNAMIC_STATE_DEPTH_CLAMP_RANGE_EXT = 73,
    CB_DYNAMIC_STATE_STATUS_NUM = 74
} CBDynamicState;

using CBDynamicFlags = std::bitset<CB_DYNAMIC_STATE_STATUS_NUM>;
VkDynamicState ConvertToDynamicState(CBDynamicState dynamic_state);
CBDynamicState ConvertToCBDynamicState(VkDynamicState dynamic_state);
const char* DynamicStateToString(CBDynamicState dynamic_state);
std::string DynamicStatesToString(CBDynamicFlags const& dynamic_states);
std::string DynamicStatesCommandsToString(CBDynamicFlags const& dynamic_states);

std::string DescribeDynamicStateCommand(CBDynamicState dynamic_state);
std::string DescribeDynamicStateDependency(CBDynamicState dynamic_state, const vvl::Pipeline* pipeline);

// NOLINTEND
