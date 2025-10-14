#!/usr/bin/python3 -i
#
# Copyright (c) 2023-2025 The Khronos Group Inc.
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

# TODO - Get in vk.xml
# This is a representation of all the dynamic state information
dynamic_state_map = {
    "VK_DYNAMIC_STATE_VIEWPORT" : {
        "command" : ["vkCmdSetViewport"],
        "stage" : "VK_SHADER_STAGE_ALL",
        "gpl" : ["PreRasterization"],
        "pipeline" : "VkPipelineViewportStateCreateInfo",
    },
    "VK_DYNAMIC_STATE_SCISSOR" : {
        "command" : ["vkCmdSetScissor"],
        "stage" : "VK_SHADER_STAGE_ALL",
        "gpl" : ["PreRasterization"],
        "pipeline" : "VkPipelineViewportStateCreateInfo",
    },
    "VK_DYNAMIC_STATE_LINE_WIDTH" : {
        "command" : ["vkCmdSetLineWidth"],
        "stage" : "VK_SHADER_STAGE_ALL",
        "gpl" : ["PreRasterization"],
        "pipeline" : "VkPipelineRasterizationStateCreateInfo",
    },
    "VK_DYNAMIC_STATE_DEPTH_BIAS" : {
        "command" : ["vkCmdSetDepthBias", "vkCmdSetDepthBias2EXT"],
        "dependency" : ["rasterizerDiscardEnable", "depthBiasEnable"],
        "stage" : "VK_SHADER_STAGE_ALL",
        "gpl" : ["PreRasterization"],
        "pipeline" : "VkPipelineRasterizationStateCreateInfo",
    },
    "VK_DYNAMIC_STATE_BLEND_CONSTANTS" : {
        "command" : ["vkCmdSetBlendConstants"],
        "dependency" : ["rasterizerDiscardEnable"],
        "stage" : "VK_SHADER_STAGE_FRAGMENT_BIT",
        "gpl" : ["FragmentOutput"],
        "pipeline" : "VkPipelineColorBlendStateCreateInfo",
    },
    "VK_DYNAMIC_STATE_DEPTH_BOUNDS" : {
        "command" : ["vkCmdSetDepthBounds"],
        "dependency" : ["rasterizerDiscardEnable", "depthBoundsTestEnable"],
        "stage" : "VK_SHADER_STAGE_ALL",
        "gpl" : ["FragmentShader"],
        "pipeline" : "VkPipelineDepthStencilStateCreateInfo",
    },
    "VK_DYNAMIC_STATE_STENCIL_COMPARE_MASK" : {
        "command" : ["vkCmdSetStencilCompareMask"],
        "dependency" : ["rasterizerDiscardEnable", "stencilTestEnable"],
        "stage" : "VK_SHADER_STAGE_ALL",
        "gpl" : ["FragmentShader"],
        "pipeline" : "VkPipelineDepthStencilStateCreateInfo",
    },
    "VK_DYNAMIC_STATE_STENCIL_WRITE_MASK" : {
        "command" : ["vkCmdSetStencilWriteMask"],
        "dependency" : ["rasterizerDiscardEnable", "stencilTestEnable"],
        "stage" : "VK_SHADER_STAGE_ALL",
        "gpl" : ["FragmentShader"],
        "pipeline" : "VkPipelineDepthStencilStateCreateInfo",
    },
    "VK_DYNAMIC_STATE_STENCIL_REFERENCE" : {
        "command" : ["vkCmdSetStencilReference"],
        "dependency" : ["rasterizerDiscardEnable", "stencilTestEnable"],
        "stage" : "VK_SHADER_STAGE_ALL",
        "gpl" : ["FragmentShader"],
        "pipeline" : "VkPipelineDepthStencilStateCreateInfo",
    },
    "VK_DYNAMIC_STATE_CULL_MODE" : {
        "command" : ["vkCmdSetCullMode"],
        "dependency" : ["rasterizerDiscardEnable"],
        "stage" : "VK_SHADER_STAGE_ALL",
        "gpl" : ["PreRasterization"],
        "pipeline" : "VkPipelineRasterizationStateCreateInfo",
    },
    "VK_DYNAMIC_STATE_FRONT_FACE" : {
        "command" : ["vkCmdSetFrontFace"],
        "dependency" : ["rasterizerDiscardEnable"],
        "stage" : "VK_SHADER_STAGE_ALL",
        "gpl" : ["PreRasterization"],
        "pipeline" : "VkPipelineRasterizationStateCreateInfo",
    },
    "VK_DYNAMIC_STATE_PRIMITIVE_TOPOLOGY" : {
        "command" : ["vkCmdSetPrimitiveTopology"],
        "stage" : "VK_SHADER_STAGE_VERTEX_BIT",
        "gpl" : ["VertexInput"],
        "pipeline" : "VkPipelineInputAssemblyStateCreateInfo",
    },
    "VK_DYNAMIC_STATE_VIEWPORT_WITH_COUNT" : {
        "command" : ["vkCmdSetViewportWithCount"],
        "stage" : "VK_SHADER_STAGE_ALL",
        "gpl" : ["PreRasterization"],
        "pipeline" : "VkPipelineViewportStateCreateInfo",
    },
    "VK_DYNAMIC_STATE_SCISSOR_WITH_COUNT" : {
        "command" : ["vkCmdSetScissorWithCount"],
        "stage" : "VK_SHADER_STAGE_ALL",
        "gpl" : ["PreRasterization"],
        "pipeline" : "VkPipelineViewportStateCreateInfo",
    },
    "VK_DYNAMIC_STATE_VERTEX_INPUT_BINDING_STRIDE" : {
        "command" : ["vkCmdBindVertexBuffers2"],
        "stage" : "VK_SHADER_STAGE_VERTEX_BIT",
        "gpl" : ["VertexInput"],
        "pipeline" : "VkPipelineVertexInputStateCreateInfo",
    },
    "VK_DYNAMIC_STATE_DEPTH_TEST_ENABLE" : {
        "command" : ["vkCmdSetDepthTestEnable"],
        "dependency" : ["rasterizerDiscardEnable"],
        "stage" : "VK_SHADER_STAGE_ALL",
        "gpl" : ["FragmentShader"],
        "pipeline" : "VkPipelineDepthStencilStateCreateInfo",
    },
    "VK_DYNAMIC_STATE_DEPTH_WRITE_ENABLE" : {
        "command" : ["vkCmdSetDepthWriteEnable"],
        "dependency" : ["rasterizerDiscardEnable", "depthTestEnable"],
        "stage" : "VK_SHADER_STAGE_ALL",
        "gpl" : ["FragmentShader"],
        "pipeline" : "VkPipelineDepthStencilStateCreateInfo",
    },
    "VK_DYNAMIC_STATE_DEPTH_COMPARE_OP" : {
        "command" : ["vkCmdSetDepthCompareOp"],
        "dependency" : ["rasterizerDiscardEnable", "depthTestEnable"],
        "stage" : "VK_SHADER_STAGE_ALL",
        "gpl" : ["FragmentShader"],
        "pipeline" : "VkPipelineDepthStencilStateCreateInfo",
    },
    "VK_DYNAMIC_STATE_DEPTH_BOUNDS_TEST_ENABLE" : {
        "command" : ["vkCmdSetDepthBoundsTestEnable"],
        "dependency" : ["rasterizerDiscardEnable"],
        "stage" : "VK_SHADER_STAGE_ALL",
        "gpl" : ["FragmentShader"],
        "pipeline" : "VkPipelineDepthStencilStateCreateInfo",
    },
    "VK_DYNAMIC_STATE_STENCIL_TEST_ENABLE" : {
        "command" : ["vkCmdSetStencilTestEnable"],
        "dependency" : ["rasterizerDiscardEnable"],
        "stage" : "VK_SHADER_STAGE_ALL",
        "gpl" : ["FragmentShader"],
        "pipeline" : "VkPipelineDepthStencilStateCreateInfo",
    },
    "VK_DYNAMIC_STATE_STENCIL_OP" : {
        "command" : ["vkCmdSetStencilOp"],
        "dependency" : ["rasterizerDiscardEnable", "stencilTestEnable"],
        "stage" : "VK_SHADER_STAGE_ALL",
        "gpl" : ["FragmentShader"],
        "pipeline" : "VkPipelineDepthStencilStateCreateInfo",
    },
    "VK_DYNAMIC_STATE_RASTERIZER_DISCARD_ENABLE" : {
        "command" : ["vkCmdSetRasterizerDiscardEnable"],
        "stage" : "VK_SHADER_STAGE_ALL",
        "gpl" : ["PreRasterization"],
        "pipeline" : "VkPipelineRasterizationStateCreateInfo",
    },
    "VK_DYNAMIC_STATE_DEPTH_BIAS_ENABLE" : {
        "command" : ["vkCmdSetDepthBiasEnable"],
        "dependency" : ["rasterizerDiscardEnable"],
        "stage" : "VK_SHADER_STAGE_ALL",
        "gpl" : ["PreRasterization"],
        "pipeline" : "VkPipelineRasterizationStateCreateInfo",
    },
    "VK_DYNAMIC_STATE_PRIMITIVE_RESTART_ENABLE" : {
        "command" : ["vkCmdSetPrimitiveRestartEnable"],
        "stage" : "VK_SHADER_STAGE_VERTEX_BIT",
        "gpl" : ["VertexInput"],
        "pipeline" : "VkPipelineInputAssemblyStateCreateInfo",
    },
    "VK_DYNAMIC_STATE_VIEWPORT_W_SCALING_NV" : {
        "command" : ["vkCmdSetViewportWScalingNV"],
        "dependency" : ["viewportWScalingEnable"],
        "stage" : "VK_SHADER_STAGE_ALL",
        "gpl" : ["PreRasterization"],
        "pipeline" : "VkPipelineViewportWScalingStateCreateInfoNV",
    },
    "VK_DYNAMIC_STATE_DISCARD_RECTANGLE_EXT" : {
        "command" : ["vkCmdSetDiscardRectangleEXT"],
        "dependency" : ["rasterizerDiscardEnable", "discardRectangleEnable"],
        "stage" : "VK_SHADER_STAGE_ALL",
        "gpl" : ["PreRasterization"],
        "pipeline" : "VkPipelineDiscardRectangleStateCreateInfoEXT",
    },
    "VK_DYNAMIC_STATE_DISCARD_RECTANGLE_ENABLE_EXT" : {
        "command" : ["vkCmdSetDiscardRectangleEnableEXT"],
        "dependency" : ["rasterizerDiscardEnable"],
        "stage" : "VK_SHADER_STAGE_ALL",
        "gpl" : ["PreRasterization"],
        "pipeline" : "VkPipelineDiscardRectangleStateCreateInfoEXT",
    },
    "VK_DYNAMIC_STATE_DISCARD_RECTANGLE_MODE_EXT" : {
        "command" : ["vkCmdSetDiscardRectangleModeEXT"],
        "dependency" : ["rasterizerDiscardEnable", "discardRectangleEnable"],
        "stage" : "VK_SHADER_STAGE_ALL",
        "gpl" : ["PreRasterization"],
        "pipeline" : "VkPipelineDiscardRectangleStateCreateInfoEXT",
    },
    "VK_DYNAMIC_STATE_SAMPLE_LOCATIONS_EXT" : {
        "command" : ["vkCmdSetSampleLocationsEXT"],
        "dependency" : ["rasterizerDiscardEnable", "sampleLocationsEnable"],
        "stage" : "VK_SHADER_STAGE_ALL",
        "gpl" : ["FragmentShader", "FragmentOutput"],
        "pipeline" : "VkPipelineSampleLocationsStateCreateInfoEXT",
    },
    "VK_DYNAMIC_STATE_VIEWPORT_SHADING_RATE_PALETTE_NV" : {
        "command" : ["vkCmdSetViewportShadingRatePaletteNV"],
        "dependency" : ["rasterizerDiscardEnable", "shadingRateImageEnable"],
        "stage" : "VK_SHADER_STAGE_ALL",
        "gpl" : ["PreRasterization"],
        "pipeline" : "VkPipelineViewportShadingRateImageStateCreateInfoNV",
    },
    "VK_DYNAMIC_STATE_VIEWPORT_COARSE_SAMPLE_ORDER_NV" : {
        "command" : ["vkCmdSetCoarseSampleOrderNV"],
        "dependency" : ["rasterizerDiscardEnable"],
        "stage" : "VK_SHADER_STAGE_ALL",
        "gpl" : ["PreRasterization"],
        "pipeline" : "VkPipelineViewportCoarseSampleOrderStateCreateInfoNV",
    },
    "VK_DYNAMIC_STATE_EXCLUSIVE_SCISSOR_ENABLE_NV" : {
        "command" : ["vkCmdSetExclusiveScissorEnableNV"],
        "stage" : "VK_SHADER_STAGE_ALL",
        "gpl" : ["PreRasterization"],
        "pipeline" : "VkPipelineViewportExclusiveScissorStateCreateInfoNV",
    },
    "VK_DYNAMIC_STATE_EXCLUSIVE_SCISSOR_NV" : {
        "command" : ["vkCmdSetExclusiveScissorNV"],
        "dependency" : ["pExclusiveScissorEnables"],
        "stage" : "VK_SHADER_STAGE_ALL",
        "gpl" : ["PreRasterization"],
        "pipeline" : "VkPipelineViewportExclusiveScissorStateCreateInfoNV",
    },
    "VK_DYNAMIC_STATE_FRAGMENT_SHADING_RATE_KHR" : {
        "command" : ["vkCmdSetFragmentShadingRateKHR"],
        "dependency" : ["rasterizerDiscardEnable"],
        "stage" : "VK_SHADER_STAGE_ALL",
        "gpl" : ["PreRasterization", "FragmentShader"],
        "pipeline" : "VkPipelineFragmentShadingRateStateCreateInfoKHR",
    },
    "VK_DYNAMIC_STATE_LINE_STIPPLE" : {
        "command" : ["vkCmdSetLineStipple"],
        "dependency" : ["rasterizerDiscardEnable", "stippledLineEnable"],
        "stage" : "VK_SHADER_STAGE_ALL",
        "gpl" : ["PreRasterization"],
        "pipeline" : "VkPipelineRasterizationLineStateCreateInfoKHR",
    },
    "VK_DYNAMIC_STATE_VERTEX_INPUT_EXT" : {
        "command" : ["vkCmdSetVertexInputEXT"],
        "stage" : "VK_SHADER_STAGE_VERTEX_BIT",
        "gpl" : ["VertexInput"],
        "pipeline" : "VkPipelineVertexInputStateCreateInfo",
    },
    "VK_DYNAMIC_STATE_PATCH_CONTROL_POINTS_EXT" : {
        "command" : ["vkCmdSetPatchControlPointsEXT"],
        "stage" : "VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT",
        "gpl" : ["PreRasterization"],
        "pipeline" : "VkPipelineTessellationStateCreateInfo",
    },
    "VK_DYNAMIC_STATE_LOGIC_OP_EXT" : {
        "command" : ["vkCmdSetLogicOpEXT"],
        "dependency" : ["rasterizerDiscardEnable", "logicOpEnable"],
        "stage" : "VK_SHADER_STAGE_FRAGMENT_BIT",
        "gpl" : ["FragmentOutput"],
        "pipeline" : "VkPipelineColorBlendStateCreateInfo",
    },
    "VK_DYNAMIC_STATE_COLOR_WRITE_ENABLE_EXT" : {
        "command" : ["vkCmdSetColorWriteEnableEXT"],
        "dependency" : ["rasterizerDiscardEnable"],
        "stage" : "VK_SHADER_STAGE_FRAGMENT_BIT",
        "gpl" : ["FragmentOutput"],
        "pipeline" : "VkPipelineColorWriteCreateInfoEXT",
    },
    "VK_DYNAMIC_STATE_TESSELLATION_DOMAIN_ORIGIN_EXT" : {
        "command" : ["vkCmdSetTessellationDomainOriginEXT"],
        "stage" : "VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT",
        "gpl" : ["PreRasterization"],
        "pipeline" : "VkPipelineTessellationDomainOriginStateCreateInfo",
    },
    "VK_DYNAMIC_STATE_DEPTH_CLAMP_ENABLE_EXT" : {
        "command" : ["vkCmdSetDepthClampEnableEXT"],
        "stage" : "VK_SHADER_STAGE_ALL",
        "gpl" : ["PreRasterization"],
        "pipeline" : "VkPipelineRasterizationStateCreateInfo",
    },
    "VK_DYNAMIC_STATE_POLYGON_MODE_EXT" : {
        "command" : ["vkCmdSetPolygonModeEXT"],
        "stage" : "VK_SHADER_STAGE_ALL",
        "gpl" : ["PreRasterization"],
        "pipeline" : "VkPipelineRasterizationStateCreateInfo",
    },
    "VK_DYNAMIC_STATE_RASTERIZATION_SAMPLES_EXT" : {
        "command" : ["vkCmdSetRasterizationSamplesEXT"],
        "stage" : "VK_SHADER_STAGE_ALL",
        "gpl" : ["FragmentShader", "FragmentOutput"],
        "pipeline" : "VkPipelineMultisampleStateCreateInfo",
    },
    "VK_DYNAMIC_STATE_SAMPLE_MASK_EXT" : {
        "command" : ["vkCmdSetSampleMaskEXT"],
        "stage" : "VK_SHADER_STAGE_ALL",
        "gpl" : ["FragmentShader", "FragmentOutput"],
        "pipeline" : "VkPipelineMultisampleStateCreateInfo",
    },
    "VK_DYNAMIC_STATE_ALPHA_TO_COVERAGE_ENABLE_EXT" : {
        "command" : ["vkCmdSetAlphaToCoverageEnableEXT"],
        "dependency" : ["rasterizerDiscardEnable"],
        "stage" : "VK_SHADER_STAGE_ALL",
        "gpl" : ["FragmentShader", "FragmentOutput"],
        "pipeline" : "VkPipelineMultisampleStateCreateInfo",
    },
    "VK_DYNAMIC_STATE_ALPHA_TO_ONE_ENABLE_EXT" : {
        "command" : ["vkCmdSetAlphaToOneEnableEXT"],
        "dependency" : ["rasterizerDiscardEnable"],
        "stage" : "VK_SHADER_STAGE_ALL",
        "gpl" : ["FragmentShader", "FragmentOutput"],
        "pipeline" : "VkPipelineMultisampleStateCreateInfo",
    },
    "VK_DYNAMIC_STATE_LOGIC_OP_ENABLE_EXT" : {
        "command" : ["vkCmdSetLogicOpEnableEXT"],
        "dependency" : ["rasterizerDiscardEnable"],
        "stage" : "VK_SHADER_STAGE_FRAGMENT_BIT",
        "gpl" : ["FragmentOutput"],
        "pipeline" : "VkPipelineColorBlendStateCreateInfo",
    },
    "VK_DYNAMIC_STATE_COLOR_BLEND_ENABLE_EXT" : {
        "command" : ["vkCmdSetColorBlendEnableEXT"],
        "dependency" : ["rasterizerDiscardEnable"],
        "stage" : "VK_SHADER_STAGE_FRAGMENT_BIT",
        "gpl" : ["FragmentOutput"],
        "pipeline" : "VkPipelineColorBlendAttachmentState",
    },
    "VK_DYNAMIC_STATE_COLOR_BLEND_EQUATION_EXT" : {
        "command" : ["vkCmdSetColorBlendEquationEXT"],
        "dependency" : ["rasterizerDiscardEnable"],
        "stage" : "VK_SHADER_STAGE_FRAGMENT_BIT",
        "gpl" : ["FragmentOutput"],
        "pipeline" : "VkPipelineColorBlendAttachmentState",
    },
    "VK_DYNAMIC_STATE_COLOR_WRITE_MASK_EXT" : {
        "command" : ["vkCmdSetColorWriteMaskEXT"],
        "dependency" : ["rasterizerDiscardEnable"],
        "stage" : "VK_SHADER_STAGE_FRAGMENT_BIT",
        "gpl" : ["FragmentOutput"],
        "pipeline" : "VkPipelineColorBlendAttachmentState",
    },
    "VK_DYNAMIC_STATE_RASTERIZATION_STREAM_EXT" : {
        "command" : ["vkCmdSetRasterizationStreamEXT"],
        "stage" : "VK_SHADER_STAGE_GEOMETRY_BIT",
        "gpl" : ["PreRasterization"],
        "pipeline" : "VkPipelineRasterizationStateStreamCreateInfoEXT",
    },
    "VK_DYNAMIC_STATE_CONSERVATIVE_RASTERIZATION_MODE_EXT" : {
        "command" : ["vkCmdSetConservativeRasterizationModeEXT"],
        "dependency" : ["rasterizerDiscardEnable"],
        "stage" : "VK_SHADER_STAGE_ALL",
        "gpl" : ["PreRasterization"],
        "pipeline" : "VkPipelineRasterizationConservativeStateCreateInfoEXT",
    },
    "VK_DYNAMIC_STATE_EXTRA_PRIMITIVE_OVERESTIMATION_SIZE_EXT" : {
        "command" : ["vkCmdSetExtraPrimitiveOverestimationSizeEXT"],
        "dependency" : ["rasterizerDiscardEnable"],
        "stage" : "VK_SHADER_STAGE_ALL",
        "gpl" : ["PreRasterization"],
        "pipeline" : "VkPipelineRasterizationConservativeStateCreateInfoEXT",
    },
    "VK_DYNAMIC_STATE_DEPTH_CLIP_ENABLE_EXT" : {
        "command" : ["vkCmdSetDepthClipEnableEXT"],
        "stage" : "VK_SHADER_STAGE_ALL",
        "gpl" : ["PreRasterization"],
        "pipeline" : "VkPipelineRasterizationDepthClipStateCreateInfoEXT",
    },
    "VK_DYNAMIC_STATE_SAMPLE_LOCATIONS_ENABLE_EXT" : {
        "command" : ["vkCmdSetSampleLocationsEnableEXT"],
        "dependency" : ["rasterizerDiscardEnable"],
        "stage" : "VK_SHADER_STAGE_ALL",
        "gpl" : ["FragmentShader", "FragmentOutput"],
        "pipeline" : "VkPipelineSampleLocationsStateCreateInfoEXT",
    },
    "VK_DYNAMIC_STATE_COLOR_BLEND_ADVANCED_EXT" : {
        "command" : ["vkCmdSetColorBlendAdvancedEXT"],
        "dependency" : ["rasterizerDiscardEnable"],
        "stage" : "VK_SHADER_STAGE_FRAGMENT_BIT",
        "gpl" : ["FragmentOutput"],
        "pipeline" : "VkPipelineColorBlendAdvancedStateCreateInfoEXT",
    },
    "VK_DYNAMIC_STATE_PROVOKING_VERTEX_MODE_EXT" : {
        "command" : ["vkCmdSetProvokingVertexModeEXT"],
        "stage" : "VK_SHADER_STAGE_VERTEX_BIT",
        "gpl" : ["PreRasterization"],
        "pipeline" : "VkPipelineRasterizationProvokingVertexStateCreateInfoEXT",
    },
    "VK_DYNAMIC_STATE_LINE_RASTERIZATION_MODE_EXT" : {
        "command" : ["vkCmdSetLineRasterizationModeEXT"],
        "dependency" : ["rasterizerDiscardEnable"],
        "stage" : "VK_SHADER_STAGE_VERTEX_BIT",
        "gpl" : ["PreRasterization"],
        "pipeline" : "VkPipelineRasterizationLineStateCreateInfoEXT",
    },
    "VK_DYNAMIC_STATE_LINE_STIPPLE_ENABLE_EXT" : {
        "command" : ["vkCmdSetLineStippleEnableEXT"],
        "dependency" : ["rasterizerDiscardEnable"],
        "stage" : "VK_SHADER_STAGE_VERTEX_BIT",
        "gpl" : ["PreRasterization"],
        "pipeline" : "VkPipelineRasterizationLineStateCreateInfoEXT",
    },
    "VK_DYNAMIC_STATE_DEPTH_CLIP_NEGATIVE_ONE_TO_ONE_EXT" : {
        "command" : ["vkCmdSetDepthClipNegativeOneToOneEXT"],
        "stage" : "VK_SHADER_STAGE_ALL",
        "gpl" : ["PreRasterization"],
        "pipeline" : "VkPipelineViewportDepthClipControlCreateInfoEXT",
    },
    "VK_DYNAMIC_STATE_VIEWPORT_W_SCALING_ENABLE_NV" : {
        "command" : ["vkCmdSetViewportWScalingEnableNV"],
        "stage" : "VK_SHADER_STAGE_ALL",
        "gpl" : ["PreRasterization"],
        "pipeline" : "VkPipelineViewportWScalingStateCreateInfoNV",
    },
    "VK_DYNAMIC_STATE_VIEWPORT_SWIZZLE_NV" : {
        "command" : ["vkCmdSetViewportSwizzleNV"],
        "stage" : "VK_SHADER_STAGE_ALL",
        "gpl" : ["PreRasterization"],
        "pipeline" : "VkPipelineViewportSwizzleStateCreateInfoNV",
    },
    "VK_DYNAMIC_STATE_COVERAGE_TO_COLOR_ENABLE_NV" : {
        "command" : ["vkCmdSetCoverageToColorEnableNV"],
        "dependency" : ["rasterizerDiscardEnable"],
        "stage" : "VK_SHADER_STAGE_FRAGMENT_BIT",
        "gpl" : ["FragmentShader", "FragmentOutput"],
        "pipeline" : "VkPipelineCoverageToColorStateCreateInfoNV",
    },
    "VK_DYNAMIC_STATE_COVERAGE_TO_COLOR_LOCATION_NV" : {
        "command" : ["vkCmdSetCoverageToColorLocationNV"],
        "dependency" : ["rasterizerDiscardEnable"],
        "stage" : "VK_SHADER_STAGE_ALL",
        "gpl" : ["FragmentShader", "FragmentOutput"],
        "pipeline" : "VkPipelineCoverageToColorStateCreateInfoNV",
    },
    "VK_DYNAMIC_STATE_COVERAGE_MODULATION_MODE_NV" : {
        "command" : ["vkCmdSetCoverageModulationModeNV"],
        "dependency" : ["rasterizerDiscardEnable"],
        "stage" : "VK_SHADER_STAGE_ALL",
        "gpl" : ["FragmentShader", "FragmentOutput"],
        "pipeline" : "VkPipelineCoverageModulationStateCreateInfoNV",
    },
    "VK_DYNAMIC_STATE_COVERAGE_MODULATION_TABLE_ENABLE_NV" : {
        "command" : ["vkCmdSetCoverageModulationTableEnableNV"],
        "dependency" : ["rasterizerDiscardEnable"],
        "stage" : "VK_SHADER_STAGE_ALL",
        "gpl" : ["FragmentShader", "FragmentOutput"],
        "pipeline" : "VkPipelineCoverageModulationStateCreateInfoNV",
    },
    "VK_DYNAMIC_STATE_COVERAGE_MODULATION_TABLE_NV" : {
        "command" : ["vkCmdSetCoverageModulationTableNV"],
        "dependency" : ["rasterizerDiscardEnable", "coverageModulationTableEnable"],
        "stage" : "VK_SHADER_STAGE_ALL",
        "gpl" : ["FragmentShader", "FragmentOutput"],
        "pipeline" : "VkPipelineCoverageModulationStateCreateInfoNV",
    },
    "VK_DYNAMIC_STATE_SHADING_RATE_IMAGE_ENABLE_NV" : {
        "command" : ["vkCmdSetShadingRateImageEnableNV"],
        "stage" : "VK_SHADER_STAGE_ALL",
        "gpl" : ["PreRasterization"],
        "pipeline" : "VkPipelineViewportShadingRateImageStateCreateInfoNV",
    },
    "VK_DYNAMIC_STATE_REPRESENTATIVE_FRAGMENT_TEST_ENABLE_NV" : {
        "command" : ["vkCmdSetRepresentativeFragmentTestEnableNV"],
        "stage" : "VK_SHADER_STAGE_ALL",
        "gpl" : ["FragmentShader"],
        "pipeline" : "VkPipelineRepresentativeFragmentTestStateCreateInfoNV",
    },
    "VK_DYNAMIC_STATE_COVERAGE_REDUCTION_MODE_NV" : {
        "command" : ["vkCmdSetCoverageReductionModeNV"],
        "stage" : "VK_SHADER_STAGE_ALL",
        "gpl" : ["FragmentShader", "FragmentOutput"],
        "pipeline" : "VkPipelineCoverageReductionStateCreateInfoNV",
    },
    "VK_DYNAMIC_STATE_ATTACHMENT_FEEDBACK_LOOP_ENABLE_EXT" : {
        "command" : ["vkCmdSetAttachmentFeedbackLoopEnableEXT"],
        "stage" : "VK_SHADER_STAGE_FRAGMENT_BIT",
        "gpl" : ["PreRasterization", "FragmentShader", "FragmentOutput"],
    },
    "VK_DYNAMIC_STATE_RAY_TRACING_PIPELINE_STACK_SIZE_KHR" : {
        "command" : ["vkCmdSetRayTracingPipelineStackSizeKHR"],
        "stage" : "VK_SHADER_STAGE_ALL",
    },
    "VK_DYNAMIC_STATE_DEPTH_CLAMP_RANGE_EXT" : {
        "command" : ["vkCmdSetDepthClampRangeEXT"],
        "dependency" : ["depthClampEnable"],
        "stage" : "VK_SHADER_STAGE_ALL",
        "gpl" : ["PreRasterization"],
        "pipeline" : "VkPipelineViewportDepthClampControlCreateInfoEXT"
    },
}

#
# DynamicStateOutputGenerator - Generate SPIR-V validation
# for SPIR-V extensions and capabilities
class DynamicStateOutputGenerator(BaseGenerator):
    def __init__(self):
        BaseGenerator.__init__(self)

    def generate(self):
        self.write(f'''// *** THIS FILE IS GENERATED - DO NOT EDIT ***
            // See {os.path.basename(__file__)} for modifications

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
        for key, value in dynamic_state_map.items():
            stage_map.setdefault(value["stage"], []).append(key)

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
                std::stringstream ss;
                vvl::Func func = vvl::Func::Empty;
                switch (dynamic_state) {
        ''')
        for key, value in dynamic_state_map.items():
            out.append(f'case CB_{key[3:]}:\n')
            out.append(f'    func = vvl::Func::{value["command"][0]};\n')
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
                std::stringstream ss;
                switch (dynamic_state) {
        ''')
        for key, value in dynamic_state_map.items():
            if 'dependency' not in value:
                continue
            out.append(f'case CB_{key[3:]}:')

            dependency = value['dependency']
            # TODO - Use XML to generate this
            if 'rasterizerDiscardEnable' in dependency:
                out.append('''
                if (!pipeline || pipeline->IsDynamic(CB_DYNAMIC_STATE_RASTERIZER_DISCARD_ENABLE)) {
                    ss << rasterizer_discard_enable_dynamic;
                } else {
                    ss << rasterizer_discard_enable_static;
                }''')
            if 'stencilTestEnable' in dependency:
                out.append('''
                if (!pipeline || pipeline->IsDynamic(CB_DYNAMIC_STATE_STENCIL_TEST_ENABLE)) {
                    ss << stencil_test_enable_dynamic;
                } else {
                    ss << stencil_test_enable_static;
                }''')
            if 'depthTestEnable' in dependency:
                out.append('''
                if (!pipeline || pipeline->IsDynamic(CB_DYNAMIC_STATE_DEPTH_TEST_ENABLE)) {
                    ss << "vkCmdSetDepthTestEnable last set depthTestEnable to VK_TRUE.\\n";
                } else {
                    ss << "VkPipelineDepthStencilStateCreateInfo::depthTestEnable was VK_TRUE in the last bound graphics pipeline.\\n";
                }''')
            if 'depthBoundsTestEnable' in dependency:
                out.append('''
                if (!pipeline || pipeline->IsDynamic(CB_DYNAMIC_STATE_DEPTH_BOUNDS_TEST_ENABLE)) {
                    ss << "vkCmdSetDepthBoundsTestEnable last set depthBoundsTestEnable to VK_TRUE.\\n";
                } else {
                    ss << "VkPipelineDepthStencilStateCreateInfo::depthBoundsTestEnable was VK_TRUE in the last bound graphics pipeline.\\n";
                }''')
            if 'depthBiasEnable' in dependency:
                out.append('''
                if (!pipeline || pipeline->IsDynamic(CB_DYNAMIC_STATE_DEPTH_BIAS_ENABLE)) {
                    ss << "vkCmdSetDepthBiasEnable last set depthTestEnable to VK_TRUE.\\n";
                } else {
                    ss << "VkPipelineRasterizationStateCreateInfo::depthTestEnable was VK_TRUE in the last bound graphics pipeline.\\n";
                }''')
            if 'depthClampEnable' in dependency:
                out.append('''
                if (!pipeline || pipeline->IsDynamic(CB_DYNAMIC_STATE_DEPTH_CLAMP_ENABLE_EXT)) {
                    ss << "vkCmdSetDepthClampEnableEXT last set depthClampEnable to VK_TRUE.\\n";
                } else {
                    ss << "VkPipelineRasterizationStateCreateInfo::depthClampEnable was VK_TRUE in the last bound graphics pipeline.\\n";
                }''')
            if 'logicOpEnable' in dependency:
                out.append('''
                if (!pipeline || pipeline->IsDynamic(CB_DYNAMIC_STATE_LOGIC_OP_ENABLE_EXT)) {
                    ss << "vkCmdSetLogicOpEnableEXT last set logicOpEnable to VK_TRUE.\\n";
                } else {
                    ss << "VkPipelineColorBlendStateCreateInfo::logicOpEnable was VK_TRUE in the last bound graphics pipeline.\\n";
                }''')
            if 'stippledLineEnable' in dependency:
                out.append('''
                if (!pipeline || pipeline->IsDynamic(CB_DYNAMIC_STATE_LINE_STIPPLE_ENABLE_EXT)) {
                    ss << "vkCmdSetLineStippleEnableEXT last set stippledLineEnable to VK_TRUE.\\n";
                } else {
                    ss << "VkPipelineRasterizationLineStateCreateInfo::stippledLineEnable was VK_TRUE in the last bound graphics pipeline.\\n";
                }''')
            if 'sampleLocationsEnable' in dependency:
                out.append('''
                if (!pipeline || pipeline->IsDynamic(CB_DYNAMIC_STATE_LOGIC_OP_ENABLE_EXT)) {
                    ss << "vkCmdSetSampleLocationsEnableEXT last set logicOpEnable to VK_TRUE.\\n";
                } else {
                    ss << "VkPipelineMultisampleStateCreateInfo::pNext->VkPipelineSampleLocationsStateCreateInfoEXT::sampleLocationsEnable was VK_TRUE in the last bound graphics pipeline.\\n";
                }''')
            if 'coverageModulationTableEnable' in dependency:
                out.append('''
                if (!pipeline || pipeline->IsDynamic(CB_DYNAMIC_STATE_COVERAGE_MODULATION_TABLE_ENABLE_NV)) {
                    ss << "vkCmdSetCoverageModulationTableEnableNV last set coverageModulationTableEnable to VK_TRUE.\\n";
                } else {
                    ss << "VkPipelineMultisampleStateCreateInfo::pNext->VkPipelineCoverageModulationStateCreateInfoNV::coverageModulationTableEnable was VK_TRUE in the last bound graphics pipeline.\\n";
                }''')
            if 'shadingRateImageEnable' in dependency:
                out.append('''
                if (!pipeline || pipeline->IsDynamic(CB_DYNAMIC_STATE_VIEWPORT_SHADING_RATE_PALETTE_NV)) {
                    ss << "vkCmdSetShadingRateImageEnableNV last set shadingRateImageEnable to VK_TRUE.\\n";
                } else {
                    ss << "VkPipelineViewportStateCreateInfo::pNext->VkPipelineViewportShadingRateImageStateCreateInfoNV::shadingRateImageEnable was VK_TRUE in the last bound graphics pipeline.\\n";
                }''')
            if 'viewportWScalingEnable' in dependency:
                out.append('''
                if (!pipeline || pipeline->IsDynamic(CB_DYNAMIC_STATE_VIEWPORT_W_SCALING_ENABLE_NV)) {
                    ss << "vkCmdSetViewportWScalingEnableNV last set viewportWScalingEnable to VK_TRUE.\\n";
                } else {
                    ss << "VkPipelineViewportStateCreateInfo::pNext->VkPipelineViewportWScalingStateCreateInfoNV::viewportWScalingEnable was VK_TRUE in the last bound graphics pipeline.\\n";
                }''')
            if 'discardRectangleEnable' in dependency:
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