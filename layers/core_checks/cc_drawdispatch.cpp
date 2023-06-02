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
#include "cc_buffer_address.h"

// clang-format off
struct DispatchVuidsCmdDraw : DrawDispatchVuid {
    DispatchVuidsCmdDraw() : DrawDispatchVuid() {
        pipeline_bound_02700                     = "VUID-vkCmdDraw-None-02700";
        vertex_binding_04007                     = "VUID-vkCmdDraw-None-04007";
        vertex_binding_null_04008                = "VUID-vkCmdDraw-None-04008";
        compatible_pipeline_02697                = "VUID-vkCmdDraw-None-02697";
        render_pass_compatible_02684             = "VUID-vkCmdDraw-renderPass-02684";
        subpass_index_02685                      = "VUID-vkCmdDraw-subpass-02685";
        sample_location_02689                    = "VUID-vkCmdDraw-sampleLocationsEnable-02689";
        linear_filter_sampler_04553              = "VUID-vkCmdDraw-magFilter-04553";
        linear_mipmap_sampler_04770              = "VUID-vkCmdDraw-mipmapMode-04770";
        cubic_sampler_02692                      = "VUID-vkCmdDraw-None-02692";
        viewport_count_03417                     = "VUID-vkCmdDraw-viewportCount-03417";
        scissor_count_03418                      = "VUID-vkCmdDraw-scissorCount-03418";
        viewport_scissor_count_03419             = "VUID-vkCmdDraw-viewportCount-03419";
        primitive_topology_class_03420           = "VUID-vkCmdDraw-primitiveTopology-03420";
        primitive_topology_class_ds3_07500       = "VUID-vkCmdDraw-dynamicPrimitiveTopologyUnrestricted-07500";
        corner_sampled_address_mode_02696        = "VUID-vkCmdDraw-flags-02696";
        imageview_atomic_02691                   = "VUID-vkCmdDraw-None-02691";
        bufferview_atomic_07888                  = "VUID-vkCmdDraw-None-07888";
        push_constants_set_06425                 = "VUID-vkCmdDraw-maintenance4-06425";
        image_subresources_render_pass_write_06537 = "VUID-vkCmdDraw-None-06537";
        image_subresources_subpass_read_06538    = "VUID-vkCmdDraw-None-06538";
        image_subresources_subpass_write_06539   = "VUID-vkCmdDraw-None-06539";
        descriptor_valid_02699                   = "VUID-vkCmdDraw-None-02699";
        sampler_imageview_type_02702             = "VUID-vkCmdDraw-None-02702";
        sampler_implicitLod_dref_proj_02703      = "VUID-vkCmdDraw-None-02703";
        sampler_bias_offset_02704                = "VUID-vkCmdDraw-None-02704";
        vertex_binding_attribute_02721           = "VUID-vkCmdDraw-None-02721";
        dynamic_state_setting_commands_02859     = "VUID-vkCmdDraw-None-02859";
        rasterization_samples_04740              = "VUID-vkCmdDraw-rasterizationSamples-04740";
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
        vertex_input_binding_stride_04884        = "VUID-vkCmdDraw-pStrides-04884";
        vertex_input_04914                       = "VUID-vkCmdDraw-None-04914";
        blend_enable_04727                       = "VUID-vkCmdDraw-blendEnable-04727";
        dynamic_discard_rectangle_07751          = "VUID-vkCmdDraw-None-07751";
        dynamic_discard_rectangle_enable_07880   = "VUID-vkCmdDraw-None-07880";
        dynamic_discard_rectangle_mode_07881     =  "VUID-vkCmdDraw-None-07881";
        dynamic_exclusive_scissor_07878          = "VUID-vkCmdDraw-None-07878";
        dynamic_exclusive_scissor_enable_07879   = "VUID-vkCmdDraw-None-07879";
        dynamic_color_write_enable_07749         = "VUID-vkCmdDraw-None-07749";
        dynamic_color_write_enable_count_07750   = "VUID-vkCmdDraw-attachmentCount-07750";
        dynamic_attachment_feedback_loop_08877   = "VUID-vkCmdDraw-None-08877";
        dynamic_rendering_view_mask_06178        = "VUID-vkCmdDraw-viewMask-06178";
        dynamic_rendering_color_count_06179      = "VUID-vkCmdDraw-colorAttachmentCount-06179";
        dynamic_rendering_color_formats_06180    = "VUID-vkCmdDraw-colorAttachmentCount-06180";
        dynamic_rendering_unused_attachments_08911 = "VUID-vkCmdDraw-dynamicRenderingUnusedAttachments-08911";
        dynamic_rendering_undefined_color_formats_07616 = "VUID-vkCmdDraw-colorAttachmentCount-07616";
        dynamic_rendering_undefined_depth_format_07617 = "VUID-vkCmdDraw-pDepthAttachment-07617";
        dynamic_rendering_undefined_stencil_format_07618 = "VUID-vkCmdDraw-pStencilAttachment-07618";
        dynamic_rendering_depth_format_06181     = "VUID-vkCmdDraw-pDepthAttachment-06181";
        dynamic_rendering_unused_attachments_08915 = "VUID-vkCmdDraw-dynamicRenderingUnusedAttachments-08915";
        dynamic_rendering_stencil_format_06182   = "VUID-vkCmdDraw-pStencilAttachment-06182";
        dynamic_rendering_unused_attachments_08918 = "VUID-vkCmdDraw-dynamicRenderingUnusedAttachments-08918";
        dynamic_rendering_fsr_06183              = "VUID-vkCmdDraw-imageView-06183";
        dynamic_rendering_fdm_06184              = "VUID-vkCmdDraw-imageView-06184";
        dynamic_rendering_color_sample_06185     = "VUID-vkCmdDraw-colorAttachmentCount-06185";
        dynamic_rendering_depth_sample_06186     = "VUID-vkCmdDraw-pDepthAttachment-06186";
        dynamic_rendering_stencil_sample_06187   = "VUID-vkCmdDraw-pStencilAttachment-06187";
        dynamic_rendering_multi_sample_06188     = "VUID-vkCmdDraw-colorAttachmentCount-06188";
        dynamic_rendering_06189                  = "VUID-vkCmdDraw-pDepthAttachment-06189";
        dynamic_rendering_06190                  = "VUID-vkCmdDraw-pStencilAttachment-06190";
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
        storage_image_write_texel_count_04115           = "VUID-vkCmdDraw-None-04115";
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
        dynamic_coverage_reduction_mode_07647    = "VUID-vkCmdDraw-None-07647";
        dynamic_representative_fragment_test_enable_07648  = "VUID-vkCmdDraw-None-07648";
        dynamic_shading_rate_image_enable_07649  = "VUID-vkCmdDraw-None-07649";
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
    }
};

struct DispatchVuidsCmdDrawMultiEXT : DrawDispatchVuid {
    DispatchVuidsCmdDrawMultiEXT() : DrawDispatchVuid() {
        pipeline_bound_02700                     = "VUID-vkCmdDrawMultiEXT-None-02700";
        vertex_binding_04007                     = "VUID-vkCmdDrawMultiEXT-None-04007";
        vertex_binding_null_04008                = "VUID-vkCmdDrawMultiEXT-None-04008";
        compatible_pipeline_02697                = "VUID-vkCmdDrawMultiEXT-None-02697";
        render_pass_compatible_02684             = "VUID-vkCmdDrawMultiEXT-renderPass-02684";
        subpass_index_02685                      = "VUID-vkCmdDrawMultiEXT-subpass-02685";
        sample_location_02689                    = "VUID-vkCmdDrawMultiEXT-sampleLocationsEnable-02689";
        linear_filter_sampler_04553              = "VUID-vkCmdDrawMultiEXT-magFilter-04553";
        linear_mipmap_sampler_04770              = "VUID-vkCmdDrawMultiEXT-mipmapMode-04770";
        cubic_sampler_02692                      = "VUID-vkCmdDrawMultiEXT-None-02692";
        viewport_count_03417                     = "VUID-vkCmdDrawMultiEXT-viewportCount-03417";
        scissor_count_03418                      = "VUID-vkCmdDrawMultiEXT-scissorCount-03418";
        viewport_scissor_count_03419             = "VUID-vkCmdDrawMultiEXT-viewportCount-03419";
        primitive_topology_class_03420           = "VUID-vkCmdDrawMultiEXT-primitiveTopology-03420";
        primitive_topology_class_ds3_07500       = "VUID-vkCmdDrawMultiEXT-dynamicPrimitiveTopologyUnrestricted-07500";
        corner_sampled_address_mode_02696        = "VUID-vkCmdDrawMultiEXT-flags-02696";
        imageview_atomic_02691                   = "VUID-vkCmdDrawMultiEXT-None-02691";
        bufferview_atomic_07888                  = "VUID-vkCmdDrawMultiEXT-None-07888";
        push_constants_set_06425                 = "VUID-vkCmdDrawMultiEXT-maintenance4-06425";
        image_subresources_render_pass_write_06537 = "VUID-vkCmdDrawMultiEXT-None-06537";
        image_subresources_subpass_read_06538    = "VUID-vkCmdDrawMultiEXT-None-06538";
        image_subresources_subpass_write_06539   = "VUID-vkCmdDrawMultiEXT-None-06539";
        descriptor_valid_02699                   = "VUID-vkCmdDrawMultiEXT-None-02699";
        sampler_imageview_type_02702             = "VUID-vkCmdDrawMultiEXT-None-02702";
        sampler_implicitLod_dref_proj_02703      = "VUID-vkCmdDrawMultiEXT-None-02703";
        sampler_bias_offset_02704                = "VUID-vkCmdDrawMultiEXT-None-02704";
        vertex_binding_attribute_02721           = "VUID-vkCmdDrawMultiEXT-None-02721";
        dynamic_state_setting_commands_02859     = "VUID-vkCmdDrawMultiEXT-None-02859";
        rasterization_samples_04740              = "VUID-vkCmdDrawMultiEXT-rasterizationSamples-04740";
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
        vertex_input_binding_stride_04884        = "VUID-vkCmdDrawMultiEXT-pStrides-04884";
        vertex_input_04914                       = "VUID-vkCmdDrawMultiEXT-None-04914";
        blend_enable_04727                       = "VUID-vkCmdDrawMultiEXT-blendEnable-04727";
        dynamic_discard_rectangle_07751          = "VUID-vkCmdDrawMultiEXT-None-07751";
        dynamic_discard_rectangle_enable_07880   = "VUID-vkCmdDrawMultiEXT-None-07880";
        dynamic_discard_rectangle_mode_07881     = "VUID-vkCmdDrawMultiEXT-None-07881";
        dynamic_exclusive_scissor_07878          = "VUID-vkCmdDrawMultiEXT-None-07878";
        dynamic_exclusive_scissor_enable_07879   = "VUID-vkCmdDrawMultiEXT-None-07879";
        dynamic_color_write_enable_07749         = "VUID-vkCmdDrawMultiEXT-None-07749";
        dynamic_color_write_enable_count_07750   = "VUID-vkCmdDrawMultiEXT-attachmentCount-07750";
        dynamic_attachment_feedback_loop_08877   = "VUID-vkCmdDrawMultiEXT-None-08877";
        dynamic_rendering_view_mask_06178        = "VUID-vkCmdDrawMultiEXT-viewMask-06178";
        dynamic_rendering_color_count_06179      = "VUID-vkCmdDrawMultiEXT-colorAttachmentCount-06179";
        dynamic_rendering_color_formats_06180    = "VUID-vkCmdDrawMultiEXT-colorAttachmentCount-06180";
        dynamic_rendering_unused_attachments_08911 = "VUID-vkCmdDrawMultiEXT-dynamicRenderingUnusedAttachments-08911";
        dynamic_rendering_undefined_color_formats_07616 = "VUID-vkCmdDrawMultiEXT-colorAttachmentCount-07616";
        dynamic_rendering_undefined_depth_format_07617 = "VUID-vkCmdDrawMultiEXT-pDepthAttachment-07617";
        dynamic_rendering_undefined_stencil_format_07618 = "VUID-vkCmdDrawMultiEXT-pStencilAttachment-07618";
        dynamic_rendering_depth_format_06181     = "VUID-vkCmdDrawMultiEXT-pDepthAttachment-06181";
        dynamic_rendering_unused_attachments_08915 = "VUID-vkCmdDrawMultiEXT-dynamicRenderingUnusedAttachments-08915";
        dynamic_rendering_stencil_format_06182   = "VUID-vkCmdDrawMultiEXT-pStencilAttachment-06182";
        dynamic_rendering_unused_attachments_08918 = "VUID-vkCmdDrawMultiEXT-dynamicRenderingUnusedAttachments-08918";
        dynamic_rendering_fsr_06183              = "VUID-vkCmdDrawMultiEXT-imageView-06183";
        dynamic_rendering_fdm_06184              = "VUID-vkCmdDrawMultiEXT-imageView-06184";
        dynamic_rendering_color_sample_06185     = "VUID-vkCmdDrawMultiEXT-colorAttachmentCount-06185";
        dynamic_rendering_depth_sample_06186     = "VUID-vkCmdDrawMultiEXT-pDepthAttachment-06186";
        dynamic_rendering_stencil_sample_06187   = "VUID-vkCmdDrawMultiEXT-pStencilAttachment-06187";
        dynamic_rendering_multi_sample_06188     = "VUID-vkCmdDrawMultiEXT-colorAttachmentCount-06188";
        dynamic_rendering_06189                  = "VUID-vkCmdDrawMultiEXT-pDepthAttachment-06189";
        dynamic_rendering_06190                  = "VUID-vkCmdDrawMultiEXT-pStencilAttachment-06190";
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
        storage_image_write_texel_count_04115           = "VUID-vkCmdDrawMultiEXT-None-04115";
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
        dynamic_coverage_reduction_mode_07647    = "VUID-vkCmdDrawMultiEXT-None-07647";
        dynamic_representative_fragment_test_enable_07648  = "VUID-vkCmdDrawMultiEXT-None-07648";
        dynamic_shading_rate_image_enable_07649  = "VUID-vkCmdDrawMultiEXT-None-07649";
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
    }
};

struct DispatchVuidsCmdDrawIndexed : DrawDispatchVuid {
    DispatchVuidsCmdDrawIndexed() : DrawDispatchVuid() {
        pipeline_bound_02700                     = "VUID-vkCmdDrawIndexed-None-02700";
        index_binding_07312                      = "VUID-vkCmdDrawIndexed-None-07312";
        vertex_binding_04007                     = "VUID-vkCmdDrawIndexed-None-04007";
        vertex_binding_null_04008                = "VUID-vkCmdDrawIndexed-None-04008";
        compatible_pipeline_02697                = "VUID-vkCmdDrawIndexed-None-02697";
        render_pass_compatible_02684             = "VUID-vkCmdDrawIndexed-renderPass-02684";
        subpass_index_02685                      = "VUID-vkCmdDrawIndexed-subpass-02685";
        sample_location_02689                    = "VUID-vkCmdDrawIndexed-sampleLocationsEnable-02689";
        linear_filter_sampler_04553              = "VUID-vkCmdDrawIndexed-magFilter-04553";
        linear_mipmap_sampler_04770              = "VUID-vkCmdDrawIndexed-mipmapMode-04770";
        cubic_sampler_02692                      = "VUID-vkCmdDrawIndexed-None-02692";
        viewport_count_03417                     = "VUID-vkCmdDrawIndexed-viewportCount-03417";
        scissor_count_03418                      = "VUID-vkCmdDrawIndexed-scissorCount-03418";
        viewport_scissor_count_03419             = "VUID-vkCmdDrawIndexed-viewportCount-03419";
        primitive_topology_class_03420           = "VUID-vkCmdDrawIndexed-primitiveTopology-03420";
        primitive_topology_class_ds3_07500       = "VUID-vkCmdDrawIndexed-dynamicPrimitiveTopologyUnrestricted-07500";
        corner_sampled_address_mode_02696        = "VUID-vkCmdDrawIndexed-flags-02696";
        imageview_atomic_02691                   = "VUID-vkCmdDrawIndexed-None-02691";
        bufferview_atomic_07888                  = "VUID-vkCmdDrawIndexed-None-07888";
        push_constants_set_06425                 = "VUID-vkCmdDrawIndexed-maintenance4-06425";
        image_subresources_render_pass_write_06537 = "VUID-vkCmdDrawIndexed-None-06537";
        image_subresources_subpass_read_06538    = "VUID-vkCmdDrawIndexed-None-06538";
        image_subresources_subpass_write_06539   = "VUID-vkCmdDrawIndexed-None-06539";
        descriptor_valid_02699                   = "VUID-vkCmdDrawIndexed-None-02699";
        sampler_imageview_type_02702             = "VUID-vkCmdDrawIndexed-None-02702";
        sampler_implicitLod_dref_proj_02703      = "VUID-vkCmdDrawIndexed-None-02703";
        sampler_bias_offset_02704                = "VUID-vkCmdDrawIndexed-None-02704";
        vertex_binding_attribute_02721           = "VUID-vkCmdDrawIndexed-None-02721";
        dynamic_state_setting_commands_02859     = "VUID-vkCmdDrawIndexed-None-02859";
        rasterization_samples_04740              = "VUID-vkCmdDrawIndexed-rasterizationSamples-04740";
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
        vertex_input_binding_stride_04884        = "VUID-vkCmdDrawIndexed-pStrides-04884";
        vertex_input_04914                       = "VUID-vkCmdDrawIndexed-None-04914";
        blend_enable_04727                       = "VUID-vkCmdDrawIndexed-blendEnable-04727";
        dynamic_discard_rectangle_07751          = "VUID-vkCmdDrawIndexed-None-07751";
        dynamic_discard_rectangle_enable_07880   = "VUID-vkCmdDrawIndexed-None-07880";
        dynamic_discard_rectangle_mode_07881     =  "VUID-vkCmdDrawIndexed-None-07881";
        dynamic_exclusive_scissor_07878          = "VUID-vkCmdDrawIndexed-None-07878";
        dynamic_exclusive_scissor_enable_07879   = "VUID-vkCmdDrawIndexed-None-07879";
        dynamic_color_write_enable_07749         = "VUID-vkCmdDrawIndexed-None-07749";
        dynamic_color_write_enable_count_07750   = "VUID-vkCmdDrawIndexed-attachmentCount-07750";
        dynamic_attachment_feedback_loop_08877   = "VUID-vkCmdDrawIndexed-None-08877";
        dynamic_rendering_view_mask_06178        = "VUID-vkCmdDrawIndexed-viewMask-06178";
        dynamic_rendering_color_count_06179      = "VUID-vkCmdDrawIndexed-colorAttachmentCount-06179";
        dynamic_rendering_color_formats_06180    = "VUID-vkCmdDrawIndexed-colorAttachmentCount-06180";
        dynamic_rendering_unused_attachments_08911 = "VUID-vkCmdDrawIndexed-dynamicRenderingUnusedAttachments-08911";
        dynamic_rendering_undefined_color_formats_07616 = "VUID-vkCmdDrawIndexed-colorAttachmentCount-07616";
        dynamic_rendering_undefined_depth_format_07617 = "VUID-vkCmdDrawIndexed-pDepthAttachment-07617";
        dynamic_rendering_undefined_stencil_format_07618 = "VUID-vkCmdDrawIndexed-pStencilAttachment-07618";
        dynamic_rendering_depth_format_06181     = "VUID-vkCmdDrawIndexed-pDepthAttachment-06181";
        dynamic_rendering_unused_attachments_08915 = "VUID-vkCmdDrawIndexed-dynamicRenderingUnusedAttachments-08915";
        dynamic_rendering_stencil_format_06182   = "VUID-vkCmdDrawIndexed-pStencilAttachment-06182";
        dynamic_rendering_unused_attachments_08918 = "VUID-vkCmdDrawIndexed-dynamicRenderingUnusedAttachments-08918";
        dynamic_rendering_fsr_06183              = "VUID-vkCmdDrawIndexed-imageView-06183";
        dynamic_rendering_fdm_06184              = "VUID-vkCmdDrawIndexed-imageView-06184";
        dynamic_rendering_color_sample_06185     = "VUID-vkCmdDrawIndexed-colorAttachmentCount-06185";
        dynamic_rendering_depth_sample_06186     = "VUID-vkCmdDrawIndexed-pDepthAttachment-06186";
        dynamic_rendering_stencil_sample_06187   = "VUID-vkCmdDrawIndexed-pStencilAttachment-06187";
        dynamic_rendering_multi_sample_06188     = "VUID-vkCmdDrawIndexed-colorAttachmentCount-06188";
        dynamic_rendering_06189                  = "VUID-vkCmdDrawIndexed-pDepthAttachment-06189";
        dynamic_rendering_06190                  = "VUID-vkCmdDrawIndexed-pStencilAttachment-06190";
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
        storage_image_write_texel_count_04115           = "VUID-vkCmdDrawIndexed-None-04115";
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
        dynamic_coverage_reduction_mode_07647    = "VUID-vkCmdDrawIndexed-None-07647";
        dynamic_representative_fragment_test_enable_07648  = "VUID-vkCmdDrawIndexed-None-07648";
        dynamic_shading_rate_image_enable_07649  = "VUID-vkCmdDrawIndexed-None-07649";
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
    }
};

struct DispatchVuidsCmdDrawMultiIndexedEXT : DrawDispatchVuid {
    DispatchVuidsCmdDrawMultiIndexedEXT() : DrawDispatchVuid() {
        pipeline_bound_02700                     = "VUID-vkCmdDrawMultiIndexedEXT-None-02700";
        index_binding_07312                      = "VUID-vkCmdDrawMultiIndexedEXT-None-07312";
        vertex_binding_04007                     = "VUID-vkCmdDrawMultiIndexedEXT-None-04007";
        vertex_binding_null_04008                = "VUID-vkCmdDrawMultiIndexedEXT-None-04008";
        compatible_pipeline_02697                = "VUID-vkCmdDrawMultiIndexedEXT-None-02697";
        render_pass_compatible_02684             = "VUID-vkCmdDrawMultiIndexedEXT-renderPass-02684";
        subpass_index_02685                      = "VUID-vkCmdDrawMultiIndexedEXT-subpass-02685";
        sample_location_02689                    = "VUID-vkCmdDrawMultiIndexedEXT-sampleLocationsEnable-02689";
        linear_filter_sampler_04553              = "VUID-vkCmdDrawMultiIndexedEXT-magFilter-04553";
        linear_mipmap_sampler_04770              = "VUID-vkCmdDrawMultiIndexedEXT-mipmapMode-04770";
        cubic_sampler_02692                      = "VUID-vkCmdDrawMultiIndexedEXT-None-02692";
        viewport_count_03417                     = "VUID-vkCmdDrawMultiIndexedEXT-viewportCount-03417";
        scissor_count_03418                      = "VUID-vkCmdDrawMultiIndexedEXT-scissorCount-03418";
        viewport_scissor_count_03419             = "VUID-vkCmdDrawMultiIndexedEXT-viewportCount-03419";
        primitive_topology_class_03420           = "VUID-vkCmdDrawMultiIndexedEXT-primitiveTopology-03420";
        primitive_topology_class_ds3_07500       = "VUID-vkCmdDrawMultiIndexedEXT-dynamicPrimitiveTopologyUnrestricted-07500";
        corner_sampled_address_mode_02696        = "VUID-vkCmdDrawMultiIndexedEXT-flags-02696";
        imageview_atomic_02691                   = "VUID-vkCmdDrawMultiIndexedEXT-None-02691";
        bufferview_atomic_07888                  = "VUID-vkCmdDrawMultiIndexedEXT-None-07888";
        push_constants_set_06425                 = "VUID-vkCmdDrawMultiIndexedEXT-maintenance4-06425";
        image_subresources_render_pass_write_06537 = "VUID-vkCmdDrawMultiIndexedEXT-None-06537";
        image_subresources_subpass_read_06538    = "VUID-vkCmdDrawMultiIndexedEXT-None-06538";
        image_subresources_subpass_write_06539   = "VUID-vkCmdDrawMultiIndexedEXT-None-06539";
        descriptor_valid_02699                   = "VUID-vkCmdDrawMultiIndexedEXT-None-02699";
        sampler_imageview_type_02702             = "VUID-vkCmdDrawMultiIndexedEXT-None-02702";
        sampler_implicitLod_dref_proj_02703      = "VUID-vkCmdDrawMultiIndexedEXT-None-02703";
        sampler_bias_offset_02704                = "VUID-vkCmdDrawMultiIndexedEXT-None-02704";
        vertex_binding_attribute_02721           = "VUID-vkCmdDrawMultiIndexedEXT-None-02721";
        dynamic_state_setting_commands_02859     = "VUID-vkCmdDrawMultiIndexedEXT-None-02859";
        rasterization_samples_04740              = "VUID-vkCmdDrawMultiIndexedEXT-rasterizationSamples-04740";
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
        vertex_input_binding_stride_04884        = "VUID-vkCmdDrawMultiIndexedEXT-pStrides-04884";
        vertex_input_04914                       = "VUID-vkCmdDrawMultiIndexedEXT-None-04914";
        blend_enable_04727                       = "VUID-vkCmdDrawMultiIndexedEXT-blendEnable-04727";
        dynamic_discard_rectangle_07751          = "VUID-vkCmdDrawMultiIndexedEXT-None-07751";
        dynamic_discard_rectangle_enable_07880   = "VUID-vkCmdDrawMultiIndexedEXT-None-07880";
        dynamic_discard_rectangle_mode_07881     =  "VUID-vkCmdDrawMultiIndexedEXT-None-07881";
        dynamic_exclusive_scissor_07878          = "VUID-vkCmdDrawMultiIndexedEXT-None-07878";
        dynamic_exclusive_scissor_enable_07879   = "VUID-vkCmdDrawMultiIndexedEXT-None-07879";
        dynamic_color_write_enable_07749         = "VUID-vkCmdDrawMultiIndexedEXT-None-07749";
        dynamic_color_write_enable_count_07750   = "VUID-vkCmdDrawMultiIndexedEXT-attachmentCount-07750";
        dynamic_attachment_feedback_loop_08877   = "VUID-vkCmdDrawMultiIndexedEXT-None-08877";
        dynamic_rendering_view_mask_06178        = "VUID-vkCmdDrawMultiIndexedEXT-viewMask-06178";
        dynamic_rendering_color_count_06179      = "VUID-vkCmdDrawMultiIndexedEXT-colorAttachmentCount-06179";
        dynamic_rendering_color_formats_06180    = "VUID-vkCmdDrawMultiIndexedEXT-colorAttachmentCount-06180";
        dynamic_rendering_unused_attachments_08911 = "VUID-vkCmdDrawMultiIndexedEXT-dynamicRenderingUnusedAttachments-08911";
        dynamic_rendering_undefined_color_formats_07616 = "VUID-vkCmdDrawMultiIndexedEXT-colorAttachmentCount-07616";
        dynamic_rendering_undefined_depth_format_07617 = "VUID-vkCmdDrawMultiIndexedEXT-pDepthAttachment-07617";
        dynamic_rendering_undefined_stencil_format_07618 = "VUID-vkCmdDrawMultiIndexedEXT-pStencilAttachment-07618";
        dynamic_rendering_depth_format_06181     = "VUID-vkCmdDrawMultiIndexedEXT-pDepthAttachment-06181";
        dynamic_rendering_unused_attachments_08915 = "VUID-vkCmdDrawMultiIndexedEXT-dynamicRenderingUnusedAttachments-08915";
        dynamic_rendering_stencil_format_06182   = "VUID-vkCmdDrawMultiIndexedEXT-pStencilAttachment-06182";
        dynamic_rendering_unused_attachments_08918 = "VUID-vkCmdDrawMultiIndexedEXT-dynamicRenderingUnusedAttachments-08918";
        dynamic_rendering_fsr_06183              = "VUID-vkCmdDrawMultiIndexedEXT-imageView-06183";
        dynamic_rendering_fdm_06184              = "VUID-vkCmdDrawMultiIndexedEXT-imageView-06184";
        dynamic_rendering_color_sample_06185     = "VUID-vkCmdDrawMultiIndexedEXT-colorAttachmentCount-06185";
        dynamic_rendering_depth_sample_06186     = "VUID-vkCmdDrawMultiIndexedEXT-pDepthAttachment-06186";
        dynamic_rendering_stencil_sample_06187   = "VUID-vkCmdDrawMultiIndexedEXT-pStencilAttachment-06187";
        dynamic_rendering_multi_sample_06188     = "VUID-vkCmdDrawMultiIndexedEXT-colorAttachmentCount-06188";
        dynamic_rendering_06189                  = "VUID-vkCmdDrawMultiIndexedEXT-pDepthAttachment-06189";
        dynamic_rendering_06190                  = "VUID-vkCmdDrawMultiIndexedEXT-pStencilAttachment-06190";
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
        storage_image_write_texel_count_04115           = "VUID-vkCmdDrawMultiIndexedEXT-None-04115";
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
        dynamic_coverage_reduction_mode_07647    = "VUID-vkCmdDrawMultiIndexedEXT-None-07647";
        dynamic_representative_fragment_test_enable_07648  = "VUID-vkCmdDrawMultiIndexedEXT-None-07648";
        dynamic_shading_rate_image_enable_07649  = "VUID-vkCmdDrawMultiIndexedEXT-None-07649";
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
    }
};

struct DispatchVuidsCmdDrawIndirect : DrawDispatchVuid {
    DispatchVuidsCmdDrawIndirect() : DrawDispatchVuid() {
        pipeline_bound_02700                     = "VUID-vkCmdDrawIndirect-None-02700";
        vertex_binding_04007                     = "VUID-vkCmdDrawIndirect-None-04007";
        vertex_binding_null_04008                = "VUID-vkCmdDrawIndirect-None-04008";
        compatible_pipeline_02697                = "VUID-vkCmdDrawIndirect-None-02697";
        render_pass_compatible_02684             = "VUID-vkCmdDrawIndirect-renderPass-02684";
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
        primitive_topology_class_03420           = "VUID-vkCmdDrawIndirect-primitiveTopology-03420";
        primitive_topology_class_ds3_07500       = "VUID-vkCmdDrawIndirect-dynamicPrimitiveTopologyUnrestricted-07500";
        corner_sampled_address_mode_02696        = "VUID-vkCmdDrawIndirect-flags-02696";
        imageview_atomic_02691                   = "VUID-vkCmdDrawIndirect-None-02691";
        bufferview_atomic_07888                  = "VUID-vkCmdDrawIndirect-None-07888";
        push_constants_set_06425                 = "VUID-vkCmdDrawIndirect-maintenance4-06425";
        image_subresources_render_pass_write_06537 = "VUID-vkCmdDrawIndirect-None-06537";
        image_subresources_subpass_read_06538    = "VUID-vkCmdDrawIndirect-None-06538";
        image_subresources_subpass_write_06539   = "VUID-vkCmdDrawIndirect-None-06539";
        descriptor_valid_02699                   = "VUID-vkCmdDrawIndirect-None-02699";
        sampler_imageview_type_02702             = "VUID-vkCmdDrawIndirect-None-02702";
        sampler_implicitLod_dref_proj_02703      = "VUID-vkCmdDrawIndirect-None-02703";
        sampler_bias_offset_02704                = "VUID-vkCmdDrawIndirect-None-02704";
        vertex_binding_attribute_02721           = "VUID-vkCmdDrawIndirect-None-02721";
        dynamic_state_setting_commands_02859     = "VUID-vkCmdDrawIndirect-None-02859";
        rasterization_samples_04740              = "VUID-vkCmdDrawIndirect-rasterizationSamples-04740";
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
        vertex_input_binding_stride_04884        = "VUID-vkCmdDrawIndirect-pStrides-04884";
        vertex_input_04914                       = "VUID-vkCmdDrawIndirect-None-04914";
        blend_enable_04727                       = "VUID-vkCmdDrawIndirect-blendEnable-04727";
        dynamic_discard_rectangle_07751          = "VUID-vkCmdDrawIndirect-None-07751";
        dynamic_discard_rectangle_enable_07880   = "VUID-vkCmdDrawIndirect-None-07880";
        dynamic_discard_rectangle_mode_07881     =  "VUID-vkCmdDrawIndirect-None-07881";
        dynamic_exclusive_scissor_07878          = "VUID-vkCmdDrawIndirect-None-07878";
        dynamic_exclusive_scissor_enable_07879   = "VUID-vkCmdDrawIndirect-None-07879";
        dynamic_color_write_enable_07749         = "VUID-vkCmdDrawIndirect-None-07749";
        dynamic_color_write_enable_count_07750   = "VUID-vkCmdDrawIndirect-attachmentCount-07750";
        dynamic_attachment_feedback_loop_08877   = "VUID-vkCmdDrawIndirect-None-08877";
        dynamic_rendering_view_mask_06178        = "VUID-vkCmdDrawIndirect-viewMask-06178";
        dynamic_rendering_color_count_06179      = "VUID-vkCmdDrawIndirect-colorAttachmentCount-06179";
        dynamic_rendering_color_formats_06180    = "VUID-vkCmdDrawIndirect-colorAttachmentCount-06180";
        dynamic_rendering_unused_attachments_08911 = "VUID-vkCmdDrawIndirect-dynamicRenderingUnusedAttachments-08911";
        dynamic_rendering_undefined_color_formats_07616 = "VUID-vkCmdDrawIndirect-colorAttachmentCount-07616";
        dynamic_rendering_undefined_depth_format_07617 = "VUID-vkCmdDrawIndirect-pDepthAttachment-07617";
        dynamic_rendering_undefined_stencil_format_07618 = "VUID-vkCmdDrawIndirect-pStencilAttachment-07618";
        dynamic_rendering_depth_format_06181     = "VUID-vkCmdDrawIndirect-pDepthAttachment-06181";
        dynamic_rendering_unused_attachments_08915 = "VUID-vkCmdDrawIndirect-dynamicRenderingUnusedAttachments-08915";
        dynamic_rendering_stencil_format_06182   = "VUID-vkCmdDrawIndirect-pStencilAttachment-06182";
        dynamic_rendering_unused_attachments_08918 = "VUID-vkCmdDrawIndirect-dynamicRenderingUnusedAttachments-08918";
        dynamic_rendering_fsr_06183              = "VUID-vkCmdDrawIndirect-imageView-06183";
        dynamic_rendering_fdm_06184              = "VUID-vkCmdDrawIndirect-imageView-06184";
        dynamic_rendering_color_sample_06185     = "VUID-vkCmdDrawIndirect-colorAttachmentCount-06185";
        dynamic_rendering_depth_sample_06186     = "VUID-vkCmdDrawIndirect-pDepthAttachment-06186";
        dynamic_rendering_stencil_sample_06187   = "VUID-vkCmdDrawIndirect-pStencilAttachment-06187";
        dynamic_rendering_multi_sample_06188     = "VUID-vkCmdDrawIndirect-colorAttachmentCount-06188";
        dynamic_rendering_06189                  = "VUID-vkCmdDrawIndirect-pDepthAttachment-06189";
        dynamic_rendering_06190                  = "VUID-vkCmdDrawIndirect-pStencilAttachment-06190";
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
        storage_image_write_texel_count_04115           = "VUID-vkCmdDrawIndirect-None-04115";
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
        dynamic_coverage_reduction_mode_07647    = "VUID-vkCmdDrawIndirect-None-07647";
        dynamic_representative_fragment_test_enable_07648  = "VUID-vkCmdDrawIndirect-None-07648";
        dynamic_shading_rate_image_enable_07649  = "VUID-vkCmdDrawIndirect-None-07649";
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
    }
};

struct DispatchVuidsCmdDrawIndexedIndirect : DrawDispatchVuid {
    DispatchVuidsCmdDrawIndexedIndirect() : DrawDispatchVuid() {
        pipeline_bound_02700                     = "VUID-vkCmdDrawIndexedIndirect-None-02700";
        index_binding_07312                      = "VUID-vkCmdDrawIndexedIndirect-None-07312";
        vertex_binding_04007                     = "VUID-vkCmdDrawIndexedIndirect-None-04007";
        vertex_binding_null_04008                = "VUID-vkCmdDrawIndexedIndirect-None-04008";
        compatible_pipeline_02697                = "VUID-vkCmdDrawIndexedIndirect-None-02697";
        render_pass_compatible_02684             = "VUID-vkCmdDrawIndexedIndirect-renderPass-02684";
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
        primitive_topology_class_03420           = "VUID-vkCmdDrawIndexedIndirect-primitiveTopology-03420";
        primitive_topology_class_ds3_07500       = "VUID-vkCmdDrawIndexedIndirect-dynamicPrimitiveTopologyUnrestricted-07500";
        corner_sampled_address_mode_02696        = "VUID-vkCmdDrawIndexedIndirect-flags-02696";
        imageview_atomic_02691                   = "VUID-vkCmdDrawIndexedIndirect-None-02691";
        bufferview_atomic_07888                  = "VUID-vkCmdDrawIndexedIndirect-None-07888";
        push_constants_set_06425                 = "VUID-vkCmdDrawIndexedIndirect-maintenance4-06425";
        image_subresources_render_pass_write_06537 = "VUID-vkCmdDrawIndexedIndirect-None-06537";
        image_subresources_subpass_read_06538    = "VUID-vkCmdDrawIndexedIndirect-None-06538";
        image_subresources_subpass_write_06539   = "VUID-vkCmdDrawIndexedIndirect-None-06539";
        descriptor_valid_02699                   = "VUID-vkCmdDrawIndexedIndirect-None-02699";
        sampler_imageview_type_02702             = "VUID-vkCmdDrawIndexedIndirect-None-02702";
        sampler_implicitLod_dref_proj_02703      = "VUID-vkCmdDrawIndexedIndirect-None-02703";
        sampler_bias_offset_02704                = "VUID-vkCmdDrawIndexedIndirect-None-02704";
        vertex_binding_attribute_02721           = "VUID-vkCmdDrawIndexedIndirect-None-02721";
        dynamic_state_setting_commands_02859     = "VUID-vkCmdDrawIndexedIndirect-None-02859";
        rasterization_samples_04740              = "VUID-vkCmdDrawIndexedIndirect-rasterizationSamples-04740";
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
        vertex_input_binding_stride_04884        = "VUID-vkCmdDrawIndexedIndirect-pStrides-04884";
        vertex_input_04914                       = "VUID-vkCmdDrawIndexedIndirect-None-04914";
        blend_enable_04727                       = "VUID-vkCmdDrawIndexedIndirect-blendEnable-04727";
        dynamic_discard_rectangle_07751          = "VUID-vkCmdDrawIndexedIndirect-None-07751";
        dynamic_discard_rectangle_enable_07880   = "VUID-vkCmdDrawIndexedIndirect-None-07880";
        dynamic_discard_rectangle_mode_07881     =  "VUID-vkCmdDrawIndexedIndirect-None-07881";
        dynamic_exclusive_scissor_07878          = "VUID-vkCmdDrawIndexedIndirect-None-07878";
        dynamic_exclusive_scissor_enable_07879   = "VUID-vkCmdDrawIndexedIndirect-None-07879";
        dynamic_color_write_enable_07749         = "VUID-vkCmdDrawIndexedIndirect-None-07749";
        dynamic_color_write_enable_count_07750   = "VUID-vkCmdDrawIndexedIndirect-attachmentCount-07750";
        dynamic_attachment_feedback_loop_08877   = "VUID-vkCmdDrawIndexedIndirect-None-08877";
        dynamic_rendering_view_mask_06178        = "VUID-vkCmdDrawIndexedIndirect-viewMask-06178";
        dynamic_rendering_color_count_06179      = "VUID-vkCmdDrawIndexedIndirect-colorAttachmentCount-06179";
        dynamic_rendering_color_formats_06180    = "VUID-vkCmdDrawIndexedIndirect-colorAttachmentCount-06180";
        dynamic_rendering_unused_attachments_08911 = "VUID-vkCmdDrawIndexedIndirect-dynamicRenderingUnusedAttachments-08911";
        dynamic_rendering_undefined_color_formats_07616 = "VUID-vkCmdDrawIndexedIndirect-colorAttachmentCount-07616";
        dynamic_rendering_undefined_depth_format_07617 = "VUID-vkCmdDrawIndexedIndirect-pDepthAttachment-07617";
        dynamic_rendering_undefined_stencil_format_07618 = "VUID-vkCmdDrawIndexedIndirect-pStencilAttachment-07618";
        dynamic_rendering_depth_format_06181     = "VUID-vkCmdDrawIndexedIndirect-pDepthAttachment-06181";
        dynamic_rendering_unused_attachments_08915 = "VUID-vkCmdDrawIndexedIndirect-dynamicRenderingUnusedAttachments-08915";
        dynamic_rendering_stencil_format_06182   = "VUID-vkCmdDrawIndexedIndirect-pStencilAttachment-06182";
        dynamic_rendering_unused_attachments_08918 = "VUID-vkCmdDrawIndexedIndirect-dynamicRenderingUnusedAttachments-08918";
        dynamic_rendering_fsr_06183              = "VUID-vkCmdDrawIndexedIndirect-imageView-06183";
        dynamic_rendering_fdm_06184              = "VUID-vkCmdDrawIndexedIndirect-imageView-06184";
        dynamic_rendering_color_sample_06185     = "VUID-vkCmdDrawIndexedIndirect-colorAttachmentCount-06185";
        dynamic_rendering_depth_sample_06186     = "VUID-vkCmdDrawIndexedIndirect-pDepthAttachment-06186";
        dynamic_rendering_stencil_sample_06187   = "VUID-vkCmdDrawIndexedIndirect-pStencilAttachment-06187";
        dynamic_rendering_multi_sample_06188     = "VUID-vkCmdDrawIndexedIndirect-colorAttachmentCount-06188";
        dynamic_rendering_06189                  = "VUID-vkCmdDrawIndexedIndirect-pDepthAttachment-06189";
        dynamic_rendering_06190                  = "VUID-vkCmdDrawIndexedIndirect-pStencilAttachment-06190";
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
        storage_image_write_texel_count_04115           = "VUID-vkCmdDrawIndexedIndirect-None-04115";
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
        dynamic_coverage_reduction_mode_07647    = "VUID-vkCmdDrawIndexedIndirect-None-07647";
        dynamic_representative_fragment_test_enable_07648  = "VUID-vkCmdDrawIndexedIndirect-None-07648";
        dynamic_shading_rate_image_enable_07649  = "VUID-vkCmdDrawIndexedIndirect-None-07649";
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
    }
};

struct DispatchVuidsCmdDispatch : DrawDispatchVuid {
    DispatchVuidsCmdDispatch() : DrawDispatchVuid() {
        pipeline_bound_02700                     = "VUID-vkCmdDispatch-None-02700";
        compatible_pipeline_02697                = "VUID-vkCmdDispatch-None-02697";
        linear_filter_sampler_04553              = "VUID-vkCmdDispatch-magFilter-04553";
        linear_mipmap_sampler_04770              = "VUID-vkCmdDispatch-mipmapMode-04770";
        cubic_sampler_02692                      = "VUID-vkCmdDispatch-None-02692";
        corner_sampled_address_mode_02696        = "VUID-vkCmdDispatch-flags-02696";
        imageview_atomic_02691                   = "VUID-vkCmdDispatch-None-02691";
        bufferview_atomic_07888                  = "VUID-vkCmdDispatch-None-07888";
        push_constants_set_06425                 = "VUID-vkCmdDispatch-maintenance4-06425";
        descriptor_valid_02699                   = "VUID-vkCmdDispatch-None-02699";
        sampler_imageview_type_02702             = "VUID-vkCmdDispatch-None-02702";
        sampler_implicitLod_dref_proj_02703      = "VUID-vkCmdDispatch-None-02703";
        sampler_bias_offset_02704                = "VUID-vkCmdDispatch-None-02704";
        dynamic_state_setting_commands_02859     = "VUID-vkCmdDispatch-None-02859";
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
        storage_image_write_texel_count_04115           = "VUID-vkCmdDispatch-None-04115";
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
        pipeline_bound_02700                     = "VUID-vkCmdDispatchIndirect-None-02700";
        compatible_pipeline_02697                = "VUID-vkCmdDispatchIndirect-None-02697";
        linear_filter_sampler_04553              = "VUID-vkCmdDispatchIndirect-magFilter-04553";
        linear_mipmap_sampler_04770              = "VUID-vkCmdDispatchIndirect-mipmapMode-04770";
        cubic_sampler_02692                      = "VUID-vkCmdDispatchIndirect-None-02692";
        indirect_protected_cb_02646              = "VUID-vkCmdDispatchIndirect-commandBuffer-02711";
        indirect_contiguous_memory_02708         = "VUID-vkCmdDispatchIndirect-buffer-02708";
        indirect_buffer_bit_02290                = "VUID-vkCmdDispatchIndirect-buffer-02709";
        corner_sampled_address_mode_02696        = "VUID-vkCmdDispatchIndirect-flags-02696";
        imageview_atomic_02691                   = "VUID-vkCmdDispatchIndirect-None-02691";
        bufferview_atomic_07888                  = "VUID-vkCmdDispatchIndirect-None-07888";
        push_constants_set_06425                 = "VUID-vkCmdDispatchIndirect-maintenance4-06425";
        descriptor_valid_02699                   = "VUID-vkCmdDispatchIndirect-None-02699";
        sampler_imageview_type_02702             = "VUID-vkCmdDispatchIndirect-None-02702";
        sampler_implicitLod_dref_proj_02703      = "VUID-vkCmdDispatchIndirect-None-02703";
        sampler_bias_offset_02704                = "VUID-vkCmdDispatchIndirect-None-02704";
        dynamic_state_setting_commands_02859     = "VUID-vkCmdDispatchIndirect-None-02859";
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
        storage_image_write_texel_count_04115           = "VUID-vkCmdDispatchIndirect-None-04115";
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
        pipeline_bound_02700                     = "VUID-vkCmdDrawIndirectCount-None-02700";
        vertex_binding_04007                     = "VUID-vkCmdDrawIndirectCount-None-04007";
        vertex_binding_null_04008                = "VUID-vkCmdDrawIndirectCount-None-04008";
        compatible_pipeline_02697                = "VUID-vkCmdDrawIndirectCount-None-02697";
        render_pass_compatible_02684             = "VUID-vkCmdDrawIndirectCount-renderPass-02684";
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
        primitive_topology_class_03420           = "VUID-vkCmdDrawIndirectCount-primitiveTopology-03420";
        primitive_topology_class_ds3_07500       = "VUID-vkCmdDrawIndirectCount-dynamicPrimitiveTopologyUnrestricted-07500";
        corner_sampled_address_mode_02696        = "VUID-vkCmdDrawIndirectCount-flags-02696";
        imageview_atomic_02691                   = "VUID-vkCmdDrawIndirectCount-None-02691";
        bufferview_atomic_07888                  = "VUID-vkCmdDrawIndirectCount-None-07888";
        push_constants_set_06425                 = "VUID-vkCmdDrawIndirectCount-maintenance4-06425";
        image_subresources_render_pass_write_06537 = "VUID-vkCmdDrawIndirectCount-None-06537";
        image_subresources_subpass_read_06538    = "VUID-vkCmdDrawIndirectCount-None-06538";
        image_subresources_subpass_write_06539   = "VUID-vkCmdDrawIndirectCount-None-06539";
        descriptor_valid_02699                   = "VUID-vkCmdDrawIndirectCount-None-02699";
        sampler_imageview_type_02702             = "VUID-vkCmdDrawIndirectCount-None-02702";
        sampler_implicitLod_dref_proj_02703      = "VUID-vkCmdDrawIndirectCount-None-02703";
        sampler_bias_offset_02704                = "VUID-vkCmdDrawIndirectCount-None-02704";
        vertex_binding_attribute_02721           = "VUID-vkCmdDrawIndirectCount-None-02721";
        dynamic_state_setting_commands_02859     = "VUID-vkCmdDrawIndirectCount-None-02859";
        rasterization_samples_04740              = "VUID-vkCmdDrawIndirectCount-rasterizationSamples-04740";
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
        vertex_input_binding_stride_04884        = "VUID-vkCmdDrawIndirectCount-pStrides-04884";
        vertex_input_04914                       = "VUID-vkCmdDrawIndirectCount-None-04914";
        blend_enable_04727                       = "VUID-vkCmdDrawIndirectCount-blendEnable-04727";
        dynamic_discard_rectangle_07751          = "VUID-vkCmdDrawIndirectCount-None-07751";
        dynamic_discard_rectangle_enable_07880   = "VUID-vkCmdDrawIndirectCount-None-07880";
        dynamic_discard_rectangle_mode_07881     =  "VUID-vkCmdDrawIndirectCount-None-07881";
        dynamic_exclusive_scissor_07878          = "VUID-vkCmdDrawIndirectCount-None-07878";
        dynamic_exclusive_scissor_enable_07879   = "VUID-vkCmdDrawIndirectCount-None-07879";
        dynamic_color_write_enable_07749         = "VUID-vkCmdDrawIndirectCount-None-07749";
        dynamic_color_write_enable_count_07750   = "VUID-vkCmdDrawIndirectCount-attachmentCount-07750";
        dynamic_attachment_feedback_loop_08877   = "VUID-vkCmdDrawIndirectCount-None-08877";
        dynamic_rendering_view_mask_06178        = "VUID-vkCmdDrawIndirectCount-viewMask-06178";
        dynamic_rendering_color_count_06179      = "VUID-vkCmdDrawIndirectCount-colorAttachmentCount-06179";
        dynamic_rendering_color_formats_06180    = "VUID-vkCmdDrawIndirectCount-colorAttachmentCount-06180";
        dynamic_rendering_unused_attachments_08911 = "VUID-vkCmdDrawIndirectCount-dynamicRenderingUnusedAttachments-08911";
        dynamic_rendering_undefined_color_formats_07616 = "VUID-vkCmdDrawIndirectCount-colorAttachmentCount-07616";
        dynamic_rendering_undefined_depth_format_07617 = "VUID-vkCmdDrawIndirectCount-pDepthAttachment-07617";
        dynamic_rendering_undefined_stencil_format_07618 = "VUID-vkCmdDrawIndirectCount-pStencilAttachment-07618";
        dynamic_rendering_depth_format_06181     = "VUID-vkCmdDrawIndirectCount-pDepthAttachment-06181";
        dynamic_rendering_unused_attachments_08915 = "VUID-vkCmdDrawIndirectCount-dynamicRenderingUnusedAttachments-08915";
        dynamic_rendering_stencil_format_06182   = "VUID-vkCmdDrawIndirectCount-pStencilAttachment-06182";
        dynamic_rendering_unused_attachments_08918 = "VUID-vkCmdDrawIndirectCount-dynamicRenderingUnusedAttachments-08918";
        dynamic_rendering_fsr_06183              = "VUID-vkCmdDrawIndirectCount-imageView-06183";
        dynamic_rendering_fdm_06184              = "VUID-vkCmdDrawIndirectCount-imageView-06184";
        dynamic_rendering_color_sample_06185     = "VUID-vkCmdDrawIndirectCount-colorAttachmentCount-06185";
        dynamic_rendering_depth_sample_06186     = "VUID-vkCmdDrawIndirectCount-pDepthAttachment-06186";
        dynamic_rendering_stencil_sample_06187   = "VUID-vkCmdDrawIndirectCount-pStencilAttachment-06187";
        dynamic_rendering_multi_sample_06188     = "VUID-vkCmdDrawIndirectCount-colorAttachmentCount-06188";
        dynamic_rendering_06189                  = "VUID-vkCmdDrawIndirectCount-pDepthAttachment-06189";
        dynamic_rendering_06190                  = "VUID-vkCmdDrawIndirectCount-pStencilAttachment-06190";
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
        storage_image_write_texel_count_04115           = "VUID-vkCmdDrawIndirectCount-None-04115";
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
        dynamic_coverage_reduction_mode_07647    = "VUID-vkCmdDrawIndirectCount-None-07647";
        dynamic_representative_fragment_test_enable_07648  = "VUID-vkCmdDrawIndirectCount-None-07648";
        dynamic_shading_rate_image_enable_07649  = "VUID-vkCmdDrawIndirectCount-None-07649";
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
    }
};

struct DispatchVuidsCmdDrawIndexedIndirectCount : DrawDispatchVuid {
    DispatchVuidsCmdDrawIndexedIndirectCount() : DrawDispatchVuid() {
        pipeline_bound_02700                     = "VUID-vkCmdDrawIndexedIndirectCount-None-02700";
        index_binding_07312                      = "VUID-vkCmdDrawIndexedIndirectCount-None-07312";
        vertex_binding_04007                     = "VUID-vkCmdDrawIndexedIndirectCount-None-04007";
        vertex_binding_null_04008                = "VUID-vkCmdDrawIndexedIndirectCount-None-04008";
        compatible_pipeline_02697                = "VUID-vkCmdDrawIndexedIndirectCount-None-02697";
        render_pass_compatible_02684             = "VUID-vkCmdDrawIndexedIndirectCount-renderPass-02684";
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
        primitive_topology_class_03420           = "VUID-vkCmdDrawIndexedIndirectCount-primitiveTopology-03420";
        primitive_topology_class_ds3_07500       = "VUID-vkCmdDrawIndexedIndirectCount-dynamicPrimitiveTopologyUnrestricted-07500";
        corner_sampled_address_mode_02696        = "VUID-vkCmdDrawIndexedIndirectCount-flags-02696";
        imageview_atomic_02691                   = "VUID-vkCmdDrawIndexedIndirectCount-None-02691";
        bufferview_atomic_07888                  = "VUID-vkCmdDrawIndexedIndirectCount-None-07888";
        push_constants_set_06425                 = "VUID-vkCmdDrawIndexedIndirectCount-maintenance4-06425";
        image_subresources_render_pass_write_06537 = "VUID-vkCmdDrawIndexedIndirectCount-None-06537";
        image_subresources_subpass_read_06538    = "VUID-vkCmdDrawIndexedIndirectCount-None-06538";
        image_subresources_subpass_write_06539   = "VUID-vkCmdDrawIndexedIndirectCount-None-06539";
        descriptor_valid_02699                   = "VUID-vkCmdDrawIndexedIndirectCount-None-02699";
        sampler_imageview_type_02702             = "VUID-vkCmdDrawIndexedIndirectCount-None-02702";
        sampler_implicitLod_dref_proj_02703      = "VUID-vkCmdDrawIndexedIndirectCount-None-02703";
        sampler_bias_offset_02704                = "VUID-vkCmdDrawIndexedIndirectCount-None-02704";
        vertex_binding_attribute_02721           = "VUID-vkCmdDrawIndexedIndirectCount-None-02721";
        dynamic_state_setting_commands_02859     = "VUID-vkCmdDrawIndexedIndirectCount-None-02859";
        rasterization_samples_04740              = "VUID-vkCmdDrawIndexedIndirectCount-rasterizationSamples-04740";
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
        vertex_input_binding_stride_04884        = "VUID-vkCmdDrawIndexedIndirectCount-pStrides-04884";
        vertex_input_04914                       = "VUID-vkCmdDrawIndexedIndirectCount-None-04914";
        blend_enable_04727                       = "VUID-vkCmdDrawIndexedIndirectCount-blendEnable-04727";
        dynamic_discard_rectangle_07751          = "VUID-vkCmdDrawIndexedIndirectCount-None-07751";
        dynamic_discard_rectangle_enable_07880   = "VUID-vkCmdDrawIndexedIndirectCount-None-07880";
        dynamic_discard_rectangle_mode_07881     =  "VUID-vkCmdDrawIndexedIndirectCount-None-07881";
        dynamic_exclusive_scissor_07878          = "VUID-vkCmdDrawIndexedIndirectCount-None-07878";
        dynamic_exclusive_scissor_enable_07879   = "VUID-vkCmdDrawIndexedIndirectCount-None-07879";
        dynamic_color_write_enable_07749         = "VUID-vkCmdDrawIndexedIndirectCount-None-07749";
        dynamic_color_write_enable_count_07750   = "VUID-vkCmdDrawIndexedIndirectCount-attachmentCount-07750";
        dynamic_attachment_feedback_loop_08877   = "VUID-vkCmdDrawIndexedIndirectCount-None-08877";
        dynamic_rendering_view_mask_06178        = "VUID-vkCmdDrawIndexedIndirectCount-viewMask-06178";
        dynamic_rendering_color_count_06179      = "VUID-vkCmdDrawIndexedIndirectCount-colorAttachmentCount-06179";
        dynamic_rendering_color_formats_06180    = "VUID-vkCmdDrawIndexedIndirectCount-colorAttachmentCount-06180";
        dynamic_rendering_unused_attachments_08911 = "VUID-vkCmdDrawIndexedIndirectCount-dynamicRenderingUnusedAttachments-08911";
        dynamic_rendering_undefined_color_formats_07616 = "VUID-vkCmdDrawIndexedIndirectCount-colorAttachmentCount-07616";
        dynamic_rendering_undefined_depth_format_07617 = "VUID-vkCmdDrawIndexedIndirectCount-pDepthAttachment-07617";
        dynamic_rendering_undefined_stencil_format_07618 = "VUID-vkCmdDrawIndexedIndirectCount-pStencilAttachment-07618";
        dynamic_rendering_depth_format_06181     = "VUID-vkCmdDrawIndexedIndirectCount-pDepthAttachment-06181";
        dynamic_rendering_unused_attachments_08915 = "VUID-vkCmdDrawIndexedIndirectCount-dynamicRenderingUnusedAttachments-08915";
        dynamic_rendering_stencil_format_06182   = "VUID-vkCmdDrawIndexedIndirectCount-pStencilAttachment-06182";
        dynamic_rendering_unused_attachments_08918 = "VUID-vkCmdDrawIndexedIndirectCount-dynamicRenderingUnusedAttachments-08918";
        dynamic_rendering_fsr_06183              = "VUID-vkCmdDrawIndexedIndirectCount-imageView-06183";
        dynamic_rendering_fdm_06184              = "VUID-vkCmdDrawIndexedIndirectCount-imageView-06184";
        dynamic_rendering_color_sample_06185     = "VUID-vkCmdDrawIndexedIndirectCount-colorAttachmentCount-06185";
        dynamic_rendering_depth_sample_06186     = "VUID-vkCmdDrawIndexedIndirectCount-pDepthAttachment-06186";
        dynamic_rendering_stencil_sample_06187   = "VUID-vkCmdDrawIndexedIndirectCount-pStencilAttachment-06187";
        dynamic_rendering_multi_sample_06188     = "VUID-vkCmdDrawIndexedIndirectCount-colorAttachmentCount-06188";
        dynamic_rendering_06189                  = "VUID-vkCmdDrawIndexedIndirectCount-pDepthAttachment-06189";
        dynamic_rendering_06190                  = "VUID-vkCmdDrawIndexedIndirectCount-pStencilAttachment-06190";
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
        storage_image_write_texel_count_04115           = "VUID-vkCmdDrawIndexedIndirectCount-None-04115";
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
        dynamic_coverage_reduction_mode_07647    = "VUID-vkCmdDrawIndexedIndirectCount-None-07647";
        dynamic_representative_fragment_test_enable_07648  = "VUID-vkCmdDrawIndexedIndirectCount-None-07648";
        dynamic_shading_rate_image_enable_07649  = "VUID-vkCmdDrawIndexedIndirectCount-None-07649";
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
    }
};

struct DispatchVuidsCmdTraceRaysNV: DrawDispatchVuid {
    DispatchVuidsCmdTraceRaysNV() : DrawDispatchVuid() {
        pipeline_bound_02700                     = "VUID-vkCmdTraceRaysNV-None-02700";
        compatible_pipeline_02697                = "VUID-vkCmdTraceRaysNV-None-02697";
        linear_filter_sampler_04553              = "VUID-vkCmdTraceRaysNV-magFilter-04553";
        linear_mipmap_sampler_04770              = "VUID-vkCmdTraceRaysNV-mipmapMode-04770";
        cubic_sampler_02692                      = "VUID-vkCmdTraceRaysNV-None-02692";
        corner_sampled_address_mode_02696        = "VUID-vkCmdTraceRaysNV-flags-02696";
        imageview_atomic_02691                   = "VUID-vkCmdTraceRaysNV-None-02691";
        bufferview_atomic_07888                  = "VUID-vkCmdTraceRaysNV-None-07888";
        push_constants_set_06425                 = "VUID-vkCmdTraceRaysNV-maintenance4-06425";
        descriptor_valid_02699                   = "VUID-vkCmdTraceRaysNV-None-02699";
        sampler_imageview_type_02702             = "VUID-vkCmdTraceRaysNV-None-02702";
        sampler_implicitLod_dref_proj_02703      = "VUID-vkCmdTraceRaysNV-None-02703";
        sampler_bias_offset_02704                = "VUID-vkCmdTraceRaysNV-None-02704";
        dynamic_state_setting_commands_02859     = "VUID-vkCmdTraceRaysNV-None-02859";
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
        storage_image_write_texel_count_04115           = "VUID-vkCmdTraceRaysNV-None-04115";
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
        pipeline_bound_02700                     = "VUID-vkCmdTraceRaysKHR-None-02700";
        compatible_pipeline_02697                = "VUID-vkCmdTraceRaysKHR-None-02697";
        linear_filter_sampler_04553              = "VUID-vkCmdTraceRaysKHR-magFilter-04553";
        linear_mipmap_sampler_04770              = "VUID-vkCmdTraceRaysKHR-mipmapMode-04770";
        cubic_sampler_02692                      = "VUID-vkCmdTraceRaysKHR-None-02692";
        corner_sampled_address_mode_02696        = "VUID-vkCmdTraceRaysKHR-flags-02696";
        imageview_atomic_02691                   = "VUID-vkCmdTraceRaysKHR-None-02691";
        bufferview_atomic_07888                  = "VUID-vkCmdTraceRaysKHR-None-07888";
        push_constants_set_06425                 = "VUID-vkCmdTraceRaysKHR-maintenance4-06425";
        descriptor_valid_02699                   = "VUID-vkCmdTraceRaysKHR-None-02699";
        sampler_imageview_type_02702             = "VUID-vkCmdTraceRaysKHR-None-02702";
        sampler_implicitLod_dref_proj_02703      = "VUID-vkCmdTraceRaysKHR-None-02703";
        sampler_bias_offset_02704                = "VUID-vkCmdTraceRaysKHR-None-02704";
        dynamic_state_setting_commands_02859     = "VUID-vkCmdTraceRaysKHR-None-02859";
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
        storage_image_write_texel_count_04115           = "VUID-vkCmdTraceRaysKHR-None-04115";
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
        pipeline_bound_02700                     = "VUID-vkCmdTraceRaysIndirectKHR-None-02700";
        compatible_pipeline_02697                = "VUID-vkCmdTraceRaysIndirectKHR-None-02697";
        linear_filter_sampler_04553              = "VUID-vkCmdTraceRaysIndirectKHR-magFilter-04553";
        linear_mipmap_sampler_04770              = "VUID-vkCmdTraceRaysIndirectKHR-mipmapMode-04770";
        cubic_sampler_02692                      = "VUID-vkCmdTraceRaysIndirectKHR-None-02692";
        indirect_contiguous_memory_02708         = "VUID-vkCmdTraceRaysIndirectKHR-indirectDeviceAddress-03632";
        indirect_buffer_bit_02290                = "VUID-vkCmdTraceRaysIndirectKHR-indirectDeviceAddress-03633";
        corner_sampled_address_mode_02696        = "VUID-vkCmdTraceRaysIndirectKHR-flags-02696";
        imageview_atomic_02691                   = "VUID-vkCmdTraceRaysIndirectKHR-None-02691";
        bufferview_atomic_07888                  = "VUID-vkCmdTraceRaysIndirectKHR-None-07888";
        push_constants_set_06425                 = "VUID-vkCmdTraceRaysIndirectKHR-maintenance4-06425";
        descriptor_valid_02699                   = "VUID-vkCmdTraceRaysIndirectKHR-None-02699";
        sampler_imageview_type_02702             = "VUID-vkCmdTraceRaysIndirectKHR-None-02702";
        sampler_implicitLod_dref_proj_02703      = "VUID-vkCmdTraceRaysIndirectKHR-None-02703";
        sampler_bias_offset_02704                = "VUID-vkCmdTraceRaysIndirectKHR-None-02704";
        dynamic_state_setting_commands_02859     = "VUID-vkCmdTraceRaysIndirectKHR-None-02859";
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
        storage_image_write_texel_count_04115           = "VUID-vkCmdTraceRaysIndirectKHR-None-04115";
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
        pipeline_bound_02700                     = "VUID-vkCmdTraceRaysIndirect2KHR-None-02700";
        compatible_pipeline_02697                = "VUID-vkCmdTraceRaysIndirect2KHR-None-02697";
        linear_filter_sampler_04553              = "VUID-vkCmdTraceRaysIndirect2KHR-magFilter-04553";
        linear_mipmap_sampler_04770              = "VUID-vkCmdTraceRaysIndirect2KHR-mipmapMode-04770";
        cubic_sampler_02692                      = "VUID-vkCmdTraceRaysIndirect2KHR-None-02692";
        indirect_contiguous_memory_02708         = "VUID-vkCmdTraceRaysIndirect2KHR-indirectDeviceAddress-03632";
        indirect_buffer_bit_02290                = "VUID-vkCmdTraceRaysIndirect2KHR-indirectDeviceAddress-03633";
        corner_sampled_address_mode_02696        = "VUID-vkCmdTraceRaysIndirect2KHR-flags-02696";
        imageview_atomic_02691                   = "VUID-vkCmdTraceRaysIndirect2KHR-None-02691";
        bufferview_atomic_07888                  = "VUID-vkCmdTraceRaysIndirect2KHR-None-07888";
        push_constants_set_06425                 = "VUID-vkCmdTraceRaysIndirect2KHR-maintenance4-06425";
        descriptor_valid_02699                   = "VUID-vkCmdTraceRaysIndirect2KHR-None-02699";
        sampler_imageview_type_02702             = "VUID-vkCmdTraceRaysIndirect2KHR-None-02702";
        sampler_implicitLod_dref_proj_02703      = "VUID-vkCmdTraceRaysIndirect2KHR-None-02703";
        sampler_bias_offset_02704                = "VUID-vkCmdTraceRaysIndirect2KHR-None-02704";
        dynamic_state_setting_commands_02859     = "VUID-vkCmdTraceRaysIndirect2KHR-None-02859";
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
        storage_image_write_texel_count_04115           = "VUID-vkCmdTraceRaysIndirect2KHR-None-04115";
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
        pipeline_bound_02700                     = "VUID-vkCmdDrawMeshTasksNV-None-02700";
        compatible_pipeline_02697                = "VUID-vkCmdDrawMeshTasksNV-None-02697";
        render_pass_compatible_02684             = "VUID-vkCmdDrawMeshTasksNV-renderPass-02684";
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
        push_constants_set_06425                 = "VUID-vkCmdDrawMeshTasksNV-maintenance4-06425";
        image_subresources_render_pass_write_06537 = "VUID-vkCmdDrawMeshTasksNV-None-06537";
        image_subresources_subpass_read_06538    = "VUID-vkCmdDrawMeshTasksNV-None-06538";
        image_subresources_subpass_write_06539   = "VUID-vkCmdDrawMeshTasksNV-None-06539";
        descriptor_valid_02699                   = "VUID-vkCmdDrawMeshTasksNV-None-02699";
        sampler_imageview_type_02702             = "VUID-vkCmdDrawMeshTasksNV-None-02702";
        sampler_implicitLod_dref_proj_02703      = "VUID-vkCmdDrawMeshTasksNV-None-02703";
        sampler_bias_offset_02704                = "VUID-vkCmdDrawMeshTasksNV-None-02704";
        dynamic_state_setting_commands_02859     = "VUID-vkCmdDrawMeshTasksNV-None-02859";
        rasterization_samples_04740              = "VUID-vkCmdDrawMeshTasksNV-rasterizationSamples-04740";
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
        dynamic_exclusive_scissor_07878          = "VUID-vkCmdDrawMeshTasksNV-None-07878";
        dynamic_exclusive_scissor_enable_07879   = "VUID-vkCmdDrawMeshTasksNV-None-07879";
        dynamic_color_write_enable_07749         = "VUID-vkCmdDrawMeshTasksNV-None-07749";
        dynamic_color_write_enable_count_07750   = "VUID-vkCmdDrawMeshTasksNV-attachmentCount-07750";
        dynamic_attachment_feedback_loop_08877   = "VUID-vkCmdDrawMeshTasksNV-None-08877";
        dynamic_rendering_view_mask_06178        = "VUID-vkCmdDrawMeshTasksNV-viewMask-06178";
        dynamic_rendering_color_count_06179      = "VUID-vkCmdDrawMeshTasksNV-colorAttachmentCount-06179";
        dynamic_rendering_color_formats_06180    = "VUID-vkCmdDrawMeshTasksNV-colorAttachmentCount-06180";
        dynamic_rendering_unused_attachments_08911 = "VUID-vkCmdDrawMeshTasksNV-dynamicRenderingUnusedAttachments-08911";
        dynamic_rendering_undefined_color_formats_07616 = "VUID-vkCmdDrawMeshTasksNV-colorAttachmentCount-07616";
        dynamic_rendering_undefined_depth_format_07617 = "VUID-vkCmdDrawMeshTasksNV-pDepthAttachment-07617";
        dynamic_rendering_undefined_stencil_format_07618 = "VUID-vkCmdDrawMeshTasksNV-pStencilAttachment-07618";
        dynamic_rendering_depth_format_06181     = "VUID-vkCmdDrawMeshTasksNV-pDepthAttachment-06181";
        dynamic_rendering_unused_attachments_08915 = "VUID-vkCmdDrawMeshTasksNV-dynamicRenderingUnusedAttachments-08915";
        dynamic_rendering_stencil_format_06182   = "VUID-vkCmdDrawMeshTasksNV-pStencilAttachment-06182";
        dynamic_rendering_unused_attachments_08918 = "VUID-vkCmdDrawMeshTasksNV-dynamicRenderingUnusedAttachments-08918";
        dynamic_rendering_fsr_06183              = "VUID-vkCmdDrawMeshTasksNV-imageView-06183";
        dynamic_rendering_fdm_06184              = "VUID-vkCmdDrawMeshTasksNV-imageView-06184";
        dynamic_rendering_color_sample_06185     = "VUID-vkCmdDrawMeshTasksNV-colorAttachmentCount-06185";
        dynamic_rendering_depth_sample_06186     = "VUID-vkCmdDrawMeshTasksNV-pDepthAttachment-06186";
        dynamic_rendering_stencil_sample_06187   = "VUID-vkCmdDrawMeshTasksNV-pStencilAttachment-06187";
        dynamic_rendering_multi_sample_06188     = "VUID-vkCmdDrawMeshTasksNV-colorAttachmentCount-06188";
        dynamic_rendering_06189                  = "VUID-vkCmdDrawMeshTasksNV-pDepthAttachment-06189";
        dynamic_rendering_06190                  = "VUID-vkCmdDrawMeshTasksNV-pStencilAttachment-06190";
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
        storage_image_write_texel_count_04115           = "VUID-vkCmdDrawMeshTasksNV-None-04115";
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
        dynamic_coverage_reduction_mode_07647    = "VUID-vkCmdDrawMeshTasksNV-None-07647";
        dynamic_representative_fragment_test_enable_07648  = "VUID-vkCmdDrawMeshTasksNV-None-07648";
        dynamic_shading_rate_image_enable_07649  = "VUID-vkCmdDrawMeshTasksNV-None-07649";
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
    }
};

struct DispatchVuidsCmdDrawMeshTasksIndirectNV: DrawDispatchVuid {
    DispatchVuidsCmdDrawMeshTasksIndirectNV() : DrawDispatchVuid() {
        pipeline_bound_02700                     = "VUID-vkCmdDrawMeshTasksIndirectNV-None-02700";
        compatible_pipeline_02697                = "VUID-vkCmdDrawMeshTasksIndirectNV-None-02697";
        render_pass_compatible_02684             = "VUID-vkCmdDrawMeshTasksIndirectNV-renderPass-02684";
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
        push_constants_set_06425                 = "VUID-vkCmdDrawMeshTasksIndirectNV-maintenance4-06425";
        image_subresources_render_pass_write_06537 = "VUID-vkCmdDrawMeshTasksIndirectNV-None-06537";
        image_subresources_subpass_read_06538    = "VUID-vkCmdDrawMeshTasksIndirectNV-None-06538";
        image_subresources_subpass_write_06539   = "VUID-vkCmdDrawMeshTasksIndirectNV-None-06539";
        descriptor_valid_02699                   = "VUID-vkCmdDrawMeshTasksIndirectNV-None-02699";
        sampler_imageview_type_02702             = "VUID-vkCmdDrawMeshTasksIndirectNV-None-02702";
        sampler_implicitLod_dref_proj_02703      = "VUID-vkCmdDrawMeshTasksIndirectNV-None-02703";
        sampler_bias_offset_02704                = "VUID-vkCmdDrawMeshTasksIndirectNV-None-02704";
        dynamic_state_setting_commands_02859     = "VUID-vkCmdDrawMeshTasksIndirectNV-None-02859";
        rasterization_samples_04740              = "VUID-vkCmdDrawMeshTasksIndirectNV-rasterizationSamples-04740";
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
        dynamic_exclusive_scissor_07878          = "VUID-vkCmdDrawMeshTasksIndirectNV-None-07878";
        dynamic_exclusive_scissor_enable_07879   = "VUID-vkCmdDrawMeshTasksIndirectNV-None-07879";
        dynamic_color_write_enable_07749         = "VUID-vkCmdDrawMeshTasksIndirectNV-None-07749";
        dynamic_color_write_enable_count_07750   = "VUID-vkCmdDrawMeshTasksIndirectNV-attachmentCount-07750";
        dynamic_attachment_feedback_loop_08877   = "VUID-vkCmdDrawMeshTasksIndirectNV-None-08877";
        dynamic_rendering_view_mask_06178        = "VUID-vkCmdDrawMeshTasksIndirectNV-viewMask-06178";
        dynamic_rendering_color_count_06179      = "VUID-vkCmdDrawMeshTasksIndirectNV-colorAttachmentCount-06179";
        dynamic_rendering_color_formats_06180    = "VUID-vkCmdDrawMeshTasksIndirectNV-colorAttachmentCount-06180";
        dynamic_rendering_unused_attachments_08911 = "VUID-vkCmdDrawMeshTasksIndirectNV-dynamicRenderingUnusedAttachments-08911";
        dynamic_rendering_undefined_color_formats_07616 = "VUID-vkCmdDrawMeshTasksIndirectNV-colorAttachmentCount-07616";
        dynamic_rendering_undefined_depth_format_07617 = "VUID-vkCmdDrawMeshTasksIndirectNV-pDepthAttachment-07617";
        dynamic_rendering_undefined_stencil_format_07618 = "VUID-vkCmdDrawMeshTasksIndirectNV-pStencilAttachment-07618";
        dynamic_rendering_depth_format_06181     = "VUID-vkCmdDrawMeshTasksIndirectNV-pDepthAttachment-06181";
        dynamic_rendering_unused_attachments_08915 = "VUID-vkCmdDrawMeshTasksIndirectNV-dynamicRenderingUnusedAttachments-08915";
        dynamic_rendering_stencil_format_06182   = "VUID-vkCmdDrawMeshTasksIndirectNV-pStencilAttachment-06182";
        dynamic_rendering_unused_attachments_08918 = "VUID-vkCmdDrawMeshTasksIndirectNV-dynamicRenderingUnusedAttachments-08918";
        dynamic_rendering_fsr_06183              = "VUID-vkCmdDrawMeshTasksIndirectNV-imageView-06183";
        dynamic_rendering_fdm_06184              = "VUID-vkCmdDrawMeshTasksIndirectNV-imageView-06184";
        dynamic_rendering_color_sample_06185     = "VUID-vkCmdDrawMeshTasksIndirectNV-colorAttachmentCount-06185";
        dynamic_rendering_depth_sample_06186     = "VUID-vkCmdDrawMeshTasksIndirectNV-pDepthAttachment-06186";
        dynamic_rendering_stencil_sample_06187   = "VUID-vkCmdDrawMeshTasksIndirectNV-pStencilAttachment-06187";
        dynamic_rendering_multi_sample_06188     = "VUID-vkCmdDrawMeshTasksIndirectNV-colorAttachmentCount-06188";
        dynamic_rendering_06189                  = "VUID-vkCmdDrawMeshTasksIndirectNV-pDepthAttachment-06189";
        dynamic_rendering_06190                  = "VUID-vkCmdDrawMeshTasksIndirectNV-pStencilAttachment-06190";
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
        storage_image_write_texel_count_04115           = "VUID-vkCmdDrawMeshTasksIndirectNV-None-04115";
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
        dynamic_coverage_reduction_mode_07647    = "VUID-vkCmdDrawMeshTasksIndirectNV-None-07647";
        dynamic_representative_fragment_test_enable_07648  = "VUID-vkCmdDrawMeshTasksIndirectNV-None-07648";
        dynamic_shading_rate_image_enable_07649  = "VUID-vkCmdDrawMeshTasksIndirectNV-None-07649";
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
    }
};

struct DispatchVuidsCmdDrawMeshTasksIndirectCountNV : DrawDispatchVuid {
    DispatchVuidsCmdDrawMeshTasksIndirectCountNV() : DrawDispatchVuid() {
        pipeline_bound_02700                     = "VUID-vkCmdDrawMeshTasksIndirectCountNV-None-02700";
        compatible_pipeline_02697                = "VUID-vkCmdDrawMeshTasksIndirectCountNV-None-02697";
        render_pass_compatible_02684             = "VUID-vkCmdDrawMeshTasksIndirectCountNV-renderPass-02684";
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
        push_constants_set_06425                 = "VUID-vkCmdDrawMeshTasksIndirectCountNV-maintenance4-06425";
        image_subresources_render_pass_write_06537 = "VUID-vkCmdDrawMeshTasksIndirectCountNV-None-06537";
        image_subresources_subpass_read_06538    = "VUID-vkCmdDrawMeshTasksIndirectCountNV-None-06538";
        image_subresources_subpass_write_06539   = "VUID-vkCmdDrawMeshTasksIndirectCountNV-None-06539";
        descriptor_valid_02699                   = "VUID-vkCmdDrawMeshTasksIndirectCountNV-None-02699";
        sampler_imageview_type_02702             = "VUID-vkCmdDrawMeshTasksIndirectCountNV-None-02702";
        sampler_implicitLod_dref_proj_02703      = "VUID-vkCmdDrawMeshTasksIndirectCountNV-None-02703";
        sampler_bias_offset_02704                = "VUID-vkCmdDrawMeshTasksIndirectCountNV-None-02704";
        dynamic_state_setting_commands_02859     = "VUID-vkCmdDrawMeshTasksIndirectCountNV-None-02859";
        rasterization_samples_04740              = "VUID-vkCmdDrawMeshTasksIndirectCountNV-rasterizationSamples-04740";
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
        dynamic_exclusive_scissor_07878          = "VUID-vkCmdDrawMeshTasksIndirectCountNV-None-07878";
        dynamic_exclusive_scissor_enable_07879   = "VUID-vkCmdDrawMeshTasksIndirectCountNV-None-07879";
        dynamic_color_write_enable_07749         = "VUID-vkCmdDrawMeshTasksIndirectCountNV-None-07749";
        dynamic_color_write_enable_count_07750   = "VUID-vkCmdDrawMeshTasksIndirectCountNV-attachmentCount-07750";
        dynamic_attachment_feedback_loop_08877   = "VUID-vkCmdDrawMeshTasksIndirectCountNV-None-08877";
        dynamic_rendering_view_mask_06178        = "VUID-vkCmdDrawMeshTasksIndirectCountNV-viewMask-06178";
        dynamic_rendering_color_count_06179      = "VUID-vkCmdDrawMeshTasksIndirectCountNV-colorAttachmentCount-06179";
        dynamic_rendering_color_formats_06180    = "VUID-vkCmdDrawMeshTasksIndirectCountNV-colorAttachmentCount-06180";
        dynamic_rendering_unused_attachments_08911 = "VUID-vkCmdDrawMeshTasksIndirectCountNV-dynamicRenderingUnusedAttachments-08911";
        dynamic_rendering_undefined_color_formats_07616 = "VUID-vkCmdDrawMeshTasksIndirectCountNV-colorAttachmentCount-07616";
        dynamic_rendering_undefined_depth_format_07617 = "VUID-vkCmdDrawMeshTasksIndirectCountNV-pDepthAttachment-07617";
        dynamic_rendering_undefined_stencil_format_07618 = "VUID-vkCmdDrawMeshTasksIndirectCountNV-pStencilAttachment-07618";
        dynamic_rendering_depth_format_06181     = "VUID-vkCmdDrawMeshTasksIndirectCountNV-pDepthAttachment-06181";
        dynamic_rendering_unused_attachments_08915 = "VUID-vkCmdDrawMeshTasksIndirectCountNV-dynamicRenderingUnusedAttachments-08915";
        dynamic_rendering_stencil_format_06182   = "VUID-vkCmdDrawMeshTasksIndirectCountNV-pStencilAttachment-06182";
        dynamic_rendering_unused_attachments_08918 = "VUID-vkCmdDrawMeshTasksIndirectCountNV-dynamicRenderingUnusedAttachments-08918";
        dynamic_rendering_fsr_06183              = "VUID-vkCmdDrawMeshTasksIndirectCountNV-imageView-06183";
        dynamic_rendering_fdm_06184              = "VUID-vkCmdDrawMeshTasksIndirectCountNV-imageView-06184";
        dynamic_rendering_color_sample_06185     = "VUID-vkCmdDrawMeshTasksIndirectCountNV-colorAttachmentCount-06185";
        dynamic_rendering_depth_sample_06186     = "VUID-vkCmdDrawMeshTasksIndirectCountNV-pDepthAttachment-06186";
        dynamic_rendering_stencil_sample_06187   = "VUID-vkCmdDrawMeshTasksIndirectCountNV-pStencilAttachment-06187";
        dynamic_rendering_multi_sample_06188     = "VUID-vkCmdDrawMeshTasksIndirectCountNV-colorAttachmentCount-06188";
        dynamic_rendering_06189                  = "VUID-vkCmdDrawMeshTasksIndirectCountNV-pDepthAttachment-06189";
        dynamic_rendering_06190                  = "VUID-vkCmdDrawMeshTasksIndirectCountNV-pStencilAttachment-06190";
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
        storage_image_write_texel_count_04115           = "VUID-vkCmdDrawMeshTasksIndirectCountNV-None-04115";
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
        dynamic_coverage_reduction_mode_07647    = "VUID-vkCmdDrawMeshTasksIndirectCountNV-None-07647";
        dynamic_representative_fragment_test_enable_07648  = "VUID-vkCmdDrawMeshTasksIndirectCountNV-None-07648";
        dynamic_shading_rate_image_enable_07649  = "VUID-vkCmdDrawMeshTasksIndirectCountNV-None-07649";
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
    }
};

struct DispatchVuidsCmdDrawMeshTasksEXT: DrawDispatchVuid {
    DispatchVuidsCmdDrawMeshTasksEXT() : DrawDispatchVuid() {
        pipeline_bound_02700                     = "VUID-vkCmdDrawMeshTasksEXT-None-02700";
        compatible_pipeline_02697                = "VUID-vkCmdDrawMeshTasksEXT-None-02697";
        render_pass_compatible_02684             = "VUID-vkCmdDrawMeshTasksEXT-renderPass-02684";
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
        push_constants_set_06425                 = "VUID-vkCmdDrawMeshTasksEXT-maintenance4-06425";
        image_subresources_render_pass_write_06537 = "VUID-vkCmdDrawMeshTasksEXT-None-06537";
        image_subresources_subpass_read_06538    = "VUID-vkCmdDrawMeshTasksEXT-None-06538";
        image_subresources_subpass_write_06539   = "VUID-vkCmdDrawMeshTasksEXT-None-06539";
        descriptor_valid_02699                   = "VUID-vkCmdDrawMeshTasksEXT-None-02699";
        sampler_imageview_type_02702             = "VUID-vkCmdDrawMeshTasksEXT-None-02702";
        sampler_implicitLod_dref_proj_02703      = "VUID-vkCmdDrawMeshTasksEXT-None-02703";
        sampler_bias_offset_02704                = "VUID-vkCmdDrawMeshTasksEXT-None-02704";
        dynamic_state_setting_commands_02859     = "VUID-vkCmdDrawMeshTasksEXT-None-02859";
        rasterization_samples_04740              = "VUID-vkCmdDrawMeshTasksEXT-rasterizationSamples-04740";
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
        dynamic_exclusive_scissor_07878          = "VUID-vkCmdDrawMeshTasksEXT-None-07878";
        dynamic_exclusive_scissor_enable_07879   = "VUID-vkCmdDrawMeshTasksEXT-None-07879";
        dynamic_color_write_enable_07749         = "VUID-vkCmdDrawMeshTasksEXT-None-07749";
        dynamic_color_write_enable_count_07750   = "VUID-vkCmdDrawMeshTasksEXT-attachmentCount-07750";
        dynamic_attachment_feedback_loop_08877   = "VUID-vkCmdDrawMeshTasksEXT-None-08877";
        dynamic_rendering_view_mask_06178        = "VUID-vkCmdDrawMeshTasksEXT-viewMask-06178";
        dynamic_rendering_color_count_06179      = "VUID-vkCmdDrawMeshTasksEXT-colorAttachmentCount-06179";
        dynamic_rendering_color_formats_06180    = "VUID-vkCmdDrawMeshTasksEXT-colorAttachmentCount-06180";
        dynamic_rendering_unused_attachments_08911 = "VUID-vkCmdDrawMeshTasksEXT-dynamicRenderingUnusedAttachments-08911";
        dynamic_rendering_undefined_color_formats_07616 = "VUID-vkCmdDrawMeshTasksEXT-colorAttachmentCount-07616";
        dynamic_rendering_undefined_depth_format_07617 = "VUID-vkCmdDrawMeshTasksEXT-pDepthAttachment-07617";
        dynamic_rendering_undefined_stencil_format_07618 = "VUID-vkCmdDrawMeshTasksEXT-pStencilAttachment-07618";
        dynamic_rendering_depth_format_06181     = "VUID-vkCmdDrawMeshTasksEXT-pDepthAttachment-06181";
        dynamic_rendering_unused_attachments_08915 = "VUID-vkCmdDrawMeshTasksEXT-dynamicRenderingUnusedAttachments-08915";
        dynamic_rendering_stencil_format_06182   = "VUID-vkCmdDrawMeshTasksEXT-pStencilAttachment-06182";
        dynamic_rendering_unused_attachments_08918 = "VUID-vkCmdDrawMeshTasksEXT-dynamicRenderingUnusedAttachments-08918";
        dynamic_rendering_fsr_06183              = "VUID-vkCmdDrawMeshTasksEXT-imageView-06183";
        dynamic_rendering_fdm_06184              = "VUID-vkCmdDrawMeshTasksEXT-imageView-06184";
        dynamic_rendering_color_sample_06185     = "VUID-vkCmdDrawMeshTasksEXT-colorAttachmentCount-06185";
        dynamic_rendering_depth_sample_06186     = "VUID-vkCmdDrawMeshTasksEXT-pDepthAttachment-06186";
        dynamic_rendering_stencil_sample_06187   = "VUID-vkCmdDrawMeshTasksEXT-pStencilAttachment-06187";
        dynamic_rendering_multi_sample_06188     = "VUID-vkCmdDrawMeshTasksEXT-colorAttachmentCount-06188";
        dynamic_rendering_06189                  = "VUID-vkCmdDrawMeshTasksEXT-pDepthAttachment-06189";
        dynamic_rendering_06190                  = "VUID-vkCmdDrawMeshTasksEXT-pStencilAttachment-06190";
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
        storage_image_write_texel_count_04115           = "VUID-vkCmdDrawMeshTasksEXT-None-04115";
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
        dynamic_coverage_reduction_mode_07647    = "VUID-vkCmdDrawMeshTasksEXT-None-07647";
        dynamic_representative_fragment_test_enable_07648  = "VUID-vkCmdDrawMeshTasksEXT-None-07648";
        dynamic_shading_rate_image_enable_07649  = "VUID-vkCmdDrawMeshTasksEXT-None-07649";
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
    }
};

struct DispatchVuidsCmdDrawMeshTasksIndirectEXT: DrawDispatchVuid {
    DispatchVuidsCmdDrawMeshTasksIndirectEXT() : DrawDispatchVuid() {
        pipeline_bound_02700                     = "VUID-vkCmdDrawMeshTasksIndirectEXT-None-02700";
        compatible_pipeline_02697                = "VUID-vkCmdDrawMeshTasksIndirectEXT-None-02697";
        render_pass_compatible_02684             = "VUID-vkCmdDrawMeshTasksIndirectEXT-renderPass-02684";
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
        push_constants_set_06425                 = "VUID-vkCmdDrawMeshTasksIndirectEXT-maintenance4-06425";
        image_subresources_render_pass_write_06537 = "VUID-vkCmdDrawMeshTasksIndirectEXT-None-06537";
        image_subresources_subpass_read_06538    = "VUID-vkCmdDrawMeshTasksIndirectEXT-None-06538";
        image_subresources_subpass_write_06539   = "VUID-vkCmdDrawMeshTasksIndirectEXT-None-06539";
        descriptor_valid_02699                   = "VUID-vkCmdDrawMeshTasksIndirectEXT-None-02699";
        sampler_imageview_type_02702             = "VUID-vkCmdDrawMeshTasksIndirectEXT-None-02702";
        sampler_implicitLod_dref_proj_02703      = "VUID-vkCmdDrawMeshTasksIndirectEXT-None-02703";
        sampler_bias_offset_02704                = "VUID-vkCmdDrawMeshTasksIndirectEXT-None-02704";
        dynamic_state_setting_commands_02859     = "VUID-vkCmdDrawMeshTasksIndirectEXT-None-02859";
        rasterization_samples_04740              = "VUID-vkCmdDrawMeshTasksIndirectEXT-rasterizationSamples-04740";
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
        dynamic_exclusive_scissor_07878          = "VUID-vkCmdDrawMeshTasksIndirectEXT-None-07878";
        dynamic_exclusive_scissor_enable_07879   = "VUID-vkCmdDrawMeshTasksIndirectEXT-None-07879";
        dynamic_color_write_enable_07749         = "VUID-vkCmdDrawMeshTasksIndirectEXT-None-07749";
        dynamic_color_write_enable_count_07750   = "VUID-vkCmdDrawMeshTasksIndirectEXT-attachmentCount-07750";
        dynamic_attachment_feedback_loop_08877   = "VUID-vkCmdDrawMeshTasksIndirectEXT-None-08877";
        dynamic_rendering_view_mask_06178        = "VUID-vkCmdDrawMeshTasksIndirectEXT-viewMask-06178";
        dynamic_rendering_color_count_06179      = "VUID-vkCmdDrawMeshTasksIndirectEXT-colorAttachmentCount-06179";
        dynamic_rendering_color_formats_06180    = "VUID-vkCmdDrawMeshTasksIndirectEXT-colorAttachmentCount-06180";
        dynamic_rendering_unused_attachments_08911 = "VUID-vkCmdDrawMeshTasksIndirectEXT-dynamicRenderingUnusedAttachments-08911";
        dynamic_rendering_undefined_color_formats_07616 = "VUID-vkCmdDrawMeshTasksIndirectEXT-colorAttachmentCount-07616";
        dynamic_rendering_undefined_depth_format_07617 = "VUID-vkCmdDrawMeshTasksIndirectEXT-pDepthAttachment-07617";
        dynamic_rendering_undefined_stencil_format_07618 = "VUID-vkCmdDrawMeshTasksIndirectEXT-pStencilAttachment-07618";
        dynamic_rendering_depth_format_06181     = "VUID-vkCmdDrawMeshTasksIndirectEXT-pDepthAttachment-06181";
        dynamic_rendering_unused_attachments_08915 = "VUID-vkCmdDrawMeshTasksIndirectEXT-dynamicRenderingUnusedAttachments-08915";
        dynamic_rendering_stencil_format_06182   = "VUID-vkCmdDrawMeshTasksIndirectEXT-pStencilAttachment-06182";
        dynamic_rendering_unused_attachments_08918 = "VUID-vkCmdDrawMeshTasksIndirectEXT-dynamicRenderingUnusedAttachments-08918";
        dynamic_rendering_fsr_06183              = "VUID-vkCmdDrawMeshTasksIndirectEXT-imageView-06183";
        dynamic_rendering_fdm_06184              = "VUID-vkCmdDrawMeshTasksIndirectEXT-imageView-06184";
        dynamic_rendering_color_sample_06185     = "VUID-vkCmdDrawMeshTasksIndirectEXT-colorAttachmentCount-06185";
        dynamic_rendering_depth_sample_06186     = "VUID-vkCmdDrawMeshTasksIndirectEXT-pDepthAttachment-06186";
        dynamic_rendering_stencil_sample_06187   = "VUID-vkCmdDrawMeshTasksIndirectEXT-pStencilAttachment-06187";
        dynamic_rendering_multi_sample_06188     = "VUID-vkCmdDrawMeshTasksIndirectEXT-colorAttachmentCount-06188";
        dynamic_rendering_06189                  = "VUID-vkCmdDrawMeshTasksIndirectEXT-pDepthAttachment-06189";
        dynamic_rendering_06190                  = "VUID-vkCmdDrawMeshTasksIndirectEXT-pStencilAttachment-06190";
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
        storage_image_write_texel_count_04115           = "VUID-vkCmdDrawMeshTasksIndirectEXT-None-04115";
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
        dynamic_coverage_reduction_mode_07647    = "VUID-vkCmdDrawMeshTasksIndirectEXT-None-07647";
        dynamic_representative_fragment_test_enable_07648  = "VUID-vkCmdDrawMeshTasksIndirectEXT-None-07648";
        dynamic_shading_rate_image_enable_07649  = "VUID-vkCmdDrawMeshTasksIndirectEXT-None-07649";
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
    }
};

struct DispatchVuidsCmdDrawMeshTasksIndirectCountEXT : DrawDispatchVuid {
    DispatchVuidsCmdDrawMeshTasksIndirectCountEXT() : DrawDispatchVuid() {
        pipeline_bound_02700                     = "VUID-vkCmdDrawMeshTasksIndirectCountEXT-None-02700";
        compatible_pipeline_02697                = "VUID-vkCmdDrawMeshTasksIndirectCountEXT-None-02697";
        render_pass_compatible_02684             = "VUID-vkCmdDrawMeshTasksIndirectCountEXT-renderPass-02684";
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
        push_constants_set_06425                 = "VUID-vkCmdDrawMeshTasksIndirectCountEXT-maintenance4-06425";
        image_subresources_render_pass_write_06537 = "VUID-vkCmdDrawMeshTasksIndirectCountEXT-None-06537";
        image_subresources_subpass_read_06538    = "VUID-vkCmdDrawMeshTasksIndirectCountEXT-None-06538";
        image_subresources_subpass_write_06539   = "VUID-vkCmdDrawMeshTasksIndirectCountEXT-None-06539";
        descriptor_valid_02699                   = "VUID-vkCmdDrawMeshTasksIndirectCountEXT-None-02699";
        sampler_imageview_type_02702             = "VUID-vkCmdDrawMeshTasksIndirectCountEXT-None-02702";
        sampler_implicitLod_dref_proj_02703      = "VUID-vkCmdDrawMeshTasksIndirectCountEXT-None-02703";
        sampler_bias_offset_02704                = "VUID-vkCmdDrawMeshTasksIndirectCountEXT-None-02704";
        dynamic_state_setting_commands_02859     = "VUID-vkCmdDrawMeshTasksIndirectCountEXT-None-02859";
        rasterization_samples_04740              = "VUID-vkCmdDrawMeshTasksIndirectCountEXT-rasterizationSamples-04740";
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
        dynamic_exclusive_scissor_07878          = "VUID-vkCmdDrawMeshTasksIndirectCountEXT-None-07878";
        dynamic_exclusive_scissor_enable_07879   = "VUID-vkCmdDrawMeshTasksIndirectCountEXT-None-07879";
        dynamic_color_write_enable_07749         = "VUID-vkCmdDrawMeshTasksIndirectCountEXT-None-07749";
        dynamic_color_write_enable_count_07750   = "VUID-vkCmdDrawMeshTasksIndirectCountEXT-attachmentCount-07750";
        dynamic_attachment_feedback_loop_08877   = "VUID-vkCmdDrawMeshTasksIndirectCountEXT-None-08877";
        dynamic_rendering_view_mask_06178        = "VUID-vkCmdDrawMeshTasksIndirectCountEXT-viewMask-06178";
        dynamic_rendering_color_count_06179      = "VUID-vkCmdDrawMeshTasksIndirectCountEXT-colorAttachmentCount-06179";
        dynamic_rendering_color_formats_06180    = "VUID-vkCmdDrawMeshTasksIndirectCountEXT-colorAttachmentCount-06180";
        dynamic_rendering_unused_attachments_08911 = "VUID-vkCmdDrawMeshTasksIndirectCountEXT-dynamicRenderingUnusedAttachments-08911";
        dynamic_rendering_undefined_color_formats_07616 = "VUID-vkCmdDrawMeshTasksIndirectCountEXT-colorAttachmentCount-07616";
        dynamic_rendering_undefined_depth_format_07617 = "VUID-vkCmdDrawMeshTasksIndirectCountEXT-pDepthAttachment-07617";
        dynamic_rendering_undefined_stencil_format_07618 = "VUID-vkCmdDrawMeshTasksIndirectCountEXT-pStencilAttachment-07618";
        dynamic_rendering_depth_format_06181     = "VUID-vkCmdDrawMeshTasksIndirectCountEXT-pDepthAttachment-06181";
        dynamic_rendering_unused_attachments_08915 = "VUID-vkCmdDrawMeshTasksIndirectCountEXT-dynamicRenderingUnusedAttachments-08915";
        dynamic_rendering_stencil_format_06182   = "VUID-vkCmdDrawMeshTasksIndirectCountEXT-pStencilAttachment-06182";
        dynamic_rendering_unused_attachments_08918 = "VUID-vkCmdDrawMeshTasksIndirectCountEXT-dynamicRenderingUnusedAttachments-08918";
        dynamic_rendering_fsr_06183              = "VUID-vkCmdDrawMeshTasksIndirectCountEXT-imageView-06183";
        dynamic_rendering_fdm_06184              = "VUID-vkCmdDrawMeshTasksIndirectCountEXT-imageView-06184";
        dynamic_rendering_color_sample_06185     = "VUID-vkCmdDrawMeshTasksIndirectCountEXT-colorAttachmentCount-06185";
        dynamic_rendering_depth_sample_06186     = "VUID-vkCmdDrawMeshTasksIndirectCountEXT-pDepthAttachment-06186";
        dynamic_rendering_stencil_sample_06187   = "VUID-vkCmdDrawMeshTasksIndirectCountEXT-pStencilAttachment-06187";
        dynamic_rendering_multi_sample_06188     = "VUID-vkCmdDrawMeshTasksIndirectCountEXT-colorAttachmentCount-06188";
        dynamic_rendering_06189                  = "VUID-vkCmdDrawMeshTasksIndirectCountEXT-pDepthAttachment-06189";
        dynamic_rendering_06190                  = "VUID-vkCmdDrawMeshTasksIndirectCountEXT-pStencilAttachment-06190";
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
        storage_image_write_texel_count_04115           = "VUID-vkCmdDrawMeshTasksIndirectCountEXT-None-04115";
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
        dynamic_coverage_reduction_mode_07647    = "VUID-vkCmdDrawMeshTasksIndirectCountEXT-None-07647";
        dynamic_representative_fragment_test_enable_07648  = "VUID-vkCmdDrawMeshTasksIndirectCountEXT-None-07648";
        dynamic_shading_rate_image_enable_07649  = "VUID-vkCmdDrawMeshTasksIndirectCountEXT-None-07649";
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
    }
};

struct DispatchVuidsCmdDrawIndirectByteCountEXT: DrawDispatchVuid {
    DispatchVuidsCmdDrawIndirectByteCountEXT() : DrawDispatchVuid() {
        pipeline_bound_02700                     = "VUID-vkCmdDrawIndirectByteCountEXT-None-02700";
        vertex_binding_04007                     = "VUID-vkCmdDrawIndirectByteCountEXT-None-04007";
        vertex_binding_null_04008                = "VUID-vkCmdDrawIndirectByteCountEXT-None-04008";
        compatible_pipeline_02697                = "VUID-vkCmdDrawIndirectByteCountEXT-None-02697";
        render_pass_compatible_02684             = "VUID-vkCmdDrawIndirectByteCountEXT-renderPass-02684";
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
        primitive_topology_class_03420           = "VUID-vkCmdDrawIndirectByteCountEXT-primitiveTopology-03420";
        primitive_topology_class_ds3_07500       = "VUID-vkCmdDrawIndirectByteCountEXT-dynamicPrimitiveTopologyUnrestricted-07500";
        corner_sampled_address_mode_02696        = "VUID-vkCmdDrawIndirectByteCountEXT-flags-02696";
        imageview_atomic_02691                   = "VUID-vkCmdDrawIndirectByteCountEXT-None-02691";
        bufferview_atomic_07888                  = "VUID-vkCmdDrawIndirectByteCountEXT-None-07888";
        push_constants_set_06425                 = "VUID-vkCmdDrawIndirectByteCountEXT-maintenance4-06425";
        image_subresources_render_pass_write_06537 = "VUID-vkCmdDrawIndirectByteCountEXT-None-06537";
        image_subresources_subpass_read_06538    = "VUID-vkCmdDrawIndirectByteCountEXT-None-06538";
        image_subresources_subpass_write_06539   = "VUID-vkCmdDrawIndirectByteCountEXT-None-06539";
        descriptor_valid_02699                   = "VUID-vkCmdDrawIndirectByteCountEXT-None-02699";
        sampler_imageview_type_02702             = "VUID-vkCmdDrawIndirectByteCountEXT-None-02702";
        sampler_implicitLod_dref_proj_02703      = "VUID-vkCmdDrawIndirectByteCountEXT-None-02703";
        sampler_bias_offset_02704                = "VUID-vkCmdDrawIndirectByteCountEXT-None-02704";
        vertex_binding_attribute_02721           = "VUID-vkCmdDrawIndirectByteCountEXT-None-02721";
        dynamic_state_setting_commands_02859     = "VUID-vkCmdDrawIndirectByteCountEXT-None-02859";
        rasterization_samples_04740              = "VUID-vkCmdDrawIndirectByteCountEXT-rasterizationSamples-04740";
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
        vertex_input_binding_stride_04884        = "VUID-vkCmdDrawIndirectByteCountEXT-pStrides-04884";
        vertex_input_04914                       = "VUID-vkCmdDrawIndirectByteCountEXT-None-04914";
        blend_enable_04727                       = "VUID-vkCmdDrawIndirectByteCountEXT-blendEnable-04727";
        dynamic_discard_rectangle_07751          = "VUID-vkCmdDrawIndirectByteCountEXT-None-07751";
        dynamic_discard_rectangle_enable_07880   = "VUID-vkCmdDrawIndirectByteCountEXT-None-07880";
        dynamic_discard_rectangle_mode_07881     =  "VUID-vkCmdDrawIndirectByteCountEXT-None-07881";
        dynamic_exclusive_scissor_07878          = "VUID-vkCmdDrawIndirectByteCountEXT-None-07878";
        dynamic_exclusive_scissor_enable_07879   = "VUID-vkCmdDrawIndirectByteCountEXT-None-07879";
        dynamic_color_write_enable_07749         = "VUID-vkCmdDrawIndirectByteCountEXT-None-07749";
        dynamic_color_write_enable_count_07750   = "VUID-vkCmdDrawIndirectByteCountEXT-attachmentCount-07750";
        dynamic_attachment_feedback_loop_08877   = "VUID-vkCmdDrawIndirectByteCountEXT-None-08877";
        dynamic_rendering_view_mask_06178        = "VUID-vkCmdDrawIndirectByteCountEXT-viewMask-06178";
        dynamic_rendering_color_count_06179      = "VUID-vkCmdDrawIndirectByteCountEXT-colorAttachmentCount-06179";
        dynamic_rendering_color_formats_06180    = "VUID-vkCmdDrawIndirectByteCountEXT-colorAttachmentCount-06180";
        dynamic_rendering_unused_attachments_08911 = "VUID-vkCmdDrawIndirectByteCountEXT-dynamicRenderingUnusedAttachments-08911";
        dynamic_rendering_undefined_color_formats_07616 = "VUID-vkCmdDrawIndirectByteCountEXT-colorAttachmentCount-07616";
        dynamic_rendering_undefined_depth_format_07617 = "VUID-vkCmdDrawIndirectByteCountEXT-pDepthAttachment-07617";
        dynamic_rendering_undefined_stencil_format_07618 = "VUID-vkCmdDrawIndirectByteCountEXT-pStencilAttachment-07618";
        dynamic_rendering_depth_format_06181     = "VUID-vkCmdDrawIndirectByteCountEXT-pDepthAttachment-06181";
        dynamic_rendering_unused_attachments_08915 = "VUID-vkCmdDrawIndirectByteCountEXT-dynamicRenderingUnusedAttachments-08915";
        dynamic_rendering_stencil_format_06182   = "VUID-vkCmdDrawIndirectByteCountEXT-pStencilAttachment-06182";
        dynamic_rendering_unused_attachments_08918 = "VUID-vkCmdDrawIndirectByteCountEXT-dynamicRenderingUnusedAttachments-08918";
        dynamic_rendering_fsr_06183              = "VUID-vkCmdDrawIndirectByteCountEXT-imageView-06183";
        dynamic_rendering_fdm_06184              = "VUID-vkCmdDrawIndirectByteCountEXT-imageView-06184";
        dynamic_rendering_color_sample_06185     = "VUID-vkCmdDrawIndirectByteCountEXT-colorAttachmentCount-06185";
        dynamic_rendering_depth_sample_06186     = "VUID-vkCmdDrawIndirectByteCountEXT-pDepthAttachment-06186";
        dynamic_rendering_stencil_sample_06187   = "VUID-vkCmdDrawIndirectByteCountEXT-pStencilAttachment-06187";
        dynamic_rendering_multi_sample_06188     = "VUID-vkCmdDrawIndirectByteCountEXT-colorAttachmentCount-06188";
        dynamic_rendering_06189                  = "VUID-vkCmdDrawIndirectByteCountEXT-pDepthAttachment-06189";
        dynamic_rendering_06190                  = "VUID-vkCmdDrawIndirectByteCountEXT-pStencilAttachment-06190";
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
        storage_image_write_texel_count_04115           = "VUID-vkCmdDrawIndirectByteCountEXT-None-04115";
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
        dynamic_coverage_reduction_mode_07647    = "VUID-vkCmdDrawIndirectByteCountEXT-None-07647";
        dynamic_representative_fragment_test_enable_07648  = "VUID-vkCmdDrawIndirectByteCountEXT-None-07648";
        dynamic_shading_rate_image_enable_07649  = "VUID-vkCmdDrawIndirectByteCountEXT-None-07649";
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
    }
};

struct DispatchVuidsCmdDispatchBase: DrawDispatchVuid {
    DispatchVuidsCmdDispatchBase() : DrawDispatchVuid() {
        pipeline_bound_02700                     = "VUID-vkCmdDispatchBase-None-02700";
        compatible_pipeline_02697                = "VUID-vkCmdDispatchBase-None-02697";
        linear_filter_sampler_04553              = "VUID-vkCmdDispatchBase-magFilter-04553";
        linear_mipmap_sampler_04770              = "VUID-vkCmdDispatchBase-mipmapMode-04770";
        cubic_sampler_02692                      = "VUID-vkCmdDispatchBase-None-02692";
        corner_sampled_address_mode_02696        = "VUID-vkCmdDispatchBase-flags-02696";
        imageview_atomic_02691                   = "VUID-vkCmdDispatchBase-None-02691";
        bufferview_atomic_07888                  = "VUID-vkCmdDispatchBase-None-07888";
        push_constants_set_06425                 = "VUID-vkCmdDispatchBase-maintenance4-06425";
        descriptor_valid_02699                   = "VUID-vkCmdDispatchBase-None-02699";
        sampler_imageview_type_02702             = "VUID-vkCmdDispatchBase-None-02702";
        sampler_implicitLod_dref_proj_02703      = "VUID-vkCmdDispatchBase-None-02703";
        sampler_bias_offset_02704                = "VUID-vkCmdDispatchBase-None-02704";
        dynamic_state_setting_commands_02859     = "VUID-vkCmdDispatchBase-None-02859";
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
        storage_image_write_texel_count_04115           = "VUID-vkCmdDispatchBase-None-04115";
        storage_texel_buffer_write_texel_count_04469    = "VUID-vkCmdDispatchBase-OpImageWrite-04469";
        depth_compare_sample_06479               = "VUID-vkCmdDispatchBase-None-06479";
        descriptor_buffer_bit_set_08114          = "VUID-vkCmdDispatchBase-None-08114";
        descriptor_buffer_bit_not_set_08115      = "VUID-vkCmdDispatchBase-None-08115";
        descriptor_buffer_set_offset_missing_08117 = "VUID-vkCmdDispatchBase-None-08117";
        image_view_dim_07752                     = "VUID-vkCmdDispatchBase-viewType-07752";
        image_view_numeric_format_07753          = "VUID-vkCmdDispatchBase-format-07753";
    }
};

// This LUT is created to allow a static listing of each VUID that is covered by drawdispatch commands
static const std::map<CMD_TYPE, DrawDispatchVuid> kDrawdispatchVuid = {
    {CMD_DRAW, DispatchVuidsCmdDraw()},
    {CMD_DRAWMULTIEXT, DispatchVuidsCmdDrawMultiEXT()},
    {CMD_DRAWINDEXED, DispatchVuidsCmdDrawIndexed()},
    {CMD_DRAWMULTIINDEXEDEXT, DispatchVuidsCmdDrawMultiIndexedEXT()},
    {CMD_DRAWINDIRECT, DispatchVuidsCmdDrawIndirect()},
    {CMD_DRAWINDEXEDINDIRECT, DispatchVuidsCmdDrawIndexedIndirect()},
    {CMD_DISPATCH, DispatchVuidsCmdDispatch()},
    {CMD_DISPATCHINDIRECT, DispatchVuidsCmdDispatchIndirect()},
    {CMD_DRAWINDIRECTCOUNT, DispatchVuidsCmdDrawIndirectCount()},
    {CMD_DRAWINDIRECTCOUNTKHR, DispatchVuidsCmdDrawIndirectCount()},
    {CMD_DRAWINDEXEDINDIRECTCOUNT, DispatchVuidsCmdDrawIndexedIndirectCount()},
    {CMD_DRAWINDEXEDINDIRECTCOUNTKHR, DispatchVuidsCmdDrawIndexedIndirectCount()},
    {CMD_TRACERAYSNV, DispatchVuidsCmdTraceRaysNV()},
    {CMD_TRACERAYSKHR, DispatchVuidsCmdTraceRaysKHR()},
    {CMD_TRACERAYSINDIRECTKHR, DispatchVuidsCmdTraceRaysIndirectKHR()},
    {CMD_TRACERAYSINDIRECT2KHR, DispatchVuidsCmdTraceRaysIndirect2KHR()},
    {CMD_DRAWMESHTASKSNV, DispatchVuidsCmdDrawMeshTasksNV()},
    {CMD_DRAWMESHTASKSINDIRECTNV, DispatchVuidsCmdDrawMeshTasksIndirectNV()},
    {CMD_DRAWMESHTASKSINDIRECTCOUNTNV, DispatchVuidsCmdDrawMeshTasksIndirectCountNV()},
    {CMD_DRAWMESHTASKSEXT, DispatchVuidsCmdDrawMeshTasksEXT()},
    {CMD_DRAWMESHTASKSINDIRECTEXT, DispatchVuidsCmdDrawMeshTasksIndirectEXT()},
    {CMD_DRAWMESHTASKSINDIRECTCOUNTEXT, DispatchVuidsCmdDrawMeshTasksIndirectCountEXT()},
    {CMD_DRAWINDIRECTBYTECOUNTEXT, DispatchVuidsCmdDrawIndirectByteCountEXT()},
    {CMD_DISPATCHBASE, DispatchVuidsCmdDispatchBase()},
    {CMD_DISPATCHBASEKHR, DispatchVuidsCmdDispatchBase()},
    // Used if invalid cmd_type is used
    {CMD_NONE, DrawDispatchVuid()}
};
// clang-format on

// Getter function to provide kVUIDUndefined in case an invalid cmd_type is passed in
const DrawDispatchVuid &CoreChecks::GetDrawDispatchVuid(CMD_TYPE cmd_type) const {
    if (kDrawdispatchVuid.find(cmd_type) != kDrawdispatchVuid.cend()) {
        return kDrawdispatchVuid.at(cmd_type);
    } else {
        return kDrawdispatchVuid.at(CMD_NONE);
    }
}

// Generic function to handle validation for all action commands
// Action command == vkCmdDraw*, vkCmdDispatch*, vkCmdTraceRays*, etc.
bool CoreChecks::ValidateActionCmd(const CMD_BUFFER_STATE &cb_state, VkPipelineBindPoint bind_point, CMD_TYPE cmd_type) const {
    bool skip = false;
    skip |= ValidateCmd(cb_state, cmd_type);
    skip |= ValidateCmdBufDrawState(cb_state, cmd_type, bind_point);
    skip |= ValidateCmdRayQueryState(cb_state, cmd_type, bind_point);
    return skip;
}

bool CoreChecks::ValidateGraphicsIndexedCmd(const CMD_BUFFER_STATE &cb_state, CMD_TYPE cmd_type) const {
    bool skip = false;
    const DrawDispatchVuid &vuid = GetDrawDispatchVuid(cmd_type);
    if (!cb_state.index_buffer_binding.bound()) {
        skip |= LogError(cb_state.commandBuffer(), vuid.index_binding_07312,
                         "%s: Index buffer object has not been bound to this command buffer.", CommandTypeString(cmd_type));
    }
    return skip;
}

bool CoreChecks::ValidateCmdDrawInstance(const CMD_BUFFER_STATE &cb_state, uint32_t instanceCount, uint32_t firstInstance,
                                         CMD_TYPE cmd_type) const {
    bool skip = false;
    const DrawDispatchVuid &vuid = GetDrawDispatchVuid(cmd_type);
    const char *caller = CommandTypeString(cmd_type);

    // Verify maxMultiviewInstanceIndex
    if (cb_state.activeRenderPass && cb_state.activeRenderPass->renderPass() && enabled_features.multiview_features.multiview &&
        ((instanceCount + firstInstance) > phys_dev_ext_props.multiview_props.maxMultiviewInstanceIndex)) {
        const LogObjectList objlist(cb_state.Handle(), cb_state.activeRenderPass->Handle());
        skip |= LogError(objlist, vuid.max_multiview_instance_index_02688,
                         "%s: renderpass %s multiview is enabled, and maxMultiviewInstanceIndex: %" PRIu32
                         ", but instanceCount: %" PRIu32 "and firstInstance: %" PRIu32 ".",
                         caller, report_data->FormatHandle(cb_state.activeRenderPass->Handle()).c_str(),
                         phys_dev_ext_props.multiview_props.maxMultiviewInstanceIndex, instanceCount, firstInstance);
    }
    return skip;
}

bool CoreChecks::ValidateVTGShaderStages(const CMD_BUFFER_STATE &cb_state, CMD_TYPE cmd_type) const {
    bool skip = false;
    const DrawDispatchVuid &vuid = GetDrawDispatchVuid(cmd_type);
    const char *api_name = CommandTypeString(cmd_type);

    const auto *pipeline_state = cb_state.lastBound[VK_PIPELINE_BIND_POINT_GRAPHICS].pipeline_state;
    if (pipeline_state && pipeline_state->active_shaders & (VK_SHADER_STAGE_TASK_BIT_EXT | VK_SHADER_STAGE_MESH_BIT_EXT)) {
        skip |= LogError(
            cb_state.commandBuffer(), vuid.invalid_mesh_shader_stages_06481,
            "%s : The bound graphics pipeline must not have been created with "
            "VK_SHADER_STAGE_TASK_BIT_EXT or VK_SHADER_STAGE_MESH_BIT_EXT. Active shader stages on the bound pipeline are %s.",
            api_name, string_VkShaderStageFlags(pipeline_state->active_shaders).c_str());
    }
    return skip;
}

bool CoreChecks::ValidateMeshShaderStage(const CMD_BUFFER_STATE &cb_state, CMD_TYPE cmd_type, bool is_NV) const {
    bool skip = false;
    const DrawDispatchVuid &vuid = GetDrawDispatchVuid(cmd_type);
    const char *api_name = CommandTypeString(cmd_type);

    const auto *pipeline_state = cb_state.lastBound[VK_PIPELINE_BIND_POINT_GRAPHICS].pipeline_state;
    if (pipeline_state && !(pipeline_state->active_shaders & VK_SHADER_STAGE_MESH_BIT_EXT)) {
        skip |= LogError(cb_state.commandBuffer(), vuid.missing_mesh_shader_stages_07080,
                         "%s : The current pipeline bound to VK_PIPELINE_BIND_POINT_GRAPHICS must contain a shader stage using the "
                         "%s Execution Model. Active shader stages on the bound pipeline are %s.",
                         api_name, is_NV ? "MeshNV" : "MeshEXT", string_VkShaderStageFlags(pipeline_state->active_shaders).c_str());
    }
    if (pipeline_state &&
        (pipeline_state->active_shaders & (VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT |
                                           VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT | VK_SHADER_STAGE_GEOMETRY_BIT))) {
        skip |= LogError(cb_state.commandBuffer(), vuid.mesh_shader_stages_06480,
                         "%s : The bound graphics pipeline must not have been created with "
                         "VK_SHADER_STAGE_VERTEX_BIT, VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT, "
                         "VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT or VK_SHADER_STAGE_GEOMETRY_BIT. Active shader stages on the "
                         "bound pipeline are %s.",
                         api_name, string_VkShaderStageFlags(pipeline_state->active_shaders).c_str());
    }
    return skip;
}

bool CoreChecks::PreCallValidateCmdDraw(VkCommandBuffer commandBuffer, uint32_t vertexCount, uint32_t instanceCount,
                                        uint32_t firstVertex, uint32_t firstInstance) const {
    bool skip = false;
    auto cb_state = GetRead<CMD_BUFFER_STATE>(commandBuffer);
    skip |= ValidateCmdDrawInstance(*cb_state, instanceCount, firstInstance, CMD_DRAW);
    skip |= ValidateActionCmd(*cb_state, VK_PIPELINE_BIND_POINT_GRAPHICS, CMD_DRAW);
    skip |= ValidateVTGShaderStages(*cb_state, CMD_DRAW);
    return skip;
}

bool CoreChecks::PreCallValidateCmdDrawMultiEXT(VkCommandBuffer commandBuffer, uint32_t drawCount,
                                                const VkMultiDrawInfoEXT *pVertexInfo, uint32_t instanceCount,
                                                uint32_t firstInstance, uint32_t stride) const {
    bool skip = false;
    if (!enabled_features.multi_draw_features.multiDraw) {
        skip |= LogError(commandBuffer, "VUID-vkCmdDrawMultiEXT-None-04933",
                         "vkCmdDrawMultiEXT(): The multiDraw feature must be enabled to "
                         "call this command.");
    }
    if (drawCount > phys_dev_ext_props.multi_draw_props.maxMultiDrawCount) {
        skip |= LogError(commandBuffer, "VUID-vkCmdDrawMultiEXT-drawCount-04934",
                         "vkCmdDrawMultiEXT(): parameter, uint32_t drawCount (%" PRIu32
                         ") must be less than VkPhysicalDeviceMultiDrawPropertiesEXT::maxMultiDrawCount (%" PRIu32 ").",
                         drawCount, phys_dev_ext_props.multi_draw_props.maxMultiDrawCount);
    }
    auto cb_state = GetRead<CMD_BUFFER_STATE>(commandBuffer);
    skip |= ValidateCmdDrawInstance(*cb_state, instanceCount, firstInstance, CMD_DRAWMULTIEXT);
    skip |= ValidateActionCmd(*cb_state, VK_PIPELINE_BIND_POINT_GRAPHICS, CMD_DRAWMULTIEXT);
    skip |= ValidateVTGShaderStages(*cb_state, CMD_DRAWMULTIEXT);
    return skip;
}

bool CoreChecks::ValidateCmdDrawIndexedBufferSize(const CMD_BUFFER_STATE &cb_state, uint32_t indexCount, uint32_t firstIndex,
                                                  const char *caller, const char *first_index_vuid) const {
    bool skip = false;
    if (!enabled_features.robustness2_features.robustBufferAccess2 && cb_state.index_buffer_binding.bound()) {
        const auto &index_buffer_binding = cb_state.index_buffer_binding;
        const uint32_t index_size = GetIndexAlignment(index_buffer_binding.index_type);
        // This doesn't exactly match the pseudocode of the VUID, but the binding size is the *bound* size, such that the offset
        // has already been accounted for (subtracted from the buffer size), and is consistent with the use of
        // BufferBinding::size for vertex buffer bindings (which record the *bound* size, not the size of the bound buffer)
        VkDeviceSize end_offset = static_cast<VkDeviceSize>(index_size * (firstIndex + indexCount));
        if (end_offset > index_buffer_binding.size) {
            skip |= LogError(index_buffer_binding.buffer_state->buffer(), first_index_vuid,
                             "%s: index size (%u) * (firstIndex (%u) + indexCount (%u)) "
                             "+ binding offset (%" PRIuLEAST64 ") = an ending offset of %" PRIuLEAST64
                             " bytes, which is greater than the index buffer size (%" PRIuLEAST64 ").",
                             caller, index_size, firstIndex, indexCount, index_buffer_binding.offset,
                             end_offset + index_buffer_binding.offset, index_buffer_binding.size + index_buffer_binding.offset);
        }
    }
    return skip;
}

bool CoreChecks::PreCallValidateCmdDrawIndexed(VkCommandBuffer commandBuffer, uint32_t indexCount, uint32_t instanceCount,
                                               uint32_t firstIndex, int32_t vertexOffset, uint32_t firstInstance) const {
    bool skip = false;
    auto cb_state = GetRead<CMD_BUFFER_STATE>(commandBuffer);
    skip |= ValidateCmdDrawInstance(*cb_state, instanceCount, firstInstance, CMD_DRAWINDEXED);
    skip |= ValidateGraphicsIndexedCmd(*cb_state, CMD_DRAWINDEXED);
    skip |= ValidateActionCmd(*cb_state, VK_PIPELINE_BIND_POINT_GRAPHICS, CMD_DRAWINDEXED);
    skip |= ValidateCmdDrawIndexedBufferSize(*cb_state, indexCount, firstIndex, "vkCmdDrawIndexed()",
                                             "VUID-vkCmdDrawIndexed-robustBufferAccess2-07825");
    skip |= ValidateVTGShaderStages(*cb_state, CMD_DRAWINDEXED);
    return skip;
}

bool CoreChecks::PreCallValidateCmdDrawMultiIndexedEXT(VkCommandBuffer commandBuffer, uint32_t drawCount,
                                                       const VkMultiDrawIndexedInfoEXT *pIndexInfo, uint32_t instanceCount,
                                                       uint32_t firstInstance, uint32_t stride,
                                                       const int32_t *pVertexOffset) const {
    bool skip = false;
    if (!enabled_features.multi_draw_features.multiDraw) {
        skip |= LogError(commandBuffer, "VUID-vkCmdDrawMultiIndexedEXT-None-04937",
                         "vkCmdDrawMultiIndexedEXT(): The multiDraw feature must be enabled to "
                         "call this command.");
    }
    if (drawCount > phys_dev_ext_props.multi_draw_props.maxMultiDrawCount) {
        skip |= LogError(commandBuffer, "VUID-vkCmdDrawMultiIndexedEXT-drawCount-04939",
                         "vkCmdDrawMultiIndexedEXT(): parameter, uint32_t drawCount (0x%" PRIu32
                         ") must be less than VkPhysicalDeviceMultiDrawPropertiesEXT::maxMultiDrawCount (0x%" PRIu32 ").",
                         drawCount, phys_dev_ext_props.multi_draw_props.maxMultiDrawCount);
    }
    auto cb_state = GetRead<CMD_BUFFER_STATE>(commandBuffer);
    skip |= ValidateCmdDrawInstance(*cb_state, instanceCount, firstInstance, CMD_DRAWMULTIINDEXEDEXT);
    skip |= ValidateGraphicsIndexedCmd(*cb_state, CMD_DRAWMULTIINDEXEDEXT);
    skip |= ValidateActionCmd(*cb_state, VK_PIPELINE_BIND_POINT_GRAPHICS, CMD_DRAWMULTIINDEXEDEXT);
    const auto info_bytes = reinterpret_cast<const char *>(pIndexInfo);
    for (uint32_t i = 0; i < drawCount; i++) {
        const auto info_ptr = reinterpret_cast<const VkMultiDrawIndexedInfoEXT *>(info_bytes + i * stride);
        skip |= ValidateCmdDrawIndexedBufferSize(*cb_state, info_ptr->indexCount, info_ptr->firstIndex,
                                                 "vkCmdDrawMultiIndexedEXT()", "VUID-vkCmdDrawMultiIndexedEXT-robustBufferAccess2-07825");
    }
    skip |= ValidateVTGShaderStages(*cb_state, CMD_DRAWMULTIINDEXEDEXT);
    return skip;
}

bool CoreChecks::PreCallValidateCmdDrawIndirect(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                uint32_t drawCount, uint32_t stride) const {
    auto cb_state = GetRead<CMD_BUFFER_STATE>(commandBuffer);
    bool skip = ValidateActionCmd(*cb_state, VK_PIPELINE_BIND_POINT_GRAPHICS, CMD_DRAWINDIRECT);
    auto buffer_state = Get<BUFFER_STATE>(buffer);
    skip |= ValidateIndirectCmd(*cb_state, *buffer_state, CMD_DRAWINDIRECT);
    if (drawCount > 1) {
        skip |= ValidateCmdDrawStrideWithStruct(commandBuffer, "VUID-vkCmdDrawIndirect-drawCount-00476", stride,
                                                "VkDrawIndirectCommand", sizeof(VkDrawIndirectCommand));
        skip |= ValidateCmdDrawStrideWithBuffer(commandBuffer, "VUID-vkCmdDrawIndirect-drawCount-00488", stride,
                                                "VkDrawIndirectCommand", sizeof(VkDrawIndirectCommand), drawCount, offset,
                                                buffer_state.get());
    } else if ((drawCount == 1) && (offset + sizeof(VkDrawIndirectCommand)) > buffer_state->createInfo.size) {
        skip |= LogError(commandBuffer, "VUID-vkCmdDrawIndirect-drawCount-00487",
                         "CmdDrawIndirect: drawCount equals 1 and (offset + sizeof(VkDrawIndirectCommand)) (%" PRIu64
                         ") is not less than "
                         "or equal to the size of buffer (%" PRIu64 ").",
                         (offset + sizeof(VkDrawIndirectCommand)), buffer_state->createInfo.size);
    }
    skip |= ValidateVTGShaderStages(*cb_state, CMD_DRAWINDIRECT);
    // TODO: If the drawIndirectFirstInstance feature is not enabled, all the firstInstance members of the
    // VkDrawIndirectCommand structures accessed by this command must be 0, which will require access to the contents of 'buffer'.
    return skip;
}

bool CoreChecks::PreCallValidateCmdDrawIndexedIndirect(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                       uint32_t drawCount, uint32_t stride) const {
    auto cb_state = GetRead<CMD_BUFFER_STATE>(commandBuffer);
    bool skip = false;
    skip |= ValidateGraphicsIndexedCmd(*cb_state, CMD_DRAWINDEXEDINDIRECT);
    skip |= ValidateActionCmd(*cb_state, VK_PIPELINE_BIND_POINT_GRAPHICS, CMD_DRAWINDEXEDINDIRECT);
    auto buffer_state = Get<BUFFER_STATE>(buffer);
    skip |= ValidateIndirectCmd(*cb_state, *buffer_state, CMD_DRAWINDEXEDINDIRECT);
    if (drawCount > 1) {
        skip |= ValidateCmdDrawStrideWithStruct(commandBuffer, "VUID-vkCmdDrawIndexedIndirect-drawCount-00528", stride,
                                                "VkDrawIndexedIndirectCommand", sizeof(VkDrawIndexedIndirectCommand));
        skip |= ValidateCmdDrawStrideWithBuffer(commandBuffer, "VUID-vkCmdDrawIndexedIndirect-drawCount-00540", stride,
                                                "VkDrawIndexedIndirectCommand", sizeof(VkDrawIndexedIndirectCommand), drawCount,
                                                offset, buffer_state.get());
    } else if ((drawCount == 1) && (offset + sizeof(VkDrawIndexedIndirectCommand)) > buffer_state->createInfo.size) {
        skip |= LogError(commandBuffer, "VUID-vkCmdDrawIndexedIndirect-drawCount-00539",
                         "CmdDrawIndexedIndirect: drawCount equals 1 and (offset + sizeof(VkDrawIndexedIndirectCommand)) (%" PRIu64
                         ") is not less than "
                         "or equal to the size of buffer (%" PRIu64 ").",
                         (offset + sizeof(VkDrawIndexedIndirectCommand)), buffer_state->createInfo.size);
    }
    skip |= ValidateVTGShaderStages(*cb_state, CMD_DRAWINDEXEDINDIRECT);
    // TODO: If the drawIndirectFirstInstance feature is not enabled, all the firstInstance members of the
    // VkDrawIndexedIndirectCommand structures accessed by this command must be 0, which will require access to the contents of
    // 'buffer'.
    return skip;
}

bool CoreChecks::PreCallValidateCmdDispatch(VkCommandBuffer commandBuffer, uint32_t x, uint32_t y, uint32_t z) const {
    bool skip = false;
    auto cb_state = GetRead<CMD_BUFFER_STATE>(commandBuffer);
    skip |= ValidateActionCmd(*cb_state, VK_PIPELINE_BIND_POINT_COMPUTE, CMD_DISPATCH);
    return skip;
}

bool CoreChecks::ValidateBaseGroups(const CMD_BUFFER_STATE &cb_state, uint32_t baseGroupX, uint32_t baseGroupY, uint32_t baseGroupZ,
                                    const char *apiName) const {
    bool skip = false;
    if (baseGroupX || baseGroupY || baseGroupZ) {
        const auto lv_bind_point = ConvertToLvlBindPoint(VK_PIPELINE_BIND_POINT_COMPUTE);
        const auto *pipeline_state = cb_state.lastBound[lv_bind_point].pipeline_state;
        if (pipeline_state && !(pipeline_state->create_flags & VK_PIPELINE_CREATE_DISPATCH_BASE)) {
            skip |= LogError(cb_state.Handle(), "VUID-vkCmdDispatchBase-baseGroupX-00427",
                             "%s(): If any of baseGroupX, baseGroupY, or baseGroupZ are not zero, then the bound compute pipeline "
                             "must have been created with the VK_PIPELINE_CREATE_DISPATCH_BASE flag",
                             apiName);
        }
    }
    return skip;
}

bool CoreChecks::PreCallValidateCmdDispatchBase(VkCommandBuffer commandBuffer, uint32_t baseGroupX, uint32_t baseGroupY,
                                                uint32_t baseGroupZ, uint32_t groupCountX, uint32_t groupCountY,
                                                uint32_t groupCountZ) const {
    bool skip = false;
    auto cb_state = GetRead<CMD_BUFFER_STATE>(commandBuffer);
    skip |= ValidateActionCmd(*cb_state, VK_PIPELINE_BIND_POINT_COMPUTE, CMD_DISPATCHBASE);
    skip |= ValidateBaseGroups(*cb_state, baseGroupX, baseGroupY, baseGroupZ, "vkCmdDispatchBase()");
    return skip;
}

bool CoreChecks::PreCallValidateCmdDispatchBaseKHR(VkCommandBuffer commandBuffer, uint32_t baseGroupX, uint32_t baseGroupY,
                                                   uint32_t baseGroupZ, uint32_t groupCountX, uint32_t groupCountY,
                                                   uint32_t groupCountZ) const {
    bool skip = false;
    auto cb_state = GetRead<CMD_BUFFER_STATE>(commandBuffer);
    skip |= ValidateActionCmd(*cb_state, VK_PIPELINE_BIND_POINT_COMPUTE, CMD_DISPATCHBASEKHR);
    skip |= ValidateBaseGroups(*cb_state, baseGroupX, baseGroupY, baseGroupZ, "vkCmdDispatchBaseKHR()");
    return skip;
}

bool CoreChecks::PreCallValidateCmdDispatchIndirect(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset) const {
    auto cb_state = GetRead<CMD_BUFFER_STATE>(commandBuffer);
    bool skip = ValidateActionCmd(*cb_state, VK_PIPELINE_BIND_POINT_COMPUTE, CMD_DISPATCHINDIRECT);
    auto buffer_state = Get<BUFFER_STATE>(buffer);
    skip |= ValidateIndirectCmd(*cb_state, *buffer_state, CMD_DISPATCHINDIRECT);
    if ((offset + sizeof(VkDispatchIndirectCommand)) > buffer_state->createInfo.size) {
        skip |= LogError(commandBuffer, "VUID-vkCmdDispatchIndirect-offset-00407",
                         "vkCmdDispatchIndirect(): The sum of offset and the size of VkDispatchIndirectCommand is greater than the "
                         "size of the buffer");
    }
    return skip;
}
bool CoreChecks::ValidateCmdDrawIndirectCount(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                              VkBuffer countBuffer, VkDeviceSize countBufferOffset, uint32_t maxDrawCount,
                                              uint32_t stride, CMD_TYPE cmd_type) const {
    bool skip = false;
    const char *apiName = CommandTypeString(cmd_type);
    if ((device_extensions.vk_khr_draw_indirect_count != kEnabledByCreateinfo) &&
        ((api_version >= VK_API_VERSION_1_2) && (enabled_features.core12.drawIndirectCount == VK_FALSE))) {
        skip |= LogError(commandBuffer, "VUID-vkCmdDrawIndirectCount-None-04445",
                         "%s(): Starting in Vulkan 1.2 the VkPhysicalDeviceVulkan12Features::drawIndirectCount must be enabled to "
                         "call this command.",
                         apiName);
    }
    skip |= ValidateCmdDrawStrideWithStruct(commandBuffer, "VUID-vkCmdDrawIndirectCount-stride-03110", stride,
                                            "VkDrawIndirectCommand", sizeof(VkDrawIndirectCommand));
    if (maxDrawCount > 1) {
        auto buffer_state = Get<BUFFER_STATE>(buffer);
        skip |= ValidateCmdDrawStrideWithBuffer(commandBuffer, "VUID-vkCmdDrawIndirectCount-maxDrawCount-03111", stride,
                                                "VkDrawIndirectCommand", sizeof(VkDrawIndirectCommand), maxDrawCount, offset,
                                                buffer_state.get());
    }

    auto cb_state = GetRead<CMD_BUFFER_STATE>(commandBuffer);
    skip |= ValidateActionCmd(*cb_state, VK_PIPELINE_BIND_POINT_GRAPHICS, cmd_type);
    auto buffer_state = Get<BUFFER_STATE>(buffer);
    skip |= ValidateIndirectCmd(*cb_state, *buffer_state, cmd_type);
    auto count_buffer_state = Get<BUFFER_STATE>(countBuffer);
    skip |= ValidateIndirectCountCmd(*cb_state, *count_buffer_state, countBufferOffset, cmd_type);
    skip |= ValidateVTGShaderStages(*cb_state, cmd_type);
    return skip;
}

bool CoreChecks::PreCallValidateCmdDrawIndirectCountKHR(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                        VkBuffer countBuffer, VkDeviceSize countBufferOffset, uint32_t maxDrawCount,
                                                        uint32_t stride) const {
    return ValidateCmdDrawIndirectCount(commandBuffer, buffer, offset, countBuffer, countBufferOffset, maxDrawCount, stride,
                                        CMD_DRAWINDIRECTCOUNTKHR);
}

bool CoreChecks::PreCallValidateCmdDrawIndirectCount(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                     VkBuffer countBuffer, VkDeviceSize countBufferOffset, uint32_t maxDrawCount,
                                                     uint32_t stride) const {
    return ValidateCmdDrawIndirectCount(commandBuffer, buffer, offset, countBuffer, countBufferOffset, maxDrawCount, stride,
                                        CMD_DRAWINDIRECTCOUNT);
}

bool CoreChecks::ValidateCmdDrawIndexedIndirectCount(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                     VkBuffer countBuffer, VkDeviceSize countBufferOffset, uint32_t maxDrawCount,
                                                     uint32_t stride, CMD_TYPE cmd_type) const {
    bool skip = false;
    const char *apiName = CommandTypeString(cmd_type);
    if ((device_extensions.vk_khr_draw_indirect_count != kEnabledByCreateinfo) &&
        ((api_version >= VK_API_VERSION_1_2) && (enabled_features.core12.drawIndirectCount == VK_FALSE))) {
        skip |= LogError(commandBuffer, "VUID-vkCmdDrawIndexedIndirectCount-None-04445",
                         "%s(): Starting in Vulkan 1.2 the VkPhysicalDeviceVulkan12Features::drawIndirectCount must be enabled to "
                         "call this command.",
                         apiName);
    }
    skip |= ValidateCmdDrawStrideWithStruct(commandBuffer, "VUID-vkCmdDrawIndexedIndirectCount-stride-03142", stride,
                                            "VkDrawIndexedIndirectCommand", sizeof(VkDrawIndexedIndirectCommand));
    auto buffer_state = Get<BUFFER_STATE>(buffer);
    if (maxDrawCount > 1) {
        skip |= ValidateCmdDrawStrideWithBuffer(commandBuffer, "VUID-vkCmdDrawIndexedIndirectCount-maxDrawCount-03143", stride,
                                                "VkDrawIndexedIndirectCommand", sizeof(VkDrawIndexedIndirectCommand), maxDrawCount,
                                                offset, buffer_state.get());
    }
    auto cb_state = GetRead<CMD_BUFFER_STATE>(commandBuffer);
    skip |= ValidateGraphicsIndexedCmd(*cb_state, cmd_type);
    skip |= ValidateActionCmd(*cb_state, VK_PIPELINE_BIND_POINT_GRAPHICS, cmd_type);
    skip |= ValidateIndirectCmd(*cb_state, *buffer_state, cmd_type);
    auto count_buffer_state = Get<BUFFER_STATE>(countBuffer);
    skip |= ValidateIndirectCountCmd(*cb_state, *count_buffer_state, countBufferOffset, cmd_type);
    skip |= ValidateVTGShaderStages(*cb_state, cmd_type);
    return skip;
}

bool CoreChecks::PreCallValidateCmdDrawIndexedIndirectCountKHR(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                               VkBuffer countBuffer, VkDeviceSize countBufferOffset,
                                                               uint32_t maxDrawCount, uint32_t stride) const {
    return ValidateCmdDrawIndexedIndirectCount(commandBuffer, buffer, offset, countBuffer, countBufferOffset, maxDrawCount, stride,
                                               CMD_DRAWINDEXEDINDIRECTCOUNTKHR);
}

bool CoreChecks::PreCallValidateCmdDrawIndexedIndirectCount(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                            VkBuffer countBuffer, VkDeviceSize countBufferOffset,
                                                            uint32_t maxDrawCount, uint32_t stride) const {
    return ValidateCmdDrawIndexedIndirectCount(commandBuffer, buffer, offset, countBuffer, countBufferOffset, maxDrawCount, stride,
                                               CMD_DRAWINDEXEDINDIRECTCOUNT);
}

bool CoreChecks::PreCallValidateCmdDrawIndirectByteCountEXT(VkCommandBuffer commandBuffer, uint32_t instanceCount,
                                                            uint32_t firstInstance, VkBuffer counterBuffer,
                                                            VkDeviceSize counterBufferOffset, uint32_t counterOffset,
                                                            uint32_t vertexStride) const {
    bool skip = false;
    if (!enabled_features.transform_feedback_features.transformFeedback) {
        skip |= LogError(commandBuffer, "VUID-vkCmdDrawIndirectByteCountEXT-transformFeedback-02287",
                         "%s: transformFeedback feature is not enabled.", "vkCmdDrawIndirectByteCountEXT()");
    }
    if (IsExtEnabled(device_extensions.vk_ext_transform_feedback) &&
        !phys_dev_ext_props.transform_feedback_props.transformFeedbackDraw) {
        skip |= LogError(commandBuffer, "VUID-vkCmdDrawIndirectByteCountEXT-transformFeedbackDraw-02288",
                         "%s: VkPhysicalDeviceTransformFeedbackPropertiesEXT::transformFeedbackDraw is not supported",
                         "vkCmdDrawIndirectByteCountEXT()");
    }
    auto cb_state = GetRead<CMD_BUFFER_STATE>(commandBuffer);
    skip |= ValidateCmdDrawInstance(*cb_state, instanceCount, firstInstance, CMD_DRAWINDIRECTBYTECOUNTEXT);
    skip |= ValidateActionCmd(*cb_state, VK_PIPELINE_BIND_POINT_GRAPHICS, CMD_DRAWINDIRECTBYTECOUNTEXT);
    auto counter_buffer_state = Get<BUFFER_STATE>(counterBuffer);
    skip |= ValidateIndirectCmd(*cb_state, *counter_buffer_state, CMD_DRAWINDIRECTBYTECOUNTEXT);
    skip |= ValidateVTGShaderStages(*cb_state, CMD_DRAWINDIRECTBYTECOUNTEXT);
    return skip;
}

bool CoreChecks::PreCallValidateCmdTraceRaysNV(VkCommandBuffer commandBuffer, VkBuffer raygenShaderBindingTableBuffer,
                                               VkDeviceSize raygenShaderBindingOffset, VkBuffer missShaderBindingTableBuffer,
                                               VkDeviceSize missShaderBindingOffset, VkDeviceSize missShaderBindingStride,
                                               VkBuffer hitShaderBindingTableBuffer, VkDeviceSize hitShaderBindingOffset,
                                               VkDeviceSize hitShaderBindingStride, VkBuffer callableShaderBindingTableBuffer,
                                               VkDeviceSize callableShaderBindingOffset, VkDeviceSize callableShaderBindingStride,
                                               uint32_t width, uint32_t height, uint32_t depth) const {
    auto cb_state = GetRead<CMD_BUFFER_STATE>(commandBuffer);
    bool skip = false;
    skip |= ValidateActionCmd(*cb_state, VK_PIPELINE_BIND_POINT_RAY_TRACING_NV, CMD_TRACERAYSNV);
    auto callable_shader_buffer_state = Get<BUFFER_STATE>(callableShaderBindingTableBuffer);
    if (callable_shader_buffer_state && callableShaderBindingOffset >= callable_shader_buffer_state->createInfo.size) {
        skip |= LogError(commandBuffer, "VUID-vkCmdTraceRaysNV-callableShaderBindingOffset-02461",
                         "vkCmdTraceRaysNV: callableShaderBindingOffset %" PRIu64
                         " must be less than the size of callableShaderBindingTableBuffer %" PRIu64 " .",
                         callableShaderBindingOffset, callable_shader_buffer_state->createInfo.size);
    }
    auto hit_shader_buffer_state = Get<BUFFER_STATE>(hitShaderBindingTableBuffer);
    if (hit_shader_buffer_state && hitShaderBindingOffset >= hit_shader_buffer_state->createInfo.size) {
        skip |= LogError(commandBuffer, "VUID-vkCmdTraceRaysNV-hitShaderBindingOffset-02459",
                         "vkCmdTraceRaysNV: hitShaderBindingOffset %" PRIu64
                         " must be less than the size of hitShaderBindingTableBuffer %" PRIu64 " .",
                         hitShaderBindingOffset, hit_shader_buffer_state->createInfo.size);
    }
    auto miss_shader_buffer_state = Get<BUFFER_STATE>(missShaderBindingTableBuffer);
    if (miss_shader_buffer_state && missShaderBindingOffset >= miss_shader_buffer_state->createInfo.size) {
        skip |= LogError(commandBuffer, "VUID-vkCmdTraceRaysNV-missShaderBindingOffset-02457",
                         "vkCmdTraceRaysNV: missShaderBindingOffset %" PRIu64
                         " must be less than the size of missShaderBindingTableBuffer %" PRIu64 " .",
                         missShaderBindingOffset, miss_shader_buffer_state->createInfo.size);
    }
    auto raygen_shader_buffer_state = Get<BUFFER_STATE>(raygenShaderBindingTableBuffer);
    if (raygenShaderBindingOffset >= raygen_shader_buffer_state->createInfo.size) {
        skip |= LogError(commandBuffer, "VUID-vkCmdTraceRaysNV-raygenShaderBindingOffset-02455",
                         "vkCmdTraceRaysNV: raygenShaderBindingOffset %" PRIu64
                         " must be less than the size of raygenShaderBindingTableBuffer %" PRIu64 " .",
                         raygenShaderBindingOffset, raygen_shader_buffer_state->createInfo.size);
    }
    return skip;
}

bool CoreChecks::ValidateCmdTraceRaysKHR(const CMD_TYPE cmd_type, const CMD_BUFFER_STATE &cb_state,
                                         const VkStridedDeviceAddressRegionKHR *pRaygenShaderBindingTable,
                                         const VkStridedDeviceAddressRegionKHR *pMissShaderBindingTable,
                                         const VkStridedDeviceAddressRegionKHR *pHitShaderBindingTable,
                                         const VkStridedDeviceAddressRegionKHR *pCallableShaderBindingTable) const {
    bool skip = false;
    const auto lv_bind_point = ConvertToLvlBindPoint(VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR);
    const PIPELINE_STATE *pipeline_state = cb_state.lastBound[lv_bind_point].pipeline_state;
    const bool is_indirect = cmd_type == CMD_TRACERAYSINDIRECTKHR;
    const char *rt_func_name = CommandTypeString(cmd_type);

    if (!pipeline_state || (pipeline_state && !pipeline_state->pipeline())) {
        return skip;
    }
    if (pHitShaderBindingTable) {
        if (pipeline_state->create_flags & VK_PIPELINE_CREATE_RAY_TRACING_NO_NULL_INTERSECTION_SHADERS_BIT_KHR) {
            if (pHitShaderBindingTable->deviceAddress == 0) {
                const char *vuid =
                    is_indirect ? "VUID-vkCmdTraceRaysIndirectKHR-flags-03697" : "VUID-vkCmdTraceRaysKHR-flags-03697";
                skip |= LogError(cb_state.commandBuffer(), vuid, "%s: pHitShaderBindingTable->deviceAddress (0).", rt_func_name);
            }
            if ((pHitShaderBindingTable->size == 0 || pHitShaderBindingTable->stride == 0)) {
                const char *vuid =
                    is_indirect ? "VUID-vkCmdTraceRaysIndirectKHR-flags-03514" : "VUID-vkCmdTraceRaysKHR-flags-03514";
                skip |= LogError(cb_state.commandBuffer(), vuid,
                                 "%s: pHitShaderBindingTable->size (%" PRIu64 ") and pHitShaderBindingTable->stride (%" PRIu64 ").",
                                 rt_func_name, pHitShaderBindingTable->size, pHitShaderBindingTable->stride);
            }
        }
        if (pipeline_state->create_flags & VK_PIPELINE_CREATE_RAY_TRACING_NO_NULL_CLOSEST_HIT_SHADERS_BIT_KHR) {
            if (pHitShaderBindingTable->deviceAddress == 0) {
                const char *vuid =
                    is_indirect ? "VUID-vkCmdTraceRaysIndirectKHR-flags-03696" : "VUID-vkCmdTraceRaysKHR-flags-03696";
                skip |= LogError(cb_state.commandBuffer(), vuid, "pHitShaderBindingTable->deviceAddress = 0");
            }
            if ((pHitShaderBindingTable->size == 0 || pHitShaderBindingTable->stride == 0)) {
                const char *vuid =
                    is_indirect ? "VUID-vkCmdTraceRaysIndirectKHR-flags-03513" : "VUID-vkCmdTraceRaysKHR-flags-03513";
                skip |= LogError(cb_state.commandBuffer(), vuid,
                                 "%s: pHitShaderBindingTable->size (%" PRIu64 ") and pHitShaderBindingTable->stride (%" PRIu64 ").",
                                 rt_func_name, pHitShaderBindingTable->size, pHitShaderBindingTable->stride);
            }
        }
        if (pipeline_state->create_flags & VK_PIPELINE_CREATE_RAY_TRACING_NO_NULL_ANY_HIT_SHADERS_BIT_KHR) {
            // No vuid to check for pHitShaderBindingTable->deviceAddress == 0 with this flag

            if (pHitShaderBindingTable->size == 0 || pHitShaderBindingTable->stride == 0) {
                const char *vuid =
                    is_indirect ? "VUID-vkCmdTraceRaysIndirectKHR-flags-03512" : "VUID-vkCmdTraceRaysKHR-flags-03512";
                skip |= LogError(cb_state.commandBuffer(), vuid,
                                 "%s: pHitShaderBindingTable->size (%" PRIu64 ") and pHitShaderBindingTable->stride (%" PRIu64 ").",
                                 rt_func_name, pHitShaderBindingTable->size, pHitShaderBindingTable->stride);
            }
        }

        const char *vuid_single_device_memory = is_indirect ? "VUID-vkCmdTraceRaysIndirectKHR-pHitShaderBindingTable-03687"
                                                            : "VUID-vkCmdTraceRaysKHR-pHitShaderBindingTable-03687";
        const char *vuid_binding_table_flag = is_indirect ? "VUID-vkCmdTraceRaysIndirectKHR-pHitShaderBindingTable-03688"
                                                          : "VUID-vkCmdTraceRaysKHR-pHitShaderBindingTable-03688";
        skip |= ValidateRaytracingShaderBindingTable(cb_state.commandBuffer(), rt_func_name, vuid_single_device_memory,
                                                     vuid_binding_table_flag, *pHitShaderBindingTable, "pHitShaderBindingTable");
    }

    if (pRaygenShaderBindingTable) {
        const char *vuid_single_device_memory = is_indirect ? "VUID-vkCmdTraceRaysIndirectKHR-pRayGenShaderBindingTable-03680"
                                                            : "VUID-vkCmdTraceRaysKHR-pRayGenShaderBindingTable-03680";
        const char *vuid_binding_table_flag = is_indirect ? "VUID-vkCmdTraceRaysIndirectKHR-pRayGenShaderBindingTable-03681"
                                                          : "VUID-vkCmdTraceRaysKHR-pRayGenShaderBindingTable-03681";
        skip |=
            ValidateRaytracingShaderBindingTable(cb_state.commandBuffer(), rt_func_name, vuid_single_device_memory,
                                                 vuid_binding_table_flag, *pRaygenShaderBindingTable, "pRaygenShaderBindingTable");
    }

    if (pMissShaderBindingTable) {
        const char *vuid_single_device_memory = is_indirect ? "VUID-vkCmdTraceRaysIndirectKHR-pMissShaderBindingTable-03683"
                                                            : "VUID-vkCmdTraceRaysKHR-pMissShaderBindingTable-03683";
        const char *vuid_binding_table_flag = is_indirect ? "VUID-vkCmdTraceRaysIndirectKHR-pMissShaderBindingTable-03684"
                                                          : "VUID-vkCmdTraceRaysKHR-pMissShaderBindingTable-03684";
        skip |= ValidateRaytracingShaderBindingTable(cb_state.commandBuffer(), rt_func_name, vuid_single_device_memory,
                                                     vuid_binding_table_flag, *pMissShaderBindingTable, "pMissShaderBindingTable");
    }

    if (pCallableShaderBindingTable) {
        const char *vuid_single_device_memory = is_indirect ? "VUID-vkCmdTraceRaysIndirectKHR-pCallableShaderBindingTable-03691"
                                                            : "VUID-vkCmdTraceRaysKHR-pCallableShaderBindingTable-03691";
        const char *vuid_binding_table_flag = is_indirect ? "VUID-vkCmdTraceRaysIndirectKHR-pCallableShaderBindingTable-03692"
                                                          : "VUID-vkCmdTraceRaysKHR-pCallableShaderBindingTable-03692";
        skip |= ValidateRaytracingShaderBindingTable(cb_state.commandBuffer(), rt_func_name, vuid_single_device_memory,
                                                     vuid_binding_table_flag, *pCallableShaderBindingTable,
                                                     "pCallableShaderBindingTable");
    }
    return skip;
}

bool CoreChecks::PreCallValidateCmdTraceRaysKHR(VkCommandBuffer commandBuffer,
                                                const VkStridedDeviceAddressRegionKHR *pRaygenShaderBindingTable,
                                                const VkStridedDeviceAddressRegionKHR *pMissShaderBindingTable,
                                                const VkStridedDeviceAddressRegionKHR *pHitShaderBindingTable,
                                                const VkStridedDeviceAddressRegionKHR *pCallableShaderBindingTable, uint32_t width,
                                                uint32_t height, uint32_t depth) const {
    bool skip = false;
    auto cb_state = GetRead<CMD_BUFFER_STATE>(commandBuffer);
    skip |= ValidateActionCmd(*cb_state, VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR, CMD_TRACERAYSKHR);
    skip |= ValidateCmdTraceRaysKHR(CMD_TRACERAYSKHR, *cb_state, pRaygenShaderBindingTable, pMissShaderBindingTable,
                                    pHitShaderBindingTable, pCallableShaderBindingTable);
    return skip;
}

bool CoreChecks::PreCallValidateCmdTraceRaysIndirectKHR(VkCommandBuffer commandBuffer,
                                                        const VkStridedDeviceAddressRegionKHR *pRaygenShaderBindingTable,
                                                        const VkStridedDeviceAddressRegionKHR *pMissShaderBindingTable,
                                                        const VkStridedDeviceAddressRegionKHR *pHitShaderBindingTable,
                                                        const VkStridedDeviceAddressRegionKHR *pCallableShaderBindingTable,
                                                        VkDeviceAddress indirectDeviceAddress) const {
    bool skip = false;
    auto cb_state = GetRead<CMD_BUFFER_STATE>(commandBuffer);
    skip |= ValidateActionCmd(*cb_state, VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR, CMD_TRACERAYSINDIRECTKHR);
    skip |= ValidateCmdTraceRaysKHR(CMD_TRACERAYSINDIRECTKHR, *cb_state, pRaygenShaderBindingTable, pMissShaderBindingTable,
                                    pHitShaderBindingTable, pCallableShaderBindingTable);
    return skip;
}

bool CoreChecks::PreCallValidateCmdTraceRaysIndirect2KHR(VkCommandBuffer commandBuffer,
                                                         VkDeviceAddress indirectDeviceAddress) const {
    auto cb_state = GetRead<CMD_BUFFER_STATE>(commandBuffer);
    bool skip = ValidateActionCmd(*cb_state, VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR, CMD_TRACERAYSINDIRECT2KHR);
    return skip;
}

bool CoreChecks::PreCallValidateCmdDrawMeshTasksNV(VkCommandBuffer commandBuffer, uint32_t taskCount, uint32_t firstTask) const {
    auto cb_state = GetRead<CMD_BUFFER_STATE>(commandBuffer);
    bool skip = ValidateActionCmd(*cb_state, VK_PIPELINE_BIND_POINT_GRAPHICS, CMD_DRAWMESHTASKSNV);
    skip |= ValidateMeshShaderStage(*cb_state, CMD_DRAWMESHTASKSNV, true);
    return skip;
}

bool CoreChecks::PreCallValidateCmdDrawMeshTasksIndirectNV(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                           uint32_t drawCount, uint32_t stride) const {
    auto cb_state = GetRead<CMD_BUFFER_STATE>(commandBuffer);
    bool skip = ValidateActionCmd(*cb_state, VK_PIPELINE_BIND_POINT_GRAPHICS, CMD_DRAWMESHTASKSINDIRECTNV);
    auto buffer_state = Get<BUFFER_STATE>(buffer);
    skip |= ValidateIndirectCmd(*cb_state, *buffer_state, CMD_DRAWMESHTASKSINDIRECTNV);
    if (drawCount > 1) {
        skip |= ValidateCmdDrawStrideWithBuffer(commandBuffer, "VUID-vkCmdDrawMeshTasksIndirectNV-drawCount-02157", stride,
                                                "VkDrawMeshTasksIndirectCommandNV", sizeof(VkDrawMeshTasksIndirectCommandNV),
                                                drawCount, offset, buffer_state.get());
    } else if (drawCount == 1 && ((offset + sizeof(VkDrawMeshTasksIndirectCommandNV)) > buffer_state.get()->createInfo.size)) {
        skip |=
            LogError(device, "VUID-vkCmdDrawMeshTasksIndirectNV-drawCount-02156",
                     "(offset + sizeof(VkDrawMeshTasksIndirectNV)) (%" PRIu64 ") is greater than the size of buffer (%" PRIu64 ").",
                     offset + sizeof(VkDrawMeshTasksIndirectCommandNV), buffer_state->createInfo.size);
    }
    skip |= ValidateMeshShaderStage(*cb_state, CMD_DRAWMESHTASKSINDIRECTNV, true);
    return skip;
}

bool CoreChecks::PreCallValidateCmdDrawMeshTasksIndirectCountNV(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                                VkBuffer countBuffer, VkDeviceSize countBufferOffset,
                                                                uint32_t maxDrawCount, uint32_t stride) const {
    auto cb_state = GetRead<CMD_BUFFER_STATE>(commandBuffer);
    bool skip = ValidateActionCmd(*cb_state, VK_PIPELINE_BIND_POINT_GRAPHICS, CMD_DRAWMESHTASKSINDIRECTCOUNTNV);
    auto buffer_state = Get<BUFFER_STATE>(buffer);
    auto count_buffer_state = Get<BUFFER_STATE>(countBuffer);
    skip |= ValidateIndirectCmd(*cb_state, *buffer_state, CMD_DRAWMESHTASKSINDIRECTCOUNTNV);
    skip |= ValidateIndirectCountCmd(*cb_state, *count_buffer_state, countBufferOffset, CMD_DRAWMESHTASKSINDIRECTCOUNTNV);
    skip |= ValidateCmdDrawStrideWithStruct(commandBuffer, "VUID-vkCmdDrawMeshTasksIndirectCountNV-stride-02182", stride,
                                            "VkDrawMeshTasksIndirectCommandNV", sizeof(VkDrawMeshTasksIndirectCommandNV));
    if (maxDrawCount > 1) {
        skip |= ValidateCmdDrawStrideWithBuffer(commandBuffer, "VUID-vkCmdDrawMeshTasksIndirectCountNV-maxDrawCount-02183", stride,
                                                "VkDrawMeshTasksIndirectCommandNV", sizeof(VkDrawMeshTasksIndirectCommandNV),
                                                maxDrawCount, offset, buffer_state.get());
    }
    skip |= ValidateMeshShaderStage(*cb_state, CMD_DRAWMESHTASKSINDIRECTCOUNTNV, true);
    return skip;
}

bool CoreChecks::PreCallValidateCmdDrawMeshTasksEXT(VkCommandBuffer commandBuffer, uint32_t groupCountX, uint32_t groupCountY,
                                                    uint32_t groupCountZ) const {
    auto cb_state = GetRead<CMD_BUFFER_STATE>(commandBuffer);
    bool skip = ValidateActionCmd(*cb_state, VK_PIPELINE_BIND_POINT_GRAPHICS, CMD_DRAWMESHTASKSEXT);
    skip |= ValidateMeshShaderStage(*cb_state, CMD_DRAWMESHTASKSEXT, false);
    return skip;
}

bool CoreChecks::PreCallValidateCmdDrawMeshTasksIndirectEXT(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                            uint32_t drawCount, uint32_t stride) const {
    auto cb_state = GetRead<CMD_BUFFER_STATE>(commandBuffer);
    bool skip = ValidateActionCmd(*cb_state, VK_PIPELINE_BIND_POINT_GRAPHICS, CMD_DRAWMESHTASKSINDIRECTEXT);
    auto buffer_state = Get<BUFFER_STATE>(buffer);
    skip |= ValidateIndirectCmd(*cb_state, *buffer_state, CMD_DRAWMESHTASKSINDIRECTEXT);
    if (drawCount > 1) {
        skip |= ValidateCmdDrawStrideWithStruct(commandBuffer, "VUID-vkCmdDrawMeshTasksIndirectEXT-drawCount-07088", stride,
                                                "VkDrawMeshTasksIndirectCommandEXT", sizeof(VkDrawMeshTasksIndirectCommandEXT));
        skip |= ValidateCmdDrawStrideWithBuffer(commandBuffer, "VUID-vkCmdDrawMeshTasksIndirectEXT-drawCount-07090", stride,
                                                "VkDrawMeshTasksIndirectCommandEXT", sizeof(VkDrawMeshTasksIndirectCommandEXT),
                                                drawCount, offset, buffer_state.get());
    }
    if ((drawCount == 1) && (offset + sizeof(VkDrawMeshTasksIndirectCommandEXT)) > buffer_state->createInfo.size) {
        skip |= LogError(
            commandBuffer, "VUID-vkCmdDrawMeshTasksIndirectEXT-drawCount-07089",
            "vkCmdDrawMeshTasksIndirectEXT: drawCount equals 1 and (offset + sizeof(vkCmdDrawMeshTasksIndirectEXT)) (%" PRIu64
            ") is not less than "
            "or equal to the size of buffer (%" PRIu64 ").",
            (offset + sizeof(VkDrawMeshTasksIndirectCommandEXT)), buffer_state->createInfo.size);
    }
    skip |= ValidateMeshShaderStage(*cb_state, CMD_DRAWMESHTASKSINDIRECTEXT, false);
    return skip;
}

bool CoreChecks::PreCallValidateCmdDrawMeshTasksIndirectCountEXT(VkCommandBuffer commandBuffer, VkBuffer buffer,
                                                                 VkDeviceSize offset, VkBuffer countBuffer,
                                                                 VkDeviceSize countBufferOffset, uint32_t maxDrawCount,
                                                                 uint32_t stride) const {
    const CMD_TYPE cmd_type = CMD_DRAWMESHTASKSINDIRECTCOUNTEXT;
    const DrawDispatchVuid &vuid = GetDrawDispatchVuid(cmd_type);
    const char *caller_name = CommandTypeString(cmd_type);

    auto cb_state = GetRead<CMD_BUFFER_STATE>(commandBuffer);
    bool skip = ValidateActionCmd(*cb_state, VK_PIPELINE_BIND_POINT_GRAPHICS, cmd_type);
    auto buffer_state = Get<BUFFER_STATE>(buffer);
    auto count_buffer_state = Get<BUFFER_STATE>(countBuffer);
    skip |= ValidateIndirectCmd(*cb_state, *buffer_state, cmd_type);
    skip |=
        ValidateMemoryIsBoundToBuffer(commandBuffer, *count_buffer_state, caller_name, vuid.indirect_count_contiguous_memory_02714);
    skip |= ValidateBufferUsageFlags(commandBuffer, *count_buffer_state, VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT, true,
                                     vuid.indirect_count_buffer_bit_02715, "vkCmdDrawMeshTasksIndirectCountEXT()",
                                     "VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT");
    skip |= ValidateCmdDrawStrideWithStruct(commandBuffer, "VUID-vkCmdDrawMeshTasksIndirectCountEXT-stride-07096", stride,
                                            "VkDrawMeshTasksIndirectCommandEXT", sizeof(VkDrawMeshTasksIndirectCommandEXT));
    if (maxDrawCount > 1) {
        skip |= ValidateCmdDrawStrideWithBuffer(commandBuffer, "VUID-vkCmdDrawMeshTasksIndirectCountEXT-maxDrawCount-07097", stride,
                                                "VkDrawMeshTasksIndirectCommandEXT", sizeof(VkDrawMeshTasksIndirectCommandEXT),
                                                maxDrawCount, offset, buffer_state.get());
    }
    skip |= ValidateMeshShaderStage(*cb_state, cmd_type, false);
    return skip;
}

// Validate overall state at the time of a draw call
bool CoreChecks::ValidateCmdBufDrawState(const CMD_BUFFER_STATE &cb_state, CMD_TYPE cmd_type,
                                         const VkPipelineBindPoint bind_point) const {
    const DrawDispatchVuid &vuid = GetDrawDispatchVuid(cmd_type);
    const char *function = CommandTypeString(cmd_type);
    const auto lv_bind_point = ConvertToLvlBindPoint(bind_point);
    const auto &last_bound_state = cb_state.lastBound[lv_bind_point];
    const auto *last_pipeline = last_bound_state.pipeline_state;

    if (!last_pipeline || !last_pipeline->pipeline()) {
        // For now, don't validate anything and just return
        // See https://github.com/KhronosGroup/Vulkan-ValidationLayers/issues/5580
        if (enabled_features.shader_object_features.shaderObject) {
            return false;
        }
        return LogError(cb_state.commandBuffer(), vuid.pipeline_bound_02700,
                        "%s: A valid %s pipeline must be bound with vkCmdBindPipeline before calling this command.", function,
                        string_VkPipelineBindPoint(bind_point));
    }
    const PIPELINE_STATE &pipeline = *last_pipeline;

    bool skip = false;

    for (const auto &ds : last_bound_state.per_set) {
        if (pipeline.descriptor_buffer_mode) {
            if (ds.bound_descriptor_set && !ds.bound_descriptor_set->IsPushDescriptor()) {
                const LogObjectList objlist(cb_state.Handle(), pipeline.Handle(), ds.bound_descriptor_set->Handle());
                skip |= LogError(objlist, vuid.descriptor_buffer_set_offset_missing_08117,
                                 "%s: pipeline bound to %s requires a descriptor buffer but has a bound descriptor set (%s)",
                                 function, string_VkPipelineBindPoint(bind_point),
                                 report_data->FormatHandle(ds.bound_descriptor_set->Handle()).c_str());
                break;
            }

        } else {
            if (ds.bound_descriptor_buffer.has_value()) {
                const LogObjectList objlist(cb_state.Handle(), pipeline.Handle());
                skip |= LogError(objlist, vuid.descriptor_buffer_bit_not_set_08115,
                                 "%s: pipeline bound to %s requires a descriptor set but has a bound descriptor buffer"
                                 " (index=%" PRIu32 " offset=%" PRIu64 ")",
                                 function, string_VkPipelineBindPoint(bind_point), ds.bound_descriptor_buffer->index,
                                 ds.bound_descriptor_buffer->offset);
                break;
            }
        }
    }

    if (VK_PIPELINE_BIND_POINT_GRAPHICS == bind_point) {
        skip |= ValidateDrawDynamicState(last_bound_state, cmd_type);
        skip |= ValidatePipelineDrawtimeState(last_bound_state, cmd_type);

        if (cb_state.activeRenderPass && cb_state.activeFramebuffer) {
            // Verify attachments for unprotected/protected command buffer.
            if (enabled_features.core11.protectedMemory == VK_TRUE && cb_state.active_attachments) {
                uint32_t i = 0;
                for (const auto &view_state : *cb_state.active_attachments.get()) {
                    const auto &subpass = cb_state.active_subpasses->at(i);
                    if (subpass.used && view_state && !view_state->Destroyed()) {
                        std::string image_desc = "Image is ";
                        image_desc.append(string_VkImageUsageFlagBits(subpass.usage));
                        // Because inputAttachment is read only, it doesn't need to care protected command buffer case.
                        // Some CMD_TYPE could not be protected. See VUID 02711.
                        if (subpass.usage != VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT &&
                            vuid.protected_command_buffer_02712 != kVUIDUndefined) {
                            skip |= ValidateUnprotectedImage(cb_state, *view_state->image_state, function,
                                                             vuid.protected_command_buffer_02712, image_desc.c_str());
                        }
                        skip |= ValidateProtectedImage(cb_state, *view_state->image_state, function,
                                                       vuid.unprotected_command_buffer_02707, image_desc.c_str());
                    }
                    ++i;
                }
            }
        }
    }
    // Now complete other state checks
    std::string error_string;
    auto const &pipeline_layout = pipeline.PipelineLayoutState();

    // Check if the current pipeline is compatible for the maximum used set with the bound sets.
    if (!pipeline.descriptor_buffer_mode) {
        if (!pipeline.active_slots.empty() && !IsBoundSetCompat(pipeline.max_active_slot, last_bound_state, *pipeline_layout)) {
            LogObjectList objlist(pipeline.pipeline());
            const auto layouts = pipeline.PipelineLayoutStateUnion();
            std::ostringstream pipe_layouts_log;
            if (layouts.size() > 1) {
                pipe_layouts_log << "a union of layouts [ ";
                for (const auto &layout : layouts) {
                    objlist.add(layout->layout());
                    pipe_layouts_log << report_data->FormatHandle(layout->layout()) << " ";
                }
                pipe_layouts_log << "]";
            } else {
                pipe_layouts_log << report_data->FormatHandle(layouts.front()->layout());
            }
            objlist.add(last_bound_state.pipeline_layout);
            skip |= LogError(objlist, vuid.compatible_pipeline_02697,
                             "%s(): The %s (created with %s) statically uses descriptor set (index #%" PRIu32
                             ") which is not compatible with the currently bound descriptor set's pipeline layout (%s)",
                             function, report_data->FormatHandle(pipeline.pipeline()).c_str(), pipe_layouts_log.str().c_str(),
                             pipeline.max_active_slot, report_data->FormatHandle(last_bound_state.pipeline_layout).c_str());
        } else {
            // if the bound set is not copmatible, the rest will just be extra redundant errors
            for (const auto &set_binding_pair : pipeline.active_slots) {
                uint32_t set_index = set_binding_pair.first;
                const auto set_info = last_bound_state.per_set[set_index];
                if (!set_info.bound_descriptor_set) {
                    skip |= LogError(cb_state.commandBuffer(), vuid.compatible_pipeline_02697,
                                     "%s(): %s uses set #%" PRIu32 " but that set is not bound.", function,
                                     report_data->FormatHandle(pipeline.pipeline()).c_str(), set_index);
                } else if (!VerifySetLayoutCompatibility(*set_info.bound_descriptor_set, *pipeline_layout, set_index,
                                                         error_string)) {
                    // Set is bound but not compatible w/ overlapping pipeline_layout from PSO
                    VkDescriptorSet set_handle = set_info.bound_descriptor_set->GetSet();
                    const LogObjectList objlist(set_handle, pipeline_layout->layout());
                    skip |= LogError(objlist, vuid.compatible_pipeline_02697,
                                     "%s(): %s bound as set #%u is not compatible with overlapping %s due to: %s", function,
                                     report_data->FormatHandle(set_handle).c_str(), set_index,
                                     report_data->FormatHandle(pipeline_layout->layout()).c_str(), error_string.c_str());
                } else {  // Valid set is bound and layout compatible, validate that it's updated
                    // Pull the set node
                    const auto *descriptor_set = set_info.bound_descriptor_set.get();
                    assert(descriptor_set);
                    // Validate the draw-time state for this descriptor set
                    std::string err_str;
                    // For the "bindless" style resource usage with many descriptors, need to optimize command <-> descriptor
                    // binding validation. Take the requested binding set and prefilter it to eliminate redundant validation checks.
                    // Here, the currently bound pipeline determines whether an image validation check is redundant...
                    // for images are the "req" portion of the binding_req is indirectly (but tightly) coupled to the pipeline.
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
                    bool need_validate =
                        descriptor_set_changed ||
                        // Revalidate if previous bindingReqMap doesn't include new bindingReqMap
                        !std::includes(set_info.validated_set_binding_req_map.begin(), set_info.validated_set_binding_req_map.end(),
                                       binding_req_map.begin(), binding_req_map.end());

                    if (need_validate) {
                        if (!descriptor_set_changed && reduced_map.IsManyDescriptors()) {
                            // Only validate the bindings that haven't already been validated
                            BindingVariableMap delta_reqs;
                            std::set_difference(binding_req_map.begin(), binding_req_map.end(),
                                                set_info.validated_set_binding_req_map.begin(),
                                                set_info.validated_set_binding_req_map.end(),
                                                vvl::insert_iterator<BindingVariableMap>(delta_reqs, delta_reqs.begin()));
                            skip |=
                                ValidateDrawState(*descriptor_set, delta_reqs, set_info.dynamicOffsets, cb_state, function, vuid);
                        } else {
                            skip |= ValidateDrawState(*descriptor_set, binding_req_map, set_info.dynamicOffsets, cb_state, function,
                                                      vuid);
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
    if (!cb_state.push_constant_data_ranges || (pipeline_layout->push_constant_ranges == cb_state.push_constant_data_ranges)) {
        for (const auto &stage : pipeline.stage_states) {
            if (!stage.entrypoint || !stage.entrypoint->push_constant_variable) {
                continue;  // no static push constant in shader
            }

            // Edge case where if the shader is using push constants statically and there never was a vkCmdPushConstants
            if (!cb_state.push_constant_data_ranges && !enabled_features.core13.maintenance4) {
                const LogObjectList objlist(cb_state.commandBuffer(), pipeline_layout->layout(), pipeline.pipeline());
                skip |= LogError(objlist, vuid.push_constants_set_06425,
                                 "%s(): Shader in %s uses push-constant statically but vkCmdPushConstants was not called yet for "
                                 "pipeline layout %s.",
                                 function, string_VkShaderStageFlags(stage.create_info->stage).c_str(),
                                 report_data->FormatHandle(pipeline_layout->layout()).c_str());
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

bool CoreChecks::ValidateIndirectCmd(const CMD_BUFFER_STATE &cb_state, const BUFFER_STATE &buffer_state, CMD_TYPE cmd_type) const {
    bool skip = false;
    const DrawDispatchVuid &vuid = GetDrawDispatchVuid(cmd_type);
    const char *caller_name = CommandTypeString(cmd_type);

    skip |=
        ValidateMemoryIsBoundToBuffer(cb_state.commandBuffer(), buffer_state, caller_name, vuid.indirect_contiguous_memory_02708);
    skip |= ValidateBufferUsageFlags(cb_state.commandBuffer(), buffer_state, VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT, true,
                                     vuid.indirect_buffer_bit_02290, caller_name, "VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT");
    if (cb_state.unprotected == false) {
        skip |= LogError(cb_state.Handle(), vuid.indirect_protected_cb_02646,
                         "%s: Indirect commands can't be used in protected command buffers.", caller_name);
    }
    return skip;
}

bool CoreChecks::ValidateIndirectCountCmd(const CMD_BUFFER_STATE &cb_state, const BUFFER_STATE &count_buffer_state,
                                          VkDeviceSize count_buffer_offset, CMD_TYPE cmd_type) const {
    bool skip = false;
    const DrawDispatchVuid &vuid = GetDrawDispatchVuid(cmd_type);
    const char *caller_name = CommandTypeString(cmd_type);

    skip |= ValidateMemoryIsBoundToBuffer(cb_state.commandBuffer(), count_buffer_state, caller_name,
                                          vuid.indirect_count_contiguous_memory_02714);
    skip |= ValidateBufferUsageFlags(cb_state.commandBuffer(), count_buffer_state, VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT, true,
                                     vuid.indirect_count_buffer_bit_02715, caller_name, "VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT");
    if (count_buffer_offset + sizeof(uint32_t) > count_buffer_state.createInfo.size) {
        const LogObjectList objlist(cb_state.Handle(), count_buffer_state.Handle());
        skip |= LogError(objlist, vuid.indirect_count_offset_04129,
                         "%s: countBufferOffset (%" PRIu64 ") + sizeof(uint32_t) is greater than the buffer size of %" PRIu64 ".",
                         caller_name, count_buffer_offset, count_buffer_state.createInfo.size);
    }
    return skip;
}
