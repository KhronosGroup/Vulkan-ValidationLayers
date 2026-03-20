/* Copyright (c) 2015-2026 The Khronos Group Inc.
 * Copyright (c) 2015-2026 Valve Corporation
 * Copyright (c) 2015-2026 LunarG, Inc.
 * Copyright (C) 2015-2026 Google Inc.
 * Modifications Copyright (C) 2020-2022 Advanced Micro Devices, Inc. All rights reserved.
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

#include <vulkan/vk_enum_string_helper.h>
#include <vulkan/utility/vk_format_utils.h>
#include <vulkan/vulkan_core.h>
#include <cstdint>
#include "core_checks/cc_state_tracker.h"
#include "core_validation.h"
#include "drawdispatch/drawdispatch_vuids.h"
#include "error_message/error_strings.h"
#include "error_message/logging.h"
#include "generated/dynamic_state_helper.h"
#include "generated/error_location_helper.h"
#include "generated/vk_extension_helper.h"
#include "generated/dispatch_functions.h"
#include "state_tracker/image_state.h"
#include "state_tracker/render_pass_state.h"
#include "state_tracker/shader_object_state.h"
#include "state_tracker/shader_module.h"
#include "state_tracker/cmd_buffer_state.h"
#include "state_tracker/pipeline_state.h"
#include "state_tracker/shader_stage_state.h"
#include "utils/math_utils.h"
#include "utils/vk_api_utils.h"
#include "containers/container_utils.h"

bool CoreChecks::ValidateDynamicStateIsSet(const LastBound& last_bound_state, const CBDynamicFlags& state_status_cb,
                                           CBDynamicState dynamic_state, const Location& loc) const {
    if (!state_status_cb[dynamic_state]) {
        LogObjectList objlist(last_bound_state.cb_state.Handle());
        const vvl::Pipeline* pipeline = last_bound_state.pipeline_state;
        if (pipeline) {
            objlist.add(pipeline->Handle());
        }

        vvl::ActionVUID vuid = vvl::ActionVUID::UNKNOWN;
        switch (dynamic_state) {
            case CB_DYNAMIC_STATE_DEPTH_COMPARE_OP:
                vuid = vvl::ActionVUID::DYNAMIC_DEPTH_COMPARE_OP_07845;
                break;
            case CB_DYNAMIC_STATE_DEPTH_BIAS:
                vuid = vvl::ActionVUID::DYNAMIC_DEPTH_BIAS_07834;
                break;
            case CB_DYNAMIC_STATE_DEPTH_BOUNDS:
                vuid = vvl::ActionVUID::DYNAMIC_DEPTH_BOUNDS_07836;
                break;
            case CB_DYNAMIC_STATE_CULL_MODE:
                vuid = vvl::ActionVUID::DYNAMIC_CULL_MODE_07840;
                break;
            case CB_DYNAMIC_STATE_DEPTH_TEST_ENABLE:
                vuid = vvl::ActionVUID::DYNAMIC_DEPTH_TEST_ENABLE_07843;
                break;
            case CB_DYNAMIC_STATE_DEPTH_WRITE_ENABLE:
                vuid = vvl::ActionVUID::DYNAMIC_DEPTH_WRITE_ENABLE_07844;
                break;
            case CB_DYNAMIC_STATE_STENCIL_TEST_ENABLE:
                vuid = vvl::ActionVUID::DYNAMIC_STENCIL_TEST_ENABLE_07847;
                break;
            case CB_DYNAMIC_STATE_DEPTH_BIAS_ENABLE:
                vuid = vvl::ActionVUID::DYNAMIC_DEPTH_BIAS_ENABLE_04877;
                break;
            case CB_DYNAMIC_STATE_DEPTH_BOUNDS_TEST_ENABLE:
                vuid = vvl::ActionVUID::DYNAMIC_DEPTH_BOUND_TEST_ENABLE_07846;
                break;
            case CB_DYNAMIC_STATE_STENCIL_COMPARE_MASK:
                vuid = vvl::ActionVUID::DYNAMIC_STENCIL_COMPARE_MASK_07837;
                break;
            case CB_DYNAMIC_STATE_STENCIL_WRITE_MASK:
                vuid = vvl::ActionVUID::DYNAMIC_STENCIL_WRITE_MASK_07838;
                break;
            case CB_DYNAMIC_STATE_STENCIL_REFERENCE:
                vuid = vvl::ActionVUID::DYNAMIC_STENCIL_REFERENCE_07839;
                break;
            case CB_DYNAMIC_STATE_STENCIL_OP:
                vuid = vvl::ActionVUID::DYNAMIC_STENCIL_OP_07848;
                break;
            case CB_DYNAMIC_STATE_PRIMITIVE_TOPOLOGY:
                vuid = vvl::ActionVUID::DYNAMIC_PRIMITIVE_TOPOLOGY_07842;
                break;
            case CB_DYNAMIC_STATE_PRIMITIVE_RESTART_ENABLE:
                vuid = vvl::ActionVUID::DYNAMIC_PRIMITIVE_RESTART_ENABLE_04879;
                break;
            case CB_DYNAMIC_STATE_VERTEX_INPUT_EXT:
                vuid = vvl::ActionVUID::DYNAMIC_VERTEX_INPUT_04914;
                break;
            case CB_DYNAMIC_STATE_FRAGMENT_SHADING_RATE_KHR:
                vuid = vvl::ActionVUID::DYNAMIC_SET_FRAGMENT_SHADING_RATE_09238;
                break;
            case CB_DYNAMIC_STATE_LOGIC_OP_EXT:
                vuid = vvl::ActionVUID::DYNAMIC_LOGIC_OP_04878;
                break;
            case CB_DYNAMIC_STATE_POLYGON_MODE_EXT:
                vuid = vvl::ActionVUID::DYNAMIC_POLYGON_MODE_07621;
                break;
            case CB_DYNAMIC_STATE_RASTERIZATION_SAMPLES_EXT:
                vuid = vvl::ActionVUID::DYNAMIC_RASTERIZATION_SAMPLES_07622;
                break;
            case CB_DYNAMIC_STATE_SAMPLE_MASK_EXT:
                vuid = vvl::ActionVUID::DYNAMIC_SAMPLE_MASK_07623;
                break;
            case CB_DYNAMIC_STATE_ALPHA_TO_COVERAGE_ENABLE_EXT:
                vuid = vvl::ActionVUID::DYNAMIC_ALPHA_TO_COVERAGE_ENABLE_07624;
                break;
            case CB_DYNAMIC_STATE_ALPHA_TO_ONE_ENABLE_EXT:
                vuid = vvl::ActionVUID::DYNAMIC_ALPHA_TO_ONE_ENABLE_07625;
                break;
            case CB_DYNAMIC_STATE_LOGIC_OP_ENABLE_EXT:
                vuid = vvl::ActionVUID::DYNAMIC_LOGIC_OP_ENABLE_07626;
                break;
            case CB_DYNAMIC_STATE_RASTERIZER_DISCARD_ENABLE:
                vuid = vvl::ActionVUID::DYNAMIC_RASTERIZER_DISCARD_ENABLE_04876;
                break;
            case CB_DYNAMIC_STATE_SAMPLE_LOCATIONS_ENABLE_EXT:
                vuid = vvl::ActionVUID::DYNAMIC_SAMPLE_LOCATIONS_ENABLE_07634;
                break;
            case CB_DYNAMIC_STATE_SAMPLE_LOCATIONS_EXT:
                vuid = vvl::ActionVUID::DYNAMIC_SAMPLE_LOCATIONS_06666;
                break;
            case CB_DYNAMIC_STATE_ATTACHMENT_FEEDBACK_LOOP_ENABLE_EXT:
                vuid = vvl::ActionVUID::DYNAMIC_ATTACHMENT_FEEDBACK_LOOP_08877;
                break;
            case CB_DYNAMIC_STATE_COLOR_WRITE_ENABLE_EXT:
                vuid = vvl::ActionVUID::DYNAMIC_COLOR_WRITE_ENABLE_07749;
                break;
            case CB_DYNAMIC_STATE_COLOR_BLEND_ENABLE_EXT:
                vuid = vvl::ActionVUID::DYNAMIC_COLOR_BLEND_ENABLE_07627;
                break;
            case CB_DYNAMIC_STATE_COLOR_WRITE_MASK_EXT:
                vuid = vvl::ActionVUID::DYNAMIC_COLOR_WRITE_MASK_07629;
                break;
            case CB_DYNAMIC_STATE_DEPTH_CLIP_NEGATIVE_ONE_TO_ONE_EXT:
                vuid = vvl::ActionVUID::DYNAMIC_DEPTH_CLIP_NEGATIVE_ONE_TO_ONE_07639;
                break;
            case CB_DYNAMIC_STATE_DEPTH_CLAMP_RANGE_EXT:
                vuid = vvl::ActionVUID::DYNAMIC_DEPTH_CLAMP_CONTROL_09650;
                break;
            case CB_DYNAMIC_STATE_DEPTH_CLIP_ENABLE_EXT:
                vuid = vvl::ActionVUID::DYNAMIC_DEPTH_CLIP_ENABLE_07633;
                break;
            case CB_DYNAMIC_STATE_DEPTH_CLAMP_ENABLE_EXT:
                vuid = vvl::ActionVUID::DYNAMIC_DEPTH_CLAMP_ENABLE_07620;
                break;
            case CB_DYNAMIC_STATE_EXCLUSIVE_SCISSOR_ENABLE_NV:
                vuid = vvl::ActionVUID::DYNAMIC_EXCLUSIVE_SCISSOR_ENABLE_07878;
                break;
            case CB_DYNAMIC_STATE_EXCLUSIVE_SCISSOR_NV:
                vuid = vvl::ActionVUID::DYNAMIC_EXCLUSIVE_SCISSOR_07879;
                break;
            case CB_DYNAMIC_STATE_TESSELLATION_DOMAIN_ORIGIN_EXT:
                vuid = vvl::ActionVUID::DYNAMIC_TESSELLATION_DOMAIN_ORIGIN_07619;
                break;
            case CB_DYNAMIC_STATE_RASTERIZATION_STREAM_EXT:
                vuid = vvl::ActionVUID::DYNAMIC_RASTERIZATION_STREAM_07630;
                break;
            case CB_DYNAMIC_STATE_CONSERVATIVE_RASTERIZATION_MODE_EXT:
                vuid = vvl::ActionVUID::DYNAMIC_CONSERVATIVE_RASTERIZATION_MODE_07631;
                break;
            case CB_DYNAMIC_STATE_EXTRA_PRIMITIVE_OVERESTIMATION_SIZE_EXT:
                vuid = vvl::ActionVUID::DYNAMIC_EXTRA_PRIMITIVE_OVERESTIMATION_SIZE_07632;
                break;
            case CB_DYNAMIC_STATE_PROVOKING_VERTEX_MODE_EXT:
                vuid = vvl::ActionVUID::DYNAMIC_PROVOKING_VERTEX_MODE_07636;
                break;
            case CB_DYNAMIC_STATE_VIEWPORT_W_SCALING_ENABLE_NV:
                vuid = vvl::ActionVUID::DYNAMIC_VIEWPORT_W_SCALING_ENABLE_07640;
                break;
            case CB_DYNAMIC_STATE_VIEWPORT_SWIZZLE_NV:
                vuid = vvl::ActionVUID::DYNAMIC_VIEWPORT_SWIZZLE_07641;
                break;
            case CB_DYNAMIC_STATE_COVERAGE_TO_COLOR_ENABLE_NV:
                vuid = vvl::ActionVUID::DYNAMIC_COVERAGE_TO_COLOR_ENABLE_07642;
                break;
            case CB_DYNAMIC_STATE_COVERAGE_TO_COLOR_LOCATION_NV:
                vuid = vvl::ActionVUID::DYNAMIC_COVERAGE_TO_COLOR_LOCATION_07643;
                break;
            case CB_DYNAMIC_STATE_COVERAGE_MODULATION_MODE_NV:
                vuid = vvl::ActionVUID::DYNAMIC_COVERAGE_MODULATION_MODE_07644;
                break;
            case CB_DYNAMIC_STATE_COVERAGE_MODULATION_TABLE_ENABLE_NV:
                vuid = vvl::ActionVUID::DYNAMIC_COVERAGE_MODULATION_TABLE_ENABLE_07645;
                break;
            case CB_DYNAMIC_STATE_COVERAGE_MODULATION_TABLE_NV:
                vuid = vvl::ActionVUID::DYNAMIC_COVERAGE_MODULATION_TABLE_07646;
                break;
            case CB_DYNAMIC_STATE_SHADING_RATE_IMAGE_ENABLE_NV:
                vuid = vvl::ActionVUID::DYNAMIC_SHADING_RATE_IMAGE_ENABLE_07647;
                break;
            case CB_DYNAMIC_STATE_REPRESENTATIVE_FRAGMENT_TEST_ENABLE_NV:
                vuid = vvl::ActionVUID::DYNAMIC_REPRESENTATIVE_FRAGMENT_TEST_ENABLE_07648;
                break;
            case CB_DYNAMIC_STATE_COVERAGE_REDUCTION_MODE_NV:
                vuid = vvl::ActionVUID::DYNAMIC_COVERAGE_REDUCTION_MODE_07649;
                break;
            case CB_DYNAMIC_STATE_FRONT_FACE:
                vuid = vvl::ActionVUID::DYNAMIC_FRONT_FACE_07841;
                break;
            case CB_DYNAMIC_STATE_VIEWPORT_WITH_COUNT:
                vuid = vvl::ActionVUID::DYNAMIC_VIEWPORT_COUNT_03417;
                break;
            case CB_DYNAMIC_STATE_SCISSOR_WITH_COUNT:
                vuid = vvl::ActionVUID::DYNAMIC_SCISSOR_COUNT_03418;
                break;
            case CB_DYNAMIC_STATE_VIEWPORT_COARSE_SAMPLE_ORDER_NV:
                vuid = vvl::ActionVUID::DYNAMIC_SET_VIEWPORT_COARSE_SAMPLE_ORDER_09233;
                break;
            case CB_DYNAMIC_STATE_VIEWPORT_SHADING_RATE_PALETTE_NV:
                vuid = vvl::ActionVUID::DYNAMIC_SET_VIEWPORT_SHADING_RATE_PALETTE_09234;
                break;
            case CB_DYNAMIC_STATE_VIEWPORT_W_SCALING_NV:
                vuid = vvl::ActionVUID::DYNAMIC_SET_CLIP_SPACE_W_SCALING_04138;
                break;
            case CB_DYNAMIC_STATE_PATCH_CONTROL_POINTS_EXT:
                vuid = vvl::ActionVUID::DYNAMIC_PATCH_CONTROL_POINTS_04875;
                break;
            case CB_DYNAMIC_STATE_DISCARD_RECTANGLE_ENABLE_EXT:
                vuid = vvl::ActionVUID::DYNAMIC_DISCARD_RECTANGLE_ENABLE_07880;
                break;
            case CB_DYNAMIC_STATE_DISCARD_RECTANGLE_MODE_EXT:
                vuid = vvl::ActionVUID::DYNAMIC_DISCARD_RECTANGLE_MODE_07881;
                break;
            case CB_DYNAMIC_STATE_LINE_STIPPLE:
                vuid = vvl::ActionVUID::DYNAMIC_LINE_STIPPLE_EXT_07849;
                break;
            case CB_DYNAMIC_STATE_LINE_WIDTH:
                vuid = vvl::ActionVUID::DYNAMIC_SET_LINE_WIDTH_08617;
                break;
            case CB_DYNAMIC_STATE_VERTEX_INPUT_BINDING_STRIDE:
                vuid = vvl::ActionVUID::DYNAMIC_VERTEX_INPUT_BINDING_STRIDE_04913;
                break;
            case CB_DYNAMIC_STATE_LINE_RASTERIZATION_MODE_EXT:
                vuid = vvl::ActionVUID::DYNAMIC_SET_LINE_RASTERIZATION_MODE_08666;
                break;
            case CB_DYNAMIC_STATE_LINE_STIPPLE_ENABLE_EXT:
                vuid = vvl::ActionVUID::DYNAMIC_SET_LINE_STIPPLE_ENABLE_08669;
                break;
            default:
                assert(false);
                break;
        }

        return LogError(CreateActionVuid(loc.function, vuid), objlist, loc,
                        "%s state is dynamic, but the command buffer never called %s.\n%s%s", DynamicStateToString(dynamic_state),
                        DescribeDynamicStateCommand(dynamic_state).c_str(),
                        DescribeDynamicStateDependency(dynamic_state, pipeline).c_str(),
                        last_bound_state.cb_state.DescribeInvalidatedState(dynamic_state).c_str());
    }
    return false;
}

static CBDynamicFlags GetDrawTimeDynamicStatus(const vvl::CommandBuffer& cb_state, const vvl::Pipeline* pipeline) {
    if (pipeline) {
        // build the mask of what has been set in the Pipeline, but yet to be set in the Command
        return cb_state.dynamic_state_status.cb | ~pipeline->dynamic_state | pipeline->ignored_dynamic_state;
    } else {
        // for Shader Object, everything is dynamic don't need a mask
        return cb_state.dynamic_state_status.cb;
    }
}

bool CoreChecks::ValidateGraphicsDynamicStateSetStatus(const LastBound& last_bound_state, const Location& loc) const {
    bool skip = false;
    const vvl::CommandBuffer& cb_state = last_bound_state.cb_state;
    const VkShaderStageFlags bound_stages = last_bound_state.GetAllActiveBoundStages();
    const bool has_pipeline = last_bound_state.pipeline_state != nullptr;
    const bool has_rasterization_pipeline = has_pipeline && !(last_bound_state.pipeline_state->active_shaders &
                                                              (VK_SHADER_STAGE_TASK_BIT_EXT | VK_SHADER_STAGE_MESH_BIT_EXT));
    // TODO - Spec clarification and testing still to prove pipeline can just check active stages only
    const bool vertex_shader_bound =
        has_rasterization_pipeline || last_bound_state.IsValidShaderObjectBound(ShaderObjectStage::VERTEX);
    const bool fragment_shader_bound = has_pipeline || last_bound_state.IsValidShaderObjectBound(ShaderObjectStage::FRAGMENT);
    const bool geom_shader_bound = (bound_stages & VK_SHADER_STAGE_GEOMETRY_BIT) != 0;
    const bool tesc_shader_bound = (bound_stages & VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT) != 0;
    const bool tese_shader_bound = (bound_stages & VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT) != 0;

    // every bit set here represents the command buffer has called vkCmdSet*() for that state
    const CBDynamicFlags state_status_cb = GetDrawTimeDynamicStatus(cb_state, last_bound_state.pipeline_state);

    skip |= ValidateDynamicStateIsSet(last_bound_state, state_status_cb, CB_DYNAMIC_STATE_RASTERIZER_DISCARD_ENABLE, loc);
    if (!last_bound_state.IsRasterizationDisabled()) {
        skip |= ValidateDynamicStateIsSet(last_bound_state, state_status_cb, CB_DYNAMIC_STATE_CULL_MODE, loc);
        skip |= ValidateDynamicStateIsSet(last_bound_state, state_status_cb, CB_DYNAMIC_STATE_DEPTH_TEST_ENABLE, loc);
        skip |= ValidateDynamicStateIsSet(last_bound_state, state_status_cb, CB_DYNAMIC_STATE_STENCIL_TEST_ENABLE, loc);
        skip |= ValidateDynamicStateIsSet(last_bound_state, state_status_cb, CB_DYNAMIC_STATE_DEPTH_BIAS_ENABLE, loc);
        skip |= ValidateDynamicStateIsSet(last_bound_state, state_status_cb, CB_DYNAMIC_STATE_POLYGON_MODE_EXT, loc);
        skip |= ValidateDynamicStateIsSet(last_bound_state, state_status_cb, CB_DYNAMIC_STATE_RASTERIZATION_SAMPLES_EXT, loc);
        skip |= ValidateDynamicStateIsSet(last_bound_state, state_status_cb, CB_DYNAMIC_STATE_SAMPLE_MASK_EXT, loc);

        if (last_bound_state.IsDepthTestEnable()) {
            skip |= ValidateDynamicStateIsSet(last_bound_state, state_status_cb, CB_DYNAMIC_STATE_DEPTH_COMPARE_OP, loc);
            skip |= ValidateDynamicStateIsSet(last_bound_state, state_status_cb, CB_DYNAMIC_STATE_DEPTH_WRITE_ENABLE, loc);
        }
        if (last_bound_state.IsDepthBiasEnable()) {
            skip |= ValidateDynamicStateIsSet(last_bound_state, state_status_cb, CB_DYNAMIC_STATE_DEPTH_BIAS, loc);
        }
        if (last_bound_state.IsDepthBoundTestEnable()) {
            skip |= ValidateDynamicStateIsSet(last_bound_state, state_status_cb, CB_DYNAMIC_STATE_DEPTH_BOUNDS, loc);
        }
        if (enabled_features.depthBounds) {
            skip |= ValidateDynamicStateIsSet(last_bound_state, state_status_cb, CB_DYNAMIC_STATE_DEPTH_BOUNDS_TEST_ENABLE, loc);
        }

        if (last_bound_state.IsStencilTestEnable()) {
            skip |= ValidateDynamicStateIsSet(last_bound_state, state_status_cb, CB_DYNAMIC_STATE_STENCIL_COMPARE_MASK, loc);
            skip |= ValidateDynamicStateIsSet(last_bound_state, state_status_cb, CB_DYNAMIC_STATE_STENCIL_WRITE_MASK, loc);
            skip |= ValidateDynamicStateIsSet(last_bound_state, state_status_cb, CB_DYNAMIC_STATE_STENCIL_REFERENCE, loc);
            skip |= ValidateDynamicStateIsSet(last_bound_state, state_status_cb, CB_DYNAMIC_STATE_STENCIL_OP, loc);
        }

        // TODO - Doesn't match up with spec
        if (last_bound_state.IsStencilTestEnable() || last_bound_state.GetCullMode() != VK_CULL_MODE_NONE) {
            skip |= ValidateDynamicStateIsSet(last_bound_state, state_status_cb, CB_DYNAMIC_STATE_FRONT_FACE, loc);
        }

        if (IsExtEnabled(extensions.vk_ext_sample_locations)) {
            skip |= ValidateDynamicStateIsSet(last_bound_state, state_status_cb, CB_DYNAMIC_STATE_SAMPLE_LOCATIONS_ENABLE_EXT, loc);
            if (last_bound_state.IsSampleLocationsEnable()) {
                skip |= ValidateDynamicStateIsSet(last_bound_state, state_status_cb, CB_DYNAMIC_STATE_SAMPLE_LOCATIONS_EXT, loc);
            }
        }

        if (enabled_features.depthClipEnable) {
            skip |= ValidateDynamicStateIsSet(last_bound_state, state_status_cb, CB_DYNAMIC_STATE_DEPTH_CLIP_ENABLE_EXT, loc);
        }
        if (enabled_features.depthClipControl) {
            skip |= ValidateDynamicStateIsSet(last_bound_state, state_status_cb,
                                              CB_DYNAMIC_STATE_DEPTH_CLIP_NEGATIVE_ONE_TO_ONE_EXT, loc);
        }
        if (enabled_features.depthClampControl) {
            if (last_bound_state.IsDepthClampEnable()) {
                skip |= ValidateDynamicStateIsSet(last_bound_state, state_status_cb, CB_DYNAMIC_STATE_DEPTH_CLAMP_RANGE_EXT, loc);
            }
        }
        if (enabled_features.depthClamp) {
            skip |= ValidateDynamicStateIsSet(last_bound_state, state_status_cb, CB_DYNAMIC_STATE_DEPTH_CLAMP_ENABLE_EXT, loc);
        }

        if (enabled_features.alphaToOne) {
            skip |= ValidateDynamicStateIsSet(last_bound_state, state_status_cb, CB_DYNAMIC_STATE_ALPHA_TO_ONE_ENABLE_EXT, loc);
        }
        skip |= ValidateDynamicStateIsSet(last_bound_state, state_status_cb, CB_DYNAMIC_STATE_ALPHA_TO_COVERAGE_ENABLE_EXT, loc);

        if (IsExtEnabled(extensions.vk_ext_conservative_rasterization)) {
            skip |= ValidateDynamicStateIsSet(last_bound_state, state_status_cb,
                                              CB_DYNAMIC_STATE_CONSERVATIVE_RASTERIZATION_MODE_EXT, loc);
            if (last_bound_state.GetConservativeRasterizationMode() == VK_CONSERVATIVE_RASTERIZATION_MODE_OVERESTIMATE_EXT) {
                skip |= ValidateDynamicStateIsSet(last_bound_state, state_status_cb,
                                                  CB_DYNAMIC_STATE_EXTRA_PRIMITIVE_OVERESTIMATION_SIZE_EXT, loc);
            }
        }

        if (IsExtEnabled(extensions.vk_nv_fragment_coverage_to_color)) {
            skip |= ValidateDynamicStateIsSet(last_bound_state, state_status_cb, CB_DYNAMIC_STATE_COVERAGE_TO_COLOR_ENABLE_NV, loc);
            if (last_bound_state.IsCoverageToColorEnabled()) {
                skip |= ValidateDynamicStateIsSet(last_bound_state, state_status_cb, CB_DYNAMIC_STATE_COVERAGE_TO_COLOR_LOCATION_NV,
                                                  loc);
            }
        }

        if (enabled_features.shadingRateImage) {
            skip |=
                ValidateDynamicStateIsSet(last_bound_state, state_status_cb, CB_DYNAMIC_STATE_SHADING_RATE_IMAGE_ENABLE_NV, loc);
            skip |=
                ValidateDynamicStateIsSet(last_bound_state, state_status_cb, CB_DYNAMIC_STATE_VIEWPORT_COARSE_SAMPLE_ORDER_NV, loc);

            if (last_bound_state.IsShadingRateImageEnable()) {
                skip |= ValidateDynamicStateIsSet(last_bound_state, state_status_cb,
                                                  CB_DYNAMIC_STATE_VIEWPORT_SHADING_RATE_PALETTE_NV, loc);
            }
        }

        if (enabled_features.representativeFragmentTest) {
            skip |= ValidateDynamicStateIsSet(last_bound_state, state_status_cb,
                                              CB_DYNAMIC_STATE_REPRESENTATIVE_FRAGMENT_TEST_ENABLE_NV, loc);
        }
        if (enabled_features.coverageReductionMode) {
            skip |= ValidateDynamicStateIsSet(last_bound_state, state_status_cb, CB_DYNAMIC_STATE_COVERAGE_REDUCTION_MODE_NV, loc);
        }

        if (IsExtEnabled(extensions.vk_nv_framebuffer_mixed_samples)) {
            skip |= ValidateDynamicStateIsSet(last_bound_state, state_status_cb, CB_DYNAMIC_STATE_COVERAGE_MODULATION_MODE_NV, loc);
            if (last_bound_state.GetCoverageModulationMode() != VK_COVERAGE_MODULATION_MODE_NONE_NV) {
                skip |= ValidateDynamicStateIsSet(last_bound_state, state_status_cb,
                                                  CB_DYNAMIC_STATE_COVERAGE_MODULATION_TABLE_ENABLE_NV, loc);
            }
            if (last_bound_state.IsCoverageModulationTableEnable()) {
                skip |= ValidateDynamicStateIsSet(last_bound_state, state_status_cb, CB_DYNAMIC_STATE_COVERAGE_MODULATION_TABLE_NV,
                                                  loc);
            }
        }

        if (IsExtEnabled(extensions.vk_ext_discard_rectangles)) {
            skip |=
                ValidateDynamicStateIsSet(last_bound_state, state_status_cb, CB_DYNAMIC_STATE_DISCARD_RECTANGLE_ENABLE_EXT, loc);
            if (last_bound_state.IsDiscardRectangleEnable()) {
                skip |=
                    ValidateDynamicStateIsSet(last_bound_state, state_status_cb, CB_DYNAMIC_STATE_DISCARD_RECTANGLE_MODE_EXT, loc);
            }
        }

        if (enabled_features.stippledRectangularLines || enabled_features.stippledBresenhamLines ||
            enabled_features.stippledSmoothLines) {
            if (last_bound_state.IsStippledLineEnable()) {
                skip |= ValidateDynamicStateIsSet(last_bound_state, state_status_cb, CB_DYNAMIC_STATE_LINE_STIPPLE, loc);
            }
        }

        if (enabled_features.pipelineFragmentShadingRate) {
            skip |= ValidateDynamicStateIsSet(last_bound_state, state_status_cb, CB_DYNAMIC_STATE_FRAGMENT_SHADING_RATE_KHR, loc);
        }

        const VkPrimitiveTopology topology = last_bound_state.GetRasterizationInputTopology();
        if (IsLineTopology(topology)) {
            skip |= ValidateDynamicStateIsSet(last_bound_state, state_status_cb, CB_DYNAMIC_STATE_LINE_WIDTH, loc);

            const bool stippled_lines = enabled_features.stippledRectangularLines || enabled_features.stippledBresenhamLines ||
                                        enabled_features.stippledSmoothLines;
            if (stippled_lines) {
                skip |=
                    ValidateDynamicStateIsSet(last_bound_state, state_status_cb, CB_DYNAMIC_STATE_LINE_RASTERIZATION_MODE_EXT, loc);
                skip |= ValidateDynamicStateIsSet(last_bound_state, state_status_cb, CB_DYNAMIC_STATE_LINE_STIPPLE_ENABLE_EXT, loc);
            }
        }

        if (vertex_shader_bound) {
            if (IsExtEnabled(extensions.vk_ext_provoking_vertex)) {
                skip |=
                    ValidateDynamicStateIsSet(last_bound_state, state_status_cb, CB_DYNAMIC_STATE_PROVOKING_VERTEX_MODE_EXT, loc);
            }
        }

        if (fragment_shader_bound) {
            if (enabled_features.logicOp) {
                skip |= ValidateDynamicStateIsSet(last_bound_state, state_status_cb, CB_DYNAMIC_STATE_LOGIC_OP_ENABLE_EXT, loc);
            }
            if (last_bound_state.IsLogicOpEnabled()) {
                skip |= ValidateDynamicStateIsSet(last_bound_state, state_status_cb, CB_DYNAMIC_STATE_LOGIC_OP_EXT, loc);
            }
            if (enabled_features.attachmentFeedbackLoopDynamicState) {
                skip |= ValidateDynamicStateIsSet(last_bound_state, state_status_cb,
                                                  CB_DYNAMIC_STATE_ATTACHMENT_FEEDBACK_LOOP_ENABLE_EXT, loc);
            }
            if (enabled_features.colorWriteEnable) {
                skip |= ValidateDynamicStateIsSet(last_bound_state, state_status_cb, CB_DYNAMIC_STATE_COLOR_WRITE_ENABLE_EXT, loc);
            }

            if (!cb_state.active_color_attachments_index.empty()) {
                skip |= ValidateDynamicStateIsSet(last_bound_state, state_status_cb, CB_DYNAMIC_STATE_COLOR_BLEND_ENABLE_EXT, loc);
                skip |= ValidateDynamicStateIsSet(last_bound_state, state_status_cb, CB_DYNAMIC_STATE_COLOR_WRITE_MASK_EXT, loc);
            }
        }
    }  // !IsRasterizationDisabled()

    auto& cb_sub_state = core::SubState(cb_state);
    if (cb_sub_state.viewport.inherited_depths.empty()) {
        skip |= ValidateDynamicStateIsSet(last_bound_state, state_status_cb, CB_DYNAMIC_STATE_VIEWPORT_WITH_COUNT, loc);
        skip |= ValidateDynamicStateIsSet(last_bound_state, state_status_cb, CB_DYNAMIC_STATE_SCISSOR_WITH_COUNT, loc);
    }

    if (vertex_shader_bound) {
        skip |= ValidateDynamicStateIsSet(last_bound_state, state_status_cb, CB_DYNAMIC_STATE_PRIMITIVE_TOPOLOGY, loc);
        skip |= ValidateDynamicStateIsSet(last_bound_state, state_status_cb, CB_DYNAMIC_STATE_PRIMITIVE_RESTART_ENABLE, loc);
        skip |= ValidateDynamicStateIsSet(last_bound_state, state_status_cb, CB_DYNAMIC_STATE_VERTEX_INPUT_EXT, loc);
    }

    if (tese_shader_bound) {
        skip |= ValidateDynamicStateIsSet(last_bound_state, state_status_cb, CB_DYNAMIC_STATE_TESSELLATION_DOMAIN_ORIGIN_EXT, loc);
    }

    if (tesc_shader_bound) {
        if (last_bound_state.GetVertexInputAssemblerTopology() == VK_PRIMITIVE_TOPOLOGY_PATCH_LIST) {
            skip |= ValidateDynamicStateIsSet(last_bound_state, state_status_cb, CB_DYNAMIC_STATE_PATCH_CONTROL_POINTS_EXT, loc);
        }
    }

    if (geom_shader_bound) {
        if (enabled_features.geometryStreams) {
            skip |= ValidateDynamicStateIsSet(last_bound_state, state_status_cb, CB_DYNAMIC_STATE_RASTERIZATION_STREAM_EXT, loc);
        }
    }

    if (enabled_features.exclusiveScissor) {
        skip |= ValidateDynamicStateIsSet(last_bound_state, state_status_cb, CB_DYNAMIC_STATE_EXCLUSIVE_SCISSOR_ENABLE_NV, loc);
        if (last_bound_state.IsExclusiveScissorEnabled()) {
            skip |= ValidateDynamicStateIsSet(last_bound_state, state_status_cb, CB_DYNAMIC_STATE_EXCLUSIVE_SCISSOR_NV, loc);
        }
    }

    if (IsExtEnabled(extensions.vk_nv_clip_space_w_scaling)) {
        skip |= ValidateDynamicStateIsSet(last_bound_state, state_status_cb, CB_DYNAMIC_STATE_VIEWPORT_W_SCALING_ENABLE_NV, loc);
        if (last_bound_state.IsViewportWScalingEnable() && last_bound_state.IsDynamic(CB_DYNAMIC_STATE_VIEWPORT_WITH_COUNT) &&
            last_bound_state.IsDynamic(CB_DYNAMIC_STATE_VIEWPORT_W_SCALING_NV)) {
            skip |= ValidateDynamicStateIsSet(last_bound_state, state_status_cb, CB_DYNAMIC_STATE_VIEWPORT_W_SCALING_NV, loc);
        }
    }

    if (IsExtEnabled(extensions.vk_nv_viewport_swizzle)) {
        skip |= ValidateDynamicStateIsSet(last_bound_state, state_status_cb, CB_DYNAMIC_STATE_VIEWPORT_SWIZZLE_NV, loc);
    }

    // VK_EXT_vertex_input_dynamic_state was never supported in shader object, so this check only happens for pipelines
    if (has_pipeline) {
        if (!last_bound_state.IsDynamic(CB_DYNAMIC_STATE_VERTEX_INPUT_EXT) &&
            last_bound_state.IsDynamic(CB_DYNAMIC_STATE_VERTEX_INPUT_BINDING_STRIDE)) {
            // The vertex buffer for DGC needs to be validated in GPU-AV
            if (loc.function != vvl::Func::vkCmdExecuteGeneratedCommandsEXT) {
                skip |=
                    ValidateDynamicStateIsSet(last_bound_state, state_status_cb, CB_DYNAMIC_STATE_VERTEX_INPUT_BINDING_STRIDE, loc);
            }
        }
    }

    if (cb_state.stride_set_with_bind_vertex_buffer_3 &&
        !last_bound_state.IsDynamic(CB_DYNAMIC_STATE_VERTEX_INPUT_BINDING_STRIDE) &&
        !last_bound_state.IsDynamic(CB_DYNAMIC_STATE_VERTEX_INPUT_EXT)) {
        LogObjectList objlist(last_bound_state.cb_state.Handle());
        skip |= LogError(CreateActionVuid(loc.function, vvl::ActionVUID::BIND_VERTEX_BUFFERS_3_STRIDE_13118), objlist, loc,
                         "vkCmdBindVertexBuffers3KHR() was called with at least one of VkBindVertexBuffer3InfoKHR::setStride being "
                         "VK_TRUE, the pipeline %s was created without VK_DYNAMIC_STATE_VERTEX_INPUT_BINDING_STRIDE or "
                         "VK_DYNAMIC_STATE_VERTEX_INPUT_EXT dynamic states",
                         FormatHandle(last_bound_state.pipeline_state->Handle()).c_str());
    }

    return skip;
}

bool CoreChecks::ValidateDrawDynamicStatePipelineRenderPass(const LastBound& last_bound_state, const vvl::Pipeline& pipeline,
                                                            const vvl::RenderPass& rp_state, const Location& loc) const {
    bool skip = false;

    const vvl::CommandBuffer& cb_state = last_bound_state.cb_state;
    if (pipeline.IsDynamic(CB_DYNAMIC_STATE_RASTERIZATION_SAMPLES_EXT)) {
        if (!enabled_features.variableMultisampleRate && rp_state.UsesNoAttachment(cb_state.GetActiveSubpass())) {
            if (std::optional<VkSampleCountFlagBits> subpass_rasterization_samples =
                    cb_state.GetActiveSubpassRasterizationSampleCount();
                subpass_rasterization_samples &&
                *subpass_rasterization_samples != cb_state.dynamic_state_value.rasterization_samples) {
                const LogObjectList objlist(cb_state.Handle(), pipeline.Handle(), rp_state.Handle());
                skip |= LogError(
                    CreateActionVuid(loc.function, vvl::ActionVUID::SAMPLE_LOCATIONS_07471), objlist, loc,
                    "VkPhysicalDeviceFeatures::variableMultisampleRate is VK_FALSE and the rasterizationSamples set with "
                    "vkCmdSetRasterizationSamplesEXT() were %s but a previous draw used rasterization samples %" PRIu32 ".%s",
                    string_VkSampleCountFlagBits(cb_state.dynamic_state_value.rasterization_samples),
                    *subpass_rasterization_samples,
                    cb_state.DescribeInvalidatedState(CB_DYNAMIC_STATE_RASTERIZATION_SAMPLES_EXT).c_str());
            } else if ((cb_state.dynamic_state_value.rasterization_samples &
                        phys_dev_props.limits.framebufferNoAttachmentsSampleCounts) == 0) {
                const LogObjectList objlist(cb_state.Handle(), pipeline.Handle(), rp_state.Handle());
                skip |= LogError(CreateActionVuid(loc.function, vvl::ActionVUID::SAMPLE_LOCATIONS_07471), objlist, loc,
                                 "rasterizationSamples set with vkCmdSetRasterizationSamplesEXT() are %s but this bit is not in "
                                 "framebufferNoAttachmentsSampleCounts (%s).%s",
                                 string_VkSampleCountFlagBits(cb_state.dynamic_state_value.rasterization_samples),
                                 string_VkSampleCountFlags(phys_dev_props.limits.framebufferNoAttachmentsSampleCounts).c_str(),
                                 cb_state.DescribeInvalidatedState(CB_DYNAMIC_STATE_RASTERIZATION_SAMPLES_EXT).c_str());
            }
        }
    }

    if (rp_state.UsesDynamicRendering()) {
        skip |= ValidateDrawRenderingAttachmentLocation(cb_state, pipeline, loc);
        skip |= ValidateDrawRenderingInputAttachmentIndex(cb_state, pipeline, loc);
    }

    return skip;
}

bool CoreChecks::ValidateDrawDynamicStatePipelineValue(const LastBound& last_bound_state, const vvl::Pipeline& pipeline,
                                                       const Location& loc) const {
    bool skip = false;
    const vvl::CommandBuffer& cb_state = last_bound_state.cb_state;
    const LogObjectList objlist(cb_state.Handle(), pipeline.Handle());

    if (pipeline.IsDynamic(CB_DYNAMIC_STATE_SAMPLE_LOCATIONS_EXT) && last_bound_state.IsSampleLocationsEnable()) {
        if (!pipeline.IsDynamic(CB_DYNAMIC_STATE_RASTERIZATION_SAMPLES_EXT)) {
            if (cb_state.dynamic_state_value.sample_locations_info.sampleLocationsPerPixel !=
                pipeline.MultisampleState()->rasterizationSamples) {
                skip |= LogError(
                    CreateActionVuid(loc.function, vvl::ActionVUID::SAMPLE_LOCATIONS_07482), objlist, loc,
                    "sampleLocationsPerPixel set with vkCmdSetSampleLocationsEXT() was %s, but "
                    "VkPipelineMultisampleStateCreateInfo::rasterizationSamples from the pipeline was %s.%s",
                    string_VkSampleCountFlagBits(cb_state.dynamic_state_value.sample_locations_info.sampleLocationsPerPixel),
                    string_VkSampleCountFlagBits(pipeline.MultisampleState()->rasterizationSamples),
                    cb_state.DescribeInvalidatedState(CB_DYNAMIC_STATE_SAMPLE_LOCATIONS_EXT).c_str());
            }
        } else if (cb_state.dynamic_state_value.sample_locations_info.sampleLocationsPerPixel !=
                   cb_state.dynamic_state_value.rasterization_samples) {
            skip |=
                LogError(CreateActionVuid(loc.function, vvl::ActionVUID::SAMPLE_LOCATIONS_07483), objlist, loc,
                         "sampleLocationsPerPixel set with vkCmdSetSampleLocationsEXT() was %s, but "
                         "rasterizationSamples set with vkCmdSetRasterizationSamplesEXT() was %s.%s",
                         string_VkSampleCountFlagBits(cb_state.dynamic_state_value.sample_locations_info.sampleLocationsPerPixel),
                         string_VkSampleCountFlagBits(cb_state.dynamic_state_value.rasterization_samples),
                         cb_state.DescribeInvalidatedState(CB_DYNAMIC_STATE_RASTERIZATION_SAMPLES_EXT).c_str());
        }
    }

    if (!pipeline.IsDynamic(CB_DYNAMIC_STATE_SAMPLE_LOCATIONS_EXT) &&
        pipeline.IsDynamic(CB_DYNAMIC_STATE_RASTERIZATION_SAMPLES_EXT) && pipeline.MultisampleState() &&
        last_bound_state.IsSampleLocationsEnable()) {
        if (const auto sample_locations =
                vku::FindStructInPNextChain<VkPipelineSampleLocationsStateCreateInfoEXT>(pipeline.MultisampleState()->pNext)) {
            VkMultisamplePropertiesEXT multisample_prop = vku::InitStructHelper();
            DispatchGetPhysicalDeviceMultisamplePropertiesEXT(physical_device, cb_state.dynamic_state_value.rasterization_samples,
                                                              &multisample_prop);

            if (!IsIntegerMultipleOf(multisample_prop.maxSampleLocationGridSize.width,
                                     sample_locations->sampleLocationsInfo.sampleLocationGridSize.width)) {
                skip |= LogError(CreateActionVuid(loc.function, vvl::ActionVUID::SAMPLE_LOCATIONS_ENABLE_07936), objlist, loc,
                                 "VkMultisamplePropertiesEXT::maxSampleLocationGridSize.width (%" PRIu32
                                 ") with rasterization samples %s is not evenly divided by "
                                 "VkMultisamplePropertiesEXT::sampleLocationGridSize.width (%" PRIu32 ").",
                                 multisample_prop.maxSampleLocationGridSize.width,
                                 string_VkSampleCountFlagBits(cb_state.dynamic_state_value.rasterization_samples),
                                 sample_locations->sampleLocationsInfo.sampleLocationGridSize.width);
            }
            if (!IsIntegerMultipleOf(multisample_prop.maxSampleLocationGridSize.height,
                                     sample_locations->sampleLocationsInfo.sampleLocationGridSize.height)) {
                skip |= LogError(CreateActionVuid(loc.function, vvl::ActionVUID::SAMPLE_LOCATIONS_ENABLE_07937), objlist, loc,
                                 "VkMultisamplePropertiesEXT::maxSampleLocationGridSize.height (%" PRIu32
                                 ") with rasterization samples %s is not evenly divided by "
                                 "VkMultisamplePropertiesEXT::sampleLocationGridSize.height (%" PRIu32 ").",
                                 multisample_prop.maxSampleLocationGridSize.height,
                                 string_VkSampleCountFlagBits(cb_state.dynamic_state_value.rasterization_samples),
                                 sample_locations->sampleLocationsInfo.sampleLocationGridSize.height);
            }
            if (sample_locations->sampleLocationsInfo.sampleLocationsPerPixel !=
                cb_state.dynamic_state_value.rasterization_samples) {
                skip |= LogError(CreateActionVuid(loc.function, vvl::ActionVUID::SAMPLE_LOCATIONS_ENABLE_07938), objlist, loc,
                                 "Pipeline was created with "
                                 "VkPipelineSampleLocationsStateCreateInfoEXT::sampleLocationsInfo.sampleLocationsPerPixel %s "
                                 "which does not match rasterization samples (%s) set with vkCmdSetRasterizationSamplesEXT().",
                                 string_VkSampleCountFlagBits(sample_locations->sampleLocationsInfo.sampleLocationsPerPixel),
                                 string_VkSampleCountFlagBits(cb_state.dynamic_state_value.rasterization_samples));
            }
        }
    }

    if (pipeline.IsDynamic(CB_DYNAMIC_STATE_SAMPLE_MASK_EXT)) {
        if (!pipeline.IsDynamic(CB_DYNAMIC_STATE_RASTERIZATION_SAMPLES_EXT)) {
            if (cb_state.dynamic_state_value.samples_mask_samples < pipeline.MultisampleState()->rasterizationSamples) {
                skip |=
                    LogError(CreateActionVuid(loc.function, vvl::ActionVUID::SAMPLE_MASK_07472), objlist, loc,
                             "Currently bound pipeline was created with VkPipelineMultisampleStateCreateInfo::rasterizationSamples "
                             "%s are greater than samples set with vkCmdSetSampleMaskEXT() were %s.%s",
                             string_VkSampleCountFlagBits(pipeline.MultisampleState()->rasterizationSamples),
                             string_VkSampleCountFlagBits(cb_state.dynamic_state_value.samples_mask_samples),
                             cb_state.DescribeInvalidatedState(CB_DYNAMIC_STATE_SAMPLE_MASK_EXT).c_str());
            }
        } else if (cb_state.dynamic_state_value.samples_mask_samples < cb_state.dynamic_state_value.rasterization_samples) {
            skip |= LogError(CreateActionVuid(loc.function, vvl::ActionVUID::SAMPLE_MASK_07473), objlist, loc,
                             "rasterizationSamples set with vkCmdSetRasterizationSamplesEXT() %s are greater than samples "
                             "set with vkCmdSetSampleMaskEXT() were %s.%s",
                             string_VkSampleCountFlagBits(cb_state.dynamic_state_value.rasterization_samples),
                             string_VkSampleCountFlagBits(cb_state.dynamic_state_value.samples_mask_samples),
                             cb_state.DescribeInvalidatedState(CB_DYNAMIC_STATE_SAMPLE_MASK_EXT).c_str());
        }
    }

    if (pipeline.IsDynamic(CB_DYNAMIC_STATE_RASTERIZATION_STREAM_EXT) &&
        !enabled_features.primitivesGeneratedQueryWithNonZeroStreams && cb_state.dynamic_state_value.rasterization_stream != 0) {
        for (const auto& active_query : cb_state.active_queries) {
            auto query_pool_state = Get<vvl::QueryPool>(active_query.pool);
            if (!query_pool_state || query_pool_state->create_info.queryType != VK_QUERY_TYPE_PRIMITIVES_GENERATED_EXT) {
                continue;
            }
            skip |= LogError(
                CreateActionVuid(loc.function, vvl::ActionVUID::PRIMITIVES_GENERATED_QUERY_07481), cb_state.Handle(), loc,
                "query %" PRIu32
                " in %s with type VK_QUERY_TYPE_PRIMITIVES_GENERATED_EXT is active and the last call to "
                "vkCmdSetRasterizationStreamEXT() set rasterizationStreams as %" PRIu32
                " (non-zero), but the primitivesGeneratedQueryWithNonZeroStreams "
                "feature was not enabled.%s",
                active_query.slot, FormatHandle(active_query.pool).c_str(), cb_state.dynamic_state_value.rasterization_stream,
                cb_state.DescribeInvalidatedState(CB_DYNAMIC_STATE_RASTERIZATION_STREAM_EXT).c_str());

            break;  // only need to check the feature VUs once
        }
    }

    // VK_EXT_shader_tile_image
    {
        const bool dyn_depth_write_enable = pipeline.IsDynamic(CB_DYNAMIC_STATE_DEPTH_WRITE_ENABLE);
        const bool dyn_stencil_write_mask = pipeline.IsDynamic(CB_DYNAMIC_STATE_STENCIL_WRITE_MASK);
        auto fragment_entry_point = last_bound_state.GetFragmentEntryPoint();
        if ((dyn_depth_write_enable || dyn_stencil_write_mask) && fragment_entry_point) {
            const bool mode_early_fragment_test =
                fragment_entry_point->execution_mode.Has(spirv::ExecutionModeSet::early_fragment_test_bit);
            const bool depth_read =
                pipeline.fragment_shader_state->fragment_shader->spirv->static_data_.has_shader_tile_image_depth_read;
            const bool stencil_read =
                pipeline.fragment_shader_state->fragment_shader->spirv->static_data_.has_shader_tile_image_stencil_read;

            if (depth_read && dyn_depth_write_enable && mode_early_fragment_test &&
                cb_state.dynamic_state_value.depth_write_enable) {
                skip |= LogError(CreateActionVuid(loc.function, vvl::ActionVUID::DEPTH_ENABLE_08715), objlist, loc,
                                 "Fragment shader contains OpDepthAttachmentReadEXT, but depthWriteEnable parameter in the last "
                                 "call to vkCmdSetDepthWriteEnable is not false.");
            }

            if (stencil_read && dyn_stencil_write_mask && mode_early_fragment_test &&
                ((cb_state.dynamic_state_value.write_mask_front != 0) || (cb_state.dynamic_state_value.write_mask_back != 0))) {
                skip |= LogError(CreateActionVuid(loc.function, vvl::ActionVUID::STENCIL_WRITE_MASK_08716), objlist, loc,
                                 "Fragment shader contains OpStencilAttachmentReadEXT, but writeMask parameter in the last "
                                 "call to vkCmdSetStencilWriteMask is not equal to 0 for both front (%" PRIu32
                                 ") and back (%" PRIu32 ").",
                                 cb_state.dynamic_state_value.write_mask_front, cb_state.dynamic_state_value.write_mask_back);
            }
        }
    }

    // Makes sure topology is compatible (in same topology class)
    // see vkspec.html#drawing-primitive-topology-class
    if (pipeline.IsDynamic(CB_DYNAMIC_STATE_PRIMITIVE_TOPOLOGY) &&
        !phys_dev_ext_props.extended_dynamic_state3_props.dynamicPrimitiveTopologyUnrestricted && pipeline.InputAssemblyState()) {
        bool compatible_topology = false;
        const VkPrimitiveTopology pipeline_topology = pipeline.InputAssemblyState()->topology;
        const VkPrimitiveTopology dynamic_topology = cb_state.dynamic_state_value.primitive_topology;
        if (IsPointTopology(pipeline_topology)) {
            compatible_topology = dynamic_topology == VK_PRIMITIVE_TOPOLOGY_POINT_LIST;
        } else if (IsLineTopology(pipeline_topology)) {
            compatible_topology = IsLineTopology(dynamic_topology);
        } else if (IsTriangleTopology(pipeline_topology)) {
            compatible_topology = IsTriangleTopology(dynamic_topology);
        } else if (pipeline_topology == VK_PRIMITIVE_TOPOLOGY_PATCH_LIST) {
            compatible_topology = dynamic_topology == VK_PRIMITIVE_TOPOLOGY_PATCH_LIST;
        }

        if (!compatible_topology) {
            skip |= LogError(CreateActionVuid(loc.function, vvl::ActionVUID::PRIMITIVE_TOPOLOGY_CLASS_07500), objlist, loc,
                             "the last primitive topology %s state set by vkCmdSetPrimitiveTopology is "
                             "not compatible with the pipeline topology %s.%s",
                             string_VkPrimitiveTopology(dynamic_topology), string_VkPrimitiveTopology(pipeline_topology),
                             cb_state.DescribeInvalidatedState(CB_DYNAMIC_STATE_PRIMITIVE_TOPOLOGY).c_str());
        }
    }

    return skip;
}

bool CoreChecks::ValidateDrawDynamicStatePipelineViewportScissor(const LastBound& last_bound_state, const vvl::Pipeline& pipeline,
                                                                 const Location& loc) const {
    bool skip = false;
    const vvl::CommandBuffer& cb_state = last_bound_state.cb_state;
    auto& cb_sub_state = core::SubState(cb_state);
    const LogObjectList objlist(cb_state.Handle(), pipeline.Handle());

    // If Viewport or scissors are dynamic, verify that dynamic count matches PSO count.
    // Skip check if rasterization is disabled, if there is no viewport, or if viewport/scissors are being inherited.
    const bool dyn_viewport = pipeline.IsDynamic(CB_DYNAMIC_STATE_VIEWPORT);
    const auto* viewport_state = pipeline.ViewportState();
    if (!pipeline.RasterizationDisabled() && viewport_state && (cb_sub_state.viewport.inherited_depths.empty())) {
        const bool dyn_scissor = pipeline.IsDynamic(CB_DYNAMIC_STATE_SCISSOR);

        // NB (akeley98): Current validation layers do not detect the error where vkCmdSetViewport (or scissor) was called, but
        // the dynamic state set is overwritten by binding a graphics pipeline with static viewport (scissor) state.
        // This condition be detected by checking trashedViewportMask & viewportMask (trashedScissorMask & scissorMask) is
        // nonzero in the range of bits needed by the pipeline.
        if (dyn_viewport) {
            const auto required_viewports_mask = (1 << viewport_state->viewportCount) - 1;
            const auto missing_viewport_mask = ~cb_sub_state.viewport.mask & required_viewports_mask;
            if (missing_viewport_mask) {
                skip |= LogError(CreateActionVuid(loc.function, vvl::ActionVUID::VIEWPORT_07831), objlist, loc,
                                 "Dynamic viewport(s) (0x%x) are used by pipeline state object, but were not provided via calls "
                                 "to vkCmdSetViewport().",
                                 missing_viewport_mask);
            }
        }

        if (dyn_scissor) {
            const auto required_scissor_mask = (1 << viewport_state->scissorCount) - 1;
            const auto missing_scissor_mask = ~cb_sub_state.scissor.mask & required_scissor_mask;
            if (missing_scissor_mask) {
                skip |= LogError(CreateActionVuid(loc.function, vvl::ActionVUID::SCISSOR_07832), objlist, loc,
                                 "Dynamic scissor(s) (0x%x) are used by pipeline state object, but were not provided via calls "
                                 "to vkCmdSetScissor().",
                                 missing_scissor_mask);
            }
        }
    }

    // If inheriting viewports, verify that not using more than inherited.
    if (!cb_sub_state.viewport.inherited_depths.empty() && dyn_viewport) {
        const uint32_t viewport_count = viewport_state->viewportCount;
        const uint32_t max_inherited = uint32_t(cb_sub_state.viewport.inherited_depths.size());
        if (viewport_count > max_inherited) {
            skip |= LogError(CreateActionVuid(loc.function, vvl::ActionVUID::STATE_INHERITED_07850), objlist, loc,
                             "Pipeline requires more viewports (%" PRIu32 ".) than inherited (viewportDepthCount = %" PRIu32 ".).",
                             viewport_count, max_inherited);
        }
    }

    return skip;
}

bool CoreChecks::ValidateDrawDynamicState(const LastBound& last_bound_state, const Location& loc) const {
    bool skip = false;

    skip |= ValidateGraphicsDynamicStateSetStatus(last_bound_state, loc);
    // Dynamic state was not set, will produce garbage when trying to read to values
    if (skip) return skip;

    const auto pipeline_state = last_bound_state.pipeline_state;
    if (pipeline_state) {
        skip |= ValidateDrawDynamicStatePipeline(last_bound_state, *pipeline_state, loc);
    }

    skip |= ValidateDrawDynamicStateVertex(last_bound_state, loc);
    skip |= ValidateDrawDynamicStateFragment(last_bound_state, loc);

    // Once we know for sure state was set, check value is valid
    skip |= ValidateDrawDynamicStateValue(last_bound_state, loc);

    return skip;
}

bool CoreChecks::ValidateDrawDynamicStatePipeline(const LastBound& last_bound_state, const vvl::Pipeline& pipeline,
                                                  const Location& loc) const {
    bool skip = false;

    const vvl::CommandBuffer& cb_state = last_bound_state.cb_state;

    // Verify vkCmdSet* calls since last bound pipeline
    const CBDynamicFlags unset_status_pipeline =
        cb_state.dynamic_state_status.pipeline & ~(pipeline.dynamic_state | pipeline.ignored_dynamic_state);

    if (unset_status_pipeline.any()) {
        const LogObjectList objlist(cb_state.Handle(), pipeline.Handle());
        // https://gitlab.khronos.org/vulkan/vulkan/-/issues/4495
        // It is not invalid to set dynamic state here, but we should warn the user what they are doing may lead to values not being
        // what they think it should have been
        skip |= LogWarning("WARNING-Ignored-DynamicState", objlist, loc,
                           "%s was not created with full VkDynamicState, since the last vkCmdBindPipeline and this %s, the "
                           "following commands have been ignored and didn't set the state you may think they did.\n%s",
                           FormatHandle(pipeline).c_str(), String(loc.function),
                           DynamicStatesCommandsToString(unset_status_pipeline).c_str());
    }

    skip |= ValidateDrawDynamicStatePipelineValue(last_bound_state, pipeline, loc);
    skip |= ValidateDrawDynamicStatePipelineViewportScissor(last_bound_state, pipeline, loc);

    if (const vvl::RenderPass* rp_state = last_bound_state.cb_state.active_render_pass.get()) {
        skip |= ValidateDrawDynamicStatePipelineRenderPass(last_bound_state, pipeline, *rp_state, loc);
    }
    return skip;
}

bool CoreChecks::ValidateDrawDynamicStateVertex(const LastBound& last_bound_state, const Location& loc) const {
    bool skip = false;

    if (!last_bound_state.IsStageBound(VK_SHADER_STAGE_VERTEX_BIT)) {
        return skip;  // using mesh shaders
    }

    const vvl::CommandBuffer& cb_state = last_bound_state.cb_state;
    if (last_bound_state.IsDynamic(CB_DYNAMIC_STATE_PRIMITIVE_RESTART_ENABLE) &&
        cb_state.dynamic_state_value.primitive_restart_enable) {
        const VkPrimitiveTopology topology = last_bound_state.GetVertexInputAssemblerTopology();
        if (!enabled_features.primitiveTopologyListRestart) {
            if (IsValueIn(topology,
                          {VK_PRIMITIVE_TOPOLOGY_POINT_LIST, VK_PRIMITIVE_TOPOLOGY_LINE_LIST, VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
                           VK_PRIMITIVE_TOPOLOGY_LINE_LIST_WITH_ADJACENCY, VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST_WITH_ADJACENCY})) {
                skip |=
                    LogError(CreateActionVuid(loc.function, vvl::ActionVUID::PRIMITIVE_RESTART_09637), cb_state.Handle(), loc,
                             "%s is %s, the primitiveTopologyListRestart feature was not enabled, but "
                             "vkCmdSetPrimitiveRestartEnable last set primitiveRestartEnable to VK_TRUE.",
                             last_bound_state.DescribeVertexInputAssemblerTopology().c_str(), string_VkPrimitiveTopology(topology));
            }
        }
        if (!enabled_features.primitiveTopologyPatchListRestart) {
            if (topology == VK_PRIMITIVE_TOPOLOGY_PATCH_LIST) {
                skip |= LogError(
                    CreateActionVuid(loc.function, vvl::ActionVUID::PRIMITIVE_RESTART_10909), cb_state.Handle(), loc,
                    "%s is VK_PRIMITIVE_TOPOLOGY_PATCH_LIST, the primitiveTopologyPatchListRestart feature was not enabled, but "
                    "vkCmdSetPrimitiveRestartEnable last set primitiveRestartEnable to VK_TRUE.",
                    last_bound_state.DescribeVertexInputAssemblerTopology().c_str());
            }
        }
    }

    const ShaderStageState* vert_state = nullptr;
    if (last_bound_state.pipeline_state) {
        vert_state = last_bound_state.pipeline_state->GetShaderStageState(VK_SHADER_STAGE_VERTEX_BIT);
    } else if (const auto& shader_object = last_bound_state.GetShaderObjectState(ShaderObjectStage::VERTEX)) {
        vert_state = &shader_object->stage;
    }
    if (!vert_state || !vert_state->HasSpirv()) {
        return skip;  // Mesh shader
    }
    const spirv::Module& vert_spirv_state = *vert_state->spirv_state;
    const spirv::EntryPoint& vert_entrypoint = *vert_state->entrypoint;

    if (last_bound_state.IsDynamic(CB_DYNAMIC_STATE_VERTEX_INPUT_EXT)) {
        for (const auto* variable_ptr : vert_entrypoint.user_defined_interface_variables) {
            // Validate only input locations
            if (variable_ptr->storage_class != spv::StorageClass::StorageClassInput) {
                continue;
            }
            bool location_provided = false;
            for (const auto& vertex_binding : cb_state.dynamic_state_value.vertex_bindings) {
                const auto* attrib = vvl::Find(vertex_binding.second.locations, variable_ptr->decorations.location);
                if (!attrib) continue;
                location_provided = true;

                const uint32_t var_base_type_id = variable_ptr->base_type.ResultId();
                const uint32_t attribute_type = spirv::GetFormatNumericType(attrib->desc.format);
                const spirv::Instruction* var_base_type = vert_spirv_state.FindDef(var_base_type_id);
                const uint32_t var_numeric_type = vert_spirv_state.GetNumericType(*var_base_type);

                const bool attribute64 = vkuFormatIs64bit(attrib->desc.format);
                const bool shader64 = vert_spirv_state.GetBaseTypeInstruction(var_base_type)->GetBitWidth() == 64;

                // first type check before doing 64-bit matching
                if ((attribute_type & var_numeric_type) == 0) {
                    if (!enabled_features.legacyVertexAttributes || shader64) {
                        skip |= LogError(
                            CreateActionVuid(loc.function, vvl::ActionVUID::VERTEX_INPUT_08734), vert_spirv_state.handle(), loc,
                            "vkCmdSetVertexInputEXT set pVertexAttributeDescriptions[%" PRIu32 "] (binding %" PRIu32
                            ", location %" PRIu32 ") with format %s but the vertex shader %s is numeric type %s",
                            attrib->index, attrib->desc.binding, attrib->desc.location, string_VkFormat(attrib->desc.format),
                            variable_ptr->Describe().c_str(), vert_spirv_state.DescribeType(var_base_type_id).c_str());
                    }
                } else if (attribute64 && !shader64) {
                    skip |= LogError(
                        CreateActionVuid(loc.function, vvl::ActionVUID::VERTEX_INPUT_FORMAT_08936), vert_spirv_state.handle(), loc,
                        "vkCmdSetVertexInputEXT set pVertexAttributeDescriptions[%" PRIu32 "] (binding %" PRIu32
                        ", location %" PRIu32 ") with a 64-bit format (%s) but the vertex shader %s is a 32-bit type (%s)",
                        attrib->index, attrib->desc.binding, attrib->desc.location, string_VkFormat(attrib->desc.format),
                        variable_ptr->Describe().c_str(), vert_spirv_state.DescribeType(var_base_type_id).c_str());
                } else if (!attribute64 && shader64) {
                    skip |= LogError(
                        CreateActionVuid(loc.function, vvl::ActionVUID::VERTEX_INPUT_FORMAT_08937), vert_spirv_state.handle(), loc,
                        "vkCmdSetVertexInputEXT set pVertexAttributeDescriptions[%" PRIu32 "] (binding %" PRIu32
                        ", location %" PRIu32 ") with a 32-bit format (%s) but the vertex shader %s is a 64-bit type (%s)",
                        attrib->index, attrib->desc.binding, attrib->desc.location, string_VkFormat(attrib->desc.format),
                        variable_ptr->Describe().c_str(), vert_spirv_state.DescribeType(var_base_type_id).c_str());
                } else if (attribute64 && shader64) {
                    const uint32_t attribute_components = vkuFormatComponentCount(attrib->desc.format);
                    const uint32_t input_components = vert_spirv_state.GetNumComponentsInBaseType(&variable_ptr->base_type);
                    if (attribute_components < input_components) {
                        skip |= LogError(CreateActionVuid(loc.function, vvl::ActionVUID::VERTEX_INPUT_FORMAT_09203),
                                         vert_spirv_state.handle(), loc,
                                         "vkCmdSetVertexInputEXT set pVertexAttributeDescriptions[%" PRIu32 "] (binding %" PRIu32
                                         ", location %" PRIu32 ") with a %" PRIu32
                                         "-wide 64-bit format (%s) but the vertex shader %s is %" PRIu32
                                         "-wide. (64-bit vertex input don't have default values and require "
                                         "components to match what is used in the shader)",
                                         attrib->index, attrib->desc.binding, attrib->desc.location, attribute_components,
                                         string_VkFormat(attrib->desc.format), variable_ptr->Describe().c_str(), input_components);
                    }
                }
            }
            if (!location_provided && !enabled_features.vertexAttributeRobustness && !enabled_features.maintenance9) {
                skip |= LogError(CreateActionVuid(loc.function, vvl::ActionVUID::VERTEX_INPUT_FORMAT_07939),
                                 vert_spirv_state.handle(), loc,
                                 "Vertex shader %s is using Location %" PRIu32
                                 ", but it was not provided with vkCmdSetVertexInputEXT(). (This can be valid if "
                                 "either the vertexAttributeRobustness or maintenance9 feature is enabled)",
                                 variable_ptr->Describe().c_str(), variable_ptr->decorations.location);
            }
        }
    }

    // With VK_DYNAMIC_STATE_PRIMITIVE_TOPOLOGY and dynamicPrimitiveTopologyUnrestricted (for pipeline) we have a runtime check that
    // the topology makes sense
    if (last_bound_state.IsDynamic(CB_DYNAMIC_STATE_PRIMITIVE_TOPOLOGY)) {
        const VkShaderStageFlags bound_stages = last_bound_state.GetAllActiveBoundStages();
        // This is the Vertex Input Assembler Topology
        const VkPrimitiveTopology topology = cb_state.dynamic_state_value.primitive_topology;

        if (((bound_stages & (VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT | VK_SHADER_STAGE_GEOMETRY_BIT)) == 0) &&
            topology == VK_PRIMITIVE_TOPOLOGY_POINT_LIST) {
            if (!vert_entrypoint.written_built_in_point_size && !enabled_features.maintenance5) {
                skip |= LogError(CreateActionVuid(loc.function, vvl::ActionVUID::PRIMITIVE_TOPOLOGY_10748), cb_state.Handle(), loc,
                                 "The bound vertex shader (%s) has a PointSize that is not written to, but the bound topology "
                                 "is set to VK_PRIMITIVE_TOPOLOGY_POINT_LIST.",
                                 FormatHandle(vert_spirv_state.handle()).c_str());
            }
        }

        const bool tess_shader_bound = (bound_stages & VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT) != 0;
        const bool patch_topology = topology == VK_PRIMITIVE_TOPOLOGY_PATCH_LIST;
        if (tess_shader_bound && !patch_topology) {
            skip |= LogError(
                CreateActionVuid(loc.function, vvl::ActionVUID::PRIMITIVE_TOPOLOGY_10286), cb_state.Handle(), loc,
                "Tessellation shaders were bound, but the last call to vkCmdSetPrimitiveTopology set primitiveTopology to %s.",
                string_VkPrimitiveTopology(topology));
        } else if (!tess_shader_bound && patch_topology) {
            skip |= LogError(CreateActionVuid(loc.function, vvl::ActionVUID::PRIMITIVE_TOPOLOGY_10747), cb_state.Handle(), loc,
                             "Tessellation shaders were not bound, but the last call to vkCmdSetPrimitiveTopology set "
                             "primitiveTopology to VK_PRIMITIVE_TOPOLOGY_PATCH_LIST.");
        }
    }

    return skip;
}

bool CoreChecks::ValidateDrawDynamicStateFragment(const LastBound& last_bound_state, const Location& loc) const {
    bool skip = false;

    const ShaderStageState* frag_state = nullptr;
    if (last_bound_state.pipeline_state) {
        frag_state = last_bound_state.pipeline_state->GetShaderStageState(VK_SHADER_STAGE_FRAGMENT_BIT);
    } else if (const auto& fragment_state = last_bound_state.GetShaderObjectState(ShaderObjectStage::FRAGMENT)) {
        frag_state = &fragment_state->stage;
    }
    if (!frag_state || !frag_state->HasSpirv()) {
        return skip;  // no fragment shader used
    }
    const spirv::Module& frag_spirv_state = *frag_state->spirv_state;

    const vvl::CommandBuffer& cb_state = last_bound_state.cb_state;
    if (last_bound_state.IsDynamic(CB_DYNAMIC_STATE_SAMPLE_LOCATIONS_ENABLE_EXT)) {
        if (cb_state.IsDynamicStateSet(CB_DYNAMIC_STATE_SAMPLE_LOCATIONS_ENABLE_EXT) &&
            cb_state.dynamic_state_value.sample_locations_enable) {
            if (last_bound_state.IsDynamic(CB_DYNAMIC_STATE_SAMPLE_LOCATIONS_EXT) &&
                cb_state.IsDynamicStateSet(CB_DYNAMIC_STATE_SAMPLE_LOCATIONS_EXT)) {
                const VkSampleCountFlagBits rasterization_samples = last_bound_state.GetRasterizationSamples();
                VkMultisamplePropertiesEXT multisample_prop = vku::InitStructHelper();
                DispatchGetPhysicalDeviceMultisamplePropertiesEXT(physical_device, rasterization_samples, &multisample_prop);
                const auto& gridSize = cb_state.dynamic_state_value.sample_locations_info.sampleLocationGridSize;
                if (!IsIntegerMultipleOf(multisample_prop.maxSampleLocationGridSize.width, gridSize.width)) {
                    const LogObjectList objlist(cb_state.Handle(), frag_spirv_state.handle());
                    skip |= LogError(CreateActionVuid(loc.function, vvl::ActionVUID::SAMPLE_LOCATION_07485), objlist, loc,
                                     "VkMultisamplePropertiesEXT::maxSampleLocationGridSize.width (%" PRIu32
                                     ") with rasterization samples %s is not evenly divided by "
                                     "sampleLocationsInfo.sampleLocationGridSize.width (%" PRIu32
                                     ") set with vkCmdSetSampleLocationsEXT().",
                                     multisample_prop.maxSampleLocationGridSize.width,
                                     string_VkSampleCountFlagBits(rasterization_samples), gridSize.width);
                }
                if (!IsIntegerMultipleOf(multisample_prop.maxSampleLocationGridSize.height, gridSize.height)) {
                    const LogObjectList objlist(cb_state.Handle(), frag_spirv_state.handle());
                    skip |= LogError(CreateActionVuid(loc.function, vvl::ActionVUID::SAMPLE_LOCATION_07486), objlist, loc,
                                     "VkMultisamplePropertiesEXT::maxSampleLocationGridSize.height (%" PRIu32
                                     ") with rasterization samples %s is not evenly divided by "
                                     "sampleLocationsInfo.sampleLocationGridSize.height (%" PRIu32
                                     ") set with vkCmdSetSampleLocationsEXT().",
                                     multisample_prop.maxSampleLocationGridSize.height,
                                     string_VkSampleCountFlagBits(rasterization_samples), gridSize.height);
                }
            }
            if (frag_spirv_state.static_data_.uses_interpolate_at_sample) {
                const LogObjectList objlist(cb_state.Handle(), frag_spirv_state.handle());
                skip |= LogError(CreateActionVuid(loc.function, vvl::ActionVUID::SAMPLE_LOCATION_07487), objlist, loc,
                                 "sampleLocationsEnable set with vkCmdSetSampleLocationsEnableEXT() was VK_TRUE, but fragment "
                                 "shader uses InterpolateAtSample instruction.");
            }
        }
    }

    const vvl::RenderPass* rp_state = cb_state.active_render_pass.get();
    if (last_bound_state.IsDynamic(CB_DYNAMIC_STATE_RASTERIZATION_SAMPLES_EXT) &&
        cb_state.IsDynamicStateSet(CB_DYNAMIC_STATE_RASTERIZATION_SAMPLES_EXT) && rp_state) {
        const VkMultisampledRenderToSingleSampledInfoEXT* msrtss_info = rp_state->GetMSRTSSInfo(cb_state.GetActiveSubpass());
        if (msrtss_info && msrtss_info->multisampledRenderToSingleSampledEnable) {
            if (msrtss_info->rasterizationSamples != cb_state.dynamic_state_value.rasterization_samples) {
                LogObjectList objlist(cb_state.Handle(), frag_spirv_state.handle());
                skip |= LogError(CreateActionVuid(loc.function, vvl::ActionVUID::RASTERIZATION_SAMPLES_09211), objlist, loc,
                                 "VkMultisampledRenderToSingleSampledInfoEXT::multisampledRenderToSingleSampledEnable is VK_TRUE "
                                 "and VkMultisampledRenderToSingleSampledInfoEXT::rasterizationSamples are %s, but rasterization "
                                 "samples set with vkCmdSetRasterizationSamplesEXT() were %s.",
                                 string_VkSampleCountFlagBits(msrtss_info->rasterizationSamples),
                                 string_VkSampleCountFlagBits(cb_state.dynamic_state_value.rasterization_samples));
            }
        }
    }

    return skip;
}

// Here we assume the dynamic state was set. These are draw time checks that things that normally checked at static pipeline
// creation time
bool CoreChecks::ValidateDrawDynamicStateValue(const LastBound& last_bound_state, const Location& loc) const {
    bool skip = false;

    const vvl::CommandBuffer& cb_state = last_bound_state.cb_state;
    const auto pipeline_state = last_bound_state.pipeline_state;
    const bool has_pipeline = last_bound_state.pipeline_state != nullptr;
    const bool fragment_shader_bound = has_pipeline || last_bound_state.IsValidShaderObjectBound(ShaderObjectStage::FRAGMENT);

    if (!last_bound_state.IsRasterizationDisabled()) {
        if (IsExtEnabled(extensions.vk_ext_discard_rectangles) && last_bound_state.IsDiscardRectangleEnable() &&
            last_bound_state.IsDynamic(CB_DYNAMIC_STATE_DISCARD_RECTANGLE_EXT)) {
            // For pipelines with VkPipelineDiscardRectangleStateCreateInfoEXT, we compare against discardRectangleCount,
            // but for Shader Objects we compare against maxDiscardRectangles (details in
            // https://gitlab.khronos.org/vulkan/vulkan/-/issues/3400)
            uint32_t rect_limit = phys_dev_ext_props.discard_rectangle_props.maxDiscardRectangles;
            bool use_max_limit = true;
            if (has_pipeline) {
                if (const auto* discard_rectangle_state = vku::FindStructInPNextChain<VkPipelineDiscardRectangleStateCreateInfoEXT>(
                        last_bound_state.pipeline_state->GetCreateInfoPNext())) {
                    rect_limit = discard_rectangle_state->discardRectangleCount;
                    use_max_limit = false;
                }
            }

            // vkCmdSetDiscardRectangleEXT needs to be set on each rectangle
            for (uint32_t i = 0; i < rect_limit; i++) {
                if (!cb_state.dynamic_state_value.discard_rectangles.test(i)) {
                    const vvl::Field limit_name =
                        use_max_limit ? vvl::Field::maxDiscardRectangles : vvl::Field::discardRectangleCount;
                    vvl::ActionVUID vuid =
                        use_max_limit ? vvl::ActionVUID::DISCARD_RECTANGLE_09236 : vvl::ActionVUID::DISCARD_RECTANGLE_07751;
                    skip |= LogError(CreateActionVuid(loc.function, vuid), cb_state.Handle(), loc,
                                     "vkCmdSetDiscardRectangleEXT was not set for discard rectangle index %" PRIu32
                                     " for this command buffer. It needs to be set once for each rectangle in %s (%" PRIu32 ").%s",
                                     i, String(limit_name), rect_limit,
                                     cb_state.DescribeInvalidatedState(CB_DYNAMIC_STATE_DISCARD_RECTANGLE_EXT).c_str());
                    break;
                }
            }
        }

        if (fragment_shader_bound) {
            if (IsExtEnabled(extensions.vk_nv_fragment_coverage_to_color) &&
                last_bound_state.IsDynamic(CB_DYNAMIC_STATE_COVERAGE_TO_COLOR_ENABLE_NV) &&
                last_bound_state.IsDynamic(CB_DYNAMIC_STATE_COVERAGE_TO_COLOR_LOCATION_NV)) {
                if (cb_state.dynamic_state_value.coverage_to_color_enable &&
                    cb_state.dynamic_state_value.coverage_to_color_location < cb_state.active_attachments.size()) {
                    // This only works because we just happen to put color attachments all in the front of active_attachments
                    VkFormat format = cb_state.active_attachments[cb_state.dynamic_state_value.coverage_to_color_location]
                                          .image_view->create_info.format;
                    if (!IsValueIn(format, {VK_FORMAT_R8_UINT, VK_FORMAT_R8_SINT, VK_FORMAT_R16_UINT, VK_FORMAT_R16_SINT,
                                            VK_FORMAT_R32_UINT, VK_FORMAT_R32_SINT})) {
                        vvl::ActionVUID vuid =
                            has_pipeline ? vvl::ActionVUID::COVERAGE_TO_COLOR_07490 : vvl::ActionVUID::COVERAGE_TO_COLOR_09420;
                        skip |=
                            LogError(CreateActionVuid(loc.function, vuid), cb_state.Handle(), loc,
                                     "coverageToColorLocation (%" PRIu32
                                     ") set by vkCmdSetCoverageToColorLocationNV points to a color attachment with format %s.\n%s",
                                     cb_state.dynamic_state_value.coverage_to_color_location, string_VkFormat(format),
                                     cb_state.DescribeActiveColorAttachment());
                    }
                }
            }
        }

        // The VU might "seem" like its not for dynamic, but if not using VK_DYNAMIC_STATE_RASTERIZATION_SAMPLES_EXT this check
        // is picked up by VUID-VkGraphicsPipelineCreateInfo-multisampledRenderToSingleSampled-06853 and
        // VUID-vkCmdDraw-multisampledRenderToSingleSampled-07285 already
        if (!IsMixSamplingSupported() && last_bound_state.IsDynamic(CB_DYNAMIC_STATE_RASTERIZATION_SAMPLES_EXT)) {
            const VkSampleCountFlagBits rasterization_samples = cb_state.dynamic_state_value.rasterization_samples;
            for (uint32_t i = 0; i < cb_state.active_attachments.size(); ++i) {
                const auto& attachment_info = cb_state.active_attachments[i];
                const auto* attachment = attachment_info.image_view;
                if (attachment && (attachment_info.IsColor() || attachment_info.IsDepthOrStencil()) &&
                    rasterization_samples != attachment->samples) {
                    skip |= LogError(
                        CreateActionVuid(loc.function, vvl::ActionVUID::SET_RASTERIZATION_SAMPLES_08644), cb_state.Handle(), loc,
                        "%s was created with %s but the last call to vkCmdSetRasterizationSamplesEXT was set to %s.",
                        attachment_info.Describe(cb_state, i).c_str(), string_VkSampleCountFlagBits(attachment->samples),
                        string_VkSampleCountFlagBits(rasterization_samples));
                }
            }
        }

        if (IsExtEnabled(extensions.vk_ext_conservative_rasterization) &&
            !phys_dev_ext_props.conservative_rasterization_props.conservativePointAndLineRasterization) {
            if (cb_state.dynamic_state_value.conservative_rasterization_mode != VK_CONSERVATIVE_RASTERIZATION_MODE_DISABLED_EXT) {
                const VkPrimitiveTopology topology = last_bound_state.GetRasterizationInputTopology();
                if (IsLineTopology(topology) || IsPointTopology(topology)) {
                    skip |= LogError(
                        CreateActionVuid(loc.function, vvl::ActionVUID::CONVERVATIVE_RASTERIZATION_07499), cb_state.Handle(), loc,
                        "the rasterization input topology is %s and conservativePointAndLineRasterization is VK_FALSE, but "
                        "conservativeRasterizationMode set with vkCmdSetConservativeRasterizationModeEXT() was %s.%s",
                        string_VkPrimitiveTopology(topology),
                        string_VkConservativeRasterizationModeEXT(cb_state.dynamic_state_value.conservative_rasterization_mode),
                        cb_state.DescribeInvalidatedState(CB_DYNAMIC_STATE_CONSERVATIVE_RASTERIZATION_MODE_EXT).c_str());
                }
            }
        }
    }

    if (last_bound_state.IsDynamic(CB_DYNAMIC_STATE_COLOR_WRITE_MASK_EXT)) {
        for (uint32_t i = 0; i < cb_state.active_attachments.size(); ++i) {
            const auto& attachment_info = cb_state.active_attachments[i];
            const auto* attachment = attachment_info.image_view;
            if (attachment && attachment->create_info.format == VK_FORMAT_E5B9G9R9_UFLOAT_PACK32) {
                const uint32_t color_index = attachment_info.type_index;
                const auto color_write_mask = cb_state.dynamic_state_value.color_write_masks[color_index];
                VkColorComponentFlags rgb = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT;
                if ((color_write_mask & rgb) != rgb && (color_write_mask & rgb) != 0) {
                    skip |= LogError(
                        CreateActionVuid(loc.function, vvl::ActionVUID::COLOR_WRITE_MASK_09116), cb_state.Handle(), loc,
                        "%s has format VK_FORMAT_E5B9G9R9_UFLOAT_PACK32, but vkCmdSetColorWriteMaskEXT::pColorWriteMasks[%" PRIu32
                        "] is %s.",
                        attachment_info.Describe(cb_state, i).c_str(), color_index,
                        string_VkColorComponentFlags(color_write_mask).c_str());
                }
            }
        }
    }

    if (last_bound_state.IsDynamic(CB_DYNAMIC_STATE_LINE_RASTERIZATION_MODE_EXT) &&
        (cb_state.dynamic_state_value.line_rasterization_mode == VK_LINE_RASTERIZATION_MODE_BRESENHAM ||
         cb_state.dynamic_state_value.line_rasterization_mode == VK_LINE_RASTERIZATION_MODE_RECTANGULAR_SMOOTH)) {
        const VkPrimitiveTopology topology = last_bound_state.GetRasterizationInputTopology();
        if (IsLineTopology(topology)) {
            const bool alpha_to_coverage_enable = last_bound_state.IsAlphaToCoverageEnable();
            const bool alpha_to_one_enable = last_bound_state.IsAlphaToOneEnable();
            const bool sample_shading_enable =
                pipeline_state && pipeline_state->MultisampleState() && pipeline_state->MultisampleState()->sampleShadingEnable;
            if (alpha_to_coverage_enable || alpha_to_one_enable || sample_shading_enable) {
                skip |= LogError(CreateActionVuid(loc.function, vvl::ActionVUID::LINE_RASTERIZATION_10608), cb_state.Handle(), loc,
                                 "lineRasterizationMode is %s, but alphaToCoverageEnable (%s), alphaToOneEnable (%s) and "
                                 "sampleShadingEnable (%s) are not all VK_FALSE.",
                                 string_VkLineRasterizationMode(cb_state.dynamic_state_value.line_rasterization_mode),
                                 alpha_to_coverage_enable ? "VK_TRUE" : "VK_FALSE", alpha_to_one_enable ? "VK_TRUE" : "VK_FALSE",
                                 sample_shading_enable ? "VK_TRUE" : "VK_FALSE");
            }
        }
    }

    if (last_bound_state.IsDynamic(CB_DYNAMIC_STATE_VIEWPORT_WITH_COUNT)) {
        if (last_bound_state.IsDynamic(CB_DYNAMIC_STATE_SCISSOR_WITH_COUNT)) {
            if (cb_state.dynamic_state_value.viewport_count != cb_state.dynamic_state_value.scissor_count) {
                skip |= LogError(
                    CreateActionVuid(loc.function, vvl::ActionVUID::VIEWPORT_AND_SCISSOR_WITH_COUNT_03419), cb_state.Handle(), loc,
                    "viewportCount (%" PRIu32 ") set with vkCmdSetViewportWithCount is not equal to scissorCount (%" PRIu32
                    ") set with vkCmdSetScissorWithCount",
                    cb_state.dynamic_state_value.viewport_count, cb_state.dynamic_state_value.scissor_count);
            }
        }

        if (!last_bound_state.IsRasterizationDisabled()) {
            if (enabled_features.shadingRateImage && last_bound_state.IsShadingRateImageEnable() &&
                last_bound_state.IsDynamic(CB_DYNAMIC_STATE_VIEWPORT_SHADING_RATE_PALETTE_NV)) {
                if (cb_state.dynamic_state_value.shading_rate_palette_count < cb_state.dynamic_state_value.viewport_count) {
                    skip |= LogError(
                        CreateActionVuid(loc.function, vvl::ActionVUID::SHADING_RATE_PALETTE_08637), cb_state.Handle(), loc,
                        "viewportCount (%" PRIu32 ") set with vkCmdSetViewportWithCount is greater than viewportCount (%" PRIu32
                        ") set with vkCmdSetViewportShadingRatePaletteNV",
                        cb_state.dynamic_state_value.viewport_count, cb_state.dynamic_state_value.shading_rate_palette_count);
                }
            }
        }

        if (IsExtEnabled(extensions.vk_nv_viewport_swizzle) && last_bound_state.IsDynamic(CB_DYNAMIC_STATE_VIEWPORT_SWIZZLE_NV)) {
            const uint32_t viewport_swizzle_count = last_bound_state.GetViewportSwizzleCount();
            if (viewport_swizzle_count < cb_state.dynamic_state_value.viewport_count) {
                vvl::ActionVUID vuid =
                    has_pipeline ? vvl::ActionVUID::SET_VIEWPORT_SWIZZLE_07493 : vvl::ActionVUID::SET_VIEWPORT_SWIZZLE_09421;
                skip |= LogError(CreateActionVuid(loc.function, vuid), cb_state.Handle(), loc,
                                 "viewportCount (%" PRIu32
                                 ") set with vkCmdSetViewportWithCount is greater than viewportCount (%" PRIu32 ") set with %s",
                                 cb_state.dynamic_state_value.viewport_count, viewport_swizzle_count,
                                 has_pipeline ? "VkPipelineViewportSwizzleStateCreateInfoNV" : "vkCmdSetViewportSwizzleNV");
            }
        }

        if (IsExtEnabled(extensions.vk_nv_clip_space_w_scaling) && last_bound_state.IsViewportWScalingEnable() &&
            last_bound_state.IsDynamic(CB_DYNAMIC_STATE_VIEWPORT_W_SCALING_NV)) {
            // There is no pipeline equivalent of vkCmdSetViewportWScalingNV
            if (cb_state.dynamic_state_value.viewport_w_scaling_count < cb_state.dynamic_state_value.viewport_count) {
                skip |= LogError(
                    CreateActionVuid(loc.function, vvl::ActionVUID::VIEWPORT_W_SCALING_08636), cb_state.Handle(), loc,
                    "viewportCount (%" PRIu32 ") set with vkCmdSetViewportWithCount is greater than viewportCount (%" PRIu32
                    ") set with vkCmdSetViewportWScalingNV",
                    cb_state.dynamic_state_value.viewport_count, cb_state.dynamic_state_value.viewport_w_scaling_count);
            }
        }
    }

    if (last_bound_state.IsDynamic(CB_DYNAMIC_STATE_ALPHA_TO_COVERAGE_ENABLE_EXT) &&
        cb_state.dynamic_state_value.alpha_to_coverage_enable) {
        auto fragment_entry_point = last_bound_state.GetFragmentEntryPoint();
        if (fragment_entry_point && !fragment_entry_point->has_alpha_to_coverage_variable) {
            LogObjectList objlist(cb_state.Handle());
            if (has_pipeline) {
                objlist.add(pipeline_state->Handle());
            } else {
                objlist.add(last_bound_state.GetShaderObject(ShaderObjectStage::FRAGMENT));
            }
            vvl::ActionVUID vuid =
                has_pipeline ? vvl::ActionVUID::ALPHA_TO_COVERAGE_COMPONENT_08919 : vvl::ActionVUID::ALPHA_COMPONENT_WORD_08920;
            skip |= LogError(CreateActionVuid(loc.function, vuid), objlist, loc,
                             "vkCmdSetAlphaToCoverageEnableEXT set alphaToCoverageEnable to true but the bound "
                             "fragment shader doesn't declare a variable that covers Location 0, Component 3 (alpha channel).");
        }
    }

    if (enabled_features.multiviewPerViewViewports) {
        const uint32_t view_mask = cb_state.GetViewMask();
        if (view_mask != 0) {
            const uint32_t msb = (uint32_t)MostSignificantBit(view_mask);
            if (last_bound_state.IsDynamic(CB_DYNAMIC_STATE_VIEWPORT_WITH_COUNT) &&
                msb >= cb_state.dynamic_state_value.viewport_count) {
                skip |= LogError(CreateActionVuid(loc.function, vvl::ActionVUID::VIEWPORT_MULTIVIEW_12262), cb_state.Handle(), loc,
                                 "The current viewMask (0x%" PRIx32 ") most significant bit index (%" PRIu32
                                 ") is not less than viewportCount (%" PRIu32 ") set with vkCmdSetViewportWithCount",
                                 view_mask, msb, cb_state.dynamic_state_value.viewport_count);
            }
            if (last_bound_state.IsDynamic(CB_DYNAMIC_STATE_SCISSOR_WITH_COUNT) &&
                msb >= cb_state.dynamic_state_value.scissor_count) {
                skip |= LogError(CreateActionVuid(loc.function, vvl::ActionVUID::SCISSOR_MULTIVIEW_12263), cb_state.Handle(), loc,
                                 "The current viewMask (0x%" PRIx32 ") most significant bit index (%" PRIu32
                                 ") is not less than scissorCount (%" PRIu32 ") set with vkCmdSetScissorWithCount",
                                 view_mask, msb, cb_state.dynamic_state_value.scissor_count);
            }
        }
    }

    return skip;
}

bool CoreChecks::ValidateDrawRenderingAttachmentLocation(const vvl::CommandBuffer& cb_state, const vvl::Pipeline& pipeline_state,
                                                         const Location& loc) const {
    bool skip = false;
    const uint32_t color_attachment_count = (uint32_t)cb_state.rendering_attachments.color_locations.size();

    // Default from spec
    uint32_t pipeline_color_count = pipeline_state.ColorBlendState() ? pipeline_state.ColorBlendState()->attachmentCount : 0;
    const uint32_t* pipeline_color_locations = nullptr;

    // if no fragment output, Locations are useless
    if (!pipeline_state.fragment_output_state) {
        return skip;
    }

    bool explicit_pipeline = false;
    if (auto info = vku::FindStructInPNextChain<VkRenderingAttachmentLocationInfo>(
            pipeline_state.fragment_output_state->parent.GetCreateInfoPNext())) {
        explicit_pipeline = true;
        pipeline_color_count = info->colorAttachmentCount;
        pipeline_color_locations = info->pColorAttachmentLocations;
    }

    // If the count mismatches, that will either be caught by 06179 or allowed,
    // and we should only check the attachments that will be rendered to
    uint32_t count = std::min(pipeline_color_count, color_attachment_count);
    for (uint32_t i = 0; i < count; i++) {
        uint32_t pipeline_color_location = pipeline_color_locations ? pipeline_color_locations[i] : i;
        if (pipeline_color_location != cb_state.rendering_attachments.color_locations[i]) {
            const LogObjectList objlist(cb_state.Handle(), pipeline_state.Handle());
            std::ostringstream ss;
            ss << "The pipeline VkRenderingAttachmentLocationInfo::pColorAttachmentLocations[" << i << "] is "
               << pipeline_color_location;
            if (!explicit_pipeline) {
                ss << " (implicitly because the pipeline was created without VkRenderingAttachmentLocationInfo)";
            }
            ss << ", but doesn't match this render pass instance because vkCmdSetRenderingAttachmentLocations ";
            if (cb_state.rendering_attachments.set_color_locations) {
                ss << "last set pColorAttachmentLocations[" << i << "] to " << cb_state.rendering_attachments.color_locations[i];
            } else {
                ss << "was not called in this render pass so the index (" << i << ") is the implicit location";
            }
            skip |= LogError(CreateActionVuid(loc.function, vvl::ActionVUID::DYNAMIC_RENDERING_LOCAL_LOCATION_09548), objlist, loc,
                             "%s", ss.str().c_str());
            break;
        }
    }
    return skip;
}

bool CoreChecks::ValidateDrawRenderingInputAttachmentIndex(const vvl::CommandBuffer& cb_state, const vvl::Pipeline& pipeline_state,
                                                           const Location& loc) const {
    bool skip = false;
    const uint32_t color_index_count = (uint32_t)cb_state.rendering_attachments.color_indexes.size();

    // Default from spec
    uint32_t pipeline_color_count = pipeline_state.ColorBlendState() ? pipeline_state.ColorBlendState()->attachmentCount : 0;
    const uint32_t* pipeline_color_indexes = nullptr;
    const uint32_t* pipeline_depth_index = nullptr;
    const uint32_t* pipeline_stencil_index = nullptr;

    // if no fragment shader, Index are useless
    if (!pipeline_state.fragment_shader_state) {
        return skip;
    }

    bool explicit_pipeline = false;
    if (auto info = vku::FindStructInPNextChain<VkRenderingInputAttachmentIndexInfo>(
            pipeline_state.fragment_shader_state->parent.GetCreateInfoPNext())) {
        explicit_pipeline = true;
        pipeline_color_count = info->colorAttachmentCount;
        pipeline_color_indexes = info->pColorAttachmentInputIndices;
        pipeline_depth_index = info->pDepthInputAttachmentIndex;
        pipeline_stencil_index = info->pStencilInputAttachmentIndex;
    }

    // If the count mismatches, that will either be caught by 06179 or allowed,
    // and we should only check the attachments that will be rendered to
    uint32_t count = std::min(pipeline_color_count, color_index_count);
    for (uint32_t i = 0; i < count; i++) {
        if (!pipeline_color_indexes && cb_state.rendering_attachments.color_indexes[i] == VK_ATTACHMENT_UNUSED) {
            continue;
        }
        uint32_t pipeline_color_index = pipeline_color_indexes ? pipeline_color_indexes[i] : i;
        if (pipeline_color_index != cb_state.rendering_attachments.color_indexes[i]) {
            const LogObjectList objlist(cb_state.Handle(), pipeline_state.Handle());
            std::ostringstream ss;
            ss << "The pipeline VkRenderingInputAttachmentIndexInfo::pColorAttachmentInputIndices[" << i << "] is "
               << pipeline_color_index;
            if (!explicit_pipeline) {
                ss << " (implicitly because the pipeline was created without VkRenderingInputAttachmentIndexInfo)";
            }
            ss << ", but doesn't match this render pass instance because vkCmdSetRenderingInputAttachmentIndices ";
            if (cb_state.rendering_attachments.set_color_indexes) {
                ss << "last set pColorAttachmentInputIndices[" << i << "] to " << cb_state.rendering_attachments.color_indexes[i];
            } else {
                ss << "was not called in this render pass so the index (" << i << ") is the implicit location";
            }
            skip |= LogError(CreateActionVuid(loc.function, vvl::ActionVUID::DYNAMIC_RENDERING_LOCAL_INDEX_09549), objlist, loc,
                             "%s", ss.str().c_str());
            break;
        }
    }

    if (!EqualValuesOrBothNull(pipeline_depth_index, cb_state.rendering_attachments.depth_index)) {
        const LogObjectList objlist(cb_state.Handle(), pipeline_state.Handle());
        std::ostringstream ss;
        ss << "The pipeline VkRenderingInputAttachmentIndexInfo::pDepthInputAttachmentIndex is "
           << string_AttachmentPointer(pipeline_depth_index);
        if (!explicit_pipeline) {
            ss << " (implicitly because the pipeline was created without VkRenderingInputAttachmentIndexInfo)";
        }
        ss << ", but doesn't match this render pass instance because vkCmdSetRenderingInputAttachmentIndices ";
        if (cb_state.rendering_attachments.set_color_indexes) {
            ss << "last set pDepthInputAttachmentIndex to " << string_AttachmentPointer(cb_state.rendering_attachments.depth_index);
        } else {
            ss << "was not called in this render pass so pDepthInputAttachmentIndex is implicitly NULL";
        }
        skip |= LogError(CreateActionVuid(loc.function, vvl::ActionVUID::DYNAMIC_RENDERING_LOCAL_INDEX_10927), objlist, loc, "%s",
                         ss.str().c_str());
    }

    if (!EqualValuesOrBothNull(pipeline_stencil_index, cb_state.rendering_attachments.stencil_index)) {
        const LogObjectList objlist(cb_state.Handle(), pipeline_state.Handle());
        std::ostringstream ss;
        ss << "The pipeline VkRenderingInputAttachmentIndexInfo::pStencilInputAttachmentIndex is "
           << string_AttachmentPointer(pipeline_stencil_index);
        if (!explicit_pipeline) {
            ss << " (implicitly because the pipeline was created without VkRenderingInputAttachmentIndexInfo)";
        }
        ss << ", but doesn't match this render pass instance because vkCmdSetRenderingInputAttachmentIndices ";
        if (cb_state.rendering_attachments.set_color_indexes) {
            ss << "last set pStencilInputAttachmentIndex to "
               << string_AttachmentPointer(cb_state.rendering_attachments.stencil_index);
        } else {
            ss << "was not called in this render pass so pStencilInputAttachmentIndex is implicitly NULL";
        }
        skip |= LogError(CreateActionVuid(loc.function, vvl::ActionVUID::DYNAMIC_RENDERING_LOCAL_INDEX_10928), objlist, loc, "%s",
                         ss.str().c_str());
    }
    return skip;
}

bool CoreChecks::ValidateTraceRaysDynamicStateSetStatus(const LastBound& last_bound_state, const vvl::Pipeline& pipeline,
                                                        const Location& loc) const {
    bool skip = false;
    const vvl::CommandBuffer& cb_state = last_bound_state.cb_state;

    if (pipeline.IsDynamic(CB_DYNAMIC_STATE_RAY_TRACING_PIPELINE_STACK_SIZE_KHR)) {
        if (!cb_state.dynamic_state_status.rtx_stack_size_cb) {
            const LogObjectList objlist(cb_state.Handle(), pipeline.Handle());
            skip |= LogError(CreateActionVuid(loc.function, vvl::ActionVUID::RTX_STACK_SIZE_09458), objlist, loc,
                             "VK_DYNAMIC_STATE_RAY_TRACING_PIPELINE_STACK_SIZE_KHR state is dynamic, but the command buffer never "
                             "called vkCmdSetRayTracingPipelineStackSizeKHR().");
        }
    } else {
        if (cb_state.dynamic_state_status.rtx_stack_size_pipeline) {
            const LogObjectList objlist(cb_state.Handle(), pipeline.Handle());
            // https://gitlab.khronos.org/vulkan/vulkan/-/issues/4495
            // We decided this is not invalid, but should be a warning to the user
            skip |= LogWarning(
                "WARNING-Ignored-vkCmdSetRayTracingPipelineStackSizeKHR", objlist, loc,
                "%s doesn't set up VK_DYNAMIC_STATE_RAY_TRACING_PIPELINE_STACK_SIZE_KHR, but since the last vkCmdBindPipeline, "
                "vkCmdSetRayTracingPipelineStackSizeKHR was called and will have no impact on the stack size.",
                FormatHandle(pipeline).c_str());
        }
    }

    return skip;
}

bool CoreChecks::ForbidInheritedViewportScissor(const vvl::CommandBuffer& cb_state, const char* vuid, const Location& loc) const {
    bool skip = false;
    auto& cb_sub_state = core::SubState(cb_state);
    if (!cb_sub_state.viewport.inherited_depths.empty()) {
        skip |= LogError(vuid, cb_state.Handle(), loc,
                         "commandBuffer must not have VkCommandBufferInheritanceViewportScissorInfoNV::viewportScissor2D enabled.");
    }
    return skip;
}

bool CoreChecks::PreCallValidateCmdSetViewport(VkCommandBuffer commandBuffer, uint32_t firstViewport, uint32_t viewportCount,
                                               const VkViewport* pViewports, const ErrorObject& error_obj) const {
    auto cb_state = GetRead<vvl::CommandBuffer>(commandBuffer);
    bool skip = false;
    skip |= ValidateCmd(*cb_state, error_obj.location);
    skip |= ForbidInheritedViewportScissor(*cb_state, "VUID-vkCmdSetViewport-commandBuffer-04821", error_obj.location);
    return skip;
}

bool CoreChecks::PreCallValidateCmdSetScissor(VkCommandBuffer commandBuffer, uint32_t firstScissor, uint32_t scissorCount,
                                              const VkRect2D* pScissors, const ErrorObject& error_obj) const {
    auto cb_state = GetRead<vvl::CommandBuffer>(commandBuffer);
    bool skip = false;
    skip |= ValidateCmd(*cb_state, error_obj.location);
    skip |= ForbidInheritedViewportScissor(*cb_state, "VUID-vkCmdSetScissor-viewportScissor2D-04789", error_obj.location);
    return skip;
}

bool CoreChecks::PreCallValidateCmdSetExclusiveScissorNV(VkCommandBuffer commandBuffer, uint32_t firstExclusiveScissor,
                                                         uint32_t exclusiveScissorCount, const VkRect2D* pExclusiveScissors,
                                                         const ErrorObject& error_obj) const {
    auto cb_state = GetRead<vvl::CommandBuffer>(commandBuffer);
    bool skip = false;
    if (!enabled_features.exclusiveScissor) {
        skip |= LogError("VUID-vkCmdSetExclusiveScissorNV-None-02031", commandBuffer, error_obj.location,
                         "exclusiveScissor feature was not enabled.");
    }
    skip |= ValidateCmd(*cb_state, error_obj.location);
    return skip;
}

bool CoreChecks::PreCallValidateCmdSetViewportShadingRatePaletteNV(VkCommandBuffer commandBuffer, uint32_t firstViewport,
                                                                   uint32_t viewportCount,
                                                                   const VkShadingRatePaletteNV* pShadingRatePalettes,
                                                                   const ErrorObject& error_obj) const {
    auto cb_state = GetRead<vvl::CommandBuffer>(commandBuffer);
    bool skip = false;

    if (!enabled_features.shadingRateImage) {
        skip |= LogError("VUID-vkCmdSetViewportShadingRatePaletteNV-None-02064", commandBuffer, error_obj.location,
                         "shadingRateImage feature was not enabled.");
    }
    skip |= ValidateCmd(*cb_state, error_obj.location);

    for (uint32_t i = 0; i < viewportCount; ++i) {
        auto* palette = &pShadingRatePalettes[i];
        if (palette->shadingRatePaletteEntryCount == 0 ||
            palette->shadingRatePaletteEntryCount > phys_dev_ext_props.shading_rate_image_props.shadingRatePaletteSize) {
            skip |=
                LogError("VUID-VkShadingRatePaletteNV-shadingRatePaletteEntryCount-02071", commandBuffer,
                         error_obj.location.dot(Field::pShadingRatePalettes, i).dot(Field::shadingRatePaletteEntryCount),
                         "(%" PRIu32 ") must be between 1 and shadingRatePaletteSize (%" PRIu32 ").",
                         palette->shadingRatePaletteEntryCount, phys_dev_ext_props.shading_rate_image_props.shadingRatePaletteSize);
        }
    }

    return skip;
}

bool CoreChecks::PreCallValidateCmdSetViewportWScalingNV(VkCommandBuffer commandBuffer, uint32_t firstViewport,
                                                         uint32_t viewportCount, const VkViewportWScalingNV* pViewportWScalings,
                                                         const ErrorObject& error_obj) const {
    auto cb_state = GetRead<vvl::CommandBuffer>(commandBuffer);
    return ValidateCmd(*cb_state, error_obj.location);
}

bool CoreChecks::PreCallValidateCmdSetLineWidth(VkCommandBuffer commandBuffer, float lineWidth,
                                                const ErrorObject& error_obj) const {
    auto cb_state = GetRead<vvl::CommandBuffer>(commandBuffer);
    return ValidateCmd(*cb_state, error_obj.location);
}

bool CoreChecks::PreCallValidateCmdSetLineStipple(VkCommandBuffer commandBuffer, uint32_t lineStippleFactor,
                                                  uint16_t lineStipplePattern, const ErrorObject& error_obj) const {
    auto cb_state = GetRead<vvl::CommandBuffer>(commandBuffer);
    return ValidateCmd(*cb_state, error_obj.location);
}

bool CoreChecks::PreCallValidateCmdSetLineStippleEXT(VkCommandBuffer commandBuffer, uint32_t lineStippleFactor,
                                                     uint16_t lineStipplePattern, const ErrorObject& error_obj) const {
    return PreCallValidateCmdSetLineStipple(commandBuffer, lineStippleFactor, lineStipplePattern, error_obj);
}

bool CoreChecks::PreCallValidateCmdSetLineStippleKHR(VkCommandBuffer commandBuffer, uint32_t lineStippleFactor,
                                                     uint16_t lineStipplePattern, const ErrorObject& error_obj) const {
    return PreCallValidateCmdSetLineStipple(commandBuffer, lineStippleFactor, lineStipplePattern, error_obj);
}

bool CoreChecks::PreCallValidateCmdSetDepthBias(VkCommandBuffer commandBuffer, float depthBiasConstantFactor, float depthBiasClamp,
                                                float depthBiasSlopeFactor, const ErrorObject& error_obj) const {
    auto cb_state = GetRead<vvl::CommandBuffer>(commandBuffer);
    bool skip = false;
    skip |= ValidateCmd(*cb_state, error_obj.location);
    if ((depthBiasClamp != 0.0) && !enabled_features.depthBiasClamp) {
        skip |=
            LogError("VUID-vkCmdSetDepthBias-depthBiasClamp-00790", commandBuffer, error_obj.location.dot(Field::depthBiasClamp),
                     "is %f (not 0.0f), but the depthBiasClamp feature was not enabled.", depthBiasClamp);
    }
    return skip;
}

bool CoreChecks::ValidateDepthBiasRepresentationInfo(const Location& loc, const LogObjectList& objlist,
                                                     const VkDepthBiasRepresentationInfoEXT& depth_bias_representation) const {
    bool skip = false;

    if ((depth_bias_representation.depthBiasRepresentation ==
         VK_DEPTH_BIAS_REPRESENTATION_LEAST_REPRESENTABLE_VALUE_FORCE_UNORM_EXT) &&
        !enabled_features.leastRepresentableValueForceUnormRepresentation) {
        skip |= LogError("VUID-VkDepthBiasRepresentationInfoEXT-leastRepresentableValueForceUnormRepresentation-08947", objlist,
                         loc.pNext(Struct::VkDepthBiasRepresentationInfoEXT, Field::depthBiasRepresentation),
                         "is %s, but the leastRepresentableValueForceUnormRepresentation feature was not enabled.",
                         string_VkDepthBiasRepresentationEXT(depth_bias_representation.depthBiasRepresentation));
    }

    if ((depth_bias_representation.depthBiasRepresentation == VK_DEPTH_BIAS_REPRESENTATION_FLOAT_EXT) &&
        !enabled_features.floatRepresentation) {
        skip |= LogError("VUID-VkDepthBiasRepresentationInfoEXT-floatRepresentation-08948", objlist,
                         loc.pNext(Struct::VkDepthBiasRepresentationInfoEXT, Field::depthBiasRepresentation),
                         "is %s but the floatRepresentation feature was not enabled.",
                         string_VkDepthBiasRepresentationEXT(depth_bias_representation.depthBiasRepresentation));
    }

    if ((depth_bias_representation.depthBiasExact == VK_TRUE) && !enabled_features.depthBiasExact) {
        skip |= LogError("VUID-VkDepthBiasRepresentationInfoEXT-depthBiasExact-08949", objlist,
                         loc.pNext(Struct::VkDepthBiasRepresentationInfoEXT, Field::depthBiasExact),
                         "is VK_TRUE, but the depthBiasExact feature was not enabled.");
    }

    return skip;
}

bool CoreChecks::PreCallValidateCmdSetDepthBias2EXT(VkCommandBuffer commandBuffer, const VkDepthBiasInfoEXT* pDepthBiasInfo,
                                                    const ErrorObject& error_obj) const {
    bool skip = false;
    auto cb_state = GetRead<vvl::CommandBuffer>(commandBuffer);
    skip |= ValidateCmd(*cb_state, error_obj.location);

    if ((pDepthBiasInfo->depthBiasClamp != 0.0) && !enabled_features.depthBiasClamp) {
        skip |= LogError("VUID-VkDepthBiasInfoEXT-depthBiasClamp-08950", commandBuffer,
                         error_obj.location.dot(Field::pDepthBiasInfo).dot(Field::depthBiasClamp),
                         "is %f (not 0.0f), but the depthBiasClamp feature was not enabled.", pDepthBiasInfo->depthBiasClamp);
    }

    if (const auto* depth_bias_representation =
            vku::FindStructInPNextChain<VkDepthBiasRepresentationInfoEXT>(pDepthBiasInfo->pNext)) {
        skip |= ValidateDepthBiasRepresentationInfo(error_obj.location, error_obj.objlist, *depth_bias_representation);
    }

    return skip;
}

bool CoreChecks::PreCallValidateCmdSetBlendConstants(VkCommandBuffer commandBuffer, const float blendConstants[4],
                                                     const ErrorObject& error_obj) const {
    auto cb_state = GetRead<vvl::CommandBuffer>(commandBuffer);
    return ValidateCmd(*cb_state, error_obj.location);
}

bool CoreChecks::PreCallValidateCmdSetDepthBounds(VkCommandBuffer commandBuffer, float minDepthBounds, float maxDepthBounds,
                                                  const ErrorObject& error_obj) const {
    auto cb_state = GetRead<vvl::CommandBuffer>(commandBuffer);
    bool skip = false;
    skip |= ValidateCmd(*cb_state, error_obj.location);

    if (!IsExtEnabled(extensions.vk_ext_depth_range_unrestricted)) {
        if (!(minDepthBounds >= 0.0) || !(minDepthBounds <= 1.0)) {
            skip |= LogError(
                "VUID-vkCmdSetDepthBounds-minDepthBounds-00600", commandBuffer, error_obj.location.dot(Field::minDepthBounds),
                "is %f which is not within the [0.0, 1.0] range and VK_EXT_depth_range_unrestricted extension was not enabled.",
                minDepthBounds);
        }

        if (!(maxDepthBounds >= 0.0) || !(maxDepthBounds <= 1.0)) {
            skip |= LogError(
                "VUID-vkCmdSetDepthBounds-maxDepthBounds-00601", commandBuffer, error_obj.location.dot(Field::maxDepthBounds),
                "is %f which is not within the [0.0, 1.0] range and VK_EXT_depth_range_unrestricted extension was not enabled.",
                maxDepthBounds);
        }
    }
    if (minDepthBounds > maxDepthBounds) {
        skip |=
            LogError("VUID-vkCmdSetDepthBounds-minDepthBounds-10912", commandBuffer, error_obj.location.dot(Field::minDepthBounds),
                     "(%f) is greater than maxDepthBounds (%f).", minDepthBounds, maxDepthBounds);
    }
    return skip;
}

bool CoreChecks::PreCallValidateCmdSetStencilCompareMask(VkCommandBuffer commandBuffer, VkStencilFaceFlags faceMask,
                                                         uint32_t compareMask, const ErrorObject& error_obj) const {
    auto cb_state = GetRead<vvl::CommandBuffer>(commandBuffer);
    return ValidateCmd(*cb_state, error_obj.location);
}

bool CoreChecks::PreCallValidateCmdSetStencilWriteMask(VkCommandBuffer commandBuffer, VkStencilFaceFlags faceMask,
                                                       uint32_t writeMask, const ErrorObject& error_obj) const {
    auto cb_state = GetRead<vvl::CommandBuffer>(commandBuffer);
    return ValidateCmd(*cb_state, error_obj.location);
}

bool CoreChecks::PreCallValidateCmdSetStencilReference(VkCommandBuffer commandBuffer, VkStencilFaceFlags faceMask,
                                                       uint32_t reference, const ErrorObject& error_obj) const {
    auto cb_state = GetRead<vvl::CommandBuffer>(commandBuffer);
    return ValidateCmd(*cb_state, error_obj.location);
}

bool CoreChecks::PreCallValidateCmdSetDiscardRectangleEXT(VkCommandBuffer commandBuffer, uint32_t firstDiscardRectangle,
                                                          uint32_t discardRectangleCount, const VkRect2D* pDiscardRectangles,
                                                          const ErrorObject& error_obj) const {
    auto cb_state = GetRead<vvl::CommandBuffer>(commandBuffer);
    bool skip = false;
    // Minimal validation for command buffer state
    skip |= ValidateCmd(*cb_state, error_obj.location);
    skip |=
        ForbidInheritedViewportScissor(*cb_state, "VUID-vkCmdSetDiscardRectangleEXT-viewportScissor2D-04788", error_obj.location);
    for (uint32_t i = 0; i < discardRectangleCount; ++i) {
        if (pDiscardRectangles[i].offset.x < 0) {
            skip |= LogError("VUID-vkCmdSetDiscardRectangleEXT-x-00587", commandBuffer,
                             error_obj.location.dot(Field::pDiscardRectangles, i).dot(Field::offset).dot(Field::x),
                             "(%" PRId32 ") is negative.", pDiscardRectangles[i].offset.x);
        }
        if (pDiscardRectangles[i].offset.y < 0) {
            skip |= LogError("VUID-vkCmdSetDiscardRectangleEXT-x-00587", commandBuffer,
                             error_obj.location.dot(Field::pDiscardRectangles, i).dot(Field::offset).dot(Field::y),
                             "(%" PRId32 ") is negative.", pDiscardRectangles[i].offset.y);
        }
    }
    if (firstDiscardRectangle + discardRectangleCount > phys_dev_ext_props.discard_rectangle_props.maxDiscardRectangles) {
        skip |=
            LogError("VUID-vkCmdSetDiscardRectangleEXT-firstDiscardRectangle-00585", commandBuffer,
                     error_obj.location.dot(Field::firstDiscardRectangle),
                     "(%" PRIu32 ") + discardRectangleCount (%" PRIu32 ") is not less than maxDiscardRectangles (%" PRIu32 ").",
                     firstDiscardRectangle, discardRectangleCount, phys_dev_ext_props.discard_rectangle_props.maxDiscardRectangles);
    }
    return skip;
}

bool CoreChecks::PreCallValidateCmdSetDiscardRectangleEnableEXT(VkCommandBuffer commandBuffer, VkBool32 discardRectangleEnable,
                                                                const ErrorObject& error_obj) const {
    auto cb_state = GetRead<vvl::CommandBuffer>(commandBuffer);
    return ValidateCmd(*cb_state, error_obj.location);
}

bool CoreChecks::PreCallValidateCmdSetDiscardRectangleModeEXT(VkCommandBuffer commandBuffer,
                                                              VkDiscardRectangleModeEXT discardRectangleMode,
                                                              const ErrorObject& error_obj) const {
    auto cb_state = GetRead<vvl::CommandBuffer>(commandBuffer);
    return ValidateCmd(*cb_state, error_obj.location);
}

bool CoreChecks::PreCallValidateCmdSetSampleLocationsEXT(VkCommandBuffer commandBuffer,
                                                         const VkSampleLocationsInfoEXT* pSampleLocationsInfo,
                                                         const ErrorObject& error_obj) const {
    bool skip = false;
    auto cb_state = GetRead<vvl::CommandBuffer>(commandBuffer);
    // Minimal validation for command buffer state
    skip |= ValidateCmd(*cb_state, error_obj.location);
    skip |= ValidateSampleLocationsInfo(*pSampleLocationsInfo, error_obj.location.dot(Field::pSampleLocationsInfo));
    return skip;
}

bool CoreChecks::PreCallValidateCmdSetCheckpointNV(VkCommandBuffer commandBuffer, const void* pCheckpointMarker,
                                                   const ErrorObject& error_obj) const {
    auto cb_state = GetRead<vvl::CommandBuffer>(commandBuffer);
    return ValidateCmd(*cb_state, error_obj.location);
}

bool CoreChecks::PreCallValidateCmdSetLogicOpEXT(VkCommandBuffer commandBuffer, VkLogicOp logicOp,
                                                 const ErrorObject& error_obj) const {
    auto cb_state = GetRead<vvl::CommandBuffer>(commandBuffer);
    bool skip = false;
    if (!enabled_features.extendedDynamicState2LogicOp && !enabled_features.shaderObject) {
        skip |= LogError("VUID-vkCmdSetLogicOpEXT-None-09422", commandBuffer, error_obj.location,
                         "extendedDynamicState2LogicOp and shaderObject features were not enabled.");
    }
    skip |= ValidateCmd(*cb_state, error_obj.location);
    return skip;
}

bool CoreChecks::PreCallValidateCmdSetPatchControlPointsEXT(VkCommandBuffer commandBuffer, uint32_t patchControlPoints,
                                                            const ErrorObject& error_obj) const {
    auto cb_state = GetRead<vvl::CommandBuffer>(commandBuffer);
    bool skip = false;
    if (!enabled_features.extendedDynamicState2PatchControlPoints && !enabled_features.shaderObject) {
        skip |= LogError("VUID-vkCmdSetPatchControlPointsEXT-None-09422", commandBuffer, error_obj.location,
                         "extendedDynamicState2PatchControlPoints and shaderObject features were not enabled.");
    }
    skip |= ValidateCmd(*cb_state, error_obj.location);

    if (patchControlPoints > phys_dev_props.limits.maxTessellationPatchSize) {
        skip |= LogError("VUID-vkCmdSetPatchControlPointsEXT-patchControlPoints-04874", commandBuffer,
                         error_obj.location.dot(Field::patchControlPoints),
                         "(%" PRIu32
                         ") must be less than "
                         "maxTessellationPatchSize (%" PRIu32 ")",
                         patchControlPoints, phys_dev_props.limits.maxTessellationPatchSize);
    }
    return skip;
}

bool CoreChecks::PreCallValidateCmdSetRasterizerDiscardEnableEXT(VkCommandBuffer commandBuffer, VkBool32 rasterizerDiscardEnable,
                                                                 const ErrorObject& error_obj) const {
    bool skip = false;
    if (!enabled_features.extendedDynamicState2 && !enabled_features.shaderObject) {
        skip |= LogError("VUID-vkCmdSetRasterizerDiscardEnable-None-08970", commandBuffer, error_obj.location,
                         "extendedDynamicState2 and shaderObject features were not enabled.");
    }
    skip |= PreCallValidateCmdSetRasterizerDiscardEnable(commandBuffer, rasterizerDiscardEnable, error_obj);
    return skip;
}

bool CoreChecks::PreCallValidateCmdSetRasterizerDiscardEnable(VkCommandBuffer commandBuffer, VkBool32 rasterizerDiscardEnable,
                                                              const ErrorObject& error_obj) const {
    auto cb_state = GetRead<vvl::CommandBuffer>(commandBuffer);
    return ValidateCmd(*cb_state, error_obj.location);
}

bool CoreChecks::PreCallValidateCmdSetDepthBiasEnableEXT(VkCommandBuffer commandBuffer, VkBool32 depthBiasEnable,
                                                         const ErrorObject& error_obj) const {
    bool skip = false;
    if (!enabled_features.extendedDynamicState2 && !enabled_features.shaderObject) {
        skip |= LogError("VUID-vkCmdSetDepthBiasEnable-None-08970", commandBuffer, error_obj.location,
                         "extendedDynamicState2 and shaderObject features were not enabled.");
    }
    skip |= PreCallValidateCmdSetDepthBiasEnable(commandBuffer, depthBiasEnable, error_obj);
    return skip;
}

bool CoreChecks::PreCallValidateCmdSetDepthBiasEnable(VkCommandBuffer commandBuffer, VkBool32 depthBiasEnable,
                                                      const ErrorObject& error_obj) const {
    auto cb_state = GetRead<vvl::CommandBuffer>(commandBuffer);
    return ValidateCmd(*cb_state, error_obj.location);
}

bool CoreChecks::PreCallValidateCmdSetPrimitiveRestartEnableEXT(VkCommandBuffer commandBuffer, VkBool32 primitiveRestartEnable,
                                                                const ErrorObject& error_obj) const {
    bool skip = false;
    if (!enabled_features.extendedDynamicState2 && !enabled_features.shaderObject) {
        skip |= LogError("VUID-vkCmdSetPrimitiveRestartEnable-None-08970", commandBuffer, error_obj.location,
                         "extendedDynamicState2 and shaderObject features were not enabled.");
    }
    skip |= PreCallValidateCmdSetPrimitiveRestartEnable(commandBuffer, primitiveRestartEnable, error_obj);
    return skip;
}

bool CoreChecks::PreCallValidateCmdSetPrimitiveRestartEnable(VkCommandBuffer commandBuffer, VkBool32 primitiveRestartEnable,
                                                             const ErrorObject& error_obj) const {
    auto cb_state = GetRead<vvl::CommandBuffer>(commandBuffer);
    return ValidateCmd(*cb_state, error_obj.location);
}

bool CoreChecks::PreCallValidateCmdSetCullModeEXT(VkCommandBuffer commandBuffer, VkCullModeFlags cullMode,
                                                  const ErrorObject& error_obj) const {
    bool skip = false;
    if (!enabled_features.extendedDynamicState && !enabled_features.shaderObject) {
        skip |= LogError("VUID-vkCmdSetCullMode-None-08971", commandBuffer, error_obj.location,
                         "extendedDynamicState and shaderObject features were not enabled.");
    }
    skip |= PreCallValidateCmdSetCullMode(commandBuffer, cullMode, error_obj);
    return skip;
}

bool CoreChecks::PreCallValidateCmdSetCullMode(VkCommandBuffer commandBuffer, VkCullModeFlags cullMode,
                                               const ErrorObject& error_obj) const {
    auto cb_state = GetRead<vvl::CommandBuffer>(commandBuffer);
    return ValidateCmd(*cb_state, error_obj.location);
}

bool CoreChecks::PreCallValidateCmdSetFrontFaceEXT(VkCommandBuffer commandBuffer, VkFrontFace frontFace,
                                                   const ErrorObject& error_obj) const {
    bool skip = false;
    if (!enabled_features.extendedDynamicState && !enabled_features.shaderObject) {
        skip |= LogError("VUID-vkCmdSetFrontFace-None-08971", commandBuffer, error_obj.location,
                         "extendedDynamicState and shaderObject features were not enabled.");
    }
    skip |= PreCallValidateCmdSetFrontFace(commandBuffer, frontFace, error_obj);
    return skip;
}

bool CoreChecks::PreCallValidateCmdSetFrontFace(VkCommandBuffer commandBuffer, VkFrontFace frontFace,
                                                const ErrorObject& error_obj) const {
    auto cb_state = GetRead<vvl::CommandBuffer>(commandBuffer);
    return ValidateCmd(*cb_state, error_obj.location);
}

bool CoreChecks::PreCallValidateCmdSetPrimitiveTopologyEXT(VkCommandBuffer commandBuffer, VkPrimitiveTopology primitiveTopology,
                                                           const ErrorObject& error_obj) const {
    bool skip = false;
    if (!enabled_features.extendedDynamicState && !enabled_features.shaderObject) {
        skip |= LogError("VUID-vkCmdSetPrimitiveTopology-None-08971", commandBuffer, error_obj.location,
                         "extendedDynamicState and shaderObject features were not enabled.");
    }
    skip |= PreCallValidateCmdSetPrimitiveTopology(commandBuffer, primitiveTopology, error_obj);
    return skip;
}

bool CoreChecks::PreCallValidateCmdSetPrimitiveTopology(VkCommandBuffer commandBuffer, VkPrimitiveTopology primitiveTopology,
                                                        const ErrorObject& error_obj) const {
    auto cb_state = GetRead<vvl::CommandBuffer>(commandBuffer);
    return ValidateCmd(*cb_state, error_obj.location);
}

bool CoreChecks::PreCallValidateCmdSetViewportWithCountEXT(VkCommandBuffer commandBuffer, uint32_t viewportCount,
                                                           const VkViewport* pViewports, const ErrorObject& error_obj) const {
    bool skip = false;
    if (!enabled_features.extendedDynamicState && !enabled_features.shaderObject) {
        skip |= LogError("VUID-vkCmdSetViewportWithCount-None-08971", commandBuffer, error_obj.location,
                         "extendedDynamicState and shaderObject features were not enabled.");
    }
    skip |= PreCallValidateCmdSetViewportWithCount(commandBuffer, viewportCount, pViewports, error_obj);
    return skip;
}

bool CoreChecks::PreCallValidateCmdSetViewportWithCount(VkCommandBuffer commandBuffer, uint32_t viewportCount,
                                                        const VkViewport* pViewports, const ErrorObject& error_obj) const {
    auto cb_state = GetRead<vvl::CommandBuffer>(commandBuffer);
    bool skip = false;
    skip |= ValidateCmd(*cb_state, error_obj.location);
    skip |= ForbidInheritedViewportScissor(*cb_state, "VUID-vkCmdSetViewportWithCount-commandBuffer-04819", error_obj.location);
    return skip;
}

bool CoreChecks::PreCallValidateCmdSetScissorWithCountEXT(VkCommandBuffer commandBuffer, uint32_t scissorCount,
                                                          const VkRect2D* pScissors, const ErrorObject& error_obj) const {
    bool skip = false;
    if (!enabled_features.extendedDynamicState && !enabled_features.shaderObject) {
        skip |= LogError("VUID-vkCmdSetScissorWithCount-None-08971", commandBuffer, error_obj.location,
                         "extendedDynamicState and shaderObject features were not enabled.");
    }
    skip |= PreCallValidateCmdSetScissorWithCount(commandBuffer, scissorCount, pScissors, error_obj);
    return skip;
}

bool CoreChecks::PreCallValidateCmdSetScissorWithCount(VkCommandBuffer commandBuffer, uint32_t scissorCount,
                                                       const VkRect2D* pScissors, const ErrorObject& error_obj) const {
    auto cb_state = GetRead<vvl::CommandBuffer>(commandBuffer);
    bool skip = false;
    skip |= ValidateCmd(*cb_state, error_obj.location);
    skip |= ForbidInheritedViewportScissor(*cb_state, "VUID-vkCmdSetScissorWithCount-commandBuffer-04820", error_obj.location);
    return skip;
}

bool CoreChecks::PreCallValidateCmdSetDepthTestEnableEXT(VkCommandBuffer commandBuffer, VkBool32 depthTestEnable,
                                                         const ErrorObject& error_obj) const {
    bool skip = false;
    if (!enabled_features.extendedDynamicState && !enabled_features.shaderObject) {
        skip |= LogError("VUID-vkCmdSetDepthTestEnable-None-08971", commandBuffer, error_obj.location,
                         "extendedDynamicState and shaderObject features were not enabled.");
    }
    skip |= PreCallValidateCmdSetDepthTestEnable(commandBuffer, depthTestEnable, error_obj);
    return skip;
}

bool CoreChecks::PreCallValidateCmdSetDepthTestEnable(VkCommandBuffer commandBuffer, VkBool32 depthTestEnable,
                                                      const ErrorObject& error_obj) const {
    auto cb_state = GetRead<vvl::CommandBuffer>(commandBuffer);
    return ValidateCmd(*cb_state, error_obj.location);
}

bool CoreChecks::PreCallValidateCmdSetDepthWriteEnableEXT(VkCommandBuffer commandBuffer, VkBool32 depthWriteEnable,
                                                          const ErrorObject& error_obj) const {
    bool skip = false;
    if (!enabled_features.extendedDynamicState && !enabled_features.shaderObject) {
        skip |= LogError("VUID-vkCmdSetDepthWriteEnable-None-08971", commandBuffer, error_obj.location,
                         "extendedDynamicState and shaderObject features were not enabled.");
    }
    skip |= PreCallValidateCmdSetDepthWriteEnable(commandBuffer, depthWriteEnable, error_obj);
    return skip;
}

bool CoreChecks::PreCallValidateCmdSetDepthWriteEnable(VkCommandBuffer commandBuffer, VkBool32 depthWriteEnable,
                                                       const ErrorObject& error_obj) const {
    auto cb_state = GetRead<vvl::CommandBuffer>(commandBuffer);
    return ValidateCmd(*cb_state, error_obj.location);
}

bool CoreChecks::PreCallValidateCmdSetDepthCompareOpEXT(VkCommandBuffer commandBuffer, VkCompareOp depthCompareOp,
                                                        const ErrorObject& error_obj) const {
    bool skip = false;
    if (!enabled_features.extendedDynamicState && !enabled_features.shaderObject) {
        skip |= LogError("VUID-vkCmdSetDepthCompareOp-None-08971", commandBuffer, error_obj.location,
                         "extendedDynamicState and shaderObject features were not enabled.");
    }
    skip |= PreCallValidateCmdSetDepthCompareOp(commandBuffer, depthCompareOp, error_obj);
    return skip;
}

bool CoreChecks::PreCallValidateCmdSetDepthCompareOp(VkCommandBuffer commandBuffer, VkCompareOp depthCompareOp,
                                                     const ErrorObject& error_obj) const {
    auto cb_state = GetRead<vvl::CommandBuffer>(commandBuffer);
    return ValidateCmd(*cb_state, error_obj.location);
}

bool CoreChecks::PreCallValidateCmdSetDepthBoundsTestEnableEXT(VkCommandBuffer commandBuffer, VkBool32 depthBoundsTestEnable,
                                                               const ErrorObject& error_obj) const {
    bool skip = false;
    if (!enabled_features.extendedDynamicState && !enabled_features.shaderObject) {
        skip |= LogError("VUID-vkCmdSetDepthBoundsTestEnable-None-08971", commandBuffer, error_obj.location,
                         "extendedDynamicState and shaderObject features were not enabled.");
    }
    skip |= PreCallValidateCmdSetDepthBoundsTestEnable(commandBuffer, depthBoundsTestEnable, error_obj);
    return skip;
}

bool CoreChecks::PreCallValidateCmdSetDepthBoundsTestEnable(VkCommandBuffer commandBuffer, VkBool32 depthBoundsTestEnable,
                                                            const ErrorObject& error_obj) const {
    bool skip = false;
    auto cb_state = GetRead<vvl::CommandBuffer>(commandBuffer);
    skip |= ValidateCmd(*cb_state, error_obj.location);
    if (depthBoundsTestEnable == VK_TRUE && !enabled_features.depthBounds) {
        skip |= LogError("VUID-vkCmdSetDepthBoundsTestEnable-depthBounds-10010", commandBuffer,
                         error_obj.location.dot(Field::depthBoundsTestEnable),
                         "is VK_TRUE but the depthBounds feature was not enabled.");
    }
    return skip;
}

bool CoreChecks::PreCallValidateCmdSetStencilTestEnableEXT(VkCommandBuffer commandBuffer, VkBool32 stencilTestEnable,
                                                           const ErrorObject& error_obj) const {
    bool skip = false;
    if (!enabled_features.extendedDynamicState && !enabled_features.shaderObject) {
        skip |= LogError("VUID-vkCmdSetStencilTestEnable-None-08971", commandBuffer, error_obj.location,
                         "extendedDynamicState and shaderObject features were not enabled.");
    }
    skip |= PreCallValidateCmdSetStencilTestEnable(commandBuffer, stencilTestEnable, error_obj);
    return skip;
}

bool CoreChecks::PreCallValidateCmdSetStencilTestEnable(VkCommandBuffer commandBuffer, VkBool32 stencilTestEnable,
                                                        const ErrorObject& error_obj) const {
    auto cb_state = GetRead<vvl::CommandBuffer>(commandBuffer);
    return ValidateCmd(*cb_state, error_obj.location);
}

bool CoreChecks::PreCallValidateCmdSetStencilOpEXT(VkCommandBuffer commandBuffer, VkStencilFaceFlags faceMask, VkStencilOp failOp,
                                                   VkStencilOp passOp, VkStencilOp depthFailOp, VkCompareOp compareOp,
                                                   const ErrorObject& error_obj) const {
    bool skip = false;
    if (!enabled_features.extendedDynamicState && !enabled_features.shaderObject) {
        skip |= LogError("VUID-vkCmdSetStencilOp-None-08971", commandBuffer, error_obj.location,
                         "extendedDynamicState and shaderObject features were not enabled.");
    }
    skip |= PreCallValidateCmdSetStencilOp(commandBuffer, faceMask, failOp, passOp, depthFailOp, compareOp, error_obj);
    return skip;
}

bool CoreChecks::PreCallValidateCmdSetStencilOp(VkCommandBuffer commandBuffer, VkStencilFaceFlags faceMask, VkStencilOp failOp,
                                                VkStencilOp passOp, VkStencilOp depthFailOp, VkCompareOp compareOp,
                                                const ErrorObject& error_obj) const {
    auto cb_state = GetRead<vvl::CommandBuffer>(commandBuffer);
    return ValidateCmd(*cb_state, error_obj.location);
}

bool CoreChecks::PreCallValidateCmdSetTessellationDomainOriginEXT(VkCommandBuffer commandBuffer,
                                                                  VkTessellationDomainOrigin domainOrigin,
                                                                  const ErrorObject& error_obj) const {
    auto cb_state = GetRead<vvl::CommandBuffer>(commandBuffer);
    bool skip = false;
    if (!enabled_features.extendedDynamicState3TessellationDomainOrigin && !enabled_features.shaderObject) {
        skip |= LogError("VUID-vkCmdSetTessellationDomainOriginEXT-None-09423", commandBuffer, error_obj.location,
                         "extendedDynamicState3TessellationDomainOrigin and shaderObject features were not enabled.");
    }
    skip |= ValidateCmd(*cb_state, error_obj.location);
    return skip;
}

bool CoreChecks::PreCallValidateCmdSetDepthClampEnableEXT(VkCommandBuffer commandBuffer, VkBool32 depthClampEnable,
                                                          const ErrorObject& error_obj) const {
    auto cb_state = GetRead<vvl::CommandBuffer>(commandBuffer);
    bool skip = false;
    if (!enabled_features.extendedDynamicState3DepthClampEnable && !enabled_features.shaderObject) {
        skip |= LogError("VUID-vkCmdSetDepthClampEnableEXT-None-09423", commandBuffer, error_obj.location,
                         "extendedDynamicState3DepthClampEnable and shaderObject features were not enabled.");
    }
    skip |= ValidateCmd(*cb_state, error_obj.location);

    if (depthClampEnable != VK_FALSE && !enabled_features.depthClamp) {
        skip |= LogError("VUID-vkCmdSetDepthClampEnableEXT-depthClamp-07449", commandBuffer,
                         error_obj.location.dot(Field::depthClampEnable), "is VK_TRUE but the depthClamp feature was not enabled.");
    }
    return skip;
}

bool CoreChecks::PreCallValidateCmdSetDepthClampRangeEXT(VkCommandBuffer commandBuffer, VkDepthClampModeEXT depthClampMode,
                                                         const VkDepthClampRangeEXT* pDepthClampRange,
                                                         const ErrorObject& error_obj) const {
    auto cb_state = GetRead<vvl::CommandBuffer>(commandBuffer);
    return ValidateCmd(*cb_state, error_obj.location);
}

bool CoreChecks::PreCallValidateCmdSetPolygonModeEXT(VkCommandBuffer commandBuffer, VkPolygonMode polygonMode,
                                                     const ErrorObject& error_obj) const {
    auto cb_state = GetRead<vvl::CommandBuffer>(commandBuffer);
    bool skip = false;
    if (!enabled_features.extendedDynamicState3PolygonMode && !enabled_features.shaderObject) {
        skip |= LogError("VUID-vkCmdSetPolygonModeEXT-None-09423", commandBuffer, error_obj.location,
                         "extendedDynamicState3PolygonMode and shaderObject features were not enabled.");
    }
    skip |= ValidateCmd(*cb_state, error_obj.location);

    if ((polygonMode == VK_POLYGON_MODE_LINE || polygonMode == VK_POLYGON_MODE_POINT) && !enabled_features.fillModeNonSolid) {
        skip |= LogError("VUID-vkCmdSetPolygonModeEXT-fillModeNonSolid-07424", commandBuffer,
                         error_obj.location.dot(Field::polygonMode),
                         "is %s but the "
                         "fillModeNonSolid feature was not enabled.",
                         string_VkPolygonMode(polygonMode));
    } else if (polygonMode == VK_POLYGON_MODE_FILL_RECTANGLE_NV && !IsExtEnabled(extensions.vk_nv_fill_rectangle)) {
        skip |= LogError("VUID-vkCmdSetPolygonModeEXT-polygonMode-07425", commandBuffer, error_obj.location.dot(Field::polygonMode),
                         "is VK_POLYGON_MODE_FILL_RECTANGLE_NV but the VK_NV_fill_rectangle "
                         "extension was not enabled.");
    }
    return skip;
}

bool CoreChecks::PreCallValidateCmdSetRasterizationSamplesEXT(VkCommandBuffer commandBuffer,
                                                              VkSampleCountFlagBits rasterizationSamples,
                                                              const ErrorObject& error_obj) const {
    auto cb_state = GetRead<vvl::CommandBuffer>(commandBuffer);
    bool skip = false;
    if (!enabled_features.extendedDynamicState3RasterizationSamples && !enabled_features.shaderObject) {
        skip |= LogError("VUID-vkCmdSetRasterizationSamplesEXT-None-09423", commandBuffer, error_obj.location,
                         "extendedDynamicState3RasterizationSamples and shaderObject features were not enabled.");
    }
    skip |= ValidateCmd(*cb_state, error_obj.location);
    return skip;
}

bool CoreChecks::PreCallValidateCmdSetSampleMaskEXT(VkCommandBuffer commandBuffer, VkSampleCountFlagBits samples,
                                                    const VkSampleMask* pSampleMask, const ErrorObject& error_obj) const {
    auto cb_state = GetRead<vvl::CommandBuffer>(commandBuffer);
    bool skip = false;
    if (!enabled_features.extendedDynamicState3SampleMask && !enabled_features.shaderObject) {
        skip |= LogError("VUID-vkCmdSetSampleMaskEXT-None-09423", commandBuffer, error_obj.location,
                         "extendedDynamicState3SampleMask and shaderObject features were not enabled.");
    }
    if (!enabled_features.maintenance10 && !pSampleMask) {
        skip |= LogError("VUID-vkCmdSetSampleMaskEXT-pSampleMask-10999", commandBuffer, error_obj.location.dot(Field::pSampleMask),
                         "is NULL.");
    }
    skip |= ValidateCmd(*cb_state, error_obj.location);
    return skip;
}

bool CoreChecks::PreCallValidateCmdSetAlphaToCoverageEnableEXT(VkCommandBuffer commandBuffer, VkBool32 alphaToCoverageEnable,
                                                               const ErrorObject& error_obj) const {
    auto cb_state = GetRead<vvl::CommandBuffer>(commandBuffer);
    bool skip = false;
    if (!enabled_features.extendedDynamicState3AlphaToCoverageEnable && !enabled_features.shaderObject) {
        skip |= LogError("VUID-vkCmdSetAlphaToCoverageEnableEXT-None-09423", commandBuffer, error_obj.location,
                         "extendedDynamicState3AlphaToCoverageEnable and shaderObject features were not enabled.");
    }
    skip |= ValidateCmd(*cb_state, error_obj.location);
    return skip;
}

bool CoreChecks::PreCallValidateCmdSetAlphaToOneEnableEXT(VkCommandBuffer commandBuffer, VkBool32 alphaToOneEnable,
                                                          const ErrorObject& error_obj) const {
    auto cb_state = GetRead<vvl::CommandBuffer>(commandBuffer);
    bool skip = false;
    if (!enabled_features.extendedDynamicState3AlphaToOneEnable && !enabled_features.shaderObject) {
        skip |= LogError("VUID-vkCmdSetAlphaToOneEnableEXT-None-09423", commandBuffer, error_obj.location,
                         "extendedDynamicState3AlphaToOneEnable and shaderObject features were not enabled.");
    }
    skip |= ValidateCmd(*cb_state, error_obj.location);

    if (alphaToOneEnable != VK_FALSE && !enabled_features.alphaToOne) {
        skip |= LogError("VUID-vkCmdSetAlphaToOneEnableEXT-alphaToOne-07607", commandBuffer,
                         error_obj.location.dot(Field::alphaToOneEnable), "is VK_TRUE but the alphaToOne feature was not enabled.");
    }
    return skip;
}

bool CoreChecks::PreCallValidateCmdSetLogicOpEnableEXT(VkCommandBuffer commandBuffer, VkBool32 logicOpEnable,
                                                       const ErrorObject& error_obj) const {
    auto cb_state = GetRead<vvl::CommandBuffer>(commandBuffer);
    bool skip = false;
    if (!enabled_features.extendedDynamicState3LogicOpEnable && !enabled_features.shaderObject) {
        skip |= LogError("VUID-vkCmdSetLogicOpEnableEXT-None-09423", commandBuffer, error_obj.location,
                         "extendedDynamicState3LogicOpEnable and shaderObject features were not enabled.");
    }
    skip |= ValidateCmd(*cb_state, error_obj.location);

    if (logicOpEnable != VK_FALSE && !enabled_features.logicOp) {
        skip |= LogError("VUID-vkCmdSetLogicOpEnableEXT-logicOp-07366", commandBuffer, error_obj.location.dot(Field::logicOpEnable),
                         "is VK_TRUE but the logicOp feature was not enabled.");
    }
    return skip;
}

bool CoreChecks::PreCallValidateCmdSetColorBlendEnableEXT(VkCommandBuffer commandBuffer, uint32_t firstAttachment,
                                                          uint32_t attachmentCount, const VkBool32* pColorBlendEnables,
                                                          const ErrorObject& error_obj) const {
    auto cb_state = GetRead<vvl::CommandBuffer>(commandBuffer);
    bool skip = false;
    if (!enabled_features.extendedDynamicState3ColorBlendEnable && !enabled_features.shaderObject) {
        skip |= LogError("VUID-vkCmdSetColorBlendEnableEXT-None-09423", commandBuffer, error_obj.location,
                         "extendedDynamicState3ColorBlendEnable and shaderObject features were not enabled.");
    }
    skip |= ValidateCmd(*cb_state, error_obj.location);
    return skip;
}

bool CoreChecks::PreCallValidateCmdSetColorBlendEquationEXT(VkCommandBuffer commandBuffer, uint32_t firstAttachment,
                                                            uint32_t attachmentCount,
                                                            const VkColorBlendEquationEXT* pColorBlendEquations,
                                                            const ErrorObject& error_obj) const {
    auto cb_state = GetRead<vvl::CommandBuffer>(commandBuffer);
    bool skip = false;
    if (!enabled_features.extendedDynamicState3ColorBlendEquation && !enabled_features.shaderObject) {
        skip |= LogError("VUID-vkCmdSetColorBlendEquationEXT-None-09423", commandBuffer, error_obj.location,
                         "extendedDynamicState3ColorBlendEquation and shaderObject features were not enabled.");
    }
    skip |= ValidateCmd(*cb_state, error_obj.location);

    for (uint32_t attachment = 0U; attachment < attachmentCount; ++attachment) {
        const Location equation_loc = error_obj.location.dot(Field::pColorBlendEquations, attachment);
        VkColorBlendEquationEXT const& equation = pColorBlendEquations[attachment];
        if (!enabled_features.dualSrcBlend) {
            if (IsSecondaryColorInputBlendFactor(equation.srcColorBlendFactor)) {
                skip |= LogError(
                    "VUID-VkColorBlendEquationEXT-dualSrcBlend-07357", commandBuffer, equation_loc.dot(Field::srcColorBlendFactor),
                    "is %s but the dualSrcBlend feature was not enabled.", string_VkBlendFactor(equation.srcColorBlendFactor));
            }
            if (IsSecondaryColorInputBlendFactor(equation.dstColorBlendFactor)) {
                skip |= LogError(
                    "VUID-VkColorBlendEquationEXT-dualSrcBlend-07358", commandBuffer, equation_loc.dot(Field::dstColorBlendFactor),
                    "is %s but the dualSrcBlend feature was not enabled.", string_VkBlendFactor(equation.dstColorBlendFactor));
            }
            if (IsSecondaryColorInputBlendFactor(equation.srcAlphaBlendFactor)) {
                skip |= LogError(
                    "VUID-VkColorBlendEquationEXT-dualSrcBlend-07359", commandBuffer, equation_loc.dot(Field::srcAlphaBlendFactor),
                    "is %s but the dualSrcBlend feature was not enabled.", string_VkBlendFactor(equation.srcAlphaBlendFactor));
            }
            if (IsSecondaryColorInputBlendFactor(equation.dstAlphaBlendFactor)) {
                skip |= LogError(
                    "VUID-VkColorBlendEquationEXT-dualSrcBlend-07360", commandBuffer, equation_loc.dot(Field::dstAlphaBlendFactor),
                    "is %s but the dualSrcBlend feature was not enabled.", string_VkBlendFactor(equation.dstAlphaBlendFactor));
            }
        }
        if (IsAdvanceBlendOperation(equation.colorBlendOp) || IsAdvanceBlendOperation(equation.alphaBlendOp)) {
            skip |=
                LogError("VUID-VkColorBlendEquationEXT-colorBlendOp-07361", commandBuffer, equation_loc.dot(Field::colorBlendOp),
                         "(%s) and alphaBlendOp (%s) must not be an advanced blending operation.",
                         string_VkBlendOp(equation.colorBlendOp), string_VkBlendOp(equation.alphaBlendOp));
        }
        if (IsExtEnabled(extensions.vk_khr_portability_subset) && !enabled_features.constantAlphaColorBlendFactors) {
            if (equation.srcColorBlendFactor == VK_BLEND_FACTOR_CONSTANT_ALPHA ||
                equation.srcColorBlendFactor == VK_BLEND_FACTOR_ONE_MINUS_CONSTANT_ALPHA) {
                skip |= LogError("VUID-VkColorBlendEquationEXT-constantAlphaColorBlendFactors-07362", commandBuffer,
                                 equation_loc.dot(Field::srcColorBlendFactor),
                                 "is %s but the constantAlphaColorBlendFactors feature was not enabled.",
                                 string_VkBlendFactor(equation.srcColorBlendFactor));
            }
            if (equation.dstColorBlendFactor == VK_BLEND_FACTOR_CONSTANT_ALPHA ||
                equation.dstColorBlendFactor == VK_BLEND_FACTOR_ONE_MINUS_CONSTANT_ALPHA) {
                skip |= LogError("VUID-VkColorBlendEquationEXT-constantAlphaColorBlendFactors-07363", commandBuffer,
                                 equation_loc.dot(Field::dstColorBlendFactor),
                                 "is %s but the constantAlphaColorBlendFactors feature was not enabled.",
                                 string_VkBlendFactor(equation.dstColorBlendFactor));
            }
        }
    }
    return skip;
}

bool CoreChecks::PreCallValidateCmdSetColorWriteMaskEXT(VkCommandBuffer commandBuffer, uint32_t firstAttachment,
                                                        uint32_t attachmentCount, const VkColorComponentFlags* pColorWriteMasks,
                                                        const ErrorObject& error_obj) const {
    auto cb_state = GetRead<vvl::CommandBuffer>(commandBuffer);
    bool skip = false;
    if (!enabled_features.extendedDynamicState3ColorWriteMask && !enabled_features.shaderObject) {
        skip |= LogError("VUID-vkCmdSetColorWriteMaskEXT-None-09423", commandBuffer, error_obj.location,
                         "extendedDynamicState3ColorWriteMask and shaderObject features were not enabled.");
    }
    skip |= ValidateCmd(*cb_state, error_obj.location);
    return skip;
}

bool CoreChecks::PreCallValidateCmdSetRasterizationStreamEXT(VkCommandBuffer commandBuffer, uint32_t rasterizationStream,
                                                             const ErrorObject& error_obj) const {
    auto cb_state = GetRead<vvl::CommandBuffer>(commandBuffer);
    bool skip = false;
    if (!enabled_features.extendedDynamicState3RasterizationStream && !enabled_features.shaderObject) {
        skip |= LogError("VUID-vkCmdSetRasterizationStreamEXT-None-09423", commandBuffer, error_obj.location,
                         "extendedDynamicState3RasterizationStream and shaderObject features were not enabled.");
    }
    skip |= ValidateCmd(*cb_state, error_obj.location);

    if (!enabled_features.transformFeedback) {
        skip |= LogError("VUID-vkCmdSetRasterizationStreamEXT-transformFeedback-07411", commandBuffer, error_obj.location,
                         "the transformFeedback feature was not enabled.");
    }
    if (rasterizationStream >= phys_dev_ext_props.transform_feedback_props.maxTransformFeedbackStreams) {
        skip |= LogError("VUID-vkCmdSetRasterizationStreamEXT-rasterizationStream-07412", commandBuffer,
                         error_obj.location.dot(Field::rasterizationStream),
                         "(%" PRIu32 ") must be less than maxTransformFeedbackStreams (%" PRIu32 ").", rasterizationStream,
                         phys_dev_ext_props.transform_feedback_props.maxTransformFeedbackStreams);
    }
    if (rasterizationStream != 0U &&
        phys_dev_ext_props.transform_feedback_props.transformFeedbackRasterizationStreamSelect == VK_FALSE) {
        skip |= LogError("VUID-vkCmdSetRasterizationStreamEXT-rasterizationStream-07413", commandBuffer,
                         error_obj.location.dot(Field::rasterizationStream),
                         "(%" PRIu32
                         ") is non-zero but "
                         "the transformFeedbackRasterizationStreamSelect feature was not enabled.",
                         rasterizationStream);
    }
    return skip;
}

bool CoreChecks::PreCallValidateCmdSetConservativeRasterizationModeEXT(
    VkCommandBuffer commandBuffer, VkConservativeRasterizationModeEXT conservativeRasterizationMode,
    const ErrorObject& error_obj) const {
    auto cb_state = GetRead<vvl::CommandBuffer>(commandBuffer);
    bool skip = false;
    if (!enabled_features.extendedDynamicState3ConservativeRasterizationMode && !enabled_features.shaderObject) {
        skip |= LogError("VUID-vkCmdSetConservativeRasterizationModeEXT-None-09423", commandBuffer, error_obj.location,
                         "extendedDynamicState3ConservativeRasterizationMode and shaderObject features were not enabled.");
    }
    skip |= ValidateCmd(*cb_state, error_obj.location);
    return skip;
}

bool CoreChecks::PreCallValidateCmdSetExtraPrimitiveOverestimationSizeEXT(VkCommandBuffer commandBuffer,
                                                                          float extraPrimitiveOverestimationSize,
                                                                          const ErrorObject& error_obj) const {
    auto cb_state = GetRead<vvl::CommandBuffer>(commandBuffer);
    bool skip = false;
    if (!enabled_features.extendedDynamicState3ExtraPrimitiveOverestimationSize && !enabled_features.shaderObject) {
        skip |= LogError("VUID-vkCmdSetExtraPrimitiveOverestimationSizeEXT-None-09423", commandBuffer, error_obj.location,
                         "extendedDynamicState3ExtraPrimitiveOverestimationSize and shaderObject features were not enabled.");
    }
    skip |= ValidateCmd(*cb_state, error_obj.location);

    if (extraPrimitiveOverestimationSize < 0.0f ||
        extraPrimitiveOverestimationSize >
            phys_dev_ext_props.conservative_rasterization_props.maxExtraPrimitiveOverestimationSize) {
        skip |= LogError("VUID-vkCmdSetExtraPrimitiveOverestimationSizeEXT-extraPrimitiveOverestimationSize-07428", commandBuffer,
                         error_obj.location.dot(Field::extraPrimitiveOverestimationSize),
                         "(%f) must be less than zero or greater than maxExtraPrimitiveOverestimationSize (%f).",
                         extraPrimitiveOverestimationSize,
                         phys_dev_ext_props.conservative_rasterization_props.maxExtraPrimitiveOverestimationSize);
    }
    return skip;
}

bool CoreChecks::PreCallValidateCmdSetDepthClipEnableEXT(VkCommandBuffer commandBuffer, VkBool32 depthClipEnable,
                                                         const ErrorObject& error_obj) const {
    auto cb_state = GetRead<vvl::CommandBuffer>(commandBuffer);
    bool skip = false;
    if (!enabled_features.extendedDynamicState3DepthClipEnable && !enabled_features.shaderObject) {
        skip |= LogError("VUID-vkCmdSetDepthClipEnableEXT-None-09423", commandBuffer, error_obj.location,
                         "extendedDynamicState3DepthClipEnable and shaderObject features were not enabled.");
    }
    skip |= ValidateCmd(*cb_state, error_obj.location);

    if (!enabled_features.depthClipEnable) {
        skip |= LogError("VUID-vkCmdSetDepthClipEnableEXT-depthClipEnable-07451", commandBuffer, error_obj.location,
                         "the depthClipEnable feature was not enabled.");
    }
    return skip;
}

bool CoreChecks::PreCallValidateCmdSetSampleLocationsEnableEXT(VkCommandBuffer commandBuffer, VkBool32 sampleLocationsEnable,
                                                               const ErrorObject& error_obj) const {
    auto cb_state = GetRead<vvl::CommandBuffer>(commandBuffer);
    bool skip = false;
    if (!enabled_features.extendedDynamicState3SampleLocationsEnable && !enabled_features.shaderObject) {
        skip |= LogError("VUID-vkCmdSetSampleLocationsEnableEXT-None-09423", commandBuffer, error_obj.location,
                         "extendedDynamicState3SampleLocationsEnable and shaderObject features were not enabled.");
    }
    skip |= ValidateCmd(*cb_state, error_obj.location);
    return skip;
}

bool CoreChecks::PreCallValidateCmdSetColorBlendAdvancedEXT(VkCommandBuffer commandBuffer, uint32_t firstAttachment,
                                                            uint32_t attachmentCount,
                                                            const VkColorBlendAdvancedEXT* pColorBlendAdvanced,
                                                            const ErrorObject& error_obj) const {
    auto cb_state = GetRead<vvl::CommandBuffer>(commandBuffer);
    bool skip = false;
    if (!enabled_features.extendedDynamicState3ColorBlendAdvanced && !enabled_features.shaderObject) {
        skip |= LogError("VUID-vkCmdSetColorBlendAdvancedEXT-None-09423", commandBuffer, error_obj.location,
                         "extendedDynamicState3ColorBlendAdvanced and shaderObject features were not enabled.");
    }
    skip |= ValidateCmd(*cb_state, error_obj.location);

    for (uint32_t attachment = 0U; attachment < attachmentCount; ++attachment) {
        VkColorBlendAdvancedEXT const& advanced = pColorBlendAdvanced[attachment];
        if (advanced.srcPremultiplied == VK_TRUE &&
            !phys_dev_ext_props.blend_operation_advanced_props.advancedBlendNonPremultipliedSrcColor) {
            skip |= LogError("VUID-VkColorBlendAdvancedEXT-srcPremultiplied-07505", commandBuffer,
                             error_obj.location.dot(Field::pColorBlendAdvanced, attachment).dot(Field::srcPremultiplied),
                             "is VK_TRUE but the advancedBlendNonPremultipliedSrcColor feature was not enabled.");
        }
        if (advanced.dstPremultiplied == VK_TRUE &&
            !phys_dev_ext_props.blend_operation_advanced_props.advancedBlendNonPremultipliedDstColor) {
            skip |= LogError("VUID-VkColorBlendAdvancedEXT-dstPremultiplied-07506", commandBuffer,
                             error_obj.location.dot(Field::pColorBlendAdvanced, attachment).dot(Field::dstPremultiplied),
                             "is VK_TRUE but the advancedBlendNonPremultipliedDstColor feature was not enabled.");
        }
        if (advanced.blendOverlap != VK_BLEND_OVERLAP_UNCORRELATED_EXT &&
            !phys_dev_ext_props.blend_operation_advanced_props.advancedBlendCorrelatedOverlap) {
            skip |= LogError("VUID-VkColorBlendAdvancedEXT-blendOverlap-07507", commandBuffer,
                             error_obj.location.dot(Field::pColorBlendAdvanced, attachment).dot(Field::blendOverlap),
                             "is %s, but the advancedBlendCorrelatedOverlap feature was not enabled.",
                             string_VkBlendOverlapEXT(advanced.blendOverlap));
        }
    }
    return skip;
}

bool CoreChecks::PreCallValidateCmdSetProvokingVertexModeEXT(VkCommandBuffer commandBuffer,
                                                             VkProvokingVertexModeEXT provokingVertexMode,
                                                             const ErrorObject& error_obj) const {
    auto cb_state = GetRead<vvl::CommandBuffer>(commandBuffer);
    bool skip = false;
    if (!enabled_features.extendedDynamicState3ProvokingVertexMode && !enabled_features.shaderObject) {
        skip |= LogError("VUID-vkCmdSetProvokingVertexModeEXT-None-09423", commandBuffer, error_obj.location,
                         "extendedDynamicState3ProvokingVertexMode and shaderObject features were not enabled.");
    }
    skip |= ValidateCmd(*cb_state, error_obj.location);

    if (provokingVertexMode == VK_PROVOKING_VERTEX_MODE_LAST_VERTEX_EXT && enabled_features.provokingVertexLast == VK_FALSE) {
        skip |= LogError("VUID-vkCmdSetProvokingVertexModeEXT-provokingVertexMode-07447", commandBuffer,
                         error_obj.location.dot(Field::provokingVertexMode),
                         "is VK_PROVOKING_VERTEX_MODE_LAST_VERTEX_EXT but the provokingVertexLast feature was not enabled.");
    }
    return skip;
}

bool CoreChecks::PreCallValidateCmdSetLineRasterizationModeEXT(VkCommandBuffer commandBuffer,
                                                               VkLineRasterizationModeEXT lineRasterizationMode,
                                                               const ErrorObject& error_obj) const {
    auto cb_state = GetRead<vvl::CommandBuffer>(commandBuffer);
    bool skip = false;
    if (!enabled_features.extendedDynamicState3LineRasterizationMode && !enabled_features.shaderObject) {
        skip |= LogError("VUID-vkCmdSetLineRasterizationModeEXT-None-09423", commandBuffer, error_obj.location,
                         "extendedDynamicState3LineRasterizationMode and shaderObject features were not enabled.");
    }
    skip |= ValidateCmd(*cb_state, error_obj.location);

    if (lineRasterizationMode == VK_LINE_RASTERIZATION_MODE_RECTANGULAR && !enabled_features.rectangularLines) {
        skip |= LogError("VUID-vkCmdSetLineRasterizationModeEXT-lineRasterizationMode-07418", commandBuffer,
                         error_obj.location.dot(Field::lineRasterizationMode),
                         "is VK_LINE_RASTERIZATION_MODE_RECTANGULAR "
                         "but the rectangularLines feature was not enabled.");
    } else if (lineRasterizationMode == VK_LINE_RASTERIZATION_MODE_BRESENHAM && !enabled_features.bresenhamLines) {
        skip |= LogError("VUID-vkCmdSetLineRasterizationModeEXT-lineRasterizationMode-07419", commandBuffer,
                         error_obj.location.dot(Field::lineRasterizationMode),
                         "is VK_LINE_RASTERIZATION_MODE_BRESENHAM "
                         "but the bresenhamLines feature was not enabled.");
    } else if (lineRasterizationMode == VK_LINE_RASTERIZATION_MODE_RECTANGULAR_SMOOTH && !enabled_features.smoothLines) {
        skip |= LogError("VUID-vkCmdSetLineRasterizationModeEXT-lineRasterizationMode-07420", commandBuffer,
                         error_obj.location.dot(Field::lineRasterizationMode),
                         "is "
                         "VK_LINE_RASTERIZATION_MODE_RECTANGULAR_SMOOTH but the smoothLines feature was not enabled.");
    }
    return skip;
}

bool CoreChecks::PreCallValidateCmdSetLineStippleEnableEXT(VkCommandBuffer commandBuffer, VkBool32 stippledLineEnable,
                                                           const ErrorObject& error_obj) const {
    auto cb_state = GetRead<vvl::CommandBuffer>(commandBuffer);
    bool skip = false;
    if (!enabled_features.extendedDynamicState3LineStippleEnable && !enabled_features.shaderObject) {
        skip |= LogError("VUID-vkCmdSetLineStippleEnableEXT-None-09423", commandBuffer, error_obj.location,
                         "extendedDynamicState3LineStippleEnable and shaderObject features were not enabled.");
    }
    skip |= ValidateCmd(*cb_state, error_obj.location);
    return skip;
}

bool CoreChecks::PreCallValidateCmdSetDepthClipNegativeOneToOneEXT(VkCommandBuffer commandBuffer, VkBool32 negativeOneToOne,
                                                                   const ErrorObject& error_obj) const {
    auto cb_state = GetRead<vvl::CommandBuffer>(commandBuffer);
    bool skip = false;
    if (!enabled_features.extendedDynamicState3DepthClipNegativeOneToOne && !enabled_features.shaderObject) {
        skip |= LogError("VUID-vkCmdSetDepthClipNegativeOneToOneEXT-None-09423", commandBuffer, error_obj.location,
                         "extendedDynamicState3DepthClipNegativeOneToOne and shaderObject features were not enabled.");
    }
    skip |= ValidateCmd(*cb_state, error_obj.location);

    if (enabled_features.depthClipControl == VK_FALSE) {
        skip |= LogError("VUID-vkCmdSetDepthClipNegativeOneToOneEXT-depthClipControl-07453", commandBuffer, error_obj.location,
                         "the depthClipControl feature was not enabled.");
    }
    return skip;
}

bool CoreChecks::PreCallValidateCmdSetViewportWScalingEnableNV(VkCommandBuffer commandBuffer, VkBool32 viewportWScalingEnable,
                                                               const ErrorObject& error_obj) const {
    auto cb_state = GetRead<vvl::CommandBuffer>(commandBuffer);
    bool skip = false;
    if (!enabled_features.extendedDynamicState3ViewportWScalingEnable && !enabled_features.shaderObject) {
        skip |= LogError("VUID-vkCmdSetViewportWScalingEnableNV-None-09423", commandBuffer, error_obj.location,
                         "extendedDynamicState3ViewportWScalingEnable and shaderObject features were not enabled.");
    }
    skip |= ValidateCmd(*cb_state, error_obj.location);
    return skip;
}

bool CoreChecks::PreCallValidateCmdSetViewportSwizzleNV(VkCommandBuffer commandBuffer, uint32_t firstViewport,
                                                        uint32_t viewportCount, const VkViewportSwizzleNV* pViewportSwizzles,
                                                        const ErrorObject& error_obj) const {
    auto cb_state = GetRead<vvl::CommandBuffer>(commandBuffer);
    bool skip = false;
    if (!enabled_features.extendedDynamicState3ViewportSwizzle && !enabled_features.shaderObject) {
        skip |= LogError("VUID-vkCmdSetViewportSwizzleNV-None-09423", commandBuffer, error_obj.location,
                         "extendedDynamicState3ViewportSwizzle and shaderObject features were not enabled.");
    }
    skip |= ValidateCmd(*cb_state, error_obj.location);
    return skip;
}

bool CoreChecks::PreCallValidateCmdSetCoverageToColorEnableNV(VkCommandBuffer commandBuffer, VkBool32 coverageToColorEnable,
                                                              const ErrorObject& error_obj) const {
    auto cb_state = GetRead<vvl::CommandBuffer>(commandBuffer);
    bool skip = false;
    if (!enabled_features.extendedDynamicState3CoverageToColorEnable && !enabled_features.shaderObject) {
        skip |= LogError("VUID-vkCmdSetCoverageToColorEnableNV-None-09423", commandBuffer, error_obj.location,
                         "extendedDynamicState3CoverageToColorEnable and shaderObject features were not enabled.");
    }
    skip |= ValidateCmd(*cb_state, error_obj.location);
    return skip;
}

bool CoreChecks::PreCallValidateCmdSetCoverageToColorLocationNV(VkCommandBuffer commandBuffer, uint32_t coverageToColorLocation,
                                                                const ErrorObject& error_obj) const {
    auto cb_state = GetRead<vvl::CommandBuffer>(commandBuffer);
    bool skip = false;
    if (!enabled_features.extendedDynamicState3CoverageToColorLocation && !enabled_features.shaderObject) {
        skip |= LogError("VUID-vkCmdSetCoverageToColorLocationNV-None-09423", commandBuffer, error_obj.location,
                         "extendedDynamicState3CoverageToColorLocation and shaderObject features were not enabled.");
    }
    skip |= ValidateCmd(*cb_state, error_obj.location);
    return skip;
}

bool CoreChecks::PreCallValidateCmdSetCoverageModulationModeNV(VkCommandBuffer commandBuffer,
                                                               VkCoverageModulationModeNV coverageModulationMode,
                                                               const ErrorObject& error_obj) const {
    auto cb_state = GetRead<vvl::CommandBuffer>(commandBuffer);
    bool skip = false;
    if (!enabled_features.extendedDynamicState3CoverageModulationMode && !enabled_features.shaderObject) {
        skip |= LogError("VUID-vkCmdSetCoverageModulationModeNV-None-09423", commandBuffer, error_obj.location,
                         "extendedDynamicState3CoverageModulationMode and shaderObject features were not enabled.");
    }
    skip |= ValidateCmd(*cb_state, error_obj.location);
    return skip;
}

bool CoreChecks::PreCallValidateCmdSetCoverageModulationTableEnableNV(VkCommandBuffer commandBuffer,
                                                                      VkBool32 coverageModulationTableEnable,
                                                                      const ErrorObject& error_obj) const {
    auto cb_state = GetRead<vvl::CommandBuffer>(commandBuffer);
    bool skip = false;
    if (!enabled_features.extendedDynamicState3CoverageModulationTableEnable && !enabled_features.shaderObject) {
        skip |= LogError("VUID-vkCmdSetCoverageModulationTableEnableNV-None-09423", commandBuffer, error_obj.location,
                         "extendedDynamicState3CoverageModulationTableEnable and shaderObject features were not enabled.");
    }
    skip |= ValidateCmd(*cb_state, error_obj.location);
    return skip;
}

bool CoreChecks::PreCallValidateCmdSetCoverageModulationTableNV(VkCommandBuffer commandBuffer,
                                                                uint32_t coverageModulationTableCount,
                                                                const float* pCoverageModulationTable,
                                                                const ErrorObject& error_obj) const {
    auto cb_state = GetRead<vvl::CommandBuffer>(commandBuffer);
    bool skip = false;
    if (!enabled_features.extendedDynamicState3CoverageModulationTable && !enabled_features.shaderObject) {
        skip |= LogError("VUID-vkCmdSetCoverageModulationTableNV-None-09423", commandBuffer, error_obj.location,
                         "extendedDynamicState3CoverageModulationTable and shaderObject features were not enabled.");
    }
    skip |= ValidateCmd(*cb_state, error_obj.location);
    return skip;
}

bool CoreChecks::PreCallValidateCmdSetShadingRateImageEnableNV(VkCommandBuffer commandBuffer, VkBool32 shadingRateImageEnable,
                                                               const ErrorObject& error_obj) const {
    auto cb_state = GetRead<vvl::CommandBuffer>(commandBuffer);
    bool skip = false;
    if (!enabled_features.extendedDynamicState3ShadingRateImageEnable && !enabled_features.shaderObject) {
        skip |= LogError("VUID-vkCmdSetShadingRateImageEnableNV-None-09423", commandBuffer, error_obj.location,
                         "extendedDynamicState3ShadingRateImageEnable and shaderObject features were not enabled.");
    }
    skip |= ValidateCmd(*cb_state, error_obj.location);
    return skip;
}

bool CoreChecks::PreCallValidateCmdSetRepresentativeFragmentTestEnableNV(VkCommandBuffer commandBuffer,
                                                                         VkBool32 representativeFragmentTestEnable,
                                                                         const ErrorObject& error_obj) const {
    auto cb_state = GetRead<vvl::CommandBuffer>(commandBuffer);
    bool skip = false;
    if (!enabled_features.extendedDynamicState3RepresentativeFragmentTestEnable && !enabled_features.shaderObject) {
        skip |= LogError("VUID-vkCmdSetRepresentativeFragmentTestEnableNV-None-09423", commandBuffer, error_obj.location,
                         "extendedDynamicState3RepresentativeFragmentTestEnable and shaderObject features were not enabled.");
    }
    skip |= ValidateCmd(*cb_state, error_obj.location);
    return skip;
}

bool CoreChecks::PreCallValidateCmdSetCoverageReductionModeNV(VkCommandBuffer commandBuffer,
                                                              VkCoverageReductionModeNV coverageReductionMode,
                                                              const ErrorObject& error_obj) const {
    auto cb_state = GetRead<vvl::CommandBuffer>(commandBuffer);
    bool skip = false;
    if (!enabled_features.extendedDynamicState3CoverageReductionMode && !enabled_features.shaderObject) {
        skip |= LogError("VUID-vkCmdSetCoverageReductionModeNV-None-09423", commandBuffer, error_obj.location,
                         "extendedDynamicState3CoverageReductionMode and shaderObject features were not enabled.");
    }
    skip |= ValidateCmd(*cb_state, error_obj.location);
    return skip;
}

bool CoreChecks::PreCallValidateCreateEvent(VkDevice device, const VkEventCreateInfo* pCreateInfo,
                                            const VkAllocationCallbacks* pAllocator, VkEvent* pEvent,
                                            const ErrorObject& error_obj) const {
    bool skip = false;
    skip |= ValidateDeviceQueueSupport(error_obj.location);
    if (IsExtEnabled(extensions.vk_khr_portability_subset)) {
        if (VK_FALSE == enabled_features.events) {
            skip |= LogError("VUID-vkCreateEvent-events-04468", device, error_obj.location,
                             "events are not supported via VK_KHR_portability_subset");
        }
    }
    return skip;
}

bool CoreChecks::PreCallValidateCmdSetFragmentShadingRateKHR(VkCommandBuffer commandBuffer, const VkExtent2D* pFragmentSize,
                                                             const VkFragmentShadingRateCombinerOpKHR combinerOps[2],
                                                             const ErrorObject& error_obj) const {
    auto cb_state = GetRead<vvl::CommandBuffer>(commandBuffer);
    bool skip = false;
    if (!enabled_features.pipelineFragmentShadingRate && !enabled_features.primitiveFragmentShadingRate &&
        !enabled_features.attachmentFragmentShadingRate) {
        skip |= LogError("VUID-vkCmdSetFragmentShadingRateKHR-pipelineFragmentShadingRate-04509", commandBuffer, error_obj.location,
                         "pipelineFragmentShadingRate, primitiveFragmentShadingRate, and attachmentFragmentShadingRate features "
                         "were not enabled.");
    }
    skip |= ValidateCmd(*cb_state, error_obj.location);

    if (!enabled_features.pipelineFragmentShadingRate && pFragmentSize->width != 1) {
        skip |= LogError("VUID-vkCmdSetFragmentShadingRateKHR-pipelineFragmentShadingRate-04507", commandBuffer,
                         error_obj.location.dot(Field::pFragmentSize).dot(Field::width),
                         "is %" PRIu32 " but the pipelineFragmentShadingRate feature was not enabled.", pFragmentSize->width);
    }

    if (!enabled_features.pipelineFragmentShadingRate && pFragmentSize->height != 1) {
        skip |= LogError("VUID-vkCmdSetFragmentShadingRateKHR-pipelineFragmentShadingRate-04508", commandBuffer,
                         error_obj.location.dot(Field::pFragmentSize).dot(Field::height),
                         "is %" PRIu32 " but the pipelineFragmentShadingRate feature was not enabled.", pFragmentSize->height);
    }

    if (!enabled_features.primitiveFragmentShadingRate && combinerOps[0] != VK_FRAGMENT_SHADING_RATE_COMBINER_OP_KEEP_KHR) {
        skip |=
            LogError("VUID-vkCmdSetFragmentShadingRateKHR-primitiveFragmentShadingRate-04510", commandBuffer,
                     error_obj.location.dot(Field::combinerOps, 0), "is %s but the primitiveFragmentShadingRate was not enabled.",
                     string_VkFragmentShadingRateCombinerOpKHR(combinerOps[0]));
    }

    if (!enabled_features.attachmentFragmentShadingRate && combinerOps[1] != VK_FRAGMENT_SHADING_RATE_COMBINER_OP_KEEP_KHR) {
        skip |=
            LogError("VUID-vkCmdSetFragmentShadingRateKHR-attachmentFragmentShadingRate-04511", commandBuffer,
                     error_obj.location.dot(Field::combinerOps, 1), "is %s but the attachmentFragmentShadingRate was not enabled.",
                     string_VkFragmentShadingRateCombinerOpKHR(combinerOps[1]));
    }

    if (!phys_dev_ext_props.fragment_shading_rate_props.fragmentShadingRateNonTrivialCombinerOps &&
        (combinerOps[0] != VK_FRAGMENT_SHADING_RATE_COMBINER_OP_KEEP_KHR &&
         combinerOps[0] != VK_FRAGMENT_SHADING_RATE_COMBINER_OP_REPLACE_KHR)) {
        skip |= LogError("VUID-vkCmdSetFragmentShadingRateKHR-fragmentSizeNonTrivialCombinerOps-04512", commandBuffer,
                         error_obj.location.dot(Field::combinerOps, 0),
                         "is %s but the fragmentShadingRateNonTrivialCombinerOps feature was not enabled.",
                         string_VkFragmentShadingRateCombinerOpKHR(combinerOps[0]));
    }

    if (!phys_dev_ext_props.fragment_shading_rate_props.fragmentShadingRateNonTrivialCombinerOps &&
        (combinerOps[1] != VK_FRAGMENT_SHADING_RATE_COMBINER_OP_KEEP_KHR &&
         combinerOps[1] != VK_FRAGMENT_SHADING_RATE_COMBINER_OP_REPLACE_KHR)) {
        skip |= LogError("VUID-vkCmdSetFragmentShadingRateKHR-fragmentSizeNonTrivialCombinerOps-04512", commandBuffer,
                         error_obj.location.dot(Field::combinerOps, 1),
                         "is %s but the fragmentShadingRateNonTrivialCombinerOps feature was not enabled.",
                         string_VkFragmentShadingRateCombinerOpKHR(combinerOps[1]));
    }

    if (pFragmentSize->width == 0) {
        skip |= LogError("VUID-vkCmdSetFragmentShadingRateKHR-pFragmentSize-04513", commandBuffer,
                         error_obj.location.dot(Field::pFragmentSize).dot(Field::width), "is zero");
    }

    if (pFragmentSize->height == 0) {
        skip |= LogError("VUID-vkCmdSetFragmentShadingRateKHR-pFragmentSize-04514", commandBuffer,
                         error_obj.location.dot(Field::pFragmentSize).dot(Field::height), "is zero");
    }

    if (pFragmentSize->width != 0 && !IsPowerOfTwo(pFragmentSize->width)) {
        skip |= LogError("VUID-vkCmdSetFragmentShadingRateKHR-pFragmentSize-04515", commandBuffer,
                         error_obj.location.dot(Field::pFragmentSize).dot(Field::width), "(%" PRIu32 ") is a non-power-of-two.",
                         pFragmentSize->width);
    }

    if (pFragmentSize->height != 0 && !IsPowerOfTwo(pFragmentSize->height)) {
        skip |= LogError("VUID-vkCmdSetFragmentShadingRateKHR-pFragmentSize-04516", commandBuffer,
                         error_obj.location.dot(Field::pFragmentSize).dot(Field::height), "(%" PRIu32 ") is a non-power-of-two.",
                         pFragmentSize->height);
    }

    if (pFragmentSize->width > 4) {
        skip |= LogError("VUID-vkCmdSetFragmentShadingRateKHR-pFragmentSize-04517", commandBuffer,
                         error_obj.location.dot(Field::pFragmentSize).dot(Field::width), "(%" PRIu32 ") is larger than 4.",
                         pFragmentSize->width);
    }

    if (pFragmentSize->height > 4) {
        skip |= LogError("VUID-vkCmdSetFragmentShadingRateKHR-pFragmentSize-04518", commandBuffer,
                         error_obj.location.dot(Field::pFragmentSize).dot(Field::height), "(%" PRIu32 ") is larger than 4.",
                         pFragmentSize->height);
    }
    return skip;
}

bool CoreChecks::PreCallValidateCmdSetColorWriteEnableEXT(VkCommandBuffer commandBuffer, uint32_t attachmentCount,
                                                          const VkBool32* pColorWriteEnables, const ErrorObject& error_obj) const {
    bool skip = false;

    auto cb_state = GetRead<vvl::CommandBuffer>(commandBuffer);
    if (!enabled_features.colorWriteEnable) {
        skip |= LogError("VUID-vkCmdSetColorWriteEnableEXT-None-04803", commandBuffer, error_obj.location,
                         "colorWriteEnable feature was not enabled.");
    }
    skip |= ValidateCmd(*cb_state, error_obj.location);

    if (attachmentCount > phys_dev_props.limits.maxColorAttachments) {
        skip |= LogError("VUID-vkCmdSetColorWriteEnableEXT-attachmentCount-06656", commandBuffer,
                         error_obj.location.dot(Field::attachmentCount),
                         "(%" PRIu32 ") is greater than the maxColorAttachments limit (%" PRIu32 ").", attachmentCount,
                         phys_dev_props.limits.maxColorAttachments);
    }
    return skip;
}

bool CoreChecks::PreCallValidateCmdSetVertexInputEXT(VkCommandBuffer commandBuffer, uint32_t vertexBindingDescriptionCount,
                                                     const VkVertexInputBindingDescription2EXT* pVertexBindingDescriptions,
                                                     uint32_t vertexAttributeDescriptionCount,
                                                     const VkVertexInputAttributeDescription2EXT* pVertexAttributeDescriptions,
                                                     const ErrorObject& error_obj) const {
    auto cb_state = GetRead<vvl::CommandBuffer>(commandBuffer);
    bool skip = false;
    if (!enabled_features.vertexInputDynamicState && !enabled_features.shaderObject) {
        skip |= LogError("VUID-vkCmdSetVertexInputEXT-None-08546", commandBuffer, error_obj.location,
                         "vertexInputDynamicState and shaderObject features were not enabled.");
    }
    skip |= ValidateCmd(*cb_state, error_obj.location);
    return skip;
}

bool CoreChecks::PreCallValidateCmdSetCoarseSampleOrderNV(VkCommandBuffer commandBuffer, VkCoarseSampleOrderTypeNV sampleOrderType,
                                                          uint32_t customSampleOrderCount,
                                                          const VkCoarseSampleOrderCustomNV* pCustomSampleOrders,
                                                          const ErrorObject& error_obj) const {
    auto cb_state = GetRead<vvl::CommandBuffer>(commandBuffer);
    return ValidateCmd(*cb_state, error_obj.location);
}

bool CoreChecks::PreCallValidateCmdSetFragmentShadingRateEnumNV(VkCommandBuffer commandBuffer, VkFragmentShadingRateNV shadingRate,
                                                                const VkFragmentShadingRateCombinerOpKHR combinerOps[2],
                                                                const ErrorObject& error_obj) const {
    auto cb_state = GetRead<vvl::CommandBuffer>(commandBuffer);
    bool skip = false;
    if (!enabled_features.fragmentShadingRateEnums) {
        skip |= LogError("VUID-vkCmdSetFragmentShadingRateEnumNV-fragmentShadingRateEnums-04579", commandBuffer, error_obj.location,
                         "fragmentShadingRateEnums feature was not enabled.");
    }
    skip |= ValidateCmd(*cb_state, error_obj.location);
    return skip;
}

bool CoreChecks::PreCallValidateCmdSetPerformanceMarkerINTEL(VkCommandBuffer commandBuffer,
                                                             const VkPerformanceMarkerInfoINTEL* pMarkerInfo,
                                                             const ErrorObject& error_obj) const {
    auto cb_state = GetRead<vvl::CommandBuffer>(commandBuffer);
    return ValidateCmd(*cb_state, error_obj.location);
}

bool CoreChecks::PreCallValidateCmdSetPerformanceStreamMarkerINTEL(VkCommandBuffer commandBuffer,
                                                                   const VkPerformanceStreamMarkerInfoINTEL* pMarkerInfo,
                                                                   const ErrorObject& error_obj) const {
    auto cb_state = GetRead<vvl::CommandBuffer>(commandBuffer);
    return ValidateCmd(*cb_state, error_obj.location);
}

bool CoreChecks::PreCallValidateCmdSetPerformanceOverrideINTEL(VkCommandBuffer commandBuffer,
                                                               const VkPerformanceOverrideInfoINTEL* pOverrideInfo,
                                                               const ErrorObject& error_obj) const {
    auto cb_state = GetRead<vvl::CommandBuffer>(commandBuffer);
    return ValidateCmd(*cb_state, error_obj.location);
}

bool CoreChecks::PreCallValidateCmdSetAttachmentFeedbackLoopEnableEXT(VkCommandBuffer commandBuffer, VkImageAspectFlags aspectMask,
                                                                      const ErrorObject& error_obj) const {
    bool skip = false;
    auto cb_state = GetRead<vvl::CommandBuffer>(commandBuffer);
    if (!enabled_features.attachmentFeedbackLoopDynamicState) {
        skip |= LogError("VUID-vkCmdSetAttachmentFeedbackLoopEnableEXT-attachmentFeedbackLoopDynamicState-08862", commandBuffer,
                         error_obj.location, "attachmentFeedbackLoopDynamicState feature was not enabled.");
    }
    skip |= ValidateCmd(*cb_state, error_obj.location);

    if (aspectMask != VK_IMAGE_ASPECT_NONE && !enabled_features.attachmentFeedbackLoopLayout) {
        skip |= LogError("VUID-vkCmdSetAttachmentFeedbackLoopEnableEXT-attachmentFeedbackLoopLayout-08864", commandBuffer,
                         error_obj.location.dot(Field::aspectMask),
                         "is %s but the attachmentFeedbackLoopLayout feature "
                         "was not enabled.",
                         string_VkImageAspectFlags(aspectMask).c_str());
    }

    if ((aspectMask &
         ~(VK_IMAGE_ASPECT_NONE | VK_IMAGE_ASPECT_COLOR_BIT | VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT)) != 0) {
        skip |= LogError("VUID-vkCmdSetAttachmentFeedbackLoopEnableEXT-aspectMask-08863", commandBuffer,
                         error_obj.location.dot(Field::aspectMask), "is %s.", string_VkImageAspectFlags(aspectMask).c_str());
    }

    return skip;
}
