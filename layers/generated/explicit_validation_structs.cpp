/* *** THIS FILE IS GENERATED - DO NOT EDIT! ***
 * See parameter_validation_generator.py for modifications
 *
 * Copyright (c) 2023-2023 The Khronos Group Inc.
 * Copyright (c) 2023-2023 LunarG, Inc.
 * Copyright (C) 2023-2023 Google Inc.
 * Copyright (c) 2023-2023 Valve Corporation
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


#include "chassis.h"

#include "explicit/explicit_validation.h"

bool ExplicitValidation::ValidateExtent2D(const LogObjectList &_parentObjects,
                const uint32_t width,
                const uint32_t height) const {
    bool skip = false;
    return skip;
}
bool ExplicitValidation::ValidateExtent3D(const LogObjectList &_parentObjects,
                const uint32_t width,
                const uint32_t height,
                const uint32_t depth) const {
    bool skip = false;
    return skip;
}
bool ExplicitValidation::ValidateOffset2D(const LogObjectList &_parentObjects,
                const int32_t x,
                const int32_t y) const {
    bool skip = false;
    return skip;
}
bool ExplicitValidation::ValidateOffset3D(const LogObjectList &_parentObjects,
                const int32_t x,
                const int32_t y,
                const int32_t z) const {
    bool skip = false;
    return skip;
}
bool ExplicitValidation::ValidateRect2D(const LogObjectList &_parentObjects,
                const VkOffset2D offset,
                const VkExtent2D extent) const {
    bool skip = false;
    const auto _s1 = &offset;
    skip |= ValidateOffset2D(_parentObjects,
        _s1->x,
        _s1->y);
    const auto _s2 = &extent;
    skip |= ValidateExtent2D(_parentObjects,
        _s2->width,
        _s2->height);
    return skip;
}
bool ExplicitValidation::ValidateBaseInStructure(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const struct VkBaseInStructure* pNext) const {
    bool skip = false;
    if (pNext != nullptr) {
        const auto _s3 = pNext;
        skip |= ValidateBaseInStructure(_parentObjects,
            _s3->sType,
            _s3->pNext);
    }
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
bool ExplicitValidation::ValidateBaseOutStructure(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const struct VkBaseOutStructure* pNext) const {
    bool skip = false;
    if (pNext != nullptr) {
        const auto _s4 = pNext;
        skip |= ValidateBaseOutStructure(_parentObjects,
            _s4->sType,
            _s4->pNext);
    }
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
bool ExplicitValidation::ValidateBufferMemoryBarrier(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const VkAccessFlags srcAccessMask,
                const VkAccessFlags dstAccessMask,
                const uint32_t srcQueueFamilyIndex,
                const uint32_t dstQueueFamilyIndex,
                const VkBuffer buffer,
                const VkDeviceSize offset,
                const VkDeviceSize size) const {
    bool skip = false;
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
bool ExplicitValidation::ValidateDispatchIndirectCommand(const LogObjectList &_parentObjects,
                const uint32_t x,
                const uint32_t y,
                const uint32_t z) const {
    bool skip = false;
    return skip;
}
bool ExplicitValidation::ValidateDrawIndexedIndirectCommand(const LogObjectList &_parentObjects,
                const uint32_t indexCount,
                const uint32_t instanceCount,
                const uint32_t firstIndex,
                const int32_t vertexOffset,
                const uint32_t firstInstance) const {
    bool skip = false;
    return skip;
}
bool ExplicitValidation::ValidateDrawIndirectCommand(const LogObjectList &_parentObjects,
                const uint32_t vertexCount,
                const uint32_t instanceCount,
                const uint32_t firstVertex,
                const uint32_t firstInstance) const {
    bool skip = false;
    return skip;
}
bool ExplicitValidation::ValidateImageSubresourceRange(const LogObjectList &_parentObjects,
                const VkImageAspectFlags aspectMask,
                const uint32_t baseMipLevel,
                const uint32_t levelCount,
                const uint32_t baseArrayLayer,
                const uint32_t layerCount) const {
    bool skip = false;
    return skip;
}
bool ExplicitValidation::ValidateImageMemoryBarrier(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const VkAccessFlags srcAccessMask,
                const VkAccessFlags dstAccessMask,
                const VkImageLayout oldLayout,
                const VkImageLayout newLayout,
                const uint32_t srcQueueFamilyIndex,
                const uint32_t dstQueueFamilyIndex,
                const VkImage image,
                const VkImageSubresourceRange subresourceRange) const {
    bool skip = false;
    const auto _s5 = &subresourceRange;
    skip |= ValidateImageSubresourceRange(_parentObjects,
        _s5->aspectMask,
        _s5->baseMipLevel,
        _s5->levelCount,
        _s5->baseArrayLayer,
        _s5->layerCount);
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
bool ExplicitValidation::ValidateMemoryBarrier(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const VkAccessFlags srcAccessMask,
                const VkAccessFlags dstAccessMask) const {
    bool skip = false;
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
bool ExplicitValidation::ValidatePipelineCacheHeaderVersionOne(const LogObjectList &_parentObjects,
                const uint32_t headerSize,
                const VkPipelineCacheHeaderVersion headerVersion,
                const uint32_t vendorID,
                const uint32_t deviceID,
                const uint8_t pipelineCacheUUID[VK_UUID_SIZE]) const {
    bool skip = false;
    return skip;
}
bool ExplicitValidation::ValidateAllocationCallbacks(const LogObjectList &_parentObjects,
                const void* pUserData,
                const PFN_vkAllocationFunction pfnAllocation,
                const PFN_vkReallocationFunction pfnReallocation,
                const PFN_vkFreeFunction pfnFree,
                const PFN_vkInternalAllocationNotification pfnInternalAllocation,
                const PFN_vkInternalFreeNotification pfnInternalFree) const {
    bool skip = false;
    return skip;
}
bool ExplicitValidation::ValidateApplicationInfo(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const char* pApplicationName,
                const uint32_t applicationVersion,
                const char* pEngineName,
                const uint32_t engineVersion,
                const uint32_t apiVersion) const {
    bool skip = false;
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
bool ExplicitValidation::ValidateInstanceCreateInfo(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const VkInstanceCreateFlags flags,
                const VkApplicationInfo* pApplicationInfo,
                const uint32_t enabledLayerCount,
                const char* const* ppEnabledLayerNames,
                const uint32_t enabledExtensionCount,
                const char* const* ppEnabledExtensionNames) const {
    bool skip = false;
    if (pApplicationInfo != nullptr) {
        const auto _s6 = pApplicationInfo;
        skip |= ValidateApplicationInfo(_parentObjects,
            _s6->sType,
            _s6->pNext,
            _s6->pApplicationName,
            _s6->applicationVersion,
            _s6->pEngineName,
            _s6->engineVersion,
            _s6->apiVersion);
    }
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
bool ExplicitValidation::ValidatePhysicalDeviceFeatures(const LogObjectList &_parentObjects,
                const VkBool32 robustBufferAccess,
                const VkBool32 fullDrawIndexUint32,
                const VkBool32 imageCubeArray,
                const VkBool32 independentBlend,
                const VkBool32 geometryShader,
                const VkBool32 tessellationShader,
                const VkBool32 sampleRateShading,
                const VkBool32 dualSrcBlend,
                const VkBool32 logicOp,
                const VkBool32 multiDrawIndirect,
                const VkBool32 drawIndirectFirstInstance,
                const VkBool32 depthClamp,
                const VkBool32 depthBiasClamp,
                const VkBool32 fillModeNonSolid,
                const VkBool32 depthBounds,
                const VkBool32 wideLines,
                const VkBool32 largePoints,
                const VkBool32 alphaToOne,
                const VkBool32 multiViewport,
                const VkBool32 samplerAnisotropy,
                const VkBool32 textureCompressionETC2,
                const VkBool32 textureCompressionASTC_LDR,
                const VkBool32 textureCompressionBC,
                const VkBool32 occlusionQueryPrecise,
                const VkBool32 pipelineStatisticsQuery,
                const VkBool32 vertexPipelineStoresAndAtomics,
                const VkBool32 fragmentStoresAndAtomics,
                const VkBool32 shaderTessellationAndGeometryPointSize,
                const VkBool32 shaderImageGatherExtended,
                const VkBool32 shaderStorageImageExtendedFormats,
                const VkBool32 shaderStorageImageMultisample,
                const VkBool32 shaderStorageImageReadWithoutFormat,
                const VkBool32 shaderStorageImageWriteWithoutFormat,
                const VkBool32 shaderUniformBufferArrayDynamicIndexing,
                const VkBool32 shaderSampledImageArrayDynamicIndexing,
                const VkBool32 shaderStorageBufferArrayDynamicIndexing,
                const VkBool32 shaderStorageImageArrayDynamicIndexing,
                const VkBool32 shaderClipDistance,
                const VkBool32 shaderCullDistance,
                const VkBool32 shaderFloat64,
                const VkBool32 shaderInt64,
                const VkBool32 shaderInt16,
                const VkBool32 shaderResourceResidency,
                const VkBool32 shaderResourceMinLod,
                const VkBool32 sparseBinding,
                const VkBool32 sparseResidencyBuffer,
                const VkBool32 sparseResidencyImage2D,
                const VkBool32 sparseResidencyImage3D,
                const VkBool32 sparseResidency2Samples,
                const VkBool32 sparseResidency4Samples,
                const VkBool32 sparseResidency8Samples,
                const VkBool32 sparseResidency16Samples,
                const VkBool32 sparseResidencyAliased,
                const VkBool32 variableMultisampleRate,
                const VkBool32 inheritedQueries) const {
    bool skip = false;
    return skip;
}
bool ExplicitValidation::ValidateDeviceQueueCreateInfo(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const VkDeviceQueueCreateFlags flags,
                const uint32_t queueFamilyIndex,
                const uint32_t queueCount,
                const float* pQueuePriorities) const {
    bool skip = false;
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
bool ExplicitValidation::ValidateDeviceCreateInfo(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const VkDeviceCreateFlags flags,
                const uint32_t queueCreateInfoCount,
                const VkDeviceQueueCreateInfo* pQueueCreateInfos,
                const uint32_t enabledLayerCount,
                const char* const* ppEnabledLayerNames,
                const uint32_t enabledExtensionCount,
                const char* const* ppEnabledExtensionNames,
                const VkPhysicalDeviceFeatures* pEnabledFeatures) const {
    bool skip = false;

    {
        for (uint32_t i2 = 0; i2 < queueCreateInfoCount; ++i2)
        {
            const VkDeviceQueueCreateInfo &queue_0 = pQueueCreateInfos[i2];
            {
                for (uint32_t i4 = 0; i4 < queueCreateInfoCount; ++i4)
                {
                    const VkDeviceQueueCreateInfo &queue2_1 = pQueueCreateInfos[i4];
                    {
                        if ((i4 >= i2))
                        {
                            break;
                        }

                        if (!((queue_0.queueFamilyIndex != queue2_1.queueFamilyIndex)))
                        {
                            const LogObjectList objlist{};
                            skip |= LogFail(objlist, "VUID-VkDeviceCreateInfo-queueFamilyIndex-00372",
                                R"(    ${vu-keyword}for${} queue${vu-value}{{@%)" PRIu32 R"(}}${} ${vu-operator}in${} pQueueCreateInfos:
      ${vu-keyword}for${} queue2${vu-value}{{@%)" PRIu32 R"(}}${} ${vu-operator}in${} pQueueCreateInfos:
        ${vu-keyword}if${} ${vu-builtin}loop_index${}(queue2${vu-value}{{@%)" PRIu32 R"(}}${}) ${vu-operator}>=${} ${vu-builtin}loop_index${}(queue${vu-value}{{@%)" PRIu32 R"(}}${}):
          ${vu-keyword}break${}
        ${vu-builtin}require${}(queue${vu-value}{{@%)" PRIu32 R"(}}${}.queueFamilyIndex${vu-value}{{%)" PRIu32 R"(}}${} ${vu-operator}!=${} queue2${vu-value}{{@%)" PRIu32 R"(}}${}.queueFamilyIndex${vu-value}{{%)" PRIu32 R"(}}${})${vu-fail} <-- Failing condition (Values captured at this instant)${})",
                                i2,
                                i4,
                                i4,
                                i2,
                                i2,
                                queue_0.queueFamilyIndex,
                                i4,
                                queue2_1.queueFamilyIndex);
                        }

                    }
                }

                for (uint32_t i4 = 0; i4 < queueCreateInfoCount; ++i4)
                {
                    const VkDeviceQueueCreateInfo &queue2_3 = pQueueCreateInfos[i4];
                    {
                        if ((i4 >= i2))
                        {
                            break;
                        }

                        bool protected_4=Builtin_has_bit(queue_0.flags, VK_DEVICE_QUEUE_CREATE_PROTECTED_BIT);
                        bool protected2_5=Builtin_has_bit(queue2_3.flags, VK_DEVICE_QUEUE_CREATE_PROTECTED_BIT);
                        if (!(((queue_0.queueFamilyIndex != queue2_3.queueFamilyIndex)||(protected_4 != protected2_5))))
                        {
                            const LogObjectList objlist{};
                            skip |= LogFail(objlist, "VUID-VkDeviceCreateInfo-queueFamilyIndex-02802",
                                R"(    ${vu-keyword}for${} queue${vu-value}{{@%)" PRIu32 R"(}}${} ${vu-operator}in${} pQueueCreateInfos:
      ${vu-keyword}for${} queue2${vu-value}{{@%)" PRIu32 R"(}}${} ${vu-operator}in${} pQueueCreateInfos:
        ${vu-keyword}if${} ${vu-builtin}loop_index${}(queue2${vu-value}{{@%)" PRIu32 R"(}}${}) ${vu-operator}>=${} ${vu-builtin}loop_index${}(queue${vu-value}{{@%)" PRIu32 R"(}}${}):
          ${vu-keyword}break${}
        protected${vu-value}{{%s}}${} ${vu-operator}=${} queue${vu-value}{{@%)" PRIu32 R"(}}${}.flags${vu-value}{{%#)" PRIx32 R"(}}${}.${vu-builtin}has_bit${}(VK_DEVICE_QUEUE_CREATE_PROTECTED_BIT)${vu-value}{{%s}}${}
        protected2${vu-value}{{%s}}${} ${vu-operator}=${} queue2${vu-value}{{@%)" PRIu32 R"(}}${}.flags${vu-value}{{%#)" PRIx32 R"(}}${}.${vu-builtin}has_bit${}(VK_DEVICE_QUEUE_CREATE_PROTECTED_BIT)${vu-value}{{%s}}${}
        ${vu-builtin}require${}(queue${vu-value}{{@%)" PRIu32 R"(}}${}.queueFamilyIndex${vu-value}{{%)" PRIu32 R"(}}${} ${vu-operator}!=${} queue2${vu-value}{{@%)" PRIu32 R"(}}${}.queueFamilyIndex${vu-value}{{%)" PRIu32 R"(}}${} ${vu-operator}or${}
                protected${vu-value}{{%s}}${} ${vu-operator}!=${} protected2${vu-value}{{%s}}${})${vu-fail} <-- Failing condition (Values captured at this instant)${})",
                                i2,
                                i4,
                                i4,
                                i2,
                                (protected_4) ? "true" : "false",
                                i2,
                                queue_0.flags,
                                (Builtin_has_bit(queue_0.flags, VK_DEVICE_QUEUE_CREATE_PROTECTED_BIT)) ? "true" : "false",
                                (protected2_5) ? "true" : "false",
                                i4,
                                queue2_3.flags,
                                (Builtin_has_bit(queue2_3.flags, VK_DEVICE_QUEUE_CREATE_PROTECTED_BIT)) ? "true" : "false",
                                i2,
                                queue_0.queueFamilyIndex,
                                i4,
                                queue2_3.queueFamilyIndex,
                                (protected_4) ? "true" : "false",
                                (protected2_5) ? "true" : "false");
                        }

                    }
                }

            }
        }

    }
    if (pQueueCreateInfos != nullptr) {
        for (uint32_t _i7 = 0;_i7 < queueCreateInfoCount; ++_i7) {
            const auto _s8 = &pQueueCreateInfos[_i7];
            skip |= ValidateDeviceQueueCreateInfo(_parentObjects,
                _s8->sType,
                _s8->pNext,
                _s8->flags,
                _s8->queueFamilyIndex,
                _s8->queueCount,
                _s8->pQueuePriorities);
        }
    }
    if (pEnabledFeatures != nullptr) {
        const auto _s9 = pEnabledFeatures;
        skip |= ValidatePhysicalDeviceFeatures(_parentObjects,
            _s9->robustBufferAccess,
            _s9->fullDrawIndexUint32,
            _s9->imageCubeArray,
            _s9->independentBlend,
            _s9->geometryShader,
            _s9->tessellationShader,
            _s9->sampleRateShading,
            _s9->dualSrcBlend,
            _s9->logicOp,
            _s9->multiDrawIndirect,
            _s9->drawIndirectFirstInstance,
            _s9->depthClamp,
            _s9->depthBiasClamp,
            _s9->fillModeNonSolid,
            _s9->depthBounds,
            _s9->wideLines,
            _s9->largePoints,
            _s9->alphaToOne,
            _s9->multiViewport,
            _s9->samplerAnisotropy,
            _s9->textureCompressionETC2,
            _s9->textureCompressionASTC_LDR,
            _s9->textureCompressionBC,
            _s9->occlusionQueryPrecise,
            _s9->pipelineStatisticsQuery,
            _s9->vertexPipelineStoresAndAtomics,
            _s9->fragmentStoresAndAtomics,
            _s9->shaderTessellationAndGeometryPointSize,
            _s9->shaderImageGatherExtended,
            _s9->shaderStorageImageExtendedFormats,
            _s9->shaderStorageImageMultisample,
            _s9->shaderStorageImageReadWithoutFormat,
            _s9->shaderStorageImageWriteWithoutFormat,
            _s9->shaderUniformBufferArrayDynamicIndexing,
            _s9->shaderSampledImageArrayDynamicIndexing,
            _s9->shaderStorageBufferArrayDynamicIndexing,
            _s9->shaderStorageImageArrayDynamicIndexing,
            _s9->shaderClipDistance,
            _s9->shaderCullDistance,
            _s9->shaderFloat64,
            _s9->shaderInt64,
            _s9->shaderInt16,
            _s9->shaderResourceResidency,
            _s9->shaderResourceMinLod,
            _s9->sparseBinding,
            _s9->sparseResidencyBuffer,
            _s9->sparseResidencyImage2D,
            _s9->sparseResidencyImage3D,
            _s9->sparseResidency2Samples,
            _s9->sparseResidency4Samples,
            _s9->sparseResidency8Samples,
            _s9->sparseResidency16Samples,
            _s9->sparseResidencyAliased,
            _s9->variableMultisampleRate,
            _s9->inheritedQueries);
    }
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
bool ExplicitValidation::ValidateSubmitInfo(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const uint32_t waitSemaphoreCount,
                const VkSemaphore* pWaitSemaphores,
                const VkPipelineStageFlags* pWaitDstStageMask,
                const uint32_t commandBufferCount,
                const VkCommandBuffer* pCommandBuffers,
                const uint32_t signalSemaphoreCount,
                const VkSemaphore* pSignalSemaphores) const {
    bool skip = false;
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
bool ExplicitValidation::ValidateMappedMemoryRange(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const VkDeviceMemory memory,
                const VkDeviceSize offset,
                const VkDeviceSize size) const {
    bool skip = false;
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
bool ExplicitValidation::ValidateMemoryAllocateInfo(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const VkDeviceSize allocationSize,
                const uint32_t memoryTypeIndex) const {
    bool skip = false;
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
bool ExplicitValidation::ValidateSparseMemoryBind(const LogObjectList &_parentObjects,
                const VkDeviceSize resourceOffset,
                const VkDeviceSize size,
                const VkDeviceMemory memory,
                const VkDeviceSize memoryOffset,
                const VkSparseMemoryBindFlags flags) const {
    bool skip = false;
    return skip;
}
bool ExplicitValidation::ValidateSparseBufferMemoryBindInfo(const LogObjectList &_parentObjects,
                const VkBuffer buffer,
                const uint32_t bindCount,
                const VkSparseMemoryBind* pBinds) const {
    bool skip = false;
    if (pBinds != nullptr) {
        for (uint32_t _i10 = 0;_i10 < bindCount; ++_i10) {
            const auto _s11 = &pBinds[_i10];
            skip |= ValidateSparseMemoryBind(_parentObjects,
                _s11->resourceOffset,
                _s11->size,
                _s11->memory,
                _s11->memoryOffset,
                _s11->flags);
        }
    }
    return skip;
}
bool ExplicitValidation::ValidateSparseImageOpaqueMemoryBindInfo(const LogObjectList &_parentObjects,
                const VkImage image,
                const uint32_t bindCount,
                const VkSparseMemoryBind* pBinds) const {
    bool skip = false;
    if (pBinds != nullptr) {
        for (uint32_t _i12 = 0;_i12 < bindCount; ++_i12) {
            const auto _s13 = &pBinds[_i12];
            skip |= ValidateSparseMemoryBind(_parentObjects,
                _s13->resourceOffset,
                _s13->size,
                _s13->memory,
                _s13->memoryOffset,
                _s13->flags);
        }
    }
    return skip;
}
bool ExplicitValidation::ValidateImageSubresource(const LogObjectList &_parentObjects,
                const VkImageAspectFlags aspectMask,
                const uint32_t mipLevel,
                const uint32_t arrayLayer) const {
    bool skip = false;
    return skip;
}
bool ExplicitValidation::ValidateSparseImageMemoryBind(const LogObjectList &_parentObjects,
                const VkImageSubresource subresource,
                const VkOffset3D offset,
                const VkExtent3D extent,
                const VkDeviceMemory memory,
                const VkDeviceSize memoryOffset,
                const VkSparseMemoryBindFlags flags) const {
    bool skip = false;
    const auto _s14 = &subresource;
    skip |= ValidateImageSubresource(_parentObjects,
        _s14->aspectMask,
        _s14->mipLevel,
        _s14->arrayLayer);
    const auto _s15 = &offset;
    skip |= ValidateOffset3D(_parentObjects,
        _s15->x,
        _s15->y,
        _s15->z);
    const auto _s16 = &extent;
    skip |= ValidateExtent3D(_parentObjects,
        _s16->width,
        _s16->height,
        _s16->depth);
    return skip;
}
bool ExplicitValidation::ValidateSparseImageMemoryBindInfo(const LogObjectList &_parentObjects,
                const VkImage image,
                const uint32_t bindCount,
                const VkSparseImageMemoryBind* pBinds) const {
    bool skip = false;
    if (pBinds != nullptr) {
        for (uint32_t _i17 = 0;_i17 < bindCount; ++_i17) {
            const auto _s18 = &pBinds[_i17];
            skip |= ValidateSparseImageMemoryBind(_parentObjects,
                _s18->subresource,
                _s18->offset,
                _s18->extent,
                _s18->memory,
                _s18->memoryOffset,
                _s18->flags);
        }
    }
    return skip;
}
bool ExplicitValidation::ValidateBindSparseInfo(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const uint32_t waitSemaphoreCount,
                const VkSemaphore* pWaitSemaphores,
                const uint32_t bufferBindCount,
                const VkSparseBufferMemoryBindInfo* pBufferBinds,
                const uint32_t imageOpaqueBindCount,
                const VkSparseImageOpaqueMemoryBindInfo* pImageOpaqueBinds,
                const uint32_t imageBindCount,
                const VkSparseImageMemoryBindInfo* pImageBinds,
                const uint32_t signalSemaphoreCount,
                const VkSemaphore* pSignalSemaphores) const {
    bool skip = false;
    if (pBufferBinds != nullptr) {
        for (uint32_t _i19 = 0;_i19 < bufferBindCount; ++_i19) {
            const auto _s20 = &pBufferBinds[_i19];
            skip |= ValidateSparseBufferMemoryBindInfo(_parentObjects,
                _s20->buffer,
                _s20->bindCount,
                _s20->pBinds);
        }
    }
    if (pImageOpaqueBinds != nullptr) {
        for (uint32_t _i21 = 0;_i21 < imageOpaqueBindCount; ++_i21) {
            const auto _s22 = &pImageOpaqueBinds[_i21];
            skip |= ValidateSparseImageOpaqueMemoryBindInfo(_parentObjects,
                _s22->image,
                _s22->bindCount,
                _s22->pBinds);
        }
    }
    if (pImageBinds != nullptr) {
        for (uint32_t _i23 = 0;_i23 < imageBindCount; ++_i23) {
            const auto _s24 = &pImageBinds[_i23];
            skip |= ValidateSparseImageMemoryBindInfo(_parentObjects,
                _s24->image,
                _s24->bindCount,
                _s24->pBinds);
        }
    }
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
bool ExplicitValidation::ValidateFenceCreateInfo(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const VkFenceCreateFlags flags) const {
    bool skip = false;
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
bool ExplicitValidation::ValidateSemaphoreCreateInfo(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const VkSemaphoreCreateFlags flags) const {
    bool skip = false;
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
bool ExplicitValidation::ValidateEventCreateInfo(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const VkEventCreateFlags flags) const {
    bool skip = false;
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
bool ExplicitValidation::ValidateQueryPoolCreateInfo(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const VkQueryPoolCreateFlags flags,
                const VkQueryType queryType,
                const uint32_t queryCount,
                const VkQueryPipelineStatisticFlags pipelineStatistics) const {
    bool skip = false;
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
bool ExplicitValidation::ValidateBufferCreateInfo(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const VkBufferCreateFlags flags,
                const VkDeviceSize size,
                const VkBufferUsageFlags usage,
                const VkSharingMode sharingMode,
                const uint32_t queueFamilyIndexCount,
                const uint32_t* pQueueFamilyIndices) const {
    bool skip = false;
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
bool ExplicitValidation::ValidateBufferViewCreateInfo(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const VkBufferViewCreateFlags flags,
                const VkBuffer buffer,
                const VkFormat format,
                const VkDeviceSize offset,
                const VkDeviceSize range) const {
    bool skip = false;
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
bool ExplicitValidation::ValidateImageCreateInfo(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const VkImageCreateFlags flags,
                const VkImageType imageType,
                const VkFormat format,
                const VkExtent3D extent,
                const uint32_t mipLevels,
                const uint32_t arrayLayers,
                const VkSampleCountFlagBits samples,
                const VkImageTiling tiling,
                const VkImageUsageFlags usage,
                const VkSharingMode sharingMode,
                const uint32_t queueFamilyIndexCount,
                const uint32_t* pQueueFamilyIndices,
                const VkImageLayout initialLayout) const {
    bool skip = false;
    const auto _s25 = &extent;
    skip |= ValidateExtent3D(_parentObjects,
        _s25->width,
        _s25->height,
        _s25->depth);
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
bool ExplicitValidation::ValidateSubresourceLayout(const LogObjectList &_parentObjects,
                const VkDeviceSize offset,
                const VkDeviceSize size,
                const VkDeviceSize rowPitch,
                const VkDeviceSize arrayPitch,
                const VkDeviceSize depthPitch) const {
    bool skip = false;
    return skip;
}
bool ExplicitValidation::ValidateComponentMapping(const LogObjectList &_parentObjects,
                const VkComponentSwizzle r,
                const VkComponentSwizzle g,
                const VkComponentSwizzle b,
                const VkComponentSwizzle a) const {
    bool skip = false;
    return skip;
}
bool ExplicitValidation::ValidateImageViewCreateInfo(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const VkImageViewCreateFlags flags,
                const VkImage image,
                const VkImageViewType viewType,
                const VkFormat format,
                const VkComponentMapping components,
                const VkImageSubresourceRange subresourceRange) const {
    bool skip = false;
    const auto _s26 = &components;
    skip |= ValidateComponentMapping(_parentObjects,
        _s26->r,
        _s26->g,
        _s26->b,
        _s26->a);
    const auto _s27 = &subresourceRange;
    skip |= ValidateImageSubresourceRange(_parentObjects,
        _s27->aspectMask,
        _s27->baseMipLevel,
        _s27->levelCount,
        _s27->baseArrayLayer,
        _s27->layerCount);
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
bool ExplicitValidation::ValidateShaderModuleCreateInfo(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const VkShaderModuleCreateFlags flags,
                const size_t codeSize,
                const uint32_t* pCode) const {
    bool skip = false;
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
bool ExplicitValidation::ValidatePipelineCacheCreateInfo(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const VkPipelineCacheCreateFlags flags,
                const size_t initialDataSize,
                const void* pInitialData) const {
    bool skip = false;
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
bool ExplicitValidation::ValidateSpecializationMapEntry(const LogObjectList &_parentObjects,
                uint32_t constantID,
                const uint32_t offset,
                const size_t size) const {
    bool skip = false;
    return skip;
}
bool ExplicitValidation::ValidateSpecializationInfo(const LogObjectList &_parentObjects,
                const uint32_t mapEntryCount,
                const VkSpecializationMapEntry* pMapEntries,
                const size_t dataSize,
                const void* pData) const {
    bool skip = false;
    if (pMapEntries != nullptr) {
        for (uint32_t _i28 = 0;_i28 < mapEntryCount; ++_i28) {
            const auto _s29 = &pMapEntries[_i28];
            skip |= ValidateSpecializationMapEntry(_parentObjects,
                _s29->constantID,
                _s29->offset,
                _s29->size);
        }
    }
    return skip;
}
bool ExplicitValidation::ValidatePipelineShaderStageCreateInfo(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const VkPipelineShaderStageCreateFlags flags,
                const VkShaderStageFlagBits stage,
                const VkShaderModule module,
                const char* pName,
                const VkSpecializationInfo* pSpecializationInfo) const {
    bool skip = false;
    if (pSpecializationInfo != nullptr) {
        const auto _s30 = pSpecializationInfo;
        skip |= ValidateSpecializationInfo(_parentObjects,
            _s30->mapEntryCount,
            _s30->pMapEntries,
            _s30->dataSize,
            _s30->pData);
    }
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
bool ExplicitValidation::ValidateComputePipelineCreateInfo(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const VkPipelineCreateFlags flags,
                const VkPipelineShaderStageCreateInfo stage,
                const VkPipelineLayout layout,
                const VkPipeline basePipelineHandle,
                const int32_t basePipelineIndex) const {
    bool skip = false;
    const auto _s31 = &stage;
    skip |= ValidatePipelineShaderStageCreateInfo(_parentObjects,
        _s31->sType,
        _s31->pNext,
        _s31->flags,
        _s31->stage,
        _s31->module,
        _s31->pName,
        _s31->pSpecializationInfo);
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
bool ExplicitValidation::ValidateVertexInputBindingDescription(const LogObjectList &_parentObjects,
                const uint32_t binding,
                const uint32_t stride,
                const VkVertexInputRate inputRate) const {
    bool skip = false;
    return skip;
}
bool ExplicitValidation::ValidateVertexInputAttributeDescription(const LogObjectList &_parentObjects,
                const uint32_t location,
                const uint32_t binding,
                const VkFormat format,
                const uint32_t offset) const {
    bool skip = false;
    return skip;
}
bool ExplicitValidation::ValidatePipelineVertexInputStateCreateInfo(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const VkPipelineVertexInputStateCreateFlags flags,
                const uint32_t vertexBindingDescriptionCount,
                const VkVertexInputBindingDescription* pVertexBindingDescriptions,
                const uint32_t vertexAttributeDescriptionCount,
                const VkVertexInputAttributeDescription* pVertexAttributeDescriptions) const {
    bool skip = false;
    if (pVertexBindingDescriptions != nullptr) {
        for (uint32_t _i32 = 0;_i32 < vertexBindingDescriptionCount; ++_i32) {
            const auto _s33 = &pVertexBindingDescriptions[_i32];
            skip |= ValidateVertexInputBindingDescription(_parentObjects,
                _s33->binding,
                _s33->stride,
                _s33->inputRate);
        }
    }
    if (pVertexAttributeDescriptions != nullptr) {
        for (uint32_t _i34 = 0;_i34 < vertexAttributeDescriptionCount; ++_i34) {
            const auto _s35 = &pVertexAttributeDescriptions[_i34];
            skip |= ValidateVertexInputAttributeDescription(_parentObjects,
                _s35->location,
                _s35->binding,
                _s35->format,
                _s35->offset);
        }
    }
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
bool ExplicitValidation::ValidatePipelineInputAssemblyStateCreateInfo(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const VkPipelineInputAssemblyStateCreateFlags flags,
                const VkPrimitiveTopology topology,
                const VkBool32 primitiveRestartEnable) const {
    bool skip = false;
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
bool ExplicitValidation::ValidatePipelineTessellationStateCreateInfo(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const VkPipelineTessellationStateCreateFlags flags,
                const uint32_t patchControlPoints) const {
    bool skip = false;
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
bool ExplicitValidation::ValidateViewport(const LogObjectList &_parentObjects,
                const float x,
                const float y,
                const float width,
                const float height,
                const float minDepth,
                const float maxDepth) const {
    bool skip = false;
    return skip;
}
bool ExplicitValidation::ValidatePipelineViewportStateCreateInfo(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const VkPipelineViewportStateCreateFlags flags,
                const uint32_t viewportCount,
                const VkViewport* pViewports,
                const uint32_t scissorCount,
                const VkRect2D* pScissors) const {
    bool skip = false;
    if (pViewports != nullptr) {
        for (uint32_t _i36 = 0;_i36 < viewportCount; ++_i36) {
            const auto _s37 = &pViewports[_i36];
            skip |= ValidateViewport(_parentObjects,
                _s37->x,
                _s37->y,
                _s37->width,
                _s37->height,
                _s37->minDepth,
                _s37->maxDepth);
        }
    }
    if (pScissors != nullptr) {
        for (uint32_t _i38 = 0;_i38 < scissorCount; ++_i38) {
            const auto _s39 = &pScissors[_i38];
            skip |= ValidateRect2D(_parentObjects,
                _s39->offset,
                _s39->extent);
        }
    }
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
bool ExplicitValidation::ValidatePipelineRasterizationStateCreateInfo(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const VkPipelineRasterizationStateCreateFlags flags,
                const VkBool32 depthClampEnable,
                const VkBool32 rasterizerDiscardEnable,
                const VkPolygonMode polygonMode,
                const VkCullModeFlags cullMode,
                const VkFrontFace frontFace,
                const VkBool32 depthBiasEnable,
                const float depthBiasConstantFactor,
                const float depthBiasClamp,
                const float depthBiasSlopeFactor,
                const float lineWidth) const {
    bool skip = false;
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
bool ExplicitValidation::ValidatePipelineMultisampleStateCreateInfo(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const VkPipelineMultisampleStateCreateFlags flags,
                const VkSampleCountFlagBits rasterizationSamples,
                const VkBool32 sampleShadingEnable,
                const float minSampleShading,
                const VkSampleMask* pSampleMask,
                const VkBool32 alphaToCoverageEnable,
                const VkBool32 alphaToOneEnable) const {
    bool skip = false;
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
bool ExplicitValidation::ValidateStencilOpState(const LogObjectList &_parentObjects,
                const VkStencilOp failOp,
                const VkStencilOp passOp,
                const VkStencilOp depthFailOp,
                const VkCompareOp compareOp,
                const uint32_t compareMask,
                const uint32_t writeMask,
                const uint32_t reference) const {
    bool skip = false;
    return skip;
}
bool ExplicitValidation::ValidatePipelineDepthStencilStateCreateInfo(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const VkPipelineDepthStencilStateCreateFlags flags,
                const VkBool32 depthTestEnable,
                const VkBool32 depthWriteEnable,
                const VkCompareOp depthCompareOp,
                const VkBool32 depthBoundsTestEnable,
                const VkBool32 stencilTestEnable,
                const VkStencilOpState front,
                const VkStencilOpState back,
                const float minDepthBounds,
                const float maxDepthBounds) const {
    bool skip = false;
    const auto _s40 = &front;
    skip |= ValidateStencilOpState(_parentObjects,
        _s40->failOp,
        _s40->passOp,
        _s40->depthFailOp,
        _s40->compareOp,
        _s40->compareMask,
        _s40->writeMask,
        _s40->reference);
    const auto _s41 = &back;
    skip |= ValidateStencilOpState(_parentObjects,
        _s41->failOp,
        _s41->passOp,
        _s41->depthFailOp,
        _s41->compareOp,
        _s41->compareMask,
        _s41->writeMask,
        _s41->reference);
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
bool ExplicitValidation::ValidatePipelineColorBlendAttachmentState(const LogObjectList &_parentObjects,
                const VkBool32 blendEnable,
                const VkBlendFactor srcColorBlendFactor,
                const VkBlendFactor dstColorBlendFactor,
                const VkBlendOp colorBlendOp,
                const VkBlendFactor srcAlphaBlendFactor,
                const VkBlendFactor dstAlphaBlendFactor,
                const VkBlendOp alphaBlendOp,
                const VkColorComponentFlags colorWriteMask) const {
    bool skip = false;
    return skip;
}
bool ExplicitValidation::ValidatePipelineColorBlendStateCreateInfo(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const VkPipelineColorBlendStateCreateFlags flags,
                const VkBool32 logicOpEnable,
                const VkLogicOp logicOp,
                const uint32_t attachmentCount,
                const VkPipelineColorBlendAttachmentState* pAttachments,
                const float blendConstants[4]) const {
    bool skip = false;
    if (pAttachments != nullptr) {
        for (uint32_t _i42 = 0;_i42 < attachmentCount; ++_i42) {
            const auto _s43 = &pAttachments[_i42];
            skip |= ValidatePipelineColorBlendAttachmentState(_parentObjects,
                _s43->blendEnable,
                _s43->srcColorBlendFactor,
                _s43->dstColorBlendFactor,
                _s43->colorBlendOp,
                _s43->srcAlphaBlendFactor,
                _s43->dstAlphaBlendFactor,
                _s43->alphaBlendOp,
                _s43->colorWriteMask);
        }
    }
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
bool ExplicitValidation::ValidatePipelineDynamicStateCreateInfo(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const VkPipelineDynamicStateCreateFlags flags,
                const uint32_t dynamicStateCount,
                const VkDynamicState* pDynamicStates) const {
    bool skip = false;
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
bool ExplicitValidation::ValidateGraphicsPipelineCreateInfo(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const VkPipelineCreateFlags flags,
                const uint32_t stageCount,
                const VkPipelineShaderStageCreateInfo* pStages,
                const VkPipelineVertexInputStateCreateInfo* pVertexInputState,
                const VkPipelineInputAssemblyStateCreateInfo* pInputAssemblyState,
                const VkPipelineTessellationStateCreateInfo* pTessellationState,
                const VkPipelineViewportStateCreateInfo* pViewportState,
                const VkPipelineRasterizationStateCreateInfo* pRasterizationState,
                const VkPipelineMultisampleStateCreateInfo* pMultisampleState,
                const VkPipelineDepthStencilStateCreateInfo* pDepthStencilState,
                const VkPipelineColorBlendStateCreateInfo* pColorBlendState,
                const VkPipelineDynamicStateCreateInfo* pDynamicState,
                const VkPipelineLayout layout,
                const VkRenderPass renderPass,
                const uint32_t subpass,
                const VkPipeline basePipelineHandle,
                const int32_t basePipelineIndex) const {
    bool skip = false;
    if (pStages != nullptr) {
        for (uint32_t _i44 = 0;_i44 < stageCount; ++_i44) {
            const auto _s45 = &pStages[_i44];
            skip |= ValidatePipelineShaderStageCreateInfo(_parentObjects,
                _s45->sType,
                _s45->pNext,
                _s45->flags,
                _s45->stage,
                _s45->module,
                _s45->pName,
                _s45->pSpecializationInfo);
        }
    }
    if (pVertexInputState != nullptr) {
        const auto _s46 = pVertexInputState;
        skip |= ValidatePipelineVertexInputStateCreateInfo(_parentObjects,
            _s46->sType,
            _s46->pNext,
            _s46->flags,
            _s46->vertexBindingDescriptionCount,
            _s46->pVertexBindingDescriptions,
            _s46->vertexAttributeDescriptionCount,
            _s46->pVertexAttributeDescriptions);
    }
    if (pInputAssemblyState != nullptr) {
        const auto _s47 = pInputAssemblyState;
        skip |= ValidatePipelineInputAssemblyStateCreateInfo(_parentObjects,
            _s47->sType,
            _s47->pNext,
            _s47->flags,
            _s47->topology,
            _s47->primitiveRestartEnable);
    }
    if (!shouldIgnore_VkGraphicsPipelineCreateInfo_pTessellationState(sType, pNext, flags, stageCount, pStages, pVertexInputState, pInputAssemblyState, pTessellationState, pViewportState, pRasterizationState, pMultisampleState, pDepthStencilState, pColorBlendState, pDynamicState, layout, renderPass, subpass, basePipelineHandle, basePipelineIndex)) {
        const auto _s48 = pTessellationState;
        skip |= ValidatePipelineTessellationStateCreateInfo(_parentObjects,
            _s48->sType,
            _s48->pNext,
            _s48->flags,
            _s48->patchControlPoints);
    }
    if (pViewportState != nullptr) {
        const auto _s49 = pViewportState;
        skip |= ValidatePipelineViewportStateCreateInfo(_parentObjects,
            _s49->sType,
            _s49->pNext,
            _s49->flags,
            _s49->viewportCount,
            _s49->pViewports,
            _s49->scissorCount,
            _s49->pScissors);
    }
    if (pRasterizationState != nullptr) {
        const auto _s50 = pRasterizationState;
        skip |= ValidatePipelineRasterizationStateCreateInfo(_parentObjects,
            _s50->sType,
            _s50->pNext,
            _s50->flags,
            _s50->depthClampEnable,
            _s50->rasterizerDiscardEnable,
            _s50->polygonMode,
            _s50->cullMode,
            _s50->frontFace,
            _s50->depthBiasEnable,
            _s50->depthBiasConstantFactor,
            _s50->depthBiasClamp,
            _s50->depthBiasSlopeFactor,
            _s50->lineWidth);
    }
    if (pMultisampleState != nullptr) {
        const auto _s51 = pMultisampleState;
        skip |= ValidatePipelineMultisampleStateCreateInfo(_parentObjects,
            _s51->sType,
            _s51->pNext,
            _s51->flags,
            _s51->rasterizationSamples,
            _s51->sampleShadingEnable,
            _s51->minSampleShading,
            _s51->pSampleMask,
            _s51->alphaToCoverageEnable,
            _s51->alphaToOneEnable);
    }
    if (pDepthStencilState != nullptr) {
        const auto _s52 = pDepthStencilState;
        skip |= ValidatePipelineDepthStencilStateCreateInfo(_parentObjects,
            _s52->sType,
            _s52->pNext,
            _s52->flags,
            _s52->depthTestEnable,
            _s52->depthWriteEnable,
            _s52->depthCompareOp,
            _s52->depthBoundsTestEnable,
            _s52->stencilTestEnable,
            _s52->front,
            _s52->back,
            _s52->minDepthBounds,
            _s52->maxDepthBounds);
    }
    if (pColorBlendState != nullptr) {
        const auto _s53 = pColorBlendState;
        skip |= ValidatePipelineColorBlendStateCreateInfo(_parentObjects,
            _s53->sType,
            _s53->pNext,
            _s53->flags,
            _s53->logicOpEnable,
            _s53->logicOp,
            _s53->attachmentCount,
            _s53->pAttachments,
            _s53->blendConstants);
    }
    if (pDynamicState != nullptr) {
        const auto _s54 = pDynamicState;
        skip |= ValidatePipelineDynamicStateCreateInfo(_parentObjects,
            _s54->sType,
            _s54->pNext,
            _s54->flags,
            _s54->dynamicStateCount,
            _s54->pDynamicStates);
    }
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
bool ExplicitValidation::ValidatePushConstantRange(const LogObjectList &_parentObjects,
                const VkShaderStageFlags stageFlags,
                const uint32_t offset,
                const uint32_t size) const {
    bool skip = false;
    return skip;
}
bool ExplicitValidation::ValidatePipelineLayoutCreateInfo(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const VkPipelineLayoutCreateFlags flags,
                const uint32_t setLayoutCount,
                const VkDescriptorSetLayout* pSetLayouts,
                const uint32_t pushConstantRangeCount,
                const VkPushConstantRange* pPushConstantRanges) const {
    bool skip = false;
    if (pPushConstantRanges != nullptr) {
        for (uint32_t _i55 = 0;_i55 < pushConstantRangeCount; ++_i55) {
            const auto _s56 = &pPushConstantRanges[_i55];
            skip |= ValidatePushConstantRange(_parentObjects,
                _s56->stageFlags,
                _s56->offset,
                _s56->size);
        }
    }
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
bool ExplicitValidation::ValidateSamplerCreateInfo(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const VkSamplerCreateFlags flags,
                const VkFilter magFilter,
                const VkFilter minFilter,
                const VkSamplerMipmapMode mipmapMode,
                const VkSamplerAddressMode addressModeU,
                const VkSamplerAddressMode addressModeV,
                const VkSamplerAddressMode addressModeW,
                const float mipLodBias,
                const VkBool32 anisotropyEnable,
                const float maxAnisotropy,
                const VkBool32 compareEnable,
                const VkCompareOp compareOp,
                const float minLod,
                const float maxLod,
                const VkBorderColor borderColor,
                const VkBool32 unnormalizedCoordinates) const {
    bool skip = false;
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
bool ExplicitValidation::ValidateCopyDescriptorSet(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const VkDescriptorSet srcSet,
                const uint32_t srcBinding,
                const uint32_t srcArrayElement,
                const VkDescriptorSet dstSet,
                const uint32_t dstBinding,
                const uint32_t dstArrayElement,
                const uint32_t descriptorCount) const {
    bool skip = false;
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
bool ExplicitValidation::ValidateDescriptorBufferInfo(const LogObjectList &_parentObjects,
                const VkBuffer buffer,
                const VkDeviceSize offset,
                const VkDeviceSize range) const {
    bool skip = false;
    return skip;
}
bool ExplicitValidation::ValidateDescriptorImageInfo(const LogObjectList &_parentObjects,
                const VkSampler sampler,
                const VkImageView imageView,
                const VkImageLayout imageLayout) const {
    bool skip = false;
    return skip;
}
bool ExplicitValidation::ValidateDescriptorPoolSize(const LogObjectList &_parentObjects,
                const VkDescriptorType type,
                const uint32_t descriptorCount) const {
    bool skip = false;
    return skip;
}
bool ExplicitValidation::ValidateDescriptorPoolCreateInfo(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const VkDescriptorPoolCreateFlags flags,
                const uint32_t maxSets,
                const uint32_t poolSizeCount,
                const VkDescriptorPoolSize* pPoolSizes) const {
    bool skip = false;
    if (pPoolSizes != nullptr) {
        for (uint32_t _i57 = 0;_i57 < poolSizeCount; ++_i57) {
            const auto _s58 = &pPoolSizes[_i57];
            skip |= ValidateDescriptorPoolSize(_parentObjects,
                _s58->type,
                _s58->descriptorCount);
        }
    }
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
bool ExplicitValidation::ValidateDescriptorSetAllocateInfo(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const VkDescriptorPool descriptorPool,
                const uint32_t descriptorSetCount,
                const VkDescriptorSetLayout* pSetLayouts) const {
    bool skip = false;
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
bool ExplicitValidation::ValidateDescriptorSetLayoutBinding(const LogObjectList &_parentObjects,
                const uint32_t binding,
                const VkDescriptorType descriptorType,
                const uint32_t descriptorCount,
                const VkShaderStageFlags stageFlags,
                const VkSampler* pImmutableSamplers) const {
    bool skip = false;
    return skip;
}
bool ExplicitValidation::ValidateDescriptorSetLayoutCreateInfo(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const VkDescriptorSetLayoutCreateFlags flags,
                const uint32_t bindingCount,
                const VkDescriptorSetLayoutBinding* pBindings) const {
    bool skip = false;

    {
        for (uint32_t i2 = 0; i2 < bindingCount; ++i2)
        {
            const VkDescriptorSetLayoutBinding &binding_0 = pBindings[i2];
            {
                for (uint32_t i4 = 0; i4 < bindingCount; ++i4)
                {
                    const VkDescriptorSetLayoutBinding &binding2_1 = pBindings[i4];
                    {
                        if ((i4 >= i2))
                        {
                            break;
                        }

                        if (!((binding_0.binding != binding2_1.binding)))
                        {
                            const LogObjectList objlist{};
                            skip |= LogFail(objlist, "VUID-VkDescriptorSetLayoutCreateInfo-binding-00279",
                                R"(    ${vu-keyword}for${} binding${vu-value}{{@%)" PRIu32 R"(}}${} ${vu-operator}in${} pBindings:
      ${vu-keyword}for${} binding2${vu-value}{{@%)" PRIu32 R"(}}${} ${vu-operator}in${} pBindings:
        ${vu-keyword}if${} ${vu-builtin}loop_index${}(binding2${vu-value}{{@%)" PRIu32 R"(}}${}) ${vu-operator}>=${} ${vu-builtin}loop_index${}(binding${vu-value}{{@%)" PRIu32 R"(}}${}):
          ${vu-keyword}break${}
        ${vu-builtin}require${}(binding${vu-value}{{@%)" PRIu32 R"(}}${}.binding${vu-value}{{%)" PRIu32 R"(}}${} ${vu-operator}!=${} binding2${vu-value}{{@%)" PRIu32 R"(}}${}.binding${vu-value}{{%)" PRIu32 R"(}}${})${vu-fail} <-- Failing condition (Values captured at this instant)${})",
                                i2,
                                i4,
                                i4,
                                i2,
                                i2,
                                binding_0.binding,
                                i4,
                                binding2_1.binding);
                        }

                    }
                }

                if ((binding_0.descriptorType == VK_DESCRIPTOR_TYPE_MUTABLE_EXT))
                {
                    if (!(Builtin_has_pnext<VkMutableDescriptorTypeCreateInfoEXT>(pNext)))
                    {
                        const LogObjectList objlist{};
                        skip |= LogFail(objlist, "VUID-VkDescriptorSetLayoutCreateInfo-pBindings-07303",
                            R"(    ${vu-keyword}for${} binding${vu-value}{{@%)" PRIu32 R"(}}${} ${vu-operator}in${} pBindings:
      ${vu-keyword}if${} binding${vu-value}{{@%)" PRIu32 R"(}}${}.descriptorType ${vu-operator}==${} VK_DESCRIPTOR_TYPE_MUTABLE_EXT:
        ${vu-builtin}require${}(${vu-builtin}has_pnext${}(VkMutableDescriptorTypeCreateInfoEXT)${vu-value}{{%s}}${})${vu-fail} <-- Failing condition (Values captured at this instant)${}
        mutableCount ${vu-operator}=${} ${vu-builtin}pnext${}(VkMutableDescriptorTypeCreateInfoEXT).mutableDescriptorTypeListCount
        ${vu-builtin}require${}(mutableCount ${vu-operator}>${} ${vu-builtin}loop_index${}(binding)))",
                            i2,
                            i2,
                            (Builtin_has_pnext<VkMutableDescriptorTypeCreateInfoEXT>(pNext)) ? "true" : "false");
                    }

                    uint32_t mutableCount_9=Builtin_pnext<VkMutableDescriptorTypeCreateInfoEXT>(pNext)->mutableDescriptorTypeListCount;
                    if (!((mutableCount_9 > i2)))
                    {
                        const LogObjectList objlist{};
                        skip |= LogFail(objlist, "VUID-VkDescriptorSetLayoutCreateInfo-pBindings-07303",
                            R"(    ${vu-keyword}for${} binding${vu-value}{{@%)" PRIu32 R"(}}${} ${vu-operator}in${} pBindings:
      ${vu-keyword}if${} binding${vu-value}{{@%)" PRIu32 R"(}}${}.descriptorType ${vu-operator}==${} VK_DESCRIPTOR_TYPE_MUTABLE_EXT:
        ${vu-builtin}require${}(${vu-builtin}has_pnext${}(VkMutableDescriptorTypeCreateInfoEXT)${vu-value}{{%s}}${})
        mutableCount${vu-value}{{%)" PRIu32 R"(}}${} ${vu-operator}=${} ${vu-builtin}pnext${}(VkMutableDescriptorTypeCreateInfoEXT).mutableDescriptorTypeListCount${vu-value}{{%)" PRIu32 R"(}}${}
        ${vu-builtin}require${}(mutableCount${vu-value}{{%)" PRIu32 R"(}}${} ${vu-operator}>${} ${vu-builtin}loop_index${}(binding${vu-value}{{@%)" PRIu32 R"(}}${}))${vu-fail} <-- Failing condition (Values captured at this instant)${})",
                            i2,
                            i2,
                            (Builtin_has_pnext<VkMutableDescriptorTypeCreateInfoEXT>(pNext)) ? "true" : "false",
                            mutableCount_9,
                            Builtin_pnext<VkMutableDescriptorTypeCreateInfoEXT>(pNext)->mutableDescriptorTypeListCount,
                            mutableCount_9,
                            i2);
                    }

                }

            }
        }

        if (Builtin_has_bit(flags, VK_DESCRIPTOR_SET_LAYOUT_CREATE_PUSH_DESCRIPTOR_BIT_KHR))
        {
            if (!(!(Builtin_has_bit(flags, VK_DESCRIPTOR_SET_LAYOUT_CREATE_HOST_ONLY_POOL_BIT_EXT))))
            {
                const LogObjectList objlist{};
                skip |= LogFail(objlist, "VUID-VkDescriptorSetLayoutCreateInfo-flags-04590",
                    R"(    ${vu-keyword}if${} flags${vu-value}{{%#)" PRIx32 R"(}}${}.${vu-builtin}has_bit${}(VK_DESCRIPTOR_SET_LAYOUT_CREATE_PUSH_DESCRIPTOR_BIT_KHR)${vu-value}{{%s}}${}:
      ${vu-builtin}require${}(${vu-operator}not${} flags${vu-value}{{%#)" PRIx32 R"(}}${}.${vu-builtin}has_bit${}(VK_DESCRIPTOR_SET_LAYOUT_CREATE_HOST_ONLY_POOL_BIT_EXT)${vu-value}{{%s}}${})${vu-fail} <-- Failing condition (Values captured at this instant)${})",
                    flags,
                    (Builtin_has_bit(flags, VK_DESCRIPTOR_SET_LAYOUT_CREATE_PUSH_DESCRIPTOR_BIT_KHR)) ? "true" : "false",
                    flags,
                    (Builtin_has_bit(flags, VK_DESCRIPTOR_SET_LAYOUT_CREATE_HOST_ONLY_POOL_BIT_EXT)) ? "true" : "false");
            }

            for (uint32_t i3 = 0; i3 < bindingCount; ++i3)
            {
                const VkDescriptorSetLayoutBinding &binding_2 = pBindings[i3];
                {
                    if (!(((binding_2.descriptorType != VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC)&&(binding_2.descriptorType != VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC))))
                    {
                        const LogObjectList objlist{};
                        skip |= LogFail(objlist, "VUID-VkDescriptorSetLayoutCreateInfo-flags-00280",
                            R"(    ${vu-keyword}if${} flags${vu-value}{{%#)" PRIx32 R"(}}${}.${vu-builtin}has_bit${}(VK_DESCRIPTOR_SET_LAYOUT_CREATE_PUSH_DESCRIPTOR_BIT_KHR)${vu-value}{{%s}}${}:
      ${vu-keyword}for${} binding${vu-value}{{@%)" PRIu32 R"(}}${} ${vu-operator}in${} pBindings:
        ${vu-builtin}require${}(binding${vu-value}{{@%)" PRIu32 R"(}}${}.descriptorType ${vu-operator}!=${} VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC ${vu-operator}and${}
                binding${vu-value}{{@%)" PRIu32 R"(}}${}.descriptorType ${vu-operator}!=${} VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC)${vu-fail} <-- Failing condition (Values captured at this instant)${})",
                            flags,
                            (Builtin_has_bit(flags, VK_DESCRIPTOR_SET_LAYOUT_CREATE_PUSH_DESCRIPTOR_BIT_KHR)) ? "true" : "false",
                            i3,
                            i3,
                            i3);
                    }

                    if (!((binding_2.descriptorType != VK_DESCRIPTOR_TYPE_INLINE_UNIFORM_BLOCK)))
                    {
                        const LogObjectList objlist{};
                        skip |= LogFail(objlist, "VUID-VkDescriptorSetLayoutCreateInfo-flags-02208",
                            R"(    ${vu-keyword}if${} flags${vu-value}{{%#)" PRIx32 R"(}}${}.${vu-builtin}has_bit${}(VK_DESCRIPTOR_SET_LAYOUT_CREATE_PUSH_DESCRIPTOR_BIT_KHR)${vu-value}{{%s}}${}:
      ${vu-keyword}for${} binding${vu-value}{{@%)" PRIu32 R"(}}${} ${vu-operator}in${} pBindings:
        ${vu-builtin}require${}(binding${vu-value}{{@%)" PRIu32 R"(}}${}.descriptorType ${vu-operator}!=${} VK_DESCRIPTOR_TYPE_INLINE_UNIFORM_BLOCK)${vu-fail} <-- Failing condition (Values captured at this instant)${})",
                            flags,
                            (Builtin_has_bit(flags, VK_DESCRIPTOR_SET_LAYOUT_CREATE_PUSH_DESCRIPTOR_BIT_KHR)) ? "true" : "false",
                            i3,
                            i3);
                    }

                    if (!((binding_2.descriptorType != VK_DESCRIPTOR_TYPE_MUTABLE_EXT)))
                    {
                        const LogObjectList objlist{};
                        skip |= LogFail(objlist, "VUID-VkDescriptorSetLayoutCreateInfo-flags-04591",
                            R"(    ${vu-keyword}if${} flags${vu-value}{{%#)" PRIx32 R"(}}${}.${vu-builtin}has_bit${}(VK_DESCRIPTOR_SET_LAYOUT_CREATE_PUSH_DESCRIPTOR_BIT_KHR)${vu-value}{{%s}}${}:
      ${vu-keyword}for${} binding${vu-value}{{@%)" PRIu32 R"(}}${} ${vu-operator}in${} pBindings:
        ${vu-builtin}require${}(binding${vu-value}{{@%)" PRIu32 R"(}}${}.descriptorType ${vu-operator}!=${} VK_DESCRIPTOR_TYPE_MUTABLE_EXT)${vu-fail} <-- Failing condition (Values captured at this instant)${})",
                            flags,
                            (Builtin_has_bit(flags, VK_DESCRIPTOR_SET_LAYOUT_CREATE_PUSH_DESCRIPTOR_BIT_KHR)) ? "true" : "false",
                            i3,
                            i3);
                    }

                }
            }

        }

        if (Builtin_has_pnext<VkDescriptorSetLayoutBindingFlagsCreateInfoEXT>(pNext))
        {
            const VkDescriptorSetLayoutBindingFlagsCreateInfoEXT *bindingFlagsCreateInfo_5=Builtin_pnext<VkDescriptorSetLayoutBindingFlagsCreateInfoEXT>(pNext);
            bool hasUpdateAfterBind_6=false;
            for (uint32_t i3 = 0; i3 < bindingCount; ++i3)
            {
                const VkDescriptorBindingFlags &bindingFlags_7 = bindingFlagsCreateInfo_5->pBindingFlags[i3];
                {
                    if (Builtin_has_bit(bindingFlags_7, VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT))
                    {
                        hasUpdateAfterBind_6=true;
                        break;
                    }

                }
            }

            if (hasUpdateAfterBind_6)
            {
                if (!(Builtin_has_bit(flags, VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT)))
                {
                    const LogObjectList objlist{};
                    skip |= LogFail(objlist, "VUID-VkDescriptorSetLayoutCreateInfo-flags-03000",
                        R"(    ${vu-keyword}if${} ${vu-builtin}has_pnext${}(VkDescriptorSetLayoutBindingFlagsCreateInfoEXT)${vu-value}{{%s}}${}:
      bindingFlagsCreateInfo ${vu-operator}=${} ${vu-builtin}pnext${}(VkDescriptorSetLayoutBindingFlagsCreateInfoEXT)
      hasUpdateAfterBind${vu-value}{{%s}}${} ${vu-operator}=${} ${vu-number}False${}
      ${vu-keyword}for${} bindingFlags ${vu-operator}in${} bindingFlagsCreateInfo.pBindingFlags:
        ${vu-keyword}if${} bindingFlags.${vu-builtin}has_bit${}(VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT):
          hasUpdateAfterBind ${vu-operator}=${} ${vu-number}True${}
          ${vu-keyword}break${}
      ${vu-keyword}if${} hasUpdateAfterBind${vu-value}{{%s}}${}:
        ${vu-builtin}require${}(flags${vu-value}{{%#)" PRIx32 R"(}}${}.${vu-builtin}has_bit${}(VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT)${vu-value}{{%s}}${})${vu-fail} <-- Failing condition (Values captured at this instant)${})",
                        (Builtin_has_pnext<VkDescriptorSetLayoutBindingFlagsCreateInfoEXT>(pNext)) ? "true" : "false",
                        (hasUpdateAfterBind_6) ? "true" : "false",
                        (hasUpdateAfterBind_6) ? "true" : "false",
                        flags,
                        (Builtin_has_bit(flags, VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT)) ? "true" : "false");
                }

            }

        }

    }
    if (pBindings != nullptr) {
        for (uint32_t _i59 = 0;_i59 < bindingCount; ++_i59) {
            const auto _s60 = &pBindings[_i59];
            skip |= ValidateDescriptorSetLayoutBinding(_parentObjects,
                _s60->binding,
                _s60->descriptorType,
                _s60->descriptorCount,
                _s60->stageFlags,
                _s60->pImmutableSamplers);
        }
    }
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
bool ExplicitValidation::ValidateWriteDescriptorSet(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const VkDescriptorSet dstSet,
                const uint32_t dstBinding,
                const uint32_t dstArrayElement,
                const uint32_t descriptorCount,
                const VkDescriptorType descriptorType,
                const VkDescriptorImageInfo* pImageInfo,
                const VkDescriptorBufferInfo* pBufferInfo,
                const VkBufferView* pTexelBufferView) const {
    bool skip = false;
    if (pImageInfo != nullptr) {
        for (uint32_t _i61 = 0;_i61 < descriptorCount; ++_i61) {
            const auto _s62 = &pImageInfo[_i61];
            skip |= ValidateDescriptorImageInfo(_parentObjects,
                _s62->sampler,
                _s62->imageView,
                _s62->imageLayout);
        }
    }
    if (pBufferInfo != nullptr) {
        for (uint32_t _i63 = 0;_i63 < descriptorCount; ++_i63) {
            const auto _s64 = &pBufferInfo[_i63];
            skip |= ValidateDescriptorBufferInfo(_parentObjects,
                _s64->buffer,
                _s64->offset,
                _s64->range);
        }
    }
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
bool ExplicitValidation::ValidateAttachmentDescription(const LogObjectList &_parentObjects,
                const VkAttachmentDescriptionFlags flags,
                const VkFormat format,
                const VkSampleCountFlagBits samples,
                const VkAttachmentLoadOp loadOp,
                const VkAttachmentStoreOp storeOp,
                const VkAttachmentLoadOp stencilLoadOp,
                const VkAttachmentStoreOp stencilStoreOp,
                const VkImageLayout initialLayout,
                const VkImageLayout finalLayout) const {
    bool skip = false;
    return skip;
}
bool ExplicitValidation::ValidateAttachmentReference(const LogObjectList &_parentObjects,
                const uint32_t attachment,
                const VkImageLayout layout) const {
    bool skip = false;
    return skip;
}
bool ExplicitValidation::ValidateFramebufferCreateInfo(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const VkFramebufferCreateFlags flags,
                const VkRenderPass renderPass,
                const uint32_t attachmentCount,
                const VkImageView* pAttachments,
                const uint32_t width,
                const uint32_t height,
                const uint32_t layers) const {
    bool skip = false;
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
bool ExplicitValidation::ValidateSubpassDescription(const LogObjectList &_parentObjects,
                const VkSubpassDescriptionFlags flags,
                const VkPipelineBindPoint pipelineBindPoint,
                const uint32_t inputAttachmentCount,
                const VkAttachmentReference* pInputAttachments,
                const uint32_t colorAttachmentCount,
                const VkAttachmentReference* pColorAttachments,
                const VkAttachmentReference* pResolveAttachments,
                const VkAttachmentReference* pDepthStencilAttachment,
                const uint32_t preserveAttachmentCount,
                const uint32_t* pPreserveAttachments) const {
    bool skip = false;
    if (pInputAttachments != nullptr) {
        for (uint32_t _i65 = 0;_i65 < inputAttachmentCount; ++_i65) {
            const auto _s66 = &pInputAttachments[_i65];
            skip |= ValidateAttachmentReference(_parentObjects,
                _s66->attachment,
                _s66->layout);
        }
    }
    if (pColorAttachments != nullptr) {
        for (uint32_t _i67 = 0;_i67 < colorAttachmentCount; ++_i67) {
            const auto _s68 = &pColorAttachments[_i67];
            skip |= ValidateAttachmentReference(_parentObjects,
                _s68->attachment,
                _s68->layout);
        }
    }
    if (pResolveAttachments != nullptr) {
        for (uint32_t _i69 = 0;_i69 < colorAttachmentCount; ++_i69) {
            const auto _s70 = &pResolveAttachments[_i69];
            skip |= ValidateAttachmentReference(_parentObjects,
                _s70->attachment,
                _s70->layout);
        }
    }
    if (pDepthStencilAttachment != nullptr) {
        const auto _s71 = pDepthStencilAttachment;
        skip |= ValidateAttachmentReference(_parentObjects,
            _s71->attachment,
            _s71->layout);
    }
    return skip;
}
bool ExplicitValidation::ValidateSubpassDependency(const LogObjectList &_parentObjects,
                const uint32_t srcSubpass,
                const uint32_t dstSubpass,
                const VkPipelineStageFlags srcStageMask,
                const VkPipelineStageFlags dstStageMask,
                const VkAccessFlags srcAccessMask,
                const VkAccessFlags dstAccessMask,
                const VkDependencyFlags dependencyFlags) const {
    bool skip = false;
    return skip;
}
bool ExplicitValidation::ValidateRenderPassCreateInfo(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const VkRenderPassCreateFlags flags,
                const uint32_t attachmentCount,
                const VkAttachmentDescription* pAttachments,
                const uint32_t subpassCount,
                const VkSubpassDescription* pSubpasses,
                const uint32_t dependencyCount,
                const VkSubpassDependency* pDependencies) const {
    bool skip = false;
    if (pAttachments != nullptr) {
        for (uint32_t _i72 = 0;_i72 < attachmentCount; ++_i72) {
            const auto _s73 = &pAttachments[_i72];
            skip |= ValidateAttachmentDescription(_parentObjects,
                _s73->flags,
                _s73->format,
                _s73->samples,
                _s73->loadOp,
                _s73->storeOp,
                _s73->stencilLoadOp,
                _s73->stencilStoreOp,
                _s73->initialLayout,
                _s73->finalLayout);
        }
    }
    if (pSubpasses != nullptr) {
        for (uint32_t _i74 = 0;_i74 < subpassCount; ++_i74) {
            const auto _s75 = &pSubpasses[_i74];
            skip |= ValidateSubpassDescription(_parentObjects,
                _s75->flags,
                _s75->pipelineBindPoint,
                _s75->inputAttachmentCount,
                _s75->pInputAttachments,
                _s75->colorAttachmentCount,
                _s75->pColorAttachments,
                _s75->pResolveAttachments,
                _s75->pDepthStencilAttachment,
                _s75->preserveAttachmentCount,
                _s75->pPreserveAttachments);
        }
    }
    if (pDependencies != nullptr) {
        for (uint32_t _i76 = 0;_i76 < dependencyCount; ++_i76) {
            const auto _s77 = &pDependencies[_i76];
            skip |= ValidateSubpassDependency(_parentObjects,
                _s77->srcSubpass,
                _s77->dstSubpass,
                _s77->srcStageMask,
                _s77->dstStageMask,
                _s77->srcAccessMask,
                _s77->dstAccessMask,
                _s77->dependencyFlags);
        }
    }
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
bool ExplicitValidation::ValidateCommandPoolCreateInfo(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const VkCommandPoolCreateFlags flags,
                const uint32_t queueFamilyIndex) const {
    bool skip = false;
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
bool ExplicitValidation::ValidateCommandBufferAllocateInfo(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const VkCommandPool commandPool,
                const VkCommandBufferLevel level,
                const uint32_t commandBufferCount) const {
    bool skip = false;
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
bool ExplicitValidation::ValidateCommandBufferInheritanceInfo(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const VkRenderPass renderPass,
                const uint32_t subpass,
                const VkFramebuffer framebuffer,
                const VkBool32 occlusionQueryEnable,
                const VkQueryControlFlags queryFlags,
                const VkQueryPipelineStatisticFlags pipelineStatistics) const {
    bool skip = false;
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
bool ExplicitValidation::ValidateCommandBufferBeginInfo(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const VkCommandBufferUsageFlags flags,
                const VkCommandBufferInheritanceInfo* pInheritanceInfo) const {
    bool skip = false;
    if (pInheritanceInfo != nullptr) {
        const auto _s78 = pInheritanceInfo;
        skip |= ValidateCommandBufferInheritanceInfo(_parentObjects,
            _s78->sType,
            _s78->pNext,
            _s78->renderPass,
            _s78->subpass,
            _s78->framebuffer,
            _s78->occlusionQueryEnable,
            _s78->queryFlags,
            _s78->pipelineStatistics);
    }
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
bool ExplicitValidation::ValidateBufferCopy(const LogObjectList &_parentObjects,
                const VkDeviceSize srcOffset,
                const VkDeviceSize dstOffset,
                const VkDeviceSize size) const {
    bool skip = false;
    return skip;
}
bool ExplicitValidation::ValidateImageSubresourceLayers(const LogObjectList &_parentObjects,
                const VkImageAspectFlags aspectMask,
                const uint32_t mipLevel,
                const uint32_t baseArrayLayer,
                const uint32_t layerCount) const {
    bool skip = false;
    return skip;
}
bool ExplicitValidation::ValidateBufferImageCopy(const LogObjectList &_parentObjects,
                const VkDeviceSize bufferOffset,
                const uint32_t bufferRowLength,
                const uint32_t bufferImageHeight,
                const VkImageSubresourceLayers imageSubresource,
                const VkOffset3D imageOffset,
                const VkExtent3D imageExtent) const {
    bool skip = false;
    const auto _s79 = &imageSubresource;
    skip |= ValidateImageSubresourceLayers(_parentObjects,
        _s79->aspectMask,
        _s79->mipLevel,
        _s79->baseArrayLayer,
        _s79->layerCount);
    const auto _s80 = &imageOffset;
    skip |= ValidateOffset3D(_parentObjects,
        _s80->x,
        _s80->y,
        _s80->z);
    const auto _s81 = &imageExtent;
    skip |= ValidateExtent3D(_parentObjects,
        _s81->width,
        _s81->height,
        _s81->depth);
    return skip;
}
bool ExplicitValidation::ValidateClearColorValue(const LogObjectList &_parentObjects,
                const float float32[4],
                const int32_t int32[4],
                const uint32_t uint32[4]) const {
    bool skip = false;
    return skip;
}
bool ExplicitValidation::ValidateClearDepthStencilValue(const LogObjectList &_parentObjects,
                const float depth,
                const uint32_t stencil) const {
    bool skip = false;
    return skip;
}
bool ExplicitValidation::ValidateClearValue(const LogObjectList &_parentObjects,
                const VkClearColorValue color,
                const VkClearDepthStencilValue depthStencil) const {
    bool skip = false;
    const auto _s82 = &color;
    skip |= ValidateClearColorValue(_parentObjects,
        _s82->float32,
        _s82->int32,
        _s82->uint32);
    const auto _s83 = &depthStencil;
    skip |= ValidateClearDepthStencilValue(_parentObjects,
        _s83->depth,
        _s83->stencil);
    return skip;
}
bool ExplicitValidation::ValidateClearAttachment(const LogObjectList &_parentObjects,
                const VkImageAspectFlags aspectMask,
                const uint32_t colorAttachment,
                const VkClearValue clearValue) const {
    bool skip = false;
    const auto _s84 = &clearValue;
    skip |= ValidateClearValue(_parentObjects,
        _s84->color,
        _s84->depthStencil);
    return skip;
}
bool ExplicitValidation::ValidateClearRect(const LogObjectList &_parentObjects,
                const VkRect2D rect,
                const uint32_t baseArrayLayer,
                const uint32_t layerCount) const {
    bool skip = false;
    const auto _s85 = &rect;
    skip |= ValidateRect2D(_parentObjects,
        _s85->offset,
        _s85->extent);
    return skip;
}
bool ExplicitValidation::ValidateImageBlit(const LogObjectList &_parentObjects,
                const VkImageSubresourceLayers srcSubresource,
                const VkOffset3D srcOffsets[2],
                const VkImageSubresourceLayers dstSubresource,
                const VkOffset3D dstOffsets[2]) const {
    bool skip = false;
    const auto _s86 = &srcSubresource;
    skip |= ValidateImageSubresourceLayers(_parentObjects,
        _s86->aspectMask,
        _s86->mipLevel,
        _s86->baseArrayLayer,
        _s86->layerCount);
    if (srcOffsets != nullptr) {
        for (uint32_t _i87 = 0;_i87 < 2; ++_i87) {
            const auto _s88 = &srcOffsets[_i87];
            skip |= ValidateOffset3D(_parentObjects,
                _s88->x,
                _s88->y,
                _s88->z);
        }
    }
    const auto _s89 = &dstSubresource;
    skip |= ValidateImageSubresourceLayers(_parentObjects,
        _s89->aspectMask,
        _s89->mipLevel,
        _s89->baseArrayLayer,
        _s89->layerCount);
    if (dstOffsets != nullptr) {
        for (uint32_t _i90 = 0;_i90 < 2; ++_i90) {
            const auto _s91 = &dstOffsets[_i90];
            skip |= ValidateOffset3D(_parentObjects,
                _s91->x,
                _s91->y,
                _s91->z);
        }
    }
    return skip;
}
bool ExplicitValidation::ValidateImageCopy(const LogObjectList &_parentObjects,
                const VkImageSubresourceLayers srcSubresource,
                const VkOffset3D srcOffset,
                const VkImageSubresourceLayers dstSubresource,
                const VkOffset3D dstOffset,
                const VkExtent3D extent) const {
    bool skip = false;
    const auto _s92 = &srcSubresource;
    skip |= ValidateImageSubresourceLayers(_parentObjects,
        _s92->aspectMask,
        _s92->mipLevel,
        _s92->baseArrayLayer,
        _s92->layerCount);
    const auto _s93 = &srcOffset;
    skip |= ValidateOffset3D(_parentObjects,
        _s93->x,
        _s93->y,
        _s93->z);
    const auto _s94 = &dstSubresource;
    skip |= ValidateImageSubresourceLayers(_parentObjects,
        _s94->aspectMask,
        _s94->mipLevel,
        _s94->baseArrayLayer,
        _s94->layerCount);
    const auto _s95 = &dstOffset;
    skip |= ValidateOffset3D(_parentObjects,
        _s95->x,
        _s95->y,
        _s95->z);
    const auto _s96 = &extent;
    skip |= ValidateExtent3D(_parentObjects,
        _s96->width,
        _s96->height,
        _s96->depth);
    return skip;
}
bool ExplicitValidation::ValidateImageResolve(const LogObjectList &_parentObjects,
                const VkImageSubresourceLayers srcSubresource,
                const VkOffset3D srcOffset,
                const VkImageSubresourceLayers dstSubresource,
                const VkOffset3D dstOffset,
                const VkExtent3D extent) const {
    bool skip = false;
    const auto _s97 = &srcSubresource;
    skip |= ValidateImageSubresourceLayers(_parentObjects,
        _s97->aspectMask,
        _s97->mipLevel,
        _s97->baseArrayLayer,
        _s97->layerCount);
    const auto _s98 = &srcOffset;
    skip |= ValidateOffset3D(_parentObjects,
        _s98->x,
        _s98->y,
        _s98->z);
    const auto _s99 = &dstSubresource;
    skip |= ValidateImageSubresourceLayers(_parentObjects,
        _s99->aspectMask,
        _s99->mipLevel,
        _s99->baseArrayLayer,
        _s99->layerCount);
    const auto _s100 = &dstOffset;
    skip |= ValidateOffset3D(_parentObjects,
        _s100->x,
        _s100->y,
        _s100->z);
    const auto _s101 = &extent;
    skip |= ValidateExtent3D(_parentObjects,
        _s101->width,
        _s101->height,
        _s101->depth);
    return skip;
}
bool ExplicitValidation::ValidateRenderPassBeginInfo(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const VkRenderPass renderPass,
                const VkFramebuffer framebuffer,
                const VkRect2D renderArea,
                const uint32_t clearValueCount,
                const VkClearValue* pClearValues) const {
    bool skip = false;
    const auto _s102 = &renderArea;
    skip |= ValidateRect2D(_parentObjects,
        _s102->offset,
        _s102->extent);
    if (pClearValues != nullptr) {
        for (uint32_t _i103 = 0;_i103 < clearValueCount; ++_i103) {
            const auto _s104 = &pClearValues[_i103];
            skip |= ValidateClearValue(_parentObjects,
                _s104->color,
                _s104->depthStencil);
        }
    }
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
bool ExplicitValidation::ValidateBindBufferMemoryInfo(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const VkBuffer buffer,
                const VkDeviceMemory memory,
                const VkDeviceSize memoryOffset) const {
    bool skip = false;
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
bool ExplicitValidation::ValidateBindImageMemoryInfo(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const VkImage image,
                const VkDeviceMemory memory,
                const VkDeviceSize memoryOffset) const {
    bool skip = false;
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
bool ExplicitValidation::ValidatePhysicalDevice16BitStorageFeatures(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const VkBool32 storageBuffer16BitAccess,
                const VkBool32 uniformAndStorageBuffer16BitAccess,
                const VkBool32 storagePushConstant16,
                const VkBool32 storageInputOutput16) const {
    bool skip = false;
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
bool ExplicitValidation::ValidateMemoryDedicatedAllocateInfo(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const VkImage image,
                const VkBuffer buffer) const {
    bool skip = false;
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
bool ExplicitValidation::ValidateMemoryAllocateFlagsInfo(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const VkMemoryAllocateFlags flags,
                const uint32_t deviceMask) const {
    bool skip = false;
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
bool ExplicitValidation::ValidateDeviceGroupRenderPassBeginInfo(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const uint32_t deviceMask,
                const uint32_t deviceRenderAreaCount,
                const VkRect2D* pDeviceRenderAreas) const {
    bool skip = false;
    if (pDeviceRenderAreas != nullptr) {
        for (uint32_t _i105 = 0;_i105 < deviceRenderAreaCount; ++_i105) {
            const auto _s106 = &pDeviceRenderAreas[_i105];
            skip |= ValidateRect2D(_parentObjects,
                _s106->offset,
                _s106->extent);
        }
    }
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
bool ExplicitValidation::ValidateDeviceGroupCommandBufferBeginInfo(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const uint32_t deviceMask) const {
    bool skip = false;
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
bool ExplicitValidation::ValidateDeviceGroupSubmitInfo(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const uint32_t waitSemaphoreCount,
                const uint32_t* pWaitSemaphoreDeviceIndices,
                const uint32_t commandBufferCount,
                const uint32_t* pCommandBufferDeviceMasks,
                const uint32_t signalSemaphoreCount,
                const uint32_t* pSignalSemaphoreDeviceIndices) const {
    bool skip = false;
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
bool ExplicitValidation::ValidateDeviceGroupBindSparseInfo(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const uint32_t resourceDeviceIndex,
                const uint32_t memoryDeviceIndex) const {
    bool skip = false;
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
bool ExplicitValidation::ValidateBindBufferMemoryDeviceGroupInfo(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const uint32_t deviceIndexCount,
                const uint32_t* pDeviceIndices) const {
    bool skip = false;
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
bool ExplicitValidation::ValidateBindImageMemoryDeviceGroupInfo(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const uint32_t deviceIndexCount,
                const uint32_t* pDeviceIndices,
                const uint32_t splitInstanceBindRegionCount,
                const VkRect2D* pSplitInstanceBindRegions) const {
    bool skip = false;
    if (pSplitInstanceBindRegions != nullptr) {
        for (uint32_t _i107 = 0;_i107 < splitInstanceBindRegionCount; ++_i107) {
            const auto _s108 = &pSplitInstanceBindRegions[_i107];
            skip |= ValidateRect2D(_parentObjects,
                _s108->offset,
                _s108->extent);
        }
    }
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
bool ExplicitValidation::ValidateDeviceGroupDeviceCreateInfo(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const uint32_t physicalDeviceCount,
                const VkPhysicalDevice* pPhysicalDevices) const {
    bool skip = false;
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
bool ExplicitValidation::ValidateBufferMemoryRequirementsInfo2(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const VkBuffer buffer) const {
    bool skip = false;
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
bool ExplicitValidation::ValidateImageMemoryRequirementsInfo2(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const VkImage image) const {
    bool skip = false;
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
bool ExplicitValidation::ValidateImageSparseMemoryRequirementsInfo2(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const VkImage image) const {
    bool skip = false;
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
bool ExplicitValidation::ValidatePhysicalDeviceFeatures2(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const VkPhysicalDeviceFeatures features) const {
    bool skip = false;
    const auto _s109 = &features;
    skip |= ValidatePhysicalDeviceFeatures(_parentObjects,
        _s109->robustBufferAccess,
        _s109->fullDrawIndexUint32,
        _s109->imageCubeArray,
        _s109->independentBlend,
        _s109->geometryShader,
        _s109->tessellationShader,
        _s109->sampleRateShading,
        _s109->dualSrcBlend,
        _s109->logicOp,
        _s109->multiDrawIndirect,
        _s109->drawIndirectFirstInstance,
        _s109->depthClamp,
        _s109->depthBiasClamp,
        _s109->fillModeNonSolid,
        _s109->depthBounds,
        _s109->wideLines,
        _s109->largePoints,
        _s109->alphaToOne,
        _s109->multiViewport,
        _s109->samplerAnisotropy,
        _s109->textureCompressionETC2,
        _s109->textureCompressionASTC_LDR,
        _s109->textureCompressionBC,
        _s109->occlusionQueryPrecise,
        _s109->pipelineStatisticsQuery,
        _s109->vertexPipelineStoresAndAtomics,
        _s109->fragmentStoresAndAtomics,
        _s109->shaderTessellationAndGeometryPointSize,
        _s109->shaderImageGatherExtended,
        _s109->shaderStorageImageExtendedFormats,
        _s109->shaderStorageImageMultisample,
        _s109->shaderStorageImageReadWithoutFormat,
        _s109->shaderStorageImageWriteWithoutFormat,
        _s109->shaderUniformBufferArrayDynamicIndexing,
        _s109->shaderSampledImageArrayDynamicIndexing,
        _s109->shaderStorageBufferArrayDynamicIndexing,
        _s109->shaderStorageImageArrayDynamicIndexing,
        _s109->shaderClipDistance,
        _s109->shaderCullDistance,
        _s109->shaderFloat64,
        _s109->shaderInt64,
        _s109->shaderInt16,
        _s109->shaderResourceResidency,
        _s109->shaderResourceMinLod,
        _s109->sparseBinding,
        _s109->sparseResidencyBuffer,
        _s109->sparseResidencyImage2D,
        _s109->sparseResidencyImage3D,
        _s109->sparseResidency2Samples,
        _s109->sparseResidency4Samples,
        _s109->sparseResidency8Samples,
        _s109->sparseResidency16Samples,
        _s109->sparseResidencyAliased,
        _s109->variableMultisampleRate,
        _s109->inheritedQueries);
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
bool ExplicitValidation::ValidatePhysicalDeviceImageFormatInfo2(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const VkFormat format,
                const VkImageType type,
                const VkImageTiling tiling,
                const VkImageUsageFlags usage,
                const VkImageCreateFlags flags) const {
    bool skip = false;
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
bool ExplicitValidation::ValidatePhysicalDeviceSparseImageFormatInfo2(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const VkFormat format,
                const VkImageType type,
                const VkSampleCountFlagBits samples,
                const VkImageUsageFlags usage,
                const VkImageTiling tiling) const {
    bool skip = false;
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
bool ExplicitValidation::ValidateInputAttachmentAspectReference(const LogObjectList &_parentObjects,
                const uint32_t subpass,
                const uint32_t inputAttachmentIndex,
                const VkImageAspectFlags aspectMask) const {
    bool skip = false;
    return skip;
}
bool ExplicitValidation::ValidateRenderPassInputAttachmentAspectCreateInfo(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const uint32_t aspectReferenceCount,
                const VkInputAttachmentAspectReference* pAspectReferences) const {
    bool skip = false;
    if (pAspectReferences != nullptr) {
        for (uint32_t _i110 = 0;_i110 < aspectReferenceCount; ++_i110) {
            const auto _s111 = &pAspectReferences[_i110];
            skip |= ValidateInputAttachmentAspectReference(_parentObjects,
                _s111->subpass,
                _s111->inputAttachmentIndex,
                _s111->aspectMask);
        }
    }
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
bool ExplicitValidation::ValidateImageViewUsageCreateInfo(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const VkImageUsageFlags usage) const {
    bool skip = false;
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
bool ExplicitValidation::ValidatePipelineTessellationDomainOriginStateCreateInfo(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const VkTessellationDomainOrigin domainOrigin) const {
    bool skip = false;
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
bool ExplicitValidation::ValidateRenderPassMultiviewCreateInfo(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const uint32_t subpassCount,
                const uint32_t* pViewMasks,
                const uint32_t dependencyCount,
                const int32_t* pViewOffsets,
                const uint32_t correlationMaskCount,
                const uint32_t* pCorrelationMasks) const {
    bool skip = false;
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
bool ExplicitValidation::ValidatePhysicalDeviceMultiviewFeatures(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const VkBool32 multiview,
                const VkBool32 multiviewGeometryShader,
                const VkBool32 multiviewTessellationShader) const {
    bool skip = false;
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
bool ExplicitValidation::ValidatePhysicalDeviceVariablePointersFeatures(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const VkBool32 variablePointersStorageBuffer,
                const VkBool32 variablePointers) const {
    bool skip = false;
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
bool ExplicitValidation::ValidatePhysicalDeviceVariablePointerFeatures(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const VkBool32 variablePointersStorageBuffer,
                const VkBool32 variablePointers) const {
    return ValidatePhysicalDeviceVariablePointersFeatures(_parentObjects, sType, pNext, variablePointersStorageBuffer, variablePointers);
}
bool ExplicitValidation::ValidatePhysicalDeviceProtectedMemoryFeatures(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const VkBool32 protectedMemory) const {
    bool skip = false;
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
bool ExplicitValidation::ValidateDeviceQueueInfo2(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const VkDeviceQueueCreateFlags flags,
                const uint32_t queueFamilyIndex,
                const uint32_t queueIndex) const {
    bool skip = false;
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
bool ExplicitValidation::ValidateProtectedSubmitInfo(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const VkBool32 protectedSubmit) const {
    bool skip = false;
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
bool ExplicitValidation::ValidateSamplerYcbcrConversionCreateInfo(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const VkFormat format,
                const VkSamplerYcbcrModelConversion ycbcrModel,
                const VkSamplerYcbcrRange ycbcrRange,
                const VkComponentMapping components,
                const VkChromaLocation xChromaOffset,
                const VkChromaLocation yChromaOffset,
                const VkFilter chromaFilter,
                VkBool32 forceExplicitReconstruction) const {
    bool skip = false;
    const auto _s112 = &components;
    skip |= ValidateComponentMapping(_parentObjects,
        _s112->r,
        _s112->g,
        _s112->b,
        _s112->a);
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
bool ExplicitValidation::ValidateSamplerYcbcrConversionInfo(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const VkSamplerYcbcrConversion conversion) const {
    bool skip = false;
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
bool ExplicitValidation::ValidateBindImagePlaneMemoryInfo(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const VkImageAspectFlagBits planeAspect) const {
    bool skip = false;
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
bool ExplicitValidation::ValidateImagePlaneMemoryRequirementsInfo(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const VkImageAspectFlagBits planeAspect) const {
    bool skip = false;
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
bool ExplicitValidation::ValidatePhysicalDeviceSamplerYcbcrConversionFeatures(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const VkBool32 samplerYcbcrConversion) const {
    bool skip = false;
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
bool ExplicitValidation::ValidateDescriptorUpdateTemplateEntry(const LogObjectList &_parentObjects,
                const uint32_t dstBinding,
                const uint32_t dstArrayElement,
                const uint32_t descriptorCount,
                const VkDescriptorType descriptorType,
                const size_t offset,
                const size_t stride) const {
    bool skip = false;
    return skip;
}
bool ExplicitValidation::ValidateDescriptorUpdateTemplateCreateInfo(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const VkDescriptorUpdateTemplateCreateFlags flags,
                const uint32_t descriptorUpdateEntryCount,
                const VkDescriptorUpdateTemplateEntry* pDescriptorUpdateEntries,
                const VkDescriptorUpdateTemplateType templateType,
                const VkDescriptorSetLayout descriptorSetLayout,
                const VkPipelineBindPoint pipelineBindPoint,
                const VkPipelineLayout pipelineLayout,
                const uint32_t set) const {
    bool skip = false;
    if (pDescriptorUpdateEntries != nullptr) {
        for (uint32_t _i113 = 0;_i113 < descriptorUpdateEntryCount; ++_i113) {
            const auto _s114 = &pDescriptorUpdateEntries[_i113];
            skip |= ValidateDescriptorUpdateTemplateEntry(_parentObjects,
                _s114->dstBinding,
                _s114->dstArrayElement,
                _s114->descriptorCount,
                _s114->descriptorType,
                _s114->offset,
                _s114->stride);
        }
    }
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
bool ExplicitValidation::ValidatePhysicalDeviceExternalImageFormatInfo(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const VkExternalMemoryHandleTypeFlagBits handleType) const {
    bool skip = false;
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
bool ExplicitValidation::ValidatePhysicalDeviceExternalBufferInfo(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const VkBufferCreateFlags flags,
                const VkBufferUsageFlags usage,
                const VkExternalMemoryHandleTypeFlagBits handleType) const {
    bool skip = false;
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
bool ExplicitValidation::ValidateExternalMemoryImageCreateInfo(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const VkExternalMemoryHandleTypeFlags handleTypes) const {
    bool skip = false;
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
bool ExplicitValidation::ValidateExternalMemoryBufferCreateInfo(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const VkExternalMemoryHandleTypeFlags handleTypes) const {
    bool skip = false;
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
bool ExplicitValidation::ValidateExportMemoryAllocateInfo(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const VkExternalMemoryHandleTypeFlags handleTypes) const {
    bool skip = false;
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
bool ExplicitValidation::ValidatePhysicalDeviceExternalFenceInfo(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const VkExternalFenceHandleTypeFlagBits handleType) const {
    bool skip = false;
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
bool ExplicitValidation::ValidateExportFenceCreateInfo(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const VkExternalFenceHandleTypeFlags handleTypes) const {
    bool skip = false;
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
bool ExplicitValidation::ValidateExportSemaphoreCreateInfo(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const VkExternalSemaphoreHandleTypeFlags handleTypes) const {
    bool skip = false;
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
bool ExplicitValidation::ValidatePhysicalDeviceExternalSemaphoreInfo(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const VkExternalSemaphoreHandleTypeFlagBits handleType) const {
    bool skip = false;
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
bool ExplicitValidation::ValidatePhysicalDeviceShaderDrawParametersFeatures(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const VkBool32 shaderDrawParameters) const {
    bool skip = false;
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
bool ExplicitValidation::ValidatePhysicalDeviceShaderDrawParameterFeatures(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const VkBool32 shaderDrawParameters) const {
    return ValidatePhysicalDeviceShaderDrawParametersFeatures(_parentObjects, sType, pNext, shaderDrawParameters);
}
bool ExplicitValidation::ValidatePhysicalDeviceVulkan11Features(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const VkBool32 storageBuffer16BitAccess,
                const VkBool32 uniformAndStorageBuffer16BitAccess,
                const VkBool32 storagePushConstant16,
                const VkBool32 storageInputOutput16,
                const VkBool32 multiview,
                const VkBool32 multiviewGeometryShader,
                const VkBool32 multiviewTessellationShader,
                const VkBool32 variablePointersStorageBuffer,
                const VkBool32 variablePointers,
                const VkBool32 protectedMemory,
                const VkBool32 samplerYcbcrConversion,
                const VkBool32 shaderDrawParameters) const {
    bool skip = false;
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
bool ExplicitValidation::ValidatePhysicalDeviceVulkan12Features(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const VkBool32 samplerMirrorClampToEdge,
                const VkBool32 drawIndirectCount,
                const VkBool32 storageBuffer8BitAccess,
                const VkBool32 uniformAndStorageBuffer8BitAccess,
                const VkBool32 storagePushConstant8,
                const VkBool32 shaderBufferInt64Atomics,
                const VkBool32 shaderSharedInt64Atomics,
                const VkBool32 shaderFloat16,
                const VkBool32 shaderInt8,
                const VkBool32 descriptorIndexing,
                const VkBool32 shaderInputAttachmentArrayDynamicIndexing,
                const VkBool32 shaderUniformTexelBufferArrayDynamicIndexing,
                const VkBool32 shaderStorageTexelBufferArrayDynamicIndexing,
                const VkBool32 shaderUniformBufferArrayNonUniformIndexing,
                const VkBool32 shaderSampledImageArrayNonUniformIndexing,
                const VkBool32 shaderStorageBufferArrayNonUniformIndexing,
                const VkBool32 shaderStorageImageArrayNonUniformIndexing,
                const VkBool32 shaderInputAttachmentArrayNonUniformIndexing,
                const VkBool32 shaderUniformTexelBufferArrayNonUniformIndexing,
                const VkBool32 shaderStorageTexelBufferArrayNonUniformIndexing,
                const VkBool32 descriptorBindingUniformBufferUpdateAfterBind,
                const VkBool32 descriptorBindingSampledImageUpdateAfterBind,
                const VkBool32 descriptorBindingStorageImageUpdateAfterBind,
                const VkBool32 descriptorBindingStorageBufferUpdateAfterBind,
                const VkBool32 descriptorBindingUniformTexelBufferUpdateAfterBind,
                const VkBool32 descriptorBindingStorageTexelBufferUpdateAfterBind,
                const VkBool32 descriptorBindingUpdateUnusedWhilePending,
                const VkBool32 descriptorBindingPartiallyBound,
                const VkBool32 descriptorBindingVariableDescriptorCount,
                const VkBool32 runtimeDescriptorArray,
                const VkBool32 samplerFilterMinmax,
                const VkBool32 scalarBlockLayout,
                const VkBool32 imagelessFramebuffer,
                const VkBool32 uniformBufferStandardLayout,
                const VkBool32 shaderSubgroupExtendedTypes,
                const VkBool32 separateDepthStencilLayouts,
                const VkBool32 hostQueryReset,
                const VkBool32 timelineSemaphore,
                const VkBool32 bufferDeviceAddress,
                const VkBool32 bufferDeviceAddressCaptureReplay,
                const VkBool32 bufferDeviceAddressMultiDevice,
                const VkBool32 vulkanMemoryModel,
                const VkBool32 vulkanMemoryModelDeviceScope,
                const VkBool32 vulkanMemoryModelAvailabilityVisibilityChains,
                const VkBool32 shaderOutputViewportIndex,
                const VkBool32 shaderOutputLayer,
                const VkBool32 subgroupBroadcastDynamicId) const {
    bool skip = false;
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
bool ExplicitValidation::ValidateConformanceVersion(const LogObjectList &_parentObjects,
                const uint8_t major,
                const uint8_t minor,
                const uint8_t subminor,
                const uint8_t patch) const {
    bool skip = false;
    return skip;
}
bool ExplicitValidation::ValidateImageFormatListCreateInfo(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const uint32_t viewFormatCount,
                const VkFormat* pViewFormats) const {
    bool skip = false;
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
bool ExplicitValidation::ValidateAttachmentDescription2(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const VkAttachmentDescriptionFlags flags,
                const VkFormat format,
                const VkSampleCountFlagBits samples,
                const VkAttachmentLoadOp loadOp,
                const VkAttachmentStoreOp storeOp,
                const VkAttachmentLoadOp stencilLoadOp,
                const VkAttachmentStoreOp stencilStoreOp,
                const VkImageLayout initialLayout,
                const VkImageLayout finalLayout) const {
    bool skip = false;
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
bool ExplicitValidation::ValidateAttachmentReference2(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const uint32_t attachment,
                const VkImageLayout layout,
                const VkImageAspectFlags aspectMask) const {
    bool skip = false;
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
bool ExplicitValidation::ValidateSubpassDescription2(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const VkSubpassDescriptionFlags flags,
                const VkPipelineBindPoint pipelineBindPoint,
                const uint32_t viewMask,
                const uint32_t inputAttachmentCount,
                const VkAttachmentReference2* pInputAttachments,
                const uint32_t colorAttachmentCount,
                const VkAttachmentReference2* pColorAttachments,
                const VkAttachmentReference2* pResolveAttachments,
                const VkAttachmentReference2* pDepthStencilAttachment,
                const uint32_t preserveAttachmentCount,
                const uint32_t* pPreserveAttachments) const {
    bool skip = false;
    if (pInputAttachments != nullptr) {
        for (uint32_t _i115 = 0;_i115 < inputAttachmentCount; ++_i115) {
            const auto _s116 = &pInputAttachments[_i115];
            skip |= ValidateAttachmentReference2(_parentObjects,
                _s116->sType,
                _s116->pNext,
                _s116->attachment,
                _s116->layout,
                _s116->aspectMask);
        }
    }
    if (pColorAttachments != nullptr) {
        for (uint32_t _i117 = 0;_i117 < colorAttachmentCount; ++_i117) {
            const auto _s118 = &pColorAttachments[_i117];
            skip |= ValidateAttachmentReference2(_parentObjects,
                _s118->sType,
                _s118->pNext,
                _s118->attachment,
                _s118->layout,
                _s118->aspectMask);
        }
    }
    if (pResolveAttachments != nullptr) {
        for (uint32_t _i119 = 0;_i119 < colorAttachmentCount; ++_i119) {
            const auto _s120 = &pResolveAttachments[_i119];
            skip |= ValidateAttachmentReference2(_parentObjects,
                _s120->sType,
                _s120->pNext,
                _s120->attachment,
                _s120->layout,
                _s120->aspectMask);
        }
    }
    if (pDepthStencilAttachment != nullptr) {
        const auto _s121 = pDepthStencilAttachment;
        skip |= ValidateAttachmentReference2(_parentObjects,
            _s121->sType,
            _s121->pNext,
            _s121->attachment,
            _s121->layout,
            _s121->aspectMask);
    }
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
bool ExplicitValidation::ValidateSubpassDependency2(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const uint32_t srcSubpass,
                const uint32_t dstSubpass,
                const VkPipelineStageFlags srcStageMask,
                const VkPipelineStageFlags dstStageMask,
                const VkAccessFlags srcAccessMask,
                const VkAccessFlags dstAccessMask,
                const VkDependencyFlags dependencyFlags,
                const int32_t viewOffset) const {
    bool skip = false;
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
bool ExplicitValidation::ValidateRenderPassCreateInfo2(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const VkRenderPassCreateFlags flags,
                const uint32_t attachmentCount,
                const VkAttachmentDescription2* pAttachments,
                const uint32_t subpassCount,
                const VkSubpassDescription2* pSubpasses,
                const uint32_t dependencyCount,
                const VkSubpassDependency2* pDependencies,
                const uint32_t correlatedViewMaskCount,
                const uint32_t* pCorrelatedViewMasks) const {
    bool skip = false;
    if (pAttachments != nullptr) {
        for (uint32_t _i122 = 0;_i122 < attachmentCount; ++_i122) {
            const auto _s123 = &pAttachments[_i122];
            skip |= ValidateAttachmentDescription2(_parentObjects,
                _s123->sType,
                _s123->pNext,
                _s123->flags,
                _s123->format,
                _s123->samples,
                _s123->loadOp,
                _s123->storeOp,
                _s123->stencilLoadOp,
                _s123->stencilStoreOp,
                _s123->initialLayout,
                _s123->finalLayout);
        }
    }
    if (pSubpasses != nullptr) {
        for (uint32_t _i124 = 0;_i124 < subpassCount; ++_i124) {
            const auto _s125 = &pSubpasses[_i124];
            skip |= ValidateSubpassDescription2(_parentObjects,
                _s125->sType,
                _s125->pNext,
                _s125->flags,
                _s125->pipelineBindPoint,
                _s125->viewMask,
                _s125->inputAttachmentCount,
                _s125->pInputAttachments,
                _s125->colorAttachmentCount,
                _s125->pColorAttachments,
                _s125->pResolveAttachments,
                _s125->pDepthStencilAttachment,
                _s125->preserveAttachmentCount,
                _s125->pPreserveAttachments);
        }
    }
    if (pDependencies != nullptr) {
        for (uint32_t _i126 = 0;_i126 < dependencyCount; ++_i126) {
            const auto _s127 = &pDependencies[_i126];
            skip |= ValidateSubpassDependency2(_parentObjects,
                _s127->sType,
                _s127->pNext,
                _s127->srcSubpass,
                _s127->dstSubpass,
                _s127->srcStageMask,
                _s127->dstStageMask,
                _s127->srcAccessMask,
                _s127->dstAccessMask,
                _s127->dependencyFlags,
                _s127->viewOffset);
        }
    }
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
bool ExplicitValidation::ValidateSubpassBeginInfo(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const VkSubpassContents contents) const {
    bool skip = false;
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
bool ExplicitValidation::ValidateSubpassEndInfo(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext) const {
    bool skip = false;
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
bool ExplicitValidation::ValidatePhysicalDevice8BitStorageFeatures(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const VkBool32 storageBuffer8BitAccess,
                const VkBool32 uniformAndStorageBuffer8BitAccess,
                const VkBool32 storagePushConstant8) const {
    bool skip = false;
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
bool ExplicitValidation::ValidatePhysicalDeviceShaderAtomicInt64Features(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const VkBool32 shaderBufferInt64Atomics,
                const VkBool32 shaderSharedInt64Atomics) const {
    bool skip = false;
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
bool ExplicitValidation::ValidatePhysicalDeviceShaderFloat16Int8Features(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const VkBool32 shaderFloat16,
                const VkBool32 shaderInt8) const {
    bool skip = false;
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
bool ExplicitValidation::ValidateDescriptorSetLayoutBindingFlagsCreateInfo(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const uint32_t bindingCount,
                const VkDescriptorBindingFlags* pBindingFlags) const {
    bool skip = false;
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
bool ExplicitValidation::ValidatePhysicalDeviceDescriptorIndexingFeatures(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const VkBool32 shaderInputAttachmentArrayDynamicIndexing,
                const VkBool32 shaderUniformTexelBufferArrayDynamicIndexing,
                const VkBool32 shaderStorageTexelBufferArrayDynamicIndexing,
                const VkBool32 shaderUniformBufferArrayNonUniformIndexing,
                const VkBool32 shaderSampledImageArrayNonUniformIndexing,
                const VkBool32 shaderStorageBufferArrayNonUniformIndexing,
                const VkBool32 shaderStorageImageArrayNonUniformIndexing,
                const VkBool32 shaderInputAttachmentArrayNonUniformIndexing,
                const VkBool32 shaderUniformTexelBufferArrayNonUniformIndexing,
                const VkBool32 shaderStorageTexelBufferArrayNonUniformIndexing,
                const VkBool32 descriptorBindingUniformBufferUpdateAfterBind,
                const VkBool32 descriptorBindingSampledImageUpdateAfterBind,
                const VkBool32 descriptorBindingStorageImageUpdateAfterBind,
                const VkBool32 descriptorBindingStorageBufferUpdateAfterBind,
                const VkBool32 descriptorBindingUniformTexelBufferUpdateAfterBind,
                const VkBool32 descriptorBindingStorageTexelBufferUpdateAfterBind,
                const VkBool32 descriptorBindingUpdateUnusedWhilePending,
                const VkBool32 descriptorBindingPartiallyBound,
                const VkBool32 descriptorBindingVariableDescriptorCount,
                const VkBool32 runtimeDescriptorArray) const {
    bool skip = false;
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
bool ExplicitValidation::ValidateDescriptorSetVariableDescriptorCountAllocateInfo(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const uint32_t descriptorSetCount,
                const uint32_t* pDescriptorCounts) const {
    bool skip = false;
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
bool ExplicitValidation::ValidateSubpassDescriptionDepthStencilResolve(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const VkResolveModeFlagBits depthResolveMode,
                const VkResolveModeFlagBits stencilResolveMode,
                const VkAttachmentReference2* pDepthStencilResolveAttachment) const {
    bool skip = false;
    if (pDepthStencilResolveAttachment != nullptr) {
        const auto _s128 = pDepthStencilResolveAttachment;
        skip |= ValidateAttachmentReference2(_parentObjects,
            _s128->sType,
            _s128->pNext,
            _s128->attachment,
            _s128->layout,
            _s128->aspectMask);
    }
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
bool ExplicitValidation::ValidatePhysicalDeviceScalarBlockLayoutFeatures(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const VkBool32 scalarBlockLayout) const {
    bool skip = false;
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
bool ExplicitValidation::ValidateImageStencilUsageCreateInfo(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const VkImageUsageFlags stencilUsage) const {
    bool skip = false;
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
bool ExplicitValidation::ValidateSamplerReductionModeCreateInfo(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const VkSamplerReductionMode reductionMode) const {
    bool skip = false;
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
bool ExplicitValidation::ValidatePhysicalDeviceVulkanMemoryModelFeatures(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const VkBool32 vulkanMemoryModel,
                const VkBool32 vulkanMemoryModelDeviceScope,
                const VkBool32 vulkanMemoryModelAvailabilityVisibilityChains) const {
    bool skip = false;
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
bool ExplicitValidation::ValidatePhysicalDeviceImagelessFramebufferFeatures(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const VkBool32 imagelessFramebuffer) const {
    bool skip = false;
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
bool ExplicitValidation::ValidateFramebufferAttachmentImageInfo(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const VkImageCreateFlags flags,
                const VkImageUsageFlags usage,
                const uint32_t width,
                const uint32_t height,
                const uint32_t layerCount,
                const uint32_t viewFormatCount,
                const VkFormat* pViewFormats) const {
    bool skip = false;
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
bool ExplicitValidation::ValidateFramebufferAttachmentsCreateInfo(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const uint32_t attachmentImageInfoCount,
                const VkFramebufferAttachmentImageInfo* pAttachmentImageInfos) const {
    bool skip = false;
    if (pAttachmentImageInfos != nullptr) {
        for (uint32_t _i129 = 0;_i129 < attachmentImageInfoCount; ++_i129) {
            const auto _s130 = &pAttachmentImageInfos[_i129];
            skip |= ValidateFramebufferAttachmentImageInfo(_parentObjects,
                _s130->sType,
                _s130->pNext,
                _s130->flags,
                _s130->usage,
                _s130->width,
                _s130->height,
                _s130->layerCount,
                _s130->viewFormatCount,
                _s130->pViewFormats);
        }
    }
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
bool ExplicitValidation::ValidateRenderPassAttachmentBeginInfo(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const uint32_t attachmentCount,
                const VkImageView* pAttachments) const {
    bool skip = false;
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
bool ExplicitValidation::ValidatePhysicalDeviceUniformBufferStandardLayoutFeatures(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const VkBool32 uniformBufferStandardLayout) const {
    bool skip = false;
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
bool ExplicitValidation::ValidatePhysicalDeviceShaderSubgroupExtendedTypesFeatures(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const VkBool32 shaderSubgroupExtendedTypes) const {
    bool skip = false;
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
bool ExplicitValidation::ValidatePhysicalDeviceSeparateDepthStencilLayoutsFeatures(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const VkBool32 separateDepthStencilLayouts) const {
    bool skip = false;
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
bool ExplicitValidation::ValidateAttachmentReferenceStencilLayout(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const VkImageLayout stencilLayout) const {
    bool skip = false;
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
bool ExplicitValidation::ValidateAttachmentDescriptionStencilLayout(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const VkImageLayout stencilInitialLayout,
                const VkImageLayout stencilFinalLayout) const {
    bool skip = false;
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
bool ExplicitValidation::ValidatePhysicalDeviceHostQueryResetFeatures(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const VkBool32 hostQueryReset) const {
    bool skip = false;
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
bool ExplicitValidation::ValidatePhysicalDeviceTimelineSemaphoreFeatures(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const VkBool32 timelineSemaphore) const {
    bool skip = false;
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
bool ExplicitValidation::ValidateSemaphoreTypeCreateInfo(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const VkSemaphoreType semaphoreType,
                const uint64_t initialValue) const {
    bool skip = false;
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
bool ExplicitValidation::ValidateTimelineSemaphoreSubmitInfo(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const uint32_t waitSemaphoreValueCount,
                const uint64_t* pWaitSemaphoreValues,
                const uint32_t signalSemaphoreValueCount,
                const uint64_t* pSignalSemaphoreValues) const {
    bool skip = false;
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
bool ExplicitValidation::ValidateSemaphoreWaitInfo(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const VkSemaphoreWaitFlags flags,
                const uint32_t semaphoreCount,
                const VkSemaphore* pSemaphores,
                const uint64_t* pValues) const {
    bool skip = false;
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
bool ExplicitValidation::ValidateSemaphoreSignalInfo(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const VkSemaphore semaphore,
                const uint64_t value) const {
    bool skip = false;
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
bool ExplicitValidation::ValidatePhysicalDeviceBufferDeviceAddressFeatures(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const VkBool32 bufferDeviceAddress,
                const VkBool32 bufferDeviceAddressCaptureReplay,
                const VkBool32 bufferDeviceAddressMultiDevice) const {
    bool skip = false;
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
bool ExplicitValidation::ValidateBufferDeviceAddressInfo(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const VkBuffer buffer) const {
    bool skip = false;
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
bool ExplicitValidation::ValidateBufferOpaqueCaptureAddressCreateInfo(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const uint64_t opaqueCaptureAddress) const {
    bool skip = false;
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
bool ExplicitValidation::ValidateMemoryOpaqueCaptureAddressAllocateInfo(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const uint64_t opaqueCaptureAddress) const {
    bool skip = false;
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
bool ExplicitValidation::ValidateDeviceMemoryOpaqueCaptureAddressInfo(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const VkDeviceMemory memory) const {
    bool skip = false;
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
bool ExplicitValidation::ValidatePhysicalDeviceVulkan13Features(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const VkBool32 robustImageAccess,
                const VkBool32 inlineUniformBlock,
                const VkBool32 descriptorBindingInlineUniformBlockUpdateAfterBind,
                const VkBool32 pipelineCreationCacheControl,
                const VkBool32 privateData,
                const VkBool32 shaderDemoteToHelperInvocation,
                const VkBool32 shaderTerminateInvocation,
                const VkBool32 subgroupSizeControl,
                const VkBool32 computeFullSubgroups,
                const VkBool32 synchronization2,
                const VkBool32 textureCompressionASTC_HDR,
                const VkBool32 shaderZeroInitializeWorkgroupMemory,
                const VkBool32 dynamicRendering,
                const VkBool32 shaderIntegerDotProduct,
                const VkBool32 maintenance4) const {
    bool skip = false;
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
bool ExplicitValidation::ValidatePipelineCreationFeedbackCreateInfo(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const VkPipelineCreationFeedback* pPipelineCreationFeedback,
                const uint32_t pipelineStageCreationFeedbackCount,
                const VkPipelineCreationFeedback* pPipelineStageCreationFeedbacks) const {
    bool skip = false;
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
bool ExplicitValidation::ValidatePhysicalDeviceShaderTerminateInvocationFeatures(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const VkBool32 shaderTerminateInvocation) const {
    bool skip = false;
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
bool ExplicitValidation::ValidatePhysicalDeviceShaderDemoteToHelperInvocationFeatures(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const VkBool32 shaderDemoteToHelperInvocation) const {
    bool skip = false;
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
bool ExplicitValidation::ValidatePhysicalDevicePrivateDataFeatures(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const VkBool32 privateData) const {
    bool skip = false;
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
bool ExplicitValidation::ValidateDevicePrivateDataCreateInfo(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const uint32_t privateDataSlotRequestCount) const {
    bool skip = false;
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
bool ExplicitValidation::ValidatePrivateDataSlotCreateInfo(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const VkPrivateDataSlotCreateFlags flags) const {
    bool skip = false;
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
bool ExplicitValidation::ValidatePhysicalDevicePipelineCreationCacheControlFeatures(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const VkBool32 pipelineCreationCacheControl) const {
    bool skip = false;
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
bool ExplicitValidation::ValidateMemoryBarrier2(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const VkPipelineStageFlags2 srcStageMask,
                const VkAccessFlags2 srcAccessMask,
                const VkPipelineStageFlags2 dstStageMask,
                const VkAccessFlags2 dstAccessMask) const {
    bool skip = false;
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
bool ExplicitValidation::ValidateBufferMemoryBarrier2(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const VkPipelineStageFlags2 srcStageMask,
                const VkAccessFlags2 srcAccessMask,
                const VkPipelineStageFlags2 dstStageMask,
                const VkAccessFlags2 dstAccessMask,
                const uint32_t srcQueueFamilyIndex,
                const uint32_t dstQueueFamilyIndex,
                const VkBuffer buffer,
                const VkDeviceSize offset,
                const VkDeviceSize size) const {
    bool skip = false;
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
bool ExplicitValidation::ValidateImageMemoryBarrier2(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const VkPipelineStageFlags2 srcStageMask,
                const VkAccessFlags2 srcAccessMask,
                const VkPipelineStageFlags2 dstStageMask,
                const VkAccessFlags2 dstAccessMask,
                const VkImageLayout oldLayout,
                const VkImageLayout newLayout,
                const uint32_t srcQueueFamilyIndex,
                const uint32_t dstQueueFamilyIndex,
                const VkImage image,
                const VkImageSubresourceRange subresourceRange) const {
    bool skip = false;
    const auto _s131 = &subresourceRange;
    skip |= ValidateImageSubresourceRange(_parentObjects,
        _s131->aspectMask,
        _s131->baseMipLevel,
        _s131->levelCount,
        _s131->baseArrayLayer,
        _s131->layerCount);
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
bool ExplicitValidation::ValidateDependencyInfo(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const VkDependencyFlags dependencyFlags,
                const uint32_t memoryBarrierCount,
                const VkMemoryBarrier2* pMemoryBarriers,
                const uint32_t bufferMemoryBarrierCount,
                const VkBufferMemoryBarrier2* pBufferMemoryBarriers,
                const uint32_t imageMemoryBarrierCount,
                const VkImageMemoryBarrier2* pImageMemoryBarriers) const {
    bool skip = false;
    if (pMemoryBarriers != nullptr) {
        for (uint32_t _i132 = 0;_i132 < memoryBarrierCount; ++_i132) {
            const auto _s133 = &pMemoryBarriers[_i132];
            skip |= ValidateMemoryBarrier2(_parentObjects,
                _s133->sType,
                _s133->pNext,
                _s133->srcStageMask,
                _s133->srcAccessMask,
                _s133->dstStageMask,
                _s133->dstAccessMask);
        }
    }
    if (pBufferMemoryBarriers != nullptr) {
        for (uint32_t _i134 = 0;_i134 < bufferMemoryBarrierCount; ++_i134) {
            const auto _s135 = &pBufferMemoryBarriers[_i134];
            skip |= ValidateBufferMemoryBarrier2(_parentObjects,
                _s135->sType,
                _s135->pNext,
                _s135->srcStageMask,
                _s135->srcAccessMask,
                _s135->dstStageMask,
                _s135->dstAccessMask,
                _s135->srcQueueFamilyIndex,
                _s135->dstQueueFamilyIndex,
                _s135->buffer,
                _s135->offset,
                _s135->size);
        }
    }
    if (pImageMemoryBarriers != nullptr) {
        for (uint32_t _i136 = 0;_i136 < imageMemoryBarrierCount; ++_i136) {
            const auto _s137 = &pImageMemoryBarriers[_i136];
            skip |= ValidateImageMemoryBarrier2(_parentObjects,
                _s137->sType,
                _s137->pNext,
                _s137->srcStageMask,
                _s137->srcAccessMask,
                _s137->dstStageMask,
                _s137->dstAccessMask,
                _s137->oldLayout,
                _s137->newLayout,
                _s137->srcQueueFamilyIndex,
                _s137->dstQueueFamilyIndex,
                _s137->image,
                _s137->subresourceRange);
        }
    }
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
bool ExplicitValidation::ValidateSemaphoreSubmitInfo(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const VkSemaphore semaphore,
                const uint64_t value,
                const VkPipelineStageFlags2 stageMask,
                const uint32_t deviceIndex) const {
    bool skip = false;
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
bool ExplicitValidation::ValidateCommandBufferSubmitInfo(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const VkCommandBuffer commandBuffer,
                const uint32_t deviceMask) const {
    bool skip = false;
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
bool ExplicitValidation::ValidateSubmitInfo2(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const VkSubmitFlags flags,
                const uint32_t waitSemaphoreInfoCount,
                const VkSemaphoreSubmitInfo* pWaitSemaphoreInfos,
                const uint32_t commandBufferInfoCount,
                const VkCommandBufferSubmitInfo* pCommandBufferInfos,
                const uint32_t signalSemaphoreInfoCount,
                const VkSemaphoreSubmitInfo* pSignalSemaphoreInfos) const {
    bool skip = false;
    if (pWaitSemaphoreInfos != nullptr) {
        for (uint32_t _i138 = 0;_i138 < waitSemaphoreInfoCount; ++_i138) {
            const auto _s139 = &pWaitSemaphoreInfos[_i138];
            skip |= ValidateSemaphoreSubmitInfo(_parentObjects,
                _s139->sType,
                _s139->pNext,
                _s139->semaphore,
                _s139->value,
                _s139->stageMask,
                _s139->deviceIndex);
        }
    }
    if (pCommandBufferInfos != nullptr) {
        for (uint32_t _i140 = 0;_i140 < commandBufferInfoCount; ++_i140) {
            const auto _s141 = &pCommandBufferInfos[_i140];
            skip |= ValidateCommandBufferSubmitInfo(_parentObjects,
                _s141->sType,
                _s141->pNext,
                _s141->commandBuffer,
                _s141->deviceMask);
        }
    }
    if (pSignalSemaphoreInfos != nullptr) {
        for (uint32_t _i142 = 0;_i142 < signalSemaphoreInfoCount; ++_i142) {
            const auto _s143 = &pSignalSemaphoreInfos[_i142];
            skip |= ValidateSemaphoreSubmitInfo(_parentObjects,
                _s143->sType,
                _s143->pNext,
                _s143->semaphore,
                _s143->value,
                _s143->stageMask,
                _s143->deviceIndex);
        }
    }
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
bool ExplicitValidation::ValidatePhysicalDeviceSynchronization2Features(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const VkBool32 synchronization2) const {
    bool skip = false;
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
bool ExplicitValidation::ValidatePhysicalDeviceZeroInitializeWorkgroupMemoryFeatures(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const VkBool32 shaderZeroInitializeWorkgroupMemory) const {
    bool skip = false;
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
bool ExplicitValidation::ValidatePhysicalDeviceImageRobustnessFeatures(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const VkBool32 robustImageAccess) const {
    bool skip = false;
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
bool ExplicitValidation::ValidateBufferCopy2(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const VkDeviceSize srcOffset,
                const VkDeviceSize dstOffset,
                const VkDeviceSize size) const {
    bool skip = false;
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
bool ExplicitValidation::ValidateCopyBufferInfo2(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const VkBuffer srcBuffer,
                const VkBuffer dstBuffer,
                const uint32_t regionCount,
                const VkBufferCopy2* pRegions) const {
    bool skip = false;
    if (pRegions != nullptr) {
        for (uint32_t _i144 = 0;_i144 < regionCount; ++_i144) {
            const auto _s145 = &pRegions[_i144];
            skip |= ValidateBufferCopy2(_parentObjects,
                _s145->sType,
                _s145->pNext,
                _s145->srcOffset,
                _s145->dstOffset,
                _s145->size);
        }
    }
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
bool ExplicitValidation::ValidateImageCopy2(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const VkImageSubresourceLayers srcSubresource,
                const VkOffset3D srcOffset,
                const VkImageSubresourceLayers dstSubresource,
                const VkOffset3D dstOffset,
                const VkExtent3D extent) const {
    bool skip = false;
    const auto _s146 = &srcSubresource;
    skip |= ValidateImageSubresourceLayers(_parentObjects,
        _s146->aspectMask,
        _s146->mipLevel,
        _s146->baseArrayLayer,
        _s146->layerCount);
    const auto _s147 = &srcOffset;
    skip |= ValidateOffset3D(_parentObjects,
        _s147->x,
        _s147->y,
        _s147->z);
    const auto _s148 = &dstSubresource;
    skip |= ValidateImageSubresourceLayers(_parentObjects,
        _s148->aspectMask,
        _s148->mipLevel,
        _s148->baseArrayLayer,
        _s148->layerCount);
    const auto _s149 = &dstOffset;
    skip |= ValidateOffset3D(_parentObjects,
        _s149->x,
        _s149->y,
        _s149->z);
    const auto _s150 = &extent;
    skip |= ValidateExtent3D(_parentObjects,
        _s150->width,
        _s150->height,
        _s150->depth);
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
bool ExplicitValidation::ValidateCopyImageInfo2(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const VkImage srcImage,
                const VkImageLayout srcImageLayout,
                const VkImage dstImage,
                const VkImageLayout dstImageLayout,
                const uint32_t regionCount,
                const VkImageCopy2* pRegions) const {
    bool skip = false;
    if (pRegions != nullptr) {
        for (uint32_t _i151 = 0;_i151 < regionCount; ++_i151) {
            const auto _s152 = &pRegions[_i151];
            skip |= ValidateImageCopy2(_parentObjects,
                _s152->sType,
                _s152->pNext,
                _s152->srcSubresource,
                _s152->srcOffset,
                _s152->dstSubresource,
                _s152->dstOffset,
                _s152->extent);
        }
    }
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
bool ExplicitValidation::ValidateBufferImageCopy2(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const VkDeviceSize bufferOffset,
                const uint32_t bufferRowLength,
                const uint32_t bufferImageHeight,
                const VkImageSubresourceLayers imageSubresource,
                const VkOffset3D imageOffset,
                const VkExtent3D imageExtent) const {
    bool skip = false;
    const auto _s153 = &imageSubresource;
    skip |= ValidateImageSubresourceLayers(_parentObjects,
        _s153->aspectMask,
        _s153->mipLevel,
        _s153->baseArrayLayer,
        _s153->layerCount);
    const auto _s154 = &imageOffset;
    skip |= ValidateOffset3D(_parentObjects,
        _s154->x,
        _s154->y,
        _s154->z);
    const auto _s155 = &imageExtent;
    skip |= ValidateExtent3D(_parentObjects,
        _s155->width,
        _s155->height,
        _s155->depth);
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
bool ExplicitValidation::ValidateCopyBufferToImageInfo2(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const VkBuffer srcBuffer,
                const VkImage dstImage,
                const VkImageLayout dstImageLayout,
                const uint32_t regionCount,
                const VkBufferImageCopy2* pRegions) const {
    bool skip = false;
    auto srcBuffer_ = Get<BUFFER_STATE>(srcBuffer);
    auto dstImage_ = Get<IMAGE_STATE>(dstImage);

    {
        if ((Builtin_create_info(dstImage_).imageType == VK_IMAGE_TYPE_1D))
        {
            for (uint32_t i3 = 0; i3 < regionCount; ++i3)
            {
                const VkBufferImageCopy2 &region_0 = pRegions[i3];
                {
                    if (!((region_0.imageOffset.y == 0)))
                    {
                        const LogObjectList objlist{dstImage};
                        skip |= LogFail(objlist, "VUID-VkCopyBufferToImageInfo2-srcImage-00199",
                            R"(    ${vu-keyword}if${} dstImage${vu-value}{{%s}}${}.${vu-builtin}create_info${}().imageType ${vu-operator}==${} VK_IMAGE_TYPE_1D:
      ${vu-keyword}for${} region${vu-value}{{@%)" PRIu32 R"(}}${} ${vu-operator}in${} pRegions:
        ${vu-builtin}require${}(region${vu-value}{{@%)" PRIu32 R"(}}${}.imageOffset.y${vu-value}{{%)" PRId32 R"(}}${} ${vu-operator}==${} ${vu-number}0${})${vu-fail} <-- Failing condition (Values captured at this instant)${}
        ${vu-builtin}require${}(region.imageExtent.height ${vu-operator}==${} ${vu-number}1${}))",
                            report_data->FormatHandle(dstImage).c_str(),
                            i3,
                            i3,
                            region_0.imageOffset.y);
                    }

                    if (!((region_0.imageExtent.height == 1)))
                    {
                        const LogObjectList objlist{dstImage};
                        skip |= LogFail(objlist, "VUID-VkCopyBufferToImageInfo2-srcImage-00199",
                            R"(    ${vu-keyword}if${} dstImage${vu-value}{{%s}}${}.${vu-builtin}create_info${}().imageType ${vu-operator}==${} VK_IMAGE_TYPE_1D:
      ${vu-keyword}for${} region${vu-value}{{@%)" PRIu32 R"(}}${} ${vu-operator}in${} pRegions:
        ${vu-builtin}require${}(region${vu-value}{{@%)" PRIu32 R"(}}${}.imageOffset.y${vu-value}{{%)" PRId32 R"(}}${} ${vu-operator}==${} ${vu-number}0${})
        ${vu-builtin}require${}(region${vu-value}{{@%)" PRIu32 R"(}}${}.imageExtent.height${vu-value}{{%)" PRIu32 R"(}}${} ${vu-operator}==${} ${vu-number}1${})${vu-fail} <-- Failing condition (Values captured at this instant)${})",
                            report_data->FormatHandle(dstImage).c_str(),
                            i3,
                            i3,
                            region_0.imageOffset.y,
                            i3,
                            region_0.imageExtent.height);
                    }

                }
            }

        }

    }
    if (pRegions != nullptr) {
        for (uint32_t _i156 = 0;_i156 < regionCount; ++_i156) {
            const auto _s157 = &pRegions[_i156];
            skip |= ValidateBufferImageCopy2(_parentObjects,
                _s157->sType,
                _s157->pNext,
                _s157->bufferOffset,
                _s157->bufferRowLength,
                _s157->bufferImageHeight,
                _s157->imageSubresource,
                _s157->imageOffset,
                _s157->imageExtent);
        }
    }
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
bool ExplicitValidation::ValidateCopyImageToBufferInfo2(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const VkImage srcImage,
                const VkImageLayout srcImageLayout,
                const VkBuffer dstBuffer,
                const uint32_t regionCount,
                const VkBufferImageCopy2* pRegions) const {
    bool skip = false;
    auto srcImage_ = Get<IMAGE_STATE>(srcImage);
    auto dstBuffer_ = Get<BUFFER_STATE>(dstBuffer);

    {
        if ((Builtin_create_info(srcImage_).imageType == VK_IMAGE_TYPE_1D))
        {
            for (uint32_t i3 = 0; i3 < regionCount; ++i3)
            {
                const VkBufferImageCopy2 &region_0 = pRegions[i3];
                {
                    if (!((region_0.imageOffset.y == 0)))
                    {
                        const LogObjectList objlist{srcImage};
                        skip |= LogFail(objlist, "VUID-VkCopyImageToBufferInfo2-srcImage-00199",
                            R"(    ${vu-keyword}if${} srcImage${vu-value}{{%s}}${}.${vu-builtin}create_info${}().imageType ${vu-operator}==${} VK_IMAGE_TYPE_1D:
      ${vu-keyword}for${} region${vu-value}{{@%)" PRIu32 R"(}}${} ${vu-operator}in${} pRegions:
        ${vu-builtin}require${}(region${vu-value}{{@%)" PRIu32 R"(}}${}.imageOffset.y${vu-value}{{%)" PRId32 R"(}}${} ${vu-operator}==${} ${vu-number}0${})${vu-fail} <-- Failing condition (Values captured at this instant)${}
        ${vu-builtin}require${}(region.imageExtent.height ${vu-operator}==${} ${vu-number}1${}))",
                            report_data->FormatHandle(srcImage).c_str(),
                            i3,
                            i3,
                            region_0.imageOffset.y);
                    }

                    if (!((region_0.imageExtent.height == 1)))
                    {
                        const LogObjectList objlist{srcImage};
                        skip |= LogFail(objlist, "VUID-VkCopyImageToBufferInfo2-srcImage-00199",
                            R"(    ${vu-keyword}if${} srcImage${vu-value}{{%s}}${}.${vu-builtin}create_info${}().imageType ${vu-operator}==${} VK_IMAGE_TYPE_1D:
      ${vu-keyword}for${} region${vu-value}{{@%)" PRIu32 R"(}}${} ${vu-operator}in${} pRegions:
        ${vu-builtin}require${}(region${vu-value}{{@%)" PRIu32 R"(}}${}.imageOffset.y${vu-value}{{%)" PRId32 R"(}}${} ${vu-operator}==${} ${vu-number}0${})
        ${vu-builtin}require${}(region${vu-value}{{@%)" PRIu32 R"(}}${}.imageExtent.height${vu-value}{{%)" PRIu32 R"(}}${} ${vu-operator}==${} ${vu-number}1${})${vu-fail} <-- Failing condition (Values captured at this instant)${})",
                            report_data->FormatHandle(srcImage).c_str(),
                            i3,
                            i3,
                            region_0.imageOffset.y,
                            i3,
                            region_0.imageExtent.height);
                    }

                }
            }

        }

    }
    if (pRegions != nullptr) {
        for (uint32_t _i158 = 0;_i158 < regionCount; ++_i158) {
            const auto _s159 = &pRegions[_i158];
            skip |= ValidateBufferImageCopy2(_parentObjects,
                _s159->sType,
                _s159->pNext,
                _s159->bufferOffset,
                _s159->bufferRowLength,
                _s159->bufferImageHeight,
                _s159->imageSubresource,
                _s159->imageOffset,
                _s159->imageExtent);
        }
    }
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
bool ExplicitValidation::ValidateImageBlit2(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const VkImageSubresourceLayers srcSubresource,
                const VkOffset3D srcOffsets[2],
                const VkImageSubresourceLayers dstSubresource,
                const VkOffset3D dstOffsets[2]) const {
    bool skip = false;
    const auto _s160 = &srcSubresource;
    skip |= ValidateImageSubresourceLayers(_parentObjects,
        _s160->aspectMask,
        _s160->mipLevel,
        _s160->baseArrayLayer,
        _s160->layerCount);
    if (srcOffsets != nullptr) {
        for (uint32_t _i161 = 0;_i161 < 2; ++_i161) {
            const auto _s162 = &srcOffsets[_i161];
            skip |= ValidateOffset3D(_parentObjects,
                _s162->x,
                _s162->y,
                _s162->z);
        }
    }
    const auto _s163 = &dstSubresource;
    skip |= ValidateImageSubresourceLayers(_parentObjects,
        _s163->aspectMask,
        _s163->mipLevel,
        _s163->baseArrayLayer,
        _s163->layerCount);
    if (dstOffsets != nullptr) {
        for (uint32_t _i164 = 0;_i164 < 2; ++_i164) {
            const auto _s165 = &dstOffsets[_i164];
            skip |= ValidateOffset3D(_parentObjects,
                _s165->x,
                _s165->y,
                _s165->z);
        }
    }
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
bool ExplicitValidation::ValidateBlitImageInfo2(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const VkImage srcImage,
                const VkImageLayout srcImageLayout,
                const VkImage dstImage,
                const VkImageLayout dstImageLayout,
                const uint32_t regionCount,
                const VkImageBlit2* pRegions,
                const VkFilter filter) const {
    bool skip = false;
    if (pRegions != nullptr) {
        for (uint32_t _i166 = 0;_i166 < regionCount; ++_i166) {
            const auto _s167 = &pRegions[_i166];
            skip |= ValidateImageBlit2(_parentObjects,
                _s167->sType,
                _s167->pNext,
                _s167->srcSubresource,
                _s167->srcOffsets,
                _s167->dstSubresource,
                _s167->dstOffsets);
        }
    }
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
bool ExplicitValidation::ValidateImageResolve2(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const VkImageSubresourceLayers srcSubresource,
                const VkOffset3D srcOffset,
                const VkImageSubresourceLayers dstSubresource,
                const VkOffset3D dstOffset,
                const VkExtent3D extent) const {
    bool skip = false;
    const auto _s168 = &srcSubresource;
    skip |= ValidateImageSubresourceLayers(_parentObjects,
        _s168->aspectMask,
        _s168->mipLevel,
        _s168->baseArrayLayer,
        _s168->layerCount);
    const auto _s169 = &srcOffset;
    skip |= ValidateOffset3D(_parentObjects,
        _s169->x,
        _s169->y,
        _s169->z);
    const auto _s170 = &dstSubresource;
    skip |= ValidateImageSubresourceLayers(_parentObjects,
        _s170->aspectMask,
        _s170->mipLevel,
        _s170->baseArrayLayer,
        _s170->layerCount);
    const auto _s171 = &dstOffset;
    skip |= ValidateOffset3D(_parentObjects,
        _s171->x,
        _s171->y,
        _s171->z);
    const auto _s172 = &extent;
    skip |= ValidateExtent3D(_parentObjects,
        _s172->width,
        _s172->height,
        _s172->depth);
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
bool ExplicitValidation::ValidateResolveImageInfo2(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const VkImage srcImage,
                const VkImageLayout srcImageLayout,
                const VkImage dstImage,
                const VkImageLayout dstImageLayout,
                const uint32_t regionCount,
                const VkImageResolve2* pRegions) const {
    bool skip = false;
    if (pRegions != nullptr) {
        for (uint32_t _i173 = 0;_i173 < regionCount; ++_i173) {
            const auto _s174 = &pRegions[_i173];
            skip |= ValidateImageResolve2(_parentObjects,
                _s174->sType,
                _s174->pNext,
                _s174->srcSubresource,
                _s174->srcOffset,
                _s174->dstSubresource,
                _s174->dstOffset,
                _s174->extent);
        }
    }
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
bool ExplicitValidation::ValidatePhysicalDeviceSubgroupSizeControlFeatures(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const VkBool32 subgroupSizeControl,
                const VkBool32 computeFullSubgroups) const {
    bool skip = false;
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
bool ExplicitValidation::ValidatePhysicalDeviceInlineUniformBlockFeatures(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const VkBool32 inlineUniformBlock,
                const VkBool32 descriptorBindingInlineUniformBlockUpdateAfterBind) const {
    bool skip = false;
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
bool ExplicitValidation::ValidateWriteDescriptorSetInlineUniformBlock(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const uint32_t dataSize,
                const void* pData) const {
    bool skip = false;
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
bool ExplicitValidation::ValidateDescriptorPoolInlineUniformBlockCreateInfo(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const uint32_t maxInlineUniformBlockBindings) const {
    bool skip = false;
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
bool ExplicitValidation::ValidatePhysicalDeviceTextureCompressionASTCHDRFeatures(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const VkBool32 textureCompressionASTC_HDR) const {
    bool skip = false;
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
bool ExplicitValidation::ValidateRenderingAttachmentInfo(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const VkImageView imageView,
                const VkImageLayout imageLayout,
                const VkResolveModeFlagBits resolveMode,
                const VkImageView resolveImageView,
                const VkImageLayout resolveImageLayout,
                const VkAttachmentLoadOp loadOp,
                const VkAttachmentStoreOp storeOp,
                const VkClearValue clearValue) const {
    bool skip = false;
    const auto _s175 = &clearValue;
    skip |= ValidateClearValue(_parentObjects,
        _s175->color,
        _s175->depthStencil);
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
bool ExplicitValidation::ValidateRenderingInfo(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const VkRenderingFlags flags,
                const VkRect2D renderArea,
                const uint32_t layerCount,
                const uint32_t viewMask,
                const uint32_t colorAttachmentCount,
                const VkRenderingAttachmentInfo* pColorAttachments,
                const VkRenderingAttachmentInfo* pDepthAttachment,
                const VkRenderingAttachmentInfo* pStencilAttachment) const {
    bool skip = false;

    {
        for (uint32_t i2 = 0; i2 < colorAttachmentCount; ++i2)
        {
            const VkRenderingAttachmentInfo &attachment_0 = pColorAttachments[i2];
            {
                if (((attachment_0.imageView != VK_NULL_HANDLE)&&(attachment_0.resolveMode != VK_RESOLVE_MODE_NONE)))
                {
                    VkImageLayout resolveLayout_1=attachment_0.resolveImageLayout;
                    if (!(((resolveLayout_1 != VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_STENCIL_ATTACHMENT_OPTIMAL)&&(resolveLayout_1 != VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_STENCIL_READ_ONLY_OPTIMAL))))
                    {
                        const LogObjectList objlist{attachment_0.imageView};
                        skip |= LogFail(objlist, "VUID-VkRenderingInfo-colorAttachmentCount-06097",
                            R"(    ${vu-keyword}for${} attachment${vu-value}{{@%)" PRIu32 R"(}}${} ${vu-operator}in${} pColorAttachments:
      ${vu-keyword}if${} (attachment${vu-value}{{@%)" PRIu32 R"(}}${}.imageView${vu-value}{{%s}}${} ${vu-operator}!=${} VK_NULL_HANDLE ${vu-operator}and${}
              attachment${vu-value}{{@%)" PRIu32 R"(}}${}.resolveMode ${vu-operator}!=${} VK_RESOLVE_MODE_NONE):
        resolveLayout ${vu-operator}=${} attachment${vu-value}{{@%)" PRIu32 R"(}}${}.resolveImageLayout
        ${vu-builtin}require${}(resolveLayout ${vu-operator}!=${} VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_STENCIL_ATTACHMENT_OPTIMAL ${vu-operator}and${}
                resolveLayout ${vu-operator}!=${} VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_STENCIL_READ_ONLY_OPTIMAL)${vu-fail} <-- Failing condition (Values captured at this instant)${})",
                            i2,
                            i2,
                            report_data->FormatHandle(attachment_0.imageView).c_str(),
                            i2,
                            i2);
                    }

                }

            }
        }

    }
    const auto _s176 = &renderArea;
    skip |= ValidateRect2D(_parentObjects,
        _s176->offset,
        _s176->extent);
    if (pColorAttachments != nullptr) {
        for (uint32_t _i177 = 0;_i177 < colorAttachmentCount; ++_i177) {
            const auto _s178 = &pColorAttachments[_i177];
            skip |= ValidateRenderingAttachmentInfo(_parentObjects,
                _s178->sType,
                _s178->pNext,
                _s178->imageView,
                _s178->imageLayout,
                _s178->resolveMode,
                _s178->resolveImageView,
                _s178->resolveImageLayout,
                _s178->loadOp,
                _s178->storeOp,
                _s178->clearValue);
        }
    }
    if (pDepthAttachment != nullptr) {
        const auto _s179 = pDepthAttachment;
        skip |= ValidateRenderingAttachmentInfo(_parentObjects,
            _s179->sType,
            _s179->pNext,
            _s179->imageView,
            _s179->imageLayout,
            _s179->resolveMode,
            _s179->resolveImageView,
            _s179->resolveImageLayout,
            _s179->loadOp,
            _s179->storeOp,
            _s179->clearValue);
    }
    if (pStencilAttachment != nullptr) {
        const auto _s180 = pStencilAttachment;
        skip |= ValidateRenderingAttachmentInfo(_parentObjects,
            _s180->sType,
            _s180->pNext,
            _s180->imageView,
            _s180->imageLayout,
            _s180->resolveMode,
            _s180->resolveImageView,
            _s180->resolveImageLayout,
            _s180->loadOp,
            _s180->storeOp,
            _s180->clearValue);
    }
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
bool ExplicitValidation::ValidatePipelineRenderingCreateInfo(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const uint32_t viewMask,
                const uint32_t colorAttachmentCount,
                const VkFormat* pColorAttachmentFormats,
                const VkFormat depthAttachmentFormat,
                const VkFormat stencilAttachmentFormat) const {
    bool skip = false;
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
bool ExplicitValidation::ValidatePhysicalDeviceDynamicRenderingFeatures(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const VkBool32 dynamicRendering) const {
    bool skip = false;
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
bool ExplicitValidation::ValidateCommandBufferInheritanceRenderingInfo(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const VkRenderingFlags flags,
                const uint32_t viewMask,
                const uint32_t colorAttachmentCount,
                const VkFormat* pColorAttachmentFormats,
                const VkFormat depthAttachmentFormat,
                const VkFormat stencilAttachmentFormat,
                const VkSampleCountFlagBits rasterizationSamples) const {
    bool skip = false;
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
bool ExplicitValidation::ValidatePhysicalDeviceShaderIntegerDotProductFeatures(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const VkBool32 shaderIntegerDotProduct) const {
    bool skip = false;
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
bool ExplicitValidation::ValidatePhysicalDeviceMaintenance4Features(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const VkBool32 maintenance4) const {
    bool skip = false;
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
bool ExplicitValidation::ValidateDeviceBufferMemoryRequirements(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const VkBufferCreateInfo* pCreateInfo) const {
    bool skip = false;
    if (pCreateInfo != nullptr) {
        const auto _s181 = pCreateInfo;
        skip |= ValidateBufferCreateInfo(_parentObjects,
            _s181->sType,
            _s181->pNext,
            _s181->flags,
            _s181->size,
            _s181->usage,
            _s181->sharingMode,
            _s181->queueFamilyIndexCount,
            _s181->pQueueFamilyIndices);
    }
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
bool ExplicitValidation::ValidateDeviceImageMemoryRequirements(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const VkImageCreateInfo* pCreateInfo,
                const VkImageAspectFlagBits planeAspect) const {
    bool skip = false;
    if (pCreateInfo != nullptr) {
        const auto _s182 = pCreateInfo;
        skip |= ValidateImageCreateInfo(_parentObjects,
            _s182->sType,
            _s182->pNext,
            _s182->flags,
            _s182->imageType,
            _s182->format,
            _s182->extent,
            _s182->mipLevels,
            _s182->arrayLayers,
            _s182->samples,
            _s182->tiling,
            _s182->usage,
            _s182->sharingMode,
            _s182->queueFamilyIndexCount,
            _s182->pQueueFamilyIndices,
            _s182->initialLayout);
    }
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
bool ExplicitValidation::ValidateSwapchainCreateInfoKHR(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const VkSwapchainCreateFlagsKHR flags,
                const VkSurfaceKHR surface,
                const uint32_t minImageCount,
                const VkFormat imageFormat,
                const VkColorSpaceKHR imageColorSpace,
                const VkExtent2D imageExtent,
                const uint32_t imageArrayLayers,
                const VkImageUsageFlags imageUsage,
                const VkSharingMode imageSharingMode,
                const uint32_t queueFamilyIndexCount,
                const uint32_t* pQueueFamilyIndices,
                const VkSurfaceTransformFlagBitsKHR preTransform,
                const VkCompositeAlphaFlagBitsKHR compositeAlpha,
                const VkPresentModeKHR presentMode,
                const VkBool32 clipped,
                const VkSwapchainKHR oldSwapchain) const {
    bool skip = false;
    const auto _s183 = &imageExtent;
    skip |= ValidateExtent2D(_parentObjects,
        _s183->width,
        _s183->height);
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
bool ExplicitValidation::ValidatePresentInfoKHR(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const uint32_t waitSemaphoreCount,
                const VkSemaphore* pWaitSemaphores,
                const uint32_t swapchainCount,
                const VkSwapchainKHR* pSwapchains,
                const uint32_t* pImageIndices,
                const VkResult* pResults) const {
    bool skip = false;
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
bool ExplicitValidation::ValidateImageSwapchainCreateInfoKHR(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const VkSwapchainKHR swapchain) const {
    bool skip = false;
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
bool ExplicitValidation::ValidateBindImageMemorySwapchainInfoKHR(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const VkSwapchainKHR swapchain,
                const uint32_t imageIndex) const {
    bool skip = false;
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
bool ExplicitValidation::ValidateAcquireNextImageInfoKHR(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const VkSwapchainKHR swapchain,
                const uint64_t timeout,
                const VkSemaphore semaphore,
                const VkFence fence,
                const uint32_t deviceMask) const {
    bool skip = false;
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
bool ExplicitValidation::ValidateDeviceGroupPresentInfoKHR(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const uint32_t swapchainCount,
                const uint32_t* pDeviceMasks,
                const VkDeviceGroupPresentModeFlagBitsKHR mode) const {
    bool skip = false;
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
bool ExplicitValidation::ValidateDeviceGroupSwapchainCreateInfoKHR(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const VkDeviceGroupPresentModeFlagsKHR modes) const {
    bool skip = false;
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
bool ExplicitValidation::ValidateDisplayModeParametersKHR(const LogObjectList &_parentObjects,
                const VkExtent2D visibleRegion,
                const uint32_t refreshRate) const {
    bool skip = false;
    const auto _s184 = &visibleRegion;
    skip |= ValidateExtent2D(_parentObjects,
        _s184->width,
        _s184->height);
    return skip;
}
bool ExplicitValidation::ValidateDisplayModeCreateInfoKHR(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const VkDisplayModeCreateFlagsKHR flags,
                const VkDisplayModeParametersKHR parameters) const {
    bool skip = false;
    const auto _s185 = &parameters;
    skip |= ValidateDisplayModeParametersKHR(_parentObjects,
        _s185->visibleRegion,
        _s185->refreshRate);
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
bool ExplicitValidation::ValidateDisplaySurfaceCreateInfoKHR(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const VkDisplaySurfaceCreateFlagsKHR flags,
                const VkDisplayModeKHR displayMode,
                const uint32_t planeIndex,
                const uint32_t planeStackIndex,
                const VkSurfaceTransformFlagBitsKHR transform,
                const float globalAlpha,
                const VkDisplayPlaneAlphaFlagBitsKHR alphaMode,
                const VkExtent2D imageExtent) const {
    bool skip = false;
    const auto _s186 = &imageExtent;
    skip |= ValidateExtent2D(_parentObjects,
        _s186->width,
        _s186->height);
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
bool ExplicitValidation::ValidateDisplayPresentInfoKHR(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const VkRect2D srcRect,
                const VkRect2D dstRect,
                const VkBool32 persistent) const {
    bool skip = false;
    const auto _s187 = &srcRect;
    skip |= ValidateRect2D(_parentObjects,
        _s187->offset,
        _s187->extent);
    const auto _s188 = &dstRect;
    skip |= ValidateRect2D(_parentObjects,
        _s188->offset,
        _s188->extent);
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
#ifdef VK_USE_PLATFORM_XLIB_KHR
bool ExplicitValidation::ValidateXlibSurfaceCreateInfoKHR(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const VkXlibSurfaceCreateFlagsKHR flags,
                const Display* dpy,
                const Window window) const {
    bool skip = false;
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
#endif
#ifdef VK_USE_PLATFORM_XCB_KHR
bool ExplicitValidation::ValidateXcbSurfaceCreateInfoKHR(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const VkXcbSurfaceCreateFlagsKHR flags,
                const xcb_connection_t* connection,
                const xcb_window_t window) const {
    bool skip = false;
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
#endif
#ifdef VK_USE_PLATFORM_WAYLAND_KHR
bool ExplicitValidation::ValidateWaylandSurfaceCreateInfoKHR(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const VkWaylandSurfaceCreateFlagsKHR flags,
                const struct wl_display* display,
                const struct wl_surface* surface) const {
    bool skip = false;
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
#endif
#ifdef VK_USE_PLATFORM_ANDROID_KHR
bool ExplicitValidation::ValidateAndroidSurfaceCreateInfoKHR(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const VkAndroidSurfaceCreateFlagsKHR flags,
                const struct ANativeWindow* window) const {
    bool skip = false;
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
#endif
#ifdef VK_USE_PLATFORM_WIN32_KHR
bool ExplicitValidation::ValidateWin32SurfaceCreateInfoKHR(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const VkWin32SurfaceCreateFlagsKHR flags,
                const HINSTANCE hinstance,
                const HWND hwnd) const {
    bool skip = false;
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
#endif
bool ExplicitValidation::ValidateVideoProfileInfoKHR(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const VkVideoCodecOperationFlagBitsKHR videoCodecOperation,
                const VkVideoChromaSubsamplingFlagsKHR chromaSubsampling,
                const VkVideoComponentBitDepthFlagsKHR lumaBitDepth,
                const VkVideoComponentBitDepthFlagsKHR chromaBitDepth) const {
    bool skip = false;
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
bool ExplicitValidation::ValidateVideoProfileListInfoKHR(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const uint32_t profileCount,
                const VkVideoProfileInfoKHR* pProfiles) const {
    bool skip = false;
    if (pProfiles != nullptr) {
        for (uint32_t _i189 = 0;_i189 < profileCount; ++_i189) {
            const auto _s190 = &pProfiles[_i189];
            skip |= ValidateVideoProfileInfoKHR(_parentObjects,
                _s190->sType,
                _s190->pNext,
                _s190->videoCodecOperation,
                _s190->chromaSubsampling,
                _s190->lumaBitDepth,
                _s190->chromaBitDepth);
        }
    }
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
bool ExplicitValidation::ValidatePhysicalDeviceVideoFormatInfoKHR(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const VkImageUsageFlags imageUsage) const {
    bool skip = false;
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
bool ExplicitValidation::ValidateVideoPictureResourceInfoKHR(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const VkOffset2D codedOffset,
                const VkExtent2D codedExtent,
                const uint32_t baseArrayLayer,
                const VkImageView imageViewBinding) const {
    bool skip = false;
    const auto _s191 = &codedOffset;
    skip |= ValidateOffset2D(_parentObjects,
        _s191->x,
        _s191->y);
    const auto _s192 = &codedExtent;
    skip |= ValidateExtent2D(_parentObjects,
        _s192->width,
        _s192->height);
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
bool ExplicitValidation::ValidateVideoReferenceSlotInfoKHR(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const int32_t slotIndex,
                const VkVideoPictureResourceInfoKHR* pPictureResource) const {
    bool skip = false;
    if (pPictureResource != nullptr) {
        const auto _s193 = pPictureResource;
        skip |= ValidateVideoPictureResourceInfoKHR(_parentObjects,
            _s193->sType,
            _s193->pNext,
            _s193->codedOffset,
            _s193->codedExtent,
            _s193->baseArrayLayer,
            _s193->imageViewBinding);
    }
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
bool ExplicitValidation::ValidateBindVideoSessionMemoryInfoKHR(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const uint32_t memoryBindIndex,
                const VkDeviceMemory memory,
                const VkDeviceSize memoryOffset,
                const VkDeviceSize memorySize) const {
    bool skip = false;
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
bool ExplicitValidation::ValidateVideoSessionCreateInfoKHR(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const uint32_t queueFamilyIndex,
                const VkVideoSessionCreateFlagsKHR flags,
                const VkVideoProfileInfoKHR* pVideoProfile,
                const VkFormat pictureFormat,
                const VkExtent2D maxCodedExtent,
                const VkFormat referencePictureFormat,
                const uint32_t maxDpbSlots,
                const uint32_t maxActiveReferencePictures,
                const VkExtensionProperties* pStdHeaderVersion) const {
    bool skip = false;
    if (pVideoProfile != nullptr) {
        const auto _s194 = pVideoProfile;
        skip |= ValidateVideoProfileInfoKHR(_parentObjects,
            _s194->sType,
            _s194->pNext,
            _s194->videoCodecOperation,
            _s194->chromaSubsampling,
            _s194->lumaBitDepth,
            _s194->chromaBitDepth);
    }
    const auto _s195 = &maxCodedExtent;
    skip |= ValidateExtent2D(_parentObjects,
        _s195->width,
        _s195->height);
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
bool ExplicitValidation::ValidateVideoSessionParametersCreateInfoKHR(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const VkVideoSessionParametersCreateFlagsKHR flags,
                const VkVideoSessionParametersKHR videoSessionParametersTemplate,
                const VkVideoSessionKHR videoSession) const {
    bool skip = false;
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
bool ExplicitValidation::ValidateVideoSessionParametersUpdateInfoKHR(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const uint32_t updateSequenceCount) const {
    bool skip = false;
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
bool ExplicitValidation::ValidateVideoBeginCodingInfoKHR(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const VkVideoBeginCodingFlagsKHR flags,
                const VkVideoSessionKHR videoSession,
                const VkVideoSessionParametersKHR videoSessionParameters,
                const uint32_t referenceSlotCount,
                const VkVideoReferenceSlotInfoKHR* pReferenceSlots) const {
    bool skip = false;
    if (pReferenceSlots != nullptr) {
        for (uint32_t _i196 = 0;_i196 < referenceSlotCount; ++_i196) {
            const auto _s197 = &pReferenceSlots[_i196];
            skip |= ValidateVideoReferenceSlotInfoKHR(_parentObjects,
                _s197->sType,
                _s197->pNext,
                _s197->slotIndex,
                _s197->pPictureResource);
        }
    }
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
bool ExplicitValidation::ValidateVideoEndCodingInfoKHR(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const VkVideoEndCodingFlagsKHR flags) const {
    bool skip = false;
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
bool ExplicitValidation::ValidateVideoCodingControlInfoKHR(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const VkVideoCodingControlFlagsKHR flags) const {
    bool skip = false;
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
bool ExplicitValidation::ValidateVideoDecodeUsageInfoKHR(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const VkVideoDecodeUsageFlagsKHR videoUsageHints) const {
    bool skip = false;
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
bool ExplicitValidation::ValidateVideoDecodeInfoKHR(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const VkVideoDecodeFlagsKHR flags,
                const VkBuffer srcBuffer,
                const VkDeviceSize srcBufferOffset,
                const VkDeviceSize srcBufferRange,
                const VkVideoPictureResourceInfoKHR dstPictureResource,
                const VkVideoReferenceSlotInfoKHR* pSetupReferenceSlot,
                const uint32_t referenceSlotCount,
                const VkVideoReferenceSlotInfoKHR* pReferenceSlots) const {
    bool skip = false;
    const auto _s198 = &dstPictureResource;
    skip |= ValidateVideoPictureResourceInfoKHR(_parentObjects,
        _s198->sType,
        _s198->pNext,
        _s198->codedOffset,
        _s198->codedExtent,
        _s198->baseArrayLayer,
        _s198->imageViewBinding);
    if (pSetupReferenceSlot != nullptr) {
        const auto _s199 = pSetupReferenceSlot;
        skip |= ValidateVideoReferenceSlotInfoKHR(_parentObjects,
            _s199->sType,
            _s199->pNext,
            _s199->slotIndex,
            _s199->pPictureResource);
    }
    if (pReferenceSlots != nullptr) {
        for (uint32_t _i200 = 0;_i200 < referenceSlotCount; ++_i200) {
            const auto _s201 = &pReferenceSlots[_i200];
            skip |= ValidateVideoReferenceSlotInfoKHR(_parentObjects,
                _s201->sType,
                _s201->pNext,
                _s201->slotIndex,
                _s201->pPictureResource);
        }
    }
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
bool ExplicitValidation::ValidateVideoDecodeH264ProfileInfoKHR(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const StdVideoH264ProfileIdc stdProfileIdc,
                const VkVideoDecodeH264PictureLayoutFlagBitsKHR pictureLayout) const {
    bool skip = false;
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
bool ExplicitValidation::ValidateVideoDecodeH264SessionParametersAddInfoKHR(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const uint32_t stdSPSCount,
                const StdVideoH264SequenceParameterSet* pStdSPSs,
                const uint32_t stdPPSCount,
                const StdVideoH264PictureParameterSet* pStdPPSs) const {
    bool skip = false;
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
bool ExplicitValidation::ValidateVideoDecodeH264SessionParametersCreateInfoKHR(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const uint32_t maxStdSPSCount,
                const uint32_t maxStdPPSCount,
                const VkVideoDecodeH264SessionParametersAddInfoKHR* pParametersAddInfo) const {
    bool skip = false;
    if (pParametersAddInfo != nullptr) {
        const auto _s202 = pParametersAddInfo;
        skip |= ValidateVideoDecodeH264SessionParametersAddInfoKHR(_parentObjects,
            _s202->sType,
            _s202->pNext,
            _s202->stdSPSCount,
            _s202->pStdSPSs,
            _s202->stdPPSCount,
            _s202->pStdPPSs);
    }
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
bool ExplicitValidation::ValidateVideoDecodeH264PictureInfoKHR(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const StdVideoDecodeH264PictureInfo* pStdPictureInfo,
                const uint32_t sliceCount,
                const uint32_t* pSliceOffsets) const {
    bool skip = false;
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
bool ExplicitValidation::ValidateVideoDecodeH264DpbSlotInfoKHR(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const StdVideoDecodeH264ReferenceInfo* pStdReferenceInfo) const {
    bool skip = false;
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
bool ExplicitValidation::ValidateRenderingInfoKHR(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const VkRenderingFlags flags,
                const VkRect2D renderArea,
                const uint32_t layerCount,
                const uint32_t viewMask,
                const uint32_t colorAttachmentCount,
                const VkRenderingAttachmentInfo* pColorAttachments,
                const VkRenderingAttachmentInfo* pDepthAttachment,
                const VkRenderingAttachmentInfo* pStencilAttachment) const {
    return ValidateRenderingInfo(_parentObjects, sType, pNext, flags, renderArea, layerCount, viewMask, colorAttachmentCount, pColorAttachments, pDepthAttachment, pStencilAttachment);
}
bool ExplicitValidation::ValidateRenderingAttachmentInfoKHR(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const VkImageView imageView,
                const VkImageLayout imageLayout,
                const VkResolveModeFlagBits resolveMode,
                const VkImageView resolveImageView,
                const VkImageLayout resolveImageLayout,
                const VkAttachmentLoadOp loadOp,
                const VkAttachmentStoreOp storeOp,
                const VkClearValue clearValue) const {
    return ValidateRenderingAttachmentInfo(_parentObjects, sType, pNext, imageView, imageLayout, resolveMode, resolveImageView, resolveImageLayout, loadOp, storeOp, clearValue);
}
bool ExplicitValidation::ValidatePipelineRenderingCreateInfoKHR(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const uint32_t viewMask,
                const uint32_t colorAttachmentCount,
                const VkFormat* pColorAttachmentFormats,
                const VkFormat depthAttachmentFormat,
                const VkFormat stencilAttachmentFormat) const {
    return ValidatePipelineRenderingCreateInfo(_parentObjects, sType, pNext, viewMask, colorAttachmentCount, pColorAttachmentFormats, depthAttachmentFormat, stencilAttachmentFormat);
}
bool ExplicitValidation::ValidatePhysicalDeviceDynamicRenderingFeaturesKHR(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const VkBool32 dynamicRendering) const {
    return ValidatePhysicalDeviceDynamicRenderingFeatures(_parentObjects, sType, pNext, dynamicRendering);
}
bool ExplicitValidation::ValidateCommandBufferInheritanceRenderingInfoKHR(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const VkRenderingFlags flags,
                const uint32_t viewMask,
                const uint32_t colorAttachmentCount,
                const VkFormat* pColorAttachmentFormats,
                const VkFormat depthAttachmentFormat,
                const VkFormat stencilAttachmentFormat,
                const VkSampleCountFlagBits rasterizationSamples) const {
    return ValidateCommandBufferInheritanceRenderingInfo(_parentObjects, sType, pNext, flags, viewMask, colorAttachmentCount, pColorAttachmentFormats, depthAttachmentFormat, stencilAttachmentFormat, rasterizationSamples);
}
bool ExplicitValidation::ValidateRenderingFragmentShadingRateAttachmentInfoKHR(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const VkImageView imageView,
                const VkImageLayout imageLayout,
                const VkExtent2D shadingRateAttachmentTexelSize) const {
    bool skip = false;
    const auto _s203 = &shadingRateAttachmentTexelSize;
    skip |= ValidateExtent2D(_parentObjects,
        _s203->width,
        _s203->height);
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
bool ExplicitValidation::ValidateRenderingFragmentDensityMapAttachmentInfoEXT(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const VkImageView imageView,
                const VkImageLayout imageLayout) const {
    bool skip = false;
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
bool ExplicitValidation::ValidateAttachmentSampleCountInfoAMD(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const uint32_t colorAttachmentCount,
                const VkSampleCountFlagBits* pColorAttachmentSamples,
                const VkSampleCountFlagBits depthStencilAttachmentSamples) const {
    bool skip = false;
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
bool ExplicitValidation::ValidateAttachmentSampleCountInfoNV(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const uint32_t colorAttachmentCount,
                const VkSampleCountFlagBits* pColorAttachmentSamples,
                const VkSampleCountFlagBits depthStencilAttachmentSamples) const {
    return ValidateAttachmentSampleCountInfoAMD(_parentObjects, sType, pNext, colorAttachmentCount, pColorAttachmentSamples, depthStencilAttachmentSamples);
}
bool ExplicitValidation::ValidateMultiviewPerViewAttributesInfoNVX(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const VkBool32 perViewAttributes,
                const VkBool32 perViewAttributesPositionXOnly) const {
    bool skip = false;
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
bool ExplicitValidation::ValidateRenderPassMultiviewCreateInfoKHR(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const uint32_t subpassCount,
                const uint32_t* pViewMasks,
                const uint32_t dependencyCount,
                const int32_t* pViewOffsets,
                const uint32_t correlationMaskCount,
                const uint32_t* pCorrelationMasks) const {
    return ValidateRenderPassMultiviewCreateInfo(_parentObjects, sType, pNext, subpassCount, pViewMasks, dependencyCount, pViewOffsets, correlationMaskCount, pCorrelationMasks);
}
bool ExplicitValidation::ValidatePhysicalDeviceMultiviewFeaturesKHR(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const VkBool32 multiview,
                const VkBool32 multiviewGeometryShader,
                const VkBool32 multiviewTessellationShader) const {
    return ValidatePhysicalDeviceMultiviewFeatures(_parentObjects, sType, pNext, multiview, multiviewGeometryShader, multiviewTessellationShader);
}
bool ExplicitValidation::ValidatePhysicalDeviceFeatures2KHR(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const VkPhysicalDeviceFeatures features) const {
    return ValidatePhysicalDeviceFeatures2(_parentObjects, sType, pNext, features);
}
bool ExplicitValidation::ValidatePhysicalDeviceImageFormatInfo2KHR(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const VkFormat format,
                const VkImageType type,
                const VkImageTiling tiling,
                const VkImageUsageFlags usage,
                const VkImageCreateFlags flags) const {
    return ValidatePhysicalDeviceImageFormatInfo2(_parentObjects, sType, pNext, format, type, tiling, usage, flags);
}
bool ExplicitValidation::ValidatePhysicalDeviceSparseImageFormatInfo2KHR(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const VkFormat format,
                const VkImageType type,
                const VkSampleCountFlagBits samples,
                const VkImageUsageFlags usage,
                const VkImageTiling tiling) const {
    return ValidatePhysicalDeviceSparseImageFormatInfo2(_parentObjects, sType, pNext, format, type, samples, usage, tiling);
}
bool ExplicitValidation::ValidateMemoryAllocateFlagsInfoKHR(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const VkMemoryAllocateFlags flags,
                const uint32_t deviceMask) const {
    return ValidateMemoryAllocateFlagsInfo(_parentObjects, sType, pNext, flags, deviceMask);
}
bool ExplicitValidation::ValidateDeviceGroupRenderPassBeginInfoKHR(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const uint32_t deviceMask,
                const uint32_t deviceRenderAreaCount,
                const VkRect2D* pDeviceRenderAreas) const {
    return ValidateDeviceGroupRenderPassBeginInfo(_parentObjects, sType, pNext, deviceMask, deviceRenderAreaCount, pDeviceRenderAreas);
}
bool ExplicitValidation::ValidateDeviceGroupCommandBufferBeginInfoKHR(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const uint32_t deviceMask) const {
    return ValidateDeviceGroupCommandBufferBeginInfo(_parentObjects, sType, pNext, deviceMask);
}
bool ExplicitValidation::ValidateDeviceGroupSubmitInfoKHR(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const uint32_t waitSemaphoreCount,
                const uint32_t* pWaitSemaphoreDeviceIndices,
                const uint32_t commandBufferCount,
                const uint32_t* pCommandBufferDeviceMasks,
                const uint32_t signalSemaphoreCount,
                const uint32_t* pSignalSemaphoreDeviceIndices) const {
    return ValidateDeviceGroupSubmitInfo(_parentObjects, sType, pNext, waitSemaphoreCount, pWaitSemaphoreDeviceIndices, commandBufferCount, pCommandBufferDeviceMasks, signalSemaphoreCount, pSignalSemaphoreDeviceIndices);
}
bool ExplicitValidation::ValidateDeviceGroupBindSparseInfoKHR(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const uint32_t resourceDeviceIndex,
                const uint32_t memoryDeviceIndex) const {
    return ValidateDeviceGroupBindSparseInfo(_parentObjects, sType, pNext, resourceDeviceIndex, memoryDeviceIndex);
}
bool ExplicitValidation::ValidateBindBufferMemoryDeviceGroupInfoKHR(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const uint32_t deviceIndexCount,
                const uint32_t* pDeviceIndices) const {
    return ValidateBindBufferMemoryDeviceGroupInfo(_parentObjects, sType, pNext, deviceIndexCount, pDeviceIndices);
}
bool ExplicitValidation::ValidateBindImageMemoryDeviceGroupInfoKHR(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const uint32_t deviceIndexCount,
                const uint32_t* pDeviceIndices,
                const uint32_t splitInstanceBindRegionCount,
                const VkRect2D* pSplitInstanceBindRegions) const {
    return ValidateBindImageMemoryDeviceGroupInfo(_parentObjects, sType, pNext, deviceIndexCount, pDeviceIndices, splitInstanceBindRegionCount, pSplitInstanceBindRegions);
}
bool ExplicitValidation::ValidateDeviceGroupDeviceCreateInfoKHR(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const uint32_t physicalDeviceCount,
                const VkPhysicalDevice* pPhysicalDevices) const {
    return ValidateDeviceGroupDeviceCreateInfo(_parentObjects, sType, pNext, physicalDeviceCount, pPhysicalDevices);
}
bool ExplicitValidation::ValidatePhysicalDeviceExternalImageFormatInfoKHR(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const VkExternalMemoryHandleTypeFlagBits handleType) const {
    return ValidatePhysicalDeviceExternalImageFormatInfo(_parentObjects, sType, pNext, handleType);
}
bool ExplicitValidation::ValidatePhysicalDeviceExternalBufferInfoKHR(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const VkBufferCreateFlags flags,
                const VkBufferUsageFlags usage,
                const VkExternalMemoryHandleTypeFlagBits handleType) const {
    return ValidatePhysicalDeviceExternalBufferInfo(_parentObjects, sType, pNext, flags, usage, handleType);
}
bool ExplicitValidation::ValidateExternalMemoryImageCreateInfoKHR(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const VkExternalMemoryHandleTypeFlags handleTypes) const {
    return ValidateExternalMemoryImageCreateInfo(_parentObjects, sType, pNext, handleTypes);
}
bool ExplicitValidation::ValidateExternalMemoryBufferCreateInfoKHR(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const VkExternalMemoryHandleTypeFlags handleTypes) const {
    return ValidateExternalMemoryBufferCreateInfo(_parentObjects, sType, pNext, handleTypes);
}
bool ExplicitValidation::ValidateExportMemoryAllocateInfoKHR(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const VkExternalMemoryHandleTypeFlags handleTypes) const {
    return ValidateExportMemoryAllocateInfo(_parentObjects, sType, pNext, handleTypes);
}
#ifdef VK_USE_PLATFORM_WIN32_KHR
bool ExplicitValidation::ValidateImportMemoryWin32HandleInfoKHR(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const VkExternalMemoryHandleTypeFlagBits handleType,
                const HANDLE handle,
                const LPCWSTR name) const {
    bool skip = false;
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
#endif
#ifdef VK_USE_PLATFORM_WIN32_KHR
bool ExplicitValidation::ValidateExportMemoryWin32HandleInfoKHR(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const SECURITY_ATTRIBUTES* pAttributes,
                const DWORD dwAccess,
                const LPCWSTR name) const {
    bool skip = false;
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
#endif
#ifdef VK_USE_PLATFORM_WIN32_KHR
bool ExplicitValidation::ValidateMemoryGetWin32HandleInfoKHR(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const VkDeviceMemory memory,
                const VkExternalMemoryHandleTypeFlagBits handleType) const {
    bool skip = false;
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
#endif
bool ExplicitValidation::ValidateImportMemoryFdInfoKHR(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const VkExternalMemoryHandleTypeFlagBits handleType,
                const int fd) const {
    bool skip = false;
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
bool ExplicitValidation::ValidateMemoryGetFdInfoKHR(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const VkDeviceMemory memory,
                const VkExternalMemoryHandleTypeFlagBits handleType) const {
    bool skip = false;
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
#ifdef VK_USE_PLATFORM_WIN32_KHR
bool ExplicitValidation::ValidateWin32KeyedMutexAcquireReleaseInfoKHR(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const uint32_t acquireCount,
                const VkDeviceMemory* pAcquireSyncs,
                const uint64_t* pAcquireKeys,
                const uint32_t* pAcquireTimeouts,
                const uint32_t releaseCount,
                const VkDeviceMemory* pReleaseSyncs,
                const uint64_t* pReleaseKeys) const {
    bool skip = false;
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
#endif
bool ExplicitValidation::ValidatePhysicalDeviceExternalSemaphoreInfoKHR(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const VkExternalSemaphoreHandleTypeFlagBits handleType) const {
    return ValidatePhysicalDeviceExternalSemaphoreInfo(_parentObjects, sType, pNext, handleType);
}
bool ExplicitValidation::ValidateExportSemaphoreCreateInfoKHR(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const VkExternalSemaphoreHandleTypeFlags handleTypes) const {
    return ValidateExportSemaphoreCreateInfo(_parentObjects, sType, pNext, handleTypes);
}
#ifdef VK_USE_PLATFORM_WIN32_KHR
bool ExplicitValidation::ValidateImportSemaphoreWin32HandleInfoKHR(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const VkSemaphore semaphore,
                const VkSemaphoreImportFlags flags,
                const VkExternalSemaphoreHandleTypeFlagBits handleType,
                const HANDLE handle,
                const LPCWSTR name) const {
    bool skip = false;
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
#endif
#ifdef VK_USE_PLATFORM_WIN32_KHR
bool ExplicitValidation::ValidateExportSemaphoreWin32HandleInfoKHR(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const SECURITY_ATTRIBUTES* pAttributes,
                const DWORD dwAccess,
                const LPCWSTR name) const {
    bool skip = false;
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
#endif
#ifdef VK_USE_PLATFORM_WIN32_KHR
bool ExplicitValidation::ValidateD3D12FenceSubmitInfoKHR(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const uint32_t waitSemaphoreValuesCount,
                const uint64_t* pWaitSemaphoreValues,
                const uint32_t signalSemaphoreValuesCount,
                const uint64_t* pSignalSemaphoreValues) const {
    bool skip = false;
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
#endif
#ifdef VK_USE_PLATFORM_WIN32_KHR
bool ExplicitValidation::ValidateSemaphoreGetWin32HandleInfoKHR(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const VkSemaphore semaphore,
                const VkExternalSemaphoreHandleTypeFlagBits handleType) const {
    bool skip = false;
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
#endif
bool ExplicitValidation::ValidateImportSemaphoreFdInfoKHR(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const VkSemaphore semaphore,
                const VkSemaphoreImportFlags flags,
                const VkExternalSemaphoreHandleTypeFlagBits handleType,
                const int fd) const {
    bool skip = false;
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
bool ExplicitValidation::ValidateSemaphoreGetFdInfoKHR(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const VkSemaphore semaphore,
                const VkExternalSemaphoreHandleTypeFlagBits handleType) const {
    bool skip = false;
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
bool ExplicitValidation::ValidatePhysicalDeviceShaderFloat16Int8FeaturesKHR(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const VkBool32 shaderFloat16,
                const VkBool32 shaderInt8) const {
    return ValidatePhysicalDeviceShaderFloat16Int8Features(_parentObjects, sType, pNext, shaderFloat16, shaderInt8);
}
bool ExplicitValidation::ValidatePhysicalDeviceFloat16Int8FeaturesKHR(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const VkBool32 shaderFloat16,
                const VkBool32 shaderInt8) const {
    return ValidatePhysicalDeviceShaderFloat16Int8Features(_parentObjects, sType, pNext, shaderFloat16, shaderInt8);
}
bool ExplicitValidation::ValidatePhysicalDevice16BitStorageFeaturesKHR(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const VkBool32 storageBuffer16BitAccess,
                const VkBool32 uniformAndStorageBuffer16BitAccess,
                const VkBool32 storagePushConstant16,
                const VkBool32 storageInputOutput16) const {
    return ValidatePhysicalDevice16BitStorageFeatures(_parentObjects, sType, pNext, storageBuffer16BitAccess, uniformAndStorageBuffer16BitAccess, storagePushConstant16, storageInputOutput16);
}
bool ExplicitValidation::ValidateRectLayerKHR(const LogObjectList &_parentObjects,
                const VkOffset2D offset,
                const VkExtent2D extent,
                const uint32_t layer) const {
    bool skip = false;
    const auto _s204 = &offset;
    skip |= ValidateOffset2D(_parentObjects,
        _s204->x,
        _s204->y);
    const auto _s205 = &extent;
    skip |= ValidateExtent2D(_parentObjects,
        _s205->width,
        _s205->height);
    return skip;
}
bool ExplicitValidation::ValidatePresentRegionKHR(const LogObjectList &_parentObjects,
                const uint32_t rectangleCount,
                const VkRectLayerKHR* pRectangles) const {
    bool skip = false;
    if (pRectangles != nullptr) {
        for (uint32_t _i206 = 0;_i206 < rectangleCount; ++_i206) {
            const auto _s207 = &pRectangles[_i206];
            skip |= ValidateRectLayerKHR(_parentObjects,
                _s207->offset,
                _s207->extent,
                _s207->layer);
        }
    }
    return skip;
}
bool ExplicitValidation::ValidatePresentRegionsKHR(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const uint32_t swapchainCount,
                const VkPresentRegionKHR* pRegions) const {
    bool skip = false;
    if (pRegions != nullptr) {
        for (uint32_t _i208 = 0;_i208 < swapchainCount; ++_i208) {
            const auto _s209 = &pRegions[_i208];
            skip |= ValidatePresentRegionKHR(_parentObjects,
                _s209->rectangleCount,
                _s209->pRectangles);
        }
    }
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
bool ExplicitValidation::ValidateDescriptorUpdateTemplateEntryKHR(const LogObjectList &_parentObjects,
                const uint32_t dstBinding,
                const uint32_t dstArrayElement,
                const uint32_t descriptorCount,
                const VkDescriptorType descriptorType,
                const size_t offset,
                const size_t stride) const {
    return ValidateDescriptorUpdateTemplateEntry(_parentObjects, dstBinding, dstArrayElement, descriptorCount, descriptorType, offset, stride);
}
bool ExplicitValidation::ValidateDescriptorUpdateTemplateCreateInfoKHR(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const VkDescriptorUpdateTemplateCreateFlags flags,
                const uint32_t descriptorUpdateEntryCount,
                const VkDescriptorUpdateTemplateEntry* pDescriptorUpdateEntries,
                const VkDescriptorUpdateTemplateType templateType,
                const VkDescriptorSetLayout descriptorSetLayout,
                const VkPipelineBindPoint pipelineBindPoint,
                const VkPipelineLayout pipelineLayout,
                const uint32_t set) const {
    return ValidateDescriptorUpdateTemplateCreateInfo(_parentObjects, sType, pNext, flags, descriptorUpdateEntryCount, pDescriptorUpdateEntries, templateType, descriptorSetLayout, pipelineBindPoint, pipelineLayout, set);
}
bool ExplicitValidation::ValidatePhysicalDeviceImagelessFramebufferFeaturesKHR(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const VkBool32 imagelessFramebuffer) const {
    return ValidatePhysicalDeviceImagelessFramebufferFeatures(_parentObjects, sType, pNext, imagelessFramebuffer);
}
bool ExplicitValidation::ValidateFramebufferAttachmentsCreateInfoKHR(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const uint32_t attachmentImageInfoCount,
                const VkFramebufferAttachmentImageInfo* pAttachmentImageInfos) const {
    return ValidateFramebufferAttachmentsCreateInfo(_parentObjects, sType, pNext, attachmentImageInfoCount, pAttachmentImageInfos);
}
bool ExplicitValidation::ValidateFramebufferAttachmentImageInfoKHR(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const VkImageCreateFlags flags,
                const VkImageUsageFlags usage,
                const uint32_t width,
                const uint32_t height,
                const uint32_t layerCount,
                const uint32_t viewFormatCount,
                const VkFormat* pViewFormats) const {
    return ValidateFramebufferAttachmentImageInfo(_parentObjects, sType, pNext, flags, usage, width, height, layerCount, viewFormatCount, pViewFormats);
}
bool ExplicitValidation::ValidateRenderPassAttachmentBeginInfoKHR(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const uint32_t attachmentCount,
                const VkImageView* pAttachments) const {
    return ValidateRenderPassAttachmentBeginInfo(_parentObjects, sType, pNext, attachmentCount, pAttachments);
}
bool ExplicitValidation::ValidateRenderPassCreateInfo2KHR(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const VkRenderPassCreateFlags flags,
                const uint32_t attachmentCount,
                const VkAttachmentDescription2* pAttachments,
                const uint32_t subpassCount,
                const VkSubpassDescription2* pSubpasses,
                const uint32_t dependencyCount,
                const VkSubpassDependency2* pDependencies,
                const uint32_t correlatedViewMaskCount,
                const uint32_t* pCorrelatedViewMasks) const {
    return ValidateRenderPassCreateInfo2(_parentObjects, sType, pNext, flags, attachmentCount, pAttachments, subpassCount, pSubpasses, dependencyCount, pDependencies, correlatedViewMaskCount, pCorrelatedViewMasks);
}
bool ExplicitValidation::ValidateAttachmentDescription2KHR(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const VkAttachmentDescriptionFlags flags,
                const VkFormat format,
                const VkSampleCountFlagBits samples,
                const VkAttachmentLoadOp loadOp,
                const VkAttachmentStoreOp storeOp,
                const VkAttachmentLoadOp stencilLoadOp,
                const VkAttachmentStoreOp stencilStoreOp,
                const VkImageLayout initialLayout,
                const VkImageLayout finalLayout) const {
    return ValidateAttachmentDescription2(_parentObjects, sType, pNext, flags, format, samples, loadOp, storeOp, stencilLoadOp, stencilStoreOp, initialLayout, finalLayout);
}
bool ExplicitValidation::ValidateAttachmentReference2KHR(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const uint32_t attachment,
                const VkImageLayout layout,
                const VkImageAspectFlags aspectMask) const {
    return ValidateAttachmentReference2(_parentObjects, sType, pNext, attachment, layout, aspectMask);
}
bool ExplicitValidation::ValidateSubpassDescription2KHR(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const VkSubpassDescriptionFlags flags,
                const VkPipelineBindPoint pipelineBindPoint,
                const uint32_t viewMask,
                const uint32_t inputAttachmentCount,
                const VkAttachmentReference2* pInputAttachments,
                const uint32_t colorAttachmentCount,
                const VkAttachmentReference2* pColorAttachments,
                const VkAttachmentReference2* pResolveAttachments,
                const VkAttachmentReference2* pDepthStencilAttachment,
                const uint32_t preserveAttachmentCount,
                const uint32_t* pPreserveAttachments) const {
    return ValidateSubpassDescription2(_parentObjects, sType, pNext, flags, pipelineBindPoint, viewMask, inputAttachmentCount, pInputAttachments, colorAttachmentCount, pColorAttachments, pResolveAttachments, pDepthStencilAttachment, preserveAttachmentCount, pPreserveAttachments);
}
bool ExplicitValidation::ValidateSubpassDependency2KHR(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const uint32_t srcSubpass,
                const uint32_t dstSubpass,
                const VkPipelineStageFlags srcStageMask,
                const VkPipelineStageFlags dstStageMask,
                const VkAccessFlags srcAccessMask,
                const VkAccessFlags dstAccessMask,
                const VkDependencyFlags dependencyFlags,
                const int32_t viewOffset) const {
    return ValidateSubpassDependency2(_parentObjects, sType, pNext, srcSubpass, dstSubpass, srcStageMask, dstStageMask, srcAccessMask, dstAccessMask, dependencyFlags, viewOffset);
}
bool ExplicitValidation::ValidateSubpassBeginInfoKHR(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const VkSubpassContents contents) const {
    return ValidateSubpassBeginInfo(_parentObjects, sType, pNext, contents);
}
bool ExplicitValidation::ValidateSubpassEndInfoKHR(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext) const {
    return ValidateSubpassEndInfo(_parentObjects, sType, pNext);
}
bool ExplicitValidation::ValidatePhysicalDeviceExternalFenceInfoKHR(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const VkExternalFenceHandleTypeFlagBits handleType) const {
    return ValidatePhysicalDeviceExternalFenceInfo(_parentObjects, sType, pNext, handleType);
}
bool ExplicitValidation::ValidateExportFenceCreateInfoKHR(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const VkExternalFenceHandleTypeFlags handleTypes) const {
    return ValidateExportFenceCreateInfo(_parentObjects, sType, pNext, handleTypes);
}
#ifdef VK_USE_PLATFORM_WIN32_KHR
bool ExplicitValidation::ValidateImportFenceWin32HandleInfoKHR(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const VkFence fence,
                const VkFenceImportFlags flags,
                const VkExternalFenceHandleTypeFlagBits handleType,
                const HANDLE handle,
                const LPCWSTR name) const {
    bool skip = false;
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
#endif
#ifdef VK_USE_PLATFORM_WIN32_KHR
bool ExplicitValidation::ValidateExportFenceWin32HandleInfoKHR(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const SECURITY_ATTRIBUTES* pAttributes,
                const DWORD dwAccess,
                const LPCWSTR name) const {
    bool skip = false;
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
#endif
#ifdef VK_USE_PLATFORM_WIN32_KHR
bool ExplicitValidation::ValidateFenceGetWin32HandleInfoKHR(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const VkFence fence,
                const VkExternalFenceHandleTypeFlagBits handleType) const {
    bool skip = false;
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
#endif
bool ExplicitValidation::ValidateImportFenceFdInfoKHR(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const VkFence fence,
                const VkFenceImportFlags flags,
                const VkExternalFenceHandleTypeFlagBits handleType,
                const int fd) const {
    bool skip = false;
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
bool ExplicitValidation::ValidateFenceGetFdInfoKHR(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const VkFence fence,
                const VkExternalFenceHandleTypeFlagBits handleType) const {
    bool skip = false;
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
bool ExplicitValidation::ValidatePhysicalDevicePerformanceQueryFeaturesKHR(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const VkBool32 performanceCounterQueryPools,
                const VkBool32 performanceCounterMultipleQueryPools) const {
    bool skip = false;
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
bool ExplicitValidation::ValidateQueryPoolPerformanceCreateInfoKHR(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const uint32_t queueFamilyIndex,
                const uint32_t counterIndexCount,
                const uint32_t* pCounterIndices) const {
    bool skip = false;
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
bool ExplicitValidation::ValidatePerformanceCounterResultKHR(const LogObjectList &_parentObjects,
                const int32_t int32,
                const int64_t int64,
                const uint32_t uint32,
                const uint64_t uint64,
                const float float32,
                const double float64) const {
    bool skip = false;
    return skip;
}
bool ExplicitValidation::ValidateAcquireProfilingLockInfoKHR(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const VkAcquireProfilingLockFlagsKHR flags,
                const uint64_t timeout) const {
    bool skip = false;
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
bool ExplicitValidation::ValidatePerformanceQuerySubmitInfoKHR(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const uint32_t counterPassIndex) const {
    bool skip = false;
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
bool ExplicitValidation::ValidateRenderPassInputAttachmentAspectCreateInfoKHR(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const uint32_t aspectReferenceCount,
                const VkInputAttachmentAspectReference* pAspectReferences) const {
    return ValidateRenderPassInputAttachmentAspectCreateInfo(_parentObjects, sType, pNext, aspectReferenceCount, pAspectReferences);
}
bool ExplicitValidation::ValidateInputAttachmentAspectReferenceKHR(const LogObjectList &_parentObjects,
                const uint32_t subpass,
                const uint32_t inputAttachmentIndex,
                const VkImageAspectFlags aspectMask) const {
    return ValidateInputAttachmentAspectReference(_parentObjects, subpass, inputAttachmentIndex, aspectMask);
}
bool ExplicitValidation::ValidateImageViewUsageCreateInfoKHR(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const VkImageUsageFlags usage) const {
    return ValidateImageViewUsageCreateInfo(_parentObjects, sType, pNext, usage);
}
bool ExplicitValidation::ValidatePipelineTessellationDomainOriginStateCreateInfoKHR(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const VkTessellationDomainOrigin domainOrigin) const {
    return ValidatePipelineTessellationDomainOriginStateCreateInfo(_parentObjects, sType, pNext, domainOrigin);
}
bool ExplicitValidation::ValidatePhysicalDeviceSurfaceInfo2KHR(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const VkSurfaceKHR surface) const {
    bool skip = false;
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
bool ExplicitValidation::ValidatePhysicalDeviceVariablePointerFeaturesKHR(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const VkBool32 variablePointersStorageBuffer,
                const VkBool32 variablePointers) const {
    return ValidatePhysicalDeviceVariablePointersFeatures(_parentObjects, sType, pNext, variablePointersStorageBuffer, variablePointers);
}
bool ExplicitValidation::ValidatePhysicalDeviceVariablePointersFeaturesKHR(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const VkBool32 variablePointersStorageBuffer,
                const VkBool32 variablePointers) const {
    return ValidatePhysicalDeviceVariablePointersFeatures(_parentObjects, sType, pNext, variablePointersStorageBuffer, variablePointers);
}
bool ExplicitValidation::ValidateDisplayPlaneInfo2KHR(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const VkDisplayModeKHR mode,
                const uint32_t planeIndex) const {
    bool skip = false;
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
bool ExplicitValidation::ValidateMemoryDedicatedAllocateInfoKHR(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const VkImage image,
                const VkBuffer buffer) const {
    return ValidateMemoryDedicatedAllocateInfo(_parentObjects, sType, pNext, image, buffer);
}
bool ExplicitValidation::ValidateBufferMemoryRequirementsInfo2KHR(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const VkBuffer buffer) const {
    return ValidateBufferMemoryRequirementsInfo2(_parentObjects, sType, pNext, buffer);
}
bool ExplicitValidation::ValidateImageMemoryRequirementsInfo2KHR(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const VkImage image) const {
    return ValidateImageMemoryRequirementsInfo2(_parentObjects, sType, pNext, image);
}
bool ExplicitValidation::ValidateImageSparseMemoryRequirementsInfo2KHR(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const VkImage image) const {
    return ValidateImageSparseMemoryRequirementsInfo2(_parentObjects, sType, pNext, image);
}
bool ExplicitValidation::ValidateImageFormatListCreateInfoKHR(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const uint32_t viewFormatCount,
                const VkFormat* pViewFormats) const {
    return ValidateImageFormatListCreateInfo(_parentObjects, sType, pNext, viewFormatCount, pViewFormats);
}
bool ExplicitValidation::ValidateSamplerYcbcrConversionCreateInfoKHR(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const VkFormat format,
                const VkSamplerYcbcrModelConversion ycbcrModel,
                const VkSamplerYcbcrRange ycbcrRange,
                const VkComponentMapping components,
                const VkChromaLocation xChromaOffset,
                const VkChromaLocation yChromaOffset,
                const VkFilter chromaFilter,
                VkBool32 forceExplicitReconstruction) const {
    return ValidateSamplerYcbcrConversionCreateInfo(_parentObjects, sType, pNext, format, ycbcrModel, ycbcrRange, components, xChromaOffset, yChromaOffset, chromaFilter, forceExplicitReconstruction);
}
bool ExplicitValidation::ValidateSamplerYcbcrConversionInfoKHR(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const VkSamplerYcbcrConversion conversion) const {
    return ValidateSamplerYcbcrConversionInfo(_parentObjects, sType, pNext, conversion);
}
bool ExplicitValidation::ValidateBindImagePlaneMemoryInfoKHR(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const VkImageAspectFlagBits planeAspect) const {
    return ValidateBindImagePlaneMemoryInfo(_parentObjects, sType, pNext, planeAspect);
}
bool ExplicitValidation::ValidateImagePlaneMemoryRequirementsInfoKHR(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const VkImageAspectFlagBits planeAspect) const {
    return ValidateImagePlaneMemoryRequirementsInfo(_parentObjects, sType, pNext, planeAspect);
}
bool ExplicitValidation::ValidatePhysicalDeviceSamplerYcbcrConversionFeaturesKHR(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const VkBool32 samplerYcbcrConversion) const {
    return ValidatePhysicalDeviceSamplerYcbcrConversionFeatures(_parentObjects, sType, pNext, samplerYcbcrConversion);
}
bool ExplicitValidation::ValidateBindBufferMemoryInfoKHR(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const VkBuffer buffer,
                const VkDeviceMemory memory,
                const VkDeviceSize memoryOffset) const {
    return ValidateBindBufferMemoryInfo(_parentObjects, sType, pNext, buffer, memory, memoryOffset);
}
bool ExplicitValidation::ValidateBindImageMemoryInfoKHR(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const VkImage image,
                const VkDeviceMemory memory,
                const VkDeviceSize memoryOffset) const {
    return ValidateBindImageMemoryInfo(_parentObjects, sType, pNext, image, memory, memoryOffset);
}
#ifdef VK_ENABLE_BETA_EXTENSIONS
bool ExplicitValidation::ValidatePhysicalDevicePortabilitySubsetFeaturesKHR(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                VkBool32 constantAlphaColorBlendFactors,
                const VkBool32 events,
                const VkBool32 imageViewFormatReinterpretation,
                const VkBool32 imageViewFormatSwizzle,
                const VkBool32 imageView2DOn3DImage,
                const VkBool32 multisampleArrayImage,
                const VkBool32 mutableComparisonSamplers,
                const VkBool32 pointPolygons,
                const VkBool32 samplerMipLodBias,
                const VkBool32 separateStencilMaskRef,
                const VkBool32 shaderSampleRateInterpolationFunctions,
                const VkBool32 tessellationIsolines,
                const VkBool32 tessellationPointMode,
                const VkBool32 triangleFans,
                const VkBool32 vertexAttributeAccessBeyondStride) const {
    bool skip = false;
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
#endif
#ifdef VK_ENABLE_BETA_EXTENSIONS
bool ExplicitValidation::ValidatePhysicalDevicePortabilitySubsetPropertiesKHR(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const uint32_t minVertexInputBindingStrideAlignment) const {
    bool skip = false;
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
#endif
bool ExplicitValidation::ValidatePhysicalDeviceShaderSubgroupExtendedTypesFeaturesKHR(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const VkBool32 shaderSubgroupExtendedTypes) const {
    return ValidatePhysicalDeviceShaderSubgroupExtendedTypesFeatures(_parentObjects, sType, pNext, shaderSubgroupExtendedTypes);
}
bool ExplicitValidation::ValidatePhysicalDevice8BitStorageFeaturesKHR(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const VkBool32 storageBuffer8BitAccess,
                const VkBool32 uniformAndStorageBuffer8BitAccess,
                const VkBool32 storagePushConstant8) const {
    return ValidatePhysicalDevice8BitStorageFeatures(_parentObjects, sType, pNext, storageBuffer8BitAccess, uniformAndStorageBuffer8BitAccess, storagePushConstant8);
}
bool ExplicitValidation::ValidatePhysicalDeviceShaderAtomicInt64FeaturesKHR(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const VkBool32 shaderBufferInt64Atomics,
                const VkBool32 shaderSharedInt64Atomics) const {
    return ValidatePhysicalDeviceShaderAtomicInt64Features(_parentObjects, sType, pNext, shaderBufferInt64Atomics, shaderSharedInt64Atomics);
}
bool ExplicitValidation::ValidatePhysicalDeviceShaderClockFeaturesKHR(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const VkBool32 shaderSubgroupClock,
                const VkBool32 shaderDeviceClock) const {
    bool skip = false;
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
bool ExplicitValidation::ValidateVideoDecodeH265ProfileInfoKHR(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const StdVideoH265ProfileIdc stdProfileIdc) const {
    bool skip = false;
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
bool ExplicitValidation::ValidateVideoDecodeH265SessionParametersAddInfoKHR(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const uint32_t stdVPSCount,
                const StdVideoH265VideoParameterSet* pStdVPSs,
                const uint32_t stdSPSCount,
                const StdVideoH265SequenceParameterSet* pStdSPSs,
                const uint32_t stdPPSCount,
                const StdVideoH265PictureParameterSet* pStdPPSs) const {
    bool skip = false;
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
bool ExplicitValidation::ValidateVideoDecodeH265SessionParametersCreateInfoKHR(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const uint32_t maxStdVPSCount,
                const uint32_t maxStdSPSCount,
                const uint32_t maxStdPPSCount,
                const VkVideoDecodeH265SessionParametersAddInfoKHR* pParametersAddInfo) const {
    bool skip = false;
    if (pParametersAddInfo != nullptr) {
        const auto _s210 = pParametersAddInfo;
        skip |= ValidateVideoDecodeH265SessionParametersAddInfoKHR(_parentObjects,
            _s210->sType,
            _s210->pNext,
            _s210->stdVPSCount,
            _s210->pStdVPSs,
            _s210->stdSPSCount,
            _s210->pStdSPSs,
            _s210->stdPPSCount,
            _s210->pStdPPSs);
    }
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
bool ExplicitValidation::ValidateVideoDecodeH265PictureInfoKHR(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const StdVideoDecodeH265PictureInfo* pStdPictureInfo,
                const uint32_t sliceSegmentCount,
                const uint32_t* pSliceSegmentOffsets) const {
    bool skip = false;
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
bool ExplicitValidation::ValidateVideoDecodeH265DpbSlotInfoKHR(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const StdVideoDecodeH265ReferenceInfo* pStdReferenceInfo) const {
    bool skip = false;
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
bool ExplicitValidation::ValidateDeviceQueueGlobalPriorityCreateInfoKHR(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const VkQueueGlobalPriorityKHR globalPriority) const {
    bool skip = false;
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
bool ExplicitValidation::ValidatePhysicalDeviceGlobalPriorityQueryFeaturesKHR(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const VkBool32 globalPriorityQuery) const {
    bool skip = false;
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
bool ExplicitValidation::ValidateQueueFamilyGlobalPriorityPropertiesKHR(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const uint32_t priorityCount,
                const VkQueueGlobalPriorityKHR priorities[VK_MAX_GLOBAL_PRIORITY_SIZE_KHR]) const {
    bool skip = false;
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
bool ExplicitValidation::ValidateConformanceVersionKHR(const LogObjectList &_parentObjects,
                const uint8_t major,
                const uint8_t minor,
                const uint8_t subminor,
                const uint8_t patch) const {
    return ValidateConformanceVersion(_parentObjects, major, minor, subminor, patch);
}
bool ExplicitValidation::ValidateSubpassDescriptionDepthStencilResolveKHR(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const VkResolveModeFlagBits depthResolveMode,
                const VkResolveModeFlagBits stencilResolveMode,
                const VkAttachmentReference2* pDepthStencilResolveAttachment) const {
    return ValidateSubpassDescriptionDepthStencilResolve(_parentObjects, sType, pNext, depthResolveMode, stencilResolveMode, pDepthStencilResolveAttachment);
}
bool ExplicitValidation::ValidatePhysicalDeviceTimelineSemaphoreFeaturesKHR(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const VkBool32 timelineSemaphore) const {
    return ValidatePhysicalDeviceTimelineSemaphoreFeatures(_parentObjects, sType, pNext, timelineSemaphore);
}
bool ExplicitValidation::ValidateSemaphoreTypeCreateInfoKHR(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const VkSemaphoreType semaphoreType,
                const uint64_t initialValue) const {
    return ValidateSemaphoreTypeCreateInfo(_parentObjects, sType, pNext, semaphoreType, initialValue);
}
bool ExplicitValidation::ValidateTimelineSemaphoreSubmitInfoKHR(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const uint32_t waitSemaphoreValueCount,
                const uint64_t* pWaitSemaphoreValues,
                const uint32_t signalSemaphoreValueCount,
                const uint64_t* pSignalSemaphoreValues) const {
    return ValidateTimelineSemaphoreSubmitInfo(_parentObjects, sType, pNext, waitSemaphoreValueCount, pWaitSemaphoreValues, signalSemaphoreValueCount, pSignalSemaphoreValues);
}
bool ExplicitValidation::ValidateSemaphoreWaitInfoKHR(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const VkSemaphoreWaitFlags flags,
                const uint32_t semaphoreCount,
                const VkSemaphore* pSemaphores,
                const uint64_t* pValues) const {
    return ValidateSemaphoreWaitInfo(_parentObjects, sType, pNext, flags, semaphoreCount, pSemaphores, pValues);
}
bool ExplicitValidation::ValidateSemaphoreSignalInfoKHR(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const VkSemaphore semaphore,
                const uint64_t value) const {
    return ValidateSemaphoreSignalInfo(_parentObjects, sType, pNext, semaphore, value);
}
bool ExplicitValidation::ValidatePhysicalDeviceVulkanMemoryModelFeaturesKHR(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const VkBool32 vulkanMemoryModel,
                const VkBool32 vulkanMemoryModelDeviceScope,
                const VkBool32 vulkanMemoryModelAvailabilityVisibilityChains) const {
    return ValidatePhysicalDeviceVulkanMemoryModelFeatures(_parentObjects, sType, pNext, vulkanMemoryModel, vulkanMemoryModelDeviceScope, vulkanMemoryModelAvailabilityVisibilityChains);
}
bool ExplicitValidation::ValidatePhysicalDeviceShaderTerminateInvocationFeaturesKHR(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const VkBool32 shaderTerminateInvocation) const {
    return ValidatePhysicalDeviceShaderTerminateInvocationFeatures(_parentObjects, sType, pNext, shaderTerminateInvocation);
}
bool ExplicitValidation::ValidateFragmentShadingRateAttachmentInfoKHR(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const VkAttachmentReference2* pFragmentShadingRateAttachment,
                const VkExtent2D shadingRateAttachmentTexelSize) const {
    bool skip = false;
    if (pFragmentShadingRateAttachment != nullptr) {
        const auto _s211 = pFragmentShadingRateAttachment;
        skip |= ValidateAttachmentReference2(_parentObjects,
            _s211->sType,
            _s211->pNext,
            _s211->attachment,
            _s211->layout,
            _s211->aspectMask);
    }
    const auto _s212 = &shadingRateAttachmentTexelSize;
    skip |= ValidateExtent2D(_parentObjects,
        _s212->width,
        _s212->height);
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
bool ExplicitValidation::ValidatePipelineFragmentShadingRateStateCreateInfoKHR(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const VkExtent2D fragmentSize,
                const VkFragmentShadingRateCombinerOpKHR combinerOps[2]) const {
    bool skip = false;
    const auto _s213 = &fragmentSize;
    skip |= ValidateExtent2D(_parentObjects,
        _s213->width,
        _s213->height);
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
bool ExplicitValidation::ValidatePhysicalDeviceFragmentShadingRateFeaturesKHR(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const VkBool32 pipelineFragmentShadingRate,
                const VkBool32 primitiveFragmentShadingRate,
                const VkBool32 attachmentFragmentShadingRate) const {
    bool skip = false;
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
bool ExplicitValidation::ValidateSurfaceProtectedCapabilitiesKHR(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const VkBool32 supportsProtected) const {
    bool skip = false;
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
bool ExplicitValidation::ValidatePhysicalDeviceSeparateDepthStencilLayoutsFeaturesKHR(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const VkBool32 separateDepthStencilLayouts) const {
    return ValidatePhysicalDeviceSeparateDepthStencilLayoutsFeatures(_parentObjects, sType, pNext, separateDepthStencilLayouts);
}
bool ExplicitValidation::ValidateAttachmentReferenceStencilLayoutKHR(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const VkImageLayout stencilLayout) const {
    return ValidateAttachmentReferenceStencilLayout(_parentObjects, sType, pNext, stencilLayout);
}
bool ExplicitValidation::ValidateAttachmentDescriptionStencilLayoutKHR(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const VkImageLayout stencilInitialLayout,
                const VkImageLayout stencilFinalLayout) const {
    return ValidateAttachmentDescriptionStencilLayout(_parentObjects, sType, pNext, stencilInitialLayout, stencilFinalLayout);
}
bool ExplicitValidation::ValidatePhysicalDevicePresentWaitFeaturesKHR(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const VkBool32 presentWait) const {
    bool skip = false;
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
bool ExplicitValidation::ValidatePhysicalDeviceUniformBufferStandardLayoutFeaturesKHR(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const VkBool32 uniformBufferStandardLayout) const {
    return ValidatePhysicalDeviceUniformBufferStandardLayoutFeatures(_parentObjects, sType, pNext, uniformBufferStandardLayout);
}
bool ExplicitValidation::ValidatePhysicalDeviceBufferDeviceAddressFeaturesKHR(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const VkBool32 bufferDeviceAddress,
                const VkBool32 bufferDeviceAddressCaptureReplay,
                const VkBool32 bufferDeviceAddressMultiDevice) const {
    return ValidatePhysicalDeviceBufferDeviceAddressFeatures(_parentObjects, sType, pNext, bufferDeviceAddress, bufferDeviceAddressCaptureReplay, bufferDeviceAddressMultiDevice);
}
bool ExplicitValidation::ValidateBufferDeviceAddressInfoKHR(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const VkBuffer buffer) const {
    return ValidateBufferDeviceAddressInfo(_parentObjects, sType, pNext, buffer);
}
bool ExplicitValidation::ValidateBufferOpaqueCaptureAddressCreateInfoKHR(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const uint64_t opaqueCaptureAddress) const {
    return ValidateBufferOpaqueCaptureAddressCreateInfo(_parentObjects, sType, pNext, opaqueCaptureAddress);
}
bool ExplicitValidation::ValidateMemoryOpaqueCaptureAddressAllocateInfoKHR(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const uint64_t opaqueCaptureAddress) const {
    return ValidateMemoryOpaqueCaptureAddressAllocateInfo(_parentObjects, sType, pNext, opaqueCaptureAddress);
}
bool ExplicitValidation::ValidateDeviceMemoryOpaqueCaptureAddressInfoKHR(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const VkDeviceMemory memory) const {
    return ValidateDeviceMemoryOpaqueCaptureAddressInfo(_parentObjects, sType, pNext, memory);
}
bool ExplicitValidation::ValidatePhysicalDevicePipelineExecutablePropertiesFeaturesKHR(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const VkBool32 pipelineExecutableInfo) const {
    bool skip = false;
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
bool ExplicitValidation::ValidatePipelineInfoKHR(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const VkPipeline pipeline) const {
    bool skip = false;
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
bool ExplicitValidation::ValidatePipelineExecutableInfoKHR(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const VkPipeline pipeline,
                const uint32_t executableIndex) const {
    bool skip = false;
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
bool ExplicitValidation::ValidateMemoryMapInfoKHR(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const VkMemoryMapFlags flags,
                const VkDeviceMemory memory,
                const VkDeviceSize offset,
                const VkDeviceSize size) const {
    bool skip = false;
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
bool ExplicitValidation::ValidateMemoryUnmapInfoKHR(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const VkMemoryUnmapFlagsKHR flags,
                const VkDeviceMemory memory) const {
    bool skip = false;
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
bool ExplicitValidation::ValidatePhysicalDeviceShaderIntegerDotProductFeaturesKHR(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const VkBool32 shaderIntegerDotProduct) const {
    return ValidatePhysicalDeviceShaderIntegerDotProductFeatures(_parentObjects, sType, pNext, shaderIntegerDotProduct);
}
bool ExplicitValidation::ValidatePipelineLibraryCreateInfoKHR(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const uint32_t libraryCount,
                const VkPipeline* pLibraries) const {
    bool skip = false;
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
bool ExplicitValidation::ValidatePresentIdKHR(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const uint32_t swapchainCount,
                const uint64_t* pPresentIds) const {
    bool skip = false;
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
bool ExplicitValidation::ValidatePhysicalDevicePresentIdFeaturesKHR(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const VkBool32 presentId) const {
    bool skip = false;
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
#ifdef VK_ENABLE_BETA_EXTENSIONS
bool ExplicitValidation::ValidateVideoEncodeInfoKHR(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const VkVideoEncodeFlagsKHR flags,
                const uint32_t qualityLevel,
                const VkBuffer dstBuffer,
                const VkDeviceSize dstBufferOffset,
                const VkDeviceSize dstBufferRange,
                const VkVideoPictureResourceInfoKHR srcPictureResource,
                const VkVideoReferenceSlotInfoKHR* pSetupReferenceSlot,
                const uint32_t referenceSlotCount,
                const VkVideoReferenceSlotInfoKHR* pReferenceSlots,
                const uint32_t precedingExternallyEncodedBytes) const {
    bool skip = false;
    const auto _s214 = &srcPictureResource;
    skip |= ValidateVideoPictureResourceInfoKHR(_parentObjects,
        _s214->sType,
        _s214->pNext,
        _s214->codedOffset,
        _s214->codedExtent,
        _s214->baseArrayLayer,
        _s214->imageViewBinding);
    if (pSetupReferenceSlot != nullptr) {
        const auto _s215 = pSetupReferenceSlot;
        skip |= ValidateVideoReferenceSlotInfoKHR(_parentObjects,
            _s215->sType,
            _s215->pNext,
            _s215->slotIndex,
            _s215->pPictureResource);
    }
    if (pReferenceSlots != nullptr) {
        for (uint32_t _i216 = 0;_i216 < referenceSlotCount; ++_i216) {
            const auto _s217 = &pReferenceSlots[_i216];
            skip |= ValidateVideoReferenceSlotInfoKHR(_parentObjects,
                _s217->sType,
                _s217->pNext,
                _s217->slotIndex,
                _s217->pPictureResource);
        }
    }
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
#endif
#ifdef VK_ENABLE_BETA_EXTENSIONS
bool ExplicitValidation::ValidateQueryPoolVideoEncodeFeedbackCreateInfoKHR(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const VkVideoEncodeFeedbackFlagsKHR encodeFeedbackFlags) const {
    bool skip = false;
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
#endif
#ifdef VK_ENABLE_BETA_EXTENSIONS
bool ExplicitValidation::ValidateVideoEncodeUsageInfoKHR(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const VkVideoEncodeUsageFlagsKHR videoUsageHints,
                const VkVideoEncodeContentFlagsKHR videoContentHints,
                const VkVideoEncodeTuningModeKHR tuningMode) const {
    bool skip = false;
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
#endif
#ifdef VK_ENABLE_BETA_EXTENSIONS
bool ExplicitValidation::ValidateVideoEncodeRateControlLayerInfoKHR(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const uint64_t averageBitrate,
                const uint64_t maxBitrate,
                const uint32_t frameRateNumerator,
                const uint32_t frameRateDenominator,
                const uint32_t virtualBufferSizeInMs,
                const uint32_t initialVirtualBufferSizeInMs) const {
    bool skip = false;
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
#endif
#ifdef VK_ENABLE_BETA_EXTENSIONS
bool ExplicitValidation::ValidateVideoEncodeRateControlInfoKHR(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const VkVideoEncodeRateControlFlagsKHR flags,
                const VkVideoEncodeRateControlModeFlagBitsKHR rateControlMode,
                const uint32_t layerCount,
                const VkVideoEncodeRateControlLayerInfoKHR* pLayers) const {
    bool skip = false;
    if (pLayers != nullptr) {
        for (uint32_t _i218 = 0;_i218 < layerCount; ++_i218) {
            const auto _s219 = &pLayers[_i218];
            skip |= ValidateVideoEncodeRateControlLayerInfoKHR(_parentObjects,
                _s219->sType,
                _s219->pNext,
                _s219->averageBitrate,
                _s219->maxBitrate,
                _s219->frameRateNumerator,
                _s219->frameRateDenominator,
                _s219->virtualBufferSizeInMs,
                _s219->initialVirtualBufferSizeInMs);
        }
    }
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
#endif
bool ExplicitValidation::ValidateMemoryBarrier2KHR(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const VkPipelineStageFlags2 srcStageMask,
                const VkAccessFlags2 srcAccessMask,
                const VkPipelineStageFlags2 dstStageMask,
                const VkAccessFlags2 dstAccessMask) const {
    return ValidateMemoryBarrier2(_parentObjects, sType, pNext, srcStageMask, srcAccessMask, dstStageMask, dstAccessMask);
}
bool ExplicitValidation::ValidateBufferMemoryBarrier2KHR(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const VkPipelineStageFlags2 srcStageMask,
                const VkAccessFlags2 srcAccessMask,
                const VkPipelineStageFlags2 dstStageMask,
                const VkAccessFlags2 dstAccessMask,
                const uint32_t srcQueueFamilyIndex,
                const uint32_t dstQueueFamilyIndex,
                const VkBuffer buffer,
                const VkDeviceSize offset,
                const VkDeviceSize size) const {
    return ValidateBufferMemoryBarrier2(_parentObjects, sType, pNext, srcStageMask, srcAccessMask, dstStageMask, dstAccessMask, srcQueueFamilyIndex, dstQueueFamilyIndex, buffer, offset, size);
}
bool ExplicitValidation::ValidateImageMemoryBarrier2KHR(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const VkPipelineStageFlags2 srcStageMask,
                const VkAccessFlags2 srcAccessMask,
                const VkPipelineStageFlags2 dstStageMask,
                const VkAccessFlags2 dstAccessMask,
                const VkImageLayout oldLayout,
                const VkImageLayout newLayout,
                const uint32_t srcQueueFamilyIndex,
                const uint32_t dstQueueFamilyIndex,
                const VkImage image,
                const VkImageSubresourceRange subresourceRange) const {
    return ValidateImageMemoryBarrier2(_parentObjects, sType, pNext, srcStageMask, srcAccessMask, dstStageMask, dstAccessMask, oldLayout, newLayout, srcQueueFamilyIndex, dstQueueFamilyIndex, image, subresourceRange);
}
bool ExplicitValidation::ValidateDependencyInfoKHR(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const VkDependencyFlags dependencyFlags,
                const uint32_t memoryBarrierCount,
                const VkMemoryBarrier2* pMemoryBarriers,
                const uint32_t bufferMemoryBarrierCount,
                const VkBufferMemoryBarrier2* pBufferMemoryBarriers,
                const uint32_t imageMemoryBarrierCount,
                const VkImageMemoryBarrier2* pImageMemoryBarriers) const {
    return ValidateDependencyInfo(_parentObjects, sType, pNext, dependencyFlags, memoryBarrierCount, pMemoryBarriers, bufferMemoryBarrierCount, pBufferMemoryBarriers, imageMemoryBarrierCount, pImageMemoryBarriers);
}
bool ExplicitValidation::ValidateSubmitInfo2KHR(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const VkSubmitFlags flags,
                const uint32_t waitSemaphoreInfoCount,
                const VkSemaphoreSubmitInfo* pWaitSemaphoreInfos,
                const uint32_t commandBufferInfoCount,
                const VkCommandBufferSubmitInfo* pCommandBufferInfos,
                const uint32_t signalSemaphoreInfoCount,
                const VkSemaphoreSubmitInfo* pSignalSemaphoreInfos) const {
    return ValidateSubmitInfo2(_parentObjects, sType, pNext, flags, waitSemaphoreInfoCount, pWaitSemaphoreInfos, commandBufferInfoCount, pCommandBufferInfos, signalSemaphoreInfoCount, pSignalSemaphoreInfos);
}
bool ExplicitValidation::ValidateSemaphoreSubmitInfoKHR(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const VkSemaphore semaphore,
                const uint64_t value,
                const VkPipelineStageFlags2 stageMask,
                const uint32_t deviceIndex) const {
    return ValidateSemaphoreSubmitInfo(_parentObjects, sType, pNext, semaphore, value, stageMask, deviceIndex);
}
bool ExplicitValidation::ValidateCommandBufferSubmitInfoKHR(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const VkCommandBuffer commandBuffer,
                const uint32_t deviceMask) const {
    return ValidateCommandBufferSubmitInfo(_parentObjects, sType, pNext, commandBuffer, deviceMask);
}
bool ExplicitValidation::ValidatePhysicalDeviceSynchronization2FeaturesKHR(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const VkBool32 synchronization2) const {
    return ValidatePhysicalDeviceSynchronization2Features(_parentObjects, sType, pNext, synchronization2);
}
bool ExplicitValidation::ValidatePhysicalDeviceFragmentShaderBarycentricFeaturesKHR(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const VkBool32 fragmentShaderBarycentric) const {
    bool skip = false;
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
bool ExplicitValidation::ValidatePhysicalDeviceShaderSubgroupUniformControlFlowFeaturesKHR(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const VkBool32 shaderSubgroupUniformControlFlow) const {
    bool skip = false;
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
bool ExplicitValidation::ValidatePhysicalDeviceZeroInitializeWorkgroupMemoryFeaturesKHR(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const VkBool32 shaderZeroInitializeWorkgroupMemory) const {
    return ValidatePhysicalDeviceZeroInitializeWorkgroupMemoryFeatures(_parentObjects, sType, pNext, shaderZeroInitializeWorkgroupMemory);
}
bool ExplicitValidation::ValidatePhysicalDeviceWorkgroupMemoryExplicitLayoutFeaturesKHR(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const VkBool32 workgroupMemoryExplicitLayout,
                const VkBool32 workgroupMemoryExplicitLayoutScalarBlockLayout,
                const VkBool32 workgroupMemoryExplicitLayout8BitAccess,
                const VkBool32 workgroupMemoryExplicitLayout16BitAccess) const {
    bool skip = false;
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
bool ExplicitValidation::ValidateCopyBufferInfo2KHR(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const VkBuffer srcBuffer,
                const VkBuffer dstBuffer,
                const uint32_t regionCount,
                const VkBufferCopy2* pRegions) const {
    return ValidateCopyBufferInfo2(_parentObjects, sType, pNext, srcBuffer, dstBuffer, regionCount, pRegions);
}
bool ExplicitValidation::ValidateCopyImageInfo2KHR(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const VkImage srcImage,
                const VkImageLayout srcImageLayout,
                const VkImage dstImage,
                const VkImageLayout dstImageLayout,
                const uint32_t regionCount,
                const VkImageCopy2* pRegions) const {
    return ValidateCopyImageInfo2(_parentObjects, sType, pNext, srcImage, srcImageLayout, dstImage, dstImageLayout, regionCount, pRegions);
}
bool ExplicitValidation::ValidateCopyBufferToImageInfo2KHR(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const VkBuffer srcBuffer,
                const VkImage dstImage,
                const VkImageLayout dstImageLayout,
                const uint32_t regionCount,
                const VkBufferImageCopy2* pRegions) const {
    return ValidateCopyBufferToImageInfo2(_parentObjects, sType, pNext, srcBuffer, dstImage, dstImageLayout, regionCount, pRegions);
}
bool ExplicitValidation::ValidateCopyImageToBufferInfo2KHR(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const VkImage srcImage,
                const VkImageLayout srcImageLayout,
                const VkBuffer dstBuffer,
                const uint32_t regionCount,
                const VkBufferImageCopy2* pRegions) const {
    return ValidateCopyImageToBufferInfo2(_parentObjects, sType, pNext, srcImage, srcImageLayout, dstBuffer, regionCount, pRegions);
}
bool ExplicitValidation::ValidateBlitImageInfo2KHR(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const VkImage srcImage,
                const VkImageLayout srcImageLayout,
                const VkImage dstImage,
                const VkImageLayout dstImageLayout,
                const uint32_t regionCount,
                const VkImageBlit2* pRegions,
                const VkFilter filter) const {
    return ValidateBlitImageInfo2(_parentObjects, sType, pNext, srcImage, srcImageLayout, dstImage, dstImageLayout, regionCount, pRegions, filter);
}
bool ExplicitValidation::ValidateResolveImageInfo2KHR(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const VkImage srcImage,
                const VkImageLayout srcImageLayout,
                const VkImage dstImage,
                const VkImageLayout dstImageLayout,
                const uint32_t regionCount,
                const VkImageResolve2* pRegions) const {
    return ValidateResolveImageInfo2(_parentObjects, sType, pNext, srcImage, srcImageLayout, dstImage, dstImageLayout, regionCount, pRegions);
}
bool ExplicitValidation::ValidateBufferCopy2KHR(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const VkDeviceSize srcOffset,
                const VkDeviceSize dstOffset,
                const VkDeviceSize size) const {
    return ValidateBufferCopy2(_parentObjects, sType, pNext, srcOffset, dstOffset, size);
}
bool ExplicitValidation::ValidateImageCopy2KHR(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const VkImageSubresourceLayers srcSubresource,
                const VkOffset3D srcOffset,
                const VkImageSubresourceLayers dstSubresource,
                const VkOffset3D dstOffset,
                const VkExtent3D extent) const {
    return ValidateImageCopy2(_parentObjects, sType, pNext, srcSubresource, srcOffset, dstSubresource, dstOffset, extent);
}
bool ExplicitValidation::ValidateImageBlit2KHR(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const VkImageSubresourceLayers srcSubresource,
                const VkOffset3D srcOffsets[2],
                const VkImageSubresourceLayers dstSubresource,
                const VkOffset3D dstOffsets[2]) const {
    return ValidateImageBlit2(_parentObjects, sType, pNext, srcSubresource, srcOffsets, dstSubresource, dstOffsets);
}
bool ExplicitValidation::ValidateBufferImageCopy2KHR(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const VkDeviceSize bufferOffset,
                const uint32_t bufferRowLength,
                const uint32_t bufferImageHeight,
                const VkImageSubresourceLayers imageSubresource,
                const VkOffset3D imageOffset,
                const VkExtent3D imageExtent) const {
    return ValidateBufferImageCopy2(_parentObjects, sType, pNext, bufferOffset, bufferRowLength, bufferImageHeight, imageSubresource, imageOffset, imageExtent);
}
bool ExplicitValidation::ValidateImageResolve2KHR(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const VkImageSubresourceLayers srcSubresource,
                const VkOffset3D srcOffset,
                const VkImageSubresourceLayers dstSubresource,
                const VkOffset3D dstOffset,
                const VkExtent3D extent) const {
    return ValidateImageResolve2(_parentObjects, sType, pNext, srcSubresource, srcOffset, dstSubresource, dstOffset, extent);
}
bool ExplicitValidation::ValidatePhysicalDeviceRayTracingMaintenance1FeaturesKHR(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const VkBool32 rayTracingMaintenance1,
                const VkBool32 rayTracingPipelineTraceRaysIndirect2) const {
    bool skip = false;
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
bool ExplicitValidation::ValidateTraceRaysIndirectCommand2KHR(const LogObjectList &_parentObjects,
                const VkDeviceAddress raygenShaderRecordAddress,
                const VkDeviceSize raygenShaderRecordSize,
                const VkDeviceAddress missShaderBindingTableAddress,
                const VkDeviceSize missShaderBindingTableSize,
                const VkDeviceSize missShaderBindingTableStride,
                const VkDeviceAddress hitShaderBindingTableAddress,
                const VkDeviceSize hitShaderBindingTableSize,
                const VkDeviceSize hitShaderBindingTableStride,
                const VkDeviceAddress callableShaderBindingTableAddress,
                const VkDeviceSize callableShaderBindingTableSize,
                const VkDeviceSize callableShaderBindingTableStride,
                const uint32_t width,
                const uint32_t height,
                const uint32_t depth) const {
    bool skip = false;
    return skip;
}
bool ExplicitValidation::ValidatePhysicalDeviceMaintenance4FeaturesKHR(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const VkBool32 maintenance4) const {
    return ValidatePhysicalDeviceMaintenance4Features(_parentObjects, sType, pNext, maintenance4);
}
bool ExplicitValidation::ValidateDeviceBufferMemoryRequirementsKHR(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const VkBufferCreateInfo* pCreateInfo) const {
    return ValidateDeviceBufferMemoryRequirements(_parentObjects, sType, pNext, pCreateInfo);
}
bool ExplicitValidation::ValidateDeviceImageMemoryRequirementsKHR(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const VkImageCreateInfo* pCreateInfo,
                const VkImageAspectFlagBits planeAspect) const {
    return ValidateDeviceImageMemoryRequirements(_parentObjects, sType, pNext, pCreateInfo, planeAspect);
}
bool ExplicitValidation::ValidateDebugReportCallbackCreateInfoEXT(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const VkDebugReportFlagsEXT flags,
                const PFN_vkDebugReportCallbackEXT pfnCallback,
                const void* pUserData) const {
    bool skip = false;
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
bool ExplicitValidation::ValidatePipelineRasterizationStateRasterizationOrderAMD(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const VkRasterizationOrderAMD rasterizationOrder) const {
    bool skip = false;
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
bool ExplicitValidation::ValidateDebugMarkerObjectNameInfoEXT(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const VkDebugReportObjectTypeEXT objectType,
                const uint64_t object,
                const char* pObjectName) const {
    bool skip = false;
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
bool ExplicitValidation::ValidateDebugMarkerObjectTagInfoEXT(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const VkDebugReportObjectTypeEXT objectType,
                const uint64_t object,
                const uint64_t tagName,
                const size_t tagSize,
                const void* pTag) const {
    bool skip = false;
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
bool ExplicitValidation::ValidateDebugMarkerMarkerInfoEXT(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const char* pMarkerName,
                const float color[4]) const {
    bool skip = false;
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
bool ExplicitValidation::ValidateDedicatedAllocationImageCreateInfoNV(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const VkBool32 dedicatedAllocation) const {
    bool skip = false;
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
bool ExplicitValidation::ValidateDedicatedAllocationBufferCreateInfoNV(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const VkBool32 dedicatedAllocation) const {
    bool skip = false;
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
bool ExplicitValidation::ValidateDedicatedAllocationMemoryAllocateInfoNV(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const VkImage image,
                const VkBuffer buffer) const {
    bool skip = false;
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
bool ExplicitValidation::ValidatePhysicalDeviceTransformFeedbackFeaturesEXT(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const VkBool32 transformFeedback,
                const VkBool32 geometryStreams) const {
    bool skip = false;
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
bool ExplicitValidation::ValidatePipelineRasterizationStateStreamCreateInfoEXT(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const VkPipelineRasterizationStateStreamCreateFlagsEXT flags,
                const uint32_t rasterizationStream) const {
    bool skip = false;
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
bool ExplicitValidation::ValidateCuModuleCreateInfoNVX(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const size_t dataSize,
                const void* pData) const {
    bool skip = false;
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
bool ExplicitValidation::ValidateCuFunctionCreateInfoNVX(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const VkCuModuleNVX module,
                const char* pName) const {
    bool skip = false;
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
bool ExplicitValidation::ValidateCuLaunchInfoNVX(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const VkCuFunctionNVX function,
                const uint32_t gridDimX,
                const uint32_t gridDimY,
                const uint32_t gridDimZ,
                const uint32_t blockDimX,
                const uint32_t blockDimY,
                const uint32_t blockDimZ,
                const uint32_t sharedMemBytes,
                const size_t paramCount,
                const void* const * pParams,
                const size_t extraCount,
                const void* const * pExtras) const {
    bool skip = false;
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
bool ExplicitValidation::ValidateImageViewHandleInfoNVX(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const VkImageView imageView,
                const VkDescriptorType descriptorType,
                const VkSampler sampler) const {
    bool skip = false;
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
#ifdef VK_ENABLE_BETA_EXTENSIONS
bool ExplicitValidation::ValidateVideoEncodeH264SessionParametersAddInfoEXT(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const uint32_t stdSPSCount,
                const StdVideoH264SequenceParameterSet* pStdSPSs,
                const uint32_t stdPPSCount,
                const StdVideoH264PictureParameterSet* pStdPPSs) const {
    bool skip = false;
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
#endif
#ifdef VK_ENABLE_BETA_EXTENSIONS
bool ExplicitValidation::ValidateVideoEncodeH264SessionParametersCreateInfoEXT(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const uint32_t maxStdSPSCount,
                const uint32_t maxStdPPSCount,
                const VkVideoEncodeH264SessionParametersAddInfoEXT* pParametersAddInfo) const {
    bool skip = false;
    if (pParametersAddInfo != nullptr) {
        const auto _s220 = pParametersAddInfo;
        skip |= ValidateVideoEncodeH264SessionParametersAddInfoEXT(_parentObjects,
            _s220->sType,
            _s220->pNext,
            _s220->stdSPSCount,
            _s220->pStdSPSs,
            _s220->stdPPSCount,
            _s220->pStdPPSs);
    }
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
#endif
#ifdef VK_ENABLE_BETA_EXTENSIONS
bool ExplicitValidation::ValidateVideoEncodeH264NaluSliceInfoEXT(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const uint32_t mbCount,
                const StdVideoEncodeH264ReferenceListsInfo* pStdReferenceFinalLists,
                const StdVideoEncodeH264SliceHeader* pStdSliceHeader) const {
    bool skip = false;
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
#endif
#ifdef VK_ENABLE_BETA_EXTENSIONS
bool ExplicitValidation::ValidateVideoEncodeH264VclFrameInfoEXT(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const StdVideoEncodeH264ReferenceListsInfo* pStdReferenceFinalLists,
                const uint32_t naluSliceEntryCount,
                const VkVideoEncodeH264NaluSliceInfoEXT* pNaluSliceEntries,
                const StdVideoEncodeH264PictureInfo* pStdPictureInfo) const {
    bool skip = false;
    if (pNaluSliceEntries != nullptr) {
        for (uint32_t _i221 = 0;_i221 < naluSliceEntryCount; ++_i221) {
            const auto _s222 = &pNaluSliceEntries[_i221];
            skip |= ValidateVideoEncodeH264NaluSliceInfoEXT(_parentObjects,
                _s222->sType,
                _s222->pNext,
                _s222->mbCount,
                _s222->pStdReferenceFinalLists,
                _s222->pStdSliceHeader);
        }
    }
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
#endif
#ifdef VK_ENABLE_BETA_EXTENSIONS
bool ExplicitValidation::ValidateVideoEncodeH264DpbSlotInfoEXT(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const StdVideoEncodeH264ReferenceInfo* pStdReferenceInfo) const {
    bool skip = false;
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
#endif
#ifdef VK_ENABLE_BETA_EXTENSIONS
bool ExplicitValidation::ValidateVideoEncodeH264ProfileInfoEXT(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const StdVideoH264ProfileIdc stdProfileIdc) const {
    bool skip = false;
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
#endif
#ifdef VK_ENABLE_BETA_EXTENSIONS
bool ExplicitValidation::ValidateVideoEncodeH264RateControlInfoEXT(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const uint32_t gopFrameCount,
                const uint32_t idrPeriod,
                const uint32_t consecutiveBFrameCount,
                const VkVideoEncodeH264RateControlStructureEXT rateControlStructure,
                const uint32_t temporalLayerCount) const {
    bool skip = false;
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
#endif
#ifdef VK_ENABLE_BETA_EXTENSIONS
bool ExplicitValidation::ValidateVideoEncodeH264QpEXT(const LogObjectList &_parentObjects,
                const int32_t qpI,
                const int32_t qpP,
                const int32_t qpB) const {
    bool skip = false;
    return skip;
}
#endif
#ifdef VK_ENABLE_BETA_EXTENSIONS
bool ExplicitValidation::ValidateVideoEncodeH264FrameSizeEXT(const LogObjectList &_parentObjects,
                const uint32_t frameISize,
                const uint32_t framePSize,
                const uint32_t frameBSize) const {
    bool skip = false;
    return skip;
}
#endif
#ifdef VK_ENABLE_BETA_EXTENSIONS
bool ExplicitValidation::ValidateVideoEncodeH264RateControlLayerInfoEXT(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const uint32_t temporalLayerId,
                const VkBool32 useInitialRcQp,
                const VkVideoEncodeH264QpEXT initialRcQp,
                const VkBool32 useMinQp,
                const VkVideoEncodeH264QpEXT minQp,
                const VkBool32 useMaxQp,
                const VkVideoEncodeH264QpEXT maxQp,
                const VkBool32 useMaxFrameSize,
                const VkVideoEncodeH264FrameSizeEXT maxFrameSize) const {
    bool skip = false;
    const auto _s223 = &initialRcQp;
    skip |= ValidateVideoEncodeH264QpEXT(_parentObjects,
        _s223->qpI,
        _s223->qpP,
        _s223->qpB);
    const auto _s224 = &minQp;
    skip |= ValidateVideoEncodeH264QpEXT(_parentObjects,
        _s224->qpI,
        _s224->qpP,
        _s224->qpB);
    const auto _s225 = &maxQp;
    skip |= ValidateVideoEncodeH264QpEXT(_parentObjects,
        _s225->qpI,
        _s225->qpP,
        _s225->qpB);
    const auto _s226 = &maxFrameSize;
    skip |= ValidateVideoEncodeH264FrameSizeEXT(_parentObjects,
        _s226->frameISize,
        _s226->framePSize,
        _s226->frameBSize);
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
#endif
#ifdef VK_ENABLE_BETA_EXTENSIONS
bool ExplicitValidation::ValidateVideoEncodeH265SessionParametersAddInfoEXT(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const uint32_t stdVPSCount,
                const StdVideoH265VideoParameterSet* pStdVPSs,
                const uint32_t stdSPSCount,
                const StdVideoH265SequenceParameterSet* pStdSPSs,
                const uint32_t stdPPSCount,
                const StdVideoH265PictureParameterSet* pStdPPSs) const {
    bool skip = false;
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
#endif
#ifdef VK_ENABLE_BETA_EXTENSIONS
bool ExplicitValidation::ValidateVideoEncodeH265SessionParametersCreateInfoEXT(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const uint32_t maxStdVPSCount,
                const uint32_t maxStdSPSCount,
                const uint32_t maxStdPPSCount,
                const VkVideoEncodeH265SessionParametersAddInfoEXT* pParametersAddInfo) const {
    bool skip = false;
    if (pParametersAddInfo != nullptr) {
        const auto _s227 = pParametersAddInfo;
        skip |= ValidateVideoEncodeH265SessionParametersAddInfoEXT(_parentObjects,
            _s227->sType,
            _s227->pNext,
            _s227->stdVPSCount,
            _s227->pStdVPSs,
            _s227->stdSPSCount,
            _s227->pStdSPSs,
            _s227->stdPPSCount,
            _s227->pStdPPSs);
    }
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
#endif
#ifdef VK_ENABLE_BETA_EXTENSIONS
bool ExplicitValidation::ValidateVideoEncodeH265NaluSliceSegmentInfoEXT(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const uint32_t ctbCount,
                const StdVideoEncodeH265ReferenceListsInfo* pStdReferenceFinalLists,
                const StdVideoEncodeH265SliceSegmentHeader* pStdSliceSegmentHeader) const {
    bool skip = false;
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
#endif
#ifdef VK_ENABLE_BETA_EXTENSIONS
bool ExplicitValidation::ValidateVideoEncodeH265VclFrameInfoEXT(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const StdVideoEncodeH265ReferenceListsInfo* pStdReferenceFinalLists,
                const uint32_t naluSliceSegmentEntryCount,
                const VkVideoEncodeH265NaluSliceSegmentInfoEXT* pNaluSliceSegmentEntries,
                const StdVideoEncodeH265PictureInfo* pStdPictureInfo) const {
    bool skip = false;
    if (pNaluSliceSegmentEntries != nullptr) {
        for (uint32_t _i228 = 0;_i228 < naluSliceSegmentEntryCount; ++_i228) {
            const auto _s229 = &pNaluSliceSegmentEntries[_i228];
            skip |= ValidateVideoEncodeH265NaluSliceSegmentInfoEXT(_parentObjects,
                _s229->sType,
                _s229->pNext,
                _s229->ctbCount,
                _s229->pStdReferenceFinalLists,
                _s229->pStdSliceSegmentHeader);
        }
    }
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
#endif
#ifdef VK_ENABLE_BETA_EXTENSIONS
bool ExplicitValidation::ValidateVideoEncodeH265DpbSlotInfoEXT(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const StdVideoEncodeH265ReferenceInfo* pStdReferenceInfo) const {
    bool skip = false;
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
#endif
#ifdef VK_ENABLE_BETA_EXTENSIONS
bool ExplicitValidation::ValidateVideoEncodeH265ProfileInfoEXT(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const StdVideoH265ProfileIdc stdProfileIdc) const {
    bool skip = false;
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
#endif
#ifdef VK_ENABLE_BETA_EXTENSIONS
bool ExplicitValidation::ValidateVideoEncodeH265RateControlInfoEXT(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const uint32_t gopFrameCount,
                const uint32_t idrPeriod,
                const uint32_t consecutiveBFrameCount,
                const VkVideoEncodeH265RateControlStructureEXT rateControlStructure,
                const uint32_t subLayerCount) const {
    bool skip = false;
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
#endif
#ifdef VK_ENABLE_BETA_EXTENSIONS
bool ExplicitValidation::ValidateVideoEncodeH265QpEXT(const LogObjectList &_parentObjects,
                const int32_t qpI,
                const int32_t qpP,
                const int32_t qpB) const {
    bool skip = false;
    return skip;
}
#endif
#ifdef VK_ENABLE_BETA_EXTENSIONS
bool ExplicitValidation::ValidateVideoEncodeH265FrameSizeEXT(const LogObjectList &_parentObjects,
                const uint32_t frameISize,
                const uint32_t framePSize,
                const uint32_t frameBSize) const {
    bool skip = false;
    return skip;
}
#endif
#ifdef VK_ENABLE_BETA_EXTENSIONS
bool ExplicitValidation::ValidateVideoEncodeH265RateControlLayerInfoEXT(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const uint32_t temporalId,
                const VkBool32 useInitialRcQp,
                const VkVideoEncodeH265QpEXT initialRcQp,
                const VkBool32 useMinQp,
                const VkVideoEncodeH265QpEXT minQp,
                const VkBool32 useMaxQp,
                const VkVideoEncodeH265QpEXT maxQp,
                const VkBool32 useMaxFrameSize,
                const VkVideoEncodeH265FrameSizeEXT maxFrameSize) const {
    bool skip = false;
    const auto _s230 = &initialRcQp;
    skip |= ValidateVideoEncodeH265QpEXT(_parentObjects,
        _s230->qpI,
        _s230->qpP,
        _s230->qpB);
    const auto _s231 = &minQp;
    skip |= ValidateVideoEncodeH265QpEXT(_parentObjects,
        _s231->qpI,
        _s231->qpP,
        _s231->qpB);
    const auto _s232 = &maxQp;
    skip |= ValidateVideoEncodeH265QpEXT(_parentObjects,
        _s232->qpI,
        _s232->qpP,
        _s232->qpB);
    const auto _s233 = &maxFrameSize;
    skip |= ValidateVideoEncodeH265FrameSizeEXT(_parentObjects,
        _s233->frameISize,
        _s233->framePSize,
        _s233->frameBSize);
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
#endif
#ifdef VK_USE_PLATFORM_GGP
bool ExplicitValidation::ValidateStreamDescriptorSurfaceCreateInfoGGP(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const VkStreamDescriptorSurfaceCreateFlagsGGP flags,
                const GgpStreamDescriptor streamDescriptor) const {
    bool skip = false;
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
#endif
bool ExplicitValidation::ValidatePhysicalDeviceCornerSampledImageFeaturesNV(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const VkBool32 cornerSampledImage) const {
    bool skip = false;
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
bool ExplicitValidation::ValidateExternalMemoryImageCreateInfoNV(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const VkExternalMemoryHandleTypeFlagsNV handleTypes) const {
    bool skip = false;
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
bool ExplicitValidation::ValidateExportMemoryAllocateInfoNV(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const VkExternalMemoryHandleTypeFlagsNV handleTypes) const {
    bool skip = false;
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
#ifdef VK_USE_PLATFORM_WIN32_KHR
bool ExplicitValidation::ValidateImportMemoryWin32HandleInfoNV(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const VkExternalMemoryHandleTypeFlagsNV handleType,
                const HANDLE handle) const {
    bool skip = false;
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
#endif
#ifdef VK_USE_PLATFORM_WIN32_KHR
bool ExplicitValidation::ValidateExportMemoryWin32HandleInfoNV(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const SECURITY_ATTRIBUTES* pAttributes,
                const DWORD dwAccess) const {
    bool skip = false;
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
#endif
#ifdef VK_USE_PLATFORM_WIN32_KHR
bool ExplicitValidation::ValidateWin32KeyedMutexAcquireReleaseInfoNV(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const uint32_t acquireCount,
                const VkDeviceMemory* pAcquireSyncs,
                const uint64_t* pAcquireKeys,
                const uint32_t* pAcquireTimeoutMilliseconds,
                const uint32_t releaseCount,
                const VkDeviceMemory* pReleaseSyncs,
                const uint64_t* pReleaseKeys) const {
    bool skip = false;
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
#endif
bool ExplicitValidation::ValidateValidationFlagsEXT(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const uint32_t disabledValidationCheckCount,
                const VkValidationCheckEXT* pDisabledValidationChecks) const {
    bool skip = false;
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
#ifdef VK_USE_PLATFORM_VI_NN
bool ExplicitValidation::ValidateViSurfaceCreateInfoNN(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const VkViSurfaceCreateFlagsNN flags,
                const void* window) const {
    bool skip = false;
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
#endif
bool ExplicitValidation::ValidatePhysicalDeviceTextureCompressionASTCHDRFeaturesEXT(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const VkBool32 textureCompressionASTC_HDR) const {
    return ValidatePhysicalDeviceTextureCompressionASTCHDRFeatures(_parentObjects, sType, pNext, textureCompressionASTC_HDR);
}
bool ExplicitValidation::ValidateImageViewASTCDecodeModeEXT(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const VkFormat decodeMode) const {
    bool skip = false;
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
bool ExplicitValidation::ValidatePhysicalDeviceASTCDecodeFeaturesEXT(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const VkBool32 decodeModeSharedExponent) const {
    bool skip = false;
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
bool ExplicitValidation::ValidatePhysicalDevicePipelineRobustnessFeaturesEXT(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const VkBool32 pipelineRobustness) const {
    bool skip = false;
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
bool ExplicitValidation::ValidatePipelineRobustnessCreateInfoEXT(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const VkPipelineRobustnessBufferBehaviorEXT storageBuffers,
                const VkPipelineRobustnessBufferBehaviorEXT uniformBuffers,
                const VkPipelineRobustnessBufferBehaviorEXT vertexInputs,
                const VkPipelineRobustnessImageBehaviorEXT images) const {
    bool skip = false;
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
bool ExplicitValidation::ValidateConditionalRenderingBeginInfoEXT(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const VkBuffer buffer,
                const VkDeviceSize offset,
                const VkConditionalRenderingFlagsEXT flags) const {
    bool skip = false;
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
bool ExplicitValidation::ValidatePhysicalDeviceConditionalRenderingFeaturesEXT(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const VkBool32 conditionalRendering,
                const VkBool32 inheritedConditionalRendering) const {
    bool skip = false;
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
bool ExplicitValidation::ValidateCommandBufferInheritanceConditionalRenderingInfoEXT(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const VkBool32 conditionalRenderingEnable) const {
    bool skip = false;
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
bool ExplicitValidation::ValidateViewportWScalingNV(const LogObjectList &_parentObjects,
                const float xcoeff,
                const float ycoeff) const {
    bool skip = false;
    return skip;
}
bool ExplicitValidation::ValidatePipelineViewportWScalingStateCreateInfoNV(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const VkBool32 viewportWScalingEnable,
                const uint32_t viewportCount,
                const VkViewportWScalingNV* pViewportWScalings) const {
    bool skip = false;
    if (pViewportWScalings != nullptr) {
        for (uint32_t _i234 = 0;_i234 < viewportCount; ++_i234) {
            const auto _s235 = &pViewportWScalings[_i234];
            skip |= ValidateViewportWScalingNV(_parentObjects,
                _s235->xcoeff,
                _s235->ycoeff);
        }
    }
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
bool ExplicitValidation::ValidateDisplayPowerInfoEXT(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const VkDisplayPowerStateEXT powerState) const {
    bool skip = false;
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
bool ExplicitValidation::ValidateDeviceEventInfoEXT(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const VkDeviceEventTypeEXT deviceEvent) const {
    bool skip = false;
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
bool ExplicitValidation::ValidateDisplayEventInfoEXT(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const VkDisplayEventTypeEXT displayEvent) const {
    bool skip = false;
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
bool ExplicitValidation::ValidateSwapchainCounterCreateInfoEXT(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const VkSurfaceCounterFlagsEXT surfaceCounters) const {
    bool skip = false;
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
bool ExplicitValidation::ValidatePresentTimeGOOGLE(const LogObjectList &_parentObjects,
                const uint32_t presentID,
                const uint64_t desiredPresentTime) const {
    bool skip = false;
    return skip;
}
bool ExplicitValidation::ValidatePresentTimesInfoGOOGLE(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const uint32_t swapchainCount,
                const VkPresentTimeGOOGLE* pTimes) const {
    bool skip = false;
    if (pTimes != nullptr) {
        for (uint32_t _i236 = 0;_i236 < swapchainCount; ++_i236) {
            const auto _s237 = &pTimes[_i236];
            skip |= ValidatePresentTimeGOOGLE(_parentObjects,
                _s237->presentID,
                _s237->desiredPresentTime);
        }
    }
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
bool ExplicitValidation::ValidateViewportSwizzleNV(const LogObjectList &_parentObjects,
                const VkViewportCoordinateSwizzleNV x,
                const VkViewportCoordinateSwizzleNV y,
                const VkViewportCoordinateSwizzleNV z,
                const VkViewportCoordinateSwizzleNV w) const {
    bool skip = false;
    return skip;
}
bool ExplicitValidation::ValidatePipelineViewportSwizzleStateCreateInfoNV(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const VkPipelineViewportSwizzleStateCreateFlagsNV flags,
                const uint32_t viewportCount,
                const VkViewportSwizzleNV* pViewportSwizzles) const {
    bool skip = false;
    if (pViewportSwizzles != nullptr) {
        for (uint32_t _i238 = 0;_i238 < viewportCount; ++_i238) {
            const auto _s239 = &pViewportSwizzles[_i238];
            skip |= ValidateViewportSwizzleNV(_parentObjects,
                _s239->x,
                _s239->y,
                _s239->z,
                _s239->w);
        }
    }
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
bool ExplicitValidation::ValidatePipelineDiscardRectangleStateCreateInfoEXT(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const VkPipelineDiscardRectangleStateCreateFlagsEXT flags,
                const VkDiscardRectangleModeEXT discardRectangleMode,
                const uint32_t discardRectangleCount,
                const VkRect2D* pDiscardRectangles) const {
    bool skip = false;
    if (pDiscardRectangles != nullptr) {
        for (uint32_t _i240 = 0;_i240 < discardRectangleCount; ++_i240) {
            const auto _s241 = &pDiscardRectangles[_i240];
            skip |= ValidateRect2D(_parentObjects,
                _s241->offset,
                _s241->extent);
        }
    }
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
bool ExplicitValidation::ValidatePipelineRasterizationConservativeStateCreateInfoEXT(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const VkPipelineRasterizationConservativeStateCreateFlagsEXT flags,
                const VkConservativeRasterizationModeEXT conservativeRasterizationMode,
                const float extraPrimitiveOverestimationSize) const {
    bool skip = false;
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
bool ExplicitValidation::ValidatePhysicalDeviceDepthClipEnableFeaturesEXT(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const VkBool32 depthClipEnable) const {
    bool skip = false;
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
bool ExplicitValidation::ValidatePipelineRasterizationDepthClipStateCreateInfoEXT(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const VkPipelineRasterizationDepthClipStateCreateFlagsEXT flags,
                const VkBool32 depthClipEnable) const {
    bool skip = false;
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
bool ExplicitValidation::ValidateXYColorEXT(const LogObjectList &_parentObjects,
                const float x,
                const float y) const {
    bool skip = false;
    return skip;
}
bool ExplicitValidation::ValidateHdrMetadataEXT(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const VkXYColorEXT displayPrimaryRed,
                const VkXYColorEXT displayPrimaryGreen,
                const VkXYColorEXT displayPrimaryBlue,
                const VkXYColorEXT whitePoint,
                const float maxLuminance,
                const float minLuminance,
                const float maxContentLightLevel,
                const float maxFrameAverageLightLevel) const {
    bool skip = false;
    const auto _s242 = &displayPrimaryRed;
    skip |= ValidateXYColorEXT(_parentObjects,
        _s242->x,
        _s242->y);
    const auto _s243 = &displayPrimaryGreen;
    skip |= ValidateXYColorEXT(_parentObjects,
        _s243->x,
        _s243->y);
    const auto _s244 = &displayPrimaryBlue;
    skip |= ValidateXYColorEXT(_parentObjects,
        _s244->x,
        _s244->y);
    const auto _s245 = &whitePoint;
    skip |= ValidateXYColorEXT(_parentObjects,
        _s245->x,
        _s245->y);
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
#ifdef VK_USE_PLATFORM_IOS_MVK
bool ExplicitValidation::ValidateIOSSurfaceCreateInfoMVK(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const VkIOSSurfaceCreateFlagsMVK flags,
                const void* pView) const {
    bool skip = false;
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
#endif
#ifdef VK_USE_PLATFORM_MACOS_MVK
bool ExplicitValidation::ValidateMacOSSurfaceCreateInfoMVK(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const VkMacOSSurfaceCreateFlagsMVK flags,
                const void* pView) const {
    bool skip = false;
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
#endif
bool ExplicitValidation::ValidateDebugUtilsLabelEXT(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const char* pLabelName,
                const float color[4]) const {
    bool skip = false;
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
bool ExplicitValidation::ValidateDebugUtilsObjectNameInfoEXT(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const VkObjectType objectType,
                const uint64_t objectHandle,
                const char* pObjectName) const {
    bool skip = false;
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
bool ExplicitValidation::ValidateDebugUtilsMessengerCallbackDataEXT(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const VkDebugUtilsMessengerCallbackDataFlagsEXT flags,
                const char* pMessageIdName,
                const int32_t messageIdNumber,
                const char* pMessage,
                const uint32_t queueLabelCount,
                const VkDebugUtilsLabelEXT* pQueueLabels,
                const uint32_t cmdBufLabelCount,
                const VkDebugUtilsLabelEXT* pCmdBufLabels,
                const uint32_t objectCount,
                const VkDebugUtilsObjectNameInfoEXT* pObjects) const {
    bool skip = false;
    if (pQueueLabels != nullptr) {
        for (uint32_t _i246 = 0;_i246 < queueLabelCount; ++_i246) {
            const auto _s247 = &pQueueLabels[_i246];
            skip |= ValidateDebugUtilsLabelEXT(_parentObjects,
                _s247->sType,
                _s247->pNext,
                _s247->pLabelName,
                _s247->color);
        }
    }
    if (pCmdBufLabels != nullptr) {
        for (uint32_t _i248 = 0;_i248 < cmdBufLabelCount; ++_i248) {
            const auto _s249 = &pCmdBufLabels[_i248];
            skip |= ValidateDebugUtilsLabelEXT(_parentObjects,
                _s249->sType,
                _s249->pNext,
                _s249->pLabelName,
                _s249->color);
        }
    }
    if (pObjects != nullptr) {
        for (uint32_t _i250 = 0;_i250 < objectCount; ++_i250) {
            const auto _s251 = &pObjects[_i250];
            skip |= ValidateDebugUtilsObjectNameInfoEXT(_parentObjects,
                _s251->sType,
                _s251->pNext,
                _s251->objectType,
                _s251->objectHandle,
                _s251->pObjectName);
        }
    }
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
bool ExplicitValidation::ValidateDebugUtilsMessengerCreateInfoEXT(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const VkDebugUtilsMessengerCreateFlagsEXT flags,
                const VkDebugUtilsMessageSeverityFlagsEXT messageSeverity,
                const VkDebugUtilsMessageTypeFlagsEXT messageType,
                const PFN_vkDebugUtilsMessengerCallbackEXT pfnUserCallback,
                const void* pUserData) const {
    bool skip = false;
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
bool ExplicitValidation::ValidateDebugUtilsObjectTagInfoEXT(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const VkObjectType objectType,
                const uint64_t objectHandle,
                const uint64_t tagName,
                const size_t tagSize,
                const void* pTag) const {
    bool skip = false;
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
#ifdef VK_USE_PLATFORM_ANDROID_KHR
bool ExplicitValidation::ValidateImportAndroidHardwareBufferInfoANDROID(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const struct AHardwareBuffer* buffer) const {
    bool skip = false;
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
#endif
#ifdef VK_USE_PLATFORM_ANDROID_KHR
bool ExplicitValidation::ValidateMemoryGetAndroidHardwareBufferInfoANDROID(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const VkDeviceMemory memory) const {
    bool skip = false;
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
#endif
#ifdef VK_USE_PLATFORM_ANDROID_KHR
bool ExplicitValidation::ValidateExternalFormatANDROID(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const uint64_t externalFormat) const {
    bool skip = false;
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
#endif
bool ExplicitValidation::ValidateSamplerReductionModeCreateInfoEXT(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const VkSamplerReductionMode reductionMode) const {
    return ValidateSamplerReductionModeCreateInfo(_parentObjects, sType, pNext, reductionMode);
}
bool ExplicitValidation::ValidatePhysicalDeviceInlineUniformBlockFeaturesEXT(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const VkBool32 inlineUniformBlock,
                const VkBool32 descriptorBindingInlineUniformBlockUpdateAfterBind) const {
    return ValidatePhysicalDeviceInlineUniformBlockFeatures(_parentObjects, sType, pNext, inlineUniformBlock, descriptorBindingInlineUniformBlockUpdateAfterBind);
}
bool ExplicitValidation::ValidateWriteDescriptorSetInlineUniformBlockEXT(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const uint32_t dataSize,
                const void* pData) const {
    return ValidateWriteDescriptorSetInlineUniformBlock(_parentObjects, sType, pNext, dataSize, pData);
}
bool ExplicitValidation::ValidateDescriptorPoolInlineUniformBlockCreateInfoEXT(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const uint32_t maxInlineUniformBlockBindings) const {
    return ValidateDescriptorPoolInlineUniformBlockCreateInfo(_parentObjects, sType, pNext, maxInlineUniformBlockBindings);
}
bool ExplicitValidation::ValidateSampleLocationEXT(const LogObjectList &_parentObjects,
                const float x,
                const float y) const {
    bool skip = false;
    return skip;
}
bool ExplicitValidation::ValidateSampleLocationsInfoEXT(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const VkSampleCountFlagBits sampleLocationsPerPixel,
                const VkExtent2D sampleLocationGridSize,
                const uint32_t sampleLocationsCount,
                const VkSampleLocationEXT* pSampleLocations) const {
    bool skip = false;
    const auto _s252 = &sampleLocationGridSize;
    skip |= ValidateExtent2D(_parentObjects,
        _s252->width,
        _s252->height);
    if (pSampleLocations != nullptr) {
        for (uint32_t _i253 = 0;_i253 < sampleLocationsCount; ++_i253) {
            const auto _s254 = &pSampleLocations[_i253];
            skip |= ValidateSampleLocationEXT(_parentObjects,
                _s254->x,
                _s254->y);
        }
    }
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
bool ExplicitValidation::ValidateAttachmentSampleLocationsEXT(const LogObjectList &_parentObjects,
                const uint32_t attachmentIndex,
                const VkSampleLocationsInfoEXT sampleLocationsInfo) const {
    bool skip = false;
    const auto _s255 = &sampleLocationsInfo;
    skip |= ValidateSampleLocationsInfoEXT(_parentObjects,
        _s255->sType,
        _s255->pNext,
        _s255->sampleLocationsPerPixel,
        _s255->sampleLocationGridSize,
        _s255->sampleLocationsCount,
        _s255->pSampleLocations);
    return skip;
}
bool ExplicitValidation::ValidateSubpassSampleLocationsEXT(const LogObjectList &_parentObjects,
                const uint32_t subpassIndex,
                const VkSampleLocationsInfoEXT sampleLocationsInfo) const {
    bool skip = false;
    const auto _s256 = &sampleLocationsInfo;
    skip |= ValidateSampleLocationsInfoEXT(_parentObjects,
        _s256->sType,
        _s256->pNext,
        _s256->sampleLocationsPerPixel,
        _s256->sampleLocationGridSize,
        _s256->sampleLocationsCount,
        _s256->pSampleLocations);
    return skip;
}
bool ExplicitValidation::ValidateRenderPassSampleLocationsBeginInfoEXT(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const uint32_t attachmentInitialSampleLocationsCount,
                const VkAttachmentSampleLocationsEXT* pAttachmentInitialSampleLocations,
                const uint32_t postSubpassSampleLocationsCount,
                const VkSubpassSampleLocationsEXT* pPostSubpassSampleLocations) const {
    bool skip = false;
    if (pAttachmentInitialSampleLocations != nullptr) {
        for (uint32_t _i257 = 0;_i257 < attachmentInitialSampleLocationsCount; ++_i257) {
            const auto _s258 = &pAttachmentInitialSampleLocations[_i257];
            skip |= ValidateAttachmentSampleLocationsEXT(_parentObjects,
                _s258->attachmentIndex,
                _s258->sampleLocationsInfo);
        }
    }
    if (pPostSubpassSampleLocations != nullptr) {
        for (uint32_t _i259 = 0;_i259 < postSubpassSampleLocationsCount; ++_i259) {
            const auto _s260 = &pPostSubpassSampleLocations[_i259];
            skip |= ValidateSubpassSampleLocationsEXT(_parentObjects,
                _s260->subpassIndex,
                _s260->sampleLocationsInfo);
        }
    }
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
bool ExplicitValidation::ValidatePipelineSampleLocationsStateCreateInfoEXT(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const VkBool32 sampleLocationsEnable,
                const VkSampleLocationsInfoEXT sampleLocationsInfo) const {
    bool skip = false;
    const auto _s261 = &sampleLocationsInfo;
    skip |= ValidateSampleLocationsInfoEXT(_parentObjects,
        _s261->sType,
        _s261->pNext,
        _s261->sampleLocationsPerPixel,
        _s261->sampleLocationGridSize,
        _s261->sampleLocationsCount,
        _s261->pSampleLocations);
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
bool ExplicitValidation::ValidatePhysicalDeviceBlendOperationAdvancedFeaturesEXT(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const VkBool32 advancedBlendCoherentOperations) const {
    bool skip = false;
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
bool ExplicitValidation::ValidatePipelineColorBlendAdvancedStateCreateInfoEXT(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const VkBool32 srcPremultiplied,
                const VkBool32 dstPremultiplied,
                const VkBlendOverlapEXT blendOverlap) const {
    bool skip = false;
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
bool ExplicitValidation::ValidatePipelineCoverageToColorStateCreateInfoNV(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const VkPipelineCoverageToColorStateCreateFlagsNV flags,
                const VkBool32 coverageToColorEnable,
                const uint32_t coverageToColorLocation) const {
    bool skip = false;
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
bool ExplicitValidation::ValidatePipelineCoverageModulationStateCreateInfoNV(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const VkPipelineCoverageModulationStateCreateFlagsNV flags,
                const VkCoverageModulationModeNV coverageModulationMode,
                const VkBool32 coverageModulationTableEnable,
                const uint32_t coverageModulationTableCount,
                const float* pCoverageModulationTable) const {
    bool skip = false;
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
bool ExplicitValidation::ValidatePhysicalDeviceShaderSMBuiltinsFeaturesNV(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const VkBool32 shaderSMBuiltins) const {
    bool skip = false;
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
bool ExplicitValidation::ValidatePhysicalDeviceImageDrmFormatModifierInfoEXT(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const uint64_t drmFormatModifier,
                const VkSharingMode sharingMode,
                const uint32_t queueFamilyIndexCount,
                const uint32_t* pQueueFamilyIndices) const {
    bool skip = false;
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
bool ExplicitValidation::ValidateImageDrmFormatModifierListCreateInfoEXT(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const uint32_t drmFormatModifierCount,
                const uint64_t* pDrmFormatModifiers) const {
    bool skip = false;
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
bool ExplicitValidation::ValidateImageDrmFormatModifierExplicitCreateInfoEXT(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const uint64_t drmFormatModifier,
                const uint32_t drmFormatModifierPlaneCount,
                const VkSubresourceLayout* pPlaneLayouts) const {
    bool skip = false;
    if (pPlaneLayouts != nullptr) {
        for (uint32_t _i262 = 0;_i262 < drmFormatModifierPlaneCount; ++_i262) {
            const auto _s263 = &pPlaneLayouts[_i262];
            skip |= ValidateSubresourceLayout(_parentObjects,
                _s263->offset,
                _s263->size,
                _s263->rowPitch,
                _s263->arrayPitch,
                _s263->depthPitch);
        }
    }
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
bool ExplicitValidation::ValidateValidationCacheCreateInfoEXT(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const VkValidationCacheCreateFlagsEXT flags,
                const size_t initialDataSize,
                const void* pInitialData) const {
    bool skip = false;
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
bool ExplicitValidation::ValidateShaderModuleValidationCacheCreateInfoEXT(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const VkValidationCacheEXT validationCache) const {
    bool skip = false;
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
bool ExplicitValidation::ValidateDescriptorSetLayoutBindingFlagsCreateInfoEXT(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const uint32_t bindingCount,
                const VkDescriptorBindingFlags* pBindingFlags) const {
    return ValidateDescriptorSetLayoutBindingFlagsCreateInfo(_parentObjects, sType, pNext, bindingCount, pBindingFlags);
}
bool ExplicitValidation::ValidatePhysicalDeviceDescriptorIndexingFeaturesEXT(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const VkBool32 shaderInputAttachmentArrayDynamicIndexing,
                const VkBool32 shaderUniformTexelBufferArrayDynamicIndexing,
                const VkBool32 shaderStorageTexelBufferArrayDynamicIndexing,
                const VkBool32 shaderUniformBufferArrayNonUniformIndexing,
                const VkBool32 shaderSampledImageArrayNonUniformIndexing,
                const VkBool32 shaderStorageBufferArrayNonUniformIndexing,
                const VkBool32 shaderStorageImageArrayNonUniformIndexing,
                const VkBool32 shaderInputAttachmentArrayNonUniformIndexing,
                const VkBool32 shaderUniformTexelBufferArrayNonUniformIndexing,
                const VkBool32 shaderStorageTexelBufferArrayNonUniformIndexing,
                const VkBool32 descriptorBindingUniformBufferUpdateAfterBind,
                const VkBool32 descriptorBindingSampledImageUpdateAfterBind,
                const VkBool32 descriptorBindingStorageImageUpdateAfterBind,
                const VkBool32 descriptorBindingStorageBufferUpdateAfterBind,
                const VkBool32 descriptorBindingUniformTexelBufferUpdateAfterBind,
                const VkBool32 descriptorBindingStorageTexelBufferUpdateAfterBind,
                const VkBool32 descriptorBindingUpdateUnusedWhilePending,
                const VkBool32 descriptorBindingPartiallyBound,
                const VkBool32 descriptorBindingVariableDescriptorCount,
                const VkBool32 runtimeDescriptorArray) const {
    return ValidatePhysicalDeviceDescriptorIndexingFeatures(_parentObjects, sType, pNext, shaderInputAttachmentArrayDynamicIndexing, shaderUniformTexelBufferArrayDynamicIndexing, shaderStorageTexelBufferArrayDynamicIndexing, shaderUniformBufferArrayNonUniformIndexing, shaderSampledImageArrayNonUniformIndexing, shaderStorageBufferArrayNonUniformIndexing, shaderStorageImageArrayNonUniformIndexing, shaderInputAttachmentArrayNonUniformIndexing, shaderUniformTexelBufferArrayNonUniformIndexing, shaderStorageTexelBufferArrayNonUniformIndexing, descriptorBindingUniformBufferUpdateAfterBind, descriptorBindingSampledImageUpdateAfterBind, descriptorBindingStorageImageUpdateAfterBind, descriptorBindingStorageBufferUpdateAfterBind, descriptorBindingUniformTexelBufferUpdateAfterBind, descriptorBindingStorageTexelBufferUpdateAfterBind, descriptorBindingUpdateUnusedWhilePending, descriptorBindingPartiallyBound, descriptorBindingVariableDescriptorCount, runtimeDescriptorArray);
}
bool ExplicitValidation::ValidateDescriptorSetVariableDescriptorCountAllocateInfoEXT(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const uint32_t descriptorSetCount,
                const uint32_t* pDescriptorCounts) const {
    return ValidateDescriptorSetVariableDescriptorCountAllocateInfo(_parentObjects, sType, pNext, descriptorSetCount, pDescriptorCounts);
}
bool ExplicitValidation::ValidateShadingRatePaletteNV(const LogObjectList &_parentObjects,
                const uint32_t shadingRatePaletteEntryCount,
                const VkShadingRatePaletteEntryNV* pShadingRatePaletteEntries) const {
    bool skip = false;
    return skip;
}
bool ExplicitValidation::ValidatePipelineViewportShadingRateImageStateCreateInfoNV(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const VkBool32 shadingRateImageEnable,
                const uint32_t viewportCount,
                const VkShadingRatePaletteNV* pShadingRatePalettes) const {
    bool skip = false;
    if (pShadingRatePalettes != nullptr) {
        for (uint32_t _i264 = 0;_i264 < viewportCount; ++_i264) {
            const auto _s265 = &pShadingRatePalettes[_i264];
            skip |= ValidateShadingRatePaletteNV(_parentObjects,
                _s265->shadingRatePaletteEntryCount,
                _s265->pShadingRatePaletteEntries);
        }
    }
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
bool ExplicitValidation::ValidatePhysicalDeviceShadingRateImageFeaturesNV(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const VkBool32 shadingRateImage,
                const VkBool32 shadingRateCoarseSampleOrder) const {
    bool skip = false;
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
bool ExplicitValidation::ValidateCoarseSampleLocationNV(const LogObjectList &_parentObjects,
                const uint32_t pixelX,
                const uint32_t pixelY,
                const uint32_t sample) const {
    bool skip = false;
    return skip;
}
bool ExplicitValidation::ValidateCoarseSampleOrderCustomNV(const LogObjectList &_parentObjects,
                const VkShadingRatePaletteEntryNV shadingRate,
                const uint32_t sampleCount,
                const uint32_t sampleLocationCount,
                const VkCoarseSampleLocationNV* pSampleLocations) const {
    bool skip = false;
    if (pSampleLocations != nullptr) {
        for (uint32_t _i266 = 0;_i266 < sampleLocationCount; ++_i266) {
            const auto _s267 = &pSampleLocations[_i266];
            skip |= ValidateCoarseSampleLocationNV(_parentObjects,
                _s267->pixelX,
                _s267->pixelY,
                _s267->sample);
        }
    }
    return skip;
}
bool ExplicitValidation::ValidatePipelineViewportCoarseSampleOrderStateCreateInfoNV(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const VkCoarseSampleOrderTypeNV sampleOrderType,
                const uint32_t customSampleOrderCount,
                const VkCoarseSampleOrderCustomNV* pCustomSampleOrders) const {
    bool skip = false;
    if (pCustomSampleOrders != nullptr) {
        for (uint32_t _i268 = 0;_i268 < customSampleOrderCount; ++_i268) {
            const auto _s269 = &pCustomSampleOrders[_i268];
            skip |= ValidateCoarseSampleOrderCustomNV(_parentObjects,
                _s269->shadingRate,
                _s269->sampleCount,
                _s269->sampleLocationCount,
                _s269->pSampleLocations);
        }
    }
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
bool ExplicitValidation::ValidateRayTracingShaderGroupCreateInfoNV(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const VkRayTracingShaderGroupTypeKHR type,
                const uint32_t generalShader,
                const uint32_t closestHitShader,
                const uint32_t anyHitShader,
                const uint32_t intersectionShader) const {
    bool skip = false;
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
bool ExplicitValidation::ValidateRayTracingPipelineCreateInfoNV(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const VkPipelineCreateFlags flags,
                const uint32_t stageCount,
                const VkPipelineShaderStageCreateInfo* pStages,
                const uint32_t groupCount,
                const VkRayTracingShaderGroupCreateInfoNV* pGroups,
                const uint32_t maxRecursionDepth,
                const VkPipelineLayout layout,
                const VkPipeline basePipelineHandle,
                const int32_t basePipelineIndex) const {
    bool skip = false;
    if (pStages != nullptr) {
        for (uint32_t _i270 = 0;_i270 < stageCount; ++_i270) {
            const auto _s271 = &pStages[_i270];
            skip |= ValidatePipelineShaderStageCreateInfo(_parentObjects,
                _s271->sType,
                _s271->pNext,
                _s271->flags,
                _s271->stage,
                _s271->module,
                _s271->pName,
                _s271->pSpecializationInfo);
        }
    }
    if (pGroups != nullptr) {
        for (uint32_t _i272 = 0;_i272 < groupCount; ++_i272) {
            const auto _s273 = &pGroups[_i272];
            skip |= ValidateRayTracingShaderGroupCreateInfoNV(_parentObjects,
                _s273->sType,
                _s273->pNext,
                _s273->type,
                _s273->generalShader,
                _s273->closestHitShader,
                _s273->anyHitShader,
                _s273->intersectionShader);
        }
    }
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
bool ExplicitValidation::ValidateGeometryTrianglesNV(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const VkBuffer vertexData,
                const VkDeviceSize vertexOffset,
                const uint32_t vertexCount,
                const VkDeviceSize vertexStride,
                const VkFormat vertexFormat,
                const VkBuffer indexData,
                const VkDeviceSize indexOffset,
                const uint32_t indexCount,
                const VkIndexType indexType,
                const VkBuffer transformData,
                const VkDeviceSize transformOffset) const {
    bool skip = false;
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
bool ExplicitValidation::ValidateGeometryAABBNV(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const VkBuffer aabbData,
                const uint32_t numAABBs,
                const uint32_t stride,
                const VkDeviceSize offset) const {
    bool skip = false;
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
bool ExplicitValidation::ValidateGeometryDataNV(const LogObjectList &_parentObjects,
                const VkGeometryTrianglesNV triangles,
                const VkGeometryAABBNV aabbs) const {
    bool skip = false;
    const auto _s274 = &triangles;
    skip |= ValidateGeometryTrianglesNV(_parentObjects,
        _s274->sType,
        _s274->pNext,
        _s274->vertexData,
        _s274->vertexOffset,
        _s274->vertexCount,
        _s274->vertexStride,
        _s274->vertexFormat,
        _s274->indexData,
        _s274->indexOffset,
        _s274->indexCount,
        _s274->indexType,
        _s274->transformData,
        _s274->transformOffset);
    const auto _s275 = &aabbs;
    skip |= ValidateGeometryAABBNV(_parentObjects,
        _s275->sType,
        _s275->pNext,
        _s275->aabbData,
        _s275->numAABBs,
        _s275->stride,
        _s275->offset);
    return skip;
}
bool ExplicitValidation::ValidateGeometryNV(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const VkGeometryTypeKHR geometryType,
                const VkGeometryDataNV geometry,
                const VkGeometryFlagsKHR flags) const {
    bool skip = false;
    const auto _s276 = &geometry;
    skip |= ValidateGeometryDataNV(_parentObjects,
        _s276->triangles,
        _s276->aabbs);
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
bool ExplicitValidation::ValidateAccelerationStructureInfoNV(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const VkAccelerationStructureTypeNV type,
                const VkBuildAccelerationStructureFlagsNV flags,
                const uint32_t instanceCount,
                const uint32_t geometryCount,
                const VkGeometryNV* pGeometries) const {
    bool skip = false;
    if (pGeometries != nullptr) {
        for (uint32_t _i277 = 0;_i277 < geometryCount; ++_i277) {
            const auto _s278 = &pGeometries[_i277];
            skip |= ValidateGeometryNV(_parentObjects,
                _s278->sType,
                _s278->pNext,
                _s278->geometryType,
                _s278->geometry,
                _s278->flags);
        }
    }
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
bool ExplicitValidation::ValidateAccelerationStructureCreateInfoNV(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const VkDeviceSize compactedSize,
                const VkAccelerationStructureInfoNV info) const {
    bool skip = false;
    const auto _s279 = &info;
    skip |= ValidateAccelerationStructureInfoNV(_parentObjects,
        _s279->sType,
        _s279->pNext,
        _s279->type,
        _s279->flags,
        _s279->instanceCount,
        _s279->geometryCount,
        _s279->pGeometries);
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
bool ExplicitValidation::ValidateBindAccelerationStructureMemoryInfoNV(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const VkAccelerationStructureNV accelerationStructure,
                const VkDeviceMemory memory,
                const VkDeviceSize memoryOffset,
                const uint32_t deviceIndexCount,
                const uint32_t* pDeviceIndices) const {
    bool skip = false;
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
bool ExplicitValidation::ValidateWriteDescriptorSetAccelerationStructureNV(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const uint32_t accelerationStructureCount,
                const VkAccelerationStructureNV* pAccelerationStructures) const {
    bool skip = false;
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
bool ExplicitValidation::ValidateAccelerationStructureMemoryRequirementsInfoNV(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const VkAccelerationStructureMemoryRequirementsTypeNV type,
                const VkAccelerationStructureNV accelerationStructure) const {
    bool skip = false;
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
bool ExplicitValidation::ValidateTransformMatrixKHR(const LogObjectList &_parentObjects,
                const float matrix[3][4]) const {
    bool skip = false;
    return skip;
}
bool ExplicitValidation::ValidateTransformMatrixNV(const LogObjectList &_parentObjects,
                const float matrix[3][4]) const {
    return ValidateTransformMatrixKHR(_parentObjects, matrix);
}
bool ExplicitValidation::ValidateAabbPositionsKHR(const LogObjectList &_parentObjects,
                const float minX,
                const float minY,
                const float minZ,
                const float maxX,
                const float maxY,
                const float maxZ) const {
    bool skip = false;
    return skip;
}
bool ExplicitValidation::ValidateAabbPositionsNV(const LogObjectList &_parentObjects,
                const float minX,
                const float minY,
                const float minZ,
                const float maxX,
                const float maxY,
                const float maxZ) const {
    return ValidateAabbPositionsKHR(_parentObjects, minX, minY, minZ, maxX, maxY, maxZ);
}
bool ExplicitValidation::ValidateAccelerationStructureInstanceKHR(const LogObjectList &_parentObjects,
                const VkTransformMatrixKHR transform,
                const uint32_t instanceCustomIndex,
                const uint32_t mask,
                const uint32_t instanceShaderBindingTableRecordOffset,
                const VkGeometryInstanceFlagsKHR flags,
                const uint64_t accelerationStructureReference) const {
    bool skip = false;
    const auto _s280 = &transform;
    skip |= ValidateTransformMatrixKHR(_parentObjects,
        _s280->matrix);
    return skip;
}
bool ExplicitValidation::ValidateAccelerationStructureInstanceNV(const LogObjectList &_parentObjects,
                const VkTransformMatrixKHR transform,
                const uint32_t instanceCustomIndex,
                const uint32_t mask,
                const uint32_t instanceShaderBindingTableRecordOffset,
                const VkGeometryInstanceFlagsKHR flags,
                const uint64_t accelerationStructureReference) const {
    return ValidateAccelerationStructureInstanceKHR(_parentObjects, transform, instanceCustomIndex, mask, instanceShaderBindingTableRecordOffset, flags, accelerationStructureReference);
}
bool ExplicitValidation::ValidatePhysicalDeviceRepresentativeFragmentTestFeaturesNV(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const VkBool32 representativeFragmentTest) const {
    bool skip = false;
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
bool ExplicitValidation::ValidatePipelineRepresentativeFragmentTestStateCreateInfoNV(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const VkBool32 representativeFragmentTestEnable) const {
    bool skip = false;
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
bool ExplicitValidation::ValidatePhysicalDeviceImageViewImageFormatInfoEXT(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const VkImageViewType imageViewType) const {
    bool skip = false;
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
bool ExplicitValidation::ValidateDeviceQueueGlobalPriorityCreateInfoEXT(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const VkQueueGlobalPriorityKHR globalPriority) const {
    return ValidateDeviceQueueGlobalPriorityCreateInfoKHR(_parentObjects, sType, pNext, globalPriority);
}
bool ExplicitValidation::ValidateImportMemoryHostPointerInfoEXT(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const VkExternalMemoryHandleTypeFlagBits handleType,
                const void* pHostPointer) const {
    bool skip = false;
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
bool ExplicitValidation::ValidatePipelineCompilerControlCreateInfoAMD(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const VkPipelineCompilerControlFlagsAMD compilerControlFlags) const {
    bool skip = false;
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
bool ExplicitValidation::ValidateCalibratedTimestampInfoEXT(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const VkTimeDomainEXT timeDomain) const {
    bool skip = false;
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
bool ExplicitValidation::ValidateDeviceMemoryOverallocationCreateInfoAMD(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const VkMemoryOverallocationBehaviorAMD overallocationBehavior) const {
    bool skip = false;
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
bool ExplicitValidation::ValidateVertexInputBindingDivisorDescriptionEXT(const LogObjectList &_parentObjects,
                const uint32_t binding,
                const uint32_t divisor) const {
    bool skip = false;
    return skip;
}
bool ExplicitValidation::ValidatePipelineVertexInputDivisorStateCreateInfoEXT(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const uint32_t vertexBindingDivisorCount,
                const VkVertexInputBindingDivisorDescriptionEXT* pVertexBindingDivisors) const {
    bool skip = false;
    if (pVertexBindingDivisors != nullptr) {
        for (uint32_t _i281 = 0;_i281 < vertexBindingDivisorCount; ++_i281) {
            const auto _s282 = &pVertexBindingDivisors[_i281];
            skip |= ValidateVertexInputBindingDivisorDescriptionEXT(_parentObjects,
                _s282->binding,
                _s282->divisor);
        }
    }
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
bool ExplicitValidation::ValidatePhysicalDeviceVertexAttributeDivisorFeaturesEXT(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const VkBool32 vertexAttributeInstanceRateDivisor,
                const VkBool32 vertexAttributeInstanceRateZeroDivisor) const {
    bool skip = false;
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
#ifdef VK_USE_PLATFORM_GGP
bool ExplicitValidation::ValidatePresentFrameTokenGGP(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const GgpFrameToken frameToken) const {
    bool skip = false;
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
#endif
bool ExplicitValidation::ValidatePipelineCreationFeedbackCreateInfoEXT(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const VkPipelineCreationFeedback* pPipelineCreationFeedback,
                const uint32_t pipelineStageCreationFeedbackCount,
                const VkPipelineCreationFeedback* pPipelineStageCreationFeedbacks) const {
    return ValidatePipelineCreationFeedbackCreateInfo(_parentObjects, sType, pNext, pPipelineCreationFeedback, pipelineStageCreationFeedbackCount, pPipelineStageCreationFeedbacks);
}
bool ExplicitValidation::ValidatePhysicalDeviceComputeShaderDerivativesFeaturesNV(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const VkBool32 computeDerivativeGroupQuads,
                const VkBool32 computeDerivativeGroupLinear) const {
    bool skip = false;
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
bool ExplicitValidation::ValidatePhysicalDeviceMeshShaderFeaturesNV(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const VkBool32 taskShader,
                const VkBool32 meshShader) const {
    bool skip = false;
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
bool ExplicitValidation::ValidateDrawMeshTasksIndirectCommandNV(const LogObjectList &_parentObjects,
                const uint32_t taskCount,
                const uint32_t firstTask) const {
    bool skip = false;
    return skip;
}
bool ExplicitValidation::ValidatePhysicalDeviceFragmentShaderBarycentricFeaturesNV(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const VkBool32 fragmentShaderBarycentric) const {
    return ValidatePhysicalDeviceFragmentShaderBarycentricFeaturesKHR(_parentObjects, sType, pNext, fragmentShaderBarycentric);
}
bool ExplicitValidation::ValidatePhysicalDeviceShaderImageFootprintFeaturesNV(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const VkBool32 imageFootprint) const {
    bool skip = false;
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
bool ExplicitValidation::ValidatePipelineViewportExclusiveScissorStateCreateInfoNV(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const uint32_t exclusiveScissorCount,
                const VkRect2D* pExclusiveScissors) const {
    bool skip = false;
    if (pExclusiveScissors != nullptr) {
        for (uint32_t _i283 = 0;_i283 < exclusiveScissorCount; ++_i283) {
            const auto _s284 = &pExclusiveScissors[_i283];
            skip |= ValidateRect2D(_parentObjects,
                _s284->offset,
                _s284->extent);
        }
    }
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
bool ExplicitValidation::ValidatePhysicalDeviceExclusiveScissorFeaturesNV(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const VkBool32 exclusiveScissor) const {
    bool skip = false;
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
bool ExplicitValidation::ValidatePhysicalDeviceShaderIntegerFunctions2FeaturesINTEL(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const VkBool32 shaderIntegerFunctions2) const {
    bool skip = false;
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
bool ExplicitValidation::ValidatePerformanceValueDataINTEL(const LogObjectList &_parentObjects,
                const uint32_t value32,
                const uint64_t value64,
                const float valueFloat,
                const VkBool32 valueBool,
                const char* valueString) const {
    bool skip = false;
    return skip;
}
bool ExplicitValidation::ValidatePerformanceValueINTEL(const LogObjectList &_parentObjects,
                const VkPerformanceValueTypeINTEL type,
                const VkPerformanceValueDataINTEL data) const {
    bool skip = false;
    const auto _s285 = &data;
    skip |= ValidatePerformanceValueDataINTEL(_parentObjects,
        _s285->value32,
        _s285->value64,
        _s285->valueFloat,
        _s285->valueBool,
        _s285->valueString);
    return skip;
}
bool ExplicitValidation::ValidateInitializePerformanceApiInfoINTEL(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const void* pUserData) const {
    bool skip = false;
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
bool ExplicitValidation::ValidateQueryPoolPerformanceQueryCreateInfoINTEL(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const VkQueryPoolSamplingModeINTEL performanceCountersSampling) const {
    bool skip = false;
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
bool ExplicitValidation::ValidateQueryPoolCreateInfoINTEL(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const VkQueryPoolSamplingModeINTEL performanceCountersSampling) const {
    return ValidateQueryPoolPerformanceQueryCreateInfoINTEL(_parentObjects, sType, pNext, performanceCountersSampling);
}
bool ExplicitValidation::ValidatePerformanceMarkerInfoINTEL(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const uint64_t marker) const {
    bool skip = false;
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
bool ExplicitValidation::ValidatePerformanceStreamMarkerInfoINTEL(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const uint32_t marker) const {
    bool skip = false;
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
bool ExplicitValidation::ValidatePerformanceOverrideInfoINTEL(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const VkPerformanceOverrideTypeINTEL type,
                const VkBool32 enable,
                const uint64_t parameter) const {
    bool skip = false;
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
bool ExplicitValidation::ValidatePerformanceConfigurationAcquireInfoINTEL(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const VkPerformanceConfigurationTypeINTEL type) const {
    bool skip = false;
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
bool ExplicitValidation::ValidateSwapchainDisplayNativeHdrCreateInfoAMD(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const VkBool32 localDimmingEnable) const {
    bool skip = false;
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
#ifdef VK_USE_PLATFORM_FUCHSIA
bool ExplicitValidation::ValidateImagePipeSurfaceCreateInfoFUCHSIA(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const VkImagePipeSurfaceCreateFlagsFUCHSIA flags,
                const zx_handle_t imagePipeHandle) const {
    bool skip = false;
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
#endif
#ifdef VK_USE_PLATFORM_METAL_EXT
bool ExplicitValidation::ValidateMetalSurfaceCreateInfoEXT(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const VkMetalSurfaceCreateFlagsEXT flags,
                const CAMetalLayer* pLayer) const {
    bool skip = false;
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
#endif
bool ExplicitValidation::ValidatePhysicalDeviceFragmentDensityMapFeaturesEXT(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const VkBool32 fragmentDensityMap,
                const VkBool32 fragmentDensityMapDynamic,
                const VkBool32 fragmentDensityMapNonSubsampledImages) const {
    bool skip = false;
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
bool ExplicitValidation::ValidateRenderPassFragmentDensityMapCreateInfoEXT(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const VkAttachmentReference fragmentDensityMapAttachment) const {
    bool skip = false;
    const auto _s286 = &fragmentDensityMapAttachment;
    skip |= ValidateAttachmentReference(_parentObjects,
        _s286->attachment,
        _s286->layout);
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
bool ExplicitValidation::ValidatePhysicalDeviceScalarBlockLayoutFeaturesEXT(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const VkBool32 scalarBlockLayout) const {
    return ValidatePhysicalDeviceScalarBlockLayoutFeatures(_parentObjects, sType, pNext, scalarBlockLayout);
}
bool ExplicitValidation::ValidatePhysicalDeviceSubgroupSizeControlFeaturesEXT(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const VkBool32 subgroupSizeControl,
                const VkBool32 computeFullSubgroups) const {
    return ValidatePhysicalDeviceSubgroupSizeControlFeatures(_parentObjects, sType, pNext, subgroupSizeControl, computeFullSubgroups);
}
bool ExplicitValidation::ValidatePhysicalDeviceCoherentMemoryFeaturesAMD(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const VkBool32 deviceCoherentMemory) const {
    bool skip = false;
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
bool ExplicitValidation::ValidatePhysicalDeviceShaderImageAtomicInt64FeaturesEXT(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const VkBool32 shaderImageInt64Atomics,
                const VkBool32 sparseImageInt64Atomics) const {
    bool skip = false;
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
bool ExplicitValidation::ValidatePhysicalDeviceMemoryPriorityFeaturesEXT(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const VkBool32 memoryPriority) const {
    bool skip = false;
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
bool ExplicitValidation::ValidateMemoryPriorityAllocateInfoEXT(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const float priority) const {
    bool skip = false;
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
bool ExplicitValidation::ValidatePhysicalDeviceDedicatedAllocationImageAliasingFeaturesNV(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const VkBool32 dedicatedAllocationImageAliasing) const {
    bool skip = false;
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
bool ExplicitValidation::ValidatePhysicalDeviceBufferDeviceAddressFeaturesEXT(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const VkBool32 bufferDeviceAddress,
                const VkBool32 bufferDeviceAddressCaptureReplay,
                const VkBool32 bufferDeviceAddressMultiDevice) const {
    bool skip = false;
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
bool ExplicitValidation::ValidatePhysicalDeviceBufferAddressFeaturesEXT(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const VkBool32 bufferDeviceAddress,
                const VkBool32 bufferDeviceAddressCaptureReplay,
                const VkBool32 bufferDeviceAddressMultiDevice) const {
    return ValidatePhysicalDeviceBufferDeviceAddressFeaturesEXT(_parentObjects, sType, pNext, bufferDeviceAddress, bufferDeviceAddressCaptureReplay, bufferDeviceAddressMultiDevice);
}
bool ExplicitValidation::ValidateBufferDeviceAddressInfoEXT(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const VkBuffer buffer) const {
    return ValidateBufferDeviceAddressInfo(_parentObjects, sType, pNext, buffer);
}
bool ExplicitValidation::ValidateBufferDeviceAddressCreateInfoEXT(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const VkDeviceAddress deviceAddress) const {
    bool skip = false;
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
bool ExplicitValidation::ValidateImageStencilUsageCreateInfoEXT(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const VkImageUsageFlags stencilUsage) const {
    return ValidateImageStencilUsageCreateInfo(_parentObjects, sType, pNext, stencilUsage);
}
bool ExplicitValidation::ValidateValidationFeaturesEXT(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const uint32_t enabledValidationFeatureCount,
                const VkValidationFeatureEnableEXT* pEnabledValidationFeatures,
                const uint32_t disabledValidationFeatureCount,
                const VkValidationFeatureDisableEXT* pDisabledValidationFeatures) const {
    bool skip = false;
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
bool ExplicitValidation::ValidateCooperativeMatrixPropertiesNV(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const uint32_t MSize,
                const uint32_t NSize,
                const uint32_t KSize,
                const VkComponentTypeNV AType,
                const VkComponentTypeNV BType,
                const VkComponentTypeNV CType,
                const VkComponentTypeNV DType,
                const VkScopeNV scope) const {
    bool skip = false;
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
bool ExplicitValidation::ValidatePhysicalDeviceCooperativeMatrixFeaturesNV(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const VkBool32 cooperativeMatrix,
                const VkBool32 cooperativeMatrixRobustBufferAccess) const {
    bool skip = false;
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
bool ExplicitValidation::ValidatePhysicalDeviceCoverageReductionModeFeaturesNV(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const VkBool32 coverageReductionMode) const {
    bool skip = false;
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
bool ExplicitValidation::ValidatePipelineCoverageReductionStateCreateInfoNV(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const VkPipelineCoverageReductionStateCreateFlagsNV flags,
                const VkCoverageReductionModeNV coverageReductionMode) const {
    bool skip = false;
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
bool ExplicitValidation::ValidatePhysicalDeviceFragmentShaderInterlockFeaturesEXT(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const VkBool32 fragmentShaderSampleInterlock,
                const VkBool32 fragmentShaderPixelInterlock,
                const VkBool32 fragmentShaderShadingRateInterlock) const {
    bool skip = false;
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
bool ExplicitValidation::ValidatePhysicalDeviceYcbcrImageArraysFeaturesEXT(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const VkBool32 ycbcrImageArrays) const {
    bool skip = false;
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
bool ExplicitValidation::ValidatePhysicalDeviceProvokingVertexFeaturesEXT(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const VkBool32 provokingVertexLast,
                const VkBool32 transformFeedbackPreservesProvokingVertex) const {
    bool skip = false;
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
bool ExplicitValidation::ValidatePipelineRasterizationProvokingVertexStateCreateInfoEXT(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const VkProvokingVertexModeEXT provokingVertexMode) const {
    bool skip = false;
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
#ifdef VK_USE_PLATFORM_WIN32_KHR
bool ExplicitValidation::ValidateSurfaceFullScreenExclusiveInfoEXT(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const VkFullScreenExclusiveEXT fullScreenExclusive) const {
    bool skip = false;
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
#endif
#ifdef VK_USE_PLATFORM_WIN32_KHR
bool ExplicitValidation::ValidateSurfaceCapabilitiesFullScreenExclusiveEXT(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const VkBool32 fullScreenExclusiveSupported) const {
    bool skip = false;
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
#endif
#ifdef VK_USE_PLATFORM_WIN32_KHR
bool ExplicitValidation::ValidateSurfaceFullScreenExclusiveWin32InfoEXT(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const HMONITOR hmonitor) const {
    bool skip = false;
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
#endif
bool ExplicitValidation::ValidateHeadlessSurfaceCreateInfoEXT(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const VkHeadlessSurfaceCreateFlagsEXT flags) const {
    bool skip = false;
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
bool ExplicitValidation::ValidatePhysicalDeviceLineRasterizationFeaturesEXT(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const VkBool32 rectangularLines,
                const VkBool32 bresenhamLines,
                const VkBool32 smoothLines,
                const VkBool32 stippledRectangularLines,
                const VkBool32 stippledBresenhamLines,
                const VkBool32 stippledSmoothLines) const {
    bool skip = false;
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
bool ExplicitValidation::ValidatePipelineRasterizationLineStateCreateInfoEXT(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const VkLineRasterizationModeEXT lineRasterizationMode,
                const VkBool32 stippledLineEnable,
                const uint32_t lineStippleFactor,
                const uint16_t lineStipplePattern) const {
    bool skip = false;
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
bool ExplicitValidation::ValidatePhysicalDeviceShaderAtomicFloatFeaturesEXT(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const VkBool32 shaderBufferFloat32Atomics,
                const VkBool32 shaderBufferFloat32AtomicAdd,
                const VkBool32 shaderBufferFloat64Atomics,
                const VkBool32 shaderBufferFloat64AtomicAdd,
                const VkBool32 shaderSharedFloat32Atomics,
                const VkBool32 shaderSharedFloat32AtomicAdd,
                const VkBool32 shaderSharedFloat64Atomics,
                const VkBool32 shaderSharedFloat64AtomicAdd,
                const VkBool32 shaderImageFloat32Atomics,
                const VkBool32 shaderImageFloat32AtomicAdd,
                const VkBool32 sparseImageFloat32Atomics,
                const VkBool32 sparseImageFloat32AtomicAdd) const {
    bool skip = false;
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
bool ExplicitValidation::ValidatePhysicalDeviceHostQueryResetFeaturesEXT(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const VkBool32 hostQueryReset) const {
    return ValidatePhysicalDeviceHostQueryResetFeatures(_parentObjects, sType, pNext, hostQueryReset);
}
bool ExplicitValidation::ValidatePhysicalDeviceIndexTypeUint8FeaturesEXT(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const VkBool32 indexTypeUint8) const {
    bool skip = false;
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
bool ExplicitValidation::ValidatePhysicalDeviceExtendedDynamicStateFeaturesEXT(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const VkBool32 extendedDynamicState) const {
    bool skip = false;
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
bool ExplicitValidation::ValidatePhysicalDeviceShaderAtomicFloat2FeaturesEXT(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const VkBool32 shaderBufferFloat16Atomics,
                const VkBool32 shaderBufferFloat16AtomicAdd,
                const VkBool32 shaderBufferFloat16AtomicMinMax,
                const VkBool32 shaderBufferFloat32AtomicMinMax,
                const VkBool32 shaderBufferFloat64AtomicMinMax,
                const VkBool32 shaderSharedFloat16Atomics,
                const VkBool32 shaderSharedFloat16AtomicAdd,
                const VkBool32 shaderSharedFloat16AtomicMinMax,
                const VkBool32 shaderSharedFloat32AtomicMinMax,
                const VkBool32 shaderSharedFloat64AtomicMinMax,
                const VkBool32 shaderImageFloat32AtomicMinMax,
                const VkBool32 sparseImageFloat32AtomicMinMax) const {
    bool skip = false;
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
bool ExplicitValidation::ValidateSurfacePresentModeEXT(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const VkPresentModeKHR presentMode) const {
    bool skip = false;
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
bool ExplicitValidation::ValidateSurfacePresentScalingCapabilitiesEXT(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const VkPresentScalingFlagsEXT supportedPresentScaling,
                const VkPresentGravityFlagsEXT supportedPresentGravityX,
                const VkPresentGravityFlagsEXT supportedPresentGravityY,
                const VkExtent2D minScaledImageExtent,
                const VkExtent2D maxScaledImageExtent) const {
    bool skip = false;
    const auto _s287 = &minScaledImageExtent;
    skip |= ValidateExtent2D(_parentObjects,
        _s287->width,
        _s287->height);
    const auto _s288 = &maxScaledImageExtent;
    skip |= ValidateExtent2D(_parentObjects,
        _s288->width,
        _s288->height);
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
bool ExplicitValidation::ValidateSurfacePresentModeCompatibilityEXT(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const uint32_t presentModeCount,
                const VkPresentModeKHR* pPresentModes) const {
    bool skip = false;
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
bool ExplicitValidation::ValidatePhysicalDeviceSwapchainMaintenance1FeaturesEXT(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const VkBool32 swapchainMaintenance1) const {
    bool skip = false;
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
bool ExplicitValidation::ValidateSwapchainPresentFenceInfoEXT(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const uint32_t swapchainCount,
                const VkFence* pFences) const {
    bool skip = false;
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
bool ExplicitValidation::ValidateSwapchainPresentModesCreateInfoEXT(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const uint32_t presentModeCount,
                const VkPresentModeKHR* pPresentModes) const {
    bool skip = false;
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
bool ExplicitValidation::ValidateSwapchainPresentModeInfoEXT(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const uint32_t swapchainCount,
                const VkPresentModeKHR* pPresentModes) const {
    bool skip = false;
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
bool ExplicitValidation::ValidateSwapchainPresentScalingCreateInfoEXT(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const VkPresentScalingFlagsEXT scalingBehavior,
                const VkPresentGravityFlagsEXT presentGravityX,
                const VkPresentGravityFlagsEXT presentGravityY) const {
    bool skip = false;
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
bool ExplicitValidation::ValidateReleaseSwapchainImagesInfoEXT(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const VkSwapchainKHR swapchain,
                const uint32_t imageIndexCount,
                const uint32_t* pImageIndices) const {
    bool skip = false;
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
bool ExplicitValidation::ValidatePhysicalDeviceShaderDemoteToHelperInvocationFeaturesEXT(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const VkBool32 shaderDemoteToHelperInvocation) const {
    return ValidatePhysicalDeviceShaderDemoteToHelperInvocationFeatures(_parentObjects, sType, pNext, shaderDemoteToHelperInvocation);
}
bool ExplicitValidation::ValidatePhysicalDeviceDeviceGeneratedCommandsFeaturesNV(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const VkBool32 deviceGeneratedCommands) const {
    bool skip = false;
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
bool ExplicitValidation::ValidateGraphicsShaderGroupCreateInfoNV(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const uint32_t stageCount,
                const VkPipelineShaderStageCreateInfo* pStages,
                const VkPipelineVertexInputStateCreateInfo* pVertexInputState,
                const VkPipelineTessellationStateCreateInfo* pTessellationState) const {
    bool skip = false;
    if (pStages != nullptr) {
        for (uint32_t _i289 = 0;_i289 < stageCount; ++_i289) {
            const auto _s290 = &pStages[_i289];
            skip |= ValidatePipelineShaderStageCreateInfo(_parentObjects,
                _s290->sType,
                _s290->pNext,
                _s290->flags,
                _s290->stage,
                _s290->module,
                _s290->pName,
                _s290->pSpecializationInfo);
        }
    }
    if (pVertexInputState != nullptr) {
        const auto _s291 = pVertexInputState;
        skip |= ValidatePipelineVertexInputStateCreateInfo(_parentObjects,
            _s291->sType,
            _s291->pNext,
            _s291->flags,
            _s291->vertexBindingDescriptionCount,
            _s291->pVertexBindingDescriptions,
            _s291->vertexAttributeDescriptionCount,
            _s291->pVertexAttributeDescriptions);
    }
    if (pTessellationState != nullptr) {
        const auto _s292 = pTessellationState;
        skip |= ValidatePipelineTessellationStateCreateInfo(_parentObjects,
            _s292->sType,
            _s292->pNext,
            _s292->flags,
            _s292->patchControlPoints);
    }
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
bool ExplicitValidation::ValidateGraphicsPipelineShaderGroupsCreateInfoNV(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const uint32_t groupCount,
                const VkGraphicsShaderGroupCreateInfoNV* pGroups,
                const uint32_t pipelineCount,
                const VkPipeline* pPipelines) const {
    bool skip = false;
    if (pGroups != nullptr) {
        for (uint32_t _i293 = 0;_i293 < groupCount; ++_i293) {
            const auto _s294 = &pGroups[_i293];
            skip |= ValidateGraphicsShaderGroupCreateInfoNV(_parentObjects,
                _s294->sType,
                _s294->pNext,
                _s294->stageCount,
                _s294->pStages,
                _s294->pVertexInputState,
                _s294->pTessellationState);
        }
    }
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
bool ExplicitValidation::ValidateBindShaderGroupIndirectCommandNV(const LogObjectList &_parentObjects,
                const uint32_t groupIndex) const {
    bool skip = false;
    return skip;
}
bool ExplicitValidation::ValidateBindIndexBufferIndirectCommandNV(const LogObjectList &_parentObjects,
                const VkDeviceAddress bufferAddress,
                const uint32_t size,
                const VkIndexType indexType) const {
    bool skip = false;
    return skip;
}
bool ExplicitValidation::ValidateBindVertexBufferIndirectCommandNV(const LogObjectList &_parentObjects,
                const VkDeviceAddress bufferAddress,
                const uint32_t size,
                const uint32_t stride) const {
    bool skip = false;
    return skip;
}
bool ExplicitValidation::ValidateSetStateFlagsIndirectCommandNV(const LogObjectList &_parentObjects,
                const uint32_t data) const {
    bool skip = false;
    return skip;
}
bool ExplicitValidation::ValidateIndirectCommandsStreamNV(const LogObjectList &_parentObjects,
                const VkBuffer buffer,
                const VkDeviceSize offset) const {
    bool skip = false;
    return skip;
}
bool ExplicitValidation::ValidateIndirectCommandsLayoutTokenNV(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const VkIndirectCommandsTokenTypeNV tokenType,
                const uint32_t stream,
                const uint32_t offset,
                const uint32_t vertexBindingUnit,
                const VkBool32 vertexDynamicStride,
                VkPipelineLayout pushconstantPipelineLayout,
                VkShaderStageFlags pushconstantShaderStageFlags,
                uint32_t pushconstantOffset,
                uint32_t pushconstantSize,
                const VkIndirectStateFlagsNV indirectStateFlags,
                const uint32_t indexTypeCount,
                const VkIndexType* pIndexTypes,
                const uint32_t* pIndexTypeValues) const {
    bool skip = false;
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
bool ExplicitValidation::ValidateIndirectCommandsLayoutCreateInfoNV(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const VkIndirectCommandsLayoutUsageFlagsNV flags,
                const VkPipelineBindPoint pipelineBindPoint,
                const uint32_t tokenCount,
                const VkIndirectCommandsLayoutTokenNV* pTokens,
                const uint32_t streamCount,
                const uint32_t* pStreamStrides) const {
    bool skip = false;
    if (pTokens != nullptr) {
        for (uint32_t _i295 = 0;_i295 < tokenCount; ++_i295) {
            const auto _s296 = &pTokens[_i295];
            skip |= ValidateIndirectCommandsLayoutTokenNV(_parentObjects,
                _s296->sType,
                _s296->pNext,
                _s296->tokenType,
                _s296->stream,
                _s296->offset,
                _s296->vertexBindingUnit,
                _s296->vertexDynamicStride,
                _s296->pushconstantPipelineLayout,
                _s296->pushconstantShaderStageFlags,
                _s296->pushconstantOffset,
                _s296->pushconstantSize,
                _s296->indirectStateFlags,
                _s296->indexTypeCount,
                _s296->pIndexTypes,
                _s296->pIndexTypeValues);
        }
    }
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
bool ExplicitValidation::ValidateGeneratedCommandsInfoNV(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const VkPipelineBindPoint pipelineBindPoint,
                const VkPipeline pipeline,
                const VkIndirectCommandsLayoutNV indirectCommandsLayout,
                const uint32_t streamCount,
                const VkIndirectCommandsStreamNV* pStreams,
                const uint32_t sequencesCount,
                const VkBuffer preprocessBuffer,
                const VkDeviceSize preprocessOffset,
                const VkDeviceSize preprocessSize,
                const VkBuffer sequencesCountBuffer,
                const VkDeviceSize sequencesCountOffset,
                const VkBuffer sequencesIndexBuffer,
                const VkDeviceSize sequencesIndexOffset) const {
    bool skip = false;
    if (pStreams != nullptr) {
        for (uint32_t _i297 = 0;_i297 < streamCount; ++_i297) {
            const auto _s298 = &pStreams[_i297];
            skip |= ValidateIndirectCommandsStreamNV(_parentObjects,
                _s298->buffer,
                _s298->offset);
        }
    }
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
bool ExplicitValidation::ValidateGeneratedCommandsMemoryRequirementsInfoNV(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const VkPipelineBindPoint pipelineBindPoint,
                const VkPipeline pipeline,
                const VkIndirectCommandsLayoutNV indirectCommandsLayout,
                const uint32_t maxSequencesCount) const {
    bool skip = false;
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
bool ExplicitValidation::ValidatePhysicalDeviceInheritedViewportScissorFeaturesNV(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const VkBool32 inheritedViewportScissor2D) const {
    bool skip = false;
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
bool ExplicitValidation::ValidateCommandBufferInheritanceViewportScissorInfoNV(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const VkBool32 viewportScissor2D,
                const uint32_t viewportDepthCount,
                const VkViewport* pViewportDepths) const {
    bool skip = false;
    if (pViewportDepths != nullptr) {
        const auto _s299 = pViewportDepths;
        skip |= ValidateViewport(_parentObjects,
            _s299->x,
            _s299->y,
            _s299->width,
            _s299->height,
            _s299->minDepth,
            _s299->maxDepth);
    }
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
bool ExplicitValidation::ValidatePhysicalDeviceTexelBufferAlignmentFeaturesEXT(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const VkBool32 texelBufferAlignment) const {
    bool skip = false;
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
bool ExplicitValidation::ValidateRenderPassTransformBeginInfoQCOM(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const VkSurfaceTransformFlagBitsKHR transform) const {
    bool skip = false;
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
bool ExplicitValidation::ValidateCommandBufferInheritanceRenderPassTransformInfoQCOM(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const VkSurfaceTransformFlagBitsKHR transform,
                const VkRect2D renderArea) const {
    bool skip = false;
    const auto _s300 = &renderArea;
    skip |= ValidateRect2D(_parentObjects,
        _s300->offset,
        _s300->extent);
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
bool ExplicitValidation::ValidatePhysicalDeviceDeviceMemoryReportFeaturesEXT(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const VkBool32 deviceMemoryReport) const {
    bool skip = false;
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
bool ExplicitValidation::ValidateDeviceDeviceMemoryReportCreateInfoEXT(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const VkDeviceMemoryReportFlagsEXT flags,
                const PFN_vkDeviceMemoryReportCallbackEXT pfnUserCallback,
                const void* pUserData) const {
    bool skip = false;
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
bool ExplicitValidation::ValidatePhysicalDeviceRobustness2FeaturesEXT(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const VkBool32 robustBufferAccess2,
                const VkBool32 robustImageAccess2,
                const VkBool32 nullDescriptor) const {
    bool skip = false;
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
bool ExplicitValidation::ValidateSamplerCustomBorderColorCreateInfoEXT(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const VkClearColorValue customBorderColor,
                const VkFormat format) const {
    bool skip = false;
    const auto _s301 = &customBorderColor;
    skip |= ValidateClearColorValue(_parentObjects,
        _s301->float32,
        _s301->int32,
        _s301->uint32);
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
bool ExplicitValidation::ValidatePhysicalDeviceCustomBorderColorFeaturesEXT(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const VkBool32 customBorderColors,
                const VkBool32 customBorderColorWithoutFormat) const {
    bool skip = false;
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
bool ExplicitValidation::ValidatePhysicalDevicePresentBarrierFeaturesNV(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const VkBool32 presentBarrier) const {
    bool skip = false;
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
bool ExplicitValidation::ValidateSurfaceCapabilitiesPresentBarrierNV(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const VkBool32 presentBarrierSupported) const {
    bool skip = false;
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
bool ExplicitValidation::ValidateSwapchainPresentBarrierCreateInfoNV(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const VkBool32 presentBarrierEnable) const {
    bool skip = false;
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
bool ExplicitValidation::ValidatePhysicalDevicePrivateDataFeaturesEXT(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const VkBool32 privateData) const {
    return ValidatePhysicalDevicePrivateDataFeatures(_parentObjects, sType, pNext, privateData);
}
bool ExplicitValidation::ValidateDevicePrivateDataCreateInfoEXT(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const uint32_t privateDataSlotRequestCount) const {
    return ValidateDevicePrivateDataCreateInfo(_parentObjects, sType, pNext, privateDataSlotRequestCount);
}
bool ExplicitValidation::ValidatePrivateDataSlotCreateInfoEXT(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const VkPrivateDataSlotCreateFlags flags) const {
    return ValidatePrivateDataSlotCreateInfo(_parentObjects, sType, pNext, flags);
}
bool ExplicitValidation::ValidatePhysicalDevicePipelineCreationCacheControlFeaturesEXT(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const VkBool32 pipelineCreationCacheControl) const {
    return ValidatePhysicalDevicePipelineCreationCacheControlFeatures(_parentObjects, sType, pNext, pipelineCreationCacheControl);
}
bool ExplicitValidation::ValidatePhysicalDeviceDiagnosticsConfigFeaturesNV(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const VkBool32 diagnosticsConfig) const {
    bool skip = false;
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
bool ExplicitValidation::ValidateDeviceDiagnosticsConfigCreateInfoNV(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const VkDeviceDiagnosticsConfigFlagsNV flags) const {
    bool skip = false;
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
bool ExplicitValidation::ValidateQueryLowLatencySupportNV(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const void* pQueriedLowLatencyData) const {
    bool skip = false;
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
#ifdef VK_USE_PLATFORM_METAL_EXT
bool ExplicitValidation::ValidateExportMetalObjectCreateInfoEXT(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const VkExportMetalObjectTypeFlagBitsEXT exportObjectType) const {
    bool skip = false;
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
#endif
#ifdef VK_USE_PLATFORM_METAL_EXT
bool ExplicitValidation::ValidateExportMetalObjectsInfoEXT(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext) const {
    bool skip = false;
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
#endif
#ifdef VK_USE_PLATFORM_METAL_EXT
bool ExplicitValidation::ValidateExportMetalDeviceInfoEXT(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const MTLDevice_id mtlDevice) const {
    bool skip = false;
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
#endif
#ifdef VK_USE_PLATFORM_METAL_EXT
bool ExplicitValidation::ValidateExportMetalCommandQueueInfoEXT(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const VkQueue queue,
                const MTLCommandQueue_id mtlCommandQueue) const {
    bool skip = false;
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
#endif
#ifdef VK_USE_PLATFORM_METAL_EXT
bool ExplicitValidation::ValidateExportMetalBufferInfoEXT(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const VkDeviceMemory memory,
                const MTLBuffer_id mtlBuffer) const {
    bool skip = false;
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
#endif
#ifdef VK_USE_PLATFORM_METAL_EXT
bool ExplicitValidation::ValidateImportMetalBufferInfoEXT(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const MTLBuffer_id mtlBuffer) const {
    bool skip = false;
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
#endif
#ifdef VK_USE_PLATFORM_METAL_EXT
bool ExplicitValidation::ValidateExportMetalTextureInfoEXT(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const VkImage image,
                const VkImageView imageView,
                const VkBufferView bufferView,
                const VkImageAspectFlagBits plane,
                const MTLTexture_id mtlTexture) const {
    bool skip = false;
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
#endif
#ifdef VK_USE_PLATFORM_METAL_EXT
bool ExplicitValidation::ValidateImportMetalTextureInfoEXT(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const VkImageAspectFlagBits plane,
                const MTLTexture_id mtlTexture) const {
    bool skip = false;
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
#endif
#ifdef VK_USE_PLATFORM_METAL_EXT
bool ExplicitValidation::ValidateExportMetalIOSurfaceInfoEXT(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const VkImage image,
                const IOSurfaceRef ioSurface) const {
    bool skip = false;
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
#endif
#ifdef VK_USE_PLATFORM_METAL_EXT
bool ExplicitValidation::ValidateImportMetalIOSurfaceInfoEXT(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const IOSurfaceRef ioSurface) const {
    bool skip = false;
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
#endif
#ifdef VK_USE_PLATFORM_METAL_EXT
bool ExplicitValidation::ValidateExportMetalSharedEventInfoEXT(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const VkSemaphore semaphore,
                const VkEvent event,
                const MTLSharedEvent_id mtlSharedEvent) const {
    bool skip = false;
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
#endif
#ifdef VK_USE_PLATFORM_METAL_EXT
bool ExplicitValidation::ValidateImportMetalSharedEventInfoEXT(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const MTLSharedEvent_id mtlSharedEvent) const {
    bool skip = false;
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
#endif
bool ExplicitValidation::ValidatePhysicalDeviceDescriptorBufferFeaturesEXT(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const VkBool32 descriptorBuffer,
                const VkBool32 descriptorBufferCaptureReplay,
                const VkBool32 descriptorBufferImageLayoutIgnored,
                const VkBool32 descriptorBufferPushDescriptors) const {
    bool skip = false;
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
bool ExplicitValidation::ValidateDescriptorAddressInfoEXT(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const VkDeviceAddress address,
                const VkDeviceSize range,
                const VkFormat format) const {
    bool skip = false;
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
bool ExplicitValidation::ValidateDescriptorBufferBindingInfoEXT(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const VkDeviceAddress address,
                const VkBufferUsageFlags usage) const {
    bool skip = false;
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
bool ExplicitValidation::ValidateDescriptorBufferBindingPushDescriptorBufferHandleEXT(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const VkBuffer buffer) const {
    bool skip = false;
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
bool ExplicitValidation::ValidateDescriptorDataEXT(const LogObjectList &_parentObjects,
                const VkSampler* pSampler,
                const VkDescriptorImageInfo* pCombinedImageSampler,
                const VkDescriptorImageInfo* pInputAttachmentImage,
                const VkDescriptorImageInfo* pSampledImage,
                const VkDescriptorImageInfo* pStorageImage,
                const VkDescriptorAddressInfoEXT* pUniformTexelBuffer,
                const VkDescriptorAddressInfoEXT* pStorageTexelBuffer,
                const VkDescriptorAddressInfoEXT* pUniformBuffer,
                const VkDescriptorAddressInfoEXT* pStorageBuffer,
                const VkDeviceAddress accelerationStructure) const {
    bool skip = false;
    if (pCombinedImageSampler != nullptr) {
        const auto _s302 = pCombinedImageSampler;
        skip |= ValidateDescriptorImageInfo(_parentObjects,
            _s302->sampler,
            _s302->imageView,
            _s302->imageLayout);
    }
    if (pInputAttachmentImage != nullptr) {
        const auto _s303 = pInputAttachmentImage;
        skip |= ValidateDescriptorImageInfo(_parentObjects,
            _s303->sampler,
            _s303->imageView,
            _s303->imageLayout);
    }
    if (pSampledImage != nullptr) {
        const auto _s304 = pSampledImage;
        skip |= ValidateDescriptorImageInfo(_parentObjects,
            _s304->sampler,
            _s304->imageView,
            _s304->imageLayout);
    }
    if (pStorageImage != nullptr) {
        const auto _s305 = pStorageImage;
        skip |= ValidateDescriptorImageInfo(_parentObjects,
            _s305->sampler,
            _s305->imageView,
            _s305->imageLayout);
    }
    if (pUniformTexelBuffer != nullptr) {
        const auto _s306 = pUniformTexelBuffer;
        skip |= ValidateDescriptorAddressInfoEXT(_parentObjects,
            _s306->sType,
            _s306->pNext,
            _s306->address,
            _s306->range,
            _s306->format);
    }
    if (pStorageTexelBuffer != nullptr) {
        const auto _s307 = pStorageTexelBuffer;
        skip |= ValidateDescriptorAddressInfoEXT(_parentObjects,
            _s307->sType,
            _s307->pNext,
            _s307->address,
            _s307->range,
            _s307->format);
    }
    if (pUniformBuffer != nullptr) {
        const auto _s308 = pUniformBuffer;
        skip |= ValidateDescriptorAddressInfoEXT(_parentObjects,
            _s308->sType,
            _s308->pNext,
            _s308->address,
            _s308->range,
            _s308->format);
    }
    if (pStorageBuffer != nullptr) {
        const auto _s309 = pStorageBuffer;
        skip |= ValidateDescriptorAddressInfoEXT(_parentObjects,
            _s309->sType,
            _s309->pNext,
            _s309->address,
            _s309->range,
            _s309->format);
    }
    return skip;
}
bool ExplicitValidation::ValidateDescriptorGetInfoEXT(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const VkDescriptorType type,
                const VkDescriptorDataEXT data) const {
    bool skip = false;
    const auto _s310 = &data;
    skip |= ValidateDescriptorDataEXT(_parentObjects,
        _s310->pSampler,
        _s310->pCombinedImageSampler,
        _s310->pInputAttachmentImage,
        _s310->pSampledImage,
        _s310->pStorageImage,
        _s310->pUniformTexelBuffer,
        _s310->pStorageTexelBuffer,
        _s310->pUniformBuffer,
        _s310->pStorageBuffer,
        _s310->accelerationStructure);
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
bool ExplicitValidation::ValidateBufferCaptureDescriptorDataInfoEXT(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const VkBuffer buffer) const {
    bool skip = false;
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
bool ExplicitValidation::ValidateImageCaptureDescriptorDataInfoEXT(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const VkImage image) const {
    bool skip = false;
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
bool ExplicitValidation::ValidateImageViewCaptureDescriptorDataInfoEXT(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const VkImageView imageView) const {
    bool skip = false;
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
bool ExplicitValidation::ValidateSamplerCaptureDescriptorDataInfoEXT(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const VkSampler sampler) const {
    bool skip = false;
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
bool ExplicitValidation::ValidateOpaqueCaptureDescriptorDataCreateInfoEXT(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const void* opaqueCaptureDescriptorData) const {
    bool skip = false;
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
bool ExplicitValidation::ValidateAccelerationStructureCaptureDescriptorDataInfoEXT(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const VkAccelerationStructureKHR accelerationStructure,
                const VkAccelerationStructureNV accelerationStructureNV) const {
    bool skip = false;
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
bool ExplicitValidation::ValidatePhysicalDeviceGraphicsPipelineLibraryFeaturesEXT(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const VkBool32 graphicsPipelineLibrary) const {
    bool skip = false;
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
bool ExplicitValidation::ValidatePhysicalDeviceGraphicsPipelineLibraryPropertiesEXT(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const VkBool32 graphicsPipelineLibraryFastLinking,
                const VkBool32 graphicsPipelineLibraryIndependentInterpolationDecoration) const {
    bool skip = false;
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
bool ExplicitValidation::ValidateGraphicsPipelineLibraryCreateInfoEXT(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const VkGraphicsPipelineLibraryFlagsEXT flags) const {
    bool skip = false;
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
bool ExplicitValidation::ValidatePhysicalDeviceShaderEarlyAndLateFragmentTestsFeaturesAMD(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const VkBool32 shaderEarlyAndLateFragmentTests) const {
    bool skip = false;
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
bool ExplicitValidation::ValidatePhysicalDeviceFragmentShadingRateEnumsFeaturesNV(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const VkBool32 fragmentShadingRateEnums,
                const VkBool32 supersampleFragmentShadingRates,
                const VkBool32 noInvocationFragmentShadingRates) const {
    bool skip = false;
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
bool ExplicitValidation::ValidatePhysicalDeviceFragmentShadingRateEnumsPropertiesNV(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const VkSampleCountFlagBits maxFragmentShadingRateInvocationCount) const {
    bool skip = false;
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
bool ExplicitValidation::ValidatePipelineFragmentShadingRateEnumStateCreateInfoNV(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const VkFragmentShadingRateTypeNV shadingRateType,
                const VkFragmentShadingRateNV shadingRate,
                const VkFragmentShadingRateCombinerOpKHR combinerOps[2]) const {
    bool skip = false;
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
bool ExplicitValidation::ValidateDeviceOrHostAddressConstKHR(const LogObjectList &_parentObjects,
                const VkDeviceAddress deviceAddress,
                const void* hostAddress) const {
    bool skip = false;
    return skip;
}
bool ExplicitValidation::ValidateAccelerationStructureGeometryMotionTrianglesDataNV(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const VkDeviceOrHostAddressConstKHR vertexData) const {
    bool skip = false;
    const auto _s311 = &vertexData;
    skip |= ValidateDeviceOrHostAddressConstKHR(_parentObjects,
        _s311->deviceAddress,
        _s311->hostAddress);
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
bool ExplicitValidation::ValidateAccelerationStructureMotionInfoNV(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const uint32_t maxInstances,
                const VkAccelerationStructureMotionInfoFlagsNV flags) const {
    bool skip = false;
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
bool ExplicitValidation::ValidateAccelerationStructureMatrixMotionInstanceNV(const LogObjectList &_parentObjects,
                const VkTransformMatrixKHR transformT0,
                const VkTransformMatrixKHR transformT1,
                const uint32_t instanceCustomIndex,
                const uint32_t mask,
                const uint32_t instanceShaderBindingTableRecordOffset,
                const VkGeometryInstanceFlagsKHR flags,
                const uint64_t accelerationStructureReference) const {
    bool skip = false;
    const auto _s312 = &transformT0;
    skip |= ValidateTransformMatrixKHR(_parentObjects,
        _s312->matrix);
    const auto _s313 = &transformT1;
    skip |= ValidateTransformMatrixKHR(_parentObjects,
        _s313->matrix);
    return skip;
}
bool ExplicitValidation::ValidateSRTDataNV(const LogObjectList &_parentObjects,
                const float sx,
                const float a,
                const float b,
                const float pvx,
                const float sy,
                const float c,
                const float pvy,
                const float sz,
                const float pvz,
                const float qx,
                const float qy,
                const float qz,
                const float qw,
                const float tx,
                const float ty,
                const float tz) const {
    bool skip = false;
    return skip;
}
bool ExplicitValidation::ValidateAccelerationStructureSRTMotionInstanceNV(const LogObjectList &_parentObjects,
                const VkSRTDataNV transformT0,
                const VkSRTDataNV transformT1,
                const uint32_t instanceCustomIndex,
                const uint32_t mask,
                const uint32_t instanceShaderBindingTableRecordOffset,
                const VkGeometryInstanceFlagsKHR flags,
                const uint64_t accelerationStructureReference) const {
    bool skip = false;
    const auto _s314 = &transformT0;
    skip |= ValidateSRTDataNV(_parentObjects,
        _s314->sx,
        _s314->a,
        _s314->b,
        _s314->pvx,
        _s314->sy,
        _s314->c,
        _s314->pvy,
        _s314->sz,
        _s314->pvz,
        _s314->qx,
        _s314->qy,
        _s314->qz,
        _s314->qw,
        _s314->tx,
        _s314->ty,
        _s314->tz);
    const auto _s315 = &transformT1;
    skip |= ValidateSRTDataNV(_parentObjects,
        _s315->sx,
        _s315->a,
        _s315->b,
        _s315->pvx,
        _s315->sy,
        _s315->c,
        _s315->pvy,
        _s315->sz,
        _s315->pvz,
        _s315->qx,
        _s315->qy,
        _s315->qz,
        _s315->qw,
        _s315->tx,
        _s315->ty,
        _s315->tz);
    return skip;
}
bool ExplicitValidation::ValidateAccelerationStructureMotionInstanceDataNV(const LogObjectList &_parentObjects,
                const VkAccelerationStructureInstanceKHR staticInstance,
                const VkAccelerationStructureMatrixMotionInstanceNV matrixMotionInstance,
                const VkAccelerationStructureSRTMotionInstanceNV srtMotionInstance) const {
    bool skip = false;
    const auto _s316 = &staticInstance;
    skip |= ValidateAccelerationStructureInstanceKHR(_parentObjects,
        _s316->transform,
        _s316->instanceCustomIndex,
        _s316->mask,
        _s316->instanceShaderBindingTableRecordOffset,
        _s316->flags,
        _s316->accelerationStructureReference);
    const auto _s317 = &matrixMotionInstance;
    skip |= ValidateAccelerationStructureMatrixMotionInstanceNV(_parentObjects,
        _s317->transformT0,
        _s317->transformT1,
        _s317->instanceCustomIndex,
        _s317->mask,
        _s317->instanceShaderBindingTableRecordOffset,
        _s317->flags,
        _s317->accelerationStructureReference);
    const auto _s318 = &srtMotionInstance;
    skip |= ValidateAccelerationStructureSRTMotionInstanceNV(_parentObjects,
        _s318->transformT0,
        _s318->transformT1,
        _s318->instanceCustomIndex,
        _s318->mask,
        _s318->instanceShaderBindingTableRecordOffset,
        _s318->flags,
        _s318->accelerationStructureReference);
    return skip;
}
bool ExplicitValidation::ValidateAccelerationStructureMotionInstanceNV(const LogObjectList &_parentObjects,
                const VkAccelerationStructureMotionInstanceTypeNV type,
                const VkAccelerationStructureMotionInstanceFlagsNV flags,
                const VkAccelerationStructureMotionInstanceDataNV data) const {
    bool skip = false;
    const auto _s319 = &data;
    skip |= ValidateAccelerationStructureMotionInstanceDataNV(_parentObjects,
        _s319->staticInstance,
        _s319->matrixMotionInstance,
        _s319->srtMotionInstance);
    return skip;
}
bool ExplicitValidation::ValidatePhysicalDeviceRayTracingMotionBlurFeaturesNV(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const VkBool32 rayTracingMotionBlur,
                const VkBool32 rayTracingMotionBlurPipelineTraceRaysIndirect) const {
    bool skip = false;
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
bool ExplicitValidation::ValidatePhysicalDeviceYcbcr2Plane444FormatsFeaturesEXT(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const VkBool32 ycbcr2plane444Formats) const {
    bool skip = false;
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
bool ExplicitValidation::ValidatePhysicalDeviceFragmentDensityMap2FeaturesEXT(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const VkBool32 fragmentDensityMapDeferred) const {
    bool skip = false;
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
bool ExplicitValidation::ValidateCopyCommandTransformInfoQCOM(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const VkSurfaceTransformFlagBitsKHR transform) const {
    bool skip = false;
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
bool ExplicitValidation::ValidatePhysicalDeviceImageRobustnessFeaturesEXT(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const VkBool32 robustImageAccess) const {
    return ValidatePhysicalDeviceImageRobustnessFeatures(_parentObjects, sType, pNext, robustImageAccess);
}
bool ExplicitValidation::ValidatePhysicalDeviceImageCompressionControlFeaturesEXT(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const VkBool32 imageCompressionControl) const {
    bool skip = false;
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
bool ExplicitValidation::ValidateImageCompressionControlEXT(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const VkImageCompressionFlagsEXT flags,
                const uint32_t compressionControlPlaneCount,
                const VkImageCompressionFixedRateFlagsEXT* pFixedRateFlags) const {
    bool skip = false;
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
bool ExplicitValidation::ValidateImageSubresource2EXT(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const VkImageSubresource imageSubresource) const {
    bool skip = false;
    const auto _s320 = &imageSubresource;
    skip |= ValidateImageSubresource(_parentObjects,
        _s320->aspectMask,
        _s320->mipLevel,
        _s320->arrayLayer);
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
bool ExplicitValidation::ValidatePhysicalDeviceAttachmentFeedbackLoopLayoutFeaturesEXT(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const VkBool32 attachmentFeedbackLoopLayout) const {
    bool skip = false;
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
bool ExplicitValidation::ValidatePhysicalDevice4444FormatsFeaturesEXT(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const VkBool32 formatA4R4G4B4,
                const VkBool32 formatA4B4G4R4) const {
    bool skip = false;
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
bool ExplicitValidation::ValidatePhysicalDeviceFaultFeaturesEXT(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const VkBool32 deviceFault,
                const VkBool32 deviceFaultVendorBinary) const {
    bool skip = false;
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
bool ExplicitValidation::ValidateDeviceFaultCountsEXT(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const uint32_t addressInfoCount,
                const uint32_t vendorInfoCount,
                const VkDeviceSize vendorBinarySize) const {
    bool skip = false;
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
bool ExplicitValidation::ValidateDeviceFaultAddressInfoEXT(const LogObjectList &_parentObjects,
                const VkDeviceFaultAddressTypeEXT addressType,
                const VkDeviceAddress reportedAddress,
                const VkDeviceSize addressPrecision) const {
    bool skip = false;
    return skip;
}
bool ExplicitValidation::ValidateDeviceFaultVendorInfoEXT(const LogObjectList &_parentObjects,
                const char description[VK_MAX_DESCRIPTION_SIZE],
                const uint64_t vendorFaultCode,
                const uint64_t vendorFaultData) const {
    bool skip = false;
    return skip;
}
bool ExplicitValidation::ValidateDeviceFaultInfoEXT(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const char description[VK_MAX_DESCRIPTION_SIZE],
                const VkDeviceFaultAddressInfoEXT* pAddressInfos,
                const VkDeviceFaultVendorInfoEXT* pVendorInfos,
                const void* pVendorBinaryData) const {
    bool skip = false;
    if (pAddressInfos != nullptr) {
        const auto _s321 = pAddressInfos;
        skip |= ValidateDeviceFaultAddressInfoEXT(_parentObjects,
            _s321->addressType,
            _s321->reportedAddress,
            _s321->addressPrecision);
    }
    if (pVendorInfos != nullptr) {
        const auto _s322 = pVendorInfos;
        skip |= ValidateDeviceFaultVendorInfoEXT(_parentObjects,
            _s322->description,
            _s322->vendorFaultCode,
            _s322->vendorFaultData);
    }
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
bool ExplicitValidation::ValidateDeviceFaultVendorBinaryHeaderVersionOneEXT(const LogObjectList &_parentObjects,
                const uint32_t headerSize,
                const VkDeviceFaultVendorBinaryHeaderVersionEXT headerVersion,
                const uint32_t vendorID,
                const uint32_t deviceID,
                const uint32_t driverVersion,
                const uint8_t pipelineCacheUUID[VK_UUID_SIZE],
                const uint32_t applicationNameOffset,
                const uint32_t applicationVersion,
                const uint32_t engineNameOffset) const {
    bool skip = false;
    return skip;
}
bool ExplicitValidation::ValidatePhysicalDeviceRasterizationOrderAttachmentAccessFeaturesEXT(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const VkBool32 rasterizationOrderColorAttachmentAccess,
                const VkBool32 rasterizationOrderDepthAttachmentAccess,
                const VkBool32 rasterizationOrderStencilAttachmentAccess) const {
    bool skip = false;
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
bool ExplicitValidation::ValidatePhysicalDeviceRasterizationOrderAttachmentAccessFeaturesARM(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const VkBool32 rasterizationOrderColorAttachmentAccess,
                const VkBool32 rasterizationOrderDepthAttachmentAccess,
                const VkBool32 rasterizationOrderStencilAttachmentAccess) const {
    return ValidatePhysicalDeviceRasterizationOrderAttachmentAccessFeaturesEXT(_parentObjects, sType, pNext, rasterizationOrderColorAttachmentAccess, rasterizationOrderDepthAttachmentAccess, rasterizationOrderStencilAttachmentAccess);
}
bool ExplicitValidation::ValidatePhysicalDeviceRGBA10X6FormatsFeaturesEXT(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const VkBool32 formatRgba10x6WithoutYCbCrSampler) const {
    bool skip = false;
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
#ifdef VK_USE_PLATFORM_DIRECTFB_EXT
bool ExplicitValidation::ValidateDirectFBSurfaceCreateInfoEXT(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const VkDirectFBSurfaceCreateFlagsEXT flags,
                const IDirectFB* dfb,
                const IDirectFBSurface* surface) const {
    bool skip = false;
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
#endif
bool ExplicitValidation::ValidatePhysicalDeviceMutableDescriptorTypeFeaturesEXT(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const VkBool32 mutableDescriptorType) const {
    bool skip = false;
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
bool ExplicitValidation::ValidatePhysicalDeviceMutableDescriptorTypeFeaturesVALVE(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const VkBool32 mutableDescriptorType) const {
    return ValidatePhysicalDeviceMutableDescriptorTypeFeaturesEXT(_parentObjects, sType, pNext, mutableDescriptorType);
}
bool ExplicitValidation::ValidateMutableDescriptorTypeListEXT(const LogObjectList &_parentObjects,
                const uint32_t descriptorTypeCount,
                const VkDescriptorType* pDescriptorTypes) const {
    bool skip = false;
    return skip;
}
bool ExplicitValidation::ValidateMutableDescriptorTypeListVALVE(const LogObjectList &_parentObjects,
                const uint32_t descriptorTypeCount,
                const VkDescriptorType* pDescriptorTypes) const {
    return ValidateMutableDescriptorTypeListEXT(_parentObjects, descriptorTypeCount, pDescriptorTypes);
}
bool ExplicitValidation::ValidateMutableDescriptorTypeCreateInfoEXT(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const uint32_t mutableDescriptorTypeListCount,
                const VkMutableDescriptorTypeListEXT* pMutableDescriptorTypeLists) const {
    bool skip = false;
    if (pMutableDescriptorTypeLists != nullptr) {
        for (uint32_t _i323 = 0;_i323 < mutableDescriptorTypeListCount; ++_i323) {
            const auto _s324 = &pMutableDescriptorTypeLists[_i323];
            skip |= ValidateMutableDescriptorTypeListEXT(_parentObjects,
                _s324->descriptorTypeCount,
                _s324->pDescriptorTypes);
        }
    }
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
bool ExplicitValidation::ValidateMutableDescriptorTypeCreateInfoVALVE(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const uint32_t mutableDescriptorTypeListCount,
                const VkMutableDescriptorTypeListEXT* pMutableDescriptorTypeLists) const {
    return ValidateMutableDescriptorTypeCreateInfoEXT(_parentObjects, sType, pNext, mutableDescriptorTypeListCount, pMutableDescriptorTypeLists);
}
bool ExplicitValidation::ValidatePhysicalDeviceVertexInputDynamicStateFeaturesEXT(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const VkBool32 vertexInputDynamicState) const {
    bool skip = false;
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
bool ExplicitValidation::ValidateVertexInputBindingDescription2EXT(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const uint32_t binding,
                const uint32_t stride,
                const VkVertexInputRate inputRate,
                const uint32_t divisor) const {
    bool skip = false;
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
bool ExplicitValidation::ValidateVertexInputAttributeDescription2EXT(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const uint32_t location,
                const uint32_t binding,
                const VkFormat format,
                const uint32_t offset) const {
    bool skip = false;
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
bool ExplicitValidation::ValidatePhysicalDeviceAddressBindingReportFeaturesEXT(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const VkBool32 reportAddressBinding) const {
    bool skip = false;
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
bool ExplicitValidation::ValidateDeviceAddressBindingCallbackDataEXT(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const VkDeviceAddressBindingFlagsEXT flags,
                const VkDeviceAddress baseAddress,
                const VkDeviceSize size,
                const VkDeviceAddressBindingTypeEXT bindingType) const {
    bool skip = false;
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
bool ExplicitValidation::ValidatePhysicalDeviceDepthClipControlFeaturesEXT(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const VkBool32 depthClipControl) const {
    bool skip = false;
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
bool ExplicitValidation::ValidatePipelineViewportDepthClipControlCreateInfoEXT(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const VkBool32 negativeOneToOne) const {
    bool skip = false;
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
bool ExplicitValidation::ValidatePhysicalDevicePrimitiveTopologyListRestartFeaturesEXT(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const VkBool32 primitiveTopologyListRestart,
                const VkBool32 primitiveTopologyPatchListRestart) const {
    bool skip = false;
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
#ifdef VK_USE_PLATFORM_FUCHSIA
bool ExplicitValidation::ValidateImportMemoryZirconHandleInfoFUCHSIA(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const VkExternalMemoryHandleTypeFlagBits handleType,
                const zx_handle_t handle) const {
    bool skip = false;
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
#endif
#ifdef VK_USE_PLATFORM_FUCHSIA
bool ExplicitValidation::ValidateMemoryGetZirconHandleInfoFUCHSIA(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const VkDeviceMemory memory,
                const VkExternalMemoryHandleTypeFlagBits handleType) const {
    bool skip = false;
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
#endif
#ifdef VK_USE_PLATFORM_FUCHSIA
bool ExplicitValidation::ValidateImportSemaphoreZirconHandleInfoFUCHSIA(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const VkSemaphore semaphore,
                const VkSemaphoreImportFlags flags,
                const VkExternalSemaphoreHandleTypeFlagBits handleType,
                const zx_handle_t zirconHandle) const {
    bool skip = false;
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
#endif
#ifdef VK_USE_PLATFORM_FUCHSIA
bool ExplicitValidation::ValidateSemaphoreGetZirconHandleInfoFUCHSIA(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const VkSemaphore semaphore,
                const VkExternalSemaphoreHandleTypeFlagBits handleType) const {
    bool skip = false;
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
#endif
#ifdef VK_USE_PLATFORM_FUCHSIA
bool ExplicitValidation::ValidateBufferCollectionCreateInfoFUCHSIA(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const zx_handle_t collectionToken) const {
    bool skip = false;
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
#endif
#ifdef VK_USE_PLATFORM_FUCHSIA
bool ExplicitValidation::ValidateImportMemoryBufferCollectionFUCHSIA(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const VkBufferCollectionFUCHSIA collection,
                const uint32_t index) const {
    bool skip = false;
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
#endif
#ifdef VK_USE_PLATFORM_FUCHSIA
bool ExplicitValidation::ValidateBufferCollectionImageCreateInfoFUCHSIA(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const VkBufferCollectionFUCHSIA collection,
                const uint32_t index) const {
    bool skip = false;
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
#endif
#ifdef VK_USE_PLATFORM_FUCHSIA
bool ExplicitValidation::ValidateBufferCollectionConstraintsInfoFUCHSIA(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const uint32_t minBufferCount,
                const uint32_t maxBufferCount,
                const uint32_t minBufferCountForCamping,
                const uint32_t minBufferCountForDedicatedSlack,
                const uint32_t minBufferCountForSharedSlack) const {
    bool skip = false;
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
#endif
#ifdef VK_USE_PLATFORM_FUCHSIA
bool ExplicitValidation::ValidateBufferConstraintsInfoFUCHSIA(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const VkBufferCreateInfo createInfo,
                const VkFormatFeatureFlags requiredFormatFeatures,
                const VkBufferCollectionConstraintsInfoFUCHSIA bufferCollectionConstraints) const {
    bool skip = false;
    const auto _s325 = &createInfo;
    skip |= ValidateBufferCreateInfo(_parentObjects,
        _s325->sType,
        _s325->pNext,
        _s325->flags,
        _s325->size,
        _s325->usage,
        _s325->sharingMode,
        _s325->queueFamilyIndexCount,
        _s325->pQueueFamilyIndices);
    const auto _s326 = &bufferCollectionConstraints;
    skip |= ValidateBufferCollectionConstraintsInfoFUCHSIA(_parentObjects,
        _s326->sType,
        _s326->pNext,
        _s326->minBufferCount,
        _s326->maxBufferCount,
        _s326->minBufferCountForCamping,
        _s326->minBufferCountForDedicatedSlack,
        _s326->minBufferCountForSharedSlack);
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
#endif
#ifdef VK_USE_PLATFORM_FUCHSIA
bool ExplicitValidation::ValidateBufferCollectionBufferCreateInfoFUCHSIA(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const VkBufferCollectionFUCHSIA collection,
                const uint32_t index) const {
    bool skip = false;
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
#endif
#ifdef VK_USE_PLATFORM_FUCHSIA
bool ExplicitValidation::ValidateSysmemColorSpaceFUCHSIA(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const uint32_t colorSpace) const {
    bool skip = false;
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
#endif
#ifdef VK_USE_PLATFORM_FUCHSIA
bool ExplicitValidation::ValidateBufferCollectionPropertiesFUCHSIA(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const uint32_t memoryTypeBits,
                const uint32_t bufferCount,
                const uint32_t createInfoIndex,
                const uint64_t sysmemPixelFormat,
                const VkFormatFeatureFlags formatFeatures,
                const VkSysmemColorSpaceFUCHSIA sysmemColorSpaceIndex,
                const VkComponentMapping samplerYcbcrConversionComponents,
                const VkSamplerYcbcrModelConversion suggestedYcbcrModel,
                const VkSamplerYcbcrRange suggestedYcbcrRange,
                const VkChromaLocation suggestedXChromaOffset,
                const VkChromaLocation suggestedYChromaOffset) const {
    bool skip = false;
    const auto _s327 = &sysmemColorSpaceIndex;
    skip |= ValidateSysmemColorSpaceFUCHSIA(_parentObjects,
        _s327->sType,
        _s327->pNext,
        _s327->colorSpace);
    const auto _s328 = &samplerYcbcrConversionComponents;
    skip |= ValidateComponentMapping(_parentObjects,
        _s328->r,
        _s328->g,
        _s328->b,
        _s328->a);
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
#endif
#ifdef VK_USE_PLATFORM_FUCHSIA
bool ExplicitValidation::ValidateImageFormatConstraintsInfoFUCHSIA(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const VkImageCreateInfo imageCreateInfo,
                const VkFormatFeatureFlags requiredFormatFeatures,
                const VkImageFormatConstraintsFlagsFUCHSIA flags,
                const uint64_t sysmemPixelFormat,
                const uint32_t colorSpaceCount,
                const VkSysmemColorSpaceFUCHSIA* pColorSpaces) const {
    bool skip = false;
    const auto _s329 = &imageCreateInfo;
    skip |= ValidateImageCreateInfo(_parentObjects,
        _s329->sType,
        _s329->pNext,
        _s329->flags,
        _s329->imageType,
        _s329->format,
        _s329->extent,
        _s329->mipLevels,
        _s329->arrayLayers,
        _s329->samples,
        _s329->tiling,
        _s329->usage,
        _s329->sharingMode,
        _s329->queueFamilyIndexCount,
        _s329->pQueueFamilyIndices,
        _s329->initialLayout);
    if (pColorSpaces != nullptr) {
        for (uint32_t _i330 = 0;_i330 < colorSpaceCount; ++_i330) {
            const auto _s331 = &pColorSpaces[_i330];
            skip |= ValidateSysmemColorSpaceFUCHSIA(_parentObjects,
                _s331->sType,
                _s331->pNext,
                _s331->colorSpace);
        }
    }
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
#endif
#ifdef VK_USE_PLATFORM_FUCHSIA
bool ExplicitValidation::ValidateImageConstraintsInfoFUCHSIA(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const uint32_t formatConstraintsCount,
                const VkImageFormatConstraintsInfoFUCHSIA* pFormatConstraints,
                const VkBufferCollectionConstraintsInfoFUCHSIA bufferCollectionConstraints,
                const VkImageConstraintsInfoFlagsFUCHSIA flags) const {
    bool skip = false;
    if (pFormatConstraints != nullptr) {
        for (uint32_t _i332 = 0;_i332 < formatConstraintsCount; ++_i332) {
            const auto _s333 = &pFormatConstraints[_i332];
            skip |= ValidateImageFormatConstraintsInfoFUCHSIA(_parentObjects,
                _s333->sType,
                _s333->pNext,
                _s333->imageCreateInfo,
                _s333->requiredFormatFeatures,
                _s333->flags,
                _s333->sysmemPixelFormat,
                _s333->colorSpaceCount,
                _s333->pColorSpaces);
        }
    }
    const auto _s334 = &bufferCollectionConstraints;
    skip |= ValidateBufferCollectionConstraintsInfoFUCHSIA(_parentObjects,
        _s334->sType,
        _s334->pNext,
        _s334->minBufferCount,
        _s334->maxBufferCount,
        _s334->minBufferCountForCamping,
        _s334->minBufferCountForDedicatedSlack,
        _s334->minBufferCountForSharedSlack);
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
#endif
bool ExplicitValidation::ValidatePhysicalDeviceSubpassShadingFeaturesHUAWEI(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const VkBool32 subpassShading) const {
    bool skip = false;
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
bool ExplicitValidation::ValidatePhysicalDeviceInvocationMaskFeaturesHUAWEI(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const VkBool32 invocationMask) const {
    bool skip = false;
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
bool ExplicitValidation::ValidateMemoryGetRemoteAddressInfoNV(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const VkDeviceMemory memory,
                const VkExternalMemoryHandleTypeFlagBits handleType) const {
    bool skip = false;
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
bool ExplicitValidation::ValidatePhysicalDeviceExternalMemoryRDMAFeaturesNV(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const VkBool32 externalMemoryRDMA) const {
    bool skip = false;
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
bool ExplicitValidation::ValidatePipelineInfoEXT(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const VkPipeline pipeline) const {
    return ValidatePipelineInfoKHR(_parentObjects, sType, pNext, pipeline);
}
bool ExplicitValidation::ValidatePipelinePropertiesIdentifierEXT(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const uint8_t pipelineIdentifier[VK_UUID_SIZE]) const {
    bool skip = false;
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
bool ExplicitValidation::ValidatePhysicalDevicePipelinePropertiesFeaturesEXT(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const VkBool32 pipelinePropertiesIdentifier) const {
    bool skip = false;
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
bool ExplicitValidation::ValidatePhysicalDeviceMultisampledRenderToSingleSampledFeaturesEXT(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const VkBool32 multisampledRenderToSingleSampled) const {
    bool skip = false;
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
bool ExplicitValidation::ValidateMultisampledRenderToSingleSampledInfoEXT(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const VkBool32 multisampledRenderToSingleSampledEnable,
                const VkSampleCountFlagBits rasterizationSamples) const {
    bool skip = false;
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
bool ExplicitValidation::ValidatePhysicalDeviceExtendedDynamicState2FeaturesEXT(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const VkBool32 extendedDynamicState2,
                const VkBool32 extendedDynamicState2LogicOp,
                const VkBool32 extendedDynamicState2PatchControlPoints) const {
    bool skip = false;
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
#ifdef VK_USE_PLATFORM_SCREEN_QNX
bool ExplicitValidation::ValidateScreenSurfaceCreateInfoQNX(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const VkScreenSurfaceCreateFlagsQNX flags,
                const struct _screen_context* context,
                const struct _screen_window* window) const {
    bool skip = false;
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
#endif
bool ExplicitValidation::ValidatePhysicalDeviceColorWriteEnableFeaturesEXT(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const VkBool32 colorWriteEnable) const {
    bool skip = false;
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
bool ExplicitValidation::ValidatePipelineColorWriteCreateInfoEXT(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const uint32_t attachmentCount,
                const VkBool32* pColorWriteEnables) const {
    bool skip = false;
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
bool ExplicitValidation::ValidatePhysicalDevicePrimitivesGeneratedQueryFeaturesEXT(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const VkBool32 primitivesGeneratedQuery,
                const VkBool32 primitivesGeneratedQueryWithRasterizerDiscard,
                const VkBool32 primitivesGeneratedQueryWithNonZeroStreams) const {
    bool skip = false;
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
bool ExplicitValidation::ValidatePhysicalDeviceGlobalPriorityQueryFeaturesEXT(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const VkBool32 globalPriorityQuery) const {
    return ValidatePhysicalDeviceGlobalPriorityQueryFeaturesKHR(_parentObjects, sType, pNext, globalPriorityQuery);
}
bool ExplicitValidation::ValidateQueueFamilyGlobalPriorityPropertiesEXT(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const uint32_t priorityCount,
                const VkQueueGlobalPriorityKHR priorities[VK_MAX_GLOBAL_PRIORITY_SIZE_KHR]) const {
    return ValidateQueueFamilyGlobalPriorityPropertiesKHR(_parentObjects, sType, pNext, priorityCount, priorities);
}
bool ExplicitValidation::ValidatePhysicalDeviceImageViewMinLodFeaturesEXT(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const VkBool32 minLod) const {
    bool skip = false;
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
bool ExplicitValidation::ValidateImageViewMinLodCreateInfoEXT(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const float minLod) const {
    bool skip = false;
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
bool ExplicitValidation::ValidatePhysicalDeviceMultiDrawFeaturesEXT(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const VkBool32 multiDraw) const {
    bool skip = false;
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
bool ExplicitValidation::ValidateMultiDrawInfoEXT(const LogObjectList &_parentObjects,
                const uint32_t firstVertex,
                const uint32_t vertexCount) const {
    bool skip = false;
    return skip;
}
bool ExplicitValidation::ValidateMultiDrawIndexedInfoEXT(const LogObjectList &_parentObjects,
                const uint32_t firstIndex,
                const uint32_t indexCount,
                const int32_t vertexOffset) const {
    bool skip = false;
    return skip;
}
bool ExplicitValidation::ValidatePhysicalDeviceImage2DViewOf3DFeaturesEXT(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const VkBool32 image2DViewOf3D,
                const VkBool32 sampler2DViewOf3D) const {
    bool skip = false;
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
bool ExplicitValidation::ValidateMicromapUsageEXT(const LogObjectList &_parentObjects,
                const uint32_t count,
                const uint32_t subdivisionLevel,
                const uint32_t format) const {
    bool skip = false;
    return skip;
}
bool ExplicitValidation::ValidateDeviceOrHostAddressKHR(const LogObjectList &_parentObjects,
                const VkDeviceAddress deviceAddress,
                const void* hostAddress) const {
    bool skip = false;
    return skip;
}
bool ExplicitValidation::ValidateMicromapBuildInfoEXT(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const VkMicromapTypeEXT type,
                const VkBuildMicromapFlagsEXT flags,
                const VkBuildMicromapModeEXT mode,
                const VkMicromapEXT dstMicromap,
                const uint32_t usageCountsCount,
                const VkMicromapUsageEXT* pUsageCounts,
                const VkMicromapUsageEXT* const* ppUsageCounts,
                const VkDeviceOrHostAddressConstKHR data,
                const VkDeviceOrHostAddressKHR scratchData,
                const VkDeviceOrHostAddressConstKHR triangleArray,
                const VkDeviceSize triangleArrayStride) const {
    bool skip = false;
    if (pUsageCounts != nullptr) {
        for (uint32_t _i335 = 0;_i335 < usageCountsCount; ++_i335) {
            const auto _s336 = &pUsageCounts[_i335];
            skip |= ValidateMicromapUsageEXT(_parentObjects,
                _s336->count,
                _s336->subdivisionLevel,
                _s336->format);
        }
    }
    if (ppUsageCounts != nullptr) {
        for (uint32_t _i337 = 0;_i337 < usageCountsCount; ++_i337) {
            const auto _s338 = ppUsageCounts[_i337];
            skip |= ValidateMicromapUsageEXT(_parentObjects,
                _s338->count,
                _s338->subdivisionLevel,
                _s338->format);
        }
    }
    const auto _s339 = &data;
    skip |= ValidateDeviceOrHostAddressConstKHR(_parentObjects,
        _s339->deviceAddress,
        _s339->hostAddress);
    const auto _s340 = &scratchData;
    skip |= ValidateDeviceOrHostAddressKHR(_parentObjects,
        _s340->deviceAddress,
        _s340->hostAddress);
    const auto _s341 = &triangleArray;
    skip |= ValidateDeviceOrHostAddressConstKHR(_parentObjects,
        _s341->deviceAddress,
        _s341->hostAddress);
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
bool ExplicitValidation::ValidateMicromapCreateInfoEXT(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const VkMicromapCreateFlagsEXT createFlags,
                const VkBuffer buffer,
                const VkDeviceSize offset,
                const VkDeviceSize size,
                const VkMicromapTypeEXT type,
                const VkDeviceAddress deviceAddress) const {
    bool skip = false;
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
bool ExplicitValidation::ValidatePhysicalDeviceOpacityMicromapFeaturesEXT(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const VkBool32 micromap,
                const VkBool32 micromapCaptureReplay,
                const VkBool32 micromapHostCommands) const {
    bool skip = false;
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
bool ExplicitValidation::ValidateMicromapVersionInfoEXT(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const uint8_t* pVersionData) const {
    bool skip = false;
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
bool ExplicitValidation::ValidateCopyMicromapToMemoryInfoEXT(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const VkMicromapEXT src,
                const VkDeviceOrHostAddressKHR dst,
                const VkCopyMicromapModeEXT mode) const {
    bool skip = false;
    const auto _s342 = &dst;
    skip |= ValidateDeviceOrHostAddressKHR(_parentObjects,
        _s342->deviceAddress,
        _s342->hostAddress);
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
bool ExplicitValidation::ValidateCopyMemoryToMicromapInfoEXT(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const VkDeviceOrHostAddressConstKHR src,
                const VkMicromapEXT dst,
                const VkCopyMicromapModeEXT mode) const {
    bool skip = false;
    const auto _s343 = &src;
    skip |= ValidateDeviceOrHostAddressConstKHR(_parentObjects,
        _s343->deviceAddress,
        _s343->hostAddress);
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
bool ExplicitValidation::ValidateCopyMicromapInfoEXT(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const VkMicromapEXT src,
                const VkMicromapEXT dst,
                const VkCopyMicromapModeEXT mode) const {
    bool skip = false;
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
bool ExplicitValidation::ValidateMicromapBuildSizesInfoEXT(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const VkDeviceSize micromapSize,
                const VkDeviceSize buildScratchSize,
                const VkBool32 discardable) const {
    bool skip = false;
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
bool ExplicitValidation::ValidateAccelerationStructureTrianglesOpacityMicromapEXT(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const VkIndexType indexType,
                const VkDeviceOrHostAddressConstKHR indexBuffer,
                const VkDeviceSize indexStride,
                const uint32_t baseTriangle,
                const uint32_t usageCountsCount,
                const VkMicromapUsageEXT* pUsageCounts,
                const VkMicromapUsageEXT* const* ppUsageCounts,
                const VkMicromapEXT micromap) const {
    bool skip = false;
    const auto _s344 = &indexBuffer;
    skip |= ValidateDeviceOrHostAddressConstKHR(_parentObjects,
        _s344->deviceAddress,
        _s344->hostAddress);
    if (pUsageCounts != nullptr) {
        for (uint32_t _i345 = 0;_i345 < usageCountsCount; ++_i345) {
            const auto _s346 = &pUsageCounts[_i345];
            skip |= ValidateMicromapUsageEXT(_parentObjects,
                _s346->count,
                _s346->subdivisionLevel,
                _s346->format);
        }
    }
    if (ppUsageCounts != nullptr) {
        for (uint32_t _i347 = 0;_i347 < usageCountsCount; ++_i347) {
            const auto _s348 = ppUsageCounts[_i347];
            skip |= ValidateMicromapUsageEXT(_parentObjects,
                _s348->count,
                _s348->subdivisionLevel,
                _s348->format);
        }
    }
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
bool ExplicitValidation::ValidateMicromapTriangleEXT(const LogObjectList &_parentObjects,
                const uint32_t dataOffset,
                const uint16_t subdivisionLevel,
                const uint16_t format) const {
    bool skip = false;
    return skip;
}
bool ExplicitValidation::ValidatePhysicalDeviceClusterCullingShaderFeaturesHUAWEI(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void*pNext,
                const VkBool32 clustercullingShader,
                const VkBool32 multiviewClusterCullingShader) const {
    bool skip = false;
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
bool ExplicitValidation::ValidatePhysicalDeviceBorderColorSwizzleFeaturesEXT(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const VkBool32 borderColorSwizzle,
                const VkBool32 borderColorSwizzleFromImage) const {
    bool skip = false;
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
bool ExplicitValidation::ValidateSamplerBorderColorComponentMappingCreateInfoEXT(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const VkComponentMapping components,
                const VkBool32 srgb) const {
    bool skip = false;
    const auto _s349 = &components;
    skip |= ValidateComponentMapping(_parentObjects,
        _s349->r,
        _s349->g,
        _s349->b,
        _s349->a);
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
bool ExplicitValidation::ValidatePhysicalDevicePageableDeviceLocalMemoryFeaturesEXT(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const VkBool32 pageableDeviceLocalMemory) const {
    bool skip = false;
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
bool ExplicitValidation::ValidatePhysicalDeviceImageSlicedViewOf3DFeaturesEXT(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const VkBool32 imageSlicedViewOf3D) const {
    bool skip = false;
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
bool ExplicitValidation::ValidateImageViewSlicedCreateInfoEXT(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const uint32_t sliceOffset,
                const uint32_t sliceCount) const {
    bool skip = false;
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
bool ExplicitValidation::ValidatePhysicalDeviceDescriptorSetHostMappingFeaturesVALVE(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const VkBool32 descriptorSetHostMapping) const {
    bool skip = false;
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
bool ExplicitValidation::ValidateDescriptorSetBindingReferenceVALVE(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const VkDescriptorSetLayout descriptorSetLayout,
                const uint32_t binding) const {
    bool skip = false;
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
bool ExplicitValidation::ValidateDescriptorSetLayoutHostMappingInfoVALVE(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const size_t descriptorOffset,
                const uint32_t descriptorSize) const {
    bool skip = false;
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
bool ExplicitValidation::ValidatePhysicalDeviceDepthClampZeroOneFeaturesEXT(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const VkBool32 depthClampZeroOne) const {
    bool skip = false;
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
bool ExplicitValidation::ValidatePhysicalDeviceNonSeamlessCubeMapFeaturesEXT(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const VkBool32 nonSeamlessCubeMap) const {
    bool skip = false;
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
bool ExplicitValidation::ValidatePhysicalDeviceFragmentDensityMapOffsetFeaturesQCOM(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const VkBool32 fragmentDensityMapOffset) const {
    bool skip = false;
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
bool ExplicitValidation::ValidateSubpassFragmentDensityMapOffsetEndInfoQCOM(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const uint32_t fragmentDensityOffsetCount,
                const VkOffset2D* pFragmentDensityOffsets) const {
    bool skip = false;
    if (pFragmentDensityOffsets != nullptr) {
        for (uint32_t _i350 = 0;_i350 < fragmentDensityOffsetCount; ++_i350) {
            const auto _s351 = &pFragmentDensityOffsets[_i350];
            skip |= ValidateOffset2D(_parentObjects,
                _s351->x,
                _s351->y);
        }
    }
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
bool ExplicitValidation::ValidateCopyMemoryIndirectCommandNV(const LogObjectList &_parentObjects,
                const VkDeviceAddress srcAddress,
                const VkDeviceAddress dstAddress,
                const VkDeviceSize size) const {
    bool skip = false;
    return skip;
}
bool ExplicitValidation::ValidateCopyMemoryToImageIndirectCommandNV(const LogObjectList &_parentObjects,
                const VkDeviceAddress srcAddress,
                const uint32_t bufferRowLength,
                const uint32_t bufferImageHeight,
                const VkImageSubresourceLayers imageSubresource,
                const VkOffset3D imageOffset,
                const VkExtent3D imageExtent) const {
    bool skip = false;
    const auto _s352 = &imageSubresource;
    skip |= ValidateImageSubresourceLayers(_parentObjects,
        _s352->aspectMask,
        _s352->mipLevel,
        _s352->baseArrayLayer,
        _s352->layerCount);
    const auto _s353 = &imageOffset;
    skip |= ValidateOffset3D(_parentObjects,
        _s353->x,
        _s353->y,
        _s353->z);
    const auto _s354 = &imageExtent;
    skip |= ValidateExtent3D(_parentObjects,
        _s354->width,
        _s354->height,
        _s354->depth);
    return skip;
}
bool ExplicitValidation::ValidatePhysicalDeviceCopyMemoryIndirectFeaturesNV(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const VkBool32 indirectCopy) const {
    bool skip = false;
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
bool ExplicitValidation::ValidateDecompressMemoryRegionNV(const LogObjectList &_parentObjects,
                const VkDeviceAddress srcAddress,
                const VkDeviceAddress dstAddress,
                const VkDeviceSize compressedSize,
                const VkDeviceSize decompressedSize,
                const VkMemoryDecompressionMethodFlagsNV decompressionMethod) const {
    bool skip = false;
    return skip;
}
bool ExplicitValidation::ValidatePhysicalDeviceMemoryDecompressionFeaturesNV(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const VkBool32 memoryDecompression) const {
    bool skip = false;
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
bool ExplicitValidation::ValidatePhysicalDeviceLinearColorAttachmentFeaturesNV(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const VkBool32 linearColorAttachment) const {
    bool skip = false;
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
bool ExplicitValidation::ValidatePhysicalDeviceImageCompressionControlSwapchainFeaturesEXT(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const VkBool32 imageCompressionControlSwapchain) const {
    bool skip = false;
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
bool ExplicitValidation::ValidateImageViewSampleWeightCreateInfoQCOM(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const VkOffset2D filterCenter,
                const VkExtent2D filterSize,
                const uint32_t numPhases) const {
    bool skip = false;
    const auto _s355 = &filterCenter;
    skip |= ValidateOffset2D(_parentObjects,
        _s355->x,
        _s355->y);
    const auto _s356 = &filterSize;
    skip |= ValidateExtent2D(_parentObjects,
        _s356->width,
        _s356->height);
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
bool ExplicitValidation::ValidatePhysicalDeviceImageProcessingFeaturesQCOM(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const VkBool32 textureSampleWeighted,
                const VkBool32 textureBoxFilter,
                const VkBool32 textureBlockMatch) const {
    bool skip = false;
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
bool ExplicitValidation::ValidatePhysicalDeviceExtendedDynamicState3FeaturesEXT(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const VkBool32 extendedDynamicState3TessellationDomainOrigin,
                const VkBool32 extendedDynamicState3DepthClampEnable,
                const VkBool32 extendedDynamicState3PolygonMode,
                const VkBool32 extendedDynamicState3RasterizationSamples,
                const VkBool32 extendedDynamicState3SampleMask,
                const VkBool32 extendedDynamicState3AlphaToCoverageEnable,
                const VkBool32 extendedDynamicState3AlphaToOneEnable,
                const VkBool32 extendedDynamicState3LogicOpEnable,
                const VkBool32 extendedDynamicState3ColorBlendEnable,
                const VkBool32 extendedDynamicState3ColorBlendEquation,
                const VkBool32 extendedDynamicState3ColorWriteMask,
                const VkBool32 extendedDynamicState3RasterizationStream,
                const VkBool32 extendedDynamicState3ConservativeRasterizationMode,
                const VkBool32 extendedDynamicState3ExtraPrimitiveOverestimationSize,
                const VkBool32 extendedDynamicState3DepthClipEnable,
                const VkBool32 extendedDynamicState3SampleLocationsEnable,
                const VkBool32 extendedDynamicState3ColorBlendAdvanced,
                const VkBool32 extendedDynamicState3ProvokingVertexMode,
                const VkBool32 extendedDynamicState3LineRasterizationMode,
                const VkBool32 extendedDynamicState3LineStippleEnable,
                const VkBool32 extendedDynamicState3DepthClipNegativeOneToOne,
                const VkBool32 extendedDynamicState3ViewportWScalingEnable,
                const VkBool32 extendedDynamicState3ViewportSwizzle,
                const VkBool32 extendedDynamicState3CoverageToColorEnable,
                const VkBool32 extendedDynamicState3CoverageToColorLocation,
                const VkBool32 extendedDynamicState3CoverageModulationMode,
                const VkBool32 extendedDynamicState3CoverageModulationTableEnable,
                const VkBool32 extendedDynamicState3CoverageModulationTable,
                const VkBool32 extendedDynamicState3CoverageReductionMode,
                const VkBool32 extendedDynamicState3RepresentativeFragmentTestEnable,
                const VkBool32 extendedDynamicState3ShadingRateImageEnable) const {
    bool skip = false;
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
bool ExplicitValidation::ValidatePhysicalDeviceExtendedDynamicState3PropertiesEXT(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const VkBool32 dynamicPrimitiveTopologyUnrestricted) const {
    bool skip = false;
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
bool ExplicitValidation::ValidateColorBlendEquationEXT(const LogObjectList &_parentObjects,
                const VkBlendFactor srcColorBlendFactor,
                const VkBlendFactor dstColorBlendFactor,
                const VkBlendOp colorBlendOp,
                const VkBlendFactor srcAlphaBlendFactor,
                const VkBlendFactor dstAlphaBlendFactor,
                const VkBlendOp alphaBlendOp) const {
    bool skip = false;
    return skip;
}
bool ExplicitValidation::ValidateColorBlendAdvancedEXT(const LogObjectList &_parentObjects,
                const VkBlendOp advancedBlendOp,
                const VkBool32 srcPremultiplied,
                const VkBool32 dstPremultiplied,
                const VkBlendOverlapEXT blendOverlap,
                const VkBool32 clampResults) const {
    bool skip = false;
    return skip;
}
bool ExplicitValidation::ValidatePhysicalDeviceSubpassMergeFeedbackFeaturesEXT(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const VkBool32 subpassMergeFeedback) const {
    bool skip = false;
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
bool ExplicitValidation::ValidateRenderPassCreationControlEXT(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const VkBool32 disallowMerging) const {
    bool skip = false;
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
bool ExplicitValidation::ValidateRenderPassCreationFeedbackCreateInfoEXT(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const VkRenderPassCreationFeedbackInfoEXT* pRenderPassFeedback) const {
    bool skip = false;
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
bool ExplicitValidation::ValidateRenderPassSubpassFeedbackCreateInfoEXT(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const VkRenderPassSubpassFeedbackInfoEXT* pSubpassFeedback) const {
    bool skip = false;
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
bool ExplicitValidation::ValidateDirectDriverLoadingInfoLUNARG(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const VkDirectDriverLoadingFlagsLUNARG flags,
                const PFN_vkGetInstanceProcAddrLUNARG pfnGetInstanceProcAddr) const {
    bool skip = false;
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
bool ExplicitValidation::ValidateDirectDriverLoadingListLUNARG(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const VkDirectDriverLoadingModeLUNARG mode,
                const uint32_t driverCount,
                const VkDirectDriverLoadingInfoLUNARG* pDrivers) const {
    bool skip = false;
    if (pDrivers != nullptr) {
        for (uint32_t _i357 = 0;_i357 < driverCount; ++_i357) {
            const auto _s358 = &pDrivers[_i357];
            skip |= ValidateDirectDriverLoadingInfoLUNARG(_parentObjects,
                _s358->sType,
                _s358->pNext,
                _s358->flags,
                _s358->pfnGetInstanceProcAddr);
        }
    }
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
bool ExplicitValidation::ValidatePhysicalDeviceShaderModuleIdentifierFeaturesEXT(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const VkBool32 shaderModuleIdentifier) const {
    bool skip = false;
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
bool ExplicitValidation::ValidatePipelineShaderStageModuleIdentifierCreateInfoEXT(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const uint32_t identifierSize,
                const uint8_t* pIdentifier) const {
    bool skip = false;
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
bool ExplicitValidation::ValidatePhysicalDeviceOpticalFlowFeaturesNV(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const VkBool32 opticalFlow) const {
    bool skip = false;
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
bool ExplicitValidation::ValidateOpticalFlowImageFormatInfoNV(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const VkOpticalFlowUsageFlagsNV usage) const {
    bool skip = false;
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
bool ExplicitValidation::ValidateOpticalFlowSessionCreateInfoNV(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const uint32_t width,
                const uint32_t height,
                const VkFormat imageFormat,
                const VkFormat flowVectorFormat,
                const VkFormat costFormat,
                const VkOpticalFlowGridSizeFlagsNV outputGridSize,
                const VkOpticalFlowGridSizeFlagsNV hintGridSize,
                const VkOpticalFlowPerformanceLevelNV performanceLevel,
                const VkOpticalFlowSessionCreateFlagsNV flags) const {
    bool skip = false;
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
bool ExplicitValidation::ValidateOpticalFlowSessionCreatePrivateDataInfoNV(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const uint32_t id,
                const uint32_t size,
                const void* pPrivateData) const {
    bool skip = false;
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
bool ExplicitValidation::ValidateOpticalFlowExecuteInfoNV(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const VkOpticalFlowExecuteFlagsNV flags,
                const uint32_t regionCount,
                const VkRect2D* pRegions) const {
    bool skip = false;
    if (pRegions != nullptr) {
        for (uint32_t _i359 = 0;_i359 < regionCount; ++_i359) {
            const auto _s360 = &pRegions[_i359];
            skip |= ValidateRect2D(_parentObjects,
                _s360->offset,
                _s360->extent);
        }
    }
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
bool ExplicitValidation::ValidatePhysicalDeviceLegacyDitheringFeaturesEXT(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const VkBool32 legacyDithering) const {
    bool skip = false;
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
bool ExplicitValidation::ValidatePhysicalDevicePipelineProtectedAccessFeaturesEXT(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const VkBool32 pipelineProtectedAccess) const {
    bool skip = false;
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
bool ExplicitValidation::ValidatePhysicalDeviceTilePropertiesFeaturesQCOM(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const VkBool32 tileProperties) const {
    bool skip = false;
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
bool ExplicitValidation::ValidateTilePropertiesQCOM(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const VkExtent3D tileSize,
                const VkExtent2D apronSize,
                const VkOffset2D origin) const {
    bool skip = false;
    const auto _s361 = &tileSize;
    skip |= ValidateExtent3D(_parentObjects,
        _s361->width,
        _s361->height,
        _s361->depth);
    const auto _s362 = &apronSize;
    skip |= ValidateExtent2D(_parentObjects,
        _s362->width,
        _s362->height);
    const auto _s363 = &origin;
    skip |= ValidateOffset2D(_parentObjects,
        _s363->x,
        _s363->y);
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
bool ExplicitValidation::ValidatePhysicalDeviceAmigoProfilingFeaturesSEC(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const VkBool32 amigoProfiling) const {
    bool skip = false;
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
bool ExplicitValidation::ValidateAmigoProfilingSubmitInfoSEC(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const uint64_t firstDrawTimestamp,
                const uint64_t swapBufferTimestamp) const {
    bool skip = false;
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
bool ExplicitValidation::ValidatePhysicalDeviceMultiviewPerViewViewportsFeaturesQCOM(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const VkBool32 multiviewPerViewViewports) const {
    bool skip = false;
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
bool ExplicitValidation::ValidatePhysicalDeviceRayTracingInvocationReorderFeaturesNV(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const VkBool32 rayTracingInvocationReorder) const {
    bool skip = false;
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
bool ExplicitValidation::ValidatePhysicalDeviceShaderCoreBuiltinsFeaturesARM(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const VkBool32 shaderCoreBuiltins) const {
    bool skip = false;
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
bool ExplicitValidation::ValidatePhysicalDevicePipelineLibraryGroupHandlesFeaturesEXT(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const VkBool32 pipelineLibraryGroupHandles) const {
    bool skip = false;
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
bool ExplicitValidation::ValidatePhysicalDeviceMultiviewPerViewRenderAreasFeaturesQCOM(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const VkBool32 multiviewPerViewRenderAreas) const {
    bool skip = false;
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
bool ExplicitValidation::ValidateMultiviewPerViewRenderAreasRenderPassBeginInfoQCOM(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const uint32_t perViewRenderAreaCount,
                const VkRect2D* pPerViewRenderAreas) const {
    bool skip = false;
    if (pPerViewRenderAreas != nullptr) {
        for (uint32_t _i364 = 0;_i364 < perViewRenderAreaCount; ++_i364) {
            const auto _s365 = &pPerViewRenderAreas[_i364];
            skip |= ValidateRect2D(_parentObjects,
                _s365->offset,
                _s365->extent);
        }
    }
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
bool ExplicitValidation::ValidateAccelerationStructureBuildRangeInfoKHR(const LogObjectList &_parentObjects,
                const uint32_t primitiveCount,
                const uint32_t primitiveOffset,
                const uint32_t firstVertex,
                const uint32_t transformOffset) const {
    bool skip = false;
    return skip;
}
bool ExplicitValidation::ValidateAccelerationStructureGeometryTrianglesDataKHR(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const VkFormat vertexFormat,
                const VkDeviceOrHostAddressConstKHR vertexData,
                const VkDeviceSize vertexStride,
                const uint32_t maxVertex,
                const VkIndexType indexType,
                const VkDeviceOrHostAddressConstKHR indexData,
                const VkDeviceOrHostAddressConstKHR transformData) const {
    bool skip = false;
    const auto _s366 = &vertexData;
    skip |= ValidateDeviceOrHostAddressConstKHR(_parentObjects,
        _s366->deviceAddress,
        _s366->hostAddress);
    const auto _s367 = &indexData;
    skip |= ValidateDeviceOrHostAddressConstKHR(_parentObjects,
        _s367->deviceAddress,
        _s367->hostAddress);
    const auto _s368 = &transformData;
    skip |= ValidateDeviceOrHostAddressConstKHR(_parentObjects,
        _s368->deviceAddress,
        _s368->hostAddress);
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
bool ExplicitValidation::ValidateAccelerationStructureGeometryAabbsDataKHR(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const VkDeviceOrHostAddressConstKHR data,
                const VkDeviceSize stride) const {
    bool skip = false;
    const auto _s369 = &data;
    skip |= ValidateDeviceOrHostAddressConstKHR(_parentObjects,
        _s369->deviceAddress,
        _s369->hostAddress);
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
bool ExplicitValidation::ValidateAccelerationStructureGeometryInstancesDataKHR(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const VkBool32 arrayOfPointers,
                const VkDeviceOrHostAddressConstKHR data) const {
    bool skip = false;
    const auto _s370 = &data;
    skip |= ValidateDeviceOrHostAddressConstKHR(_parentObjects,
        _s370->deviceAddress,
        _s370->hostAddress);
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
bool ExplicitValidation::ValidateAccelerationStructureGeometryDataKHR(const LogObjectList &_parentObjects,
                const VkAccelerationStructureGeometryTrianglesDataKHR triangles,
                const VkAccelerationStructureGeometryAabbsDataKHR aabbs,
                const VkAccelerationStructureGeometryInstancesDataKHR instances) const {
    bool skip = false;
    const auto _s371 = &triangles;
    skip |= ValidateAccelerationStructureGeometryTrianglesDataKHR(_parentObjects,
        _s371->sType,
        _s371->pNext,
        _s371->vertexFormat,
        _s371->vertexData,
        _s371->vertexStride,
        _s371->maxVertex,
        _s371->indexType,
        _s371->indexData,
        _s371->transformData);
    const auto _s372 = &aabbs;
    skip |= ValidateAccelerationStructureGeometryAabbsDataKHR(_parentObjects,
        _s372->sType,
        _s372->pNext,
        _s372->data,
        _s372->stride);
    const auto _s373 = &instances;
    skip |= ValidateAccelerationStructureGeometryInstancesDataKHR(_parentObjects,
        _s373->sType,
        _s373->pNext,
        _s373->arrayOfPointers,
        _s373->data);
    return skip;
}
bool ExplicitValidation::ValidateAccelerationStructureGeometryKHR(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const VkGeometryTypeKHR geometryType,
                const VkAccelerationStructureGeometryDataKHR geometry,
                const VkGeometryFlagsKHR flags) const {
    bool skip = false;
    const auto _s374 = &geometry;
    skip |= ValidateAccelerationStructureGeometryDataKHR(_parentObjects,
        _s374->triangles,
        _s374->aabbs,
        _s374->instances);
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
bool ExplicitValidation::ValidateAccelerationStructureBuildGeometryInfoKHR(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const VkAccelerationStructureTypeKHR type,
                const VkBuildAccelerationStructureFlagsKHR flags,
                const VkBuildAccelerationStructureModeKHR mode,
                const VkAccelerationStructureKHR srcAccelerationStructure,
                const VkAccelerationStructureKHR dstAccelerationStructure,
                const uint32_t geometryCount,
                const VkAccelerationStructureGeometryKHR* pGeometries,
                const VkAccelerationStructureGeometryKHR* const* ppGeometries,
                const VkDeviceOrHostAddressKHR scratchData) const {
    bool skip = false;
    if (pGeometries != nullptr) {
        for (uint32_t _i375 = 0;_i375 < geometryCount; ++_i375) {
            const auto _s376 = &pGeometries[_i375];
            skip |= ValidateAccelerationStructureGeometryKHR(_parentObjects,
                _s376->sType,
                _s376->pNext,
                _s376->geometryType,
                _s376->geometry,
                _s376->flags);
        }
    }
    if (ppGeometries != nullptr) {
        for (uint32_t _i377 = 0;_i377 < geometryCount; ++_i377) {
            const auto _s378 = ppGeometries[_i377];
            skip |= ValidateAccelerationStructureGeometryKHR(_parentObjects,
                _s378->sType,
                _s378->pNext,
                _s378->geometryType,
                _s378->geometry,
                _s378->flags);
        }
    }
    const auto _s379 = &scratchData;
    skip |= ValidateDeviceOrHostAddressKHR(_parentObjects,
        _s379->deviceAddress,
        _s379->hostAddress);
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
bool ExplicitValidation::ValidateAccelerationStructureCreateInfoKHR(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const VkAccelerationStructureCreateFlagsKHR createFlags,
                const VkBuffer buffer,
                const VkDeviceSize offset,
                const VkDeviceSize size,
                const VkAccelerationStructureTypeKHR type,
                const VkDeviceAddress deviceAddress) const {
    bool skip = false;
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
bool ExplicitValidation::ValidateWriteDescriptorSetAccelerationStructureKHR(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const uint32_t accelerationStructureCount,
                const VkAccelerationStructureKHR* pAccelerationStructures) const {
    bool skip = false;
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
bool ExplicitValidation::ValidatePhysicalDeviceAccelerationStructureFeaturesKHR(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const VkBool32 accelerationStructure,
                const VkBool32 accelerationStructureCaptureReplay,
                const VkBool32 accelerationStructureIndirectBuild,
                const VkBool32 accelerationStructureHostCommands,
                const VkBool32 descriptorBindingAccelerationStructureUpdateAfterBind) const {
    bool skip = false;
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
bool ExplicitValidation::ValidateAccelerationStructureDeviceAddressInfoKHR(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const VkAccelerationStructureKHR accelerationStructure) const {
    bool skip = false;
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
bool ExplicitValidation::ValidateAccelerationStructureVersionInfoKHR(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const uint8_t* pVersionData) const {
    bool skip = false;
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
bool ExplicitValidation::ValidateCopyAccelerationStructureToMemoryInfoKHR(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const VkAccelerationStructureKHR src,
                const VkDeviceOrHostAddressKHR dst,
                const VkCopyAccelerationStructureModeKHR mode) const {
    bool skip = false;
    const auto _s380 = &dst;
    skip |= ValidateDeviceOrHostAddressKHR(_parentObjects,
        _s380->deviceAddress,
        _s380->hostAddress);
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
bool ExplicitValidation::ValidateCopyMemoryToAccelerationStructureInfoKHR(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const VkDeviceOrHostAddressConstKHR src,
                const VkAccelerationStructureKHR dst,
                const VkCopyAccelerationStructureModeKHR mode) const {
    bool skip = false;
    const auto _s381 = &src;
    skip |= ValidateDeviceOrHostAddressConstKHR(_parentObjects,
        _s381->deviceAddress,
        _s381->hostAddress);
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
bool ExplicitValidation::ValidateCopyAccelerationStructureInfoKHR(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const VkAccelerationStructureKHR src,
                const VkAccelerationStructureKHR dst,
                const VkCopyAccelerationStructureModeKHR mode) const {
    bool skip = false;
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
bool ExplicitValidation::ValidateAccelerationStructureBuildSizesInfoKHR(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const VkDeviceSize accelerationStructureSize,
                const VkDeviceSize updateScratchSize,
                const VkDeviceSize buildScratchSize) const {
    bool skip = false;
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
bool ExplicitValidation::ValidateRayTracingShaderGroupCreateInfoKHR(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const VkRayTracingShaderGroupTypeKHR type,
                const uint32_t generalShader,
                const uint32_t closestHitShader,
                const uint32_t anyHitShader,
                const uint32_t intersectionShader,
                const void* pShaderGroupCaptureReplayHandle) const {
    bool skip = false;
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
bool ExplicitValidation::ValidateRayTracingPipelineInterfaceCreateInfoKHR(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const uint32_t maxPipelineRayPayloadSize,
                const uint32_t maxPipelineRayHitAttributeSize) const {
    bool skip = false;
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
bool ExplicitValidation::ValidateRayTracingPipelineCreateInfoKHR(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const VkPipelineCreateFlags flags,
                const uint32_t stageCount,
                const VkPipelineShaderStageCreateInfo* pStages,
                const uint32_t groupCount,
                const VkRayTracingShaderGroupCreateInfoKHR* pGroups,
                const uint32_t maxPipelineRayRecursionDepth,
                const VkPipelineLibraryCreateInfoKHR* pLibraryInfo,
                const VkRayTracingPipelineInterfaceCreateInfoKHR* pLibraryInterface,
                const VkPipelineDynamicStateCreateInfo* pDynamicState,
                const VkPipelineLayout layout,
                const VkPipeline basePipelineHandle,
                const int32_t basePipelineIndex) const {
    bool skip = false;
    if (pStages != nullptr) {
        for (uint32_t _i382 = 0;_i382 < stageCount; ++_i382) {
            const auto _s383 = &pStages[_i382];
            skip |= ValidatePipelineShaderStageCreateInfo(_parentObjects,
                _s383->sType,
                _s383->pNext,
                _s383->flags,
                _s383->stage,
                _s383->module,
                _s383->pName,
                _s383->pSpecializationInfo);
        }
    }
    if (pGroups != nullptr) {
        for (uint32_t _i384 = 0;_i384 < groupCount; ++_i384) {
            const auto _s385 = &pGroups[_i384];
            skip |= ValidateRayTracingShaderGroupCreateInfoKHR(_parentObjects,
                _s385->sType,
                _s385->pNext,
                _s385->type,
                _s385->generalShader,
                _s385->closestHitShader,
                _s385->anyHitShader,
                _s385->intersectionShader,
                _s385->pShaderGroupCaptureReplayHandle);
        }
    }
    if (pLibraryInfo != nullptr) {
        const auto _s386 = pLibraryInfo;
        skip |= ValidatePipelineLibraryCreateInfoKHR(_parentObjects,
            _s386->sType,
            _s386->pNext,
            _s386->libraryCount,
            _s386->pLibraries);
    }
    if (pLibraryInterface != nullptr) {
        const auto _s387 = pLibraryInterface;
        skip |= ValidateRayTracingPipelineInterfaceCreateInfoKHR(_parentObjects,
            _s387->sType,
            _s387->pNext,
            _s387->maxPipelineRayPayloadSize,
            _s387->maxPipelineRayHitAttributeSize);
    }
    if (pDynamicState != nullptr) {
        const auto _s388 = pDynamicState;
        skip |= ValidatePipelineDynamicStateCreateInfo(_parentObjects,
            _s388->sType,
            _s388->pNext,
            _s388->flags,
            _s388->dynamicStateCount,
            _s388->pDynamicStates);
    }
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
bool ExplicitValidation::ValidatePhysicalDeviceRayTracingPipelineFeaturesKHR(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const VkBool32 rayTracingPipeline,
                const VkBool32 rayTracingPipelineShaderGroupHandleCaptureReplay,
                const VkBool32 rayTracingPipelineShaderGroupHandleCaptureReplayMixed,
                const VkBool32 rayTracingPipelineTraceRaysIndirect,
                const VkBool32 rayTraversalPrimitiveCulling) const {
    bool skip = false;
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
bool ExplicitValidation::ValidateStridedDeviceAddressRegionKHR(const LogObjectList &_parentObjects,
                const VkDeviceAddress deviceAddress,
                const VkDeviceSize stride,
                const VkDeviceSize size) const {
    bool skip = false;
    return skip;
}
bool ExplicitValidation::ValidateTraceRaysIndirectCommandKHR(const LogObjectList &_parentObjects,
                const uint32_t width,
                const uint32_t height,
                const uint32_t depth) const {
    bool skip = false;
    return skip;
}
bool ExplicitValidation::ValidatePhysicalDeviceRayQueryFeaturesKHR(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const VkBool32 rayQuery) const {
    bool skip = false;
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
bool ExplicitValidation::ValidatePhysicalDeviceMeshShaderFeaturesEXT(const LogObjectList &_parentObjects,
                const VkStructureType sType,
                const void* pNext,
                const VkBool32 taskShader,
                const VkBool32 meshShader,
                const VkBool32 multiviewMeshShader,
                const VkBool32 primitiveFragmentShadingRateMeshShader,
                const VkBool32 meshShaderQueries) const {
    bool skip = false;
    skip |= ValidatePNext(_parentObjects, pNext);
    return skip;
}
bool ExplicitValidation::ValidateDrawMeshTasksIndirectCommandEXT(const LogObjectList &_parentObjects,
                const uint32_t groupCountX,
                const uint32_t groupCountY,
                const uint32_t groupCountZ) const {
    bool skip = false;
    return skip;
}
bool ExplicitValidation::ValidatePNext(const LogObjectList &_parentObjects, const void *pnext) const {
    bool skip = false;
    const VkBaseInStructure *header = reinterpret_cast<const VkBaseInStructure *>(pnext);
    while (header) {
        switch (header->sType) {
        case VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO: {
            const auto _s389 = reinterpret_cast<const VkShaderModuleCreateInfo *>(header);
            skip |= ValidateShaderModuleCreateInfo(_parentObjects,
                _s389->sType,
                _s389->pNext,
                _s389->flags,
                _s389->codeSize,
                _s389->pCode);
            break;
        }
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_16BIT_STORAGE_FEATURES: {
            const auto _s390 = reinterpret_cast<const VkPhysicalDevice16BitStorageFeatures *>(header);
            skip |= ValidatePhysicalDevice16BitStorageFeatures(_parentObjects,
                _s390->sType,
                _s390->pNext,
                _s390->storageBuffer16BitAccess,
                _s390->uniformAndStorageBuffer16BitAccess,
                _s390->storagePushConstant16,
                _s390->storageInputOutput16);
            break;
        }
        case VK_STRUCTURE_TYPE_MEMORY_DEDICATED_ALLOCATE_INFO: {
            const auto _s391 = reinterpret_cast<const VkMemoryDedicatedAllocateInfo *>(header);
            skip |= ValidateMemoryDedicatedAllocateInfo(_parentObjects,
                _s391->sType,
                _s391->pNext,
                _s391->image,
                _s391->buffer);
            break;
        }
        case VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_FLAGS_INFO: {
            const auto _s392 = reinterpret_cast<const VkMemoryAllocateFlagsInfo *>(header);
            skip |= ValidateMemoryAllocateFlagsInfo(_parentObjects,
                _s392->sType,
                _s392->pNext,
                _s392->flags,
                _s392->deviceMask);
            break;
        }
        case VK_STRUCTURE_TYPE_DEVICE_GROUP_RENDER_PASS_BEGIN_INFO: {
            const auto _s393 = reinterpret_cast<const VkDeviceGroupRenderPassBeginInfo *>(header);
            skip |= ValidateDeviceGroupRenderPassBeginInfo(_parentObjects,
                _s393->sType,
                _s393->pNext,
                _s393->deviceMask,
                _s393->deviceRenderAreaCount,
                _s393->pDeviceRenderAreas);
            break;
        }
        case VK_STRUCTURE_TYPE_DEVICE_GROUP_COMMAND_BUFFER_BEGIN_INFO: {
            const auto _s394 = reinterpret_cast<const VkDeviceGroupCommandBufferBeginInfo *>(header);
            skip |= ValidateDeviceGroupCommandBufferBeginInfo(_parentObjects,
                _s394->sType,
                _s394->pNext,
                _s394->deviceMask);
            break;
        }
        case VK_STRUCTURE_TYPE_DEVICE_GROUP_SUBMIT_INFO: {
            const auto _s395 = reinterpret_cast<const VkDeviceGroupSubmitInfo *>(header);
            skip |= ValidateDeviceGroupSubmitInfo(_parentObjects,
                _s395->sType,
                _s395->pNext,
                _s395->waitSemaphoreCount,
                _s395->pWaitSemaphoreDeviceIndices,
                _s395->commandBufferCount,
                _s395->pCommandBufferDeviceMasks,
                _s395->signalSemaphoreCount,
                _s395->pSignalSemaphoreDeviceIndices);
            break;
        }
        case VK_STRUCTURE_TYPE_DEVICE_GROUP_BIND_SPARSE_INFO: {
            const auto _s396 = reinterpret_cast<const VkDeviceGroupBindSparseInfo *>(header);
            skip |= ValidateDeviceGroupBindSparseInfo(_parentObjects,
                _s396->sType,
                _s396->pNext,
                _s396->resourceDeviceIndex,
                _s396->memoryDeviceIndex);
            break;
        }
        case VK_STRUCTURE_TYPE_BIND_BUFFER_MEMORY_DEVICE_GROUP_INFO: {
            const auto _s397 = reinterpret_cast<const VkBindBufferMemoryDeviceGroupInfo *>(header);
            skip |= ValidateBindBufferMemoryDeviceGroupInfo(_parentObjects,
                _s397->sType,
                _s397->pNext,
                _s397->deviceIndexCount,
                _s397->pDeviceIndices);
            break;
        }
        case VK_STRUCTURE_TYPE_BIND_IMAGE_MEMORY_DEVICE_GROUP_INFO: {
            const auto _s398 = reinterpret_cast<const VkBindImageMemoryDeviceGroupInfo *>(header);
            skip |= ValidateBindImageMemoryDeviceGroupInfo(_parentObjects,
                _s398->sType,
                _s398->pNext,
                _s398->deviceIndexCount,
                _s398->pDeviceIndices,
                _s398->splitInstanceBindRegionCount,
                _s398->pSplitInstanceBindRegions);
            break;
        }
        case VK_STRUCTURE_TYPE_DEVICE_GROUP_DEVICE_CREATE_INFO: {
            const auto _s399 = reinterpret_cast<const VkDeviceGroupDeviceCreateInfo *>(header);
            skip |= ValidateDeviceGroupDeviceCreateInfo(_parentObjects,
                _s399->sType,
                _s399->pNext,
                _s399->physicalDeviceCount,
                _s399->pPhysicalDevices);
            break;
        }
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2: {
            const auto _s400 = reinterpret_cast<const VkPhysicalDeviceFeatures2 *>(header);
            skip |= ValidatePhysicalDeviceFeatures2(_parentObjects,
                _s400->sType,
                _s400->pNext,
                _s400->features);
            break;
        }
        case VK_STRUCTURE_TYPE_RENDER_PASS_INPUT_ATTACHMENT_ASPECT_CREATE_INFO: {
            const auto _s401 = reinterpret_cast<const VkRenderPassInputAttachmentAspectCreateInfo *>(header);
            skip |= ValidateRenderPassInputAttachmentAspectCreateInfo(_parentObjects,
                _s401->sType,
                _s401->pNext,
                _s401->aspectReferenceCount,
                _s401->pAspectReferences);
            break;
        }
        case VK_STRUCTURE_TYPE_IMAGE_VIEW_USAGE_CREATE_INFO: {
            const auto _s402 = reinterpret_cast<const VkImageViewUsageCreateInfo *>(header);
            skip |= ValidateImageViewUsageCreateInfo(_parentObjects,
                _s402->sType,
                _s402->pNext,
                _s402->usage);
            break;
        }
        case VK_STRUCTURE_TYPE_PIPELINE_TESSELLATION_DOMAIN_ORIGIN_STATE_CREATE_INFO: {
            const auto _s403 = reinterpret_cast<const VkPipelineTessellationDomainOriginStateCreateInfo *>(header);
            skip |= ValidatePipelineTessellationDomainOriginStateCreateInfo(_parentObjects,
                _s403->sType,
                _s403->pNext,
                _s403->domainOrigin);
            break;
        }
        case VK_STRUCTURE_TYPE_RENDER_PASS_MULTIVIEW_CREATE_INFO: {
            const auto _s404 = reinterpret_cast<const VkRenderPassMultiviewCreateInfo *>(header);
            skip |= ValidateRenderPassMultiviewCreateInfo(_parentObjects,
                _s404->sType,
                _s404->pNext,
                _s404->subpassCount,
                _s404->pViewMasks,
                _s404->dependencyCount,
                _s404->pViewOffsets,
                _s404->correlationMaskCount,
                _s404->pCorrelationMasks);
            break;
        }
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MULTIVIEW_FEATURES: {
            const auto _s405 = reinterpret_cast<const VkPhysicalDeviceMultiviewFeatures *>(header);
            skip |= ValidatePhysicalDeviceMultiviewFeatures(_parentObjects,
                _s405->sType,
                _s405->pNext,
                _s405->multiview,
                _s405->multiviewGeometryShader,
                _s405->multiviewTessellationShader);
            break;
        }
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VARIABLE_POINTERS_FEATURES: {
            const auto _s406 = reinterpret_cast<const VkPhysicalDeviceVariablePointersFeatures *>(header);
            skip |= ValidatePhysicalDeviceVariablePointersFeatures(_parentObjects,
                _s406->sType,
                _s406->pNext,
                _s406->variablePointersStorageBuffer,
                _s406->variablePointers);
            break;
        }
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROTECTED_MEMORY_FEATURES: {
            const auto _s407 = reinterpret_cast<const VkPhysicalDeviceProtectedMemoryFeatures *>(header);
            skip |= ValidatePhysicalDeviceProtectedMemoryFeatures(_parentObjects,
                _s407->sType,
                _s407->pNext,
                _s407->protectedMemory);
            break;
        }
        case VK_STRUCTURE_TYPE_PROTECTED_SUBMIT_INFO: {
            const auto _s408 = reinterpret_cast<const VkProtectedSubmitInfo *>(header);
            skip |= ValidateProtectedSubmitInfo(_parentObjects,
                _s408->sType,
                _s408->pNext,
                _s408->protectedSubmit);
            break;
        }
        case VK_STRUCTURE_TYPE_SAMPLER_YCBCR_CONVERSION_INFO: {
            const auto _s409 = reinterpret_cast<const VkSamplerYcbcrConversionInfo *>(header);
            skip |= ValidateSamplerYcbcrConversionInfo(_parentObjects,
                _s409->sType,
                _s409->pNext,
                _s409->conversion);
            break;
        }
        case VK_STRUCTURE_TYPE_BIND_IMAGE_PLANE_MEMORY_INFO: {
            const auto _s410 = reinterpret_cast<const VkBindImagePlaneMemoryInfo *>(header);
            skip |= ValidateBindImagePlaneMemoryInfo(_parentObjects,
                _s410->sType,
                _s410->pNext,
                _s410->planeAspect);
            break;
        }
        case VK_STRUCTURE_TYPE_IMAGE_PLANE_MEMORY_REQUIREMENTS_INFO: {
            const auto _s411 = reinterpret_cast<const VkImagePlaneMemoryRequirementsInfo *>(header);
            skip |= ValidateImagePlaneMemoryRequirementsInfo(_parentObjects,
                _s411->sType,
                _s411->pNext,
                _s411->planeAspect);
            break;
        }
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SAMPLER_YCBCR_CONVERSION_FEATURES: {
            const auto _s412 = reinterpret_cast<const VkPhysicalDeviceSamplerYcbcrConversionFeatures *>(header);
            skip |= ValidatePhysicalDeviceSamplerYcbcrConversionFeatures(_parentObjects,
                _s412->sType,
                _s412->pNext,
                _s412->samplerYcbcrConversion);
            break;
        }
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_EXTERNAL_IMAGE_FORMAT_INFO: {
            const auto _s413 = reinterpret_cast<const VkPhysicalDeviceExternalImageFormatInfo *>(header);
            skip |= ValidatePhysicalDeviceExternalImageFormatInfo(_parentObjects,
                _s413->sType,
                _s413->pNext,
                _s413->handleType);
            break;
        }
        case VK_STRUCTURE_TYPE_EXTERNAL_MEMORY_IMAGE_CREATE_INFO: {
            const auto _s414 = reinterpret_cast<const VkExternalMemoryImageCreateInfo *>(header);
            skip |= ValidateExternalMemoryImageCreateInfo(_parentObjects,
                _s414->sType,
                _s414->pNext,
                _s414->handleTypes);
            break;
        }
        case VK_STRUCTURE_TYPE_EXTERNAL_MEMORY_BUFFER_CREATE_INFO: {
            const auto _s415 = reinterpret_cast<const VkExternalMemoryBufferCreateInfo *>(header);
            skip |= ValidateExternalMemoryBufferCreateInfo(_parentObjects,
                _s415->sType,
                _s415->pNext,
                _s415->handleTypes);
            break;
        }
        case VK_STRUCTURE_TYPE_EXPORT_MEMORY_ALLOCATE_INFO: {
            const auto _s416 = reinterpret_cast<const VkExportMemoryAllocateInfo *>(header);
            skip |= ValidateExportMemoryAllocateInfo(_parentObjects,
                _s416->sType,
                _s416->pNext,
                _s416->handleTypes);
            break;
        }
        case VK_STRUCTURE_TYPE_EXPORT_FENCE_CREATE_INFO: {
            const auto _s417 = reinterpret_cast<const VkExportFenceCreateInfo *>(header);
            skip |= ValidateExportFenceCreateInfo(_parentObjects,
                _s417->sType,
                _s417->pNext,
                _s417->handleTypes);
            break;
        }
        case VK_STRUCTURE_TYPE_EXPORT_SEMAPHORE_CREATE_INFO: {
            const auto _s418 = reinterpret_cast<const VkExportSemaphoreCreateInfo *>(header);
            skip |= ValidateExportSemaphoreCreateInfo(_parentObjects,
                _s418->sType,
                _s418->pNext,
                _s418->handleTypes);
            break;
        }
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_DRAW_PARAMETERS_FEATURES: {
            const auto _s419 = reinterpret_cast<const VkPhysicalDeviceShaderDrawParametersFeatures *>(header);
            skip |= ValidatePhysicalDeviceShaderDrawParametersFeatures(_parentObjects,
                _s419->sType,
                _s419->pNext,
                _s419->shaderDrawParameters);
            break;
        }
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_1_FEATURES: {
            const auto _s420 = reinterpret_cast<const VkPhysicalDeviceVulkan11Features *>(header);
            skip |= ValidatePhysicalDeviceVulkan11Features(_parentObjects,
                _s420->sType,
                _s420->pNext,
                _s420->storageBuffer16BitAccess,
                _s420->uniformAndStorageBuffer16BitAccess,
                _s420->storagePushConstant16,
                _s420->storageInputOutput16,
                _s420->multiview,
                _s420->multiviewGeometryShader,
                _s420->multiviewTessellationShader,
                _s420->variablePointersStorageBuffer,
                _s420->variablePointers,
                _s420->protectedMemory,
                _s420->samplerYcbcrConversion,
                _s420->shaderDrawParameters);
            break;
        }
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES: {
            const auto _s421 = reinterpret_cast<const VkPhysicalDeviceVulkan12Features *>(header);
            skip |= ValidatePhysicalDeviceVulkan12Features(_parentObjects,
                _s421->sType,
                _s421->pNext,
                _s421->samplerMirrorClampToEdge,
                _s421->drawIndirectCount,
                _s421->storageBuffer8BitAccess,
                _s421->uniformAndStorageBuffer8BitAccess,
                _s421->storagePushConstant8,
                _s421->shaderBufferInt64Atomics,
                _s421->shaderSharedInt64Atomics,
                _s421->shaderFloat16,
                _s421->shaderInt8,
                _s421->descriptorIndexing,
                _s421->shaderInputAttachmentArrayDynamicIndexing,
                _s421->shaderUniformTexelBufferArrayDynamicIndexing,
                _s421->shaderStorageTexelBufferArrayDynamicIndexing,
                _s421->shaderUniformBufferArrayNonUniformIndexing,
                _s421->shaderSampledImageArrayNonUniformIndexing,
                _s421->shaderStorageBufferArrayNonUniformIndexing,
                _s421->shaderStorageImageArrayNonUniformIndexing,
                _s421->shaderInputAttachmentArrayNonUniformIndexing,
                _s421->shaderUniformTexelBufferArrayNonUniformIndexing,
                _s421->shaderStorageTexelBufferArrayNonUniformIndexing,
                _s421->descriptorBindingUniformBufferUpdateAfterBind,
                _s421->descriptorBindingSampledImageUpdateAfterBind,
                _s421->descriptorBindingStorageImageUpdateAfterBind,
                _s421->descriptorBindingStorageBufferUpdateAfterBind,
                _s421->descriptorBindingUniformTexelBufferUpdateAfterBind,
                _s421->descriptorBindingStorageTexelBufferUpdateAfterBind,
                _s421->descriptorBindingUpdateUnusedWhilePending,
                _s421->descriptorBindingPartiallyBound,
                _s421->descriptorBindingVariableDescriptorCount,
                _s421->runtimeDescriptorArray,
                _s421->samplerFilterMinmax,
                _s421->scalarBlockLayout,
                _s421->imagelessFramebuffer,
                _s421->uniformBufferStandardLayout,
                _s421->shaderSubgroupExtendedTypes,
                _s421->separateDepthStencilLayouts,
                _s421->hostQueryReset,
                _s421->timelineSemaphore,
                _s421->bufferDeviceAddress,
                _s421->bufferDeviceAddressCaptureReplay,
                _s421->bufferDeviceAddressMultiDevice,
                _s421->vulkanMemoryModel,
                _s421->vulkanMemoryModelDeviceScope,
                _s421->vulkanMemoryModelAvailabilityVisibilityChains,
                _s421->shaderOutputViewportIndex,
                _s421->shaderOutputLayer,
                _s421->subgroupBroadcastDynamicId);
            break;
        }
        case VK_STRUCTURE_TYPE_IMAGE_FORMAT_LIST_CREATE_INFO: {
            const auto _s422 = reinterpret_cast<const VkImageFormatListCreateInfo *>(header);
            skip |= ValidateImageFormatListCreateInfo(_parentObjects,
                _s422->sType,
                _s422->pNext,
                _s422->viewFormatCount,
                _s422->pViewFormats);
            break;
        }
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_8BIT_STORAGE_FEATURES: {
            const auto _s423 = reinterpret_cast<const VkPhysicalDevice8BitStorageFeatures *>(header);
            skip |= ValidatePhysicalDevice8BitStorageFeatures(_parentObjects,
                _s423->sType,
                _s423->pNext,
                _s423->storageBuffer8BitAccess,
                _s423->uniformAndStorageBuffer8BitAccess,
                _s423->storagePushConstant8);
            break;
        }
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_ATOMIC_INT64_FEATURES: {
            const auto _s424 = reinterpret_cast<const VkPhysicalDeviceShaderAtomicInt64Features *>(header);
            skip |= ValidatePhysicalDeviceShaderAtomicInt64Features(_parentObjects,
                _s424->sType,
                _s424->pNext,
                _s424->shaderBufferInt64Atomics,
                _s424->shaderSharedInt64Atomics);
            break;
        }
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_FLOAT16_INT8_FEATURES: {
            const auto _s425 = reinterpret_cast<const VkPhysicalDeviceShaderFloat16Int8Features *>(header);
            skip |= ValidatePhysicalDeviceShaderFloat16Int8Features(_parentObjects,
                _s425->sType,
                _s425->pNext,
                _s425->shaderFloat16,
                _s425->shaderInt8);
            break;
        }
        case VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_BINDING_FLAGS_CREATE_INFO: {
            const auto _s426 = reinterpret_cast<const VkDescriptorSetLayoutBindingFlagsCreateInfo *>(header);
            skip |= ValidateDescriptorSetLayoutBindingFlagsCreateInfo(_parentObjects,
                _s426->sType,
                _s426->pNext,
                _s426->bindingCount,
                _s426->pBindingFlags);
            break;
        }
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_INDEXING_FEATURES: {
            const auto _s427 = reinterpret_cast<const VkPhysicalDeviceDescriptorIndexingFeatures *>(header);
            skip |= ValidatePhysicalDeviceDescriptorIndexingFeatures(_parentObjects,
                _s427->sType,
                _s427->pNext,
                _s427->shaderInputAttachmentArrayDynamicIndexing,
                _s427->shaderUniformTexelBufferArrayDynamicIndexing,
                _s427->shaderStorageTexelBufferArrayDynamicIndexing,
                _s427->shaderUniformBufferArrayNonUniformIndexing,
                _s427->shaderSampledImageArrayNonUniformIndexing,
                _s427->shaderStorageBufferArrayNonUniformIndexing,
                _s427->shaderStorageImageArrayNonUniformIndexing,
                _s427->shaderInputAttachmentArrayNonUniformIndexing,
                _s427->shaderUniformTexelBufferArrayNonUniformIndexing,
                _s427->shaderStorageTexelBufferArrayNonUniformIndexing,
                _s427->descriptorBindingUniformBufferUpdateAfterBind,
                _s427->descriptorBindingSampledImageUpdateAfterBind,
                _s427->descriptorBindingStorageImageUpdateAfterBind,
                _s427->descriptorBindingStorageBufferUpdateAfterBind,
                _s427->descriptorBindingUniformTexelBufferUpdateAfterBind,
                _s427->descriptorBindingStorageTexelBufferUpdateAfterBind,
                _s427->descriptorBindingUpdateUnusedWhilePending,
                _s427->descriptorBindingPartiallyBound,
                _s427->descriptorBindingVariableDescriptorCount,
                _s427->runtimeDescriptorArray);
            break;
        }
        case VK_STRUCTURE_TYPE_DESCRIPTOR_SET_VARIABLE_DESCRIPTOR_COUNT_ALLOCATE_INFO: {
            const auto _s428 = reinterpret_cast<const VkDescriptorSetVariableDescriptorCountAllocateInfo *>(header);
            skip |= ValidateDescriptorSetVariableDescriptorCountAllocateInfo(_parentObjects,
                _s428->sType,
                _s428->pNext,
                _s428->descriptorSetCount,
                _s428->pDescriptorCounts);
            break;
        }
        case VK_STRUCTURE_TYPE_SUBPASS_DESCRIPTION_DEPTH_STENCIL_RESOLVE: {
            const auto _s429 = reinterpret_cast<const VkSubpassDescriptionDepthStencilResolve *>(header);
            skip |= ValidateSubpassDescriptionDepthStencilResolve(_parentObjects,
                _s429->sType,
                _s429->pNext,
                _s429->depthResolveMode,
                _s429->stencilResolveMode,
                _s429->pDepthStencilResolveAttachment);
            break;
        }
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SCALAR_BLOCK_LAYOUT_FEATURES: {
            const auto _s430 = reinterpret_cast<const VkPhysicalDeviceScalarBlockLayoutFeatures *>(header);
            skip |= ValidatePhysicalDeviceScalarBlockLayoutFeatures(_parentObjects,
                _s430->sType,
                _s430->pNext,
                _s430->scalarBlockLayout);
            break;
        }
        case VK_STRUCTURE_TYPE_IMAGE_STENCIL_USAGE_CREATE_INFO: {
            const auto _s431 = reinterpret_cast<const VkImageStencilUsageCreateInfo *>(header);
            skip |= ValidateImageStencilUsageCreateInfo(_parentObjects,
                _s431->sType,
                _s431->pNext,
                _s431->stencilUsage);
            break;
        }
        case VK_STRUCTURE_TYPE_SAMPLER_REDUCTION_MODE_CREATE_INFO: {
            const auto _s432 = reinterpret_cast<const VkSamplerReductionModeCreateInfo *>(header);
            skip |= ValidateSamplerReductionModeCreateInfo(_parentObjects,
                _s432->sType,
                _s432->pNext,
                _s432->reductionMode);
            break;
        }
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_MEMORY_MODEL_FEATURES: {
            const auto _s433 = reinterpret_cast<const VkPhysicalDeviceVulkanMemoryModelFeatures *>(header);
            skip |= ValidatePhysicalDeviceVulkanMemoryModelFeatures(_parentObjects,
                _s433->sType,
                _s433->pNext,
                _s433->vulkanMemoryModel,
                _s433->vulkanMemoryModelDeviceScope,
                _s433->vulkanMemoryModelAvailabilityVisibilityChains);
            break;
        }
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_IMAGELESS_FRAMEBUFFER_FEATURES: {
            const auto _s434 = reinterpret_cast<const VkPhysicalDeviceImagelessFramebufferFeatures *>(header);
            skip |= ValidatePhysicalDeviceImagelessFramebufferFeatures(_parentObjects,
                _s434->sType,
                _s434->pNext,
                _s434->imagelessFramebuffer);
            break;
        }
        case VK_STRUCTURE_TYPE_FRAMEBUFFER_ATTACHMENTS_CREATE_INFO: {
            const auto _s435 = reinterpret_cast<const VkFramebufferAttachmentsCreateInfo *>(header);
            skip |= ValidateFramebufferAttachmentsCreateInfo(_parentObjects,
                _s435->sType,
                _s435->pNext,
                _s435->attachmentImageInfoCount,
                _s435->pAttachmentImageInfos);
            break;
        }
        case VK_STRUCTURE_TYPE_RENDER_PASS_ATTACHMENT_BEGIN_INFO: {
            const auto _s436 = reinterpret_cast<const VkRenderPassAttachmentBeginInfo *>(header);
            skip |= ValidateRenderPassAttachmentBeginInfo(_parentObjects,
                _s436->sType,
                _s436->pNext,
                _s436->attachmentCount,
                _s436->pAttachments);
            break;
        }
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_UNIFORM_BUFFER_STANDARD_LAYOUT_FEATURES: {
            const auto _s437 = reinterpret_cast<const VkPhysicalDeviceUniformBufferStandardLayoutFeatures *>(header);
            skip |= ValidatePhysicalDeviceUniformBufferStandardLayoutFeatures(_parentObjects,
                _s437->sType,
                _s437->pNext,
                _s437->uniformBufferStandardLayout);
            break;
        }
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_SUBGROUP_EXTENDED_TYPES_FEATURES: {
            const auto _s438 = reinterpret_cast<const VkPhysicalDeviceShaderSubgroupExtendedTypesFeatures *>(header);
            skip |= ValidatePhysicalDeviceShaderSubgroupExtendedTypesFeatures(_parentObjects,
                _s438->sType,
                _s438->pNext,
                _s438->shaderSubgroupExtendedTypes);
            break;
        }
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SEPARATE_DEPTH_STENCIL_LAYOUTS_FEATURES: {
            const auto _s439 = reinterpret_cast<const VkPhysicalDeviceSeparateDepthStencilLayoutsFeatures *>(header);
            skip |= ValidatePhysicalDeviceSeparateDepthStencilLayoutsFeatures(_parentObjects,
                _s439->sType,
                _s439->pNext,
                _s439->separateDepthStencilLayouts);
            break;
        }
        case VK_STRUCTURE_TYPE_ATTACHMENT_REFERENCE_STENCIL_LAYOUT: {
            const auto _s440 = reinterpret_cast<const VkAttachmentReferenceStencilLayout *>(header);
            skip |= ValidateAttachmentReferenceStencilLayout(_parentObjects,
                _s440->sType,
                _s440->pNext,
                _s440->stencilLayout);
            break;
        }
        case VK_STRUCTURE_TYPE_ATTACHMENT_DESCRIPTION_STENCIL_LAYOUT: {
            const auto _s441 = reinterpret_cast<const VkAttachmentDescriptionStencilLayout *>(header);
            skip |= ValidateAttachmentDescriptionStencilLayout(_parentObjects,
                _s441->sType,
                _s441->pNext,
                _s441->stencilInitialLayout,
                _s441->stencilFinalLayout);
            break;
        }
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_HOST_QUERY_RESET_FEATURES: {
            const auto _s442 = reinterpret_cast<const VkPhysicalDeviceHostQueryResetFeatures *>(header);
            skip |= ValidatePhysicalDeviceHostQueryResetFeatures(_parentObjects,
                _s442->sType,
                _s442->pNext,
                _s442->hostQueryReset);
            break;
        }
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_TIMELINE_SEMAPHORE_FEATURES: {
            const auto _s443 = reinterpret_cast<const VkPhysicalDeviceTimelineSemaphoreFeatures *>(header);
            skip |= ValidatePhysicalDeviceTimelineSemaphoreFeatures(_parentObjects,
                _s443->sType,
                _s443->pNext,
                _s443->timelineSemaphore);
            break;
        }
        case VK_STRUCTURE_TYPE_SEMAPHORE_TYPE_CREATE_INFO: {
            const auto _s444 = reinterpret_cast<const VkSemaphoreTypeCreateInfo *>(header);
            skip |= ValidateSemaphoreTypeCreateInfo(_parentObjects,
                _s444->sType,
                _s444->pNext,
                _s444->semaphoreType,
                _s444->initialValue);
            break;
        }
        case VK_STRUCTURE_TYPE_TIMELINE_SEMAPHORE_SUBMIT_INFO: {
            const auto _s445 = reinterpret_cast<const VkTimelineSemaphoreSubmitInfo *>(header);
            skip |= ValidateTimelineSemaphoreSubmitInfo(_parentObjects,
                _s445->sType,
                _s445->pNext,
                _s445->waitSemaphoreValueCount,
                _s445->pWaitSemaphoreValues,
                _s445->signalSemaphoreValueCount,
                _s445->pSignalSemaphoreValues);
            break;
        }
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_BUFFER_DEVICE_ADDRESS_FEATURES: {
            const auto _s446 = reinterpret_cast<const VkPhysicalDeviceBufferDeviceAddressFeatures *>(header);
            skip |= ValidatePhysicalDeviceBufferDeviceAddressFeatures(_parentObjects,
                _s446->sType,
                _s446->pNext,
                _s446->bufferDeviceAddress,
                _s446->bufferDeviceAddressCaptureReplay,
                _s446->bufferDeviceAddressMultiDevice);
            break;
        }
        case VK_STRUCTURE_TYPE_BUFFER_OPAQUE_CAPTURE_ADDRESS_CREATE_INFO: {
            const auto _s447 = reinterpret_cast<const VkBufferOpaqueCaptureAddressCreateInfo *>(header);
            skip |= ValidateBufferOpaqueCaptureAddressCreateInfo(_parentObjects,
                _s447->sType,
                _s447->pNext,
                _s447->opaqueCaptureAddress);
            break;
        }
        case VK_STRUCTURE_TYPE_MEMORY_OPAQUE_CAPTURE_ADDRESS_ALLOCATE_INFO: {
            const auto _s448 = reinterpret_cast<const VkMemoryOpaqueCaptureAddressAllocateInfo *>(header);
            skip |= ValidateMemoryOpaqueCaptureAddressAllocateInfo(_parentObjects,
                _s448->sType,
                _s448->pNext,
                _s448->opaqueCaptureAddress);
            break;
        }
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES: {
            const auto _s449 = reinterpret_cast<const VkPhysicalDeviceVulkan13Features *>(header);
            skip |= ValidatePhysicalDeviceVulkan13Features(_parentObjects,
                _s449->sType,
                _s449->pNext,
                _s449->robustImageAccess,
                _s449->inlineUniformBlock,
                _s449->descriptorBindingInlineUniformBlockUpdateAfterBind,
                _s449->pipelineCreationCacheControl,
                _s449->privateData,
                _s449->shaderDemoteToHelperInvocation,
                _s449->shaderTerminateInvocation,
                _s449->subgroupSizeControl,
                _s449->computeFullSubgroups,
                _s449->synchronization2,
                _s449->textureCompressionASTC_HDR,
                _s449->shaderZeroInitializeWorkgroupMemory,
                _s449->dynamicRendering,
                _s449->shaderIntegerDotProduct,
                _s449->maintenance4);
            break;
        }
        case VK_STRUCTURE_TYPE_PIPELINE_CREATION_FEEDBACK_CREATE_INFO: {
            const auto _s450 = reinterpret_cast<const VkPipelineCreationFeedbackCreateInfo *>(header);
            skip |= ValidatePipelineCreationFeedbackCreateInfo(_parentObjects,
                _s450->sType,
                _s450->pNext,
                _s450->pPipelineCreationFeedback,
                _s450->pipelineStageCreationFeedbackCount,
                _s450->pPipelineStageCreationFeedbacks);
            break;
        }
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_TERMINATE_INVOCATION_FEATURES: {
            const auto _s451 = reinterpret_cast<const VkPhysicalDeviceShaderTerminateInvocationFeatures *>(header);
            skip |= ValidatePhysicalDeviceShaderTerminateInvocationFeatures(_parentObjects,
                _s451->sType,
                _s451->pNext,
                _s451->shaderTerminateInvocation);
            break;
        }
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_DEMOTE_TO_HELPER_INVOCATION_FEATURES: {
            const auto _s452 = reinterpret_cast<const VkPhysicalDeviceShaderDemoteToHelperInvocationFeatures *>(header);
            skip |= ValidatePhysicalDeviceShaderDemoteToHelperInvocationFeatures(_parentObjects,
                _s452->sType,
                _s452->pNext,
                _s452->shaderDemoteToHelperInvocation);
            break;
        }
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PRIVATE_DATA_FEATURES: {
            const auto _s453 = reinterpret_cast<const VkPhysicalDevicePrivateDataFeatures *>(header);
            skip |= ValidatePhysicalDevicePrivateDataFeatures(_parentObjects,
                _s453->sType,
                _s453->pNext,
                _s453->privateData);
            break;
        }
        case VK_STRUCTURE_TYPE_DEVICE_PRIVATE_DATA_CREATE_INFO: {
            const auto _s454 = reinterpret_cast<const VkDevicePrivateDataCreateInfo *>(header);
            skip |= ValidateDevicePrivateDataCreateInfo(_parentObjects,
                _s454->sType,
                _s454->pNext,
                _s454->privateDataSlotRequestCount);
            break;
        }
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PIPELINE_CREATION_CACHE_CONTROL_FEATURES: {
            const auto _s455 = reinterpret_cast<const VkPhysicalDevicePipelineCreationCacheControlFeatures *>(header);
            skip |= ValidatePhysicalDevicePipelineCreationCacheControlFeatures(_parentObjects,
                _s455->sType,
                _s455->pNext,
                _s455->pipelineCreationCacheControl);
            break;
        }
        case VK_STRUCTURE_TYPE_MEMORY_BARRIER_2: {
            const auto _s456 = reinterpret_cast<const VkMemoryBarrier2 *>(header);
            skip |= ValidateMemoryBarrier2(_parentObjects,
                _s456->sType,
                _s456->pNext,
                _s456->srcStageMask,
                _s456->srcAccessMask,
                _s456->dstStageMask,
                _s456->dstAccessMask);
            break;
        }
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SYNCHRONIZATION_2_FEATURES: {
            const auto _s457 = reinterpret_cast<const VkPhysicalDeviceSynchronization2Features *>(header);
            skip |= ValidatePhysicalDeviceSynchronization2Features(_parentObjects,
                _s457->sType,
                _s457->pNext,
                _s457->synchronization2);
            break;
        }
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ZERO_INITIALIZE_WORKGROUP_MEMORY_FEATURES: {
            const auto _s458 = reinterpret_cast<const VkPhysicalDeviceZeroInitializeWorkgroupMemoryFeatures *>(header);
            skip |= ValidatePhysicalDeviceZeroInitializeWorkgroupMemoryFeatures(_parentObjects,
                _s458->sType,
                _s458->pNext,
                _s458->shaderZeroInitializeWorkgroupMemory);
            break;
        }
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_IMAGE_ROBUSTNESS_FEATURES: {
            const auto _s459 = reinterpret_cast<const VkPhysicalDeviceImageRobustnessFeatures *>(header);
            skip |= ValidatePhysicalDeviceImageRobustnessFeatures(_parentObjects,
                _s459->sType,
                _s459->pNext,
                _s459->robustImageAccess);
            break;
        }
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SUBGROUP_SIZE_CONTROL_FEATURES: {
            const auto _s460 = reinterpret_cast<const VkPhysicalDeviceSubgroupSizeControlFeatures *>(header);
            skip |= ValidatePhysicalDeviceSubgroupSizeControlFeatures(_parentObjects,
                _s460->sType,
                _s460->pNext,
                _s460->subgroupSizeControl,
                _s460->computeFullSubgroups);
            break;
        }
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_INLINE_UNIFORM_BLOCK_FEATURES: {
            const auto _s461 = reinterpret_cast<const VkPhysicalDeviceInlineUniformBlockFeatures *>(header);
            skip |= ValidatePhysicalDeviceInlineUniformBlockFeatures(_parentObjects,
                _s461->sType,
                _s461->pNext,
                _s461->inlineUniformBlock,
                _s461->descriptorBindingInlineUniformBlockUpdateAfterBind);
            break;
        }
        case VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET_INLINE_UNIFORM_BLOCK: {
            const auto _s462 = reinterpret_cast<const VkWriteDescriptorSetInlineUniformBlock *>(header);
            skip |= ValidateWriteDescriptorSetInlineUniformBlock(_parentObjects,
                _s462->sType,
                _s462->pNext,
                _s462->dataSize,
                _s462->pData);
            break;
        }
        case VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_INLINE_UNIFORM_BLOCK_CREATE_INFO: {
            const auto _s463 = reinterpret_cast<const VkDescriptorPoolInlineUniformBlockCreateInfo *>(header);
            skip |= ValidateDescriptorPoolInlineUniformBlockCreateInfo(_parentObjects,
                _s463->sType,
                _s463->pNext,
                _s463->maxInlineUniformBlockBindings);
            break;
        }
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_TEXTURE_COMPRESSION_ASTC_HDR_FEATURES: {
            const auto _s464 = reinterpret_cast<const VkPhysicalDeviceTextureCompressionASTCHDRFeatures *>(header);
            skip |= ValidatePhysicalDeviceTextureCompressionASTCHDRFeatures(_parentObjects,
                _s464->sType,
                _s464->pNext,
                _s464->textureCompressionASTC_HDR);
            break;
        }
        case VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO: {
            const auto _s465 = reinterpret_cast<const VkPipelineRenderingCreateInfo *>(header);
            skip |= ValidatePipelineRenderingCreateInfo(_parentObjects,
                _s465->sType,
                _s465->pNext,
                _s465->viewMask,
                _s465->colorAttachmentCount,
                _s465->pColorAttachmentFormats,
                _s465->depthAttachmentFormat,
                _s465->stencilAttachmentFormat);
            break;
        }
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DYNAMIC_RENDERING_FEATURES: {
            const auto _s466 = reinterpret_cast<const VkPhysicalDeviceDynamicRenderingFeatures *>(header);
            skip |= ValidatePhysicalDeviceDynamicRenderingFeatures(_parentObjects,
                _s466->sType,
                _s466->pNext,
                _s466->dynamicRendering);
            break;
        }
        case VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_RENDERING_INFO: {
            const auto _s467 = reinterpret_cast<const VkCommandBufferInheritanceRenderingInfo *>(header);
            skip |= ValidateCommandBufferInheritanceRenderingInfo(_parentObjects,
                _s467->sType,
                _s467->pNext,
                _s467->flags,
                _s467->viewMask,
                _s467->colorAttachmentCount,
                _s467->pColorAttachmentFormats,
                _s467->depthAttachmentFormat,
                _s467->stencilAttachmentFormat,
                _s467->rasterizationSamples);
            break;
        }
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_INTEGER_DOT_PRODUCT_FEATURES: {
            const auto _s468 = reinterpret_cast<const VkPhysicalDeviceShaderIntegerDotProductFeatures *>(header);
            skip |= ValidatePhysicalDeviceShaderIntegerDotProductFeatures(_parentObjects,
                _s468->sType,
                _s468->pNext,
                _s468->shaderIntegerDotProduct);
            break;
        }
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MAINTENANCE_4_FEATURES: {
            const auto _s469 = reinterpret_cast<const VkPhysicalDeviceMaintenance4Features *>(header);
            skip |= ValidatePhysicalDeviceMaintenance4Features(_parentObjects,
                _s469->sType,
                _s469->pNext,
                _s469->maintenance4);
            break;
        }
        case VK_STRUCTURE_TYPE_IMAGE_SWAPCHAIN_CREATE_INFO_KHR: {
            const auto _s470 = reinterpret_cast<const VkImageSwapchainCreateInfoKHR *>(header);
            skip |= ValidateImageSwapchainCreateInfoKHR(_parentObjects,
                _s470->sType,
                _s470->pNext,
                _s470->swapchain);
            break;
        }
        case VK_STRUCTURE_TYPE_BIND_IMAGE_MEMORY_SWAPCHAIN_INFO_KHR: {
            const auto _s471 = reinterpret_cast<const VkBindImageMemorySwapchainInfoKHR *>(header);
            skip |= ValidateBindImageMemorySwapchainInfoKHR(_parentObjects,
                _s471->sType,
                _s471->pNext,
                _s471->swapchain,
                _s471->imageIndex);
            break;
        }
        case VK_STRUCTURE_TYPE_DEVICE_GROUP_PRESENT_INFO_KHR: {
            const auto _s472 = reinterpret_cast<const VkDeviceGroupPresentInfoKHR *>(header);
            skip |= ValidateDeviceGroupPresentInfoKHR(_parentObjects,
                _s472->sType,
                _s472->pNext,
                _s472->swapchainCount,
                _s472->pDeviceMasks,
                _s472->mode);
            break;
        }
        case VK_STRUCTURE_TYPE_DEVICE_GROUP_SWAPCHAIN_CREATE_INFO_KHR: {
            const auto _s473 = reinterpret_cast<const VkDeviceGroupSwapchainCreateInfoKHR *>(header);
            skip |= ValidateDeviceGroupSwapchainCreateInfoKHR(_parentObjects,
                _s473->sType,
                _s473->pNext,
                _s473->modes);
            break;
        }
        case VK_STRUCTURE_TYPE_DISPLAY_PRESENT_INFO_KHR: {
            const auto _s474 = reinterpret_cast<const VkDisplayPresentInfoKHR *>(header);
            skip |= ValidateDisplayPresentInfoKHR(_parentObjects,
                _s474->sType,
                _s474->pNext,
                _s474->srcRect,
                _s474->dstRect,
                _s474->persistent);
            break;
        }
        case VK_STRUCTURE_TYPE_VIDEO_PROFILE_INFO_KHR: {
            const auto _s475 = reinterpret_cast<const VkVideoProfileInfoKHR *>(header);
            skip |= ValidateVideoProfileInfoKHR(_parentObjects,
                _s475->sType,
                _s475->pNext,
                _s475->videoCodecOperation,
                _s475->chromaSubsampling,
                _s475->lumaBitDepth,
                _s475->chromaBitDepth);
            break;
        }
        case VK_STRUCTURE_TYPE_VIDEO_PROFILE_LIST_INFO_KHR: {
            const auto _s476 = reinterpret_cast<const VkVideoProfileListInfoKHR *>(header);
            skip |= ValidateVideoProfileListInfoKHR(_parentObjects,
                _s476->sType,
                _s476->pNext,
                _s476->profileCount,
                _s476->pProfiles);
            break;
        }
        case VK_STRUCTURE_TYPE_VIDEO_DECODE_USAGE_INFO_KHR: {
            const auto _s477 = reinterpret_cast<const VkVideoDecodeUsageInfoKHR *>(header);
            skip |= ValidateVideoDecodeUsageInfoKHR(_parentObjects,
                _s477->sType,
                _s477->pNext,
                _s477->videoUsageHints);
            break;
        }
        case VK_STRUCTURE_TYPE_VIDEO_DECODE_H264_PROFILE_INFO_KHR: {
            const auto _s478 = reinterpret_cast<const VkVideoDecodeH264ProfileInfoKHR *>(header);
            skip |= ValidateVideoDecodeH264ProfileInfoKHR(_parentObjects,
                _s478->sType,
                _s478->pNext,
                _s478->stdProfileIdc,
                _s478->pictureLayout);
            break;
        }
        case VK_STRUCTURE_TYPE_VIDEO_DECODE_H264_SESSION_PARAMETERS_ADD_INFO_KHR: {
            const auto _s479 = reinterpret_cast<const VkVideoDecodeH264SessionParametersAddInfoKHR *>(header);
            skip |= ValidateVideoDecodeH264SessionParametersAddInfoKHR(_parentObjects,
                _s479->sType,
                _s479->pNext,
                _s479->stdSPSCount,
                _s479->pStdSPSs,
                _s479->stdPPSCount,
                _s479->pStdPPSs);
            break;
        }
        case VK_STRUCTURE_TYPE_VIDEO_DECODE_H264_SESSION_PARAMETERS_CREATE_INFO_KHR: {
            const auto _s480 = reinterpret_cast<const VkVideoDecodeH264SessionParametersCreateInfoKHR *>(header);
            skip |= ValidateVideoDecodeH264SessionParametersCreateInfoKHR(_parentObjects,
                _s480->sType,
                _s480->pNext,
                _s480->maxStdSPSCount,
                _s480->maxStdPPSCount,
                _s480->pParametersAddInfo);
            break;
        }
        case VK_STRUCTURE_TYPE_VIDEO_DECODE_H264_PICTURE_INFO_KHR: {
            const auto _s481 = reinterpret_cast<const VkVideoDecodeH264PictureInfoKHR *>(header);
            skip |= ValidateVideoDecodeH264PictureInfoKHR(_parentObjects,
                _s481->sType,
                _s481->pNext,
                _s481->pStdPictureInfo,
                _s481->sliceCount,
                _s481->pSliceOffsets);
            break;
        }
        case VK_STRUCTURE_TYPE_VIDEO_DECODE_H264_DPB_SLOT_INFO_KHR: {
            const auto _s482 = reinterpret_cast<const VkVideoDecodeH264DpbSlotInfoKHR *>(header);
            skip |= ValidateVideoDecodeH264DpbSlotInfoKHR(_parentObjects,
                _s482->sType,
                _s482->pNext,
                _s482->pStdReferenceInfo);
            break;
        }
        case VK_STRUCTURE_TYPE_RENDERING_FRAGMENT_SHADING_RATE_ATTACHMENT_INFO_KHR: {
            const auto _s483 = reinterpret_cast<const VkRenderingFragmentShadingRateAttachmentInfoKHR *>(header);
            skip |= ValidateRenderingFragmentShadingRateAttachmentInfoKHR(_parentObjects,
                _s483->sType,
                _s483->pNext,
                _s483->imageView,
                _s483->imageLayout,
                _s483->shadingRateAttachmentTexelSize);
            break;
        }
        case VK_STRUCTURE_TYPE_RENDERING_FRAGMENT_DENSITY_MAP_ATTACHMENT_INFO_EXT: {
            const auto _s484 = reinterpret_cast<const VkRenderingFragmentDensityMapAttachmentInfoEXT *>(header);
            skip |= ValidateRenderingFragmentDensityMapAttachmentInfoEXT(_parentObjects,
                _s484->sType,
                _s484->pNext,
                _s484->imageView,
                _s484->imageLayout);
            break;
        }
        case VK_STRUCTURE_TYPE_ATTACHMENT_SAMPLE_COUNT_INFO_AMD: {
            const auto _s485 = reinterpret_cast<const VkAttachmentSampleCountInfoAMD *>(header);
            skip |= ValidateAttachmentSampleCountInfoAMD(_parentObjects,
                _s485->sType,
                _s485->pNext,
                _s485->colorAttachmentCount,
                _s485->pColorAttachmentSamples,
                _s485->depthStencilAttachmentSamples);
            break;
        }
        case VK_STRUCTURE_TYPE_MULTIVIEW_PER_VIEW_ATTRIBUTES_INFO_NVX: {
            const auto _s486 = reinterpret_cast<const VkMultiviewPerViewAttributesInfoNVX *>(header);
            skip |= ValidateMultiviewPerViewAttributesInfoNVX(_parentObjects,
                _s486->sType,
                _s486->pNext,
                _s486->perViewAttributes,
                _s486->perViewAttributesPositionXOnly);
            break;
        }
#ifdef VK_USE_PLATFORM_WIN32_KHR
        case VK_STRUCTURE_TYPE_IMPORT_MEMORY_WIN32_HANDLE_INFO_KHR: {
            const auto _s487 = reinterpret_cast<const VkImportMemoryWin32HandleInfoKHR *>(header);
            skip |= ValidateImportMemoryWin32HandleInfoKHR(_parentObjects,
                _s487->sType,
                _s487->pNext,
                _s487->handleType,
                _s487->handle,
                _s487->name);
            break;
        }
#endif
#ifdef VK_USE_PLATFORM_WIN32_KHR
        case VK_STRUCTURE_TYPE_EXPORT_MEMORY_WIN32_HANDLE_INFO_KHR: {
            const auto _s488 = reinterpret_cast<const VkExportMemoryWin32HandleInfoKHR *>(header);
            skip |= ValidateExportMemoryWin32HandleInfoKHR(_parentObjects,
                _s488->sType,
                _s488->pNext,
                _s488->pAttributes,
                _s488->dwAccess,
                _s488->name);
            break;
        }
#endif
        case VK_STRUCTURE_TYPE_IMPORT_MEMORY_FD_INFO_KHR: {
            const auto _s489 = reinterpret_cast<const VkImportMemoryFdInfoKHR *>(header);
            skip |= ValidateImportMemoryFdInfoKHR(_parentObjects,
                _s489->sType,
                _s489->pNext,
                _s489->handleType,
                _s489->fd);
            break;
        }
#ifdef VK_USE_PLATFORM_WIN32_KHR
        case VK_STRUCTURE_TYPE_WIN32_KEYED_MUTEX_ACQUIRE_RELEASE_INFO_KHR: {
            const auto _s490 = reinterpret_cast<const VkWin32KeyedMutexAcquireReleaseInfoKHR *>(header);
            skip |= ValidateWin32KeyedMutexAcquireReleaseInfoKHR(_parentObjects,
                _s490->sType,
                _s490->pNext,
                _s490->acquireCount,
                _s490->pAcquireSyncs,
                _s490->pAcquireKeys,
                _s490->pAcquireTimeouts,
                _s490->releaseCount,
                _s490->pReleaseSyncs,
                _s490->pReleaseKeys);
            break;
        }
#endif
#ifdef VK_USE_PLATFORM_WIN32_KHR
        case VK_STRUCTURE_TYPE_EXPORT_SEMAPHORE_WIN32_HANDLE_INFO_KHR: {
            const auto _s491 = reinterpret_cast<const VkExportSemaphoreWin32HandleInfoKHR *>(header);
            skip |= ValidateExportSemaphoreWin32HandleInfoKHR(_parentObjects,
                _s491->sType,
                _s491->pNext,
                _s491->pAttributes,
                _s491->dwAccess,
                _s491->name);
            break;
        }
#endif
#ifdef VK_USE_PLATFORM_WIN32_KHR
        case VK_STRUCTURE_TYPE_D3D12_FENCE_SUBMIT_INFO_KHR: {
            const auto _s492 = reinterpret_cast<const VkD3D12FenceSubmitInfoKHR *>(header);
            skip |= ValidateD3D12FenceSubmitInfoKHR(_parentObjects,
                _s492->sType,
                _s492->pNext,
                _s492->waitSemaphoreValuesCount,
                _s492->pWaitSemaphoreValues,
                _s492->signalSemaphoreValuesCount,
                _s492->pSignalSemaphoreValues);
            break;
        }
#endif
        case VK_STRUCTURE_TYPE_PRESENT_REGIONS_KHR: {
            const auto _s493 = reinterpret_cast<const VkPresentRegionsKHR *>(header);
            skip |= ValidatePresentRegionsKHR(_parentObjects,
                _s493->sType,
                _s493->pNext,
                _s493->swapchainCount,
                _s493->pRegions);
            break;
        }
#ifdef VK_USE_PLATFORM_WIN32_KHR
        case VK_STRUCTURE_TYPE_EXPORT_FENCE_WIN32_HANDLE_INFO_KHR: {
            const auto _s494 = reinterpret_cast<const VkExportFenceWin32HandleInfoKHR *>(header);
            skip |= ValidateExportFenceWin32HandleInfoKHR(_parentObjects,
                _s494->sType,
                _s494->pNext,
                _s494->pAttributes,
                _s494->dwAccess,
                _s494->name);
            break;
        }
#endif
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PERFORMANCE_QUERY_FEATURES_KHR: {
            const auto _s495 = reinterpret_cast<const VkPhysicalDevicePerformanceQueryFeaturesKHR *>(header);
            skip |= ValidatePhysicalDevicePerformanceQueryFeaturesKHR(_parentObjects,
                _s495->sType,
                _s495->pNext,
                _s495->performanceCounterQueryPools,
                _s495->performanceCounterMultipleQueryPools);
            break;
        }
        case VK_STRUCTURE_TYPE_QUERY_POOL_PERFORMANCE_CREATE_INFO_KHR: {
            const auto _s496 = reinterpret_cast<const VkQueryPoolPerformanceCreateInfoKHR *>(header);
            skip |= ValidateQueryPoolPerformanceCreateInfoKHR(_parentObjects,
                _s496->sType,
                _s496->pNext,
                _s496->queueFamilyIndex,
                _s496->counterIndexCount,
                _s496->pCounterIndices);
            break;
        }
        case VK_STRUCTURE_TYPE_PERFORMANCE_QUERY_SUBMIT_INFO_KHR: {
            const auto _s497 = reinterpret_cast<const VkPerformanceQuerySubmitInfoKHR *>(header);
            skip |= ValidatePerformanceQuerySubmitInfoKHR(_parentObjects,
                _s497->sType,
                _s497->pNext,
                _s497->counterPassIndex);
            break;
        }
#ifdef VK_ENABLE_BETA_EXTENSIONS
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PORTABILITY_SUBSET_FEATURES_KHR: {
            const auto _s498 = reinterpret_cast<const VkPhysicalDevicePortabilitySubsetFeaturesKHR *>(header);
            skip |= ValidatePhysicalDevicePortabilitySubsetFeaturesKHR(_parentObjects,
                _s498->sType,
                _s498->pNext,
                _s498->constantAlphaColorBlendFactors,
                _s498->events,
                _s498->imageViewFormatReinterpretation,
                _s498->imageViewFormatSwizzle,
                _s498->imageView2DOn3DImage,
                _s498->multisampleArrayImage,
                _s498->mutableComparisonSamplers,
                _s498->pointPolygons,
                _s498->samplerMipLodBias,
                _s498->separateStencilMaskRef,
                _s498->shaderSampleRateInterpolationFunctions,
                _s498->tessellationIsolines,
                _s498->tessellationPointMode,
                _s498->triangleFans,
                _s498->vertexAttributeAccessBeyondStride);
            break;
        }
#endif
#ifdef VK_ENABLE_BETA_EXTENSIONS
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PORTABILITY_SUBSET_PROPERTIES_KHR: {
            const auto _s499 = reinterpret_cast<const VkPhysicalDevicePortabilitySubsetPropertiesKHR *>(header);
            skip |= ValidatePhysicalDevicePortabilitySubsetPropertiesKHR(_parentObjects,
                _s499->sType,
                _s499->pNext,
                _s499->minVertexInputBindingStrideAlignment);
            break;
        }
#endif
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_CLOCK_FEATURES_KHR: {
            const auto _s500 = reinterpret_cast<const VkPhysicalDeviceShaderClockFeaturesKHR *>(header);
            skip |= ValidatePhysicalDeviceShaderClockFeaturesKHR(_parentObjects,
                _s500->sType,
                _s500->pNext,
                _s500->shaderSubgroupClock,
                _s500->shaderDeviceClock);
            break;
        }
        case VK_STRUCTURE_TYPE_VIDEO_DECODE_H265_PROFILE_INFO_KHR: {
            const auto _s501 = reinterpret_cast<const VkVideoDecodeH265ProfileInfoKHR *>(header);
            skip |= ValidateVideoDecodeH265ProfileInfoKHR(_parentObjects,
                _s501->sType,
                _s501->pNext,
                _s501->stdProfileIdc);
            break;
        }
        case VK_STRUCTURE_TYPE_VIDEO_DECODE_H265_SESSION_PARAMETERS_ADD_INFO_KHR: {
            const auto _s502 = reinterpret_cast<const VkVideoDecodeH265SessionParametersAddInfoKHR *>(header);
            skip |= ValidateVideoDecodeH265SessionParametersAddInfoKHR(_parentObjects,
                _s502->sType,
                _s502->pNext,
                _s502->stdVPSCount,
                _s502->pStdVPSs,
                _s502->stdSPSCount,
                _s502->pStdSPSs,
                _s502->stdPPSCount,
                _s502->pStdPPSs);
            break;
        }
        case VK_STRUCTURE_TYPE_VIDEO_DECODE_H265_SESSION_PARAMETERS_CREATE_INFO_KHR: {
            const auto _s503 = reinterpret_cast<const VkVideoDecodeH265SessionParametersCreateInfoKHR *>(header);
            skip |= ValidateVideoDecodeH265SessionParametersCreateInfoKHR(_parentObjects,
                _s503->sType,
                _s503->pNext,
                _s503->maxStdVPSCount,
                _s503->maxStdSPSCount,
                _s503->maxStdPPSCount,
                _s503->pParametersAddInfo);
            break;
        }
        case VK_STRUCTURE_TYPE_VIDEO_DECODE_H265_PICTURE_INFO_KHR: {
            const auto _s504 = reinterpret_cast<const VkVideoDecodeH265PictureInfoKHR *>(header);
            skip |= ValidateVideoDecodeH265PictureInfoKHR(_parentObjects,
                _s504->sType,
                _s504->pNext,
                _s504->pStdPictureInfo,
                _s504->sliceSegmentCount,
                _s504->pSliceSegmentOffsets);
            break;
        }
        case VK_STRUCTURE_TYPE_VIDEO_DECODE_H265_DPB_SLOT_INFO_KHR: {
            const auto _s505 = reinterpret_cast<const VkVideoDecodeH265DpbSlotInfoKHR *>(header);
            skip |= ValidateVideoDecodeH265DpbSlotInfoKHR(_parentObjects,
                _s505->sType,
                _s505->pNext,
                _s505->pStdReferenceInfo);
            break;
        }
        case VK_STRUCTURE_TYPE_DEVICE_QUEUE_GLOBAL_PRIORITY_CREATE_INFO_KHR: {
            const auto _s506 = reinterpret_cast<const VkDeviceQueueGlobalPriorityCreateInfoKHR *>(header);
            skip |= ValidateDeviceQueueGlobalPriorityCreateInfoKHR(_parentObjects,
                _s506->sType,
                _s506->pNext,
                _s506->globalPriority);
            break;
        }
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_GLOBAL_PRIORITY_QUERY_FEATURES_KHR: {
            const auto _s507 = reinterpret_cast<const VkPhysicalDeviceGlobalPriorityQueryFeaturesKHR *>(header);
            skip |= ValidatePhysicalDeviceGlobalPriorityQueryFeaturesKHR(_parentObjects,
                _s507->sType,
                _s507->pNext,
                _s507->globalPriorityQuery);
            break;
        }
        case VK_STRUCTURE_TYPE_QUEUE_FAMILY_GLOBAL_PRIORITY_PROPERTIES_KHR: {
            const auto _s508 = reinterpret_cast<const VkQueueFamilyGlobalPriorityPropertiesKHR *>(header);
            skip |= ValidateQueueFamilyGlobalPriorityPropertiesKHR(_parentObjects,
                _s508->sType,
                _s508->pNext,
                _s508->priorityCount,
                _s508->priorities);
            break;
        }
        case VK_STRUCTURE_TYPE_FRAGMENT_SHADING_RATE_ATTACHMENT_INFO_KHR: {
            const auto _s509 = reinterpret_cast<const VkFragmentShadingRateAttachmentInfoKHR *>(header);
            skip |= ValidateFragmentShadingRateAttachmentInfoKHR(_parentObjects,
                _s509->sType,
                _s509->pNext,
                _s509->pFragmentShadingRateAttachment,
                _s509->shadingRateAttachmentTexelSize);
            break;
        }
        case VK_STRUCTURE_TYPE_PIPELINE_FRAGMENT_SHADING_RATE_STATE_CREATE_INFO_KHR: {
            const auto _s510 = reinterpret_cast<const VkPipelineFragmentShadingRateStateCreateInfoKHR *>(header);
            skip |= ValidatePipelineFragmentShadingRateStateCreateInfoKHR(_parentObjects,
                _s510->sType,
                _s510->pNext,
                _s510->fragmentSize,
                _s510->combinerOps);
            break;
        }
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FRAGMENT_SHADING_RATE_FEATURES_KHR: {
            const auto _s511 = reinterpret_cast<const VkPhysicalDeviceFragmentShadingRateFeaturesKHR *>(header);
            skip |= ValidatePhysicalDeviceFragmentShadingRateFeaturesKHR(_parentObjects,
                _s511->sType,
                _s511->pNext,
                _s511->pipelineFragmentShadingRate,
                _s511->primitiveFragmentShadingRate,
                _s511->attachmentFragmentShadingRate);
            break;
        }
        case VK_STRUCTURE_TYPE_SURFACE_PROTECTED_CAPABILITIES_KHR: {
            const auto _s512 = reinterpret_cast<const VkSurfaceProtectedCapabilitiesKHR *>(header);
            skip |= ValidateSurfaceProtectedCapabilitiesKHR(_parentObjects,
                _s512->sType,
                _s512->pNext,
                _s512->supportsProtected);
            break;
        }
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PRESENT_WAIT_FEATURES_KHR: {
            const auto _s513 = reinterpret_cast<const VkPhysicalDevicePresentWaitFeaturesKHR *>(header);
            skip |= ValidatePhysicalDevicePresentWaitFeaturesKHR(_parentObjects,
                _s513->sType,
                _s513->pNext,
                _s513->presentWait);
            break;
        }
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PIPELINE_EXECUTABLE_PROPERTIES_FEATURES_KHR: {
            const auto _s514 = reinterpret_cast<const VkPhysicalDevicePipelineExecutablePropertiesFeaturesKHR *>(header);
            skip |= ValidatePhysicalDevicePipelineExecutablePropertiesFeaturesKHR(_parentObjects,
                _s514->sType,
                _s514->pNext,
                _s514->pipelineExecutableInfo);
            break;
        }
        case VK_STRUCTURE_TYPE_PIPELINE_LIBRARY_CREATE_INFO_KHR: {
            const auto _s515 = reinterpret_cast<const VkPipelineLibraryCreateInfoKHR *>(header);
            skip |= ValidatePipelineLibraryCreateInfoKHR(_parentObjects,
                _s515->sType,
                _s515->pNext,
                _s515->libraryCount,
                _s515->pLibraries);
            break;
        }
        case VK_STRUCTURE_TYPE_PRESENT_ID_KHR: {
            const auto _s516 = reinterpret_cast<const VkPresentIdKHR *>(header);
            skip |= ValidatePresentIdKHR(_parentObjects,
                _s516->sType,
                _s516->pNext,
                _s516->swapchainCount,
                _s516->pPresentIds);
            break;
        }
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PRESENT_ID_FEATURES_KHR: {
            const auto _s517 = reinterpret_cast<const VkPhysicalDevicePresentIdFeaturesKHR *>(header);
            skip |= ValidatePhysicalDevicePresentIdFeaturesKHR(_parentObjects,
                _s517->sType,
                _s517->pNext,
                _s517->presentId);
            break;
        }
#ifdef VK_ENABLE_BETA_EXTENSIONS
        case VK_STRUCTURE_TYPE_QUERY_POOL_VIDEO_ENCODE_FEEDBACK_CREATE_INFO_KHR: {
            const auto _s518 = reinterpret_cast<const VkQueryPoolVideoEncodeFeedbackCreateInfoKHR *>(header);
            skip |= ValidateQueryPoolVideoEncodeFeedbackCreateInfoKHR(_parentObjects,
                _s518->sType,
                _s518->pNext,
                _s518->encodeFeedbackFlags);
            break;
        }
#endif
#ifdef VK_ENABLE_BETA_EXTENSIONS
        case VK_STRUCTURE_TYPE_VIDEO_ENCODE_USAGE_INFO_KHR: {
            const auto _s519 = reinterpret_cast<const VkVideoEncodeUsageInfoKHR *>(header);
            skip |= ValidateVideoEncodeUsageInfoKHR(_parentObjects,
                _s519->sType,
                _s519->pNext,
                _s519->videoUsageHints,
                _s519->videoContentHints,
                _s519->tuningMode);
            break;
        }
#endif
#ifdef VK_ENABLE_BETA_EXTENSIONS
        case VK_STRUCTURE_TYPE_VIDEO_ENCODE_RATE_CONTROL_LAYER_INFO_KHR: {
            const auto _s520 = reinterpret_cast<const VkVideoEncodeRateControlLayerInfoKHR *>(header);
            skip |= ValidateVideoEncodeRateControlLayerInfoKHR(_parentObjects,
                _s520->sType,
                _s520->pNext,
                _s520->averageBitrate,
                _s520->maxBitrate,
                _s520->frameRateNumerator,
                _s520->frameRateDenominator,
                _s520->virtualBufferSizeInMs,
                _s520->initialVirtualBufferSizeInMs);
            break;
        }
#endif
#ifdef VK_ENABLE_BETA_EXTENSIONS
        case VK_STRUCTURE_TYPE_VIDEO_ENCODE_RATE_CONTROL_INFO_KHR: {
            const auto _s521 = reinterpret_cast<const VkVideoEncodeRateControlInfoKHR *>(header);
            skip |= ValidateVideoEncodeRateControlInfoKHR(_parentObjects,
                _s521->sType,
                _s521->pNext,
                _s521->flags,
                _s521->rateControlMode,
                _s521->layerCount,
                _s521->pLayers);
            break;
        }
#endif
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FRAGMENT_SHADER_BARYCENTRIC_FEATURES_KHR: {
            const auto _s522 = reinterpret_cast<const VkPhysicalDeviceFragmentShaderBarycentricFeaturesKHR *>(header);
            skip |= ValidatePhysicalDeviceFragmentShaderBarycentricFeaturesKHR(_parentObjects,
                _s522->sType,
                _s522->pNext,
                _s522->fragmentShaderBarycentric);
            break;
        }
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_SUBGROUP_UNIFORM_CONTROL_FLOW_FEATURES_KHR: {
            const auto _s523 = reinterpret_cast<const VkPhysicalDeviceShaderSubgroupUniformControlFlowFeaturesKHR *>(header);
            skip |= ValidatePhysicalDeviceShaderSubgroupUniformControlFlowFeaturesKHR(_parentObjects,
                _s523->sType,
                _s523->pNext,
                _s523->shaderSubgroupUniformControlFlow);
            break;
        }
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_WORKGROUP_MEMORY_EXPLICIT_LAYOUT_FEATURES_KHR: {
            const auto _s524 = reinterpret_cast<const VkPhysicalDeviceWorkgroupMemoryExplicitLayoutFeaturesKHR *>(header);
            skip |= ValidatePhysicalDeviceWorkgroupMemoryExplicitLayoutFeaturesKHR(_parentObjects,
                _s524->sType,
                _s524->pNext,
                _s524->workgroupMemoryExplicitLayout,
                _s524->workgroupMemoryExplicitLayoutScalarBlockLayout,
                _s524->workgroupMemoryExplicitLayout8BitAccess,
                _s524->workgroupMemoryExplicitLayout16BitAccess);
            break;
        }
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_MAINTENANCE_1_FEATURES_KHR: {
            const auto _s525 = reinterpret_cast<const VkPhysicalDeviceRayTracingMaintenance1FeaturesKHR *>(header);
            skip |= ValidatePhysicalDeviceRayTracingMaintenance1FeaturesKHR(_parentObjects,
                _s525->sType,
                _s525->pNext,
                _s525->rayTracingMaintenance1,
                _s525->rayTracingPipelineTraceRaysIndirect2);
            break;
        }
        case VK_STRUCTURE_TYPE_DEBUG_REPORT_CALLBACK_CREATE_INFO_EXT: {
            const auto _s526 = reinterpret_cast<const VkDebugReportCallbackCreateInfoEXT *>(header);
            skip |= ValidateDebugReportCallbackCreateInfoEXT(_parentObjects,
                _s526->sType,
                _s526->pNext,
                _s526->flags,
                _s526->pfnCallback,
                _s526->pUserData);
            break;
        }
        case VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_RASTERIZATION_ORDER_AMD: {
            const auto _s527 = reinterpret_cast<const VkPipelineRasterizationStateRasterizationOrderAMD *>(header);
            skip |= ValidatePipelineRasterizationStateRasterizationOrderAMD(_parentObjects,
                _s527->sType,
                _s527->pNext,
                _s527->rasterizationOrder);
            break;
        }
        case VK_STRUCTURE_TYPE_DEDICATED_ALLOCATION_IMAGE_CREATE_INFO_NV: {
            const auto _s528 = reinterpret_cast<const VkDedicatedAllocationImageCreateInfoNV *>(header);
            skip |= ValidateDedicatedAllocationImageCreateInfoNV(_parentObjects,
                _s528->sType,
                _s528->pNext,
                _s528->dedicatedAllocation);
            break;
        }
        case VK_STRUCTURE_TYPE_DEDICATED_ALLOCATION_BUFFER_CREATE_INFO_NV: {
            const auto _s529 = reinterpret_cast<const VkDedicatedAllocationBufferCreateInfoNV *>(header);
            skip |= ValidateDedicatedAllocationBufferCreateInfoNV(_parentObjects,
                _s529->sType,
                _s529->pNext,
                _s529->dedicatedAllocation);
            break;
        }
        case VK_STRUCTURE_TYPE_DEDICATED_ALLOCATION_MEMORY_ALLOCATE_INFO_NV: {
            const auto _s530 = reinterpret_cast<const VkDedicatedAllocationMemoryAllocateInfoNV *>(header);
            skip |= ValidateDedicatedAllocationMemoryAllocateInfoNV(_parentObjects,
                _s530->sType,
                _s530->pNext,
                _s530->image,
                _s530->buffer);
            break;
        }
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_TRANSFORM_FEEDBACK_FEATURES_EXT: {
            const auto _s531 = reinterpret_cast<const VkPhysicalDeviceTransformFeedbackFeaturesEXT *>(header);
            skip |= ValidatePhysicalDeviceTransformFeedbackFeaturesEXT(_parentObjects,
                _s531->sType,
                _s531->pNext,
                _s531->transformFeedback,
                _s531->geometryStreams);
            break;
        }
        case VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_STREAM_CREATE_INFO_EXT: {
            const auto _s532 = reinterpret_cast<const VkPipelineRasterizationStateStreamCreateInfoEXT *>(header);
            skip |= ValidatePipelineRasterizationStateStreamCreateInfoEXT(_parentObjects,
                _s532->sType,
                _s532->pNext,
                _s532->flags,
                _s532->rasterizationStream);
            break;
        }
#ifdef VK_ENABLE_BETA_EXTENSIONS
        case VK_STRUCTURE_TYPE_VIDEO_ENCODE_H264_SESSION_PARAMETERS_ADD_INFO_EXT: {
            const auto _s533 = reinterpret_cast<const VkVideoEncodeH264SessionParametersAddInfoEXT *>(header);
            skip |= ValidateVideoEncodeH264SessionParametersAddInfoEXT(_parentObjects,
                _s533->sType,
                _s533->pNext,
                _s533->stdSPSCount,
                _s533->pStdSPSs,
                _s533->stdPPSCount,
                _s533->pStdPPSs);
            break;
        }
#endif
#ifdef VK_ENABLE_BETA_EXTENSIONS
        case VK_STRUCTURE_TYPE_VIDEO_ENCODE_H264_SESSION_PARAMETERS_CREATE_INFO_EXT: {
            const auto _s534 = reinterpret_cast<const VkVideoEncodeH264SessionParametersCreateInfoEXT *>(header);
            skip |= ValidateVideoEncodeH264SessionParametersCreateInfoEXT(_parentObjects,
                _s534->sType,
                _s534->pNext,
                _s534->maxStdSPSCount,
                _s534->maxStdPPSCount,
                _s534->pParametersAddInfo);
            break;
        }
#endif
#ifdef VK_ENABLE_BETA_EXTENSIONS
        case VK_STRUCTURE_TYPE_VIDEO_ENCODE_H264_VCL_FRAME_INFO_EXT: {
            const auto _s535 = reinterpret_cast<const VkVideoEncodeH264VclFrameInfoEXT *>(header);
            skip |= ValidateVideoEncodeH264VclFrameInfoEXT(_parentObjects,
                _s535->sType,
                _s535->pNext,
                _s535->pStdReferenceFinalLists,
                _s535->naluSliceEntryCount,
                _s535->pNaluSliceEntries,
                _s535->pStdPictureInfo);
            break;
        }
#endif
#ifdef VK_ENABLE_BETA_EXTENSIONS
        case VK_STRUCTURE_TYPE_VIDEO_ENCODE_H264_DPB_SLOT_INFO_EXT: {
            const auto _s536 = reinterpret_cast<const VkVideoEncodeH264DpbSlotInfoEXT *>(header);
            skip |= ValidateVideoEncodeH264DpbSlotInfoEXT(_parentObjects,
                _s536->sType,
                _s536->pNext,
                _s536->pStdReferenceInfo);
            break;
        }
#endif
#ifdef VK_ENABLE_BETA_EXTENSIONS
        case VK_STRUCTURE_TYPE_VIDEO_ENCODE_H264_PROFILE_INFO_EXT: {
            const auto _s537 = reinterpret_cast<const VkVideoEncodeH264ProfileInfoEXT *>(header);
            skip |= ValidateVideoEncodeH264ProfileInfoEXT(_parentObjects,
                _s537->sType,
                _s537->pNext,
                _s537->stdProfileIdc);
            break;
        }
#endif
#ifdef VK_ENABLE_BETA_EXTENSIONS
        case VK_STRUCTURE_TYPE_VIDEO_ENCODE_H264_RATE_CONTROL_INFO_EXT: {
            const auto _s538 = reinterpret_cast<const VkVideoEncodeH264RateControlInfoEXT *>(header);
            skip |= ValidateVideoEncodeH264RateControlInfoEXT(_parentObjects,
                _s538->sType,
                _s538->pNext,
                _s538->gopFrameCount,
                _s538->idrPeriod,
                _s538->consecutiveBFrameCount,
                _s538->rateControlStructure,
                _s538->temporalLayerCount);
            break;
        }
#endif
#ifdef VK_ENABLE_BETA_EXTENSIONS
        case VK_STRUCTURE_TYPE_VIDEO_ENCODE_H264_RATE_CONTROL_LAYER_INFO_EXT: {
            const auto _s539 = reinterpret_cast<const VkVideoEncodeH264RateControlLayerInfoEXT *>(header);
            skip |= ValidateVideoEncodeH264RateControlLayerInfoEXT(_parentObjects,
                _s539->sType,
                _s539->pNext,
                _s539->temporalLayerId,
                _s539->useInitialRcQp,
                _s539->initialRcQp,
                _s539->useMinQp,
                _s539->minQp,
                _s539->useMaxQp,
                _s539->maxQp,
                _s539->useMaxFrameSize,
                _s539->maxFrameSize);
            break;
        }
#endif
#ifdef VK_ENABLE_BETA_EXTENSIONS
        case VK_STRUCTURE_TYPE_VIDEO_ENCODE_H265_SESSION_PARAMETERS_ADD_INFO_EXT: {
            const auto _s540 = reinterpret_cast<const VkVideoEncodeH265SessionParametersAddInfoEXT *>(header);
            skip |= ValidateVideoEncodeH265SessionParametersAddInfoEXT(_parentObjects,
                _s540->sType,
                _s540->pNext,
                _s540->stdVPSCount,
                _s540->pStdVPSs,
                _s540->stdSPSCount,
                _s540->pStdSPSs,
                _s540->stdPPSCount,
                _s540->pStdPPSs);
            break;
        }
#endif
#ifdef VK_ENABLE_BETA_EXTENSIONS
        case VK_STRUCTURE_TYPE_VIDEO_ENCODE_H265_SESSION_PARAMETERS_CREATE_INFO_EXT: {
            const auto _s541 = reinterpret_cast<const VkVideoEncodeH265SessionParametersCreateInfoEXT *>(header);
            skip |= ValidateVideoEncodeH265SessionParametersCreateInfoEXT(_parentObjects,
                _s541->sType,
                _s541->pNext,
                _s541->maxStdVPSCount,
                _s541->maxStdSPSCount,
                _s541->maxStdPPSCount,
                _s541->pParametersAddInfo);
            break;
        }
#endif
#ifdef VK_ENABLE_BETA_EXTENSIONS
        case VK_STRUCTURE_TYPE_VIDEO_ENCODE_H265_VCL_FRAME_INFO_EXT: {
            const auto _s542 = reinterpret_cast<const VkVideoEncodeH265VclFrameInfoEXT *>(header);
            skip |= ValidateVideoEncodeH265VclFrameInfoEXT(_parentObjects,
                _s542->sType,
                _s542->pNext,
                _s542->pStdReferenceFinalLists,
                _s542->naluSliceSegmentEntryCount,
                _s542->pNaluSliceSegmentEntries,
                _s542->pStdPictureInfo);
            break;
        }
#endif
#ifdef VK_ENABLE_BETA_EXTENSIONS
        case VK_STRUCTURE_TYPE_VIDEO_ENCODE_H265_DPB_SLOT_INFO_EXT: {
            const auto _s543 = reinterpret_cast<const VkVideoEncodeH265DpbSlotInfoEXT *>(header);
            skip |= ValidateVideoEncodeH265DpbSlotInfoEXT(_parentObjects,
                _s543->sType,
                _s543->pNext,
                _s543->pStdReferenceInfo);
            break;
        }
#endif
#ifdef VK_ENABLE_BETA_EXTENSIONS
        case VK_STRUCTURE_TYPE_VIDEO_ENCODE_H265_PROFILE_INFO_EXT: {
            const auto _s544 = reinterpret_cast<const VkVideoEncodeH265ProfileInfoEXT *>(header);
            skip |= ValidateVideoEncodeH265ProfileInfoEXT(_parentObjects,
                _s544->sType,
                _s544->pNext,
                _s544->stdProfileIdc);
            break;
        }
#endif
#ifdef VK_ENABLE_BETA_EXTENSIONS
        case VK_STRUCTURE_TYPE_VIDEO_ENCODE_H265_RATE_CONTROL_INFO_EXT: {
            const auto _s545 = reinterpret_cast<const VkVideoEncodeH265RateControlInfoEXT *>(header);
            skip |= ValidateVideoEncodeH265RateControlInfoEXT(_parentObjects,
                _s545->sType,
                _s545->pNext,
                _s545->gopFrameCount,
                _s545->idrPeriod,
                _s545->consecutiveBFrameCount,
                _s545->rateControlStructure,
                _s545->subLayerCount);
            break;
        }
#endif
#ifdef VK_ENABLE_BETA_EXTENSIONS
        case VK_STRUCTURE_TYPE_VIDEO_ENCODE_H265_RATE_CONTROL_LAYER_INFO_EXT: {
            const auto _s546 = reinterpret_cast<const VkVideoEncodeH265RateControlLayerInfoEXT *>(header);
            skip |= ValidateVideoEncodeH265RateControlLayerInfoEXT(_parentObjects,
                _s546->sType,
                _s546->pNext,
                _s546->temporalId,
                _s546->useInitialRcQp,
                _s546->initialRcQp,
                _s546->useMinQp,
                _s546->minQp,
                _s546->useMaxQp,
                _s546->maxQp,
                _s546->useMaxFrameSize,
                _s546->maxFrameSize);
            break;
        }
#endif
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_CORNER_SAMPLED_IMAGE_FEATURES_NV: {
            const auto _s547 = reinterpret_cast<const VkPhysicalDeviceCornerSampledImageFeaturesNV *>(header);
            skip |= ValidatePhysicalDeviceCornerSampledImageFeaturesNV(_parentObjects,
                _s547->sType,
                _s547->pNext,
                _s547->cornerSampledImage);
            break;
        }
        case VK_STRUCTURE_TYPE_EXTERNAL_MEMORY_IMAGE_CREATE_INFO_NV: {
            const auto _s548 = reinterpret_cast<const VkExternalMemoryImageCreateInfoNV *>(header);
            skip |= ValidateExternalMemoryImageCreateInfoNV(_parentObjects,
                _s548->sType,
                _s548->pNext,
                _s548->handleTypes);
            break;
        }
        case VK_STRUCTURE_TYPE_EXPORT_MEMORY_ALLOCATE_INFO_NV: {
            const auto _s549 = reinterpret_cast<const VkExportMemoryAllocateInfoNV *>(header);
            skip |= ValidateExportMemoryAllocateInfoNV(_parentObjects,
                _s549->sType,
                _s549->pNext,
                _s549->handleTypes);
            break;
        }
#ifdef VK_USE_PLATFORM_WIN32_KHR
        case VK_STRUCTURE_TYPE_IMPORT_MEMORY_WIN32_HANDLE_INFO_NV: {
            const auto _s550 = reinterpret_cast<const VkImportMemoryWin32HandleInfoNV *>(header);
            skip |= ValidateImportMemoryWin32HandleInfoNV(_parentObjects,
                _s550->sType,
                _s550->pNext,
                _s550->handleType,
                _s550->handle);
            break;
        }
#endif
#ifdef VK_USE_PLATFORM_WIN32_KHR
        case VK_STRUCTURE_TYPE_EXPORT_MEMORY_WIN32_HANDLE_INFO_NV: {
            const auto _s551 = reinterpret_cast<const VkExportMemoryWin32HandleInfoNV *>(header);
            skip |= ValidateExportMemoryWin32HandleInfoNV(_parentObjects,
                _s551->sType,
                _s551->pNext,
                _s551->pAttributes,
                _s551->dwAccess);
            break;
        }
#endif
#ifdef VK_USE_PLATFORM_WIN32_KHR
        case VK_STRUCTURE_TYPE_WIN32_KEYED_MUTEX_ACQUIRE_RELEASE_INFO_NV: {
            const auto _s552 = reinterpret_cast<const VkWin32KeyedMutexAcquireReleaseInfoNV *>(header);
            skip |= ValidateWin32KeyedMutexAcquireReleaseInfoNV(_parentObjects,
                _s552->sType,
                _s552->pNext,
                _s552->acquireCount,
                _s552->pAcquireSyncs,
                _s552->pAcquireKeys,
                _s552->pAcquireTimeoutMilliseconds,
                _s552->releaseCount,
                _s552->pReleaseSyncs,
                _s552->pReleaseKeys);
            break;
        }
#endif
        case VK_STRUCTURE_TYPE_VALIDATION_FLAGS_EXT: {
            const auto _s553 = reinterpret_cast<const VkValidationFlagsEXT *>(header);
            skip |= ValidateValidationFlagsEXT(_parentObjects,
                _s553->sType,
                _s553->pNext,
                _s553->disabledValidationCheckCount,
                _s553->pDisabledValidationChecks);
            break;
        }
        case VK_STRUCTURE_TYPE_IMAGE_VIEW_ASTC_DECODE_MODE_EXT: {
            const auto _s554 = reinterpret_cast<const VkImageViewASTCDecodeModeEXT *>(header);
            skip |= ValidateImageViewASTCDecodeModeEXT(_parentObjects,
                _s554->sType,
                _s554->pNext,
                _s554->decodeMode);
            break;
        }
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ASTC_DECODE_FEATURES_EXT: {
            const auto _s555 = reinterpret_cast<const VkPhysicalDeviceASTCDecodeFeaturesEXT *>(header);
            skip |= ValidatePhysicalDeviceASTCDecodeFeaturesEXT(_parentObjects,
                _s555->sType,
                _s555->pNext,
                _s555->decodeModeSharedExponent);
            break;
        }
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PIPELINE_ROBUSTNESS_FEATURES_EXT: {
            const auto _s556 = reinterpret_cast<const VkPhysicalDevicePipelineRobustnessFeaturesEXT *>(header);
            skip |= ValidatePhysicalDevicePipelineRobustnessFeaturesEXT(_parentObjects,
                _s556->sType,
                _s556->pNext,
                _s556->pipelineRobustness);
            break;
        }
        case VK_STRUCTURE_TYPE_PIPELINE_ROBUSTNESS_CREATE_INFO_EXT: {
            const auto _s557 = reinterpret_cast<const VkPipelineRobustnessCreateInfoEXT *>(header);
            skip |= ValidatePipelineRobustnessCreateInfoEXT(_parentObjects,
                _s557->sType,
                _s557->pNext,
                _s557->storageBuffers,
                _s557->uniformBuffers,
                _s557->vertexInputs,
                _s557->images);
            break;
        }
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_CONDITIONAL_RENDERING_FEATURES_EXT: {
            const auto _s558 = reinterpret_cast<const VkPhysicalDeviceConditionalRenderingFeaturesEXT *>(header);
            skip |= ValidatePhysicalDeviceConditionalRenderingFeaturesEXT(_parentObjects,
                _s558->sType,
                _s558->pNext,
                _s558->conditionalRendering,
                _s558->inheritedConditionalRendering);
            break;
        }
        case VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_CONDITIONAL_RENDERING_INFO_EXT: {
            const auto _s559 = reinterpret_cast<const VkCommandBufferInheritanceConditionalRenderingInfoEXT *>(header);
            skip |= ValidateCommandBufferInheritanceConditionalRenderingInfoEXT(_parentObjects,
                _s559->sType,
                _s559->pNext,
                _s559->conditionalRenderingEnable);
            break;
        }
        case VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_W_SCALING_STATE_CREATE_INFO_NV: {
            const auto _s560 = reinterpret_cast<const VkPipelineViewportWScalingStateCreateInfoNV *>(header);
            skip |= ValidatePipelineViewportWScalingStateCreateInfoNV(_parentObjects,
                _s560->sType,
                _s560->pNext,
                _s560->viewportWScalingEnable,
                _s560->viewportCount,
                _s560->pViewportWScalings);
            break;
        }
        case VK_STRUCTURE_TYPE_SWAPCHAIN_COUNTER_CREATE_INFO_EXT: {
            const auto _s561 = reinterpret_cast<const VkSwapchainCounterCreateInfoEXT *>(header);
            skip |= ValidateSwapchainCounterCreateInfoEXT(_parentObjects,
                _s561->sType,
                _s561->pNext,
                _s561->surfaceCounters);
            break;
        }
        case VK_STRUCTURE_TYPE_PRESENT_TIMES_INFO_GOOGLE: {
            const auto _s562 = reinterpret_cast<const VkPresentTimesInfoGOOGLE *>(header);
            skip |= ValidatePresentTimesInfoGOOGLE(_parentObjects,
                _s562->sType,
                _s562->pNext,
                _s562->swapchainCount,
                _s562->pTimes);
            break;
        }
        case VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_SWIZZLE_STATE_CREATE_INFO_NV: {
            const auto _s563 = reinterpret_cast<const VkPipelineViewportSwizzleStateCreateInfoNV *>(header);
            skip |= ValidatePipelineViewportSwizzleStateCreateInfoNV(_parentObjects,
                _s563->sType,
                _s563->pNext,
                _s563->flags,
                _s563->viewportCount,
                _s563->pViewportSwizzles);
            break;
        }
        case VK_STRUCTURE_TYPE_PIPELINE_DISCARD_RECTANGLE_STATE_CREATE_INFO_EXT: {
            const auto _s564 = reinterpret_cast<const VkPipelineDiscardRectangleStateCreateInfoEXT *>(header);
            skip |= ValidatePipelineDiscardRectangleStateCreateInfoEXT(_parentObjects,
                _s564->sType,
                _s564->pNext,
                _s564->flags,
                _s564->discardRectangleMode,
                _s564->discardRectangleCount,
                _s564->pDiscardRectangles);
            break;
        }
        case VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_CONSERVATIVE_STATE_CREATE_INFO_EXT: {
            const auto _s565 = reinterpret_cast<const VkPipelineRasterizationConservativeStateCreateInfoEXT *>(header);
            skip |= ValidatePipelineRasterizationConservativeStateCreateInfoEXT(_parentObjects,
                _s565->sType,
                _s565->pNext,
                _s565->flags,
                _s565->conservativeRasterizationMode,
                _s565->extraPrimitiveOverestimationSize);
            break;
        }
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DEPTH_CLIP_ENABLE_FEATURES_EXT: {
            const auto _s566 = reinterpret_cast<const VkPhysicalDeviceDepthClipEnableFeaturesEXT *>(header);
            skip |= ValidatePhysicalDeviceDepthClipEnableFeaturesEXT(_parentObjects,
                _s566->sType,
                _s566->pNext,
                _s566->depthClipEnable);
            break;
        }
        case VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_DEPTH_CLIP_STATE_CREATE_INFO_EXT: {
            const auto _s567 = reinterpret_cast<const VkPipelineRasterizationDepthClipStateCreateInfoEXT *>(header);
            skip |= ValidatePipelineRasterizationDepthClipStateCreateInfoEXT(_parentObjects,
                _s567->sType,
                _s567->pNext,
                _s567->flags,
                _s567->depthClipEnable);
            break;
        }
        case VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT: {
            const auto _s568 = reinterpret_cast<const VkDebugUtilsObjectNameInfoEXT *>(header);
            skip |= ValidateDebugUtilsObjectNameInfoEXT(_parentObjects,
                _s568->sType,
                _s568->pNext,
                _s568->objectType,
                _s568->objectHandle,
                _s568->pObjectName);
            break;
        }
        case VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT: {
            const auto _s569 = reinterpret_cast<const VkDebugUtilsMessengerCreateInfoEXT *>(header);
            skip |= ValidateDebugUtilsMessengerCreateInfoEXT(_parentObjects,
                _s569->sType,
                _s569->pNext,
                _s569->flags,
                _s569->messageSeverity,
                _s569->messageType,
                _s569->pfnUserCallback,
                _s569->pUserData);
            break;
        }
#ifdef VK_USE_PLATFORM_ANDROID_KHR
        case VK_STRUCTURE_TYPE_IMPORT_ANDROID_HARDWARE_BUFFER_INFO_ANDROID: {
            const auto _s570 = reinterpret_cast<const VkImportAndroidHardwareBufferInfoANDROID *>(header);
            skip |= ValidateImportAndroidHardwareBufferInfoANDROID(_parentObjects,
                _s570->sType,
                _s570->pNext,
                _s570->buffer);
            break;
        }
#endif
#ifdef VK_USE_PLATFORM_ANDROID_KHR
        case VK_STRUCTURE_TYPE_EXTERNAL_FORMAT_ANDROID: {
            const auto _s571 = reinterpret_cast<const VkExternalFormatANDROID *>(header);
            skip |= ValidateExternalFormatANDROID(_parentObjects,
                _s571->sType,
                _s571->pNext,
                _s571->externalFormat);
            break;
        }
#endif
        case VK_STRUCTURE_TYPE_SAMPLE_LOCATIONS_INFO_EXT: {
            const auto _s572 = reinterpret_cast<const VkSampleLocationsInfoEXT *>(header);
            skip |= ValidateSampleLocationsInfoEXT(_parentObjects,
                _s572->sType,
                _s572->pNext,
                _s572->sampleLocationsPerPixel,
                _s572->sampleLocationGridSize,
                _s572->sampleLocationsCount,
                _s572->pSampleLocations);
            break;
        }
        case VK_STRUCTURE_TYPE_RENDER_PASS_SAMPLE_LOCATIONS_BEGIN_INFO_EXT: {
            const auto _s573 = reinterpret_cast<const VkRenderPassSampleLocationsBeginInfoEXT *>(header);
            skip |= ValidateRenderPassSampleLocationsBeginInfoEXT(_parentObjects,
                _s573->sType,
                _s573->pNext,
                _s573->attachmentInitialSampleLocationsCount,
                _s573->pAttachmentInitialSampleLocations,
                _s573->postSubpassSampleLocationsCount,
                _s573->pPostSubpassSampleLocations);
            break;
        }
        case VK_STRUCTURE_TYPE_PIPELINE_SAMPLE_LOCATIONS_STATE_CREATE_INFO_EXT: {
            const auto _s574 = reinterpret_cast<const VkPipelineSampleLocationsStateCreateInfoEXT *>(header);
            skip |= ValidatePipelineSampleLocationsStateCreateInfoEXT(_parentObjects,
                _s574->sType,
                _s574->pNext,
                _s574->sampleLocationsEnable,
                _s574->sampleLocationsInfo);
            break;
        }
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_BLEND_OPERATION_ADVANCED_FEATURES_EXT: {
            const auto _s575 = reinterpret_cast<const VkPhysicalDeviceBlendOperationAdvancedFeaturesEXT *>(header);
            skip |= ValidatePhysicalDeviceBlendOperationAdvancedFeaturesEXT(_parentObjects,
                _s575->sType,
                _s575->pNext,
                _s575->advancedBlendCoherentOperations);
            break;
        }
        case VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_ADVANCED_STATE_CREATE_INFO_EXT: {
            const auto _s576 = reinterpret_cast<const VkPipelineColorBlendAdvancedStateCreateInfoEXT *>(header);
            skip |= ValidatePipelineColorBlendAdvancedStateCreateInfoEXT(_parentObjects,
                _s576->sType,
                _s576->pNext,
                _s576->srcPremultiplied,
                _s576->dstPremultiplied,
                _s576->blendOverlap);
            break;
        }
        case VK_STRUCTURE_TYPE_PIPELINE_COVERAGE_TO_COLOR_STATE_CREATE_INFO_NV: {
            const auto _s577 = reinterpret_cast<const VkPipelineCoverageToColorStateCreateInfoNV *>(header);
            skip |= ValidatePipelineCoverageToColorStateCreateInfoNV(_parentObjects,
                _s577->sType,
                _s577->pNext,
                _s577->flags,
                _s577->coverageToColorEnable,
                _s577->coverageToColorLocation);
            break;
        }
        case VK_STRUCTURE_TYPE_PIPELINE_COVERAGE_MODULATION_STATE_CREATE_INFO_NV: {
            const auto _s578 = reinterpret_cast<const VkPipelineCoverageModulationStateCreateInfoNV *>(header);
            skip |= ValidatePipelineCoverageModulationStateCreateInfoNV(_parentObjects,
                _s578->sType,
                _s578->pNext,
                _s578->flags,
                _s578->coverageModulationMode,
                _s578->coverageModulationTableEnable,
                _s578->coverageModulationTableCount,
                _s578->pCoverageModulationTable);
            break;
        }
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_SM_BUILTINS_FEATURES_NV: {
            const auto _s579 = reinterpret_cast<const VkPhysicalDeviceShaderSMBuiltinsFeaturesNV *>(header);
            skip |= ValidatePhysicalDeviceShaderSMBuiltinsFeaturesNV(_parentObjects,
                _s579->sType,
                _s579->pNext,
                _s579->shaderSMBuiltins);
            break;
        }
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_IMAGE_DRM_FORMAT_MODIFIER_INFO_EXT: {
            const auto _s580 = reinterpret_cast<const VkPhysicalDeviceImageDrmFormatModifierInfoEXT *>(header);
            skip |= ValidatePhysicalDeviceImageDrmFormatModifierInfoEXT(_parentObjects,
                _s580->sType,
                _s580->pNext,
                _s580->drmFormatModifier,
                _s580->sharingMode,
                _s580->queueFamilyIndexCount,
                _s580->pQueueFamilyIndices);
            break;
        }
        case VK_STRUCTURE_TYPE_IMAGE_DRM_FORMAT_MODIFIER_LIST_CREATE_INFO_EXT: {
            const auto _s581 = reinterpret_cast<const VkImageDrmFormatModifierListCreateInfoEXT *>(header);
            skip |= ValidateImageDrmFormatModifierListCreateInfoEXT(_parentObjects,
                _s581->sType,
                _s581->pNext,
                _s581->drmFormatModifierCount,
                _s581->pDrmFormatModifiers);
            break;
        }
        case VK_STRUCTURE_TYPE_IMAGE_DRM_FORMAT_MODIFIER_EXPLICIT_CREATE_INFO_EXT: {
            const auto _s582 = reinterpret_cast<const VkImageDrmFormatModifierExplicitCreateInfoEXT *>(header);
            skip |= ValidateImageDrmFormatModifierExplicitCreateInfoEXT(_parentObjects,
                _s582->sType,
                _s582->pNext,
                _s582->drmFormatModifier,
                _s582->drmFormatModifierPlaneCount,
                _s582->pPlaneLayouts);
            break;
        }
        case VK_STRUCTURE_TYPE_SHADER_MODULE_VALIDATION_CACHE_CREATE_INFO_EXT: {
            const auto _s583 = reinterpret_cast<const VkShaderModuleValidationCacheCreateInfoEXT *>(header);
            skip |= ValidateShaderModuleValidationCacheCreateInfoEXT(_parentObjects,
                _s583->sType,
                _s583->pNext,
                _s583->validationCache);
            break;
        }
        case VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_SHADING_RATE_IMAGE_STATE_CREATE_INFO_NV: {
            const auto _s584 = reinterpret_cast<const VkPipelineViewportShadingRateImageStateCreateInfoNV *>(header);
            skip |= ValidatePipelineViewportShadingRateImageStateCreateInfoNV(_parentObjects,
                _s584->sType,
                _s584->pNext,
                _s584->shadingRateImageEnable,
                _s584->viewportCount,
                _s584->pShadingRatePalettes);
            break;
        }
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADING_RATE_IMAGE_FEATURES_NV: {
            const auto _s585 = reinterpret_cast<const VkPhysicalDeviceShadingRateImageFeaturesNV *>(header);
            skip |= ValidatePhysicalDeviceShadingRateImageFeaturesNV(_parentObjects,
                _s585->sType,
                _s585->pNext,
                _s585->shadingRateImage,
                _s585->shadingRateCoarseSampleOrder);
            break;
        }
        case VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_COARSE_SAMPLE_ORDER_STATE_CREATE_INFO_NV: {
            const auto _s586 = reinterpret_cast<const VkPipelineViewportCoarseSampleOrderStateCreateInfoNV *>(header);
            skip |= ValidatePipelineViewportCoarseSampleOrderStateCreateInfoNV(_parentObjects,
                _s586->sType,
                _s586->pNext,
                _s586->sampleOrderType,
                _s586->customSampleOrderCount,
                _s586->pCustomSampleOrders);
            break;
        }
        case VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET_ACCELERATION_STRUCTURE_NV: {
            const auto _s587 = reinterpret_cast<const VkWriteDescriptorSetAccelerationStructureNV *>(header);
            skip |= ValidateWriteDescriptorSetAccelerationStructureNV(_parentObjects,
                _s587->sType,
                _s587->pNext,
                _s587->accelerationStructureCount,
                _s587->pAccelerationStructures);
            break;
        }
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_REPRESENTATIVE_FRAGMENT_TEST_FEATURES_NV: {
            const auto _s588 = reinterpret_cast<const VkPhysicalDeviceRepresentativeFragmentTestFeaturesNV *>(header);
            skip |= ValidatePhysicalDeviceRepresentativeFragmentTestFeaturesNV(_parentObjects,
                _s588->sType,
                _s588->pNext,
                _s588->representativeFragmentTest);
            break;
        }
        case VK_STRUCTURE_TYPE_PIPELINE_REPRESENTATIVE_FRAGMENT_TEST_STATE_CREATE_INFO_NV: {
            const auto _s589 = reinterpret_cast<const VkPipelineRepresentativeFragmentTestStateCreateInfoNV *>(header);
            skip |= ValidatePipelineRepresentativeFragmentTestStateCreateInfoNV(_parentObjects,
                _s589->sType,
                _s589->pNext,
                _s589->representativeFragmentTestEnable);
            break;
        }
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_IMAGE_VIEW_IMAGE_FORMAT_INFO_EXT: {
            const auto _s590 = reinterpret_cast<const VkPhysicalDeviceImageViewImageFormatInfoEXT *>(header);
            skip |= ValidatePhysicalDeviceImageViewImageFormatInfoEXT(_parentObjects,
                _s590->sType,
                _s590->pNext,
                _s590->imageViewType);
            break;
        }
        case VK_STRUCTURE_TYPE_IMPORT_MEMORY_HOST_POINTER_INFO_EXT: {
            const auto _s591 = reinterpret_cast<const VkImportMemoryHostPointerInfoEXT *>(header);
            skip |= ValidateImportMemoryHostPointerInfoEXT(_parentObjects,
                _s591->sType,
                _s591->pNext,
                _s591->handleType,
                _s591->pHostPointer);
            break;
        }
        case VK_STRUCTURE_TYPE_PIPELINE_COMPILER_CONTROL_CREATE_INFO_AMD: {
            const auto _s592 = reinterpret_cast<const VkPipelineCompilerControlCreateInfoAMD *>(header);
            skip |= ValidatePipelineCompilerControlCreateInfoAMD(_parentObjects,
                _s592->sType,
                _s592->pNext,
                _s592->compilerControlFlags);
            break;
        }
        case VK_STRUCTURE_TYPE_DEVICE_MEMORY_OVERALLOCATION_CREATE_INFO_AMD: {
            const auto _s593 = reinterpret_cast<const VkDeviceMemoryOverallocationCreateInfoAMD *>(header);
            skip |= ValidateDeviceMemoryOverallocationCreateInfoAMD(_parentObjects,
                _s593->sType,
                _s593->pNext,
                _s593->overallocationBehavior);
            break;
        }
        case VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_DIVISOR_STATE_CREATE_INFO_EXT: {
            const auto _s594 = reinterpret_cast<const VkPipelineVertexInputDivisorStateCreateInfoEXT *>(header);
            skip |= ValidatePipelineVertexInputDivisorStateCreateInfoEXT(_parentObjects,
                _s594->sType,
                _s594->pNext,
                _s594->vertexBindingDivisorCount,
                _s594->pVertexBindingDivisors);
            break;
        }
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VERTEX_ATTRIBUTE_DIVISOR_FEATURES_EXT: {
            const auto _s595 = reinterpret_cast<const VkPhysicalDeviceVertexAttributeDivisorFeaturesEXT *>(header);
            skip |= ValidatePhysicalDeviceVertexAttributeDivisorFeaturesEXT(_parentObjects,
                _s595->sType,
                _s595->pNext,
                _s595->vertexAttributeInstanceRateDivisor,
                _s595->vertexAttributeInstanceRateZeroDivisor);
            break;
        }
#ifdef VK_USE_PLATFORM_GGP
        case VK_STRUCTURE_TYPE_PRESENT_FRAME_TOKEN_GGP: {
            const auto _s596 = reinterpret_cast<const VkPresentFrameTokenGGP *>(header);
            skip |= ValidatePresentFrameTokenGGP(_parentObjects,
                _s596->sType,
                _s596->pNext,
                _s596->frameToken);
            break;
        }
#endif
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_COMPUTE_SHADER_DERIVATIVES_FEATURES_NV: {
            const auto _s597 = reinterpret_cast<const VkPhysicalDeviceComputeShaderDerivativesFeaturesNV *>(header);
            skip |= ValidatePhysicalDeviceComputeShaderDerivativesFeaturesNV(_parentObjects,
                _s597->sType,
                _s597->pNext,
                _s597->computeDerivativeGroupQuads,
                _s597->computeDerivativeGroupLinear);
            break;
        }
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MESH_SHADER_FEATURES_NV: {
            const auto _s598 = reinterpret_cast<const VkPhysicalDeviceMeshShaderFeaturesNV *>(header);
            skip |= ValidatePhysicalDeviceMeshShaderFeaturesNV(_parentObjects,
                _s598->sType,
                _s598->pNext,
                _s598->taskShader,
                _s598->meshShader);
            break;
        }
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_IMAGE_FOOTPRINT_FEATURES_NV: {
            const auto _s599 = reinterpret_cast<const VkPhysicalDeviceShaderImageFootprintFeaturesNV *>(header);
            skip |= ValidatePhysicalDeviceShaderImageFootprintFeaturesNV(_parentObjects,
                _s599->sType,
                _s599->pNext,
                _s599->imageFootprint);
            break;
        }
        case VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_EXCLUSIVE_SCISSOR_STATE_CREATE_INFO_NV: {
            const auto _s600 = reinterpret_cast<const VkPipelineViewportExclusiveScissorStateCreateInfoNV *>(header);
            skip |= ValidatePipelineViewportExclusiveScissorStateCreateInfoNV(_parentObjects,
                _s600->sType,
                _s600->pNext,
                _s600->exclusiveScissorCount,
                _s600->pExclusiveScissors);
            break;
        }
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_EXCLUSIVE_SCISSOR_FEATURES_NV: {
            const auto _s601 = reinterpret_cast<const VkPhysicalDeviceExclusiveScissorFeaturesNV *>(header);
            skip |= ValidatePhysicalDeviceExclusiveScissorFeaturesNV(_parentObjects,
                _s601->sType,
                _s601->pNext,
                _s601->exclusiveScissor);
            break;
        }
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_INTEGER_FUNCTIONS_2_FEATURES_INTEL: {
            const auto _s602 = reinterpret_cast<const VkPhysicalDeviceShaderIntegerFunctions2FeaturesINTEL *>(header);
            skip |= ValidatePhysicalDeviceShaderIntegerFunctions2FeaturesINTEL(_parentObjects,
                _s602->sType,
                _s602->pNext,
                _s602->shaderIntegerFunctions2);
            break;
        }
        case VK_STRUCTURE_TYPE_QUERY_POOL_PERFORMANCE_QUERY_CREATE_INFO_INTEL: {
            const auto _s603 = reinterpret_cast<const VkQueryPoolPerformanceQueryCreateInfoINTEL *>(header);
            skip |= ValidateQueryPoolPerformanceQueryCreateInfoINTEL(_parentObjects,
                _s603->sType,
                _s603->pNext,
                _s603->performanceCountersSampling);
            break;
        }
        case VK_STRUCTURE_TYPE_SWAPCHAIN_DISPLAY_NATIVE_HDR_CREATE_INFO_AMD: {
            const auto _s604 = reinterpret_cast<const VkSwapchainDisplayNativeHdrCreateInfoAMD *>(header);
            skip |= ValidateSwapchainDisplayNativeHdrCreateInfoAMD(_parentObjects,
                _s604->sType,
                _s604->pNext,
                _s604->localDimmingEnable);
            break;
        }
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FRAGMENT_DENSITY_MAP_FEATURES_EXT: {
            const auto _s605 = reinterpret_cast<const VkPhysicalDeviceFragmentDensityMapFeaturesEXT *>(header);
            skip |= ValidatePhysicalDeviceFragmentDensityMapFeaturesEXT(_parentObjects,
                _s605->sType,
                _s605->pNext,
                _s605->fragmentDensityMap,
                _s605->fragmentDensityMapDynamic,
                _s605->fragmentDensityMapNonSubsampledImages);
            break;
        }
        case VK_STRUCTURE_TYPE_RENDER_PASS_FRAGMENT_DENSITY_MAP_CREATE_INFO_EXT: {
            const auto _s606 = reinterpret_cast<const VkRenderPassFragmentDensityMapCreateInfoEXT *>(header);
            skip |= ValidateRenderPassFragmentDensityMapCreateInfoEXT(_parentObjects,
                _s606->sType,
                _s606->pNext,
                _s606->fragmentDensityMapAttachment);
            break;
        }
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_COHERENT_MEMORY_FEATURES_AMD: {
            const auto _s607 = reinterpret_cast<const VkPhysicalDeviceCoherentMemoryFeaturesAMD *>(header);
            skip |= ValidatePhysicalDeviceCoherentMemoryFeaturesAMD(_parentObjects,
                _s607->sType,
                _s607->pNext,
                _s607->deviceCoherentMemory);
            break;
        }
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_IMAGE_ATOMIC_INT64_FEATURES_EXT: {
            const auto _s608 = reinterpret_cast<const VkPhysicalDeviceShaderImageAtomicInt64FeaturesEXT *>(header);
            skip |= ValidatePhysicalDeviceShaderImageAtomicInt64FeaturesEXT(_parentObjects,
                _s608->sType,
                _s608->pNext,
                _s608->shaderImageInt64Atomics,
                _s608->sparseImageInt64Atomics);
            break;
        }
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MEMORY_PRIORITY_FEATURES_EXT: {
            const auto _s609 = reinterpret_cast<const VkPhysicalDeviceMemoryPriorityFeaturesEXT *>(header);
            skip |= ValidatePhysicalDeviceMemoryPriorityFeaturesEXT(_parentObjects,
                _s609->sType,
                _s609->pNext,
                _s609->memoryPriority);
            break;
        }
        case VK_STRUCTURE_TYPE_MEMORY_PRIORITY_ALLOCATE_INFO_EXT: {
            const auto _s610 = reinterpret_cast<const VkMemoryPriorityAllocateInfoEXT *>(header);
            skip |= ValidateMemoryPriorityAllocateInfoEXT(_parentObjects,
                _s610->sType,
                _s610->pNext,
                _s610->priority);
            break;
        }
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DEDICATED_ALLOCATION_IMAGE_ALIASING_FEATURES_NV: {
            const auto _s611 = reinterpret_cast<const VkPhysicalDeviceDedicatedAllocationImageAliasingFeaturesNV *>(header);
            skip |= ValidatePhysicalDeviceDedicatedAllocationImageAliasingFeaturesNV(_parentObjects,
                _s611->sType,
                _s611->pNext,
                _s611->dedicatedAllocationImageAliasing);
            break;
        }
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_BUFFER_DEVICE_ADDRESS_FEATURES_EXT: {
            const auto _s612 = reinterpret_cast<const VkPhysicalDeviceBufferDeviceAddressFeaturesEXT *>(header);
            skip |= ValidatePhysicalDeviceBufferDeviceAddressFeaturesEXT(_parentObjects,
                _s612->sType,
                _s612->pNext,
                _s612->bufferDeviceAddress,
                _s612->bufferDeviceAddressCaptureReplay,
                _s612->bufferDeviceAddressMultiDevice);
            break;
        }
        case VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_CREATE_INFO_EXT: {
            const auto _s613 = reinterpret_cast<const VkBufferDeviceAddressCreateInfoEXT *>(header);
            skip |= ValidateBufferDeviceAddressCreateInfoEXT(_parentObjects,
                _s613->sType,
                _s613->pNext,
                _s613->deviceAddress);
            break;
        }
        case VK_STRUCTURE_TYPE_VALIDATION_FEATURES_EXT: {
            const auto _s614 = reinterpret_cast<const VkValidationFeaturesEXT *>(header);
            skip |= ValidateValidationFeaturesEXT(_parentObjects,
                _s614->sType,
                _s614->pNext,
                _s614->enabledValidationFeatureCount,
                _s614->pEnabledValidationFeatures,
                _s614->disabledValidationFeatureCount,
                _s614->pDisabledValidationFeatures);
            break;
        }
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_COOPERATIVE_MATRIX_FEATURES_NV: {
            const auto _s615 = reinterpret_cast<const VkPhysicalDeviceCooperativeMatrixFeaturesNV *>(header);
            skip |= ValidatePhysicalDeviceCooperativeMatrixFeaturesNV(_parentObjects,
                _s615->sType,
                _s615->pNext,
                _s615->cooperativeMatrix,
                _s615->cooperativeMatrixRobustBufferAccess);
            break;
        }
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_COVERAGE_REDUCTION_MODE_FEATURES_NV: {
            const auto _s616 = reinterpret_cast<const VkPhysicalDeviceCoverageReductionModeFeaturesNV *>(header);
            skip |= ValidatePhysicalDeviceCoverageReductionModeFeaturesNV(_parentObjects,
                _s616->sType,
                _s616->pNext,
                _s616->coverageReductionMode);
            break;
        }
        case VK_STRUCTURE_TYPE_PIPELINE_COVERAGE_REDUCTION_STATE_CREATE_INFO_NV: {
            const auto _s617 = reinterpret_cast<const VkPipelineCoverageReductionStateCreateInfoNV *>(header);
            skip |= ValidatePipelineCoverageReductionStateCreateInfoNV(_parentObjects,
                _s617->sType,
                _s617->pNext,
                _s617->flags,
                _s617->coverageReductionMode);
            break;
        }
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FRAGMENT_SHADER_INTERLOCK_FEATURES_EXT: {
            const auto _s618 = reinterpret_cast<const VkPhysicalDeviceFragmentShaderInterlockFeaturesEXT *>(header);
            skip |= ValidatePhysicalDeviceFragmentShaderInterlockFeaturesEXT(_parentObjects,
                _s618->sType,
                _s618->pNext,
                _s618->fragmentShaderSampleInterlock,
                _s618->fragmentShaderPixelInterlock,
                _s618->fragmentShaderShadingRateInterlock);
            break;
        }
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_YCBCR_IMAGE_ARRAYS_FEATURES_EXT: {
            const auto _s619 = reinterpret_cast<const VkPhysicalDeviceYcbcrImageArraysFeaturesEXT *>(header);
            skip |= ValidatePhysicalDeviceYcbcrImageArraysFeaturesEXT(_parentObjects,
                _s619->sType,
                _s619->pNext,
                _s619->ycbcrImageArrays);
            break;
        }
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROVOKING_VERTEX_FEATURES_EXT: {
            const auto _s620 = reinterpret_cast<const VkPhysicalDeviceProvokingVertexFeaturesEXT *>(header);
            skip |= ValidatePhysicalDeviceProvokingVertexFeaturesEXT(_parentObjects,
                _s620->sType,
                _s620->pNext,
                _s620->provokingVertexLast,
                _s620->transformFeedbackPreservesProvokingVertex);
            break;
        }
        case VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_PROVOKING_VERTEX_STATE_CREATE_INFO_EXT: {
            const auto _s621 = reinterpret_cast<const VkPipelineRasterizationProvokingVertexStateCreateInfoEXT *>(header);
            skip |= ValidatePipelineRasterizationProvokingVertexStateCreateInfoEXT(_parentObjects,
                _s621->sType,
                _s621->pNext,
                _s621->provokingVertexMode);
            break;
        }
#ifdef VK_USE_PLATFORM_WIN32_KHR
        case VK_STRUCTURE_TYPE_SURFACE_FULL_SCREEN_EXCLUSIVE_INFO_EXT: {
            const auto _s622 = reinterpret_cast<const VkSurfaceFullScreenExclusiveInfoEXT *>(header);
            skip |= ValidateSurfaceFullScreenExclusiveInfoEXT(_parentObjects,
                _s622->sType,
                _s622->pNext,
                _s622->fullScreenExclusive);
            break;
        }
#endif
#ifdef VK_USE_PLATFORM_WIN32_KHR
        case VK_STRUCTURE_TYPE_SURFACE_CAPABILITIES_FULL_SCREEN_EXCLUSIVE_EXT: {
            const auto _s623 = reinterpret_cast<const VkSurfaceCapabilitiesFullScreenExclusiveEXT *>(header);
            skip |= ValidateSurfaceCapabilitiesFullScreenExclusiveEXT(_parentObjects,
                _s623->sType,
                _s623->pNext,
                _s623->fullScreenExclusiveSupported);
            break;
        }
#endif
#ifdef VK_USE_PLATFORM_WIN32_KHR
        case VK_STRUCTURE_TYPE_SURFACE_FULL_SCREEN_EXCLUSIVE_WIN32_INFO_EXT: {
            const auto _s624 = reinterpret_cast<const VkSurfaceFullScreenExclusiveWin32InfoEXT *>(header);
            skip |= ValidateSurfaceFullScreenExclusiveWin32InfoEXT(_parentObjects,
                _s624->sType,
                _s624->pNext,
                _s624->hmonitor);
            break;
        }
#endif
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_LINE_RASTERIZATION_FEATURES_EXT: {
            const auto _s625 = reinterpret_cast<const VkPhysicalDeviceLineRasterizationFeaturesEXT *>(header);
            skip |= ValidatePhysicalDeviceLineRasterizationFeaturesEXT(_parentObjects,
                _s625->sType,
                _s625->pNext,
                _s625->rectangularLines,
                _s625->bresenhamLines,
                _s625->smoothLines,
                _s625->stippledRectangularLines,
                _s625->stippledBresenhamLines,
                _s625->stippledSmoothLines);
            break;
        }
        case VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_LINE_STATE_CREATE_INFO_EXT: {
            const auto _s626 = reinterpret_cast<const VkPipelineRasterizationLineStateCreateInfoEXT *>(header);
            skip |= ValidatePipelineRasterizationLineStateCreateInfoEXT(_parentObjects,
                _s626->sType,
                _s626->pNext,
                _s626->lineRasterizationMode,
                _s626->stippledLineEnable,
                _s626->lineStippleFactor,
                _s626->lineStipplePattern);
            break;
        }
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_ATOMIC_FLOAT_FEATURES_EXT: {
            const auto _s627 = reinterpret_cast<const VkPhysicalDeviceShaderAtomicFloatFeaturesEXT *>(header);
            skip |= ValidatePhysicalDeviceShaderAtomicFloatFeaturesEXT(_parentObjects,
                _s627->sType,
                _s627->pNext,
                _s627->shaderBufferFloat32Atomics,
                _s627->shaderBufferFloat32AtomicAdd,
                _s627->shaderBufferFloat64Atomics,
                _s627->shaderBufferFloat64AtomicAdd,
                _s627->shaderSharedFloat32Atomics,
                _s627->shaderSharedFloat32AtomicAdd,
                _s627->shaderSharedFloat64Atomics,
                _s627->shaderSharedFloat64AtomicAdd,
                _s627->shaderImageFloat32Atomics,
                _s627->shaderImageFloat32AtomicAdd,
                _s627->sparseImageFloat32Atomics,
                _s627->sparseImageFloat32AtomicAdd);
            break;
        }
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_INDEX_TYPE_UINT8_FEATURES_EXT: {
            const auto _s628 = reinterpret_cast<const VkPhysicalDeviceIndexTypeUint8FeaturesEXT *>(header);
            skip |= ValidatePhysicalDeviceIndexTypeUint8FeaturesEXT(_parentObjects,
                _s628->sType,
                _s628->pNext,
                _s628->indexTypeUint8);
            break;
        }
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_EXTENDED_DYNAMIC_STATE_FEATURES_EXT: {
            const auto _s629 = reinterpret_cast<const VkPhysicalDeviceExtendedDynamicStateFeaturesEXT *>(header);
            skip |= ValidatePhysicalDeviceExtendedDynamicStateFeaturesEXT(_parentObjects,
                _s629->sType,
                _s629->pNext,
                _s629->extendedDynamicState);
            break;
        }
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_ATOMIC_FLOAT_2_FEATURES_EXT: {
            const auto _s630 = reinterpret_cast<const VkPhysicalDeviceShaderAtomicFloat2FeaturesEXT *>(header);
            skip |= ValidatePhysicalDeviceShaderAtomicFloat2FeaturesEXT(_parentObjects,
                _s630->sType,
                _s630->pNext,
                _s630->shaderBufferFloat16Atomics,
                _s630->shaderBufferFloat16AtomicAdd,
                _s630->shaderBufferFloat16AtomicMinMax,
                _s630->shaderBufferFloat32AtomicMinMax,
                _s630->shaderBufferFloat64AtomicMinMax,
                _s630->shaderSharedFloat16Atomics,
                _s630->shaderSharedFloat16AtomicAdd,
                _s630->shaderSharedFloat16AtomicMinMax,
                _s630->shaderSharedFloat32AtomicMinMax,
                _s630->shaderSharedFloat64AtomicMinMax,
                _s630->shaderImageFloat32AtomicMinMax,
                _s630->sparseImageFloat32AtomicMinMax);
            break;
        }
        case VK_STRUCTURE_TYPE_SURFACE_PRESENT_MODE_EXT: {
            const auto _s631 = reinterpret_cast<const VkSurfacePresentModeEXT *>(header);
            skip |= ValidateSurfacePresentModeEXT(_parentObjects,
                _s631->sType,
                _s631->pNext,
                _s631->presentMode);
            break;
        }
        case VK_STRUCTURE_TYPE_SURFACE_PRESENT_SCALING_CAPABILITIES_EXT: {
            const auto _s632 = reinterpret_cast<const VkSurfacePresentScalingCapabilitiesEXT *>(header);
            skip |= ValidateSurfacePresentScalingCapabilitiesEXT(_parentObjects,
                _s632->sType,
                _s632->pNext,
                _s632->supportedPresentScaling,
                _s632->supportedPresentGravityX,
                _s632->supportedPresentGravityY,
                _s632->minScaledImageExtent,
                _s632->maxScaledImageExtent);
            break;
        }
        case VK_STRUCTURE_TYPE_SURFACE_PRESENT_MODE_COMPATIBILITY_EXT: {
            const auto _s633 = reinterpret_cast<const VkSurfacePresentModeCompatibilityEXT *>(header);
            skip |= ValidateSurfacePresentModeCompatibilityEXT(_parentObjects,
                _s633->sType,
                _s633->pNext,
                _s633->presentModeCount,
                _s633->pPresentModes);
            break;
        }
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SWAPCHAIN_MAINTENANCE_1_FEATURES_EXT: {
            const auto _s634 = reinterpret_cast<const VkPhysicalDeviceSwapchainMaintenance1FeaturesEXT *>(header);
            skip |= ValidatePhysicalDeviceSwapchainMaintenance1FeaturesEXT(_parentObjects,
                _s634->sType,
                _s634->pNext,
                _s634->swapchainMaintenance1);
            break;
        }
        case VK_STRUCTURE_TYPE_SWAPCHAIN_PRESENT_FENCE_INFO_EXT: {
            const auto _s635 = reinterpret_cast<const VkSwapchainPresentFenceInfoEXT *>(header);
            skip |= ValidateSwapchainPresentFenceInfoEXT(_parentObjects,
                _s635->sType,
                _s635->pNext,
                _s635->swapchainCount,
                _s635->pFences);
            break;
        }
        case VK_STRUCTURE_TYPE_SWAPCHAIN_PRESENT_MODES_CREATE_INFO_EXT: {
            const auto _s636 = reinterpret_cast<const VkSwapchainPresentModesCreateInfoEXT *>(header);
            skip |= ValidateSwapchainPresentModesCreateInfoEXT(_parentObjects,
                _s636->sType,
                _s636->pNext,
                _s636->presentModeCount,
                _s636->pPresentModes);
            break;
        }
        case VK_STRUCTURE_TYPE_SWAPCHAIN_PRESENT_MODE_INFO_EXT: {
            const auto _s637 = reinterpret_cast<const VkSwapchainPresentModeInfoEXT *>(header);
            skip |= ValidateSwapchainPresentModeInfoEXT(_parentObjects,
                _s637->sType,
                _s637->pNext,
                _s637->swapchainCount,
                _s637->pPresentModes);
            break;
        }
        case VK_STRUCTURE_TYPE_SWAPCHAIN_PRESENT_SCALING_CREATE_INFO_EXT: {
            const auto _s638 = reinterpret_cast<const VkSwapchainPresentScalingCreateInfoEXT *>(header);
            skip |= ValidateSwapchainPresentScalingCreateInfoEXT(_parentObjects,
                _s638->sType,
                _s638->pNext,
                _s638->scalingBehavior,
                _s638->presentGravityX,
                _s638->presentGravityY);
            break;
        }
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DEVICE_GENERATED_COMMANDS_FEATURES_NV: {
            const auto _s639 = reinterpret_cast<const VkPhysicalDeviceDeviceGeneratedCommandsFeaturesNV *>(header);
            skip |= ValidatePhysicalDeviceDeviceGeneratedCommandsFeaturesNV(_parentObjects,
                _s639->sType,
                _s639->pNext,
                _s639->deviceGeneratedCommands);
            break;
        }
        case VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_SHADER_GROUPS_CREATE_INFO_NV: {
            const auto _s640 = reinterpret_cast<const VkGraphicsPipelineShaderGroupsCreateInfoNV *>(header);
            skip |= ValidateGraphicsPipelineShaderGroupsCreateInfoNV(_parentObjects,
                _s640->sType,
                _s640->pNext,
                _s640->groupCount,
                _s640->pGroups,
                _s640->pipelineCount,
                _s640->pPipelines);
            break;
        }
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_INHERITED_VIEWPORT_SCISSOR_FEATURES_NV: {
            const auto _s641 = reinterpret_cast<const VkPhysicalDeviceInheritedViewportScissorFeaturesNV *>(header);
            skip |= ValidatePhysicalDeviceInheritedViewportScissorFeaturesNV(_parentObjects,
                _s641->sType,
                _s641->pNext,
                _s641->inheritedViewportScissor2D);
            break;
        }
        case VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_VIEWPORT_SCISSOR_INFO_NV: {
            const auto _s642 = reinterpret_cast<const VkCommandBufferInheritanceViewportScissorInfoNV *>(header);
            skip |= ValidateCommandBufferInheritanceViewportScissorInfoNV(_parentObjects,
                _s642->sType,
                _s642->pNext,
                _s642->viewportScissor2D,
                _s642->viewportDepthCount,
                _s642->pViewportDepths);
            break;
        }
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_TEXEL_BUFFER_ALIGNMENT_FEATURES_EXT: {
            const auto _s643 = reinterpret_cast<const VkPhysicalDeviceTexelBufferAlignmentFeaturesEXT *>(header);
            skip |= ValidatePhysicalDeviceTexelBufferAlignmentFeaturesEXT(_parentObjects,
                _s643->sType,
                _s643->pNext,
                _s643->texelBufferAlignment);
            break;
        }
        case VK_STRUCTURE_TYPE_RENDER_PASS_TRANSFORM_BEGIN_INFO_QCOM: {
            const auto _s644 = reinterpret_cast<const VkRenderPassTransformBeginInfoQCOM *>(header);
            skip |= ValidateRenderPassTransformBeginInfoQCOM(_parentObjects,
                _s644->sType,
                _s644->pNext,
                _s644->transform);
            break;
        }
        case VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_RENDER_PASS_TRANSFORM_INFO_QCOM: {
            const auto _s645 = reinterpret_cast<const VkCommandBufferInheritanceRenderPassTransformInfoQCOM *>(header);
            skip |= ValidateCommandBufferInheritanceRenderPassTransformInfoQCOM(_parentObjects,
                _s645->sType,
                _s645->pNext,
                _s645->transform,
                _s645->renderArea);
            break;
        }
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DEVICE_MEMORY_REPORT_FEATURES_EXT: {
            const auto _s646 = reinterpret_cast<const VkPhysicalDeviceDeviceMemoryReportFeaturesEXT *>(header);
            skip |= ValidatePhysicalDeviceDeviceMemoryReportFeaturesEXT(_parentObjects,
                _s646->sType,
                _s646->pNext,
                _s646->deviceMemoryReport);
            break;
        }
        case VK_STRUCTURE_TYPE_DEVICE_DEVICE_MEMORY_REPORT_CREATE_INFO_EXT: {
            const auto _s647 = reinterpret_cast<const VkDeviceDeviceMemoryReportCreateInfoEXT *>(header);
            skip |= ValidateDeviceDeviceMemoryReportCreateInfoEXT(_parentObjects,
                _s647->sType,
                _s647->pNext,
                _s647->flags,
                _s647->pfnUserCallback,
                _s647->pUserData);
            break;
        }
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ROBUSTNESS_2_FEATURES_EXT: {
            const auto _s648 = reinterpret_cast<const VkPhysicalDeviceRobustness2FeaturesEXT *>(header);
            skip |= ValidatePhysicalDeviceRobustness2FeaturesEXT(_parentObjects,
                _s648->sType,
                _s648->pNext,
                _s648->robustBufferAccess2,
                _s648->robustImageAccess2,
                _s648->nullDescriptor);
            break;
        }
        case VK_STRUCTURE_TYPE_SAMPLER_CUSTOM_BORDER_COLOR_CREATE_INFO_EXT: {
            const auto _s649 = reinterpret_cast<const VkSamplerCustomBorderColorCreateInfoEXT *>(header);
            skip |= ValidateSamplerCustomBorderColorCreateInfoEXT(_parentObjects,
                _s649->sType,
                _s649->pNext,
                _s649->customBorderColor,
                _s649->format);
            break;
        }
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_CUSTOM_BORDER_COLOR_FEATURES_EXT: {
            const auto _s650 = reinterpret_cast<const VkPhysicalDeviceCustomBorderColorFeaturesEXT *>(header);
            skip |= ValidatePhysicalDeviceCustomBorderColorFeaturesEXT(_parentObjects,
                _s650->sType,
                _s650->pNext,
                _s650->customBorderColors,
                _s650->customBorderColorWithoutFormat);
            break;
        }
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PRESENT_BARRIER_FEATURES_NV: {
            const auto _s651 = reinterpret_cast<const VkPhysicalDevicePresentBarrierFeaturesNV *>(header);
            skip |= ValidatePhysicalDevicePresentBarrierFeaturesNV(_parentObjects,
                _s651->sType,
                _s651->pNext,
                _s651->presentBarrier);
            break;
        }
        case VK_STRUCTURE_TYPE_SURFACE_CAPABILITIES_PRESENT_BARRIER_NV: {
            const auto _s652 = reinterpret_cast<const VkSurfaceCapabilitiesPresentBarrierNV *>(header);
            skip |= ValidateSurfaceCapabilitiesPresentBarrierNV(_parentObjects,
                _s652->sType,
                _s652->pNext,
                _s652->presentBarrierSupported);
            break;
        }
        case VK_STRUCTURE_TYPE_SWAPCHAIN_PRESENT_BARRIER_CREATE_INFO_NV: {
            const auto _s653 = reinterpret_cast<const VkSwapchainPresentBarrierCreateInfoNV *>(header);
            skip |= ValidateSwapchainPresentBarrierCreateInfoNV(_parentObjects,
                _s653->sType,
                _s653->pNext,
                _s653->presentBarrierEnable);
            break;
        }
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DIAGNOSTICS_CONFIG_FEATURES_NV: {
            const auto _s654 = reinterpret_cast<const VkPhysicalDeviceDiagnosticsConfigFeaturesNV *>(header);
            skip |= ValidatePhysicalDeviceDiagnosticsConfigFeaturesNV(_parentObjects,
                _s654->sType,
                _s654->pNext,
                _s654->diagnosticsConfig);
            break;
        }
        case VK_STRUCTURE_TYPE_DEVICE_DIAGNOSTICS_CONFIG_CREATE_INFO_NV: {
            const auto _s655 = reinterpret_cast<const VkDeviceDiagnosticsConfigCreateInfoNV *>(header);
            skip |= ValidateDeviceDiagnosticsConfigCreateInfoNV(_parentObjects,
                _s655->sType,
                _s655->pNext,
                _s655->flags);
            break;
        }
        case VK_STRUCTURE_TYPE_QUERY_LOW_LATENCY_SUPPORT_NV: {
            const auto _s656 = reinterpret_cast<const VkQueryLowLatencySupportNV *>(header);
            skip |= ValidateQueryLowLatencySupportNV(_parentObjects,
                _s656->sType,
                _s656->pNext,
                _s656->pQueriedLowLatencyData);
            break;
        }
#ifdef VK_USE_PLATFORM_METAL_EXT
        case VK_STRUCTURE_TYPE_EXPORT_METAL_OBJECT_CREATE_INFO_EXT: {
            const auto _s657 = reinterpret_cast<const VkExportMetalObjectCreateInfoEXT *>(header);
            skip |= ValidateExportMetalObjectCreateInfoEXT(_parentObjects,
                _s657->sType,
                _s657->pNext,
                _s657->exportObjectType);
            break;
        }
#endif
#ifdef VK_USE_PLATFORM_METAL_EXT
        case VK_STRUCTURE_TYPE_EXPORT_METAL_DEVICE_INFO_EXT: {
            const auto _s658 = reinterpret_cast<const VkExportMetalDeviceInfoEXT *>(header);
            skip |= ValidateExportMetalDeviceInfoEXT(_parentObjects,
                _s658->sType,
                _s658->pNext,
                _s658->mtlDevice);
            break;
        }
#endif
#ifdef VK_USE_PLATFORM_METAL_EXT
        case VK_STRUCTURE_TYPE_EXPORT_METAL_COMMAND_QUEUE_INFO_EXT: {
            const auto _s659 = reinterpret_cast<const VkExportMetalCommandQueueInfoEXT *>(header);
            skip |= ValidateExportMetalCommandQueueInfoEXT(_parentObjects,
                _s659->sType,
                _s659->pNext,
                _s659->queue,
                _s659->mtlCommandQueue);
            break;
        }
#endif
#ifdef VK_USE_PLATFORM_METAL_EXT
        case VK_STRUCTURE_TYPE_EXPORT_METAL_BUFFER_INFO_EXT: {
            const auto _s660 = reinterpret_cast<const VkExportMetalBufferInfoEXT *>(header);
            skip |= ValidateExportMetalBufferInfoEXT(_parentObjects,
                _s660->sType,
                _s660->pNext,
                _s660->memory,
                _s660->mtlBuffer);
            break;
        }
#endif
#ifdef VK_USE_PLATFORM_METAL_EXT
        case VK_STRUCTURE_TYPE_IMPORT_METAL_BUFFER_INFO_EXT: {
            const auto _s661 = reinterpret_cast<const VkImportMetalBufferInfoEXT *>(header);
            skip |= ValidateImportMetalBufferInfoEXT(_parentObjects,
                _s661->sType,
                _s661->pNext,
                _s661->mtlBuffer);
            break;
        }
#endif
#ifdef VK_USE_PLATFORM_METAL_EXT
        case VK_STRUCTURE_TYPE_EXPORT_METAL_TEXTURE_INFO_EXT: {
            const auto _s662 = reinterpret_cast<const VkExportMetalTextureInfoEXT *>(header);
            skip |= ValidateExportMetalTextureInfoEXT(_parentObjects,
                _s662->sType,
                _s662->pNext,
                _s662->image,
                _s662->imageView,
                _s662->bufferView,
                _s662->plane,
                _s662->mtlTexture);
            break;
        }
#endif
#ifdef VK_USE_PLATFORM_METAL_EXT
        case VK_STRUCTURE_TYPE_IMPORT_METAL_TEXTURE_INFO_EXT: {
            const auto _s663 = reinterpret_cast<const VkImportMetalTextureInfoEXT *>(header);
            skip |= ValidateImportMetalTextureInfoEXT(_parentObjects,
                _s663->sType,
                _s663->pNext,
                _s663->plane,
                _s663->mtlTexture);
            break;
        }
#endif
#ifdef VK_USE_PLATFORM_METAL_EXT
        case VK_STRUCTURE_TYPE_EXPORT_METAL_IO_SURFACE_INFO_EXT: {
            const auto _s664 = reinterpret_cast<const VkExportMetalIOSurfaceInfoEXT *>(header);
            skip |= ValidateExportMetalIOSurfaceInfoEXT(_parentObjects,
                _s664->sType,
                _s664->pNext,
                _s664->image,
                _s664->ioSurface);
            break;
        }
#endif
#ifdef VK_USE_PLATFORM_METAL_EXT
        case VK_STRUCTURE_TYPE_IMPORT_METAL_IO_SURFACE_INFO_EXT: {
            const auto _s665 = reinterpret_cast<const VkImportMetalIOSurfaceInfoEXT *>(header);
            skip |= ValidateImportMetalIOSurfaceInfoEXT(_parentObjects,
                _s665->sType,
                _s665->pNext,
                _s665->ioSurface);
            break;
        }
#endif
#ifdef VK_USE_PLATFORM_METAL_EXT
        case VK_STRUCTURE_TYPE_EXPORT_METAL_SHARED_EVENT_INFO_EXT: {
            const auto _s666 = reinterpret_cast<const VkExportMetalSharedEventInfoEXT *>(header);
            skip |= ValidateExportMetalSharedEventInfoEXT(_parentObjects,
                _s666->sType,
                _s666->pNext,
                _s666->semaphore,
                _s666->event,
                _s666->mtlSharedEvent);
            break;
        }
#endif
#ifdef VK_USE_PLATFORM_METAL_EXT
        case VK_STRUCTURE_TYPE_IMPORT_METAL_SHARED_EVENT_INFO_EXT: {
            const auto _s667 = reinterpret_cast<const VkImportMetalSharedEventInfoEXT *>(header);
            skip |= ValidateImportMetalSharedEventInfoEXT(_parentObjects,
                _s667->sType,
                _s667->pNext,
                _s667->mtlSharedEvent);
            break;
        }
#endif
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_BUFFER_FEATURES_EXT: {
            const auto _s668 = reinterpret_cast<const VkPhysicalDeviceDescriptorBufferFeaturesEXT *>(header);
            skip |= ValidatePhysicalDeviceDescriptorBufferFeaturesEXT(_parentObjects,
                _s668->sType,
                _s668->pNext,
                _s668->descriptorBuffer,
                _s668->descriptorBufferCaptureReplay,
                _s668->descriptorBufferImageLayoutIgnored,
                _s668->descriptorBufferPushDescriptors);
            break;
        }
        case VK_STRUCTURE_TYPE_DESCRIPTOR_BUFFER_BINDING_PUSH_DESCRIPTOR_BUFFER_HANDLE_EXT: {
            const auto _s669 = reinterpret_cast<const VkDescriptorBufferBindingPushDescriptorBufferHandleEXT *>(header);
            skip |= ValidateDescriptorBufferBindingPushDescriptorBufferHandleEXT(_parentObjects,
                _s669->sType,
                _s669->pNext,
                _s669->buffer);
            break;
        }
        case VK_STRUCTURE_TYPE_OPAQUE_CAPTURE_DESCRIPTOR_DATA_CREATE_INFO_EXT: {
            const auto _s670 = reinterpret_cast<const VkOpaqueCaptureDescriptorDataCreateInfoEXT *>(header);
            skip |= ValidateOpaqueCaptureDescriptorDataCreateInfoEXT(_parentObjects,
                _s670->sType,
                _s670->pNext,
                _s670->opaqueCaptureDescriptorData);
            break;
        }
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_GRAPHICS_PIPELINE_LIBRARY_FEATURES_EXT: {
            const auto _s671 = reinterpret_cast<const VkPhysicalDeviceGraphicsPipelineLibraryFeaturesEXT *>(header);
            skip |= ValidatePhysicalDeviceGraphicsPipelineLibraryFeaturesEXT(_parentObjects,
                _s671->sType,
                _s671->pNext,
                _s671->graphicsPipelineLibrary);
            break;
        }
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_GRAPHICS_PIPELINE_LIBRARY_PROPERTIES_EXT: {
            const auto _s672 = reinterpret_cast<const VkPhysicalDeviceGraphicsPipelineLibraryPropertiesEXT *>(header);
            skip |= ValidatePhysicalDeviceGraphicsPipelineLibraryPropertiesEXT(_parentObjects,
                _s672->sType,
                _s672->pNext,
                _s672->graphicsPipelineLibraryFastLinking,
                _s672->graphicsPipelineLibraryIndependentInterpolationDecoration);
            break;
        }
        case VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_LIBRARY_CREATE_INFO_EXT: {
            const auto _s673 = reinterpret_cast<const VkGraphicsPipelineLibraryCreateInfoEXT *>(header);
            skip |= ValidateGraphicsPipelineLibraryCreateInfoEXT(_parentObjects,
                _s673->sType,
                _s673->pNext,
                _s673->flags);
            break;
        }
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_EARLY_AND_LATE_FRAGMENT_TESTS_FEATURES_AMD: {
            const auto _s674 = reinterpret_cast<const VkPhysicalDeviceShaderEarlyAndLateFragmentTestsFeaturesAMD *>(header);
            skip |= ValidatePhysicalDeviceShaderEarlyAndLateFragmentTestsFeaturesAMD(_parentObjects,
                _s674->sType,
                _s674->pNext,
                _s674->shaderEarlyAndLateFragmentTests);
            break;
        }
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FRAGMENT_SHADING_RATE_ENUMS_FEATURES_NV: {
            const auto _s675 = reinterpret_cast<const VkPhysicalDeviceFragmentShadingRateEnumsFeaturesNV *>(header);
            skip |= ValidatePhysicalDeviceFragmentShadingRateEnumsFeaturesNV(_parentObjects,
                _s675->sType,
                _s675->pNext,
                _s675->fragmentShadingRateEnums,
                _s675->supersampleFragmentShadingRates,
                _s675->noInvocationFragmentShadingRates);
            break;
        }
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FRAGMENT_SHADING_RATE_ENUMS_PROPERTIES_NV: {
            const auto _s676 = reinterpret_cast<const VkPhysicalDeviceFragmentShadingRateEnumsPropertiesNV *>(header);
            skip |= ValidatePhysicalDeviceFragmentShadingRateEnumsPropertiesNV(_parentObjects,
                _s676->sType,
                _s676->pNext,
                _s676->maxFragmentShadingRateInvocationCount);
            break;
        }
        case VK_STRUCTURE_TYPE_PIPELINE_FRAGMENT_SHADING_RATE_ENUM_STATE_CREATE_INFO_NV: {
            const auto _s677 = reinterpret_cast<const VkPipelineFragmentShadingRateEnumStateCreateInfoNV *>(header);
            skip |= ValidatePipelineFragmentShadingRateEnumStateCreateInfoNV(_parentObjects,
                _s677->sType,
                _s677->pNext,
                _s677->shadingRateType,
                _s677->shadingRate,
                _s677->combinerOps);
            break;
        }
        case VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_MOTION_TRIANGLES_DATA_NV: {
            const auto _s678 = reinterpret_cast<const VkAccelerationStructureGeometryMotionTrianglesDataNV *>(header);
            skip |= ValidateAccelerationStructureGeometryMotionTrianglesDataNV(_parentObjects,
                _s678->sType,
                _s678->pNext,
                _s678->vertexData);
            break;
        }
        case VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_MOTION_INFO_NV: {
            const auto _s679 = reinterpret_cast<const VkAccelerationStructureMotionInfoNV *>(header);
            skip |= ValidateAccelerationStructureMotionInfoNV(_parentObjects,
                _s679->sType,
                _s679->pNext,
                _s679->maxInstances,
                _s679->flags);
            break;
        }
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_MOTION_BLUR_FEATURES_NV: {
            const auto _s680 = reinterpret_cast<const VkPhysicalDeviceRayTracingMotionBlurFeaturesNV *>(header);
            skip |= ValidatePhysicalDeviceRayTracingMotionBlurFeaturesNV(_parentObjects,
                _s680->sType,
                _s680->pNext,
                _s680->rayTracingMotionBlur,
                _s680->rayTracingMotionBlurPipelineTraceRaysIndirect);
            break;
        }
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_YCBCR_2_PLANE_444_FORMATS_FEATURES_EXT: {
            const auto _s681 = reinterpret_cast<const VkPhysicalDeviceYcbcr2Plane444FormatsFeaturesEXT *>(header);
            skip |= ValidatePhysicalDeviceYcbcr2Plane444FormatsFeaturesEXT(_parentObjects,
                _s681->sType,
                _s681->pNext,
                _s681->ycbcr2plane444Formats);
            break;
        }
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FRAGMENT_DENSITY_MAP_2_FEATURES_EXT: {
            const auto _s682 = reinterpret_cast<const VkPhysicalDeviceFragmentDensityMap2FeaturesEXT *>(header);
            skip |= ValidatePhysicalDeviceFragmentDensityMap2FeaturesEXT(_parentObjects,
                _s682->sType,
                _s682->pNext,
                _s682->fragmentDensityMapDeferred);
            break;
        }
        case VK_STRUCTURE_TYPE_COPY_COMMAND_TRANSFORM_INFO_QCOM: {
            const auto _s683 = reinterpret_cast<const VkCopyCommandTransformInfoQCOM *>(header);
            skip |= ValidateCopyCommandTransformInfoQCOM(_parentObjects,
                _s683->sType,
                _s683->pNext,
                _s683->transform);
            break;
        }
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_IMAGE_COMPRESSION_CONTROL_FEATURES_EXT: {
            const auto _s684 = reinterpret_cast<const VkPhysicalDeviceImageCompressionControlFeaturesEXT *>(header);
            skip |= ValidatePhysicalDeviceImageCompressionControlFeaturesEXT(_parentObjects,
                _s684->sType,
                _s684->pNext,
                _s684->imageCompressionControl);
            break;
        }
        case VK_STRUCTURE_TYPE_IMAGE_COMPRESSION_CONTROL_EXT: {
            const auto _s685 = reinterpret_cast<const VkImageCompressionControlEXT *>(header);
            skip |= ValidateImageCompressionControlEXT(_parentObjects,
                _s685->sType,
                _s685->pNext,
                _s685->flags,
                _s685->compressionControlPlaneCount,
                _s685->pFixedRateFlags);
            break;
        }
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ATTACHMENT_FEEDBACK_LOOP_LAYOUT_FEATURES_EXT: {
            const auto _s686 = reinterpret_cast<const VkPhysicalDeviceAttachmentFeedbackLoopLayoutFeaturesEXT *>(header);
            skip |= ValidatePhysicalDeviceAttachmentFeedbackLoopLayoutFeaturesEXT(_parentObjects,
                _s686->sType,
                _s686->pNext,
                _s686->attachmentFeedbackLoopLayout);
            break;
        }
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_4444_FORMATS_FEATURES_EXT: {
            const auto _s687 = reinterpret_cast<const VkPhysicalDevice4444FormatsFeaturesEXT *>(header);
            skip |= ValidatePhysicalDevice4444FormatsFeaturesEXT(_parentObjects,
                _s687->sType,
                _s687->pNext,
                _s687->formatA4R4G4B4,
                _s687->formatA4B4G4R4);
            break;
        }
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FAULT_FEATURES_EXT: {
            const auto _s688 = reinterpret_cast<const VkPhysicalDeviceFaultFeaturesEXT *>(header);
            skip |= ValidatePhysicalDeviceFaultFeaturesEXT(_parentObjects,
                _s688->sType,
                _s688->pNext,
                _s688->deviceFault,
                _s688->deviceFaultVendorBinary);
            break;
        }
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RASTERIZATION_ORDER_ATTACHMENT_ACCESS_FEATURES_EXT: {
            const auto _s689 = reinterpret_cast<const VkPhysicalDeviceRasterizationOrderAttachmentAccessFeaturesEXT *>(header);
            skip |= ValidatePhysicalDeviceRasterizationOrderAttachmentAccessFeaturesEXT(_parentObjects,
                _s689->sType,
                _s689->pNext,
                _s689->rasterizationOrderColorAttachmentAccess,
                _s689->rasterizationOrderDepthAttachmentAccess,
                _s689->rasterizationOrderStencilAttachmentAccess);
            break;
        }
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RGBA10X6_FORMATS_FEATURES_EXT: {
            const auto _s690 = reinterpret_cast<const VkPhysicalDeviceRGBA10X6FormatsFeaturesEXT *>(header);
            skip |= ValidatePhysicalDeviceRGBA10X6FormatsFeaturesEXT(_parentObjects,
                _s690->sType,
                _s690->pNext,
                _s690->formatRgba10x6WithoutYCbCrSampler);
            break;
        }
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MUTABLE_DESCRIPTOR_TYPE_FEATURES_EXT: {
            const auto _s691 = reinterpret_cast<const VkPhysicalDeviceMutableDescriptorTypeFeaturesEXT *>(header);
            skip |= ValidatePhysicalDeviceMutableDescriptorTypeFeaturesEXT(_parentObjects,
                _s691->sType,
                _s691->pNext,
                _s691->mutableDescriptorType);
            break;
        }
        case VK_STRUCTURE_TYPE_MUTABLE_DESCRIPTOR_TYPE_CREATE_INFO_EXT: {
            const auto _s692 = reinterpret_cast<const VkMutableDescriptorTypeCreateInfoEXT *>(header);
            skip |= ValidateMutableDescriptorTypeCreateInfoEXT(_parentObjects,
                _s692->sType,
                _s692->pNext,
                _s692->mutableDescriptorTypeListCount,
                _s692->pMutableDescriptorTypeLists);
            break;
        }
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VERTEX_INPUT_DYNAMIC_STATE_FEATURES_EXT: {
            const auto _s693 = reinterpret_cast<const VkPhysicalDeviceVertexInputDynamicStateFeaturesEXT *>(header);
            skip |= ValidatePhysicalDeviceVertexInputDynamicStateFeaturesEXT(_parentObjects,
                _s693->sType,
                _s693->pNext,
                _s693->vertexInputDynamicState);
            break;
        }
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ADDRESS_BINDING_REPORT_FEATURES_EXT: {
            const auto _s694 = reinterpret_cast<const VkPhysicalDeviceAddressBindingReportFeaturesEXT *>(header);
            skip |= ValidatePhysicalDeviceAddressBindingReportFeaturesEXT(_parentObjects,
                _s694->sType,
                _s694->pNext,
                _s694->reportAddressBinding);
            break;
        }
        case VK_STRUCTURE_TYPE_DEVICE_ADDRESS_BINDING_CALLBACK_DATA_EXT: {
            const auto _s695 = reinterpret_cast<const VkDeviceAddressBindingCallbackDataEXT *>(header);
            skip |= ValidateDeviceAddressBindingCallbackDataEXT(_parentObjects,
                _s695->sType,
                _s695->pNext,
                _s695->flags,
                _s695->baseAddress,
                _s695->size,
                _s695->bindingType);
            break;
        }
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DEPTH_CLIP_CONTROL_FEATURES_EXT: {
            const auto _s696 = reinterpret_cast<const VkPhysicalDeviceDepthClipControlFeaturesEXT *>(header);
            skip |= ValidatePhysicalDeviceDepthClipControlFeaturesEXT(_parentObjects,
                _s696->sType,
                _s696->pNext,
                _s696->depthClipControl);
            break;
        }
        case VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_DEPTH_CLIP_CONTROL_CREATE_INFO_EXT: {
            const auto _s697 = reinterpret_cast<const VkPipelineViewportDepthClipControlCreateInfoEXT *>(header);
            skip |= ValidatePipelineViewportDepthClipControlCreateInfoEXT(_parentObjects,
                _s697->sType,
                _s697->pNext,
                _s697->negativeOneToOne);
            break;
        }
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PRIMITIVE_TOPOLOGY_LIST_RESTART_FEATURES_EXT: {
            const auto _s698 = reinterpret_cast<const VkPhysicalDevicePrimitiveTopologyListRestartFeaturesEXT *>(header);
            skip |= ValidatePhysicalDevicePrimitiveTopologyListRestartFeaturesEXT(_parentObjects,
                _s698->sType,
                _s698->pNext,
                _s698->primitiveTopologyListRestart,
                _s698->primitiveTopologyPatchListRestart);
            break;
        }
#ifdef VK_USE_PLATFORM_FUCHSIA
        case VK_STRUCTURE_TYPE_IMPORT_MEMORY_ZIRCON_HANDLE_INFO_FUCHSIA: {
            const auto _s699 = reinterpret_cast<const VkImportMemoryZirconHandleInfoFUCHSIA *>(header);
            skip |= ValidateImportMemoryZirconHandleInfoFUCHSIA(_parentObjects,
                _s699->sType,
                _s699->pNext,
                _s699->handleType,
                _s699->handle);
            break;
        }
#endif
#ifdef VK_USE_PLATFORM_FUCHSIA
        case VK_STRUCTURE_TYPE_IMPORT_MEMORY_BUFFER_COLLECTION_FUCHSIA: {
            const auto _s700 = reinterpret_cast<const VkImportMemoryBufferCollectionFUCHSIA *>(header);
            skip |= ValidateImportMemoryBufferCollectionFUCHSIA(_parentObjects,
                _s700->sType,
                _s700->pNext,
                _s700->collection,
                _s700->index);
            break;
        }
#endif
#ifdef VK_USE_PLATFORM_FUCHSIA
        case VK_STRUCTURE_TYPE_BUFFER_COLLECTION_IMAGE_CREATE_INFO_FUCHSIA: {
            const auto _s701 = reinterpret_cast<const VkBufferCollectionImageCreateInfoFUCHSIA *>(header);
            skip |= ValidateBufferCollectionImageCreateInfoFUCHSIA(_parentObjects,
                _s701->sType,
                _s701->pNext,
                _s701->collection,
                _s701->index);
            break;
        }
#endif
#ifdef VK_USE_PLATFORM_FUCHSIA
        case VK_STRUCTURE_TYPE_BUFFER_COLLECTION_BUFFER_CREATE_INFO_FUCHSIA: {
            const auto _s702 = reinterpret_cast<const VkBufferCollectionBufferCreateInfoFUCHSIA *>(header);
            skip |= ValidateBufferCollectionBufferCreateInfoFUCHSIA(_parentObjects,
                _s702->sType,
                _s702->pNext,
                _s702->collection,
                _s702->index);
            break;
        }
#endif
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SUBPASS_SHADING_FEATURES_HUAWEI: {
            const auto _s703 = reinterpret_cast<const VkPhysicalDeviceSubpassShadingFeaturesHUAWEI *>(header);
            skip |= ValidatePhysicalDeviceSubpassShadingFeaturesHUAWEI(_parentObjects,
                _s703->sType,
                _s703->pNext,
                _s703->subpassShading);
            break;
        }
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_INVOCATION_MASK_FEATURES_HUAWEI: {
            const auto _s704 = reinterpret_cast<const VkPhysicalDeviceInvocationMaskFeaturesHUAWEI *>(header);
            skip |= ValidatePhysicalDeviceInvocationMaskFeaturesHUAWEI(_parentObjects,
                _s704->sType,
                _s704->pNext,
                _s704->invocationMask);
            break;
        }
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_EXTERNAL_MEMORY_RDMA_FEATURES_NV: {
            const auto _s705 = reinterpret_cast<const VkPhysicalDeviceExternalMemoryRDMAFeaturesNV *>(header);
            skip |= ValidatePhysicalDeviceExternalMemoryRDMAFeaturesNV(_parentObjects,
                _s705->sType,
                _s705->pNext,
                _s705->externalMemoryRDMA);
            break;
        }
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PIPELINE_PROPERTIES_FEATURES_EXT: {
            const auto _s706 = reinterpret_cast<const VkPhysicalDevicePipelinePropertiesFeaturesEXT *>(header);
            skip |= ValidatePhysicalDevicePipelinePropertiesFeaturesEXT(_parentObjects,
                _s706->sType,
                _s706->pNext,
                _s706->pipelinePropertiesIdentifier);
            break;
        }
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MULTISAMPLED_RENDER_TO_SINGLE_SAMPLED_FEATURES_EXT: {
            const auto _s707 = reinterpret_cast<const VkPhysicalDeviceMultisampledRenderToSingleSampledFeaturesEXT *>(header);
            skip |= ValidatePhysicalDeviceMultisampledRenderToSingleSampledFeaturesEXT(_parentObjects,
                _s707->sType,
                _s707->pNext,
                _s707->multisampledRenderToSingleSampled);
            break;
        }
        case VK_STRUCTURE_TYPE_MULTISAMPLED_RENDER_TO_SINGLE_SAMPLED_INFO_EXT: {
            const auto _s708 = reinterpret_cast<const VkMultisampledRenderToSingleSampledInfoEXT *>(header);
            skip |= ValidateMultisampledRenderToSingleSampledInfoEXT(_parentObjects,
                _s708->sType,
                _s708->pNext,
                _s708->multisampledRenderToSingleSampledEnable,
                _s708->rasterizationSamples);
            break;
        }
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_EXTENDED_DYNAMIC_STATE_2_FEATURES_EXT: {
            const auto _s709 = reinterpret_cast<const VkPhysicalDeviceExtendedDynamicState2FeaturesEXT *>(header);
            skip |= ValidatePhysicalDeviceExtendedDynamicState2FeaturesEXT(_parentObjects,
                _s709->sType,
                _s709->pNext,
                _s709->extendedDynamicState2,
                _s709->extendedDynamicState2LogicOp,
                _s709->extendedDynamicState2PatchControlPoints);
            break;
        }
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_COLOR_WRITE_ENABLE_FEATURES_EXT: {
            const auto _s710 = reinterpret_cast<const VkPhysicalDeviceColorWriteEnableFeaturesEXT *>(header);
            skip |= ValidatePhysicalDeviceColorWriteEnableFeaturesEXT(_parentObjects,
                _s710->sType,
                _s710->pNext,
                _s710->colorWriteEnable);
            break;
        }
        case VK_STRUCTURE_TYPE_PIPELINE_COLOR_WRITE_CREATE_INFO_EXT: {
            const auto _s711 = reinterpret_cast<const VkPipelineColorWriteCreateInfoEXT *>(header);
            skip |= ValidatePipelineColorWriteCreateInfoEXT(_parentObjects,
                _s711->sType,
                _s711->pNext,
                _s711->attachmentCount,
                _s711->pColorWriteEnables);
            break;
        }
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PRIMITIVES_GENERATED_QUERY_FEATURES_EXT: {
            const auto _s712 = reinterpret_cast<const VkPhysicalDevicePrimitivesGeneratedQueryFeaturesEXT *>(header);
            skip |= ValidatePhysicalDevicePrimitivesGeneratedQueryFeaturesEXT(_parentObjects,
                _s712->sType,
                _s712->pNext,
                _s712->primitivesGeneratedQuery,
                _s712->primitivesGeneratedQueryWithRasterizerDiscard,
                _s712->primitivesGeneratedQueryWithNonZeroStreams);
            break;
        }
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_IMAGE_VIEW_MIN_LOD_FEATURES_EXT: {
            const auto _s713 = reinterpret_cast<const VkPhysicalDeviceImageViewMinLodFeaturesEXT *>(header);
            skip |= ValidatePhysicalDeviceImageViewMinLodFeaturesEXT(_parentObjects,
                _s713->sType,
                _s713->pNext,
                _s713->minLod);
            break;
        }
        case VK_STRUCTURE_TYPE_IMAGE_VIEW_MIN_LOD_CREATE_INFO_EXT: {
            const auto _s714 = reinterpret_cast<const VkImageViewMinLodCreateInfoEXT *>(header);
            skip |= ValidateImageViewMinLodCreateInfoEXT(_parentObjects,
                _s714->sType,
                _s714->pNext,
                _s714->minLod);
            break;
        }
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MULTI_DRAW_FEATURES_EXT: {
            const auto _s715 = reinterpret_cast<const VkPhysicalDeviceMultiDrawFeaturesEXT *>(header);
            skip |= ValidatePhysicalDeviceMultiDrawFeaturesEXT(_parentObjects,
                _s715->sType,
                _s715->pNext,
                _s715->multiDraw);
            break;
        }
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_IMAGE_2D_VIEW_OF_3D_FEATURES_EXT: {
            const auto _s716 = reinterpret_cast<const VkPhysicalDeviceImage2DViewOf3DFeaturesEXT *>(header);
            skip |= ValidatePhysicalDeviceImage2DViewOf3DFeaturesEXT(_parentObjects,
                _s716->sType,
                _s716->pNext,
                _s716->image2DViewOf3D,
                _s716->sampler2DViewOf3D);
            break;
        }
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_OPACITY_MICROMAP_FEATURES_EXT: {
            const auto _s717 = reinterpret_cast<const VkPhysicalDeviceOpacityMicromapFeaturesEXT *>(header);
            skip |= ValidatePhysicalDeviceOpacityMicromapFeaturesEXT(_parentObjects,
                _s717->sType,
                _s717->pNext,
                _s717->micromap,
                _s717->micromapCaptureReplay,
                _s717->micromapHostCommands);
            break;
        }
        case VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_TRIANGLES_OPACITY_MICROMAP_EXT: {
            const auto _s718 = reinterpret_cast<const VkAccelerationStructureTrianglesOpacityMicromapEXT *>(header);
            skip |= ValidateAccelerationStructureTrianglesOpacityMicromapEXT(_parentObjects,
                _s718->sType,
                _s718->pNext,
                _s718->indexType,
                _s718->indexBuffer,
                _s718->indexStride,
                _s718->baseTriangle,
                _s718->usageCountsCount,
                _s718->pUsageCounts,
                _s718->ppUsageCounts,
                _s718->micromap);
            break;
        }
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_CLUSTER_CULLING_SHADER_FEATURES_HUAWEI: {
            const auto _s719 = reinterpret_cast<const VkPhysicalDeviceClusterCullingShaderFeaturesHUAWEI *>(header);
            skip |= ValidatePhysicalDeviceClusterCullingShaderFeaturesHUAWEI(_parentObjects,
                _s719->sType,
                _s719->pNext,
                _s719->clustercullingShader,
                _s719->multiviewClusterCullingShader);
            break;
        }
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_BORDER_COLOR_SWIZZLE_FEATURES_EXT: {
            const auto _s720 = reinterpret_cast<const VkPhysicalDeviceBorderColorSwizzleFeaturesEXT *>(header);
            skip |= ValidatePhysicalDeviceBorderColorSwizzleFeaturesEXT(_parentObjects,
                _s720->sType,
                _s720->pNext,
                _s720->borderColorSwizzle,
                _s720->borderColorSwizzleFromImage);
            break;
        }
        case VK_STRUCTURE_TYPE_SAMPLER_BORDER_COLOR_COMPONENT_MAPPING_CREATE_INFO_EXT: {
            const auto _s721 = reinterpret_cast<const VkSamplerBorderColorComponentMappingCreateInfoEXT *>(header);
            skip |= ValidateSamplerBorderColorComponentMappingCreateInfoEXT(_parentObjects,
                _s721->sType,
                _s721->pNext,
                _s721->components,
                _s721->srgb);
            break;
        }
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PAGEABLE_DEVICE_LOCAL_MEMORY_FEATURES_EXT: {
            const auto _s722 = reinterpret_cast<const VkPhysicalDevicePageableDeviceLocalMemoryFeaturesEXT *>(header);
            skip |= ValidatePhysicalDevicePageableDeviceLocalMemoryFeaturesEXT(_parentObjects,
                _s722->sType,
                _s722->pNext,
                _s722->pageableDeviceLocalMemory);
            break;
        }
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_IMAGE_SLICED_VIEW_OF_3D_FEATURES_EXT: {
            const auto _s723 = reinterpret_cast<const VkPhysicalDeviceImageSlicedViewOf3DFeaturesEXT *>(header);
            skip |= ValidatePhysicalDeviceImageSlicedViewOf3DFeaturesEXT(_parentObjects,
                _s723->sType,
                _s723->pNext,
                _s723->imageSlicedViewOf3D);
            break;
        }
        case VK_STRUCTURE_TYPE_IMAGE_VIEW_SLICED_CREATE_INFO_EXT: {
            const auto _s724 = reinterpret_cast<const VkImageViewSlicedCreateInfoEXT *>(header);
            skip |= ValidateImageViewSlicedCreateInfoEXT(_parentObjects,
                _s724->sType,
                _s724->pNext,
                _s724->sliceOffset,
                _s724->sliceCount);
            break;
        }
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_SET_HOST_MAPPING_FEATURES_VALVE: {
            const auto _s725 = reinterpret_cast<const VkPhysicalDeviceDescriptorSetHostMappingFeaturesVALVE *>(header);
            skip |= ValidatePhysicalDeviceDescriptorSetHostMappingFeaturesVALVE(_parentObjects,
                _s725->sType,
                _s725->pNext,
                _s725->descriptorSetHostMapping);
            break;
        }
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DEPTH_CLAMP_ZERO_ONE_FEATURES_EXT: {
            const auto _s726 = reinterpret_cast<const VkPhysicalDeviceDepthClampZeroOneFeaturesEXT *>(header);
            skip |= ValidatePhysicalDeviceDepthClampZeroOneFeaturesEXT(_parentObjects,
                _s726->sType,
                _s726->pNext,
                _s726->depthClampZeroOne);
            break;
        }
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_NON_SEAMLESS_CUBE_MAP_FEATURES_EXT: {
            const auto _s727 = reinterpret_cast<const VkPhysicalDeviceNonSeamlessCubeMapFeaturesEXT *>(header);
            skip |= ValidatePhysicalDeviceNonSeamlessCubeMapFeaturesEXT(_parentObjects,
                _s727->sType,
                _s727->pNext,
                _s727->nonSeamlessCubeMap);
            break;
        }
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FRAGMENT_DENSITY_MAP_OFFSET_FEATURES_QCOM: {
            const auto _s728 = reinterpret_cast<const VkPhysicalDeviceFragmentDensityMapOffsetFeaturesQCOM *>(header);
            skip |= ValidatePhysicalDeviceFragmentDensityMapOffsetFeaturesQCOM(_parentObjects,
                _s728->sType,
                _s728->pNext,
                _s728->fragmentDensityMapOffset);
            break;
        }
        case VK_STRUCTURE_TYPE_SUBPASS_FRAGMENT_DENSITY_MAP_OFFSET_END_INFO_QCOM: {
            const auto _s729 = reinterpret_cast<const VkSubpassFragmentDensityMapOffsetEndInfoQCOM *>(header);
            skip |= ValidateSubpassFragmentDensityMapOffsetEndInfoQCOM(_parentObjects,
                _s729->sType,
                _s729->pNext,
                _s729->fragmentDensityOffsetCount,
                _s729->pFragmentDensityOffsets);
            break;
        }
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_COPY_MEMORY_INDIRECT_FEATURES_NV: {
            const auto _s730 = reinterpret_cast<const VkPhysicalDeviceCopyMemoryIndirectFeaturesNV *>(header);
            skip |= ValidatePhysicalDeviceCopyMemoryIndirectFeaturesNV(_parentObjects,
                _s730->sType,
                _s730->pNext,
                _s730->indirectCopy);
            break;
        }
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MEMORY_DECOMPRESSION_FEATURES_NV: {
            const auto _s731 = reinterpret_cast<const VkPhysicalDeviceMemoryDecompressionFeaturesNV *>(header);
            skip |= ValidatePhysicalDeviceMemoryDecompressionFeaturesNV(_parentObjects,
                _s731->sType,
                _s731->pNext,
                _s731->memoryDecompression);
            break;
        }
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_LINEAR_COLOR_ATTACHMENT_FEATURES_NV: {
            const auto _s732 = reinterpret_cast<const VkPhysicalDeviceLinearColorAttachmentFeaturesNV *>(header);
            skip |= ValidatePhysicalDeviceLinearColorAttachmentFeaturesNV(_parentObjects,
                _s732->sType,
                _s732->pNext,
                _s732->linearColorAttachment);
            break;
        }
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_IMAGE_COMPRESSION_CONTROL_SWAPCHAIN_FEATURES_EXT: {
            const auto _s733 = reinterpret_cast<const VkPhysicalDeviceImageCompressionControlSwapchainFeaturesEXT *>(header);
            skip |= ValidatePhysicalDeviceImageCompressionControlSwapchainFeaturesEXT(_parentObjects,
                _s733->sType,
                _s733->pNext,
                _s733->imageCompressionControlSwapchain);
            break;
        }
        case VK_STRUCTURE_TYPE_IMAGE_VIEW_SAMPLE_WEIGHT_CREATE_INFO_QCOM: {
            const auto _s734 = reinterpret_cast<const VkImageViewSampleWeightCreateInfoQCOM *>(header);
            skip |= ValidateImageViewSampleWeightCreateInfoQCOM(_parentObjects,
                _s734->sType,
                _s734->pNext,
                _s734->filterCenter,
                _s734->filterSize,
                _s734->numPhases);
            break;
        }
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_IMAGE_PROCESSING_FEATURES_QCOM: {
            const auto _s735 = reinterpret_cast<const VkPhysicalDeviceImageProcessingFeaturesQCOM *>(header);
            skip |= ValidatePhysicalDeviceImageProcessingFeaturesQCOM(_parentObjects,
                _s735->sType,
                _s735->pNext,
                _s735->textureSampleWeighted,
                _s735->textureBoxFilter,
                _s735->textureBlockMatch);
            break;
        }
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_EXTENDED_DYNAMIC_STATE_3_FEATURES_EXT: {
            const auto _s736 = reinterpret_cast<const VkPhysicalDeviceExtendedDynamicState3FeaturesEXT *>(header);
            skip |= ValidatePhysicalDeviceExtendedDynamicState3FeaturesEXT(_parentObjects,
                _s736->sType,
                _s736->pNext,
                _s736->extendedDynamicState3TessellationDomainOrigin,
                _s736->extendedDynamicState3DepthClampEnable,
                _s736->extendedDynamicState3PolygonMode,
                _s736->extendedDynamicState3RasterizationSamples,
                _s736->extendedDynamicState3SampleMask,
                _s736->extendedDynamicState3AlphaToCoverageEnable,
                _s736->extendedDynamicState3AlphaToOneEnable,
                _s736->extendedDynamicState3LogicOpEnable,
                _s736->extendedDynamicState3ColorBlendEnable,
                _s736->extendedDynamicState3ColorBlendEquation,
                _s736->extendedDynamicState3ColorWriteMask,
                _s736->extendedDynamicState3RasterizationStream,
                _s736->extendedDynamicState3ConservativeRasterizationMode,
                _s736->extendedDynamicState3ExtraPrimitiveOverestimationSize,
                _s736->extendedDynamicState3DepthClipEnable,
                _s736->extendedDynamicState3SampleLocationsEnable,
                _s736->extendedDynamicState3ColorBlendAdvanced,
                _s736->extendedDynamicState3ProvokingVertexMode,
                _s736->extendedDynamicState3LineRasterizationMode,
                _s736->extendedDynamicState3LineStippleEnable,
                _s736->extendedDynamicState3DepthClipNegativeOneToOne,
                _s736->extendedDynamicState3ViewportWScalingEnable,
                _s736->extendedDynamicState3ViewportSwizzle,
                _s736->extendedDynamicState3CoverageToColorEnable,
                _s736->extendedDynamicState3CoverageToColorLocation,
                _s736->extendedDynamicState3CoverageModulationMode,
                _s736->extendedDynamicState3CoverageModulationTableEnable,
                _s736->extendedDynamicState3CoverageModulationTable,
                _s736->extendedDynamicState3CoverageReductionMode,
                _s736->extendedDynamicState3RepresentativeFragmentTestEnable,
                _s736->extendedDynamicState3ShadingRateImageEnable);
            break;
        }
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_EXTENDED_DYNAMIC_STATE_3_PROPERTIES_EXT: {
            const auto _s737 = reinterpret_cast<const VkPhysicalDeviceExtendedDynamicState3PropertiesEXT *>(header);
            skip |= ValidatePhysicalDeviceExtendedDynamicState3PropertiesEXT(_parentObjects,
                _s737->sType,
                _s737->pNext,
                _s737->dynamicPrimitiveTopologyUnrestricted);
            break;
        }
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SUBPASS_MERGE_FEEDBACK_FEATURES_EXT: {
            const auto _s738 = reinterpret_cast<const VkPhysicalDeviceSubpassMergeFeedbackFeaturesEXT *>(header);
            skip |= ValidatePhysicalDeviceSubpassMergeFeedbackFeaturesEXT(_parentObjects,
                _s738->sType,
                _s738->pNext,
                _s738->subpassMergeFeedback);
            break;
        }
        case VK_STRUCTURE_TYPE_RENDER_PASS_CREATION_CONTROL_EXT: {
            const auto _s739 = reinterpret_cast<const VkRenderPassCreationControlEXT *>(header);
            skip |= ValidateRenderPassCreationControlEXT(_parentObjects,
                _s739->sType,
                _s739->pNext,
                _s739->disallowMerging);
            break;
        }
        case VK_STRUCTURE_TYPE_RENDER_PASS_CREATION_FEEDBACK_CREATE_INFO_EXT: {
            const auto _s740 = reinterpret_cast<const VkRenderPassCreationFeedbackCreateInfoEXT *>(header);
            skip |= ValidateRenderPassCreationFeedbackCreateInfoEXT(_parentObjects,
                _s740->sType,
                _s740->pNext,
                _s740->pRenderPassFeedback);
            break;
        }
        case VK_STRUCTURE_TYPE_RENDER_PASS_SUBPASS_FEEDBACK_CREATE_INFO_EXT: {
            const auto _s741 = reinterpret_cast<const VkRenderPassSubpassFeedbackCreateInfoEXT *>(header);
            skip |= ValidateRenderPassSubpassFeedbackCreateInfoEXT(_parentObjects,
                _s741->sType,
                _s741->pNext,
                _s741->pSubpassFeedback);
            break;
        }
        case VK_STRUCTURE_TYPE_DIRECT_DRIVER_LOADING_LIST_LUNARG: {
            const auto _s742 = reinterpret_cast<const VkDirectDriverLoadingListLUNARG *>(header);
            skip |= ValidateDirectDriverLoadingListLUNARG(_parentObjects,
                _s742->sType,
                _s742->pNext,
                _s742->mode,
                _s742->driverCount,
                _s742->pDrivers);
            break;
        }
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_MODULE_IDENTIFIER_FEATURES_EXT: {
            const auto _s743 = reinterpret_cast<const VkPhysicalDeviceShaderModuleIdentifierFeaturesEXT *>(header);
            skip |= ValidatePhysicalDeviceShaderModuleIdentifierFeaturesEXT(_parentObjects,
                _s743->sType,
                _s743->pNext,
                _s743->shaderModuleIdentifier);
            break;
        }
        case VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_MODULE_IDENTIFIER_CREATE_INFO_EXT: {
            const auto _s744 = reinterpret_cast<const VkPipelineShaderStageModuleIdentifierCreateInfoEXT *>(header);
            skip |= ValidatePipelineShaderStageModuleIdentifierCreateInfoEXT(_parentObjects,
                _s744->sType,
                _s744->pNext,
                _s744->identifierSize,
                _s744->pIdentifier);
            break;
        }
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_OPTICAL_FLOW_FEATURES_NV: {
            const auto _s745 = reinterpret_cast<const VkPhysicalDeviceOpticalFlowFeaturesNV *>(header);
            skip |= ValidatePhysicalDeviceOpticalFlowFeaturesNV(_parentObjects,
                _s745->sType,
                _s745->pNext,
                _s745->opticalFlow);
            break;
        }
        case VK_STRUCTURE_TYPE_OPTICAL_FLOW_IMAGE_FORMAT_INFO_NV: {
            const auto _s746 = reinterpret_cast<const VkOpticalFlowImageFormatInfoNV *>(header);
            skip |= ValidateOpticalFlowImageFormatInfoNV(_parentObjects,
                _s746->sType,
                _s746->pNext,
                _s746->usage);
            break;
        }
        case VK_STRUCTURE_TYPE_OPTICAL_FLOW_SESSION_CREATE_PRIVATE_DATA_INFO_NV: {
            const auto _s747 = reinterpret_cast<const VkOpticalFlowSessionCreatePrivateDataInfoNV *>(header);
            skip |= ValidateOpticalFlowSessionCreatePrivateDataInfoNV(_parentObjects,
                _s747->sType,
                _s747->pNext,
                _s747->id,
                _s747->size,
                _s747->pPrivateData);
            break;
        }
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_LEGACY_DITHERING_FEATURES_EXT: {
            const auto _s748 = reinterpret_cast<const VkPhysicalDeviceLegacyDitheringFeaturesEXT *>(header);
            skip |= ValidatePhysicalDeviceLegacyDitheringFeaturesEXT(_parentObjects,
                _s748->sType,
                _s748->pNext,
                _s748->legacyDithering);
            break;
        }
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PIPELINE_PROTECTED_ACCESS_FEATURES_EXT: {
            const auto _s749 = reinterpret_cast<const VkPhysicalDevicePipelineProtectedAccessFeaturesEXT *>(header);
            skip |= ValidatePhysicalDevicePipelineProtectedAccessFeaturesEXT(_parentObjects,
                _s749->sType,
                _s749->pNext,
                _s749->pipelineProtectedAccess);
            break;
        }
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_TILE_PROPERTIES_FEATURES_QCOM: {
            const auto _s750 = reinterpret_cast<const VkPhysicalDeviceTilePropertiesFeaturesQCOM *>(header);
            skip |= ValidatePhysicalDeviceTilePropertiesFeaturesQCOM(_parentObjects,
                _s750->sType,
                _s750->pNext,
                _s750->tileProperties);
            break;
        }
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_AMIGO_PROFILING_FEATURES_SEC: {
            const auto _s751 = reinterpret_cast<const VkPhysicalDeviceAmigoProfilingFeaturesSEC *>(header);
            skip |= ValidatePhysicalDeviceAmigoProfilingFeaturesSEC(_parentObjects,
                _s751->sType,
                _s751->pNext,
                _s751->amigoProfiling);
            break;
        }
        case VK_STRUCTURE_TYPE_AMIGO_PROFILING_SUBMIT_INFO_SEC: {
            const auto _s752 = reinterpret_cast<const VkAmigoProfilingSubmitInfoSEC *>(header);
            skip |= ValidateAmigoProfilingSubmitInfoSEC(_parentObjects,
                _s752->sType,
                _s752->pNext,
                _s752->firstDrawTimestamp,
                _s752->swapBufferTimestamp);
            break;
        }
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MULTIVIEW_PER_VIEW_VIEWPORTS_FEATURES_QCOM: {
            const auto _s753 = reinterpret_cast<const VkPhysicalDeviceMultiviewPerViewViewportsFeaturesQCOM *>(header);
            skip |= ValidatePhysicalDeviceMultiviewPerViewViewportsFeaturesQCOM(_parentObjects,
                _s753->sType,
                _s753->pNext,
                _s753->multiviewPerViewViewports);
            break;
        }
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_INVOCATION_REORDER_FEATURES_NV: {
            const auto _s754 = reinterpret_cast<const VkPhysicalDeviceRayTracingInvocationReorderFeaturesNV *>(header);
            skip |= ValidatePhysicalDeviceRayTracingInvocationReorderFeaturesNV(_parentObjects,
                _s754->sType,
                _s754->pNext,
                _s754->rayTracingInvocationReorder);
            break;
        }
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_CORE_BUILTINS_FEATURES_ARM: {
            const auto _s755 = reinterpret_cast<const VkPhysicalDeviceShaderCoreBuiltinsFeaturesARM *>(header);
            skip |= ValidatePhysicalDeviceShaderCoreBuiltinsFeaturesARM(_parentObjects,
                _s755->sType,
                _s755->pNext,
                _s755->shaderCoreBuiltins);
            break;
        }
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PIPELINE_LIBRARY_GROUP_HANDLES_FEATURES_EXT: {
            const auto _s756 = reinterpret_cast<const VkPhysicalDevicePipelineLibraryGroupHandlesFeaturesEXT *>(header);
            skip |= ValidatePhysicalDevicePipelineLibraryGroupHandlesFeaturesEXT(_parentObjects,
                _s756->sType,
                _s756->pNext,
                _s756->pipelineLibraryGroupHandles);
            break;
        }
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MULTIVIEW_PER_VIEW_RENDER_AREAS_FEATURES_QCOM: {
            const auto _s757 = reinterpret_cast<const VkPhysicalDeviceMultiviewPerViewRenderAreasFeaturesQCOM *>(header);
            skip |= ValidatePhysicalDeviceMultiviewPerViewRenderAreasFeaturesQCOM(_parentObjects,
                _s757->sType,
                _s757->pNext,
                _s757->multiviewPerViewRenderAreas);
            break;
        }
        case VK_STRUCTURE_TYPE_MULTIVIEW_PER_VIEW_RENDER_AREAS_RENDER_PASS_BEGIN_INFO_QCOM: {
            const auto _s758 = reinterpret_cast<const VkMultiviewPerViewRenderAreasRenderPassBeginInfoQCOM *>(header);
            skip |= ValidateMultiviewPerViewRenderAreasRenderPassBeginInfoQCOM(_parentObjects,
                _s758->sType,
                _s758->pNext,
                _s758->perViewRenderAreaCount,
                _s758->pPerViewRenderAreas);
            break;
        }
        case VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET_ACCELERATION_STRUCTURE_KHR: {
            const auto _s759 = reinterpret_cast<const VkWriteDescriptorSetAccelerationStructureKHR *>(header);
            skip |= ValidateWriteDescriptorSetAccelerationStructureKHR(_parentObjects,
                _s759->sType,
                _s759->pNext,
                _s759->accelerationStructureCount,
                _s759->pAccelerationStructures);
            break;
        }
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ACCELERATION_STRUCTURE_FEATURES_KHR: {
            const auto _s760 = reinterpret_cast<const VkPhysicalDeviceAccelerationStructureFeaturesKHR *>(header);
            skip |= ValidatePhysicalDeviceAccelerationStructureFeaturesKHR(_parentObjects,
                _s760->sType,
                _s760->pNext,
                _s760->accelerationStructure,
                _s760->accelerationStructureCaptureReplay,
                _s760->accelerationStructureIndirectBuild,
                _s760->accelerationStructureHostCommands,
                _s760->descriptorBindingAccelerationStructureUpdateAfterBind);
            break;
        }
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_PIPELINE_FEATURES_KHR: {
            const auto _s761 = reinterpret_cast<const VkPhysicalDeviceRayTracingPipelineFeaturesKHR *>(header);
            skip |= ValidatePhysicalDeviceRayTracingPipelineFeaturesKHR(_parentObjects,
                _s761->sType,
                _s761->pNext,
                _s761->rayTracingPipeline,
                _s761->rayTracingPipelineShaderGroupHandleCaptureReplay,
                _s761->rayTracingPipelineShaderGroupHandleCaptureReplayMixed,
                _s761->rayTracingPipelineTraceRaysIndirect,
                _s761->rayTraversalPrimitiveCulling);
            break;
        }
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_QUERY_FEATURES_KHR: {
            const auto _s762 = reinterpret_cast<const VkPhysicalDeviceRayQueryFeaturesKHR *>(header);
            skip |= ValidatePhysicalDeviceRayQueryFeaturesKHR(_parentObjects,
                _s762->sType,
                _s762->pNext,
                _s762->rayQuery);
            break;
        }
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MESH_SHADER_FEATURES_EXT: {
            const auto _s763 = reinterpret_cast<const VkPhysicalDeviceMeshShaderFeaturesEXT *>(header);
            skip |= ValidatePhysicalDeviceMeshShaderFeaturesEXT(_parentObjects,
                _s763->sType,
                _s763->pNext,
                _s763->taskShader,
                _s763->meshShader,
                _s763->multiviewMeshShader,
                _s763->primitiveFragmentShadingRateMeshShader,
                _s763->meshShaderQueries);
            break;
        }
        default:
            break;
        }
        header = header->pNext;
    }
    return skip;
}

