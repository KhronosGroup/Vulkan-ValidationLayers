#!/usr/bin/python3 -i
#
# Copyright (c) 2023-2026 The Khronos Group Inc.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

import os
from base_generator import BaseGenerator

from dataclasses import dataclass, field
@dataclass
class EnableState:
    """What is needed to enable the an element (the <enable> tag in the XML)"""
    # There are 4 options for what will be non-None
    # 1. version
    # 2. extension
    # 3. struct + feature + requires
    # 4. property + member + values + (optional)requires
    version: (str | None)
    extension: (str | None)
    struct: (str | None)
    feature: (str | None)
    requires: (str | None)
    property: (str | None)
    member: (str | None)
    value: (str | None)

@dataclass
class DynamicState:
    name: str # ex) VK_DYNAMIC_STATE_BLEND_CONSTANTS
    commands: list[str] # ex) [vkCmdSetDepthBias, vkCmdSetDepthBias2EXT]
    shaderStage: str # ex) VK_SHADER_STAGE_FRAGMENT_BIT
    # A list of one of the 4 pipeline sub states:
    # ['Vertex Input', 'Pre-Rasterization Shader, 'Fragment Output', 'Fragment Shader']
    # Will be empty if not used for graphics (ex VK_DYNAMIC_STATE_RAY_TRACING_PIPELINE_STACK_SIZE_KHR)
    pipelineSubStates: list[str]
    # Some dynamic state (VK_DYNAMIC_STATE_VIEWPORT) is not used in Shader Objects
    pipelineOnly: bool
    # If the dynamic state is ignored when rasterizerDiscardEnable is VK_FALSE
    requiresRasterization: bool
    # The VkDynamicState that is required for this dynamic state to be active
    stateRequired: (str | None)
    # Unique string to identify some complex state required for this dynamic state to be active
    specialRequired: (str | None)
    # list of extensions/features for this dynamic state to be active
    enable: list[EnableState]

#
# DynamicStateOutputGenerator - Generate SPIR-V validation
# for SPIR-V extensions and capabilities
class DynamicStateOutputGenerator(BaseGenerator):
    def __init__(self):
        BaseGenerator.__init__(self)

    def generate(self):
        self.vk.dynamicStates = {
            'VK_DYNAMIC_STATE_VIEWPORT': DynamicState('VK_DYNAMIC_STATE_VIEWPORT', ['vkCmdSetViewport'], 'VK_SHADER_STAGE_ALL', ['Pre-Rasterization Shader'], True, False, None, None, []),
            'VK_DYNAMIC_STATE_SCISSOR': DynamicState('VK_DYNAMIC_STATE_SCISSOR', ['vkCmdSetScissor'], 'VK_SHADER_STAGE_ALL', ['Pre-Rasterization Shader'], True, False, None, None, []),
            'VK_DYNAMIC_STATE_LINE_WIDTH': DynamicState('VK_DYNAMIC_STATE_LINE_WIDTH', ['vkCmdSetLineWidth'], 'VK_SHADER_STAGE_ALL', ['Pre-Rasterization Shader'], False, True, None, 'lineTopology', []),
            'VK_DYNAMIC_STATE_DEPTH_BIAS': DynamicState('VK_DYNAMIC_STATE_DEPTH_BIAS', ['vkCmdSetDepthBias', 'vkCmdSetDepthBias2EXT'], 'VK_SHADER_STAGE_ALL', ['Pre-Rasterization Shader'], False, True, 'VK_DYNAMIC_STATE_DEPTH_BIAS_ENABLE', None, []),
            'VK_DYNAMIC_STATE_BLEND_CONSTANTS': DynamicState('VK_DYNAMIC_STATE_BLEND_CONSTANTS', ['vkCmdSetBlendConstants'], 'VK_SHADER_STAGE_FRAGMENT_BIT', ['Fragment Output'], False, True, None, 'colorBlendFactor', []),
            'VK_DYNAMIC_STATE_DEPTH_BOUNDS': DynamicState('VK_DYNAMIC_STATE_DEPTH_BOUNDS', ['vkCmdSetDepthBounds'], 'VK_SHADER_STAGE_ALL', ['Fragment Shader'], False, True, 'VK_DYNAMIC_STATE_DEPTH_BOUNDS_TEST_ENABLE', None, []),
            'VK_DYNAMIC_STATE_STENCIL_COMPARE_MASK': DynamicState('VK_DYNAMIC_STATE_STENCIL_COMPARE_MASK', ['vkCmdSetStencilCompareMask'], 'VK_SHADER_STAGE_ALL', ['Fragment Shader'], False, True, 'VK_DYNAMIC_STATE_STENCIL_TEST_ENABLE', None, []),
            'VK_DYNAMIC_STATE_STENCIL_WRITE_MASK': DynamicState('VK_DYNAMIC_STATE_STENCIL_WRITE_MASK', ['vkCmdSetStencilWriteMask'], 'VK_SHADER_STAGE_ALL', ['Fragment Shader'], False, True, 'VK_DYNAMIC_STATE_STENCIL_TEST_ENABLE', None, []),
            'VK_DYNAMIC_STATE_STENCIL_REFERENCE': DynamicState('VK_DYNAMIC_STATE_STENCIL_REFERENCE', ['vkCmdSetStencilReference'], 'VK_SHADER_STAGE_ALL', ['Fragment Shader'], False, True, 'VK_DYNAMIC_STATE_STENCIL_TEST_ENABLE', None, []),
            'VK_DYNAMIC_STATE_CULL_MODE': DynamicState('VK_DYNAMIC_STATE_CULL_MODE', ['vkCmdSetCullMode'], 'VK_SHADER_STAGE_ALL', ['Pre-Rasterization Shader'], False, True, None, None, []),
            'VK_DYNAMIC_STATE_FRONT_FACE': DynamicState('VK_DYNAMIC_STATE_FRONT_FACE', ['vkCmdSetFrontFace'], 'VK_SHADER_STAGE_ALL', ['Pre-Rasterization Shader'], False, True, None, None, []),
            'VK_DYNAMIC_STATE_PRIMITIVE_TOPOLOGY': DynamicState('VK_DYNAMIC_STATE_PRIMITIVE_TOPOLOGY', ['vkCmdSetPrimitiveTopology'], 'VK_SHADER_STAGE_VERTEX_BIT', ['Vertex Input'], False, False, None, None, []),
            'VK_DYNAMIC_STATE_VIEWPORT_WITH_COUNT': DynamicState('VK_DYNAMIC_STATE_VIEWPORT_WITH_COUNT', ['vkCmdSetViewportWithCount'], 'VK_SHADER_STAGE_ALL', ['Pre-Rasterization Shader'], False, False, None, None, []),
            'VK_DYNAMIC_STATE_SCISSOR_WITH_COUNT': DynamicState('VK_DYNAMIC_STATE_SCISSOR_WITH_COUNT', ['vkCmdSetScissorWithCount'], 'VK_SHADER_STAGE_ALL', ['Pre-Rasterization Shader'], False, False, None, None, []),
            'VK_DYNAMIC_STATE_VERTEX_INPUT_BINDING_STRIDE': DynamicState('VK_DYNAMIC_STATE_VERTEX_INPUT_BINDING_STRIDE', ['vkCmdBindVertexBuffers2'], 'VK_SHADER_STAGE_VERTEX_BIT', ['Vertex Input'], True, False, None, None, []),
            'VK_DYNAMIC_STATE_DEPTH_TEST_ENABLE': DynamicState('VK_DYNAMIC_STATE_DEPTH_TEST_ENABLE', ['vkCmdSetDepthTestEnable'], 'VK_SHADER_STAGE_ALL', ['Fragment Shader'], False, True, None, None, []),
            'VK_DYNAMIC_STATE_DEPTH_WRITE_ENABLE': DynamicState('VK_DYNAMIC_STATE_DEPTH_WRITE_ENABLE', ['vkCmdSetDepthWriteEnable'], 'VK_SHADER_STAGE_ALL', ['Fragment Shader'], False, True, 'VK_DYNAMIC_STATE_DEPTH_TEST_ENABLE', None, []),
            'VK_DYNAMIC_STATE_DEPTH_COMPARE_OP': DynamicState('VK_DYNAMIC_STATE_DEPTH_COMPARE_OP', ['vkCmdSetDepthCompareOp'], 'VK_SHADER_STAGE_ALL', ['Fragment Shader'], False, True, 'VK_DYNAMIC_STATE_DEPTH_TEST_ENABLE', None, []),
            'VK_DYNAMIC_STATE_DEPTH_BOUNDS_TEST_ENABLE': DynamicState('VK_DYNAMIC_STATE_DEPTH_BOUNDS_TEST_ENABLE', ['vkCmdSetDepthBoundsTestEnable'], 'VK_SHADER_STAGE_ALL', ['Fragment Shader'], False, True, None, None, [EnableState(version=None, extension=None, struct='VkPhysicalDeviceFeatures', feature='depthBounds', requires='VK_VERSION_1_0', property=None, member=None, value=None)]),
            'VK_DYNAMIC_STATE_STENCIL_TEST_ENABLE': DynamicState('VK_DYNAMIC_STATE_STENCIL_TEST_ENABLE', ['vkCmdSetStencilTestEnable'], 'VK_SHADER_STAGE_ALL', ['Fragment Shader'], False, True, None, None, []),
            'VK_DYNAMIC_STATE_STENCIL_OP': DynamicState('VK_DYNAMIC_STATE_STENCIL_OP', ['vkCmdSetStencilOp'], 'VK_SHADER_STAGE_ALL', ['Fragment Shader'], False, True, 'VK_DYNAMIC_STATE_STENCIL_TEST_ENABLE', None, []),
            'VK_DYNAMIC_STATE_RASTERIZER_DISCARD_ENABLE': DynamicState('VK_DYNAMIC_STATE_RASTERIZER_DISCARD_ENABLE', ['vkCmdSetRasterizerDiscardEnable'], 'VK_SHADER_STAGE_ALL', ['Pre-Rasterization Shader'], False, False, None, None, []),
            'VK_DYNAMIC_STATE_DEPTH_BIAS_ENABLE': DynamicState('VK_DYNAMIC_STATE_DEPTH_BIAS_ENABLE', ['vkCmdSetDepthBiasEnable'], 'VK_SHADER_STAGE_ALL', ['Pre-Rasterization Shader'], False, True, None, None, []),
            'VK_DYNAMIC_STATE_PRIMITIVE_RESTART_ENABLE': DynamicState('VK_DYNAMIC_STATE_PRIMITIVE_RESTART_ENABLE', ['vkCmdSetPrimitiveRestartEnable'], 'VK_SHADER_STAGE_VERTEX_BIT', ['Vertex Input'], False, False, None, None, []),
            'VK_DYNAMIC_STATE_VIEWPORT_W_SCALING_NV': DynamicState('VK_DYNAMIC_STATE_VIEWPORT_W_SCALING_NV', ['vkCmdSetViewportWScalingNV'], 'VK_SHADER_STAGE_ALL', ['Pre-Rasterization Shader'], False, False, 'VK_DYNAMIC_STATE_VIEWPORT_W_SCALING_ENABLE_NV', None, [EnableState(version=None, extension='VK_NV_clip_space_w_scaling', struct=None, feature=None, requires=None, property=None, member=None, value=None)]),
            'VK_DYNAMIC_STATE_DISCARD_RECTANGLE_EXT': DynamicState('VK_DYNAMIC_STATE_DISCARD_RECTANGLE_EXT', ['vkCmdSetDiscardRectangleEXT'], 'VK_SHADER_STAGE_ALL', ['Pre-Rasterization Shader'], False, True, 'VK_DYNAMIC_STATE_DISCARD_RECTANGLE_ENABLE_EXT', None, [EnableState(version=None, extension='VK_EXT_discard_rectangles', struct=None, feature=None, requires=None, property=None, member=None, value=None)]),
            'VK_DYNAMIC_STATE_DISCARD_RECTANGLE_ENABLE_EXT': DynamicState('VK_DYNAMIC_STATE_DISCARD_RECTANGLE_ENABLE_EXT', ['vkCmdSetDiscardRectangleEnableEXT'], 'VK_SHADER_STAGE_ALL', ['Pre-Rasterization Shader'], False, True, None, None, [EnableState(version=None, extension='VK_EXT_discard_rectangles', struct=None, feature=None, requires=None, property=None, member=None, value=None)]),
            'VK_DYNAMIC_STATE_DISCARD_RECTANGLE_MODE_EXT': DynamicState('VK_DYNAMIC_STATE_DISCARD_RECTANGLE_MODE_EXT', ['vkCmdSetDiscardRectangleModeEXT'], 'VK_SHADER_STAGE_ALL', ['Pre-Rasterization Shader'], False, True, 'VK_DYNAMIC_STATE_DISCARD_RECTANGLE_ENABLE_EXT', None, [EnableState(version=None, extension='VK_EXT_discard_rectangles', struct=None, feature=None, requires=None, property=None, member=None, value=None)]),
            'VK_DYNAMIC_STATE_SAMPLE_LOCATIONS_EXT': DynamicState('VK_DYNAMIC_STATE_SAMPLE_LOCATIONS_EXT', ['vkCmdSetSampleLocationsEXT'], 'VK_SHADER_STAGE_ALL', ['Fragment Shader', 'Fragment Output'], False, True, 'VK_DYNAMIC_STATE_SAMPLE_LOCATIONS_ENABLE_EXT', None, [EnableState(version=None, extension='VK_EXT_sample_locations', struct=None, feature=None, requires=None, property=None, member=None, value=None)]),
            'VK_DYNAMIC_STATE_VIEWPORT_SHADING_RATE_PALETTE_NV': DynamicState('VK_DYNAMIC_STATE_VIEWPORT_SHADING_RATE_PALETTE_NV', ['vkCmdSetViewportShadingRatePaletteNV'], 'VK_SHADER_STAGE_ALL', ['Pre-Rasterization Shader'], False, True, 'VK_DYNAMIC_STATE_SHADING_RATE_IMAGE_ENABLE_NV', None, [EnableState(version=None, extension=None, struct='VkPhysicalDeviceShadingRateImageFeaturesNV', feature='shadingRateImage', requires='VK_NV_shading_rate_image', property=None, member=None, value=None)]),
            'VK_DYNAMIC_STATE_VIEWPORT_COARSE_SAMPLE_ORDER_NV': DynamicState('VK_DYNAMIC_STATE_VIEWPORT_COARSE_SAMPLE_ORDER_NV', ['vkCmdSetCoarseSampleOrderNV'], 'VK_SHADER_STAGE_ALL', ['Pre-Rasterization Shader'], False, True, None, None, [EnableState(version=None, extension=None, struct='VkPhysicalDeviceShadingRateImageFeaturesNV', feature='shadingRateImage', requires='VK_NV_shading_rate_image', property=None, member=None, value=None)]),
            'VK_DYNAMIC_STATE_EXCLUSIVE_SCISSOR_ENABLE_NV': DynamicState('VK_DYNAMIC_STATE_EXCLUSIVE_SCISSOR_ENABLE_NV', ['vkCmdSetExclusiveScissorEnableNV'], 'VK_SHADER_STAGE_ALL', ['Pre-Rasterization Shader'], False, False, None, None, [EnableState(version=None, extension=None, struct='VkPhysicalDeviceExclusiveScissorFeaturesNV', feature='exclusiveScissor', requires='VK_NV_scissor_exclusive', property=None, member=None, value=None)]),
            'VK_DYNAMIC_STATE_EXCLUSIVE_SCISSOR_NV': DynamicState('VK_DYNAMIC_STATE_EXCLUSIVE_SCISSOR_NV', ['vkCmdSetExclusiveScissorNV'], 'VK_SHADER_STAGE_ALL', ['Pre-Rasterization Shader'], False, False, 'VK_DYNAMIC_STATE_EXCLUSIVE_SCISSOR_ENABLE_NV', None, [EnableState(version=None, extension=None, struct='VkPhysicalDeviceExclusiveScissorFeaturesNV', feature='exclusiveScissor', requires='VK_NV_scissor_exclusive', property=None, member=None, value=None)]),
            'VK_DYNAMIC_STATE_FRAGMENT_SHADING_RATE_KHR': DynamicState('VK_DYNAMIC_STATE_FRAGMENT_SHADING_RATE_KHR', ['vkCmdSetFragmentShadingRateKHR'], 'VK_SHADER_STAGE_ALL', ['Pre-Rasterization Shader', 'Fragment Shader'], False, True, None, None, [EnableState(version=None, extension=None, struct='VkPhysicalDeviceFragmentShadingRateFeaturesKHR', feature='pipelineFragmentShadingRate', requires='VK_KHR_fragment_shading_rate', property=None, member=None, value=None)]),
            'VK_DYNAMIC_STATE_LINE_STIPPLE': DynamicState('VK_DYNAMIC_STATE_LINE_STIPPLE', ['vkCmdSetLineStipple'], 'VK_SHADER_STAGE_ALL', ['Pre-Rasterization Shader'], False, True, 'VK_DYNAMIC_STATE_LINE_STIPPLE_ENABLE_EXT', 'lineTopology', [EnableState(version=None, extension=None, struct='VkPhysicalDeviceLineRasterizationFeaturesKHR', feature='stippledRectangularLines', requires='VK_KHR_line_rasterization', property=None, member=None, value=None), EnableState(version=None, extension=None, struct='VkPhysicalDeviceLineRasterizationFeaturesKHR', feature='stippledBresenhamLines', requires='VK_KHR_line_rasterization', property=None, member=None, value=None), EnableState(version=None, extension=None, struct='VkPhysicalDeviceLineRasterizationFeaturesKHR', feature='stippledSmoothLines', requires='VK_KHR_line_rasterization', property=None, member=None, value=None)]),
            'VK_DYNAMIC_STATE_VERTEX_INPUT_EXT': DynamicState('VK_DYNAMIC_STATE_VERTEX_INPUT_EXT', ['vkCmdSetVertexInputEXT'], 'VK_SHADER_STAGE_VERTEX_BIT', ['Vertex Input'], False, False, None, None, []),
            'VK_DYNAMIC_STATE_PATCH_CONTROL_POINTS_EXT': DynamicState('VK_DYNAMIC_STATE_PATCH_CONTROL_POINTS_EXT', ['vkCmdSetPatchControlPointsEXT'], 'VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT', ['Pre-Rasterization Shader'], False, False, None, 'patchListPrimitiveTopology', []),
            'VK_DYNAMIC_STATE_LOGIC_OP_EXT': DynamicState('VK_DYNAMIC_STATE_LOGIC_OP_EXT', ['vkCmdSetLogicOpEXT'], 'VK_SHADER_STAGE_FRAGMENT_BIT', ['Fragment Output'], False, True, 'VK_DYNAMIC_STATE_LOGIC_OP_ENABLE_EXT', None, []),
            'VK_DYNAMIC_STATE_COLOR_WRITE_ENABLE_EXT': DynamicState('VK_DYNAMIC_STATE_COLOR_WRITE_ENABLE_EXT', ['vkCmdSetColorWriteEnableEXT'], 'VK_SHADER_STAGE_FRAGMENT_BIT', ['Fragment Output'], False, True, None, 'boundColorAttachment', [EnableState(version=None, extension=None, struct='VkPhysicalDeviceColorWriteEnableFeaturesEXT', feature='colorWriteEnable', requires='VK_EXT_color_write_enable', property=None, member=None, value=None)]),
            'VK_DYNAMIC_STATE_TESSELLATION_DOMAIN_ORIGIN_EXT': DynamicState('VK_DYNAMIC_STATE_TESSELLATION_DOMAIN_ORIGIN_EXT', ['vkCmdSetTessellationDomainOriginEXT'], 'VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT', ['Pre-Rasterization Shader'], False, False, None, None, []),
            'VK_DYNAMIC_STATE_DEPTH_CLAMP_ENABLE_EXT': DynamicState('VK_DYNAMIC_STATE_DEPTH_CLAMP_ENABLE_EXT', ['vkCmdSetDepthClampEnableEXT'], 'VK_SHADER_STAGE_ALL', ['Pre-Rasterization Shader'], False, True, None, None, [EnableState(version=None, extension=None, struct='VkPhysicalDeviceFeatures', feature='depthClamp', requires='VK_VERSION_1_0', property=None, member=None, value=None)]),
            'VK_DYNAMIC_STATE_POLYGON_MODE_EXT': DynamicState('VK_DYNAMIC_STATE_POLYGON_MODE_EXT', ['vkCmdSetPolygonModeEXT'], 'VK_SHADER_STAGE_ALL', ['Pre-Rasterization Shader'], False, True, None, None, []),
            'VK_DYNAMIC_STATE_RASTERIZATION_SAMPLES_EXT': DynamicState('VK_DYNAMIC_STATE_RASTERIZATION_SAMPLES_EXT', ['vkCmdSetRasterizationSamplesEXT'], 'VK_SHADER_STAGE_ALL', ['Fragment Shader', 'Fragment Output'], False, True, None, None, []),
            'VK_DYNAMIC_STATE_SAMPLE_MASK_EXT': DynamicState('VK_DYNAMIC_STATE_SAMPLE_MASK_EXT', ['vkCmdSetSampleMaskEXT'], 'VK_SHADER_STAGE_ALL', ['Fragment Shader', 'Fragment Output'], False, True, None, None, []),
            'VK_DYNAMIC_STATE_ALPHA_TO_COVERAGE_ENABLE_EXT': DynamicState('VK_DYNAMIC_STATE_ALPHA_TO_COVERAGE_ENABLE_EXT', ['vkCmdSetAlphaToCoverageEnableEXT'], 'VK_SHADER_STAGE_ALL', ['Fragment Shader', 'Fragment Output'], False, True, None, None, []),
            'VK_DYNAMIC_STATE_ALPHA_TO_ONE_ENABLE_EXT': DynamicState('VK_DYNAMIC_STATE_ALPHA_TO_ONE_ENABLE_EXT', ['vkCmdSetAlphaToOneEnableEXT'], 'VK_SHADER_STAGE_ALL', ['Fragment Shader', 'Fragment Output'], False, True, None, None, [EnableState(version=None, extension=None, struct='VkPhysicalDeviceFeatures', feature='alphaToOne', requires='VK_VERSION_1_0', property=None, member=None, value=None)]),
            'VK_DYNAMIC_STATE_LOGIC_OP_ENABLE_EXT': DynamicState('VK_DYNAMIC_STATE_LOGIC_OP_ENABLE_EXT', ['vkCmdSetLogicOpEnableEXT'], 'VK_SHADER_STAGE_FRAGMENT_BIT', ['Fragment Output'], False, True, None, None, [EnableState(version=None, extension=None, struct='VkPhysicalDeviceFeatures', feature='logicOp', requires='VK_VERSION_1_0', property=None, member=None, value=None)]),
            'VK_DYNAMIC_STATE_COLOR_BLEND_ENABLE_EXT': DynamicState('VK_DYNAMIC_STATE_COLOR_BLEND_ENABLE_EXT', ['vkCmdSetColorBlendEnableEXT'], 'VK_SHADER_STAGE_FRAGMENT_BIT', ['Fragment Output'], False, True, None, 'boundColorAttachment', []),
            'VK_DYNAMIC_STATE_COLOR_BLEND_EQUATION_EXT': DynamicState('VK_DYNAMIC_STATE_COLOR_BLEND_EQUATION_EXT', ['vkCmdSetColorBlendEquationEXT'], 'VK_SHADER_STAGE_FRAGMENT_BIT', ['Fragment Output'], False, True, None, 'colorBlendEnableIndex', []),
            'VK_DYNAMIC_STATE_COLOR_WRITE_MASK_EXT': DynamicState('VK_DYNAMIC_STATE_COLOR_WRITE_MASK_EXT', ['vkCmdSetColorWriteMaskEXT'], 'VK_SHADER_STAGE_FRAGMENT_BIT', ['Fragment Output'], False, True, None, 'boundColorAttachment', []),
            'VK_DYNAMIC_STATE_RASTERIZATION_STREAM_EXT': DynamicState('VK_DYNAMIC_STATE_RASTERIZATION_STREAM_EXT', ['vkCmdSetRasterizationStreamEXT'], 'VK_SHADER_STAGE_GEOMETRY_BIT', ['Pre-Rasterization Shader'], False, False, None, None, [EnableState(version=None, extension=None, struct='VkPhysicalDeviceTransformFeedbackFeaturesEXT', feature='geometryStreams', requires='VK_EXT_transform_feedback', property=None, member=None, value=None)]),
            'VK_DYNAMIC_STATE_CONSERVATIVE_RASTERIZATION_MODE_EXT': DynamicState('VK_DYNAMIC_STATE_CONSERVATIVE_RASTERIZATION_MODE_EXT', ['vkCmdSetConservativeRasterizationModeEXT'], 'VK_SHADER_STAGE_ALL', ['Pre-Rasterization Shader'], False, True, None, None, [EnableState(version=None, extension='VK_EXT_conservative_rasterization', struct=None, feature=None, requires=None, property=None, member=None, value=None)]),
            'VK_DYNAMIC_STATE_EXTRA_PRIMITIVE_OVERESTIMATION_SIZE_EXT': DynamicState('VK_DYNAMIC_STATE_EXTRA_PRIMITIVE_OVERESTIMATION_SIZE_EXT', ['vkCmdSetExtraPrimitiveOverestimationSizeEXT'], 'VK_SHADER_STAGE_ALL', ['Pre-Rasterization Shader'], False, True, None, 'conservativeRasterizationMode', [EnableState(version=None, extension='VK_EXT_conservative_rasterization', struct=None, feature=None, requires=None, property=None, member=None, value=None)]),
            'VK_DYNAMIC_STATE_DEPTH_CLIP_ENABLE_EXT': DynamicState('VK_DYNAMIC_STATE_DEPTH_CLIP_ENABLE_EXT', ['vkCmdSetDepthClipEnableEXT'], 'VK_SHADER_STAGE_ALL', ['Pre-Rasterization Shader'], False, False, None, None, [EnableState(version=None, extension=None, struct='VkPhysicalDeviceDepthClipEnableFeaturesEXT', feature='depthClipEnable', requires='VK_EXT_depth_clip_enable', property=None, member=None, value=None)]),
            'VK_DYNAMIC_STATE_SAMPLE_LOCATIONS_ENABLE_EXT': DynamicState('VK_DYNAMIC_STATE_SAMPLE_LOCATIONS_ENABLE_EXT', ['vkCmdSetSampleLocationsEnableEXT'], 'VK_SHADER_STAGE_ALL', ['Fragment Shader', 'Fragment Output'], False, True, None, None, [EnableState(version=None, extension='VK_EXT_sample_locations', struct=None, feature=None, requires=None, property=None, member=None, value=None)]),
            'VK_DYNAMIC_STATE_COLOR_BLEND_ADVANCED_EXT': DynamicState('VK_DYNAMIC_STATE_COLOR_BLEND_ADVANCED_EXT', ['vkCmdSetColorBlendAdvancedEXT'], 'VK_SHADER_STAGE_FRAGMENT_BIT', ['Fragment Output'], False, True, None, 'colorBlendEnableIndex', []),
            'VK_DYNAMIC_STATE_PROVOKING_VERTEX_MODE_EXT': DynamicState('VK_DYNAMIC_STATE_PROVOKING_VERTEX_MODE_EXT', ['vkCmdSetProvokingVertexModeEXT'], 'VK_SHADER_STAGE_VERTEX_BIT', ['Pre-Rasterization Shader'], False, True, None, None, [EnableState(version=None, extension='VK_EXT_provoking_vertex', struct=None, feature=None, requires=None, property=None, member=None, value=None)]),
            'VK_DYNAMIC_STATE_LINE_RASTERIZATION_MODE_EXT': DynamicState('VK_DYNAMIC_STATE_LINE_RASTERIZATION_MODE_EXT', ['vkCmdSetLineRasterizationModeEXT'], 'VK_SHADER_STAGE_VERTEX_BIT', ['Pre-Rasterization Shader'], False, True, None, 'lineTopology', [EnableState(version=None, extension=None, struct='VkPhysicalDeviceLineRasterizationFeaturesKHR', feature='stippledRectangularLines', requires='VK_KHR_line_rasterization', property=None, member=None, value=None), EnableState(version=None, extension=None, struct='VkPhysicalDeviceLineRasterizationFeaturesKHR', feature='stippledBresenhamLines', requires='VK_KHR_line_rasterization', property=None, member=None, value=None), EnableState(version=None, extension=None, struct='VkPhysicalDeviceLineRasterizationFeaturesKHR', feature='stippledSmoothLines', requires='VK_KHR_line_rasterization', property=None, member=None, value=None)]),
            'VK_DYNAMIC_STATE_LINE_STIPPLE_ENABLE_EXT': DynamicState('VK_DYNAMIC_STATE_LINE_STIPPLE_ENABLE_EXT', ['vkCmdSetLineStippleEnableEXT'], 'VK_SHADER_STAGE_VERTEX_BIT', ['Pre-Rasterization Shader'], False, True, None, 'lineTopology', [EnableState(version=None, extension=None, struct='VkPhysicalDeviceLineRasterizationFeaturesKHR', feature='stippledRectangularLines', requires='VK_KHR_line_rasterization', property=None, member=None, value=None), EnableState(version=None, extension=None, struct='VkPhysicalDeviceLineRasterizationFeaturesKHR', feature='stippledBresenhamLines', requires='VK_KHR_line_rasterization', property=None, member=None, value=None), EnableState(version=None, extension=None, struct='VkPhysicalDeviceLineRasterizationFeaturesKHR', feature='stippledSmoothLines', requires='VK_KHR_line_rasterization', property=None, member=None, value=None)]),
            'VK_DYNAMIC_STATE_DEPTH_CLIP_NEGATIVE_ONE_TO_ONE_EXT': DynamicState('VK_DYNAMIC_STATE_DEPTH_CLIP_NEGATIVE_ONE_TO_ONE_EXT', ['vkCmdSetDepthClipNegativeOneToOneEXT'], 'VK_SHADER_STAGE_ALL', ['Pre-Rasterization Shader'], False, False, None, None, [EnableState(version=None, extension=None, struct='VkPhysicalDeviceDepthClipControlFeaturesEXT', feature='depthClipControl', requires='VK_EXT_depth_clip_control', property=None, member=None, value=None)]),
            'VK_DYNAMIC_STATE_VIEWPORT_W_SCALING_ENABLE_NV': DynamicState('VK_DYNAMIC_STATE_VIEWPORT_W_SCALING_ENABLE_NV', ['vkCmdSetViewportWScalingEnableNV'], 'VK_SHADER_STAGE_ALL', ['Pre-Rasterization Shader'], False, False, None, None, [EnableState(version=None, extension='VK_NV_clip_space_w_scaling', struct=None, feature=None, requires=None, property=None, member=None, value=None)]),
            'VK_DYNAMIC_STATE_VIEWPORT_SWIZZLE_NV': DynamicState('VK_DYNAMIC_STATE_VIEWPORT_SWIZZLE_NV', ['vkCmdSetViewportSwizzleNV'], 'VK_SHADER_STAGE_ALL', ['Pre-Rasterization Shader'], False, False, None, None, [EnableState(version=None, extension='VK_NV_viewport_swizzle', struct=None, feature=None, requires=None, property=None, member=None, value=None)]),
            'VK_DYNAMIC_STATE_COVERAGE_TO_COLOR_ENABLE_NV': DynamicState('VK_DYNAMIC_STATE_COVERAGE_TO_COLOR_ENABLE_NV', ['vkCmdSetCoverageToColorEnableNV'], 'VK_SHADER_STAGE_FRAGMENT_BIT', ['Fragment Shader', 'Fragment Output'], False, True, None, None, [EnableState(version=None, extension='VK_NV_fragment_coverage_to_color', struct=None, feature=None, requires=None, property=None, member=None, value=None)]),
            'VK_DYNAMIC_STATE_COVERAGE_TO_COLOR_LOCATION_NV': DynamicState('VK_DYNAMIC_STATE_COVERAGE_TO_COLOR_LOCATION_NV', ['vkCmdSetCoverageToColorLocationNV'], 'VK_SHADER_STAGE_ALL', ['Fragment Shader', 'Fragment Output'], False, True, 'VK_DYNAMIC_STATE_COVERAGE_TO_COLOR_ENABLE_NV', None, [EnableState(version=None, extension='VK_NV_fragment_coverage_to_color', struct=None, feature=None, requires=None, property=None, member=None, value=None)]),
            'VK_DYNAMIC_STATE_COVERAGE_MODULATION_MODE_NV': DynamicState('VK_DYNAMIC_STATE_COVERAGE_MODULATION_MODE_NV', ['vkCmdSetCoverageModulationModeNV'], 'VK_SHADER_STAGE_ALL', ['Fragment Shader', 'Fragment Output'], False, True, None, None, [EnableState(version=None, extension='VK_NV_framebuffer_mixed_samples', struct=None, feature=None, requires=None, property=None, member=None, value=None)]),
            'VK_DYNAMIC_STATE_COVERAGE_MODULATION_TABLE_ENABLE_NV': DynamicState('VK_DYNAMIC_STATE_COVERAGE_MODULATION_TABLE_ENABLE_NV', ['vkCmdSetCoverageModulationTableEnableNV'], 'VK_SHADER_STAGE_ALL', ['Fragment Shader', 'Fragment Output'], False, True, None, 'coverageModulationMode', [EnableState(version=None, extension='VK_NV_framebuffer_mixed_samples', struct=None, feature=None, requires=None, property=None, member=None, value=None)]),
            'VK_DYNAMIC_STATE_COVERAGE_MODULATION_TABLE_NV': DynamicState('VK_DYNAMIC_STATE_COVERAGE_MODULATION_TABLE_NV', ['vkCmdSetCoverageModulationTableNV'], 'VK_SHADER_STAGE_ALL', ['Fragment Shader', 'Fragment Output'], False, True, 'VK_DYNAMIC_STATE_COVERAGE_MODULATION_TABLE_ENABLE_NV', None, [EnableState(version=None, extension='VK_NV_framebuffer_mixed_samples', struct=None, feature=None, requires=None, property=None, member=None, value=None)]),
            'VK_DYNAMIC_STATE_SHADING_RATE_IMAGE_ENABLE_NV': DynamicState('VK_DYNAMIC_STATE_SHADING_RATE_IMAGE_ENABLE_NV', ['vkCmdSetShadingRateImageEnableNV'], 'VK_SHADER_STAGE_ALL', ['Pre-Rasterization Shader'], False, True, None, None, [EnableState(version=None, extension=None, struct='VkPhysicalDeviceShadingRateImageFeaturesNV', feature='shadingRateImage', requires='VK_NV_shading_rate_image', property=None, member=None, value=None)]),
            'VK_DYNAMIC_STATE_REPRESENTATIVE_FRAGMENT_TEST_ENABLE_NV': DynamicState('VK_DYNAMIC_STATE_REPRESENTATIVE_FRAGMENT_TEST_ENABLE_NV', ['vkCmdSetRepresentativeFragmentTestEnableNV'], 'VK_SHADER_STAGE_ALL', ['Fragment Shader'], False, True, None, None, [EnableState(version=None, extension=None, struct='VkPhysicalDeviceRepresentativeFragmentTestFeaturesNV', feature='representativeFragmentTest', requires='VK_NV_representative_fragment_test', property=None, member=None, value=None)]),
            'VK_DYNAMIC_STATE_COVERAGE_REDUCTION_MODE_NV': DynamicState('VK_DYNAMIC_STATE_COVERAGE_REDUCTION_MODE_NV', ['vkCmdSetCoverageReductionModeNV'], 'VK_SHADER_STAGE_ALL', ['Fragment Shader', 'Fragment Output'], False, True, None, None, [EnableState(version=None, extension=None, struct='VkPhysicalDeviceCoverageReductionModeFeaturesNV', feature='coverageReductionMode', requires='VK_NV_coverage_reduction_mode', property=None, member=None, value=None)]),
            'VK_DYNAMIC_STATE_ATTACHMENT_FEEDBACK_LOOP_ENABLE_EXT': DynamicState('VK_DYNAMIC_STATE_ATTACHMENT_FEEDBACK_LOOP_ENABLE_EXT', ['vkCmdSetAttachmentFeedbackLoopEnableEXT'], 'VK_SHADER_STAGE_FRAGMENT_BIT', ['Pre-Rasterization Shader', 'Fragment Shader', 'Fragment Output'], False, True, None, None, [EnableState(version=None, extension=None, struct='VkPhysicalDeviceAttachmentFeedbackLoopDynamicStateFeaturesEXT', feature='attachmentFeedbackLoopDynamicState', requires='VK_EXT_attachment_feedback_loop_dynamic_state', property=None, member=None, value=None)]),
            'VK_DYNAMIC_STATE_DEPTH_CLAMP_RANGE_EXT': DynamicState('VK_DYNAMIC_STATE_DEPTH_CLAMP_RANGE_EXT', ['vkCmdSetDepthClampRangeEXT'], 'VK_SHADER_STAGE_ALL', ['Pre-Rasterization Shader'], False, False, 'VK_DYNAMIC_STATE_DEPTH_CLAMP_ENABLE_EXT', None, [EnableState(version=None, extension=None, struct='VkPhysicalDeviceDepthClampControlFeaturesEXT', feature='depthClampControl', requires='VK_EXT_depth_clamp_control', property=None, member=None, value=None)]),
            'VK_DYNAMIC_STATE_RAY_TRACING_PIPELINE_STACK_SIZE_KHR': DynamicState('VK_DYNAMIC_STATE_RAY_TRACING_PIPELINE_STACK_SIZE_KHR', ['vkCmdSetRayTracingPipelineStackSizeKHR'], 'VK_SHADER_STAGE_ALL', [], False, False, None, None, [EnableState(version=None, extension='VK_KHR_ray_tracing_pipeline', struct=None, feature=None, requires=None, property=None, member=None, value=None)]),
        }

        self.write(f'''// *** THIS FILE IS GENERATED - DO NOT EDIT ***
            // See {os.path.basename(__file__)} for modifications

            /***************************************************************************
            *
            * Copyright (c) 2023-2026 Valve Corporation
            * Copyright (c) 2023-2026 LunarG, Inc.
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
            ****************************************************************************/\n''')

        self.write('// NOLINTBEGIN') # Wrap for clang-tidy to ignore

        if self.filename == 'dynamic_state_helper.h':
            self.generateHeader()
        elif self.filename == 'dynamic_state_helper.cpp':
            self.generateSource()
        else:
            self.write(f'\nFile name {self.filename} has no code to generate\n')

        self.write('// NOLINTEND') # Wrap for clang-tidy to ignore

    def generateHeader(self):
        out = []
        out.append('''
            #pragma once
            #include <vulkan/vulkan_core.h>
            #include <bitset>

            namespace vvl {
                class Pipeline;
            }  // namespace vvl


            // Reorders VkDynamicState so it can be a bitset
            typedef enum CBDynamicState {
            ''')
        for index, field in enumerate(self.vk.enums['VkDynamicState'].fields, start=1):
            # VK_DYNAMIC_STATE_LINE_WIDTH -> STATE_LINE_WIDTH
            out.append(f'CB_DYNAMIC_{field.name[11:]} = {index},\n')

        out.append(f'CB_DYNAMIC_STATE_STATUS_NUM = {len(self.vk.enums["VkDynamicState"].fields) + 1}')
        out.append('''
            } CBDynamicState;

            using CBDynamicFlags = std::bitset<CB_DYNAMIC_STATE_STATUS_NUM>;
            VkDynamicState ConvertToDynamicState(CBDynamicState dynamic_state);
            CBDynamicState ConvertToCBDynamicState(VkDynamicState dynamic_state);
            const char* DynamicStateToString(CBDynamicState dynamic_state);
            std::string DynamicStatesToString(CBDynamicFlags const &dynamic_states);
            std::string DynamicStatesCommandsToString(CBDynamicFlags const &dynamic_states);

            std::string DescribeDynamicStateCommand(CBDynamicState dynamic_state);
            std::string DescribeDynamicStateDependency(CBDynamicState dynamic_state, const vvl::Pipeline* pipeline);

            // We build these up at code gen time to quickly use at runtime
            // TODO - tried to make these constexpr, but seems bitset over 64 bits doesn't like that it seems
            //
            ''')

        const_names = {
            "VK_SHADER_STAGE_VERTEX_BIT" : "kVertexDynamicState",
            "VK_SHADER_STAGE_FRAGMENT_BIT" : "kFragmentDynamicState",
            "VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT" : "kTessControlDynamicState",
            "VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT" : "kTessEvalDynamicState",
            "VK_SHADER_STAGE_GEOMETRY_BIT" : "kGeometryDynamicState",
        }
        stage_map = dict()
        for dynamicState in self.vk.dynamicStates.values():
            stage_map.setdefault(dynamicState.shaderStage, []).append(dynamicState.name)

        # sort so each OS generates same order here
        for stage, states in sorted(stage_map.items()):
            if stage == 'VK_SHADER_STAGE_ALL':
                continue
            out.append(f'// All state is only tied to {stage}\n')
            out.append(f'const CBDynamicFlags {const_names[stage]} =\n')
            for index, state in enumerate(states):
                seperator = ' |' if (index + 1) != len(states) else ';\n'
                out.append(f'CBDynamicFlags(1) << CB_{state[3:]}{seperator}\n')

        self.write("".join(out))

    def generateSource(self):
        out = []
        out.append('''
            #include "state_tracker/pipeline_state.h"

            VkDynamicState ConvertToDynamicState(CBDynamicState dynamic_state) {
                switch (dynamic_state) {
            ''')
        for field in self.vk.enums['VkDynamicState'].fields:
            # VK_DYNAMIC_STATE_LINE_WIDTH -> STATE_LINE_WIDTH
            out.append(f'case CB_DYNAMIC_{field.name[11:]}:\n')
            out.append(f'    return {field.name};\n')

        out.append('''
                    default:
                        return VK_DYNAMIC_STATE_MAX_ENUM;
                }
            }
            ''')

        out.append('''
            CBDynamicState ConvertToCBDynamicState(VkDynamicState dynamic_state) {
                switch (dynamic_state) {
            ''')

        for field in self.vk.enums['VkDynamicState'].fields:
            # VK_DYNAMIC_STATE_LINE_WIDTH -> STATE_LINE_WIDTH
            out.append(f'case {field.name}:\n')
            out.append(f'    return CB_DYNAMIC_{field.name[11:]};\n')
        out.append('''
                    default:
                        return CB_DYNAMIC_STATE_STATUS_NUM;
                }
            }
            ''')

        out.append('''
            const char* DynamicStateToString(CBDynamicState dynamic_state) {
                return string_VkDynamicState(ConvertToDynamicState(dynamic_state));
            }

            std::string DynamicStatesToString(CBDynamicFlags const& dynamic_states) {
                std::string ret;
                // enum is not zero based
                for (int index = 1; index < CB_DYNAMIC_STATE_STATUS_NUM; ++index) {
                    CBDynamicState status = static_cast<CBDynamicState>(index);
                    if (dynamic_states[status]) {
                        if (!ret.empty()) ret.append("|");
                        ret.append(string_VkDynamicState(ConvertToDynamicState(status)));
                    }
                }
                if (ret.empty()) ret.append("(Unknown Dynamic State)");
                return ret;
            }

            std::string DynamicStatesCommandsToString(CBDynamicFlags const& dynamic_states) {
                std::string ret;
                // enum is not zero based
                for (int index = 1; index < CB_DYNAMIC_STATE_STATUS_NUM; ++index) {
                    CBDynamicState status = static_cast<CBDynamicState>(index);
                    if (dynamic_states[status]) {
                        if (!ret.empty()) ret.append(", ");
                        ret.append(DescribeDynamicStateCommand(status));
                    }
                }
                if (ret.empty()) ret.append("(Unknown Dynamic State)");
                return ret;
            }
            ''')

        out.append('''
            std::string DescribeDynamicStateCommand(CBDynamicState dynamic_state) {
                std::ostringstream ss;
                vvl::Func func = vvl::Func::Empty;
                switch (dynamic_state) {
        ''')
        for dynamicState in self.vk.dynamicStates.values():
            out.append(f'case CB_{dynamicState.name[3:]}:\n')
            out.append(f'    func = vvl::Func::{dynamicState.commands[0]};\n')
            out.append('    break;')
        out.append('''
                    default:
                        ss << "(Unknown Dynamic State) ";
                }

                ss << String(func);

                // Currently only exception that has 2 commands that can set it
                if (dynamic_state == CB_DYNAMIC_STATE_DEPTH_BIAS) {
                    ss << " or " << String(vvl::Func::vkCmdSetDepthBias2EXT);
                }

                return ss.str();
            }
        ''')

        out.append('''
            // For anything with multple uses
            static std::string_view rasterizer_discard_enable_dynamic{"vkCmdSetRasterizerDiscardEnable last set rasterizerDiscardEnable to VK_FALSE.\\n"};
            static std::string_view rasterizer_discard_enable_static{"VkPipelineRasterizationStateCreateInfo::rasterizerDiscardEnable was VK_FALSE in the last bound graphics pipeline.\\n"};
            static std::string_view stencil_test_enable_dynamic{"vkCmdSetStencilTestEnable last set stencilTestEnable to VK_TRUE.\\n"};
            static std::string_view stencil_test_enable_static{"VkPipelineDepthStencilStateCreateInfo::stencilTestEnable was VK_TRUE in the last bound graphics pipeline.\\n"};

            std::string DescribeDynamicStateDependency(CBDynamicState dynamic_state, const vvl::Pipeline* pipeline) {
                std::ostringstream ss;
                switch (dynamic_state) {
        ''')
        for dynamicState in self.vk.dynamicStates.values():
            if dynamicState.requiresRasterization is False and dynamicState.stateRequired is None:
                continue
            out.append(f'case CB_{dynamicState.name[3:]}:')

            if dynamicState.requiresRasterization:
                out.append('''
                if (!pipeline || pipeline->IsDynamic(CB_DYNAMIC_STATE_RASTERIZER_DISCARD_ENABLE)) {
                    ss << rasterizer_discard_enable_dynamic;
                } else {
                    ss << rasterizer_discard_enable_static;
                }''')
            if dynamicState.stateRequired == 'VK_DYNAMIC_STATE_STENCIL_TEST_ENABLE':
                out.append('''
                if (!pipeline || pipeline->IsDynamic(CB_DYNAMIC_STATE_STENCIL_TEST_ENABLE)) {
                    ss << stencil_test_enable_dynamic;
                } else {
                    ss << stencil_test_enable_static;
                }''')
            if dynamicState.stateRequired == 'VK_DYNAMIC_STATE_DEPTH_TEST_ENABLE':
                out.append('''
                if (!pipeline || pipeline->IsDynamic(CB_DYNAMIC_STATE_DEPTH_TEST_ENABLE)) {
                    ss << "vkCmdSetDepthTestEnable last set depthTestEnable to VK_TRUE.\\n";
                } else {
                    ss << "VkPipelineDepthStencilStateCreateInfo::depthTestEnable was VK_TRUE in the last bound graphics pipeline.\\n";
                }''')
            if dynamicState.stateRequired == 'VK_DYNAMIC_STATE_DEPTH_BOUNDS_TEST_ENABLE':
                out.append('''
                if (!pipeline || pipeline->IsDynamic(CB_DYNAMIC_STATE_DEPTH_BOUNDS_TEST_ENABLE)) {
                    ss << "vkCmdSetDepthBoundsTestEnable last set depthBoundsTestEnable to VK_TRUE.\\n";
                } else {
                    ss << "VkPipelineDepthStencilStateCreateInfo::depthBoundsTestEnable was VK_TRUE in the last bound graphics pipeline.\\n";
                }''')
            if dynamicState.stateRequired == 'VK_DYNAMIC_STATE_DEPTH_BIAS_ENABLE':
                out.append('''
                if (!pipeline || pipeline->IsDynamic(CB_DYNAMIC_STATE_DEPTH_BIAS_ENABLE)) {
                    ss << "vkCmdSetDepthBiasEnable last set depthTestEnable to VK_TRUE.\\n";
                } else {
                    ss << "VkPipelineRasterizationStateCreateInfo::depthTestEnable was VK_TRUE in the last bound graphics pipeline.\\n";
                }''')
            if dynamicState.stateRequired == 'VK_DYNAMIC_STATE_DEPTH_CLAMP_ENABLE_EXT':
                out.append('''
                if (!pipeline || pipeline->IsDynamic(CB_DYNAMIC_STATE_DEPTH_CLAMP_ENABLE_EXT)) {
                    ss << "vkCmdSetDepthClampEnableEXT last set depthClampEnable to VK_TRUE.\\n";
                } else {
                    ss << "VkPipelineRasterizationStateCreateInfo::depthClampEnable was VK_TRUE in the last bound graphics pipeline.\\n";
                }''')
            if dynamicState.stateRequired == 'VK_DYNAMIC_STATE_LOGIC_OP_ENABLE_EXT':
                out.append('''
                if (!pipeline || pipeline->IsDynamic(CB_DYNAMIC_STATE_LOGIC_OP_ENABLE_EXT)) {
                    ss << "vkCmdSetLogicOpEnableEXT last set logicOpEnable to VK_TRUE.\\n";
                } else {
                    ss << "VkPipelineColorBlendStateCreateInfo::logicOpEnable was VK_TRUE in the last bound graphics pipeline.\\n";
                }''')
            if dynamicState.stateRequired == 'VK_DYNAMIC_STATE_LINE_STIPPLE_ENABLE_EXT':
                out.append('''
                if (!pipeline || pipeline->IsDynamic(CB_DYNAMIC_STATE_LINE_STIPPLE_ENABLE_EXT)) {
                    ss << "vkCmdSetLineStippleEnableEXT last set stippledLineEnable to VK_TRUE.\\n";
                } else {
                    ss << "VkPipelineRasterizationLineStateCreateInfo::stippledLineEnable was VK_TRUE in the last bound graphics pipeline.\\n";
                }''')
            if dynamicState.stateRequired == 'VK_DYNAMIC_STATE_SAMPLE_LOCATIONS_ENABLE_EXT':
                out.append('''
                if (!pipeline || pipeline->IsDynamic(CB_DYNAMIC_STATE_SAMPLE_LOCATIONS_ENABLE_EXT)) {
                    ss << "vkCmdSetSampleLocationsEnableEXT last set sampleLocationsEnable to VK_TRUE.\\n";
                } else {
                    ss << "VkPipelineMultisampleStateCreateInfo::pNext->VkPipelineSampleLocationsStateCreateInfoEXT::sampleLocationsEnable was VK_TRUE in the last bound graphics pipeline.\\n";
                }''')
            if dynamicState.stateRequired == 'VK_DYNAMIC_STATE_COVERAGE_MODULATION_TABLE_ENABLE_NV':
                out.append('''
                if (!pipeline || pipeline->IsDynamic(CB_DYNAMIC_STATE_COVERAGE_MODULATION_TABLE_ENABLE_NV)) {
                    ss << "vkCmdSetCoverageModulationTableEnableNV last set coverageModulationTableEnable to VK_TRUE.\\n";
                } else {
                    ss << "VkPipelineMultisampleStateCreateInfo::pNext->VkPipelineCoverageModulationStateCreateInfoNV::coverageModulationTableEnable was VK_TRUE in the last bound graphics pipeline.\\n";
                }''')
            if dynamicState.stateRequired == 'VK_DYNAMIC_STATE_SHADING_RATE_IMAGE_ENABLE_NV':
                out.append('''
                if (!pipeline || pipeline->IsDynamic(CB_DYNAMIC_STATE_SHADING_RATE_IMAGE_ENABLE_NV)) {
                    ss << "vkCmdSetShadingRateImageEnableNV last set shadingRateImageEnable to VK_TRUE.\\n";
                } else {
                    ss << "VkPipelineViewportStateCreateInfo::pNext->VkPipelineViewportShadingRateImageStateCreateInfoNV::shadingRateImageEnable was VK_TRUE in the last bound graphics pipeline.\\n";
                }''')
            if dynamicState.stateRequired == 'VK_DYNAMIC_STATE_VIEWPORT_W_SCALING_ENABLE_NV':
                out.append('''
                if (!pipeline || pipeline->IsDynamic(CB_DYNAMIC_STATE_VIEWPORT_W_SCALING_ENABLE_NV)) {
                    ss << "vkCmdSetViewportWScalingEnableNV last set viewportWScalingEnable to VK_TRUE.\\n";
                } else {
                    ss << "VkPipelineViewportStateCreateInfo::pNext->VkPipelineViewportWScalingStateCreateInfoNV::viewportWScalingEnable was VK_TRUE in the last bound graphics pipeline.\\n";
                }''')
            if dynamicState.stateRequired == 'VK_DYNAMIC_STATE_DISCARD_RECTANGLE_ENABLE_EXT':
                out.append('''
                if (!pipeline || pipeline->IsDynamic(CB_DYNAMIC_STATE_DISCARD_RECTANGLE_ENABLE_EXT)) {
                    ss << "vkCmdSetDiscardRectangleEnableEXT last set discardRectangleEnable to VK_TRUE.\\n";
                } else {
                    ss << "VkGraphicsPipelineCreateInfo::pNext->VkPipelineDiscardRectangleStateCreateInfoEXT::discardRectangleCount was greater than zero in the last bound graphics pipeline.\\n";
                }''')

            out.append('    break;')
        out.append('''
                    default:
                        break; // not all state will be dependent on other state
                }

                return ss.str();
            }
        ''')

        self.write("".join(out))