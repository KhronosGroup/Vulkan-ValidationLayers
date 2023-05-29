/* Copyright (c) 2015-2023 The Khronos Group Inc.
 * Copyright (c) 2015-2023 Valve Corporation
 * Copyright (c) 2015-2023 LunarG, Inc.
 * Copyright (C) 2015-2023 Google Inc.
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

#include <sstream>

#include "generated/vk_enum_string_helper.h"
#include "generated/chassis.h"
#include "core_validation.h"

bool CoreChecks::ValidateDynamicStateIsSet(CBDynamicFlags state_status_cb, CBDynamicState dynamic_state,
                                           const LogObjectList &objlist, CMD_TYPE cmd_type, const char *msg_code) const {
    if (!state_status_cb[dynamic_state]) {
        return LogError(objlist, msg_code, "%s: %s state not set for this command buffer.", CommandTypeString(cmd_type),
                        DynamicStateToString(dynamic_state));
    }
    return false;
}

// Makes sure the vkCmdSet* call was called correctly prior to a draw
bool CoreChecks::ValidateDynamicStateSetStatus(const LAST_BOUND_STATE &last_bound_state, CMD_TYPE cmd_type) const {
    bool skip = false;
    const CMD_BUFFER_STATE &cb_state = last_bound_state.cb_state;
    const PIPELINE_STATE &pipeline = *last_bound_state.pipeline_state;
    const DrawDispatchVuid &vuid = GetDrawDispatchVuid(cmd_type);
    const LogObjectList objlist(cb_state.commandBuffer(), pipeline.pipeline());

    // Verify vkCmdSet* calls since last bound pipeline
    const CBDynamicFlags unset_status_pipeline =
        (cb_state.dynamic_state_status.pipeline ^ pipeline.dynamic_state) & cb_state.dynamic_state_status.pipeline;
    if (unset_status_pipeline.any()) {
        skip |= LogError(objlist, vuid.dynamic_state_setting_commands_02859,
                         "%s: %s doesn't set up %s, but it calls the related dynamic state setting commands.",
                         CommandTypeString(cmd_type), report_data->FormatHandle(pipeline.pipeline()).c_str(),
                         DynamicStatesToString(unset_status_pipeline).c_str());
    }

    // build the mask of what has been set in the Pipeline, but yet to be set in the Command Buffer
    const CBDynamicFlags state_status_cb = ~((cb_state.dynamic_state_status.cb ^ pipeline.dynamic_state) & pipeline.dynamic_state);

    // VK_EXT_extended_dynamic_state
    {
        skip |=
            ValidateDynamicStateIsSet(state_status_cb, CB_DYNAMIC_STATE_CULL_MODE, objlist, cmd_type, vuid.dynamic_cull_mode_07840);
        skip |= ValidateDynamicStateIsSet(state_status_cb, CB_DYNAMIC_STATE_FRONT_FACE, objlist, cmd_type,
                                          vuid.dynamic_front_face_07841);
        skip |= ValidateDynamicStateIsSet(state_status_cb, CB_DYNAMIC_STATE_PRIMITIVE_TOPOLOGY, objlist, cmd_type,
                                          vuid.dynamic_primitive_topology_07842);
        skip |= ValidateDynamicStateIsSet(state_status_cb, CB_DYNAMIC_STATE_DEPTH_TEST_ENABLE, objlist, cmd_type,
                                          vuid.dynamic_depth_test_enable_07843);
        skip |= ValidateDynamicStateIsSet(state_status_cb, CB_DYNAMIC_STATE_DEPTH_WRITE_ENABLE, objlist, cmd_type,
                                          vuid.dynamic_depth_write_enable_07844);
        skip |= ValidateDynamicStateIsSet(state_status_cb, CB_DYNAMIC_STATE_DEPTH_COMPARE_OP, objlist, cmd_type,
                                          vuid.dynamic_depth_compare_op_07845);
        skip |= ValidateDynamicStateIsSet(state_status_cb, CB_DYNAMIC_STATE_DEPTH_BOUNDS_TEST_ENABLE, objlist, cmd_type,
                                          vuid.dynamic_depth_bound_test_enable_07846);
        skip |= ValidateDynamicStateIsSet(state_status_cb, CB_DYNAMIC_STATE_STENCIL_TEST_ENABLE, objlist, cmd_type,
                                          vuid.dynamic_stencil_test_enable_07847);
        skip |= ValidateDynamicStateIsSet(state_status_cb, CB_DYNAMIC_STATE_STENCIL_OP, objlist, cmd_type,
                                          vuid.dynamic_stencil_op_07848);
        skip |= ValidateDynamicStateIsSet(state_status_cb, CB_DYNAMIC_STATE_VERTEX_INPUT_BINDING_STRIDE, objlist, cmd_type,
                                          vuid.vertex_input_binding_stride_04884);
    }

    // VK_EXT_extended_dynamic_state2
    {
        skip |= ValidateDynamicStateIsSet(state_status_cb, CB_DYNAMIC_STATE_PATCH_CONTROL_POINTS_EXT, objlist, cmd_type,
                                          vuid.patch_control_points_04875);
        skip |= ValidateDynamicStateIsSet(state_status_cb, CB_DYNAMIC_STATE_RASTERIZER_DISCARD_ENABLE, objlist, cmd_type,
                                          vuid.rasterizer_discard_enable_04876);
        skip |= ValidateDynamicStateIsSet(state_status_cb, CB_DYNAMIC_STATE_DEPTH_BIAS_ENABLE, objlist, cmd_type,
                                          vuid.depth_bias_enable_04877);
        skip |= ValidateDynamicStateIsSet(state_status_cb, CB_DYNAMIC_STATE_LOGIC_OP_EXT, objlist, cmd_type, vuid.logic_op_04878);
        skip |= ValidateDynamicStateIsSet(state_status_cb, CB_DYNAMIC_STATE_PRIMITIVE_RESTART_ENABLE, objlist, cmd_type,
                                          vuid.primitive_restart_enable_04879);
    }

    // VK_EXT_extended_dynamic_state3
    {
        skip |= ValidateDynamicStateIsSet(state_status_cb, CB_DYNAMIC_STATE_DEPTH_CLAMP_ENABLE_EXT, objlist, cmd_type,
                                          vuid.dynamic_depth_clamp_enable_07620);
        skip |= ValidateDynamicStateIsSet(state_status_cb, CB_DYNAMIC_STATE_POLYGON_MODE_EXT, objlist, cmd_type,
                                          vuid.dynamic_polygon_mode_07621);
        skip |= ValidateDynamicStateIsSet(state_status_cb, CB_DYNAMIC_STATE_RASTERIZATION_SAMPLES_EXT, objlist, cmd_type,
                                          vuid.dynamic_rasterization_samples_07622);
        skip |= ValidateDynamicStateIsSet(state_status_cb, CB_DYNAMIC_STATE_SAMPLE_MASK_EXT, objlist, cmd_type,
                                          vuid.dynamic_sample_mask_07623);
        skip |= ValidateDynamicStateIsSet(state_status_cb, CB_DYNAMIC_STATE_TESSELLATION_DOMAIN_ORIGIN_EXT, objlist, cmd_type,
                                          vuid.dynamic_tessellation_domain_origin_07619);
        skip |= ValidateDynamicStateIsSet(state_status_cb, CB_DYNAMIC_STATE_ALPHA_TO_COVERAGE_ENABLE_EXT, objlist, cmd_type,
                                          vuid.dynamic_alpha_to_coverage_enable_07624);
        skip |= ValidateDynamicStateIsSet(state_status_cb, CB_DYNAMIC_STATE_ALPHA_TO_ONE_ENABLE_EXT, objlist, cmd_type,
                                          vuid.dynamic_alpha_to_one_enable_07625);
        skip |= ValidateDynamicStateIsSet(state_status_cb, CB_DYNAMIC_STATE_LOGIC_OP_ENABLE_EXT, objlist, cmd_type,
                                          vuid.dynamic_logic_op_enable_07626);
        skip |= ValidateDynamicStateIsSet(state_status_cb, CB_DYNAMIC_STATE_RASTERIZATION_STREAM_EXT, objlist, cmd_type,
                                          vuid.dynamic_rasterization_stream_07630);
        skip |= ValidateDynamicStateIsSet(state_status_cb, CB_DYNAMIC_STATE_CONSERVATIVE_RASTERIZATION_MODE_EXT, objlist, cmd_type,
                                          vuid.dynamic_conservative_rasterization_mode_07631);
        skip |= ValidateDynamicStateIsSet(state_status_cb, CB_DYNAMIC_STATE_EXTRA_PRIMITIVE_OVERESTIMATION_SIZE_EXT, objlist,
                                          cmd_type, vuid.dynamic_extra_primitive_overestimation_size_07632);
        skip |= ValidateDynamicStateIsSet(state_status_cb, CB_DYNAMIC_STATE_DEPTH_CLIP_ENABLE_EXT, objlist, cmd_type,
                                          vuid.dynamic_depth_clip_enable_07633);
        skip |= ValidateDynamicStateIsSet(state_status_cb, CB_DYNAMIC_STATE_SAMPLE_LOCATIONS_ENABLE_EXT, objlist, cmd_type,
                                          vuid.dynamic_sample_locations_enable_07634);
        skip |= ValidateDynamicStateIsSet(state_status_cb, CB_DYNAMIC_STATE_PROVOKING_VERTEX_MODE_EXT, objlist, cmd_type,
                                          vuid.dynamic_provoking_vertex_mode_07636);
        skip |= ValidateDynamicStateIsSet(state_status_cb, CB_DYNAMIC_STATE_LINE_RASTERIZATION_MODE_EXT, objlist, cmd_type,
                                          vuid.dynamic_line_rasterization_mode_07637);
        skip |= ValidateDynamicStateIsSet(state_status_cb, CB_DYNAMIC_STATE_LINE_STIPPLE_ENABLE_EXT, objlist, cmd_type,
                                          vuid.dynamic_line_stipple_enable_07638);
        skip |= ValidateDynamicStateIsSet(state_status_cb, CB_DYNAMIC_STATE_DEPTH_CLIP_NEGATIVE_ONE_TO_ONE_EXT, objlist, cmd_type,
                                          vuid.dynamic_depth_clip_negative_one_to_one_07639);
        skip |= ValidateDynamicStateIsSet(state_status_cb, CB_DYNAMIC_STATE_VIEWPORT_W_SCALING_ENABLE_NV, objlist, cmd_type,
                                          vuid.dynamic_viewport_w_scaling_enable_07640);
        skip |= ValidateDynamicStateIsSet(state_status_cb, CB_DYNAMIC_STATE_VIEWPORT_SWIZZLE_NV, objlist, cmd_type,
                                          vuid.dynamic_viewport_swizzle_07641);
        skip |= ValidateDynamicStateIsSet(state_status_cb, CB_DYNAMIC_STATE_COVERAGE_TO_COLOR_ENABLE_NV, objlist, cmd_type,
                                          vuid.dynamic_coverage_to_color_enable_07642);
        skip |= ValidateDynamicStateIsSet(state_status_cb, CB_DYNAMIC_STATE_COVERAGE_TO_COLOR_LOCATION_NV, objlist, cmd_type,
                                          vuid.dynamic_coverage_to_color_location_07643);
        skip |= ValidateDynamicStateIsSet(state_status_cb, CB_DYNAMIC_STATE_COVERAGE_MODULATION_MODE_NV, objlist, cmd_type,
                                          vuid.dynamic_coverage_modulation_mode_07644);
        skip |= ValidateDynamicStateIsSet(state_status_cb, CB_DYNAMIC_STATE_COVERAGE_MODULATION_TABLE_ENABLE_NV, objlist, cmd_type,
                                          vuid.dynamic_coverage_modulation_table_enable_07645);
        skip |= ValidateDynamicStateIsSet(state_status_cb, CB_DYNAMIC_STATE_COVERAGE_MODULATION_TABLE_NV, objlist, cmd_type,
                                          vuid.dynamic_coverage_modulation_table_07646);
        skip |= ValidateDynamicStateIsSet(state_status_cb, CB_DYNAMIC_STATE_SHADING_RATE_IMAGE_ENABLE_NV, objlist, cmd_type,
                                          vuid.dynamic_shading_rate_image_enable_07649);
        skip |= ValidateDynamicStateIsSet(state_status_cb, CB_DYNAMIC_STATE_REPRESENTATIVE_FRAGMENT_TEST_ENABLE_NV, objlist,
                                          cmd_type, vuid.dynamic_representative_fragment_test_enable_07648);
        skip |= ValidateDynamicStateIsSet(state_status_cb, CB_DYNAMIC_STATE_COVERAGE_REDUCTION_MODE_NV, objlist, cmd_type,
                                          vuid.dynamic_coverage_reduction_mode_07647);
        skip |= ValidateDynamicStateIsSet(state_status_cb, CB_DYNAMIC_STATE_SAMPLE_LOCATIONS_EXT, objlist, cmd_type,
                                          vuid.dynamic_sample_locations_06666);
        skip |= ValidateDynamicStateIsSet(state_status_cb, CB_DYNAMIC_STATE_EXCLUSIVE_SCISSOR_NV, objlist, cmd_type,
                                          vuid.dynamic_exclusive_scissor_07878);
        skip |= ValidateDynamicStateIsSet(state_status_cb, CB_DYNAMIC_STATE_EXCLUSIVE_SCISSOR_ENABLE_NV, objlist, cmd_type,
                                          vuid.dynamic_exclusive_scissor_enable_07879);
    }

    // VK_EXT_discard_rectangles
    {
        skip |= ValidateDynamicStateIsSet(state_status_cb, CB_DYNAMIC_STATE_DISCARD_RECTANGLE_ENABLE_EXT, objlist, cmd_type,
                                          vuid.dynamic_discard_rectangle_enable_07880);
        skip |= ValidateDynamicStateIsSet(state_status_cb, CB_DYNAMIC_STATE_DISCARD_RECTANGLE_MODE_EXT, objlist, cmd_type,
                                          vuid.dynamic_discard_rectangle_mode_07881);
    }

    // VK_EXT_vertex_input_dynamic_state
    {
        skip |= ValidateDynamicStateIsSet(state_status_cb, CB_DYNAMIC_STATE_VERTEX_INPUT_EXT, objlist, cmd_type,
                                          vuid.vertex_input_04914);
    }

    // VK_EXT_color_write_enable
    {
        skip |= ValidateDynamicStateIsSet(state_status_cb, CB_DYNAMIC_STATE_COLOR_WRITE_ENABLE_EXT, objlist, cmd_type,
                                          vuid.dynamic_color_write_enable_07749);
    }

    // VK_EXT_attachment_feedback_loop_dynamic_state
    {
        skip |= ValidateDynamicStateIsSet(state_status_cb, CB_DYNAMIC_STATE_ATTACHMENT_FEEDBACK_LOOP_ENABLE_EXT, objlist, cmd_type,
                                          vuid.dynamic_attachment_feedback_loop_08877);
    }

    if (const auto *rp_state = pipeline.RasterizationState(); rp_state) {
        if (rp_state->depthBiasEnable == VK_TRUE) {
            skip |= ValidateDynamicStateIsSet(state_status_cb, CB_DYNAMIC_STATE_DEPTH_BIAS, objlist, cmd_type,
                                              vuid.dynamic_depth_bias_07834);
        }

        // Any line topology
        if (pipeline.topology_at_rasterizer == VK_PRIMITIVE_TOPOLOGY_LINE_LIST ||
            pipeline.topology_at_rasterizer == VK_PRIMITIVE_TOPOLOGY_LINE_STRIP ||
            pipeline.topology_at_rasterizer == VK_PRIMITIVE_TOPOLOGY_LINE_LIST_WITH_ADJACENCY ||
            pipeline.topology_at_rasterizer == VK_PRIMITIVE_TOPOLOGY_LINE_STRIP_WITH_ADJACENCY) {
            skip |= ValidateDynamicStateIsSet(state_status_cb, CB_DYNAMIC_STATE_LINE_WIDTH, objlist, cmd_type,
                                              vuid.dynamic_line_width_07833);
            const auto *line_state = LvlFindInChain<VkPipelineRasterizationLineStateCreateInfoEXT>(rp_state);
            if (line_state && line_state->stippledLineEnable) {
                skip |= ValidateDynamicStateIsSet(state_status_cb, CB_DYNAMIC_STATE_LINE_STIPPLE_EXT, objlist, cmd_type,
                                                  vuid.dynamic_line_stipple_ext_07849);
            }
        }
    }

    if (pipeline.BlendConstantsEnabled()) {
        skip |= ValidateDynamicStateIsSet(state_status_cb, CB_DYNAMIC_STATE_BLEND_CONSTANTS, objlist, cmd_type,
                                          vuid.dynamic_blend_constants_07835);
    }

    if (const auto *ds_state = pipeline.DepthStencilState(); ds_state) {
        if (ds_state->depthBoundsTestEnable == VK_TRUE) {
            skip |= ValidateDynamicStateIsSet(state_status_cb, CB_DYNAMIC_STATE_DEPTH_BOUNDS, objlist, cmd_type,
                                              vuid.dynamic_depth_bounds_07836);
        }
        if (ds_state->stencilTestEnable == VK_TRUE) {
            skip |= ValidateDynamicStateIsSet(state_status_cb, CB_DYNAMIC_STATE_STENCIL_COMPARE_MASK, objlist, cmd_type,
                                              vuid.dynamic_stencil_compare_mask_07837);
            skip |= ValidateDynamicStateIsSet(state_status_cb, CB_DYNAMIC_STATE_STENCIL_WRITE_MASK, objlist, cmd_type,
                                              vuid.dynamic_stencil_write_mask_07838);
            skip |= ValidateDynamicStateIsSet(state_status_cb, CB_DYNAMIC_STATE_STENCIL_REFERENCE, objlist, cmd_type,
                                              vuid.dynamic_stencil_reference_07839);
        }
    }

    return skip;
}

bool CoreChecks::ValidateDrawDynamicState(const LAST_BOUND_STATE &last_bound_state, CMD_TYPE cmd_type) const {
    bool skip = false;
    const CMD_BUFFER_STATE &cb_state = last_bound_state.cb_state;
    const PIPELINE_STATE &pipeline = *last_bound_state.pipeline_state;
    skip = ValidateDynamicStateSetStatus(last_bound_state, cmd_type);
    // Dynamic state was not set, will produce garbage when trying to read to values
    if (skip) return skip;

    const DrawDispatchVuid &vuid = GetDrawDispatchVuid(cmd_type);

    // vkCmdSetDiscardRectangleEXT needs to be set on each rectangle
    const auto *discard_rectangle_state = LvlFindInChain<VkPipelineDiscardRectangleStateCreateInfoEXT>(pipeline.PNext());
    if (discard_rectangle_state && pipeline.IsDynamic(VK_DYNAMIC_STATE_DISCARD_RECTANGLE_EXT)) {
        for (uint32_t i = 0; i < discard_rectangle_state->discardRectangleCount; i++) {
            if (!cb_state.dynamic_state_value.discard_rectangles.test(i)) {
                const LogObjectList objlist(cb_state.commandBuffer(), pipeline.pipeline());
                skip |= LogError(objlist, vuid.dynamic_discard_rectangle_07751,
                                 "%s: vkCmdSetDiscardRectangleEXT was not set for discard rectangle index %" PRIu32
                                 " for this command buffer.",
                                 CommandTypeString(cmd_type), i);
                break;
            }
        }
    }

    // must set the state for all active color attachments in the current subpass
    for (const uint32_t &color_index : cb_state.active_color_attachments_index) {
        if (pipeline.IsDynamic(VK_DYNAMIC_STATE_COLOR_BLEND_ENABLE_EXT) &&
            !cb_state.dynamic_state_value.color_blend_enable_attachments.test(color_index)) {
            const LogObjectList objlist(cb_state.commandBuffer(), pipeline.pipeline());
            skip |= LogError(objlist, vuid.dynamic_color_blend_enable_07476,
                             "%s: vkCmdSetColorBlendEnableEXT was not set for color attachment index %" PRIu32
                             " for this command buffer.",
                             CommandTypeString(cmd_type), color_index);
        }
        if (pipeline.IsDynamic(VK_DYNAMIC_STATE_COLOR_BLEND_EQUATION_EXT) &&
            !cb_state.dynamic_state_value.color_blend_equation_attachments.test(color_index)) {
            const LogObjectList objlist(cb_state.commandBuffer(), pipeline.pipeline());
            skip |= LogError(objlist, vuid.dynamic_color_blend_equation_07477,
                             "%s: vkCmdSetColorBlendEquationEXT was not set for color attachment index %" PRIu32
                             " for this command buffer.",
                             CommandTypeString(cmd_type), color_index);
        }
        if (pipeline.IsDynamic(VK_DYNAMIC_STATE_COLOR_WRITE_MASK_EXT) &&
            !cb_state.dynamic_state_value.color_write_mask_attachments.test(color_index)) {
            const LogObjectList objlist(cb_state.commandBuffer(), pipeline.pipeline());
            skip |= LogError(objlist, vuid.dynamic_color_write_mask_07478,
                             "%s: vkCmdSetColorWriteMaskEXT was not set for color attachment index %" PRIu32
                             " for this command buffer.",
                             CommandTypeString(cmd_type), color_index);
        }
        if (pipeline.IsDynamic(VK_DYNAMIC_STATE_COLOR_BLEND_ADVANCED_EXT) &&
            !cb_state.dynamic_state_value.color_blend_advanced_attachments.test(color_index)) {
            const LogObjectList objlist(cb_state.commandBuffer(), pipeline.pipeline());
            skip |= LogError(objlist, vuid.dynamic_color_blend_advanced_07479,
                             "%s: vkCmdSetColorBlendAdvancedEXT was not set for color attachment index %" PRIu32
                             " for this command buffer.",
                             CommandTypeString(cmd_type), color_index);
        }
    }

    // If Viewport or scissors are dynamic, verify that dynamic count matches PSO count.
    // Skip check if rasterization is disabled, if there is no viewport, or if viewport/scissors are being inherited.
    const bool dyn_viewport = pipeline.IsDynamic(VK_DYNAMIC_STATE_VIEWPORT);
    const auto *rp_state = pipeline.RasterizationState();
    const auto *viewport_state = pipeline.ViewportState();
    if ((!rp_state || (rp_state->rasterizerDiscardEnable == VK_FALSE)) && viewport_state &&
        (cb_state.inheritedViewportDepths.size() == 0)) {
        const bool dyn_scissor = pipeline.IsDynamic(VK_DYNAMIC_STATE_SCISSOR);

        // NB (akeley98): Current validation layers do not detect the error where vkCmdSetViewport (or scissor) was called, but
        // the dynamic state set is overwritten by binding a graphics pipeline with static viewport (scissor) state.
        // This condition be detected by checking trashedViewportMask & viewportMask (trashedScissorMask & scissorMask) is
        // nonzero in the range of bits needed by the pipeline.
        if (dyn_viewport) {
            const auto required_viewports_mask = (1 << viewport_state->viewportCount) - 1;
            const auto missing_viewport_mask = ~cb_state.viewportMask & required_viewports_mask;
            if (missing_viewport_mask) {
                const LogObjectList objlist(cb_state.commandBuffer(), pipeline.pipeline());
                skip |=
                    LogError(objlist, vuid.dynamic_viewport_07831,
                             "%s: Dynamic viewport(s) (0x%x) are used by pipeline state object, but were not provided via calls "
                             "to vkCmdSetViewport().",
                             CommandTypeString(cmd_type), missing_viewport_mask);
            }
        }

        if (dyn_scissor) {
            const auto required_scissor_mask = (1 << viewport_state->scissorCount) - 1;
            const auto missing_scissor_mask = ~cb_state.scissorMask & required_scissor_mask;
            if (missing_scissor_mask) {
                const LogObjectList objlist(cb_state.commandBuffer(), pipeline.pipeline());
                skip |= LogError(objlist, vuid.dynamic_scissor_07832,
                                 "%s: Dynamic scissor(s) (0x%x) are used by pipeline state object, but were not provided via calls "
                                 "to vkCmdSetScissor().",
                                 CommandTypeString(cmd_type), missing_scissor_mask);
            }
        }

        const bool dyn_viewport_count = pipeline.IsDynamic(VK_DYNAMIC_STATE_VIEWPORT_WITH_COUNT);
        const bool dyn_scissor_count = pipeline.IsDynamic(VK_DYNAMIC_STATE_SCISSOR_WITH_COUNT);

        if (dyn_viewport_count && !dyn_scissor_count) {
            const auto required_viewport_mask = (1 << viewport_state->scissorCount) - 1;
            const auto missing_viewport_mask = ~cb_state.viewportWithCountMask & required_viewport_mask;
            if (missing_viewport_mask) {
                const LogObjectList objlist(cb_state.commandBuffer(), pipeline.pipeline());
                skip |= LogError(objlist, vuid.viewport_count_03417,
                                 "%s: Dynamic viewport with count 0x%x are used by pipeline state object, but were not provided "
                                 "via calls to vkCmdSetViewportWithCountEXT().",
                                 CommandTypeString(cmd_type), missing_viewport_mask);
            }
        }

        if (dyn_scissor_count && !dyn_viewport_count) {
            const auto required_scissor_mask = (1 << viewport_state->viewportCount) - 1;
            const auto missing_scissor_mask = ~cb_state.scissorWithCountMask & required_scissor_mask;
            if (missing_scissor_mask) {
                const LogObjectList objlist(cb_state.commandBuffer(), pipeline.pipeline());
                skip |= LogError(objlist, vuid.scissor_count_03418,
                                 "%s: Dynamic scissor with count 0x%x are used by pipeline state object, but were not provided via "
                                 "calls to vkCmdSetScissorWithCountEXT().",
                                 CommandTypeString(cmd_type), missing_scissor_mask);
            }
        }

        if (dyn_scissor_count && dyn_viewport_count) {
            if (cb_state.viewportWithCountMask != cb_state.scissorWithCountMask) {
                const LogObjectList objlist(cb_state.commandBuffer(), pipeline.pipeline());
                skip |= LogError(objlist, vuid.viewport_scissor_count_03419,
                                 "%s: Dynamic viewport and scissor with count 0x%x are used by pipeline state object, but were not "
                                 "provided via matching calls to "
                                 "vkCmdSetViewportWithCountEXT and vkCmdSetScissorWithCountEXT().",
                                 CommandTypeString(cmd_type), (cb_state.viewportWithCountMask ^ cb_state.scissorWithCountMask));
            }
        }
    }

    // If inheriting viewports, verify that not using more than inherited.
    if (cb_state.inheritedViewportDepths.size() != 0 && dyn_viewport) {
        const uint32_t viewport_count = viewport_state->viewportCount;
        const uint32_t max_inherited = uint32_t(cb_state.inheritedViewportDepths.size());
        if (viewport_count > max_inherited) {
            const LogObjectList objlist(cb_state.commandBuffer(), pipeline.pipeline());
            skip |=
                LogError(objlist, vuid.dynamic_state_inherited_07850,
                         "%s: Pipeline requires more viewports (%" PRIu32 ".) than inherited (viewportDepthCount = %" PRIu32 ".).",
                         CommandTypeString(cmd_type), viewport_count, max_inherited);
        }
    }

    if (pipeline.IsDynamic(VK_DYNAMIC_STATE_COLOR_WRITE_ENABLE_EXT) &&
        cb_state.dynamic_state_status.cb[CB_DYNAMIC_STATE_COLOR_WRITE_ENABLE_EXT]) {
        const auto color_blend_state = cb_state.GetCurrentPipeline(VK_PIPELINE_BIND_POINT_GRAPHICS)->ColorBlendState();
        if (color_blend_state) {
            uint32_t blend_attachment_count = color_blend_state->attachmentCount;
            uint32_t dynamic_attachment_count = cb_state.dynamic_state_value.color_write_enable_attachment_count;
            if (dynamic_attachment_count < blend_attachment_count) {
                const LogObjectList objlist(cb_state.commandBuffer(), pipeline.pipeline());
                skip |= LogError(
                    objlist, vuid.dynamic_color_write_enable_count_07750,
                    "%s(): Currently bound pipeline was created with VkPipelineColorBlendStateCreateInfo::attachmentCount %" PRIu32
                    " and VK_DYNAMIC_STATE_COLOR_WRITE_ENABLE_EXT, but the number of attachments written by "
                    "vkCmdSetColorWriteEnableEXT() is %" PRIu32 ".",
                    CommandTypeString(cmd_type), blend_attachment_count, dynamic_attachment_count);
            }
        }
    }

    // VK_EXT_shader_tile_image
    {
        const bool dyn_depth_write_enable = pipeline.IsDynamic(VK_DYNAMIC_STATE_DEPTH_WRITE_ENABLE);
        const bool dyn_stencil_write_mask = pipeline.IsDynamic(VK_DYNAMIC_STATE_STENCIL_WRITE_MASK);
        if ((dyn_depth_write_enable || dyn_stencil_write_mask) &&
            (pipeline.fragment_shader_state && pipeline.fragment_shader_state->fragment_shader)) {
            std::shared_ptr<const SHADER_MODULE_STATE> module_state = pipeline.fragment_shader_state->fragment_shader;
            const safe_VkPipelineShaderStageCreateInfo *stage_ci = pipeline.fragment_shader_state->fragment_shader_ci.get();
            auto entrypoint = module_state->FindEntrypoint(stage_ci->pName, stage_ci->stage);
            const bool mode_early_fragment_test =
                entrypoint && entrypoint->execution_mode.Has(ExecutionModeSet::early_fragment_test_bit);

            if (module_state->static_data_.has_shader_tile_image_depth_read && dyn_depth_write_enable && mode_early_fragment_test &&
                cb_state.dynamic_state_value.depth_write_enable) {
                const LogObjectList objlist(cb_state.commandBuffer(), pipeline.pipeline());
                skip |=
                    LogError(objlist, vuid.dynamic_depth_enable_08715,
                             "%s(): Fragment shader contains OpDepthAttachmentReadEXT, but depthWriteEnable parameter in the last "
                             "call to vkCmdSetDepthWriteEnable is not false.",
                             CommandTypeString(cmd_type));
            }

            if (module_state->static_data_.has_shader_tile_image_stencil_read && dyn_stencil_write_mask &&
                mode_early_fragment_test &&
                ((cb_state.dynamic_state_value.write_mask_front != 0) || (cb_state.dynamic_state_value.write_mask_back != 0))) {
                const LogObjectList objlist(cb_state.commandBuffer(), pipeline.pipeline());
                skip |= LogError(objlist, vuid.dynamic_stencil_write_mask_08716,
                                 "%s(): Fragment shader contains OpStencilAttachmentReadEXT, but writeMask parameter in the last "
                                 "call to vkCmdSetStencilWriteMask is not equal to 0 for both front (=%" PRIu32
                                 ") and back (=%" PRIu32 ").",
                                 CommandTypeString(cmd_type), cb_state.dynamic_state_value.write_mask_front,
                                 cb_state.dynamic_state_value.write_mask_back);
            }
        }
    }

    // Makes sure topology is compatible (in same topology class)
    // see vkspec.html#drawing-primitive-topology-class
    if (pipeline.IsDynamic(VK_DYNAMIC_STATE_PRIMITIVE_TOPOLOGY) &&
        !phys_dev_ext_props.extended_dynamic_state3_props.dynamicPrimitiveTopologyUnrestricted) {
        bool compatible_topology = false;
        const VkPrimitiveTopology pipeline_topology = pipeline.InputAssemblyState()->topology;
        const VkPrimitiveTopology dynamic_topology = cb_state.dynamic_state_value.primitive_topology;
        switch (pipeline_topology) {
            case VK_PRIMITIVE_TOPOLOGY_POINT_LIST:
                switch (dynamic_topology) {
                    case VK_PRIMITIVE_TOPOLOGY_POINT_LIST:
                        compatible_topology = true;
                        break;
                    default:
                        break;
                }
                break;
            case VK_PRIMITIVE_TOPOLOGY_LINE_LIST:
            case VK_PRIMITIVE_TOPOLOGY_LINE_STRIP:
            case VK_PRIMITIVE_TOPOLOGY_LINE_LIST_WITH_ADJACENCY:
            case VK_PRIMITIVE_TOPOLOGY_LINE_STRIP_WITH_ADJACENCY:
                switch (dynamic_topology) {
                    case VK_PRIMITIVE_TOPOLOGY_LINE_LIST:
                    case VK_PRIMITIVE_TOPOLOGY_LINE_STRIP:
                    case VK_PRIMITIVE_TOPOLOGY_LINE_LIST_WITH_ADJACENCY:
                    case VK_PRIMITIVE_TOPOLOGY_LINE_STRIP_WITH_ADJACENCY:
                        compatible_topology = true;
                        break;
                    default:
                        break;
                }
                break;
            case VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST:
            case VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP:
            case VK_PRIMITIVE_TOPOLOGY_TRIANGLE_FAN:
            case VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST_WITH_ADJACENCY:
            case VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP_WITH_ADJACENCY:
                switch (dynamic_topology) {
                    case VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST:
                    case VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP:
                    case VK_PRIMITIVE_TOPOLOGY_TRIANGLE_FAN:
                    case VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST_WITH_ADJACENCY:
                    case VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP_WITH_ADJACENCY:
                        compatible_topology = true;
                        break;
                    default:
                        break;
                }
                break;
            case VK_PRIMITIVE_TOPOLOGY_PATCH_LIST:
                switch (dynamic_topology) {
                    case VK_PRIMITIVE_TOPOLOGY_PATCH_LIST:
                        compatible_topology = true;
                        break;
                    default:
                        break;
                }
                break;
            default:
                break;
        }
        if (!compatible_topology) {
            const char *vuid_error = IsExtEnabled(device_extensions.vk_ext_extended_dynamic_state3)
                                         ? vuid.primitive_topology_class_ds3_07500
                                         : vuid.primitive_topology_class_03420;
            const LogObjectList objlist(cb_state.commandBuffer(), pipeline.pipeline());
            skip |= LogError(objlist, vuid_error,
                             "%s: the last primitive topology %s state set by vkCmdSetPrimitiveTopology is "
                             "not compatible with the pipeline topology %s.",
                             CommandTypeString(cmd_type), string_VkPrimitiveTopology(dynamic_topology),
                             string_VkPrimitiveTopology(pipeline_topology));
        }
    }

    return skip;
}

bool CoreChecks::ForbidInheritedViewportScissor(const CMD_BUFFER_STATE &cb_state, const char *vuid, const CMD_TYPE cmd_type) const {
    bool skip = false;
    if (cb_state.inheritedViewportDepths.size() != 0) {
        skip |=
            LogError(cb_state.commandBuffer(), vuid,
                     "%s: commandBuffer must not have VkCommandBufferInheritanceViewportScissorInfoNV::viewportScissor2D enabled.",
                     CommandTypeString(cmd_type));
    }
    return skip;
}

// Used for all vkCmdSet* functions
// Some calls are behind a feature bit that needs to be enabled
bool CoreChecks::ValidateExtendedDynamicState(const CMD_BUFFER_STATE &cb_state, const CMD_TYPE cmd_type, bool feature,
                                              const char *vuid, const char *feature_name) const {
    bool skip = false;
    skip |= ValidateCmd(cb_state, cmd_type);

    if (!feature) {
        const char *func_name = CommandTypeString(cmd_type);
        skip |= LogError(cb_state.Handle(), vuid, "%s(): %s feature is not enabled.", func_name, feature_name);
    }
    return skip;
}

bool CoreChecks::PreCallValidateCmdSetViewport(VkCommandBuffer commandBuffer, uint32_t firstViewport, uint32_t viewportCount,
                                               const VkViewport *pViewports) const {
    auto cb_state = GetRead<CMD_BUFFER_STATE>(commandBuffer);
    bool skip = false;
    skip |= ValidateExtendedDynamicState(*cb_state, CMD_SETVIEWPORT, VK_TRUE, nullptr, nullptr);
    skip |= ForbidInheritedViewportScissor(*cb_state, "VUID-vkCmdSetViewport-commandBuffer-04821", CMD_SETVIEWPORT);
    return skip;
}

bool CoreChecks::PreCallValidateCmdSetScissor(VkCommandBuffer commandBuffer, uint32_t firstScissor, uint32_t scissorCount,
                                              const VkRect2D *pScissors) const {
    auto cb_state = GetRead<CMD_BUFFER_STATE>(commandBuffer);
    bool skip = false;
    skip |= ValidateExtendedDynamicState(*cb_state, CMD_SETSCISSOR, VK_TRUE, nullptr, nullptr);
    skip |= ForbidInheritedViewportScissor(*cb_state, "VUID-vkCmdSetScissor-viewportScissor2D-04789", CMD_SETSCISSOR);
    return skip;
}

bool CoreChecks::PreCallValidateCmdSetExclusiveScissorNV(VkCommandBuffer commandBuffer, uint32_t firstExclusiveScissor,
                                                         uint32_t exclusiveScissorCount, const VkRect2D *pExclusiveScissors) const {
    auto cb_state = GetRead<CMD_BUFFER_STATE>(commandBuffer);
    return ValidateExtendedDynamicState(*cb_state, CMD_SETEXCLUSIVESCISSORNV,
                                        enabled_features.exclusive_scissor_features.exclusiveScissor,
                                        "VUID-vkCmdSetExclusiveScissorNV-None-02031", "exclusiveScissor");
}

bool CoreChecks::PreCallValidateCmdSetViewportShadingRatePaletteNV(VkCommandBuffer commandBuffer, uint32_t firstViewport,
                                                                   uint32_t viewportCount,
                                                                   const VkShadingRatePaletteNV *pShadingRatePalettes) const {
    auto cb_state = GetRead<CMD_BUFFER_STATE>(commandBuffer);
    bool skip = false;

    skip |= ValidateExtendedDynamicState(*cb_state, CMD_SETVIEWPORTSHADINGRATEPALETTENV,
                                         enabled_features.shading_rate_image_features.shadingRateImage,
                                         "VUID-vkCmdSetViewportShadingRatePaletteNV-None-02064", "shadingRateImage");

    for (uint32_t i = 0; i < viewportCount; ++i) {
        auto *palette = &pShadingRatePalettes[i];
        if (palette->shadingRatePaletteEntryCount == 0 ||
            palette->shadingRatePaletteEntryCount > phys_dev_ext_props.shading_rate_image_props.shadingRatePaletteSize) {
            skip |= LogError(
                commandBuffer, "VUID-VkShadingRatePaletteNV-shadingRatePaletteEntryCount-02071",
                "vkCmdSetViewportShadingRatePaletteNV: shadingRatePaletteEntryCount must be between 1 and shadingRatePaletteSize.");
        }
    }

    return skip;
}

bool CoreChecks::PreCallValidateCmdSetViewportWScalingNV(VkCommandBuffer commandBuffer, uint32_t firstViewport,
                                                         uint32_t viewportCount,
                                                         const VkViewportWScalingNV *pViewportWScalings) const {
    auto cb_state = GetRead<CMD_BUFFER_STATE>(commandBuffer);
    return ValidateExtendedDynamicState(*cb_state, CMD_SETVIEWPORTWSCALINGNV, VK_TRUE, nullptr, nullptr);
}

bool CoreChecks::PreCallValidateCmdSetLineWidth(VkCommandBuffer commandBuffer, float lineWidth) const {
    auto cb_state = GetRead<CMD_BUFFER_STATE>(commandBuffer);
    return ValidateExtendedDynamicState(*cb_state, CMD_SETLINEWIDTH, VK_TRUE, nullptr, nullptr);
}

bool CoreChecks::PreCallValidateCmdSetLineStippleEXT(VkCommandBuffer commandBuffer, uint32_t lineStippleFactor,
                                                     uint16_t lineStipplePattern) const {
    auto cb_state = GetRead<CMD_BUFFER_STATE>(commandBuffer);
    return ValidateExtendedDynamicState(*cb_state, CMD_SETLINESTIPPLEEXT, VK_TRUE, nullptr, nullptr);
}

bool CoreChecks::PreCallValidateCmdSetDepthBias(VkCommandBuffer commandBuffer, float depthBiasConstantFactor, float depthBiasClamp,
                                                float depthBiasSlopeFactor) const {
    auto cb_state = GetRead<CMD_BUFFER_STATE>(commandBuffer);
    bool skip = false;
    skip |= ValidateExtendedDynamicState(*cb_state, CMD_SETDEPTHBIAS, VK_TRUE, nullptr, nullptr);
    if ((depthBiasClamp != 0.0) && (!enabled_features.core.depthBiasClamp)) {
        skip |= LogError(commandBuffer, "VUID-vkCmdSetDepthBias-depthBiasClamp-00790",
                         "vkCmdSetDepthBias(): the depthBiasClamp device feature is disabled: the depthBiasClamp parameter must "
                         "be set to 0.0.");
    }
    return skip;
}

bool CoreChecks::PreCallValidateCmdSetBlendConstants(VkCommandBuffer commandBuffer, const float blendConstants[4]) const {
    auto cb_state = GetRead<CMD_BUFFER_STATE>(commandBuffer);
    return ValidateExtendedDynamicState(*cb_state, CMD_SETBLENDCONSTANTS, VK_TRUE, nullptr, nullptr);
}

bool CoreChecks::PreCallValidateCmdSetDepthBounds(VkCommandBuffer commandBuffer, float minDepthBounds, float maxDepthBounds) const {
    auto cb_state = GetRead<CMD_BUFFER_STATE>(commandBuffer);
    bool skip = false;
    skip |= ValidateExtendedDynamicState(*cb_state, CMD_SETDEPTHBOUNDS, VK_TRUE, nullptr, nullptr);

    if (!IsExtEnabled(device_extensions.vk_ext_depth_range_unrestricted)) {
        if (!(minDepthBounds >= 0.0) || !(minDepthBounds <= 1.0)) {
            skip |= LogError(commandBuffer, "VUID-vkCmdSetDepthBounds-minDepthBounds-02508",
                             "vkCmdSetDepthBounds(): VK_EXT_depth_range_unrestricted extension is not enabled and minDepthBounds "
                             "(=%f) is not within the [0.0, 1.0] range.",
                             minDepthBounds);
        }

        if (!(maxDepthBounds >= 0.0) || !(maxDepthBounds <= 1.0)) {
            skip |= LogError(commandBuffer, "VUID-vkCmdSetDepthBounds-maxDepthBounds-02509",
                             "vkCmdSetDepthBounds(): VK_EXT_depth_range_unrestricted extension is not enabled and maxDepthBounds "
                             "(=%f) is not within the [0.0, 1.0] range.",
                             maxDepthBounds);
        }
    }
    return skip;
}

bool CoreChecks::PreCallValidateCmdSetStencilCompareMask(VkCommandBuffer commandBuffer, VkStencilFaceFlags faceMask,
                                                         uint32_t compareMask) const {
    auto cb_state = GetRead<CMD_BUFFER_STATE>(commandBuffer);
    return ValidateExtendedDynamicState(*cb_state, CMD_SETSTENCILCOMPAREMASK, VK_TRUE, nullptr, nullptr);
}

bool CoreChecks::PreCallValidateCmdSetStencilWriteMask(VkCommandBuffer commandBuffer, VkStencilFaceFlags faceMask,
                                                       uint32_t writeMask) const {
    auto cb_state = GetRead<CMD_BUFFER_STATE>(commandBuffer);
    return ValidateExtendedDynamicState(*cb_state, CMD_SETSTENCILWRITEMASK, VK_TRUE, nullptr, nullptr);
}

bool CoreChecks::PreCallValidateCmdSetStencilReference(VkCommandBuffer commandBuffer, VkStencilFaceFlags faceMask,
                                                       uint32_t reference) const {
    auto cb_state = GetRead<CMD_BUFFER_STATE>(commandBuffer);
    return ValidateExtendedDynamicState(*cb_state, CMD_SETSTENCILREFERENCE, VK_TRUE, nullptr, nullptr);
}

bool CoreChecks::PreCallValidateCmdSetDiscardRectangleEXT(VkCommandBuffer commandBuffer, uint32_t firstDiscardRectangle,
                                                          uint32_t discardRectangleCount,
                                                          const VkRect2D *pDiscardRectangles) const {
    auto cb_state = GetRead<CMD_BUFFER_STATE>(commandBuffer);
    bool skip = false;
    // Minimal validation for command buffer state
    skip |= ValidateExtendedDynamicState(*cb_state, CMD_SETDISCARDRECTANGLEEXT, VK_TRUE, nullptr, nullptr);
    skip |= ForbidInheritedViewportScissor(*cb_state, "VUID-vkCmdSetDiscardRectangleEXT-viewportScissor2D-04788",
                                           CMD_SETDISCARDRECTANGLEEXT);
    for (uint32_t i = 0; i < discardRectangleCount; ++i) {
        if (pDiscardRectangles[i].offset.x < 0) {
            skip |= LogError(cb_state->commandBuffer(), "VUID-vkCmdSetDiscardRectangleEXT-x-00587",
                             "vkCmdSetDiscardRectangleEXT(): pDiscardRectangles[%" PRIu32 "].x (%" PRIi32 ") is negative.", i,
                             pDiscardRectangles[i].offset.x);
        }
        if (pDiscardRectangles[i].offset.y < 0) {
            skip |= LogError(cb_state->commandBuffer(), "VUID-vkCmdSetDiscardRectangleEXT-x-00587",
                             "vkCmdSetDiscardRectangleEXT(): pDiscardRectangles[%" PRIu32 "].y (%" PRIi32 ") is negative.", i,
                             pDiscardRectangles[i].offset.y);
        }
    }
    if (firstDiscardRectangle + discardRectangleCount > phys_dev_ext_props.discard_rectangle_props.maxDiscardRectangles) {
        skip |=
            LogError(cb_state->commandBuffer(), "VUID-vkCmdSetDiscardRectangleEXT-firstDiscardRectangle-00585",
                     "vkCmdSetDiscardRectangleEXT(): firstDiscardRectangle (%" PRIu32 ") + discardRectangleCount (%" PRIu32
                     ") is not less than VkPhysicalDeviceDiscardRectanglePropertiesEXT::maxDiscardRectangles (%" PRIu32 ").",
                     firstDiscardRectangle, discardRectangleCount, phys_dev_ext_props.discard_rectangle_props.maxDiscardRectangles);
    }
    return skip;
}

bool CoreChecks::PreCallValidateCmdSetSampleLocationsEXT(VkCommandBuffer commandBuffer,
                                                         const VkSampleLocationsInfoEXT *pSampleLocationsInfo) const {
    bool skip = false;
    auto cb_state = GetRead<CMD_BUFFER_STATE>(commandBuffer);
    // Minimal validation for command buffer state
    skip |= ValidateExtendedDynamicState(*cb_state, CMD_SETSAMPLELOCATIONSEXT, VK_TRUE, nullptr, nullptr);
    skip |= ValidateSampleLocationsInfo(pSampleLocationsInfo, "vkCmdSetSampleLocationsEXT");

    return skip;
}

bool CoreChecks::PreCallValidateCmdSetCheckpointNV(VkCommandBuffer commandBuffer, const void *pCheckpointMarker) const {
    auto cb_state = GetRead<CMD_BUFFER_STATE>(commandBuffer);
    return ValidateExtendedDynamicState(*cb_state, CMD_SETCHECKPOINTNV, VK_TRUE, nullptr, nullptr);
}

bool CoreChecks::PreCallValidateCmdSetLogicOpEXT(VkCommandBuffer commandBuffer, VkLogicOp logicOp) const {
    auto cb_state = GetRead<CMD_BUFFER_STATE>(commandBuffer);
    return ValidateExtendedDynamicState(*cb_state, CMD_SETLOGICOPEXT,
                                        enabled_features.extended_dynamic_state2_features.extendedDynamicState2LogicOp ||
                                            enabled_features.shader_object_features.shaderObject,
                                        "VUID-vkCmdSetLogicOpEXT-None-08544", "extendedDynamicState2LogicOp or shaderObject");
}

bool CoreChecks::PreCallValidateCmdSetPatchControlPointsEXT(VkCommandBuffer commandBuffer, uint32_t patchControlPoints) const {
    auto cb_state = GetRead<CMD_BUFFER_STATE>(commandBuffer);
    bool skip = false;
    skip |=
        ValidateExtendedDynamicState(*cb_state, CMD_SETPATCHCONTROLPOINTSEXT,
                                     enabled_features.extended_dynamic_state2_features.extendedDynamicState2PatchControlPoints ||
                                         enabled_features.shader_object_features.shaderObject,
                                     "VUID-vkCmdSetPatchControlPointsEXT-None-08574",
                                     "extendedDynamicState2PatchControlPoints or shaderObject");

    if (patchControlPoints > phys_dev_props.limits.maxTessellationPatchSize) {
        skip |= LogError(commandBuffer, "VUID-vkCmdSetPatchControlPointsEXT-patchControlPoints-04874",
                         "vkCmdSetPatchControlPointsEXT: The value of patchControlPoints (%" PRIu32
                         ") must be less than "
                         "VkPhysicalDeviceLimits::maxTessellationPatchSize (%" PRIu32 ")",
                         patchControlPoints, phys_dev_props.limits.maxTessellationPatchSize);
    }
    return skip;
}

bool CoreChecks::PreCallValidateCmdSetRasterizerDiscardEnableEXT(VkCommandBuffer commandBuffer,
                                                                 VkBool32 rasterizerDiscardEnable) const {
    auto cb_state = GetRead<CMD_BUFFER_STATE>(commandBuffer);
    return ValidateExtendedDynamicState(*cb_state, CMD_SETRASTERIZERDISCARDENABLEEXT,
                                        enabled_features.extended_dynamic_state2_features.extendedDynamicState2 ||
                                            enabled_features.shader_object_features.shaderObject,
                                        "VUID-vkCmdSetRasterizerDiscardEnable-None-08548", "extendedDynamicState2 or shaderObject");
}

bool CoreChecks::PreCallValidateCmdSetRasterizerDiscardEnable(VkCommandBuffer commandBuffer,
                                                              VkBool32 rasterizerDiscardEnable) const {
    auto cb_state = GetRead<CMD_BUFFER_STATE>(commandBuffer);
    return ValidateExtendedDynamicState(*cb_state, CMD_SETRASTERIZERDISCARDENABLE, VK_TRUE, nullptr, nullptr);
}

bool CoreChecks::PreCallValidateCmdSetDepthBiasEnableEXT(VkCommandBuffer commandBuffer, VkBool32 depthBiasEnable) const {
    auto cb_state = GetRead<CMD_BUFFER_STATE>(commandBuffer);
    return ValidateExtendedDynamicState(*cb_state, CMD_SETDEPTHBIASENABLEEXT,
                                        enabled_features.extended_dynamic_state2_features.extendedDynamicState2 ||
                                            enabled_features.shader_object_features.shaderObject,
                                        "VUID-vkCmdSetDepthBiasEnable-None-08568", "extendedDynamicState2 or shaderObject");
}

bool CoreChecks::PreCallValidateCmdSetDepthBiasEnable(VkCommandBuffer commandBuffer, VkBool32 depthBiasEnable) const {
    auto cb_state = GetRead<CMD_BUFFER_STATE>(commandBuffer);
    return ValidateExtendedDynamicState(*cb_state, CMD_SETDEPTHBIASENABLE, VK_TRUE, nullptr, nullptr);
}

bool CoreChecks::PreCallValidateCmdSetPrimitiveRestartEnableEXT(VkCommandBuffer commandBuffer,
                                                                VkBool32 primitiveRestartEnable) const {
    auto cb_state = GetRead<CMD_BUFFER_STATE>(commandBuffer);
    return ValidateExtendedDynamicState(*cb_state, CMD_SETPRIMITIVERESTARTENABLEEXT,
                                        enabled_features.extended_dynamic_state2_features.extendedDynamicState2 ||
                                            enabled_features.shader_object_features.shaderObject,
                                        "VUID-vkCmdSetPrimitiveRestartEnable-None-08500", "extendedDynamicState2 or shaderObject");
}

bool CoreChecks::PreCallValidateCmdSetPrimitiveRestartEnable(VkCommandBuffer commandBuffer, VkBool32 primitiveRestartEnable) const {
    auto cb_state = GetRead<CMD_BUFFER_STATE>(commandBuffer);
    return ValidateExtendedDynamicState(*cb_state, CMD_SETPRIMITIVERESTARTENABLE, VK_TRUE, nullptr, nullptr);
}

bool CoreChecks::PreCallValidateCmdSetCullModeEXT(VkCommandBuffer commandBuffer, VkCullModeFlags cullMode) const {
    auto cb_state = GetRead<CMD_BUFFER_STATE>(commandBuffer);
    return ValidateExtendedDynamicState(*cb_state, CMD_SETCULLMODEEXT,
                                        enabled_features.extended_dynamic_state_features.extendedDynamicState ||
                                            enabled_features.shader_object_features.shaderObject,
                                        "VUID-vkCmdSetCullMode-None-08564", "extendedDynamicState or shaderObject");
}

bool CoreChecks::PreCallValidateCmdSetCullMode(VkCommandBuffer commandBuffer, VkCullModeFlags cullMode) const {
    auto cb_state = GetRead<CMD_BUFFER_STATE>(commandBuffer);
    return ValidateExtendedDynamicState(*cb_state, CMD_SETCULLMODE, VK_TRUE, nullptr, nullptr);
}

bool CoreChecks::PreCallValidateCmdSetFrontFaceEXT(VkCommandBuffer commandBuffer, VkFrontFace frontFace) const {
    auto cb_state = GetRead<CMD_BUFFER_STATE>(commandBuffer);
    return ValidateExtendedDynamicState(*cb_state, CMD_SETFRONTFACEEXT,
                                        enabled_features.extended_dynamic_state_features.extendedDynamicState ||
                                            enabled_features.shader_object_features.shaderObject,
                                        "VUID-vkCmdSetFrontFace-None-08562", "extendedDynamicState or shaderObject");
}

bool CoreChecks::PreCallValidateCmdSetFrontFace(VkCommandBuffer commandBuffer, VkFrontFace frontFace) const {
    auto cb_state = GetRead<CMD_BUFFER_STATE>(commandBuffer);
    return ValidateExtendedDynamicState(*cb_state, CMD_SETFRONTFACE, VK_TRUE, nullptr, nullptr);
}

bool CoreChecks::PreCallValidateCmdSetPrimitiveTopologyEXT(VkCommandBuffer commandBuffer,
                                                           VkPrimitiveTopology primitiveTopology) const {
    auto cb_state = GetRead<CMD_BUFFER_STATE>(commandBuffer);
    return ValidateExtendedDynamicState(*cb_state, CMD_SETPRIMITIVETOPOLOGYEXT,
                                        enabled_features.extended_dynamic_state_features.extendedDynamicState ||
                                            enabled_features.shader_object_features.shaderObject,
                                        "VUID-vkCmdSetPrimitiveTopology-None-08502", "extendedDynamicState or shaderObject");
}

bool CoreChecks::PreCallValidateCmdSetPrimitiveTopology(VkCommandBuffer commandBuffer,
                                                        VkPrimitiveTopology primitiveTopology) const {
    auto cb_state = GetRead<CMD_BUFFER_STATE>(commandBuffer);
    return ValidateExtendedDynamicState(*cb_state, CMD_SETPRIMITIVETOPOLOGY, VK_TRUE, nullptr, nullptr);
}

bool CoreChecks::PreCallValidateCmdSetViewportWithCountEXT(VkCommandBuffer commandBuffer, uint32_t viewportCount,
                                                           const VkViewport *pViewports) const {
    auto cb_state = GetRead<CMD_BUFFER_STATE>(commandBuffer);
    bool skip = false;
    skip = ValidateExtendedDynamicState(*cb_state, CMD_SETVIEWPORTWITHCOUNTEXT,
                                        enabled_features.extended_dynamic_state_features.extendedDynamicState ||
                                            enabled_features.shader_object_features.shaderObject,
                                        "VUID-vkCmdSetViewportWithCount-None-08588", "extendedDynamicState or shaderObject");
    skip |= ForbidInheritedViewportScissor(*cb_state, "VUID-vkCmdSetViewportWithCount-commandBuffer-04819",
                                           CMD_SETVIEWPORTWITHCOUNTEXT);

    return skip;
}

bool CoreChecks::PreCallValidateCmdSetViewportWithCount(VkCommandBuffer commandBuffer, uint32_t viewportCount,
                                                        const VkViewport *pViewports) const {
    auto cb_state = GetRead<CMD_BUFFER_STATE>(commandBuffer);
    bool skip = false;
    skip = ValidateExtendedDynamicState(*cb_state, CMD_SETVIEWPORTWITHCOUNT, VK_TRUE, nullptr, nullptr);
    skip |=
        ForbidInheritedViewportScissor(*cb_state, "VUID-vkCmdSetViewportWithCount-commandBuffer-04819", CMD_SETVIEWPORTWITHCOUNT);

    return skip;
}

bool CoreChecks::PreCallValidateCmdSetScissorWithCountEXT(VkCommandBuffer commandBuffer, uint32_t scissorCount,
                                                          const VkRect2D *pScissors) const {
    auto cb_state = GetRead<CMD_BUFFER_STATE>(commandBuffer);
    bool skip = false;
    skip = ValidateExtendedDynamicState(*cb_state, CMD_SETSCISSORWITHCOUNTEXT,
                                        enabled_features.extended_dynamic_state_features.extendedDynamicState ||
                                            enabled_features.shader_object_features.shaderObject,
                                        "VUID-vkCmdSetScissorWithCount-None-08590", "extendedDynamicState or shaderObject");
    skip |=
        ForbidInheritedViewportScissor(*cb_state, "VUID-vkCmdSetScissorWithCount-commandBuffer-04820", CMD_SETSCISSORWITHCOUNTEXT);

    return skip;
}

bool CoreChecks::PreCallValidateCmdSetScissorWithCount(VkCommandBuffer commandBuffer, uint32_t scissorCount,
                                                       const VkRect2D *pScissors) const {
    auto cb_state = GetRead<CMD_BUFFER_STATE>(commandBuffer);
    bool skip = false;
    skip = ValidateExtendedDynamicState(*cb_state, CMD_SETSCISSORWITHCOUNT, VK_TRUE, nullptr, nullptr);
    skip |= ForbidInheritedViewportScissor(*cb_state, "VUID-vkCmdSetScissorWithCount-commandBuffer-04820", CMD_SETSCISSORWITHCOUNT);

    return skip;
}

bool CoreChecks::PreCallValidateCmdSetDepthTestEnableEXT(VkCommandBuffer commandBuffer, VkBool32 depthTestEnable) const {
    auto cb_state = GetRead<CMD_BUFFER_STATE>(commandBuffer);
    return ValidateExtendedDynamicState(*cb_state, CMD_SETDEPTHTESTENABLEEXT,
                                        enabled_features.extended_dynamic_state_features.extendedDynamicState ||
                                            enabled_features.shader_object_features.shaderObject,
                                        "VUID-vkCmdSetDepthTestEnable-None-08516", "extendedDynamicState or shaderObject");
}

bool CoreChecks::PreCallValidateCmdSetDepthTestEnable(VkCommandBuffer commandBuffer, VkBool32 depthTestEnable) const {
    auto cb_state = GetRead<CMD_BUFFER_STATE>(commandBuffer);
    return ValidateExtendedDynamicState(*cb_state, CMD_SETDEPTHTESTENABLE, VK_TRUE, nullptr, nullptr);
}

bool CoreChecks::PreCallValidateCmdSetDepthWriteEnableEXT(VkCommandBuffer commandBuffer, VkBool32 depthWriteEnable) const {
    auto cb_state = GetRead<CMD_BUFFER_STATE>(commandBuffer);
    return ValidateExtendedDynamicState(*cb_state, CMD_SETDEPTHWRITEENABLEEXT,
                                        enabled_features.extended_dynamic_state_features.extendedDynamicState ||
                                            enabled_features.shader_object_features.shaderObject,
                                        "VUID-vkCmdSetDepthWriteEnable-None-08520", "extendedDynamicState or shaderObject");
}

bool CoreChecks::PreCallValidateCmdSetDepthWriteEnable(VkCommandBuffer commandBuffer, VkBool32 depthWriteEnable) const {
    auto cb_state = GetRead<CMD_BUFFER_STATE>(commandBuffer);
    return ValidateExtendedDynamicState(*cb_state, CMD_SETDEPTHWRITEENABLE, VK_TRUE, nullptr, nullptr);
}

bool CoreChecks::PreCallValidateCmdSetDepthCompareOpEXT(VkCommandBuffer commandBuffer, VkCompareOp depthCompareOp) const {
    auto cb_state = GetRead<CMD_BUFFER_STATE>(commandBuffer);
    return ValidateExtendedDynamicState(*cb_state, CMD_SETDEPTHCOMPAREOPEXT,
                                        enabled_features.extended_dynamic_state_features.extendedDynamicState ||
                                            enabled_features.shader_object_features.shaderObject,
                                        "VUID-vkCmdSetDepthCompareOp-None-08518", "extendedDynamicState or shaderObject");
}

bool CoreChecks::PreCallValidateCmdSetDepthCompareOp(VkCommandBuffer commandBuffer, VkCompareOp depthCompareOp) const {
    auto cb_state = GetRead<CMD_BUFFER_STATE>(commandBuffer);
    return ValidateExtendedDynamicState(*cb_state, CMD_SETDEPTHCOMPAREOP, VK_TRUE, nullptr, nullptr);
}

bool CoreChecks::PreCallValidateCmdSetDepthBoundsTestEnableEXT(VkCommandBuffer commandBuffer,
                                                               VkBool32 depthBoundsTestEnable) const {
    auto cb_state = GetRead<CMD_BUFFER_STATE>(commandBuffer);
    return ValidateExtendedDynamicState(*cb_state, CMD_SETDEPTHBOUNDSTESTENABLEEXT,
                                        enabled_features.extended_dynamic_state_features.extendedDynamicState ||
                                            enabled_features.shader_object_features.shaderObject,
                                        "VUID-vkCmdSetDepthBoundsTestEnable-None-08510", "extendedDynamicState or shaderObject");
}

bool CoreChecks::PreCallValidateCmdSetDepthBoundsTestEnable(VkCommandBuffer commandBuffer, VkBool32 depthBoundsTestEnable) const {
    auto cb_state = GetRead<CMD_BUFFER_STATE>(commandBuffer);
    return ValidateExtendedDynamicState(*cb_state, CMD_SETDEPTHBOUNDSTESTENABLE, VK_TRUE, nullptr, nullptr);
}

bool CoreChecks::PreCallValidateCmdSetStencilTestEnableEXT(VkCommandBuffer commandBuffer, VkBool32 stencilTestEnable) const {
    auto cb_state = GetRead<CMD_BUFFER_STATE>(commandBuffer);
    return ValidateExtendedDynamicState(*cb_state, CMD_SETSTENCILTESTENABLEEXT,
                                        enabled_features.extended_dynamic_state_features.extendedDynamicState ||
                                            enabled_features.shader_object_features.shaderObject,
                                        "VUID-vkCmdSetStencilTestEnable-None-08512", "extendedDynamicState or shaderObject");
}

bool CoreChecks::PreCallValidateCmdSetStencilTestEnable(VkCommandBuffer commandBuffer, VkBool32 stencilTestEnable) const {
    auto cb_state = GetRead<CMD_BUFFER_STATE>(commandBuffer);
    return ValidateExtendedDynamicState(*cb_state, CMD_SETSTENCILTESTENABLE, VK_TRUE, nullptr, nullptr);
}

bool CoreChecks::PreCallValidateCmdSetStencilOpEXT(VkCommandBuffer commandBuffer, VkStencilFaceFlags faceMask, VkStencilOp failOp,
                                                   VkStencilOp passOp, VkStencilOp depthFailOp, VkCompareOp compareOp) const {
    auto cb_state = GetRead<CMD_BUFFER_STATE>(commandBuffer);
    return ValidateExtendedDynamicState(*cb_state, CMD_SETSTENCILOPEXT,
                                        enabled_features.extended_dynamic_state_features.extendedDynamicState ||
                                            enabled_features.shader_object_features.shaderObject,
                                        "VUID-vkCmdSetStencilOp-None-08514", "extendedDynamicState or shaderObject");
}

bool CoreChecks::PreCallValidateCmdSetStencilOp(VkCommandBuffer commandBuffer, VkStencilFaceFlags faceMask, VkStencilOp failOp,
                                                VkStencilOp passOp, VkStencilOp depthFailOp, VkCompareOp compareOp) const {
    auto cb_state = GetRead<CMD_BUFFER_STATE>(commandBuffer);
    return ValidateExtendedDynamicState(*cb_state, CMD_SETSTENCILOP, VK_TRUE, nullptr, nullptr);
}

bool CoreChecks::PreCallValidateCmdSetTessellationDomainOriginEXT(VkCommandBuffer commandBuffer,
                                                                  VkTessellationDomainOrigin domainOrigin) const {
    auto cb_state = GetRead<CMD_BUFFER_STATE>(commandBuffer);
    return ValidateExtendedDynamicState(
        *cb_state, CMD_SETTESSELLATIONDOMAINORIGINEXT,
        enabled_features.extended_dynamic_state3_features.extendedDynamicState3TessellationDomainOrigin ||
            enabled_features.shader_object_features.shaderObject,
        "VUID-vkCmdSetTessellationDomainOriginEXT-None-08576",
        "extendedDynamicState3TessellationDomainOrigin or shaderObject");
}

bool CoreChecks::PreCallValidateCmdSetDepthClampEnableEXT(VkCommandBuffer commandBuffer, VkBool32 depthClampEnable) const {
    auto cb_state = GetRead<CMD_BUFFER_STATE>(commandBuffer);
    bool skip = false;
    skip |= ValidateExtendedDynamicState(*cb_state, CMD_SETDEPTHCLAMPENABLEEXT,
                                         enabled_features.extended_dynamic_state3_features.extendedDynamicState3DepthClampEnable ||
                                            enabled_features.shader_object_features.shaderObject,
                                         "VUID-vkCmdSetDepthClampEnableEXT-None-08582",
                                         "extendedDynamicState3DepthClampEnable or shaderObject");
    if (depthClampEnable != VK_FALSE && !enabled_features.core.depthClamp) {
        skip |= LogError(cb_state->Handle(), "VUID-vkCmdSetDepthClampEnableEXT-depthClamp-07449",
                         "vkCmdSetDepthClampEnableEXT(): depthClampEnable is VK_TRUE but the depthClamp feature is not enabled.");
    }
    return skip;
}

bool CoreChecks::PreCallValidateCmdSetPolygonModeEXT(VkCommandBuffer commandBuffer, VkPolygonMode polygonMode) const {
    auto cb_state = GetRead<CMD_BUFFER_STATE>(commandBuffer);
    bool skip = false;
    skip |= ValidateExtendedDynamicState(
        *cb_state, CMD_SETPOLYGONMODEEXT, enabled_features.extended_dynamic_state3_features.extendedDynamicState3PolygonMode ||
            enabled_features.shader_object_features.shaderObject,
        "VUID-vkCmdSetPolygonModeEXT-None-08566", "extendedDynamicState3PolygonMode or shaderObject");
    if ((polygonMode == VK_POLYGON_MODE_LINE || polygonMode == VK_POLYGON_MODE_POINT) && !enabled_features.core.fillModeNonSolid) {
        skip |= LogError(cb_state->Handle(), "VUID-vkCmdSetPolygonModeEXT-fillModeNonSolid-07424",
                         "vkCmdSetPolygonModeEXT(): polygonMode is %s but the "
                         "fillModeNonSolid feature is not enabled.",
                         string_VkPolygonMode(polygonMode));
    } else if (polygonMode == VK_POLYGON_MODE_FILL_RECTANGLE_NV && !IsExtEnabled(device_extensions.vk_nv_fill_rectangle)) {
        skip |= LogError(cb_state->Handle(), "VUID-vkCmdSetPolygonModeEXT-polygonMode-07425",
                         "vkCmdSetPolygonModeEXT(): polygonMode is VK_POLYGON_MODE_FILL_RECTANGLE_NV but the VK_NV_fill_rectangle "
                         "extension is not enabled.");
    }
    return skip;
}

bool CoreChecks::PreCallValidateCmdSetRasterizationSamplesEXT(VkCommandBuffer commandBuffer,
                                                              VkSampleCountFlagBits rasterizationSamples) const {
    auto cb_state = GetRead<CMD_BUFFER_STATE>(commandBuffer);
    return ValidateExtendedDynamicState(
        *cb_state, CMD_SETRASTERIZATIONSAMPLESEXT,
        enabled_features.extended_dynamic_state3_features.extendedDynamicState3RasterizationSamples ||
            enabled_features.shader_object_features.shaderObject,
        "VUID-vkCmdSetRasterizationSamplesEXT-None-08552",
        "extendedDynamicState3RasterizationSamples or shaderObject");
}

bool CoreChecks::PreCallValidateCmdSetSampleMaskEXT(VkCommandBuffer commandBuffer, VkSampleCountFlagBits samples,
                                                    const VkSampleMask *pSampleMask) const {
    auto cb_state = GetRead<CMD_BUFFER_STATE>(commandBuffer);
    return ValidateExtendedDynamicState(
        *cb_state, CMD_SETSAMPLEMASKEXT, enabled_features.extended_dynamic_state3_features.extendedDynamicState3SampleMask ||
            enabled_features.shader_object_features.shaderObject,
        "VUID-vkCmdSetSampleMaskEXT-None-08504", "extendedDynamicState3SampleMask or shaderObject");
}

bool CoreChecks::PreCallValidateCmdSetAlphaToCoverageEnableEXT(VkCommandBuffer commandBuffer,
                                                               VkBool32 alphaToCoverageEnable) const {
    auto cb_state = GetRead<CMD_BUFFER_STATE>(commandBuffer);
    return ValidateExtendedDynamicState(
        *cb_state, CMD_SETALPHATOCOVERAGEENABLEEXT,
        enabled_features.extended_dynamic_state3_features.extendedDynamicState3AlphaToCoverageEnable ||
            enabled_features.shader_object_features.shaderObject,
        "VUID-vkCmdSetAlphaToCoverageEnableEXT-None-08506",
        "extendedDynamicState3AlphaToCoverageEnable or shaderObject");
}

bool CoreChecks::PreCallValidateCmdSetAlphaToOneEnableEXT(VkCommandBuffer commandBuffer, VkBool32 alphaToOneEnable) const {
    auto cb_state = GetRead<CMD_BUFFER_STATE>(commandBuffer);
    bool skip = false;
    skip |= ValidateExtendedDynamicState(*cb_state, CMD_SETALPHATOONEENABLEEXT,
                                         enabled_features.extended_dynamic_state3_features.extendedDynamicState3AlphaToOneEnable ||
                                             enabled_features.shader_object_features.shaderObject,
                                         "VUID-vkCmdSetAlphaToOneEnableEXT-None-08508",
                                         "extendedDynamicState3AlphaToOneEnable or shaderObject");
    if (alphaToOneEnable != VK_FALSE && !enabled_features.core.alphaToOne) {
        skip |= LogError(cb_state->Handle(), "VUID-vkCmdSetAlphaToOneEnableEXT-alphaToOne-07607",
                         "vkCmdSetAlphaToOneEnableEXT(): alphaToOneEnable is VK_TRUE but the alphaToOne feature is not enabled.");
    }
    return skip;
}

bool CoreChecks::PreCallValidateCmdSetLogicOpEnableEXT(VkCommandBuffer commandBuffer, VkBool32 logicOpEnable) const {
    auto cb_state = GetRead<CMD_BUFFER_STATE>(commandBuffer);
    bool skip = false;
    skip |= ValidateExtendedDynamicState(
        *cb_state, CMD_SETLOGICOPENABLEEXT, enabled_features.extended_dynamic_state3_features.extendedDynamicState3LogicOpEnable ||
            enabled_features.shader_object_features.shaderObject,
        "VUID-vkCmdSetLogicOpEnableEXT-None-08542",
        "extendedDynamicState3LogicOpEnable or shaderObject");
    if (logicOpEnable != VK_FALSE && !enabled_features.core.logicOp) {
        skip |= LogError(cb_state->Handle(), "VUID-vkCmdSetLogicOpEnableEXT-logicOp-07366",
                         "vkCmdSetLogicOpEnableEXT(): logicOpEnable is VK_TRUE but the logicOp feature is not enabled.");
    }
    return skip;
}

bool CoreChecks::PreCallValidateCmdSetColorBlendEnableEXT(VkCommandBuffer commandBuffer, uint32_t firstAttachment,
                                                          uint32_t attachmentCount, const VkBool32 *pColorBlendEnables) const {
    auto cb_state = GetRead<CMD_BUFFER_STATE>(commandBuffer);
    return ValidateExtendedDynamicState(*cb_state, CMD_SETCOLORBLENDENABLEEXT,
                                        enabled_features.extended_dynamic_state3_features.extendedDynamicState3ColorBlendEnable ||
                                            enabled_features.shader_object_features.shaderObject,
                                        "VUID-vkCmdSetColorBlendEnableEXT-None-08536",
                                        "extendedDynamicState3ColorBlendEnable or shaderObject");
}

bool CoreChecks::PreCallValidateCmdSetColorBlendEquationEXT(VkCommandBuffer commandBuffer, uint32_t firstAttachment,
                                                            uint32_t attachmentCount,
                                                            const VkColorBlendEquationEXT *pColorBlendEquations) const {
    auto cb_state = GetRead<CMD_BUFFER_STATE>(commandBuffer);
    bool skip = false;
    skip |= ValidateExtendedDynamicState(
        *cb_state, CMD_SETCOLORBLENDEQUATIONEXT,
        enabled_features.extended_dynamic_state3_features.extendedDynamicState3ColorBlendEquation ||
            enabled_features.shader_object_features.shaderObject,
        "VUID-vkCmdSetColorBlendEquationEXT-None-08538",
        "extendedDynamicState3ColorBlendEquation or shaderObject");
    for (uint32_t attachment = 0U; attachment < attachmentCount; ++attachment) {
        VkColorBlendEquationEXT const &equation = pColorBlendEquations[attachment];
        if (!enabled_features.core.dualSrcBlend) {
            if (IsSecondaryColorInputBlendFactor(equation.srcColorBlendFactor)) {
                skip |= LogError(cb_state->Handle(), "VUID-VkColorBlendEquationEXT-dualSrcBlend-07357",
                                 "vkCmdSetColorBlendEquationEXT(): pColorBlendEquations[%u].srcColorBlendFactor is %s but the "
                                 "dualSrcBlend feature is not enabled.",
                                 attachment, string_VkBlendFactor(equation.srcColorBlendFactor));
            }
            if (IsSecondaryColorInputBlendFactor(equation.dstColorBlendFactor)) {
                skip |= LogError(cb_state->Handle(), "VUID-VkColorBlendEquationEXT-dualSrcBlend-07358",
                                 "vkCmdSetColorBlendEquationEXT(): pColorBlendEquations[%u].dstColorBlendFactor is %s but the "
                                 "dualSrcBlend feature is not enabled.",
                                 attachment, string_VkBlendFactor(equation.dstColorBlendFactor));
            }
            if (IsSecondaryColorInputBlendFactor(equation.srcAlphaBlendFactor)) {
                skip |= LogError(cb_state->Handle(), "VUID-VkColorBlendEquationEXT-dualSrcBlend-07359",
                                 "vkCmdSetColorBlendEquationEXT(): pColorBlendEquations[%u].srcAlphaBlendFactor is %s but the "
                                 "dualSrcBlend feature is not enabled.",
                                 attachment, string_VkBlendFactor(equation.srcAlphaBlendFactor));
            }
            if (IsSecondaryColorInputBlendFactor(equation.dstAlphaBlendFactor)) {
                skip |= LogError(cb_state->Handle(), "VUID-VkColorBlendEquationEXT-dualSrcBlend-07360",
                                 "vkCmdSetColorBlendEquationEXT(): pColorBlendEquations[%u].dstAlphaBlendFactor is %s but the "
                                 "dualSrcBlend feature is not enabled.",
                                 attachment, string_VkBlendFactor(equation.dstAlphaBlendFactor));
            }
        }
        if (IsAdvanceBlendOperation(equation.colorBlendOp) || IsAdvanceBlendOperation(equation.alphaBlendOp)) {
            skip |= LogError(cb_state->Handle(), "VUID-VkColorBlendEquationEXT-colorBlendOp-07361",
                             "vkCmdSetColorBlendEquationEXT(): pColorBlendEquations[%u].colorBlendOp and "
                             "pColorBlendEquations[%u].alphaBlendOp must not be an advanced blending operation.",
                             attachment, attachment);
        }
        if (IsExtEnabled(device_extensions.vk_khr_portability_subset) &&
            !enabled_features.portability_subset_features.constantAlphaColorBlendFactors) {
            if (equation.srcColorBlendFactor == VK_BLEND_FACTOR_CONSTANT_ALPHA ||
                equation.srcColorBlendFactor == VK_BLEND_FACTOR_ONE_MINUS_CONSTANT_ALPHA) {
                skip |= LogError(cb_state->Handle(), "VUID-VkColorBlendEquationEXT-constantAlphaColorBlendFactors-07362",
                                 "vkCmdSetColorBlendEquationEXT(): pColorBlendEquations[%u].srcColorBlendFactor must not be %s "
                                 "when constantAlphaColorBlendFactors is not supported.",
                                 attachment, string_VkBlendFactor(equation.srcColorBlendFactor));
            }
            if (equation.dstColorBlendFactor == VK_BLEND_FACTOR_CONSTANT_ALPHA ||
                equation.dstColorBlendFactor == VK_BLEND_FACTOR_ONE_MINUS_CONSTANT_ALPHA) {
                skip |= LogError(cb_state->Handle(), "VUID-VkColorBlendEquationEXT-constantAlphaColorBlendFactors-07363",
                                 "vkCmdSetColorBlendEquationEXT(): pColorBlendEquations[%u].dstColorBlendFactor must not be %s "
                                 "constantAlphaColorBlendFactors is not supported.",
                                 attachment, string_VkBlendFactor(equation.dstColorBlendFactor));
            }
        }
    }
    return skip;
}

bool CoreChecks::PreCallValidateCmdSetColorWriteMaskEXT(VkCommandBuffer commandBuffer, uint32_t firstAttachment,
                                                        uint32_t attachmentCount,
                                                        const VkColorComponentFlags *pColorWriteMasks) const {
    auto cb_state = GetRead<CMD_BUFFER_STATE>(commandBuffer);
    return ValidateExtendedDynamicState(
        *cb_state, CMD_SETCOLORWRITEMASKEXT,
        enabled_features.extended_dynamic_state3_features.extendedDynamicState3ColorWriteMask ||
            enabled_features.shader_object_features.shaderObject,
        "VUID-vkCmdSetColorWriteMaskEXT-None-08540",
        "extendedDynamicState3ColorWriteMask or shaderObject");
}

bool CoreChecks::PreCallValidateCmdSetRasterizationStreamEXT(VkCommandBuffer commandBuffer, uint32_t rasterizationStream) const {
    auto cb_state = GetRead<CMD_BUFFER_STATE>(commandBuffer);
    bool skip = false;
    skip |= ValidateExtendedDynamicState(
        *cb_state, CMD_SETRASTERIZATIONSTREAMEXT,
        enabled_features.extended_dynamic_state3_features.extendedDynamicState3RasterizationStream ||
            enabled_features.shader_object_features.shaderObject,
        "VUID-vkCmdSetRasterizationStreamEXT-None-08550",
        "extendedDynamicState3RasterizationStream or shaderObject");
    if (!enabled_features.transform_feedback_features.transformFeedback) {
        skip |= LogError(cb_state->Handle(), "VUID-vkCmdSetRasterizationStreamEXT-transformFeedback-07411",
                         "vkCmdSetRasterizationStreamEXT(): the transformFeedback feature is not enabled.");
    }
    if (rasterizationStream >= phys_dev_ext_props.transform_feedback_props.maxTransformFeedbackStreams) {
        skip |= LogError(cb_state->Handle(), "VUID-vkCmdSetRasterizationStreamEXT-rasterizationStream-07412",
                         "vkCmdSetRasterizationStreamEXT(): rasterizationStream (%" PRIu32
                         ") must be less than maxTransformFeedbackStreams (%" PRIu32 ").",
                         rasterizationStream, phys_dev_ext_props.transform_feedback_props.maxTransformFeedbackStreams);
    }
    if (rasterizationStream != 0U &&
        phys_dev_ext_props.transform_feedback_props.transformFeedbackRasterizationStreamSelect == VK_FALSE) {
        skip |= LogError(cb_state->Handle(), "VUID-vkCmdSetRasterizationStreamEXT-rasterizationStream-07413",
                         "vkCmdSetRasterizationStreamEXT(): rasterizationStream is non-zero but "
                         "transformFeedbackRasterizationStreamSelect is not supported.");
    }
    return skip;
}

bool CoreChecks::PreCallValidateCmdSetConservativeRasterizationModeEXT(
    VkCommandBuffer commandBuffer, VkConservativeRasterizationModeEXT conservativeRasterizationMode) const {
    auto cb_state = GetRead<CMD_BUFFER_STATE>(commandBuffer);
    return ValidateExtendedDynamicState(
        *cb_state, CMD_SETCONSERVATIVERASTERIZATIONMODEEXT,
        enabled_features.extended_dynamic_state3_features.extendedDynamicState3ConservativeRasterizationMode ||
            enabled_features.shader_object_features.shaderObject,
        "VUID-vkCmdSetConservativeRasterizationModeEXT-None-08570",
        "extendedDynamicState3ConservativeRasterizationMode or shaderObject");
}

bool CoreChecks::PreCallValidateCmdSetExtraPrimitiveOverestimationSizeEXT(VkCommandBuffer commandBuffer,
                                                                          float extraPrimitiveOverestimationSize) const {
    auto cb_state = GetRead<CMD_BUFFER_STATE>(commandBuffer);
    bool skip = false;
    skip |= ValidateExtendedDynamicState(
        *cb_state, CMD_SETEXTRAPRIMITIVEOVERESTIMATIONSIZEEXT,
        enabled_features.extended_dynamic_state3_features.extendedDynamicState3ExtraPrimitiveOverestimationSize ||
            enabled_features.shader_object_features.shaderObject,
        "VUID-vkCmdSetExtraPrimitiveOverestimationSizeEXT-None-08572",
        "extendedDynamicState3ExtraPrimitiveOverestimationSize or shaderObject");
    if (extraPrimitiveOverestimationSize < 0.0f ||
        extraPrimitiveOverestimationSize >
            phys_dev_ext_props.conservative_rasterization_props.maxExtraPrimitiveOverestimationSize) {
        skip |=
            LogError(cb_state->Handle(), "VUID-vkCmdSetExtraPrimitiveOverestimationSizeEXT-extraPrimitiveOverestimationSize-07428",
                     "vkCmdSetExtraPrimitiveOverestimationSizeEXT(): extraPrimitiveOverestimationSize (%f) must be less then zero "
                     "or greater "
                     "than maxExtraPrimitiveOverestimationSize (%f).",
                     extraPrimitiveOverestimationSize,
                     phys_dev_ext_props.conservative_rasterization_props.maxExtraPrimitiveOverestimationSize);
    }
    return skip;
}

bool CoreChecks::PreCallValidateCmdSetDepthClipEnableEXT(VkCommandBuffer commandBuffer, VkBool32 depthClipEnable) const {
    auto cb_state = GetRead<CMD_BUFFER_STATE>(commandBuffer);
    bool skip = false;
    skip |= ValidateExtendedDynamicState(*cb_state, CMD_SETDEPTHCLIPENABLEEXT,
                                         enabled_features.extended_dynamic_state3_features.extendedDynamicState3DepthClipEnable ||
                                             enabled_features.shader_object_features.shaderObject,
                                         "VUID-vkCmdSetDepthClipEnableEXT-None-08584",
                                         "extendedDynamicState3DepthClipEnable or shaderObject");
    if (!enabled_features.depth_clip_enable_features.depthClipEnable) {
        skip |= LogError(cb_state->Handle(), "VUID-vkCmdSetDepthClipEnableEXT-depthClipEnable-07451",
                         "vkCmdSetDepthClipEnableEXT(): the depthClipEnable feature is not enabled.");
    }
    return skip;
}

bool CoreChecks::PreCallValidateCmdSetSampleLocationsEnableEXT(VkCommandBuffer commandBuffer,
                                                               VkBool32 sampleLocationsEnable) const {
    auto cb_state = GetRead<CMD_BUFFER_STATE>(commandBuffer);
    return ValidateExtendedDynamicState(
        *cb_state, CMD_SETSAMPLELOCATIONSENABLEEXT,
        enabled_features.extended_dynamic_state3_features.extendedDynamicState3SampleLocationsEnable ||
            enabled_features.shader_object_features.shaderObject,
        "VUID-vkCmdSetSampleLocationsEnableEXT-None-08554",
        "extendedDynamicState3SampleLocationsEnable or shaderObject");
}

bool CoreChecks::PreCallValidateCmdSetColorBlendAdvancedEXT(VkCommandBuffer commandBuffer, uint32_t firstAttachment,
                                                            uint32_t attachmentCount,
                                                            const VkColorBlendAdvancedEXT *pColorBlendAdvanced) const {
    auto cb_state = GetRead<CMD_BUFFER_STATE>(commandBuffer);
    bool skip = false;
    skip |= ValidateExtendedDynamicState(
        *cb_state, CMD_SETCOLORBLENDADVANCEDEXT,
        enabled_features.extended_dynamic_state3_features.extendedDynamicState3ColorBlendAdvanced ||
            enabled_features.shader_object_features.shaderObject,
        "VUID-vkCmdSetColorBlendAdvancedEXT-None-08592",
        "extendedDynamicState3ColorBlendAdvanced or shaderObject");
    for (uint32_t attachment = 0U; attachment < attachmentCount; ++attachment) {
        VkColorBlendAdvancedEXT const &advanced = pColorBlendAdvanced[attachment];
        if (advanced.srcPremultiplied != VK_FALSE &&
            !phys_dev_ext_props.blend_operation_advanced_props.advancedBlendNonPremultipliedSrcColor) {
            skip |= LogError(cb_state->Handle(), "VUID-VkColorBlendAdvancedEXT-srcPremultiplied-07505",
                             "vkCmdSetColorBlendAdvancedEXT(): pColorBlendAdvanced[%u].srcPremultiplied must not be VK_TRUE when "
                             "advancedBlendNonPremultipliedSrcColor is not supported.",
                             attachment);
        }
        if (advanced.dstPremultiplied != VK_FALSE &&
            !phys_dev_ext_props.blend_operation_advanced_props.advancedBlendNonPremultipliedDstColor) {
            skip |= LogError(cb_state->Handle(), "VUID-VkColorBlendAdvancedEXT-dstPremultiplied-07506",
                             "vkCmdSetColorBlendAdvancedEXT(): pColorBlendAdvanced[%u].dstPremultiplied must not be VK_TRUE when "
                             "advancedBlendNonPremultipliedDstColor is not supported.",
                             attachment);
        }
        if (advanced.blendOverlap != VK_BLEND_OVERLAP_UNCORRELATED_EXT &&
            !phys_dev_ext_props.blend_operation_advanced_props.advancedBlendCorrelatedOverlap) {
            skip |= LogError(cb_state->Handle(), "VUID-VkColorBlendAdvancedEXT-blendOverlap-07507",
                             "vkCmdSetColorBlendAdvancedEXT(): pColorBlendAdvanced[%u].blendOverlap must be "
                             "VK_BLEND_OVERLAP_UNCORRELATED_EXT when advancedBlendCorrelatedOverlap is not supported.",
                             attachment);
        }
    }
    return skip;
}

bool CoreChecks::PreCallValidateCmdSetProvokingVertexModeEXT(VkCommandBuffer commandBuffer,
                                                             VkProvokingVertexModeEXT provokingVertexMode) const {
    auto cb_state = GetRead<CMD_BUFFER_STATE>(commandBuffer);
    bool skip = false;
    skip |= ValidateExtendedDynamicState(
        *cb_state, CMD_SETPROVOKINGVERTEXMODEEXT,
        enabled_features.extended_dynamic_state3_features.extendedDynamicState3ProvokingVertexMode ||
            enabled_features.shader_object_features.shaderObject,
        "VUID-vkCmdSetProvokingVertexModeEXT-None-08580",
        "extendedDynamicState3ProvokingVertexMode or shaderObject");
    if (provokingVertexMode == VK_PROVOKING_VERTEX_MODE_LAST_VERTEX_EXT &&
        enabled_features.provoking_vertex_features.provokingVertexLast == VK_FALSE) {
        skip |= LogError(cb_state->Handle(), "VUID-vkCmdSetProvokingVertexModeEXT-provokingVertexMode-07447",
                         "vkCmdSetProvokingVertexModeEXT(): provokingVertexMode is VK_PROVOKING_VERTEX_MODE_LAST_VERTEX_EXT but "
                         "the provokingVertexLast feature is not enabled.");
    }
    return skip;
}

bool CoreChecks::PreCallValidateCmdSetLineRasterizationModeEXT(VkCommandBuffer commandBuffer,
                                                               VkLineRasterizationModeEXT lineRasterizationMode) const {
    auto cb_state = GetRead<CMD_BUFFER_STATE>(commandBuffer);
    bool skip = false;
    skip |=
        ValidateExtendedDynamicState(*cb_state, CMD_SETLINERASTERIZATIONMODEEXT,
                                     enabled_features.extended_dynamic_state3_features.extendedDynamicState3LineRasterizationMode ||
                                         enabled_features.shader_object_features.shaderObject,
                                     "VUID-vkCmdSetLineRasterizationModeEXT-None-08558",
                                     "extendedDynamicState3LineRasterizationMode or shaderObject");
    if (lineRasterizationMode == VK_LINE_RASTERIZATION_MODE_RECTANGULAR_EXT &&
        !enabled_features.line_rasterization_features.rectangularLines) {
        skip |= LogError(cb_state->Handle(), "VUID-vkCmdSetLineRasterizationModeEXT-lineRasterizationMode-07418",
                         "vkCmdSetLineRasterizationModeEXT(): lineRasterizationMode is VK_LINE_RASTERIZATION_MODE_RECTANGULAR_EXT "
                         "but the rectangularLines feature is not enabled.");
    } else if (lineRasterizationMode == VK_LINE_RASTERIZATION_MODE_BRESENHAM_EXT &&
               !enabled_features.line_rasterization_features.bresenhamLines) {
        skip |= LogError(cb_state->Handle(), "VUID-vkCmdSetLineRasterizationModeEXT-lineRasterizationMode-07419",
                         "vkCmdSetLineRasterizationModeEXT(): lineRasterizationMode is VK_LINE_RASTERIZATION_MODE_BRESENHAM_EXT "
                         "but the bresenhamLines feature is not enabled.");
    } else if (lineRasterizationMode == VK_LINE_RASTERIZATION_MODE_RECTANGULAR_SMOOTH_EXT &&
               !enabled_features.line_rasterization_features.smoothLines) {
        skip |= LogError(cb_state->Handle(), "VUID-vkCmdSetLineRasterizationModeEXT-lineRasterizationMode-07420",
                         "vkCmdSetLineRasterizationModeEXT(): lineRasterizationMode is "
                         "VK_LINE_RASTERIZATION_MODE_RECTANGULAR_SMOOTH_EXT but the smoothLines feature is not enabled.");
    }
    return skip;
}

bool CoreChecks::PreCallValidateCmdSetLineStippleEnableEXT(VkCommandBuffer commandBuffer, VkBool32 stippledLineEnable) const {
    auto cb_state = GetRead<CMD_BUFFER_STATE>(commandBuffer);
    return ValidateExtendedDynamicState(*cb_state, CMD_SETLINESTIPPLEENABLEEXT,
                                        enabled_features.extended_dynamic_state3_features.extendedDynamicState3LineStippleEnable ||
                                            enabled_features.shader_object_features.shaderObject,
                                        "VUID-vkCmdSetLineStippleEnableEXT-None-08560",
                                        "extendedDynamicState3LineStippleEnable or shaderObject");
}

bool CoreChecks::PreCallValidateCmdSetDepthClipNegativeOneToOneEXT(VkCommandBuffer commandBuffer, VkBool32 negativeOneToOne) const {
    auto cb_state = GetRead<CMD_BUFFER_STATE>(commandBuffer);
    bool skip = false;
    skip |= ValidateExtendedDynamicState(
        *cb_state, CMD_SETDEPTHCLIPNEGATIVEONETOONEEXT,
        enabled_features.extended_dynamic_state3_features.extendedDynamicState3DepthClipNegativeOneToOne ||
            enabled_features.shader_object_features.shaderObject,
        "VUID-vkCmdSetDepthClipNegativeOneToOneEXT-None-08586",
        "extendedDynamicState3DepthClipNegativeOneToOne or shaderObject");
    if (enabled_features.depth_clip_control_features.depthClipControl == VK_FALSE) {
        skip |= LogError(cb_state->Handle(), "VUID-vkCmdSetDepthClipNegativeOneToOneEXT-depthClipControl-07453",
                         "vkCmdSetDepthClipNegativeOneToOneEXT(): the depthClipControl feature is not enabled.");
    }
    return skip;
}

bool CoreChecks::PreCallValidateCmdSetViewportWScalingEnableNV(VkCommandBuffer commandBuffer,
                                                               VkBool32 viewportWScalingEnable) const {
    auto cb_state = GetRead<CMD_BUFFER_STATE>(commandBuffer);
    return ValidateExtendedDynamicState(
        *cb_state, CMD_SETVIEWPORTWSCALINGENABLENV,
        enabled_features.extended_dynamic_state3_features.extendedDynamicState3ViewportWScalingEnable ||
            enabled_features.shader_object_features.shaderObject,
        "VUID-vkCmdSetViewportWScalingEnableNV-None-08594",
        "extendedDynamicState3ViewportWScalingEnable or shaderObject");
}

bool CoreChecks::PreCallValidateCmdSetViewportSwizzleNV(VkCommandBuffer commandBuffer, uint32_t firstViewport,
                                                        uint32_t viewportCount,
                                                        const VkViewportSwizzleNV *pViewportSwizzles) const {
    auto cb_state = GetRead<CMD_BUFFER_STATE>(commandBuffer);
    return ValidateExtendedDynamicState(
        *cb_state, CMD_SETVIEWPORTSWIZZLENV,
        enabled_features.extended_dynamic_state3_features.extendedDynamicState3ViewportSwizzle ||
            enabled_features.shader_object_features.shaderObject,
        "VUID-vkCmdSetViewportSwizzleNV-None-08578",
        "extendedDynamicState3ViewportSwizzle or shaderObject");
}

bool CoreChecks::PreCallValidateCmdSetCoverageToColorEnableNV(VkCommandBuffer commandBuffer, VkBool32 coverageToColorEnable) const {
    auto cb_state = GetRead<CMD_BUFFER_STATE>(commandBuffer);
    return ValidateExtendedDynamicState(
        *cb_state, CMD_SETCOVERAGETOCOLORENABLENV,
        enabled_features.extended_dynamic_state3_features.extendedDynamicState3CoverageToColorEnable ||
            enabled_features.shader_object_features.shaderObject,
        "VUID-vkCmdSetCoverageToColorEnableNV-None-08524",
        "extendedDynamicState3CoverageToColorEnable or shaderObject");
}

bool CoreChecks::PreCallValidateCmdSetCoverageToColorLocationNV(VkCommandBuffer commandBuffer,
                                                                uint32_t coverageToColorLocation) const {
    auto cb_state = GetRead<CMD_BUFFER_STATE>(commandBuffer);
    return ValidateExtendedDynamicState(
        *cb_state, CMD_SETCOVERAGETOCOLORLOCATIONNV,
        enabled_features.extended_dynamic_state3_features.extendedDynamicState3CoverageToColorLocation ||
            enabled_features.shader_object_features.shaderObject,
        "VUID-vkCmdSetCoverageToColorLocationNV-None-08526",
        "extendedDynamicState3CoverageToColorLocation or shaderObject");
}

bool CoreChecks::PreCallValidateCmdSetCoverageModulationModeNV(VkCommandBuffer commandBuffer,
                                                               VkCoverageModulationModeNV coverageModulationMode) const {
    auto cb_state = GetRead<CMD_BUFFER_STATE>(commandBuffer);
    return ValidateExtendedDynamicState(
        *cb_state, CMD_SETCOVERAGEMODULATIONMODENV,
        enabled_features.extended_dynamic_state3_features.extendedDynamicState3CoverageModulationMode ||
            enabled_features.shader_object_features.shaderObject,
        "VUID-vkCmdSetCoverageModulationModeNV-None-08530",
        "extendedDynamicState3CoverageModulationMode or shaderObject");
}

bool CoreChecks::PreCallValidateCmdSetCoverageModulationTableEnableNV(VkCommandBuffer commandBuffer,
                                                                      VkBool32 coverageModulationTableEnable) const {
    auto cb_state = GetRead<CMD_BUFFER_STATE>(commandBuffer);
    return ValidateExtendedDynamicState(
        *cb_state, CMD_SETCOVERAGEMODULATIONTABLEENABLENV,
        enabled_features.extended_dynamic_state3_features.extendedDynamicState3CoverageModulationTableEnable ||
            enabled_features.shader_object_features.shaderObject,
        "VUID-vkCmdSetCoverageModulationTableEnableNV-None-08532",
        "extendedDynamicState3CoverageModulationTableEnable or shaderObject");
}

bool CoreChecks::PreCallValidateCmdSetCoverageModulationTableNV(VkCommandBuffer commandBuffer,
                                                                uint32_t coverageModulationTableCount,
                                                                const float *pCoverageModulationTable) const {
    auto cb_state = GetRead<CMD_BUFFER_STATE>(commandBuffer);
    return ValidateExtendedDynamicState(
        *cb_state, CMD_SETCOVERAGEMODULATIONTABLENV,
        enabled_features.extended_dynamic_state3_features.extendedDynamicState3CoverageModulationTable ||
            enabled_features.shader_object_features.shaderObject,
        "VUID-vkCmdSetCoverageModulationTableNV-None-08534",
        "extendedDynamicState3CoverageModulationTable or shaderObject");
}

bool CoreChecks::PreCallValidateCmdSetShadingRateImageEnableNV(VkCommandBuffer commandBuffer,
                                                               VkBool32 shadingRateImageEnable) const {
    auto cb_state = GetRead<CMD_BUFFER_STATE>(commandBuffer);
    return ValidateExtendedDynamicState(
        *cb_state, CMD_SETSHADINGRATEIMAGEENABLENV,
        enabled_features.extended_dynamic_state3_features.extendedDynamicState3ShadingRateImageEnable ||
            enabled_features.shader_object_features.shaderObject,
        "VUID-vkCmdSetShadingRateImageEnableNV-None-08556",
        "extendedDynamicState3ShadingRateImageEnable or shaderObject");
}

bool CoreChecks::PreCallValidateCmdSetRepresentativeFragmentTestEnableNV(VkCommandBuffer commandBuffer,
                                                                         VkBool32 representativeFragmentTestEnable) const {
    auto cb_state = GetRead<CMD_BUFFER_STATE>(commandBuffer);
    return ValidateExtendedDynamicState(
        *cb_state, CMD_SETREPRESENTATIVEFRAGMENTTESTENABLENV,
        enabled_features.extended_dynamic_state3_features.extendedDynamicState3RepresentativeFragmentTestEnable ||
            enabled_features.shader_object_features.shaderObject,
        "VUID-vkCmdSetRepresentativeFragmentTestEnableNV-None-08522",
        "extendedDynamicState3RepresentativeFragmentTestEnable or shaderObject");
}

bool CoreChecks::PreCallValidateCmdSetCoverageReductionModeNV(VkCommandBuffer commandBuffer,
                                                              VkCoverageReductionModeNV coverageReductionMode) const {
    auto cb_state = GetRead<CMD_BUFFER_STATE>(commandBuffer);
    return ValidateExtendedDynamicState(
        *cb_state, CMD_SETCOVERAGEREDUCTIONMODENV,
        enabled_features.extended_dynamic_state3_features.extendedDynamicState3CoverageReductionMode ||
            enabled_features.shader_object_features.shaderObject,
        "VUID-vkCmdSetCoverageReductionModeNV-None-08528",
        "extendedDynamicState3CoverageReductionMode or shaderObject");
}

bool CoreChecks::PreCallValidateCreateEvent(VkDevice device, const VkEventCreateInfo *pCreateInfo,
                                            const VkAllocationCallbacks *pAllocator, VkEvent *pEvent) const {
    bool skip = false;
    if (IsExtEnabled(device_extensions.vk_khr_portability_subset)) {
        if (VK_FALSE == enabled_features.portability_subset_features.events) {
            skip |= LogError(device, "VUID-vkCreateEvent-events-04468",
                             "vkCreateEvent: events are not supported via VK_KHR_portability_subset");
        }
    }
    return skip;
}

bool CoreChecks::PreCallValidateCmdSetFragmentShadingRateKHR(VkCommandBuffer commandBuffer, const VkExtent2D *pFragmentSize,
                                                             const VkFragmentShadingRateCombinerOpKHR combinerOps[2]) const {
    auto cb_state = GetRead<CMD_BUFFER_STATE>(commandBuffer);
    const char *cmd_name = "vkCmdSetFragmentShadingRateKHR()";
    bool skip = false;
    skip |=
        ValidateExtendedDynamicState(*cb_state, CMD_SETFRAGMENTSHADINGRATEKHR,
                                     enabled_features.fragment_shading_rate_features.pipelineFragmentShadingRate ||
                                         enabled_features.fragment_shading_rate_features.primitiveFragmentShadingRate ||
                                         enabled_features.fragment_shading_rate_features.attachmentFragmentShadingRate,
                                     "VUID-vkCmdSetFragmentShadingRateKHR-pipelineFragmentShadingRate-04509",
                                     "pipelineFragmentShadingRate, primitiveFragmentShadingRate, or attachmentFragmentShadingRate");

    if (!enabled_features.fragment_shading_rate_features.pipelineFragmentShadingRate && pFragmentSize->width != 1) {
        skip |= LogError(cb_state->commandBuffer(), "VUID-vkCmdSetFragmentShadingRateKHR-pipelineFragmentShadingRate-04507",
                         "vkCmdSetFragmentShadingRateKHR: Pipeline fragment width of %u has been specified in %s, but "
                         "pipelineFragmentShadingRate is not enabled",
                         pFragmentSize->width, cmd_name);
    }

    if (!enabled_features.fragment_shading_rate_features.pipelineFragmentShadingRate && pFragmentSize->height != 1) {
        skip |= LogError(cb_state->commandBuffer(), "VUID-vkCmdSetFragmentShadingRateKHR-pipelineFragmentShadingRate-04508",
                         "vkCmdSetFragmentShadingRateKHR: Pipeline fragment height of %u has been specified in %s, but "
                         "pipelineFragmentShadingRate is not enabled",
                         pFragmentSize->height, cmd_name);
    }

    if (!enabled_features.fragment_shading_rate_features.primitiveFragmentShadingRate &&
        combinerOps[0] != VK_FRAGMENT_SHADING_RATE_COMBINER_OP_KEEP_KHR) {
        skip |= LogError(cb_state->commandBuffer(), "VUID-vkCmdSetFragmentShadingRateKHR-primitiveFragmentShadingRate-04510",
                         "vkCmdSetFragmentShadingRateKHR: First combiner operation of %s has been specified in %s, but "
                         "primitiveFragmentShadingRate is not enabled",
                         string_VkFragmentShadingRateCombinerOpKHR(combinerOps[0]), cmd_name);
    }

    if (!enabled_features.fragment_shading_rate_features.attachmentFragmentShadingRate &&
        combinerOps[1] != VK_FRAGMENT_SHADING_RATE_COMBINER_OP_KEEP_KHR) {
        skip |= LogError(cb_state->commandBuffer(), "VUID-vkCmdSetFragmentShadingRateKHR-attachmentFragmentShadingRate-04511",
                         "vkCmdSetFragmentShadingRateKHR: Second combiner operation of %s has been specified in %s, but "
                         "attachmentFragmentShadingRate is not enabled",
                         string_VkFragmentShadingRateCombinerOpKHR(combinerOps[1]), cmd_name);
    }

    if (!phys_dev_ext_props.fragment_shading_rate_props.fragmentShadingRateNonTrivialCombinerOps &&
        (combinerOps[0] != VK_FRAGMENT_SHADING_RATE_COMBINER_OP_KEEP_KHR &&
         combinerOps[0] != VK_FRAGMENT_SHADING_RATE_COMBINER_OP_REPLACE_KHR)) {
        skip |= LogError(cb_state->commandBuffer(), "VUID-vkCmdSetFragmentShadingRateKHR-fragmentSizeNonTrivialCombinerOps-04512",
                         "vkCmdSetFragmentShadingRateKHR: First combiner operation of %s has been specified in %s, but "
                         "fragmentShadingRateNonTrivialCombinerOps is "
                         "not supported",
                         string_VkFragmentShadingRateCombinerOpKHR(combinerOps[0]), cmd_name);
    }

    if (!phys_dev_ext_props.fragment_shading_rate_props.fragmentShadingRateNonTrivialCombinerOps &&
        (combinerOps[1] != VK_FRAGMENT_SHADING_RATE_COMBINER_OP_KEEP_KHR &&
         combinerOps[1] != VK_FRAGMENT_SHADING_RATE_COMBINER_OP_REPLACE_KHR)) {
        skip |= LogError(cb_state->commandBuffer(), "VUID-vkCmdSetFragmentShadingRateKHR-fragmentSizeNonTrivialCombinerOps-04512",
                         "vkCmdSetFragmentShadingRateKHR: Second combiner operation of %s has been specified in %s, but "
                         "fragmentShadingRateNonTrivialCombinerOps "
                         "is not supported",
                         string_VkFragmentShadingRateCombinerOpKHR(combinerOps[1]), cmd_name);
    }

    if (pFragmentSize->width == 0) {
        skip |= LogError(cb_state->commandBuffer(), "VUID-vkCmdSetFragmentShadingRateKHR-pFragmentSize-04513",
                         "vkCmdSetFragmentShadingRateKHR: Fragment width of %u has been specified in %s.", pFragmentSize->width,
                         cmd_name);
    }

    if (pFragmentSize->height == 0) {
        skip |= LogError(cb_state->commandBuffer(), "VUID-vkCmdSetFragmentShadingRateKHR-pFragmentSize-04514",
                         "vkCmdSetFragmentShadingRateKHR: Fragment height of %u has been specified in %s.", pFragmentSize->height,
                         cmd_name);
    }

    if (pFragmentSize->width != 0 && !IsPowerOfTwo(pFragmentSize->width)) {
        skip |= LogError(cb_state->commandBuffer(), "VUID-vkCmdSetFragmentShadingRateKHR-pFragmentSize-04515",
                         "vkCmdSetFragmentShadingRateKHR: Non-power-of-two fragment width of %u has been specified in %s.",
                         pFragmentSize->width, cmd_name);
    }

    if (pFragmentSize->height != 0 && !IsPowerOfTwo(pFragmentSize->height)) {
        skip |= LogError(cb_state->commandBuffer(), "VUID-vkCmdSetFragmentShadingRateKHR-pFragmentSize-04516",
                         "vkCmdSetFragmentShadingRateKHR: Non-power-of-two fragment height of %u has been specified in %s.",
                         pFragmentSize->height, cmd_name);
    }

    if (pFragmentSize->width > 4) {
        skip |= LogError(cb_state->commandBuffer(), "VUID-vkCmdSetFragmentShadingRateKHR-pFragmentSize-04517",
                         "vkCmdSetFragmentShadingRateKHR: Fragment width of %u specified in %s is too large.", pFragmentSize->width,
                         cmd_name);
    }

    if (pFragmentSize->height > 4) {
        skip |= LogError(cb_state->commandBuffer(), "VUID-vkCmdSetFragmentShadingRateKHR-pFragmentSize-04518",
                         "vkCmdSetFragmentShadingRateKHR: Fragment height of %u specified in %s is too large",
                         pFragmentSize->height, cmd_name);
    }
    return skip;
}

bool CoreChecks::PreCallValidateCmdSetColorWriteEnableEXT(VkCommandBuffer commandBuffer, uint32_t attachmentCount,
                                                          const VkBool32 *pColorWriteEnables) const {
    bool skip = false;

    auto cb_state = GetRead<CMD_BUFFER_STATE>(commandBuffer);
    skip |=
        ValidateExtendedDynamicState(*cb_state, CMD_SETCOLORWRITEENABLEEXT, enabled_features.color_write_features.colorWriteEnable,
                                     "VUID-vkCmdSetColorWriteEnableEXT-None-04803", "colorWriteEnable");

    if (attachmentCount > phys_dev_props.limits.maxColorAttachments) {
        skip |= LogError(commandBuffer, "VUID-vkCmdSetColorWriteEnableEXT-attachmentCount-06656",
                         "vkCmdSetColorWriteEnableEXT(): attachmentCount (%" PRIu32
                         ") is greater than the VkPhysicalDeviceLimits::maxColorAttachments limit (%" PRIu32 ").",
                         attachmentCount, phys_dev_props.limits.maxColorAttachments);
    }
    return skip;
}

bool CoreChecks::PreCallValidateCmdSetVertexInputEXT(
    VkCommandBuffer commandBuffer, uint32_t vertexBindingDescriptionCount,
    const VkVertexInputBindingDescription2EXT *pVertexBindingDescriptions, uint32_t vertexAttributeDescriptionCount,
    const VkVertexInputAttributeDescription2EXT *pVertexAttributeDescriptions) const {
    auto cb_state = GetRead<CMD_BUFFER_STATE>(commandBuffer);
    return ValidateExtendedDynamicState(*cb_state, CMD_SETVERTEXINPUTEXT,
                                        enabled_features.vertex_input_dynamic_state_features.vertexInputDynamicState ||
                                            enabled_features.shader_object_features.shaderObject,
                                        "VUID-vkCmdSetVertexInputEXT-None-08546", "vertexInputDynamicState or shaderObject");
}

bool CoreChecks::PreCallValidateCmdSetCoarseSampleOrderNV(VkCommandBuffer commandBuffer, VkCoarseSampleOrderTypeNV sampleOrderType,
                                                          uint32_t customSampleOrderCount,
                                                          const VkCoarseSampleOrderCustomNV *pCustomSampleOrders) const {
    auto cb_state = GetRead<CMD_BUFFER_STATE>(commandBuffer);
    return ValidateExtendedDynamicState(*cb_state, CMD_SETCOARSESAMPLEORDERNV, VK_TRUE, nullptr, nullptr);
}

bool CoreChecks::PreCallValidateCmdSetFragmentShadingRateEnumNV(VkCommandBuffer commandBuffer, VkFragmentShadingRateNV shadingRate,
                                                                const VkFragmentShadingRateCombinerOpKHR combinerOps[2]) const {
    auto cb_state = GetRead<CMD_BUFFER_STATE>(commandBuffer);
    return ValidateExtendedDynamicState(
        *cb_state, CMD_SETFRAGMENTSHADINGRATEENUMNV, enabled_features.fragment_shading_rate_enums_features.fragmentShadingRateEnums,
        "VUID-vkCmdSetFragmentShadingRateEnumNV-fragmentShadingRateEnums-04579", "fragmentShadingRateEnums");
}

bool CoreChecks::PreCallValidateCmdSetPerformanceMarkerINTEL(VkCommandBuffer commandBuffer,
                                                             const VkPerformanceMarkerInfoINTEL *pMarkerInfo) const {
    auto cb_state = GetRead<CMD_BUFFER_STATE>(commandBuffer);
    return ValidateExtendedDynamicState(*cb_state, CMD_SETPERFORMANCEMARKERINTEL, VK_TRUE, nullptr, nullptr);
}

bool CoreChecks::PreCallValidateCmdSetPerformanceStreamMarkerINTEL(VkCommandBuffer commandBuffer,
                                                                   const VkPerformanceStreamMarkerInfoINTEL *pMarkerInfo) const {
    auto cb_state = GetRead<CMD_BUFFER_STATE>(commandBuffer);
    return ValidateExtendedDynamicState(*cb_state, CMD_SETPERFORMANCEOVERRIDEINTEL, VK_TRUE, nullptr, nullptr);
}

bool CoreChecks::PreCallValidateCmdSetPerformanceOverrideINTEL(VkCommandBuffer commandBuffer,
                                                               const VkPerformanceOverrideInfoINTEL *pOverrideInfo) const {
    auto cb_state = GetRead<CMD_BUFFER_STATE>(commandBuffer);
    return ValidateExtendedDynamicState(*cb_state, CMD_SETCOARSESAMPLEORDERNV, VK_TRUE, nullptr, nullptr);
}

bool CoreChecks::PreCallValidateCmdSetAttachmentFeedbackLoopEnableEXT(VkCommandBuffer commandBuffer,
                                                                      VkImageAspectFlags aspectMask) const {
    bool skip = false;
    auto cb_state = GetRead<CMD_BUFFER_STATE>(commandBuffer);
    skip =
        ValidateExtendedDynamicState(*cb_state, CMD_SETATTACHMENTFEEDBACKLOOPENABLEEXT,
                                     enabled_features.attachment_feedback_loop_dynamic_features.attachmentFeedbackLoopDynamicState,
                                     "VUID-vkCmdSetAttachmentFeedbackLoopEnableEXT-attachmentFeedbackLoopDynamicState-08862",
                                     "attachmentFeedbackLoopDynamicState");

    if (aspectMask != VK_IMAGE_ASPECT_NONE &&
        !enabled_features.attachment_feedback_loop_layout_features.attachmentFeedbackLoopLayout) {
        skip |= LogError(commandBuffer, "VUID-vkCmdSetAttachmentFeedbackLoopEnableEXT-attachmentFeedbackLoopLayout-08864",
                         "vkCmdSetAttachmentFeedbackLoopEnableEXT(): aspestMask is %s but the attachmentFeedbackLoopLayout feature "
                         "is not enabled.",
                         string_VkImageAspectFlags(aspectMask).c_str());
    }

    if (aspectMask != VK_IMAGE_ASPECT_NONE && aspectMask != VK_IMAGE_ASPECT_COLOR_BIT && aspectMask != VK_IMAGE_ASPECT_DEPTH_BIT &&
        aspectMask != VK_IMAGE_ASPECT_STENCIL_BIT) {
        skip |=
            LogError(commandBuffer, "VUID-vkCmdSetAttachmentFeedbackLoopEnableEXT-aspectMask-08863",
                     "vkCmdSetAttachmentFeedbackLoopEnableEXT(): aspestMask is %s.", string_VkImageAspectFlags(aspectMask).c_str());
    }

    return skip;
}