/* Copyright (c) 2015-2023 The Khronos Group Inc.
 * Copyright (c) 2015-2023 Valve Corporation
 * Copyright (c) 2015-2023 LunarG, Inc.
 * Copyright (C) 2015-2023 Google Inc.
 * Modifications Copyright (C) 2020-2023 Advanced Micro Devices, Inc. All rights reserved.
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

#include "generated/chassis.h"
#include "core_validation.h"

// clang-format off
struct DispatchVuidsCmdDraw : DrawDispatchVuid {
    DispatchVuidsCmdDraw() : DrawDispatchVuid() {
        pipeline_bound_08606                     = "VUID-vkCmdDraw-None-08606";
        pipeline_or_shaders_bound_08607          = "VUID-vkCmdDraw-None-08607";
        vertex_binding_04007                     = "VUID-vkCmdDraw-None-04007";
        vertex_binding_null_04008                = "VUID-vkCmdDraw-None-04008";
        compatible_pipeline_08600                = "VUID-vkCmdDraw-None-08600";
        render_pass_compatible_02684             = "VUID-vkCmdDraw-renderPass-02684";
        render_pass_began_08876                  = "VUID-vkCmdDraw-None-08876";
        subpass_index_02685                      = "VUID-vkCmdDraw-subpass-02685";
        sample_location_02689                    = "VUID-vkCmdDraw-sampleLocationsEnable-02689";
        linear_filter_sampler_04553              = "VUID-vkCmdDraw-magFilter-04553";
        linear_mipmap_sampler_04770              = "VUID-vkCmdDraw-mipmapMode-04770";
        cubic_sampler_02692                      = "VUID-vkCmdDraw-None-02692";
        viewport_count_03417                     = "VUID-vkCmdDraw-viewportCount-03417";
        scissor_count_03418                      = "VUID-vkCmdDraw-scissorCount-03418";
        viewport_scissor_count_03419             = "VUID-vkCmdDraw-viewportCount-03419";
        primitive_topology_class_07500           = "VUID-vkCmdDraw-dynamicPrimitiveTopologyUnrestricted-07500";
        corner_sampled_address_mode_02696        = "VUID-vkCmdDraw-flags-02696";
        imageview_atomic_02691                   = "VUID-vkCmdDraw-None-02691";
        bufferview_atomic_07888                  = "VUID-vkCmdDraw-None-07888";
        push_constants_set_08602                 = "VUID-vkCmdDraw-maintenance4-08602";
        image_subresources_render_pass_write_06537 = "VUID-vkCmdDraw-None-06537";
        image_subresources_subpass_read_09003    = "VUID-vkCmdDraw-None-09003";
        image_subresources_subpass_write_06539   = "VUID-vkCmdDraw-None-06539";
        sampler_imageview_type_08609             = "VUID-vkCmdDraw-None-08609";
        sampler_implicitLod_dref_proj_08610      = "VUID-vkCmdDraw-None-08610";
        sampler_bias_offset_08611                = "VUID-vkCmdDraw-None-08611";
        vertex_binding_attribute_02721           = "VUID-vkCmdDraw-None-02721";
        dynamic_state_setting_commands_08608     = "VUID-vkCmdDraw-None-08608";
        msrtss_rasterization_samples_07284       = "VUID-vkCmdDraw-multisampledRenderToSingleSampled-07284";
        unprotected_command_buffer_02707         = "VUID-vkCmdDraw-commandBuffer-02707";
        protected_command_buffer_02712           = "VUID-vkCmdDraw-commandBuffer-02712";
        max_multiview_instance_index_02688       = "VUID-vkCmdDraw-maxMultiviewInstanceIndex-02688";
        img_filter_cubic_02693                   = "VUID-vkCmdDraw-None-02693";
        filter_cubic_02694                       = "VUID-vkCmdDraw-filterCubic-02694";
        filter_cubic_min_max_02695               = "VUID-vkCmdDraw-filterCubicMinmax-02695";
        viewport_count_primitive_shading_rate_04552 = "VUID-vkCmdDraw-primitiveFragmentShadingRateWithMultipleViewports-04552";
        patch_control_points_04875               = "VUID-vkCmdDraw-None-04875";
        rasterizer_discard_enable_04876          = "VUID-vkCmdDraw-None-04876";
        depth_bias_enable_04877                  = "VUID-vkCmdDraw-None-04877";
        logic_op_04878                           = "VUID-vkCmdDraw-logicOp-04878";
        primitive_restart_enable_04879           = "VUID-vkCmdDraw-None-04879";
        vertex_input_04912                       = "VUID-vkCmdDraw-None-04912";
        vertex_input_binding_stride_04913        = "VUID-vkCmdDraw-pStrides-04913";
        vertex_input_04914                       = "VUID-vkCmdDraw-None-04914";
        blend_enable_04727                       = "VUID-vkCmdDraw-blendEnable-04727";
        dynamic_discard_rectangle_07751          = "VUID-vkCmdDraw-None-07751";
        dynamic_discard_rectangle_enable_07880   = "VUID-vkCmdDraw-None-07880";
        dynamic_discard_rectangle_mode_07881     = "VUID-vkCmdDraw-None-07881";
        dynamic_exclusive_scissor_enable_07878   = "VUID-vkCmdDraw-None-07878";
        dynamic_exclusive_scissor_07879          = "VUID-vkCmdDraw-None-07879";
        dynamic_color_write_enable_07749         = "VUID-vkCmdDraw-None-07749";
        dynamic_color_write_enable_count_07750   = "VUID-vkCmdDraw-attachmentCount-07750";
        dynamic_attachment_feedback_loop_08877   = "VUID-vkCmdDraw-None-08877";
        dynamic_rendering_view_mask_06178        = "VUID-vkCmdDraw-viewMask-06178";
        dynamic_rendering_color_count_06179      = "VUID-vkCmdDraw-colorAttachmentCount-06179";
        dynamic_rendering_color_formats_08910    = "VUID-vkCmdDraw-dynamicRenderingUnusedAttachments-08910";
        dynamic_rendering_unused_attachments_08911 = "VUID-vkCmdDraw-dynamicRenderingUnusedAttachments-08911";
        dynamic_rendering_undefined_color_formats_08912 = "VUID-vkCmdDraw-dynamicRenderingUnusedAttachments-08912";
        dynamic_rendering_undefined_depth_format_08916 = "VUID-vkCmdDraw-dynamicRenderingUnusedAttachments-08916";
        dynamic_rendering_undefined_stencil_format_08916 = "VUID-vkCmdDraw-dynamicRenderingUnusedAttachments-08916";
        dynamic_rendering_depth_format_08914     = "VUID-vkCmdDraw-dynamicRenderingUnusedAttachments-08914";
        dynamic_rendering_unused_attachments_08915 = "VUID-vkCmdDraw-dynamicRenderingUnusedAttachments-08915";
        dynamic_rendering_stencil_format_08917   = "VUID-vkCmdDraw-dynamicRenderingUnusedAttachments-08917";
        dynamic_rendering_unused_attachments_08918 = "VUID-vkCmdDraw-dynamicRenderingUnusedAttachments-08918";
        dynamic_rendering_fsr_06183              = "VUID-vkCmdDraw-imageView-06183";
        dynamic_rendering_fdm_06184              = "VUID-vkCmdDraw-imageView-06184";
        dynamic_rendering_color_sample_06185     = "VUID-vkCmdDraw-colorAttachmentCount-06185";
        dynamic_rendering_depth_sample_06186     = "VUID-vkCmdDraw-pDepthAttachment-06186";
        dynamic_rendering_stencil_sample_06187   = "VUID-vkCmdDraw-pStencilAttachment-06187";
        dynamic_rendering_06198                  = "VUID-vkCmdDraw-renderPass-06198";
        dynamic_rendering_07285                  = "VUID-vkCmdDraw-multisampledRenderToSingleSampled-07285";
        dynamic_rendering_07286                  = "VUID-vkCmdDraw-multisampledRenderToSingleSampled-07286";
        dynamic_rendering_07287                  = "VUID-vkCmdDraw-multisampledRenderToSingleSampled-07287";
        image_view_access_64_04470               = "VUID-vkCmdDraw-SampledType-04470";
        image_view_access_32_04471               = "VUID-vkCmdDraw-SampledType-04471";
        image_view_sparse_64_04474               = "VUID-vkCmdDraw-sparseImageInt64Atomics-04474";
        buffer_view_access_64_04472              = "VUID-vkCmdDraw-SampledType-04472";
        buffer_view_access_32_04473              = "VUID-vkCmdDraw-SampledType-04473";
        storage_image_read_without_format_07028  = "VUID-vkCmdDraw-OpTypeImage-07028";
        storage_image_write_without_format_07027 = "VUID-vkCmdDraw-OpTypeImage-07027";
        storage_texel_buffer_read_without_format_07030  = "VUID-vkCmdDraw-OpTypeImage-07030";
        storage_texel_buffer_write_without_format_07029 = "VUID-vkCmdDraw-OpTypeImage-07029";
        storage_image_write_texel_count_08795           = "VUID-vkCmdDraw-OpImageWrite-08795";
        storage_image_write_texel_count_08796           = "VUID-vkCmdDraw-OpImageWrite-08796";
        storage_texel_buffer_write_texel_count_04469    = "VUID-vkCmdDraw-OpImageWrite-04469";
        depth_compare_sample_06479               = "VUID-vkCmdDraw-None-06479";
        depth_read_only_06886                    = "VUID-vkCmdDraw-None-06886";
        stencil_read_only_06887                  = "VUID-vkCmdDraw-None-06887";
        dynamic_sample_locations_06666           = "VUID-vkCmdDraw-None-06666";
        dynamic_tessellation_domain_origin_07619 = "VUID-vkCmdDraw-None-07619";
        dynamic_depth_clamp_enable_07620         = "VUID-vkCmdDraw-None-07620";
        dynamic_polygon_mode_07621               = "VUID-vkCmdDraw-None-07621";
        dynamic_rasterization_samples_07622      = "VUID-vkCmdDraw-None-07622";
        dynamic_sample_mask_07623                = "VUID-vkCmdDraw-None-07623";
        dynamic_alpha_to_coverage_enable_07624   = "VUID-vkCmdDraw-None-07624";
        dynamic_alpha_to_coverage_component_08919 = "VUID-vkCmdDraw-alphaToCoverageEnable-08919";
        dynamic_alpha_to_one_enable_07625        = "VUID-vkCmdDraw-None-07625";
        dynamic_logic_op_enable_07626            = "VUID-vkCmdDraw-None-07626";
        dynamic_color_blend_enable_07476         = "VUID-vkCmdDraw-firstAttachment-07476";
        dynamic_color_blend_equation_07477       = "VUID-vkCmdDraw-firstAttachment-07477";
        dynamic_color_write_mask_07478           = "VUID-vkCmdDraw-firstAttachment-07478";
        dynamic_rasterization_stream_07630       = "VUID-vkCmdDraw-None-07630";
        dynamic_conservative_rasterization_mode_07631 = "VUID-vkCmdDraw-None-07631";
        dynamic_extra_primitive_overestimation_size_07632 = "VUID-vkCmdDraw-None-07632";
        dynamic_depth_clip_enable_07633          = "VUID-vkCmdDraw-None-07633";
        dynamic_sample_locations_enable_07634    = "VUID-vkCmdDraw-None-07634";
        dynamic_color_blend_advanced_07479       = "VUID-vkCmdDraw-firstAttachment-07479";
        dynamic_provoking_vertex_mode_07636      = "VUID-vkCmdDraw-None-07636";
        dynamic_line_rasterization_mode_07637    = "VUID-vkCmdDraw-None-07637";
        dynamic_line_stipple_enable_07638        = "VUID-vkCmdDraw-None-07638";
        dynamic_depth_clip_negative_one_to_one_07639 = "VUID-vkCmdDraw-None-07639";
        dynamic_viewport_w_scaling_enable_07640  = "VUID-vkCmdDraw-None-07640";
        dynamic_viewport_swizzle_07641           = "VUID-vkCmdDraw-None-07641";
        dynamic_coverage_to_color_enable_07642   = "VUID-vkCmdDraw-None-07642";
        dynamic_coverage_to_color_location_07643 = "VUID-vkCmdDraw-None-07643";
        dynamic_coverage_modulation_mode_07644   = "VUID-vkCmdDraw-None-07644";
        dynamic_coverage_modulation_table_enable_07645 = "VUID-vkCmdDraw-None-07645";
        dynamic_coverage_modulation_table_07646  = "VUID-vkCmdDraw-None-07646";
        dynamic_shading_rate_image_enable_07647  = "VUID-vkCmdDraw-None-07647";
        dynamic_representative_fragment_test_enable_07648  = "VUID-vkCmdDraw-None-07648";
        dynamic_coverage_reduction_mode_07649    = "VUID-vkCmdDraw-None-07649";
        dynamic_viewport_07831                   = "VUID-vkCmdDraw-None-07831";
        dynamic_scissor_07832                    = "VUID-vkCmdDraw-None-07832";
        dynamic_depth_bias_07834                 = "VUID-vkCmdDraw-None-07834";
        dynamic_line_width_07833                 = "VUID-vkCmdDraw-None-07833";
        dynamic_line_stipple_ext_07849           = "VUID-vkCmdDraw-None-07849";
        dynamic_blend_constants_07835            = "VUID-vkCmdDraw-None-07835";
        dynamic_depth_bounds_07836               = "VUID-vkCmdDraw-None-07836";
        dynamic_depth_enable_08715               = "VUID-vkCmdDraw-pDynamicStates-08715";
        dynamic_stencil_compare_mask_07837       = "VUID-vkCmdDraw-None-07837";
        dynamic_stencil_write_mask_07838         = "VUID-vkCmdDraw-None-07838";
        dynamic_stencil_write_mask_08716         = "VUID-vkCmdDraw-pDynamicStates-08716";
        dynamic_stencil_reference_07839          = "VUID-vkCmdDraw-None-07839";
        dynamic_state_inherited_07850            = "VUID-vkCmdDraw-None-07850";
        dynamic_cull_mode_07840                  = "VUID-vkCmdDraw-None-07840";
        dynamic_front_face_07841                 = "VUID-vkCmdDraw-None-07841";
        dynamic_primitive_topology_07842         = "VUID-vkCmdDraw-None-07842";
        dynamic_depth_test_enable_07843          = "VUID-vkCmdDraw-None-07843";
        dynamic_depth_write_enable_07844         = "VUID-vkCmdDraw-None-07844";
        dynamic_depth_compare_op_07845           = "VUID-vkCmdDraw-None-07845";
        dynamic_depth_bound_test_enable_07846    = "VUID-vkCmdDraw-None-07846";
        dynamic_stencil_test_enable_07847        = "VUID-vkCmdDraw-None-07847";
        dynamic_stencil_op_07848                 = "VUID-vkCmdDraw-None-07848";
        primitives_generated_06708               = "VUID-vkCmdDraw-primitivesGeneratedQueryWithRasterizerDiscard-06708";
        primitives_generated_streams_06709       = "VUID-vkCmdDraw-primitivesGeneratedQueryWithNonZeroStreams-06709";
        invalid_mesh_shader_stages_06481         = "VUID-vkCmdDraw-stage-06481";
        descriptor_buffer_bit_set_08114          = "VUID-vkCmdDraw-None-08114";
        descriptor_buffer_bit_not_set_08115      = "VUID-vkCmdDraw-None-08115";
        descriptor_buffer_set_offset_missing_08117 = "VUID-vkCmdDraw-None-08117";
        image_view_dim_07752                     = "VUID-vkCmdDraw-viewType-07752";
        image_view_numeric_format_07753          = "VUID-vkCmdDraw-format-07753";
        stippled_rectangular_lines_07495         = "VUID-vkCmdDraw-stippledLineEnable-07495";
        stippled_bresenham_lines_07496           = "VUID-vkCmdDraw-stippledLineEnable-07496";
        stippled_smooth_lines_07497              = "VUID-vkCmdDraw-stippledLineEnable-07497";
        stippled_default_strict_07498            = "VUID-vkCmdDraw-stippledLineEnable-07498";
        viewport_and_scissor_with_count_08635    = "VUID-vkCmdDraw-None-08635";
        viewport_w_scaling_08636                 = "VUID-vkCmdDraw-None-08636";
        shading_rate_palette_08637               = "VUID-vkCmdDraw-None-08637";
        exclusive_scissor_08638                  = "VUID-vkCmdDraw-None-08638";
        external_format_resolve_09362            = "VUID-vkCmdDraw-colorAttachmentCount-09362";
        external_format_resolve_09363            = "VUID-vkCmdDraw-None-09363";
        external_format_resolve_09364            = "VUID-vkCmdDraw-None-09364";
        external_format_resolve_09365            = "VUID-vkCmdDraw-None-09365";
        external_format_resolve_09368            = "VUID-vkCmdDraw-None-09368";
        external_format_resolve_09369            = "VUID-vkCmdDraw-None-09369";
        external_format_resolve_09372            = "VUID-vkCmdDraw-colorAttachmentCount-09372";
        set_rasterizer_discard_enable_08639      = "VUID-vkCmdDraw-None-08639";
        set_depth_bias_enable_08640              = "VUID-vkCmdDraw-None-08640";
        set_logic_op_08641                       = "VUID-vkCmdDraw-None-08641";
        set_color_blend_enable_08643             = "VUID-vkCmdDraw-None-08643";
        set_rasterization_samples_08644              = "VUID-vkCmdDraw-None-08644";
        set_color_write_enable_08646             = "VUID-vkCmdDraw-None-08646";
        set_color_write_enable_08647             = "VUID-vkCmdDraw-None-08647";
        set_discard_rectangles_enable_08648      = "VUID-vkCmdDraw-None-08648";
        set_discard_rectangles_mode_08649      = "VUID-vkCmdDraw-None-08649";
        set_depth_clamp_enable_08650             = "VUID-vkCmdDraw-None-08650";
        set_polygon_mode_08651                   = "VUID-vkCmdDraw-None-08651";
        set_rasterization_samples_08652          = "VUID-vkCmdDraw-None-08652";
        set_sample_mask_08653                    = "VUID-vkCmdDraw-None-08653";
        set_alpha_to_coverage_enable_08654       = "VUID-vkCmdDraw-None-08654";
        set_alpha_to_one_enable_08655            = "VUID-vkCmdDraw-None-08655";
        set_logic_op_enable_08656                = "VUID-vkCmdDraw-None-08656";
        set_color_blend_enable_08657             = "VUID-vkCmdDraw-None-08657";
        set_color_blend_equation_08658           = "VUID-vkCmdDraw-None-08658";
        set_color_write_mask_08659               = "VUID-vkCmdDraw-None-08659";
        set_rasterization_streams_08660          = "VUID-vkCmdDraw-None-08660";
        set_conservative_rasterization_mode_08661 = "VUID-vkCmdDraw-None-08661";
        set_extra_primitive_overestimation_size_08662 = "VUID-vkCmdDraw-None-08662";
        set_depth_clip_enable_08663              = "VUID-vkCmdDraw-None-08663";
        set_sample_locations_enable_08664        = "VUID-vkCmdDraw-None-08664";
        set_provoking_vertex_mode_08665          = "VUID-vkCmdDraw-None-08665";
        set_line_rasterization_mode_08666        = "VUID-vkCmdDraw-None-08666";
        set_line_rasterization_mode_08667        = "VUID-vkCmdDraw-None-08667";
        set_line_rasterization_mode_08668        = "VUID-vkCmdDraw-None-08668";
        set_line_stipple_enable_08669            = "VUID-vkCmdDraw-None-08669";
        set_line_stipple_enable_08670            = "VUID-vkCmdDraw-None-08670";
        set_line_stipple_enable_08671            = "VUID-vkCmdDraw-None-08671";
        set_line_stipple_08672                   = "VUID-vkCmdDraw-None-08672";
        set_depth_clip_negative_one_to_one_08673 = "VUID-vkCmdDraw-None-08673";
        set_viewport_w_scaling_enable_08674      = "VUID-vkCmdDraw-None-08674";
        set_viewport_swizzle_08675               = "VUID-vkCmdDraw-None-08675";
        set_coverage_to_color_enable_08676       = "VUID-vkCmdDraw-None-08676";
        set_coverage_to_color_location_08677     = "VUID-vkCmdDraw-None-08677";
        set_coverage_modulation_mode_08678       = "VUID-vkCmdDraw-None-08678";
        set_coverage_modulation_table_enable_08679 = "VUID-vkCmdDraw-None-08679";
        set_coverage_modulation_table_08680      = "VUID-vkCmdDraw-None-08680";
        set_shading_rate_image_enable_08681      = "VUID-vkCmdDraw-None-08681";
        set_representative_fragment_test_enable_08682 = "VUID-vkCmdDraw-None-08682";
        set_coverage_reduction_mode_08683        = "VUID-vkCmdDraw-None-08683";
        vertex_shader_08684                      = "VUID-vkCmdDraw-None-08684";
        tessellation_control_shader_08685        = "VUID-vkCmdDraw-None-08685";
        tessellation_evaluation_shader_08686     = "VUID-vkCmdDraw-None-08686";
        geometry_shader_08687                    = "VUID-vkCmdDraw-None-08687";
        fragment_shader_08688                    = "VUID-vkCmdDraw-None-08688";
        task_shader_08689                        = "VUID-vkCmdDraw-None-08689";
        mesh_shader_08690                        = "VUID-vkCmdDraw-None-08690";
        vert_mesh_shader_08693                   = "VUID-vkCmdDraw-None-08693";
        task_mesh_shader_08694                   = "VUID-vkCmdDraw-None-08694";
        task_mesh_shader_08695                   = "VUID-vkCmdDraw-None-08695";
        vert_task_mesh_shader_08696              = "VUID-vkCmdDraw-None-08696";
        linked_shaders_08698                     = "VUID-vkCmdDraw-None-08698";
        linked_shaders_08699                     = "VUID-vkCmdDraw-None-08699";
        shaders_push_constants_08878             = "VUID-vkCmdDraw-None-08878";
        shaders_descriptor_layouts_08879         = "VUID-vkCmdDraw-None-08879";
        set_attachment_feedback_loop_enable_08880 = "VUID-vkCmdDraw-None-08880";
        set_vertex_input_08882                   = "VUID-vkCmdDraw-None-08882";
        draw_shaders_no_task_mesh_08885          = "VUID-vkCmdDraw-None-08885";
        set_line_width_08617                     = "VUID-vkCmdDraw-None-08617";
        set_line_width_08618                     = "VUID-vkCmdDraw-None-08618";
        set_depth_bias_08620                     = "VUID-vkCmdDraw-None-08620";
        set_blend_constants_08621                = "VUID-vkCmdDraw-None-08621";
        set_depth_bounds_08622                   = "VUID-vkCmdDraw-None-08622";
        set_stencil_compare_mask_08623           = "VUID-vkCmdDraw-None-08623";
        set_stencil_write_mask_08624             = "VUID-vkCmdDraw-None-08624";
        set_stencil_reference_08625              = "VUID-vkCmdDraw-None-08625";
        set_sample_locations_08626               = "VUID-vkCmdDraw-None-08626";
        set_cull_mode_08627                      = "VUID-vkCmdDraw-None-08627";
        set_front_face_08628                     = "VUID-vkCmdDraw-None-08628";
        set_depth_test_enable_08629              = "VUID-vkCmdDraw-None-08629";
        set_depth_write_enable_08630             = "VUID-vkCmdDraw-None-08630";
        set_depth_comapre_op_08631               = "VUID-vkCmdDraw-None-08631";
        set_depth_bounds_test_enable_08632       = "VUID-vkCmdDraw-None-08632";
        set_stencil_test_enable_08633            = "VUID-vkCmdDraw-None-08633";
        set_stencil_op_08634                     = "VUID-vkCmdDraw-None-08634";
		set_line_width_08619                     = "VUID-vkCmdDraw-None-08619";
        set_viewport_with_count_08642            = "VUID-vkCmdDraw-primitiveFragmentShadingRateWithMultipleViewports-08642";
        alpha_component_word_08920               = "VUID-vkCmdDraw-alphaToCoverageEnable-08920";
        color_write_mask_09116                   = "VUID-vkCmdDraw-None-09116";
        vertex_input_format_08936                = "VUID-vkCmdDraw-format-08936";
        vertex_input_format_08937                = "VUID-vkCmdDraw-format-08937";
        vertex_input_format_09203                = "VUID-vkCmdDraw-None-09203";
        vertex_input_format_07939                = "VUID-vkCmdDraw-Input-07939";
        set_clip_space_w_scaling_09232           = "VUID-vkCmdDraw-None-09232";
        set_discard_rectangle_09236              = "VUID-vkCmdDraw-rasterizerDiscardEnable-09236";
        set_viewport_coarse_sample_order_09233   = "VUID-vkCmdDraw-shadingRateImage-09233";
        set_viewport_shading_rate_palette_09234  = "VUID-vkCmdDraw-shadingRateImage-09234";
        set_exclusive_scissor_enable_09235       = "VUID-vkCmdDraw-exclusiveScissor-09235";
        set_fragment_shading_rate_09238          = "VUID-vkCmdDraw-pipelineFragmentShadingRate-09238";
        set_tessellation_domain_origin_09237     = "VUID-vkCmdDraw-None-09237";
        rasterization_samples_07935              = "VUID-vkCmdDraw-pNext-07935";
        mesh_shader_queries_07073                = "VUID-vkCmdDraw-stage-07073";
        blend_advanced_07480                     = "VUID-vkCmdDraw-advancedBlendMaxColorAttachments-07480";
        blend_feature_07470                      = "VUID-vkCmdDraw-pColorBlendEnables-07470";
        color_attachment_08963                   = "VUID-vkCmdDraw-pColorAttachments-08963";
        depth_attachment_08964                   = "VUID-vkCmdDraw-pDepthAttachment-08964";
        stencil_attachment_08965                 = "VUID-vkCmdDraw-pStencilAttachment-08965";
        sample_locations_07482                   = "VUID-vkCmdDraw-sampleLocationsPerPixel-07482";
    }
};

struct DispatchVuidsCmdDrawMultiEXT : DrawDispatchVuid {
    DispatchVuidsCmdDrawMultiEXT() : DrawDispatchVuid() {
        pipeline_bound_08606                     = "VUID-vkCmdDrawMultiEXT-None-08606";
        pipeline_or_shaders_bound_08607          = "VUID-vkCmdDrawMultiEXT-None-08607";
        vertex_binding_04007                     = "VUID-vkCmdDrawMultiEXT-None-04007";
        vertex_binding_null_04008                = "VUID-vkCmdDrawMultiEXT-None-04008";
        compatible_pipeline_08600                = "VUID-vkCmdDrawMultiEXT-None-08600";
        render_pass_compatible_02684             = "VUID-vkCmdDrawMultiEXT-renderPass-02684";
        render_pass_began_08876                  = "VUID-vkCmdDrawMultiEXT-None-08876";
        subpass_index_02685                      = "VUID-vkCmdDrawMultiEXT-subpass-02685";
        sample_location_02689                    = "VUID-vkCmdDrawMultiEXT-sampleLocationsEnable-02689";
        linear_filter_sampler_04553              = "VUID-vkCmdDrawMultiEXT-magFilter-04553";
        linear_mipmap_sampler_04770              = "VUID-vkCmdDrawMultiEXT-mipmapMode-04770";
        cubic_sampler_02692                      = "VUID-vkCmdDrawMultiEXT-None-02692";
        viewport_count_03417                     = "VUID-vkCmdDrawMultiEXT-viewportCount-03417";
        scissor_count_03418                      = "VUID-vkCmdDrawMultiEXT-scissorCount-03418";
        viewport_scissor_count_03419             = "VUID-vkCmdDrawMultiEXT-viewportCount-03419";
        primitive_topology_class_07500           = "VUID-vkCmdDrawMultiEXT-dynamicPrimitiveTopologyUnrestricted-07500";
        corner_sampled_address_mode_02696        = "VUID-vkCmdDrawMultiEXT-flags-02696";
        imageview_atomic_02691                   = "VUID-vkCmdDrawMultiEXT-None-02691";
        bufferview_atomic_07888                  = "VUID-vkCmdDrawMultiEXT-None-07888";
        push_constants_set_08602                 = "VUID-vkCmdDrawMultiEXT-maintenance4-08602";
        image_subresources_render_pass_write_06537 = "VUID-vkCmdDrawMultiEXT-None-06537";
        image_subresources_subpass_read_09003    = "VUID-vkCmdDrawMultiEXT-None-09003";
        image_subresources_subpass_write_06539   = "VUID-vkCmdDrawMultiEXT-None-06539";
        sampler_imageview_type_08609             = "VUID-vkCmdDrawMultiEXT-None-08609";
        sampler_implicitLod_dref_proj_08610      = "VUID-vkCmdDrawMultiEXT-None-08610";
        sampler_bias_offset_08611                = "VUID-vkCmdDrawMultiEXT-None-08611";
        vertex_binding_attribute_02721           = "VUID-vkCmdDrawMultiEXT-None-02721";
        dynamic_state_setting_commands_08608     = "VUID-vkCmdDrawMultiEXT-None-08608";
        msrtss_rasterization_samples_07284       = "VUID-vkCmdDrawMultiEXT-multisampledRenderToSingleSampled-07284";
        unprotected_command_buffer_02707         = "VUID-vkCmdDrawMultiEXT-commandBuffer-02707";
        protected_command_buffer_02712           = "VUID-vkCmdDrawMultiEXT-commandBuffer-02712";
        max_multiview_instance_index_02688       = "VUID-vkCmdDrawMultiEXT-maxMultiviewInstanceIndex-02688";
        img_filter_cubic_02693                   = "VUID-vkCmdDrawMultiEXT-None-02693";
        filter_cubic_02694                       = "VUID-vkCmdDrawMultiEXT-filterCubic-02694";
        filter_cubic_min_max_02695               = "VUID-vkCmdDrawMultiEXT-filterCubicMinmax-02695";
        viewport_count_primitive_shading_rate_04552 = "VUID-vkCmdDrawMultiEXT-primitiveFragmentShadingRateWithMultipleViewports-04552";
        patch_control_points_04875               = "VUID-vkCmdDrawMultiEXT-None-04875";
        rasterizer_discard_enable_04876          = "VUID-vkCmdDrawMultiEXT-None-04876";
        depth_bias_enable_04877                  = "VUID-vkCmdDrawMultiEXT-None-04877";
        logic_op_04878                           = "VUID-vkCmdDrawMultiEXT-logicOp-04878";
        primitive_restart_enable_04879           = "VUID-vkCmdDrawMultiEXT-None-04879";
        vertex_input_04912                       = "VUID-vkCmdDrawMultiEXT-None-04912";
        vertex_input_binding_stride_04913        = "VUID-vkCmdDrawMultiEXT-pStrides-04913";
        vertex_input_04914                       = "VUID-vkCmdDrawMultiEXT-None-04914";
        blend_enable_04727                       = "VUID-vkCmdDrawMultiEXT-blendEnable-04727";
        dynamic_discard_rectangle_07751          = "VUID-vkCmdDrawMultiEXT-None-07751";
        dynamic_discard_rectangle_enable_07880   = "VUID-vkCmdDrawMultiEXT-None-07880";
        dynamic_discard_rectangle_mode_07881     = "VUID-vkCmdDrawMultiEXT-None-07881";
        dynamic_exclusive_scissor_enable_07878   = "VUID-vkCmdDrawMultiEXT-None-07878";
        dynamic_exclusive_scissor_07879          = "VUID-vkCmdDrawMultiEXT-None-07879";
        dynamic_color_write_enable_07749         = "VUID-vkCmdDrawMultiEXT-None-07749";
        dynamic_color_write_enable_count_07750   = "VUID-vkCmdDrawMultiEXT-attachmentCount-07750";
        dynamic_attachment_feedback_loop_08877   = "VUID-vkCmdDrawMultiEXT-None-08877";
        dynamic_rendering_view_mask_06178        = "VUID-vkCmdDrawMultiEXT-viewMask-06178";
        dynamic_rendering_color_count_06179      = "VUID-vkCmdDrawMultiEXT-colorAttachmentCount-06179";
        dynamic_rendering_color_formats_08910    = "VUID-vkCmdDrawMultiEXT-dynamicRenderingUnusedAttachments-08910";
        dynamic_rendering_unused_attachments_08911 = "VUID-vkCmdDrawMultiEXT-dynamicRenderingUnusedAttachments-08911";
        dynamic_rendering_undefined_color_formats_08912 = "VUID-vkCmdDrawMultiEXT-dynamicRenderingUnusedAttachments-08912";
        dynamic_rendering_undefined_depth_format_08916 = "VUID-vkCmdDrawMultiEXT-dynamicRenderingUnusedAttachments-08916";
        dynamic_rendering_undefined_stencil_format_08916 = "VUID-vkCmdDrawMultiEXT-dynamicRenderingUnusedAttachments-08916";
        dynamic_rendering_depth_format_08914     = "VUID-vkCmdDrawMultiEXT-dynamicRenderingUnusedAttachments-08914";
        dynamic_rendering_unused_attachments_08915 = "VUID-vkCmdDrawMultiEXT-dynamicRenderingUnusedAttachments-08915";
        dynamic_rendering_stencil_format_08917   = "VUID-vkCmdDrawMultiEXT-dynamicRenderingUnusedAttachments-08917";
        dynamic_rendering_unused_attachments_08918 = "VUID-vkCmdDrawMultiEXT-dynamicRenderingUnusedAttachments-08918";
        dynamic_rendering_fsr_06183              = "VUID-vkCmdDrawMultiEXT-imageView-06183";
        dynamic_rendering_fdm_06184              = "VUID-vkCmdDrawMultiEXT-imageView-06184";
        dynamic_rendering_color_sample_06185     = "VUID-vkCmdDrawMultiEXT-colorAttachmentCount-06185";
        dynamic_rendering_depth_sample_06186     = "VUID-vkCmdDrawMultiEXT-pDepthAttachment-06186";
        dynamic_rendering_stencil_sample_06187   = "VUID-vkCmdDrawMultiEXT-pStencilAttachment-06187";
        dynamic_rendering_06198                  = "VUID-vkCmdDrawMultiEXT-renderPass-06198";
        dynamic_rendering_07285                  = "VUID-vkCmdDrawMultiEXT-multisampledRenderToSingleSampled-07285";
        dynamic_rendering_07286                  = "VUID-vkCmdDrawMultiEXT-multisampledRenderToSingleSampled-07286";
        dynamic_rendering_07287                  = "VUID-vkCmdDrawMultiEXT-multisampledRenderToSingleSampled-07287";
        image_view_access_64_04470               = "VUID-vkCmdDrawMultiEXT-SampledType-04470";
        image_view_access_32_04471               = "VUID-vkCmdDrawMultiEXT-SampledType-04471";
        image_view_sparse_64_04474               = "VUID-vkCmdDrawMultiEXT-sparseImageInt64Atomics-04474";
        buffer_view_access_64_04472              = "VUID-vkCmdDrawMultiEXT-SampledType-04472";
        buffer_view_access_32_04473              = "VUID-vkCmdDrawMultiEXT-SampledType-04473";
        storage_image_read_without_format_07028  = "VUID-vkCmdDrawMultiEXT-OpTypeImage-07028";
        storage_image_write_without_format_07027 = "VUID-vkCmdDrawMultiEXT-OpTypeImage-07027";
        storage_texel_buffer_read_without_format_07030  = "VUID-vkCmdDrawMultiEXT-OpTypeImage-07030";
        storage_texel_buffer_write_without_format_07029 = "VUID-vkCmdDrawMultiEXT-OpTypeImage-07029";
        storage_image_write_texel_count_08795           = "VUID-vkCmdDrawMultiEXT-OpImageWrite-08795";
        storage_image_write_texel_count_08796           = "VUID-vkCmdDrawMultiEXT-OpImageWrite-08796";
        storage_texel_buffer_write_texel_count_04469    = "VUID-vkCmdDrawMultiEXT-OpImageWrite-04469";
        depth_compare_sample_06479               = "VUID-vkCmdDrawMultiEXT-None-06479";
        depth_read_only_06886                    = "VUID-vkCmdDrawMultiEXT-None-06886";
        stencil_read_only_06887                  = "VUID-vkCmdDrawMultiEXT-None-06887";
        dynamic_sample_locations_06666           = "VUID-vkCmdDrawMultiEXT-None-06666";
        dynamic_tessellation_domain_origin_07619 = "VUID-vkCmdDrawMultiEXT-None-07619";
        dynamic_depth_clamp_enable_07620         = "VUID-vkCmdDrawMultiEXT-None-07620";
        dynamic_polygon_mode_07621               = "VUID-vkCmdDrawMultiEXT-None-07621";
        dynamic_rasterization_samples_07622      = "VUID-vkCmdDrawMultiEXT-None-07622";
        dynamic_sample_mask_07623                = "VUID-vkCmdDrawMultiEXT-None-07623";
        dynamic_alpha_to_coverage_enable_07624   = "VUID-vkCmdDrawMultiEXT-None-07624";
        dynamic_alpha_to_coverage_component_08919 = "VUID-vkCmdDrawMultiEXT-alphaToCoverageEnable-08919";
        dynamic_alpha_to_one_enable_07625        = "VUID-vkCmdDrawMultiEXT-None-07625";
        dynamic_logic_op_enable_07626            = "VUID-vkCmdDrawMultiEXT-None-07626";
        dynamic_color_blend_enable_07476         = "VUID-vkCmdDrawMultiEXT-firstAttachment-07476";
        dynamic_color_blend_equation_07477       = "VUID-vkCmdDrawMultiEXT-firstAttachment-07477";
        dynamic_color_write_mask_07478           = "VUID-vkCmdDrawMultiEXT-firstAttachment-07478";
        dynamic_rasterization_stream_07630       = "VUID-vkCmdDrawMultiEXT-None-07630";
        dynamic_conservative_rasterization_mode_07631 = "VUID-vkCmdDrawMultiEXT-None-07631";
        dynamic_extra_primitive_overestimation_size_07632 = "VUID-vkCmdDrawMultiEXT-None-07632";
        dynamic_depth_clip_enable_07633          = "VUID-vkCmdDrawMultiEXT-None-07633";
        dynamic_sample_locations_enable_07634    = "VUID-vkCmdDrawMultiEXT-None-07634";
        dynamic_color_blend_advanced_07479       = "VUID-vkCmdDrawMultiEXT-firstAttachment-07479";
        dynamic_provoking_vertex_mode_07636      = "VUID-vkCmdDrawMultiEXT-None-07636";
        dynamic_line_rasterization_mode_07637    = "VUID-vkCmdDrawMultiEXT-None-07637";
        dynamic_line_stipple_enable_07638        = "VUID-vkCmdDrawMultiEXT-None-07638";
        dynamic_depth_clip_negative_one_to_one_07639 = "VUID-vkCmdDrawMultiEXT-None-07639";
        dynamic_viewport_w_scaling_enable_07640  = "VUID-vkCmdDrawMultiEXT-None-07640";
        dynamic_viewport_swizzle_07641           = "VUID-vkCmdDrawMultiEXT-None-07641";
        dynamic_coverage_to_color_enable_07642   = "VUID-vkCmdDrawMultiEXT-None-07642";
        dynamic_coverage_to_color_location_07643 = "VUID-vkCmdDrawMultiEXT-None-07643";
        dynamic_coverage_modulation_mode_07644   = "VUID-vkCmdDrawMultiEXT-None-07644";
        dynamic_coverage_modulation_table_enable_07645 = "VUID-vkCmdDrawMultiEXT-None-07645";
        dynamic_coverage_modulation_table_07646  = "VUID-vkCmdDrawMultiEXT-None-07646";
        dynamic_shading_rate_image_enable_07647  = "VUID-vkCmdDrawMultiEXT-None-07647";
        dynamic_representative_fragment_test_enable_07648  = "VUID-vkCmdDrawMultiEXT-None-07648";
        dynamic_coverage_reduction_mode_07649    = "VUID-vkCmdDrawMultiEXT-None-07649";
        dynamic_viewport_07831                   = "VUID-vkCmdDrawMultiEXT-None-07831";
        dynamic_scissor_07832                    = "VUID-vkCmdDrawMultiEXT-None-07832";
        dynamic_depth_bias_07834                 = "VUID-vkCmdDrawMultiEXT-None-07834";
        dynamic_line_width_07833                 = "VUID-vkCmdDrawMultiEXT-None-07833";
        dynamic_line_stipple_ext_07849           = "VUID-vkCmdDrawMultiEXT-None-07849";
        dynamic_blend_constants_07835            = "VUID-vkCmdDrawMultiEXT-None-07835";
        dynamic_depth_bounds_07836               = "VUID-vkCmdDrawMultiEXT-None-07836";
        dynamic_depth_enable_08715               = "VUID-vkCmdDrawMultiEXT-pDynamicStates-08715";
        dynamic_stencil_compare_mask_07837       = "VUID-vkCmdDrawMultiEXT-None-07837";
        dynamic_stencil_write_mask_07838         = "VUID-vkCmdDrawMultiEXT-None-07838";
        dynamic_stencil_write_mask_08716         = "VUID-vkCmdDrawMultiEXT-pDynamicStates-08716";
        dynamic_stencil_reference_07839          = "VUID-vkCmdDrawMultiEXT-None-07839";
        dynamic_state_inherited_07850            = "VUID-vkCmdDrawMultiEXT-None-07850";
        dynamic_cull_mode_07840                  = "VUID-vkCmdDrawMultiEXT-None-07840";
        dynamic_front_face_07841                 = "VUID-vkCmdDrawMultiEXT-None-07841";
        dynamic_primitive_topology_07842         = "VUID-vkCmdDrawMultiEXT-None-07842";
        dynamic_depth_test_enable_07843          = "VUID-vkCmdDrawMultiEXT-None-07843";
        dynamic_depth_write_enable_07844         = "VUID-vkCmdDrawMultiEXT-None-07844";
        dynamic_depth_compare_op_07845           = "VUID-vkCmdDrawMultiEXT-None-07845";
        dynamic_depth_bound_test_enable_07846    = "VUID-vkCmdDrawMultiEXT-None-07846";
        dynamic_stencil_test_enable_07847        = "VUID-vkCmdDrawMultiEXT-None-07847";
        dynamic_stencil_op_07848                 = "VUID-vkCmdDrawMultiEXT-None-07848";
        primitives_generated_06708               = "VUID-vkCmdDrawMultiEXT-primitivesGeneratedQueryWithRasterizerDiscard-06708";
        primitives_generated_streams_06709       = "VUID-vkCmdDrawMultiEXT-primitivesGeneratedQueryWithNonZeroStreams-06709";
        invalid_mesh_shader_stages_06481         = "VUID-vkCmdDrawMultiEXT-stage-06481";
        descriptor_buffer_bit_set_08114          = "VUID-vkCmdDrawMultiEXT-None-08114";
        descriptor_buffer_bit_not_set_08115      = "VUID-vkCmdDrawMultiEXT-None-08115";
        descriptor_buffer_set_offset_missing_08117 = "VUID-vkCmdDrawMultiEXT-None-08117";
        image_view_dim_07752                     = "VUID-vkCmdDrawMultiEXT-viewType-07752";
        image_view_numeric_format_07753          = "VUID-vkCmdDrawMultiEXT-format-07753";
        stippled_rectangular_lines_07495         = "VUID-vkCmdDrawMultiEXT-stippledLineEnable-07495";
        stippled_bresenham_lines_07496           = "VUID-vkCmdDrawMultiEXT-stippledLineEnable-07496";
        stippled_smooth_lines_07497              = "VUID-vkCmdDrawMultiEXT-stippledLineEnable-07497";
        stippled_default_strict_07498            = "VUID-vkCmdDrawMultiEXT-stippledLineEnable-07498";
        viewport_and_scissor_with_count_08635    = "VUID-vkCmdDrawMultiEXT-None-08635";
        viewport_w_scaling_08636                 = "VUID-vkCmdDrawMultiEXT-None-08636";
        shading_rate_palette_08637               = "VUID-vkCmdDrawMultiEXT-None-08637";
        exclusive_scissor_08638                  = "VUID-vkCmdDrawMultiEXT-None-08638";
        external_format_resolve_09362            = "VUID-vkCmdDrawMultiEXT-colorAttachmentCount-09362";
        external_format_resolve_09363            = "VUID-vkCmdDrawMultiEXT-None-09363";
        external_format_resolve_09364            = "VUID-vkCmdDrawMultiEXT-None-09364";
        external_format_resolve_09365            = "VUID-vkCmdDrawMultiEXT-None-09365";
        external_format_resolve_09368            = "VUID-vkCmdDrawMultiEXT-None-09368";
        external_format_resolve_09369            = "VUID-vkCmdDrawMultiEXT-None-09369";
        external_format_resolve_09372            = "VUID-vkCmdDrawMultiEXT-colorAttachmentCount-09372";
        set_rasterizer_discard_enable_08639      = "VUID-vkCmdDrawMultiEXT-None-08639";
        set_depth_bias_enable_08640              = "VUID-vkCmdDrawMultiEXT-None-08640";
        set_logic_op_08641                       = "VUID-vkCmdDrawMultiEXT-None-08641";
        set_color_blend_enable_08643             = "VUID-vkCmdDrawMultiEXT-None-08643";
        set_rasterization_samples_08644              = "VUID-vkCmdDrawMultiEXT-None-08644";
        set_color_write_enable_08646             = "VUID-vkCmdDrawMultiEXT-None-08646";
        set_color_write_enable_08647             = "VUID-vkCmdDrawMultiEXT-None-08647";
        set_discard_rectangles_enable_08648      = "VUID-vkCmdDrawMultiEXT-None-08648";
        set_discard_rectangles_mode_08649      = "VUID-vkCmdDrawMultiEXT-None-08649";
        set_depth_clamp_enable_08650             = "VUID-vkCmdDrawMultiEXT-None-08650";
        set_polygon_mode_08651                   = "VUID-vkCmdDrawMultiEXT-None-08651";
        set_rasterization_samples_08652          = "VUID-vkCmdDrawMultiEXT-None-08652";
        set_sample_mask_08653                    = "VUID-vkCmdDrawMultiEXT-None-08653";
        set_alpha_to_coverage_enable_08654       = "VUID-vkCmdDrawMultiEXT-None-08654";
        set_alpha_to_one_enable_08655            = "VUID-vkCmdDrawMultiEXT-None-08655";
        set_logic_op_enable_08656                = "VUID-vkCmdDrawMultiEXT-None-08656";
        set_color_blend_enable_08657             = "VUID-vkCmdDrawMultiEXT-None-08657";
        set_color_blend_equation_08658           = "VUID-vkCmdDrawMultiEXT-None-08658";
        set_color_write_mask_08659               = "VUID-vkCmdDrawMultiEXT-None-08659";
        set_rasterization_streams_08660          = "VUID-vkCmdDrawMultiEXT-None-08660";
        set_conservative_rasterization_mode_08661 = "VUID-vkCmdDrawMultiEXT-None-08661";
        set_extra_primitive_overestimation_size_08662 = "VUID-vkCmdDrawMultiEXT-None-08662";
        set_depth_clip_enable_08663              = "VUID-vkCmdDrawMultiEXT-None-08663";
        set_sample_locations_enable_08664        = "VUID-vkCmdDrawMultiEXT-None-08664";
        set_provoking_vertex_mode_08665          = "VUID-vkCmdDrawMultiEXT-None-08665";
        set_line_rasterization_mode_08666        = "VUID-vkCmdDrawMultiEXT-None-08666";
        set_line_rasterization_mode_08667        = "VUID-vkCmdDrawMultiEXT-None-08667";
        set_line_rasterization_mode_08668        = "VUID-vkCmdDrawMultiEXT-None-08668";
        set_line_stipple_enable_08669            = "VUID-vkCmdDrawMultiEXT-None-08669";
        set_line_stipple_enable_08670            = "VUID-vkCmdDrawMultiEXT-None-08670";
        set_line_stipple_enable_08671            = "VUID-vkCmdDrawMultiEXT-None-08671";
        set_line_stipple_08672                   = "VUID-vkCmdDrawMultiEXT-None-08672";
        set_depth_clip_negative_one_to_one_08673 = "VUID-vkCmdDrawMultiEXT-None-08673";
        set_viewport_w_scaling_enable_08674      = "VUID-vkCmdDrawMultiEXT-None-08674";
        set_viewport_swizzle_08675               = "VUID-vkCmdDrawMultiEXT-None-08675";
        set_coverage_to_color_enable_08676       = "VUID-vkCmdDrawMultiEXT-None-08676";
        set_coverage_to_color_location_08677     = "VUID-vkCmdDrawMultiEXT-None-08677";
        set_coverage_modulation_mode_08678       = "VUID-vkCmdDrawMultiEXT-None-08678";
        set_coverage_modulation_table_enable_08679 = "VUID-vkCmdDrawMultiEXT-None-08679";
        set_coverage_modulation_table_08680      = "VUID-vkCmdDrawMultiEXT-None-08680";
        set_shading_rate_image_enable_08681      = "VUID-vkCmdDrawMultiEXT-None-08681";
        set_representative_fragment_test_enable_08682 = "VUID-vkCmdDrawMultiEXT-None-08682";
        set_coverage_reduction_mode_08683        = "VUID-vkCmdDrawMultiEXT-None-08683";
        vertex_shader_08684                      = "VUID-vkCmdDrawMultiEXT-None-08684";
        tessellation_control_shader_08685        = "VUID-vkCmdDrawMultiEXT-None-08685";
        tessellation_evaluation_shader_08686     = "VUID-vkCmdDrawMultiEXT-None-08686";
        geometry_shader_08687                    = "VUID-vkCmdDrawMultiEXT-None-08687";
        fragment_shader_08688                    = "VUID-vkCmdDrawMultiEXT-None-08688";
        task_shader_08689                        = "VUID-vkCmdDrawMultiEXT-None-08689";
        mesh_shader_08690                        = "VUID-vkCmdDrawMultiEXT-None-08690";
        vert_mesh_shader_08693                   = "VUID-vkCmdDrawMultiEXT-None-08693";
        task_mesh_shader_08694                   = "VUID-vkCmdDrawMultiEXT-None-08694";
        task_mesh_shader_08695                   = "VUID-vkCmdDrawMultiEXT-None-08695";
        vert_task_mesh_shader_08696              = "VUID-vkCmdDrawMultiEXT-None-08696";
        linked_shaders_08698                     = "VUID-vkCmdDrawMultiEXT-None-08698";
        linked_shaders_08699                     = "VUID-vkCmdDrawMultiEXT-None-08699";
        shaders_push_constants_08878             = "VUID-vkCmdDrawMultiEXT-None-08878";
        shaders_descriptor_layouts_08879         = "VUID-vkCmdDrawMultiEXT-None-08879";
        set_attachment_feedback_loop_enable_08880 = "VUID-vkCmdDrawMultiEXT-None-08880";
        set_vertex_input_08882                   = "VUID-vkCmdDrawMultiEXT-None-08882";
        draw_shaders_no_task_mesh_08885          = "VUID-vkCmdDrawMultiEXT-None-08885";
        set_line_width_08617                     = "VUID-vkCmdDrawMultiEXT-None-08617";
        set_line_width_08618                     = "VUID-vkCmdDrawMultiEXT-None-08618";
        set_depth_bias_08620                     = "VUID-vkCmdDrawMultiEXT-None-08620";
        set_blend_constants_08621                = "VUID-vkCmdDrawMultiEXT-None-08621";
        set_depth_bounds_08622                   = "VUID-vkCmdDrawMultiEXT-None-08622";
        set_stencil_compare_mask_08623           = "VUID-vkCmdDrawMultiEXT-None-08623";
        set_stencil_write_mask_08624             = "VUID-vkCmdDrawMultiEXT-None-08624";
        set_stencil_reference_08625              = "VUID-vkCmdDrawMultiEXT-None-08625";
        set_sample_locations_08626               = "VUID-vkCmdDrawMultiEXT-None-08626";
        set_cull_mode_08627                      = "VUID-vkCmdDrawMultiEXT-None-08627";
        set_front_face_08628                     = "VUID-vkCmdDrawMultiEXT-None-08628";
        set_depth_test_enable_08629              = "VUID-vkCmdDrawMultiEXT-None-08629";
        set_depth_write_enable_08630             = "VUID-vkCmdDrawMultiEXT-None-08630";
        set_depth_comapre_op_08631               = "VUID-vkCmdDrawMultiEXT-None-08631";
        set_depth_bounds_test_enable_08632       = "VUID-vkCmdDrawMultiEXT-None-08632";
        set_stencil_test_enable_08633            = "VUID-vkCmdDrawMultiEXT-None-08633";
        set_stencil_op_08634                     = "VUID-vkCmdDrawMultiEXT-None-08634";
        set_line_width_08619                     = "VUID-vkCmdDrawMultiEXT-None-08619";
        set_viewport_with_count_08642            = "VUID-vkCmdDrawMultiEXT-primitiveFragmentShadingRateWithMultipleViewports-08642";
        alpha_component_word_08920               = "VUID-vkCmdDrawMultiEXT-alphaToCoverageEnable-08920";
        color_write_mask_09116                   = "VUID-vkCmdDrawMultiEXT-None-09116";
        vertex_input_format_08936                = "VUID-vkCmdDrawMultiEXT-format-08936";
        vertex_input_format_08937                = "VUID-vkCmdDrawMultiEXT-format-08937";
        vertex_input_format_09203                = "VUID-vkCmdDrawMultiEXT-None-09203";
        vertex_input_format_07939                = "VUID-vkCmdDrawMultiEXT-Input-07939";
        set_clip_space_w_scaling_09232           = "VUID-vkCmdDrawMultiEXT-None-09232";
        set_discard_rectangle_09236              = "VUID-vkCmdDrawMultiEXT-rasterizerDiscardEnable-09236";
        set_viewport_coarse_sample_order_09233   = "VUID-vkCmdDrawMultiEXT-shadingRateImage-09233";
        set_viewport_shading_rate_palette_09234  = "VUID-vkCmdDrawMultiEXT-shadingRateImage-09234";
        set_exclusive_scissor_enable_09235       = "VUID-vkCmdDrawMultiEXT-exclusiveScissor-09235";
        set_fragment_shading_rate_09238          = "VUID-vkCmdDrawMultiEXT-pipelineFragmentShadingRate-09238";
        set_tessellation_domain_origin_09237     = "VUID-vkCmdDrawMultiEXT-None-09237";
        rasterization_samples_07935              = "VUID-vkCmdDrawMultiEXT-pNext-07935";
        mesh_shader_queries_07073                = "VUID-vkCmdDrawMultiEXT-stage-07073";
        blend_advanced_07480                     = "VUID-vkCmdDrawMultiEXT-advancedBlendMaxColorAttachments-07480";
        blend_feature_07470                      = "VUID-vkCmdDrawMultiEXT-pColorBlendEnables-07470";
        color_attachment_08963                   = "VUID-vkCmdDrawMultiEXT-pColorAttachments-08963";
        depth_attachment_08964                   = "VUID-vkCmdDrawMultiEXT-pDepthAttachment-08964";
        stencil_attachment_08965                 = "VUID-vkCmdDrawMultiEXT-pStencilAttachment-08965";
        sample_locations_07482                   = "VUID-vkCmdDrawMultiEXT-sampleLocationsPerPixel-07482";
    }
};

struct DispatchVuidsCmdDrawIndexed : DrawDispatchVuid {
    DispatchVuidsCmdDrawIndexed() : DrawDispatchVuid() {
        pipeline_bound_08606                     = "VUID-vkCmdDrawIndexed-None-08606";
        pipeline_or_shaders_bound_08607          = "VUID-vkCmdDrawIndexed-None-08607";
        index_binding_07312                      = "VUID-vkCmdDrawIndexed-None-07312";
        vertex_binding_04007                     = "VUID-vkCmdDrawIndexed-None-04007";
        vertex_binding_null_04008                = "VUID-vkCmdDrawIndexed-None-04008";
        compatible_pipeline_08600                = "VUID-vkCmdDrawIndexed-None-08600";
        render_pass_compatible_02684             = "VUID-vkCmdDrawIndexed-renderPass-02684";
        render_pass_began_08876                  = "VUID-vkCmdDrawIndexed-None-08876";
        subpass_index_02685                      = "VUID-vkCmdDrawIndexed-subpass-02685";
        sample_location_02689                    = "VUID-vkCmdDrawIndexed-sampleLocationsEnable-02689";
        linear_filter_sampler_04553              = "VUID-vkCmdDrawIndexed-magFilter-04553";
        linear_mipmap_sampler_04770              = "VUID-vkCmdDrawIndexed-mipmapMode-04770";
        cubic_sampler_02692                      = "VUID-vkCmdDrawIndexed-None-02692";
        viewport_count_03417                     = "VUID-vkCmdDrawIndexed-viewportCount-03417";
        scissor_count_03418                      = "VUID-vkCmdDrawIndexed-scissorCount-03418";
        viewport_scissor_count_03419             = "VUID-vkCmdDrawIndexed-viewportCount-03419";
        primitive_topology_class_07500           = "VUID-vkCmdDrawIndexed-dynamicPrimitiveTopologyUnrestricted-07500";
        corner_sampled_address_mode_02696        = "VUID-vkCmdDrawIndexed-flags-02696";
        imageview_atomic_02691                   = "VUID-vkCmdDrawIndexed-None-02691";
        bufferview_atomic_07888                  = "VUID-vkCmdDrawIndexed-None-07888";
        push_constants_set_08602                 = "VUID-vkCmdDrawIndexed-maintenance4-08602";
        image_subresources_render_pass_write_06537 = "VUID-vkCmdDrawIndexed-None-06537";
        image_subresources_subpass_read_09003    = "VUID-vkCmdDrawIndexed-None-09003";
        image_subresources_subpass_write_06539   = "VUID-vkCmdDrawIndexed-None-06539";
        sampler_imageview_type_08609             = "VUID-vkCmdDrawIndexed-None-08609";
        sampler_implicitLod_dref_proj_08610      = "VUID-vkCmdDrawIndexed-None-08610";
        sampler_bias_offset_08611                = "VUID-vkCmdDrawIndexed-None-08611";
        vertex_binding_attribute_02721           = "VUID-vkCmdDrawIndexed-None-02721";
        dynamic_state_setting_commands_08608     = "VUID-vkCmdDrawIndexed-None-08608";
        msrtss_rasterization_samples_07284       = "VUID-vkCmdDrawIndexed-multisampledRenderToSingleSampled-07284";
        unprotected_command_buffer_02707         = "VUID-vkCmdDrawIndexed-commandBuffer-02707";
        protected_command_buffer_02712           = "VUID-vkCmdDrawIndexed-commandBuffer-02712";
        max_multiview_instance_index_02688       = "VUID-vkCmdDrawIndexed-maxMultiviewInstanceIndex-02688";
        img_filter_cubic_02693                   = "VUID-vkCmdDrawIndexed-None-02693";
        filter_cubic_02694                       = "VUID-vkCmdDrawIndexed-filterCubic-02694";
        filter_cubic_min_max_02695               = "VUID-vkCmdDrawIndexed-filterCubicMinmax-02695";
        viewport_count_primitive_shading_rate_04552 = "VUID-vkCmdDrawIndexed-primitiveFragmentShadingRateWithMultipleViewports-04552";
        patch_control_points_04875               = "VUID-vkCmdDrawIndexed-None-04875";
        rasterizer_discard_enable_04876          = "VUID-vkCmdDrawIndexed-None-04876";
        depth_bias_enable_04877                  = "VUID-vkCmdDrawIndexed-None-04877";
        logic_op_04878                           = "VUID-vkCmdDrawIndexed-logicOp-04878";
        primitive_restart_enable_04879           = "VUID-vkCmdDrawIndexed-None-04879";
        vertex_input_04912                       = "VUID-vkCmdDrawIndexed-None-04912";
        vertex_input_binding_stride_04913        = "VUID-vkCmdDrawIndexed-pStrides-04913";
        vertex_input_04914                       = "VUID-vkCmdDrawIndexed-None-04914";
        blend_enable_04727                       = "VUID-vkCmdDrawIndexed-blendEnable-04727";
        dynamic_discard_rectangle_07751          = "VUID-vkCmdDrawIndexed-None-07751";
        dynamic_discard_rectangle_enable_07880   = "VUID-vkCmdDrawIndexed-None-07880";
        dynamic_discard_rectangle_mode_07881     =  "VUID-vkCmdDrawIndexed-None-07881";
        dynamic_exclusive_scissor_enable_07878   = "VUID-vkCmdDrawIndexed-None-07878";
        dynamic_exclusive_scissor_07879          = "VUID-vkCmdDrawIndexed-None-07879";
        dynamic_color_write_enable_07749         = "VUID-vkCmdDrawIndexed-None-07749";
        dynamic_color_write_enable_count_07750   = "VUID-vkCmdDrawIndexed-attachmentCount-07750";
        dynamic_attachment_feedback_loop_08877   = "VUID-vkCmdDrawIndexed-None-08877";
        dynamic_rendering_view_mask_06178        = "VUID-vkCmdDrawIndexed-viewMask-06178";
        dynamic_rendering_color_count_06179      = "VUID-vkCmdDrawIndexed-colorAttachmentCount-06179";
        dynamic_rendering_color_formats_08910    = "VUID-vkCmdDrawIndexed-dynamicRenderingUnusedAttachments-08910";
        dynamic_rendering_unused_attachments_08911 = "VUID-vkCmdDrawIndexed-dynamicRenderingUnusedAttachments-08911";
        dynamic_rendering_undefined_color_formats_08912 = "VUID-vkCmdDrawIndexed-dynamicRenderingUnusedAttachments-08912";
        dynamic_rendering_undefined_depth_format_08916 = "VUID-vkCmdDrawIndexed-dynamicRenderingUnusedAttachments-08916";
        dynamic_rendering_undefined_stencil_format_08916 = "VUID-vkCmdDrawIndexed-dynamicRenderingUnusedAttachments-08916";
        dynamic_rendering_depth_format_08914     = "VUID-vkCmdDrawIndexed-dynamicRenderingUnusedAttachments-08914";
        dynamic_rendering_unused_attachments_08915 = "VUID-vkCmdDrawIndexed-dynamicRenderingUnusedAttachments-08915";
        dynamic_rendering_stencil_format_08917   = "VUID-vkCmdDrawIndexed-dynamicRenderingUnusedAttachments-08917";
        dynamic_rendering_unused_attachments_08918 = "VUID-vkCmdDrawIndexed-dynamicRenderingUnusedAttachments-08918";
        dynamic_rendering_fsr_06183              = "VUID-vkCmdDrawIndexed-imageView-06183";
        dynamic_rendering_fdm_06184              = "VUID-vkCmdDrawIndexed-imageView-06184";
        dynamic_rendering_color_sample_06185     = "VUID-vkCmdDrawIndexed-colorAttachmentCount-06185";
        dynamic_rendering_depth_sample_06186     = "VUID-vkCmdDrawIndexed-pDepthAttachment-06186";
        dynamic_rendering_stencil_sample_06187   = "VUID-vkCmdDrawIndexed-pStencilAttachment-06187";
        dynamic_rendering_06198                  = "VUID-vkCmdDrawIndexed-renderPass-06198";
        dynamic_rendering_07285                  = "VUID-vkCmdDrawIndexed-multisampledRenderToSingleSampled-07285";
        dynamic_rendering_07286                  = "VUID-vkCmdDrawIndexed-multisampledRenderToSingleSampled-07286";
        dynamic_rendering_07287                  = "VUID-vkCmdDrawIndexed-multisampledRenderToSingleSampled-07287";
        image_view_access_64_04470               = "VUID-vkCmdDrawIndexed-SampledType-04470";
        image_view_access_32_04471               = "VUID-vkCmdDrawIndexed-SampledType-04471";
        image_view_sparse_64_04474               = "VUID-vkCmdDrawIndexed-sparseImageInt64Atomics-04474";
        buffer_view_access_64_04472              = "VUID-vkCmdDrawIndexed-SampledType-04472";
        buffer_view_access_32_04473              = "VUID-vkCmdDrawIndexed-SampledType-04473";
        storage_image_read_without_format_07028  = "VUID-vkCmdDrawIndexed-OpTypeImage-07028";
        storage_image_write_without_format_07027 = "VUID-vkCmdDrawIndexed-OpTypeImage-07027";
        storage_texel_buffer_read_without_format_07030  = "VUID-vkCmdDrawIndexed-OpTypeImage-07030";
        storage_texel_buffer_write_without_format_07029 = "VUID-vkCmdDrawIndexed-OpTypeImage-07029";
        storage_image_write_texel_count_08795           = "VUID-vkCmdDrawIndexed-OpImageWrite-08795";
        storage_image_write_texel_count_08796           = "VUID-vkCmdDrawIndexed-OpImageWrite-08796";
        storage_texel_buffer_write_texel_count_04469    = "VUID-vkCmdDrawIndexed-OpImageWrite-04469";
        depth_compare_sample_06479               = "VUID-vkCmdDrawIndexed-None-06479";
        depth_read_only_06886                    = "VUID-vkCmdDrawIndexed-None-06886";
        stencil_read_only_06887                  = "VUID-vkCmdDrawIndexed-None-06887";
        dynamic_sample_locations_06666           = "VUID-vkCmdDrawIndexed-None-06666";
        dynamic_tessellation_domain_origin_07619 = "VUID-vkCmdDrawIndexed-None-07619";
        dynamic_depth_clamp_enable_07620         = "VUID-vkCmdDrawIndexed-None-07620";
        dynamic_polygon_mode_07621               = "VUID-vkCmdDrawIndexed-None-07621";
        dynamic_rasterization_samples_07622      = "VUID-vkCmdDrawIndexed-None-07622";
        dynamic_sample_mask_07623                = "VUID-vkCmdDrawIndexed-None-07623";
        dynamic_alpha_to_coverage_enable_07624   = "VUID-vkCmdDrawIndexed-None-07624";
        dynamic_alpha_to_coverage_component_08919 = "VUID-vkCmdDrawIndexed-alphaToCoverageEnable-08919";
        dynamic_alpha_to_one_enable_07625        = "VUID-vkCmdDrawIndexed-None-07625";
        dynamic_logic_op_enable_07626            = "VUID-vkCmdDrawIndexed-None-07626";
        dynamic_color_blend_enable_07476         = "VUID-vkCmdDrawIndexed-firstAttachment-07476";
        dynamic_color_blend_equation_07477       = "VUID-vkCmdDrawIndexed-firstAttachment-07477";
        dynamic_color_write_mask_07478           = "VUID-vkCmdDrawIndexed-firstAttachment-07478";
        dynamic_rasterization_stream_07630       = "VUID-vkCmdDrawIndexed-None-07630";
        dynamic_conservative_rasterization_mode_07631 = "VUID-vkCmdDrawIndexed-None-07631";
        dynamic_extra_primitive_overestimation_size_07632 = "VUID-vkCmdDrawIndexed-None-07632";
        dynamic_depth_clip_enable_07633          = "VUID-vkCmdDrawIndexed-None-07633";
        dynamic_sample_locations_enable_07634    = "VUID-vkCmdDrawIndexed-None-07634";
        dynamic_color_blend_advanced_07479       = "VUID-vkCmdDrawIndexed-firstAttachment-07479";
        dynamic_provoking_vertex_mode_07636      = "VUID-vkCmdDrawIndexed-None-07636";
        dynamic_line_rasterization_mode_07637    = "VUID-vkCmdDrawIndexed-None-07637";
        dynamic_line_stipple_enable_07638        = "VUID-vkCmdDrawIndexed-None-07638";
        dynamic_depth_clip_negative_one_to_one_07639 = "VUID-vkCmdDrawIndexed-None-07639";
        dynamic_viewport_w_scaling_enable_07640  = "VUID-vkCmdDrawIndexed-None-07640";
        dynamic_viewport_swizzle_07641           = "VUID-vkCmdDrawIndexed-None-07641";
        dynamic_coverage_to_color_enable_07642   = "VUID-vkCmdDrawIndexed-None-07642";
        dynamic_coverage_to_color_location_07643 = "VUID-vkCmdDrawIndexed-None-07643";
        dynamic_coverage_modulation_mode_07644   = "VUID-vkCmdDrawIndexed-None-07644";
        dynamic_coverage_modulation_table_enable_07645 = "VUID-vkCmdDrawIndexed-None-07645";
        dynamic_coverage_modulation_table_07646  = "VUID-vkCmdDrawIndexed-None-07646";
        dynamic_shading_rate_image_enable_07647  = "VUID-vkCmdDrawIndexed-None-07647";
        dynamic_representative_fragment_test_enable_07648  = "VUID-vkCmdDrawIndexed-None-07648";
        dynamic_coverage_reduction_mode_07649    = "VUID-vkCmdDrawIndexed-None-07649";
        dynamic_viewport_07831                   = "VUID-vkCmdDrawIndexed-None-07831";
        dynamic_scissor_07832                    = "VUID-vkCmdDrawIndexed-None-07832";
        dynamic_depth_bias_07834                 = "VUID-vkCmdDrawIndexed-None-07834";
        dynamic_line_width_07833                 = "VUID-vkCmdDrawIndexed-None-07833";
        dynamic_line_stipple_ext_07849           = "VUID-vkCmdDrawIndexed-None-07849";
        dynamic_blend_constants_07835            = "VUID-vkCmdDrawIndexed-None-07835";
        dynamic_depth_bounds_07836               = "VUID-vkCmdDrawIndexed-None-07836";
        dynamic_depth_enable_08715               = "VUID-vkCmdDrawIndexed-pDynamicStates-08715";
        dynamic_stencil_compare_mask_07837       = "VUID-vkCmdDrawIndexed-None-07837";
        dynamic_stencil_write_mask_07838         = "VUID-vkCmdDrawIndexed-None-07838";
        dynamic_stencil_write_mask_08716         = "VUID-vkCmdDrawIndexed-pDynamicStates-08716";
        dynamic_stencil_reference_07839          = "VUID-vkCmdDrawIndexed-None-07839";
        dynamic_state_inherited_07850            = "VUID-vkCmdDrawIndexed-None-07850";
        dynamic_cull_mode_07840                  = "VUID-vkCmdDrawIndexed-None-07840";
        dynamic_front_face_07841                 = "VUID-vkCmdDrawIndexed-None-07841";
        dynamic_primitive_topology_07842         = "VUID-vkCmdDrawIndexed-None-07842";
        dynamic_depth_test_enable_07843          = "VUID-vkCmdDrawIndexed-None-07843";
        dynamic_depth_write_enable_07844         = "VUID-vkCmdDrawIndexed-None-07844";
        dynamic_depth_compare_op_07845           = "VUID-vkCmdDrawIndexed-None-07845";
        dynamic_depth_bound_test_enable_07846    = "VUID-vkCmdDrawIndexed-None-07846";
        dynamic_stencil_test_enable_07847        = "VUID-vkCmdDrawIndexed-None-07847";
        dynamic_stencil_op_07848                 = "VUID-vkCmdDrawIndexed-None-07848";
        primitives_generated_06708               = "VUID-vkCmdDrawIndexed-primitivesGeneratedQueryWithRasterizerDiscard-06708";
        primitives_generated_streams_06709       = "VUID-vkCmdDrawIndexed-primitivesGeneratedQueryWithNonZeroStreams-06709";
        invalid_mesh_shader_stages_06481         = "VUID-vkCmdDrawIndexed-stage-06481";
        descriptor_buffer_bit_set_08114          = "VUID-vkCmdDrawIndexed-None-08114";
        descriptor_buffer_bit_not_set_08115      = "VUID-vkCmdDrawIndexed-None-08115";
        descriptor_buffer_set_offset_missing_08117 = "VUID-vkCmdDrawIndexed-None-08117";
        image_view_dim_07752                     = "VUID-vkCmdDrawIndexed-viewType-07752";
        image_view_numeric_format_07753          = "VUID-vkCmdDrawIndexed-format-07753";
        stippled_rectangular_lines_07495         = "VUID-vkCmdDrawIndexed-stippledLineEnable-07495";
        stippled_bresenham_lines_07496           = "VUID-vkCmdDrawIndexed-stippledLineEnable-07496";
        stippled_smooth_lines_07497              = "VUID-vkCmdDrawIndexed-stippledLineEnable-07497";
        stippled_default_strict_07498            = "VUID-vkCmdDrawIndexed-stippledLineEnable-07498";
        viewport_and_scissor_with_count_08635    = "VUID-vkCmdDrawIndexed-None-08635";
        viewport_w_scaling_08636                 = "VUID-vkCmdDrawIndexed-None-08636";
        shading_rate_palette_08637               = "VUID-vkCmdDrawIndexed-None-08637";
        exclusive_scissor_08638                  = "VUID-vkCmdDrawIndexed-None-08638";
        external_format_resolve_09362            = "VUID-vkCmdDrawIndexed-colorAttachmentCount-09362";
        external_format_resolve_09363            = "VUID-vkCmdDrawIndexed-None-09363";
        external_format_resolve_09364            = "VUID-vkCmdDrawIndexed-None-09364";
        external_format_resolve_09365            = "VUID-vkCmdDrawIndexed-None-09365";
        external_format_resolve_09368            = "VUID-vkCmdDrawIndexed-None-09368";
        external_format_resolve_09369            = "VUID-vkCmdDrawIndexed-None-09369";
        external_format_resolve_09372            = "VUID-vkCmdDrawIndexed-colorAttachmentCount-09372";
        set_rasterizer_discard_enable_08639      = "VUID-vkCmdDrawIndexed-None-08639";
        set_depth_bias_enable_08640              = "VUID-vkCmdDrawIndexed-None-08640";
        set_logic_op_08641                       = "VUID-vkCmdDrawIndexed-None-08641";
        set_color_blend_enable_08643             = "VUID-vkCmdDrawIndexed-None-08643";
        set_rasterization_samples_08644              = "VUID-vkCmdDrawIndexed-None-08644";
        set_color_write_enable_08646             = "VUID-vkCmdDrawIndexed-None-08646";
        set_color_write_enable_08647             = "VUID-vkCmdDrawIndexed-None-08647";
        set_discard_rectangles_enable_08648      = "VUID-vkCmdDrawIndexed-None-08648";
        set_discard_rectangles_mode_08649      = "VUID-vkCmdDrawIndexed-None-08649";
        set_depth_clamp_enable_08650             = "VUID-vkCmdDrawIndexed-None-08650";
        set_polygon_mode_08651                   = "VUID-vkCmdDrawIndexed-None-08651";
        set_rasterization_samples_08652          = "VUID-vkCmdDrawIndexed-None-08652";
        set_sample_mask_08653                    = "VUID-vkCmdDrawIndexed-None-08653";
        set_alpha_to_coverage_enable_08654       = "VUID-vkCmdDrawIndexed-None-08654";
        set_alpha_to_one_enable_08655            = "VUID-vkCmdDrawIndexed-None-08655";
        set_logic_op_enable_08656                = "VUID-vkCmdDrawIndexed-None-08656";
        set_color_blend_enable_08657             = "VUID-vkCmdDrawIndexed-None-08657";
        set_color_blend_equation_08658           = "VUID-vkCmdDrawIndexed-None-08658";
        set_color_write_mask_08659               = "VUID-vkCmdDrawIndexed-None-08659";
        set_rasterization_streams_08660          = "VUID-vkCmdDrawIndexed-None-08660";
        set_conservative_rasterization_mode_08661 = "VUID-vkCmdDrawIndexed-None-08661";
        set_extra_primitive_overestimation_size_08662 = "VUID-vkCmdDrawIndexed-None-08662";
        set_depth_clip_enable_08663              = "VUID-vkCmdDrawIndexed-None-08663";
        set_sample_locations_enable_08664        = "VUID-vkCmdDrawIndexed-None-08664";
        set_provoking_vertex_mode_08665          = "VUID-vkCmdDrawIndexed-None-08665";
        set_line_rasterization_mode_08666        = "VUID-vkCmdDrawIndexed-None-08666";
        set_line_rasterization_mode_08667        = "VUID-vkCmdDrawIndexed-None-08667";
        set_line_rasterization_mode_08668        = "VUID-vkCmdDrawIndexed-None-08668";
        set_line_stipple_enable_08669            = "VUID-vkCmdDrawIndexed-None-08669";
        set_line_stipple_enable_08670            = "VUID-vkCmdDrawIndexed-None-08670";
        set_line_stipple_enable_08671            = "VUID-vkCmdDrawIndexed-None-08671";
        set_line_stipple_08672                   = "VUID-vkCmdDrawIndexed-None-08672";
        set_depth_clip_negative_one_to_one_08673 = "VUID-vkCmdDrawIndexed-None-08673";
        set_viewport_w_scaling_enable_08674      = "VUID-vkCmdDrawIndexed-None-08674";
        set_viewport_swizzle_08675               = "VUID-vkCmdDrawIndexed-None-08675";
        set_coverage_to_color_enable_08676       = "VUID-vkCmdDrawIndexed-None-08676";
        set_coverage_to_color_location_08677     = "VUID-vkCmdDrawIndexed-None-08677";
        set_coverage_modulation_mode_08678       = "VUID-vkCmdDrawIndexed-None-08678";
        set_coverage_modulation_table_enable_08679 = "VUID-vkCmdDrawIndexed-None-08679";
        set_coverage_modulation_table_08680      = "VUID-vkCmdDrawIndexed-None-08680";
        set_shading_rate_image_enable_08681      = "VUID-vkCmdDrawIndexed-None-08681";
        set_representative_fragment_test_enable_08682 = "VUID-vkCmdDrawIndexed-None-08682";
        set_coverage_reduction_mode_08683        = "VUID-vkCmdDrawIndexed-None-08683";
        vertex_shader_08684                      = "VUID-vkCmdDrawIndexed-None-08684";
        tessellation_control_shader_08685        = "VUID-vkCmdDrawIndexed-None-08685";
        tessellation_evaluation_shader_08686     = "VUID-vkCmdDrawIndexed-None-08686";
        geometry_shader_08687                    = "VUID-vkCmdDrawIndexed-None-08687";
        fragment_shader_08688                    = "VUID-vkCmdDrawIndexed-None-08688";
        task_shader_08689                        = "VUID-vkCmdDrawIndexed-None-08689";
        mesh_shader_08690                        = "VUID-vkCmdDrawIndexed-None-08690";
        vert_mesh_shader_08693                   = "VUID-vkCmdDrawIndexed-None-08693";
        task_mesh_shader_08694                   = "VUID-vkCmdDrawIndexed-None-08694";
        task_mesh_shader_08695                   = "VUID-vkCmdDrawIndexed-None-08695";
        vert_task_mesh_shader_08696              = "VUID-vkCmdDrawIndexed-None-08696";
        linked_shaders_08698                     = "VUID-vkCmdDrawIndexed-None-08698";
        linked_shaders_08699                     = "VUID-vkCmdDrawIndexed-None-08699";
        shaders_push_constants_08878             = "VUID-vkCmdDrawIndexed-None-08878";
        shaders_descriptor_layouts_08879         = "VUID-vkCmdDrawIndexed-None-08879";
        set_attachment_feedback_loop_enable_08880 = "VUID-vkCmdDrawIndexed-None-08880";
        set_vertex_input_08882                   = "VUID-vkCmdDrawIndexed-None-08882";
        draw_shaders_no_task_mesh_08885          = "VUID-vkCmdDrawIndexed-None-08885";
        set_line_width_08617                     = "VUID-vkCmdDrawIndexed-None-08617";
        set_line_width_08618                     = "VUID-vkCmdDrawIndexed-None-08618";
        set_depth_bias_08620                     = "VUID-vkCmdDrawIndexed-None-08620";
        set_blend_constants_08621                = "VUID-vkCmdDrawIndexed-None-08621";
        set_depth_bounds_08622                   = "VUID-vkCmdDrawIndexed-None-08622";
        set_stencil_compare_mask_08623           = "VUID-vkCmdDrawIndexed-None-08623";
        set_stencil_write_mask_08624             = "VUID-vkCmdDrawIndexed-None-08624";
        set_stencil_reference_08625              = "VUID-vkCmdDrawIndexed-None-08625";
        set_sample_locations_08626               = "VUID-vkCmdDrawIndexed-None-08626";
        set_cull_mode_08627                      = "VUID-vkCmdDrawIndexed-None-08627";
        set_front_face_08628                     = "VUID-vkCmdDrawIndexed-None-08628";
        set_depth_test_enable_08629              = "VUID-vkCmdDrawIndexed-None-08629";
        set_depth_write_enable_08630             = "VUID-vkCmdDrawIndexed-None-08630";
        set_depth_comapre_op_08631               = "VUID-vkCmdDrawIndexed-None-08631";
        set_depth_bounds_test_enable_08632       = "VUID-vkCmdDrawIndexed-None-08632";
        set_stencil_test_enable_08633            = "VUID-vkCmdDrawIndexed-None-08633";
        set_stencil_op_08634                     = "VUID-vkCmdDrawIndexed-None-08634";
        set_line_width_08619                     = "VUID-vkCmdDrawIndexed-None-08619";
        set_viewport_with_count_08642            = "VUID-vkCmdDrawIndexed-primitiveFragmentShadingRateWithMultipleViewports-08642";
        alpha_component_word_08920               = "VUID-vkCmdDrawIndexed-alphaToCoverageEnable-08920";
        color_write_mask_09116                   = "VUID-vkCmdDrawIndexed-None-09116";
        vertex_input_format_08936                = "VUID-vkCmdDrawIndexed-format-08936";
        vertex_input_format_08937                = "VUID-vkCmdDrawIndexed-format-08937";
        vertex_input_format_09203                = "VUID-vkCmdDrawIndexed-None-09203";
        vertex_input_format_07939                = "VUID-vkCmdDrawIndexed-Input-07939";
        set_clip_space_w_scaling_09232           = "VUID-vkCmdDrawIndexed-None-09232";
        set_discard_rectangle_09236              = "VUID-vkCmdDrawIndexed-rasterizerDiscardEnable-09236";
        set_viewport_coarse_sample_order_09233   = "VUID-vkCmdDrawIndexed-shadingRateImage-09233";
        set_viewport_shading_rate_palette_09234  = "VUID-vkCmdDrawIndexed-shadingRateImage-09234";
        set_exclusive_scissor_enable_09235       = "VUID-vkCmdDrawIndexed-exclusiveScissor-09235";
        set_fragment_shading_rate_09238          = "VUID-vkCmdDrawIndexed-pipelineFragmentShadingRate-09238";
        set_tessellation_domain_origin_09237     = "VUID-vkCmdDrawIndexed-None-09237";
        rasterization_samples_07935              = "VUID-vkCmdDrawIndexed-pNext-07935";
        mesh_shader_queries_07073                = "VUID-vkCmdDrawIndexed-stage-07073";
        blend_advanced_07480                     = "VUID-vkCmdDrawIndexed-advancedBlendMaxColorAttachments-07480";
        blend_feature_07470                      = "VUID-vkCmdDrawIndexed-pColorBlendEnables-07470";
        color_attachment_08963                   = "VUID-vkCmdDrawIndexed-pColorAttachments-08963";
        depth_attachment_08964                   = "VUID-vkCmdDrawIndexed-pDepthAttachment-08964";
        stencil_attachment_08965                 = "VUID-vkCmdDrawIndexed-pStencilAttachment-08965";
        sample_locations_07482                   = "VUID-vkCmdDrawIndexed-sampleLocationsPerPixel-07482";
    }
};

struct DispatchVuidsCmdDrawMultiIndexedEXT : DrawDispatchVuid {
    DispatchVuidsCmdDrawMultiIndexedEXT() : DrawDispatchVuid() {
        pipeline_bound_08606                     = "VUID-vkCmdDrawMultiIndexedEXT-None-08606";
        pipeline_or_shaders_bound_08607          = "VUID-vkCmdDrawMultiIndexedEXT-None-08607";
        index_binding_07312                      = "VUID-vkCmdDrawMultiIndexedEXT-None-07312";
        vertex_binding_04007                     = "VUID-vkCmdDrawMultiIndexedEXT-None-04007";
        vertex_binding_null_04008                = "VUID-vkCmdDrawMultiIndexedEXT-None-04008";
        compatible_pipeline_08600                = "VUID-vkCmdDrawMultiIndexedEXT-None-08600";
        render_pass_compatible_02684             = "VUID-vkCmdDrawMultiIndexedEXT-renderPass-02684";
        render_pass_began_08876                  = "VUID-vkCmdDrawMultiIndexedEXT-None-08876";
        subpass_index_02685                      = "VUID-vkCmdDrawMultiIndexedEXT-subpass-02685";
        sample_location_02689                    = "VUID-vkCmdDrawMultiIndexedEXT-sampleLocationsEnable-02689";
        linear_filter_sampler_04553              = "VUID-vkCmdDrawMultiIndexedEXT-magFilter-04553";
        linear_mipmap_sampler_04770              = "VUID-vkCmdDrawMultiIndexedEXT-mipmapMode-04770";
        cubic_sampler_02692                      = "VUID-vkCmdDrawMultiIndexedEXT-None-02692";
        viewport_count_03417                     = "VUID-vkCmdDrawMultiIndexedEXT-viewportCount-03417";
        scissor_count_03418                      = "VUID-vkCmdDrawMultiIndexedEXT-scissorCount-03418";
        viewport_scissor_count_03419             = "VUID-vkCmdDrawMultiIndexedEXT-viewportCount-03419";
        primitive_topology_class_07500           = "VUID-vkCmdDrawMultiIndexedEXT-dynamicPrimitiveTopologyUnrestricted-07500";
        corner_sampled_address_mode_02696        = "VUID-vkCmdDrawMultiIndexedEXT-flags-02696";
        imageview_atomic_02691                   = "VUID-vkCmdDrawMultiIndexedEXT-None-02691";
        bufferview_atomic_07888                  = "VUID-vkCmdDrawMultiIndexedEXT-None-07888";
        push_constants_set_08602                 = "VUID-vkCmdDrawMultiIndexedEXT-maintenance4-08602";
        image_subresources_render_pass_write_06537 = "VUID-vkCmdDrawMultiIndexedEXT-None-06537";
        image_subresources_subpass_read_09003    = "VUID-vkCmdDrawMultiIndexedEXT-None-09003";
        image_subresources_subpass_write_06539   = "VUID-vkCmdDrawMultiIndexedEXT-None-06539";
        sampler_imageview_type_08609             = "VUID-vkCmdDrawMultiIndexedEXT-None-08609";
        sampler_implicitLod_dref_proj_08610      = "VUID-vkCmdDrawMultiIndexedEXT-None-08610";
        sampler_bias_offset_08611                = "VUID-vkCmdDrawMultiIndexedEXT-None-08611";
        vertex_binding_attribute_02721           = "VUID-vkCmdDrawMultiIndexedEXT-None-02721";
        dynamic_state_setting_commands_08608     = "VUID-vkCmdDrawMultiIndexedEXT-None-08608";
        msrtss_rasterization_samples_07284       = "VUID-vkCmdDrawMultiIndexedEXT-multisampledRenderToSingleSampled-07284";
        unprotected_command_buffer_02707         = "VUID-vkCmdDrawMultiIndexedEXT-commandBuffer-02707";
        protected_command_buffer_02712           = "VUID-vkCmdDrawMultiIndexedEXT-commandBuffer-02712";
        max_multiview_instance_index_02688       = "VUID-vkCmdDrawMultiIndexedEXT-maxMultiviewInstanceIndex-02688";
        img_filter_cubic_02693                   = "VUID-vkCmdDrawMultiIndexedEXT-None-02693";
        filter_cubic_02694                       = "VUID-vkCmdDrawMultiIndexedEXT-filterCubic-02694";
        filter_cubic_min_max_02695               = "VUID-vkCmdDrawMultiIndexedEXT-filterCubicMinmax-02695";
        viewport_count_primitive_shading_rate_04552 = "VUID-vkCmdDrawMultiIndexedEXT-primitiveFragmentShadingRateWithMultipleViewports-04552";
        patch_control_points_04875               = "VUID-vkCmdDrawMultiIndexedEXT-None-04875";
        rasterizer_discard_enable_04876          = "VUID-vkCmdDrawMultiIndexedEXT-None-04876";
        depth_bias_enable_04877                  = "VUID-vkCmdDrawMultiIndexedEXT-None-04877";
        logic_op_04878                           = "VUID-vkCmdDrawMultiIndexedEXT-logicOp-04878";
        primitive_restart_enable_04879           = "VUID-vkCmdDrawMultiIndexedEXT-None-04879";
        vertex_input_04912                       = "VUID-vkCmdDrawMultiIndexedEXT-None-04912";
        vertex_input_binding_stride_04913        = "VUID-vkCmdDrawMultiIndexedEXT-pStrides-04913";
        vertex_input_04914                       = "VUID-vkCmdDrawMultiIndexedEXT-None-04914";
        blend_enable_04727                       = "VUID-vkCmdDrawMultiIndexedEXT-blendEnable-04727";
        dynamic_discard_rectangle_07751          = "VUID-vkCmdDrawMultiIndexedEXT-None-07751";
        dynamic_discard_rectangle_enable_07880   = "VUID-vkCmdDrawMultiIndexedEXT-None-07880";
        dynamic_discard_rectangle_mode_07881     =  "VUID-vkCmdDrawMultiIndexedEXT-None-07881";
        dynamic_exclusive_scissor_enable_07878   = "VUID-vkCmdDrawMultiIndexedEXT-None-07878";
        dynamic_exclusive_scissor_07879          = "VUID-vkCmdDrawMultiIndexedEXT-None-07879";
        dynamic_color_write_enable_07749         = "VUID-vkCmdDrawMultiIndexedEXT-None-07749";
        dynamic_color_write_enable_count_07750   = "VUID-vkCmdDrawMultiIndexedEXT-attachmentCount-07750";
        dynamic_attachment_feedback_loop_08877   = "VUID-vkCmdDrawMultiIndexedEXT-None-08877";
        dynamic_rendering_view_mask_06178        = "VUID-vkCmdDrawMultiIndexedEXT-viewMask-06178";
        dynamic_rendering_color_count_06179      = "VUID-vkCmdDrawMultiIndexedEXT-colorAttachmentCount-06179";
        dynamic_rendering_color_formats_08910    = "VUID-vkCmdDrawMultiIndexedEXT-dynamicRenderingUnusedAttachments-08910";
        dynamic_rendering_unused_attachments_08911 = "VUID-vkCmdDrawMultiIndexedEXT-dynamicRenderingUnusedAttachments-08911";
        dynamic_rendering_undefined_color_formats_08912 = "VUID-vkCmdDrawMultiIndexedEXT-dynamicRenderingUnusedAttachments-08912";
        dynamic_rendering_undefined_depth_format_08916 = "VUID-vkCmdDrawMultiIndexedEXT-dynamicRenderingUnusedAttachments-08916";
        dynamic_rendering_undefined_stencil_format_08916 = "VUID-vkCmdDrawMultiIndexedEXT-dynamicRenderingUnusedAttachments-08916";
        dynamic_rendering_depth_format_08914     = "VUID-vkCmdDrawMultiIndexedEXT-dynamicRenderingUnusedAttachments-08914";
        dynamic_rendering_unused_attachments_08915 = "VUID-vkCmdDrawMultiIndexedEXT-dynamicRenderingUnusedAttachments-08915";
        dynamic_rendering_stencil_format_08917   = "VUID-vkCmdDrawMultiIndexedEXT-dynamicRenderingUnusedAttachments-08917";
        dynamic_rendering_unused_attachments_08918 = "VUID-vkCmdDrawMultiIndexedEXT-dynamicRenderingUnusedAttachments-08918";
        dynamic_rendering_fsr_06183              = "VUID-vkCmdDrawMultiIndexedEXT-imageView-06183";
        dynamic_rendering_fdm_06184              = "VUID-vkCmdDrawMultiIndexedEXT-imageView-06184";
        dynamic_rendering_color_sample_06185     = "VUID-vkCmdDrawMultiIndexedEXT-colorAttachmentCount-06185";
        dynamic_rendering_depth_sample_06186     = "VUID-vkCmdDrawMultiIndexedEXT-pDepthAttachment-06186";
        dynamic_rendering_stencil_sample_06187   = "VUID-vkCmdDrawMultiIndexedEXT-pStencilAttachment-06187";
        dynamic_rendering_06198                  = "VUID-vkCmdDrawMultiIndexedEXT-renderPass-06198";
        dynamic_rendering_07285                  = "VUID-vkCmdDrawMultiIndexedEXT-multisampledRenderToSingleSampled-07285";
        dynamic_rendering_07286                  = "VUID-vkCmdDrawMultiIndexedEXT-multisampledRenderToSingleSampled-07286";
        dynamic_rendering_07287                  = "VUID-vkCmdDrawMultiIndexedEXT-multisampledRenderToSingleSampled-07287";
        image_view_access_64_04470               = "VUID-vkCmdDrawMultiIndexedEXT-SampledType-04470";
        image_view_access_32_04471               = "VUID-vkCmdDrawMultiIndexedEXT-SampledType-04471";
        image_view_sparse_64_04474               = "VUID-vkCmdDrawMultiIndexedEXT-sparseImageInt64Atomics-04474";
        buffer_view_access_64_04472              = "VUID-vkCmdDrawMultiIndexedEXT-SampledType-04472";
        buffer_view_access_32_04473              = "VUID-vkCmdDrawMultiIndexedEXT-SampledType-04473";
        storage_image_read_without_format_07028  = "VUID-vkCmdDrawMultiIndexedEXT-OpTypeImage-07028";
        storage_image_write_without_format_07027 = "VUID-vkCmdDrawMultiIndexedEXT-OpTypeImage-07027";
        storage_texel_buffer_read_without_format_07030  = "VUID-vkCmdDrawMultiIndexedEXT-OpTypeImage-07030";
        storage_texel_buffer_write_without_format_07029 = "VUID-vkCmdDrawMultiIndexedEXT-OpTypeImage-07029";
        storage_image_write_texel_count_08795           = "VUID-vkCmdDrawMultiIndexedEXT-OpImageWrite-08795";
        storage_image_write_texel_count_08796           = "VUID-vkCmdDrawMultiIndexedEXT-OpImageWrite-08796";
        storage_texel_buffer_write_texel_count_04469    = "VUID-vkCmdDrawMultiIndexedEXT-OpImageWrite-04469";
        depth_compare_sample_06479               = "VUID-vkCmdDrawMultiIndexedEXT-None-06479";
        depth_read_only_06886                    = "VUID-vkCmdDrawMultiIndexedEXT-None-06886";
        stencil_read_only_06887                  = "VUID-vkCmdDrawMultiIndexedEXT-None-06887";
        dynamic_sample_locations_06666           = "VUID-vkCmdDrawMultiIndexedEXT-None-06666";
        dynamic_tessellation_domain_origin_07619 = "VUID-vkCmdDrawMultiIndexedEXT-None-07619";
        dynamic_depth_clamp_enable_07620         = "VUID-vkCmdDrawMultiIndexedEXT-None-07620";
        dynamic_polygon_mode_07621               = "VUID-vkCmdDrawMultiIndexedEXT-None-07621";
        dynamic_rasterization_samples_07622      = "VUID-vkCmdDrawMultiIndexedEXT-None-07622";
        dynamic_sample_mask_07623                = "VUID-vkCmdDrawMultiIndexedEXT-None-07623";
        dynamic_alpha_to_coverage_enable_07624   = "VUID-vkCmdDrawMultiIndexedEXT-None-07624";
        dynamic_alpha_to_coverage_component_08919 = "VUID-vkCmdDrawMultiIndexedEXT-alphaToCoverageEnable-08919";
        dynamic_alpha_to_one_enable_07625        = "VUID-vkCmdDrawMultiIndexedEXT-None-07625";
        dynamic_logic_op_enable_07626            = "VUID-vkCmdDrawMultiIndexedEXT-None-07626";
        dynamic_color_blend_enable_07476         = "VUID-vkCmdDrawMultiIndexedEXT-firstAttachment-07476";
        dynamic_color_blend_equation_07477       = "VUID-vkCmdDrawMultiIndexedEXT-firstAttachment-07477";
        dynamic_color_write_mask_07478           = "VUID-vkCmdDrawMultiIndexedEXT-firstAttachment-07478";
        dynamic_rasterization_stream_07630       = "VUID-vkCmdDrawMultiIndexedEXT-None-07630";
        dynamic_conservative_rasterization_mode_07631 = "VUID-vkCmdDrawMultiIndexedEXT-None-07631";
        dynamic_extra_primitive_overestimation_size_07632 = "VUID-vkCmdDrawMultiIndexedEXT-None-07632";
        dynamic_depth_clip_enable_07633          = "VUID-vkCmdDrawMultiIndexedEXT-None-07633";
        dynamic_sample_locations_enable_07634    = "VUID-vkCmdDrawMultiIndexedEXT-None-07634";
        dynamic_color_blend_advanced_07479       = "VUID-vkCmdDrawMultiIndexedEXT-firstAttachment-07479";
        dynamic_provoking_vertex_mode_07636      = "VUID-vkCmdDrawMultiIndexedEXT-None-07636";
        dynamic_line_rasterization_mode_07637    = "VUID-vkCmdDrawMultiIndexedEXT-None-07637";
        dynamic_line_stipple_enable_07638        = "VUID-vkCmdDrawMultiIndexedEXT-None-07638";
        dynamic_depth_clip_negative_one_to_one_07639 = "VUID-vkCmdDrawMultiIndexedEXT-None-07639";
        dynamic_viewport_w_scaling_enable_07640  = "VUID-vkCmdDrawMultiIndexedEXT-None-07640";
        dynamic_viewport_swizzle_07641           = "VUID-vkCmdDrawMultiIndexedEXT-None-07641";
        dynamic_coverage_to_color_enable_07642   = "VUID-vkCmdDrawMultiIndexedEXT-None-07642";
        dynamic_coverage_to_color_location_07643 = "VUID-vkCmdDrawMultiIndexedEXT-None-07643";
        dynamic_coverage_modulation_mode_07644   = "VUID-vkCmdDrawMultiIndexedEXT-None-07644";
        dynamic_coverage_modulation_table_enable_07645 = "VUID-vkCmdDrawMultiIndexedEXT-None-07645";
        dynamic_coverage_modulation_table_07646  = "VUID-vkCmdDrawMultiIndexedEXT-None-07646";
        dynamic_shading_rate_image_enable_07647  = "VUID-vkCmdDrawMultiIndexedEXT-None-07647";
        dynamic_representative_fragment_test_enable_07648  = "VUID-vkCmdDrawMultiIndexedEXT-None-07648";
        dynamic_coverage_reduction_mode_07649    = "VUID-vkCmdDrawMultiIndexedEXT-None-07649";
        dynamic_viewport_07831                   = "VUID-vkCmdDrawMultiIndexedEXT-None-07831";
        dynamic_scissor_07832                    = "VUID-vkCmdDrawMultiIndexedEXT-None-07832";
        dynamic_depth_bias_07834                 = "VUID-vkCmdDrawMultiIndexedEXT-None-07834";
        dynamic_line_width_07833                 = "VUID-vkCmdDrawMultiIndexedEXT-None-07833";
        dynamic_line_stipple_ext_07849           = "VUID-vkCmdDrawMultiIndexedEXT-None-07849";
        dynamic_blend_constants_07835            = "VUID-vkCmdDrawMultiIndexedEXT-None-07835";
        dynamic_depth_bounds_07836               = "VUID-vkCmdDrawMultiIndexedEXT-None-07836";
        dynamic_depth_enable_08715               = "VUID-vkCmdDrawMultiIndexedEXT-pDynamicStates-08715";
        dynamic_stencil_compare_mask_07837       = "VUID-vkCmdDrawMultiIndexedEXT-None-07837";
        dynamic_stencil_write_mask_07838         = "VUID-vkCmdDrawMultiIndexedEXT-None-07838";
        dynamic_stencil_write_mask_08716         = "VUID-vkCmdDrawMultiIndexedEXT-pDynamicStates-08716";
        dynamic_stencil_reference_07839          = "VUID-vkCmdDrawMultiIndexedEXT-None-07839";
        dynamic_state_inherited_07850            = "VUID-vkCmdDrawMultiIndexedEXT-None-07850";
        dynamic_cull_mode_07840                  = "VUID-vkCmdDrawMultiIndexedEXT-None-07840";
        dynamic_front_face_07841                 = "VUID-vkCmdDrawMultiIndexedEXT-None-07841";
        dynamic_primitive_topology_07842         = "VUID-vkCmdDrawMultiIndexedEXT-None-07842";
        dynamic_depth_test_enable_07843          = "VUID-vkCmdDrawMultiIndexedEXT-None-07843";
        dynamic_depth_write_enable_07844         = "VUID-vkCmdDrawMultiIndexedEXT-None-07844";
        dynamic_depth_compare_op_07845           = "VUID-vkCmdDrawMultiIndexedEXT-None-07845";
        dynamic_depth_bound_test_enable_07846    = "VUID-vkCmdDrawMultiIndexedEXT-None-07846";
        dynamic_stencil_test_enable_07847        = "VUID-vkCmdDrawMultiIndexedEXT-None-07847";
        dynamic_stencil_op_07848                 = "VUID-vkCmdDrawMultiIndexedEXT-None-07848";
        primitives_generated_06708               = "VUID-vkCmdDrawMultiIndexedEXT-primitivesGeneratedQueryWithRasterizerDiscard-06708";
        primitives_generated_streams_06709       = "VUID-vkCmdDrawMultiIndexedEXT-primitivesGeneratedQueryWithNonZeroStreams-06709";
        invalid_mesh_shader_stages_06481         = "VUID-vkCmdDrawMultiIndexedEXT-stage-06481";
        descriptor_buffer_bit_set_08114          = "VUID-vkCmdDrawMultiIndexedEXT-None-08114";
        descriptor_buffer_bit_not_set_08115      = "VUID-vkCmdDrawMultiIndexedEXT-None-08115";
        descriptor_buffer_set_offset_missing_08117 = "VUID-vkCmdDrawMultiIndexedEXT-None-08117";
        image_view_dim_07752                     = "VUID-vkCmdDrawMultiIndexedEXT-viewType-07752";
        image_view_numeric_format_07753          = "VUID-vkCmdDrawMultiIndexedEXT-format-07753";
        stippled_rectangular_lines_07495         = "VUID-vkCmdDrawMultiIndexedEXT-stippledLineEnable-07495";
        stippled_bresenham_lines_07496           = "VUID-vkCmdDrawMultiIndexedEXT-stippledLineEnable-07496";
        stippled_smooth_lines_07497              = "VUID-vkCmdDrawMultiIndexedEXT-stippledLineEnable-07497";
        stippled_default_strict_07498            = "VUID-vkCmdDrawMultiIndexedEXT-stippledLineEnable-07498";
        viewport_and_scissor_with_count_08635    = "VUID-vkCmdDrawMultiIndexedEXT-None-08635";
        viewport_w_scaling_08636                 = "VUID-vkCmdDrawMultiIndexedEXT-None-08636";
        shading_rate_palette_08637               = "VUID-vkCmdDrawMultiIndexedEXT-None-08637";
        exclusive_scissor_08638                  = "VUID-vkCmdDrawMultiIndexedEXT-None-08638";
        external_format_resolve_09362            = "VUID-vkCmdDrawMultiIndexedEXT-colorAttachmentCount-09362";
        external_format_resolve_09363            = "VUID-vkCmdDrawMultiIndexedEXT-None-09363";
        external_format_resolve_09364            = "VUID-vkCmdDrawMultiIndexedEXT-None-09364";
        external_format_resolve_09365            = "VUID-vkCmdDrawMultiIndexedEXT-None-09365";
        external_format_resolve_09368            = "VUID-vkCmdDrawMultiIndexedEXT-None-09368";
        external_format_resolve_09369            = "VUID-vkCmdDrawMultiIndexedEXT-None-09369";
        external_format_resolve_09372            = "VUID-vkCmdDrawMultiIndexedEXT-colorAttachmentCount-09372";
        set_rasterizer_discard_enable_08639      = "VUID-vkCmdDrawMultiIndexedEXT-None-08639";
        set_depth_bias_enable_08640              = "VUID-vkCmdDrawMultiIndexedEXT-None-08640";
        set_logic_op_08641                       = "VUID-vkCmdDrawMultiIndexedEXT-None-08641";
        set_color_blend_enable_08643             = "VUID-vkCmdDrawMultiIndexedEXT-None-08643";
        set_rasterization_samples_08644              = "VUID-vkCmdDrawMultiIndexedEXT-None-08644";
        set_color_write_enable_08646             = "VUID-vkCmdDrawMultiIndexedEXT-None-08646";
        set_color_write_enable_08647             = "VUID-vkCmdDrawMultiIndexedEXT-None-08647";
        set_discard_rectangles_enable_08648      = "VUID-vkCmdDrawMultiIndexedEXT-None-08648";
        set_discard_rectangles_mode_08649      = "VUID-vkCmdDrawMultiIndexedEXT-None-08649";
        set_depth_clamp_enable_08650             = "VUID-vkCmdDrawMultiIndexedEXT-None-08650";
        set_polygon_mode_08651                   = "VUID-vkCmdDrawMultiIndexedEXT-None-08651";
        set_rasterization_samples_08652          = "VUID-vkCmdDrawMultiIndexedEXT-None-08652";
        set_sample_mask_08653                    = "VUID-vkCmdDrawMultiIndexedEXT-None-08653";
        set_alpha_to_coverage_enable_08654       = "VUID-vkCmdDrawMultiIndexedEXT-None-08654";
        set_alpha_to_one_enable_08655            = "VUID-vkCmdDrawMultiIndexedEXT-None-08655";
        set_logic_op_enable_08656                = "VUID-vkCmdDrawMultiIndexedEXT-None-08656";
        set_color_blend_enable_08657             = "VUID-vkCmdDrawMultiIndexedEXT-None-08657";
        set_color_blend_equation_08658           = "VUID-vkCmdDrawMultiIndexedEXT-None-08658";
        set_color_write_mask_08659               = "VUID-vkCmdDrawMultiIndexedEXT-None-08659";
        set_rasterization_streams_08660          = "VUID-vkCmdDrawMultiIndexedEXT-None-08660";
        set_conservative_rasterization_mode_08661 = "VUID-vkCmdDrawMultiIndexedEXT-None-08661";
        set_extra_primitive_overestimation_size_08662 = "VUID-vkCmdDrawMultiIndexedEXT-None-08662";
        set_depth_clip_enable_08663              = "VUID-vkCmdDrawMultiIndexedEXT-None-08663";
        set_sample_locations_enable_08664        = "VUID-vkCmdDrawMultiIndexedEXT-None-08664";
        set_provoking_vertex_mode_08665          = "VUID-vkCmdDrawMultiIndexedEXT-None-08665";
        set_line_rasterization_mode_08666        = "VUID-vkCmdDrawMultiIndexedEXT-None-08666";
        set_line_rasterization_mode_08667        = "VUID-vkCmdDrawMultiIndexedEXT-None-08667";
        set_line_rasterization_mode_08668        = "VUID-vkCmdDrawMultiIndexedEXT-None-08668";
        set_line_stipple_enable_08669            = "VUID-vkCmdDrawMultiIndexedEXT-None-08669";
        set_line_stipple_enable_08670            = "VUID-vkCmdDrawMultiIndexedEXT-None-08670";
        set_line_stipple_enable_08671            = "VUID-vkCmdDrawMultiIndexedEXT-None-08671";
        set_line_stipple_08672                   = "VUID-vkCmdDrawMultiIndexedEXT-None-08672";
        set_depth_clip_negative_one_to_one_08673 = "VUID-vkCmdDrawMultiIndexedEXT-None-08673";
        set_viewport_w_scaling_enable_08674      = "VUID-vkCmdDrawMultiIndexedEXT-None-08674";
        set_viewport_swizzle_08675               = "VUID-vkCmdDrawMultiIndexedEXT-None-08675";
        set_coverage_to_color_enable_08676       = "VUID-vkCmdDrawMultiIndexedEXT-None-08676";
        set_coverage_to_color_location_08677     = "VUID-vkCmdDrawMultiIndexedEXT-None-08677";
        set_coverage_modulation_mode_08678       = "VUID-vkCmdDrawMultiIndexedEXT-None-08678";
        set_coverage_modulation_table_enable_08679 = "VUID-vkCmdDrawMultiIndexedEXT-None-08679";
        set_coverage_modulation_table_08680      = "VUID-vkCmdDrawMultiIndexedEXT-None-08680";
        set_shading_rate_image_enable_08681      = "VUID-vkCmdDrawMultiIndexedEXT-None-08681";
        set_representative_fragment_test_enable_08682 = "VUID-vkCmdDrawMultiIndexedEXT-None-08682";
        set_coverage_reduction_mode_08683        = "VUID-vkCmdDrawMultiIndexedEXT-None-08683";
        vertex_shader_08684                      = "VUID-vkCmdDrawMultiIndexedEXT-None-08684";
        tessellation_control_shader_08685        = "VUID-vkCmdDrawMultiIndexedEXT-None-08685";
        tessellation_evaluation_shader_08686     = "VUID-vkCmdDrawMultiIndexedEXT-None-08686";
        geometry_shader_08687                    = "VUID-vkCmdDrawMultiIndexedEXT-None-08687";
        fragment_shader_08688                    = "VUID-vkCmdDrawMultiIndexedEXT-None-08688";
        task_shader_08689                        = "VUID-vkCmdDrawMultiIndexedEXT-None-08689";
        mesh_shader_08690                        = "VUID-vkCmdDrawMultiIndexedEXT-None-08690";
        vert_mesh_shader_08693                   = "VUID-vkCmdDrawMultiIndexedEXT-None-08693";
        task_mesh_shader_08694                   = "VUID-vkCmdDrawMultiIndexedEXT-None-08694";
        task_mesh_shader_08695                   = "VUID-vkCmdDrawMultiIndexedEXT-None-08695";
        vert_task_mesh_shader_08696              = "VUID-vkCmdDrawMultiIndexedEXT-None-08696";
        linked_shaders_08698                     = "VUID-vkCmdDrawMultiIndexedEXT-None-08698";
        linked_shaders_08699                     = "VUID-vkCmdDrawMultiIndexedEXT-None-08699";
        shaders_push_constants_08878             = "VUID-vkCmdDrawMultiIndexedEXT-None-08878";
        shaders_descriptor_layouts_08879         = "VUID-vkCmdDrawMultiIndexedEXT-None-08879";
        set_attachment_feedback_loop_enable_08880 = "VUID-vkCmdDrawMultiIndexedEXT-None-08880";
        set_vertex_input_08882                   = "VUID-vkCmdDrawMultiIndexedEXT-None-08882";
        draw_shaders_no_task_mesh_08885          = "VUID-vkCmdDrawMultiIndexedEXT-None-08885";
        set_line_width_08617                     = "VUID-vkCmdDrawMultiIndexedEXT-None-08617";
        set_line_width_08618                     = "VUID-vkCmdDrawMultiIndexedEXT-None-08618";
        set_depth_bias_08620                     = "VUID-vkCmdDrawMultiIndexedEXT-None-08620";
        set_blend_constants_08621                = "VUID-vkCmdDrawMultiIndexedEXT-None-08621";
        set_depth_bounds_08622                   = "VUID-vkCmdDrawMultiIndexedEXT-None-08622";
        set_stencil_compare_mask_08623           = "VUID-vkCmdDrawMultiIndexedEXT-None-08623";
        set_stencil_write_mask_08624             = "VUID-vkCmdDrawMultiIndexedEXT-None-08624";
        set_stencil_reference_08625              = "VUID-vkCmdDrawMultiIndexedEXT-None-08625";
        set_sample_locations_08626               = "VUID-vkCmdDrawMultiIndexedEXT-None-08626";
        set_cull_mode_08627                      = "VUID-vkCmdDrawMultiIndexedEXT-None-08627";
        set_front_face_08628                     = "VUID-vkCmdDrawMultiIndexedEXT-None-08628";
        set_depth_test_enable_08629              = "VUID-vkCmdDrawMultiIndexedEXT-None-08629";
        set_depth_write_enable_08630             = "VUID-vkCmdDrawMultiIndexedEXT-None-08630";
        set_depth_comapre_op_08631               = "VUID-vkCmdDrawMultiIndexedEXT-None-08631";
        set_depth_bounds_test_enable_08632       = "VUID-vkCmdDrawMultiIndexedEXT-None-08632";
        set_stencil_test_enable_08633            = "VUID-vkCmdDrawMultiIndexedEXT-None-08633";
        set_stencil_op_08634                     = "VUID-vkCmdDrawMultiIndexedEXT-None-08634";
        set_line_width_08619                     = "VUID-vkCmdDrawMultiIndexedEXT-None-08619";
        set_viewport_with_count_08642            = "VUID-vkCmdDrawMultiIndexedEXT-primitiveFragmentShadingRateWithMultipleViewports-08642";
        alpha_component_word_08920               = "VUID-vkCmdDrawMultiIndexedEXT-alphaToCoverageEnable-08920";
        color_write_mask_09116                   = "VUID-vkCmdDrawMultiIndexedEXT-None-09116";
        vertex_input_format_08936                = "VUID-vkCmdDrawMultiIndexedEXT-format-08936";
        vertex_input_format_08937                = "VUID-vkCmdDrawMultiIndexedEXT-format-08937";
        vertex_input_format_09203                = "VUID-vkCmdDrawMultiIndexedEXT-None-09203";
        vertex_input_format_07939                = "VUID-vkCmdDrawMultiIndexedEXT-Input-07939";
        set_clip_space_w_scaling_09232           = "VUID-vkCmdDrawMultiIndexedEXT-None-09232";
        set_discard_rectangle_09236              = "VUID-vkCmdDrawMultiIndexedEXT-rasterizerDiscardEnable-09236";
        set_viewport_coarse_sample_order_09233   = "VUID-vkCmdDrawMultiIndexedEXT-shadingRateImage-09233";
        set_viewport_shading_rate_palette_09234  = "VUID-vkCmdDrawMultiIndexedEXT-shadingRateImage-09234";
        set_exclusive_scissor_enable_09235       = "VUID-vkCmdDrawMultiIndexedEXT-exclusiveScissor-09235";
        set_fragment_shading_rate_09238          = "VUID-vkCmdDrawMultiIndexedEXT-pipelineFragmentShadingRate-09238";
        set_tessellation_domain_origin_09237     = "VUID-vkCmdDrawMultiIndexedEXT-None-09237";
        rasterization_samples_07935              = "VUID-vkCmdDrawMultiIndexedEXT-pNext-07935";
        mesh_shader_queries_07073                = "VUID-vkCmdDrawMultiIndexedEXT-stage-07073";
        blend_advanced_07480                     = "VUID-vkCmdDrawMultiIndexedEXT-advancedBlendMaxColorAttachments-07480";
        blend_feature_07470                      = "VUID-vkCmdDrawMultiIndexedEXT-pColorBlendEnables-07470";
        color_attachment_08963                   = "VUID-vkCmdDrawMultiIndexedEXT-pColorAttachments-08963";
        depth_attachment_08964                   = "VUID-vkCmdDrawMultiIndexedEXT-pDepthAttachment-08964";
        stencil_attachment_08965                 = "VUID-vkCmdDrawMultiIndexedEXT-pStencilAttachment-08965";
        sample_locations_07482                   = "VUID-vkCmdDrawMultiIndexedEXT-sampleLocationsPerPixel-07482";
    }
};

struct DispatchVuidsCmdDrawIndirect : DrawDispatchVuid {
    DispatchVuidsCmdDrawIndirect() : DrawDispatchVuid() {
        pipeline_bound_08606                     = "VUID-vkCmdDrawIndirect-None-08606";
        pipeline_or_shaders_bound_08607          = "VUID-vkCmdDrawIndirect-None-08607";
        vertex_binding_04007                     = "VUID-vkCmdDrawIndirect-None-04007";
        vertex_binding_null_04008                = "VUID-vkCmdDrawIndirect-None-04008";
        compatible_pipeline_08600                = "VUID-vkCmdDrawIndirect-None-08600";
        render_pass_compatible_02684             = "VUID-vkCmdDrawIndirect-renderPass-02684";
        render_pass_began_08876                  = "VUID-vkCmdDrawIndirect-None-08876";
        subpass_index_02685                      = "VUID-vkCmdDrawIndirect-subpass-02685";
        sample_location_02689                    = "VUID-vkCmdDrawIndirect-sampleLocationsEnable-02689";
        linear_filter_sampler_04553              = "VUID-vkCmdDrawIndirect-magFilter-04553";
        linear_mipmap_sampler_04770              = "VUID-vkCmdDrawIndirect-mipmapMode-04770";
        cubic_sampler_02692                      = "VUID-vkCmdDrawIndirect-None-02692";
        indirect_protected_cb_02646              = "VUID-vkCmdDrawIndirect-commandBuffer-02711";
        indirect_contiguous_memory_02708         = "VUID-vkCmdDrawIndirect-buffer-02708";
        indirect_buffer_bit_02290                = "VUID-vkCmdDrawIndirect-buffer-02709";
        viewport_count_03417                     = "VUID-vkCmdDrawIndirect-viewportCount-03417";
        scissor_count_03418                      = "VUID-vkCmdDrawIndirect-scissorCount-03418";
        viewport_scissor_count_03419             = "VUID-vkCmdDrawIndirect-viewportCount-03419";
        primitive_topology_class_07500           = "VUID-vkCmdDrawIndirect-dynamicPrimitiveTopologyUnrestricted-07500";
        corner_sampled_address_mode_02696        = "VUID-vkCmdDrawIndirect-flags-02696";
        imageview_atomic_02691                   = "VUID-vkCmdDrawIndirect-None-02691";
        bufferview_atomic_07888                  = "VUID-vkCmdDrawIndirect-None-07888";
        push_constants_set_08602                 = "VUID-vkCmdDrawIndirect-maintenance4-08602";
        image_subresources_render_pass_write_06537 = "VUID-vkCmdDrawIndirect-None-06537";
        image_subresources_subpass_read_09003    = "VUID-vkCmdDrawIndirect-None-09003";
        image_subresources_subpass_write_06539   = "VUID-vkCmdDrawIndirect-None-06539";
        sampler_imageview_type_08609             = "VUID-vkCmdDrawIndirect-None-08609";
        sampler_implicitLod_dref_proj_08610      = "VUID-vkCmdDrawIndirect-None-08610";
        sampler_bias_offset_08611                = "VUID-vkCmdDrawIndirect-None-08611";
        vertex_binding_attribute_02721           = "VUID-vkCmdDrawIndirect-None-02721";
        dynamic_state_setting_commands_08608     = "VUID-vkCmdDrawIndirect-None-08608";
        msrtss_rasterization_samples_07284       = "VUID-vkCmdDrawIndirect-multisampledRenderToSingleSampled-07284";
        unprotected_command_buffer_02707         = "VUID-vkCmdDrawIndirect-commandBuffer-02707";
        max_multiview_instance_index_02688       = "VUID-vkCmdDrawIndirect-maxMultiviewInstanceIndex-02688";
        img_filter_cubic_02693                   = "VUID-vkCmdDrawIndirect-None-02693";
        filter_cubic_02694                       = "VUID-vkCmdDrawIndirect-filterCubic-02694";
        filter_cubic_min_max_02695               = "VUID-vkCmdDrawIndirect-filterCubicMinmax-02695";
        viewport_count_primitive_shading_rate_04552 = "VUID-vkCmdDrawIndirect-primitiveFragmentShadingRateWithMultipleViewports-04552";
        patch_control_points_04875               = "VUID-vkCmdDrawIndirect-None-04875";
        rasterizer_discard_enable_04876          = "VUID-vkCmdDrawIndirect-None-04876";
        depth_bias_enable_04877                  = "VUID-vkCmdDrawIndirect-None-04877";
        logic_op_04878                           = "VUID-vkCmdDrawIndirect-logicOp-04878";
        primitive_restart_enable_04879           = "VUID-vkCmdDrawIndirect-None-04879";
        vertex_input_04912                       = "VUID-vkCmdDrawIndirect-None-04912";
        vertex_input_binding_stride_04913        = "VUID-vkCmdDrawIndirect-pStrides-04913";
        vertex_input_04914                       = "VUID-vkCmdDrawIndirect-None-04914";
        blend_enable_04727                       = "VUID-vkCmdDrawIndirect-blendEnable-04727";
        dynamic_discard_rectangle_07751          = "VUID-vkCmdDrawIndirect-None-07751";
        dynamic_discard_rectangle_enable_07880   = "VUID-vkCmdDrawIndirect-None-07880";
        dynamic_discard_rectangle_mode_07881     =  "VUID-vkCmdDrawIndirect-None-07881";
        dynamic_exclusive_scissor_enable_07878   = "VUID-vkCmdDrawIndirect-None-07878";
        dynamic_exclusive_scissor_07879          = "VUID-vkCmdDrawIndirect-None-07879";
        dynamic_color_write_enable_07749         = "VUID-vkCmdDrawIndirect-None-07749";
        dynamic_color_write_enable_count_07750   = "VUID-vkCmdDrawIndirect-attachmentCount-07750";
        dynamic_attachment_feedback_loop_08877   = "VUID-vkCmdDrawIndirect-None-08877";
        dynamic_rendering_view_mask_06178        = "VUID-vkCmdDrawIndirect-viewMask-06178";
        dynamic_rendering_color_count_06179      = "VUID-vkCmdDrawIndirect-colorAttachmentCount-06179";
        dynamic_rendering_color_formats_08910    = "VUID-vkCmdDrawIndirect-dynamicRenderingUnusedAttachments-08910";
        dynamic_rendering_unused_attachments_08911 = "VUID-vkCmdDrawIndirect-dynamicRenderingUnusedAttachments-08911";
        dynamic_rendering_undefined_color_formats_08912 = "VUID-vkCmdDrawIndirect-dynamicRenderingUnusedAttachments-08912";
        dynamic_rendering_undefined_depth_format_08916 = "VUID-vkCmdDrawIndirect-dynamicRenderingUnusedAttachments-08916";
        dynamic_rendering_undefined_stencil_format_08916 = "VUID-vkCmdDrawIndirect-dynamicRenderingUnusedAttachments-08916";
        dynamic_rendering_depth_format_08914     = "VUID-vkCmdDrawIndirect-dynamicRenderingUnusedAttachments-08914";
        dynamic_rendering_unused_attachments_08915 = "VUID-vkCmdDrawIndirect-dynamicRenderingUnusedAttachments-08915";
        dynamic_rendering_stencil_format_08917   = "VUID-vkCmdDrawIndirect-dynamicRenderingUnusedAttachments-08917";
        dynamic_rendering_unused_attachments_08918 = "VUID-vkCmdDrawIndirect-dynamicRenderingUnusedAttachments-08918";
        dynamic_rendering_fsr_06183              = "VUID-vkCmdDrawIndirect-imageView-06183";
        dynamic_rendering_fdm_06184              = "VUID-vkCmdDrawIndirect-imageView-06184";
        dynamic_rendering_color_sample_06185     = "VUID-vkCmdDrawIndirect-colorAttachmentCount-06185";
        dynamic_rendering_depth_sample_06186     = "VUID-vkCmdDrawIndirect-pDepthAttachment-06186";
        dynamic_rendering_stencil_sample_06187   = "VUID-vkCmdDrawIndirect-pStencilAttachment-06187";
        dynamic_rendering_06198                  = "VUID-vkCmdDrawIndirect-renderPass-06198";
        dynamic_rendering_07285                  = "VUID-vkCmdDrawIndirect-multisampledRenderToSingleSampled-07285";
        dynamic_rendering_07286                  = "VUID-vkCmdDrawIndirect-multisampledRenderToSingleSampled-07286";
        dynamic_rendering_07287                  = "VUID-vkCmdDrawIndirect-multisampledRenderToSingleSampled-07287";
        image_view_access_64_04470               = "VUID-vkCmdDrawIndirect-SampledType-04470";
        image_view_access_32_04471               = "VUID-vkCmdDrawIndirect-SampledType-04471";
        image_view_sparse_64_04474               = "VUID-vkCmdDrawIndirect-sparseImageInt64Atomics-04474";
        buffer_view_access_64_04472              = "VUID-vkCmdDrawIndirect-SampledType-04472";
        buffer_view_access_32_04473              = "VUID-vkCmdDrawIndirect-SampledType-04473";
        storage_image_read_without_format_07028  = "VUID-vkCmdDrawIndirect-OpTypeImage-07028";
        storage_image_write_without_format_07027 = "VUID-vkCmdDrawIndirect-OpTypeImage-07027";
        storage_texel_buffer_read_without_format_07030  = "VUID-vkCmdDrawIndirect-OpTypeImage-07030";
        storage_texel_buffer_write_without_format_07029 = "VUID-vkCmdDrawIndirect-OpTypeImage-07029";
        storage_image_write_texel_count_08795           = "VUID-vkCmdDrawIndirect-OpImageWrite-08795";
        storage_image_write_texel_count_08796           = "VUID-vkCmdDrawIndirect-OpImageWrite-08796";
        storage_texel_buffer_write_texel_count_04469    = "VUID-vkCmdDrawIndirect-OpImageWrite-04469";
        depth_compare_sample_06479               = "VUID-vkCmdDrawIndirect-None-06479";
        depth_read_only_06886                    = "VUID-vkCmdDrawIndirect-None-06886";
        stencil_read_only_06887                  = "VUID-vkCmdDrawIndirect-None-06887";
        dynamic_sample_locations_06666           = "VUID-vkCmdDrawIndirect-None-06666";
        dynamic_tessellation_domain_origin_07619 = "VUID-vkCmdDrawIndirect-None-07619";
        dynamic_depth_clamp_enable_07620         = "VUID-vkCmdDrawIndirect-None-07620";
        dynamic_polygon_mode_07621               = "VUID-vkCmdDrawIndirect-None-07621";
        dynamic_rasterization_samples_07622      = "VUID-vkCmdDrawIndirect-None-07622";
        dynamic_sample_mask_07623                = "VUID-vkCmdDrawIndirect-None-07623";
        dynamic_alpha_to_coverage_enable_07624   = "VUID-vkCmdDrawIndirect-None-07624";
        dynamic_alpha_to_coverage_component_08919 = "VUID-vkCmdDrawIndirect-alphaToCoverageEnable-08919";
        dynamic_alpha_to_one_enable_07625        = "VUID-vkCmdDrawIndirect-None-07625";
        dynamic_logic_op_enable_07626            = "VUID-vkCmdDrawIndirect-None-07626";
        dynamic_color_blend_enable_07476         = "VUID-vkCmdDrawIndirect-firstAttachment-07476";
        dynamic_color_blend_equation_07477       = "VUID-vkCmdDrawIndirect-firstAttachment-07477";
        dynamic_color_write_mask_07478           = "VUID-vkCmdDrawIndirect-firstAttachment-07478";
        dynamic_rasterization_stream_07630       = "VUID-vkCmdDrawIndirect-None-07630";
        dynamic_conservative_rasterization_mode_07631 = "VUID-vkCmdDrawIndirect-None-07631";
        dynamic_extra_primitive_overestimation_size_07632 = "VUID-vkCmdDrawIndirect-None-07632";
        dynamic_depth_clip_enable_07633          = "VUID-vkCmdDrawIndirect-None-07633";
        dynamic_sample_locations_enable_07634    = "VUID-vkCmdDrawIndirect-None-07634";
        dynamic_color_blend_advanced_07479       = "VUID-vkCmdDrawIndirect-firstAttachment-07479";
        dynamic_provoking_vertex_mode_07636      = "VUID-vkCmdDrawIndirect-None-07636";
        dynamic_line_rasterization_mode_07637    = "VUID-vkCmdDrawIndirect-None-07637";
        dynamic_line_stipple_enable_07638        = "VUID-vkCmdDrawIndirect-None-07638";
        dynamic_depth_clip_negative_one_to_one_07639 = "VUID-vkCmdDrawIndirect-None-07639";
        dynamic_viewport_w_scaling_enable_07640  = "VUID-vkCmdDrawIndirect-None-07640";
        dynamic_viewport_swizzle_07641           = "VUID-vkCmdDrawIndirect-None-07641";
        dynamic_coverage_to_color_enable_07642   = "VUID-vkCmdDrawIndirect-None-07642";
        dynamic_coverage_to_color_location_07643 = "VUID-vkCmdDrawIndirect-None-07643";
        dynamic_coverage_modulation_mode_07644   = "VUID-vkCmdDrawIndirect-None-07644";
        dynamic_coverage_modulation_table_enable_07645 = "VUID-vkCmdDrawIndirect-None-07645";
        dynamic_coverage_modulation_table_07646  = "VUID-vkCmdDrawIndirect-None-07646";
        dynamic_shading_rate_image_enable_07647  = "VUID-vkCmdDrawIndirect-None-07647";
        dynamic_representative_fragment_test_enable_07648  = "VUID-vkCmdDrawIndirect-None-07648";
        dynamic_coverage_reduction_mode_07649    = "VUID-vkCmdDrawIndirect-None-07649";
        dynamic_viewport_07831                   = "VUID-vkCmdDrawIndirect-None-07831";
        dynamic_scissor_07832                    = "VUID-vkCmdDrawIndirect-None-07832";
        dynamic_depth_bias_07834                 = "VUID-vkCmdDrawIndirect-None-07834";
        dynamic_line_width_07833                 = "VUID-vkCmdDrawIndirect-None-07833";
        dynamic_line_stipple_ext_07849           = "VUID-vkCmdDrawIndirect-None-07849";
        dynamic_blend_constants_07835            = "VUID-vkCmdDrawIndirect-None-07835";
        dynamic_depth_bounds_07836               = "VUID-vkCmdDrawIndirect-None-07836";
        dynamic_depth_enable_08715               = "VUID-vkCmdDrawIndirect-pDynamicStates-08715";
        dynamic_stencil_compare_mask_07837       = "VUID-vkCmdDrawIndirect-None-07837";
        dynamic_stencil_write_mask_07838         = "VUID-vkCmdDrawIndirect-None-07838";
        dynamic_stencil_write_mask_08716         = "VUID-vkCmdDrawIndirect-pDynamicStates-08716";
        dynamic_stencil_reference_07839          = "VUID-vkCmdDrawIndirect-None-07839";
        dynamic_state_inherited_07850            = "VUID-vkCmdDrawIndirect-None-07850";
        dynamic_cull_mode_07840                  = "VUID-vkCmdDrawIndirect-None-07840";
        dynamic_front_face_07841                 = "VUID-vkCmdDrawIndirect-None-07841";
        dynamic_primitive_topology_07842         = "VUID-vkCmdDrawIndirect-None-07842";
        dynamic_depth_test_enable_07843          = "VUID-vkCmdDrawIndirect-None-07843";
        dynamic_depth_write_enable_07844         = "VUID-vkCmdDrawIndirect-None-07844";
        dynamic_depth_compare_op_07845           = "VUID-vkCmdDrawIndirect-None-07845";
        dynamic_depth_bound_test_enable_07846    = "VUID-vkCmdDrawIndirect-None-07846";
        dynamic_stencil_test_enable_07847        = "VUID-vkCmdDrawIndirect-None-07847";
        dynamic_stencil_op_07848                 = "VUID-vkCmdDrawIndirect-None-07848";
        primitives_generated_06708               = "VUID-vkCmdDrawIndirect-primitivesGeneratedQueryWithRasterizerDiscard-06708";
        primitives_generated_streams_06709       = "VUID-vkCmdDrawIndirect-primitivesGeneratedQueryWithNonZeroStreams-06709";
        invalid_mesh_shader_stages_06481         = "VUID-vkCmdDrawIndirect-stage-06481";
        descriptor_buffer_bit_set_08114          = "VUID-vkCmdDrawIndirect-None-08114";
        descriptor_buffer_bit_not_set_08115      = "VUID-vkCmdDrawIndirect-None-08115";
        descriptor_buffer_set_offset_missing_08117 = "VUID-vkCmdDrawIndirect-None-08117";
        image_view_dim_07752                     = "VUID-vkCmdDrawIndirect-viewType-07752";
        image_view_numeric_format_07753          = "VUID-vkCmdDrawIndirect-format-07753";
        stippled_rectangular_lines_07495         = "VUID-vkCmdDrawIndirect-stippledLineEnable-07495";
        stippled_bresenham_lines_07496           = "VUID-vkCmdDrawIndirect-stippledLineEnable-07496";
        stippled_smooth_lines_07497              = "VUID-vkCmdDrawIndirect-stippledLineEnable-07497";
        stippled_default_strict_07498            = "VUID-vkCmdDrawIndirect-stippledLineEnable-07498";
        viewport_and_scissor_with_count_08635    = "VUID-vkCmdDrawIndirect-None-08635";
        viewport_w_scaling_08636                 = "VUID-vkCmdDrawIndirect-None-08636";
        shading_rate_palette_08637               = "VUID-vkCmdDrawIndirect-None-08637";
        exclusive_scissor_08638                  = "VUID-vkCmdDrawIndirect-None-08638";
        external_format_resolve_09362            = "VUID-vkCmdDrawIndirect-colorAttachmentCount-09362";
        external_format_resolve_09363            = "VUID-vkCmdDrawIndirect-None-09363";
        external_format_resolve_09364            = "VUID-vkCmdDrawIndirect-None-09364";
        external_format_resolve_09365            = "VUID-vkCmdDrawIndirect-None-09365";
        external_format_resolve_09368            = "VUID-vkCmdDrawIndirect-None-09368";
        external_format_resolve_09369            = "VUID-vkCmdDrawIndirect-None-09369";
        external_format_resolve_09372            = "VUID-vkCmdDrawIndirect-colorAttachmentCount-09372";
        set_rasterizer_discard_enable_08639      = "VUID-vkCmdDrawIndirect-None-08639";
        set_depth_bias_enable_08640              = "VUID-vkCmdDrawIndirect-None-08640";
        set_logic_op_08641                       = "VUID-vkCmdDrawIndirect-None-08641";
        set_color_blend_enable_08643             = "VUID-vkCmdDrawIndirect-None-08643";
        set_rasterization_samples_08644              = "VUID-vkCmdDrawIndirect-None-08644";
        set_color_write_enable_08646             = "VUID-vkCmdDrawIndirect-None-08646";
        set_color_write_enable_08647             = "VUID-vkCmdDrawIndirect-None-08647";
        set_discard_rectangles_enable_08648      = "VUID-vkCmdDrawIndirect-None-08648";
        set_discard_rectangles_mode_08649      = "VUID-vkCmdDrawIndirect-None-08649";
        set_depth_clamp_enable_08650             = "VUID-vkCmdDrawIndirect-None-08650";
        set_polygon_mode_08651                   = "VUID-vkCmdDrawIndirect-None-08651";
        set_rasterization_samples_08652          = "VUID-vkCmdDrawIndirect-None-08652";
        set_sample_mask_08653                    = "VUID-vkCmdDrawIndirect-None-08653";
        set_alpha_to_coverage_enable_08654       = "VUID-vkCmdDrawIndirect-None-08654";
        set_alpha_to_one_enable_08655            = "VUID-vkCmdDrawIndirect-None-08655";
        set_logic_op_enable_08656                = "VUID-vkCmdDrawIndirect-None-08656";
        set_color_blend_enable_08657             = "VUID-vkCmdDrawIndirect-None-08657";
        set_color_blend_equation_08658           = "VUID-vkCmdDrawIndirect-None-08658";
        set_color_write_mask_08659               = "VUID-vkCmdDrawIndirect-None-08659";
        set_rasterization_streams_08660          = "VUID-vkCmdDrawIndirect-None-08660";
        set_conservative_rasterization_mode_08661 = "VUID-vkCmdDrawIndirect-None-08661";
        set_extra_primitive_overestimation_size_08662 = "VUID-vkCmdDrawIndirect-None-08662";
        set_depth_clip_enable_08663              = "VUID-vkCmdDrawIndirect-None-08663";
        set_sample_locations_enable_08664        = "VUID-vkCmdDrawIndirect-None-08664";
        set_provoking_vertex_mode_08665          = "VUID-vkCmdDrawIndirect-None-08665";
        set_line_rasterization_mode_08666        = "VUID-vkCmdDrawIndirect-None-08666";
        set_line_rasterization_mode_08667        = "VUID-vkCmdDrawIndirect-None-08667";
        set_line_rasterization_mode_08668        = "VUID-vkCmdDrawIndirect-None-08668";
        set_line_stipple_enable_08669            = "VUID-vkCmdDrawIndirect-None-08669";
        set_line_stipple_enable_08670            = "VUID-vkCmdDrawIndirect-None-08670";
        set_line_stipple_enable_08671            = "VUID-vkCmdDrawIndirect-None-08671";
        set_line_stipple_08672                   = "VUID-vkCmdDrawIndirect-None-08672";
        set_depth_clip_negative_one_to_one_08673 = "VUID-vkCmdDrawIndirect-None-08673";
        set_viewport_w_scaling_enable_08674      = "VUID-vkCmdDrawIndirect-None-08674";
        set_viewport_swizzle_08675               = "VUID-vkCmdDrawIndirect-None-08675";
        set_coverage_to_color_enable_08676       = "VUID-vkCmdDrawIndirect-None-08676";
        set_coverage_to_color_location_08677     = "VUID-vkCmdDrawIndirect-None-08677";
        set_coverage_modulation_mode_08678       = "VUID-vkCmdDrawIndirect-None-08678";
        set_coverage_modulation_table_enable_08679 = "VUID-vkCmdDrawIndirect-None-08679";
        set_coverage_modulation_table_08680      = "VUID-vkCmdDrawIndirect-None-08680";
        set_shading_rate_image_enable_08681      = "VUID-vkCmdDrawIndirect-None-08681";
        set_representative_fragment_test_enable_08682 = "VUID-vkCmdDrawIndirect-None-08682";
        set_coverage_reduction_mode_08683        = "VUID-vkCmdDrawIndirect-None-08683";
        vertex_shader_08684                      = "VUID-vkCmdDrawIndirect-None-08684";
        tessellation_control_shader_08685        = "VUID-vkCmdDrawIndirect-None-08685";
        tessellation_evaluation_shader_08686     = "VUID-vkCmdDrawIndirect-None-08686";
        geometry_shader_08687                    = "VUID-vkCmdDrawIndirect-None-08687";
        fragment_shader_08688                    = "VUID-vkCmdDrawIndirect-None-08688";
        task_shader_08689                        = "VUID-vkCmdDrawIndirect-None-08689";
        mesh_shader_08690                        = "VUID-vkCmdDrawIndirect-None-08690";
        vert_mesh_shader_08693                   = "VUID-vkCmdDrawIndirect-None-08693";
        task_mesh_shader_08694                   = "VUID-vkCmdDrawIndirect-None-08694";
        task_mesh_shader_08695                   = "VUID-vkCmdDrawIndirect-None-08695";
        vert_task_mesh_shader_08696              = "VUID-vkCmdDrawIndirect-None-08696";
        linked_shaders_08698                     = "VUID-vkCmdDrawIndirect-None-08698";
        linked_shaders_08699                     = "VUID-vkCmdDrawIndirect-None-08699";
        shaders_push_constants_08878             = "VUID-vkCmdDrawIndirect-None-08878";
        shaders_descriptor_layouts_08879         = "VUID-vkCmdDrawIndirect-None-08879";
        set_attachment_feedback_loop_enable_08880 = "VUID-vkCmdDrawIndirect-None-08880";
        set_vertex_input_08882                   = "VUID-vkCmdDrawIndirect-None-08882";
        draw_shaders_no_task_mesh_08885          = "VUID-vkCmdDrawIndirect-None-08885";
        set_line_width_08617                     = "VUID-vkCmdDrawIndirect-None-08617";
        set_line_width_08618                     = "VUID-vkCmdDrawIndirect-None-08618";
        set_depth_bias_08620                     = "VUID-vkCmdDrawIndirect-None-08620";
        set_blend_constants_08621                = "VUID-vkCmdDrawIndirect-None-08621";
        set_depth_bounds_08622                   = "VUID-vkCmdDrawIndirect-None-08622";
        set_stencil_compare_mask_08623           = "VUID-vkCmdDrawIndirect-None-08623";
        set_stencil_write_mask_08624             = "VUID-vkCmdDrawIndirect-None-08624";
        set_stencil_reference_08625              = "VUID-vkCmdDrawIndirect-None-08625";
        set_sample_locations_08626               = "VUID-vkCmdDrawIndirect-None-08626";
        set_cull_mode_08627                      = "VUID-vkCmdDrawIndirect-None-08627";
        set_front_face_08628                     = "VUID-vkCmdDrawIndirect-None-08628";
        set_depth_test_enable_08629              = "VUID-vkCmdDrawIndirect-None-08629";
        set_depth_write_enable_08630             = "VUID-vkCmdDrawIndirect-None-08630";
        set_depth_comapre_op_08631               = "VUID-vkCmdDrawIndirect-None-08631";
        set_depth_bounds_test_enable_08632       = "VUID-vkCmdDrawIndirect-None-08632";
        set_stencil_test_enable_08633            = "VUID-vkCmdDrawIndirect-None-08633";
        set_stencil_op_08634                     = "VUID-vkCmdDrawIndirect-None-08634";
        set_line_width_08619                     = "VUID-vkCmdDrawIndirect-None-08619";
        set_viewport_with_count_08642            = "VUID-vkCmdDrawIndirect-primitiveFragmentShadingRateWithMultipleViewports-08642";
        alpha_component_word_08920               = "VUID-vkCmdDrawIndirect-alphaToCoverageEnable-08920";
        color_write_mask_09116                   = "VUID-vkCmdDrawIndirect-None-09116";
        vertex_input_format_08936                = "VUID-vkCmdDrawIndirect-format-08936";
        vertex_input_format_08937                = "VUID-vkCmdDrawIndirect-format-08937";
        vertex_input_format_09203                = "VUID-vkCmdDrawIndirect-None-09203";
        vertex_input_format_07939                = "VUID-vkCmdDrawIndirect-Input-07939";
        set_clip_space_w_scaling_09232           = "VUID-vkCmdDrawIndirect-None-09232";
        set_discard_rectangle_09236              = "VUID-vkCmdDrawIndirect-rasterizerDiscardEnable-09236";
        set_viewport_coarse_sample_order_09233   = "VUID-vkCmdDrawIndirect-shadingRateImage-09233";
        set_viewport_shading_rate_palette_09234  = "VUID-vkCmdDrawIndirect-shadingRateImage-09234";
        set_exclusive_scissor_enable_09235       = "VUID-vkCmdDrawIndirect-exclusiveScissor-09235";
        set_fragment_shading_rate_09238          = "VUID-vkCmdDrawIndirect-pipelineFragmentShadingRate-09238";
        set_tessellation_domain_origin_09237     = "VUID-vkCmdDrawIndirect-None-09237";
        rasterization_samples_07935              = "VUID-vkCmdDrawIndirect-pNext-07935";
        mesh_shader_queries_07073                = "VUID-vkCmdDrawIndirect-stage-07073";
        blend_advanced_07480                     = "VUID-vkCmdDrawIndirect-advancedBlendMaxColorAttachments-07480";
        blend_feature_07470                      = "VUID-vkCmdDrawIndirect-pColorBlendEnables-07470";
        color_attachment_08963                   = "VUID-vkCmdDrawIndirect-pColorAttachments-08963";
        depth_attachment_08964                   = "VUID-vkCmdDrawIndirect-pDepthAttachment-08964";
        stencil_attachment_08965                 = "VUID-vkCmdDrawIndirect-pStencilAttachment-08965";
        sample_locations_07482                   = "VUID-vkCmdDrawIndirect-sampleLocationsPerPixel-07482";
    }
};

struct DispatchVuidsCmdDrawIndexedIndirect : DrawDispatchVuid {
    DispatchVuidsCmdDrawIndexedIndirect() : DrawDispatchVuid() {
        pipeline_bound_08606                     = "VUID-vkCmdDrawIndexedIndirect-None-08606";
        pipeline_or_shaders_bound_08607          = "VUID-vkCmdDrawIndexedIndirect-None-08607";
        index_binding_07312                      = "VUID-vkCmdDrawIndexedIndirect-None-07312";
        vertex_binding_04007                     = "VUID-vkCmdDrawIndexedIndirect-None-04007";
        vertex_binding_null_04008                = "VUID-vkCmdDrawIndexedIndirect-None-04008";
        compatible_pipeline_08600                = "VUID-vkCmdDrawIndexedIndirect-None-08600";
        render_pass_compatible_02684             = "VUID-vkCmdDrawIndexedIndirect-renderPass-02684";
        render_pass_began_08876                  = "VUID-vkCmdDrawIndexedIndirect-None-08876";
        subpass_index_02685                      = "VUID-vkCmdDrawIndexedIndirect-subpass-02685";
        sample_location_02689                    = "VUID-vkCmdDrawIndexedIndirect-sampleLocationsEnable-02689";
        linear_filter_sampler_04553              = "VUID-vkCmdDrawIndexedIndirect-magFilter-04553";
        linear_mipmap_sampler_04770              = "VUID-vkCmdDrawIndexedIndirect-mipmapMode-04770";
        cubic_sampler_02692                      = "VUID-vkCmdDrawIndexedIndirect-None-02692";
        indirect_protected_cb_02646              = "VUID-vkCmdDrawIndexedIndirect-commandBuffer-02711";
        indirect_contiguous_memory_02708         = "VUID-vkCmdDrawIndexedIndirect-buffer-02708";
        indirect_buffer_bit_02290                = "VUID-vkCmdDrawIndexedIndirect-buffer-02709";
        viewport_count_03417                     = "VUID-vkCmdDrawIndexedIndirect-viewportCount-03417";
        scissor_count_03418                      = "VUID-vkCmdDrawIndexedIndirect-scissorCount-03418";
        viewport_scissor_count_03419             = "VUID-vkCmdDrawIndexedIndirect-viewportCount-03419";
        primitive_topology_class_07500           = "VUID-vkCmdDrawIndexedIndirect-dynamicPrimitiveTopologyUnrestricted-07500";
        corner_sampled_address_mode_02696        = "VUID-vkCmdDrawIndexedIndirect-flags-02696";
        imageview_atomic_02691                   = "VUID-vkCmdDrawIndexedIndirect-None-02691";
        bufferview_atomic_07888                  = "VUID-vkCmdDrawIndexedIndirect-None-07888";
        push_constants_set_08602                 = "VUID-vkCmdDrawIndexedIndirect-maintenance4-08602";
        image_subresources_render_pass_write_06537 = "VUID-vkCmdDrawIndexedIndirect-None-06537";
        image_subresources_subpass_read_09003    = "VUID-vkCmdDrawIndexedIndirect-None-09003";
        image_subresources_subpass_write_06539   = "VUID-vkCmdDrawIndexedIndirect-None-06539";
        sampler_imageview_type_08609             = "VUID-vkCmdDrawIndexedIndirect-None-08609";
        sampler_implicitLod_dref_proj_08610      = "VUID-vkCmdDrawIndexedIndirect-None-08610";
        sampler_bias_offset_08611                = "VUID-vkCmdDrawIndexedIndirect-None-08611";
        vertex_binding_attribute_02721           = "VUID-vkCmdDrawIndexedIndirect-None-02721";
        dynamic_state_setting_commands_08608     = "VUID-vkCmdDrawIndexedIndirect-None-08608";
        msrtss_rasterization_samples_07284       = "VUID-vkCmdDrawIndexedIndirect-multisampledRenderToSingleSampled-07284";
        unprotected_command_buffer_02707         = "VUID-vkCmdDrawIndexedIndirect-commandBuffer-02707";
        max_multiview_instance_index_02688       = "VUID-vkCmdDrawIndexedIndirect-maxMultiviewInstanceIndex-02688";
        img_filter_cubic_02693                   = "VUID-vkCmdDrawIndexedIndirect-None-02693";
        filter_cubic_02694                       = "VUID-vkCmdDrawIndexedIndirect-filterCubic-02694";
        filter_cubic_min_max_02695               = "VUID-vkCmdDrawIndexedIndirect-filterCubicMinmax-02695";
        viewport_count_primitive_shading_rate_04552 = "VUID-vkCmdDrawIndexedIndirect-primitiveFragmentShadingRateWithMultipleViewports-04552";
        patch_control_points_04875               = "VUID-vkCmdDrawIndexedIndirect-None-04875";
        rasterizer_discard_enable_04876          = "VUID-vkCmdDrawIndexedIndirect-None-04876";
        depth_bias_enable_04877                  = "VUID-vkCmdDrawIndexedIndirect-None-04877";
        logic_op_04878                           = "VUID-vkCmdDrawIndexedIndirect-logicOp-04878";
        primitive_restart_enable_04879           = "VUID-vkCmdDrawIndexedIndirect-None-04879";
        vertex_input_04912                       = "VUID-vkCmdDrawIndexedIndirect-None-04912";
        vertex_input_binding_stride_04913        = "VUID-vkCmdDrawIndexedIndirect-pStrides-04913";
        vertex_input_04914                       = "VUID-vkCmdDrawIndexedIndirect-None-04914";
        blend_enable_04727                       = "VUID-vkCmdDrawIndexedIndirect-blendEnable-04727";
        dynamic_discard_rectangle_07751          = "VUID-vkCmdDrawIndexedIndirect-None-07751";
        dynamic_discard_rectangle_enable_07880   = "VUID-vkCmdDrawIndexedIndirect-None-07880";
        dynamic_discard_rectangle_mode_07881     =  "VUID-vkCmdDrawIndexedIndirect-None-07881";
        dynamic_exclusive_scissor_enable_07878   = "VUID-vkCmdDrawIndexedIndirect-None-07878";
        dynamic_exclusive_scissor_07879          = "VUID-vkCmdDrawIndexedIndirect-None-07879";
        dynamic_color_write_enable_07749         = "VUID-vkCmdDrawIndexedIndirect-None-07749";
        dynamic_color_write_enable_count_07750   = "VUID-vkCmdDrawIndexedIndirect-attachmentCount-07750";
        dynamic_attachment_feedback_loop_08877   = "VUID-vkCmdDrawIndexedIndirect-None-08877";
        dynamic_rendering_view_mask_06178        = "VUID-vkCmdDrawIndexedIndirect-viewMask-06178";
        dynamic_rendering_color_count_06179      = "VUID-vkCmdDrawIndexedIndirect-colorAttachmentCount-06179";
        dynamic_rendering_color_formats_08910    = "VUID-vkCmdDrawIndexedIndirect-dynamicRenderingUnusedAttachments-08910";
        dynamic_rendering_unused_attachments_08911 = "VUID-vkCmdDrawIndexedIndirect-dynamicRenderingUnusedAttachments-08911";
        dynamic_rendering_undefined_color_formats_08912 = "VUID-vkCmdDrawIndexedIndirect-dynamicRenderingUnusedAttachments-08912";
        dynamic_rendering_undefined_depth_format_08916 = "VUID-vkCmdDrawIndexedIndirect-dynamicRenderingUnusedAttachments-08916";
        dynamic_rendering_undefined_stencil_format_08916 = "VUID-vkCmdDrawIndexedIndirect-dynamicRenderingUnusedAttachments-08916";
        dynamic_rendering_depth_format_08914     = "VUID-vkCmdDrawIndexedIndirect-dynamicRenderingUnusedAttachments-08914";
        dynamic_rendering_unused_attachments_08915 = "VUID-vkCmdDrawIndexedIndirect-dynamicRenderingUnusedAttachments-08915";
        dynamic_rendering_stencil_format_08917   = "VUID-vkCmdDrawIndexedIndirect-dynamicRenderingUnusedAttachments-08917";
        dynamic_rendering_unused_attachments_08918 = "VUID-vkCmdDrawIndexedIndirect-dynamicRenderingUnusedAttachments-08918";
        dynamic_rendering_fsr_06183              = "VUID-vkCmdDrawIndexedIndirect-imageView-06183";
        dynamic_rendering_fdm_06184              = "VUID-vkCmdDrawIndexedIndirect-imageView-06184";
        dynamic_rendering_color_sample_06185     = "VUID-vkCmdDrawIndexedIndirect-colorAttachmentCount-06185";
        dynamic_rendering_depth_sample_06186     = "VUID-vkCmdDrawIndexedIndirect-pDepthAttachment-06186";
        dynamic_rendering_stencil_sample_06187   = "VUID-vkCmdDrawIndexedIndirect-pStencilAttachment-06187";
        dynamic_rendering_06198                  = "VUID-vkCmdDrawIndexedIndirect-renderPass-06198";
        dynamic_rendering_07285                  = "VUID-vkCmdDrawIndexedIndirect-multisampledRenderToSingleSampled-07285";
        dynamic_rendering_07286                  = "VUID-vkCmdDrawIndexedIndirect-multisampledRenderToSingleSampled-07286";
        dynamic_rendering_07287                  = "VUID-vkCmdDrawIndexedIndirect-multisampledRenderToSingleSampled-07287";
        image_view_access_64_04470               = "VUID-vkCmdDrawIndexedIndirect-SampledType-04470";
        image_view_access_32_04471               = "VUID-vkCmdDrawIndexedIndirect-SampledType-04471";
        image_view_sparse_64_04474               = "VUID-vkCmdDrawIndexedIndirect-sparseImageInt64Atomics-04474";
        buffer_view_access_64_04472              = "VUID-vkCmdDrawIndexedIndirect-SampledType-04472";
        buffer_view_access_32_04473              = "VUID-vkCmdDrawIndexedIndirect-SampledType-04473";
        storage_image_read_without_format_07028  = "VUID-vkCmdDrawIndexedIndirect-OpTypeImage-07028";
        storage_image_write_without_format_07027 = "VUID-vkCmdDrawIndexedIndirect-OpTypeImage-07027";
        storage_texel_buffer_read_without_format_07030  = "VUID-vkCmdDrawIndexedIndirect-OpTypeImage-07030";
        storage_texel_buffer_write_without_format_07029 = "VUID-vkCmdDrawIndexedIndirect-OpTypeImage-07029";
        storage_image_write_texel_count_08795           = "VUID-vkCmdDrawIndexedIndirect-OpImageWrite-08795";
        storage_image_write_texel_count_08796           = "VUID-vkCmdDrawIndexedIndirect-OpImageWrite-08796";
        storage_texel_buffer_write_texel_count_04469    = "VUID-vkCmdDrawIndexedIndirect-OpImageWrite-04469";
        depth_compare_sample_06479               = "VUID-vkCmdDrawIndexedIndirect-None-06479";
        depth_read_only_06886                    = "VUID-vkCmdDrawIndexedIndirect-None-06886";
        stencil_read_only_06887                  = "VUID-vkCmdDrawIndexedIndirect-None-06887";
        dynamic_sample_locations_06666           = "VUID-vkCmdDrawIndexedIndirect-None-06666";
        dynamic_tessellation_domain_origin_07619 = "VUID-vkCmdDrawIndexedIndirect-None-07619";
        dynamic_depth_clamp_enable_07620         = "VUID-vkCmdDrawIndexedIndirect-None-07620";
        dynamic_polygon_mode_07621               = "VUID-vkCmdDrawIndexedIndirect-None-07621";
        dynamic_rasterization_samples_07622      = "VUID-vkCmdDrawIndexedIndirect-None-07622";
        dynamic_sample_mask_07623                = "VUID-vkCmdDrawIndexedIndirect-None-07623";
        dynamic_alpha_to_coverage_enable_07624   = "VUID-vkCmdDrawIndexedIndirect-None-07624";
        dynamic_alpha_to_coverage_component_08919 = "VUID-vkCmdDrawIndexedIndirect-alphaToCoverageEnable-08919";
        dynamic_alpha_to_one_enable_07625        = "VUID-vkCmdDrawIndexedIndirect-None-07625";
        dynamic_logic_op_enable_07626            = "VUID-vkCmdDrawIndexedIndirect-None-07626";
        dynamic_color_blend_enable_07476         = "VUID-vkCmdDrawIndexedIndirect-firstAttachment-07476";
        dynamic_color_blend_equation_07477       = "VUID-vkCmdDrawIndexedIndirect-firstAttachment-07477";
        dynamic_color_write_mask_07478           = "VUID-vkCmdDrawIndexedIndirect-firstAttachment-07478";
        dynamic_rasterization_stream_07630       = "VUID-vkCmdDrawIndexedIndirect-None-07630";
        dynamic_conservative_rasterization_mode_07631 = "VUID-vkCmdDrawIndexedIndirect-None-07631";
        dynamic_extra_primitive_overestimation_size_07632 = "VUID-vkCmdDrawIndexedIndirect-None-07632";
        dynamic_depth_clip_enable_07633          = "VUID-vkCmdDrawIndexedIndirect-None-07633";
        dynamic_sample_locations_enable_07634    = "VUID-vkCmdDrawIndexedIndirect-None-07634";
        dynamic_color_blend_advanced_07479       = "VUID-vkCmdDrawIndexedIndirect-firstAttachment-07479";
        dynamic_provoking_vertex_mode_07636      = "VUID-vkCmdDrawIndexedIndirect-None-07636";
        dynamic_line_rasterization_mode_07637    = "VUID-vkCmdDrawIndexedIndirect-None-07637";
        dynamic_line_stipple_enable_07638        = "VUID-vkCmdDrawIndexedIndirect-None-07638";
        dynamic_depth_clip_negative_one_to_one_07639 = "VUID-vkCmdDrawIndexedIndirect-None-07639";
        dynamic_viewport_w_scaling_enable_07640  = "VUID-vkCmdDrawIndexedIndirect-None-07640";
        dynamic_viewport_swizzle_07641           = "VUID-vkCmdDrawIndexedIndirect-None-07641";
        dynamic_coverage_to_color_enable_07642   = "VUID-vkCmdDrawIndexedIndirect-None-07642";
        dynamic_coverage_to_color_location_07643 = "VUID-vkCmdDrawIndexedIndirect-None-07643";
        dynamic_coverage_modulation_mode_07644   = "VUID-vkCmdDrawIndexedIndirect-None-07644";
        dynamic_coverage_modulation_table_enable_07645 = "VUID-vkCmdDrawIndexedIndirect-None-07645";
        dynamic_coverage_modulation_table_07646  = "VUID-vkCmdDrawIndexedIndirect-None-07646";
        dynamic_shading_rate_image_enable_07647  = "VUID-vkCmdDrawIndexedIndirect-None-07647";
        dynamic_representative_fragment_test_enable_07648  = "VUID-vkCmdDrawIndexedIndirect-None-07648";
        dynamic_coverage_reduction_mode_07649    = "VUID-vkCmdDrawIndexedIndirect-None-07649";
        dynamic_viewport_07831                   = "VUID-vkCmdDrawIndexedIndirect-None-07831";
        dynamic_scissor_07832                    = "VUID-vkCmdDrawIndexedIndirect-None-07832";
        dynamic_depth_bias_07834                 = "VUID-vkCmdDrawIndexedIndirect-None-07834";
        dynamic_line_width_07833                 = "VUID-vkCmdDrawIndexedIndirect-None-07833";
        dynamic_line_stipple_ext_07849           = "VUID-vkCmdDrawIndexedIndirect-None-07849";
        dynamic_blend_constants_07835            = "VUID-vkCmdDrawIndexedIndirect-None-07835";
        dynamic_depth_bounds_07836               = "VUID-vkCmdDrawIndexedIndirect-None-07836";
        dynamic_depth_enable_08715               = "VUID-vkCmdDrawIndexedIndirect-pDynamicStates-08715";
        dynamic_stencil_compare_mask_07837       = "VUID-vkCmdDrawIndexedIndirect-None-07837";
        dynamic_stencil_write_mask_07838         = "VUID-vkCmdDrawIndexedIndirect-None-07838";
        dynamic_stencil_write_mask_08716         = "VUID-vkCmdDrawIndexedIndirect-pDynamicStates-08716";
        dynamic_stencil_reference_07839          = "VUID-vkCmdDrawIndexedIndirect-None-07839";
        dynamic_state_inherited_07850            = "VUID-vkCmdDrawIndexedIndirect-None-07850";
        dynamic_cull_mode_07840                  = "VUID-vkCmdDrawIndexedIndirect-None-07840";
        dynamic_front_face_07841                 = "VUID-vkCmdDrawIndexedIndirect-None-07841";
        dynamic_primitive_topology_07842         = "VUID-vkCmdDrawIndexedIndirect-None-07842";
        dynamic_depth_test_enable_07843          = "VUID-vkCmdDrawIndexedIndirect-None-07843";
        dynamic_depth_write_enable_07844         = "VUID-vkCmdDrawIndexedIndirect-None-07844";
        dynamic_depth_compare_op_07845           = "VUID-vkCmdDrawIndexedIndirect-None-07845";
        dynamic_depth_bound_test_enable_07846    = "VUID-vkCmdDrawIndexedIndirect-None-07846";
        dynamic_stencil_test_enable_07847        = "VUID-vkCmdDrawIndexedIndirect-None-07847";
        dynamic_stencil_op_07848                 = "VUID-vkCmdDrawIndexedIndirect-None-07848";
        primitives_generated_06708               = "VUID-vkCmdDrawIndexedIndirect-primitivesGeneratedQueryWithRasterizerDiscard-06708";
        primitives_generated_streams_06709       = "VUID-vkCmdDrawIndexedIndirect-primitivesGeneratedQueryWithNonZeroStreams-06709";
        invalid_mesh_shader_stages_06481         = "VUID-vkCmdDrawIndexedIndirect-stage-06481";
        descriptor_buffer_bit_set_08114          = "VUID-vkCmdDrawIndexedIndirect-None-08114";
        descriptor_buffer_bit_not_set_08115      = "VUID-vkCmdDrawIndexedIndirect-None-08115";
        descriptor_buffer_set_offset_missing_08117 = "VUID-vkCmdDrawIndexedIndirect-None-08117";
        image_view_dim_07752                     = "VUID-vkCmdDrawIndexedIndirect-viewType-07752";
        image_view_numeric_format_07753          = "VUID-vkCmdDrawIndexedIndirect-format-07753";
        stippled_rectangular_lines_07495         = "VUID-vkCmdDrawIndexedIndirect-stippledLineEnable-07495";
        stippled_bresenham_lines_07496           = "VUID-vkCmdDrawIndexedIndirect-stippledLineEnable-07496";
        stippled_smooth_lines_07497              = "VUID-vkCmdDrawIndexedIndirect-stippledLineEnable-07497";
        stippled_default_strict_07498            = "VUID-vkCmdDrawIndexedIndirect-stippledLineEnable-07498";
        viewport_and_scissor_with_count_08635    = "VUID-vkCmdDrawIndexedIndirect-None-08635";
        viewport_w_scaling_08636                 = "VUID-vkCmdDrawIndexedIndirect-None-08636";
        shading_rate_palette_08637               = "VUID-vkCmdDrawIndexedIndirect-None-08637";
        exclusive_scissor_08638                  = "VUID-vkCmdDrawIndexedIndirect-None-08638";
        external_format_resolve_09362            = "VUID-vkCmdDrawIndexedIndirect-colorAttachmentCount-09362";
        external_format_resolve_09363            = "VUID-vkCmdDrawIndexedIndirect-None-09363";
        external_format_resolve_09364            = "VUID-vkCmdDrawIndexedIndirect-None-09364";
        external_format_resolve_09365            = "VUID-vkCmdDrawIndexedIndirect-None-09365";
        external_format_resolve_09368            = "VUID-vkCmdDrawIndexedIndirect-None-09368";
        external_format_resolve_09369            = "VUID-vkCmdDrawIndexedIndirect-None-09369";
        external_format_resolve_09372            = "VUID-vkCmdDrawIndexedIndirect-colorAttachmentCount-09372";
        set_rasterizer_discard_enable_08639      = "VUID-vkCmdDrawIndexedIndirect-None-08639";
        set_depth_bias_enable_08640              = "VUID-vkCmdDrawIndexedIndirect-None-08640";
        set_logic_op_08641                       = "VUID-vkCmdDrawIndexedIndirect-None-08641";
        set_color_blend_enable_08643             = "VUID-vkCmdDrawIndexedIndirect-None-08643";
        set_rasterization_samples_08644              = "VUID-vkCmdDrawIndexedIndirect-None-08644";
        set_color_write_enable_08646             = "VUID-vkCmdDrawIndexedIndirect-None-08646";
        set_color_write_enable_08647             = "VUID-vkCmdDrawIndexedIndirect-None-08647";
        set_discard_rectangles_enable_08648      = "VUID-vkCmdDrawIndexedIndirect-None-08648";
        set_discard_rectangles_mode_08649      = "VUID-vkCmdDrawIndexedIndirect-None-08649";
        set_depth_clamp_enable_08650             = "VUID-vkCmdDrawIndexedIndirect-None-08650";
        set_polygon_mode_08651                   = "VUID-vkCmdDrawIndexedIndirect-None-08651";
        set_rasterization_samples_08652          = "VUID-vkCmdDrawIndexedIndirect-None-08652";
        set_sample_mask_08653                    = "VUID-vkCmdDrawIndexedIndirect-None-08653";
        set_alpha_to_coverage_enable_08654       = "VUID-vkCmdDrawIndexedIndirect-None-08654";
        set_alpha_to_one_enable_08655            = "VUID-vkCmdDrawIndexedIndirect-None-08655";
        set_logic_op_enable_08656                = "VUID-vkCmdDrawIndexedIndirect-None-08656";
        set_color_blend_enable_08657             = "VUID-vkCmdDrawIndexedIndirect-None-08657";
        set_color_blend_equation_08658           = "VUID-vkCmdDrawIndexedIndirect-None-08658";
        set_color_write_mask_08659               = "VUID-vkCmdDrawIndexedIndirect-None-08659";
        set_rasterization_streams_08660          = "VUID-vkCmdDrawIndexedIndirect-None-08660";
        set_conservative_rasterization_mode_08661 = "VUID-vkCmdDrawIndexedIndirect-None-08661";
        set_extra_primitive_overestimation_size_08662 = "VUID-vkCmdDrawIndexedIndirect-None-08662";
        set_depth_clip_enable_08663              = "VUID-vkCmdDrawIndexedIndirect-None-08663";
        set_sample_locations_enable_08664        = "VUID-vkCmdDrawIndexedIndirect-None-08664";
        set_provoking_vertex_mode_08665          = "VUID-vkCmdDrawIndexedIndirect-None-08665";
        set_line_rasterization_mode_08666        = "VUID-vkCmdDrawIndexedIndirect-None-08666";
        set_line_rasterization_mode_08667        = "VUID-vkCmdDrawIndexedIndirect-None-08667";
        set_line_rasterization_mode_08668        = "VUID-vkCmdDrawIndexedIndirect-None-08668";
        set_line_stipple_enable_08669            = "VUID-vkCmdDrawIndexedIndirect-None-08669";
        set_line_stipple_enable_08670            = "VUID-vkCmdDrawIndexedIndirect-None-08670";
        set_line_stipple_enable_08671            = "VUID-vkCmdDrawIndexedIndirect-None-08671";
        set_line_stipple_08672                   = "VUID-vkCmdDrawIndexedIndirect-None-08672";
        set_depth_clip_negative_one_to_one_08673 = "VUID-vkCmdDrawIndexedIndirect-None-08673";
        set_viewport_w_scaling_enable_08674      = "VUID-vkCmdDrawIndexedIndirect-None-08674";
        set_viewport_swizzle_08675               = "VUID-vkCmdDrawIndexedIndirect-None-08675";
        set_coverage_to_color_enable_08676       = "VUID-vkCmdDrawIndexedIndirect-None-08676";
        set_coverage_to_color_location_08677     = "VUID-vkCmdDrawIndexedIndirect-None-08677";
        set_coverage_modulation_mode_08678       = "VUID-vkCmdDrawIndexedIndirect-None-08678";
        set_coverage_modulation_table_enable_08679 = "VUID-vkCmdDrawIndexedIndirect-None-08679";
        set_coverage_modulation_table_08680      = "VUID-vkCmdDrawIndexedIndirect-None-08680";
        set_shading_rate_image_enable_08681      = "VUID-vkCmdDrawIndexedIndirect-None-08681";
        set_representative_fragment_test_enable_08682 = "VUID-vkCmdDrawIndexedIndirect-None-08682";
        set_coverage_reduction_mode_08683        = "VUID-vkCmdDrawIndexedIndirect-None-08683";
        vertex_shader_08684                      = "VUID-vkCmdDrawIndexedIndirect-None-08684";
        tessellation_control_shader_08685        = "VUID-vkCmdDrawIndexedIndirect-None-08685";
        tessellation_evaluation_shader_08686     = "VUID-vkCmdDrawIndexedIndirect-None-08686";
        geometry_shader_08687                    = "VUID-vkCmdDrawIndexedIndirect-None-08687";
        fragment_shader_08688                    = "VUID-vkCmdDrawIndexedIndirect-None-08688";
        task_shader_08689                        = "VUID-vkCmdDrawIndexedIndirect-None-08689";
        mesh_shader_08690                        = "VUID-vkCmdDrawIndexedIndirect-None-08690";
        vert_mesh_shader_08693                   = "VUID-vkCmdDrawIndexedIndirect-None-08693";
        task_mesh_shader_08694                   = "VUID-vkCmdDrawIndexedIndirect-None-08694";
        task_mesh_shader_08695                   = "VUID-vkCmdDrawIndexedIndirect-None-08695";
        vert_task_mesh_shader_08696              = "VUID-vkCmdDrawIndexedIndirect-None-08696";
        linked_shaders_08698                     = "VUID-vkCmdDrawIndexedIndirect-None-08698";
        linked_shaders_08699                     = "VUID-vkCmdDrawIndexedIndirect-None-08699";
        shaders_push_constants_08878             = "VUID-vkCmdDrawIndexedIndirect-None-08878";
        shaders_descriptor_layouts_08879         = "VUID-vkCmdDrawIndexedIndirect-None-08879";
        set_attachment_feedback_loop_enable_08880 = "VUID-vkCmdDrawIndexedIndirect-None-08880";
        set_vertex_input_08882                   = "VUID-vkCmdDrawIndexedIndirect-None-08882";
        draw_shaders_no_task_mesh_08885          = "VUID-vkCmdDrawIndexedIndirect-None-08885";
        set_line_width_08617                     = "VUID-vkCmdDrawIndexedIndirect-None-08617";
        set_line_width_08618                     = "VUID-vkCmdDrawIndexedIndirect-None-08618";
        set_depth_bias_08620                     = "VUID-vkCmdDrawIndexedIndirect-None-08620";
        set_blend_constants_08621                = "VUID-vkCmdDrawIndexedIndirect-None-08621";
        set_depth_bounds_08622                   = "VUID-vkCmdDrawIndexedIndirect-None-08622";
        set_stencil_compare_mask_08623           = "VUID-vkCmdDrawIndexedIndirect-None-08623";
        set_stencil_write_mask_08624             = "VUID-vkCmdDrawIndexedIndirect-None-08624";
        set_stencil_reference_08625              = "VUID-vkCmdDrawIndexedIndirect-None-08625";
        set_sample_locations_08626               = "VUID-vkCmdDrawIndexedIndirect-None-08626";
        set_cull_mode_08627                      = "VUID-vkCmdDrawIndexedIndirect-None-08627";
        set_front_face_08628                     = "VUID-vkCmdDrawIndexedIndirect-None-08628";
        set_depth_test_enable_08629              = "VUID-vkCmdDrawIndexedIndirect-None-08629";
        set_depth_write_enable_08630             = "VUID-vkCmdDrawIndexedIndirect-None-08630";
        set_depth_comapre_op_08631               = "VUID-vkCmdDrawIndexedIndirect-None-08631";
        set_depth_bounds_test_enable_08632       = "VUID-vkCmdDrawIndexedIndirect-None-08632";
        set_stencil_test_enable_08633            = "VUID-vkCmdDrawIndexedIndirect-None-08633";
        set_stencil_op_08634                     = "VUID-vkCmdDrawIndexedIndirect-None-08634";
        set_line_width_08619                     = "VUID-vkCmdDrawIndexedIndirect-None-08619";
        set_viewport_with_count_08642            = "VUID-vkCmdDrawIndexedIndirect-primitiveFragmentShadingRateWithMultipleViewports-08642";
        alpha_component_word_08920               = "VUID-vkCmdDrawIndexedIndirect-alphaToCoverageEnable-08920";
        color_write_mask_09116                   = "VUID-vkCmdDrawIndexedIndirect-None-09116";
        vertex_input_format_08936                = "VUID-vkCmdDrawIndexedIndirect-format-08936";
        vertex_input_format_08937                = "VUID-vkCmdDrawIndexedIndirect-format-08937";
        vertex_input_format_09203                = "VUID-vkCmdDrawIndexedIndirect-None-09203";
        vertex_input_format_07939                = "VUID-vkCmdDrawIndexedIndirect-Input-07939";
        set_clip_space_w_scaling_09232           = "VUID-vkCmdDrawIndexedIndirect-None-09232";
        set_discard_rectangle_09236              = "VUID-vkCmdDrawIndexedIndirect-rasterizerDiscardEnable-09236";
        set_viewport_coarse_sample_order_09233   = "VUID-vkCmdDrawIndexedIndirect-shadingRateImage-09233";
        set_viewport_shading_rate_palette_09234  = "VUID-vkCmdDrawIndexedIndirect-shadingRateImage-09234";
        set_exclusive_scissor_enable_09235       = "VUID-vkCmdDrawIndexedIndirect-exclusiveScissor-09235";
        set_fragment_shading_rate_09238          = "VUID-vkCmdDrawIndexedIndirect-pipelineFragmentShadingRate-09238";
        set_tessellation_domain_origin_09237     = "VUID-vkCmdDrawIndexedIndirect-None-09237";
        rasterization_samples_07935              = "VUID-vkCmdDrawIndexedIndirect-pNext-07935";
        mesh_shader_queries_07073                = "VUID-vkCmdDrawIndexedIndirect-stage-07073";
        blend_advanced_07480                     = "VUID-vkCmdDrawIndexedIndirect-advancedBlendMaxColorAttachments-07480";
        blend_feature_07470                      = "VUID-vkCmdDrawIndexedIndirect-pColorBlendEnables-07470";
        color_attachment_08963                   = "VUID-vkCmdDrawIndexedIndirect-pColorAttachments-08963";
        depth_attachment_08964                   = "VUID-vkCmdDrawIndexedIndirect-pDepthAttachment-08964";
        stencil_attachment_08965                 = "VUID-vkCmdDrawIndexedIndirect-pStencilAttachment-08965";
        sample_locations_07482                   = "VUID-vkCmdDrawIndexedIndirect-sampleLocationsPerPixel-07482";
    }
};

struct DispatchVuidsCmdDispatch : DrawDispatchVuid {
    DispatchVuidsCmdDispatch() : DrawDispatchVuid() {
        pipeline_bound_08606                     = "VUID-vkCmdDispatch-None-08606";
        pipeline_or_shaders_bound_08607          = "VUID-vkCmdDispatch-None-08607";
        compatible_pipeline_08600                = "VUID-vkCmdDispatch-None-08600";
        linear_filter_sampler_04553              = "VUID-vkCmdDispatch-magFilter-04553";
        linear_mipmap_sampler_04770              = "VUID-vkCmdDispatch-mipmapMode-04770";
        cubic_sampler_02692                      = "VUID-vkCmdDispatch-None-02692";
        corner_sampled_address_mode_02696        = "VUID-vkCmdDispatch-flags-02696";
        imageview_atomic_02691                   = "VUID-vkCmdDispatch-None-02691";
        bufferview_atomic_07888                  = "VUID-vkCmdDispatch-None-07888";
        push_constants_set_08602                 = "VUID-vkCmdDispatch-maintenance4-08602";
        sampler_imageview_type_08609             = "VUID-vkCmdDispatch-None-08609";
        sampler_implicitLod_dref_proj_08610      = "VUID-vkCmdDispatch-None-08610";
        sampler_bias_offset_08611                = "VUID-vkCmdDispatch-None-08611";
        dynamic_state_setting_commands_08608     = "VUID-vkCmdDispatch-None-08608";
        unprotected_command_buffer_02707         = "VUID-vkCmdDispatch-commandBuffer-02707";
        protected_command_buffer_02712           = "VUID-vkCmdDispatch-commandBuffer-02712";
        img_filter_cubic_02693                   = "VUID-vkCmdDispatch-None-02693";
        filter_cubic_02694                       = "VUID-vkCmdDispatch-filterCubic-02694";
        filter_cubic_min_max_02695               = "VUID-vkCmdDispatch-filterCubicMinmax-02695";
        image_view_access_64_04470               = "VUID-vkCmdDispatch-SampledType-04470";
        image_view_access_32_04471               = "VUID-vkCmdDispatch-SampledType-04471";
        image_view_sparse_64_04474               = "VUID-vkCmdDispatch-sparseImageInt64Atomics-04474";
        buffer_view_access_64_04472              = "VUID-vkCmdDispatch-SampledType-04472";
        buffer_view_access_32_04473              = "VUID-vkCmdDispatch-SampledType-04473";
        storage_image_read_without_format_07028  = "VUID-vkCmdDispatch-OpTypeImage-07028";
        storage_image_write_without_format_07027 = "VUID-vkCmdDispatch-OpTypeImage-07027";
        storage_texel_buffer_read_without_format_07030  = "VUID-vkCmdDispatch-OpTypeImage-07030";
        storage_texel_buffer_write_without_format_07029 = "VUID-vkCmdDispatch-OpTypeImage-07029";
        storage_image_write_texel_count_08795           = "VUID-vkCmdDispatch-OpImageWrite-08795";
        storage_image_write_texel_count_08796           = "VUID-vkCmdDispatch-OpImageWrite-08796";
        storage_texel_buffer_write_texel_count_04469    = "VUID-vkCmdDispatch-OpImageWrite-04469";
        depth_compare_sample_06479               = "VUID-vkCmdDispatch-None-06479";
        descriptor_buffer_bit_set_08114          = "VUID-vkCmdDispatch-None-08114";
        descriptor_buffer_bit_not_set_08115      = "VUID-vkCmdDispatch-None-08115";
        descriptor_buffer_set_offset_missing_08117 = "VUID-vkCmdDispatch-None-08117";
        image_view_dim_07752                     = "VUID-vkCmdDispatch-viewType-07752";
        image_view_numeric_format_07753          = "VUID-vkCmdDispatch-format-07753";
    }
};

struct DispatchVuidsCmdDispatchIndirect : DrawDispatchVuid {
    DispatchVuidsCmdDispatchIndirect() : DrawDispatchVuid() {
        pipeline_bound_08606                     = "VUID-vkCmdDispatchIndirect-None-08606";
        pipeline_or_shaders_bound_08607          = "VUID-vkCmdDispatchIndirect-None-08607";
        compatible_pipeline_08600                = "VUID-vkCmdDispatchIndirect-None-08600";
        linear_filter_sampler_04553              = "VUID-vkCmdDispatchIndirect-magFilter-04553";
        linear_mipmap_sampler_04770              = "VUID-vkCmdDispatchIndirect-mipmapMode-04770";
        cubic_sampler_02692                      = "VUID-vkCmdDispatchIndirect-None-02692";
        indirect_protected_cb_02646              = "VUID-vkCmdDispatchIndirect-commandBuffer-02711";
        indirect_contiguous_memory_02708         = "VUID-vkCmdDispatchIndirect-buffer-02708";
        indirect_buffer_bit_02290                = "VUID-vkCmdDispatchIndirect-buffer-02709";
        corner_sampled_address_mode_02696        = "VUID-vkCmdDispatchIndirect-flags-02696";
        imageview_atomic_02691                   = "VUID-vkCmdDispatchIndirect-None-02691";
        bufferview_atomic_07888                  = "VUID-vkCmdDispatchIndirect-None-07888";
        push_constants_set_08602                 = "VUID-vkCmdDispatchIndirect-maintenance4-08602";
        sampler_imageview_type_08609             = "VUID-vkCmdDispatchIndirect-None-08609";
        sampler_implicitLod_dref_proj_08610      = "VUID-vkCmdDispatchIndirect-None-08610";
        sampler_bias_offset_08611                = "VUID-vkCmdDispatchIndirect-None-08611";
        dynamic_state_setting_commands_08608     = "VUID-vkCmdDispatchIndirect-None-08608";
        unprotected_command_buffer_02707         = "VUID-vkCmdDispatchIndirect-commandBuffer-02707";
        img_filter_cubic_02693                   = "VUID-vkCmdDispatchIndirect-None-02693";
        filter_cubic_02694                       = "VUID-vkCmdDispatchIndirect-filterCubic-02694";
        filter_cubic_min_max_02695               = "VUID-vkCmdDispatchIndirect-filterCubicMinmax-02695";
        image_view_access_64_04470               = "VUID-vkCmdDispatchIndirect-SampledType-04470";
        image_view_access_32_04471               = "VUID-vkCmdDispatchIndirect-SampledType-04471";
        image_view_sparse_64_04474               = "VUID-vkCmdDispatchIndirect-sparseImageInt64Atomics-04474";
        buffer_view_access_64_04472              = "VUID-vkCmdDispatchIndirect-SampledType-04472";
        buffer_view_access_32_04473              = "VUID-vkCmdDispatchIndirect-SampledType-04473";
        storage_image_read_without_format_07028  = "VUID-vkCmdDispatchIndirect-OpTypeImage-07028";
        storage_image_write_without_format_07027 = "VUID-vkCmdDispatchIndirect-OpTypeImage-07027";
        storage_texel_buffer_read_without_format_07030  = "VUID-vkCmdDispatchIndirect-OpTypeImage-07030";
        storage_texel_buffer_write_without_format_07029 = "VUID-vkCmdDispatchIndirect-OpTypeImage-07029";
        storage_image_write_texel_count_08795           = "VUID-vkCmdDispatchIndirect-OpImageWrite-08795";
        storage_image_write_texel_count_08796           = "VUID-vkCmdDispatchIndirect-OpImageWrite-08796";
        storage_texel_buffer_write_texel_count_04469    = "VUID-vkCmdDispatchIndirect-OpImageWrite-04469";
        depth_compare_sample_06479               = "VUID-vkCmdDispatchIndirect-None-06479";
        descriptor_buffer_bit_set_08114          = "VUID-vkCmdDispatchIndirect-None-08114";
        descriptor_buffer_bit_not_set_08115      = "VUID-vkCmdDispatchIndirect-None-08115";
        descriptor_buffer_set_offset_missing_08117 = "VUID-vkCmdDispatchIndirect-None-08117";
        image_view_dim_07752                     = "VUID-vkCmdDispatchIndirect-viewType-07752";
        image_view_numeric_format_07753          = "VUID-vkCmdDispatchIndirect-format-07753";
    }
};

struct DispatchVuidsCmdDrawIndirectCount : DrawDispatchVuid {
    DispatchVuidsCmdDrawIndirectCount() : DrawDispatchVuid() {
        pipeline_bound_08606                     = "VUID-vkCmdDrawIndirectCount-None-08606";
        pipeline_or_shaders_bound_08607          = "VUID-vkCmdDrawIndirectCount-None-08607";
        vertex_binding_04007                     = "VUID-vkCmdDrawIndirectCount-None-04007";
        vertex_binding_null_04008                = "VUID-vkCmdDrawIndirectCount-None-04008";
        compatible_pipeline_08600                = "VUID-vkCmdDrawIndirectCount-None-08600";
        render_pass_compatible_02684             = "VUID-vkCmdDrawIndirectCount-renderPass-02684";
        render_pass_began_08876                  = "VUID-vkCmdDrawIndirectCount-None-08876";
        subpass_index_02685                      = "VUID-vkCmdDrawIndirectCount-subpass-02685";
        sample_location_02689                    = "VUID-vkCmdDrawIndirectCount-sampleLocationsEnable-02689";
        linear_filter_sampler_04553              = "VUID-vkCmdDrawIndirectCount-magFilter-04553";
        linear_mipmap_sampler_04770              = "VUID-vkCmdDrawIndirectCount-mipmapMode-04770";
        cubic_sampler_02692                      = "VUID-vkCmdDrawIndirectCount-None-02692";
        indirect_protected_cb_02646              = "VUID-vkCmdDrawIndirectCount-commandBuffer-02711";
        indirect_contiguous_memory_02708         = "VUID-vkCmdDrawIndirectCount-buffer-02708";
        indirect_count_contiguous_memory_02714   = "VUID-vkCmdDrawIndirectCount-countBuffer-02714";
        indirect_buffer_bit_02290                = "VUID-vkCmdDrawIndirectCount-buffer-02709";
        indirect_count_buffer_bit_02715          = "VUID-vkCmdDrawIndirectCount-countBuffer-02715";
        indirect_count_offset_04129              = "VUID-vkCmdDrawIndirectCount-countBufferOffset-04129";
        viewport_count_03417                     = "VUID-vkCmdDrawIndirectCount-viewportCount-03417";
        scissor_count_03418                      = "VUID-vkCmdDrawIndirectCount-scissorCount-03418";
        viewport_scissor_count_03419             = "VUID-vkCmdDrawIndirectCount-viewportCount-03419";
        primitive_topology_class_07500           = "VUID-vkCmdDrawIndirectCount-dynamicPrimitiveTopologyUnrestricted-07500";
        corner_sampled_address_mode_02696        = "VUID-vkCmdDrawIndirectCount-flags-02696";
        imageview_atomic_02691                   = "VUID-vkCmdDrawIndirectCount-None-02691";
        bufferview_atomic_07888                  = "VUID-vkCmdDrawIndirectCount-None-07888";
        push_constants_set_08602                 = "VUID-vkCmdDrawIndirectCount-maintenance4-08602";
        image_subresources_render_pass_write_06537 = "VUID-vkCmdDrawIndirectCount-None-06537";
        image_subresources_subpass_read_09003    = "VUID-vkCmdDrawIndirectCount-None-09003";
        image_subresources_subpass_write_06539   = "VUID-vkCmdDrawIndirectCount-None-06539";
        sampler_imageview_type_08609             = "VUID-vkCmdDrawIndirectCount-None-08609";
        sampler_implicitLod_dref_proj_08610      = "VUID-vkCmdDrawIndirectCount-None-08610";
        sampler_bias_offset_08611                = "VUID-vkCmdDrawIndirectCount-None-08611";
        vertex_binding_attribute_02721           = "VUID-vkCmdDrawIndirectCount-None-02721";
        dynamic_state_setting_commands_08608     = "VUID-vkCmdDrawIndirectCount-None-08608";
        msrtss_rasterization_samples_07284       = "VUID-vkCmdDrawIndirectCount-multisampledRenderToSingleSampled-07284";
        unprotected_command_buffer_02707         = "VUID-vkCmdDrawIndirectCount-commandBuffer-02707";
        max_multiview_instance_index_02688       = "VUID-vkCmdDrawIndirectCount-maxMultiviewInstanceIndex-02688";
        img_filter_cubic_02693                   = "VUID-vkCmdDrawIndirectCount-None-02693";
        filter_cubic_02694                       = "VUID-vkCmdDrawIndirectCount-filterCubic-02694";
        filter_cubic_min_max_02695               = "VUID-vkCmdDrawIndirectCount-filterCubicMinmax-02695";
        viewport_count_primitive_shading_rate_04552 = "VUID-vkCmdDrawIndirectCount-primitiveFragmentShadingRateWithMultipleViewports-04552";
        patch_control_points_04875               = "VUID-vkCmdDrawIndirectCount-None-04875";
        rasterizer_discard_enable_04876          = "VUID-vkCmdDrawIndirectCount-None-04876";
        depth_bias_enable_04877                  = "VUID-vkCmdDrawIndirectCount-None-04877";
        logic_op_04878                           = "VUID-vkCmdDrawIndirectCount-logicOp-04878";
        primitive_restart_enable_04879           = "VUID-vkCmdDrawIndirectCount-None-04879";
        vertex_input_04912                       = "VUID-vkCmdDrawIndirectCount-None-04912";
        vertex_input_binding_stride_04913        = "VUID-vkCmdDrawIndirectCount-pStrides-04913";
        vertex_input_04914                       = "VUID-vkCmdDrawIndirectCount-None-04914";
        blend_enable_04727                       = "VUID-vkCmdDrawIndirectCount-blendEnable-04727";
        dynamic_discard_rectangle_07751          = "VUID-vkCmdDrawIndirectCount-None-07751";
        dynamic_discard_rectangle_enable_07880   = "VUID-vkCmdDrawIndirectCount-None-07880";
        dynamic_discard_rectangle_mode_07881     =  "VUID-vkCmdDrawIndirectCount-None-07881";
        dynamic_exclusive_scissor_enable_07878   = "VUID-vkCmdDrawIndirectCount-None-07878";
        dynamic_exclusive_scissor_07879          = "VUID-vkCmdDrawIndirectCount-None-07879";
        dynamic_color_write_enable_07749         = "VUID-vkCmdDrawIndirectCount-None-07749";
        dynamic_color_write_enable_count_07750   = "VUID-vkCmdDrawIndirectCount-attachmentCount-07750";
        dynamic_attachment_feedback_loop_08877   = "VUID-vkCmdDrawIndirectCount-None-08877";
        dynamic_rendering_view_mask_06178        = "VUID-vkCmdDrawIndirectCount-viewMask-06178";
        dynamic_rendering_color_count_06179      = "VUID-vkCmdDrawIndirectCount-colorAttachmentCount-06179";
        dynamic_rendering_color_formats_08910    = "VUID-vkCmdDrawIndirectCount-dynamicRenderingUnusedAttachments-08910";
        dynamic_rendering_unused_attachments_08911 = "VUID-vkCmdDrawIndirectCount-dynamicRenderingUnusedAttachments-08911";
        dynamic_rendering_undefined_color_formats_08912 = "VUID-vkCmdDrawIndirectCount-dynamicRenderingUnusedAttachments-08912";
        dynamic_rendering_undefined_depth_format_08916 = "VUID-vkCmdDrawIndirectCount-dynamicRenderingUnusedAttachments-08916";
        dynamic_rendering_undefined_stencil_format_08916 = "VUID-vkCmdDrawIndirectCount-dynamicRenderingUnusedAttachments-08916";
        dynamic_rendering_depth_format_08914     = "VUID-vkCmdDrawIndirectCount-dynamicRenderingUnusedAttachments-08914";
        dynamic_rendering_unused_attachments_08915 = "VUID-vkCmdDrawIndirectCount-dynamicRenderingUnusedAttachments-08915";
        dynamic_rendering_stencil_format_08917   = "VUID-vkCmdDrawIndirectCount-dynamicRenderingUnusedAttachments-08917";
        dynamic_rendering_unused_attachments_08918 = "VUID-vkCmdDrawIndirectCount-dynamicRenderingUnusedAttachments-08918";
        dynamic_rendering_fsr_06183              = "VUID-vkCmdDrawIndirectCount-imageView-06183";
        dynamic_rendering_fdm_06184              = "VUID-vkCmdDrawIndirectCount-imageView-06184";
        dynamic_rendering_color_sample_06185     = "VUID-vkCmdDrawIndirectCount-colorAttachmentCount-06185";
        dynamic_rendering_depth_sample_06186     = "VUID-vkCmdDrawIndirectCount-pDepthAttachment-06186";
        dynamic_rendering_stencil_sample_06187   = "VUID-vkCmdDrawIndirectCount-pStencilAttachment-06187";
        dynamic_rendering_06198                  = "VUID-vkCmdDrawIndirectCount-renderPass-06198";
        dynamic_rendering_07285                  = "VUID-vkCmdDrawIndirectCount-multisampledRenderToSingleSampled-07285";
        dynamic_rendering_07286                  = "VUID-vkCmdDrawIndirectCount-multisampledRenderToSingleSampled-07286";
        dynamic_rendering_07287                  = "VUID-vkCmdDrawIndirectCount-multisampledRenderToSingleSampled-07287";
        image_view_access_64_04470               = "VUID-vkCmdDrawIndirectCount-SampledType-04470";
        image_view_access_32_04471               = "VUID-vkCmdDrawIndirectCount-SampledType-04471";
        image_view_sparse_64_04474               = "VUID-vkCmdDrawIndirectCount-sparseImageInt64Atomics-04474";
        buffer_view_access_64_04472              = "VUID-vkCmdDrawIndirectCount-SampledType-04472";
        buffer_view_access_32_04473              = "VUID-vkCmdDrawIndirectCount-SampledType-04473";
        storage_image_read_without_format_07028  = "VUID-vkCmdDrawIndirectCount-OpTypeImage-07028";
        storage_image_write_without_format_07027 = "VUID-vkCmdDrawIndirectCount-OpTypeImage-07027";
        storage_texel_buffer_read_without_format_07030  = "VUID-vkCmdDrawIndirectCount-OpTypeImage-07030";
        storage_texel_buffer_write_without_format_07029 = "VUID-vkCmdDrawIndirectCount-OpTypeImage-07029";
        storage_image_write_texel_count_08795           = "VUID-vkCmdDrawIndirectCount-OpImageWrite-08795";
        storage_image_write_texel_count_08796           = "VUID-vkCmdDrawIndirectCount-OpImageWrite-08796";
        storage_texel_buffer_write_texel_count_04469    = "VUID-vkCmdDrawIndirectCount-OpImageWrite-04469";
        depth_compare_sample_06479               = "VUID-vkCmdDrawIndirectCount-None-06479";
        depth_read_only_06886                    = "VUID-vkCmdDrawIndirectCount-None-06886";
        stencil_read_only_06887                  = "VUID-vkCmdDrawIndirectCount-None-06887";
        dynamic_sample_locations_06666           = "VUID-vkCmdDrawIndirectCount-None-06666";
        dynamic_tessellation_domain_origin_07619 = "VUID-vkCmdDrawIndirectCount-None-07619";
        dynamic_depth_clamp_enable_07620         = "VUID-vkCmdDrawIndirectCount-None-07620";
        dynamic_polygon_mode_07621               = "VUID-vkCmdDrawIndirectCount-None-07621";
        dynamic_rasterization_samples_07622      = "VUID-vkCmdDrawIndirectCount-None-07622";
        dynamic_sample_mask_07623                = "VUID-vkCmdDrawIndirectCount-None-07623";
        dynamic_alpha_to_coverage_enable_07624   = "VUID-vkCmdDrawIndirectCount-None-07624";
        dynamic_alpha_to_coverage_component_08919 = "VUID-vkCmdDrawIndirectCount-alphaToCoverageEnable-08919";
        dynamic_alpha_to_one_enable_07625        = "VUID-vkCmdDrawIndirectCount-None-07625";
        dynamic_logic_op_enable_07626            = "VUID-vkCmdDrawIndirectCount-None-07626";
        dynamic_color_blend_enable_07476         = "VUID-vkCmdDrawIndirectCount-firstAttachment-07476";
        dynamic_color_blend_equation_07477       = "VUID-vkCmdDrawIndirectCount-firstAttachment-07477";
        dynamic_color_write_mask_07478           = "VUID-vkCmdDrawIndirectCount-firstAttachment-07478";
        dynamic_rasterization_stream_07630       = "VUID-vkCmdDrawIndirectCount-None-07630";
        dynamic_conservative_rasterization_mode_07631 = "VUID-vkCmdDrawIndirectCount-None-07631";
        dynamic_extra_primitive_overestimation_size_07632 = "VUID-vkCmdDrawIndirectCount-None-07632";
        dynamic_depth_clip_enable_07633          = "VUID-vkCmdDrawIndirectCount-None-07633";
        dynamic_sample_locations_enable_07634    = "VUID-vkCmdDrawIndirectCount-None-07634";
        dynamic_color_blend_advanced_07479       = "VUID-vkCmdDrawIndirectCount-firstAttachment-07479";
        dynamic_provoking_vertex_mode_07636      = "VUID-vkCmdDrawIndirectCount-None-07636";
        dynamic_line_rasterization_mode_07637    = "VUID-vkCmdDrawIndirectCount-None-07637";
        dynamic_line_stipple_enable_07638        = "VUID-vkCmdDrawIndirectCount-None-07638";
        dynamic_depth_clip_negative_one_to_one_07639 = "VUID-vkCmdDrawIndirectCount-None-07639";
        dynamic_viewport_w_scaling_enable_07640  = "VUID-vkCmdDrawIndirectCount-None-07640";
        dynamic_viewport_swizzle_07641           = "VUID-vkCmdDrawIndirectCount-None-07641";
        dynamic_coverage_to_color_enable_07642   = "VUID-vkCmdDrawIndirectCount-None-07642";
        dynamic_coverage_to_color_location_07643 = "VUID-vkCmdDrawIndirectCount-None-07643";
        dynamic_coverage_modulation_mode_07644   = "VUID-vkCmdDrawIndirectCount-None-07644";
        dynamic_coverage_modulation_table_enable_07645 = "VUID-vkCmdDrawIndirectCount-None-07645";
        dynamic_coverage_modulation_table_07646  = "VUID-vkCmdDrawIndirectCount-None-07646";
        dynamic_shading_rate_image_enable_07647  = "VUID-vkCmdDrawIndirectCount-None-07647";
        dynamic_representative_fragment_test_enable_07648  = "VUID-vkCmdDrawIndirectCount-None-07648";
        dynamic_coverage_reduction_mode_07649    = "VUID-vkCmdDrawIndirectCount-None-07649";
        dynamic_viewport_07831                   = "VUID-vkCmdDrawIndirectCount-None-07831";
        dynamic_scissor_07832                    = "VUID-vkCmdDrawIndirectCount-None-07832";
        dynamic_depth_bias_07834                 = "VUID-vkCmdDrawIndirectCount-None-07834";
        dynamic_line_width_07833                 = "VUID-vkCmdDrawIndirectCount-None-07833";
        dynamic_line_stipple_ext_07849           = "VUID-vkCmdDrawIndirectCount-None-07849";
        dynamic_blend_constants_07835            = "VUID-vkCmdDrawIndirectCount-None-07835";
        dynamic_depth_bounds_07836               = "VUID-vkCmdDrawIndirectCount-None-07836";
        dynamic_depth_enable_08715               = "VUID-vkCmdDrawIndirectCount-pDynamicStates-08715";
        dynamic_stencil_compare_mask_07837       = "VUID-vkCmdDrawIndirectCount-None-07837";
        dynamic_stencil_write_mask_07838         = "VUID-vkCmdDrawIndirectCount-None-07838";
        dynamic_stencil_write_mask_08716         = "VUID-vkCmdDrawIndirectCount-pDynamicStates-08716";
        dynamic_stencil_reference_07839          = "VUID-vkCmdDrawIndirectCount-None-07839";
        dynamic_state_inherited_07850            = "VUID-vkCmdDrawIndirectCount-None-07850";
        dynamic_cull_mode_07840                  = "VUID-vkCmdDrawIndirectCount-None-07840";
        dynamic_front_face_07841                 = "VUID-vkCmdDrawIndirectCount-None-07841";
        dynamic_primitive_topology_07842         = "VUID-vkCmdDrawIndirectCount-None-07842";
        dynamic_depth_test_enable_07843          = "VUID-vkCmdDrawIndirectCount-None-07843";
        dynamic_depth_write_enable_07844         = "VUID-vkCmdDrawIndirectCount-None-07844";
        dynamic_depth_compare_op_07845           = "VUID-vkCmdDrawIndirectCount-None-07845";
        dynamic_depth_bound_test_enable_07846    = "VUID-vkCmdDrawIndirectCount-None-07846";
        dynamic_stencil_test_enable_07847        = "VUID-vkCmdDrawIndirectCount-None-07847";
        dynamic_stencil_op_07848                 = "VUID-vkCmdDrawIndirectCount-None-07848";
        primitives_generated_06708               = "VUID-vkCmdDrawIndirectCount-primitivesGeneratedQueryWithRasterizerDiscard-06708";
        primitives_generated_streams_06709       = "VUID-vkCmdDrawIndirectCount-primitivesGeneratedQueryWithNonZeroStreams-06709";
        invalid_mesh_shader_stages_06481         = "VUID-vkCmdDrawIndirectCount-stage-06481";
        descriptor_buffer_bit_set_08114          = "VUID-vkCmdDrawIndirectCount-None-08114";
        descriptor_buffer_bit_not_set_08115      = "VUID-vkCmdDrawIndirectCount-None-08115";
        descriptor_buffer_set_offset_missing_08117 = "VUID-vkCmdDrawIndirectCount-None-08117";
        image_view_dim_07752                     = "VUID-vkCmdDrawIndirectCount-viewType-07752";
        image_view_numeric_format_07753          = "VUID-vkCmdDrawIndirectCount-format-07753";
        stippled_rectangular_lines_07495         = "VUID-vkCmdDrawIndirectCount-stippledLineEnable-07495";
        stippled_bresenham_lines_07496           = "VUID-vkCmdDrawIndirectCount-stippledLineEnable-07496";
        stippled_smooth_lines_07497              = "VUID-vkCmdDrawIndirectCount-stippledLineEnable-07497";
        stippled_default_strict_07498            = "VUID-vkCmdDrawIndirectCount-stippledLineEnable-07498";
        viewport_and_scissor_with_count_08635    = "VUID-vkCmdDrawIndirectCount-None-08635";
        viewport_w_scaling_08636                 = "VUID-vkCmdDrawIndirectCount-None-08636";
        shading_rate_palette_08637               = "VUID-vkCmdDrawIndirectCount-None-08637";
        exclusive_scissor_08638                  = "VUID-vkCmdDrawIndirectCount-None-08638";
        external_format_resolve_09362            = "VUID-vkCmdDrawIndirectCount-colorAttachmentCount-09362";
        external_format_resolve_09363            = "VUID-vkCmdDrawIndirectCount-None-09363";
        external_format_resolve_09364            = "VUID-vkCmdDrawIndirectCount-None-09364";
        external_format_resolve_09365            = "VUID-vkCmdDrawIndirectCount-None-09365";
        external_format_resolve_09368            = "VUID-vkCmdDrawIndirectCount-None-09368";
        external_format_resolve_09369            = "VUID-vkCmdDrawIndirectCount-None-09369";
        external_format_resolve_09372            = "VUID-vkCmdDrawIndirectCount-colorAttachmentCount-09372";
        set_rasterizer_discard_enable_08639      = "VUID-vkCmdDrawIndirectCount-None-08639";
        set_depth_bias_enable_08640              = "VUID-vkCmdDrawIndirectCount-None-08640";
        set_logic_op_08641                       = "VUID-vkCmdDrawIndirectCount-None-08641";
        set_color_blend_enable_08643             = "VUID-vkCmdDrawIndirectCount-None-08643";
        set_rasterization_samples_08644              = "VUID-vkCmdDrawIndirectCount-None-08644";
        set_color_write_enable_08646             = "VUID-vkCmdDrawIndirectCount-None-08646";
        set_color_write_enable_08647             = "VUID-vkCmdDrawIndirectCount-None-08647";
        set_discard_rectangles_enable_08648      = "VUID-vkCmdDrawIndirectCount-None-08648";
        set_discard_rectangles_mode_08649      = "VUID-vkCmdDrawIndirectCount-None-08649";
        set_depth_clamp_enable_08650             = "VUID-vkCmdDrawIndirectCount-None-08650";
        set_polygon_mode_08651                   = "VUID-vkCmdDrawIndirectCount-None-08651";
        set_rasterization_samples_08652          = "VUID-vkCmdDrawIndirectCount-None-08652";
        set_sample_mask_08653                    = "VUID-vkCmdDrawIndirectCount-None-08653";
        set_alpha_to_coverage_enable_08654       = "VUID-vkCmdDrawIndirectCount-None-08654";
        set_alpha_to_one_enable_08655            = "VUID-vkCmdDrawIndirectCount-None-08655";
        set_logic_op_enable_08656                = "VUID-vkCmdDrawIndirectCount-None-08656";
        set_color_blend_enable_08657             = "VUID-vkCmdDrawIndirectCount-None-08657";
        set_color_blend_equation_08658           = "VUID-vkCmdDrawIndirectCount-None-08658";
        set_color_write_mask_08659               = "VUID-vkCmdDrawIndirectCount-None-08659";
        set_rasterization_streams_08660          = "VUID-vkCmdDrawIndirectCount-None-08660";
        set_conservative_rasterization_mode_08661 = "VUID-vkCmdDrawIndirectCount-None-08661";
        set_extra_primitive_overestimation_size_08662 = "VUID-vkCmdDrawIndirectCount-None-08662";
        set_depth_clip_enable_08663              = "VUID-vkCmdDrawIndirectCount-None-08663";
        set_sample_locations_enable_08664        = "VUID-vkCmdDrawIndirectCount-None-08664";
        set_provoking_vertex_mode_08665          = "VUID-vkCmdDrawIndirectCount-None-08665";
        set_line_rasterization_mode_08666        = "VUID-vkCmdDrawIndirectCount-None-08666";
        set_line_rasterization_mode_08667        = "VUID-vkCmdDrawIndirectCount-None-08667";
        set_line_rasterization_mode_08668        = "VUID-vkCmdDrawIndirectCount-None-08668";
        set_line_stipple_enable_08669            = "VUID-vkCmdDrawIndirectCount-None-08669";
        set_line_stipple_enable_08670            = "VUID-vkCmdDrawIndirectCount-None-08670";
        set_line_stipple_enable_08671            = "VUID-vkCmdDrawIndirectCount-None-08671";
        set_line_stipple_08672                   = "VUID-vkCmdDrawIndirectCount-None-08672";
        set_depth_clip_negative_one_to_one_08673 = "VUID-vkCmdDrawIndirectCount-None-08673";
        set_viewport_w_scaling_enable_08674      = "VUID-vkCmdDrawIndirectCount-None-08674";
        set_viewport_swizzle_08675               = "VUID-vkCmdDrawIndirectCount-None-08675";
        set_coverage_to_color_enable_08676       = "VUID-vkCmdDrawIndirectCount-None-08676";
        set_coverage_to_color_location_08677     = "VUID-vkCmdDrawIndirectCount-None-08677";
        set_coverage_modulation_mode_08678       = "VUID-vkCmdDrawIndirectCount-None-08678";
        set_coverage_modulation_table_enable_08679 = "VUID-vkCmdDrawIndirectCount-None-08679";
        set_coverage_modulation_table_08680      = "VUID-vkCmdDrawIndirectCount-None-08680";
        set_shading_rate_image_enable_08681      = "VUID-vkCmdDrawIndirectCount-None-08681";
        set_representative_fragment_test_enable_08682 = "VUID-vkCmdDrawIndirectCount-None-08682";
        set_coverage_reduction_mode_08683        = "VUID-vkCmdDrawIndirectCount-None-08683";
        vertex_shader_08684                      = "VUID-vkCmdDrawIndirectCount-None-08684";
        tessellation_control_shader_08685        = "VUID-vkCmdDrawIndirectCount-None-08685";
        tessellation_evaluation_shader_08686     = "VUID-vkCmdDrawIndirectCount-None-08686";
        geometry_shader_08687                    = "VUID-vkCmdDrawIndirectCount-None-08687";
        fragment_shader_08688                    = "VUID-vkCmdDrawIndirectCount-None-08688";
        task_shader_08689                        = "VUID-vkCmdDrawIndirectCount-None-08689";
        mesh_shader_08690                        = "VUID-vkCmdDrawIndirectCount-None-08690";
        vert_mesh_shader_08693                   = "VUID-vkCmdDrawIndirectCount-None-08693";
        task_mesh_shader_08694                   = "VUID-vkCmdDrawIndirectCount-None-08694";
        task_mesh_shader_08695                   = "VUID-vkCmdDrawIndirectCount-None-08695";
        vert_task_mesh_shader_08696              = "VUID-vkCmdDrawIndirectCount-None-08696";
        linked_shaders_08698                     = "VUID-vkCmdDrawIndirectCount-None-08698";
        linked_shaders_08699                     = "VUID-vkCmdDrawIndirectCount-None-08699";
        shaders_push_constants_08878             = "VUID-vkCmdDrawIndirectCount-None-08878";
        shaders_descriptor_layouts_08879         = "VUID-vkCmdDrawIndirectCount-None-08879";
        set_attachment_feedback_loop_enable_08880 = "VUID-vkCmdDrawIndirectCount-None-08880";
        set_vertex_input_08882                   = "VUID-vkCmdDrawIndirectCount-None-08882";
        draw_shaders_no_task_mesh_08885          = "VUID-vkCmdDrawIndirectCount-None-08885";
        set_line_width_08617                     = "VUID-vkCmdDrawIndirectCount-None-08617";
        set_line_width_08618                     = "VUID-vkCmdDrawIndirectCount-None-08618";
        set_depth_bias_08620                     = "VUID-vkCmdDrawIndirectCount-None-08620";
        set_blend_constants_08621                = "VUID-vkCmdDrawIndirectCount-None-08621";
        set_depth_bounds_08622                   = "VUID-vkCmdDrawIndirectCount-None-08622";
        set_stencil_compare_mask_08623           = "VUID-vkCmdDrawIndirectCount-None-08623";
        set_stencil_write_mask_08624             = "VUID-vkCmdDrawIndirectCount-None-08624";
        set_stencil_reference_08625              = "VUID-vkCmdDrawIndirectCount-None-08625";
        set_sample_locations_08626               = "VUID-vkCmdDrawIndirectCount-None-08626";
        set_cull_mode_08627                      = "VUID-vkCmdDrawIndirectCount-None-08627";
        set_front_face_08628                     = "VUID-vkCmdDrawIndirectCount-None-08628";
        set_depth_test_enable_08629              = "VUID-vkCmdDrawIndirectCount-None-08629";
        set_depth_write_enable_08630             = "VUID-vkCmdDrawIndirectCount-None-08630";
        set_depth_comapre_op_08631               = "VUID-vkCmdDrawIndirectCount-None-08631";
        set_depth_bounds_test_enable_08632       = "VUID-vkCmdDrawIndirectCount-None-08632";
        set_stencil_test_enable_08633            = "VUID-vkCmdDrawIndirectCount-None-08633";
        set_stencil_op_08634                     = "VUID-vkCmdDrawIndirectCount-None-08634";
        set_line_width_08619                     = "VUID-vkCmdDrawIndirectCount-None-08619";
        set_viewport_with_count_08642            = "VUID-vkCmdDrawIndirectCount-primitiveFragmentShadingRateWithMultipleViewports-08642";
        alpha_component_word_08920               = "VUID-vkCmdDrawIndirectCount-alphaToCoverageEnable-08920";
        color_write_mask_09116                   = "VUID-vkCmdDrawIndirectCount-None-09116";
        vertex_input_format_08936                = "VUID-vkCmdDrawIndirectCount-format-08936";
        vertex_input_format_08937                = "VUID-vkCmdDrawIndirectCount-format-08937";
        vertex_input_format_09203                = "VUID-vkCmdDrawIndirectCount-None-09203";
        vertex_input_format_07939                = "VUID-vkCmdDrawIndirectCount-Input-07939";
        set_clip_space_w_scaling_09232           = "VUID-vkCmdDrawIndirectCount-None-09232";
        set_discard_rectangle_09236              = "VUID-vkCmdDrawIndirectCount-rasterizerDiscardEnable-09236";
        set_viewport_coarse_sample_order_09233   = "VUID-vkCmdDrawIndirectCount-shadingRateImage-09233";
        set_viewport_shading_rate_palette_09234  = "VUID-vkCmdDrawIndirectCount-shadingRateImage-09234";
        set_exclusive_scissor_enable_09235       = "VUID-vkCmdDrawIndirectCount-exclusiveScissor-09235";
        set_fragment_shading_rate_09238          = "VUID-vkCmdDrawIndirectCount-pipelineFragmentShadingRate-09238";
        set_tessellation_domain_origin_09237     = "VUID-vkCmdDrawIndirectCount-None-09237";
        rasterization_samples_07935              = "VUID-vkCmdDrawIndirectCount-pNext-07935";
        mesh_shader_queries_07073                = "VUID-vkCmdDrawIndirectCount-stage-07073";
        blend_advanced_07480                     = "VUID-vkCmdDrawIndirectCount-advancedBlendMaxColorAttachments-07480";
        blend_feature_07470                      = "VUID-vkCmdDrawIndirectCount-pColorBlendEnables-07470";
        color_attachment_08963                   = "VUID-vkCmdDrawIndirectCount-pColorAttachments-08963";
        depth_attachment_08964                   = "VUID-vkCmdDrawIndirectCount-pDepthAttachment-08964";
        stencil_attachment_08965                 = "VUID-vkCmdDrawIndirectCount-pStencilAttachment-08965";
        sample_locations_07482                   = "VUID-vkCmdDrawIndirectCount-sampleLocationsPerPixel-07482";
    }
};

struct DispatchVuidsCmdDrawIndexedIndirectCount : DrawDispatchVuid {
    DispatchVuidsCmdDrawIndexedIndirectCount() : DrawDispatchVuid() {
        pipeline_bound_08606                     = "VUID-vkCmdDrawIndexedIndirectCount-None-08606";
        pipeline_or_shaders_bound_08607          = "VUID-vkCmdDrawIndexedIndirectCount-None-08607";
        index_binding_07312                      = "VUID-vkCmdDrawIndexedIndirectCount-None-07312";
        vertex_binding_04007                     = "VUID-vkCmdDrawIndexedIndirectCount-None-04007";
        vertex_binding_null_04008                = "VUID-vkCmdDrawIndexedIndirectCount-None-04008";
        compatible_pipeline_08600                = "VUID-vkCmdDrawIndexedIndirectCount-None-08600";
        render_pass_compatible_02684             = "VUID-vkCmdDrawIndexedIndirectCount-renderPass-02684";
        render_pass_began_08876                  = "VUID-vkCmdDrawIndexedIndirectCount-None-08876";
        subpass_index_02685                      = "VUID-vkCmdDrawIndexedIndirectCount-subpass-02685";
        sample_location_02689                    = "VUID-vkCmdDrawIndexedIndirectCount-sampleLocationsEnable-02689";
        linear_filter_sampler_04553              = "VUID-vkCmdDrawIndexedIndirectCount-magFilter-04553";
        linear_mipmap_sampler_04770              = "VUID-vkCmdDrawIndexedIndirectCount-mipmapMode-04770";
        cubic_sampler_02692                      = "VUID-vkCmdDrawIndexedIndirectCount-None-02692";
        indirect_protected_cb_02646              = "VUID-vkCmdDrawIndexedIndirectCount-commandBuffer-02711";
        indirect_contiguous_memory_02708         = "VUID-vkCmdDrawIndexedIndirectCount-buffer-02708";
        indirect_count_contiguous_memory_02714   = "VUID-vkCmdDrawIndexedIndirectCount-countBuffer-02714";
        indirect_buffer_bit_02290                = "VUID-vkCmdDrawIndexedIndirectCount-buffer-02709";
        indirect_count_buffer_bit_02715          = "VUID-vkCmdDrawIndexedIndirectCount-countBuffer-02715";
        indirect_count_offset_04129              = "VUID-vkCmdDrawIndexedIndirectCount-countBufferOffset-04129";
        viewport_count_03417                     = "VUID-vkCmdDrawIndexedIndirectCount-viewportCount-03417";
        scissor_count_03418                      = "VUID-vkCmdDrawIndexedIndirectCount-scissorCount-03418";
        viewport_scissor_count_03419             = "VUID-vkCmdDrawIndexedIndirectCount-viewportCount-03419";
        primitive_topology_class_07500           = "VUID-vkCmdDrawIndexedIndirectCount-dynamicPrimitiveTopologyUnrestricted-07500";
        corner_sampled_address_mode_02696        = "VUID-vkCmdDrawIndexedIndirectCount-flags-02696";
        imageview_atomic_02691                   = "VUID-vkCmdDrawIndexedIndirectCount-None-02691";
        bufferview_atomic_07888                  = "VUID-vkCmdDrawIndexedIndirectCount-None-07888";
        push_constants_set_08602                 = "VUID-vkCmdDrawIndexedIndirectCount-maintenance4-08602";
        image_subresources_render_pass_write_06537 = "VUID-vkCmdDrawIndexedIndirectCount-None-06537";
        image_subresources_subpass_read_09003    = "VUID-vkCmdDrawIndexedIndirectCount-None-09003";
        image_subresources_subpass_write_06539   = "VUID-vkCmdDrawIndexedIndirectCount-None-06539";
        sampler_imageview_type_08609             = "VUID-vkCmdDrawIndexedIndirectCount-None-08609";
        sampler_implicitLod_dref_proj_08610      = "VUID-vkCmdDrawIndexedIndirectCount-None-08610";
        sampler_bias_offset_08611                = "VUID-vkCmdDrawIndexedIndirectCount-None-08611";
        vertex_binding_attribute_02721           = "VUID-vkCmdDrawIndexedIndirectCount-None-02721";
        dynamic_state_setting_commands_08608     = "VUID-vkCmdDrawIndexedIndirectCount-None-08608";
        msrtss_rasterization_samples_07284       = "VUID-vkCmdDrawIndexedIndirectCount-multisampledRenderToSingleSampled-07284";
        unprotected_command_buffer_02707         = "VUID-vkCmdDrawIndexedIndirectCount-commandBuffer-02707";
        max_multiview_instance_index_02688       = "VUID-vkCmdDrawIndexedIndirectCount-maxMultiviewInstanceIndex-02688";
        img_filter_cubic_02693                   = "VUID-vkCmdDrawIndexedIndirectCount-None-02693";
        filter_cubic_02694                       = "VUID-vkCmdDrawIndexedIndirectCount-filterCubic-02694";
        filter_cubic_min_max_02695               = "VUID-vkCmdDrawIndexedIndirectCount-filterCubicMinmax-02695";
        viewport_count_primitive_shading_rate_04552 = "VUID-vkCmdDrawIndexedIndirectCount-primitiveFragmentShadingRateWithMultipleViewports-04552";
        patch_control_points_04875               = "VUID-vkCmdDrawIndexedIndirectCount-None-04875";
        rasterizer_discard_enable_04876          = "VUID-vkCmdDrawIndexedIndirectCount-None-04876";
        depth_bias_enable_04877                  = "VUID-vkCmdDrawIndexedIndirectCount-None-04877";
        logic_op_04878                           = "VUID-vkCmdDrawIndexedIndirectCount-logicOp-04878";
        primitive_restart_enable_04879           = "VUID-vkCmdDrawIndexedIndirectCount-None-04879";
        vertex_input_04912                       = "VUID-vkCmdDrawIndexedIndirectCount-None-04912";
        vertex_input_binding_stride_04913        = "VUID-vkCmdDrawIndexedIndirectCount-pStrides-04913";
        vertex_input_04914                       = "VUID-vkCmdDrawIndexedIndirectCount-None-04914";
        blend_enable_04727                       = "VUID-vkCmdDrawIndexedIndirectCount-blendEnable-04727";
        dynamic_discard_rectangle_07751          = "VUID-vkCmdDrawIndexedIndirectCount-None-07751";
        dynamic_discard_rectangle_enable_07880   = "VUID-vkCmdDrawIndexedIndirectCount-None-07880";
        dynamic_discard_rectangle_mode_07881     =  "VUID-vkCmdDrawIndexedIndirectCount-None-07881";
        dynamic_exclusive_scissor_enable_07878   = "VUID-vkCmdDrawIndexedIndirectCount-None-07878";
        dynamic_exclusive_scissor_07879          = "VUID-vkCmdDrawIndexedIndirectCount-None-07879";
        dynamic_color_write_enable_07749         = "VUID-vkCmdDrawIndexedIndirectCount-None-07749";
        dynamic_color_write_enable_count_07750   = "VUID-vkCmdDrawIndexedIndirectCount-attachmentCount-07750";
        dynamic_attachment_feedback_loop_08877   = "VUID-vkCmdDrawIndexedIndirectCount-None-08877";
        dynamic_rendering_view_mask_06178        = "VUID-vkCmdDrawIndexedIndirectCount-viewMask-06178";
        dynamic_rendering_color_count_06179      = "VUID-vkCmdDrawIndexedIndirectCount-colorAttachmentCount-06179";
        dynamic_rendering_color_formats_08910    = "VUID-vkCmdDrawIndexedIndirectCount-dynamicRenderingUnusedAttachments-08910";
        dynamic_rendering_unused_attachments_08911 = "VUID-vkCmdDrawIndexedIndirectCount-dynamicRenderingUnusedAttachments-08911";
        dynamic_rendering_undefined_color_formats_08912 = "VUID-vkCmdDrawIndexedIndirectCount-dynamicRenderingUnusedAttachments-08912";
        dynamic_rendering_undefined_depth_format_08916 = "VUID-vkCmdDrawIndexedIndirectCount-dynamicRenderingUnusedAttachments-08916";
        dynamic_rendering_undefined_stencil_format_08916 = "VUID-vkCmdDrawIndexedIndirectCount-dynamicRenderingUnusedAttachments-08916";
        dynamic_rendering_depth_format_08914     = "VUID-vkCmdDrawIndexedIndirectCount-dynamicRenderingUnusedAttachments-08914";
        dynamic_rendering_unused_attachments_08915 = "VUID-vkCmdDrawIndexedIndirectCount-dynamicRenderingUnusedAttachments-08915";
        dynamic_rendering_stencil_format_08917   = "VUID-vkCmdDrawIndexedIndirectCount-dynamicRenderingUnusedAttachments-08917";
        dynamic_rendering_unused_attachments_08918 = "VUID-vkCmdDrawIndexedIndirectCount-dynamicRenderingUnusedAttachments-08918";
        dynamic_rendering_fsr_06183              = "VUID-vkCmdDrawIndexedIndirectCount-imageView-06183";
        dynamic_rendering_fdm_06184              = "VUID-vkCmdDrawIndexedIndirectCount-imageView-06184";
        dynamic_rendering_color_sample_06185     = "VUID-vkCmdDrawIndexedIndirectCount-colorAttachmentCount-06185";
        dynamic_rendering_depth_sample_06186     = "VUID-vkCmdDrawIndexedIndirectCount-pDepthAttachment-06186";
        dynamic_rendering_stencil_sample_06187   = "VUID-vkCmdDrawIndexedIndirectCount-pStencilAttachment-06187";
        dynamic_rendering_06198                  = "VUID-vkCmdDrawIndexedIndirectCount-renderPass-06198";
        dynamic_rendering_07285                  = "VUID-vkCmdDrawIndexedIndirectCount-multisampledRenderToSingleSampled-07285";
        dynamic_rendering_07286                  = "VUID-vkCmdDrawIndexedIndirectCount-multisampledRenderToSingleSampled-07286";
        dynamic_rendering_07287                  = "VUID-vkCmdDrawIndexedIndirectCount-multisampledRenderToSingleSampled-07287";
        image_view_access_64_04470               = "VUID-vkCmdDrawIndexedIndirectCount-SampledType-04470";
        image_view_access_32_04471               = "VUID-vkCmdDrawIndexedIndirectCount-SampledType-04471";
        image_view_sparse_64_04474               = "VUID-vkCmdDraw-sparseImageInt64Atomics-04474";
        buffer_view_access_64_04472              = "VUID-vkCmdDrawIndexedIndirectCount-SampledType-04472";
        buffer_view_access_32_04473              = "VUID-vkCmdDrawIndexedIndirectCount-SampledType-04473";
        storage_image_read_without_format_07028  = "VUID-vkCmdDrawIndexedIndirectCount-OpTypeImage-07028";
        storage_image_write_without_format_07027 = "VUID-vkCmdDrawIndexedIndirectCount-OpTypeImage-07027";
        storage_texel_buffer_read_without_format_07030  = "VUID-vkCmdDrawIndexedIndirectCount-OpTypeImage-07030";
        storage_texel_buffer_write_without_format_07029 = "VUID-vkCmdDrawIndexedIndirectCount-OpTypeImage-07029";
        storage_image_write_texel_count_08795           = "VUID-vkCmdDrawIndexedIndirectCount-OpImageWrite-08795";
        storage_image_write_texel_count_08796           = "VUID-vkCmdDrawIndexedIndirectCount-OpImageWrite-08796";
        storage_texel_buffer_write_texel_count_04469    = "VUID-vkCmdDrawIndexedIndirectCount-OpImageWrite-04469";
        depth_compare_sample_06479               = "VUID-vkCmdDrawIndexedIndirectCount-None-06479";
        depth_read_only_06886                    = "VUID-vkCmdDrawIndexedIndirectCount-None-06886";
        stencil_read_only_06887                  = "VUID-vkCmdDrawIndexedIndirectCount-None-06887";
        dynamic_sample_locations_06666           = "VUID-vkCmdDrawIndexedIndirectCount-None-06666";
        dynamic_tessellation_domain_origin_07619 = "VUID-vkCmdDrawIndexedIndirectCount-None-07619";
        dynamic_depth_clamp_enable_07620         = "VUID-vkCmdDrawIndexedIndirectCount-None-07620";
        dynamic_polygon_mode_07621               = "VUID-vkCmdDrawIndexedIndirectCount-None-07621";
        dynamic_rasterization_samples_07622      = "VUID-vkCmdDrawIndexedIndirectCount-None-07622";
        dynamic_sample_mask_07623                = "VUID-vkCmdDrawIndexedIndirectCount-None-07623";
        dynamic_alpha_to_coverage_enable_07624   = "VUID-vkCmdDrawIndexedIndirectCount-None-07624";
        dynamic_alpha_to_coverage_component_08919 = "VUID-vkCmdDrawIndexedIndirectCount-alphaToCoverageEnable-08919";
        dynamic_alpha_to_one_enable_07625        = "VUID-vkCmdDrawIndexedIndirectCount-None-07625";
        dynamic_logic_op_enable_07626            = "VUID-vkCmdDrawIndexedIndirectCount-None-07626";
        dynamic_color_blend_enable_07476         = "VUID-vkCmdDrawIndexedIndirectCount-firstAttachment-07476";
        dynamic_color_blend_equation_07477       = "VUID-vkCmdDrawIndexedIndirectCount-firstAttachment-07477";
        dynamic_color_write_mask_07478           = "VUID-vkCmdDrawIndexedIndirectCount-firstAttachment-07478";
        dynamic_rasterization_stream_07630       = "VUID-vkCmdDrawIndexedIndirectCount-None-07630";
        dynamic_conservative_rasterization_mode_07631 = "VUID-vkCmdDrawIndexedIndirectCount-None-07631";
        dynamic_extra_primitive_overestimation_size_07632 = "VUID-vkCmdDrawIndexedIndirectCount-None-07632";
        dynamic_depth_clip_enable_07633          = "VUID-vkCmdDrawIndexedIndirectCount-None-07633";
        dynamic_sample_locations_enable_07634    = "VUID-vkCmdDrawIndexedIndirectCount-None-07634";
        dynamic_color_blend_advanced_07479       = "VUID-vkCmdDrawIndexedIndirectCount-firstAttachment-07479";
        dynamic_provoking_vertex_mode_07636      = "VUID-vkCmdDrawIndexedIndirectCount-None-07636";
        dynamic_line_rasterization_mode_07637    = "VUID-vkCmdDrawIndexedIndirectCount-None-07637";
        dynamic_line_stipple_enable_07638        = "VUID-vkCmdDrawIndexedIndirectCount-None-07638";
        dynamic_depth_clip_negative_one_to_one_07639 = "VUID-vkCmdDrawIndexedIndirectCount-None-07639";
        dynamic_viewport_w_scaling_enable_07640  = "VUID-vkCmdDrawIndexedIndirectCount-None-07640";
        dynamic_viewport_swizzle_07641           = "VUID-vkCmdDrawIndexedIndirectCount-None-07641";
        dynamic_coverage_to_color_enable_07642   = "VUID-vkCmdDrawIndexedIndirectCount-None-07642";
        dynamic_coverage_to_color_location_07643 = "VUID-vkCmdDrawIndexedIndirectCount-None-07643";
        dynamic_coverage_modulation_mode_07644   = "VUID-vkCmdDrawIndexedIndirectCount-None-07644";
        dynamic_coverage_modulation_table_enable_07645 = "VUID-vkCmdDrawIndexedIndirectCount-None-07645";
        dynamic_coverage_modulation_table_07646  = "VUID-vkCmdDrawIndexedIndirectCount-None-07646";
        dynamic_shading_rate_image_enable_07647  = "VUID-vkCmdDrawIndexedIndirectCount-None-07647";
        dynamic_representative_fragment_test_enable_07648  = "VUID-vkCmdDrawIndexedIndirectCount-None-07648";
        dynamic_coverage_reduction_mode_07649    = "VUID-vkCmdDrawIndexedIndirectCount-None-07649";
        dynamic_viewport_07831                   = "VUID-vkCmdDrawIndexedIndirectCount-None-07831";
        dynamic_scissor_07832                    = "VUID-vkCmdDrawIndexedIndirectCount-None-07832";
        dynamic_depth_bias_07834                 = "VUID-vkCmdDrawIndexedIndirectCount-None-07834";
        dynamic_line_width_07833                 = "VUID-vkCmdDrawIndexedIndirectCount-None-07833";
        dynamic_line_stipple_ext_07849           = "VUID-vkCmdDrawIndexedIndirectCount-None-07849";
        dynamic_blend_constants_07835            = "VUID-vkCmdDrawIndexedIndirectCount-None-07835";
        dynamic_depth_bounds_07836               = "VUID-vkCmdDrawIndexedIndirectCount-None-07836";
        dynamic_depth_enable_08715               = "VUID-vkCmdDrawIndexedIndirectCount-pDynamicStates-08715";
        dynamic_stencil_compare_mask_07837       = "VUID-vkCmdDrawIndexedIndirectCount-None-07837";
        dynamic_stencil_write_mask_07838         = "VUID-vkCmdDrawIndexedIndirectCount-None-07838";
        dynamic_stencil_write_mask_08716         = "VUID-vkCmdDrawIndexedIndirectCount-pDynamicStates-08716";
        dynamic_stencil_reference_07839          = "VUID-vkCmdDrawIndexedIndirectCount-None-07839";
        dynamic_state_inherited_07850            = "VUID-vkCmdDrawIndexedIndirectCount-None-07850";
        dynamic_cull_mode_07840                  = "VUID-vkCmdDrawIndexedIndirectCount-None-07840";
        dynamic_front_face_07841                 = "VUID-vkCmdDrawIndexedIndirectCount-None-07841";
        dynamic_primitive_topology_07842         = "VUID-vkCmdDrawIndexedIndirectCount-None-07842";
        dynamic_depth_test_enable_07843          = "VUID-vkCmdDrawIndexedIndirectCount-None-07843";
        dynamic_depth_write_enable_07844         = "VUID-vkCmdDrawIndexedIndirectCount-None-07844";
        dynamic_depth_compare_op_07845           = "VUID-vkCmdDrawIndexedIndirectCount-None-07845";
        dynamic_depth_bound_test_enable_07846    = "VUID-vkCmdDrawIndexedIndirectCount-None-07846";
        dynamic_stencil_test_enable_07847        = "VUID-vkCmdDrawIndexedIndirectCount-None-07847";
        dynamic_stencil_op_07848                 = "VUID-vkCmdDrawIndexedIndirectCount-None-07848";
        primitives_generated_06708               = "VUID-vkCmdDrawIndexedIndirectCount-primitivesGeneratedQueryWithRasterizerDiscard-06708";
        primitives_generated_streams_06709       = "VUID-vkCmdDrawIndexedIndirectCount-primitivesGeneratedQueryWithNonZeroStreams-06709";
        invalid_mesh_shader_stages_06481         = "VUID-vkCmdDrawIndexedIndirectCount-stage-06481";
        descriptor_buffer_bit_set_08114          = "VUID-vkCmdDrawIndexedIndirectCount-None-08114";
        descriptor_buffer_bit_not_set_08115      = "VUID-vkCmdDrawIndexedIndirectCount-None-08115";
        descriptor_buffer_set_offset_missing_08117 = "VUID-vkCmdDrawIndexedIndirectCount-None-08117";
        image_view_dim_07752                     = "VUID-vkCmdDrawIndexedIndirectCount-viewType-07752";
        image_view_numeric_format_07753          = "VUID-vkCmdDrawIndexedIndirectCount-format-07753";
        stippled_rectangular_lines_07495         = "VUID-vkCmdDrawIndexedIndirectCount-stippledLineEnable-07495";
        stippled_bresenham_lines_07496           = "VUID-vkCmdDrawIndexedIndirectCount-stippledLineEnable-07496";
        stippled_smooth_lines_07497              = "VUID-vkCmdDrawIndexedIndirectCount-stippledLineEnable-07497";
        stippled_default_strict_07498            = "VUID-vkCmdDrawIndexedIndirectCount-stippledLineEnable-07498";
        viewport_and_scissor_with_count_08635    = "VUID-vkCmdDrawIndexedIndirectCount-None-08635";
        viewport_w_scaling_08636                 = "VUID-vkCmdDrawIndexedIndirectCount-None-08636";
        shading_rate_palette_08637               = "VUID-vkCmdDrawIndexedIndirectCount-None-08637";
        exclusive_scissor_08638                  = "VUID-vkCmdDrawIndexedIndirectCount-None-08638";
        external_format_resolve_09362            = "VUID-vkCmdDrawIndexedIndirectCount-colorAttachmentCount-09362";
        external_format_resolve_09363            = "VUID-vkCmdDrawIndexedIndirectCount-None-09363";
        external_format_resolve_09364            = "VUID-vkCmdDrawIndexedIndirectCount-None-09364";
        external_format_resolve_09365            = "VUID-vkCmdDrawIndexedIndirectCount-None-09365";
        external_format_resolve_09368            = "VUID-vkCmdDrawIndexedIndirectCount-None-09368";
        external_format_resolve_09369            = "VUID-vkCmdDrawIndexedIndirectCount-None-09369";
        external_format_resolve_09372            = "VUID-vkCmdDrawIndexedIndirectCount-colorAttachmentCount-09372";
        set_rasterizer_discard_enable_08639      = "VUID-vkCmdDrawIndexedIndirectCount-None-08639";
        set_depth_bias_enable_08640              = "VUID-vkCmdDrawIndexedIndirectCount-None-08640";
        set_logic_op_08641                       = "VUID-vkCmdDrawIndexedIndirectCount-None-08641";
        set_color_blend_enable_08643             = "VUID-vkCmdDrawIndexedIndirectCount-None-08643";
        set_rasterization_samples_08644              = "VUID-vkCmdDrawIndexedIndirectCount-None-08644";
        set_color_write_enable_08646             = "VUID-vkCmdDrawIndexedIndirectCount-None-08646";
        set_color_write_enable_08647             = "VUID-vkCmdDrawIndexedIndirectCount-None-08647";
        set_discard_rectangles_enable_08648      = "VUID-vkCmdDrawIndexedIndirectCount-None-08648";
        set_discard_rectangles_mode_08649      = "VUID-vkCmdDrawIndexedIndirectCount-None-08649";
        set_depth_clamp_enable_08650             = "VUID-vkCmdDrawIndexedIndirectCount-None-08650";
        set_polygon_mode_08651                   = "VUID-vkCmdDrawIndexedIndirectCount-None-08651";
        set_rasterization_samples_08652          = "VUID-vkCmdDrawIndexedIndirectCount-None-08652";
        set_sample_mask_08653                    = "VUID-vkCmdDrawIndexedIndirectCount-None-08653";
        set_alpha_to_coverage_enable_08654       = "VUID-vkCmdDrawIndexedIndirectCount-None-08654";
        set_alpha_to_one_enable_08655            = "VUID-vkCmdDrawIndexedIndirectCount-None-08655";
        set_logic_op_enable_08656                = "VUID-vkCmdDrawIndexedIndirectCount-None-08656";
        set_color_blend_enable_08657             = "VUID-vkCmdDrawIndexedIndirectCount-None-08657";
        set_color_blend_equation_08658           = "VUID-vkCmdDrawIndexedIndirectCount-None-08658";
        set_color_write_mask_08659               = "VUID-vkCmdDrawIndexedIndirectCount-None-08659";
        set_rasterization_streams_08660          = "VUID-vkCmdDrawIndexedIndirectCount-None-08660";
        set_conservative_rasterization_mode_08661 = "VUID-vkCmdDrawIndexedIndirectCount-None-08661";
        set_extra_primitive_overestimation_size_08662 = "VUID-vkCmdDrawIndexedIndirectCount-None-08662";
        set_depth_clip_enable_08663              = "VUID-vkCmdDrawIndexedIndirectCount-None-08663";
        set_sample_locations_enable_08664        = "VUID-vkCmdDrawIndexedIndirectCount-None-08664";
        set_provoking_vertex_mode_08665          = "VUID-vkCmdDrawIndexedIndirectCount-None-08665";
        set_line_rasterization_mode_08666        = "VUID-vkCmdDrawIndexedIndirectCount-None-08666";
        set_line_rasterization_mode_08667        = "VUID-vkCmdDrawIndexedIndirectCount-None-08667";
        set_line_rasterization_mode_08668        = "VUID-vkCmdDrawIndexedIndirectCount-None-08668";
        set_line_stipple_enable_08669            = "VUID-vkCmdDrawIndexedIndirectCount-None-08669";
        set_line_stipple_enable_08670            = "VUID-vkCmdDrawIndexedIndirectCount-None-08670";
        set_line_stipple_enable_08671            = "VUID-vkCmdDrawIndexedIndirectCount-None-08671";
        set_line_stipple_08672                   = "VUID-vkCmdDrawIndexedIndirectCount-None-08672";
        set_depth_clip_negative_one_to_one_08673 = "VUID-vkCmdDrawIndexedIndirectCount-None-08673";
        set_viewport_w_scaling_enable_08674      = "VUID-vkCmdDrawIndexedIndirectCount-None-08674";
        set_viewport_swizzle_08675               = "VUID-vkCmdDrawIndexedIndirectCount-None-08675";
        set_coverage_to_color_enable_08676       = "VUID-vkCmdDrawIndexedIndirectCount-None-08676";
        set_coverage_to_color_location_08677     = "VUID-vkCmdDrawIndexedIndirectCount-None-08677";
        set_coverage_modulation_mode_08678       = "VUID-vkCmdDrawIndexedIndirectCount-None-08678";
        set_coverage_modulation_table_enable_08679 = "VUID-vkCmdDrawIndexedIndirectCount-None-08679";
        set_coverage_modulation_table_08680      = "VUID-vkCmdDrawIndexedIndirectCount-None-08680";
        set_shading_rate_image_enable_08681      = "VUID-vkCmdDrawIndexedIndirectCount-None-08681";
        set_representative_fragment_test_enable_08682 = "VUID-vkCmdDrawIndexedIndirectCount-None-08682";
        set_coverage_reduction_mode_08683        = "VUID-vkCmdDrawIndexedIndirectCount-None-08683";
        vertex_shader_08684                      = "VUID-vkCmdDrawIndexedIndirectCount-None-08684";
        tessellation_control_shader_08685        = "VUID-vkCmdDrawIndexedIndirectCount-None-08685";
        tessellation_evaluation_shader_08686     = "VUID-vkCmdDrawIndexedIndirectCount-None-08686";
        geometry_shader_08687                    = "VUID-vkCmdDrawIndexedIndirectCount-None-08687";
        fragment_shader_08688                    = "VUID-vkCmdDrawIndexedIndirectCount-None-08688";
        task_shader_08689                        = "VUID-vkCmdDrawIndexedIndirectCount-None-08689";
        mesh_shader_08690                        = "VUID-vkCmdDrawIndexedIndirectCount-None-08690";
        vert_mesh_shader_08693                   = "VUID-vkCmdDrawIndexedIndirectCount-None-08693";
        task_mesh_shader_08694                   = "VUID-vkCmdDrawIndexedIndirectCount-None-08694";
        task_mesh_shader_08695                   = "VUID-vkCmdDrawIndexedIndirectCount-None-08695";
        vert_task_mesh_shader_08696              = "VUID-vkCmdDrawIndexedIndirectCount-None-08696";
        linked_shaders_08698                     = "VUID-vkCmdDrawIndexedIndirectCount-None-08698";
        linked_shaders_08699                     = "VUID-vkCmdDrawIndexedIndirectCount-None-08699";
        shaders_push_constants_08878             = "VUID-vkCmdDrawIndexedIndirectCount-None-08878";
        shaders_descriptor_layouts_08879         = "VUID-vkCmdDrawIndexedIndirectCount-None-08879";
        set_attachment_feedback_loop_enable_08880 = "VUID-vkCmdDrawIndexedIndirectCount-None-08880";
        set_vertex_input_08882                   = "VUID-vkCmdDrawIndexedIndirectCount-None-08882";
        draw_shaders_no_task_mesh_08885          = "VUID-vkCmdDrawIndexedIndirectCount-None-08885";
        set_line_width_08617                     = "VUID-vkCmdDrawIndexedIndirectCount-None-08617";
        set_line_width_08618                     = "VUID-vkCmdDrawIndexedIndirectCount-None-08618";
        set_depth_bias_08620                     = "VUID-vkCmdDrawIndexedIndirectCount-None-08620";
        set_blend_constants_08621                = "VUID-vkCmdDrawIndexedIndirectCount-None-08621";
        set_depth_bounds_08622                   = "VUID-vkCmdDrawIndexedIndirectCount-None-08622";
        set_stencil_compare_mask_08623           = "VUID-vkCmdDrawIndexedIndirectCount-None-08623";
        set_stencil_write_mask_08624             = "VUID-vkCmdDrawIndexedIndirectCount-None-08624";
        set_stencil_reference_08625              = "VUID-vkCmdDrawIndexedIndirectCount-None-08625";
        set_sample_locations_08626               = "VUID-vkCmdDrawIndexedIndirectCount-None-08626";
        set_cull_mode_08627                      = "VUID-vkCmdDrawIndexedIndirectCount-None-08627";
        set_front_face_08628                     = "VUID-vkCmdDrawIndexedIndirectCount-None-08628";
        set_depth_test_enable_08629              = "VUID-vkCmdDrawIndexedIndirectCount-None-08629";
        set_depth_write_enable_08630             = "VUID-vkCmdDrawIndexedIndirectCount-None-08630";
        set_depth_comapre_op_08631               = "VUID-vkCmdDrawIndexedIndirectCount-None-08631";
        set_depth_bounds_test_enable_08632       = "VUID-vkCmdDrawIndexedIndirectCount-None-08632";
        set_stencil_test_enable_08633            = "VUID-vkCmdDrawIndexedIndirectCount-None-08633";
        set_stencil_op_08634                     = "VUID-vkCmdDrawIndexedIndirectCount-None-08634";
        set_line_width_08619                     = "VUID-vkCmdDrawIndexedIndirectCount-None-08619";
        set_viewport_with_count_08642            = "VUID-vkCmdDrawIndexedIndirectCount-primitiveFragmentShadingRateWithMultipleViewports-08642";
        alpha_component_word_08920               = "VUID-vkCmdDrawIndexedIndirectCount-alphaToCoverageEnable-08920";
        color_write_mask_09116                   = "VUID-vkCmdDrawIndexedIndirectCount-None-09116";
        vertex_input_format_08936                = "VUID-vkCmdDrawIndexedIndirectCount-format-08936";
        vertex_input_format_08937                = "VUID-vkCmdDrawIndexedIndirectCount-format-08937";
        vertex_input_format_09203                = "VUID-vkCmdDrawIndexedIndirectCount-None-09203";
        vertex_input_format_07939                = "VUID-vkCmdDrawIndexedIndirectCount-Input-07939";
        set_clip_space_w_scaling_09232           = "VUID-vkCmdDrawIndexedIndirectCount-None-09232";
        set_discard_rectangle_09236              = "VUID-vkCmdDrawIndexedIndirectCount-rasterizerDiscardEnable-09236";
        set_viewport_coarse_sample_order_09233   = "VUID-vkCmdDrawIndexedIndirectCount-shadingRateImage-09233";
        set_viewport_shading_rate_palette_09234  = "VUID-vkCmdDrawIndexedIndirectCount-shadingRateImage-09234";
        set_exclusive_scissor_enable_09235       = "VUID-vkCmdDrawIndexedIndirectCount-exclusiveScissor-09235";
        set_fragment_shading_rate_09238          = "VUID-vkCmdDrawIndexedIndirectCount-pipelineFragmentShadingRate-09238";
        set_tessellation_domain_origin_09237     = "VUID-vkCmdDrawIndexedIndirectCount-None-09237";
        rasterization_samples_07935              = "VUID-vkCmdDrawIndexedIndirectCount-pNext-07935";
        mesh_shader_queries_07073                = "VUID-vkCmdDrawIndexedIndirectCount-stage-07073";
        blend_advanced_07480                     = "VUID-vkCmdDrawIndexedIndirectCount-advancedBlendMaxColorAttachments-07480";
        blend_feature_07470                      = "VUID-vkCmdDrawIndexedIndirectCount-pColorBlendEnables-07470";
        color_attachment_08963                   = "VUID-vkCmdDrawIndexedIndirectCount-pColorAttachments-08963";
        depth_attachment_08964                   = "VUID-vkCmdDrawIndexedIndirectCount-pDepthAttachment-08964";
        stencil_attachment_08965                 = "VUID-vkCmdDrawIndexedIndirectCount-pStencilAttachment-08965";
        sample_locations_07482                   = "VUID-vkCmdDrawIndexedIndirectCount-sampleLocationsPerPixel-07482";
    }
};

struct DispatchVuidsCmdTraceRaysNV: DrawDispatchVuid {
    DispatchVuidsCmdTraceRaysNV() : DrawDispatchVuid() {
        pipeline_bound_08606                     = "VUID-vkCmdTraceRaysNV-None-08606";
        pipeline_or_shaders_bound_08607          = "VUID-vkCmdTraceRaysNV-None-08607";
        compatible_pipeline_08600                = "VUID-vkCmdTraceRaysNV-None-08600";
        linear_filter_sampler_04553              = "VUID-vkCmdTraceRaysNV-magFilter-04553";
        linear_mipmap_sampler_04770              = "VUID-vkCmdTraceRaysNV-mipmapMode-04770";
        cubic_sampler_02692                      = "VUID-vkCmdTraceRaysNV-None-02692";
        corner_sampled_address_mode_02696        = "VUID-vkCmdTraceRaysNV-flags-02696";
        imageview_atomic_02691                   = "VUID-vkCmdTraceRaysNV-None-02691";
        bufferview_atomic_07888                  = "VUID-vkCmdTraceRaysNV-None-07888";
        push_constants_set_08602                 = "VUID-vkCmdTraceRaysNV-maintenance4-08602";
        sampler_imageview_type_08609             = "VUID-vkCmdTraceRaysNV-None-08609";
        sampler_implicitLod_dref_proj_08610      = "VUID-vkCmdTraceRaysNV-None-08610";
        sampler_bias_offset_08611                = "VUID-vkCmdTraceRaysNV-None-08611";
        dynamic_state_setting_commands_08608     = "VUID-vkCmdTraceRaysNV-None-08608";
        unprotected_command_buffer_02707         = "VUID-vkCmdTraceRaysNV-commandBuffer-02707";
        ray_query_protected_cb_03635             = "VUID-vkCmdTraceRaysNV-commandBuffer-04624";
        img_filter_cubic_02693                   = "VUID-vkCmdTraceRaysNV-None-02693";
        filter_cubic_02694                       = "VUID-vkCmdTraceRaysNV-filterCubic-02694";
        filter_cubic_min_max_02695               = "VUID-vkCmdTraceRaysNV-filterCubicMinmax-02695";
        image_view_access_64_04470               = "VUID-vkCmdTraceRaysNV-SampledType-04470";
        image_view_access_32_04471               = "VUID-vkCmdTraceRaysNV-SampledType-04471";
        image_view_sparse_64_04474               = "VUID-vkCmdTraceRaysNV-sparseImageInt64Atomics-04474";
        buffer_view_access_64_04472              = "VUID-vkCmdTraceRaysNV-SampledType-04472";
        buffer_view_access_32_04473              = "VUID-vkCmdTraceRaysNV-SampledType-04473";
        storage_image_read_without_format_07028  = "VUID-vkCmdTraceRaysNV-OpTypeImage-07028";
        storage_image_write_without_format_07027 = "VUID-vkCmdTraceRaysNV-OpTypeImage-07027";
        storage_texel_buffer_read_without_format_07030  = "VUID-vkCmdTraceRaysNV-OpTypeImage-07030";
        storage_texel_buffer_write_without_format_07029 = "VUID-vkCmdTraceRaysNV-OpTypeImage-07029";
        storage_image_write_texel_count_08795           = "VUID-vkCmdTraceRaysNV-OpImageWrite-08795";
        storage_image_write_texel_count_08796           = "VUID-vkCmdTraceRaysNV-OpImageWrite-08796";
        storage_texel_buffer_write_texel_count_04469    = "VUID-vkCmdTraceRaysNV-OpImageWrite-04469";
        depth_compare_sample_06479               = "VUID-vkCmdTraceRaysNV-None-06479";
        descriptor_buffer_bit_set_08114          = "VUID-vkCmdTraceRaysNV-None-08114";
        descriptor_buffer_bit_not_set_08115      = "VUID-vkCmdTraceRaysNV-None-08115";
        descriptor_buffer_set_offset_missing_08117 = "VUID-vkCmdTraceRaysNV-None-08117";
        image_view_dim_07752                     = "VUID-vkCmdTraceRaysNV-viewType-07752";
        image_view_numeric_format_07753          = "VUID-vkCmdTraceRaysNV-format-07753";
    }
};

struct DispatchVuidsCmdTraceRaysKHR: DrawDispatchVuid {
    DispatchVuidsCmdTraceRaysKHR() : DrawDispatchVuid() {
        pipeline_bound_08606                     = "VUID-vkCmdTraceRaysKHR-None-08606";
        pipeline_or_shaders_bound_08607          = "VUID-vkCmdTraceRaysKHR-None-08607";
        compatible_pipeline_08600                = "VUID-vkCmdTraceRaysKHR-None-08600";
        linear_filter_sampler_04553              = "VUID-vkCmdTraceRaysKHR-magFilter-04553";
        linear_mipmap_sampler_04770              = "VUID-vkCmdTraceRaysKHR-mipmapMode-04770";
        cubic_sampler_02692                      = "VUID-vkCmdTraceRaysKHR-None-02692";
        corner_sampled_address_mode_02696        = "VUID-vkCmdTraceRaysKHR-flags-02696";
        imageview_atomic_02691                   = "VUID-vkCmdTraceRaysKHR-None-02691";
        bufferview_atomic_07888                  = "VUID-vkCmdTraceRaysKHR-None-07888";
        push_constants_set_08602                 = "VUID-vkCmdTraceRaysKHR-maintenance4-08602";
        sampler_imageview_type_08609             = "VUID-vkCmdTraceRaysKHR-None-08609";
        sampler_implicitLod_dref_proj_08610      = "VUID-vkCmdTraceRaysKHR-None-08610";
        sampler_bias_offset_08611                = "VUID-vkCmdTraceRaysKHR-None-08611";
        dynamic_state_setting_commands_08608     = "VUID-vkCmdTraceRaysKHR-None-08608";
        unprotected_command_buffer_02707         = "VUID-vkCmdTraceRaysKHR-commandBuffer-02707";
        ray_query_protected_cb_03635             = "VUID-vkCmdTraceRaysKHR-commandBuffer-03635";
        img_filter_cubic_02693                   = "VUID-vkCmdTraceRaysKHR-None-02693";
        filter_cubic_02694                       = "VUID-vkCmdTraceRaysKHR-filterCubic-02694";
        filter_cubic_min_max_02695               = "VUID-vkCmdTraceRaysKHR-filterCubicMinmax-02695";
        image_view_access_64_04470               = "VUID-vkCmdTraceRaysKHR-SampledType-04470";
        image_view_access_32_04471               = "VUID-vkCmdTraceRaysKHR-SampledType-04471";
        image_view_sparse_64_04474               = "VUID-vkCmdTraceRaysKHR-sparseImageInt64Atomics-04474";
        buffer_view_access_64_04472              = "VUID-vkCmdTraceRaysKHR-SampledType-04472";
        buffer_view_access_32_04473              = "VUID-vkCmdTraceRaysKHR-SampledType-04473";
        storage_image_read_without_format_07028  = "VUID-vkCmdTraceRaysKHR-OpTypeImage-07028";
        storage_image_write_without_format_07027 = "VUID-vkCmdTraceRaysKHR-OpTypeImage-07027";
        storage_texel_buffer_read_without_format_07030  = "VUID-vkCmdTraceRaysKHR-OpTypeImage-07030";
        storage_texel_buffer_write_without_format_07029 = "VUID-vkCmdTraceRaysKHR-OpTypeImage-07029";
        storage_image_write_texel_count_08795           = "VUID-vkCmdTraceRaysKHR-OpImageWrite-08795";
        storage_image_write_texel_count_08796           = "VUID-vkCmdTraceRaysKHR-OpImageWrite-08796";
        storage_texel_buffer_write_texel_count_04469    = "VUID-vkCmdTraceRaysKHR-OpImageWrite-04469";
        depth_compare_sample_06479               = "VUID-vkCmdTraceRaysKHR-None-06479";
        descriptor_buffer_bit_set_08114          = "VUID-vkCmdTraceRaysKHR-None-08114";
        descriptor_buffer_bit_not_set_08115      = "VUID-vkCmdTraceRaysKHR-None-08115";
        descriptor_buffer_set_offset_missing_08117 = "VUID-vkCmdTraceRaysKHR-None-08117";
        image_view_dim_07752                     = "VUID-vkCmdTraceRaysKHR-viewType-07752";
        image_view_numeric_format_07753          = "VUID-vkCmdTraceRaysKHR-format-07753";
    }
};

struct DispatchVuidsCmdTraceRaysIndirectKHR: DrawDispatchVuid {
    DispatchVuidsCmdTraceRaysIndirectKHR() : DrawDispatchVuid() {
        pipeline_bound_08606                     = "VUID-vkCmdTraceRaysIndirectKHR-None-08606";
        pipeline_or_shaders_bound_08607          = "VUID-vkCmdTraceRaysIndirectKHR-None-08607";
        compatible_pipeline_08600                = "VUID-vkCmdTraceRaysIndirectKHR-None-08600";
        linear_filter_sampler_04553              = "VUID-vkCmdTraceRaysIndirectKHR-magFilter-04553";
        linear_mipmap_sampler_04770              = "VUID-vkCmdTraceRaysIndirectKHR-mipmapMode-04770";
        cubic_sampler_02692                      = "VUID-vkCmdTraceRaysIndirectKHR-None-02692";
        indirect_contiguous_memory_02708         = "VUID-vkCmdTraceRaysIndirectKHR-indirectDeviceAddress-03632";
        indirect_buffer_bit_02290                = "VUID-vkCmdTraceRaysIndirectKHR-indirectDeviceAddress-03633";
        corner_sampled_address_mode_02696        = "VUID-vkCmdTraceRaysIndirectKHR-flags-02696";
        imageview_atomic_02691                   = "VUID-vkCmdTraceRaysIndirectKHR-None-02691";
        bufferview_atomic_07888                  = "VUID-vkCmdTraceRaysIndirectKHR-None-07888";
        push_constants_set_08602                 = "VUID-vkCmdTraceRaysIndirectKHR-maintenance4-08602";
        sampler_imageview_type_08609             = "VUID-vkCmdTraceRaysIndirectKHR-None-08609";
        sampler_implicitLod_dref_proj_08610      = "VUID-vkCmdTraceRaysIndirectKHR-None-08610";
        sampler_bias_offset_08611                = "VUID-vkCmdTraceRaysIndirectKHR-None-08611";
        dynamic_state_setting_commands_08608     = "VUID-vkCmdTraceRaysIndirectKHR-None-08608";
        unprotected_command_buffer_02707         = "VUID-vkCmdTraceRaysIndirectKHR-commandBuffer-02707";
        ray_query_protected_cb_03635             = "VUID-vkCmdTraceRaysIndirectKHR-commandBuffer-03635";
        img_filter_cubic_02693                   = "VUID-vkCmdTraceRaysIndirectKHR-None-02693";
        filter_cubic_02694                       = "VUID-vkCmdTraceRaysIndirectKHR-filterCubic-02694";
        filter_cubic_min_max_02695               = "VUID-vkCmdTraceRaysIndirectKHR-filterCubicMinmax-02695";
        image_view_access_64_04470               = "VUID-vkCmdTraceRaysIndirectKHR-SampledType-04470";
        image_view_access_32_04471               = "VUID-vkCmdTraceRaysIndirectKHR-SampledType-04471";
        image_view_sparse_64_04474               = "VUID-vkCmdTraceRaysIndirectKHR-sparseImageInt64Atomics-04474";
        buffer_view_access_64_04472              = "VUID-vkCmdTraceRaysIndirectKHR-SampledType-04472";
        buffer_view_access_32_04473              = "VUID-vkCmdTraceRaysIndirectKHR-SampledType-04473";
        storage_image_read_without_format_07028  = "VUID-vkCmdTraceRaysIndirectKHR-OpTypeImage-07028";
        storage_image_write_without_format_07027 = "VUID-vkCmdTraceRaysIndirectKHR-OpTypeImage-07027";
        storage_texel_buffer_read_without_format_07030  = "VUID-vkCmdTraceRaysIndirectKHR-OpTypeImage-07030";
        storage_texel_buffer_write_without_format_07029 = "VUID-vkCmdTraceRaysIndirectKHR-OpTypeImage-07029";
        storage_image_write_texel_count_08795           = "VUID-vkCmdTraceRaysIndirectKHR-OpImageWrite-08795";
        storage_image_write_texel_count_08796           = "VUID-vkCmdTraceRaysIndirectKHR-OpImageWrite-08796";
        storage_texel_buffer_write_texel_count_04469    = "VUID-vkCmdTraceRaysIndirectKHR-OpImageWrite-04469";
        depth_compare_sample_06479               = "VUID-vkCmdTraceRaysIndirectKHR-None-06479";
        descriptor_buffer_bit_set_08114          = "VUID-vkCmdTraceRaysIndirectKHR-None-08114";
        descriptor_buffer_bit_not_set_08115      = "VUID-vkCmdTraceRaysIndirectKHR-None-08115";
        descriptor_buffer_set_offset_missing_08117 = "VUID-vkCmdTraceRaysIndirectKHR-None-08117";
        image_view_dim_07752                     = "VUID-vkCmdTraceRaysIndirectKHR-viewType-07752";
        image_view_numeric_format_07753          = "VUID-vkCmdTraceRaysIndirectKHR-format-07753";
    }
};

struct DispatchVuidsCmdTraceRaysIndirect2KHR: DrawDispatchVuid {
    DispatchVuidsCmdTraceRaysIndirect2KHR() : DrawDispatchVuid() {
        pipeline_bound_08606                     = "VUID-vkCmdTraceRaysIndirect2KHR-None-08606";
        pipeline_or_shaders_bound_08607          = "VUID-vkCmdTraceRaysIndirect2KHR-None-08607";
        compatible_pipeline_08600                = "VUID-vkCmdTraceRaysIndirect2KHR-None-08600";
        linear_filter_sampler_04553              = "VUID-vkCmdTraceRaysIndirect2KHR-magFilter-04553";
        linear_mipmap_sampler_04770              = "VUID-vkCmdTraceRaysIndirect2KHR-mipmapMode-04770";
        cubic_sampler_02692                      = "VUID-vkCmdTraceRaysIndirect2KHR-None-02692";
        indirect_contiguous_memory_02708         = "VUID-vkCmdTraceRaysIndirect2KHR-indirectDeviceAddress-03632";
        indirect_buffer_bit_02290                = "VUID-vkCmdTraceRaysIndirect2KHR-indirectDeviceAddress-03633";
        corner_sampled_address_mode_02696        = "VUID-vkCmdTraceRaysIndirect2KHR-flags-02696";
        imageview_atomic_02691                   = "VUID-vkCmdTraceRaysIndirect2KHR-None-02691";
        bufferview_atomic_07888                  = "VUID-vkCmdTraceRaysIndirect2KHR-None-07888";
        push_constants_set_08602                 = "VUID-vkCmdTraceRaysIndirect2KHR-maintenance4-08602";
        sampler_imageview_type_08609             = "VUID-vkCmdTraceRaysIndirect2KHR-None-08609";
        sampler_implicitLod_dref_proj_08610      = "VUID-vkCmdTraceRaysIndirect2KHR-None-08610";
        sampler_bias_offset_08611                = "VUID-vkCmdTraceRaysIndirect2KHR-None-08611";
        dynamic_state_setting_commands_08608     = "VUID-vkCmdTraceRaysIndirect2KHR-None-08608";
        unprotected_command_buffer_02707         = "VUID-vkCmdTraceRaysIndirect2KHR-commandBuffer-02707";
        ray_query_protected_cb_03635             = "VUID-vkCmdTraceRaysIndirect2KHR-commandBuffer-03635";
        img_filter_cubic_02693                   = "VUID-vkCmdTraceRaysIndirect2KHR-None-02693";
        filter_cubic_02694                       = "VUID-vkCmdTraceRaysIndirect2KHR-filterCubic-02694";
        filter_cubic_min_max_02695               = "VUID-vkCmdTraceRaysIndirect2KHR-filterCubicMinmax-02695";
        image_view_access_64_04470               = "VUID-vkCmdTraceRaysIndirect2KHR-SampledType-04470";
        image_view_access_32_04471               = "VUID-vkCmdTraceRaysIndirect2KHR-SampledType-04471";
        image_view_sparse_64_04474               = "VUID-vkCmdTraceRaysIndirect2KHR-sparseImageInt64Atomics-04474";
        buffer_view_access_64_04472              = "VUID-vkCmdTraceRaysIndirect2KHR-SampledType-04472";
        buffer_view_access_32_04473              = "VUID-vkCmdTraceRaysIndirect2KHR-SampledType-04473";
        storage_image_read_without_format_07028  = "VUID-vkCmdTraceRaysIndirect2KHR-OpTypeImage-07028";
        storage_image_write_without_format_07027 = "VUID-vkCmdTraceRaysIndirect2KHR-OpTypeImage-07027";
        storage_texel_buffer_read_without_format_07030  = "VUID-vkCmdTraceRaysIndirect2KHR-OpTypeImage-07030";
        storage_texel_buffer_write_without_format_07029 = "VUID-vkCmdTraceRaysIndirect2KHR-OpTypeImage-07029";
        storage_image_write_texel_count_08795           = "VUID-vkCmdTraceRaysIndirect2KHR-OpImageWrite-08795";
        storage_image_write_texel_count_08796           = "VUID-vkCmdTraceRaysIndirect2KHR-OpImageWrite-08796";
        storage_texel_buffer_write_texel_count_04469    = "VUID-vkCmdTraceRaysIndirect2KHR-OpImageWrite-04469";
        depth_compare_sample_06479               = "VUID-vkCmdTraceRaysIndirect2KHR-None-06479";
        descriptor_buffer_bit_set_08114          = "VUID-vkCmdTraceRaysIndirect2KHR-None-08114";
        descriptor_buffer_bit_not_set_08115      = "VUID-vkCmdTraceRaysIndirect2KHR-None-08115";
        descriptor_buffer_set_offset_missing_08117 = "VUID-vkCmdTraceRaysIndirect2KHR-None-08117";
        image_view_dim_07752                     = "VUID-vkCmdTraceRaysIndirect2KHR-viewType-07752";
        image_view_numeric_format_07753          = "VUID-vkCmdTraceRaysIndirect2KHR-format-07753";
    }
};

struct DispatchVuidsCmdDrawMeshTasksNV: DrawDispatchVuid {
    DispatchVuidsCmdDrawMeshTasksNV() : DrawDispatchVuid() {
        pipeline_bound_08606                     = "VUID-vkCmdDrawMeshTasksNV-None-08606";
        pipeline_or_shaders_bound_08607          = "VUID-vkCmdDrawMeshTasksNV-None-08607";
        compatible_pipeline_08600                = "VUID-vkCmdDrawMeshTasksNV-None-08600";
        render_pass_compatible_02684             = "VUID-vkCmdDrawMeshTasksNV-renderPass-02684";
        render_pass_began_08876                  = "VUID-vkCmdDrawMeshTasksNV-None-08876";
        subpass_index_02685                      = "VUID-vkCmdDrawMeshTasksNV-subpass-02685";
        sample_location_02689                    = "VUID-vkCmdDrawMeshTasksNV-sampleLocationsEnable-02689";
        linear_filter_sampler_04553              = "VUID-vkCmdDrawMeshTasksNV-magFilter-04553";
        linear_mipmap_sampler_04770              = "VUID-vkCmdDrawMeshTasksNV-mipmapMode-04770";
        cubic_sampler_02692                      = "VUID-vkCmdDrawMeshTasksNV-None-02692";
        viewport_count_03417                     = "VUID-vkCmdDrawMeshTasksNV-viewportCount-03417";
        scissor_count_03418                      = "VUID-vkCmdDrawMeshTasksNV-scissorCount-03418";
        viewport_scissor_count_03419             = "VUID-vkCmdDrawMeshTasksNV-viewportCount-03419";
        corner_sampled_address_mode_02696        = "VUID-vkCmdDrawMeshTasksNV-flags-02696";
        imageview_atomic_02691                   = "VUID-vkCmdDrawMeshTasksNV-None-02691";
        bufferview_atomic_07888                  = "VUID-vkCmdDrawMeshTasksNV-None-07888";
        push_constants_set_08602                 = "VUID-vkCmdDrawMeshTasksNV-maintenance4-08602";
        image_subresources_render_pass_write_06537 = "VUID-vkCmdDrawMeshTasksNV-None-06537";
        image_subresources_subpass_read_09003    = "VUID-vkCmdDrawMeshTasksNV-None-09003";
        image_subresources_subpass_write_06539   = "VUID-vkCmdDrawMeshTasksNV-None-06539";
        sampler_imageview_type_08609             = "VUID-vkCmdDrawMeshTasksNV-None-08609";
        sampler_implicitLod_dref_proj_08610      = "VUID-vkCmdDrawMeshTasksNV-None-08610";
        sampler_bias_offset_08611                = "VUID-vkCmdDrawMeshTasksNV-None-08611";
        dynamic_state_setting_commands_08608     = "VUID-vkCmdDrawMeshTasksNV-None-08608";
        msrtss_rasterization_samples_07284       = "VUID-vkCmdDrawMeshTasksNV-multisampledRenderToSingleSampled-07284";
        unprotected_command_buffer_02707         = "VUID-vkCmdDrawMeshTasksNV-commandBuffer-02707";
        max_multiview_instance_index_02688       = "VUID-vkCmdDrawMeshTasksNV-maxMultiviewInstanceIndex-02688";
        img_filter_cubic_02693                   = "VUID-vkCmdDrawMeshTasksNV-None-02693";
        filter_cubic_02694                       = "VUID-vkCmdDrawMeshTasksNV-filterCubic-02694";
        filter_cubic_min_max_02695               = "VUID-vkCmdDrawMeshTasksNV-filterCubicMinmax-02695";
        viewport_count_primitive_shading_rate_04552 = "VUID-vkCmdDrawMeshTasksNV-primitiveFragmentShadingRateWithMultipleViewports-04552";
        rasterizer_discard_enable_04876          = "VUID-vkCmdDrawMeshTasksNV-None-04876";
        depth_bias_enable_04877                  = "VUID-vkCmdDrawMeshTasksNV-None-04877";
        logic_op_04878                           = "VUID-vkCmdDrawMeshTasksNV-logicOp-04878";
        blend_enable_04727                       = "VUID-vkCmdDrawMeshTasksNV-blendEnable-04727";
        dynamic_discard_rectangle_07751          = "VUID-vkCmdDrawMeshTasksNV-None-07751";
        dynamic_discard_rectangle_enable_07880   = "VUID-vkCmdDrawMeshTasksNV-None-07880";
        dynamic_discard_rectangle_mode_07881     =  "VUID-vkCmdDrawMeshTasksNV-None-07881";
        dynamic_exclusive_scissor_enable_07878   = "VUID-vkCmdDrawMeshTasksNV-None-07878";
        dynamic_exclusive_scissor_07879          = "VUID-vkCmdDrawMeshTasksNV-None-07879";
        dynamic_color_write_enable_07749         = "VUID-vkCmdDrawMeshTasksNV-None-07749";
        dynamic_color_write_enable_count_07750   = "VUID-vkCmdDrawMeshTasksNV-attachmentCount-07750";
        dynamic_attachment_feedback_loop_08877   = "VUID-vkCmdDrawMeshTasksNV-None-08877";
        dynamic_rendering_view_mask_06178        = "VUID-vkCmdDrawMeshTasksNV-viewMask-06178";
        dynamic_rendering_color_count_06179      = "VUID-vkCmdDrawMeshTasksNV-colorAttachmentCount-06179";
        dynamic_rendering_color_formats_08910    = "VUID-vkCmdDrawMeshTasksNV-dynamicRenderingUnusedAttachments-08910";
        dynamic_rendering_unused_attachments_08911 = "VUID-vkCmdDrawMeshTasksNV-dynamicRenderingUnusedAttachments-08911";
        dynamic_rendering_undefined_color_formats_08912 = "VUID-vkCmdDrawMeshTasksNV-dynamicRenderingUnusedAttachments-08912";
        dynamic_rendering_undefined_depth_format_08916 = "VUID-vkCmdDrawMeshTasksNV-dynamicRenderingUnusedAttachments-08916";
        dynamic_rendering_undefined_stencil_format_08916 = "VUID-vkCmdDrawMeshTasksNV-dynamicRenderingUnusedAttachments-08916";
        dynamic_rendering_depth_format_08914     = "VUID-vkCmdDrawMeshTasksNV-dynamicRenderingUnusedAttachments-08914";
        dynamic_rendering_unused_attachments_08915 = "VUID-vkCmdDrawMeshTasksNV-dynamicRenderingUnusedAttachments-08915";
        dynamic_rendering_stencil_format_08917   = "VUID-vkCmdDrawMeshTasksNV-dynamicRenderingUnusedAttachments-08917";
        dynamic_rendering_unused_attachments_08918 = "VUID-vkCmdDrawMeshTasksNV-dynamicRenderingUnusedAttachments-08918";
        dynamic_rendering_fsr_06183              = "VUID-vkCmdDrawMeshTasksNV-imageView-06183";
        dynamic_rendering_fdm_06184              = "VUID-vkCmdDrawMeshTasksNV-imageView-06184";
        dynamic_rendering_color_sample_06185     = "VUID-vkCmdDrawMeshTasksNV-colorAttachmentCount-06185";
        dynamic_rendering_depth_sample_06186     = "VUID-vkCmdDrawMeshTasksNV-pDepthAttachment-06186";
        dynamic_rendering_stencil_sample_06187   = "VUID-vkCmdDrawMeshTasksNV-pStencilAttachment-06187";
        dynamic_rendering_06198                  = "VUID-vkCmdDrawMeshTasksNV-renderPass-06198";
        dynamic_rendering_07285                  = "VUID-vkCmdDrawMeshTasksNV-multisampledRenderToSingleSampled-07285";
        dynamic_rendering_07286                  = "VUID-vkCmdDrawMeshTasksNV-multisampledRenderToSingleSampled-07286";
        dynamic_rendering_07287                  = "VUID-vkCmdDrawMeshTasksNV-multisampledRenderToSingleSampled-07287";
        image_view_access_64_04470               = "VUID-vkCmdDrawMeshTasksNV-SampledType-04470";
        image_view_access_32_04471               = "VUID-vkCmdDrawMeshTasksNV-SampledType-04471";
        image_view_sparse_64_04474               = "VUID-vkCmdDrawMeshTasksNV-sparseImageInt64Atomics-04474";
        buffer_view_access_64_04472              = "VUID-vkCmdDrawMeshTasksNV-SampledType-04472";
        buffer_view_access_32_04473              = "VUID-vkCmdDrawMeshTasksNV-SampledType-04473";
        storage_image_read_without_format_07028  = "VUID-vkCmdDrawMeshTasksNV-OpTypeImage-07028";
        storage_image_write_without_format_07027 = "VUID-vkCmdDrawMeshTasksNV-OpTypeImage-07027";
        storage_texel_buffer_read_without_format_07030  = "VUID-vkCmdDrawMeshTasksNV-OpTypeImage-07030";
        storage_texel_buffer_write_without_format_07029 = "VUID-vkCmdDrawMeshTasksNV-OpTypeImage-07029";
        storage_image_write_texel_count_08795           = "VUID-vkCmdDrawMeshTasksNV-OpImageWrite-08795";
        storage_image_write_texel_count_08796           = "VUID-vkCmdDrawMeshTasksNV-OpImageWrite-08796";
        storage_texel_buffer_write_texel_count_04469    = "VUID-vkCmdDrawMeshTasksNV-OpImageWrite-04469";
        depth_compare_sample_06479               = "VUID-vkCmdDrawMeshTasksNV-None-06479";
        depth_read_only_06886                    = "VUID-vkCmdDrawMeshTasksNV-None-06886";
        stencil_read_only_06887                  = "VUID-vkCmdDrawMeshTasksNV-None-06887";
        dynamic_sample_locations_06666           = "VUID-vkCmdDrawMeshTasksNV-None-06666";
        dynamic_tessellation_domain_origin_07619 = "VUID-vkCmdDrawMeshTasksNV-None-07619";
        dynamic_depth_clamp_enable_07620         = "VUID-vkCmdDrawMeshTasksNV-None-07620";
        dynamic_polygon_mode_07621               = "VUID-vkCmdDrawMeshTasksNV-None-07621";
        dynamic_rasterization_samples_07622      = "VUID-vkCmdDrawMeshTasksNV-None-07622";
        dynamic_sample_mask_07623                = "VUID-vkCmdDrawMeshTasksNV-None-07623";
        dynamic_alpha_to_coverage_enable_07624   = "VUID-vkCmdDrawMeshTasksNV-None-07624";
        dynamic_alpha_to_coverage_component_08919 = "VUID-vkCmdDrawMeshTasksNV-alphaToCoverageEnable-08919";
        dynamic_alpha_to_one_enable_07625        = "VUID-vkCmdDrawMeshTasksNV-None-07625";
        dynamic_logic_op_enable_07626            = "VUID-vkCmdDrawMeshTasksNV-None-07626";
        dynamic_color_blend_enable_07476         = "VUID-vkCmdDrawMeshTasksNV-firstAttachment-07476";
        dynamic_color_blend_equation_07477       = "VUID-vkCmdDrawMeshTasksNV-firstAttachment-07477";
        dynamic_color_write_mask_07478           = "VUID-vkCmdDrawMeshTasksNV-firstAttachment-07478";
        dynamic_rasterization_stream_07630       = "VUID-vkCmdDrawMeshTasksNV-None-07630";
        dynamic_conservative_rasterization_mode_07631 = "VUID-vkCmdDrawMeshTasksNV-None-07631";
        dynamic_extra_primitive_overestimation_size_07632 = "VUID-vkCmdDrawMeshTasksNV-None-07632";
        dynamic_depth_clip_enable_07633          = "VUID-vkCmdDrawMeshTasksNV-None-07633";
        dynamic_sample_locations_enable_07634    = "VUID-vkCmdDrawMeshTasksNV-None-07634";
        dynamic_color_blend_advanced_07479       = "VUID-vkCmdDrawMeshTasksNV-firstAttachment-07479";
        dynamic_provoking_vertex_mode_07636      = "VUID-vkCmdDrawMeshTasksNV-None-07636";
        dynamic_line_rasterization_mode_07637    = "VUID-vkCmdDrawMeshTasksNV-None-07637";
        dynamic_line_stipple_enable_07638        = "VUID-vkCmdDrawMeshTasksNV-None-07638";
        dynamic_depth_clip_negative_one_to_one_07639 = "VUID-vkCmdDrawMeshTasksNV-None-07639";
        dynamic_viewport_w_scaling_enable_07640  = "VUID-vkCmdDrawMeshTasksNV-None-07640";
        dynamic_viewport_swizzle_07641           = "VUID-vkCmdDrawMeshTasksNV-None-07641";
        dynamic_coverage_to_color_enable_07642   = "VUID-vkCmdDrawMeshTasksNV-None-07642";
        dynamic_coverage_to_color_location_07643 = "VUID-vkCmdDrawMeshTasksNV-None-07643";
        dynamic_coverage_modulation_mode_07644   = "VUID-vkCmdDrawMeshTasksNV-None-07644";
        dynamic_coverage_modulation_table_enable_07645 = "VUID-vkCmdDrawMeshTasksNV-None-07645";
        dynamic_coverage_modulation_table_07646  = "VUID-vkCmdDrawMeshTasksNV-None-07646";
        dynamic_shading_rate_image_enable_07647  = "VUID-vkCmdDrawMeshTasksNV-None-07647";
        dynamic_representative_fragment_test_enable_07648  = "VUID-vkCmdDrawMeshTasksNV-None-07648";
        dynamic_coverage_reduction_mode_07649    = "VUID-vkCmdDrawMeshTasksNV-None-07649";
        dynamic_viewport_07831                   = "VUID-vkCmdDrawMeshTasksNV-None-07831";
        dynamic_scissor_07832                    = "VUID-vkCmdDrawMeshTasksNV-None-07832";
        dynamic_depth_bias_07834                 = "VUID-vkCmdDrawMeshTasksNV-None-07834";
        dynamic_line_width_07833                 = "VUID-vkCmdDrawMeshTasksNV-None-07833";
        dynamic_line_stipple_ext_07849           = "VUID-vkCmdDrawMeshTasksNV-None-07849";
        dynamic_blend_constants_07835            = "VUID-vkCmdDrawMeshTasksNV-None-07835";
        dynamic_depth_bounds_07836               = "VUID-vkCmdDrawMeshTasksNV-None-07836";
        dynamic_depth_enable_08715               = "VUID-vkCmdDrawMeshTasksNV-pDynamicStates-08715";
        dynamic_stencil_compare_mask_07837       = "VUID-vkCmdDrawMeshTasksNV-None-07837";
        dynamic_stencil_write_mask_07838         = "VUID-vkCmdDrawMeshTasksNV-None-07838";
        dynamic_stencil_write_mask_08716         = "VUID-vkCmdDrawMeshTasksNV-pDynamicStates-08716";
        dynamic_stencil_reference_07839          = "VUID-vkCmdDrawMeshTasksNV-None-07839";
        dynamic_state_inherited_07850            = "VUID-vkCmdDrawMeshTasksNV-None-07850";
        dynamic_cull_mode_07840                  = "VUID-vkCmdDrawMeshTasksNV-None-07840";
        dynamic_front_face_07841                 = "VUID-vkCmdDrawMeshTasksNV-None-07841";
        dynamic_depth_test_enable_07843          = "VUID-vkCmdDrawMeshTasksNV-None-07843";
        dynamic_depth_write_enable_07844         = "VUID-vkCmdDrawMeshTasksNV-None-07844";
        dynamic_depth_compare_op_07845           = "VUID-vkCmdDrawMeshTasksNV-None-07845";
        dynamic_depth_bound_test_enable_07846    = "VUID-vkCmdDrawMeshTasksNV-None-07846";
        dynamic_stencil_test_enable_07847        = "VUID-vkCmdDrawMeshTasksNV-None-07847";
        dynamic_stencil_op_07848                 = "VUID-vkCmdDrawMeshTasksNV-None-07848";
        primitives_generated_06708               = "VUID-vkCmdDrawMeshTasksNV-primitivesGeneratedQueryWithRasterizerDiscard-06708";
        primitives_generated_streams_06709       = "VUID-vkCmdDrawMeshTasksNV-primitivesGeneratedQueryWithNonZeroStreams-06709";
        mesh_shader_stages_06480                 = "VUID-vkCmdDrawMeshTasksNV-stage-06480";
        missing_mesh_shader_stages_07080         = "VUID-vkCmdDrawMeshTasksNV-MeshNV-07080";
        descriptor_buffer_bit_set_08114          = "VUID-vkCmdDrawMeshTasksNV-None-08114";
        descriptor_buffer_bit_not_set_08115      = "VUID-vkCmdDrawMeshTasksNV-None-08115";
        descriptor_buffer_set_offset_missing_08117 = "VUID-vkCmdDrawMeshTasksNV-None-08117";
        image_view_dim_07752                     = "VUID-vkCmdDrawMeshTasksNV-viewType-07752";
        image_view_numeric_format_07753          = "VUID-vkCmdDrawMeshTasksNV-format-07753";
        stippled_rectangular_lines_07495         = "VUID-vkCmdDrawMeshTasksNV-stippledLineEnable-07495";
        stippled_bresenham_lines_07496           = "VUID-vkCmdDrawMeshTasksNV-stippledLineEnable-07496";
        stippled_smooth_lines_07497              = "VUID-vkCmdDrawMeshTasksNV-stippledLineEnable-07497";
        stippled_default_strict_07498            = "VUID-vkCmdDrawMeshTasksNV-stippledLineEnable-07498";
        viewport_and_scissor_with_count_08635    = "VUID-vkCmdDrawMeshTasksNV-None-08635";
        viewport_w_scaling_08636                 = "VUID-vkCmdDrawMeshTasksNV-None-08636";
        shading_rate_palette_08637               = "VUID-vkCmdDrawMeshTasksNV-None-08637";
        exclusive_scissor_08638                  = "VUID-vkCmdDrawMeshTasksNV-None-08638";
        external_format_resolve_09362            = "VUID-vkCmdDrawMeshTasksNV-colorAttachmentCount-09362";
        external_format_resolve_09363            = "VUID-vkCmdDrawMeshTasksNV-None-09363";
        external_format_resolve_09364            = "VUID-vkCmdDrawMeshTasksNV-None-09364";
        external_format_resolve_09365            = "VUID-vkCmdDrawMeshTasksNV-None-09365";
        external_format_resolve_09368            = "VUID-vkCmdDrawMeshTasksNV-None-09368";
        external_format_resolve_09369            = "VUID-vkCmdDrawMeshTasksNV-None-09369";
        external_format_resolve_09372            = "VUID-vkCmdDrawMeshTasksNV-colorAttachmentCount-09372";
        set_rasterizer_discard_enable_08639      = "VUID-vkCmdDrawMeshTasksNV-None-08639";
        set_depth_bias_enable_08640              = "VUID-vkCmdDrawMeshTasksNV-None-08640";
        set_logic_op_08641                       = "VUID-vkCmdDrawMeshTasksNV-None-08641";
        set_color_blend_enable_08643             = "VUID-vkCmdDrawMeshTasksNV-None-08643";
        set_rasterization_samples_08644              = "VUID-vkCmdDrawMeshTasksNV-None-08644";
        set_color_write_enable_08646             = "VUID-vkCmdDrawMeshTasksNV-None-08646";
        set_color_write_enable_08647             = "VUID-vkCmdDrawMeshTasksNV-None-08647";
        set_discard_rectangles_enable_08648      = "VUID-vkCmdDrawMeshTasksNV-None-08648";
        set_discard_rectangles_mode_08649      = "VUID-vkCmdDrawMeshTasksNV-None-08649";
        set_depth_clamp_enable_08650             = "VUID-vkCmdDrawMeshTasksNV-None-08650";
        set_polygon_mode_08651                   = "VUID-vkCmdDrawMeshTasksNV-None-08651";
        set_rasterization_samples_08652          = "VUID-vkCmdDrawMeshTasksNV-None-08652";
        set_sample_mask_08653                    = "VUID-vkCmdDrawMeshTasksNV-None-08653";
        set_alpha_to_coverage_enable_08654       = "VUID-vkCmdDrawMeshTasksNV-None-08654";
        set_alpha_to_one_enable_08655            = "VUID-vkCmdDrawMeshTasksNV-None-08655";
        set_logic_op_enable_08656                = "VUID-vkCmdDrawMeshTasksNV-None-08656";
        set_color_blend_enable_08657             = "VUID-vkCmdDrawMeshTasksNV-None-08657";
        set_color_blend_equation_08658           = "VUID-vkCmdDrawMeshTasksNV-None-08658";
        set_color_write_mask_08659               = "VUID-vkCmdDrawMeshTasksNV-None-08659";
        set_rasterization_streams_08660          = "VUID-vkCmdDrawMeshTasksNV-None-08660";
        set_conservative_rasterization_mode_08661 = "VUID-vkCmdDrawMeshTasksNV-None-08661";
        set_extra_primitive_overestimation_size_08662 = "VUID-vkCmdDrawMeshTasksNV-None-08662";
        set_depth_clip_enable_08663              = "VUID-vkCmdDrawMeshTasksNV-None-08663";
        set_sample_locations_enable_08664        = "VUID-vkCmdDrawMeshTasksNV-None-08664";
        set_provoking_vertex_mode_08665          = "VUID-vkCmdDrawMeshTasksNV-None-08665";
        set_line_rasterization_mode_08666        = "VUID-vkCmdDrawMeshTasksNV-None-08666";
        set_line_rasterization_mode_08667        = "VUID-vkCmdDrawMeshTasksNV-None-08667";
        set_line_rasterization_mode_08668        = "VUID-vkCmdDrawMeshTasksNV-None-08668";
        set_line_stipple_enable_08669            = "VUID-vkCmdDrawMeshTasksNV-None-08669";
        set_line_stipple_enable_08670            = "VUID-vkCmdDrawMeshTasksNV-None-08670";
        set_line_stipple_enable_08671            = "VUID-vkCmdDrawMeshTasksNV-None-08671";
        set_line_stipple_08672                   = "VUID-vkCmdDrawMeshTasksNV-None-08672";
        set_depth_clip_negative_one_to_one_08673 = "VUID-vkCmdDrawMeshTasksNV-None-08673";
        set_viewport_w_scaling_enable_08674      = "VUID-vkCmdDrawMeshTasksNV-None-08674";
        set_viewport_swizzle_08675               = "VUID-vkCmdDrawMeshTasksNV-None-08675";
        set_coverage_to_color_enable_08676       = "VUID-vkCmdDrawMeshTasksNV-None-08676";
        set_coverage_to_color_location_08677     = "VUID-vkCmdDrawMeshTasksNV-None-08677";
        set_coverage_modulation_mode_08678       = "VUID-vkCmdDrawMeshTasksNV-None-08678";
        set_coverage_modulation_table_enable_08679 = "VUID-vkCmdDrawMeshTasksNV-None-08679";
        set_coverage_modulation_table_08680      = "VUID-vkCmdDrawMeshTasksNV-None-08680";
        set_shading_rate_image_enable_08681      = "VUID-vkCmdDrawMeshTasksNV-None-08681";
        set_representative_fragment_test_enable_08682 = "VUID-vkCmdDrawMeshTasksNV-None-08682";
        set_coverage_reduction_mode_08683        = "VUID-vkCmdDrawMeshTasksNV-None-08683";
        vertex_shader_08684                      = "VUID-vkCmdDrawMeshTasksNV-None-08684";
        tessellation_control_shader_08685        = "VUID-vkCmdDrawMeshTasksNV-None-08685";
        tessellation_evaluation_shader_08686     = "VUID-vkCmdDrawMeshTasksNV-None-08686";
        geometry_shader_08687                    = "VUID-vkCmdDrawMeshTasksNV-None-08687";
        fragment_shader_08688                    = "VUID-vkCmdDrawMeshTasksNV-None-08688";
        task_shader_08689                        = "VUID-vkCmdDrawMeshTasksNV-None-08689";
        mesh_shader_08690                        = "VUID-vkCmdDrawMeshTasksNV-None-08690";
        vert_mesh_shader_08693                   = "VUID-vkCmdDrawMeshTasksNV-None-08693";
        task_mesh_shader_08694                   = "VUID-vkCmdDrawMeshTasksNV-None-08694";
        task_mesh_shader_08695                   = "VUID-vkCmdDrawMeshTasksNV-None-08695";
        vert_task_mesh_shader_08696              = "VUID-vkCmdDrawMeshTasksNV-None-08696";
        linked_shaders_08698                     = "VUID-vkCmdDrawMeshTasksNV-None-08698";
        linked_shaders_08699                     = "VUID-vkCmdDrawMeshTasksNV-None-08699";
        shaders_push_constants_08878             = "VUID-vkCmdDrawMeshTasksNV-None-08878";
        shaders_descriptor_layouts_08879         = "VUID-vkCmdDrawMeshTasksNV-None-08879";
        set_attachment_feedback_loop_enable_08880 = "VUID-vkCmdDrawMeshTasksNV-None-08880";
        set_line_width_08617                     = "VUID-vkCmdDrawMeshTasksNV-None-08617";
        set_line_width_08618                     = "VUID-vkCmdDrawMeshTasksNV-None-08618";
        set_depth_bias_08620                     = "VUID-vkCmdDrawMeshTasksNV-None-08620";
        set_blend_constants_08621                = "VUID-vkCmdDrawMeshTasksNV-None-08621";
        set_depth_bounds_08622                   = "VUID-vkCmdDrawMeshTasksNV-None-08622";
        set_stencil_compare_mask_08623           = "VUID-vkCmdDrawMeshTasksNV-None-08623";
        set_stencil_write_mask_08624             = "VUID-vkCmdDrawMeshTasksNV-None-08624";
        set_stencil_reference_08625              = "VUID-vkCmdDrawMeshTasksNV-None-08625";
        set_sample_locations_08626               = "VUID-vkCmdDrawMeshTasksNV-None-08626";
        set_cull_mode_08627                      = "VUID-vkCmdDrawMeshTasksNV-None-08627";
        set_front_face_08628                     = "VUID-vkCmdDrawMeshTasksNV-None-08628";
        set_depth_test_enable_08629              = "VUID-vkCmdDrawMeshTasksNV-None-08629";
        set_depth_write_enable_08630             = "VUID-vkCmdDrawMeshTasksNV-None-08630";
        set_depth_comapre_op_08631               = "VUID-vkCmdDrawMeshTasksNV-None-08631";
        set_depth_bounds_test_enable_08632       = "VUID-vkCmdDrawMeshTasksNV-None-08632";
        set_stencil_test_enable_08633            = "VUID-vkCmdDrawMeshTasksNV-None-08633";
        set_stencil_op_08634                     = "VUID-vkCmdDrawMeshTasksNV-None-08634";
        set_line_width_08619                     = "VUID-vkCmdDrawMeshTasksNV-None-08619";
        set_viewport_with_count_08642            = "VUID-vkCmdDrawMeshTasksNV-primitiveFragmentShadingRateWithMultipleViewports-08642";
        alpha_component_word_08920               = "VUID-vkCmdDrawMeshTasksNV-alphaToCoverageEnable-08920";
        color_write_mask_09116                   = "VUID-vkCmdDrawMeshTasksNV-None-09116";
        set_clip_space_w_scaling_09232           = "VUID-vkCmdDrawMeshTasksNV-None-09232";
        set_discard_rectangle_09236              = "VUID-vkCmdDrawMeshTasksNV-rasterizerDiscardEnable-09236";
        set_viewport_coarse_sample_order_09233   = "VUID-vkCmdDrawMeshTasksNV-shadingRateImage-09233";
        set_viewport_shading_rate_palette_09234  = "VUID-vkCmdDrawMeshTasksNV-shadingRateImage-09234";
        set_exclusive_scissor_enable_09235       = "VUID-vkCmdDrawMeshTasksNV-exclusiveScissor-09235";
        set_fragment_shading_rate_09238          = "VUID-vkCmdDrawMeshTasksNV-pipelineFragmentShadingRate-09238";
        set_tessellation_domain_origin_09237     = "VUID-vkCmdDrawMeshTasksNV-None-09237";
        rasterization_samples_07935              = "VUID-vkCmdDrawMeshTasksNV-pNext-07935";
        mesh_shader_queries_07073                = "VUID-vkCmdDrawMeshTasksNV-stage-07073";
        blend_advanced_07480                     = "VUID-vkCmdDrawMeshTasksNV-advancedBlendMaxColorAttachments-07480";
        blend_feature_07470                      = "VUID-vkCmdDrawMeshTasksNV-pColorBlendEnables-07470";
        color_attachment_08963                   = "VUID-vkCmdDrawMeshTasksNV-pColorAttachments-08963";
        depth_attachment_08964                   = "VUID-vkCmdDrawMeshTasksNV-pDepthAttachment-08964";
        stencil_attachment_08965                 = "VUID-vkCmdDrawMeshTasksNV-pStencilAttachment-08965";
        sample_locations_07482                   = "VUID-vkCmdDrawMeshTasksNV-sampleLocationsPerPixel-07482";
    }
};

struct DispatchVuidsCmdDrawMeshTasksIndirectNV: DrawDispatchVuid {
    DispatchVuidsCmdDrawMeshTasksIndirectNV() : DrawDispatchVuid() {
        pipeline_bound_08606                     = "VUID-vkCmdDrawMeshTasksIndirectNV-None-08606";
        pipeline_or_shaders_bound_08607          = "VUID-vkCmdDrawMeshTasksIndirectNV-None-08607";
        compatible_pipeline_08600                = "VUID-vkCmdDrawMeshTasksIndirectNV-None-08600";
        render_pass_compatible_02684             = "VUID-vkCmdDrawMeshTasksIndirectNV-renderPass-02684";
        render_pass_began_08876                  = "VUID-vkCmdDrawMeshTasksIndirectNV-None-08876";
        subpass_index_02685                      = "VUID-vkCmdDrawMeshTasksIndirectNV-subpass-02685";
        sample_location_02689                    = "VUID-vkCmdDrawMeshTasksIndirectNV-sampleLocationsEnable-02689";
        linear_filter_sampler_04553              = "VUID-vkCmdDrawMeshTasksIndirectNV-magFilter-04553";
        linear_mipmap_sampler_04770              = "VUID-vkCmdDrawMeshTasksIndirectNV-mipmapMode-04770";
        cubic_sampler_02692                      = "VUID-vkCmdDrawMeshTasksIndirectNV-None-02692";
        indirect_protected_cb_02646              = "VUID-vkCmdDrawMeshTasksIndirectNV-commandBuffer-02711";
        indirect_contiguous_memory_02708         = "VUID-vkCmdDrawMeshTasksIndirectNV-buffer-02708";
        indirect_buffer_bit_02290                = "VUID-vkCmdDrawMeshTasksIndirectNV-buffer-02709";
        viewport_count_03417                     = "VUID-vkCmdDrawMeshTasksIndirectNV-viewportCount-03417";
        scissor_count_03418                      = "VUID-vkCmdDrawMeshTasksIndirectNV-scissorCount-03418";
        viewport_scissor_count_03419             = "VUID-vkCmdDrawMeshTasksIndirectNV-viewportCount-03419";
        corner_sampled_address_mode_02696        = "VUID-vkCmdDrawMeshTasksIndirectNV-flags-02696";
        imageview_atomic_02691                   = "VUID-vkCmdDrawMeshTasksIndirectNV-None-02691";
        bufferview_atomic_07888                  = "VUID-vkCmdDrawMeshTasksIndirectNV-None-07888";
        push_constants_set_08602                 = "VUID-vkCmdDrawMeshTasksIndirectNV-maintenance4-08602";
        image_subresources_render_pass_write_06537 = "VUID-vkCmdDrawMeshTasksIndirectNV-None-06537";
        image_subresources_subpass_read_09003    = "VUID-vkCmdDrawMeshTasksIndirectNV-None-09003";
        image_subresources_subpass_write_06539   = "VUID-vkCmdDrawMeshTasksIndirectNV-None-06539";
        sampler_imageview_type_08609             = "VUID-vkCmdDrawMeshTasksIndirectNV-None-08609";
        sampler_implicitLod_dref_proj_08610      = "VUID-vkCmdDrawMeshTasksIndirectNV-None-08610";
        sampler_bias_offset_08611                = "VUID-vkCmdDrawMeshTasksIndirectNV-None-08611";
        dynamic_state_setting_commands_08608     = "VUID-vkCmdDrawMeshTasksIndirectNV-None-08608";
        msrtss_rasterization_samples_07284       = "VUID-vkCmdDrawMeshTasksIndirectNV-multisampledRenderToSingleSampled-07284";
        unprotected_command_buffer_02707         = "VUID-vkCmdDrawMeshTasksIndirectNV-commandBuffer-02707";
        max_multiview_instance_index_02688       = "VUID-vkCmdDrawMeshTasksIndirectNV-maxMultiviewInstanceIndex-02688";
        img_filter_cubic_02693                   = "VUID-vkCmdDrawMeshTasksIndirectNV-None-02693";
        filter_cubic_02694                       = "VUID-vkCmdDrawMeshTasksIndirectNV-filterCubic-02694";
        filter_cubic_min_max_02695               = "VUID-vkCmdDrawMeshTasksIndirectNV-filterCubicMinmax-02695";
        viewport_count_primitive_shading_rate_04552 = "VUID-vkCmdDrawMeshTasksIndirectNV-primitiveFragmentShadingRateWithMultipleViewports-04552";
        rasterizer_discard_enable_04876          = "VUID-vkCmdDrawMeshTasksIndirectNV-None-04876";
        depth_bias_enable_04877                  = "VUID-vkCmdDrawMeshTasksIndirectNV-None-04877";
        logic_op_04878                           = "VUID-vkCmdDrawMeshTasksIndirectNV-logicOp-04878";
        blend_enable_04727                       = "VUID-vkCmdDrawMeshTasksIndirectNV-blendEnable-04727";
        dynamic_discard_rectangle_07751          = "VUID-vkCmdDrawMeshTasksIndirectNV-None-07751";
        dynamic_discard_rectangle_enable_07880   = "VUID-vkCmdDrawMeshTasksIndirectNV-None-07880";
        dynamic_discard_rectangle_mode_07881     =  "VUID-vkCmdDrawMeshTasksIndirectNV-None-07881";
        dynamic_exclusive_scissor_enable_07878   = "VUID-vkCmdDrawMeshTasksIndirectNV-None-07878";
        dynamic_exclusive_scissor_07879          = "VUID-vkCmdDrawMeshTasksIndirectNV-None-07879";
        dynamic_color_write_enable_07749         = "VUID-vkCmdDrawMeshTasksIndirectNV-None-07749";
        dynamic_color_write_enable_count_07750   = "VUID-vkCmdDrawMeshTasksIndirectNV-attachmentCount-07750";
        dynamic_attachment_feedback_loop_08877   = "VUID-vkCmdDrawMeshTasksIndirectNV-None-08877";
        dynamic_rendering_view_mask_06178        = "VUID-vkCmdDrawMeshTasksIndirectNV-viewMask-06178";
        dynamic_rendering_color_count_06179      = "VUID-vkCmdDrawMeshTasksIndirectNV-colorAttachmentCount-06179";
        dynamic_rendering_color_formats_08910    = "VUID-vkCmdDrawMeshTasksIndirectNV-dynamicRenderingUnusedAttachments-08910";
        dynamic_rendering_unused_attachments_08911 = "VUID-vkCmdDrawMeshTasksIndirectNV-dynamicRenderingUnusedAttachments-08911";
        dynamic_rendering_undefined_color_formats_08912 = "VUID-vkCmdDrawMeshTasksIndirectNV-dynamicRenderingUnusedAttachments-08912";
        dynamic_rendering_undefined_depth_format_08916 = "VUID-vkCmdDrawMeshTasksIndirectNV-dynamicRenderingUnusedAttachments-08916";
        dynamic_rendering_undefined_stencil_format_08916 = "VUID-vkCmdDrawMeshTasksIndirectNV-dynamicRenderingUnusedAttachments-08916";
        dynamic_rendering_depth_format_08914     = "VUID-vkCmdDrawMeshTasksIndirectNV-dynamicRenderingUnusedAttachments-08914";
        dynamic_rendering_unused_attachments_08915 = "VUID-vkCmdDrawMeshTasksIndirectNV-dynamicRenderingUnusedAttachments-08915";
        dynamic_rendering_stencil_format_08917   = "VUID-vkCmdDrawMeshTasksIndirectNV-dynamicRenderingUnusedAttachments-08917";
        dynamic_rendering_unused_attachments_08918 = "VUID-vkCmdDrawMeshTasksIndirectNV-dynamicRenderingUnusedAttachments-08918";
        dynamic_rendering_fsr_06183              = "VUID-vkCmdDrawMeshTasksIndirectNV-imageView-06183";
        dynamic_rendering_fdm_06184              = "VUID-vkCmdDrawMeshTasksIndirectNV-imageView-06184";
        dynamic_rendering_color_sample_06185     = "VUID-vkCmdDrawMeshTasksIndirectNV-colorAttachmentCount-06185";
        dynamic_rendering_depth_sample_06186     = "VUID-vkCmdDrawMeshTasksIndirectNV-pDepthAttachment-06186";
        dynamic_rendering_stencil_sample_06187   = "VUID-vkCmdDrawMeshTasksIndirectNV-pStencilAttachment-06187";
        dynamic_rendering_06198                  = "VUID-vkCmdDrawMeshTasksIndirectNV-renderPass-06198";
        dynamic_rendering_07285                  = "VUID-vkCmdDrawMeshTasksIndirectNV-multisampledRenderToSingleSampled-07285";
        dynamic_rendering_07286                  = "VUID-vkCmdDrawMeshTasksIndirectNV-multisampledRenderToSingleSampled-07286";
        dynamic_rendering_07287                  = "VUID-vkCmdDrawMeshTasksIndirectNV-multisampledRenderToSingleSampled-07287";
        image_view_access_64_04470               = "VUID-vkCmdDrawMeshTasksIndirectNV-SampledType-04470";
        image_view_access_32_04471               = "VUID-vkCmdDrawMeshTasksIndirectNV-SampledType-04471";
        image_view_sparse_64_04474               = "VUID-vkCmdDrawMeshTasksIndirectNV-sparseImageInt64Atomics-04474";
        buffer_view_access_64_04472              = "VUID-vkCmdDrawMeshTasksIndirectNV-SampledType-04472";
        buffer_view_access_32_04473              = "VUID-vkCmdDrawMeshTasksIndirectNV-SampledType-04473";
        storage_image_read_without_format_07028  = "VUID-vkCmdDrawMeshTasksIndirectNV-OpTypeImage-07028";
        storage_image_write_without_format_07027 = "VUID-vkCmdDrawMeshTasksIndirectNV-OpTypeImage-07027";
        storage_texel_buffer_read_without_format_07030  = "VUID-vkCmdDrawMeshTasksIndirectNV-OpTypeImage-07030";
        storage_texel_buffer_write_without_format_07029 = "VUID-vkCmdDrawMeshTasksIndirectNV-OpTypeImage-07029";
        storage_image_write_texel_count_08795           = "VUID-vkCmdDrawMeshTasksIndirectNV-OpImageWrite-08795";
        storage_image_write_texel_count_08796           = "VUID-vkCmdDrawMeshTasksIndirectNV-OpImageWrite-08796";
        storage_texel_buffer_write_texel_count_04469    = "VUID-vkCmdDrawMeshTasksIndirectNV-OpImageWrite-04469";
        depth_compare_sample_06479               = "VUID-vkCmdDrawMeshTasksIndirectNV-None-06479";
        depth_read_only_06886                    = "VUID-vkCmdDrawMeshTasksIndirectNV-None-06886";
        stencil_read_only_06887                  = "VUID-vkCmdDrawMeshTasksIndirectNV-None-06887";
        dynamic_sample_locations_06666           = "VUID-vkCmdDrawMeshTasksIndirectNV-None-06666";
        dynamic_tessellation_domain_origin_07619 = "VUID-vkCmdDrawMeshTasksIndirectNV-None-07619";
        dynamic_depth_clamp_enable_07620         = "VUID-vkCmdDrawMeshTasksIndirectNV-None-07620";
        dynamic_polygon_mode_07621               = "VUID-vkCmdDrawMeshTasksIndirectNV-None-07621";
        dynamic_rasterization_samples_07622      = "VUID-vkCmdDrawMeshTasksIndirectNV-None-07622";
        dynamic_sample_mask_07623                = "VUID-vkCmdDrawMeshTasksIndirectNV-None-07623";
        dynamic_alpha_to_coverage_enable_07624   = "VUID-vkCmdDrawMeshTasksIndirectNV-None-07624";
        dynamic_alpha_to_coverage_component_08919 = "VUID-vkCmdDrawMeshTasksIndirectNV-alphaToCoverageEnable-08919";
        dynamic_alpha_to_one_enable_07625        = "VUID-vkCmdDrawMeshTasksIndirectNV-None-07625";
        dynamic_logic_op_enable_07626            = "VUID-vkCmdDrawMeshTasksIndirectNV-None-07626";
        dynamic_color_blend_enable_07476         = "VUID-vkCmdDrawMeshTasksIndirectNV-firstAttachment-07476";
        dynamic_color_blend_equation_07477       = "VUID-vkCmdDrawMeshTasksIndirectNV-firstAttachment-07477";
        dynamic_color_write_mask_07478           = "VUID-vkCmdDrawMeshTasksIndirectNV-firstAttachment-07478";
        dynamic_rasterization_stream_07630       = "VUID-vkCmdDrawMeshTasksIndirectNV-None-07630";
        dynamic_conservative_rasterization_mode_07631 = "VUID-vkCmdDrawMeshTasksIndirectNV-None-07631";
        dynamic_extra_primitive_overestimation_size_07632 = "VUID-vkCmdDrawMeshTasksIndirectNV-None-07632";
        dynamic_depth_clip_enable_07633          = "VUID-vkCmdDrawMeshTasksIndirectNV-None-07633";
        dynamic_sample_locations_enable_07634    = "VUID-vkCmdDrawMeshTasksIndirectNV-None-07634";
        dynamic_color_blend_advanced_07479       = "VUID-vkCmdDrawMeshTasksIndirectNV-firstAttachment-07479";
        dynamic_provoking_vertex_mode_07636      = "VUID-vkCmdDrawMeshTasksIndirectNV-None-07636";
        dynamic_line_rasterization_mode_07637    = "VUID-vkCmdDrawMeshTasksIndirectNV-None-07637";
        dynamic_line_stipple_enable_07638        = "VUID-vkCmdDrawMeshTasksIndirectNV-None-07638";
        dynamic_depth_clip_negative_one_to_one_07639 = "VUID-vkCmdDrawMeshTasksIndirectNV-None-07639";
        dynamic_viewport_w_scaling_enable_07640  = "VUID-vkCmdDrawMeshTasksIndirectNV-None-07640";
        dynamic_viewport_swizzle_07641           = "VUID-vkCmdDrawMeshTasksIndirectNV-None-07641";
        dynamic_coverage_to_color_enable_07642   = "VUID-vkCmdDrawMeshTasksIndirectNV-None-07642";
        dynamic_coverage_to_color_location_07643 = "VUID-vkCmdDrawMeshTasksIndirectNV-None-07643";
        dynamic_coverage_modulation_mode_07644   = "VUID-vkCmdDrawMeshTasksIndirectNV-None-07644";
        dynamic_coverage_modulation_table_enable_07645 = "VUID-vkCmdDrawMeshTasksIndirectNV-None-07645";
        dynamic_coverage_modulation_table_07646  = "VUID-vkCmdDrawMeshTasksIndirectNV-None-07646";
        dynamic_shading_rate_image_enable_07647  = "VUID-vkCmdDrawMeshTasksIndirectNV-None-07647";
        dynamic_representative_fragment_test_enable_07648  = "VUID-vkCmdDrawMeshTasksIndirectNV-None-07648";
        dynamic_coverage_reduction_mode_07649    = "VUID-vkCmdDrawMeshTasksIndirectNV-None-07649";
        dynamic_viewport_07831                   = "VUID-vkCmdDrawMeshTasksIndirectNV-None-07831";
        dynamic_scissor_07832                    = "VUID-vkCmdDrawMeshTasksIndirectNV-None-07832";
        dynamic_depth_bias_07834                 = "VUID-vkCmdDrawMeshTasksIndirectNV-None-07834";
        dynamic_line_width_07833                 = "VUID-vkCmdDrawMeshTasksIndirectNV-None-07833";
        dynamic_line_stipple_ext_07849           = "VUID-vkCmdDrawMeshTasksIndirectNV-None-07849";
        dynamic_blend_constants_07835            = "VUID-vkCmdDrawMeshTasksIndirectNV-None-07835";
        dynamic_depth_bounds_07836               = "VUID-vkCmdDrawMeshTasksIndirectNV-None-07836";
        dynamic_depth_enable_08715               = "VUID-vkCmdDrawMeshTasksIndirectNV-pDynamicStates-08715";
        dynamic_stencil_compare_mask_07837       = "VUID-vkCmdDrawMeshTasksIndirectNV-None-07837";
        dynamic_stencil_write_mask_07838         = "VUID-vkCmdDrawMeshTasksIndirectNV-None-07838";
        dynamic_stencil_write_mask_08716         = "VUID-vkCmdDrawMeshTasksIndirectNV-pDynamicStates-08716";
        dynamic_stencil_reference_07839          = "VUID-vkCmdDrawMeshTasksIndirectNV-None-07839";
        dynamic_state_inherited_07850            = "VUID-vkCmdDrawMeshTasksIndirectNV-None-07850";
        dynamic_cull_mode_07840                  = "VUID-vkCmdDrawMeshTasksIndirectNV-None-07840";
        dynamic_front_face_07841                 = "VUID-vkCmdDrawMeshTasksIndirectNV-None-07841";
        dynamic_depth_test_enable_07843          = "VUID-vkCmdDrawMeshTasksIndirectNV-None-07843";
        dynamic_depth_write_enable_07844         = "VUID-vkCmdDrawMeshTasksIndirectNV-None-07844";
        dynamic_depth_compare_op_07845           = "VUID-vkCmdDrawMeshTasksIndirectNV-None-07845";
        dynamic_depth_bound_test_enable_07846    = "VUID-vkCmdDrawMeshTasksIndirectNV-None-07846";
        dynamic_stencil_test_enable_07847        = "VUID-vkCmdDrawMeshTasksIndirectNV-None-07847";
        dynamic_stencil_op_07848                 = "VUID-vkCmdDrawMeshTasksIndirectNV-None-07848";
        primitives_generated_06708               = "VUID-vkCmdDrawMeshTasksIndirectNV-primitivesGeneratedQueryWithRasterizerDiscard-06708";
        primitives_generated_streams_06709       = "VUID-vkCmdDrawMeshTasksIndirectNV-primitivesGeneratedQueryWithNonZeroStreams-06709";
        mesh_shader_stages_06480                 = "VUID-vkCmdDrawMeshTasksIndirectNV-stage-06480";
        missing_mesh_shader_stages_07080         = "VUID-vkCmdDrawMeshTasksIndirectNV-MeshNV-07081";
        descriptor_buffer_bit_set_08114          = "VUID-vkCmdDrawMeshTasksIndirectNV-None-08114";
        descriptor_buffer_bit_not_set_08115      = "VUID-vkCmdDrawMeshTasksIndirectNV-None-08115";
        descriptor_buffer_set_offset_missing_08117 = "VUID-vkCmdDrawMeshTasksIndirectNV-None-08117";
        image_view_dim_07752                     = "VUID-vkCmdDrawMeshTasksIndirectNV-viewType-07752";
        image_view_numeric_format_07753          = "VUID-vkCmdDrawMeshTasksIndirectNV-format-07753";
        stippled_rectangular_lines_07495         = "VUID-vkCmdDrawMeshTasksIndirectNV-stippledLineEnable-07495";
        stippled_bresenham_lines_07496           = "VUID-vkCmdDrawMeshTasksIndirectNV-stippledLineEnable-07496";
        stippled_smooth_lines_07497              = "VUID-vkCmdDrawMeshTasksIndirectNV-stippledLineEnable-07497";
        stippled_default_strict_07498            = "VUID-vkCmdDrawMeshTasksIndirectNV-stippledLineEnable-07498";
        viewport_and_scissor_with_count_08635    = "VUID-vkCmdDrawMeshTasksIndirectNV-None-08635";
        viewport_w_scaling_08636                 = "VUID-vkCmdDrawMeshTasksIndirectNV-None-08636";
        shading_rate_palette_08637               = "VUID-vkCmdDrawMeshTasksIndirectNV-None-08637";
        exclusive_scissor_08638                  = "VUID-vkCmdDrawMeshTasksIndirectNV-None-08638";
        external_format_resolve_09362            = "VUID-vkCmdDrawMeshTasksIndirectNV-colorAttachmentCount-09362";
        external_format_resolve_09363            = "VUID-vkCmdDrawMeshTasksIndirectNV-None-09363";
        external_format_resolve_09364            = "VUID-vkCmdDrawMeshTasksIndirectNV-None-09364";
        external_format_resolve_09365            = "VUID-vkCmdDrawMeshTasksIndirectNV-None-09365";
        external_format_resolve_09368            = "VUID-vkCmdDrawMeshTasksIndirectNV-None-09368";
        external_format_resolve_09369            = "VUID-vkCmdDrawMeshTasksIndirectNV-None-09369";
        external_format_resolve_09372            = "VUID-vkCmdDrawMeshTasksIndirectNV-colorAttachmentCount-09372";
        set_rasterizer_discard_enable_08639      = "VUID-vkCmdDrawMeshTasksIndirectNV-None-08639";
        set_depth_bias_enable_08640              = "VUID-vkCmdDrawMeshTasksIndirectNV-None-08640";
        set_logic_op_08641                       = "VUID-vkCmdDrawMeshTasksIndirectNV-None-08641";
        set_color_blend_enable_08643             = "VUID-vkCmdDrawMeshTasksIndirectNV-None-08643";
        set_rasterization_samples_08644              = "VUID-vkCmdDrawMeshTasksIndirectNV-None-08644";
        set_color_write_enable_08646             = "VUID-vkCmdDrawMeshTasksIndirectNV-None-08646";
        set_color_write_enable_08647             = "VUID-vkCmdDrawMeshTasksIndirectNV-None-08647";
        set_discard_rectangles_enable_08648      = "VUID-vkCmdDrawMeshTasksIndirectNV-None-08648";
        set_discard_rectangles_mode_08649      = "VUID-vkCmdDrawMeshTasksIndirectNV-None-08649";
        set_depth_clamp_enable_08650             = "VUID-vkCmdDrawMeshTasksIndirectNV-None-08650";
        set_polygon_mode_08651                   = "VUID-vkCmdDrawMeshTasksIndirectNV-None-08651";
        set_rasterization_samples_08652          = "VUID-vkCmdDrawMeshTasksIndirectNV-None-08652";
        set_sample_mask_08653                    = "VUID-vkCmdDrawMeshTasksIndirectNV-None-08653";
        set_alpha_to_coverage_enable_08654       = "VUID-vkCmdDrawMeshTasksIndirectNV-None-08654";
        set_alpha_to_one_enable_08655            = "VUID-vkCmdDrawMeshTasksIndirectNV-None-08655";
        set_logic_op_enable_08656                = "VUID-vkCmdDrawMeshTasksIndirectNV-None-08656";
        set_color_blend_enable_08657             = "VUID-vkCmdDrawMeshTasksIndirectNV-None-08657";
        set_color_blend_equation_08658           = "VUID-vkCmdDrawMeshTasksIndirectNV-None-08658";
        set_color_write_mask_08659               = "VUID-vkCmdDrawMeshTasksIndirectNV-None-08659";
        set_rasterization_streams_08660          = "VUID-vkCmdDrawMeshTasksIndirectNV-None-08660";
        set_conservative_rasterization_mode_08661 = "VUID-vkCmdDrawMeshTasksIndirectNV-None-08661";
        set_extra_primitive_overestimation_size_08662 = "VUID-vkCmdDrawMeshTasksIndirectNV-None-08662";
        set_depth_clip_enable_08663              = "VUID-vkCmdDrawMeshTasksIndirectNV-None-08663";
        set_sample_locations_enable_08664        = "VUID-vkCmdDrawMeshTasksIndirectNV-None-08664";
        set_provoking_vertex_mode_08665          = "VUID-vkCmdDrawMeshTasksIndirectNV-None-08665";
        set_line_rasterization_mode_08666        = "VUID-vkCmdDrawMeshTasksIndirectNV-None-08666";
        set_line_rasterization_mode_08667        = "VUID-vkCmdDrawMeshTasksIndirectNV-None-08667";
        set_line_rasterization_mode_08668        = "VUID-vkCmdDrawMeshTasksIndirectNV-None-08668";
        set_line_stipple_enable_08669            = "VUID-vkCmdDrawMeshTasksIndirectNV-None-08669";
        set_line_stipple_enable_08670            = "VUID-vkCmdDrawMeshTasksIndirectNV-None-08670";
        set_line_stipple_enable_08671            = "VUID-vkCmdDrawMeshTasksIndirectNV-None-08671";
        set_line_stipple_08672                   = "VUID-vkCmdDrawMeshTasksIndirectNV-None-08672";
        set_depth_clip_negative_one_to_one_08673 = "VUID-vkCmdDrawMeshTasksIndirectNV-None-08673";
        set_viewport_w_scaling_enable_08674      = "VUID-vkCmdDrawMeshTasksIndirectNV-None-08674";
        set_viewport_swizzle_08675               = "VUID-vkCmdDrawMeshTasksIndirectNV-None-08675";
        set_coverage_to_color_enable_08676       = "VUID-vkCmdDrawMeshTasksIndirectNV-None-08676";
        set_coverage_to_color_location_08677     = "VUID-vkCmdDrawMeshTasksIndirectNV-None-08677";
        set_coverage_modulation_mode_08678       = "VUID-vkCmdDrawMeshTasksIndirectNV-None-08678";
        set_coverage_modulation_table_enable_08679 = "VUID-vkCmdDrawMeshTasksIndirectNV-None-08679";
        set_coverage_modulation_table_08680      = "VUID-vkCmdDrawMeshTasksIndirectNV-None-08680";
        set_shading_rate_image_enable_08681      = "VUID-vkCmdDrawMeshTasksIndirectNV-None-08681";
        set_representative_fragment_test_enable_08682 = "VUID-vkCmdDrawMeshTasksIndirectNV-None-08682";
        set_coverage_reduction_mode_08683        = "VUID-vkCmdDrawMeshTasksIndirectNV-None-08683";
        vertex_shader_08684                      = "VUID-vkCmdDrawMeshTasksIndirectNV-None-08684";
        tessellation_control_shader_08685        = "VUID-vkCmdDrawMeshTasksIndirectNV-None-08685";
        tessellation_evaluation_shader_08686     = "VUID-vkCmdDrawMeshTasksIndirectNV-None-08686";
        geometry_shader_08687                    = "VUID-vkCmdDrawMeshTasksIndirectNV-None-08687";
        fragment_shader_08688                    = "VUID-vkCmdDrawMeshTasksIndirectNV-None-08688";
        task_shader_08689                        = "VUID-vkCmdDrawMeshTasksIndirectNV-None-08689";
        mesh_shader_08690                        = "VUID-vkCmdDrawMeshTasksIndirectNV-None-08690";
        vert_mesh_shader_08693                   = "VUID-vkCmdDrawMeshTasksIndirectNV-None-08693";
        task_mesh_shader_08694                   = "VUID-vkCmdDrawMeshTasksIndirectNV-None-08694";
        task_mesh_shader_08695                   = "VUID-vkCmdDrawMeshTasksIndirectNV-None-08695";
        vert_task_mesh_shader_08696              = "VUID-vkCmdDrawMeshTasksIndirectNV-None-08696";
        linked_shaders_08698                     = "VUID-vkCmdDrawMeshTasksIndirectNV-None-08698";
        linked_shaders_08699                     = "VUID-vkCmdDrawMeshTasksIndirectNV-None-08699";
        shaders_push_constants_08878             = "VUID-vkCmdDrawMeshTasksIndirectNV-None-08878";
        shaders_descriptor_layouts_08879         = "VUID-vkCmdDrawMeshTasksIndirectNV-None-08879";
        set_attachment_feedback_loop_enable_08880 = "VUID-vkCmdDrawMeshTasksIndirectNV-None-08880";
        set_line_width_08617                     = "VUID-vkCmdDrawMeshTasksIndirectNV-None-08617";
        set_line_width_08618                     = "VUID-vkCmdDrawMeshTasksIndirectNV-None-08618";
        set_depth_bias_08620                     = "VUID-vkCmdDrawMeshTasksIndirectNV-None-08620";
        set_blend_constants_08621                = "VUID-vkCmdDrawMeshTasksIndirectNV-None-08621";
        set_depth_bounds_08622                   = "VUID-vkCmdDrawMeshTasksIndirectNV-None-08622";
        set_stencil_compare_mask_08623           = "VUID-vkCmdDrawMeshTasksIndirectNV-None-08623";
        set_stencil_write_mask_08624             = "VUID-vkCmdDrawMeshTasksIndirectNV-None-08624";
        set_stencil_reference_08625              = "VUID-vkCmdDrawMeshTasksIndirectNV-None-08625";
        set_sample_locations_08626               = "VUID-vkCmdDrawMeshTasksIndirectNV-None-08626";
        set_cull_mode_08627                      = "VUID-vkCmdDrawMeshTasksIndirectNV-None-08627";
        set_front_face_08628                     = "VUID-vkCmdDrawMeshTasksIndirectNV-None-08628";
        set_depth_test_enable_08629              = "VUID-vkCmdDrawMeshTasksIndirectNV-None-08629";
        set_depth_write_enable_08630             = "VUID-vkCmdDrawMeshTasksIndirectNV-None-08630";
        set_depth_comapre_op_08631               = "VUID-vkCmdDrawMeshTasksIndirectNV-None-08631";
        set_depth_bounds_test_enable_08632       = "VUID-vkCmdDrawMeshTasksIndirectNV-None-08632";
        set_stencil_test_enable_08633            = "VUID-vkCmdDrawMeshTasksIndirectNV-None-08633";
        set_stencil_op_08634                     = "VUID-vkCmdDrawMeshTasksIndirectNV-None-08634";
        set_line_width_08619                     = "VUID-vkCmdDrawMeshTasksIndirectNV-None-08619";
        set_viewport_with_count_08642            = "VUID-vkCmdDrawMeshTasksIndirectNV-primitiveFragmentShadingRateWithMultipleViewports-08642";
        alpha_component_word_08920               = "VUID-vkCmdDrawMeshTasksIndirectNV-alphaToCoverageEnable-08920";
        color_write_mask_09116                   = "VUID-vkCmdDrawMeshTasksIndirectNV-None-09116";
        set_clip_space_w_scaling_09232           = "VUID-vkCmdDrawMeshTasksIndirectNV-None-09232";
        set_discard_rectangle_09236              = "VUID-vkCmdDrawMeshTasksIndirectNV-rasterizerDiscardEnable-09236";
        set_viewport_coarse_sample_order_09233   = "VUID-vkCmdDrawMeshTasksIndirectNV-shadingRateImage-09233";
        set_viewport_shading_rate_palette_09234  = "VUID-vkCmdDrawMeshTasksIndirectNV-shadingRateImage-09234";
        set_exclusive_scissor_enable_09235       = "VUID-vkCmdDrawMeshTasksIndirectNV-exclusiveScissor-09235";
        set_fragment_shading_rate_09238          = "VUID-vkCmdDrawMeshTasksIndirectNV-pipelineFragmentShadingRate-09238";
        set_tessellation_domain_origin_09237     = "VUID-vkCmdDrawMeshTasksIndirectNV-None-09237";
        rasterization_samples_07935              = "VUID-vkCmdDrawMeshTasksIndirectNV-pNext-07935";
        mesh_shader_queries_07073                = "VUID-vkCmdDrawMeshTasksIndirectNV-stage-07073";
        blend_advanced_07480                     = "VUID-vkCmdDrawMeshTasksIndirectNV-advancedBlendMaxColorAttachments-07480";
        blend_feature_07470                      = "VUID-vkCmdDrawMeshTasksIndirectNV-pColorBlendEnables-07470";
        color_attachment_08963                   = "VUID-vkCmdDrawMeshTasksIndirectNV-pColorAttachments-08963";
        depth_attachment_08964                   = "VUID-vkCmdDrawMeshTasksIndirectNV-pDepthAttachment-08964";
        stencil_attachment_08965                 = "VUID-vkCmdDrawMeshTasksIndirectNV-pStencilAttachment-08965";
        sample_locations_07482                   = "VUID-vkCmdDrawMeshTasksIndirectNV-sampleLocationsPerPixel-07482";
    }
};

struct DispatchVuidsCmdDrawMeshTasksIndirectCountNV : DrawDispatchVuid {
    DispatchVuidsCmdDrawMeshTasksIndirectCountNV() : DrawDispatchVuid() {
        pipeline_bound_08606                     = "VUID-vkCmdDrawMeshTasksIndirectCountNV-None-08606";
        pipeline_or_shaders_bound_08607          = "VUID-vkCmdDrawMeshTasksIndirectCountNV-None-08607";
        compatible_pipeline_08600                = "VUID-vkCmdDrawMeshTasksIndirectCountNV-None-08600";
        render_pass_compatible_02684             = "VUID-vkCmdDrawMeshTasksIndirectCountNV-renderPass-02684";
        render_pass_began_08876                  = "VUID-vkCmdDrawMeshTasksIndirectCountNV-None-08876";
        subpass_index_02685                      = "VUID-vkCmdDrawMeshTasksIndirectCountNV-subpass-02685";
        sample_location_02689                    = "VUID-vkCmdDrawMeshTasksIndirectCountNV-sampleLocationsEnable-02689";
        linear_filter_sampler_04553              = "VUID-vkCmdDrawMeshTasksIndirectCountNV-magFilter-04553";
        linear_mipmap_sampler_04770              = "VUID-vkCmdDrawMeshTasksIndirectCountNV-mipmapMode-04770";
        cubic_sampler_02692                      = "VUID-vkCmdDrawMeshTasksIndirectCountNV-None-02692";
        indirect_protected_cb_02646              = "VUID-vkCmdDrawMeshTasksIndirectCountNV-commandBuffer-02711";
        indirect_contiguous_memory_02708         = "VUID-vkCmdDrawMeshTasksIndirectCountNV-buffer-02708";
        indirect_buffer_bit_02290                = "VUID-vkCmdDrawMeshTasksIndirectCountNV-buffer-02709";
        indirect_count_contiguous_memory_02714   = "VUID-vkCmdDrawMeshTasksIndirectCountNV-countBuffer-02714";
        indirect_count_buffer_bit_02715          = "VUID-vkCmdDrawMeshTasksIndirectCountNV-countBuffer-02715";
        indirect_count_offset_04129              = "VUID-vkCmdDrawMeshTasksIndirectCountNV-countBufferOffset-04129";
        viewport_count_03417                     = "VUID-vkCmdDrawMeshTasksIndirectCountNV-viewportCount-03417";
        scissor_count_03418                      = "VUID-vkCmdDrawMeshTasksIndirectCountNV-scissorCount-03418";
        viewport_scissor_count_03419             = "VUID-vkCmdDrawMeshTasksIndirectCountNV-viewportCount-03419";
        corner_sampled_address_mode_02696        = "VUID-vkCmdDrawMeshTasksIndirectCountNV-flags-02696";
        imageview_atomic_02691                   = "VUID-vkCmdDrawMeshTasksIndirectCountNV-None-02691";
        bufferview_atomic_07888                  = "VUID-vkCmdDrawMeshTasksIndirectCountNV-None-07888";
        push_constants_set_08602                 = "VUID-vkCmdDrawMeshTasksIndirectCountNV-maintenance4-08602";
        image_subresources_render_pass_write_06537 = "VUID-vkCmdDrawMeshTasksIndirectCountNV-None-06537";
        image_subresources_subpass_read_09003    = "VUID-vkCmdDrawMeshTasksIndirectCountNV-None-09003";
        image_subresources_subpass_write_06539   = "VUID-vkCmdDrawMeshTasksIndirectCountNV-None-06539";
        sampler_imageview_type_08609             = "VUID-vkCmdDrawMeshTasksIndirectCountNV-None-08609";
        sampler_implicitLod_dref_proj_08610      = "VUID-vkCmdDrawMeshTasksIndirectCountNV-None-08610";
        sampler_bias_offset_08611                = "VUID-vkCmdDrawMeshTasksIndirectCountNV-None-08611";
        dynamic_state_setting_commands_08608     = "VUID-vkCmdDrawMeshTasksIndirectCountNV-None-08608";
        msrtss_rasterization_samples_07284       = "VUID-vkCmdDrawMeshTasksIndirectCountNV-multisampledRenderToSingleSampled-07284";
        unprotected_command_buffer_02707         = "VUID-vkCmdDrawMeshTasksIndirectCountNV-commandBuffer-02707";
        max_multiview_instance_index_02688       = "VUID-vkCmdDrawMeshTasksIndirectCountNV-maxMultiviewInstanceIndex-02688";
        img_filter_cubic_02693                   = "VUID-vkCmdDrawMeshTasksIndirectCountNV-None-02693";
        filter_cubic_02694                       = "VUID-vkCmdDrawMeshTasksIndirectCountNV-filterCubic-02694";
        filter_cubic_min_max_02695               = "VUID-vkCmdDrawMeshTasksIndirectCountNV-filterCubicMinmax-02695";
        viewport_count_primitive_shading_rate_04552 = "VUID-vkCmdDrawMeshTasksIndirectCountNV-primitiveFragmentShadingRateWithMultipleViewports-04552";
        rasterizer_discard_enable_04876          = "VUID-vkCmdDrawMeshTasksIndirectCountNV-None-04876";
        depth_bias_enable_04877                  = "VUID-vkCmdDrawMeshTasksIndirectCountNV-None-04877";
        logic_op_04878                           = "VUID-vkCmdDrawMeshTasksIndirectCountNV-logicOp-04878";
        blend_enable_04727                       = "VUID-vkCmdDrawMeshTasksIndirectCountNV-blendEnable-04727";
        dynamic_discard_rectangle_07751          = "VUID-vkCmdDrawMeshTasksIndirectCountNV-None-07751";
        dynamic_discard_rectangle_enable_07880   = "VUID-vkCmdDrawMeshTasksIndirectCountNV-None-07880";
        dynamic_discard_rectangle_mode_07881     =  "VUID-vkCmdDrawMeshTasksIndirectCountNV-None-07881";
        dynamic_exclusive_scissor_enable_07878   = "VUID-vkCmdDrawMeshTasksIndirectCountNV-None-07878";
        dynamic_exclusive_scissor_07879          = "VUID-vkCmdDrawMeshTasksIndirectCountNV-None-07879";
        dynamic_color_write_enable_07749         = "VUID-vkCmdDrawMeshTasksIndirectCountNV-None-07749";
        dynamic_color_write_enable_count_07750   = "VUID-vkCmdDrawMeshTasksIndirectCountNV-attachmentCount-07750";
        dynamic_attachment_feedback_loop_08877   = "VUID-vkCmdDrawMeshTasksIndirectCountNV-None-08877";
        dynamic_rendering_view_mask_06178        = "VUID-vkCmdDrawMeshTasksIndirectCountNV-viewMask-06178";
        dynamic_rendering_color_count_06179      = "VUID-vkCmdDrawMeshTasksIndirectCountNV-colorAttachmentCount-06179";
        dynamic_rendering_color_formats_08910    = "VUID-vkCmdDrawMeshTasksIndirectCountNV-dynamicRenderingUnusedAttachments-08910";
        dynamic_rendering_unused_attachments_08911 = "VUID-vkCmdDrawMeshTasksIndirectCountNV-dynamicRenderingUnusedAttachments-08911";
        dynamic_rendering_undefined_color_formats_08912 = "VUID-vkCmdDrawMeshTasksIndirectCountNV-dynamicRenderingUnusedAttachments-08912";
        dynamic_rendering_undefined_depth_format_08916 = "VUID-vkCmdDrawMeshTasksIndirectCountNV-dynamicRenderingUnusedAttachments-08916";
        dynamic_rendering_undefined_stencil_format_08916 = "VUID-vkCmdDrawMeshTasksIndirectCountNV-dynamicRenderingUnusedAttachments-08916";
        dynamic_rendering_depth_format_08914     = "VUID-vkCmdDrawMeshTasksIndirectCountNV-dynamicRenderingUnusedAttachments-08914";
        dynamic_rendering_unused_attachments_08915 = "VUID-vkCmdDrawMeshTasksIndirectCountNV-dynamicRenderingUnusedAttachments-08915";
        dynamic_rendering_stencil_format_08917   = "VUID-vkCmdDrawMeshTasksIndirectCountNV-dynamicRenderingUnusedAttachments-08917";
        dynamic_rendering_unused_attachments_08918 = "VUID-vkCmdDrawMeshTasksIndirectCountNV-dynamicRenderingUnusedAttachments-08918";
        dynamic_rendering_fsr_06183              = "VUID-vkCmdDrawMeshTasksIndirectCountNV-imageView-06183";
        dynamic_rendering_fdm_06184              = "VUID-vkCmdDrawMeshTasksIndirectCountNV-imageView-06184";
        dynamic_rendering_color_sample_06185     = "VUID-vkCmdDrawMeshTasksIndirectCountNV-colorAttachmentCount-06185";
        dynamic_rendering_depth_sample_06186     = "VUID-vkCmdDrawMeshTasksIndirectCountNV-pDepthAttachment-06186";
        dynamic_rendering_stencil_sample_06187   = "VUID-vkCmdDrawMeshTasksIndirectCountNV-pStencilAttachment-06187";
        dynamic_rendering_06198                  = "VUID-vkCmdDrawMeshTasksIndirectCountNV-renderPass-06198";
        dynamic_rendering_07285                  = "VUID-vkCmdDrawMeshTasksIndirectCountNV-multisampledRenderToSingleSampled-07285";
        dynamic_rendering_07286                  = "VUID-vkCmdDrawMeshTasksIndirectCountNV-multisampledRenderToSingleSampled-07286";
        dynamic_rendering_07287                  = "VUID-vkCmdDrawMeshTasksIndirectCountNV-multisampledRenderToSingleSampled-07287";
        image_view_access_64_04470               = "VUID-vkCmdDrawMeshTasksIndirectCountNV-SampledType-04470";
        image_view_access_32_04471               = "VUID-vkCmdDrawMeshTasksIndirectCountNV-SampledType-04471";
        image_view_sparse_64_04474               = "VUID-vkCmdDrawMeshTasksIndirectCountNV-sparseImageInt64Atomics-04474";
        buffer_view_access_64_04472              = "VUID-vkCmdDrawMeshTasksIndirectCountNV-SampledType-04472";
        buffer_view_access_32_04473              = "VUID-vkCmdDrawMeshTasksIndirectCountNV-SampledType-04473";
        storage_image_read_without_format_07028  = "VUID-vkCmdDrawMeshTasksIndirectCountNV-OpTypeImage-07028";
        storage_image_write_without_format_07027 = "VUID-vkCmdDrawMeshTasksIndirectCountNV-OpTypeImage-07027";
        storage_texel_buffer_read_without_format_07030  = "VUID-vkCmdDrawMeshTasksIndirectCountNV-OpTypeImage-07030";
        storage_texel_buffer_write_without_format_07029 = "VUID-vkCmdDrawMeshTasksIndirectCountNV-OpTypeImage-07029";
        storage_image_write_texel_count_08795           = "VUID-vkCmdDrawMeshTasksIndirectCountNV-OpImageWrite-08795";
        storage_image_write_texel_count_08796           = "VUID-vkCmdDrawMeshTasksIndirectCountNV-OpImageWrite-08796";
        storage_texel_buffer_write_texel_count_04469    = "VUID-vkCmdDrawMeshTasksIndirectCountNV-OpImageWrite-04469";
        depth_compare_sample_06479               = "VUID-vkCmdDrawMeshTasksIndirectCountNV-None-06479";
        depth_read_only_06886                    = "VUID-vkCmdDrawMeshTasksIndirectCountNV-None-06886";
        stencil_read_only_06887                  = "VUID-vkCmdDrawMeshTasksIndirectCountNV-None-06887";
        dynamic_sample_locations_06666           = "VUID-vkCmdDrawMeshTasksIndirectCountNV-None-06666";
        dynamic_tessellation_domain_origin_07619 = "VUID-vkCmdDrawMeshTasksIndirectCountNV-None-07619";
        dynamic_depth_clamp_enable_07620         = "VUID-vkCmdDrawMeshTasksIndirectCountNV-None-07620";
        dynamic_polygon_mode_07621               = "VUID-vkCmdDrawMeshTasksIndirectCountNV-None-07621";
        dynamic_rasterization_samples_07622      = "VUID-vkCmdDrawMeshTasksIndirectCountNV-None-07622";
        dynamic_sample_mask_07623                = "VUID-vkCmdDrawMeshTasksIndirectCountNV-None-07623";
        dynamic_alpha_to_coverage_enable_07624   = "VUID-vkCmdDrawMeshTasksIndirectCountNV-None-07624";
        dynamic_alpha_to_coverage_component_08919 = "VUID-vkCmdDrawMeshTasksIndirectCountNV-alphaToCoverageEnable-08919";
        dynamic_alpha_to_one_enable_07625        = "VUID-vkCmdDrawMeshTasksIndirectCountNV-None-07625";
        dynamic_logic_op_enable_07626            = "VUID-vkCmdDrawMeshTasksIndirectCountNV-None-07626";
        dynamic_color_blend_enable_07476         = "VUID-vkCmdDrawMeshTasksIndirectCountNV-firstAttachment-07476";
        dynamic_color_blend_equation_07477       = "VUID-vkCmdDrawMeshTasksIndirectCountNV-firstAttachment-07477";
        dynamic_color_write_mask_07478           = "VUID-vkCmdDrawMeshTasksIndirectCountNV-firstAttachment-07478";
        dynamic_rasterization_stream_07630       = "VUID-vkCmdDrawMeshTasksIndirectCountNV-None-07630";
        dynamic_conservative_rasterization_mode_07631 = "VUID-vkCmdDrawMeshTasksIndirectCountNV-None-07631";
        dynamic_extra_primitive_overestimation_size_07632 = "VUID-vkCmdDrawMeshTasksIndirectCountNV-None-07632";
        dynamic_depth_clip_enable_07633          = "VUID-vkCmdDrawMeshTasksIndirectCountNV-None-07633";
        dynamic_sample_locations_enable_07634    = "VUID-vkCmdDrawMeshTasksIndirectCountNV-None-07634";
        dynamic_color_blend_advanced_07479       = "VUID-vkCmdDrawMeshTasksIndirectCountNV-firstAttachment-07479";
        dynamic_provoking_vertex_mode_07636      = "VUID-vkCmdDrawMeshTasksIndirectCountNV-None-07636";
        dynamic_line_rasterization_mode_07637    = "VUID-vkCmdDrawMeshTasksIndirectCountNV-None-07637";
        dynamic_line_stipple_enable_07638        = "VUID-vkCmdDrawMeshTasksIndirectCountNV-None-07638";
        dynamic_depth_clip_negative_one_to_one_07639 = "VUID-vkCmdDrawMeshTasksIndirectCountNV-None-07639";
        dynamic_viewport_w_scaling_enable_07640  = "VUID-vkCmdDrawMeshTasksIndirectCountNV-None-07640";
        dynamic_viewport_swizzle_07641           = "VUID-vkCmdDrawMeshTasksIndirectCountNV-None-07641";
        dynamic_coverage_to_color_enable_07642   = "VUID-vkCmdDrawMeshTasksIndirectCountNV-None-07642";
        dynamic_coverage_to_color_location_07643 = "VUID-vkCmdDrawMeshTasksIndirectCountNV-None-07643";
        dynamic_coverage_modulation_mode_07644   = "VUID-vkCmdDrawMeshTasksIndirectCountNV-None-07644";
        dynamic_coverage_modulation_table_enable_07645 = "VUID-vkCmdDrawMeshTasksIndirectCountNV-None-07645";
        dynamic_coverage_modulation_table_07646  = "VUID-vkCmdDrawMeshTasksIndirectCountNV-None-07646";
        dynamic_shading_rate_image_enable_07647  = "VUID-vkCmdDrawMeshTasksIndirectCountNV-None-07647";
        dynamic_representative_fragment_test_enable_07648  = "VUID-vkCmdDrawMeshTasksIndirectCountNV-None-07648";
        dynamic_coverage_reduction_mode_07649    = "VUID-vkCmdDrawMeshTasksIndirectCountNV-None-07649";
        dynamic_viewport_07831                   = "VUID-vkCmdDrawMeshTasksIndirectCountNV-None-07831";
        dynamic_scissor_07832                    = "VUID-vkCmdDrawMeshTasksIndirectCountNV-None-07832";
        dynamic_depth_bias_07834                 = "VUID-vkCmdDrawMeshTasksIndirectCountNV-None-07834";
        dynamic_line_width_07833                 = "VUID-vkCmdDrawMeshTasksIndirectCountNV-None-07833";
        dynamic_line_stipple_ext_07849           = "VUID-vkCmdDrawMeshTasksIndirectCountNV-None-07849";
        dynamic_blend_constants_07835            = "VUID-vkCmdDrawMeshTasksIndirectCountNV-None-07835";
        dynamic_depth_bounds_07836               = "VUID-vkCmdDrawMeshTasksIndirectCountNV-None-07836";
        dynamic_depth_enable_08715               = "VUID-vkCmdDrawMeshTasksIndirectCountNV-pDynamicStates-08715";
        dynamic_stencil_compare_mask_07837       = "VUID-vkCmdDrawMeshTasksIndirectCountNV-None-07837";
        dynamic_stencil_write_mask_07838         = "VUID-vkCmdDrawMeshTasksIndirectCountNV-None-07838";
        dynamic_stencil_write_mask_08716         = "VUID-vkCmdDrawMeshTasksIndirectCountNV-pDynamicStates-08716";
        dynamic_stencil_reference_07839          = "VUID-vkCmdDrawMeshTasksIndirectCountNV-None-07839";
        dynamic_state_inherited_07850            = "VUID-vkCmdDrawMeshTasksIndirectCountNV-None-07850";
        dynamic_cull_mode_07840                  = "VUID-vkCmdDrawMeshTasksIndirectCountNV-None-07840";
        dynamic_front_face_07841                 = "VUID-vkCmdDrawMeshTasksIndirectCountNV-None-07841";
        dynamic_depth_test_enable_07843          = "VUID-vkCmdDrawMeshTasksIndirectCountNV-None-07843";
        dynamic_depth_write_enable_07844         = "VUID-vkCmdDrawMeshTasksIndirectCountNV-None-07844";
        dynamic_depth_compare_op_07845           = "VUID-vkCmdDrawMeshTasksIndirectCountNV-None-07845";
        dynamic_depth_bound_test_enable_07846    = "VUID-vkCmdDrawMeshTasksIndirectCountNV-None-07846";
        dynamic_stencil_test_enable_07847        = "VUID-vkCmdDrawMeshTasksIndirectCountNV-None-07847";
        dynamic_stencil_op_07848                 = "VUID-vkCmdDrawMeshTasksIndirectCountNV-None-07848";
        primitives_generated_06708               = "VUID-vkCmdDrawMeshTasksIndirectCountNV-primitivesGeneratedQueryWithRasterizerDiscard-06708";
        primitives_generated_streams_06709       = "VUID-vkCmdDrawMeshTasksIndirectCountNV-primitivesGeneratedQueryWithNonZeroStreams-06709";
        mesh_shader_stages_06480                 = "VUID-vkCmdDrawMeshTasksIndirectCountNV-stage-06480";
        missing_mesh_shader_stages_07080         = "VUID-vkCmdDrawMeshTasksIndirectCountNV-MeshNV-07082";
        descriptor_buffer_bit_set_08114          = "VUID-vkCmdDrawMeshTasksIndirectCountNV-None-08114";
        descriptor_buffer_bit_not_set_08115      = "VUID-vkCmdDrawMeshTasksIndirectCountNV-None-08115";
        descriptor_buffer_set_offset_missing_08117 = "VUID-vkCmdDrawMeshTasksIndirectCountNV-None-08117";
        image_view_dim_07752                     = "VUID-vkCmdDrawMeshTasksIndirectCountNV-viewType-07752";
        image_view_numeric_format_07753          = "VUID-vkCmdDrawMeshTasksIndirectCountNV-format-07753";
        stippled_rectangular_lines_07495         = "VUID-vkCmdDrawMeshTasksIndirectCountNV-stippledLineEnable-07495";
        stippled_bresenham_lines_07496           = "VUID-vkCmdDrawMeshTasksIndirectCountNV-stippledLineEnable-07496";
        stippled_smooth_lines_07497              = "VUID-vkCmdDrawMeshTasksIndirectCountNV-stippledLineEnable-07497";
        stippled_default_strict_07498            = "VUID-vkCmdDrawMeshTasksIndirectCountNV-stippledLineEnable-07498";
        viewport_and_scissor_with_count_08635    = "VUID-vkCmdDrawMeshTasksIndirectCountNV-None-08635";
        viewport_w_scaling_08636                 = "VUID-vkCmdDrawMeshTasksIndirectCountNV-None-08636";
        shading_rate_palette_08637               = "VUID-vkCmdDrawMeshTasksIndirectCountNV-None-08637";
        exclusive_scissor_08638                  = "VUID-vkCmdDrawMeshTasksIndirectCountNV-None-08638";
        external_format_resolve_09362            = "VUID-vkCmdDrawMeshTasksIndirectCountNV-colorAttachmentCount-09362";
        external_format_resolve_09363            = "VUID-vkCmdDrawMeshTasksIndirectCountNV-None-09363";
        external_format_resolve_09364            = "VUID-vkCmdDrawMeshTasksIndirectCountNV-None-09364";
        external_format_resolve_09365            = "VUID-vkCmdDrawMeshTasksIndirectCountNV-None-09365";
        external_format_resolve_09368            = "VUID-vkCmdDrawMeshTasksIndirectCountNV-None-09368";
        external_format_resolve_09369            = "VUID-vkCmdDrawMeshTasksIndirectCountNV-None-09369";
        external_format_resolve_09372            = "VUID-vkCmdDrawMeshTasksIndirectCountNV-colorAttachmentCount-09372";
        set_rasterizer_discard_enable_08639      = "VUID-vkCmdDrawMeshTasksIndirectCountNV-None-08639";
        set_depth_bias_enable_08640              = "VUID-vkCmdDrawMeshTasksIndirectCountNV-None-08640";
        set_logic_op_08641                       = "VUID-vkCmdDrawMeshTasksIndirectCountNV-None-08641";
        set_color_blend_enable_08643             = "VUID-vkCmdDrawMeshTasksIndirectCountNV-None-08643";
        set_rasterization_samples_08644              = "VUID-vkCmdDrawMeshTasksIndirectCountNV-None-08644";
        set_color_write_enable_08646             = "VUID-vkCmdDrawMeshTasksIndirectCountNV-None-08646";
        set_color_write_enable_08647             = "VUID-vkCmdDrawMeshTasksIndirectCountNV-None-08647";
        set_discard_rectangles_enable_08648      = "VUID-vkCmdDrawMeshTasksIndirectCountNV-None-08648";
        set_discard_rectangles_mode_08649      = "VUID-vkCmdDrawMeshTasksIndirectCountNV-None-08649";
        set_depth_clamp_enable_08650             = "VUID-vkCmdDrawMeshTasksIndirectCountNV-None-08650";
        set_polygon_mode_08651                   = "VUID-vkCmdDrawMeshTasksIndirectCountNV-None-08651";
        set_rasterization_samples_08652          = "VUID-vkCmdDrawMeshTasksIndirectCountNV-None-08652";
        set_sample_mask_08653                    = "VUID-vkCmdDrawMeshTasksIndirectCountNV-None-08653";
        set_alpha_to_coverage_enable_08654       = "VUID-vkCmdDrawMeshTasksIndirectCountNV-None-08654";
        set_alpha_to_one_enable_08655            = "VUID-vkCmdDrawMeshTasksIndirectCountNV-None-08655";
        set_logic_op_enable_08656                = "VUID-vkCmdDrawMeshTasksIndirectCountNV-None-08656";
        set_color_blend_enable_08657             = "VUID-vkCmdDrawMeshTasksIndirectCountNV-None-08657";
        set_color_blend_equation_08658           = "VUID-vkCmdDrawMeshTasksIndirectCountNV-None-08658";
        set_color_write_mask_08659               = "VUID-vkCmdDrawMeshTasksIndirectCountNV-None-08659";
        set_rasterization_streams_08660          = "VUID-vkCmdDrawMeshTasksIndirectCountNV-None-08660";
        set_conservative_rasterization_mode_08661 = "VUID-vkCmdDrawMeshTasksIndirectCountNV-None-08661";
        set_extra_primitive_overestimation_size_08662 = "VUID-vkCmdDrawMeshTasksIndirectCountNV-None-08662";
        set_depth_clip_enable_08663              = "VUID-vkCmdDrawMeshTasksIndirectCountNV-None-08663";
        set_sample_locations_enable_08664        = "VUID-vkCmdDrawMeshTasksIndirectCountNV-None-08664";
        set_provoking_vertex_mode_08665          = "VUID-vkCmdDrawMeshTasksIndirectCountNV-None-08665";
        set_line_rasterization_mode_08666        = "VUID-vkCmdDrawMeshTasksIndirectCountNV-None-08666";
        set_line_rasterization_mode_08667        = "VUID-vkCmdDrawMeshTasksIndirectCountNV-None-08667";
        set_line_rasterization_mode_08668        = "VUID-vkCmdDrawMeshTasksIndirectCountNV-None-08668";
        set_line_stipple_enable_08669            = "VUID-vkCmdDrawMeshTasksIndirectCountNV-None-08669";
        set_line_stipple_enable_08670            = "VUID-vkCmdDrawMeshTasksIndirectCountNV-None-08670";
        set_line_stipple_enable_08671            = "VUID-vkCmdDrawMeshTasksIndirectCountNV-None-08671";
        set_line_stipple_08672                   = "VUID-vkCmdDrawMeshTasksIndirectCountNV-None-08672";
        set_depth_clip_negative_one_to_one_08673 = "VUID-vkCmdDrawMeshTasksIndirectCountNV-None-08673";
        set_viewport_w_scaling_enable_08674      = "VUID-vkCmdDrawMeshTasksIndirectCountNV-None-08674";
        set_viewport_swizzle_08675               = "VUID-vkCmdDrawMeshTasksIndirectCountNV-None-08675";
        set_coverage_to_color_enable_08676       = "VUID-vkCmdDrawMeshTasksIndirectCountNV-None-08676";
        set_coverage_to_color_location_08677     = "VUID-vkCmdDrawMeshTasksIndirectCountNV-None-08677";
        set_coverage_modulation_mode_08678       = "VUID-vkCmdDrawMeshTasksIndirectCountNV-None-08678";
        set_coverage_modulation_table_enable_08679 = "VUID-vkCmdDrawMeshTasksIndirectCountNV-None-08679";
        set_coverage_modulation_table_08680      = "VUID-vkCmdDrawMeshTasksIndirectCountNV-None-08680";
        set_shading_rate_image_enable_08681      = "VUID-vkCmdDrawMeshTasksIndirectCountNV-None-08681";
        set_representative_fragment_test_enable_08682 = "VUID-vkCmdDrawMeshTasksIndirectCountNV-None-08682";
        set_coverage_reduction_mode_08683        = "VUID-vkCmdDrawMeshTasksIndirectCountNV-None-08683";
        vertex_shader_08684                      = "VUID-vkCmdDrawMeshTasksIndirectCountNV-None-08684";
        tessellation_control_shader_08685        = "VUID-vkCmdDrawMeshTasksIndirectCountNV-None-08685";
        tessellation_evaluation_shader_08686     = "VUID-vkCmdDrawMeshTasksIndirectCountNV-None-08686";
        geometry_shader_08687                    = "VUID-vkCmdDrawMeshTasksIndirectCountNV-None-08687";
        fragment_shader_08688                    = "VUID-vkCmdDrawMeshTasksIndirectCountNV-None-08688";
        task_shader_08689                        = "VUID-vkCmdDrawMeshTasksIndirectCountNV-None-08689";
        mesh_shader_08690                        = "VUID-vkCmdDrawMeshTasksIndirectCountNV-None-08690";
        vert_mesh_shader_08693                   = "VUID-vkCmdDrawMeshTasksIndirectCountNV-None-08693";
        task_mesh_shader_08694                   = "VUID-vkCmdDrawMeshTasksIndirectCountNV-None-08694";
        task_mesh_shader_08695                   = "VUID-vkCmdDrawMeshTasksIndirectCountNV-None-08695";
        vert_task_mesh_shader_08696              = "VUID-vkCmdDrawMeshTasksIndirectCountNV-None-08696";
        linked_shaders_08698                     = "VUID-vkCmdDrawMeshTasksIndirectCountNV-None-08698";
        linked_shaders_08699                     = "VUID-vkCmdDrawMeshTasksIndirectCountNV-None-08699";
        shaders_push_constants_08878             = "VUID-vkCmdDrawMeshTasksIndirectCountNV-None-08878";
        shaders_descriptor_layouts_08879         = "VUID-vkCmdDrawMeshTasksIndirectCountNV-None-08879";
        set_attachment_feedback_loop_enable_08880 = "VUID-vkCmdDrawMeshTasksIndirectCountNV-None-08880";
        set_line_width_08617                     = "VUID-vkCmdDrawMeshTasksIndirectCountNV-None-08617";
        set_line_width_08618                     = "VUID-vkCmdDrawMeshTasksIndirectCountNV-None-08618";
        set_depth_bias_08620                     = "VUID-vkCmdDrawMeshTasksIndirectCountNV-None-08620";
        set_blend_constants_08621                = "VUID-vkCmdDrawMeshTasksIndirectCountNV-None-08621";
        set_depth_bounds_08622                   = "VUID-vkCmdDrawMeshTasksIndirectCountNV-None-08622";
        set_stencil_compare_mask_08623           = "VUID-vkCmdDrawMeshTasksIndirectCountNV-None-08623";
        set_stencil_write_mask_08624             = "VUID-vkCmdDrawMeshTasksIndirectCountNV-None-08624";
        set_stencil_reference_08625              = "VUID-vkCmdDrawMeshTasksIndirectCountNV-None-08625";
        set_sample_locations_08626               = "VUID-vkCmdDrawMeshTasksIndirectCountNV-None-08626";
        set_cull_mode_08627                      = "VUID-vkCmdDrawMeshTasksIndirectCountNV-None-08627";
        set_front_face_08628                     = "VUID-vkCmdDrawMeshTasksIndirectCountNV-None-08628";
        set_depth_test_enable_08629              = "VUID-vkCmdDrawMeshTasksIndirectCountNV-None-08629";
        set_depth_write_enable_08630             = "VUID-vkCmdDrawMeshTasksIndirectCountNV-None-08630";
        set_depth_comapre_op_08631               = "VUID-vkCmdDrawMeshTasksIndirectCountNV-None-08631";
        set_depth_bounds_test_enable_08632       = "VUID-vkCmdDrawMeshTasksIndirectCountNV-None-08632";
        set_stencil_test_enable_08633            = "VUID-vkCmdDrawMeshTasksIndirectCountNV-None-08633";
        set_stencil_op_08634                     = "VUID-vkCmdDrawMeshTasksIndirectCountNV-None-08634";
        set_line_width_08619                     = "VUID-vkCmdDrawMeshTasksIndirectCountNV-None-08619";
        set_viewport_with_count_08642            = "VUID-vkCmdDrawMeshTasksIndirectCountNV-primitiveFragmentShadingRateWithMultipleViewports-08642";
        alpha_component_word_08920               = "VUID-vkCmdDrawMeshTasksIndirectCountNV-alphaToCoverageEnable-08920";
        color_write_mask_09116                   = "VUID-vkCmdDrawMeshTasksIndirectCountNV-None-09116";
        set_clip_space_w_scaling_09232           = "VUID-vkCmdDrawMeshTasksIndirectCountNV-None-09232";
        set_discard_rectangle_09236              = "VUID-vkCmdDrawMeshTasksIndirectCountNV-rasterizerDiscardEnable-09236";
        set_viewport_coarse_sample_order_09233   = "VUID-vkCmdDrawMeshTasksIndirectCountNV-shadingRateImage-09233";
        set_viewport_shading_rate_palette_09234  = "VUID-vkCmdDrawMeshTasksIndirectCountNV-shadingRateImage-09234";
        set_exclusive_scissor_enable_09235       = "VUID-vkCmdDrawMeshTasksIndirectCountNV-exclusiveScissor-09235";
        set_fragment_shading_rate_09238          = "VUID-vkCmdDrawMeshTasksIndirectCountNV-pipelineFragmentShadingRate-09238";
        set_tessellation_domain_origin_09237     = "VUID-vkCmdDrawMeshTasksIndirectCountNV-None-09237";
        rasterization_samples_07935              = "VUID-vkCmdDrawMeshTasksIndirectCountNV-pNext-07935";
        mesh_shader_queries_07073                = "VUID-vkCmdDrawMeshTasksIndirectCountNV-stage-07073";
        blend_advanced_07480                     = "VUID-vkCmdDrawMeshTasksIndirectCountNV-advancedBlendMaxColorAttachments-07480";
        blend_feature_07470                      = "VUID-vkCmdDrawMeshTasksIndirectCountNV-pColorBlendEnables-07470";
        color_attachment_08963                   = "VUID-vkCmdDrawMeshTasksIndirectCountNV-pColorAttachments-08963";
        depth_attachment_08964                   = "VUID-vkCmdDrawMeshTasksIndirectCountNV-pDepthAttachment-08964";
        stencil_attachment_08965                 = "VUID-vkCmdDrawMeshTasksIndirectCountNV-pStencilAttachment-08965";
        sample_locations_07482                   = "VUID-vkCmdDrawMeshTasksIndirectCountNV-sampleLocationsPerPixel-07482";
    }
};

struct DispatchVuidsCmdDrawMeshTasksEXT: DrawDispatchVuid {
    DispatchVuidsCmdDrawMeshTasksEXT() : DrawDispatchVuid() {
        pipeline_bound_08606                     = "VUID-vkCmdDrawMeshTasksEXT-None-08606";
        pipeline_or_shaders_bound_08607          = "VUID-vkCmdDrawMeshTasksEXT-None-08607";
        compatible_pipeline_08600                = "VUID-vkCmdDrawMeshTasksEXT-None-08600";
        render_pass_compatible_02684             = "VUID-vkCmdDrawMeshTasksEXT-renderPass-02684";
        render_pass_began_08876                  = "VUID-vkCmdDrawMeshTasksEXT-None-08876";
        subpass_index_02685                      = "VUID-vkCmdDrawMeshTasksEXT-subpass-02685";
        sample_location_02689                    = "VUID-vkCmdDrawMeshTasksEXT-sampleLocationsEnable-02689";
        linear_filter_sampler_04553              = "VUID-vkCmdDrawMeshTasksEXT-magFilter-04553";
        linear_mipmap_sampler_04770              = "VUID-vkCmdDrawMeshTasksEXT-mipmapMode-04770";
        cubic_sampler_02692                      = "VUID-vkCmdDrawMeshTasksEXT-None-02692";
        viewport_count_03417                     = "VUID-vkCmdDrawMeshTasksEXT-viewportCount-03417";
        scissor_count_03418                      = "VUID-vkCmdDrawMeshTasksEXT-scissorCount-03418";
        viewport_scissor_count_03419             = "VUID-vkCmdDrawMeshTasksEXT-viewportCount-03419";
        corner_sampled_address_mode_02696        = "VUID-vkCmdDrawMeshTasksEXT-flags-02696";
        imageview_atomic_02691                   = "VUID-vkCmdDrawMeshTasksEXT-None-02691";
        bufferview_atomic_07888                  = "VUID-vkCmdDrawMeshTasksEXT-None-07888";
        push_constants_set_08602                 = "VUID-vkCmdDrawMeshTasksEXT-maintenance4-08602";
        image_subresources_render_pass_write_06537 = "VUID-vkCmdDrawMeshTasksEXT-None-06537";
        image_subresources_subpass_read_09003    = "VUID-vkCmdDrawMeshTasksEXT-None-09003";
        image_subresources_subpass_write_06539   = "VUID-vkCmdDrawMeshTasksEXT-None-06539";
        sampler_imageview_type_08609             = "VUID-vkCmdDrawMeshTasksEXT-None-08609";
        sampler_implicitLod_dref_proj_08610      = "VUID-vkCmdDrawMeshTasksEXT-None-08610";
        sampler_bias_offset_08611                = "VUID-vkCmdDrawMeshTasksEXT-None-08611";
        dynamic_state_setting_commands_08608     = "VUID-vkCmdDrawMeshTasksEXT-None-08608";
        msrtss_rasterization_samples_07284       = "VUID-vkCmdDrawMeshTasksEXT-multisampledRenderToSingleSampled-07284";
        unprotected_command_buffer_02707         = "VUID-vkCmdDrawMeshTasksEXT-commandBuffer-02707";
        max_multiview_instance_index_02688       = "VUID-vkCmdDrawMeshTasksEXT-maxMultiviewInstanceIndex-02688";
        img_filter_cubic_02693                   = "VUID-vkCmdDrawMeshTasksEXT-None-02693";
        filter_cubic_02694                       = "VUID-vkCmdDrawMeshTasksEXT-filterCubic-02694";
        filter_cubic_min_max_02695               = "VUID-vkCmdDrawMeshTasksEXT-filterCubicMinmax-02695";
        viewport_count_primitive_shading_rate_04552 = "VUID-vkCmdDrawMeshTasksEXT-primitiveFragmentShadingRateWithMultipleViewports-04552";
        rasterizer_discard_enable_04876          = "VUID-vkCmdDrawMeshTasksEXT-None-04876";
        depth_bias_enable_04877                  = "VUID-vkCmdDrawMeshTasksEXT-None-04877";
        logic_op_04878                           = "VUID-vkCmdDrawMeshTasksEXT-logicOp-04878";
        blend_enable_04727                       = "VUID-vkCmdDrawMeshTasksEXT-blendEnable-04727";
        dynamic_discard_rectangle_07751          = "VUID-vkCmdDrawMeshTasksEXT-None-07751";
        dynamic_discard_rectangle_enable_07880   = "VUID-vkCmdDrawMeshTasksEXT-None-07880";
        dynamic_discard_rectangle_mode_07881     =  "VUID-vkCmdDrawMeshTasksEXT-None-07881";
        dynamic_exclusive_scissor_enable_07878   = "VUID-vkCmdDrawMeshTasksEXT-None-07878";
        dynamic_exclusive_scissor_07879          = "VUID-vkCmdDrawMeshTasksEXT-None-07879";
        dynamic_color_write_enable_07749         = "VUID-vkCmdDrawMeshTasksEXT-None-07749";
        dynamic_color_write_enable_count_07750   = "VUID-vkCmdDrawMeshTasksEXT-attachmentCount-07750";
        dynamic_attachment_feedback_loop_08877   = "VUID-vkCmdDrawMeshTasksEXT-None-08877";
        dynamic_rendering_view_mask_06178        = "VUID-vkCmdDrawMeshTasksEXT-viewMask-06178";
        dynamic_rendering_color_count_06179      = "VUID-vkCmdDrawMeshTasksEXT-colorAttachmentCount-06179";
        dynamic_rendering_color_formats_08910    = "VUID-vkCmdDrawMeshTasksEXT-dynamicRenderingUnusedAttachments-08910";
        dynamic_rendering_unused_attachments_08911 = "VUID-vkCmdDrawMeshTasksEXT-dynamicRenderingUnusedAttachments-08911";
        dynamic_rendering_undefined_color_formats_08912 = "VUID-vkCmdDrawMeshTasksEXT-dynamicRenderingUnusedAttachments-08912";
        dynamic_rendering_undefined_depth_format_08916 = "VUID-vkCmdDrawMeshTasksEXT-dynamicRenderingUnusedAttachments-08916";
        dynamic_rendering_undefined_stencil_format_08916 = "VUID-vkCmdDrawMeshTasksEXT-dynamicRenderingUnusedAttachments-08916";
        dynamic_rendering_depth_format_08914     = "VUID-vkCmdDrawMeshTasksEXT-dynamicRenderingUnusedAttachments-08914";
        dynamic_rendering_unused_attachments_08915 = "VUID-vkCmdDrawMeshTasksEXT-dynamicRenderingUnusedAttachments-08915";
        dynamic_rendering_stencil_format_08917   = "VUID-vkCmdDrawMeshTasksEXT-dynamicRenderingUnusedAttachments-08917";
        dynamic_rendering_unused_attachments_08918 = "VUID-vkCmdDrawMeshTasksEXT-dynamicRenderingUnusedAttachments-08918";
        dynamic_rendering_fsr_06183              = "VUID-vkCmdDrawMeshTasksEXT-imageView-06183";
        dynamic_rendering_fdm_06184              = "VUID-vkCmdDrawMeshTasksEXT-imageView-06184";
        dynamic_rendering_color_sample_06185     = "VUID-vkCmdDrawMeshTasksEXT-colorAttachmentCount-06185";
        dynamic_rendering_depth_sample_06186     = "VUID-vkCmdDrawMeshTasksEXT-pDepthAttachment-06186";
        dynamic_rendering_stencil_sample_06187   = "VUID-vkCmdDrawMeshTasksEXT-pStencilAttachment-06187";
        dynamic_rendering_06198                  = "VUID-vkCmdDrawMeshTasksEXT-renderPass-06198";
        dynamic_rendering_07285                  = "VUID-vkCmdDrawMeshTasksEXT-multisampledRenderToSingleSampled-07285";
        dynamic_rendering_07286                  = "VUID-vkCmdDrawMeshTasksEXT-multisampledRenderToSingleSampled-07286";
        dynamic_rendering_07287                  = "VUID-vkCmdDrawMeshTasksEXT-multisampledRenderToSingleSampled-07287";
        image_view_access_64_04470               = "VUID-vkCmdDrawMeshTasksEXT-SampledType-04470";
        image_view_access_32_04471               = "VUID-vkCmdDrawMeshTasksEXT-SampledType-04471";
        image_view_sparse_64_04474               = "VUID-vkCmdDrawMeshTasksEXT-sparseImageInt64Atomics-04474";
        buffer_view_access_64_04472              = "VUID-vkCmdDrawMeshTasksEXT-SampledType-04472";
        buffer_view_access_32_04473              = "VUID-vkCmdDrawMeshTasksEXT-SampledType-04473";
        storage_image_read_without_format_07028  = "VUID-vkCmdDrawMeshTasksEXT-OpTypeImage-07028";
        storage_image_write_without_format_07027 = "VUID-vkCmdDrawMeshTasksEXT-OpTypeImage-07027";
        storage_texel_buffer_read_without_format_07030  = "VUID-vkCmdDrawMeshTasksEXT-OpTypeImage-07030";
        storage_texel_buffer_write_without_format_07029 = "VUID-vkCmdDrawMeshTasksEXT-OpTypeImage-07029";
        storage_image_write_texel_count_08795           = "VUID-vkCmdDrawMeshTasksEXT-OpImageWrite-08795";
        storage_image_write_texel_count_08796           = "VUID-vkCmdDrawMeshTasksEXT-OpImageWrite-08796";
        storage_texel_buffer_write_texel_count_04469    = "VUID-vkCmdDrawMeshTasksEXT-OpImageWrite-04469";
        depth_compare_sample_06479               = "VUID-vkCmdDrawMeshTasksEXT-None-06479";
        depth_read_only_06886                    = "VUID-vkCmdDrawMeshTasksEXT-None-06886";
        stencil_read_only_06887                  = "VUID-vkCmdDrawMeshTasksEXT-None-06887";
        dynamic_sample_locations_06666           = "VUID-vkCmdDrawMeshTasksEXT-None-06666";
        dynamic_tessellation_domain_origin_07619 = "VUID-vkCmdDrawMeshTasksEXT-None-07619";
        dynamic_depth_clamp_enable_07620         = "VUID-vkCmdDrawMeshTasksEXT-None-07620";
        dynamic_polygon_mode_07621               = "VUID-vkCmdDrawMeshTasksEXT-None-07621";
        dynamic_rasterization_samples_07622      = "VUID-vkCmdDrawMeshTasksEXT-None-07622";
        dynamic_sample_mask_07623                = "VUID-vkCmdDrawMeshTasksEXT-None-07623";
        dynamic_alpha_to_coverage_enable_07624   = "VUID-vkCmdDrawMeshTasksEXT-None-07624";
        dynamic_alpha_to_coverage_component_08919 = "VUID-vkCmdDrawMeshTasksEXT-alphaToCoverageEnable-08919";
        dynamic_alpha_to_one_enable_07625        = "VUID-vkCmdDrawMeshTasksEXT-None-07625";
        dynamic_logic_op_enable_07626            = "VUID-vkCmdDrawMeshTasksEXT-None-07626";
        dynamic_color_blend_enable_07476         = "VUID-vkCmdDrawMeshTasksEXT-firstAttachment-07476";
        dynamic_color_blend_equation_07477       = "VUID-vkCmdDrawMeshTasksEXT-firstAttachment-07477";
        dynamic_color_write_mask_07478           = "VUID-vkCmdDrawMeshTasksEXT-firstAttachment-07478";
        dynamic_rasterization_stream_07630       = "VUID-vkCmdDrawMeshTasksEXT-None-07630";
        dynamic_conservative_rasterization_mode_07631 = "VUID-vkCmdDrawMeshTasksEXT-None-07631";
        dynamic_extra_primitive_overestimation_size_07632 = "VUID-vkCmdDrawMeshTasksEXT-None-07632";
        dynamic_depth_clip_enable_07633          = "VUID-vkCmdDrawMeshTasksEXT-None-07633";
        dynamic_sample_locations_enable_07634    = "VUID-vkCmdDrawMeshTasksEXT-None-07634";
        dynamic_color_blend_advanced_07479       = "VUID-vkCmdDrawMeshTasksEXT-firstAttachment-07479";
        dynamic_provoking_vertex_mode_07636      = "VUID-vkCmdDrawMeshTasksEXT-None-07636";
        dynamic_line_rasterization_mode_07637    = "VUID-vkCmdDrawMeshTasksEXT-None-07637";
        dynamic_line_stipple_enable_07638        = "VUID-vkCmdDrawMeshTasksEXT-None-07638";
        dynamic_depth_clip_negative_one_to_one_07639 = "VUID-vkCmdDrawMeshTasksEXT-None-07639";
        dynamic_viewport_w_scaling_enable_07640  = "VUID-vkCmdDrawMeshTasksEXT-None-07640";
        dynamic_viewport_swizzle_07641           = "VUID-vkCmdDrawMeshTasksEXT-None-07641";
        dynamic_coverage_to_color_enable_07642   = "VUID-vkCmdDrawMeshTasksEXT-None-07642";
        dynamic_coverage_to_color_location_07643 = "VUID-vkCmdDrawMeshTasksEXT-None-07643";
        dynamic_coverage_modulation_mode_07644   = "VUID-vkCmdDrawMeshTasksEXT-None-07644";
        dynamic_coverage_modulation_table_enable_07645 = "VUID-vkCmdDrawMeshTasksEXT-None-07645";
        dynamic_coverage_modulation_table_07646  = "VUID-vkCmdDrawMeshTasksEXT-None-07646";
        dynamic_shading_rate_image_enable_07647  = "VUID-vkCmdDrawMeshTasksEXT-None-07647";
        dynamic_representative_fragment_test_enable_07648  = "VUID-vkCmdDrawMeshTasksEXT-None-07648";
        dynamic_coverage_reduction_mode_07649    = "VUID-vkCmdDrawMeshTasksEXT-None-07649";
        dynamic_viewport_07831                   = "VUID-vkCmdDrawMeshTasksEXT-None-07831";
        dynamic_scissor_07832                    = "VUID-vkCmdDrawMeshTasksEXT-None-07832";
        dynamic_depth_bias_07834                 = "VUID-vkCmdDrawMeshTasksEXT-None-07834";
        dynamic_line_width_07833                 = "VUID-vkCmdDrawMeshTasksEXT-None-07833";
        dynamic_line_stipple_ext_07849           = "VUID-vkCmdDrawMeshTasksEXT-None-07849";
        dynamic_blend_constants_07835            = "VUID-vkCmdDrawMeshTasksEXT-None-07835";
        dynamic_depth_bounds_07836               = "VUID-vkCmdDrawMeshTasksEXT-None-07836";
        dynamic_depth_enable_08715               = "VUID-vkCmdDrawMeshTasksEXT-pDynamicStates-08715";
        dynamic_stencil_compare_mask_07837       = "VUID-vkCmdDrawMeshTasksEXT-None-07837";
        dynamic_stencil_write_mask_07838         = "VUID-vkCmdDrawMeshTasksEXT-None-07838";
        dynamic_stencil_write_mask_08716         = "VUID-vkCmdDrawMeshTasksEXT-pDynamicStates-08716";
        dynamic_stencil_reference_07839          = "VUID-vkCmdDrawMeshTasksEXT-None-07839";
        dynamic_state_inherited_07850            = "VUID-vkCmdDrawMeshTasksEXT-None-07850";
        dynamic_cull_mode_07840                  = "VUID-vkCmdDrawMeshTasksEXT-None-07840";
        dynamic_front_face_07841                 = "VUID-vkCmdDrawMeshTasksEXT-None-07841";
        dynamic_depth_test_enable_07843          = "VUID-vkCmdDrawMeshTasksEXT-None-07843";
        dynamic_depth_write_enable_07844         = "VUID-vkCmdDrawMeshTasksEXT-None-07844";
        dynamic_depth_compare_op_07845           = "VUID-vkCmdDrawMeshTasksEXT-None-07845";
        dynamic_depth_bound_test_enable_07846    = "VUID-vkCmdDrawMeshTasksEXT-None-07846";
        dynamic_stencil_test_enable_07847        = "VUID-vkCmdDrawMeshTasksEXT-None-07847";
        dynamic_stencil_op_07848                 = "VUID-vkCmdDrawMeshTasksEXT-None-07848";
        primitives_generated_06708               = "VUID-vkCmdDrawMeshTasksEXT-primitivesGeneratedQueryWithRasterizerDiscard-06708";
        primitives_generated_streams_06709       = "VUID-vkCmdDrawMeshTasksEXT-primitivesGeneratedQueryWithNonZeroStreams-06709";
        mesh_shader_stages_06480                 = "VUID-vkCmdDrawMeshTasksEXT-stage-06480";
        missing_mesh_shader_stages_07080         = "VUID-vkCmdDrawMeshTasksEXT-MeshEXT-07087";
        descriptor_buffer_bit_set_08114          = "VUID-vkCmdDrawMeshTasksEXT-None-08114";
        descriptor_buffer_bit_not_set_08115      = "VUID-vkCmdDrawMeshTasksEXT-None-08115";
        descriptor_buffer_set_offset_missing_08117 = "VUID-vkCmdDrawMeshTasksEXT-None-08117";
        image_view_dim_07752                     = "VUID-vkCmdDrawMeshTasksEXT-viewType-07752";
        image_view_numeric_format_07753          = "VUID-vkCmdDrawMeshTasksEXT-format-07753";
        stippled_rectangular_lines_07495         = "VUID-vkCmdDrawMeshTasksEXT-stippledLineEnable-07495";
        stippled_bresenham_lines_07496           = "VUID-vkCmdDrawMeshTasksEXT-stippledLineEnable-07496";
        stippled_smooth_lines_07497              = "VUID-vkCmdDrawMeshTasksEXT-stippledLineEnable-07497";
        stippled_default_strict_07498            = "VUID-vkCmdDrawMeshTasksEXT-stippledLineEnable-07498";
        viewport_and_scissor_with_count_08635    = "VUID-vkCmdDrawMeshTasksEXT-None-08635";
        viewport_w_scaling_08636                 = "VUID-vkCmdDrawMeshTasksEXT-None-08636";
        shading_rate_palette_08637               = "VUID-vkCmdDrawMeshTasksEXT-None-08637";
        exclusive_scissor_08638                  = "VUID-vkCmdDrawMeshTasksEXT-None-08638";
        external_format_resolve_09362            = "VUID-vkCmdDrawMeshTasksEXT-colorAttachmentCount-09362";
        external_format_resolve_09363            = "VUID-vkCmdDrawMeshTasksEXT-None-09363";
        external_format_resolve_09364            = "VUID-vkCmdDrawMeshTasksEXT-None-09364";
        external_format_resolve_09365            = "VUID-vkCmdDrawMeshTasksEXT-None-09365";
        external_format_resolve_09368            = "VUID-vkCmdDrawMeshTasksEXT-None-09368";
        external_format_resolve_09369            = "VUID-vkCmdDrawMeshTasksEXT-None-09369";
        external_format_resolve_09372            = "VUID-vkCmdDrawMeshTasksEXT-colorAttachmentCount-09372";
        set_rasterizer_discard_enable_08639      = "VUID-vkCmdDrawMeshTasksEXT-None-08639";
        set_depth_bias_enable_08640              = "VUID-vkCmdDrawMeshTasksEXT-None-08640";
        set_logic_op_08641                       = "VUID-vkCmdDrawMeshTasksEXT-None-08641";
        set_color_blend_enable_08643             = "VUID-vkCmdDrawMeshTasksEXT-None-08643";
        set_rasterization_samples_08644              = "VUID-vkCmdDrawMeshTasksEXT-None-08644";
        set_color_write_enable_08646             = "VUID-vkCmdDrawMeshTasksEXT-None-08646";
        set_color_write_enable_08647             = "VUID-vkCmdDrawMeshTasksEXT-None-08647";
        set_discard_rectangles_enable_08648      = "VUID-vkCmdDrawMeshTasksEXT-None-08648";
        set_discard_rectangles_mode_08649      = "VUID-vkCmdDrawMeshTasksEXT-None-08649";
        set_depth_clamp_enable_08650             = "VUID-vkCmdDrawMeshTasksEXT-None-08650";
        set_polygon_mode_08651                   = "VUID-vkCmdDrawMeshTasksEXT-None-08651";
        set_rasterization_samples_08652          = "VUID-vkCmdDrawMeshTasksEXT-None-08652";
        set_sample_mask_08653                    = "VUID-vkCmdDrawMeshTasksEXT-None-08653";
        set_alpha_to_coverage_enable_08654       = "VUID-vkCmdDrawMeshTasksEXT-None-08654";
        set_alpha_to_one_enable_08655            = "VUID-vkCmdDrawMeshTasksEXT-None-08655";
        set_logic_op_enable_08656                = "VUID-vkCmdDrawMeshTasksEXT-None-08656";
        set_color_blend_enable_08657             = "VUID-vkCmdDrawMeshTasksEXT-None-08657";
        set_color_blend_equation_08658           = "VUID-vkCmdDrawMeshTasksEXT-None-08658";
        set_color_write_mask_08659               = "VUID-vkCmdDrawMeshTasksEXT-None-08659";
        set_rasterization_streams_08660          = "VUID-vkCmdDrawMeshTasksEXT-None-08660";
        set_conservative_rasterization_mode_08661 = "VUID-vkCmdDrawMeshTasksEXT-None-08661";
        set_extra_primitive_overestimation_size_08662 = "VUID-vkCmdDrawMeshTasksEXT-None-08662";
        set_depth_clip_enable_08663              = "VUID-vkCmdDrawMeshTasksEXT-None-08663";
        set_sample_locations_enable_08664        = "VUID-vkCmdDrawMeshTasksEXT-None-08664";
        set_provoking_vertex_mode_08665          = "VUID-vkCmdDrawMeshTasksEXT-None-08665";
        set_line_rasterization_mode_08666        = "VUID-vkCmdDrawMeshTasksEXT-None-08666";
        set_line_rasterization_mode_08667        = "VUID-vkCmdDrawMeshTasksEXT-None-08667";
        set_line_rasterization_mode_08668        = "VUID-vkCmdDrawMeshTasksEXT-None-08668";
        set_line_stipple_enable_08669            = "VUID-vkCmdDrawMeshTasksEXT-None-08669";
        set_line_stipple_enable_08670            = "VUID-vkCmdDrawMeshTasksEXT-None-08670";
        set_line_stipple_enable_08671            = "VUID-vkCmdDrawMeshTasksEXT-None-08671";
        set_line_stipple_08672                   = "VUID-vkCmdDrawMeshTasksEXT-None-08672";
        set_depth_clip_negative_one_to_one_08673 = "VUID-vkCmdDrawMeshTasksEXT-None-08673";
        set_viewport_w_scaling_enable_08674      = "VUID-vkCmdDrawMeshTasksEXT-None-08674";
        set_viewport_swizzle_08675               = "VUID-vkCmdDrawMeshTasksEXT-None-08675";
        set_coverage_to_color_enable_08676       = "VUID-vkCmdDrawMeshTasksEXT-None-08676";
        set_coverage_to_color_location_08677     = "VUID-vkCmdDrawMeshTasksEXT-None-08677";
        set_coverage_modulation_mode_08678       = "VUID-vkCmdDrawMeshTasksEXT-None-08678";
        set_coverage_modulation_table_enable_08679 = "VUID-vkCmdDrawMeshTasksEXT-None-08679";
        set_coverage_modulation_table_08680      = "VUID-vkCmdDrawMeshTasksEXT-None-08680";
        set_shading_rate_image_enable_08681      = "VUID-vkCmdDrawMeshTasksEXT-None-08681";
        set_representative_fragment_test_enable_08682 = "VUID-vkCmdDrawMeshTasksEXT-None-08682";
        set_coverage_reduction_mode_08683        = "VUID-vkCmdDrawMeshTasksEXT-None-08683";
        vertex_shader_08684                      = "VUID-vkCmdDrawMeshTasksEXT-None-08684";
        tessellation_control_shader_08685        = "VUID-vkCmdDrawMeshTasksEXT-None-08685";
        tessellation_evaluation_shader_08686     = "VUID-vkCmdDrawMeshTasksEXT-None-08686";
        geometry_shader_08687                    = "VUID-vkCmdDrawMeshTasksEXT-None-08687";
        fragment_shader_08688                    = "VUID-vkCmdDrawMeshTasksEXT-None-08688";
        task_shader_08689                        = "VUID-vkCmdDrawMeshTasksEXT-None-08689";
        mesh_shader_08690                        = "VUID-vkCmdDrawMeshTasksEXT-None-08690";
        vert_mesh_shader_08693                   = "VUID-vkCmdDrawMeshTasksEXT-None-08693";
        task_mesh_shader_08694                   = "VUID-vkCmdDrawMeshTasksEXT-None-08694";
        task_mesh_shader_08695                   = "VUID-vkCmdDrawMeshTasksEXT-None-08695";
        vert_task_mesh_shader_08696              = "VUID-vkCmdDrawMeshTasksEXT-None-08696";
        linked_shaders_08698                     = "VUID-vkCmdDrawMeshTasksEXT-None-08698";
        linked_shaders_08699                     = "VUID-vkCmdDrawMeshTasksEXT-None-08699";
        shaders_push_constants_08878             = "VUID-vkCmdDrawMeshTasksEXT-None-08878";
        shaders_descriptor_layouts_08879         = "VUID-vkCmdDrawMeshTasksEXT-None-08879";
        set_attachment_feedback_loop_enable_08880 = "VUID-vkCmdDrawMeshTasksEXT-None-08880";
        set_line_width_08617                     = "VUID-vkCmdDrawMeshTasksEXT-None-08617";
        set_line_width_08618                     = "VUID-vkCmdDrawMeshTasksEXT-None-08618";
        set_depth_bias_08620                     = "VUID-vkCmdDrawMeshTasksEXT-None-08620";
        set_blend_constants_08621                = "VUID-vkCmdDrawMeshTasksEXT-None-08621";
        set_depth_bounds_08622                   = "VUID-vkCmdDrawMeshTasksEXT-None-08622";
        set_stencil_compare_mask_08623           = "VUID-vkCmdDrawMeshTasksEXT-None-08623";
        set_stencil_write_mask_08624             = "VUID-vkCmdDrawMeshTasksEXT-None-08624";
        set_stencil_reference_08625              = "VUID-vkCmdDrawMeshTasksEXT-None-08625";
        set_sample_locations_08626               = "VUID-vkCmdDrawMeshTasksEXT-None-08626";
        set_cull_mode_08627                      = "VUID-vkCmdDrawMeshTasksEXT-None-08627";
        set_front_face_08628                     = "VUID-vkCmdDrawMeshTasksEXT-None-08628";
        set_depth_test_enable_08629              = "VUID-vkCmdDrawMeshTasksEXT-None-08629";
        set_depth_write_enable_08630             = "VUID-vkCmdDrawMeshTasksEXT-None-08630";
        set_depth_comapre_op_08631               = "VUID-vkCmdDrawMeshTasksEXT-None-08631";
        set_depth_bounds_test_enable_08632       = "VUID-vkCmdDrawMeshTasksEXT-None-08632";
        set_stencil_test_enable_08633            = "VUID-vkCmdDrawMeshTasksEXT-None-08633";
        set_stencil_op_08634                     = "VUID-vkCmdDrawMeshTasksEXT-None-08634";
        set_line_width_08619                     = "VUID-vkCmdDrawMeshTasksEXT-None-08619";
        set_viewport_with_count_08642            = "VUID-vkCmdDrawMeshTasksEXT-primitiveFragmentShadingRateWithMultipleViewports-08642";
        alpha_component_word_08920               = "VUID-vkCmdDrawMeshTasksEXT-alphaToCoverageEnable-08920";
        color_write_mask_09116                   = "VUID-vkCmdDrawMeshTasksEXT-None-09116";
        set_clip_space_w_scaling_09232           = "VUID-vkCmdDrawMeshTasksEXT-None-09232";
        set_discard_rectangle_09236              = "VUID-vkCmdDrawMeshTasksEXT-rasterizerDiscardEnable-09236";
        set_viewport_coarse_sample_order_09233   = "VUID-vkCmdDrawMeshTasksEXT-shadingRateImage-09233";
        set_viewport_shading_rate_palette_09234  = "VUID-vkCmdDrawMeshTasksEXT-shadingRateImage-09234";
        set_exclusive_scissor_enable_09235       = "VUID-vkCmdDrawMeshTasksEXT-exclusiveScissor-09235";
        set_fragment_shading_rate_09238          = "VUID-vkCmdDrawMeshTasksEXT-pipelineFragmentShadingRate-09238";
        set_tessellation_domain_origin_09237     = "VUID-vkCmdDrawMeshTasksEXT-None-09237";
        rasterization_samples_07935              = "VUID-vkCmdDrawMeshTasksEXT-pNext-07935";
        mesh_shader_queries_07073                = "VUID-vkCmdDrawMeshTasksEXT-stage-07073";
        blend_advanced_07480                     = "VUID-vkCmdDrawMeshTasksEXT-advancedBlendMaxColorAttachments-07480";
        blend_feature_07470                      = "VUID-vkCmdDrawMeshTasksEXT-pColorBlendEnables-07470";
        color_attachment_08963                   = "VUID-vkCmdDrawMeshTasksEXT-pColorAttachments-08963";
        depth_attachment_08964                   = "VUID-vkCmdDrawMeshTasksEXT-pDepthAttachment-08964";
        stencil_attachment_08965                 = "VUID-vkCmdDrawMeshTasksEXT-pStencilAttachment-08965";
        sample_locations_07482                   = "VUID-vkCmdDrawMeshTasksEXT-sampleLocationsPerPixel-07482";
    }
};

struct DispatchVuidsCmdDrawMeshTasksIndirectEXT: DrawDispatchVuid {
    DispatchVuidsCmdDrawMeshTasksIndirectEXT() : DrawDispatchVuid() {
        pipeline_bound_08606                     = "VUID-vkCmdDrawMeshTasksIndirectEXT-None-08606";
        pipeline_or_shaders_bound_08607          = "VUID-vkCmdDrawMeshTasksIndirectEXT-None-08607";
        compatible_pipeline_08600                = "VUID-vkCmdDrawMeshTasksIndirectEXT-None-08600";
        render_pass_compatible_02684             = "VUID-vkCmdDrawMeshTasksIndirectEXT-renderPass-02684";
        render_pass_began_08876                  = "VUID-vkCmdDrawMeshTasksIndirectEXT-None-08876";
        subpass_index_02685                      = "VUID-vkCmdDrawMeshTasksIndirectEXT-subpass-02685";
        sample_location_02689                    = "VUID-vkCmdDrawMeshTasksIndirectEXT-sampleLocationsEnable-02689";
        linear_filter_sampler_04553              = "VUID-vkCmdDrawMeshTasksIndirectEXT-magFilter-04553";
        linear_mipmap_sampler_04770              = "VUID-vkCmdDrawMeshTasksIndirectEXT-mipmapMode-04770";
        cubic_sampler_02692                      = "VUID-vkCmdDrawMeshTasksIndirectEXT-None-02692";
        indirect_protected_cb_02646              = "VUID-vkCmdDrawMeshTasksIndirectEXT-commandBuffer-02711";
        indirect_contiguous_memory_02708         = "VUID-vkCmdDrawMeshTasksIndirectEXT-buffer-02708";
        indirect_buffer_bit_02290                = "VUID-vkCmdDrawMeshTasksIndirectEXT-buffer-02709";
        viewport_count_03417                     = "VUID-vkCmdDrawMeshTasksIndirectEXT-viewportCount-03417";
        scissor_count_03418                      = "VUID-vkCmdDrawMeshTasksIndirectEXT-scissorCount-03418";
        viewport_scissor_count_03419             = "VUID-vkCmdDrawMeshTasksIndirectEXT-viewportCount-03419";
        corner_sampled_address_mode_02696        = "VUID-vkCmdDrawMeshTasksIndirectEXT-flags-02696";
        imageview_atomic_02691                   = "VUID-vkCmdDrawMeshTasksIndirectEXT-None-02691";
        bufferview_atomic_07888                  = "VUID-vkCmdDrawMeshTasksIndirectEXT-None-07888";
        push_constants_set_08602                 = "VUID-vkCmdDrawMeshTasksIndirectEXT-maintenance4-08602";
        image_subresources_render_pass_write_06537 = "VUID-vkCmdDrawMeshTasksIndirectEXT-None-06537";
        image_subresources_subpass_read_09003    = "VUID-vkCmdDrawMeshTasksIndirectEXT-None-09003";
        image_subresources_subpass_write_06539   = "VUID-vkCmdDrawMeshTasksIndirectEXT-None-06539";
        sampler_imageview_type_08609             = "VUID-vkCmdDrawMeshTasksIndirectEXT-None-08609";
        sampler_implicitLod_dref_proj_08610      = "VUID-vkCmdDrawMeshTasksIndirectEXT-None-08610";
        sampler_bias_offset_08611                = "VUID-vkCmdDrawMeshTasksIndirectEXT-None-08611";
        dynamic_state_setting_commands_08608     = "VUID-vkCmdDrawMeshTasksIndirectEXT-None-08608";
        msrtss_rasterization_samples_07284       = "VUID-vkCmdDrawMeshTasksIndirectEXT-multisampledRenderToSingleSampled-07284";
        unprotected_command_buffer_02707         = "VUID-vkCmdDrawMeshTasksIndirectEXT-commandBuffer-02707";
        max_multiview_instance_index_02688       = "VUID-vkCmdDrawMeshTasksIndirectEXT-maxMultiviewInstanceIndex-02688";
        img_filter_cubic_02693                   = "VUID-vkCmdDrawMeshTasksIndirectEXT-None-02693";
        filter_cubic_02694                       = "VUID-vkCmdDrawMeshTasksIndirectEXT-filterCubic-02694";
        filter_cubic_min_max_02695               = "VUID-vkCmdDrawMeshTasksIndirectEXT-filterCubicMinmax-02695";
        viewport_count_primitive_shading_rate_04552 = "VUID-vkCmdDrawMeshTasksIndirectEXT-primitiveFragmentShadingRateWithMultipleViewports-04552";
        rasterizer_discard_enable_04876          = "VUID-vkCmdDrawMeshTasksIndirectEXT-None-04876";
        depth_bias_enable_04877                  = "VUID-vkCmdDrawMeshTasksIndirectEXT-None-04877";
        logic_op_04878                           = "VUID-vkCmdDrawMeshTasksIndirectEXT-logicOp-04878";
        blend_enable_04727                       = "VUID-vkCmdDrawMeshTasksIndirectEXT-blendEnable-04727";
        dynamic_discard_rectangle_07751          = "VUID-vkCmdDrawMeshTasksIndirectEXT-None-07751";
        dynamic_discard_rectangle_enable_07880   = "VUID-vkCmdDrawMeshTasksIndirectEXT-None-07880";
        dynamic_discard_rectangle_mode_07881     =  "VUID-vkCmdDrawMeshTasksIndirectEXT-None-07881";
        dynamic_exclusive_scissor_enable_07878   = "VUID-vkCmdDrawMeshTasksIndirectEXT-None-07878";
        dynamic_exclusive_scissor_07879          = "VUID-vkCmdDrawMeshTasksIndirectEXT-None-07879";
        dynamic_color_write_enable_07749         = "VUID-vkCmdDrawMeshTasksIndirectEXT-None-07749";
        dynamic_color_write_enable_count_07750   = "VUID-vkCmdDrawMeshTasksIndirectEXT-attachmentCount-07750";
        dynamic_attachment_feedback_loop_08877   = "VUID-vkCmdDrawMeshTasksIndirectEXT-None-08877";
        dynamic_rendering_view_mask_06178        = "VUID-vkCmdDrawMeshTasksIndirectEXT-viewMask-06178";
        dynamic_rendering_color_count_06179      = "VUID-vkCmdDrawMeshTasksIndirectEXT-colorAttachmentCount-06179";
        dynamic_rendering_color_formats_08910    = "VUID-vkCmdDrawMeshTasksIndirectEXT-dynamicRenderingUnusedAttachments-08910";
        dynamic_rendering_unused_attachments_08911 = "VUID-vkCmdDrawMeshTasksIndirectEXT-dynamicRenderingUnusedAttachments-08911";
        dynamic_rendering_undefined_color_formats_08912 = "VUID-vkCmdDrawMeshTasksIndirectEXT-dynamicRenderingUnusedAttachments-08912";
        dynamic_rendering_undefined_depth_format_08916 = "VUID-vkCmdDrawMeshTasksIndirectEXT-dynamicRenderingUnusedAttachments-08916";
        dynamic_rendering_undefined_stencil_format_08916 = "VUID-vkCmdDrawMeshTasksIndirectEXT-dynamicRenderingUnusedAttachments-08916";
        dynamic_rendering_depth_format_08914     = "VUID-vkCmdDrawMeshTasksIndirectEXT-dynamicRenderingUnusedAttachments-08914";
        dynamic_rendering_unused_attachments_08915 = "VUID-vkCmdDrawMeshTasksIndirectEXT-dynamicRenderingUnusedAttachments-08915";
        dynamic_rendering_stencil_format_08917   = "VUID-vkCmdDrawMeshTasksIndirectEXT-dynamicRenderingUnusedAttachments-08917";
        dynamic_rendering_unused_attachments_08918 = "VUID-vkCmdDrawMeshTasksIndirectEXT-dynamicRenderingUnusedAttachments-08918";
        dynamic_rendering_fsr_06183              = "VUID-vkCmdDrawMeshTasksIndirectEXT-imageView-06183";
        dynamic_rendering_fdm_06184              = "VUID-vkCmdDrawMeshTasksIndirectEXT-imageView-06184";
        dynamic_rendering_color_sample_06185     = "VUID-vkCmdDrawMeshTasksIndirectEXT-colorAttachmentCount-06185";
        dynamic_rendering_depth_sample_06186     = "VUID-vkCmdDrawMeshTasksIndirectEXT-pDepthAttachment-06186";
        dynamic_rendering_stencil_sample_06187   = "VUID-vkCmdDrawMeshTasksIndirectEXT-pStencilAttachment-06187";
        dynamic_rendering_06198                  = "VUID-vkCmdDrawMeshTasksIndirectEXT-renderPass-06198";
        dynamic_rendering_07285                  = "VUID-vkCmdDrawMeshTasksIndirectEXT-multisampledRenderToSingleSampled-07285";
        dynamic_rendering_07286                  = "VUID-vkCmdDrawMeshTasksIndirectEXT-multisampledRenderToSingleSampled-07286";
        dynamic_rendering_07287                  = "VUID-vkCmdDrawMeshTasksIndirectEXT-multisampledRenderToSingleSampled-07287";
        image_view_access_64_04470               = "VUID-vkCmdDrawMeshTasksIndirectEXT-SampledType-04470";
        image_view_access_32_04471               = "VUID-vkCmdDrawMeshTasksIndirectEXT-SampledType-04471";
        image_view_sparse_64_04474               = "VUID-vkCmdDrawMeshTasksIndirectEXT-sparseImageInt64Atomics-04474";
        buffer_view_access_64_04472              = "VUID-vkCmdDrawMeshTasksIndirectEXT-SampledType-04472";
        buffer_view_access_32_04473              = "VUID-vkCmdDrawMeshTasksIndirectEXT-SampledType-04473";
        storage_image_read_without_format_07028  = "VUID-vkCmdDrawMeshTasksIndirectEXT-OpTypeImage-07028";
        storage_image_write_without_format_07027 = "VUID-vkCmdDrawMeshTasksIndirectEXT-OpTypeImage-07027";
        storage_texel_buffer_read_without_format_07030  = "VUID-vkCmdDrawMeshTasksIndirectEXT-OpTypeImage-07030";
        storage_texel_buffer_write_without_format_07029 = "VUID-vkCmdDrawMeshTasksIndirectEXT-OpTypeImage-07029";
        storage_image_write_texel_count_08795           = "VUID-vkCmdDrawMeshTasksIndirectEXT-OpImageWrite-08795";
        storage_image_write_texel_count_08796           = "VUID-vkCmdDrawMeshTasksIndirectEXT-OpImageWrite-08796";
        storage_texel_buffer_write_texel_count_04469    = "VUID-vkCmdDrawMeshTasksIndirectEXT-OpImageWrite-04469";
        depth_compare_sample_06479               = "VUID-vkCmdDrawMeshTasksIndirectEXT-None-06479";
        depth_read_only_06886                    = "VUID-vkCmdDrawMeshTasksIndirectEXT-None-06886";
        stencil_read_only_06887                  = "VUID-vkCmdDrawMeshTasksIndirectEXT-None-06887";
        dynamic_sample_locations_06666           = "VUID-vkCmdDrawMeshTasksIndirectEXT-None-06666";
        dynamic_tessellation_domain_origin_07619 = "VUID-vkCmdDrawMeshTasksIndirectEXT-None-07619";
        dynamic_depth_clamp_enable_07620         = "VUID-vkCmdDrawMeshTasksIndirectEXT-None-07620";
        dynamic_polygon_mode_07621               = "VUID-vkCmdDrawMeshTasksIndirectEXT-None-07621";
        dynamic_rasterization_samples_07622      = "VUID-vkCmdDrawMeshTasksIndirectEXT-None-07622";
        dynamic_sample_mask_07623                = "VUID-vkCmdDrawMeshTasksIndirectEXT-None-07623";
        dynamic_alpha_to_coverage_enable_07624   = "VUID-vkCmdDrawMeshTasksIndirectEXT-None-07624";
        dynamic_alpha_to_coverage_component_08919 = "VUID-vkCmdDrawMeshTasksIndirectEXT-alphaToCoverageEnable-08919";
        dynamic_alpha_to_one_enable_07625        = "VUID-vkCmdDrawMeshTasksIndirectEXT-None-07625";
        dynamic_logic_op_enable_07626            = "VUID-vkCmdDrawMeshTasksIndirectEXT-None-07626";
        dynamic_color_blend_enable_07476         = "VUID-vkCmdDrawMeshTasksIndirectEXT-firstAttachment-07476";
        dynamic_color_blend_equation_07477       = "VUID-vkCmdDrawMeshTasksIndirectEXT-firstAttachment-07477";
        dynamic_color_write_mask_07478           = "VUID-vkCmdDrawMeshTasksIndirectEXT-firstAttachment-07478";
        dynamic_rasterization_stream_07630       = "VUID-vkCmdDrawMeshTasksIndirectEXT-None-07630";
        dynamic_conservative_rasterization_mode_07631 = "VUID-vkCmdDrawMeshTasksIndirectEXT-None-07631";
        dynamic_extra_primitive_overestimation_size_07632 = "VUID-vkCmdDrawMeshTasksIndirectEXT-None-07632";
        dynamic_depth_clip_enable_07633          = "VUID-vkCmdDrawMeshTasksIndirectEXT-None-07633";
        dynamic_sample_locations_enable_07634    = "VUID-vkCmdDrawMeshTasksIndirectEXT-None-07634";
        dynamic_color_blend_advanced_07479       = "VUID-vkCmdDrawMeshTasksIndirectEXT-firstAttachment-07479";
        dynamic_provoking_vertex_mode_07636      = "VUID-vkCmdDrawMeshTasksIndirectEXT-None-07636";
        dynamic_line_rasterization_mode_07637    = "VUID-vkCmdDrawMeshTasksIndirectEXT-None-07637";
        dynamic_line_stipple_enable_07638        = "VUID-vkCmdDrawMeshTasksIndirectEXT-None-07638";
        dynamic_depth_clip_negative_one_to_one_07639 = "VUID-vkCmdDrawMeshTasksIndirectEXT-None-07639";
        dynamic_viewport_w_scaling_enable_07640  = "VUID-vkCmdDrawMeshTasksIndirectEXT-None-07640";
        dynamic_viewport_swizzle_07641           = "VUID-vkCmdDrawMeshTasksIndirectEXT-None-07641";
        dynamic_coverage_to_color_enable_07642   = "VUID-vkCmdDrawMeshTasksIndirectEXT-None-07642";
        dynamic_coverage_to_color_location_07643 = "VUID-vkCmdDrawMeshTasksIndirectEXT-None-07643";
        dynamic_coverage_modulation_mode_07644   = "VUID-vkCmdDrawMeshTasksIndirectEXT-None-07644";
        dynamic_coverage_modulation_table_enable_07645 = "VUID-vkCmdDrawMeshTasksIndirectEXT-None-07645";
        dynamic_coverage_modulation_table_07646  = "VUID-vkCmdDrawMeshTasksIndirectEXT-None-07646";
        dynamic_shading_rate_image_enable_07647  = "VUID-vkCmdDrawMeshTasksIndirectEXT-None-07647";
        dynamic_representative_fragment_test_enable_07648  = "VUID-vkCmdDrawMeshTasksIndirectEXT-None-07648";
        dynamic_coverage_reduction_mode_07649    = "VUID-vkCmdDrawMeshTasksIndirectEXT-None-07649";
        dynamic_viewport_07831                   = "VUID-vkCmdDrawMeshTasksIndirectEXT-None-07831";
        dynamic_scissor_07832                    = "VUID-vkCmdDrawMeshTasksIndirectEXT-None-07832";
        dynamic_depth_bias_07834                 = "VUID-vkCmdDrawMeshTasksIndirectEXT-None-07834";
        dynamic_line_width_07833                 = "VUID-vkCmdDrawMeshTasksIndirectEXT-None-07833";
        dynamic_line_stipple_ext_07849           = "VUID-vkCmdDrawMeshTasksIndirectEXT-None-07849";
        dynamic_blend_constants_07835            = "VUID-vkCmdDrawMeshTasksIndirectEXT-None-07835";
        dynamic_depth_bounds_07836               = "VUID-vkCmdDrawMeshTasksIndirectEXT-None-07836";
        dynamic_depth_enable_08715               = "VUID-vkCmdDrawMeshTasksIndirectEXT-pDynamicStates-08715";
        dynamic_stencil_compare_mask_07837       = "VUID-vkCmdDrawMeshTasksIndirectEXT-None-07837";
        dynamic_stencil_write_mask_07838         = "VUID-vkCmdDrawMeshTasksIndirectEXT-None-07838";
        dynamic_stencil_write_mask_08716         = "VUID-vkCmdDrawMeshTasksIndirectEXT-pDynamicStates-08716";
        dynamic_stencil_reference_07839          = "VUID-vkCmdDrawMeshTasksIndirectEXT-None-07839";
        dynamic_state_inherited_07850            = "VUID-vkCmdDrawMeshTasksIndirectEXT-None-07850";
        dynamic_cull_mode_07840                  = "VUID-vkCmdDrawMeshTasksIndirectEXT-None-07840";
        dynamic_front_face_07841                 = "VUID-vkCmdDrawMeshTasksIndirectEXT-None-07841";
        dynamic_depth_test_enable_07843          = "VUID-vkCmdDrawMeshTasksIndirectEXT-None-07843";
        dynamic_depth_write_enable_07844         = "VUID-vkCmdDrawMeshTasksIndirectEXT-None-07844";
        dynamic_depth_compare_op_07845           = "VUID-vkCmdDrawMeshTasksIndirectEXT-None-07845";
        dynamic_depth_bound_test_enable_07846    = "VUID-vkCmdDrawMeshTasksIndirectEXT-None-07846";
        dynamic_stencil_test_enable_07847        = "VUID-vkCmdDrawMeshTasksIndirectEXT-None-07847";
        dynamic_stencil_op_07848                 = "VUID-vkCmdDrawMeshTasksIndirectEXT-None-07848";
        primitives_generated_06708               = "VUID-vkCmdDrawMeshTasksIndirectEXT-primitivesGeneratedQueryWithRasterizerDiscard-06708";
        primitives_generated_streams_06709       = "VUID-vkCmdDrawMeshTasksIndirectEXT-primitivesGeneratedQueryWithNonZeroStreams-06709";
        mesh_shader_stages_06480                 = "VUID-vkCmdDrawMeshTasksIndirectEXT-stage-06480";
        missing_mesh_shader_stages_07080         = "VUID-vkCmdDrawMeshTasksIndirectEXT-MeshEXT-07091";
        descriptor_buffer_bit_set_08114          = "VUID-vkCmdDrawMeshTasksIndirectEXT-None-08114";
        descriptor_buffer_bit_not_set_08115      = "VUID-vkCmdDrawMeshTasksIndirectEXT-None-08115";
        descriptor_buffer_set_offset_missing_08117 = "VUID-vkCmdDrawMeshTasksIndirectEXT-None-08117";
        image_view_dim_07752                     = "VUID-vkCmdDrawMeshTasksIndirectEXT-viewType-07752";
        image_view_numeric_format_07753          = "VUID-vkCmdDrawMeshTasksIndirectEXT-format-07753";
        stippled_rectangular_lines_07495         = "VUID-vkCmdDrawMeshTasksIndirectEXT-stippledLineEnable-07495";
        stippled_bresenham_lines_07496           = "VUID-vkCmdDrawMeshTasksIndirectEXT-stippledLineEnable-07496";
        stippled_smooth_lines_07497              = "VUID-vkCmdDrawMeshTasksIndirectEXT-stippledLineEnable-07497";
        stippled_default_strict_07498            = "VUID-vkCmdDrawMeshTasksIndirectEXT-stippledLineEnable-07498";
        viewport_and_scissor_with_count_08635    = "VUID-vkCmdDrawMeshTasksIndirectEXT-None-08635";
        viewport_w_scaling_08636                 = "VUID-vkCmdDrawMeshTasksIndirectEXT-None-08636";
        shading_rate_palette_08637               = "VUID-vkCmdDrawMeshTasksIndirectEXT-None-08637";
        exclusive_scissor_08638                  = "VUID-vkCmdDrawMeshTasksIndirectEXT-None-08638";
        external_format_resolve_09362            = "VUID-vkCmdDrawMeshTasksIndirectEXT-colorAttachmentCount-09362";
        external_format_resolve_09363            = "VUID-vkCmdDrawMeshTasksIndirectEXT-None-09363";
        external_format_resolve_09364            = "VUID-vkCmdDrawMeshTasksIndirectEXT-None-09364";
        external_format_resolve_09365            = "VUID-vkCmdDrawMeshTasksIndirectEXT-None-09365";
        external_format_resolve_09368            = "VUID-vkCmdDrawMeshTasksIndirectEXT-None-09368";
        external_format_resolve_09369            = "VUID-vkCmdDrawMeshTasksIndirectEXT-None-09369";
        external_format_resolve_09372            = "VUID-vkCmdDrawMeshTasksIndirectEXT-colorAttachmentCount-09372";
        set_rasterizer_discard_enable_08639      = "VUID-vkCmdDrawMeshTasksIndirectEXT-None-08639";
        set_depth_bias_enable_08640              = "VUID-vkCmdDrawMeshTasksIndirectEXT-None-08640";
        set_logic_op_08641                       = "VUID-vkCmdDrawMeshTasksIndirectEXT-None-08641";
        set_color_blend_enable_08643             = "VUID-vkCmdDrawMeshTasksIndirectEXT-None-08643";
        set_rasterization_samples_08644              = "VUID-vkCmdDrawMeshTasksIndirectEXT-None-08644";
        set_color_write_enable_08646             = "VUID-vkCmdDrawMeshTasksIndirectEXT-None-08646";
        set_color_write_enable_08647             = "VUID-vkCmdDrawMeshTasksIndirectEXT-None-08647";
        set_discard_rectangles_enable_08648      = "VUID-vkCmdDrawMeshTasksIndirectEXT-None-08648";
        set_discard_rectangles_mode_08649      = "VUID-vkCmdDrawMeshTasksIndirectEXT-None-08649";
        set_depth_clamp_enable_08650             = "VUID-vkCmdDrawMeshTasksIndirectEXT-None-08650";
        set_polygon_mode_08651                   = "VUID-vkCmdDrawMeshTasksIndirectEXT-None-08651";
        set_rasterization_samples_08652          = "VUID-vkCmdDrawMeshTasksIndirectEXT-None-08652";
        set_sample_mask_08653                    = "VUID-vkCmdDrawMeshTasksIndirectEXT-None-08653";
        set_alpha_to_coverage_enable_08654       = "VUID-vkCmdDrawMeshTasksIndirectEXT-None-08654";
        set_alpha_to_one_enable_08655            = "VUID-vkCmdDrawMeshTasksIndirectEXT-None-08655";
        set_logic_op_enable_08656                = "VUID-vkCmdDrawMeshTasksIndirectEXT-None-08656";
        set_color_blend_enable_08657             = "VUID-vkCmdDrawMeshTasksIndirectEXT-None-08657";
        set_color_blend_equation_08658           = "VUID-vkCmdDrawMeshTasksIndirectEXT-None-08658";
        set_color_write_mask_08659               = "VUID-vkCmdDrawMeshTasksIndirectEXT-None-08659";
        set_rasterization_streams_08660          = "VUID-vkCmdDrawMeshTasksIndirectEXT-None-08660";
        set_conservative_rasterization_mode_08661 = "VUID-vkCmdDrawMeshTasksIndirectEXT-None-08661";
        set_extra_primitive_overestimation_size_08662 = "VUID-vkCmdDrawMeshTasksIndirectEXT-None-08662";
        set_depth_clip_enable_08663              = "VUID-vkCmdDrawMeshTasksIndirectEXT-None-08663";
        set_sample_locations_enable_08664        = "VUID-vkCmdDrawMeshTasksIndirectEXT-None-08664";
        set_provoking_vertex_mode_08665          = "VUID-vkCmdDrawMeshTasksIndirectEXT-None-08665";
        set_line_rasterization_mode_08666        = "VUID-vkCmdDrawMeshTasksIndirectEXT-None-08666";
        set_line_rasterization_mode_08667        = "VUID-vkCmdDrawMeshTasksIndirectEXT-None-08667";
        set_line_rasterization_mode_08668        = "VUID-vkCmdDrawMeshTasksIndirectEXT-None-08668";
        set_line_stipple_enable_08669            = "VUID-vkCmdDrawMeshTasksIndirectEXT-None-08669";
        set_line_stipple_enable_08670            = "VUID-vkCmdDrawMeshTasksIndirectEXT-None-08670";
        set_line_stipple_enable_08671            = "VUID-vkCmdDrawMeshTasksIndirectEXT-None-08671";
        set_line_stipple_08672                   = "VUID-vkCmdDrawMeshTasksIndirectEXT-None-08672";
        set_depth_clip_negative_one_to_one_08673 = "VUID-vkCmdDrawMeshTasksIndirectEXT-None-08673";
        set_viewport_w_scaling_enable_08674      = "VUID-vkCmdDrawMeshTasksIndirectEXT-None-08674";
        set_viewport_swizzle_08675               = "VUID-vkCmdDrawMeshTasksIndirectEXT-None-08675";
        set_coverage_to_color_enable_08676       = "VUID-vkCmdDrawMeshTasksIndirectEXT-None-08676";
        set_coverage_to_color_location_08677     = "VUID-vkCmdDrawMeshTasksIndirectEXT-None-08677";
        set_coverage_modulation_mode_08678       = "VUID-vkCmdDrawMeshTasksIndirectEXT-None-08678";
        set_coverage_modulation_table_enable_08679 = "VUID-vkCmdDrawMeshTasksIndirectEXT-None-08679";
        set_coverage_modulation_table_08680      = "VUID-vkCmdDrawMeshTasksIndirectEXT-None-08680";
        set_shading_rate_image_enable_08681      = "VUID-vkCmdDrawMeshTasksIndirectEXT-None-08681";
        set_representative_fragment_test_enable_08682 = "VUID-vkCmdDrawMeshTasksIndirectEXT-None-08682";
        set_coverage_reduction_mode_08683        = "VUID-vkCmdDrawMeshTasksIndirectEXT-None-08683";
        vertex_shader_08684                      = "VUID-vkCmdDrawMeshTasksIndirectEXT-None-08684";
        tessellation_control_shader_08685        = "VUID-vkCmdDrawMeshTasksIndirectEXT-None-08685";
        tessellation_evaluation_shader_08686     = "VUID-vkCmdDrawMeshTasksIndirectEXT-None-08686";
        geometry_shader_08687                    = "VUID-vkCmdDrawMeshTasksIndirectEXT-None-08687";
        fragment_shader_08688                    = "VUID-vkCmdDrawMeshTasksIndirectEXT-None-08688";
        task_shader_08689                        = "VUID-vkCmdDrawMeshTasksIndirectEXT-None-08689";
        mesh_shader_08690                        = "VUID-vkCmdDrawMeshTasksIndirectEXT-None-08690";
        vert_mesh_shader_08693                   = "VUID-vkCmdDrawMeshTasksIndirectEXT-None-08693";
        task_mesh_shader_08694                   = "VUID-vkCmdDrawMeshTasksIndirectEXT-None-08694";
        task_mesh_shader_08695                   = "VUID-vkCmdDrawMeshTasksIndirectEXT-None-08695";
        vert_task_mesh_shader_08696              = "VUID-vkCmdDrawMeshTasksIndirectEXT-None-08696";
        linked_shaders_08698                     = "VUID-vkCmdDrawMeshTasksIndirectEXT-None-08698";
        linked_shaders_08699                     = "VUID-vkCmdDrawMeshTasksIndirectEXT-None-08699";
        shaders_push_constants_08878             = "VUID-vkCmdDrawMeshTasksIndirectEXT-None-08878";
        shaders_descriptor_layouts_08879         = "VUID-vkCmdDrawMeshTasksIndirectEXT-None-08879";
        set_attachment_feedback_loop_enable_08880 = "VUID-vkCmdDrawMeshTasksIndirectEXT-None-08880";
        set_line_width_08617                     = "VUID-vkCmdDrawMeshTasksIndirectEXT-None-08617";
        set_line_width_08618                     = "VUID-vkCmdDrawMeshTasksIndirectEXT-None-08618";
        set_depth_bias_08620                     = "VUID-vkCmdDrawMeshTasksIndirectEXT-None-08620";
        set_blend_constants_08621                = "VUID-vkCmdDrawMeshTasksIndirectEXT-None-08621";
        set_depth_bounds_08622                   = "VUID-vkCmdDrawMeshTasksIndirectEXT-None-08622";
        set_stencil_compare_mask_08623           = "VUID-vkCmdDrawMeshTasksIndirectEXT-None-08623";
        set_stencil_write_mask_08624             = "VUID-vkCmdDrawMeshTasksIndirectEXT-None-08624";
        set_stencil_reference_08625              = "VUID-vkCmdDrawMeshTasksIndirectEXT-None-08625";
        set_sample_locations_08626               = "VUID-vkCmdDrawMeshTasksIndirectEXT-None-08626";
        set_cull_mode_08627                      = "VUID-vkCmdDrawMeshTasksIndirectEXT-None-08627";
        set_front_face_08628                     = "VUID-vkCmdDrawMeshTasksIndirectEXT-None-08628";
        set_depth_test_enable_08629              = "VUID-vkCmdDrawMeshTasksIndirectEXT-None-08629";
        set_depth_write_enable_08630             = "VUID-vkCmdDrawMeshTasksIndirectEXT-None-08630";
        set_depth_comapre_op_08631               = "VUID-vkCmdDrawMeshTasksIndirectEXT-None-08631";
        set_depth_bounds_test_enable_08632       = "VUID-vkCmdDrawMeshTasksIndirectEXT-None-08632";
        set_stencil_test_enable_08633            = "VUID-vkCmdDrawMeshTasksIndirectEXT-None-08633";
        set_stencil_op_08634                     = "VUID-vkCmdDrawMeshTasksIndirectEXT-None-08634";
        set_line_width_08619                     = "VUID-vkCmdDrawMeshTasksIndirectEXT-None-08619";
        set_viewport_with_count_08642            = "VUID-vkCmdDrawMeshTasksIndirectEXT-primitiveFragmentShadingRateWithMultipleViewports-08642";
        alpha_component_word_08920               = "VUID-vkCmdDrawMeshTasksIndirectEXT-alphaToCoverageEnable-08920";
        color_write_mask_09116                   = "VUID-vkCmdDrawMeshTasksIndirectEXT-None-09116";
        set_clip_space_w_scaling_09232           = "VUID-vkCmdDrawMeshTasksIndirectEXT-None-09232";
        set_discard_rectangle_09236              = "VUID-vkCmdDrawMeshTasksIndirectEXT-rasterizerDiscardEnable-09236";
        set_viewport_coarse_sample_order_09233   = "VUID-vkCmdDrawMeshTasksIndirectEXT-shadingRateImage-09233";
        set_viewport_shading_rate_palette_09234  = "VUID-vkCmdDrawMeshTasksIndirectEXT-shadingRateImage-09234";
        set_exclusive_scissor_enable_09235       = "VUID-vkCmdDrawMeshTasksIndirectEXT-exclusiveScissor-09235";
        set_fragment_shading_rate_09238          = "VUID-vkCmdDrawMeshTasksIndirectEXT-pipelineFragmentShadingRate-09238";
        set_tessellation_domain_origin_09237     = "VUID-vkCmdDrawMeshTasksIndirectEXT-None-09237";
        rasterization_samples_07935              = "VUID-vkCmdDrawMeshTasksIndirectEXT-pNext-07935";
        mesh_shader_queries_07073                = "VUID-vkCmdDrawMeshTasksIndirectEXT-stage-07073";
        blend_advanced_07480                     = "VUID-vkCmdDrawMeshTasksIndirectEXT-advancedBlendMaxColorAttachments-07480";
        blend_feature_07470                      = "VUID-vkCmdDrawMeshTasksIndirectEXT-pColorBlendEnables-07470";
        color_attachment_08963                   = "VUID-vkCmdDrawMeshTasksIndirectEXT-pColorAttachments-08963";
        depth_attachment_08964                   = "VUID-vkCmdDrawMeshTasksIndirectEXT-pDepthAttachment-08964";
        stencil_attachment_08965                 = "VUID-vkCmdDrawMeshTasksIndirectEXT-pStencilAttachment-08965";
        sample_locations_07482                   = "VUID-vkCmdDrawMeshTasksIndirectEXT-sampleLocationsPerPixel-07482";
    }
};

struct DispatchVuidsCmdDrawMeshTasksIndirectCountEXT : DrawDispatchVuid {
    DispatchVuidsCmdDrawMeshTasksIndirectCountEXT() : DrawDispatchVuid() {
        pipeline_bound_08606                     = "VUID-vkCmdDrawMeshTasksIndirectCountEXT-None-08606";
        pipeline_or_shaders_bound_08607          = "VUID-vkCmdDrawMeshTasksIndirectCountEXT-None-08607";
        compatible_pipeline_08600                = "VUID-vkCmdDrawMeshTasksIndirectCountEXT-None-08600";
        render_pass_compatible_02684             = "VUID-vkCmdDrawMeshTasksIndirectCountEXT-renderPass-02684";
        render_pass_began_08876                  = "VUID-vkCmdDrawMeshTasksIndirectCountEXT-None-08876";
        subpass_index_02685                      = "VUID-vkCmdDrawMeshTasksIndirectCountEXT-subpass-02685";
        sample_location_02689                    = "VUID-vkCmdDrawMeshTasksIndirectCountEXT-sampleLocationsEnable-02689";
        linear_filter_sampler_04553              = "VUID-vkCmdDrawMeshTasksIndirectCountEXT-magFilter-04553";
        linear_mipmap_sampler_04770              = "VUID-vkCmdDrawMeshTasksIndirectCountEXT-mipmapMode-04770";
        cubic_sampler_02692                      = "VUID-vkCmdDrawMeshTasksIndirectCountEXT-None-02692";
        indirect_protected_cb_02646              = "VUID-vkCmdDrawMeshTasksIndirectCountEXT-commandBuffer-02711";
        indirect_contiguous_memory_02708         = "VUID-vkCmdDrawMeshTasksIndirectCountEXT-buffer-02708";
        indirect_buffer_bit_02290                = "VUID-vkCmdDrawMeshTasksIndirectCountEXT-buffer-02709";
        indirect_count_contiguous_memory_02714   = "VUID-vkCmdDrawMeshTasksIndirectCountEXT-countBuffer-02714";
        indirect_count_buffer_bit_02715          = "VUID-vkCmdDrawMeshTasksIndirectCountEXT-countBuffer-02715";
        indirect_count_offset_04129              = "VUID-vkCmdDrawMeshTasksIndirectCountEXT-countBufferOffset-04129";
        viewport_count_03417                     = "VUID-vkCmdDrawMeshTasksIndirectCountEXT-viewportCount-03417";
        scissor_count_03418                      = "VUID-vkCmdDrawMeshTasksIndirectCountEXT-scissorCount-03418";
        viewport_scissor_count_03419             = "VUID-vkCmdDrawMeshTasksIndirectCountEXT-viewportCount-03419";
        corner_sampled_address_mode_02696        = "VUID-vkCmdDrawMeshTasksIndirectCountEXT-flags-02696";
        imageview_atomic_02691                   = "VUID-vkCmdDrawMeshTasksIndirectCountEXT-None-02691";
        bufferview_atomic_07888                  = "VUID-vkCmdDrawMeshTasksIndirectCountEXT-None-07888";
        push_constants_set_08602                 = "VUID-vkCmdDrawMeshTasksIndirectCountEXT-maintenance4-08602";
        image_subresources_render_pass_write_06537 = "VUID-vkCmdDrawMeshTasksIndirectCountEXT-None-06537";
        image_subresources_subpass_read_09003    = "VUID-vkCmdDrawMeshTasksIndirectCountEXT-None-09003";
        image_subresources_subpass_write_06539   = "VUID-vkCmdDrawMeshTasksIndirectCountEXT-None-06539";
        sampler_imageview_type_08609             = "VUID-vkCmdDrawMeshTasksIndirectCountEXT-None-08609";
        sampler_implicitLod_dref_proj_08610      = "VUID-vkCmdDrawMeshTasksIndirectCountEXT-None-08610";
        sampler_bias_offset_08611                = "VUID-vkCmdDrawMeshTasksIndirectCountEXT-None-08611";
        dynamic_state_setting_commands_08608     = "VUID-vkCmdDrawMeshTasksIndirectCountEXT-None-08608";
        msrtss_rasterization_samples_07284       = "VUID-vkCmdDrawMeshTasksIndirectCountEXT-multisampledRenderToSingleSampled-07284";
        unprotected_command_buffer_02707         = "VUID-vkCmdDrawMeshTasksIndirectCountEXT-commandBuffer-02707";
        max_multiview_instance_index_02688       = "VUID-vkCmdDrawMeshTasksIndirectCountEXT-maxMultiviewInstanceIndex-02688";
        img_filter_cubic_02693                   = "VUID-vkCmdDrawMeshTasksIndirectCountEXT-None-02693";
        filter_cubic_02694                       = "VUID-vkCmdDrawMeshTasksIndirectCountEXT-filterCubic-02694";
        filter_cubic_min_max_02695               = "VUID-vkCmdDrawMeshTasksIndirectCountEXT-filterCubicMinmax-02695";
        viewport_count_primitive_shading_rate_04552 = "VUID-vkCmdDrawMeshTasksIndirectCountEXT-primitiveFragmentShadingRateWithMultipleViewports-04552";
        rasterizer_discard_enable_04876          = "VUID-vkCmdDrawMeshTasksIndirectCountEXT-None-04876";
        depth_bias_enable_04877                  = "VUID-vkCmdDrawMeshTasksIndirectCountEXT-None-04877";
        logic_op_04878                           = "VUID-vkCmdDrawMeshTasksIndirectCountEXT-logicOp-04878";
        blend_enable_04727                       = "VUID-vkCmdDrawMeshTasksIndirectCountEXT-blendEnable-04727";
        dynamic_discard_rectangle_07751          = "VUID-vkCmdDrawMeshTasksIndirectCountEXT-None-07751";
        dynamic_discard_rectangle_enable_07880   = "VUID-vkCmdDrawMeshTasksIndirectCountEXT-None-07880";
        dynamic_discard_rectangle_mode_07881     =  "VUID-vkCmdDrawMeshTasksIndirectCountEXT-None-07881";
        dynamic_exclusive_scissor_enable_07878   = "VUID-vkCmdDrawMeshTasksIndirectCountEXT-None-07878";
        dynamic_exclusive_scissor_07879          = "VUID-vkCmdDrawMeshTasksIndirectCountEXT-None-07879";
        dynamic_color_write_enable_07749         = "VUID-vkCmdDrawMeshTasksIndirectCountEXT-None-07749";
        dynamic_color_write_enable_count_07750   = "VUID-vkCmdDrawMeshTasksIndirectCountEXT-attachmentCount-07750";
        dynamic_attachment_feedback_loop_08877   = "VUID-vkCmdDrawMeshTasksIndirectCountEXT-None-08877";
        dynamic_rendering_view_mask_06178        = "VUID-vkCmdDrawMeshTasksIndirectCountEXT-viewMask-06178";
        dynamic_rendering_color_count_06179      = "VUID-vkCmdDrawMeshTasksIndirectCountEXT-colorAttachmentCount-06179";
        dynamic_rendering_color_formats_08910    = "VUID-vkCmdDrawMeshTasksIndirectCountEXT-dynamicRenderingUnusedAttachments-08910";
        dynamic_rendering_unused_attachments_08911 = "VUID-vkCmdDrawMeshTasksIndirectCountEXT-dynamicRenderingUnusedAttachments-08911";
        dynamic_rendering_undefined_color_formats_08912 = "VUID-vkCmdDrawMeshTasksIndirectCountEXT-dynamicRenderingUnusedAttachments-08912";
        dynamic_rendering_undefined_depth_format_08916 = "VUID-vkCmdDrawMeshTasksIndirectCountEXT-dynamicRenderingUnusedAttachments-08916";
        dynamic_rendering_undefined_stencil_format_08916 = "VUID-vkCmdDrawMeshTasksIndirectCountEXT-dynamicRenderingUnusedAttachments-08916";
        dynamic_rendering_depth_format_08914     = "VUID-vkCmdDrawMeshTasksIndirectCountEXT-dynamicRenderingUnusedAttachments-08914";
        dynamic_rendering_unused_attachments_08915 = "VUID-vkCmdDrawMeshTasksIndirectCountEXT-dynamicRenderingUnusedAttachments-08915";
        dynamic_rendering_stencil_format_08917   = "VUID-vkCmdDrawMeshTasksIndirectCountEXT-dynamicRenderingUnusedAttachments-08917";
        dynamic_rendering_unused_attachments_08918 = "VUID-vkCmdDrawMeshTasksIndirectCountEXT-dynamicRenderingUnusedAttachments-08918";
        dynamic_rendering_fsr_06183              = "VUID-vkCmdDrawMeshTasksIndirectCountEXT-imageView-06183";
        dynamic_rendering_fdm_06184              = "VUID-vkCmdDrawMeshTasksIndirectCountEXT-imageView-06184";
        dynamic_rendering_color_sample_06185     = "VUID-vkCmdDrawMeshTasksIndirectCountEXT-colorAttachmentCount-06185";
        dynamic_rendering_depth_sample_06186     = "VUID-vkCmdDrawMeshTasksIndirectCountEXT-pDepthAttachment-06186";
        dynamic_rendering_stencil_sample_06187   = "VUID-vkCmdDrawMeshTasksIndirectCountEXT-pStencilAttachment-06187";
        dynamic_rendering_06198                  = "VUID-vkCmdDrawMeshTasksIndirectCountEXT-renderPass-06198";
        dynamic_rendering_07285                  = "VUID-vkCmdDrawMeshTasksIndirectCountEXT-multisampledRenderToSingleSampled-07285";
        dynamic_rendering_07286                  = "VUID-vkCmdDrawMeshTasksIndirectCountEXT-multisampledRenderToSingleSampled-07286";
        dynamic_rendering_07287                  = "VUID-vkCmdDrawMeshTasksIndirectCountEXT-multisampledRenderToSingleSampled-07287";
        image_view_access_64_04470               = "VUID-vkCmdDrawMeshTasksIndirectCountEXT-SampledType-04470";
        image_view_access_32_04471               = "VUID-vkCmdDrawMeshTasksIndirectCountEXT-SampledType-04471";
        image_view_sparse_64_04474               = "VUID-vkCmdDrawMeshTasksIndirectCountEXT-sparseImageInt64Atomics-04474";
        buffer_view_access_64_04472              = "VUID-vkCmdDrawMeshTasksIndirectCountEXT-SampledType-04472";
        buffer_view_access_32_04473              = "VUID-vkCmdDrawMeshTasksIndirectCountEXT-SampledType-04473";
        storage_image_read_without_format_07028  = "VUID-vkCmdDrawMeshTasksIndirectCountEXT-OpTypeImage-07028";
        storage_image_write_without_format_07027 = "VUID-vkCmdDrawMeshTasksIndirectCountEXT-OpTypeImage-07027";
        storage_texel_buffer_read_without_format_07030  = "VUID-vkCmdDrawMeshTasksIndirectCountEXT-OpTypeImage-07030";
        storage_texel_buffer_write_without_format_07029 = "VUID-vkCmdDrawMeshTasksIndirectCountEXT-OpTypeImage-07029";
        storage_image_write_texel_count_08795           = "VUID-vkCmdDrawMeshTasksIndirectCountEXT-OpImageWrite-08795";
        storage_image_write_texel_count_08796           = "VUID-vkCmdDrawMeshTasksIndirectCountEXT-OpImageWrite-08796";
        storage_texel_buffer_write_texel_count_04469    = "VUID-vkCmdDrawMeshTasksIndirectCountEXT-OpImageWrite-04469";
        depth_compare_sample_06479               = "VUID-vkCmdDrawMeshTasksIndirectCountEXT-None-06479";
        depth_read_only_06886                    = "VUID-vkCmdDrawMeshTasksIndirectCountEXT-None-06886";
        stencil_read_only_06887                  = "VUID-vkCmdDrawMeshTasksIndirectCountEXT-None-06887";
        dynamic_sample_locations_06666           = "VUID-vkCmdDrawMeshTasksIndirectCountEXT-None-06666";
        dynamic_tessellation_domain_origin_07619 = "VUID-vkCmdDrawMeshTasksIndirectCountEXT-None-07619";
        dynamic_depth_clamp_enable_07620         = "VUID-vkCmdDrawMeshTasksIndirectCountEXT-None-07620";
        dynamic_polygon_mode_07621               = "VUID-vkCmdDrawMeshTasksIndirectCountEXT-None-07621";
        dynamic_rasterization_samples_07622      = "VUID-vkCmdDrawMeshTasksIndirectCountEXT-None-07622";
        dynamic_sample_mask_07623                = "VUID-vkCmdDrawMeshTasksIndirectCountEXT-None-07623";
        dynamic_alpha_to_coverage_enable_07624   = "VUID-vkCmdDrawMeshTasksIndirectCountEXT-None-07624";
        dynamic_alpha_to_coverage_component_08919 = "VUID-vkCmdDrawMeshTasksIndirectCountEXT-alphaToCoverageEnable-08919";
        dynamic_alpha_to_one_enable_07625        = "VUID-vkCmdDrawMeshTasksIndirectCountEXT-None-07625";
        dynamic_logic_op_enable_07626            = "VUID-vkCmdDrawMeshTasksIndirectCountEXT-None-07626";
        dynamic_color_blend_enable_07476         = "VUID-vkCmdDrawMeshTasksIndirectCountEXT-firstAttachment-07476";
        dynamic_color_blend_equation_07477       = "VUID-vkCmdDrawMeshTasksIndirectCountEXT-firstAttachment-07477";
        dynamic_color_write_mask_07478           = "VUID-vkCmdDrawMeshTasksIndirectCountEXT-firstAttachment-07478";
        dynamic_rasterization_stream_07630       = "VUID-vkCmdDrawMeshTasksIndirectCountEXT-None-07630";
        dynamic_conservative_rasterization_mode_07631 = "VUID-vkCmdDrawMeshTasksIndirectCountEXT-None-07631";
        dynamic_extra_primitive_overestimation_size_07632 = "VUID-vkCmdDrawMeshTasksIndirectCountEXT-None-07632";
        dynamic_depth_clip_enable_07633          = "VUID-vkCmdDrawMeshTasksIndirectCountEXT-None-07633";
        dynamic_sample_locations_enable_07634    = "VUID-vkCmdDrawMeshTasksIndirectCountEXT-None-07634";
        dynamic_color_blend_advanced_07479       = "VUID-vkCmdDrawMeshTasksIndirectCountEXT-firstAttachment-07479";
        dynamic_provoking_vertex_mode_07636      = "VUID-vkCmdDrawMeshTasksIndirectCountEXT-None-07636";
        dynamic_line_rasterization_mode_07637    = "VUID-vkCmdDrawMeshTasksIndirectCountEXT-None-07637";
        dynamic_line_stipple_enable_07638        = "VUID-vkCmdDrawMeshTasksIndirectCountEXT-None-07638";
        dynamic_depth_clip_negative_one_to_one_07639 = "VUID-vkCmdDrawMeshTasksIndirectCountEXT-None-07639";
        dynamic_viewport_w_scaling_enable_07640  = "VUID-vkCmdDrawMeshTasksIndirectCountEXT-None-07640";
        dynamic_viewport_swizzle_07641           = "VUID-vkCmdDrawMeshTasksIndirectCountEXT-None-07641";
        dynamic_coverage_to_color_enable_07642   = "VUID-vkCmdDrawMeshTasksIndirectCountEXT-None-07642";
        dynamic_coverage_to_color_location_07643 = "VUID-vkCmdDrawMeshTasksIndirectCountEXT-None-07643";
        dynamic_coverage_modulation_mode_07644   = "VUID-vkCmdDrawMeshTasksIndirectCountEXT-None-07644";
        dynamic_coverage_modulation_table_enable_07645 = "VUID-vkCmdDrawMeshTasksIndirectCountEXT-None-07645";
        dynamic_coverage_modulation_table_07646  = "VUID-vkCmdDrawMeshTasksIndirectCountEXT-None-07646";
        dynamic_shading_rate_image_enable_07647  = "VUID-vkCmdDrawMeshTasksIndirectCountEXT-None-07647";
        dynamic_representative_fragment_test_enable_07648  = "VUID-vkCmdDrawMeshTasksIndirectCountEXT-None-07648";
        dynamic_coverage_reduction_mode_07649    = "VUID-vkCmdDrawMeshTasksIndirectCountEXT-None-07649";
        dynamic_viewport_07831                   = "VUID-vkCmdDrawMeshTasksIndirectCountEXT-None-07831";
        dynamic_scissor_07832                    = "VUID-vkCmdDrawMeshTasksIndirectCountEXT-None-07832";
        dynamic_depth_bias_07834                 = "VUID-vkCmdDrawMeshTasksIndirectCountEXT-None-07834";
        dynamic_line_width_07833                 = "VUID-vkCmdDrawMeshTasksIndirectCountEXT-None-07833";
        dynamic_line_stipple_ext_07849           = "VUID-vkCmdDrawMeshTasksIndirectCountEXT-None-07849";
        dynamic_blend_constants_07835            = "VUID-vkCmdDrawMeshTasksIndirectCountEXT-None-07835";
        dynamic_depth_bounds_07836               = "VUID-vkCmdDrawMeshTasksIndirectCountEXT-None-07836";
        dynamic_depth_enable_08715               = "VUID-vkCmdDrawMeshTasksIndirectCountEXT-pDynamicStates-08715";
        dynamic_stencil_compare_mask_07837       = "VUID-vkCmdDrawMeshTasksIndirectCountEXT-None-07837";
        dynamic_stencil_write_mask_07838         = "VUID-vkCmdDrawMeshTasksIndirectCountEXT-None-07838";
        dynamic_stencil_write_mask_08716         = "VUID-vkCmdDrawMeshTasksIndirectCountEXT-pDynamicStates-08716";
        dynamic_stencil_reference_07839          = "VUID-vkCmdDrawMeshTasksIndirectCountEXT-None-07839";
        dynamic_state_inherited_07850            = "VUID-vkCmdDrawMeshTasksIndirectCountEXT-None-07850";
        dynamic_cull_mode_07840                  = "VUID-vkCmdDrawMeshTasksIndirectCountEXT-None-07840";
        dynamic_front_face_07841                 = "VUID-vkCmdDrawMeshTasksIndirectCountEXT-None-07841";
        dynamic_depth_test_enable_07843          = "VUID-vkCmdDrawMeshTasksIndirectCountEXT-None-07843";
        dynamic_depth_write_enable_07844         = "VUID-vkCmdDrawMeshTasksIndirectCountEXT-None-07844";
        dynamic_depth_compare_op_07845           = "VUID-vkCmdDrawMeshTasksIndirectCountEXT-None-07845";
        dynamic_depth_bound_test_enable_07846    = "VUID-vkCmdDrawMeshTasksIndirectCountEXT-None-07846";
        dynamic_stencil_test_enable_07847        = "VUID-vkCmdDrawMeshTasksIndirectCountEXT-None-07847";
        dynamic_stencil_op_07848                 = "VUID-vkCmdDrawMeshTasksIndirectCountEXT-None-07848";
        primitives_generated_06708               = "VUID-vkCmdDrawMeshTasksIndirectCountEXT-primitivesGeneratedQueryWithRasterizerDiscard-06708";
        primitives_generated_streams_06709       = "VUID-vkCmdDrawMeshTasksIndirectCountEXT-primitivesGeneratedQueryWithNonZeroStreams-06709";
        mesh_shader_stages_06480                 = "VUID-vkCmdDrawMeshTasksIndirectCountEXT-stage-06480";
        missing_mesh_shader_stages_07080         = "VUID-vkCmdDrawMeshTasksIndirectCountEXT-MeshEXT-07100";
        descriptor_buffer_bit_set_08114          = "VUID-vkCmdDrawMeshTasksIndirectCountEXT-None-08114";
        descriptor_buffer_bit_not_set_08115      = "VUID-vkCmdDrawMeshTasksIndirectCountEXT-None-08115";
        descriptor_buffer_set_offset_missing_08117 = "VUID-vkCmdDrawMeshTasksIndirectCountEXT-None-08117";
        image_view_dim_07752                     = "VUID-vkCmdDrawMeshTasksIndirectCountEXT-viewType-07752";
        image_view_numeric_format_07753          = "VUID-vkCmdDrawMeshTasksIndirectCountEXT-format-07753";
        stippled_rectangular_lines_07495         = "VUID-vkCmdDrawMeshTasksIndirectCountEXT-stippledLineEnable-07495";
        stippled_bresenham_lines_07496           = "VUID-vkCmdDrawMeshTasksIndirectCountEXT-stippledLineEnable-07496";
        stippled_smooth_lines_07497              = "VUID-vkCmdDrawMeshTasksIndirectCountEXT-stippledLineEnable-07497";
        stippled_default_strict_07498            = "VUID-vkCmdDrawMeshTasksIndirectCountEXT-stippledLineEnable-07498";
        viewport_and_scissor_with_count_08635    = "VUID-vkCmdDrawMeshTasksIndirectCountEXT-None-08635";
        viewport_w_scaling_08636                 = "VUID-vkCmdDrawMeshTasksIndirectCountEXT-None-08636";
        shading_rate_palette_08637               = "VUID-vkCmdDrawMeshTasksIndirectCountEXT-None-08637";
        exclusive_scissor_08638                  = "VUID-vkCmdDrawMeshTasksIndirectCountEXT-None-08638";
        external_format_resolve_09362            = "VUID-vkCmdDrawMeshTasksIndirectCountEXT-colorAttachmentCount-09362";
        external_format_resolve_09363            = "VUID-vkCmdDrawMeshTasksIndirectCountEXT-None-09363";
        external_format_resolve_09364            = "VUID-vkCmdDrawMeshTasksIndirectCountEXT-None-09364";
        external_format_resolve_09365            = "VUID-vkCmdDrawMeshTasksIndirectCountEXT-None-09365";
        external_format_resolve_09368            = "VUID-vkCmdDrawMeshTasksIndirectCountEXT-None-09368";
        external_format_resolve_09369            = "VUID-vkCmdDrawMeshTasksIndirectCountEXT-None-09369";
        external_format_resolve_09372            = "VUID-vkCmdDrawMeshTasksIndirectCountEXT-colorAttachmentCount-09372";
        set_rasterizer_discard_enable_08639      = "VUID-vkCmdDrawMeshTasksIndirectCountEXT-None-08639";
        set_depth_bias_enable_08640              = "VUID-vkCmdDrawMeshTasksIndirectCountEXT-None-08640";
        set_logic_op_08641                       = "VUID-vkCmdDrawMeshTasksIndirectCountEXT-None-08641";
        set_color_blend_enable_08643             = "VUID-vkCmdDrawMeshTasksIndirectCountEXT-None-08643";
        set_rasterization_samples_08644              = "VUID-vkCmdDrawMeshTasksIndirectCountEXT-None-08644";
        set_color_write_enable_08646             = "VUID-vkCmdDrawMeshTasksIndirectCountEXT-None-08646";
        set_color_write_enable_08647             = "VUID-vkCmdDrawMeshTasksIndirectCountEXT-None-08647";
        set_discard_rectangles_enable_08648      = "VUID-vkCmdDrawMeshTasksIndirectCountEXT-None-08648";
        set_discard_rectangles_mode_08649      = "VUID-vkCmdDrawMeshTasksIndirectCountEXT-None-08649";
        set_depth_clamp_enable_08650             = "VUID-vkCmdDrawMeshTasksIndirectCountEXT-None-08650";
        set_polygon_mode_08651                   = "VUID-vkCmdDrawMeshTasksIndirectCountEXT-None-08651";
        set_rasterization_samples_08652          = "VUID-vkCmdDrawMeshTasksIndirectCountEXT-None-08652";
        set_sample_mask_08653                    = "VUID-vkCmdDrawMeshTasksIndirectCountEXT-None-08653";
        set_alpha_to_coverage_enable_08654       = "VUID-vkCmdDrawMeshTasksIndirectCountEXT-None-08654";
        set_alpha_to_one_enable_08655            = "VUID-vkCmdDrawMeshTasksIndirectCountEXT-None-08655";
        set_logic_op_enable_08656                = "VUID-vkCmdDrawMeshTasksIndirectCountEXT-None-08656";
        set_color_blend_enable_08657             = "VUID-vkCmdDrawMeshTasksIndirectCountEXT-None-08657";
        set_color_blend_equation_08658           = "VUID-vkCmdDrawMeshTasksIndirectCountEXT-None-08658";
        set_color_write_mask_08659               = "VUID-vkCmdDrawMeshTasksIndirectCountEXT-None-08659";
        set_rasterization_streams_08660          = "VUID-vkCmdDrawMeshTasksIndirectCountEXT-None-08660";
        set_conservative_rasterization_mode_08661 = "VUID-vkCmdDrawMeshTasksIndirectCountEXT-None-08661";
        set_extra_primitive_overestimation_size_08662 = "VUID-vkCmdDrawMeshTasksIndirectCountEXT-None-08662";
        set_depth_clip_enable_08663              = "VUID-vkCmdDrawMeshTasksIndirectCountEXT-None-08663";
        set_sample_locations_enable_08664        = "VUID-vkCmdDrawMeshTasksIndirectCountEXT-None-08664";
        set_provoking_vertex_mode_08665          = "VUID-vkCmdDrawMeshTasksIndirectCountEXT-None-08665";
        set_line_rasterization_mode_08666        = "VUID-vkCmdDrawMeshTasksIndirectCountEXT-None-08666";
        set_line_rasterization_mode_08667        = "VUID-vkCmdDrawMeshTasksIndirectCountEXT-None-08667";
        set_line_rasterization_mode_08668        = "VUID-vkCmdDrawMeshTasksIndirectCountEXT-None-08668";
        set_line_stipple_enable_08669            = "VUID-vkCmdDrawMeshTasksIndirectCountEXT-None-08669";
        set_line_stipple_enable_08670            = "VUID-vkCmdDrawMeshTasksIndirectCountEXT-None-08670";
        set_line_stipple_enable_08671            = "VUID-vkCmdDrawMeshTasksIndirectCountEXT-None-08671";
        set_line_stipple_08672                   = "VUID-vkCmdDrawMeshTasksIndirectCountEXT-None-08672";
        set_depth_clip_negative_one_to_one_08673 = "VUID-vkCmdDrawMeshTasksIndirectCountEXT-None-08673";
        set_viewport_w_scaling_enable_08674      = "VUID-vkCmdDrawMeshTasksIndirectCountEXT-None-08674";
        set_viewport_swizzle_08675               = "VUID-vkCmdDrawMeshTasksIndirectCountEXT-None-08675";
        set_coverage_to_color_enable_08676       = "VUID-vkCmdDrawMeshTasksIndirectCountEXT-None-08676";
        set_coverage_to_color_location_08677     = "VUID-vkCmdDrawMeshTasksIndirectCountEXT-None-08677";
        set_coverage_modulation_mode_08678       = "VUID-vkCmdDrawMeshTasksIndirectCountEXT-None-08678";
        set_coverage_modulation_table_enable_08679 = "VUID-vkCmdDrawMeshTasksIndirectCountEXT-None-08679";
        set_coverage_modulation_table_08680      = "VUID-vkCmdDrawMeshTasksIndirectCountEXT-None-08680";
        set_shading_rate_image_enable_08681      = "VUID-vkCmdDrawMeshTasksIndirectCountEXT-None-08681";
        set_representative_fragment_test_enable_08682 = "VUID-vkCmdDrawMeshTasksIndirectCountEXT-None-08682";
        set_coverage_reduction_mode_08683        = "VUID-vkCmdDrawMeshTasksIndirectCountEXT-None-08683";
        vertex_shader_08684                      = "VUID-vkCmdDrawMeshTasksIndirectCountEXT-None-08684";
        tessellation_control_shader_08685        = "VUID-vkCmdDrawMeshTasksIndirectCountEXT-None-08685";
        tessellation_evaluation_shader_08686     = "VUID-vkCmdDrawMeshTasksIndirectCountEXT-None-08686";
        geometry_shader_08687                    = "VUID-vkCmdDrawMeshTasksIndirectCountEXT-None-08687";
        fragment_shader_08688                    = "VUID-vkCmdDrawMeshTasksIndirectCountEXT-None-08688";
        task_shader_08689                        = "VUID-vkCmdDrawMeshTasksIndirectCountEXT-None-08689";
        mesh_shader_08690                        = "VUID-vkCmdDrawMeshTasksIndirectCountEXT-None-08690";
        vert_mesh_shader_08693                   = "VUID-vkCmdDrawMeshTasksIndirectCountEXT-None-08693";
        task_mesh_shader_08694                   = "VUID-vkCmdDrawMeshTasksIndirectCountEXT-None-08694";
        task_mesh_shader_08695                   = "VUID-vkCmdDrawMeshTasksIndirectCountEXT-None-08695";
        vert_task_mesh_shader_08696              = "VUID-vkCmdDrawMeshTasksIndirectCountEXT-None-08696";
        linked_shaders_08698                     = "VUID-vkCmdDrawMeshTasksIndirectCountEXT-None-08698";
        linked_shaders_08699                     = "VUID-vkCmdDrawMeshTasksIndirectCountEXT-None-08699";
        shaders_push_constants_08878             = "VUID-vkCmdDrawMeshTasksIndirectCountEXT-None-08878";
        shaders_descriptor_layouts_08879         = "VUID-vkCmdDrawMeshTasksIndirectCountEXT-None-08879";
        set_attachment_feedback_loop_enable_08880 = "VUID-vkCmdDrawMeshTasksIndirectCountEXT-None-08880";
        set_line_width_08617                     = "VUID-vkCmdDrawMeshTasksIndirectCountEXT-None-08617";
        set_line_width_08618                     = "VUID-vkCmdDrawMeshTasksIndirectCountEXT-None-08618";
        set_depth_bias_08620                     = "VUID-vkCmdDrawMeshTasksIndirectCountEXT-None-08620";
        set_blend_constants_08621                = "VUID-vkCmdDrawMeshTasksIndirectCountEXT-None-08621";
        set_depth_bounds_08622                   = "VUID-vkCmdDrawMeshTasksIndirectCountEXT-None-08622";
        set_stencil_compare_mask_08623           = "VUID-vkCmdDrawMeshTasksIndirectCountEXT-None-08623";
        set_stencil_write_mask_08624             = "VUID-vkCmdDrawMeshTasksIndirectCountEXT-None-08624";
        set_stencil_reference_08625              = "VUID-vkCmdDrawMeshTasksIndirectCountEXT-None-08625";
        set_sample_locations_08626               = "VUID-vkCmdDrawMeshTasksIndirectCountEXT-None-08626";
        set_cull_mode_08627                      = "VUID-vkCmdDrawMeshTasksIndirectCountEXT-None-08627";
        set_front_face_08628                     = "VUID-vkCmdDrawMeshTasksIndirectCountEXT-None-08628";
        set_depth_test_enable_08629              = "VUID-vkCmdDrawMeshTasksIndirectCountEXT-None-08629";
        set_depth_write_enable_08630             = "VUID-vkCmdDrawMeshTasksIndirectCountEXT-None-08630";
        set_depth_comapre_op_08631               = "VUID-vkCmdDrawMeshTasksIndirectCountEXT-None-08631";
        set_depth_bounds_test_enable_08632       = "VUID-vkCmdDrawMeshTasksIndirectCountEXT-None-08632";
        set_stencil_test_enable_08633            = "VUID-vkCmdDrawMeshTasksIndirectCountEXT-None-08633";
        set_stencil_op_08634                     = "VUID-vkCmdDrawMeshTasksIndirectCountEXT-None-08634";
        set_line_width_08619                     = "VUID-vkCmdDrawMeshTasksIndirectCountEXT-None-08619";
        set_viewport_with_count_08642            = "VUID-vkCmdDrawMeshTasksIndirectCountEXT-primitiveFragmentShadingRateWithMultipleViewports-08642";
        alpha_component_word_08920               = "VUID-vkCmdDrawMeshTasksIndirectCountEXT-alphaToCoverageEnable-08920";
        color_write_mask_09116                   = "VUID-vkCmdDrawMeshTasksIndirectCountEXT-None-09116";
        set_clip_space_w_scaling_09232           = "VUID-vkCmdDrawMeshTasksIndirectCountEXT-None-09232";
        set_discard_rectangle_09236              = "VUID-vkCmdDrawMeshTasksIndirectCountEXT-rasterizerDiscardEnable-09236";
        set_viewport_coarse_sample_order_09233   = "VUID-vkCmdDrawMeshTasksIndirectCountEXT-shadingRateImage-09233";
        set_viewport_shading_rate_palette_09234  = "VUID-vkCmdDrawMeshTasksIndirectCountEXT-shadingRateImage-09234";
        set_exclusive_scissor_enable_09235       = "VUID-vkCmdDrawMeshTasksIndirectCountEXT-exclusiveScissor-09235";
        set_fragment_shading_rate_09238          = "VUID-vkCmdDrawMeshTasksIndirectCountEXT-pipelineFragmentShadingRate-09238";
        set_tessellation_domain_origin_09237     = "VUID-vkCmdDrawMeshTasksIndirectCountEXT-None-09237";
        rasterization_samples_07935              = "VUID-vkCmdDrawMeshTasksIndirectCountEXT-pNext-07935";
        mesh_shader_queries_07073                = "VUID-vkCmdDrawMeshTasksIndirectCountEXT-stage-07073";
        blend_advanced_07480                     = "VUID-vkCmdDrawMeshTasksIndirectCountEXT-advancedBlendMaxColorAttachments-07480";
        blend_feature_07470                      = "VUID-vkCmdDrawMeshTasksIndirectCountEXT-pColorBlendEnables-07470";
        color_attachment_08963                   = "VUID-vkCmdDrawMeshTasksIndirectCountEXT-pColorAttachments-08963";
        depth_attachment_08964                   = "VUID-vkCmdDrawMeshTasksIndirectCountEXT-pDepthAttachment-08964";
        stencil_attachment_08965                 = "VUID-vkCmdDrawMeshTasksIndirectCountEXT-pStencilAttachment-08965";
        sample_locations_07482                   = "VUID-vkCmdDrawMeshTasksIndirectCountEXT-sampleLocationsPerPixel-07482";
    }
};

struct DispatchVuidsCmdDrawIndirectByteCountEXT: DrawDispatchVuid {
    DispatchVuidsCmdDrawIndirectByteCountEXT() : DrawDispatchVuid() {
        pipeline_bound_08606                     = "VUID-vkCmdDrawIndirectByteCountEXT-None-08606";
        pipeline_or_shaders_bound_08607          = "VUID-vkCmdDrawIndirectByteCountEXT-None-08607";
        vertex_binding_04007                     = "VUID-vkCmdDrawIndirectByteCountEXT-None-04007";
        vertex_binding_null_04008                = "VUID-vkCmdDrawIndirectByteCountEXT-None-04008";
        compatible_pipeline_08600                = "VUID-vkCmdDrawIndirectByteCountEXT-None-08600";
        render_pass_compatible_02684             = "VUID-vkCmdDrawIndirectByteCountEXT-renderPass-02684";
        render_pass_began_08876                  = "VUID-vkCmdDrawIndirectByteCountEXT-None-08876";
        subpass_index_02685                      = "VUID-vkCmdDrawIndirectByteCountEXT-subpass-02685";
        sample_location_02689                    = "VUID-vkCmdDrawIndirectByteCountEXT-sampleLocationsEnable-02689";
        linear_filter_sampler_04553              = "VUID-vkCmdDrawIndirectByteCountEXT-magFilter-04553";
        linear_mipmap_sampler_04770              = "VUID-vkCmdDrawIndirectByteCountEXT-mipmapMode-04770";
        cubic_sampler_02692                      = "VUID-vkCmdDrawIndirectByteCountEXT-None-02692";
        indirect_protected_cb_02646              = "VUID-vkCmdDrawIndirectByteCountEXT-commandBuffer-02646";
        indirect_contiguous_memory_02708         = "VUID-vkCmdDrawIndirectByteCountEXT-counterBuffer-04567",
        indirect_buffer_bit_02290                = "VUID-vkCmdDrawIndirectByteCountEXT-counterBuffer-02290";
        viewport_count_03417                     = "VUID-vkCmdDrawIndirectByteCountEXT-viewportCount-03417";
        scissor_count_03418                      = "VUID-vkCmdDrawIndirectByteCountEXT-scissorCount-03418";
        viewport_scissor_count_03419             = "VUID-vkCmdDrawIndirectByteCountEXT-viewportCount-03419";
        primitive_topology_class_07500           = "VUID-vkCmdDrawIndirectByteCountEXT-dynamicPrimitiveTopologyUnrestricted-07500";
        corner_sampled_address_mode_02696        = "VUID-vkCmdDrawIndirectByteCountEXT-flags-02696";
        imageview_atomic_02691                   = "VUID-vkCmdDrawIndirectByteCountEXT-None-02691";
        bufferview_atomic_07888                  = "VUID-vkCmdDrawIndirectByteCountEXT-None-07888";
        push_constants_set_08602                 = "VUID-vkCmdDrawIndirectByteCountEXT-maintenance4-08602";
        image_subresources_render_pass_write_06537 = "VUID-vkCmdDrawIndirectByteCountEXT-None-06537";
        image_subresources_subpass_read_09003    = "VUID-vkCmdDrawIndirectByteCountEXT-None-09003";
        image_subresources_subpass_write_06539   = "VUID-vkCmdDrawIndirectByteCountEXT-None-06539";
        sampler_imageview_type_08609             = "VUID-vkCmdDrawIndirectByteCountEXT-None-08609";
        sampler_implicitLod_dref_proj_08610      = "VUID-vkCmdDrawIndirectByteCountEXT-None-08610";
        sampler_bias_offset_08611                = "VUID-vkCmdDrawIndirectByteCountEXT-None-08611";
        vertex_binding_attribute_02721           = "VUID-vkCmdDrawIndirectByteCountEXT-None-02721";
        dynamic_state_setting_commands_08608     = "VUID-vkCmdDrawIndirectByteCountEXT-None-08608";
        msrtss_rasterization_samples_07284       = "VUID-vkCmdDrawIndirectByteCountEXT-multisampledRenderToSingleSampled-07284";
        unprotected_command_buffer_02707         = "VUID-vkCmdDrawIndirectByteCountEXT-commandBuffer-02707";
        max_multiview_instance_index_02688       = "VUID-vkCmdDrawIndirectByteCountEXT-maxMultiviewInstanceIndex-02688";
        img_filter_cubic_02693                   = "VUID-vkCmdDrawIndirectByteCountEXT-None-02693";
        filter_cubic_02694                       = "VUID-vkCmdDrawIndirectByteCountEXT-filterCubic-02694";
        filter_cubic_min_max_02695               = "VUID-vkCmdDrawIndirectByteCountEXT-filterCubicMinmax-02695";
        viewport_count_primitive_shading_rate_04552 = "VUID-vkCmdDrawIndirectByteCountEXT-primitiveFragmentShadingRateWithMultipleViewports-04552";
        patch_control_points_04875               = "VUID-vkCmdDrawIndirectByteCountEXT-None-04875";
        rasterizer_discard_enable_04876          = "VUID-vkCmdDrawIndirectByteCountEXT-None-04876";
        depth_bias_enable_04877                  = "VUID-vkCmdDrawIndirectByteCountEXT-None-04877";
        logic_op_04878                           = "VUID-vkCmdDrawIndirectByteCountEXT-logicOp-04878";
        primitive_restart_enable_04879           = "VUID-vkCmdDrawIndirectByteCountEXT-None-04879";
        vertex_input_04912                       = "VUID-vkCmdDrawIndirectByteCountEXT-None-04912";
        vertex_input_binding_stride_04913        = "VUID-vkCmdDrawIndirectByteCountEXT-pStrides-04913";
        vertex_input_04914                       = "VUID-vkCmdDrawIndirectByteCountEXT-None-04914";
        blend_enable_04727                       = "VUID-vkCmdDrawIndirectByteCountEXT-blendEnable-04727";
        dynamic_discard_rectangle_07751          = "VUID-vkCmdDrawIndirectByteCountEXT-None-07751";
        dynamic_discard_rectangle_enable_07880   = "VUID-vkCmdDrawIndirectByteCountEXT-None-07880";
        dynamic_discard_rectangle_mode_07881     =  "VUID-vkCmdDrawIndirectByteCountEXT-None-07881";
        dynamic_exclusive_scissor_enable_07878   = "VUID-vkCmdDrawIndirectByteCountEXT-None-07878";
        dynamic_exclusive_scissor_07879          = "VUID-vkCmdDrawIndirectByteCountEXT-None-07879";
        dynamic_color_write_enable_07749         = "VUID-vkCmdDrawIndirectByteCountEXT-None-07749";
        dynamic_color_write_enable_count_07750   = "VUID-vkCmdDrawIndirectByteCountEXT-attachmentCount-07750";
        dynamic_attachment_feedback_loop_08877   = "VUID-vkCmdDrawIndirectByteCountEXT-None-08877";
        dynamic_rendering_view_mask_06178        = "VUID-vkCmdDrawIndirectByteCountEXT-viewMask-06178";
        dynamic_rendering_color_count_06179      = "VUID-vkCmdDrawIndirectByteCountEXT-colorAttachmentCount-06179";
        dynamic_rendering_color_formats_08910    = "VUID-vkCmdDrawIndirectByteCountEXT-dynamicRenderingUnusedAttachments-08910";
        dynamic_rendering_unused_attachments_08911 = "VUID-vkCmdDrawIndirectByteCountEXT-dynamicRenderingUnusedAttachments-08911";
        dynamic_rendering_undefined_color_formats_08912 = "VUID-vkCmdDrawIndirectByteCountEXT-dynamicRenderingUnusedAttachments-08912";
        dynamic_rendering_undefined_depth_format_08916 = "VUID-vkCmdDrawIndirectByteCountEXT-dynamicRenderingUnusedAttachments-08916";
        dynamic_rendering_undefined_stencil_format_08916 = "VUID-vkCmdDrawIndirectByteCountEXT-dynamicRenderingUnusedAttachments-08916";
        dynamic_rendering_depth_format_08914     = "VUID-vkCmdDrawIndirectByteCountEXT-dynamicRenderingUnusedAttachments-08914";
        dynamic_rendering_unused_attachments_08915 = "VUID-vkCmdDrawIndirectByteCountEXT-dynamicRenderingUnusedAttachments-08915";
        dynamic_rendering_stencil_format_08917   = "VUID-vkCmdDrawIndirectByteCountEXT-dynamicRenderingUnusedAttachments-08917";
        dynamic_rendering_unused_attachments_08918 = "VUID-vkCmdDrawIndirectByteCountEXT-dynamicRenderingUnusedAttachments-08918";
        dynamic_rendering_fsr_06183              = "VUID-vkCmdDrawIndirectByteCountEXT-imageView-06183";
        dynamic_rendering_fdm_06184              = "VUID-vkCmdDrawIndirectByteCountEXT-imageView-06184";
        dynamic_rendering_color_sample_06185     = "VUID-vkCmdDrawIndirectByteCountEXT-colorAttachmentCount-06185";
        dynamic_rendering_depth_sample_06186     = "VUID-vkCmdDrawIndirectByteCountEXT-pDepthAttachment-06186";
        dynamic_rendering_stencil_sample_06187   = "VUID-vkCmdDrawIndirectByteCountEXT-pStencilAttachment-06187";
        dynamic_rendering_06198                  = "VUID-vkCmdDrawIndirectByteCountEXT-renderPass-06198";
        dynamic_rendering_07285                  = "VUID-vkCmdDrawIndirectByteCountEXT-multisampledRenderToSingleSampled-07285";
        dynamic_rendering_07286                  = "VUID-vkCmdDrawIndirectByteCountEXT-multisampledRenderToSingleSampled-07286";
        dynamic_rendering_07287                  = "VUID-vkCmdDrawIndirectByteCountEXT-multisampledRenderToSingleSampled-07287";
        image_view_access_64_04470               = "VUID-vkCmdDrawIndirectByteCountEXT-SampledType-04470";
        image_view_access_32_04471               = "VUID-vkCmdDrawIndirectByteCountEXT-SampledType-04471";
        image_view_sparse_64_04474               = "VUID-vkCmdDrawIndirectByteCountEXT-sparseImageInt64Atomics-04474";
        buffer_view_access_64_04472              = "VUID-vkCmdDrawIndirectByteCountEXT-SampledType-04472";
        buffer_view_access_32_04473              = "VUID-vkCmdDrawIndirectByteCountEXT-SampledType-04473";
        storage_image_read_without_format_07028  = "VUID-vkCmdDrawIndirectByteCountEXT-OpTypeImage-07028";
        storage_image_write_without_format_07027 = "VUID-vkCmdDrawIndirectByteCountEXT-OpTypeImage-07027";
        storage_texel_buffer_read_without_format_07030  = "VUID-vkCmdDrawIndirectByteCountEXT-OpTypeImage-07030";
        storage_texel_buffer_write_without_format_07029 = "VUID-vkCmdDrawIndirectByteCountEXT-OpTypeImage-07029";
        storage_image_write_texel_count_08795           = "VUID-vkCmdDrawIndirectByteCountEXT-OpImageWrite-08795";
        storage_image_write_texel_count_08796           = "VUID-vkCmdDrawIndirectByteCountEXT-OpImageWrite-08796";
        storage_texel_buffer_write_texel_count_04469    = "VUID-vkCmdDrawIndirectByteCountEXT-OpImageWrite-04469";
        depth_compare_sample_06479               = "VUID-vkCmdDrawIndirectByteCountEXT-None-06479";
        depth_read_only_06886                    = "VUID-vkCmdDrawIndirectByteCountEXT-None-06886";
        stencil_read_only_06887                  = "VUID-vkCmdDrawIndirectByteCountEXT-None-06887";
        dynamic_sample_locations_06666           = "VUID-vkCmdDrawIndirectByteCountEXT-None-06666";
        dynamic_tessellation_domain_origin_07619 = "VUID-vkCmdDrawIndirectByteCountEXT-None-07619";
        dynamic_depth_clamp_enable_07620         = "VUID-vkCmdDrawIndirectByteCountEXT-None-07620";
        dynamic_polygon_mode_07621               = "VUID-vkCmdDrawIndirectByteCountEXT-None-07621";
        dynamic_rasterization_samples_07622      = "VUID-vkCmdDrawIndirectByteCountEXT-None-07622";
        dynamic_sample_mask_07623                = "VUID-vkCmdDrawIndirectByteCountEXT-None-07623";
        dynamic_alpha_to_coverage_enable_07624   = "VUID-vkCmdDrawIndirectByteCountEXT-None-07624";
        dynamic_alpha_to_coverage_component_08919 = "VUID-vkCmdDrawIndirectByteCountEXT-alphaToCoverageEnable-08919";
        dynamic_alpha_to_one_enable_07625        = "VUID-vkCmdDrawIndirectByteCountEXT-None-07625";
        dynamic_logic_op_enable_07626            = "VUID-vkCmdDrawIndirectByteCountEXT-None-07626";
        dynamic_color_blend_enable_07476         = "VUID-vkCmdDrawIndirectByteCountEXT-firstAttachment-07476";
        dynamic_color_blend_equation_07477       = "VUID-vkCmdDrawIndirectByteCountEXT-firstAttachment-07477";
        dynamic_color_write_mask_07478           = "VUID-vkCmdDrawIndirectByteCountEXT-firstAttachment-07478";
        dynamic_rasterization_stream_07630       = "VUID-vkCmdDrawIndirectByteCountEXT-None-07630";
        dynamic_conservative_rasterization_mode_07631 = "VUID-vkCmdDrawIndirectByteCountEXT-None-07631";
        dynamic_extra_primitive_overestimation_size_07632 = "VUID-vkCmdDrawIndirectByteCountEXT-None-07632";
        dynamic_depth_clip_enable_07633          = "VUID-vkCmdDrawIndirectByteCountEXT-None-07633";
        dynamic_sample_locations_enable_07634    = "VUID-vkCmdDrawIndirectByteCountEXT-None-07634";
        dynamic_color_blend_advanced_07479       = "VUID-vkCmdDrawIndirectByteCountEXT-firstAttachment-07479";
        dynamic_provoking_vertex_mode_07636      = "VUID-vkCmdDrawIndirectByteCountEXT-None-07636";
        dynamic_line_rasterization_mode_07637    = "VUID-vkCmdDrawIndirectByteCountEXT-None-07637";
        dynamic_line_stipple_enable_07638        = "VUID-vkCmdDrawIndirectByteCountEXT-None-07638";
        dynamic_depth_clip_negative_one_to_one_07639 = "VUID-vkCmdDrawIndirectByteCountEXT-None-07639";
        dynamic_viewport_w_scaling_enable_07640  = "VUID-vkCmdDrawIndirectByteCountEXT-None-07640";
        dynamic_viewport_swizzle_07641           = "VUID-vkCmdDrawIndirectByteCountEXT-None-07641";
        dynamic_coverage_to_color_enable_07642   = "VUID-vkCmdDrawIndirectByteCountEXT-None-07642";
        dynamic_coverage_to_color_location_07643 = "VUID-vkCmdDrawIndirectByteCountEXT-None-07643";
        dynamic_coverage_modulation_mode_07644   = "VUID-vkCmdDrawIndirectByteCountEXT-None-07644";
        dynamic_coverage_modulation_table_enable_07645 = "VUID-vkCmdDrawIndirectByteCountEXT-None-07645";
        dynamic_coverage_modulation_table_07646  = "VUID-vkCmdDrawIndirectByteCountEXT-None-07646";
        dynamic_shading_rate_image_enable_07647  = "VUID-vkCmdDrawIndirectByteCountEXT-None-07647";
        dynamic_representative_fragment_test_enable_07648  = "VUID-vkCmdDrawIndirectByteCountEXT-None-07648";
        dynamic_coverage_reduction_mode_07649    = "VUID-vkCmdDrawIndirectByteCountEXT-None-07649";
        dynamic_viewport_07831                   = "VUID-vkCmdDrawIndirectByteCountEXT-None-07831";
        dynamic_scissor_07832                    = "VUID-vkCmdDrawIndirectByteCountEXT-None-07832";
        dynamic_depth_bias_07834                 = "VUID-vkCmdDrawIndirectByteCountEXT-None-07834";
        dynamic_line_width_07833                 = "VUID-vkCmdDrawIndirectByteCountEXT-None-07833";
        dynamic_line_stipple_ext_07849           = "VUID-vkCmdDrawIndirectByteCountEXT-None-07849";
        dynamic_blend_constants_07835            = "VUID-vkCmdDrawIndirectByteCountEXT-None-07835";
        dynamic_depth_bounds_07836               = "VUID-vkCmdDrawIndirectByteCountEXT-None-07836";
        dynamic_depth_enable_08715               = "VUID-vkCmdDrawIndirectByteCountEXT-pDynamicStates-08715";
        dynamic_stencil_compare_mask_07837       = "VUID-vkCmdDrawIndirectByteCountEXT-None-07837";
        dynamic_stencil_write_mask_07838         = "VUID-vkCmdDrawIndirectByteCountEXT-None-07838";
        dynamic_stencil_write_mask_08716         = "VUID-vkCmdDrawIndirectByteCountEXT-pDynamicStates-08716";
        dynamic_stencil_reference_07839          = "VUID-vkCmdDrawIndirectByteCountEXT-None-07839";
        dynamic_state_inherited_07850            = "VUID-vkCmdDrawIndirectByteCountEXT-None-07850";
        dynamic_cull_mode_07840                  = "VUID-vkCmdDrawIndirectByteCountEXT-None-07840";
        dynamic_front_face_07841                 = "VUID-vkCmdDrawIndirectByteCountEXT-None-07841";
        dynamic_primitive_topology_07842         = "VUID-vkCmdDrawIndirectByteCountEXT-None-07842";
        dynamic_depth_test_enable_07843          = "VUID-vkCmdDrawIndirectByteCountEXT-None-07843";
        dynamic_depth_write_enable_07844         = "VUID-vkCmdDrawIndirectByteCountEXT-None-07844";
        dynamic_depth_compare_op_07845           = "VUID-vkCmdDrawIndirectByteCountEXT-None-07845";
        dynamic_depth_bound_test_enable_07846    = "VUID-vkCmdDrawIndirectByteCountEXT-None-07846";
        dynamic_stencil_test_enable_07847        = "VUID-vkCmdDrawIndirectByteCountEXT-None-07847";
        dynamic_stencil_op_07848                 = "VUID-vkCmdDrawIndirectByteCountEXT-None-07848";
        primitives_generated_06708               = "VUID-vkCmdDrawIndirectByteCountEXT-primitivesGeneratedQueryWithRasterizerDiscard-06708";
        primitives_generated_streams_06709       = "VUID-vkCmdDrawIndirectByteCountEXT-primitivesGeneratedQueryWithNonZeroStreams-06709";
        invalid_mesh_shader_stages_06481         = "VUID-vkCmdDrawIndirectByteCountEXT-stage-06481";
        descriptor_buffer_bit_set_08114          = "VUID-vkCmdDrawIndirectByteCountEXT-None-08114";
        descriptor_buffer_bit_not_set_08115      = "VUID-vkCmdDrawIndirectByteCountEXT-None-08115";
        descriptor_buffer_set_offset_missing_08117 = "VUID-vkCmdDrawIndirectByteCountEXT-None-08117";
        image_view_dim_07752                     = "VUID-vkCmdDrawIndirectByteCountEXT-viewType-07752";
        image_view_numeric_format_07753          = "VUID-vkCmdDrawIndirectByteCountEXT-format-07753";
        stippled_rectangular_lines_07495         = "VUID-vkCmdDrawIndirectByteCountEXT-stippledLineEnable-07495";
        stippled_bresenham_lines_07496           = "VUID-vkCmdDrawIndirectByteCountEXT-stippledLineEnable-07496";
        stippled_smooth_lines_07497              = "VUID-vkCmdDrawIndirectByteCountEXT-stippledLineEnable-07497";
        stippled_default_strict_07498            = "VUID-vkCmdDrawIndirectByteCountEXT-stippledLineEnable-07498";
        viewport_and_scissor_with_count_08635    = "VUID-vkCmdDrawIndirectByteCountEXT-None-08635";
        viewport_w_scaling_08636                 = "VUID-vkCmdDrawIndirectByteCountEXT-None-08636";
        shading_rate_palette_08637               = "VUID-vkCmdDrawIndirectByteCountEXT-None-08637";
        exclusive_scissor_08638                  = "VUID-vkCmdDrawIndirectByteCountEXT-None-08638";
        external_format_resolve_09362            = "VUID-vkCmdDrawIndirectByteCountEXT-colorAttachmentCount-09362";
        external_format_resolve_09363            = "VUID-vkCmdDrawIndirectByteCountEXT-None-09363";
        external_format_resolve_09364            = "VUID-vkCmdDrawIndirectByteCountEXT-None-09364";
        external_format_resolve_09365            = "VUID-vkCmdDrawIndirectByteCountEXT-None-09365";
        external_format_resolve_09368            = "VUID-vkCmdDrawIndirectByteCountEXT-None-09368";
        external_format_resolve_09369            = "VUID-vkCmdDrawIndirectByteCountEXT-None-09369";
        external_format_resolve_09372            = "VUID-vkCmdDrawIndirectByteCountEXT-colorAttachmentCount-09372";
        set_rasterizer_discard_enable_08639      = "VUID-vkCmdDrawIndirectByteCountEXT-None-08639";
        set_depth_bias_enable_08640              = "VUID-vkCmdDrawIndirectByteCountEXT-None-08640";
        set_logic_op_08641                       = "VUID-vkCmdDrawIndirectByteCountEXT-None-08641";
        set_color_blend_enable_08643             = "VUID-vkCmdDrawIndirectByteCountEXT-None-08643";
        set_rasterization_samples_08644              = "VUID-vkCmdDrawIndirectByteCountEXT-None-08644";
        set_color_write_enable_08646             = "VUID-vkCmdDrawIndirectByteCountEXT-None-08646";
        set_color_write_enable_08647             = "VUID-vkCmdDrawIndirectByteCountEXT-None-08647";
        set_discard_rectangles_enable_08648      = "VUID-vkCmdDrawIndirectByteCountEXT-None-08648";
        set_discard_rectangles_mode_08649      = "VUID-vkCmdDrawIndirectByteCountEXT-None-08649";
        set_depth_clamp_enable_08650             = "VUID-vkCmdDrawIndirectByteCountEXT-None-08650";
        set_polygon_mode_08651                   = "VUID-vkCmdDrawIndirectByteCountEXT-None-08651";
        set_rasterization_samples_08652          = "VUID-vkCmdDrawIndirectByteCountEXT-None-08652";
        set_sample_mask_08653                    = "VUID-vkCmdDrawIndirectByteCountEXT-None-08653";
        set_alpha_to_coverage_enable_08654       = "VUID-vkCmdDrawIndirectByteCountEXT-None-08654";
        set_alpha_to_one_enable_08655            = "VUID-vkCmdDrawIndirectByteCountEXT-None-08655";
        set_logic_op_enable_08656                = "VUID-vkCmdDrawIndirectByteCountEXT-None-08656";
        set_color_blend_enable_08657             = "VUID-vkCmdDrawIndirectByteCountEXT-None-08657";
        set_color_blend_equation_08658           = "VUID-vkCmdDrawIndirectByteCountEXT-None-08658";
        set_color_write_mask_08659               = "VUID-vkCmdDrawIndirectByteCountEXT-None-08659";
        set_rasterization_streams_08660          = "VUID-vkCmdDrawIndirectByteCountEXT-None-08660";
        set_conservative_rasterization_mode_08661 = "VUID-vkCmdDrawIndirectByteCountEXT-None-08661";
        set_extra_primitive_overestimation_size_08662 = "VUID-vkCmdDrawIndirectByteCountEXT-None-08662";
        set_depth_clip_enable_08663              = "VUID-vkCmdDrawIndirectByteCountEXT-None-08663";
        set_sample_locations_enable_08664        = "VUID-vkCmdDrawIndirectByteCountEXT-None-08664";
        set_provoking_vertex_mode_08665          = "VUID-vkCmdDrawIndirectByteCountEXT-None-08665";
        set_line_rasterization_mode_08666        = "VUID-vkCmdDrawIndirectByteCountEXT-None-08666";
        set_line_rasterization_mode_08667        = "VUID-vkCmdDrawIndirectByteCountEXT-None-08667";
        set_line_rasterization_mode_08668        = "VUID-vkCmdDrawIndirectByteCountEXT-None-08668";
        set_line_stipple_enable_08669            = "VUID-vkCmdDrawIndirectByteCountEXT-None-08669";
        set_line_stipple_enable_08670            = "VUID-vkCmdDrawIndirectByteCountEXT-None-08670";
        set_line_stipple_enable_08671            = "VUID-vkCmdDrawIndirectByteCountEXT-None-08671";
        set_line_stipple_08672                   = "VUID-vkCmdDrawIndirectByteCountEXT-None-08672";
        set_depth_clip_negative_one_to_one_08673 = "VUID-vkCmdDrawIndirectByteCountEXT-None-08673";
        set_viewport_w_scaling_enable_08674      = "VUID-vkCmdDrawIndirectByteCountEXT-None-08674";
        set_viewport_swizzle_08675               = "VUID-vkCmdDrawIndirectByteCountEXT-None-08675";
        set_coverage_to_color_enable_08676       = "VUID-vkCmdDrawIndirectByteCountEXT-None-08676";
        set_coverage_to_color_location_08677     = "VUID-vkCmdDrawIndirectByteCountEXT-None-08677";
        set_coverage_modulation_mode_08678       = "VUID-vkCmdDrawIndirectByteCountEXT-None-08678";
        set_coverage_modulation_table_enable_08679 = "VUID-vkCmdDrawIndirectByteCountEXT-None-08679";
        set_coverage_modulation_table_08680      = "VUID-vkCmdDrawIndirectByteCountEXT-None-08680";
        set_shading_rate_image_enable_08681      = "VUID-vkCmdDrawIndirectByteCountEXT-None-08681";
        set_representative_fragment_test_enable_08682 = "VUID-vkCmdDrawIndirectByteCountEXT-None-08682";
        set_coverage_reduction_mode_08683        = "VUID-vkCmdDrawIndirectByteCountEXT-None-08683";
        vertex_shader_08684                      = "VUID-vkCmdDrawIndirectByteCountEXT-None-08684";
        tessellation_control_shader_08685        = "VUID-vkCmdDrawIndirectByteCountEXT-None-08685";
        tessellation_evaluation_shader_08686     = "VUID-vkCmdDrawIndirectByteCountEXT-None-08686";
        geometry_shader_08687                    = "VUID-vkCmdDrawIndirectByteCountEXT-None-08687";
        fragment_shader_08688                    = "VUID-vkCmdDrawIndirectByteCountEXT-None-08688";
        task_shader_08689                        = "VUID-vkCmdDrawIndirectByteCountEXT-None-08689";
        mesh_shader_08690                        = "VUID-vkCmdDrawIndirectByteCountEXT-None-08690";
        vert_mesh_shader_08693                   = "VUID-vkCmdDrawIndirectByteCountEXT-None-08693";
        task_mesh_shader_08694                   = "VUID-vkCmdDrawIndirectByteCountEXT-None-08694";
        task_mesh_shader_08695                   = "VUID-vkCmdDrawIndirectByteCountEXT-None-08695";
        vert_task_mesh_shader_08696              = "VUID-vkCmdDrawIndirectByteCountEXT-None-08696";
        linked_shaders_08698                     = "VUID-vkCmdDrawIndirectByteCountEXT-None-08698";
        linked_shaders_08699                     = "VUID-vkCmdDrawIndirectByteCountEXT-None-08699";
        shaders_push_constants_08878             = "VUID-vkCmdDrawIndirectByteCountEXT-None-08878";
        shaders_descriptor_layouts_08879         = "VUID-vkCmdDrawIndirectByteCountEXT-None-08879";
        set_attachment_feedback_loop_enable_08880 = "VUID-vkCmdDrawIndirectByteCountEXT-None-08880";
        set_vertex_input_08882                   = "VUID-vkCmdDrawIndirectByteCountEXT-None-08882";
        draw_shaders_no_task_mesh_08885          = "VUID-vkCmdDrawIndirectByteCountEXT-None-08885";
        set_line_width_08617                     = "VUID-vkCmdDrawIndirectByteCountEXT-None-08617";
        set_line_width_08618                     = "VUID-vkCmdDrawIndirectByteCountEXT-None-08618";
        set_depth_bias_08620                     = "VUID-vkCmdDrawIndirectByteCountEXT-None-08620";
        set_blend_constants_08621                = "VUID-vkCmdDrawIndirectByteCountEXT-None-08621";
        set_depth_bounds_08622                   = "VUID-vkCmdDrawIndirectByteCountEXT-None-08622";
        set_stencil_compare_mask_08623           = "VUID-vkCmdDrawIndirectByteCountEXT-None-08623";
        set_stencil_write_mask_08624             = "VUID-vkCmdDrawIndirectByteCountEXT-None-08624";
        set_stencil_reference_08625              = "VUID-vkCmdDrawIndirectByteCountEXT-None-08625";
        set_sample_locations_08626               = "VUID-vkCmdDrawIndirectByteCountEXT-None-08626";
        set_cull_mode_08627                      = "VUID-vkCmdDrawIndirectByteCountEXT-None-08627";
        set_front_face_08628                     = "VUID-vkCmdDrawIndirectByteCountEXT-None-08628";
        set_depth_test_enable_08629              = "VUID-vkCmdDrawIndirectByteCountEXT-None-08629";
        set_depth_write_enable_08630             = "VUID-vkCmdDrawIndirectByteCountEXT-None-08630";
        set_depth_comapre_op_08631               = "VUID-vkCmdDrawIndirectByteCountEXT-None-08631";
        set_depth_bounds_test_enable_08632       = "VUID-vkCmdDrawIndirectByteCountEXT-None-08632";
        set_stencil_test_enable_08633            = "VUID-vkCmdDrawIndirectByteCountEXT-None-08633";
        set_stencil_op_08634                     = "VUID-vkCmdDrawIndirectByteCountEXT-None-08634";
        set_line_width_08619                     = "VUID-vkCmdDrawIndirectByteCountEXT-None-08619";
        set_viewport_with_count_08642            = "VUID-vkCmdDrawIndirectByteCountEXT-primitiveFragmentShadingRateWithMultipleViewports-08642";
        alpha_component_word_08920               = "VUID-vkCmdDrawIndirectByteCountEXT-alphaToCoverageEnable-08920";
        color_write_mask_09116                   = "VUID-vkCmdDrawIndirectByteCountEXT-None-09116";
        vertex_input_format_08936                = "VUID-vkCmdDrawIndirectByteCountEXT-format-08936";
        vertex_input_format_08937                = "VUID-vkCmdDrawIndirectByteCountEXT-format-08937";
        vertex_input_format_09203                = "VUID-vkCmdDrawIndirectByteCountEXT-None-09203";
        vertex_input_format_07939                = "VUID-vkCmdDrawIndirectByteCountEXT-Input-07939";
        set_clip_space_w_scaling_09232           = "VUID-vkCmdDrawIndirectByteCountEXT-None-09232";
        set_discard_rectangle_09236              = "VUID-vkCmdDrawIndirectByteCountEXT-rasterizerDiscardEnable-09236";
        set_viewport_coarse_sample_order_09233   = "VUID-vkCmdDrawIndirectByteCountEXT-shadingRateImage-09233";
        set_viewport_shading_rate_palette_09234  = "VUID-vkCmdDrawIndirectByteCountEXT-shadingRateImage-09234";
        set_exclusive_scissor_enable_09235       = "VUID-vkCmdDrawIndirectByteCountEXT-exclusiveScissor-09235";
        set_fragment_shading_rate_09238          = "VUID-vkCmdDrawIndirectByteCountEXT-pipelineFragmentShadingRate-09238";
        set_tessellation_domain_origin_09237     = "VUID-vkCmdDrawIndirectByteCountEXT-None-09237";
        rasterization_samples_07935              = "VUID-vkCmdDrawIndirectByteCountEXT-pNext-07935";
        mesh_shader_queries_07073                = "VUID-vkCmdDrawIndirectByteCountEXT-stage-07073";
        blend_advanced_07480                     = "VUID-vkCmdDrawIndirectByteCountEXT-advancedBlendMaxColorAttachments-07480";
        blend_feature_07470                      = "VUID-vkCmdDrawIndirectByteCountEXT-pColorBlendEnables-07470";
        color_attachment_08963                   = "VUID-vkCmdDrawIndirectByteCountEXT-pColorAttachments-08963";
        depth_attachment_08964                   = "VUID-vkCmdDrawIndirectByteCountEXT-pDepthAttachment-08964";
        stencil_attachment_08965                 = "VUID-vkCmdDrawIndirectByteCountEXT-pStencilAttachment-08965";
        sample_locations_07482                   = "VUID-vkCmdDrawIndirectByteCountEXT-sampleLocationsPerPixel-07482";
    }
};

struct DispatchVuidsCmdDispatchBase: DrawDispatchVuid {
    DispatchVuidsCmdDispatchBase() : DrawDispatchVuid() {
        pipeline_bound_08606                     = "VUID-vkCmdDispatchBase-None-08606";
        pipeline_or_shaders_bound_08607          = "VUID-vkCmdDispatchBase-None-08607";
        compatible_pipeline_08600                = "VUID-vkCmdDispatchBase-None-08600";
        linear_filter_sampler_04553              = "VUID-vkCmdDispatchBase-magFilter-04553";
        linear_mipmap_sampler_04770              = "VUID-vkCmdDispatchBase-mipmapMode-04770";
        cubic_sampler_02692                      = "VUID-vkCmdDispatchBase-None-02692";
        corner_sampled_address_mode_02696        = "VUID-vkCmdDispatchBase-flags-02696";
        imageview_atomic_02691                   = "VUID-vkCmdDispatchBase-None-02691";
        bufferview_atomic_07888                  = "VUID-vkCmdDispatchBase-None-07888";
        push_constants_set_08602                 = "VUID-vkCmdDispatchBase-maintenance4-08602";
        sampler_imageview_type_08609             = "VUID-vkCmdDispatchBase-None-08609";
        sampler_implicitLod_dref_proj_08610      = "VUID-vkCmdDispatchBase-None-08610";
        sampler_bias_offset_08611                = "VUID-vkCmdDispatchBase-None-08611";
        dynamic_state_setting_commands_08608     = "VUID-vkCmdDispatchBase-None-08608";
        unprotected_command_buffer_02707         = "VUID-vkCmdDispatchBase-commandBuffer-02707";
        protected_command_buffer_02712           = "VUID-vkCmdDispatchBase-commandBuffer-02712";
        img_filter_cubic_02693                   = "VUID-vkCmdDispatchBase-None-02693";
        filter_cubic_02694                       = "VUID-vkCmdDispatchBase-filterCubic-02694";
        filter_cubic_min_max_02695               = "VUID-vkCmdDispatchBase-filterCubicMinmax-02695";
        image_view_access_64_04470               = "VUID-vkCmdDispatchBase-SampledType-04470";
        image_view_access_32_04471               = "VUID-vkCmdDispatchBase-SampledType-04471";
        image_view_sparse_64_04474               = "VUID-vkCmdDispatchBase-sparseImageInt64Atomics-04474";
        buffer_view_access_64_04472              = "VUID-vkCmdDispatchBase-SampledType-04472";
        buffer_view_access_32_04473              = "VUID-vkCmdDispatchBase-SampledType-04473";
        storage_image_read_without_format_07028  = "VUID-vkCmdDispatchBase-OpTypeImage-07028";
        storage_image_write_without_format_07027 = "VUID-vkCmdDispatchBase-OpTypeImage-07027";
        storage_texel_buffer_read_without_format_07030  = "VUID-vkCmdDispatchBase-OpTypeImage-07030";
        storage_texel_buffer_write_without_format_07029 = "VUID-vkCmdDispatchBase-OpTypeImage-07029";
        storage_image_write_texel_count_08795           = "VUID-vkCmdDispatchBase-OpImageWrite-08795";
        storage_image_write_texel_count_08796           = "VUID-vkCmdDispatchBase-OpImageWrite-08796";
        storage_texel_buffer_write_texel_count_04469    = "VUID-vkCmdDispatchBase-OpImageWrite-04469";
        depth_compare_sample_06479               = "VUID-vkCmdDispatchBase-None-06479";
        descriptor_buffer_bit_set_08114          = "VUID-vkCmdDispatchBase-None-08114";
        descriptor_buffer_bit_not_set_08115      = "VUID-vkCmdDispatchBase-None-08115";
        descriptor_buffer_set_offset_missing_08117 = "VUID-vkCmdDispatchBase-None-08117";
        image_view_dim_07752                     = "VUID-vkCmdDispatchBase-viewType-07752";
        image_view_numeric_format_07753          = "VUID-vkCmdDispatchBase-format-07753";
    }
};

using Func = vvl::Func;
// This LUT is created to allow a static listing of each VUID that is covered by drawdispatch commands
static const std::map<Func, DrawDispatchVuid> kDrawdispatchVuid = {
    {Func::vkCmdDraw, DispatchVuidsCmdDraw()},
    {Func::vkCmdDrawMultiEXT, DispatchVuidsCmdDrawMultiEXT()},
    {Func::vkCmdDrawIndexed, DispatchVuidsCmdDrawIndexed()},
    {Func::vkCmdDrawMultiIndexedEXT, DispatchVuidsCmdDrawMultiIndexedEXT()},
    {Func::vkCmdDrawIndirect, DispatchVuidsCmdDrawIndirect()},
    {Func::vkCmdDrawIndexedIndirect, DispatchVuidsCmdDrawIndexedIndirect()},
    {Func::vkCmdDispatch, DispatchVuidsCmdDispatch()},
    {Func::vkCmdDispatchIndirect, DispatchVuidsCmdDispatchIndirect()},
    {Func::vkCmdDrawIndirectCount, DispatchVuidsCmdDrawIndirectCount()},
    {Func::vkCmdDrawIndirectCountKHR , DispatchVuidsCmdDrawIndirectCount()},
    {Func::vkCmdDrawIndexedIndirectCount, DispatchVuidsCmdDrawIndexedIndirectCount()},
    {Func::vkCmdDrawIndexedIndirectCountKHR, DispatchVuidsCmdDrawIndexedIndirectCount()},
    {Func::vkCmdTraceRaysNV, DispatchVuidsCmdTraceRaysNV()},
    {Func::vkCmdTraceRaysKHR, DispatchVuidsCmdTraceRaysKHR()},
    {Func::vkCmdTraceRaysIndirectKHR, DispatchVuidsCmdTraceRaysIndirectKHR()},
    {Func::vkCmdTraceRaysIndirect2KHR, DispatchVuidsCmdTraceRaysIndirect2KHR()},
    {Func::vkCmdDrawMeshTasksNV, DispatchVuidsCmdDrawMeshTasksNV()},
    {Func::vkCmdDrawMeshTasksIndirectNV, DispatchVuidsCmdDrawMeshTasksIndirectNV()},
    {Func::vkCmdDrawMeshTasksIndirectCountNV, DispatchVuidsCmdDrawMeshTasksIndirectCountNV()},
    {Func::vkCmdDrawMeshTasksEXT, DispatchVuidsCmdDrawMeshTasksEXT()},
    {Func::vkCmdDrawMeshTasksIndirectEXT, DispatchVuidsCmdDrawMeshTasksIndirectEXT()},
    {Func::vkCmdDrawMeshTasksIndirectCountEXT, DispatchVuidsCmdDrawMeshTasksIndirectCountEXT()},
    {Func::vkCmdDrawIndirectByteCountEXT, DispatchVuidsCmdDrawIndirectByteCountEXT()},
    {Func::vkCmdDispatchBase, DispatchVuidsCmdDispatchBase()},
    {Func::vkCmdDispatchBaseKHR, DispatchVuidsCmdDispatchBase()},
    // Used if invalid function is used
    {Func::Empty, DrawDispatchVuid()}
};
// clang-format on

// Getter function to provide kVUIDUndefined in case an invalid function is passed in. Likely if new extension adds command and
// VUIDs are not added yet
const DrawDispatchVuid &CoreChecks::GetDrawDispatchVuid(Func function) const {
    if (kDrawdispatchVuid.find(function) != kDrawdispatchVuid.cend()) {
        return kDrawdispatchVuid.at(function);
    } else {
        return kDrawdispatchVuid.at(Func::Empty);
    }
}

bool CoreChecks::ValidateGraphicsIndexedCmd(const CMD_BUFFER_STATE &cb_state, const Location &loc) const {
    bool skip = false;
    const DrawDispatchVuid &vuid = GetDrawDispatchVuid(loc.function);
    if (!cb_state.index_buffer_binding.bound()) {
        skip |= LogError(vuid.index_binding_07312, cb_state.GetObjectList(VK_PIPELINE_BIND_POINT_GRAPHICS), loc,
                         "Index buffer object has not been bound to this command buffer.");
    }
    return skip;
}

bool CoreChecks::ValidateCmdDrawInstance(const CMD_BUFFER_STATE &cb_state, uint32_t instanceCount, uint32_t firstInstance,
                                         const Location &loc) const {
    bool skip = false;
    const DrawDispatchVuid &vuid = GetDrawDispatchVuid(loc.function);

    // Verify maxMultiviewInstanceIndex
    if (cb_state.activeRenderPass && enabled_features.multiview &&
        ((static_cast<uint64_t>(instanceCount) + static_cast<uint64_t>(firstInstance)) >
         static_cast<uint64_t>(phys_dev_props_core11.maxMultiviewInstanceIndex))) {
        LogObjectList objlist = cb_state.GetObjectList(VK_PIPELINE_BIND_POINT_GRAPHICS);
        objlist.add(cb_state.activeRenderPass->Handle());
        skip |= LogError(vuid.max_multiview_instance_index_02688, objlist, loc,
                         "renderpass instance has multiview enabled, and maxMultiviewInstanceIndex: %" PRIu32
                         ", but instanceCount: %" PRIu32 "and firstInstance: %" PRIu32 ".",
                         phys_dev_props_core11.maxMultiviewInstanceIndex, instanceCount, firstInstance);
    }
    return skip;
}

bool CoreChecks::ValidateVTGShaderStages(const CMD_BUFFER_STATE &cb_state, const Location &loc) const {
    bool skip = false;
    const DrawDispatchVuid &vuid = GetDrawDispatchVuid(loc.function);

    const auto *pipeline_state = cb_state.lastBound[VK_PIPELINE_BIND_POINT_GRAPHICS].pipeline_state;
    if (pipeline_state && pipeline_state->active_shaders & (VK_SHADER_STAGE_TASK_BIT_EXT | VK_SHADER_STAGE_MESH_BIT_EXT)) {
        skip |= LogError(
            vuid.invalid_mesh_shader_stages_06481, cb_state.GetObjectList(VK_PIPELINE_BIND_POINT_GRAPHICS), loc,
            "The bound graphics pipeline must not have been created with "
            "VK_SHADER_STAGE_TASK_BIT_EXT or VK_SHADER_STAGE_MESH_BIT_EXT. Active shader stages on the bound pipeline are %s.",
            string_VkShaderStageFlags(pipeline_state->active_shaders).c_str());
    }
    return skip;
}

bool CoreChecks::ValidateMeshShaderStage(const CMD_BUFFER_STATE &cb_state, const Location &loc, bool is_NV) const {
    bool skip = false;
    const DrawDispatchVuid &vuid = GetDrawDispatchVuid(loc.function);

    const auto *pipeline_state = cb_state.lastBound[VK_PIPELINE_BIND_POINT_GRAPHICS].pipeline_state;
    if (pipeline_state && !(pipeline_state->active_shaders & VK_SHADER_STAGE_MESH_BIT_EXT)) {
        skip |= LogError(vuid.missing_mesh_shader_stages_07080, cb_state.GetObjectList(VK_PIPELINE_BIND_POINT_GRAPHICS), loc,
                         "The current pipeline bound to VK_PIPELINE_BIND_POINT_GRAPHICS must contain a shader stage using the "
                         "%s Execution Model. Active shader stages on the bound pipeline are %s.",
                         is_NV ? "MeshNV" : "MeshEXT", string_VkShaderStageFlags(pipeline_state->active_shaders).c_str());
    }
    if (pipeline_state &&
        (pipeline_state->active_shaders & (VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT |
                                           VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT | VK_SHADER_STAGE_GEOMETRY_BIT))) {
        skip |= LogError(vuid.mesh_shader_stages_06480, cb_state.GetObjectList(VK_PIPELINE_BIND_POINT_GRAPHICS), loc,
                         "The bound graphics pipeline must not have been created with "
                         "VK_SHADER_STAGE_VERTEX_BIT, VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT, "
                         "VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT or VK_SHADER_STAGE_GEOMETRY_BIT. Active shader stages on the "
                         "bound pipeline are %s.",
                         string_VkShaderStageFlags(pipeline_state->active_shaders).c_str());
    }
    return skip;
}

bool CoreChecks::PreCallValidateCmdDraw(VkCommandBuffer commandBuffer, uint32_t vertexCount, uint32_t instanceCount,
                                        uint32_t firstVertex, uint32_t firstInstance, const ErrorObject &error_obj) const {
    bool skip = false;
    const auto &cb_state = *GetRead<CMD_BUFFER_STATE>(commandBuffer);
    skip |= ValidateCmd(cb_state, error_obj.location);
    if (skip) return skip;  // basic validation failed, might have null pointers

    skip |= ValidateCmdDrawInstance(cb_state, instanceCount, firstInstance, error_obj.location);
    skip |= ValidateActionState(cb_state, VK_PIPELINE_BIND_POINT_GRAPHICS, error_obj.location);
    skip |= ValidateVTGShaderStages(cb_state, error_obj.location);
    return skip;
}

bool CoreChecks::PreCallValidateCmdDrawMultiEXT(VkCommandBuffer commandBuffer, uint32_t drawCount,
                                                const VkMultiDrawInfoEXT *pVertexInfo, uint32_t instanceCount,
                                                uint32_t firstInstance, uint32_t stride, const ErrorObject &error_obj) const {
    bool skip = false;
    const auto &cb_state = *GetRead<CMD_BUFFER_STATE>(commandBuffer);
    skip |= ValidateCmd(cb_state, error_obj.location);
    if (skip) return skip;  // basic validation failed, might have null pointers

    if (!enabled_features.multiDraw) {
        skip |= LogError("VUID-vkCmdDrawMultiEXT-None-04933", cb_state.GetObjectList(VK_PIPELINE_BIND_POINT_GRAPHICS),
                         error_obj.location, "The multiDraw feature was not enabled.");
    }
    if (drawCount > phys_dev_ext_props.multi_draw_props.maxMultiDrawCount) {
        skip |=
            LogError("VUID-vkCmdDrawMultiEXT-drawCount-04934", cb_state.GetObjectList(VK_PIPELINE_BIND_POINT_GRAPHICS),
                     error_obj.location.dot(Field::drawCount), "(%" PRIu32 ") must be less than maxMultiDrawCount (%" PRIu32 ").",
                     drawCount, phys_dev_ext_props.multi_draw_props.maxMultiDrawCount);
    }
    if (stride & 3) {
        skip |= LogError("VUID-vkCmdDrawMultiEXT-stride-04936", cb_state.GetObjectList(VK_PIPELINE_BIND_POINT_GRAPHICS),
                         error_obj.location.dot(Field::stride), "(%" PRIu32 ") is not a multiple of 4.", stride);
    }
    if (drawCount != 0 && !pVertexInfo) {
        skip |= LogError("VUID-vkCmdDrawMultiEXT-drawCount-04935", cb_state.GetObjectList(VK_PIPELINE_BIND_POINT_GRAPHICS),
                         error_obj.location.dot(Field::drawCount), "is %" PRIu32 " but pVertexInfo is NULL.", drawCount);
    }

    skip |= ValidateCmdDrawInstance(cb_state, instanceCount, firstInstance, error_obj.location);
    skip |= ValidateActionState(cb_state, VK_PIPELINE_BIND_POINT_GRAPHICS, error_obj.location);
    skip |= ValidateVTGShaderStages(cb_state, error_obj.location);
    return skip;
}

bool CoreChecks::ValidateCmdDrawIndexedBufferSize(const CMD_BUFFER_STATE &cb_state, uint32_t indexCount, uint32_t firstIndex,
                                                  const Location &loc, const char *first_index_vuid) const {
    bool skip = false;
    if (!enabled_features.robustBufferAccess2 && cb_state.index_buffer_binding.bound()) {
        const auto &index_buffer_binding = cb_state.index_buffer_binding;
        const uint32_t index_size = GetIndexAlignment(index_buffer_binding.index_type);
        // This doesn't exactly match the pseudocode of the VUID, but the binding size is the *bound* size, such that the offset
        // has already been accounted for (subtracted from the buffer size), and is consistent with the use of
        // BufferBinding::size for vertex buffer bindings (which record the *bound* size, not the size of the bound buffer)
        VkDeviceSize end_offset = static_cast<VkDeviceSize>(index_size * (firstIndex + indexCount));
        if (end_offset > index_buffer_binding.size) {
            LogObjectList objlist = cb_state.GetObjectList(VK_PIPELINE_BIND_POINT_GRAPHICS);
            objlist.add(index_buffer_binding.buffer_state->buffer());
            skip |= LogError(first_index_vuid, objlist, loc,
                             "index size (%" PRIu32 ") * (firstIndex (%" PRIu32 ") + indexCount (%" PRIu32
                             ")) "
                             "+ binding offset (%" PRIuLEAST64 ") = an ending offset of %" PRIuLEAST64
                             " bytes, which is greater than the index buffer size (%" PRIuLEAST64 ").",
                             index_size, firstIndex, indexCount, index_buffer_binding.offset,
                             end_offset + index_buffer_binding.offset, index_buffer_binding.size + index_buffer_binding.offset);
        }
    }
    return skip;
}

bool CoreChecks::PreCallValidateCmdDrawIndexed(VkCommandBuffer commandBuffer, uint32_t indexCount, uint32_t instanceCount,
                                               uint32_t firstIndex, int32_t vertexOffset, uint32_t firstInstance,
                                               const ErrorObject &error_obj) const {
    bool skip = false;
    const auto &cb_state = *GetRead<CMD_BUFFER_STATE>(commandBuffer);
    skip |= ValidateCmd(cb_state, error_obj.location);
    if (skip) return skip;  // basic validation failed, might have null pointers

    skip |= ValidateCmdDrawInstance(cb_state, instanceCount, firstInstance, error_obj.location);
    skip |= ValidateGraphicsIndexedCmd(cb_state, error_obj.location);
    skip |= ValidateActionState(cb_state, VK_PIPELINE_BIND_POINT_GRAPHICS, error_obj.location);
    skip |= ValidateCmdDrawIndexedBufferSize(cb_state, indexCount, firstIndex, error_obj.location,
                                             "VUID-vkCmdDrawIndexed-robustBufferAccess2-07825");
    skip |= ValidateVTGShaderStages(cb_state, error_obj.location);
    return skip;
}

bool CoreChecks::PreCallValidateCmdDrawMultiIndexedEXT(VkCommandBuffer commandBuffer, uint32_t drawCount,
                                                       const VkMultiDrawIndexedInfoEXT *pIndexInfo, uint32_t instanceCount,
                                                       uint32_t firstInstance, uint32_t stride, const int32_t *pVertexOffset,
                                                       const ErrorObject &error_obj) const {
    bool skip = false;
    const auto &cb_state = *GetRead<CMD_BUFFER_STATE>(commandBuffer);
    skip |= ValidateCmd(cb_state, error_obj.location);
    if (skip) return skip;  // basic validation failed, might have null pointers

    if (!enabled_features.multiDraw) {
        skip |= LogError("VUID-vkCmdDrawMultiIndexedEXT-None-04937", cb_state.GetObjectList(VK_PIPELINE_BIND_POINT_GRAPHICS),
                         error_obj.location, "multiDraw feature was not enabled.");
    }
    if (drawCount > phys_dev_ext_props.multi_draw_props.maxMultiDrawCount) {
        skip |= LogError("VUID-vkCmdDrawMultiIndexedEXT-drawCount-04939", cb_state.GetObjectList(VK_PIPELINE_BIND_POINT_GRAPHICS),
                         error_obj.location.dot(Field::drawCount),
                         "(%" PRIu32 ") must be less than VkPhysicalDeviceMultiDrawPropertiesEXT::maxMultiDrawCount (%" PRIu32 ").",
                         drawCount, phys_dev_ext_props.multi_draw_props.maxMultiDrawCount);
    }

    skip |= ValidateCmdDrawInstance(cb_state, instanceCount, firstInstance, error_obj.location);
    skip |= ValidateGraphicsIndexedCmd(cb_state, error_obj.location);
    skip |= ValidateActionState(cb_state, VK_PIPELINE_BIND_POINT_GRAPHICS, error_obj.location);
    skip |= ValidateVTGShaderStages(cb_state, error_obj.location);

    // only index into pIndexInfo if we know parameters are sane
    if (drawCount != 0 && !pIndexInfo) {
        skip |= LogError("VUID-vkCmdDrawMultiIndexedEXT-drawCount-04940", cb_state.GetObjectList(VK_PIPELINE_BIND_POINT_GRAPHICS),
                         error_obj.location.dot(Field::drawCount), "is %" PRIu32 " but pIndexInfo is NULL.", drawCount);
    } else if (stride & 3) {
        skip |= LogError("VUID-vkCmdDrawMultiIndexedEXT-stride-04941", cb_state.GetObjectList(VK_PIPELINE_BIND_POINT_GRAPHICS),
                         error_obj.location.dot(Field::stride), "(%" PRIu32 ") is not a multiple of 4.", stride);
    } else {
        const auto info_bytes = reinterpret_cast<const char *>(pIndexInfo);
        for (uint32_t i = 0; i < drawCount; i++) {
            const auto info_ptr = reinterpret_cast<const VkMultiDrawIndexedInfoEXT *>(info_bytes + i * stride);
            skip |= ValidateCmdDrawIndexedBufferSize(cb_state, info_ptr->indexCount, info_ptr->firstIndex,
                                                     error_obj.location.dot(Field::pIndexInfo, i),
                                                     "VUID-vkCmdDrawMultiIndexedEXT-robustBufferAccess2-07825");
        }
    }
    return skip;
}

bool CoreChecks::PreCallValidateCmdDrawIndirect(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                uint32_t drawCount, uint32_t stride, const ErrorObject &error_obj) const {
    bool skip = false;
    const auto &cb_state = *GetRead<CMD_BUFFER_STATE>(commandBuffer);
    skip |= ValidateCmd(cb_state, error_obj.location);
    if (skip) return skip;  // basic validation failed, might have null pointers

    skip = ValidateActionState(cb_state, VK_PIPELINE_BIND_POINT_GRAPHICS, error_obj.location);
    auto buffer_state = Get<BUFFER_STATE>(buffer);
    skip |= ValidateIndirectCmd(cb_state, *buffer_state, error_obj.location);
    skip |= ValidateVTGShaderStages(cb_state, error_obj.location);

    if (!enabled_features.multiDrawIndirect && ((drawCount > 1))) {
        skip |= LogError("VUID-vkCmdDrawIndirect-drawCount-02718", cb_state.GetObjectList(VK_PIPELINE_BIND_POINT_GRAPHICS),
                         error_obj.location.dot(Field::drawCount),
                         "(%" PRIu32 ") must be 0 or 1 if multiDrawIndirect feature is not enabled.", drawCount);
    }
    if (drawCount > phys_dev_props.limits.maxDrawIndirectCount) {
        skip |= LogError("VUID-vkCmdDrawIndirect-drawCount-02719", cb_state.GetObjectList(VK_PIPELINE_BIND_POINT_GRAPHICS),
                         error_obj.location.dot(Field::drawCount),
                         "(%" PRIu32 ") is not less than or equal to the maximum allowed (%" PRIu32 ").", drawCount,
                         phys_dev_props.limits.maxDrawIndirectCount);
    }
    if (offset & 3) {
        skip |= LogError("VUID-vkCmdDrawIndirect-offset-02710", cb_state.GetObjectList(VK_PIPELINE_BIND_POINT_GRAPHICS),
                         error_obj.location.dot(Field::offset), "(%" PRIu64 ") must be a multiple of 4.", offset);
    }
    if (drawCount > 1) {
        skip |= ValidateCmdDrawStrideWithStruct(cb_state, "VUID-vkCmdDrawIndirect-drawCount-00476", stride,
                                                Struct::VkDrawIndirectCommand, sizeof(VkDrawIndirectCommand), error_obj.location);
        skip |= ValidateCmdDrawStrideWithBuffer(cb_state, "VUID-vkCmdDrawIndirect-drawCount-00488", stride,
                                                Struct::VkDrawIndirectCommand, sizeof(VkDrawIndirectCommand), drawCount, offset,
                                                buffer_state.get(), error_obj.location);
    } else if ((drawCount == 1) && (offset + sizeof(VkDrawIndirectCommand)) > buffer_state->createInfo.size) {
        LogObjectList objlist = cb_state.GetObjectList(VK_PIPELINE_BIND_POINT_GRAPHICS);
        objlist.add(buffer);
        skip |= LogError("VUID-vkCmdDrawIndirect-drawCount-00487", objlist, error_obj.location.dot(Field::drawCount),
                         "is 1 and (offset + sizeof(VkDrawIndirectCommand)) (%" PRIu64
                         ") is not less than "
                         "or equal to the size of buffer (%" PRIu64 ").",
                         (offset + sizeof(VkDrawIndirectCommand)), buffer_state->createInfo.size);
    }
    // TODO: If the drawIndirectFirstInstance feature is not enabled, all the firstInstance members of the
    // VkDrawIndirectCommand structures accessed by this command must be 0, which will require access to the contents of 'buffer'.
    return skip;
}

bool CoreChecks::PreCallValidateCmdDrawIndexedIndirect(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                       uint32_t drawCount, uint32_t stride, const ErrorObject &error_obj) const {
    bool skip = false;
    const auto &cb_state = *GetRead<CMD_BUFFER_STATE>(commandBuffer);
    skip |= ValidateCmd(cb_state, error_obj.location);
    if (skip) return skip;  // basic validation failed, might have null pointers

    skip |= ValidateGraphicsIndexedCmd(cb_state, error_obj.location);
    skip |= ValidateActionState(cb_state, VK_PIPELINE_BIND_POINT_GRAPHICS, error_obj.location);
    auto buffer_state = Get<BUFFER_STATE>(buffer);
    skip |= ValidateIndirectCmd(cb_state, *buffer_state, error_obj.location);
    skip |= ValidateVTGShaderStages(cb_state, error_obj.location);

    if (!enabled_features.multiDrawIndirect && ((drawCount > 1))) {
        skip |= LogError("VUID-vkCmdDrawIndexedIndirect-drawCount-02718", cb_state.GetObjectList(VK_PIPELINE_BIND_POINT_GRAPHICS),
                         error_obj.location.dot(Field::drawCount),
                         "(%" PRIu32 ") must be 0 or 1 if multiDrawIndirect feature is not enabled.", drawCount);
    }
    if (drawCount > phys_dev_props.limits.maxDrawIndirectCount) {
        skip |= LogError("VUID-vkCmdDrawIndexedIndirect-drawCount-02719", cb_state.GetObjectList(VK_PIPELINE_BIND_POINT_GRAPHICS),
                         error_obj.location.dot(Field::drawCount),
                         "(%" PRIu32 ") is not less than or equal to the maximum allowed (%" PRIu32 ").", drawCount,
                         phys_dev_props.limits.maxDrawIndirectCount);
    }

    if (drawCount > 1) {
        skip |= ValidateCmdDrawStrideWithStruct(cb_state, "VUID-vkCmdDrawIndexedIndirect-drawCount-00528", stride,
                                                Struct::VkDrawIndexedIndirectCommand, sizeof(VkDrawIndexedIndirectCommand),
                                                error_obj.location);
        skip |= ValidateCmdDrawStrideWithBuffer(cb_state, "VUID-vkCmdDrawIndexedIndirect-drawCount-00540", stride,
                                                Struct::VkDrawIndexedIndirectCommand, sizeof(VkDrawIndexedIndirectCommand),
                                                drawCount, offset, buffer_state.get(), error_obj.location);
    } else if (offset & 3) {
        skip |= LogError("VUID-vkCmdDrawIndexedIndirect-offset-02710", cb_state.GetObjectList(VK_PIPELINE_BIND_POINT_GRAPHICS),
                         error_obj.location.dot(Field::offset), "(%" PRIu64 ") must be a multiple of 4.", offset);
    } else if ((drawCount == 1) && (offset + sizeof(VkDrawIndexedIndirectCommand)) > buffer_state->createInfo.size) {
        LogObjectList objlist = cb_state.GetObjectList(VK_PIPELINE_BIND_POINT_GRAPHICS);
        objlist.add(buffer);
        skip |= LogError("VUID-vkCmdDrawIndexedIndirect-drawCount-00539", objlist, error_obj.location.dot(Field::drawCount),
                         "is 1 and (offset + sizeof(VkDrawIndexedIndirectCommand)) (%" PRIu64
                         ") is not less than "
                         "or equal to the size of buffer (%" PRIu64 ").",
                         (offset + sizeof(VkDrawIndexedIndirectCommand)), buffer_state->createInfo.size);
    }
    // TODO: If the drawIndirectFirstInstance feature is not enabled, all the firstInstance members of the
    // VkDrawIndexedIndirectCommand structures accessed by this command must be 0, which will require access to the contents of
    // 'buffer'.
    return skip;
}

bool CoreChecks::PreCallValidateCmdDispatch(VkCommandBuffer commandBuffer, uint32_t groupCountX, uint32_t groupCountY,
                                            uint32_t groupCountZ, const ErrorObject &error_obj) const {
    bool skip = false;
    const auto &cb_state = *GetRead<CMD_BUFFER_STATE>(commandBuffer);
    skip |= ValidateCmd(cb_state, error_obj.location);
    if (skip) return skip;  // basic validation failed, might have null pointers

    skip |= ValidateActionState(cb_state, VK_PIPELINE_BIND_POINT_COMPUTE, error_obj.location);

    if (groupCountX > phys_dev_props.limits.maxComputeWorkGroupCount[0]) {
        skip |= LogError("VUID-vkCmdDispatch-groupCountX-00386", cb_state.GetObjectList(VK_SHADER_STAGE_COMPUTE_BIT),
                         error_obj.location.dot(Field::groupCountX),
                         "(%" PRIu32 ") exceeds device limit maxComputeWorkGroupCount[0] (%" PRIu32 ").", groupCountX,
                         phys_dev_props.limits.maxComputeWorkGroupCount[0]);
    }

    if (groupCountY > phys_dev_props.limits.maxComputeWorkGroupCount[1]) {
        skip |= LogError("VUID-vkCmdDispatch-groupCountY-00387", cb_state.GetObjectList(VK_SHADER_STAGE_COMPUTE_BIT),
                         error_obj.location.dot(Field::groupCountY),
                         "(%" PRIu32 ") exceeds device limit maxComputeWorkGroupCount[1] (%" PRIu32 ").", groupCountY,
                         phys_dev_props.limits.maxComputeWorkGroupCount[1]);
    }

    if (groupCountZ > phys_dev_props.limits.maxComputeWorkGroupCount[2]) {
        skip |= LogError("VUID-vkCmdDispatch-groupCountZ-00388", cb_state.GetObjectList(VK_SHADER_STAGE_COMPUTE_BIT),
                         error_obj.location.dot(Field::groupCountZ),
                         "(%" PRIu32 ") exceeds device limit maxComputeWorkGroupCount[2] (%" PRIu32 ").", groupCountZ,
                         phys_dev_props.limits.maxComputeWorkGroupCount[2]);
    }

    return skip;
}

bool CoreChecks::PreCallValidateCmdDispatchBase(VkCommandBuffer commandBuffer, uint32_t baseGroupX, uint32_t baseGroupY,
                                                uint32_t baseGroupZ, uint32_t groupCountX, uint32_t groupCountY,
                                                uint32_t groupCountZ, const ErrorObject &error_obj) const {
    bool skip = false;
    const auto &cb_state = *GetRead<CMD_BUFFER_STATE>(commandBuffer);
    skip |= ValidateCmd(cb_state, error_obj.location);
    if (skip) return skip;  // basic validation failed, might have null pointers

    skip |= ValidateActionState(cb_state, VK_PIPELINE_BIND_POINT_COMPUTE, error_obj.location);

    // Paired if {} else if {} tests used to avoid any possible uint underflow
    uint32_t limit = phys_dev_props.limits.maxComputeWorkGroupCount[0];
    if (baseGroupX >= limit) {
        skip |=
            LogError("VUID-vkCmdDispatchBase-baseGroupX-00421", cb_state.GetObjectList(VK_SHADER_STAGE_COMPUTE_BIT),
                     error_obj.location.dot(Field::baseGroupX),
                     "(%" PRIu32 ") equals or exceeds device limit maxComputeWorkGroupCount[0] (%" PRIu32 ").", baseGroupX, limit);
    } else if (groupCountX > (limit - baseGroupX)) {
        skip |=
            LogError("VUID-vkCmdDispatchBase-groupCountX-00424", cb_state.GetObjectList(VK_SHADER_STAGE_COMPUTE_BIT),
                     error_obj.location.dot(Field::baseGroupX),
                     "(%" PRIu32 ") + groupCountX (%" PRIu32 ") exceeds device limit maxComputeWorkGroupCount[0] (%" PRIu32 ").",
                     baseGroupX, groupCountX, limit);
    }

    limit = phys_dev_props.limits.maxComputeWorkGroupCount[1];
    if (baseGroupY >= limit) {
        skip |=
            LogError("VUID-vkCmdDispatchBase-baseGroupX-00422", cb_state.GetObjectList(VK_SHADER_STAGE_COMPUTE_BIT),
                     error_obj.location.dot(Field::baseGroupY),
                     "(%" PRIu32 ") equals or exceeds device limit maxComputeWorkGroupCount[1] (%" PRIu32 ").", baseGroupY, limit);
    } else if (groupCountY > (limit - baseGroupY)) {
        skip |=
            LogError("VUID-vkCmdDispatchBase-groupCountY-00425", cb_state.GetObjectList(VK_SHADER_STAGE_COMPUTE_BIT),
                     error_obj.location.dot(Field::baseGroupY),
                     "(%" PRIu32 ") + groupCountY (%" PRIu32 ") exceeds device limit maxComputeWorkGroupCount[1] (%" PRIu32 ").",
                     baseGroupY, groupCountY, limit);
    }

    limit = phys_dev_props.limits.maxComputeWorkGroupCount[2];
    if (baseGroupZ >= limit) {
        skip |=
            LogError("VUID-vkCmdDispatchBase-baseGroupZ-00423", cb_state.GetObjectList(VK_SHADER_STAGE_COMPUTE_BIT),
                     error_obj.location.dot(Field::baseGroupZ),
                     "(%" PRIu32 ") equals or exceeds device limit maxComputeWorkGroupCount[2] (%" PRIu32 ").", baseGroupZ, limit);
    } else if (groupCountZ > (limit - baseGroupZ)) {
        skip |=
            LogError("VUID-vkCmdDispatchBase-groupCountZ-00426", cb_state.GetObjectList(VK_SHADER_STAGE_COMPUTE_BIT),
                     error_obj.location.dot(Field::baseGroupZ),
                     "(%" PRIu32 ") + groupCountZ (%" PRIu32 ") exceeds device limit maxComputeWorkGroupCount[2] (%" PRIu32 ").",
                     baseGroupZ, groupCountZ, limit);
    }

    if (baseGroupX || baseGroupY || baseGroupZ) {
        const auto lv_bind_point = ConvertToLvlBindPoint(VK_PIPELINE_BIND_POINT_COMPUTE);
        const auto *pipeline_state = cb_state.lastBound[lv_bind_point].pipeline_state;
        if (pipeline_state && !(pipeline_state->create_flags & VK_PIPELINE_CREATE_DISPATCH_BASE)) {
            skip |= LogError("VUID-vkCmdDispatchBase-baseGroupX-00427", cb_state.GetObjectList(VK_SHADER_STAGE_COMPUTE_BIT),
                             error_obj.location,
                             "If any of baseGroupX (%" PRIu32 "), baseGroupY (%" PRIu32 "), or baseGroupZ (%" PRIu32
                             ") are not zero, then the bound compute pipeline "
                             "must have been created with the VK_PIPELINE_CREATE_DISPATCH_BASE flag",
                             baseGroupX, baseGroupY, baseGroupZ);
        }
    }

    return skip;
}

bool CoreChecks::PreCallValidateCmdDispatchBaseKHR(VkCommandBuffer commandBuffer, uint32_t baseGroupX, uint32_t baseGroupY,
                                                   uint32_t baseGroupZ, uint32_t groupCountX, uint32_t groupCountY,
                                                   uint32_t groupCountZ, const ErrorObject &error_obj) const {
    return PreCallValidateCmdDispatchBase(commandBuffer, baseGroupX, baseGroupY, baseGroupZ, groupCountX, groupCountY, groupCountZ,
                                          error_obj);
}

bool CoreChecks::PreCallValidateCmdDispatchIndirect(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                    const ErrorObject &error_obj) const {
    bool skip = false;
    const auto &cb_state = *GetRead<CMD_BUFFER_STATE>(commandBuffer);
    skip |= ValidateCmd(cb_state, error_obj.location);
    if (skip) return skip;  // basic validation failed, might have null pointers

    skip = ValidateActionState(cb_state, VK_PIPELINE_BIND_POINT_COMPUTE, error_obj.location);
    auto buffer_state = Get<BUFFER_STATE>(buffer);
    skip |= ValidateIndirectCmd(cb_state, *buffer_state, error_obj.location);
    if (offset & 3) {
        skip |= LogError("VUID-vkCmdDispatchIndirect-offset-02710", cb_state.GetObjectList(VK_SHADER_STAGE_COMPUTE_BIT),
                         error_obj.location.dot(Field::offset), "(%" PRIu64 ") must be a multiple of 4.", offset);
    }
    if ((offset + sizeof(VkDispatchIndirectCommand)) > buffer_state->createInfo.size) {
        skip |= LogError("VUID-vkCmdDispatchIndirect-offset-00407", cb_state.GetObjectList(VK_SHADER_STAGE_COMPUTE_BIT),
                         error_obj.location,
                         "The (offset + sizeof(VkDrawIndexedIndirectCommand)) (%" PRIu64
                         ")  is greater than the "
                         "size of the buffer (%" PRIu64 ").",
                         offset + sizeof(VkDispatchIndirectCommand), buffer_state->createInfo.size);
    }
    return skip;
}
bool CoreChecks::PreCallValidateCmdDrawIndirectCount(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                     VkBuffer countBuffer, VkDeviceSize countBufferOffset, uint32_t maxDrawCount,
                                                     uint32_t stride, const ErrorObject &error_obj) const {
    bool skip = false;
    const auto &cb_state = *GetRead<CMD_BUFFER_STATE>(commandBuffer);
    skip |= ValidateCmd(cb_state, error_obj.location);
    if (skip) return skip;  // basic validation failed, might have null pointers

    if (offset & 3) {
        skip |= LogError("VUID-vkCmdDrawIndirectCount-offset-02710", cb_state.GetObjectList(VK_PIPELINE_BIND_POINT_GRAPHICS),
                         error_obj.location.dot(Field::offset), "(%" PRIu64 "), is not a multiple of 4.", offset);
    }

    if (countBufferOffset & 3) {
        skip |=
            LogError("VUID-vkCmdDrawIndirectCount-countBufferOffset-02716", cb_state.GetObjectList(VK_PIPELINE_BIND_POINT_GRAPHICS),
                     error_obj.location.dot(Field::countBufferOffset), "(%" PRIu64 "), is not a multiple of 4.", countBufferOffset);
    }

    if ((device_extensions.vk_khr_draw_indirect_count != kEnabledByCreateinfo) &&
        ((api_version >= VK_API_VERSION_1_2) && (enabled_features.drawIndirectCount == VK_FALSE))) {
        skip |= LogError("VUID-vkCmdDrawIndirectCount-None-04445", cb_state.GetObjectList(VK_PIPELINE_BIND_POINT_GRAPHICS),
                         error_obj.location,
                         "Starting in Vulkan 1.2 the VkPhysicalDeviceVulkan12Features::drawIndirectCount must be enabled to "
                         "call this command.");
    }
    auto buffer_state = Get<BUFFER_STATE>(buffer);
    skip |= ValidateCmdDrawStrideWithStruct(cb_state, "VUID-vkCmdDrawIndirectCount-stride-03110", stride,
                                            Struct::VkDrawIndirectCommand, sizeof(VkDrawIndirectCommand), error_obj.location);
    if (maxDrawCount > 1) {
        skip |= ValidateCmdDrawStrideWithBuffer(cb_state, "VUID-vkCmdDrawIndirectCount-maxDrawCount-03111", stride,
                                                Struct::VkDrawIndirectCommand, sizeof(VkDrawIndirectCommand), maxDrawCount, offset,
                                                buffer_state.get(), error_obj.location);
    }

    skip |= ValidateActionState(cb_state, VK_PIPELINE_BIND_POINT_GRAPHICS, error_obj.location);
    skip |= ValidateIndirectCmd(cb_state, *buffer_state, error_obj.location);
    auto count_buffer_state = Get<BUFFER_STATE>(countBuffer);
    skip |= ValidateIndirectCountCmd(cb_state, *count_buffer_state, countBufferOffset, error_obj.location);
    skip |= ValidateVTGShaderStages(cb_state, error_obj.location);
    return skip;
}

bool CoreChecks::PreCallValidateCmdDrawIndirectCountKHR(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                        VkBuffer countBuffer, VkDeviceSize countBufferOffset, uint32_t maxDrawCount,
                                                        uint32_t stride, const ErrorObject &error_obj) const {
    return PreCallValidateCmdDrawIndirectCount(commandBuffer, buffer, offset, countBuffer, countBufferOffset, maxDrawCount, stride,
                                               error_obj);
}

bool CoreChecks::PreCallValidateCmdDrawIndexedIndirectCount(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                            VkBuffer countBuffer, VkDeviceSize countBufferOffset,
                                                            uint32_t maxDrawCount, uint32_t stride,
                                                            const ErrorObject &error_obj) const {
    bool skip = false;
    const auto &cb_state = *GetRead<CMD_BUFFER_STATE>(commandBuffer);
    skip |= ValidateCmd(cb_state, error_obj.location);
    if (skip) return skip;  // basic validation failed, might have null pointers

    if (offset & 3) {
        skip |= LogError("VUID-vkCmdDrawIndexedIndirectCount-offset-02710", cb_state.GetObjectList(VK_PIPELINE_BIND_POINT_GRAPHICS),
                         error_obj.location.dot(Field::offset), "(%" PRIu64 "), is not a multiple of 4.", offset);
    }
    if (countBufferOffset & 3) {
        skip |= LogError("VUID-vkCmdDrawIndexedIndirectCount-countBufferOffset-02716",
                         cb_state.GetObjectList(VK_PIPELINE_BIND_POINT_GRAPHICS), error_obj.location.dot(Field::countBufferOffset),
                         "(%" PRIu64 "), is not a multiple of 4.", countBufferOffset);
    }
    if ((device_extensions.vk_khr_draw_indirect_count != kEnabledByCreateinfo) &&
        ((api_version >= VK_API_VERSION_1_2) && (enabled_features.drawIndirectCount == VK_FALSE))) {
        skip |= LogError("VUID-vkCmdDrawIndexedIndirectCount-None-04445", cb_state.GetObjectList(VK_PIPELINE_BIND_POINT_GRAPHICS),
                         error_obj.location,
                         "Starting in Vulkan 1.2 the VkPhysicalDeviceVulkan12Features::drawIndirectCount must be enabled to "
                         "call this command.");
    }
    skip |= ValidateCmdDrawStrideWithStruct(cb_state, "VUID-vkCmdDrawIndexedIndirectCount-stride-03142", stride,
                                            Struct::VkDrawIndexedIndirectCommand, sizeof(VkDrawIndexedIndirectCommand),
                                            error_obj.location);
    auto buffer_state = Get<BUFFER_STATE>(buffer);
    if (maxDrawCount > 1) {
        skip |= ValidateCmdDrawStrideWithBuffer(cb_state, "VUID-vkCmdDrawIndexedIndirectCount-maxDrawCount-03143", stride,
                                                Struct::VkDrawIndexedIndirectCommand, sizeof(VkDrawIndexedIndirectCommand),
                                                maxDrawCount, offset, buffer_state.get(), error_obj.location);
    }

    skip |= ValidateGraphicsIndexedCmd(cb_state, error_obj.location);
    skip |= ValidateActionState(cb_state, VK_PIPELINE_BIND_POINT_GRAPHICS, error_obj.location);
    skip |= ValidateIndirectCmd(cb_state, *buffer_state, error_obj.location);
    auto count_buffer_state = Get<BUFFER_STATE>(countBuffer);
    skip |= ValidateIndirectCountCmd(cb_state, *count_buffer_state, countBufferOffset, error_obj.location);
    skip |= ValidateVTGShaderStages(cb_state, error_obj.location);
    return skip;
}

bool CoreChecks::PreCallValidateCmdDrawIndexedIndirectCountKHR(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                               VkBuffer countBuffer, VkDeviceSize countBufferOffset,
                                                               uint32_t maxDrawCount, uint32_t stride,
                                                               const ErrorObject &error_obj) const {
    return PreCallValidateCmdDrawIndexedIndirectCount(commandBuffer, buffer, offset, countBuffer, countBufferOffset, maxDrawCount,
                                                      stride, error_obj);
}

bool CoreChecks::PreCallValidateCmdDrawIndirectByteCountEXT(VkCommandBuffer commandBuffer, uint32_t instanceCount,
                                                            uint32_t firstInstance, VkBuffer counterBuffer,
                                                            VkDeviceSize counterBufferOffset, uint32_t counterOffset,
                                                            uint32_t vertexStride, const ErrorObject &error_obj) const {
    bool skip = false;
    const auto &cb_state = *GetRead<CMD_BUFFER_STATE>(commandBuffer);
    skip |= ValidateCmd(cb_state, error_obj.location);
    if (skip) return skip;  // basic validation failed, might have null pointers

    if (!enabled_features.transformFeedback) {
        skip |= LogError("VUID-vkCmdDrawIndirectByteCountEXT-transformFeedback-02287",
                         cb_state.GetObjectList(VK_PIPELINE_BIND_POINT_GRAPHICS), error_obj.location,
                         "transformFeedback feature is not enabled.");
    }
    if (IsExtEnabled(device_extensions.vk_ext_transform_feedback) &&
        !phys_dev_ext_props.transform_feedback_props.transformFeedbackDraw) {
        skip |= LogError("VUID-vkCmdDrawIndirectByteCountEXT-transformFeedbackDraw-02288",
                         cb_state.GetObjectList(VK_PIPELINE_BIND_POINT_GRAPHICS), error_obj.location,
                         "VkPhysicalDeviceTransformFeedbackPropertiesEXT::transformFeedbackDraw is not supported");
    }
    if ((vertexStride <= 0) || (vertexStride > phys_dev_ext_props.transform_feedback_props.maxTransformFeedbackBufferDataStride)) {
        skip |= LogError("VUID-vkCmdDrawIndirectByteCountEXT-vertexStride-02289",
                         cb_state.GetObjectList(VK_PIPELINE_BIND_POINT_GRAPHICS), error_obj.location.dot(Field::vertexStride),
                         "(%" PRIu32 ") must be between 0 and maxTransformFeedbackBufferDataStride (%" PRIu32 ").", vertexStride,
                         phys_dev_ext_props.transform_feedback_props.maxTransformFeedbackBufferDataStride);
    }

    if ((counterOffset % 4) != 0) {
        skip |= LogError("VUID-vkCmdDrawIndirectByteCountEXT-counterBufferOffset-04568",
                         cb_state.GetObjectList(VK_PIPELINE_BIND_POINT_GRAPHICS), error_obj.location.dot(Field::offset),
                         "(%" PRIu32 ") must be a multiple of 4.", counterOffset);
    }

    skip |= ValidateCmdDrawInstance(cb_state, instanceCount, firstInstance, error_obj.location);
    skip |= ValidateActionState(cb_state, VK_PIPELINE_BIND_POINT_GRAPHICS, error_obj.location);
    auto counter_buffer_state = Get<BUFFER_STATE>(counterBuffer);
    skip |= ValidateIndirectCmd(cb_state, *counter_buffer_state, error_obj.location);
    skip |= ValidateVTGShaderStages(cb_state, error_obj.location);
    return skip;
}

bool CoreChecks::PreCallValidateCmdTraceRaysNV(VkCommandBuffer commandBuffer, VkBuffer raygenShaderBindingTableBuffer,
                                               VkDeviceSize raygenShaderBindingOffset, VkBuffer missShaderBindingTableBuffer,
                                               VkDeviceSize missShaderBindingOffset, VkDeviceSize missShaderBindingStride,
                                               VkBuffer hitShaderBindingTableBuffer, VkDeviceSize hitShaderBindingOffset,
                                               VkDeviceSize hitShaderBindingStride, VkBuffer callableShaderBindingTableBuffer,
                                               VkDeviceSize callableShaderBindingOffset, VkDeviceSize callableShaderBindingStride,
                                               uint32_t width, uint32_t height, uint32_t depth,
                                               const ErrorObject &error_obj) const {
    bool skip = false;
    const auto &cb_state = *GetRead<CMD_BUFFER_STATE>(commandBuffer);
    skip |= ValidateCmd(cb_state, error_obj.location);
    if (skip) return skip;  // basic validation failed, might have null pointers

    if (SafeModulo(callableShaderBindingOffset, phys_dev_ext_props.ray_tracing_props_nv.shaderGroupBaseAlignment) != 0) {
        skip |= LogError("VUID-vkCmdTraceRaysNV-callableShaderBindingOffset-02462",
                         cb_state.GetObjectList(VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR),
                         error_obj.location.dot(Field::callableShaderBindingOffset),
                         "must be a multiple of "
                         "VkPhysicalDeviceRayTracingPropertiesNV::shaderGroupBaseAlignment.");
    }
    if (SafeModulo(callableShaderBindingStride, phys_dev_ext_props.ray_tracing_props_nv.shaderGroupHandleSize) != 0) {
        skip |= LogError("VUID-vkCmdTraceRaysNV-callableShaderBindingStride-02465",
                         cb_state.GetObjectList(VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR),
                         error_obj.location.dot(Field::callableShaderBindingStride),
                         "must be a multiple of "
                         "VkPhysicalDeviceRayTracingPropertiesNV::shaderGroupHandleSize.");
    }
    if (callableShaderBindingStride > phys_dev_ext_props.ray_tracing_props_nv.maxShaderGroupStride) {
        skip |= LogError("VUID-vkCmdTraceRaysNV-callableShaderBindingStride-02468",
                         cb_state.GetObjectList(VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR),
                         error_obj.location.dot(Field::callableShaderBindingStride),
                         "must be less than or equal to "
                         "VkPhysicalDeviceRayTracingPropertiesNV::maxShaderGroupStride. ");
    }

    // hitShader
    if (SafeModulo(hitShaderBindingOffset, phys_dev_ext_props.ray_tracing_props_nv.shaderGroupBaseAlignment) != 0) {
        skip |= LogError("VUID-vkCmdTraceRaysNV-hitShaderBindingOffset-02460",
                         cb_state.GetObjectList(VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR),
                         error_obj.location.dot(Field::hitShaderBindingOffset),
                         "must be a multiple of "
                         "VkPhysicalDeviceRayTracingPropertiesNV::shaderGroupBaseAlignment.");
    }
    if (SafeModulo(hitShaderBindingStride, phys_dev_ext_props.ray_tracing_props_nv.shaderGroupHandleSize) != 0) {
        skip |= LogError("VUID-vkCmdTraceRaysNV-hitShaderBindingStride-02464",
                         cb_state.GetObjectList(VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR),
                         error_obj.location.dot(Field::hitShaderBindingStride),
                         "must be a multiple of "
                         "VkPhysicalDeviceRayTracingPropertiesNV::shaderGroupHandleSize.");
    }
    if (hitShaderBindingStride > phys_dev_ext_props.ray_tracing_props_nv.maxShaderGroupStride) {
        skip |= LogError("VUID-vkCmdTraceRaysNV-hitShaderBindingStride-02467",
                         cb_state.GetObjectList(VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR),
                         error_obj.location.dot(Field::hitShaderBindingStride),
                         "must be less than or equal to "
                         "VkPhysicalDeviceRayTracingPropertiesNV::maxShaderGroupStride.");
    }

    // missShader
    if (SafeModulo(missShaderBindingOffset, phys_dev_ext_props.ray_tracing_props_nv.shaderGroupBaseAlignment) != 0) {
        skip |= LogError("VUID-vkCmdTraceRaysNV-missShaderBindingOffset-02458",
                         cb_state.GetObjectList(VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR),
                         error_obj.location.dot(Field::missShaderBindingOffset),
                         "must be a multiple of "
                         "VkPhysicalDeviceRayTracingPropertiesNV::shaderGroupBaseAlignment.");
    }
    if (SafeModulo(missShaderBindingStride, phys_dev_ext_props.ray_tracing_props_nv.shaderGroupHandleSize) != 0) {
        skip |= LogError("VUID-vkCmdTraceRaysNV-missShaderBindingStride-02463",
                         cb_state.GetObjectList(VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR),
                         error_obj.location.dot(Field::missShaderBindingStride),
                         "must be a multiple of "
                         "VkPhysicalDeviceRayTracingPropertiesNV::shaderGroupHandleSize.");
    }
    if (missShaderBindingStride > phys_dev_ext_props.ray_tracing_props_nv.maxShaderGroupStride) {
        skip |= LogError("VUID-vkCmdTraceRaysNV-missShaderBindingStride-02466",
                         cb_state.GetObjectList(VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR),
                         error_obj.location.dot(Field::missShaderBindingStride),
                         "must be less than or equal to "
                         "VkPhysicalDeviceRayTracingPropertiesNV::maxShaderGroupStride.");
    }

    // raygenShader
    if (SafeModulo(raygenShaderBindingOffset, phys_dev_ext_props.ray_tracing_props_nv.shaderGroupBaseAlignment) != 0) {
        skip |= LogError("VUID-vkCmdTraceRaysNV-raygenShaderBindingOffset-02456",
                         cb_state.GetObjectList(VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR),
                         error_obj.location.dot(Field::raygenShaderBindingOffset),
                         "must be a multiple of "
                         "VkPhysicalDeviceRayTracingPropertiesNV::shaderGroupBaseAlignment.");
    }
    if (width > phys_dev_props.limits.maxComputeWorkGroupCount[0]) {
        skip |= LogError("VUID-vkCmdTraceRaysNV-width-02469", cb_state.GetObjectList(VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR),
                         error_obj.location.dot(Field::width),
                         "must be less than or equal to VkPhysicalDeviceLimits::maxComputeWorkGroupCount[0].");
    }
    if (height > phys_dev_props.limits.maxComputeWorkGroupCount[1]) {
        skip |= LogError("VUID-vkCmdTraceRaysNV-height-02470", cb_state.GetObjectList(VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR),
                         error_obj.location.dot(Field::height),
                         "must be less than or equal to VkPhysicalDeviceLimits::maxComputeWorkGroupCount[1].");
    }
    if (depth > phys_dev_props.limits.maxComputeWorkGroupCount[2]) {
        skip |= LogError("VUID-vkCmdTraceRaysNV-depth-02471", cb_state.GetObjectList(VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR),
                         error_obj.location.dot(Field::depth),
                         "must be less than or equal to VkPhysicalDeviceLimits::maxComputeWorkGroupCount[2].");
    }

    skip |= ValidateActionState(cb_state, VK_PIPELINE_BIND_POINT_RAY_TRACING_NV, error_obj.location);
    auto callable_shader_buffer_state = Get<BUFFER_STATE>(callableShaderBindingTableBuffer);
    if (callable_shader_buffer_state && callableShaderBindingOffset >= callable_shader_buffer_state->createInfo.size) {
        LogObjectList objlist = cb_state.GetObjectList(VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR);
        objlist.add(callableShaderBindingTableBuffer);
        skip |= LogError("VUID-vkCmdTraceRaysNV-callableShaderBindingOffset-02461", objlist,
                         error_obj.location.dot(Field::callableShaderBindingOffset),
                         "%" PRIu64 " must be less than the size of callableShaderBindingTableBuffer %" PRIu64 " .",
                         callableShaderBindingOffset, callable_shader_buffer_state->createInfo.size);
    }
    auto hit_shader_buffer_state = Get<BUFFER_STATE>(hitShaderBindingTableBuffer);
    if (hit_shader_buffer_state && hitShaderBindingOffset >= hit_shader_buffer_state->createInfo.size) {
        LogObjectList objlist = cb_state.GetObjectList(VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR);
        objlist.add(hitShaderBindingTableBuffer);
        skip |= LogError("VUID-vkCmdTraceRaysNV-hitShaderBindingOffset-02459", objlist,
                         error_obj.location.dot(Field::hitShaderBindingOffset),
                         "%" PRIu64 " must be less than the size of hitShaderBindingTableBuffer %" PRIu64 " .",
                         hitShaderBindingOffset, hit_shader_buffer_state->createInfo.size);
    }
    auto miss_shader_buffer_state = Get<BUFFER_STATE>(missShaderBindingTableBuffer);
    if (miss_shader_buffer_state && missShaderBindingOffset >= miss_shader_buffer_state->createInfo.size) {
        LogObjectList objlist = cb_state.GetObjectList(VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR);
        objlist.add(missShaderBindingTableBuffer);
        skip |= LogError("VUID-vkCmdTraceRaysNV-missShaderBindingOffset-02457", objlist,
                         error_obj.location.dot(Field::missShaderBindingOffset),
                         "%" PRIu64 " must be less than the size of missShaderBindingTableBuffer %" PRIu64 " .",
                         missShaderBindingOffset, miss_shader_buffer_state->createInfo.size);
    }
    auto raygen_shader_buffer_state = Get<BUFFER_STATE>(raygenShaderBindingTableBuffer);
    if (raygenShaderBindingOffset >= raygen_shader_buffer_state->createInfo.size) {
        LogObjectList objlist = cb_state.GetObjectList(VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR);
        objlist.add(raygenShaderBindingTableBuffer);
        skip |= LogError("VUID-vkCmdTraceRaysNV-raygenShaderBindingOffset-02455", objlist,
                         error_obj.location.dot(Field::raygenShaderBindingOffset),
                         "%" PRIu64 " must be less than the size of raygenShaderBindingTableBuffer %" PRIu64 " .",
                         raygenShaderBindingOffset, raygen_shader_buffer_state->createInfo.size);
    }
    return skip;
}

bool CoreChecks::ValidateCmdTraceRaysKHR(const Location &loc, const CMD_BUFFER_STATE &cb_state,
                                         const VkStridedDeviceAddressRegionKHR *pRaygenShaderBindingTable,
                                         const VkStridedDeviceAddressRegionKHR *pMissShaderBindingTable,
                                         const VkStridedDeviceAddressRegionKHR *pHitShaderBindingTable,
                                         const VkStridedDeviceAddressRegionKHR *pCallableShaderBindingTable) const {
    bool skip = false;
    const auto lv_bind_point = ConvertToLvlBindPoint(VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR);
    const PIPELINE_STATE *pipeline_state = cb_state.lastBound[lv_bind_point].pipeline_state;
    const bool is_indirect = loc.function == Func::vkCmdTraceRaysIndirectKHR;

    if (!pipeline_state || (pipeline_state && !pipeline_state->pipeline())) {
        return skip;
    }
    if (pHitShaderBindingTable) {
        const Location table_loc = loc.dot(Field::pHitShaderBindingTable);
        if (pipeline_state->create_flags & VK_PIPELINE_CREATE_RAY_TRACING_NO_NULL_INTERSECTION_SHADERS_BIT_KHR) {
            if (pHitShaderBindingTable->deviceAddress == 0) {
                const char *vuid =
                    is_indirect ? "VUID-vkCmdTraceRaysIndirectKHR-flags-03697" : "VUID-vkCmdTraceRaysKHR-flags-03697";
                skip |= LogError(vuid, cb_state.commandBuffer(), table_loc.dot(Field::deviceAddress), "is zero.");
            }
            if ((pHitShaderBindingTable->size == 0 || pHitShaderBindingTable->stride == 0)) {
                const char *vuid =
                    is_indirect ? "VUID-vkCmdTraceRaysIndirectKHR-flags-03514" : "VUID-vkCmdTraceRaysKHR-flags-03514";
                skip |= LogError(vuid, cb_state.commandBuffer(), table_loc,
                                 "either size (%" PRIu64 ") and stride (%" PRIu64 ") is zero.", pHitShaderBindingTable->size,
                                 pHitShaderBindingTable->stride);
            }
        }
        if (pipeline_state->create_flags & VK_PIPELINE_CREATE_RAY_TRACING_NO_NULL_CLOSEST_HIT_SHADERS_BIT_KHR) {
            if (pHitShaderBindingTable->deviceAddress == 0) {
                const char *vuid =
                    is_indirect ? "VUID-vkCmdTraceRaysIndirectKHR-flags-03696" : "VUID-vkCmdTraceRaysKHR-flags-03696";
                skip |= LogError(vuid, cb_state.commandBuffer(), table_loc.dot(Field::deviceAddress), "is zero.");
            }
            if ((pHitShaderBindingTable->size == 0 || pHitShaderBindingTable->stride == 0)) {
                const char *vuid =
                    is_indirect ? "VUID-vkCmdTraceRaysIndirectKHR-flags-03513" : "VUID-vkCmdTraceRaysKHR-flags-03513";
                skip |= LogError(vuid, cb_state.commandBuffer(), table_loc,
                                 "either size (%" PRIu64 ") and stride (%" PRIu64 ") is zero.", pHitShaderBindingTable->size,
                                 pHitShaderBindingTable->stride);
            }
        }
        if (pipeline_state->create_flags & VK_PIPELINE_CREATE_RAY_TRACING_NO_NULL_ANY_HIT_SHADERS_BIT_KHR) {
            // No vuid to check for pHitShaderBindingTable->deviceAddress == 0 with this flag

            if (pHitShaderBindingTable->size == 0 || pHitShaderBindingTable->stride == 0) {
                const char *vuid =
                    is_indirect ? "VUID-vkCmdTraceRaysIndirectKHR-flags-03512" : "VUID-vkCmdTraceRaysKHR-flags-03512";
                skip |= LogError(vuid, cb_state.commandBuffer(), table_loc,
                                 "either size (%" PRIu64 ") and stride (%" PRIu64 ") is zero.", pHitShaderBindingTable->size,
                                 pHitShaderBindingTable->stride);
            }
        }

        const char *vuid_single_device_memory = is_indirect ? "VUID-vkCmdTraceRaysIndirectKHR-pHitShaderBindingTable-03687"
                                                            : "VUID-vkCmdTraceRaysKHR-pHitShaderBindingTable-03687";
        const char *vuid_binding_table_flag = is_indirect ? "VUID-vkCmdTraceRaysIndirectKHR-pHitShaderBindingTable-03688"
                                                          : "VUID-vkCmdTraceRaysKHR-pHitShaderBindingTable-03688";
        skip |= ValidateRaytracingShaderBindingTable(cb_state.commandBuffer(), table_loc, vuid_single_device_memory,
                                                     vuid_binding_table_flag, *pHitShaderBindingTable);
    }

    if (pRaygenShaderBindingTable) {
        const Location table_loc = loc.dot(Field::pRaygenShaderBindingTable);
        const char *vuid_single_device_memory = is_indirect ? "VUID-vkCmdTraceRaysIndirectKHR-pRayGenShaderBindingTable-03680"
                                                            : "VUID-vkCmdTraceRaysKHR-pRayGenShaderBindingTable-03680";
        const char *vuid_binding_table_flag = is_indirect ? "VUID-vkCmdTraceRaysIndirectKHR-pRayGenShaderBindingTable-03681"
                                                          : "VUID-vkCmdTraceRaysKHR-pRayGenShaderBindingTable-03681";
        skip |= ValidateRaytracingShaderBindingTable(cb_state.commandBuffer(), table_loc, vuid_single_device_memory,
                                                     vuid_binding_table_flag, *pRaygenShaderBindingTable);
    }

    if (pMissShaderBindingTable) {
        const Location table_loc = loc.dot(Field::pMissShaderBindingTable);
        const char *vuid_single_device_memory = is_indirect ? "VUID-vkCmdTraceRaysIndirectKHR-pMissShaderBindingTable-03683"
                                                            : "VUID-vkCmdTraceRaysKHR-pMissShaderBindingTable-03683";
        const char *vuid_binding_table_flag = is_indirect ? "VUID-vkCmdTraceRaysIndirectKHR-pMissShaderBindingTable-03684"
                                                          : "VUID-vkCmdTraceRaysKHR-pMissShaderBindingTable-03684";
        skip |= ValidateRaytracingShaderBindingTable(cb_state.commandBuffer(), table_loc, vuid_single_device_memory,
                                                     vuid_binding_table_flag, *pMissShaderBindingTable);
    }

    if (pCallableShaderBindingTable) {
        const Location table_loc = loc.dot(Field::pCallableShaderBindingTable);
        const char *vuid_single_device_memory = is_indirect ? "VUID-vkCmdTraceRaysIndirectKHR-pCallableShaderBindingTable-03691"
                                                            : "VUID-vkCmdTraceRaysKHR-pCallableShaderBindingTable-03691";
        const char *vuid_binding_table_flag = is_indirect ? "VUID-vkCmdTraceRaysIndirectKHR-pCallableShaderBindingTable-03692"
                                                          : "VUID-vkCmdTraceRaysKHR-pCallableShaderBindingTable-03692";
        skip |= ValidateRaytracingShaderBindingTable(cb_state.commandBuffer(), table_loc, vuid_single_device_memory,
                                                     vuid_binding_table_flag, *pCallableShaderBindingTable);
    }
    return skip;
}

bool CoreChecks::PreCallValidateCmdTraceRaysKHR(VkCommandBuffer commandBuffer,
                                                const VkStridedDeviceAddressRegionKHR *pRaygenShaderBindingTable,
                                                const VkStridedDeviceAddressRegionKHR *pMissShaderBindingTable,
                                                const VkStridedDeviceAddressRegionKHR *pHitShaderBindingTable,
                                                const VkStridedDeviceAddressRegionKHR *pCallableShaderBindingTable, uint32_t width,
                                                uint32_t height, uint32_t depth, const ErrorObject &error_obj) const {
    bool skip = false;
    const auto &cb_state = *GetRead<CMD_BUFFER_STATE>(commandBuffer);
    skip |= ValidateCmd(cb_state, error_obj.location);
    if (skip) return skip;  // basic validation failed, might have null pointers

    // RayGen
    if (pRaygenShaderBindingTable->size != pRaygenShaderBindingTable->stride) {
        skip |= LogError("VUID-vkCmdTraceRaysKHR-size-04023", cb_state.GetObjectList(VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR),
                         error_obj.location.dot(Field::pRaygenShaderBindingTable),
                         "size (%" PRIu64 ") is not equal to stride (%" PRIu64 ").", pRaygenShaderBindingTable->size,
                         pRaygenShaderBindingTable->stride);
    }
    if (SafeModulo(pRaygenShaderBindingTable->deviceAddress, phys_dev_ext_props.ray_tracing_props_khr.shaderGroupBaseAlignment) !=
        0) {
        skip |=
            LogError("VUID-vkCmdTraceRaysKHR-pRayGenShaderBindingTable-03682",
                     cb_state.GetObjectList(VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR),
                     error_obj.location.dot(Field::pRaygenShaderBindingTable).dot(Field::deviceAddress),
                     "(%" PRIu64
                     ") must be a multiple of "
                     "VkPhysicalDeviceRayTracingPipelinePropertiesKHR::shaderGroupBaseAlignment (%" PRIu32 ").",
                     pRaygenShaderBindingTable->deviceAddress, phys_dev_ext_props.ray_tracing_props_khr.shaderGroupBaseAlignment);
    }
    // Callable
    if (SafeModulo(pCallableShaderBindingTable->stride, phys_dev_ext_props.ray_tracing_props_khr.shaderGroupHandleAlignment) != 0) {
        skip |= LogError("VUID-vkCmdTraceRaysKHR-stride-03694", cb_state.GetObjectList(VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR),
                         error_obj.location.dot(Field::pCallableShaderBindingTable).dot(Field::stride),
                         "(%" PRIu64
                         ") must be a multiple of "
                         "VkPhysicalDeviceRayTracingPipelinePropertiesKHR::shaderGroupHandleAlignment (%" PRIu32 ").",
                         pCallableShaderBindingTable->stride, phys_dev_ext_props.ray_tracing_props_khr.shaderGroupHandleAlignment);
    }
    if (pCallableShaderBindingTable->stride > phys_dev_ext_props.ray_tracing_props_khr.maxShaderGroupStride) {
        skip |= LogError(
            "VUID-vkCmdTraceRaysKHR-stride-04041", cb_state.GetObjectList(VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR),
            error_obj.location.dot(Field::pCallableShaderBindingTable).dot(Field::stride),
            "(%" PRIu64
            ") must be less than or equal to VkPhysicalDeviceRayTracingPipelinePropertiesKHR::maxShaderGroupStride (%" PRIu32 ").",
            pCallableShaderBindingTable->stride, phys_dev_ext_props.ray_tracing_props_khr.maxShaderGroupStride);
    }
    if (SafeModulo(pCallableShaderBindingTable->deviceAddress, phys_dev_ext_props.ray_tracing_props_khr.shaderGroupBaseAlignment) !=
        0) {
        skip |=
            LogError("VUID-vkCmdTraceRaysKHR-pCallableShaderBindingTable-03693",
                     cb_state.GetObjectList(VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR),
                     error_obj.location.dot(Field::pCallableShaderBindingTable).dot(Field::deviceAddress),
                     "(%" PRIu64
                     ") must be a multiple of "
                     "VkPhysicalDeviceRayTracingPipelinePropertiesKHR::shaderGroupBaseAlignment (%" PRIu32 ").",
                     pCallableShaderBindingTable->deviceAddress, phys_dev_ext_props.ray_tracing_props_khr.shaderGroupBaseAlignment);
    }
    // hitShader
    if (SafeModulo(pHitShaderBindingTable->stride, phys_dev_ext_props.ray_tracing_props_khr.shaderGroupHandleAlignment) != 0) {
        skip |= LogError("VUID-vkCmdTraceRaysKHR-stride-03690", cb_state.GetObjectList(VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR),
                         error_obj.location.dot(Field::pHitShaderBindingTable).dot(Field::stride),
                         "(%" PRIu64
                         ") must be a multiple of "
                         "VkPhysicalDeviceRayTracingPipelinePropertiesKHR::shaderGroupHandleAlignment (%" PRIu32 ").",
                         pHitShaderBindingTable->stride, phys_dev_ext_props.ray_tracing_props_khr.shaderGroupHandleAlignment);
    }
    if (pHitShaderBindingTable->stride > phys_dev_ext_props.ray_tracing_props_khr.maxShaderGroupStride) {
        skip |= LogError("VUID-vkCmdTraceRaysKHR-stride-04035", cb_state.GetObjectList(VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR),
                         error_obj.location.dot(Field::pHitShaderBindingTable).dot(Field::stride),
                         "(%" PRIu64
                         ") must be less than or equal to "
                         "VkPhysicalDeviceRayTracingPipelinePropertiesKHR::maxShaderGroupStride (%" PRIu32 ").",
                         pHitShaderBindingTable->stride, phys_dev_ext_props.ray_tracing_props_khr.maxShaderGroupStride);
    }
    if (SafeModulo(pHitShaderBindingTable->deviceAddress, phys_dev_ext_props.ray_tracing_props_khr.shaderGroupBaseAlignment) != 0) {
        skip |= LogError("VUID-vkCmdTraceRaysKHR-pHitShaderBindingTable-03689",
                         cb_state.GetObjectList(VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR),
                         error_obj.location.dot(Field::pHitShaderBindingTable).dot(Field::deviceAddress),
                         "(%" PRIu64
                         ") must be a multiple of "
                         "VkPhysicalDeviceRayTracingPipelinePropertiesKHR::shaderGroupBaseAlignment (%" PRIu32 ").",
                         pHitShaderBindingTable->deviceAddress, phys_dev_ext_props.ray_tracing_props_khr.shaderGroupBaseAlignment);
    }
    // missShader
    if (SafeModulo(pMissShaderBindingTable->stride, phys_dev_ext_props.ray_tracing_props_khr.shaderGroupHandleAlignment) != 0) {
        skip |= LogError("VUID-vkCmdTraceRaysKHR-stride-03686", cb_state.GetObjectList(VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR),
                         error_obj.location.dot(Field::pMissShaderBindingTable).dot(Field::stride),
                         "(%" PRIu64
                         ") must be a multiple of "
                         "VkPhysicalDeviceRayTracingPipelinePropertiesKHR::shaderGroupHandleAlignment (%" PRIu32 ").",
                         pMissShaderBindingTable->stride, phys_dev_ext_props.ray_tracing_props_khr.shaderGroupHandleAlignment);
    }
    if (pMissShaderBindingTable->stride > phys_dev_ext_props.ray_tracing_props_khr.maxShaderGroupStride) {
        skip |=
            LogError("VUID-vkCmdTraceRaysKHR-stride-04029", cb_state.GetObjectList(VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR),
                     error_obj.location.dot(Field::pMissShaderBindingTable).dot(Field::stride),
                     "(%" PRIu64
                     ") must be "
                     "less than or equal to VkPhysicalDeviceRayTracingPipelinePropertiesKHR::maxShaderGroupStride (%" PRIu32 ").",
                     pMissShaderBindingTable->stride, phys_dev_ext_props.ray_tracing_props_khr.maxShaderGroupStride);
    }
    if (SafeModulo(pMissShaderBindingTable->deviceAddress, phys_dev_ext_props.ray_tracing_props_khr.shaderGroupBaseAlignment) !=
        0) {
        skip |= LogError("VUID-vkCmdTraceRaysKHR-pMissShaderBindingTable-03685",
                         cb_state.GetObjectList(VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR),
                         error_obj.location.dot(Field::pMissShaderBindingTable).dot(Field::deviceAddress),
                         "(%" PRIu64
                         ") must be a multiple of "
                         "VkPhysicalDeviceRayTracingPipelinePropertiesKHR::shaderGroupBaseAlignment (%" PRIu32 ").",
                         pMissShaderBindingTable->deviceAddress, phys_dev_ext_props.ray_tracing_props_khr.shaderGroupBaseAlignment);
    }
    if (width * depth * height > phys_dev_ext_props.ray_tracing_props_khr.maxRayDispatchInvocationCount) {
        skip |= LogError("VUID-vkCmdTraceRaysKHR-width-03641", cb_state.GetObjectList(VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR),
                         error_obj.location,
                         "width x height x depth (%" PRIu32 " x %" PRIu32 " x %" PRIu32
                         ") must be less than or equal to "
                         "VkPhysicalDeviceRayTracingPipelinePropertiesKHR::maxRayDispatchInvocationCount (%" PRIu32 ").",
                         width, depth, height, phys_dev_ext_props.ray_tracing_props_khr.maxRayDispatchInvocationCount);
    }
    if (width > phys_dev_props.limits.maxComputeWorkGroupCount[0] * phys_dev_props.limits.maxComputeWorkGroupSize[0]) {
        skip |=
            LogError("VUID-vkCmdTraceRaysKHR-width-03638", cb_state.GetObjectList(VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR),
                     error_obj.location.dot(Field::width),
                     "(%" PRIu32
                     ") must be less than or equal to VkPhysicalDeviceLimits::maxComputeWorkGroupCount[0] "
                     "x VkPhysicalDeviceLimits::maxComputeWorkGroupSize[0] (%" PRIu32 " x %" PRIu32 ").",
                     width, phys_dev_props.limits.maxComputeWorkGroupCount[0], phys_dev_props.limits.maxComputeWorkGroupSize[0]);
    }

    if (height > phys_dev_props.limits.maxComputeWorkGroupCount[1] * phys_dev_props.limits.maxComputeWorkGroupSize[1]) {
        skip |=
            LogError("VUID-vkCmdTraceRaysKHR-height-03639", cb_state.GetObjectList(VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR),
                     error_obj.location.dot(Field::height),
                     "(%" PRIu32
                     ") must be less than or equal to VkPhysicalDeviceLimits::maxComputeWorkGroupCount[1] "
                     "x VkPhysicalDeviceLimits::maxComputeWorkGroupSize[1] (%" PRIu32 " x %" PRIu32 ").",
                     height, phys_dev_props.limits.maxComputeWorkGroupCount[1], phys_dev_props.limits.maxComputeWorkGroupSize[1]);
    }

    if (depth > phys_dev_props.limits.maxComputeWorkGroupCount[2] * phys_dev_props.limits.maxComputeWorkGroupSize[2]) {
        skip |=
            LogError("VUID-vkCmdTraceRaysKHR-depth-03640", cb_state.GetObjectList(VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR),
                     error_obj.location.dot(Field::depth),
                     "(%" PRIu32
                     ") must be less than or equal to VkPhysicalDeviceLimits::maxComputeWorkGroupCount[2] "
                     "x VkPhysicalDeviceLimits::maxComputeWorkGroupSize[2] (%" PRIu32 " x %" PRIu32 ").",
                     depth, phys_dev_props.limits.maxComputeWorkGroupCount[2], phys_dev_props.limits.maxComputeWorkGroupSize[2]);
    }

    skip |= ValidateActionState(cb_state, VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR, error_obj.location);
    skip |= ValidateCmdTraceRaysKHR(error_obj.location, cb_state, pRaygenShaderBindingTable, pMissShaderBindingTable,
                                    pHitShaderBindingTable, pCallableShaderBindingTable);
    return skip;
}

bool CoreChecks::PreCallValidateCmdTraceRaysIndirectKHR(VkCommandBuffer commandBuffer,
                                                        const VkStridedDeviceAddressRegionKHR *pRaygenShaderBindingTable,
                                                        const VkStridedDeviceAddressRegionKHR *pMissShaderBindingTable,
                                                        const VkStridedDeviceAddressRegionKHR *pHitShaderBindingTable,
                                                        const VkStridedDeviceAddressRegionKHR *pCallableShaderBindingTable,
                                                        VkDeviceAddress indirectDeviceAddress, const ErrorObject &error_obj) const {
    bool skip = false;
    const auto &cb_state = *GetRead<CMD_BUFFER_STATE>(commandBuffer);
    skip |= ValidateCmd(cb_state, error_obj.location);
    if (skip) return skip;  // basic validation failed, might have null pointers

    if (!enabled_features.rayTracingPipelineTraceRaysIndirect) {
        skip |= LogError("VUID-vkCmdTraceRaysIndirectKHR-rayTracingPipelineTraceRaysIndirect-03637",
                         cb_state.GetObjectList(VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR), error_obj.location,
                         "rayTracingPipelineTraceRaysIndirect feature must be enabled.");
    }
    // RayGen
    if (pRaygenShaderBindingTable->size != pRaygenShaderBindingTable->stride) {
        skip |= LogError(
            "VUID-vkCmdTraceRaysIndirectKHR-size-04023", cb_state.GetObjectList(VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR),
            error_obj.location.dot(Field::pRaygenShaderBindingTable), "size (%" PRIu64 ") is not equal to stride (%" PRIu64 ").",
            pRaygenShaderBindingTable->size, pRaygenShaderBindingTable->stride);
    }
    if (SafeModulo(pRaygenShaderBindingTable->deviceAddress, phys_dev_ext_props.ray_tracing_props_khr.shaderGroupBaseAlignment) !=
        0) {
        skip |=
            LogError("VUID-vkCmdTraceRaysIndirectKHR-pRayGenShaderBindingTable-03682",
                     cb_state.GetObjectList(VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR),
                     error_obj.location.dot(Field::pRaygenShaderBindingTable).dot(Field::deviceAddress),
                     "(%" PRIu64
                     ") must be a multiple of "
                     "VkPhysicalDeviceRayTracingPipelinePropertiesKHR::shaderGroupBaseAlignment (%" PRIu32 ").",
                     pRaygenShaderBindingTable->deviceAddress, phys_dev_ext_props.ray_tracing_props_khr.shaderGroupBaseAlignment);
    }
    // Callabe
    if (SafeModulo(pCallableShaderBindingTable->stride, phys_dev_ext_props.ray_tracing_props_khr.shaderGroupHandleAlignment) != 0) {
        skip |=
            LogError("VUID-vkCmdTraceRaysIndirectKHR-stride-03694", cb_state.GetObjectList(VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR),
                     error_obj.location.dot(Field::pCallableShaderBindingTable).dot(Field::stride),
                     "(%" PRIu64
                     ") must be a multiple of "
                     "VkPhysicalDeviceRayTracingPipelinePropertiesKHR::shaderGroupHandleAlignment (%" PRIu32 ").",
                     pCallableShaderBindingTable->stride, phys_dev_ext_props.ray_tracing_props_khr.shaderGroupHandleAlignment);
    }
    if (pCallableShaderBindingTable->stride > phys_dev_ext_props.ray_tracing_props_khr.maxShaderGroupStride) {
        skip |= LogError(
            "VUID-vkCmdTraceRaysIndirectKHR-stride-04041", cb_state.GetObjectList(VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR),
            error_obj.location.dot(Field::pCallableShaderBindingTable).dot(Field::stride),
            "(%" PRIu64
            ") must be less than or equal to VkPhysicalDeviceRayTracingPipelinePropertiesKHR::maxShaderGroupStride (%" PRIu32 ").",
            pCallableShaderBindingTable->stride, phys_dev_ext_props.ray_tracing_props_khr.maxShaderGroupStride);
    }
    if (SafeModulo(pCallableShaderBindingTable->deviceAddress, phys_dev_ext_props.ray_tracing_props_khr.shaderGroupBaseAlignment) !=
        0) {
        skip |=
            LogError("VUID-vkCmdTraceRaysIndirectKHR-pCallableShaderBindingTable-03693",
                     cb_state.GetObjectList(VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR),
                     error_obj.location.dot(Field::pCallableShaderBindingTable).dot(Field::deviceAddress),
                     "(%" PRIu64
                     ") must be a multiple of "
                     "VkPhysicalDeviceRayTracingPipelinePropertiesKHR::shaderGroupBaseAlignment (%" PRIu32 ").",
                     pCallableShaderBindingTable->deviceAddress, phys_dev_ext_props.ray_tracing_props_khr.shaderGroupBaseAlignment);
    }
    // hitShader
    if (SafeModulo(pHitShaderBindingTable->stride, phys_dev_ext_props.ray_tracing_props_khr.shaderGroupHandleAlignment) != 0) {
        skip |=
            LogError("VUID-vkCmdTraceRaysIndirectKHR-stride-03690", cb_state.GetObjectList(VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR),
                     error_obj.location.dot(Field::pHitShaderBindingTable).dot(Field::stride),
                     "(%" PRIu64
                     ") must be a multiple of "
                     "VkPhysicalDeviceRayTracingPipelinePropertiesKHR::shaderGroupHandleAlignment (%" PRIu32 ").",
                     pHitShaderBindingTable->stride, phys_dev_ext_props.ray_tracing_props_khr.shaderGroupHandleAlignment);
    }
    if (pHitShaderBindingTable->stride > phys_dev_ext_props.ray_tracing_props_khr.maxShaderGroupStride) {
        skip |=
            LogError("VUID-vkCmdTraceRaysIndirectKHR-stride-04035", cb_state.GetObjectList(VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR),
                     error_obj.location.dot(Field::pHitShaderBindingTable).dot(Field::stride),
                     "(%" PRIu64
                     ") must be less than or equal to "
                     "VkPhysicalDeviceRayTracingPipelinePropertiesKHR::maxShaderGroupStride (%" PRIu32 ").",
                     pHitShaderBindingTable->stride, phys_dev_ext_props.ray_tracing_props_khr.maxShaderGroupStride);
    }
    if (SafeModulo(pHitShaderBindingTable->deviceAddress, phys_dev_ext_props.ray_tracing_props_khr.shaderGroupBaseAlignment) != 0) {
        skip |= LogError("VUID-vkCmdTraceRaysIndirectKHR-pHitShaderBindingTable-03689",
                         cb_state.GetObjectList(VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR),
                         error_obj.location.dot(Field::pHitShaderBindingTable).dot(Field::deviceAddress),
                         "(%" PRIu64
                         ") must be a multiple of "
                         "VkPhysicalDeviceRayTracingPipelinePropertiesKHR::shaderGroupBaseAlignment (%" PRIu32 ").",
                         pHitShaderBindingTable->deviceAddress, phys_dev_ext_props.ray_tracing_props_khr.shaderGroupBaseAlignment);
    }
    // missShader
    if (SafeModulo(pMissShaderBindingTable->stride, phys_dev_ext_props.ray_tracing_props_khr.shaderGroupHandleAlignment) != 0) {
        skip |=
            LogError("VUID-vkCmdTraceRaysIndirectKHR-stride-03686", cb_state.GetObjectList(VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR),
                     error_obj.location.dot(Field::pMissShaderBindingTable).dot(Field::stride),
                     "(%" PRIu64
                     ") must be a multiple of "
                     "VkPhysicalDeviceRayTracingPipelinePropertiesKHR::shaderGroupHandleAlignment (%" PRIu32 ").",
                     pMissShaderBindingTable->stride, phys_dev_ext_props.ray_tracing_props_khr.shaderGroupHandleAlignment);
    }
    if (pMissShaderBindingTable->stride > phys_dev_ext_props.ray_tracing_props_khr.maxShaderGroupStride) {
        skip |=
            LogError("VUID-vkCmdTraceRaysIndirectKHR-stride-04029", cb_state.GetObjectList(VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR),
                     error_obj.location.dot(Field::pMissShaderBindingTable).dot(Field::stride),
                     "(%" PRIu64
                     ") must be "
                     "less than or equal to VkPhysicalDeviceRayTracingPipelinePropertiesKHR::maxShaderGroupStride (%" PRIu32 ").",
                     pMissShaderBindingTable->stride, phys_dev_ext_props.ray_tracing_props_khr.maxShaderGroupStride);
    }
    if (SafeModulo(pMissShaderBindingTable->deviceAddress, phys_dev_ext_props.ray_tracing_props_khr.shaderGroupBaseAlignment) !=
        0) {
        skip |= LogError("VUID-vkCmdTraceRaysIndirectKHR-pMissShaderBindingTable-03685",
                         cb_state.GetObjectList(VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR),
                         error_obj.location.dot(Field::pMissShaderBindingTable).dot(Field::deviceAddress),
                         "(%" PRIu64
                         ") must be a multiple of "
                         "VkPhysicalDeviceRayTracingPipelinePropertiesKHR::shaderGroupBaseAlignment (%" PRIu32 ").",
                         pMissShaderBindingTable->deviceAddress, phys_dev_ext_props.ray_tracing_props_khr.shaderGroupBaseAlignment);
    }
    if (SafeModulo(indirectDeviceAddress, 4) != 0) {
        skip |= LogError("VUID-vkCmdTraceRaysIndirectKHR-indirectDeviceAddress-03634",
                         cb_state.GetObjectList(VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR),
                         error_obj.location.dot(Field::indirectDeviceAddress), "(%" PRIu64 ") must be a multiple of 4.",
                         indirectDeviceAddress);
    }

    skip |= ValidateActionState(cb_state, VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR, error_obj.location);
    skip |= ValidateCmdTraceRaysKHR(error_obj.location, cb_state, pRaygenShaderBindingTable, pMissShaderBindingTable,
                                    pHitShaderBindingTable, pCallableShaderBindingTable);
    return skip;
}

bool CoreChecks::PreCallValidateCmdTraceRaysIndirect2KHR(VkCommandBuffer commandBuffer, VkDeviceAddress indirectDeviceAddress,
                                                         const ErrorObject &error_obj) const {
    bool skip = false;
    const auto &cb_state = *GetRead<CMD_BUFFER_STATE>(commandBuffer);
    skip |= ValidateCmd(cb_state, error_obj.location);
    if (skip) return skip;  // basic validation failed, might have null pointers

    if (!enabled_features.rayTracingPipelineTraceRaysIndirect2) {
        skip |= LogError("VUID-vkCmdTraceRaysIndirect2KHR-rayTracingPipelineTraceRaysIndirect2-03637",
                         cb_state.GetObjectList(VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR), error_obj.location,
                         "rayTracingPipelineTraceRaysIndirect2 feature was not enabled.");
    }

    if (SafeModulo(indirectDeviceAddress, 4) != 0) {
        skip |= LogError("VUID-vkCmdTraceRaysIndirect2KHR-indirectDeviceAddress-03634",
                         cb_state.GetObjectList(VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR),
                         error_obj.location.dot(Field::indirectDeviceAddress), "(%" PRIu64 ") must be a multiple of 4.",
                         indirectDeviceAddress);
    }

    skip = ValidateActionState(cb_state, VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR, error_obj.location);
    return skip;
}

bool CoreChecks::PreCallValidateCmdDrawMeshTasksNV(VkCommandBuffer commandBuffer, uint32_t taskCount, uint32_t firstTask,
                                                   const ErrorObject &error_obj) const {
    bool skip = false;
    const auto &cb_state = *GetRead<CMD_BUFFER_STATE>(commandBuffer);
    skip |= ValidateCmd(cb_state, error_obj.location);
    if (skip) return skip;  // basic validation failed, might have null pointers

    if (taskCount > phys_dev_ext_props.mesh_shader_props_nv.maxDrawMeshTasksCount) {
        skip |= LogError(
            "VUID-vkCmdDrawMeshTasksNV-taskCount-02119", cb_state.GetObjectList(VK_PIPELINE_BIND_POINT_GRAPHICS),
            error_obj.location.dot(Field::taskCount),
            "(0x%" PRIxLEAST32
            "), must be less than or equal to VkPhysicalDeviceMeshShaderPropertiesNV::maxDrawMeshTasksCount (0x%" PRIxLEAST32 ").",
            taskCount, phys_dev_ext_props.mesh_shader_props_nv.maxDrawMeshTasksCount);
    }
    skip = ValidateActionState(cb_state, VK_PIPELINE_BIND_POINT_GRAPHICS, error_obj.location);
    skip |= ValidateMeshShaderStage(cb_state, error_obj.location, true);
    return skip;
}

bool CoreChecks::PreCallValidateCmdDrawMeshTasksIndirectNV(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                           uint32_t drawCount, uint32_t stride,
                                                           const ErrorObject &error_obj) const {
    bool skip = false;
    const auto &cb_state = *GetRead<CMD_BUFFER_STATE>(commandBuffer);
    skip |= ValidateCmd(cb_state, error_obj.location);
    if (skip) return skip;  // basic validation failed, might have null pointers

    skip = ValidateActionState(cb_state, VK_PIPELINE_BIND_POINT_GRAPHICS, error_obj.location);
    auto buffer_state = Get<BUFFER_STATE>(buffer);
    skip |= ValidateIndirectCmd(cb_state, *buffer_state, error_obj.location);

    if (drawCount > 1) {
        skip |= ValidateCmdDrawStrideWithBuffer(cb_state, "VUID-vkCmdDrawMeshTasksIndirectNV-drawCount-02157", stride,
                                                Struct::VkDrawMeshTasksIndirectCommandNV, sizeof(VkDrawMeshTasksIndirectCommandNV),
                                                drawCount, offset, buffer_state.get(), error_obj.location);
        if (!enabled_features.multiDrawIndirect) {
            skip |= LogError("VUID-vkCmdDrawMeshTasksIndirectNV-drawCount-02718",
                             cb_state.GetObjectList(VK_PIPELINE_BIND_POINT_GRAPHICS), error_obj.location.dot(Field::drawCount),
                             "(%" PRIu32 ") must be 0 or 1 if multiDrawIndirect feature is not enabled.", drawCount);
        }
        if ((stride & 3) || stride < sizeof(VkDrawMeshTasksIndirectCommandNV)) {
            skip |= LogError(
                "VUID-vkCmdDrawMeshTasksIndirectNV-drawCount-02146", cb_state.GetObjectList(VK_PIPELINE_BIND_POINT_GRAPHICS),
                error_obj.location.dot(Field::stride),
                "(0x%" PRIxLEAST32 "), is not a multiple of 4 or smaller than sizeof (VkDrawMeshTasksIndirectCommandNV).", stride);
        }
    } else if (drawCount == 1 && ((offset + sizeof(VkDrawMeshTasksIndirectCommandNV)) > buffer_state.get()->createInfo.size)) {
        LogObjectList objlist = cb_state.GetObjectList(VK_PIPELINE_BIND_POINT_GRAPHICS);
        objlist.add(buffer);
        skip |=
            LogError("VUID-vkCmdDrawMeshTasksIndirectNV-drawCount-02156", objlist, error_obj.location,
                     "(offset + sizeof(VkDrawMeshTasksIndirectNV)) (%" PRIu64 ") is greater than the size of buffer (%" PRIu64 ").",
                     offset + sizeof(VkDrawMeshTasksIndirectCommandNV), buffer_state->createInfo.size);
    }
    if (offset & 3) {
        skip |= LogError("VUID-vkCmdDrawMeshTasksIndirectNV-offset-02710", cb_state.GetObjectList(VK_PIPELINE_BIND_POINT_GRAPHICS),
                         error_obj.location.dot(Field::offset), "(%" PRIu64 "), is not a multiple of 4.", offset);
    }
    if (drawCount > phys_dev_props.limits.maxDrawIndirectCount) {
        skip |= LogError("VUID-vkCmdDrawMeshTasksIndirectNV-drawCount-02719",
                         cb_state.GetObjectList(VK_PIPELINE_BIND_POINT_GRAPHICS), error_obj.location.dot(Field::drawCount),
                         "(%" PRIu32 ") is not less than or equal to maxDrawIndirectCount (%" PRIu32 ").", drawCount,
                         phys_dev_props.limits.maxDrawIndirectCount);
    }
    skip |= ValidateMeshShaderStage(cb_state, error_obj.location, true);
    return skip;
}

bool CoreChecks::PreCallValidateCmdDrawMeshTasksIndirectCountNV(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                                VkBuffer countBuffer, VkDeviceSize countBufferOffset,
                                                                uint32_t maxDrawCount, uint32_t stride,
                                                                const ErrorObject &error_obj) const {
    bool skip = false;
    const auto &cb_state = *GetRead<CMD_BUFFER_STATE>(commandBuffer);
    skip |= ValidateCmd(cb_state, error_obj.location);
    if (skip) return skip;  // basic validation failed, might have null pointers

    if (offset & 3) {
        skip |=
            LogError("VUID-vkCmdDrawMeshTasksIndirectCountNV-offset-02710", cb_state.GetObjectList(VK_PIPELINE_BIND_POINT_GRAPHICS),
                     error_obj.location.dot(Field::offset), "(%" PRIu64 "), is not a multiple of 4.", offset);
    }
    if (countBufferOffset & 3) {
        skip |= LogError("VUID-vkCmdDrawMeshTasksIndirectCountNV-countBufferOffset-02716",
                         cb_state.GetObjectList(VK_PIPELINE_BIND_POINT_GRAPHICS), error_obj.location.dot(Field::countBufferOffset),
                         "(%" PRIu64 "), is not a multiple of 4.", countBufferOffset);
    }

    skip = ValidateActionState(cb_state, VK_PIPELINE_BIND_POINT_GRAPHICS, error_obj.location);
    auto buffer_state = Get<BUFFER_STATE>(buffer);
    auto count_buffer_state = Get<BUFFER_STATE>(countBuffer);
    skip |= ValidateIndirectCmd(cb_state, *buffer_state, error_obj.location);
    skip |= ValidateIndirectCountCmd(cb_state, *count_buffer_state, countBufferOffset, error_obj.location);
    skip |= ValidateCmdDrawStrideWithStruct(cb_state, "VUID-vkCmdDrawMeshTasksIndirectCountNV-stride-02182", stride,
                                            Struct::VkDrawMeshTasksIndirectCommandNV, sizeof(VkDrawMeshTasksIndirectCommandNV),
                                            error_obj.location);
    if (maxDrawCount > 1) {
        skip |= ValidateCmdDrawStrideWithBuffer(cb_state, "VUID-vkCmdDrawMeshTasksIndirectCountNV-maxDrawCount-02183", stride,
                                                Struct::VkDrawMeshTasksIndirectCommandNV, sizeof(VkDrawMeshTasksIndirectCommandNV),
                                                maxDrawCount, offset, buffer_state.get(), error_obj.location);
    }
    skip |= ValidateMeshShaderStage(cb_state, error_obj.location, true);
    return skip;
}

bool CoreChecks::PreCallValidateCmdDrawMeshTasksEXT(VkCommandBuffer commandBuffer, uint32_t groupCountX, uint32_t groupCountY,
                                                    uint32_t groupCountZ, const ErrorObject &error_obj) const {
    bool skip = false;
    const auto &cb_state = *GetRead<CMD_BUFFER_STATE>(commandBuffer);
    skip |= ValidateCmd(cb_state, error_obj.location);
    if (skip) return skip;  // basic validation failed, might have null pointers

    skip = ValidateActionState(cb_state, VK_PIPELINE_BIND_POINT_GRAPHICS, error_obj.location);
    skip |= ValidateMeshShaderStage(cb_state, error_obj.location, false);

    if (groupCountX > phys_dev_ext_props.mesh_shader_props_ext.maxTaskWorkGroupCount[0]) {
        skip |= LogError(
            "VUID-vkCmdDrawMeshTasksEXT-TaskEXT-07322", cb_state.GetObjectList(VK_PIPELINE_BIND_POINT_GRAPHICS),
            error_obj.location.dot(Field::groupCountX),
            "(0x%" PRIxLEAST32
            "), must be less than or equal to VkPhysicalDeviceMeshShaderPropertiesEXT::maxTaskWorkGroupCount[0] (0x%" PRIxLEAST32
            ").",
            groupCountX, phys_dev_ext_props.mesh_shader_props_ext.maxTaskWorkGroupCount[0]);
    }
    if (groupCountY > phys_dev_ext_props.mesh_shader_props_ext.maxTaskWorkGroupCount[1]) {
        skip |= LogError(
            "VUID-vkCmdDrawMeshTasksEXT-TaskEXT-07323", cb_state.GetObjectList(VK_PIPELINE_BIND_POINT_GRAPHICS),
            error_obj.location.dot(Field::groupCountY),
            "(0x%" PRIxLEAST32
            "), must be less than or equal to VkPhysicalDeviceMeshShaderPropertiesEXT::maxTaskWorkGroupCount[1] (0x%" PRIxLEAST32
            ").",
            groupCountY, phys_dev_ext_props.mesh_shader_props_ext.maxTaskWorkGroupCount[1]);
    }
    if (groupCountZ > phys_dev_ext_props.mesh_shader_props_ext.maxTaskWorkGroupCount[2]) {
        skip |= LogError(
            "VUID-vkCmdDrawMeshTasksEXT-TaskEXT-07324", cb_state.GetObjectList(VK_PIPELINE_BIND_POINT_GRAPHICS),
            error_obj.location.dot(Field::groupCountZ),
            "(0x%" PRIxLEAST32
            "), must be less than or equal to VkPhysicalDeviceMeshShaderPropertiesEXT::maxTaskWorkGroupCount[2] (0x%" PRIxLEAST32
            ").",
            groupCountZ, phys_dev_ext_props.mesh_shader_props_ext.maxTaskWorkGroupCount[2]);
    }

    uint32_t maxTaskWorkGroupTotalCount = phys_dev_ext_props.mesh_shader_props_ext.maxTaskWorkGroupTotalCount;
    uint64_t invocations = static_cast<uint64_t>(groupCountX) * static_cast<uint64_t>(groupCountY);
    // Prevent overflow.
    bool fail = false;
    if (invocations > vvl::MaxTypeValue(maxTaskWorkGroupTotalCount) || invocations > maxTaskWorkGroupTotalCount) {
        fail = true;
    }
    if (!fail) {
        invocations *= static_cast<uint64_t>(groupCountZ);
        if (invocations > vvl::MaxTypeValue(maxTaskWorkGroupTotalCount) || invocations > maxTaskWorkGroupTotalCount) {
            fail = true;
        }
    }
    if (fail) {
        skip |= LogError(
            "VUID-vkCmdDrawMeshTasksEXT-TaskEXT-07325", cb_state.GetObjectList(VK_PIPELINE_BIND_POINT_GRAPHICS), error_obj.location,
            "The product of groupCountX (0x%" PRIxLEAST32 "), groupCountY (0x%" PRIxLEAST32 ") and groupCountZ (0x%" PRIxLEAST32
            ") must be less than or equal to "
            "VkPhysicalDeviceMeshShaderPropertiesEXT::maxTaskWorkGroupTotalCount (0x%" PRIxLEAST32 ").",
            groupCountX, groupCountY, groupCountZ, maxTaskWorkGroupTotalCount);
    }

    return skip;
}

bool CoreChecks::PreCallValidateCmdDrawMeshTasksIndirectEXT(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                            uint32_t drawCount, uint32_t stride,
                                                            const ErrorObject &error_obj) const {
    bool skip = false;
    const auto &cb_state = *GetRead<CMD_BUFFER_STATE>(commandBuffer);
    skip |= ValidateCmd(cb_state, error_obj.location);
    if (skip) return skip;  // basic validation failed, might have null pointers

    skip = ValidateActionState(cb_state, VK_PIPELINE_BIND_POINT_GRAPHICS, error_obj.location);
    auto buffer_state = Get<BUFFER_STATE>(buffer);
    skip |= ValidateIndirectCmd(cb_state, *buffer_state, error_obj.location);

    if (drawCount > 1) {
        skip |= ValidateCmdDrawStrideWithStruct(cb_state, "VUID-vkCmdDrawMeshTasksIndirectEXT-drawCount-07088", stride,
                                                Struct::VkDrawMeshTasksIndirectCommandEXT,
                                                sizeof(VkDrawMeshTasksIndirectCommandEXT), error_obj.location);
        skip |= ValidateCmdDrawStrideWithBuffer(
            cb_state, "VUID-vkCmdDrawMeshTasksIndirectEXT-drawCount-07090", stride, Struct::VkDrawMeshTasksIndirectCommandEXT,
            sizeof(VkDrawMeshTasksIndirectCommandEXT), drawCount, offset, buffer_state.get(), error_obj.location);
    }
    if ((drawCount == 1) && (offset + sizeof(VkDrawMeshTasksIndirectCommandEXT)) > buffer_state->createInfo.size) {
        LogObjectList objlist = cb_state.GetObjectList(VK_PIPELINE_BIND_POINT_GRAPHICS);
        objlist.add(buffer);
        skip |= LogError("VUID-vkCmdDrawMeshTasksIndirectEXT-drawCount-07089", objlist, error_obj.location.dot(Field::drawCount),
                         "is 1 and (offset + sizeof(vkCmdDrawMeshTasksIndirectEXT)) (%" PRIu64
                         ") is not less than "
                         "or equal to the size of buffer (%" PRIu64 ").",
                         (offset + sizeof(VkDrawMeshTasksIndirectCommandEXT)), buffer_state->createInfo.size);
    }
    // TODO: vkMapMemory() and check the contents of buffer at offset
    // issue #4547 (https://github.com/KhronosGroup/Vulkan-ValidationLayers/issues/4547)
    if (!enabled_features.multiDrawIndirect && ((drawCount > 1))) {
        skip |= LogError("VUID-vkCmdDrawMeshTasksIndirectEXT-drawCount-02718",
                         cb_state.GetObjectList(VK_PIPELINE_BIND_POINT_GRAPHICS), error_obj.location.dot(Field::drawCount),
                         "(%" PRIu32 ") must be 0 or 1 if multiDrawIndirect feature is not enabled.", drawCount);
    }
    if (drawCount > phys_dev_props.limits.maxDrawIndirectCount) {
        skip |= LogError("VUID-vkCmdDrawMeshTasksIndirectEXT-drawCount-02719",
                         cb_state.GetObjectList(VK_PIPELINE_BIND_POINT_GRAPHICS), error_obj.location.dot(Field::drawCount),
                         "%" PRIu32 ") is not less than or equal to maxDrawIndirectCount (%" PRIu32 ").", drawCount,
                         phys_dev_props.limits.maxDrawIndirectCount);
    }
    skip |= ValidateMeshShaderStage(cb_state, error_obj.location, false);
    return skip;
}

bool CoreChecks::PreCallValidateCmdDrawMeshTasksIndirectCountEXT(VkCommandBuffer commandBuffer, VkBuffer buffer,
                                                                 VkDeviceSize offset, VkBuffer countBuffer,
                                                                 VkDeviceSize countBufferOffset, uint32_t maxDrawCount,
                                                                 uint32_t stride, const ErrorObject &error_obj) const {
    const DrawDispatchVuid &vuid = GetDrawDispatchVuid(error_obj.location.function);

    bool skip = false;
    const auto &cb_state = *GetRead<CMD_BUFFER_STATE>(commandBuffer);
    skip |= ValidateCmd(cb_state, error_obj.location);
    if (skip) return skip;  // basic validation failed, might have null pointers

    skip = ValidateActionState(cb_state, VK_PIPELINE_BIND_POINT_GRAPHICS, error_obj.location);
    auto buffer_state = Get<BUFFER_STATE>(buffer);
    auto count_buffer_state = Get<BUFFER_STATE>(countBuffer);
    skip |= ValidateIndirectCmd(cb_state, *buffer_state, error_obj.location);
    skip |= ValidateMemoryIsBoundToBuffer(commandBuffer, *count_buffer_state, error_obj.location.dot(Field::countBuffer),
                                          vuid.indirect_count_contiguous_memory_02714);
    skip |= ValidateBufferUsageFlags(LogObjectList(commandBuffer, countBuffer), *count_buffer_state,
                                     VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT, true, vuid.indirect_count_buffer_bit_02715,
                                     error_obj.location.dot(Field::countBuffer));
    skip |= ValidateCmdDrawStrideWithStruct(cb_state, "VUID-vkCmdDrawMeshTasksIndirectCountEXT-stride-07096", stride,
                                            Struct::VkDrawMeshTasksIndirectCommandEXT, sizeof(VkDrawMeshTasksIndirectCommandEXT),
                                            error_obj.location);
    if (maxDrawCount > 1) {
        skip |=
            ValidateCmdDrawStrideWithBuffer(cb_state, "VUID-vkCmdDrawMeshTasksIndirectCountEXT-maxDrawCount-07097", stride,
                                            Struct::VkDrawMeshTasksIndirectCommandEXT, sizeof(VkDrawMeshTasksIndirectCommandEXT),
                                            maxDrawCount, offset, buffer_state.get(), error_obj.location);
    }
    skip |= ValidateMeshShaderStage(cb_state, error_obj.location, false);
    return skip;
}

// Action command == vkCmdDraw*, vkCmdDispatch*, vkCmdTraceRays*
// This is the main logic shared by all action commands
bool CoreChecks::ValidateActionState(const CMD_BUFFER_STATE &cb_state, const VkPipelineBindPoint bind_point,
                                     const Location &loc) const {
    const DrawDispatchVuid &vuid = GetDrawDispatchVuid(loc.function);
    const auto lv_bind_point = ConvertToLvlBindPoint(bind_point);
    const auto &last_bound_state = cb_state.lastBound[lv_bind_point];
    const auto *last_pipeline = last_bound_state.pipeline_state;
    const bool has_last_pipeline = last_pipeline && last_pipeline->pipeline() != VK_NULL_HANDLE;

    bool skip = false;

    if (!last_pipeline || !last_pipeline->pipeline()) {
        if (enabled_features.shaderObject == VK_FALSE) {
            return LogError(vuid.pipeline_bound_08606, cb_state.GetObjectList(bind_point), loc,
                            "A valid %s pipeline must be bound with vkCmdBindPipeline before calling this command.",
                            string_VkPipelineBindPoint(bind_point));
        } else if (!last_bound_state.ValidShaderObjectCombination(bind_point, enabled_features)) {
            skip |= LogError(vuid.pipeline_or_shaders_bound_08607, cb_state.GetObjectList(bind_point), loc,
                             "A valid %s pipeline must be bound with vkCmdBindPipeline or shader objects with "
                             "vkCmdBindShadersEXT before calling this command.",
                             string_VkPipelineBindPoint(bind_point));
        }
    }

    if (VK_PIPELINE_BIND_POINT_GRAPHICS == bind_point) {
        skip |= ValidateDrawDynamicState(last_bound_state, loc);
        skip |= ValidatePipelineDrawtimeState(last_bound_state, loc);

        if (enabled_features.shaderObject && !has_last_pipeline) {
            skip |= ValidateShaderObjectDrawtimeState(last_bound_state, loc);
        }

        if (cb_state.activeFramebuffer) {
            // Verify attachments for unprotected/protected command buffer.
            if (enabled_features.protectedMemory == VK_TRUE && cb_state.active_attachments) {
                uint32_t i = 0;
                for (const auto &view_state : *cb_state.active_attachments.get()) {
                    const auto &subpass = cb_state.active_subpasses->at(i);
                    if (subpass.used && view_state && !view_state->Destroyed()) {
                        std::string image_desc = "Image is ";
                        image_desc.append(string_VkImageUsageFlagBits(subpass.usage));
                        // Because inputAttachment is read only, it doesn't need to care protected command buffer case.
                        // Some Functions could not be protected. See VUID 02711.
                        if (subpass.usage != VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT &&
                            vuid.protected_command_buffer_02712 != kVUIDUndefined) {
                            skip |= ValidateUnprotectedImage(cb_state, *view_state->image_state, loc,
                                                             vuid.protected_command_buffer_02712, image_desc.c_str());
                        }
                        skip |= ValidateProtectedImage(cb_state, *view_state->image_state, loc,
                                                       vuid.unprotected_command_buffer_02707, image_desc.c_str());
                    }
                    ++i;
                }
            }
        }
    }

    const PIPELINE_STATE *pipeline = last_pipeline;
    // Now complete other state checks
    if (pipeline) {
        for (const auto &ds : last_bound_state.per_set) {
            if (pipeline->descriptor_buffer_mode) {
                if (ds.bound_descriptor_set && !ds.bound_descriptor_set->IsPushDescriptor()) {
                    const LogObjectList objlist(cb_state.Handle(), pipeline->Handle(), ds.bound_descriptor_set->Handle());
                    skip |=
                        LogError(vuid.descriptor_buffer_set_offset_missing_08117, objlist, loc,
                                 "pipeline bound to %s requires a descriptor buffer but has a bound descriptor set (%s)",
                                 string_VkPipelineBindPoint(bind_point), FormatHandle(ds.bound_descriptor_set->Handle()).c_str());
                    break;
                }

            } else {
                if (ds.bound_descriptor_buffer.has_value()) {
                    const LogObjectList objlist(cb_state.Handle(), pipeline->Handle());
                    skip |= LogError(vuid.descriptor_buffer_bit_not_set_08115, objlist, loc,
                                     "pipeline bound to %s requires a descriptor set but has a bound descriptor buffer"
                                     " (index=%" PRIu32 " offset=%" PRIu64 ")",
                                     string_VkPipelineBindPoint(bind_point), ds.bound_descriptor_buffer->index,
                                     ds.bound_descriptor_buffer->offset);
                    break;
                }
            }
        }

        std::string error_string;
        auto const &pipeline_layout = pipeline->PipelineLayoutState();

        // Check if the current pipeline is compatible for the maximum used set with the bound sets.
        if (!pipeline->descriptor_buffer_mode) {
            if (!pipeline->active_slots.empty() &&
                !IsBoundSetCompat(pipeline->max_active_slot, last_bound_state, *pipeline_layout)) {
                LogObjectList objlist(pipeline->pipeline());
                const auto layouts = pipeline->PipelineLayoutStateUnion();
                std::ostringstream pipe_layouts_log;
                if (layouts.size() > 1) {
                    pipe_layouts_log << "a union of layouts [ ";
                    for (const auto &layout : layouts) {
                        objlist.add(layout->layout());
                        pipe_layouts_log << FormatHandle(*layout) << " ";
                    }
                    pipe_layouts_log << "]";
                } else {
                    pipe_layouts_log << FormatHandle(*layouts.front());
                }
                objlist.add(last_bound_state.pipeline_layout);
                skip |= LogError(vuid.compatible_pipeline_08600, objlist, loc,
                                 "The %s (created with %s) statically uses descriptor set (index #%" PRIu32
                                 ") which is not compatible with the currently bound descriptor set's pipeline layout (%s)",
                                 FormatHandle(*pipeline).c_str(), pipe_layouts_log.str().c_str(), pipeline->max_active_slot,
                                 FormatHandle(last_bound_state.pipeline_layout).c_str());
            } else {
                // if the bound set is not copmatible, the rest will just be extra redundant errors
                for (const auto &set_binding_pair : pipeline->active_slots) {
                    uint32_t set_index = set_binding_pair.first;
                    const auto set_info = last_bound_state.per_set[set_index];
                    if (!set_info.bound_descriptor_set) {
                        skip |= LogError(vuid.compatible_pipeline_08600, cb_state.GetObjectList(bind_point), loc,
                                         "%s uses set #%" PRIu32 " but that set is not bound.", FormatHandle(*pipeline).c_str(),
                                         set_index);
                    } else if (!VerifySetLayoutCompatibility(*set_info.bound_descriptor_set, pipeline_layout->set_layouts,
                                                             pipeline_layout->Handle(), set_index, error_string)) {
                        // Set is bound but not compatible w/ overlapping pipeline_layout from PSO
                        VkDescriptorSet set_handle = set_info.bound_descriptor_set->GetSet();
                        LogObjectList objlist = cb_state.GetObjectList(bind_point);
                        objlist.add(set_handle);
                        objlist.add(pipeline_layout->layout());
                        skip |= LogError(vuid.compatible_pipeline_08600, objlist, loc,
                                         "%s bound as set #%" PRIu32 " is not compatible with overlapping %s due to: %s",
                                         FormatHandle(set_handle).c_str(), set_index, FormatHandle(*pipeline_layout).c_str(),
                                         error_string.c_str());
                    } else {  // Valid set is bound and layout compatible, validate that it's updated
                        // Pull the set node
                        const auto *descriptor_set = set_info.bound_descriptor_set.get();
                        assert(descriptor_set);
                        // Validate the draw-time state for this descriptor set
                        std::string err_str;
                        // For the "bindless" style resource usage with many descriptors, need to optimize command <-> descriptor
                        // binding validation. Take the requested binding set and prefilter it to eliminate redundant validation
                        // checks. Here, the currently bound pipeline determines whether an image validation check is redundant...
                        // for images are the "req" portion of the binding_req is indirectly (but tightly) coupled to the pipeline->
                        cvdescriptorset::PrefilterBindRequestMap reduced_map(*descriptor_set, set_binding_pair.second);
                        const auto &binding_req_map = reduced_map.FilteredMap(cb_state, pipeline);

                        // We can skip validating the descriptor set if "nothing" has changed since the last validation.
                        // Same set, no image layout changes, and same "pipeline state" (binding_req_map). If there are
                        // any dynamic descriptors, always revalidate rather than caching the values. We currently only
                        // apply this optimization if IsManyDescriptors is true, to avoid the overhead of copying the
                        // binding_req_map which could potentially be expensive.
                        bool descriptor_set_changed =
                            !reduced_map.IsManyDescriptors() ||
                            // Revalidate each time if the set has dynamic offsets
                            set_info.dynamicOffsets.size() > 0 ||
                            // Revalidate if descriptor set (or contents) has changed
                            set_info.validated_set != descriptor_set ||
                            set_info.validated_set_change_count != descriptor_set->GetChangeCount() ||
                            (!disabled[image_layout_validation] &&
                             set_info.validated_set_image_layout_change_count != cb_state.image_layout_change_count);
                        bool need_validate = descriptor_set_changed ||
                                             // Revalidate if previous bindingReqMap doesn't include new bindingReqMap
                                             !std::includes(set_info.validated_set_binding_req_map.begin(),
                                                            set_info.validated_set_binding_req_map.end(), binding_req_map.begin(),
                                                            binding_req_map.end());

                        if (need_validate) {
                            if (!descriptor_set_changed && reduced_map.IsManyDescriptors()) {
                                // Only validate the bindings that haven't already been validated
                                BindingVariableMap delta_reqs;
                                std::set_difference(binding_req_map.begin(), binding_req_map.end(),
                                                    set_info.validated_set_binding_req_map.begin(),
                                                    set_info.validated_set_binding_req_map.end(),
                                                    vvl::insert_iterator<BindingVariableMap>(delta_reqs, delta_reqs.begin()));
                                skip |=
                                    ValidateDrawState(*descriptor_set, delta_reqs, set_info.dynamicOffsets, cb_state, loc, vuid);
                            } else {
                                skip |= ValidateDrawState(*descriptor_set, binding_req_map, set_info.dynamicOffsets, cb_state, loc,
                                                          vuid);
                            }
                        }
                    }
                }
            }
        }
    } else {
        std::string error_string;

        const auto are_bound_sets_compat = [](uint32_t set, const LAST_BOUND_STATE &last_bound,
                                              const SHADER_OBJECT_STATE &shader_object_state) {
            if ((set >= last_bound.per_set.size()) || (set >= shader_object_state.set_compat_ids.size())) {
                return false;
            }
            return (*(last_bound.per_set[set].compat_id_for_set) == *(shader_object_state.set_compat_ids[set]));
        };

        // Check if the current shader objects are compatible for the maximum used set with the bound sets.
        for (const auto &shader_state : last_bound_state.shader_object_states) {
            if (!shader_state) {
                continue;
            }
            if (shader_state && !shader_state->active_slots.empty() &&
                !are_bound_sets_compat(shader_state->max_active_slot, last_bound_state, *shader_state)) {
                LogObjectList objlist(cb_state.commandBuffer(), shader_state->shader());
                skip |= LogError(vuid.compatible_pipeline_08600, objlist, loc,
                                 "The %s statically uses descriptor set (index #%" PRIu32
                                 ") which is not compatible with the currently bound descriptor set's layout",
                                 FormatHandle(shader_state->shader()).c_str(), shader_state->max_active_slot);
            } else {
                // if the bound set is not copmatible, the rest will just be extra redundant errors
                for (const auto &set_binding_pair : shader_state->active_slots) {
                    uint32_t set_index = set_binding_pair.first;
                    const auto set_info = last_bound_state.per_set[set_index];
                    if (!set_info.bound_descriptor_set) {
                        const LogObjectList objlist(cb_state.commandBuffer(), shader_state->shader());
                        skip |= LogError(vuid.compatible_pipeline_08600, objlist, loc,
                                         "%s uses set #%" PRIu32 " but that set is not bound.",
                                         FormatHandle(shader_state->shader()).c_str(), set_index);
                    } else if (!VerifySetLayoutCompatibility(*set_info.bound_descriptor_set, shader_state->set_layouts,
                                                             shader_state->Handle(), set_index, error_string)) {
                        // Set is bound but not compatible w/ overlapping pipeline_layout from PSO
                        VkDescriptorSet set_handle = set_info.bound_descriptor_set->GetSet();
                        const LogObjectList objlist(cb_state.commandBuffer(), set_handle, shader_state->shader());
                        skip |= LogError(vuid.compatible_pipeline_08600, objlist, loc,
                                         "%s bound as set #%" PRIu32 " is not compatible with overlapping %s due to: %s",
                                         FormatHandle(set_handle).c_str(), set_index, FormatHandle(shader_state->shader()).c_str(),
                                         error_string.c_str());
                    } else {  // Valid set is bound and layout compatible, validate that it's updated
                        // Pull the set node
                        const auto *descriptor_set = set_info.bound_descriptor_set.get();
                        assert(descriptor_set);
                        // Validate the draw-time state for this descriptor set
                        std::string err_str;
                        // For the "bindless" style resource usage with many descriptors, need to optimize command <-> descriptor
                        // binding validation. Take the requested binding set and prefilter it to eliminate redundant validation
                        // checks. Here, the currently bound pipeline determines whether an image validation check is redundant...
                        // for images are the "req" portion of the binding_req is indirectly (but tightly) coupled to the pipeline->
                        cvdescriptorset::PrefilterBindRequestMap reduced_map(*descriptor_set, set_binding_pair.second);
                        const auto &binding_req_map = reduced_map.FilteredMap(cb_state, nullptr);

                        // We can skip validating the descriptor set if "nothing" has changed since the last validation.
                        // Same set, no image layout changes, and same "pipeline state" (binding_req_map). If there are
                        // any dynamic descriptors, always revalidate rather than caching the values. We currently only
                        // apply this optimization if IsManyDescriptors is true, to avoid the overhead of copying the
                        // binding_req_map which could potentially be expensive.
                        bool descriptor_set_changed =
                            !reduced_map.IsManyDescriptors() ||
                            // Revalidate each time if the set has dynamic offsets
                            set_info.dynamicOffsets.size() > 0 ||
                            // Revalidate if descriptor set (or contents) has changed
                            set_info.validated_set != descriptor_set ||
                            set_info.validated_set_change_count != descriptor_set->GetChangeCount() ||
                            (!disabled[image_layout_validation] &&
                             set_info.validated_set_image_layout_change_count != cb_state.image_layout_change_count);
                        bool need_validate = descriptor_set_changed ||
                                             // Revalidate if previous bindingReqMap doesn't include new bindingReqMap
                                             !std::includes(set_info.validated_set_binding_req_map.begin(),
                                                            set_info.validated_set_binding_req_map.end(), binding_req_map.begin(),
                                                            binding_req_map.end());

                        if (need_validate) {
                            if (!descriptor_set_changed && reduced_map.IsManyDescriptors()) {
                                // Only validate the bindings that haven't already been validated
                                BindingVariableMap delta_reqs;
                                std::set_difference(binding_req_map.begin(), binding_req_map.end(),
                                                    set_info.validated_set_binding_req_map.begin(),
                                                    set_info.validated_set_binding_req_map.end(),
                                                    vvl::insert_iterator<BindingVariableMap>(delta_reqs, delta_reqs.begin()));
                                skip |=
                                    ValidateDrawState(*descriptor_set, delta_reqs, set_info.dynamicOffsets, cb_state, loc, vuid);
                            } else {
                                skip |= ValidateDrawState(*descriptor_set, binding_req_map, set_info.dynamicOffsets, cb_state, loc,
                                                          vuid);
                            }
                        }
                    }
                }
            }
        }
    }

    // Verify if push constants have been set
    // NOTE: Currently not checking whether active push constants are compatible with the active pipeline, nor whether the
    //       "life times" of push constants are correct.
    //       Discussion on validity of these checks can be found at https://gitlab.khronos.org/vulkan/vulkan/-/issues/2602.
    if (pipeline) {
        auto const &pipeline_layout = pipeline->PipelineLayoutState();
        if (!cb_state.push_constant_data_ranges || (pipeline_layout->push_constant_ranges == cb_state.push_constant_data_ranges)) {
            for (const auto &stage : pipeline->stage_states) {
                if (!stage.entrypoint || !stage.entrypoint->push_constant_variable) {
                    continue;  // no static push constant in shader
                }

                // Edge case where if the shader is using push constants statically and there never was a vkCmdPushConstants
                if (!cb_state.push_constant_data_ranges && !enabled_features.maintenance4) {
                    const LogObjectList objlist(cb_state.commandBuffer(), pipeline_layout->layout(), pipeline->pipeline());
                    skip |= LogError(vuid.push_constants_set_08602, objlist, loc,
                                     "Shader in %s uses push-constant statically but vkCmdPushConstants was not called yet for "
                                     "pipeline layout %s.",
                                     string_VkShaderStageFlags(stage.GetStage()).c_str(),
                                     FormatHandle(pipeline_layout->layout()).c_str());
                }
            }
        }
    } else {
        if (!cb_state.push_constant_data_ranges) {
            for (const auto &stage : last_bound_state.shader_object_states) {
                if (!stage || !stage->entrypoint || !stage->entrypoint->push_constant_variable) {
                    continue;
                }
                // Edge case where if the shader is using push constants statically and there never was a vkCmdPushConstants
                if (!cb_state.push_constant_data_ranges && !enabled_features.maintenance4) {
                    const LogObjectList objlist(cb_state.commandBuffer(), stage->shader());
                    skip |= LogError(vuid.push_constants_set_08602, objlist, loc,
                                     "Shader in %s uses push-constant statically but vkCmdPushConstants was not called yet.",
                                     string_VkShaderStageFlags(stage->create_info.stage).c_str());
                }
            }
        }
    }

    skip |= ValidateCmdRayQueryState(cb_state, bind_point, loc);

    if (pipeline) {
        if ((pipeline->create_info_shaders & (VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT |
                                             VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT | VK_SHADER_STAGE_GEOMETRY_BIT)) != 0) {
            for (const auto &query : cb_state.activeQueries) {
                const auto query_pool_state = Get<QUERY_POOL_STATE>(query.pool);
                if (query_pool_state->createInfo.queryType == VK_QUERY_TYPE_MESH_PRIMITIVES_GENERATED_EXT) {
                    const LogObjectList objlist(cb_state.commandBuffer(), query.pool);
                    skip |= LogError(vuid.mesh_shader_queries_07073, objlist, loc,
                                     "Query (slot %" PRIu32 ") with type VK_QUERY_TYPE_MESH_PRIMITIVES_GENERATED_EXT is active.",
                                     query.slot);
                }
            }
        }
    }

    return skip;
}

bool CoreChecks::MatchSampleLocationsInfo(const VkSampleLocationsInfoEXT *pSampleLocationsInfo1,
                                          const VkSampleLocationsInfoEXT *pSampleLocationsInfo2) const {
    if (pSampleLocationsInfo1->sampleLocationsPerPixel != pSampleLocationsInfo2->sampleLocationsPerPixel ||
        pSampleLocationsInfo1->sampleLocationGridSize.width != pSampleLocationsInfo2->sampleLocationGridSize.width ||
        pSampleLocationsInfo1->sampleLocationGridSize.height != pSampleLocationsInfo2->sampleLocationGridSize.height ||
        pSampleLocationsInfo1->sampleLocationsCount != pSampleLocationsInfo2->sampleLocationsCount) {
        return false;
    }
    for (uint32_t i = 0; i < pSampleLocationsInfo1->sampleLocationsCount; ++i) {
        if (pSampleLocationsInfo1->pSampleLocations[i].x != pSampleLocationsInfo2->pSampleLocations[i].x ||
            pSampleLocationsInfo1->pSampleLocations[i].y != pSampleLocationsInfo2->pSampleLocations[i].y) {
            return false;
        }
    }
    return true;
}

bool CoreChecks::ValidateIndirectCmd(const CMD_BUFFER_STATE &cb_state, const BUFFER_STATE &buffer_state,
                                     const Location &loc) const {
    bool skip = false;
    const DrawDispatchVuid &vuid = GetDrawDispatchVuid(loc.function);
    LogObjectList objlist = cb_state.GetObjectList(VK_PIPELINE_BIND_POINT_GRAPHICS);
    objlist.add(buffer_state.Handle());

    skip |= ValidateMemoryIsBoundToBuffer(cb_state.commandBuffer(), buffer_state, loc.dot(Field::buffer),
                                          vuid.indirect_contiguous_memory_02708);
    skip |= ValidateBufferUsageFlags(objlist, buffer_state, VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT, true,
                                     vuid.indirect_buffer_bit_02290, loc.dot(Field::buffer));
    if (cb_state.unprotected == false) {
        skip |= LogError(vuid.indirect_protected_cb_02646, objlist, loc,
                         "Indirect commands can't be used in protected command buffers.");
    }
    return skip;
}

bool CoreChecks::ValidateIndirectCountCmd(const CMD_BUFFER_STATE &cb_state, const BUFFER_STATE &count_buffer_state,
                                          VkDeviceSize count_buffer_offset, const Location &loc) const {
    bool skip = false;
    const DrawDispatchVuid &vuid = GetDrawDispatchVuid(loc.function);
    LogObjectList objlist = cb_state.GetObjectList(VK_PIPELINE_BIND_POINT_GRAPHICS);
    objlist.add(count_buffer_state.Handle());

    skip |= ValidateMemoryIsBoundToBuffer(cb_state.commandBuffer(), count_buffer_state, loc.dot(Field::countBuffer),
                                          vuid.indirect_count_contiguous_memory_02714);
    skip |= ValidateBufferUsageFlags(objlist, count_buffer_state, VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT, true,
                                     vuid.indirect_count_buffer_bit_02715, loc.dot(Field::countBuffer));
    if (count_buffer_offset + sizeof(uint32_t) > count_buffer_state.createInfo.size) {
        skip |= LogError(vuid.indirect_count_offset_04129, objlist, loc,
                         "countBufferOffset (%" PRIu64 ") + sizeof(uint32_t) is greater than the buffer size of %" PRIu64 ".",
                         count_buffer_offset, count_buffer_state.createInfo.size);
    }
    return skip;
}
