// *** THIS FILE IS GENERATED - DO NOT EDIT ***
// See command_counter_generator.py for modifications

/*
 * Copyright (c) 2015-2020 The Khronos Group Inc.
 * Copyright (c) 2015-2020 Valve Corporation
 * Copyright (c) 2015-2020 LunarG, Inc.
 * Copyright (c) 2019-2020 Intel Corporation
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
 * Author: Mark Lobodzinski <mark@lunarg.com>
 * Author: Lionel Landwerlin <lionel.g.landwerlin@intel.com>
 */

#include "chassis.h"
#include "state_tracker.h"
#include "command_counter.h"

void CommandCounter::PreCallRecordCmdBindPipeline(VkCommandBuffer commandBuffer, VkPipelineBindPoint pipelineBindPoint, VkPipeline pipeline) {
    trackerObject->IncrementCommandCount(commandBuffer);
    trackerObject->RecordIntoCommandList(commandBuffer, CMD_BINDPIPELINE);
}
void CommandCounter::PreCallRecordCmdSetViewport(VkCommandBuffer commandBuffer, uint32_t firstViewport, uint32_t viewportCount, const VkViewport* pViewports) {
    trackerObject->IncrementCommandCount(commandBuffer);
    trackerObject->RecordIntoCommandList(commandBuffer, CMD_SETVIEWPORT);
}
void CommandCounter::PreCallRecordCmdSetScissor(VkCommandBuffer commandBuffer, uint32_t firstScissor, uint32_t scissorCount, const VkRect2D* pScissors) {
    trackerObject->IncrementCommandCount(commandBuffer);
    trackerObject->RecordIntoCommandList(commandBuffer, CMD_SETSCISSOR);
}
void CommandCounter::PreCallRecordCmdSetLineWidth(VkCommandBuffer commandBuffer, float lineWidth) {
    trackerObject->IncrementCommandCount(commandBuffer);
    trackerObject->RecordIntoCommandList(commandBuffer, CMD_SETLINEWIDTH);
}
void CommandCounter::PreCallRecordCmdSetDepthBias(VkCommandBuffer commandBuffer, float depthBiasConstantFactor, float depthBiasClamp, float depthBiasSlopeFactor) {
    trackerObject->IncrementCommandCount(commandBuffer);
    trackerObject->RecordIntoCommandList(commandBuffer, CMD_SETDEPTHBIAS);
}
void CommandCounter::PreCallRecordCmdSetBlendConstants(VkCommandBuffer commandBuffer, const float blendConstants[4]) {
    trackerObject->IncrementCommandCount(commandBuffer);
    trackerObject->RecordIntoCommandList(commandBuffer, CMD_SETBLENDCONSTANTS);
}
void CommandCounter::PreCallRecordCmdSetDepthBounds(VkCommandBuffer commandBuffer, float minDepthBounds, float maxDepthBounds) {
    trackerObject->IncrementCommandCount(commandBuffer);
    trackerObject->RecordIntoCommandList(commandBuffer, CMD_SETDEPTHBOUNDS);
}
void CommandCounter::PreCallRecordCmdSetStencilCompareMask(VkCommandBuffer commandBuffer, VkStencilFaceFlags faceMask, uint32_t compareMask) {
    trackerObject->IncrementCommandCount(commandBuffer);
    trackerObject->RecordIntoCommandList(commandBuffer, CMD_SETSTENCILCOMPAREMASK);
}
void CommandCounter::PreCallRecordCmdSetStencilWriteMask(VkCommandBuffer commandBuffer, VkStencilFaceFlags faceMask, uint32_t writeMask) {
    trackerObject->IncrementCommandCount(commandBuffer);
    trackerObject->RecordIntoCommandList(commandBuffer, CMD_SETSTENCILWRITEMASK);
}
void CommandCounter::PreCallRecordCmdSetStencilReference(VkCommandBuffer commandBuffer, VkStencilFaceFlags faceMask, uint32_t reference) {
    trackerObject->IncrementCommandCount(commandBuffer);
    trackerObject->RecordIntoCommandList(commandBuffer, CMD_SETSTENCILREFERENCE);
}
void CommandCounter::PreCallRecordCmdBindDescriptorSets(VkCommandBuffer commandBuffer, VkPipelineBindPoint pipelineBindPoint, VkPipelineLayout layout, uint32_t firstSet, uint32_t descriptorSetCount, const VkDescriptorSet* pDescriptorSets, uint32_t dynamicOffsetCount, const uint32_t* pDynamicOffsets) {
    trackerObject->IncrementCommandCount(commandBuffer);
    trackerObject->RecordIntoCommandList(commandBuffer, CMD_BINDDESCRIPTORSETS);
}
void CommandCounter::PreCallRecordCmdBindIndexBuffer(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset, VkIndexType indexType) {
    trackerObject->IncrementCommandCount(commandBuffer);
    trackerObject->RecordIntoCommandList(commandBuffer, CMD_BINDINDEXBUFFER);
}
void CommandCounter::PreCallRecordCmdBindVertexBuffers(VkCommandBuffer commandBuffer, uint32_t firstBinding, uint32_t bindingCount, const VkBuffer* pBuffers, const VkDeviceSize* pOffsets) {
    trackerObject->IncrementCommandCount(commandBuffer);
    trackerObject->RecordIntoCommandList(commandBuffer, CMD_BINDVERTEXBUFFERS);
}
void CommandCounter::PreCallRecordCmdDraw(VkCommandBuffer commandBuffer, uint32_t vertexCount, uint32_t instanceCount, uint32_t firstVertex, uint32_t firstInstance) {
    trackerObject->IncrementCommandCount(commandBuffer);
    trackerObject->RecordIntoCommandList(commandBuffer, CMD_DRAW);
}
void CommandCounter::PreCallRecordCmdDrawIndexed(VkCommandBuffer commandBuffer, uint32_t indexCount, uint32_t instanceCount, uint32_t firstIndex, int32_t vertexOffset, uint32_t firstInstance) {
    trackerObject->IncrementCommandCount(commandBuffer);
    trackerObject->RecordIntoCommandList(commandBuffer, CMD_DRAWINDEXED);
}
void CommandCounter::PreCallRecordCmdDrawIndirect(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset, uint32_t drawCount, uint32_t stride) {
    trackerObject->IncrementCommandCount(commandBuffer);
    trackerObject->RecordIntoCommandList(commandBuffer, CMD_DRAWINDIRECT);
}
void CommandCounter::PreCallRecordCmdDrawIndexedIndirect(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset, uint32_t drawCount, uint32_t stride) {
    trackerObject->IncrementCommandCount(commandBuffer);
    trackerObject->RecordIntoCommandList(commandBuffer, CMD_DRAWINDEXEDINDIRECT);
}
void CommandCounter::PreCallRecordCmdDispatch(VkCommandBuffer commandBuffer, uint32_t groupCountX, uint32_t groupCountY, uint32_t groupCountZ) {
    trackerObject->IncrementCommandCount(commandBuffer);
    trackerObject->RecordIntoCommandList(commandBuffer, CMD_DISPATCH);
}
void CommandCounter::PreCallRecordCmdDispatchIndirect(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset) {
    trackerObject->IncrementCommandCount(commandBuffer);
    trackerObject->RecordIntoCommandList(commandBuffer, CMD_DISPATCHINDIRECT);
}
void CommandCounter::PreCallRecordCmdCopyBuffer(VkCommandBuffer commandBuffer, VkBuffer srcBuffer, VkBuffer dstBuffer, uint32_t regionCount, const VkBufferCopy* pRegions) {
    trackerObject->IncrementCommandCount(commandBuffer);
    trackerObject->RecordIntoCommandList(commandBuffer, CMD_COPYBUFFER);
}
void CommandCounter::PreCallRecordCmdCopyImage(VkCommandBuffer commandBuffer, VkImage srcImage, VkImageLayout srcImageLayout, VkImage dstImage, VkImageLayout dstImageLayout, uint32_t regionCount, const VkImageCopy* pRegions) {
    trackerObject->IncrementCommandCount(commandBuffer);
    trackerObject->RecordIntoCommandList(commandBuffer, CMD_COPYIMAGE);
}
void CommandCounter::PreCallRecordCmdBlitImage(VkCommandBuffer commandBuffer, VkImage srcImage, VkImageLayout srcImageLayout, VkImage dstImage, VkImageLayout dstImageLayout, uint32_t regionCount, const VkImageBlit* pRegions, VkFilter filter) {
    trackerObject->IncrementCommandCount(commandBuffer);
    trackerObject->RecordIntoCommandList(commandBuffer, CMD_BLITIMAGE);
}
void CommandCounter::PreCallRecordCmdCopyBufferToImage(VkCommandBuffer commandBuffer, VkBuffer srcBuffer, VkImage dstImage, VkImageLayout dstImageLayout, uint32_t regionCount, const VkBufferImageCopy* pRegions) {
    trackerObject->IncrementCommandCount(commandBuffer);
    trackerObject->RecordIntoCommandList(commandBuffer, CMD_COPYBUFFERTOIMAGE);
}
void CommandCounter::PreCallRecordCmdCopyImageToBuffer(VkCommandBuffer commandBuffer, VkImage srcImage, VkImageLayout srcImageLayout, VkBuffer dstBuffer, uint32_t regionCount, const VkBufferImageCopy* pRegions) {
    trackerObject->IncrementCommandCount(commandBuffer);
    trackerObject->RecordIntoCommandList(commandBuffer, CMD_COPYIMAGETOBUFFER);
}
void CommandCounter::PreCallRecordCmdUpdateBuffer(VkCommandBuffer commandBuffer, VkBuffer dstBuffer, VkDeviceSize dstOffset, VkDeviceSize dataSize, const void* pData) {
    trackerObject->IncrementCommandCount(commandBuffer);
    trackerObject->RecordIntoCommandList(commandBuffer, CMD_UPDATEBUFFER);
}
void CommandCounter::PreCallRecordCmdFillBuffer(VkCommandBuffer commandBuffer, VkBuffer dstBuffer, VkDeviceSize dstOffset, VkDeviceSize size, uint32_t data) {
    trackerObject->IncrementCommandCount(commandBuffer);
    trackerObject->RecordIntoCommandList(commandBuffer, CMD_FILLBUFFER);
}
void CommandCounter::PreCallRecordCmdClearColorImage(VkCommandBuffer commandBuffer, VkImage image, VkImageLayout imageLayout, const VkClearColorValue* pColor, uint32_t rangeCount, const VkImageSubresourceRange* pRanges) {
    trackerObject->IncrementCommandCount(commandBuffer);
    trackerObject->RecordIntoCommandList(commandBuffer, CMD_CLEARCOLORIMAGE);
}
void CommandCounter::PreCallRecordCmdClearDepthStencilImage(VkCommandBuffer commandBuffer, VkImage image, VkImageLayout imageLayout, const VkClearDepthStencilValue* pDepthStencil, uint32_t rangeCount, const VkImageSubresourceRange* pRanges) {
    trackerObject->IncrementCommandCount(commandBuffer);
    trackerObject->RecordIntoCommandList(commandBuffer, CMD_CLEARDEPTHSTENCILIMAGE);
}
void CommandCounter::PreCallRecordCmdClearAttachments(VkCommandBuffer commandBuffer, uint32_t attachmentCount, const VkClearAttachment* pAttachments, uint32_t rectCount, const VkClearRect* pRects) {
    trackerObject->IncrementCommandCount(commandBuffer);
    trackerObject->RecordIntoCommandList(commandBuffer, CMD_CLEARATTACHMENTS);
}
void CommandCounter::PreCallRecordCmdResolveImage(VkCommandBuffer commandBuffer, VkImage srcImage, VkImageLayout srcImageLayout, VkImage dstImage, VkImageLayout dstImageLayout, uint32_t regionCount, const VkImageResolve* pRegions) {
    trackerObject->IncrementCommandCount(commandBuffer);
    trackerObject->RecordIntoCommandList(commandBuffer, CMD_RESOLVEIMAGE);
}
void CommandCounter::PreCallRecordCmdSetEvent(VkCommandBuffer commandBuffer, VkEvent event, VkPipelineStageFlags stageMask) {
    trackerObject->IncrementCommandCount(commandBuffer);
    trackerObject->RecordIntoCommandList(commandBuffer, CMD_SETEVENT);
}
void CommandCounter::PreCallRecordCmdResetEvent(VkCommandBuffer commandBuffer, VkEvent event, VkPipelineStageFlags stageMask) {
    trackerObject->IncrementCommandCount(commandBuffer);
    trackerObject->RecordIntoCommandList(commandBuffer, CMD_RESETEVENT);
}
void CommandCounter::PreCallRecordCmdWaitEvents(VkCommandBuffer commandBuffer, uint32_t eventCount, const VkEvent* pEvents, VkPipelineStageFlags srcStageMask, VkPipelineStageFlags dstStageMask, uint32_t memoryBarrierCount, const VkMemoryBarrier* pMemoryBarriers, uint32_t bufferMemoryBarrierCount, const VkBufferMemoryBarrier* pBufferMemoryBarriers, uint32_t imageMemoryBarrierCount, const VkImageMemoryBarrier* pImageMemoryBarriers) {
    trackerObject->IncrementCommandCount(commandBuffer);
    trackerObject->RecordIntoCommandList(commandBuffer, CMD_WAITEVENTS);
}
void CommandCounter::PreCallRecordCmdPipelineBarrier(VkCommandBuffer commandBuffer, VkPipelineStageFlags srcStageMask, VkPipelineStageFlags dstStageMask, VkDependencyFlags dependencyFlags, uint32_t memoryBarrierCount, const VkMemoryBarrier* pMemoryBarriers, uint32_t bufferMemoryBarrierCount, const VkBufferMemoryBarrier* pBufferMemoryBarriers, uint32_t imageMemoryBarrierCount, const VkImageMemoryBarrier* pImageMemoryBarriers) {
    trackerObject->IncrementCommandCount(commandBuffer);
    trackerObject->RecordIntoCommandList(commandBuffer, CMD_PIPELINEBARRIER);
}
void CommandCounter::PreCallRecordCmdBeginQuery(VkCommandBuffer commandBuffer, VkQueryPool queryPool, uint32_t query, VkQueryControlFlags flags) {
    trackerObject->IncrementCommandCount(commandBuffer);
    trackerObject->RecordIntoCommandList(commandBuffer, CMD_BEGINQUERY);
}
void CommandCounter::PreCallRecordCmdEndQuery(VkCommandBuffer commandBuffer, VkQueryPool queryPool, uint32_t query) {
    trackerObject->IncrementCommandCount(commandBuffer);
    trackerObject->RecordIntoCommandList(commandBuffer, CMD_ENDQUERY);
}
void CommandCounter::PreCallRecordCmdResetQueryPool(VkCommandBuffer commandBuffer, VkQueryPool queryPool, uint32_t firstQuery, uint32_t queryCount) {
    trackerObject->IncrementCommandCount(commandBuffer);
    trackerObject->RecordIntoCommandList(commandBuffer, CMD_RESETQUERYPOOL);
}
void CommandCounter::PreCallRecordCmdWriteTimestamp(VkCommandBuffer commandBuffer, VkPipelineStageFlagBits pipelineStage, VkQueryPool queryPool, uint32_t query) {
    trackerObject->IncrementCommandCount(commandBuffer);
    trackerObject->RecordIntoCommandList(commandBuffer, CMD_WRITETIMESTAMP);
}
void CommandCounter::PreCallRecordCmdCopyQueryPoolResults(VkCommandBuffer commandBuffer, VkQueryPool queryPool, uint32_t firstQuery, uint32_t queryCount, VkBuffer dstBuffer, VkDeviceSize dstOffset, VkDeviceSize stride, VkQueryResultFlags flags) {
    trackerObject->IncrementCommandCount(commandBuffer);
    trackerObject->RecordIntoCommandList(commandBuffer, CMD_COPYQUERYPOOLRESULTS);
}
void CommandCounter::PreCallRecordCmdPushConstants(VkCommandBuffer commandBuffer, VkPipelineLayout layout, VkShaderStageFlags stageFlags, uint32_t offset, uint32_t size, const void* pValues) {
    trackerObject->IncrementCommandCount(commandBuffer);
    trackerObject->RecordIntoCommandList(commandBuffer, CMD_PUSHCONSTANTS);
}
void CommandCounter::PreCallRecordCmdBeginRenderPass(VkCommandBuffer commandBuffer, const VkRenderPassBeginInfo* pRenderPassBegin, VkSubpassContents contents) {
    trackerObject->IncrementCommandCount(commandBuffer);
    trackerObject->RecordIntoCommandList(commandBuffer, CMD_BEGINRENDERPASS);
}
void CommandCounter::PreCallRecordCmdNextSubpass(VkCommandBuffer commandBuffer, VkSubpassContents contents) {
    trackerObject->IncrementCommandCount(commandBuffer);
    trackerObject->RecordIntoCommandList(commandBuffer, CMD_NEXTSUBPASS);
}
void CommandCounter::PreCallRecordCmdEndRenderPass(VkCommandBuffer commandBuffer) {
    trackerObject->IncrementCommandCount(commandBuffer);
    trackerObject->RecordIntoCommandList(commandBuffer, CMD_ENDRENDERPASS);
}
void CommandCounter::PreCallRecordCmdExecuteCommands(VkCommandBuffer commandBuffer, uint32_t commandBufferCount, const VkCommandBuffer* pCommandBuffers) {
    trackerObject->IncrementCommandCount(commandBuffer);
    trackerObject->RecordIntoCommandList(commandBuffer, CMD_EXECUTECOMMANDS);
}
void CommandCounter::PreCallRecordCmdSetDeviceMask(VkCommandBuffer commandBuffer, uint32_t deviceMask) {
    trackerObject->IncrementCommandCount(commandBuffer);
    trackerObject->RecordIntoCommandList(commandBuffer, CMD_SETDEVICEMASK);
}
void CommandCounter::PreCallRecordCmdDispatchBase(VkCommandBuffer commandBuffer, uint32_t baseGroupX, uint32_t baseGroupY, uint32_t baseGroupZ, uint32_t groupCountX, uint32_t groupCountY, uint32_t groupCountZ) {
    trackerObject->IncrementCommandCount(commandBuffer);
    trackerObject->RecordIntoCommandList(commandBuffer, CMD_DISPATCHBASE);
}
void CommandCounter::PreCallRecordCmdDrawIndirectCount(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset, VkBuffer countBuffer, VkDeviceSize countBufferOffset, uint32_t maxDrawCount, uint32_t stride) {
    trackerObject->IncrementCommandCount(commandBuffer);
    trackerObject->RecordIntoCommandList(commandBuffer, CMD_DRAWINDIRECTCOUNT);
}
void CommandCounter::PreCallRecordCmdDrawIndexedIndirectCount(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset, VkBuffer countBuffer, VkDeviceSize countBufferOffset, uint32_t maxDrawCount, uint32_t stride) {
    trackerObject->IncrementCommandCount(commandBuffer);
    trackerObject->RecordIntoCommandList(commandBuffer, CMD_DRAWINDEXEDINDIRECTCOUNT);
}
void CommandCounter::PreCallRecordCmdBeginRenderPass2(VkCommandBuffer commandBuffer, const VkRenderPassBeginInfo*      pRenderPassBegin, const VkSubpassBeginInfo*      pSubpassBeginInfo) {
    trackerObject->IncrementCommandCount(commandBuffer);
    trackerObject->RecordIntoCommandList(commandBuffer, CMD_BEGINRENDERPASS2);
}
void CommandCounter::PreCallRecordCmdNextSubpass2(VkCommandBuffer commandBuffer, const VkSubpassBeginInfo*      pSubpassBeginInfo, const VkSubpassEndInfo*        pSubpassEndInfo) {
    trackerObject->IncrementCommandCount(commandBuffer);
    trackerObject->RecordIntoCommandList(commandBuffer, CMD_NEXTSUBPASS2);
}
void CommandCounter::PreCallRecordCmdEndRenderPass2(VkCommandBuffer commandBuffer, const VkSubpassEndInfo*        pSubpassEndInfo) {
    trackerObject->IncrementCommandCount(commandBuffer);
    trackerObject->RecordIntoCommandList(commandBuffer, CMD_ENDRENDERPASS2);
}
void CommandCounter::PreCallRecordCmdSetDeviceMaskKHR(VkCommandBuffer commandBuffer, uint32_t deviceMask) {
    trackerObject->IncrementCommandCount(commandBuffer);
    trackerObject->RecordIntoCommandList(commandBuffer, CMD_SETDEVICEMASK);
}
void CommandCounter::PreCallRecordCmdDispatchBaseKHR(VkCommandBuffer commandBuffer, uint32_t baseGroupX, uint32_t baseGroupY, uint32_t baseGroupZ, uint32_t groupCountX, uint32_t groupCountY, uint32_t groupCountZ) {
    trackerObject->IncrementCommandCount(commandBuffer);
    trackerObject->RecordIntoCommandList(commandBuffer, CMD_DISPATCHBASE);
}
void CommandCounter::PreCallRecordCmdPushDescriptorSetKHR(VkCommandBuffer commandBuffer, VkPipelineBindPoint pipelineBindPoint, VkPipelineLayout layout, uint32_t set, uint32_t descriptorWriteCount, const VkWriteDescriptorSet* pDescriptorWrites) {
    trackerObject->IncrementCommandCount(commandBuffer);
    trackerObject->RecordIntoCommandList(commandBuffer, CMD_PUSHDESCRIPTORSETKHR);
}
void CommandCounter::PreCallRecordCmdPushDescriptorSetWithTemplateKHR(VkCommandBuffer commandBuffer, VkDescriptorUpdateTemplate descriptorUpdateTemplate, VkPipelineLayout layout, uint32_t set, const void* pData) {
    trackerObject->IncrementCommandCount(commandBuffer);
    trackerObject->RecordIntoCommandList(commandBuffer, CMD_PUSHDESCRIPTORSETWITHTEMPLATEKHR);
}
void CommandCounter::PreCallRecordCmdBeginRenderPass2KHR(VkCommandBuffer commandBuffer, const VkRenderPassBeginInfo*      pRenderPassBegin, const VkSubpassBeginInfo*      pSubpassBeginInfo) {
    trackerObject->IncrementCommandCount(commandBuffer);
    trackerObject->RecordIntoCommandList(commandBuffer, CMD_BEGINRENDERPASS2);
}
void CommandCounter::PreCallRecordCmdNextSubpass2KHR(VkCommandBuffer commandBuffer, const VkSubpassBeginInfo*      pSubpassBeginInfo, const VkSubpassEndInfo*        pSubpassEndInfo) {
    trackerObject->IncrementCommandCount(commandBuffer);
    trackerObject->RecordIntoCommandList(commandBuffer, CMD_NEXTSUBPASS2);
}
void CommandCounter::PreCallRecordCmdEndRenderPass2KHR(VkCommandBuffer commandBuffer, const VkSubpassEndInfo*        pSubpassEndInfo) {
    trackerObject->IncrementCommandCount(commandBuffer);
    trackerObject->RecordIntoCommandList(commandBuffer, CMD_ENDRENDERPASS2);
}
void CommandCounter::PreCallRecordCmdDrawIndirectCountKHR(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset, VkBuffer countBuffer, VkDeviceSize countBufferOffset, uint32_t maxDrawCount, uint32_t stride) {
    trackerObject->IncrementCommandCount(commandBuffer);
    trackerObject->RecordIntoCommandList(commandBuffer, CMD_DRAWINDIRECTCOUNT);
}
void CommandCounter::PreCallRecordCmdDrawIndexedIndirectCountKHR(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset, VkBuffer countBuffer, VkDeviceSize countBufferOffset, uint32_t maxDrawCount, uint32_t stride) {
    trackerObject->IncrementCommandCount(commandBuffer);
    trackerObject->RecordIntoCommandList(commandBuffer, CMD_DRAWINDEXEDINDIRECTCOUNT);
}
void CommandCounter::PreCallRecordCmdSetFragmentShadingRateKHR(VkCommandBuffer           commandBuffer, const VkExtent2D*                           pFragmentSize, const VkFragmentShadingRateCombinerOpKHR    combinerOps[2]) {
    trackerObject->IncrementCommandCount(commandBuffer);
    trackerObject->RecordIntoCommandList(commandBuffer, CMD_SETFRAGMENTSHADINGRATEKHR);
}
void CommandCounter::PreCallRecordCmdCopyBuffer2KHR(VkCommandBuffer commandBuffer, const VkCopyBufferInfo2KHR* pCopyBufferInfo) {
    trackerObject->IncrementCommandCount(commandBuffer);
    trackerObject->RecordIntoCommandList(commandBuffer, CMD_COPYBUFFER2KHR);
}
void CommandCounter::PreCallRecordCmdCopyImage2KHR(VkCommandBuffer commandBuffer, const VkCopyImageInfo2KHR* pCopyImageInfo) {
    trackerObject->IncrementCommandCount(commandBuffer);
    trackerObject->RecordIntoCommandList(commandBuffer, CMD_COPYIMAGE2KHR);
}
void CommandCounter::PreCallRecordCmdCopyBufferToImage2KHR(VkCommandBuffer commandBuffer, const VkCopyBufferToImageInfo2KHR* pCopyBufferToImageInfo) {
    trackerObject->IncrementCommandCount(commandBuffer);
    trackerObject->RecordIntoCommandList(commandBuffer, CMD_COPYBUFFERTOIMAGE2KHR);
}
void CommandCounter::PreCallRecordCmdCopyImageToBuffer2KHR(VkCommandBuffer commandBuffer, const VkCopyImageToBufferInfo2KHR* pCopyImageToBufferInfo) {
    trackerObject->IncrementCommandCount(commandBuffer);
    trackerObject->RecordIntoCommandList(commandBuffer, CMD_COPYIMAGETOBUFFER2KHR);
}
void CommandCounter::PreCallRecordCmdBlitImage2KHR(VkCommandBuffer commandBuffer, const VkBlitImageInfo2KHR* pBlitImageInfo) {
    trackerObject->IncrementCommandCount(commandBuffer);
    trackerObject->RecordIntoCommandList(commandBuffer, CMD_BLITIMAGE2KHR);
}
void CommandCounter::PreCallRecordCmdResolveImage2KHR(VkCommandBuffer commandBuffer, const VkResolveImageInfo2KHR* pResolveImageInfo) {
    trackerObject->IncrementCommandCount(commandBuffer);
    trackerObject->RecordIntoCommandList(commandBuffer, CMD_RESOLVEIMAGE2KHR);
}
void CommandCounter::PreCallRecordCmdDebugMarkerBeginEXT(VkCommandBuffer commandBuffer, const VkDebugMarkerMarkerInfoEXT* pMarkerInfo) {
    trackerObject->IncrementCommandCount(commandBuffer);
    trackerObject->RecordIntoCommandList(commandBuffer, CMD_DEBUGMARKERBEGINEXT);
}
void CommandCounter::PreCallRecordCmdDebugMarkerEndEXT(VkCommandBuffer commandBuffer) {
    trackerObject->IncrementCommandCount(commandBuffer);
    trackerObject->RecordIntoCommandList(commandBuffer, CMD_DEBUGMARKERENDEXT);
}
void CommandCounter::PreCallRecordCmdDebugMarkerInsertEXT(VkCommandBuffer commandBuffer, const VkDebugMarkerMarkerInfoEXT* pMarkerInfo) {
    trackerObject->IncrementCommandCount(commandBuffer);
    trackerObject->RecordIntoCommandList(commandBuffer, CMD_DEBUGMARKERINSERTEXT);
}
void CommandCounter::PreCallRecordCmdBindTransformFeedbackBuffersEXT(VkCommandBuffer commandBuffer, uint32_t firstBinding, uint32_t bindingCount, const VkBuffer* pBuffers, const VkDeviceSize* pOffsets, const VkDeviceSize* pSizes) {
    trackerObject->IncrementCommandCount(commandBuffer);
    trackerObject->RecordIntoCommandList(commandBuffer, CMD_BINDTRANSFORMFEEDBACKBUFFERSEXT);
}
void CommandCounter::PreCallRecordCmdBeginTransformFeedbackEXT(VkCommandBuffer commandBuffer, uint32_t firstCounterBuffer, uint32_t counterBufferCount, const VkBuffer* pCounterBuffers, const VkDeviceSize* pCounterBufferOffsets) {
    trackerObject->IncrementCommandCount(commandBuffer);
    trackerObject->RecordIntoCommandList(commandBuffer, CMD_BEGINTRANSFORMFEEDBACKEXT);
}
void CommandCounter::PreCallRecordCmdEndTransformFeedbackEXT(VkCommandBuffer commandBuffer, uint32_t firstCounterBuffer, uint32_t counterBufferCount, const VkBuffer* pCounterBuffers, const VkDeviceSize* pCounterBufferOffsets) {
    trackerObject->IncrementCommandCount(commandBuffer);
    trackerObject->RecordIntoCommandList(commandBuffer, CMD_ENDTRANSFORMFEEDBACKEXT);
}
void CommandCounter::PreCallRecordCmdBeginQueryIndexedEXT(VkCommandBuffer commandBuffer, VkQueryPool queryPool, uint32_t query, VkQueryControlFlags flags, uint32_t index) {
    trackerObject->IncrementCommandCount(commandBuffer);
    trackerObject->RecordIntoCommandList(commandBuffer, CMD_BEGINQUERYINDEXEDEXT);
}
void CommandCounter::PreCallRecordCmdEndQueryIndexedEXT(VkCommandBuffer commandBuffer, VkQueryPool queryPool, uint32_t query, uint32_t index) {
    trackerObject->IncrementCommandCount(commandBuffer);
    trackerObject->RecordIntoCommandList(commandBuffer, CMD_ENDQUERYINDEXEDEXT);
}
void CommandCounter::PreCallRecordCmdDrawIndirectByteCountEXT(VkCommandBuffer commandBuffer, uint32_t instanceCount, uint32_t firstInstance, VkBuffer counterBuffer, VkDeviceSize counterBufferOffset, uint32_t counterOffset, uint32_t vertexStride) {
    trackerObject->IncrementCommandCount(commandBuffer);
    trackerObject->RecordIntoCommandList(commandBuffer, CMD_DRAWINDIRECTBYTECOUNTEXT);
}
void CommandCounter::PreCallRecordCmdDrawIndirectCountAMD(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset, VkBuffer countBuffer, VkDeviceSize countBufferOffset, uint32_t maxDrawCount, uint32_t stride) {
    trackerObject->IncrementCommandCount(commandBuffer);
    trackerObject->RecordIntoCommandList(commandBuffer, CMD_DRAWINDIRECTCOUNT);
}
void CommandCounter::PreCallRecordCmdDrawIndexedIndirectCountAMD(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset, VkBuffer countBuffer, VkDeviceSize countBufferOffset, uint32_t maxDrawCount, uint32_t stride) {
    trackerObject->IncrementCommandCount(commandBuffer);
    trackerObject->RecordIntoCommandList(commandBuffer, CMD_DRAWINDEXEDINDIRECTCOUNT);
}
void CommandCounter::PreCallRecordCmdBeginConditionalRenderingEXT(VkCommandBuffer commandBuffer, const VkConditionalRenderingBeginInfoEXT* pConditionalRenderingBegin) {
    trackerObject->IncrementCommandCount(commandBuffer);
    trackerObject->RecordIntoCommandList(commandBuffer, CMD_BEGINCONDITIONALRENDERINGEXT);
}
void CommandCounter::PreCallRecordCmdEndConditionalRenderingEXT(VkCommandBuffer commandBuffer) {
    trackerObject->IncrementCommandCount(commandBuffer);
    trackerObject->RecordIntoCommandList(commandBuffer, CMD_ENDCONDITIONALRENDERINGEXT);
}
void CommandCounter::PreCallRecordCmdSetViewportWScalingNV(VkCommandBuffer commandBuffer, uint32_t firstViewport, uint32_t viewportCount, const VkViewportWScalingNV* pViewportWScalings) {
    trackerObject->IncrementCommandCount(commandBuffer);
    trackerObject->RecordIntoCommandList(commandBuffer, CMD_SETVIEWPORTWSCALINGNV);
}
void CommandCounter::PreCallRecordCmdSetDiscardRectangleEXT(VkCommandBuffer commandBuffer, uint32_t firstDiscardRectangle, uint32_t discardRectangleCount, const VkRect2D* pDiscardRectangles) {
    trackerObject->IncrementCommandCount(commandBuffer);
    trackerObject->RecordIntoCommandList(commandBuffer, CMD_SETDISCARDRECTANGLEEXT);
}
void CommandCounter::PreCallRecordCmdBeginDebugUtilsLabelEXT(VkCommandBuffer commandBuffer, const VkDebugUtilsLabelEXT* pLabelInfo) {
    trackerObject->IncrementCommandCount(commandBuffer);
    trackerObject->RecordIntoCommandList(commandBuffer, CMD_BEGINDEBUGUTILSLABELEXT);
}
void CommandCounter::PreCallRecordCmdEndDebugUtilsLabelEXT(VkCommandBuffer commandBuffer) {
    trackerObject->IncrementCommandCount(commandBuffer);
    trackerObject->RecordIntoCommandList(commandBuffer, CMD_ENDDEBUGUTILSLABELEXT);
}
void CommandCounter::PreCallRecordCmdInsertDebugUtilsLabelEXT(VkCommandBuffer commandBuffer, const VkDebugUtilsLabelEXT* pLabelInfo) {
    trackerObject->IncrementCommandCount(commandBuffer);
    trackerObject->RecordIntoCommandList(commandBuffer, CMD_INSERTDEBUGUTILSLABELEXT);
}
void CommandCounter::PreCallRecordCmdSetSampleLocationsEXT(VkCommandBuffer commandBuffer, const VkSampleLocationsInfoEXT* pSampleLocationsInfo) {
    trackerObject->IncrementCommandCount(commandBuffer);
    trackerObject->RecordIntoCommandList(commandBuffer, CMD_SETSAMPLELOCATIONSEXT);
}
void CommandCounter::PreCallRecordCmdBindShadingRateImageNV(VkCommandBuffer commandBuffer, VkImageView imageView, VkImageLayout imageLayout) {
    trackerObject->IncrementCommandCount(commandBuffer);
    trackerObject->RecordIntoCommandList(commandBuffer, CMD_BINDSHADINGRATEIMAGENV);
}
void CommandCounter::PreCallRecordCmdSetViewportShadingRatePaletteNV(VkCommandBuffer commandBuffer, uint32_t firstViewport, uint32_t viewportCount, const VkShadingRatePaletteNV* pShadingRatePalettes) {
    trackerObject->IncrementCommandCount(commandBuffer);
    trackerObject->RecordIntoCommandList(commandBuffer, CMD_SETVIEWPORTSHADINGRATEPALETTENV);
}
void CommandCounter::PreCallRecordCmdSetCoarseSampleOrderNV(VkCommandBuffer commandBuffer, VkCoarseSampleOrderTypeNV sampleOrderType, uint32_t customSampleOrderCount, const VkCoarseSampleOrderCustomNV* pCustomSampleOrders) {
    trackerObject->IncrementCommandCount(commandBuffer);
    trackerObject->RecordIntoCommandList(commandBuffer, CMD_SETCOARSESAMPLEORDERNV);
}
void CommandCounter::PreCallRecordCmdBuildAccelerationStructureNV(VkCommandBuffer commandBuffer, const VkAccelerationStructureInfoNV* pInfo, VkBuffer instanceData, VkDeviceSize instanceOffset, VkBool32 update, VkAccelerationStructureKHR dst, VkAccelerationStructureKHR src, VkBuffer scratch, VkDeviceSize scratchOffset) {
    trackerObject->IncrementCommandCount(commandBuffer);
    trackerObject->RecordIntoCommandList(commandBuffer, CMD_BUILDACCELERATIONSTRUCTURENV);
}
void CommandCounter::PreCallRecordCmdCopyAccelerationStructureNV(VkCommandBuffer commandBuffer, VkAccelerationStructureKHR dst, VkAccelerationStructureKHR src, VkCopyAccelerationStructureModeKHR mode) {
    trackerObject->IncrementCommandCount(commandBuffer);
    trackerObject->RecordIntoCommandList(commandBuffer, CMD_COPYACCELERATIONSTRUCTURENV);
}
void CommandCounter::PreCallRecordCmdTraceRaysNV(VkCommandBuffer commandBuffer, VkBuffer raygenShaderBindingTableBuffer, VkDeviceSize raygenShaderBindingOffset, VkBuffer missShaderBindingTableBuffer, VkDeviceSize missShaderBindingOffset, VkDeviceSize missShaderBindingStride, VkBuffer hitShaderBindingTableBuffer, VkDeviceSize hitShaderBindingOffset, VkDeviceSize hitShaderBindingStride, VkBuffer callableShaderBindingTableBuffer, VkDeviceSize callableShaderBindingOffset, VkDeviceSize callableShaderBindingStride, uint32_t width, uint32_t height, uint32_t depth) {
    trackerObject->IncrementCommandCount(commandBuffer);
    trackerObject->RecordIntoCommandList(commandBuffer, CMD_TRACERAYSNV);
}
void CommandCounter::PreCallRecordCmdWriteAccelerationStructuresPropertiesKHR(VkCommandBuffer commandBuffer, uint32_t accelerationStructureCount, const VkAccelerationStructureKHR* pAccelerationStructures, VkQueryType queryType, VkQueryPool queryPool, uint32_t firstQuery) {
    trackerObject->IncrementCommandCount(commandBuffer);
    trackerObject->RecordIntoCommandList(commandBuffer, CMD_WRITEACCELERATIONSTRUCTURESPROPERTIESKHR);
}
void CommandCounter::PreCallRecordCmdWriteAccelerationStructuresPropertiesNV(VkCommandBuffer commandBuffer, uint32_t accelerationStructureCount, const VkAccelerationStructureKHR* pAccelerationStructures, VkQueryType queryType, VkQueryPool queryPool, uint32_t firstQuery) {
    trackerObject->IncrementCommandCount(commandBuffer);
    trackerObject->RecordIntoCommandList(commandBuffer, CMD_WRITEACCELERATIONSTRUCTURESPROPERTIESKHR);
}
void CommandCounter::PreCallRecordCmdWriteBufferMarkerAMD(VkCommandBuffer commandBuffer, VkPipelineStageFlagBits pipelineStage, VkBuffer dstBuffer, VkDeviceSize dstOffset, uint32_t marker) {
    trackerObject->IncrementCommandCount(commandBuffer);
    trackerObject->RecordIntoCommandList(commandBuffer, CMD_WRITEBUFFERMARKERAMD);
}
void CommandCounter::PreCallRecordCmdDrawMeshTasksNV(VkCommandBuffer commandBuffer, uint32_t taskCount, uint32_t firstTask) {
    trackerObject->IncrementCommandCount(commandBuffer);
    trackerObject->RecordIntoCommandList(commandBuffer, CMD_DRAWMESHTASKSNV);
}
void CommandCounter::PreCallRecordCmdDrawMeshTasksIndirectNV(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset, uint32_t drawCount, uint32_t stride) {
    trackerObject->IncrementCommandCount(commandBuffer);
    trackerObject->RecordIntoCommandList(commandBuffer, CMD_DRAWMESHTASKSINDIRECTNV);
}
void CommandCounter::PreCallRecordCmdDrawMeshTasksIndirectCountNV(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset, VkBuffer countBuffer, VkDeviceSize countBufferOffset, uint32_t maxDrawCount, uint32_t stride) {
    trackerObject->IncrementCommandCount(commandBuffer);
    trackerObject->RecordIntoCommandList(commandBuffer, CMD_DRAWMESHTASKSINDIRECTCOUNTNV);
}
void CommandCounter::PreCallRecordCmdSetExclusiveScissorNV(VkCommandBuffer commandBuffer, uint32_t firstExclusiveScissor, uint32_t exclusiveScissorCount, const VkRect2D* pExclusiveScissors) {
    trackerObject->IncrementCommandCount(commandBuffer);
    trackerObject->RecordIntoCommandList(commandBuffer, CMD_SETEXCLUSIVESCISSORNV);
}
void CommandCounter::PreCallRecordCmdSetCheckpointNV(VkCommandBuffer commandBuffer, const void* pCheckpointMarker) {
    trackerObject->IncrementCommandCount(commandBuffer);
    trackerObject->RecordIntoCommandList(commandBuffer, CMD_SETCHECKPOINTNV);
}
void CommandCounter::PreCallRecordCmdSetPerformanceMarkerINTEL(VkCommandBuffer commandBuffer, const VkPerformanceMarkerInfoINTEL* pMarkerInfo) {
    trackerObject->IncrementCommandCount(commandBuffer);
    trackerObject->RecordIntoCommandList(commandBuffer, CMD_SETPERFORMANCEMARKERINTEL);
}
void CommandCounter::PreCallRecordCmdSetPerformanceStreamMarkerINTEL(VkCommandBuffer commandBuffer, const VkPerformanceStreamMarkerInfoINTEL* pMarkerInfo) {
    trackerObject->IncrementCommandCount(commandBuffer);
    trackerObject->RecordIntoCommandList(commandBuffer, CMD_SETPERFORMANCESTREAMMARKERINTEL);
}
void CommandCounter::PreCallRecordCmdSetPerformanceOverrideINTEL(VkCommandBuffer commandBuffer, const VkPerformanceOverrideInfoINTEL* pOverrideInfo) {
    trackerObject->IncrementCommandCount(commandBuffer);
    trackerObject->RecordIntoCommandList(commandBuffer, CMD_SETPERFORMANCEOVERRIDEINTEL);
}
void CommandCounter::PreCallRecordCmdSetLineStippleEXT(VkCommandBuffer commandBuffer, uint32_t lineStippleFactor, uint16_t lineStipplePattern) {
    trackerObject->IncrementCommandCount(commandBuffer);
    trackerObject->RecordIntoCommandList(commandBuffer, CMD_SETLINESTIPPLEEXT);
}
void CommandCounter::PreCallRecordCmdSetCullModeEXT(VkCommandBuffer commandBuffer, VkCullModeFlags cullMode) {
    trackerObject->IncrementCommandCount(commandBuffer);
    trackerObject->RecordIntoCommandList(commandBuffer, CMD_SETCULLMODEEXT);
}
void CommandCounter::PreCallRecordCmdSetFrontFaceEXT(VkCommandBuffer commandBuffer, VkFrontFace frontFace) {
    trackerObject->IncrementCommandCount(commandBuffer);
    trackerObject->RecordIntoCommandList(commandBuffer, CMD_SETFRONTFACEEXT);
}
void CommandCounter::PreCallRecordCmdSetPrimitiveTopologyEXT(VkCommandBuffer commandBuffer, VkPrimitiveTopology primitiveTopology) {
    trackerObject->IncrementCommandCount(commandBuffer);
    trackerObject->RecordIntoCommandList(commandBuffer, CMD_SETPRIMITIVETOPOLOGYEXT);
}
void CommandCounter::PreCallRecordCmdSetViewportWithCountEXT(VkCommandBuffer commandBuffer, uint32_t viewportCount, const VkViewport* pViewports) {
    trackerObject->IncrementCommandCount(commandBuffer);
    trackerObject->RecordIntoCommandList(commandBuffer, CMD_SETVIEWPORTWITHCOUNTEXT);
}
void CommandCounter::PreCallRecordCmdSetScissorWithCountEXT(VkCommandBuffer commandBuffer, uint32_t scissorCount, const VkRect2D* pScissors) {
    trackerObject->IncrementCommandCount(commandBuffer);
    trackerObject->RecordIntoCommandList(commandBuffer, CMD_SETSCISSORWITHCOUNTEXT);
}
void CommandCounter::PreCallRecordCmdBindVertexBuffers2EXT(VkCommandBuffer commandBuffer, uint32_t firstBinding, uint32_t bindingCount, const VkBuffer* pBuffers, const VkDeviceSize* pOffsets, const VkDeviceSize* pSizes, const VkDeviceSize* pStrides) {
    trackerObject->IncrementCommandCount(commandBuffer);
    trackerObject->RecordIntoCommandList(commandBuffer, CMD_BINDVERTEXBUFFERS2EXT);
}
void CommandCounter::PreCallRecordCmdSetDepthTestEnableEXT(VkCommandBuffer commandBuffer, VkBool32 depthTestEnable) {
    trackerObject->IncrementCommandCount(commandBuffer);
    trackerObject->RecordIntoCommandList(commandBuffer, CMD_SETDEPTHTESTENABLEEXT);
}
void CommandCounter::PreCallRecordCmdSetDepthWriteEnableEXT(VkCommandBuffer commandBuffer, VkBool32 depthWriteEnable) {
    trackerObject->IncrementCommandCount(commandBuffer);
    trackerObject->RecordIntoCommandList(commandBuffer, CMD_SETDEPTHWRITEENABLEEXT);
}
void CommandCounter::PreCallRecordCmdSetDepthCompareOpEXT(VkCommandBuffer commandBuffer, VkCompareOp depthCompareOp) {
    trackerObject->IncrementCommandCount(commandBuffer);
    trackerObject->RecordIntoCommandList(commandBuffer, CMD_SETDEPTHCOMPAREOPEXT);
}
void CommandCounter::PreCallRecordCmdSetDepthBoundsTestEnableEXT(VkCommandBuffer commandBuffer, VkBool32 depthBoundsTestEnable) {
    trackerObject->IncrementCommandCount(commandBuffer);
    trackerObject->RecordIntoCommandList(commandBuffer, CMD_SETDEPTHBOUNDSTESTENABLEEXT);
}
void CommandCounter::PreCallRecordCmdSetStencilTestEnableEXT(VkCommandBuffer commandBuffer, VkBool32 stencilTestEnable) {
    trackerObject->IncrementCommandCount(commandBuffer);
    trackerObject->RecordIntoCommandList(commandBuffer, CMD_SETSTENCILTESTENABLEEXT);
}
void CommandCounter::PreCallRecordCmdSetStencilOpEXT(VkCommandBuffer commandBuffer, VkStencilFaceFlags faceMask, VkStencilOp failOp, VkStencilOp passOp, VkStencilOp depthFailOp, VkCompareOp compareOp) {
    trackerObject->IncrementCommandCount(commandBuffer);
    trackerObject->RecordIntoCommandList(commandBuffer, CMD_SETSTENCILOPEXT);
}
void CommandCounter::PreCallRecordCmdPreprocessGeneratedCommandsNV(VkCommandBuffer commandBuffer, const VkGeneratedCommandsInfoNV* pGeneratedCommandsInfo) {
    trackerObject->IncrementCommandCount(commandBuffer);
    trackerObject->RecordIntoCommandList(commandBuffer, CMD_PREPROCESSGENERATEDCOMMANDSNV);
}
void CommandCounter::PreCallRecordCmdExecuteGeneratedCommandsNV(VkCommandBuffer commandBuffer, VkBool32 isPreprocessed, const VkGeneratedCommandsInfoNV* pGeneratedCommandsInfo) {
    trackerObject->IncrementCommandCount(commandBuffer);
    trackerObject->RecordIntoCommandList(commandBuffer, CMD_EXECUTEGENERATEDCOMMANDSNV);
}
void CommandCounter::PreCallRecordCmdBindPipelineShaderGroupNV(VkCommandBuffer commandBuffer, VkPipelineBindPoint pipelineBindPoint, VkPipeline pipeline, uint32_t groupIndex) {
    trackerObject->IncrementCommandCount(commandBuffer);
    trackerObject->RecordIntoCommandList(commandBuffer, CMD_BINDPIPELINESHADERGROUPNV);
}
void CommandCounter::PreCallRecordCmdSetFragmentShadingRateEnumNV(VkCommandBuffer           commandBuffer, VkFragmentShadingRateNV                     shadingRate, const VkFragmentShadingRateCombinerOpKHR    combinerOps[2]) {
    trackerObject->IncrementCommandCount(commandBuffer);
    trackerObject->RecordIntoCommandList(commandBuffer, CMD_SETFRAGMENTSHADINGRATEENUMNV);
}
#ifdef VK_ENABLE_BETA_EXTENSIONS
void CommandCounter::PreCallRecordCmdBuildAccelerationStructureKHR(VkCommandBuffer                                    commandBuffer, uint32_t infoCount, const VkAccelerationStructureBuildGeometryInfoKHR* pInfos, const VkAccelerationStructureBuildOffsetInfoKHR* const* ppOffsetInfos) {
    trackerObject->IncrementCommandCount(commandBuffer);
    trackerObject->RecordIntoCommandList(commandBuffer, CMD_BUILDACCELERATIONSTRUCTUREKHR);
}
#endif // VK_ENABLE_BETA_EXTENSIONS
#ifdef VK_ENABLE_BETA_EXTENSIONS
void CommandCounter::PreCallRecordCmdBuildAccelerationStructureIndirectKHR(VkCommandBuffer                  commandBuffer, const VkAccelerationStructureBuildGeometryInfoKHR* pInfo, VkBuffer                                           indirectBuffer, VkDeviceSize                                       indirectOffset, uint32_t                                           indirectStride) {
    trackerObject->IncrementCommandCount(commandBuffer);
    trackerObject->RecordIntoCommandList(commandBuffer, CMD_BUILDACCELERATIONSTRUCTUREINDIRECTKHR);
}
#endif // VK_ENABLE_BETA_EXTENSIONS
#ifdef VK_ENABLE_BETA_EXTENSIONS
void CommandCounter::PreCallRecordCmdCopyAccelerationStructureKHR(VkCommandBuffer commandBuffer, const VkCopyAccelerationStructureInfoKHR* pInfo) {
    trackerObject->IncrementCommandCount(commandBuffer);
    trackerObject->RecordIntoCommandList(commandBuffer, CMD_COPYACCELERATIONSTRUCTUREKHR);
}
#endif // VK_ENABLE_BETA_EXTENSIONS
#ifdef VK_ENABLE_BETA_EXTENSIONS
void CommandCounter::PreCallRecordCmdCopyAccelerationStructureToMemoryKHR(VkCommandBuffer commandBuffer, const VkCopyAccelerationStructureToMemoryInfoKHR* pInfo) {
    trackerObject->IncrementCommandCount(commandBuffer);
    trackerObject->RecordIntoCommandList(commandBuffer, CMD_COPYACCELERATIONSTRUCTURETOMEMORYKHR);
}
#endif // VK_ENABLE_BETA_EXTENSIONS
#ifdef VK_ENABLE_BETA_EXTENSIONS
void CommandCounter::PreCallRecordCmdCopyMemoryToAccelerationStructureKHR(VkCommandBuffer commandBuffer, const VkCopyMemoryToAccelerationStructureInfoKHR* pInfo) {
    trackerObject->IncrementCommandCount(commandBuffer);
    trackerObject->RecordIntoCommandList(commandBuffer, CMD_COPYMEMORYTOACCELERATIONSTRUCTUREKHR);
}
#endif // VK_ENABLE_BETA_EXTENSIONS
#ifdef VK_ENABLE_BETA_EXTENSIONS
void CommandCounter::PreCallRecordCmdTraceRaysKHR(VkCommandBuffer commandBuffer, const VkStridedBufferRegionKHR* pRaygenShaderBindingTable, const VkStridedBufferRegionKHR* pMissShaderBindingTable, const VkStridedBufferRegionKHR* pHitShaderBindingTable, const VkStridedBufferRegionKHR* pCallableShaderBindingTable, uint32_t width, uint32_t height, uint32_t depth) {
    trackerObject->IncrementCommandCount(commandBuffer);
    trackerObject->RecordIntoCommandList(commandBuffer, CMD_TRACERAYSKHR);
}
#endif // VK_ENABLE_BETA_EXTENSIONS
#ifdef VK_ENABLE_BETA_EXTENSIONS
void CommandCounter::PreCallRecordCmdTraceRaysIndirectKHR(VkCommandBuffer commandBuffer, const VkStridedBufferRegionKHR* pRaygenShaderBindingTable, const VkStridedBufferRegionKHR* pMissShaderBindingTable, const VkStridedBufferRegionKHR* pHitShaderBindingTable, const VkStridedBufferRegionKHR* pCallableShaderBindingTable, VkBuffer buffer, VkDeviceSize offset) {
    trackerObject->IncrementCommandCount(commandBuffer);
    trackerObject->RecordIntoCommandList(commandBuffer, CMD_TRACERAYSINDIRECTKHR);
}
#endif // VK_ENABLE_BETA_EXTENSIONS
