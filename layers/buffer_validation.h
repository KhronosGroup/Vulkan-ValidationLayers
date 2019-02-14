/* Copyright (c) 2015-2019 The Khronos Group Inc.
 * Copyright (c) 2015-2019 Valve Corporation
 * Copyright (c) 2015-2019 LunarG, Inc.
 * Copyright (C) 2015-2019 Google Inc.
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
 * Mark Lobodzinski <mark@lunarg.com>
 * Dave Houlton <daveh@lunarg.com>
 */
#ifndef CORE_VALIDATION_BUFFER_VALIDATION_H_
#define CORE_VALIDATION_BUFFER_VALIDATION_H_

#include "vulkan/vk_layer.h"
#include <limits.h>
#include <memory>
#include <unordered_map>
#include <vector>
#include <utility>
#include <algorithm>
#include <bitset>

using core_validation::instance_layer_data;
using core_validation::layer_data;

uint32_t FullMipChainLevels(uint32_t height, uint32_t width = 1, uint32_t depth = 1);
uint32_t FullMipChainLevels(VkExtent3D);
uint32_t FullMipChainLevels(VkExtent2D);

bool PreCallValidateCreateImage(VkDevice device, const VkImageCreateInfo *pCreateInfo, const VkAllocationCallbacks *pAllocator,
                                VkImage *pImage);

void PostCallRecordCreateImage(VkDevice device, const VkImageCreateInfo *pCreateInfo, const VkAllocationCallbacks *pAllocator,
                               VkImage *pImage, VkResult result);

void PreCallRecordDestroyImage(VkDevice device, VkImage image, const VkAllocationCallbacks *pAllocator);

bool PreCallValidateDestroyImage(VkDevice device, VkImage image, const VkAllocationCallbacks *pAllocator);

bool ValidateImageAttributes(layer_data *device_data, IMAGE_STATE *image_state, VkImageSubresourceRange range);

uint32_t ResolveRemainingLevels(const VkImageSubresourceRange *range, uint32_t mip_levels);

uint32_t ResolveRemainingLayers(const VkImageSubresourceRange *range, uint32_t layers);

bool VerifyClearImageLayout(layer_data *device_data, GLOBAL_CB_NODE *cb_node, IMAGE_STATE *image_state,
                            VkImageSubresourceRange range, VkImageLayout dest_image_layout, const char *func_name);

bool VerifyImageLayout(layer_data const *device_data, GLOBAL_CB_NODE const *cb_node, IMAGE_STATE *image_state,
                       VkImageSubresourceLayers subLayers, VkImageLayout explicit_layout, VkImageLayout optimal_layout,
                       const char *caller, const char *layout_invalid_msg_code, const char *layout_mismatch_msg_code, bool *error);

void RecordClearImageLayout(layer_data *dev_data, GLOBAL_CB_NODE *cb_node, VkImage image, VkImageSubresourceRange range,
                            VkImageLayout dest_image_layout);

bool PreCallValidateCmdClearColorImage(VkCommandBuffer commandBuffer, VkImage image, VkImageLayout imageLayout,
                                       const VkClearColorValue *pColor, uint32_t rangeCount,
                                       const VkImageSubresourceRange *pRanges);

void PreCallRecordCmdClearColorImage(VkCommandBuffer commandBuffer, VkImage image, VkImageLayout imageLayout,
                                     const VkClearColorValue *pColor, uint32_t rangeCount, const VkImageSubresourceRange *pRanges);

bool PreCallValidateCmdClearDepthStencilImage(VkCommandBuffer commandBuffer, VkImage image, VkImageLayout imageLayout,
                                              const VkClearDepthStencilValue *pDepthStencil, uint32_t rangeCount,
                                              const VkImageSubresourceRange *pRanges);

void PreCallRecordCmdClearDepthStencilImage(VkCommandBuffer commandBuffer, VkImage image, VkImageLayout imageLayout,
                                            const VkClearDepthStencilValue *pDepthStencil, uint32_t rangeCount,
                                            const VkImageSubresourceRange *pRanges);

bool FindLayoutVerifyNode(layer_data const *device_data, GLOBAL_CB_NODE const *pCB, ImageSubresourcePair imgpair,
                          IMAGE_CMD_BUF_LAYOUT_NODE &node, const VkImageAspectFlags aspectMask);

bool FindLayoutVerifyLayout(layer_data const *device_data, ImageSubresourcePair imgpair, VkImageLayout &layout,
                            const VkImageAspectFlags aspectMask);

bool FindCmdBufLayout(layer_data const *device_data, GLOBAL_CB_NODE const *pCB, VkImage image, VkImageSubresource range,
                      IMAGE_CMD_BUF_LAYOUT_NODE &node);

bool FindGlobalLayout(layer_data *device_data, ImageSubresourcePair imgpair, VkImageLayout &layout);

bool FindLayouts(layer_data *device_data, VkImage image, std::vector<VkImageLayout> &layouts);

bool FindLayout(const std::unordered_map<ImageSubresourcePair, IMAGE_LAYOUT_NODE> &imageLayoutMap, ImageSubresourcePair imgpair,
                VkImageLayout &layout, const VkImageAspectFlags aspectMask);

bool FindLayout(layer_data *device_data, const std::unordered_map<ImageSubresourcePair, IMAGE_LAYOUT_NODE> &imageLayoutMap,
                ImageSubresourcePair imgpair, VkImageLayout &layout);

void SetGlobalLayout(layer_data *device_data, ImageSubresourcePair imgpair, const VkImageLayout &layout);

void SetLayout(layer_data *device_data, GLOBAL_CB_NODE *pCB, ImageSubresourcePair imgpair, const IMAGE_CMD_BUF_LAYOUT_NODE &node);

void SetLayout(layer_data *device_data, GLOBAL_CB_NODE *pCB, ImageSubresourcePair imgpair, const VkImageLayout &layout);

void SetLayout(std::unordered_map<ImageSubresourcePair, IMAGE_LAYOUT_NODE> &imageLayoutMap, ImageSubresourcePair imgpair,
               VkImageLayout layout);

void SetImageViewLayout(layer_data *device_data, GLOBAL_CB_NODE *pCB, VkImageView imageView, const VkImageLayout &layout);
void SetImageViewLayout(layer_data *device_data, GLOBAL_CB_NODE *cb_node, IMAGE_VIEW_STATE *view_state,
                        const VkImageLayout &layout);

bool VerifyFramebufferAndRenderPassLayouts(layer_data *dev_data, RenderPassCreateVersion rp_version, GLOBAL_CB_NODE *pCB,
                                           const VkRenderPassBeginInfo *pRenderPassBegin,
                                           const FRAMEBUFFER_STATE *framebuffer_state);

void TransitionAttachmentRefLayout(layer_data *dev_data, GLOBAL_CB_NODE *pCB, FRAMEBUFFER_STATE *pFramebuffer,
                                   const safe_VkAttachmentReference2KHR &ref);

void TransitionSubpassLayouts(layer_data *, GLOBAL_CB_NODE *, const RENDER_PASS_STATE *, const int, FRAMEBUFFER_STATE *);

void TransitionBeginRenderPassLayouts(layer_data *, GLOBAL_CB_NODE *, const RENDER_PASS_STATE *, FRAMEBUFFER_STATE *);

bool ValidateImageAspectLayout(layer_data *device_data, GLOBAL_CB_NODE *pCB, const VkImageMemoryBarrier *mem_barrier,
                               uint32_t level, uint32_t layer, VkImageAspectFlags aspect);

void TransitionImageAspectLayout(layer_data *dev_data, GLOBAL_CB_NODE *pCB, const VkImageMemoryBarrier *mem_barrier, uint32_t level,
                                 uint32_t layer, VkImageAspectFlags aspect);

bool ValidateBarrierLayoutToImageUsage(layer_data *device_data, const VkImageMemoryBarrier *img_barrier, bool new_not_old,
                                       VkImageUsageFlags usage, const char *func_name);

bool ValidateBarriersToImages(layer_data *device_data, GLOBAL_CB_NODE const *cb_state, uint32_t imageMemoryBarrierCount,
                              const VkImageMemoryBarrier *pImageMemoryBarriers, const char *func_name);

bool ValidateBarriersQFOTransferUniqueness(layer_data *device_data, const char *func_name, GLOBAL_CB_NODE *cb_state,
                                           uint32_t bufferBarrierCount, const VkBufferMemoryBarrier *pBufferMemBarriers,
                                           uint32_t imageMemBarrierCount, const VkImageMemoryBarrier *pImageMemBarriers);

void RecordBarriersQFOTransfers(layer_data *device_data, GLOBAL_CB_NODE *cb_state, uint32_t bufferBarrierCount,
                                const VkBufferMemoryBarrier *pBufferMemBarriers, uint32_t imageMemBarrierCount,
                                const VkImageMemoryBarrier *pImageMemBarriers);

bool ValidateQueuedQFOTransfers(layer_data *dev_data, GLOBAL_CB_NODE *pCB,
                                QFOTransferCBScoreboards<VkImageMemoryBarrier> *qfo_image_scoreboards,
                                QFOTransferCBScoreboards<VkBufferMemoryBarrier> *qfo_buffer_scoreboards);

void RecordQueuedQFOTransfers(layer_data *dev_data, GLOBAL_CB_NODE *pCB);
void EraseQFOImageRelaseBarriers(layer_data *device_data, const VkImage &image);

void TransitionImageLayouts(layer_data *device_data, GLOBAL_CB_NODE *cb_state, uint32_t memBarrierCount,
                            const VkImageMemoryBarrier *pImgMemBarriers);

void TransitionFinalSubpassLayouts(layer_data *dev_data, GLOBAL_CB_NODE *pCB, const VkRenderPassBeginInfo *pRenderPassBegin,
                                   FRAMEBUFFER_STATE *framebuffer_state);

bool PreCallValidateCmdCopyImage(VkCommandBuffer commandBuffer, VkImage srcImage, VkImageLayout srcImageLayout, VkImage dstImage,
                                 VkImageLayout dstImageLayout, uint32_t regionCount, const VkImageCopy *pRegions);

bool PreCallValidateCmdClearAttachments(VkCommandBuffer commandBuffer, uint32_t attachmentCount,
                                        const VkClearAttachment *pAttachments, uint32_t rectCount, const VkClearRect *pRects);

bool PreCallValidateCmdResolveImage(VkCommandBuffer commandBuffer, VkImage srcImage, VkImageLayout srcImageLayout, VkImage dstImage,
                                    VkImageLayout dstImageLayout, uint32_t regionCount, const VkImageResolve *pRegions);

void PreCallRecordCmdResolveImage(VkCommandBuffer commandBuffer, VkImage srcImage, VkImageLayout srcImageLayout, VkImage dstImage,
                                  VkImageLayout dstImageLayout, uint32_t regionCount, const VkImageResolve *pRegions);

bool PreCallValidateCmdBlitImage(VkCommandBuffer commandBuffer, VkImage srcImage, VkImageLayout srcImageLayout, VkImage dstImage,
                                 VkImageLayout dstImageLayout, uint32_t regionCount, const VkImageBlit *pRegions, VkFilter filter);

void PreCallRecordCmdBlitImage(VkCommandBuffer commandBuffer, VkImage srcImage, VkImageLayout srcImageLayout, VkImage dstImage,
                               VkImageLayout dstImageLayout, uint32_t regionCount, const VkImageBlit *pRegions, VkFilter filter);

bool ValidateCmdBufImageLayouts(layer_data *device_data, GLOBAL_CB_NODE *pCB,
                                std::unordered_map<ImageSubresourcePair, IMAGE_LAYOUT_NODE> const &globalImageLayoutMap,
                                std::unordered_map<ImageSubresourcePair, IMAGE_LAYOUT_NODE> &overlayLayoutMap);

void UpdateCmdBufImageLayouts(layer_data *device_data, GLOBAL_CB_NODE *pCB);

bool ValidateMaskBitsFromLayouts(core_validation::layer_data *device_data, VkCommandBuffer cmdBuffer,
                                 const VkAccessFlags &accessMask, const VkImageLayout &layout, const char *type);

bool ValidateLayoutVsAttachmentDescription(const debug_report_data *report_data, RenderPassCreateVersion rp_version,
                                           const VkImageLayout first_layout, const uint32_t attachment,
                                           const VkAttachmentDescription2KHR &attachment_description);

bool ValidateLayouts(const core_validation::layer_data *dev_data, RenderPassCreateVersion rp_version, VkDevice device,
                     const VkRenderPassCreateInfo2KHR *pCreateInfo);

bool ValidateMapImageLayouts(core_validation::layer_data *dev_data, VkDevice device, DEVICE_MEM_INFO const *mem_info,
                             VkDeviceSize offset, VkDeviceSize end_offset);

bool ValidateImageUsageFlags(layer_data *dev_data, IMAGE_STATE const *image_state, VkFlags desired, bool strict,
                             const char *msgCode, char const *func_name, char const *usage_string);

bool ValidateImageFormatFeatureFlags(layer_data *dev_data, IMAGE_STATE const *image_state, VkFormatFeatureFlags desired,
                                     char const *func_name, const char *linear_vuid, const char *optimal_vuid);

bool ValidateImageSubresourceLayers(layer_data *dev_data, const GLOBAL_CB_NODE *cb_node,
                                    const VkImageSubresourceLayers *subresource_layers, char const *func_name, char const *member,
                                    uint32_t i);

bool ValidateBufferUsageFlags(const layer_data *dev_data, BUFFER_STATE const *buffer_state, VkFlags desired, bool strict,
                              const char *msgCode, char const *func_name, char const *usage_string);

bool PreCallValidateCreateBuffer(VkDevice device, const VkBufferCreateInfo *pCreateInfo, const VkAllocationCallbacks *pAllocator,
                                 VkBuffer *pBuffer);

void PostCallRecordCreateBuffer(VkDevice device, const VkBufferCreateInfo *pCreateInfo, const VkAllocationCallbacks *pAllocator,
                                VkBuffer *pBuffer, VkResult result);

bool PreCallValidateCreateBufferView(VkDevice device, const VkBufferViewCreateInfo *pCreateInfo,
                                     const VkAllocationCallbacks *pAllocator, VkBufferView *pView);

void PostCallRecordCreateBufferView(VkDevice device, const VkBufferViewCreateInfo *pCreateInfo,
                                    const VkAllocationCallbacks *pAllocator, VkBufferView *pView, VkResult result);

bool ValidateImageAspectMask(const layer_data *device_data, VkImage image, VkFormat format, VkImageAspectFlags aspect_mask,
                             const char *func_name, const char *vuid = "VUID-VkImageSubresource-aspectMask-parameter");

bool ValidateCreateImageViewSubresourceRange(const layer_data *device_data, const IMAGE_STATE *image_state,
                                             bool is_imageview_2d_type, const VkImageSubresourceRange &subresourceRange);

bool ValidateCmdClearColorSubresourceRange(const layer_data *device_data, const IMAGE_STATE *image_state,
                                           const VkImageSubresourceRange &subresourceRange, const char *param_name);

bool ValidateCmdClearDepthSubresourceRange(const layer_data *device_data, const IMAGE_STATE *image_state,
                                           const VkImageSubresourceRange &subresourceRange, const char *param_name);

bool ValidateImageBarrierSubresourceRange(const layer_data *device_data, const IMAGE_STATE *image_state,
                                          const VkImageSubresourceRange &subresourceRange, const char *cmd_name,
                                          const char *param_name);

bool PreCallValidateCreateImageView(VkDevice device, const VkImageViewCreateInfo *pCreateInfo,
                                    const VkAllocationCallbacks *pAllocator, VkImageView *pView);

void PostCallRecordCreateImageView(VkDevice device, const VkImageViewCreateInfo *pCreateInfo,
                                   const VkAllocationCallbacks *pAllocator, VkImageView *pView, VkResult result);

bool ValidateCopyBufferImageTransferGranularityRequirements(layer_data *device_data, const GLOBAL_CB_NODE *cb_node,
                                                            const IMAGE_STATE *img, const VkBufferImageCopy *region,
                                                            const uint32_t i, const char *function, const char *vuid);

bool ValidateImageMipLevel(layer_data *device_data, const GLOBAL_CB_NODE *cb_node, const IMAGE_STATE *img, uint32_t mip_level,
                           const uint32_t i, const char *function, const char *member, const char *vuid);

bool ValidateImageArrayLayerRange(layer_data *device_data, const GLOBAL_CB_NODE *cb_node, const IMAGE_STATE *img,
                                  const uint32_t base_layer, const uint32_t layer_count, const uint32_t i, const char *function,
                                  const char *member, const char *vuid);

void PreCallRecordCmdCopyImage(VkCommandBuffer commandBuffer, VkImage srcImage, VkImageLayout srcImageLayout, VkImage dstImage,
                               VkImageLayout dstImageLayout, uint32_t regionCount, const VkImageCopy *pRegions);

bool PreCallValidateCmdCopyBuffer(VkCommandBuffer commandBuffer, VkBuffer srcBuffer, VkBuffer dstBuffer, uint32_t regionCount,
                                  const VkBufferCopy *pRegions);

void PreCallRecordCmdCopyBuffer(VkCommandBuffer commandBuffer, VkBuffer srcBuffer, VkBuffer dstBuffer, uint32_t regionCount,
                                const VkBufferCopy *pRegions);

bool PreCallValidateDestroyImageView(VkDevice device, VkImageView imageView, const VkAllocationCallbacks *pAllocator);

void PreCallRecordDestroyImageView(VkDevice device, VkImageView imageView, const VkAllocationCallbacks *pAllocator);

bool PreCallValidateDestroyBuffer(VkDevice device, VkBuffer buffer, const VkAllocationCallbacks *pAllocator);

void PreCallRecordDestroyBuffer(VkDevice device, VkBuffer buffer, const VkAllocationCallbacks *pAllocator);

bool PreCallValidateDestroyBufferView(VkDevice device, VkBufferView bufferView, const VkAllocationCallbacks *pAllocator);

void PreCallRecordDestroyBufferView(VkDevice device, VkBufferView bufferView, const VkAllocationCallbacks *pAllocator);

bool PreCallValidateCmdFillBuffer(VkCommandBuffer commandBuffer, VkBuffer dstBuffer, VkDeviceSize dstOffset, VkDeviceSize size,
                                  uint32_t data);

void PreCallRecordCmdFillBuffer(VkCommandBuffer commandBuffer, VkBuffer dstBuffer, VkDeviceSize dstOffset, VkDeviceSize size,
                                uint32_t data);

bool PreCallValidateCmdCopyImageToBuffer(VkCommandBuffer commandBuffer, VkImage srcImage, VkImageLayout srcImageLayout,
                                         VkBuffer dstBuffer, uint32_t regionCount, const VkBufferImageCopy *pRegions);

void PreCallRecordCmdCopyImageToBuffer(VkCommandBuffer commandBuffer, VkImage srcImage, VkImageLayout srcImageLayout,
                                       VkBuffer dstBuffer, uint32_t regionCount, const VkBufferImageCopy *pRegions);

bool PreCallValidateCmdCopyBufferToImage(VkCommandBuffer commandBuffer, VkBuffer srcBuffer, VkImage dstImage,
                                         VkImageLayout dstImageLayout, uint32_t regionCount, const VkBufferImageCopy *pRegions);

void PreCallRecordCmdCopyBufferToImage(VkCommandBuffer commandBuffer, VkBuffer srcBuffer, VkImage dstImage,
                                       VkImageLayout dstImageLayout, uint32_t regionCount, const VkBufferImageCopy *pRegions);

bool PreCallValidateGetImageSubresourceLayout(VkDevice device, VkImage image, const VkImageSubresource *pSubresource,
                                              VkSubresourceLayout *pLayout);

#endif  // CORE_VALIDATION_BUFFER_VALIDATION_H_
