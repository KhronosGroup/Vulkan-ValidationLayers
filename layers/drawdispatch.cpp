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
 *
 * Author: Cody Northrop <cnorthrop@google.com>
 * Author: Michael Lentine <mlentine@google.com>
 * Author: Tobin Ehlis <tobine@google.com>
 * Author: Chia-I Wu <olv@google.com>
 * Author: Chris Forbes <chrisf@ijw.co.nz>
 * Author: Mark Lobodzinski <mark@lunarg.com>
 * Author: Ian Elliott <ianelliott@google.com>
 * Author: Dave Houlton <daveh@lunarg.com>
 * Author: Dustin Graves <dustin@lunarg.com>
 * Author: Jeremy Hayes <jeremy@lunarg.com>
 * Author: Jon Ashburn <jon@lunarg.com>
 * Author: Karl Schultz <karl@lunarg.com>
 * Author: Mark Young <marky@lunarg.com>
 * Author: Mike Schuchardt <mikes@lunarg.com>
 * Author: Mike Weiblen <mikew@lunarg.com>
 * Author: Tony Barbour <tony@LunarG.com>
 * Author: John Zulauf <jzulauf@lunarg.com>
 * Author: Shannon McPherson <shannon@lunarg.com>
 * Author: Jeremy Kniager <jeremyk@lunarg.com>
 * Author: Nathaniel Cesario <nathaniel@lunarg.com>
 * Author: Tobias Hector <tobias.hector@amd.com>
 */

#include "chassis.h"
#include "core_validation.h"

// clang-format off
struct DispatchVuidsCmdDraw : DrawDispatchVuid {
    DispatchVuidsCmdDraw() : DrawDispatchVuid() {
        pipeline_bound                     = "VUID-vkCmdDraw-None-02700";
        dynamic_state                      = "VUID-vkCmdDraw-commandBuffer-02701";
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
        primitive_topology                 = "VUID-vkCmdDraw-primitiveTopology-03420";
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
        dynamic_color_blend_enable         = "VUID-vkCmdDraw-None-07627";
        dynamic_color_blend_equation       = "VUID-vkCmdDraw-None-07628";
        dynamic_color_write_mask           = "VUID-vkCmdDraw-None-07629";
        dynamic_rasterization_stream       = "VUID-vkCmdDraw-None-07630";
        dynamic_conservative_rasterization_mode = "VUID-vkCmdDraw-None-07631";
        dynamic_extra_primitive_overestimation_size = "VUID-vkCmdDraw-None-07632";
        dynamic_depth_clip_enable          = "VUID-vkCmdDraw-None-07633";
        dynamic_sample_locations_enable    = "VUID-vkCmdDraw-None-07634";
        dynamic_color_blend_advanced       = "VUID-vkCmdDraw-None-07635";
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
        primitives_generated               = "VUID-vkCmdDraw-primitivesGeneratedQueryWithRasterizerDiscard-06708";
        primitives_generated_streams       = "VUID-vkCmdDraw-primitivesGeneratedQueryWithNonZeroStreams-06709";
        invalid_mesh_shader_stages         = "VUID-vkCmdDraw-stage-06481";
        descriptor_buffer_bit_set          = "VUID-vkCmdDraw-None-08114";
        descriptor_buffer_bit_not_set      = "VUID-vkCmdDraw-None-08115";
        descriptor_buffer_set_offset_missing = "VUID-vkCmdDraw-None-08117";
        image_view_dim                     = "VUID-vkCmdDraw-viewType-07752";
        image_view_numeric_format          = "VUID-vkCmdDraw-format-07753";
    }
};

struct DispatchVuidsCmdDrawMultiEXT : DrawDispatchVuid {
    DispatchVuidsCmdDrawMultiEXT() : DrawDispatchVuid() {
        pipeline_bound                     = "VUID-vkCmdDrawMultiEXT-None-02700";
        dynamic_state                      = "VUID-vkCmdDrawMultiEXT-commandBuffer-02701";
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
        primitive_topology                 = "VUID-vkCmdDrawMultiEXT-primitiveTopology-03420";
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
        dynamic_color_blend_enable         = "VUID-vkCmdDrawMultiEXT-None-07627";
        dynamic_color_blend_equation       = "VUID-vkCmdDrawMultiEXT-None-07628";
        dynamic_color_write_mask           = "VUID-vkCmdDrawMultiEXT-None-07629";
        dynamic_rasterization_stream       = "VUID-vkCmdDrawMultiEXT-None-07630";
        dynamic_conservative_rasterization_mode = "VUID-vkCmdDrawMultiEXT-None-07631";
        dynamic_extra_primitive_overestimation_size = "VUID-vkCmdDrawMultiEXT-None-07632";
        dynamic_depth_clip_enable          = "VUID-vkCmdDrawMultiEXT-None-07633";
        dynamic_sample_locations_enable    = "VUID-vkCmdDrawMultiEXT-None-07634";
        dynamic_color_blend_advanced       = "VUID-vkCmdDrawMultiEXT-None-07635";
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
        primitives_generated               = "VUID-vkCmdDrawMultiEXT-primitivesGeneratedQueryWithRasterizerDiscard-06708";
        primitives_generated_streams       = "VUID-vkCmdDrawMultiEXT-primitivesGeneratedQueryWithNonZeroStreams-06709";
        invalid_mesh_shader_stages         = "VUID-vkCmdDrawMultiEXT-stage-06481";
        descriptor_buffer_bit_set          = "VUID-vkCmdDrawMultiEXT-None-08114";
        descriptor_buffer_bit_not_set      = "VUID-vkCmdDrawMultiEXT-None-08115";
        descriptor_buffer_set_offset_missing = "VUID-vkCmdDrawMultiEXT-None-08117";
        image_view_dim                     = "VUID-vkCmdDrawMultiEXT-viewType-07752";
        image_view_numeric_format          = "VUID-vkCmdDrawMultiEXT-format-07753";
    }
};

struct DispatchVuidsCmdDrawIndexed : DrawDispatchVuid {
    DispatchVuidsCmdDrawIndexed() : DrawDispatchVuid() {
        pipeline_bound                     = "VUID-vkCmdDrawIndexed-None-02700";
        dynamic_state                      = "VUID-vkCmdDrawIndexed-commandBuffer-02701";
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
        primitive_topology                 = "VUID-vkCmdDrawIndexed-primitiveTopology-03420";
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
        dynamic_color_blend_enable         = "VUID-vkCmdDrawIndexed-None-07627";
        dynamic_color_blend_equation       = "VUID-vkCmdDrawIndexed-None-07628";
        dynamic_color_write_mask           = "VUID-vkCmdDrawIndexed-None-07629";
        dynamic_rasterization_stream       = "VUID-vkCmdDrawIndexed-None-07630";
        dynamic_conservative_rasterization_mode = "VUID-vkCmdDrawIndexed-None-07631";
        dynamic_extra_primitive_overestimation_size = "VUID-vkCmdDrawIndexed-None-07632";
        dynamic_depth_clip_enable          = "VUID-vkCmdDrawIndexed-None-07633";
        dynamic_sample_locations_enable    = "VUID-vkCmdDrawIndexed-None-07634";
        dynamic_color_blend_advanced       = "VUID-vkCmdDrawIndexed-None-07635";
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
        primitives_generated               = "VUID-vkCmdDrawIndexed-primitivesGeneratedQueryWithRasterizerDiscard-06708";
        primitives_generated_streams       = "VUID-vkCmdDrawIndexed-primitivesGeneratedQueryWithNonZeroStreams-06709";
        invalid_mesh_shader_stages         = "VUID-vkCmdDrawIndexed-stage-06481";
        descriptor_buffer_bit_set          = "VUID-vkCmdDrawIndexed-None-08114";
        descriptor_buffer_bit_not_set      = "VUID-vkCmdDrawIndexed-None-08115";
        descriptor_buffer_set_offset_missing = "VUID-vkCmdDrawIndexed-None-08117";
        image_view_dim                     = "VUID-vkCmdDrawIndexed-viewType-07752";
        image_view_numeric_format          = "VUID-vkCmdDrawIndexed-format-07753";
    }
};

struct DispatchVuidsCmdDrawMultiIndexedEXT : DrawDispatchVuid {
    DispatchVuidsCmdDrawMultiIndexedEXT() : DrawDispatchVuid() {
        pipeline_bound                     = "VUID-vkCmdDrawMultiIndexedEXT-None-02700";
        dynamic_state                      = "VUID-vkCmdDrawMultiIndexedEXT-commandBuffer-02701";
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
        primitive_topology                 = "VUID-vkCmdDrawMultiIndexedEXT-primitiveTopology-03420";
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
        dynamic_color_blend_enable         = "VUID-vkCmdDrawMultiIndexedEXT-None-07627";
        dynamic_color_blend_equation       = "VUID-vkCmdDrawMultiIndexedEXT-None-07628";
        dynamic_color_write_mask           = "VUID-vkCmdDrawMultiIndexedEXT-None-07629";
        dynamic_rasterization_stream       = "VUID-vkCmdDrawMultiIndexedEXT-None-07630";
        dynamic_conservative_rasterization_mode = "VUID-vkCmdDrawMultiIndexedEXT-None-07631";
        dynamic_extra_primitive_overestimation_size = "VUID-vkCmdDrawMultiIndexedEXT-None-07632";
        dynamic_depth_clip_enable          = "VUID-vkCmdDrawMultiIndexedEXT-None-07633";
        dynamic_sample_locations_enable    = "VUID-vkCmdDrawMultiIndexedEXT-None-07634";
        dynamic_color_blend_advanced       = "VUID-vkCmdDrawMultiIndexedEXT-None-07635";
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
        primitives_generated               = "VUID-vkCmdDrawMultiIndexedEXT-primitivesGeneratedQueryWithRasterizerDiscard-06708";
        primitives_generated_streams       = "VUID-vkCmdDrawMultiIndexedEXT-primitivesGeneratedQueryWithNonZeroStreams-06709";
        invalid_mesh_shader_stages         = "VUID-vkCmdDrawMultiIndexedEXT-stage-06481";
        descriptor_buffer_bit_set          = "VUID-vkCmdDrawMultiIndexedEXT-None-08114";
        descriptor_buffer_bit_not_set      = "VUID-vkCmdDrawMultiIndexedEXT-None-08115";
        descriptor_buffer_set_offset_missing = "VUID-vkCmdDrawMultiIndexedEXT-None-08117";
        image_view_dim                     = "VUID-vkCmdDrawMultiIndexedEXT-viewType-07752";
        image_view_numeric_format          = "VUID-vkCmdDrawMultiIndexedEXT-format-07753";
    }
};

struct DispatchVuidsCmdDrawIndirect : DrawDispatchVuid {
    DispatchVuidsCmdDrawIndirect() : DrawDispatchVuid() {
        pipeline_bound                     = "VUID-vkCmdDrawIndirect-None-02700";
        dynamic_state                      = "VUID-vkCmdDrawIndirect-commandBuffer-02701";
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
        primitive_topology                 = "VUID-vkCmdDrawIndirect-primitiveTopology-03420";
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
        dynamic_color_blend_enable         = "VUID-vkCmdDrawIndirect-None-07627";
        dynamic_color_blend_equation       = "VUID-vkCmdDrawIndirect-None-07628";
        dynamic_color_write_mask           = "VUID-vkCmdDrawIndirect-None-07629";
        dynamic_rasterization_stream       = "VUID-vkCmdDrawIndirect-None-07630";
        dynamic_conservative_rasterization_mode = "VUID-vkCmdDrawIndirect-None-07631";
        dynamic_extra_primitive_overestimation_size = "VUID-vkCmdDrawIndirect-None-07632";
        dynamic_depth_clip_enable          = "VUID-vkCmdDrawIndirect-None-07633";
        dynamic_sample_locations_enable    = "VUID-vkCmdDrawIndirect-None-07634";
        dynamic_color_blend_advanced       = "VUID-vkCmdDrawIndirect-None-07635";
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
        primitives_generated               = "VUID-vkCmdDrawIndirect-primitivesGeneratedQueryWithRasterizerDiscard-06708";
        primitives_generated_streams       = "VUID-vkCmdDrawIndirect-primitivesGeneratedQueryWithNonZeroStreams-06709";
        invalid_mesh_shader_stages         = "VUID-vkCmdDrawIndirect-stage-06481";
        descriptor_buffer_bit_set          = "VUID-vkCmdDrawIndirect-None-08114";
        descriptor_buffer_bit_not_set      = "VUID-vkCmdDrawIndirect-None-08115";
        descriptor_buffer_set_offset_missing = "VUID-vkCmdDrawIndirect-None-08117";
        image_view_dim                     = "VUID-vkCmdDrawIndirect-viewType-07752";
        image_view_numeric_format          = "VUID-vkCmdDrawIndirect-format-07753";
    }
};

struct DispatchVuidsCmdDrawIndexedIndirect : DrawDispatchVuid {
    DispatchVuidsCmdDrawIndexedIndirect() : DrawDispatchVuid() {
        pipeline_bound                     = "VUID-vkCmdDrawIndexedIndirect-None-02700";
        dynamic_state                      = "VUID-vkCmdDrawIndexedIndirect-commandBuffer-02701";
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
        primitive_topology                 = "VUID-vkCmdDrawIndexedIndirect-primitiveTopology-03420";
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
        dynamic_color_blend_enable         = "VUID-vkCmdDrawIndexedIndirect-None-07627";
        dynamic_color_blend_equation       = "VUID-vkCmdDrawIndexedIndirect-None-07628";
        dynamic_color_write_mask           = "VUID-vkCmdDrawIndexedIndirect-None-07629";
        dynamic_rasterization_stream       = "VUID-vkCmdDrawIndexedIndirect-None-07630";
        dynamic_conservative_rasterization_mode = "VUID-vkCmdDrawIndexedIndirect-None-07631";
        dynamic_extra_primitive_overestimation_size = "VUID-vkCmdDrawIndexedIndirect-None-07632";
        dynamic_depth_clip_enable          = "VUID-vkCmdDrawIndexedIndirect-None-07633";
        dynamic_sample_locations_enable    = "VUID-vkCmdDrawIndexedIndirect-None-07634";
        dynamic_color_blend_advanced       = "VUID-vkCmdDrawIndexedIndirect-None-07635";
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
        primitives_generated               = "VUID-vkCmdDrawIndexedIndirect-primitivesGeneratedQueryWithRasterizerDiscard-06708";
        primitives_generated_streams       = "VUID-vkCmdDrawIndexedIndirect-primitivesGeneratedQueryWithNonZeroStreams-06709";
        invalid_mesh_shader_stages         = "VUID-vkCmdDrawIndexedIndirect-stage-06481";
        descriptor_buffer_bit_set          = "VUID-vkCmdDrawIndexedIndirect-None-08114";
        descriptor_buffer_bit_not_set      = "VUID-vkCmdDrawIndexedIndirect-None-08115";
        descriptor_buffer_set_offset_missing = "VUID-vkCmdDrawIndexedIndirect-None-08117";
        image_view_dim                     = "VUID-vkCmdDrawIndexedIndirect-viewType-07752";
        image_view_numeric_format          = "VUID-vkCmdDrawIndexedIndirect-format-07753";
    }
};

struct DispatchVuidsCmdDispatch : DrawDispatchVuid {
    DispatchVuidsCmdDispatch() : DrawDispatchVuid() {
        pipeline_bound                     = "VUID-vkCmdDispatch-None-02700";
        dynamic_state                      = "VUID-vkCmdDispatch-commandBuffer-02701";
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
        dynamic_state                      = "VUID-vkCmdDispatchIndirect-commandBuffer-02701";
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
        dynamic_state                      = "VUID-vkCmdDrawIndirectCount-commandBuffer-02701";
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
        primitive_topology                 = "VUID-vkCmdDrawIndirectCount-primitiveTopology-03420";
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
        dynamic_color_blend_enable         = "VUID-vkCmdDrawIndirectCount-None-07627";
        dynamic_color_blend_equation       = "VUID-vkCmdDrawIndirectCount-None-07628";
        dynamic_color_write_mask           = "VUID-vkCmdDrawIndirectCount-None-07629";
        dynamic_rasterization_stream       = "VUID-vkCmdDrawIndirectCount-None-07630";
        dynamic_conservative_rasterization_mode = "VUID-vkCmdDrawIndirectCount-None-07631";
        dynamic_extra_primitive_overestimation_size = "VUID-vkCmdDrawIndirectCount-None-07632";
        dynamic_depth_clip_enable          = "VUID-vkCmdDrawIndirectCount-None-07633";
        dynamic_sample_locations_enable    = "VUID-vkCmdDrawIndirectCount-None-07634";
        dynamic_color_blend_advanced       = "VUID-vkCmdDrawIndirectCount-None-07635";
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
        primitives_generated               = "VUID-vkCmdDrawIndirectCount-primitivesGeneratedQueryWithRasterizerDiscard-06708";
        primitives_generated_streams       = "VUID-vkCmdDrawIndirectCount-primitivesGeneratedQueryWithNonZeroStreams-06709";
        invalid_mesh_shader_stages         = "VUID-vkCmdDrawIndirectCount-stage-06481";
        descriptor_buffer_bit_set          = "VUID-vkCmdDrawIndirectCount-None-08114";
        descriptor_buffer_bit_not_set      = "VUID-vkCmdDrawIndirectCount-None-08115";
        descriptor_buffer_set_offset_missing = "VUID-vkCmdDrawIndirectCount-None-08117";
        image_view_dim                     = "VUID-vkCmdDrawIndirectCount-viewType-07752";
        image_view_numeric_format          = "VUID-vkCmdDrawIndirectCount-format-07753";
    }
};

struct DispatchVuidsCmdDrawIndexedIndirectCount : DrawDispatchVuid {
    DispatchVuidsCmdDrawIndexedIndirectCount() : DrawDispatchVuid() {
        pipeline_bound                     = "VUID-vkCmdDrawIndexedIndirectCount-None-02700";
        dynamic_state                      = "VUID-vkCmdDrawIndexedIndirectCount-commandBuffer-02701";
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
        primitive_topology                 = "VUID-vkCmdDrawIndexedIndirectCount-primitiveTopology-03420";
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
        dynamic_color_blend_enable         = "VUID-vkCmdDrawIndexedIndirectCount-None-07627";
        dynamic_color_blend_equation       = "VUID-vkCmdDrawIndexedIndirectCount-None-07628";
        dynamic_color_write_mask           = "VUID-vkCmdDrawIndexedIndirectCount-None-07629";
        dynamic_rasterization_stream       = "VUID-vkCmdDrawIndexedIndirectCount-None-07630";
        dynamic_conservative_rasterization_mode = "VUID-vkCmdDrawIndexedIndirectCount-None-07631";
        dynamic_extra_primitive_overestimation_size = "VUID-vkCmdDrawIndexedIndirectCount-None-07632";
        dynamic_depth_clip_enable          = "VUID-vkCmdDrawIndexedIndirectCount-None-07633";
        dynamic_sample_locations_enable    = "VUID-vkCmdDrawIndexedIndirectCount-None-07634";
        dynamic_color_blend_advanced       = "VUID-vkCmdDrawIndexedIndirectCount-None-07635";
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
        primitives_generated               = "VUID-vkCmdDrawIndexedIndirectCount-primitivesGeneratedQueryWithRasterizerDiscard-06708";
        primitives_generated_streams       = "VUID-vkCmdDrawIndexedIndirectCount-primitivesGeneratedQueryWithNonZeroStreams-06709";
        invalid_mesh_shader_stages         = "VUID-vkCmdDrawIndexedIndirectCount-stage-06481";
        descriptor_buffer_bit_set          = "VUID-vkCmdDrawIndexedIndirectCount-None-08114";
        descriptor_buffer_bit_not_set      = "VUID-vkCmdDrawIndexedIndirectCount-None-08115";
        descriptor_buffer_set_offset_missing = "VUID-vkCmdDrawIndexedIndirectCount-None-08117";
        image_view_dim                     = "VUID-vkCmdDrawIndexedIndirectCount-viewType-07752";
        image_view_numeric_format          = "VUID-vkCmdDrawIndexedIndirectCount-format-07753";
    }
};

struct DispatchVuidsCmdTraceRaysNV: DrawDispatchVuid {
    DispatchVuidsCmdTraceRaysNV() : DrawDispatchVuid() {
        pipeline_bound                     = "VUID-vkCmdTraceRaysNV-None-02700";
        dynamic_state                      = "VUID-vkCmdTraceRaysNV-commandBuffer-02701";
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
        dynamic_state                      = "VUID-vkCmdTraceRaysKHR-commandBuffer-02701";
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
        dynamic_state                      = "VUID-vkCmdTraceRaysIndirectKHR-commandBuffer-02701";
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
        dynamic_state                      = "VUID-vkCmdTraceRaysIndirect2KHR-commandBuffer-02701";
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
        dynamic_state                      = "VUID-vkCmdDrawMeshTasksNV-commandBuffer-02701";
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
        dynamic_color_blend_enable         = "VUID-vkCmdDrawMeshTasksNV-None-07627";
        dynamic_color_blend_equation       = "VUID-vkCmdDrawMeshTasksNV-None-07628";
        dynamic_color_write_mask           = "VUID-vkCmdDrawMeshTasksNV-None-07629";
        dynamic_rasterization_stream       = "VUID-vkCmdDrawMeshTasksNV-None-07630";
        dynamic_conservative_rasterization_mode = "VUID-vkCmdDrawMeshTasksNV-None-07631";
        dynamic_extra_primitive_overestimation_size = "VUID-vkCmdDrawMeshTasksNV-None-07632";
        dynamic_depth_clip_enable          = "VUID-vkCmdDrawMeshTasksNV-None-07633";
        dynamic_sample_locations_enable    = "VUID-vkCmdDrawMeshTasksNV-None-07634";
        dynamic_color_blend_advanced       = "VUID-vkCmdDrawMeshTasksNV-None-07635";
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
        primitives_generated               = "VUID-vkCmdDrawMeshTasksNV-primitivesGeneratedQueryWithRasterizerDiscard-06708";
        primitives_generated_streams       = "VUID-vkCmdDrawMeshTasksNV-primitivesGeneratedQueryWithNonZeroStreams-06709";
        mesh_shader_stages                 = "VUID-vkCmdDrawMeshTasksNV-stage-06480";
        missing_mesh_shader_stages         = "VUID-vkCmdDrawMeshTasksNV-MeshNV-07080";
        descriptor_buffer_bit_set          = "VUID-vkCmdDrawMeshTasksNV-None-08114";
        descriptor_buffer_bit_not_set      = "VUID-vkCmdDrawMeshTasksNV-None-08115";
        descriptor_buffer_set_offset_missing = "VUID-vkCmdDrawMeshTasksNV-None-08117";
        image_view_dim                     = "VUID-vkCmdDrawMeshTasksNV-viewType-07752";
        image_view_numeric_format          = "VUID-vkCmdDrawMeshTasksNV-format-07753";
    }
};

struct DispatchVuidsCmdDrawMeshTasksIndirectNV: DrawDispatchVuid {
    DispatchVuidsCmdDrawMeshTasksIndirectNV() : DrawDispatchVuid() {
        pipeline_bound                     = "VUID-vkCmdDrawMeshTasksIndirectNV-None-02700";
        dynamic_state                      = "VUID-vkCmdDrawMeshTasksIndirectNV-commandBuffer-02701";
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
        dynamic_color_blend_enable         = "VUID-vkCmdDrawMeshTasksIndirectNV-None-07627";
        dynamic_color_blend_equation       = "VUID-vkCmdDrawMeshTasksIndirectNV-None-07628";
        dynamic_color_write_mask           = "VUID-vkCmdDrawMeshTasksIndirectNV-None-07629";
        dynamic_rasterization_stream       = "VUID-vkCmdDrawMeshTasksIndirectNV-None-07630";
        dynamic_conservative_rasterization_mode = "VUID-vkCmdDrawMeshTasksIndirectNV-None-07631";
        dynamic_extra_primitive_overestimation_size = "VUID-vkCmdDrawMeshTasksIndirectNV-None-07632";
        dynamic_depth_clip_enable          = "VUID-vkCmdDrawMeshTasksIndirectNV-None-07633";
        dynamic_sample_locations_enable    = "VUID-vkCmdDrawMeshTasksIndirectNV-None-07634";
        dynamic_color_blend_advanced       = "VUID-vkCmdDrawMeshTasksIndirectNV-None-07635";
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
        primitives_generated               = "VUID-vkCmdDrawMeshTasksIndirectNV-primitivesGeneratedQueryWithRasterizerDiscard-06708";
        primitives_generated_streams       = "VUID-vkCmdDrawMeshTasksIndirectNV-primitivesGeneratedQueryWithNonZeroStreams-06709";
        mesh_shader_stages                 = "VUID-vkCmdDrawMeshTasksIndirectNV-stage-06480";
        missing_mesh_shader_stages         = "VUID-vkCmdDrawMeshTasksIndirectNV-MeshNV-07081";
        descriptor_buffer_bit_set          = "VUID-vkCmdDrawMeshTasksIndirectNV-None-08114";
        descriptor_buffer_bit_not_set      = "VUID-vkCmdDrawMeshTasksIndirectNV-None-08115";
        descriptor_buffer_set_offset_missing = "VUID-vkCmdDrawMeshTasksIndirectNV-None-08117";
        image_view_dim                     = "VUID-vkCmdDrawMeshTasksIndirectNV-viewType-07752";
        image_view_numeric_format          = "VUID-vkCmdDrawMeshTasksIndirectNV-format-07753";
    }
};

struct DispatchVuidsCmdDrawMeshTasksIndirectCountNV : DrawDispatchVuid {
    DispatchVuidsCmdDrawMeshTasksIndirectCountNV() : DrawDispatchVuid() {
        pipeline_bound                     = "VUID-vkCmdDrawMeshTasksIndirectCountNV-None-02700";
        dynamic_state                      = "VUID-vkCmdDrawMeshTasksIndirectCountNV-commandBuffer-02701";
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
        dynamic_color_blend_enable         = "VUID-vkCmdDrawMeshTasksIndirectCountNV-None-07627";
        dynamic_color_blend_equation       = "VUID-vkCmdDrawMeshTasksIndirectCountNV-None-07628";
        dynamic_color_write_mask           = "VUID-vkCmdDrawMeshTasksIndirectCountNV-None-07629";
        dynamic_rasterization_stream       = "VUID-vkCmdDrawMeshTasksIndirectCountNV-None-07630";
        dynamic_conservative_rasterization_mode = "VUID-vkCmdDrawMeshTasksIndirectCountNV-None-07631";
        dynamic_extra_primitive_overestimation_size = "VUID-vkCmdDrawMeshTasksIndirectCountNV-None-07632";
        dynamic_depth_clip_enable          = "VUID-vkCmdDrawMeshTasksIndirectCountNV-None-07633";
        dynamic_sample_locations_enable    = "VUID-vkCmdDrawMeshTasksIndirectCountNV-None-07634";
        dynamic_color_blend_advanced       = "VUID-vkCmdDrawMeshTasksIndirectCountNV-None-07635";
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
        primitives_generated               = "VUID-vkCmdDrawMeshTasksIndirectCountNV-primitivesGeneratedQueryWithRasterizerDiscard-06708";
        primitives_generated_streams       = "VUID-vkCmdDrawMeshTasksIndirectCountNV-primitivesGeneratedQueryWithNonZeroStreams-06709";
        mesh_shader_stages                 = "VUID-vkCmdDrawMeshTasksIndirectCountNV-stage-06480";
        missing_mesh_shader_stages         = "VUID-vkCmdDrawMeshTasksIndirectCountNV-MeshNV-07082";
        descriptor_buffer_bit_set          = "VUID-vkCmdDrawMeshTasksIndirectCountNV-None-08114";
        descriptor_buffer_bit_not_set      = "VUID-vkCmdDrawMeshTasksIndirectCountNV-None-08115";
        descriptor_buffer_set_offset_missing = "VUID-vkCmdDrawMeshTasksIndirectCountNV-None-08117";
        image_view_dim                     = "VUID-vkCmdDrawMeshTasksIndirectCountNV-viewType-07752";
        image_view_numeric_format          = "VUID-vkCmdDrawMeshTasksIndirectCountNV-format-07753";
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
        dynamic_state                      = "VUID-vkCmdDrawIndirectByteCountEXT-commandBuffer-02701";
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
        primitive_topology                 = "VUID-vkCmdDrawIndirectByteCountEXT-primitiveTopology-03420";
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
        dynamic_color_blend_enable         = "VUID-vkCmdDrawIndirectByteCountEXT-None-07627";
        dynamic_color_blend_equation       = "VUID-vkCmdDrawIndirectByteCountEXT-None-07628";
        dynamic_color_write_mask           = "VUID-vkCmdDrawIndirectByteCountEXT-None-07629";
        dynamic_rasterization_stream       = "VUID-vkCmdDrawIndirectByteCountEXT-None-07630";
        dynamic_conservative_rasterization_mode = "VUID-vkCmdDrawIndirectByteCountEXT-None-07631";
        dynamic_extra_primitive_overestimation_size = "VUID-vkCmdDrawIndirectByteCountEXT-None-07632";
        dynamic_depth_clip_enable          = "VUID-vkCmdDrawIndirectByteCountEXT-None-07633";
        dynamic_sample_locations_enable    = "VUID-vkCmdDrawIndirectByteCountEXT-None-07634";
        dynamic_color_blend_advanced       = "VUID-vkCmdDrawIndirectByteCountEXT-None-07635";
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
        primitives_generated               = "VUID-vkCmdDrawIndirectByteCountEXT-primitivesGeneratedQueryWithRasterizerDiscard-06708";
        primitives_generated_streams       = "VUID-vkCmdDrawIndirectByteCountEXT-primitivesGeneratedQueryWithNonZeroStreams-06709";
        invalid_mesh_shader_stages         = "VUID-vkCmdDrawIndirectByteCountEXT-stage-06481";
        descriptor_buffer_bit_set          = "VUID-vkCmdDrawIndirectByteCountEXT-None-08114";
        descriptor_buffer_bit_not_set      = "VUID-vkCmdDrawIndirectByteCountEXT-None-08115";
        descriptor_buffer_set_offset_missing = "VUID-vkCmdDrawIndirectByteCountEXT-None-08117";
        image_view_dim                     = "VUID-vkCmdDrawIndirectByteCountEXT-viewType-07752";
        image_view_numeric_format          = "VUID-vkCmdDrawIndirectByteCountEXT-format-07753";
    }
};

struct DispatchVuidsCmdDispatchBase: DrawDispatchVuid {
    DispatchVuidsCmdDispatchBase() : DrawDispatchVuid() {
        pipeline_bound                     = "VUID-vkCmdDispatchBase-None-02700";
        dynamic_state                      = "VUID-vkCmdDispatchBase-commandBuffer-02701";
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
                         "CmdDrawIndirect: drawCount equals 1 and (offset + sizeof(VkDrawIndirectCommand)) (%" PRIu64 ") is not less than "
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
        skip |= LogError(
            commandBuffer, "VUID-vkCmdDrawIndexedIndirect-drawCount-00539",
            "CmdDrawIndexedIndirect: drawCount equals 1 and (offset + sizeof(VkDrawIndexedIndirectCommand)) (%" PRIu64 ") is not less than "
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

bool CoreChecks::ValidateRaytracingShaderBindingTable(VkCommandBuffer commandBuffer, const char *rt_func_name,
                                                      const char *vuid_single_device_memory, const char *vuid_binding_table_flag,
                                                      const VkStridedDeviceAddressRegionKHR &binding_table,
                                                      const char *binding_table_name) const {
    bool skip = false;

    if (binding_table.deviceAddress == 0) {
        return skip;
    }

    const auto buffer_states = GetBuffersByAddress(binding_table.deviceAddress);
    if (buffer_states.empty()) {
        skip |= LogError(device, "VUID-VkStridedDeviceAddressRegionKHR-size-04631",
                         "%s: no buffer is associated with %s->deviceAddress (0x%" PRIx64 ").", rt_func_name, binding_table_name,
                         binding_table.deviceAddress);
    } else {
        // Try to find a buffer satisfying all VUIDs
        const bool no_valid_buffer_found = std::none_of(
            buffer_states.begin(), buffer_states.end(),
            [&binding_table](const ValidationStateTracker::BUFFER_STATE_PTR &buffer_state) {
                assert(buffer_state);

                if (!buffer_state) {
                    return false;
                }

                if (!(static_cast<uint32_t>(buffer_state->createInfo.usage) & VK_BUFFER_USAGE_SHADER_BINDING_TABLE_BIT_KHR)) {
                    return false;
                }
                if (!buffer_state->sparse) {
                    if (const auto mem_state = buffer_state->MemState(); !mem_state || mem_state->Destroyed()) {
                        return false;
                    }
                }
                if (binding_table.size != 0) {
                    const auto device_address_range = buffer_state->DeviceAddressRange();
                    const sparse_container::range<VkDeviceSize> requested_range(
                        binding_table.deviceAddress, binding_table.deviceAddress + binding_table.size - 1);
                    if (!device_address_range.includes(requested_range)) {
                        return false;
                    }

                    if (binding_table.size != 0 && binding_table.stride > buffer_state->createInfo.size) {
                        return false;
                    }
                }

                return true;
            });

        // If no valid buffer was found, for each violated VUID,
        // output the list of buffers (associated to binding_table.deviceAddress) that violates it,
        // alongside relevant info.
        if (no_valid_buffer_found) {
            struct InvalidBuffers {
                LogObjectList buffers;
                std::string error_msg;
            };

            InvalidBuffers vuid_binding_table_flag_invalid_buffers{{commandBuffer}};
            InvalidBuffers vuid_address_range_invalid_buffers{{commandBuffer}};
            InvalidBuffers vuid_stride_invalid_buffers{{commandBuffer}};
            const std::string address_string = std::to_string(binding_table.deviceAddress);
            const sparse_container::range<VkDeviceSize> requested_range(binding_table.deviceAddress,
                                                                        binding_table.deviceAddress + binding_table.size - 1);
            std::string requested_range_string =
                '[' + std::to_string(requested_range.begin) + ", " + std::to_string(requested_range.end) + ')';
            std::stringstream error_msg_prefix_ss;
            error_msg_prefix_ss << rt_func_name << ": No buffer associated to" << binding_table_name << "->deviceAddress (0x"
                                << address_string
                                << ") was found such that valid usage passes. "
                                   "At least one buffer associated to this device address must be valid. The following buffers ";
            const auto error_msg_prefix = error_msg_prefix_ss.str();
            for (const auto &buffer_state : buffer_states) {
                assert(buffer_state);

                if (!buffer_state) {
                    continue;
                }

                skip |= ValidateMemoryIsBoundToBuffer(commandBuffer, *buffer_state, rt_func_name, vuid_single_device_memory);

                if (!(static_cast<uint32_t>(buffer_state->createInfo.usage) & VK_BUFFER_USAGE_SHADER_BINDING_TABLE_BIT_KHR)) {
                    vuid_binding_table_flag_invalid_buffers.buffers.add(buffer_state->Handle());

                    std::string &error_msg = vuid_binding_table_flag_invalid_buffers.error_msg;
                    if (error_msg.empty()) {
                        error_msg += error_msg_prefix +
                                     "have not been created with the VK_BUFFER_USAGE_SHADER_BINDING_TABLE_BIT_KHR usage flag:\n";
                    }
                    // +1 because commandBuffer takes index 0 in the log object list
                    const auto obj_index = vuid_binding_table_flag_invalid_buffers.buffers.object_list.size() - 1 + 1;
                    error_msg += "Object " + std::to_string(obj_index) + ": buffer has usage " +
                                 string_VkBufferUsageFlags(buffer_state->createInfo.usage) + '\n';
                }

                if (binding_table.size != 0) {
                    const auto buffer_address_range = buffer_state->DeviceAddressRange();
                    if (!buffer_address_range.includes(requested_range)) {
                        vuid_address_range_invalid_buffers.buffers.add(buffer_state->Handle());

                        std::string &error_msg = vuid_address_range_invalid_buffers.error_msg;
                        if (error_msg.empty()) {
                            error_msg += error_msg_prefix + "do not include " + binding_table_name +
                                         " buffer device address range " + requested_range_string + ":\n";
                        }
                        const auto obj_index = vuid_address_range_invalid_buffers.buffers.object_list.size() - 1 + 1;
                        const std::string buffer_address_range_string = '[' + std::to_string(buffer_address_range.begin) + ", " +
                                                                        std::to_string(buffer_address_range.end) + ')';
                        error_msg += "Object " + std::to_string(obj_index) + ": buffer device address range is " +
                                     buffer_address_range_string + '\n';
                    }

                    if (binding_table.size != 0 && binding_table.stride > buffer_state->createInfo.size) {
                        vuid_stride_invalid_buffers.buffers.add(buffer_state->Handle());

                        std::string &error_msg = vuid_stride_invalid_buffers.error_msg;
                        if (error_msg.empty()) {
                            error_msg += error_msg_prefix + "have a size inferior to " + binding_table_name + "->stride (" +
                                         std::to_string(binding_table.stride) + "):\n";
                        }
                        const auto obj_index = vuid_stride_invalid_buffers.buffers.object_list.size() - 1 + 1;
                        error_msg += "Object " + std::to_string(obj_index) + ": buffer size is " +
                                     std::to_string(buffer_state->createInfo.size) + '\n';
                    }
                }
            }

            if (!vuid_binding_table_flag_invalid_buffers.error_msg.empty()) {
                skip |= LogError(vuid_binding_table_flag_invalid_buffers.buffers, vuid_binding_table_flag, "%s",
                                 vuid_binding_table_flag_invalid_buffers.error_msg.c_str());
            }

            if (!vuid_address_range_invalid_buffers.error_msg.empty()) {
                skip |= LogError(vuid_address_range_invalid_buffers.buffers, "VUID-VkStridedDeviceAddressRegionKHR-size-04631",
                                 "%s", vuid_address_range_invalid_buffers.error_msg.c_str());
            }

            if (!vuid_stride_invalid_buffers.error_msg.empty()) {
                skip |= LogError(vuid_stride_invalid_buffers.buffers, "VUID-VkStridedDeviceAddressRegionKHR-size-04632", "%s",
                                 vuid_stride_invalid_buffers.error_msg.c_str());
            }
        }
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
