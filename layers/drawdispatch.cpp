/* Copyright (c) 2015-2021 The Khronos Group Inc.
 * Copyright (c) 2015-2021 Valve Corporation
 * Copyright (c) 2015-2021 LunarG, Inc.
 * Copyright (C) 2015-2021 Google Inc.
 * Modifications Copyright (C) 2020-2021 Advanced Micro Devices, Inc. All rights reserved.
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
        linear_sampler                     = "VUID-vkCmdDraw-magFilter-04553";
        cubic_sampler                      = "VUID-vkCmdDraw-None-02692";
        viewport_count                     = "VUID-vkCmdDraw-viewportCount-03417";
        scissor_count                      = "VUID-vkCmdDraw-scissorCount-03418";
        viewport_scissor_count             = "VUID-vkCmdDraw-viewportCount-03419";
        primitive_topology                 = "VUID-vkCmdDraw-primitiveTopology-03420";
        corner_sampled_address_mode        = "VUID-vkCmdDraw-flags-02696";
        subpass_input                      = "VUID-vkCmdDraw-None-02686";
        imageview_atomic                   = "VUID-vkCmdDraw-None-02691";
        push_constants_set                 = "VUID-vkCmdDraw-None-02698";
        image_subresources                 = "VUID-vkCmdDraw-None-04584";
        descriptor_valid                   = "VUID-vkCmdDraw-None-02699";
        sampler_imageview_type             = "VUID-vkCmdDraw-None-02702";
        sampler_implicitLod_dref_proj      = "VUID-vkCmdDraw-None-02703";
        sampler_bias_offset                = "VUID-vkCmdDraw-None-02704";
        vertex_binding_attribute           = "VUID-vkCmdDraw-None-02721";
        dynamic_state_setting_commands     = "VUID-vkCmdDraw-None-02859";
        rasterization_samples              = "VUID-vkCmdDraw-rasterizationSamples-04740";
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
        vertex_input                       = kVUID_Core_CmdDraw_VertexInput;
        blend_enable                       = "VUID-vkCmdDraw-blendEnable-04727";
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
        linear_sampler                     = "VUID-vkCmdDrawMultiEXT-magFilter-04553";
        cubic_sampler                      = "VUID-vkCmdDrawMultiEXT-None-02692";
        viewport_count                     = "VUID-vkCmdDrawMultiEXT-viewportCount-03417";
        scissor_count                      = "VUID-vkCmdDrawMultiEXT-scissorCount-03418";
        viewport_scissor_count             = "VUID-vkCmdDrawMultiEXT-viewportCount-03419";
        primitive_topology                 = "VUID-vkCmdDrawMultiEXT-primitiveTopology-03420";
        corner_sampled_address_mode        = "VUID-vkCmdDrawMultiEXT-flags-02696";
        subpass_input                      = "VUID-vkCmdDrawMultiEXT-None-02686";
        imageview_atomic                   = "VUID-vkCmdDrawMultiEXT-None-02691";
        push_constants_set                 = "VUID-vkCmdDrawMultiEXT-None-02698";
        image_subresources                 = "VUID-vkCmdDrawMultiEXT-None-04584";
        descriptor_valid                   = "VUID-vkCmdDrawMultiEXT-None-02699";
        sampler_imageview_type             = "VUID-vkCmdDrawMultiEXT-None-02702";
        sampler_implicitLod_dref_proj      = "VUID-vkCmdDrawMultiEXT-None-02703";
        sampler_bias_offset                = "VUID-vkCmdDrawMultiEXT-None-02704";
        vertex_binding_attribute           = "VUID-vkCmdDrawMultiEXT-None-02721";
        dynamic_state_setting_commands     = "VUID-vkCmdDrawMultiEXT-None-02859";
        rasterization_samples              = "VUID-vkCmdDrawMultiEXT-rasterizationSamples-04740";
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
        vertex_input                       = kVUID_Core_CmdDrawMultiEXT_VertexInput;
        blend_enable                       = "VUID-vkCmdDrawMultiEXT-blendEnable-04727";
    }
};

struct DispatchVuidsCmdDrawIndexed : DrawDispatchVuid {
    DispatchVuidsCmdDrawIndexed() : DrawDispatchVuid() {
        pipeline_bound                     = "VUID-vkCmdDrawIndexed-None-02700";
        dynamic_state                      = "VUID-vkCmdDrawIndexed-commandBuffer-02701";
        vertex_binding                     = "VUID-vkCmdDrawIndexed-None-04007";
        vertex_binding_null                = "VUID-vkCmdDrawIndexed-None-04008";
        compatible_pipeline                = "VUID-vkCmdDrawIndexed-None-02697";
        render_pass_compatible             = "VUID-vkCmdDrawIndexed-renderPass-02684";
        subpass_index                      = "VUID-vkCmdDrawIndexed-subpass-02685";
        sample_location                    = "VUID-vkCmdDrawIndexed-sampleLocationsEnable-02689";
        linear_sampler                     = "VUID-vkCmdDrawIndexed-magFilter-04553";
        cubic_sampler                      = "VUID-vkCmdDrawIndexed-None-02692";
        viewport_count                     = "VUID-vkCmdDrawIndexed-viewportCount-03417";
        scissor_count                      = "VUID-vkCmdDrawIndexed-scissorCount-03418";
        viewport_scissor_count             = "VUID-vkCmdDrawIndexed-viewportCount-03419";
        primitive_topology                 = "VUID-vkCmdDrawIndexed-primitiveTopology-03420";
        corner_sampled_address_mode        = "VUID-vkCmdDrawIndexed-flags-02696";
        subpass_input                      = "VUID-vkCmdDrawIndexed-None-02686";
        imageview_atomic                   = "VUID-vkCmdDrawIndexed-None-02691";
        push_constants_set                 = "VUID-vkCmdDrawIndexed-None-02698";
        image_subresources                 = "VUID-vkCmdDrawIndexed-None-04584";
        descriptor_valid                   = "VUID-vkCmdDrawIndexed-None-02699";
        sampler_imageview_type             = "VUID-vkCmdDrawIndexed-None-02702";
        sampler_implicitLod_dref_proj      = "VUID-vkCmdDrawIndexed-None-02703";
        sampler_bias_offset                = "VUID-vkCmdDrawIndexed-None-02704";
        vertex_binding_attribute           = "VUID-vkCmdDrawIndexed-None-02721";
        dynamic_state_setting_commands     = "VUID-vkCmdDrawIndexed-None-02859";
        rasterization_samples              = "VUID-vkCmdDrawIndexed-rasterizationSamples-04740";
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
        vertex_input                       = kVUID_Core_CmdDrawIndexed_VertexInput;
        blend_enable                       = "VUID-vkCmdDrawIndexed-blendEnable-04727";
    }
};

struct DispatchVuidsCmdDrawMultiIndexedEXT : DrawDispatchVuid {
    DispatchVuidsCmdDrawMultiIndexedEXT() : DrawDispatchVuid() {
        pipeline_bound = "VUID-vkCmdDrawMultiIndexedEXT-None-02700";
        dynamic_state = "VUID-vkCmdDrawMultiIndexedEXT-commandBuffer-02701";
        vertex_binding = "VUID-vkCmdDrawMultiIndexedEXT-None-04007";
        vertex_binding_null = "VUID-vkCmdDrawMultiIndexedEXT-None-04008";
        compatible_pipeline = "VUID-vkCmdDrawMultiIndexedEXT-None-02697";
        render_pass_compatible = "VUID-vkCmdDrawMultiIndexedEXT-renderPass-02684";
        subpass_index = "VUID-vkCmdDrawMultiIndexedEXT-subpass-02685";
        sample_location = "VUID-vkCmdDrawMultiIndexedEXT-sampleLocationsEnable-02689";
        linear_sampler = "VUID-vkCmdDrawMultiIndexedEXT-magFilter-04553";
        cubic_sampler = "VUID-vkCmdDrawMultiIndexedEXT-None-02692";
        viewport_count = "VUID-vkCmdDrawMultiIndexedEXT-viewportCount-03417";
        scissor_count = "VUID-vkCmdDrawMultiIndexedEXT-scissorCount-03418";
        viewport_scissor_count = "VUID-vkCmdDrawMultiIndexedEXT-viewportCount-03419";
        primitive_topology = "VUID-vkCmdDrawMultiIndexedEXT-primitiveTopology-03420";
        corner_sampled_address_mode = "VUID-vkCmdDrawMultiIndexedEXT-flags-02696";
        subpass_input = "VUID-vkCmdDrawMultiIndexedEXT-None-02686";
        imageview_atomic = "VUID-vkCmdDrawMultiIndexedEXT-None-02691";
        push_constants_set = "VUID-vkCmdDrawMultiIndexedEXT-None-02698";
        image_subresources = "VUID-vkCmdDrawMultiIndexedEXT-None-04584";
        descriptor_valid = "VUID-vkCmdDrawMultiIndexedEXT-None-02699";
        sampler_imageview_type = "VUID-vkCmdDrawMultiIndexedEXT-None-02702";
        sampler_implicitLod_dref_proj = "VUID-vkCmdDrawMultiIndexedEXT-None-02703";
        sampler_bias_offset = "VUID-vkCmdDrawMultiIndexedEXT-None-02704";
        vertex_binding_attribute = "VUID-vkCmdDrawMultiIndexedEXT-None-02721";
        dynamic_state_setting_commands = "VUID-vkCmdDrawMultiIndexedEXT-None-02859";
        rasterization_samples = "VUID-vkCmdDrawMultiIndexedEXT-rasterizationSamples-04740";
        unprotected_command_buffer = "VUID-vkCmdDrawMultiIndexedEXT-commandBuffer-02707";
        protected_command_buffer = "VUID-vkCmdDrawMultiIndexedEXT-commandBuffer-02712";
        max_multiview_instance_index = "VUID-vkCmdDrawMultiIndexedEXT-maxMultiviewInstanceIndex-02688";
        img_filter_cubic = "VUID-vkCmdDrawMultiIndexedEXT-None-02693";
        filter_cubic = "VUID-vkCmdDrawMultiIndexedEXT-filterCubic-02694";
        filter_cubic_min_max = "VUID-vkCmdDrawMultiIndexedEXT-filterCubicMinmax-02695";
        viewport_count_primitive_shading_rate = "VUID-vkCmdDrawMultiIndexedEXT-primitiveFragmentShadingRateWithMultipleViewports-04552";
        patch_control_points = "VUID-vkCmdDrawMultiIndexedEXT-None-04875";
        rasterizer_discard_enable = "VUID-vkCmdDrawMultiIndexedEXT-None-04876";
        depth_bias_enable = "VUID-vkCmdDrawMultiIndexedEXT-None-04877";
        logic_op = "VUID-vkCmdDrawMultiIndexedEXT-logicOp-04878";
        primitive_restart_enable = "VUID-vkCmdDrawMultiIndexedEXT-None-04879";
        vertex_input_binding_stride = "VUID-vkCmdDrawMultiIndexedEXT-pStrides-04884";
        vertex_input = kVUID_Core_CmdDrawMultiIndexedEXT_VertexInput;
        blend_enable = "VUID-vkCmdDrawMultiIndexedEXT-blendEnable-04727";
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
        linear_sampler                     = "VUID-vkCmdDrawIndirect-magFilter-04553";
        cubic_sampler                      = "VUID-vkCmdDrawIndirect-None-02692";
        indirect_protected_cb              = "VUID-vkCmdDrawIndirect-commandBuffer-02711";
        indirect_contiguous_memory         = "VUID-vkCmdDrawIndirect-buffer-02708";
        indirect_buffer_bit                = "VUID-vkCmdDrawIndirect-buffer-02709";
        viewport_count                     = "VUID-vkCmdDrawIndirect-viewportCount-03417";
        scissor_count                      = "VUID-vkCmdDrawIndirect-scissorCount-03418";
        viewport_scissor_count             = "VUID-vkCmdDrawIndirect-viewportCount-03419";
        primitive_topology                 = "VUID-vkCmdDrawIndirect-primitiveTopology-03420";
        corner_sampled_address_mode        = "VUID-vkCmdDrawIndirect-flags-02696";
        subpass_input                      = "VUID-vkCmdDrawIndirect-None-02686";
        imageview_atomic                   = "VUID-vkCmdDrawIndirect-None-02691";
        push_constants_set                 = "VUID-vkCmdDrawIndirect-None-02698";
        image_subresources                 = "VUID-vkCmdDrawIndirect-None-04584";
        descriptor_valid                   = "VUID-vkCmdDrawIndirect-None-02699";
        sampler_imageview_type             = "VUID-vkCmdDrawIndirect-None-02702";
        sampler_implicitLod_dref_proj      = "VUID-vkCmdDrawIndirect-None-02703";
        sampler_bias_offset                = "VUID-vkCmdDrawIndirect-None-02704";
        vertex_binding_attribute           = "VUID-vkCmdDrawIndirect-None-02721";
        dynamic_state_setting_commands     = "VUID-vkCmdDrawIndirect-None-02859";
        rasterization_samples              = "VUID-vkCmdDrawIndirect-rasterizationSamples-04740";
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
        vertex_input                       = kVUID_Core_CmdDrawIndirect_VertexInput;
        blend_enable                       = "VUID-vkCmdDrawIndirect-blendEnable-04727";
    }
};

struct DispatchVuidsCmdDrawIndexedIndirect : DrawDispatchVuid {
    DispatchVuidsCmdDrawIndexedIndirect() : DrawDispatchVuid() {
        pipeline_bound                     = "VUID-vkCmdDrawIndexedIndirect-None-02700";
        dynamic_state                      = "VUID-vkCmdDrawIndexedIndirect-commandBuffer-02701";
        vertex_binding                     = "VUID-vkCmdDrawIndexedIndirect-None-04007";
        vertex_binding_null                = "VUID-vkCmdDrawIndexedIndirect-None-04008";
        compatible_pipeline                = "VUID-vkCmdDrawIndexedIndirect-None-02697";
        render_pass_compatible             = "VUID-vkCmdDrawIndexedIndirect-renderPass-02684";
        subpass_index                      = "VUID-vkCmdDrawIndexedIndirect-subpass-02685";
        sample_location                    = "VUID-vkCmdDrawIndexedIndirect-sampleLocationsEnable-02689";
        linear_sampler                     = "VUID-vkCmdDrawIndexedIndirect-magFilter-04553";
        cubic_sampler                      = "VUID-vkCmdDrawIndexedIndirect-None-02692";
        indirect_protected_cb              = "VUID-vkCmdDrawIndexedIndirect-commandBuffer-02711";
        indirect_contiguous_memory         = "VUID-vkCmdDrawIndexedIndirect-buffer-02708";
        indirect_buffer_bit                = "VUID-vkCmdDrawIndexedIndirect-buffer-02709";
        viewport_count                     = "VUID-vkCmdDrawIndexedIndirect-viewportCount-03417";
        scissor_count                      = "VUID-vkCmdDrawIndexedIndirect-scissorCount-03418";
        viewport_scissor_count             = "VUID-vkCmdDrawIndexedIndirect-viewportCount-03419";
        primitive_topology                 = "VUID-vkCmdDrawIndexedIndirect-primitiveTopology-03420";
        corner_sampled_address_mode        = "VUID-vkCmdDrawIndexedIndirect-flags-02696";
        subpass_input                      = "VUID-vkCmdDrawIndexedIndirect-None-02686";
        imageview_atomic                   = "VUID-vkCmdDrawIndexedIndirect-None-02691";
        push_constants_set                 = "VUID-vkCmdDrawIndexedIndirect-None-02698";
        image_subresources                 = "VUID-vkCmdDrawIndexedIndirect-None-04584";
        descriptor_valid                   = "VUID-vkCmdDrawIndexedIndirect-None-02699";
        sampler_imageview_type             = "VUID-vkCmdDrawIndexedIndirect-None-02702";
        sampler_implicitLod_dref_proj      = "VUID-vkCmdDrawIndexedIndirect-None-02703";
        sampler_bias_offset                = "VUID-vkCmdDrawIndexedIndirect-None-02704";
        vertex_binding_attribute           = "VUID-vkCmdDrawIndexedIndirect-None-02721";
        dynamic_state_setting_commands     = "VUID-vkCmdDrawIndexedIndirect-None-02859";
        rasterization_samples              = "VUID-vkCmdDrawIndexedIndirect-rasterizationSamples-04740";
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
        vertex_input                       = kVUID_Core_CmdDrawIndexedIndirect_VertexInput;
        blend_enable                       = "VUID-vkCmdDrawIndexedIndirect-blendEnable-04727";
    }
};

struct DispatchVuidsCmdDispatch : DrawDispatchVuid {
    DispatchVuidsCmdDispatch() : DrawDispatchVuid() {
        pipeline_bound                     = "VUID-vkCmdDispatch-None-02700";
        dynamic_state                      = "VUID-vkCmdDispatch-commandBuffer-02701";
        compatible_pipeline                = "VUID-vkCmdDispatch-None-02697";
        linear_sampler                     = "VUID-vkCmdDispatch-magFilter-04553";
        cubic_sampler                      = "VUID-vkCmdDispatch-None-02692";
        corner_sampled_address_mode        = "VUID-vkCmdDispatch-flags-02696";
        imageview_atomic                   = "VUID-vkCmdDispatch-None-02691";
        push_constants_set                 = "VUID-vkCmdDispatch-None-02698";
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
    }
};

struct DispatchVuidsCmdDispatchIndirect : DrawDispatchVuid {
    DispatchVuidsCmdDispatchIndirect() : DrawDispatchVuid() {
        pipeline_bound                     = "VUID-vkCmdDispatchIndirect-None-02700";
        dynamic_state                      = "VUID-vkCmdDispatchIndirect-commandBuffer-02701";
        compatible_pipeline                = "VUID-vkCmdDispatchIndirect-None-02697";
        linear_sampler                     = "VUID-vkCmdDispatchIndirect-magFilter-04553";
        cubic_sampler                      = "VUID-vkCmdDispatchIndirect-None-02692";
        indirect_protected_cb              = "VUID-vkCmdDispatchIndirect-commandBuffer-02711";
        indirect_contiguous_memory         = "VUID-vkCmdDispatchIndirect-buffer-02708";
        indirect_buffer_bit                = "VUID-vkCmdDispatchIndirect-buffer-02709";
        corner_sampled_address_mode        = "VUID-vkCmdDispatchIndirect-flags-02696";
        imageview_atomic                   = "VUID-vkCmdDispatchIndirect-None-02691";
        push_constants_set                 = "VUID-vkCmdDispatchIndirect-None-02698";
        descriptor_valid                   = "VUID-vkCmdDispatchIndirect-None-02699";
        sampler_imageview_type             = "VUID-vkCmdDispatchIndirect-None-02702";
        sampler_implicitLod_dref_proj      = "VUID-vkCmdDispatchIndirect-None-02703";
        sampler_bias_offset                = "VUID-vkCmdDispatchIndirect-None-02704";
        dynamic_state_setting_commands     = "VUID-vkCmdDispatchIndirect-None-02859";
        unprotected_command_buffer         = "VUID-vkCmdDispatchIndirect-commandBuffer-02707";
        img_filter_cubic                   = "VUID-vkCmdDispatchIndirect-None-02693";
        filter_cubic                       = "VUID-vkCmdDispatchIndirect-filterCubic-02694";
        filter_cubic_min_max               = "VUID-vkCmdDispatchIndirect-filterCubicMinmax-02695";
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
        linear_sampler                     = "VUID-vkCmdDrawIndirectCount-magFilter-04553";
        cubic_sampler                      = "VUID-vkCmdDrawIndirectCount-None-02692";
        indirect_protected_cb              = "VUID-vkCmdDrawIndirectCount-commandBuffer-02711";
        indirect_contiguous_memory         = "VUID-vkCmdDrawIndirectCount-buffer-02708";
        indirect_buffer_bit                = "VUID-vkCmdDrawIndirectCount-buffer-02709";
        viewport_count                     = "VUID-vkCmdDrawIndirectCount-viewportCount-03417";
        scissor_count                      = "VUID-vkCmdDrawIndirectCount-scissorCount-03418";
        viewport_scissor_count             = "VUID-vkCmdDrawIndirectCount-viewportCount-03419";
        primitive_topology                 = "VUID-vkCmdDrawIndirectCount-primitiveTopology-03420";
        corner_sampled_address_mode        = "VUID-vkCmdDrawIndirectCount-flags-02696";
        subpass_input                      = "VUID-vkCmdDrawIndirectCount-None-02686";
        imageview_atomic                   = "VUID-vkCmdDrawIndirectCount-None-02691";
        push_constants_set                 = "VUID-vkCmdDrawIndirectCount-None-02698";
        image_subresources                 = "VUID-vkCmdDrawIndirectCount-None-04584";
        descriptor_valid                   = "VUID-vkCmdDrawIndirectCount-None-02699";
        sampler_imageview_type             = "VUID-vkCmdDrawIndirectCount-None-02702";
        sampler_implicitLod_dref_proj      = "VUID-vkCmdDrawIndirectCount-None-02703";
        sampler_bias_offset                = "VUID-vkCmdDrawIndirectCount-None-02704";
        vertex_binding_attribute           = "VUID-vkCmdDrawIndirectCount-None-02721";
        dynamic_state_setting_commands     = "VUID-vkCmdDrawIndirectCount-None-02859";
        rasterization_samples              = "VUID-vkCmdDrawIndirectCount-rasterizationSamples-04740";
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
        vertex_input                       = kVUID_Core_CmdDrawIndirectCount_VertexInput;
        blend_enable                       = "VUID-vkCmdDrawIndirectCount-blendEnable-04727";
    }
};

struct DispatchVuidsCmdDrawIndexedIndirectCount : DrawDispatchVuid {
    DispatchVuidsCmdDrawIndexedIndirectCount() : DrawDispatchVuid() {
        pipeline_bound                     = "VUID-vkCmdDrawIndexedIndirectCount-None-02700";
        dynamic_state                      = "VUID-vkCmdDrawIndexedIndirectCount-commandBuffer-02701";
        vertex_binding                     = "VUID-vkCmdDrawIndexedIndirectCount-None-04007";
        vertex_binding_null                = "VUID-vkCmdDrawIndexedIndirectCount-None-04008";
        compatible_pipeline                = "VUID-vkCmdDrawIndexedIndirectCount-None-02697";
        render_pass_compatible             = "VUID-vkCmdDrawIndexedIndirectCount-renderPass-02684";
        subpass_index                      = "VUID-vkCmdDrawIndexedIndirectCount-subpass-02685";
        sample_location                    = "VUID-vkCmdDrawIndexedIndirectCount-sampleLocationsEnable-02689";
        linear_sampler                     = "VUID-vkCmdDrawIndexedIndirectCount-magFilter-04553";
        cubic_sampler                      = "VUID-vkCmdDrawIndexedIndirectCount-None-02692";
        indirect_protected_cb              = "VUID-vkCmdDrawIndexedIndirectCount-commandBuffer-02711";
        indirect_contiguous_memory         = "VUID-vkCmdDrawIndexedIndirectCount-buffer-02708";
        indirect_buffer_bit                = "VUID-vkCmdDrawIndexedIndirectCount-buffer-02709";
        viewport_count                     = "VUID-vkCmdDrawIndexedIndirectCount-viewportCount-03417";
        scissor_count                      = "VUID-vkCmdDrawIndexedIndirectCount-scissorCount-03418";
        viewport_scissor_count             = "VUID-vkCmdDrawIndexedIndirectCount-viewportCount-03419";
        primitive_topology                 = "VUID-vkCmdDrawIndexedIndirectCount-primitiveTopology-03420";
        corner_sampled_address_mode        = "VUID-vkCmdDrawIndexedIndirectCount-flags-02696";
        subpass_input                      = "VUID-vkCmdDrawIndexedIndirectCount-None-02686";
        imageview_atomic                   = "VUID-vkCmdDrawIndexedIndirectCount-None-02691";
        push_constants_set                 = "VUID-vkCmdDrawIndexedIndirectCount-None-02698";
        image_subresources                 = "VUID-vkCmdDrawIndexedIndirectCount-None-04584";
        descriptor_valid                   = "VUID-vkCmdDrawIndexedIndirectCount-None-02699";
        sampler_imageview_type             = "VUID-vkCmdDrawIndexedIndirectCount-None-02702";
        sampler_implicitLod_dref_proj      = "VUID-vkCmdDrawIndexedIndirectCount-None-02703";
        sampler_bias_offset                = "VUID-vkCmdDrawIndexedIndirectCount-None-02704";
        vertex_binding_attribute           = "VUID-vkCmdDrawIndexedIndirectCount-None-02721";
        dynamic_state_setting_commands     = "VUID-vkCmdDrawIndexedIndirectCount-None-02859";
        rasterization_samples              = "VUID-vkCmdDrawIndexedIndirectCount-rasterizationSamples-04740";
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
        vertex_input                       = kVUID_Core_CmdDrawIndexedIndirectCount_VertexInput;
        blend_enable                       = "VUID-vkCmdDrawIndexedIndirectCount-blendEnable-04727";
    }
};

struct DispatchVuidsCmdTraceRaysNV: DrawDispatchVuid {
    DispatchVuidsCmdTraceRaysNV() : DrawDispatchVuid() {
        pipeline_bound                     = "VUID-vkCmdTraceRaysNV-None-02700";
        dynamic_state                      = "VUID-vkCmdTraceRaysNV-commandBuffer-02701";
        compatible_pipeline                = "VUID-vkCmdTraceRaysNV-None-02697";
        linear_sampler                     = "VUID-vkCmdTraceRaysNV-magFilter-04553";
        cubic_sampler                      = "VUID-vkCmdTraceRaysNV-None-02692";
        corner_sampled_address_mode        = "VUID-vkCmdTraceRaysNV-flags-02696";
        imageview_atomic                   = "VUID-vkCmdTraceRaysNV-None-02691";
        push_constants_set                 = "VUID-vkCmdTraceRaysNV-None-02698";
        descriptor_valid                   = "VUID-vkCmdTraceRaysNV-None-02699";
        sampler_imageview_type             = "VUID-vkCmdTraceRaysNV-None-02702";
        sampler_implicitLod_dref_proj      = "VUID-vkCmdTraceRaysNV-None-02703";
        sampler_bias_offset                = "VUID-vkCmdTraceRaysNV-None-02704";
        dynamic_state_setting_commands     = "VUID-vkCmdTraceRaysNV-None-02859";
        unprotected_command_buffer         = "VUID-vkCmdTraceRaysNV-commandBuffer-02707";
        protected_command_buffer           = "VUID-vkCmdTraceRaysNV-commandBuffer-04624";
        img_filter_cubic                   = "VUID-vkCmdTraceRaysNV-None-02693";
        filter_cubic                       = "VUID-vkCmdTraceRaysNV-filterCubic-02694";
        filter_cubic_min_max               = "VUID-vkCmdTraceRaysNV-filterCubicMinmax-02695";
    }
};

struct DispatchVuidsCmdTraceRaysKHR: DrawDispatchVuid {
    DispatchVuidsCmdTraceRaysKHR() : DrawDispatchVuid() {
        pipeline_bound                     = "VUID-vkCmdTraceRaysKHR-None-02700";
        dynamic_state                      = "VUID-vkCmdTraceRaysKHR-commandBuffer-02701";
        compatible_pipeline                = "VUID-vkCmdTraceRaysKHR-None-02697";
        linear_sampler                     = "VUID-vkCmdTraceRaysKHR-magFilter-04553";
        cubic_sampler                      = "VUID-vkCmdTraceRaysKHR-None-02692";
        corner_sampled_address_mode        = "VUID-vkCmdTraceRaysKHR-flags-02696";
        imageview_atomic                   = "VUID-vkCmdTraceRaysKHR-None-02691";
        push_constants_set                 = "VUID-vkCmdTraceRaysKHR-None-02698";
        descriptor_valid                   = "VUID-vkCmdTraceRaysKHR-None-02699";
        sampler_imageview_type             = "VUID-vkCmdTraceRaysKHR-None-02702";
        sampler_implicitLod_dref_proj      = "VUID-vkCmdTraceRaysKHR-None-02703";
        sampler_bias_offset                = "VUID-vkCmdTraceRaysKHR-None-02704";
        dynamic_state_setting_commands     = "VUID-vkCmdTraceRaysKHR-None-02859";
        unprotected_command_buffer         = "VUID-vkCmdTraceRaysKHR-commandBuffer-02707";
        protected_command_buffer           = "VUID-vkCmdTraceRaysKHR-commandBuffer-04625";
        img_filter_cubic                   = "VUID-vkCmdTraceRaysKHR-None-02693";
        filter_cubic                       = "VUID-vkCmdTraceRaysKHR-filterCubic-02694";
        filter_cubic_min_max               = "VUID-vkCmdTraceRaysKHR-filterCubicMinmax-02695";
    }
};

struct DispatchVuidsCmdTraceRaysIndirectKHR: DrawDispatchVuid {
    DispatchVuidsCmdTraceRaysIndirectKHR() : DrawDispatchVuid() {
        pipeline_bound                     = "VUID-vkCmdTraceRaysIndirectKHR-None-02700";
        dynamic_state                      = "VUID-vkCmdTraceRaysIndirectKHR-commandBuffer-02701";
        compatible_pipeline                = "VUID-vkCmdTraceRaysIndirectKHR-None-02697";
        linear_sampler                     = "VUID-vkCmdTraceRaysIndirectKHR-magFilter-04553";
        cubic_sampler                      = "VUID-vkCmdTraceRaysIndirectKHR-None-02692";
        indirect_protected_cb              = "VUID-vkCmdTraceRaysIndirectKHR-commandBuffer-03635";
        indirect_contiguous_memory         = "VUID-vkCmdTraceRaysIndirectKHR-indirectDeviceAddress-03632";
        indirect_buffer_bit                = "VUID-vkCmdTraceRaysIndirectKHR-indirectDeviceAddress-03633";
        corner_sampled_address_mode        = "VUID-vkCmdTraceRaysIndirectKHR-flags-02696";
        imageview_atomic                   = "VUID-vkCmdTraceRaysIndirectKHR-None-02691";
        push_constants_set                 = "VUID-vkCmdTraceRaysIndirectKHR-None-02698";
        descriptor_valid                   = "VUID-vkCmdTraceRaysIndirectKHR-None-02699";
        sampler_imageview_type             = "VUID-vkCmdTraceRaysIndirectKHR-None-02702";
        sampler_implicitLod_dref_proj      = "VUID-vkCmdTraceRaysIndirectKHR-None-02703";
        sampler_bias_offset                = "VUID-vkCmdTraceRaysIndirectKHR-None-02704";
        dynamic_state_setting_commands     = "VUID-vkCmdTraceRaysIndirectKHR-None-02859";
        unprotected_command_buffer         = "VUID-vkCmdTraceRaysIndirectKHR-commandBuffer-02707";
        img_filter_cubic                   = "VUID-vkCmdTraceRaysIndirectKHR-None-02693";
        filter_cubic                       = "VUID-vkCmdTraceRaysIndirectKHR-filterCubic-02694";
        filter_cubic_min_max               = "VUID-vkCmdTraceRaysIndirectKHR-filterCubicMinmax-02695";
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
        linear_sampler                     = "VUID-vkCmdDrawMeshTasksNV-magFilter-04553";
        cubic_sampler                      = "VUID-vkCmdDrawMeshTasksNV-None-02692";
        viewport_count                     = "VUID-vkCmdDrawMeshTasksNV-viewportCount-03417";
        scissor_count                      = "VUID-vkCmdDrawMeshTasksNV-scissorCount-03418";
        viewport_scissor_count             = "VUID-vkCmdDrawMeshTasksNV-viewportCount-03419";
        primitive_topology                 = "VUID-vkCmdDrawMeshTasksNV-primitiveTopology-03420";
        corner_sampled_address_mode        = "VUID-vkCmdDrawMeshTasksNV-flags-02696";
        subpass_input                      = "VUID-vkCmdDrawMeshTasksNV-None-02686";
        imageview_atomic                   = "VUID-vkCmdDrawMeshTasksNV-None-02691";
        push_constants_set                 = "VUID-vkCmdDrawMeshTasksNV-None-02698";
        image_subresources                 = "VUID-vkCmdDrawMeshTasksNV-None-04584";
        descriptor_valid                   = "VUID-vkCmdDrawMeshTasksNV-None-02699";
        sampler_imageview_type             = "VUID-vkCmdDrawMeshTasksNV-None-02702";
        sampler_implicitLod_dref_proj      = "VUID-vkCmdDrawMeshTasksNV-None-02703";
        sampler_bias_offset                = "VUID-vkCmdDrawMeshTasksNV-None-02704";
        dynamic_state_setting_commands     = "VUID-vkCmdDrawMeshTasksNV-None-02859";
        rasterization_samples              = "VUID-vkCmdDrawMeshTasksNV-rasterizationSamples-04740";
        unprotected_command_buffer         = "VUID-vkCmdDrawMeshTasksNV-commandBuffer-02707";
        max_multiview_instance_index       = "VUID-vkCmdDrawMeshTasksNV-maxMultiviewInstanceIndex-02688";
        img_filter_cubic                   = "VUID-vkCmdDrawMeshTasksNV-None-02693";
        filter_cubic                       = "VUID-vkCmdDrawMeshTasksNV-filterCubic-02694";
        filter_cubic_min_max               = "VUID-vkCmdDrawMeshTasksNV-filterCubicMinmax-02695";
        viewport_count_primitive_shading_rate = "VUID-vkCmdDrawMeshTasksNV-primitiveFragmentShadingRateWithMultipleViewports-04552";
        patch_control_points               = "VUID-vkCmdDrawMeshTasksNV-None-04875";
        rasterizer_discard_enable          = "VUID-vkCmdDrawMeshTasksNV-None-04876";
        depth_bias_enable                  = "VUID-vkCmdDrawMeshTasksNV-None-04877";
        logic_op                           = "VUID-vkCmdDrawMeshTasksNV-logicOp-04878";
        primitive_restart_enable           = "VUID-vkCmdDrawMeshTasksNV-None-04879";
        vertex_input_binding_stride        = "VUID-vkCmdDrawMeshTasksNV-pStrides-04884";
        vertex_input                       = kVUID_Core_CmdDrawMeshTasksNV_VertexInput;
        blend_enable                       = "VUID-vkCmdDrawMeshTasksNV-blendEnable-04727";
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
        linear_sampler                     = "VUID-vkCmdDrawMeshTasksIndirectNV-magFilter-04553";
        cubic_sampler                      = "VUID-vkCmdDrawMeshTasksIndirectNV-None-02692";
        indirect_protected_cb              = "VUID-vkCmdDrawMeshTasksIndirectNV-commandBuffer-02711";
        indirect_contiguous_memory         = "VUID-vkCmdDrawMeshTasksIndirectNV-buffer-02708";
        indirect_buffer_bit                = "VUID-vkCmdDrawMeshTasksIndirectNV-buffer-02709";
        viewport_count                     = "VUID-vkCmdDrawMeshTasksIndirectNV-viewportCount-03417";
        scissor_count                      = "VUID-vkCmdDrawMeshTasksIndirectNV-scissorCount-03418";
        viewport_scissor_count             = "VUID-vkCmdDrawMeshTasksIndirectNV-viewportCount-03419";
        primitive_topology                 = "VUID-vkCmdDrawMeshTasksIndirectNV-primitiveTopology-03420";
        corner_sampled_address_mode        = "VUID-vkCmdDrawMeshTasksIndirectNV-flags-02696";
        subpass_input                      = "VUID-vkCmdDrawMeshTasksIndirectNV-None-02686";
        imageview_atomic                   = "VUID-vkCmdDrawMeshTasksIndirectNV-None-02691";
        push_constants_set                 = "VUID-vkCmdDrawMeshTasksIndirectNV-None-02698";
        image_subresources                 = "VUID-vkCmdDrawMeshTasksIndirectNV-None-04584";
        descriptor_valid                   = "VUID-vkCmdDrawMeshTasksIndirectNV-None-02699";
        sampler_imageview_type             = "VUID-vkCmdDrawMeshTasksIndirectNV-None-02702";
        sampler_implicitLod_dref_proj      = "VUID-vkCmdDrawMeshTasksIndirectNV-None-02703";
        sampler_bias_offset                = "VUID-vkCmdDrawMeshTasksIndirectNV-None-02704";
        dynamic_state_setting_commands     = "VUID-vkCmdDrawMeshTasksIndirectNV-None-02859";
        rasterization_samples              = "VUID-vkCmdDrawMeshTasksIndirectNV-rasterizationSamples-04740";
        unprotected_command_buffer         = "VUID-vkCmdDrawMeshTasksIndirectNV-commandBuffer-02707";
        max_multiview_instance_index       = "VUID-vkCmdDrawMeshTasksIndirectNV-maxMultiviewInstanceIndex-02688";
        img_filter_cubic                   = "VUID-vkCmdDrawMeshTasksIndirectNV-None-02693";
        filter_cubic                       = "VUID-vkCmdDrawMeshTasksIndirectNV-filterCubic-02694";
        filter_cubic_min_max               = "VUID-vkCmdDrawMeshTasksIndirectNV-filterCubicMinmax-02695";
        viewport_count_primitive_shading_rate = "VUID-vkCmdDrawMeshTasksIndirectNV-primitiveFragmentShadingRateWithMultipleViewports-04552";
        patch_control_points               = "VUID-vkCmdDrawMeshTasksIndirectNV-None-04875";
        rasterizer_discard_enable          = "VUID-vkCmdDrawMeshTasksIndirectNV-None-04876";
        depth_bias_enable                  = "VUID-vkCmdDrawMeshTasksIndirectNV-None-04877";
        logic_op                           = "VUID-vkCmdDrawMeshTasksIndirectNV-logicOp-04878";
        primitive_restart_enable           = "VUID-vkCmdDrawMeshTasksIndirectNV-None-04879";
        vertex_input_binding_stride        = "VUID-vkCmdDrawMeshTasksIndirectNV-pStrides-04884";
        vertex_input                       = kVUID_Core_CmdDrawMeshTasksIndirectNV_VertexInput;
        blend_enable                       = "VUID-vkCmdDrawMeshTasksIndirectNV-blendEnable-04727";
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
        linear_sampler                     = "VUID-vkCmdDrawMeshTasksIndirectCountNV-magFilter-04553";
        cubic_sampler                      = "VUID-vkCmdDrawMeshTasksIndirectCountNV-None-02692";
        indirect_protected_cb              = "VUID-vkCmdDrawMeshTasksIndirectCountNV-commandBuffer-02711";
        indirect_contiguous_memory         = "VUID-vkCmdDrawMeshTasksIndirectCountNV-buffer-02708";
        indirect_buffer_bit                = "VUID-vkCmdDrawMeshTasksIndirectCountNV-buffer-02709";
        viewport_count                     = "VUID-vkCmdDrawMeshTasksIndirectCountNV-viewportCount-03417";
        scissor_count                      = "VUID-vkCmdDrawMeshTasksIndirectCountNV-scissorCount-03418";
        viewport_scissor_count             = "VUID-vkCmdDrawMeshTasksIndirectCountNV-viewportCount-03419";
        primitive_topology                 = "VUID-vkCmdDrawMeshTasksIndirectCountNV-primitiveTopology-03420";
        corner_sampled_address_mode        = "VUID-vkCmdDrawMeshTasksIndirectCountNV-flags-02696";
        subpass_input                      = "VUID-vkCmdDrawMeshTasksIndirectCountNV-None-02686";
        imageview_atomic                   = "VUID-vkCmdDrawMeshTasksIndirectCountNV-None-02691";
        push_constants_set                 = "VUID-vkCmdDrawMeshTasksIndirectCountNV-None-02698";
        image_subresources                 = "VUID-vkCmdDrawMeshTasksIndirectCountNV-None-04584";
        descriptor_valid                   = "VUID-vkCmdDrawMeshTasksIndirectCountNV-None-02699";
        sampler_imageview_type             = "VUID-vkCmdDrawMeshTasksIndirectCountNV-None-02702";
        sampler_implicitLod_dref_proj      = "VUID-vkCmdDrawMeshTasksIndirectCountNV-None-02703";
        sampler_bias_offset                = "VUID-vkCmdDrawMeshTasksIndirectCountNV-None-02704";
        dynamic_state_setting_commands     = "VUID-vkCmdDrawMeshTasksIndirectCountNV-None-02859";
        rasterization_samples              = "VUID-vkCmdDrawMeshTasksIndirectCountNV-rasterizationSamples-04740";
        unprotected_command_buffer         = "VUID-vkCmdDrawMeshTasksIndirectCountNV-commandBuffer-02707";
        max_multiview_instance_index       = "VUID-vkCmdDrawMeshTasksIndirectCountNV-maxMultiviewInstanceIndex-02688";
        img_filter_cubic                   = "VUID-vkCmdDrawMeshTasksIndirectCountNV-None-02693";
        filter_cubic                       = "VUID-vkCmdDrawMeshTasksIndirectCountNV-filterCubic-02694";
        filter_cubic_min_max               = "VUID-vkCmdDrawMeshTasksIndirectCountNV-filterCubicMinmax-02695";
        viewport_count_primitive_shading_rate = "VUID-vkCmdDrawMeshTasksIndirectCountNV-primitiveFragmentShadingRateWithMultipleViewports-04552";
        patch_control_points               = "VUID-vkCmdDrawMeshTasksIndirectCountNV-None-04875";
        rasterizer_discard_enable          = "VUID-vkCmdDrawMeshTasksIndirectCountNV-None-04876";
        depth_bias_enable                  = "VUID-vkCmdDrawMeshTasksIndirectCountNV-None-04877";
        logic_op                           = "VUID-vkCmdDrawMeshTasksIndirectCountNV-logicOp-04878";
        primitive_restart_enable           = "VUID-vkCmdDrawMeshTasksIndirectCountNV-None-04879";
        vertex_input_binding_stride        = "VUID-vkCmdDrawMeshTasksIndirectCountNV-pStrides-04884";
        vertex_input                       = kVUID_Core_CmdDrawMeshTasksIndirectCountNV_VertexInput;
        blend_enable                       = "VUID-vkCmdDrawMeshTasksIndirectCountNV-blendEnable-04727";
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
        linear_sampler                     = "VUID-vkCmdDrawIndirectByteCountEXT-magFilter-04553";
        cubic_sampler                      = "VUID-vkCmdDrawIndirectByteCountEXT-None-02692";
        indirect_protected_cb              = "VUID-vkCmdDrawIndirectByteCountEXT-commandBuffer-02646";
        indirect_contiguous_memory         = "VUID-vkCmdDrawIndirectByteCountEXT-counterBuffer-04567",
        indirect_buffer_bit                = "VUID-vkCmdDrawIndirectByteCountEXT-counterBuffer-02290";
        viewport_count                     = "VUID-vkCmdDrawIndirectByteCountEXT-viewportCount-03417";
        scissor_count                      = "VUID-vkCmdDrawIndirectByteCountEXT-scissorCount-03418";
        viewport_scissor_count             = "VUID-vkCmdDrawIndirectByteCountEXT-viewportCount-03419";
        primitive_topology                 = "VUID-vkCmdDrawIndirectByteCountEXT-primitiveTopology-03420";
        corner_sampled_address_mode        = "VUID-vkCmdDrawIndirectByteCountEXT-flags-02696";
        subpass_input                      = "VUID-vkCmdDrawIndirectByteCountEXT-None-02686";
        imageview_atomic                   = "VUID-vkCmdDrawIndirectByteCountEXT-None-02691";
        push_constants_set                 = "VUID-vkCmdDrawIndirectByteCountEXT-None-02698";
        image_subresources                 = "VUID-vkCmdDrawIndirectByteCountEXT-None-04584";
        descriptor_valid                   = "VUID-vkCmdDrawIndirectByteCountEXT-None-02699";
        sampler_imageview_type             = "VUID-vkCmdDrawIndirectByteCountEXT-None-02702";
        sampler_implicitLod_dref_proj      = "VUID-vkCmdDrawIndirectByteCountEXT-None-02703";
        sampler_bias_offset                = "VUID-vkCmdDrawIndirectByteCountEXT-None-02704";
        vertex_binding_attribute           = "VUID-vkCmdDrawIndirectByteCountEXT-None-02721";
        dynamic_state_setting_commands     = "VUID-vkCmdDrawIndirectByteCountEXT-None-02859";
        rasterization_samples              = "VUID-vkCmdDrawIndirectByteCountEXT-rasterizationSamples-04740";
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
        vertex_input                       = kVUID_Core_CmdDrawIndirectByteCountEXT_VertexInput;
        blend_enable                       = "VUID-vkCmdDrawIndirectByteCountEXT-blendEnable-04727";
    }
};

struct DispatchVuidsCmdDispatchBase: DrawDispatchVuid {
    DispatchVuidsCmdDispatchBase() : DrawDispatchVuid() {
        pipeline_bound                     = "VUID-vkCmdDispatchBase-None-02700";
        dynamic_state                      = "VUID-vkCmdDispatchBase-commandBuffer-02701";
        compatible_pipeline                = "VUID-vkCmdDispatchBase-None-02697";
        linear_sampler                     = "VUID-vkCmdDispatchBase-magFilter-04553";
        cubic_sampler                      = "VUID-vkCmdDispatchBase-None-02692";
        corner_sampled_address_mode        = "VUID-vkCmdDispatchBase-flags-02696";
        imageview_atomic                   = "VUID-vkCmdDispatchBase-None-02691";
        push_constants_set                 = "VUID-vkCmdDispatchBase-None-02698";
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
    {CMD_DRAWINDEXEDINDIRECTCOUNT, DispatchVuidsCmdDrawIndexedIndirectCount()},
    {CMD_TRACERAYSNV, DispatchVuidsCmdTraceRaysNV()},
    {CMD_TRACERAYSKHR, DispatchVuidsCmdTraceRaysKHR()},
    {CMD_TRACERAYSINDIRECTKHR, DispatchVuidsCmdTraceRaysIndirectKHR()},
    {CMD_DRAWMESHTASKSNV, DispatchVuidsCmdDrawMeshTasksNV()},
    {CMD_DRAWMESHTASKSINDIRECTNV, DispatchVuidsCmdDrawMeshTasksIndirectNV()},
    {CMD_DRAWMESHTASKSINDIRECTCOUNTNV, DispatchVuidsCmdDrawMeshTasksIndirectCountNV()},
    {CMD_DRAWINDIRECTBYTECOUNTEXT, DispatchVuidsCmdDrawIndirectByteCountEXT()},
    {CMD_DISPATCHBASE, DispatchVuidsCmdDispatchBase()},
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
bool CoreChecks::ValidateCmdDrawType(VkCommandBuffer cmd_buffer, bool indexed, VkPipelineBindPoint bind_point, CMD_TYPE cmd_type,
                                     const char *caller) const {
    bool skip = false;
    const CMD_BUFFER_STATE *cb_state = GetCBState(cmd_buffer);
    if (cb_state) {
        skip |= ValidateCmd(cb_state, cmd_type, caller);
        skip |= ValidateCmdBufDrawState(cb_state, cmd_type, indexed, bind_point, caller);
    }
    return skip;
}

bool CoreChecks::ValidateCmdDrawInstance(VkCommandBuffer commandBuffer, uint32_t instanceCount, uint32_t firstInstance,
                                         CMD_TYPE cmd_type, const char *caller) const {
    bool skip = false;
    const DrawDispatchVuid vuid = GetDrawDispatchVuid(cmd_type);
    const auto *cb_node = Get<CMD_BUFFER_STATE>(commandBuffer);
    if (!cb_node) return skip;

    // Verify maxMultiviewInstanceIndex
    if (cb_node->activeRenderPass && cb_node->activeRenderPass->renderPass() && enabled_features.multiview_features.multiview &&
        ((instanceCount + firstInstance) > phys_dev_ext_props.multiview_props.maxMultiviewInstanceIndex)) {
        LogObjectList objlist(commandBuffer);
        objlist.add(cb_node->activeRenderPass->renderPass());
        skip |= LogError(objlist, vuid.max_multiview_instance_index,
                         "%s: renderpass %s multiview is enabled, and maxMultiviewInstanceIndex: %" PRIu32 ", but instanceCount: %" PRIu32
                         "and firstInstance: %" PRIu32 ".",
                         caller, report_data->FormatHandle(cb_node->activeRenderPass->renderPass()).c_str(),
                         phys_dev_ext_props.multiview_props.maxMultiviewInstanceIndex, instanceCount, firstInstance);
    }
    return skip;
}

bool CoreChecks::PreCallValidateCmdDraw(VkCommandBuffer commandBuffer, uint32_t vertexCount, uint32_t instanceCount,
                                        uint32_t firstVertex, uint32_t firstInstance) const {
    bool skip = false;
    skip |= ValidateCmdDrawInstance(commandBuffer, instanceCount, firstInstance, CMD_DRAW, "vkCmdDraw()");
    skip |= ValidateCmdDrawType(commandBuffer, false, VK_PIPELINE_BIND_POINT_GRAPHICS, CMD_DRAW, "vkCmdDraw()");
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

    skip |= ValidateCmdDrawInstance(commandBuffer, instanceCount, firstInstance, CMD_DRAWMULTIEXT, "vkCmdDrawMultiEXT()");
    skip |= ValidateCmdDrawType(commandBuffer, false, VK_PIPELINE_BIND_POINT_GRAPHICS, CMD_DRAWMULTIEXT, "vkCmdDrawMultiEXT()");
    return skip;
}

bool CoreChecks::ValidateCmdDrawIndexedBufferSize(VkCommandBuffer commandBuffer, uint32_t indexCount,
    uint32_t firstIndex, const char *caller, const char *first_index_vuid) const {
    bool skip = false;
    const CMD_BUFFER_STATE *cb_state = GetCBState(commandBuffer);
    if (cb_state->status & CBSTATUS_INDEX_BUFFER_BOUND) {
        unsigned int index_size = 0;
        const auto &index_buffer_binding = cb_state->index_buffer_binding;
        if (index_buffer_binding.index_type == VK_INDEX_TYPE_UINT16) {
            index_size = 2;
        }
        else if (index_buffer_binding.index_type == VK_INDEX_TYPE_UINT32) {
            index_size = 4;
        }
        else if (index_buffer_binding.index_type == VK_INDEX_TYPE_UINT8_EXT) {
            index_size = 1;
        }
        VkDeviceSize end_offset = (index_size * (static_cast<VkDeviceSize>(firstIndex) + indexCount)) + index_buffer_binding.offset;
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
    skip |= ValidateCmdDrawInstance(commandBuffer, instanceCount, firstInstance, CMD_DRAWINDEXED, "vkCmdDrawIndexed()");
    skip |= ValidateCmdDrawType(commandBuffer, true, VK_PIPELINE_BIND_POINT_GRAPHICS, CMD_DRAWINDEXED, "vkCmdDrawIndexed()");
    skip |= ValidateCmdDrawIndexedBufferSize(commandBuffer, indexCount, firstIndex, "vkCmdDrawIndexed()",
                                             "VUID-vkCmdDrawIndexed-firstIndex-04932");
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
    skip |=
        ValidateCmdDrawInstance(commandBuffer, instanceCount, firstInstance, CMD_DRAWMULTIINDEXEDEXT, "vkCmdDrawMultiIndexedEXT()");
    skip |= ValidateCmdDrawType(commandBuffer, true, VK_PIPELINE_BIND_POINT_GRAPHICS, CMD_DRAWMULTIINDEXEDEXT,
                                "vkCmdDrawMultiIndexedEXT()");
    const auto info_bytes = reinterpret_cast<const char *>(pIndexInfo);
    for (uint32_t i = 0; i < drawCount; i++) {
        const auto info_ptr = reinterpret_cast<const VkMultiDrawIndexedInfoEXT *>(info_bytes + i * stride);
        skip |= ValidateCmdDrawIndexedBufferSize(commandBuffer, info_ptr->indexCount, info_ptr->firstIndex,
                                                 "vkCmdDrawMultiIndexedEXT()", "VUID-vkCmdDrawMultiIndexedEXT-firstIndex-04938");
    }
    return skip;
}

bool CoreChecks::PreCallValidateCmdDrawIndirect(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                uint32_t drawCount, uint32_t stride) const {
    bool skip = ValidateCmdDrawType(commandBuffer, false, VK_PIPELINE_BIND_POINT_GRAPHICS, CMD_DRAWINDIRECT, "vkCmdDrawIndirect()");
    const BUFFER_STATE *buffer_state = GetBufferState(buffer);
    skip |= ValidateIndirectCmd(commandBuffer, buffer, CMD_DRAWINDIRECT, "vkCmdDrawIndirect()");
    if (drawCount > 1) {
        skip |= ValidateCmdDrawStrideWithStruct(commandBuffer, "VUID-vkCmdDrawIndirect-drawCount-00476", stride,
                                                "VkDrawIndirectCommand", sizeof(VkDrawIndirectCommand));
        skip |= ValidateCmdDrawStrideWithBuffer(commandBuffer, "VUID-vkCmdDrawIndirect-drawCount-00488", stride,
                                                "VkDrawIndirectCommand", sizeof(VkDrawIndirectCommand), drawCount, offset,
                                                buffer_state);
    } else if ((drawCount == 1) && (offset + sizeof(VkDrawIndirectCommand)) > buffer_state->createInfo.size) {
        skip |= LogError(commandBuffer, "VUID-vkCmdDrawIndirect-drawCount-00487",
                         "CmdDrawIndirect: drawCount equals 1 and (offset + sizeof(VkDrawIndirectCommand)) (%" PRIu64 ") is not less than "
                         "or equal to the size of buffer (%" PRIu64 ").",
                         (offset + sizeof(VkDrawIndirectCommand)), buffer_state->createInfo.size);
    }
    // TODO: If the drawIndirectFirstInstance feature is not enabled, all the firstInstance members of the
    // VkDrawIndirectCommand structures accessed by this command must be 0, which will require access to the contents of 'buffer'.
    return skip;
}

bool CoreChecks::PreCallValidateCmdDrawIndexedIndirect(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                       uint32_t drawCount, uint32_t stride) const {
    bool skip = ValidateCmdDrawType(commandBuffer, true, VK_PIPELINE_BIND_POINT_GRAPHICS, CMD_DRAWINDEXEDINDIRECT,
                                    "vkCmdDrawIndexedIndirect()");
    const BUFFER_STATE *buffer_state = GetBufferState(buffer);
    skip |= ValidateIndirectCmd(commandBuffer, buffer, CMD_DRAWINDEXEDINDIRECT, "vkCmdDrawIndexedIndirect()");
    if (drawCount > 1) {
        skip |= ValidateCmdDrawStrideWithStruct(commandBuffer, "VUID-vkCmdDrawIndexedIndirect-drawCount-00528", stride,
                                                "VkDrawIndexedIndirectCommand", sizeof(VkDrawIndexedIndirectCommand));
        skip |= ValidateCmdDrawStrideWithBuffer(commandBuffer, "VUID-vkCmdDrawIndexedIndirect-drawCount-00540", stride,
                                                "VkDrawIndexedIndirectCommand", sizeof(VkDrawIndexedIndirectCommand), drawCount,
                                                offset, buffer_state);
    } else if ((drawCount == 1) && (offset + sizeof(VkDrawIndexedIndirectCommand)) > buffer_state->createInfo.size) {
        skip |= LogError(
            commandBuffer, "VUID-vkCmdDrawIndexedIndirect-drawCount-00539",
            "CmdDrawIndexedIndirect: drawCount equals 1 and (offset + sizeof(VkDrawIndexedIndirectCommand)) (%" PRIu64 ") is not less than "
            "or equal to the size of buffer (%" PRIu64 ").",
            (offset + sizeof(VkDrawIndexedIndirectCommand)), buffer_state->createInfo.size);
    }
    // TODO: If the drawIndirectFirstInstance feature is not enabled, all the firstInstance members of the
    // VkDrawIndexedIndirectCommand structures accessed by this command must be 0, which will require access to the contents of
    // 'buffer'.
    return skip;
}

bool CoreChecks::PreCallValidateCmdDispatch(VkCommandBuffer commandBuffer, uint32_t x, uint32_t y, uint32_t z) const {
    bool skip = false;
    skip |= ValidateCmdDrawType(commandBuffer, false, VK_PIPELINE_BIND_POINT_COMPUTE, CMD_DISPATCH, "vkCmdDispatch()");
    return skip;
}

bool CoreChecks::ValidateBaseGroups(VkCommandBuffer commandBuffer, uint32_t baseGroupX, uint32_t baseGroupY, uint32_t baseGroupZ,
                                    const char *apiName) const {
    bool skip = false;
    if (baseGroupX || baseGroupY || baseGroupZ) {
        const CMD_BUFFER_STATE *cb_state = GetCBState(commandBuffer);
        const auto lv_bind_point = ConvertToLvlBindPoint(VK_PIPELINE_BIND_POINT_COMPUTE);
        const PIPELINE_STATE *pipeline_state = cb_state->lastBound[lv_bind_point].pipeline_state;
        if (pipeline_state && !(pipeline_state->GetPipelineCreateFlags() & VK_PIPELINE_CREATE_DISPATCH_BASE)) {
            skip |= LogError(commandBuffer, "VUID-vkCmdDispatchBase-baseGroupX-00427",
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
    skip |= ValidateCmdDrawType(commandBuffer, false, VK_PIPELINE_BIND_POINT_COMPUTE, CMD_DISPATCHBASE, "vkCmdDispatchBase()");
    skip |= ValidateBaseGroups(commandBuffer, baseGroupX, baseGroupY, baseGroupZ, "vkCmdDispatchBase()");
    return skip;
}

bool CoreChecks::PreCallValidateCmdDispatchBaseKHR(VkCommandBuffer commandBuffer, uint32_t baseGroupX, uint32_t baseGroupY,
                                                   uint32_t baseGroupZ, uint32_t groupCountX, uint32_t groupCountY,
                                                   uint32_t groupCountZ) const {
    bool skip = false;
    skip |= ValidateCmdDrawType(commandBuffer, false, VK_PIPELINE_BIND_POINT_COMPUTE, CMD_DISPATCHBASE, "vkCmdDispatchBaseKHR()");
    skip |= ValidateBaseGroups(commandBuffer, baseGroupX, baseGroupY, baseGroupZ, "vkCmdDispatchBaseKHR()");
    return skip;
}

bool CoreChecks::PreCallValidateCmdDispatchIndirect(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset) const {
    bool skip =
        ValidateCmdDrawType(commandBuffer, false, VK_PIPELINE_BIND_POINT_COMPUTE, CMD_DISPATCHINDIRECT, "vkCmdDispatchIndirect()");
    skip |= ValidateIndirectCmd(commandBuffer, buffer, CMD_DISPATCHINDIRECT, "vkCmdDispatchIndirect()");
    const BUFFER_STATE *buffer_state = GetBufferState(buffer);
    if ((offset + sizeof(VkDispatchIndirectCommand)) > buffer_state->createInfo.size) {
        skip |= LogError(commandBuffer, "VUID-vkCmdDispatchIndirect-offset-00407",
                         "vkCmdDispatchIndirect(): The sum of offset and the size of VkDispatchIndirectCommand is greater than the "
                         "size of the buffer");
    }
    return skip;
}
bool CoreChecks::ValidateCmdDrawIndirectCount(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                              VkBuffer countBuffer, VkDeviceSize countBufferOffset, uint32_t maxDrawCount,
                                              uint32_t stride, const char *apiName) const {
    bool skip = false;
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
        const BUFFER_STATE *buffer_state = GetBufferState(buffer);
        skip |= ValidateCmdDrawStrideWithBuffer(commandBuffer, "VUID-vkCmdDrawIndirectCount-maxDrawCount-03111", stride,
                                                "VkDrawIndirectCommand", sizeof(VkDrawIndirectCommand), maxDrawCount, offset,
                                                buffer_state);
    }

    skip |= ValidateCmdDrawType(commandBuffer, false, VK_PIPELINE_BIND_POINT_GRAPHICS, CMD_DRAWINDIRECTCOUNT, apiName);
    const BUFFER_STATE *count_buffer_state = GetBufferState(countBuffer);
    skip |= ValidateIndirectCmd(commandBuffer, buffer, CMD_DRAWINDIRECTCOUNT, apiName);
    skip |= ValidateMemoryIsBoundToBuffer(count_buffer_state, apiName, "VUID-vkCmdDrawIndirectCount-countBuffer-02714");
    skip |=
        ValidateBufferUsageFlags(count_buffer_state, VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT, true,
                                 "VUID-vkCmdDrawIndirectCount-countBuffer-02715", apiName, "VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT");
    return skip;
}

bool CoreChecks::PreCallValidateCmdDrawIndirectCountKHR(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                        VkBuffer countBuffer, VkDeviceSize countBufferOffset, uint32_t maxDrawCount,
                                                        uint32_t stride) const {
    return ValidateCmdDrawIndirectCount(commandBuffer, buffer, offset, countBuffer, countBufferOffset, maxDrawCount, stride,
                                        "vkCmdDrawIndirectCountKHR");
}

bool CoreChecks::PreCallValidateCmdDrawIndirectCount(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                     VkBuffer countBuffer, VkDeviceSize countBufferOffset, uint32_t maxDrawCount,
                                                     uint32_t stride) const {
    return ValidateCmdDrawIndirectCount(commandBuffer, buffer, offset, countBuffer, countBufferOffset, maxDrawCount, stride,
                                        "vkCmdDrawIndirectCount");
}

bool CoreChecks::ValidateCmdDrawIndexedIndirectCount(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                     VkBuffer countBuffer, VkDeviceSize countBufferOffset, uint32_t maxDrawCount,
                                                     uint32_t stride, const char *apiName) const {
    bool skip = false;
    if ((device_extensions.vk_khr_draw_indirect_count != kEnabledByCreateinfo) &&
        ((api_version >= VK_API_VERSION_1_2) && (enabled_features.core12.drawIndirectCount == VK_FALSE))) {
        skip |= LogError(commandBuffer, "VUID-vkCmdDrawIndexedIndirectCount-None-04445",
                         "%s(): Starting in Vulkan 1.2 the VkPhysicalDeviceVulkan12Features::drawIndirectCount must be enabled to "
                         "call this command.",
                         apiName);
    }
    skip |= ValidateCmdDrawStrideWithStruct(commandBuffer, "VUID-vkCmdDrawIndexedIndirectCount-stride-03142", stride,
                                            "VkDrawIndexedIndirectCommand", sizeof(VkDrawIndexedIndirectCommand));
    if (maxDrawCount > 1) {
        const BUFFER_STATE *buffer_state = GetBufferState(buffer);
        skip |= ValidateCmdDrawStrideWithBuffer(commandBuffer, "VUID-vkCmdDrawIndexedIndirectCount-maxDrawCount-03143", stride,
                                                "VkDrawIndexedIndirectCommand", sizeof(VkDrawIndexedIndirectCommand), maxDrawCount,
                                                offset, buffer_state);
    }

    skip |= ValidateCmdDrawType(commandBuffer, true, VK_PIPELINE_BIND_POINT_GRAPHICS, CMD_DRAWINDEXEDINDIRECTCOUNT, apiName);
    const BUFFER_STATE *count_buffer_state = GetBufferState(countBuffer);
    skip |= ValidateIndirectCmd(commandBuffer, buffer, CMD_DRAWINDEXEDINDIRECTCOUNT, apiName);
    skip |= ValidateMemoryIsBoundToBuffer(count_buffer_state, apiName, "VUID-vkCmdDrawIndexedIndirectCount-countBuffer-02714");
    skip |= ValidateBufferUsageFlags(count_buffer_state, VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT, true,
                                     "VUID-vkCmdDrawIndexedIndirectCount-countBuffer-02715", apiName,
                                     "VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT");
    return skip;
}

bool CoreChecks::PreCallValidateCmdDrawIndexedIndirectCountKHR(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                               VkBuffer countBuffer, VkDeviceSize countBufferOffset,
                                                               uint32_t maxDrawCount, uint32_t stride) const {
    return ValidateCmdDrawIndexedIndirectCount(commandBuffer, buffer, offset, countBuffer, countBufferOffset, maxDrawCount, stride,
                                               "vkCmdDrawIndexedIndirectCountKHR");
}

bool CoreChecks::PreCallValidateCmdDrawIndexedIndirectCount(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                            VkBuffer countBuffer, VkDeviceSize countBufferOffset,
                                                            uint32_t maxDrawCount, uint32_t stride) const {
    return ValidateCmdDrawIndexedIndirectCount(commandBuffer, buffer, offset, countBuffer, countBufferOffset, maxDrawCount, stride,
                                               "vkCmdDrawIndexedIndirectCount");
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
    if (device_extensions.vk_ext_transform_feedback && !phys_dev_ext_props.transform_feedback_props.transformFeedbackDraw) {
        skip |= LogError(commandBuffer, "VUID-vkCmdDrawIndirectByteCountEXT-transformFeedbackDraw-02288",
                         "%s: VkPhysicalDeviceTransformFeedbackPropertiesEXT::transformFeedbackDraw is not supported",
                         "vkCmdDrawIndirectByteCountEXT()");
    }
    skip |= ValidateCmdDrawInstance(commandBuffer, instanceCount, firstInstance, CMD_DRAWINDIRECTBYTECOUNTEXT,
                                    "vkCmdDrawIndirectByteCountEXT()");
    skip |= ValidateCmdDrawType(commandBuffer, false, VK_PIPELINE_BIND_POINT_GRAPHICS, CMD_DRAWINDIRECTBYTECOUNTEXT,
                                "vkCmdDrawIndirectByteCountEXT()");
    skip |= ValidateIndirectCmd(commandBuffer, counterBuffer, CMD_DRAWINDIRECTBYTECOUNTEXT, "vkCmdDrawIndirectByteCountEXT()");
    return skip;
}

bool CoreChecks::PreCallValidateCmdTraceRaysNV(VkCommandBuffer commandBuffer, VkBuffer raygenShaderBindingTableBuffer,
                                               VkDeviceSize raygenShaderBindingOffset, VkBuffer missShaderBindingTableBuffer,
                                               VkDeviceSize missShaderBindingOffset, VkDeviceSize missShaderBindingStride,
                                               VkBuffer hitShaderBindingTableBuffer, VkDeviceSize hitShaderBindingOffset,
                                               VkDeviceSize hitShaderBindingStride, VkBuffer callableShaderBindingTableBuffer,
                                               VkDeviceSize callableShaderBindingOffset, VkDeviceSize callableShaderBindingStride,
                                               uint32_t width, uint32_t height, uint32_t depth) const {
    bool skip =
        ValidateCmdDrawType(commandBuffer, true, VK_PIPELINE_BIND_POINT_RAY_TRACING_NV, CMD_TRACERAYSNV, "vkCmdTraceRaysNV()");
    const CMD_BUFFER_STATE *cb_state = GetCBState(commandBuffer);
    auto callable_shader_buffer_state = const_cast<BUFFER_STATE *>(GetBufferState(callableShaderBindingTableBuffer));
    if (callable_shader_buffer_state && callableShaderBindingOffset >= callable_shader_buffer_state->createInfo.size) {
        skip |= LogError(commandBuffer, "VUID-vkCmdTraceRaysNV-callableShaderBindingOffset-02461",
                         "vkCmdTraceRaysNV: callableShaderBindingOffset %" PRIu64
                         " must be less than the size of callableShaderBindingTableBuffer %" PRIu64 " .",
                         callableShaderBindingOffset, callable_shader_buffer_state->createInfo.size);
    }
    auto hit_shader_buffer_state = const_cast<BUFFER_STATE *>(GetBufferState(hitShaderBindingTableBuffer));
    if (hit_shader_buffer_state && hitShaderBindingOffset >= hit_shader_buffer_state->createInfo.size) {
        skip |= LogError(commandBuffer, "VUID-vkCmdTraceRaysNV-hitShaderBindingOffset-02459",
                         "vkCmdTraceRaysNV: hitShaderBindingOffset %" PRIu64
                         " must be less than the size of hitShaderBindingTableBuffer %" PRIu64 " .",
                         hitShaderBindingOffset, hit_shader_buffer_state->createInfo.size);
    }
    auto miss_shader_buffer_state = const_cast<BUFFER_STATE *>(GetBufferState(missShaderBindingTableBuffer));
    if (miss_shader_buffer_state && missShaderBindingOffset >= miss_shader_buffer_state->createInfo.size) {
        skip |= LogError(commandBuffer, "VUID-vkCmdTraceRaysNV-missShaderBindingOffset-02457",
                         "vkCmdTraceRaysNV: missShaderBindingOffset %" PRIu64
                         " must be less than the size of missShaderBindingTableBuffer %" PRIu64 " .",
                         missShaderBindingOffset, miss_shader_buffer_state->createInfo.size);
    }
    auto raygen_shader_buffer_state = const_cast<BUFFER_STATE *>(GetBufferState(raygenShaderBindingTableBuffer));
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

bool CoreChecks::PreCallValidateCmdTraceRaysKHR(VkCommandBuffer commandBuffer,
                                                const VkStridedDeviceAddressRegionKHR *pRaygenShaderBindingTable,
                                                const VkStridedDeviceAddressRegionKHR *pMissShaderBindingTable,
                                                const VkStridedDeviceAddressRegionKHR *pHitShaderBindingTable,
                                                const VkStridedDeviceAddressRegionKHR *pCallableShaderBindingTable, uint32_t width,
                                                uint32_t height, uint32_t depth) const {
    bool skip =
        ValidateCmdDrawType(commandBuffer, true, VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR, CMD_TRACERAYSKHR, "vkCmdTraceRaysKHR()");
    const CMD_BUFFER_STATE *cb_state = GetCBState(commandBuffer);
    const auto lv_bind_point = ConvertToLvlBindPoint(VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR);
    const PIPELINE_STATE *pipeline_state = cb_state->lastBound[lv_bind_point].pipeline_state;
    if (!pipeline_state || (pipeline_state && !pipeline_state->pipeline())) {
        skip |= LogError(device, "VUID-vkCmdTraceRaysKHR-None-02700",
                         "vkCmdTraceRaysKHR: A valid pipeline must be bound to the pipeline bind point used by this command.");
    } else {  // bound to valid RT pipeline
        if (pipeline_state->GetPipelineCreateFlags() & VK_PIPELINE_CREATE_RAY_TRACING_NO_NULL_INTERSECTION_SHADERS_BIT_KHR) {
            if (!pHitShaderBindingTable->deviceAddress) {
                skip |= LogError(device, "VUID-vkCmdTraceRaysKHR-flags-03697",
                                 "vkCmdTraceRaysKHR: If the currently bound ray tracing pipeline was created with flags "
                                 "that included VK_PIPELINE_CREATE_RAY_TRACING_NO_NULL_INTERSECTION_SHADERS_BIT_KHR, the "
                                 "deviceAddress member of pHitShaderBindingTable must not be zero.");
            }
            if (!pHitShaderBindingTable || pHitShaderBindingTable->size == 0 || pHitShaderBindingTable->stride == 0) {
                skip |= LogError(device, "VUID-vkCmdTraceRaysKHR-flags-03514",
                                 "vkCmdTraceRaysKHR: If the currently bound ray tracing pipeline was created with "
                                 "flags that included VK_PIPELINE_CREATE_RAY_TRACING_NO_NULL_INTERSECTION_SHADERS_BIT_KHR, "
                                 "entries in pHitShaderBindingTable accessed as a result of this command in order to "
                                 "execute an intersection shader must not be set to zero.");
            }
        }
        if (pipeline_state->GetPipelineCreateFlags() & VK_PIPELINE_CREATE_RAY_TRACING_NO_NULL_CLOSEST_HIT_SHADERS_BIT_KHR) {
            if (!pHitShaderBindingTable->deviceAddress) {
                skip |= LogError(device, "VUID-vkCmdTraceRaysKHR-flags-03696",
                                 "vkCmdTraceRaysKHR: If the currently bound ray tracing pipeline was created with flags "
                                 "that included VK_PIPELINE_CREATE_RAY_TRACING_NO_NULL_CLOSEST_HIT_SHADERS_BIT_KHR, the "
                                 "deviceAddress member of pHitShaderBindingTable must not be zero.");
            }
            if (!pHitShaderBindingTable || pHitShaderBindingTable->size == 0 || pHitShaderBindingTable->stride == 0) {
                skip |= LogError(device, "VUID-vkCmdTraceRaysKHR-flags-03513",
                                 "vkCmdTraceRaysKHR: If the currently bound ray tracing pipeline was created with "
                                 "flags that included VK_PIPELINE_CREATE_RAY_TRACING_NO_NULL_CLOSEST_HIT_SHADERS_BIT_KHR, "
                                 "entries in pHitShaderBindingTable accessed as a result of this command in order to "
                                 "execute a closest hit shader must not be set to zero.");
            }
        }
        if (pipeline_state->GetPipelineCreateFlags() & VK_PIPELINE_CREATE_RAY_TRACING_NO_NULL_ANY_HIT_SHADERS_BIT_KHR) {
            if (!pHitShaderBindingTable || pHitShaderBindingTable->size == 0 || pHitShaderBindingTable->stride == 0) {
                skip |= LogError(device, "VUID-vkCmdTraceRaysKHR-flags-03512",
                                 "vkCmdTraceRaysKHR: If the currently bound ray tracing pipeline was created with "
                                 "flags that included VK_PIPELINE_CREATE_RAY_TRACING_NO_NULL_ANY_HIT_SHADERS_BIT_KHR, "
                                 "entries in pHitShaderBindingTable accessed as a result of this command in order to "
                                 "execute an any hit shader must not be set to zero.");
            }
        }
    }
    return skip;
}

bool CoreChecks::PreCallValidateCmdTraceRaysIndirectKHR(VkCommandBuffer commandBuffer,
                                                        const VkStridedDeviceAddressRegionKHR *pRaygenShaderBindingTable,
                                                        const VkStridedDeviceAddressRegionKHR *pMissShaderBindingTable,
                                                        const VkStridedDeviceAddressRegionKHR *pHitShaderBindingTable,
                                                        const VkStridedDeviceAddressRegionKHR *pCallableShaderBindingTable,
                                                        VkDeviceAddress indirectDeviceAddress) const {
    bool skip = ValidateCmdDrawType(commandBuffer, true, VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR, CMD_TRACERAYSINDIRECTKHR,
                                    "vkCmdTraceRaysIndirectKHR()");
    const CMD_BUFFER_STATE *cb_state = GetCBState(commandBuffer);
    const auto lv_bind_point = ConvertToLvlBindPoint(VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR);
    const PIPELINE_STATE *pipeline_state = cb_state->lastBound[lv_bind_point].pipeline_state;
    if (!pipeline_state || (pipeline_state && !pipeline_state->pipeline())) {
        skip |=
            LogError(device, "VUID-vkCmdTraceRaysIndirectKHR-None-02700",
                     "vkCmdTraceRaysIndirectKHR: A valid pipeline must be bound to the pipeline bind point used by this command.");
    } else {  // valid bind point
        if (pipeline_state->GetPipelineCreateFlags() & VK_PIPELINE_CREATE_RAY_TRACING_NO_NULL_INTERSECTION_SHADERS_BIT_KHR) {
            if (!pHitShaderBindingTable || pHitShaderBindingTable->deviceAddress == 0) {
                skip |= LogError(device, "VUID-vkCmdTraceRaysIndirectKHR-flags-03697",
                                 "vkCmdTraceRaysIndirectKHR: If the currently bound ray tracing pipeline was created with "
                                 "flags that included VK_PIPELINE_CREATE_RAY_TRACING_NO_NULL_INTERSECTION_SHADERS_BIT_KHR, the "
                                 "deviceAddress member of pHitShaderBindingTable must not be zero.");
            }
            if (!pHitShaderBindingTable || pHitShaderBindingTable->size == 0 || pHitShaderBindingTable->stride == 0) {
                skip |= LogError(device, "VUID-vkCmdTraceRaysIndirectKHR-flags-03514",
                                 "vkCmdTraceRaysIndirectKHR: If the currently bound ray tracing pipeline was created with "
                                 "flags that included VK_PIPELINE_CREATE_RAY_TRACING_NO_NULL_INTERSECTION_SHADERS_BIT_KHR, "
                                 "entries in pHitShaderBindingTable accessed as a result of this command in order to "
                                 "execute an intersection shader must not be set to zero.");
            }
        }
        if (pipeline_state->GetPipelineCreateFlags() & VK_PIPELINE_CREATE_RAY_TRACING_NO_NULL_CLOSEST_HIT_SHADERS_BIT_KHR) {
            if (!pHitShaderBindingTable || pHitShaderBindingTable->deviceAddress == 0) {
                skip |= LogError(device, "VUID-vkCmdTraceRaysIndirectKHR-flags-03696",
                                 "vkCmdTraceRaysIndirectKHR:If the currently bound ray tracing pipeline was created with "
                                 "flags that included VK_PIPELINE_CREATE_RAY_TRACING_NO_NULL_CLOSEST_HIT_SHADERS_BIT_KHR, "
                                 "the deviceAddress member of pHitShaderBindingTable must not be zero.");
            }
            if (!pHitShaderBindingTable || pHitShaderBindingTable->size == 0 || pHitShaderBindingTable->stride == 0) {
                skip |= LogError(device, "VUID-vkCmdTraceRaysIndirectKHR-flags-03513",
                                 "vkCmdTraceRaysIndirectKHR: If the currently bound ray tracing pipeline was created with "
                                 "flags that included VK_PIPELINE_CREATE_RAY_TRACING_NO_NULL_CLOSEST_HIT_SHADERS_BIT_KHR, "
                                 "entries in pHitShaderBindingTable accessed as a result of this command in order to "
                                 "execute a closest hit shader must not be set to zero.");
            }
        }
        if (pipeline_state->GetPipelineCreateFlags() & VK_PIPELINE_CREATE_RAY_TRACING_NO_NULL_ANY_HIT_SHADERS_BIT_KHR) {
            if (!pHitShaderBindingTable || pHitShaderBindingTable->size == 0 || pHitShaderBindingTable->stride == 0) {
                skip |= LogError(device, "VUID-vkCmdTraceRaysIndirectKHR-flags-03512",
                                 "vkCmdTraceRaysIndirectKHR: If the currently bound ray tracing pipeline was created with "
                                 "flags that included VK_PIPELINE_CREATE_RAY_TRACING_NO_NULL_ANY_HIT_SHADERS_BIT_KHR, "
                                 "entries in pHitShaderBindingTable accessed as a result of this command in order to "
                                 "execute an any hit shader must not be set to zero.");
            }
        }
    }
    return skip;
}

bool CoreChecks::PreCallValidateCmdDrawMeshTasksNV(VkCommandBuffer commandBuffer, uint32_t taskCount, uint32_t firstTask) const {
    bool skip =
        ValidateCmdDrawType(commandBuffer, false, VK_PIPELINE_BIND_POINT_GRAPHICS, CMD_DRAWMESHTASKSNV, "vkCmdDrawMeshTasksNV()");
    return skip;
}

bool CoreChecks::PreCallValidateCmdDrawMeshTasksIndirectNV(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                           uint32_t drawCount, uint32_t stride) const {
    bool skip = ValidateCmdDrawType(commandBuffer, false, VK_PIPELINE_BIND_POINT_GRAPHICS, CMD_DRAWMESHTASKSINDIRECTNV,
                                    "vkCmdDrawMeshTasksIndirectNV()");
    const BUFFER_STATE *buffer_state = GetBufferState(buffer);
    skip |= ValidateIndirectCmd(commandBuffer, buffer, CMD_DRAWMESHTASKSINDIRECTNV, "vkCmdDrawMeshTasksIndirectNV()");
    if (drawCount > 1) {
        skip |= ValidateCmdDrawStrideWithBuffer(commandBuffer, "VUID-vkCmdDrawMeshTasksIndirectNV-drawCount-02157", stride,
                                                "VkDrawMeshTasksIndirectCommandNV", sizeof(VkDrawMeshTasksIndirectCommandNV),
                                                drawCount, offset, buffer_state);
    }
    return skip;
}

bool CoreChecks::PreCallValidateCmdDrawMeshTasksIndirectCountNV(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                                VkBuffer countBuffer, VkDeviceSize countBufferOffset,
                                                                uint32_t maxDrawCount, uint32_t stride) const {
    bool skip = ValidateCmdDrawType(commandBuffer, false, VK_PIPELINE_BIND_POINT_GRAPHICS, CMD_DRAWMESHTASKSINDIRECTCOUNTNV,
                                    "vkCmdDrawMeshTasksIndirectCountNV()");
    const BUFFER_STATE *buffer_state = GetBufferState(buffer);
    const BUFFER_STATE *count_buffer_state = GetBufferState(countBuffer);
    skip |= ValidateIndirectCmd(commandBuffer, buffer, CMD_DRAWMESHTASKSINDIRECTCOUNTNV, "vkCmdDrawMeshTasksIndirectCountNV()");
    skip |= ValidateMemoryIsBoundToBuffer(count_buffer_state, "vkCmdDrawMeshTasksIndirectCountNV()",
                                          "VUID-vkCmdDrawMeshTasksIndirectCountNV-countBuffer-02714");
    skip |= ValidateBufferUsageFlags(count_buffer_state, VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT, true,
                                     "VUID-vkCmdDrawMeshTasksIndirectCountNV-countBuffer-02715",
                                     "vkCmdDrawMeshTasksIndirectCountNV()", "VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT");
    skip |= ValidateCmdDrawStrideWithStruct(commandBuffer, "VUID-vkCmdDrawMeshTasksIndirectCountNV-stride-02182", stride,
                                            "VkDrawMeshTasksIndirectCommandNV", sizeof(VkDrawMeshTasksIndirectCommandNV));
    if (maxDrawCount > 1) {
        skip |= ValidateCmdDrawStrideWithBuffer(commandBuffer, "VUID-vkCmdDrawMeshTasksIndirectCountNV-maxDrawCount-02183", stride,
                                                "VkDrawMeshTasksIndirectCommandNV", sizeof(VkDrawMeshTasksIndirectCommandNV),
                                                maxDrawCount, offset, buffer_state);
    }
    return skip;
}
