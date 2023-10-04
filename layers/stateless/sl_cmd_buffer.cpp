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
                                                                   VkDeviceSize offset, VkIndexType indexType,
                                                                   const ErrorObject &error_obj) const {
    bool skip = false;

    if (indexType == VK_INDEX_TYPE_NONE_KHR) {
        skip |= LogError("VUID-vkCmdBindIndexBuffer-indexType-08786", commandBuffer, error_obj.location.dot(Field::indexType),
                         "is VK_INDEX_TYPE_NONE_KHR.");
    }

    const auto *index_type_uint8_features = vku::FindStructInPNextChain<VkPhysicalDeviceIndexTypeUint8FeaturesEXT>(device_createinfo_pnext);
    if (indexType == VK_INDEX_TYPE_UINT8_EXT && (!index_type_uint8_features || !index_type_uint8_features->indexTypeUint8)) {
        skip |= LogError("VUID-vkCmdBindIndexBuffer-indexType-08787", commandBuffer, error_obj.location.dot(Field::indexType),
                         "is VK_INDEX_TYPE_UINT8_EXT but indexTypeUint8 feature was not enabled.");
    }

    return skip;
}

bool StatelessValidation::manual_PreCallValidateCmdBindIndexBuffer2KHR(VkCommandBuffer commandBuffer, VkBuffer buffer,
                                                                       VkDeviceSize offset, VkDeviceSize size,
                                                                       VkIndexType indexType, const ErrorObject &error_obj) const {
    bool skip = false;

    if (indexType == VK_INDEX_TYPE_NONE_KHR) {
        skip |= LogError("VUID-vkCmdBindIndexBuffer2KHR-indexType-08786", commandBuffer, error_obj.location.dot(Field::indexType),
                         "is VK_INDEX_TYPE_NONE_KHR.");
    } else if (indexType == VK_INDEX_TYPE_UINT8_EXT) {
        const auto *index_type_uint8_features = vku::FindStructInPNextChain<VkPhysicalDeviceIndexTypeUint8FeaturesEXT>(device_createinfo_pnext);
        if (!index_type_uint8_features || !index_type_uint8_features->indexTypeUint8) {
            skip |=
                LogError("VUID-vkCmdBindIndexBuffer2KHR-indexType-08787", commandBuffer, error_obj.location.dot(Field::indexType),
                         "is VK_INDEX_TYPE_UINT8_EXT but indexTypeUint8 feature was not enabled.");
        }
    }

    return skip;
}

bool StatelessValidation::manual_PreCallValidateCmdBindVertexBuffers(VkCommandBuffer commandBuffer, uint32_t firstBinding,
                                                                     uint32_t bindingCount, const VkBuffer *pBuffers,
                                                                     const VkDeviceSize *pOffsets,
                                                                     const ErrorObject &error_obj) const {
    bool skip = false;
    if (firstBinding > device_limits.maxVertexInputBindings) {
        skip |= LogError("VUID-vkCmdBindVertexBuffers-firstBinding-00624", commandBuffer, error_obj.location,
                         "firstBinding (%" PRIu32 ") must be less than maxVertexInputBindings (%" PRIu32 ").", firstBinding,
                         device_limits.maxVertexInputBindings);
    } else if ((firstBinding + bindingCount) > device_limits.maxVertexInputBindings) {
        skip |= LogError("VUID-vkCmdBindVertexBuffers-firstBinding-00625", commandBuffer, error_obj.location,
                         "sum of firstBinding (%" PRIu32 ") and bindingCount (%" PRIu32
                         ") must be less than "
                         "maxVertexInputBindings (%" PRIu32 ").",
                         firstBinding, bindingCount, device_limits.maxVertexInputBindings);
    }

    for (uint32_t i = 0; i < bindingCount; ++i) {
        if (pBuffers[i] == VK_NULL_HANDLE) {
            const Location buffer_loc = error_obj.location.dot(Field::pBuffers, i);
            const auto *robustness2_features = vku::FindStructInPNextChain<VkPhysicalDeviceRobustness2FeaturesEXT>(device_createinfo_pnext);
            if (!(robustness2_features && robustness2_features->nullDescriptor)) {
                skip |= LogError("VUID-vkCmdBindVertexBuffers-pBuffers-04001", commandBuffer, buffer_loc, "is VK_NULL_HANDLE.");
            } else {
                if (pOffsets[i] != 0) {
                    skip |= LogError("VUID-vkCmdBindVertexBuffers-pBuffers-04002", commandBuffer, buffer_loc,
                                     "is VK_NULL_HANDLE, but pOffsets[%" PRIu32 "] is not 0.", i);
                }
            }
        }
    }

    return skip;
}

bool StatelessValidation::manual_PreCallValidateCmdBindTransformFeedbackBuffersEXT(
    VkCommandBuffer commandBuffer, uint32_t firstBinding, uint32_t bindingCount, const VkBuffer *pBuffers,
    const VkDeviceSize *pOffsets, const VkDeviceSize *pSizes, const ErrorObject &error_obj) const {
    bool skip = false;

    for (uint32_t i = 0; i < bindingCount; ++i) {
        if (pOffsets[i] & 3) {
            skip |= LogError("VUID-vkCmdBindTransformFeedbackBuffersEXT-pOffsets-02359", commandBuffer,
                             error_obj.location.dot(Field::pOffsets, i), "(%" PRIu64 ") is not a multiple of 4.", pOffsets[i]);
        }
    }

    if (firstBinding >= phys_dev_ext_props.transform_feedback_props.maxTransformFeedbackBuffers) {
        skip |= LogError("VUID-vkCmdBindTransformFeedbackBuffersEXT-firstBinding-02356", commandBuffer, error_obj.location,
                         "The firstBinding(%" PRIu32
                         ") index is greater than or equal to "
                         "maxTransformFeedbackBuffers(%" PRIu32 ").",
                         firstBinding, phys_dev_ext_props.transform_feedback_props.maxTransformFeedbackBuffers);
    }

    if (firstBinding + bindingCount > phys_dev_ext_props.transform_feedback_props.maxTransformFeedbackBuffers) {
        skip |= LogError("VUID-vkCmdBindTransformFeedbackBuffersEXT-firstBinding-02357", commandBuffer, error_obj.location,
                         "The sum of firstBinding(%" PRIu32 ") and bindCount(%" PRIu32
                         ") is greater than maxTransformFeedbackBuffers(%" PRIu32 ").",
                         firstBinding, bindingCount, phys_dev_ext_props.transform_feedback_props.maxTransformFeedbackBuffers);
    }

    for (uint32_t i = 0; i < bindingCount; ++i) {
        // pSizes is optional and may be nullptr.
        if (pSizes != nullptr) {
            if (pSizes[i] != VK_WHOLE_SIZE &&
                pSizes[i] > phys_dev_ext_props.transform_feedback_props.maxTransformFeedbackBufferSize) {
                skip |= LogError("VUID-vkCmdBindTransformFeedbackBuffersEXT-pSize-02361", commandBuffer,
                                 error_obj.location.dot(Field::pSizes, i),
                                 "(%" PRIu64
                                 ") is not VK_WHOLE_SIZE and is greater than "
                                 "maxTransformFeedbackBufferSize.",
                                 pSizes[i]);
            }
        }
    }

    return skip;
}

bool StatelessValidation::manual_PreCallValidateCmdBeginTransformFeedbackEXT(
    VkCommandBuffer commandBuffer, uint32_t firstCounterBuffer, uint32_t counterBufferCount, const VkBuffer *pCounterBuffers,
    const VkDeviceSize *pCounterBufferOffsets, const ErrorObject &error_obj) const {
    bool skip = false;

    if (firstCounterBuffer >= phys_dev_ext_props.transform_feedback_props.maxTransformFeedbackBuffers) {
        skip |= LogError("VUID-vkCmdBeginTransformFeedbackEXT-firstCounterBuffer-02368", commandBuffer, error_obj.location,
                         "The firstCounterBuffer(%" PRIu32
                         ") index is greater than or equal to "
                         "maxTransformFeedbackBuffers(%" PRIu32 ").",
                         firstCounterBuffer, phys_dev_ext_props.transform_feedback_props.maxTransformFeedbackBuffers);
    }

    if (firstCounterBuffer + counterBufferCount > phys_dev_ext_props.transform_feedback_props.maxTransformFeedbackBuffers) {
        skip |= LogError("VUID-vkCmdBeginTransformFeedbackEXT-firstCounterBuffer-02369", commandBuffer, error_obj.location,
                         "The sum of firstCounterBuffer(%" PRIu32 ") and counterBufferCount(%" PRIu32
                         ") is greater than maxTransformFeedbackBuffers(%" PRIu32 ").",
                         firstCounterBuffer, counterBufferCount,
                         phys_dev_ext_props.transform_feedback_props.maxTransformFeedbackBuffers);
    }

    return skip;
}

bool StatelessValidation::manual_PreCallValidateCmdEndTransformFeedbackEXT(VkCommandBuffer commandBuffer,
                                                                           uint32_t firstCounterBuffer, uint32_t counterBufferCount,
                                                                           const VkBuffer *pCounterBuffers,
                                                                           const VkDeviceSize *pCounterBufferOffsets,
                                                                           const ErrorObject &error_obj) const {
    bool skip = false;

    if (firstCounterBuffer >= phys_dev_ext_props.transform_feedback_props.maxTransformFeedbackBuffers) {
        skip |= LogError("VUID-vkCmdEndTransformFeedbackEXT-firstCounterBuffer-02376", commandBuffer, error_obj.location,
                         "The firstCounterBuffer(%" PRIu32
                         ") index is greater than or equal to "
                         "maxTransformFeedbackBuffers(%" PRIu32 ").",
                         firstCounterBuffer, phys_dev_ext_props.transform_feedback_props.maxTransformFeedbackBuffers);
    }

    if (firstCounterBuffer + counterBufferCount > phys_dev_ext_props.transform_feedback_props.maxTransformFeedbackBuffers) {
        skip |= LogError("VUID-vkCmdEndTransformFeedbackEXT-firstCounterBuffer-02377", commandBuffer, error_obj.location,
                         "The sum of firstCounterBuffer(%" PRIu32 ") and counterBufferCount(%" PRIu32
                         ") is greater than maxTransformFeedbackBuffers(%" PRIu32 ").",
                         firstCounterBuffer, counterBufferCount,
                         phys_dev_ext_props.transform_feedback_props.maxTransformFeedbackBuffers);
    }

    return skip;
}

bool StatelessValidation::manual_PreCallValidateCmdBindVertexBuffers2(VkCommandBuffer commandBuffer, uint32_t firstBinding,
                                                                      uint32_t bindingCount, const VkBuffer *pBuffers,
                                                                      const VkDeviceSize *pOffsets, const VkDeviceSize *pSizes,
                                                                      const VkDeviceSize *pStrides,
                                                                      const ErrorObject &error_obj) const {
    bool skip = false;

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
                skip |= LogError("VUID-vkCmdBindVertexBuffers2-bindingCount-arraylength", commandBuffer, error_obj.location,
                                 "%s, so bindingCount must be greater than 0.", not_null_msg);
            }
        }
    }

    if (firstBinding >= device_limits.maxVertexInputBindings) {
        skip |= LogError("VUID-vkCmdBindVertexBuffers2-firstBinding-03355", commandBuffer, error_obj.location,
                         "firstBinding (%" PRIu32 ") must be less than maxVertexInputBindings (%" PRIu32 ").", firstBinding,
                         device_limits.maxVertexInputBindings);
    } else if ((firstBinding + bindingCount) > device_limits.maxVertexInputBindings) {
        skip |= LogError("VUID-vkCmdBindVertexBuffers2-firstBinding-03356", commandBuffer, error_obj.location,
                         "sum of firstBinding (%" PRIu32 ") and bindingCount (%" PRIu32
                         ") must be less than "
                         "maxVertexInputBindings (%" PRIu32 ").",
                         firstBinding, bindingCount, device_limits.maxVertexInputBindings);
    }

    for (uint32_t i = 0; i < bindingCount; ++i) {
        if (pBuffers[i] == VK_NULL_HANDLE) {
            const Location buffer_loc = error_obj.location.dot(Field::pBuffers, i);
            const auto *robustness2_features = vku::FindStructInPNextChain<VkPhysicalDeviceRobustness2FeaturesEXT>(device_createinfo_pnext);
            if (!(robustness2_features && robustness2_features->nullDescriptor)) {
                skip |= LogError("VUID-vkCmdBindVertexBuffers2-pBuffers-04111", commandBuffer, buffer_loc, "is VK_NULL_HANDLE.");
            } else {
                if (pOffsets[i] != 0) {
                    skip |= LogError("VUID-vkCmdBindVertexBuffers2-pBuffers-04112", commandBuffer, buffer_loc,
                                     "is VK_NULL_HANDLE, but pOffsets[%" PRIu32 "] is not 0.", i);
                }
            }
        }
        if (pStrides) {
            if (pStrides[i] > device_limits.maxVertexInputBindingStride) {
                skip |= LogError("VUID-vkCmdBindVertexBuffers2-pStrides-03362", commandBuffer,
                                 error_obj.location.dot(Field::pStrides, i),
                                 "(%" PRIu64 ") must be less than maxVertexInputBindingStride (%" PRIu32 ").", pStrides[i],
                                 device_limits.maxVertexInputBindingStride);
            }
        }
    }

    return skip;
}

bool StatelessValidation::manual_PreCallValidateCmdBindVertexBuffers2EXT(VkCommandBuffer commandBuffer, uint32_t firstBinding,
                                                                         uint32_t bindingCount, const VkBuffer *pBuffers,
                                                                         const VkDeviceSize *pOffsets, const VkDeviceSize *pSizes,
                                                                         const VkDeviceSize *pStrides,
                                                                         const ErrorObject &error_obj) const {
    return manual_PreCallValidateCmdBindVertexBuffers2(commandBuffer, firstBinding, bindingCount, pBuffers, pOffsets, pSizes,
                                                       pStrides, error_obj);
}

bool StatelessValidation::manual_PreCallValidateCmdPushConstants(VkCommandBuffer commandBuffer, VkPipelineLayout layout,
                                                                 VkShaderStageFlags stageFlags, uint32_t offset, uint32_t size,
                                                                 const void *pValues, const ErrorObject &error_obj) const {
    bool skip = false;
    const uint32_t max_push_constants_size = device_limits.maxPushConstantsSize;
    // Check that offset + size don't exceed the max.
    // Prevent arithetic overflow here by avoiding addition and testing in this order.
    if (offset >= max_push_constants_size) {
        skip |= LogError("VUID-vkCmdPushConstants-offset-00370", device, error_obj.location.dot(Field::offset),
                         "(%" PRIu32 ") that exceeds this device's maxPushConstantSize of %" PRIu32 ".", offset,
                         max_push_constants_size);
    }
    if (size > max_push_constants_size - offset) {
        skip |= LogError("VUID-vkCmdPushConstants-size-00371", device, error_obj.location.dot(Field::offset),
                         "(%" PRIu32 ") and size (%" PRIu32 ") that exceeds this device's maxPushConstantSize of %" PRIu32 ".",
                         offset, size, max_push_constants_size);
    }

    // size needs to be non-zero and a multiple of 4.
    if (size & 0x3) {
        skip |= LogError("VUID-vkCmdPushConstants-size-00369", device, error_obj.location.dot(Field::size),
                         "(%" PRIu32 ") must be a multiple of 4.", size);
    }

    // offset needs to be a multiple of 4.
    if ((offset & 0x3) != 0) {
        skip |= LogError("VUID-vkCmdPushConstants-offset-00368", device, error_obj.location.dot(Field::offset),
                         "(%" PRIu32 ") must be a multiple of 4.", offset);
    }
    return skip;
}

bool StatelessValidation::manual_PreCallValidateCmdClearColorImage(VkCommandBuffer commandBuffer, VkImage image,
                                                                   VkImageLayout imageLayout, const VkClearColorValue *pColor,
                                                                   uint32_t rangeCount, const VkImageSubresourceRange *pRanges,
                                                                   const ErrorObject &error_obj) const {
    bool skip = false;
    if (!pColor) {
        skip |= LogError("VUID-vkCmdClearColorImage-pColor-04961", commandBuffer, error_obj.location, "pColor must not be null");
    }
    return skip;
}

bool StatelessValidation::ValidateCmdBeginRenderPass(const VkRenderPassBeginInfo *const rp_begin,
                                                     const ErrorObject &error_obj) const {
    bool skip = false;
    if ((rp_begin->clearValueCount != 0) && !rp_begin->pClearValues) {
        skip |= LogError("VUID-VkRenderPassBeginInfo-clearValueCount-04962", rp_begin->renderPass,
                         error_obj.location.dot(Field::pRenderPassBegin).dot(Field::clearValueCount),
                         "(%" PRIu32 ") is not zero, but pRenderPassBegin->pClearValues is NULL.", rp_begin->clearValueCount);
    }
    return skip;
}

bool StatelessValidation::manual_PreCallValidateCmdBeginRenderPass(VkCommandBuffer, const VkRenderPassBeginInfo *pRenderPassBegin,
                                                                   VkSubpassContents, const ErrorObject &error_obj) const {
    bool skip = ValidateCmdBeginRenderPass(pRenderPassBegin, error_obj);
    return skip;
}

bool StatelessValidation::manual_PreCallValidateCmdBeginRenderPass2KHR(VkCommandBuffer commandBuffer,
                                                                       const VkRenderPassBeginInfo *pRenderPassBegin,
                                                                       const VkSubpassBeginInfo *pSubpassBeginInfo,
                                                                       const ErrorObject &error_obj) const {
    return manual_PreCallValidateCmdBeginRenderPass2(commandBuffer, pRenderPassBegin, pSubpassBeginInfo, error_obj);
}

bool StatelessValidation::manual_PreCallValidateCmdBeginRenderPass2(VkCommandBuffer, const VkRenderPassBeginInfo *pRenderPassBegin,
                                                                    const VkSubpassBeginInfo *,
                                                                    const ErrorObject &error_obj) const {
    bool skip = ValidateCmdBeginRenderPass(pRenderPassBegin, error_obj);
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

bool StatelessValidation::manual_PreCallValidateCmdBeginRendering(VkCommandBuffer commandBuffer,
                                                                  const VkRenderingInfo *pRenderingInfo,
                                                                  const ErrorObject &error_obj) const {
    bool skip = false;
    const Location rendering_info_loc = error_obj.location.dot(Field::pRenderingInfo);

    if (pRenderingInfo->viewMask == 0 && pRenderingInfo->layerCount == 0) {
        skip |= LogError("VUID-VkRenderingInfo-viewMask-06069", commandBuffer, rendering_info_loc,
                         "viewMask and layerCount are both zero");
    }

    if (pRenderingInfo->colorAttachmentCount > device_limits.maxColorAttachments) {
        skip |= LogError("VUID-VkRenderingInfo-colorAttachmentCount-06106", commandBuffer,
                         rendering_info_loc.dot(Field::colorAttachmentCount),
                         "(%" PRIu32
                         ") must be less than or equal to "
                         "maxColorAttachments (%" PRIu32 ").",
                         pRenderingInfo->colorAttachmentCount, device_limits.maxColorAttachments);
    }

    const auto rendering_fragment_shading_rate_attachment_info =
        vku::FindStructInPNextChain<VkRenderingFragmentShadingRateAttachmentInfoKHR>(pRenderingInfo->pNext);
    if (rendering_fragment_shading_rate_attachment_info &&
        (rendering_fragment_shading_rate_attachment_info->imageView != VK_NULL_HANDLE)) {
        if (UniqueRenderingInfoImageViews(pRenderingInfo, rendering_fragment_shading_rate_attachment_info->imageView) == false) {
            skip |= LogError("VUID-VkRenderingInfo-imageView-06125", commandBuffer,
                             rendering_info_loc.pNext(Struct::VkRenderingFragmentShadingRateAttachmentInfoKHR, Field::imageView),
                             "is %s.", FormatHandle(rendering_fragment_shading_rate_attachment_info->imageView).c_str());
        }

        const VkImageLayout image_layout = rendering_fragment_shading_rate_attachment_info->imageLayout;
        if (image_layout != VK_IMAGE_LAYOUT_GENERAL &&
            image_layout != VK_IMAGE_LAYOUT_FRAGMENT_SHADING_RATE_ATTACHMENT_OPTIMAL_KHR) {
            skip |= LogError("VUID-VkRenderingFragmentShadingRateAttachmentInfoKHR-imageView-06147", commandBuffer,
                             rendering_info_loc.pNext(Struct::VkRenderingFragmentShadingRateAttachmentInfoKHR, Field::layout),
                             "is (%s).", string_VkImageLayout(image_layout));
        }

        if (!IsPowerOfTwo(rendering_fragment_shading_rate_attachment_info->shadingRateAttachmentTexelSize.width)) {
            skip |=
                LogError("VUID-VkRenderingFragmentShadingRateAttachmentInfoKHR-imageView-06149", commandBuffer,
                         rendering_info_loc
                             .pNext(Struct::VkRenderingFragmentShadingRateAttachmentInfoKHR, Field::shadingRateAttachmentTexelSize)
                             .dot(Field::width),
                         "(%" PRIu32 ") must be a power of two.",
                         rendering_fragment_shading_rate_attachment_info->shadingRateAttachmentTexelSize.width);
        }

        const uint32_t max_frs_attach_texel_width =
            phys_dev_ext_props.fragment_shading_rate_props.maxFragmentShadingRateAttachmentTexelSize.width;
        if (rendering_fragment_shading_rate_attachment_info->shadingRateAttachmentTexelSize.width > max_frs_attach_texel_width) {
            skip |= LogError(
                "VUID-VkRenderingFragmentShadingRateAttachmentInfoKHR-imageView-06150", commandBuffer,
                rendering_info_loc
                    .pNext(Struct::VkRenderingFragmentShadingRateAttachmentInfoKHR, Field::shadingRateAttachmentTexelSize)
                    .dot(Field::width),
                "(%" PRIu32
                ") must be less than or equal to "
                "maxFragmentShadingRateAttachmentTexelSize.width (%" PRIu32 ").",
                rendering_fragment_shading_rate_attachment_info->shadingRateAttachmentTexelSize.width, max_frs_attach_texel_width);
        }

        const uint32_t min_frs_attach_texel_width =
            phys_dev_ext_props.fragment_shading_rate_props.minFragmentShadingRateAttachmentTexelSize.width;
        if (rendering_fragment_shading_rate_attachment_info->shadingRateAttachmentTexelSize.width < min_frs_attach_texel_width) {
            skip |= LogError(
                "VUID-VkRenderingFragmentShadingRateAttachmentInfoKHR-imageView-06151", commandBuffer,
                rendering_info_loc
                    .pNext(Struct::VkRenderingFragmentShadingRateAttachmentInfoKHR, Field::shadingRateAttachmentTexelSize)
                    .dot(Field::width),
                "(%" PRIu32
                ") must be greater than or equal to "
                "minFragmentShadingRateAttachmentTexelSize.width (%" PRIu32 ").",
                rendering_fragment_shading_rate_attachment_info->shadingRateAttachmentTexelSize.width, min_frs_attach_texel_width);
        }

        if (!IsPowerOfTwo(rendering_fragment_shading_rate_attachment_info->shadingRateAttachmentTexelSize.height)) {
            skip |=
                LogError("VUID-VkRenderingFragmentShadingRateAttachmentInfoKHR-imageView-06152", commandBuffer,
                         rendering_info_loc
                             .pNext(Struct::VkRenderingFragmentShadingRateAttachmentInfoKHR, Field::shadingRateAttachmentTexelSize)
                             .dot(Field::height),
                         "(%" PRIu32 ") must be a power of two.",
                         rendering_fragment_shading_rate_attachment_info->shadingRateAttachmentTexelSize.height);
        }

        const uint32_t max_frs_attach_texel_height =
            phys_dev_ext_props.fragment_shading_rate_props.maxFragmentShadingRateAttachmentTexelSize.height;
        if (rendering_fragment_shading_rate_attachment_info->shadingRateAttachmentTexelSize.height > max_frs_attach_texel_height) {
            skip |=
                LogError("VUID-VkRenderingFragmentShadingRateAttachmentInfoKHR-imageView-06153", commandBuffer,
                         rendering_info_loc
                             .pNext(Struct::VkRenderingFragmentShadingRateAttachmentInfoKHR, Field::shadingRateAttachmentTexelSize)
                             .dot(Field::height),
                         "(%" PRIu32
                         ") must be less than or equal to "
                         "maxFragmentShadingRateAttachmentTexelSize.height (%" PRIu32 ").",
                         rendering_fragment_shading_rate_attachment_info->shadingRateAttachmentTexelSize.height,
                         max_frs_attach_texel_height);
        }

        const uint32_t min_frs_attach_texel_height =
            phys_dev_ext_props.fragment_shading_rate_props.minFragmentShadingRateAttachmentTexelSize.height;
        if (rendering_fragment_shading_rate_attachment_info->shadingRateAttachmentTexelSize.height < min_frs_attach_texel_height) {
            skip |=
                LogError("VUID-VkRenderingFragmentShadingRateAttachmentInfoKHR-imageView-06154", commandBuffer,
                         rendering_info_loc
                             .pNext(Struct::VkRenderingFragmentShadingRateAttachmentInfoKHR, Field::shadingRateAttachmentTexelSize)
                             .dot(Field::height),
                         "(%" PRIu32
                         ") must be greater than or equal to "
                         "minFragmentShadingRateAttachmentTexelSize.height (%" PRIu32 ").",
                         rendering_fragment_shading_rate_attachment_info->shadingRateAttachmentTexelSize.height,
                         min_frs_attach_texel_height);
        }

        const uint32_t max_frs_attach_texel_aspect_ratio =
            phys_dev_ext_props.fragment_shading_rate_props.maxFragmentShadingRateAttachmentTexelSizeAspectRatio;
        if ((rendering_fragment_shading_rate_attachment_info->shadingRateAttachmentTexelSize.width /
             rendering_fragment_shading_rate_attachment_info->shadingRateAttachmentTexelSize.height) >
            max_frs_attach_texel_aspect_ratio) {
            skip |= LogError("VUID-VkRenderingFragmentShadingRateAttachmentInfoKHR-imageView-06155", commandBuffer,
                             rendering_info_loc.pNext(Struct::VkRenderingFragmentShadingRateAttachmentInfoKHR,
                                                      Field::shadingRateAttachmentTexelSize),
                             "the quotient of width (%" PRIu32 ") and height (%" PRIu32
                             ") "
                             "must be less than or equal to maxFragmentShadingRateAttachmentTexelSizeAspectRatio (%" PRIu32 ").",
                             rendering_fragment_shading_rate_attachment_info->shadingRateAttachmentTexelSize.width,
                             rendering_fragment_shading_rate_attachment_info->shadingRateAttachmentTexelSize.height,
                             max_frs_attach_texel_aspect_ratio);
        }

        if ((rendering_fragment_shading_rate_attachment_info->shadingRateAttachmentTexelSize.height /
             rendering_fragment_shading_rate_attachment_info->shadingRateAttachmentTexelSize.width) >
            max_frs_attach_texel_aspect_ratio) {
            skip |= LogError("VUID-VkRenderingFragmentShadingRateAttachmentInfoKHR-imageView-06156", commandBuffer,
                             rendering_info_loc.pNext(Struct::VkRenderingFragmentShadingRateAttachmentInfoKHR,
                                                      Field::shadingRateAttachmentTexelSize),
                             "the quotient of height (%" PRIu32 ") and width (%" PRIu32
                             ") "
                             "must be less than or equal to maxFragmentShadingRateAttachmentTexelSizeAspectRatio (%" PRIu32 ").",
                             rendering_fragment_shading_rate_attachment_info->shadingRateAttachmentTexelSize.height,
                             rendering_fragment_shading_rate_attachment_info->shadingRateAttachmentTexelSize.width,
                             max_frs_attach_texel_aspect_ratio);
        }
    }

    const auto fragment_density_map_attachment_info =
        vku::FindStructInPNextChain<VkRenderingFragmentDensityMapAttachmentInfoEXT>(pRenderingInfo->pNext);
    if (fragment_density_map_attachment_info && (fragment_density_map_attachment_info->imageView != VK_NULL_HANDLE)) {
        if (UniqueRenderingInfoImageViews(pRenderingInfo, fragment_density_map_attachment_info->imageView) == false) {
            skip |= LogError("VUID-VkRenderingInfo-imageView-06116", commandBuffer,
                             rendering_info_loc.pNext(Struct::VkRenderingFragmentDensityMapAttachmentInfoEXT, Field::imageView),
                             "is %s.", FormatHandle(fragment_density_map_attachment_info->imageView).c_str());
        }

        if (fragment_density_map_attachment_info->imageLayout != VK_IMAGE_LAYOUT_GENERAL &&
            fragment_density_map_attachment_info->imageLayout != VK_IMAGE_LAYOUT_FRAGMENT_DENSITY_MAP_OPTIMAL_EXT) {
            skip |= LogError("VUID-VkRenderingFragmentDensityMapAttachmentInfoEXT-imageView-06157", commandBuffer,
                             rendering_info_loc.pNext(Struct::VkRenderingFragmentDensityMapAttachmentInfoEXT, Field::imageView),
                             "is %s, but "
                             "VkRenderingFragmentDensityMapAttachmentInfoEXT::imageLayout is %s.",
                             FormatHandle(fragment_density_map_attachment_info->imageView).c_str(),
                             string_VkImageLayout(fragment_density_map_attachment_info->imageLayout));
        }

        if (rendering_fragment_shading_rate_attachment_info &&
            (rendering_fragment_shading_rate_attachment_info->imageView == fragment_density_map_attachment_info->imageView)) {
            skip |= LogError("VUID-VkRenderingInfo-imageView-06126", commandBuffer,
                             rendering_info_loc.pNext(Struct::VkRenderingFragmentDensityMapAttachmentInfoEXT, Field::imageView),
                             "and VkRenderingFragmentShadingRateAttachmentInfoKHR::imageView are the same (%s).",
                             FormatHandle(fragment_density_map_attachment_info->imageView).c_str());
        }
    }

    for (uint32_t j = 0; j < pRenderingInfo->colorAttachmentCount; ++j) {
        if (pRenderingInfo->pColorAttachments[j].imageView == VK_NULL_HANDLE) {
            continue;
        }
        const Location attachment_loc = rendering_info_loc.dot(Field::pColorAttachments, j);
        const VkImageLayout image_layout = pRenderingInfo->pColorAttachments[j].imageLayout;
        const VkResolveModeFlagBits resolve_mode = pRenderingInfo->pColorAttachments[j].resolveMode;
        const VkImageLayout resolve_image_layout = pRenderingInfo->pColorAttachments[j].resolveImageLayout;
        if (image_layout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL ||
            image_layout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL) {
            skip |= LogError("VUID-VkRenderingInfo-colorAttachmentCount-06090", commandBuffer,
                             attachment_loc.dot(Field::imageLayout), "is %s.", string_VkImageLayout(image_layout));
        }

        if (resolve_mode != VK_RESOLVE_MODE_NONE) {
            if (resolve_image_layout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL ||
                resolve_image_layout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL) {
                skip |= LogError("VUID-VkRenderingInfo-colorAttachmentCount-06091", commandBuffer,
                                 attachment_loc.dot(Field::resolveImageLayout), "is %s and resolve_mode is %s.",
                                 string_VkImageLayout(resolve_image_layout), string_VkResolveModeFlagBits(resolve_mode));
            }
        }

        if (IsExtEnabled(device_extensions.vk_khr_maintenance2)) {
            if (image_layout == VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_STENCIL_ATTACHMENT_OPTIMAL ||
                image_layout == VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_STENCIL_READ_ONLY_OPTIMAL) {
                skip |= LogError("VUID-VkRenderingInfo-colorAttachmentCount-06096", commandBuffer,
                                 attachment_loc.dot(Field::imageLayout), "is %s.", string_VkImageLayout(image_layout));
            }

            if (resolve_mode != VK_RESOLVE_MODE_NONE) {
                if (resolve_image_layout == VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_STENCIL_ATTACHMENT_OPTIMAL ||
                    resolve_image_layout == VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_STENCIL_READ_ONLY_OPTIMAL) {
                    skip |= LogError("VUID-VkRenderingInfo-colorAttachmentCount-06097", commandBuffer,
                                     attachment_loc.dot(Field::resolveImageLayout), "is %s and resolve_mode is %s.",
                                     string_VkImageLayout(resolve_image_layout), string_VkResolveModeFlagBits(resolve_mode));
                }
            }
        }

        if (IsImageLayoutDepthOnly(image_layout) || IsImageLayoutStencilOnly(image_layout)) {
            skip |= LogError("VUID-VkRenderingInfo-colorAttachmentCount-06100", commandBuffer,
                             attachment_loc.dot(Field::imageLayout), "is %s.", string_VkImageLayout(image_layout));
        }

        if (resolve_mode != VK_RESOLVE_MODE_NONE) {
            if (resolve_image_layout == VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL ||
                resolve_image_layout == VK_IMAGE_LAYOUT_STENCIL_ATTACHMENT_OPTIMAL) {
                skip |= LogError("VUID-VkRenderingInfo-colorAttachmentCount-06101", commandBuffer,
                                 attachment_loc.dot(Field::resolveImageLayout), "is %s and resolve_mode is %s.",
                                 string_VkImageLayout(resolve_image_layout), string_VkResolveModeFlagBits(resolve_mode));
            }
        }
    }

    if (pRenderingInfo->pDepthAttachment && pRenderingInfo->pDepthAttachment->imageView != VK_NULL_HANDLE) {
        const Location attachment_loc = rendering_info_loc.dot(Field::pDepthAttachment);
        const VkImageLayout layout = pRenderingInfo->pDepthAttachment->imageLayout;
        if (layout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL) {
            skip |= LogError("VUID-VkRenderingInfo-pDepthAttachment-06092", commandBuffer, attachment_loc.dot(Field::imageLayout),
                             "is VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL.");
        } else if (IsImageLayoutStencilOnly(layout)) {
            skip |= LogError("VUID-VkRenderingInfo-pDepthAttachment-07732", commandBuffer, attachment_loc.dot(Field::imageLayout),
                             "is %s.", string_VkImageLayout(layout));
        }

        if (pRenderingInfo->pDepthAttachment->resolveMode != VK_RESOLVE_MODE_NONE) {
            const VkImageLayout resolve_layout = pRenderingInfo->pDepthAttachment->resolveImageLayout;
            if (resolve_layout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL) {
                skip |= LogError("VUID-VkRenderingInfo-pDepthAttachment-06093", commandBuffer,
                                 attachment_loc.dot(Field::resolveImageLayout),
                                 "is VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL and resolve_mode is %s.",
                                 string_VkResolveModeFlagBits(pRenderingInfo->pDepthAttachment->resolveMode));
            } else if (IsImageLayoutStencilOnly(resolve_layout)) {
                skip |= LogError("VUID-VkRenderingInfo-pDepthAttachment-07733", commandBuffer,
                                 attachment_loc.dot(Field::resolveImageLayout), "is %s and resolve_mode is %s.",
                                 string_VkImageLayout(resolve_layout),
                                 string_VkResolveModeFlagBits(pRenderingInfo->pDepthAttachment->resolveMode));
            }

            if (IsExtEnabled(device_extensions.vk_khr_maintenance2) &&
                resolve_layout == VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_STENCIL_ATTACHMENT_OPTIMAL) {
                skip |= LogError("VUID-VkRenderingInfo-pDepthAttachment-06098", commandBuffer,
                                 attachment_loc.dot(Field::resolveImageLayout),
                                 "is VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_STENCIL_ATTACHMENT_OPTIMAL.");
            }

            if (!(pRenderingInfo->pDepthAttachment->resolveMode &
                  phys_dev_ext_props.depth_stencil_resolve_props.supportedDepthResolveModes)) {
                skip |= LogError(
                    "VUID-VkRenderingInfo-pDepthAttachment-06102", commandBuffer, attachment_loc.dot(Field::resolveMode),
                    "is %s, but supportedDepthResolveModes is %s.",
                    string_VkResolveModeFlagBits(pRenderingInfo->pDepthAttachment->resolveMode),
                    string_VkResolveModeFlags(phys_dev_ext_props.depth_stencil_resolve_props.supportedDepthResolveModes).c_str());
            }
        }
    }

    if (pRenderingInfo->pStencilAttachment != nullptr && pRenderingInfo->pStencilAttachment->imageView != VK_NULL_HANDLE) {
        const Location attachment_loc = rendering_info_loc.dot(Field::pStencilAttachment);
        const VkImageLayout layout = pRenderingInfo->pStencilAttachment->imageLayout;
        if (layout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL) {
            skip |= LogError("VUID-VkRenderingInfo-pStencilAttachment-06094", commandBuffer, attachment_loc.dot(Field::imageLayout),
                             "is VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL.");
        } else if (IsImageLayoutDepthOnly(layout)) {
            skip |= LogError("VUID-VkRenderingInfo-pStencilAttachment-07734", commandBuffer, attachment_loc.dot(Field::imageLayout),
                             "is %s.", string_VkImageLayout(layout));
        }

        if (pRenderingInfo->pStencilAttachment->resolveMode != VK_RESOLVE_MODE_NONE) {
            const VkImageLayout resolve_layout = pRenderingInfo->pStencilAttachment->resolveImageLayout;
            if (resolve_layout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL) {
                skip |= LogError("VUID-VkRenderingInfo-pStencilAttachment-06095", commandBuffer,
                                 attachment_loc.dot(Field::resolveImageLayout),
                                 "is VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL and resolve_mode is %s.",
                                 string_VkResolveModeFlagBits(pRenderingInfo->pStencilAttachment->resolveMode));
            } else if (IsImageLayoutDepthOnly(resolve_layout)) {
                skip |= LogError("VUID-VkRenderingInfo-pStencilAttachment-07735", commandBuffer,
                                 attachment_loc.dot(Field::resolveImageLayout), "is %s and resolve_mode is %s.",
                                 string_VkImageLayout(resolve_layout),
                                 string_VkResolveModeFlagBits(pRenderingInfo->pStencilAttachment->resolveMode));
            }

            if (IsExtEnabled(device_extensions.vk_khr_maintenance2) &&
                resolve_layout == VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_STENCIL_READ_ONLY_OPTIMAL) {
                skip |= LogError("VUID-VkRenderingInfo-pStencilAttachment-06099", commandBuffer,
                                 attachment_loc.dot(Field::resolveImageLayout),
                                 "is VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_STENCIL_ATTACHMENT_OPTIMAL.");
            }

            if (!(pRenderingInfo->pStencilAttachment->resolveMode &
                  phys_dev_ext_props.depth_stencil_resolve_props.supportedStencilResolveModes)) {
                skip |= LogError(
                    "VUID-VkRenderingInfo-pStencilAttachment-06103", commandBuffer, attachment_loc.dot(Field::resolveMode),
                    "is %s, but supportedStencilResolveModes is %s.",
                    string_VkResolveModeFlagBits(pRenderingInfo->pStencilAttachment->resolveMode),
                    string_VkResolveModeFlags(phys_dev_ext_props.depth_stencil_resolve_props.supportedStencilResolveModes).c_str());
            }
        }
    }

    return skip;
}

bool StatelessValidation::manual_PreCallValidateCmdBeginRenderingKHR(VkCommandBuffer commandBuffer,
                                                                     const VkRenderingInfo *pRenderingInfo,
                                                                     const ErrorObject &error_obj) const {
    return manual_PreCallValidateCmdBeginRendering(commandBuffer, pRenderingInfo, error_obj);
}

bool StatelessValidation::manual_PreCallValidateGetQueryPoolResults(VkDevice device, VkQueryPool queryPool, uint32_t firstQuery,
                                                                    uint32_t queryCount, size_t dataSize, void *pData,
                                                                    VkDeviceSize stride, VkQueryResultFlags flags,
                                                                    const ErrorObject &error_obj) const {
    bool skip = false;

    if ((flags & VK_QUERY_RESULT_WITH_STATUS_BIT_KHR) && (flags & VK_QUERY_RESULT_WITH_AVAILABILITY_BIT)) {
        skip |= LogError("VUID-vkGetQueryPoolResults-flags-04811", device, error_obj.location.dot(Field::flags),
                         "(%s) include both STATUS_BIT and AVAILABILITY_BIT.", string_VkQueryResultFlags(flags).c_str());
    }

    return skip;
}

bool StatelessValidation::manual_PreCallValidateCmdBeginConditionalRenderingEXT(
    VkCommandBuffer commandBuffer, const VkConditionalRenderingBeginInfoEXT *pConditionalRenderingBegin,
    const ErrorObject &error_obj) const {
    bool skip = false;

    if ((pConditionalRenderingBegin->offset & 3) != 0) {
        skip |= LogError("VUID-VkConditionalRenderingBeginInfoEXT-offset-01984", commandBuffer, error_obj.location,
                         " pConditionalRenderingBegin->offset (%" PRIu64 ") is not a multiple of 4.",
                         pConditionalRenderingBegin->offset);
    }

    return skip;
}

bool StatelessValidation::manual_PreCallValidateCmdClearAttachments(VkCommandBuffer commandBuffer, uint32_t attachmentCount,
                                                                    const VkClearAttachment *pAttachments, uint32_t rectCount,
                                                                    const VkClearRect *pRects, const ErrorObject &error_obj) const {
    bool skip = false;
    for (uint32_t rect = 0; rect < rectCount; rect++) {
        const Location rect_loc = error_obj.location.dot(Field::pRects, rect);
        if (pRects[rect].layerCount == 0) {
            skip |=
                LogError("VUID-vkCmdClearAttachments-layerCount-01934", commandBuffer, rect_loc.dot(Field::layerCount), "is zero.");
        }
        if (pRects[rect].rect.extent.width == 0) {
            skip |= LogError("VUID-vkCmdClearAttachments-rect-02682", commandBuffer,
                             rect_loc.dot(Field::rect).dot(Field::extent).dot(Field::width), "is zero.");
        }
        if (pRects[rect].rect.extent.height == 0) {
            skip |= LogError("VUID-vkCmdClearAttachments-rect-02683", commandBuffer,
                             rect_loc.dot(Field::rect).dot(Field::extent).dot(Field::height), "is zero.");
        }
    }
    return skip;
}

bool StatelessValidation::manual_PreCallValidateCmdCopyBuffer(VkCommandBuffer commandBuffer, VkBuffer srcBuffer, VkBuffer dstBuffer,
                                                              uint32_t regionCount, const VkBufferCopy *pRegions,
                                                              const ErrorObject &error_obj) const {
    bool skip = false;

    if (pRegions != nullptr) {
        for (uint32_t i = 0; i < regionCount; i++) {
            if (pRegions[i].size == 0) {
                skip |= LogError("VUID-VkBufferCopy-size-01988", commandBuffer,
                                 error_obj.location.dot(Field::pRegions, i).dot(Field::size), "is zero");
            }
        }
    }
    return skip;
}

bool StatelessValidation::manual_PreCallValidateCmdCopyBuffer2KHR(VkCommandBuffer commandBuffer,
                                                                  const VkCopyBufferInfo2KHR *pCopyBufferInfo,
                                                                  const ErrorObject &error_obj) const {
    return manual_PreCallValidateCmdCopyBuffer2(commandBuffer, pCopyBufferInfo, error_obj);
}

bool StatelessValidation::manual_PreCallValidateCmdCopyBuffer2(VkCommandBuffer commandBuffer,
                                                               const VkCopyBufferInfo2 *pCopyBufferInfo,
                                                               const ErrorObject &error_obj) const {
    bool skip = false;

    if (pCopyBufferInfo->pRegions != nullptr) {
        for (uint32_t i = 0; i < pCopyBufferInfo->regionCount; i++) {
            if (pCopyBufferInfo->pRegions[i].size == 0) {
                skip |=
                    LogError("VUID-VkBufferCopy2-size-01988", commandBuffer,
                             error_obj.location.dot(Field::pCopyBufferInfo).dot(Field::pRegions, i).dot(Field::size), "is zero");
            }
        }
    }
    return skip;
}

bool StatelessValidation::manual_PreCallValidateCmdUpdateBuffer(VkCommandBuffer commandBuffer, VkBuffer dstBuffer,
                                                                VkDeviceSize dstOffset, VkDeviceSize dataSize, const void *pData,
                                                                const ErrorObject &error_obj) const {
    bool skip = false;

    if (dstOffset & 3) {
        skip |= LogError("VUID-vkCmdUpdateBuffer-dstOffset-00036", commandBuffer, error_obj.location.dot(Field::dstOffset),
                         "(%" PRIu64 "), is not a multiple of 4.", dstOffset);
    }

    if ((dataSize <= 0) || (dataSize > 65536)) {
        skip |= LogError("VUID-vkCmdUpdateBuffer-dataSize-00037", commandBuffer, error_obj.location.dot(Field::dataSize),
                         "(%" PRIu64 "), must be greater than zero and less than or equal to 65536.", dataSize);
    } else if (dataSize & 3) {
        skip |= LogError("VUID-vkCmdUpdateBuffer-dataSize-00038", commandBuffer, error_obj.location.dot(Field::dataSize),
                         "(%" PRIu64 "), is not a multiple of 4.", dataSize);
    }
    return skip;
}

bool StatelessValidation::manual_PreCallValidateCmdFillBuffer(VkCommandBuffer commandBuffer, VkBuffer dstBuffer,
                                                              VkDeviceSize dstOffset, VkDeviceSize size, uint32_t data,
                                                              const ErrorObject &error_obj) const {
    bool skip = false;

    if (dstOffset & 3) {
        skip |= LogError("VUID-vkCmdFillBuffer-dstOffset-00025", dstBuffer, error_obj.location.dot(Field::dstOffset),
                         "(%" PRIu64 ") is not a multiple of 4.", dstOffset);
    }

    if (size != VK_WHOLE_SIZE) {
        if (size <= 0) {
            skip |= LogError("VUID-vkCmdFillBuffer-size-00026", dstBuffer, error_obj.location.dot(Field::size),
                             "(%" PRIu64 ") must be greater than zero.", size);
        } else if (size & 3) {
            skip |= LogError("VUID-vkCmdFillBuffer-size-00028", dstBuffer, error_obj.location.dot(Field::size),
                             "(%" PRIu64 ") is not a multiple of 4.", size);
        }
    }
    return skip;
}

bool StatelessValidation::manual_PreCallValidateCmdPushDescriptorSetKHR(
    VkCommandBuffer commandBuffer, VkPipelineBindPoint pipelineBindPoint, VkPipelineLayout layout, uint32_t set,
    uint32_t descriptorWriteCount, const VkWriteDescriptorSet *pDescriptorWrites, const ErrorObject &error_obj) const {
    return ValidateWriteDescriptorSet(error_obj.location, descriptorWriteCount, pDescriptorWrites, true);
}

bool StatelessValidation::ValidateViewport(const VkViewport &viewport, VkCommandBuffer object, const Location &loc) const {
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
        skip |=
            LogError("VUID-VkViewport-width-01770", object, loc.dot(Field::width), "(%f) is not greater than 0.0.", viewport.width);
    } else if (!(f_lte_u32_exact(viewport.width, max_w) || f_lte_u32_direct(viewport.width, max_w))) {
        width_healthy = false;
        skip |= LogError("VUID-VkViewport-width-01771", object, loc.dot(Field::width),
                         "(%f) exceeds VkPhysicalDeviceLimits::maxViewportDimensions[0] (%" PRIu32 ").", viewport.width, max_w);
    }

    // height
    bool height_healthy = true;
    const bool negative_height_enabled =
        IsExtEnabled(device_extensions.vk_khr_maintenance1) || IsExtEnabled(device_extensions.vk_amd_negative_viewport_height);
    const auto max_h = device_limits.maxViewportDimensions[1];

    if (!negative_height_enabled && !(viewport.height > 0.0f)) {
        height_healthy = false;
        skip |= LogError("VUID-VkViewport-apiVersion-07917", object, loc.dot(Field::height), "(%f) is not greater 0.0.",
                         viewport.height);
    } else if (!(f_lte_u32_exact(fabsf(viewport.height), max_h) || f_lte_u32_direct(fabsf(viewport.height), max_h))) {
        height_healthy = false;

        skip |= LogError("VUID-VkViewport-height-01773", object, loc.dot(Field::height),
                         "absolute value (%f) exceeds VkPhysicalDeviceLimits::maxViewportDimensions[1] (%" PRIu32 ").",
                         viewport.height, max_h);
    }

    // x
    bool x_healthy = true;
    if (!(viewport.x >= device_limits.viewportBoundsRange[0])) {
        x_healthy = false;
        skip |= LogError("VUID-VkViewport-x-01774", object, loc.dot(Field::x),
                         "(%f) is less than VkPhysicalDeviceLimits::viewportBoundsRange[0] (%f).", viewport.x,
                         device_limits.viewportBoundsRange[0]);
    }

    // x + width
    if (x_healthy && width_healthy) {
        const float right_bound = viewport.x + viewport.width;
        if (right_bound > device_limits.viewportBoundsRange[1]) {
            skip |= LogError("VUID-VkViewport-x-01232", object, loc,
                             "x (%f) + width (%f) is %f which is greater than VkPhysicalDeviceLimits::viewportBoundsRange[1] (%f).",
                             viewport.x, viewport.width, right_bound, device_limits.viewportBoundsRange[1]);
        }
    }

    // y
    bool y_healthy = true;
    if (!(viewport.y >= device_limits.viewportBoundsRange[0])) {
        y_healthy = false;
        skip |= LogError("VUID-VkViewport-y-01775", object, loc.dot(Field::y),
                         "(%f) is less than VkPhysicalDeviceLimits::viewportBoundsRange[0] (%f).", viewport.y,
                         device_limits.viewportBoundsRange[0]);
    } else if (negative_height_enabled && viewport.y > device_limits.viewportBoundsRange[1]) {
        y_healthy = false;
        skip |= LogError("VUID-VkViewport-y-01776", object, loc.dot(Field::y),
                         "(%f) exceeds VkPhysicalDeviceLimits::viewportBoundsRange[1] (%f).", viewport.y,
                         device_limits.viewportBoundsRange[1]);
    }

    // y + height
    if (y_healthy && height_healthy) {
        const float boundary = viewport.y + viewport.height;

        if (boundary > device_limits.viewportBoundsRange[1]) {
            skip |= LogError("VUID-VkViewport-y-01233", object, loc,
                             "y (%f) + height (%f) is %f which exceeds VkPhysicalDeviceLimits::viewportBoundsRange[1] (%f).",
                             viewport.y, viewport.height, boundary, device_limits.viewportBoundsRange[1]);
        } else if (negative_height_enabled && boundary < device_limits.viewportBoundsRange[0]) {
            skip |= LogError("VUID-VkViewport-y-01777", object, loc,
                             "y (%f) + height (%f) is %f which is less than VkPhysicalDeviceLimits::viewportBoundsRange[0] (%f).",
                             viewport.y, viewport.height, boundary, device_limits.viewportBoundsRange[0]);
        }
    }

    if (!IsExtEnabled(device_extensions.vk_ext_depth_range_unrestricted)) {
        // minDepth
        if (!(viewport.minDepth >= 0.0) || !(viewport.minDepth <= 1.0)) {
            skip |= LogError("VUID-VkViewport-minDepth-01234", object, loc.dot(Field::minDepth), "is %f.", viewport.minDepth);
        }

        // maxDepth
        if (!(viewport.maxDepth >= 0.0) || !(viewport.maxDepth <= 1.0)) {
            skip |= LogError("VUID-VkViewport-maxDepth-01235", object, loc.dot(Field::maxDepth), "is %f.", viewport.maxDepth);
        }
    }

    return skip;
}

bool StatelessValidation::manual_PreCallValidateFreeCommandBuffers(VkDevice device, VkCommandPool commandPool,
                                                                   uint32_t commandBufferCount,
                                                                   const VkCommandBuffer *pCommandBuffers,
                                                                   const ErrorObject &error_obj) const {
    bool skip = false;

    // Validation for parameters excluded from the generated validation code due to a 'noautovalidity' tag in vk.xml
    // This is an array of handles, where the elements are allowed to be VK_NULL_HANDLE, and does not require any validation beyond
    // ValidateArray()
    skip |= ValidateArray(error_obj.location.dot(Field::commandBufferCount), error_obj.location.dot(Field::pCommandBuffers),
                          commandBufferCount, &pCommandBuffers, true, true, kVUIDUndefined,
                          "VUID-vkFreeCommandBuffers-pCommandBuffers-00048");
    return skip;
}

bool StatelessValidation::manual_PreCallValidateBeginCommandBuffer(VkCommandBuffer commandBuffer,
                                                                   const VkCommandBufferBeginInfo *pBeginInfo,
                                                                   const ErrorObject &error_obj) const {
    bool skip = false;

    // VkCommandBufferInheritanceInfo validation, due to a 'noautovalidity' of pBeginInfo->pInheritanceInfo in vkBeginCommandBuffer
    bool cb_is_secondary;
    {
        auto lock = CBReadLock();
        cb_is_secondary = (secondary_cb_map.find(commandBuffer) != secondary_cb_map.end());
    }

    if (!cb_is_secondary) {
        return skip;
    }
    // Implicit VUs
    // validate only sType here; pointer has to be validated in core_validation
    const bool k_not_required = false;
    const char *k_no_vuid = nullptr;
    const VkCommandBufferInheritanceInfo *info = pBeginInfo->pInheritanceInfo;
    const Location begin_info_loc = error_obj.location.dot(Field::pBeginInfo);
    const Location inheritance_loc = begin_info_loc.dot(Field::pInheritanceInfo);
    skip |= ValidateStructType(inheritance_loc, "VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_INFO", info,
                               VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_INFO, k_not_required, k_no_vuid,
                               "VUID-VkCommandBufferInheritanceInfo-sType-sType");

    if (info) {
        constexpr std::array allowed_structs = {VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_CONDITIONAL_RENDERING_INFO_EXT,
                                                VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_RENDERING_INFO_KHR,
                                                VK_STRUCTURE_TYPE_ATTACHMENT_SAMPLE_COUNT_INFO_AMD,
                                                VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_VIEWPORT_SCISSOR_INFO_NV};
        skip |= ValidateStructPnext(inheritance_loc, info->pNext, allowed_structs.size(), allowed_structs.data(),
                                    GeneratedVulkanHeaderVersion, "VUID-VkCommandBufferInheritanceInfo-pNext-pNext",
                                    "VUID-VkCommandBufferInheritanceInfo-sType-unique");

        skip |= ValidateBool32(inheritance_loc.dot(Field::occlusionQueryEnable), info->occlusionQueryEnable);

        // Explicit VUs
        if (!physical_device_features.inheritedQueries && info->occlusionQueryEnable == VK_TRUE) {
            skip |= LogError(
                "VUID-VkCommandBufferInheritanceInfo-occlusionQueryEnable-00056", commandBuffer, error_obj.location,
                "Inherited queries feature is disabled, but pBeginInfo->pInheritanceInfo->occlusionQueryEnable is VK_TRUE.");
        }

        if (physical_device_features.inheritedQueries) {
            skip |= ValidateFlags(inheritance_loc.dot(Field::queryFlags), "VkQueryControlFlagBits", AllVkQueryControlFlagBits,
                                  info->queryFlags, kOptionalFlags, "VUID-VkCommandBufferInheritanceInfo-queryFlags-00057");
        } else {  // !inheritedQueries
            skip |= ValidateReservedFlags(inheritance_loc.dot(Field::queryFlags), info->queryFlags,
                                          "VUID-VkCommandBufferInheritanceInfo-queryFlags-02788");
        }

        if (physical_device_features.pipelineStatisticsQuery) {
            skip |= ValidateFlags(inheritance_loc.dot(Field::pipelineStatistics), "VkQueryPipelineStatisticFlagBits",
                                  AllVkQueryPipelineStatisticFlagBits, info->pipelineStatistics, kOptionalFlags,
                                  "VUID-VkCommandBufferInheritanceInfo-pipelineStatistics-02789");
        } else {  // !pipelineStatisticsQuery
            skip |= ValidateReservedFlags(inheritance_loc.dot(Field::pipelineStatistics), info->pipelineStatistics,
                                          "VUID-VkCommandBufferInheritanceInfo-pipelineStatistics-00058");
        }

        const auto *conditional_rendering = vku::FindStructInPNextChain<VkCommandBufferInheritanceConditionalRenderingInfoEXT>(info->pNext);
        if (conditional_rendering) {
            const auto *cr_features = vku::FindStructInPNextChain<VkPhysicalDeviceConditionalRenderingFeaturesEXT>(device_createinfo_pnext);
            const auto inherited_conditional_rendering = cr_features && cr_features->inheritedConditionalRendering;
            if (!inherited_conditional_rendering && conditional_rendering->conditionalRenderingEnable == VK_TRUE) {
                skip |= LogError(
                    "VUID-VkCommandBufferInheritanceConditionalRenderingInfoEXT-conditionalRenderingEnable-01977", commandBuffer,
                    error_obj.location,
                    "Inherited conditional rendering is disabled, but "
                    "pBeginInfo->pInheritanceInfo->pNext<VkCommandBufferInheritanceConditionalRenderingInfoEXT> is VK_TRUE.");
            }
        }

        auto p_inherited_viewport_scissor_info = vku::FindStructInPNextChain<VkCommandBufferInheritanceViewportScissorInfoNV>(info->pNext);
        if (p_inherited_viewport_scissor_info != nullptr && !physical_device_features.multiViewport &&
            p_inherited_viewport_scissor_info->viewportScissor2D == VK_TRUE &&
            p_inherited_viewport_scissor_info->viewportDepthCount != 1) {
            skip |= LogError("VUID-VkCommandBufferInheritanceViewportScissorInfoNV-viewportScissor2D-04783", commandBuffer,
                             error_obj.location,
                             "multiViewport feature was not enabled, but "
                             "VkCommandBufferInheritanceViewportScissorInfoNV::viewportScissor2D in "
                             "pBeginInfo->pInheritanceInfo->pNext is VK_TRUE and viewportDepthCount is not 1.");
        }
    }
    return skip;
}

void StatelessValidation::PostCallRecordAllocateCommandBuffers(VkDevice device, const VkCommandBufferAllocateInfo *pAllocateInfo,
                                                               VkCommandBuffer *pCommandBuffers, const RecordObject &record_obj) {
    if ((record_obj.result == VK_SUCCESS) && pAllocateInfo && (pAllocateInfo->level == VK_COMMAND_BUFFER_LEVEL_SECONDARY)) {
        auto lock = CBWriteLock();
        for (uint32_t cb_index = 0; cb_index < pAllocateInfo->commandBufferCount; cb_index++) {
            secondary_cb_map.emplace(pCommandBuffers[cb_index], pAllocateInfo->commandPool);
        }
    }
}

void StatelessValidation::PostCallRecordFreeCommandBuffers(VkDevice device, VkCommandPool commandPool, uint32_t commandBufferCount,
                                                           const VkCommandBuffer *pCommandBuffers, const RecordObject &record_obj) {
    auto lock = CBWriteLock();
    for (uint32_t cb_index = 0; cb_index < commandBufferCount; cb_index++) {
        secondary_cb_map.erase(pCommandBuffers[cb_index]);
    }
}

void StatelessValidation::PostCallRecordDestroyCommandPool(VkDevice device, VkCommandPool commandPool,
                                                           const VkAllocationCallbacks *pAllocator,
                                                           const RecordObject &record_obj) {
    auto lock = CBWriteLock();
    for (auto item = secondary_cb_map.begin(); item != secondary_cb_map.end();) {
        if (item->second == commandPool) {
            item = secondary_cb_map.erase(item);
        } else {
            ++item;
        }
    }
}
