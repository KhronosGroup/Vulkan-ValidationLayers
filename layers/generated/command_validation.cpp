// *** THIS FILE IS GENERATED - DO NOT EDIT ***
// See command_validation_generator.py for modifications


/***************************************************************************
 *
 * Copyright (c) 2021-2023 The Khronos Group Inc.
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

#include "vk_layer_logging.h"
#include "core_checks/core_validation.h"

static const std::array<const char *, CMD_RANGE_SIZE> kGeneratedMustBeRecordingList = {{
    kVUIDUndefined,
    "VUID-vkCmdBeginConditionalRenderingEXT-commandBuffer-recording",
    "VUID-vkCmdBeginDebugUtilsLabelEXT-commandBuffer-recording",
    "VUID-vkCmdBeginQuery-commandBuffer-recording",
    "VUID-vkCmdBeginQueryIndexedEXT-commandBuffer-recording",
    "VUID-vkCmdBeginRenderPass-commandBuffer-recording",
    "VUID-vkCmdBeginRenderPass2-commandBuffer-recording",
    "VUID-vkCmdBeginRenderPass2-commandBuffer-recording",
    "VUID-vkCmdBeginRendering-commandBuffer-recording",
    "VUID-vkCmdBeginRendering-commandBuffer-recording",
    "VUID-vkCmdBeginTransformFeedbackEXT-commandBuffer-recording",
    "VUID-vkCmdBeginVideoCodingKHR-commandBuffer-recording",
    "VUID-vkCmdBindDescriptorBufferEmbeddedSamplersEXT-commandBuffer-recording",
    "VUID-vkCmdBindDescriptorBuffersEXT-commandBuffer-recording",
    "VUID-vkCmdBindDescriptorSets-commandBuffer-recording",
    "VUID-vkCmdBindIndexBuffer-commandBuffer-recording",
    "VUID-vkCmdBindInvocationMaskHUAWEI-commandBuffer-recording",
    "VUID-vkCmdBindPipeline-commandBuffer-recording",
    "VUID-vkCmdBindPipelineShaderGroupNV-commandBuffer-recording",
    "VUID-vkCmdBindShadingRateImageNV-commandBuffer-recording",
    "VUID-vkCmdBindTransformFeedbackBuffersEXT-commandBuffer-recording",
    "VUID-vkCmdBindVertexBuffers-commandBuffer-recording",
    "VUID-vkCmdBindVertexBuffers2-commandBuffer-recording",
    "VUID-vkCmdBindVertexBuffers2-commandBuffer-recording",
    "VUID-vkCmdBlitImage-commandBuffer-recording",
    "VUID-vkCmdBlitImage2-commandBuffer-recording",
    "VUID-vkCmdBlitImage2-commandBuffer-recording",
    "VUID-vkCmdBuildAccelerationStructureNV-commandBuffer-recording",
    "VUID-vkCmdBuildAccelerationStructuresIndirectKHR-commandBuffer-recording",
    "VUID-vkCmdBuildAccelerationStructuresKHR-commandBuffer-recording",
    "VUID-vkCmdBuildMicromapsEXT-commandBuffer-recording",
    "VUID-vkCmdClearAttachments-commandBuffer-recording",
    "VUID-vkCmdClearColorImage-commandBuffer-recording",
    "VUID-vkCmdClearDepthStencilImage-commandBuffer-recording",
    "VUID-vkCmdControlVideoCodingKHR-commandBuffer-recording",
    "VUID-vkCmdCopyAccelerationStructureKHR-commandBuffer-recording",
    "VUID-vkCmdCopyAccelerationStructureNV-commandBuffer-recording",
    "VUID-vkCmdCopyAccelerationStructureToMemoryKHR-commandBuffer-recording",
    "VUID-vkCmdCopyBuffer-commandBuffer-recording",
    "VUID-vkCmdCopyBuffer2-commandBuffer-recording",
    "VUID-vkCmdCopyBuffer2-commandBuffer-recording",
    "VUID-vkCmdCopyBufferToImage-commandBuffer-recording",
    "VUID-vkCmdCopyBufferToImage2-commandBuffer-recording",
    "VUID-vkCmdCopyBufferToImage2-commandBuffer-recording",
    "VUID-vkCmdCopyImage-commandBuffer-recording",
    "VUID-vkCmdCopyImage2-commandBuffer-recording",
    "VUID-vkCmdCopyImage2-commandBuffer-recording",
    "VUID-vkCmdCopyImageToBuffer-commandBuffer-recording",
    "VUID-vkCmdCopyImageToBuffer2-commandBuffer-recording",
    "VUID-vkCmdCopyImageToBuffer2-commandBuffer-recording",
    "VUID-vkCmdCopyMemoryIndirectNV-commandBuffer-recording",
    "VUID-vkCmdCopyMemoryToAccelerationStructureKHR-commandBuffer-recording",
    "VUID-vkCmdCopyMemoryToImageIndirectNV-commandBuffer-recording",
    "VUID-vkCmdCopyMemoryToMicromapEXT-commandBuffer-recording",
    "VUID-vkCmdCopyMicromapEXT-commandBuffer-recording",
    "VUID-vkCmdCopyMicromapToMemoryEXT-commandBuffer-recording",
    "VUID-vkCmdCopyQueryPoolResults-commandBuffer-recording",
    "VUID-vkCmdCuLaunchKernelNVX-commandBuffer-recording",
    "VUID-vkCmdDebugMarkerBeginEXT-commandBuffer-recording",
    "VUID-vkCmdDebugMarkerEndEXT-commandBuffer-recording",
    "VUID-vkCmdDebugMarkerInsertEXT-commandBuffer-recording",
    "VUID-vkCmdDecodeVideoKHR-commandBuffer-recording",
    "VUID-vkCmdDecompressMemoryIndirectCountNV-commandBuffer-recording",
    "VUID-vkCmdDecompressMemoryNV-commandBuffer-recording",
    "VUID-vkCmdDispatch-commandBuffer-recording",
    "VUID-vkCmdDispatchBase-commandBuffer-recording",
    "VUID-vkCmdDispatchBase-commandBuffer-recording",
    "VUID-vkCmdDispatchIndirect-commandBuffer-recording",
    "VUID-vkCmdDraw-commandBuffer-recording",
    "VUID-vkCmdDrawClusterHUAWEI-commandBuffer-recording",
    "VUID-vkCmdDrawClusterIndirectHUAWEI-commandBuffer-recording",
    "VUID-vkCmdDrawIndexed-commandBuffer-recording",
    "VUID-vkCmdDrawIndexedIndirect-commandBuffer-recording",
    "VUID-vkCmdDrawIndexedIndirectCount-commandBuffer-recording",
    "VUID-vkCmdDrawIndexedIndirectCount-commandBuffer-recording",
    "VUID-vkCmdDrawIndexedIndirectCount-commandBuffer-recording",
    "VUID-vkCmdDrawIndirect-commandBuffer-recording",
    "VUID-vkCmdDrawIndirectByteCountEXT-commandBuffer-recording",
    "VUID-vkCmdDrawIndirectCount-commandBuffer-recording",
    "VUID-vkCmdDrawIndirectCount-commandBuffer-recording",
    "VUID-vkCmdDrawIndirectCount-commandBuffer-recording",
    "VUID-vkCmdDrawMeshTasksEXT-commandBuffer-recording",
    "VUID-vkCmdDrawMeshTasksIndirectCountEXT-commandBuffer-recording",
    "VUID-vkCmdDrawMeshTasksIndirectCountNV-commandBuffer-recording",
    "VUID-vkCmdDrawMeshTasksIndirectEXT-commandBuffer-recording",
    "VUID-vkCmdDrawMeshTasksIndirectNV-commandBuffer-recording",
    "VUID-vkCmdDrawMeshTasksNV-commandBuffer-recording",
    "VUID-vkCmdDrawMultiEXT-commandBuffer-recording",
    "VUID-vkCmdDrawMultiIndexedEXT-commandBuffer-recording",
    "VUID-vkCmdEncodeVideoKHR-commandBuffer-recording",
    "VUID-vkCmdEndConditionalRenderingEXT-commandBuffer-recording",
    "VUID-vkCmdEndDebugUtilsLabelEXT-commandBuffer-recording",
    "VUID-vkCmdEndQuery-commandBuffer-recording",
    "VUID-vkCmdEndQueryIndexedEXT-commandBuffer-recording",
    "VUID-vkCmdEndRenderPass-commandBuffer-recording",
    "VUID-vkCmdEndRenderPass2-commandBuffer-recording",
    "VUID-vkCmdEndRenderPass2-commandBuffer-recording",
    "VUID-vkCmdEndRendering-commandBuffer-recording",
    "VUID-vkCmdEndRendering-commandBuffer-recording",
    "VUID-vkCmdEndTransformFeedbackEXT-commandBuffer-recording",
    "VUID-vkCmdEndVideoCodingKHR-commandBuffer-recording",
    "VUID-vkCmdExecuteCommands-commandBuffer-recording",
    "VUID-vkCmdExecuteGeneratedCommandsNV-commandBuffer-recording",
    "VUID-vkCmdFillBuffer-commandBuffer-recording",
    "VUID-vkCmdInsertDebugUtilsLabelEXT-commandBuffer-recording",
    "VUID-vkCmdNextSubpass-commandBuffer-recording",
    "VUID-vkCmdNextSubpass2-commandBuffer-recording",
    "VUID-vkCmdNextSubpass2-commandBuffer-recording",
    "VUID-vkCmdOpticalFlowExecuteNV-commandBuffer-recording",
    "VUID-vkCmdPipelineBarrier-commandBuffer-recording",
    "VUID-vkCmdPipelineBarrier2-commandBuffer-recording",
    "VUID-vkCmdPipelineBarrier2-commandBuffer-recording",
    "VUID-vkCmdPreprocessGeneratedCommandsNV-commandBuffer-recording",
    "VUID-vkCmdPushConstants-commandBuffer-recording",
    "VUID-vkCmdPushDescriptorSetKHR-commandBuffer-recording",
    "VUID-vkCmdPushDescriptorSetWithTemplateKHR-commandBuffer-recording",
    "VUID-vkCmdResetEvent-commandBuffer-recording",
    "VUID-vkCmdResetEvent2-commandBuffer-recording",
    "VUID-vkCmdResetEvent2-commandBuffer-recording",
    "VUID-vkCmdResetQueryPool-commandBuffer-recording",
    "VUID-vkCmdResolveImage-commandBuffer-recording",
    "VUID-vkCmdResolveImage2-commandBuffer-recording",
    "VUID-vkCmdResolveImage2-commandBuffer-recording",
    "VUID-vkCmdSetAlphaToCoverageEnableEXT-commandBuffer-recording",
    "VUID-vkCmdSetAlphaToOneEnableEXT-commandBuffer-recording",
    "VUID-vkCmdSetBlendConstants-commandBuffer-recording",
    "VUID-vkCmdSetCheckpointNV-commandBuffer-recording",
    "VUID-vkCmdSetCoarseSampleOrderNV-commandBuffer-recording",
    "VUID-vkCmdSetColorBlendAdvancedEXT-commandBuffer-recording",
    "VUID-vkCmdSetColorBlendEnableEXT-commandBuffer-recording",
    "VUID-vkCmdSetColorBlendEquationEXT-commandBuffer-recording",
    "VUID-vkCmdSetColorWriteEnableEXT-commandBuffer-recording",
    "VUID-vkCmdSetColorWriteMaskEXT-commandBuffer-recording",
    "VUID-vkCmdSetConservativeRasterizationModeEXT-commandBuffer-recording",
    "VUID-vkCmdSetCoverageModulationModeNV-commandBuffer-recording",
    "VUID-vkCmdSetCoverageModulationTableEnableNV-commandBuffer-recording",
    "VUID-vkCmdSetCoverageModulationTableNV-commandBuffer-recording",
    "VUID-vkCmdSetCoverageReductionModeNV-commandBuffer-recording",
    "VUID-vkCmdSetCoverageToColorEnableNV-commandBuffer-recording",
    "VUID-vkCmdSetCoverageToColorLocationNV-commandBuffer-recording",
    "VUID-vkCmdSetCullMode-commandBuffer-recording",
    "VUID-vkCmdSetCullMode-commandBuffer-recording",
    "VUID-vkCmdSetDepthBias-commandBuffer-recording",
    "VUID-vkCmdSetDepthBiasEnable-commandBuffer-recording",
    "VUID-vkCmdSetDepthBiasEnable-commandBuffer-recording",
    "VUID-vkCmdSetDepthBounds-commandBuffer-recording",
    "VUID-vkCmdSetDepthBoundsTestEnable-commandBuffer-recording",
    "VUID-vkCmdSetDepthBoundsTestEnable-commandBuffer-recording",
    "VUID-vkCmdSetDepthClampEnableEXT-commandBuffer-recording",
    "VUID-vkCmdSetDepthClipEnableEXT-commandBuffer-recording",
    "VUID-vkCmdSetDepthClipNegativeOneToOneEXT-commandBuffer-recording",
    "VUID-vkCmdSetDepthCompareOp-commandBuffer-recording",
    "VUID-vkCmdSetDepthCompareOp-commandBuffer-recording",
    "VUID-vkCmdSetDepthTestEnable-commandBuffer-recording",
    "VUID-vkCmdSetDepthTestEnable-commandBuffer-recording",
    "VUID-vkCmdSetDepthWriteEnable-commandBuffer-recording",
    "VUID-vkCmdSetDepthWriteEnable-commandBuffer-recording",
    "VUID-vkCmdSetDescriptorBufferOffsetsEXT-commandBuffer-recording",
    "VUID-vkCmdSetDeviceMask-commandBuffer-recording",
    "VUID-vkCmdSetDeviceMask-commandBuffer-recording",
    "VUID-vkCmdSetDiscardRectangleEXT-commandBuffer-recording",
    "VUID-vkCmdSetEvent-commandBuffer-recording",
    "VUID-vkCmdSetEvent2-commandBuffer-recording",
    "VUID-vkCmdSetEvent2-commandBuffer-recording",
    "VUID-vkCmdSetExclusiveScissorNV-commandBuffer-recording",
    "VUID-vkCmdSetExtraPrimitiveOverestimationSizeEXT-commandBuffer-recording",
    "VUID-vkCmdSetFragmentShadingRateEnumNV-commandBuffer-recording",
    "VUID-vkCmdSetFragmentShadingRateKHR-commandBuffer-recording",
    "VUID-vkCmdSetFrontFace-commandBuffer-recording",
    "VUID-vkCmdSetFrontFace-commandBuffer-recording",
    "VUID-vkCmdSetLineRasterizationModeEXT-commandBuffer-recording",
    "VUID-vkCmdSetLineStippleEXT-commandBuffer-recording",
    "VUID-vkCmdSetLineStippleEnableEXT-commandBuffer-recording",
    "VUID-vkCmdSetLineWidth-commandBuffer-recording",
    "VUID-vkCmdSetLogicOpEXT-commandBuffer-recording",
    "VUID-vkCmdSetLogicOpEnableEXT-commandBuffer-recording",
    "VUID-vkCmdSetPatchControlPointsEXT-commandBuffer-recording",
    "VUID-vkCmdSetPerformanceMarkerINTEL-commandBuffer-recording",
    "VUID-vkCmdSetPerformanceOverrideINTEL-commandBuffer-recording",
    "VUID-vkCmdSetPerformanceStreamMarkerINTEL-commandBuffer-recording",
    "VUID-vkCmdSetPolygonModeEXT-commandBuffer-recording",
    "VUID-vkCmdSetPrimitiveRestartEnable-commandBuffer-recording",
    "VUID-vkCmdSetPrimitiveRestartEnable-commandBuffer-recording",
    "VUID-vkCmdSetPrimitiveTopology-commandBuffer-recording",
    "VUID-vkCmdSetPrimitiveTopology-commandBuffer-recording",
    "VUID-vkCmdSetProvokingVertexModeEXT-commandBuffer-recording",
    "VUID-vkCmdSetRasterizationSamplesEXT-commandBuffer-recording",
    "VUID-vkCmdSetRasterizationStreamEXT-commandBuffer-recording",
    "VUID-vkCmdSetRasterizerDiscardEnable-commandBuffer-recording",
    "VUID-vkCmdSetRasterizerDiscardEnable-commandBuffer-recording",
    "VUID-vkCmdSetRayTracingPipelineStackSizeKHR-commandBuffer-recording",
    "VUID-vkCmdSetRepresentativeFragmentTestEnableNV-commandBuffer-recording",
    "VUID-vkCmdSetSampleLocationsEXT-commandBuffer-recording",
    "VUID-vkCmdSetSampleLocationsEnableEXT-commandBuffer-recording",
    "VUID-vkCmdSetSampleMaskEXT-commandBuffer-recording",
    "VUID-vkCmdSetScissor-commandBuffer-recording",
    "VUID-vkCmdSetScissorWithCount-commandBuffer-recording",
    "VUID-vkCmdSetScissorWithCount-commandBuffer-recording",
    "VUID-vkCmdSetShadingRateImageEnableNV-commandBuffer-recording",
    "VUID-vkCmdSetStencilCompareMask-commandBuffer-recording",
    "VUID-vkCmdSetStencilOp-commandBuffer-recording",
    "VUID-vkCmdSetStencilOp-commandBuffer-recording",
    "VUID-vkCmdSetStencilReference-commandBuffer-recording",
    "VUID-vkCmdSetStencilTestEnable-commandBuffer-recording",
    "VUID-vkCmdSetStencilTestEnable-commandBuffer-recording",
    "VUID-vkCmdSetStencilWriteMask-commandBuffer-recording",
    "VUID-vkCmdSetTessellationDomainOriginEXT-commandBuffer-recording",
    "VUID-vkCmdSetVertexInputEXT-commandBuffer-recording",
    "VUID-vkCmdSetViewport-commandBuffer-recording",
    "VUID-vkCmdSetViewportShadingRatePaletteNV-commandBuffer-recording",
    "VUID-vkCmdSetViewportSwizzleNV-commandBuffer-recording",
    "VUID-vkCmdSetViewportWScalingEnableNV-commandBuffer-recording",
    "VUID-vkCmdSetViewportWScalingNV-commandBuffer-recording",
    "VUID-vkCmdSetViewportWithCount-commandBuffer-recording",
    "VUID-vkCmdSetViewportWithCount-commandBuffer-recording",
    "VUID-vkCmdSubpassShadingHUAWEI-commandBuffer-recording",
    "VUID-vkCmdTraceRaysIndirect2KHR-commandBuffer-recording",
    "VUID-vkCmdTraceRaysIndirectKHR-commandBuffer-recording",
    "VUID-vkCmdTraceRaysKHR-commandBuffer-recording",
    "VUID-vkCmdTraceRaysNV-commandBuffer-recording",
    "VUID-vkCmdUpdateBuffer-commandBuffer-recording",
    "VUID-vkCmdWaitEvents-commandBuffer-recording",
    "VUID-vkCmdWaitEvents2-commandBuffer-recording",
    "VUID-vkCmdWaitEvents2-commandBuffer-recording",
    "VUID-vkCmdWriteAccelerationStructuresPropertiesKHR-commandBuffer-recording",
    "VUID-vkCmdWriteAccelerationStructuresPropertiesNV-commandBuffer-recording",
    "VUID-vkCmdWriteBufferMarker2AMD-commandBuffer-recording",
    "VUID-vkCmdWriteBufferMarkerAMD-commandBuffer-recording",
    "VUID-vkCmdWriteMicromapsPropertiesEXT-commandBuffer-recording",
    "VUID-vkCmdWriteTimestamp-commandBuffer-recording",
    "VUID-vkCmdWriteTimestamp2-commandBuffer-recording",
    "VUID-vkCmdWriteTimestamp2-commandBuffer-recording",
}};

struct CommandSupportedQueueType {
    VkQueueFlags flags;
    const char* vuid;
};
static const std::array<CommandSupportedQueueType, CMD_RANGE_SIZE> kGeneratedQueueTypeList = {{
    {VK_QUEUE_FLAG_BITS_MAX_ENUM, kVUIDUndefined},
    {VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_COMPUTE_BIT, "VUID-vkCmdBeginConditionalRenderingEXT-commandBuffer-cmdpool"},
    {VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_COMPUTE_BIT, "VUID-vkCmdBeginDebugUtilsLabelEXT-commandBuffer-cmdpool"},
    {VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_COMPUTE_BIT | VK_QUEUE_VIDEO_DECODE_BIT_KHR | VK_QUEUE_VIDEO_ENCODE_BIT_KHR, "VUID-vkCmdBeginQuery-commandBuffer-cmdpool"},
    {VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_COMPUTE_BIT | VK_QUEUE_VIDEO_DECODE_BIT_KHR | VK_QUEUE_VIDEO_ENCODE_BIT_KHR, "VUID-vkCmdBeginQueryIndexedEXT-commandBuffer-cmdpool"},
    {VK_QUEUE_GRAPHICS_BIT, "VUID-vkCmdBeginRenderPass-commandBuffer-cmdpool"},
    {VK_QUEUE_GRAPHICS_BIT, "VUID-vkCmdBeginRenderPass2-commandBuffer-cmdpool"},
    {VK_QUEUE_GRAPHICS_BIT, "VUID-vkCmdBeginRenderPass2-commandBuffer-cmdpool"},
    {VK_QUEUE_GRAPHICS_BIT, "VUID-vkCmdBeginRendering-commandBuffer-cmdpool"},
    {VK_QUEUE_GRAPHICS_BIT, "VUID-vkCmdBeginRendering-commandBuffer-cmdpool"},
    {VK_QUEUE_GRAPHICS_BIT, "VUID-vkCmdBeginTransformFeedbackEXT-commandBuffer-cmdpool"},
    {VK_QUEUE_VIDEO_DECODE_BIT_KHR | VK_QUEUE_VIDEO_ENCODE_BIT_KHR, "VUID-vkCmdBeginVideoCodingKHR-commandBuffer-cmdpool"},
    {VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_COMPUTE_BIT, "VUID-vkCmdBindDescriptorBufferEmbeddedSamplersEXT-commandBuffer-cmdpool"},
    {VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_COMPUTE_BIT, "VUID-vkCmdBindDescriptorBuffersEXT-commandBuffer-cmdpool"},
    {VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_COMPUTE_BIT, "VUID-vkCmdBindDescriptorSets-commandBuffer-cmdpool"},
    {VK_QUEUE_GRAPHICS_BIT, "VUID-vkCmdBindIndexBuffer-commandBuffer-cmdpool"},
    {VK_QUEUE_COMPUTE_BIT, "VUID-vkCmdBindInvocationMaskHUAWEI-commandBuffer-cmdpool"},
    {VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_COMPUTE_BIT, "VUID-vkCmdBindPipeline-commandBuffer-cmdpool"},
    {VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_COMPUTE_BIT, "VUID-vkCmdBindPipelineShaderGroupNV-commandBuffer-cmdpool"},
    {VK_QUEUE_GRAPHICS_BIT, "VUID-vkCmdBindShadingRateImageNV-commandBuffer-cmdpool"},
    {VK_QUEUE_GRAPHICS_BIT, "VUID-vkCmdBindTransformFeedbackBuffersEXT-commandBuffer-cmdpool"},
    {VK_QUEUE_GRAPHICS_BIT, "VUID-vkCmdBindVertexBuffers-commandBuffer-cmdpool"},
    {VK_QUEUE_GRAPHICS_BIT, "VUID-vkCmdBindVertexBuffers2-commandBuffer-cmdpool"},
    {VK_QUEUE_GRAPHICS_BIT, "VUID-vkCmdBindVertexBuffers2-commandBuffer-cmdpool"},
    {VK_QUEUE_GRAPHICS_BIT, "VUID-vkCmdBlitImage-commandBuffer-cmdpool"},
    {VK_QUEUE_GRAPHICS_BIT, "VUID-vkCmdBlitImage2-commandBuffer-cmdpool"},
    {VK_QUEUE_GRAPHICS_BIT, "VUID-vkCmdBlitImage2-commandBuffer-cmdpool"},
    {VK_QUEUE_COMPUTE_BIT, "VUID-vkCmdBuildAccelerationStructureNV-commandBuffer-cmdpool"},
    {VK_QUEUE_COMPUTE_BIT, "VUID-vkCmdBuildAccelerationStructuresIndirectKHR-commandBuffer-cmdpool"},
    {VK_QUEUE_COMPUTE_BIT, "VUID-vkCmdBuildAccelerationStructuresKHR-commandBuffer-cmdpool"},
    {VK_QUEUE_COMPUTE_BIT, "VUID-vkCmdBuildMicromapsEXT-commandBuffer-cmdpool"},
    {VK_QUEUE_GRAPHICS_BIT, "VUID-vkCmdClearAttachments-commandBuffer-cmdpool"},
    {VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_COMPUTE_BIT, "VUID-vkCmdClearColorImage-commandBuffer-cmdpool"},
    {VK_QUEUE_GRAPHICS_BIT, "VUID-vkCmdClearDepthStencilImage-commandBuffer-cmdpool"},
    {VK_QUEUE_VIDEO_DECODE_BIT_KHR | VK_QUEUE_VIDEO_ENCODE_BIT_KHR, "VUID-vkCmdControlVideoCodingKHR-commandBuffer-cmdpool"},
    {VK_QUEUE_COMPUTE_BIT, "VUID-vkCmdCopyAccelerationStructureKHR-commandBuffer-cmdpool"},
    {VK_QUEUE_COMPUTE_BIT, "VUID-vkCmdCopyAccelerationStructureNV-commandBuffer-cmdpool"},
    {VK_QUEUE_COMPUTE_BIT, "VUID-vkCmdCopyAccelerationStructureToMemoryKHR-commandBuffer-cmdpool"},
    {VK_QUEUE_TRANSFER_BIT | VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_COMPUTE_BIT, "VUID-vkCmdCopyBuffer-commandBuffer-cmdpool"},
    {VK_QUEUE_TRANSFER_BIT | VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_COMPUTE_BIT, "VUID-vkCmdCopyBuffer2-commandBuffer-cmdpool"},
    {VK_QUEUE_TRANSFER_BIT | VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_COMPUTE_BIT, "VUID-vkCmdCopyBuffer2-commandBuffer-cmdpool"},
    {VK_QUEUE_TRANSFER_BIT | VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_COMPUTE_BIT, "VUID-vkCmdCopyBufferToImage-commandBuffer-cmdpool"},
    {VK_QUEUE_TRANSFER_BIT | VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_COMPUTE_BIT, "VUID-vkCmdCopyBufferToImage2-commandBuffer-cmdpool"},
    {VK_QUEUE_TRANSFER_BIT | VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_COMPUTE_BIT, "VUID-vkCmdCopyBufferToImage2-commandBuffer-cmdpool"},
    {VK_QUEUE_TRANSFER_BIT | VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_COMPUTE_BIT, "VUID-vkCmdCopyImage-commandBuffer-cmdpool"},
    {VK_QUEUE_TRANSFER_BIT | VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_COMPUTE_BIT, "VUID-vkCmdCopyImage2-commandBuffer-cmdpool"},
    {VK_QUEUE_TRANSFER_BIT | VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_COMPUTE_BIT, "VUID-vkCmdCopyImage2-commandBuffer-cmdpool"},
    {VK_QUEUE_TRANSFER_BIT | VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_COMPUTE_BIT, "VUID-vkCmdCopyImageToBuffer-commandBuffer-cmdpool"},
    {VK_QUEUE_TRANSFER_BIT | VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_COMPUTE_BIT, "VUID-vkCmdCopyImageToBuffer2-commandBuffer-cmdpool"},
    {VK_QUEUE_TRANSFER_BIT | VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_COMPUTE_BIT, "VUID-vkCmdCopyImageToBuffer2-commandBuffer-cmdpool"},
    {VK_QUEUE_TRANSFER_BIT | VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_COMPUTE_BIT, "VUID-vkCmdCopyMemoryIndirectNV-commandBuffer-cmdpool"},
    {VK_QUEUE_COMPUTE_BIT, "VUID-vkCmdCopyMemoryToAccelerationStructureKHR-commandBuffer-cmdpool"},
    {VK_QUEUE_TRANSFER_BIT | VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_COMPUTE_BIT, "VUID-vkCmdCopyMemoryToImageIndirectNV-commandBuffer-cmdpool"},
    {VK_QUEUE_COMPUTE_BIT, "VUID-vkCmdCopyMemoryToMicromapEXT-commandBuffer-cmdpool"},
    {VK_QUEUE_COMPUTE_BIT, "VUID-vkCmdCopyMicromapEXT-commandBuffer-cmdpool"},
    {VK_QUEUE_COMPUTE_BIT, "VUID-vkCmdCopyMicromapToMemoryEXT-commandBuffer-cmdpool"},
    {VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_COMPUTE_BIT, "VUID-vkCmdCopyQueryPoolResults-commandBuffer-cmdpool"},
    {VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_COMPUTE_BIT, "VUID-vkCmdCuLaunchKernelNVX-commandBuffer-cmdpool"},
    {VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_COMPUTE_BIT, "VUID-vkCmdDebugMarkerBeginEXT-commandBuffer-cmdpool"},
    {VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_COMPUTE_BIT, "VUID-vkCmdDebugMarkerEndEXT-commandBuffer-cmdpool"},
    {VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_COMPUTE_BIT, "VUID-vkCmdDebugMarkerInsertEXT-commandBuffer-cmdpool"},
    {VK_QUEUE_VIDEO_DECODE_BIT_KHR, "VUID-vkCmdDecodeVideoKHR-commandBuffer-cmdpool"},
    {VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_COMPUTE_BIT, "VUID-vkCmdDecompressMemoryIndirectCountNV-commandBuffer-cmdpool"},
    {VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_COMPUTE_BIT, "VUID-vkCmdDecompressMemoryNV-commandBuffer-cmdpool"},
    {VK_QUEUE_COMPUTE_BIT, "VUID-vkCmdDispatch-commandBuffer-cmdpool"},
    {VK_QUEUE_COMPUTE_BIT, "VUID-vkCmdDispatchBase-commandBuffer-cmdpool"},
    {VK_QUEUE_COMPUTE_BIT, "VUID-vkCmdDispatchBase-commandBuffer-cmdpool"},
    {VK_QUEUE_COMPUTE_BIT, "VUID-vkCmdDispatchIndirect-commandBuffer-cmdpool"},
    {VK_QUEUE_GRAPHICS_BIT, "VUID-vkCmdDraw-commandBuffer-cmdpool"},
    {VK_QUEUE_GRAPHICS_BIT, "VUID-vkCmdDrawClusterHUAWEI-commandBuffer-cmdpool"},
    {VK_QUEUE_GRAPHICS_BIT, "VUID-vkCmdDrawClusterIndirectHUAWEI-commandBuffer-cmdpool"},
    {VK_QUEUE_GRAPHICS_BIT, "VUID-vkCmdDrawIndexed-commandBuffer-cmdpool"},
    {VK_QUEUE_GRAPHICS_BIT, "VUID-vkCmdDrawIndexedIndirect-commandBuffer-cmdpool"},
    {VK_QUEUE_GRAPHICS_BIT, "VUID-vkCmdDrawIndexedIndirectCount-commandBuffer-cmdpool"},
    {VK_QUEUE_GRAPHICS_BIT, "VUID-vkCmdDrawIndexedIndirectCount-commandBuffer-cmdpool"},
    {VK_QUEUE_GRAPHICS_BIT, "VUID-vkCmdDrawIndexedIndirectCount-commandBuffer-cmdpool"},
    {VK_QUEUE_GRAPHICS_BIT, "VUID-vkCmdDrawIndirect-commandBuffer-cmdpool"},
    {VK_QUEUE_GRAPHICS_BIT, "VUID-vkCmdDrawIndirectByteCountEXT-commandBuffer-cmdpool"},
    {VK_QUEUE_GRAPHICS_BIT, "VUID-vkCmdDrawIndirectCount-commandBuffer-cmdpool"},
    {VK_QUEUE_GRAPHICS_BIT, "VUID-vkCmdDrawIndirectCount-commandBuffer-cmdpool"},
    {VK_QUEUE_GRAPHICS_BIT, "VUID-vkCmdDrawIndirectCount-commandBuffer-cmdpool"},
    {VK_QUEUE_GRAPHICS_BIT, "VUID-vkCmdDrawMeshTasksEXT-commandBuffer-cmdpool"},
    {VK_QUEUE_GRAPHICS_BIT, "VUID-vkCmdDrawMeshTasksIndirectCountEXT-commandBuffer-cmdpool"},
    {VK_QUEUE_GRAPHICS_BIT, "VUID-vkCmdDrawMeshTasksIndirectCountNV-commandBuffer-cmdpool"},
    {VK_QUEUE_GRAPHICS_BIT, "VUID-vkCmdDrawMeshTasksIndirectEXT-commandBuffer-cmdpool"},
    {VK_QUEUE_GRAPHICS_BIT, "VUID-vkCmdDrawMeshTasksIndirectNV-commandBuffer-cmdpool"},
    {VK_QUEUE_GRAPHICS_BIT, "VUID-vkCmdDrawMeshTasksNV-commandBuffer-cmdpool"},
    {VK_QUEUE_GRAPHICS_BIT, "VUID-vkCmdDrawMultiEXT-commandBuffer-cmdpool"},
    {VK_QUEUE_GRAPHICS_BIT, "VUID-vkCmdDrawMultiIndexedEXT-commandBuffer-cmdpool"},
    {VK_QUEUE_VIDEO_ENCODE_BIT_KHR, "VUID-vkCmdEncodeVideoKHR-commandBuffer-cmdpool"},
    {VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_COMPUTE_BIT, "VUID-vkCmdEndConditionalRenderingEXT-commandBuffer-cmdpool"},
    {VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_COMPUTE_BIT, "VUID-vkCmdEndDebugUtilsLabelEXT-commandBuffer-cmdpool"},
    {VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_COMPUTE_BIT | VK_QUEUE_VIDEO_DECODE_BIT_KHR | VK_QUEUE_VIDEO_ENCODE_BIT_KHR, "VUID-vkCmdEndQuery-commandBuffer-cmdpool"},
    {VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_COMPUTE_BIT | VK_QUEUE_VIDEO_DECODE_BIT_KHR | VK_QUEUE_VIDEO_ENCODE_BIT_KHR, "VUID-vkCmdEndQueryIndexedEXT-commandBuffer-cmdpool"},
    {VK_QUEUE_GRAPHICS_BIT, "VUID-vkCmdEndRenderPass-commandBuffer-cmdpool"},
    {VK_QUEUE_GRAPHICS_BIT, "VUID-vkCmdEndRenderPass2-commandBuffer-cmdpool"},
    {VK_QUEUE_GRAPHICS_BIT, "VUID-vkCmdEndRenderPass2-commandBuffer-cmdpool"},
    {VK_QUEUE_GRAPHICS_BIT, "VUID-vkCmdEndRendering-commandBuffer-cmdpool"},
    {VK_QUEUE_GRAPHICS_BIT, "VUID-vkCmdEndRendering-commandBuffer-cmdpool"},
    {VK_QUEUE_GRAPHICS_BIT, "VUID-vkCmdEndTransformFeedbackEXT-commandBuffer-cmdpool"},
    {VK_QUEUE_VIDEO_DECODE_BIT_KHR | VK_QUEUE_VIDEO_ENCODE_BIT_KHR, "VUID-vkCmdEndVideoCodingKHR-commandBuffer-cmdpool"},
    {VK_QUEUE_TRANSFER_BIT | VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_COMPUTE_BIT, "VUID-vkCmdExecuteCommands-commandBuffer-cmdpool"},
    {VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_COMPUTE_BIT, "VUID-vkCmdExecuteGeneratedCommandsNV-commandBuffer-cmdpool"},
    {VK_QUEUE_TRANSFER_BIT | VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_COMPUTE_BIT, "VUID-vkCmdFillBuffer-commandBuffer-cmdpool"},
    {VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_COMPUTE_BIT, "VUID-vkCmdInsertDebugUtilsLabelEXT-commandBuffer-cmdpool"},
    {VK_QUEUE_GRAPHICS_BIT, "VUID-vkCmdNextSubpass-commandBuffer-cmdpool"},
    {VK_QUEUE_GRAPHICS_BIT, "VUID-vkCmdNextSubpass2-commandBuffer-cmdpool"},
    {VK_QUEUE_GRAPHICS_BIT, "VUID-vkCmdNextSubpass2-commandBuffer-cmdpool"},
    {VK_QUEUE_OPTICAL_FLOW_BIT_NV, "VUID-vkCmdOpticalFlowExecuteNV-commandBuffer-cmdpool"},
    {VK_QUEUE_TRANSFER_BIT | VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_COMPUTE_BIT | VK_QUEUE_VIDEO_DECODE_BIT_KHR | VK_QUEUE_VIDEO_ENCODE_BIT_KHR, "VUID-vkCmdPipelineBarrier-commandBuffer-cmdpool"},
    {VK_QUEUE_TRANSFER_BIT | VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_COMPUTE_BIT | VK_QUEUE_VIDEO_DECODE_BIT_KHR | VK_QUEUE_VIDEO_ENCODE_BIT_KHR, "VUID-vkCmdPipelineBarrier2-commandBuffer-cmdpool"},
    {VK_QUEUE_TRANSFER_BIT | VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_COMPUTE_BIT | VK_QUEUE_VIDEO_DECODE_BIT_KHR | VK_QUEUE_VIDEO_ENCODE_BIT_KHR, "VUID-vkCmdPipelineBarrier2-commandBuffer-cmdpool"},
    {VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_COMPUTE_BIT, "VUID-vkCmdPreprocessGeneratedCommandsNV-commandBuffer-cmdpool"},
    {VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_COMPUTE_BIT, "VUID-vkCmdPushConstants-commandBuffer-cmdpool"},
    {VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_COMPUTE_BIT, "VUID-vkCmdPushDescriptorSetKHR-commandBuffer-cmdpool"},
    {VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_COMPUTE_BIT, "VUID-vkCmdPushDescriptorSetWithTemplateKHR-commandBuffer-cmdpool"},
    {VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_COMPUTE_BIT | VK_QUEUE_VIDEO_DECODE_BIT_KHR | VK_QUEUE_VIDEO_ENCODE_BIT_KHR, "VUID-vkCmdResetEvent-commandBuffer-cmdpool"},
    {VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_COMPUTE_BIT | VK_QUEUE_VIDEO_DECODE_BIT_KHR | VK_QUEUE_VIDEO_ENCODE_BIT_KHR, "VUID-vkCmdResetEvent2-commandBuffer-cmdpool"},
    {VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_COMPUTE_BIT | VK_QUEUE_VIDEO_DECODE_BIT_KHR | VK_QUEUE_VIDEO_ENCODE_BIT_KHR, "VUID-vkCmdResetEvent2-commandBuffer-cmdpool"},
    {VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_COMPUTE_BIT | VK_QUEUE_VIDEO_DECODE_BIT_KHR | VK_QUEUE_VIDEO_ENCODE_BIT_KHR | VK_QUEUE_OPTICAL_FLOW_BIT_NV, "VUID-vkCmdResetQueryPool-commandBuffer-cmdpool"},
    {VK_QUEUE_GRAPHICS_BIT, "VUID-vkCmdResolveImage-commandBuffer-cmdpool"},
    {VK_QUEUE_GRAPHICS_BIT, "VUID-vkCmdResolveImage2-commandBuffer-cmdpool"},
    {VK_QUEUE_GRAPHICS_BIT, "VUID-vkCmdResolveImage2-commandBuffer-cmdpool"},
    {VK_QUEUE_GRAPHICS_BIT, "VUID-vkCmdSetAlphaToCoverageEnableEXT-commandBuffer-cmdpool"},
    {VK_QUEUE_GRAPHICS_BIT, "VUID-vkCmdSetAlphaToOneEnableEXT-commandBuffer-cmdpool"},
    {VK_QUEUE_GRAPHICS_BIT, "VUID-vkCmdSetBlendConstants-commandBuffer-cmdpool"},
    {VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_COMPUTE_BIT | VK_QUEUE_TRANSFER_BIT, "VUID-vkCmdSetCheckpointNV-commandBuffer-cmdpool"},
    {VK_QUEUE_GRAPHICS_BIT, "VUID-vkCmdSetCoarseSampleOrderNV-commandBuffer-cmdpool"},
    {VK_QUEUE_GRAPHICS_BIT, "VUID-vkCmdSetColorBlendAdvancedEXT-commandBuffer-cmdpool"},
    {VK_QUEUE_GRAPHICS_BIT, "VUID-vkCmdSetColorBlendEnableEXT-commandBuffer-cmdpool"},
    {VK_QUEUE_GRAPHICS_BIT, "VUID-vkCmdSetColorBlendEquationEXT-commandBuffer-cmdpool"},
    {VK_QUEUE_GRAPHICS_BIT, "VUID-vkCmdSetColorWriteEnableEXT-commandBuffer-cmdpool"},
    {VK_QUEUE_GRAPHICS_BIT, "VUID-vkCmdSetColorWriteMaskEXT-commandBuffer-cmdpool"},
    {VK_QUEUE_GRAPHICS_BIT, "VUID-vkCmdSetConservativeRasterizationModeEXT-commandBuffer-cmdpool"},
    {VK_QUEUE_GRAPHICS_BIT, "VUID-vkCmdSetCoverageModulationModeNV-commandBuffer-cmdpool"},
    {VK_QUEUE_GRAPHICS_BIT, "VUID-vkCmdSetCoverageModulationTableEnableNV-commandBuffer-cmdpool"},
    {VK_QUEUE_GRAPHICS_BIT, "VUID-vkCmdSetCoverageModulationTableNV-commandBuffer-cmdpool"},
    {VK_QUEUE_GRAPHICS_BIT, "VUID-vkCmdSetCoverageReductionModeNV-commandBuffer-cmdpool"},
    {VK_QUEUE_GRAPHICS_BIT, "VUID-vkCmdSetCoverageToColorEnableNV-commandBuffer-cmdpool"},
    {VK_QUEUE_GRAPHICS_BIT, "VUID-vkCmdSetCoverageToColorLocationNV-commandBuffer-cmdpool"},
    {VK_QUEUE_GRAPHICS_BIT, "VUID-vkCmdSetCullMode-commandBuffer-cmdpool"},
    {VK_QUEUE_GRAPHICS_BIT, "VUID-vkCmdSetCullMode-commandBuffer-cmdpool"},
    {VK_QUEUE_GRAPHICS_BIT, "VUID-vkCmdSetDepthBias-commandBuffer-cmdpool"},
    {VK_QUEUE_GRAPHICS_BIT, "VUID-vkCmdSetDepthBiasEnable-commandBuffer-cmdpool"},
    {VK_QUEUE_GRAPHICS_BIT, "VUID-vkCmdSetDepthBiasEnable-commandBuffer-cmdpool"},
    {VK_QUEUE_GRAPHICS_BIT, "VUID-vkCmdSetDepthBounds-commandBuffer-cmdpool"},
    {VK_QUEUE_GRAPHICS_BIT, "VUID-vkCmdSetDepthBoundsTestEnable-commandBuffer-cmdpool"},
    {VK_QUEUE_GRAPHICS_BIT, "VUID-vkCmdSetDepthBoundsTestEnable-commandBuffer-cmdpool"},
    {VK_QUEUE_GRAPHICS_BIT, "VUID-vkCmdSetDepthClampEnableEXT-commandBuffer-cmdpool"},
    {VK_QUEUE_GRAPHICS_BIT, "VUID-vkCmdSetDepthClipEnableEXT-commandBuffer-cmdpool"},
    {VK_QUEUE_GRAPHICS_BIT, "VUID-vkCmdSetDepthClipNegativeOneToOneEXT-commandBuffer-cmdpool"},
    {VK_QUEUE_GRAPHICS_BIT, "VUID-vkCmdSetDepthCompareOp-commandBuffer-cmdpool"},
    {VK_QUEUE_GRAPHICS_BIT, "VUID-vkCmdSetDepthCompareOp-commandBuffer-cmdpool"},
    {VK_QUEUE_GRAPHICS_BIT, "VUID-vkCmdSetDepthTestEnable-commandBuffer-cmdpool"},
    {VK_QUEUE_GRAPHICS_BIT, "VUID-vkCmdSetDepthTestEnable-commandBuffer-cmdpool"},
    {VK_QUEUE_GRAPHICS_BIT, "VUID-vkCmdSetDepthWriteEnable-commandBuffer-cmdpool"},
    {VK_QUEUE_GRAPHICS_BIT, "VUID-vkCmdSetDepthWriteEnable-commandBuffer-cmdpool"},
    {VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_COMPUTE_BIT, "VUID-vkCmdSetDescriptorBufferOffsetsEXT-commandBuffer-cmdpool"},
    {VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_COMPUTE_BIT | VK_QUEUE_TRANSFER_BIT, "VUID-vkCmdSetDeviceMask-commandBuffer-cmdpool"},
    {VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_COMPUTE_BIT | VK_QUEUE_TRANSFER_BIT, "VUID-vkCmdSetDeviceMask-commandBuffer-cmdpool"},
    {VK_QUEUE_GRAPHICS_BIT, "VUID-vkCmdSetDiscardRectangleEXT-commandBuffer-cmdpool"},
    {VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_COMPUTE_BIT | VK_QUEUE_VIDEO_DECODE_BIT_KHR | VK_QUEUE_VIDEO_ENCODE_BIT_KHR, "VUID-vkCmdSetEvent-commandBuffer-cmdpool"},
    {VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_COMPUTE_BIT | VK_QUEUE_VIDEO_DECODE_BIT_KHR | VK_QUEUE_VIDEO_ENCODE_BIT_KHR, "VUID-vkCmdSetEvent2-commandBuffer-cmdpool"},
    {VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_COMPUTE_BIT | VK_QUEUE_VIDEO_DECODE_BIT_KHR | VK_QUEUE_VIDEO_ENCODE_BIT_KHR, "VUID-vkCmdSetEvent2-commandBuffer-cmdpool"},
    {VK_QUEUE_GRAPHICS_BIT, "VUID-vkCmdSetExclusiveScissorNV-commandBuffer-cmdpool"},
    {VK_QUEUE_GRAPHICS_BIT, "VUID-vkCmdSetExtraPrimitiveOverestimationSizeEXT-commandBuffer-cmdpool"},
    {VK_QUEUE_GRAPHICS_BIT, "VUID-vkCmdSetFragmentShadingRateEnumNV-commandBuffer-cmdpool"},
    {VK_QUEUE_GRAPHICS_BIT, "VUID-vkCmdSetFragmentShadingRateKHR-commandBuffer-cmdpool"},
    {VK_QUEUE_GRAPHICS_BIT, "VUID-vkCmdSetFrontFace-commandBuffer-cmdpool"},
    {VK_QUEUE_GRAPHICS_BIT, "VUID-vkCmdSetFrontFace-commandBuffer-cmdpool"},
    {VK_QUEUE_GRAPHICS_BIT, "VUID-vkCmdSetLineRasterizationModeEXT-commandBuffer-cmdpool"},
    {VK_QUEUE_GRAPHICS_BIT, "VUID-vkCmdSetLineStippleEXT-commandBuffer-cmdpool"},
    {VK_QUEUE_GRAPHICS_BIT, "VUID-vkCmdSetLineStippleEnableEXT-commandBuffer-cmdpool"},
    {VK_QUEUE_GRAPHICS_BIT, "VUID-vkCmdSetLineWidth-commandBuffer-cmdpool"},
    {VK_QUEUE_GRAPHICS_BIT, "VUID-vkCmdSetLogicOpEXT-commandBuffer-cmdpool"},
    {VK_QUEUE_GRAPHICS_BIT, "VUID-vkCmdSetLogicOpEnableEXT-commandBuffer-cmdpool"},
    {VK_QUEUE_GRAPHICS_BIT, "VUID-vkCmdSetPatchControlPointsEXT-commandBuffer-cmdpool"},
    {VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_COMPUTE_BIT | VK_QUEUE_TRANSFER_BIT, "VUID-vkCmdSetPerformanceMarkerINTEL-commandBuffer-cmdpool"},
    {VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_COMPUTE_BIT | VK_QUEUE_TRANSFER_BIT, "VUID-vkCmdSetPerformanceOverrideINTEL-commandBuffer-cmdpool"},
    {VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_COMPUTE_BIT | VK_QUEUE_TRANSFER_BIT, "VUID-vkCmdSetPerformanceStreamMarkerINTEL-commandBuffer-cmdpool"},
    {VK_QUEUE_GRAPHICS_BIT, "VUID-vkCmdSetPolygonModeEXT-commandBuffer-cmdpool"},
    {VK_QUEUE_GRAPHICS_BIT, "VUID-vkCmdSetPrimitiveRestartEnable-commandBuffer-cmdpool"},
    {VK_QUEUE_GRAPHICS_BIT, "VUID-vkCmdSetPrimitiveRestartEnable-commandBuffer-cmdpool"},
    {VK_QUEUE_GRAPHICS_BIT, "VUID-vkCmdSetPrimitiveTopology-commandBuffer-cmdpool"},
    {VK_QUEUE_GRAPHICS_BIT, "VUID-vkCmdSetPrimitiveTopology-commandBuffer-cmdpool"},
    {VK_QUEUE_GRAPHICS_BIT, "VUID-vkCmdSetProvokingVertexModeEXT-commandBuffer-cmdpool"},
    {VK_QUEUE_GRAPHICS_BIT, "VUID-vkCmdSetRasterizationSamplesEXT-commandBuffer-cmdpool"},
    {VK_QUEUE_GRAPHICS_BIT, "VUID-vkCmdSetRasterizationStreamEXT-commandBuffer-cmdpool"},
    {VK_QUEUE_GRAPHICS_BIT, "VUID-vkCmdSetRasterizerDiscardEnable-commandBuffer-cmdpool"},
    {VK_QUEUE_GRAPHICS_BIT, "VUID-vkCmdSetRasterizerDiscardEnable-commandBuffer-cmdpool"},
    {VK_QUEUE_COMPUTE_BIT, "VUID-vkCmdSetRayTracingPipelineStackSizeKHR-commandBuffer-cmdpool"},
    {VK_QUEUE_GRAPHICS_BIT, "VUID-vkCmdSetRepresentativeFragmentTestEnableNV-commandBuffer-cmdpool"},
    {VK_QUEUE_GRAPHICS_BIT, "VUID-vkCmdSetSampleLocationsEXT-commandBuffer-cmdpool"},
    {VK_QUEUE_GRAPHICS_BIT, "VUID-vkCmdSetSampleLocationsEnableEXT-commandBuffer-cmdpool"},
    {VK_QUEUE_GRAPHICS_BIT, "VUID-vkCmdSetSampleMaskEXT-commandBuffer-cmdpool"},
    {VK_QUEUE_GRAPHICS_BIT, "VUID-vkCmdSetScissor-commandBuffer-cmdpool"},
    {VK_QUEUE_GRAPHICS_BIT, "VUID-vkCmdSetScissorWithCount-commandBuffer-cmdpool"},
    {VK_QUEUE_GRAPHICS_BIT, "VUID-vkCmdSetScissorWithCount-commandBuffer-cmdpool"},
    {VK_QUEUE_GRAPHICS_BIT, "VUID-vkCmdSetShadingRateImageEnableNV-commandBuffer-cmdpool"},
    {VK_QUEUE_GRAPHICS_BIT, "VUID-vkCmdSetStencilCompareMask-commandBuffer-cmdpool"},
    {VK_QUEUE_GRAPHICS_BIT, "VUID-vkCmdSetStencilOp-commandBuffer-cmdpool"},
    {VK_QUEUE_GRAPHICS_BIT, "VUID-vkCmdSetStencilOp-commandBuffer-cmdpool"},
    {VK_QUEUE_GRAPHICS_BIT, "VUID-vkCmdSetStencilReference-commandBuffer-cmdpool"},
    {VK_QUEUE_GRAPHICS_BIT, "VUID-vkCmdSetStencilTestEnable-commandBuffer-cmdpool"},
    {VK_QUEUE_GRAPHICS_BIT, "VUID-vkCmdSetStencilTestEnable-commandBuffer-cmdpool"},
    {VK_QUEUE_GRAPHICS_BIT, "VUID-vkCmdSetStencilWriteMask-commandBuffer-cmdpool"},
    {VK_QUEUE_GRAPHICS_BIT, "VUID-vkCmdSetTessellationDomainOriginEXT-commandBuffer-cmdpool"},
    {VK_QUEUE_GRAPHICS_BIT, "VUID-vkCmdSetVertexInputEXT-commandBuffer-cmdpool"},
    {VK_QUEUE_GRAPHICS_BIT, "VUID-vkCmdSetViewport-commandBuffer-cmdpool"},
    {VK_QUEUE_GRAPHICS_BIT, "VUID-vkCmdSetViewportShadingRatePaletteNV-commandBuffer-cmdpool"},
    {VK_QUEUE_GRAPHICS_BIT, "VUID-vkCmdSetViewportSwizzleNV-commandBuffer-cmdpool"},
    {VK_QUEUE_GRAPHICS_BIT, "VUID-vkCmdSetViewportWScalingEnableNV-commandBuffer-cmdpool"},
    {VK_QUEUE_GRAPHICS_BIT, "VUID-vkCmdSetViewportWScalingNV-commandBuffer-cmdpool"},
    {VK_QUEUE_GRAPHICS_BIT, "VUID-vkCmdSetViewportWithCount-commandBuffer-cmdpool"},
    {VK_QUEUE_GRAPHICS_BIT, "VUID-vkCmdSetViewportWithCount-commandBuffer-cmdpool"},
    {VK_QUEUE_GRAPHICS_BIT, "VUID-vkCmdSubpassShadingHUAWEI-commandBuffer-cmdpool"},
    {VK_QUEUE_COMPUTE_BIT, "VUID-vkCmdTraceRaysIndirect2KHR-commandBuffer-cmdpool"},
    {VK_QUEUE_COMPUTE_BIT, "VUID-vkCmdTraceRaysIndirectKHR-commandBuffer-cmdpool"},
    {VK_QUEUE_COMPUTE_BIT, "VUID-vkCmdTraceRaysKHR-commandBuffer-cmdpool"},
    {VK_QUEUE_COMPUTE_BIT, "VUID-vkCmdTraceRaysNV-commandBuffer-cmdpool"},
    {VK_QUEUE_TRANSFER_BIT | VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_COMPUTE_BIT, "VUID-vkCmdUpdateBuffer-commandBuffer-cmdpool"},
    {VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_COMPUTE_BIT | VK_QUEUE_VIDEO_DECODE_BIT_KHR | VK_QUEUE_VIDEO_ENCODE_BIT_KHR, "VUID-vkCmdWaitEvents-commandBuffer-cmdpool"},
    {VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_COMPUTE_BIT | VK_QUEUE_VIDEO_DECODE_BIT_KHR | VK_QUEUE_VIDEO_ENCODE_BIT_KHR, "VUID-vkCmdWaitEvents2-commandBuffer-cmdpool"},
    {VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_COMPUTE_BIT | VK_QUEUE_VIDEO_DECODE_BIT_KHR | VK_QUEUE_VIDEO_ENCODE_BIT_KHR, "VUID-vkCmdWaitEvents2-commandBuffer-cmdpool"},
    {VK_QUEUE_COMPUTE_BIT, "VUID-vkCmdWriteAccelerationStructuresPropertiesKHR-commandBuffer-cmdpool"},
    {VK_QUEUE_COMPUTE_BIT, "VUID-vkCmdWriteAccelerationStructuresPropertiesNV-commandBuffer-cmdpool"},
    {VK_QUEUE_TRANSFER_BIT | VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_COMPUTE_BIT, "VUID-vkCmdWriteBufferMarker2AMD-commandBuffer-cmdpool"},
    {VK_QUEUE_TRANSFER_BIT | VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_COMPUTE_BIT, "VUID-vkCmdWriteBufferMarkerAMD-commandBuffer-cmdpool"},
    {VK_QUEUE_COMPUTE_BIT, "VUID-vkCmdWriteMicromapsPropertiesEXT-commandBuffer-cmdpool"},
    {VK_QUEUE_TRANSFER_BIT | VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_COMPUTE_BIT | VK_QUEUE_VIDEO_DECODE_BIT_KHR | VK_QUEUE_VIDEO_ENCODE_BIT_KHR | VK_QUEUE_OPTICAL_FLOW_BIT_NV, "VUID-vkCmdWriteTimestamp-commandBuffer-cmdpool"},
    {VK_QUEUE_TRANSFER_BIT | VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_COMPUTE_BIT | VK_QUEUE_VIDEO_DECODE_BIT_KHR | VK_QUEUE_VIDEO_ENCODE_BIT_KHR, "VUID-vkCmdWriteTimestamp2-commandBuffer-cmdpool"},
    {VK_QUEUE_TRANSFER_BIT | VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_COMPUTE_BIT | VK_QUEUE_VIDEO_DECODE_BIT_KHR | VK_QUEUE_VIDEO_ENCODE_BIT_KHR, "VUID-vkCmdWriteTimestamp2-commandBuffer-cmdpool"},
}};

enum CMD_RENDER_PASS_TYPE {
    CMD_RENDER_PASS_INSIDE,
    CMD_RENDER_PASS_OUTSIDE,
    CMD_RENDER_PASS_BOTH
};
struct CommandSupportedRenderPass {
    CMD_RENDER_PASS_TYPE renderPass;
    const char* vuid;
};
static const std::array<CommandSupportedRenderPass, CMD_RANGE_SIZE> kGeneratedRenderPassList = {{
    {CMD_RENDER_PASS_BOTH, kVUIDUndefined}, // CMD_NONE
    {CMD_RENDER_PASS_BOTH, kVUIDUndefined},
    {CMD_RENDER_PASS_BOTH, kVUIDUndefined},
    {CMD_RENDER_PASS_BOTH, kVUIDUndefined},
    {CMD_RENDER_PASS_BOTH, kVUIDUndefined},
    {CMD_RENDER_PASS_OUTSIDE, "VUID-vkCmdBeginRenderPass-renderpass"},
    {CMD_RENDER_PASS_OUTSIDE, "VUID-vkCmdBeginRenderPass2-renderpass"},
    {CMD_RENDER_PASS_OUTSIDE, "VUID-vkCmdBeginRenderPass2-renderpass"},
    {CMD_RENDER_PASS_OUTSIDE, "VUID-vkCmdBeginRendering-renderpass"},
    {CMD_RENDER_PASS_OUTSIDE, "VUID-vkCmdBeginRendering-renderpass"},
    {CMD_RENDER_PASS_INSIDE, "VUID-vkCmdBeginTransformFeedbackEXT-renderpass"},
    {CMD_RENDER_PASS_OUTSIDE, "VUID-vkCmdBeginVideoCodingKHR-renderpass"},
    {CMD_RENDER_PASS_BOTH, kVUIDUndefined},
    {CMD_RENDER_PASS_BOTH, kVUIDUndefined},
    {CMD_RENDER_PASS_BOTH, kVUIDUndefined},
    {CMD_RENDER_PASS_BOTH, kVUIDUndefined},
    {CMD_RENDER_PASS_OUTSIDE, "VUID-vkCmdBindInvocationMaskHUAWEI-renderpass"},
    {CMD_RENDER_PASS_BOTH, kVUIDUndefined},
    {CMD_RENDER_PASS_BOTH, kVUIDUndefined},
    {CMD_RENDER_PASS_BOTH, kVUIDUndefined},
    {CMD_RENDER_PASS_BOTH, kVUIDUndefined},
    {CMD_RENDER_PASS_BOTH, kVUIDUndefined},
    {CMD_RENDER_PASS_BOTH, kVUIDUndefined},
    {CMD_RENDER_PASS_BOTH, kVUIDUndefined},
    {CMD_RENDER_PASS_OUTSIDE, "VUID-vkCmdBlitImage-renderpass"},
    {CMD_RENDER_PASS_OUTSIDE, "VUID-vkCmdBlitImage2-renderpass"},
    {CMD_RENDER_PASS_OUTSIDE, "VUID-vkCmdBlitImage2-renderpass"},
    {CMD_RENDER_PASS_OUTSIDE, "VUID-vkCmdBuildAccelerationStructureNV-renderpass"},
    {CMD_RENDER_PASS_OUTSIDE, "VUID-vkCmdBuildAccelerationStructuresIndirectKHR-renderpass"},
    {CMD_RENDER_PASS_OUTSIDE, "VUID-vkCmdBuildAccelerationStructuresKHR-renderpass"},
    {CMD_RENDER_PASS_OUTSIDE, "VUID-vkCmdBuildMicromapsEXT-renderpass"},
    {CMD_RENDER_PASS_INSIDE, "VUID-vkCmdClearAttachments-renderpass"},
    {CMD_RENDER_PASS_OUTSIDE, "VUID-vkCmdClearColorImage-renderpass"},
    {CMD_RENDER_PASS_OUTSIDE, "VUID-vkCmdClearDepthStencilImage-renderpass"},
    {CMD_RENDER_PASS_OUTSIDE, "VUID-vkCmdControlVideoCodingKHR-renderpass"},
    {CMD_RENDER_PASS_OUTSIDE, "VUID-vkCmdCopyAccelerationStructureKHR-renderpass"},
    {CMD_RENDER_PASS_OUTSIDE, "VUID-vkCmdCopyAccelerationStructureNV-renderpass"},
    {CMD_RENDER_PASS_OUTSIDE, "VUID-vkCmdCopyAccelerationStructureToMemoryKHR-renderpass"},
    {CMD_RENDER_PASS_OUTSIDE, "VUID-vkCmdCopyBuffer-renderpass"},
    {CMD_RENDER_PASS_OUTSIDE, "VUID-vkCmdCopyBuffer2-renderpass"},
    {CMD_RENDER_PASS_OUTSIDE, "VUID-vkCmdCopyBuffer2-renderpass"},
    {CMD_RENDER_PASS_OUTSIDE, "VUID-vkCmdCopyBufferToImage-renderpass"},
    {CMD_RENDER_PASS_OUTSIDE, "VUID-vkCmdCopyBufferToImage2-renderpass"},
    {CMD_RENDER_PASS_OUTSIDE, "VUID-vkCmdCopyBufferToImage2-renderpass"},
    {CMD_RENDER_PASS_OUTSIDE, "VUID-vkCmdCopyImage-renderpass"},
    {CMD_RENDER_PASS_OUTSIDE, "VUID-vkCmdCopyImage2-renderpass"},
    {CMD_RENDER_PASS_OUTSIDE, "VUID-vkCmdCopyImage2-renderpass"},
    {CMD_RENDER_PASS_OUTSIDE, "VUID-vkCmdCopyImageToBuffer-renderpass"},
    {CMD_RENDER_PASS_OUTSIDE, "VUID-vkCmdCopyImageToBuffer2-renderpass"},
    {CMD_RENDER_PASS_OUTSIDE, "VUID-vkCmdCopyImageToBuffer2-renderpass"},
    {CMD_RENDER_PASS_OUTSIDE, "VUID-vkCmdCopyMemoryIndirectNV-renderpass"},
    {CMD_RENDER_PASS_OUTSIDE, "VUID-vkCmdCopyMemoryToAccelerationStructureKHR-renderpass"},
    {CMD_RENDER_PASS_OUTSIDE, "VUID-vkCmdCopyMemoryToImageIndirectNV-renderpass"},
    {CMD_RENDER_PASS_OUTSIDE, "VUID-vkCmdCopyMemoryToMicromapEXT-renderpass"},
    {CMD_RENDER_PASS_OUTSIDE, "VUID-vkCmdCopyMicromapEXT-renderpass"},
    {CMD_RENDER_PASS_OUTSIDE, "VUID-vkCmdCopyMicromapToMemoryEXT-renderpass"},
    {CMD_RENDER_PASS_OUTSIDE, "VUID-vkCmdCopyQueryPoolResults-renderpass"},
    {CMD_RENDER_PASS_BOTH, kVUIDUndefined},
    {CMD_RENDER_PASS_BOTH, kVUIDUndefined},
    {CMD_RENDER_PASS_BOTH, kVUIDUndefined},
    {CMD_RENDER_PASS_BOTH, kVUIDUndefined},
    {CMD_RENDER_PASS_OUTSIDE, "VUID-vkCmdDecodeVideoKHR-renderpass"},
    {CMD_RENDER_PASS_OUTSIDE, "VUID-vkCmdDecompressMemoryIndirectCountNV-renderpass"},
    {CMD_RENDER_PASS_OUTSIDE, "VUID-vkCmdDecompressMemoryNV-renderpass"},
    {CMD_RENDER_PASS_OUTSIDE, "VUID-vkCmdDispatch-renderpass"},
    {CMD_RENDER_PASS_OUTSIDE, "VUID-vkCmdDispatchBase-renderpass"},
    {CMD_RENDER_PASS_OUTSIDE, "VUID-vkCmdDispatchBase-renderpass"},
    {CMD_RENDER_PASS_OUTSIDE, "VUID-vkCmdDispatchIndirect-renderpass"},
    {CMD_RENDER_PASS_INSIDE, "VUID-vkCmdDraw-renderpass"},
    {CMD_RENDER_PASS_INSIDE, "VUID-vkCmdDrawClusterHUAWEI-renderpass"},
    {CMD_RENDER_PASS_INSIDE, "VUID-vkCmdDrawClusterIndirectHUAWEI-renderpass"},
    {CMD_RENDER_PASS_INSIDE, "VUID-vkCmdDrawIndexed-renderpass"},
    {CMD_RENDER_PASS_INSIDE, "VUID-vkCmdDrawIndexedIndirect-renderpass"},
    {CMD_RENDER_PASS_INSIDE, "VUID-vkCmdDrawIndexedIndirectCount-renderpass"},
    {CMD_RENDER_PASS_INSIDE, "VUID-vkCmdDrawIndexedIndirectCount-renderpass"},
    {CMD_RENDER_PASS_INSIDE, "VUID-vkCmdDrawIndexedIndirectCount-renderpass"},
    {CMD_RENDER_PASS_INSIDE, "VUID-vkCmdDrawIndirect-renderpass"},
    {CMD_RENDER_PASS_INSIDE, "VUID-vkCmdDrawIndirectByteCountEXT-renderpass"},
    {CMD_RENDER_PASS_INSIDE, "VUID-vkCmdDrawIndirectCount-renderpass"},
    {CMD_RENDER_PASS_INSIDE, "VUID-vkCmdDrawIndirectCount-renderpass"},
    {CMD_RENDER_PASS_INSIDE, "VUID-vkCmdDrawIndirectCount-renderpass"},
    {CMD_RENDER_PASS_INSIDE, "VUID-vkCmdDrawMeshTasksEXT-renderpass"},
    {CMD_RENDER_PASS_INSIDE, "VUID-vkCmdDrawMeshTasksIndirectCountEXT-renderpass"},
    {CMD_RENDER_PASS_INSIDE, "VUID-vkCmdDrawMeshTasksIndirectCountNV-renderpass"},
    {CMD_RENDER_PASS_INSIDE, "VUID-vkCmdDrawMeshTasksIndirectEXT-renderpass"},
    {CMD_RENDER_PASS_INSIDE, "VUID-vkCmdDrawMeshTasksIndirectNV-renderpass"},
    {CMD_RENDER_PASS_INSIDE, "VUID-vkCmdDrawMeshTasksNV-renderpass"},
    {CMD_RENDER_PASS_INSIDE, "VUID-vkCmdDrawMultiEXT-renderpass"},
    {CMD_RENDER_PASS_INSIDE, "VUID-vkCmdDrawMultiIndexedEXT-renderpass"},
    {CMD_RENDER_PASS_OUTSIDE, "VUID-vkCmdEncodeVideoKHR-renderpass"},
    {CMD_RENDER_PASS_BOTH, kVUIDUndefined},
    {CMD_RENDER_PASS_BOTH, kVUIDUndefined},
    {CMD_RENDER_PASS_BOTH, kVUIDUndefined},
    {CMD_RENDER_PASS_BOTH, kVUIDUndefined},
    {CMD_RENDER_PASS_INSIDE, "VUID-vkCmdEndRenderPass-renderpass"},
    {CMD_RENDER_PASS_INSIDE, "VUID-vkCmdEndRenderPass2-renderpass"},
    {CMD_RENDER_PASS_INSIDE, "VUID-vkCmdEndRenderPass2-renderpass"},
    {CMD_RENDER_PASS_INSIDE, "VUID-vkCmdEndRendering-renderpass"},
    {CMD_RENDER_PASS_INSIDE, "VUID-vkCmdEndRendering-renderpass"},
    {CMD_RENDER_PASS_INSIDE, "VUID-vkCmdEndTransformFeedbackEXT-renderpass"},
    {CMD_RENDER_PASS_OUTSIDE, "VUID-vkCmdEndVideoCodingKHR-renderpass"},
    {CMD_RENDER_PASS_BOTH, kVUIDUndefined},
    {CMD_RENDER_PASS_INSIDE, "VUID-vkCmdExecuteGeneratedCommandsNV-renderpass"},
    {CMD_RENDER_PASS_OUTSIDE, "VUID-vkCmdFillBuffer-renderpass"},
    {CMD_RENDER_PASS_BOTH, kVUIDUndefined},
    {CMD_RENDER_PASS_INSIDE, "VUID-vkCmdNextSubpass-renderpass"},
    {CMD_RENDER_PASS_INSIDE, "VUID-vkCmdNextSubpass2-renderpass"},
    {CMD_RENDER_PASS_INSIDE, "VUID-vkCmdNextSubpass2-renderpass"},
    {CMD_RENDER_PASS_OUTSIDE, "VUID-vkCmdOpticalFlowExecuteNV-renderpass"},
    {CMD_RENDER_PASS_BOTH, kVUIDUndefined},
    {CMD_RENDER_PASS_BOTH, kVUIDUndefined},
    {CMD_RENDER_PASS_BOTH, kVUIDUndefined},
    {CMD_RENDER_PASS_OUTSIDE, "VUID-vkCmdPreprocessGeneratedCommandsNV-renderpass"},
    {CMD_RENDER_PASS_BOTH, kVUIDUndefined},
    {CMD_RENDER_PASS_BOTH, kVUIDUndefined},
    {CMD_RENDER_PASS_BOTH, kVUIDUndefined},
    {CMD_RENDER_PASS_OUTSIDE, "VUID-vkCmdResetEvent-renderpass"},
    {CMD_RENDER_PASS_OUTSIDE, "VUID-vkCmdResetEvent2-renderpass"},
    {CMD_RENDER_PASS_OUTSIDE, "VUID-vkCmdResetEvent2-renderpass"},
    {CMD_RENDER_PASS_OUTSIDE, "VUID-vkCmdResetQueryPool-renderpass"},
    {CMD_RENDER_PASS_OUTSIDE, "VUID-vkCmdResolveImage-renderpass"},
    {CMD_RENDER_PASS_OUTSIDE, "VUID-vkCmdResolveImage2-renderpass"},
    {CMD_RENDER_PASS_OUTSIDE, "VUID-vkCmdResolveImage2-renderpass"},
    {CMD_RENDER_PASS_BOTH, kVUIDUndefined},
    {CMD_RENDER_PASS_BOTH, kVUIDUndefined},
    {CMD_RENDER_PASS_BOTH, kVUIDUndefined},
    {CMD_RENDER_PASS_BOTH, kVUIDUndefined},
    {CMD_RENDER_PASS_BOTH, kVUIDUndefined},
    {CMD_RENDER_PASS_BOTH, kVUIDUndefined},
    {CMD_RENDER_PASS_BOTH, kVUIDUndefined},
    {CMD_RENDER_PASS_BOTH, kVUIDUndefined},
    {CMD_RENDER_PASS_BOTH, kVUIDUndefined},
    {CMD_RENDER_PASS_BOTH, kVUIDUndefined},
    {CMD_RENDER_PASS_BOTH, kVUIDUndefined},
    {CMD_RENDER_PASS_BOTH, kVUIDUndefined},
    {CMD_RENDER_PASS_BOTH, kVUIDUndefined},
    {CMD_RENDER_PASS_BOTH, kVUIDUndefined},
    {CMD_RENDER_PASS_BOTH, kVUIDUndefined},
    {CMD_RENDER_PASS_BOTH, kVUIDUndefined},
    {CMD_RENDER_PASS_BOTH, kVUIDUndefined},
    {CMD_RENDER_PASS_BOTH, kVUIDUndefined},
    {CMD_RENDER_PASS_BOTH, kVUIDUndefined},
    {CMD_RENDER_PASS_BOTH, kVUIDUndefined},
    {CMD_RENDER_PASS_BOTH, kVUIDUndefined},
    {CMD_RENDER_PASS_BOTH, kVUIDUndefined},
    {CMD_RENDER_PASS_BOTH, kVUIDUndefined},
    {CMD_RENDER_PASS_BOTH, kVUIDUndefined},
    {CMD_RENDER_PASS_BOTH, kVUIDUndefined},
    {CMD_RENDER_PASS_BOTH, kVUIDUndefined},
    {CMD_RENDER_PASS_BOTH, kVUIDUndefined},
    {CMD_RENDER_PASS_BOTH, kVUIDUndefined},
    {CMD_RENDER_PASS_BOTH, kVUIDUndefined},
    {CMD_RENDER_PASS_BOTH, kVUIDUndefined},
    {CMD_RENDER_PASS_BOTH, kVUIDUndefined},
    {CMD_RENDER_PASS_BOTH, kVUIDUndefined},
    {CMD_RENDER_PASS_BOTH, kVUIDUndefined},
    {CMD_RENDER_PASS_BOTH, kVUIDUndefined},
    {CMD_RENDER_PASS_BOTH, kVUIDUndefined},
    {CMD_RENDER_PASS_BOTH, kVUIDUndefined},
    {CMD_RENDER_PASS_BOTH, kVUIDUndefined},
    {CMD_RENDER_PASS_BOTH, kVUIDUndefined},
    {CMD_RENDER_PASS_OUTSIDE, "VUID-vkCmdSetEvent-renderpass"},
    {CMD_RENDER_PASS_OUTSIDE, "VUID-vkCmdSetEvent2-renderpass"},
    {CMD_RENDER_PASS_OUTSIDE, "VUID-vkCmdSetEvent2-renderpass"},
    {CMD_RENDER_PASS_BOTH, kVUIDUndefined},
    {CMD_RENDER_PASS_BOTH, kVUIDUndefined},
    {CMD_RENDER_PASS_BOTH, kVUIDUndefined},
    {CMD_RENDER_PASS_BOTH, kVUIDUndefined},
    {CMD_RENDER_PASS_BOTH, kVUIDUndefined},
    {CMD_RENDER_PASS_BOTH, kVUIDUndefined},
    {CMD_RENDER_PASS_BOTH, kVUIDUndefined},
    {CMD_RENDER_PASS_BOTH, kVUIDUndefined},
    {CMD_RENDER_PASS_BOTH, kVUIDUndefined},
    {CMD_RENDER_PASS_BOTH, kVUIDUndefined},
    {CMD_RENDER_PASS_BOTH, kVUIDUndefined},
    {CMD_RENDER_PASS_BOTH, kVUIDUndefined},
    {CMD_RENDER_PASS_BOTH, kVUIDUndefined},
    {CMD_RENDER_PASS_BOTH, kVUIDUndefined},
    {CMD_RENDER_PASS_BOTH, kVUIDUndefined},
    {CMD_RENDER_PASS_BOTH, kVUIDUndefined},
    {CMD_RENDER_PASS_BOTH, kVUIDUndefined},
    {CMD_RENDER_PASS_BOTH, kVUIDUndefined},
    {CMD_RENDER_PASS_BOTH, kVUIDUndefined},
    {CMD_RENDER_PASS_BOTH, kVUIDUndefined},
    {CMD_RENDER_PASS_BOTH, kVUIDUndefined},
    {CMD_RENDER_PASS_BOTH, kVUIDUndefined},
    {CMD_RENDER_PASS_BOTH, kVUIDUndefined},
    {CMD_RENDER_PASS_BOTH, kVUIDUndefined},
    {CMD_RENDER_PASS_BOTH, kVUIDUndefined},
    {CMD_RENDER_PASS_BOTH, kVUIDUndefined},
    {CMD_RENDER_PASS_OUTSIDE, "VUID-vkCmdSetRayTracingPipelineStackSizeKHR-renderpass"},
    {CMD_RENDER_PASS_BOTH, kVUIDUndefined},
    {CMD_RENDER_PASS_BOTH, kVUIDUndefined},
    {CMD_RENDER_PASS_BOTH, kVUIDUndefined},
    {CMD_RENDER_PASS_BOTH, kVUIDUndefined},
    {CMD_RENDER_PASS_BOTH, kVUIDUndefined},
    {CMD_RENDER_PASS_BOTH, kVUIDUndefined},
    {CMD_RENDER_PASS_BOTH, kVUIDUndefined},
    {CMD_RENDER_PASS_BOTH, kVUIDUndefined},
    {CMD_RENDER_PASS_BOTH, kVUIDUndefined},
    {CMD_RENDER_PASS_BOTH, kVUIDUndefined},
    {CMD_RENDER_PASS_BOTH, kVUIDUndefined},
    {CMD_RENDER_PASS_BOTH, kVUIDUndefined},
    {CMD_RENDER_PASS_BOTH, kVUIDUndefined},
    {CMD_RENDER_PASS_BOTH, kVUIDUndefined},
    {CMD_RENDER_PASS_BOTH, kVUIDUndefined},
    {CMD_RENDER_PASS_BOTH, kVUIDUndefined},
    {CMD_RENDER_PASS_BOTH, kVUIDUndefined},
    {CMD_RENDER_PASS_BOTH, kVUIDUndefined},
    {CMD_RENDER_PASS_BOTH, kVUIDUndefined},
    {CMD_RENDER_PASS_BOTH, kVUIDUndefined},
    {CMD_RENDER_PASS_BOTH, kVUIDUndefined},
    {CMD_RENDER_PASS_BOTH, kVUIDUndefined},
    {CMD_RENDER_PASS_BOTH, kVUIDUndefined},
    {CMD_RENDER_PASS_BOTH, kVUIDUndefined},
    {CMD_RENDER_PASS_INSIDE, "VUID-vkCmdSubpassShadingHUAWEI-renderpass"},
    {CMD_RENDER_PASS_OUTSIDE, "VUID-vkCmdTraceRaysIndirect2KHR-renderpass"},
    {CMD_RENDER_PASS_OUTSIDE, "VUID-vkCmdTraceRaysIndirectKHR-renderpass"},
    {CMD_RENDER_PASS_OUTSIDE, "VUID-vkCmdTraceRaysKHR-renderpass"},
    {CMD_RENDER_PASS_OUTSIDE, "VUID-vkCmdTraceRaysNV-renderpass"},
    {CMD_RENDER_PASS_OUTSIDE, "VUID-vkCmdUpdateBuffer-renderpass"},
    {CMD_RENDER_PASS_BOTH, kVUIDUndefined},
    {CMD_RENDER_PASS_BOTH, kVUIDUndefined},
    {CMD_RENDER_PASS_BOTH, kVUIDUndefined},
    {CMD_RENDER_PASS_OUTSIDE, "VUID-vkCmdWriteAccelerationStructuresPropertiesKHR-renderpass"},
    {CMD_RENDER_PASS_OUTSIDE, "VUID-vkCmdWriteAccelerationStructuresPropertiesNV-renderpass"},
    {CMD_RENDER_PASS_BOTH, kVUIDUndefined},
    {CMD_RENDER_PASS_BOTH, kVUIDUndefined},
    {CMD_RENDER_PASS_OUTSIDE, "VUID-vkCmdWriteMicromapsPropertiesEXT-renderpass"},
    {CMD_RENDER_PASS_BOTH, kVUIDUndefined},
    {CMD_RENDER_PASS_BOTH, kVUIDUndefined},
    {CMD_RENDER_PASS_BOTH, kVUIDUndefined},
}};

enum CMD_VIDEO_CODING_TYPE {
    CMD_VIDEO_CODING_INSIDE,
    CMD_VIDEO_CODING_OUTSIDE,
    CMD_VIDEO_CODING_BOTH
};
struct CommandSupportedVideoCoding {
    CMD_VIDEO_CODING_TYPE videoCoding;
    const char* vuid;
};
static const std::array<CommandSupportedVideoCoding, CMD_RANGE_SIZE> kGeneratedVideoCodingList = {{
    {CMD_VIDEO_CODING_BOTH, kVUIDUndefined}, // CMD_NONE
    {CMD_VIDEO_CODING_OUTSIDE, "VUID-vkCmdBeginConditionalRenderingEXT-videocoding"},
    {CMD_VIDEO_CODING_OUTSIDE, "VUID-vkCmdBeginDebugUtilsLabelEXT-videocoding"},
    {CMD_VIDEO_CODING_BOTH, kVUIDUndefined},
    {CMD_VIDEO_CODING_OUTSIDE, "VUID-vkCmdBeginQueryIndexedEXT-videocoding"},
    {CMD_VIDEO_CODING_OUTSIDE, "VUID-vkCmdBeginRenderPass-videocoding"},
    {CMD_VIDEO_CODING_OUTSIDE, "VUID-vkCmdBeginRenderPass2-videocoding"},
    {CMD_VIDEO_CODING_OUTSIDE, "VUID-vkCmdBeginRenderPass2-videocoding"},
    {CMD_VIDEO_CODING_OUTSIDE, "VUID-vkCmdBeginRendering-videocoding"},
    {CMD_VIDEO_CODING_OUTSIDE, "VUID-vkCmdBeginRendering-videocoding"},
    {CMD_VIDEO_CODING_OUTSIDE, "VUID-vkCmdBeginTransformFeedbackEXT-videocoding"},
    {CMD_VIDEO_CODING_OUTSIDE, "VUID-vkCmdBeginVideoCodingKHR-videocoding"},
    {CMD_VIDEO_CODING_OUTSIDE, "VUID-vkCmdBindDescriptorBufferEmbeddedSamplersEXT-videocoding"},
    {CMD_VIDEO_CODING_OUTSIDE, "VUID-vkCmdBindDescriptorBuffersEXT-videocoding"},
    {CMD_VIDEO_CODING_OUTSIDE, "VUID-vkCmdBindDescriptorSets-videocoding"},
    {CMD_VIDEO_CODING_OUTSIDE, "VUID-vkCmdBindIndexBuffer-videocoding"},
    {CMD_VIDEO_CODING_OUTSIDE, "VUID-vkCmdBindInvocationMaskHUAWEI-videocoding"},
    {CMD_VIDEO_CODING_OUTSIDE, "VUID-vkCmdBindPipeline-videocoding"},
    {CMD_VIDEO_CODING_OUTSIDE, "VUID-vkCmdBindPipelineShaderGroupNV-videocoding"},
    {CMD_VIDEO_CODING_OUTSIDE, "VUID-vkCmdBindShadingRateImageNV-videocoding"},
    {CMD_VIDEO_CODING_OUTSIDE, "VUID-vkCmdBindTransformFeedbackBuffersEXT-videocoding"},
    {CMD_VIDEO_CODING_OUTSIDE, "VUID-vkCmdBindVertexBuffers-videocoding"},
    {CMD_VIDEO_CODING_OUTSIDE, "VUID-vkCmdBindVertexBuffers2-videocoding"},
    {CMD_VIDEO_CODING_OUTSIDE, "VUID-vkCmdBindVertexBuffers2-videocoding"},
    {CMD_VIDEO_CODING_OUTSIDE, "VUID-vkCmdBlitImage-videocoding"},
    {CMD_VIDEO_CODING_OUTSIDE, "VUID-vkCmdBlitImage2-videocoding"},
    {CMD_VIDEO_CODING_OUTSIDE, "VUID-vkCmdBlitImage2-videocoding"},
    {CMD_VIDEO_CODING_OUTSIDE, "VUID-vkCmdBuildAccelerationStructureNV-videocoding"},
    {CMD_VIDEO_CODING_OUTSIDE, "VUID-vkCmdBuildAccelerationStructuresIndirectKHR-videocoding"},
    {CMD_VIDEO_CODING_OUTSIDE, "VUID-vkCmdBuildAccelerationStructuresKHR-videocoding"},
    {CMD_VIDEO_CODING_OUTSIDE, "VUID-vkCmdBuildMicromapsEXT-videocoding"},
    {CMD_VIDEO_CODING_OUTSIDE, "VUID-vkCmdClearAttachments-videocoding"},
    {CMD_VIDEO_CODING_OUTSIDE, "VUID-vkCmdClearColorImage-videocoding"},
    {CMD_VIDEO_CODING_OUTSIDE, "VUID-vkCmdClearDepthStencilImage-videocoding"},
    {CMD_VIDEO_CODING_INSIDE, "VUID-vkCmdControlVideoCodingKHR-videocoding"},
    {CMD_VIDEO_CODING_OUTSIDE, "VUID-vkCmdCopyAccelerationStructureKHR-videocoding"},
    {CMD_VIDEO_CODING_OUTSIDE, "VUID-vkCmdCopyAccelerationStructureNV-videocoding"},
    {CMD_VIDEO_CODING_OUTSIDE, "VUID-vkCmdCopyAccelerationStructureToMemoryKHR-videocoding"},
    {CMD_VIDEO_CODING_OUTSIDE, "VUID-vkCmdCopyBuffer-videocoding"},
    {CMD_VIDEO_CODING_OUTSIDE, "VUID-vkCmdCopyBuffer2-videocoding"},
    {CMD_VIDEO_CODING_OUTSIDE, "VUID-vkCmdCopyBuffer2-videocoding"},
    {CMD_VIDEO_CODING_OUTSIDE, "VUID-vkCmdCopyBufferToImage-videocoding"},
    {CMD_VIDEO_CODING_OUTSIDE, "VUID-vkCmdCopyBufferToImage2-videocoding"},
    {CMD_VIDEO_CODING_OUTSIDE, "VUID-vkCmdCopyBufferToImage2-videocoding"},
    {CMD_VIDEO_CODING_OUTSIDE, "VUID-vkCmdCopyImage-videocoding"},
    {CMD_VIDEO_CODING_OUTSIDE, "VUID-vkCmdCopyImage2-videocoding"},
    {CMD_VIDEO_CODING_OUTSIDE, "VUID-vkCmdCopyImage2-videocoding"},
    {CMD_VIDEO_CODING_OUTSIDE, "VUID-vkCmdCopyImageToBuffer-videocoding"},
    {CMD_VIDEO_CODING_OUTSIDE, "VUID-vkCmdCopyImageToBuffer2-videocoding"},
    {CMD_VIDEO_CODING_OUTSIDE, "VUID-vkCmdCopyImageToBuffer2-videocoding"},
    {CMD_VIDEO_CODING_OUTSIDE, "VUID-vkCmdCopyMemoryIndirectNV-videocoding"},
    {CMD_VIDEO_CODING_OUTSIDE, "VUID-vkCmdCopyMemoryToAccelerationStructureKHR-videocoding"},
    {CMD_VIDEO_CODING_OUTSIDE, "VUID-vkCmdCopyMemoryToImageIndirectNV-videocoding"},
    {CMD_VIDEO_CODING_OUTSIDE, "VUID-vkCmdCopyMemoryToMicromapEXT-videocoding"},
    {CMD_VIDEO_CODING_OUTSIDE, "VUID-vkCmdCopyMicromapEXT-videocoding"},
    {CMD_VIDEO_CODING_OUTSIDE, "VUID-vkCmdCopyMicromapToMemoryEXT-videocoding"},
    {CMD_VIDEO_CODING_OUTSIDE, "VUID-vkCmdCopyQueryPoolResults-videocoding"},
    {CMD_VIDEO_CODING_OUTSIDE, "VUID-vkCmdCuLaunchKernelNVX-videocoding"},
    {CMD_VIDEO_CODING_OUTSIDE, "VUID-vkCmdDebugMarkerBeginEXT-videocoding"},
    {CMD_VIDEO_CODING_OUTSIDE, "VUID-vkCmdDebugMarkerEndEXT-videocoding"},
    {CMD_VIDEO_CODING_OUTSIDE, "VUID-vkCmdDebugMarkerInsertEXT-videocoding"},
    {CMD_VIDEO_CODING_INSIDE, "VUID-vkCmdDecodeVideoKHR-videocoding"},
    {CMD_VIDEO_CODING_OUTSIDE, "VUID-vkCmdDecompressMemoryIndirectCountNV-videocoding"},
    {CMD_VIDEO_CODING_OUTSIDE, "VUID-vkCmdDecompressMemoryNV-videocoding"},
    {CMD_VIDEO_CODING_OUTSIDE, "VUID-vkCmdDispatch-videocoding"},
    {CMD_VIDEO_CODING_OUTSIDE, "VUID-vkCmdDispatchBase-videocoding"},
    {CMD_VIDEO_CODING_OUTSIDE, "VUID-vkCmdDispatchBase-videocoding"},
    {CMD_VIDEO_CODING_OUTSIDE, "VUID-vkCmdDispatchIndirect-videocoding"},
    {CMD_VIDEO_CODING_OUTSIDE, "VUID-vkCmdDraw-videocoding"},
    {CMD_VIDEO_CODING_OUTSIDE, "VUID-vkCmdDrawClusterHUAWEI-videocoding"},
    {CMD_VIDEO_CODING_OUTSIDE, "VUID-vkCmdDrawClusterIndirectHUAWEI-videocoding"},
    {CMD_VIDEO_CODING_OUTSIDE, "VUID-vkCmdDrawIndexed-videocoding"},
    {CMD_VIDEO_CODING_OUTSIDE, "VUID-vkCmdDrawIndexedIndirect-videocoding"},
    {CMD_VIDEO_CODING_OUTSIDE, "VUID-vkCmdDrawIndexedIndirectCount-videocoding"},
    {CMD_VIDEO_CODING_OUTSIDE, "VUID-vkCmdDrawIndexedIndirectCount-videocoding"},
    {CMD_VIDEO_CODING_OUTSIDE, "VUID-vkCmdDrawIndexedIndirectCount-videocoding"},
    {CMD_VIDEO_CODING_OUTSIDE, "VUID-vkCmdDrawIndirect-videocoding"},
    {CMD_VIDEO_CODING_OUTSIDE, "VUID-vkCmdDrawIndirectByteCountEXT-videocoding"},
    {CMD_VIDEO_CODING_OUTSIDE, "VUID-vkCmdDrawIndirectCount-videocoding"},
    {CMD_VIDEO_CODING_OUTSIDE, "VUID-vkCmdDrawIndirectCount-videocoding"},
    {CMD_VIDEO_CODING_OUTSIDE, "VUID-vkCmdDrawIndirectCount-videocoding"},
    {CMD_VIDEO_CODING_OUTSIDE, "VUID-vkCmdDrawMeshTasksEXT-videocoding"},
    {CMD_VIDEO_CODING_OUTSIDE, "VUID-vkCmdDrawMeshTasksIndirectCountEXT-videocoding"},
    {CMD_VIDEO_CODING_OUTSIDE, "VUID-vkCmdDrawMeshTasksIndirectCountNV-videocoding"},
    {CMD_VIDEO_CODING_OUTSIDE, "VUID-vkCmdDrawMeshTasksIndirectEXT-videocoding"},
    {CMD_VIDEO_CODING_OUTSIDE, "VUID-vkCmdDrawMeshTasksIndirectNV-videocoding"},
    {CMD_VIDEO_CODING_OUTSIDE, "VUID-vkCmdDrawMeshTasksNV-videocoding"},
    {CMD_VIDEO_CODING_OUTSIDE, "VUID-vkCmdDrawMultiEXT-videocoding"},
    {CMD_VIDEO_CODING_OUTSIDE, "VUID-vkCmdDrawMultiIndexedEXT-videocoding"},
    {CMD_VIDEO_CODING_INSIDE, "VUID-vkCmdEncodeVideoKHR-videocoding"},
    {CMD_VIDEO_CODING_OUTSIDE, "VUID-vkCmdEndConditionalRenderingEXT-videocoding"},
    {CMD_VIDEO_CODING_OUTSIDE, "VUID-vkCmdEndDebugUtilsLabelEXT-videocoding"},
    {CMD_VIDEO_CODING_BOTH, kVUIDUndefined},
    {CMD_VIDEO_CODING_OUTSIDE, "VUID-vkCmdEndQueryIndexedEXT-videocoding"},
    {CMD_VIDEO_CODING_OUTSIDE, "VUID-vkCmdEndRenderPass-videocoding"},
    {CMD_VIDEO_CODING_OUTSIDE, "VUID-vkCmdEndRenderPass2-videocoding"},
    {CMD_VIDEO_CODING_OUTSIDE, "VUID-vkCmdEndRenderPass2-videocoding"},
    {CMD_VIDEO_CODING_OUTSIDE, "VUID-vkCmdEndRendering-videocoding"},
    {CMD_VIDEO_CODING_OUTSIDE, "VUID-vkCmdEndRendering-videocoding"},
    {CMD_VIDEO_CODING_OUTSIDE, "VUID-vkCmdEndTransformFeedbackEXT-videocoding"},
    {CMD_VIDEO_CODING_INSIDE, "VUID-vkCmdEndVideoCodingKHR-videocoding"},
    {CMD_VIDEO_CODING_OUTSIDE, "VUID-vkCmdExecuteCommands-videocoding"},
    {CMD_VIDEO_CODING_OUTSIDE, "VUID-vkCmdExecuteGeneratedCommandsNV-videocoding"},
    {CMD_VIDEO_CODING_OUTSIDE, "VUID-vkCmdFillBuffer-videocoding"},
    {CMD_VIDEO_CODING_OUTSIDE, "VUID-vkCmdInsertDebugUtilsLabelEXT-videocoding"},
    {CMD_VIDEO_CODING_OUTSIDE, "VUID-vkCmdNextSubpass-videocoding"},
    {CMD_VIDEO_CODING_OUTSIDE, "VUID-vkCmdNextSubpass2-videocoding"},
    {CMD_VIDEO_CODING_OUTSIDE, "VUID-vkCmdNextSubpass2-videocoding"},
    {CMD_VIDEO_CODING_OUTSIDE, "VUID-vkCmdOpticalFlowExecuteNV-videocoding"},
    {CMD_VIDEO_CODING_BOTH, kVUIDUndefined},
    {CMD_VIDEO_CODING_BOTH, kVUIDUndefined},
    {CMD_VIDEO_CODING_BOTH, kVUIDUndefined},
    {CMD_VIDEO_CODING_OUTSIDE, "VUID-vkCmdPreprocessGeneratedCommandsNV-videocoding"},
    {CMD_VIDEO_CODING_OUTSIDE, "VUID-vkCmdPushConstants-videocoding"},
    {CMD_VIDEO_CODING_OUTSIDE, "VUID-vkCmdPushDescriptorSetKHR-videocoding"},
    {CMD_VIDEO_CODING_OUTSIDE, "VUID-vkCmdPushDescriptorSetWithTemplateKHR-videocoding"},
    {CMD_VIDEO_CODING_BOTH, kVUIDUndefined},
    {CMD_VIDEO_CODING_BOTH, kVUIDUndefined},
    {CMD_VIDEO_CODING_BOTH, kVUIDUndefined},
    {CMD_VIDEO_CODING_OUTSIDE, "VUID-vkCmdResetQueryPool-videocoding"},
    {CMD_VIDEO_CODING_OUTSIDE, "VUID-vkCmdResolveImage-videocoding"},
    {CMD_VIDEO_CODING_OUTSIDE, "VUID-vkCmdResolveImage2-videocoding"},
    {CMD_VIDEO_CODING_OUTSIDE, "VUID-vkCmdResolveImage2-videocoding"},
    {CMD_VIDEO_CODING_OUTSIDE, "VUID-vkCmdSetAlphaToCoverageEnableEXT-videocoding"},
    {CMD_VIDEO_CODING_OUTSIDE, "VUID-vkCmdSetAlphaToOneEnableEXT-videocoding"},
    {CMD_VIDEO_CODING_OUTSIDE, "VUID-vkCmdSetBlendConstants-videocoding"},
    {CMD_VIDEO_CODING_OUTSIDE, "VUID-vkCmdSetCheckpointNV-videocoding"},
    {CMD_VIDEO_CODING_OUTSIDE, "VUID-vkCmdSetCoarseSampleOrderNV-videocoding"},
    {CMD_VIDEO_CODING_OUTSIDE, "VUID-vkCmdSetColorBlendAdvancedEXT-videocoding"},
    {CMD_VIDEO_CODING_OUTSIDE, "VUID-vkCmdSetColorBlendEnableEXT-videocoding"},
    {CMD_VIDEO_CODING_OUTSIDE, "VUID-vkCmdSetColorBlendEquationEXT-videocoding"},
    {CMD_VIDEO_CODING_OUTSIDE, "VUID-vkCmdSetColorWriteEnableEXT-videocoding"},
    {CMD_VIDEO_CODING_OUTSIDE, "VUID-vkCmdSetColorWriteMaskEXT-videocoding"},
    {CMD_VIDEO_CODING_OUTSIDE, "VUID-vkCmdSetConservativeRasterizationModeEXT-videocoding"},
    {CMD_VIDEO_CODING_OUTSIDE, "VUID-vkCmdSetCoverageModulationModeNV-videocoding"},
    {CMD_VIDEO_CODING_OUTSIDE, "VUID-vkCmdSetCoverageModulationTableEnableNV-videocoding"},
    {CMD_VIDEO_CODING_OUTSIDE, "VUID-vkCmdSetCoverageModulationTableNV-videocoding"},
    {CMD_VIDEO_CODING_OUTSIDE, "VUID-vkCmdSetCoverageReductionModeNV-videocoding"},
    {CMD_VIDEO_CODING_OUTSIDE, "VUID-vkCmdSetCoverageToColorEnableNV-videocoding"},
    {CMD_VIDEO_CODING_OUTSIDE, "VUID-vkCmdSetCoverageToColorLocationNV-videocoding"},
    {CMD_VIDEO_CODING_OUTSIDE, "VUID-vkCmdSetCullMode-videocoding"},
    {CMD_VIDEO_CODING_OUTSIDE, "VUID-vkCmdSetCullMode-videocoding"},
    {CMD_VIDEO_CODING_OUTSIDE, "VUID-vkCmdSetDepthBias-videocoding"},
    {CMD_VIDEO_CODING_OUTSIDE, "VUID-vkCmdSetDepthBiasEnable-videocoding"},
    {CMD_VIDEO_CODING_OUTSIDE, "VUID-vkCmdSetDepthBiasEnable-videocoding"},
    {CMD_VIDEO_CODING_OUTSIDE, "VUID-vkCmdSetDepthBounds-videocoding"},
    {CMD_VIDEO_CODING_OUTSIDE, "VUID-vkCmdSetDepthBoundsTestEnable-videocoding"},
    {CMD_VIDEO_CODING_OUTSIDE, "VUID-vkCmdSetDepthBoundsTestEnable-videocoding"},
    {CMD_VIDEO_CODING_OUTSIDE, "VUID-vkCmdSetDepthClampEnableEXT-videocoding"},
    {CMD_VIDEO_CODING_OUTSIDE, "VUID-vkCmdSetDepthClipEnableEXT-videocoding"},
    {CMD_VIDEO_CODING_OUTSIDE, "VUID-vkCmdSetDepthClipNegativeOneToOneEXT-videocoding"},
    {CMD_VIDEO_CODING_OUTSIDE, "VUID-vkCmdSetDepthCompareOp-videocoding"},
    {CMD_VIDEO_CODING_OUTSIDE, "VUID-vkCmdSetDepthCompareOp-videocoding"},
    {CMD_VIDEO_CODING_OUTSIDE, "VUID-vkCmdSetDepthTestEnable-videocoding"},
    {CMD_VIDEO_CODING_OUTSIDE, "VUID-vkCmdSetDepthTestEnable-videocoding"},
    {CMD_VIDEO_CODING_OUTSIDE, "VUID-vkCmdSetDepthWriteEnable-videocoding"},
    {CMD_VIDEO_CODING_OUTSIDE, "VUID-vkCmdSetDepthWriteEnable-videocoding"},
    {CMD_VIDEO_CODING_OUTSIDE, "VUID-vkCmdSetDescriptorBufferOffsetsEXT-videocoding"},
    {CMD_VIDEO_CODING_BOTH, kVUIDUndefined},
    {CMD_VIDEO_CODING_BOTH, kVUIDUndefined},
    {CMD_VIDEO_CODING_OUTSIDE, "VUID-vkCmdSetDiscardRectangleEXT-videocoding"},
    {CMD_VIDEO_CODING_BOTH, kVUIDUndefined},
    {CMD_VIDEO_CODING_BOTH, kVUIDUndefined},
    {CMD_VIDEO_CODING_BOTH, kVUIDUndefined},
    {CMD_VIDEO_CODING_OUTSIDE, "VUID-vkCmdSetExclusiveScissorNV-videocoding"},
    {CMD_VIDEO_CODING_OUTSIDE, "VUID-vkCmdSetExtraPrimitiveOverestimationSizeEXT-videocoding"},
    {CMD_VIDEO_CODING_OUTSIDE, "VUID-vkCmdSetFragmentShadingRateEnumNV-videocoding"},
    {CMD_VIDEO_CODING_OUTSIDE, "VUID-vkCmdSetFragmentShadingRateKHR-videocoding"},
    {CMD_VIDEO_CODING_OUTSIDE, "VUID-vkCmdSetFrontFace-videocoding"},
    {CMD_VIDEO_CODING_OUTSIDE, "VUID-vkCmdSetFrontFace-videocoding"},
    {CMD_VIDEO_CODING_OUTSIDE, "VUID-vkCmdSetLineRasterizationModeEXT-videocoding"},
    {CMD_VIDEO_CODING_OUTSIDE, "VUID-vkCmdSetLineStippleEXT-videocoding"},
    {CMD_VIDEO_CODING_OUTSIDE, "VUID-vkCmdSetLineStippleEnableEXT-videocoding"},
    {CMD_VIDEO_CODING_OUTSIDE, "VUID-vkCmdSetLineWidth-videocoding"},
    {CMD_VIDEO_CODING_OUTSIDE, "VUID-vkCmdSetLogicOpEXT-videocoding"},
    {CMD_VIDEO_CODING_OUTSIDE, "VUID-vkCmdSetLogicOpEnableEXT-videocoding"},
    {CMD_VIDEO_CODING_OUTSIDE, "VUID-vkCmdSetPatchControlPointsEXT-videocoding"},
    {CMD_VIDEO_CODING_OUTSIDE, "VUID-vkCmdSetPerformanceMarkerINTEL-videocoding"},
    {CMD_VIDEO_CODING_OUTSIDE, "VUID-vkCmdSetPerformanceOverrideINTEL-videocoding"},
    {CMD_VIDEO_CODING_OUTSIDE, "VUID-vkCmdSetPerformanceStreamMarkerINTEL-videocoding"},
    {CMD_VIDEO_CODING_OUTSIDE, "VUID-vkCmdSetPolygonModeEXT-videocoding"},
    {CMD_VIDEO_CODING_OUTSIDE, "VUID-vkCmdSetPrimitiveRestartEnable-videocoding"},
    {CMD_VIDEO_CODING_OUTSIDE, "VUID-vkCmdSetPrimitiveRestartEnable-videocoding"},
    {CMD_VIDEO_CODING_OUTSIDE, "VUID-vkCmdSetPrimitiveTopology-videocoding"},
    {CMD_VIDEO_CODING_OUTSIDE, "VUID-vkCmdSetPrimitiveTopology-videocoding"},
    {CMD_VIDEO_CODING_OUTSIDE, "VUID-vkCmdSetProvokingVertexModeEXT-videocoding"},
    {CMD_VIDEO_CODING_OUTSIDE, "VUID-vkCmdSetRasterizationSamplesEXT-videocoding"},
    {CMD_VIDEO_CODING_OUTSIDE, "VUID-vkCmdSetRasterizationStreamEXT-videocoding"},
    {CMD_VIDEO_CODING_OUTSIDE, "VUID-vkCmdSetRasterizerDiscardEnable-videocoding"},
    {CMD_VIDEO_CODING_OUTSIDE, "VUID-vkCmdSetRasterizerDiscardEnable-videocoding"},
    {CMD_VIDEO_CODING_OUTSIDE, "VUID-vkCmdSetRayTracingPipelineStackSizeKHR-videocoding"},
    {CMD_VIDEO_CODING_OUTSIDE, "VUID-vkCmdSetRepresentativeFragmentTestEnableNV-videocoding"},
    {CMD_VIDEO_CODING_OUTSIDE, "VUID-vkCmdSetSampleLocationsEXT-videocoding"},
    {CMD_VIDEO_CODING_OUTSIDE, "VUID-vkCmdSetSampleLocationsEnableEXT-videocoding"},
    {CMD_VIDEO_CODING_OUTSIDE, "VUID-vkCmdSetSampleMaskEXT-videocoding"},
    {CMD_VIDEO_CODING_OUTSIDE, "VUID-vkCmdSetScissor-videocoding"},
    {CMD_VIDEO_CODING_OUTSIDE, "VUID-vkCmdSetScissorWithCount-videocoding"},
    {CMD_VIDEO_CODING_OUTSIDE, "VUID-vkCmdSetScissorWithCount-videocoding"},
    {CMD_VIDEO_CODING_OUTSIDE, "VUID-vkCmdSetShadingRateImageEnableNV-videocoding"},
    {CMD_VIDEO_CODING_OUTSIDE, "VUID-vkCmdSetStencilCompareMask-videocoding"},
    {CMD_VIDEO_CODING_OUTSIDE, "VUID-vkCmdSetStencilOp-videocoding"},
    {CMD_VIDEO_CODING_OUTSIDE, "VUID-vkCmdSetStencilOp-videocoding"},
    {CMD_VIDEO_CODING_OUTSIDE, "VUID-vkCmdSetStencilReference-videocoding"},
    {CMD_VIDEO_CODING_OUTSIDE, "VUID-vkCmdSetStencilTestEnable-videocoding"},
    {CMD_VIDEO_CODING_OUTSIDE, "VUID-vkCmdSetStencilTestEnable-videocoding"},
    {CMD_VIDEO_CODING_OUTSIDE, "VUID-vkCmdSetStencilWriteMask-videocoding"},
    {CMD_VIDEO_CODING_OUTSIDE, "VUID-vkCmdSetTessellationDomainOriginEXT-videocoding"},
    {CMD_VIDEO_CODING_OUTSIDE, "VUID-vkCmdSetVertexInputEXT-videocoding"},
    {CMD_VIDEO_CODING_OUTSIDE, "VUID-vkCmdSetViewport-videocoding"},
    {CMD_VIDEO_CODING_OUTSIDE, "VUID-vkCmdSetViewportShadingRatePaletteNV-videocoding"},
    {CMD_VIDEO_CODING_OUTSIDE, "VUID-vkCmdSetViewportSwizzleNV-videocoding"},
    {CMD_VIDEO_CODING_OUTSIDE, "VUID-vkCmdSetViewportWScalingEnableNV-videocoding"},
    {CMD_VIDEO_CODING_OUTSIDE, "VUID-vkCmdSetViewportWScalingNV-videocoding"},
    {CMD_VIDEO_CODING_OUTSIDE, "VUID-vkCmdSetViewportWithCount-videocoding"},
    {CMD_VIDEO_CODING_OUTSIDE, "VUID-vkCmdSetViewportWithCount-videocoding"},
    {CMD_VIDEO_CODING_OUTSIDE, "VUID-vkCmdSubpassShadingHUAWEI-videocoding"},
    {CMD_VIDEO_CODING_OUTSIDE, "VUID-vkCmdTraceRaysIndirect2KHR-videocoding"},
    {CMD_VIDEO_CODING_OUTSIDE, "VUID-vkCmdTraceRaysIndirectKHR-videocoding"},
    {CMD_VIDEO_CODING_OUTSIDE, "VUID-vkCmdTraceRaysKHR-videocoding"},
    {CMD_VIDEO_CODING_OUTSIDE, "VUID-vkCmdTraceRaysNV-videocoding"},
    {CMD_VIDEO_CODING_OUTSIDE, "VUID-vkCmdUpdateBuffer-videocoding"},
    {CMD_VIDEO_CODING_BOTH, kVUIDUndefined},
    {CMD_VIDEO_CODING_BOTH, kVUIDUndefined},
    {CMD_VIDEO_CODING_BOTH, kVUIDUndefined},
    {CMD_VIDEO_CODING_OUTSIDE, "VUID-vkCmdWriteAccelerationStructuresPropertiesKHR-videocoding"},
    {CMD_VIDEO_CODING_OUTSIDE, "VUID-vkCmdWriteAccelerationStructuresPropertiesNV-videocoding"},
    {CMD_VIDEO_CODING_OUTSIDE, "VUID-vkCmdWriteBufferMarker2AMD-videocoding"},
    {CMD_VIDEO_CODING_OUTSIDE, "VUID-vkCmdWriteBufferMarkerAMD-videocoding"},
    {CMD_VIDEO_CODING_OUTSIDE, "VUID-vkCmdWriteMicromapsPropertiesEXT-videocoding"},
    {CMD_VIDEO_CODING_BOTH, kVUIDUndefined},
    {CMD_VIDEO_CODING_BOTH, kVUIDUndefined},
    {CMD_VIDEO_CODING_BOTH, kVUIDUndefined},
}};

static const std::array<const char *, CMD_RANGE_SIZE> kGeneratedBufferLevelList = {{
    kVUIDUndefined, // CMD_NONE
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    "VUID-vkCmdBeginRenderPass-bufferlevel",
    "VUID-vkCmdBeginRenderPass2-bufferlevel",
    "VUID-vkCmdBeginRenderPass2-bufferlevel",
    nullptr,
    nullptr,
    nullptr,
    "VUID-vkCmdBeginVideoCodingKHR-bufferlevel",
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    "VUID-vkCmdControlVideoCodingKHR-bufferlevel",
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    "VUID-vkCmdDecodeVideoKHR-bufferlevel",
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    "VUID-vkCmdEncodeVideoKHR-bufferlevel",
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    "VUID-vkCmdEndRenderPass-bufferlevel",
    "VUID-vkCmdEndRenderPass2-bufferlevel",
    "VUID-vkCmdEndRenderPass2-bufferlevel",
    nullptr,
    nullptr,
    nullptr,
    "VUID-vkCmdEndVideoCodingKHR-bufferlevel",
    "VUID-vkCmdExecuteCommands-bufferlevel",
    nullptr,
    nullptr,
    nullptr,
    "VUID-vkCmdNextSubpass-bufferlevel",
    "VUID-vkCmdNextSubpass2-bufferlevel",
    "VUID-vkCmdNextSubpass2-bufferlevel",
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
}};

// Used to handle all the implicit VUs that are autogenerated from the registry
bool CoreChecks::ValidateCmd(const CMD_BUFFER_STATE &cb_state, const CMD_TYPE cmd) const {
    bool skip = false;
    const char *caller_name = CommandTypeString(cmd);

    // Validate the given command being added to the specified cmd buffer,
    // flagging errors if CB is not in the recording state or if there's an issue with the Cmd ordering
    switch (cb_state.state) {
        case CB_RECORDING:
            skip |= ValidateCmdSubpassState(cb_state, cmd);
            break;

        case CB_INVALID_COMPLETE:
        case CB_INVALID_INCOMPLETE:
            skip |= ReportInvalidCommandBuffer(cb_state, caller_name);
            break;

        default:
            assert(cmd != CMD_NONE);
            const auto error = kGeneratedMustBeRecordingList[cmd];
            skip |= LogError(cb_state.commandBuffer(), error, "You must call vkBeginCommandBuffer() before this call to %s.",
                            caller_name);
    }

    // Validate the command pool from which the command buffer is from that the command is allowed for queue type
    const auto supportedQueueType = kGeneratedQueueTypeList[cmd];
    skip |= ValidateCmdQueueFlags(cb_state, caller_name, supportedQueueType.flags, supportedQueueType.vuid);

    // Validate if command is inside or outside a render pass if applicable
    const auto supportedRenderPass = kGeneratedRenderPassList[cmd];
    if (supportedRenderPass.renderPass == CMD_RENDER_PASS_INSIDE) {
        skip |= OutsideRenderPass(cb_state, caller_name, supportedRenderPass.vuid);
    } else if (supportedRenderPass.renderPass == CMD_RENDER_PASS_OUTSIDE) {
        skip |= InsideRenderPass(cb_state, caller_name, supportedRenderPass.vuid);
    }

    // Validate if command is inside or outside a video coding scope if applicable
    const auto supportedVideoCoding = kGeneratedVideoCodingList[cmd];
    if (supportedVideoCoding.videoCoding == CMD_VIDEO_CODING_INSIDE) {
        skip |= OutsideVideoCodingScope(cb_state, caller_name, supportedVideoCoding.vuid);
    } else if (supportedVideoCoding.videoCoding == CMD_VIDEO_CODING_OUTSIDE) {
        skip |= InsideVideoCodingScope(cb_state, caller_name, supportedVideoCoding.vuid);
    }

    // Validate if command has to be recorded in a primary command buffer
    const auto supportedBufferLevel = kGeneratedBufferLevelList[cmd];
    if (supportedBufferLevel != nullptr) {
        skip |= ValidatePrimaryCommandBuffer(cb_state, caller_name, supportedBufferLevel);
    }

    return skip;
}

static VkDynamicState ConvertToDynamicState(CBDynamicStatus flag) {
    switch (flag) {
        case CB_DYNAMIC_VIEWPORT_SET:
            return VK_DYNAMIC_STATE_VIEWPORT;
        case CB_DYNAMIC_SCISSOR_SET:
            return VK_DYNAMIC_STATE_SCISSOR;
        case CB_DYNAMIC_LINE_WIDTH_SET:
            return VK_DYNAMIC_STATE_LINE_WIDTH;
        case CB_DYNAMIC_DEPTH_BIAS_SET:
            return VK_DYNAMIC_STATE_DEPTH_BIAS;
        case CB_DYNAMIC_BLEND_CONSTANTS_SET:
            return VK_DYNAMIC_STATE_BLEND_CONSTANTS;
        case CB_DYNAMIC_DEPTH_BOUNDS_SET:
            return VK_DYNAMIC_STATE_DEPTH_BOUNDS;
        case CB_DYNAMIC_STENCIL_COMPARE_MASK_SET:
            return VK_DYNAMIC_STATE_STENCIL_COMPARE_MASK;
        case CB_DYNAMIC_STENCIL_WRITE_MASK_SET:
            return VK_DYNAMIC_STATE_STENCIL_WRITE_MASK;
        case CB_DYNAMIC_STENCIL_REFERENCE_SET:
            return VK_DYNAMIC_STATE_STENCIL_REFERENCE;
        case CB_DYNAMIC_CULL_MODE_SET:
            return VK_DYNAMIC_STATE_CULL_MODE;
        case CB_DYNAMIC_FRONT_FACE_SET:
            return VK_DYNAMIC_STATE_FRONT_FACE;
        case CB_DYNAMIC_PRIMITIVE_TOPOLOGY_SET:
            return VK_DYNAMIC_STATE_PRIMITIVE_TOPOLOGY;
        case CB_DYNAMIC_VIEWPORT_WITH_COUNT_SET:
            return VK_DYNAMIC_STATE_VIEWPORT_WITH_COUNT;
        case CB_DYNAMIC_SCISSOR_WITH_COUNT_SET:
            return VK_DYNAMIC_STATE_SCISSOR_WITH_COUNT;
        case CB_DYNAMIC_VERTEX_INPUT_BINDING_STRIDE_SET:
            return VK_DYNAMIC_STATE_VERTEX_INPUT_BINDING_STRIDE;
        case CB_DYNAMIC_DEPTH_TEST_ENABLE_SET:
            return VK_DYNAMIC_STATE_DEPTH_TEST_ENABLE;
        case CB_DYNAMIC_DEPTH_WRITE_ENABLE_SET:
            return VK_DYNAMIC_STATE_DEPTH_WRITE_ENABLE;
        case CB_DYNAMIC_DEPTH_COMPARE_OP_SET:
            return VK_DYNAMIC_STATE_DEPTH_COMPARE_OP;
        case CB_DYNAMIC_DEPTH_BOUNDS_TEST_ENABLE_SET:
            return VK_DYNAMIC_STATE_DEPTH_BOUNDS_TEST_ENABLE;
        case CB_DYNAMIC_STENCIL_TEST_ENABLE_SET:
            return VK_DYNAMIC_STATE_STENCIL_TEST_ENABLE;
        case CB_DYNAMIC_STENCIL_OP_SET:
            return VK_DYNAMIC_STATE_STENCIL_OP;
        case CB_DYNAMIC_RASTERIZER_DISCARD_ENABLE_SET:
            return VK_DYNAMIC_STATE_RASTERIZER_DISCARD_ENABLE;
        case CB_DYNAMIC_DEPTH_BIAS_ENABLE_SET:
            return VK_DYNAMIC_STATE_DEPTH_BIAS_ENABLE;
        case CB_DYNAMIC_PRIMITIVE_RESTART_ENABLE_SET:
            return VK_DYNAMIC_STATE_PRIMITIVE_RESTART_ENABLE;
        case CB_DYNAMIC_VIEWPORT_W_SCALING_NV_SET:
            return VK_DYNAMIC_STATE_VIEWPORT_W_SCALING_NV;
        case CB_DYNAMIC_DISCARD_RECTANGLE_EXT_SET:
            return VK_DYNAMIC_STATE_DISCARD_RECTANGLE_EXT;
        case CB_DYNAMIC_SAMPLE_LOCATIONS_EXT_SET:
            return VK_DYNAMIC_STATE_SAMPLE_LOCATIONS_EXT;
        case CB_DYNAMIC_RAY_TRACING_PIPELINE_STACK_SIZE_KHR_SET:
            return VK_DYNAMIC_STATE_RAY_TRACING_PIPELINE_STACK_SIZE_KHR;
        case CB_DYNAMIC_VIEWPORT_SHADING_RATE_PALETTE_NV_SET:
            return VK_DYNAMIC_STATE_VIEWPORT_SHADING_RATE_PALETTE_NV;
        case CB_DYNAMIC_VIEWPORT_COARSE_SAMPLE_ORDER_NV_SET:
            return VK_DYNAMIC_STATE_VIEWPORT_COARSE_SAMPLE_ORDER_NV;
        case CB_DYNAMIC_EXCLUSIVE_SCISSOR_NV_SET:
            return VK_DYNAMIC_STATE_EXCLUSIVE_SCISSOR_NV;
        case CB_DYNAMIC_FRAGMENT_SHADING_RATE_KHR_SET:
            return VK_DYNAMIC_STATE_FRAGMENT_SHADING_RATE_KHR;
        case CB_DYNAMIC_LINE_STIPPLE_EXT_SET:
            return VK_DYNAMIC_STATE_LINE_STIPPLE_EXT;
        case CB_DYNAMIC_VERTEX_INPUT_EXT_SET:
            return VK_DYNAMIC_STATE_VERTEX_INPUT_EXT;
        case CB_DYNAMIC_PATCH_CONTROL_POINTS_EXT_SET:
            return VK_DYNAMIC_STATE_PATCH_CONTROL_POINTS_EXT;
        case CB_DYNAMIC_LOGIC_OP_EXT_SET:
            return VK_DYNAMIC_STATE_LOGIC_OP_EXT;
        case CB_DYNAMIC_COLOR_WRITE_ENABLE_EXT_SET:
            return VK_DYNAMIC_STATE_COLOR_WRITE_ENABLE_EXT;
        case CB_DYNAMIC_TESSELLATION_DOMAIN_ORIGIN_EXT_SET:
            return VK_DYNAMIC_STATE_TESSELLATION_DOMAIN_ORIGIN_EXT;
        case CB_DYNAMIC_DEPTH_CLAMP_ENABLE_EXT_SET:
            return VK_DYNAMIC_STATE_DEPTH_CLAMP_ENABLE_EXT;
        case CB_DYNAMIC_POLYGON_MODE_EXT_SET:
            return VK_DYNAMIC_STATE_POLYGON_MODE_EXT;
        case CB_DYNAMIC_RASTERIZATION_SAMPLES_EXT_SET:
            return VK_DYNAMIC_STATE_RASTERIZATION_SAMPLES_EXT;
        case CB_DYNAMIC_SAMPLE_MASK_EXT_SET:
            return VK_DYNAMIC_STATE_SAMPLE_MASK_EXT;
        case CB_DYNAMIC_ALPHA_TO_COVERAGE_ENABLE_EXT_SET:
            return VK_DYNAMIC_STATE_ALPHA_TO_COVERAGE_ENABLE_EXT;
        case CB_DYNAMIC_ALPHA_TO_ONE_ENABLE_EXT_SET:
            return VK_DYNAMIC_STATE_ALPHA_TO_ONE_ENABLE_EXT;
        case CB_DYNAMIC_LOGIC_OP_ENABLE_EXT_SET:
            return VK_DYNAMIC_STATE_LOGIC_OP_ENABLE_EXT;
        case CB_DYNAMIC_COLOR_BLEND_ENABLE_EXT_SET:
            return VK_DYNAMIC_STATE_COLOR_BLEND_ENABLE_EXT;
        case CB_DYNAMIC_COLOR_BLEND_EQUATION_EXT_SET:
            return VK_DYNAMIC_STATE_COLOR_BLEND_EQUATION_EXT;
        case CB_DYNAMIC_COLOR_WRITE_MASK_EXT_SET:
            return VK_DYNAMIC_STATE_COLOR_WRITE_MASK_EXT;
        case CB_DYNAMIC_RASTERIZATION_STREAM_EXT_SET:
            return VK_DYNAMIC_STATE_RASTERIZATION_STREAM_EXT;
        case CB_DYNAMIC_CONSERVATIVE_RASTERIZATION_MODE_EXT_SET:
            return VK_DYNAMIC_STATE_CONSERVATIVE_RASTERIZATION_MODE_EXT;
        case CB_DYNAMIC_EXTRA_PRIMITIVE_OVERESTIMATION_SIZE_EXT_SET:
            return VK_DYNAMIC_STATE_EXTRA_PRIMITIVE_OVERESTIMATION_SIZE_EXT;
        case CB_DYNAMIC_DEPTH_CLIP_ENABLE_EXT_SET:
            return VK_DYNAMIC_STATE_DEPTH_CLIP_ENABLE_EXT;
        case CB_DYNAMIC_SAMPLE_LOCATIONS_ENABLE_EXT_SET:
            return VK_DYNAMIC_STATE_SAMPLE_LOCATIONS_ENABLE_EXT;
        case CB_DYNAMIC_COLOR_BLEND_ADVANCED_EXT_SET:
            return VK_DYNAMIC_STATE_COLOR_BLEND_ADVANCED_EXT;
        case CB_DYNAMIC_PROVOKING_VERTEX_MODE_EXT_SET:
            return VK_DYNAMIC_STATE_PROVOKING_VERTEX_MODE_EXT;
        case CB_DYNAMIC_LINE_RASTERIZATION_MODE_EXT_SET:
            return VK_DYNAMIC_STATE_LINE_RASTERIZATION_MODE_EXT;
        case CB_DYNAMIC_LINE_STIPPLE_ENABLE_EXT_SET:
            return VK_DYNAMIC_STATE_LINE_STIPPLE_ENABLE_EXT;
        case CB_DYNAMIC_DEPTH_CLIP_NEGATIVE_ONE_TO_ONE_EXT_SET:
            return VK_DYNAMIC_STATE_DEPTH_CLIP_NEGATIVE_ONE_TO_ONE_EXT;
        case CB_DYNAMIC_VIEWPORT_W_SCALING_ENABLE_NV_SET:
            return VK_DYNAMIC_STATE_VIEWPORT_W_SCALING_ENABLE_NV;
        case CB_DYNAMIC_VIEWPORT_SWIZZLE_NV_SET:
            return VK_DYNAMIC_STATE_VIEWPORT_SWIZZLE_NV;
        case CB_DYNAMIC_COVERAGE_TO_COLOR_ENABLE_NV_SET:
            return VK_DYNAMIC_STATE_COVERAGE_TO_COLOR_ENABLE_NV;
        case CB_DYNAMIC_COVERAGE_TO_COLOR_LOCATION_NV_SET:
            return VK_DYNAMIC_STATE_COVERAGE_TO_COLOR_LOCATION_NV;
        case CB_DYNAMIC_COVERAGE_MODULATION_MODE_NV_SET:
            return VK_DYNAMIC_STATE_COVERAGE_MODULATION_MODE_NV;
        case CB_DYNAMIC_COVERAGE_MODULATION_TABLE_ENABLE_NV_SET:
            return VK_DYNAMIC_STATE_COVERAGE_MODULATION_TABLE_ENABLE_NV;
        case CB_DYNAMIC_COVERAGE_MODULATION_TABLE_NV_SET:
            return VK_DYNAMIC_STATE_COVERAGE_MODULATION_TABLE_NV;
        case CB_DYNAMIC_SHADING_RATE_IMAGE_ENABLE_NV_SET:
            return VK_DYNAMIC_STATE_SHADING_RATE_IMAGE_ENABLE_NV;
        case CB_DYNAMIC_REPRESENTATIVE_FRAGMENT_TEST_ENABLE_NV_SET:
            return VK_DYNAMIC_STATE_REPRESENTATIVE_FRAGMENT_TEST_ENABLE_NV;
        case CB_DYNAMIC_COVERAGE_REDUCTION_MODE_NV_SET:
            return VK_DYNAMIC_STATE_COVERAGE_REDUCTION_MODE_NV;
        default:
            return VK_DYNAMIC_STATE_MAX_ENUM;
    }
}

static CBDynamicStatus ConvertToCBDynamicStatus(VkDynamicState state) {
    switch (state) {
        case VK_DYNAMIC_STATE_VIEWPORT:
            return CB_DYNAMIC_VIEWPORT_SET;
        case VK_DYNAMIC_STATE_SCISSOR:
            return CB_DYNAMIC_SCISSOR_SET;
        case VK_DYNAMIC_STATE_LINE_WIDTH:
            return CB_DYNAMIC_LINE_WIDTH_SET;
        case VK_DYNAMIC_STATE_DEPTH_BIAS:
            return CB_DYNAMIC_DEPTH_BIAS_SET;
        case VK_DYNAMIC_STATE_BLEND_CONSTANTS:
            return CB_DYNAMIC_BLEND_CONSTANTS_SET;
        case VK_DYNAMIC_STATE_DEPTH_BOUNDS:
            return CB_DYNAMIC_DEPTH_BOUNDS_SET;
        case VK_DYNAMIC_STATE_STENCIL_COMPARE_MASK:
            return CB_DYNAMIC_STENCIL_COMPARE_MASK_SET;
        case VK_DYNAMIC_STATE_STENCIL_WRITE_MASK:
            return CB_DYNAMIC_STENCIL_WRITE_MASK_SET;
        case VK_DYNAMIC_STATE_STENCIL_REFERENCE:
            return CB_DYNAMIC_STENCIL_REFERENCE_SET;
        case VK_DYNAMIC_STATE_CULL_MODE:
            return CB_DYNAMIC_CULL_MODE_SET;
        case VK_DYNAMIC_STATE_FRONT_FACE:
            return CB_DYNAMIC_FRONT_FACE_SET;
        case VK_DYNAMIC_STATE_PRIMITIVE_TOPOLOGY:
            return CB_DYNAMIC_PRIMITIVE_TOPOLOGY_SET;
        case VK_DYNAMIC_STATE_VIEWPORT_WITH_COUNT:
            return CB_DYNAMIC_VIEWPORT_WITH_COUNT_SET;
        case VK_DYNAMIC_STATE_SCISSOR_WITH_COUNT:
            return CB_DYNAMIC_SCISSOR_WITH_COUNT_SET;
        case VK_DYNAMIC_STATE_VERTEX_INPUT_BINDING_STRIDE:
            return CB_DYNAMIC_VERTEX_INPUT_BINDING_STRIDE_SET;
        case VK_DYNAMIC_STATE_DEPTH_TEST_ENABLE:
            return CB_DYNAMIC_DEPTH_TEST_ENABLE_SET;
        case VK_DYNAMIC_STATE_DEPTH_WRITE_ENABLE:
            return CB_DYNAMIC_DEPTH_WRITE_ENABLE_SET;
        case VK_DYNAMIC_STATE_DEPTH_COMPARE_OP:
            return CB_DYNAMIC_DEPTH_COMPARE_OP_SET;
        case VK_DYNAMIC_STATE_DEPTH_BOUNDS_TEST_ENABLE:
            return CB_DYNAMIC_DEPTH_BOUNDS_TEST_ENABLE_SET;
        case VK_DYNAMIC_STATE_STENCIL_TEST_ENABLE:
            return CB_DYNAMIC_STENCIL_TEST_ENABLE_SET;
        case VK_DYNAMIC_STATE_STENCIL_OP:
            return CB_DYNAMIC_STENCIL_OP_SET;
        case VK_DYNAMIC_STATE_RASTERIZER_DISCARD_ENABLE:
            return CB_DYNAMIC_RASTERIZER_DISCARD_ENABLE_SET;
        case VK_DYNAMIC_STATE_DEPTH_BIAS_ENABLE:
            return CB_DYNAMIC_DEPTH_BIAS_ENABLE_SET;
        case VK_DYNAMIC_STATE_PRIMITIVE_RESTART_ENABLE:
            return CB_DYNAMIC_PRIMITIVE_RESTART_ENABLE_SET;
        case VK_DYNAMIC_STATE_VIEWPORT_W_SCALING_NV:
            return CB_DYNAMIC_VIEWPORT_W_SCALING_NV_SET;
        case VK_DYNAMIC_STATE_DISCARD_RECTANGLE_EXT:
            return CB_DYNAMIC_DISCARD_RECTANGLE_EXT_SET;
        case VK_DYNAMIC_STATE_SAMPLE_LOCATIONS_EXT:
            return CB_DYNAMIC_SAMPLE_LOCATIONS_EXT_SET;
        case VK_DYNAMIC_STATE_RAY_TRACING_PIPELINE_STACK_SIZE_KHR:
            return CB_DYNAMIC_RAY_TRACING_PIPELINE_STACK_SIZE_KHR_SET;
        case VK_DYNAMIC_STATE_VIEWPORT_SHADING_RATE_PALETTE_NV:
            return CB_DYNAMIC_VIEWPORT_SHADING_RATE_PALETTE_NV_SET;
        case VK_DYNAMIC_STATE_VIEWPORT_COARSE_SAMPLE_ORDER_NV:
            return CB_DYNAMIC_VIEWPORT_COARSE_SAMPLE_ORDER_NV_SET;
        case VK_DYNAMIC_STATE_EXCLUSIVE_SCISSOR_NV:
            return CB_DYNAMIC_EXCLUSIVE_SCISSOR_NV_SET;
        case VK_DYNAMIC_STATE_FRAGMENT_SHADING_RATE_KHR:
            return CB_DYNAMIC_FRAGMENT_SHADING_RATE_KHR_SET;
        case VK_DYNAMIC_STATE_LINE_STIPPLE_EXT:
            return CB_DYNAMIC_LINE_STIPPLE_EXT_SET;
        case VK_DYNAMIC_STATE_VERTEX_INPUT_EXT:
            return CB_DYNAMIC_VERTEX_INPUT_EXT_SET;
        case VK_DYNAMIC_STATE_PATCH_CONTROL_POINTS_EXT:
            return CB_DYNAMIC_PATCH_CONTROL_POINTS_EXT_SET;
        case VK_DYNAMIC_STATE_LOGIC_OP_EXT:
            return CB_DYNAMIC_LOGIC_OP_EXT_SET;
        case VK_DYNAMIC_STATE_COLOR_WRITE_ENABLE_EXT:
            return CB_DYNAMIC_COLOR_WRITE_ENABLE_EXT_SET;
        case VK_DYNAMIC_STATE_TESSELLATION_DOMAIN_ORIGIN_EXT:
            return CB_DYNAMIC_TESSELLATION_DOMAIN_ORIGIN_EXT_SET;
        case VK_DYNAMIC_STATE_DEPTH_CLAMP_ENABLE_EXT:
            return CB_DYNAMIC_DEPTH_CLAMP_ENABLE_EXT_SET;
        case VK_DYNAMIC_STATE_POLYGON_MODE_EXT:
            return CB_DYNAMIC_POLYGON_MODE_EXT_SET;
        case VK_DYNAMIC_STATE_RASTERIZATION_SAMPLES_EXT:
            return CB_DYNAMIC_RASTERIZATION_SAMPLES_EXT_SET;
        case VK_DYNAMIC_STATE_SAMPLE_MASK_EXT:
            return CB_DYNAMIC_SAMPLE_MASK_EXT_SET;
        case VK_DYNAMIC_STATE_ALPHA_TO_COVERAGE_ENABLE_EXT:
            return CB_DYNAMIC_ALPHA_TO_COVERAGE_ENABLE_EXT_SET;
        case VK_DYNAMIC_STATE_ALPHA_TO_ONE_ENABLE_EXT:
            return CB_DYNAMIC_ALPHA_TO_ONE_ENABLE_EXT_SET;
        case VK_DYNAMIC_STATE_LOGIC_OP_ENABLE_EXT:
            return CB_DYNAMIC_LOGIC_OP_ENABLE_EXT_SET;
        case VK_DYNAMIC_STATE_COLOR_BLEND_ENABLE_EXT:
            return CB_DYNAMIC_COLOR_BLEND_ENABLE_EXT_SET;
        case VK_DYNAMIC_STATE_COLOR_BLEND_EQUATION_EXT:
            return CB_DYNAMIC_COLOR_BLEND_EQUATION_EXT_SET;
        case VK_DYNAMIC_STATE_COLOR_WRITE_MASK_EXT:
            return CB_DYNAMIC_COLOR_WRITE_MASK_EXT_SET;
        case VK_DYNAMIC_STATE_RASTERIZATION_STREAM_EXT:
            return CB_DYNAMIC_RASTERIZATION_STREAM_EXT_SET;
        case VK_DYNAMIC_STATE_CONSERVATIVE_RASTERIZATION_MODE_EXT:
            return CB_DYNAMIC_CONSERVATIVE_RASTERIZATION_MODE_EXT_SET;
        case VK_DYNAMIC_STATE_EXTRA_PRIMITIVE_OVERESTIMATION_SIZE_EXT:
            return CB_DYNAMIC_EXTRA_PRIMITIVE_OVERESTIMATION_SIZE_EXT_SET;
        case VK_DYNAMIC_STATE_DEPTH_CLIP_ENABLE_EXT:
            return CB_DYNAMIC_DEPTH_CLIP_ENABLE_EXT_SET;
        case VK_DYNAMIC_STATE_SAMPLE_LOCATIONS_ENABLE_EXT:
            return CB_DYNAMIC_SAMPLE_LOCATIONS_ENABLE_EXT_SET;
        case VK_DYNAMIC_STATE_COLOR_BLEND_ADVANCED_EXT:
            return CB_DYNAMIC_COLOR_BLEND_ADVANCED_EXT_SET;
        case VK_DYNAMIC_STATE_PROVOKING_VERTEX_MODE_EXT:
            return CB_DYNAMIC_PROVOKING_VERTEX_MODE_EXT_SET;
        case VK_DYNAMIC_STATE_LINE_RASTERIZATION_MODE_EXT:
            return CB_DYNAMIC_LINE_RASTERIZATION_MODE_EXT_SET;
        case VK_DYNAMIC_STATE_LINE_STIPPLE_ENABLE_EXT:
            return CB_DYNAMIC_LINE_STIPPLE_ENABLE_EXT_SET;
        case VK_DYNAMIC_STATE_DEPTH_CLIP_NEGATIVE_ONE_TO_ONE_EXT:
            return CB_DYNAMIC_DEPTH_CLIP_NEGATIVE_ONE_TO_ONE_EXT_SET;
        case VK_DYNAMIC_STATE_VIEWPORT_W_SCALING_ENABLE_NV:
            return CB_DYNAMIC_VIEWPORT_W_SCALING_ENABLE_NV_SET;
        case VK_DYNAMIC_STATE_VIEWPORT_SWIZZLE_NV:
            return CB_DYNAMIC_VIEWPORT_SWIZZLE_NV_SET;
        case VK_DYNAMIC_STATE_COVERAGE_TO_COLOR_ENABLE_NV:
            return CB_DYNAMIC_COVERAGE_TO_COLOR_ENABLE_NV_SET;
        case VK_DYNAMIC_STATE_COVERAGE_TO_COLOR_LOCATION_NV:
            return CB_DYNAMIC_COVERAGE_TO_COLOR_LOCATION_NV_SET;
        case VK_DYNAMIC_STATE_COVERAGE_MODULATION_MODE_NV:
            return CB_DYNAMIC_COVERAGE_MODULATION_MODE_NV_SET;
        case VK_DYNAMIC_STATE_COVERAGE_MODULATION_TABLE_ENABLE_NV:
            return CB_DYNAMIC_COVERAGE_MODULATION_TABLE_ENABLE_NV_SET;
        case VK_DYNAMIC_STATE_COVERAGE_MODULATION_TABLE_NV:
            return CB_DYNAMIC_COVERAGE_MODULATION_TABLE_NV_SET;
        case VK_DYNAMIC_STATE_SHADING_RATE_IMAGE_ENABLE_NV:
            return CB_DYNAMIC_SHADING_RATE_IMAGE_ENABLE_NV_SET;
        case VK_DYNAMIC_STATE_REPRESENTATIVE_FRAGMENT_TEST_ENABLE_NV:
            return CB_DYNAMIC_REPRESENTATIVE_FRAGMENT_TEST_ENABLE_NV_SET;
        case VK_DYNAMIC_STATE_COVERAGE_REDUCTION_MODE_NV:
            return CB_DYNAMIC_COVERAGE_REDUCTION_MODE_NV_SET;
        default:
            return CB_DYNAMIC_STATUS_NUM;
    }
}

const char* DynamicStateToString(CBDynamicStatus status) {
    return string_VkDynamicState(ConvertToDynamicState(status));
}

std::string DynamicStatesToString(CBDynamicFlags const &dynamic_state) {
    std::string ret;
    // enum is not zero based
    for (int index = 1; index < CB_DYNAMIC_STATUS_NUM; ++index) {
        CBDynamicStatus status = static_cast<CBDynamicStatus>(index);
        if (dynamic_state[status]) {
            if (!ret.empty()) ret.append("|");
            ret.append(string_VkDynamicState(ConvertToDynamicState(status)));
        }
    }
    if (ret.empty()) ret.append(string_VkDynamicState(ConvertToDynamicState(CB_DYNAMIC_STATUS_NUM)));
    return ret;
}

CBDynamicFlags MakeStaticStateMask(VkPipelineDynamicStateCreateInfo const *info) {
    // initially assume everything is static state
    CBDynamicFlags flags(~CBDynamicFlags(0));

    if (info) {
        for (uint32_t i = 0; i < info->dynamicStateCount; i++) {
            flags.reset(ConvertToCBDynamicStatus(info->pDynamicStates[i]));
        }
    }
    return flags;
}

