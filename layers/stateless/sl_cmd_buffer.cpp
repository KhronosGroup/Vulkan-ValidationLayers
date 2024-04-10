/* Copyright (c) 2015-2024 The Khronos Group Inc.
 * Copyright (c) 2015-2024 Valve Corporation
 * Copyright (c) 2015-2024 LunarG, Inc.
 * Copyright (C) 2015-2024 Google Inc.
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

bool StatelessValidation::manual_PreCallValidateCmdBindIndexBuffer(VkCommandBuffer commandBuffer, VkBuffer buffer,
                                                                   VkDeviceSize offset, VkIndexType indexType,
                                                                   const ErrorObject &error_obj) const {
    bool skip = false;

    if (indexType == VK_INDEX_TYPE_NONE_KHR) {
        skip |= LogError("VUID-vkCmdBindIndexBuffer-indexType-08786", commandBuffer, error_obj.location.dot(Field::indexType),
                         "is VK_INDEX_TYPE_NONE_KHR.");
    }

    const auto *index_type_uint8_features = vku::FindStructInPNextChain<VkPhysicalDeviceIndexTypeUint8FeaturesEXT>(device_createinfo_pnext);
    if (indexType == VK_INDEX_TYPE_UINT8_KHR && (!index_type_uint8_features || !index_type_uint8_features->indexTypeUint8)) {
        skip |= LogError("VUID-vkCmdBindIndexBuffer-indexType-08787", commandBuffer, error_obj.location.dot(Field::indexType),
                         "is VK_INDEX_TYPE_UINT8_KHR but indexTypeUint8 feature was not enabled.");
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
    } else if (indexType == VK_INDEX_TYPE_UINT8_KHR) {
        const auto *index_type_uint8_features = vku::FindStructInPNextChain<VkPhysicalDeviceIndexTypeUint8FeaturesEXT>(device_createinfo_pnext);
        if (!index_type_uint8_features || !index_type_uint8_features->indexTypeUint8) {
            skip |=
                LogError("VUID-vkCmdBindIndexBuffer2KHR-indexType-08787", commandBuffer, error_obj.location.dot(Field::indexType),
                         "is VK_INDEX_TYPE_UINT8_KHR but indexTypeUint8 feature was not enabled.");
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
        if (pBuffers == nullptr) {
            skip |= LogError("VUID-vkCmdBindVertexBuffers-pBuffers-parameter", commandBuffer,
                             error_obj.location.dot(Field::pBuffers), "is NULL.");
            return skip;
        }
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
    // This is a special case and generator currently skips it
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

        if (!pOffsets && bindingCount > 0) {
            skip |= LogError("VUID-vkCmdBindVertexBuffers2-pOffsets-parameter", commandBuffer,
                             error_obj.location.dot(Field::pOffsets), "is NULL.");
        }
    }

    if (firstBinding >= device_limits.maxVertexInputBindings) {
        skip |=
            LogError("VUID-vkCmdBindVertexBuffers2-firstBinding-03355", commandBuffer, error_obj.location.dot(Field::firstBinding),
                     "(%" PRIu32 ") must be less than maxVertexInputBindings (%" PRIu32 ").", firstBinding,
                     device_limits.maxVertexInputBindings);
    } else if ((firstBinding + bindingCount) > device_limits.maxVertexInputBindings) {
        skip |=
            LogError("VUID-vkCmdBindVertexBuffers2-firstBinding-03356", commandBuffer, error_obj.location.dot(Field::firstBinding),
                     "(%" PRIu32 ") + bindingCount (%" PRIu32
                     ") must be less than "
                     "maxVertexInputBindings (%" PRIu32 ").",
                     firstBinding, bindingCount, device_limits.maxVertexInputBindings);
    }

    for (uint32_t i = 0; i < bindingCount; ++i) {
        if (pBuffers == nullptr) {
            skip |= LogError("VUID-vkCmdBindVertexBuffers2-pBuffers-parameter", commandBuffer,
                             error_obj.location.dot(Field::pBuffers), "is NULL.");
            return skip;
        }
        if (pBuffers[i] == VK_NULL_HANDLE) {
            const Location buffer_loc = error_obj.location.dot(Field::pBuffers, i);
            const auto *robustness2_features = vku::FindStructInPNextChain<VkPhysicalDeviceRobustness2FeaturesEXT>(device_createinfo_pnext);
            if (!(robustness2_features && robustness2_features->nullDescriptor)) {
                skip |= LogError("VUID-vkCmdBindVertexBuffers2-pBuffers-04111", commandBuffer, buffer_loc, "is VK_NULL_HANDLE.");
            } else if (pOffsets && pOffsets[i] != 0) {
                skip |= LogError("VUID-vkCmdBindVertexBuffers2-pBuffers-04112", commandBuffer, buffer_loc,
                                 "is VK_NULL_HANDLE, but pOffsets[%" PRIu32 "] is not 0.", i);
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

bool StatelessValidation::ValidateCmdPushConstants(VkCommandBuffer commandBuffer, uint32_t offset, uint32_t size,
                                                   const Location &loc) const {
    bool skip = false;
    const bool is_2 = loc.function != Func::vkCmdPushConstants;
    const uint32_t max_push_constants_size = device_limits.maxPushConstantsSize;
    // Check that offset + size don't exceed the max.
    // Prevent arithetic overflow here by avoiding addition and testing in this order.
    if (offset >= max_push_constants_size) {
        const char *vuid = is_2 ? "VUID-VkPushConstantsInfoKHR-offset-00370" : "VUID-vkCmdPushConstants-offset-00370";
        skip |= LogError(vuid, commandBuffer, loc.dot(Field::offset),
                         "(%" PRIu32 ") that exceeds this device's maxPushConstantSize of %" PRIu32 ".", offset,
                         max_push_constants_size);
    }
    if (size > max_push_constants_size - offset) {
        const char *vuid = is_2 ? "VUID-VkPushConstantsInfoKHR-size-00371" : "VUID-vkCmdPushConstants-size-00371";
        skip |= LogError(vuid, commandBuffer, loc.dot(Field::offset),
                         "(%" PRIu32 ") and size (%" PRIu32 ") that exceeds this device's maxPushConstantSize of %" PRIu32 ".",
                         offset, size, max_push_constants_size);
    }

    if (SafeModulo(size, 4) != 0) {
        const char *vuid = is_2 ? "VUID-VkPushConstantsInfoKHR-size-00369" : "VUID-vkCmdPushConstants-size-00369";
        skip |= LogError(vuid, commandBuffer, loc.dot(Field::size), "(%" PRIu32 ") must be a multiple of 4.", size);
    }

    if (SafeModulo(offset, 4) != 0) {
        const char *vuid = is_2 ? "VUID-VkPushConstantsInfoKHR-offset-00368" : "VUID-vkCmdPushConstants-offset-00368";
        skip |= LogError(vuid, commandBuffer, loc.dot(Field::offset), "(%" PRIu32 ") must be a multiple of 4.", offset);
    }
    return skip;
}

bool StatelessValidation::manual_PreCallValidateCmdPushConstants(VkCommandBuffer commandBuffer, VkPipelineLayout layout,
                                                                 VkShaderStageFlags stageFlags, uint32_t offset, uint32_t size,
                                                                 const void *pValues, const ErrorObject &error_obj) const {
    return ValidateCmdPushConstants(commandBuffer, offset, size, error_obj.location);
}

bool StatelessValidation::manual_PreCallValidateCmdPushConstants2KHR(VkCommandBuffer commandBuffer,
                                                                     const VkPushConstantsInfoKHR *pPushConstantsInfo,
                                                                     const ErrorObject &error_obj) const {
    bool skip = false;
    skip |= ValidateCmdPushConstants(commandBuffer, pPushConstantsInfo->offset, pPushConstantsInfo->size,
                                     error_obj.location.dot(Field::pPushConstantsInfo));
    if (pPushConstantsInfo->layout == VK_NULL_HANDLE &&
        !vku::FindStructInPNextChain<VkPipelineLayoutCreateInfo>(pPushConstantsInfo->pNext)) {
        skip |= LogError("VUID-VkPushConstantsInfoKHR-layout-09496", commandBuffer,
                         error_obj.location.dot(Field::pPushConstantsInfo).dot(Field::layout),
                         "is VK_NULL_HANDLE and pNext is missing VkPipelineLayoutCreateInfo.");
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

bool StatelessValidation::manual_PreCallValidateGetQueryPoolResults(VkDevice device, VkQueryPool queryPool, uint32_t firstQuery,
                                                                    uint32_t queryCount, size_t dataSize, void *pData,
                                                                    VkDeviceSize stride, VkQueryResultFlags flags,
                                                                    const ErrorObject &error_obj) const {
    bool skip = false;

    if ((flags & VK_QUERY_RESULT_WITH_STATUS_BIT_KHR) && (flags & VK_QUERY_RESULT_WITH_AVAILABILITY_BIT)) {
        skip |= LogError("VUID-vkGetQueryPoolResults-flags-09443", device, error_obj.location.dot(Field::flags),
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
        const LogObjectList objlist(commandBuffer, dstBuffer);
        skip |= LogError("VUID-vkCmdUpdateBuffer-dstOffset-00036", objlist, error_obj.location.dot(Field::dstOffset),
                         "(%" PRIu64 "), is not a multiple of 4.", dstOffset);
    }

    if ((dataSize <= 0) || (dataSize > 65536)) {
        const LogObjectList objlist(commandBuffer, dstBuffer);
        skip |= LogError("VUID-vkCmdUpdateBuffer-dataSize-00037", objlist, error_obj.location.dot(Field::dataSize),
                         "(%" PRIu64 "), must be greater than zero and less than or equal to 65536.", dataSize);
    } else if (dataSize & 3) {
        const LogObjectList objlist(commandBuffer, dstBuffer);
        skip |= LogError("VUID-vkCmdUpdateBuffer-dataSize-00038", objlist, error_obj.location.dot(Field::dataSize),
                         "(%" PRIu64 "), is not a multiple of 4.", dataSize);
    }
    return skip;
}

bool StatelessValidation::manual_PreCallValidateCmdFillBuffer(VkCommandBuffer commandBuffer, VkBuffer dstBuffer,
                                                              VkDeviceSize dstOffset, VkDeviceSize size, uint32_t data,
                                                              const ErrorObject &error_obj) const {
    bool skip = false;

    if (dstOffset & 3) {
        const LogObjectList objlist(commandBuffer, dstBuffer);
        skip |= LogError("VUID-vkCmdFillBuffer-dstOffset-00025", objlist, error_obj.location.dot(Field::dstOffset),
                         "(%" PRIu64 ") is not a multiple of 4.", dstOffset);
    }

    if (size != VK_WHOLE_SIZE) {
        if (size <= 0) {
            const LogObjectList objlist(commandBuffer, dstBuffer);
            skip |= LogError("VUID-vkCmdFillBuffer-size-00026", objlist, error_obj.location.dot(Field::size),
                             "(%" PRIu64 ") must be greater than zero.", size);
        } else if (size & 3) {
            const LogObjectList objlist(commandBuffer, dstBuffer);
            skip |= LogError("VUID-vkCmdFillBuffer-size-00028", objlist, error_obj.location.dot(Field::size),
                             "(%" PRIu64 ") is not a multiple of 4.", size);
        }
    }
    return skip;
}

bool StatelessValidation::manual_PreCallValidateCmdBindDescriptorBuffersEXT(VkCommandBuffer commandBuffer, uint32_t bufferCount,
                                                                            const VkDescriptorBufferBindingInfoEXT *pBindingInfos,
                                                                            const ErrorObject &error_obj) const {
    bool skip = false;

    for (uint32_t i = 0; i < bufferCount; i++) {
        if (!vku::FindStructInPNextChain<VkBufferUsageFlags2CreateInfoKHR>(pBindingInfos[i].pNext)) {
            skip |= ValidateFlags(error_obj.location.dot(Field::pBindingInfos, i).dot(Field::usage),
                                  vvl::FlagBitmask::VkBufferUsageFlagBits, AllVkBufferUsageFlagBits, pBindingInfos[i].usage,
                                  kRequiredFlags, "VUID-VkDescriptorBufferBindingInfoEXT-None-09499",
                                  "VUID-VkDescriptorBufferBindingInfoEXT-None-09500");
        }
    }

    return skip;
}

bool StatelessValidation::manual_PreCallValidateGetPhysicalDeviceExternalBufferProperties(
    VkPhysicalDevice physicalDevice, const VkPhysicalDeviceExternalBufferInfo *pExternalBufferInfo,
    VkExternalBufferProperties *pExternalBufferProperties, const ErrorObject &error_obj) const {
    bool skip = false;

    if (!vku::FindStructInPNextChain<VkBufferUsageFlags2CreateInfoKHR>(pExternalBufferInfo->pNext)) {
        skip |= ValidateFlags(error_obj.location.dot(Field::pExternalBufferInfo).dot(Field::usage),
                              vvl::FlagBitmask::VkBufferUsageFlagBits, AllVkBufferUsageFlagBits, pExternalBufferInfo->usage,
                              kRequiredFlags, "VUID-VkPhysicalDeviceExternalBufferInfo-None-09499",
                              "VUID-VkPhysicalDeviceExternalBufferInfo-None-09500");
    }

    return skip;
}

bool StatelessValidation::manual_PreCallValidateCmdPushDescriptorSetKHR(
    VkCommandBuffer commandBuffer, VkPipelineBindPoint pipelineBindPoint, VkPipelineLayout layout, uint32_t set,
    uint32_t descriptorWriteCount, const VkWriteDescriptorSet *pDescriptorWrites, const ErrorObject &error_obj) const {
    return ValidateWriteDescriptorSet(error_obj.location, descriptorWriteCount, pDescriptorWrites);
}

bool StatelessValidation::manual_PreCallValidateCmdPushDescriptorSet2KHR(VkCommandBuffer commandBuffer,
                                                                         const VkPushDescriptorSetInfoKHR *pPushDescriptorSetInfo,
                                                                         const ErrorObject &error_obj) const {
    bool skip = false;
    skip |= ValidateWriteDescriptorSet(error_obj.location, pPushDescriptorSetInfo->descriptorWriteCount,
                                       pPushDescriptorSetInfo->pDescriptorWrites);
    if (pPushDescriptorSetInfo->layout == VK_NULL_HANDLE &&
        !vku::FindStructInPNextChain<VkPipelineLayoutCreateInfo>(pPushDescriptorSetInfo->pNext)) {
        skip |= LogError("VUID-VkPushDescriptorSetInfoKHR-layout-09496", commandBuffer,
                         error_obj.location.dot(Field::pPushDescriptorSetInfo).dot(Field::layout),
                         "is VK_NULL_HANDLE and pNext is missing VkPipelineLayoutCreateInfo.");
    }
    return skip;
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
    if (!error_obj.handle_data->command_buffer.is_secondary) {
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
                                                VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_VIEWPORT_SCISSOR_INFO_NV,
                                                VK_STRUCTURE_TYPE_RENDERING_INPUT_ATTACHMENT_INDEX_INFO_KHR,
                                                VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_LOCATION_INFO_KHR};
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
            skip |= ValidateFlags(inheritance_loc.dot(Field::queryFlags), vvl::FlagBitmask::VkQueryControlFlagBits,
                                  AllVkQueryControlFlagBits, info->queryFlags, kOptionalFlags,
                                  "VUID-VkCommandBufferInheritanceInfo-queryFlags-00057");
        } else {  // !inheritedQueries
            skip |= ValidateReservedFlags(inheritance_loc.dot(Field::queryFlags), info->queryFlags,
                                          "VUID-VkCommandBufferInheritanceInfo-queryFlags-02788");
        }

        if (physical_device_features.pipelineStatisticsQuery) {
            skip |=
                ValidateFlags(inheritance_loc.dot(Field::pipelineStatistics), vvl::FlagBitmask::VkQueryPipelineStatisticFlagBits,
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
