/* Copyright (c) 2015-2023 The Khronos Group Inc.
 * Copyright (c) 2015-2023 Valve Corporation
 * Copyright (c) 2015-2023 LunarG, Inc.
 * Copyright (C) 2015-2023 Google Inc.
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

#include "stateless/stateless_validation.h"
#include "generated/enum_flag_bits.h"

ReadLockGuard StatelessValidation::ReadLock() const { return ReadLockGuard(validation_object_mutex, std::defer_lock); }
WriteLockGuard StatelessValidation::WriteLock() { return WriteLockGuard(validation_object_mutex, std::defer_lock); }

static vvl::unordered_map<VkCommandBuffer, VkCommandPool> secondary_cb_map{};
static std::shared_mutex secondary_cb_map_mutex;
static ReadLockGuard CBReadLock() { return ReadLockGuard(secondary_cb_map_mutex); }
static WriteLockGuard CBWriteLock() { return WriteLockGuard(secondary_cb_map_mutex); }

bool StatelessValidation::manual_PreCallValidateCmdBindIndexBuffer(VkCommandBuffer commandBuffer, VkBuffer buffer,
                                                                   VkDeviceSize offset, VkIndexType indexType) const {
    bool skip = false;

    if (indexType == VK_INDEX_TYPE_NONE_NV) {
        skip |= LogError(commandBuffer, "VUID-vkCmdBindIndexBuffer-indexType-02507",
                         "vkCmdBindIndexBuffer() indexType must not be VK_INDEX_TYPE_NONE_NV.");
    }

    const auto *index_type_uint8_features = LvlFindInChain<VkPhysicalDeviceIndexTypeUint8FeaturesEXT>(device_createinfo_pnext);
    if (indexType == VK_INDEX_TYPE_UINT8_EXT && (!index_type_uint8_features || !index_type_uint8_features->indexTypeUint8)) {
        skip |= LogError(commandBuffer, "VUID-vkCmdBindIndexBuffer-indexType-02765",
                         "vkCmdBindIndexBuffer() indexType is VK_INDEX_TYPE_UINT8_EXT but indexTypeUint8 feature is not enabled.");
    }

    return skip;
}

bool StatelessValidation::manual_PreCallValidateCmdBindVertexBuffers(VkCommandBuffer commandBuffer, uint32_t firstBinding,
                                                                     uint32_t bindingCount, const VkBuffer *pBuffers,
                                                                     const VkDeviceSize *pOffsets) const {
    bool skip = false;
    if (firstBinding > device_limits.maxVertexInputBindings) {
        skip |=
            LogError(commandBuffer, "VUID-vkCmdBindVertexBuffers-firstBinding-00624",
                     "vkCmdBindVertexBuffers() firstBinding (%" PRIu32 ") must be less than maxVertexInputBindings (%" PRIu32 ")",
                     firstBinding, device_limits.maxVertexInputBindings);
    } else if ((firstBinding + bindingCount) > device_limits.maxVertexInputBindings) {
        skip |= LogError(commandBuffer, "VUID-vkCmdBindVertexBuffers-firstBinding-00625",
                         "vkCmdBindVertexBuffers() sum of firstBinding (%" PRIu32 ") and bindingCount (%" PRIu32
                         ") must be less than "
                         "maxVertexInputBindings (%" PRIu32 ")",
                         firstBinding, bindingCount, device_limits.maxVertexInputBindings);
    }

    for (uint32_t i = 0; i < bindingCount; ++i) {
        if (pBuffers[i] == VK_NULL_HANDLE) {
            const auto *robustness2_features = LvlFindInChain<VkPhysicalDeviceRobustness2FeaturesEXT>(device_createinfo_pnext);
            if (!(robustness2_features && robustness2_features->nullDescriptor)) {
                skip |=
                    LogError(commandBuffer, "VUID-vkCmdBindVertexBuffers-pBuffers-04001",
                             "vkCmdBindVertexBuffers() required parameter pBuffers[%" PRIu32 "] specified as VK_NULL_HANDLE", i);
            } else {
                if (pOffsets[i] != 0) {
                    skip |= LogError(commandBuffer, "VUID-vkCmdBindVertexBuffers-pBuffers-04002",
                                     "vkCmdBindVertexBuffers() pBuffers[%" PRIu32 "] is VK_NULL_HANDLE, but pOffsets[%" PRIu32
                                     "] is not 0",
                                     i, i);
                }
            }
        }
    }

    return skip;
}

bool StatelessValidation::manual_PreCallValidateCmdBindTransformFeedbackBuffersEXT(VkCommandBuffer commandBuffer,
                                                                                   uint32_t firstBinding, uint32_t bindingCount,
                                                                                   const VkBuffer *pBuffers,
                                                                                   const VkDeviceSize *pOffsets,
                                                                                   const VkDeviceSize *pSizes) const {
    bool skip = false;

    char const *const cmd_name = "CmdBindTransformFeedbackBuffersEXT";
    for (uint32_t i = 0; i < bindingCount; ++i) {
        if (pOffsets[i] & 3) {
            skip |= LogError(commandBuffer, "VUID-vkCmdBindTransformFeedbackBuffersEXT-pOffsets-02359",
                             "%s: pOffsets[%" PRIu32 "](0x%" PRIxLEAST64 ") is not a multiple of 4.", cmd_name, i, pOffsets[i]);
        }
    }

    if (firstBinding >= phys_dev_ext_props.transform_feedback_props.maxTransformFeedbackBuffers) {
        skip |= LogError(commandBuffer, "VUID-vkCmdBindTransformFeedbackBuffersEXT-firstBinding-02356",
                         "%s: The firstBinding(%" PRIu32
                         ") index is greater than or equal to "
                         "VkPhysicalDeviceTransformFeedbackPropertiesEXT::maxTransformFeedbackBuffers(%" PRIu32 ").",
                         cmd_name, firstBinding, phys_dev_ext_props.transform_feedback_props.maxTransformFeedbackBuffers);
    }

    if (firstBinding + bindingCount > phys_dev_ext_props.transform_feedback_props.maxTransformFeedbackBuffers) {
        skip |=
            LogError(commandBuffer, "VUID-vkCmdBindTransformFeedbackBuffersEXT-firstBinding-02357",
                     "%s: The sum of firstBinding(%" PRIu32 ") and bindCount(%" PRIu32
                     ") is greater than VkPhysicalDeviceTransformFeedbackPropertiesEXT::maxTransformFeedbackBuffers(%" PRIu32 ").",
                     cmd_name, firstBinding, bindingCount, phys_dev_ext_props.transform_feedback_props.maxTransformFeedbackBuffers);
    }

    for (uint32_t i = 0; i < bindingCount; ++i) {
        // pSizes is optional and may be nullptr.
        if (pSizes != nullptr) {
            if (pSizes[i] != VK_WHOLE_SIZE &&
                pSizes[i] > phys_dev_ext_props.transform_feedback_props.maxTransformFeedbackBufferSize) {
                skip |= LogError(commandBuffer, "VUID-vkCmdBindTransformFeedbackBuffersEXT-pSize-02361",
                                 "%s: pSizes[%" PRIu32 "] (0x%" PRIxLEAST64
                                 ") is not VK_WHOLE_SIZE and is greater than "
                                 "VkPhysicalDeviceTransformFeedbackPropertiesEXT::maxTransformFeedbackBufferSize.",
                                 cmd_name, i, pSizes[i]);
            }
        }
    }

    return skip;
}

bool StatelessValidation::manual_PreCallValidateCmdBeginTransformFeedbackEXT(VkCommandBuffer commandBuffer,
                                                                             uint32_t firstCounterBuffer,
                                                                             uint32_t counterBufferCount,
                                                                             const VkBuffer *pCounterBuffers,
                                                                             const VkDeviceSize *pCounterBufferOffsets) const {
    bool skip = false;

    char const *const cmd_name = "CmdBeginTransformFeedbackEXT";
    if (firstCounterBuffer >= phys_dev_ext_props.transform_feedback_props.maxTransformFeedbackBuffers) {
        skip |= LogError(commandBuffer, "VUID-vkCmdBeginTransformFeedbackEXT-firstCounterBuffer-02368",
                         "%s: The firstCounterBuffer(%" PRIu32
                         ") index is greater than or equal to "
                         "VkPhysicalDeviceTransformFeedbackPropertiesEXT::maxTransformFeedbackBuffers(%" PRIu32 ").",
                         cmd_name, firstCounterBuffer, phys_dev_ext_props.transform_feedback_props.maxTransformFeedbackBuffers);
    }

    if (firstCounterBuffer + counterBufferCount > phys_dev_ext_props.transform_feedback_props.maxTransformFeedbackBuffers) {
        skip |=
            LogError(commandBuffer, "VUID-vkCmdBeginTransformFeedbackEXT-firstCounterBuffer-02369",
                     "%s: The sum of firstCounterBuffer(%" PRIu32 ") and counterBufferCount(%" PRIu32
                     ") is greater than VkPhysicalDeviceTransformFeedbackPropertiesEXT::maxTransformFeedbackBuffers(%" PRIu32 ").",
                     cmd_name, firstCounterBuffer, counterBufferCount,
                     phys_dev_ext_props.transform_feedback_props.maxTransformFeedbackBuffers);
    }

    return skip;
}

bool StatelessValidation::manual_PreCallValidateCmdEndTransformFeedbackEXT(VkCommandBuffer commandBuffer,
                                                                           uint32_t firstCounterBuffer, uint32_t counterBufferCount,
                                                                           const VkBuffer *pCounterBuffers,
                                                                           const VkDeviceSize *pCounterBufferOffsets) const {
    bool skip = false;

    char const *const cmd_name = "CmdEndTransformFeedbackEXT";
    if (firstCounterBuffer >= phys_dev_ext_props.transform_feedback_props.maxTransformFeedbackBuffers) {
        skip |= LogError(commandBuffer, "VUID-vkCmdEndTransformFeedbackEXT-firstCounterBuffer-02376",
                         "%s: The firstCounterBuffer(%" PRIu32
                         ") index is greater than or equal to "
                         "VkPhysicalDeviceTransformFeedbackPropertiesEXT::maxTransformFeedbackBuffers(%" PRIu32 ").",
                         cmd_name, firstCounterBuffer, phys_dev_ext_props.transform_feedback_props.maxTransformFeedbackBuffers);
    }

    if (firstCounterBuffer + counterBufferCount > phys_dev_ext_props.transform_feedback_props.maxTransformFeedbackBuffers) {
        skip |=
            LogError(commandBuffer, "VUID-vkCmdEndTransformFeedbackEXT-firstCounterBuffer-02377",
                     "%s: The sum of firstCounterBuffer(%" PRIu32 ") and counterBufferCount(%" PRIu32
                     ") is greater than VkPhysicalDeviceTransformFeedbackPropertiesEXT::maxTransformFeedbackBuffers(%" PRIu32 ").",
                     cmd_name, firstCounterBuffer, counterBufferCount,
                     phys_dev_ext_props.transform_feedback_props.maxTransformFeedbackBuffers);
    }

    return skip;
}

bool StatelessValidation::manual_PreCallValidateCmdDrawIndirectByteCountEXT(VkCommandBuffer commandBuffer, uint32_t instanceCount,
                                                                            uint32_t firstInstance, VkBuffer counterBuffer,
                                                                            VkDeviceSize counterBufferOffset,
                                                                            uint32_t counterOffset, uint32_t vertexStride) const {
    bool skip = false;

    if ((vertexStride <= 0) || (vertexStride > phys_dev_ext_props.transform_feedback_props.maxTransformFeedbackBufferDataStride)) {
        skip |= LogError(counterBuffer, "VUID-vkCmdDrawIndirectByteCountEXT-vertexStride-02289",
                         "vkCmdDrawIndirectByteCountEXT: vertexStride (%" PRIu32
                         ") must be between 0 and maxTransformFeedbackBufferDataStride (%" PRIu32 ").",
                         vertexStride, phys_dev_ext_props.transform_feedback_props.maxTransformFeedbackBufferDataStride);
    }

    if ((counterOffset % 4) != 0) {
        skip |= LogError(commandBuffer, "VUID-vkCmdDrawIndirectByteCountEXT-counterBufferOffset-04568",
                         "vkCmdDrawIndirectByteCountEXT(): offset (%" PRIu32 ") must be a multiple of 4.", counterOffset);
    }

    return skip;
}

bool StatelessValidation::ValidateCmdBindVertexBuffers2(VkCommandBuffer commandBuffer, uint32_t firstBinding, uint32_t bindingCount,
                                                        const VkBuffer *pBuffers, const VkDeviceSize *pOffsets,
                                                        const VkDeviceSize *pSizes, const VkDeviceSize *pStrides,
                                                        CMD_TYPE cmd_type) const {
    bool skip = false;
    const char *api_call = CommandTypeString(cmd_type);

    // Check VUID-vkCmdBindVertexBuffers2-bindingCount-arraylength
    {
        const bool vuidCondition = (pSizes != nullptr) || (pStrides != nullptr);
        const bool vuidExpectation = bindingCount > 0;
        if (vuidCondition) {
            if (!vuidExpectation) {
                const char *not_null_msg = "";
                if ((pSizes != nullptr) && (pStrides != nullptr))
                    not_null_msg = "pSizes and pStrides are not NULL";
                else if (pSizes != nullptr)
                    not_null_msg = "pSizes is not NULL";
                else
                    not_null_msg = "pStrides is not NULL";
                skip |= LogError(commandBuffer, "VUID-vkCmdBindVertexBuffers2-bindingCount-arraylength",
                                 "%s: %s, so bindingCount must be greater than 0.", api_call, not_null_msg);
            }
        }
    }

    if (firstBinding >= device_limits.maxVertexInputBindings) {
        skip |= LogError(commandBuffer, "VUID-vkCmdBindVertexBuffers2-firstBinding-03355",
                         "%s firstBinding (%" PRIu32 ") must be less than maxVertexInputBindings (%" PRIu32 ")", api_call,
                         firstBinding, device_limits.maxVertexInputBindings);
    } else if ((firstBinding + bindingCount) > device_limits.maxVertexInputBindings) {
        skip |= LogError(commandBuffer, "VUID-vkCmdBindVertexBuffers2-firstBinding-03356",
                         "%s sum of firstBinding (%" PRIu32 ") and bindingCount (%" PRIu32
                         ") must be less than "
                         "maxVertexInputBindings (%" PRIu32 ")",
                         api_call, firstBinding, bindingCount, device_limits.maxVertexInputBindings);
    }

    for (uint32_t i = 0; i < bindingCount; ++i) {
        if (pBuffers[i] == VK_NULL_HANDLE) {
            const auto *robustness2_features = LvlFindInChain<VkPhysicalDeviceRobustness2FeaturesEXT>(device_createinfo_pnext);
            if (!(robustness2_features && robustness2_features->nullDescriptor)) {
                skip |= LogError(commandBuffer, "VUID-vkCmdBindVertexBuffers2-pBuffers-04111",
                                 "%s required parameter pBuffers[%" PRIu32 "] specified as VK_NULL_HANDLE", api_call, i);
            } else {
                if (pOffsets[i] != 0) {
                    skip |=
                        LogError(commandBuffer, "VUID-vkCmdBindVertexBuffers2-pBuffers-04112",
                                 "%s pBuffers[%" PRIu32 "] is VK_NULL_HANDLE, but pOffsets[%" PRIu32 "] is not 0", api_call, i, i);
                }
            }
        }
        if (pStrides) {
            if (pStrides[i] > device_limits.maxVertexInputBindingStride) {
                skip |=
                    LogError(commandBuffer, "VUID-vkCmdBindVertexBuffers2-pStrides-03362",
                             "%s pStrides[%" PRIu32 "] (%" PRIu64 ") must be less than maxVertexInputBindingStride (%" PRIu32 ")",
                             api_call, i, pStrides[i], device_limits.maxVertexInputBindingStride);
            }
        }
    }

    return skip;
}

bool StatelessValidation::manual_PreCallValidateCmdBindVertexBuffers2EXT(VkCommandBuffer commandBuffer, uint32_t firstBinding,
                                                                         uint32_t bindingCount, const VkBuffer *pBuffers,
                                                                         const VkDeviceSize *pOffsets, const VkDeviceSize *pSizes,
                                                                         const VkDeviceSize *pStrides) const {
    bool skip = false;
    skip = ValidateCmdBindVertexBuffers2(commandBuffer, firstBinding, bindingCount, pBuffers, pOffsets, pSizes, pStrides,
                                         CMD_BINDVERTEXBUFFERS2EXT);
    return skip;
}

bool StatelessValidation::manual_PreCallValidateCmdBindVertexBuffers2(VkCommandBuffer commandBuffer, uint32_t firstBinding,
                                                                      uint32_t bindingCount, const VkBuffer *pBuffers,
                                                                      const VkDeviceSize *pOffsets, const VkDeviceSize *pSizes,
                                                                      const VkDeviceSize *pStrides) const {
    bool skip = false;
    skip = ValidateCmdBindVertexBuffers2(commandBuffer, firstBinding, bindingCount, pBuffers, pOffsets, pSizes, pStrides,
                                         CMD_BINDVERTEXBUFFERS2);
    return skip;
}

bool StatelessValidation::manual_PreCallValidateCmdPushConstants(VkCommandBuffer commandBuffer, VkPipelineLayout layout,
                                                                 VkShaderStageFlags stageFlags, uint32_t offset, uint32_t size,
                                                                 const void *pValues) const {
    bool skip = false;
    const uint32_t max_push_constants_size = device_limits.maxPushConstantsSize;
    // Check that offset + size don't exceed the max.
    // Prevent arithetic overflow here by avoiding addition and testing in this order.
    if (offset >= max_push_constants_size) {
        skip |=
            LogError(device, "VUID-vkCmdPushConstants-offset-00370",
                     "vkCmdPushConstants(): offset (%" PRIu32 ") that exceeds this device's maxPushConstantSize of %" PRIu32 ".",
                     offset, max_push_constants_size);
    }
    if (size > max_push_constants_size - offset) {
        skip |= LogError(device, "VUID-vkCmdPushConstants-size-00371",
                         "vkCmdPushConstants(): offset (%" PRIu32 ") and size (%" PRIu32
                         ") that exceeds this device's maxPushConstantSize of %" PRIu32 ".",
                         offset, size, max_push_constants_size);
    }

    // size needs to be non-zero and a multiple of 4.
    if (size & 0x3) {
        skip |= LogError(device, "VUID-vkCmdPushConstants-size-00369",
                         "vkCmdPushConstants(): size (%" PRIu32 ") must be a multiple of 4.", size);
    }

    // offset needs to be a multiple of 4.
    if ((offset & 0x3) != 0) {
        skip |= LogError(device, "VUID-vkCmdPushConstants-offset-00368",
                         "vkCmdPushConstants(): offset (%" PRIu32 ") must be a multiple of 4.", offset);
    }
    return skip;
}

bool StatelessValidation::manual_PreCallValidateCmdClearColorImage(VkCommandBuffer commandBuffer, VkImage image,
                                                                   VkImageLayout imageLayout, const VkClearColorValue *pColor,
                                                                   uint32_t rangeCount,
                                                                   const VkImageSubresourceRange *pRanges) const {
    bool skip = false;
    if (!pColor) {
        skip |=
            LogError(commandBuffer, "VUID-vkCmdClearColorImage-pColor-04961", "vkCmdClearColorImage(): pColor must not be null");
    }
    return skip;
}

bool StatelessValidation::ValidateCmdBeginRenderPass(const VkRenderPassBeginInfo *const rp_begin, CMD_TYPE cmd_type) const {
    bool skip = false;
    if ((rp_begin->clearValueCount != 0) && !rp_begin->pClearValues) {
        skip |= LogError(rp_begin->renderPass, "VUID-VkRenderPassBeginInfo-clearValueCount-04962",
                         "%s: VkRenderPassBeginInfo::clearValueCount != 0 (%" PRIu32
                         "), but VkRenderPassBeginInfo::pClearValues is null.",
                         CommandTypeString(cmd_type), rp_begin->clearValueCount);
    }
    return skip;
}

bool StatelessValidation::manual_PreCallValidateCmdBeginRenderPass(VkCommandBuffer, const VkRenderPassBeginInfo *pRenderPassBegin,
                                                                   VkSubpassContents) const {
    bool skip = ValidateCmdBeginRenderPass(pRenderPassBegin, CMD_BEGINRENDERPASS);
    return skip;
}

bool StatelessValidation::manual_PreCallValidateCmdBeginRenderPass2KHR(VkCommandBuffer,
                                                                       const VkRenderPassBeginInfo *pRenderPassBegin,
                                                                       const VkSubpassBeginInfo *) const {
    bool skip = ValidateCmdBeginRenderPass(pRenderPassBegin, CMD_BEGINRENDERPASS2KHR);
    return skip;
}

bool StatelessValidation::manual_PreCallValidateCmdBeginRenderPass2(VkCommandBuffer, const VkRenderPassBeginInfo *pRenderPassBegin,
                                                                    const VkSubpassBeginInfo *) const {
    bool skip = ValidateCmdBeginRenderPass(pRenderPassBegin, CMD_BEGINRENDERPASS2);
    return skip;
}

static bool UniqueRenderingInfoImageViews(const VkRenderingInfo *pRenderingInfo, VkImageView imageView) {
    bool unique_views = true;
    for (uint32_t i = 0; i < pRenderingInfo->colorAttachmentCount; ++i) {
        if (pRenderingInfo->pColorAttachments[i].imageView == imageView) {
            unique_views = false;
        }

        if (pRenderingInfo->pColorAttachments[i].resolveImageView == imageView) {
            unique_views = false;
        }
    }

    if (pRenderingInfo->pDepthAttachment) {
        if (pRenderingInfo->pDepthAttachment->imageView == imageView) {
            unique_views = false;
        }

        if (pRenderingInfo->pDepthAttachment->resolveImageView == imageView) {
            unique_views = false;
        }
    }

    if (pRenderingInfo->pStencilAttachment) {
        if (pRenderingInfo->pStencilAttachment->imageView == imageView) {
            unique_views = false;
        }

        if (pRenderingInfo->pStencilAttachment->resolveImageView == imageView) {
            unique_views = false;
        }
    }
    return unique_views;
}

bool StatelessValidation::ValidateCmdBeginRendering(VkCommandBuffer commandBuffer, const VkRenderingInfo *pRenderingInfo,
                                                    CMD_TYPE cmd_type) const {
    bool skip = false;
    const char *func_name = CommandTypeString(cmd_type);

    if (pRenderingInfo->viewMask == 0 && pRenderingInfo->layerCount == 0) {
        skip |= LogError(commandBuffer, "VUID-VkRenderingInfo-viewMask-06069",
                         "%s(): If viewMask is 0 (%" PRIu32 "), layerCount must not be 0 (%" PRIu32 ").", func_name,
                         pRenderingInfo->viewMask, pRenderingInfo->layerCount);
    }

    if (pRenderingInfo->colorAttachmentCount > device_limits.maxColorAttachments) {
        skip |= LogError(commandBuffer, "VUID-VkRenderingInfo-colorAttachmentCount-06106",
                         "%s(): colorAttachmentCount (%u) must be less than or equal to "
                         "VkPhysicalDeviceLimits::maxColorAttachments (%u).",
                         func_name, pRenderingInfo->colorAttachmentCount, device_limits.maxColorAttachments);
    }

    const auto rendering_fragment_shading_rate_attachment_info =
        LvlFindInChain<VkRenderingFragmentShadingRateAttachmentInfoKHR>(pRenderingInfo->pNext);
    if (rendering_fragment_shading_rate_attachment_info &&
        (rendering_fragment_shading_rate_attachment_info->imageView != VK_NULL_HANDLE)) {
        if (UniqueRenderingInfoImageViews(pRenderingInfo, rendering_fragment_shading_rate_attachment_info->imageView) == false) {
            skip |= LogError(commandBuffer, "VUID-VkRenderingInfo-imageView-06125",
                             "%s(): imageView or resolveImageView member of pDepthAttachment, pStencilAttachment, or any element "
                             "of pColorAttachments must not equal VkRenderingFragmentShadingRateAttachmentInfoKHR->imageView.",
                             func_name);
        }

        const VkImageLayout image_layout = rendering_fragment_shading_rate_attachment_info->imageLayout;
        if (image_layout != VK_IMAGE_LAYOUT_GENERAL &&
            image_layout != VK_IMAGE_LAYOUT_FRAGMENT_SHADING_RATE_ATTACHMENT_OPTIMAL_KHR) {
            skip |= LogError(commandBuffer, "VUID-VkRenderingFragmentShadingRateAttachmentInfoKHR-imageView-06147",
                             "%s(): VkRenderingFragmentShadingRateAttachmentInfoKHR->layout (%s) must be VK_IMAGE_LAYOUT_GENERAL "
                             "or VK_IMAGE_LAYOUT_FRAGMENT_SHADING_RATE_ATTACHMENT_OPTIMAL_KHR.",
                             func_name, string_VkImageLayout(image_layout));
        }

        if (!IsPowerOfTwo(rendering_fragment_shading_rate_attachment_info->shadingRateAttachmentTexelSize.width)) {
            skip |= LogError(commandBuffer, "VUID-VkRenderingFragmentShadingRateAttachmentInfoKHR-imageView-06149",
                             "%s(): shadingRateAttachmentTexelSize.width (%u) must be a power of two.", func_name,
                             rendering_fragment_shading_rate_attachment_info->shadingRateAttachmentTexelSize.width);
        }

        const uint32_t max_frs_attach_texel_width =
            phys_dev_ext_props.fragment_shading_rate_props.maxFragmentShadingRateAttachmentTexelSize.width;
        if (rendering_fragment_shading_rate_attachment_info->shadingRateAttachmentTexelSize.width > max_frs_attach_texel_width) {
            skip |= LogError(commandBuffer, "VUID-VkRenderingFragmentShadingRateAttachmentInfoKHR-imageView-06150",
                             "%s(): shadingRateAttachmentTexelSize.width (%u) must be less than or equal to "
                             "maxFragmentShadingRateAttachmentTexelSize.width (%u).",
                             func_name, rendering_fragment_shading_rate_attachment_info->shadingRateAttachmentTexelSize.width,
                             max_frs_attach_texel_width);
        }

        const uint32_t min_frs_attach_texel_width =
            phys_dev_ext_props.fragment_shading_rate_props.minFragmentShadingRateAttachmentTexelSize.width;
        if (rendering_fragment_shading_rate_attachment_info->shadingRateAttachmentTexelSize.width < min_frs_attach_texel_width) {
            skip |= LogError(commandBuffer, "VUID-VkRenderingFragmentShadingRateAttachmentInfoKHR-imageView-06151",
                             "%s(): shadingRateAttachmentTexelSize.width (%u) must be greater than or equal to "
                             "minFragmentShadingRateAttachmentTexelSize.width (%u).",
                             func_name, rendering_fragment_shading_rate_attachment_info->shadingRateAttachmentTexelSize.width,
                             min_frs_attach_texel_width);
        }

        if (!IsPowerOfTwo(rendering_fragment_shading_rate_attachment_info->shadingRateAttachmentTexelSize.height)) {
            skip |= LogError(commandBuffer, "VUID-VkRenderingFragmentShadingRateAttachmentInfoKHR-imageView-06152",
                             "%s(): shadingRateAttachmentTexelSize.height (%u) must be a power of two.", func_name,
                             rendering_fragment_shading_rate_attachment_info->shadingRateAttachmentTexelSize.height);
        }

        const uint32_t max_frs_attach_texel_height =
            phys_dev_ext_props.fragment_shading_rate_props.maxFragmentShadingRateAttachmentTexelSize.height;
        if (rendering_fragment_shading_rate_attachment_info->shadingRateAttachmentTexelSize.height > max_frs_attach_texel_height) {
            skip |= LogError(commandBuffer, "VUID-VkRenderingFragmentShadingRateAttachmentInfoKHR-imageView-06153",
                             "%s(): shadingRateAttachmentTexelSize.height (%u) must be less than or equal to "
                             "maxFragmentShadingRateAttachmentTexelSize.height (%u).",
                             func_name, rendering_fragment_shading_rate_attachment_info->shadingRateAttachmentTexelSize.height,
                             max_frs_attach_texel_height);
        }

        const uint32_t min_frs_attach_texel_height =
            phys_dev_ext_props.fragment_shading_rate_props.minFragmentShadingRateAttachmentTexelSize.height;
        if (rendering_fragment_shading_rate_attachment_info->shadingRateAttachmentTexelSize.height < min_frs_attach_texel_height) {
            skip |= LogError(commandBuffer, "VUID-VkRenderingFragmentShadingRateAttachmentInfoKHR-imageView-06154",
                             "%s(): shadingRateAttachmentTexelSize.height (%u) must be greater than or equal to "
                             "minFragmentShadingRateAttachmentTexelSize.height (%u).",
                             func_name, rendering_fragment_shading_rate_attachment_info->shadingRateAttachmentTexelSize.height,
                             min_frs_attach_texel_height);
        }

        const uint32_t max_frs_attach_texel_aspect_ratio =
            phys_dev_ext_props.fragment_shading_rate_props.maxFragmentShadingRateAttachmentTexelSizeAspectRatio;
        if ((rendering_fragment_shading_rate_attachment_info->shadingRateAttachmentTexelSize.width /
             rendering_fragment_shading_rate_attachment_info->shadingRateAttachmentTexelSize.height) >
            max_frs_attach_texel_aspect_ratio) {
            skip |= LogError(
                commandBuffer, "VUID-VkRenderingFragmentShadingRateAttachmentInfoKHR-imageView-06155",
                "%s(): the quotient of shadingRateAttachmentTexelSize.width (%u) and shadingRateAttachmentTexelSize.height (%u) "
                "must be less than or equal to maxFragmentShadingRateAttachmentTexelSizeAspectRatio (%u).",
                func_name, rendering_fragment_shading_rate_attachment_info->shadingRateAttachmentTexelSize.width,
                rendering_fragment_shading_rate_attachment_info->shadingRateAttachmentTexelSize.height,
                max_frs_attach_texel_aspect_ratio);
        }

        if ((rendering_fragment_shading_rate_attachment_info->shadingRateAttachmentTexelSize.height /
             rendering_fragment_shading_rate_attachment_info->shadingRateAttachmentTexelSize.width) >
            max_frs_attach_texel_aspect_ratio) {
            skip |= LogError(
                commandBuffer, "VUID-VkRenderingFragmentShadingRateAttachmentInfoKHR-imageView-06156",
                "%s(): the quotient of shadingRateAttachmentTexelSize.height (%u) and shadingRateAttachmentTexelSize.width (%u) "
                "must be less than or equal to maxFragmentShadingRateAttachmentTexelSizeAspectRatio (%u).",
                func_name, rendering_fragment_shading_rate_attachment_info->shadingRateAttachmentTexelSize.height,
                rendering_fragment_shading_rate_attachment_info->shadingRateAttachmentTexelSize.width,
                max_frs_attach_texel_aspect_ratio);
        }
    }

    const auto fragment_density_map_attachment_info =
        LvlFindInChain<VkRenderingFragmentDensityMapAttachmentInfoEXT>(pRenderingInfo->pNext);
    if (fragment_density_map_attachment_info && (fragment_density_map_attachment_info->imageView != VK_NULL_HANDLE)) {
        if (UniqueRenderingInfoImageViews(pRenderingInfo, fragment_density_map_attachment_info->imageView) == false) {
            skip |=
                LogError(commandBuffer, "VUID-VkRenderingInfo-imageView-06116",
                         "%s(): imageView or resolveImageView member of pDepthAttachment, pStencilAttachment, or any"
                         "element of pColorAttachments must not equal VkRenderingFragmentDensityMapAttachmentInfoEXT->imageView.",
                         func_name);
        }

        if (fragment_density_map_attachment_info->imageLayout != VK_IMAGE_LAYOUT_GENERAL &&
            fragment_density_map_attachment_info->imageLayout != VK_IMAGE_LAYOUT_FRAGMENT_DENSITY_MAP_OPTIMAL_EXT) {
            skip |= LogError(commandBuffer, "VUID-VkRenderingFragmentDensityMapAttachmentInfoEXT-imageView-06157",
                             "%s(): VkRenderingFragmentDensityMapAttachmentInfoEXT::imageView is not VK_NULL_HANDLE, but "
                             "VkRenderingFragmentDensityMapAttachmentInfoEXT::imageLayout is %s.",
                             func_name, string_VkImageLayout(fragment_density_map_attachment_info->imageLayout));
        }

        if (rendering_fragment_shading_rate_attachment_info &&
            (rendering_fragment_shading_rate_attachment_info->imageView == fragment_density_map_attachment_info->imageView)) {
            skip |= LogError(commandBuffer, "VUID-VkRenderingInfo-imageView-06126",
                             "%s(): VkRenderingFragmentDensityMapAttachmentInfoEXT::imageView and "
                             "VkRenderingFragmentShadingRateAttachmentInfoKHR::imageView are the same.",
                             func_name);
        }
    }

    for (uint32_t j = 0; j < pRenderingInfo->colorAttachmentCount; ++j) {
        if (pRenderingInfo->pColorAttachments[j].imageView != VK_NULL_HANDLE) {
            const VkImageLayout image_layout = pRenderingInfo->pColorAttachments[j].imageLayout;
            const VkResolveModeFlagBits resolve_mode = pRenderingInfo->pColorAttachments[j].resolveMode;
            const VkImageLayout resolve_image_layout = pRenderingInfo->pColorAttachments[j].resolveImageLayout;
            if (image_layout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL ||
                image_layout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL) {
                skip |= LogError(commandBuffer, "VUID-VkRenderingInfo-colorAttachmentCount-06090",
                                 "%s(): imageLayout must not be "
                                 "VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL or "
                                 "VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL.",
                                 func_name);
            }

            if (resolve_mode != VK_RESOLVE_MODE_NONE) {
                if (resolve_image_layout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL ||
                    resolve_image_layout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL) {
                    skip |= LogError(commandBuffer, "VUID-VkRenderingInfo-colorAttachmentCount-06091",
                                     "%s(): resolveImageLayout must not be "
                                     "VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL or "
                                     "VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL.",
                                     func_name);
                }
            }

            if (IsExtEnabled(device_extensions.vk_khr_maintenance2)) {
                if (image_layout == VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_STENCIL_ATTACHMENT_OPTIMAL ||
                    image_layout == VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_STENCIL_READ_ONLY_OPTIMAL) {
                    skip |= LogError(commandBuffer, "VUID-VkRenderingInfo-colorAttachmentCount-06096",
                                     "%s(): imageLayout must not be "
                                     "VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_STENCIL_ATTACHMENT_OPTIMAL "
                                     "or VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_STENCIL_READ_ONLY_OPTIMAL.",
                                     func_name);
                }

                if (resolve_mode != VK_RESOLVE_MODE_NONE) {
                    if (resolve_image_layout == VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_STENCIL_ATTACHMENT_OPTIMAL ||
                        resolve_image_layout == VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_STENCIL_READ_ONLY_OPTIMAL) {
                        skip |= LogError(commandBuffer, "VUID-VkRenderingInfo-colorAttachmentCount-06097",
                                         "%s(): resolveImageLayout must not be "
                                         "VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_STENCIL_ATTACHMENT_OPTIMAL or "
                                         "VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_STENCIL_READ_ONLY_OPTIMAL.",
                                         func_name);
                    }
                }
            }

            if (IsImageLayoutDepthOnly(image_layout) || IsImageLayoutStencilOnly(image_layout)) {
                skip |= LogError(commandBuffer, "VUID-VkRenderingInfo-colorAttachmentCount-06100",
                                 "%s(): imageLayout must not be VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL"
                                 " or VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_STENCIL_READ_ONLY_OPTIMAL"
                                 " or VK_IMAGE_LAYOUT_STENCIL_ATTACHMENT_OPTIMAL"
                                 " or VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_STENCIL_READ_ONLY_OPTIMAL.",
                                 func_name);
            }

            if (resolve_mode != VK_RESOLVE_MODE_NONE) {
                if (resolve_image_layout == VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL ||
                    resolve_image_layout == VK_IMAGE_LAYOUT_STENCIL_ATTACHMENT_OPTIMAL) {
                    skip |= LogError(commandBuffer, "VUID-VkRenderingInfo-colorAttachmentCount-06101",
                                     "%s(): resolveImageLayout must not be VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL or "
                                     "VK_IMAGE_LAYOUT_STENCIL_ATTACHMENT_OPTIMAL.",
                                     func_name);
                }
            }
        }
    }

    if (pRenderingInfo->pDepthAttachment && pRenderingInfo->pDepthAttachment->imageView != VK_NULL_HANDLE) {
        const VkImageLayout layout = pRenderingInfo->pDepthAttachment->imageLayout;
        if (layout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL) {
            skip |=
                LogError(commandBuffer, "VUID-VkRenderingInfo-pDepthAttachment-06092",
                         "%s(): pDepthAttachment->imageLayout is can't be VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL.", func_name);
        } else if (IsImageLayoutStencilOnly(layout)) {
            skip |= LogError(commandBuffer, "VUID-VkRenderingInfo-pDepthAttachment-07732",
                             "%s(): pDepthAttachment->imageLayout is can't be VK_IMAGE_LAYOUT_STENCIL_ATTACHMENT_OPTIMAL or "
                             "VK_IMAGE_LAYOUT_STENCIL_READ_ONLY_OPTIMAL.",
                             func_name);
        }

        if (pRenderingInfo->pDepthAttachment->resolveMode != VK_RESOLVE_MODE_NONE) {
            const VkImageLayout resolve_layout = pRenderingInfo->pDepthAttachment->resolveImageLayout;
            if (resolve_layout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL) {
                skip |= LogError(commandBuffer, "VUID-VkRenderingInfo-pDepthAttachment-06093",
                                 "%s(): pDepthAttachment->resolveImageLayout must not be VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL.",
                                 func_name);
            } else if (IsImageLayoutStencilOnly(resolve_layout)) {
                skip |= LogError(commandBuffer, "VUID-VkRenderingInfo-pDepthAttachment-07733",
                                 "%s(): pDepthAttachment->resolveImageLayout must not be "
                                 "VK_IMAGE_LAYOUT_STENCIL_ATTACHMENT_OPTIMAL or VK_IMAGE_LAYOUT_STENCIL_READ_ONLY_OPTIMAL.",
                                 func_name);
            }

            if (IsExtEnabled(device_extensions.vk_khr_maintenance2) &&
                resolve_layout == VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_STENCIL_ATTACHMENT_OPTIMAL) {
                skip |= LogError(commandBuffer, "VUID-VkRenderingInfo-pDepthAttachment-06098",
                                 "%s(): resolveImageLayout must not be "
                                 "VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_STENCIL_ATTACHMENT_OPTIMAL.",
                                 func_name);
            }

            if (!(pRenderingInfo->pDepthAttachment->resolveMode &
                  phys_dev_ext_props.depth_stencil_resolve_props.supportedDepthResolveModes)) {
                skip |= LogError(device, "VUID-VkRenderingInfo-pDepthAttachment-06102",
                                 "%s(): Includes a resolveMode structure with invalid mode=%u.", func_name,
                                 pRenderingInfo->pDepthAttachment->resolveMode);
            }
        }
    }

    if (pRenderingInfo->pStencilAttachment != nullptr && pRenderingInfo->pStencilAttachment->imageView != VK_NULL_HANDLE) {
        const VkImageLayout layout = pRenderingInfo->pStencilAttachment->imageLayout;
        if (layout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL) {
            skip |=
                LogError(commandBuffer, "VUID-VkRenderingInfo-pStencilAttachment-06094",
                         "%s(): pStencilAttachment->imageLayout is can't be VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL.", func_name);
        } else if (IsImageLayoutDepthOnly(layout)) {
            skip |= LogError(commandBuffer, "VUID-VkRenderingInfo-pStencilAttachment-07734",
                             "%s(): pStencilAttachment->imageLayout is can't be VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL or "
                             "VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_OPTIMAL.",
                             func_name);
        }

        if (pRenderingInfo->pStencilAttachment->resolveMode != VK_RESOLVE_MODE_NONE) {
            const VkImageLayout resolve_layout = pRenderingInfo->pStencilAttachment->resolveImageLayout;
            if (resolve_layout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL) {
                skip |=
                    LogError(commandBuffer, "VUID-VkRenderingInfo-pStencilAttachment-06095",
                             "%s(): pStencilAttachment->resolveImageLayout must not be VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL.",
                             func_name);
            } else if (IsImageLayoutDepthOnly(resolve_layout)) {
                skip |=
                    LogError(commandBuffer, "VUID-VkRenderingInfo-pStencilAttachment-07735",
                             "%s(): pStencilAttachment->resolveImageLayout must not be VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL.",
                             func_name);
            }

            if (IsExtEnabled(device_extensions.vk_khr_maintenance2) &&
                resolve_layout == VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_STENCIL_READ_ONLY_OPTIMAL) {
                skip |= LogError(commandBuffer, "VUID-VkRenderingInfo-pStencilAttachment-06099",
                                 "%s(): resolveImageLayout must not be "
                                 "VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_STENCIL_READ_ONLY_OPTIMAL.",
                                 func_name);
            }

            if (!(pRenderingInfo->pStencilAttachment->resolveMode &
                  phys_dev_ext_props.depth_stencil_resolve_props.supportedStencilResolveModes)) {
                skip |= LogError(device, "VUID-VkRenderingInfo-pStencilAttachment-06103",
                                 "%s(): Includes a resolveMode structure with invalid mode (%s).", func_name,
                                 string_VkResolveModeFlagBits(pRenderingInfo->pStencilAttachment->resolveMode));
            }
        }
    }

    return skip;
}

bool StatelessValidation::manual_PreCallValidateCmdBeginRenderingKHR(VkCommandBuffer commandBuffer,
                                                                     const VkRenderingInfo *pRenderingInfo) const {
    bool skip = ValidateCmdBeginRendering(commandBuffer, pRenderingInfo, CMD_BEGINRENDERINGKHR);
    return skip;
}

bool StatelessValidation::manual_PreCallValidateCmdBeginRendering(VkCommandBuffer commandBuffer,
                                                                  const VkRenderingInfo *pRenderingInfo) const {
    bool skip = ValidateCmdBeginRendering(commandBuffer, pRenderingInfo, CMD_BEGINRENDERING);
    return skip;
}

bool StatelessValidation::manual_PreCallValidateGetQueryPoolResults(VkDevice device, VkQueryPool queryPool, uint32_t firstQuery,
                                                                    uint32_t queryCount, size_t dataSize, void *pData,
                                                                    VkDeviceSize stride, VkQueryResultFlags flags) const {
    bool skip = false;

    if ((flags & VK_QUERY_RESULT_WITH_STATUS_BIT_KHR) && (flags & VK_QUERY_RESULT_WITH_AVAILABILITY_BIT)) {
        skip |= LogError(device, "VUID-vkGetQueryPoolResults-flags-04811",
                         "vkGetQueryPoolResults(): flags include both VK_QUERY_RESULT_WITH_STATUS_BIT_KHR bit and "
                         "VK_QUERY_RESULT_WITH_AVAILABILITY_BIT bit.");
    }

    return skip;
}

bool StatelessValidation::manual_PreCallValidateCmdBeginConditionalRenderingEXT(
    VkCommandBuffer commandBuffer, const VkConditionalRenderingBeginInfoEXT *pConditionalRenderingBegin) const {
    bool skip = false;

    if ((pConditionalRenderingBegin->offset & 3) != 0) {
        skip |= LogError(commandBuffer, "VUID-VkConditionalRenderingBeginInfoEXT-offset-01984",
                         "vkCmdBeginConditionalRenderingEXT(): pConditionalRenderingBegin->offset (%" PRIu64
                         ") is not a multiple of 4.",
                         pConditionalRenderingBegin->offset);
    }

    return skip;
}

bool StatelessValidation::manual_PreCallValidateCmdDrawIndirect(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                                uint32_t drawCount, uint32_t stride) const {
    bool skip = false;

    if (!physical_device_features.multiDrawIndirect && ((drawCount > 1))) {
        skip |= LogError(device, "VUID-vkCmdDrawIndirect-drawCount-02718",
                         "vkCmdDrawIndirect(): Device feature multiDrawIndirect disabled: count must be 0 or 1 but is %" PRIu32 "",
                         drawCount);
    }
    if (drawCount > device_limits.maxDrawIndirectCount) {
        skip |=
            LogError(commandBuffer, "VUID-vkCmdDrawIndirect-drawCount-02719",
                     "vkCmdDrawIndirect(): drawCount (%" PRIu32 ") is not less than or equal to the maximum allowed (%" PRIu32 ").",
                     drawCount, device_limits.maxDrawIndirectCount);
    }
    if (offset & 3) {
        skip |= LogError(commandBuffer, "VUID-vkCmdDrawIndirect-offset-02710",
                         "vkCmdDrawIndirect(): offset (%" PRIxLEAST64 ") must be a multiple of 4.", offset);
    }
    return skip;
}

bool StatelessValidation::manual_PreCallValidateCmdDrawIndexedIndirect(VkCommandBuffer commandBuffer, VkBuffer buffer,
                                                                       VkDeviceSize offset, uint32_t drawCount,
                                                                       uint32_t stride) const {
    bool skip = false;
    if (!physical_device_features.multiDrawIndirect && ((drawCount > 1))) {
        skip |= LogError(
            device, "VUID-vkCmdDrawIndexedIndirect-drawCount-02718",
            "vkCmdDrawIndexedIndirect(): Device feature multiDrawIndirect disabled: count must be 0 or 1 but is %" PRIu32 "",
            drawCount);
    }
    if (drawCount > device_limits.maxDrawIndirectCount) {
        skip |= LogError(commandBuffer, "VUID-vkCmdDrawIndexedIndirect-drawCount-02719",
                         "vkCmdDrawIndexedIndirect(): drawCount (%" PRIu32
                         ") is not less than or equal to the maximum allowed (%" PRIu32 ").",
                         drawCount, device_limits.maxDrawIndirectCount);
    }
    if (offset & 3) {
        skip |= LogError(commandBuffer, "VUID-vkCmdDrawIndexedIndirect-offset-02710",
                         "vkCmdDrawIndexedIndirect(): offset (%" PRIxLEAST64 ") must be a multiple of 4.", offset);
    }
    return skip;
}

bool StatelessValidation::ValidateCmdDrawIndirectCount(VkCommandBuffer commandBuffer, VkDeviceSize offset,
                                                       VkDeviceSize countBufferOffset, CMD_TYPE cmd_type) const {
    bool skip = false;
    if (offset & 3) {
        skip |= LogError(commandBuffer, "VUID-vkCmdDrawIndirectCount-offset-02710",
                         "%s: parameter, VkDeviceSize offset (0x%" PRIxLEAST64 "), is not a multiple of 4.",
                         CommandTypeString(cmd_type), offset);
    }

    if (countBufferOffset & 3) {
        skip |= LogError(commandBuffer, "VUID-vkCmdDrawIndirectCount-countBufferOffset-02716",
                         "%s: parameter, VkDeviceSize countBufferOffset (0x%" PRIxLEAST64 "), is not a multiple of 4.",
                         CommandTypeString(cmd_type), countBufferOffset);
    }
    return skip;
}

bool StatelessValidation::manual_PreCallValidateCmdDrawIndirectCount(VkCommandBuffer commandBuffer, VkBuffer buffer,
                                                                     VkDeviceSize offset, VkBuffer countBuffer,
                                                                     VkDeviceSize countBufferOffset, uint32_t maxDrawCount,
                                                                     uint32_t stride) const {
    return ValidateCmdDrawIndirectCount(commandBuffer, offset, countBufferOffset, CMD_DRAWINDIRECTCOUNT);
}

bool StatelessValidation::manual_PreCallValidateCmdDrawIndirectCountAMD(VkCommandBuffer commandBuffer, VkBuffer buffer,
                                                                        VkDeviceSize offset, VkBuffer countBuffer,
                                                                        VkDeviceSize countBufferOffset, uint32_t maxDrawCount,
                                                                        uint32_t stride) const {
    return ValidateCmdDrawIndirectCount(commandBuffer, offset, countBufferOffset, CMD_DRAWINDIRECTCOUNTAMD);
}

bool StatelessValidation::manual_PreCallValidateCmdDrawIndirectCountKHR(VkCommandBuffer commandBuffer, VkBuffer buffer,
                                                                        VkDeviceSize offset, VkBuffer countBuffer,
                                                                        VkDeviceSize countBufferOffset, uint32_t maxDrawCount,
                                                                        uint32_t stride) const {
    return ValidateCmdDrawIndirectCount(commandBuffer, offset, countBufferOffset, CMD_DRAWINDIRECTCOUNTKHR);
}

bool StatelessValidation::ValidateCmdDrawIndexedIndirectCount(VkCommandBuffer commandBuffer, VkDeviceSize offset,
                                                              VkDeviceSize countBufferOffset, CMD_TYPE cmd_type) const {
    bool skip = false;
    if (offset & 3) {
        skip |= LogError(commandBuffer, "VUID-vkCmdDrawIndexedIndirectCount-offset-02710",
                         "%s: parameter, VkDeviceSize offset (0x%" PRIxLEAST64 "), is not a multiple of 4.",
                         CommandTypeString(cmd_type), offset);
    }

    if (countBufferOffset & 3) {
        skip |= LogError(commandBuffer, "VUID-vkCmdDrawIndexedIndirectCount-countBufferOffset-02716",
                         "%s: parameter, VkDeviceSize countBufferOffset (0x%" PRIxLEAST64 "), is not a multiple of 4.",
                         CommandTypeString(cmd_type), countBufferOffset);
    }
    return skip;
}

bool StatelessValidation::manual_PreCallValidateCmdDrawIndexedIndirectCount(VkCommandBuffer commandBuffer, VkBuffer buffer,
                                                                            VkDeviceSize offset, VkBuffer countBuffer,
                                                                            VkDeviceSize countBufferOffset, uint32_t maxDrawCount,
                                                                            uint32_t stride) const {
    return ValidateCmdDrawIndexedIndirectCount(commandBuffer, offset, countBufferOffset, CMD_DRAWINDEXEDINDIRECTCOUNT);
}

bool StatelessValidation::manual_PreCallValidateCmdDrawIndexedIndirectCountAMD(VkCommandBuffer commandBuffer, VkBuffer buffer,
                                                                               VkDeviceSize offset, VkBuffer countBuffer,
                                                                               VkDeviceSize countBufferOffset,
                                                                               uint32_t maxDrawCount, uint32_t stride) const {
    return ValidateCmdDrawIndexedIndirectCount(commandBuffer, offset, countBufferOffset, CMD_DRAWINDEXEDINDIRECTCOUNTAMD);
}

bool StatelessValidation::manual_PreCallValidateCmdDrawIndexedIndirectCountKHR(VkCommandBuffer commandBuffer, VkBuffer buffer,
                                                                               VkDeviceSize offset, VkBuffer countBuffer,
                                                                               VkDeviceSize countBufferOffset,
                                                                               uint32_t maxDrawCount, uint32_t stride) const {
    return ValidateCmdDrawIndexedIndirectCount(commandBuffer, offset, countBufferOffset, CMD_DRAWINDEXEDINDIRECTCOUNTKHR);
}

bool StatelessValidation::manual_PreCallValidateCmdDrawMultiEXT(VkCommandBuffer commandBuffer, uint32_t drawCount,
                                                                const VkMultiDrawInfoEXT *pVertexInfo, uint32_t instanceCount,
                                                                uint32_t firstInstance, uint32_t stride) const {
    bool skip = false;
    if (stride & 3) {
        skip |= LogError(commandBuffer, "VUID-vkCmdDrawMultiEXT-stride-04936",
                         "CmdDrawMultiEXT: parameter, uint32_t stride (%" PRIu32 ") is not a multiple of 4.", stride);
    }
    if (drawCount && nullptr == pVertexInfo) {
        skip |= LogError(commandBuffer, "VUID-vkCmdDrawMultiEXT-drawCount-04935",
                         "CmdDrawMultiEXT: parameter, VkMultiDrawInfoEXT *pVertexInfo must be a valid pointer to memory containing "
                         "one or more valid instances of VkMultiDrawInfoEXT structures");
    }
    return skip;
}

bool StatelessValidation::manual_PreCallValidateCmdDrawMultiIndexedEXT(VkCommandBuffer commandBuffer, uint32_t drawCount,
                                                                       const VkMultiDrawIndexedInfoEXT *pIndexInfo,
                                                                       uint32_t instanceCount, uint32_t firstInstance,
                                                                       uint32_t stride, const int32_t *pVertexOffset) const {
    bool skip = false;
    if (stride & 3) {
        skip |= LogError(commandBuffer, "VUID-vkCmdDrawMultiIndexedEXT-stride-04941",
                         "CmdDrawMultiIndexedEXT: parameter, uint32_t stride (%" PRIu32 ") is not a multiple of 4.", stride);
    }
    if (drawCount && nullptr == pIndexInfo) {
        skip |= LogError(commandBuffer, "VUID-vkCmdDrawMultiIndexedEXT-drawCount-04940",
                         "CmdDrawMultiIndexedEXT: parameter, VkMultiDrawIndexedInfoEXT *pIndexInfo must be a valid pointer to "
                         "memory containing one or more valid instances of VkMultiDrawIndexedInfoEXT structures");
    }
    return skip;
}

bool StatelessValidation::manual_PreCallValidateCmdClearAttachments(VkCommandBuffer commandBuffer, uint32_t attachmentCount,
                                                                    const VkClearAttachment *pAttachments, uint32_t rectCount,
                                                                    const VkClearRect *pRects) const {
    bool skip = false;
    for (uint32_t rect = 0; rect < rectCount; rect++) {
        if (pRects[rect].layerCount == 0) {
            skip |= LogError(commandBuffer, "VUID-vkCmdClearAttachments-layerCount-01934",
                             "CmdClearAttachments(): pRects[%" PRIu32 "].layerCount is zero.", rect);
        }
        if (pRects[rect].rect.extent.width == 0) {
            skip |= LogError(commandBuffer, "VUID-vkCmdClearAttachments-rect-02682",
                             "CmdClearAttachments(): pRects[%" PRIu32 "].rect.extent.width is zero.", rect);
        }
        if (pRects[rect].rect.extent.height == 0) {
            skip |= LogError(commandBuffer, "VUID-vkCmdClearAttachments-rect-02683",
                             "CmdClearAttachments(): pRects[%" PRIu32 "].rect.extent.height is zero.", rect);
        }
    }
    return skip;
}

bool StatelessValidation::manual_PreCallValidateCmdCopyBuffer(VkCommandBuffer commandBuffer, VkBuffer srcBuffer, VkBuffer dstBuffer,
                                                              uint32_t regionCount, const VkBufferCopy *pRegions) const {
    bool skip = false;

    if (pRegions != nullptr) {
        for (uint32_t i = 0; i < regionCount; i++) {
            if (pRegions[i].size == 0) {
                skip |= LogError(device, "VUID-VkBufferCopy-size-01988",
                                 "vkCmdCopyBuffer() pRegions[%" PRIu32 "].size must be greater than zero", i);
            }
        }
    }
    return skip;
}

bool StatelessValidation::manual_PreCallValidateCmdCopyBuffer2KHR(VkCommandBuffer commandBuffer,
                                                                  const VkCopyBufferInfo2KHR *pCopyBufferInfo) const {
    bool skip = false;

    if (pCopyBufferInfo->pRegions != nullptr) {
        for (uint32_t i = 0; i < pCopyBufferInfo->regionCount; i++) {
            if (pCopyBufferInfo->pRegions[i].size == 0) {
                skip |= LogError(device, "VUID-VkBufferCopy2-size-01988",
                                 "vkCmdCopyBuffer2KHR() pCopyBufferInfo->pRegions[%" PRIu32 "].size must be greater than zero", i);
            }
        }
    }
    return skip;
}

bool StatelessValidation::manual_PreCallValidateCmdCopyBuffer2(VkCommandBuffer commandBuffer,
                                                               const VkCopyBufferInfo2 *pCopyBufferInfo) const {
    bool skip = false;

    if (pCopyBufferInfo->pRegions != nullptr) {
        for (uint32_t i = 0; i < pCopyBufferInfo->regionCount; i++) {
            if (pCopyBufferInfo->pRegions[i].size == 0) {
                skip |= LogError(device, "VUID-VkBufferCopy2-size-01988",
                                 "vkCmdCopyBuffer2() pCopyBufferInfo->pRegions[%" PRIu32 "].size must be greater than zero", i);
            }
        }
    }
    return skip;
}

bool StatelessValidation::manual_PreCallValidateCmdUpdateBuffer(VkCommandBuffer commandBuffer, VkBuffer dstBuffer,
                                                                VkDeviceSize dstOffset, VkDeviceSize dataSize,
                                                                const void *pData) const {
    bool skip = false;

    if (dstOffset & 3) {
        skip |= LogError(device, "VUID-vkCmdUpdateBuffer-dstOffset-00036",
                         "vkCmdUpdateBuffer() parameter, VkDeviceSize dstOffset (0x%" PRIxLEAST64 "), is not a multiple of 4.",
                         dstOffset);
    }

    if ((dataSize <= 0) || (dataSize > 65536)) {
        skip |= LogError(device, "VUID-vkCmdUpdateBuffer-dataSize-00037",
                         "vkCmdUpdateBuffer() parameter, VkDeviceSize dataSize (0x%" PRIxLEAST64
                         "), must be greater than zero and less than or equal to 65536.",
                         dataSize);
    } else if (dataSize & 3) {
        skip |= LogError(device, "VUID-vkCmdUpdateBuffer-dataSize-00038",
                         "vkCmdUpdateBuffer() parameter, VkDeviceSize dataSize (0x%" PRIxLEAST64 "), is not a multiple of 4.",
                         dataSize);
    }
    return skip;
}

bool StatelessValidation::manual_PreCallValidateCmdFillBuffer(VkCommandBuffer commandBuffer, VkBuffer dstBuffer,
                                                              VkDeviceSize dstOffset, VkDeviceSize size, uint32_t data) const {
    bool skip = false;

    if (dstOffset & 3) {
        skip |= LogError(device, "VUID-vkCmdFillBuffer-dstOffset-00025",
                         "vkCmdFillBuffer() parameter, VkDeviceSize dstOffset (0x%" PRIxLEAST64 "), is not a multiple of 4.",
                         dstOffset);
    }

    if (size != VK_WHOLE_SIZE) {
        if (size <= 0) {
            skip |=
                LogError(device, "VUID-vkCmdFillBuffer-size-00026",
                         "vkCmdFillBuffer() parameter, VkDeviceSize size (0x%" PRIxLEAST64 "), must be greater than zero.", size);
        } else if (size & 3) {
            skip |= LogError(device, "VUID-vkCmdFillBuffer-size-00028",
                             "vkCmdFillBuffer() parameter, VkDeviceSize size (0x%" PRIxLEAST64 "), is not a multiple of 4.", size);
        }
    }
    return skip;
}

bool StatelessValidation::manual_PreCallValidateCmdDispatch(VkCommandBuffer commandBuffer, uint32_t groupCountX,
                                                            uint32_t groupCountY, uint32_t groupCountZ) const {
    bool skip = false;

    if (groupCountX > device_limits.maxComputeWorkGroupCount[0]) {
        skip |=
            LogError(commandBuffer, "VUID-vkCmdDispatch-groupCountX-00386",
                     "vkCmdDispatch(): groupCountX (%" PRIu32 ") exceeds device limit maxComputeWorkGroupCount[0] (%" PRIu32 ").",
                     groupCountX, device_limits.maxComputeWorkGroupCount[0]);
    }

    if (groupCountY > device_limits.maxComputeWorkGroupCount[1]) {
        skip |=
            LogError(commandBuffer, "VUID-vkCmdDispatch-groupCountY-00387",
                     "vkCmdDispatch(): groupCountY (%" PRIu32 ") exceeds device limit maxComputeWorkGroupCount[1] (%" PRIu32 ").",
                     groupCountY, device_limits.maxComputeWorkGroupCount[1]);
    }

    if (groupCountZ > device_limits.maxComputeWorkGroupCount[2]) {
        skip |=
            LogError(commandBuffer, "VUID-vkCmdDispatch-groupCountZ-00388",
                     "vkCmdDispatch(): groupCountZ (%" PRIu32 ") exceeds device limit maxComputeWorkGroupCount[2] (%" PRIu32 ").",
                     groupCountZ, device_limits.maxComputeWorkGroupCount[2]);
    }

    return skip;
}

bool StatelessValidation::manual_PreCallValidateCmdDispatchIndirect(VkCommandBuffer commandBuffer, VkBuffer buffer,
                                                                    VkDeviceSize offset) const {
    bool skip = false;

    if (offset & 3) {
        skip |= LogError(commandBuffer, "VUID-vkCmdDispatchIndirect-offset-02710",
                         "vkCmdDispatchIndirect(): offset (%" PRIxLEAST64 ") must be a multiple of 4.", offset);
    }
    return skip;
}

bool StatelessValidation::manual_PreCallValidateCmdDispatchBaseKHR(VkCommandBuffer commandBuffer, uint32_t baseGroupX,
                                                                   uint32_t baseGroupY, uint32_t baseGroupZ, uint32_t groupCountX,
                                                                   uint32_t groupCountY, uint32_t groupCountZ) const {
    bool skip = false;

    // Paired if {} else if {} tests used to avoid any possible uint underflow
    uint32_t limit = device_limits.maxComputeWorkGroupCount[0];
    if (baseGroupX >= limit) {
        skip |= LogError(commandBuffer, "VUID-vkCmdDispatchBase-baseGroupX-00421",
                         "vkCmdDispatch(): baseGroupX (%" PRIu32
                         ") equals or exceeds device limit maxComputeWorkGroupCount[0] (%" PRIu32 ").",
                         baseGroupX, limit);
    } else if (groupCountX > (limit - baseGroupX)) {
        skip |= LogError(commandBuffer, "VUID-vkCmdDispatchBase-groupCountX-00424",
                         "vkCmdDispatchBaseKHR(): baseGroupX (%" PRIu32 ") + groupCountX (%" PRIu32
                         ") exceeds device limit maxComputeWorkGroupCount[0] (%" PRIu32 ").",
                         baseGroupX, groupCountX, limit);
    }

    limit = device_limits.maxComputeWorkGroupCount[1];
    if (baseGroupY >= limit) {
        skip |= LogError(commandBuffer, "VUID-vkCmdDispatchBase-baseGroupX-00422",
                         "vkCmdDispatch(): baseGroupY (%" PRIu32
                         ") equals or exceeds device limit maxComputeWorkGroupCount[1] (%" PRIu32 ").",
                         baseGroupY, limit);
    } else if (groupCountY > (limit - baseGroupY)) {
        skip |= LogError(commandBuffer, "VUID-vkCmdDispatchBase-groupCountY-00425",
                         "vkCmdDispatchBaseKHR(): baseGroupY (%" PRIu32 ") + groupCountY (%" PRIu32
                         ") exceeds device limit maxComputeWorkGroupCount[1] (%" PRIu32 ").",
                         baseGroupY, groupCountY, limit);
    }

    limit = device_limits.maxComputeWorkGroupCount[2];
    if (baseGroupZ >= limit) {
        skip |= LogError(commandBuffer, "VUID-vkCmdDispatchBase-baseGroupZ-00423",
                         "vkCmdDispatch(): baseGroupZ (%" PRIu32
                         ") equals or exceeds device limit maxComputeWorkGroupCount[2] (%" PRIu32 ").",
                         baseGroupZ, limit);
    } else if (groupCountZ > (limit - baseGroupZ)) {
        skip |= LogError(commandBuffer, "VUID-vkCmdDispatchBase-groupCountZ-00426",
                         "vkCmdDispatchBaseKHR(): baseGroupZ (%" PRIu32 ") + groupCountZ (%" PRIu32
                         ") exceeds device limit maxComputeWorkGroupCount[2] (%" PRIu32 ").",
                         baseGroupZ, groupCountZ, limit);
    }

    return skip;
}

bool StatelessValidation::manual_PreCallValidateCmdPushDescriptorSetKHR(VkCommandBuffer commandBuffer,
                                                                        VkPipelineBindPoint pipelineBindPoint,
                                                                        VkPipelineLayout layout, uint32_t set,
                                                                        uint32_t descriptorWriteCount,
                                                                        const VkWriteDescriptorSet *pDescriptorWrites) const {
    return ValidateWriteDescriptorSet("vkCmdPushDescriptorSetKHR", descriptorWriteCount, pDescriptorWrites, true);
}

bool StatelessValidation::manual_PreCallValidateCmdDrawMeshTasksNV(VkCommandBuffer commandBuffer, uint32_t taskCount,
                                                                   uint32_t firstTask) const {
    bool skip = false;

    if (taskCount > phys_dev_ext_props.mesh_shader_props_nv.maxDrawMeshTasksCount) {
        skip |= LogError(
            commandBuffer, "VUID-vkCmdDrawMeshTasksNV-taskCount-02119",
            "vkCmdDrawMeshTasksNV() parameter, uint32_t taskCount (0x%" PRIxLEAST32
            "), must be less than or equal to VkPhysicalDeviceMeshShaderPropertiesNV::maxDrawMeshTasksCount (0x%" PRIxLEAST32 ").",
            taskCount, phys_dev_ext_props.mesh_shader_props_nv.maxDrawMeshTasksCount);
    }

    return skip;
}

bool StatelessValidation::manual_PreCallValidateCmdDrawMeshTasksIndirectNV(VkCommandBuffer commandBuffer, VkBuffer buffer,
                                                                           VkDeviceSize offset, uint32_t drawCount,
                                                                           uint32_t stride) const {
    bool skip = false;
    if (offset & 3) {
        skip |= LogError(
            commandBuffer, "VUID-vkCmdDrawMeshTasksIndirectNV-offset-02710",
            "vkCmdDrawMeshTasksIndirectNV() parameter, VkDeviceSize offset (0x%" PRIxLEAST64 "), is not a multiple of 4.", offset);
    }
    if (drawCount > 1 && ((stride & 3) || stride < sizeof(VkDrawMeshTasksIndirectCommandNV))) {
        skip |= LogError(commandBuffer, "VUID-vkCmdDrawMeshTasksIndirectNV-drawCount-02146",
                         "vkCmdDrawMeshTasksIndirectNV() parameter, uint32_t stride (0x%" PRIxLEAST32
                         "), is not a multiple of 4 or smaller than sizeof (VkDrawMeshTasksIndirectCommandNV).",
                         stride);
    }
    if (!physical_device_features.multiDrawIndirect && ((drawCount > 1))) {
        skip |= LogError(
            commandBuffer, "VUID-vkCmdDrawMeshTasksIndirectNV-drawCount-02718",
            "vkCmdDrawMeshTasksIndirectNV(): Device feature multiDrawIndirect disabled: count must be 0 or 1 but is %" PRIu32 "",
            drawCount);
    }
    if (drawCount > device_limits.maxDrawIndirectCount) {
        skip |= LogError(commandBuffer, "VUID-vkCmdDrawMeshTasksIndirectNV-drawCount-02719",
                         "vkCmdDrawMeshTasksIndirectNV: drawCount (%" PRIu32
                         ") is not less than or equal to the maximum allowed (%" PRIu32 ").",
                         drawCount, device_limits.maxDrawIndirectCount);
    }
    return skip;
}

bool StatelessValidation::manual_PreCallValidateCmdDrawMeshTasksIndirectCountNV(VkCommandBuffer commandBuffer, VkBuffer buffer,
                                                                                VkDeviceSize offset, VkBuffer countBuffer,
                                                                                VkDeviceSize countBufferOffset,
                                                                                uint32_t maxDrawCount, uint32_t stride) const {
    bool skip = false;

    if (offset & 3) {
        skip |= LogError(commandBuffer, "VUID-vkCmdDrawMeshTasksIndirectCountNV-offset-02710",
                         "vkCmdDrawMeshTasksIndirectCountNV() parameter, VkDeviceSize offset (0x%" PRIxLEAST64
                         "), is not a multiple of 4.",
                         offset);
    }

    if (countBufferOffset & 3) {
        skip |= LogError(commandBuffer, "VUID-vkCmdDrawMeshTasksIndirectCountNV-countBufferOffset-02716",
                         "vkCmdDrawMeshTasksIndirectCountNV() parameter, VkDeviceSize countBufferOffset (0x%" PRIxLEAST64
                         "), is not a multiple of 4.",
                         countBufferOffset);
    }

    return skip;
}

bool StatelessValidation::manual_PreCallValidateCmdDrawMeshTasksEXT(VkCommandBuffer commandBuffer, uint32_t groupCountX,
                                                                    uint32_t groupCountY, uint32_t groupCountZ) const {
    bool skip = false;

    if (groupCountX > phys_dev_ext_props.mesh_shader_props_ext.maxTaskWorkGroupCount[0]) {
        skip |= LogError(
            commandBuffer, "VUID-vkCmdDrawMeshTasksEXT-TaskEXT-07322",
            "vkCmdDrawMeshTasksEXT() parameter, uint32_t groupCountX (0x%" PRIxLEAST32
            "), must be less than or equal to VkPhysicalDeviceMeshShaderPropertiesEXT::maxTaskWorkGroupCount[0] (0x%" PRIxLEAST32
            ").",
            groupCountX, phys_dev_ext_props.mesh_shader_props_ext.maxTaskWorkGroupCount[0]);
    }
    if (groupCountY > phys_dev_ext_props.mesh_shader_props_ext.maxTaskWorkGroupCount[1]) {
        skip |= LogError(
            commandBuffer, "VUID-vkCmdDrawMeshTasksEXT-TaskEXT-07323",
            "vkCmdDrawMeshTasksEXT() parameter, uint32_t groupCountY (0x%" PRIxLEAST32
            "), must be less than or equal to VkPhysicalDeviceMeshShaderPropertiesEXT::maxTaskWorkGroupCount[1] (0x%" PRIxLEAST32
            ").",
            groupCountY, phys_dev_ext_props.mesh_shader_props_ext.maxTaskWorkGroupCount[1]);
    }
    if (groupCountZ > phys_dev_ext_props.mesh_shader_props_ext.maxTaskWorkGroupCount[2]) {
        skip |= LogError(
            commandBuffer, "VUID-vkCmdDrawMeshTasksEXT-TaskEXT-07324",
            "vkCmdDrawMeshTasksEXT() parameter, uint32_t groupCountZ (0x%" PRIxLEAST32
            "), must be less than or equal to VkPhysicalDeviceMeshShaderPropertiesEXT::maxTaskWorkGroupCount[2] (0x%" PRIxLEAST32
            ").",
            groupCountZ, phys_dev_ext_props.mesh_shader_props_ext.maxTaskWorkGroupCount[2]);
    }

    uint32_t maxTaskWorkGroupTotalCount = phys_dev_ext_props.mesh_shader_props_ext.maxTaskWorkGroupTotalCount;
    uint64_t invocations = static_cast<uint64_t>(groupCountX) * static_cast<uint64_t>(groupCountY);
    // Prevent overflow.
    bool fail = false;
    if (invocations > vvl::MaxTypeValue(maxTaskWorkGroupTotalCount) || invocations > maxTaskWorkGroupTotalCount) {
        fail = true;
    }
    if (!fail) {
        invocations *= static_cast<uint64_t>(groupCountZ);
        if (invocations > vvl::MaxTypeValue(maxTaskWorkGroupTotalCount) || invocations > maxTaskWorkGroupTotalCount) {
            fail = true;
        }
    }
    if (fail) {
        skip |= LogError(commandBuffer, "VUID-vkCmdDrawMeshTasksEXT-TaskEXT-07325",
                         "vkCmdDrawMeshTasksEXT(): The product of groupCountX (0x%" PRIxLEAST32 "), groupCountY (0x%" PRIxLEAST32
                         ") and groupCountZ (0x%" PRIxLEAST32
                         ") must be less than or equal to "
                         "VkPhysicalDeviceMeshShaderPropertiesEXT::maxTaskWorkGroupTotalCount (0x%" PRIxLEAST32 ").",
                         groupCountX, groupCountY, groupCountZ, maxTaskWorkGroupTotalCount);
    }

    return skip;
}

bool StatelessValidation::manual_PreCallValidateCmdDrawMeshTasksIndirectEXT(VkCommandBuffer commandBuffer, VkBuffer buffer,
                                                                            VkDeviceSize offset, uint32_t drawCount,
                                                                            uint32_t stride) const {
    bool skip = false;

    // TODO: vkMapMemory() and check the contents of buffer at offset
    // issue #4547 (https://github.com/KhronosGroup/Vulkan-ValidationLayers/issues/4547)
    if (!physical_device_features.multiDrawIndirect && ((drawCount > 1))) {
        skip |= LogError(
            commandBuffer, "VUID-vkCmdDrawMeshTasksIndirectEXT-drawCount-02718",
            "vkCmdDrawMeshTasksIndirectEXT(): Device feature multiDrawIndirect disabled: count must be 0 or 1 but is %" PRIu32 "",
            drawCount);
    }
    if (drawCount > device_limits.maxDrawIndirectCount) {
        skip |= LogError(commandBuffer, "VUID-vkCmdDrawMeshTasksIndirectEXT-drawCount-02719",
                         "vkCmdDrawMeshTasksIndirectEXT: drawCount (%" PRIu32
                         ") is not less than or equal to the maximum allowed (%" PRIu32 ").",
                         drawCount, device_limits.maxDrawIndirectCount);
    }

    return skip;
}

bool StatelessValidation::manual_PreCallValidateViewport(const VkViewport &viewport, const char *fn_name,
                                                         const ParameterName &parameter_name, VkCommandBuffer object) const {
    bool skip = false;

    // Note: for numerical correctness
    //       - float comparisons should expect NaN (comparison always false).
    //       - VkPhysicalDeviceLimits::maxViewportDimensions is uint32_t, not float -> careful.

    const auto f_lte_u32_exact = [](const float v1_f, const uint32_t v2_u32) {
        if (std::isnan(v1_f)) return false;
        if (v1_f <= 0.0f) return true;

        float intpart;
        const float fract = modff(v1_f, &intpart);

        assert(std::numeric_limits<float>::radix == 2);
        const float u32_max_plus1 = ldexpf(1.0f, 32);  // hopefully exact
        if (intpart >= u32_max_plus1) return false;

        uint32_t v1_u32 = static_cast<uint32_t>(intpart);
        if (v1_u32 < v2_u32) {
            return true;
        } else if (v1_u32 == v2_u32 && fract == 0.0f) {
            return true;
        } else {
            return false;
        }
    };

    const auto f_lte_u32_direct = [](const float v1_f, const uint32_t v2_u32) {
        const float v2_f = static_cast<float>(v2_u32);  // not accurate for > radix^digits; and undefined rounding mode
        return (v1_f <= v2_f);
    };

    // width
    bool width_healthy = true;
    const auto max_w = device_limits.maxViewportDimensions[0];

    if (!(viewport.width > 0.0f)) {
        width_healthy = false;
        skip |= LogError(object, "VUID-VkViewport-width-01770", "%s: %s.width (=%f) is not greater than 0.0.", fn_name,
                         parameter_name.get_name().c_str(), viewport.width);
    } else if (!(f_lte_u32_exact(viewport.width, max_w) || f_lte_u32_direct(viewport.width, max_w))) {
        width_healthy = false;
        skip |= LogError(object, "VUID-VkViewport-width-01771",
                         "%s: %s.width (=%f) exceeds VkPhysicalDeviceLimits::maxViewportDimensions[0] (=%" PRIu32 ").", fn_name,
                         parameter_name.get_name().c_str(), viewport.width, max_w);
    }

    // height
    bool height_healthy = true;
    const bool negative_height_enabled =
        IsExtEnabled(device_extensions.vk_khr_maintenance1) || IsExtEnabled(device_extensions.vk_amd_negative_viewport_height);
    const auto max_h = device_limits.maxViewportDimensions[1];

    if (!negative_height_enabled && !(viewport.height > 0.0f)) {
        height_healthy = false;
        skip |= LogError(object, "VUID-VkViewport-apiVersion-07917", "%s: %s.height (=%f) is not greater 0.0.", fn_name,
                         parameter_name.get_name().c_str(), viewport.height);
    } else if (!(f_lte_u32_exact(fabsf(viewport.height), max_h) || f_lte_u32_direct(fabsf(viewport.height), max_h))) {
        height_healthy = false;

        skip |= LogError(object, "VUID-VkViewport-height-01773",
                         "%s: Absolute value of %s.height (=%f) exceeds VkPhysicalDeviceLimits::maxViewportDimensions[1] (=%" PRIu32
                         ").",
                         fn_name, parameter_name.get_name().c_str(), viewport.height, max_h);
    }

    // x
    bool x_healthy = true;
    if (!(viewport.x >= device_limits.viewportBoundsRange[0])) {
        x_healthy = false;
        skip |= LogError(object, "VUID-VkViewport-x-01774",
                         "%s: %s.x (=%f) is less than VkPhysicalDeviceLimits::viewportBoundsRange[0] (=%f).", fn_name,
                         parameter_name.get_name().c_str(), viewport.x, device_limits.viewportBoundsRange[0]);
    }

    // x + width
    if (x_healthy && width_healthy) {
        const float right_bound = viewport.x + viewport.width;
        if (right_bound > device_limits.viewportBoundsRange[1]) {
            skip |= LogError(
                object, "VUID-VkViewport-x-01232",
                "%s: %s.x + %s.width (=%f + %f = %f) is greater than VkPhysicalDeviceLimits::viewportBoundsRange[1] (=%f).",
                fn_name, parameter_name.get_name().c_str(), parameter_name.get_name().c_str(), viewport.x, viewport.width,
                right_bound, device_limits.viewportBoundsRange[1]);
        }
    }

    // y
    bool y_healthy = true;
    if (!(viewport.y >= device_limits.viewportBoundsRange[0])) {
        y_healthy = false;
        skip |= LogError(object, "VUID-VkViewport-y-01775",
                         "%s: %s.y (=%f) is less than VkPhysicalDeviceLimits::viewportBoundsRange[0] (=%f).", fn_name,
                         parameter_name.get_name().c_str(), viewport.y, device_limits.viewportBoundsRange[0]);
    } else if (negative_height_enabled && viewport.y > device_limits.viewportBoundsRange[1]) {
        y_healthy = false;
        skip |= LogError(object, "VUID-VkViewport-y-01776",
                         "%s: %s.y (=%f) exceeds VkPhysicalDeviceLimits::viewportBoundsRange[1] (=%f).", fn_name,
                         parameter_name.get_name().c_str(), viewport.y, device_limits.viewportBoundsRange[1]);
    }

    // y + height
    if (y_healthy && height_healthy) {
        const float boundary = viewport.y + viewport.height;

        if (boundary > device_limits.viewportBoundsRange[1]) {
            skip |= LogError(object, "VUID-VkViewport-y-01233",
                             "%s: %s.y + %s.height (=%f + %f = %f) exceeds VkPhysicalDeviceLimits::viewportBoundsRange[1] (=%f).",
                             fn_name, parameter_name.get_name().c_str(), parameter_name.get_name().c_str(), viewport.y,
                             viewport.height, boundary, device_limits.viewportBoundsRange[1]);
        } else if (negative_height_enabled && boundary < device_limits.viewportBoundsRange[0]) {
            skip |=
                LogError(object, "VUID-VkViewport-y-01777",
                         "%s: %s.y + %s.height (=%f + %f = %f) is less than VkPhysicalDeviceLimits::viewportBoundsRange[0] (=%f).",
                         fn_name, parameter_name.get_name().c_str(), parameter_name.get_name().c_str(), viewport.y, viewport.height,
                         boundary, device_limits.viewportBoundsRange[0]);
        }
    }

    if (!IsExtEnabled(device_extensions.vk_ext_depth_range_unrestricted)) {
        // minDepth
        if (!(viewport.minDepth >= 0.0) || !(viewport.minDepth <= 1.0)) {
            skip |= LogError(object, "VUID-VkViewport-minDepth-01234",
                             "%s: VK_EXT_depth_range_unrestricted extension is not enabled and %s.minDepth (=%f) is not within the "
                             "[0.0, 1.0] range.",
                             fn_name, parameter_name.get_name().c_str(), viewport.minDepth);
        }

        // maxDepth
        if (!(viewport.maxDepth >= 0.0) || !(viewport.maxDepth <= 1.0)) {
            skip |= LogError(object, "VUID-VkViewport-maxDepth-01235",
                             "%s: VK_EXT_depth_range_unrestricted extension is not enabled and %s.maxDepth (=%f) is not within the "
                             "[0.0, 1.0] range.",
                             fn_name, parameter_name.get_name().c_str(), viewport.maxDepth);
        }
    }

    return skip;
}

bool StatelessValidation::manual_PreCallValidateFreeCommandBuffers(VkDevice device, VkCommandPool commandPool,
                                                                   uint32_t commandBufferCount,
                                                                   const VkCommandBuffer *pCommandBuffers) const {
    bool skip = false;

    // Validation for parameters excluded from the generated validation code due to a 'noautovalidity' tag in vk.xml
    // This is an array of handles, where the elements are allowed to be VK_NULL_HANDLE, and does not require any validation beyond
    // ValidateArray()
    skip |= ValidateArray("vkFreeCommandBuffers", "commandBufferCount", "pCommandBuffers", commandBufferCount, &pCommandBuffers,
                          true, true, kVUIDUndefined, "VUID-vkFreeCommandBuffers-pCommandBuffers-00048");
    return skip;
}

bool StatelessValidation::manual_PreCallValidateBeginCommandBuffer(VkCommandBuffer commandBuffer,
                                                                   const VkCommandBufferBeginInfo *pBeginInfo) const {
    bool skip = false;

    // VkCommandBufferInheritanceInfo validation, due to a 'noautovalidity' of pBeginInfo->pInheritanceInfo in vkBeginCommandBuffer
    const char *cmd_name = "vkBeginCommandBuffer";
    bool cb_is_secondary;
    {
        auto lock = CBReadLock();
        cb_is_secondary = (secondary_cb_map.find(commandBuffer) != secondary_cb_map.end());
    }

    if (cb_is_secondary) {
        // Implicit VUs
        // validate only sType here; pointer has to be validated in core_validation
        const bool k_not_required = false;
        const char *k_no_vuid = nullptr;
        const VkCommandBufferInheritanceInfo *info = pBeginInfo->pInheritanceInfo;
        skip |= ValidateStructType(cmd_name, "pBeginInfo->pInheritanceInfo", "VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_INFO",
                                   info, VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_INFO, k_not_required, k_no_vuid,
                                   "VUID-VkCommandBufferInheritanceInfo-sType-sType");

        if (info) {
            constexpr std::array allowed_structs = {VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_CONDITIONAL_RENDERING_INFO_EXT,
                                                    VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_RENDERING_INFO_KHR,
                                                    VK_STRUCTURE_TYPE_ATTACHMENT_SAMPLE_COUNT_INFO_AMD,
                                                    VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_VIEWPORT_SCISSOR_INFO_NV};
            skip |= ValidateStructPnext(
                cmd_name, "pBeginInfo->pInheritanceInfo->pNext", "VkCommandBufferInheritanceConditionalRenderingInfoEXT",
                info->pNext, allowed_structs.size(), allowed_structs.data(), GeneratedVulkanHeaderVersion,
                "VUID-VkCommandBufferInheritanceInfo-pNext-pNext", "VUID-VkCommandBufferInheritanceInfo-sType-unique");

            skip |= ValidateBool32(cmd_name, "pBeginInfo->pInheritanceInfo->occlusionQueryEnable", info->occlusionQueryEnable);

            // Explicit VUs
            if (!physical_device_features.inheritedQueries && info->occlusionQueryEnable == VK_TRUE) {
                skip |= LogError(
                    commandBuffer, "VUID-VkCommandBufferInheritanceInfo-occlusionQueryEnable-00056",
                    "%s: Inherited queries feature is disabled, but pBeginInfo->pInheritanceInfo->occlusionQueryEnable is VK_TRUE.",
                    cmd_name);
            }

            if (physical_device_features.inheritedQueries) {
                skip |= ValidateFlags(cmd_name, "pBeginInfo->pInheritanceInfo->queryFlags", "VkQueryControlFlagBits",
                                      AllVkQueryControlFlagBits, info->queryFlags, kOptionalFlags,
                                      "VUID-VkCommandBufferInheritanceInfo-queryFlags-00057");
            } else {  // !inheritedQueries
                skip |= ValidateReservedFlags(cmd_name, "pBeginInfo->pInheritanceInfo->queryFlags", info->queryFlags,
                                              "VUID-VkCommandBufferInheritanceInfo-queryFlags-02788");
            }

            if (physical_device_features.pipelineStatisticsQuery) {
                skip |=
                    ValidateFlags(cmd_name, "pBeginInfo->pInheritanceInfo->pipelineStatistics", "VkQueryPipelineStatisticFlagBits",
                                  AllVkQueryPipelineStatisticFlagBits, info->pipelineStatistics, kOptionalFlags,
                                  "VUID-VkCommandBufferInheritanceInfo-pipelineStatistics-02789");
            } else {  // !pipelineStatisticsQuery
                skip |=
                    ValidateReservedFlags(cmd_name, "pBeginInfo->pInheritanceInfo->pipelineStatistics", info->pipelineStatistics,
                                          "VUID-VkCommandBufferInheritanceInfo-pipelineStatistics-00058");
            }

            const auto *conditional_rendering = LvlFindInChain<VkCommandBufferInheritanceConditionalRenderingInfoEXT>(info->pNext);
            if (conditional_rendering) {
                const auto *cr_features = LvlFindInChain<VkPhysicalDeviceConditionalRenderingFeaturesEXT>(device_createinfo_pnext);
                const auto inherited_conditional_rendering = cr_features && cr_features->inheritedConditionalRendering;
                if (!inherited_conditional_rendering && conditional_rendering->conditionalRenderingEnable == VK_TRUE) {
                    skip |= LogError(
                        commandBuffer,
                        "VUID-VkCommandBufferInheritanceConditionalRenderingInfoEXT-conditionalRenderingEnable-01977",
                        "vkBeginCommandBuffer: Inherited conditional rendering is disabled, but "
                        "pBeginInfo->pInheritanceInfo->pNext<VkCommandBufferInheritanceConditionalRenderingInfoEXT> is VK_TRUE.");
                }
            }

            auto p_inherited_viewport_scissor_info = LvlFindInChain<VkCommandBufferInheritanceViewportScissorInfoNV>(info->pNext);
            if (p_inherited_viewport_scissor_info != nullptr && !physical_device_features.multiViewport &&
                p_inherited_viewport_scissor_info->viewportScissor2D == VK_TRUE &&
                p_inherited_viewport_scissor_info->viewportDepthCount != 1) {
                skip |= LogError(commandBuffer, "VUID-VkCommandBufferInheritanceViewportScissorInfoNV-viewportScissor2D-04783",
                                 "vkBeginCommandBuffer: multiViewport feature is disabled, but "
                                 "VkCommandBufferInheritanceViewportScissorInfoNV::viewportScissor2D in "
                                 "pBeginInfo->pInheritanceInfo->pNext is VK_TRUE and viewportDepthCount is not 1.");
            }
        }
    }
    return skip;
}

void StatelessValidation::PostCallRecordAllocateCommandBuffers(VkDevice device, const VkCommandBufferAllocateInfo *pAllocateInfo,
                                                               VkCommandBuffer *pCommandBuffers, VkResult result) {
    if ((result == VK_SUCCESS) && pAllocateInfo && (pAllocateInfo->level == VK_COMMAND_BUFFER_LEVEL_SECONDARY)) {
        auto lock = CBWriteLock();
        for (uint32_t cb_index = 0; cb_index < pAllocateInfo->commandBufferCount; cb_index++) {
            secondary_cb_map.emplace(pCommandBuffers[cb_index], pAllocateInfo->commandPool);
        }
    }
}

void StatelessValidation::PostCallRecordFreeCommandBuffers(VkDevice device, VkCommandPool commandPool, uint32_t commandBufferCount,
                                                           const VkCommandBuffer *pCommandBuffers) {
    auto lock = CBWriteLock();
    for (uint32_t cb_index = 0; cb_index < commandBufferCount; cb_index++) {
        secondary_cb_map.erase(pCommandBuffers[cb_index]);
    }
}

void StatelessValidation::PostCallRecordDestroyCommandPool(VkDevice device, VkCommandPool commandPool,
                                                           const VkAllocationCallbacks *pAllocator) {
    auto lock = CBWriteLock();
    for (auto item = secondary_cb_map.begin(); item != secondary_cb_map.end();) {
        if (item->second == commandPool) {
            item = secondary_cb_map.erase(item);
        } else {
            ++item;
        }
    }
}
