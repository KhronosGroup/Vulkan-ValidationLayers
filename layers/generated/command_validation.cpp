// *** THIS FILE IS GENERATED - DO NOT EDIT ***
// See command_validation_generator.py for modifications


/***************************************************************************
 *
 * Copyright (c) 2021 The Khronos Group Inc.
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
 * Author: Spencer Fricke <s.fricke@samsung.com>
 *
 ****************************************************************************/

#include "vk_layer_logging.h"
#include "core_validation.h"

static const std::array<const char *, CMD_RANGE_SIZE> KGeneratedMustBeRecordingList = {{
    kVUIDUndefined,
    "VUID-vkCmdBeginConditionalRenderingEXT-commandBuffer-recording",
    "VUID-vkCmdBeginDebugUtilsLabelEXT-commandBuffer-recording",
    "VUID-vkCmdBeginQuery-commandBuffer-recording",
    "VUID-vkCmdBeginQueryIndexedEXT-commandBuffer-recording",
    "VUID-vkCmdBeginRenderPass-commandBuffer-recording",
    "VUID-vkCmdBeginRenderPass2-commandBuffer-recording",
    "VUID-vkCmdBeginTransformFeedbackEXT-commandBuffer-recording",
    "VUID-vkCmdBeginVideoCodingKHR-commandBuffer-recording",
    "VUID-vkCmdBindDescriptorSets-commandBuffer-recording",
    "VUID-vkCmdBindIndexBuffer-commandBuffer-recording",
    "VUID-vkCmdBindPipeline-commandBuffer-recording",
    "VUID-vkCmdBindPipelineShaderGroupNV-commandBuffer-recording",
    "VUID-vkCmdBindShadingRateImageNV-commandBuffer-recording",
    "VUID-vkCmdBindTransformFeedbackBuffersEXT-commandBuffer-recording",
    "VUID-vkCmdBindVertexBuffers-commandBuffer-recording",
    "VUID-vkCmdBindVertexBuffers2EXT-commandBuffer-recording",
    "VUID-vkCmdBlitImage-commandBuffer-recording",
    "VUID-vkCmdBlitImage2KHR-commandBuffer-recording",
    "VUID-vkCmdBuildAccelerationStructureNV-commandBuffer-recording",
    "VUID-vkCmdBuildAccelerationStructuresIndirectKHR-commandBuffer-recording",
    "VUID-vkCmdBuildAccelerationStructuresKHR-commandBuffer-recording",
    "VUID-vkCmdClearAttachments-commandBuffer-recording",
    "VUID-vkCmdClearColorImage-commandBuffer-recording",
    "VUID-vkCmdClearDepthStencilImage-commandBuffer-recording",
    "VUID-vkCmdControlVideoCodingKHR-commandBuffer-recording",
    "VUID-vkCmdCopyAccelerationStructureKHR-commandBuffer-recording",
    "VUID-vkCmdCopyAccelerationStructureNV-commandBuffer-recording",
    "VUID-vkCmdCopyAccelerationStructureToMemoryKHR-commandBuffer-recording",
    "VUID-vkCmdCopyBuffer-commandBuffer-recording",
    "VUID-vkCmdCopyBuffer2KHR-commandBuffer-recording",
    "VUID-vkCmdCopyBufferToImage-commandBuffer-recording",
    "VUID-vkCmdCopyBufferToImage2KHR-commandBuffer-recording",
    "VUID-vkCmdCopyImage-commandBuffer-recording",
    "VUID-vkCmdCopyImage2KHR-commandBuffer-recording",
    "VUID-vkCmdCopyImageToBuffer-commandBuffer-recording",
    "VUID-vkCmdCopyImageToBuffer2KHR-commandBuffer-recording",
    "VUID-vkCmdCopyMemoryToAccelerationStructureKHR-commandBuffer-recording",
    "VUID-vkCmdCopyQueryPoolResults-commandBuffer-recording",
    "VUID-vkCmdDebugMarkerBeginEXT-commandBuffer-recording",
    "VUID-vkCmdDebugMarkerEndEXT-commandBuffer-recording",
    "VUID-vkCmdDebugMarkerInsertEXT-commandBuffer-recording",
    "VUID-vkCmdDecodeVideoKHR-commandBuffer-recording",
    "VUID-vkCmdDispatch-commandBuffer-recording",
    "VUID-vkCmdDispatchBase-commandBuffer-recording",
    "VUID-vkCmdDispatchIndirect-commandBuffer-recording",
    "VUID-vkCmdDraw-commandBuffer-recording",
    "VUID-vkCmdDrawIndexed-commandBuffer-recording",
    "VUID-vkCmdDrawIndexedIndirect-commandBuffer-recording",
    "VUID-vkCmdDrawIndexedIndirectCount-commandBuffer-recording",
    "VUID-vkCmdDrawIndirect-commandBuffer-recording",
    "VUID-vkCmdDrawIndirectByteCountEXT-commandBuffer-recording",
    "VUID-vkCmdDrawIndirectCount-commandBuffer-recording",
    "VUID-vkCmdDrawMeshTasksIndirectCountNV-commandBuffer-recording",
    "VUID-vkCmdDrawMeshTasksIndirectNV-commandBuffer-recording",
    "VUID-vkCmdDrawMeshTasksNV-commandBuffer-recording",
    "VUID-vkCmdEncodeVideoKHR-commandBuffer-recording",
    "VUID-vkCmdEndConditionalRenderingEXT-commandBuffer-recording",
    "VUID-vkCmdEndDebugUtilsLabelEXT-commandBuffer-recording",
    "VUID-vkCmdEndQuery-commandBuffer-recording",
    "VUID-vkCmdEndQueryIndexedEXT-commandBuffer-recording",
    "VUID-vkCmdEndRenderPass-commandBuffer-recording",
    "VUID-vkCmdEndRenderPass2-commandBuffer-recording",
    "VUID-vkCmdEndTransformFeedbackEXT-commandBuffer-recording",
    "VUID-vkCmdEndVideoCodingKHR-commandBuffer-recording",
    "VUID-vkCmdExecuteCommands-commandBuffer-recording",
    "VUID-vkCmdExecuteGeneratedCommandsNV-commandBuffer-recording",
    "VUID-vkCmdFillBuffer-commandBuffer-recording",
    "VUID-vkCmdInsertDebugUtilsLabelEXT-commandBuffer-recording",
    "VUID-vkCmdNextSubpass-commandBuffer-recording",
    "VUID-vkCmdNextSubpass2-commandBuffer-recording",
    "VUID-vkCmdPipelineBarrier-commandBuffer-recording",
    "VUID-vkCmdPipelineBarrier2KHR-commandBuffer-recording",
    "VUID-vkCmdPreprocessGeneratedCommandsNV-commandBuffer-recording",
    "VUID-vkCmdPushConstants-commandBuffer-recording",
    "VUID-vkCmdPushDescriptorSetKHR-commandBuffer-recording",
    "VUID-vkCmdPushDescriptorSetWithTemplateKHR-commandBuffer-recording",
    "VUID-vkCmdResetEvent-commandBuffer-recording",
    "VUID-vkCmdResetEvent2KHR-commandBuffer-recording",
    "VUID-vkCmdResetQueryPool-commandBuffer-recording",
    "VUID-vkCmdResolveImage-commandBuffer-recording",
    "VUID-vkCmdResolveImage2KHR-commandBuffer-recording",
    "VUID-vkCmdSetBlendConstants-commandBuffer-recording",
    "VUID-vkCmdSetCheckpointNV-commandBuffer-recording",
    "VUID-vkCmdSetCoarseSampleOrderNV-commandBuffer-recording",
    "VUID-vkCmdSetColorWriteEnableEXT-commandBuffer-recording",
    "VUID-vkCmdSetCullModeEXT-commandBuffer-recording",
    "VUID-vkCmdSetDepthBias-commandBuffer-recording",
    "VUID-vkCmdSetDepthBiasEnableEXT-commandBuffer-recording",
    "VUID-vkCmdSetDepthBounds-commandBuffer-recording",
    "VUID-vkCmdSetDepthBoundsTestEnableEXT-commandBuffer-recording",
    "VUID-vkCmdSetDepthCompareOpEXT-commandBuffer-recording",
    "VUID-vkCmdSetDepthTestEnableEXT-commandBuffer-recording",
    "VUID-vkCmdSetDepthWriteEnableEXT-commandBuffer-recording",
    "VUID-vkCmdSetDeviceMask-commandBuffer-recording",
    "VUID-vkCmdSetDiscardRectangleEXT-commandBuffer-recording",
    "VUID-vkCmdSetEvent-commandBuffer-recording",
    "VUID-vkCmdSetEvent2KHR-commandBuffer-recording",
    "VUID-vkCmdSetExclusiveScissorNV-commandBuffer-recording",
    "VUID-vkCmdSetFragmentShadingRateEnumNV-commandBuffer-recording",
    "VUID-vkCmdSetFragmentShadingRateKHR-commandBuffer-recording",
    "VUID-vkCmdSetFrontFaceEXT-commandBuffer-recording",
    "VUID-vkCmdSetLineStippleEXT-commandBuffer-recording",
    "VUID-vkCmdSetLineWidth-commandBuffer-recording",
    "VUID-vkCmdSetLogicOpEXT-commandBuffer-recording",
    "VUID-vkCmdSetPatchControlPointsEXT-commandBuffer-recording",
    "VUID-vkCmdSetPerformanceMarkerINTEL-commandBuffer-recording",
    "VUID-vkCmdSetPerformanceOverrideINTEL-commandBuffer-recording",
    "VUID-vkCmdSetPerformanceStreamMarkerINTEL-commandBuffer-recording",
    "VUID-vkCmdSetPrimitiveRestartEnableEXT-commandBuffer-recording",
    "VUID-vkCmdSetPrimitiveTopologyEXT-commandBuffer-recording",
    "VUID-vkCmdSetRasterizerDiscardEnableEXT-commandBuffer-recording",
    "VUID-vkCmdSetRayTracingPipelineStackSizeKHR-commandBuffer-recording",
    "VUID-vkCmdSetSampleLocationsEXT-commandBuffer-recording",
    "VUID-vkCmdSetScissor-commandBuffer-recording",
    "VUID-vkCmdSetScissorWithCountEXT-commandBuffer-recording",
    "VUID-vkCmdSetStencilCompareMask-commandBuffer-recording",
    "VUID-vkCmdSetStencilOpEXT-commandBuffer-recording",
    "VUID-vkCmdSetStencilReference-commandBuffer-recording",
    "VUID-vkCmdSetStencilTestEnableEXT-commandBuffer-recording",
    "VUID-vkCmdSetStencilWriteMask-commandBuffer-recording",
    "VUID-vkCmdSetVertexInputEXT-commandBuffer-recording",
    "VUID-vkCmdSetViewport-commandBuffer-recording",
    "VUID-vkCmdSetViewportShadingRatePaletteNV-commandBuffer-recording",
    "VUID-vkCmdSetViewportWScalingNV-commandBuffer-recording",
    "VUID-vkCmdSetViewportWithCountEXT-commandBuffer-recording",
    "VUID-vkCmdTraceRaysIndirectKHR-commandBuffer-recording",
    "VUID-vkCmdTraceRaysKHR-commandBuffer-recording",
    "VUID-vkCmdTraceRaysNV-commandBuffer-recording",
    "VUID-vkCmdUpdateBuffer-commandBuffer-recording",
    "VUID-vkCmdWaitEvents-commandBuffer-recording",
    "VUID-vkCmdWaitEvents2KHR-commandBuffer-recording",
    "VUID-vkCmdWriteAccelerationStructuresPropertiesKHR-commandBuffer-recording",
    "VUID-vkCmdWriteAccelerationStructuresPropertiesNV-commandBuffer-recording",
    "VUID-vkCmdWriteBufferMarker2AMD-commandBuffer-recording",
    "VUID-vkCmdWriteBufferMarkerAMD-commandBuffer-recording",
    "VUID-vkCmdWriteTimestamp-commandBuffer-recording",
    "VUID-vkCmdWriteTimestamp2KHR-commandBuffer-recording",
}};

// Used to handle all the implicit VUs that are autogenerated from the registry
bool CoreChecks::ValidateCmd(const CMD_BUFFER_STATE *cb_state, const CMD_TYPE cmd, const char *caller_name) const {
    // Validate the given command being added to the specified cmd buffer,
    // flagging errors if CB is not in the recording state or if there's an issue with the Cmd ordering
    switch (cb_state->state) {
        case CB_RECORDING:
            return ValidateCmdSubpassState(cb_state, cmd);

        case CB_INVALID_COMPLETE:
        case CB_INVALID_INCOMPLETE:
            return ReportInvalidCommandBuffer(cb_state, caller_name);

        default:
            assert(cmd != CMD_NONE);
            const auto error = KGeneratedMustBeRecordingList[cmd];
            return LogError(cb_state->commandBuffer, error, "You must call vkBeginCommandBuffer() before this call to %s.",
                            caller_name);
    }
}
