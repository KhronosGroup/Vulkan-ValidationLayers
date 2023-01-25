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

bool ExplicitValidation::PreCallValidateCreateInstance(const VkInstanceCreateInfo* pCreateInfo,
                const VkAllocationCallbacks* pAllocator,
                VkInstance* pInstance) const {
    bool skip = false;
    if (pCreateInfo != nullptr) {
        const auto _s1 = pCreateInfo;
        skip |= ValidateInstanceCreateInfo({},
            _s1->sType,
            _s1->pNext,
            _s1->flags,
            _s1->pApplicationInfo,
            _s1->enabledLayerCount,
            _s1->ppEnabledLayerNames,
            _s1->enabledExtensionCount,
            _s1->ppEnabledExtensionNames);
    }
    if (pAllocator != nullptr) {
        const auto _s2 = pAllocator;
        skip |= ValidateAllocationCallbacks({},
            _s2->pUserData,
            _s2->pfnAllocation,
            _s2->pfnReallocation,
            _s2->pfnFree,
            _s2->pfnInternalAllocation,
            _s2->pfnInternalFree);
    }
    return skip;
}
bool ExplicitValidation::PreCallValidateDestroyInstance(VkInstance instance,
                const VkAllocationCallbacks* pAllocator) const {
    bool skip = false;
    if (pAllocator != nullptr) {
        const auto _s3 = pAllocator;
        skip |= ValidateAllocationCallbacks({},
            _s3->pUserData,
            _s3->pfnAllocation,
            _s3->pfnReallocation,
            _s3->pfnFree,
            _s3->pfnInternalAllocation,
            _s3->pfnInternalFree);
    }
    return skip;
}
bool ExplicitValidation::PreCallValidateEnumeratePhysicalDevices(VkInstance instance,
                uint32_t* pPhysicalDeviceCount,
                VkPhysicalDevice* pPhysicalDevices) const {
    bool skip = false;
    return skip;
}
bool ExplicitValidation::PreCallValidateGetPhysicalDeviceFeatures(VkPhysicalDevice physicalDevice,
                VkPhysicalDeviceFeatures* pFeatures) const {
    bool skip = false;
    if (pFeatures != nullptr) {
        const auto _s4 = pFeatures;
        skip |= ValidatePhysicalDeviceFeatures({},
            _s4->robustBufferAccess,
            _s4->fullDrawIndexUint32,
            _s4->imageCubeArray,
            _s4->independentBlend,
            _s4->geometryShader,
            _s4->tessellationShader,
            _s4->sampleRateShading,
            _s4->dualSrcBlend,
            _s4->logicOp,
            _s4->multiDrawIndirect,
            _s4->drawIndirectFirstInstance,
            _s4->depthClamp,
            _s4->depthBiasClamp,
            _s4->fillModeNonSolid,
            _s4->depthBounds,
            _s4->wideLines,
            _s4->largePoints,
            _s4->alphaToOne,
            _s4->multiViewport,
            _s4->samplerAnisotropy,
            _s4->textureCompressionETC2,
            _s4->textureCompressionASTC_LDR,
            _s4->textureCompressionBC,
            _s4->occlusionQueryPrecise,
            _s4->pipelineStatisticsQuery,
            _s4->vertexPipelineStoresAndAtomics,
            _s4->fragmentStoresAndAtomics,
            _s4->shaderTessellationAndGeometryPointSize,
            _s4->shaderImageGatherExtended,
            _s4->shaderStorageImageExtendedFormats,
            _s4->shaderStorageImageMultisample,
            _s4->shaderStorageImageReadWithoutFormat,
            _s4->shaderStorageImageWriteWithoutFormat,
            _s4->shaderUniformBufferArrayDynamicIndexing,
            _s4->shaderSampledImageArrayDynamicIndexing,
            _s4->shaderStorageBufferArrayDynamicIndexing,
            _s4->shaderStorageImageArrayDynamicIndexing,
            _s4->shaderClipDistance,
            _s4->shaderCullDistance,
            _s4->shaderFloat64,
            _s4->shaderInt64,
            _s4->shaderInt16,
            _s4->shaderResourceResidency,
            _s4->shaderResourceMinLod,
            _s4->sparseBinding,
            _s4->sparseResidencyBuffer,
            _s4->sparseResidencyImage2D,
            _s4->sparseResidencyImage3D,
            _s4->sparseResidency2Samples,
            _s4->sparseResidency4Samples,
            _s4->sparseResidency8Samples,
            _s4->sparseResidency16Samples,
            _s4->sparseResidencyAliased,
            _s4->variableMultisampleRate,
            _s4->inheritedQueries);
    }
    return skip;
}
bool ExplicitValidation::PreCallValidateGetPhysicalDeviceFormatProperties(VkPhysicalDevice physicalDevice,
                VkFormat format,
                VkFormatProperties* pFormatProperties) const {
    bool skip = false;
    return skip;
}
bool ExplicitValidation::PreCallValidateGetPhysicalDeviceImageFormatProperties(VkPhysicalDevice physicalDevice,
                VkFormat format,
                VkImageType type,
                VkImageTiling tiling,
                VkImageUsageFlags usage,
                VkImageCreateFlags flags,
                VkImageFormatProperties* pImageFormatProperties) const {
    bool skip = false;
    return skip;
}
bool ExplicitValidation::PreCallValidateGetPhysicalDeviceProperties(VkPhysicalDevice physicalDevice,
                VkPhysicalDeviceProperties* pProperties) const {
    bool skip = false;
    return skip;
}
bool ExplicitValidation::PreCallValidateGetPhysicalDeviceQueueFamilyProperties(VkPhysicalDevice physicalDevice,
                uint32_t* pQueueFamilyPropertyCount,
                VkQueueFamilyProperties* pQueueFamilyProperties) const {
    bool skip = false;
    return skip;
}
bool ExplicitValidation::PreCallValidateGetPhysicalDeviceMemoryProperties(VkPhysicalDevice physicalDevice,
                VkPhysicalDeviceMemoryProperties* pMemoryProperties) const {
    bool skip = false;
    return skip;
}
bool ExplicitValidation::PreCallValidateGetInstanceProcAddr(VkInstance instance,
                const char* pName) const {
    bool skip = false;
    return skip;
}
bool ExplicitValidation::PreCallValidateGetDeviceProcAddr(VkDevice device,
                const char* pName) const {
    bool skip = false;
    return skip;
}
bool ExplicitValidation::PreCallValidateCreateDevice(VkPhysicalDevice physicalDevice,
                const VkDeviceCreateInfo* pCreateInfo,
                const VkAllocationCallbacks* pAllocator,
                VkDevice* pDevice) const {
    bool skip = false;
    if (pCreateInfo != nullptr) {
        const auto _s5 = pCreateInfo;
        skip |= ValidateDeviceCreateInfo({},
            _s5->sType,
            _s5->pNext,
            _s5->flags,
            _s5->queueCreateInfoCount,
            _s5->pQueueCreateInfos,
            _s5->enabledLayerCount,
            _s5->ppEnabledLayerNames,
            _s5->enabledExtensionCount,
            _s5->ppEnabledExtensionNames,
            _s5->pEnabledFeatures);
    }
    if (pAllocator != nullptr) {
        const auto _s6 = pAllocator;
        skip |= ValidateAllocationCallbacks({},
            _s6->pUserData,
            _s6->pfnAllocation,
            _s6->pfnReallocation,
            _s6->pfnFree,
            _s6->pfnInternalAllocation,
            _s6->pfnInternalFree);
    }
    return skip;
}
bool ExplicitValidation::PreCallValidateDestroyDevice(VkDevice device,
                const VkAllocationCallbacks* pAllocator) const {
    bool skip = false;
    if (pAllocator != nullptr) {
        const auto _s7 = pAllocator;
        skip |= ValidateAllocationCallbacks({},
            _s7->pUserData,
            _s7->pfnAllocation,
            _s7->pfnReallocation,
            _s7->pfnFree,
            _s7->pfnInternalAllocation,
            _s7->pfnInternalFree);
    }
    return skip;
}
bool ExplicitValidation::PreCallValidateEnumerateInstanceExtensionProperties(const char* pLayerName,
                uint32_t* pPropertyCount,
                VkExtensionProperties* pProperties) const {
    bool skip = false;
    return skip;
}
bool ExplicitValidation::PreCallValidateEnumerateDeviceExtensionProperties(VkPhysicalDevice physicalDevice,
                const char* pLayerName,
                uint32_t* pPropertyCount,
                VkExtensionProperties* pProperties) const {
    bool skip = false;
    return skip;
}
bool ExplicitValidation::PreCallValidateEnumerateInstanceLayerProperties(uint32_t* pPropertyCount,
                VkLayerProperties* pProperties) const {
    bool skip = false;
    return skip;
}
bool ExplicitValidation::PreCallValidateEnumerateDeviceLayerProperties(VkPhysicalDevice physicalDevice,
                uint32_t* pPropertyCount,
                VkLayerProperties* pProperties) const {
    bool skip = false;
    return skip;
}
bool ExplicitValidation::PreCallValidateGetDeviceQueue(VkDevice device,
                uint32_t queueFamilyIndex,
                uint32_t queueIndex,
                VkQueue* pQueue) const {
    bool skip = false;
    return skip;
}
bool ExplicitValidation::PreCallValidateQueueSubmit(VkQueue queue,
                uint32_t submitCount,
                const VkSubmitInfo* pSubmits,
                VkFence fence) const {
    bool skip = false;
    if (pSubmits != nullptr) {
        for (uint32_t _i8 = 0;_i8 < submitCount; ++_i8) {
            const auto _s9 = &pSubmits[_i8];
            skip |= ValidateSubmitInfo({},
                _s9->sType,
                _s9->pNext,
                _s9->waitSemaphoreCount,
                _s9->pWaitSemaphores,
                _s9->pWaitDstStageMask,
                _s9->commandBufferCount,
                _s9->pCommandBuffers,
                _s9->signalSemaphoreCount,
                _s9->pSignalSemaphores);
        }
    }
    return skip;
}
bool ExplicitValidation::PreCallValidateQueueWaitIdle(VkQueue queue) const {
    bool skip = false;
    return skip;
}
bool ExplicitValidation::PreCallValidateDeviceWaitIdle(VkDevice device) const {
    bool skip = false;
    return skip;
}
bool ExplicitValidation::PreCallValidateAllocateMemory(VkDevice device,
                const VkMemoryAllocateInfo* pAllocateInfo,
                const VkAllocationCallbacks* pAllocator,
                VkDeviceMemory* pMemory) const {
    bool skip = false;
    if (pAllocateInfo != nullptr) {
        const auto _s10 = pAllocateInfo;
        skip |= ValidateMemoryAllocateInfo({},
            _s10->sType,
            _s10->pNext,
            _s10->allocationSize,
            _s10->memoryTypeIndex);
    }
    if (pAllocator != nullptr) {
        const auto _s11 = pAllocator;
        skip |= ValidateAllocationCallbacks({},
            _s11->pUserData,
            _s11->pfnAllocation,
            _s11->pfnReallocation,
            _s11->pfnFree,
            _s11->pfnInternalAllocation,
            _s11->pfnInternalFree);
    }
    return skip;
}
bool ExplicitValidation::PreCallValidateFreeMemory(VkDevice device,
                VkDeviceMemory memory,
                const VkAllocationCallbacks* pAllocator) const {
    bool skip = false;
    if (pAllocator != nullptr) {
        const auto _s12 = pAllocator;
        skip |= ValidateAllocationCallbacks({},
            _s12->pUserData,
            _s12->pfnAllocation,
            _s12->pfnReallocation,
            _s12->pfnFree,
            _s12->pfnInternalAllocation,
            _s12->pfnInternalFree);
    }
    return skip;
}
bool ExplicitValidation::PreCallValidateMapMemory(VkDevice device,
                VkDeviceMemory memory,
                VkDeviceSize offset,
                VkDeviceSize size,
                VkMemoryMapFlags flags,
                void** ppData) const {
    bool skip = false;
    return skip;
}
bool ExplicitValidation::PreCallValidateUnmapMemory(VkDevice device,
                VkDeviceMemory memory) const {
    bool skip = false;
    return skip;
}
bool ExplicitValidation::PreCallValidateFlushMappedMemoryRanges(VkDevice device,
                uint32_t memoryRangeCount,
                const VkMappedMemoryRange* pMemoryRanges) const {
    bool skip = false;
    if (pMemoryRanges != nullptr) {
        for (uint32_t _i13 = 0;_i13 < memoryRangeCount; ++_i13) {
            const auto _s14 = &pMemoryRanges[_i13];
            skip |= ValidateMappedMemoryRange({},
                _s14->sType,
                _s14->pNext,
                _s14->memory,
                _s14->offset,
                _s14->size);
        }
    }
    return skip;
}
bool ExplicitValidation::PreCallValidateInvalidateMappedMemoryRanges(VkDevice device,
                uint32_t memoryRangeCount,
                const VkMappedMemoryRange* pMemoryRanges) const {
    bool skip = false;
    if (pMemoryRanges != nullptr) {
        for (uint32_t _i15 = 0;_i15 < memoryRangeCount; ++_i15) {
            const auto _s16 = &pMemoryRanges[_i15];
            skip |= ValidateMappedMemoryRange({},
                _s16->sType,
                _s16->pNext,
                _s16->memory,
                _s16->offset,
                _s16->size);
        }
    }
    return skip;
}
bool ExplicitValidation::PreCallValidateGetDeviceMemoryCommitment(VkDevice device,
                VkDeviceMemory memory,
                VkDeviceSize* pCommittedMemoryInBytes) const {
    bool skip = false;
    return skip;
}
bool ExplicitValidation::PreCallValidateBindBufferMemory(VkDevice device,
                VkBuffer buffer,
                VkDeviceMemory memory,
                VkDeviceSize memoryOffset) const {
    bool skip = false;
    return skip;
}
bool ExplicitValidation::PreCallValidateBindImageMemory(VkDevice device,
                VkImage image,
                VkDeviceMemory memory,
                VkDeviceSize memoryOffset) const {
    bool skip = false;
    return skip;
}
bool ExplicitValidation::PreCallValidateGetBufferMemoryRequirements(VkDevice device,
                VkBuffer buffer,
                VkMemoryRequirements* pMemoryRequirements) const {
    bool skip = false;
    return skip;
}
bool ExplicitValidation::PreCallValidateGetImageMemoryRequirements(VkDevice device,
                VkImage image,
                VkMemoryRequirements* pMemoryRequirements) const {
    bool skip = false;
    return skip;
}
bool ExplicitValidation::PreCallValidateGetImageSparseMemoryRequirements(VkDevice device,
                VkImage image,
                uint32_t* pSparseMemoryRequirementCount,
                VkSparseImageMemoryRequirements* pSparseMemoryRequirements) const {
    bool skip = false;
    return skip;
}
bool ExplicitValidation::PreCallValidateGetPhysicalDeviceSparseImageFormatProperties(VkPhysicalDevice physicalDevice,
                VkFormat format,
                VkImageType type,
                VkSampleCountFlagBits samples,
                VkImageUsageFlags usage,
                VkImageTiling tiling,
                uint32_t* pPropertyCount,
                VkSparseImageFormatProperties* pProperties) const {
    bool skip = false;
    return skip;
}
bool ExplicitValidation::PreCallValidateQueueBindSparse(VkQueue queue,
                uint32_t bindInfoCount,
                const VkBindSparseInfo* pBindInfo,
                VkFence fence) const {
    bool skip = false;
    if (pBindInfo != nullptr) {
        for (uint32_t _i17 = 0;_i17 < bindInfoCount; ++_i17) {
            const auto _s18 = &pBindInfo[_i17];
            skip |= ValidateBindSparseInfo({},
                _s18->sType,
                _s18->pNext,
                _s18->waitSemaphoreCount,
                _s18->pWaitSemaphores,
                _s18->bufferBindCount,
                _s18->pBufferBinds,
                _s18->imageOpaqueBindCount,
                _s18->pImageOpaqueBinds,
                _s18->imageBindCount,
                _s18->pImageBinds,
                _s18->signalSemaphoreCount,
                _s18->pSignalSemaphores);
        }
    }
    return skip;
}
bool ExplicitValidation::PreCallValidateCreateFence(VkDevice device,
                const VkFenceCreateInfo* pCreateInfo,
                const VkAllocationCallbacks* pAllocator,
                VkFence* pFence) const {
    bool skip = false;
    if (pCreateInfo != nullptr) {
        const auto _s19 = pCreateInfo;
        skip |= ValidateFenceCreateInfo({},
            _s19->sType,
            _s19->pNext,
            _s19->flags);
    }
    if (pAllocator != nullptr) {
        const auto _s20 = pAllocator;
        skip |= ValidateAllocationCallbacks({},
            _s20->pUserData,
            _s20->pfnAllocation,
            _s20->pfnReallocation,
            _s20->pfnFree,
            _s20->pfnInternalAllocation,
            _s20->pfnInternalFree);
    }
    return skip;
}
bool ExplicitValidation::PreCallValidateDestroyFence(VkDevice device,
                VkFence fence,
                const VkAllocationCallbacks* pAllocator) const {
    bool skip = false;
    if (pAllocator != nullptr) {
        const auto _s21 = pAllocator;
        skip |= ValidateAllocationCallbacks({},
            _s21->pUserData,
            _s21->pfnAllocation,
            _s21->pfnReallocation,
            _s21->pfnFree,
            _s21->pfnInternalAllocation,
            _s21->pfnInternalFree);
    }
    return skip;
}
bool ExplicitValidation::PreCallValidateResetFences(VkDevice device,
                uint32_t fenceCount,
                const VkFence* pFences) const {
    bool skip = false;
    return skip;
}
bool ExplicitValidation::PreCallValidateGetFenceStatus(VkDevice device,
                VkFence fence) const {
    bool skip = false;
    return skip;
}
bool ExplicitValidation::PreCallValidateWaitForFences(VkDevice device,
                uint32_t fenceCount,
                const VkFence* pFences,
                VkBool32 waitAll,
                uint64_t timeout) const {
    bool skip = false;
    return skip;
}
bool ExplicitValidation::PreCallValidateCreateSemaphore(VkDevice device,
                const VkSemaphoreCreateInfo* pCreateInfo,
                const VkAllocationCallbacks* pAllocator,
                VkSemaphore* pSemaphore) const {
    bool skip = false;
    if (pCreateInfo != nullptr) {
        const auto _s22 = pCreateInfo;
        skip |= ValidateSemaphoreCreateInfo({},
            _s22->sType,
            _s22->pNext,
            _s22->flags);
    }
    if (pAllocator != nullptr) {
        const auto _s23 = pAllocator;
        skip |= ValidateAllocationCallbacks({},
            _s23->pUserData,
            _s23->pfnAllocation,
            _s23->pfnReallocation,
            _s23->pfnFree,
            _s23->pfnInternalAllocation,
            _s23->pfnInternalFree);
    }
    return skip;
}
bool ExplicitValidation::PreCallValidateDestroySemaphore(VkDevice device,
                VkSemaphore semaphore,
                const VkAllocationCallbacks* pAllocator) const {
    bool skip = false;
    if (pAllocator != nullptr) {
        const auto _s24 = pAllocator;
        skip |= ValidateAllocationCallbacks({},
            _s24->pUserData,
            _s24->pfnAllocation,
            _s24->pfnReallocation,
            _s24->pfnFree,
            _s24->pfnInternalAllocation,
            _s24->pfnInternalFree);
    }
    return skip;
}
bool ExplicitValidation::PreCallValidateCreateEvent(VkDevice device,
                const VkEventCreateInfo* pCreateInfo,
                const VkAllocationCallbacks* pAllocator,
                VkEvent* pEvent) const {
    bool skip = false;
    if (pCreateInfo != nullptr) {
        const auto _s25 = pCreateInfo;
        skip |= ValidateEventCreateInfo({},
            _s25->sType,
            _s25->pNext,
            _s25->flags);
    }
    if (pAllocator != nullptr) {
        const auto _s26 = pAllocator;
        skip |= ValidateAllocationCallbacks({},
            _s26->pUserData,
            _s26->pfnAllocation,
            _s26->pfnReallocation,
            _s26->pfnFree,
            _s26->pfnInternalAllocation,
            _s26->pfnInternalFree);
    }
    return skip;
}
bool ExplicitValidation::PreCallValidateDestroyEvent(VkDevice device,
                VkEvent event,
                const VkAllocationCallbacks* pAllocator) const {
    bool skip = false;
    if (pAllocator != nullptr) {
        const auto _s27 = pAllocator;
        skip |= ValidateAllocationCallbacks({},
            _s27->pUserData,
            _s27->pfnAllocation,
            _s27->pfnReallocation,
            _s27->pfnFree,
            _s27->pfnInternalAllocation,
            _s27->pfnInternalFree);
    }
    return skip;
}
bool ExplicitValidation::PreCallValidateGetEventStatus(VkDevice device,
                VkEvent event) const {
    bool skip = false;
    return skip;
}
bool ExplicitValidation::PreCallValidateSetEvent(VkDevice device,
                VkEvent event) const {
    bool skip = false;
    return skip;
}
bool ExplicitValidation::PreCallValidateResetEvent(VkDevice device,
                VkEvent event) const {
    bool skip = false;
    return skip;
}
bool ExplicitValidation::PreCallValidateCreateQueryPool(VkDevice device,
                const VkQueryPoolCreateInfo* pCreateInfo,
                const VkAllocationCallbacks* pAllocator,
                VkQueryPool* pQueryPool) const {
    bool skip = false;
    if (pCreateInfo != nullptr) {
        const auto _s28 = pCreateInfo;
        skip |= ValidateQueryPoolCreateInfo({},
            _s28->sType,
            _s28->pNext,
            _s28->flags,
            _s28->queryType,
            _s28->queryCount,
            _s28->pipelineStatistics);
    }
    if (pAllocator != nullptr) {
        const auto _s29 = pAllocator;
        skip |= ValidateAllocationCallbacks({},
            _s29->pUserData,
            _s29->pfnAllocation,
            _s29->pfnReallocation,
            _s29->pfnFree,
            _s29->pfnInternalAllocation,
            _s29->pfnInternalFree);
    }
    return skip;
}
bool ExplicitValidation::PreCallValidateDestroyQueryPool(VkDevice device,
                VkQueryPool queryPool,
                const VkAllocationCallbacks* pAllocator) const {
    bool skip = false;
    if (pAllocator != nullptr) {
        const auto _s30 = pAllocator;
        skip |= ValidateAllocationCallbacks({},
            _s30->pUserData,
            _s30->pfnAllocation,
            _s30->pfnReallocation,
            _s30->pfnFree,
            _s30->pfnInternalAllocation,
            _s30->pfnInternalFree);
    }
    return skip;
}
bool ExplicitValidation::PreCallValidateGetQueryPoolResults(VkDevice device,
                VkQueryPool queryPool,
                uint32_t firstQuery,
                uint32_t queryCount,
                size_t dataSize,
                void* pData,
                VkDeviceSize stride,
                VkQueryResultFlags flags) const {
    bool skip = false;
    return skip;
}
bool ExplicitValidation::PreCallValidateCreateBuffer(VkDevice device,
                const VkBufferCreateInfo* pCreateInfo,
                const VkAllocationCallbacks* pAllocator,
                VkBuffer* pBuffer) const {
    bool skip = false;
    if (pCreateInfo != nullptr) {
        const auto _s31 = pCreateInfo;
        skip |= ValidateBufferCreateInfo({},
            _s31->sType,
            _s31->pNext,
            _s31->flags,
            _s31->size,
            _s31->usage,
            _s31->sharingMode,
            _s31->queueFamilyIndexCount,
            _s31->pQueueFamilyIndices);
    }
    if (pAllocator != nullptr) {
        const auto _s32 = pAllocator;
        skip |= ValidateAllocationCallbacks({},
            _s32->pUserData,
            _s32->pfnAllocation,
            _s32->pfnReallocation,
            _s32->pfnFree,
            _s32->pfnInternalAllocation,
            _s32->pfnInternalFree);
    }
    return skip;
}
bool ExplicitValidation::PreCallValidateDestroyBuffer(VkDevice device,
                VkBuffer buffer,
                const VkAllocationCallbacks* pAllocator) const {
    bool skip = false;
    if (pAllocator != nullptr) {
        const auto _s33 = pAllocator;
        skip |= ValidateAllocationCallbacks({},
            _s33->pUserData,
            _s33->pfnAllocation,
            _s33->pfnReallocation,
            _s33->pfnFree,
            _s33->pfnInternalAllocation,
            _s33->pfnInternalFree);
    }
    return skip;
}
bool ExplicitValidation::PreCallValidateCreateBufferView(VkDevice device,
                const VkBufferViewCreateInfo* pCreateInfo,
                const VkAllocationCallbacks* pAllocator,
                VkBufferView* pView) const {
    bool skip = false;
    if (pCreateInfo != nullptr) {
        const auto _s34 = pCreateInfo;
        skip |= ValidateBufferViewCreateInfo({},
            _s34->sType,
            _s34->pNext,
            _s34->flags,
            _s34->buffer,
            _s34->format,
            _s34->offset,
            _s34->range);
    }
    if (pAllocator != nullptr) {
        const auto _s35 = pAllocator;
        skip |= ValidateAllocationCallbacks({},
            _s35->pUserData,
            _s35->pfnAllocation,
            _s35->pfnReallocation,
            _s35->pfnFree,
            _s35->pfnInternalAllocation,
            _s35->pfnInternalFree);
    }
    return skip;
}
bool ExplicitValidation::PreCallValidateDestroyBufferView(VkDevice device,
                VkBufferView bufferView,
                const VkAllocationCallbacks* pAllocator) const {
    bool skip = false;
    if (pAllocator != nullptr) {
        const auto _s36 = pAllocator;
        skip |= ValidateAllocationCallbacks({},
            _s36->pUserData,
            _s36->pfnAllocation,
            _s36->pfnReallocation,
            _s36->pfnFree,
            _s36->pfnInternalAllocation,
            _s36->pfnInternalFree);
    }
    return skip;
}
bool ExplicitValidation::PreCallValidateCreateImage(VkDevice device,
                const VkImageCreateInfo* pCreateInfo,
                const VkAllocationCallbacks* pAllocator,
                VkImage* pImage) const {
    bool skip = false;
    if (pCreateInfo != nullptr) {
        const auto _s37 = pCreateInfo;
        skip |= ValidateImageCreateInfo({},
            _s37->sType,
            _s37->pNext,
            _s37->flags,
            _s37->imageType,
            _s37->format,
            _s37->extent,
            _s37->mipLevels,
            _s37->arrayLayers,
            _s37->samples,
            _s37->tiling,
            _s37->usage,
            _s37->sharingMode,
            _s37->queueFamilyIndexCount,
            _s37->pQueueFamilyIndices,
            _s37->initialLayout);
    }
    if (pAllocator != nullptr) {
        const auto _s38 = pAllocator;
        skip |= ValidateAllocationCallbacks({},
            _s38->pUserData,
            _s38->pfnAllocation,
            _s38->pfnReallocation,
            _s38->pfnFree,
            _s38->pfnInternalAllocation,
            _s38->pfnInternalFree);
    }
    return skip;
}
bool ExplicitValidation::PreCallValidateDestroyImage(VkDevice device,
                VkImage image,
                const VkAllocationCallbacks* pAllocator) const {
    bool skip = false;
    if (pAllocator != nullptr) {
        const auto _s39 = pAllocator;
        skip |= ValidateAllocationCallbacks({},
            _s39->pUserData,
            _s39->pfnAllocation,
            _s39->pfnReallocation,
            _s39->pfnFree,
            _s39->pfnInternalAllocation,
            _s39->pfnInternalFree);
    }
    return skip;
}
bool ExplicitValidation::PreCallValidateGetImageSubresourceLayout(VkDevice device,
                VkImage image,
                const VkImageSubresource* pSubresource,
                VkSubresourceLayout* pLayout) const {
    bool skip = false;
    if (pSubresource != nullptr) {
        const auto _s40 = pSubresource;
        skip |= ValidateImageSubresource({},
            _s40->aspectMask,
            _s40->mipLevel,
            _s40->arrayLayer);
    }
    if (pLayout != nullptr) {
        const auto _s41 = pLayout;
        skip |= ValidateSubresourceLayout({},
            _s41->offset,
            _s41->size,
            _s41->rowPitch,
            _s41->arrayPitch,
            _s41->depthPitch);
    }
    return skip;
}
bool ExplicitValidation::PreCallValidateCreateImageView(VkDevice device,
                const VkImageViewCreateInfo* pCreateInfo,
                const VkAllocationCallbacks* pAllocator,
                VkImageView* pView) const {
    bool skip = false;
    if (pCreateInfo != nullptr) {
        const auto _s42 = pCreateInfo;
        skip |= ValidateImageViewCreateInfo({},
            _s42->sType,
            _s42->pNext,
            _s42->flags,
            _s42->image,
            _s42->viewType,
            _s42->format,
            _s42->components,
            _s42->subresourceRange);
    }
    if (pAllocator != nullptr) {
        const auto _s43 = pAllocator;
        skip |= ValidateAllocationCallbacks({},
            _s43->pUserData,
            _s43->pfnAllocation,
            _s43->pfnReallocation,
            _s43->pfnFree,
            _s43->pfnInternalAllocation,
            _s43->pfnInternalFree);
    }
    return skip;
}
bool ExplicitValidation::PreCallValidateDestroyImageView(VkDevice device,
                VkImageView imageView,
                const VkAllocationCallbacks* pAllocator) const {
    bool skip = false;
    if (pAllocator != nullptr) {
        const auto _s44 = pAllocator;
        skip |= ValidateAllocationCallbacks({},
            _s44->pUserData,
            _s44->pfnAllocation,
            _s44->pfnReallocation,
            _s44->pfnFree,
            _s44->pfnInternalAllocation,
            _s44->pfnInternalFree);
    }
    return skip;
}
bool ExplicitValidation::PreCallValidateCreateShaderModule(VkDevice device,
                const VkShaderModuleCreateInfo* pCreateInfo,
                const VkAllocationCallbacks* pAllocator,
                VkShaderModule* pShaderModule) const {
    bool skip = false;
    if (pCreateInfo != nullptr) {
        const auto _s45 = pCreateInfo;
        skip |= ValidateShaderModuleCreateInfo({},
            _s45->sType,
            _s45->pNext,
            _s45->flags,
            _s45->codeSize,
            _s45->pCode);
    }
    if (pAllocator != nullptr) {
        const auto _s46 = pAllocator;
        skip |= ValidateAllocationCallbacks({},
            _s46->pUserData,
            _s46->pfnAllocation,
            _s46->pfnReallocation,
            _s46->pfnFree,
            _s46->pfnInternalAllocation,
            _s46->pfnInternalFree);
    }
    return skip;
}
bool ExplicitValidation::PreCallValidateDestroyShaderModule(VkDevice device,
                VkShaderModule shaderModule,
                const VkAllocationCallbacks* pAllocator) const {
    bool skip = false;
    if (pAllocator != nullptr) {
        const auto _s47 = pAllocator;
        skip |= ValidateAllocationCallbacks({},
            _s47->pUserData,
            _s47->pfnAllocation,
            _s47->pfnReallocation,
            _s47->pfnFree,
            _s47->pfnInternalAllocation,
            _s47->pfnInternalFree);
    }
    return skip;
}
bool ExplicitValidation::PreCallValidateCreatePipelineCache(VkDevice device,
                const VkPipelineCacheCreateInfo* pCreateInfo,
                const VkAllocationCallbacks* pAllocator,
                VkPipelineCache* pPipelineCache) const {
    bool skip = false;
    if (pCreateInfo != nullptr) {
        const auto _s48 = pCreateInfo;
        skip |= ValidatePipelineCacheCreateInfo({},
            _s48->sType,
            _s48->pNext,
            _s48->flags,
            _s48->initialDataSize,
            _s48->pInitialData);
    }
    if (pAllocator != nullptr) {
        const auto _s49 = pAllocator;
        skip |= ValidateAllocationCallbacks({},
            _s49->pUserData,
            _s49->pfnAllocation,
            _s49->pfnReallocation,
            _s49->pfnFree,
            _s49->pfnInternalAllocation,
            _s49->pfnInternalFree);
    }
    return skip;
}
bool ExplicitValidation::PreCallValidateDestroyPipelineCache(VkDevice device,
                VkPipelineCache pipelineCache,
                const VkAllocationCallbacks* pAllocator) const {
    bool skip = false;
    if (pAllocator != nullptr) {
        const auto _s50 = pAllocator;
        skip |= ValidateAllocationCallbacks({},
            _s50->pUserData,
            _s50->pfnAllocation,
            _s50->pfnReallocation,
            _s50->pfnFree,
            _s50->pfnInternalAllocation,
            _s50->pfnInternalFree);
    }
    return skip;
}
bool ExplicitValidation::PreCallValidateGetPipelineCacheData(VkDevice device,
                VkPipelineCache pipelineCache,
                size_t* pDataSize,
                void* pData) const {
    bool skip = false;
    return skip;
}
bool ExplicitValidation::PreCallValidateMergePipelineCaches(VkDevice device,
                VkPipelineCache dstCache,
                uint32_t srcCacheCount,
                const VkPipelineCache* pSrcCaches) const {
    bool skip = false;
    return skip;
}
bool ExplicitValidation::PreCallValidateCreateGraphicsPipelines(VkDevice device,
                VkPipelineCache pipelineCache,
                uint32_t createInfoCount,
                const VkGraphicsPipelineCreateInfo* pCreateInfos,
                const VkAllocationCallbacks* pAllocator,
                VkPipeline* pPipelines,
                void *validation_state) const {
    bool skip = false;
    if (pCreateInfos != nullptr) {
        for (uint32_t _i51 = 0;_i51 < createInfoCount; ++_i51) {
            const auto _s52 = &pCreateInfos[_i51];
            skip |= ValidateGraphicsPipelineCreateInfo({},
                _s52->sType,
                _s52->pNext,
                _s52->flags,
                _s52->stageCount,
                _s52->pStages,
                _s52->pVertexInputState,
                _s52->pInputAssemblyState,
                _s52->pTessellationState,
                _s52->pViewportState,
                _s52->pRasterizationState,
                _s52->pMultisampleState,
                _s52->pDepthStencilState,
                _s52->pColorBlendState,
                _s52->pDynamicState,
                _s52->layout,
                _s52->renderPass,
                _s52->subpass,
                _s52->basePipelineHandle,
                _s52->basePipelineIndex);
        }
    }
    if (pAllocator != nullptr) {
        const auto _s53 = pAllocator;
        skip |= ValidateAllocationCallbacks({},
            _s53->pUserData,
            _s53->pfnAllocation,
            _s53->pfnReallocation,
            _s53->pfnFree,
            _s53->pfnInternalAllocation,
            _s53->pfnInternalFree);
    }
    return skip;
}
bool ExplicitValidation::PreCallValidateCreateComputePipelines(VkDevice device,
                VkPipelineCache pipelineCache,
                uint32_t createInfoCount,
                const VkComputePipelineCreateInfo* pCreateInfos,
                const VkAllocationCallbacks* pAllocator,
                VkPipeline* pPipelines,
                void *validation_state) const {
    bool skip = false;
    if (pCreateInfos != nullptr) {
        for (uint32_t _i54 = 0;_i54 < createInfoCount; ++_i54) {
            const auto _s55 = &pCreateInfos[_i54];
            skip |= ValidateComputePipelineCreateInfo({},
                _s55->sType,
                _s55->pNext,
                _s55->flags,
                _s55->stage,
                _s55->layout,
                _s55->basePipelineHandle,
                _s55->basePipelineIndex);
        }
    }
    if (pAllocator != nullptr) {
        const auto _s56 = pAllocator;
        skip |= ValidateAllocationCallbacks({},
            _s56->pUserData,
            _s56->pfnAllocation,
            _s56->pfnReallocation,
            _s56->pfnFree,
            _s56->pfnInternalAllocation,
            _s56->pfnInternalFree);
    }
    return skip;
}
bool ExplicitValidation::PreCallValidateDestroyPipeline(VkDevice device,
                VkPipeline pipeline,
                const VkAllocationCallbacks* pAllocator) const {
    bool skip = false;
    if (pAllocator != nullptr) {
        const auto _s57 = pAllocator;
        skip |= ValidateAllocationCallbacks({},
            _s57->pUserData,
            _s57->pfnAllocation,
            _s57->pfnReallocation,
            _s57->pfnFree,
            _s57->pfnInternalAllocation,
            _s57->pfnInternalFree);
    }
    return skip;
}
bool ExplicitValidation::PreCallValidateCreatePipelineLayout(VkDevice device,
                const VkPipelineLayoutCreateInfo* pCreateInfo,
                const VkAllocationCallbacks* pAllocator,
                VkPipelineLayout* pPipelineLayout) const {
    bool skip = false;
    if (pCreateInfo != nullptr) {
        const auto _s58 = pCreateInfo;
        skip |= ValidatePipelineLayoutCreateInfo({},
            _s58->sType,
            _s58->pNext,
            _s58->flags,
            _s58->setLayoutCount,
            _s58->pSetLayouts,
            _s58->pushConstantRangeCount,
            _s58->pPushConstantRanges);
    }
    if (pAllocator != nullptr) {
        const auto _s59 = pAllocator;
        skip |= ValidateAllocationCallbacks({},
            _s59->pUserData,
            _s59->pfnAllocation,
            _s59->pfnReallocation,
            _s59->pfnFree,
            _s59->pfnInternalAllocation,
            _s59->pfnInternalFree);
    }
    return skip;
}
bool ExplicitValidation::PreCallValidateDestroyPipelineLayout(VkDevice device,
                VkPipelineLayout pipelineLayout,
                const VkAllocationCallbacks* pAllocator) const {
    bool skip = false;
    if (pAllocator != nullptr) {
        const auto _s60 = pAllocator;
        skip |= ValidateAllocationCallbacks({},
            _s60->pUserData,
            _s60->pfnAllocation,
            _s60->pfnReallocation,
            _s60->pfnFree,
            _s60->pfnInternalAllocation,
            _s60->pfnInternalFree);
    }
    return skip;
}
bool ExplicitValidation::PreCallValidateCreateSampler(VkDevice device,
                const VkSamplerCreateInfo* pCreateInfo,
                const VkAllocationCallbacks* pAllocator,
                VkSampler* pSampler) const {
    bool skip = false;
    if (pCreateInfo != nullptr) {
        const auto _s61 = pCreateInfo;
        skip |= ValidateSamplerCreateInfo({},
            _s61->sType,
            _s61->pNext,
            _s61->flags,
            _s61->magFilter,
            _s61->minFilter,
            _s61->mipmapMode,
            _s61->addressModeU,
            _s61->addressModeV,
            _s61->addressModeW,
            _s61->mipLodBias,
            _s61->anisotropyEnable,
            _s61->maxAnisotropy,
            _s61->compareEnable,
            _s61->compareOp,
            _s61->minLod,
            _s61->maxLod,
            _s61->borderColor,
            _s61->unnormalizedCoordinates);
    }
    if (pAllocator != nullptr) {
        const auto _s62 = pAllocator;
        skip |= ValidateAllocationCallbacks({},
            _s62->pUserData,
            _s62->pfnAllocation,
            _s62->pfnReallocation,
            _s62->pfnFree,
            _s62->pfnInternalAllocation,
            _s62->pfnInternalFree);
    }
    return skip;
}
bool ExplicitValidation::PreCallValidateDestroySampler(VkDevice device,
                VkSampler sampler,
                const VkAllocationCallbacks* pAllocator) const {
    bool skip = false;
    if (pAllocator != nullptr) {
        const auto _s63 = pAllocator;
        skip |= ValidateAllocationCallbacks({},
            _s63->pUserData,
            _s63->pfnAllocation,
            _s63->pfnReallocation,
            _s63->pfnFree,
            _s63->pfnInternalAllocation,
            _s63->pfnInternalFree);
    }
    return skip;
}
bool ExplicitValidation::PreCallValidateCreateDescriptorSetLayout(VkDevice device,
                const VkDescriptorSetLayoutCreateInfo* pCreateInfo,
                const VkAllocationCallbacks* pAllocator,
                VkDescriptorSetLayout* pSetLayout) const {
    bool skip = false;
    if (pCreateInfo != nullptr) {
        const auto _s64 = pCreateInfo;
        skip |= ValidateDescriptorSetLayoutCreateInfo({},
            _s64->sType,
            _s64->pNext,
            _s64->flags,
            _s64->bindingCount,
            _s64->pBindings);
    }
    if (pAllocator != nullptr) {
        const auto _s65 = pAllocator;
        skip |= ValidateAllocationCallbacks({},
            _s65->pUserData,
            _s65->pfnAllocation,
            _s65->pfnReallocation,
            _s65->pfnFree,
            _s65->pfnInternalAllocation,
            _s65->pfnInternalFree);
    }
    return skip;
}
bool ExplicitValidation::PreCallValidateDestroyDescriptorSetLayout(VkDevice device,
                VkDescriptorSetLayout descriptorSetLayout,
                const VkAllocationCallbacks* pAllocator) const {
    bool skip = false;
    if (pAllocator != nullptr) {
        const auto _s66 = pAllocator;
        skip |= ValidateAllocationCallbacks({},
            _s66->pUserData,
            _s66->pfnAllocation,
            _s66->pfnReallocation,
            _s66->pfnFree,
            _s66->pfnInternalAllocation,
            _s66->pfnInternalFree);
    }
    return skip;
}
bool ExplicitValidation::PreCallValidateCreateDescriptorPool(VkDevice device,
                const VkDescriptorPoolCreateInfo* pCreateInfo,
                const VkAllocationCallbacks* pAllocator,
                VkDescriptorPool* pDescriptorPool) const {
    bool skip = false;
    if (pCreateInfo != nullptr) {
        const auto _s67 = pCreateInfo;
        skip |= ValidateDescriptorPoolCreateInfo({},
            _s67->sType,
            _s67->pNext,
            _s67->flags,
            _s67->maxSets,
            _s67->poolSizeCount,
            _s67->pPoolSizes);
    }
    if (pAllocator != nullptr) {
        const auto _s68 = pAllocator;
        skip |= ValidateAllocationCallbacks({},
            _s68->pUserData,
            _s68->pfnAllocation,
            _s68->pfnReallocation,
            _s68->pfnFree,
            _s68->pfnInternalAllocation,
            _s68->pfnInternalFree);
    }
    return skip;
}
bool ExplicitValidation::PreCallValidateDestroyDescriptorPool(VkDevice device,
                VkDescriptorPool descriptorPool,
                const VkAllocationCallbacks* pAllocator) const {
    bool skip = false;
    if (pAllocator != nullptr) {
        const auto _s69 = pAllocator;
        skip |= ValidateAllocationCallbacks({},
            _s69->pUserData,
            _s69->pfnAllocation,
            _s69->pfnReallocation,
            _s69->pfnFree,
            _s69->pfnInternalAllocation,
            _s69->pfnInternalFree);
    }
    return skip;
}
bool ExplicitValidation::PreCallValidateResetDescriptorPool(VkDevice device,
                VkDescriptorPool descriptorPool,
                VkDescriptorPoolResetFlags flags) const {
    bool skip = false;
    return skip;
}
bool ExplicitValidation::PreCallValidateAllocateDescriptorSets(VkDevice device,
                const VkDescriptorSetAllocateInfo* pAllocateInfo,
                VkDescriptorSet* pDescriptorSets,
                void *validation_state) const {
    bool skip = false;
    if (pAllocateInfo != nullptr) {
        const auto _s70 = pAllocateInfo;
        skip |= ValidateDescriptorSetAllocateInfo({},
            _s70->sType,
            _s70->pNext,
            _s70->descriptorPool,
            _s70->descriptorSetCount,
            _s70->pSetLayouts);
    }
    return skip;
}
bool ExplicitValidation::PreCallValidateFreeDescriptorSets(VkDevice device,
                VkDescriptorPool descriptorPool,
                uint32_t descriptorSetCount,
                const VkDescriptorSet* pDescriptorSets) const {
    bool skip = false;
    return skip;
}
bool ExplicitValidation::PreCallValidateUpdateDescriptorSets(VkDevice device,
                uint32_t descriptorWriteCount,
                const VkWriteDescriptorSet* pDescriptorWrites,
                uint32_t descriptorCopyCount,
                const VkCopyDescriptorSet* pDescriptorCopies) const {
    bool skip = false;
    if (pDescriptorWrites != nullptr) {
        for (uint32_t _i71 = 0;_i71 < descriptorWriteCount; ++_i71) {
            const auto _s72 = &pDescriptorWrites[_i71];
            skip |= ValidateWriteDescriptorSet({},
                _s72->sType,
                _s72->pNext,
                _s72->dstSet,
                _s72->dstBinding,
                _s72->dstArrayElement,
                _s72->descriptorCount,
                _s72->descriptorType,
                _s72->pImageInfo,
                _s72->pBufferInfo,
                _s72->pTexelBufferView);
        }
    }
    if (pDescriptorCopies != nullptr) {
        for (uint32_t _i73 = 0;_i73 < descriptorCopyCount; ++_i73) {
            const auto _s74 = &pDescriptorCopies[_i73];
            skip |= ValidateCopyDescriptorSet({},
                _s74->sType,
                _s74->pNext,
                _s74->srcSet,
                _s74->srcBinding,
                _s74->srcArrayElement,
                _s74->dstSet,
                _s74->dstBinding,
                _s74->dstArrayElement,
                _s74->descriptorCount);
        }
    }
    return skip;
}
bool ExplicitValidation::PreCallValidateCreateFramebuffer(VkDevice device,
                const VkFramebufferCreateInfo* pCreateInfo,
                const VkAllocationCallbacks* pAllocator,
                VkFramebuffer* pFramebuffer) const {
    bool skip = false;
    if (pCreateInfo != nullptr) {
        const auto _s75 = pCreateInfo;
        skip |= ValidateFramebufferCreateInfo({},
            _s75->sType,
            _s75->pNext,
            _s75->flags,
            _s75->renderPass,
            _s75->attachmentCount,
            _s75->pAttachments,
            _s75->width,
            _s75->height,
            _s75->layers);
    }
    if (pAllocator != nullptr) {
        const auto _s76 = pAllocator;
        skip |= ValidateAllocationCallbacks({},
            _s76->pUserData,
            _s76->pfnAllocation,
            _s76->pfnReallocation,
            _s76->pfnFree,
            _s76->pfnInternalAllocation,
            _s76->pfnInternalFree);
    }
    return skip;
}
bool ExplicitValidation::PreCallValidateDestroyFramebuffer(VkDevice device,
                VkFramebuffer framebuffer,
                const VkAllocationCallbacks* pAllocator) const {
    bool skip = false;
    if (pAllocator != nullptr) {
        const auto _s77 = pAllocator;
        skip |= ValidateAllocationCallbacks({},
            _s77->pUserData,
            _s77->pfnAllocation,
            _s77->pfnReallocation,
            _s77->pfnFree,
            _s77->pfnInternalAllocation,
            _s77->pfnInternalFree);
    }
    return skip;
}
bool ExplicitValidation::PreCallValidateCreateRenderPass(VkDevice device,
                const VkRenderPassCreateInfo* pCreateInfo,
                const VkAllocationCallbacks* pAllocator,
                VkRenderPass* pRenderPass) const {
    bool skip = false;
    if (pCreateInfo != nullptr) {
        const auto _s78 = pCreateInfo;
        skip |= ValidateRenderPassCreateInfo({},
            _s78->sType,
            _s78->pNext,
            _s78->flags,
            _s78->attachmentCount,
            _s78->pAttachments,
            _s78->subpassCount,
            _s78->pSubpasses,
            _s78->dependencyCount,
            _s78->pDependencies);
    }
    if (pAllocator != nullptr) {
        const auto _s79 = pAllocator;
        skip |= ValidateAllocationCallbacks({},
            _s79->pUserData,
            _s79->pfnAllocation,
            _s79->pfnReallocation,
            _s79->pfnFree,
            _s79->pfnInternalAllocation,
            _s79->pfnInternalFree);
    }
    return skip;
}
bool ExplicitValidation::PreCallValidateDestroyRenderPass(VkDevice device,
                VkRenderPass renderPass,
                const VkAllocationCallbacks* pAllocator) const {
    bool skip = false;
    if (pAllocator != nullptr) {
        const auto _s80 = pAllocator;
        skip |= ValidateAllocationCallbacks({},
            _s80->pUserData,
            _s80->pfnAllocation,
            _s80->pfnReallocation,
            _s80->pfnFree,
            _s80->pfnInternalAllocation,
            _s80->pfnInternalFree);
    }
    return skip;
}
bool ExplicitValidation::PreCallValidateGetRenderAreaGranularity(VkDevice device,
                VkRenderPass renderPass,
                VkExtent2D* pGranularity) const {
    bool skip = false;
    if (pGranularity != nullptr) {
        const auto _s81 = pGranularity;
        skip |= ValidateExtent2D({},
            _s81->width,
            _s81->height);
    }
    return skip;
}
bool ExplicitValidation::PreCallValidateCreateCommandPool(VkDevice device,
                const VkCommandPoolCreateInfo* pCreateInfo,
                const VkAllocationCallbacks* pAllocator,
                VkCommandPool* pCommandPool) const {
    bool skip = false;
    if (pCreateInfo != nullptr) {
        const auto _s82 = pCreateInfo;
        skip |= ValidateCommandPoolCreateInfo({},
            _s82->sType,
            _s82->pNext,
            _s82->flags,
            _s82->queueFamilyIndex);
    }
    if (pAllocator != nullptr) {
        const auto _s83 = pAllocator;
        skip |= ValidateAllocationCallbacks({},
            _s83->pUserData,
            _s83->pfnAllocation,
            _s83->pfnReallocation,
            _s83->pfnFree,
            _s83->pfnInternalAllocation,
            _s83->pfnInternalFree);
    }
    return skip;
}
bool ExplicitValidation::PreCallValidateDestroyCommandPool(VkDevice device,
                VkCommandPool commandPool,
                const VkAllocationCallbacks* pAllocator) const {
    bool skip = false;
    if (pAllocator != nullptr) {
        const auto _s84 = pAllocator;
        skip |= ValidateAllocationCallbacks({},
            _s84->pUserData,
            _s84->pfnAllocation,
            _s84->pfnReallocation,
            _s84->pfnFree,
            _s84->pfnInternalAllocation,
            _s84->pfnInternalFree);
    }
    return skip;
}
bool ExplicitValidation::PreCallValidateResetCommandPool(VkDevice device,
                VkCommandPool commandPool,
                VkCommandPoolResetFlags flags) const {
    bool skip = false;
    return skip;
}
bool ExplicitValidation::PreCallValidateAllocateCommandBuffers(VkDevice device,
                const VkCommandBufferAllocateInfo* pAllocateInfo,
                VkCommandBuffer* pCommandBuffers) const {
    bool skip = false;
    if (pAllocateInfo != nullptr) {
        const auto _s85 = pAllocateInfo;
        skip |= ValidateCommandBufferAllocateInfo({},
            _s85->sType,
            _s85->pNext,
            _s85->commandPool,
            _s85->level,
            _s85->commandBufferCount);
    }
    return skip;
}
bool ExplicitValidation::PreCallValidateFreeCommandBuffers(VkDevice device,
                VkCommandPool commandPool,
                uint32_t commandBufferCount,
                const VkCommandBuffer* pCommandBuffers) const {
    bool skip = false;
    return skip;
}
bool ExplicitValidation::PreCallValidateBeginCommandBuffer(VkCommandBuffer commandBuffer,
                const VkCommandBufferBeginInfo* pBeginInfo) const {
    bool skip = false;
    if (pBeginInfo != nullptr) {
        const auto _s86 = pBeginInfo;
        skip |= ValidateCommandBufferBeginInfo({},
            _s86->sType,
            _s86->pNext,
            _s86->flags,
            _s86->pInheritanceInfo);
    }
    return skip;
}
bool ExplicitValidation::PreCallValidateEndCommandBuffer(VkCommandBuffer commandBuffer) const {
    bool skip = false;
    return skip;
}
bool ExplicitValidation::PreCallValidateResetCommandBuffer(VkCommandBuffer commandBuffer,
                VkCommandBufferResetFlags flags) const {
    bool skip = false;
    return skip;
}
bool ExplicitValidation::PreCallValidateCmdBindPipeline(VkCommandBuffer commandBuffer,
                VkPipelineBindPoint pipelineBindPoint,
                VkPipeline pipeline) const {
    bool skip = false;
    LogObjectList _carryOverObjects;
    _carryOverObjects.add(commandBuffer);
    return skip;
}
bool ExplicitValidation::PreCallValidateCmdSetViewport(VkCommandBuffer commandBuffer,
                uint32_t firstViewport,
                uint32_t viewportCount,
                const VkViewport* pViewports) const {
    bool skip = false;
    LogObjectList _carryOverObjects;
    _carryOverObjects.add(commandBuffer);
    if (pViewports != nullptr) {
        for (uint32_t _i87 = 0;_i87 < viewportCount; ++_i87) {
            const auto _s88 = &pViewports[_i87];
            skip |= ValidateViewport(_carryOverObjects,
                _s88->x,
                _s88->y,
                _s88->width,
                _s88->height,
                _s88->minDepth,
                _s88->maxDepth);
        }
    }
    return skip;
}
bool ExplicitValidation::PreCallValidateCmdSetScissor(VkCommandBuffer commandBuffer,
                uint32_t firstScissor,
                uint32_t scissorCount,
                const VkRect2D* pScissors) const {
    bool skip = false;
    LogObjectList _carryOverObjects;
    _carryOverObjects.add(commandBuffer);
    if (pScissors != nullptr) {
        for (uint32_t _i89 = 0;_i89 < scissorCount; ++_i89) {
            const auto _s90 = &pScissors[_i89];
            skip |= ValidateRect2D(_carryOverObjects,
                _s90->offset,
                _s90->extent);
        }
    }
    return skip;
}
bool ExplicitValidation::PreCallValidateCmdSetLineWidth(VkCommandBuffer commandBuffer,
                float lineWidth) const {
    bool skip = false;
    LogObjectList _carryOverObjects;
    _carryOverObjects.add(commandBuffer);
    return skip;
}
bool ExplicitValidation::PreCallValidateCmdSetDepthBias(VkCommandBuffer commandBuffer,
                float depthBiasConstantFactor,
                float depthBiasClamp,
                float depthBiasSlopeFactor) const {
    bool skip = false;
    LogObjectList _carryOverObjects;
    _carryOverObjects.add(commandBuffer);
    return skip;
}
bool ExplicitValidation::PreCallValidateCmdSetBlendConstants(VkCommandBuffer commandBuffer,
                const float blendConstants[4]) const {
    bool skip = false;
    LogObjectList _carryOverObjects;
    _carryOverObjects.add(commandBuffer);
    return skip;
}
bool ExplicitValidation::PreCallValidateCmdSetDepthBounds(VkCommandBuffer commandBuffer,
                float minDepthBounds,
                float maxDepthBounds) const {
    bool skip = false;
    LogObjectList _carryOverObjects;
    _carryOverObjects.add(commandBuffer);
    return skip;
}
bool ExplicitValidation::PreCallValidateCmdSetStencilCompareMask(VkCommandBuffer commandBuffer,
                VkStencilFaceFlags faceMask,
                uint32_t compareMask) const {
    bool skip = false;
    LogObjectList _carryOverObjects;
    _carryOverObjects.add(commandBuffer);
    return skip;
}
bool ExplicitValidation::PreCallValidateCmdSetStencilWriteMask(VkCommandBuffer commandBuffer,
                VkStencilFaceFlags faceMask,
                uint32_t writeMask) const {
    bool skip = false;
    LogObjectList _carryOverObjects;
    _carryOverObjects.add(commandBuffer);
    return skip;
}
bool ExplicitValidation::PreCallValidateCmdSetStencilReference(VkCommandBuffer commandBuffer,
                VkStencilFaceFlags faceMask,
                uint32_t reference) const {
    bool skip = false;
    LogObjectList _carryOverObjects;
    _carryOverObjects.add(commandBuffer);
    return skip;
}
bool ExplicitValidation::PreCallValidateCmdBindDescriptorSets(VkCommandBuffer commandBuffer,
                VkPipelineBindPoint pipelineBindPoint,
                VkPipelineLayout layout,
                uint32_t firstSet,
                uint32_t descriptorSetCount,
                const VkDescriptorSet* pDescriptorSets,
                uint32_t dynamicOffsetCount,
                const uint32_t* pDynamicOffsets) const {
    bool skip = false;
    LogObjectList _carryOverObjects;
    _carryOverObjects.add(commandBuffer);
    return skip;
}
bool ExplicitValidation::PreCallValidateCmdBindIndexBuffer(VkCommandBuffer commandBuffer,
                VkBuffer buffer,
                VkDeviceSize offset,
                VkIndexType indexType) const {
    bool skip = false;
    LogObjectList _carryOverObjects;
    _carryOverObjects.add(commandBuffer);
    return skip;
}
bool ExplicitValidation::PreCallValidateCmdBindVertexBuffers(VkCommandBuffer commandBuffer,
                uint32_t firstBinding,
                uint32_t bindingCount,
                const VkBuffer* pBuffers,
                const VkDeviceSize* pOffsets) const {
    bool skip = false;
    LogObjectList _carryOverObjects;
    _carryOverObjects.add(commandBuffer);
    return skip;
}
bool ExplicitValidation::PreCallValidateCmdDraw(VkCommandBuffer commandBuffer,
                uint32_t vertexCount,
                uint32_t instanceCount,
                uint32_t firstVertex,
                uint32_t firstInstance) const {
    bool skip = false;
    LogObjectList _carryOverObjects;
    _carryOverObjects.add(commandBuffer);
    return skip;
}
bool ExplicitValidation::PreCallValidateCmdDrawIndexed(VkCommandBuffer commandBuffer,
                uint32_t indexCount,
                uint32_t instanceCount,
                uint32_t firstIndex,
                int32_t vertexOffset,
                uint32_t firstInstance) const {
    bool skip = false;
    LogObjectList _carryOverObjects;
    _carryOverObjects.add(commandBuffer);
    return skip;
}
bool ExplicitValidation::PreCallValidateCmdDrawIndirect(VkCommandBuffer commandBuffer,
                VkBuffer buffer,
                VkDeviceSize offset,
                uint32_t drawCount,
                uint32_t stride) const {
    bool skip = false;
    LogObjectList _carryOverObjects;
    _carryOverObjects.add(commandBuffer);
    return skip;
}
bool ExplicitValidation::PreCallValidateCmdDrawIndexedIndirect(VkCommandBuffer commandBuffer,
                VkBuffer buffer,
                VkDeviceSize offset,
                uint32_t drawCount,
                uint32_t stride) const {
    bool skip = false;
    LogObjectList _carryOverObjects;
    _carryOverObjects.add(commandBuffer);
    return skip;
}
bool ExplicitValidation::PreCallValidateCmdDispatch(VkCommandBuffer commandBuffer,
                uint32_t groupCountX,
                uint32_t groupCountY,
                uint32_t groupCountZ) const {
    bool skip = false;
    LogObjectList _carryOverObjects;
    _carryOverObjects.add(commandBuffer);
    return skip;
}
bool ExplicitValidation::PreCallValidateCmdDispatchIndirect(VkCommandBuffer commandBuffer,
                VkBuffer buffer,
                VkDeviceSize offset) const {
    bool skip = false;
    LogObjectList _carryOverObjects;
    _carryOverObjects.add(commandBuffer);
    return skip;
}
bool ExplicitValidation::PreCallValidateCmdCopyBuffer(VkCommandBuffer commandBuffer,
                VkBuffer srcBuffer,
                VkBuffer dstBuffer,
                uint32_t regionCount,
                const VkBufferCopy* pRegions) const {
    bool skip = false;
    LogObjectList _carryOverObjects;
    _carryOverObjects.add(commandBuffer);
    if (pRegions != nullptr) {
        for (uint32_t _i91 = 0;_i91 < regionCount; ++_i91) {
            const auto _s92 = &pRegions[_i91];
            skip |= ValidateBufferCopy(_carryOverObjects,
                _s92->srcOffset,
                _s92->dstOffset,
                _s92->size);
        }
    }
    return skip;
}
bool ExplicitValidation::PreCallValidateCmdCopyImage(VkCommandBuffer commandBuffer,
                VkImage srcImage,
                VkImageLayout srcImageLayout,
                VkImage dstImage,
                VkImageLayout dstImageLayout,
                uint32_t regionCount,
                const VkImageCopy* pRegions) const {
    bool skip = false;
    LogObjectList _carryOverObjects;
    _carryOverObjects.add(commandBuffer);
    if (pRegions != nullptr) {
        for (uint32_t _i93 = 0;_i93 < regionCount; ++_i93) {
            const auto _s94 = &pRegions[_i93];
            skip |= ValidateImageCopy(_carryOverObjects,
                _s94->srcSubresource,
                _s94->srcOffset,
                _s94->dstSubresource,
                _s94->dstOffset,
                _s94->extent);
        }
    }
    return skip;
}
bool ExplicitValidation::PreCallValidateCmdBlitImage(VkCommandBuffer commandBuffer,
                VkImage srcImage,
                VkImageLayout srcImageLayout,
                VkImage dstImage,
                VkImageLayout dstImageLayout,
                uint32_t regionCount,
                const VkImageBlit* pRegions,
                VkFilter filter) const {
    bool skip = false;
    LogObjectList _carryOverObjects;
    _carryOverObjects.add(commandBuffer);
    if (pRegions != nullptr) {
        for (uint32_t _i95 = 0;_i95 < regionCount; ++_i95) {
            const auto _s96 = &pRegions[_i95];
            skip |= ValidateImageBlit(_carryOverObjects,
                _s96->srcSubresource,
                _s96->srcOffsets,
                _s96->dstSubresource,
                _s96->dstOffsets);
        }
    }
    return skip;
}
bool ExplicitValidation::PreCallValidateCmdCopyBufferToImage(VkCommandBuffer commandBuffer,
                VkBuffer srcBuffer,
                VkImage dstImage,
                VkImageLayout dstImageLayout,
                uint32_t regionCount,
                const VkBufferImageCopy* pRegions) const {
    bool skip = false;
    auto srcBuffer_ = Get<BUFFER_STATE>(srcBuffer);
    auto dstImage_ = Get<IMAGE_STATE>(dstImage);

    {
        if ((Builtin_create_info(dstImage_).imageType == VK_IMAGE_TYPE_1D))
        {
            for (uint32_t i3 = 0; i3 < regionCount; ++i3)
            {
                const VkBufferImageCopy &region_0 = pRegions[i3];
                {
                    if (!((region_0.imageOffset.y == 0)))
                    {
                        const LogObjectList objlist{dstImage};
                        skip |= LogFail(objlist, "VUID-vkCmdCopyBufferToImage-srcImage-00199",
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
                        skip |= LogFail(objlist, "VUID-vkCmdCopyBufferToImage-srcImage-00199",
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
    LogObjectList _carryOverObjects;
    _carryOverObjects.add(commandBuffer);
    if (pRegions != nullptr) {
        for (uint32_t _i97 = 0;_i97 < regionCount; ++_i97) {
            const auto _s98 = &pRegions[_i97];
            skip |= ValidateBufferImageCopy(_carryOverObjects,
                _s98->bufferOffset,
                _s98->bufferRowLength,
                _s98->bufferImageHeight,
                _s98->imageSubresource,
                _s98->imageOffset,
                _s98->imageExtent);
        }
    }
    return skip;
}
bool ExplicitValidation::PreCallValidateCmdCopyImageToBuffer(VkCommandBuffer commandBuffer,
                VkImage srcImage,
                VkImageLayout srcImageLayout,
                VkBuffer dstBuffer,
                uint32_t regionCount,
                const VkBufferImageCopy* pRegions) const {
    bool skip = false;
    auto srcImage_ = Get<IMAGE_STATE>(srcImage);
    auto dstBuffer_ = Get<BUFFER_STATE>(dstBuffer);

    {
        if ((Builtin_create_info(srcImage_).imageType == VK_IMAGE_TYPE_1D))
        {
            for (uint32_t i3 = 0; i3 < regionCount; ++i3)
            {
                const VkBufferImageCopy &region_0 = pRegions[i3];
                {
                    if (!((region_0.imageOffset.y == 0)))
                    {
                        const LogObjectList objlist{srcImage};
                        skip |= LogFail(objlist, "VUID-vkCmdCopyImageToBuffer-srcImage-00199",
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
                        skip |= LogFail(objlist, "VUID-vkCmdCopyImageToBuffer-srcImage-00199",
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
    LogObjectList _carryOverObjects;
    _carryOverObjects.add(commandBuffer);
    if (pRegions != nullptr) {
        for (uint32_t _i99 = 0;_i99 < regionCount; ++_i99) {
            const auto _s100 = &pRegions[_i99];
            skip |= ValidateBufferImageCopy(_carryOverObjects,
                _s100->bufferOffset,
                _s100->bufferRowLength,
                _s100->bufferImageHeight,
                _s100->imageSubresource,
                _s100->imageOffset,
                _s100->imageExtent);
        }
    }
    return skip;
}
bool ExplicitValidation::PreCallValidateCmdUpdateBuffer(VkCommandBuffer commandBuffer,
                VkBuffer dstBuffer,
                VkDeviceSize dstOffset,
                VkDeviceSize dataSize,
                const void* pData) const {
    bool skip = false;
    LogObjectList _carryOverObjects;
    _carryOverObjects.add(commandBuffer);
    return skip;
}
bool ExplicitValidation::PreCallValidateCmdFillBuffer(VkCommandBuffer commandBuffer,
                VkBuffer dstBuffer,
                VkDeviceSize dstOffset,
                VkDeviceSize size,
                uint32_t data) const {
    bool skip = false;
    LogObjectList _carryOverObjects;
    _carryOverObjects.add(commandBuffer);
    return skip;
}
bool ExplicitValidation::PreCallValidateCmdClearColorImage(VkCommandBuffer commandBuffer,
                VkImage image,
                VkImageLayout imageLayout,
                const VkClearColorValue* pColor,
                uint32_t rangeCount,
                const VkImageSubresourceRange* pRanges) const {
    bool skip = false;
    auto image_ = Get<IMAGE_STATE>(image);

    {
        if (!(Builtin_has_bit(Builtin_create_info(image_).usage, VK_IMAGE_USAGE_TRANSFER_DST_BIT)))
        {
            const LogObjectList objlist{image};
            skip |= LogFail(objlist, "VUID-vkCmdClearColorImage-image-00002",
                R"(    ${vu-builtin}require${}(image${vu-value}{{%s}}${}.${vu-builtin}create_info${}().usage${vu-value}{{%#)" PRIx32 R"(}}${}.${vu-builtin}has_bit${}(VK_IMAGE_USAGE_TRANSFER_DST_BIT)${vu-value}{{%s}}${})${vu-fail} <-- Failing condition (Values captured at this instant)${})",
                report_data->FormatHandle(image).c_str(),
                Builtin_create_info(image_).usage,
                (Builtin_has_bit(Builtin_create_info(image_).usage, VK_IMAGE_USAGE_TRANSFER_DST_BIT)) ? "true" : "false");
        }

        uint32_t mipLevels_1=Builtin_create_info(image_).mipLevels;
        uint32_t arrayLayers_5=Builtin_create_info(image_).arrayLayers;
        if (!(((imageLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL)||(imageLayout == VK_IMAGE_LAYOUT_GENERAL))))
        {
            const LogObjectList objlist{};
            skip |= LogFail(objlist, "VUID-vkCmdClearColorImage-imageLayout-00005",
                R"(    ${vu-builtin}require${}(imageLayout ${vu-operator}==${} VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL ${vu-operator}or${}
            imageLayout ${vu-operator}==${} VK_IMAGE_LAYOUT_GENERAL)${vu-fail} <-- Failing condition (Values captured at this instant)${})");
        }

        if (!(((imageLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL)||(imageLayout == VK_IMAGE_LAYOUT_GENERAL)||(imageLayout == VK_IMAGE_LAYOUT_SHARED_PRESENT_KHR))))
        {
            const LogObjectList objlist{};
            skip |= LogFail(objlist, "VUID-vkCmdClearColorImage-imageLayout-01394",
                R"(    ${vu-builtin}require${}(imageLayout ${vu-operator}==${} VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL ${vu-operator}or${}
            imageLayout ${vu-operator}==${} VK_IMAGE_LAYOUT_GENERAL ${vu-operator}or${}
            imageLayout ${vu-operator}==${} VK_IMAGE_LAYOUT_SHARED_PRESENT_KHR)${vu-fail} <-- Failing condition (Values captured at this instant)${})");
        }

        for (uint32_t i2 = 0; i2 < rangeCount; ++i2)
        {
            const VkImageSubresourceRange &range_0 = pRanges[i2];
            {
                if (!((range_0.aspectMask == VK_IMAGE_ASPECT_COLOR_BIT)))
                {
                    const LogObjectList objlist{};
                    skip |= LogFail(objlist, "VUID-vkCmdClearColorImage-aspectMask-02498",
                        R"(    ${vu-keyword}for${} range${vu-value}{{@%)" PRIu32 R"(}}${} ${vu-operator}in${} pRanges:
      ${vu-builtin}require${}(range${vu-value}{{@%)" PRIu32 R"(}}${}.aspectMask${vu-value}{{%#)" PRIx32 R"(}}${} ${vu-operator}==${} VK_IMAGE_ASPECT_COLOR_BIT)${vu-fail} <-- Failing condition (Values captured at this instant)${})",
                        i2,
                        i2,
                        range_0.aspectMask);
                }

                if (!((range_0.baseMipLevel < mipLevels_1)))
                {
                    const LogObjectList objlist{image};
                    skip |= LogFail(objlist, "VUID-vkCmdClearColorImage-baseMipLevel-01470",
                        R"(    mipLevels${vu-value}{{%)" PRIu32 R"(}}${} ${vu-operator}=${} image${vu-value}{{%s}}${}.${vu-builtin}create_info${}().mipLevels${vu-value}{{%)" PRIu32 R"(}}${}
    ${vu-keyword}for${} range${vu-value}{{@%)" PRIu32 R"(}}${} ${vu-operator}in${} pRanges:
      ${vu-builtin}require${}(range${vu-value}{{@%)" PRIu32 R"(}}${}.baseMipLevel${vu-value}{{%)" PRIu32 R"(}}${} ${vu-operator}<${} mipLevels${vu-value}{{%)" PRIu32 R"(}}${})${vu-fail} <-- Failing condition (Values captured at this instant)${})",
                        mipLevels_1,
                        report_data->FormatHandle(image).c_str(),
                        Builtin_create_info(image_).mipLevels,
                        i2,
                        i2,
                        range_0.baseMipLevel,
                        mipLevels_1);
                }

                if (!((range_0.baseArrayLayer < arrayLayers_5)))
                {
                    const LogObjectList objlist{image};
                    skip |= LogFail(objlist, "VUID-vkCmdClearColorImage-baseArrayLayer-01472",
                        R"(    arrayLayers${vu-value}{{%)" PRIu32 R"(}}${} ${vu-operator}=${} image${vu-value}{{%s}}${}.${vu-builtin}create_info${}().arrayLayers${vu-value}{{%)" PRIu32 R"(}}${}
    ${vu-keyword}for${} range${vu-value}{{@%)" PRIu32 R"(}}${} ${vu-operator}in${} pRanges:
      ${vu-builtin}require${}(range${vu-value}{{@%)" PRIu32 R"(}}${}.baseArrayLayer${vu-value}{{%)" PRIu32 R"(}}${} ${vu-operator}<${} arrayLayers${vu-value}{{%)" PRIu32 R"(}}${})${vu-fail} <-- Failing condition (Values captured at this instant)${})",
                        arrayLayers_5,
                        report_data->FormatHandle(image).c_str(),
                        Builtin_create_info(image_).arrayLayers,
                        i2,
                        i2,
                        range_0.baseArrayLayer,
                        arrayLayers_5);
                }

                if ((range_0.levelCount != VK_REMAINING_MIP_LEVELS))
                {
                    if (!(((range_0.baseMipLevel + range_0.levelCount) <= mipLevels_1)))
                    {
                        const LogObjectList objlist{image};
                        skip |= LogFail(objlist, "VUID-vkCmdClearColorImage-pRanges-01692",
                            R"(    mipLevels${vu-value}{{%)" PRIu32 R"(}}${} ${vu-operator}=${} image${vu-value}{{%s}}${}.${vu-builtin}create_info${}().mipLevels${vu-value}{{%)" PRIu32 R"(}}${}
    ${vu-keyword}for${} range${vu-value}{{@%)" PRIu32 R"(}}${} ${vu-operator}in${} pRanges:
      ${vu-keyword}if${} range${vu-value}{{@%)" PRIu32 R"(}}${}.levelCount${vu-value}{{%)" PRIu32 R"(}}${} ${vu-operator}!=${} VK_REMAINING_MIP_LEVELS:
        ${vu-builtin}require${}(range${vu-value}{{@%)" PRIu32 R"(}}${}.baseMipLevel${vu-value}{{%)" PRIu32 R"(}}${} ${vu-operator}+${} range${vu-value}{{@%)" PRIu32 R"(}}${}.levelCount${vu-value}{{%)" PRIu32 R"(}}${} ${vu-operator}<=${} mipLevels${vu-value}{{%)" PRIu32 R"(}}${})${vu-fail} <-- Failing condition (Values captured at this instant)${})",
                            mipLevels_1,
                            report_data->FormatHandle(image).c_str(),
                            Builtin_create_info(image_).mipLevels,
                            i2,
                            i2,
                            range_0.levelCount,
                            i2,
                            range_0.baseMipLevel,
                            i2,
                            range_0.levelCount,
                            mipLevels_1);
                    }

                }

                if ((range_0.layerCount != VK_REMAINING_ARRAY_LAYERS))
                {
                    if (!(((range_0.baseArrayLayer + range_0.layerCount) <= arrayLayers_5)))
                    {
                        const LogObjectList objlist{image};
                        skip |= LogFail(objlist, "VUID-vkCmdClearColorImage-pRanges-01693",
                            R"(    arrayLayers${vu-value}{{%)" PRIu32 R"(}}${} ${vu-operator}=${} image${vu-value}{{%s}}${}.${vu-builtin}create_info${}().arrayLayers${vu-value}{{%)" PRIu32 R"(}}${}
    ${vu-keyword}for${} range${vu-value}{{@%)" PRIu32 R"(}}${} ${vu-operator}in${} pRanges:
      ${vu-keyword}if${} range${vu-value}{{@%)" PRIu32 R"(}}${}.layerCount${vu-value}{{%)" PRIu32 R"(}}${} ${vu-operator}!=${} VK_REMAINING_ARRAY_LAYERS:
        ${vu-builtin}require${}(range${vu-value}{{@%)" PRIu32 R"(}}${}.baseArrayLayer${vu-value}{{%)" PRIu32 R"(}}${} ${vu-operator}+${} range${vu-value}{{@%)" PRIu32 R"(}}${}.layerCount${vu-value}{{%)" PRIu32 R"(}}${} ${vu-operator}<=${} arrayLayers${vu-value}{{%)" PRIu32 R"(}}${})${vu-fail} <-- Failing condition (Values captured at this instant)${})",
                            arrayLayers_5,
                            report_data->FormatHandle(image).c_str(),
                            Builtin_create_info(image_).arrayLayers,
                            i2,
                            i2,
                            range_0.layerCount,
                            i2,
                            range_0.baseArrayLayer,
                            i2,
                            range_0.layerCount,
                            arrayLayers_5);
                    }

                }

            }
        }

    }
    LogObjectList _carryOverObjects;
    _carryOverObjects.add(commandBuffer);
    if (pColor != nullptr) {
        const auto _s101 = pColor;
        skip |= ValidateClearColorValue(_carryOverObjects,
            _s101->float32,
            _s101->int32,
            _s101->uint32);
    }
    if (pRanges != nullptr) {
        for (uint32_t _i102 = 0;_i102 < rangeCount; ++_i102) {
            const auto _s103 = &pRanges[_i102];
            skip |= ValidateImageSubresourceRange(_carryOverObjects,
                _s103->aspectMask,
                _s103->baseMipLevel,
                _s103->levelCount,
                _s103->baseArrayLayer,
                _s103->layerCount);
        }
    }
    return skip;
}
bool ExplicitValidation::PreCallValidateCmdClearDepthStencilImage(VkCommandBuffer commandBuffer,
                VkImage image,
                VkImageLayout imageLayout,
                const VkClearDepthStencilValue* pDepthStencil,
                uint32_t rangeCount,
                const VkImageSubresourceRange* pRanges) const {
    bool skip = false;
    LogObjectList _carryOverObjects;
    _carryOverObjects.add(commandBuffer);
    if (pDepthStencil != nullptr) {
        const auto _s104 = pDepthStencil;
        skip |= ValidateClearDepthStencilValue(_carryOverObjects,
            _s104->depth,
            _s104->stencil);
    }
    if (pRanges != nullptr) {
        for (uint32_t _i105 = 0;_i105 < rangeCount; ++_i105) {
            const auto _s106 = &pRanges[_i105];
            skip |= ValidateImageSubresourceRange(_carryOverObjects,
                _s106->aspectMask,
                _s106->baseMipLevel,
                _s106->levelCount,
                _s106->baseArrayLayer,
                _s106->layerCount);
        }
    }
    return skip;
}
bool ExplicitValidation::PreCallValidateCmdClearAttachments(VkCommandBuffer commandBuffer,
                uint32_t attachmentCount,
                const VkClearAttachment* pAttachments,
                uint32_t rectCount,
                const VkClearRect* pRects) const {
    bool skip = false;
    LogObjectList _carryOverObjects;
    _carryOverObjects.add(commandBuffer);
    if (pAttachments != nullptr) {
        for (uint32_t _i107 = 0;_i107 < attachmentCount; ++_i107) {
            const auto _s108 = &pAttachments[_i107];
            skip |= ValidateClearAttachment(_carryOverObjects,
                _s108->aspectMask,
                _s108->colorAttachment,
                _s108->clearValue);
        }
    }
    if (pRects != nullptr) {
        for (uint32_t _i109 = 0;_i109 < rectCount; ++_i109) {
            const auto _s110 = &pRects[_i109];
            skip |= ValidateClearRect(_carryOverObjects,
                _s110->rect,
                _s110->baseArrayLayer,
                _s110->layerCount);
        }
    }
    return skip;
}
bool ExplicitValidation::PreCallValidateCmdResolveImage(VkCommandBuffer commandBuffer,
                VkImage srcImage,
                VkImageLayout srcImageLayout,
                VkImage dstImage,
                VkImageLayout dstImageLayout,
                uint32_t regionCount,
                const VkImageResolve* pRegions) const {
    bool skip = false;
    LogObjectList _carryOverObjects;
    _carryOverObjects.add(commandBuffer);
    if (pRegions != nullptr) {
        for (uint32_t _i111 = 0;_i111 < regionCount; ++_i111) {
            const auto _s112 = &pRegions[_i111];
            skip |= ValidateImageResolve(_carryOverObjects,
                _s112->srcSubresource,
                _s112->srcOffset,
                _s112->dstSubresource,
                _s112->dstOffset,
                _s112->extent);
        }
    }
    return skip;
}
bool ExplicitValidation::PreCallValidateCmdSetEvent(VkCommandBuffer commandBuffer,
                VkEvent event,
                VkPipelineStageFlags stageMask) const {
    bool skip = false;
    LogObjectList _carryOverObjects;
    _carryOverObjects.add(commandBuffer);
    return skip;
}
bool ExplicitValidation::PreCallValidateCmdResetEvent(VkCommandBuffer commandBuffer,
                VkEvent event,
                VkPipelineStageFlags stageMask) const {
    bool skip = false;
    LogObjectList _carryOverObjects;
    _carryOverObjects.add(commandBuffer);
    return skip;
}
bool ExplicitValidation::PreCallValidateCmdWaitEvents(VkCommandBuffer commandBuffer,
                uint32_t eventCount,
                const VkEvent* pEvents,
                VkPipelineStageFlags srcStageMask,
                VkPipelineStageFlags dstStageMask,
                uint32_t memoryBarrierCount,
                const VkMemoryBarrier* pMemoryBarriers,
                uint32_t bufferMemoryBarrierCount,
                const VkBufferMemoryBarrier* pBufferMemoryBarriers,
                uint32_t imageMemoryBarrierCount,
                const VkImageMemoryBarrier* pImageMemoryBarriers) const {
    bool skip = false;
    LogObjectList _carryOverObjects;
    _carryOverObjects.add(commandBuffer);
    if (pMemoryBarriers != nullptr) {
        for (uint32_t _i113 = 0;_i113 < memoryBarrierCount; ++_i113) {
            const auto _s114 = &pMemoryBarriers[_i113];
            skip |= ValidateMemoryBarrier(_carryOverObjects,
                _s114->sType,
                _s114->pNext,
                _s114->srcAccessMask,
                _s114->dstAccessMask);
        }
    }
    if (pBufferMemoryBarriers != nullptr) {
        for (uint32_t _i115 = 0;_i115 < bufferMemoryBarrierCount; ++_i115) {
            const auto _s116 = &pBufferMemoryBarriers[_i115];
            skip |= ValidateBufferMemoryBarrier(_carryOverObjects,
                _s116->sType,
                _s116->pNext,
                _s116->srcAccessMask,
                _s116->dstAccessMask,
                _s116->srcQueueFamilyIndex,
                _s116->dstQueueFamilyIndex,
                _s116->buffer,
                _s116->offset,
                _s116->size);
        }
    }
    if (pImageMemoryBarriers != nullptr) {
        for (uint32_t _i117 = 0;_i117 < imageMemoryBarrierCount; ++_i117) {
            const auto _s118 = &pImageMemoryBarriers[_i117];
            skip |= ValidateImageMemoryBarrier(_carryOverObjects,
                _s118->sType,
                _s118->pNext,
                _s118->srcAccessMask,
                _s118->dstAccessMask,
                _s118->oldLayout,
                _s118->newLayout,
                _s118->srcQueueFamilyIndex,
                _s118->dstQueueFamilyIndex,
                _s118->image,
                _s118->subresourceRange);
        }
    }
    return skip;
}
bool ExplicitValidation::PreCallValidateCmdPipelineBarrier(VkCommandBuffer commandBuffer,
                VkPipelineStageFlags srcStageMask,
                VkPipelineStageFlags dstStageMask,
                VkDependencyFlags dependencyFlags,
                uint32_t memoryBarrierCount,
                const VkMemoryBarrier* pMemoryBarriers,
                uint32_t bufferMemoryBarrierCount,
                const VkBufferMemoryBarrier* pBufferMemoryBarriers,
                uint32_t imageMemoryBarrierCount,
                const VkImageMemoryBarrier* pImageMemoryBarriers) const {
    bool skip = false;
    LogObjectList _carryOverObjects;
    _carryOverObjects.add(commandBuffer);
    if (pMemoryBarriers != nullptr) {
        for (uint32_t _i119 = 0;_i119 < memoryBarrierCount; ++_i119) {
            const auto _s120 = &pMemoryBarriers[_i119];
            skip |= ValidateMemoryBarrier(_carryOverObjects,
                _s120->sType,
                _s120->pNext,
                _s120->srcAccessMask,
                _s120->dstAccessMask);
        }
    }
    if (pBufferMemoryBarriers != nullptr) {
        for (uint32_t _i121 = 0;_i121 < bufferMemoryBarrierCount; ++_i121) {
            const auto _s122 = &pBufferMemoryBarriers[_i121];
            skip |= ValidateBufferMemoryBarrier(_carryOverObjects,
                _s122->sType,
                _s122->pNext,
                _s122->srcAccessMask,
                _s122->dstAccessMask,
                _s122->srcQueueFamilyIndex,
                _s122->dstQueueFamilyIndex,
                _s122->buffer,
                _s122->offset,
                _s122->size);
        }
    }
    if (pImageMemoryBarriers != nullptr) {
        for (uint32_t _i123 = 0;_i123 < imageMemoryBarrierCount; ++_i123) {
            const auto _s124 = &pImageMemoryBarriers[_i123];
            skip |= ValidateImageMemoryBarrier(_carryOverObjects,
                _s124->sType,
                _s124->pNext,
                _s124->srcAccessMask,
                _s124->dstAccessMask,
                _s124->oldLayout,
                _s124->newLayout,
                _s124->srcQueueFamilyIndex,
                _s124->dstQueueFamilyIndex,
                _s124->image,
                _s124->subresourceRange);
        }
    }
    return skip;
}
bool ExplicitValidation::PreCallValidateCmdBeginQuery(VkCommandBuffer commandBuffer,
                VkQueryPool queryPool,
                uint32_t query,
                VkQueryControlFlags flags) const {
    bool skip = false;
    LogObjectList _carryOverObjects;
    _carryOverObjects.add(commandBuffer);
    return skip;
}
bool ExplicitValidation::PreCallValidateCmdEndQuery(VkCommandBuffer commandBuffer,
                VkQueryPool queryPool,
                uint32_t query) const {
    bool skip = false;
    LogObjectList _carryOverObjects;
    _carryOverObjects.add(commandBuffer);
    return skip;
}
bool ExplicitValidation::PreCallValidateCmdResetQueryPool(VkCommandBuffer commandBuffer,
                VkQueryPool queryPool,
                uint32_t firstQuery,
                uint32_t queryCount) const {
    bool skip = false;
    LogObjectList _carryOverObjects;
    _carryOverObjects.add(commandBuffer);
    return skip;
}
bool ExplicitValidation::PreCallValidateCmdWriteTimestamp(VkCommandBuffer commandBuffer,
                VkPipelineStageFlagBits pipelineStage,
                VkQueryPool queryPool,
                uint32_t query) const {
    bool skip = false;
    LogObjectList _carryOverObjects;
    _carryOverObjects.add(commandBuffer);
    return skip;
}
bool ExplicitValidation::PreCallValidateCmdCopyQueryPoolResults(VkCommandBuffer commandBuffer,
                VkQueryPool queryPool,
                uint32_t firstQuery,
                uint32_t queryCount,
                VkBuffer dstBuffer,
                VkDeviceSize dstOffset,
                VkDeviceSize stride,
                VkQueryResultFlags flags) const {
    bool skip = false;
    LogObjectList _carryOverObjects;
    _carryOverObjects.add(commandBuffer);
    return skip;
}
bool ExplicitValidation::PreCallValidateCmdPushConstants(VkCommandBuffer commandBuffer,
                VkPipelineLayout layout,
                VkShaderStageFlags stageFlags,
                uint32_t offset,
                uint32_t size,
                const void* pValues) const {
    bool skip = false;
    LogObjectList _carryOverObjects;
    _carryOverObjects.add(commandBuffer);
    return skip;
}
bool ExplicitValidation::PreCallValidateCmdBeginRenderPass(VkCommandBuffer commandBuffer,
                const VkRenderPassBeginInfo* pRenderPassBegin,
                VkSubpassContents contents) const {
    bool skip = false;
    LogObjectList _carryOverObjects;
    _carryOverObjects.add(commandBuffer);
    if (pRenderPassBegin != nullptr) {
        const auto _s125 = pRenderPassBegin;
        skip |= ValidateRenderPassBeginInfo(_carryOverObjects,
            _s125->sType,
            _s125->pNext,
            _s125->renderPass,
            _s125->framebuffer,
            _s125->renderArea,
            _s125->clearValueCount,
            _s125->pClearValues);
    }
    return skip;
}
bool ExplicitValidation::PreCallValidateCmdNextSubpass(VkCommandBuffer commandBuffer,
                VkSubpassContents contents) const {
    bool skip = false;
    LogObjectList _carryOverObjects;
    _carryOverObjects.add(commandBuffer);
    return skip;
}
bool ExplicitValidation::PreCallValidateCmdEndRenderPass(VkCommandBuffer commandBuffer) const {
    bool skip = false;
    LogObjectList _carryOverObjects;
    _carryOverObjects.add(commandBuffer);
    return skip;
}
bool ExplicitValidation::PreCallValidateCmdExecuteCommands(VkCommandBuffer commandBuffer,
                uint32_t commandBufferCount,
                const VkCommandBuffer* pCommandBuffers) const {
    bool skip = false;
    LogObjectList _carryOverObjects;
    _carryOverObjects.add(commandBuffer);
    return skip;
}
bool ExplicitValidation::PreCallValidateBindBufferMemory2(VkDevice device,
                uint32_t bindInfoCount,
                const VkBindBufferMemoryInfo* pBindInfos) const {
    bool skip = false;
    if (pBindInfos != nullptr) {
        for (uint32_t _i126 = 0;_i126 < bindInfoCount; ++_i126) {
            const auto _s127 = &pBindInfos[_i126];
            skip |= ValidateBindBufferMemoryInfo({},
                _s127->sType,
                _s127->pNext,
                _s127->buffer,
                _s127->memory,
                _s127->memoryOffset);
        }
    }
    return skip;
}
bool ExplicitValidation::PreCallValidateBindImageMemory2(VkDevice device,
                uint32_t bindInfoCount,
                const VkBindImageMemoryInfo* pBindInfos) const {
    bool skip = false;
    if (pBindInfos != nullptr) {
        for (uint32_t _i128 = 0;_i128 < bindInfoCount; ++_i128) {
            const auto _s129 = &pBindInfos[_i128];
            skip |= ValidateBindImageMemoryInfo({},
                _s129->sType,
                _s129->pNext,
                _s129->image,
                _s129->memory,
                _s129->memoryOffset);
        }
    }
    return skip;
}
bool ExplicitValidation::PreCallValidateGetDeviceGroupPeerMemoryFeatures(VkDevice device,
                uint32_t heapIndex,
                uint32_t localDeviceIndex,
                uint32_t remoteDeviceIndex,
                VkPeerMemoryFeatureFlags* pPeerMemoryFeatures) const {
    bool skip = false;
    return skip;
}
bool ExplicitValidation::PreCallValidateCmdSetDeviceMask(VkCommandBuffer commandBuffer,
                uint32_t deviceMask) const {
    bool skip = false;
    LogObjectList _carryOverObjects;
    _carryOverObjects.add(commandBuffer);
    return skip;
}
bool ExplicitValidation::PreCallValidateCmdDispatchBase(VkCommandBuffer commandBuffer,
                uint32_t baseGroupX,
                uint32_t baseGroupY,
                uint32_t baseGroupZ,
                uint32_t groupCountX,
                uint32_t groupCountY,
                uint32_t groupCountZ) const {
    bool skip = false;
    LogObjectList _carryOverObjects;
    _carryOverObjects.add(commandBuffer);
    return skip;
}
bool ExplicitValidation::PreCallValidateEnumeratePhysicalDeviceGroups(VkInstance instance,
                uint32_t* pPhysicalDeviceGroupCount,
                VkPhysicalDeviceGroupProperties* pPhysicalDeviceGroupProperties) const {
    bool skip = false;
    return skip;
}
bool ExplicitValidation::PreCallValidateGetImageMemoryRequirements2(VkDevice device,
                const VkImageMemoryRequirementsInfo2* pInfo,
                VkMemoryRequirements2* pMemoryRequirements) const {
    bool skip = false;
    if (pInfo != nullptr) {
        const auto _s130 = pInfo;
        skip |= ValidateImageMemoryRequirementsInfo2({},
            _s130->sType,
            _s130->pNext,
            _s130->image);
    }
    return skip;
}
bool ExplicitValidation::PreCallValidateGetBufferMemoryRequirements2(VkDevice device,
                const VkBufferMemoryRequirementsInfo2* pInfo,
                VkMemoryRequirements2* pMemoryRequirements) const {
    bool skip = false;
    if (pInfo != nullptr) {
        const auto _s131 = pInfo;
        skip |= ValidateBufferMemoryRequirementsInfo2({},
            _s131->sType,
            _s131->pNext,
            _s131->buffer);
    }
    return skip;
}
bool ExplicitValidation::PreCallValidateGetImageSparseMemoryRequirements2(VkDevice device,
                const VkImageSparseMemoryRequirementsInfo2* pInfo,
                uint32_t* pSparseMemoryRequirementCount,
                VkSparseImageMemoryRequirements2* pSparseMemoryRequirements) const {
    bool skip = false;
    if (pInfo != nullptr) {
        const auto _s132 = pInfo;
        skip |= ValidateImageSparseMemoryRequirementsInfo2({},
            _s132->sType,
            _s132->pNext,
            _s132->image);
    }
    return skip;
}
bool ExplicitValidation::PreCallValidateGetPhysicalDeviceFeatures2(VkPhysicalDevice physicalDevice,
                VkPhysicalDeviceFeatures2* pFeatures) const {
    bool skip = false;
    if (pFeatures != nullptr) {
        const auto _s133 = pFeatures;
        skip |= ValidatePhysicalDeviceFeatures2({},
            _s133->sType,
            _s133->pNext,
            _s133->features);
    }
    return skip;
}
bool ExplicitValidation::PreCallValidateGetPhysicalDeviceProperties2(VkPhysicalDevice physicalDevice,
                VkPhysicalDeviceProperties2* pProperties) const {
    bool skip = false;
    return skip;
}
bool ExplicitValidation::PreCallValidateGetPhysicalDeviceFormatProperties2(VkPhysicalDevice physicalDevice,
                VkFormat format,
                VkFormatProperties2* pFormatProperties) const {
    bool skip = false;
    return skip;
}
bool ExplicitValidation::PreCallValidateGetPhysicalDeviceImageFormatProperties2(VkPhysicalDevice physicalDevice,
                const VkPhysicalDeviceImageFormatInfo2* pImageFormatInfo,
                VkImageFormatProperties2* pImageFormatProperties) const {
    bool skip = false;
    if (pImageFormatInfo != nullptr) {
        const auto _s134 = pImageFormatInfo;
        skip |= ValidatePhysicalDeviceImageFormatInfo2({},
            _s134->sType,
            _s134->pNext,
            _s134->format,
            _s134->type,
            _s134->tiling,
            _s134->usage,
            _s134->flags);
    }
    return skip;
}
bool ExplicitValidation::PreCallValidateGetPhysicalDeviceQueueFamilyProperties2(VkPhysicalDevice physicalDevice,
                uint32_t* pQueueFamilyPropertyCount,
                VkQueueFamilyProperties2* pQueueFamilyProperties) const {
    bool skip = false;
    return skip;
}
bool ExplicitValidation::PreCallValidateGetPhysicalDeviceMemoryProperties2(VkPhysicalDevice physicalDevice,
                VkPhysicalDeviceMemoryProperties2* pMemoryProperties) const {
    bool skip = false;
    return skip;
}
bool ExplicitValidation::PreCallValidateGetPhysicalDeviceSparseImageFormatProperties2(VkPhysicalDevice physicalDevice,
                const VkPhysicalDeviceSparseImageFormatInfo2* pFormatInfo,
                uint32_t* pPropertyCount,
                VkSparseImageFormatProperties2* pProperties) const {
    bool skip = false;
    if (pFormatInfo != nullptr) {
        const auto _s135 = pFormatInfo;
        skip |= ValidatePhysicalDeviceSparseImageFormatInfo2({},
            _s135->sType,
            _s135->pNext,
            _s135->format,
            _s135->type,
            _s135->samples,
            _s135->usage,
            _s135->tiling);
    }
    return skip;
}
bool ExplicitValidation::PreCallValidateTrimCommandPool(VkDevice device,
                VkCommandPool commandPool,
                VkCommandPoolTrimFlags flags) const {
    bool skip = false;
    return skip;
}
bool ExplicitValidation::PreCallValidateGetDeviceQueue2(VkDevice device,
                const VkDeviceQueueInfo2* pQueueInfo,
                VkQueue* pQueue) const {
    bool skip = false;
    if (pQueueInfo != nullptr) {
        const auto _s136 = pQueueInfo;
        skip |= ValidateDeviceQueueInfo2({},
            _s136->sType,
            _s136->pNext,
            _s136->flags,
            _s136->queueFamilyIndex,
            _s136->queueIndex);
    }
    return skip;
}
bool ExplicitValidation::PreCallValidateCreateSamplerYcbcrConversion(VkDevice device,
                const VkSamplerYcbcrConversionCreateInfo* pCreateInfo,
                const VkAllocationCallbacks* pAllocator,
                VkSamplerYcbcrConversion* pYcbcrConversion) const {
    bool skip = false;
    if (pCreateInfo != nullptr) {
        const auto _s137 = pCreateInfo;
        skip |= ValidateSamplerYcbcrConversionCreateInfo({},
            _s137->sType,
            _s137->pNext,
            _s137->format,
            _s137->ycbcrModel,
            _s137->ycbcrRange,
            _s137->components,
            _s137->xChromaOffset,
            _s137->yChromaOffset,
            _s137->chromaFilter,
            _s137->forceExplicitReconstruction);
    }
    if (pAllocator != nullptr) {
        const auto _s138 = pAllocator;
        skip |= ValidateAllocationCallbacks({},
            _s138->pUserData,
            _s138->pfnAllocation,
            _s138->pfnReallocation,
            _s138->pfnFree,
            _s138->pfnInternalAllocation,
            _s138->pfnInternalFree);
    }
    return skip;
}
bool ExplicitValidation::PreCallValidateDestroySamplerYcbcrConversion(VkDevice device,
                VkSamplerYcbcrConversion ycbcrConversion,
                const VkAllocationCallbacks* pAllocator) const {
    bool skip = false;
    if (pAllocator != nullptr) {
        const auto _s139 = pAllocator;
        skip |= ValidateAllocationCallbacks({},
            _s139->pUserData,
            _s139->pfnAllocation,
            _s139->pfnReallocation,
            _s139->pfnFree,
            _s139->pfnInternalAllocation,
            _s139->pfnInternalFree);
    }
    return skip;
}
bool ExplicitValidation::PreCallValidateCreateDescriptorUpdateTemplate(VkDevice device,
                const VkDescriptorUpdateTemplateCreateInfo* pCreateInfo,
                const VkAllocationCallbacks* pAllocator,
                VkDescriptorUpdateTemplate* pDescriptorUpdateTemplate) const {
    bool skip = false;
    if (pCreateInfo != nullptr) {
        const auto _s140 = pCreateInfo;
        skip |= ValidateDescriptorUpdateTemplateCreateInfo({},
            _s140->sType,
            _s140->pNext,
            _s140->flags,
            _s140->descriptorUpdateEntryCount,
            _s140->pDescriptorUpdateEntries,
            _s140->templateType,
            _s140->descriptorSetLayout,
            _s140->pipelineBindPoint,
            _s140->pipelineLayout,
            _s140->set);
    }
    if (pAllocator != nullptr) {
        const auto _s141 = pAllocator;
        skip |= ValidateAllocationCallbacks({},
            _s141->pUserData,
            _s141->pfnAllocation,
            _s141->pfnReallocation,
            _s141->pfnFree,
            _s141->pfnInternalAllocation,
            _s141->pfnInternalFree);
    }
    return skip;
}
bool ExplicitValidation::PreCallValidateDestroyDescriptorUpdateTemplate(VkDevice device,
                VkDescriptorUpdateTemplate descriptorUpdateTemplate,
                const VkAllocationCallbacks* pAllocator) const {
    bool skip = false;
    if (pAllocator != nullptr) {
        const auto _s142 = pAllocator;
        skip |= ValidateAllocationCallbacks({},
            _s142->pUserData,
            _s142->pfnAllocation,
            _s142->pfnReallocation,
            _s142->pfnFree,
            _s142->pfnInternalAllocation,
            _s142->pfnInternalFree);
    }
    return skip;
}
bool ExplicitValidation::PreCallValidateUpdateDescriptorSetWithTemplate(VkDevice device,
                VkDescriptorSet descriptorSet,
                VkDescriptorUpdateTemplate descriptorUpdateTemplate,
                const void* pData) const {
    bool skip = false;
    return skip;
}
bool ExplicitValidation::PreCallValidateGetPhysicalDeviceExternalBufferProperties(VkPhysicalDevice physicalDevice,
                const VkPhysicalDeviceExternalBufferInfo* pExternalBufferInfo,
                VkExternalBufferProperties* pExternalBufferProperties) const {
    bool skip = false;
    if (pExternalBufferInfo != nullptr) {
        const auto _s143 = pExternalBufferInfo;
        skip |= ValidatePhysicalDeviceExternalBufferInfo({},
            _s143->sType,
            _s143->pNext,
            _s143->flags,
            _s143->usage,
            _s143->handleType);
    }
    return skip;
}
bool ExplicitValidation::PreCallValidateGetPhysicalDeviceExternalFenceProperties(VkPhysicalDevice physicalDevice,
                const VkPhysicalDeviceExternalFenceInfo* pExternalFenceInfo,
                VkExternalFenceProperties* pExternalFenceProperties) const {
    bool skip = false;
    if (pExternalFenceInfo != nullptr) {
        const auto _s144 = pExternalFenceInfo;
        skip |= ValidatePhysicalDeviceExternalFenceInfo({},
            _s144->sType,
            _s144->pNext,
            _s144->handleType);
    }
    return skip;
}
bool ExplicitValidation::PreCallValidateGetPhysicalDeviceExternalSemaphoreProperties(VkPhysicalDevice physicalDevice,
                const VkPhysicalDeviceExternalSemaphoreInfo* pExternalSemaphoreInfo,
                VkExternalSemaphoreProperties* pExternalSemaphoreProperties) const {
    bool skip = false;
    if (pExternalSemaphoreInfo != nullptr) {
        const auto _s145 = pExternalSemaphoreInfo;
        skip |= ValidatePhysicalDeviceExternalSemaphoreInfo({},
            _s145->sType,
            _s145->pNext,
            _s145->handleType);
    }
    return skip;
}
bool ExplicitValidation::PreCallValidateGetDescriptorSetLayoutSupport(VkDevice device,
                const VkDescriptorSetLayoutCreateInfo* pCreateInfo,
                VkDescriptorSetLayoutSupport* pSupport) const {
    bool skip = false;
    if (pCreateInfo != nullptr) {
        const auto _s146 = pCreateInfo;
        skip |= ValidateDescriptorSetLayoutCreateInfo({},
            _s146->sType,
            _s146->pNext,
            _s146->flags,
            _s146->bindingCount,
            _s146->pBindings);
    }
    return skip;
}
bool ExplicitValidation::PreCallValidateCmdDrawIndirectCount(VkCommandBuffer commandBuffer,
                VkBuffer buffer,
                VkDeviceSize offset,
                VkBuffer countBuffer,
                VkDeviceSize countBufferOffset,
                uint32_t maxDrawCount,
                uint32_t stride) const {
    bool skip = false;
    LogObjectList _carryOverObjects;
    _carryOverObjects.add(commandBuffer);
    return skip;
}
bool ExplicitValidation::PreCallValidateCmdDrawIndexedIndirectCount(VkCommandBuffer commandBuffer,
                VkBuffer buffer,
                VkDeviceSize offset,
                VkBuffer countBuffer,
                VkDeviceSize countBufferOffset,
                uint32_t maxDrawCount,
                uint32_t stride) const {
    bool skip = false;
    LogObjectList _carryOverObjects;
    _carryOverObjects.add(commandBuffer);
    return skip;
}
bool ExplicitValidation::PreCallValidateCreateRenderPass2(VkDevice device,
                const VkRenderPassCreateInfo2* pCreateInfo,
                const VkAllocationCallbacks* pAllocator,
                VkRenderPass* pRenderPass) const {
    bool skip = false;
    if (pCreateInfo != nullptr) {
        const auto _s147 = pCreateInfo;
        skip |= ValidateRenderPassCreateInfo2({},
            _s147->sType,
            _s147->pNext,
            _s147->flags,
            _s147->attachmentCount,
            _s147->pAttachments,
            _s147->subpassCount,
            _s147->pSubpasses,
            _s147->dependencyCount,
            _s147->pDependencies,
            _s147->correlatedViewMaskCount,
            _s147->pCorrelatedViewMasks);
    }
    if (pAllocator != nullptr) {
        const auto _s148 = pAllocator;
        skip |= ValidateAllocationCallbacks({},
            _s148->pUserData,
            _s148->pfnAllocation,
            _s148->pfnReallocation,
            _s148->pfnFree,
            _s148->pfnInternalAllocation,
            _s148->pfnInternalFree);
    }
    return skip;
}
bool ExplicitValidation::PreCallValidateCmdBeginRenderPass2(VkCommandBuffer commandBuffer,
                const VkRenderPassBeginInfo* pRenderPassBegin,
                const VkSubpassBeginInfo* pSubpassBeginInfo) const {
    bool skip = false;
    LogObjectList _carryOverObjects;
    _carryOverObjects.add(commandBuffer);
    if (pRenderPassBegin != nullptr) {
        const auto _s149 = pRenderPassBegin;
        skip |= ValidateRenderPassBeginInfo(_carryOverObjects,
            _s149->sType,
            _s149->pNext,
            _s149->renderPass,
            _s149->framebuffer,
            _s149->renderArea,
            _s149->clearValueCount,
            _s149->pClearValues);
    }
    if (pSubpassBeginInfo != nullptr) {
        const auto _s150 = pSubpassBeginInfo;
        skip |= ValidateSubpassBeginInfo(_carryOverObjects,
            _s150->sType,
            _s150->pNext,
            _s150->contents);
    }
    return skip;
}
bool ExplicitValidation::PreCallValidateCmdNextSubpass2(VkCommandBuffer commandBuffer,
                const VkSubpassBeginInfo* pSubpassBeginInfo,
                const VkSubpassEndInfo* pSubpassEndInfo) const {
    bool skip = false;
    LogObjectList _carryOverObjects;
    _carryOverObjects.add(commandBuffer);
    if (pSubpassBeginInfo != nullptr) {
        const auto _s151 = pSubpassBeginInfo;
        skip |= ValidateSubpassBeginInfo(_carryOverObjects,
            _s151->sType,
            _s151->pNext,
            _s151->contents);
    }
    if (pSubpassEndInfo != nullptr) {
        const auto _s152 = pSubpassEndInfo;
        skip |= ValidateSubpassEndInfo(_carryOverObjects,
            _s152->sType,
            _s152->pNext);
    }
    return skip;
}
bool ExplicitValidation::PreCallValidateCmdEndRenderPass2(VkCommandBuffer commandBuffer,
                const VkSubpassEndInfo* pSubpassEndInfo) const {
    bool skip = false;
    LogObjectList _carryOverObjects;
    _carryOverObjects.add(commandBuffer);
    if (pSubpassEndInfo != nullptr) {
        const auto _s153 = pSubpassEndInfo;
        skip |= ValidateSubpassEndInfo(_carryOverObjects,
            _s153->sType,
            _s153->pNext);
    }
    return skip;
}
bool ExplicitValidation::PreCallValidateResetQueryPool(VkDevice device,
                VkQueryPool queryPool,
                uint32_t firstQuery,
                uint32_t queryCount) const {
    bool skip = false;
    return skip;
}
bool ExplicitValidation::PreCallValidateGetSemaphoreCounterValue(VkDevice device,
                VkSemaphore semaphore,
                uint64_t* pValue) const {
    bool skip = false;
    return skip;
}
bool ExplicitValidation::PreCallValidateWaitSemaphores(VkDevice device,
                const VkSemaphoreWaitInfo* pWaitInfo,
                uint64_t timeout) const {
    bool skip = false;
    if (pWaitInfo != nullptr) {
        const auto _s154 = pWaitInfo;
        skip |= ValidateSemaphoreWaitInfo({},
            _s154->sType,
            _s154->pNext,
            _s154->flags,
            _s154->semaphoreCount,
            _s154->pSemaphores,
            _s154->pValues);
    }
    return skip;
}
bool ExplicitValidation::PreCallValidateSignalSemaphore(VkDevice device,
                const VkSemaphoreSignalInfo* pSignalInfo) const {
    bool skip = false;
    if (pSignalInfo != nullptr) {
        const auto _s155 = pSignalInfo;
        skip |= ValidateSemaphoreSignalInfo({},
            _s155->sType,
            _s155->pNext,
            _s155->semaphore,
            _s155->value);
    }
    return skip;
}
bool ExplicitValidation::PreCallValidateGetBufferDeviceAddress(VkDevice device,
                const VkBufferDeviceAddressInfo* pInfo) const {
    bool skip = false;
    if (pInfo != nullptr) {
        const auto _s156 = pInfo;
        skip |= ValidateBufferDeviceAddressInfo({},
            _s156->sType,
            _s156->pNext,
            _s156->buffer);
    }
    return skip;
}
bool ExplicitValidation::PreCallValidateGetBufferOpaqueCaptureAddress(VkDevice device,
                const VkBufferDeviceAddressInfo* pInfo) const {
    bool skip = false;
    if (pInfo != nullptr) {
        const auto _s157 = pInfo;
        skip |= ValidateBufferDeviceAddressInfo({},
            _s157->sType,
            _s157->pNext,
            _s157->buffer);
    }
    return skip;
}
bool ExplicitValidation::PreCallValidateGetDeviceMemoryOpaqueCaptureAddress(VkDevice device,
                const VkDeviceMemoryOpaqueCaptureAddressInfo* pInfo) const {
    bool skip = false;
    if (pInfo != nullptr) {
        const auto _s158 = pInfo;
        skip |= ValidateDeviceMemoryOpaqueCaptureAddressInfo({},
            _s158->sType,
            _s158->pNext,
            _s158->memory);
    }
    return skip;
}
bool ExplicitValidation::PreCallValidateGetPhysicalDeviceToolProperties(VkPhysicalDevice physicalDevice,
                uint32_t* pToolCount,
                VkPhysicalDeviceToolProperties* pToolProperties) const {
    bool skip = false;
    return skip;
}
bool ExplicitValidation::PreCallValidateCreatePrivateDataSlot(VkDevice device,
                const VkPrivateDataSlotCreateInfo* pCreateInfo,
                const VkAllocationCallbacks* pAllocator,
                VkPrivateDataSlot* pPrivateDataSlot) const {
    bool skip = false;
    if (pCreateInfo != nullptr) {
        const auto _s159 = pCreateInfo;
        skip |= ValidatePrivateDataSlotCreateInfo({},
            _s159->sType,
            _s159->pNext,
            _s159->flags);
    }
    if (pAllocator != nullptr) {
        const auto _s160 = pAllocator;
        skip |= ValidateAllocationCallbacks({},
            _s160->pUserData,
            _s160->pfnAllocation,
            _s160->pfnReallocation,
            _s160->pfnFree,
            _s160->pfnInternalAllocation,
            _s160->pfnInternalFree);
    }
    return skip;
}
bool ExplicitValidation::PreCallValidateDestroyPrivateDataSlot(VkDevice device,
                VkPrivateDataSlot privateDataSlot,
                const VkAllocationCallbacks* pAllocator) const {
    bool skip = false;
    if (pAllocator != nullptr) {
        const auto _s161 = pAllocator;
        skip |= ValidateAllocationCallbacks({},
            _s161->pUserData,
            _s161->pfnAllocation,
            _s161->pfnReallocation,
            _s161->pfnFree,
            _s161->pfnInternalAllocation,
            _s161->pfnInternalFree);
    }
    return skip;
}
bool ExplicitValidation::PreCallValidateSetPrivateData(VkDevice device,
                VkObjectType objectType,
                uint64_t objectHandle,
                VkPrivateDataSlot privateDataSlot,
                uint64_t data) const {
    bool skip = false;
    return skip;
}
bool ExplicitValidation::PreCallValidateGetPrivateData(VkDevice device,
                VkObjectType objectType,
                uint64_t objectHandle,
                VkPrivateDataSlot privateDataSlot,
                uint64_t* pData) const {
    bool skip = false;
    return skip;
}
bool ExplicitValidation::PreCallValidateCmdSetEvent2(VkCommandBuffer commandBuffer,
                VkEvent event,
                const VkDependencyInfo* pDependencyInfo) const {
    bool skip = false;
    LogObjectList _carryOverObjects;
    _carryOverObjects.add(commandBuffer);
    if (pDependencyInfo != nullptr) {
        const auto _s162 = pDependencyInfo;
        skip |= ValidateDependencyInfo(_carryOverObjects,
            _s162->sType,
            _s162->pNext,
            _s162->dependencyFlags,
            _s162->memoryBarrierCount,
            _s162->pMemoryBarriers,
            _s162->bufferMemoryBarrierCount,
            _s162->pBufferMemoryBarriers,
            _s162->imageMemoryBarrierCount,
            _s162->pImageMemoryBarriers);
    }
    return skip;
}
bool ExplicitValidation::PreCallValidateCmdResetEvent2(VkCommandBuffer commandBuffer,
                VkEvent event,
                VkPipelineStageFlags2 stageMask) const {
    bool skip = false;
    LogObjectList _carryOverObjects;
    _carryOverObjects.add(commandBuffer);
    return skip;
}
bool ExplicitValidation::PreCallValidateCmdWaitEvents2(VkCommandBuffer commandBuffer,
                uint32_t eventCount,
                const VkEvent* pEvents,
                const VkDependencyInfo* pDependencyInfos) const {
    bool skip = false;
    LogObjectList _carryOverObjects;
    _carryOverObjects.add(commandBuffer);
    if (pDependencyInfos != nullptr) {
        for (uint32_t _i163 = 0;_i163 < eventCount; ++_i163) {
            const auto _s164 = &pDependencyInfos[_i163];
            skip |= ValidateDependencyInfo(_carryOverObjects,
                _s164->sType,
                _s164->pNext,
                _s164->dependencyFlags,
                _s164->memoryBarrierCount,
                _s164->pMemoryBarriers,
                _s164->bufferMemoryBarrierCount,
                _s164->pBufferMemoryBarriers,
                _s164->imageMemoryBarrierCount,
                _s164->pImageMemoryBarriers);
        }
    }
    return skip;
}
bool ExplicitValidation::PreCallValidateCmdPipelineBarrier2(VkCommandBuffer commandBuffer,
                const VkDependencyInfo* pDependencyInfo) const {
    bool skip = false;
    LogObjectList _carryOverObjects;
    _carryOverObjects.add(commandBuffer);
    if (pDependencyInfo != nullptr) {
        const auto _s165 = pDependencyInfo;
        skip |= ValidateDependencyInfo(_carryOverObjects,
            _s165->sType,
            _s165->pNext,
            _s165->dependencyFlags,
            _s165->memoryBarrierCount,
            _s165->pMemoryBarriers,
            _s165->bufferMemoryBarrierCount,
            _s165->pBufferMemoryBarriers,
            _s165->imageMemoryBarrierCount,
            _s165->pImageMemoryBarriers);
    }
    return skip;
}
bool ExplicitValidation::PreCallValidateCmdWriteTimestamp2(VkCommandBuffer commandBuffer,
                VkPipelineStageFlags2 stage,
                VkQueryPool queryPool,
                uint32_t query) const {
    bool skip = false;
    LogObjectList _carryOverObjects;
    _carryOverObjects.add(commandBuffer);
    return skip;
}
bool ExplicitValidation::PreCallValidateQueueSubmit2(VkQueue queue,
                uint32_t submitCount,
                const VkSubmitInfo2* pSubmits,
                VkFence fence) const {
    bool skip = false;
    if (pSubmits != nullptr) {
        for (uint32_t _i166 = 0;_i166 < submitCount; ++_i166) {
            const auto _s167 = &pSubmits[_i166];
            skip |= ValidateSubmitInfo2({},
                _s167->sType,
                _s167->pNext,
                _s167->flags,
                _s167->waitSemaphoreInfoCount,
                _s167->pWaitSemaphoreInfos,
                _s167->commandBufferInfoCount,
                _s167->pCommandBufferInfos,
                _s167->signalSemaphoreInfoCount,
                _s167->pSignalSemaphoreInfos);
        }
    }
    return skip;
}
bool ExplicitValidation::PreCallValidateCmdCopyBuffer2(VkCommandBuffer commandBuffer,
                const VkCopyBufferInfo2* pCopyBufferInfo) const {
    bool skip = false;
    LogObjectList _carryOverObjects;
    _carryOverObjects.add(commandBuffer);
    if (pCopyBufferInfo != nullptr) {
        const auto _s168 = pCopyBufferInfo;
        skip |= ValidateCopyBufferInfo2(_carryOverObjects,
            _s168->sType,
            _s168->pNext,
            _s168->srcBuffer,
            _s168->dstBuffer,
            _s168->regionCount,
            _s168->pRegions);
    }
    return skip;
}
bool ExplicitValidation::PreCallValidateCmdCopyImage2(VkCommandBuffer commandBuffer,
                const VkCopyImageInfo2* pCopyImageInfo) const {
    bool skip = false;
    LogObjectList _carryOverObjects;
    _carryOverObjects.add(commandBuffer);
    if (pCopyImageInfo != nullptr) {
        const auto _s169 = pCopyImageInfo;
        skip |= ValidateCopyImageInfo2(_carryOverObjects,
            _s169->sType,
            _s169->pNext,
            _s169->srcImage,
            _s169->srcImageLayout,
            _s169->dstImage,
            _s169->dstImageLayout,
            _s169->regionCount,
            _s169->pRegions);
    }
    return skip;
}
bool ExplicitValidation::PreCallValidateCmdCopyBufferToImage2(VkCommandBuffer commandBuffer,
                const VkCopyBufferToImageInfo2* pCopyBufferToImageInfo) const {
    bool skip = false;
    LogObjectList _carryOverObjects;
    _carryOverObjects.add(commandBuffer);
    if (pCopyBufferToImageInfo != nullptr) {
        const auto _s170 = pCopyBufferToImageInfo;
        skip |= ValidateCopyBufferToImageInfo2(_carryOverObjects,
            _s170->sType,
            _s170->pNext,
            _s170->srcBuffer,
            _s170->dstImage,
            _s170->dstImageLayout,
            _s170->regionCount,
            _s170->pRegions);
    }
    return skip;
}
bool ExplicitValidation::PreCallValidateCmdCopyImageToBuffer2(VkCommandBuffer commandBuffer,
                const VkCopyImageToBufferInfo2* pCopyImageToBufferInfo) const {
    bool skip = false;
    LogObjectList _carryOverObjects;
    _carryOverObjects.add(commandBuffer);
    if (pCopyImageToBufferInfo != nullptr) {
        const auto _s171 = pCopyImageToBufferInfo;
        skip |= ValidateCopyImageToBufferInfo2(_carryOverObjects,
            _s171->sType,
            _s171->pNext,
            _s171->srcImage,
            _s171->srcImageLayout,
            _s171->dstBuffer,
            _s171->regionCount,
            _s171->pRegions);
    }
    return skip;
}
bool ExplicitValidation::PreCallValidateCmdBlitImage2(VkCommandBuffer commandBuffer,
                const VkBlitImageInfo2* pBlitImageInfo) const {
    bool skip = false;
    LogObjectList _carryOverObjects;
    _carryOverObjects.add(commandBuffer);
    if (pBlitImageInfo != nullptr) {
        const auto _s172 = pBlitImageInfo;
        skip |= ValidateBlitImageInfo2(_carryOverObjects,
            _s172->sType,
            _s172->pNext,
            _s172->srcImage,
            _s172->srcImageLayout,
            _s172->dstImage,
            _s172->dstImageLayout,
            _s172->regionCount,
            _s172->pRegions,
            _s172->filter);
    }
    return skip;
}
bool ExplicitValidation::PreCallValidateCmdResolveImage2(VkCommandBuffer commandBuffer,
                const VkResolveImageInfo2* pResolveImageInfo) const {
    bool skip = false;
    LogObjectList _carryOverObjects;
    _carryOverObjects.add(commandBuffer);
    if (pResolveImageInfo != nullptr) {
        const auto _s173 = pResolveImageInfo;
        skip |= ValidateResolveImageInfo2(_carryOverObjects,
            _s173->sType,
            _s173->pNext,
            _s173->srcImage,
            _s173->srcImageLayout,
            _s173->dstImage,
            _s173->dstImageLayout,
            _s173->regionCount,
            _s173->pRegions);
    }
    return skip;
}
bool ExplicitValidation::PreCallValidateCmdBeginRendering(VkCommandBuffer commandBuffer,
                const VkRenderingInfo* pRenderingInfo) const {
    bool skip = false;
    LogObjectList _carryOverObjects;
    _carryOverObjects.add(commandBuffer);
    if (pRenderingInfo != nullptr) {
        const auto _s174 = pRenderingInfo;
        skip |= ValidateRenderingInfo(_carryOverObjects,
            _s174->sType,
            _s174->pNext,
            _s174->flags,
            _s174->renderArea,
            _s174->layerCount,
            _s174->viewMask,
            _s174->colorAttachmentCount,
            _s174->pColorAttachments,
            _s174->pDepthAttachment,
            _s174->pStencilAttachment);
    }
    return skip;
}
bool ExplicitValidation::PreCallValidateCmdEndRendering(VkCommandBuffer commandBuffer) const {
    bool skip = false;
    LogObjectList _carryOverObjects;
    _carryOverObjects.add(commandBuffer);
    return skip;
}
bool ExplicitValidation::PreCallValidateCmdSetCullMode(VkCommandBuffer commandBuffer,
                VkCullModeFlags cullMode) const {
    bool skip = false;
    LogObjectList _carryOverObjects;
    _carryOverObjects.add(commandBuffer);
    return skip;
}
bool ExplicitValidation::PreCallValidateCmdSetFrontFace(VkCommandBuffer commandBuffer,
                VkFrontFace frontFace) const {
    bool skip = false;
    LogObjectList _carryOverObjects;
    _carryOverObjects.add(commandBuffer);
    return skip;
}
bool ExplicitValidation::PreCallValidateCmdSetPrimitiveTopology(VkCommandBuffer commandBuffer,
                VkPrimitiveTopology primitiveTopology) const {
    bool skip = false;
    LogObjectList _carryOverObjects;
    _carryOverObjects.add(commandBuffer);
    return skip;
}
bool ExplicitValidation::PreCallValidateCmdSetViewportWithCount(VkCommandBuffer commandBuffer,
                uint32_t viewportCount,
                const VkViewport* pViewports) const {
    bool skip = false;
    LogObjectList _carryOverObjects;
    _carryOverObjects.add(commandBuffer);
    if (pViewports != nullptr) {
        for (uint32_t _i175 = 0;_i175 < viewportCount; ++_i175) {
            const auto _s176 = &pViewports[_i175];
            skip |= ValidateViewport(_carryOverObjects,
                _s176->x,
                _s176->y,
                _s176->width,
                _s176->height,
                _s176->minDepth,
                _s176->maxDepth);
        }
    }
    return skip;
}
bool ExplicitValidation::PreCallValidateCmdSetScissorWithCount(VkCommandBuffer commandBuffer,
                uint32_t scissorCount,
                const VkRect2D* pScissors) const {
    bool skip = false;
    LogObjectList _carryOverObjects;
    _carryOverObjects.add(commandBuffer);
    if (pScissors != nullptr) {
        for (uint32_t _i177 = 0;_i177 < scissorCount; ++_i177) {
            const auto _s178 = &pScissors[_i177];
            skip |= ValidateRect2D(_carryOverObjects,
                _s178->offset,
                _s178->extent);
        }
    }
    return skip;
}
bool ExplicitValidation::PreCallValidateCmdBindVertexBuffers2(VkCommandBuffer commandBuffer,
                uint32_t firstBinding,
                uint32_t bindingCount,
                const VkBuffer* pBuffers,
                const VkDeviceSize* pOffsets,
                const VkDeviceSize* pSizes,
                const VkDeviceSize* pStrides) const {
    bool skip = false;
    LogObjectList _carryOverObjects;
    _carryOverObjects.add(commandBuffer);
    return skip;
}
bool ExplicitValidation::PreCallValidateCmdSetDepthTestEnable(VkCommandBuffer commandBuffer,
                VkBool32 depthTestEnable) const {
    bool skip = false;
    LogObjectList _carryOverObjects;
    _carryOverObjects.add(commandBuffer);
    return skip;
}
bool ExplicitValidation::PreCallValidateCmdSetDepthWriteEnable(VkCommandBuffer commandBuffer,
                VkBool32 depthWriteEnable) const {
    bool skip = false;
    LogObjectList _carryOverObjects;
    _carryOverObjects.add(commandBuffer);
    return skip;
}
bool ExplicitValidation::PreCallValidateCmdSetDepthCompareOp(VkCommandBuffer commandBuffer,
                VkCompareOp depthCompareOp) const {
    bool skip = false;
    LogObjectList _carryOverObjects;
    _carryOverObjects.add(commandBuffer);
    return skip;
}
bool ExplicitValidation::PreCallValidateCmdSetDepthBoundsTestEnable(VkCommandBuffer commandBuffer,
                VkBool32 depthBoundsTestEnable) const {
    bool skip = false;
    LogObjectList _carryOverObjects;
    _carryOverObjects.add(commandBuffer);
    return skip;
}
bool ExplicitValidation::PreCallValidateCmdSetStencilTestEnable(VkCommandBuffer commandBuffer,
                VkBool32 stencilTestEnable) const {
    bool skip = false;
    LogObjectList _carryOverObjects;
    _carryOverObjects.add(commandBuffer);
    return skip;
}
bool ExplicitValidation::PreCallValidateCmdSetStencilOp(VkCommandBuffer commandBuffer,
                VkStencilFaceFlags faceMask,
                VkStencilOp failOp,
                VkStencilOp passOp,
                VkStencilOp depthFailOp,
                VkCompareOp compareOp) const {
    bool skip = false;
    LogObjectList _carryOverObjects;
    _carryOverObjects.add(commandBuffer);
    return skip;
}
bool ExplicitValidation::PreCallValidateCmdSetRasterizerDiscardEnable(VkCommandBuffer commandBuffer,
                VkBool32 rasterizerDiscardEnable) const {
    bool skip = false;
    LogObjectList _carryOverObjects;
    _carryOverObjects.add(commandBuffer);
    return skip;
}
bool ExplicitValidation::PreCallValidateCmdSetDepthBiasEnable(VkCommandBuffer commandBuffer,
                VkBool32 depthBiasEnable) const {
    bool skip = false;
    LogObjectList _carryOverObjects;
    _carryOverObjects.add(commandBuffer);
    return skip;
}
bool ExplicitValidation::PreCallValidateCmdSetPrimitiveRestartEnable(VkCommandBuffer commandBuffer,
                VkBool32 primitiveRestartEnable) const {
    bool skip = false;
    LogObjectList _carryOverObjects;
    _carryOverObjects.add(commandBuffer);
    return skip;
}
bool ExplicitValidation::PreCallValidateGetDeviceBufferMemoryRequirements(VkDevice device,
                const VkDeviceBufferMemoryRequirements* pInfo,
                VkMemoryRequirements2* pMemoryRequirements) const {
    bool skip = false;
    if (pInfo != nullptr) {
        const auto _s179 = pInfo;
        skip |= ValidateDeviceBufferMemoryRequirements({},
            _s179->sType,
            _s179->pNext,
            _s179->pCreateInfo);
    }
    return skip;
}
bool ExplicitValidation::PreCallValidateGetDeviceImageMemoryRequirements(VkDevice device,
                const VkDeviceImageMemoryRequirements* pInfo,
                VkMemoryRequirements2* pMemoryRequirements) const {
    bool skip = false;
    if (pInfo != nullptr) {
        const auto _s180 = pInfo;
        skip |= ValidateDeviceImageMemoryRequirements({},
            _s180->sType,
            _s180->pNext,
            _s180->pCreateInfo,
            _s180->planeAspect);
    }
    return skip;
}
bool ExplicitValidation::PreCallValidateGetDeviceImageSparseMemoryRequirements(VkDevice device,
                const VkDeviceImageMemoryRequirements* pInfo,
                uint32_t* pSparseMemoryRequirementCount,
                VkSparseImageMemoryRequirements2* pSparseMemoryRequirements) const {
    bool skip = false;
    if (pInfo != nullptr) {
        const auto _s181 = pInfo;
        skip |= ValidateDeviceImageMemoryRequirements({},
            _s181->sType,
            _s181->pNext,
            _s181->pCreateInfo,
            _s181->planeAspect);
    }
    return skip;
}
bool ExplicitValidation::PreCallValidateDestroySurfaceKHR(VkInstance instance,
                VkSurfaceKHR surface,
                const VkAllocationCallbacks* pAllocator) const {
    bool skip = false;
    if (pAllocator != nullptr) {
        const auto _s182 = pAllocator;
        skip |= ValidateAllocationCallbacks({},
            _s182->pUserData,
            _s182->pfnAllocation,
            _s182->pfnReallocation,
            _s182->pfnFree,
            _s182->pfnInternalAllocation,
            _s182->pfnInternalFree);
    }
    return skip;
}
bool ExplicitValidation::PreCallValidateGetPhysicalDeviceSurfaceSupportKHR(VkPhysicalDevice physicalDevice,
                uint32_t queueFamilyIndex,
                VkSurfaceKHR surface,
                VkBool32* pSupported) const {
    bool skip = false;
    return skip;
}
bool ExplicitValidation::PreCallValidateGetPhysicalDeviceSurfaceCapabilitiesKHR(VkPhysicalDevice physicalDevice,
                VkSurfaceKHR surface,
                VkSurfaceCapabilitiesKHR* pSurfaceCapabilities) const {
    bool skip = false;
    return skip;
}
bool ExplicitValidation::PreCallValidateGetPhysicalDeviceSurfaceFormatsKHR(VkPhysicalDevice physicalDevice,
                VkSurfaceKHR surface,
                uint32_t* pSurfaceFormatCount,
                VkSurfaceFormatKHR* pSurfaceFormats) const {
    bool skip = false;
    return skip;
}
bool ExplicitValidation::PreCallValidateGetPhysicalDeviceSurfacePresentModesKHR(VkPhysicalDevice physicalDevice,
                VkSurfaceKHR surface,
                uint32_t* pPresentModeCount,
                VkPresentModeKHR* pPresentModes) const {
    bool skip = false;
    return skip;
}
bool ExplicitValidation::PreCallValidateCreateSwapchainKHR(VkDevice device,
                const VkSwapchainCreateInfoKHR* pCreateInfo,
                const VkAllocationCallbacks* pAllocator,
                VkSwapchainKHR* pSwapchain) const {
    bool skip = false;
    if (pCreateInfo != nullptr) {
        const auto _s183 = pCreateInfo;
        skip |= ValidateSwapchainCreateInfoKHR({},
            _s183->sType,
            _s183->pNext,
            _s183->flags,
            _s183->surface,
            _s183->minImageCount,
            _s183->imageFormat,
            _s183->imageColorSpace,
            _s183->imageExtent,
            _s183->imageArrayLayers,
            _s183->imageUsage,
            _s183->imageSharingMode,
            _s183->queueFamilyIndexCount,
            _s183->pQueueFamilyIndices,
            _s183->preTransform,
            _s183->compositeAlpha,
            _s183->presentMode,
            _s183->clipped,
            _s183->oldSwapchain);
    }
    if (pAllocator != nullptr) {
        const auto _s184 = pAllocator;
        skip |= ValidateAllocationCallbacks({},
            _s184->pUserData,
            _s184->pfnAllocation,
            _s184->pfnReallocation,
            _s184->pfnFree,
            _s184->pfnInternalAllocation,
            _s184->pfnInternalFree);
    }
    return skip;
}
bool ExplicitValidation::PreCallValidateDestroySwapchainKHR(VkDevice device,
                VkSwapchainKHR swapchain,
                const VkAllocationCallbacks* pAllocator) const {
    bool skip = false;
    if (pAllocator != nullptr) {
        const auto _s185 = pAllocator;
        skip |= ValidateAllocationCallbacks({},
            _s185->pUserData,
            _s185->pfnAllocation,
            _s185->pfnReallocation,
            _s185->pfnFree,
            _s185->pfnInternalAllocation,
            _s185->pfnInternalFree);
    }
    return skip;
}
bool ExplicitValidation::PreCallValidateGetSwapchainImagesKHR(VkDevice device,
                VkSwapchainKHR swapchain,
                uint32_t* pSwapchainImageCount,
                VkImage* pSwapchainImages) const {
    bool skip = false;
    return skip;
}
bool ExplicitValidation::PreCallValidateAcquireNextImageKHR(VkDevice device,
                VkSwapchainKHR swapchain,
                uint64_t timeout,
                VkSemaphore semaphore,
                VkFence fence,
                uint32_t* pImageIndex) const {
    bool skip = false;
    return skip;
}
bool ExplicitValidation::PreCallValidateQueuePresentKHR(VkQueue queue,
                const VkPresentInfoKHR* pPresentInfo) const {
    bool skip = false;
    if (pPresentInfo != nullptr) {
        const auto _s186 = pPresentInfo;
        skip |= ValidatePresentInfoKHR({},
            _s186->sType,
            _s186->pNext,
            _s186->waitSemaphoreCount,
            _s186->pWaitSemaphores,
            _s186->swapchainCount,
            _s186->pSwapchains,
            _s186->pImageIndices,
            _s186->pResults);
    }
    return skip;
}
bool ExplicitValidation::PreCallValidateGetDeviceGroupPresentCapabilitiesKHR(VkDevice device,
                VkDeviceGroupPresentCapabilitiesKHR* pDeviceGroupPresentCapabilities) const {
    bool skip = false;
    return skip;
}
bool ExplicitValidation::PreCallValidateGetDeviceGroupSurfacePresentModesKHR(VkDevice device,
                VkSurfaceKHR surface,
                VkDeviceGroupPresentModeFlagsKHR* pModes) const {
    bool skip = false;
    return skip;
}
bool ExplicitValidation::PreCallValidateGetPhysicalDevicePresentRectanglesKHR(VkPhysicalDevice physicalDevice,
                VkSurfaceKHR surface,
                uint32_t* pRectCount,
                VkRect2D* pRects) const {
    bool skip = false;
    if (pRects != nullptr) {
        for (uint32_t _i187 = 0;_i187 < *pRectCount; ++_i187) {
            const auto _s188 = &pRects[_i187];
            skip |= ValidateRect2D({},
                _s188->offset,
                _s188->extent);
        }
    }
    return skip;
}
bool ExplicitValidation::PreCallValidateAcquireNextImage2KHR(VkDevice device,
                const VkAcquireNextImageInfoKHR* pAcquireInfo,
                uint32_t* pImageIndex) const {
    bool skip = false;
    if (pAcquireInfo != nullptr) {
        const auto _s189 = pAcquireInfo;
        skip |= ValidateAcquireNextImageInfoKHR({},
            _s189->sType,
            _s189->pNext,
            _s189->swapchain,
            _s189->timeout,
            _s189->semaphore,
            _s189->fence,
            _s189->deviceMask);
    }
    return skip;
}
bool ExplicitValidation::PreCallValidateGetPhysicalDeviceDisplayPropertiesKHR(VkPhysicalDevice physicalDevice,
                uint32_t* pPropertyCount,
                VkDisplayPropertiesKHR* pProperties) const {
    bool skip = false;
    return skip;
}
bool ExplicitValidation::PreCallValidateGetPhysicalDeviceDisplayPlanePropertiesKHR(VkPhysicalDevice physicalDevice,
                uint32_t* pPropertyCount,
                VkDisplayPlanePropertiesKHR* pProperties) const {
    bool skip = false;
    return skip;
}
bool ExplicitValidation::PreCallValidateGetDisplayPlaneSupportedDisplaysKHR(VkPhysicalDevice physicalDevice,
                uint32_t planeIndex,
                uint32_t* pDisplayCount,
                VkDisplayKHR* pDisplays) const {
    bool skip = false;
    return skip;
}
bool ExplicitValidation::PreCallValidateGetDisplayModePropertiesKHR(VkPhysicalDevice physicalDevice,
                VkDisplayKHR display,
                uint32_t* pPropertyCount,
                VkDisplayModePropertiesKHR* pProperties) const {
    bool skip = false;
    return skip;
}
bool ExplicitValidation::PreCallValidateCreateDisplayModeKHR(VkPhysicalDevice physicalDevice,
                VkDisplayKHR display,
                const VkDisplayModeCreateInfoKHR* pCreateInfo,
                const VkAllocationCallbacks* pAllocator,
                VkDisplayModeKHR* pMode) const {
    bool skip = false;
    if (pCreateInfo != nullptr) {
        const auto _s190 = pCreateInfo;
        skip |= ValidateDisplayModeCreateInfoKHR({},
            _s190->sType,
            _s190->pNext,
            _s190->flags,
            _s190->parameters);
    }
    if (pAllocator != nullptr) {
        const auto _s191 = pAllocator;
        skip |= ValidateAllocationCallbacks({},
            _s191->pUserData,
            _s191->pfnAllocation,
            _s191->pfnReallocation,
            _s191->pfnFree,
            _s191->pfnInternalAllocation,
            _s191->pfnInternalFree);
    }
    return skip;
}
bool ExplicitValidation::PreCallValidateGetDisplayPlaneCapabilitiesKHR(VkPhysicalDevice physicalDevice,
                VkDisplayModeKHR mode,
                uint32_t planeIndex,
                VkDisplayPlaneCapabilitiesKHR* pCapabilities) const {
    bool skip = false;
    return skip;
}
bool ExplicitValidation::PreCallValidateCreateDisplayPlaneSurfaceKHR(VkInstance instance,
                const VkDisplaySurfaceCreateInfoKHR* pCreateInfo,
                const VkAllocationCallbacks* pAllocator,
                VkSurfaceKHR* pSurface) const {
    bool skip = false;
    if (pCreateInfo != nullptr) {
        const auto _s192 = pCreateInfo;
        skip |= ValidateDisplaySurfaceCreateInfoKHR({},
            _s192->sType,
            _s192->pNext,
            _s192->flags,
            _s192->displayMode,
            _s192->planeIndex,
            _s192->planeStackIndex,
            _s192->transform,
            _s192->globalAlpha,
            _s192->alphaMode,
            _s192->imageExtent);
    }
    if (pAllocator != nullptr) {
        const auto _s193 = pAllocator;
        skip |= ValidateAllocationCallbacks({},
            _s193->pUserData,
            _s193->pfnAllocation,
            _s193->pfnReallocation,
            _s193->pfnFree,
            _s193->pfnInternalAllocation,
            _s193->pfnInternalFree);
    }
    return skip;
}
bool ExplicitValidation::PreCallValidateCreateSharedSwapchainsKHR(VkDevice device,
                uint32_t swapchainCount,
                const VkSwapchainCreateInfoKHR* pCreateInfos,
                const VkAllocationCallbacks* pAllocator,
                VkSwapchainKHR* pSwapchains) const {
    bool skip = false;
    if (pCreateInfos != nullptr) {
        for (uint32_t _i194 = 0;_i194 < swapchainCount; ++_i194) {
            const auto _s195 = &pCreateInfos[_i194];
            skip |= ValidateSwapchainCreateInfoKHR({},
                _s195->sType,
                _s195->pNext,
                _s195->flags,
                _s195->surface,
                _s195->minImageCount,
                _s195->imageFormat,
                _s195->imageColorSpace,
                _s195->imageExtent,
                _s195->imageArrayLayers,
                _s195->imageUsage,
                _s195->imageSharingMode,
                _s195->queueFamilyIndexCount,
                _s195->pQueueFamilyIndices,
                _s195->preTransform,
                _s195->compositeAlpha,
                _s195->presentMode,
                _s195->clipped,
                _s195->oldSwapchain);
        }
    }
    if (pAllocator != nullptr) {
        const auto _s196 = pAllocator;
        skip |= ValidateAllocationCallbacks({},
            _s196->pUserData,
            _s196->pfnAllocation,
            _s196->pfnReallocation,
            _s196->pfnFree,
            _s196->pfnInternalAllocation,
            _s196->pfnInternalFree);
    }
    return skip;
}
#ifdef VK_USE_PLATFORM_XLIB_KHR
bool ExplicitValidation::PreCallValidateCreateXlibSurfaceKHR(VkInstance instance,
                const VkXlibSurfaceCreateInfoKHR* pCreateInfo,
                const VkAllocationCallbacks* pAllocator,
                VkSurfaceKHR* pSurface) const {
    bool skip = false;
    if (pCreateInfo != nullptr) {
        const auto _s197 = pCreateInfo;
        skip |= ValidateXlibSurfaceCreateInfoKHR({},
            _s197->sType,
            _s197->pNext,
            _s197->flags,
            _s197->dpy,
            _s197->window);
    }
    if (pAllocator != nullptr) {
        const auto _s198 = pAllocator;
        skip |= ValidateAllocationCallbacks({},
            _s198->pUserData,
            _s198->pfnAllocation,
            _s198->pfnReallocation,
            _s198->pfnFree,
            _s198->pfnInternalAllocation,
            _s198->pfnInternalFree);
    }
    return skip;
}
#endif
#ifdef VK_USE_PLATFORM_XLIB_KHR
bool ExplicitValidation::PreCallValidateGetPhysicalDeviceXlibPresentationSupportKHR(VkPhysicalDevice physicalDevice,
                uint32_t queueFamilyIndex,
                Display* dpy,
                VisualID visualID) const {
    bool skip = false;
    return skip;
}
#endif
#ifdef VK_USE_PLATFORM_XCB_KHR
bool ExplicitValidation::PreCallValidateCreateXcbSurfaceKHR(VkInstance instance,
                const VkXcbSurfaceCreateInfoKHR* pCreateInfo,
                const VkAllocationCallbacks* pAllocator,
                VkSurfaceKHR* pSurface) const {
    bool skip = false;
    if (pCreateInfo != nullptr) {
        const auto _s199 = pCreateInfo;
        skip |= ValidateXcbSurfaceCreateInfoKHR({},
            _s199->sType,
            _s199->pNext,
            _s199->flags,
            _s199->connection,
            _s199->window);
    }
    if (pAllocator != nullptr) {
        const auto _s200 = pAllocator;
        skip |= ValidateAllocationCallbacks({},
            _s200->pUserData,
            _s200->pfnAllocation,
            _s200->pfnReallocation,
            _s200->pfnFree,
            _s200->pfnInternalAllocation,
            _s200->pfnInternalFree);
    }
    return skip;
}
#endif
#ifdef VK_USE_PLATFORM_XCB_KHR
bool ExplicitValidation::PreCallValidateGetPhysicalDeviceXcbPresentationSupportKHR(VkPhysicalDevice physicalDevice,
                uint32_t queueFamilyIndex,
                xcb_connection_t* connection,
                xcb_visualid_t visual_id) const {
    bool skip = false;
    return skip;
}
#endif
#ifdef VK_USE_PLATFORM_WAYLAND_KHR
bool ExplicitValidation::PreCallValidateCreateWaylandSurfaceKHR(VkInstance instance,
                const VkWaylandSurfaceCreateInfoKHR* pCreateInfo,
                const VkAllocationCallbacks* pAllocator,
                VkSurfaceKHR* pSurface) const {
    bool skip = false;
    if (pCreateInfo != nullptr) {
        const auto _s201 = pCreateInfo;
        skip |= ValidateWaylandSurfaceCreateInfoKHR({},
            _s201->sType,
            _s201->pNext,
            _s201->flags,
            _s201->display,
            _s201->surface);
    }
    if (pAllocator != nullptr) {
        const auto _s202 = pAllocator;
        skip |= ValidateAllocationCallbacks({},
            _s202->pUserData,
            _s202->pfnAllocation,
            _s202->pfnReallocation,
            _s202->pfnFree,
            _s202->pfnInternalAllocation,
            _s202->pfnInternalFree);
    }
    return skip;
}
#endif
#ifdef VK_USE_PLATFORM_WAYLAND_KHR
bool ExplicitValidation::PreCallValidateGetPhysicalDeviceWaylandPresentationSupportKHR(VkPhysicalDevice physicalDevice,
                uint32_t queueFamilyIndex,
                struct wl_display* display) const {
    bool skip = false;
    return skip;
}
#endif
#ifdef VK_USE_PLATFORM_ANDROID_KHR
bool ExplicitValidation::PreCallValidateCreateAndroidSurfaceKHR(VkInstance instance,
                const VkAndroidSurfaceCreateInfoKHR* pCreateInfo,
                const VkAllocationCallbacks* pAllocator,
                VkSurfaceKHR* pSurface) const {
    bool skip = false;
    if (pCreateInfo != nullptr) {
        const auto _s203 = pCreateInfo;
        skip |= ValidateAndroidSurfaceCreateInfoKHR({},
            _s203->sType,
            _s203->pNext,
            _s203->flags,
            _s203->window);
    }
    if (pAllocator != nullptr) {
        const auto _s204 = pAllocator;
        skip |= ValidateAllocationCallbacks({},
            _s204->pUserData,
            _s204->pfnAllocation,
            _s204->pfnReallocation,
            _s204->pfnFree,
            _s204->pfnInternalAllocation,
            _s204->pfnInternalFree);
    }
    return skip;
}
#endif
#ifdef VK_USE_PLATFORM_WIN32_KHR
bool ExplicitValidation::PreCallValidateCreateWin32SurfaceKHR(VkInstance instance,
                const VkWin32SurfaceCreateInfoKHR* pCreateInfo,
                const VkAllocationCallbacks* pAllocator,
                VkSurfaceKHR* pSurface) const {
    bool skip = false;
    if (pCreateInfo != nullptr) {
        const auto _s205 = pCreateInfo;
        skip |= ValidateWin32SurfaceCreateInfoKHR({},
            _s205->sType,
            _s205->pNext,
            _s205->flags,
            _s205->hinstance,
            _s205->hwnd);
    }
    if (pAllocator != nullptr) {
        const auto _s206 = pAllocator;
        skip |= ValidateAllocationCallbacks({},
            _s206->pUserData,
            _s206->pfnAllocation,
            _s206->pfnReallocation,
            _s206->pfnFree,
            _s206->pfnInternalAllocation,
            _s206->pfnInternalFree);
    }
    return skip;
}
#endif
#ifdef VK_USE_PLATFORM_WIN32_KHR
bool ExplicitValidation::PreCallValidateGetPhysicalDeviceWin32PresentationSupportKHR(VkPhysicalDevice physicalDevice,
                uint32_t queueFamilyIndex) const {
    bool skip = false;
    return skip;
}
#endif
bool ExplicitValidation::PreCallValidateGetPhysicalDeviceVideoCapabilitiesKHR(VkPhysicalDevice physicalDevice,
                const VkVideoProfileInfoKHR* pVideoProfile,
                VkVideoCapabilitiesKHR* pCapabilities) const {
    bool skip = false;
    if (pVideoProfile != nullptr) {
        const auto _s207 = pVideoProfile;
        skip |= ValidateVideoProfileInfoKHR({},
            _s207->sType,
            _s207->pNext,
            _s207->videoCodecOperation,
            _s207->chromaSubsampling,
            _s207->lumaBitDepth,
            _s207->chromaBitDepth);
    }
    return skip;
}
bool ExplicitValidation::PreCallValidateGetPhysicalDeviceVideoFormatPropertiesKHR(VkPhysicalDevice physicalDevice,
                const VkPhysicalDeviceVideoFormatInfoKHR* pVideoFormatInfo,
                uint32_t* pVideoFormatPropertyCount,
                VkVideoFormatPropertiesKHR* pVideoFormatProperties) const {
    bool skip = false;
    if (pVideoFormatInfo != nullptr) {
        const auto _s208 = pVideoFormatInfo;
        skip |= ValidatePhysicalDeviceVideoFormatInfoKHR({},
            _s208->sType,
            _s208->pNext,
            _s208->imageUsage);
    }
    return skip;
}
bool ExplicitValidation::PreCallValidateCreateVideoSessionKHR(VkDevice device,
                const VkVideoSessionCreateInfoKHR* pCreateInfo,
                const VkAllocationCallbacks* pAllocator,
                VkVideoSessionKHR* pVideoSession) const {
    bool skip = false;
    if (pCreateInfo != nullptr) {
        const auto _s209 = pCreateInfo;
        skip |= ValidateVideoSessionCreateInfoKHR({},
            _s209->sType,
            _s209->pNext,
            _s209->queueFamilyIndex,
            _s209->flags,
            _s209->pVideoProfile,
            _s209->pictureFormat,
            _s209->maxCodedExtent,
            _s209->referencePictureFormat,
            _s209->maxDpbSlots,
            _s209->maxActiveReferencePictures,
            _s209->pStdHeaderVersion);
    }
    if (pAllocator != nullptr) {
        const auto _s210 = pAllocator;
        skip |= ValidateAllocationCallbacks({},
            _s210->pUserData,
            _s210->pfnAllocation,
            _s210->pfnReallocation,
            _s210->pfnFree,
            _s210->pfnInternalAllocation,
            _s210->pfnInternalFree);
    }
    return skip;
}
bool ExplicitValidation::PreCallValidateDestroyVideoSessionKHR(VkDevice device,
                VkVideoSessionKHR videoSession,
                const VkAllocationCallbacks* pAllocator) const {
    bool skip = false;
    if (pAllocator != nullptr) {
        const auto _s211 = pAllocator;
        skip |= ValidateAllocationCallbacks({},
            _s211->pUserData,
            _s211->pfnAllocation,
            _s211->pfnReallocation,
            _s211->pfnFree,
            _s211->pfnInternalAllocation,
            _s211->pfnInternalFree);
    }
    return skip;
}
bool ExplicitValidation::PreCallValidateGetVideoSessionMemoryRequirementsKHR(VkDevice device,
                VkVideoSessionKHR videoSession,
                uint32_t* pMemoryRequirementsCount,
                VkVideoSessionMemoryRequirementsKHR* pMemoryRequirements) const {
    bool skip = false;
    return skip;
}
bool ExplicitValidation::PreCallValidateBindVideoSessionMemoryKHR(VkDevice device,
                VkVideoSessionKHR videoSession,
                uint32_t bindSessionMemoryInfoCount,
                const VkBindVideoSessionMemoryInfoKHR* pBindSessionMemoryInfos) const {
    bool skip = false;
    if (pBindSessionMemoryInfos != nullptr) {
        for (uint32_t _i212 = 0;_i212 < bindSessionMemoryInfoCount; ++_i212) {
            const auto _s213 = &pBindSessionMemoryInfos[_i212];
            skip |= ValidateBindVideoSessionMemoryInfoKHR({},
                _s213->sType,
                _s213->pNext,
                _s213->memoryBindIndex,
                _s213->memory,
                _s213->memoryOffset,
                _s213->memorySize);
        }
    }
    return skip;
}
bool ExplicitValidation::PreCallValidateCreateVideoSessionParametersKHR(VkDevice device,
                const VkVideoSessionParametersCreateInfoKHR* pCreateInfo,
                const VkAllocationCallbacks* pAllocator,
                VkVideoSessionParametersKHR* pVideoSessionParameters) const {
    bool skip = false;
    if (pCreateInfo != nullptr) {
        const auto _s214 = pCreateInfo;
        skip |= ValidateVideoSessionParametersCreateInfoKHR({},
            _s214->sType,
            _s214->pNext,
            _s214->flags,
            _s214->videoSessionParametersTemplate,
            _s214->videoSession);
    }
    if (pAllocator != nullptr) {
        const auto _s215 = pAllocator;
        skip |= ValidateAllocationCallbacks({},
            _s215->pUserData,
            _s215->pfnAllocation,
            _s215->pfnReallocation,
            _s215->pfnFree,
            _s215->pfnInternalAllocation,
            _s215->pfnInternalFree);
    }
    return skip;
}
bool ExplicitValidation::PreCallValidateUpdateVideoSessionParametersKHR(VkDevice device,
                VkVideoSessionParametersKHR videoSessionParameters,
                const VkVideoSessionParametersUpdateInfoKHR* pUpdateInfo) const {
    bool skip = false;
    if (pUpdateInfo != nullptr) {
        const auto _s216 = pUpdateInfo;
        skip |= ValidateVideoSessionParametersUpdateInfoKHR({},
            _s216->sType,
            _s216->pNext,
            _s216->updateSequenceCount);
    }
    return skip;
}
bool ExplicitValidation::PreCallValidateDestroyVideoSessionParametersKHR(VkDevice device,
                VkVideoSessionParametersKHR videoSessionParameters,
                const VkAllocationCallbacks* pAllocator) const {
    bool skip = false;
    if (pAllocator != nullptr) {
        const auto _s217 = pAllocator;
        skip |= ValidateAllocationCallbacks({},
            _s217->pUserData,
            _s217->pfnAllocation,
            _s217->pfnReallocation,
            _s217->pfnFree,
            _s217->pfnInternalAllocation,
            _s217->pfnInternalFree);
    }
    return skip;
}
bool ExplicitValidation::PreCallValidateCmdBeginVideoCodingKHR(VkCommandBuffer commandBuffer,
                const VkVideoBeginCodingInfoKHR* pBeginInfo) const {
    bool skip = false;
    LogObjectList _carryOverObjects;
    _carryOverObjects.add(commandBuffer);
    if (pBeginInfo != nullptr) {
        const auto _s218 = pBeginInfo;
        skip |= ValidateVideoBeginCodingInfoKHR(_carryOverObjects,
            _s218->sType,
            _s218->pNext,
            _s218->flags,
            _s218->videoSession,
            _s218->videoSessionParameters,
            _s218->referenceSlotCount,
            _s218->pReferenceSlots);
    }
    return skip;
}
bool ExplicitValidation::PreCallValidateCmdEndVideoCodingKHR(VkCommandBuffer commandBuffer,
                const VkVideoEndCodingInfoKHR* pEndCodingInfo) const {
    bool skip = false;
    LogObjectList _carryOverObjects;
    _carryOverObjects.add(commandBuffer);
    if (pEndCodingInfo != nullptr) {
        const auto _s219 = pEndCodingInfo;
        skip |= ValidateVideoEndCodingInfoKHR(_carryOverObjects,
            _s219->sType,
            _s219->pNext,
            _s219->flags);
    }
    return skip;
}
bool ExplicitValidation::PreCallValidateCmdControlVideoCodingKHR(VkCommandBuffer commandBuffer,
                const VkVideoCodingControlInfoKHR* pCodingControlInfo) const {
    bool skip = false;
    LogObjectList _carryOverObjects;
    _carryOverObjects.add(commandBuffer);
    if (pCodingControlInfo != nullptr) {
        const auto _s220 = pCodingControlInfo;
        skip |= ValidateVideoCodingControlInfoKHR(_carryOverObjects,
            _s220->sType,
            _s220->pNext,
            _s220->flags);
    }
    return skip;
}
bool ExplicitValidation::PreCallValidateCmdDecodeVideoKHR(VkCommandBuffer commandBuffer,
                const VkVideoDecodeInfoKHR* pDecodeInfo) const {
    bool skip = false;
    LogObjectList _carryOverObjects;
    _carryOverObjects.add(commandBuffer);
    if (pDecodeInfo != nullptr) {
        const auto _s221 = pDecodeInfo;
        skip |= ValidateVideoDecodeInfoKHR(_carryOverObjects,
            _s221->sType,
            _s221->pNext,
            _s221->flags,
            _s221->srcBuffer,
            _s221->srcBufferOffset,
            _s221->srcBufferRange,
            _s221->dstPictureResource,
            _s221->pSetupReferenceSlot,
            _s221->referenceSlotCount,
            _s221->pReferenceSlots);
    }
    return skip;
}
bool ExplicitValidation::PreCallValidateCmdBeginRenderingKHR(VkCommandBuffer commandBuffer,
                const VkRenderingInfo* pRenderingInfo) const {
    return PreCallValidateCmdBeginRendering(commandBuffer, pRenderingInfo);
}
bool ExplicitValidation::PreCallValidateCmdEndRenderingKHR(VkCommandBuffer commandBuffer) const {
    return PreCallValidateCmdEndRendering(commandBuffer);
}
bool ExplicitValidation::PreCallValidateGetPhysicalDeviceFeatures2KHR(VkPhysicalDevice physicalDevice,
                VkPhysicalDeviceFeatures2* pFeatures) const {
    return PreCallValidateGetPhysicalDeviceFeatures2(physicalDevice, pFeatures);
}
bool ExplicitValidation::PreCallValidateGetPhysicalDeviceProperties2KHR(VkPhysicalDevice physicalDevice,
                VkPhysicalDeviceProperties2* pProperties) const {
    return PreCallValidateGetPhysicalDeviceProperties2(physicalDevice, pProperties);
}
bool ExplicitValidation::PreCallValidateGetPhysicalDeviceFormatProperties2KHR(VkPhysicalDevice physicalDevice,
                VkFormat format,
                VkFormatProperties2* pFormatProperties) const {
    return PreCallValidateGetPhysicalDeviceFormatProperties2(physicalDevice, format, pFormatProperties);
}
bool ExplicitValidation::PreCallValidateGetPhysicalDeviceImageFormatProperties2KHR(VkPhysicalDevice physicalDevice,
                const VkPhysicalDeviceImageFormatInfo2* pImageFormatInfo,
                VkImageFormatProperties2* pImageFormatProperties) const {
    return PreCallValidateGetPhysicalDeviceImageFormatProperties2(physicalDevice, pImageFormatInfo, pImageFormatProperties);
}
bool ExplicitValidation::PreCallValidateGetPhysicalDeviceQueueFamilyProperties2KHR(VkPhysicalDevice physicalDevice,
                uint32_t* pQueueFamilyPropertyCount,
                VkQueueFamilyProperties2* pQueueFamilyProperties) const {
    return PreCallValidateGetPhysicalDeviceQueueFamilyProperties2(physicalDevice, pQueueFamilyPropertyCount, pQueueFamilyProperties);
}
bool ExplicitValidation::PreCallValidateGetPhysicalDeviceMemoryProperties2KHR(VkPhysicalDevice physicalDevice,
                VkPhysicalDeviceMemoryProperties2* pMemoryProperties) const {
    return PreCallValidateGetPhysicalDeviceMemoryProperties2(physicalDevice, pMemoryProperties);
}
bool ExplicitValidation::PreCallValidateGetPhysicalDeviceSparseImageFormatProperties2KHR(VkPhysicalDevice physicalDevice,
                const VkPhysicalDeviceSparseImageFormatInfo2* pFormatInfo,
                uint32_t* pPropertyCount,
                VkSparseImageFormatProperties2* pProperties) const {
    return PreCallValidateGetPhysicalDeviceSparseImageFormatProperties2(physicalDevice, pFormatInfo, pPropertyCount, pProperties);
}
bool ExplicitValidation::PreCallValidateGetDeviceGroupPeerMemoryFeaturesKHR(VkDevice device,
                uint32_t heapIndex,
                uint32_t localDeviceIndex,
                uint32_t remoteDeviceIndex,
                VkPeerMemoryFeatureFlags* pPeerMemoryFeatures) const {
    return PreCallValidateGetDeviceGroupPeerMemoryFeatures(device, heapIndex, localDeviceIndex, remoteDeviceIndex, pPeerMemoryFeatures);
}
bool ExplicitValidation::PreCallValidateCmdSetDeviceMaskKHR(VkCommandBuffer commandBuffer,
                uint32_t deviceMask) const {
    return PreCallValidateCmdSetDeviceMask(commandBuffer, deviceMask);
}
bool ExplicitValidation::PreCallValidateCmdDispatchBaseKHR(VkCommandBuffer commandBuffer,
                uint32_t baseGroupX,
                uint32_t baseGroupY,
                uint32_t baseGroupZ,
                uint32_t groupCountX,
                uint32_t groupCountY,
                uint32_t groupCountZ) const {
    return PreCallValidateCmdDispatchBase(commandBuffer, baseGroupX, baseGroupY, baseGroupZ, groupCountX, groupCountY, groupCountZ);
}
bool ExplicitValidation::PreCallValidateTrimCommandPoolKHR(VkDevice device,
                VkCommandPool commandPool,
                VkCommandPoolTrimFlags flags) const {
    return PreCallValidateTrimCommandPool(device, commandPool, flags);
}
bool ExplicitValidation::PreCallValidateEnumeratePhysicalDeviceGroupsKHR(VkInstance instance,
                uint32_t* pPhysicalDeviceGroupCount,
                VkPhysicalDeviceGroupProperties* pPhysicalDeviceGroupProperties) const {
    return PreCallValidateEnumeratePhysicalDeviceGroups(instance, pPhysicalDeviceGroupCount, pPhysicalDeviceGroupProperties);
}
bool ExplicitValidation::PreCallValidateGetPhysicalDeviceExternalBufferPropertiesKHR(VkPhysicalDevice physicalDevice,
                const VkPhysicalDeviceExternalBufferInfo* pExternalBufferInfo,
                VkExternalBufferProperties* pExternalBufferProperties) const {
    return PreCallValidateGetPhysicalDeviceExternalBufferProperties(physicalDevice, pExternalBufferInfo, pExternalBufferProperties);
}
#ifdef VK_USE_PLATFORM_WIN32_KHR
bool ExplicitValidation::PreCallValidateGetMemoryWin32HandleKHR(VkDevice device,
                const VkMemoryGetWin32HandleInfoKHR* pGetWin32HandleInfo,
                HANDLE* pHandle) const {
    bool skip = false;
    if (pGetWin32HandleInfo != nullptr) {
        const auto _s222 = pGetWin32HandleInfo;
        skip |= ValidateMemoryGetWin32HandleInfoKHR({},
            _s222->sType,
            _s222->pNext,
            _s222->memory,
            _s222->handleType);
    }
    return skip;
}
#endif
#ifdef VK_USE_PLATFORM_WIN32_KHR
bool ExplicitValidation::PreCallValidateGetMemoryWin32HandlePropertiesKHR(VkDevice device,
                VkExternalMemoryHandleTypeFlagBits handleType,
                HANDLE handle,
                VkMemoryWin32HandlePropertiesKHR* pMemoryWin32HandleProperties) const {
    bool skip = false;
    return skip;
}
#endif
bool ExplicitValidation::PreCallValidateGetMemoryFdKHR(VkDevice device,
                const VkMemoryGetFdInfoKHR* pGetFdInfo,
                int* pFd) const {
    bool skip = false;
    if (pGetFdInfo != nullptr) {
        const auto _s223 = pGetFdInfo;
        skip |= ValidateMemoryGetFdInfoKHR({},
            _s223->sType,
            _s223->pNext,
            _s223->memory,
            _s223->handleType);
    }
    return skip;
}
bool ExplicitValidation::PreCallValidateGetMemoryFdPropertiesKHR(VkDevice device,
                VkExternalMemoryHandleTypeFlagBits handleType,
                int fd,
                VkMemoryFdPropertiesKHR* pMemoryFdProperties) const {
    bool skip = false;
    return skip;
}
bool ExplicitValidation::PreCallValidateGetPhysicalDeviceExternalSemaphorePropertiesKHR(VkPhysicalDevice physicalDevice,
                const VkPhysicalDeviceExternalSemaphoreInfo* pExternalSemaphoreInfo,
                VkExternalSemaphoreProperties* pExternalSemaphoreProperties) const {
    return PreCallValidateGetPhysicalDeviceExternalSemaphoreProperties(physicalDevice, pExternalSemaphoreInfo, pExternalSemaphoreProperties);
}
#ifdef VK_USE_PLATFORM_WIN32_KHR
bool ExplicitValidation::PreCallValidateImportSemaphoreWin32HandleKHR(VkDevice device,
                const VkImportSemaphoreWin32HandleInfoKHR* pImportSemaphoreWin32HandleInfo) const {
    bool skip = false;
    if (pImportSemaphoreWin32HandleInfo != nullptr) {
        const auto _s224 = pImportSemaphoreWin32HandleInfo;
        skip |= ValidateImportSemaphoreWin32HandleInfoKHR({},
            _s224->sType,
            _s224->pNext,
            _s224->semaphore,
            _s224->flags,
            _s224->handleType,
            _s224->handle,
            _s224->name);
    }
    return skip;
}
#endif
#ifdef VK_USE_PLATFORM_WIN32_KHR
bool ExplicitValidation::PreCallValidateGetSemaphoreWin32HandleKHR(VkDevice device,
                const VkSemaphoreGetWin32HandleInfoKHR* pGetWin32HandleInfo,
                HANDLE* pHandle) const {
    bool skip = false;
    if (pGetWin32HandleInfo != nullptr) {
        const auto _s225 = pGetWin32HandleInfo;
        skip |= ValidateSemaphoreGetWin32HandleInfoKHR({},
            _s225->sType,
            _s225->pNext,
            _s225->semaphore,
            _s225->handleType);
    }
    return skip;
}
#endif
bool ExplicitValidation::PreCallValidateImportSemaphoreFdKHR(VkDevice device,
                const VkImportSemaphoreFdInfoKHR* pImportSemaphoreFdInfo) const {
    bool skip = false;
    if (pImportSemaphoreFdInfo != nullptr) {
        const auto _s226 = pImportSemaphoreFdInfo;
        skip |= ValidateImportSemaphoreFdInfoKHR({},
            _s226->sType,
            _s226->pNext,
            _s226->semaphore,
            _s226->flags,
            _s226->handleType,
            _s226->fd);
    }
    return skip;
}
bool ExplicitValidation::PreCallValidateGetSemaphoreFdKHR(VkDevice device,
                const VkSemaphoreGetFdInfoKHR* pGetFdInfo,
                int* pFd) const {
    bool skip = false;
    if (pGetFdInfo != nullptr) {
        const auto _s227 = pGetFdInfo;
        skip |= ValidateSemaphoreGetFdInfoKHR({},
            _s227->sType,
            _s227->pNext,
            _s227->semaphore,
            _s227->handleType);
    }
    return skip;
}
bool ExplicitValidation::PreCallValidateCmdPushDescriptorSetKHR(VkCommandBuffer commandBuffer,
                VkPipelineBindPoint pipelineBindPoint,
                VkPipelineLayout layout,
                uint32_t set,
                uint32_t descriptorWriteCount,
                const VkWriteDescriptorSet* pDescriptorWrites) const {
    bool skip = false;
    LogObjectList _carryOverObjects;
    _carryOverObjects.add(commandBuffer);
    if (pDescriptorWrites != nullptr) {
        for (uint32_t _i228 = 0;_i228 < descriptorWriteCount; ++_i228) {
            const auto _s229 = &pDescriptorWrites[_i228];
            skip |= ValidateWriteDescriptorSet(_carryOverObjects,
                _s229->sType,
                _s229->pNext,
                _s229->dstSet,
                _s229->dstBinding,
                _s229->dstArrayElement,
                _s229->descriptorCount,
                _s229->descriptorType,
                _s229->pImageInfo,
                _s229->pBufferInfo,
                _s229->pTexelBufferView);
        }
    }
    return skip;
}
bool ExplicitValidation::PreCallValidateCmdPushDescriptorSetWithTemplateKHR(VkCommandBuffer commandBuffer,
                VkDescriptorUpdateTemplate descriptorUpdateTemplate,
                VkPipelineLayout layout,
                uint32_t set,
                const void* pData) const {
    bool skip = false;
    LogObjectList _carryOverObjects;
    _carryOverObjects.add(commandBuffer);
    return skip;
}
bool ExplicitValidation::PreCallValidateCreateDescriptorUpdateTemplateKHR(VkDevice device,
                const VkDescriptorUpdateTemplateCreateInfo* pCreateInfo,
                const VkAllocationCallbacks* pAllocator,
                VkDescriptorUpdateTemplate* pDescriptorUpdateTemplate) const {
    return PreCallValidateCreateDescriptorUpdateTemplate(device, pCreateInfo, pAllocator, pDescriptorUpdateTemplate);
}
bool ExplicitValidation::PreCallValidateDestroyDescriptorUpdateTemplateKHR(VkDevice device,
                VkDescriptorUpdateTemplate descriptorUpdateTemplate,
                const VkAllocationCallbacks* pAllocator) const {
    return PreCallValidateDestroyDescriptorUpdateTemplate(device, descriptorUpdateTemplate, pAllocator);
}
bool ExplicitValidation::PreCallValidateUpdateDescriptorSetWithTemplateKHR(VkDevice device,
                VkDescriptorSet descriptorSet,
                VkDescriptorUpdateTemplate descriptorUpdateTemplate,
                const void* pData) const {
    return PreCallValidateUpdateDescriptorSetWithTemplate(device, descriptorSet, descriptorUpdateTemplate, pData);
}
bool ExplicitValidation::PreCallValidateCreateRenderPass2KHR(VkDevice device,
                const VkRenderPassCreateInfo2* pCreateInfo,
                const VkAllocationCallbacks* pAllocator,
                VkRenderPass* pRenderPass) const {
    return PreCallValidateCreateRenderPass2(device, pCreateInfo, pAllocator, pRenderPass);
}
bool ExplicitValidation::PreCallValidateCmdBeginRenderPass2KHR(VkCommandBuffer commandBuffer,
                const VkRenderPassBeginInfo* pRenderPassBegin,
                const VkSubpassBeginInfo* pSubpassBeginInfo) const {
    return PreCallValidateCmdBeginRenderPass2(commandBuffer, pRenderPassBegin, pSubpassBeginInfo);
}
bool ExplicitValidation::PreCallValidateCmdNextSubpass2KHR(VkCommandBuffer commandBuffer,
                const VkSubpassBeginInfo* pSubpassBeginInfo,
                const VkSubpassEndInfo* pSubpassEndInfo) const {
    return PreCallValidateCmdNextSubpass2(commandBuffer, pSubpassBeginInfo, pSubpassEndInfo);
}
bool ExplicitValidation::PreCallValidateCmdEndRenderPass2KHR(VkCommandBuffer commandBuffer,
                const VkSubpassEndInfo* pSubpassEndInfo) const {
    return PreCallValidateCmdEndRenderPass2(commandBuffer, pSubpassEndInfo);
}
bool ExplicitValidation::PreCallValidateGetSwapchainStatusKHR(VkDevice device,
                VkSwapchainKHR swapchain) const {
    bool skip = false;
    return skip;
}
bool ExplicitValidation::PreCallValidateGetPhysicalDeviceExternalFencePropertiesKHR(VkPhysicalDevice physicalDevice,
                const VkPhysicalDeviceExternalFenceInfo* pExternalFenceInfo,
                VkExternalFenceProperties* pExternalFenceProperties) const {
    return PreCallValidateGetPhysicalDeviceExternalFenceProperties(physicalDevice, pExternalFenceInfo, pExternalFenceProperties);
}
#ifdef VK_USE_PLATFORM_WIN32_KHR
bool ExplicitValidation::PreCallValidateImportFenceWin32HandleKHR(VkDevice device,
                const VkImportFenceWin32HandleInfoKHR* pImportFenceWin32HandleInfo) const {
    bool skip = false;
    if (pImportFenceWin32HandleInfo != nullptr) {
        const auto _s230 = pImportFenceWin32HandleInfo;
        skip |= ValidateImportFenceWin32HandleInfoKHR({},
            _s230->sType,
            _s230->pNext,
            _s230->fence,
            _s230->flags,
            _s230->handleType,
            _s230->handle,
            _s230->name);
    }
    return skip;
}
#endif
#ifdef VK_USE_PLATFORM_WIN32_KHR
bool ExplicitValidation::PreCallValidateGetFenceWin32HandleKHR(VkDevice device,
                const VkFenceGetWin32HandleInfoKHR* pGetWin32HandleInfo,
                HANDLE* pHandle) const {
    bool skip = false;
    if (pGetWin32HandleInfo != nullptr) {
        const auto _s231 = pGetWin32HandleInfo;
        skip |= ValidateFenceGetWin32HandleInfoKHR({},
            _s231->sType,
            _s231->pNext,
            _s231->fence,
            _s231->handleType);
    }
    return skip;
}
#endif
bool ExplicitValidation::PreCallValidateImportFenceFdKHR(VkDevice device,
                const VkImportFenceFdInfoKHR* pImportFenceFdInfo) const {
    bool skip = false;
    if (pImportFenceFdInfo != nullptr) {
        const auto _s232 = pImportFenceFdInfo;
        skip |= ValidateImportFenceFdInfoKHR({},
            _s232->sType,
            _s232->pNext,
            _s232->fence,
            _s232->flags,
            _s232->handleType,
            _s232->fd);
    }
    return skip;
}
bool ExplicitValidation::PreCallValidateGetFenceFdKHR(VkDevice device,
                const VkFenceGetFdInfoKHR* pGetFdInfo,
                int* pFd) const {
    bool skip = false;
    if (pGetFdInfo != nullptr) {
        const auto _s233 = pGetFdInfo;
        skip |= ValidateFenceGetFdInfoKHR({},
            _s233->sType,
            _s233->pNext,
            _s233->fence,
            _s233->handleType);
    }
    return skip;
}
bool ExplicitValidation::PreCallValidateEnumeratePhysicalDeviceQueueFamilyPerformanceQueryCountersKHR(VkPhysicalDevice physicalDevice,
                uint32_t queueFamilyIndex,
                uint32_t* pCounterCount,
                VkPerformanceCounterKHR* pCounters,
                VkPerformanceCounterDescriptionKHR* pCounterDescriptions) const {
    bool skip = false;
    return skip;
}
bool ExplicitValidation::PreCallValidateGetPhysicalDeviceQueueFamilyPerformanceQueryPassesKHR(VkPhysicalDevice physicalDevice,
                const VkQueryPoolPerformanceCreateInfoKHR* pPerformanceQueryCreateInfo,
                uint32_t* pNumPasses) const {
    bool skip = false;
    if (pPerformanceQueryCreateInfo != nullptr) {
        const auto _s234 = pPerformanceQueryCreateInfo;
        skip |= ValidateQueryPoolPerformanceCreateInfoKHR({},
            _s234->sType,
            _s234->pNext,
            _s234->queueFamilyIndex,
            _s234->counterIndexCount,
            _s234->pCounterIndices);
    }
    return skip;
}
bool ExplicitValidation::PreCallValidateAcquireProfilingLockKHR(VkDevice device,
                const VkAcquireProfilingLockInfoKHR* pInfo) const {
    bool skip = false;
    if (pInfo != nullptr) {
        const auto _s235 = pInfo;
        skip |= ValidateAcquireProfilingLockInfoKHR({},
            _s235->sType,
            _s235->pNext,
            _s235->flags,
            _s235->timeout);
    }
    return skip;
}
bool ExplicitValidation::PreCallValidateReleaseProfilingLockKHR(VkDevice device) const {
    bool skip = false;
    return skip;
}
bool ExplicitValidation::PreCallValidateGetPhysicalDeviceSurfaceCapabilities2KHR(VkPhysicalDevice physicalDevice,
                const VkPhysicalDeviceSurfaceInfo2KHR* pSurfaceInfo,
                VkSurfaceCapabilities2KHR* pSurfaceCapabilities) const {
    bool skip = false;
    if (pSurfaceInfo != nullptr) {
        const auto _s236 = pSurfaceInfo;
        skip |= ValidatePhysicalDeviceSurfaceInfo2KHR({},
            _s236->sType,
            _s236->pNext,
            _s236->surface);
    }
    return skip;
}
bool ExplicitValidation::PreCallValidateGetPhysicalDeviceSurfaceFormats2KHR(VkPhysicalDevice physicalDevice,
                const VkPhysicalDeviceSurfaceInfo2KHR* pSurfaceInfo,
                uint32_t* pSurfaceFormatCount,
                VkSurfaceFormat2KHR* pSurfaceFormats) const {
    bool skip = false;
    if (pSurfaceInfo != nullptr) {
        const auto _s237 = pSurfaceInfo;
        skip |= ValidatePhysicalDeviceSurfaceInfo2KHR({},
            _s237->sType,
            _s237->pNext,
            _s237->surface);
    }
    return skip;
}
bool ExplicitValidation::PreCallValidateGetPhysicalDeviceDisplayProperties2KHR(VkPhysicalDevice physicalDevice,
                uint32_t* pPropertyCount,
                VkDisplayProperties2KHR* pProperties) const {
    bool skip = false;
    return skip;
}
bool ExplicitValidation::PreCallValidateGetPhysicalDeviceDisplayPlaneProperties2KHR(VkPhysicalDevice physicalDevice,
                uint32_t* pPropertyCount,
                VkDisplayPlaneProperties2KHR* pProperties) const {
    bool skip = false;
    return skip;
}
bool ExplicitValidation::PreCallValidateGetDisplayModeProperties2KHR(VkPhysicalDevice physicalDevice,
                VkDisplayKHR display,
                uint32_t* pPropertyCount,
                VkDisplayModeProperties2KHR* pProperties) const {
    bool skip = false;
    return skip;
}
bool ExplicitValidation::PreCallValidateGetDisplayPlaneCapabilities2KHR(VkPhysicalDevice physicalDevice,
                const VkDisplayPlaneInfo2KHR* pDisplayPlaneInfo,
                VkDisplayPlaneCapabilities2KHR* pCapabilities) const {
    bool skip = false;
    if (pDisplayPlaneInfo != nullptr) {
        const auto _s238 = pDisplayPlaneInfo;
        skip |= ValidateDisplayPlaneInfo2KHR({},
            _s238->sType,
            _s238->pNext,
            _s238->mode,
            _s238->planeIndex);
    }
    return skip;
}
bool ExplicitValidation::PreCallValidateGetImageMemoryRequirements2KHR(VkDevice device,
                const VkImageMemoryRequirementsInfo2* pInfo,
                VkMemoryRequirements2* pMemoryRequirements) const {
    return PreCallValidateGetImageMemoryRequirements2(device, pInfo, pMemoryRequirements);
}
bool ExplicitValidation::PreCallValidateGetBufferMemoryRequirements2KHR(VkDevice device,
                const VkBufferMemoryRequirementsInfo2* pInfo,
                VkMemoryRequirements2* pMemoryRequirements) const {
    return PreCallValidateGetBufferMemoryRequirements2(device, pInfo, pMemoryRequirements);
}
bool ExplicitValidation::PreCallValidateGetImageSparseMemoryRequirements2KHR(VkDevice device,
                const VkImageSparseMemoryRequirementsInfo2* pInfo,
                uint32_t* pSparseMemoryRequirementCount,
                VkSparseImageMemoryRequirements2* pSparseMemoryRequirements) const {
    return PreCallValidateGetImageSparseMemoryRequirements2(device, pInfo, pSparseMemoryRequirementCount, pSparseMemoryRequirements);
}
bool ExplicitValidation::PreCallValidateCreateSamplerYcbcrConversionKHR(VkDevice device,
                const VkSamplerYcbcrConversionCreateInfo* pCreateInfo,
                const VkAllocationCallbacks* pAllocator,
                VkSamplerYcbcrConversion* pYcbcrConversion) const {
    return PreCallValidateCreateSamplerYcbcrConversion(device, pCreateInfo, pAllocator, pYcbcrConversion);
}
bool ExplicitValidation::PreCallValidateDestroySamplerYcbcrConversionKHR(VkDevice device,
                VkSamplerYcbcrConversion ycbcrConversion,
                const VkAllocationCallbacks* pAllocator) const {
    return PreCallValidateDestroySamplerYcbcrConversion(device, ycbcrConversion, pAllocator);
}
bool ExplicitValidation::PreCallValidateBindBufferMemory2KHR(VkDevice device,
                uint32_t bindInfoCount,
                const VkBindBufferMemoryInfo* pBindInfos) const {
    return PreCallValidateBindBufferMemory2(device, bindInfoCount, pBindInfos);
}
bool ExplicitValidation::PreCallValidateBindImageMemory2KHR(VkDevice device,
                uint32_t bindInfoCount,
                const VkBindImageMemoryInfo* pBindInfos) const {
    return PreCallValidateBindImageMemory2(device, bindInfoCount, pBindInfos);
}
bool ExplicitValidation::PreCallValidateGetDescriptorSetLayoutSupportKHR(VkDevice device,
                const VkDescriptorSetLayoutCreateInfo* pCreateInfo,
                VkDescriptorSetLayoutSupport* pSupport) const {
    return PreCallValidateGetDescriptorSetLayoutSupport(device, pCreateInfo, pSupport);
}
bool ExplicitValidation::PreCallValidateCmdDrawIndirectCountKHR(VkCommandBuffer commandBuffer,
                VkBuffer buffer,
                VkDeviceSize offset,
                VkBuffer countBuffer,
                VkDeviceSize countBufferOffset,
                uint32_t maxDrawCount,
                uint32_t stride) const {
    return PreCallValidateCmdDrawIndirectCount(commandBuffer, buffer, offset, countBuffer, countBufferOffset, maxDrawCount, stride);
}
bool ExplicitValidation::PreCallValidateCmdDrawIndexedIndirectCountKHR(VkCommandBuffer commandBuffer,
                VkBuffer buffer,
                VkDeviceSize offset,
                VkBuffer countBuffer,
                VkDeviceSize countBufferOffset,
                uint32_t maxDrawCount,
                uint32_t stride) const {
    return PreCallValidateCmdDrawIndexedIndirectCount(commandBuffer, buffer, offset, countBuffer, countBufferOffset, maxDrawCount, stride);
}
bool ExplicitValidation::PreCallValidateGetSemaphoreCounterValueKHR(VkDevice device,
                VkSemaphore semaphore,
                uint64_t* pValue) const {
    return PreCallValidateGetSemaphoreCounterValue(device, semaphore, pValue);
}
bool ExplicitValidation::PreCallValidateWaitSemaphoresKHR(VkDevice device,
                const VkSemaphoreWaitInfo* pWaitInfo,
                uint64_t timeout) const {
    return PreCallValidateWaitSemaphores(device, pWaitInfo, timeout);
}
bool ExplicitValidation::PreCallValidateSignalSemaphoreKHR(VkDevice device,
                const VkSemaphoreSignalInfo* pSignalInfo) const {
    return PreCallValidateSignalSemaphore(device, pSignalInfo);
}
bool ExplicitValidation::PreCallValidateGetPhysicalDeviceFragmentShadingRatesKHR(VkPhysicalDevice physicalDevice,
                uint32_t* pFragmentShadingRateCount,
                VkPhysicalDeviceFragmentShadingRateKHR* pFragmentShadingRates) const {
    bool skip = false;
    return skip;
}
bool ExplicitValidation::PreCallValidateCmdSetFragmentShadingRateKHR(VkCommandBuffer commandBuffer,
                const VkExtent2D* pFragmentSize,
                const VkFragmentShadingRateCombinerOpKHR combinerOps[2]) const {
    bool skip = false;
    LogObjectList _carryOverObjects;
    _carryOverObjects.add(commandBuffer);
    if (pFragmentSize != nullptr) {
        const auto _s239 = pFragmentSize;
        skip |= ValidateExtent2D(_carryOverObjects,
            _s239->width,
            _s239->height);
    }
    return skip;
}
bool ExplicitValidation::PreCallValidateWaitForPresentKHR(VkDevice device,
                VkSwapchainKHR swapchain,
                uint64_t presentId,
                uint64_t timeout) const {
    bool skip = false;
    return skip;
}
bool ExplicitValidation::PreCallValidateGetBufferDeviceAddressKHR(VkDevice device,
                const VkBufferDeviceAddressInfo* pInfo) const {
    return PreCallValidateGetBufferDeviceAddress(device, pInfo);
}
bool ExplicitValidation::PreCallValidateGetBufferOpaqueCaptureAddressKHR(VkDevice device,
                const VkBufferDeviceAddressInfo* pInfo) const {
    return PreCallValidateGetBufferOpaqueCaptureAddress(device, pInfo);
}
bool ExplicitValidation::PreCallValidateGetDeviceMemoryOpaqueCaptureAddressKHR(VkDevice device,
                const VkDeviceMemoryOpaqueCaptureAddressInfo* pInfo) const {
    return PreCallValidateGetDeviceMemoryOpaqueCaptureAddress(device, pInfo);
}
bool ExplicitValidation::PreCallValidateCreateDeferredOperationKHR(VkDevice device,
                const VkAllocationCallbacks* pAllocator,
                VkDeferredOperationKHR* pDeferredOperation) const {
    bool skip = false;
    if (pAllocator != nullptr) {
        const auto _s240 = pAllocator;
        skip |= ValidateAllocationCallbacks({},
            _s240->pUserData,
            _s240->pfnAllocation,
            _s240->pfnReallocation,
            _s240->pfnFree,
            _s240->pfnInternalAllocation,
            _s240->pfnInternalFree);
    }
    return skip;
}
bool ExplicitValidation::PreCallValidateDestroyDeferredOperationKHR(VkDevice device,
                VkDeferredOperationKHR operation,
                const VkAllocationCallbacks* pAllocator) const {
    bool skip = false;
    if (pAllocator != nullptr) {
        const auto _s241 = pAllocator;
        skip |= ValidateAllocationCallbacks({},
            _s241->pUserData,
            _s241->pfnAllocation,
            _s241->pfnReallocation,
            _s241->pfnFree,
            _s241->pfnInternalAllocation,
            _s241->pfnInternalFree);
    }
    return skip;
}
bool ExplicitValidation::PreCallValidateGetDeferredOperationMaxConcurrencyKHR(VkDevice device,
                VkDeferredOperationKHR operation) const {
    bool skip = false;
    return skip;
}
bool ExplicitValidation::PreCallValidateGetDeferredOperationResultKHR(VkDevice device,
                VkDeferredOperationKHR operation) const {
    bool skip = false;
    return skip;
}
bool ExplicitValidation::PreCallValidateDeferredOperationJoinKHR(VkDevice device,
                VkDeferredOperationKHR operation) const {
    bool skip = false;
    return skip;
}
bool ExplicitValidation::PreCallValidateGetPipelineExecutablePropertiesKHR(VkDevice device,
                const VkPipelineInfoKHR* pPipelineInfo,
                uint32_t* pExecutableCount,
                VkPipelineExecutablePropertiesKHR* pProperties) const {
    bool skip = false;
    if (pPipelineInfo != nullptr) {
        const auto _s242 = pPipelineInfo;
        skip |= ValidatePipelineInfoKHR({},
            _s242->sType,
            _s242->pNext,
            _s242->pipeline);
    }
    return skip;
}
bool ExplicitValidation::PreCallValidateGetPipelineExecutableStatisticsKHR(VkDevice device,
                const VkPipelineExecutableInfoKHR* pExecutableInfo,
                uint32_t* pStatisticCount,
                VkPipelineExecutableStatisticKHR* pStatistics) const {
    bool skip = false;
    if (pExecutableInfo != nullptr) {
        const auto _s243 = pExecutableInfo;
        skip |= ValidatePipelineExecutableInfoKHR({},
            _s243->sType,
            _s243->pNext,
            _s243->pipeline,
            _s243->executableIndex);
    }
    return skip;
}
bool ExplicitValidation::PreCallValidateGetPipelineExecutableInternalRepresentationsKHR(VkDevice device,
                const VkPipelineExecutableInfoKHR* pExecutableInfo,
                uint32_t* pInternalRepresentationCount,
                VkPipelineExecutableInternalRepresentationKHR* pInternalRepresentations) const {
    bool skip = false;
    if (pExecutableInfo != nullptr) {
        const auto _s244 = pExecutableInfo;
        skip |= ValidatePipelineExecutableInfoKHR({},
            _s244->sType,
            _s244->pNext,
            _s244->pipeline,
            _s244->executableIndex);
    }
    return skip;
}
bool ExplicitValidation::PreCallValidateMapMemory2KHR(VkDevice device,
                const VkMemoryMapInfoKHR* pMemoryMapInfo,
                void** ppData) const {
    bool skip = false;
    if (pMemoryMapInfo != nullptr) {
        const auto _s245 = pMemoryMapInfo;
        skip |= ValidateMemoryMapInfoKHR({},
            _s245->sType,
            _s245->pNext,
            _s245->flags,
            _s245->memory,
            _s245->offset,
            _s245->size);
    }
    return skip;
}
bool ExplicitValidation::PreCallValidateUnmapMemory2KHR(VkDevice device,
                const VkMemoryUnmapInfoKHR* pMemoryUnmapInfo) const {
    bool skip = false;
    if (pMemoryUnmapInfo != nullptr) {
        const auto _s246 = pMemoryUnmapInfo;
        skip |= ValidateMemoryUnmapInfoKHR({},
            _s246->sType,
            _s246->pNext,
            _s246->flags,
            _s246->memory);
    }
    return skip;
}
#ifdef VK_ENABLE_BETA_EXTENSIONS
bool ExplicitValidation::PreCallValidateCmdEncodeVideoKHR(VkCommandBuffer commandBuffer,
                const VkVideoEncodeInfoKHR* pEncodeInfo) const {
    bool skip = false;
    LogObjectList _carryOverObjects;
    _carryOverObjects.add(commandBuffer);
    if (pEncodeInfo != nullptr) {
        const auto _s247 = pEncodeInfo;
        skip |= ValidateVideoEncodeInfoKHR(_carryOverObjects,
            _s247->sType,
            _s247->pNext,
            _s247->flags,
            _s247->qualityLevel,
            _s247->dstBuffer,
            _s247->dstBufferOffset,
            _s247->dstBufferRange,
            _s247->srcPictureResource,
            _s247->pSetupReferenceSlot,
            _s247->referenceSlotCount,
            _s247->pReferenceSlots,
            _s247->precedingExternallyEncodedBytes);
    }
    return skip;
}
#endif
bool ExplicitValidation::PreCallValidateCmdSetEvent2KHR(VkCommandBuffer commandBuffer,
                VkEvent event,
                const VkDependencyInfo* pDependencyInfo) const {
    return PreCallValidateCmdSetEvent2(commandBuffer, event, pDependencyInfo);
}
bool ExplicitValidation::PreCallValidateCmdResetEvent2KHR(VkCommandBuffer commandBuffer,
                VkEvent event,
                VkPipelineStageFlags2 stageMask) const {
    return PreCallValidateCmdResetEvent2(commandBuffer, event, stageMask);
}
bool ExplicitValidation::PreCallValidateCmdWaitEvents2KHR(VkCommandBuffer commandBuffer,
                uint32_t eventCount,
                const VkEvent* pEvents,
                const VkDependencyInfo* pDependencyInfos) const {
    return PreCallValidateCmdWaitEvents2(commandBuffer, eventCount, pEvents, pDependencyInfos);
}
bool ExplicitValidation::PreCallValidateCmdPipelineBarrier2KHR(VkCommandBuffer commandBuffer,
                const VkDependencyInfo* pDependencyInfo) const {
    return PreCallValidateCmdPipelineBarrier2(commandBuffer, pDependencyInfo);
}
bool ExplicitValidation::PreCallValidateCmdWriteTimestamp2KHR(VkCommandBuffer commandBuffer,
                VkPipelineStageFlags2 stage,
                VkQueryPool queryPool,
                uint32_t query) const {
    return PreCallValidateCmdWriteTimestamp2(commandBuffer, stage, queryPool, query);
}
bool ExplicitValidation::PreCallValidateQueueSubmit2KHR(VkQueue queue,
                uint32_t submitCount,
                const VkSubmitInfo2* pSubmits,
                VkFence fence) const {
    return PreCallValidateQueueSubmit2(queue, submitCount, pSubmits, fence);
}
bool ExplicitValidation::PreCallValidateCmdWriteBufferMarker2AMD(VkCommandBuffer commandBuffer,
                VkPipelineStageFlags2 stage,
                VkBuffer dstBuffer,
                VkDeviceSize dstOffset,
                uint32_t marker) const {
    bool skip = false;
    LogObjectList _carryOverObjects;
    _carryOverObjects.add(commandBuffer);
    return skip;
}
bool ExplicitValidation::PreCallValidateGetQueueCheckpointData2NV(VkQueue queue,
                uint32_t* pCheckpointDataCount,
                VkCheckpointData2NV* pCheckpointData) const {
    bool skip = false;
    return skip;
}
bool ExplicitValidation::PreCallValidateCmdCopyBuffer2KHR(VkCommandBuffer commandBuffer,
                const VkCopyBufferInfo2* pCopyBufferInfo) const {
    return PreCallValidateCmdCopyBuffer2(commandBuffer, pCopyBufferInfo);
}
bool ExplicitValidation::PreCallValidateCmdCopyImage2KHR(VkCommandBuffer commandBuffer,
                const VkCopyImageInfo2* pCopyImageInfo) const {
    return PreCallValidateCmdCopyImage2(commandBuffer, pCopyImageInfo);
}
bool ExplicitValidation::PreCallValidateCmdCopyBufferToImage2KHR(VkCommandBuffer commandBuffer,
                const VkCopyBufferToImageInfo2* pCopyBufferToImageInfo) const {
    return PreCallValidateCmdCopyBufferToImage2(commandBuffer, pCopyBufferToImageInfo);
}
bool ExplicitValidation::PreCallValidateCmdCopyImageToBuffer2KHR(VkCommandBuffer commandBuffer,
                const VkCopyImageToBufferInfo2* pCopyImageToBufferInfo) const {
    return PreCallValidateCmdCopyImageToBuffer2(commandBuffer, pCopyImageToBufferInfo);
}
bool ExplicitValidation::PreCallValidateCmdBlitImage2KHR(VkCommandBuffer commandBuffer,
                const VkBlitImageInfo2* pBlitImageInfo) const {
    return PreCallValidateCmdBlitImage2(commandBuffer, pBlitImageInfo);
}
bool ExplicitValidation::PreCallValidateCmdResolveImage2KHR(VkCommandBuffer commandBuffer,
                const VkResolveImageInfo2* pResolveImageInfo) const {
    return PreCallValidateCmdResolveImage2(commandBuffer, pResolveImageInfo);
}
bool ExplicitValidation::PreCallValidateCmdTraceRaysIndirect2KHR(VkCommandBuffer commandBuffer,
                VkDeviceAddress indirectDeviceAddress) const {
    bool skip = false;
    LogObjectList _carryOverObjects;
    _carryOverObjects.add(commandBuffer);
    return skip;
}
bool ExplicitValidation::PreCallValidateGetDeviceBufferMemoryRequirementsKHR(VkDevice device,
                const VkDeviceBufferMemoryRequirements* pInfo,
                VkMemoryRequirements2* pMemoryRequirements) const {
    return PreCallValidateGetDeviceBufferMemoryRequirements(device, pInfo, pMemoryRequirements);
}
bool ExplicitValidation::PreCallValidateGetDeviceImageMemoryRequirementsKHR(VkDevice device,
                const VkDeviceImageMemoryRequirements* pInfo,
                VkMemoryRequirements2* pMemoryRequirements) const {
    return PreCallValidateGetDeviceImageMemoryRequirements(device, pInfo, pMemoryRequirements);
}
bool ExplicitValidation::PreCallValidateGetDeviceImageSparseMemoryRequirementsKHR(VkDevice device,
                const VkDeviceImageMemoryRequirements* pInfo,
                uint32_t* pSparseMemoryRequirementCount,
                VkSparseImageMemoryRequirements2* pSparseMemoryRequirements) const {
    return PreCallValidateGetDeviceImageSparseMemoryRequirements(device, pInfo, pSparseMemoryRequirementCount, pSparseMemoryRequirements);
}
bool ExplicitValidation::PreCallValidateCreateDebugReportCallbackEXT(VkInstance instance,
                const VkDebugReportCallbackCreateInfoEXT* pCreateInfo,
                const VkAllocationCallbacks* pAllocator,
                VkDebugReportCallbackEXT* pCallback) const {
    bool skip = false;
    if (pCreateInfo != nullptr) {
        const auto _s248 = pCreateInfo;
        skip |= ValidateDebugReportCallbackCreateInfoEXT({},
            _s248->sType,
            _s248->pNext,
            _s248->flags,
            _s248->pfnCallback,
            _s248->pUserData);
    }
    if (pAllocator != nullptr) {
        const auto _s249 = pAllocator;
        skip |= ValidateAllocationCallbacks({},
            _s249->pUserData,
            _s249->pfnAllocation,
            _s249->pfnReallocation,
            _s249->pfnFree,
            _s249->pfnInternalAllocation,
            _s249->pfnInternalFree);
    }
    return skip;
}
bool ExplicitValidation::PreCallValidateDestroyDebugReportCallbackEXT(VkInstance instance,
                VkDebugReportCallbackEXT callback,
                const VkAllocationCallbacks* pAllocator) const {
    bool skip = false;
    if (pAllocator != nullptr) {
        const auto _s250 = pAllocator;
        skip |= ValidateAllocationCallbacks({},
            _s250->pUserData,
            _s250->pfnAllocation,
            _s250->pfnReallocation,
            _s250->pfnFree,
            _s250->pfnInternalAllocation,
            _s250->pfnInternalFree);
    }
    return skip;
}
bool ExplicitValidation::PreCallValidateDebugReportMessageEXT(VkInstance instance,
                VkDebugReportFlagsEXT flags,
                VkDebugReportObjectTypeEXT objectType,
                uint64_t object,
                size_t location,
                int32_t messageCode,
                const char* pLayerPrefix,
                const char* pMessage) const {
    bool skip = false;
    return skip;
}
bool ExplicitValidation::PreCallValidateDebugMarkerSetObjectTagEXT(VkDevice device,
                const VkDebugMarkerObjectTagInfoEXT* pTagInfo) const {
    bool skip = false;
    if (pTagInfo != nullptr) {
        const auto _s251 = pTagInfo;
        skip |= ValidateDebugMarkerObjectTagInfoEXT({},
            _s251->sType,
            _s251->pNext,
            _s251->objectType,
            _s251->object,
            _s251->tagName,
            _s251->tagSize,
            _s251->pTag);
    }
    return skip;
}
bool ExplicitValidation::PreCallValidateDebugMarkerSetObjectNameEXT(VkDevice device,
                const VkDebugMarkerObjectNameInfoEXT* pNameInfo) const {
    bool skip = false;
    if (pNameInfo != nullptr) {
        const auto _s252 = pNameInfo;
        skip |= ValidateDebugMarkerObjectNameInfoEXT({},
            _s252->sType,
            _s252->pNext,
            _s252->objectType,
            _s252->object,
            _s252->pObjectName);
    }
    return skip;
}
bool ExplicitValidation::PreCallValidateCmdDebugMarkerBeginEXT(VkCommandBuffer commandBuffer,
                const VkDebugMarkerMarkerInfoEXT* pMarkerInfo) const {
    bool skip = false;
    LogObjectList _carryOverObjects;
    _carryOverObjects.add(commandBuffer);
    if (pMarkerInfo != nullptr) {
        const auto _s253 = pMarkerInfo;
        skip |= ValidateDebugMarkerMarkerInfoEXT(_carryOverObjects,
            _s253->sType,
            _s253->pNext,
            _s253->pMarkerName,
            _s253->color);
    }
    return skip;
}
bool ExplicitValidation::PreCallValidateCmdDebugMarkerEndEXT(VkCommandBuffer commandBuffer) const {
    bool skip = false;
    LogObjectList _carryOverObjects;
    _carryOverObjects.add(commandBuffer);
    return skip;
}
bool ExplicitValidation::PreCallValidateCmdDebugMarkerInsertEXT(VkCommandBuffer commandBuffer,
                const VkDebugMarkerMarkerInfoEXT* pMarkerInfo) const {
    bool skip = false;
    LogObjectList _carryOverObjects;
    _carryOverObjects.add(commandBuffer);
    if (pMarkerInfo != nullptr) {
        const auto _s254 = pMarkerInfo;
        skip |= ValidateDebugMarkerMarkerInfoEXT(_carryOverObjects,
            _s254->sType,
            _s254->pNext,
            _s254->pMarkerName,
            _s254->color);
    }
    return skip;
}
bool ExplicitValidation::PreCallValidateCmdBindTransformFeedbackBuffersEXT(VkCommandBuffer commandBuffer,
                uint32_t firstBinding,
                uint32_t bindingCount,
                const VkBuffer* pBuffers,
                const VkDeviceSize* pOffsets,
                const VkDeviceSize* pSizes) const {
    bool skip = false;
    LogObjectList _carryOverObjects;
    _carryOverObjects.add(commandBuffer);
    return skip;
}
bool ExplicitValidation::PreCallValidateCmdBeginTransformFeedbackEXT(VkCommandBuffer commandBuffer,
                uint32_t firstCounterBuffer,
                uint32_t counterBufferCount,
                const VkBuffer* pCounterBuffers,
                const VkDeviceSize* pCounterBufferOffsets) const {
    bool skip = false;
    LogObjectList _carryOverObjects;
    _carryOverObjects.add(commandBuffer);
    return skip;
}
bool ExplicitValidation::PreCallValidateCmdEndTransformFeedbackEXT(VkCommandBuffer commandBuffer,
                uint32_t firstCounterBuffer,
                uint32_t counterBufferCount,
                const VkBuffer* pCounterBuffers,
                const VkDeviceSize* pCounterBufferOffsets) const {
    bool skip = false;
    LogObjectList _carryOverObjects;
    _carryOverObjects.add(commandBuffer);
    return skip;
}
bool ExplicitValidation::PreCallValidateCmdBeginQueryIndexedEXT(VkCommandBuffer commandBuffer,
                VkQueryPool queryPool,
                uint32_t query,
                VkQueryControlFlags flags,
                uint32_t index) const {
    bool skip = false;
    LogObjectList _carryOverObjects;
    _carryOverObjects.add(commandBuffer);
    return skip;
}
bool ExplicitValidation::PreCallValidateCmdEndQueryIndexedEXT(VkCommandBuffer commandBuffer,
                VkQueryPool queryPool,
                uint32_t query,
                uint32_t index) const {
    bool skip = false;
    LogObjectList _carryOverObjects;
    _carryOverObjects.add(commandBuffer);
    return skip;
}
bool ExplicitValidation::PreCallValidateCmdDrawIndirectByteCountEXT(VkCommandBuffer commandBuffer,
                uint32_t instanceCount,
                uint32_t firstInstance,
                VkBuffer counterBuffer,
                VkDeviceSize counterBufferOffset,
                uint32_t counterOffset,
                uint32_t vertexStride) const {
    bool skip = false;
    LogObjectList _carryOverObjects;
    _carryOverObjects.add(commandBuffer);
    return skip;
}
bool ExplicitValidation::PreCallValidateCreateCuModuleNVX(VkDevice device,
                const VkCuModuleCreateInfoNVX* pCreateInfo,
                const VkAllocationCallbacks* pAllocator,
                VkCuModuleNVX* pModule) const {
    bool skip = false;
    if (pCreateInfo != nullptr) {
        const auto _s255 = pCreateInfo;
        skip |= ValidateCuModuleCreateInfoNVX({},
            _s255->sType,
            _s255->pNext,
            _s255->dataSize,
            _s255->pData);
    }
    if (pAllocator != nullptr) {
        const auto _s256 = pAllocator;
        skip |= ValidateAllocationCallbacks({},
            _s256->pUserData,
            _s256->pfnAllocation,
            _s256->pfnReallocation,
            _s256->pfnFree,
            _s256->pfnInternalAllocation,
            _s256->pfnInternalFree);
    }
    return skip;
}
bool ExplicitValidation::PreCallValidateCreateCuFunctionNVX(VkDevice device,
                const VkCuFunctionCreateInfoNVX* pCreateInfo,
                const VkAllocationCallbacks* pAllocator,
                VkCuFunctionNVX* pFunction) const {
    bool skip = false;
    if (pCreateInfo != nullptr) {
        const auto _s257 = pCreateInfo;
        skip |= ValidateCuFunctionCreateInfoNVX({},
            _s257->sType,
            _s257->pNext,
            _s257->module,
            _s257->pName);
    }
    if (pAllocator != nullptr) {
        const auto _s258 = pAllocator;
        skip |= ValidateAllocationCallbacks({},
            _s258->pUserData,
            _s258->pfnAllocation,
            _s258->pfnReallocation,
            _s258->pfnFree,
            _s258->pfnInternalAllocation,
            _s258->pfnInternalFree);
    }
    return skip;
}
bool ExplicitValidation::PreCallValidateDestroyCuModuleNVX(VkDevice device,
                VkCuModuleNVX module,
                const VkAllocationCallbacks* pAllocator) const {
    bool skip = false;
    if (pAllocator != nullptr) {
        const auto _s259 = pAllocator;
        skip |= ValidateAllocationCallbacks({},
            _s259->pUserData,
            _s259->pfnAllocation,
            _s259->pfnReallocation,
            _s259->pfnFree,
            _s259->pfnInternalAllocation,
            _s259->pfnInternalFree);
    }
    return skip;
}
bool ExplicitValidation::PreCallValidateDestroyCuFunctionNVX(VkDevice device,
                VkCuFunctionNVX function,
                const VkAllocationCallbacks* pAllocator) const {
    bool skip = false;
    if (pAllocator != nullptr) {
        const auto _s260 = pAllocator;
        skip |= ValidateAllocationCallbacks({},
            _s260->pUserData,
            _s260->pfnAllocation,
            _s260->pfnReallocation,
            _s260->pfnFree,
            _s260->pfnInternalAllocation,
            _s260->pfnInternalFree);
    }
    return skip;
}
bool ExplicitValidation::PreCallValidateCmdCuLaunchKernelNVX(VkCommandBuffer commandBuffer,
                const VkCuLaunchInfoNVX* pLaunchInfo) const {
    bool skip = false;
    LogObjectList _carryOverObjects;
    _carryOverObjects.add(commandBuffer);
    if (pLaunchInfo != nullptr) {
        const auto _s261 = pLaunchInfo;
        skip |= ValidateCuLaunchInfoNVX(_carryOverObjects,
            _s261->sType,
            _s261->pNext,
            _s261->function,
            _s261->gridDimX,
            _s261->gridDimY,
            _s261->gridDimZ,
            _s261->blockDimX,
            _s261->blockDimY,
            _s261->blockDimZ,
            _s261->sharedMemBytes,
            _s261->paramCount,
            _s261->pParams,
            _s261->extraCount,
            _s261->pExtras);
    }
    return skip;
}
bool ExplicitValidation::PreCallValidateGetImageViewHandleNVX(VkDevice device,
                const VkImageViewHandleInfoNVX* pInfo) const {
    bool skip = false;
    if (pInfo != nullptr) {
        const auto _s262 = pInfo;
        skip |= ValidateImageViewHandleInfoNVX({},
            _s262->sType,
            _s262->pNext,
            _s262->imageView,
            _s262->descriptorType,
            _s262->sampler);
    }
    return skip;
}
bool ExplicitValidation::PreCallValidateGetImageViewAddressNVX(VkDevice device,
                VkImageView imageView,
                VkImageViewAddressPropertiesNVX* pProperties) const {
    bool skip = false;
    return skip;
}
bool ExplicitValidation::PreCallValidateCmdDrawIndirectCountAMD(VkCommandBuffer commandBuffer,
                VkBuffer buffer,
                VkDeviceSize offset,
                VkBuffer countBuffer,
                VkDeviceSize countBufferOffset,
                uint32_t maxDrawCount,
                uint32_t stride) const {
    return PreCallValidateCmdDrawIndirectCount(commandBuffer, buffer, offset, countBuffer, countBufferOffset, maxDrawCount, stride);
}
bool ExplicitValidation::PreCallValidateCmdDrawIndexedIndirectCountAMD(VkCommandBuffer commandBuffer,
                VkBuffer buffer,
                VkDeviceSize offset,
                VkBuffer countBuffer,
                VkDeviceSize countBufferOffset,
                uint32_t maxDrawCount,
                uint32_t stride) const {
    return PreCallValidateCmdDrawIndexedIndirectCount(commandBuffer, buffer, offset, countBuffer, countBufferOffset, maxDrawCount, stride);
}
bool ExplicitValidation::PreCallValidateGetShaderInfoAMD(VkDevice device,
                VkPipeline pipeline,
                VkShaderStageFlagBits shaderStage,
                VkShaderInfoTypeAMD infoType,
                size_t* pInfoSize,
                void* pInfo) const {
    bool skip = false;
    return skip;
}
#ifdef VK_USE_PLATFORM_GGP
bool ExplicitValidation::PreCallValidateCreateStreamDescriptorSurfaceGGP(VkInstance instance,
                const VkStreamDescriptorSurfaceCreateInfoGGP* pCreateInfo,
                const VkAllocationCallbacks* pAllocator,
                VkSurfaceKHR* pSurface) const {
    bool skip = false;
    if (pCreateInfo != nullptr) {
        const auto _s263 = pCreateInfo;
        skip |= ValidateStreamDescriptorSurfaceCreateInfoGGP({},
            _s263->sType,
            _s263->pNext,
            _s263->flags,
            _s263->streamDescriptor);
    }
    if (pAllocator != nullptr) {
        const auto _s264 = pAllocator;
        skip |= ValidateAllocationCallbacks({},
            _s264->pUserData,
            _s264->pfnAllocation,
            _s264->pfnReallocation,
            _s264->pfnFree,
            _s264->pfnInternalAllocation,
            _s264->pfnInternalFree);
    }
    return skip;
}
#endif
bool ExplicitValidation::PreCallValidateGetPhysicalDeviceExternalImageFormatPropertiesNV(VkPhysicalDevice physicalDevice,
                VkFormat format,
                VkImageType type,
                VkImageTiling tiling,
                VkImageUsageFlags usage,
                VkImageCreateFlags flags,
                VkExternalMemoryHandleTypeFlagsNV externalHandleType,
                VkExternalImageFormatPropertiesNV* pExternalImageFormatProperties) const {
    bool skip = false;
    return skip;
}
#ifdef VK_USE_PLATFORM_WIN32_KHR
bool ExplicitValidation::PreCallValidateGetMemoryWin32HandleNV(VkDevice device,
                VkDeviceMemory memory,
                VkExternalMemoryHandleTypeFlagsNV handleType,
                HANDLE* pHandle) const {
    bool skip = false;
    return skip;
}
#endif
#ifdef VK_USE_PLATFORM_VI_NN
bool ExplicitValidation::PreCallValidateCreateViSurfaceNN(VkInstance instance,
                const VkViSurfaceCreateInfoNN* pCreateInfo,
                const VkAllocationCallbacks* pAllocator,
                VkSurfaceKHR* pSurface) const {
    bool skip = false;
    if (pCreateInfo != nullptr) {
        const auto _s265 = pCreateInfo;
        skip |= ValidateViSurfaceCreateInfoNN({},
            _s265->sType,
            _s265->pNext,
            _s265->flags,
            _s265->window);
    }
    if (pAllocator != nullptr) {
        const auto _s266 = pAllocator;
        skip |= ValidateAllocationCallbacks({},
            _s266->pUserData,
            _s266->pfnAllocation,
            _s266->pfnReallocation,
            _s266->pfnFree,
            _s266->pfnInternalAllocation,
            _s266->pfnInternalFree);
    }
    return skip;
}
#endif
bool ExplicitValidation::PreCallValidateCmdBeginConditionalRenderingEXT(VkCommandBuffer commandBuffer,
                const VkConditionalRenderingBeginInfoEXT* pConditionalRenderingBegin) const {
    bool skip = false;
    LogObjectList _carryOverObjects;
    _carryOverObjects.add(commandBuffer);
    if (pConditionalRenderingBegin != nullptr) {
        const auto _s267 = pConditionalRenderingBegin;
        skip |= ValidateConditionalRenderingBeginInfoEXT(_carryOverObjects,
            _s267->sType,
            _s267->pNext,
            _s267->buffer,
            _s267->offset,
            _s267->flags);
    }
    return skip;
}
bool ExplicitValidation::PreCallValidateCmdEndConditionalRenderingEXT(VkCommandBuffer commandBuffer) const {
    bool skip = false;
    LogObjectList _carryOverObjects;
    _carryOverObjects.add(commandBuffer);
    return skip;
}
bool ExplicitValidation::PreCallValidateCmdSetViewportWScalingNV(VkCommandBuffer commandBuffer,
                uint32_t firstViewport,
                uint32_t viewportCount,
                const VkViewportWScalingNV* pViewportWScalings) const {
    bool skip = false;
    LogObjectList _carryOverObjects;
    _carryOverObjects.add(commandBuffer);
    if (pViewportWScalings != nullptr) {
        for (uint32_t _i268 = 0;_i268 < viewportCount; ++_i268) {
            const auto _s269 = &pViewportWScalings[_i268];
            skip |= ValidateViewportWScalingNV(_carryOverObjects,
                _s269->xcoeff,
                _s269->ycoeff);
        }
    }
    return skip;
}
bool ExplicitValidation::PreCallValidateReleaseDisplayEXT(VkPhysicalDevice physicalDevice,
                VkDisplayKHR display) const {
    bool skip = false;
    return skip;
}
#ifdef VK_USE_PLATFORM_XLIB_XRANDR_EXT
bool ExplicitValidation::PreCallValidateAcquireXlibDisplayEXT(VkPhysicalDevice physicalDevice,
                Display* dpy,
                VkDisplayKHR display) const {
    bool skip = false;
    return skip;
}
#endif
#ifdef VK_USE_PLATFORM_XLIB_XRANDR_EXT
bool ExplicitValidation::PreCallValidateGetRandROutputDisplayEXT(VkPhysicalDevice physicalDevice,
                Display* dpy,
                RROutput rrOutput,
                VkDisplayKHR* pDisplay) const {
    bool skip = false;
    return skip;
}
#endif
bool ExplicitValidation::PreCallValidateGetPhysicalDeviceSurfaceCapabilities2EXT(VkPhysicalDevice physicalDevice,
                VkSurfaceKHR surface,
                VkSurfaceCapabilities2EXT* pSurfaceCapabilities) const {
    bool skip = false;
    return skip;
}
bool ExplicitValidation::PreCallValidateDisplayPowerControlEXT(VkDevice device,
                VkDisplayKHR display,
                const VkDisplayPowerInfoEXT* pDisplayPowerInfo) const {
    bool skip = false;
    if (pDisplayPowerInfo != nullptr) {
        const auto _s270 = pDisplayPowerInfo;
        skip |= ValidateDisplayPowerInfoEXT({},
            _s270->sType,
            _s270->pNext,
            _s270->powerState);
    }
    return skip;
}
bool ExplicitValidation::PreCallValidateRegisterDeviceEventEXT(VkDevice device,
                const VkDeviceEventInfoEXT* pDeviceEventInfo,
                const VkAllocationCallbacks* pAllocator,
                VkFence* pFence) const {
    bool skip = false;
    if (pDeviceEventInfo != nullptr) {
        const auto _s271 = pDeviceEventInfo;
        skip |= ValidateDeviceEventInfoEXT({},
            _s271->sType,
            _s271->pNext,
            _s271->deviceEvent);
    }
    if (pAllocator != nullptr) {
        const auto _s272 = pAllocator;
        skip |= ValidateAllocationCallbacks({},
            _s272->pUserData,
            _s272->pfnAllocation,
            _s272->pfnReallocation,
            _s272->pfnFree,
            _s272->pfnInternalAllocation,
            _s272->pfnInternalFree);
    }
    return skip;
}
bool ExplicitValidation::PreCallValidateRegisterDisplayEventEXT(VkDevice device,
                VkDisplayKHR display,
                const VkDisplayEventInfoEXT* pDisplayEventInfo,
                const VkAllocationCallbacks* pAllocator,
                VkFence* pFence) const {
    bool skip = false;
    if (pDisplayEventInfo != nullptr) {
        const auto _s273 = pDisplayEventInfo;
        skip |= ValidateDisplayEventInfoEXT({},
            _s273->sType,
            _s273->pNext,
            _s273->displayEvent);
    }
    if (pAllocator != nullptr) {
        const auto _s274 = pAllocator;
        skip |= ValidateAllocationCallbacks({},
            _s274->pUserData,
            _s274->pfnAllocation,
            _s274->pfnReallocation,
            _s274->pfnFree,
            _s274->pfnInternalAllocation,
            _s274->pfnInternalFree);
    }
    return skip;
}
bool ExplicitValidation::PreCallValidateGetSwapchainCounterEXT(VkDevice device,
                VkSwapchainKHR swapchain,
                VkSurfaceCounterFlagBitsEXT counter,
                uint64_t* pCounterValue) const {
    bool skip = false;
    return skip;
}
bool ExplicitValidation::PreCallValidateGetRefreshCycleDurationGOOGLE(VkDevice device,
                VkSwapchainKHR swapchain,
                VkRefreshCycleDurationGOOGLE* pDisplayTimingProperties) const {
    bool skip = false;
    return skip;
}
bool ExplicitValidation::PreCallValidateGetPastPresentationTimingGOOGLE(VkDevice device,
                VkSwapchainKHR swapchain,
                uint32_t* pPresentationTimingCount,
                VkPastPresentationTimingGOOGLE* pPresentationTimings) const {
    bool skip = false;
    return skip;
}
bool ExplicitValidation::PreCallValidateCmdSetDiscardRectangleEXT(VkCommandBuffer commandBuffer,
                uint32_t firstDiscardRectangle,
                uint32_t discardRectangleCount,
                const VkRect2D* pDiscardRectangles) const {
    bool skip = false;
    LogObjectList _carryOverObjects;
    _carryOverObjects.add(commandBuffer);
    if (pDiscardRectangles != nullptr) {
        for (uint32_t _i275 = 0;_i275 < discardRectangleCount; ++_i275) {
            const auto _s276 = &pDiscardRectangles[_i275];
            skip |= ValidateRect2D(_carryOverObjects,
                _s276->offset,
                _s276->extent);
        }
    }
    return skip;
}
bool ExplicitValidation::PreCallValidateCmdSetDiscardRectangleEnableEXT(VkCommandBuffer commandBuffer,
                VkBool32 discardRectangleEnable) const {
    bool skip = false;
    LogObjectList _carryOverObjects;
    _carryOverObjects.add(commandBuffer);
    return skip;
}
bool ExplicitValidation::PreCallValidateCmdSetDiscardRectangleModeEXT(VkCommandBuffer commandBuffer,
                VkDiscardRectangleModeEXT discardRectangleMode) const {
    bool skip = false;
    LogObjectList _carryOverObjects;
    _carryOverObjects.add(commandBuffer);
    return skip;
}
bool ExplicitValidation::PreCallValidateSetHdrMetadataEXT(VkDevice device,
                uint32_t swapchainCount,
                const VkSwapchainKHR* pSwapchains,
                const VkHdrMetadataEXT* pMetadata) const {
    bool skip = false;
    if (pMetadata != nullptr) {
        for (uint32_t _i277 = 0;_i277 < swapchainCount; ++_i277) {
            const auto _s278 = &pMetadata[_i277];
            skip |= ValidateHdrMetadataEXT({},
                _s278->sType,
                _s278->pNext,
                _s278->displayPrimaryRed,
                _s278->displayPrimaryGreen,
                _s278->displayPrimaryBlue,
                _s278->whitePoint,
                _s278->maxLuminance,
                _s278->minLuminance,
                _s278->maxContentLightLevel,
                _s278->maxFrameAverageLightLevel);
        }
    }
    return skip;
}
#ifdef VK_USE_PLATFORM_IOS_MVK
bool ExplicitValidation::PreCallValidateCreateIOSSurfaceMVK(VkInstance instance,
                const VkIOSSurfaceCreateInfoMVK* pCreateInfo,
                const VkAllocationCallbacks* pAllocator,
                VkSurfaceKHR* pSurface) const {
    bool skip = false;
    if (pCreateInfo != nullptr) {
        const auto _s279 = pCreateInfo;
        skip |= ValidateIOSSurfaceCreateInfoMVK({},
            _s279->sType,
            _s279->pNext,
            _s279->flags,
            _s279->pView);
    }
    if (pAllocator != nullptr) {
        const auto _s280 = pAllocator;
        skip |= ValidateAllocationCallbacks({},
            _s280->pUserData,
            _s280->pfnAllocation,
            _s280->pfnReallocation,
            _s280->pfnFree,
            _s280->pfnInternalAllocation,
            _s280->pfnInternalFree);
    }
    return skip;
}
#endif
#ifdef VK_USE_PLATFORM_MACOS_MVK
bool ExplicitValidation::PreCallValidateCreateMacOSSurfaceMVK(VkInstance instance,
                const VkMacOSSurfaceCreateInfoMVK* pCreateInfo,
                const VkAllocationCallbacks* pAllocator,
                VkSurfaceKHR* pSurface) const {
    bool skip = false;
    if (pCreateInfo != nullptr) {
        const auto _s281 = pCreateInfo;
        skip |= ValidateMacOSSurfaceCreateInfoMVK({},
            _s281->sType,
            _s281->pNext,
            _s281->flags,
            _s281->pView);
    }
    if (pAllocator != nullptr) {
        const auto _s282 = pAllocator;
        skip |= ValidateAllocationCallbacks({},
            _s282->pUserData,
            _s282->pfnAllocation,
            _s282->pfnReallocation,
            _s282->pfnFree,
            _s282->pfnInternalAllocation,
            _s282->pfnInternalFree);
    }
    return skip;
}
#endif
bool ExplicitValidation::PreCallValidateSetDebugUtilsObjectNameEXT(VkDevice device,
                const VkDebugUtilsObjectNameInfoEXT* pNameInfo) const {
    bool skip = false;
    if (pNameInfo != nullptr) {
        const auto _s283 = pNameInfo;
        skip |= ValidateDebugUtilsObjectNameInfoEXT({},
            _s283->sType,
            _s283->pNext,
            _s283->objectType,
            _s283->objectHandle,
            _s283->pObjectName);
    }
    return skip;
}
bool ExplicitValidation::PreCallValidateSetDebugUtilsObjectTagEXT(VkDevice device,
                const VkDebugUtilsObjectTagInfoEXT* pTagInfo) const {
    bool skip = false;
    if (pTagInfo != nullptr) {
        const auto _s284 = pTagInfo;
        skip |= ValidateDebugUtilsObjectTagInfoEXT({},
            _s284->sType,
            _s284->pNext,
            _s284->objectType,
            _s284->objectHandle,
            _s284->tagName,
            _s284->tagSize,
            _s284->pTag);
    }
    return skip;
}
bool ExplicitValidation::PreCallValidateQueueBeginDebugUtilsLabelEXT(VkQueue queue,
                const VkDebugUtilsLabelEXT* pLabelInfo) const {
    bool skip = false;
    if (pLabelInfo != nullptr) {
        const auto _s285 = pLabelInfo;
        skip |= ValidateDebugUtilsLabelEXT({},
            _s285->sType,
            _s285->pNext,
            _s285->pLabelName,
            _s285->color);
    }
    return skip;
}
bool ExplicitValidation::PreCallValidateQueueEndDebugUtilsLabelEXT(VkQueue queue) const {
    bool skip = false;
    return skip;
}
bool ExplicitValidation::PreCallValidateQueueInsertDebugUtilsLabelEXT(VkQueue queue,
                const VkDebugUtilsLabelEXT* pLabelInfo) const {
    bool skip = false;
    if (pLabelInfo != nullptr) {
        const auto _s286 = pLabelInfo;
        skip |= ValidateDebugUtilsLabelEXT({},
            _s286->sType,
            _s286->pNext,
            _s286->pLabelName,
            _s286->color);
    }
    return skip;
}
bool ExplicitValidation::PreCallValidateCmdBeginDebugUtilsLabelEXT(VkCommandBuffer commandBuffer,
                const VkDebugUtilsLabelEXT* pLabelInfo) const {
    bool skip = false;
    LogObjectList _carryOverObjects;
    _carryOverObjects.add(commandBuffer);
    if (pLabelInfo != nullptr) {
        const auto _s287 = pLabelInfo;
        skip |= ValidateDebugUtilsLabelEXT(_carryOverObjects,
            _s287->sType,
            _s287->pNext,
            _s287->pLabelName,
            _s287->color);
    }
    return skip;
}
bool ExplicitValidation::PreCallValidateCmdEndDebugUtilsLabelEXT(VkCommandBuffer commandBuffer) const {
    bool skip = false;
    LogObjectList _carryOverObjects;
    _carryOverObjects.add(commandBuffer);
    return skip;
}
bool ExplicitValidation::PreCallValidateCmdInsertDebugUtilsLabelEXT(VkCommandBuffer commandBuffer,
                const VkDebugUtilsLabelEXT* pLabelInfo) const {
    bool skip = false;
    LogObjectList _carryOverObjects;
    _carryOverObjects.add(commandBuffer);
    if (pLabelInfo != nullptr) {
        const auto _s288 = pLabelInfo;
        skip |= ValidateDebugUtilsLabelEXT(_carryOverObjects,
            _s288->sType,
            _s288->pNext,
            _s288->pLabelName,
            _s288->color);
    }
    return skip;
}
bool ExplicitValidation::PreCallValidateCreateDebugUtilsMessengerEXT(VkInstance instance,
                const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
                const VkAllocationCallbacks* pAllocator,
                VkDebugUtilsMessengerEXT* pMessenger) const {
    bool skip = false;
    if (pCreateInfo != nullptr) {
        const auto _s289 = pCreateInfo;
        skip |= ValidateDebugUtilsMessengerCreateInfoEXT({},
            _s289->sType,
            _s289->pNext,
            _s289->flags,
            _s289->messageSeverity,
            _s289->messageType,
            _s289->pfnUserCallback,
            _s289->pUserData);
    }
    if (pAllocator != nullptr) {
        const auto _s290 = pAllocator;
        skip |= ValidateAllocationCallbacks({},
            _s290->pUserData,
            _s290->pfnAllocation,
            _s290->pfnReallocation,
            _s290->pfnFree,
            _s290->pfnInternalAllocation,
            _s290->pfnInternalFree);
    }
    return skip;
}
bool ExplicitValidation::PreCallValidateDestroyDebugUtilsMessengerEXT(VkInstance instance,
                VkDebugUtilsMessengerEXT messenger,
                const VkAllocationCallbacks* pAllocator) const {
    bool skip = false;
    if (pAllocator != nullptr) {
        const auto _s291 = pAllocator;
        skip |= ValidateAllocationCallbacks({},
            _s291->pUserData,
            _s291->pfnAllocation,
            _s291->pfnReallocation,
            _s291->pfnFree,
            _s291->pfnInternalAllocation,
            _s291->pfnInternalFree);
    }
    return skip;
}
bool ExplicitValidation::PreCallValidateSubmitDebugUtilsMessageEXT(VkInstance instance,
                VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
                VkDebugUtilsMessageTypeFlagsEXT messageTypes,
                const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData) const {
    bool skip = false;
    if (pCallbackData != nullptr) {
        const auto _s292 = pCallbackData;
        skip |= ValidateDebugUtilsMessengerCallbackDataEXT({},
            _s292->sType,
            _s292->pNext,
            _s292->flags,
            _s292->pMessageIdName,
            _s292->messageIdNumber,
            _s292->pMessage,
            _s292->queueLabelCount,
            _s292->pQueueLabels,
            _s292->cmdBufLabelCount,
            _s292->pCmdBufLabels,
            _s292->objectCount,
            _s292->pObjects);
    }
    return skip;
}
#ifdef VK_USE_PLATFORM_ANDROID_KHR
bool ExplicitValidation::PreCallValidateGetAndroidHardwareBufferPropertiesANDROID(VkDevice device,
                const struct AHardwareBuffer* buffer,
                VkAndroidHardwareBufferPropertiesANDROID* pProperties) const {
    bool skip = false;
    return skip;
}
#endif
#ifdef VK_USE_PLATFORM_ANDROID_KHR
bool ExplicitValidation::PreCallValidateGetMemoryAndroidHardwareBufferANDROID(VkDevice device,
                const VkMemoryGetAndroidHardwareBufferInfoANDROID* pInfo,
                struct AHardwareBuffer** pBuffer) const {
    bool skip = false;
    if (pInfo != nullptr) {
        const auto _s293 = pInfo;
        skip |= ValidateMemoryGetAndroidHardwareBufferInfoANDROID({},
            _s293->sType,
            _s293->pNext,
            _s293->memory);
    }
    return skip;
}
#endif
bool ExplicitValidation::PreCallValidateCmdSetSampleLocationsEXT(VkCommandBuffer commandBuffer,
                const VkSampleLocationsInfoEXT* pSampleLocationsInfo) const {
    bool skip = false;
    LogObjectList _carryOverObjects;
    _carryOverObjects.add(commandBuffer);
    if (pSampleLocationsInfo != nullptr) {
        const auto _s294 = pSampleLocationsInfo;
        skip |= ValidateSampleLocationsInfoEXT(_carryOverObjects,
            _s294->sType,
            _s294->pNext,
            _s294->sampleLocationsPerPixel,
            _s294->sampleLocationGridSize,
            _s294->sampleLocationsCount,
            _s294->pSampleLocations);
    }
    return skip;
}
bool ExplicitValidation::PreCallValidateGetPhysicalDeviceMultisamplePropertiesEXT(VkPhysicalDevice physicalDevice,
                VkSampleCountFlagBits samples,
                VkMultisamplePropertiesEXT* pMultisampleProperties) const {
    bool skip = false;
    return skip;
}
bool ExplicitValidation::PreCallValidateGetImageDrmFormatModifierPropertiesEXT(VkDevice device,
                VkImage image,
                VkImageDrmFormatModifierPropertiesEXT* pProperties) const {
    bool skip = false;
    return skip;
}
bool ExplicitValidation::PreCallValidateCmdBindShadingRateImageNV(VkCommandBuffer commandBuffer,
                VkImageView imageView,
                VkImageLayout imageLayout) const {
    bool skip = false;
    LogObjectList _carryOverObjects;
    _carryOverObjects.add(commandBuffer);
    return skip;
}
bool ExplicitValidation::PreCallValidateCmdSetViewportShadingRatePaletteNV(VkCommandBuffer commandBuffer,
                uint32_t firstViewport,
                uint32_t viewportCount,
                const VkShadingRatePaletteNV* pShadingRatePalettes) const {
    bool skip = false;
    LogObjectList _carryOverObjects;
    _carryOverObjects.add(commandBuffer);
    if (pShadingRatePalettes != nullptr) {
        for (uint32_t _i295 = 0;_i295 < viewportCount; ++_i295) {
            const auto _s296 = &pShadingRatePalettes[_i295];
            skip |= ValidateShadingRatePaletteNV(_carryOverObjects,
                _s296->shadingRatePaletteEntryCount,
                _s296->pShadingRatePaletteEntries);
        }
    }
    return skip;
}
bool ExplicitValidation::PreCallValidateCmdSetCoarseSampleOrderNV(VkCommandBuffer commandBuffer,
                VkCoarseSampleOrderTypeNV sampleOrderType,
                uint32_t customSampleOrderCount,
                const VkCoarseSampleOrderCustomNV* pCustomSampleOrders) const {
    bool skip = false;
    LogObjectList _carryOverObjects;
    _carryOverObjects.add(commandBuffer);
    if (pCustomSampleOrders != nullptr) {
        for (uint32_t _i297 = 0;_i297 < customSampleOrderCount; ++_i297) {
            const auto _s298 = &pCustomSampleOrders[_i297];
            skip |= ValidateCoarseSampleOrderCustomNV(_carryOverObjects,
                _s298->shadingRate,
                _s298->sampleCount,
                _s298->sampleLocationCount,
                _s298->pSampleLocations);
        }
    }
    return skip;
}
bool ExplicitValidation::PreCallValidateCreateAccelerationStructureNV(VkDevice device,
                const VkAccelerationStructureCreateInfoNV* pCreateInfo,
                const VkAllocationCallbacks* pAllocator,
                VkAccelerationStructureNV* pAccelerationStructure) const {
    bool skip = false;
    if (pCreateInfo != nullptr) {
        const auto _s299 = pCreateInfo;
        skip |= ValidateAccelerationStructureCreateInfoNV({},
            _s299->sType,
            _s299->pNext,
            _s299->compactedSize,
            _s299->info);
    }
    if (pAllocator != nullptr) {
        const auto _s300 = pAllocator;
        skip |= ValidateAllocationCallbacks({},
            _s300->pUserData,
            _s300->pfnAllocation,
            _s300->pfnReallocation,
            _s300->pfnFree,
            _s300->pfnInternalAllocation,
            _s300->pfnInternalFree);
    }
    return skip;
}
bool ExplicitValidation::PreCallValidateDestroyAccelerationStructureNV(VkDevice device,
                VkAccelerationStructureNV accelerationStructure,
                const VkAllocationCallbacks* pAllocator) const {
    bool skip = false;
    if (pAllocator != nullptr) {
        const auto _s301 = pAllocator;
        skip |= ValidateAllocationCallbacks({},
            _s301->pUserData,
            _s301->pfnAllocation,
            _s301->pfnReallocation,
            _s301->pfnFree,
            _s301->pfnInternalAllocation,
            _s301->pfnInternalFree);
    }
    return skip;
}
bool ExplicitValidation::PreCallValidateGetAccelerationStructureMemoryRequirementsNV(VkDevice device,
                const VkAccelerationStructureMemoryRequirementsInfoNV* pInfo,
                VkMemoryRequirements2KHR* pMemoryRequirements) const {
    bool skip = false;
    if (pInfo != nullptr) {
        const auto _s302 = pInfo;
        skip |= ValidateAccelerationStructureMemoryRequirementsInfoNV({},
            _s302->sType,
            _s302->pNext,
            _s302->type,
            _s302->accelerationStructure);
    }
    return skip;
}
bool ExplicitValidation::PreCallValidateBindAccelerationStructureMemoryNV(VkDevice device,
                uint32_t bindInfoCount,
                const VkBindAccelerationStructureMemoryInfoNV* pBindInfos) const {
    bool skip = false;
    if (pBindInfos != nullptr) {
        for (uint32_t _i303 = 0;_i303 < bindInfoCount; ++_i303) {
            const auto _s304 = &pBindInfos[_i303];
            skip |= ValidateBindAccelerationStructureMemoryInfoNV({},
                _s304->sType,
                _s304->pNext,
                _s304->accelerationStructure,
                _s304->memory,
                _s304->memoryOffset,
                _s304->deviceIndexCount,
                _s304->pDeviceIndices);
        }
    }
    return skip;
}
bool ExplicitValidation::PreCallValidateCmdBuildAccelerationStructureNV(VkCommandBuffer commandBuffer,
                const VkAccelerationStructureInfoNV* pInfo,
                VkBuffer instanceData,
                VkDeviceSize instanceOffset,
                VkBool32 update,
                VkAccelerationStructureNV dst,
                VkAccelerationStructureNV src,
                VkBuffer scratch,
                VkDeviceSize scratchOffset) const {
    bool skip = false;
    LogObjectList _carryOverObjects;
    _carryOverObjects.add(commandBuffer);
    if (pInfo != nullptr) {
        const auto _s305 = pInfo;
        skip |= ValidateAccelerationStructureInfoNV(_carryOverObjects,
            _s305->sType,
            _s305->pNext,
            _s305->type,
            _s305->flags,
            _s305->instanceCount,
            _s305->geometryCount,
            _s305->pGeometries);
    }
    return skip;
}
bool ExplicitValidation::PreCallValidateCmdCopyAccelerationStructureNV(VkCommandBuffer commandBuffer,
                VkAccelerationStructureNV dst,
                VkAccelerationStructureNV src,
                VkCopyAccelerationStructureModeKHR mode) const {
    bool skip = false;
    LogObjectList _carryOverObjects;
    _carryOverObjects.add(commandBuffer);
    return skip;
}
bool ExplicitValidation::PreCallValidateCmdTraceRaysNV(VkCommandBuffer commandBuffer,
                VkBuffer raygenShaderBindingTableBuffer,
                VkDeviceSize raygenShaderBindingOffset,
                VkBuffer missShaderBindingTableBuffer,
                VkDeviceSize missShaderBindingOffset,
                VkDeviceSize missShaderBindingStride,
                VkBuffer hitShaderBindingTableBuffer,
                VkDeviceSize hitShaderBindingOffset,
                VkDeviceSize hitShaderBindingStride,
                VkBuffer callableShaderBindingTableBuffer,
                VkDeviceSize callableShaderBindingOffset,
                VkDeviceSize callableShaderBindingStride,
                uint32_t width,
                uint32_t height,
                uint32_t depth) const {
    bool skip = false;
    LogObjectList _carryOverObjects;
    _carryOverObjects.add(commandBuffer);
    return skip;
}
bool ExplicitValidation::PreCallValidateCreateRayTracingPipelinesNV(VkDevice device,
                VkPipelineCache pipelineCache,
                uint32_t createInfoCount,
                const VkRayTracingPipelineCreateInfoNV* pCreateInfos,
                const VkAllocationCallbacks* pAllocator,
                VkPipeline* pPipelines,
                void *validation_state) const {
    bool skip = false;
    if (pCreateInfos != nullptr) {
        for (uint32_t _i306 = 0;_i306 < createInfoCount; ++_i306) {
            const auto _s307 = &pCreateInfos[_i306];
            skip |= ValidateRayTracingPipelineCreateInfoNV({},
                _s307->sType,
                _s307->pNext,
                _s307->flags,
                _s307->stageCount,
                _s307->pStages,
                _s307->groupCount,
                _s307->pGroups,
                _s307->maxRecursionDepth,
                _s307->layout,
                _s307->basePipelineHandle,
                _s307->basePipelineIndex);
        }
    }
    if (pAllocator != nullptr) {
        const auto _s308 = pAllocator;
        skip |= ValidateAllocationCallbacks({},
            _s308->pUserData,
            _s308->pfnAllocation,
            _s308->pfnReallocation,
            _s308->pfnFree,
            _s308->pfnInternalAllocation,
            _s308->pfnInternalFree);
    }
    return skip;
}
bool ExplicitValidation::PreCallValidateGetRayTracingShaderGroupHandlesKHR(VkDevice device,
                VkPipeline pipeline,
                uint32_t firstGroup,
                uint32_t groupCount,
                size_t dataSize,
                void* pData) const {
    bool skip = false;
    return skip;
}
bool ExplicitValidation::PreCallValidateGetRayTracingShaderGroupHandlesNV(VkDevice device,
                VkPipeline pipeline,
                uint32_t firstGroup,
                uint32_t groupCount,
                size_t dataSize,
                void* pData) const {
    return PreCallValidateGetRayTracingShaderGroupHandlesKHR(device, pipeline, firstGroup, groupCount, dataSize, pData);
}
bool ExplicitValidation::PreCallValidateGetAccelerationStructureHandleNV(VkDevice device,
                VkAccelerationStructureNV accelerationStructure,
                size_t dataSize,
                void* pData) const {
    bool skip = false;
    return skip;
}
bool ExplicitValidation::PreCallValidateCmdWriteAccelerationStructuresPropertiesNV(VkCommandBuffer commandBuffer,
                uint32_t accelerationStructureCount,
                const VkAccelerationStructureNV* pAccelerationStructures,
                VkQueryType queryType,
                VkQueryPool queryPool,
                uint32_t firstQuery) const {
    bool skip = false;
    LogObjectList _carryOverObjects;
    _carryOverObjects.add(commandBuffer);
    return skip;
}
bool ExplicitValidation::PreCallValidateCompileDeferredNV(VkDevice device,
                VkPipeline pipeline,
                uint32_t shader) const {
    bool skip = false;
    return skip;
}
bool ExplicitValidation::PreCallValidateGetMemoryHostPointerPropertiesEXT(VkDevice device,
                VkExternalMemoryHandleTypeFlagBits handleType,
                const void* pHostPointer,
                VkMemoryHostPointerPropertiesEXT* pMemoryHostPointerProperties) const {
    bool skip = false;
    return skip;
}
bool ExplicitValidation::PreCallValidateCmdWriteBufferMarkerAMD(VkCommandBuffer commandBuffer,
                VkPipelineStageFlagBits pipelineStage,
                VkBuffer dstBuffer,
                VkDeviceSize dstOffset,
                uint32_t marker) const {
    bool skip = false;
    LogObjectList _carryOverObjects;
    _carryOverObjects.add(commandBuffer);
    return skip;
}
bool ExplicitValidation::PreCallValidateGetPhysicalDeviceCalibrateableTimeDomainsEXT(VkPhysicalDevice physicalDevice,
                uint32_t* pTimeDomainCount,
                VkTimeDomainEXT* pTimeDomains) const {
    bool skip = false;
    return skip;
}
bool ExplicitValidation::PreCallValidateGetCalibratedTimestampsEXT(VkDevice device,
                uint32_t timestampCount,
                const VkCalibratedTimestampInfoEXT* pTimestampInfos,
                uint64_t* pTimestamps,
                uint64_t* pMaxDeviation) const {
    bool skip = false;
    if (pTimestampInfos != nullptr) {
        for (uint32_t _i309 = 0;_i309 < timestampCount; ++_i309) {
            const auto _s310 = &pTimestampInfos[_i309];
            skip |= ValidateCalibratedTimestampInfoEXT({},
                _s310->sType,
                _s310->pNext,
                _s310->timeDomain);
        }
    }
    return skip;
}
bool ExplicitValidation::PreCallValidateCmdDrawMeshTasksNV(VkCommandBuffer commandBuffer,
                uint32_t taskCount,
                uint32_t firstTask) const {
    bool skip = false;
    LogObjectList _carryOverObjects;
    _carryOverObjects.add(commandBuffer);
    return skip;
}
bool ExplicitValidation::PreCallValidateCmdDrawMeshTasksIndirectNV(VkCommandBuffer commandBuffer,
                VkBuffer buffer,
                VkDeviceSize offset,
                uint32_t drawCount,
                uint32_t stride) const {
    bool skip = false;
    LogObjectList _carryOverObjects;
    _carryOverObjects.add(commandBuffer);
    return skip;
}
bool ExplicitValidation::PreCallValidateCmdDrawMeshTasksIndirectCountNV(VkCommandBuffer commandBuffer,
                VkBuffer buffer,
                VkDeviceSize offset,
                VkBuffer countBuffer,
                VkDeviceSize countBufferOffset,
                uint32_t maxDrawCount,
                uint32_t stride) const {
    bool skip = false;
    LogObjectList _carryOverObjects;
    _carryOverObjects.add(commandBuffer);
    return skip;
}
bool ExplicitValidation::PreCallValidateCmdSetExclusiveScissorEnableNV(VkCommandBuffer commandBuffer,
                uint32_t firstExclusiveScissor,
                uint32_t exclusiveScissorCount,
                const VkBool32* pExclusiveScissorEnables) const {
    bool skip = false;
    LogObjectList _carryOverObjects;
    _carryOverObjects.add(commandBuffer);
    return skip;
}
bool ExplicitValidation::PreCallValidateCmdSetExclusiveScissorNV(VkCommandBuffer commandBuffer,
                uint32_t firstExclusiveScissor,
                uint32_t exclusiveScissorCount,
                const VkRect2D* pExclusiveScissors) const {
    bool skip = false;
    LogObjectList _carryOverObjects;
    _carryOverObjects.add(commandBuffer);
    if (pExclusiveScissors != nullptr) {
        for (uint32_t _i311 = 0;_i311 < exclusiveScissorCount; ++_i311) {
            const auto _s312 = &pExclusiveScissors[_i311];
            skip |= ValidateRect2D(_carryOverObjects,
                _s312->offset,
                _s312->extent);
        }
    }
    return skip;
}
bool ExplicitValidation::PreCallValidateCmdSetCheckpointNV(VkCommandBuffer commandBuffer,
                const void* pCheckpointMarker) const {
    bool skip = false;
    LogObjectList _carryOverObjects;
    _carryOverObjects.add(commandBuffer);
    return skip;
}
bool ExplicitValidation::PreCallValidateGetQueueCheckpointDataNV(VkQueue queue,
                uint32_t* pCheckpointDataCount,
                VkCheckpointDataNV* pCheckpointData) const {
    bool skip = false;
    return skip;
}
bool ExplicitValidation::PreCallValidateInitializePerformanceApiINTEL(VkDevice device,
                const VkInitializePerformanceApiInfoINTEL* pInitializeInfo) const {
    bool skip = false;
    if (pInitializeInfo != nullptr) {
        const auto _s313 = pInitializeInfo;
        skip |= ValidateInitializePerformanceApiInfoINTEL({},
            _s313->sType,
            _s313->pNext,
            _s313->pUserData);
    }
    return skip;
}
bool ExplicitValidation::PreCallValidateUninitializePerformanceApiINTEL(VkDevice device) const {
    bool skip = false;
    return skip;
}
bool ExplicitValidation::PreCallValidateCmdSetPerformanceMarkerINTEL(VkCommandBuffer commandBuffer,
                const VkPerformanceMarkerInfoINTEL* pMarkerInfo) const {
    bool skip = false;
    LogObjectList _carryOverObjects;
    _carryOverObjects.add(commandBuffer);
    if (pMarkerInfo != nullptr) {
        const auto _s314 = pMarkerInfo;
        skip |= ValidatePerformanceMarkerInfoINTEL(_carryOverObjects,
            _s314->sType,
            _s314->pNext,
            _s314->marker);
    }
    return skip;
}
bool ExplicitValidation::PreCallValidateCmdSetPerformanceStreamMarkerINTEL(VkCommandBuffer commandBuffer,
                const VkPerformanceStreamMarkerInfoINTEL* pMarkerInfo) const {
    bool skip = false;
    LogObjectList _carryOverObjects;
    _carryOverObjects.add(commandBuffer);
    if (pMarkerInfo != nullptr) {
        const auto _s315 = pMarkerInfo;
        skip |= ValidatePerformanceStreamMarkerInfoINTEL(_carryOverObjects,
            _s315->sType,
            _s315->pNext,
            _s315->marker);
    }
    return skip;
}
bool ExplicitValidation::PreCallValidateCmdSetPerformanceOverrideINTEL(VkCommandBuffer commandBuffer,
                const VkPerformanceOverrideInfoINTEL* pOverrideInfo) const {
    bool skip = false;
    LogObjectList _carryOverObjects;
    _carryOverObjects.add(commandBuffer);
    if (pOverrideInfo != nullptr) {
        const auto _s316 = pOverrideInfo;
        skip |= ValidatePerformanceOverrideInfoINTEL(_carryOverObjects,
            _s316->sType,
            _s316->pNext,
            _s316->type,
            _s316->enable,
            _s316->parameter);
    }
    return skip;
}
bool ExplicitValidation::PreCallValidateAcquirePerformanceConfigurationINTEL(VkDevice device,
                const VkPerformanceConfigurationAcquireInfoINTEL* pAcquireInfo,
                VkPerformanceConfigurationINTEL* pConfiguration) const {
    bool skip = false;
    if (pAcquireInfo != nullptr) {
        const auto _s317 = pAcquireInfo;
        skip |= ValidatePerformanceConfigurationAcquireInfoINTEL({},
            _s317->sType,
            _s317->pNext,
            _s317->type);
    }
    return skip;
}
bool ExplicitValidation::PreCallValidateReleasePerformanceConfigurationINTEL(VkDevice device,
                VkPerformanceConfigurationINTEL configuration) const {
    bool skip = false;
    return skip;
}
bool ExplicitValidation::PreCallValidateQueueSetPerformanceConfigurationINTEL(VkQueue queue,
                VkPerformanceConfigurationINTEL configuration) const {
    bool skip = false;
    return skip;
}
bool ExplicitValidation::PreCallValidateGetPerformanceParameterINTEL(VkDevice device,
                VkPerformanceParameterTypeINTEL parameter,
                VkPerformanceValueINTEL* pValue) const {
    bool skip = false;
    if (pValue != nullptr) {
        const auto _s318 = pValue;
        skip |= ValidatePerformanceValueINTEL({},
            _s318->type,
            _s318->data);
    }
    return skip;
}
bool ExplicitValidation::PreCallValidateSetLocalDimmingAMD(VkDevice device,
                VkSwapchainKHR swapChain,
                VkBool32 localDimmingEnable) const {
    bool skip = false;
    return skip;
}
#ifdef VK_USE_PLATFORM_FUCHSIA
bool ExplicitValidation::PreCallValidateCreateImagePipeSurfaceFUCHSIA(VkInstance instance,
                const VkImagePipeSurfaceCreateInfoFUCHSIA* pCreateInfo,
                const VkAllocationCallbacks* pAllocator,
                VkSurfaceKHR* pSurface) const {
    bool skip = false;
    if (pCreateInfo != nullptr) {
        const auto _s319 = pCreateInfo;
        skip |= ValidateImagePipeSurfaceCreateInfoFUCHSIA({},
            _s319->sType,
            _s319->pNext,
            _s319->flags,
            _s319->imagePipeHandle);
    }
    if (pAllocator != nullptr) {
        const auto _s320 = pAllocator;
        skip |= ValidateAllocationCallbacks({},
            _s320->pUserData,
            _s320->pfnAllocation,
            _s320->pfnReallocation,
            _s320->pfnFree,
            _s320->pfnInternalAllocation,
            _s320->pfnInternalFree);
    }
    return skip;
}
#endif
#ifdef VK_USE_PLATFORM_METAL_EXT
bool ExplicitValidation::PreCallValidateCreateMetalSurfaceEXT(VkInstance instance,
                const VkMetalSurfaceCreateInfoEXT* pCreateInfo,
                const VkAllocationCallbacks* pAllocator,
                VkSurfaceKHR* pSurface) const {
    bool skip = false;
    if (pCreateInfo != nullptr) {
        const auto _s321 = pCreateInfo;
        skip |= ValidateMetalSurfaceCreateInfoEXT({},
            _s321->sType,
            _s321->pNext,
            _s321->flags,
            _s321->pLayer);
    }
    if (pAllocator != nullptr) {
        const auto _s322 = pAllocator;
        skip |= ValidateAllocationCallbacks({},
            _s322->pUserData,
            _s322->pfnAllocation,
            _s322->pfnReallocation,
            _s322->pfnFree,
            _s322->pfnInternalAllocation,
            _s322->pfnInternalFree);
    }
    return skip;
}
#endif
bool ExplicitValidation::PreCallValidateGetBufferDeviceAddressEXT(VkDevice device,
                const VkBufferDeviceAddressInfo* pInfo) const {
    return PreCallValidateGetBufferDeviceAddress(device, pInfo);
}
bool ExplicitValidation::PreCallValidateGetPhysicalDeviceToolPropertiesEXT(VkPhysicalDevice physicalDevice,
                uint32_t* pToolCount,
                VkPhysicalDeviceToolProperties* pToolProperties) const {
    return PreCallValidateGetPhysicalDeviceToolProperties(physicalDevice, pToolCount, pToolProperties);
}
bool ExplicitValidation::PreCallValidateGetPhysicalDeviceCooperativeMatrixPropertiesNV(VkPhysicalDevice physicalDevice,
                uint32_t* pPropertyCount,
                VkCooperativeMatrixPropertiesNV* pProperties) const {
    bool skip = false;
    if (pProperties != nullptr) {
        for (uint32_t _i323 = 0;_i323 < *pPropertyCount; ++_i323) {
            const auto _s324 = &pProperties[_i323];
            skip |= ValidateCooperativeMatrixPropertiesNV({},
                _s324->sType,
                _s324->pNext,
                _s324->MSize,
                _s324->NSize,
                _s324->KSize,
                _s324->AType,
                _s324->BType,
                _s324->CType,
                _s324->DType,
                _s324->scope);
        }
    }
    return skip;
}
bool ExplicitValidation::PreCallValidateGetPhysicalDeviceSupportedFramebufferMixedSamplesCombinationsNV(VkPhysicalDevice physicalDevice,
                uint32_t* pCombinationCount,
                VkFramebufferMixedSamplesCombinationNV* pCombinations) const {
    bool skip = false;
    return skip;
}
#ifdef VK_USE_PLATFORM_WIN32_KHR
bool ExplicitValidation::PreCallValidateGetPhysicalDeviceSurfacePresentModes2EXT(VkPhysicalDevice physicalDevice,
                const VkPhysicalDeviceSurfaceInfo2KHR* pSurfaceInfo,
                uint32_t* pPresentModeCount,
                VkPresentModeKHR* pPresentModes) const {
    bool skip = false;
    if (pSurfaceInfo != nullptr) {
        const auto _s325 = pSurfaceInfo;
        skip |= ValidatePhysicalDeviceSurfaceInfo2KHR({},
            _s325->sType,
            _s325->pNext,
            _s325->surface);
    }
    return skip;
}
#endif
#ifdef VK_USE_PLATFORM_WIN32_KHR
bool ExplicitValidation::PreCallValidateAcquireFullScreenExclusiveModeEXT(VkDevice device,
                VkSwapchainKHR swapchain) const {
    bool skip = false;
    return skip;
}
#endif
#ifdef VK_USE_PLATFORM_WIN32_KHR
bool ExplicitValidation::PreCallValidateReleaseFullScreenExclusiveModeEXT(VkDevice device,
                VkSwapchainKHR swapchain) const {
    bool skip = false;
    return skip;
}
#endif
#ifdef VK_USE_PLATFORM_WIN32_KHR
bool ExplicitValidation::PreCallValidateGetDeviceGroupSurfacePresentModes2EXT(VkDevice device,
                const VkPhysicalDeviceSurfaceInfo2KHR* pSurfaceInfo,
                VkDeviceGroupPresentModeFlagsKHR* pModes) const {
    bool skip = false;
    if (pSurfaceInfo != nullptr) {
        const auto _s326 = pSurfaceInfo;
        skip |= ValidatePhysicalDeviceSurfaceInfo2KHR({},
            _s326->sType,
            _s326->pNext,
            _s326->surface);
    }
    return skip;
}
#endif
bool ExplicitValidation::PreCallValidateCreateHeadlessSurfaceEXT(VkInstance instance,
                const VkHeadlessSurfaceCreateInfoEXT* pCreateInfo,
                const VkAllocationCallbacks* pAllocator,
                VkSurfaceKHR* pSurface) const {
    bool skip = false;
    if (pCreateInfo != nullptr) {
        const auto _s327 = pCreateInfo;
        skip |= ValidateHeadlessSurfaceCreateInfoEXT({},
            _s327->sType,
            _s327->pNext,
            _s327->flags);
    }
    if (pAllocator != nullptr) {
        const auto _s328 = pAllocator;
        skip |= ValidateAllocationCallbacks({},
            _s328->pUserData,
            _s328->pfnAllocation,
            _s328->pfnReallocation,
            _s328->pfnFree,
            _s328->pfnInternalAllocation,
            _s328->pfnInternalFree);
    }
    return skip;
}
bool ExplicitValidation::PreCallValidateCmdSetLineStippleEXT(VkCommandBuffer commandBuffer,
                uint32_t lineStippleFactor,
                uint16_t lineStipplePattern) const {
    bool skip = false;
    LogObjectList _carryOverObjects;
    _carryOverObjects.add(commandBuffer);
    return skip;
}
bool ExplicitValidation::PreCallValidateResetQueryPoolEXT(VkDevice device,
                VkQueryPool queryPool,
                uint32_t firstQuery,
                uint32_t queryCount) const {
    return PreCallValidateResetQueryPool(device, queryPool, firstQuery, queryCount);
}
bool ExplicitValidation::PreCallValidateCmdSetCullModeEXT(VkCommandBuffer commandBuffer,
                VkCullModeFlags cullMode) const {
    return PreCallValidateCmdSetCullMode(commandBuffer, cullMode);
}
bool ExplicitValidation::PreCallValidateCmdSetFrontFaceEXT(VkCommandBuffer commandBuffer,
                VkFrontFace frontFace) const {
    return PreCallValidateCmdSetFrontFace(commandBuffer, frontFace);
}
bool ExplicitValidation::PreCallValidateCmdSetPrimitiveTopologyEXT(VkCommandBuffer commandBuffer,
                VkPrimitiveTopology primitiveTopology) const {
    return PreCallValidateCmdSetPrimitiveTopology(commandBuffer, primitiveTopology);
}
bool ExplicitValidation::PreCallValidateCmdSetViewportWithCountEXT(VkCommandBuffer commandBuffer,
                uint32_t viewportCount,
                const VkViewport* pViewports) const {
    return PreCallValidateCmdSetViewportWithCount(commandBuffer, viewportCount, pViewports);
}
bool ExplicitValidation::PreCallValidateCmdSetScissorWithCountEXT(VkCommandBuffer commandBuffer,
                uint32_t scissorCount,
                const VkRect2D* pScissors) const {
    return PreCallValidateCmdSetScissorWithCount(commandBuffer, scissorCount, pScissors);
}
bool ExplicitValidation::PreCallValidateCmdBindVertexBuffers2EXT(VkCommandBuffer commandBuffer,
                uint32_t firstBinding,
                uint32_t bindingCount,
                const VkBuffer* pBuffers,
                const VkDeviceSize* pOffsets,
                const VkDeviceSize* pSizes,
                const VkDeviceSize* pStrides) const {
    return PreCallValidateCmdBindVertexBuffers2(commandBuffer, firstBinding, bindingCount, pBuffers, pOffsets, pSizes, pStrides);
}
bool ExplicitValidation::PreCallValidateCmdSetDepthTestEnableEXT(VkCommandBuffer commandBuffer,
                VkBool32 depthTestEnable) const {
    return PreCallValidateCmdSetDepthTestEnable(commandBuffer, depthTestEnable);
}
bool ExplicitValidation::PreCallValidateCmdSetDepthWriteEnableEXT(VkCommandBuffer commandBuffer,
                VkBool32 depthWriteEnable) const {
    return PreCallValidateCmdSetDepthWriteEnable(commandBuffer, depthWriteEnable);
}
bool ExplicitValidation::PreCallValidateCmdSetDepthCompareOpEXT(VkCommandBuffer commandBuffer,
                VkCompareOp depthCompareOp) const {
    return PreCallValidateCmdSetDepthCompareOp(commandBuffer, depthCompareOp);
}
bool ExplicitValidation::PreCallValidateCmdSetDepthBoundsTestEnableEXT(VkCommandBuffer commandBuffer,
                VkBool32 depthBoundsTestEnable) const {
    return PreCallValidateCmdSetDepthBoundsTestEnable(commandBuffer, depthBoundsTestEnable);
}
bool ExplicitValidation::PreCallValidateCmdSetStencilTestEnableEXT(VkCommandBuffer commandBuffer,
                VkBool32 stencilTestEnable) const {
    return PreCallValidateCmdSetStencilTestEnable(commandBuffer, stencilTestEnable);
}
bool ExplicitValidation::PreCallValidateCmdSetStencilOpEXT(VkCommandBuffer commandBuffer,
                VkStencilFaceFlags faceMask,
                VkStencilOp failOp,
                VkStencilOp passOp,
                VkStencilOp depthFailOp,
                VkCompareOp compareOp) const {
    return PreCallValidateCmdSetStencilOp(commandBuffer, faceMask, failOp, passOp, depthFailOp, compareOp);
}
bool ExplicitValidation::PreCallValidateReleaseSwapchainImagesEXT(VkDevice device,
                const VkReleaseSwapchainImagesInfoEXT* pReleaseInfo) const {
    bool skip = false;
    if (pReleaseInfo != nullptr) {
        const auto _s329 = pReleaseInfo;
        skip |= ValidateReleaseSwapchainImagesInfoEXT({},
            _s329->sType,
            _s329->pNext,
            _s329->swapchain,
            _s329->imageIndexCount,
            _s329->pImageIndices);
    }
    return skip;
}
bool ExplicitValidation::PreCallValidateGetGeneratedCommandsMemoryRequirementsNV(VkDevice device,
                const VkGeneratedCommandsMemoryRequirementsInfoNV* pInfo,
                VkMemoryRequirements2* pMemoryRequirements) const {
    bool skip = false;
    if (pInfo != nullptr) {
        const auto _s330 = pInfo;
        skip |= ValidateGeneratedCommandsMemoryRequirementsInfoNV({},
            _s330->sType,
            _s330->pNext,
            _s330->pipelineBindPoint,
            _s330->pipeline,
            _s330->indirectCommandsLayout,
            _s330->maxSequencesCount);
    }
    return skip;
}
bool ExplicitValidation::PreCallValidateCmdPreprocessGeneratedCommandsNV(VkCommandBuffer commandBuffer,
                const VkGeneratedCommandsInfoNV* pGeneratedCommandsInfo) const {
    bool skip = false;
    LogObjectList _carryOverObjects;
    _carryOverObjects.add(commandBuffer);
    if (pGeneratedCommandsInfo != nullptr) {
        const auto _s331 = pGeneratedCommandsInfo;
        skip |= ValidateGeneratedCommandsInfoNV(_carryOverObjects,
            _s331->sType,
            _s331->pNext,
            _s331->pipelineBindPoint,
            _s331->pipeline,
            _s331->indirectCommandsLayout,
            _s331->streamCount,
            _s331->pStreams,
            _s331->sequencesCount,
            _s331->preprocessBuffer,
            _s331->preprocessOffset,
            _s331->preprocessSize,
            _s331->sequencesCountBuffer,
            _s331->sequencesCountOffset,
            _s331->sequencesIndexBuffer,
            _s331->sequencesIndexOffset);
    }
    return skip;
}
bool ExplicitValidation::PreCallValidateCmdExecuteGeneratedCommandsNV(VkCommandBuffer commandBuffer,
                VkBool32 isPreprocessed,
                const VkGeneratedCommandsInfoNV* pGeneratedCommandsInfo) const {
    bool skip = false;
    LogObjectList _carryOverObjects;
    _carryOverObjects.add(commandBuffer);
    if (pGeneratedCommandsInfo != nullptr) {
        const auto _s332 = pGeneratedCommandsInfo;
        skip |= ValidateGeneratedCommandsInfoNV(_carryOverObjects,
            _s332->sType,
            _s332->pNext,
            _s332->pipelineBindPoint,
            _s332->pipeline,
            _s332->indirectCommandsLayout,
            _s332->streamCount,
            _s332->pStreams,
            _s332->sequencesCount,
            _s332->preprocessBuffer,
            _s332->preprocessOffset,
            _s332->preprocessSize,
            _s332->sequencesCountBuffer,
            _s332->sequencesCountOffset,
            _s332->sequencesIndexBuffer,
            _s332->sequencesIndexOffset);
    }
    return skip;
}
bool ExplicitValidation::PreCallValidateCmdBindPipelineShaderGroupNV(VkCommandBuffer commandBuffer,
                VkPipelineBindPoint pipelineBindPoint,
                VkPipeline pipeline,
                uint32_t groupIndex) const {
    bool skip = false;
    LogObjectList _carryOverObjects;
    _carryOverObjects.add(commandBuffer);
    return skip;
}
bool ExplicitValidation::PreCallValidateCreateIndirectCommandsLayoutNV(VkDevice device,
                const VkIndirectCommandsLayoutCreateInfoNV* pCreateInfo,
                const VkAllocationCallbacks* pAllocator,
                VkIndirectCommandsLayoutNV* pIndirectCommandsLayout) const {
    bool skip = false;
    if (pCreateInfo != nullptr) {
        const auto _s333 = pCreateInfo;
        skip |= ValidateIndirectCommandsLayoutCreateInfoNV({},
            _s333->sType,
            _s333->pNext,
            _s333->flags,
            _s333->pipelineBindPoint,
            _s333->tokenCount,
            _s333->pTokens,
            _s333->streamCount,
            _s333->pStreamStrides);
    }
    if (pAllocator != nullptr) {
        const auto _s334 = pAllocator;
        skip |= ValidateAllocationCallbacks({},
            _s334->pUserData,
            _s334->pfnAllocation,
            _s334->pfnReallocation,
            _s334->pfnFree,
            _s334->pfnInternalAllocation,
            _s334->pfnInternalFree);
    }
    return skip;
}
bool ExplicitValidation::PreCallValidateDestroyIndirectCommandsLayoutNV(VkDevice device,
                VkIndirectCommandsLayoutNV indirectCommandsLayout,
                const VkAllocationCallbacks* pAllocator) const {
    bool skip = false;
    if (pAllocator != nullptr) {
        const auto _s335 = pAllocator;
        skip |= ValidateAllocationCallbacks({},
            _s335->pUserData,
            _s335->pfnAllocation,
            _s335->pfnReallocation,
            _s335->pfnFree,
            _s335->pfnInternalAllocation,
            _s335->pfnInternalFree);
    }
    return skip;
}
bool ExplicitValidation::PreCallValidateAcquireDrmDisplayEXT(VkPhysicalDevice physicalDevice,
                int32_t drmFd,
                VkDisplayKHR display) const {
    bool skip = false;
    return skip;
}
bool ExplicitValidation::PreCallValidateGetDrmDisplayEXT(VkPhysicalDevice physicalDevice,
                int32_t drmFd,
                uint32_t connectorId,
                VkDisplayKHR* display) const {
    bool skip = false;
    return skip;
}
bool ExplicitValidation::PreCallValidateCreatePrivateDataSlotEXT(VkDevice device,
                const VkPrivateDataSlotCreateInfo* pCreateInfo,
                const VkAllocationCallbacks* pAllocator,
                VkPrivateDataSlot* pPrivateDataSlot) const {
    return PreCallValidateCreatePrivateDataSlot(device, pCreateInfo, pAllocator, pPrivateDataSlot);
}
bool ExplicitValidation::PreCallValidateDestroyPrivateDataSlotEXT(VkDevice device,
                VkPrivateDataSlot privateDataSlot,
                const VkAllocationCallbacks* pAllocator) const {
    return PreCallValidateDestroyPrivateDataSlot(device, privateDataSlot, pAllocator);
}
bool ExplicitValidation::PreCallValidateSetPrivateDataEXT(VkDevice device,
                VkObjectType objectType,
                uint64_t objectHandle,
                VkPrivateDataSlot privateDataSlot,
                uint64_t data) const {
    return PreCallValidateSetPrivateData(device, objectType, objectHandle, privateDataSlot, data);
}
bool ExplicitValidation::PreCallValidateGetPrivateDataEXT(VkDevice device,
                VkObjectType objectType,
                uint64_t objectHandle,
                VkPrivateDataSlot privateDataSlot,
                uint64_t* pData) const {
    return PreCallValidateGetPrivateData(device, objectType, objectHandle, privateDataSlot, pData);
}
#ifdef VK_USE_PLATFORM_METAL_EXT
bool ExplicitValidation::PreCallValidateExportMetalObjectsEXT(VkDevice device,
                VkExportMetalObjectsInfoEXT* pMetalObjectsInfo) const {
    bool skip = false;
    if (pMetalObjectsInfo != nullptr) {
        const auto _s336 = pMetalObjectsInfo;
        skip |= ValidateExportMetalObjectsInfoEXT({},
            _s336->sType,
            _s336->pNext);
    }
    return skip;
}
#endif
bool ExplicitValidation::PreCallValidateGetDescriptorSetLayoutSizeEXT(VkDevice device,
                VkDescriptorSetLayout layout,
                VkDeviceSize* pLayoutSizeInBytes) const {
    bool skip = false;
    return skip;
}
bool ExplicitValidation::PreCallValidateGetDescriptorSetLayoutBindingOffsetEXT(VkDevice device,
                VkDescriptorSetLayout layout,
                uint32_t binding,
                VkDeviceSize* pOffset) const {
    bool skip = false;
    return skip;
}
bool ExplicitValidation::PreCallValidateGetDescriptorEXT(VkDevice device,
                const VkDescriptorGetInfoEXT* pDescriptorInfo,
                size_t dataSize,
                void* pDescriptor) const {
    bool skip = false;
    if (pDescriptorInfo != nullptr) {
        const auto _s337 = pDescriptorInfo;
        skip |= ValidateDescriptorGetInfoEXT({},
            _s337->sType,
            _s337->pNext,
            _s337->type,
            _s337->data);
    }
    return skip;
}
bool ExplicitValidation::PreCallValidateCmdBindDescriptorBuffersEXT(VkCommandBuffer commandBuffer,
                uint32_t bufferCount,
                const VkDescriptorBufferBindingInfoEXT* pBindingInfos) const {
    bool skip = false;
    LogObjectList _carryOverObjects;
    _carryOverObjects.add(commandBuffer);
    if (pBindingInfos != nullptr) {
        for (uint32_t _i338 = 0;_i338 < bufferCount; ++_i338) {
            const auto _s339 = &pBindingInfos[_i338];
            skip |= ValidateDescriptorBufferBindingInfoEXT(_carryOverObjects,
                _s339->sType,
                _s339->pNext,
                _s339->address,
                _s339->usage);
        }
    }
    return skip;
}
bool ExplicitValidation::PreCallValidateCmdSetDescriptorBufferOffsetsEXT(VkCommandBuffer commandBuffer,
                VkPipelineBindPoint pipelineBindPoint,
                VkPipelineLayout layout,
                uint32_t firstSet,
                uint32_t setCount,
                const uint32_t* pBufferIndices,
                const VkDeviceSize* pOffsets) const {
    bool skip = false;
    LogObjectList _carryOverObjects;
    _carryOverObjects.add(commandBuffer);
    return skip;
}
bool ExplicitValidation::PreCallValidateCmdBindDescriptorBufferEmbeddedSamplersEXT(VkCommandBuffer commandBuffer,
                VkPipelineBindPoint pipelineBindPoint,
                VkPipelineLayout layout,
                uint32_t set) const {
    bool skip = false;
    LogObjectList _carryOverObjects;
    _carryOverObjects.add(commandBuffer);
    return skip;
}
bool ExplicitValidation::PreCallValidateGetBufferOpaqueCaptureDescriptorDataEXT(VkDevice device,
                const VkBufferCaptureDescriptorDataInfoEXT* pInfo,
                void* pData) const {
    bool skip = false;
    if (pInfo != nullptr) {
        const auto _s340 = pInfo;
        skip |= ValidateBufferCaptureDescriptorDataInfoEXT({},
            _s340->sType,
            _s340->pNext,
            _s340->buffer);
    }
    return skip;
}
bool ExplicitValidation::PreCallValidateGetImageOpaqueCaptureDescriptorDataEXT(VkDevice device,
                const VkImageCaptureDescriptorDataInfoEXT* pInfo,
                void* pData) const {
    bool skip = false;
    if (pInfo != nullptr) {
        const auto _s341 = pInfo;
        skip |= ValidateImageCaptureDescriptorDataInfoEXT({},
            _s341->sType,
            _s341->pNext,
            _s341->image);
    }
    return skip;
}
bool ExplicitValidation::PreCallValidateGetImageViewOpaqueCaptureDescriptorDataEXT(VkDevice device,
                const VkImageViewCaptureDescriptorDataInfoEXT* pInfo,
                void* pData) const {
    bool skip = false;
    if (pInfo != nullptr) {
        const auto _s342 = pInfo;
        skip |= ValidateImageViewCaptureDescriptorDataInfoEXT({},
            _s342->sType,
            _s342->pNext,
            _s342->imageView);
    }
    return skip;
}
bool ExplicitValidation::PreCallValidateGetSamplerOpaqueCaptureDescriptorDataEXT(VkDevice device,
                const VkSamplerCaptureDescriptorDataInfoEXT* pInfo,
                void* pData) const {
    bool skip = false;
    if (pInfo != nullptr) {
        const auto _s343 = pInfo;
        skip |= ValidateSamplerCaptureDescriptorDataInfoEXT({},
            _s343->sType,
            _s343->pNext,
            _s343->sampler);
    }
    return skip;
}
bool ExplicitValidation::PreCallValidateGetAccelerationStructureOpaqueCaptureDescriptorDataEXT(VkDevice device,
                const VkAccelerationStructureCaptureDescriptorDataInfoEXT* pInfo,
                void* pData) const {
    bool skip = false;
    if (pInfo != nullptr) {
        const auto _s344 = pInfo;
        skip |= ValidateAccelerationStructureCaptureDescriptorDataInfoEXT({},
            _s344->sType,
            _s344->pNext,
            _s344->accelerationStructure,
            _s344->accelerationStructureNV);
    }
    return skip;
}
bool ExplicitValidation::PreCallValidateCmdSetFragmentShadingRateEnumNV(VkCommandBuffer commandBuffer,
                VkFragmentShadingRateNV shadingRate,
                const VkFragmentShadingRateCombinerOpKHR combinerOps[2]) const {
    bool skip = false;
    LogObjectList _carryOverObjects;
    _carryOverObjects.add(commandBuffer);
    return skip;
}
bool ExplicitValidation::PreCallValidateGetImageSubresourceLayout2EXT(VkDevice device,
                VkImage image,
                const VkImageSubresource2EXT* pSubresource,
                VkSubresourceLayout2EXT* pLayout) const {
    bool skip = false;
    if (pSubresource != nullptr) {
        const auto _s345 = pSubresource;
        skip |= ValidateImageSubresource2EXT({},
            _s345->sType,
            _s345->pNext,
            _s345->imageSubresource);
    }
    return skip;
}
bool ExplicitValidation::PreCallValidateGetDeviceFaultInfoEXT(VkDevice device,
                VkDeviceFaultCountsEXT* pFaultCounts,
                VkDeviceFaultInfoEXT* pFaultInfo) const {
    bool skip = false;
    if (pFaultCounts != nullptr) {
        const auto _s346 = pFaultCounts;
        skip |= ValidateDeviceFaultCountsEXT({},
            _s346->sType,
            _s346->pNext,
            _s346->addressInfoCount,
            _s346->vendorInfoCount,
            _s346->vendorBinarySize);
    }
    if (pFaultInfo != nullptr) {
        const auto _s347 = pFaultInfo;
        skip |= ValidateDeviceFaultInfoEXT({},
            _s347->sType,
            _s347->pNext,
            _s347->description,
            _s347->pAddressInfos,
            _s347->pVendorInfos,
            _s347->pVendorBinaryData);
    }
    return skip;
}
#ifdef VK_USE_PLATFORM_WIN32_KHR
bool ExplicitValidation::PreCallValidateAcquireWinrtDisplayNV(VkPhysicalDevice physicalDevice,
                VkDisplayKHR display) const {
    bool skip = false;
    return skip;
}
#endif
#ifdef VK_USE_PLATFORM_WIN32_KHR
bool ExplicitValidation::PreCallValidateGetWinrtDisplayNV(VkPhysicalDevice physicalDevice,
                uint32_t deviceRelativeId,
                VkDisplayKHR* pDisplay) const {
    bool skip = false;
    return skip;
}
#endif
#ifdef VK_USE_PLATFORM_DIRECTFB_EXT
bool ExplicitValidation::PreCallValidateCreateDirectFBSurfaceEXT(VkInstance instance,
                const VkDirectFBSurfaceCreateInfoEXT* pCreateInfo,
                const VkAllocationCallbacks* pAllocator,
                VkSurfaceKHR* pSurface) const {
    bool skip = false;
    if (pCreateInfo != nullptr) {
        const auto _s348 = pCreateInfo;
        skip |= ValidateDirectFBSurfaceCreateInfoEXT({},
            _s348->sType,
            _s348->pNext,
            _s348->flags,
            _s348->dfb,
            _s348->surface);
    }
    if (pAllocator != nullptr) {
        const auto _s349 = pAllocator;
        skip |= ValidateAllocationCallbacks({},
            _s349->pUserData,
            _s349->pfnAllocation,
            _s349->pfnReallocation,
            _s349->pfnFree,
            _s349->pfnInternalAllocation,
            _s349->pfnInternalFree);
    }
    return skip;
}
#endif
#ifdef VK_USE_PLATFORM_DIRECTFB_EXT
bool ExplicitValidation::PreCallValidateGetPhysicalDeviceDirectFBPresentationSupportEXT(VkPhysicalDevice physicalDevice,
                uint32_t queueFamilyIndex,
                IDirectFB* dfb) const {
    bool skip = false;
    return skip;
}
#endif
bool ExplicitValidation::PreCallValidateCmdSetVertexInputEXT(VkCommandBuffer commandBuffer,
                uint32_t vertexBindingDescriptionCount,
                const VkVertexInputBindingDescription2EXT* pVertexBindingDescriptions,
                uint32_t vertexAttributeDescriptionCount,
                const VkVertexInputAttributeDescription2EXT* pVertexAttributeDescriptions) const {
    bool skip = false;
    LogObjectList _carryOverObjects;
    _carryOverObjects.add(commandBuffer);
    if (pVertexBindingDescriptions != nullptr) {
        for (uint32_t _i350 = 0;_i350 < vertexBindingDescriptionCount; ++_i350) {
            const auto _s351 = &pVertexBindingDescriptions[_i350];
            skip |= ValidateVertexInputBindingDescription2EXT(_carryOverObjects,
                _s351->sType,
                _s351->pNext,
                _s351->binding,
                _s351->stride,
                _s351->inputRate,
                _s351->divisor);
        }
    }
    if (pVertexAttributeDescriptions != nullptr) {
        for (uint32_t _i352 = 0;_i352 < vertexAttributeDescriptionCount; ++_i352) {
            const auto _s353 = &pVertexAttributeDescriptions[_i352];
            skip |= ValidateVertexInputAttributeDescription2EXT(_carryOverObjects,
                _s353->sType,
                _s353->pNext,
                _s353->location,
                _s353->binding,
                _s353->format,
                _s353->offset);
        }
    }
    return skip;
}
#ifdef VK_USE_PLATFORM_FUCHSIA
bool ExplicitValidation::PreCallValidateGetMemoryZirconHandleFUCHSIA(VkDevice device,
                const VkMemoryGetZirconHandleInfoFUCHSIA* pGetZirconHandleInfo,
                zx_handle_t* pZirconHandle) const {
    bool skip = false;
    if (pGetZirconHandleInfo != nullptr) {
        const auto _s354 = pGetZirconHandleInfo;
        skip |= ValidateMemoryGetZirconHandleInfoFUCHSIA({},
            _s354->sType,
            _s354->pNext,
            _s354->memory,
            _s354->handleType);
    }
    return skip;
}
#endif
#ifdef VK_USE_PLATFORM_FUCHSIA
bool ExplicitValidation::PreCallValidateGetMemoryZirconHandlePropertiesFUCHSIA(VkDevice device,
                VkExternalMemoryHandleTypeFlagBits handleType,
                zx_handle_t zirconHandle,
                VkMemoryZirconHandlePropertiesFUCHSIA* pMemoryZirconHandleProperties) const {
    bool skip = false;
    return skip;
}
#endif
#ifdef VK_USE_PLATFORM_FUCHSIA
bool ExplicitValidation::PreCallValidateImportSemaphoreZirconHandleFUCHSIA(VkDevice device,
                const VkImportSemaphoreZirconHandleInfoFUCHSIA* pImportSemaphoreZirconHandleInfo) const {
    bool skip = false;
    if (pImportSemaphoreZirconHandleInfo != nullptr) {
        const auto _s355 = pImportSemaphoreZirconHandleInfo;
        skip |= ValidateImportSemaphoreZirconHandleInfoFUCHSIA({},
            _s355->sType,
            _s355->pNext,
            _s355->semaphore,
            _s355->flags,
            _s355->handleType,
            _s355->zirconHandle);
    }
    return skip;
}
#endif
#ifdef VK_USE_PLATFORM_FUCHSIA
bool ExplicitValidation::PreCallValidateGetSemaphoreZirconHandleFUCHSIA(VkDevice device,
                const VkSemaphoreGetZirconHandleInfoFUCHSIA* pGetZirconHandleInfo,
                zx_handle_t* pZirconHandle) const {
    bool skip = false;
    if (pGetZirconHandleInfo != nullptr) {
        const auto _s356 = pGetZirconHandleInfo;
        skip |= ValidateSemaphoreGetZirconHandleInfoFUCHSIA({},
            _s356->sType,
            _s356->pNext,
            _s356->semaphore,
            _s356->handleType);
    }
    return skip;
}
#endif
#ifdef VK_USE_PLATFORM_FUCHSIA
bool ExplicitValidation::PreCallValidateCreateBufferCollectionFUCHSIA(VkDevice device,
                const VkBufferCollectionCreateInfoFUCHSIA* pCreateInfo,
                const VkAllocationCallbacks* pAllocator,
                VkBufferCollectionFUCHSIA* pCollection) const {
    bool skip = false;
    if (pCreateInfo != nullptr) {
        const auto _s357 = pCreateInfo;
        skip |= ValidateBufferCollectionCreateInfoFUCHSIA({},
            _s357->sType,
            _s357->pNext,
            _s357->collectionToken);
    }
    if (pAllocator != nullptr) {
        const auto _s358 = pAllocator;
        skip |= ValidateAllocationCallbacks({},
            _s358->pUserData,
            _s358->pfnAllocation,
            _s358->pfnReallocation,
            _s358->pfnFree,
            _s358->pfnInternalAllocation,
            _s358->pfnInternalFree);
    }
    return skip;
}
#endif
#ifdef VK_USE_PLATFORM_FUCHSIA
bool ExplicitValidation::PreCallValidateSetBufferCollectionImageConstraintsFUCHSIA(VkDevice device,
                VkBufferCollectionFUCHSIA collection,
                const VkImageConstraintsInfoFUCHSIA* pImageConstraintsInfo) const {
    bool skip = false;
    if (pImageConstraintsInfo != nullptr) {
        const auto _s359 = pImageConstraintsInfo;
        skip |= ValidateImageConstraintsInfoFUCHSIA({},
            _s359->sType,
            _s359->pNext,
            _s359->formatConstraintsCount,
            _s359->pFormatConstraints,
            _s359->bufferCollectionConstraints,
            _s359->flags);
    }
    return skip;
}
#endif
#ifdef VK_USE_PLATFORM_FUCHSIA
bool ExplicitValidation::PreCallValidateSetBufferCollectionBufferConstraintsFUCHSIA(VkDevice device,
                VkBufferCollectionFUCHSIA collection,
                const VkBufferConstraintsInfoFUCHSIA* pBufferConstraintsInfo) const {
    bool skip = false;
    if (pBufferConstraintsInfo != nullptr) {
        const auto _s360 = pBufferConstraintsInfo;
        skip |= ValidateBufferConstraintsInfoFUCHSIA({},
            _s360->sType,
            _s360->pNext,
            _s360->createInfo,
            _s360->requiredFormatFeatures,
            _s360->bufferCollectionConstraints);
    }
    return skip;
}
#endif
#ifdef VK_USE_PLATFORM_FUCHSIA
bool ExplicitValidation::PreCallValidateDestroyBufferCollectionFUCHSIA(VkDevice device,
                VkBufferCollectionFUCHSIA collection,
                const VkAllocationCallbacks* pAllocator) const {
    bool skip = false;
    if (pAllocator != nullptr) {
        const auto _s361 = pAllocator;
        skip |= ValidateAllocationCallbacks({},
            _s361->pUserData,
            _s361->pfnAllocation,
            _s361->pfnReallocation,
            _s361->pfnFree,
            _s361->pfnInternalAllocation,
            _s361->pfnInternalFree);
    }
    return skip;
}
#endif
#ifdef VK_USE_PLATFORM_FUCHSIA
bool ExplicitValidation::PreCallValidateGetBufferCollectionPropertiesFUCHSIA(VkDevice device,
                VkBufferCollectionFUCHSIA collection,
                VkBufferCollectionPropertiesFUCHSIA* pProperties) const {
    bool skip = false;
    if (pProperties != nullptr) {
        const auto _s362 = pProperties;
        skip |= ValidateBufferCollectionPropertiesFUCHSIA({},
            _s362->sType,
            _s362->pNext,
            _s362->memoryTypeBits,
            _s362->bufferCount,
            _s362->createInfoIndex,
            _s362->sysmemPixelFormat,
            _s362->formatFeatures,
            _s362->sysmemColorSpaceIndex,
            _s362->samplerYcbcrConversionComponents,
            _s362->suggestedYcbcrModel,
            _s362->suggestedYcbcrRange,
            _s362->suggestedXChromaOffset,
            _s362->suggestedYChromaOffset);
    }
    return skip;
}
#endif
bool ExplicitValidation::PreCallValidateGetDeviceSubpassShadingMaxWorkgroupSizeHUAWEI(VkDevice device,
                VkRenderPass renderpass,
                VkExtent2D* pMaxWorkgroupSize) const {
    bool skip = false;
    if (pMaxWorkgroupSize != nullptr) {
        const auto _s363 = pMaxWorkgroupSize;
        skip |= ValidateExtent2D({},
            _s363->width,
            _s363->height);
    }
    return skip;
}
bool ExplicitValidation::PreCallValidateCmdSubpassShadingHUAWEI(VkCommandBuffer commandBuffer) const {
    bool skip = false;
    LogObjectList _carryOverObjects;
    _carryOverObjects.add(commandBuffer);
    return skip;
}
bool ExplicitValidation::PreCallValidateCmdBindInvocationMaskHUAWEI(VkCommandBuffer commandBuffer,
                VkImageView imageView,
                VkImageLayout imageLayout) const {
    bool skip = false;
    LogObjectList _carryOverObjects;
    _carryOverObjects.add(commandBuffer);
    return skip;
}
bool ExplicitValidation::PreCallValidateGetMemoryRemoteAddressNV(VkDevice device,
                const VkMemoryGetRemoteAddressInfoNV* pMemoryGetRemoteAddressInfo,
                VkRemoteAddressNV* pAddress) const {
    bool skip = false;
    if (pMemoryGetRemoteAddressInfo != nullptr) {
        const auto _s364 = pMemoryGetRemoteAddressInfo;
        skip |= ValidateMemoryGetRemoteAddressInfoNV({},
            _s364->sType,
            _s364->pNext,
            _s364->memory,
            _s364->handleType);
    }
    return skip;
}
bool ExplicitValidation::PreCallValidateGetPipelinePropertiesEXT(VkDevice device,
                const VkPipelineInfoEXT* pPipelineInfo,
                VkBaseOutStructure* pPipelineProperties) const {
    bool skip = false;
    if (pPipelineInfo != nullptr) {
        const auto _s365 = pPipelineInfo;
        skip |= ValidatePipelineInfoKHR({},
            _s365->sType,
            _s365->pNext,
            _s365->pipeline);
    }
    if (pPipelineProperties != nullptr) {
        const auto _s366 = pPipelineProperties;
        skip |= ValidateBaseOutStructure({},
            _s366->sType,
            _s366->pNext);
    }
    return skip;
}
bool ExplicitValidation::PreCallValidateCmdSetPatchControlPointsEXT(VkCommandBuffer commandBuffer,
                uint32_t patchControlPoints) const {
    bool skip = false;
    LogObjectList _carryOverObjects;
    _carryOverObjects.add(commandBuffer);
    return skip;
}
bool ExplicitValidation::PreCallValidateCmdSetRasterizerDiscardEnableEXT(VkCommandBuffer commandBuffer,
                VkBool32 rasterizerDiscardEnable) const {
    return PreCallValidateCmdSetRasterizerDiscardEnable(commandBuffer, rasterizerDiscardEnable);
}
bool ExplicitValidation::PreCallValidateCmdSetDepthBiasEnableEXT(VkCommandBuffer commandBuffer,
                VkBool32 depthBiasEnable) const {
    return PreCallValidateCmdSetDepthBiasEnable(commandBuffer, depthBiasEnable);
}
bool ExplicitValidation::PreCallValidateCmdSetLogicOpEXT(VkCommandBuffer commandBuffer,
                VkLogicOp logicOp) const {
    bool skip = false;
    LogObjectList _carryOverObjects;
    _carryOverObjects.add(commandBuffer);
    return skip;
}
bool ExplicitValidation::PreCallValidateCmdSetPrimitiveRestartEnableEXT(VkCommandBuffer commandBuffer,
                VkBool32 primitiveRestartEnable) const {
    return PreCallValidateCmdSetPrimitiveRestartEnable(commandBuffer, primitiveRestartEnable);
}
#ifdef VK_USE_PLATFORM_SCREEN_QNX
bool ExplicitValidation::PreCallValidateCreateScreenSurfaceQNX(VkInstance instance,
                const VkScreenSurfaceCreateInfoQNX* pCreateInfo,
                const VkAllocationCallbacks* pAllocator,
                VkSurfaceKHR* pSurface) const {
    bool skip = false;
    if (pCreateInfo != nullptr) {
        const auto _s367 = pCreateInfo;
        skip |= ValidateScreenSurfaceCreateInfoQNX({},
            _s367->sType,
            _s367->pNext,
            _s367->flags,
            _s367->context,
            _s367->window);
    }
    if (pAllocator != nullptr) {
        const auto _s368 = pAllocator;
        skip |= ValidateAllocationCallbacks({},
            _s368->pUserData,
            _s368->pfnAllocation,
            _s368->pfnReallocation,
            _s368->pfnFree,
            _s368->pfnInternalAllocation,
            _s368->pfnInternalFree);
    }
    return skip;
}
#endif
#ifdef VK_USE_PLATFORM_SCREEN_QNX
bool ExplicitValidation::PreCallValidateGetPhysicalDeviceScreenPresentationSupportQNX(VkPhysicalDevice physicalDevice,
                uint32_t queueFamilyIndex,
                struct _screen_window* window) const {
    bool skip = false;
    return skip;
}
#endif
bool ExplicitValidation::PreCallValidateCmdSetColorWriteEnableEXT(VkCommandBuffer commandBuffer,
                uint32_t attachmentCount,
                const VkBool32* pColorWriteEnables) const {
    bool skip = false;
    LogObjectList _carryOverObjects;
    _carryOverObjects.add(commandBuffer);
    return skip;
}
bool ExplicitValidation::PreCallValidateCmdDrawMultiEXT(VkCommandBuffer commandBuffer,
                uint32_t drawCount,
                const VkMultiDrawInfoEXT* pVertexInfo,
                uint32_t instanceCount,
                uint32_t firstInstance,
                uint32_t stride) const {
    bool skip = false;
    LogObjectList _carryOverObjects;
    _carryOverObjects.add(commandBuffer);
    if (pVertexInfo != nullptr) {
        for (uint32_t _i369 = 0;_i369 < drawCount; ++_i369) {
            const auto _s370 = &pVertexInfo[_i369];
            skip |= ValidateMultiDrawInfoEXT(_carryOverObjects,
                _s370->firstVertex,
                _s370->vertexCount);
        }
    }
    return skip;
}
bool ExplicitValidation::PreCallValidateCmdDrawMultiIndexedEXT(VkCommandBuffer commandBuffer,
                uint32_t drawCount,
                const VkMultiDrawIndexedInfoEXT* pIndexInfo,
                uint32_t instanceCount,
                uint32_t firstInstance,
                uint32_t stride,
                const int32_t* pVertexOffset) const {
    bool skip = false;
    LogObjectList _carryOverObjects;
    _carryOverObjects.add(commandBuffer);
    if (pIndexInfo != nullptr) {
        for (uint32_t _i371 = 0;_i371 < drawCount; ++_i371) {
            const auto _s372 = &pIndexInfo[_i371];
            skip |= ValidateMultiDrawIndexedInfoEXT(_carryOverObjects,
                _s372->firstIndex,
                _s372->indexCount,
                _s372->vertexOffset);
        }
    }
    return skip;
}
bool ExplicitValidation::PreCallValidateCreateMicromapEXT(VkDevice device,
                const VkMicromapCreateInfoEXT* pCreateInfo,
                const VkAllocationCallbacks* pAllocator,
                VkMicromapEXT* pMicromap) const {
    bool skip = false;
    if (pCreateInfo != nullptr) {
        const auto _s373 = pCreateInfo;
        skip |= ValidateMicromapCreateInfoEXT({},
            _s373->sType,
            _s373->pNext,
            _s373->createFlags,
            _s373->buffer,
            _s373->offset,
            _s373->size,
            _s373->type,
            _s373->deviceAddress);
    }
    if (pAllocator != nullptr) {
        const auto _s374 = pAllocator;
        skip |= ValidateAllocationCallbacks({},
            _s374->pUserData,
            _s374->pfnAllocation,
            _s374->pfnReallocation,
            _s374->pfnFree,
            _s374->pfnInternalAllocation,
            _s374->pfnInternalFree);
    }
    return skip;
}
bool ExplicitValidation::PreCallValidateDestroyMicromapEXT(VkDevice device,
                VkMicromapEXT micromap,
                const VkAllocationCallbacks* pAllocator) const {
    bool skip = false;
    if (pAllocator != nullptr) {
        const auto _s375 = pAllocator;
        skip |= ValidateAllocationCallbacks({},
            _s375->pUserData,
            _s375->pfnAllocation,
            _s375->pfnReallocation,
            _s375->pfnFree,
            _s375->pfnInternalAllocation,
            _s375->pfnInternalFree);
    }
    return skip;
}
bool ExplicitValidation::PreCallValidateCmdBuildMicromapsEXT(VkCommandBuffer commandBuffer,
                uint32_t infoCount,
                const VkMicromapBuildInfoEXT* pInfos) const {
    bool skip = false;
    LogObjectList _carryOverObjects;
    _carryOverObjects.add(commandBuffer);
    if (pInfos != nullptr) {
        for (uint32_t _i376 = 0;_i376 < infoCount; ++_i376) {
            const auto _s377 = &pInfos[_i376];
            skip |= ValidateMicromapBuildInfoEXT(_carryOverObjects,
                _s377->sType,
                _s377->pNext,
                _s377->type,
                _s377->flags,
                _s377->mode,
                _s377->dstMicromap,
                _s377->usageCountsCount,
                _s377->pUsageCounts,
                _s377->ppUsageCounts,
                _s377->data,
                _s377->scratchData,
                _s377->triangleArray,
                _s377->triangleArrayStride);
        }
    }
    return skip;
}
bool ExplicitValidation::PreCallValidateBuildMicromapsEXT(VkDevice device,
                VkDeferredOperationKHR deferredOperation,
                uint32_t infoCount,
                const VkMicromapBuildInfoEXT* pInfos) const {
    bool skip = false;
    if (pInfos != nullptr) {
        for (uint32_t _i378 = 0;_i378 < infoCount; ++_i378) {
            const auto _s379 = &pInfos[_i378];
            skip |= ValidateMicromapBuildInfoEXT({},
                _s379->sType,
                _s379->pNext,
                _s379->type,
                _s379->flags,
                _s379->mode,
                _s379->dstMicromap,
                _s379->usageCountsCount,
                _s379->pUsageCounts,
                _s379->ppUsageCounts,
                _s379->data,
                _s379->scratchData,
                _s379->triangleArray,
                _s379->triangleArrayStride);
        }
    }
    return skip;
}
bool ExplicitValidation::PreCallValidateCopyMicromapEXT(VkDevice device,
                VkDeferredOperationKHR deferredOperation,
                const VkCopyMicromapInfoEXT* pInfo) const {
    bool skip = false;
    if (pInfo != nullptr) {
        const auto _s380 = pInfo;
        skip |= ValidateCopyMicromapInfoEXT({},
            _s380->sType,
            _s380->pNext,
            _s380->src,
            _s380->dst,
            _s380->mode);
    }
    return skip;
}
bool ExplicitValidation::PreCallValidateCopyMicromapToMemoryEXT(VkDevice device,
                VkDeferredOperationKHR deferredOperation,
                const VkCopyMicromapToMemoryInfoEXT* pInfo) const {
    bool skip = false;
    if (pInfo != nullptr) {
        const auto _s381 = pInfo;
        skip |= ValidateCopyMicromapToMemoryInfoEXT({},
            _s381->sType,
            _s381->pNext,
            _s381->src,
            _s381->dst,
            _s381->mode);
    }
    return skip;
}
bool ExplicitValidation::PreCallValidateCopyMemoryToMicromapEXT(VkDevice device,
                VkDeferredOperationKHR deferredOperation,
                const VkCopyMemoryToMicromapInfoEXT* pInfo) const {
    bool skip = false;
    if (pInfo != nullptr) {
        const auto _s382 = pInfo;
        skip |= ValidateCopyMemoryToMicromapInfoEXT({},
            _s382->sType,
            _s382->pNext,
            _s382->src,
            _s382->dst,
            _s382->mode);
    }
    return skip;
}
bool ExplicitValidation::PreCallValidateWriteMicromapsPropertiesEXT(VkDevice device,
                uint32_t micromapCount,
                const VkMicromapEXT* pMicromaps,
                VkQueryType queryType,
                size_t dataSize,
                void* pData,
                size_t stride) const {
    bool skip = false;
    return skip;
}
bool ExplicitValidation::PreCallValidateCmdCopyMicromapEXT(VkCommandBuffer commandBuffer,
                const VkCopyMicromapInfoEXT* pInfo) const {
    bool skip = false;
    LogObjectList _carryOverObjects;
    _carryOverObjects.add(commandBuffer);
    if (pInfo != nullptr) {
        const auto _s383 = pInfo;
        skip |= ValidateCopyMicromapInfoEXT(_carryOverObjects,
            _s383->sType,
            _s383->pNext,
            _s383->src,
            _s383->dst,
            _s383->mode);
    }
    return skip;
}
bool ExplicitValidation::PreCallValidateCmdCopyMicromapToMemoryEXT(VkCommandBuffer commandBuffer,
                const VkCopyMicromapToMemoryInfoEXT* pInfo) const {
    bool skip = false;
    LogObjectList _carryOverObjects;
    _carryOverObjects.add(commandBuffer);
    if (pInfo != nullptr) {
        const auto _s384 = pInfo;
        skip |= ValidateCopyMicromapToMemoryInfoEXT(_carryOverObjects,
            _s384->sType,
            _s384->pNext,
            _s384->src,
            _s384->dst,
            _s384->mode);
    }
    return skip;
}
bool ExplicitValidation::PreCallValidateCmdCopyMemoryToMicromapEXT(VkCommandBuffer commandBuffer,
                const VkCopyMemoryToMicromapInfoEXT* pInfo) const {
    bool skip = false;
    LogObjectList _carryOverObjects;
    _carryOverObjects.add(commandBuffer);
    if (pInfo != nullptr) {
        const auto _s385 = pInfo;
        skip |= ValidateCopyMemoryToMicromapInfoEXT(_carryOverObjects,
            _s385->sType,
            _s385->pNext,
            _s385->src,
            _s385->dst,
            _s385->mode);
    }
    return skip;
}
bool ExplicitValidation::PreCallValidateCmdWriteMicromapsPropertiesEXT(VkCommandBuffer commandBuffer,
                uint32_t micromapCount,
                const VkMicromapEXT* pMicromaps,
                VkQueryType queryType,
                VkQueryPool queryPool,
                uint32_t firstQuery) const {
    bool skip = false;
    LogObjectList _carryOverObjects;
    _carryOverObjects.add(commandBuffer);
    return skip;
}
bool ExplicitValidation::PreCallValidateGetDeviceMicromapCompatibilityEXT(VkDevice device,
                const VkMicromapVersionInfoEXT* pVersionInfo,
                VkAccelerationStructureCompatibilityKHR* pCompatibility) const {
    bool skip = false;
    if (pVersionInfo != nullptr) {
        const auto _s386 = pVersionInfo;
        skip |= ValidateMicromapVersionInfoEXT({},
            _s386->sType,
            _s386->pNext,
            _s386->pVersionData);
    }
    return skip;
}
bool ExplicitValidation::PreCallValidateGetMicromapBuildSizesEXT(VkDevice device,
                VkAccelerationStructureBuildTypeKHR buildType,
                const VkMicromapBuildInfoEXT* pBuildInfo,
                VkMicromapBuildSizesInfoEXT* pSizeInfo) const {
    bool skip = false;
    if (pBuildInfo != nullptr) {
        const auto _s387 = pBuildInfo;
        skip |= ValidateMicromapBuildInfoEXT({},
            _s387->sType,
            _s387->pNext,
            _s387->type,
            _s387->flags,
            _s387->mode,
            _s387->dstMicromap,
            _s387->usageCountsCount,
            _s387->pUsageCounts,
            _s387->ppUsageCounts,
            _s387->data,
            _s387->scratchData,
            _s387->triangleArray,
            _s387->triangleArrayStride);
    }
    if (pSizeInfo != nullptr) {
        const auto _s388 = pSizeInfo;
        skip |= ValidateMicromapBuildSizesInfoEXT({},
            _s388->sType,
            _s388->pNext,
            _s388->micromapSize,
            _s388->buildScratchSize,
            _s388->discardable);
    }
    return skip;
}
bool ExplicitValidation::PreCallValidateCmdDrawClusterHUAWEI(VkCommandBuffer commandBuffer,
                uint32_t groupCountX,
                uint32_t groupCountY,
                uint32_t groupCountZ) const {
    bool skip = false;
    LogObjectList _carryOverObjects;
    _carryOverObjects.add(commandBuffer);
    return skip;
}
bool ExplicitValidation::PreCallValidateCmdDrawClusterIndirectHUAWEI(VkCommandBuffer commandBuffer,
                VkBuffer buffer,
                VkDeviceSize offset) const {
    bool skip = false;
    LogObjectList _carryOverObjects;
    _carryOverObjects.add(commandBuffer);
    return skip;
}
bool ExplicitValidation::PreCallValidateSetDeviceMemoryPriorityEXT(VkDevice device,
                VkDeviceMemory memory,
                float priority) const {
    bool skip = false;
    return skip;
}
bool ExplicitValidation::PreCallValidateGetDescriptorSetLayoutHostMappingInfoVALVE(VkDevice device,
                const VkDescriptorSetBindingReferenceVALVE* pBindingReference,
                VkDescriptorSetLayoutHostMappingInfoVALVE* pHostMapping) const {
    bool skip = false;
    if (pBindingReference != nullptr) {
        const auto _s389 = pBindingReference;
        skip |= ValidateDescriptorSetBindingReferenceVALVE({},
            _s389->sType,
            _s389->pNext,
            _s389->descriptorSetLayout,
            _s389->binding);
    }
    if (pHostMapping != nullptr) {
        const auto _s390 = pHostMapping;
        skip |= ValidateDescriptorSetLayoutHostMappingInfoVALVE({},
            _s390->sType,
            _s390->pNext,
            _s390->descriptorOffset,
            _s390->descriptorSize);
    }
    return skip;
}
bool ExplicitValidation::PreCallValidateGetDescriptorSetHostMappingVALVE(VkDevice device,
                VkDescriptorSet descriptorSet,
                void** ppData) const {
    bool skip = false;
    return skip;
}
bool ExplicitValidation::PreCallValidateCmdCopyMemoryIndirectNV(VkCommandBuffer commandBuffer,
                VkDeviceAddress copyBufferAddress,
                uint32_t copyCount,
                uint32_t stride) const {
    bool skip = false;
    LogObjectList _carryOverObjects;
    _carryOverObjects.add(commandBuffer);
    return skip;
}
bool ExplicitValidation::PreCallValidateCmdCopyMemoryToImageIndirectNV(VkCommandBuffer commandBuffer,
                VkDeviceAddress copyBufferAddress,
                uint32_t copyCount,
                uint32_t stride,
                VkImage dstImage,
                VkImageLayout dstImageLayout,
                const VkImageSubresourceLayers* pImageSubresources) const {
    bool skip = false;
    LogObjectList _carryOverObjects;
    _carryOverObjects.add(commandBuffer);
    if (pImageSubresources != nullptr) {
        for (uint32_t _i391 = 0;_i391 < copyCount; ++_i391) {
            const auto _s392 = &pImageSubresources[_i391];
            skip |= ValidateImageSubresourceLayers(_carryOverObjects,
                _s392->aspectMask,
                _s392->mipLevel,
                _s392->baseArrayLayer,
                _s392->layerCount);
        }
    }
    return skip;
}
bool ExplicitValidation::PreCallValidateCmdDecompressMemoryNV(VkCommandBuffer commandBuffer,
                uint32_t decompressRegionCount,
                const VkDecompressMemoryRegionNV* pDecompressMemoryRegions) const {
    bool skip = false;
    LogObjectList _carryOverObjects;
    _carryOverObjects.add(commandBuffer);
    if (pDecompressMemoryRegions != nullptr) {
        for (uint32_t _i393 = 0;_i393 < decompressRegionCount; ++_i393) {
            const auto _s394 = &pDecompressMemoryRegions[_i393];
            skip |= ValidateDecompressMemoryRegionNV(_carryOverObjects,
                _s394->srcAddress,
                _s394->dstAddress,
                _s394->compressedSize,
                _s394->decompressedSize,
                _s394->decompressionMethod);
        }
    }
    return skip;
}
bool ExplicitValidation::PreCallValidateCmdDecompressMemoryIndirectCountNV(VkCommandBuffer commandBuffer,
                VkDeviceAddress indirectCommandsAddress,
                VkDeviceAddress indirectCommandsCountAddress,
                uint32_t stride) const {
    bool skip = false;
    LogObjectList _carryOverObjects;
    _carryOverObjects.add(commandBuffer);
    return skip;
}
bool ExplicitValidation::PreCallValidateCmdSetTessellationDomainOriginEXT(VkCommandBuffer commandBuffer,
                VkTessellationDomainOrigin domainOrigin) const {
    bool skip = false;
    LogObjectList _carryOverObjects;
    _carryOverObjects.add(commandBuffer);
    return skip;
}
bool ExplicitValidation::PreCallValidateCmdSetDepthClampEnableEXT(VkCommandBuffer commandBuffer,
                VkBool32 depthClampEnable) const {
    bool skip = false;
    LogObjectList _carryOverObjects;
    _carryOverObjects.add(commandBuffer);
    return skip;
}
bool ExplicitValidation::PreCallValidateCmdSetPolygonModeEXT(VkCommandBuffer commandBuffer,
                VkPolygonMode polygonMode) const {
    bool skip = false;
    LogObjectList _carryOverObjects;
    _carryOverObjects.add(commandBuffer);
    return skip;
}
bool ExplicitValidation::PreCallValidateCmdSetRasterizationSamplesEXT(VkCommandBuffer commandBuffer,
                VkSampleCountFlagBits rasterizationSamples) const {
    bool skip = false;
    LogObjectList _carryOverObjects;
    _carryOverObjects.add(commandBuffer);
    return skip;
}
bool ExplicitValidation::PreCallValidateCmdSetSampleMaskEXT(VkCommandBuffer commandBuffer,
                VkSampleCountFlagBits samples,
                const VkSampleMask* pSampleMask) const {
    bool skip = false;
    LogObjectList _carryOverObjects;
    _carryOverObjects.add(commandBuffer);
    return skip;
}
bool ExplicitValidation::PreCallValidateCmdSetAlphaToCoverageEnableEXT(VkCommandBuffer commandBuffer,
                VkBool32 alphaToCoverageEnable) const {
    bool skip = false;
    LogObjectList _carryOverObjects;
    _carryOverObjects.add(commandBuffer);
    return skip;
}
bool ExplicitValidation::PreCallValidateCmdSetAlphaToOneEnableEXT(VkCommandBuffer commandBuffer,
                VkBool32 alphaToOneEnable) const {
    bool skip = false;
    LogObjectList _carryOverObjects;
    _carryOverObjects.add(commandBuffer);
    return skip;
}
bool ExplicitValidation::PreCallValidateCmdSetLogicOpEnableEXT(VkCommandBuffer commandBuffer,
                VkBool32 logicOpEnable) const {
    bool skip = false;
    LogObjectList _carryOverObjects;
    _carryOverObjects.add(commandBuffer);
    return skip;
}
bool ExplicitValidation::PreCallValidateCmdSetColorBlendEnableEXT(VkCommandBuffer commandBuffer,
                uint32_t firstAttachment,
                uint32_t attachmentCount,
                const VkBool32* pColorBlendEnables) const {
    bool skip = false;
    LogObjectList _carryOverObjects;
    _carryOverObjects.add(commandBuffer);
    return skip;
}
bool ExplicitValidation::PreCallValidateCmdSetColorBlendEquationEXT(VkCommandBuffer commandBuffer,
                uint32_t firstAttachment,
                uint32_t attachmentCount,
                const VkColorBlendEquationEXT* pColorBlendEquations) const {
    bool skip = false;
    LogObjectList _carryOverObjects;
    _carryOverObjects.add(commandBuffer);
    if (pColorBlendEquations != nullptr) {
        for (uint32_t _i395 = 0;_i395 < attachmentCount; ++_i395) {
            const auto _s396 = &pColorBlendEquations[_i395];
            skip |= ValidateColorBlendEquationEXT(_carryOverObjects,
                _s396->srcColorBlendFactor,
                _s396->dstColorBlendFactor,
                _s396->colorBlendOp,
                _s396->srcAlphaBlendFactor,
                _s396->dstAlphaBlendFactor,
                _s396->alphaBlendOp);
        }
    }
    return skip;
}
bool ExplicitValidation::PreCallValidateCmdSetColorWriteMaskEXT(VkCommandBuffer commandBuffer,
                uint32_t firstAttachment,
                uint32_t attachmentCount,
                const VkColorComponentFlags* pColorWriteMasks) const {
    bool skip = false;
    LogObjectList _carryOverObjects;
    _carryOverObjects.add(commandBuffer);
    return skip;
}
bool ExplicitValidation::PreCallValidateCmdSetRasterizationStreamEXT(VkCommandBuffer commandBuffer,
                uint32_t rasterizationStream) const {
    bool skip = false;
    LogObjectList _carryOverObjects;
    _carryOverObjects.add(commandBuffer);
    return skip;
}
bool ExplicitValidation::PreCallValidateCmdSetConservativeRasterizationModeEXT(VkCommandBuffer commandBuffer,
                VkConservativeRasterizationModeEXT conservativeRasterizationMode) const {
    bool skip = false;
    LogObjectList _carryOverObjects;
    _carryOverObjects.add(commandBuffer);
    return skip;
}
bool ExplicitValidation::PreCallValidateCmdSetExtraPrimitiveOverestimationSizeEXT(VkCommandBuffer commandBuffer,
                float extraPrimitiveOverestimationSize) const {
    bool skip = false;
    LogObjectList _carryOverObjects;
    _carryOverObjects.add(commandBuffer);
    return skip;
}
bool ExplicitValidation::PreCallValidateCmdSetDepthClipEnableEXT(VkCommandBuffer commandBuffer,
                VkBool32 depthClipEnable) const {
    bool skip = false;
    LogObjectList _carryOverObjects;
    _carryOverObjects.add(commandBuffer);
    return skip;
}
bool ExplicitValidation::PreCallValidateCmdSetSampleLocationsEnableEXT(VkCommandBuffer commandBuffer,
                VkBool32 sampleLocationsEnable) const {
    bool skip = false;
    LogObjectList _carryOverObjects;
    _carryOverObjects.add(commandBuffer);
    return skip;
}
bool ExplicitValidation::PreCallValidateCmdSetColorBlendAdvancedEXT(VkCommandBuffer commandBuffer,
                uint32_t firstAttachment,
                uint32_t attachmentCount,
                const VkColorBlendAdvancedEXT* pColorBlendAdvanced) const {
    bool skip = false;
    LogObjectList _carryOverObjects;
    _carryOverObjects.add(commandBuffer);
    if (pColorBlendAdvanced != nullptr) {
        for (uint32_t _i397 = 0;_i397 < attachmentCount; ++_i397) {
            const auto _s398 = &pColorBlendAdvanced[_i397];
            skip |= ValidateColorBlendAdvancedEXT(_carryOverObjects,
                _s398->advancedBlendOp,
                _s398->srcPremultiplied,
                _s398->dstPremultiplied,
                _s398->blendOverlap,
                _s398->clampResults);
        }
    }
    return skip;
}
bool ExplicitValidation::PreCallValidateCmdSetProvokingVertexModeEXT(VkCommandBuffer commandBuffer,
                VkProvokingVertexModeEXT provokingVertexMode) const {
    bool skip = false;
    LogObjectList _carryOverObjects;
    _carryOverObjects.add(commandBuffer);
    return skip;
}
bool ExplicitValidation::PreCallValidateCmdSetLineRasterizationModeEXT(VkCommandBuffer commandBuffer,
                VkLineRasterizationModeEXT lineRasterizationMode) const {
    bool skip = false;
    LogObjectList _carryOverObjects;
    _carryOverObjects.add(commandBuffer);
    return skip;
}
bool ExplicitValidation::PreCallValidateCmdSetLineStippleEnableEXT(VkCommandBuffer commandBuffer,
                VkBool32 stippledLineEnable) const {
    bool skip = false;
    LogObjectList _carryOverObjects;
    _carryOverObjects.add(commandBuffer);
    return skip;
}
bool ExplicitValidation::PreCallValidateCmdSetDepthClipNegativeOneToOneEXT(VkCommandBuffer commandBuffer,
                VkBool32 negativeOneToOne) const {
    bool skip = false;
    LogObjectList _carryOverObjects;
    _carryOverObjects.add(commandBuffer);
    return skip;
}
bool ExplicitValidation::PreCallValidateCmdSetViewportWScalingEnableNV(VkCommandBuffer commandBuffer,
                VkBool32 viewportWScalingEnable) const {
    bool skip = false;
    LogObjectList _carryOverObjects;
    _carryOverObjects.add(commandBuffer);
    return skip;
}
bool ExplicitValidation::PreCallValidateCmdSetViewportSwizzleNV(VkCommandBuffer commandBuffer,
                uint32_t firstViewport,
                uint32_t viewportCount,
                const VkViewportSwizzleNV* pViewportSwizzles) const {
    bool skip = false;
    LogObjectList _carryOverObjects;
    _carryOverObjects.add(commandBuffer);
    if (pViewportSwizzles != nullptr) {
        for (uint32_t _i399 = 0;_i399 < viewportCount; ++_i399) {
            const auto _s400 = &pViewportSwizzles[_i399];
            skip |= ValidateViewportSwizzleNV(_carryOverObjects,
                _s400->x,
                _s400->y,
                _s400->z,
                _s400->w);
        }
    }
    return skip;
}
bool ExplicitValidation::PreCallValidateCmdSetCoverageToColorEnableNV(VkCommandBuffer commandBuffer,
                VkBool32 coverageToColorEnable) const {
    bool skip = false;
    LogObjectList _carryOverObjects;
    _carryOverObjects.add(commandBuffer);
    return skip;
}
bool ExplicitValidation::PreCallValidateCmdSetCoverageToColorLocationNV(VkCommandBuffer commandBuffer,
                uint32_t coverageToColorLocation) const {
    bool skip = false;
    LogObjectList _carryOverObjects;
    _carryOverObjects.add(commandBuffer);
    return skip;
}
bool ExplicitValidation::PreCallValidateCmdSetCoverageModulationModeNV(VkCommandBuffer commandBuffer,
                VkCoverageModulationModeNV coverageModulationMode) const {
    bool skip = false;
    LogObjectList _carryOverObjects;
    _carryOverObjects.add(commandBuffer);
    return skip;
}
bool ExplicitValidation::PreCallValidateCmdSetCoverageModulationTableEnableNV(VkCommandBuffer commandBuffer,
                VkBool32 coverageModulationTableEnable) const {
    bool skip = false;
    LogObjectList _carryOverObjects;
    _carryOverObjects.add(commandBuffer);
    return skip;
}
bool ExplicitValidation::PreCallValidateCmdSetCoverageModulationTableNV(VkCommandBuffer commandBuffer,
                uint32_t coverageModulationTableCount,
                const float* pCoverageModulationTable) const {
    bool skip = false;
    LogObjectList _carryOverObjects;
    _carryOverObjects.add(commandBuffer);
    return skip;
}
bool ExplicitValidation::PreCallValidateCmdSetShadingRateImageEnableNV(VkCommandBuffer commandBuffer,
                VkBool32 shadingRateImageEnable) const {
    bool skip = false;
    LogObjectList _carryOverObjects;
    _carryOverObjects.add(commandBuffer);
    return skip;
}
bool ExplicitValidation::PreCallValidateCmdSetRepresentativeFragmentTestEnableNV(VkCommandBuffer commandBuffer,
                VkBool32 representativeFragmentTestEnable) const {
    bool skip = false;
    LogObjectList _carryOverObjects;
    _carryOverObjects.add(commandBuffer);
    return skip;
}
bool ExplicitValidation::PreCallValidateCmdSetCoverageReductionModeNV(VkCommandBuffer commandBuffer,
                VkCoverageReductionModeNV coverageReductionMode) const {
    bool skip = false;
    LogObjectList _carryOverObjects;
    _carryOverObjects.add(commandBuffer);
    return skip;
}
bool ExplicitValidation::PreCallValidateGetShaderModuleIdentifierEXT(VkDevice device,
                VkShaderModule shaderModule,
                VkShaderModuleIdentifierEXT* pIdentifier) const {
    bool skip = false;
    return skip;
}
bool ExplicitValidation::PreCallValidateGetShaderModuleCreateInfoIdentifierEXT(VkDevice device,
                const VkShaderModuleCreateInfo* pCreateInfo,
                VkShaderModuleIdentifierEXT* pIdentifier) const {
    bool skip = false;
    if (pCreateInfo != nullptr) {
        const auto _s401 = pCreateInfo;
        skip |= ValidateShaderModuleCreateInfo({},
            _s401->sType,
            _s401->pNext,
            _s401->flags,
            _s401->codeSize,
            _s401->pCode);
    }
    return skip;
}
bool ExplicitValidation::PreCallValidateGetPhysicalDeviceOpticalFlowImageFormatsNV(VkPhysicalDevice physicalDevice,
                const VkOpticalFlowImageFormatInfoNV* pOpticalFlowImageFormatInfo,
                uint32_t* pFormatCount,
                VkOpticalFlowImageFormatPropertiesNV* pImageFormatProperties) const {
    bool skip = false;
    if (pOpticalFlowImageFormatInfo != nullptr) {
        const auto _s402 = pOpticalFlowImageFormatInfo;
        skip |= ValidateOpticalFlowImageFormatInfoNV({},
            _s402->sType,
            _s402->pNext,
            _s402->usage);
    }
    return skip;
}
bool ExplicitValidation::PreCallValidateCreateOpticalFlowSessionNV(VkDevice device,
                const VkOpticalFlowSessionCreateInfoNV* pCreateInfo,
                const VkAllocationCallbacks* pAllocator,
                VkOpticalFlowSessionNV* pSession) const {
    bool skip = false;
    if (pCreateInfo != nullptr) {
        const auto _s403 = pCreateInfo;
        skip |= ValidateOpticalFlowSessionCreateInfoNV({},
            _s403->sType,
            _s403->pNext,
            _s403->width,
            _s403->height,
            _s403->imageFormat,
            _s403->flowVectorFormat,
            _s403->costFormat,
            _s403->outputGridSize,
            _s403->hintGridSize,
            _s403->performanceLevel,
            _s403->flags);
    }
    if (pAllocator != nullptr) {
        const auto _s404 = pAllocator;
        skip |= ValidateAllocationCallbacks({},
            _s404->pUserData,
            _s404->pfnAllocation,
            _s404->pfnReallocation,
            _s404->pfnFree,
            _s404->pfnInternalAllocation,
            _s404->pfnInternalFree);
    }
    return skip;
}
bool ExplicitValidation::PreCallValidateDestroyOpticalFlowSessionNV(VkDevice device,
                VkOpticalFlowSessionNV session,
                const VkAllocationCallbacks* pAllocator) const {
    bool skip = false;
    if (pAllocator != nullptr) {
        const auto _s405 = pAllocator;
        skip |= ValidateAllocationCallbacks({},
            _s405->pUserData,
            _s405->pfnAllocation,
            _s405->pfnReallocation,
            _s405->pfnFree,
            _s405->pfnInternalAllocation,
            _s405->pfnInternalFree);
    }
    return skip;
}
bool ExplicitValidation::PreCallValidateBindOpticalFlowSessionImageNV(VkDevice device,
                VkOpticalFlowSessionNV session,
                VkOpticalFlowSessionBindingPointNV bindingPoint,
                VkImageView view,
                VkImageLayout layout) const {
    bool skip = false;
    return skip;
}
bool ExplicitValidation::PreCallValidateCmdOpticalFlowExecuteNV(VkCommandBuffer commandBuffer,
                VkOpticalFlowSessionNV session,
                const VkOpticalFlowExecuteInfoNV* pExecuteInfo) const {
    bool skip = false;
    LogObjectList _carryOverObjects;
    _carryOverObjects.add(commandBuffer);
    if (pExecuteInfo != nullptr) {
        const auto _s406 = pExecuteInfo;
        skip |= ValidateOpticalFlowExecuteInfoNV(_carryOverObjects,
            _s406->sType,
            _s406->pNext,
            _s406->flags,
            _s406->regionCount,
            _s406->pRegions);
    }
    return skip;
}
bool ExplicitValidation::PreCallValidateGetFramebufferTilePropertiesQCOM(VkDevice device,
                VkFramebuffer framebuffer,
                uint32_t* pPropertiesCount,
                VkTilePropertiesQCOM* pProperties) const {
    bool skip = false;
    if (pProperties != nullptr) {
        for (uint32_t _i407 = 0;_i407 < *pPropertiesCount; ++_i407) {
            const auto _s408 = &pProperties[_i407];
            skip |= ValidateTilePropertiesQCOM({},
                _s408->sType,
                _s408->pNext,
                _s408->tileSize,
                _s408->apronSize,
                _s408->origin);
        }
    }
    return skip;
}
bool ExplicitValidation::PreCallValidateGetDynamicRenderingTilePropertiesQCOM(VkDevice device,
                const VkRenderingInfo* pRenderingInfo,
                VkTilePropertiesQCOM* pProperties) const {
    bool skip = false;
    if (pRenderingInfo != nullptr) {
        const auto _s409 = pRenderingInfo;
        skip |= ValidateRenderingInfo({},
            _s409->sType,
            _s409->pNext,
            _s409->flags,
            _s409->renderArea,
            _s409->layerCount,
            _s409->viewMask,
            _s409->colorAttachmentCount,
            _s409->pColorAttachments,
            _s409->pDepthAttachment,
            _s409->pStencilAttachment);
    }
    if (pProperties != nullptr) {
        const auto _s410 = pProperties;
        skip |= ValidateTilePropertiesQCOM({},
            _s410->sType,
            _s410->pNext,
            _s410->tileSize,
            _s410->apronSize,
            _s410->origin);
    }
    return skip;
}
bool ExplicitValidation::PreCallValidateCreateAccelerationStructureKHR(VkDevice device,
                const VkAccelerationStructureCreateInfoKHR* pCreateInfo,
                const VkAllocationCallbacks* pAllocator,
                VkAccelerationStructureKHR* pAccelerationStructure) const {
    bool skip = false;
    if (pCreateInfo != nullptr) {
        const auto _s411 = pCreateInfo;
        skip |= ValidateAccelerationStructureCreateInfoKHR({},
            _s411->sType,
            _s411->pNext,
            _s411->createFlags,
            _s411->buffer,
            _s411->offset,
            _s411->size,
            _s411->type,
            _s411->deviceAddress);
    }
    if (pAllocator != nullptr) {
        const auto _s412 = pAllocator;
        skip |= ValidateAllocationCallbacks({},
            _s412->pUserData,
            _s412->pfnAllocation,
            _s412->pfnReallocation,
            _s412->pfnFree,
            _s412->pfnInternalAllocation,
            _s412->pfnInternalFree);
    }
    return skip;
}
bool ExplicitValidation::PreCallValidateDestroyAccelerationStructureKHR(VkDevice device,
                VkAccelerationStructureKHR accelerationStructure,
                const VkAllocationCallbacks* pAllocator) const {
    bool skip = false;
    if (pAllocator != nullptr) {
        const auto _s413 = pAllocator;
        skip |= ValidateAllocationCallbacks({},
            _s413->pUserData,
            _s413->pfnAllocation,
            _s413->pfnReallocation,
            _s413->pfnFree,
            _s413->pfnInternalAllocation,
            _s413->pfnInternalFree);
    }
    return skip;
}
bool ExplicitValidation::PreCallValidateCmdBuildAccelerationStructuresKHR(VkCommandBuffer commandBuffer,
                uint32_t infoCount,
                const VkAccelerationStructureBuildGeometryInfoKHR* pInfos,
                const VkAccelerationStructureBuildRangeInfoKHR* const* ppBuildRangeInfos) const {
    bool skip = false;
    LogObjectList _carryOverObjects;
    _carryOverObjects.add(commandBuffer);
    if (pInfos != nullptr) {
        for (uint32_t _i414 = 0;_i414 < infoCount; ++_i414) {
            const auto _s415 = &pInfos[_i414];
            skip |= ValidateAccelerationStructureBuildGeometryInfoKHR(_carryOverObjects,
                _s415->sType,
                _s415->pNext,
                _s415->type,
                _s415->flags,
                _s415->mode,
                _s415->srcAccelerationStructure,
                _s415->dstAccelerationStructure,
                _s415->geometryCount,
                _s415->pGeometries,
                _s415->ppGeometries,
                _s415->scratchData);
        }
    }
    if (ppBuildRangeInfos != nullptr) {
        for (uint32_t _i416 = 0;_i416 < infoCount; ++_i416) {
            const auto _s417 = ppBuildRangeInfos[_i416];
            skip |= ValidateAccelerationStructureBuildRangeInfoKHR(_carryOverObjects,
                _s417->primitiveCount,
                _s417->primitiveOffset,
                _s417->firstVertex,
                _s417->transformOffset);
        }
    }
    return skip;
}
bool ExplicitValidation::PreCallValidateCmdBuildAccelerationStructuresIndirectKHR(VkCommandBuffer commandBuffer,
                uint32_t infoCount,
                const VkAccelerationStructureBuildGeometryInfoKHR* pInfos,
                const VkDeviceAddress* pIndirectDeviceAddresses,
                const uint32_t* pIndirectStrides,
                const uint32_t* const* ppMaxPrimitiveCounts) const {
    bool skip = false;
    LogObjectList _carryOverObjects;
    _carryOverObjects.add(commandBuffer);
    if (pInfos != nullptr) {
        for (uint32_t _i418 = 0;_i418 < infoCount; ++_i418) {
            const auto _s419 = &pInfos[_i418];
            skip |= ValidateAccelerationStructureBuildGeometryInfoKHR(_carryOverObjects,
                _s419->sType,
                _s419->pNext,
                _s419->type,
                _s419->flags,
                _s419->mode,
                _s419->srcAccelerationStructure,
                _s419->dstAccelerationStructure,
                _s419->geometryCount,
                _s419->pGeometries,
                _s419->ppGeometries,
                _s419->scratchData);
        }
    }
    return skip;
}
bool ExplicitValidation::PreCallValidateBuildAccelerationStructuresKHR(VkDevice device,
                VkDeferredOperationKHR deferredOperation,
                uint32_t infoCount,
                const VkAccelerationStructureBuildGeometryInfoKHR* pInfos,
                const VkAccelerationStructureBuildRangeInfoKHR* const* ppBuildRangeInfos) const {
    bool skip = false;
    if (pInfos != nullptr) {
        for (uint32_t _i420 = 0;_i420 < infoCount; ++_i420) {
            const auto _s421 = &pInfos[_i420];
            skip |= ValidateAccelerationStructureBuildGeometryInfoKHR({},
                _s421->sType,
                _s421->pNext,
                _s421->type,
                _s421->flags,
                _s421->mode,
                _s421->srcAccelerationStructure,
                _s421->dstAccelerationStructure,
                _s421->geometryCount,
                _s421->pGeometries,
                _s421->ppGeometries,
                _s421->scratchData);
        }
    }
    if (ppBuildRangeInfos != nullptr) {
        for (uint32_t _i422 = 0;_i422 < infoCount; ++_i422) {
            const auto _s423 = ppBuildRangeInfos[_i422];
            skip |= ValidateAccelerationStructureBuildRangeInfoKHR({},
                _s423->primitiveCount,
                _s423->primitiveOffset,
                _s423->firstVertex,
                _s423->transformOffset);
        }
    }
    return skip;
}
bool ExplicitValidation::PreCallValidateCopyAccelerationStructureKHR(VkDevice device,
                VkDeferredOperationKHR deferredOperation,
                const VkCopyAccelerationStructureInfoKHR* pInfo) const {
    bool skip = false;
    if (pInfo != nullptr) {
        const auto _s424 = pInfo;
        skip |= ValidateCopyAccelerationStructureInfoKHR({},
            _s424->sType,
            _s424->pNext,
            _s424->src,
            _s424->dst,
            _s424->mode);
    }
    return skip;
}
bool ExplicitValidation::PreCallValidateCopyAccelerationStructureToMemoryKHR(VkDevice device,
                VkDeferredOperationKHR deferredOperation,
                const VkCopyAccelerationStructureToMemoryInfoKHR* pInfo) const {
    bool skip = false;
    if (pInfo != nullptr) {
        const auto _s425 = pInfo;
        skip |= ValidateCopyAccelerationStructureToMemoryInfoKHR({},
            _s425->sType,
            _s425->pNext,
            _s425->src,
            _s425->dst,
            _s425->mode);
    }
    return skip;
}
bool ExplicitValidation::PreCallValidateCopyMemoryToAccelerationStructureKHR(VkDevice device,
                VkDeferredOperationKHR deferredOperation,
                const VkCopyMemoryToAccelerationStructureInfoKHR* pInfo) const {
    bool skip = false;
    if (pInfo != nullptr) {
        const auto _s426 = pInfo;
        skip |= ValidateCopyMemoryToAccelerationStructureInfoKHR({},
            _s426->sType,
            _s426->pNext,
            _s426->src,
            _s426->dst,
            _s426->mode);
    }
    return skip;
}
bool ExplicitValidation::PreCallValidateWriteAccelerationStructuresPropertiesKHR(VkDevice device,
                uint32_t accelerationStructureCount,
                const VkAccelerationStructureKHR* pAccelerationStructures,
                VkQueryType queryType,
                size_t dataSize,
                void* pData,
                size_t stride) const {
    bool skip = false;
    return skip;
}
bool ExplicitValidation::PreCallValidateCmdCopyAccelerationStructureKHR(VkCommandBuffer commandBuffer,
                const VkCopyAccelerationStructureInfoKHR* pInfo) const {
    bool skip = false;
    LogObjectList _carryOverObjects;
    _carryOverObjects.add(commandBuffer);
    if (pInfo != nullptr) {
        const auto _s427 = pInfo;
        skip |= ValidateCopyAccelerationStructureInfoKHR(_carryOverObjects,
            _s427->sType,
            _s427->pNext,
            _s427->src,
            _s427->dst,
            _s427->mode);
    }
    return skip;
}
bool ExplicitValidation::PreCallValidateCmdCopyAccelerationStructureToMemoryKHR(VkCommandBuffer commandBuffer,
                const VkCopyAccelerationStructureToMemoryInfoKHR* pInfo) const {
    bool skip = false;
    LogObjectList _carryOverObjects;
    _carryOverObjects.add(commandBuffer);
    if (pInfo != nullptr) {
        const auto _s428 = pInfo;
        skip |= ValidateCopyAccelerationStructureToMemoryInfoKHR(_carryOverObjects,
            _s428->sType,
            _s428->pNext,
            _s428->src,
            _s428->dst,
            _s428->mode);
    }
    return skip;
}
bool ExplicitValidation::PreCallValidateCmdCopyMemoryToAccelerationStructureKHR(VkCommandBuffer commandBuffer,
                const VkCopyMemoryToAccelerationStructureInfoKHR* pInfo) const {
    bool skip = false;
    LogObjectList _carryOverObjects;
    _carryOverObjects.add(commandBuffer);
    if (pInfo != nullptr) {
        const auto _s429 = pInfo;
        skip |= ValidateCopyMemoryToAccelerationStructureInfoKHR(_carryOverObjects,
            _s429->sType,
            _s429->pNext,
            _s429->src,
            _s429->dst,
            _s429->mode);
    }
    return skip;
}
bool ExplicitValidation::PreCallValidateGetAccelerationStructureDeviceAddressKHR(VkDevice device,
                const VkAccelerationStructureDeviceAddressInfoKHR* pInfo) const {
    bool skip = false;
    if (pInfo != nullptr) {
        const auto _s430 = pInfo;
        skip |= ValidateAccelerationStructureDeviceAddressInfoKHR({},
            _s430->sType,
            _s430->pNext,
            _s430->accelerationStructure);
    }
    return skip;
}
bool ExplicitValidation::PreCallValidateCmdWriteAccelerationStructuresPropertiesKHR(VkCommandBuffer commandBuffer,
                uint32_t accelerationStructureCount,
                const VkAccelerationStructureKHR* pAccelerationStructures,
                VkQueryType queryType,
                VkQueryPool queryPool,
                uint32_t firstQuery) const {
    bool skip = false;
    LogObjectList _carryOverObjects;
    _carryOverObjects.add(commandBuffer);
    return skip;
}
bool ExplicitValidation::PreCallValidateGetDeviceAccelerationStructureCompatibilityKHR(VkDevice device,
                const VkAccelerationStructureVersionInfoKHR* pVersionInfo,
                VkAccelerationStructureCompatibilityKHR* pCompatibility) const {
    bool skip = false;
    if (pVersionInfo != nullptr) {
        const auto _s431 = pVersionInfo;
        skip |= ValidateAccelerationStructureVersionInfoKHR({},
            _s431->sType,
            _s431->pNext,
            _s431->pVersionData);
    }
    return skip;
}
bool ExplicitValidation::PreCallValidateGetAccelerationStructureBuildSizesKHR(VkDevice device,
                VkAccelerationStructureBuildTypeKHR buildType,
                const VkAccelerationStructureBuildGeometryInfoKHR* pBuildInfo,
                const uint32_t* pMaxPrimitiveCounts,
                VkAccelerationStructureBuildSizesInfoKHR* pSizeInfo) const {
    bool skip = false;
    if (pBuildInfo != nullptr) {
        const auto _s432 = pBuildInfo;
        skip |= ValidateAccelerationStructureBuildGeometryInfoKHR({},
            _s432->sType,
            _s432->pNext,
            _s432->type,
            _s432->flags,
            _s432->mode,
            _s432->srcAccelerationStructure,
            _s432->dstAccelerationStructure,
            _s432->geometryCount,
            _s432->pGeometries,
            _s432->ppGeometries,
            _s432->scratchData);
    }
    if (pSizeInfo != nullptr) {
        const auto _s433 = pSizeInfo;
        skip |= ValidateAccelerationStructureBuildSizesInfoKHR({},
            _s433->sType,
            _s433->pNext,
            _s433->accelerationStructureSize,
            _s433->updateScratchSize,
            _s433->buildScratchSize);
    }
    return skip;
}
bool ExplicitValidation::PreCallValidateCmdTraceRaysKHR(VkCommandBuffer commandBuffer,
                const VkStridedDeviceAddressRegionKHR* pRaygenShaderBindingTable,
                const VkStridedDeviceAddressRegionKHR* pMissShaderBindingTable,
                const VkStridedDeviceAddressRegionKHR* pHitShaderBindingTable,
                const VkStridedDeviceAddressRegionKHR* pCallableShaderBindingTable,
                uint32_t width,
                uint32_t height,
                uint32_t depth) const {
    bool skip = false;
    LogObjectList _carryOverObjects;
    _carryOverObjects.add(commandBuffer);
    if (pRaygenShaderBindingTable != nullptr) {
        const auto _s434 = pRaygenShaderBindingTable;
        skip |= ValidateStridedDeviceAddressRegionKHR(_carryOverObjects,
            _s434->deviceAddress,
            _s434->stride,
            _s434->size);
    }
    if (pMissShaderBindingTable != nullptr) {
        const auto _s435 = pMissShaderBindingTable;
        skip |= ValidateStridedDeviceAddressRegionKHR(_carryOverObjects,
            _s435->deviceAddress,
            _s435->stride,
            _s435->size);
    }
    if (pHitShaderBindingTable != nullptr) {
        const auto _s436 = pHitShaderBindingTable;
        skip |= ValidateStridedDeviceAddressRegionKHR(_carryOverObjects,
            _s436->deviceAddress,
            _s436->stride,
            _s436->size);
    }
    if (pCallableShaderBindingTable != nullptr) {
        const auto _s437 = pCallableShaderBindingTable;
        skip |= ValidateStridedDeviceAddressRegionKHR(_carryOverObjects,
            _s437->deviceAddress,
            _s437->stride,
            _s437->size);
    }
    return skip;
}
bool ExplicitValidation::PreCallValidateCreateRayTracingPipelinesKHR(VkDevice device,
                VkDeferredOperationKHR deferredOperation,
                VkPipelineCache pipelineCache,
                uint32_t createInfoCount,
                const VkRayTracingPipelineCreateInfoKHR* pCreateInfos,
                const VkAllocationCallbacks* pAllocator,
                VkPipeline* pPipelines,
                void *validation_state) const {
    bool skip = false;
    if (pCreateInfos != nullptr) {
        for (uint32_t _i438 = 0;_i438 < createInfoCount; ++_i438) {
            const auto _s439 = &pCreateInfos[_i438];
            skip |= ValidateRayTracingPipelineCreateInfoKHR({},
                _s439->sType,
                _s439->pNext,
                _s439->flags,
                _s439->stageCount,
                _s439->pStages,
                _s439->groupCount,
                _s439->pGroups,
                _s439->maxPipelineRayRecursionDepth,
                _s439->pLibraryInfo,
                _s439->pLibraryInterface,
                _s439->pDynamicState,
                _s439->layout,
                _s439->basePipelineHandle,
                _s439->basePipelineIndex);
        }
    }
    if (pAllocator != nullptr) {
        const auto _s440 = pAllocator;
        skip |= ValidateAllocationCallbacks({},
            _s440->pUserData,
            _s440->pfnAllocation,
            _s440->pfnReallocation,
            _s440->pfnFree,
            _s440->pfnInternalAllocation,
            _s440->pfnInternalFree);
    }
    return skip;
}
bool ExplicitValidation::PreCallValidateGetRayTracingCaptureReplayShaderGroupHandlesKHR(VkDevice device,
                VkPipeline pipeline,
                uint32_t firstGroup,
                uint32_t groupCount,
                size_t dataSize,
                void* pData) const {
    bool skip = false;
    return skip;
}
bool ExplicitValidation::PreCallValidateCmdTraceRaysIndirectKHR(VkCommandBuffer commandBuffer,
                const VkStridedDeviceAddressRegionKHR* pRaygenShaderBindingTable,
                const VkStridedDeviceAddressRegionKHR* pMissShaderBindingTable,
                const VkStridedDeviceAddressRegionKHR* pHitShaderBindingTable,
                const VkStridedDeviceAddressRegionKHR* pCallableShaderBindingTable,
                VkDeviceAddress indirectDeviceAddress) const {
    bool skip = false;
    LogObjectList _carryOverObjects;
    _carryOverObjects.add(commandBuffer);
    if (pRaygenShaderBindingTable != nullptr) {
        const auto _s441 = pRaygenShaderBindingTable;
        skip |= ValidateStridedDeviceAddressRegionKHR(_carryOverObjects,
            _s441->deviceAddress,
            _s441->stride,
            _s441->size);
    }
    if (pMissShaderBindingTable != nullptr) {
        const auto _s442 = pMissShaderBindingTable;
        skip |= ValidateStridedDeviceAddressRegionKHR(_carryOverObjects,
            _s442->deviceAddress,
            _s442->stride,
            _s442->size);
    }
    if (pHitShaderBindingTable != nullptr) {
        const auto _s443 = pHitShaderBindingTable;
        skip |= ValidateStridedDeviceAddressRegionKHR(_carryOverObjects,
            _s443->deviceAddress,
            _s443->stride,
            _s443->size);
    }
    if (pCallableShaderBindingTable != nullptr) {
        const auto _s444 = pCallableShaderBindingTable;
        skip |= ValidateStridedDeviceAddressRegionKHR(_carryOverObjects,
            _s444->deviceAddress,
            _s444->stride,
            _s444->size);
    }
    return skip;
}
bool ExplicitValidation::PreCallValidateGetRayTracingShaderGroupStackSizeKHR(VkDevice device,
                VkPipeline pipeline,
                uint32_t group,
                VkShaderGroupShaderKHR groupShader) const {
    bool skip = false;
    return skip;
}
bool ExplicitValidation::PreCallValidateCmdSetRayTracingPipelineStackSizeKHR(VkCommandBuffer commandBuffer,
                uint32_t pipelineStackSize) const {
    bool skip = false;
    LogObjectList _carryOverObjects;
    _carryOverObjects.add(commandBuffer);
    return skip;
}
bool ExplicitValidation::PreCallValidateCmdDrawMeshTasksEXT(VkCommandBuffer commandBuffer,
                uint32_t groupCountX,
                uint32_t groupCountY,
                uint32_t groupCountZ) const {
    bool skip = false;
    LogObjectList _carryOverObjects;
    _carryOverObjects.add(commandBuffer);
    return skip;
}
bool ExplicitValidation::PreCallValidateCmdDrawMeshTasksIndirectEXT(VkCommandBuffer commandBuffer,
                VkBuffer buffer,
                VkDeviceSize offset,
                uint32_t drawCount,
                uint32_t stride) const {
    bool skip = false;
    LogObjectList _carryOverObjects;
    _carryOverObjects.add(commandBuffer);
    return skip;
}
bool ExplicitValidation::PreCallValidateCmdDrawMeshTasksIndirectCountEXT(VkCommandBuffer commandBuffer,
                VkBuffer buffer,
                VkDeviceSize offset,
                VkBuffer countBuffer,
                VkDeviceSize countBufferOffset,
                uint32_t maxDrawCount,
                uint32_t stride) const {
    bool skip = false;
    LogObjectList _carryOverObjects;
    _carryOverObjects.add(commandBuffer);
    return skip;
}

