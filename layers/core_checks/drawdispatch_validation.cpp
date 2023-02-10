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

#include "chassis.h"
#include "core_checks/core_validation.h"

// clang-format off
struct DispatchVuidsCmdDraw : DrawDispatchVuid {
    DispatchVuidsCmdDraw() : DrawDispatchVuid() {
        pipeline_bound                     = "VUID-vkCmdDraw-None-02700";
        vertex_binding                     = "VUID-vkCmdDraw-None-04007";
        vertex_binding_null                = "VUID-vkCmdDraw-None-04008";
        compatible_pipeline                = "VUID-vkCmdDraw-None-02697";
        render_pass_compatible             = "VUID-vkCmdDraw-renderPass-02684";
        subpass_index                      = "VUID-vkCmdDraw-subpass-02685";
        sample_location                    = "VUID-vkCmdDraw-sampleLocationsEnable-02689";
        linear_filter_sampler              = "VUID-vkCmdDraw-magFilter-04553";
        linear_mipmap_sampler              = "VUID-vkCmdDraw-mipmapMode-04770";
        cubic_sampler                      = "VUID-vkCmdDraw-None-02692";
        viewport_count                     = "VUID-vkCmdDraw-viewportCount-03417";
        scissor_count                      = "VUID-vkCmdDraw-scissorCount-03418";
        viewport_scissor_count             = "VUID-vkCmdDraw-viewportCount-03419";
        primitive_topology_class           = "VUID-vkCmdDraw-primitiveTopology-03420";
        primitive_topology_class_ds3       = "VUID-vkCmdDraw-dynamicPrimitiveTopologyUnrestricted-07500";
        dynamic_primitive_topology         = "VUID-vkCmdDraw-None-07842";
        corner_sampled_address_mode        = "VUID-vkCmdDraw-flags-02696";
        imageview_atomic                   = "VUID-vkCmdDraw-None-02691";
        push_constants_set                 = "VUID-vkCmdDraw-maintenance4-06425";
        image_subresources_render_pass_write = "VUID-vkCmdDraw-None-06537";
        image_subresources_subpass_read    = "VUID-vkCmdDraw-None-06538";
        image_subresources_subpass_write   = "VUID-vkCmdDraw-None-06539";
        descriptor_valid                   = "VUID-vkCmdDraw-None-02699";
        sampler_imageview_type             = "VUID-vkCmdDraw-None-02702";
        sampler_implicitLod_dref_proj      = "VUID-vkCmdDraw-None-02703";
        sampler_bias_offset                = "VUID-vkCmdDraw-None-02704";
        vertex_binding_attribute           = "VUID-vkCmdDraw-None-02721";
        dynamic_state_setting_commands     = "VUID-vkCmdDraw-None-02859";
        rasterization_samples              = "VUID-vkCmdDraw-rasterizationSamples-04740";
        msrtss_rasterization_samples       = "VUID-vkCmdDraw-multisampledRenderToSingleSampled-07284";
        unprotected_command_buffer         = "VUID-vkCmdDraw-commandBuffer-02707";
        protected_command_buffer           = "VUID-vkCmdDraw-commandBuffer-02712";
        max_multiview_instance_index       = "VUID-vkCmdDraw-maxMultiviewInstanceIndex-02688";
        img_filter_cubic                   = "VUID-vkCmdDraw-None-02693";
        filter_cubic                       = "VUID-vkCmdDraw-filterCubic-02694";
        filter_cubic_min_max               = "VUID-vkCmdDraw-filterCubicMinmax-02695";
        viewport_count_primitive_shading_rate = "VUID-vkCmdDraw-primitiveFragmentShadingRateWithMultipleViewports-04552";
        patch_control_points               = "VUID-vkCmdDraw-None-04875";
        rasterizer_discard_enable          = "VUID-vkCmdDraw-None-04876";
        depth_bias_enable                  = "VUID-vkCmdDraw-None-04877";
        logic_op                           = "VUID-vkCmdDraw-logicOp-04878";
        primitive_restart_enable           = "VUID-vkCmdDraw-None-04879";
        vertex_input_binding_stride        = "VUID-vkCmdDraw-pStrides-04884";
        vertex_input                       = "VUID-vkCmdDraw-None-04914";
        blend_enable                       = "VUID-vkCmdDraw-blendEnable-04727";
        dynamic_discard_rectangle          = "VUID-vkCmdDraw-None-07751";
        dynamic_color_write_enable         = "VUID-vkCmdDraw-None-07749";
        dynamic_color_write_enable_count   = "VUID-vkCmdDraw-attachmentCount-07750";
        dynamic_rendering_view_mask        = "VUID-vkCmdDraw-viewMask-06178";
        dynamic_rendering_color_count      = "VUID-vkCmdDraw-colorAttachmentCount-06179";
        dynamic_rendering_color_formats    = "VUID-vkCmdDraw-colorAttachmentCount-06180";
        dynamic_rendering_depth_format     = "VUID-vkCmdDraw-pDepthAttachment-06181";
        dynamic_rendering_stencil_format   = "VUID-vkCmdDraw-pStencilAttachment-06182";
        dynamic_rendering_fsr              = "VUID-vkCmdDraw-imageView-06183";
        dynamic_rendering_fdm              = "VUID-vkCmdDraw-imageView-06184";
        dynamic_rendering_color_sample     = "VUID-vkCmdDraw-colorAttachmentCount-06185";
        dynamic_rendering_depth_sample     = "VUID-vkCmdDraw-pDepthAttachment-06186";
        dynamic_rendering_stencil_sample   = "VUID-vkCmdDraw-pStencilAttachment-06187";
        dynamic_rendering_multi_sample     = "VUID-vkCmdDraw-colorAttachmentCount-06188";
        dynamic_rendering_06189            = "VUID-vkCmdDraw-pDepthAttachment-06189";
        dynamic_rendering_06190            = "VUID-vkCmdDraw-pStencilAttachment-06190";
        dynamic_rendering_06198            = "VUID-vkCmdDraw-renderPass-06198";
        dynamic_rendering_07285            = "VUID-vkCmdDraw-multisampledRenderToSingleSampled-07285";
        dynamic_rendering_07286            = "VUID-vkCmdDraw-multisampledRenderToSingleSampled-07286";
        dynamic_rendering_07287            = "VUID-vkCmdDraw-multisampledRenderToSingleSampled-07287";
        image_view_access_64               = "VUID-vkCmdDraw-SampledType-04470";
        image_view_access_32               = "VUID-vkCmdDraw-SampledType-04471";
        buffer_view_access_64              = "VUID-vkCmdDraw-SampledType-04472";
        buffer_view_access_32              = "VUID-vkCmdDraw-SampledType-04473";
        storage_image_read_without_format  = "VUID-vkCmdDraw-OpTypeImage-07028";
        storage_image_write_without_format = "VUID-vkCmdDraw-OpTypeImage-07027";
        storage_texel_buffer_read_without_format  = "VUID-vkCmdDraw-OpTypeImage-07030";
        storage_texel_buffer_write_without_format = "VUID-vkCmdDraw-OpTypeImage-07029";
        storage_image_write_texel_count           = "VUID-vkCmdDraw-None-04115";
        storage_texel_buffer_write_texel_count    = "VUID-vkCmdDraw-OpImageWrite-04469";
        depth_compare_sample               = "VUID-vkCmdDraw-None-06479";
        depth_read_only                    = "VUID-vkCmdDraw-None-06886";
        stencil_read_only                  = "VUID-vkCmdDraw-None-06887";
        dynamic_sample_locations           = "VUID-vkCmdDraw-None-06666";
        dynamic_tessellation_domain_origin = "VUID-vkCmdDraw-None-07619";
        dynamic_depth_clamp_enable         = "VUID-vkCmdDraw-None-07620";
        dynamic_polygon_mode               = "VUID-vkCmdDraw-None-07621";
        dynamic_rasterization_samples      = "VUID-vkCmdDraw-None-07622";
        dynamic_sample_mask                = "VUID-vkCmdDraw-None-07623";
        dynamic_alpha_to_coverage_enable   = "VUID-vkCmdDraw-None-07624";
        dynamic_alpha_to_one_enable        = "VUID-vkCmdDraw-None-07625";
        dynamic_logic_op_enable            = "VUID-vkCmdDraw-None-07626";
        dynamic_color_blend_enable         = "VUID-vkCmdDraw-firstAttachment-07476";
        dynamic_color_blend_equation       = "VUID-vkCmdDraw-firstAttachment-07477";
        dynamic_color_write_mask           = "VUID-vkCmdDraw-firstAttachment-07478";
        dynamic_rasterization_stream       = "VUID-vkCmdDraw-None-07630";
        dynamic_conservative_rasterization_mode = "VUID-vkCmdDraw-None-07631";
        dynamic_extra_primitive_overestimation_size = "VUID-vkCmdDraw-None-07632";
        dynamic_depth_clip_enable          = "VUID-vkCmdDraw-None-07633";
        dynamic_sample_locations_enable    = "VUID-vkCmdDraw-None-07634";
        dynamic_color_blend_advanced       = "VUID-vkCmdDraw-firstAttachment-07479";
        dynamic_provoking_vertex_mode      = "VUID-vkCmdDraw-None-07636";
        dynamic_line_rasterization_mode    = "VUID-vkCmdDraw-None-07637";
        dynamic_line_stipple_enable        = "VUID-vkCmdDraw-None-07638";
        dynamic_depth_clip_negative_one_to_one = "VUID-vkCmdDraw-None-07639";
        dynamic_viewport_w_scaling_enable  = "VUID-vkCmdDraw-None-07640";
        dynamic_viewport_swizzle           = "VUID-vkCmdDraw-None-07641";
        dynamic_coverage_to_color_enable   = "VUID-vkCmdDraw-None-07642";
        dynamic_coverage_to_color_location = "VUID-vkCmdDraw-None-07643";
        dynamic_coverage_modulation_mode   = "VUID-vkCmdDraw-None-07644";
        dynamic_coverage_modulation_table_enable = "VUID-vkCmdDraw-None-07645";
        dynamic_coverage_modulation_table  = "VUID-vkCmdDraw-None-07646";
        dynamic_coverage_reduction_mode    = "VUID-vkCmdDraw-None-07647";
        dynamic_representative_fragment_test_enable  = "VUID-vkCmdDraw-None-07648";
        dynamic_shading_rate_image_enable  = "VUID-vkCmdDraw-None-07649";
        dynamic_viewport                   = "VUID-vkCmdDraw-None-07831";
        dynamic_scissor                    = "VUID-vkCmdDraw-None-07832";
        dynamic_depth_bias                 = "VUID-vkCmdDraw-None-07834";
        dynamic_line_width                 = "VUID-vkCmdDraw-None-07833";
        dynamic_line_stipple_ext           = "VUID-vkCmdDraw-None-07849";
        dynamic_blend_constants            = "VUID-vkCmdDraw-None-07835";
        dynamic_depth_bounds               = "VUID-vkCmdDraw-None-07836";
        dynamic_stencil_compare_mask       = "VUID-vkCmdDraw-None-07837";
        dynamic_stencil_write_mask         = "VUID-vkCmdDraw-None-07838";
        dynamic_stencil_reference          = "VUID-vkCmdDraw-None-07839";
        dynamic_state_inherited            = "VUID-vkCmdDraw-None-07850";
        primitives_generated               = "VUID-vkCmdDraw-primitivesGeneratedQueryWithRasterizerDiscard-06708";
        primitives_generated_streams       = "VUID-vkCmdDraw-primitivesGeneratedQueryWithNonZeroStreams-06709";
        invalid_mesh_shader_stages         = "VUID-vkCmdDraw-stage-06481";
        descriptor_buffer_bit_set          = "VUID-vkCmdDraw-None-08114";
        descriptor_buffer_bit_not_set      = "VUID-vkCmdDraw-None-08115";
        descriptor_buffer_set_offset_missing = "VUID-vkCmdDraw-None-08117";
        image_view_dim                     = "VUID-vkCmdDraw-viewType-07752";
        image_view_numeric_format          = "VUID-vkCmdDraw-format-07753";
        stippled_rectangular_lines         = "VUID-vkCmdDraw-stippledLineEnable-07495";
        stippled_bresenham_lines           = "VUID-vkCmdDraw-stippledLineEnable-07496";
        stippled_smooth_lines              = "VUID-vkCmdDraw-stippledLineEnable-07497";
        stippled_default_strict            = "VUID-vkCmdDraw-stippledLineEnable-07498";
    }
};

struct DispatchVuidsCmdDrawMultiEXT : DrawDispatchVuid {
    DispatchVuidsCmdDrawMultiEXT() : DrawDispatchVuid() {
        pipeline_bound                     = "VUID-vkCmdDrawMultiEXT-None-02700";
        vertex_binding                     = "VUID-vkCmdDrawMultiEXT-None-04007";
        vertex_binding_null                = "VUID-vkCmdDrawMultiEXT-None-04008";
        compatible_pipeline                = "VUID-vkCmdDrawMultiEXT-None-02697";
        render_pass_compatible             = "VUID-vkCmdDrawMultiEXT-renderPass-02684";
        subpass_index                      = "VUID-vkCmdDrawMultiEXT-subpass-02685";
        sample_location                    = "VUID-vkCmdDrawMultiEXT-sampleLocationsEnable-02689";
        linear_filter_sampler              = "VUID-vkCmdDrawMultiEXT-magFilter-04553";
        linear_mipmap_sampler              = "VUID-vkCmdDrawMultiEXT-mipmapMode-04770";
        cubic_sampler                      = "VUID-vkCmdDrawMultiEXT-None-02692";
        viewport_count                     = "VUID-vkCmdDrawMultiEXT-viewportCount-03417";
        scissor_count                      = "VUID-vkCmdDrawMultiEXT-scissorCount-03418";
        viewport_scissor_count             = "VUID-vkCmdDrawMultiEXT-viewportCount-03419";
        primitive_topology_class           = "VUID-vkCmdDrawMultiEXT-primitiveTopology-03420";
        primitive_topology_class_ds3       = "VUID-vkCmdDrawMultiEXT-dynamicPrimitiveTopologyUnrestricted-07500";
        dynamic_primitive_topology         = "VUID-vkCmdDrawMultiEXT-None-07842";
        corner_sampled_address_mode        = "VUID-vkCmdDrawMultiEXT-flags-02696";
        imageview_atomic                   = "VUID-vkCmdDrawMultiEXT-None-02691";
        push_constants_set                 = "VUID-vkCmdDrawMultiEXT-maintenance4-06425";
        image_subresources_render_pass_write = "VUID-vkCmdDrawMultiEXT-None-06537";
        image_subresources_subpass_read    = "VUID-vkCmdDrawMultiEXT-None-06538";
        image_subresources_subpass_write   = "VUID-vkCmdDrawMultiEXT-None-06539";
        descriptor_valid                   = "VUID-vkCmdDrawMultiEXT-None-02699";
        sampler_imageview_type             = "VUID-vkCmdDrawMultiEXT-None-02702";
        sampler_implicitLod_dref_proj      = "VUID-vkCmdDrawMultiEXT-None-02703";
        sampler_bias_offset                = "VUID-vkCmdDrawMultiEXT-None-02704";
        vertex_binding_attribute           = "VUID-vkCmdDrawMultiEXT-None-02721";
        dynamic_state_setting_commands     = "VUID-vkCmdDrawMultiEXT-None-02859";
        rasterization_samples              = "VUID-vkCmdDrawMultiEXT-rasterizationSamples-04740";
        msrtss_rasterization_samples       = "VUID-vkCmdDrawMultiEXT-multisampledRenderToSingleSampled-07284";
        unprotected_command_buffer         = "VUID-vkCmdDrawMultiEXT-commandBuffer-02707";
        protected_command_buffer           = "VUID-vkCmdDrawMultiEXT-commandBuffer-02712";
        max_multiview_instance_index       = "VUID-vkCmdDrawMultiEXT-maxMultiviewInstanceIndex-02688";
        img_filter_cubic                   = "VUID-vkCmdDrawMultiEXT-None-02693";
        filter_cubic                       = "VUID-vkCmdDrawMultiEXT-filterCubic-02694";
        filter_cubic_min_max               = "VUID-vkCmdDrawMultiEXT-filterCubicMinmax-02695";
        viewport_count_primitive_shading_rate = "VUID-vkCmdDrawMultiEXT-primitiveFragmentShadingRateWithMultipleViewports-04552";
        patch_control_points               = "VUID-vkCmdDrawMultiEXT-None-04875";
        rasterizer_discard_enable          = "VUID-vkCmdDrawMultiEXT-None-04876";
        depth_bias_enable                  = "VUID-vkCmdDrawMultiEXT-None-04877";
        logic_op                           = "VUID-vkCmdDrawMultiEXT-logicOp-04878";
        primitive_restart_enable           = "VUID-vkCmdDrawMultiEXT-None-04879";
        vertex_input_binding_stride        = "VUID-vkCmdDrawMultiEXT-pStrides-04884";
        vertex_input                       = "VUID-vkCmdDrawMultiEXT-None-04914";
        blend_enable                       = "VUID-vkCmdDrawMultiEXT-blendEnable-04727";
        dynamic_discard_rectangle          = "VUID-vkCmdDrawMultiEXT-None-07751";
        dynamic_color_write_enable         = "VUID-vkCmdDrawMultiEXT-None-07749";
        dynamic_color_write_enable_count   = "VUID-vkCmdDrawMultiEXT-attachmentCount-07750";
        dynamic_rendering_view_mask        = "VUID-vkCmdDrawMultiEXT-viewMask-06178";
        dynamic_rendering_color_count      = "VUID-vkCmdDrawMultiEXT-colorAttachmentCount-06179";
        dynamic_rendering_color_formats    = "VUID-vkCmdDrawMultiEXT-colorAttachmentCount-06180";
        dynamic_rendering_depth_format     = "VUID-vkCmdDrawMultiEXT-pDepthAttachment-06181";
        dynamic_rendering_stencil_format   = "VUID-vkCmdDrawMultiEXT-pStencilAttachment-06182";
        dynamic_rendering_fsr              = "VUID-vkCmdDrawMultiEXT-imageView-06183";
        dynamic_rendering_fdm              = "VUID-vkCmdDrawMultiEXT-imageView-06184";
        dynamic_rendering_color_sample     = "VUID-vkCmdDrawMultiEXT-colorAttachmentCount-06185";
        dynamic_rendering_depth_sample     = "VUID-vkCmdDrawMultiEXT-pDepthAttachment-06186";
        dynamic_rendering_stencil_sample   = "VUID-vkCmdDrawMultiEXT-pStencilAttachment-06187";
        dynamic_rendering_multi_sample     = "VUID-vkCmdDrawMultiEXT-colorAttachmentCount-06188";
        dynamic_rendering_06189            = "VUID-vkCmdDrawMultiEXT-pDepthAttachment-06189";
        dynamic_rendering_06190            = "VUID-vkCmdDrawMultiEXT-pStencilAttachment-06190";
        dynamic_rendering_06198            = "VUID-vkCmdDrawMultiEXT-renderPass-06198";
        dynamic_rendering_07285            = "VUID-vkCmdDrawMultiEXT-multisampledRenderToSingleSampled-07285";
        dynamic_rendering_07286            = "VUID-vkCmdDrawMultiEXT-multisampledRenderToSingleSampled-07286";
        dynamic_rendering_07287            = "VUID-vkCmdDrawMultiEXT-multisampledRenderToSingleSampled-07287";
        image_view_access_64               = "VUID-vkCmdDrawMultiEXT-SampledType-04470";
        image_view_access_32               = "VUID-vkCmdDrawMultiEXT-SampledType-04471";
        buffer_view_access_64              = "VUID-vkCmdDrawMultiEXT-SampledType-04472";
        buffer_view_access_32              = "VUID-vkCmdDrawMultiEXT-SampledType-04473";
        storage_image_read_without_format  = "VUID-vkCmdDrawMultiEXT-OpTypeImage-07028";
        storage_image_write_without_format = "VUID-vkCmdDrawMultiEXT-OpTypeImage-07027";
        storage_texel_buffer_read_without_format  = "VUID-vkCmdDrawMultiEXT-OpTypeImage-07030";
        storage_texel_buffer_write_without_format = "VUID-vkCmdDrawMultiEXT-OpTypeImage-07029";
        storage_image_write_texel_count           = "VUID-vkCmdDrawMultiEXT-None-04115";
        storage_texel_buffer_write_texel_count    = "VUID-vkCmdDrawMultiEXT-OpImageWrite-04469";
        depth_compare_sample               = "VUID-vkCmdDrawMultiEXT-None-06479";
        depth_read_only                    = "VUID-vkCmdDrawMultiEXT-None-06886";
        stencil_read_only                  = "VUID-vkCmdDrawMultiEXT-None-06887";
        dynamic_sample_locations           = "VUID-vkCmdDrawMultiEXT-None-06666";
        dynamic_tessellation_domain_origin = "VUID-vkCmdDrawMultiEXT-None-07619";
        dynamic_depth_clamp_enable         = "VUID-vkCmdDrawMultiEXT-None-07620";
        dynamic_polygon_mode               = "VUID-vkCmdDrawMultiEXT-None-07621";
        dynamic_rasterization_samples      = "VUID-vkCmdDrawMultiEXT-None-07622";
        dynamic_sample_mask                = "VUID-vkCmdDrawMultiEXT-None-07623";
        dynamic_alpha_to_coverage_enable   = "VUID-vkCmdDrawMultiEXT-None-07624";
        dynamic_alpha_to_one_enable        = "VUID-vkCmdDrawMultiEXT-None-07625";
        dynamic_logic_op_enable            = "VUID-vkCmdDrawMultiEXT-None-07626";
        dynamic_color_blend_enable         = "VUID-vkCmdDrawMultiEXT-firstAttachment-07476";
        dynamic_color_blend_equation       = "VUID-vkCmdDrawMultiEXT-firstAttachment-07477";
        dynamic_color_write_mask           = "VUID-vkCmdDrawMultiEXT-firstAttachment-07478";
        dynamic_rasterization_stream       = "VUID-vkCmdDrawMultiEXT-None-07630";
        dynamic_conservative_rasterization_mode = "VUID-vkCmdDrawMultiEXT-None-07631";
        dynamic_extra_primitive_overestimation_size = "VUID-vkCmdDrawMultiEXT-None-07632";
        dynamic_depth_clip_enable          = "VUID-vkCmdDrawMultiEXT-None-07633";
        dynamic_sample_locations_enable    = "VUID-vkCmdDrawMultiEXT-None-07634";
        dynamic_color_blend_advanced       = "VUID-vkCmdDrawMultiEXT-firstAttachment-07479";
        dynamic_provoking_vertex_mode      = "VUID-vkCmdDrawMultiEXT-None-07636";
        dynamic_line_rasterization_mode    = "VUID-vkCmdDrawMultiEXT-None-07637";
        dynamic_line_stipple_enable        = "VUID-vkCmdDrawMultiEXT-None-07638";
        dynamic_depth_clip_negative_one_to_one = "VUID-vkCmdDrawMultiEXT-None-07639";
        dynamic_viewport_w_scaling_enable  = "VUID-vkCmdDrawMultiEXT-None-07640";
        dynamic_viewport_swizzle           = "VUID-vkCmdDrawMultiEXT-None-07641";
        dynamic_coverage_to_color_enable   = "VUID-vkCmdDrawMultiEXT-None-07642";
        dynamic_coverage_to_color_location = "VUID-vkCmdDrawMultiEXT-None-07643";
        dynamic_coverage_modulation_mode   = "VUID-vkCmdDrawMultiEXT-None-07644";
        dynamic_coverage_modulation_table_enable = "VUID-vkCmdDrawMultiEXT-None-07645";
        dynamic_coverage_modulation_table  = "VUID-vkCmdDrawMultiEXT-None-07646";
        dynamic_coverage_reduction_mode    = "VUID-vkCmdDrawMultiEXT-None-07647";
        dynamic_representative_fragment_test_enable  = "VUID-vkCmdDrawMultiEXT-None-07648";
        dynamic_shading_rate_image_enable  = "VUID-vkCmdDrawMultiEXT-None-07649";
        dynamic_viewport                   = "VUID-vkCmdDrawMultiEXT-None-07831";
        dynamic_scissor                    = "VUID-vkCmdDrawMultiEXT-None-07832";
        dynamic_depth_bias                 = "VUID-vkCmdDrawMultiEXT-None-07834";
        dynamic_line_width                 = "VUID-vkCmdDrawMultiEXT-None-07833";
        dynamic_line_stipple_ext           = "VUID-vkCmdDrawMultiEXT-None-07849";
        dynamic_blend_constants            = "VUID-vkCmdDrawMultiEXT-None-07835";
        dynamic_depth_bounds               = "VUID-vkCmdDrawMultiEXT-None-07836";
        dynamic_stencil_compare_mask       = "VUID-vkCmdDrawMultiEXT-None-07837";
        dynamic_stencil_write_mask         = "VUID-vkCmdDrawMultiEXT-None-07838";
        dynamic_stencil_reference          = "VUID-vkCmdDrawMultiEXT-None-07839";
        dynamic_state_inherited            = "VUID-vkCmdDrawMultiEXT-None-07850";
        primitives_generated               = "VUID-vkCmdDrawMultiEXT-primitivesGeneratedQueryWithRasterizerDiscard-06708";
        primitives_generated_streams       = "VUID-vkCmdDrawMultiEXT-primitivesGeneratedQueryWithNonZeroStreams-06709";
        invalid_mesh_shader_stages         = "VUID-vkCmdDrawMultiEXT-stage-06481";
        descriptor_buffer_bit_set          = "VUID-vkCmdDrawMultiEXT-None-08114";
        descriptor_buffer_bit_not_set      = "VUID-vkCmdDrawMultiEXT-None-08115";
        descriptor_buffer_set_offset_missing = "VUID-vkCmdDrawMultiEXT-None-08117";
        image_view_dim                     = "VUID-vkCmdDrawMultiEXT-viewType-07752";
        image_view_numeric_format          = "VUID-vkCmdDrawMultiEXT-format-07753";
        stippled_rectangular_lines         = "VUID-vkCmdDrawMultiEXT-stippledLineEnable-07495";
        stippled_bresenham_lines           = "VUID-vkCmdDrawMultiEXT-stippledLineEnable-07496";
        stippled_smooth_lines              = "VUID-vkCmdDrawMultiEXT-stippledLineEnable-07497";
        stippled_default_strict            = "VUID-vkCmdDrawMultiEXT-stippledLineEnable-07498";
    }
};

struct DispatchVuidsCmdDrawIndexed : DrawDispatchVuid {
    DispatchVuidsCmdDrawIndexed() : DrawDispatchVuid() {
        pipeline_bound                     = "VUID-vkCmdDrawIndexed-None-02700";
        index_binding                      = "VUID-vkCmdDrawIndexed-None-07312";
        vertex_binding                     = "VUID-vkCmdDrawIndexed-None-04007";
        vertex_binding_null                = "VUID-vkCmdDrawIndexed-None-04008";
        compatible_pipeline                = "VUID-vkCmdDrawIndexed-None-02697";
        render_pass_compatible             = "VUID-vkCmdDrawIndexed-renderPass-02684";
        subpass_index                      = "VUID-vkCmdDrawIndexed-subpass-02685";
        sample_location                    = "VUID-vkCmdDrawIndexed-sampleLocationsEnable-02689";
        linear_filter_sampler              = "VUID-vkCmdDrawIndexed-magFilter-04553";
        linear_mipmap_sampler              = "VUID-vkCmdDrawIndexed-mipmapMode-04770";
        cubic_sampler                      = "VUID-vkCmdDrawIndexed-None-02692";
        viewport_count                     = "VUID-vkCmdDrawIndexed-viewportCount-03417";
        scissor_count                      = "VUID-vkCmdDrawIndexed-scissorCount-03418";
        viewport_scissor_count             = "VUID-vkCmdDrawIndexed-viewportCount-03419";
        primitive_topology_class           = "VUID-vkCmdDrawIndexed-primitiveTopology-03420";
        primitive_topology_class_ds3       = "VUID-vkCmdDrawIndexed-dynamicPrimitiveTopologyUnrestricted-07500";
        dynamic_primitive_topology         = "VUID-vkCmdDrawIndexed-None-07842";
        corner_sampled_address_mode        = "VUID-vkCmdDrawIndexed-flags-02696";
        imageview_atomic                   = "VUID-vkCmdDrawIndexed-None-02691";
        push_constants_set                 = "VUID-vkCmdDrawIndexed-maintenance4-06425";
        image_subresources_render_pass_write = "VUID-vkCmdDrawIndexed-None-06537";
        image_subresources_subpass_read    = "VUID-vkCmdDrawIndexed-None-06538";
        image_subresources_subpass_write   = "VUID-vkCmdDrawIndexed-None-06539";
        descriptor_valid                   = "VUID-vkCmdDrawIndexed-None-02699";
        sampler_imageview_type             = "VUID-vkCmdDrawIndexed-None-02702";
        sampler_implicitLod_dref_proj      = "VUID-vkCmdDrawIndexed-None-02703";
        sampler_bias_offset                = "VUID-vkCmdDrawIndexed-None-02704";
        vertex_binding_attribute           = "VUID-vkCmdDrawIndexed-None-02721";
        dynamic_state_setting_commands     = "VUID-vkCmdDrawIndexed-None-02859";
        rasterization_samples              = "VUID-vkCmdDrawIndexed-rasterizationSamples-04740";
        msrtss_rasterization_samples       = "VUID-vkCmdDrawIndexed-multisampledRenderToSingleSampled-07284";
        unprotected_command_buffer         = "VUID-vkCmdDrawIndexed-commandBuffer-02707";
        protected_command_buffer           = "VUID-vkCmdDrawIndexed-commandBuffer-02712";
        max_multiview_instance_index       = "VUID-vkCmdDrawIndexed-maxMultiviewInstanceIndex-02688";
        img_filter_cubic                   = "VUID-vkCmdDrawIndexed-None-02693";
        filter_cubic                       = "VUID-vkCmdDrawIndexed-filterCubic-02694";
        filter_cubic_min_max               = "VUID-vkCmdDrawIndexed-filterCubicMinmax-02695";
        viewport_count_primitive_shading_rate = "VUID-vkCmdDrawIndexed-primitiveFragmentShadingRateWithMultipleViewports-04552";
        patch_control_points               = "VUID-vkCmdDrawIndexed-None-04875";
        rasterizer_discard_enable          = "VUID-vkCmdDrawIndexed-None-04876";
        depth_bias_enable                  = "VUID-vkCmdDrawIndexed-None-04877";
        logic_op                           = "VUID-vkCmdDrawIndexed-logicOp-04878";
        primitive_restart_enable           = "VUID-vkCmdDrawIndexed-None-04879";
        vertex_input_binding_stride        = "VUID-vkCmdDrawIndexed-pStrides-04884";
        vertex_input                       = "VUID-vkCmdDrawIndexed-None-04914";
        blend_enable                       = "VUID-vkCmdDrawIndexed-blendEnable-04727";
        dynamic_discard_rectangle          = "VUID-vkCmdDrawIndexed-None-07751";
        dynamic_color_write_enable         = "VUID-vkCmdDrawIndexed-None-07749";
        dynamic_color_write_enable_count   = "VUID-vkCmdDrawIndexed-attachmentCount-07750";
        dynamic_rendering_view_mask        = "VUID-vkCmdDrawIndexed-viewMask-06178";
        dynamic_rendering_color_count      = "VUID-vkCmdDrawIndexed-colorAttachmentCount-06179";
        dynamic_rendering_color_formats    = "VUID-vkCmdDrawIndexed-colorAttachmentCount-06180";
        dynamic_rendering_depth_format     = "VUID-vkCmdDrawIndexed-pDepthAttachment-06181";
        dynamic_rendering_stencil_format   = "VUID-vkCmdDrawIndexed-pStencilAttachment-06182";
        dynamic_rendering_fsr              = "VUID-vkCmdDrawIndexed-imageView-06183";
        dynamic_rendering_fdm              = "VUID-vkCmdDrawIndexed-imageView-06184";
        dynamic_rendering_color_sample     = "VUID-vkCmdDrawIndexed-colorAttachmentCount-06185";
        dynamic_rendering_depth_sample     = "VUID-vkCmdDrawIndexed-pDepthAttachment-06186";
        dynamic_rendering_stencil_sample   = "VUID-vkCmdDrawIndexed-pStencilAttachment-06187";
        dynamic_rendering_multi_sample     = "VUID-vkCmdDrawIndexed-colorAttachmentCount-06188";
        dynamic_rendering_06189            = "VUID-vkCmdDrawIndexed-pDepthAttachment-06189";
        dynamic_rendering_06190            = "VUID-vkCmdDrawIndexed-pStencilAttachment-06190";
        dynamic_rendering_06198            = "VUID-vkCmdDrawIndexed-renderPass-06198";
        dynamic_rendering_07285            = "VUID-vkCmdDrawIndexed-multisampledRenderToSingleSampled-07285";
        dynamic_rendering_07286            = "VUID-vkCmdDrawIndexed-multisampledRenderToSingleSampled-07286";
        dynamic_rendering_07287            = "VUID-vkCmdDrawIndexed-multisampledRenderToSingleSampled-07287";
        image_view_access_64               = "VUID-vkCmdDrawIndexed-SampledType-04470";
        image_view_access_32               = "VUID-vkCmdDrawIndexed-SampledType-04471";
        buffer_view_access_64              = "VUID-vkCmdDrawIndexed-SampledType-04472";
        buffer_view_access_32              = "VUID-vkCmdDrawIndexed-SampledType-04473";
        storage_image_read_without_format  = "VUID-vkCmdDrawIndexed-OpTypeImage-07028";
        storage_image_write_without_format = "VUID-vkCmdDrawIndexed-OpTypeImage-07027";
        storage_texel_buffer_read_without_format  = "VUID-vkCmdDrawIndexed-OpTypeImage-07030";
        storage_texel_buffer_write_without_format = "VUID-vkCmdDrawIndexed-OpTypeImage-07029";
        storage_image_write_texel_count           = "VUID-vkCmdDrawIndexed-None-04115";
        storage_texel_buffer_write_texel_count    = "VUID-vkCmdDrawIndexed-OpImageWrite-04469";
        depth_compare_sample               = "VUID-vkCmdDrawIndexed-None-06479";
        depth_read_only                    = "VUID-vkCmdDrawIndexed-None-06886";
        stencil_read_only                  = "VUID-vkCmdDrawIndexed-None-06887";
        dynamic_sample_locations           = "VUID-vkCmdDrawIndexed-None-06666";
        dynamic_tessellation_domain_origin = "VUID-vkCmdDrawIndexed-None-07619";
        dynamic_depth_clamp_enable         = "VUID-vkCmdDrawIndexed-None-07620";
        dynamic_polygon_mode               = "VUID-vkCmdDrawIndexed-None-07621";
        dynamic_rasterization_samples      = "VUID-vkCmdDrawIndexed-None-07622";
        dynamic_sample_mask                = "VUID-vkCmdDrawIndexed-None-07623";
        dynamic_alpha_to_coverage_enable   = "VUID-vkCmdDrawIndexed-None-07624";
        dynamic_alpha_to_one_enable        = "VUID-vkCmdDrawIndexed-None-07625";
        dynamic_logic_op_enable            = "VUID-vkCmdDrawIndexed-None-07626";
        dynamic_color_blend_enable         = "VUID-vkCmdDrawIndexed-firstAttachment-07476";
        dynamic_color_blend_equation       = "VUID-vkCmdDrawIndexed-firstAttachment-07477";
        dynamic_color_write_mask           = "VUID-vkCmdDrawIndexed-firstAttachment-07478";
        dynamic_rasterization_stream       = "VUID-vkCmdDrawIndexed-None-07630";
        dynamic_conservative_rasterization_mode = "VUID-vkCmdDrawIndexed-None-07631";
        dynamic_extra_primitive_overestimation_size = "VUID-vkCmdDrawIndexed-None-07632";
        dynamic_depth_clip_enable          = "VUID-vkCmdDrawIndexed-None-07633";
        dynamic_sample_locations_enable    = "VUID-vkCmdDrawIndexed-None-07634";
        dynamic_color_blend_advanced       = "VUID-vkCmdDrawIndexed-firstAttachment-07479";
        dynamic_provoking_vertex_mode      = "VUID-vkCmdDrawIndexed-None-07636";
        dynamic_line_rasterization_mode    = "VUID-vkCmdDrawIndexed-None-07637";
        dynamic_line_stipple_enable        = "VUID-vkCmdDrawIndexed-None-07638";
        dynamic_depth_clip_negative_one_to_one = "VUID-vkCmdDrawIndexed-None-07639";
        dynamic_viewport_w_scaling_enable  = "VUID-vkCmdDrawIndexed-None-07640";
        dynamic_viewport_swizzle           = "VUID-vkCmdDrawIndexed-None-07641";
        dynamic_coverage_to_color_enable   = "VUID-vkCmdDrawIndexed-None-07642";
        dynamic_coverage_to_color_location = "VUID-vkCmdDrawIndexed-None-07643";
        dynamic_coverage_modulation_mode   = "VUID-vkCmdDrawIndexed-None-07644";
        dynamic_coverage_modulation_table_enable = "VUID-vkCmdDrawIndexed-None-07645";
        dynamic_coverage_modulation_table  = "VUID-vkCmdDrawIndexed-None-07646";
        dynamic_coverage_reduction_mode    = "VUID-vkCmdDrawIndexed-None-07647";
        dynamic_representative_fragment_test_enable  = "VUID-vkCmdDrawIndexed-None-07648";
        dynamic_shading_rate_image_enable  = "VUID-vkCmdDrawIndexed-None-07649";
        dynamic_viewport                   = "VUID-vkCmdDrawIndexed-None-07831";
        dynamic_scissor                    = "VUID-vkCmdDrawIndexed-None-07832";
        dynamic_depth_bias                 = "VUID-vkCmdDrawIndexed-None-07834";
        dynamic_line_width                 = "VUID-vkCmdDrawIndexed-None-07833";
        dynamic_line_stipple_ext           = "VUID-vkCmdDrawIndexed-None-07849";
        dynamic_blend_constants            = "VUID-vkCmdDrawIndexed-None-07835";
        dynamic_depth_bounds               = "VUID-vkCmdDrawIndexed-None-07836";
        dynamic_stencil_compare_mask       = "VUID-vkCmdDrawIndexed-None-07837";
        dynamic_stencil_write_mask         = "VUID-vkCmdDrawIndexed-None-07838";
        dynamic_stencil_reference          = "VUID-vkCmdDrawIndexed-None-07839";
        dynamic_state_inherited            = "VUID-vkCmdDrawIndexed-None-07850";
        primitives_generated               = "VUID-vkCmdDrawIndexed-primitivesGeneratedQueryWithRasterizerDiscard-06708";
        primitives_generated_streams       = "VUID-vkCmdDrawIndexed-primitivesGeneratedQueryWithNonZeroStreams-06709";
        invalid_mesh_shader_stages         = "VUID-vkCmdDrawIndexed-stage-06481";
        descriptor_buffer_bit_set          = "VUID-vkCmdDrawIndexed-None-08114";
        descriptor_buffer_bit_not_set      = "VUID-vkCmdDrawIndexed-None-08115";
        descriptor_buffer_set_offset_missing = "VUID-vkCmdDrawIndexed-None-08117";
        image_view_dim                     = "VUID-vkCmdDrawIndexed-viewType-07752";
        image_view_numeric_format          = "VUID-vkCmdDrawIndexed-format-07753";
        stippled_rectangular_lines         = "VUID-vkCmdDrawIndexed-stippledLineEnable-07495";
        stippled_bresenham_lines           = "VUID-vkCmdDrawIndexed-stippledLineEnable-07496";
        stippled_smooth_lines              = "VUID-vkCmdDrawIndexed-stippledLineEnable-07497";
        stippled_default_strict            = "VUID-vkCmdDrawIndexed-stippledLineEnable-07498";
    }
};

struct DispatchVuidsCmdDrawMultiIndexedEXT : DrawDispatchVuid {
    DispatchVuidsCmdDrawMultiIndexedEXT() : DrawDispatchVuid() {
        pipeline_bound                     = "VUID-vkCmdDrawMultiIndexedEXT-None-02700";
        index_binding                      = "VUID-vkCmdDrawMultiIndexedEXT-None-07312";
        vertex_binding                     = "VUID-vkCmdDrawMultiIndexedEXT-None-04007";
        vertex_binding_null                = "VUID-vkCmdDrawMultiIndexedEXT-None-04008";
        compatible_pipeline                = "VUID-vkCmdDrawMultiIndexedEXT-None-02697";
        render_pass_compatible             = "VUID-vkCmdDrawMultiIndexedEXT-renderPass-02684";
        subpass_index                      = "VUID-vkCmdDrawMultiIndexedEXT-subpass-02685";
        sample_location                    = "VUID-vkCmdDrawMultiIndexedEXT-sampleLocationsEnable-02689";
        linear_filter_sampler              = "VUID-vkCmdDrawMultiIndexedEXT-magFilter-04553";
        linear_mipmap_sampler              = "VUID-vkCmdDrawMultiIndexedEXT-mipmapMode-04770";
        cubic_sampler                      = "VUID-vkCmdDrawMultiIndexedEXT-None-02692";
        viewport_count                     = "VUID-vkCmdDrawMultiIndexedEXT-viewportCount-03417";
        scissor_count                      = "VUID-vkCmdDrawMultiIndexedEXT-scissorCount-03418";
        viewport_scissor_count             = "VUID-vkCmdDrawMultiIndexedEXT-viewportCount-03419";
        primitive_topology_class           = "VUID-vkCmdDrawMultiIndexedEXT-primitiveTopology-03420";
        primitive_topology_class_ds3       = "VUID-vkCmdDrawMultiIndexedEXT-dynamicPrimitiveTopologyUnrestricted-07500";
        dynamic_primitive_topology         = "VUID-vkCmdDrawMultiIndexedEXT-None-07842";
        corner_sampled_address_mode        = "VUID-vkCmdDrawMultiIndexedEXT-flags-02696";
        imageview_atomic                   = "VUID-vkCmdDrawMultiIndexedEXT-None-02691";
        push_constants_set                 = "VUID-vkCmdDrawMultiIndexedEXT-maintenance4-06425";
        image_subresources_render_pass_write = "VUID-vkCmdDrawMultiIndexedEXT-None-06537";
        image_subresources_subpass_read    = "VUID-vkCmdDrawMultiIndexedEXT-None-06538";
        image_subresources_subpass_write   = "VUID-vkCmdDrawMultiIndexedEXT-None-06539";
        descriptor_valid                   = "VUID-vkCmdDrawMultiIndexedEXT-None-02699";
        sampler_imageview_type             = "VUID-vkCmdDrawMultiIndexedEXT-None-02702";
        sampler_implicitLod_dref_proj      = "VUID-vkCmdDrawMultiIndexedEXT-None-02703";
        sampler_bias_offset                = "VUID-vkCmdDrawMultiIndexedEXT-None-02704";
        vertex_binding_attribute           = "VUID-vkCmdDrawMultiIndexedEXT-None-02721";
        dynamic_state_setting_commands     = "VUID-vkCmdDrawMultiIndexedEXT-None-02859";
        rasterization_samples              = "VUID-vkCmdDrawMultiIndexedEXT-rasterizationSamples-04740";
        msrtss_rasterization_samples       = "VUID-vkCmdDrawMultiIndexedEXT-multisampledRenderToSingleSampled-07284";
        unprotected_command_buffer         = "VUID-vkCmdDrawMultiIndexedEXT-commandBuffer-02707";
        protected_command_buffer           = "VUID-vkCmdDrawMultiIndexedEXT-commandBuffer-02712";
        max_multiview_instance_index       = "VUID-vkCmdDrawMultiIndexedEXT-maxMultiviewInstanceIndex-02688";
        img_filter_cubic                   = "VUID-vkCmdDrawMultiIndexedEXT-None-02693";
        filter_cubic                       = "VUID-vkCmdDrawMultiIndexedEXT-filterCubic-02694";
        filter_cubic_min_max               = "VUID-vkCmdDrawMultiIndexedEXT-filterCubicMinmax-02695";
        viewport_count_primitive_shading_rate = "VUID-vkCmdDrawMultiIndexedEXT-primitiveFragmentShadingRateWithMultipleViewports-04552";
        patch_control_points               = "VUID-vkCmdDrawMultiIndexedEXT-None-04875";
        rasterizer_discard_enable          = "VUID-vkCmdDrawMultiIndexedEXT-None-04876";
        depth_bias_enable                  = "VUID-vkCmdDrawMultiIndexedEXT-None-04877";
        logic_op                           = "VUID-vkCmdDrawMultiIndexedEXT-logicOp-04878";
        primitive_restart_enable           = "VUID-vkCmdDrawMultiIndexedEXT-None-04879";
        vertex_input_binding_stride        = "VUID-vkCmdDrawMultiIndexedEXT-pStrides-04884";
        vertex_input                       = "VUID-vkCmdDrawMultiIndexedEXT-None-04914";
        blend_enable                       = "VUID-vkCmdDrawMultiIndexedEXT-blendEnable-04727";
        dynamic_discard_rectangle          = "VUID-vkCmdDrawMultiIndexedEXT-None-07751";
        dynamic_color_write_enable         = "VUID-vkCmdDrawMultiIndexedEXT-None-07749";
        dynamic_color_write_enable_count   = "VUID-vkCmdDrawMultiIndexedEXT-attachmentCount-07750";
        dynamic_rendering_view_mask        = "VUID-vkCmdDrawMultiIndexedEXT-viewMask-06178";
        dynamic_rendering_color_count      = "VUID-vkCmdDrawMultiIndexedEXT-colorAttachmentCount-06179";
        dynamic_rendering_color_formats    = "VUID-vkCmdDrawMultiIndexedEXT-colorAttachmentCount-06180";
        dynamic_rendering_depth_format     = "VUID-vkCmdDrawMultiIndexedEXT-pDepthAttachment-06181";
        dynamic_rendering_stencil_format   = "VUID-vkCmdDrawMultiIndexedEXT-pStencilAttachment-06182";
        dynamic_rendering_fsr              = "VUID-vkCmdDrawMultiIndexedEXT-imageView-06183";
        dynamic_rendering_fdm              = "VUID-vkCmdDrawMultiIndexedEXT-imageView-06184";
        dynamic_rendering_color_sample     = "VUID-vkCmdDrawMultiIndexedEXT-colorAttachmentCount-06185";
        dynamic_rendering_depth_sample     = "VUID-vkCmdDrawMultiIndexedEXT-pDepthAttachment-06186";
        dynamic_rendering_stencil_sample   = "VUID-vkCmdDrawMultiIndexedEXT-pStencilAttachment-06187";
        dynamic_rendering_multi_sample     = "VUID-vkCmdDrawMultiIndexedEXT-colorAttachmentCount-06188";
        dynamic_rendering_06189            = "VUID-vkCmdDrawMultiIndexedEXT-pDepthAttachment-06189";
        dynamic_rendering_06190            = "VUID-vkCmdDrawMultiIndexedEXT-pStencilAttachment-06190";
        dynamic_rendering_06198            = "VUID-vkCmdDrawMultiIndexedEXT-renderPass-06198";
        dynamic_rendering_07285            = "VUID-vkCmdDrawMultiIndexedEXT-multisampledRenderToSingleSampled-07285";
        dynamic_rendering_07286            = "VUID-vkCmdDrawMultiIndexedEXT-multisampledRenderToSingleSampled-07286";
        dynamic_rendering_07287            = "VUID-vkCmdDrawMultiIndexedEXT-multisampledRenderToSingleSampled-07287";
        image_view_access_64               = "VUID-vkCmdDrawMultiIndexedEXT-SampledType-04470";
        image_view_access_32               = "VUID-vkCmdDrawMultiIndexedEXT-SampledType-04471";
        buffer_view_access_64              = "VUID-vkCmdDrawMultiIndexedEXT-SampledType-04472";
        buffer_view_access_32              = "VUID-vkCmdDrawMultiIndexedEXT-SampledType-04473";
        storage_image_read_without_format  = "VUID-vkCmdDrawMultiIndexedEXT-OpTypeImage-07028";
        storage_image_write_without_format = "VUID-vkCmdDrawMultiIndexedEXT-OpTypeImage-07027";
        storage_texel_buffer_read_without_format  = "VUID-vkCmdDrawMultiIndexedEXT-OpTypeImage-07030";
        storage_texel_buffer_write_without_format = "VUID-vkCmdDrawMultiIndexedEXT-OpTypeImage-07029";
        storage_image_write_texel_count           = "VUID-vkCmdDrawMultiIndexedEXT-None-04115";
        storage_texel_buffer_write_texel_count    = "VUID-vkCmdDrawMultiIndexedEXT-OpImageWrite-04469";
        depth_compare_sample               = "VUID-vkCmdDrawMultiIndexedEXT-None-06479";
        depth_read_only                    = "VUID-vkCmdDrawMultiIndexedEXT-None-06886";
        stencil_read_only                  = "VUID-vkCmdDrawMultiIndexedEXT-None-06887";
        dynamic_sample_locations           = "VUID-vkCmdDrawMultiIndexedEXT-None-06666";
        dynamic_tessellation_domain_origin = "VUID-vkCmdDrawMultiIndexedEXT-None-07619";
        dynamic_depth_clamp_enable         = "VUID-vkCmdDrawMultiIndexedEXT-None-07620";
        dynamic_polygon_mode               = "VUID-vkCmdDrawMultiIndexedEXT-None-07621";
        dynamic_rasterization_samples      = "VUID-vkCmdDrawMultiIndexedEXT-None-07622";
        dynamic_sample_mask                = "VUID-vkCmdDrawMultiIndexedEXT-None-07623";
        dynamic_alpha_to_coverage_enable   = "VUID-vkCmdDrawMultiIndexedEXT-None-07624";
        dynamic_alpha_to_one_enable        = "VUID-vkCmdDrawMultiIndexedEXT-None-07625";
        dynamic_logic_op_enable            = "VUID-vkCmdDrawMultiIndexedEXT-None-07626";
        dynamic_color_blend_enable         = "VUID-vkCmdDrawMultiIndexedEXT-firstAttachment-07476";
        dynamic_color_blend_equation       = "VUID-vkCmdDrawMultiIndexedEXT-firstAttachment-07477";
        dynamic_color_write_mask           = "VUID-vkCmdDrawMultiIndexedEXT-firstAttachment-07478";
        dynamic_rasterization_stream       = "VUID-vkCmdDrawMultiIndexedEXT-None-07630";
        dynamic_conservative_rasterization_mode = "VUID-vkCmdDrawMultiIndexedEXT-None-07631";
        dynamic_extra_primitive_overestimation_size = "VUID-vkCmdDrawMultiIndexedEXT-None-07632";
        dynamic_depth_clip_enable          = "VUID-vkCmdDrawMultiIndexedEXT-None-07633";
        dynamic_sample_locations_enable    = "VUID-vkCmdDrawMultiIndexedEXT-None-07634";
        dynamic_color_blend_advanced       = "VUID-vkCmdDrawMultiIndexedEXT-firstAttachment-07479";
        dynamic_provoking_vertex_mode      = "VUID-vkCmdDrawMultiIndexedEXT-None-07636";
        dynamic_line_rasterization_mode    = "VUID-vkCmdDrawMultiIndexedEXT-None-07637";
        dynamic_line_stipple_enable        = "VUID-vkCmdDrawMultiIndexedEXT-None-07638";
        dynamic_depth_clip_negative_one_to_one = "VUID-vkCmdDrawMultiIndexedEXT-None-07639";
        dynamic_viewport_w_scaling_enable  = "VUID-vkCmdDrawMultiIndexedEXT-None-07640";
        dynamic_viewport_swizzle           = "VUID-vkCmdDrawMultiIndexedEXT-None-07641";
        dynamic_coverage_to_color_enable   = "VUID-vkCmdDrawMultiIndexedEXT-None-07642";
        dynamic_coverage_to_color_location = "VUID-vkCmdDrawMultiIndexedEXT-None-07643";
        dynamic_coverage_modulation_mode   = "VUID-vkCmdDrawMultiIndexedEXT-None-07644";
        dynamic_coverage_modulation_table_enable = "VUID-vkCmdDrawMultiIndexedEXT-None-07645";
        dynamic_coverage_modulation_table  = "VUID-vkCmdDrawMultiIndexedEXT-None-07646";
        dynamic_coverage_reduction_mode    = "VUID-vkCmdDrawMultiIndexedEXT-None-07647";
        dynamic_representative_fragment_test_enable  = "VUID-vkCmdDrawMultiIndexedEXT-None-07648";
        dynamic_shading_rate_image_enable  = "VUID-vkCmdDrawMultiIndexedEXT-None-07649";
        dynamic_viewport                   = "VUID-vkCmdDrawMultiIndexedEXT-None-07831";
        dynamic_scissor                    = "VUID-vkCmdDrawMultiIndexedEXT-None-07832";
        dynamic_depth_bias                 = "VUID-vkCmdDrawMultiIndexedEXT-None-07834";
        dynamic_line_width                 = "VUID-vkCmdDrawMultiIndexedEXT-None-07833";
        dynamic_line_stipple_ext           = "VUID-vkCmdDrawMultiIndexedEXT-None-07849";
        dynamic_blend_constants            = "VUID-vkCmdDrawMultiIndexedEXT-None-07835";
        dynamic_depth_bounds               = "VUID-vkCmdDrawMultiIndexedEXT-None-07836";
        dynamic_stencil_compare_mask       = "VUID-vkCmdDrawMultiIndexedEXT-None-07837";
        dynamic_stencil_write_mask         = "VUID-vkCmdDrawMultiIndexedEXT-None-07838";
        dynamic_stencil_reference          = "VUID-vkCmdDrawMultiIndexedEXT-None-07839";
        dynamic_state_inherited            = "VUID-vkCmdDrawMultiIndexedEXT-None-07850";
        primitives_generated               = "VUID-vkCmdDrawMultiIndexedEXT-primitivesGeneratedQueryWithRasterizerDiscard-06708";
        primitives_generated_streams       = "VUID-vkCmdDrawMultiIndexedEXT-primitivesGeneratedQueryWithNonZeroStreams-06709";
        invalid_mesh_shader_stages         = "VUID-vkCmdDrawMultiIndexedEXT-stage-06481";
        descriptor_buffer_bit_set          = "VUID-vkCmdDrawMultiIndexedEXT-None-08114";
        descriptor_buffer_bit_not_set      = "VUID-vkCmdDrawMultiIndexedEXT-None-08115";
        descriptor_buffer_set_offset_missing = "VUID-vkCmdDrawMultiIndexedEXT-None-08117";
        image_view_dim                     = "VUID-vkCmdDrawMultiIndexedEXT-viewType-07752";
        image_view_numeric_format          = "VUID-vkCmdDrawMultiIndexedEXT-format-07753";
        stippled_rectangular_lines         = "VUID-vkCmdDrawMultiIndexedEXT-stippledLineEnable-07495";
        stippled_bresenham_lines           = "VUID-vkCmdDrawMultiIndexedEXT-stippledLineEnable-07496";
        stippled_smooth_lines              = "VUID-vkCmdDrawMultiIndexedEXT-stippledLineEnable-07497";
        stippled_default_strict            = "VUID-vkCmdDrawMultiIndexedEXT-stippledLineEnable-07498";
    }
};

struct DispatchVuidsCmdDrawIndirect : DrawDispatchVuid {
    DispatchVuidsCmdDrawIndirect() : DrawDispatchVuid() {
        pipeline_bound                     = "VUID-vkCmdDrawIndirect-None-02700";
        vertex_binding                     = "VUID-vkCmdDrawIndirect-None-04007";
        vertex_binding_null                = "VUID-vkCmdDrawIndirect-None-04008";
        compatible_pipeline                = "VUID-vkCmdDrawIndirect-None-02697";
        render_pass_compatible             = "VUID-vkCmdDrawIndirect-renderPass-02684";
        subpass_index                      = "VUID-vkCmdDrawIndirect-subpass-02685";
        sample_location                    = "VUID-vkCmdDrawIndirect-sampleLocationsEnable-02689";
        linear_filter_sampler              = "VUID-vkCmdDrawIndirect-magFilter-04553";
        linear_mipmap_sampler              = "VUID-vkCmdDrawIndirect-mipmapMode-04770";
        cubic_sampler                      = "VUID-vkCmdDrawIndirect-None-02692";
        indirect_protected_cb              = "VUID-vkCmdDrawIndirect-commandBuffer-02711";
        indirect_contiguous_memory         = "VUID-vkCmdDrawIndirect-buffer-02708";
        indirect_buffer_bit                = "VUID-vkCmdDrawIndirect-buffer-02709";
        viewport_count                     = "VUID-vkCmdDrawIndirect-viewportCount-03417";
        scissor_count                      = "VUID-vkCmdDrawIndirect-scissorCount-03418";
        viewport_scissor_count             = "VUID-vkCmdDrawIndirect-viewportCount-03419";
        primitive_topology_class           = "VUID-vkCmdDrawIndirect-primitiveTopology-03420";
        primitive_topology_class_ds3       = "VUID-vkCmdDrawIndirect-dynamicPrimitiveTopologyUnrestricted-07500";
        dynamic_primitive_topology         = "VUID-vkCmdDrawIndirect-None-07842";
        corner_sampled_address_mode        = "VUID-vkCmdDrawIndirect-flags-02696";
        imageview_atomic                   = "VUID-vkCmdDrawIndirect-None-02691";
        push_constants_set                 = "VUID-vkCmdDrawIndirect-maintenance4-06425";
        image_subresources_render_pass_write = "VUID-vkCmdDrawIndirect-None-06537";
        image_subresources_subpass_read    = "VUID-vkCmdDrawIndirect-None-06538";
        image_subresources_subpass_write   = "VUID-vkCmdDrawIndirect-None-06539";
        descriptor_valid                   = "VUID-vkCmdDrawIndirect-None-02699";
        sampler_imageview_type             = "VUID-vkCmdDrawIndirect-None-02702";
        sampler_implicitLod_dref_proj      = "VUID-vkCmdDrawIndirect-None-02703";
        sampler_bias_offset                = "VUID-vkCmdDrawIndirect-None-02704";
        vertex_binding_attribute           = "VUID-vkCmdDrawIndirect-None-02721";
        dynamic_state_setting_commands     = "VUID-vkCmdDrawIndirect-None-02859";
        rasterization_samples              = "VUID-vkCmdDrawIndirect-rasterizationSamples-04740";
        msrtss_rasterization_samples       = "VUID-vkCmdDrawIndirect-multisampledRenderToSingleSampled-07284";
        unprotected_command_buffer         = "VUID-vkCmdDrawIndirect-commandBuffer-02707";
        max_multiview_instance_index       = "VUID-vkCmdDrawIndirect-maxMultiviewInstanceIndex-02688";
        img_filter_cubic                   = "VUID-vkCmdDrawIndirect-None-02693";
        filter_cubic                       = "VUID-vkCmdDrawIndirect-filterCubic-02694";
        filter_cubic_min_max               = "VUID-vkCmdDrawIndirect-filterCubicMinmax-02695";
        viewport_count_primitive_shading_rate = "VUID-vkCmdDrawIndirect-primitiveFragmentShadingRateWithMultipleViewports-04552";
        patch_control_points               = "VUID-vkCmdDrawIndirect-None-04875";
        rasterizer_discard_enable          = "VUID-vkCmdDrawIndirect-None-04876";
        depth_bias_enable                  = "VUID-vkCmdDrawIndirect-None-04877";
        logic_op                           = "VUID-vkCmdDrawIndirect-logicOp-04878";
        primitive_restart_enable           = "VUID-vkCmdDrawIndirect-None-04879";
        vertex_input_binding_stride        = "VUID-vkCmdDrawIndirect-pStrides-04884";
        vertex_input                       = "VUID-vkCmdDrawIndirect-None-04914";
        blend_enable                       = "VUID-vkCmdDrawIndirect-blendEnable-04727";
        dynamic_discard_rectangle          = "VUID-vkCmdDrawIndirect-None-07751";
        dynamic_color_write_enable         = "VUID-vkCmdDrawIndirect-None-07749";
        dynamic_color_write_enable_count   = "VUID-vkCmdDrawIndirect-attachmentCount-07750";
        dynamic_rendering_view_mask        = "VUID-vkCmdDrawIndirect-viewMask-06178";
        dynamic_rendering_color_count      = "VUID-vkCmdDrawIndirect-colorAttachmentCount-06179";
        dynamic_rendering_color_formats    = "VUID-vkCmdDrawIndirect-colorAttachmentCount-06180";
        dynamic_rendering_depth_format     = "VUID-vkCmdDrawIndirect-pDepthAttachment-06181";
        dynamic_rendering_stencil_format   = "VUID-vkCmdDrawIndirect-pStencilAttachment-06182";
        dynamic_rendering_fsr              = "VUID-vkCmdDrawIndirect-imageView-06183";
        dynamic_rendering_fdm              = "VUID-vkCmdDrawIndirect-imageView-06184";
        dynamic_rendering_color_sample     = "VUID-vkCmdDrawIndirect-colorAttachmentCount-06185";
        dynamic_rendering_depth_sample     = "VUID-vkCmdDrawIndirect-pDepthAttachment-06186";
        dynamic_rendering_stencil_sample   = "VUID-vkCmdDrawIndirect-pStencilAttachment-06187";
        dynamic_rendering_multi_sample     = "VUID-vkCmdDrawIndirect-colorAttachmentCount-06188";
        dynamic_rendering_06189            = "VUID-vkCmdDrawIndirect-pDepthAttachment-06189";
        dynamic_rendering_06190            = "VUID-vkCmdDrawIndirect-pStencilAttachment-06190";
        dynamic_rendering_06198            = "VUID-vkCmdDrawIndirect-renderPass-06198";
        dynamic_rendering_07285            = "VUID-vkCmdDrawIndirect-multisampledRenderToSingleSampled-07285";
        dynamic_rendering_07286            = "VUID-vkCmdDrawIndirect-multisampledRenderToSingleSampled-07286";
        dynamic_rendering_07287            = "VUID-vkCmdDrawIndirect-multisampledRenderToSingleSampled-07287";
        image_view_access_64               = "VUID-vkCmdDrawIndirect-SampledType-04470";
        image_view_access_32               = "VUID-vkCmdDrawIndirect-SampledType-04471";
        buffer_view_access_64              = "VUID-vkCmdDrawIndirect-SampledType-04472";
        buffer_view_access_32              = "VUID-vkCmdDrawIndirect-SampledType-04473";
        storage_image_read_without_format  = "VUID-vkCmdDrawIndirect-OpTypeImage-07028";
        storage_image_write_without_format = "VUID-vkCmdDrawIndirect-OpTypeImage-07027";
        storage_texel_buffer_read_without_format  = "VUID-vkCmdDrawIndirect-OpTypeImage-07030";
        storage_texel_buffer_write_without_format = "VUID-vkCmdDrawIndirect-OpTypeImage-07029";
        storage_image_write_texel_count           = "VUID-vkCmdDrawIndirect-None-04115";
        storage_texel_buffer_write_texel_count    = "VUID-vkCmdDrawIndirect-OpImageWrite-04469";
        depth_compare_sample               = "VUID-vkCmdDrawIndirect-None-06479";
        depth_read_only                    = "VUID-vkCmdDrawIndirect-None-06886";
        stencil_read_only                  = "VUID-vkCmdDrawIndirect-None-06887";
        dynamic_sample_locations           = "VUID-vkCmdDrawIndirect-None-06666";
        dynamic_tessellation_domain_origin = "VUID-vkCmdDrawIndirect-None-07619";
        dynamic_depth_clamp_enable         = "VUID-vkCmdDrawIndirect-None-07620";
        dynamic_polygon_mode               = "VUID-vkCmdDrawIndirect-None-07621";
        dynamic_rasterization_samples      = "VUID-vkCmdDrawIndirect-None-07622";
        dynamic_sample_mask                = "VUID-vkCmdDrawIndirect-None-07623";
        dynamic_alpha_to_coverage_enable   = "VUID-vkCmdDrawIndirect-None-07624";
        dynamic_alpha_to_one_enable        = "VUID-vkCmdDrawIndirect-None-07625";
        dynamic_logic_op_enable            = "VUID-vkCmdDrawIndirect-None-07626";
        dynamic_color_blend_enable         = "VUID-vkCmdDrawIndirect-firstAttachment-07476";
        dynamic_color_blend_equation       = "VUID-vkCmdDrawIndirect-firstAttachment-07477";
        dynamic_color_write_mask           = "VUID-vkCmdDrawIndirect-firstAttachment-07478";
        dynamic_rasterization_stream       = "VUID-vkCmdDrawIndirect-None-07630";
        dynamic_conservative_rasterization_mode = "VUID-vkCmdDrawIndirect-None-07631";
        dynamic_extra_primitive_overestimation_size = "VUID-vkCmdDrawIndirect-None-07632";
        dynamic_depth_clip_enable          = "VUID-vkCmdDrawIndirect-None-07633";
        dynamic_sample_locations_enable    = "VUID-vkCmdDrawIndirect-None-07634";
        dynamic_color_blend_advanced       = "VUID-vkCmdDrawIndirect-firstAttachment-07479";
        dynamic_provoking_vertex_mode      = "VUID-vkCmdDrawIndirect-None-07636";
        dynamic_line_rasterization_mode    = "VUID-vkCmdDrawIndirect-None-07637";
        dynamic_line_stipple_enable        = "VUID-vkCmdDrawIndirect-None-07638";
        dynamic_depth_clip_negative_one_to_one = "VUID-vkCmdDrawIndirect-None-07639";
        dynamic_viewport_w_scaling_enable  = "VUID-vkCmdDrawIndirect-None-07640";
        dynamic_viewport_swizzle           = "VUID-vkCmdDrawIndirect-None-07641";
        dynamic_coverage_to_color_enable   = "VUID-vkCmdDrawIndirect-None-07642";
        dynamic_coverage_to_color_location = "VUID-vkCmdDrawIndirect-None-07643";
        dynamic_coverage_modulation_mode   = "VUID-vkCmdDrawIndirect-None-07644";
        dynamic_coverage_modulation_table_enable = "VUID-vkCmdDrawIndirect-None-07645";
        dynamic_coverage_modulation_table  = "VUID-vkCmdDrawIndirect-None-07646";
        dynamic_coverage_reduction_mode    = "VUID-vkCmdDrawIndirect-None-07647";
        dynamic_representative_fragment_test_enable  = "VUID-vkCmdDrawIndirect-None-07648";
        dynamic_shading_rate_image_enable  = "VUID-vkCmdDrawIndirect-None-07649";
        dynamic_viewport                   = "VUID-vkCmdDrawIndirect-None-07831";
        dynamic_scissor                    = "VUID-vkCmdDrawIndirect-None-07832";
        dynamic_depth_bias                 = "VUID-vkCmdDrawIndirect-None-07834";
        dynamic_line_width                 = "VUID-vkCmdDrawIndirect-None-07833";
        dynamic_line_stipple_ext           = "VUID-vkCmdDrawIndirect-None-07849";
        dynamic_blend_constants            = "VUID-vkCmdDrawIndirect-None-07835";
        dynamic_depth_bounds               = "VUID-vkCmdDrawIndirect-None-07836";
        dynamic_stencil_compare_mask       = "VUID-vkCmdDrawIndirect-None-07837";
        dynamic_stencil_write_mask         = "VUID-vkCmdDrawIndirect-None-07838";
        dynamic_stencil_reference          = "VUID-vkCmdDrawIndirect-None-07839";
        dynamic_state_inherited            = "VUID-vkCmdDrawIndirect-None-07850";
        primitives_generated               = "VUID-vkCmdDrawIndirect-primitivesGeneratedQueryWithRasterizerDiscard-06708";
        primitives_generated_streams       = "VUID-vkCmdDrawIndirect-primitivesGeneratedQueryWithNonZeroStreams-06709";
        invalid_mesh_shader_stages         = "VUID-vkCmdDrawIndirect-stage-06481";
        descriptor_buffer_bit_set          = "VUID-vkCmdDrawIndirect-None-08114";
        descriptor_buffer_bit_not_set      = "VUID-vkCmdDrawIndirect-None-08115";
        descriptor_buffer_set_offset_missing = "VUID-vkCmdDrawIndirect-None-08117";
        image_view_dim                     = "VUID-vkCmdDrawIndirect-viewType-07752";
        image_view_numeric_format          = "VUID-vkCmdDrawIndirect-format-07753";
        stippled_rectangular_lines         = "VUID-vkCmdDrawIndirect-stippledLineEnable-07495";
        stippled_bresenham_lines           = "VUID-vkCmdDrawIndirect-stippledLineEnable-07496";
        stippled_smooth_lines              = "VUID-vkCmdDrawIndirect-stippledLineEnable-07497";
        stippled_default_strict            = "VUID-vkCmdDrawIndirect-stippledLineEnable-07498";
    }
};

struct DispatchVuidsCmdDrawIndexedIndirect : DrawDispatchVuid {
    DispatchVuidsCmdDrawIndexedIndirect() : DrawDispatchVuid() {
        pipeline_bound                     = "VUID-vkCmdDrawIndexedIndirect-None-02700";
        index_binding                      = "VUID-vkCmdDrawIndexedIndirect-None-07312";
        vertex_binding                     = "VUID-vkCmdDrawIndexedIndirect-None-04007";
        vertex_binding_null                = "VUID-vkCmdDrawIndexedIndirect-None-04008";
        compatible_pipeline                = "VUID-vkCmdDrawIndexedIndirect-None-02697";
        render_pass_compatible             = "VUID-vkCmdDrawIndexedIndirect-renderPass-02684";
        subpass_index                      = "VUID-vkCmdDrawIndexedIndirect-subpass-02685";
        sample_location                    = "VUID-vkCmdDrawIndexedIndirect-sampleLocationsEnable-02689";
        linear_filter_sampler              = "VUID-vkCmdDrawIndexedIndirect-magFilter-04553";
        linear_mipmap_sampler              = "VUID-vkCmdDrawIndexedIndirect-mipmapMode-04770";
        cubic_sampler                      = "VUID-vkCmdDrawIndexedIndirect-None-02692";
        indirect_protected_cb              = "VUID-vkCmdDrawIndexedIndirect-commandBuffer-02711";
        indirect_contiguous_memory         = "VUID-vkCmdDrawIndexedIndirect-buffer-02708";
        indirect_buffer_bit                = "VUID-vkCmdDrawIndexedIndirect-buffer-02709";
        viewport_count                     = "VUID-vkCmdDrawIndexedIndirect-viewportCount-03417";
        scissor_count                      = "VUID-vkCmdDrawIndexedIndirect-scissorCount-03418";
        viewport_scissor_count             = "VUID-vkCmdDrawIndexedIndirect-viewportCount-03419";
        primitive_topology_class           = "VUID-vkCmdDrawIndexedIndirect-primitiveTopology-03420";
        primitive_topology_class_ds3       = "VUID-vkCmdDrawIndexedIndirect-dynamicPrimitiveTopologyUnrestricted-07500";
        dynamic_primitive_topology         = "VUID-vkCmdDrawIndexedIndirect-None-07842";
        corner_sampled_address_mode        = "VUID-vkCmdDrawIndexedIndirect-flags-02696";
        imageview_atomic                   = "VUID-vkCmdDrawIndexedIndirect-None-02691";
        push_constants_set                 = "VUID-vkCmdDrawIndexedIndirect-maintenance4-06425";
        image_subresources_render_pass_write = "VUID-vkCmdDrawIndexedIndirect-None-06537";
        image_subresources_subpass_read    = "VUID-vkCmdDrawIndexedIndirect-None-06538";
        image_subresources_subpass_write   = "VUID-vkCmdDrawIndexedIndirect-None-06539";
        descriptor_valid                   = "VUID-vkCmdDrawIndexedIndirect-None-02699";
        sampler_imageview_type             = "VUID-vkCmdDrawIndexedIndirect-None-02702";
        sampler_implicitLod_dref_proj      = "VUID-vkCmdDrawIndexedIndirect-None-02703";
        sampler_bias_offset                = "VUID-vkCmdDrawIndexedIndirect-None-02704";
        vertex_binding_attribute           = "VUID-vkCmdDrawIndexedIndirect-None-02721";
        dynamic_state_setting_commands     = "VUID-vkCmdDrawIndexedIndirect-None-02859";
        rasterization_samples              = "VUID-vkCmdDrawIndexedIndirect-rasterizationSamples-04740";
        msrtss_rasterization_samples       = "VUID-vkCmdDrawIndexedIndirect-multisampledRenderToSingleSampled-07284";
        unprotected_command_buffer         = "VUID-vkCmdDrawIndexedIndirect-commandBuffer-02707";
        max_multiview_instance_index       = "VUID-vkCmdDrawIndexedIndirect-maxMultiviewInstanceIndex-02688";
        img_filter_cubic                   = "VUID-vkCmdDrawIndexedIndirect-None-02693";
        filter_cubic                       = "VUID-vkCmdDrawIndexedIndirect-filterCubic-02694";
        filter_cubic_min_max               = "VUID-vkCmdDrawIndexedIndirect-filterCubicMinmax-02695";
        viewport_count_primitive_shading_rate = "VUID-vkCmdDrawIndexedIndirect-primitiveFragmentShadingRateWithMultipleViewports-04552";
        patch_control_points               = "VUID-vkCmdDrawIndexedIndirect-None-04875";
        rasterizer_discard_enable          = "VUID-vkCmdDrawIndexedIndirect-None-04876";
        depth_bias_enable                  = "VUID-vkCmdDrawIndexedIndirect-None-04877";
        logic_op                           = "VUID-vkCmdDrawIndexedIndirect-logicOp-04878";
        primitive_restart_enable           = "VUID-vkCmdDrawIndexedIndirect-None-04879";
        vertex_input_binding_stride        = "VUID-vkCmdDrawIndexedIndirect-pStrides-04884";
        vertex_input                       = "VUID-vkCmdDrawIndexedIndirect-None-04914";
        blend_enable                       = "VUID-vkCmdDrawIndexedIndirect-blendEnable-04727";
        dynamic_discard_rectangle          = "VUID-vkCmdDrawIndexedIndirect-None-07751";
        dynamic_color_write_enable         = "VUID-vkCmdDrawIndexedIndirect-None-07749";
        dynamic_color_write_enable_count   = "VUID-vkCmdDrawIndexedIndirect-attachmentCount-07750";
        dynamic_rendering_view_mask        = "VUID-vkCmdDrawIndexedIndirect-viewMask-06178";
        dynamic_rendering_color_count      = "VUID-vkCmdDrawIndexedIndirect-colorAttachmentCount-06179";
        dynamic_rendering_color_formats    = "VUID-vkCmdDrawIndexedIndirect-colorAttachmentCount-06180";
        dynamic_rendering_depth_format     = "VUID-vkCmdDrawIndexedIndirect-pDepthAttachment-06181";
        dynamic_rendering_stencil_format   = "VUID-vkCmdDrawIndexedIndirect-pStencilAttachment-06182";
        dynamic_rendering_fsr              = "VUID-vkCmdDrawIndexedIndirect-imageView-06183";
        dynamic_rendering_fdm              = "VUID-vkCmdDrawIndexedIndirect-imageView-06184";
        dynamic_rendering_color_sample     = "VUID-vkCmdDrawIndexedIndirect-colorAttachmentCount-06185";
        dynamic_rendering_depth_sample     = "VUID-vkCmdDrawIndexedIndirect-pDepthAttachment-06186";
        dynamic_rendering_stencil_sample   = "VUID-vkCmdDrawIndexedIndirect-pStencilAttachment-06187";
        dynamic_rendering_multi_sample     = "VUID-vkCmdDrawIndexedIndirect-colorAttachmentCount-06188";
        dynamic_rendering_06189            = "VUID-vkCmdDrawIndexedIndirect-pDepthAttachment-06189";
        dynamic_rendering_06190            = "VUID-vkCmdDrawIndexedIndirect-pStencilAttachment-06190";
        dynamic_rendering_06198            = "VUID-vkCmdDrawIndexedIndirect-renderPass-06198";
        dynamic_rendering_07285            = "VUID-vkCmdDrawIndexedIndirect-multisampledRenderToSingleSampled-07285";
        dynamic_rendering_07286            = "VUID-vkCmdDrawIndexedIndirect-multisampledRenderToSingleSampled-07286";
        dynamic_rendering_07287            = "VUID-vkCmdDrawIndexedIndirect-multisampledRenderToSingleSampled-07287";
        image_view_access_64               = "VUID-vkCmdDrawIndexedIndirect-SampledType-04470";
        image_view_access_32               = "VUID-vkCmdDrawIndexedIndirect-SampledType-04471";
        buffer_view_access_64              = "VUID-vkCmdDrawIndexedIndirect-SampledType-04472";
        buffer_view_access_32              = "VUID-vkCmdDrawIndexedIndirect-SampledType-04473";
        storage_image_read_without_format  = "VUID-vkCmdDrawIndexedIndirect-OpTypeImage-07028";
        storage_image_write_without_format = "VUID-vkCmdDrawIndexedIndirect-OpTypeImage-07027";
        storage_texel_buffer_read_without_format  = "VUID-vkCmdDrawIndexedIndirect-OpTypeImage-07030";
        storage_texel_buffer_write_without_format = "VUID-vkCmdDrawIndexedIndirect-OpTypeImage-07029";
        storage_image_write_texel_count           = "VUID-vkCmdDrawIndexedIndirect-None-04115";
        storage_texel_buffer_write_texel_count    = "VUID-vkCmdDrawIndexedIndirect-OpImageWrite-04469";
        depth_compare_sample               = "VUID-vkCmdDrawIndexedIndirect-None-06479";
        depth_read_only                    = "VUID-vkCmdDrawIndexedIndirect-None-06886";
        stencil_read_only                  = "VUID-vkCmdDrawIndexedIndirect-None-06887";
        dynamic_sample_locations           = "VUID-vkCmdDrawIndexedIndirect-None-06666";
        dynamic_tessellation_domain_origin = "VUID-vkCmdDrawIndexedIndirect-None-07619";
        dynamic_depth_clamp_enable         = "VUID-vkCmdDrawIndexedIndirect-None-07620";
        dynamic_polygon_mode               = "VUID-vkCmdDrawIndexedIndirect-None-07621";
        dynamic_rasterization_samples      = "VUID-vkCmdDrawIndexedIndirect-None-07622";
        dynamic_sample_mask                = "VUID-vkCmdDrawIndexedIndirect-None-07623";
        dynamic_alpha_to_coverage_enable   = "VUID-vkCmdDrawIndexedIndirect-None-07624";
        dynamic_alpha_to_one_enable        = "VUID-vkCmdDrawIndexedIndirect-None-07625";
        dynamic_logic_op_enable            = "VUID-vkCmdDrawIndexedIndirect-None-07626";
        dynamic_color_blend_enable         = "VUID-vkCmdDrawIndexedIndirect-firstAttachment-07476";
        dynamic_color_blend_equation       = "VUID-vkCmdDrawIndexedIndirect-firstAttachment-07477";
        dynamic_color_write_mask           = "VUID-vkCmdDrawIndexedIndirect-firstAttachment-07478";
        dynamic_rasterization_stream       = "VUID-vkCmdDrawIndexedIndirect-None-07630";
        dynamic_conservative_rasterization_mode = "VUID-vkCmdDrawIndexedIndirect-None-07631";
        dynamic_extra_primitive_overestimation_size = "VUID-vkCmdDrawIndexedIndirect-None-07632";
        dynamic_depth_clip_enable          = "VUID-vkCmdDrawIndexedIndirect-None-07633";
        dynamic_sample_locations_enable    = "VUID-vkCmdDrawIndexedIndirect-None-07634";
        dynamic_color_blend_advanced       = "VUID-vkCmdDrawIndexedIndirect-firstAttachment-07479";
        dynamic_provoking_vertex_mode      = "VUID-vkCmdDrawIndexedIndirect-None-07636";
        dynamic_line_rasterization_mode    = "VUID-vkCmdDrawIndexedIndirect-None-07637";
        dynamic_line_stipple_enable        = "VUID-vkCmdDrawIndexedIndirect-None-07638";
        dynamic_depth_clip_negative_one_to_one = "VUID-vkCmdDrawIndexedIndirect-None-07639";
        dynamic_viewport_w_scaling_enable  = "VUID-vkCmdDrawIndexedIndirect-None-07640";
        dynamic_viewport_swizzle           = "VUID-vkCmdDrawIndexedIndirect-None-07641";
        dynamic_coverage_to_color_enable   = "VUID-vkCmdDrawIndexedIndirect-None-07642";
        dynamic_coverage_to_color_location = "VUID-vkCmdDrawIndexedIndirect-None-07643";
        dynamic_coverage_modulation_mode   = "VUID-vkCmdDrawIndexedIndirect-None-07644";
        dynamic_coverage_modulation_table_enable = "VUID-vkCmdDrawIndexedIndirect-None-07645";
        dynamic_coverage_modulation_table  = "VUID-vkCmdDrawIndexedIndirect-None-07646";
        dynamic_coverage_reduction_mode    = "VUID-vkCmdDrawIndexedIndirect-None-07647";
        dynamic_representative_fragment_test_enable  = "VUID-vkCmdDrawIndexedIndirect-None-07648";
        dynamic_shading_rate_image_enable  = "VUID-vkCmdDrawIndexedIndirect-None-07649";
        dynamic_viewport                   = "VUID-vkCmdDrawIndexedIndirect-None-07831";
        dynamic_scissor                    = "VUID-vkCmdDrawIndexedIndirect-None-07832";
        dynamic_depth_bias                 = "VUID-vkCmdDrawIndexedIndirect-None-07834";
        dynamic_line_width                 = "VUID-vkCmdDrawIndexedIndirect-None-07833";
        dynamic_line_stipple_ext           = "VUID-vkCmdDrawIndexedIndirect-None-07849";
        dynamic_blend_constants            = "VUID-vkCmdDrawIndexedIndirect-None-07835";
        dynamic_depth_bounds               = "VUID-vkCmdDrawIndexedIndirect-None-07836";
        dynamic_stencil_compare_mask       = "VUID-vkCmdDrawIndexedIndirect-None-07837";
        dynamic_stencil_write_mask         = "VUID-vkCmdDrawIndexedIndirect-None-07838";
        dynamic_stencil_reference          = "VUID-vkCmdDrawIndexedIndirect-None-07839";
        dynamic_state_inherited            = "VUID-vkCmdDrawIndexedIndirect-None-07850";
        primitives_generated               = "VUID-vkCmdDrawIndexedIndirect-primitivesGeneratedQueryWithRasterizerDiscard-06708";
        primitives_generated_streams       = "VUID-vkCmdDrawIndexedIndirect-primitivesGeneratedQueryWithNonZeroStreams-06709";
        invalid_mesh_shader_stages         = "VUID-vkCmdDrawIndexedIndirect-stage-06481";
        descriptor_buffer_bit_set          = "VUID-vkCmdDrawIndexedIndirect-None-08114";
        descriptor_buffer_bit_not_set      = "VUID-vkCmdDrawIndexedIndirect-None-08115";
        descriptor_buffer_set_offset_missing = "VUID-vkCmdDrawIndexedIndirect-None-08117";
        image_view_dim                     = "VUID-vkCmdDrawIndexedIndirect-viewType-07752";
        image_view_numeric_format          = "VUID-vkCmdDrawIndexedIndirect-format-07753";
        stippled_rectangular_lines         = "VUID-vkCmdDrawIndexedIndirect-stippledLineEnable-07495";
        stippled_bresenham_lines           = "VUID-vkCmdDrawIndexedIndirect-stippledLineEnable-07496";
        stippled_smooth_lines              = "VUID-vkCmdDrawIndexedIndirect-stippledLineEnable-07497";
        stippled_default_strict            = "VUID-vkCmdDrawIndexedIndirect-stippledLineEnable-07498";
    }
};

struct DispatchVuidsCmdDispatch : DrawDispatchVuid {
    DispatchVuidsCmdDispatch() : DrawDispatchVuid() {
        pipeline_bound                     = "VUID-vkCmdDispatch-None-02700";
        compatible_pipeline                = "VUID-vkCmdDispatch-None-02697";
        linear_filter_sampler              = "VUID-vkCmdDispatch-magFilter-04553";
        linear_mipmap_sampler              = "VUID-vkCmdDispatch-mipmapMode-04770";
        cubic_sampler                      = "VUID-vkCmdDispatch-None-02692";
        corner_sampled_address_mode        = "VUID-vkCmdDispatch-flags-02696";
        imageview_atomic                   = "VUID-vkCmdDispatch-None-02691";
        push_constants_set                 = "VUID-vkCmdDispatch-maintenance4-06425";
        descriptor_valid                   = "VUID-vkCmdDispatch-None-02699";
        sampler_imageview_type             = "VUID-vkCmdDispatch-None-02702";
        sampler_implicitLod_dref_proj      = "VUID-vkCmdDispatch-None-02703";
        sampler_bias_offset                = "VUID-vkCmdDispatch-None-02704";
        dynamic_state_setting_commands     = "VUID-vkCmdDispatch-None-02859";
        unprotected_command_buffer         = "VUID-vkCmdDispatch-commandBuffer-02707";
        protected_command_buffer           = "VUID-vkCmdDispatch-commandBuffer-02712";
        img_filter_cubic                   = "VUID-vkCmdDispatch-None-02693";
        filter_cubic                       = "VUID-vkCmdDispatch-filterCubic-02694";
        filter_cubic_min_max               = "VUID-vkCmdDispatch-filterCubicMinmax-02695";
        image_view_access_64               = "VUID-vkCmdDispatch-SampledType-04470";
        image_view_access_32               = "VUID-vkCmdDispatch-SampledType-04471";
        buffer_view_access_64              = "VUID-vkCmdDispatch-SampledType-04472";
        buffer_view_access_32              = "VUID-vkCmdDispatch-SampledType-04473";
        storage_image_read_without_format  = "VUID-vkCmdDispatch-OpTypeImage-07028";
        storage_image_write_without_format = "VUID-vkCmdDispatch-OpTypeImage-07027";
        storage_texel_buffer_read_without_format  = "VUID-vkCmdDispatch-OpTypeImage-07030";
        storage_texel_buffer_write_without_format = "VUID-vkCmdDispatch-OpTypeImage-07029";
        storage_image_write_texel_count           = "VUID-vkCmdDispatch-None-04115";
        storage_texel_buffer_write_texel_count    = "VUID-vkCmdDispatch-OpImageWrite-04469";
        depth_compare_sample               = "VUID-vkCmdDispatch-None-06479";
        descriptor_buffer_bit_set          = "VUID-vkCmdDispatch-None-08114";
        descriptor_buffer_bit_not_set      = "VUID-vkCmdDispatch-None-08115";
        descriptor_buffer_set_offset_missing = "VUID-vkCmdDispatch-None-08117";
        image_view_dim                     = "VUID-vkCmdDispatch-viewType-07752";
        image_view_numeric_format          = "VUID-vkCmdDispatch-format-07753";
    }
};

struct DispatchVuidsCmdDispatchIndirect : DrawDispatchVuid {
    DispatchVuidsCmdDispatchIndirect() : DrawDispatchVuid() {
        pipeline_bound                     = "VUID-vkCmdDispatchIndirect-None-02700";
        compatible_pipeline                = "VUID-vkCmdDispatchIndirect-None-02697";
        linear_filter_sampler              = "VUID-vkCmdDispatchIndirect-magFilter-04553";
        linear_mipmap_sampler              = "VUID-vkCmdDispatchIndirect-mipmapMode-04770";
        cubic_sampler                      = "VUID-vkCmdDispatchIndirect-None-02692";
        indirect_protected_cb              = "VUID-vkCmdDispatchIndirect-commandBuffer-02711";
        indirect_contiguous_memory         = "VUID-vkCmdDispatchIndirect-buffer-02708";
        indirect_buffer_bit                = "VUID-vkCmdDispatchIndirect-buffer-02709";
        corner_sampled_address_mode        = "VUID-vkCmdDispatchIndirect-flags-02696";
        imageview_atomic                   = "VUID-vkCmdDispatchIndirect-None-02691";
        push_constants_set                 = "VUID-vkCmdDispatchIndirect-maintenance4-06425";
        descriptor_valid                   = "VUID-vkCmdDispatchIndirect-None-02699";
        sampler_imageview_type             = "VUID-vkCmdDispatchIndirect-None-02702";
        sampler_implicitLod_dref_proj      = "VUID-vkCmdDispatchIndirect-None-02703";
        sampler_bias_offset                = "VUID-vkCmdDispatchIndirect-None-02704";
        dynamic_state_setting_commands     = "VUID-vkCmdDispatchIndirect-None-02859";
        unprotected_command_buffer         = "VUID-vkCmdDispatchIndirect-commandBuffer-02707";
        img_filter_cubic                   = "VUID-vkCmdDispatchIndirect-None-02693";
        filter_cubic                       = "VUID-vkCmdDispatchIndirect-filterCubic-02694";
        filter_cubic_min_max               = "VUID-vkCmdDispatchIndirect-filterCubicMinmax-02695";
        image_view_access_64               = "VUID-vkCmdDispatchIndirect-SampledType-04470";
        image_view_access_32               = "VUID-vkCmdDispatchIndirect-SampledType-04471";
        buffer_view_access_64              = "VUID-vkCmdDispatchIndirect-SampledType-04472";
        buffer_view_access_32              = "VUID-vkCmdDispatchIndirect-SampledType-04473";
        storage_image_read_without_format  = "VUID-vkCmdDispatchIndirect-OpTypeImage-07028";
        storage_image_write_without_format = "VUID-vkCmdDispatchIndirect-OpTypeImage-07027";
        storage_texel_buffer_read_without_format  = "VUID-vkCmdDispatchIndirect-OpTypeImage-07030";
        storage_texel_buffer_write_without_format = "VUID-vkCmdDispatchIndirect-OpTypeImage-07029";
        storage_image_write_texel_count           = "VUID-vkCmdDispatchIndirect-None-04115";
        storage_texel_buffer_write_texel_count    = "VUID-vkCmdDispatchIndirect-OpImageWrite-04469";
        depth_compare_sample               = "VUID-vkCmdDispatchIndirect-None-06479";
        descriptor_buffer_bit_set          = "VUID-vkCmdDispatchIndirect-None-08114";
        descriptor_buffer_bit_not_set      = "VUID-vkCmdDispatchIndirect-None-08115";
        descriptor_buffer_set_offset_missing = "VUID-vkCmdDispatchIndirect-None-08117";
        image_view_dim                     = "VUID-vkCmdDispatchIndirect-viewType-07752";
        image_view_numeric_format          = "VUID-vkCmdDispatchIndirect-format-07753";
    }
};

struct DispatchVuidsCmdDrawIndirectCount : DrawDispatchVuid {
    DispatchVuidsCmdDrawIndirectCount() : DrawDispatchVuid() {
        pipeline_bound                     = "VUID-vkCmdDrawIndirectCount-None-02700";
        vertex_binding                     = "VUID-vkCmdDrawIndirectCount-None-04007";
        vertex_binding_null                = "VUID-vkCmdDrawIndirectCount-None-04008";
        compatible_pipeline                = "VUID-vkCmdDrawIndirectCount-None-02697";
        render_pass_compatible             = "VUID-vkCmdDrawIndirectCount-renderPass-02684";
        subpass_index                      = "VUID-vkCmdDrawIndirectCount-subpass-02685";
        sample_location                    = "VUID-vkCmdDrawIndirectCount-sampleLocationsEnable-02689";
        linear_filter_sampler              = "VUID-vkCmdDrawIndirectCount-magFilter-04553";
        linear_mipmap_sampler              = "VUID-vkCmdDrawIndirectCount-mipmapMode-04770";
        cubic_sampler                      = "VUID-vkCmdDrawIndirectCount-None-02692";
        indirect_protected_cb              = "VUID-vkCmdDrawIndirectCount-commandBuffer-02711";
        indirect_contiguous_memory         = "VUID-vkCmdDrawIndirectCount-buffer-02708";
        indirect_count_contiguous_memory   = "VUID-vkCmdDrawIndirectCount-countBuffer-02714";
        indirect_buffer_bit                = "VUID-vkCmdDrawIndirectCount-buffer-02709";
        indirect_count_buffer_bit          = "VUID-vkCmdDrawIndirectCount-countBuffer-02715";
        indirect_count_offset              = "VUID-vkCmdDrawIndirectCount-countBufferOffset-04129";
        viewport_count                     = "VUID-vkCmdDrawIndirectCount-viewportCount-03417";
        scissor_count                      = "VUID-vkCmdDrawIndirectCount-scissorCount-03418";
        viewport_scissor_count             = "VUID-vkCmdDrawIndirectCount-viewportCount-03419";
        primitive_topology_class           = "VUID-vkCmdDrawIndirectCount-primitiveTopology-03420";
        primitive_topology_class_ds3       = "VUID-vkCmdDrawIndirectCount-dynamicPrimitiveTopologyUnrestricted-07500";
        dynamic_primitive_topology         = "VUID-vkCmdDrawIndirectCount-None-07842";
        corner_sampled_address_mode        = "VUID-vkCmdDrawIndirectCount-flags-02696";
        imageview_atomic                   = "VUID-vkCmdDrawIndirectCount-None-02691";
        push_constants_set                 = "VUID-vkCmdDrawIndirectCount-maintenance4-06425";
        image_subresources_render_pass_write = "VUID-vkCmdDrawIndirectCount-None-06537";
        image_subresources_subpass_read    = "VUID-vkCmdDrawIndirectCount-None-06538";
        image_subresources_subpass_write   = "VUID-vkCmdDrawIndirectCount-None-06539";
        descriptor_valid                   = "VUID-vkCmdDrawIndirectCount-None-02699";
        sampler_imageview_type             = "VUID-vkCmdDrawIndirectCount-None-02702";
        sampler_implicitLod_dref_proj      = "VUID-vkCmdDrawIndirectCount-None-02703";
        sampler_bias_offset                = "VUID-vkCmdDrawIndirectCount-None-02704";
        vertex_binding_attribute           = "VUID-vkCmdDrawIndirectCount-None-02721";
        dynamic_state_setting_commands     = "VUID-vkCmdDrawIndirectCount-None-02859";
        rasterization_samples              = "VUID-vkCmdDrawIndirectCount-rasterizationSamples-04740";
        msrtss_rasterization_samples       = "VUID-vkCmdDrawIndirectCount-multisampledRenderToSingleSampled-07284";
        unprotected_command_buffer         = "VUID-vkCmdDrawIndirectCount-commandBuffer-02707";
        max_multiview_instance_index       = "VUID-vkCmdDrawIndirectCount-maxMultiviewInstanceIndex-02688";
        img_filter_cubic                   = "VUID-vkCmdDrawIndirectCount-None-02693";
        filter_cubic                       = "VUID-vkCmdDrawIndirectCount-filterCubic-02694";
        filter_cubic_min_max               = "VUID-vkCmdDrawIndirectCount-filterCubicMinmax-02695";
        viewport_count_primitive_shading_rate = "VUID-vkCmdDrawIndirectCount-primitiveFragmentShadingRateWithMultipleViewports-04552";
        patch_control_points               = "VUID-vkCmdDrawIndirectCount-None-04875";
        rasterizer_discard_enable          = "VUID-vkCmdDrawIndirectCount-None-04876";
        depth_bias_enable                  = "VUID-vkCmdDrawIndirectCount-None-04877";
        logic_op                           = "VUID-vkCmdDrawIndirectCount-logicOp-04878";
        primitive_restart_enable           = "VUID-vkCmdDrawIndirectCount-None-04879";
        vertex_input_binding_stride        = "VUID-vkCmdDrawIndirectCount-pStrides-04884";
        vertex_input                       = "VUID-vkCmdDrawIndirectCount-None-04914";
        blend_enable                       = "VUID-vkCmdDrawIndirectCount-blendEnable-04727";
        dynamic_discard_rectangle          = "VUID-vkCmdDrawIndirectCount-None-07751";
        dynamic_color_write_enable         = "VUID-vkCmdDrawIndirectCount-None-07749";
        dynamic_color_write_enable_count   = "VUID-vkCmdDrawIndirectCount-attachmentCount-07750";
        dynamic_rendering_view_mask        = "VUID-vkCmdDrawIndirectCount-viewMask-06178";
        dynamic_rendering_color_count      = "VUID-vkCmdDrawIndirectCount-colorAttachmentCount-06179";
        dynamic_rendering_color_formats    = "VUID-vkCmdDrawIndirectCount-colorAttachmentCount-06180";
        dynamic_rendering_depth_format     = "VUID-vkCmdDrawIndirectCount-pDepthAttachment-06181";
        dynamic_rendering_stencil_format   = "VUID-vkCmdDrawIndirectCount-pStencilAttachment-06182";
        dynamic_rendering_fsr              = "VUID-vkCmdDrawIndirectCount-imageView-06183";
        dynamic_rendering_fdm              = "VUID-vkCmdDrawIndirectCount-imageView-06184";
        dynamic_rendering_color_sample     = "VUID-vkCmdDrawIndirectCount-colorAttachmentCount-06185";
        dynamic_rendering_depth_sample     = "VUID-vkCmdDrawIndirectCount-pDepthAttachment-06186";
        dynamic_rendering_stencil_sample   = "VUID-vkCmdDrawIndirectCount-pStencilAttachment-06187";
        dynamic_rendering_multi_sample     = "VUID-vkCmdDrawIndirectCount-colorAttachmentCount-06188";
        dynamic_rendering_06189            = "VUID-vkCmdDrawIndirectCount-pDepthAttachment-06189";
        dynamic_rendering_06190            = "VUID-vkCmdDrawIndirectCount-pStencilAttachment-06190";
        dynamic_rendering_06198            = "VUID-vkCmdDrawIndirectCount-renderPass-06198";
        dynamic_rendering_07285            = "VUID-vkCmdDrawIndirectCount-multisampledRenderToSingleSampled-07285";
        dynamic_rendering_07286            = "VUID-vkCmdDrawIndirectCount-multisampledRenderToSingleSampled-07286";
        dynamic_rendering_07287            = "VUID-vkCmdDrawIndirectCount-multisampledRenderToSingleSampled-07287";
        image_view_access_64               = "VUID-vkCmdDrawIndirectCount-SampledType-04470";
        image_view_access_32               = "VUID-vkCmdDrawIndirectCount-SampledType-04471";
        buffer_view_access_64              = "VUID-vkCmdDrawIndirectCount-SampledType-04472";
        buffer_view_access_32              = "VUID-vkCmdDrawIndirectCount-SampledType-04473";
        storage_image_read_without_format  = "VUID-vkCmdDrawIndirectCount-OpTypeImage-07028";
        storage_image_write_without_format = "VUID-vkCmdDrawIndirectCount-OpTypeImage-07027";
        storage_texel_buffer_read_without_format  = "VUID-vkCmdDrawIndirectCount-OpTypeImage-07030";
        storage_texel_buffer_write_without_format = "VUID-vkCmdDrawIndirectCount-OpTypeImage-07029";
        storage_image_write_texel_count           = "VUID-vkCmdDrawIndirectCount-None-04115";
        storage_texel_buffer_write_texel_count    = "VUID-vkCmdDrawIndirectCount-OpImageWrite-04469";
        depth_compare_sample               = "VUID-vkCmdDrawIndirectCount-None-06479";
        depth_read_only                    = "VUID-vkCmdDrawIndirectCount-None-06886";
        stencil_read_only                  = "VUID-vkCmdDrawIndirectCount-None-06887";
        dynamic_sample_locations           = "VUID-vkCmdDrawIndirectCount-None-06666";
        dynamic_tessellation_domain_origin = "VUID-vkCmdDrawIndirectCount-None-07619";
        dynamic_depth_clamp_enable         = "VUID-vkCmdDrawIndirectCount-None-07620";
        dynamic_polygon_mode               = "VUID-vkCmdDrawIndirectCount-None-07621";
        dynamic_rasterization_samples      = "VUID-vkCmdDrawIndirectCount-None-07622";
        dynamic_sample_mask                = "VUID-vkCmdDrawIndirectCount-None-07623";
        dynamic_alpha_to_coverage_enable   = "VUID-vkCmdDrawIndirectCount-None-07624";
        dynamic_alpha_to_one_enable        = "VUID-vkCmdDrawIndirectCount-None-07625";
        dynamic_logic_op_enable            = "VUID-vkCmdDrawIndirectCount-None-07626";
        dynamic_color_blend_enable         = "VUID-vkCmdDrawIndirectCount-firstAttachment-07476";
        dynamic_color_blend_equation       = "VUID-vkCmdDrawIndirectCount-firstAttachment-07477";
        dynamic_color_write_mask           = "VUID-vkCmdDrawIndirectCount-firstAttachment-07478";
        dynamic_rasterization_stream       = "VUID-vkCmdDrawIndirectCount-None-07630";
        dynamic_conservative_rasterization_mode = "VUID-vkCmdDrawIndirectCount-None-07631";
        dynamic_extra_primitive_overestimation_size = "VUID-vkCmdDrawIndirectCount-None-07632";
        dynamic_depth_clip_enable          = "VUID-vkCmdDrawIndirectCount-None-07633";
        dynamic_sample_locations_enable    = "VUID-vkCmdDrawIndirectCount-None-07634";
        dynamic_color_blend_advanced       = "VUID-vkCmdDrawIndirectCount-firstAttachment-07479";
        dynamic_provoking_vertex_mode      = "VUID-vkCmdDrawIndirectCount-None-07636";
        dynamic_line_rasterization_mode    = "VUID-vkCmdDrawIndirectCount-None-07637";
        dynamic_line_stipple_enable        = "VUID-vkCmdDrawIndirectCount-None-07638";
        dynamic_depth_clip_negative_one_to_one = "VUID-vkCmdDrawIndirectCount-None-07639";
        dynamic_viewport_w_scaling_enable  = "VUID-vkCmdDrawIndirectCount-None-07640";
        dynamic_viewport_swizzle           = "VUID-vkCmdDrawIndirectCount-None-07641";
        dynamic_coverage_to_color_enable   = "VUID-vkCmdDrawIndirectCount-None-07642";
        dynamic_coverage_to_color_location = "VUID-vkCmdDrawIndirectCount-None-07643";
        dynamic_coverage_modulation_mode   = "VUID-vkCmdDrawIndirectCount-None-07644";
        dynamic_coverage_modulation_table_enable = "VUID-vkCmdDrawIndirectCount-None-07645";
        dynamic_coverage_modulation_table  = "VUID-vkCmdDrawIndirectCount-None-07646";
        dynamic_coverage_reduction_mode    = "VUID-vkCmdDrawIndirectCount-None-07647";
        dynamic_representative_fragment_test_enable  = "VUID-vkCmdDrawIndirectCount-None-07648";
        dynamic_shading_rate_image_enable  = "VUID-vkCmdDrawIndirectCount-None-07649";
        dynamic_viewport                   = "VUID-vkCmdDrawIndirectCount-None-07831";
        dynamic_scissor                    = "VUID-vkCmdDrawIndirectCount-None-07832";
        dynamic_depth_bias                 = "VUID-vkCmdDrawIndirectCount-None-07834";
        dynamic_line_width                 = "VUID-vkCmdDrawIndirectCount-None-07833";
        dynamic_line_stipple_ext           = "VUID-vkCmdDrawIndirectCount-None-07849";
        dynamic_blend_constants            = "VUID-vkCmdDrawIndirectCount-None-07835";
        dynamic_depth_bounds               = "VUID-vkCmdDrawIndirectCount-None-07836";
        dynamic_stencil_compare_mask       = "VUID-vkCmdDrawIndirectCount-None-07837";
        dynamic_stencil_write_mask         = "VUID-vkCmdDrawIndirectCount-None-07838";
        dynamic_stencil_reference          = "VUID-vkCmdDrawIndirectCount-None-07839";
        dynamic_state_inherited            = "VUID-vkCmdDrawIndirectCount-None-07850";
        primitives_generated               = "VUID-vkCmdDrawIndirectCount-primitivesGeneratedQueryWithRasterizerDiscard-06708";
        primitives_generated_streams       = "VUID-vkCmdDrawIndirectCount-primitivesGeneratedQueryWithNonZeroStreams-06709";
        invalid_mesh_shader_stages         = "VUID-vkCmdDrawIndirectCount-stage-06481";
        descriptor_buffer_bit_set          = "VUID-vkCmdDrawIndirectCount-None-08114";
        descriptor_buffer_bit_not_set      = "VUID-vkCmdDrawIndirectCount-None-08115";
        descriptor_buffer_set_offset_missing = "VUID-vkCmdDrawIndirectCount-None-08117";
        image_view_dim                     = "VUID-vkCmdDrawIndirectCount-viewType-07752";
        image_view_numeric_format          = "VUID-vkCmdDrawIndirectCount-format-07753";
        stippled_rectangular_lines         = "VUID-vkCmdDrawIndirectCount-stippledLineEnable-07495";
        stippled_bresenham_lines           = "VUID-vkCmdDrawIndirectCount-stippledLineEnable-07496";
        stippled_smooth_lines              = "VUID-vkCmdDrawIndirectCount-stippledLineEnable-07497";
        stippled_default_strict            = "VUID-vkCmdDrawIndirectCount-stippledLineEnable-07498";
    }
};

struct DispatchVuidsCmdDrawIndexedIndirectCount : DrawDispatchVuid {
    DispatchVuidsCmdDrawIndexedIndirectCount() : DrawDispatchVuid() {
        pipeline_bound                     = "VUID-vkCmdDrawIndexedIndirectCount-None-02700";
        index_binding                      = "VUID-vkCmdDrawIndexedIndirectCount-None-07312";
        vertex_binding                     = "VUID-vkCmdDrawIndexedIndirectCount-None-04007";
        vertex_binding_null                = "VUID-vkCmdDrawIndexedIndirectCount-None-04008";
        compatible_pipeline                = "VUID-vkCmdDrawIndexedIndirectCount-None-02697";
        render_pass_compatible             = "VUID-vkCmdDrawIndexedIndirectCount-renderPass-02684";
        subpass_index                      = "VUID-vkCmdDrawIndexedIndirectCount-subpass-02685";
        sample_location                    = "VUID-vkCmdDrawIndexedIndirectCount-sampleLocationsEnable-02689";
        linear_filter_sampler              = "VUID-vkCmdDrawIndexedIndirectCount-magFilter-04553";
        linear_mipmap_sampler              = "VUID-vkCmdDrawIndexedIndirectCount-mipmapMode-04770";
        cubic_sampler                      = "VUID-vkCmdDrawIndexedIndirectCount-None-02692";
        indirect_protected_cb              = "VUID-vkCmdDrawIndexedIndirectCount-commandBuffer-02711";
        indirect_contiguous_memory         = "VUID-vkCmdDrawIndexedIndirectCount-buffer-02708";
        indirect_count_contiguous_memory   = "VUID-vkCmdDrawIndexedIndirectCount-countBuffer-02714";
        indirect_buffer_bit                = "VUID-vkCmdDrawIndexedIndirectCount-buffer-02709";
        indirect_count_buffer_bit          = "VUID-vkCmdDrawIndexedIndirectCount-countBuffer-02715";
        indirect_count_offset              = "VUID-vkCmdDrawIndexedIndirectCount-countBufferOffset-04129";
        viewport_count                     = "VUID-vkCmdDrawIndexedIndirectCount-viewportCount-03417";
        scissor_count                      = "VUID-vkCmdDrawIndexedIndirectCount-scissorCount-03418";
        viewport_scissor_count             = "VUID-vkCmdDrawIndexedIndirectCount-viewportCount-03419";
        primitive_topology_class           = "VUID-vkCmdDrawIndexedIndirectCount-primitiveTopology-03420";
        primitive_topology_class_ds3       = "VUID-vkCmdDrawIndexedIndirectCount-dynamicPrimitiveTopologyUnrestricted-07500";
        dynamic_primitive_topology         = "VUID-vkCmdDrawIndexedIndirectCount-None-07842";
        corner_sampled_address_mode        = "VUID-vkCmdDrawIndexedIndirectCount-flags-02696";
        imageview_atomic                   = "VUID-vkCmdDrawIndexedIndirectCount-None-02691";
        push_constants_set                 = "VUID-vkCmdDrawIndexedIndirectCount-maintenance4-06425";
        image_subresources_render_pass_write = "VUID-vkCmdDrawIndexedIndirectCount-None-06537";
        image_subresources_subpass_read    = "VUID-vkCmdDrawIndexedIndirectCount-None-06538";
        image_subresources_subpass_write   = "VUID-vkCmdDrawIndexedIndirectCount-None-06539";
        descriptor_valid                   = "VUID-vkCmdDrawIndexedIndirectCount-None-02699";
        sampler_imageview_type             = "VUID-vkCmdDrawIndexedIndirectCount-None-02702";
        sampler_implicitLod_dref_proj      = "VUID-vkCmdDrawIndexedIndirectCount-None-02703";
        sampler_bias_offset                = "VUID-vkCmdDrawIndexedIndirectCount-None-02704";
        vertex_binding_attribute           = "VUID-vkCmdDrawIndexedIndirectCount-None-02721";
        dynamic_state_setting_commands     = "VUID-vkCmdDrawIndexedIndirectCount-None-02859";
        rasterization_samples              = "VUID-vkCmdDrawIndexedIndirectCount-rasterizationSamples-04740";
        msrtss_rasterization_samples       = "VUID-vkCmdDrawIndexedIndirectCount-multisampledRenderToSingleSampled-07284";
        unprotected_command_buffer         = "VUID-vkCmdDrawIndexedIndirectCount-commandBuffer-02707";
        max_multiview_instance_index       = "VUID-vkCmdDrawIndexedIndirectCount-maxMultiviewInstanceIndex-02688";
        img_filter_cubic                   = "VUID-vkCmdDrawIndexedIndirectCount-None-02693";
        filter_cubic                       = "VUID-vkCmdDrawIndexedIndirectCount-filterCubic-02694";
        filter_cubic_min_max               = "VUID-vkCmdDrawIndexedIndirectCount-filterCubicMinmax-02695";
        viewport_count_primitive_shading_rate = "VUID-vkCmdDrawIndexedIndirectCount-primitiveFragmentShadingRateWithMultipleViewports-04552";
        patch_control_points               = "VUID-vkCmdDrawIndexedIndirectCount-None-04875";
        rasterizer_discard_enable          = "VUID-vkCmdDrawIndexedIndirectCount-None-04876";
        depth_bias_enable                  = "VUID-vkCmdDrawIndexedIndirectCount-None-04877";
        logic_op                           = "VUID-vkCmdDrawIndexedIndirectCount-logicOp-04878";
        primitive_restart_enable           = "VUID-vkCmdDrawIndexedIndirectCount-None-04879";
        vertex_input_binding_stride        = "VUID-vkCmdDrawIndexedIndirectCount-pStrides-04884";
        vertex_input                       = "VUID-vkCmdDrawIndexedIndirectCount-None-04914";
        blend_enable                       = "VUID-vkCmdDrawIndexedIndirectCount-blendEnable-04727";
        dynamic_discard_rectangle          = "VUID-vkCmdDrawIndexedIndirectCount-None-07751";
        dynamic_color_write_enable         = "VUID-vkCmdDrawIndexedIndirectCount-None-07749";
        dynamic_color_write_enable_count   = "VUID-vkCmdDrawIndexedIndirectCount-attachmentCount-07750";
        dynamic_rendering_view_mask        = "VUID-vkCmdDrawIndexedIndirectCount-viewMask-06178";
        dynamic_rendering_color_count      = "VUID-vkCmdDrawIndexedIndirectCount-colorAttachmentCount-06179";
        dynamic_rendering_color_formats    = "VUID-vkCmdDrawIndexedIndirectCount-colorAttachmentCount-06180";
        dynamic_rendering_depth_format     = "VUID-vkCmdDrawIndexedIndirectCount-pDepthAttachment-06181";
        dynamic_rendering_stencil_format   = "VUID-vkCmdDrawIndexedIndirectCount-pStencilAttachment-06182";
        dynamic_rendering_fsr              = "VUID-vkCmdDrawIndexedIndirectCount-imageView-06183";
        dynamic_rendering_fdm              = "VUID-vkCmdDrawIndexedIndirectCount-imageView-06184";
        dynamic_rendering_color_sample     = "VUID-vkCmdDrawIndexedIndirectCount-colorAttachmentCount-06185";
        dynamic_rendering_depth_sample     = "VUID-vkCmdDrawIndexedIndirectCount-pDepthAttachment-06186";
        dynamic_rendering_stencil_sample   = "VUID-vkCmdDrawIndexedIndirectCount-pStencilAttachment-06187";
        dynamic_rendering_multi_sample     = "VUID-vkCmdDrawIndexedIndirectCount-colorAttachmentCount-06188";
        dynamic_rendering_06189            = "VUID-vkCmdDrawIndexedIndirectCount-pDepthAttachment-06189";
        dynamic_rendering_06190            = "VUID-vkCmdDrawIndexedIndirectCount-pStencilAttachment-06190";
        dynamic_rendering_06198            = "VUID-vkCmdDrawIndexedIndirectCount-renderPass-06198";
        dynamic_rendering_07285            = "VUID-vkCmdDrawIndexedIndirectCount-multisampledRenderToSingleSampled-07285";
        dynamic_rendering_07286            = "VUID-vkCmdDrawIndexedIndirectCount-multisampledRenderToSingleSampled-07286";
        dynamic_rendering_07287            = "VUID-vkCmdDrawIndexedIndirectCount-multisampledRenderToSingleSampled-07287";
        image_view_access_64               = "VUID-vkCmdDrawIndexedIndirectCount-SampledType-04470";
        image_view_access_32               = "VUID-vkCmdDrawIndexedIndirectCount-SampledType-04471";
        buffer_view_access_64              = "VUID-vkCmdDrawIndexedIndirectCount-SampledType-04472";
        buffer_view_access_32              = "VUID-vkCmdDrawIndexedIndirectCount-SampledType-04473";
        storage_image_read_without_format  = "VUID-vkCmdDrawIndexedIndirectCount-OpTypeImage-07028";
        storage_image_write_without_format = "VUID-vkCmdDrawIndexedIndirectCount-OpTypeImage-07027";
        storage_texel_buffer_read_without_format  = "VUID-vkCmdDrawIndexedIndirectCount-OpTypeImage-07030";
        storage_texel_buffer_write_without_format = "VUID-vkCmdDrawIndexedIndirectCount-OpTypeImage-07029";
        storage_image_write_texel_count           = "VUID-vkCmdDrawIndexedIndirectCount-None-04115";
        storage_texel_buffer_write_texel_count    = "VUID-vkCmdDrawIndexedIndirectCount-OpImageWrite-04469";
        depth_compare_sample               = "VUID-vkCmdDrawIndexedIndirectCount-None-06479";
        depth_read_only                    = "VUID-vkCmdDrawIndexedIndirectCount-None-06886";
        stencil_read_only                  = "VUID-vkCmdDrawIndexedIndirectCount-None-06887";
        dynamic_sample_locations           = "VUID-vkCmdDrawIndexedIndirectCount-None-06666";
        dynamic_tessellation_domain_origin = "VUID-vkCmdDrawIndexedIndirectCount-None-07619";
        dynamic_depth_clamp_enable         = "VUID-vkCmdDrawIndexedIndirectCount-None-07620";
        dynamic_polygon_mode               = "VUID-vkCmdDrawIndexedIndirectCount-None-07621";
        dynamic_rasterization_samples      = "VUID-vkCmdDrawIndexedIndirectCount-None-07622";
        dynamic_sample_mask                = "VUID-vkCmdDrawIndexedIndirectCount-None-07623";
        dynamic_alpha_to_coverage_enable   = "VUID-vkCmdDrawIndexedIndirectCount-None-07624";
        dynamic_alpha_to_one_enable        = "VUID-vkCmdDrawIndexedIndirectCount-None-07625";
        dynamic_logic_op_enable            = "VUID-vkCmdDrawIndexedIndirectCount-None-07626";
        dynamic_color_blend_enable         = "VUID-vkCmdDrawIndexedIndirectCount-firstAttachment-07476";
        dynamic_color_blend_equation       = "VUID-vkCmdDrawIndexedIndirectCount-firstAttachment-07477";
        dynamic_color_write_mask           = "VUID-vkCmdDrawIndexedIndirectCount-firstAttachment-07478";
        dynamic_rasterization_stream       = "VUID-vkCmdDrawIndexedIndirectCount-None-07630";
        dynamic_conservative_rasterization_mode = "VUID-vkCmdDrawIndexedIndirectCount-None-07631";
        dynamic_extra_primitive_overestimation_size = "VUID-vkCmdDrawIndexedIndirectCount-None-07632";
        dynamic_depth_clip_enable          = "VUID-vkCmdDrawIndexedIndirectCount-None-07633";
        dynamic_sample_locations_enable    = "VUID-vkCmdDrawIndexedIndirectCount-None-07634";
        dynamic_color_blend_advanced       = "VUID-vkCmdDrawIndexedIndirectCount-firstAttachment-07479";
        dynamic_provoking_vertex_mode      = "VUID-vkCmdDrawIndexedIndirectCount-None-07636";
        dynamic_line_rasterization_mode    = "VUID-vkCmdDrawIndexedIndirectCount-None-07637";
        dynamic_line_stipple_enable        = "VUID-vkCmdDrawIndexedIndirectCount-None-07638";
        dynamic_depth_clip_negative_one_to_one = "VUID-vkCmdDrawIndexedIndirectCount-None-07639";
        dynamic_viewport_w_scaling_enable  = "VUID-vkCmdDrawIndexedIndirectCount-None-07640";
        dynamic_viewport_swizzle           = "VUID-vkCmdDrawIndexedIndirectCount-None-07641";
        dynamic_coverage_to_color_enable   = "VUID-vkCmdDrawIndexedIndirectCount-None-07642";
        dynamic_coverage_to_color_location = "VUID-vkCmdDrawIndexedIndirectCount-None-07643";
        dynamic_coverage_modulation_mode   = "VUID-vkCmdDrawIndexedIndirectCount-None-07644";
        dynamic_coverage_modulation_table_enable = "VUID-vkCmdDrawIndexedIndirectCount-None-07645";
        dynamic_coverage_modulation_table  = "VUID-vkCmdDrawIndexedIndirectCount-None-07646";
        dynamic_coverage_reduction_mode    = "VUID-vkCmdDrawIndexedIndirectCount-None-07647";
        dynamic_representative_fragment_test_enable  = "VUID-vkCmdDrawIndexedIndirectCount-None-07648";
        dynamic_shading_rate_image_enable  = "VUID-vkCmdDrawIndexedIndirectCount-None-07649";
        dynamic_viewport                   = "VUID-vkCmdDrawIndexedIndirectCount-None-07831";
        dynamic_scissor                    = "VUID-vkCmdDrawIndexedIndirectCount-None-07832";
        dynamic_depth_bias                 = "VUID-vkCmdDrawIndexedIndirectCount-None-07834";
        dynamic_line_width                 = "VUID-vkCmdDrawIndexedIndirectCount-None-07833";
        dynamic_line_stipple_ext           = "VUID-vkCmdDrawIndexedIndirectCount-None-07849";
        dynamic_blend_constants            = "VUID-vkCmdDrawIndexedIndirectCount-None-07835";
        dynamic_depth_bounds               = "VUID-vkCmdDrawIndexedIndirectCount-None-07836";
        dynamic_stencil_compare_mask       = "VUID-vkCmdDrawIndexedIndirectCount-None-07837";
        dynamic_stencil_write_mask         = "VUID-vkCmdDrawIndexedIndirectCount-None-07838";
        dynamic_stencil_reference          = "VUID-vkCmdDrawIndexedIndirectCount-None-07839";
        dynamic_state_inherited            = "VUID-vkCmdDrawIndexedIndirectCount-None-07850";
        primitives_generated               = "VUID-vkCmdDrawIndexedIndirectCount-primitivesGeneratedQueryWithRasterizerDiscard-06708";
        primitives_generated_streams       = "VUID-vkCmdDrawIndexedIndirectCount-primitivesGeneratedQueryWithNonZeroStreams-06709";
        invalid_mesh_shader_stages         = "VUID-vkCmdDrawIndexedIndirectCount-stage-06481";
        descriptor_buffer_bit_set          = "VUID-vkCmdDrawIndexedIndirectCount-None-08114";
        descriptor_buffer_bit_not_set      = "VUID-vkCmdDrawIndexedIndirectCount-None-08115";
        descriptor_buffer_set_offset_missing = "VUID-vkCmdDrawIndexedIndirectCount-None-08117";
        image_view_dim                     = "VUID-vkCmdDrawIndexedIndirectCount-viewType-07752";
        image_view_numeric_format          = "VUID-vkCmdDrawIndexedIndirectCount-format-07753";
        stippled_rectangular_lines         = "VUID-vkCmdDrawIndexedIndirectCount-stippledLineEnable-07495";
        stippled_bresenham_lines           = "VUID-vkCmdDrawIndexedIndirectCount-stippledLineEnable-07496";
        stippled_smooth_lines              = "VUID-vkCmdDrawIndexedIndirectCount-stippledLineEnable-07497";
        stippled_default_strict            = "VUID-vkCmdDrawIndexedIndirectCount-stippledLineEnable-07498";
    }
};

struct DispatchVuidsCmdTraceRaysNV: DrawDispatchVuid {
    DispatchVuidsCmdTraceRaysNV() : DrawDispatchVuid() {
        pipeline_bound                     = "VUID-vkCmdTraceRaysNV-None-02700";
        compatible_pipeline                = "VUID-vkCmdTraceRaysNV-None-02697";
        linear_filter_sampler              = "VUID-vkCmdTraceRaysNV-magFilter-04553";
        linear_mipmap_sampler              = "VUID-vkCmdTraceRaysNV-mipmapMode-04770";
        cubic_sampler                      = "VUID-vkCmdTraceRaysNV-None-02692";
        corner_sampled_address_mode        = "VUID-vkCmdTraceRaysNV-flags-02696";
        imageview_atomic                   = "VUID-vkCmdTraceRaysNV-None-02691";
        push_constants_set                 = "VUID-vkCmdTraceRaysNV-maintenance4-06425";
        descriptor_valid                   = "VUID-vkCmdTraceRaysNV-None-02699";
        sampler_imageview_type             = "VUID-vkCmdTraceRaysNV-None-02702";
        sampler_implicitLod_dref_proj      = "VUID-vkCmdTraceRaysNV-None-02703";
        sampler_bias_offset                = "VUID-vkCmdTraceRaysNV-None-02704";
        dynamic_state_setting_commands     = "VUID-vkCmdTraceRaysNV-None-02859";
        unprotected_command_buffer         = "VUID-vkCmdTraceRaysNV-commandBuffer-02707";
        ray_query_protected_cb             = "VUID-vkCmdTraceRaysNV-commandBuffer-04624";
        img_filter_cubic                   = "VUID-vkCmdTraceRaysNV-None-02693";
        filter_cubic                       = "VUID-vkCmdTraceRaysNV-filterCubic-02694";
        filter_cubic_min_max               = "VUID-vkCmdTraceRaysNV-filterCubicMinmax-02695";
        image_view_access_64               = "VUID-vkCmdTraceRaysNV-SampledType-04470";
        image_view_access_32               = "VUID-vkCmdTraceRaysNV-SampledType-04471";
        buffer_view_access_64              = "VUID-vkCmdTraceRaysNV-SampledType-04472";
        buffer_view_access_32              = "VUID-vkCmdTraceRaysNV-SampledType-04473";
        storage_image_read_without_format  = "VUID-vkCmdTraceRaysNV-OpTypeImage-07028";
        storage_image_write_without_format = "VUID-vkCmdTraceRaysNV-OpTypeImage-07027";
        storage_texel_buffer_read_without_format  = "VUID-vkCmdTraceRaysNV-OpTypeImage-07030";
        storage_texel_buffer_write_without_format = "VUID-vkCmdTraceRaysNV-OpTypeImage-07029";
        storage_image_write_texel_count           = "VUID-vkCmdTraceRaysNV-None-04115";
        storage_texel_buffer_write_texel_count    = "VUID-vkCmdTraceRaysNV-OpImageWrite-04469";
        depth_compare_sample               = "VUID-vkCmdTraceRaysNV-None-06479";
        descriptor_buffer_bit_set          = "VUID-vkCmdTraceRaysNV-None-08114";
        descriptor_buffer_bit_not_set      = "VUID-vkCmdTraceRaysNV-None-08115";
        descriptor_buffer_set_offset_missing = "VUID-vkCmdTraceRaysNV-None-08117";
        image_view_dim                     = "VUID-vkCmdTraceRaysNV-viewType-07752";
        image_view_numeric_format          = "VUID-vkCmdTraceRaysNV-format-07753";
    }
};

struct DispatchVuidsCmdTraceRaysKHR: DrawDispatchVuid {
    DispatchVuidsCmdTraceRaysKHR() : DrawDispatchVuid() {
        pipeline_bound                     = "VUID-vkCmdTraceRaysKHR-None-02700";
        compatible_pipeline                = "VUID-vkCmdTraceRaysKHR-None-02697";
        linear_filter_sampler              = "VUID-vkCmdTraceRaysKHR-magFilter-04553";
        linear_mipmap_sampler              = "VUID-vkCmdTraceRaysKHR-mipmapMode-04770";
        cubic_sampler                      = "VUID-vkCmdTraceRaysKHR-None-02692";
        corner_sampled_address_mode        = "VUID-vkCmdTraceRaysKHR-flags-02696";
        imageview_atomic                   = "VUID-vkCmdTraceRaysKHR-None-02691";
        push_constants_set                 = "VUID-vkCmdTraceRaysKHR-maintenance4-06425";
        descriptor_valid                   = "VUID-vkCmdTraceRaysKHR-None-02699";
        sampler_imageview_type             = "VUID-vkCmdTraceRaysKHR-None-02702";
        sampler_implicitLod_dref_proj      = "VUID-vkCmdTraceRaysKHR-None-02703";
        sampler_bias_offset                = "VUID-vkCmdTraceRaysKHR-None-02704";
        dynamic_state_setting_commands     = "VUID-vkCmdTraceRaysKHR-None-02859";
        unprotected_command_buffer         = "VUID-vkCmdTraceRaysKHR-commandBuffer-02707";
        ray_query_protected_cb             = "VUID-vkCmdTraceRaysKHR-commandBuffer-03635";
        img_filter_cubic                   = "VUID-vkCmdTraceRaysKHR-None-02693";
        filter_cubic                       = "VUID-vkCmdTraceRaysKHR-filterCubic-02694";
        filter_cubic_min_max               = "VUID-vkCmdTraceRaysKHR-filterCubicMinmax-02695";
        image_view_access_64               = "VUID-vkCmdTraceRaysKHR-SampledType-04470";
        image_view_access_32               = "VUID-vkCmdTraceRaysKHR-SampledType-04471";
        buffer_view_access_64              = "VUID-vkCmdTraceRaysKHR-SampledType-04472";
        buffer_view_access_32              = "VUID-vkCmdTraceRaysKHR-SampledType-04473";
        storage_image_read_without_format  = "VUID-vkCmdTraceRaysKHR-OpTypeImage-07028";
        storage_image_write_without_format = "VUID-vkCmdTraceRaysKHR-OpTypeImage-07027";
        storage_texel_buffer_read_without_format  = "VUID-vkCmdTraceRaysKHR-OpTypeImage-07030";
        storage_texel_buffer_write_without_format = "VUID-vkCmdTraceRaysKHR-OpTypeImage-07029";
        storage_image_write_texel_count           = "VUID-vkCmdTraceRaysKHR-None-04115";
        storage_texel_buffer_write_texel_count    = "VUID-vkCmdTraceRaysKHR-OpImageWrite-04469";
        depth_compare_sample               = "VUID-vkCmdTraceRaysKHR-None-06479";
        descriptor_buffer_bit_set          = "VUID-vkCmdTraceRaysKHR-None-08114";
        descriptor_buffer_bit_not_set      = "VUID-vkCmdTraceRaysKHR-None-08115";
        descriptor_buffer_set_offset_missing = "VUID-vkCmdTraceRaysKHR-None-08117";
        image_view_dim                     = "VUID-vkCmdTraceRaysKHR-viewType-07752";
        image_view_numeric_format          = "VUID-vkCmdTraceRaysKHR-format-07753";
    }
};

struct DispatchVuidsCmdTraceRaysIndirectKHR: DrawDispatchVuid {
    DispatchVuidsCmdTraceRaysIndirectKHR() : DrawDispatchVuid() {
        pipeline_bound                     = "VUID-vkCmdTraceRaysIndirectKHR-None-02700";
        compatible_pipeline                = "VUID-vkCmdTraceRaysIndirectKHR-None-02697";
        linear_filter_sampler              = "VUID-vkCmdTraceRaysIndirectKHR-magFilter-04553";
        linear_mipmap_sampler              = "VUID-vkCmdTraceRaysIndirectKHR-mipmapMode-04770";
        cubic_sampler                      = "VUID-vkCmdTraceRaysIndirectKHR-None-02692";
        indirect_contiguous_memory         = "VUID-vkCmdTraceRaysIndirectKHR-indirectDeviceAddress-03632";
        indirect_buffer_bit                = "VUID-vkCmdTraceRaysIndirectKHR-indirectDeviceAddress-03633";
        corner_sampled_address_mode        = "VUID-vkCmdTraceRaysIndirectKHR-flags-02696";
        imageview_atomic                   = "VUID-vkCmdTraceRaysIndirectKHR-None-02691";
        push_constants_set                 = "VUID-vkCmdTraceRaysIndirectKHR-maintenance4-06425";
        descriptor_valid                   = "VUID-vkCmdTraceRaysIndirectKHR-None-02699";
        sampler_imageview_type             = "VUID-vkCmdTraceRaysIndirectKHR-None-02702";
        sampler_implicitLod_dref_proj      = "VUID-vkCmdTraceRaysIndirectKHR-None-02703";
        sampler_bias_offset                = "VUID-vkCmdTraceRaysIndirectKHR-None-02704";
        dynamic_state_setting_commands     = "VUID-vkCmdTraceRaysIndirectKHR-None-02859";
        unprotected_command_buffer         = "VUID-vkCmdTraceRaysIndirectKHR-commandBuffer-02707";
        ray_query_protected_cb             = "VUID-vkCmdTraceRaysIndirectKHR-commandBuffer-03635";
        img_filter_cubic                   = "VUID-vkCmdTraceRaysIndirectKHR-None-02693";
        filter_cubic                       = "VUID-vkCmdTraceRaysIndirectKHR-filterCubic-02694";
        filter_cubic_min_max               = "VUID-vkCmdTraceRaysIndirectKHR-filterCubicMinmax-02695";
        image_view_access_64               = "VUID-vkCmdTraceRaysIndirectKHR-SampledType-04470";
        image_view_access_32               = "VUID-vkCmdTraceRaysIndirectKHR-SampledType-04471";
        buffer_view_access_64              = "VUID-vkCmdTraceRaysIndirectKHR-SampledType-04472";
        buffer_view_access_32              = "VUID-vkCmdTraceRaysIndirectKHR-SampledType-04473";
        storage_image_read_without_format  = "VUID-vkCmdTraceRaysIndirectKHR-OpTypeImage-07028";
        storage_image_write_without_format = "VUID-vkCmdTraceRaysIndirectKHR-OpTypeImage-07027";
        storage_texel_buffer_read_without_format  = "VUID-vkCmdTraceRaysIndirectKHR-OpTypeImage-07030";
        storage_texel_buffer_write_without_format = "VUID-vkCmdTraceRaysIndirectKHR-OpTypeImage-07029";
        storage_image_write_texel_count           = "VUID-vkCmdTraceRaysIndirectKHR-None-04115";
        storage_texel_buffer_write_texel_count    = "VUID-vkCmdTraceRaysIndirectKHR-OpImageWrite-04469";
        depth_compare_sample               = "VUID-vkCmdTraceRaysIndirectKHR-None-06479";
        descriptor_buffer_bit_set          = "VUID-vkCmdTraceRaysIndirectKHR-None-08114";
        descriptor_buffer_bit_not_set      = "VUID-vkCmdTraceRaysIndirectKHR-None-08115";
        descriptor_buffer_set_offset_missing = "VUID-vkCmdTraceRaysIndirectKHR-None-08117";
        image_view_dim                     = "VUID-vkCmdTraceRaysIndirectKHR-viewType-07752";
        image_view_numeric_format          = "VUID-vkCmdTraceRaysIndirectKHR-format-07753";
    }
};

struct DispatchVuidsCmdTraceRaysIndirect2KHR: DrawDispatchVuid {
    DispatchVuidsCmdTraceRaysIndirect2KHR() : DrawDispatchVuid() {
        pipeline_bound                     = "VUID-vkCmdTraceRaysIndirect2KHR-None-02700";
        compatible_pipeline                = "VUID-vkCmdTraceRaysIndirect2KHR-None-02697";
        linear_filter_sampler              = "VUID-vkCmdTraceRaysIndirect2KHR-magFilter-04553";
        linear_mipmap_sampler              = "VUID-vkCmdTraceRaysIndirect2KHR-mipmapMode-04770";
        cubic_sampler                      = "VUID-vkCmdTraceRaysIndirect2KHR-None-02692";
        indirect_contiguous_memory         = "VUID-vkCmdTraceRaysIndirect2KHR-indirectDeviceAddress-03632";
        indirect_buffer_bit                = "VUID-vkCmdTraceRaysIndirect2KHR-indirectDeviceAddress-03633";
        corner_sampled_address_mode        = "VUID-vkCmdTraceRaysIndirect2KHR-flags-02696";
        imageview_atomic                   = "VUID-vkCmdTraceRaysIndirect2KHR-None-02691";
        push_constants_set                 = "VUID-vkCmdTraceRaysIndirect2KHR-maintenance4-06425";
        descriptor_valid                   = "VUID-vkCmdTraceRaysIndirect2KHR-None-02699";
        sampler_imageview_type             = "VUID-vkCmdTraceRaysIndirect2KHR-None-02702";
        sampler_implicitLod_dref_proj      = "VUID-vkCmdTraceRaysIndirect2KHR-None-02703";
        sampler_bias_offset                = "VUID-vkCmdTraceRaysIndirect2KHR-None-02704";
        dynamic_state_setting_commands     = "VUID-vkCmdTraceRaysIndirect2KHR-None-02859";
        unprotected_command_buffer         = "VUID-vkCmdTraceRaysIndirect2KHR-commandBuffer-02707";
        ray_query_protected_cb             = "VUID-vkCmdTraceRaysIndirect2KHR-commandBuffer-03635";
        img_filter_cubic                   = "VUID-vkCmdTraceRaysIndirect2KHR-None-02693";
        filter_cubic                       = "VUID-vkCmdTraceRaysIndirect2KHR-filterCubic-02694";
        filter_cubic_min_max               = "VUID-vkCmdTraceRaysIndirect2KHR-filterCubicMinmax-02695";
        image_view_access_64               = "VUID-vkCmdTraceRaysIndirect2KHR-SampledType-04470";
        image_view_access_32               = "VUID-vkCmdTraceRaysIndirect2KHR-SampledType-04471";
        buffer_view_access_64              = "VUID-vkCmdTraceRaysIndirect2KHR-SampledType-04472";
        buffer_view_access_32              = "VUID-vkCmdTraceRaysIndirect2KHR-SampledType-04473";
        storage_image_read_without_format  = "VUID-vkCmdTraceRaysIndirect2KHR-OpTypeImage-07028";
        storage_image_write_without_format = "VUID-vkCmdTraceRaysIndirect2KHR-OpTypeImage-07027";
        storage_texel_buffer_read_without_format  = "VUID-vkCmdTraceRaysIndirect2KHR-OpTypeImage-07030";
        storage_texel_buffer_write_without_format = "VUID-vkCmdTraceRaysIndirect2KHR-OpTypeImage-07029";
        storage_image_write_texel_count           = "VUID-vkCmdTraceRaysIndirect2KHR-None-04115";
        storage_texel_buffer_write_texel_count    = "VUID-vkCmdTraceRaysIndirect2KHR-OpImageWrite-04469";
        depth_compare_sample               = "VUID-vkCmdTraceRaysIndirect2KHR-None-06479";
        descriptor_buffer_bit_set          = "VUID-vkCmdTraceRaysIndirect2KHR-None-08114";
        descriptor_buffer_bit_not_set      = "VUID-vkCmdTraceRaysIndirect2KHR-None-08115";
        descriptor_buffer_set_offset_missing = "VUID-vkCmdTraceRaysIndirect2KHR-None-08117";
        image_view_dim                     = "VUID-vkCmdTraceRaysIndirect2KHR-viewType-07752";
        image_view_numeric_format          = "VUID-vkCmdTraceRaysIndirect2KHR-format-07753";
    }
};

struct DispatchVuidsCmdDrawMeshTasksNV: DrawDispatchVuid {
    DispatchVuidsCmdDrawMeshTasksNV() : DrawDispatchVuid() {
        pipeline_bound                     = "VUID-vkCmdDrawMeshTasksNV-None-02700";
        compatible_pipeline                = "VUID-vkCmdDrawMeshTasksNV-None-02697";
        render_pass_compatible             = "VUID-vkCmdDrawMeshTasksNV-renderPass-02684";
        subpass_index                      = "VUID-vkCmdDrawMeshTasksNV-subpass-02685";
        sample_location                    = "VUID-vkCmdDrawMeshTasksNV-sampleLocationsEnable-02689";
        linear_filter_sampler              = "VUID-vkCmdDrawMeshTasksNV-magFilter-04553";
        linear_mipmap_sampler              = "VUID-vkCmdDrawMeshTasksNV-mipmapMode-04770";
        cubic_sampler                      = "VUID-vkCmdDrawMeshTasksNV-None-02692";
        viewport_count                     = "VUID-vkCmdDrawMeshTasksNV-viewportCount-03417";
        scissor_count                      = "VUID-vkCmdDrawMeshTasksNV-scissorCount-03418";
        viewport_scissor_count             = "VUID-vkCmdDrawMeshTasksNV-viewportCount-03419";
        corner_sampled_address_mode        = "VUID-vkCmdDrawMeshTasksNV-flags-02696";
        imageview_atomic                   = "VUID-vkCmdDrawMeshTasksNV-None-02691";
        push_constants_set                 = "VUID-vkCmdDrawMeshTasksNV-maintenance4-06425";
        image_subresources_render_pass_write = "VUID-vkCmdDrawMeshTasksNV-None-06537";
        image_subresources_subpass_read    = "VUID-vkCmdDrawMeshTasksNV-None-06538";
        image_subresources_subpass_write   = "VUID-vkCmdDrawMeshTasksNV-None-06539";
        descriptor_valid                   = "VUID-vkCmdDrawMeshTasksNV-None-02699";
        sampler_imageview_type             = "VUID-vkCmdDrawMeshTasksNV-None-02702";
        sampler_implicitLod_dref_proj      = "VUID-vkCmdDrawMeshTasksNV-None-02703";
        sampler_bias_offset                = "VUID-vkCmdDrawMeshTasksNV-None-02704";
        dynamic_state_setting_commands     = "VUID-vkCmdDrawMeshTasksNV-None-02859";
        rasterization_samples              = "VUID-vkCmdDrawMeshTasksNV-rasterizationSamples-04740";
        msrtss_rasterization_samples       = "VUID-vkCmdDrawMeshTasksNV-multisampledRenderToSingleSampled-07284";
        unprotected_command_buffer         = "VUID-vkCmdDrawMeshTasksNV-commandBuffer-02707";
        max_multiview_instance_index       = "VUID-vkCmdDrawMeshTasksNV-maxMultiviewInstanceIndex-02688";
        img_filter_cubic                   = "VUID-vkCmdDrawMeshTasksNV-None-02693";
        filter_cubic                       = "VUID-vkCmdDrawMeshTasksNV-filterCubic-02694";
        filter_cubic_min_max               = "VUID-vkCmdDrawMeshTasksNV-filterCubicMinmax-02695";
        viewport_count_primitive_shading_rate = "VUID-vkCmdDrawMeshTasksNV-primitiveFragmentShadingRateWithMultipleViewports-04552";
        rasterizer_discard_enable          = "VUID-vkCmdDrawMeshTasksNV-None-04876";
        depth_bias_enable                  = "VUID-vkCmdDrawMeshTasksNV-None-04877";
        logic_op                           = "VUID-vkCmdDrawMeshTasksNV-logicOp-04878";
        blend_enable                       = "VUID-vkCmdDrawMeshTasksNV-blendEnable-04727";
        dynamic_discard_rectangle          = "VUID-vkCmdDrawMeshTasksNV-None-07751";
        dynamic_color_write_enable         = "VUID-vkCmdDrawMeshTasksNV-None-07749";
        dynamic_color_write_enable_count   = "VUID-vkCmdDrawMeshTasksNV-attachmentCount-07750";
        dynamic_rendering_view_mask        = "VUID-vkCmdDrawMeshTasksNV-viewMask-06178";
        dynamic_rendering_color_count      = "VUID-vkCmdDrawMeshTasksNV-colorAttachmentCount-06179";
        dynamic_rendering_color_formats    = "VUID-vkCmdDrawMeshTasksNV-colorAttachmentCount-06180";
        dynamic_rendering_depth_format     = "VUID-vkCmdDrawMeshTasksNV-pDepthAttachment-06181";
        dynamic_rendering_stencil_format   = "VUID-vkCmdDrawMeshTasksNV-pStencilAttachment-06182";
        dynamic_rendering_fsr              = "VUID-vkCmdDrawMeshTasksNV-imageView-06183";
        dynamic_rendering_fdm              = "VUID-vkCmdDrawMeshTasksNV-imageView-06184";
        dynamic_rendering_color_sample     = "VUID-vkCmdDrawMeshTasksNV-colorAttachmentCount-06185";
        dynamic_rendering_depth_sample     = "VUID-vkCmdDrawMeshTasksNV-pDepthAttachment-06186";
        dynamic_rendering_stencil_sample   = "VUID-vkCmdDrawMeshTasksNV-pStencilAttachment-06187";
        dynamic_rendering_multi_sample     = "VUID-vkCmdDrawMeshTasksNV-colorAttachmentCount-06188";
        dynamic_rendering_06189            = "VUID-vkCmdDrawMeshTasksNV-pDepthAttachment-06189";
        dynamic_rendering_06190            = "VUID-vkCmdDrawMeshTasksNV-pStencilAttachment-06190";
        dynamic_rendering_06198            = "VUID-vkCmdDrawMeshTasksNV-renderPass-06198";
        dynamic_rendering_07285            = "VUID-vkCmdDrawMeshTasksNV-multisampledRenderToSingleSampled-07285";
        dynamic_rendering_07286            = "VUID-vkCmdDrawMeshTasksNV-multisampledRenderToSingleSampled-07286";
        dynamic_rendering_07287            = "VUID-vkCmdDrawMeshTasksNV-multisampledRenderToSingleSampled-07287";
        image_view_access_64               = "VUID-vkCmdDrawMeshTasksNV-SampledType-04470";
        image_view_access_32               = "VUID-vkCmdDrawMeshTasksNV-SampledType-04471";
        buffer_view_access_64              = "VUID-vkCmdDrawMeshTasksNV-SampledType-04472";
        buffer_view_access_32              = "VUID-vkCmdDrawMeshTasksNV-SampledType-04473";
        storage_image_read_without_format  = "VUID-vkCmdDrawMeshTasksNV-OpTypeImage-07028";
        storage_image_write_without_format = "VUID-vkCmdDrawMeshTasksNV-OpTypeImage-07027";
        storage_texel_buffer_read_without_format  = "VUID-vkCmdDrawMeshTasksNV-OpTypeImage-07030";
        storage_texel_buffer_write_without_format = "VUID-vkCmdDrawMeshTasksNV-OpTypeImage-07029";
        storage_image_write_texel_count           = "VUID-vkCmdDrawMeshTasksNV-None-04115";
        storage_texel_buffer_write_texel_count    = "VUID-vkCmdDrawMeshTasksNV-OpImageWrite-04469";
        depth_compare_sample               = "VUID-vkCmdDrawMeshTasksNV-None-06479";
        depth_read_only                    = "VUID-vkCmdDrawMeshTasksNV-None-06886";
        stencil_read_only                  = "VUID-vkCmdDrawMeshTasksNV-None-06887";
        dynamic_sample_locations           = "VUID-vkCmdDrawMeshTasksNV-None-06666";
        dynamic_tessellation_domain_origin = "VUID-vkCmdDrawMeshTasksNV-None-07619";
        dynamic_depth_clamp_enable         = "VUID-vkCmdDrawMeshTasksNV-None-07620";
        dynamic_polygon_mode               = "VUID-vkCmdDrawMeshTasksNV-None-07621";
        dynamic_rasterization_samples      = "VUID-vkCmdDrawMeshTasksNV-None-07622";
        dynamic_sample_mask                = "VUID-vkCmdDrawMeshTasksNV-None-07623";
        dynamic_alpha_to_coverage_enable   = "VUID-vkCmdDrawMeshTasksNV-None-07624";
        dynamic_alpha_to_one_enable        = "VUID-vkCmdDrawMeshTasksNV-None-07625";
        dynamic_logic_op_enable            = "VUID-vkCmdDrawMeshTasksNV-None-07626";
        dynamic_color_blend_enable         = "VUID-vkCmdDrawMeshTasksNV-firstAttachment-07476";
        dynamic_color_blend_equation       = "VUID-vkCmdDrawMeshTasksNV-firstAttachment-07477";
        dynamic_color_write_mask           = "VUID-vkCmdDrawMeshTasksNV-firstAttachment-07478";
        dynamic_rasterization_stream       = "VUID-vkCmdDrawMeshTasksNV-None-07630";
        dynamic_conservative_rasterization_mode = "VUID-vkCmdDrawMeshTasksNV-None-07631";
        dynamic_extra_primitive_overestimation_size = "VUID-vkCmdDrawMeshTasksNV-None-07632";
        dynamic_depth_clip_enable          = "VUID-vkCmdDrawMeshTasksNV-None-07633";
        dynamic_sample_locations_enable    = "VUID-vkCmdDrawMeshTasksNV-None-07634";
        dynamic_color_blend_advanced       = "VUID-vkCmdDrawMeshTasksNV-firstAttachment-07479";
        dynamic_provoking_vertex_mode      = "VUID-vkCmdDrawMeshTasksNV-None-07636";
        dynamic_line_rasterization_mode    = "VUID-vkCmdDrawMeshTasksNV-None-07637";
        dynamic_line_stipple_enable        = "VUID-vkCmdDrawMeshTasksNV-None-07638";
        dynamic_depth_clip_negative_one_to_one = "VUID-vkCmdDrawMeshTasksNV-None-07639";
        dynamic_viewport_w_scaling_enable  = "VUID-vkCmdDrawMeshTasksNV-None-07640";
        dynamic_viewport_swizzle           = "VUID-vkCmdDrawMeshTasksNV-None-07641";
        dynamic_coverage_to_color_enable   = "VUID-vkCmdDrawMeshTasksNV-None-07642";
        dynamic_coverage_to_color_location = "VUID-vkCmdDrawMeshTasksNV-None-07643";
        dynamic_coverage_modulation_mode   = "VUID-vkCmdDrawMeshTasksNV-None-07644";
        dynamic_coverage_modulation_table_enable = "VUID-vkCmdDrawMeshTasksNV-None-07645";
        dynamic_coverage_modulation_table  = "VUID-vkCmdDrawMeshTasksNV-None-07646";
        dynamic_coverage_reduction_mode    = "VUID-vkCmdDrawMeshTasksNV-None-07647";
        dynamic_representative_fragment_test_enable  = "VUID-vkCmdDrawMeshTasksNV-None-07648";
        dynamic_shading_rate_image_enable  = "VUID-vkCmdDrawMeshTasksNV-None-07649";
        dynamic_viewport                   = "VUID-vkCmdDrawMeshTasksNV-None-07831";
        dynamic_scissor                    = "VUID-vkCmdDrawMeshTasksNV-None-07832";
        dynamic_depth_bias                 = "VUID-vkCmdDrawMeshTasksNV-None-07834";
        dynamic_line_width                 = "VUID-vkCmdDrawMeshTasksNV-None-07833";
        dynamic_line_stipple_ext           = "VUID-vkCmdDrawMeshTasksNV-None-07849";
        dynamic_blend_constants            = "VUID-vkCmdDrawMeshTasksNV-None-07835";
        dynamic_depth_bounds               = "VUID-vkCmdDrawMeshTasksNV-None-07836";
        dynamic_stencil_compare_mask       = "VUID-vkCmdDrawMeshTasksNV-None-07837";
        dynamic_stencil_write_mask         = "VUID-vkCmdDrawMeshTasksNV-None-07838";
        dynamic_stencil_reference          = "VUID-vkCmdDrawMeshTasksNV-None-07839";
        dynamic_state_inherited            = "VUID-vkCmdDrawMeshTasksNV-None-07850";
        primitives_generated               = "VUID-vkCmdDrawMeshTasksNV-primitivesGeneratedQueryWithRasterizerDiscard-06708";
        primitives_generated_streams       = "VUID-vkCmdDrawMeshTasksNV-primitivesGeneratedQueryWithNonZeroStreams-06709";
        mesh_shader_stages                 = "VUID-vkCmdDrawMeshTasksNV-stage-06480";
        missing_mesh_shader_stages         = "VUID-vkCmdDrawMeshTasksNV-MeshNV-07080";
        descriptor_buffer_bit_set          = "VUID-vkCmdDrawMeshTasksNV-None-08114";
        descriptor_buffer_bit_not_set      = "VUID-vkCmdDrawMeshTasksNV-None-08115";
        descriptor_buffer_set_offset_missing = "VUID-vkCmdDrawMeshTasksNV-None-08117";
        image_view_dim                     = "VUID-vkCmdDrawMeshTasksNV-viewType-07752";
        image_view_numeric_format          = "VUID-vkCmdDrawMeshTasksNV-format-07753";
        stippled_rectangular_lines         = "VUID-vkCmdDrawMeshTasksNV-stippledLineEnable-07495";
        stippled_bresenham_lines           = "VUID-vkCmdDrawMeshTasksNV-stippledLineEnable-07496";
        stippled_smooth_lines              = "VUID-vkCmdDrawMeshTasksNV-stippledLineEnable-07497";
        stippled_default_strict            = "VUID-vkCmdDrawMeshTasksNV-stippledLineEnable-07498";
    }
};

struct DispatchVuidsCmdDrawMeshTasksIndirectNV: DrawDispatchVuid {
    DispatchVuidsCmdDrawMeshTasksIndirectNV() : DrawDispatchVuid() {
        pipeline_bound                     = "VUID-vkCmdDrawMeshTasksIndirectNV-None-02700";
        compatible_pipeline                = "VUID-vkCmdDrawMeshTasksIndirectNV-None-02697";
        render_pass_compatible             = "VUID-vkCmdDrawMeshTasksIndirectNV-renderPass-02684";
        subpass_index                      = "VUID-vkCmdDrawMeshTasksIndirectNV-subpass-02685";
        sample_location                    = "VUID-vkCmdDrawMeshTasksIndirectNV-sampleLocationsEnable-02689";
        linear_filter_sampler              = "VUID-vkCmdDrawMeshTasksIndirectNV-magFilter-04553";
        linear_mipmap_sampler              = "VUID-vkCmdDrawMeshTasksIndirectNV-mipmapMode-04770";
        cubic_sampler                      = "VUID-vkCmdDrawMeshTasksIndirectNV-None-02692";
        indirect_protected_cb              = "VUID-vkCmdDrawMeshTasksIndirectNV-commandBuffer-02711";
        indirect_contiguous_memory         = "VUID-vkCmdDrawMeshTasksIndirectNV-buffer-02708";
        indirect_buffer_bit                = "VUID-vkCmdDrawMeshTasksIndirectNV-buffer-02709";
        viewport_count                     = "VUID-vkCmdDrawMeshTasksIndirectNV-viewportCount-03417";
        scissor_count                      = "VUID-vkCmdDrawMeshTasksIndirectNV-scissorCount-03418";
        viewport_scissor_count             = "VUID-vkCmdDrawMeshTasksIndirectNV-viewportCount-03419";
        corner_sampled_address_mode        = "VUID-vkCmdDrawMeshTasksIndirectNV-flags-02696";
        imageview_atomic                   = "VUID-vkCmdDrawMeshTasksIndirectNV-None-02691";
        push_constants_set                 = "VUID-vkCmdDrawMeshTasksIndirectNV-maintenance4-06425";
        image_subresources_render_pass_write = "VUID-vkCmdDrawMeshTasksIndirectNV-None-06537";
        image_subresources_subpass_read    = "VUID-vkCmdDrawMeshTasksIndirectNV-None-06538";
        image_subresources_subpass_write   = "VUID-vkCmdDrawMeshTasksIndirectNV-None-06539";
        descriptor_valid                   = "VUID-vkCmdDrawMeshTasksIndirectNV-None-02699";
        sampler_imageview_type             = "VUID-vkCmdDrawMeshTasksIndirectNV-None-02702";
        sampler_implicitLod_dref_proj      = "VUID-vkCmdDrawMeshTasksIndirectNV-None-02703";
        sampler_bias_offset                = "VUID-vkCmdDrawMeshTasksIndirectNV-None-02704";
        dynamic_state_setting_commands     = "VUID-vkCmdDrawMeshTasksIndirectNV-None-02859";
        rasterization_samples              = "VUID-vkCmdDrawMeshTasksIndirectNV-rasterizationSamples-04740";
        msrtss_rasterization_samples       = "VUID-vkCmdDrawMeshTasksIndirectNV-multisampledRenderToSingleSampled-07284";
        unprotected_command_buffer         = "VUID-vkCmdDrawMeshTasksIndirectNV-commandBuffer-02707";
        max_multiview_instance_index       = "VUID-vkCmdDrawMeshTasksIndirectNV-maxMultiviewInstanceIndex-02688";
        img_filter_cubic                   = "VUID-vkCmdDrawMeshTasksIndirectNV-None-02693";
        filter_cubic                       = "VUID-vkCmdDrawMeshTasksIndirectNV-filterCubic-02694";
        filter_cubic_min_max               = "VUID-vkCmdDrawMeshTasksIndirectNV-filterCubicMinmax-02695";
        viewport_count_primitive_shading_rate = "VUID-vkCmdDrawMeshTasksIndirectNV-primitiveFragmentShadingRateWithMultipleViewports-04552";
        rasterizer_discard_enable          = "VUID-vkCmdDrawMeshTasksIndirectNV-None-04876";
        depth_bias_enable                  = "VUID-vkCmdDrawMeshTasksIndirectNV-None-04877";
        logic_op                           = "VUID-vkCmdDrawMeshTasksIndirectNV-logicOp-04878";
        blend_enable                       = "VUID-vkCmdDrawMeshTasksIndirectNV-blendEnable-04727";
        dynamic_discard_rectangle          = "VUID-vkCmdDrawMeshTasksIndirectNV-None-07751";
        dynamic_color_write_enable         = "VUID-vkCmdDrawMeshTasksIndirectNV-None-07749";
        dynamic_color_write_enable_count   = "VUID-vkCmdDrawMeshTasksIndirectNV-attachmentCount-07750";
        dynamic_rendering_view_mask        = "VUID-vkCmdDrawMeshTasksIndirectNV-viewMask-06178";
        dynamic_rendering_color_count      = "VUID-vkCmdDrawMeshTasksIndirectNV-colorAttachmentCount-06179";
        dynamic_rendering_color_formats    = "VUID-vkCmdDrawMeshTasksIndirectNV-colorAttachmentCount-06180";
        dynamic_rendering_depth_format     = "VUID-vkCmdDrawMeshTasksIndirectNV-pDepthAttachment-06181";
        dynamic_rendering_stencil_format   = "VUID-vkCmdDrawMeshTasksIndirectNV-pStencilAttachment-06182";
        dynamic_rendering_fsr              = "VUID-vkCmdDrawMeshTasksIndirectNV-imageView-06183";
        dynamic_rendering_fdm              = "VUID-vkCmdDrawMeshTasksIndirectNV-imageView-06184";
        dynamic_rendering_color_sample     = "VUID-vkCmdDrawMeshTasksIndirectNV-colorAttachmentCount-06185";
        dynamic_rendering_depth_sample     = "VUID-vkCmdDrawMeshTasksIndirectNV-pDepthAttachment-06186";
        dynamic_rendering_stencil_sample   = "VUID-vkCmdDrawMeshTasksIndirectNV-pStencilAttachment-06187";
        dynamic_rendering_multi_sample     = "VUID-vkCmdDrawMeshTasksIndirectNV-colorAttachmentCount-06188";
        dynamic_rendering_06189            = "VUID-vkCmdDrawMeshTasksIndirectNV-pDepthAttachment-06189";
        dynamic_rendering_06190            = "VUID-vkCmdDrawMeshTasksIndirectNV-pStencilAttachment-06190";
        dynamic_rendering_06198            = "VUID-vkCmdDrawMeshTasksIndirectNV-renderPass-06198";
        dynamic_rendering_07285            = "VUID-vkCmdDrawMeshTasksIndirectNV-multisampledRenderToSingleSampled-07285";
        dynamic_rendering_07286            = "VUID-vkCmdDrawMeshTasksIndirectNV-multisampledRenderToSingleSampled-07286";
        dynamic_rendering_07287            = "VUID-vkCmdDrawMeshTasksIndirectNV-multisampledRenderToSingleSampled-07287";
        image_view_access_64               = "VUID-vkCmdDrawMeshTasksIndirectNV-SampledType-04470";
        image_view_access_32               = "VUID-vkCmdDrawMeshTasksIndirectNV-SampledType-04471";
        buffer_view_access_64              = "VUID-vkCmdDrawMeshTasksIndirectNV-SampledType-04472";
        buffer_view_access_32              = "VUID-vkCmdDrawMeshTasksIndirectNV-SampledType-04473";
        storage_image_read_without_format  = "VUID-vkCmdDrawMeshTasksIndirectNV-OpTypeImage-07028";
        storage_image_write_without_format = "VUID-vkCmdDrawMeshTasksIndirectNV-OpTypeImage-07027";
        storage_texel_buffer_read_without_format  = "VUID-vkCmdDrawMeshTasksIndirectNV-OpTypeImage-07030";
        storage_texel_buffer_write_without_format = "VUID-vkCmdDrawMeshTasksIndirectNV-OpTypeImage-07029";
        storage_image_write_texel_count           = "VUID-vkCmdDrawMeshTasksIndirectNV-None-04115";
        storage_texel_buffer_write_texel_count    = "VUID-vkCmdDrawMeshTasksIndirectNV-OpImageWrite-04469";
        depth_compare_sample               = "VUID-vkCmdDrawMeshTasksIndirectNV-None-06479";
        depth_read_only                    = "VUID-vkCmdDrawMeshTasksIndirectNV-None-06886";
        stencil_read_only                  = "VUID-vkCmdDrawMeshTasksIndirectNV-None-06887";
        dynamic_sample_locations           = "VUID-vkCmdDrawMeshTasksIndirectNV-None-06666";
        dynamic_tessellation_domain_origin = "VUID-vkCmdDrawMeshTasksIndirectNV-None-07619";
        dynamic_depth_clamp_enable         = "VUID-vkCmdDrawMeshTasksIndirectNV-None-07620";
        dynamic_polygon_mode               = "VUID-vkCmdDrawMeshTasksIndirectNV-None-07621";
        dynamic_rasterization_samples      = "VUID-vkCmdDrawMeshTasksIndirectNV-None-07622";
        dynamic_sample_mask                = "VUID-vkCmdDrawMeshTasksIndirectNV-None-07623";
        dynamic_alpha_to_coverage_enable   = "VUID-vkCmdDrawMeshTasksIndirectNV-None-07624";
        dynamic_alpha_to_one_enable        = "VUID-vkCmdDrawMeshTasksIndirectNV-None-07625";
        dynamic_logic_op_enable            = "VUID-vkCmdDrawMeshTasksIndirectNV-None-07626";
        dynamic_color_blend_enable         = "VUID-vkCmdDrawMeshTasksIndirectNV-firstAttachment-07476";
        dynamic_color_blend_equation       = "VUID-vkCmdDrawMeshTasksIndirectNV-firstAttachment-07477";
        dynamic_color_write_mask           = "VUID-vkCmdDrawMeshTasksIndirectNV-firstAttachment-07478";
        dynamic_rasterization_stream       = "VUID-vkCmdDrawMeshTasksIndirectNV-None-07630";
        dynamic_conservative_rasterization_mode = "VUID-vkCmdDrawMeshTasksIndirectNV-None-07631";
        dynamic_extra_primitive_overestimation_size = "VUID-vkCmdDrawMeshTasksIndirectNV-None-07632";
        dynamic_depth_clip_enable          = "VUID-vkCmdDrawMeshTasksIndirectNV-None-07633";
        dynamic_sample_locations_enable    = "VUID-vkCmdDrawMeshTasksIndirectNV-None-07634";
        dynamic_color_blend_advanced       = "VUID-vkCmdDrawMeshTasksIndirectNV-firstAttachment-07479";
        dynamic_provoking_vertex_mode      = "VUID-vkCmdDrawMeshTasksIndirectNV-None-07636";
        dynamic_line_rasterization_mode    = "VUID-vkCmdDrawMeshTasksIndirectNV-None-07637";
        dynamic_line_stipple_enable        = "VUID-vkCmdDrawMeshTasksIndirectNV-None-07638";
        dynamic_depth_clip_negative_one_to_one = "VUID-vkCmdDrawMeshTasksIndirectNV-None-07639";
        dynamic_viewport_w_scaling_enable  = "VUID-vkCmdDrawMeshTasksIndirectNV-None-07640";
        dynamic_viewport_swizzle           = "VUID-vkCmdDrawMeshTasksIndirectNV-None-07641";
        dynamic_coverage_to_color_enable   = "VUID-vkCmdDrawMeshTasksIndirectNV-None-07642";
        dynamic_coverage_to_color_location = "VUID-vkCmdDrawMeshTasksIndirectNV-None-07643";
        dynamic_coverage_modulation_mode   = "VUID-vkCmdDrawMeshTasksIndirectNV-None-07644";
        dynamic_coverage_modulation_table_enable = "VUID-vkCmdDrawMeshTasksIndirectNV-None-07645";
        dynamic_coverage_modulation_table  = "VUID-vkCmdDrawMeshTasksIndirectNV-None-07646";
        dynamic_coverage_reduction_mode    = "VUID-vkCmdDrawMeshTasksIndirectNV-None-07647";
        dynamic_representative_fragment_test_enable  = "VUID-vkCmdDrawMeshTasksIndirectNV-None-07648";
        dynamic_shading_rate_image_enable  = "VUID-vkCmdDrawMeshTasksIndirectNV-None-07649";
        dynamic_viewport                   = "VUID-vkCmdDrawMeshTasksIndirectNV-None-07831";
        dynamic_scissor                    = "VUID-vkCmdDrawMeshTasksIndirectNV-None-07832";
        dynamic_depth_bias                 = "VUID-vkCmdDrawMeshTasksIndirectNV-None-07834";
        dynamic_line_width                 = "VUID-vkCmdDrawMeshTasksIndirectNV-None-07833";
        dynamic_line_stipple_ext           = "VUID-vkCmdDrawMeshTasksIndirectNV-None-07849";
        dynamic_blend_constants            = "VUID-vkCmdDrawMeshTasksIndirectNV-None-07835";
        dynamic_depth_bounds               = "VUID-vkCmdDrawMeshTasksIndirectNV-None-07836";
        dynamic_stencil_compare_mask       = "VUID-vkCmdDrawMeshTasksIndirectNV-None-07837";
        dynamic_stencil_write_mask         = "VUID-vkCmdDrawMeshTasksIndirectNV-None-07838";
        dynamic_stencil_reference          = "VUID-vkCmdDrawMeshTasksIndirectNV-None-07839";
        dynamic_state_inherited            = "VUID-vkCmdDrawMeshTasksIndirectNV-None-07850";
        primitives_generated               = "VUID-vkCmdDrawMeshTasksIndirectNV-primitivesGeneratedQueryWithRasterizerDiscard-06708";
        primitives_generated_streams       = "VUID-vkCmdDrawMeshTasksIndirectNV-primitivesGeneratedQueryWithNonZeroStreams-06709";
        mesh_shader_stages                 = "VUID-vkCmdDrawMeshTasksIndirectNV-stage-06480";
        missing_mesh_shader_stages         = "VUID-vkCmdDrawMeshTasksIndirectNV-MeshNV-07081";
        descriptor_buffer_bit_set          = "VUID-vkCmdDrawMeshTasksIndirectNV-None-08114";
        descriptor_buffer_bit_not_set      = "VUID-vkCmdDrawMeshTasksIndirectNV-None-08115";
        descriptor_buffer_set_offset_missing = "VUID-vkCmdDrawMeshTasksIndirectNV-None-08117";
        image_view_dim                     = "VUID-vkCmdDrawMeshTasksIndirectNV-viewType-07752";
        image_view_numeric_format          = "VUID-vkCmdDrawMeshTasksIndirectNV-format-07753";
        stippled_rectangular_lines         = "VUID-vkCmdDrawMeshTasksIndirectNV-stippledLineEnable-07495";
        stippled_bresenham_lines           = "VUID-vkCmdDrawMeshTasksIndirectNV-stippledLineEnable-07496";
        stippled_smooth_lines              = "VUID-vkCmdDrawMeshTasksIndirectNV-stippledLineEnable-07497";
        stippled_default_strict            = "VUID-vkCmdDrawMeshTasksIndirectNV-stippledLineEnable-07498";
    }
};

struct DispatchVuidsCmdDrawMeshTasksIndirectCountNV : DrawDispatchVuid {
    DispatchVuidsCmdDrawMeshTasksIndirectCountNV() : DrawDispatchVuid() {
        pipeline_bound                     = "VUID-vkCmdDrawMeshTasksIndirectCountNV-None-02700";
        compatible_pipeline                = "VUID-vkCmdDrawMeshTasksIndirectCountNV-None-02697";
        render_pass_compatible             = "VUID-vkCmdDrawMeshTasksIndirectCountNV-renderPass-02684";
        subpass_index                      = "VUID-vkCmdDrawMeshTasksIndirectCountNV-subpass-02685";
        sample_location                    = "VUID-vkCmdDrawMeshTasksIndirectCountNV-sampleLocationsEnable-02689";
        linear_filter_sampler              = "VUID-vkCmdDrawMeshTasksIndirectCountNV-magFilter-04553";
        linear_mipmap_sampler              = "VUID-vkCmdDrawMeshTasksIndirectCountNV-mipmapMode-04770";
        cubic_sampler                      = "VUID-vkCmdDrawMeshTasksIndirectCountNV-None-02692";
        indirect_protected_cb              = "VUID-vkCmdDrawMeshTasksIndirectCountNV-commandBuffer-02711";
        indirect_contiguous_memory         = "VUID-vkCmdDrawMeshTasksIndirectCountNV-buffer-02708";
        indirect_buffer_bit                = "VUID-vkCmdDrawMeshTasksIndirectCountNV-buffer-02709";
        indirect_count_contiguous_memory   = "VUID-vkCmdDrawMeshTasksIndirectCountNV-countBuffer-02714";
        indirect_count_buffer_bit          = "VUID-vkCmdDrawMeshTasksIndirectCountNV-countBuffer-02715";
        indirect_count_offset              = "VUID-vkCmdDrawMeshTasksIndirectCountNV-countBufferOffset-04129";
        viewport_count                     = "VUID-vkCmdDrawMeshTasksIndirectCountNV-viewportCount-03417";
        scissor_count                      = "VUID-vkCmdDrawMeshTasksIndirectCountNV-scissorCount-03418";
        viewport_scissor_count             = "VUID-vkCmdDrawMeshTasksIndirectCountNV-viewportCount-03419";
        corner_sampled_address_mode        = "VUID-vkCmdDrawMeshTasksIndirectCountNV-flags-02696";
        imageview_atomic                   = "VUID-vkCmdDrawMeshTasksIndirectCountNV-None-02691";
        push_constants_set                 = "VUID-vkCmdDrawMeshTasksIndirectCountNV-maintenance4-06425";
        image_subresources_render_pass_write = "VUID-vkCmdDrawMeshTasksIndirectCountNV-None-06537";
        image_subresources_subpass_read    = "VUID-vkCmdDrawMeshTasksIndirectCountNV-None-06538";
        image_subresources_subpass_write   = "VUID-vkCmdDrawMeshTasksIndirectCountNV-None-06539";
        descriptor_valid                   = "VUID-vkCmdDrawMeshTasksIndirectCountNV-None-02699";
        sampler_imageview_type             = "VUID-vkCmdDrawMeshTasksIndirectCountNV-None-02702";
        sampler_implicitLod_dref_proj      = "VUID-vkCmdDrawMeshTasksIndirectCountNV-None-02703";
        sampler_bias_offset                = "VUID-vkCmdDrawMeshTasksIndirectCountNV-None-02704";
        dynamic_state_setting_commands     = "VUID-vkCmdDrawMeshTasksIndirectCountNV-None-02859";
        rasterization_samples              = "VUID-vkCmdDrawMeshTasksIndirectCountNV-rasterizationSamples-04740";
        msrtss_rasterization_samples       = "VUID-vkCmdDrawMeshTasksIndirectCountNV-multisampledRenderToSingleSampled-07284";
        unprotected_command_buffer         = "VUID-vkCmdDrawMeshTasksIndirectCountNV-commandBuffer-02707";
        max_multiview_instance_index       = "VUID-vkCmdDrawMeshTasksIndirectCountNV-maxMultiviewInstanceIndex-02688";
        img_filter_cubic                   = "VUID-vkCmdDrawMeshTasksIndirectCountNV-None-02693";
        filter_cubic                       = "VUID-vkCmdDrawMeshTasksIndirectCountNV-filterCubic-02694";
        filter_cubic_min_max               = "VUID-vkCmdDrawMeshTasksIndirectCountNV-filterCubicMinmax-02695";
        viewport_count_primitive_shading_rate = "VUID-vkCmdDrawMeshTasksIndirectCountNV-primitiveFragmentShadingRateWithMultipleViewports-04552";
        rasterizer_discard_enable          = "VUID-vkCmdDrawMeshTasksIndirectCountNV-None-04876";
        depth_bias_enable                  = "VUID-vkCmdDrawMeshTasksIndirectCountNV-None-04877";
        logic_op                           = "VUID-vkCmdDrawMeshTasksIndirectCountNV-logicOp-04878";
        blend_enable                       = "VUID-vkCmdDrawMeshTasksIndirectCountNV-blendEnable-04727";
        dynamic_discard_rectangle          = "VUID-vkCmdDrawMeshTasksIndirectCountNV-None-07751";
        dynamic_color_write_enable         = "VUID-vkCmdDrawMeshTasksIndirectCountNV-None-07749";
        dynamic_color_write_enable_count   = "VUID-vkCmdDrawMeshTasksIndirectCountNV-attachmentCount-07750";
        dynamic_rendering_view_mask        = "VUID-vkCmdDrawMeshTasksIndirectCountNV-viewMask-06178";
        dynamic_rendering_color_count      = "VUID-vkCmdDrawMeshTasksIndirectCountNV-colorAttachmentCount-06179";
        dynamic_rendering_color_formats    = "VUID-vkCmdDrawMeshTasksIndirectCountNV-colorAttachmentCount-06180";
        dynamic_rendering_depth_format     = "VUID-vkCmdDrawMeshTasksIndirectCountNV-pDepthAttachment-06181";
        dynamic_rendering_stencil_format   = "VUID-vkCmdDrawMeshTasksIndirectCountNV-pStencilAttachment-06182";
        dynamic_rendering_fsr              = "VUID-vkCmdDrawMeshTasksIndirectCountNV-imageView-06183";
        dynamic_rendering_fdm              = "VUID-vkCmdDrawMeshTasksIndirectCountNV-imageView-06184";
        dynamic_rendering_color_sample     = "VUID-vkCmdDrawMeshTasksIndirectCountNV-colorAttachmentCount-06185";
        dynamic_rendering_depth_sample     = "VUID-vkCmdDrawMeshTasksIndirectCountNV-pDepthAttachment-06186";
        dynamic_rendering_stencil_sample   = "VUID-vkCmdDrawMeshTasksIndirectCountNV-pStencilAttachment-06187";
        dynamic_rendering_multi_sample     = "VUID-vkCmdDrawMeshTasksIndirectCountNV-colorAttachmentCount-06188";
        dynamic_rendering_06189            = "VUID-vkCmdDrawMeshTasksIndirectCountNV-pDepthAttachment-06189";
        dynamic_rendering_06190            = "VUID-vkCmdDrawMeshTasksIndirectCountNV-pStencilAttachment-06190";
        dynamic_rendering_06198            = "VUID-vkCmdDrawMeshTasksIndirectCountNV-renderPass-06198";
        dynamic_rendering_07285            = "VUID-vkCmdDrawMeshTasksIndirectCountNV-multisampledRenderToSingleSampled-07285";
        dynamic_rendering_07286            = "VUID-vkCmdDrawMeshTasksIndirectCountNV-multisampledRenderToSingleSampled-07286";
        dynamic_rendering_07287            = "VUID-vkCmdDrawMeshTasksIndirectCountNV-multisampledRenderToSingleSampled-07287";
        image_view_access_64               = "VUID-vkCmdDrawMeshTasksIndirectCountNV-SampledType-04470";
        image_view_access_32               = "VUID-vkCmdDrawMeshTasksIndirectCountNV-SampledType-04471";
        buffer_view_access_64              = "VUID-vkCmdDrawMeshTasksIndirectCountNV-SampledType-04472";
        buffer_view_access_32              = "VUID-vkCmdDrawMeshTasksIndirectCountNV-SampledType-04473";
        storage_image_read_without_format  = "VUID-vkCmdDrawMeshTasksIndirectCountNV-OpTypeImage-07028";
        storage_image_write_without_format = "VUID-vkCmdDrawMeshTasksIndirectCountNV-OpTypeImage-07027";
        storage_texel_buffer_read_without_format  = "VUID-vkCmdDrawMeshTasksIndirectCountNV-OpTypeImage-07030";
        storage_texel_buffer_write_without_format = "VUID-vkCmdDrawMeshTasksIndirectCountNV-OpTypeImage-07029";
        storage_image_write_texel_count           = "VUID-vkCmdDrawMeshTasksIndirectCountNV-None-04115";
        storage_texel_buffer_write_texel_count    = "VUID-vkCmdDrawMeshTasksIndirectCountNV-OpImageWrite-04469";
        depth_compare_sample               = "VUID-vkCmdDrawMeshTasksIndirectCountNV-None-06479";
        depth_read_only                    = "VUID-vkCmdDrawMeshTasksIndirectCountNV-None-06886";
        stencil_read_only                  = "VUID-vkCmdDrawMeshTasksIndirectCountNV-None-06887";
        dynamic_sample_locations           = "VUID-vkCmdDrawMeshTasksIndirectCountNV-None-06666";
        dynamic_tessellation_domain_origin = "VUID-vkCmdDrawMeshTasksIndirectCountNV-None-07619";
        dynamic_depth_clamp_enable         = "VUID-vkCmdDrawMeshTasksIndirectCountNV-None-07620";
        dynamic_polygon_mode               = "VUID-vkCmdDrawMeshTasksIndirectCountNV-None-07621";
        dynamic_rasterization_samples      = "VUID-vkCmdDrawMeshTasksIndirectCountNV-None-07622";
        dynamic_sample_mask                = "VUID-vkCmdDrawMeshTasksIndirectCountNV-None-07623";
        dynamic_alpha_to_coverage_enable   = "VUID-vkCmdDrawMeshTasksIndirectCountNV-None-07624";
        dynamic_alpha_to_one_enable        = "VUID-vkCmdDrawMeshTasksIndirectCountNV-None-07625";
        dynamic_logic_op_enable            = "VUID-vkCmdDrawMeshTasksIndirectCountNV-None-07626";
        dynamic_color_blend_enable         = "VUID-vkCmdDrawMeshTasksIndirectCountNV-firstAttachment-07476";
        dynamic_color_blend_equation       = "VUID-vkCmdDrawMeshTasksIndirectCountNV-firstAttachment-07477";
        dynamic_color_write_mask           = "VUID-vkCmdDrawMeshTasksIndirectCountNV-firstAttachment-07478";
        dynamic_rasterization_stream       = "VUID-vkCmdDrawMeshTasksIndirectCountNV-None-07630";
        dynamic_conservative_rasterization_mode = "VUID-vkCmdDrawMeshTasksIndirectCountNV-None-07631";
        dynamic_extra_primitive_overestimation_size = "VUID-vkCmdDrawMeshTasksIndirectCountNV-None-07632";
        dynamic_depth_clip_enable          = "VUID-vkCmdDrawMeshTasksIndirectCountNV-None-07633";
        dynamic_sample_locations_enable    = "VUID-vkCmdDrawMeshTasksIndirectCountNV-None-07634";
        dynamic_color_blend_advanced       = "VUID-vkCmdDrawMeshTasksIndirectCountNV-firstAttachment-07479";
        dynamic_provoking_vertex_mode      = "VUID-vkCmdDrawMeshTasksIndirectCountNV-None-07636";
        dynamic_line_rasterization_mode    = "VUID-vkCmdDrawMeshTasksIndirectCountNV-None-07637";
        dynamic_line_stipple_enable        = "VUID-vkCmdDrawMeshTasksIndirectCountNV-None-07638";
        dynamic_depth_clip_negative_one_to_one = "VUID-vkCmdDrawMeshTasksIndirectCountNV-None-07639";
        dynamic_viewport_w_scaling_enable  = "VUID-vkCmdDrawMeshTasksIndirectCountNV-None-07640";
        dynamic_viewport_swizzle           = "VUID-vkCmdDrawMeshTasksIndirectCountNV-None-07641";
        dynamic_coverage_to_color_enable   = "VUID-vkCmdDrawMeshTasksIndirectCountNV-None-07642";
        dynamic_coverage_to_color_location = "VUID-vkCmdDrawMeshTasksIndirectCountNV-None-07643";
        dynamic_coverage_modulation_mode   = "VUID-vkCmdDrawMeshTasksIndirectCountNV-None-07644";
        dynamic_coverage_modulation_table_enable = "VUID-vkCmdDrawMeshTasksIndirectCountNV-None-07645";
        dynamic_coverage_modulation_table  = "VUID-vkCmdDrawMeshTasksIndirectCountNV-None-07646";
        dynamic_coverage_reduction_mode    = "VUID-vkCmdDrawMeshTasksIndirectCountNV-None-07647";
        dynamic_representative_fragment_test_enable  = "VUID-vkCmdDrawMeshTasksIndirectCountNV-None-07648";
        dynamic_shading_rate_image_enable  = "VUID-vkCmdDrawMeshTasksIndirectCountNV-None-07649";
        dynamic_viewport                   = "VUID-vkCmdDrawMeshTasksIndirectCountNV-None-07831";
        dynamic_scissor                    = "VUID-vkCmdDrawMeshTasksIndirectCountNV-None-07832";
        dynamic_depth_bias                 = "VUID-vkCmdDrawMeshTasksIndirectCountNV-None-07834";
        dynamic_line_width                 = "VUID-vkCmdDrawMeshTasksIndirectCountNV-None-07833";
        dynamic_line_stipple_ext           = "VUID-vkCmdDrawMeshTasksIndirectCountNV-None-07849";
        dynamic_blend_constants            = "VUID-vkCmdDrawMeshTasksIndirectCountNV-None-07835";
        dynamic_depth_bounds               = "VUID-vkCmdDrawMeshTasksIndirectCountNV-None-07836";
        dynamic_stencil_compare_mask       = "VUID-vkCmdDrawMeshTasksIndirectCountNV-None-07837";
        dynamic_stencil_write_mask         = "VUID-vkCmdDrawMeshTasksIndirectCountNV-None-07838";
        dynamic_stencil_reference          = "VUID-vkCmdDrawMeshTasksIndirectCountNV-None-07839";
        dynamic_state_inherited            = "VUID-vkCmdDrawMeshTasksIndirectCountNV-None-07850";
        primitives_generated               = "VUID-vkCmdDrawMeshTasksIndirectCountNV-primitivesGeneratedQueryWithRasterizerDiscard-06708";
        primitives_generated_streams       = "VUID-vkCmdDrawMeshTasksIndirectCountNV-primitivesGeneratedQueryWithNonZeroStreams-06709";
        mesh_shader_stages                 = "VUID-vkCmdDrawMeshTasksIndirectCountNV-stage-06480";
        missing_mesh_shader_stages         = "VUID-vkCmdDrawMeshTasksIndirectCountNV-MeshNV-07082";
        descriptor_buffer_bit_set          = "VUID-vkCmdDrawMeshTasksIndirectCountNV-None-08114";
        descriptor_buffer_bit_not_set      = "VUID-vkCmdDrawMeshTasksIndirectCountNV-None-08115";
        descriptor_buffer_set_offset_missing = "VUID-vkCmdDrawMeshTasksIndirectCountNV-None-08117";
        image_view_dim                     = "VUID-vkCmdDrawMeshTasksIndirectCountNV-viewType-07752";
        image_view_numeric_format          = "VUID-vkCmdDrawMeshTasksIndirectCountNV-format-07753";
        stippled_rectangular_lines         = "VUID-vkCmdDrawMeshTasksIndirectCountNV-stippledLineEnable-07495";
        stippled_bresenham_lines           = "VUID-vkCmdDrawMeshTasksIndirectCountNV-stippledLineEnable-07496";
        stippled_smooth_lines              = "VUID-vkCmdDrawMeshTasksIndirectCountNV-stippledLineEnable-07497";
        stippled_default_strict            = "VUID-vkCmdDrawMeshTasksIndirectCountNV-stippledLineEnable-07498";
    }
};

struct DispatchVuidsCmdDrawMeshTasksEXT: DrawDispatchVuid {
    DispatchVuidsCmdDrawMeshTasksEXT() : DrawDispatchVuid() {
        mesh_shader_stages                 = "VUID-vkCmdDrawMeshTasksEXT-stage-06480";
        missing_mesh_shader_stages         = "VUID-vkCmdDrawMeshTasksEXT-MeshEXT-07087";
        descriptor_buffer_bit_set          = "VUID-vkCmdDrawMeshTasksEXT-None-08114";
        descriptor_buffer_bit_not_set      = "VUID-vkCmdDrawMeshTasksEXT-None-08115";
        descriptor_buffer_set_offset_missing = "VUID-vkCmdDrawMeshTasksEXT-None-08117";
        image_view_dim                     = "VUID-vkCmdDrawMeshTasksEXT-viewType-07752";
        image_view_numeric_format          = "VUID-vkCmdDrawMeshTasksEXT-format-07753";
    }
};

struct DispatchVuidsCmdDrawMeshTasksIndirectEXT: DrawDispatchVuid {
    DispatchVuidsCmdDrawMeshTasksIndirectEXT() : DrawDispatchVuid() {
        mesh_shader_stages                 = "VUID-vkCmdDrawMeshTasksIndirectEXT-stage-06480";
        missing_mesh_shader_stages         = "VUID-vkCmdDrawMeshTasksIndirectEXT-MeshEXT-07091";
        descriptor_buffer_bit_set          = "VUID-vkCmdDrawMeshTasksIndirectEXT-None-08114";
        descriptor_buffer_bit_not_set      = "VUID-vkCmdDrawMeshTasksIndirectEXT-None-08115";
        descriptor_buffer_set_offset_missing = "VUID-vkCmdDrawMeshTasksIndirectEXT-None-08117";
        image_view_dim                     = "VUID-vkCmdDrawMeshTasksIndirectEXT-viewType-07752";
        image_view_numeric_format          = "VUID-vkCmdDrawMeshTasksIndirectEXT-format-07753";
    }
};

struct DispatchVuidsCmdDrawMeshTasksIndirectCountEXT : DrawDispatchVuid {
    DispatchVuidsCmdDrawMeshTasksIndirectCountEXT() : DrawDispatchVuid() {
        mesh_shader_stages                 = "VUID-vkCmdDrawMeshTasksIndirectCountEXT-stage-06480";
        missing_mesh_shader_stages         = "VUID-vkCmdDrawMeshTasksIndirectCountEXT-MeshEXT-07100";
        indirect_count_contiguous_memory   = "VUID-vkCmdDrawMeshTasksIndirectCountEXT-countBuffer-02714";
        indirect_count_buffer_bit          = "VUID-vkCmdDrawMeshTasksIndirectCountEXT-countBuffer-02715";
        descriptor_buffer_bit_set          = "VUID-vkCmdDrawMeshTasksIndirectCountEXT-None-08114";
        descriptor_buffer_bit_not_set      = "VUID-vkCmdDrawMeshTasksIndirectCountEXT-None-08115";
        descriptor_buffer_set_offset_missing = "VUID-vkCmdDrawMeshTasksIndirectCountEXT-None-08117";
        image_view_dim                     = "VUID-vkCmdDrawMeshTasksIndirectCountEXT-viewType-07752";
        image_view_numeric_format          = "VUID-vkCmdDrawMeshTasksIndirectCountEXT-format-07753";
    }
};

struct DispatchVuidsCmdDrawIndirectByteCountEXT: DrawDispatchVuid {
    DispatchVuidsCmdDrawIndirectByteCountEXT() : DrawDispatchVuid() {
        pipeline_bound                     = "VUID-vkCmdDrawIndirectByteCountEXT-None-02700";
        vertex_binding                     = "VUID-vkCmdDrawIndirectByteCountEXT-None-04007";
        vertex_binding_null                = "VUID-vkCmdDrawIndirectByteCountEXT-None-04008";
        compatible_pipeline                = "VUID-vkCmdDrawIndirectByteCountEXT-None-02697";
        render_pass_compatible             = "VUID-vkCmdDrawIndirectByteCountEXT-renderPass-02684";
        subpass_index                      = "VUID-vkCmdDrawIndirectByteCountEXT-subpass-02685";
        sample_location                    = "VUID-vkCmdDrawIndirectByteCountEXT-sampleLocationsEnable-02689";
        linear_filter_sampler              = "VUID-vkCmdDrawIndirectByteCountEXT-magFilter-04553";
        linear_mipmap_sampler              = "VUID-vkCmdDrawIndirectByteCountEXT-mipmapMode-04770";
        cubic_sampler                      = "VUID-vkCmdDrawIndirectByteCountEXT-None-02692";
        indirect_protected_cb              = "VUID-vkCmdDrawIndirectByteCountEXT-commandBuffer-02646";
        indirect_contiguous_memory         = "VUID-vkCmdDrawIndirectByteCountEXT-counterBuffer-04567",
        indirect_buffer_bit                = "VUID-vkCmdDrawIndirectByteCountEXT-counterBuffer-02290";
        viewport_count                     = "VUID-vkCmdDrawIndirectByteCountEXT-viewportCount-03417";
        scissor_count                      = "VUID-vkCmdDrawIndirectByteCountEXT-scissorCount-03418";
        viewport_scissor_count             = "VUID-vkCmdDrawIndirectByteCountEXT-viewportCount-03419";
        primitive_topology_class           = "VUID-vkCmdDrawIndirectByteCountEXT-primitiveTopology-03420";
        primitive_topology_class_ds3       = "VUID-vkCmdDrawIndirectByteCountEXT-dynamicPrimitiveTopologyUnrestricted-07500";
        dynamic_primitive_topology         = "VUID-vkCmdDrawIndirectByteCountEXT-None-07842";
        corner_sampled_address_mode        = "VUID-vkCmdDrawIndirectByteCountEXT-flags-02696";
        imageview_atomic                   = "VUID-vkCmdDrawIndirectByteCountEXT-None-02691";
        push_constants_set                 = "VUID-vkCmdDrawIndirectByteCountEXT-maintenance4-06425";
        image_subresources_render_pass_write = "VUID-vkCmdDrawIndirectByteCountEXT-None-06537";
        image_subresources_subpass_read    = "VUID-vkCmdDrawIndirectByteCountEXT-None-06538";
        image_subresources_subpass_write   = "VUID-vkCmdDrawIndirectByteCountEXT-None-06539";
        descriptor_valid                   = "VUID-vkCmdDrawIndirectByteCountEXT-None-02699";
        sampler_imageview_type             = "VUID-vkCmdDrawIndirectByteCountEXT-None-02702";
        sampler_implicitLod_dref_proj      = "VUID-vkCmdDrawIndirectByteCountEXT-None-02703";
        sampler_bias_offset                = "VUID-vkCmdDrawIndirectByteCountEXT-None-02704";
        vertex_binding_attribute           = "VUID-vkCmdDrawIndirectByteCountEXT-None-02721";
        dynamic_state_setting_commands     = "VUID-vkCmdDrawIndirectByteCountEXT-None-02859";
        rasterization_samples              = "VUID-vkCmdDrawIndirectByteCountEXT-rasterizationSamples-04740";
        msrtss_rasterization_samples       = "VUID-vkCmdDrawIndirectByteCountEXT-multisampledRenderToSingleSampled-07284";
        unprotected_command_buffer         = "VUID-vkCmdDrawIndirectByteCountEXT-commandBuffer-02707";
        max_multiview_instance_index       = "VUID-vkCmdDrawIndirectByteCountEXT-maxMultiviewInstanceIndex-02688";
        img_filter_cubic                   = "VUID-vkCmdDrawIndirectByteCountEXT-None-02693";
        filter_cubic                       = "VUID-vkCmdDrawIndirectByteCountEXT-filterCubic-02694";
        filter_cubic_min_max               = "VUID-vkCmdDrawIndirectByteCountEXT-filterCubicMinmax-02695";
        viewport_count_primitive_shading_rate = "VUID-vkCmdDrawIndirectByteCountEXT-primitiveFragmentShadingRateWithMultipleViewports-04552";
        patch_control_points               = "VUID-vkCmdDrawIndirectByteCountEXT-None-04875";
        rasterizer_discard_enable          = "VUID-vkCmdDrawIndirectByteCountEXT-None-04876";
        depth_bias_enable                  = "VUID-vkCmdDrawIndirectByteCountEXT-None-04877";
        logic_op                           = "VUID-vkCmdDrawIndirectByteCountEXT-logicOp-04878";
        primitive_restart_enable           = "VUID-vkCmdDrawIndirectByteCountEXT-None-04879";
        vertex_input_binding_stride        = "VUID-vkCmdDrawIndirectByteCountEXT-pStrides-04884";
        vertex_input                       = "VUID-vkCmdDrawIndirectByteCountEXT-None-04914";
        blend_enable                       = "VUID-vkCmdDrawIndirectByteCountEXT-blendEnable-04727";
        dynamic_discard_rectangle          = "VUID-vkCmdDrawIndirectByteCountEXT-None-07751";
        dynamic_color_write_enable         = "VUID-vkCmdDrawIndirectByteCountEXT-None-07749";
        dynamic_color_write_enable_count   = "VUID-vkCmdDrawIndirectByteCountEXT-attachmentCount-07750";
        dynamic_rendering_view_mask        = "VUID-vkCmdDrawIndirectByteCountEXT-viewMask-06178";
        dynamic_rendering_color_count      = "VUID-vkCmdDrawIndirectByteCountEXT-colorAttachmentCount-06179";
        dynamic_rendering_color_formats    = "VUID-vkCmdDrawIndirectByteCountEXT-colorAttachmentCount-06180";
        dynamic_rendering_depth_format     = "VUID-vkCmdDrawIndirectByteCountEXT-pDepthAttachment-06181";
        dynamic_rendering_stencil_format   = "VUID-vkCmdDrawIndirectByteCountEXT-pStencilAttachment-06182";
        dynamic_rendering_fsr              = "VUID-vkCmdDrawIndirectByteCountEXT-imageView-06183";
        dynamic_rendering_fdm              = "VUID-vkCmdDrawIndirectByteCountEXT-imageView-06184";
        dynamic_rendering_color_sample     = "VUID-vkCmdDrawIndirectByteCountEXT-colorAttachmentCount-06185";
        dynamic_rendering_depth_sample     = "VUID-vkCmdDrawIndirectByteCountEXT-pDepthAttachment-06186";
        dynamic_rendering_stencil_sample   = "VUID-vkCmdDrawIndirectByteCountEXT-pStencilAttachment-06187";
        dynamic_rendering_multi_sample     = "VUID-vkCmdDrawIndirectByteCountEXT-colorAttachmentCount-06188";
        dynamic_rendering_06189            = "VUID-vkCmdDrawIndirectByteCountEXT-pDepthAttachment-06189";
        dynamic_rendering_06190            = "VUID-vkCmdDrawIndirectByteCountEXT-pStencilAttachment-06190";
        dynamic_rendering_06198            = "VUID-vkCmdDrawIndirectByteCountEXT-renderPass-06198";
        dynamic_rendering_07285            = "VUID-vkCmdDrawIndirectByteCountEXT-multisampledRenderToSingleSampled-07285";
        dynamic_rendering_07286            = "VUID-vkCmdDrawIndirectByteCountEXT-multisampledRenderToSingleSampled-07286";
        dynamic_rendering_07287            = "VUID-vkCmdDrawIndirectByteCountEXT-multisampledRenderToSingleSampled-07287";
        image_view_access_64               = "VUID-vkCmdDrawIndirectByteCountEXT-SampledType-04470";
        image_view_access_32               = "VUID-vkCmdDrawIndirectByteCountEXT-SampledType-04471";
        buffer_view_access_64              = "VUID-vkCmdDrawIndirectByteCountEXT-SampledType-04472";
        buffer_view_access_32              = "VUID-vkCmdDrawIndirectByteCountEXT-SampledType-04473";
        storage_image_read_without_format  = "VUID-vkCmdDrawIndirectByteCountEXT-OpTypeImage-07028";
        storage_image_write_without_format = "VUID-vkCmdDrawIndirectByteCountEXT-OpTypeImage-07027";
        storage_texel_buffer_read_without_format  = "VUID-vkCmdDrawIndirectByteCountEXT-OpTypeImage-07030";
        storage_texel_buffer_write_without_format = "VUID-vkCmdDrawIndirectByteCountEXT-OpTypeImage-07029";
        storage_image_write_texel_count           = "VUID-vkCmdDrawIndirectByteCountEXT-None-04115";
        storage_texel_buffer_write_texel_count    = "VUID-vkCmdDrawIndirectByteCountEXT-OpImageWrite-04469";
        depth_compare_sample               = "VUID-vkCmdDrawIndirectByteCountEXT-None-06479";
        depth_read_only                    = "VUID-vkCmdDrawIndirectByteCountEXT-None-06886";
        stencil_read_only                  = "VUID-vkCmdDrawIndirectByteCountEXT-None-06887";
        dynamic_sample_locations           = "VUID-vkCmdDrawIndirectByteCountEXT-None-06666";
        dynamic_tessellation_domain_origin = "VUID-vkCmdDrawIndirectByteCountEXT-None-07619";
        dynamic_depth_clamp_enable         = "VUID-vkCmdDrawIndirectByteCountEXT-None-07620";
        dynamic_polygon_mode               = "VUID-vkCmdDrawIndirectByteCountEXT-None-07621";
        dynamic_rasterization_samples      = "VUID-vkCmdDrawIndirectByteCountEXT-None-07622";
        dynamic_sample_mask                = "VUID-vkCmdDrawIndirectByteCountEXT-None-07623";
        dynamic_alpha_to_coverage_enable   = "VUID-vkCmdDrawIndirectByteCountEXT-None-07624";
        dynamic_alpha_to_one_enable        = "VUID-vkCmdDrawIndirectByteCountEXT-None-07625";
        dynamic_logic_op_enable            = "VUID-vkCmdDrawIndirectByteCountEXT-None-07626";
        dynamic_color_blend_enable         = "VUID-vkCmdDrawIndirectByteCountEXT-firstAttachment-07476";
        dynamic_color_blend_equation       = "VUID-vkCmdDrawIndirectByteCountEXT-firstAttachment-07477";
        dynamic_color_write_mask           = "VUID-vkCmdDrawIndirectByteCountEXT-firstAttachment-07478";
        dynamic_rasterization_stream       = "VUID-vkCmdDrawIndirectByteCountEXT-None-07630";
        dynamic_conservative_rasterization_mode = "VUID-vkCmdDrawIndirectByteCountEXT-None-07631";
        dynamic_extra_primitive_overestimation_size = "VUID-vkCmdDrawIndirectByteCountEXT-None-07632";
        dynamic_depth_clip_enable          = "VUID-vkCmdDrawIndirectByteCountEXT-None-07633";
        dynamic_sample_locations_enable    = "VUID-vkCmdDrawIndirectByteCountEXT-None-07634";
        dynamic_color_blend_advanced       = "VUID-vkCmdDrawIndirectByteCountEXT-firstAttachment-07479";
        dynamic_provoking_vertex_mode      = "VUID-vkCmdDrawIndirectByteCountEXT-None-07636";
        dynamic_line_rasterization_mode    = "VUID-vkCmdDrawIndirectByteCountEXT-None-07637";
        dynamic_line_stipple_enable        = "VUID-vkCmdDrawIndirectByteCountEXT-None-07638";
        dynamic_depth_clip_negative_one_to_one = "VUID-vkCmdDrawIndirectByteCountEXT-None-07639";
        dynamic_viewport_w_scaling_enable  = "VUID-vkCmdDrawIndirectByteCountEXT-None-07640";
        dynamic_viewport_swizzle           = "VUID-vkCmdDrawIndirectByteCountEXT-None-07641";
        dynamic_coverage_to_color_enable   = "VUID-vkCmdDrawIndirectByteCountEXT-None-07642";
        dynamic_coverage_to_color_location = "VUID-vkCmdDrawIndirectByteCountEXT-None-07643";
        dynamic_coverage_modulation_mode   = "VUID-vkCmdDrawIndirectByteCountEXT-None-07644";
        dynamic_coverage_modulation_table_enable = "VUID-vkCmdDrawIndirectByteCountEXT-None-07645";
        dynamic_coverage_modulation_table  = "VUID-vkCmdDrawIndirectByteCountEXT-None-07646";
        dynamic_coverage_reduction_mode    = "VUID-vkCmdDrawIndirectByteCountEXT-None-07647";
        dynamic_representative_fragment_test_enable  = "VUID-vkCmdDrawIndirectByteCountEXT-None-07648";
        dynamic_shading_rate_image_enable  = "VUID-vkCmdDrawIndirectByteCountEXT-None-07649";
        dynamic_viewport                   = "VUID-vkCmdDrawIndirectByteCountEXT-None-07831";
        dynamic_scissor                    = "VUID-vkCmdDrawIndirectByteCountEXT-None-07832";
        dynamic_depth_bias                 = "VUID-vkCmdDrawIndirectByteCountEXT-None-07834";
        dynamic_line_width                 = "VUID-vkCmdDrawIndirectByteCountEXT-None-07833";
        dynamic_line_stipple_ext           = "VUID-vkCmdDrawIndirectByteCountEXT-None-07849";
        dynamic_blend_constants            = "VUID-vkCmdDrawIndirectByteCountEXT-None-07835";
        dynamic_depth_bounds               = "VUID-vkCmdDrawIndirectByteCountEXT-None-07836";
        dynamic_stencil_compare_mask       = "VUID-vkCmdDrawIndirectByteCountEXT-None-07837";
        dynamic_stencil_write_mask         = "VUID-vkCmdDrawIndirectByteCountEXT-None-07838";
        dynamic_stencil_reference          = "VUID-vkCmdDrawIndirectByteCountEXT-None-07839";
        dynamic_state_inherited            = "VUID-vkCmdDrawIndirectByteCountEXT-None-07850";
        primitives_generated               = "VUID-vkCmdDrawIndirectByteCountEXT-primitivesGeneratedQueryWithRasterizerDiscard-06708";
        primitives_generated_streams       = "VUID-vkCmdDrawIndirectByteCountEXT-primitivesGeneratedQueryWithNonZeroStreams-06709";
        invalid_mesh_shader_stages         = "VUID-vkCmdDrawIndirectByteCountEXT-stage-06481";
        descriptor_buffer_bit_set          = "VUID-vkCmdDrawIndirectByteCountEXT-None-08114";
        descriptor_buffer_bit_not_set      = "VUID-vkCmdDrawIndirectByteCountEXT-None-08115";
        descriptor_buffer_set_offset_missing = "VUID-vkCmdDrawIndirectByteCountEXT-None-08117";
        image_view_dim                     = "VUID-vkCmdDrawIndirectByteCountEXT-viewType-07752";
        image_view_numeric_format          = "VUID-vkCmdDrawIndirectByteCountEXT-format-07753";
        stippled_rectangular_lines         = "VUID-vkCmdDrawIndirectByteCountEXT-stippledLineEnable-07495";
        stippled_bresenham_lines           = "VUID-vkCmdDrawIndirectByteCountEXT-stippledLineEnable-07496";
        stippled_smooth_lines              = "VUID-vkCmdDrawIndirectByteCountEXT-stippledLineEnable-07497";
        stippled_default_strict            = "VUID-vkCmdDrawIndirectByteCountEXT-stippledLineEnable-07498";
    }
};

struct DispatchVuidsCmdDispatchBase: DrawDispatchVuid {
    DispatchVuidsCmdDispatchBase() : DrawDispatchVuid() {
        pipeline_bound                     = "VUID-vkCmdDispatchBase-None-02700";
        compatible_pipeline                = "VUID-vkCmdDispatchBase-None-02697";
        linear_filter_sampler              = "VUID-vkCmdDispatchBase-magFilter-04553";
        linear_mipmap_sampler              = "VUID-vkCmdDispatchBase-mipmapMode-04770";
        cubic_sampler                      = "VUID-vkCmdDispatchBase-None-02692";
        corner_sampled_address_mode        = "VUID-vkCmdDispatchBase-flags-02696";
        imageview_atomic                   = "VUID-vkCmdDispatchBase-None-02691";
        push_constants_set                 = "VUID-vkCmdDispatchBase-maintenance4-06425";
        descriptor_valid                   = "VUID-vkCmdDispatchBase-None-02699";
        sampler_imageview_type             = "VUID-vkCmdDispatchBase-None-02702";
        sampler_implicitLod_dref_proj      = "VUID-vkCmdDispatchBase-None-02703";
        sampler_bias_offset                = "VUID-vkCmdDispatchBase-None-02704";
        dynamic_state_setting_commands     = "VUID-vkCmdDispatchBase-None-02859";
        unprotected_command_buffer         = "VUID-vkCmdDispatchBase-commandBuffer-02707";
        protected_command_buffer           = "VUID-vkCmdDispatchBase-commandBuffer-02712";
        img_filter_cubic                   = "VUID-vkCmdDispatchBase-None-02693";
        filter_cubic                       = "VUID-vkCmdDispatchBase-filterCubic-02694";
        filter_cubic_min_max               = "VUID-vkCmdDispatchBase-filterCubicMinmax-02695";
        image_view_access_64               = "VUID-vkCmdDispatchBase-SampledType-04470";
        image_view_access_32               = "VUID-vkCmdDispatchBase-SampledType-04471";
        buffer_view_access_64              = "VUID-vkCmdDispatchBase-SampledType-04472";
        buffer_view_access_32              = "VUID-vkCmdDispatchBase-SampledType-04473";
        storage_image_read_without_format  = "VUID-vkCmdDispatchBase-OpTypeImage-07028";
        storage_image_write_without_format = "VUID-vkCmdDispatchBase-OpTypeImage-07027";
        storage_texel_buffer_read_without_format  = "VUID-vkCmdDispatchBase-OpTypeImage-07030";
        storage_texel_buffer_write_without_format = "VUID-vkCmdDispatchBase-OpTypeImage-07029";
        storage_image_write_texel_count           = "VUID-vkCmdDispatchBase-None-04115";
        storage_texel_buffer_write_texel_count    = "VUID-vkCmdDispatchBase-OpImageWrite-04469";
        depth_compare_sample               = "VUID-vkCmdDispatchBase-None-06479";
        descriptor_buffer_bit_set          = "VUID-vkCmdDispatchBase-None-08114";
        descriptor_buffer_bit_not_set      = "VUID-vkCmdDispatchBase-None-08115";
        descriptor_buffer_set_offset_missing = "VUID-vkCmdDispatchBase-None-08117";
        image_view_dim                     = "VUID-vkCmdDispatchBase-viewType-07752";
        image_view_numeric_format          = "VUID-vkCmdDispatchBase-format-07753";
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

// Generic function to handle validation for all CmdDraw* type functions
bool CoreChecks::ValidateCmdDrawType(const CMD_BUFFER_STATE &cb_state, bool indexed, VkPipelineBindPoint bind_point,
                                     CMD_TYPE cmd_type) const {
    bool skip = false;
    skip |= ValidateCmd(cb_state, cmd_type);
    skip |= ValidateCmdBufDrawState(cb_state, cmd_type, indexed, bind_point);
    skip |= ValidateCmdRayQueryState(cb_state, cmd_type, bind_point);
    return skip;
}

bool CoreChecks::ValidateCmdDrawInstance(const CMD_BUFFER_STATE &cb_state, uint32_t instanceCount, uint32_t firstInstance,
                                         CMD_TYPE cmd_type) const {
    bool skip = false;
    const DrawDispatchVuid vuid = GetDrawDispatchVuid(cmd_type);
    const char *caller = CommandTypeString(cmd_type);

    // Verify maxMultiviewInstanceIndex
    if (cb_state.activeRenderPass && cb_state.activeRenderPass->renderPass() && enabled_features.multiview_features.multiview &&
        ((instanceCount + firstInstance) > phys_dev_ext_props.multiview_props.maxMultiviewInstanceIndex)) {
        const LogObjectList objlist(cb_state.Handle(), cb_state.activeRenderPass->Handle());
        skip |= LogError(objlist, vuid.max_multiview_instance_index,
                         "%s: renderpass %s multiview is enabled, and maxMultiviewInstanceIndex: %" PRIu32
                         ", but instanceCount: %" PRIu32 "and firstInstance: %" PRIu32 ".",
                         caller, report_data->FormatHandle(cb_state.activeRenderPass->Handle()).c_str(),
                         phys_dev_ext_props.multiview_props.maxMultiviewInstanceIndex, instanceCount, firstInstance);
    }
    return skip;
}

bool CoreChecks::ValidateVTGShaderStages(const CMD_BUFFER_STATE &cb_state, CMD_TYPE cmd_type) const {
    bool skip = false;
    const DrawDispatchVuid vuid = GetDrawDispatchVuid(cmd_type);
    const char *api_name = CommandTypeString(cmd_type);

    const auto *pipeline_state = cb_state.lastBound[VK_PIPELINE_BIND_POINT_GRAPHICS].pipeline_state;
    if (pipeline_state && pipeline_state->active_shaders & (VK_SHADER_STAGE_TASK_BIT_EXT | VK_SHADER_STAGE_MESH_BIT_EXT)) {
        skip |= LogError(
            cb_state.commandBuffer(), vuid.invalid_mesh_shader_stages,
            "%s : The bound graphics pipeline must not have been created with "
            "VK_SHADER_STAGE_TASK_BIT_EXT or VK_SHADER_STAGE_MESH_BIT_EXT. Active shader stages on the bound pipeline are %s.",
            api_name, string_VkShaderStageFlags(pipeline_state->active_shaders).c_str());
    }
    return skip;
}

bool CoreChecks::ValidateMeshShaderStage(const CMD_BUFFER_STATE &cb_state, CMD_TYPE cmd_type, bool is_NV) const {
    bool skip = false;
    const DrawDispatchVuid vuid = GetDrawDispatchVuid(cmd_type);
    const char *api_name = CommandTypeString(cmd_type);

    const auto *pipeline_state = cb_state.lastBound[VK_PIPELINE_BIND_POINT_GRAPHICS].pipeline_state;
    if (pipeline_state && !(pipeline_state->active_shaders & VK_SHADER_STAGE_MESH_BIT_EXT)) {
        skip |= LogError(cb_state.commandBuffer(), vuid.missing_mesh_shader_stages,
                         "%s : The current pipeline bound to VK_PIPELINE_BIND_POINT_GRAPHICS must contain a shader stage using the "
                         "%s Execution Model. Active shader stages on the bound pipeline are %s.",
                         api_name, is_NV ? "MeshNV" : "MeshEXT", string_VkShaderStageFlags(pipeline_state->active_shaders).c_str());
    }
    if (pipeline_state &&
        (pipeline_state->active_shaders & (VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT |
                                           VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT | VK_SHADER_STAGE_GEOMETRY_BIT))) {
        skip |= LogError(cb_state.commandBuffer(), vuid.mesh_shader_stages,
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
    skip |= ValidateCmdDrawType(*cb_state, false, VK_PIPELINE_BIND_POINT_GRAPHICS, CMD_DRAW);
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
    skip |= ValidateCmdDrawType(*cb_state, false, VK_PIPELINE_BIND_POINT_GRAPHICS, CMD_DRAWMULTIEXT);
    skip |= ValidateVTGShaderStages(*cb_state, CMD_DRAWMULTIEXT);
    return skip;
}

bool CoreChecks::ValidateCmdDrawIndexedBufferSize(const CMD_BUFFER_STATE &cb_state, uint32_t indexCount, uint32_t firstIndex,
                                                  const char *caller, const char *first_index_vuid) const {
    bool skip = false;
    if (!enabled_features.robustness2_features.robustBufferAccess2 && cb_state.index_buffer_binding.bound()) {
        const auto &index_buffer_binding = cb_state.index_buffer_binding;
        const uint32_t index_size = GetIndexAlignment(index_buffer_binding.index_type);
        VkDeviceSize end_offset = static_cast<VkDeviceSize>(index_size * (firstIndex + indexCount)) + index_buffer_binding.offset;
        if (end_offset > index_buffer_binding.size) {
            skip |= LogError(index_buffer_binding.buffer_state->buffer(), first_index_vuid,
                             "%s: index size (%u) * (firstIndex (%u) + indexCount (%u)) "
                             "+ binding offset (%" PRIuLEAST64 ") = an ending offset of %" PRIuLEAST64
                             " bytes, which is greater than the index buffer size (%" PRIuLEAST64 ").",
                             caller, index_size, firstIndex, indexCount, index_buffer_binding.offset, end_offset,
                             index_buffer_binding.size);
        }
    }
    return skip;
}

bool CoreChecks::PreCallValidateCmdDrawIndexed(VkCommandBuffer commandBuffer, uint32_t indexCount, uint32_t instanceCount,
                                               uint32_t firstIndex, int32_t vertexOffset, uint32_t firstInstance) const {
    bool skip = false;
    auto cb_state = GetRead<CMD_BUFFER_STATE>(commandBuffer);
    skip |= ValidateCmdDrawInstance(*cb_state, instanceCount, firstInstance, CMD_DRAWINDEXED);
    skip |= ValidateCmdDrawType(*cb_state, true, VK_PIPELINE_BIND_POINT_GRAPHICS, CMD_DRAWINDEXED);
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
    skip |= ValidateCmdDrawType(*cb_state, true, VK_PIPELINE_BIND_POINT_GRAPHICS, CMD_DRAWMULTIINDEXEDEXT);
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
    bool skip = ValidateCmdDrawType(*cb_state, false, VK_PIPELINE_BIND_POINT_GRAPHICS, CMD_DRAWINDIRECT);
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
    bool skip = ValidateCmdDrawType(*cb_state, true, VK_PIPELINE_BIND_POINT_GRAPHICS, CMD_DRAWINDEXEDINDIRECT);
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
    skip |= ValidateCmdDrawType(*cb_state, false, VK_PIPELINE_BIND_POINT_COMPUTE, CMD_DISPATCH);
    return skip;
}

bool CoreChecks::ValidateBaseGroups(const CMD_BUFFER_STATE &cb_state, uint32_t baseGroupX, uint32_t baseGroupY, uint32_t baseGroupZ,
                                    const char *apiName) const {
    bool skip = false;
    if (baseGroupX || baseGroupY || baseGroupZ) {
        const auto lv_bind_point = ConvertToLvlBindPoint(VK_PIPELINE_BIND_POINT_COMPUTE);
        const auto *pipeline_state = cb_state.lastBound[lv_bind_point].pipeline_state;
        if (pipeline_state && !(pipeline_state->GetPipelineCreateFlags() & VK_PIPELINE_CREATE_DISPATCH_BASE)) {
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
    skip |= ValidateCmdDrawType(*cb_state, false, VK_PIPELINE_BIND_POINT_COMPUTE, CMD_DISPATCHBASE);
    skip |= ValidateBaseGroups(*cb_state, baseGroupX, baseGroupY, baseGroupZ, "vkCmdDispatchBase()");
    return skip;
}

bool CoreChecks::PreCallValidateCmdDispatchBaseKHR(VkCommandBuffer commandBuffer, uint32_t baseGroupX, uint32_t baseGroupY,
                                                   uint32_t baseGroupZ, uint32_t groupCountX, uint32_t groupCountY,
                                                   uint32_t groupCountZ) const {
    bool skip = false;
    auto cb_state = GetRead<CMD_BUFFER_STATE>(commandBuffer);
    skip |= ValidateCmdDrawType(*cb_state, false, VK_PIPELINE_BIND_POINT_COMPUTE, CMD_DISPATCHBASEKHR);
    skip |= ValidateBaseGroups(*cb_state, baseGroupX, baseGroupY, baseGroupZ, "vkCmdDispatchBaseKHR()");
    return skip;
}

bool CoreChecks::PreCallValidateCmdDispatchIndirect(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset) const {
    auto cb_state = GetRead<CMD_BUFFER_STATE>(commandBuffer);
    bool skip = ValidateCmdDrawType(*cb_state, false, VK_PIPELINE_BIND_POINT_COMPUTE, CMD_DISPATCHINDIRECT);
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
    skip |= ValidateCmdDrawType(*cb_state, false, VK_PIPELINE_BIND_POINT_GRAPHICS, cmd_type);
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
    skip |= ValidateCmdDrawType(*cb_state, true, VK_PIPELINE_BIND_POINT_GRAPHICS, cmd_type);
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
    skip |= ValidateCmdDrawType(*cb_state, false, VK_PIPELINE_BIND_POINT_GRAPHICS, CMD_DRAWINDIRECTBYTECOUNTEXT);
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
    bool skip = ValidateCmdDrawType(*cb_state, true, VK_PIPELINE_BIND_POINT_RAY_TRACING_NV, CMD_TRACERAYSNV);
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

    const auto lv_bind_point = ConvertToLvlBindPoint(VK_PIPELINE_BIND_POINT_RAY_TRACING_NV);
    const PIPELINE_STATE *pipeline_state = cb_state->lastBound[lv_bind_point].pipeline_state;
    if (!pipeline_state || (pipeline_state && !pipeline_state->pipeline())) {
        skip |= LogError(device, "VUID-vkCmdTraceRaysNV-None-02700",
                         "vkCmdTraceRaysKHR: A valid pipeline must be bound to the pipeline bind point used by this command.");
    }
    return skip;
}

bool CoreChecks::ValidateCmdTraceRaysKHR(bool isIndirect, VkCommandBuffer commandBuffer,
                                         const VkStridedDeviceAddressRegionKHR *pRaygenShaderBindingTable,
                                         const VkStridedDeviceAddressRegionKHR *pMissShaderBindingTable,
                                         const VkStridedDeviceAddressRegionKHR *pHitShaderBindingTable,
                                         const VkStridedDeviceAddressRegionKHR *pCallableShaderBindingTable) const {
    auto cb_state = GetRead<CMD_BUFFER_STATE>(commandBuffer);
    bool skip = ValidateCmdDrawType(*cb_state, true, VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR, CMD_TRACERAYSKHR);
    const auto lv_bind_point = ConvertToLvlBindPoint(VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR);
    const PIPELINE_STATE *pipeline_state = cb_state->lastBound[lv_bind_point].pipeline_state;
    const char *rt_func_name = isIndirect ? "vkCmdTraceRaysIndirectKHR" : "vkCmdTraceRaysKHR";

    if (!pipeline_state || (pipeline_state && !pipeline_state->pipeline())) {
        const char *vuid = isIndirect ? "VUID-vkCmdTraceRaysIndirectKHR-None-02700" : "VUID-vkCmdTraceRaysKHR-None-02700";
        skip |= LogError(device, vuid,
                         "vkCmdTraceRaysKHR: A valid pipeline must be bound to the pipeline bind point used by this command.");
    } else {  // bound to valid RT pipeline
        if (pHitShaderBindingTable) {
            if (pipeline_state->GetPipelineCreateFlags() & VK_PIPELINE_CREATE_RAY_TRACING_NO_NULL_INTERSECTION_SHADERS_BIT_KHR) {
                if (pHitShaderBindingTable->deviceAddress == 0) {
                    const char *vuid =
                        isIndirect ? "VUID-vkCmdTraceRaysIndirectKHR-flags-03697" : "VUID-vkCmdTraceRaysKHR-flags-03697";
                    skip |= LogError(device, vuid, "%s: pHitShaderBindingTable->deviceAddress (0).", rt_func_name);
                }
                if ((pHitShaderBindingTable->size == 0 || pHitShaderBindingTable->stride == 0)) {
                    const char *vuid =
                        isIndirect ? "VUID-vkCmdTraceRaysIndirectKHR-flags-03514" : "VUID-vkCmdTraceRaysKHR-flags-03514";
                    skip |=
                        LogError(device, vuid,
                                 "%s: pHitShaderBindingTable->size (%" PRIu64 ") and pHitShaderBindingTable->stride (%" PRIu64 ").",
                                 rt_func_name, pHitShaderBindingTable->size, pHitShaderBindingTable->stride);
                }
            }
            if (pipeline_state->GetPipelineCreateFlags() & VK_PIPELINE_CREATE_RAY_TRACING_NO_NULL_CLOSEST_HIT_SHADERS_BIT_KHR) {
                if (pHitShaderBindingTable->deviceAddress == 0) {
                    const char *vuid =
                        isIndirect ? "VUID-vkCmdTraceRaysIndirectKHR-flags-03696" : "VUID-vkCmdTraceRaysKHR-flags-03696";
                    skip |= LogError(device, vuid, "pHitShaderBindingTable->deviceAddress = 0");
                }
                if ((pHitShaderBindingTable->size == 0 || pHitShaderBindingTable->stride == 0)) {
                    const char *vuid =
                        isIndirect ? "VUID-vkCmdTraceRaysIndirectKHR-flags-03513" : "VUID-vkCmdTraceRaysKHR-flags-03513";
                    skip |=
                        LogError(device, vuid,
                                 "%s: pHitShaderBindingTable->size (%" PRIu64 ") and pHitShaderBindingTable->stride (%" PRIu64 ").",
                                 rt_func_name, pHitShaderBindingTable->size, pHitShaderBindingTable->stride);
                }
            }
            if (pipeline_state->GetPipelineCreateFlags() & VK_PIPELINE_CREATE_RAY_TRACING_NO_NULL_ANY_HIT_SHADERS_BIT_KHR) {
                // No vuid to check for pHitShaderBindingTable->deviceAddress == 0 with this flag

                if (pHitShaderBindingTable->size == 0 || pHitShaderBindingTable->stride == 0) {
                    const char *vuid =
                        isIndirect ? "VUID-vkCmdTraceRaysIndirectKHR-flags-03512" : "VUID-vkCmdTraceRaysKHR-flags-03512";
                    skip |=
                        LogError(device, vuid,
                                 "%s: pHitShaderBindingTable->size (%" PRIu64 ") and pHitShaderBindingTable->stride (%" PRIu64 ").",
                                 rt_func_name, pHitShaderBindingTable->size, pHitShaderBindingTable->stride);
                }
            }
        }

        if (pRaygenShaderBindingTable) {
            const char *vuid_single_device_memory = isIndirect ? "VUID-vkCmdTraceRaysIndirectKHR-pRayGenShaderBindingTable-03680"
                                                               : "VUID-vkCmdTraceRaysKHR-pRayGenShaderBindingTable-03680";
            const char *vuid_binding_table_flag = isIndirect ? "VUID-vkCmdTraceRaysIndirectKHR-pRayGenShaderBindingTable-03681"
                                                             : "VUID-vkCmdTraceRaysKHR-pRayGenShaderBindingTable-03681";
            skip |= ValidateRaytracingShaderBindingTable(commandBuffer, rt_func_name, vuid_single_device_memory,
                                                         vuid_binding_table_flag, *pRaygenShaderBindingTable,
                                                         "pRaygenShaderBindingTable");
        }

        if (pMissShaderBindingTable) {
            const char *vuid_single_device_memory = isIndirect ? "VUID-vkCmdTraceRaysIndirectKHR-pMissShaderBindingTable-03683"
                                                               : "VUID-vkCmdTraceRaysKHR-pMissShaderBindingTable-03683";
            const char *vuid_binding_table_flag = isIndirect ? "VUID-vkCmdTraceRaysIndirectKHR-pMissShaderBindingTable-03684"
                                                             : "VUID-vkCmdTraceRaysKHR-pMissShaderBindingTable-03684";
            skip |=
                ValidateRaytracingShaderBindingTable(commandBuffer, rt_func_name, vuid_single_device_memory,
                                                     vuid_binding_table_flag, *pMissShaderBindingTable, "pMissShaderBindingTable");
        }

        if (pHitShaderBindingTable) {
            const char *vuid_single_device_memory = isIndirect ? "VUID-vkCmdTraceRaysIndirectKHR-pHitShaderBindingTable-03687"
                                                               : "VUID-vkCmdTraceRaysKHR-pHitShaderBindingTable-03687";
            const char *vuid_binding_table_flag = isIndirect ? "VUID-vkCmdTraceRaysIndirectKHR-pHitShaderBindingTable-03688"
                                                             : "VUID-vkCmdTraceRaysKHR-pHitShaderBindingTable-03688";
            skip |=
                ValidateRaytracingShaderBindingTable(commandBuffer, rt_func_name, vuid_single_device_memory,
                                                     vuid_binding_table_flag, *pHitShaderBindingTable, "pHitShaderBindingTable");
        }

        if (pCallableShaderBindingTable) {
            const char *vuid_single_device_memory = isIndirect ? "VUID-vkCmdTraceRaysIndirectKHR-pCallableShaderBindingTable-03691"
                                                               : "VUID-vkCmdTraceRaysKHR-pCallableShaderBindingTable-03691";
            const char *vuid_binding_table_flag = isIndirect ? "VUID-vkCmdTraceRaysIndirectKHR-pCallableShaderBindingTable-03692"
                                                             : "VUID-vkCmdTraceRaysKHR-pCallableShaderBindingTable-03692";
            skip |= ValidateRaytracingShaderBindingTable(commandBuffer, rt_func_name, vuid_single_device_memory,
                                                         vuid_binding_table_flag, *pCallableShaderBindingTable,
                                                         "pCallableShaderBindingTable");
        }
    }
    return skip;
}

bool CoreChecks::PreCallValidateCmdTraceRaysKHR(VkCommandBuffer commandBuffer,
                                                const VkStridedDeviceAddressRegionKHR *pRaygenShaderBindingTable,
                                                const VkStridedDeviceAddressRegionKHR *pMissShaderBindingTable,
                                                const VkStridedDeviceAddressRegionKHR *pHitShaderBindingTable,
                                                const VkStridedDeviceAddressRegionKHR *pCallableShaderBindingTable, uint32_t width,
                                                uint32_t height, uint32_t depth) const {
    return ValidateCmdTraceRaysKHR(false, commandBuffer, pRaygenShaderBindingTable, pMissShaderBindingTable, pHitShaderBindingTable,
                                   pCallableShaderBindingTable);
}

bool CoreChecks::PreCallValidateCmdTraceRaysIndirectKHR(VkCommandBuffer commandBuffer,
                                                        const VkStridedDeviceAddressRegionKHR *pRaygenShaderBindingTable,
                                                        const VkStridedDeviceAddressRegionKHR *pMissShaderBindingTable,
                                                        const VkStridedDeviceAddressRegionKHR *pHitShaderBindingTable,
                                                        const VkStridedDeviceAddressRegionKHR *pCallableShaderBindingTable,
                                                        VkDeviceAddress indirectDeviceAddress) const {
    return ValidateCmdTraceRaysKHR(true, commandBuffer, pRaygenShaderBindingTable, pMissShaderBindingTable, pHitShaderBindingTable,
                                   pCallableShaderBindingTable);
}

bool CoreChecks::PreCallValidateCmdTraceRaysIndirect2KHR(VkCommandBuffer commandBuffer,
                                                         VkDeviceAddress indirectDeviceAddress) const {
    auto cb_state = GetRead<CMD_BUFFER_STATE>(commandBuffer);
    bool skip = ValidateCmdDrawType(*cb_state, true, VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR, CMD_TRACERAYSINDIRECTKHR);
    return skip;
}

bool CoreChecks::PreCallValidateCmdDrawMeshTasksNV(VkCommandBuffer commandBuffer, uint32_t taskCount, uint32_t firstTask) const {
    auto cb_state = GetRead<CMD_BUFFER_STATE>(commandBuffer);
    bool skip = ValidateCmdDrawType(*cb_state, false, VK_PIPELINE_BIND_POINT_GRAPHICS, CMD_DRAWMESHTASKSNV);
    skip |= ValidateMeshShaderStage(*cb_state, CMD_DRAWMESHTASKSNV, true);
    return skip;
}

bool CoreChecks::PreCallValidateCmdDrawMeshTasksIndirectNV(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                           uint32_t drawCount, uint32_t stride) const {
    auto cb_state = GetRead<CMD_BUFFER_STATE>(commandBuffer);
    bool skip = ValidateCmdDrawType(*cb_state, false, VK_PIPELINE_BIND_POINT_GRAPHICS, CMD_DRAWMESHTASKSINDIRECTNV);
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
    bool skip = ValidateCmdDrawType(*cb_state, false, VK_PIPELINE_BIND_POINT_GRAPHICS, CMD_DRAWMESHTASKSINDIRECTCOUNTNV);
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
    bool skip = ValidateCmdDrawType(*cb_state, false, VK_PIPELINE_BIND_POINT_GRAPHICS, CMD_DRAWMESHTASKSEXT);
    skip |= ValidateMeshShaderStage(*cb_state, CMD_DRAWMESHTASKSEXT, false);
    return skip;
}

bool CoreChecks::PreCallValidateCmdDrawMeshTasksIndirectEXT(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                            uint32_t drawCount, uint32_t stride) const {
    auto cb_state = GetRead<CMD_BUFFER_STATE>(commandBuffer);
    bool skip = ValidateCmdDrawType(*cb_state, false, VK_PIPELINE_BIND_POINT_GRAPHICS, CMD_DRAWMESHTASKSINDIRECTEXT);
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
    const DrawDispatchVuid vuid = GetDrawDispatchVuid(cmd_type);
    const char *caller_name = CommandTypeString(cmd_type);

    auto cb_state = GetRead<CMD_BUFFER_STATE>(commandBuffer);
    bool skip = ValidateCmdDrawType(*cb_state, false, VK_PIPELINE_BIND_POINT_GRAPHICS, cmd_type);
    auto buffer_state = Get<BUFFER_STATE>(buffer);
    auto count_buffer_state = Get<BUFFER_STATE>(countBuffer);
    skip |= ValidateIndirectCmd(*cb_state, *buffer_state, cmd_type);
    skip |= ValidateMemoryIsBoundToBuffer(commandBuffer, *count_buffer_state, caller_name, vuid.indirect_count_contiguous_memory);
    skip |= ValidateBufferUsageFlags(commandBuffer, *count_buffer_state, VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT, true,
                                     vuid.indirect_count_buffer_bit, "vkCmdDrawMeshTasksIndirectCountEXT()",
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
bool CoreChecks::ValidateCmdBufDrawState(const CMD_BUFFER_STATE &cb_state, CMD_TYPE cmd_type, const bool indexed,
                                         const VkPipelineBindPoint bind_point) const {
    const DrawDispatchVuid vuid = GetDrawDispatchVuid(cmd_type);
    const char *function = CommandTypeString(cmd_type);
    const auto lv_bind_point = ConvertToLvlBindPoint(bind_point);
    const auto &last_bound = cb_state.lastBound[lv_bind_point];
    const auto *last_pipeline = last_bound.pipeline_state;

    if (nullptr == last_pipeline) {
        return LogError(cb_state.commandBuffer(), vuid.pipeline_bound,
                        "Must not call %s on this command buffer while there is no %s pipeline bound.", function,
                        string_VkPipelineBindPoint(bind_point));
    }
    const PIPELINE_STATE &pipeline = *last_pipeline;

    bool result = false;

    for (const auto &ds : last_bound.per_set) {
        if (pipeline.descriptor_buffer_mode) {
            if (ds.bound_descriptor_set && !ds.bound_descriptor_set->IsPushDescriptor()) {
                const LogObjectList objlist(cb_state.Handle(), pipeline.Handle(), ds.bound_descriptor_set->Handle());
                result |= LogError(objlist, vuid.descriptor_buffer_set_offset_missing,
                                   "%s: pipeline bound to %s requires a descriptor buffer but has a bound descriptor set (%s)",
                                   function, string_VkPipelineBindPoint(bind_point),
                                   report_data->FormatHandle(ds.bound_descriptor_set->Handle()).c_str());
                break;
            }

        } else {
            if (ds.bound_descriptor_buffer.has_value()) {
                const LogObjectList objlist(cb_state.Handle(), pipeline.Handle());
                result |= LogError(objlist, vuid.descriptor_buffer_bit_not_set,
                                   "%s: pipeline bound to %s requires a descriptor set but has a bound descriptor buffer"
                                   " (index=%" PRIu32 " offset=%" PRIu64 ")",
                                   function, string_VkPipelineBindPoint(bind_point), ds.bound_descriptor_buffer->index,
                                   ds.bound_descriptor_buffer->offset);
                break;
            }
        }
    }

    if (VK_PIPELINE_BIND_POINT_GRAPHICS == bind_point) {
        result |= ValidateDrawDynamicState(cb_state, pipeline, cmd_type);
        result |= ValidatePipelineDrawtimeState(last_bound, cb_state, cmd_type, pipeline);

        if (indexed && !cb_state.index_buffer_binding.bound()) {
            return LogError(cb_state.commandBuffer(), vuid.index_binding,
                            "%s: Index buffer object has not been bound to this command buffer.", function);
        }

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
                            vuid.protected_command_buffer != kVUIDUndefined) {
                            result |= ValidateUnprotectedImage(cb_state, *view_state->image_state, function,
                                                               vuid.protected_command_buffer, image_desc.c_str());
                        }
                        result |= ValidateProtectedImage(cb_state, *view_state->image_state, function,
                                                         vuid.unprotected_command_buffer, image_desc.c_str());
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
        if (!pipeline.active_slots.empty() && !IsBoundSetCompat(pipeline.max_active_slot, last_bound, *pipeline_layout)) {
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
            objlist.add(last_bound.pipeline_layout);
            result |= LogError(objlist, vuid.compatible_pipeline,
                               "%s(): The %s (created with %s) statically uses descriptor set (index #%" PRIu32
                               ") which is not compatible with the currently bound descriptor set's pipeline layout (%s)",
                               function, report_data->FormatHandle(pipeline.pipeline()).c_str(), pipe_layouts_log.str().c_str(),
                               pipeline.max_active_slot, report_data->FormatHandle(last_bound.pipeline_layout).c_str());
        } else {
            // if the bound set is not copmatible, the rest will just be extra redundant errors
            for (const auto &set_binding_pair : pipeline.active_slots) {
                uint32_t set_index = set_binding_pair.first;
                const auto set_info = last_bound.per_set[set_index];
                if (!set_info.bound_descriptor_set) {
                    result |= LogError(cb_state.commandBuffer(), vuid.compatible_pipeline,
                                       "%s(): %s uses set #%" PRIu32 " but that set is not bound.", function,
                                       report_data->FormatHandle(pipeline.pipeline()).c_str(), set_index);
                } else if (!VerifySetLayoutCompatibility(*set_info.bound_descriptor_set, *pipeline_layout, set_index,
                                                         error_string)) {
                    // Set is bound but not compatible w/ overlapping pipeline_layout from PSO
                    VkDescriptorSet set_handle = set_info.bound_descriptor_set->GetSet();
                    const LogObjectList objlist(set_handle, pipeline_layout->layout());
                    result |= LogError(objlist, vuid.compatible_pipeline,
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
                            BindingReqMap delta_reqs;
                            std::set_difference(binding_req_map.begin(), binding_req_map.end(),
                                                set_info.validated_set_binding_req_map.begin(),
                                                set_info.validated_set_binding_req_map.end(),
                                                vvl::insert_iterator<BindingReqMap>(delta_reqs, delta_reqs.begin()));
                            result |=
                                ValidateDrawState(*descriptor_set, delta_reqs, set_info.dynamicOffsets, cb_state, function, vuid);
                        } else {
                            result |= ValidateDrawState(*descriptor_set, binding_req_map, set_info.dynamicOffsets, cb_state,
                                                        function, vuid);
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
        for (const auto &stage : pipeline.stage_state) {
            const auto *push_constants =
                stage.module_state->FindEntrypointPushConstant(stage.create_info->pName, stage.create_info->stage);
            if (!push_constants || !push_constants->IsUsed()) {
                continue;
            }

            // Edge case where if the shader is using push constants statically and there never was a vkCmdPushConstants
            if (!cb_state.push_constant_data_ranges && !enabled_features.core13.maintenance4) {
                const LogObjectList objlist(cb_state.commandBuffer(), pipeline_layout->layout(), pipeline.pipeline());
                result |= LogError(objlist, vuid.push_constants_set,
                                   "%s(): Shader in %s uses push-constant statically but vkCmdPushConstants was not called yet for "
                                   "pipeline layout %s.",
                                   function, string_VkShaderStageFlags(stage.stage_flag).c_str(),
                                   report_data->FormatHandle(pipeline_layout->layout()).c_str());
            }

            const auto it = cb_state.push_constant_data_update.find(stage.stage_flag);
            if (it == cb_state.push_constant_data_update.end()) {
                // This error has been printed in ValidatePushConstantUsage.
                break;
            }
        }
    }

    return result;
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
    const DrawDispatchVuid vuid = GetDrawDispatchVuid(cmd_type);
    const char *caller_name = CommandTypeString(cmd_type);

    skip |= ValidateMemoryIsBoundToBuffer(cb_state.commandBuffer(), buffer_state, caller_name, vuid.indirect_contiguous_memory);
    skip |= ValidateBufferUsageFlags(cb_state.commandBuffer(), buffer_state, VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT, true,
                                     vuid.indirect_buffer_bit, caller_name, "VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT");
    if (cb_state.unprotected == false) {
        skip |= LogError(cb_state.Handle(), vuid.indirect_protected_cb,
                         "%s: Indirect commands can't be used in protected command buffers.", caller_name);
    }
    return skip;
}

bool CoreChecks::ValidateIndirectCountCmd(const CMD_BUFFER_STATE &cb_state, const BUFFER_STATE &count_buffer_state,
                                          VkDeviceSize count_buffer_offset, CMD_TYPE cmd_type) const {
    bool skip = false;
    const DrawDispatchVuid vuid = GetDrawDispatchVuid(cmd_type);
    const char *caller_name = CommandTypeString(cmd_type);

    skip |= ValidateMemoryIsBoundToBuffer(cb_state.commandBuffer(), count_buffer_state, caller_name,
                                          vuid.indirect_count_contiguous_memory);
    skip |= ValidateBufferUsageFlags(cb_state.commandBuffer(), count_buffer_state, VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT, true,
                                     vuid.indirect_count_buffer_bit, caller_name, "VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT");
    if (count_buffer_offset + sizeof(uint32_t) > count_buffer_state.createInfo.size) {
        const LogObjectList objlist(cb_state.Handle(), count_buffer_state.Handle());
        skip |= LogError(objlist, vuid.indirect_count_offset,
                         "%s: countBufferOffset (%" PRIu64 ") + sizeof(uint32_t) is greater than the buffer size of %" PRIu64 ".",
                         caller_name, count_buffer_offset, count_buffer_state.createInfo.size);
    }
    return skip;
}
