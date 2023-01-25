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


#pragma once

bool ValidateExtent2D(const LogObjectList &_parentObjects,
    const uint32_t width,
    const uint32_t height) const;
bool ValidateExtent3D(const LogObjectList &_parentObjects,
    const uint32_t width,
    const uint32_t height,
    const uint32_t depth) const;
bool ValidateOffset2D(const LogObjectList &_parentObjects,
    const int32_t x,
    const int32_t y) const;
bool ValidateOffset3D(const LogObjectList &_parentObjects,
    const int32_t x,
    const int32_t y,
    const int32_t z) const;
bool ValidateRect2D(const LogObjectList &_parentObjects,
    const VkOffset2D offset,
    const VkExtent2D extent) const;
bool ValidateBaseInStructure(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const struct VkBaseInStructure* pNext) const;
bool ValidateBaseOutStructure(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const struct VkBaseOutStructure* pNext) const;
bool ValidateBufferMemoryBarrier(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const VkAccessFlags srcAccessMask,
    const VkAccessFlags dstAccessMask,
    const uint32_t srcQueueFamilyIndex,
    const uint32_t dstQueueFamilyIndex,
    const VkBuffer buffer,
    const VkDeviceSize offset,
    const VkDeviceSize size) const;
bool ValidateDispatchIndirectCommand(const LogObjectList &_parentObjects,
    const uint32_t x,
    const uint32_t y,
    const uint32_t z) const;
bool ValidateDrawIndexedIndirectCommand(const LogObjectList &_parentObjects,
    const uint32_t indexCount,
    const uint32_t instanceCount,
    const uint32_t firstIndex,
    const int32_t vertexOffset,
    const uint32_t firstInstance) const;
bool ValidateDrawIndirectCommand(const LogObjectList &_parentObjects,
    const uint32_t vertexCount,
    const uint32_t instanceCount,
    const uint32_t firstVertex,
    const uint32_t firstInstance) const;
bool ValidateImageSubresourceRange(const LogObjectList &_parentObjects,
    const VkImageAspectFlags aspectMask,
    const uint32_t baseMipLevel,
    const uint32_t levelCount,
    const uint32_t baseArrayLayer,
    const uint32_t layerCount) const;
bool ValidateImageMemoryBarrier(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const VkAccessFlags srcAccessMask,
    const VkAccessFlags dstAccessMask,
    const VkImageLayout oldLayout,
    const VkImageLayout newLayout,
    const uint32_t srcQueueFamilyIndex,
    const uint32_t dstQueueFamilyIndex,
    const VkImage image,
    const VkImageSubresourceRange subresourceRange) const;
bool ValidateMemoryBarrier(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const VkAccessFlags srcAccessMask,
    const VkAccessFlags dstAccessMask) const;
bool ValidatePipelineCacheHeaderVersionOne(const LogObjectList &_parentObjects,
    const uint32_t headerSize,
    const VkPipelineCacheHeaderVersion headerVersion,
    const uint32_t vendorID,
    const uint32_t deviceID,
    const uint8_t pipelineCacheUUID[VK_UUID_SIZE]) const;
bool ValidateAllocationCallbacks(const LogObjectList &_parentObjects,
    const void* pUserData,
    const PFN_vkAllocationFunction pfnAllocation,
    const PFN_vkReallocationFunction pfnReallocation,
    const PFN_vkFreeFunction pfnFree,
    const PFN_vkInternalAllocationNotification pfnInternalAllocation,
    const PFN_vkInternalFreeNotification pfnInternalFree) const;
bool ValidateApplicationInfo(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const char* pApplicationName,
    const uint32_t applicationVersion,
    const char* pEngineName,
    const uint32_t engineVersion,
    const uint32_t apiVersion) const;
bool ValidateInstanceCreateInfo(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const VkInstanceCreateFlags flags,
    const VkApplicationInfo* pApplicationInfo,
    const uint32_t enabledLayerCount,
    const char* const* ppEnabledLayerNames,
    const uint32_t enabledExtensionCount,
    const char* const* ppEnabledExtensionNames) const;
bool ValidatePhysicalDeviceFeatures(const LogObjectList &_parentObjects,
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
    const VkBool32 inheritedQueries) const;
bool PreCallValidateCreateInstance(const VkInstanceCreateInfo* pCreateInfo,
    const VkAllocationCallbacks* pAllocator,
    VkInstance* pInstance) const override;
bool PreCallValidateDestroyInstance(VkInstance instance,
    const VkAllocationCallbacks* pAllocator) const override;
bool PreCallValidateEnumeratePhysicalDevices(VkInstance instance,
    uint32_t* pPhysicalDeviceCount,
    VkPhysicalDevice* pPhysicalDevices) const override;
bool PreCallValidateGetPhysicalDeviceFeatures(VkPhysicalDevice physicalDevice,
    VkPhysicalDeviceFeatures* pFeatures) const override;
bool PreCallValidateGetPhysicalDeviceFormatProperties(VkPhysicalDevice physicalDevice,
    VkFormat format,
    VkFormatProperties* pFormatProperties) const override;
bool PreCallValidateGetPhysicalDeviceImageFormatProperties(VkPhysicalDevice physicalDevice,
    VkFormat format,
    VkImageType type,
    VkImageTiling tiling,
    VkImageUsageFlags usage,
    VkImageCreateFlags flags,
    VkImageFormatProperties* pImageFormatProperties) const override;
bool PreCallValidateGetPhysicalDeviceProperties(VkPhysicalDevice physicalDevice,
    VkPhysicalDeviceProperties* pProperties) const override;
bool PreCallValidateGetPhysicalDeviceQueueFamilyProperties(VkPhysicalDevice physicalDevice,
    uint32_t* pQueueFamilyPropertyCount,
    VkQueueFamilyProperties* pQueueFamilyProperties) const override;
bool PreCallValidateGetPhysicalDeviceMemoryProperties(VkPhysicalDevice physicalDevice,
    VkPhysicalDeviceMemoryProperties* pMemoryProperties) const override;
bool PreCallValidateGetInstanceProcAddr(VkInstance instance,
    const char* pName) const override;
bool PreCallValidateGetDeviceProcAddr(VkDevice device,
    const char* pName) const override;
bool ValidateDeviceQueueCreateInfo(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const VkDeviceQueueCreateFlags flags,
    const uint32_t queueFamilyIndex,
    const uint32_t queueCount,
    const float* pQueuePriorities) const;
bool ValidateDeviceCreateInfo(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const VkDeviceCreateFlags flags,
    const uint32_t queueCreateInfoCount,
    const VkDeviceQueueCreateInfo* pQueueCreateInfos,
    const uint32_t enabledLayerCount,
    const char* const* ppEnabledLayerNames,
    const uint32_t enabledExtensionCount,
    const char* const* ppEnabledExtensionNames,
    const VkPhysicalDeviceFeatures* pEnabledFeatures) const;
bool PreCallValidateCreateDevice(VkPhysicalDevice physicalDevice,
    const VkDeviceCreateInfo* pCreateInfo,
    const VkAllocationCallbacks* pAllocator,
    VkDevice* pDevice) const override;
bool PreCallValidateDestroyDevice(VkDevice device,
    const VkAllocationCallbacks* pAllocator) const override;
bool PreCallValidateEnumerateInstanceExtensionProperties(const char* pLayerName,
    uint32_t* pPropertyCount,
    VkExtensionProperties* pProperties) const override;
bool PreCallValidateEnumerateDeviceExtensionProperties(VkPhysicalDevice physicalDevice,
    const char* pLayerName,
    uint32_t* pPropertyCount,
    VkExtensionProperties* pProperties) const override;
bool PreCallValidateEnumerateInstanceLayerProperties(uint32_t* pPropertyCount,
    VkLayerProperties* pProperties) const override;
bool PreCallValidateEnumerateDeviceLayerProperties(VkPhysicalDevice physicalDevice,
    uint32_t* pPropertyCount,
    VkLayerProperties* pProperties) const override;
bool ValidateSubmitInfo(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const uint32_t waitSemaphoreCount,
    const VkSemaphore* pWaitSemaphores,
    const VkPipelineStageFlags* pWaitDstStageMask,
    const uint32_t commandBufferCount,
    const VkCommandBuffer* pCommandBuffers,
    const uint32_t signalSemaphoreCount,
    const VkSemaphore* pSignalSemaphores) const;
bool PreCallValidateGetDeviceQueue(VkDevice device,
    uint32_t queueFamilyIndex,
    uint32_t queueIndex,
    VkQueue* pQueue) const override;
bool PreCallValidateQueueSubmit(VkQueue queue,
    uint32_t submitCount,
    const VkSubmitInfo* pSubmits,
    VkFence fence) const override;
bool PreCallValidateQueueWaitIdle(VkQueue queue) const override;
bool PreCallValidateDeviceWaitIdle(VkDevice device) const override;
bool ValidateMappedMemoryRange(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const VkDeviceMemory memory,
    const VkDeviceSize offset,
    const VkDeviceSize size) const;
bool ValidateMemoryAllocateInfo(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const VkDeviceSize allocationSize,
    const uint32_t memoryTypeIndex) const;
bool PreCallValidateAllocateMemory(VkDevice device,
    const VkMemoryAllocateInfo* pAllocateInfo,
    const VkAllocationCallbacks* pAllocator,
    VkDeviceMemory* pMemory) const override;
bool PreCallValidateFreeMemory(VkDevice device,
    VkDeviceMemory memory,
    const VkAllocationCallbacks* pAllocator) const override;
bool PreCallValidateMapMemory(VkDevice device,
    VkDeviceMemory memory,
    VkDeviceSize offset,
    VkDeviceSize size,
    VkMemoryMapFlags flags,
    void** ppData) const override;
bool PreCallValidateUnmapMemory(VkDevice device,
    VkDeviceMemory memory) const override;
bool PreCallValidateFlushMappedMemoryRanges(VkDevice device,
    uint32_t memoryRangeCount,
    const VkMappedMemoryRange* pMemoryRanges) const override;
bool PreCallValidateInvalidateMappedMemoryRanges(VkDevice device,
    uint32_t memoryRangeCount,
    const VkMappedMemoryRange* pMemoryRanges) const override;
bool PreCallValidateGetDeviceMemoryCommitment(VkDevice device,
    VkDeviceMemory memory,
    VkDeviceSize* pCommittedMemoryInBytes) const override;
bool PreCallValidateBindBufferMemory(VkDevice device,
    VkBuffer buffer,
    VkDeviceMemory memory,
    VkDeviceSize memoryOffset) const override;
bool PreCallValidateBindImageMemory(VkDevice device,
    VkImage image,
    VkDeviceMemory memory,
    VkDeviceSize memoryOffset) const override;
bool PreCallValidateGetBufferMemoryRequirements(VkDevice device,
    VkBuffer buffer,
    VkMemoryRequirements* pMemoryRequirements) const override;
bool PreCallValidateGetImageMemoryRequirements(VkDevice device,
    VkImage image,
    VkMemoryRequirements* pMemoryRequirements) const override;
bool ValidateSparseMemoryBind(const LogObjectList &_parentObjects,
    const VkDeviceSize resourceOffset,
    const VkDeviceSize size,
    const VkDeviceMemory memory,
    const VkDeviceSize memoryOffset,
    const VkSparseMemoryBindFlags flags) const;
bool ValidateSparseBufferMemoryBindInfo(const LogObjectList &_parentObjects,
    const VkBuffer buffer,
    const uint32_t bindCount,
    const VkSparseMemoryBind* pBinds) const;
bool ValidateSparseImageOpaqueMemoryBindInfo(const LogObjectList &_parentObjects,
    const VkImage image,
    const uint32_t bindCount,
    const VkSparseMemoryBind* pBinds) const;
bool ValidateImageSubresource(const LogObjectList &_parentObjects,
    const VkImageAspectFlags aspectMask,
    const uint32_t mipLevel,
    const uint32_t arrayLayer) const;
bool ValidateSparseImageMemoryBind(const LogObjectList &_parentObjects,
    const VkImageSubresource subresource,
    const VkOffset3D offset,
    const VkExtent3D extent,
    const VkDeviceMemory memory,
    const VkDeviceSize memoryOffset,
    const VkSparseMemoryBindFlags flags) const;
bool ValidateSparseImageMemoryBindInfo(const LogObjectList &_parentObjects,
    const VkImage image,
    const uint32_t bindCount,
    const VkSparseImageMemoryBind* pBinds) const;
bool ValidateBindSparseInfo(const LogObjectList &_parentObjects,
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
    const VkSemaphore* pSignalSemaphores) const;
bool PreCallValidateGetImageSparseMemoryRequirements(VkDevice device,
    VkImage image,
    uint32_t* pSparseMemoryRequirementCount,
    VkSparseImageMemoryRequirements* pSparseMemoryRequirements) const override;
bool PreCallValidateGetPhysicalDeviceSparseImageFormatProperties(VkPhysicalDevice physicalDevice,
    VkFormat format,
    VkImageType type,
    VkSampleCountFlagBits samples,
    VkImageUsageFlags usage,
    VkImageTiling tiling,
    uint32_t* pPropertyCount,
    VkSparseImageFormatProperties* pProperties) const override;
bool PreCallValidateQueueBindSparse(VkQueue queue,
    uint32_t bindInfoCount,
    const VkBindSparseInfo* pBindInfo,
    VkFence fence) const override;
bool ValidateFenceCreateInfo(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const VkFenceCreateFlags flags) const;
bool PreCallValidateCreateFence(VkDevice device,
    const VkFenceCreateInfo* pCreateInfo,
    const VkAllocationCallbacks* pAllocator,
    VkFence* pFence) const override;
bool PreCallValidateDestroyFence(VkDevice device,
    VkFence fence,
    const VkAllocationCallbacks* pAllocator) const override;
bool PreCallValidateResetFences(VkDevice device,
    uint32_t fenceCount,
    const VkFence* pFences) const override;
bool PreCallValidateGetFenceStatus(VkDevice device,
    VkFence fence) const override;
bool PreCallValidateWaitForFences(VkDevice device,
    uint32_t fenceCount,
    const VkFence* pFences,
    VkBool32 waitAll,
    uint64_t timeout) const override;
bool ValidateSemaphoreCreateInfo(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const VkSemaphoreCreateFlags flags) const;
bool PreCallValidateCreateSemaphore(VkDevice device,
    const VkSemaphoreCreateInfo* pCreateInfo,
    const VkAllocationCallbacks* pAllocator,
    VkSemaphore* pSemaphore) const override;
bool PreCallValidateDestroySemaphore(VkDevice device,
    VkSemaphore semaphore,
    const VkAllocationCallbacks* pAllocator) const override;
bool ValidateEventCreateInfo(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const VkEventCreateFlags flags) const;
bool PreCallValidateCreateEvent(VkDevice device,
    const VkEventCreateInfo* pCreateInfo,
    const VkAllocationCallbacks* pAllocator,
    VkEvent* pEvent) const override;
bool PreCallValidateDestroyEvent(VkDevice device,
    VkEvent event,
    const VkAllocationCallbacks* pAllocator) const override;
bool PreCallValidateGetEventStatus(VkDevice device,
    VkEvent event) const override;
bool PreCallValidateSetEvent(VkDevice device,
    VkEvent event) const override;
bool PreCallValidateResetEvent(VkDevice device,
    VkEvent event) const override;
bool ValidateQueryPoolCreateInfo(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const VkQueryPoolCreateFlags flags,
    const VkQueryType queryType,
    const uint32_t queryCount,
    const VkQueryPipelineStatisticFlags pipelineStatistics) const;
bool PreCallValidateCreateQueryPool(VkDevice device,
    const VkQueryPoolCreateInfo* pCreateInfo,
    const VkAllocationCallbacks* pAllocator,
    VkQueryPool* pQueryPool) const override;
bool PreCallValidateDestroyQueryPool(VkDevice device,
    VkQueryPool queryPool,
    const VkAllocationCallbacks* pAllocator) const override;
bool PreCallValidateGetQueryPoolResults(VkDevice device,
    VkQueryPool queryPool,
    uint32_t firstQuery,
    uint32_t queryCount,
    size_t dataSize,
    void* pData,
    VkDeviceSize stride,
    VkQueryResultFlags flags) const override;
bool ValidateBufferCreateInfo(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const VkBufferCreateFlags flags,
    const VkDeviceSize size,
    const VkBufferUsageFlags usage,
    const VkSharingMode sharingMode,
    const uint32_t queueFamilyIndexCount,
    const uint32_t* pQueueFamilyIndices) const;
bool PreCallValidateCreateBuffer(VkDevice device,
    const VkBufferCreateInfo* pCreateInfo,
    const VkAllocationCallbacks* pAllocator,
    VkBuffer* pBuffer) const override;
bool PreCallValidateDestroyBuffer(VkDevice device,
    VkBuffer buffer,
    const VkAllocationCallbacks* pAllocator) const override;
bool ValidateBufferViewCreateInfo(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const VkBufferViewCreateFlags flags,
    const VkBuffer buffer,
    const VkFormat format,
    const VkDeviceSize offset,
    const VkDeviceSize range) const;
bool PreCallValidateCreateBufferView(VkDevice device,
    const VkBufferViewCreateInfo* pCreateInfo,
    const VkAllocationCallbacks* pAllocator,
    VkBufferView* pView) const override;
bool PreCallValidateDestroyBufferView(VkDevice device,
    VkBufferView bufferView,
    const VkAllocationCallbacks* pAllocator) const override;
bool ValidateImageCreateInfo(const LogObjectList &_parentObjects,
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
    const VkImageLayout initialLayout) const;
bool ValidateSubresourceLayout(const LogObjectList &_parentObjects,
    const VkDeviceSize offset,
    const VkDeviceSize size,
    const VkDeviceSize rowPitch,
    const VkDeviceSize arrayPitch,
    const VkDeviceSize depthPitch) const;
bool PreCallValidateCreateImage(VkDevice device,
    const VkImageCreateInfo* pCreateInfo,
    const VkAllocationCallbacks* pAllocator,
    VkImage* pImage) const override;
bool PreCallValidateDestroyImage(VkDevice device,
    VkImage image,
    const VkAllocationCallbacks* pAllocator) const override;
bool PreCallValidateGetImageSubresourceLayout(VkDevice device,
    VkImage image,
    const VkImageSubresource* pSubresource,
    VkSubresourceLayout* pLayout) const override;
bool ValidateComponentMapping(const LogObjectList &_parentObjects,
    const VkComponentSwizzle r,
    const VkComponentSwizzle g,
    const VkComponentSwizzle b,
    const VkComponentSwizzle a) const;
bool ValidateImageViewCreateInfo(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const VkImageViewCreateFlags flags,
    const VkImage image,
    const VkImageViewType viewType,
    const VkFormat format,
    const VkComponentMapping components,
    const VkImageSubresourceRange subresourceRange) const;
bool PreCallValidateCreateImageView(VkDevice device,
    const VkImageViewCreateInfo* pCreateInfo,
    const VkAllocationCallbacks* pAllocator,
    VkImageView* pView) const override;
bool PreCallValidateDestroyImageView(VkDevice device,
    VkImageView imageView,
    const VkAllocationCallbacks* pAllocator) const override;
bool ValidateShaderModuleCreateInfo(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const VkShaderModuleCreateFlags flags,
    const size_t codeSize,
    const uint32_t* pCode) const;
bool PreCallValidateCreateShaderModule(VkDevice device,
    const VkShaderModuleCreateInfo* pCreateInfo,
    const VkAllocationCallbacks* pAllocator,
    VkShaderModule* pShaderModule) const override;
bool PreCallValidateDestroyShaderModule(VkDevice device,
    VkShaderModule shaderModule,
    const VkAllocationCallbacks* pAllocator) const override;
bool ValidatePipelineCacheCreateInfo(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const VkPipelineCacheCreateFlags flags,
    const size_t initialDataSize,
    const void* pInitialData) const;
bool PreCallValidateCreatePipelineCache(VkDevice device,
    const VkPipelineCacheCreateInfo* pCreateInfo,
    const VkAllocationCallbacks* pAllocator,
    VkPipelineCache* pPipelineCache) const override;
bool PreCallValidateDestroyPipelineCache(VkDevice device,
    VkPipelineCache pipelineCache,
    const VkAllocationCallbacks* pAllocator) const override;
bool PreCallValidateGetPipelineCacheData(VkDevice device,
    VkPipelineCache pipelineCache,
    size_t* pDataSize,
    void* pData) const override;
bool PreCallValidateMergePipelineCaches(VkDevice device,
    VkPipelineCache dstCache,
    uint32_t srcCacheCount,
    const VkPipelineCache* pSrcCaches) const override;
bool ValidateSpecializationMapEntry(const LogObjectList &_parentObjects,
    uint32_t constantID,
    const uint32_t offset,
    const size_t size) const;
bool ValidateSpecializationInfo(const LogObjectList &_parentObjects,
    const uint32_t mapEntryCount,
    const VkSpecializationMapEntry* pMapEntries,
    const size_t dataSize,
    const void* pData) const;
bool ValidatePipelineShaderStageCreateInfo(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const VkPipelineShaderStageCreateFlags flags,
    const VkShaderStageFlagBits stage,
    const VkShaderModule module,
    const char* pName,
    const VkSpecializationInfo* pSpecializationInfo) const;
bool ValidateComputePipelineCreateInfo(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const VkPipelineCreateFlags flags,
    const VkPipelineShaderStageCreateInfo stage,
    const VkPipelineLayout layout,
    const VkPipeline basePipelineHandle,
    const int32_t basePipelineIndex) const;
bool ValidateVertexInputBindingDescription(const LogObjectList &_parentObjects,
    const uint32_t binding,
    const uint32_t stride,
    const VkVertexInputRate inputRate) const;
bool ValidateVertexInputAttributeDescription(const LogObjectList &_parentObjects,
    const uint32_t location,
    const uint32_t binding,
    const VkFormat format,
    const uint32_t offset) const;
bool ValidatePipelineVertexInputStateCreateInfo(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const VkPipelineVertexInputStateCreateFlags flags,
    const uint32_t vertexBindingDescriptionCount,
    const VkVertexInputBindingDescription* pVertexBindingDescriptions,
    const uint32_t vertexAttributeDescriptionCount,
    const VkVertexInputAttributeDescription* pVertexAttributeDescriptions) const;
bool ValidatePipelineInputAssemblyStateCreateInfo(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const VkPipelineInputAssemblyStateCreateFlags flags,
    const VkPrimitiveTopology topology,
    const VkBool32 primitiveRestartEnable) const;
bool ValidatePipelineTessellationStateCreateInfo(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const VkPipelineTessellationStateCreateFlags flags,
    const uint32_t patchControlPoints) const;
bool ValidateViewport(const LogObjectList &_parentObjects,
    const float x,
    const float y,
    const float width,
    const float height,
    const float minDepth,
    const float maxDepth) const;
bool ValidatePipelineViewportStateCreateInfo(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const VkPipelineViewportStateCreateFlags flags,
    const uint32_t viewportCount,
    const VkViewport* pViewports,
    const uint32_t scissorCount,
    const VkRect2D* pScissors) const;
bool ValidatePipelineRasterizationStateCreateInfo(const LogObjectList &_parentObjects,
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
    const float lineWidth) const;
bool ValidatePipelineMultisampleStateCreateInfo(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const VkPipelineMultisampleStateCreateFlags flags,
    const VkSampleCountFlagBits rasterizationSamples,
    const VkBool32 sampleShadingEnable,
    const float minSampleShading,
    const VkSampleMask* pSampleMask,
    const VkBool32 alphaToCoverageEnable,
    const VkBool32 alphaToOneEnable) const;
bool ValidateStencilOpState(const LogObjectList &_parentObjects,
    const VkStencilOp failOp,
    const VkStencilOp passOp,
    const VkStencilOp depthFailOp,
    const VkCompareOp compareOp,
    const uint32_t compareMask,
    const uint32_t writeMask,
    const uint32_t reference) const;
bool ValidatePipelineDepthStencilStateCreateInfo(const LogObjectList &_parentObjects,
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
    const float maxDepthBounds) const;
bool ValidatePipelineColorBlendAttachmentState(const LogObjectList &_parentObjects,
    const VkBool32 blendEnable,
    const VkBlendFactor srcColorBlendFactor,
    const VkBlendFactor dstColorBlendFactor,
    const VkBlendOp colorBlendOp,
    const VkBlendFactor srcAlphaBlendFactor,
    const VkBlendFactor dstAlphaBlendFactor,
    const VkBlendOp alphaBlendOp,
    const VkColorComponentFlags colorWriteMask) const;
bool ValidatePipelineColorBlendStateCreateInfo(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const VkPipelineColorBlendStateCreateFlags flags,
    const VkBool32 logicOpEnable,
    const VkLogicOp logicOp,
    const uint32_t attachmentCount,
    const VkPipelineColorBlendAttachmentState* pAttachments,
    const float blendConstants[4]) const;
bool ValidatePipelineDynamicStateCreateInfo(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const VkPipelineDynamicStateCreateFlags flags,
    const uint32_t dynamicStateCount,
    const VkDynamicState* pDynamicStates) const;
bool ValidateGraphicsPipelineCreateInfo(const LogObjectList &_parentObjects,
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
    const int32_t basePipelineIndex) const;
bool PreCallValidateCreateGraphicsPipelines(VkDevice device,
    VkPipelineCache pipelineCache,
    uint32_t createInfoCount,
    const VkGraphicsPipelineCreateInfo* pCreateInfos,
    const VkAllocationCallbacks* pAllocator,
    VkPipeline* pPipelines,
    void *validation_state) const override;
bool PreCallValidateCreateComputePipelines(VkDevice device,
    VkPipelineCache pipelineCache,
    uint32_t createInfoCount,
    const VkComputePipelineCreateInfo* pCreateInfos,
    const VkAllocationCallbacks* pAllocator,
    VkPipeline* pPipelines,
    void *validation_state) const override;
bool PreCallValidateDestroyPipeline(VkDevice device,
    VkPipeline pipeline,
    const VkAllocationCallbacks* pAllocator) const override;
bool ValidatePushConstantRange(const LogObjectList &_parentObjects,
    const VkShaderStageFlags stageFlags,
    const uint32_t offset,
    const uint32_t size) const;
bool ValidatePipelineLayoutCreateInfo(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const VkPipelineLayoutCreateFlags flags,
    const uint32_t setLayoutCount,
    const VkDescriptorSetLayout* pSetLayouts,
    const uint32_t pushConstantRangeCount,
    const VkPushConstantRange* pPushConstantRanges) const;
bool PreCallValidateCreatePipelineLayout(VkDevice device,
    const VkPipelineLayoutCreateInfo* pCreateInfo,
    const VkAllocationCallbacks* pAllocator,
    VkPipelineLayout* pPipelineLayout) const override;
bool PreCallValidateDestroyPipelineLayout(VkDevice device,
    VkPipelineLayout pipelineLayout,
    const VkAllocationCallbacks* pAllocator) const override;
bool ValidateSamplerCreateInfo(const LogObjectList &_parentObjects,
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
    const VkBool32 unnormalizedCoordinates) const;
bool PreCallValidateCreateSampler(VkDevice device,
    const VkSamplerCreateInfo* pCreateInfo,
    const VkAllocationCallbacks* pAllocator,
    VkSampler* pSampler) const override;
bool PreCallValidateDestroySampler(VkDevice device,
    VkSampler sampler,
    const VkAllocationCallbacks* pAllocator) const override;
bool ValidateCopyDescriptorSet(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const VkDescriptorSet srcSet,
    const uint32_t srcBinding,
    const uint32_t srcArrayElement,
    const VkDescriptorSet dstSet,
    const uint32_t dstBinding,
    const uint32_t dstArrayElement,
    const uint32_t descriptorCount) const;
bool ValidateDescriptorBufferInfo(const LogObjectList &_parentObjects,
    const VkBuffer buffer,
    const VkDeviceSize offset,
    const VkDeviceSize range) const;
bool ValidateDescriptorImageInfo(const LogObjectList &_parentObjects,
    const VkSampler sampler,
    const VkImageView imageView,
    const VkImageLayout imageLayout) const;
bool ValidateDescriptorPoolSize(const LogObjectList &_parentObjects,
    const VkDescriptorType type,
    const uint32_t descriptorCount) const;
bool ValidateDescriptorPoolCreateInfo(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const VkDescriptorPoolCreateFlags flags,
    const uint32_t maxSets,
    const uint32_t poolSizeCount,
    const VkDescriptorPoolSize* pPoolSizes) const;
bool ValidateDescriptorSetAllocateInfo(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const VkDescriptorPool descriptorPool,
    const uint32_t descriptorSetCount,
    const VkDescriptorSetLayout* pSetLayouts) const;
bool ValidateDescriptorSetLayoutBinding(const LogObjectList &_parentObjects,
    const uint32_t binding,
    const VkDescriptorType descriptorType,
    const uint32_t descriptorCount,
    const VkShaderStageFlags stageFlags,
    const VkSampler* pImmutableSamplers) const;
bool ValidateDescriptorSetLayoutCreateInfo(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const VkDescriptorSetLayoutCreateFlags flags,
    const uint32_t bindingCount,
    const VkDescriptorSetLayoutBinding* pBindings) const;
bool ValidateWriteDescriptorSet(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const VkDescriptorSet dstSet,
    const uint32_t dstBinding,
    const uint32_t dstArrayElement,
    const uint32_t descriptorCount,
    const VkDescriptorType descriptorType,
    const VkDescriptorImageInfo* pImageInfo,
    const VkDescriptorBufferInfo* pBufferInfo,
    const VkBufferView* pTexelBufferView) const;
bool PreCallValidateCreateDescriptorSetLayout(VkDevice device,
    const VkDescriptorSetLayoutCreateInfo* pCreateInfo,
    const VkAllocationCallbacks* pAllocator,
    VkDescriptorSetLayout* pSetLayout) const override;
bool PreCallValidateDestroyDescriptorSetLayout(VkDevice device,
    VkDescriptorSetLayout descriptorSetLayout,
    const VkAllocationCallbacks* pAllocator) const override;
bool PreCallValidateCreateDescriptorPool(VkDevice device,
    const VkDescriptorPoolCreateInfo* pCreateInfo,
    const VkAllocationCallbacks* pAllocator,
    VkDescriptorPool* pDescriptorPool) const override;
bool PreCallValidateDestroyDescriptorPool(VkDevice device,
    VkDescriptorPool descriptorPool,
    const VkAllocationCallbacks* pAllocator) const override;
bool PreCallValidateResetDescriptorPool(VkDevice device,
    VkDescriptorPool descriptorPool,
    VkDescriptorPoolResetFlags flags) const override;
bool PreCallValidateAllocateDescriptorSets(VkDevice device,
    const VkDescriptorSetAllocateInfo* pAllocateInfo,
    VkDescriptorSet* pDescriptorSets,
    void *validation_state) const override;
bool PreCallValidateFreeDescriptorSets(VkDevice device,
    VkDescriptorPool descriptorPool,
    uint32_t descriptorSetCount,
    const VkDescriptorSet* pDescriptorSets) const override;
bool PreCallValidateUpdateDescriptorSets(VkDevice device,
    uint32_t descriptorWriteCount,
    const VkWriteDescriptorSet* pDescriptorWrites,
    uint32_t descriptorCopyCount,
    const VkCopyDescriptorSet* pDescriptorCopies) const override;
bool ValidateAttachmentDescription(const LogObjectList &_parentObjects,
    const VkAttachmentDescriptionFlags flags,
    const VkFormat format,
    const VkSampleCountFlagBits samples,
    const VkAttachmentLoadOp loadOp,
    const VkAttachmentStoreOp storeOp,
    const VkAttachmentLoadOp stencilLoadOp,
    const VkAttachmentStoreOp stencilStoreOp,
    const VkImageLayout initialLayout,
    const VkImageLayout finalLayout) const;
bool ValidateAttachmentReference(const LogObjectList &_parentObjects,
    const uint32_t attachment,
    const VkImageLayout layout) const;
bool ValidateFramebufferCreateInfo(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const VkFramebufferCreateFlags flags,
    const VkRenderPass renderPass,
    const uint32_t attachmentCount,
    const VkImageView* pAttachments,
    const uint32_t width,
    const uint32_t height,
    const uint32_t layers) const;
bool ValidateSubpassDescription(const LogObjectList &_parentObjects,
    const VkSubpassDescriptionFlags flags,
    const VkPipelineBindPoint pipelineBindPoint,
    const uint32_t inputAttachmentCount,
    const VkAttachmentReference* pInputAttachments,
    const uint32_t colorAttachmentCount,
    const VkAttachmentReference* pColorAttachments,
    const VkAttachmentReference* pResolveAttachments,
    const VkAttachmentReference* pDepthStencilAttachment,
    const uint32_t preserveAttachmentCount,
    const uint32_t* pPreserveAttachments) const;
bool ValidateSubpassDependency(const LogObjectList &_parentObjects,
    const uint32_t srcSubpass,
    const uint32_t dstSubpass,
    const VkPipelineStageFlags srcStageMask,
    const VkPipelineStageFlags dstStageMask,
    const VkAccessFlags srcAccessMask,
    const VkAccessFlags dstAccessMask,
    const VkDependencyFlags dependencyFlags) const;
bool ValidateRenderPassCreateInfo(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const VkRenderPassCreateFlags flags,
    const uint32_t attachmentCount,
    const VkAttachmentDescription* pAttachments,
    const uint32_t subpassCount,
    const VkSubpassDescription* pSubpasses,
    const uint32_t dependencyCount,
    const VkSubpassDependency* pDependencies) const;
bool PreCallValidateCreateFramebuffer(VkDevice device,
    const VkFramebufferCreateInfo* pCreateInfo,
    const VkAllocationCallbacks* pAllocator,
    VkFramebuffer* pFramebuffer) const override;
bool PreCallValidateDestroyFramebuffer(VkDevice device,
    VkFramebuffer framebuffer,
    const VkAllocationCallbacks* pAllocator) const override;
bool PreCallValidateCreateRenderPass(VkDevice device,
    const VkRenderPassCreateInfo* pCreateInfo,
    const VkAllocationCallbacks* pAllocator,
    VkRenderPass* pRenderPass) const override;
bool PreCallValidateDestroyRenderPass(VkDevice device,
    VkRenderPass renderPass,
    const VkAllocationCallbacks* pAllocator) const override;
bool PreCallValidateGetRenderAreaGranularity(VkDevice device,
    VkRenderPass renderPass,
    VkExtent2D* pGranularity) const override;
bool ValidateCommandPoolCreateInfo(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const VkCommandPoolCreateFlags flags,
    const uint32_t queueFamilyIndex) const;
bool PreCallValidateCreateCommandPool(VkDevice device,
    const VkCommandPoolCreateInfo* pCreateInfo,
    const VkAllocationCallbacks* pAllocator,
    VkCommandPool* pCommandPool) const override;
bool PreCallValidateDestroyCommandPool(VkDevice device,
    VkCommandPool commandPool,
    const VkAllocationCallbacks* pAllocator) const override;
bool PreCallValidateResetCommandPool(VkDevice device,
    VkCommandPool commandPool,
    VkCommandPoolResetFlags flags) const override;
bool ValidateCommandBufferAllocateInfo(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const VkCommandPool commandPool,
    const VkCommandBufferLevel level,
    const uint32_t commandBufferCount) const;
bool ValidateCommandBufferInheritanceInfo(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const VkRenderPass renderPass,
    const uint32_t subpass,
    const VkFramebuffer framebuffer,
    const VkBool32 occlusionQueryEnable,
    const VkQueryControlFlags queryFlags,
    const VkQueryPipelineStatisticFlags pipelineStatistics) const;
bool ValidateCommandBufferBeginInfo(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const VkCommandBufferUsageFlags flags,
    const VkCommandBufferInheritanceInfo* pInheritanceInfo) const;
bool PreCallValidateAllocateCommandBuffers(VkDevice device,
    const VkCommandBufferAllocateInfo* pAllocateInfo,
    VkCommandBuffer* pCommandBuffers) const override;
bool PreCallValidateFreeCommandBuffers(VkDevice device,
    VkCommandPool commandPool,
    uint32_t commandBufferCount,
    const VkCommandBuffer* pCommandBuffers) const override;
bool PreCallValidateBeginCommandBuffer(VkCommandBuffer commandBuffer,
    const VkCommandBufferBeginInfo* pBeginInfo) const override;
bool PreCallValidateEndCommandBuffer(VkCommandBuffer commandBuffer) const override;
bool PreCallValidateResetCommandBuffer(VkCommandBuffer commandBuffer,
    VkCommandBufferResetFlags flags) const override;
bool ValidateBufferCopy(const LogObjectList &_parentObjects,
    const VkDeviceSize srcOffset,
    const VkDeviceSize dstOffset,
    const VkDeviceSize size) const;
bool ValidateImageSubresourceLayers(const LogObjectList &_parentObjects,
    const VkImageAspectFlags aspectMask,
    const uint32_t mipLevel,
    const uint32_t baseArrayLayer,
    const uint32_t layerCount) const;
bool ValidateBufferImageCopy(const LogObjectList &_parentObjects,
    const VkDeviceSize bufferOffset,
    const uint32_t bufferRowLength,
    const uint32_t bufferImageHeight,
    const VkImageSubresourceLayers imageSubresource,
    const VkOffset3D imageOffset,
    const VkExtent3D imageExtent) const;
bool ValidateClearColorValue(const LogObjectList &_parentObjects,
    const float float32[4],
    const int32_t int32[4],
    const uint32_t uint32[4]) const;
bool ValidateClearDepthStencilValue(const LogObjectList &_parentObjects,
    const float depth,
    const uint32_t stencil) const;
bool ValidateClearValue(const LogObjectList &_parentObjects,
    const VkClearColorValue color,
    const VkClearDepthStencilValue depthStencil) const;
bool ValidateClearAttachment(const LogObjectList &_parentObjects,
    const VkImageAspectFlags aspectMask,
    const uint32_t colorAttachment,
    const VkClearValue clearValue) const;
bool ValidateClearRect(const LogObjectList &_parentObjects,
    const VkRect2D rect,
    const uint32_t baseArrayLayer,
    const uint32_t layerCount) const;
bool ValidateImageBlit(const LogObjectList &_parentObjects,
    const VkImageSubresourceLayers srcSubresource,
    const VkOffset3D srcOffsets[2],
    const VkImageSubresourceLayers dstSubresource,
    const VkOffset3D dstOffsets[2]) const;
bool ValidateImageCopy(const LogObjectList &_parentObjects,
    const VkImageSubresourceLayers srcSubresource,
    const VkOffset3D srcOffset,
    const VkImageSubresourceLayers dstSubresource,
    const VkOffset3D dstOffset,
    const VkExtent3D extent) const;
bool ValidateImageResolve(const LogObjectList &_parentObjects,
    const VkImageSubresourceLayers srcSubresource,
    const VkOffset3D srcOffset,
    const VkImageSubresourceLayers dstSubresource,
    const VkOffset3D dstOffset,
    const VkExtent3D extent) const;
bool ValidateRenderPassBeginInfo(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const VkRenderPass renderPass,
    const VkFramebuffer framebuffer,
    const VkRect2D renderArea,
    const uint32_t clearValueCount,
    const VkClearValue* pClearValues) const;
bool PreCallValidateCmdBindPipeline(VkCommandBuffer commandBuffer,
    VkPipelineBindPoint pipelineBindPoint,
    VkPipeline pipeline) const override;
bool PreCallValidateCmdSetViewport(VkCommandBuffer commandBuffer,
    uint32_t firstViewport,
    uint32_t viewportCount,
    const VkViewport* pViewports) const override;
bool PreCallValidateCmdSetScissor(VkCommandBuffer commandBuffer,
    uint32_t firstScissor,
    uint32_t scissorCount,
    const VkRect2D* pScissors) const override;
bool PreCallValidateCmdSetLineWidth(VkCommandBuffer commandBuffer,
    float lineWidth) const override;
bool PreCallValidateCmdSetDepthBias(VkCommandBuffer commandBuffer,
    float depthBiasConstantFactor,
    float depthBiasClamp,
    float depthBiasSlopeFactor) const override;
bool PreCallValidateCmdSetBlendConstants(VkCommandBuffer commandBuffer,
    const float blendConstants[4]) const override;
bool PreCallValidateCmdSetDepthBounds(VkCommandBuffer commandBuffer,
    float minDepthBounds,
    float maxDepthBounds) const override;
bool PreCallValidateCmdSetStencilCompareMask(VkCommandBuffer commandBuffer,
    VkStencilFaceFlags faceMask,
    uint32_t compareMask) const override;
bool PreCallValidateCmdSetStencilWriteMask(VkCommandBuffer commandBuffer,
    VkStencilFaceFlags faceMask,
    uint32_t writeMask) const override;
bool PreCallValidateCmdSetStencilReference(VkCommandBuffer commandBuffer,
    VkStencilFaceFlags faceMask,
    uint32_t reference) const override;
bool PreCallValidateCmdBindDescriptorSets(VkCommandBuffer commandBuffer,
    VkPipelineBindPoint pipelineBindPoint,
    VkPipelineLayout layout,
    uint32_t firstSet,
    uint32_t descriptorSetCount,
    const VkDescriptorSet* pDescriptorSets,
    uint32_t dynamicOffsetCount,
    const uint32_t* pDynamicOffsets) const override;
bool PreCallValidateCmdBindIndexBuffer(VkCommandBuffer commandBuffer,
    VkBuffer buffer,
    VkDeviceSize offset,
    VkIndexType indexType) const override;
bool PreCallValidateCmdBindVertexBuffers(VkCommandBuffer commandBuffer,
    uint32_t firstBinding,
    uint32_t bindingCount,
    const VkBuffer* pBuffers,
    const VkDeviceSize* pOffsets) const override;
bool PreCallValidateCmdDraw(VkCommandBuffer commandBuffer,
    uint32_t vertexCount,
    uint32_t instanceCount,
    uint32_t firstVertex,
    uint32_t firstInstance) const override;
bool PreCallValidateCmdDrawIndexed(VkCommandBuffer commandBuffer,
    uint32_t indexCount,
    uint32_t instanceCount,
    uint32_t firstIndex,
    int32_t vertexOffset,
    uint32_t firstInstance) const override;
bool PreCallValidateCmdDrawIndirect(VkCommandBuffer commandBuffer,
    VkBuffer buffer,
    VkDeviceSize offset,
    uint32_t drawCount,
    uint32_t stride) const override;
bool PreCallValidateCmdDrawIndexedIndirect(VkCommandBuffer commandBuffer,
    VkBuffer buffer,
    VkDeviceSize offset,
    uint32_t drawCount,
    uint32_t stride) const override;
bool PreCallValidateCmdDispatch(VkCommandBuffer commandBuffer,
    uint32_t groupCountX,
    uint32_t groupCountY,
    uint32_t groupCountZ) const override;
bool PreCallValidateCmdDispatchIndirect(VkCommandBuffer commandBuffer,
    VkBuffer buffer,
    VkDeviceSize offset) const override;
bool PreCallValidateCmdCopyBuffer(VkCommandBuffer commandBuffer,
    VkBuffer srcBuffer,
    VkBuffer dstBuffer,
    uint32_t regionCount,
    const VkBufferCopy* pRegions) const override;
bool PreCallValidateCmdCopyImage(VkCommandBuffer commandBuffer,
    VkImage srcImage,
    VkImageLayout srcImageLayout,
    VkImage dstImage,
    VkImageLayout dstImageLayout,
    uint32_t regionCount,
    const VkImageCopy* pRegions) const override;
bool PreCallValidateCmdBlitImage(VkCommandBuffer commandBuffer,
    VkImage srcImage,
    VkImageLayout srcImageLayout,
    VkImage dstImage,
    VkImageLayout dstImageLayout,
    uint32_t regionCount,
    const VkImageBlit* pRegions,
    VkFilter filter) const override;
bool PreCallValidateCmdCopyBufferToImage(VkCommandBuffer commandBuffer,
    VkBuffer srcBuffer,
    VkImage dstImage,
    VkImageLayout dstImageLayout,
    uint32_t regionCount,
    const VkBufferImageCopy* pRegions) const override;
bool PreCallValidateCmdCopyImageToBuffer(VkCommandBuffer commandBuffer,
    VkImage srcImage,
    VkImageLayout srcImageLayout,
    VkBuffer dstBuffer,
    uint32_t regionCount,
    const VkBufferImageCopy* pRegions) const override;
bool PreCallValidateCmdUpdateBuffer(VkCommandBuffer commandBuffer,
    VkBuffer dstBuffer,
    VkDeviceSize dstOffset,
    VkDeviceSize dataSize,
    const void* pData) const override;
bool PreCallValidateCmdFillBuffer(VkCommandBuffer commandBuffer,
    VkBuffer dstBuffer,
    VkDeviceSize dstOffset,
    VkDeviceSize size,
    uint32_t data) const override;
bool PreCallValidateCmdClearColorImage(VkCommandBuffer commandBuffer,
    VkImage image,
    VkImageLayout imageLayout,
    const VkClearColorValue* pColor,
    uint32_t rangeCount,
    const VkImageSubresourceRange* pRanges) const override;
bool PreCallValidateCmdClearDepthStencilImage(VkCommandBuffer commandBuffer,
    VkImage image,
    VkImageLayout imageLayout,
    const VkClearDepthStencilValue* pDepthStencil,
    uint32_t rangeCount,
    const VkImageSubresourceRange* pRanges) const override;
bool PreCallValidateCmdClearAttachments(VkCommandBuffer commandBuffer,
    uint32_t attachmentCount,
    const VkClearAttachment* pAttachments,
    uint32_t rectCount,
    const VkClearRect* pRects) const override;
bool PreCallValidateCmdResolveImage(VkCommandBuffer commandBuffer,
    VkImage srcImage,
    VkImageLayout srcImageLayout,
    VkImage dstImage,
    VkImageLayout dstImageLayout,
    uint32_t regionCount,
    const VkImageResolve* pRegions) const override;
bool PreCallValidateCmdSetEvent(VkCommandBuffer commandBuffer,
    VkEvent event,
    VkPipelineStageFlags stageMask) const override;
bool PreCallValidateCmdResetEvent(VkCommandBuffer commandBuffer,
    VkEvent event,
    VkPipelineStageFlags stageMask) const override;
bool PreCallValidateCmdWaitEvents(VkCommandBuffer commandBuffer,
    uint32_t eventCount,
    const VkEvent* pEvents,
    VkPipelineStageFlags srcStageMask,
    VkPipelineStageFlags dstStageMask,
    uint32_t memoryBarrierCount,
    const VkMemoryBarrier* pMemoryBarriers,
    uint32_t bufferMemoryBarrierCount,
    const VkBufferMemoryBarrier* pBufferMemoryBarriers,
    uint32_t imageMemoryBarrierCount,
    const VkImageMemoryBarrier* pImageMemoryBarriers) const override;
bool PreCallValidateCmdPipelineBarrier(VkCommandBuffer commandBuffer,
    VkPipelineStageFlags srcStageMask,
    VkPipelineStageFlags dstStageMask,
    VkDependencyFlags dependencyFlags,
    uint32_t memoryBarrierCount,
    const VkMemoryBarrier* pMemoryBarriers,
    uint32_t bufferMemoryBarrierCount,
    const VkBufferMemoryBarrier* pBufferMemoryBarriers,
    uint32_t imageMemoryBarrierCount,
    const VkImageMemoryBarrier* pImageMemoryBarriers) const override;
bool PreCallValidateCmdBeginQuery(VkCommandBuffer commandBuffer,
    VkQueryPool queryPool,
    uint32_t query,
    VkQueryControlFlags flags) const override;
bool PreCallValidateCmdEndQuery(VkCommandBuffer commandBuffer,
    VkQueryPool queryPool,
    uint32_t query) const override;
bool PreCallValidateCmdResetQueryPool(VkCommandBuffer commandBuffer,
    VkQueryPool queryPool,
    uint32_t firstQuery,
    uint32_t queryCount) const override;
bool PreCallValidateCmdWriteTimestamp(VkCommandBuffer commandBuffer,
    VkPipelineStageFlagBits pipelineStage,
    VkQueryPool queryPool,
    uint32_t query) const override;
bool PreCallValidateCmdCopyQueryPoolResults(VkCommandBuffer commandBuffer,
    VkQueryPool queryPool,
    uint32_t firstQuery,
    uint32_t queryCount,
    VkBuffer dstBuffer,
    VkDeviceSize dstOffset,
    VkDeviceSize stride,
    VkQueryResultFlags flags) const override;
bool PreCallValidateCmdPushConstants(VkCommandBuffer commandBuffer,
    VkPipelineLayout layout,
    VkShaderStageFlags stageFlags,
    uint32_t offset,
    uint32_t size,
    const void* pValues) const override;
bool PreCallValidateCmdBeginRenderPass(VkCommandBuffer commandBuffer,
    const VkRenderPassBeginInfo* pRenderPassBegin,
    VkSubpassContents contents) const override;
bool PreCallValidateCmdNextSubpass(VkCommandBuffer commandBuffer,
    VkSubpassContents contents) const override;
bool PreCallValidateCmdEndRenderPass(VkCommandBuffer commandBuffer) const override;
bool PreCallValidateCmdExecuteCommands(VkCommandBuffer commandBuffer,
    uint32_t commandBufferCount,
    const VkCommandBuffer* pCommandBuffers) const override;
bool ValidateBindBufferMemoryInfo(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const VkBuffer buffer,
    const VkDeviceMemory memory,
    const VkDeviceSize memoryOffset) const;
bool ValidateBindImageMemoryInfo(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const VkImage image,
    const VkDeviceMemory memory,
    const VkDeviceSize memoryOffset) const;
bool PreCallValidateBindBufferMemory2(VkDevice device,
    uint32_t bindInfoCount,
    const VkBindBufferMemoryInfo* pBindInfos) const override;
bool PreCallValidateBindImageMemory2(VkDevice device,
    uint32_t bindInfoCount,
    const VkBindImageMemoryInfo* pBindInfos) const override;
bool ValidatePhysicalDevice16BitStorageFeatures(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const VkBool32 storageBuffer16BitAccess,
    const VkBool32 uniformAndStorageBuffer16BitAccess,
    const VkBool32 storagePushConstant16,
    const VkBool32 storageInputOutput16) const;
bool ValidateMemoryDedicatedAllocateInfo(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const VkImage image,
    const VkBuffer buffer) const;
bool ValidateMemoryAllocateFlagsInfo(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const VkMemoryAllocateFlags flags,
    const uint32_t deviceMask) const;
bool ValidateDeviceGroupRenderPassBeginInfo(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const uint32_t deviceMask,
    const uint32_t deviceRenderAreaCount,
    const VkRect2D* pDeviceRenderAreas) const;
bool ValidateDeviceGroupCommandBufferBeginInfo(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const uint32_t deviceMask) const;
bool ValidateDeviceGroupSubmitInfo(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const uint32_t waitSemaphoreCount,
    const uint32_t* pWaitSemaphoreDeviceIndices,
    const uint32_t commandBufferCount,
    const uint32_t* pCommandBufferDeviceMasks,
    const uint32_t signalSemaphoreCount,
    const uint32_t* pSignalSemaphoreDeviceIndices) const;
bool ValidateDeviceGroupBindSparseInfo(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const uint32_t resourceDeviceIndex,
    const uint32_t memoryDeviceIndex) const;
bool PreCallValidateGetDeviceGroupPeerMemoryFeatures(VkDevice device,
    uint32_t heapIndex,
    uint32_t localDeviceIndex,
    uint32_t remoteDeviceIndex,
    VkPeerMemoryFeatureFlags* pPeerMemoryFeatures) const override;
bool PreCallValidateCmdSetDeviceMask(VkCommandBuffer commandBuffer,
    uint32_t deviceMask) const override;
bool PreCallValidateCmdDispatchBase(VkCommandBuffer commandBuffer,
    uint32_t baseGroupX,
    uint32_t baseGroupY,
    uint32_t baseGroupZ,
    uint32_t groupCountX,
    uint32_t groupCountY,
    uint32_t groupCountZ) const override;
bool ValidateBindBufferMemoryDeviceGroupInfo(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const uint32_t deviceIndexCount,
    const uint32_t* pDeviceIndices) const;
bool ValidateBindImageMemoryDeviceGroupInfo(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const uint32_t deviceIndexCount,
    const uint32_t* pDeviceIndices,
    const uint32_t splitInstanceBindRegionCount,
    const VkRect2D* pSplitInstanceBindRegions) const;
bool ValidateDeviceGroupDeviceCreateInfo(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const uint32_t physicalDeviceCount,
    const VkPhysicalDevice* pPhysicalDevices) const;
bool PreCallValidateEnumeratePhysicalDeviceGroups(VkInstance instance,
    uint32_t* pPhysicalDeviceGroupCount,
    VkPhysicalDeviceGroupProperties* pPhysicalDeviceGroupProperties) const override;
bool ValidateBufferMemoryRequirementsInfo2(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const VkBuffer buffer) const;
bool ValidateImageMemoryRequirementsInfo2(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const VkImage image) const;
bool ValidateImageSparseMemoryRequirementsInfo2(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const VkImage image) const;
bool PreCallValidateGetImageMemoryRequirements2(VkDevice device,
    const VkImageMemoryRequirementsInfo2* pInfo,
    VkMemoryRequirements2* pMemoryRequirements) const override;
bool PreCallValidateGetBufferMemoryRequirements2(VkDevice device,
    const VkBufferMemoryRequirementsInfo2* pInfo,
    VkMemoryRequirements2* pMemoryRequirements) const override;
bool PreCallValidateGetImageSparseMemoryRequirements2(VkDevice device,
    const VkImageSparseMemoryRequirementsInfo2* pInfo,
    uint32_t* pSparseMemoryRequirementCount,
    VkSparseImageMemoryRequirements2* pSparseMemoryRequirements) const override;
bool ValidatePhysicalDeviceFeatures2(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const VkPhysicalDeviceFeatures features) const;
bool ValidatePhysicalDeviceImageFormatInfo2(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const VkFormat format,
    const VkImageType type,
    const VkImageTiling tiling,
    const VkImageUsageFlags usage,
    const VkImageCreateFlags flags) const;
bool ValidatePhysicalDeviceSparseImageFormatInfo2(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const VkFormat format,
    const VkImageType type,
    const VkSampleCountFlagBits samples,
    const VkImageUsageFlags usage,
    const VkImageTiling tiling) const;
bool PreCallValidateGetPhysicalDeviceFeatures2(VkPhysicalDevice physicalDevice,
    VkPhysicalDeviceFeatures2* pFeatures) const override;
bool PreCallValidateGetPhysicalDeviceProperties2(VkPhysicalDevice physicalDevice,
    VkPhysicalDeviceProperties2* pProperties) const override;
bool PreCallValidateGetPhysicalDeviceFormatProperties2(VkPhysicalDevice physicalDevice,
    VkFormat format,
    VkFormatProperties2* pFormatProperties) const override;
bool PreCallValidateGetPhysicalDeviceImageFormatProperties2(VkPhysicalDevice physicalDevice,
    const VkPhysicalDeviceImageFormatInfo2* pImageFormatInfo,
    VkImageFormatProperties2* pImageFormatProperties) const override;
bool PreCallValidateGetPhysicalDeviceQueueFamilyProperties2(VkPhysicalDevice physicalDevice,
    uint32_t* pQueueFamilyPropertyCount,
    VkQueueFamilyProperties2* pQueueFamilyProperties) const override;
bool PreCallValidateGetPhysicalDeviceMemoryProperties2(VkPhysicalDevice physicalDevice,
    VkPhysicalDeviceMemoryProperties2* pMemoryProperties) const override;
bool PreCallValidateGetPhysicalDeviceSparseImageFormatProperties2(VkPhysicalDevice physicalDevice,
    const VkPhysicalDeviceSparseImageFormatInfo2* pFormatInfo,
    uint32_t* pPropertyCount,
    VkSparseImageFormatProperties2* pProperties) const override;
bool PreCallValidateTrimCommandPool(VkDevice device,
    VkCommandPool commandPool,
    VkCommandPoolTrimFlags flags) const override;
bool ValidateInputAttachmentAspectReference(const LogObjectList &_parentObjects,
    const uint32_t subpass,
    const uint32_t inputAttachmentIndex,
    const VkImageAspectFlags aspectMask) const;
bool ValidateRenderPassInputAttachmentAspectCreateInfo(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const uint32_t aspectReferenceCount,
    const VkInputAttachmentAspectReference* pAspectReferences) const;
bool ValidateImageViewUsageCreateInfo(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const VkImageUsageFlags usage) const;
bool ValidatePipelineTessellationDomainOriginStateCreateInfo(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const VkTessellationDomainOrigin domainOrigin) const;
bool ValidateRenderPassMultiviewCreateInfo(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const uint32_t subpassCount,
    const uint32_t* pViewMasks,
    const uint32_t dependencyCount,
    const int32_t* pViewOffsets,
    const uint32_t correlationMaskCount,
    const uint32_t* pCorrelationMasks) const;
bool ValidatePhysicalDeviceMultiviewFeatures(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const VkBool32 multiview,
    const VkBool32 multiviewGeometryShader,
    const VkBool32 multiviewTessellationShader) const;
bool ValidatePhysicalDeviceVariablePointersFeatures(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const VkBool32 variablePointersStorageBuffer,
    const VkBool32 variablePointers) const;
bool ValidatePhysicalDeviceVariablePointerFeatures(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const VkBool32 variablePointersStorageBuffer,
    const VkBool32 variablePointers) const;
bool ValidatePhysicalDeviceProtectedMemoryFeatures(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const VkBool32 protectedMemory) const;
bool ValidateDeviceQueueInfo2(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const VkDeviceQueueCreateFlags flags,
    const uint32_t queueFamilyIndex,
    const uint32_t queueIndex) const;
bool ValidateProtectedSubmitInfo(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const VkBool32 protectedSubmit) const;
bool PreCallValidateGetDeviceQueue2(VkDevice device,
    const VkDeviceQueueInfo2* pQueueInfo,
    VkQueue* pQueue) const override;
bool ValidateSamplerYcbcrConversionCreateInfo(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const VkFormat format,
    const VkSamplerYcbcrModelConversion ycbcrModel,
    const VkSamplerYcbcrRange ycbcrRange,
    const VkComponentMapping components,
    const VkChromaLocation xChromaOffset,
    const VkChromaLocation yChromaOffset,
    const VkFilter chromaFilter,
    VkBool32 forceExplicitReconstruction) const;
bool ValidateSamplerYcbcrConversionInfo(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const VkSamplerYcbcrConversion conversion) const;
bool ValidateBindImagePlaneMemoryInfo(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const VkImageAspectFlagBits planeAspect) const;
bool ValidateImagePlaneMemoryRequirementsInfo(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const VkImageAspectFlagBits planeAspect) const;
bool ValidatePhysicalDeviceSamplerYcbcrConversionFeatures(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const VkBool32 samplerYcbcrConversion) const;
bool PreCallValidateCreateSamplerYcbcrConversion(VkDevice device,
    const VkSamplerYcbcrConversionCreateInfo* pCreateInfo,
    const VkAllocationCallbacks* pAllocator,
    VkSamplerYcbcrConversion* pYcbcrConversion) const override;
bool PreCallValidateDestroySamplerYcbcrConversion(VkDevice device,
    VkSamplerYcbcrConversion ycbcrConversion,
    const VkAllocationCallbacks* pAllocator) const override;
bool ValidateDescriptorUpdateTemplateEntry(const LogObjectList &_parentObjects,
    const uint32_t dstBinding,
    const uint32_t dstArrayElement,
    const uint32_t descriptorCount,
    const VkDescriptorType descriptorType,
    const size_t offset,
    const size_t stride) const;
bool ValidateDescriptorUpdateTemplateCreateInfo(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const VkDescriptorUpdateTemplateCreateFlags flags,
    const uint32_t descriptorUpdateEntryCount,
    const VkDescriptorUpdateTemplateEntry* pDescriptorUpdateEntries,
    const VkDescriptorUpdateTemplateType templateType,
    const VkDescriptorSetLayout descriptorSetLayout,
    const VkPipelineBindPoint pipelineBindPoint,
    const VkPipelineLayout pipelineLayout,
    const uint32_t set) const;
bool PreCallValidateCreateDescriptorUpdateTemplate(VkDevice device,
    const VkDescriptorUpdateTemplateCreateInfo* pCreateInfo,
    const VkAllocationCallbacks* pAllocator,
    VkDescriptorUpdateTemplate* pDescriptorUpdateTemplate) const override;
bool PreCallValidateDestroyDescriptorUpdateTemplate(VkDevice device,
    VkDescriptorUpdateTemplate descriptorUpdateTemplate,
    const VkAllocationCallbacks* pAllocator) const override;
bool PreCallValidateUpdateDescriptorSetWithTemplate(VkDevice device,
    VkDescriptorSet descriptorSet,
    VkDescriptorUpdateTemplate descriptorUpdateTemplate,
    const void* pData) const override;
bool ValidatePhysicalDeviceExternalImageFormatInfo(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const VkExternalMemoryHandleTypeFlagBits handleType) const;
bool ValidatePhysicalDeviceExternalBufferInfo(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const VkBufferCreateFlags flags,
    const VkBufferUsageFlags usage,
    const VkExternalMemoryHandleTypeFlagBits handleType) const;
bool PreCallValidateGetPhysicalDeviceExternalBufferProperties(VkPhysicalDevice physicalDevice,
    const VkPhysicalDeviceExternalBufferInfo* pExternalBufferInfo,
    VkExternalBufferProperties* pExternalBufferProperties) const override;
bool ValidateExternalMemoryImageCreateInfo(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const VkExternalMemoryHandleTypeFlags handleTypes) const;
bool ValidateExternalMemoryBufferCreateInfo(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const VkExternalMemoryHandleTypeFlags handleTypes) const;
bool ValidateExportMemoryAllocateInfo(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const VkExternalMemoryHandleTypeFlags handleTypes) const;
bool ValidatePhysicalDeviceExternalFenceInfo(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const VkExternalFenceHandleTypeFlagBits handleType) const;
bool PreCallValidateGetPhysicalDeviceExternalFenceProperties(VkPhysicalDevice physicalDevice,
    const VkPhysicalDeviceExternalFenceInfo* pExternalFenceInfo,
    VkExternalFenceProperties* pExternalFenceProperties) const override;
bool ValidateExportFenceCreateInfo(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const VkExternalFenceHandleTypeFlags handleTypes) const;
bool ValidateExportSemaphoreCreateInfo(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const VkExternalSemaphoreHandleTypeFlags handleTypes) const;
bool ValidatePhysicalDeviceExternalSemaphoreInfo(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const VkExternalSemaphoreHandleTypeFlagBits handleType) const;
bool PreCallValidateGetPhysicalDeviceExternalSemaphoreProperties(VkPhysicalDevice physicalDevice,
    const VkPhysicalDeviceExternalSemaphoreInfo* pExternalSemaphoreInfo,
    VkExternalSemaphoreProperties* pExternalSemaphoreProperties) const override;
bool PreCallValidateGetDescriptorSetLayoutSupport(VkDevice device,
    const VkDescriptorSetLayoutCreateInfo* pCreateInfo,
    VkDescriptorSetLayoutSupport* pSupport) const override;
bool ValidatePhysicalDeviceShaderDrawParametersFeatures(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const VkBool32 shaderDrawParameters) const;
bool ValidatePhysicalDeviceShaderDrawParameterFeatures(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const VkBool32 shaderDrawParameters) const;
bool ValidatePhysicalDeviceVulkan11Features(const LogObjectList &_parentObjects,
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
    const VkBool32 shaderDrawParameters) const;
bool ValidatePhysicalDeviceVulkan12Features(const LogObjectList &_parentObjects,
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
    const VkBool32 subgroupBroadcastDynamicId) const;
bool ValidateConformanceVersion(const LogObjectList &_parentObjects,
    const uint8_t major,
    const uint8_t minor,
    const uint8_t subminor,
    const uint8_t patch) const;
bool ValidateImageFormatListCreateInfo(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const uint32_t viewFormatCount,
    const VkFormat* pViewFormats) const;
bool PreCallValidateCmdDrawIndirectCount(VkCommandBuffer commandBuffer,
    VkBuffer buffer,
    VkDeviceSize offset,
    VkBuffer countBuffer,
    VkDeviceSize countBufferOffset,
    uint32_t maxDrawCount,
    uint32_t stride) const override;
bool PreCallValidateCmdDrawIndexedIndirectCount(VkCommandBuffer commandBuffer,
    VkBuffer buffer,
    VkDeviceSize offset,
    VkBuffer countBuffer,
    VkDeviceSize countBufferOffset,
    uint32_t maxDrawCount,
    uint32_t stride) const override;
bool ValidateAttachmentDescription2(const LogObjectList &_parentObjects,
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
    const VkImageLayout finalLayout) const;
bool ValidateAttachmentReference2(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const uint32_t attachment,
    const VkImageLayout layout,
    const VkImageAspectFlags aspectMask) const;
bool ValidateSubpassDescription2(const LogObjectList &_parentObjects,
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
    const uint32_t* pPreserveAttachments) const;
bool ValidateSubpassDependency2(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const uint32_t srcSubpass,
    const uint32_t dstSubpass,
    const VkPipelineStageFlags srcStageMask,
    const VkPipelineStageFlags dstStageMask,
    const VkAccessFlags srcAccessMask,
    const VkAccessFlags dstAccessMask,
    const VkDependencyFlags dependencyFlags,
    const int32_t viewOffset) const;
bool ValidateRenderPassCreateInfo2(const LogObjectList &_parentObjects,
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
    const uint32_t* pCorrelatedViewMasks) const;
bool ValidateSubpassBeginInfo(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const VkSubpassContents contents) const;
bool ValidateSubpassEndInfo(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext) const;
bool PreCallValidateCreateRenderPass2(VkDevice device,
    const VkRenderPassCreateInfo2* pCreateInfo,
    const VkAllocationCallbacks* pAllocator,
    VkRenderPass* pRenderPass) const override;
bool PreCallValidateCmdBeginRenderPass2(VkCommandBuffer commandBuffer,
    const VkRenderPassBeginInfo* pRenderPassBegin,
    const VkSubpassBeginInfo* pSubpassBeginInfo) const override;
bool PreCallValidateCmdNextSubpass2(VkCommandBuffer commandBuffer,
    const VkSubpassBeginInfo* pSubpassBeginInfo,
    const VkSubpassEndInfo* pSubpassEndInfo) const override;
bool PreCallValidateCmdEndRenderPass2(VkCommandBuffer commandBuffer,
    const VkSubpassEndInfo* pSubpassEndInfo) const override;
bool ValidatePhysicalDevice8BitStorageFeatures(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const VkBool32 storageBuffer8BitAccess,
    const VkBool32 uniformAndStorageBuffer8BitAccess,
    const VkBool32 storagePushConstant8) const;
bool ValidatePhysicalDeviceShaderAtomicInt64Features(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const VkBool32 shaderBufferInt64Atomics,
    const VkBool32 shaderSharedInt64Atomics) const;
bool ValidatePhysicalDeviceShaderFloat16Int8Features(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const VkBool32 shaderFloat16,
    const VkBool32 shaderInt8) const;
bool ValidateDescriptorSetLayoutBindingFlagsCreateInfo(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const uint32_t bindingCount,
    const VkDescriptorBindingFlags* pBindingFlags) const;
bool ValidatePhysicalDeviceDescriptorIndexingFeatures(const LogObjectList &_parentObjects,
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
    const VkBool32 runtimeDescriptorArray) const;
bool ValidateDescriptorSetVariableDescriptorCountAllocateInfo(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const uint32_t descriptorSetCount,
    const uint32_t* pDescriptorCounts) const;
bool ValidateSubpassDescriptionDepthStencilResolve(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const VkResolveModeFlagBits depthResolveMode,
    const VkResolveModeFlagBits stencilResolveMode,
    const VkAttachmentReference2* pDepthStencilResolveAttachment) const;
bool ValidatePhysicalDeviceScalarBlockLayoutFeatures(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const VkBool32 scalarBlockLayout) const;
bool ValidateImageStencilUsageCreateInfo(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const VkImageUsageFlags stencilUsage) const;
bool ValidateSamplerReductionModeCreateInfo(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const VkSamplerReductionMode reductionMode) const;
bool ValidatePhysicalDeviceVulkanMemoryModelFeatures(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const VkBool32 vulkanMemoryModel,
    const VkBool32 vulkanMemoryModelDeviceScope,
    const VkBool32 vulkanMemoryModelAvailabilityVisibilityChains) const;
bool ValidatePhysicalDeviceImagelessFramebufferFeatures(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const VkBool32 imagelessFramebuffer) const;
bool ValidateFramebufferAttachmentImageInfo(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const VkImageCreateFlags flags,
    const VkImageUsageFlags usage,
    const uint32_t width,
    const uint32_t height,
    const uint32_t layerCount,
    const uint32_t viewFormatCount,
    const VkFormat* pViewFormats) const;
bool ValidateFramebufferAttachmentsCreateInfo(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const uint32_t attachmentImageInfoCount,
    const VkFramebufferAttachmentImageInfo* pAttachmentImageInfos) const;
bool ValidateRenderPassAttachmentBeginInfo(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const uint32_t attachmentCount,
    const VkImageView* pAttachments) const;
bool ValidatePhysicalDeviceUniformBufferStandardLayoutFeatures(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const VkBool32 uniformBufferStandardLayout) const;
bool ValidatePhysicalDeviceShaderSubgroupExtendedTypesFeatures(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const VkBool32 shaderSubgroupExtendedTypes) const;
bool ValidatePhysicalDeviceSeparateDepthStencilLayoutsFeatures(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const VkBool32 separateDepthStencilLayouts) const;
bool ValidateAttachmentReferenceStencilLayout(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const VkImageLayout stencilLayout) const;
bool ValidateAttachmentDescriptionStencilLayout(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const VkImageLayout stencilInitialLayout,
    const VkImageLayout stencilFinalLayout) const;
bool ValidatePhysicalDeviceHostQueryResetFeatures(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const VkBool32 hostQueryReset) const;
bool PreCallValidateResetQueryPool(VkDevice device,
    VkQueryPool queryPool,
    uint32_t firstQuery,
    uint32_t queryCount) const override;
bool ValidatePhysicalDeviceTimelineSemaphoreFeatures(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const VkBool32 timelineSemaphore) const;
bool ValidateSemaphoreTypeCreateInfo(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const VkSemaphoreType semaphoreType,
    const uint64_t initialValue) const;
bool ValidateTimelineSemaphoreSubmitInfo(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const uint32_t waitSemaphoreValueCount,
    const uint64_t* pWaitSemaphoreValues,
    const uint32_t signalSemaphoreValueCount,
    const uint64_t* pSignalSemaphoreValues) const;
bool ValidateSemaphoreWaitInfo(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const VkSemaphoreWaitFlags flags,
    const uint32_t semaphoreCount,
    const VkSemaphore* pSemaphores,
    const uint64_t* pValues) const;
bool ValidateSemaphoreSignalInfo(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const VkSemaphore semaphore,
    const uint64_t value) const;
bool PreCallValidateGetSemaphoreCounterValue(VkDevice device,
    VkSemaphore semaphore,
    uint64_t* pValue) const override;
bool PreCallValidateWaitSemaphores(VkDevice device,
    const VkSemaphoreWaitInfo* pWaitInfo,
    uint64_t timeout) const override;
bool PreCallValidateSignalSemaphore(VkDevice device,
    const VkSemaphoreSignalInfo* pSignalInfo) const override;
bool ValidatePhysicalDeviceBufferDeviceAddressFeatures(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const VkBool32 bufferDeviceAddress,
    const VkBool32 bufferDeviceAddressCaptureReplay,
    const VkBool32 bufferDeviceAddressMultiDevice) const;
bool ValidateBufferDeviceAddressInfo(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const VkBuffer buffer) const;
bool ValidateBufferOpaqueCaptureAddressCreateInfo(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const uint64_t opaqueCaptureAddress) const;
bool ValidateMemoryOpaqueCaptureAddressAllocateInfo(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const uint64_t opaqueCaptureAddress) const;
bool ValidateDeviceMemoryOpaqueCaptureAddressInfo(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const VkDeviceMemory memory) const;
bool PreCallValidateGetBufferDeviceAddress(VkDevice device,
    const VkBufferDeviceAddressInfo* pInfo) const override;
bool PreCallValidateGetBufferOpaqueCaptureAddress(VkDevice device,
    const VkBufferDeviceAddressInfo* pInfo) const override;
bool PreCallValidateGetDeviceMemoryOpaqueCaptureAddress(VkDevice device,
    const VkDeviceMemoryOpaqueCaptureAddressInfo* pInfo) const override;
bool ValidatePhysicalDeviceVulkan13Features(const LogObjectList &_parentObjects,
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
    const VkBool32 maintenance4) const;
bool ValidatePipelineCreationFeedbackCreateInfo(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const VkPipelineCreationFeedback* pPipelineCreationFeedback,
    const uint32_t pipelineStageCreationFeedbackCount,
    const VkPipelineCreationFeedback* pPipelineStageCreationFeedbacks) const;
bool ValidatePhysicalDeviceShaderTerminateInvocationFeatures(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const VkBool32 shaderTerminateInvocation) const;
bool PreCallValidateGetPhysicalDeviceToolProperties(VkPhysicalDevice physicalDevice,
    uint32_t* pToolCount,
    VkPhysicalDeviceToolProperties* pToolProperties) const override;
bool ValidatePhysicalDeviceShaderDemoteToHelperInvocationFeatures(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const VkBool32 shaderDemoteToHelperInvocation) const;
bool ValidatePhysicalDevicePrivateDataFeatures(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const VkBool32 privateData) const;
bool ValidateDevicePrivateDataCreateInfo(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const uint32_t privateDataSlotRequestCount) const;
bool ValidatePrivateDataSlotCreateInfo(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const VkPrivateDataSlotCreateFlags flags) const;
bool PreCallValidateCreatePrivateDataSlot(VkDevice device,
    const VkPrivateDataSlotCreateInfo* pCreateInfo,
    const VkAllocationCallbacks* pAllocator,
    VkPrivateDataSlot* pPrivateDataSlot) const override;
bool PreCallValidateDestroyPrivateDataSlot(VkDevice device,
    VkPrivateDataSlot privateDataSlot,
    const VkAllocationCallbacks* pAllocator) const override;
bool PreCallValidateSetPrivateData(VkDevice device,
    VkObjectType objectType,
    uint64_t objectHandle,
    VkPrivateDataSlot privateDataSlot,
    uint64_t data) const override;
bool PreCallValidateGetPrivateData(VkDevice device,
    VkObjectType objectType,
    uint64_t objectHandle,
    VkPrivateDataSlot privateDataSlot,
    uint64_t* pData) const override;
bool ValidatePhysicalDevicePipelineCreationCacheControlFeatures(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const VkBool32 pipelineCreationCacheControl) const;
bool ValidateMemoryBarrier2(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const VkPipelineStageFlags2 srcStageMask,
    const VkAccessFlags2 srcAccessMask,
    const VkPipelineStageFlags2 dstStageMask,
    const VkAccessFlags2 dstAccessMask) const;
bool ValidateBufferMemoryBarrier2(const LogObjectList &_parentObjects,
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
    const VkDeviceSize size) const;
bool ValidateImageMemoryBarrier2(const LogObjectList &_parentObjects,
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
    const VkImageSubresourceRange subresourceRange) const;
bool ValidateDependencyInfo(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const VkDependencyFlags dependencyFlags,
    const uint32_t memoryBarrierCount,
    const VkMemoryBarrier2* pMemoryBarriers,
    const uint32_t bufferMemoryBarrierCount,
    const VkBufferMemoryBarrier2* pBufferMemoryBarriers,
    const uint32_t imageMemoryBarrierCount,
    const VkImageMemoryBarrier2* pImageMemoryBarriers) const;
bool ValidateSemaphoreSubmitInfo(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const VkSemaphore semaphore,
    const uint64_t value,
    const VkPipelineStageFlags2 stageMask,
    const uint32_t deviceIndex) const;
bool ValidateCommandBufferSubmitInfo(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const VkCommandBuffer commandBuffer,
    const uint32_t deviceMask) const;
bool ValidateSubmitInfo2(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const VkSubmitFlags flags,
    const uint32_t waitSemaphoreInfoCount,
    const VkSemaphoreSubmitInfo* pWaitSemaphoreInfos,
    const uint32_t commandBufferInfoCount,
    const VkCommandBufferSubmitInfo* pCommandBufferInfos,
    const uint32_t signalSemaphoreInfoCount,
    const VkSemaphoreSubmitInfo* pSignalSemaphoreInfos) const;
bool ValidatePhysicalDeviceSynchronization2Features(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const VkBool32 synchronization2) const;
bool PreCallValidateCmdSetEvent2(VkCommandBuffer commandBuffer,
    VkEvent event,
    const VkDependencyInfo* pDependencyInfo) const override;
bool PreCallValidateCmdResetEvent2(VkCommandBuffer commandBuffer,
    VkEvent event,
    VkPipelineStageFlags2 stageMask) const override;
bool PreCallValidateCmdWaitEvents2(VkCommandBuffer commandBuffer,
    uint32_t eventCount,
    const VkEvent* pEvents,
    const VkDependencyInfo* pDependencyInfos) const override;
bool PreCallValidateCmdPipelineBarrier2(VkCommandBuffer commandBuffer,
    const VkDependencyInfo* pDependencyInfo) const override;
bool PreCallValidateCmdWriteTimestamp2(VkCommandBuffer commandBuffer,
    VkPipelineStageFlags2 stage,
    VkQueryPool queryPool,
    uint32_t query) const override;
bool PreCallValidateQueueSubmit2(VkQueue queue,
    uint32_t submitCount,
    const VkSubmitInfo2* pSubmits,
    VkFence fence) const override;
bool ValidatePhysicalDeviceZeroInitializeWorkgroupMemoryFeatures(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const VkBool32 shaderZeroInitializeWorkgroupMemory) const;
bool ValidatePhysicalDeviceImageRobustnessFeatures(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const VkBool32 robustImageAccess) const;
bool ValidateBufferCopy2(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const VkDeviceSize srcOffset,
    const VkDeviceSize dstOffset,
    const VkDeviceSize size) const;
bool ValidateCopyBufferInfo2(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const VkBuffer srcBuffer,
    const VkBuffer dstBuffer,
    const uint32_t regionCount,
    const VkBufferCopy2* pRegions) const;
bool ValidateImageCopy2(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const VkImageSubresourceLayers srcSubresource,
    const VkOffset3D srcOffset,
    const VkImageSubresourceLayers dstSubresource,
    const VkOffset3D dstOffset,
    const VkExtent3D extent) const;
bool ValidateCopyImageInfo2(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const VkImage srcImage,
    const VkImageLayout srcImageLayout,
    const VkImage dstImage,
    const VkImageLayout dstImageLayout,
    const uint32_t regionCount,
    const VkImageCopy2* pRegions) const;
bool ValidateBufferImageCopy2(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const VkDeviceSize bufferOffset,
    const uint32_t bufferRowLength,
    const uint32_t bufferImageHeight,
    const VkImageSubresourceLayers imageSubresource,
    const VkOffset3D imageOffset,
    const VkExtent3D imageExtent) const;
bool ValidateCopyBufferToImageInfo2(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const VkBuffer srcBuffer,
    const VkImage dstImage,
    const VkImageLayout dstImageLayout,
    const uint32_t regionCount,
    const VkBufferImageCopy2* pRegions) const;
bool ValidateCopyImageToBufferInfo2(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const VkImage srcImage,
    const VkImageLayout srcImageLayout,
    const VkBuffer dstBuffer,
    const uint32_t regionCount,
    const VkBufferImageCopy2* pRegions) const;
bool ValidateImageBlit2(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const VkImageSubresourceLayers srcSubresource,
    const VkOffset3D srcOffsets[2],
    const VkImageSubresourceLayers dstSubresource,
    const VkOffset3D dstOffsets[2]) const;
bool ValidateBlitImageInfo2(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const VkImage srcImage,
    const VkImageLayout srcImageLayout,
    const VkImage dstImage,
    const VkImageLayout dstImageLayout,
    const uint32_t regionCount,
    const VkImageBlit2* pRegions,
    const VkFilter filter) const;
bool ValidateImageResolve2(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const VkImageSubresourceLayers srcSubresource,
    const VkOffset3D srcOffset,
    const VkImageSubresourceLayers dstSubresource,
    const VkOffset3D dstOffset,
    const VkExtent3D extent) const;
bool ValidateResolveImageInfo2(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const VkImage srcImage,
    const VkImageLayout srcImageLayout,
    const VkImage dstImage,
    const VkImageLayout dstImageLayout,
    const uint32_t regionCount,
    const VkImageResolve2* pRegions) const;
bool PreCallValidateCmdCopyBuffer2(VkCommandBuffer commandBuffer,
    const VkCopyBufferInfo2* pCopyBufferInfo) const override;
bool PreCallValidateCmdCopyImage2(VkCommandBuffer commandBuffer,
    const VkCopyImageInfo2* pCopyImageInfo) const override;
bool PreCallValidateCmdCopyBufferToImage2(VkCommandBuffer commandBuffer,
    const VkCopyBufferToImageInfo2* pCopyBufferToImageInfo) const override;
bool PreCallValidateCmdCopyImageToBuffer2(VkCommandBuffer commandBuffer,
    const VkCopyImageToBufferInfo2* pCopyImageToBufferInfo) const override;
bool PreCallValidateCmdBlitImage2(VkCommandBuffer commandBuffer,
    const VkBlitImageInfo2* pBlitImageInfo) const override;
bool PreCallValidateCmdResolveImage2(VkCommandBuffer commandBuffer,
    const VkResolveImageInfo2* pResolveImageInfo) const override;
bool ValidatePhysicalDeviceSubgroupSizeControlFeatures(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const VkBool32 subgroupSizeControl,
    const VkBool32 computeFullSubgroups) const;
bool ValidatePhysicalDeviceInlineUniformBlockFeatures(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const VkBool32 inlineUniformBlock,
    const VkBool32 descriptorBindingInlineUniformBlockUpdateAfterBind) const;
bool ValidateWriteDescriptorSetInlineUniformBlock(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const uint32_t dataSize,
    const void* pData) const;
bool ValidateDescriptorPoolInlineUniformBlockCreateInfo(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const uint32_t maxInlineUniformBlockBindings) const;
bool ValidatePhysicalDeviceTextureCompressionASTCHDRFeatures(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const VkBool32 textureCompressionASTC_HDR) const;
bool ValidateRenderingAttachmentInfo(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const VkImageView imageView,
    const VkImageLayout imageLayout,
    const VkResolveModeFlagBits resolveMode,
    const VkImageView resolveImageView,
    const VkImageLayout resolveImageLayout,
    const VkAttachmentLoadOp loadOp,
    const VkAttachmentStoreOp storeOp,
    const VkClearValue clearValue) const;
bool ValidateRenderingInfo(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const VkRenderingFlags flags,
    const VkRect2D renderArea,
    const uint32_t layerCount,
    const uint32_t viewMask,
    const uint32_t colorAttachmentCount,
    const VkRenderingAttachmentInfo* pColorAttachments,
    const VkRenderingAttachmentInfo* pDepthAttachment,
    const VkRenderingAttachmentInfo* pStencilAttachment) const;
bool ValidatePipelineRenderingCreateInfo(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const uint32_t viewMask,
    const uint32_t colorAttachmentCount,
    const VkFormat* pColorAttachmentFormats,
    const VkFormat depthAttachmentFormat,
    const VkFormat stencilAttachmentFormat) const;
bool ValidatePhysicalDeviceDynamicRenderingFeatures(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const VkBool32 dynamicRendering) const;
bool ValidateCommandBufferInheritanceRenderingInfo(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const VkRenderingFlags flags,
    const uint32_t viewMask,
    const uint32_t colorAttachmentCount,
    const VkFormat* pColorAttachmentFormats,
    const VkFormat depthAttachmentFormat,
    const VkFormat stencilAttachmentFormat,
    const VkSampleCountFlagBits rasterizationSamples) const;
bool PreCallValidateCmdBeginRendering(VkCommandBuffer commandBuffer,
    const VkRenderingInfo* pRenderingInfo) const override;
bool PreCallValidateCmdEndRendering(VkCommandBuffer commandBuffer) const override;
bool PreCallValidateCmdSetCullMode(VkCommandBuffer commandBuffer,
    VkCullModeFlags cullMode) const override;
bool PreCallValidateCmdSetFrontFace(VkCommandBuffer commandBuffer,
    VkFrontFace frontFace) const override;
bool PreCallValidateCmdSetPrimitiveTopology(VkCommandBuffer commandBuffer,
    VkPrimitiveTopology primitiveTopology) const override;
bool PreCallValidateCmdSetViewportWithCount(VkCommandBuffer commandBuffer,
    uint32_t viewportCount,
    const VkViewport* pViewports) const override;
bool PreCallValidateCmdSetScissorWithCount(VkCommandBuffer commandBuffer,
    uint32_t scissorCount,
    const VkRect2D* pScissors) const override;
bool PreCallValidateCmdBindVertexBuffers2(VkCommandBuffer commandBuffer,
    uint32_t firstBinding,
    uint32_t bindingCount,
    const VkBuffer* pBuffers,
    const VkDeviceSize* pOffsets,
    const VkDeviceSize* pSizes,
    const VkDeviceSize* pStrides) const override;
bool PreCallValidateCmdSetDepthTestEnable(VkCommandBuffer commandBuffer,
    VkBool32 depthTestEnable) const override;
bool PreCallValidateCmdSetDepthWriteEnable(VkCommandBuffer commandBuffer,
    VkBool32 depthWriteEnable) const override;
bool PreCallValidateCmdSetDepthCompareOp(VkCommandBuffer commandBuffer,
    VkCompareOp depthCompareOp) const override;
bool PreCallValidateCmdSetDepthBoundsTestEnable(VkCommandBuffer commandBuffer,
    VkBool32 depthBoundsTestEnable) const override;
bool PreCallValidateCmdSetStencilTestEnable(VkCommandBuffer commandBuffer,
    VkBool32 stencilTestEnable) const override;
bool PreCallValidateCmdSetStencilOp(VkCommandBuffer commandBuffer,
    VkStencilFaceFlags faceMask,
    VkStencilOp failOp,
    VkStencilOp passOp,
    VkStencilOp depthFailOp,
    VkCompareOp compareOp) const override;
bool ValidatePhysicalDeviceShaderIntegerDotProductFeatures(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const VkBool32 shaderIntegerDotProduct) const;
bool PreCallValidateCmdSetRasterizerDiscardEnable(VkCommandBuffer commandBuffer,
    VkBool32 rasterizerDiscardEnable) const override;
bool PreCallValidateCmdSetDepthBiasEnable(VkCommandBuffer commandBuffer,
    VkBool32 depthBiasEnable) const override;
bool PreCallValidateCmdSetPrimitiveRestartEnable(VkCommandBuffer commandBuffer,
    VkBool32 primitiveRestartEnable) const override;
bool ValidatePhysicalDeviceMaintenance4Features(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const VkBool32 maintenance4) const;
bool ValidateDeviceBufferMemoryRequirements(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const VkBufferCreateInfo* pCreateInfo) const;
bool ValidateDeviceImageMemoryRequirements(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const VkImageCreateInfo* pCreateInfo,
    const VkImageAspectFlagBits planeAspect) const;
bool PreCallValidateGetDeviceBufferMemoryRequirements(VkDevice device,
    const VkDeviceBufferMemoryRequirements* pInfo,
    VkMemoryRequirements2* pMemoryRequirements) const override;
bool PreCallValidateGetDeviceImageMemoryRequirements(VkDevice device,
    const VkDeviceImageMemoryRequirements* pInfo,
    VkMemoryRequirements2* pMemoryRequirements) const override;
bool PreCallValidateGetDeviceImageSparseMemoryRequirements(VkDevice device,
    const VkDeviceImageMemoryRequirements* pInfo,
    uint32_t* pSparseMemoryRequirementCount,
    VkSparseImageMemoryRequirements2* pSparseMemoryRequirements) const override;
bool PreCallValidateDestroySurfaceKHR(VkInstance instance,
    VkSurfaceKHR surface,
    const VkAllocationCallbacks* pAllocator) const override;
bool PreCallValidateGetPhysicalDeviceSurfaceSupportKHR(VkPhysicalDevice physicalDevice,
    uint32_t queueFamilyIndex,
    VkSurfaceKHR surface,
    VkBool32* pSupported) const override;
bool PreCallValidateGetPhysicalDeviceSurfaceCapabilitiesKHR(VkPhysicalDevice physicalDevice,
    VkSurfaceKHR surface,
    VkSurfaceCapabilitiesKHR* pSurfaceCapabilities) const override;
bool PreCallValidateGetPhysicalDeviceSurfaceFormatsKHR(VkPhysicalDevice physicalDevice,
    VkSurfaceKHR surface,
    uint32_t* pSurfaceFormatCount,
    VkSurfaceFormatKHR* pSurfaceFormats) const override;
bool PreCallValidateGetPhysicalDeviceSurfacePresentModesKHR(VkPhysicalDevice physicalDevice,
    VkSurfaceKHR surface,
    uint32_t* pPresentModeCount,
    VkPresentModeKHR* pPresentModes) const override;
bool ValidateSwapchainCreateInfoKHR(const LogObjectList &_parentObjects,
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
    const VkSwapchainKHR oldSwapchain) const;
bool ValidatePresentInfoKHR(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const uint32_t waitSemaphoreCount,
    const VkSemaphore* pWaitSemaphores,
    const uint32_t swapchainCount,
    const VkSwapchainKHR* pSwapchains,
    const uint32_t* pImageIndices,
    const VkResult* pResults) const;
bool PreCallValidateCreateSwapchainKHR(VkDevice device,
    const VkSwapchainCreateInfoKHR* pCreateInfo,
    const VkAllocationCallbacks* pAllocator,
    VkSwapchainKHR* pSwapchain) const override;
bool PreCallValidateDestroySwapchainKHR(VkDevice device,
    VkSwapchainKHR swapchain,
    const VkAllocationCallbacks* pAllocator) const override;
bool PreCallValidateGetSwapchainImagesKHR(VkDevice device,
    VkSwapchainKHR swapchain,
    uint32_t* pSwapchainImageCount,
    VkImage* pSwapchainImages) const override;
bool PreCallValidateAcquireNextImageKHR(VkDevice device,
    VkSwapchainKHR swapchain,
    uint64_t timeout,
    VkSemaphore semaphore,
    VkFence fence,
    uint32_t* pImageIndex) const override;
bool PreCallValidateQueuePresentKHR(VkQueue queue,
    const VkPresentInfoKHR* pPresentInfo) const override;
bool ValidateImageSwapchainCreateInfoKHR(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const VkSwapchainKHR swapchain) const;
bool ValidateBindImageMemorySwapchainInfoKHR(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const VkSwapchainKHR swapchain,
    const uint32_t imageIndex) const;
bool ValidateAcquireNextImageInfoKHR(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const VkSwapchainKHR swapchain,
    const uint64_t timeout,
    const VkSemaphore semaphore,
    const VkFence fence,
    const uint32_t deviceMask) const;
bool ValidateDeviceGroupPresentInfoKHR(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const uint32_t swapchainCount,
    const uint32_t* pDeviceMasks,
    const VkDeviceGroupPresentModeFlagBitsKHR mode) const;
bool ValidateDeviceGroupSwapchainCreateInfoKHR(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const VkDeviceGroupPresentModeFlagsKHR modes) const;
bool PreCallValidateGetDeviceGroupPresentCapabilitiesKHR(VkDevice device,
    VkDeviceGroupPresentCapabilitiesKHR* pDeviceGroupPresentCapabilities) const override;
bool PreCallValidateGetDeviceGroupSurfacePresentModesKHR(VkDevice device,
    VkSurfaceKHR surface,
    VkDeviceGroupPresentModeFlagsKHR* pModes) const override;
bool PreCallValidateGetPhysicalDevicePresentRectanglesKHR(VkPhysicalDevice physicalDevice,
    VkSurfaceKHR surface,
    uint32_t* pRectCount,
    VkRect2D* pRects) const override;
bool PreCallValidateAcquireNextImage2KHR(VkDevice device,
    const VkAcquireNextImageInfoKHR* pAcquireInfo,
    uint32_t* pImageIndex) const override;
bool ValidateDisplayModeParametersKHR(const LogObjectList &_parentObjects,
    const VkExtent2D visibleRegion,
    const uint32_t refreshRate) const;
bool ValidateDisplayModeCreateInfoKHR(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const VkDisplayModeCreateFlagsKHR flags,
    const VkDisplayModeParametersKHR parameters) const;
bool ValidateDisplaySurfaceCreateInfoKHR(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const VkDisplaySurfaceCreateFlagsKHR flags,
    const VkDisplayModeKHR displayMode,
    const uint32_t planeIndex,
    const uint32_t planeStackIndex,
    const VkSurfaceTransformFlagBitsKHR transform,
    const float globalAlpha,
    const VkDisplayPlaneAlphaFlagBitsKHR alphaMode,
    const VkExtent2D imageExtent) const;
bool PreCallValidateGetPhysicalDeviceDisplayPropertiesKHR(VkPhysicalDevice physicalDevice,
    uint32_t* pPropertyCount,
    VkDisplayPropertiesKHR* pProperties) const override;
bool PreCallValidateGetPhysicalDeviceDisplayPlanePropertiesKHR(VkPhysicalDevice physicalDevice,
    uint32_t* pPropertyCount,
    VkDisplayPlanePropertiesKHR* pProperties) const override;
bool PreCallValidateGetDisplayPlaneSupportedDisplaysKHR(VkPhysicalDevice physicalDevice,
    uint32_t planeIndex,
    uint32_t* pDisplayCount,
    VkDisplayKHR* pDisplays) const override;
bool PreCallValidateGetDisplayModePropertiesKHR(VkPhysicalDevice physicalDevice,
    VkDisplayKHR display,
    uint32_t* pPropertyCount,
    VkDisplayModePropertiesKHR* pProperties) const override;
bool PreCallValidateCreateDisplayModeKHR(VkPhysicalDevice physicalDevice,
    VkDisplayKHR display,
    const VkDisplayModeCreateInfoKHR* pCreateInfo,
    const VkAllocationCallbacks* pAllocator,
    VkDisplayModeKHR* pMode) const override;
bool PreCallValidateGetDisplayPlaneCapabilitiesKHR(VkPhysicalDevice physicalDevice,
    VkDisplayModeKHR mode,
    uint32_t planeIndex,
    VkDisplayPlaneCapabilitiesKHR* pCapabilities) const override;
bool PreCallValidateCreateDisplayPlaneSurfaceKHR(VkInstance instance,
    const VkDisplaySurfaceCreateInfoKHR* pCreateInfo,
    const VkAllocationCallbacks* pAllocator,
    VkSurfaceKHR* pSurface) const override;
bool ValidateDisplayPresentInfoKHR(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const VkRect2D srcRect,
    const VkRect2D dstRect,
    const VkBool32 persistent) const;
bool PreCallValidateCreateSharedSwapchainsKHR(VkDevice device,
    uint32_t swapchainCount,
    const VkSwapchainCreateInfoKHR* pCreateInfos,
    const VkAllocationCallbacks* pAllocator,
    VkSwapchainKHR* pSwapchains) const override;
#ifdef VK_USE_PLATFORM_XLIB_KHR
bool ValidateXlibSurfaceCreateInfoKHR(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const VkXlibSurfaceCreateFlagsKHR flags,
    const Display* dpy,
    const Window window) const;
#endif
#ifdef VK_USE_PLATFORM_XLIB_KHR
bool PreCallValidateCreateXlibSurfaceKHR(VkInstance instance,
    const VkXlibSurfaceCreateInfoKHR* pCreateInfo,
    const VkAllocationCallbacks* pAllocator,
    VkSurfaceKHR* pSurface) const override;
#endif
#ifdef VK_USE_PLATFORM_XLIB_KHR
bool PreCallValidateGetPhysicalDeviceXlibPresentationSupportKHR(VkPhysicalDevice physicalDevice,
    uint32_t queueFamilyIndex,
    Display* dpy,
    VisualID visualID) const override;
#endif
#ifdef VK_USE_PLATFORM_XCB_KHR
bool ValidateXcbSurfaceCreateInfoKHR(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const VkXcbSurfaceCreateFlagsKHR flags,
    const xcb_connection_t* connection,
    const xcb_window_t window) const;
#endif
#ifdef VK_USE_PLATFORM_XCB_KHR
bool PreCallValidateCreateXcbSurfaceKHR(VkInstance instance,
    const VkXcbSurfaceCreateInfoKHR* pCreateInfo,
    const VkAllocationCallbacks* pAllocator,
    VkSurfaceKHR* pSurface) const override;
#endif
#ifdef VK_USE_PLATFORM_XCB_KHR
bool PreCallValidateGetPhysicalDeviceXcbPresentationSupportKHR(VkPhysicalDevice physicalDevice,
    uint32_t queueFamilyIndex,
    xcb_connection_t* connection,
    xcb_visualid_t visual_id) const override;
#endif
#ifdef VK_USE_PLATFORM_WAYLAND_KHR
bool ValidateWaylandSurfaceCreateInfoKHR(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const VkWaylandSurfaceCreateFlagsKHR flags,
    const struct wl_display* display,
    const struct wl_surface* surface) const;
#endif
#ifdef VK_USE_PLATFORM_WAYLAND_KHR
bool PreCallValidateCreateWaylandSurfaceKHR(VkInstance instance,
    const VkWaylandSurfaceCreateInfoKHR* pCreateInfo,
    const VkAllocationCallbacks* pAllocator,
    VkSurfaceKHR* pSurface) const override;
#endif
#ifdef VK_USE_PLATFORM_WAYLAND_KHR
bool PreCallValidateGetPhysicalDeviceWaylandPresentationSupportKHR(VkPhysicalDevice physicalDevice,
    uint32_t queueFamilyIndex,
    struct wl_display* display) const override;
#endif
#ifdef VK_USE_PLATFORM_ANDROID_KHR
bool ValidateAndroidSurfaceCreateInfoKHR(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const VkAndroidSurfaceCreateFlagsKHR flags,
    const struct ANativeWindow* window) const;
#endif
#ifdef VK_USE_PLATFORM_ANDROID_KHR
bool PreCallValidateCreateAndroidSurfaceKHR(VkInstance instance,
    const VkAndroidSurfaceCreateInfoKHR* pCreateInfo,
    const VkAllocationCallbacks* pAllocator,
    VkSurfaceKHR* pSurface) const override;
#endif
#ifdef VK_USE_PLATFORM_WIN32_KHR
bool ValidateWin32SurfaceCreateInfoKHR(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const VkWin32SurfaceCreateFlagsKHR flags,
    const HINSTANCE hinstance,
    const HWND hwnd) const;
#endif
#ifdef VK_USE_PLATFORM_WIN32_KHR
bool PreCallValidateCreateWin32SurfaceKHR(VkInstance instance,
    const VkWin32SurfaceCreateInfoKHR* pCreateInfo,
    const VkAllocationCallbacks* pAllocator,
    VkSurfaceKHR* pSurface) const override;
#endif
#ifdef VK_USE_PLATFORM_WIN32_KHR
bool PreCallValidateGetPhysicalDeviceWin32PresentationSupportKHR(VkPhysicalDevice physicalDevice,
    uint32_t queueFamilyIndex) const override;
#endif
bool ValidateVideoProfileInfoKHR(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const VkVideoCodecOperationFlagBitsKHR videoCodecOperation,
    const VkVideoChromaSubsamplingFlagsKHR chromaSubsampling,
    const VkVideoComponentBitDepthFlagsKHR lumaBitDepth,
    const VkVideoComponentBitDepthFlagsKHR chromaBitDepth) const;
bool ValidateVideoProfileListInfoKHR(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const uint32_t profileCount,
    const VkVideoProfileInfoKHR* pProfiles) const;
bool ValidatePhysicalDeviceVideoFormatInfoKHR(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const VkImageUsageFlags imageUsage) const;
bool ValidateVideoPictureResourceInfoKHR(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const VkOffset2D codedOffset,
    const VkExtent2D codedExtent,
    const uint32_t baseArrayLayer,
    const VkImageView imageViewBinding) const;
bool ValidateVideoReferenceSlotInfoKHR(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const int32_t slotIndex,
    const VkVideoPictureResourceInfoKHR* pPictureResource) const;
bool ValidateBindVideoSessionMemoryInfoKHR(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const uint32_t memoryBindIndex,
    const VkDeviceMemory memory,
    const VkDeviceSize memoryOffset,
    const VkDeviceSize memorySize) const;
bool ValidateVideoSessionCreateInfoKHR(const LogObjectList &_parentObjects,
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
    const VkExtensionProperties* pStdHeaderVersion) const;
bool ValidateVideoSessionParametersCreateInfoKHR(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const VkVideoSessionParametersCreateFlagsKHR flags,
    const VkVideoSessionParametersKHR videoSessionParametersTemplate,
    const VkVideoSessionKHR videoSession) const;
bool ValidateVideoSessionParametersUpdateInfoKHR(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const uint32_t updateSequenceCount) const;
bool ValidateVideoBeginCodingInfoKHR(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const VkVideoBeginCodingFlagsKHR flags,
    const VkVideoSessionKHR videoSession,
    const VkVideoSessionParametersKHR videoSessionParameters,
    const uint32_t referenceSlotCount,
    const VkVideoReferenceSlotInfoKHR* pReferenceSlots) const;
bool ValidateVideoEndCodingInfoKHR(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const VkVideoEndCodingFlagsKHR flags) const;
bool ValidateVideoCodingControlInfoKHR(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const VkVideoCodingControlFlagsKHR flags) const;
bool PreCallValidateGetPhysicalDeviceVideoCapabilitiesKHR(VkPhysicalDevice physicalDevice,
    const VkVideoProfileInfoKHR* pVideoProfile,
    VkVideoCapabilitiesKHR* pCapabilities) const override;
bool PreCallValidateGetPhysicalDeviceVideoFormatPropertiesKHR(VkPhysicalDevice physicalDevice,
    const VkPhysicalDeviceVideoFormatInfoKHR* pVideoFormatInfo,
    uint32_t* pVideoFormatPropertyCount,
    VkVideoFormatPropertiesKHR* pVideoFormatProperties) const override;
bool PreCallValidateCreateVideoSessionKHR(VkDevice device,
    const VkVideoSessionCreateInfoKHR* pCreateInfo,
    const VkAllocationCallbacks* pAllocator,
    VkVideoSessionKHR* pVideoSession) const override;
bool PreCallValidateDestroyVideoSessionKHR(VkDevice device,
    VkVideoSessionKHR videoSession,
    const VkAllocationCallbacks* pAllocator) const override;
bool PreCallValidateGetVideoSessionMemoryRequirementsKHR(VkDevice device,
    VkVideoSessionKHR videoSession,
    uint32_t* pMemoryRequirementsCount,
    VkVideoSessionMemoryRequirementsKHR* pMemoryRequirements) const override;
bool PreCallValidateBindVideoSessionMemoryKHR(VkDevice device,
    VkVideoSessionKHR videoSession,
    uint32_t bindSessionMemoryInfoCount,
    const VkBindVideoSessionMemoryInfoKHR* pBindSessionMemoryInfos) const override;
bool PreCallValidateCreateVideoSessionParametersKHR(VkDevice device,
    const VkVideoSessionParametersCreateInfoKHR* pCreateInfo,
    const VkAllocationCallbacks* pAllocator,
    VkVideoSessionParametersKHR* pVideoSessionParameters) const override;
bool PreCallValidateUpdateVideoSessionParametersKHR(VkDevice device,
    VkVideoSessionParametersKHR videoSessionParameters,
    const VkVideoSessionParametersUpdateInfoKHR* pUpdateInfo) const override;
bool PreCallValidateDestroyVideoSessionParametersKHR(VkDevice device,
    VkVideoSessionParametersKHR videoSessionParameters,
    const VkAllocationCallbacks* pAllocator) const override;
bool PreCallValidateCmdBeginVideoCodingKHR(VkCommandBuffer commandBuffer,
    const VkVideoBeginCodingInfoKHR* pBeginInfo) const override;
bool PreCallValidateCmdEndVideoCodingKHR(VkCommandBuffer commandBuffer,
    const VkVideoEndCodingInfoKHR* pEndCodingInfo) const override;
bool PreCallValidateCmdControlVideoCodingKHR(VkCommandBuffer commandBuffer,
    const VkVideoCodingControlInfoKHR* pCodingControlInfo) const override;
bool ValidateVideoDecodeUsageInfoKHR(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const VkVideoDecodeUsageFlagsKHR videoUsageHints) const;
bool ValidateVideoDecodeInfoKHR(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const VkVideoDecodeFlagsKHR flags,
    const VkBuffer srcBuffer,
    const VkDeviceSize srcBufferOffset,
    const VkDeviceSize srcBufferRange,
    const VkVideoPictureResourceInfoKHR dstPictureResource,
    const VkVideoReferenceSlotInfoKHR* pSetupReferenceSlot,
    const uint32_t referenceSlotCount,
    const VkVideoReferenceSlotInfoKHR* pReferenceSlots) const;
bool PreCallValidateCmdDecodeVideoKHR(VkCommandBuffer commandBuffer,
    const VkVideoDecodeInfoKHR* pDecodeInfo) const override;
bool ValidateVideoDecodeH264ProfileInfoKHR(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const StdVideoH264ProfileIdc stdProfileIdc,
    const VkVideoDecodeH264PictureLayoutFlagBitsKHR pictureLayout) const;
bool ValidateVideoDecodeH264SessionParametersAddInfoKHR(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const uint32_t stdSPSCount,
    const StdVideoH264SequenceParameterSet* pStdSPSs,
    const uint32_t stdPPSCount,
    const StdVideoH264PictureParameterSet* pStdPPSs) const;
bool ValidateVideoDecodeH264SessionParametersCreateInfoKHR(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const uint32_t maxStdSPSCount,
    const uint32_t maxStdPPSCount,
    const VkVideoDecodeH264SessionParametersAddInfoKHR* pParametersAddInfo) const;
bool ValidateVideoDecodeH264PictureInfoKHR(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const StdVideoDecodeH264PictureInfo* pStdPictureInfo,
    const uint32_t sliceCount,
    const uint32_t* pSliceOffsets) const;
bool ValidateVideoDecodeH264DpbSlotInfoKHR(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const StdVideoDecodeH264ReferenceInfo* pStdReferenceInfo) const;
bool ValidateRenderingInfoKHR(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const VkRenderingFlags flags,
    const VkRect2D renderArea,
    const uint32_t layerCount,
    const uint32_t viewMask,
    const uint32_t colorAttachmentCount,
    const VkRenderingAttachmentInfo* pColorAttachments,
    const VkRenderingAttachmentInfo* pDepthAttachment,
    const VkRenderingAttachmentInfo* pStencilAttachment) const;
bool ValidateRenderingAttachmentInfoKHR(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const VkImageView imageView,
    const VkImageLayout imageLayout,
    const VkResolveModeFlagBits resolveMode,
    const VkImageView resolveImageView,
    const VkImageLayout resolveImageLayout,
    const VkAttachmentLoadOp loadOp,
    const VkAttachmentStoreOp storeOp,
    const VkClearValue clearValue) const;
bool ValidatePipelineRenderingCreateInfoKHR(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const uint32_t viewMask,
    const uint32_t colorAttachmentCount,
    const VkFormat* pColorAttachmentFormats,
    const VkFormat depthAttachmentFormat,
    const VkFormat stencilAttachmentFormat) const;
bool ValidatePhysicalDeviceDynamicRenderingFeaturesKHR(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const VkBool32 dynamicRendering) const;
bool ValidateCommandBufferInheritanceRenderingInfoKHR(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const VkRenderingFlags flags,
    const uint32_t viewMask,
    const uint32_t colorAttachmentCount,
    const VkFormat* pColorAttachmentFormats,
    const VkFormat depthAttachmentFormat,
    const VkFormat stencilAttachmentFormat,
    const VkSampleCountFlagBits rasterizationSamples) const;
bool PreCallValidateCmdBeginRenderingKHR(VkCommandBuffer commandBuffer,
    const VkRenderingInfo* pRenderingInfo) const override;
bool PreCallValidateCmdEndRenderingKHR(VkCommandBuffer commandBuffer) const override;
bool ValidateRenderingFragmentShadingRateAttachmentInfoKHR(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const VkImageView imageView,
    const VkImageLayout imageLayout,
    const VkExtent2D shadingRateAttachmentTexelSize) const;
bool ValidateRenderingFragmentDensityMapAttachmentInfoEXT(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const VkImageView imageView,
    const VkImageLayout imageLayout) const;
bool ValidateAttachmentSampleCountInfoAMD(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const uint32_t colorAttachmentCount,
    const VkSampleCountFlagBits* pColorAttachmentSamples,
    const VkSampleCountFlagBits depthStencilAttachmentSamples) const;
bool ValidateAttachmentSampleCountInfoNV(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const uint32_t colorAttachmentCount,
    const VkSampleCountFlagBits* pColorAttachmentSamples,
    const VkSampleCountFlagBits depthStencilAttachmentSamples) const;
bool ValidateMultiviewPerViewAttributesInfoNVX(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const VkBool32 perViewAttributes,
    const VkBool32 perViewAttributesPositionXOnly) const;
bool ValidateRenderPassMultiviewCreateInfoKHR(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const uint32_t subpassCount,
    const uint32_t* pViewMasks,
    const uint32_t dependencyCount,
    const int32_t* pViewOffsets,
    const uint32_t correlationMaskCount,
    const uint32_t* pCorrelationMasks) const;
bool ValidatePhysicalDeviceMultiviewFeaturesKHR(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const VkBool32 multiview,
    const VkBool32 multiviewGeometryShader,
    const VkBool32 multiviewTessellationShader) const;
bool ValidatePhysicalDeviceFeatures2KHR(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const VkPhysicalDeviceFeatures features) const;
bool ValidatePhysicalDeviceImageFormatInfo2KHR(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const VkFormat format,
    const VkImageType type,
    const VkImageTiling tiling,
    const VkImageUsageFlags usage,
    const VkImageCreateFlags flags) const;
bool ValidatePhysicalDeviceSparseImageFormatInfo2KHR(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const VkFormat format,
    const VkImageType type,
    const VkSampleCountFlagBits samples,
    const VkImageUsageFlags usage,
    const VkImageTiling tiling) const;
bool PreCallValidateGetPhysicalDeviceFeatures2KHR(VkPhysicalDevice physicalDevice,
    VkPhysicalDeviceFeatures2* pFeatures) const override;
bool PreCallValidateGetPhysicalDeviceProperties2KHR(VkPhysicalDevice physicalDevice,
    VkPhysicalDeviceProperties2* pProperties) const override;
bool PreCallValidateGetPhysicalDeviceFormatProperties2KHR(VkPhysicalDevice physicalDevice,
    VkFormat format,
    VkFormatProperties2* pFormatProperties) const override;
bool PreCallValidateGetPhysicalDeviceImageFormatProperties2KHR(VkPhysicalDevice physicalDevice,
    const VkPhysicalDeviceImageFormatInfo2* pImageFormatInfo,
    VkImageFormatProperties2* pImageFormatProperties) const override;
bool PreCallValidateGetPhysicalDeviceQueueFamilyProperties2KHR(VkPhysicalDevice physicalDevice,
    uint32_t* pQueueFamilyPropertyCount,
    VkQueueFamilyProperties2* pQueueFamilyProperties) const override;
bool PreCallValidateGetPhysicalDeviceMemoryProperties2KHR(VkPhysicalDevice physicalDevice,
    VkPhysicalDeviceMemoryProperties2* pMemoryProperties) const override;
bool PreCallValidateGetPhysicalDeviceSparseImageFormatProperties2KHR(VkPhysicalDevice physicalDevice,
    const VkPhysicalDeviceSparseImageFormatInfo2* pFormatInfo,
    uint32_t* pPropertyCount,
    VkSparseImageFormatProperties2* pProperties) const override;
bool ValidateMemoryAllocateFlagsInfoKHR(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const VkMemoryAllocateFlags flags,
    const uint32_t deviceMask) const;
bool ValidateDeviceGroupRenderPassBeginInfoKHR(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const uint32_t deviceMask,
    const uint32_t deviceRenderAreaCount,
    const VkRect2D* pDeviceRenderAreas) const;
bool ValidateDeviceGroupCommandBufferBeginInfoKHR(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const uint32_t deviceMask) const;
bool ValidateDeviceGroupSubmitInfoKHR(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const uint32_t waitSemaphoreCount,
    const uint32_t* pWaitSemaphoreDeviceIndices,
    const uint32_t commandBufferCount,
    const uint32_t* pCommandBufferDeviceMasks,
    const uint32_t signalSemaphoreCount,
    const uint32_t* pSignalSemaphoreDeviceIndices) const;
bool ValidateDeviceGroupBindSparseInfoKHR(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const uint32_t resourceDeviceIndex,
    const uint32_t memoryDeviceIndex) const;
bool PreCallValidateGetDeviceGroupPeerMemoryFeaturesKHR(VkDevice device,
    uint32_t heapIndex,
    uint32_t localDeviceIndex,
    uint32_t remoteDeviceIndex,
    VkPeerMemoryFeatureFlags* pPeerMemoryFeatures) const override;
bool PreCallValidateCmdSetDeviceMaskKHR(VkCommandBuffer commandBuffer,
    uint32_t deviceMask) const override;
bool PreCallValidateCmdDispatchBaseKHR(VkCommandBuffer commandBuffer,
    uint32_t baseGroupX,
    uint32_t baseGroupY,
    uint32_t baseGroupZ,
    uint32_t groupCountX,
    uint32_t groupCountY,
    uint32_t groupCountZ) const override;
bool ValidateBindBufferMemoryDeviceGroupInfoKHR(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const uint32_t deviceIndexCount,
    const uint32_t* pDeviceIndices) const;
bool ValidateBindImageMemoryDeviceGroupInfoKHR(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const uint32_t deviceIndexCount,
    const uint32_t* pDeviceIndices,
    const uint32_t splitInstanceBindRegionCount,
    const VkRect2D* pSplitInstanceBindRegions) const;
bool PreCallValidateTrimCommandPoolKHR(VkDevice device,
    VkCommandPool commandPool,
    VkCommandPoolTrimFlags flags) const override;
bool ValidateDeviceGroupDeviceCreateInfoKHR(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const uint32_t physicalDeviceCount,
    const VkPhysicalDevice* pPhysicalDevices) const;
bool PreCallValidateEnumeratePhysicalDeviceGroupsKHR(VkInstance instance,
    uint32_t* pPhysicalDeviceGroupCount,
    VkPhysicalDeviceGroupProperties* pPhysicalDeviceGroupProperties) const override;
bool ValidatePhysicalDeviceExternalImageFormatInfoKHR(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const VkExternalMemoryHandleTypeFlagBits handleType) const;
bool ValidatePhysicalDeviceExternalBufferInfoKHR(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const VkBufferCreateFlags flags,
    const VkBufferUsageFlags usage,
    const VkExternalMemoryHandleTypeFlagBits handleType) const;
bool PreCallValidateGetPhysicalDeviceExternalBufferPropertiesKHR(VkPhysicalDevice physicalDevice,
    const VkPhysicalDeviceExternalBufferInfo* pExternalBufferInfo,
    VkExternalBufferProperties* pExternalBufferProperties) const override;
bool ValidateExternalMemoryImageCreateInfoKHR(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const VkExternalMemoryHandleTypeFlags handleTypes) const;
bool ValidateExternalMemoryBufferCreateInfoKHR(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const VkExternalMemoryHandleTypeFlags handleTypes) const;
bool ValidateExportMemoryAllocateInfoKHR(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const VkExternalMemoryHandleTypeFlags handleTypes) const;
#ifdef VK_USE_PLATFORM_WIN32_KHR
bool ValidateImportMemoryWin32HandleInfoKHR(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const VkExternalMemoryHandleTypeFlagBits handleType,
    const HANDLE handle,
    const LPCWSTR name) const;
#endif
#ifdef VK_USE_PLATFORM_WIN32_KHR
bool ValidateExportMemoryWin32HandleInfoKHR(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const SECURITY_ATTRIBUTES* pAttributes,
    const DWORD dwAccess,
    const LPCWSTR name) const;
#endif
#ifdef VK_USE_PLATFORM_WIN32_KHR
bool ValidateMemoryGetWin32HandleInfoKHR(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const VkDeviceMemory memory,
    const VkExternalMemoryHandleTypeFlagBits handleType) const;
#endif
#ifdef VK_USE_PLATFORM_WIN32_KHR
bool PreCallValidateGetMemoryWin32HandleKHR(VkDevice device,
    const VkMemoryGetWin32HandleInfoKHR* pGetWin32HandleInfo,
    HANDLE* pHandle) const override;
#endif
#ifdef VK_USE_PLATFORM_WIN32_KHR
bool PreCallValidateGetMemoryWin32HandlePropertiesKHR(VkDevice device,
    VkExternalMemoryHandleTypeFlagBits handleType,
    HANDLE handle,
    VkMemoryWin32HandlePropertiesKHR* pMemoryWin32HandleProperties) const override;
#endif
bool ValidateImportMemoryFdInfoKHR(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const VkExternalMemoryHandleTypeFlagBits handleType,
    const int fd) const;
bool ValidateMemoryGetFdInfoKHR(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const VkDeviceMemory memory,
    const VkExternalMemoryHandleTypeFlagBits handleType) const;
bool PreCallValidateGetMemoryFdKHR(VkDevice device,
    const VkMemoryGetFdInfoKHR* pGetFdInfo,
    int* pFd) const override;
bool PreCallValidateGetMemoryFdPropertiesKHR(VkDevice device,
    VkExternalMemoryHandleTypeFlagBits handleType,
    int fd,
    VkMemoryFdPropertiesKHR* pMemoryFdProperties) const override;
#ifdef VK_USE_PLATFORM_WIN32_KHR
bool ValidateWin32KeyedMutexAcquireReleaseInfoKHR(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const uint32_t acquireCount,
    const VkDeviceMemory* pAcquireSyncs,
    const uint64_t* pAcquireKeys,
    const uint32_t* pAcquireTimeouts,
    const uint32_t releaseCount,
    const VkDeviceMemory* pReleaseSyncs,
    const uint64_t* pReleaseKeys) const;
#endif
bool ValidatePhysicalDeviceExternalSemaphoreInfoKHR(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const VkExternalSemaphoreHandleTypeFlagBits handleType) const;
bool PreCallValidateGetPhysicalDeviceExternalSemaphorePropertiesKHR(VkPhysicalDevice physicalDevice,
    const VkPhysicalDeviceExternalSemaphoreInfo* pExternalSemaphoreInfo,
    VkExternalSemaphoreProperties* pExternalSemaphoreProperties) const override;
bool ValidateExportSemaphoreCreateInfoKHR(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const VkExternalSemaphoreHandleTypeFlags handleTypes) const;
#ifdef VK_USE_PLATFORM_WIN32_KHR
bool ValidateImportSemaphoreWin32HandleInfoKHR(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const VkSemaphore semaphore,
    const VkSemaphoreImportFlags flags,
    const VkExternalSemaphoreHandleTypeFlagBits handleType,
    const HANDLE handle,
    const LPCWSTR name) const;
#endif
#ifdef VK_USE_PLATFORM_WIN32_KHR
bool ValidateExportSemaphoreWin32HandleInfoKHR(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const SECURITY_ATTRIBUTES* pAttributes,
    const DWORD dwAccess,
    const LPCWSTR name) const;
#endif
#ifdef VK_USE_PLATFORM_WIN32_KHR
bool ValidateD3D12FenceSubmitInfoKHR(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const uint32_t waitSemaphoreValuesCount,
    const uint64_t* pWaitSemaphoreValues,
    const uint32_t signalSemaphoreValuesCount,
    const uint64_t* pSignalSemaphoreValues) const;
#endif
#ifdef VK_USE_PLATFORM_WIN32_KHR
bool ValidateSemaphoreGetWin32HandleInfoKHR(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const VkSemaphore semaphore,
    const VkExternalSemaphoreHandleTypeFlagBits handleType) const;
#endif
#ifdef VK_USE_PLATFORM_WIN32_KHR
bool PreCallValidateImportSemaphoreWin32HandleKHR(VkDevice device,
    const VkImportSemaphoreWin32HandleInfoKHR* pImportSemaphoreWin32HandleInfo) const override;
#endif
#ifdef VK_USE_PLATFORM_WIN32_KHR
bool PreCallValidateGetSemaphoreWin32HandleKHR(VkDevice device,
    const VkSemaphoreGetWin32HandleInfoKHR* pGetWin32HandleInfo,
    HANDLE* pHandle) const override;
#endif
bool ValidateImportSemaphoreFdInfoKHR(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const VkSemaphore semaphore,
    const VkSemaphoreImportFlags flags,
    const VkExternalSemaphoreHandleTypeFlagBits handleType,
    const int fd) const;
bool ValidateSemaphoreGetFdInfoKHR(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const VkSemaphore semaphore,
    const VkExternalSemaphoreHandleTypeFlagBits handleType) const;
bool PreCallValidateImportSemaphoreFdKHR(VkDevice device,
    const VkImportSemaphoreFdInfoKHR* pImportSemaphoreFdInfo) const override;
bool PreCallValidateGetSemaphoreFdKHR(VkDevice device,
    const VkSemaphoreGetFdInfoKHR* pGetFdInfo,
    int* pFd) const override;
bool PreCallValidateCmdPushDescriptorSetKHR(VkCommandBuffer commandBuffer,
    VkPipelineBindPoint pipelineBindPoint,
    VkPipelineLayout layout,
    uint32_t set,
    uint32_t descriptorWriteCount,
    const VkWriteDescriptorSet* pDescriptorWrites) const override;
bool PreCallValidateCmdPushDescriptorSetWithTemplateKHR(VkCommandBuffer commandBuffer,
    VkDescriptorUpdateTemplate descriptorUpdateTemplate,
    VkPipelineLayout layout,
    uint32_t set,
    const void* pData) const override;
bool ValidatePhysicalDeviceShaderFloat16Int8FeaturesKHR(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const VkBool32 shaderFloat16,
    const VkBool32 shaderInt8) const;
bool ValidatePhysicalDeviceFloat16Int8FeaturesKHR(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const VkBool32 shaderFloat16,
    const VkBool32 shaderInt8) const;
bool ValidatePhysicalDevice16BitStorageFeaturesKHR(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const VkBool32 storageBuffer16BitAccess,
    const VkBool32 uniformAndStorageBuffer16BitAccess,
    const VkBool32 storagePushConstant16,
    const VkBool32 storageInputOutput16) const;
bool ValidateRectLayerKHR(const LogObjectList &_parentObjects,
    const VkOffset2D offset,
    const VkExtent2D extent,
    const uint32_t layer) const;
bool ValidatePresentRegionKHR(const LogObjectList &_parentObjects,
    const uint32_t rectangleCount,
    const VkRectLayerKHR* pRectangles) const;
bool ValidatePresentRegionsKHR(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const uint32_t swapchainCount,
    const VkPresentRegionKHR* pRegions) const;
bool ValidateDescriptorUpdateTemplateEntryKHR(const LogObjectList &_parentObjects,
    const uint32_t dstBinding,
    const uint32_t dstArrayElement,
    const uint32_t descriptorCount,
    const VkDescriptorType descriptorType,
    const size_t offset,
    const size_t stride) const;
bool ValidateDescriptorUpdateTemplateCreateInfoKHR(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const VkDescriptorUpdateTemplateCreateFlags flags,
    const uint32_t descriptorUpdateEntryCount,
    const VkDescriptorUpdateTemplateEntry* pDescriptorUpdateEntries,
    const VkDescriptorUpdateTemplateType templateType,
    const VkDescriptorSetLayout descriptorSetLayout,
    const VkPipelineBindPoint pipelineBindPoint,
    const VkPipelineLayout pipelineLayout,
    const uint32_t set) const;
bool PreCallValidateCreateDescriptorUpdateTemplateKHR(VkDevice device,
    const VkDescriptorUpdateTemplateCreateInfo* pCreateInfo,
    const VkAllocationCallbacks* pAllocator,
    VkDescriptorUpdateTemplate* pDescriptorUpdateTemplate) const override;
bool PreCallValidateDestroyDescriptorUpdateTemplateKHR(VkDevice device,
    VkDescriptorUpdateTemplate descriptorUpdateTemplate,
    const VkAllocationCallbacks* pAllocator) const override;
bool PreCallValidateUpdateDescriptorSetWithTemplateKHR(VkDevice device,
    VkDescriptorSet descriptorSet,
    VkDescriptorUpdateTemplate descriptorUpdateTemplate,
    const void* pData) const override;
bool ValidatePhysicalDeviceImagelessFramebufferFeaturesKHR(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const VkBool32 imagelessFramebuffer) const;
bool ValidateFramebufferAttachmentsCreateInfoKHR(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const uint32_t attachmentImageInfoCount,
    const VkFramebufferAttachmentImageInfo* pAttachmentImageInfos) const;
bool ValidateFramebufferAttachmentImageInfoKHR(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const VkImageCreateFlags flags,
    const VkImageUsageFlags usage,
    const uint32_t width,
    const uint32_t height,
    const uint32_t layerCount,
    const uint32_t viewFormatCount,
    const VkFormat* pViewFormats) const;
bool ValidateRenderPassAttachmentBeginInfoKHR(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const uint32_t attachmentCount,
    const VkImageView* pAttachments) const;
bool ValidateRenderPassCreateInfo2KHR(const LogObjectList &_parentObjects,
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
    const uint32_t* pCorrelatedViewMasks) const;
bool ValidateAttachmentDescription2KHR(const LogObjectList &_parentObjects,
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
    const VkImageLayout finalLayout) const;
bool ValidateAttachmentReference2KHR(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const uint32_t attachment,
    const VkImageLayout layout,
    const VkImageAspectFlags aspectMask) const;
bool ValidateSubpassDescription2KHR(const LogObjectList &_parentObjects,
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
    const uint32_t* pPreserveAttachments) const;
bool ValidateSubpassDependency2KHR(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const uint32_t srcSubpass,
    const uint32_t dstSubpass,
    const VkPipelineStageFlags srcStageMask,
    const VkPipelineStageFlags dstStageMask,
    const VkAccessFlags srcAccessMask,
    const VkAccessFlags dstAccessMask,
    const VkDependencyFlags dependencyFlags,
    const int32_t viewOffset) const;
bool ValidateSubpassBeginInfoKHR(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const VkSubpassContents contents) const;
bool ValidateSubpassEndInfoKHR(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext) const;
bool PreCallValidateCreateRenderPass2KHR(VkDevice device,
    const VkRenderPassCreateInfo2* pCreateInfo,
    const VkAllocationCallbacks* pAllocator,
    VkRenderPass* pRenderPass) const override;
bool PreCallValidateCmdBeginRenderPass2KHR(VkCommandBuffer commandBuffer,
    const VkRenderPassBeginInfo* pRenderPassBegin,
    const VkSubpassBeginInfo* pSubpassBeginInfo) const override;
bool PreCallValidateCmdNextSubpass2KHR(VkCommandBuffer commandBuffer,
    const VkSubpassBeginInfo* pSubpassBeginInfo,
    const VkSubpassEndInfo* pSubpassEndInfo) const override;
bool PreCallValidateCmdEndRenderPass2KHR(VkCommandBuffer commandBuffer,
    const VkSubpassEndInfo* pSubpassEndInfo) const override;
bool PreCallValidateGetSwapchainStatusKHR(VkDevice device,
    VkSwapchainKHR swapchain) const override;
bool ValidatePhysicalDeviceExternalFenceInfoKHR(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const VkExternalFenceHandleTypeFlagBits handleType) const;
bool PreCallValidateGetPhysicalDeviceExternalFencePropertiesKHR(VkPhysicalDevice physicalDevice,
    const VkPhysicalDeviceExternalFenceInfo* pExternalFenceInfo,
    VkExternalFenceProperties* pExternalFenceProperties) const override;
bool ValidateExportFenceCreateInfoKHR(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const VkExternalFenceHandleTypeFlags handleTypes) const;
#ifdef VK_USE_PLATFORM_WIN32_KHR
bool ValidateImportFenceWin32HandleInfoKHR(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const VkFence fence,
    const VkFenceImportFlags flags,
    const VkExternalFenceHandleTypeFlagBits handleType,
    const HANDLE handle,
    const LPCWSTR name) const;
#endif
#ifdef VK_USE_PLATFORM_WIN32_KHR
bool ValidateExportFenceWin32HandleInfoKHR(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const SECURITY_ATTRIBUTES* pAttributes,
    const DWORD dwAccess,
    const LPCWSTR name) const;
#endif
#ifdef VK_USE_PLATFORM_WIN32_KHR
bool ValidateFenceGetWin32HandleInfoKHR(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const VkFence fence,
    const VkExternalFenceHandleTypeFlagBits handleType) const;
#endif
#ifdef VK_USE_PLATFORM_WIN32_KHR
bool PreCallValidateImportFenceWin32HandleKHR(VkDevice device,
    const VkImportFenceWin32HandleInfoKHR* pImportFenceWin32HandleInfo) const override;
#endif
#ifdef VK_USE_PLATFORM_WIN32_KHR
bool PreCallValidateGetFenceWin32HandleKHR(VkDevice device,
    const VkFenceGetWin32HandleInfoKHR* pGetWin32HandleInfo,
    HANDLE* pHandle) const override;
#endif
bool ValidateImportFenceFdInfoKHR(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const VkFence fence,
    const VkFenceImportFlags flags,
    const VkExternalFenceHandleTypeFlagBits handleType,
    const int fd) const;
bool ValidateFenceGetFdInfoKHR(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const VkFence fence,
    const VkExternalFenceHandleTypeFlagBits handleType) const;
bool PreCallValidateImportFenceFdKHR(VkDevice device,
    const VkImportFenceFdInfoKHR* pImportFenceFdInfo) const override;
bool PreCallValidateGetFenceFdKHR(VkDevice device,
    const VkFenceGetFdInfoKHR* pGetFdInfo,
    int* pFd) const override;
bool ValidatePhysicalDevicePerformanceQueryFeaturesKHR(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const VkBool32 performanceCounterQueryPools,
    const VkBool32 performanceCounterMultipleQueryPools) const;
bool ValidateQueryPoolPerformanceCreateInfoKHR(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const uint32_t queueFamilyIndex,
    const uint32_t counterIndexCount,
    const uint32_t* pCounterIndices) const;
bool ValidatePerformanceCounterResultKHR(const LogObjectList &_parentObjects,
    const int32_t int32,
    const int64_t int64,
    const uint32_t uint32,
    const uint64_t uint64,
    const float float32,
    const double float64) const;
bool ValidateAcquireProfilingLockInfoKHR(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const VkAcquireProfilingLockFlagsKHR flags,
    const uint64_t timeout) const;
bool ValidatePerformanceQuerySubmitInfoKHR(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const uint32_t counterPassIndex) const;
bool PreCallValidateEnumeratePhysicalDeviceQueueFamilyPerformanceQueryCountersKHR(VkPhysicalDevice physicalDevice,
    uint32_t queueFamilyIndex,
    uint32_t* pCounterCount,
    VkPerformanceCounterKHR* pCounters,
    VkPerformanceCounterDescriptionKHR* pCounterDescriptions) const override;
bool PreCallValidateGetPhysicalDeviceQueueFamilyPerformanceQueryPassesKHR(VkPhysicalDevice physicalDevice,
    const VkQueryPoolPerformanceCreateInfoKHR* pPerformanceQueryCreateInfo,
    uint32_t* pNumPasses) const override;
bool PreCallValidateAcquireProfilingLockKHR(VkDevice device,
    const VkAcquireProfilingLockInfoKHR* pInfo) const override;
bool PreCallValidateReleaseProfilingLockKHR(VkDevice device) const override;
bool ValidateRenderPassInputAttachmentAspectCreateInfoKHR(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const uint32_t aspectReferenceCount,
    const VkInputAttachmentAspectReference* pAspectReferences) const;
bool ValidateInputAttachmentAspectReferenceKHR(const LogObjectList &_parentObjects,
    const uint32_t subpass,
    const uint32_t inputAttachmentIndex,
    const VkImageAspectFlags aspectMask) const;
bool ValidateImageViewUsageCreateInfoKHR(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const VkImageUsageFlags usage) const;
bool ValidatePipelineTessellationDomainOriginStateCreateInfoKHR(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const VkTessellationDomainOrigin domainOrigin) const;
bool ValidatePhysicalDeviceSurfaceInfo2KHR(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const VkSurfaceKHR surface) const;
bool PreCallValidateGetPhysicalDeviceSurfaceCapabilities2KHR(VkPhysicalDevice physicalDevice,
    const VkPhysicalDeviceSurfaceInfo2KHR* pSurfaceInfo,
    VkSurfaceCapabilities2KHR* pSurfaceCapabilities) const override;
bool PreCallValidateGetPhysicalDeviceSurfaceFormats2KHR(VkPhysicalDevice physicalDevice,
    const VkPhysicalDeviceSurfaceInfo2KHR* pSurfaceInfo,
    uint32_t* pSurfaceFormatCount,
    VkSurfaceFormat2KHR* pSurfaceFormats) const override;
bool ValidatePhysicalDeviceVariablePointerFeaturesKHR(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const VkBool32 variablePointersStorageBuffer,
    const VkBool32 variablePointers) const;
bool ValidatePhysicalDeviceVariablePointersFeaturesKHR(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const VkBool32 variablePointersStorageBuffer,
    const VkBool32 variablePointers) const;
bool ValidateDisplayPlaneInfo2KHR(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const VkDisplayModeKHR mode,
    const uint32_t planeIndex) const;
bool PreCallValidateGetPhysicalDeviceDisplayProperties2KHR(VkPhysicalDevice physicalDevice,
    uint32_t* pPropertyCount,
    VkDisplayProperties2KHR* pProperties) const override;
bool PreCallValidateGetPhysicalDeviceDisplayPlaneProperties2KHR(VkPhysicalDevice physicalDevice,
    uint32_t* pPropertyCount,
    VkDisplayPlaneProperties2KHR* pProperties) const override;
bool PreCallValidateGetDisplayModeProperties2KHR(VkPhysicalDevice physicalDevice,
    VkDisplayKHR display,
    uint32_t* pPropertyCount,
    VkDisplayModeProperties2KHR* pProperties) const override;
bool PreCallValidateGetDisplayPlaneCapabilities2KHR(VkPhysicalDevice physicalDevice,
    const VkDisplayPlaneInfo2KHR* pDisplayPlaneInfo,
    VkDisplayPlaneCapabilities2KHR* pCapabilities) const override;
bool ValidateMemoryDedicatedAllocateInfoKHR(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const VkImage image,
    const VkBuffer buffer) const;
bool ValidateBufferMemoryRequirementsInfo2KHR(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const VkBuffer buffer) const;
bool ValidateImageMemoryRequirementsInfo2KHR(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const VkImage image) const;
bool ValidateImageSparseMemoryRequirementsInfo2KHR(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const VkImage image) const;
bool PreCallValidateGetImageMemoryRequirements2KHR(VkDevice device,
    const VkImageMemoryRequirementsInfo2* pInfo,
    VkMemoryRequirements2* pMemoryRequirements) const override;
bool PreCallValidateGetBufferMemoryRequirements2KHR(VkDevice device,
    const VkBufferMemoryRequirementsInfo2* pInfo,
    VkMemoryRequirements2* pMemoryRequirements) const override;
bool PreCallValidateGetImageSparseMemoryRequirements2KHR(VkDevice device,
    const VkImageSparseMemoryRequirementsInfo2* pInfo,
    uint32_t* pSparseMemoryRequirementCount,
    VkSparseImageMemoryRequirements2* pSparseMemoryRequirements) const override;
bool ValidateImageFormatListCreateInfoKHR(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const uint32_t viewFormatCount,
    const VkFormat* pViewFormats) const;
bool ValidateSamplerYcbcrConversionCreateInfoKHR(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const VkFormat format,
    const VkSamplerYcbcrModelConversion ycbcrModel,
    const VkSamplerYcbcrRange ycbcrRange,
    const VkComponentMapping components,
    const VkChromaLocation xChromaOffset,
    const VkChromaLocation yChromaOffset,
    const VkFilter chromaFilter,
    VkBool32 forceExplicitReconstruction) const;
bool ValidateSamplerYcbcrConversionInfoKHR(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const VkSamplerYcbcrConversion conversion) const;
bool ValidateBindImagePlaneMemoryInfoKHR(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const VkImageAspectFlagBits planeAspect) const;
bool ValidateImagePlaneMemoryRequirementsInfoKHR(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const VkImageAspectFlagBits planeAspect) const;
bool ValidatePhysicalDeviceSamplerYcbcrConversionFeaturesKHR(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const VkBool32 samplerYcbcrConversion) const;
bool PreCallValidateCreateSamplerYcbcrConversionKHR(VkDevice device,
    const VkSamplerYcbcrConversionCreateInfo* pCreateInfo,
    const VkAllocationCallbacks* pAllocator,
    VkSamplerYcbcrConversion* pYcbcrConversion) const override;
bool PreCallValidateDestroySamplerYcbcrConversionKHR(VkDevice device,
    VkSamplerYcbcrConversion ycbcrConversion,
    const VkAllocationCallbacks* pAllocator) const override;
bool ValidateBindBufferMemoryInfoKHR(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const VkBuffer buffer,
    const VkDeviceMemory memory,
    const VkDeviceSize memoryOffset) const;
bool ValidateBindImageMemoryInfoKHR(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const VkImage image,
    const VkDeviceMemory memory,
    const VkDeviceSize memoryOffset) const;
bool PreCallValidateBindBufferMemory2KHR(VkDevice device,
    uint32_t bindInfoCount,
    const VkBindBufferMemoryInfo* pBindInfos) const override;
bool PreCallValidateBindImageMemory2KHR(VkDevice device,
    uint32_t bindInfoCount,
    const VkBindImageMemoryInfo* pBindInfos) const override;
#ifdef VK_ENABLE_BETA_EXTENSIONS
bool ValidatePhysicalDevicePortabilitySubsetFeaturesKHR(const LogObjectList &_parentObjects,
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
    const VkBool32 vertexAttributeAccessBeyondStride) const;
#endif
#ifdef VK_ENABLE_BETA_EXTENSIONS
bool ValidatePhysicalDevicePortabilitySubsetPropertiesKHR(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const uint32_t minVertexInputBindingStrideAlignment) const;
#endif
bool PreCallValidateGetDescriptorSetLayoutSupportKHR(VkDevice device,
    const VkDescriptorSetLayoutCreateInfo* pCreateInfo,
    VkDescriptorSetLayoutSupport* pSupport) const override;
bool PreCallValidateCmdDrawIndirectCountKHR(VkCommandBuffer commandBuffer,
    VkBuffer buffer,
    VkDeviceSize offset,
    VkBuffer countBuffer,
    VkDeviceSize countBufferOffset,
    uint32_t maxDrawCount,
    uint32_t stride) const override;
bool PreCallValidateCmdDrawIndexedIndirectCountKHR(VkCommandBuffer commandBuffer,
    VkBuffer buffer,
    VkDeviceSize offset,
    VkBuffer countBuffer,
    VkDeviceSize countBufferOffset,
    uint32_t maxDrawCount,
    uint32_t stride) const override;
bool ValidatePhysicalDeviceShaderSubgroupExtendedTypesFeaturesKHR(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const VkBool32 shaderSubgroupExtendedTypes) const;
bool ValidatePhysicalDevice8BitStorageFeaturesKHR(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const VkBool32 storageBuffer8BitAccess,
    const VkBool32 uniformAndStorageBuffer8BitAccess,
    const VkBool32 storagePushConstant8) const;
bool ValidatePhysicalDeviceShaderAtomicInt64FeaturesKHR(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const VkBool32 shaderBufferInt64Atomics,
    const VkBool32 shaderSharedInt64Atomics) const;
bool ValidatePhysicalDeviceShaderClockFeaturesKHR(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const VkBool32 shaderSubgroupClock,
    const VkBool32 shaderDeviceClock) const;
bool ValidateVideoDecodeH265ProfileInfoKHR(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const StdVideoH265ProfileIdc stdProfileIdc) const;
bool ValidateVideoDecodeH265SessionParametersAddInfoKHR(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const uint32_t stdVPSCount,
    const StdVideoH265VideoParameterSet* pStdVPSs,
    const uint32_t stdSPSCount,
    const StdVideoH265SequenceParameterSet* pStdSPSs,
    const uint32_t stdPPSCount,
    const StdVideoH265PictureParameterSet* pStdPPSs) const;
bool ValidateVideoDecodeH265SessionParametersCreateInfoKHR(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const uint32_t maxStdVPSCount,
    const uint32_t maxStdSPSCount,
    const uint32_t maxStdPPSCount,
    const VkVideoDecodeH265SessionParametersAddInfoKHR* pParametersAddInfo) const;
bool ValidateVideoDecodeH265PictureInfoKHR(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const StdVideoDecodeH265PictureInfo* pStdPictureInfo,
    const uint32_t sliceSegmentCount,
    const uint32_t* pSliceSegmentOffsets) const;
bool ValidateVideoDecodeH265DpbSlotInfoKHR(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const StdVideoDecodeH265ReferenceInfo* pStdReferenceInfo) const;
bool ValidateDeviceQueueGlobalPriorityCreateInfoKHR(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const VkQueueGlobalPriorityKHR globalPriority) const;
bool ValidatePhysicalDeviceGlobalPriorityQueryFeaturesKHR(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const VkBool32 globalPriorityQuery) const;
bool ValidateQueueFamilyGlobalPriorityPropertiesKHR(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const uint32_t priorityCount,
    const VkQueueGlobalPriorityKHR priorities[VK_MAX_GLOBAL_PRIORITY_SIZE_KHR]) const;
bool ValidateConformanceVersionKHR(const LogObjectList &_parentObjects,
    const uint8_t major,
    const uint8_t minor,
    const uint8_t subminor,
    const uint8_t patch) const;
bool ValidateSubpassDescriptionDepthStencilResolveKHR(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const VkResolveModeFlagBits depthResolveMode,
    const VkResolveModeFlagBits stencilResolveMode,
    const VkAttachmentReference2* pDepthStencilResolveAttachment) const;
bool ValidatePhysicalDeviceTimelineSemaphoreFeaturesKHR(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const VkBool32 timelineSemaphore) const;
bool ValidateSemaphoreTypeCreateInfoKHR(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const VkSemaphoreType semaphoreType,
    const uint64_t initialValue) const;
bool ValidateTimelineSemaphoreSubmitInfoKHR(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const uint32_t waitSemaphoreValueCount,
    const uint64_t* pWaitSemaphoreValues,
    const uint32_t signalSemaphoreValueCount,
    const uint64_t* pSignalSemaphoreValues) const;
bool ValidateSemaphoreWaitInfoKHR(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const VkSemaphoreWaitFlags flags,
    const uint32_t semaphoreCount,
    const VkSemaphore* pSemaphores,
    const uint64_t* pValues) const;
bool ValidateSemaphoreSignalInfoKHR(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const VkSemaphore semaphore,
    const uint64_t value) const;
bool PreCallValidateGetSemaphoreCounterValueKHR(VkDevice device,
    VkSemaphore semaphore,
    uint64_t* pValue) const override;
bool PreCallValidateWaitSemaphoresKHR(VkDevice device,
    const VkSemaphoreWaitInfo* pWaitInfo,
    uint64_t timeout) const override;
bool PreCallValidateSignalSemaphoreKHR(VkDevice device,
    const VkSemaphoreSignalInfo* pSignalInfo) const override;
bool ValidatePhysicalDeviceVulkanMemoryModelFeaturesKHR(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const VkBool32 vulkanMemoryModel,
    const VkBool32 vulkanMemoryModelDeviceScope,
    const VkBool32 vulkanMemoryModelAvailabilityVisibilityChains) const;
bool ValidatePhysicalDeviceShaderTerminateInvocationFeaturesKHR(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const VkBool32 shaderTerminateInvocation) const;
bool ValidateFragmentShadingRateAttachmentInfoKHR(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const VkAttachmentReference2* pFragmentShadingRateAttachment,
    const VkExtent2D shadingRateAttachmentTexelSize) const;
bool ValidatePipelineFragmentShadingRateStateCreateInfoKHR(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const VkExtent2D fragmentSize,
    const VkFragmentShadingRateCombinerOpKHR combinerOps[2]) const;
bool ValidatePhysicalDeviceFragmentShadingRateFeaturesKHR(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const VkBool32 pipelineFragmentShadingRate,
    const VkBool32 primitiveFragmentShadingRate,
    const VkBool32 attachmentFragmentShadingRate) const;
bool PreCallValidateGetPhysicalDeviceFragmentShadingRatesKHR(VkPhysicalDevice physicalDevice,
    uint32_t* pFragmentShadingRateCount,
    VkPhysicalDeviceFragmentShadingRateKHR* pFragmentShadingRates) const override;
bool PreCallValidateCmdSetFragmentShadingRateKHR(VkCommandBuffer commandBuffer,
    const VkExtent2D* pFragmentSize,
    const VkFragmentShadingRateCombinerOpKHR combinerOps[2]) const override;
bool ValidateSurfaceProtectedCapabilitiesKHR(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const VkBool32 supportsProtected) const;
bool ValidatePhysicalDeviceSeparateDepthStencilLayoutsFeaturesKHR(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const VkBool32 separateDepthStencilLayouts) const;
bool ValidateAttachmentReferenceStencilLayoutKHR(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const VkImageLayout stencilLayout) const;
bool ValidateAttachmentDescriptionStencilLayoutKHR(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const VkImageLayout stencilInitialLayout,
    const VkImageLayout stencilFinalLayout) const;
bool ValidatePhysicalDevicePresentWaitFeaturesKHR(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const VkBool32 presentWait) const;
bool PreCallValidateWaitForPresentKHR(VkDevice device,
    VkSwapchainKHR swapchain,
    uint64_t presentId,
    uint64_t timeout) const override;
bool ValidatePhysicalDeviceUniformBufferStandardLayoutFeaturesKHR(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const VkBool32 uniformBufferStandardLayout) const;
bool ValidatePhysicalDeviceBufferDeviceAddressFeaturesKHR(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const VkBool32 bufferDeviceAddress,
    const VkBool32 bufferDeviceAddressCaptureReplay,
    const VkBool32 bufferDeviceAddressMultiDevice) const;
bool ValidateBufferDeviceAddressInfoKHR(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const VkBuffer buffer) const;
bool ValidateBufferOpaqueCaptureAddressCreateInfoKHR(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const uint64_t opaqueCaptureAddress) const;
bool ValidateMemoryOpaqueCaptureAddressAllocateInfoKHR(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const uint64_t opaqueCaptureAddress) const;
bool ValidateDeviceMemoryOpaqueCaptureAddressInfoKHR(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const VkDeviceMemory memory) const;
bool PreCallValidateGetBufferDeviceAddressKHR(VkDevice device,
    const VkBufferDeviceAddressInfo* pInfo) const override;
bool PreCallValidateGetBufferOpaqueCaptureAddressKHR(VkDevice device,
    const VkBufferDeviceAddressInfo* pInfo) const override;
bool PreCallValidateGetDeviceMemoryOpaqueCaptureAddressKHR(VkDevice device,
    const VkDeviceMemoryOpaqueCaptureAddressInfo* pInfo) const override;
bool PreCallValidateCreateDeferredOperationKHR(VkDevice device,
    const VkAllocationCallbacks* pAllocator,
    VkDeferredOperationKHR* pDeferredOperation) const override;
bool PreCallValidateDestroyDeferredOperationKHR(VkDevice device,
    VkDeferredOperationKHR operation,
    const VkAllocationCallbacks* pAllocator) const override;
bool PreCallValidateGetDeferredOperationMaxConcurrencyKHR(VkDevice device,
    VkDeferredOperationKHR operation) const override;
bool PreCallValidateGetDeferredOperationResultKHR(VkDevice device,
    VkDeferredOperationKHR operation) const override;
bool PreCallValidateDeferredOperationJoinKHR(VkDevice device,
    VkDeferredOperationKHR operation) const override;
bool ValidatePhysicalDevicePipelineExecutablePropertiesFeaturesKHR(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const VkBool32 pipelineExecutableInfo) const;
bool ValidatePipelineInfoKHR(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const VkPipeline pipeline) const;
bool ValidatePipelineExecutableInfoKHR(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const VkPipeline pipeline,
    const uint32_t executableIndex) const;
bool PreCallValidateGetPipelineExecutablePropertiesKHR(VkDevice device,
    const VkPipelineInfoKHR* pPipelineInfo,
    uint32_t* pExecutableCount,
    VkPipelineExecutablePropertiesKHR* pProperties) const override;
bool PreCallValidateGetPipelineExecutableStatisticsKHR(VkDevice device,
    const VkPipelineExecutableInfoKHR* pExecutableInfo,
    uint32_t* pStatisticCount,
    VkPipelineExecutableStatisticKHR* pStatistics) const override;
bool PreCallValidateGetPipelineExecutableInternalRepresentationsKHR(VkDevice device,
    const VkPipelineExecutableInfoKHR* pExecutableInfo,
    uint32_t* pInternalRepresentationCount,
    VkPipelineExecutableInternalRepresentationKHR* pInternalRepresentations) const override;
bool ValidateMemoryMapInfoKHR(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const VkMemoryMapFlags flags,
    const VkDeviceMemory memory,
    const VkDeviceSize offset,
    const VkDeviceSize size) const;
bool ValidateMemoryUnmapInfoKHR(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const VkMemoryUnmapFlagsKHR flags,
    const VkDeviceMemory memory) const;
bool PreCallValidateMapMemory2KHR(VkDevice device,
    const VkMemoryMapInfoKHR* pMemoryMapInfo,
    void** ppData) const override;
bool PreCallValidateUnmapMemory2KHR(VkDevice device,
    const VkMemoryUnmapInfoKHR* pMemoryUnmapInfo) const override;
bool ValidatePhysicalDeviceShaderIntegerDotProductFeaturesKHR(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const VkBool32 shaderIntegerDotProduct) const;
bool ValidatePipelineLibraryCreateInfoKHR(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const uint32_t libraryCount,
    const VkPipeline* pLibraries) const;
bool ValidatePresentIdKHR(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const uint32_t swapchainCount,
    const uint64_t* pPresentIds) const;
bool ValidatePhysicalDevicePresentIdFeaturesKHR(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const VkBool32 presentId) const;
#ifdef VK_ENABLE_BETA_EXTENSIONS
bool ValidateVideoEncodeInfoKHR(const LogObjectList &_parentObjects,
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
    const uint32_t precedingExternallyEncodedBytes) const;
#endif
#ifdef VK_ENABLE_BETA_EXTENSIONS
bool ValidateQueryPoolVideoEncodeFeedbackCreateInfoKHR(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const VkVideoEncodeFeedbackFlagsKHR encodeFeedbackFlags) const;
#endif
#ifdef VK_ENABLE_BETA_EXTENSIONS
bool ValidateVideoEncodeUsageInfoKHR(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const VkVideoEncodeUsageFlagsKHR videoUsageHints,
    const VkVideoEncodeContentFlagsKHR videoContentHints,
    const VkVideoEncodeTuningModeKHR tuningMode) const;
#endif
#ifdef VK_ENABLE_BETA_EXTENSIONS
bool ValidateVideoEncodeRateControlLayerInfoKHR(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const uint64_t averageBitrate,
    const uint64_t maxBitrate,
    const uint32_t frameRateNumerator,
    const uint32_t frameRateDenominator,
    const uint32_t virtualBufferSizeInMs,
    const uint32_t initialVirtualBufferSizeInMs) const;
#endif
#ifdef VK_ENABLE_BETA_EXTENSIONS
bool ValidateVideoEncodeRateControlInfoKHR(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const VkVideoEncodeRateControlFlagsKHR flags,
    const VkVideoEncodeRateControlModeFlagBitsKHR rateControlMode,
    const uint32_t layerCount,
    const VkVideoEncodeRateControlLayerInfoKHR* pLayers) const;
#endif
#ifdef VK_ENABLE_BETA_EXTENSIONS
bool PreCallValidateCmdEncodeVideoKHR(VkCommandBuffer commandBuffer,
    const VkVideoEncodeInfoKHR* pEncodeInfo) const override;
#endif
bool ValidateMemoryBarrier2KHR(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const VkPipelineStageFlags2 srcStageMask,
    const VkAccessFlags2 srcAccessMask,
    const VkPipelineStageFlags2 dstStageMask,
    const VkAccessFlags2 dstAccessMask) const;
bool ValidateBufferMemoryBarrier2KHR(const LogObjectList &_parentObjects,
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
    const VkDeviceSize size) const;
bool ValidateImageMemoryBarrier2KHR(const LogObjectList &_parentObjects,
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
    const VkImageSubresourceRange subresourceRange) const;
bool ValidateDependencyInfoKHR(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const VkDependencyFlags dependencyFlags,
    const uint32_t memoryBarrierCount,
    const VkMemoryBarrier2* pMemoryBarriers,
    const uint32_t bufferMemoryBarrierCount,
    const VkBufferMemoryBarrier2* pBufferMemoryBarriers,
    const uint32_t imageMemoryBarrierCount,
    const VkImageMemoryBarrier2* pImageMemoryBarriers) const;
bool ValidateSubmitInfo2KHR(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const VkSubmitFlags flags,
    const uint32_t waitSemaphoreInfoCount,
    const VkSemaphoreSubmitInfo* pWaitSemaphoreInfos,
    const uint32_t commandBufferInfoCount,
    const VkCommandBufferSubmitInfo* pCommandBufferInfos,
    const uint32_t signalSemaphoreInfoCount,
    const VkSemaphoreSubmitInfo* pSignalSemaphoreInfos) const;
bool ValidateSemaphoreSubmitInfoKHR(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const VkSemaphore semaphore,
    const uint64_t value,
    const VkPipelineStageFlags2 stageMask,
    const uint32_t deviceIndex) const;
bool ValidateCommandBufferSubmitInfoKHR(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const VkCommandBuffer commandBuffer,
    const uint32_t deviceMask) const;
bool ValidatePhysicalDeviceSynchronization2FeaturesKHR(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const VkBool32 synchronization2) const;
bool PreCallValidateCmdSetEvent2KHR(VkCommandBuffer commandBuffer,
    VkEvent event,
    const VkDependencyInfo* pDependencyInfo) const override;
bool PreCallValidateCmdResetEvent2KHR(VkCommandBuffer commandBuffer,
    VkEvent event,
    VkPipelineStageFlags2 stageMask) const override;
bool PreCallValidateCmdWaitEvents2KHR(VkCommandBuffer commandBuffer,
    uint32_t eventCount,
    const VkEvent* pEvents,
    const VkDependencyInfo* pDependencyInfos) const override;
bool PreCallValidateCmdPipelineBarrier2KHR(VkCommandBuffer commandBuffer,
    const VkDependencyInfo* pDependencyInfo) const override;
bool PreCallValidateCmdWriteTimestamp2KHR(VkCommandBuffer commandBuffer,
    VkPipelineStageFlags2 stage,
    VkQueryPool queryPool,
    uint32_t query) const override;
bool PreCallValidateQueueSubmit2KHR(VkQueue queue,
    uint32_t submitCount,
    const VkSubmitInfo2* pSubmits,
    VkFence fence) const override;
bool PreCallValidateCmdWriteBufferMarker2AMD(VkCommandBuffer commandBuffer,
    VkPipelineStageFlags2 stage,
    VkBuffer dstBuffer,
    VkDeviceSize dstOffset,
    uint32_t marker) const override;
bool PreCallValidateGetQueueCheckpointData2NV(VkQueue queue,
    uint32_t* pCheckpointDataCount,
    VkCheckpointData2NV* pCheckpointData) const override;
bool ValidatePhysicalDeviceFragmentShaderBarycentricFeaturesKHR(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const VkBool32 fragmentShaderBarycentric) const;
bool ValidatePhysicalDeviceShaderSubgroupUniformControlFlowFeaturesKHR(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const VkBool32 shaderSubgroupUniformControlFlow) const;
bool ValidatePhysicalDeviceZeroInitializeWorkgroupMemoryFeaturesKHR(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const VkBool32 shaderZeroInitializeWorkgroupMemory) const;
bool ValidatePhysicalDeviceWorkgroupMemoryExplicitLayoutFeaturesKHR(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const VkBool32 workgroupMemoryExplicitLayout,
    const VkBool32 workgroupMemoryExplicitLayoutScalarBlockLayout,
    const VkBool32 workgroupMemoryExplicitLayout8BitAccess,
    const VkBool32 workgroupMemoryExplicitLayout16BitAccess) const;
bool ValidateCopyBufferInfo2KHR(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const VkBuffer srcBuffer,
    const VkBuffer dstBuffer,
    const uint32_t regionCount,
    const VkBufferCopy2* pRegions) const;
bool ValidateCopyImageInfo2KHR(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const VkImage srcImage,
    const VkImageLayout srcImageLayout,
    const VkImage dstImage,
    const VkImageLayout dstImageLayout,
    const uint32_t regionCount,
    const VkImageCopy2* pRegions) const;
bool ValidateCopyBufferToImageInfo2KHR(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const VkBuffer srcBuffer,
    const VkImage dstImage,
    const VkImageLayout dstImageLayout,
    const uint32_t regionCount,
    const VkBufferImageCopy2* pRegions) const;
bool ValidateCopyImageToBufferInfo2KHR(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const VkImage srcImage,
    const VkImageLayout srcImageLayout,
    const VkBuffer dstBuffer,
    const uint32_t regionCount,
    const VkBufferImageCopy2* pRegions) const;
bool ValidateBlitImageInfo2KHR(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const VkImage srcImage,
    const VkImageLayout srcImageLayout,
    const VkImage dstImage,
    const VkImageLayout dstImageLayout,
    const uint32_t regionCount,
    const VkImageBlit2* pRegions,
    const VkFilter filter) const;
bool ValidateResolveImageInfo2KHR(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const VkImage srcImage,
    const VkImageLayout srcImageLayout,
    const VkImage dstImage,
    const VkImageLayout dstImageLayout,
    const uint32_t regionCount,
    const VkImageResolve2* pRegions) const;
bool ValidateBufferCopy2KHR(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const VkDeviceSize srcOffset,
    const VkDeviceSize dstOffset,
    const VkDeviceSize size) const;
bool ValidateImageCopy2KHR(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const VkImageSubresourceLayers srcSubresource,
    const VkOffset3D srcOffset,
    const VkImageSubresourceLayers dstSubresource,
    const VkOffset3D dstOffset,
    const VkExtent3D extent) const;
bool ValidateImageBlit2KHR(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const VkImageSubresourceLayers srcSubresource,
    const VkOffset3D srcOffsets[2],
    const VkImageSubresourceLayers dstSubresource,
    const VkOffset3D dstOffsets[2]) const;
bool ValidateBufferImageCopy2KHR(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const VkDeviceSize bufferOffset,
    const uint32_t bufferRowLength,
    const uint32_t bufferImageHeight,
    const VkImageSubresourceLayers imageSubresource,
    const VkOffset3D imageOffset,
    const VkExtent3D imageExtent) const;
bool ValidateImageResolve2KHR(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const VkImageSubresourceLayers srcSubresource,
    const VkOffset3D srcOffset,
    const VkImageSubresourceLayers dstSubresource,
    const VkOffset3D dstOffset,
    const VkExtent3D extent) const;
bool PreCallValidateCmdCopyBuffer2KHR(VkCommandBuffer commandBuffer,
    const VkCopyBufferInfo2* pCopyBufferInfo) const override;
bool PreCallValidateCmdCopyImage2KHR(VkCommandBuffer commandBuffer,
    const VkCopyImageInfo2* pCopyImageInfo) const override;
bool PreCallValidateCmdCopyBufferToImage2KHR(VkCommandBuffer commandBuffer,
    const VkCopyBufferToImageInfo2* pCopyBufferToImageInfo) const override;
bool PreCallValidateCmdCopyImageToBuffer2KHR(VkCommandBuffer commandBuffer,
    const VkCopyImageToBufferInfo2* pCopyImageToBufferInfo) const override;
bool PreCallValidateCmdBlitImage2KHR(VkCommandBuffer commandBuffer,
    const VkBlitImageInfo2* pBlitImageInfo) const override;
bool PreCallValidateCmdResolveImage2KHR(VkCommandBuffer commandBuffer,
    const VkResolveImageInfo2* pResolveImageInfo) const override;
bool ValidatePhysicalDeviceRayTracingMaintenance1FeaturesKHR(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const VkBool32 rayTracingMaintenance1,
    const VkBool32 rayTracingPipelineTraceRaysIndirect2) const;
bool ValidateTraceRaysIndirectCommand2KHR(const LogObjectList &_parentObjects,
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
    const uint32_t depth) const;
bool PreCallValidateCmdTraceRaysIndirect2KHR(VkCommandBuffer commandBuffer,
    VkDeviceAddress indirectDeviceAddress) const override;
bool ValidatePhysicalDeviceMaintenance4FeaturesKHR(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const VkBool32 maintenance4) const;
bool ValidateDeviceBufferMemoryRequirementsKHR(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const VkBufferCreateInfo* pCreateInfo) const;
bool ValidateDeviceImageMemoryRequirementsKHR(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const VkImageCreateInfo* pCreateInfo,
    const VkImageAspectFlagBits planeAspect) const;
bool PreCallValidateGetDeviceBufferMemoryRequirementsKHR(VkDevice device,
    const VkDeviceBufferMemoryRequirements* pInfo,
    VkMemoryRequirements2* pMemoryRequirements) const override;
bool PreCallValidateGetDeviceImageMemoryRequirementsKHR(VkDevice device,
    const VkDeviceImageMemoryRequirements* pInfo,
    VkMemoryRequirements2* pMemoryRequirements) const override;
bool PreCallValidateGetDeviceImageSparseMemoryRequirementsKHR(VkDevice device,
    const VkDeviceImageMemoryRequirements* pInfo,
    uint32_t* pSparseMemoryRequirementCount,
    VkSparseImageMemoryRequirements2* pSparseMemoryRequirements) const override;
bool ValidateDebugReportCallbackCreateInfoEXT(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const VkDebugReportFlagsEXT flags,
    const PFN_vkDebugReportCallbackEXT pfnCallback,
    const void* pUserData) const;
bool PreCallValidateCreateDebugReportCallbackEXT(VkInstance instance,
    const VkDebugReportCallbackCreateInfoEXT* pCreateInfo,
    const VkAllocationCallbacks* pAllocator,
    VkDebugReportCallbackEXT* pCallback) const override;
bool PreCallValidateDestroyDebugReportCallbackEXT(VkInstance instance,
    VkDebugReportCallbackEXT callback,
    const VkAllocationCallbacks* pAllocator) const override;
bool PreCallValidateDebugReportMessageEXT(VkInstance instance,
    VkDebugReportFlagsEXT flags,
    VkDebugReportObjectTypeEXT objectType,
    uint64_t object,
    size_t location,
    int32_t messageCode,
    const char* pLayerPrefix,
    const char* pMessage) const override;
bool ValidatePipelineRasterizationStateRasterizationOrderAMD(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const VkRasterizationOrderAMD rasterizationOrder) const;
bool ValidateDebugMarkerObjectNameInfoEXT(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const VkDebugReportObjectTypeEXT objectType,
    const uint64_t object,
    const char* pObjectName) const;
bool ValidateDebugMarkerObjectTagInfoEXT(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const VkDebugReportObjectTypeEXT objectType,
    const uint64_t object,
    const uint64_t tagName,
    const size_t tagSize,
    const void* pTag) const;
bool ValidateDebugMarkerMarkerInfoEXT(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const char* pMarkerName,
    const float color[4]) const;
bool PreCallValidateDebugMarkerSetObjectTagEXT(VkDevice device,
    const VkDebugMarkerObjectTagInfoEXT* pTagInfo) const override;
bool PreCallValidateDebugMarkerSetObjectNameEXT(VkDevice device,
    const VkDebugMarkerObjectNameInfoEXT* pNameInfo) const override;
bool PreCallValidateCmdDebugMarkerBeginEXT(VkCommandBuffer commandBuffer,
    const VkDebugMarkerMarkerInfoEXT* pMarkerInfo) const override;
bool PreCallValidateCmdDebugMarkerEndEXT(VkCommandBuffer commandBuffer) const override;
bool PreCallValidateCmdDebugMarkerInsertEXT(VkCommandBuffer commandBuffer,
    const VkDebugMarkerMarkerInfoEXT* pMarkerInfo) const override;
bool ValidateDedicatedAllocationImageCreateInfoNV(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const VkBool32 dedicatedAllocation) const;
bool ValidateDedicatedAllocationBufferCreateInfoNV(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const VkBool32 dedicatedAllocation) const;
bool ValidateDedicatedAllocationMemoryAllocateInfoNV(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const VkImage image,
    const VkBuffer buffer) const;
bool ValidatePhysicalDeviceTransformFeedbackFeaturesEXT(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const VkBool32 transformFeedback,
    const VkBool32 geometryStreams) const;
bool ValidatePipelineRasterizationStateStreamCreateInfoEXT(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const VkPipelineRasterizationStateStreamCreateFlagsEXT flags,
    const uint32_t rasterizationStream) const;
bool PreCallValidateCmdBindTransformFeedbackBuffersEXT(VkCommandBuffer commandBuffer,
    uint32_t firstBinding,
    uint32_t bindingCount,
    const VkBuffer* pBuffers,
    const VkDeviceSize* pOffsets,
    const VkDeviceSize* pSizes) const override;
bool PreCallValidateCmdBeginTransformFeedbackEXT(VkCommandBuffer commandBuffer,
    uint32_t firstCounterBuffer,
    uint32_t counterBufferCount,
    const VkBuffer* pCounterBuffers,
    const VkDeviceSize* pCounterBufferOffsets) const override;
bool PreCallValidateCmdEndTransformFeedbackEXT(VkCommandBuffer commandBuffer,
    uint32_t firstCounterBuffer,
    uint32_t counterBufferCount,
    const VkBuffer* pCounterBuffers,
    const VkDeviceSize* pCounterBufferOffsets) const override;
bool PreCallValidateCmdBeginQueryIndexedEXT(VkCommandBuffer commandBuffer,
    VkQueryPool queryPool,
    uint32_t query,
    VkQueryControlFlags flags,
    uint32_t index) const override;
bool PreCallValidateCmdEndQueryIndexedEXT(VkCommandBuffer commandBuffer,
    VkQueryPool queryPool,
    uint32_t query,
    uint32_t index) const override;
bool PreCallValidateCmdDrawIndirectByteCountEXT(VkCommandBuffer commandBuffer,
    uint32_t instanceCount,
    uint32_t firstInstance,
    VkBuffer counterBuffer,
    VkDeviceSize counterBufferOffset,
    uint32_t counterOffset,
    uint32_t vertexStride) const override;
bool ValidateCuModuleCreateInfoNVX(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const size_t dataSize,
    const void* pData) const;
bool ValidateCuFunctionCreateInfoNVX(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const VkCuModuleNVX module,
    const char* pName) const;
bool ValidateCuLaunchInfoNVX(const LogObjectList &_parentObjects,
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
    const void* const * pExtras) const;
bool PreCallValidateCreateCuModuleNVX(VkDevice device,
    const VkCuModuleCreateInfoNVX* pCreateInfo,
    const VkAllocationCallbacks* pAllocator,
    VkCuModuleNVX* pModule) const override;
bool PreCallValidateCreateCuFunctionNVX(VkDevice device,
    const VkCuFunctionCreateInfoNVX* pCreateInfo,
    const VkAllocationCallbacks* pAllocator,
    VkCuFunctionNVX* pFunction) const override;
bool PreCallValidateDestroyCuModuleNVX(VkDevice device,
    VkCuModuleNVX module,
    const VkAllocationCallbacks* pAllocator) const override;
bool PreCallValidateDestroyCuFunctionNVX(VkDevice device,
    VkCuFunctionNVX function,
    const VkAllocationCallbacks* pAllocator) const override;
bool PreCallValidateCmdCuLaunchKernelNVX(VkCommandBuffer commandBuffer,
    const VkCuLaunchInfoNVX* pLaunchInfo) const override;
bool ValidateImageViewHandleInfoNVX(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const VkImageView imageView,
    const VkDescriptorType descriptorType,
    const VkSampler sampler) const;
bool PreCallValidateGetImageViewHandleNVX(VkDevice device,
    const VkImageViewHandleInfoNVX* pInfo) const override;
bool PreCallValidateGetImageViewAddressNVX(VkDevice device,
    VkImageView imageView,
    VkImageViewAddressPropertiesNVX* pProperties) const override;
bool PreCallValidateCmdDrawIndirectCountAMD(VkCommandBuffer commandBuffer,
    VkBuffer buffer,
    VkDeviceSize offset,
    VkBuffer countBuffer,
    VkDeviceSize countBufferOffset,
    uint32_t maxDrawCount,
    uint32_t stride) const override;
bool PreCallValidateCmdDrawIndexedIndirectCountAMD(VkCommandBuffer commandBuffer,
    VkBuffer buffer,
    VkDeviceSize offset,
    VkBuffer countBuffer,
    VkDeviceSize countBufferOffset,
    uint32_t maxDrawCount,
    uint32_t stride) const override;
#ifdef VK_ENABLE_BETA_EXTENSIONS
bool ValidateVideoEncodeH264SessionParametersAddInfoEXT(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const uint32_t stdSPSCount,
    const StdVideoH264SequenceParameterSet* pStdSPSs,
    const uint32_t stdPPSCount,
    const StdVideoH264PictureParameterSet* pStdPPSs) const;
#endif
#ifdef VK_ENABLE_BETA_EXTENSIONS
bool ValidateVideoEncodeH264SessionParametersCreateInfoEXT(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const uint32_t maxStdSPSCount,
    const uint32_t maxStdPPSCount,
    const VkVideoEncodeH264SessionParametersAddInfoEXT* pParametersAddInfo) const;
#endif
#ifdef VK_ENABLE_BETA_EXTENSIONS
bool ValidateVideoEncodeH264NaluSliceInfoEXT(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const uint32_t mbCount,
    const StdVideoEncodeH264ReferenceListsInfo* pStdReferenceFinalLists,
    const StdVideoEncodeH264SliceHeader* pStdSliceHeader) const;
#endif
#ifdef VK_ENABLE_BETA_EXTENSIONS
bool ValidateVideoEncodeH264VclFrameInfoEXT(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const StdVideoEncodeH264ReferenceListsInfo* pStdReferenceFinalLists,
    const uint32_t naluSliceEntryCount,
    const VkVideoEncodeH264NaluSliceInfoEXT* pNaluSliceEntries,
    const StdVideoEncodeH264PictureInfo* pStdPictureInfo) const;
#endif
#ifdef VK_ENABLE_BETA_EXTENSIONS
bool ValidateVideoEncodeH264DpbSlotInfoEXT(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const StdVideoEncodeH264ReferenceInfo* pStdReferenceInfo) const;
#endif
#ifdef VK_ENABLE_BETA_EXTENSIONS
bool ValidateVideoEncodeH264ProfileInfoEXT(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const StdVideoH264ProfileIdc stdProfileIdc) const;
#endif
#ifdef VK_ENABLE_BETA_EXTENSIONS
bool ValidateVideoEncodeH264RateControlInfoEXT(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const uint32_t gopFrameCount,
    const uint32_t idrPeriod,
    const uint32_t consecutiveBFrameCount,
    const VkVideoEncodeH264RateControlStructureEXT rateControlStructure,
    const uint32_t temporalLayerCount) const;
#endif
#ifdef VK_ENABLE_BETA_EXTENSIONS
bool ValidateVideoEncodeH264QpEXT(const LogObjectList &_parentObjects,
    const int32_t qpI,
    const int32_t qpP,
    const int32_t qpB) const;
#endif
#ifdef VK_ENABLE_BETA_EXTENSIONS
bool ValidateVideoEncodeH264FrameSizeEXT(const LogObjectList &_parentObjects,
    const uint32_t frameISize,
    const uint32_t framePSize,
    const uint32_t frameBSize) const;
#endif
#ifdef VK_ENABLE_BETA_EXTENSIONS
bool ValidateVideoEncodeH264RateControlLayerInfoEXT(const LogObjectList &_parentObjects,
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
    const VkVideoEncodeH264FrameSizeEXT maxFrameSize) const;
#endif
#ifdef VK_ENABLE_BETA_EXTENSIONS
bool ValidateVideoEncodeH265SessionParametersAddInfoEXT(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const uint32_t stdVPSCount,
    const StdVideoH265VideoParameterSet* pStdVPSs,
    const uint32_t stdSPSCount,
    const StdVideoH265SequenceParameterSet* pStdSPSs,
    const uint32_t stdPPSCount,
    const StdVideoH265PictureParameterSet* pStdPPSs) const;
#endif
#ifdef VK_ENABLE_BETA_EXTENSIONS
bool ValidateVideoEncodeH265SessionParametersCreateInfoEXT(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const uint32_t maxStdVPSCount,
    const uint32_t maxStdSPSCount,
    const uint32_t maxStdPPSCount,
    const VkVideoEncodeH265SessionParametersAddInfoEXT* pParametersAddInfo) const;
#endif
#ifdef VK_ENABLE_BETA_EXTENSIONS
bool ValidateVideoEncodeH265NaluSliceSegmentInfoEXT(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const uint32_t ctbCount,
    const StdVideoEncodeH265ReferenceListsInfo* pStdReferenceFinalLists,
    const StdVideoEncodeH265SliceSegmentHeader* pStdSliceSegmentHeader) const;
#endif
#ifdef VK_ENABLE_BETA_EXTENSIONS
bool ValidateVideoEncodeH265VclFrameInfoEXT(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const StdVideoEncodeH265ReferenceListsInfo* pStdReferenceFinalLists,
    const uint32_t naluSliceSegmentEntryCount,
    const VkVideoEncodeH265NaluSliceSegmentInfoEXT* pNaluSliceSegmentEntries,
    const StdVideoEncodeH265PictureInfo* pStdPictureInfo) const;
#endif
#ifdef VK_ENABLE_BETA_EXTENSIONS
bool ValidateVideoEncodeH265DpbSlotInfoEXT(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const StdVideoEncodeH265ReferenceInfo* pStdReferenceInfo) const;
#endif
#ifdef VK_ENABLE_BETA_EXTENSIONS
bool ValidateVideoEncodeH265ProfileInfoEXT(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const StdVideoH265ProfileIdc stdProfileIdc) const;
#endif
#ifdef VK_ENABLE_BETA_EXTENSIONS
bool ValidateVideoEncodeH265RateControlInfoEXT(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const uint32_t gopFrameCount,
    const uint32_t idrPeriod,
    const uint32_t consecutiveBFrameCount,
    const VkVideoEncodeH265RateControlStructureEXT rateControlStructure,
    const uint32_t subLayerCount) const;
#endif
#ifdef VK_ENABLE_BETA_EXTENSIONS
bool ValidateVideoEncodeH265QpEXT(const LogObjectList &_parentObjects,
    const int32_t qpI,
    const int32_t qpP,
    const int32_t qpB) const;
#endif
#ifdef VK_ENABLE_BETA_EXTENSIONS
bool ValidateVideoEncodeH265FrameSizeEXT(const LogObjectList &_parentObjects,
    const uint32_t frameISize,
    const uint32_t framePSize,
    const uint32_t frameBSize) const;
#endif
#ifdef VK_ENABLE_BETA_EXTENSIONS
bool ValidateVideoEncodeH265RateControlLayerInfoEXT(const LogObjectList &_parentObjects,
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
    const VkVideoEncodeH265FrameSizeEXT maxFrameSize) const;
#endif
bool PreCallValidateGetShaderInfoAMD(VkDevice device,
    VkPipeline pipeline,
    VkShaderStageFlagBits shaderStage,
    VkShaderInfoTypeAMD infoType,
    size_t* pInfoSize,
    void* pInfo) const override;
#ifdef VK_USE_PLATFORM_GGP
bool ValidateStreamDescriptorSurfaceCreateInfoGGP(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const VkStreamDescriptorSurfaceCreateFlagsGGP flags,
    const GgpStreamDescriptor streamDescriptor) const;
#endif
#ifdef VK_USE_PLATFORM_GGP
bool PreCallValidateCreateStreamDescriptorSurfaceGGP(VkInstance instance,
    const VkStreamDescriptorSurfaceCreateInfoGGP* pCreateInfo,
    const VkAllocationCallbacks* pAllocator,
    VkSurfaceKHR* pSurface) const override;
#endif
bool ValidatePhysicalDeviceCornerSampledImageFeaturesNV(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const VkBool32 cornerSampledImage) const;
bool PreCallValidateGetPhysicalDeviceExternalImageFormatPropertiesNV(VkPhysicalDevice physicalDevice,
    VkFormat format,
    VkImageType type,
    VkImageTiling tiling,
    VkImageUsageFlags usage,
    VkImageCreateFlags flags,
    VkExternalMemoryHandleTypeFlagsNV externalHandleType,
    VkExternalImageFormatPropertiesNV* pExternalImageFormatProperties) const override;
bool ValidateExternalMemoryImageCreateInfoNV(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const VkExternalMemoryHandleTypeFlagsNV handleTypes) const;
bool ValidateExportMemoryAllocateInfoNV(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const VkExternalMemoryHandleTypeFlagsNV handleTypes) const;
#ifdef VK_USE_PLATFORM_WIN32_KHR
bool ValidateImportMemoryWin32HandleInfoNV(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const VkExternalMemoryHandleTypeFlagsNV handleType,
    const HANDLE handle) const;
#endif
#ifdef VK_USE_PLATFORM_WIN32_KHR
bool ValidateExportMemoryWin32HandleInfoNV(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const SECURITY_ATTRIBUTES* pAttributes,
    const DWORD dwAccess) const;
#endif
#ifdef VK_USE_PLATFORM_WIN32_KHR
bool PreCallValidateGetMemoryWin32HandleNV(VkDevice device,
    VkDeviceMemory memory,
    VkExternalMemoryHandleTypeFlagsNV handleType,
    HANDLE* pHandle) const override;
#endif
#ifdef VK_USE_PLATFORM_WIN32_KHR
bool ValidateWin32KeyedMutexAcquireReleaseInfoNV(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const uint32_t acquireCount,
    const VkDeviceMemory* pAcquireSyncs,
    const uint64_t* pAcquireKeys,
    const uint32_t* pAcquireTimeoutMilliseconds,
    const uint32_t releaseCount,
    const VkDeviceMemory* pReleaseSyncs,
    const uint64_t* pReleaseKeys) const;
#endif
bool ValidateValidationFlagsEXT(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const uint32_t disabledValidationCheckCount,
    const VkValidationCheckEXT* pDisabledValidationChecks) const;
#ifdef VK_USE_PLATFORM_VI_NN
bool ValidateViSurfaceCreateInfoNN(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const VkViSurfaceCreateFlagsNN flags,
    const void* window) const;
#endif
#ifdef VK_USE_PLATFORM_VI_NN
bool PreCallValidateCreateViSurfaceNN(VkInstance instance,
    const VkViSurfaceCreateInfoNN* pCreateInfo,
    const VkAllocationCallbacks* pAllocator,
    VkSurfaceKHR* pSurface) const override;
#endif
bool ValidatePhysicalDeviceTextureCompressionASTCHDRFeaturesEXT(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const VkBool32 textureCompressionASTC_HDR) const;
bool ValidateImageViewASTCDecodeModeEXT(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const VkFormat decodeMode) const;
bool ValidatePhysicalDeviceASTCDecodeFeaturesEXT(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const VkBool32 decodeModeSharedExponent) const;
bool ValidatePhysicalDevicePipelineRobustnessFeaturesEXT(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const VkBool32 pipelineRobustness) const;
bool ValidatePipelineRobustnessCreateInfoEXT(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const VkPipelineRobustnessBufferBehaviorEXT storageBuffers,
    const VkPipelineRobustnessBufferBehaviorEXT uniformBuffers,
    const VkPipelineRobustnessBufferBehaviorEXT vertexInputs,
    const VkPipelineRobustnessImageBehaviorEXT images) const;
bool ValidateConditionalRenderingBeginInfoEXT(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const VkBuffer buffer,
    const VkDeviceSize offset,
    const VkConditionalRenderingFlagsEXT flags) const;
bool ValidatePhysicalDeviceConditionalRenderingFeaturesEXT(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const VkBool32 conditionalRendering,
    const VkBool32 inheritedConditionalRendering) const;
bool ValidateCommandBufferInheritanceConditionalRenderingInfoEXT(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const VkBool32 conditionalRenderingEnable) const;
bool PreCallValidateCmdBeginConditionalRenderingEXT(VkCommandBuffer commandBuffer,
    const VkConditionalRenderingBeginInfoEXT* pConditionalRenderingBegin) const override;
bool PreCallValidateCmdEndConditionalRenderingEXT(VkCommandBuffer commandBuffer) const override;
bool ValidateViewportWScalingNV(const LogObjectList &_parentObjects,
    const float xcoeff,
    const float ycoeff) const;
bool ValidatePipelineViewportWScalingStateCreateInfoNV(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const VkBool32 viewportWScalingEnable,
    const uint32_t viewportCount,
    const VkViewportWScalingNV* pViewportWScalings) const;
bool PreCallValidateCmdSetViewportWScalingNV(VkCommandBuffer commandBuffer,
    uint32_t firstViewport,
    uint32_t viewportCount,
    const VkViewportWScalingNV* pViewportWScalings) const override;
bool PreCallValidateReleaseDisplayEXT(VkPhysicalDevice physicalDevice,
    VkDisplayKHR display) const override;
#ifdef VK_USE_PLATFORM_XLIB_XRANDR_EXT
bool PreCallValidateAcquireXlibDisplayEXT(VkPhysicalDevice physicalDevice,
    Display* dpy,
    VkDisplayKHR display) const override;
#endif
#ifdef VK_USE_PLATFORM_XLIB_XRANDR_EXT
bool PreCallValidateGetRandROutputDisplayEXT(VkPhysicalDevice physicalDevice,
    Display* dpy,
    RROutput rrOutput,
    VkDisplayKHR* pDisplay) const override;
#endif
bool PreCallValidateGetPhysicalDeviceSurfaceCapabilities2EXT(VkPhysicalDevice physicalDevice,
    VkSurfaceKHR surface,
    VkSurfaceCapabilities2EXT* pSurfaceCapabilities) const override;
bool ValidateDisplayPowerInfoEXT(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const VkDisplayPowerStateEXT powerState) const;
bool ValidateDeviceEventInfoEXT(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const VkDeviceEventTypeEXT deviceEvent) const;
bool ValidateDisplayEventInfoEXT(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const VkDisplayEventTypeEXT displayEvent) const;
bool ValidateSwapchainCounterCreateInfoEXT(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const VkSurfaceCounterFlagsEXT surfaceCounters) const;
bool PreCallValidateDisplayPowerControlEXT(VkDevice device,
    VkDisplayKHR display,
    const VkDisplayPowerInfoEXT* pDisplayPowerInfo) const override;
bool PreCallValidateRegisterDeviceEventEXT(VkDevice device,
    const VkDeviceEventInfoEXT* pDeviceEventInfo,
    const VkAllocationCallbacks* pAllocator,
    VkFence* pFence) const override;
bool PreCallValidateRegisterDisplayEventEXT(VkDevice device,
    VkDisplayKHR display,
    const VkDisplayEventInfoEXT* pDisplayEventInfo,
    const VkAllocationCallbacks* pAllocator,
    VkFence* pFence) const override;
bool PreCallValidateGetSwapchainCounterEXT(VkDevice device,
    VkSwapchainKHR swapchain,
    VkSurfaceCounterFlagBitsEXT counter,
    uint64_t* pCounterValue) const override;
bool ValidatePresentTimeGOOGLE(const LogObjectList &_parentObjects,
    const uint32_t presentID,
    const uint64_t desiredPresentTime) const;
bool ValidatePresentTimesInfoGOOGLE(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const uint32_t swapchainCount,
    const VkPresentTimeGOOGLE* pTimes) const;
bool PreCallValidateGetRefreshCycleDurationGOOGLE(VkDevice device,
    VkSwapchainKHR swapchain,
    VkRefreshCycleDurationGOOGLE* pDisplayTimingProperties) const override;
bool PreCallValidateGetPastPresentationTimingGOOGLE(VkDevice device,
    VkSwapchainKHR swapchain,
    uint32_t* pPresentationTimingCount,
    VkPastPresentationTimingGOOGLE* pPresentationTimings) const override;
bool ValidateViewportSwizzleNV(const LogObjectList &_parentObjects,
    const VkViewportCoordinateSwizzleNV x,
    const VkViewportCoordinateSwizzleNV y,
    const VkViewportCoordinateSwizzleNV z,
    const VkViewportCoordinateSwizzleNV w) const;
bool ValidatePipelineViewportSwizzleStateCreateInfoNV(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const VkPipelineViewportSwizzleStateCreateFlagsNV flags,
    const uint32_t viewportCount,
    const VkViewportSwizzleNV* pViewportSwizzles) const;
bool ValidatePipelineDiscardRectangleStateCreateInfoEXT(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const VkPipelineDiscardRectangleStateCreateFlagsEXT flags,
    const VkDiscardRectangleModeEXT discardRectangleMode,
    const uint32_t discardRectangleCount,
    const VkRect2D* pDiscardRectangles) const;
bool PreCallValidateCmdSetDiscardRectangleEXT(VkCommandBuffer commandBuffer,
    uint32_t firstDiscardRectangle,
    uint32_t discardRectangleCount,
    const VkRect2D* pDiscardRectangles) const override;
bool PreCallValidateCmdSetDiscardRectangleEnableEXT(VkCommandBuffer commandBuffer,
    VkBool32 discardRectangleEnable) const override;
bool PreCallValidateCmdSetDiscardRectangleModeEXT(VkCommandBuffer commandBuffer,
    VkDiscardRectangleModeEXT discardRectangleMode) const override;
bool ValidatePipelineRasterizationConservativeStateCreateInfoEXT(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const VkPipelineRasterizationConservativeStateCreateFlagsEXT flags,
    const VkConservativeRasterizationModeEXT conservativeRasterizationMode,
    const float extraPrimitiveOverestimationSize) const;
bool ValidatePhysicalDeviceDepthClipEnableFeaturesEXT(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const VkBool32 depthClipEnable) const;
bool ValidatePipelineRasterizationDepthClipStateCreateInfoEXT(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const VkPipelineRasterizationDepthClipStateCreateFlagsEXT flags,
    const VkBool32 depthClipEnable) const;
bool ValidateXYColorEXT(const LogObjectList &_parentObjects,
    const float x,
    const float y) const;
bool ValidateHdrMetadataEXT(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const VkXYColorEXT displayPrimaryRed,
    const VkXYColorEXT displayPrimaryGreen,
    const VkXYColorEXT displayPrimaryBlue,
    const VkXYColorEXT whitePoint,
    const float maxLuminance,
    const float minLuminance,
    const float maxContentLightLevel,
    const float maxFrameAverageLightLevel) const;
bool PreCallValidateSetHdrMetadataEXT(VkDevice device,
    uint32_t swapchainCount,
    const VkSwapchainKHR* pSwapchains,
    const VkHdrMetadataEXT* pMetadata) const override;
#ifdef VK_USE_PLATFORM_IOS_MVK
bool ValidateIOSSurfaceCreateInfoMVK(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const VkIOSSurfaceCreateFlagsMVK flags,
    const void* pView) const;
#endif
#ifdef VK_USE_PLATFORM_IOS_MVK
bool PreCallValidateCreateIOSSurfaceMVK(VkInstance instance,
    const VkIOSSurfaceCreateInfoMVK* pCreateInfo,
    const VkAllocationCallbacks* pAllocator,
    VkSurfaceKHR* pSurface) const override;
#endif
#ifdef VK_USE_PLATFORM_MACOS_MVK
bool ValidateMacOSSurfaceCreateInfoMVK(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const VkMacOSSurfaceCreateFlagsMVK flags,
    const void* pView) const;
#endif
#ifdef VK_USE_PLATFORM_MACOS_MVK
bool PreCallValidateCreateMacOSSurfaceMVK(VkInstance instance,
    const VkMacOSSurfaceCreateInfoMVK* pCreateInfo,
    const VkAllocationCallbacks* pAllocator,
    VkSurfaceKHR* pSurface) const override;
#endif
bool ValidateDebugUtilsLabelEXT(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const char* pLabelName,
    const float color[4]) const;
bool ValidateDebugUtilsObjectNameInfoEXT(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const VkObjectType objectType,
    const uint64_t objectHandle,
    const char* pObjectName) const;
bool ValidateDebugUtilsMessengerCallbackDataEXT(const LogObjectList &_parentObjects,
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
    const VkDebugUtilsObjectNameInfoEXT* pObjects) const;
bool ValidateDebugUtilsMessengerCreateInfoEXT(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const VkDebugUtilsMessengerCreateFlagsEXT flags,
    const VkDebugUtilsMessageSeverityFlagsEXT messageSeverity,
    const VkDebugUtilsMessageTypeFlagsEXT messageType,
    const PFN_vkDebugUtilsMessengerCallbackEXT pfnUserCallback,
    const void* pUserData) const;
bool ValidateDebugUtilsObjectTagInfoEXT(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const VkObjectType objectType,
    const uint64_t objectHandle,
    const uint64_t tagName,
    const size_t tagSize,
    const void* pTag) const;
bool PreCallValidateSetDebugUtilsObjectNameEXT(VkDevice device,
    const VkDebugUtilsObjectNameInfoEXT* pNameInfo) const override;
bool PreCallValidateSetDebugUtilsObjectTagEXT(VkDevice device,
    const VkDebugUtilsObjectTagInfoEXT* pTagInfo) const override;
bool PreCallValidateQueueBeginDebugUtilsLabelEXT(VkQueue queue,
    const VkDebugUtilsLabelEXT* pLabelInfo) const override;
bool PreCallValidateQueueEndDebugUtilsLabelEXT(VkQueue queue) const override;
bool PreCallValidateQueueInsertDebugUtilsLabelEXT(VkQueue queue,
    const VkDebugUtilsLabelEXT* pLabelInfo) const override;
bool PreCallValidateCmdBeginDebugUtilsLabelEXT(VkCommandBuffer commandBuffer,
    const VkDebugUtilsLabelEXT* pLabelInfo) const override;
bool PreCallValidateCmdEndDebugUtilsLabelEXT(VkCommandBuffer commandBuffer) const override;
bool PreCallValidateCmdInsertDebugUtilsLabelEXT(VkCommandBuffer commandBuffer,
    const VkDebugUtilsLabelEXT* pLabelInfo) const override;
bool PreCallValidateCreateDebugUtilsMessengerEXT(VkInstance instance,
    const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
    const VkAllocationCallbacks* pAllocator,
    VkDebugUtilsMessengerEXT* pMessenger) const override;
bool PreCallValidateDestroyDebugUtilsMessengerEXT(VkInstance instance,
    VkDebugUtilsMessengerEXT messenger,
    const VkAllocationCallbacks* pAllocator) const override;
bool PreCallValidateSubmitDebugUtilsMessageEXT(VkInstance instance,
    VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
    VkDebugUtilsMessageTypeFlagsEXT messageTypes,
    const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData) const override;
#ifdef VK_USE_PLATFORM_ANDROID_KHR
bool ValidateImportAndroidHardwareBufferInfoANDROID(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const struct AHardwareBuffer* buffer) const;
#endif
#ifdef VK_USE_PLATFORM_ANDROID_KHR
bool ValidateMemoryGetAndroidHardwareBufferInfoANDROID(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const VkDeviceMemory memory) const;
#endif
#ifdef VK_USE_PLATFORM_ANDROID_KHR
bool ValidateExternalFormatANDROID(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const uint64_t externalFormat) const;
#endif
#ifdef VK_USE_PLATFORM_ANDROID_KHR
bool PreCallValidateGetAndroidHardwareBufferPropertiesANDROID(VkDevice device,
    const struct AHardwareBuffer* buffer,
    VkAndroidHardwareBufferPropertiesANDROID* pProperties) const override;
#endif
#ifdef VK_USE_PLATFORM_ANDROID_KHR
bool PreCallValidateGetMemoryAndroidHardwareBufferANDROID(VkDevice device,
    const VkMemoryGetAndroidHardwareBufferInfoANDROID* pInfo,
    struct AHardwareBuffer** pBuffer) const override;
#endif
bool ValidateSamplerReductionModeCreateInfoEXT(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const VkSamplerReductionMode reductionMode) const;
bool ValidatePhysicalDeviceInlineUniformBlockFeaturesEXT(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const VkBool32 inlineUniformBlock,
    const VkBool32 descriptorBindingInlineUniformBlockUpdateAfterBind) const;
bool ValidateWriteDescriptorSetInlineUniformBlockEXT(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const uint32_t dataSize,
    const void* pData) const;
bool ValidateDescriptorPoolInlineUniformBlockCreateInfoEXT(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const uint32_t maxInlineUniformBlockBindings) const;
bool ValidateSampleLocationEXT(const LogObjectList &_parentObjects,
    const float x,
    const float y) const;
bool ValidateSampleLocationsInfoEXT(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const VkSampleCountFlagBits sampleLocationsPerPixel,
    const VkExtent2D sampleLocationGridSize,
    const uint32_t sampleLocationsCount,
    const VkSampleLocationEXT* pSampleLocations) const;
bool ValidateAttachmentSampleLocationsEXT(const LogObjectList &_parentObjects,
    const uint32_t attachmentIndex,
    const VkSampleLocationsInfoEXT sampleLocationsInfo) const;
bool ValidateSubpassSampleLocationsEXT(const LogObjectList &_parentObjects,
    const uint32_t subpassIndex,
    const VkSampleLocationsInfoEXT sampleLocationsInfo) const;
bool ValidateRenderPassSampleLocationsBeginInfoEXT(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const uint32_t attachmentInitialSampleLocationsCount,
    const VkAttachmentSampleLocationsEXT* pAttachmentInitialSampleLocations,
    const uint32_t postSubpassSampleLocationsCount,
    const VkSubpassSampleLocationsEXT* pPostSubpassSampleLocations) const;
bool ValidatePipelineSampleLocationsStateCreateInfoEXT(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const VkBool32 sampleLocationsEnable,
    const VkSampleLocationsInfoEXT sampleLocationsInfo) const;
bool PreCallValidateCmdSetSampleLocationsEXT(VkCommandBuffer commandBuffer,
    const VkSampleLocationsInfoEXT* pSampleLocationsInfo) const override;
bool PreCallValidateGetPhysicalDeviceMultisamplePropertiesEXT(VkPhysicalDevice physicalDevice,
    VkSampleCountFlagBits samples,
    VkMultisamplePropertiesEXT* pMultisampleProperties) const override;
bool ValidatePhysicalDeviceBlendOperationAdvancedFeaturesEXT(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const VkBool32 advancedBlendCoherentOperations) const;
bool ValidatePipelineColorBlendAdvancedStateCreateInfoEXT(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const VkBool32 srcPremultiplied,
    const VkBool32 dstPremultiplied,
    const VkBlendOverlapEXT blendOverlap) const;
bool ValidatePipelineCoverageToColorStateCreateInfoNV(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const VkPipelineCoverageToColorStateCreateFlagsNV flags,
    const VkBool32 coverageToColorEnable,
    const uint32_t coverageToColorLocation) const;
bool ValidatePipelineCoverageModulationStateCreateInfoNV(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const VkPipelineCoverageModulationStateCreateFlagsNV flags,
    const VkCoverageModulationModeNV coverageModulationMode,
    const VkBool32 coverageModulationTableEnable,
    const uint32_t coverageModulationTableCount,
    const float* pCoverageModulationTable) const;
bool ValidatePhysicalDeviceShaderSMBuiltinsFeaturesNV(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const VkBool32 shaderSMBuiltins) const;
bool ValidatePhysicalDeviceImageDrmFormatModifierInfoEXT(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const uint64_t drmFormatModifier,
    const VkSharingMode sharingMode,
    const uint32_t queueFamilyIndexCount,
    const uint32_t* pQueueFamilyIndices) const;
bool ValidateImageDrmFormatModifierListCreateInfoEXT(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const uint32_t drmFormatModifierCount,
    const uint64_t* pDrmFormatModifiers) const;
bool ValidateImageDrmFormatModifierExplicitCreateInfoEXT(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const uint64_t drmFormatModifier,
    const uint32_t drmFormatModifierPlaneCount,
    const VkSubresourceLayout* pPlaneLayouts) const;
bool PreCallValidateGetImageDrmFormatModifierPropertiesEXT(VkDevice device,
    VkImage image,
    VkImageDrmFormatModifierPropertiesEXT* pProperties) const override;
bool ValidateValidationCacheCreateInfoEXT(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const VkValidationCacheCreateFlagsEXT flags,
    const size_t initialDataSize,
    const void* pInitialData) const;
bool ValidateShaderModuleValidationCacheCreateInfoEXT(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const VkValidationCacheEXT validationCache) const;
bool ValidateDescriptorSetLayoutBindingFlagsCreateInfoEXT(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const uint32_t bindingCount,
    const VkDescriptorBindingFlags* pBindingFlags) const;
bool ValidatePhysicalDeviceDescriptorIndexingFeaturesEXT(const LogObjectList &_parentObjects,
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
    const VkBool32 runtimeDescriptorArray) const;
bool ValidateDescriptorSetVariableDescriptorCountAllocateInfoEXT(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const uint32_t descriptorSetCount,
    const uint32_t* pDescriptorCounts) const;
bool ValidateShadingRatePaletteNV(const LogObjectList &_parentObjects,
    const uint32_t shadingRatePaletteEntryCount,
    const VkShadingRatePaletteEntryNV* pShadingRatePaletteEntries) const;
bool ValidatePipelineViewportShadingRateImageStateCreateInfoNV(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const VkBool32 shadingRateImageEnable,
    const uint32_t viewportCount,
    const VkShadingRatePaletteNV* pShadingRatePalettes) const;
bool ValidatePhysicalDeviceShadingRateImageFeaturesNV(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const VkBool32 shadingRateImage,
    const VkBool32 shadingRateCoarseSampleOrder) const;
bool ValidateCoarseSampleLocationNV(const LogObjectList &_parentObjects,
    const uint32_t pixelX,
    const uint32_t pixelY,
    const uint32_t sample) const;
bool ValidateCoarseSampleOrderCustomNV(const LogObjectList &_parentObjects,
    const VkShadingRatePaletteEntryNV shadingRate,
    const uint32_t sampleCount,
    const uint32_t sampleLocationCount,
    const VkCoarseSampleLocationNV* pSampleLocations) const;
bool ValidatePipelineViewportCoarseSampleOrderStateCreateInfoNV(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const VkCoarseSampleOrderTypeNV sampleOrderType,
    const uint32_t customSampleOrderCount,
    const VkCoarseSampleOrderCustomNV* pCustomSampleOrders) const;
bool PreCallValidateCmdBindShadingRateImageNV(VkCommandBuffer commandBuffer,
    VkImageView imageView,
    VkImageLayout imageLayout) const override;
bool PreCallValidateCmdSetViewportShadingRatePaletteNV(VkCommandBuffer commandBuffer,
    uint32_t firstViewport,
    uint32_t viewportCount,
    const VkShadingRatePaletteNV* pShadingRatePalettes) const override;
bool PreCallValidateCmdSetCoarseSampleOrderNV(VkCommandBuffer commandBuffer,
    VkCoarseSampleOrderTypeNV sampleOrderType,
    uint32_t customSampleOrderCount,
    const VkCoarseSampleOrderCustomNV* pCustomSampleOrders) const override;
bool ValidateRayTracingShaderGroupCreateInfoNV(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const VkRayTracingShaderGroupTypeKHR type,
    const uint32_t generalShader,
    const uint32_t closestHitShader,
    const uint32_t anyHitShader,
    const uint32_t intersectionShader) const;
bool ValidateRayTracingPipelineCreateInfoNV(const LogObjectList &_parentObjects,
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
    const int32_t basePipelineIndex) const;
bool ValidateGeometryTrianglesNV(const LogObjectList &_parentObjects,
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
    const VkDeviceSize transformOffset) const;
bool ValidateGeometryAABBNV(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const VkBuffer aabbData,
    const uint32_t numAABBs,
    const uint32_t stride,
    const VkDeviceSize offset) const;
bool ValidateGeometryDataNV(const LogObjectList &_parentObjects,
    const VkGeometryTrianglesNV triangles,
    const VkGeometryAABBNV aabbs) const;
bool ValidateGeometryNV(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const VkGeometryTypeKHR geometryType,
    const VkGeometryDataNV geometry,
    const VkGeometryFlagsKHR flags) const;
bool ValidateAccelerationStructureInfoNV(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const VkAccelerationStructureTypeNV type,
    const VkBuildAccelerationStructureFlagsNV flags,
    const uint32_t instanceCount,
    const uint32_t geometryCount,
    const VkGeometryNV* pGeometries) const;
bool ValidateAccelerationStructureCreateInfoNV(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const VkDeviceSize compactedSize,
    const VkAccelerationStructureInfoNV info) const;
bool ValidateBindAccelerationStructureMemoryInfoNV(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const VkAccelerationStructureNV accelerationStructure,
    const VkDeviceMemory memory,
    const VkDeviceSize memoryOffset,
    const uint32_t deviceIndexCount,
    const uint32_t* pDeviceIndices) const;
bool ValidateWriteDescriptorSetAccelerationStructureNV(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const uint32_t accelerationStructureCount,
    const VkAccelerationStructureNV* pAccelerationStructures) const;
bool ValidateAccelerationStructureMemoryRequirementsInfoNV(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const VkAccelerationStructureMemoryRequirementsTypeNV type,
    const VkAccelerationStructureNV accelerationStructure) const;
bool ValidateTransformMatrixKHR(const LogObjectList &_parentObjects,
    const float matrix[3][4]) const;
bool ValidateTransformMatrixNV(const LogObjectList &_parentObjects,
    const float matrix[3][4]) const;
bool ValidateAabbPositionsKHR(const LogObjectList &_parentObjects,
    const float minX,
    const float minY,
    const float minZ,
    const float maxX,
    const float maxY,
    const float maxZ) const;
bool ValidateAabbPositionsNV(const LogObjectList &_parentObjects,
    const float minX,
    const float minY,
    const float minZ,
    const float maxX,
    const float maxY,
    const float maxZ) const;
bool ValidateAccelerationStructureInstanceKHR(const LogObjectList &_parentObjects,
    const VkTransformMatrixKHR transform,
    const uint32_t instanceCustomIndex,
    const uint32_t mask,
    const uint32_t instanceShaderBindingTableRecordOffset,
    const VkGeometryInstanceFlagsKHR flags,
    const uint64_t accelerationStructureReference) const;
bool ValidateAccelerationStructureInstanceNV(const LogObjectList &_parentObjects,
    const VkTransformMatrixKHR transform,
    const uint32_t instanceCustomIndex,
    const uint32_t mask,
    const uint32_t instanceShaderBindingTableRecordOffset,
    const VkGeometryInstanceFlagsKHR flags,
    const uint64_t accelerationStructureReference) const;
bool PreCallValidateCreateAccelerationStructureNV(VkDevice device,
    const VkAccelerationStructureCreateInfoNV* pCreateInfo,
    const VkAllocationCallbacks* pAllocator,
    VkAccelerationStructureNV* pAccelerationStructure) const override;
bool PreCallValidateDestroyAccelerationStructureNV(VkDevice device,
    VkAccelerationStructureNV accelerationStructure,
    const VkAllocationCallbacks* pAllocator) const override;
bool PreCallValidateGetAccelerationStructureMemoryRequirementsNV(VkDevice device,
    const VkAccelerationStructureMemoryRequirementsInfoNV* pInfo,
    VkMemoryRequirements2KHR* pMemoryRequirements) const override;
bool PreCallValidateBindAccelerationStructureMemoryNV(VkDevice device,
    uint32_t bindInfoCount,
    const VkBindAccelerationStructureMemoryInfoNV* pBindInfos) const override;
bool PreCallValidateCmdBuildAccelerationStructureNV(VkCommandBuffer commandBuffer,
    const VkAccelerationStructureInfoNV* pInfo,
    VkBuffer instanceData,
    VkDeviceSize instanceOffset,
    VkBool32 update,
    VkAccelerationStructureNV dst,
    VkAccelerationStructureNV src,
    VkBuffer scratch,
    VkDeviceSize scratchOffset) const override;
bool PreCallValidateCmdCopyAccelerationStructureNV(VkCommandBuffer commandBuffer,
    VkAccelerationStructureNV dst,
    VkAccelerationStructureNV src,
    VkCopyAccelerationStructureModeKHR mode) const override;
bool PreCallValidateCmdTraceRaysNV(VkCommandBuffer commandBuffer,
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
    uint32_t depth) const override;
bool PreCallValidateCreateRayTracingPipelinesNV(VkDevice device,
    VkPipelineCache pipelineCache,
    uint32_t createInfoCount,
    const VkRayTracingPipelineCreateInfoNV* pCreateInfos,
    const VkAllocationCallbacks* pAllocator,
    VkPipeline* pPipelines,
    void *validation_state) const override;
bool PreCallValidateGetRayTracingShaderGroupHandlesKHR(VkDevice device,
    VkPipeline pipeline,
    uint32_t firstGroup,
    uint32_t groupCount,
    size_t dataSize,
    void* pData) const override;
bool PreCallValidateGetRayTracingShaderGroupHandlesNV(VkDevice device,
    VkPipeline pipeline,
    uint32_t firstGroup,
    uint32_t groupCount,
    size_t dataSize,
    void* pData) const override;
bool PreCallValidateGetAccelerationStructureHandleNV(VkDevice device,
    VkAccelerationStructureNV accelerationStructure,
    size_t dataSize,
    void* pData) const override;
bool PreCallValidateCmdWriteAccelerationStructuresPropertiesNV(VkCommandBuffer commandBuffer,
    uint32_t accelerationStructureCount,
    const VkAccelerationStructureNV* pAccelerationStructures,
    VkQueryType queryType,
    VkQueryPool queryPool,
    uint32_t firstQuery) const override;
bool PreCallValidateCompileDeferredNV(VkDevice device,
    VkPipeline pipeline,
    uint32_t shader) const override;
bool ValidatePhysicalDeviceRepresentativeFragmentTestFeaturesNV(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const VkBool32 representativeFragmentTest) const;
bool ValidatePipelineRepresentativeFragmentTestStateCreateInfoNV(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const VkBool32 representativeFragmentTestEnable) const;
bool ValidatePhysicalDeviceImageViewImageFormatInfoEXT(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const VkImageViewType imageViewType) const;
bool ValidateDeviceQueueGlobalPriorityCreateInfoEXT(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const VkQueueGlobalPriorityKHR globalPriority) const;
bool ValidateImportMemoryHostPointerInfoEXT(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const VkExternalMemoryHandleTypeFlagBits handleType,
    const void* pHostPointer) const;
bool PreCallValidateGetMemoryHostPointerPropertiesEXT(VkDevice device,
    VkExternalMemoryHandleTypeFlagBits handleType,
    const void* pHostPointer,
    VkMemoryHostPointerPropertiesEXT* pMemoryHostPointerProperties) const override;
bool PreCallValidateCmdWriteBufferMarkerAMD(VkCommandBuffer commandBuffer,
    VkPipelineStageFlagBits pipelineStage,
    VkBuffer dstBuffer,
    VkDeviceSize dstOffset,
    uint32_t marker) const override;
bool ValidatePipelineCompilerControlCreateInfoAMD(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const VkPipelineCompilerControlFlagsAMD compilerControlFlags) const;
bool ValidateCalibratedTimestampInfoEXT(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const VkTimeDomainEXT timeDomain) const;
bool PreCallValidateGetPhysicalDeviceCalibrateableTimeDomainsEXT(VkPhysicalDevice physicalDevice,
    uint32_t* pTimeDomainCount,
    VkTimeDomainEXT* pTimeDomains) const override;
bool PreCallValidateGetCalibratedTimestampsEXT(VkDevice device,
    uint32_t timestampCount,
    const VkCalibratedTimestampInfoEXT* pTimestampInfos,
    uint64_t* pTimestamps,
    uint64_t* pMaxDeviation) const override;
bool ValidateDeviceMemoryOverallocationCreateInfoAMD(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const VkMemoryOverallocationBehaviorAMD overallocationBehavior) const;
bool ValidateVertexInputBindingDivisorDescriptionEXT(const LogObjectList &_parentObjects,
    const uint32_t binding,
    const uint32_t divisor) const;
bool ValidatePipelineVertexInputDivisorStateCreateInfoEXT(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const uint32_t vertexBindingDivisorCount,
    const VkVertexInputBindingDivisorDescriptionEXT* pVertexBindingDivisors) const;
bool ValidatePhysicalDeviceVertexAttributeDivisorFeaturesEXT(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const VkBool32 vertexAttributeInstanceRateDivisor,
    const VkBool32 vertexAttributeInstanceRateZeroDivisor) const;
#ifdef VK_USE_PLATFORM_GGP
bool ValidatePresentFrameTokenGGP(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const GgpFrameToken frameToken) const;
#endif
bool ValidatePipelineCreationFeedbackCreateInfoEXT(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const VkPipelineCreationFeedback* pPipelineCreationFeedback,
    const uint32_t pipelineStageCreationFeedbackCount,
    const VkPipelineCreationFeedback* pPipelineStageCreationFeedbacks) const;
bool ValidatePhysicalDeviceComputeShaderDerivativesFeaturesNV(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const VkBool32 computeDerivativeGroupQuads,
    const VkBool32 computeDerivativeGroupLinear) const;
bool ValidatePhysicalDeviceMeshShaderFeaturesNV(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const VkBool32 taskShader,
    const VkBool32 meshShader) const;
bool ValidateDrawMeshTasksIndirectCommandNV(const LogObjectList &_parentObjects,
    const uint32_t taskCount,
    const uint32_t firstTask) const;
bool PreCallValidateCmdDrawMeshTasksNV(VkCommandBuffer commandBuffer,
    uint32_t taskCount,
    uint32_t firstTask) const override;
bool PreCallValidateCmdDrawMeshTasksIndirectNV(VkCommandBuffer commandBuffer,
    VkBuffer buffer,
    VkDeviceSize offset,
    uint32_t drawCount,
    uint32_t stride) const override;
bool PreCallValidateCmdDrawMeshTasksIndirectCountNV(VkCommandBuffer commandBuffer,
    VkBuffer buffer,
    VkDeviceSize offset,
    VkBuffer countBuffer,
    VkDeviceSize countBufferOffset,
    uint32_t maxDrawCount,
    uint32_t stride) const override;
bool ValidatePhysicalDeviceFragmentShaderBarycentricFeaturesNV(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const VkBool32 fragmentShaderBarycentric) const;
bool ValidatePhysicalDeviceShaderImageFootprintFeaturesNV(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const VkBool32 imageFootprint) const;
bool ValidatePipelineViewportExclusiveScissorStateCreateInfoNV(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const uint32_t exclusiveScissorCount,
    const VkRect2D* pExclusiveScissors) const;
bool ValidatePhysicalDeviceExclusiveScissorFeaturesNV(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const VkBool32 exclusiveScissor) const;
bool PreCallValidateCmdSetExclusiveScissorEnableNV(VkCommandBuffer commandBuffer,
    uint32_t firstExclusiveScissor,
    uint32_t exclusiveScissorCount,
    const VkBool32* pExclusiveScissorEnables) const override;
bool PreCallValidateCmdSetExclusiveScissorNV(VkCommandBuffer commandBuffer,
    uint32_t firstExclusiveScissor,
    uint32_t exclusiveScissorCount,
    const VkRect2D* pExclusiveScissors) const override;
bool PreCallValidateCmdSetCheckpointNV(VkCommandBuffer commandBuffer,
    const void* pCheckpointMarker) const override;
bool PreCallValidateGetQueueCheckpointDataNV(VkQueue queue,
    uint32_t* pCheckpointDataCount,
    VkCheckpointDataNV* pCheckpointData) const override;
bool ValidatePhysicalDeviceShaderIntegerFunctions2FeaturesINTEL(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const VkBool32 shaderIntegerFunctions2) const;
bool ValidatePerformanceValueDataINTEL(const LogObjectList &_parentObjects,
    const uint32_t value32,
    const uint64_t value64,
    const float valueFloat,
    const VkBool32 valueBool,
    const char* valueString) const;
bool ValidatePerformanceValueINTEL(const LogObjectList &_parentObjects,
    const VkPerformanceValueTypeINTEL type,
    const VkPerformanceValueDataINTEL data) const;
bool ValidateInitializePerformanceApiInfoINTEL(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const void* pUserData) const;
bool ValidateQueryPoolPerformanceQueryCreateInfoINTEL(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const VkQueryPoolSamplingModeINTEL performanceCountersSampling) const;
bool ValidateQueryPoolCreateInfoINTEL(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const VkQueryPoolSamplingModeINTEL performanceCountersSampling) const;
bool ValidatePerformanceMarkerInfoINTEL(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const uint64_t marker) const;
bool ValidatePerformanceStreamMarkerInfoINTEL(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const uint32_t marker) const;
bool ValidatePerformanceOverrideInfoINTEL(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const VkPerformanceOverrideTypeINTEL type,
    const VkBool32 enable,
    const uint64_t parameter) const;
bool ValidatePerformanceConfigurationAcquireInfoINTEL(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const VkPerformanceConfigurationTypeINTEL type) const;
bool PreCallValidateInitializePerformanceApiINTEL(VkDevice device,
    const VkInitializePerformanceApiInfoINTEL* pInitializeInfo) const override;
bool PreCallValidateUninitializePerformanceApiINTEL(VkDevice device) const override;
bool PreCallValidateCmdSetPerformanceMarkerINTEL(VkCommandBuffer commandBuffer,
    const VkPerformanceMarkerInfoINTEL* pMarkerInfo) const override;
bool PreCallValidateCmdSetPerformanceStreamMarkerINTEL(VkCommandBuffer commandBuffer,
    const VkPerformanceStreamMarkerInfoINTEL* pMarkerInfo) const override;
bool PreCallValidateCmdSetPerformanceOverrideINTEL(VkCommandBuffer commandBuffer,
    const VkPerformanceOverrideInfoINTEL* pOverrideInfo) const override;
bool PreCallValidateAcquirePerformanceConfigurationINTEL(VkDevice device,
    const VkPerformanceConfigurationAcquireInfoINTEL* pAcquireInfo,
    VkPerformanceConfigurationINTEL* pConfiguration) const override;
bool PreCallValidateReleasePerformanceConfigurationINTEL(VkDevice device,
    VkPerformanceConfigurationINTEL configuration) const override;
bool PreCallValidateQueueSetPerformanceConfigurationINTEL(VkQueue queue,
    VkPerformanceConfigurationINTEL configuration) const override;
bool PreCallValidateGetPerformanceParameterINTEL(VkDevice device,
    VkPerformanceParameterTypeINTEL parameter,
    VkPerformanceValueINTEL* pValue) const override;
bool ValidateSwapchainDisplayNativeHdrCreateInfoAMD(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const VkBool32 localDimmingEnable) const;
bool PreCallValidateSetLocalDimmingAMD(VkDevice device,
    VkSwapchainKHR swapChain,
    VkBool32 localDimmingEnable) const override;
#ifdef VK_USE_PLATFORM_FUCHSIA
bool ValidateImagePipeSurfaceCreateInfoFUCHSIA(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const VkImagePipeSurfaceCreateFlagsFUCHSIA flags,
    const zx_handle_t imagePipeHandle) const;
#endif
#ifdef VK_USE_PLATFORM_FUCHSIA
bool PreCallValidateCreateImagePipeSurfaceFUCHSIA(VkInstance instance,
    const VkImagePipeSurfaceCreateInfoFUCHSIA* pCreateInfo,
    const VkAllocationCallbacks* pAllocator,
    VkSurfaceKHR* pSurface) const override;
#endif
#ifdef VK_USE_PLATFORM_METAL_EXT
bool ValidateMetalSurfaceCreateInfoEXT(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const VkMetalSurfaceCreateFlagsEXT flags,
    const CAMetalLayer* pLayer) const;
#endif
#ifdef VK_USE_PLATFORM_METAL_EXT
bool PreCallValidateCreateMetalSurfaceEXT(VkInstance instance,
    const VkMetalSurfaceCreateInfoEXT* pCreateInfo,
    const VkAllocationCallbacks* pAllocator,
    VkSurfaceKHR* pSurface) const override;
#endif
bool ValidatePhysicalDeviceFragmentDensityMapFeaturesEXT(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const VkBool32 fragmentDensityMap,
    const VkBool32 fragmentDensityMapDynamic,
    const VkBool32 fragmentDensityMapNonSubsampledImages) const;
bool ValidateRenderPassFragmentDensityMapCreateInfoEXT(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const VkAttachmentReference fragmentDensityMapAttachment) const;
bool ValidatePhysicalDeviceScalarBlockLayoutFeaturesEXT(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const VkBool32 scalarBlockLayout) const;
bool ValidatePhysicalDeviceSubgroupSizeControlFeaturesEXT(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const VkBool32 subgroupSizeControl,
    const VkBool32 computeFullSubgroups) const;
bool ValidatePhysicalDeviceCoherentMemoryFeaturesAMD(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const VkBool32 deviceCoherentMemory) const;
bool ValidatePhysicalDeviceShaderImageAtomicInt64FeaturesEXT(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const VkBool32 shaderImageInt64Atomics,
    const VkBool32 sparseImageInt64Atomics) const;
bool ValidatePhysicalDeviceMemoryPriorityFeaturesEXT(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const VkBool32 memoryPriority) const;
bool ValidateMemoryPriorityAllocateInfoEXT(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const float priority) const;
bool ValidatePhysicalDeviceDedicatedAllocationImageAliasingFeaturesNV(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const VkBool32 dedicatedAllocationImageAliasing) const;
bool ValidatePhysicalDeviceBufferDeviceAddressFeaturesEXT(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const VkBool32 bufferDeviceAddress,
    const VkBool32 bufferDeviceAddressCaptureReplay,
    const VkBool32 bufferDeviceAddressMultiDevice) const;
bool ValidatePhysicalDeviceBufferAddressFeaturesEXT(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const VkBool32 bufferDeviceAddress,
    const VkBool32 bufferDeviceAddressCaptureReplay,
    const VkBool32 bufferDeviceAddressMultiDevice) const;
bool ValidateBufferDeviceAddressInfoEXT(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const VkBuffer buffer) const;
bool ValidateBufferDeviceAddressCreateInfoEXT(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const VkDeviceAddress deviceAddress) const;
bool PreCallValidateGetBufferDeviceAddressEXT(VkDevice device,
    const VkBufferDeviceAddressInfo* pInfo) const override;
bool PreCallValidateGetPhysicalDeviceToolPropertiesEXT(VkPhysicalDevice physicalDevice,
    uint32_t* pToolCount,
    VkPhysicalDeviceToolProperties* pToolProperties) const override;
bool ValidateImageStencilUsageCreateInfoEXT(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const VkImageUsageFlags stencilUsage) const;
bool ValidateValidationFeaturesEXT(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const uint32_t enabledValidationFeatureCount,
    const VkValidationFeatureEnableEXT* pEnabledValidationFeatures,
    const uint32_t disabledValidationFeatureCount,
    const VkValidationFeatureDisableEXT* pDisabledValidationFeatures) const;
bool ValidateCooperativeMatrixPropertiesNV(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const uint32_t MSize,
    const uint32_t NSize,
    const uint32_t KSize,
    const VkComponentTypeNV AType,
    const VkComponentTypeNV BType,
    const VkComponentTypeNV CType,
    const VkComponentTypeNV DType,
    const VkScopeNV scope) const;
bool ValidatePhysicalDeviceCooperativeMatrixFeaturesNV(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const VkBool32 cooperativeMatrix,
    const VkBool32 cooperativeMatrixRobustBufferAccess) const;
bool PreCallValidateGetPhysicalDeviceCooperativeMatrixPropertiesNV(VkPhysicalDevice physicalDevice,
    uint32_t* pPropertyCount,
    VkCooperativeMatrixPropertiesNV* pProperties) const override;
bool ValidatePhysicalDeviceCoverageReductionModeFeaturesNV(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const VkBool32 coverageReductionMode) const;
bool ValidatePipelineCoverageReductionStateCreateInfoNV(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const VkPipelineCoverageReductionStateCreateFlagsNV flags,
    const VkCoverageReductionModeNV coverageReductionMode) const;
bool PreCallValidateGetPhysicalDeviceSupportedFramebufferMixedSamplesCombinationsNV(VkPhysicalDevice physicalDevice,
    uint32_t* pCombinationCount,
    VkFramebufferMixedSamplesCombinationNV* pCombinations) const override;
bool ValidatePhysicalDeviceFragmentShaderInterlockFeaturesEXT(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const VkBool32 fragmentShaderSampleInterlock,
    const VkBool32 fragmentShaderPixelInterlock,
    const VkBool32 fragmentShaderShadingRateInterlock) const;
bool ValidatePhysicalDeviceYcbcrImageArraysFeaturesEXT(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const VkBool32 ycbcrImageArrays) const;
bool ValidatePhysicalDeviceProvokingVertexFeaturesEXT(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const VkBool32 provokingVertexLast,
    const VkBool32 transformFeedbackPreservesProvokingVertex) const;
bool ValidatePipelineRasterizationProvokingVertexStateCreateInfoEXT(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const VkProvokingVertexModeEXT provokingVertexMode) const;
#ifdef VK_USE_PLATFORM_WIN32_KHR
bool ValidateSurfaceFullScreenExclusiveInfoEXT(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const VkFullScreenExclusiveEXT fullScreenExclusive) const;
#endif
#ifdef VK_USE_PLATFORM_WIN32_KHR
bool ValidateSurfaceCapabilitiesFullScreenExclusiveEXT(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const VkBool32 fullScreenExclusiveSupported) const;
#endif
#ifdef VK_USE_PLATFORM_WIN32_KHR
bool PreCallValidateGetPhysicalDeviceSurfacePresentModes2EXT(VkPhysicalDevice physicalDevice,
    const VkPhysicalDeviceSurfaceInfo2KHR* pSurfaceInfo,
    uint32_t* pPresentModeCount,
    VkPresentModeKHR* pPresentModes) const override;
#endif
#ifdef VK_USE_PLATFORM_WIN32_KHR
bool PreCallValidateAcquireFullScreenExclusiveModeEXT(VkDevice device,
    VkSwapchainKHR swapchain) const override;
#endif
#ifdef VK_USE_PLATFORM_WIN32_KHR
bool PreCallValidateReleaseFullScreenExclusiveModeEXT(VkDevice device,
    VkSwapchainKHR swapchain) const override;
#endif
#ifdef VK_USE_PLATFORM_WIN32_KHR
bool ValidateSurfaceFullScreenExclusiveWin32InfoEXT(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const HMONITOR hmonitor) const;
#endif
#ifdef VK_USE_PLATFORM_WIN32_KHR
bool PreCallValidateGetDeviceGroupSurfacePresentModes2EXT(VkDevice device,
    const VkPhysicalDeviceSurfaceInfo2KHR* pSurfaceInfo,
    VkDeviceGroupPresentModeFlagsKHR* pModes) const override;
#endif
bool ValidateHeadlessSurfaceCreateInfoEXT(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const VkHeadlessSurfaceCreateFlagsEXT flags) const;
bool PreCallValidateCreateHeadlessSurfaceEXT(VkInstance instance,
    const VkHeadlessSurfaceCreateInfoEXT* pCreateInfo,
    const VkAllocationCallbacks* pAllocator,
    VkSurfaceKHR* pSurface) const override;
bool ValidatePhysicalDeviceLineRasterizationFeaturesEXT(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const VkBool32 rectangularLines,
    const VkBool32 bresenhamLines,
    const VkBool32 smoothLines,
    const VkBool32 stippledRectangularLines,
    const VkBool32 stippledBresenhamLines,
    const VkBool32 stippledSmoothLines) const;
bool ValidatePipelineRasterizationLineStateCreateInfoEXT(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const VkLineRasterizationModeEXT lineRasterizationMode,
    const VkBool32 stippledLineEnable,
    const uint32_t lineStippleFactor,
    const uint16_t lineStipplePattern) const;
bool PreCallValidateCmdSetLineStippleEXT(VkCommandBuffer commandBuffer,
    uint32_t lineStippleFactor,
    uint16_t lineStipplePattern) const override;
bool ValidatePhysicalDeviceShaderAtomicFloatFeaturesEXT(const LogObjectList &_parentObjects,
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
    const VkBool32 sparseImageFloat32AtomicAdd) const;
bool ValidatePhysicalDeviceHostQueryResetFeaturesEXT(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const VkBool32 hostQueryReset) const;
bool PreCallValidateResetQueryPoolEXT(VkDevice device,
    VkQueryPool queryPool,
    uint32_t firstQuery,
    uint32_t queryCount) const override;
bool ValidatePhysicalDeviceIndexTypeUint8FeaturesEXT(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const VkBool32 indexTypeUint8) const;
bool ValidatePhysicalDeviceExtendedDynamicStateFeaturesEXT(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const VkBool32 extendedDynamicState) const;
bool PreCallValidateCmdSetCullModeEXT(VkCommandBuffer commandBuffer,
    VkCullModeFlags cullMode) const override;
bool PreCallValidateCmdSetFrontFaceEXT(VkCommandBuffer commandBuffer,
    VkFrontFace frontFace) const override;
bool PreCallValidateCmdSetPrimitiveTopologyEXT(VkCommandBuffer commandBuffer,
    VkPrimitiveTopology primitiveTopology) const override;
bool PreCallValidateCmdSetViewportWithCountEXT(VkCommandBuffer commandBuffer,
    uint32_t viewportCount,
    const VkViewport* pViewports) const override;
bool PreCallValidateCmdSetScissorWithCountEXT(VkCommandBuffer commandBuffer,
    uint32_t scissorCount,
    const VkRect2D* pScissors) const override;
bool PreCallValidateCmdBindVertexBuffers2EXT(VkCommandBuffer commandBuffer,
    uint32_t firstBinding,
    uint32_t bindingCount,
    const VkBuffer* pBuffers,
    const VkDeviceSize* pOffsets,
    const VkDeviceSize* pSizes,
    const VkDeviceSize* pStrides) const override;
bool PreCallValidateCmdSetDepthTestEnableEXT(VkCommandBuffer commandBuffer,
    VkBool32 depthTestEnable) const override;
bool PreCallValidateCmdSetDepthWriteEnableEXT(VkCommandBuffer commandBuffer,
    VkBool32 depthWriteEnable) const override;
bool PreCallValidateCmdSetDepthCompareOpEXT(VkCommandBuffer commandBuffer,
    VkCompareOp depthCompareOp) const override;
bool PreCallValidateCmdSetDepthBoundsTestEnableEXT(VkCommandBuffer commandBuffer,
    VkBool32 depthBoundsTestEnable) const override;
bool PreCallValidateCmdSetStencilTestEnableEXT(VkCommandBuffer commandBuffer,
    VkBool32 stencilTestEnable) const override;
bool PreCallValidateCmdSetStencilOpEXT(VkCommandBuffer commandBuffer,
    VkStencilFaceFlags faceMask,
    VkStencilOp failOp,
    VkStencilOp passOp,
    VkStencilOp depthFailOp,
    VkCompareOp compareOp) const override;
bool ValidatePhysicalDeviceShaderAtomicFloat2FeaturesEXT(const LogObjectList &_parentObjects,
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
    const VkBool32 sparseImageFloat32AtomicMinMax) const;
bool ValidateSurfacePresentModeEXT(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const VkPresentModeKHR presentMode) const;
bool ValidateSurfacePresentScalingCapabilitiesEXT(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const VkPresentScalingFlagsEXT supportedPresentScaling,
    const VkPresentGravityFlagsEXT supportedPresentGravityX,
    const VkPresentGravityFlagsEXT supportedPresentGravityY,
    const VkExtent2D minScaledImageExtent,
    const VkExtent2D maxScaledImageExtent) const;
bool ValidateSurfacePresentModeCompatibilityEXT(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const uint32_t presentModeCount,
    const VkPresentModeKHR* pPresentModes) const;
bool ValidatePhysicalDeviceSwapchainMaintenance1FeaturesEXT(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const VkBool32 swapchainMaintenance1) const;
bool ValidateSwapchainPresentFenceInfoEXT(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const uint32_t swapchainCount,
    const VkFence* pFences) const;
bool ValidateSwapchainPresentModesCreateInfoEXT(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const uint32_t presentModeCount,
    const VkPresentModeKHR* pPresentModes) const;
bool ValidateSwapchainPresentModeInfoEXT(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const uint32_t swapchainCount,
    const VkPresentModeKHR* pPresentModes) const;
bool ValidateSwapchainPresentScalingCreateInfoEXT(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const VkPresentScalingFlagsEXT scalingBehavior,
    const VkPresentGravityFlagsEXT presentGravityX,
    const VkPresentGravityFlagsEXT presentGravityY) const;
bool ValidateReleaseSwapchainImagesInfoEXT(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const VkSwapchainKHR swapchain,
    const uint32_t imageIndexCount,
    const uint32_t* pImageIndices) const;
bool PreCallValidateReleaseSwapchainImagesEXT(VkDevice device,
    const VkReleaseSwapchainImagesInfoEXT* pReleaseInfo) const override;
bool ValidatePhysicalDeviceShaderDemoteToHelperInvocationFeaturesEXT(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const VkBool32 shaderDemoteToHelperInvocation) const;
bool ValidatePhysicalDeviceDeviceGeneratedCommandsFeaturesNV(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const VkBool32 deviceGeneratedCommands) const;
bool ValidateGraphicsShaderGroupCreateInfoNV(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const uint32_t stageCount,
    const VkPipelineShaderStageCreateInfo* pStages,
    const VkPipelineVertexInputStateCreateInfo* pVertexInputState,
    const VkPipelineTessellationStateCreateInfo* pTessellationState) const;
bool ValidateGraphicsPipelineShaderGroupsCreateInfoNV(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const uint32_t groupCount,
    const VkGraphicsShaderGroupCreateInfoNV* pGroups,
    const uint32_t pipelineCount,
    const VkPipeline* pPipelines) const;
bool ValidateBindShaderGroupIndirectCommandNV(const LogObjectList &_parentObjects,
    const uint32_t groupIndex) const;
bool ValidateBindIndexBufferIndirectCommandNV(const LogObjectList &_parentObjects,
    const VkDeviceAddress bufferAddress,
    const uint32_t size,
    const VkIndexType indexType) const;
bool ValidateBindVertexBufferIndirectCommandNV(const LogObjectList &_parentObjects,
    const VkDeviceAddress bufferAddress,
    const uint32_t size,
    const uint32_t stride) const;
bool ValidateSetStateFlagsIndirectCommandNV(const LogObjectList &_parentObjects,
    const uint32_t data) const;
bool ValidateIndirectCommandsStreamNV(const LogObjectList &_parentObjects,
    const VkBuffer buffer,
    const VkDeviceSize offset) const;
bool ValidateIndirectCommandsLayoutTokenNV(const LogObjectList &_parentObjects,
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
    const uint32_t* pIndexTypeValues) const;
bool ValidateIndirectCommandsLayoutCreateInfoNV(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const VkIndirectCommandsLayoutUsageFlagsNV flags,
    const VkPipelineBindPoint pipelineBindPoint,
    const uint32_t tokenCount,
    const VkIndirectCommandsLayoutTokenNV* pTokens,
    const uint32_t streamCount,
    const uint32_t* pStreamStrides) const;
bool ValidateGeneratedCommandsInfoNV(const LogObjectList &_parentObjects,
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
    const VkDeviceSize sequencesIndexOffset) const;
bool ValidateGeneratedCommandsMemoryRequirementsInfoNV(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const VkPipelineBindPoint pipelineBindPoint,
    const VkPipeline pipeline,
    const VkIndirectCommandsLayoutNV indirectCommandsLayout,
    const uint32_t maxSequencesCount) const;
bool PreCallValidateGetGeneratedCommandsMemoryRequirementsNV(VkDevice device,
    const VkGeneratedCommandsMemoryRequirementsInfoNV* pInfo,
    VkMemoryRequirements2* pMemoryRequirements) const override;
bool PreCallValidateCmdPreprocessGeneratedCommandsNV(VkCommandBuffer commandBuffer,
    const VkGeneratedCommandsInfoNV* pGeneratedCommandsInfo) const override;
bool PreCallValidateCmdExecuteGeneratedCommandsNV(VkCommandBuffer commandBuffer,
    VkBool32 isPreprocessed,
    const VkGeneratedCommandsInfoNV* pGeneratedCommandsInfo) const override;
bool PreCallValidateCmdBindPipelineShaderGroupNV(VkCommandBuffer commandBuffer,
    VkPipelineBindPoint pipelineBindPoint,
    VkPipeline pipeline,
    uint32_t groupIndex) const override;
bool PreCallValidateCreateIndirectCommandsLayoutNV(VkDevice device,
    const VkIndirectCommandsLayoutCreateInfoNV* pCreateInfo,
    const VkAllocationCallbacks* pAllocator,
    VkIndirectCommandsLayoutNV* pIndirectCommandsLayout) const override;
bool PreCallValidateDestroyIndirectCommandsLayoutNV(VkDevice device,
    VkIndirectCommandsLayoutNV indirectCommandsLayout,
    const VkAllocationCallbacks* pAllocator) const override;
bool ValidatePhysicalDeviceInheritedViewportScissorFeaturesNV(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const VkBool32 inheritedViewportScissor2D) const;
bool ValidateCommandBufferInheritanceViewportScissorInfoNV(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const VkBool32 viewportScissor2D,
    const uint32_t viewportDepthCount,
    const VkViewport* pViewportDepths) const;
bool ValidatePhysicalDeviceTexelBufferAlignmentFeaturesEXT(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const VkBool32 texelBufferAlignment) const;
bool ValidateRenderPassTransformBeginInfoQCOM(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const VkSurfaceTransformFlagBitsKHR transform) const;
bool ValidateCommandBufferInheritanceRenderPassTransformInfoQCOM(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const VkSurfaceTransformFlagBitsKHR transform,
    const VkRect2D renderArea) const;
bool ValidatePhysicalDeviceDeviceMemoryReportFeaturesEXT(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const VkBool32 deviceMemoryReport) const;
bool ValidateDeviceDeviceMemoryReportCreateInfoEXT(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const VkDeviceMemoryReportFlagsEXT flags,
    const PFN_vkDeviceMemoryReportCallbackEXT pfnUserCallback,
    const void* pUserData) const;
bool PreCallValidateAcquireDrmDisplayEXT(VkPhysicalDevice physicalDevice,
    int32_t drmFd,
    VkDisplayKHR display) const override;
bool PreCallValidateGetDrmDisplayEXT(VkPhysicalDevice physicalDevice,
    int32_t drmFd,
    uint32_t connectorId,
    VkDisplayKHR* display) const override;
bool ValidatePhysicalDeviceRobustness2FeaturesEXT(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const VkBool32 robustBufferAccess2,
    const VkBool32 robustImageAccess2,
    const VkBool32 nullDescriptor) const;
bool ValidateSamplerCustomBorderColorCreateInfoEXT(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const VkClearColorValue customBorderColor,
    const VkFormat format) const;
bool ValidatePhysicalDeviceCustomBorderColorFeaturesEXT(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const VkBool32 customBorderColors,
    const VkBool32 customBorderColorWithoutFormat) const;
bool ValidatePhysicalDevicePresentBarrierFeaturesNV(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const VkBool32 presentBarrier) const;
bool ValidateSurfaceCapabilitiesPresentBarrierNV(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const VkBool32 presentBarrierSupported) const;
bool ValidateSwapchainPresentBarrierCreateInfoNV(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const VkBool32 presentBarrierEnable) const;
bool ValidatePhysicalDevicePrivateDataFeaturesEXT(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const VkBool32 privateData) const;
bool ValidateDevicePrivateDataCreateInfoEXT(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const uint32_t privateDataSlotRequestCount) const;
bool ValidatePrivateDataSlotCreateInfoEXT(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const VkPrivateDataSlotCreateFlags flags) const;
bool PreCallValidateCreatePrivateDataSlotEXT(VkDevice device,
    const VkPrivateDataSlotCreateInfo* pCreateInfo,
    const VkAllocationCallbacks* pAllocator,
    VkPrivateDataSlot* pPrivateDataSlot) const override;
bool PreCallValidateDestroyPrivateDataSlotEXT(VkDevice device,
    VkPrivateDataSlot privateDataSlot,
    const VkAllocationCallbacks* pAllocator) const override;
bool PreCallValidateSetPrivateDataEXT(VkDevice device,
    VkObjectType objectType,
    uint64_t objectHandle,
    VkPrivateDataSlot privateDataSlot,
    uint64_t data) const override;
bool PreCallValidateGetPrivateDataEXT(VkDevice device,
    VkObjectType objectType,
    uint64_t objectHandle,
    VkPrivateDataSlot privateDataSlot,
    uint64_t* pData) const override;
bool ValidatePhysicalDevicePipelineCreationCacheControlFeaturesEXT(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const VkBool32 pipelineCreationCacheControl) const;
bool ValidatePhysicalDeviceDiagnosticsConfigFeaturesNV(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const VkBool32 diagnosticsConfig) const;
bool ValidateDeviceDiagnosticsConfigCreateInfoNV(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const VkDeviceDiagnosticsConfigFlagsNV flags) const;
bool ValidateQueryLowLatencySupportNV(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const void* pQueriedLowLatencyData) const;
#ifdef VK_USE_PLATFORM_METAL_EXT
bool ValidateExportMetalObjectCreateInfoEXT(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const VkExportMetalObjectTypeFlagBitsEXT exportObjectType) const;
#endif
#ifdef VK_USE_PLATFORM_METAL_EXT
bool ValidateExportMetalObjectsInfoEXT(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext) const;
#endif
#ifdef VK_USE_PLATFORM_METAL_EXT
bool ValidateExportMetalDeviceInfoEXT(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const MTLDevice_id mtlDevice) const;
#endif
#ifdef VK_USE_PLATFORM_METAL_EXT
bool ValidateExportMetalCommandQueueInfoEXT(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const VkQueue queue,
    const MTLCommandQueue_id mtlCommandQueue) const;
#endif
#ifdef VK_USE_PLATFORM_METAL_EXT
bool ValidateExportMetalBufferInfoEXT(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const VkDeviceMemory memory,
    const MTLBuffer_id mtlBuffer) const;
#endif
#ifdef VK_USE_PLATFORM_METAL_EXT
bool ValidateImportMetalBufferInfoEXT(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const MTLBuffer_id mtlBuffer) const;
#endif
#ifdef VK_USE_PLATFORM_METAL_EXT
bool ValidateExportMetalTextureInfoEXT(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const VkImage image,
    const VkImageView imageView,
    const VkBufferView bufferView,
    const VkImageAspectFlagBits plane,
    const MTLTexture_id mtlTexture) const;
#endif
#ifdef VK_USE_PLATFORM_METAL_EXT
bool ValidateImportMetalTextureInfoEXT(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const VkImageAspectFlagBits plane,
    const MTLTexture_id mtlTexture) const;
#endif
#ifdef VK_USE_PLATFORM_METAL_EXT
bool ValidateExportMetalIOSurfaceInfoEXT(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const VkImage image,
    const IOSurfaceRef ioSurface) const;
#endif
#ifdef VK_USE_PLATFORM_METAL_EXT
bool ValidateImportMetalIOSurfaceInfoEXT(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const IOSurfaceRef ioSurface) const;
#endif
#ifdef VK_USE_PLATFORM_METAL_EXT
bool ValidateExportMetalSharedEventInfoEXT(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const VkSemaphore semaphore,
    const VkEvent event,
    const MTLSharedEvent_id mtlSharedEvent) const;
#endif
#ifdef VK_USE_PLATFORM_METAL_EXT
bool ValidateImportMetalSharedEventInfoEXT(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const MTLSharedEvent_id mtlSharedEvent) const;
#endif
#ifdef VK_USE_PLATFORM_METAL_EXT
bool PreCallValidateExportMetalObjectsEXT(VkDevice device,
    VkExportMetalObjectsInfoEXT* pMetalObjectsInfo) const override;
#endif
bool ValidatePhysicalDeviceDescriptorBufferFeaturesEXT(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const VkBool32 descriptorBuffer,
    const VkBool32 descriptorBufferCaptureReplay,
    const VkBool32 descriptorBufferImageLayoutIgnored,
    const VkBool32 descriptorBufferPushDescriptors) const;
bool ValidateDescriptorAddressInfoEXT(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const VkDeviceAddress address,
    const VkDeviceSize range,
    const VkFormat format) const;
bool ValidateDescriptorBufferBindingInfoEXT(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const VkDeviceAddress address,
    const VkBufferUsageFlags usage) const;
bool ValidateDescriptorBufferBindingPushDescriptorBufferHandleEXT(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const VkBuffer buffer) const;
bool ValidateDescriptorDataEXT(const LogObjectList &_parentObjects,
    const VkSampler* pSampler,
    const VkDescriptorImageInfo* pCombinedImageSampler,
    const VkDescriptorImageInfo* pInputAttachmentImage,
    const VkDescriptorImageInfo* pSampledImage,
    const VkDescriptorImageInfo* pStorageImage,
    const VkDescriptorAddressInfoEXT* pUniformTexelBuffer,
    const VkDescriptorAddressInfoEXT* pStorageTexelBuffer,
    const VkDescriptorAddressInfoEXT* pUniformBuffer,
    const VkDescriptorAddressInfoEXT* pStorageBuffer,
    const VkDeviceAddress accelerationStructure) const;
bool ValidateDescriptorGetInfoEXT(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const VkDescriptorType type,
    const VkDescriptorDataEXT data) const;
bool ValidateBufferCaptureDescriptorDataInfoEXT(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const VkBuffer buffer) const;
bool ValidateImageCaptureDescriptorDataInfoEXT(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const VkImage image) const;
bool ValidateImageViewCaptureDescriptorDataInfoEXT(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const VkImageView imageView) const;
bool ValidateSamplerCaptureDescriptorDataInfoEXT(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const VkSampler sampler) const;
bool ValidateOpaqueCaptureDescriptorDataCreateInfoEXT(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const void* opaqueCaptureDescriptorData) const;
bool PreCallValidateGetDescriptorSetLayoutSizeEXT(VkDevice device,
    VkDescriptorSetLayout layout,
    VkDeviceSize* pLayoutSizeInBytes) const override;
bool PreCallValidateGetDescriptorSetLayoutBindingOffsetEXT(VkDevice device,
    VkDescriptorSetLayout layout,
    uint32_t binding,
    VkDeviceSize* pOffset) const override;
bool PreCallValidateGetDescriptorEXT(VkDevice device,
    const VkDescriptorGetInfoEXT* pDescriptorInfo,
    size_t dataSize,
    void* pDescriptor) const override;
bool PreCallValidateCmdBindDescriptorBuffersEXT(VkCommandBuffer commandBuffer,
    uint32_t bufferCount,
    const VkDescriptorBufferBindingInfoEXT* pBindingInfos) const override;
bool PreCallValidateCmdSetDescriptorBufferOffsetsEXT(VkCommandBuffer commandBuffer,
    VkPipelineBindPoint pipelineBindPoint,
    VkPipelineLayout layout,
    uint32_t firstSet,
    uint32_t setCount,
    const uint32_t* pBufferIndices,
    const VkDeviceSize* pOffsets) const override;
bool PreCallValidateCmdBindDescriptorBufferEmbeddedSamplersEXT(VkCommandBuffer commandBuffer,
    VkPipelineBindPoint pipelineBindPoint,
    VkPipelineLayout layout,
    uint32_t set) const override;
bool PreCallValidateGetBufferOpaqueCaptureDescriptorDataEXT(VkDevice device,
    const VkBufferCaptureDescriptorDataInfoEXT* pInfo,
    void* pData) const override;
bool PreCallValidateGetImageOpaqueCaptureDescriptorDataEXT(VkDevice device,
    const VkImageCaptureDescriptorDataInfoEXT* pInfo,
    void* pData) const override;
bool PreCallValidateGetImageViewOpaqueCaptureDescriptorDataEXT(VkDevice device,
    const VkImageViewCaptureDescriptorDataInfoEXT* pInfo,
    void* pData) const override;
bool PreCallValidateGetSamplerOpaqueCaptureDescriptorDataEXT(VkDevice device,
    const VkSamplerCaptureDescriptorDataInfoEXT* pInfo,
    void* pData) const override;
bool ValidateAccelerationStructureCaptureDescriptorDataInfoEXT(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const VkAccelerationStructureKHR accelerationStructure,
    const VkAccelerationStructureNV accelerationStructureNV) const;
bool PreCallValidateGetAccelerationStructureOpaqueCaptureDescriptorDataEXT(VkDevice device,
    const VkAccelerationStructureCaptureDescriptorDataInfoEXT* pInfo,
    void* pData) const override;
bool ValidatePhysicalDeviceGraphicsPipelineLibraryFeaturesEXT(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const VkBool32 graphicsPipelineLibrary) const;
bool ValidatePhysicalDeviceGraphicsPipelineLibraryPropertiesEXT(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const VkBool32 graphicsPipelineLibraryFastLinking,
    const VkBool32 graphicsPipelineLibraryIndependentInterpolationDecoration) const;
bool ValidateGraphicsPipelineLibraryCreateInfoEXT(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const VkGraphicsPipelineLibraryFlagsEXT flags) const;
bool ValidatePhysicalDeviceShaderEarlyAndLateFragmentTestsFeaturesAMD(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const VkBool32 shaderEarlyAndLateFragmentTests) const;
bool ValidatePhysicalDeviceFragmentShadingRateEnumsFeaturesNV(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const VkBool32 fragmentShadingRateEnums,
    const VkBool32 supersampleFragmentShadingRates,
    const VkBool32 noInvocationFragmentShadingRates) const;
bool ValidatePhysicalDeviceFragmentShadingRateEnumsPropertiesNV(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const VkSampleCountFlagBits maxFragmentShadingRateInvocationCount) const;
bool ValidatePipelineFragmentShadingRateEnumStateCreateInfoNV(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const VkFragmentShadingRateTypeNV shadingRateType,
    const VkFragmentShadingRateNV shadingRate,
    const VkFragmentShadingRateCombinerOpKHR combinerOps[2]) const;
bool PreCallValidateCmdSetFragmentShadingRateEnumNV(VkCommandBuffer commandBuffer,
    VkFragmentShadingRateNV shadingRate,
    const VkFragmentShadingRateCombinerOpKHR combinerOps[2]) const override;
bool ValidateDeviceOrHostAddressConstKHR(const LogObjectList &_parentObjects,
    const VkDeviceAddress deviceAddress,
    const void* hostAddress) const;
bool ValidateAccelerationStructureGeometryMotionTrianglesDataNV(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const VkDeviceOrHostAddressConstKHR vertexData) const;
bool ValidateAccelerationStructureMotionInfoNV(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const uint32_t maxInstances,
    const VkAccelerationStructureMotionInfoFlagsNV flags) const;
bool ValidateAccelerationStructureMatrixMotionInstanceNV(const LogObjectList &_parentObjects,
    const VkTransformMatrixKHR transformT0,
    const VkTransformMatrixKHR transformT1,
    const uint32_t instanceCustomIndex,
    const uint32_t mask,
    const uint32_t instanceShaderBindingTableRecordOffset,
    const VkGeometryInstanceFlagsKHR flags,
    const uint64_t accelerationStructureReference) const;
bool ValidateSRTDataNV(const LogObjectList &_parentObjects,
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
    const float tz) const;
bool ValidateAccelerationStructureSRTMotionInstanceNV(const LogObjectList &_parentObjects,
    const VkSRTDataNV transformT0,
    const VkSRTDataNV transformT1,
    const uint32_t instanceCustomIndex,
    const uint32_t mask,
    const uint32_t instanceShaderBindingTableRecordOffset,
    const VkGeometryInstanceFlagsKHR flags,
    const uint64_t accelerationStructureReference) const;
bool ValidateAccelerationStructureMotionInstanceDataNV(const LogObjectList &_parentObjects,
    const VkAccelerationStructureInstanceKHR staticInstance,
    const VkAccelerationStructureMatrixMotionInstanceNV matrixMotionInstance,
    const VkAccelerationStructureSRTMotionInstanceNV srtMotionInstance) const;
bool ValidateAccelerationStructureMotionInstanceNV(const LogObjectList &_parentObjects,
    const VkAccelerationStructureMotionInstanceTypeNV type,
    const VkAccelerationStructureMotionInstanceFlagsNV flags,
    const VkAccelerationStructureMotionInstanceDataNV data) const;
bool ValidatePhysicalDeviceRayTracingMotionBlurFeaturesNV(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const VkBool32 rayTracingMotionBlur,
    const VkBool32 rayTracingMotionBlurPipelineTraceRaysIndirect) const;
bool ValidatePhysicalDeviceYcbcr2Plane444FormatsFeaturesEXT(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const VkBool32 ycbcr2plane444Formats) const;
bool ValidatePhysicalDeviceFragmentDensityMap2FeaturesEXT(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const VkBool32 fragmentDensityMapDeferred) const;
bool ValidateCopyCommandTransformInfoQCOM(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const VkSurfaceTransformFlagBitsKHR transform) const;
bool ValidatePhysicalDeviceImageRobustnessFeaturesEXT(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const VkBool32 robustImageAccess) const;
bool ValidatePhysicalDeviceImageCompressionControlFeaturesEXT(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const VkBool32 imageCompressionControl) const;
bool ValidateImageCompressionControlEXT(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const VkImageCompressionFlagsEXT flags,
    const uint32_t compressionControlPlaneCount,
    const VkImageCompressionFixedRateFlagsEXT* pFixedRateFlags) const;
bool ValidateImageSubresource2EXT(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const VkImageSubresource imageSubresource) const;
bool PreCallValidateGetImageSubresourceLayout2EXT(VkDevice device,
    VkImage image,
    const VkImageSubresource2EXT* pSubresource,
    VkSubresourceLayout2EXT* pLayout) const override;
bool ValidatePhysicalDeviceAttachmentFeedbackLoopLayoutFeaturesEXT(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const VkBool32 attachmentFeedbackLoopLayout) const;
bool ValidatePhysicalDevice4444FormatsFeaturesEXT(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const VkBool32 formatA4R4G4B4,
    const VkBool32 formatA4B4G4R4) const;
bool ValidatePhysicalDeviceFaultFeaturesEXT(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const VkBool32 deviceFault,
    const VkBool32 deviceFaultVendorBinary) const;
bool ValidateDeviceFaultCountsEXT(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const uint32_t addressInfoCount,
    const uint32_t vendorInfoCount,
    const VkDeviceSize vendorBinarySize) const;
bool ValidateDeviceFaultAddressInfoEXT(const LogObjectList &_parentObjects,
    const VkDeviceFaultAddressTypeEXT addressType,
    const VkDeviceAddress reportedAddress,
    const VkDeviceSize addressPrecision) const;
bool ValidateDeviceFaultVendorInfoEXT(const LogObjectList &_parentObjects,
    const char description[VK_MAX_DESCRIPTION_SIZE],
    const uint64_t vendorFaultCode,
    const uint64_t vendorFaultData) const;
bool ValidateDeviceFaultInfoEXT(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const char description[VK_MAX_DESCRIPTION_SIZE],
    const VkDeviceFaultAddressInfoEXT* pAddressInfos,
    const VkDeviceFaultVendorInfoEXT* pVendorInfos,
    const void* pVendorBinaryData) const;
bool ValidateDeviceFaultVendorBinaryHeaderVersionOneEXT(const LogObjectList &_parentObjects,
    const uint32_t headerSize,
    const VkDeviceFaultVendorBinaryHeaderVersionEXT headerVersion,
    const uint32_t vendorID,
    const uint32_t deviceID,
    const uint32_t driverVersion,
    const uint8_t pipelineCacheUUID[VK_UUID_SIZE],
    const uint32_t applicationNameOffset,
    const uint32_t applicationVersion,
    const uint32_t engineNameOffset) const;
bool PreCallValidateGetDeviceFaultInfoEXT(VkDevice device,
    VkDeviceFaultCountsEXT* pFaultCounts,
    VkDeviceFaultInfoEXT* pFaultInfo) const override;
bool ValidatePhysicalDeviceRasterizationOrderAttachmentAccessFeaturesEXT(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const VkBool32 rasterizationOrderColorAttachmentAccess,
    const VkBool32 rasterizationOrderDepthAttachmentAccess,
    const VkBool32 rasterizationOrderStencilAttachmentAccess) const;
bool ValidatePhysicalDeviceRasterizationOrderAttachmentAccessFeaturesARM(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const VkBool32 rasterizationOrderColorAttachmentAccess,
    const VkBool32 rasterizationOrderDepthAttachmentAccess,
    const VkBool32 rasterizationOrderStencilAttachmentAccess) const;
bool ValidatePhysicalDeviceRGBA10X6FormatsFeaturesEXT(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const VkBool32 formatRgba10x6WithoutYCbCrSampler) const;
#ifdef VK_USE_PLATFORM_WIN32_KHR
bool PreCallValidateAcquireWinrtDisplayNV(VkPhysicalDevice physicalDevice,
    VkDisplayKHR display) const override;
#endif
#ifdef VK_USE_PLATFORM_WIN32_KHR
bool PreCallValidateGetWinrtDisplayNV(VkPhysicalDevice physicalDevice,
    uint32_t deviceRelativeId,
    VkDisplayKHR* pDisplay) const override;
#endif
#ifdef VK_USE_PLATFORM_DIRECTFB_EXT
bool ValidateDirectFBSurfaceCreateInfoEXT(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const VkDirectFBSurfaceCreateFlagsEXT flags,
    const IDirectFB* dfb,
    const IDirectFBSurface* surface) const;
#endif
#ifdef VK_USE_PLATFORM_DIRECTFB_EXT
bool PreCallValidateCreateDirectFBSurfaceEXT(VkInstance instance,
    const VkDirectFBSurfaceCreateInfoEXT* pCreateInfo,
    const VkAllocationCallbacks* pAllocator,
    VkSurfaceKHR* pSurface) const override;
#endif
#ifdef VK_USE_PLATFORM_DIRECTFB_EXT
bool PreCallValidateGetPhysicalDeviceDirectFBPresentationSupportEXT(VkPhysicalDevice physicalDevice,
    uint32_t queueFamilyIndex,
    IDirectFB* dfb) const override;
#endif
bool ValidatePhysicalDeviceMutableDescriptorTypeFeaturesEXT(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const VkBool32 mutableDescriptorType) const;
bool ValidatePhysicalDeviceMutableDescriptorTypeFeaturesVALVE(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const VkBool32 mutableDescriptorType) const;
bool ValidateMutableDescriptorTypeListEXT(const LogObjectList &_parentObjects,
    const uint32_t descriptorTypeCount,
    const VkDescriptorType* pDescriptorTypes) const;
bool ValidateMutableDescriptorTypeListVALVE(const LogObjectList &_parentObjects,
    const uint32_t descriptorTypeCount,
    const VkDescriptorType* pDescriptorTypes) const;
bool ValidateMutableDescriptorTypeCreateInfoEXT(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const uint32_t mutableDescriptorTypeListCount,
    const VkMutableDescriptorTypeListEXT* pMutableDescriptorTypeLists) const;
bool ValidateMutableDescriptorTypeCreateInfoVALVE(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const uint32_t mutableDescriptorTypeListCount,
    const VkMutableDescriptorTypeListEXT* pMutableDescriptorTypeLists) const;
bool ValidatePhysicalDeviceVertexInputDynamicStateFeaturesEXT(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const VkBool32 vertexInputDynamicState) const;
bool ValidateVertexInputBindingDescription2EXT(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const uint32_t binding,
    const uint32_t stride,
    const VkVertexInputRate inputRate,
    const uint32_t divisor) const;
bool ValidateVertexInputAttributeDescription2EXT(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const uint32_t location,
    const uint32_t binding,
    const VkFormat format,
    const uint32_t offset) const;
bool PreCallValidateCmdSetVertexInputEXT(VkCommandBuffer commandBuffer,
    uint32_t vertexBindingDescriptionCount,
    const VkVertexInputBindingDescription2EXT* pVertexBindingDescriptions,
    uint32_t vertexAttributeDescriptionCount,
    const VkVertexInputAttributeDescription2EXT* pVertexAttributeDescriptions) const override;
bool ValidatePhysicalDeviceAddressBindingReportFeaturesEXT(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const VkBool32 reportAddressBinding) const;
bool ValidateDeviceAddressBindingCallbackDataEXT(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const VkDeviceAddressBindingFlagsEXT flags,
    const VkDeviceAddress baseAddress,
    const VkDeviceSize size,
    const VkDeviceAddressBindingTypeEXT bindingType) const;
bool ValidatePhysicalDeviceDepthClipControlFeaturesEXT(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const VkBool32 depthClipControl) const;
bool ValidatePipelineViewportDepthClipControlCreateInfoEXT(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const VkBool32 negativeOneToOne) const;
bool ValidatePhysicalDevicePrimitiveTopologyListRestartFeaturesEXT(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const VkBool32 primitiveTopologyListRestart,
    const VkBool32 primitiveTopologyPatchListRestart) const;
#ifdef VK_USE_PLATFORM_FUCHSIA
bool ValidateImportMemoryZirconHandleInfoFUCHSIA(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const VkExternalMemoryHandleTypeFlagBits handleType,
    const zx_handle_t handle) const;
#endif
#ifdef VK_USE_PLATFORM_FUCHSIA
bool ValidateMemoryGetZirconHandleInfoFUCHSIA(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const VkDeviceMemory memory,
    const VkExternalMemoryHandleTypeFlagBits handleType) const;
#endif
#ifdef VK_USE_PLATFORM_FUCHSIA
bool PreCallValidateGetMemoryZirconHandleFUCHSIA(VkDevice device,
    const VkMemoryGetZirconHandleInfoFUCHSIA* pGetZirconHandleInfo,
    zx_handle_t* pZirconHandle) const override;
#endif
#ifdef VK_USE_PLATFORM_FUCHSIA
bool PreCallValidateGetMemoryZirconHandlePropertiesFUCHSIA(VkDevice device,
    VkExternalMemoryHandleTypeFlagBits handleType,
    zx_handle_t zirconHandle,
    VkMemoryZirconHandlePropertiesFUCHSIA* pMemoryZirconHandleProperties) const override;
#endif
#ifdef VK_USE_PLATFORM_FUCHSIA
bool ValidateImportSemaphoreZirconHandleInfoFUCHSIA(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const VkSemaphore semaphore,
    const VkSemaphoreImportFlags flags,
    const VkExternalSemaphoreHandleTypeFlagBits handleType,
    const zx_handle_t zirconHandle) const;
#endif
#ifdef VK_USE_PLATFORM_FUCHSIA
bool ValidateSemaphoreGetZirconHandleInfoFUCHSIA(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const VkSemaphore semaphore,
    const VkExternalSemaphoreHandleTypeFlagBits handleType) const;
#endif
#ifdef VK_USE_PLATFORM_FUCHSIA
bool PreCallValidateImportSemaphoreZirconHandleFUCHSIA(VkDevice device,
    const VkImportSemaphoreZirconHandleInfoFUCHSIA* pImportSemaphoreZirconHandleInfo) const override;
#endif
#ifdef VK_USE_PLATFORM_FUCHSIA
bool PreCallValidateGetSemaphoreZirconHandleFUCHSIA(VkDevice device,
    const VkSemaphoreGetZirconHandleInfoFUCHSIA* pGetZirconHandleInfo,
    zx_handle_t* pZirconHandle) const override;
#endif
#ifdef VK_USE_PLATFORM_FUCHSIA
bool ValidateBufferCollectionCreateInfoFUCHSIA(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const zx_handle_t collectionToken) const;
#endif
#ifdef VK_USE_PLATFORM_FUCHSIA
bool ValidateImportMemoryBufferCollectionFUCHSIA(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const VkBufferCollectionFUCHSIA collection,
    const uint32_t index) const;
#endif
#ifdef VK_USE_PLATFORM_FUCHSIA
bool ValidateBufferCollectionImageCreateInfoFUCHSIA(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const VkBufferCollectionFUCHSIA collection,
    const uint32_t index) const;
#endif
#ifdef VK_USE_PLATFORM_FUCHSIA
bool ValidateBufferCollectionConstraintsInfoFUCHSIA(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const uint32_t minBufferCount,
    const uint32_t maxBufferCount,
    const uint32_t minBufferCountForCamping,
    const uint32_t minBufferCountForDedicatedSlack,
    const uint32_t minBufferCountForSharedSlack) const;
#endif
#ifdef VK_USE_PLATFORM_FUCHSIA
bool ValidateBufferConstraintsInfoFUCHSIA(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const VkBufferCreateInfo createInfo,
    const VkFormatFeatureFlags requiredFormatFeatures,
    const VkBufferCollectionConstraintsInfoFUCHSIA bufferCollectionConstraints) const;
#endif
#ifdef VK_USE_PLATFORM_FUCHSIA
bool ValidateBufferCollectionBufferCreateInfoFUCHSIA(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const VkBufferCollectionFUCHSIA collection,
    const uint32_t index) const;
#endif
#ifdef VK_USE_PLATFORM_FUCHSIA
bool ValidateSysmemColorSpaceFUCHSIA(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const uint32_t colorSpace) const;
#endif
#ifdef VK_USE_PLATFORM_FUCHSIA
bool ValidateBufferCollectionPropertiesFUCHSIA(const LogObjectList &_parentObjects,
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
    const VkChromaLocation suggestedYChromaOffset) const;
#endif
#ifdef VK_USE_PLATFORM_FUCHSIA
bool ValidateImageFormatConstraintsInfoFUCHSIA(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const VkImageCreateInfo imageCreateInfo,
    const VkFormatFeatureFlags requiredFormatFeatures,
    const VkImageFormatConstraintsFlagsFUCHSIA flags,
    const uint64_t sysmemPixelFormat,
    const uint32_t colorSpaceCount,
    const VkSysmemColorSpaceFUCHSIA* pColorSpaces) const;
#endif
#ifdef VK_USE_PLATFORM_FUCHSIA
bool ValidateImageConstraintsInfoFUCHSIA(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const uint32_t formatConstraintsCount,
    const VkImageFormatConstraintsInfoFUCHSIA* pFormatConstraints,
    const VkBufferCollectionConstraintsInfoFUCHSIA bufferCollectionConstraints,
    const VkImageConstraintsInfoFlagsFUCHSIA flags) const;
#endif
#ifdef VK_USE_PLATFORM_FUCHSIA
bool PreCallValidateCreateBufferCollectionFUCHSIA(VkDevice device,
    const VkBufferCollectionCreateInfoFUCHSIA* pCreateInfo,
    const VkAllocationCallbacks* pAllocator,
    VkBufferCollectionFUCHSIA* pCollection) const override;
#endif
#ifdef VK_USE_PLATFORM_FUCHSIA
bool PreCallValidateSetBufferCollectionImageConstraintsFUCHSIA(VkDevice device,
    VkBufferCollectionFUCHSIA collection,
    const VkImageConstraintsInfoFUCHSIA* pImageConstraintsInfo) const override;
#endif
#ifdef VK_USE_PLATFORM_FUCHSIA
bool PreCallValidateSetBufferCollectionBufferConstraintsFUCHSIA(VkDevice device,
    VkBufferCollectionFUCHSIA collection,
    const VkBufferConstraintsInfoFUCHSIA* pBufferConstraintsInfo) const override;
#endif
#ifdef VK_USE_PLATFORM_FUCHSIA
bool PreCallValidateDestroyBufferCollectionFUCHSIA(VkDevice device,
    VkBufferCollectionFUCHSIA collection,
    const VkAllocationCallbacks* pAllocator) const override;
#endif
#ifdef VK_USE_PLATFORM_FUCHSIA
bool PreCallValidateGetBufferCollectionPropertiesFUCHSIA(VkDevice device,
    VkBufferCollectionFUCHSIA collection,
    VkBufferCollectionPropertiesFUCHSIA* pProperties) const override;
#endif
bool ValidatePhysicalDeviceSubpassShadingFeaturesHUAWEI(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const VkBool32 subpassShading) const;
bool PreCallValidateGetDeviceSubpassShadingMaxWorkgroupSizeHUAWEI(VkDevice device,
    VkRenderPass renderpass,
    VkExtent2D* pMaxWorkgroupSize) const override;
bool PreCallValidateCmdSubpassShadingHUAWEI(VkCommandBuffer commandBuffer) const override;
bool ValidatePhysicalDeviceInvocationMaskFeaturesHUAWEI(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const VkBool32 invocationMask) const;
bool PreCallValidateCmdBindInvocationMaskHUAWEI(VkCommandBuffer commandBuffer,
    VkImageView imageView,
    VkImageLayout imageLayout) const override;
bool ValidateMemoryGetRemoteAddressInfoNV(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const VkDeviceMemory memory,
    const VkExternalMemoryHandleTypeFlagBits handleType) const;
bool ValidatePhysicalDeviceExternalMemoryRDMAFeaturesNV(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const VkBool32 externalMemoryRDMA) const;
bool PreCallValidateGetMemoryRemoteAddressNV(VkDevice device,
    const VkMemoryGetRemoteAddressInfoNV* pMemoryGetRemoteAddressInfo,
    VkRemoteAddressNV* pAddress) const override;
bool ValidatePipelineInfoEXT(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const VkPipeline pipeline) const;
bool ValidatePipelinePropertiesIdentifierEXT(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const uint8_t pipelineIdentifier[VK_UUID_SIZE]) const;
bool ValidatePhysicalDevicePipelinePropertiesFeaturesEXT(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const VkBool32 pipelinePropertiesIdentifier) const;
bool PreCallValidateGetPipelinePropertiesEXT(VkDevice device,
    const VkPipelineInfoEXT* pPipelineInfo,
    VkBaseOutStructure* pPipelineProperties) const override;
bool ValidatePhysicalDeviceMultisampledRenderToSingleSampledFeaturesEXT(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const VkBool32 multisampledRenderToSingleSampled) const;
bool ValidateMultisampledRenderToSingleSampledInfoEXT(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const VkBool32 multisampledRenderToSingleSampledEnable,
    const VkSampleCountFlagBits rasterizationSamples) const;
bool ValidatePhysicalDeviceExtendedDynamicState2FeaturesEXT(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const VkBool32 extendedDynamicState2,
    const VkBool32 extendedDynamicState2LogicOp,
    const VkBool32 extendedDynamicState2PatchControlPoints) const;
bool PreCallValidateCmdSetPatchControlPointsEXT(VkCommandBuffer commandBuffer,
    uint32_t patchControlPoints) const override;
bool PreCallValidateCmdSetRasterizerDiscardEnableEXT(VkCommandBuffer commandBuffer,
    VkBool32 rasterizerDiscardEnable) const override;
bool PreCallValidateCmdSetDepthBiasEnableEXT(VkCommandBuffer commandBuffer,
    VkBool32 depthBiasEnable) const override;
bool PreCallValidateCmdSetLogicOpEXT(VkCommandBuffer commandBuffer,
    VkLogicOp logicOp) const override;
bool PreCallValidateCmdSetPrimitiveRestartEnableEXT(VkCommandBuffer commandBuffer,
    VkBool32 primitiveRestartEnable) const override;
#ifdef VK_USE_PLATFORM_SCREEN_QNX
bool ValidateScreenSurfaceCreateInfoQNX(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const VkScreenSurfaceCreateFlagsQNX flags,
    const struct _screen_context* context,
    const struct _screen_window* window) const;
#endif
#ifdef VK_USE_PLATFORM_SCREEN_QNX
bool PreCallValidateCreateScreenSurfaceQNX(VkInstance instance,
    const VkScreenSurfaceCreateInfoQNX* pCreateInfo,
    const VkAllocationCallbacks* pAllocator,
    VkSurfaceKHR* pSurface) const override;
#endif
#ifdef VK_USE_PLATFORM_SCREEN_QNX
bool PreCallValidateGetPhysicalDeviceScreenPresentationSupportQNX(VkPhysicalDevice physicalDevice,
    uint32_t queueFamilyIndex,
    struct _screen_window* window) const override;
#endif
bool ValidatePhysicalDeviceColorWriteEnableFeaturesEXT(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const VkBool32 colorWriteEnable) const;
bool ValidatePipelineColorWriteCreateInfoEXT(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const uint32_t attachmentCount,
    const VkBool32* pColorWriteEnables) const;
bool PreCallValidateCmdSetColorWriteEnableEXT(VkCommandBuffer commandBuffer,
    uint32_t attachmentCount,
    const VkBool32* pColorWriteEnables) const override;
bool ValidatePhysicalDevicePrimitivesGeneratedQueryFeaturesEXT(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const VkBool32 primitivesGeneratedQuery,
    const VkBool32 primitivesGeneratedQueryWithRasterizerDiscard,
    const VkBool32 primitivesGeneratedQueryWithNonZeroStreams) const;
bool ValidatePhysicalDeviceGlobalPriorityQueryFeaturesEXT(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const VkBool32 globalPriorityQuery) const;
bool ValidateQueueFamilyGlobalPriorityPropertiesEXT(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const uint32_t priorityCount,
    const VkQueueGlobalPriorityKHR priorities[VK_MAX_GLOBAL_PRIORITY_SIZE_KHR]) const;
bool ValidatePhysicalDeviceImageViewMinLodFeaturesEXT(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const VkBool32 minLod) const;
bool ValidateImageViewMinLodCreateInfoEXT(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const float minLod) const;
bool ValidatePhysicalDeviceMultiDrawFeaturesEXT(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const VkBool32 multiDraw) const;
bool ValidateMultiDrawInfoEXT(const LogObjectList &_parentObjects,
    const uint32_t firstVertex,
    const uint32_t vertexCount) const;
bool ValidateMultiDrawIndexedInfoEXT(const LogObjectList &_parentObjects,
    const uint32_t firstIndex,
    const uint32_t indexCount,
    const int32_t vertexOffset) const;
bool PreCallValidateCmdDrawMultiEXT(VkCommandBuffer commandBuffer,
    uint32_t drawCount,
    const VkMultiDrawInfoEXT* pVertexInfo,
    uint32_t instanceCount,
    uint32_t firstInstance,
    uint32_t stride) const override;
bool PreCallValidateCmdDrawMultiIndexedEXT(VkCommandBuffer commandBuffer,
    uint32_t drawCount,
    const VkMultiDrawIndexedInfoEXT* pIndexInfo,
    uint32_t instanceCount,
    uint32_t firstInstance,
    uint32_t stride,
    const int32_t* pVertexOffset) const override;
bool ValidatePhysicalDeviceImage2DViewOf3DFeaturesEXT(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const VkBool32 image2DViewOf3D,
    const VkBool32 sampler2DViewOf3D) const;
bool ValidateMicromapUsageEXT(const LogObjectList &_parentObjects,
    const uint32_t count,
    const uint32_t subdivisionLevel,
    const uint32_t format) const;
bool ValidateDeviceOrHostAddressKHR(const LogObjectList &_parentObjects,
    const VkDeviceAddress deviceAddress,
    const void* hostAddress) const;
bool ValidateMicromapBuildInfoEXT(const LogObjectList &_parentObjects,
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
    const VkDeviceSize triangleArrayStride) const;
bool ValidateMicromapCreateInfoEXT(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const VkMicromapCreateFlagsEXT createFlags,
    const VkBuffer buffer,
    const VkDeviceSize offset,
    const VkDeviceSize size,
    const VkMicromapTypeEXT type,
    const VkDeviceAddress deviceAddress) const;
bool ValidatePhysicalDeviceOpacityMicromapFeaturesEXT(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const VkBool32 micromap,
    const VkBool32 micromapCaptureReplay,
    const VkBool32 micromapHostCommands) const;
bool ValidateMicromapVersionInfoEXT(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const uint8_t* pVersionData) const;
bool ValidateCopyMicromapToMemoryInfoEXT(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const VkMicromapEXT src,
    const VkDeviceOrHostAddressKHR dst,
    const VkCopyMicromapModeEXT mode) const;
bool ValidateCopyMemoryToMicromapInfoEXT(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const VkDeviceOrHostAddressConstKHR src,
    const VkMicromapEXT dst,
    const VkCopyMicromapModeEXT mode) const;
bool ValidateCopyMicromapInfoEXT(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const VkMicromapEXT src,
    const VkMicromapEXT dst,
    const VkCopyMicromapModeEXT mode) const;
bool ValidateMicromapBuildSizesInfoEXT(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const VkDeviceSize micromapSize,
    const VkDeviceSize buildScratchSize,
    const VkBool32 discardable) const;
bool ValidateAccelerationStructureTrianglesOpacityMicromapEXT(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const VkIndexType indexType,
    const VkDeviceOrHostAddressConstKHR indexBuffer,
    const VkDeviceSize indexStride,
    const uint32_t baseTriangle,
    const uint32_t usageCountsCount,
    const VkMicromapUsageEXT* pUsageCounts,
    const VkMicromapUsageEXT* const* ppUsageCounts,
    const VkMicromapEXT micromap) const;
bool ValidateMicromapTriangleEXT(const LogObjectList &_parentObjects,
    const uint32_t dataOffset,
    const uint16_t subdivisionLevel,
    const uint16_t format) const;
bool PreCallValidateCreateMicromapEXT(VkDevice device,
    const VkMicromapCreateInfoEXT* pCreateInfo,
    const VkAllocationCallbacks* pAllocator,
    VkMicromapEXT* pMicromap) const override;
bool PreCallValidateDestroyMicromapEXT(VkDevice device,
    VkMicromapEXT micromap,
    const VkAllocationCallbacks* pAllocator) const override;
bool PreCallValidateCmdBuildMicromapsEXT(VkCommandBuffer commandBuffer,
    uint32_t infoCount,
    const VkMicromapBuildInfoEXT* pInfos) const override;
bool PreCallValidateBuildMicromapsEXT(VkDevice device,
    VkDeferredOperationKHR deferredOperation,
    uint32_t infoCount,
    const VkMicromapBuildInfoEXT* pInfos) const override;
bool PreCallValidateCopyMicromapEXT(VkDevice device,
    VkDeferredOperationKHR deferredOperation,
    const VkCopyMicromapInfoEXT* pInfo) const override;
bool PreCallValidateCopyMicromapToMemoryEXT(VkDevice device,
    VkDeferredOperationKHR deferredOperation,
    const VkCopyMicromapToMemoryInfoEXT* pInfo) const override;
bool PreCallValidateCopyMemoryToMicromapEXT(VkDevice device,
    VkDeferredOperationKHR deferredOperation,
    const VkCopyMemoryToMicromapInfoEXT* pInfo) const override;
bool PreCallValidateWriteMicromapsPropertiesEXT(VkDevice device,
    uint32_t micromapCount,
    const VkMicromapEXT* pMicromaps,
    VkQueryType queryType,
    size_t dataSize,
    void* pData,
    size_t stride) const override;
bool PreCallValidateCmdCopyMicromapEXT(VkCommandBuffer commandBuffer,
    const VkCopyMicromapInfoEXT* pInfo) const override;
bool PreCallValidateCmdCopyMicromapToMemoryEXT(VkCommandBuffer commandBuffer,
    const VkCopyMicromapToMemoryInfoEXT* pInfo) const override;
bool PreCallValidateCmdCopyMemoryToMicromapEXT(VkCommandBuffer commandBuffer,
    const VkCopyMemoryToMicromapInfoEXT* pInfo) const override;
bool PreCallValidateCmdWriteMicromapsPropertiesEXT(VkCommandBuffer commandBuffer,
    uint32_t micromapCount,
    const VkMicromapEXT* pMicromaps,
    VkQueryType queryType,
    VkQueryPool queryPool,
    uint32_t firstQuery) const override;
bool PreCallValidateGetDeviceMicromapCompatibilityEXT(VkDevice device,
    const VkMicromapVersionInfoEXT* pVersionInfo,
    VkAccelerationStructureCompatibilityKHR* pCompatibility) const override;
bool PreCallValidateGetMicromapBuildSizesEXT(VkDevice device,
    VkAccelerationStructureBuildTypeKHR buildType,
    const VkMicromapBuildInfoEXT* pBuildInfo,
    VkMicromapBuildSizesInfoEXT* pSizeInfo) const override;
bool ValidatePhysicalDeviceClusterCullingShaderFeaturesHUAWEI(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void*pNext,
    const VkBool32 clustercullingShader,
    const VkBool32 multiviewClusterCullingShader) const;
bool PreCallValidateCmdDrawClusterHUAWEI(VkCommandBuffer commandBuffer,
    uint32_t groupCountX,
    uint32_t groupCountY,
    uint32_t groupCountZ) const override;
bool PreCallValidateCmdDrawClusterIndirectHUAWEI(VkCommandBuffer commandBuffer,
    VkBuffer buffer,
    VkDeviceSize offset) const override;
bool ValidatePhysicalDeviceBorderColorSwizzleFeaturesEXT(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const VkBool32 borderColorSwizzle,
    const VkBool32 borderColorSwizzleFromImage) const;
bool ValidateSamplerBorderColorComponentMappingCreateInfoEXT(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const VkComponentMapping components,
    const VkBool32 srgb) const;
bool ValidatePhysicalDevicePageableDeviceLocalMemoryFeaturesEXT(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const VkBool32 pageableDeviceLocalMemory) const;
bool PreCallValidateSetDeviceMemoryPriorityEXT(VkDevice device,
    VkDeviceMemory memory,
    float priority) const override;
bool ValidatePhysicalDeviceImageSlicedViewOf3DFeaturesEXT(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const VkBool32 imageSlicedViewOf3D) const;
bool ValidateImageViewSlicedCreateInfoEXT(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const uint32_t sliceOffset,
    const uint32_t sliceCount) const;
bool ValidatePhysicalDeviceDescriptorSetHostMappingFeaturesVALVE(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const VkBool32 descriptorSetHostMapping) const;
bool ValidateDescriptorSetBindingReferenceVALVE(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const VkDescriptorSetLayout descriptorSetLayout,
    const uint32_t binding) const;
bool ValidateDescriptorSetLayoutHostMappingInfoVALVE(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const size_t descriptorOffset,
    const uint32_t descriptorSize) const;
bool PreCallValidateGetDescriptorSetLayoutHostMappingInfoVALVE(VkDevice device,
    const VkDescriptorSetBindingReferenceVALVE* pBindingReference,
    VkDescriptorSetLayoutHostMappingInfoVALVE* pHostMapping) const override;
bool PreCallValidateGetDescriptorSetHostMappingVALVE(VkDevice device,
    VkDescriptorSet descriptorSet,
    void** ppData) const override;
bool ValidatePhysicalDeviceDepthClampZeroOneFeaturesEXT(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const VkBool32 depthClampZeroOne) const;
bool ValidatePhysicalDeviceNonSeamlessCubeMapFeaturesEXT(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const VkBool32 nonSeamlessCubeMap) const;
bool ValidatePhysicalDeviceFragmentDensityMapOffsetFeaturesQCOM(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const VkBool32 fragmentDensityMapOffset) const;
bool ValidateSubpassFragmentDensityMapOffsetEndInfoQCOM(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const uint32_t fragmentDensityOffsetCount,
    const VkOffset2D* pFragmentDensityOffsets) const;
bool ValidateCopyMemoryIndirectCommandNV(const LogObjectList &_parentObjects,
    const VkDeviceAddress srcAddress,
    const VkDeviceAddress dstAddress,
    const VkDeviceSize size) const;
bool ValidateCopyMemoryToImageIndirectCommandNV(const LogObjectList &_parentObjects,
    const VkDeviceAddress srcAddress,
    const uint32_t bufferRowLength,
    const uint32_t bufferImageHeight,
    const VkImageSubresourceLayers imageSubresource,
    const VkOffset3D imageOffset,
    const VkExtent3D imageExtent) const;
bool ValidatePhysicalDeviceCopyMemoryIndirectFeaturesNV(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const VkBool32 indirectCopy) const;
bool PreCallValidateCmdCopyMemoryIndirectNV(VkCommandBuffer commandBuffer,
    VkDeviceAddress copyBufferAddress,
    uint32_t copyCount,
    uint32_t stride) const override;
bool PreCallValidateCmdCopyMemoryToImageIndirectNV(VkCommandBuffer commandBuffer,
    VkDeviceAddress copyBufferAddress,
    uint32_t copyCount,
    uint32_t stride,
    VkImage dstImage,
    VkImageLayout dstImageLayout,
    const VkImageSubresourceLayers* pImageSubresources) const override;
bool ValidateDecompressMemoryRegionNV(const LogObjectList &_parentObjects,
    const VkDeviceAddress srcAddress,
    const VkDeviceAddress dstAddress,
    const VkDeviceSize compressedSize,
    const VkDeviceSize decompressedSize,
    const VkMemoryDecompressionMethodFlagsNV decompressionMethod) const;
bool ValidatePhysicalDeviceMemoryDecompressionFeaturesNV(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const VkBool32 memoryDecompression) const;
bool PreCallValidateCmdDecompressMemoryNV(VkCommandBuffer commandBuffer,
    uint32_t decompressRegionCount,
    const VkDecompressMemoryRegionNV* pDecompressMemoryRegions) const override;
bool PreCallValidateCmdDecompressMemoryIndirectCountNV(VkCommandBuffer commandBuffer,
    VkDeviceAddress indirectCommandsAddress,
    VkDeviceAddress indirectCommandsCountAddress,
    uint32_t stride) const override;
bool ValidatePhysicalDeviceLinearColorAttachmentFeaturesNV(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const VkBool32 linearColorAttachment) const;
bool ValidatePhysicalDeviceImageCompressionControlSwapchainFeaturesEXT(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const VkBool32 imageCompressionControlSwapchain) const;
bool ValidateImageViewSampleWeightCreateInfoQCOM(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const VkOffset2D filterCenter,
    const VkExtent2D filterSize,
    const uint32_t numPhases) const;
bool ValidatePhysicalDeviceImageProcessingFeaturesQCOM(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const VkBool32 textureSampleWeighted,
    const VkBool32 textureBoxFilter,
    const VkBool32 textureBlockMatch) const;
bool ValidatePhysicalDeviceExtendedDynamicState3FeaturesEXT(const LogObjectList &_parentObjects,
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
    const VkBool32 extendedDynamicState3ShadingRateImageEnable) const;
bool ValidatePhysicalDeviceExtendedDynamicState3PropertiesEXT(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const VkBool32 dynamicPrimitiveTopologyUnrestricted) const;
bool ValidateColorBlendEquationEXT(const LogObjectList &_parentObjects,
    const VkBlendFactor srcColorBlendFactor,
    const VkBlendFactor dstColorBlendFactor,
    const VkBlendOp colorBlendOp,
    const VkBlendFactor srcAlphaBlendFactor,
    const VkBlendFactor dstAlphaBlendFactor,
    const VkBlendOp alphaBlendOp) const;
bool ValidateColorBlendAdvancedEXT(const LogObjectList &_parentObjects,
    const VkBlendOp advancedBlendOp,
    const VkBool32 srcPremultiplied,
    const VkBool32 dstPremultiplied,
    const VkBlendOverlapEXT blendOverlap,
    const VkBool32 clampResults) const;
bool PreCallValidateCmdSetTessellationDomainOriginEXT(VkCommandBuffer commandBuffer,
    VkTessellationDomainOrigin domainOrigin) const override;
bool PreCallValidateCmdSetDepthClampEnableEXT(VkCommandBuffer commandBuffer,
    VkBool32 depthClampEnable) const override;
bool PreCallValidateCmdSetPolygonModeEXT(VkCommandBuffer commandBuffer,
    VkPolygonMode polygonMode) const override;
bool PreCallValidateCmdSetRasterizationSamplesEXT(VkCommandBuffer commandBuffer,
    VkSampleCountFlagBits rasterizationSamples) const override;
bool PreCallValidateCmdSetSampleMaskEXT(VkCommandBuffer commandBuffer,
    VkSampleCountFlagBits samples,
    const VkSampleMask* pSampleMask) const override;
bool PreCallValidateCmdSetAlphaToCoverageEnableEXT(VkCommandBuffer commandBuffer,
    VkBool32 alphaToCoverageEnable) const override;
bool PreCallValidateCmdSetAlphaToOneEnableEXT(VkCommandBuffer commandBuffer,
    VkBool32 alphaToOneEnable) const override;
bool PreCallValidateCmdSetLogicOpEnableEXT(VkCommandBuffer commandBuffer,
    VkBool32 logicOpEnable) const override;
bool PreCallValidateCmdSetColorBlendEnableEXT(VkCommandBuffer commandBuffer,
    uint32_t firstAttachment,
    uint32_t attachmentCount,
    const VkBool32* pColorBlendEnables) const override;
bool PreCallValidateCmdSetColorBlendEquationEXT(VkCommandBuffer commandBuffer,
    uint32_t firstAttachment,
    uint32_t attachmentCount,
    const VkColorBlendEquationEXT* pColorBlendEquations) const override;
bool PreCallValidateCmdSetColorWriteMaskEXT(VkCommandBuffer commandBuffer,
    uint32_t firstAttachment,
    uint32_t attachmentCount,
    const VkColorComponentFlags* pColorWriteMasks) const override;
bool PreCallValidateCmdSetRasterizationStreamEXT(VkCommandBuffer commandBuffer,
    uint32_t rasterizationStream) const override;
bool PreCallValidateCmdSetConservativeRasterizationModeEXT(VkCommandBuffer commandBuffer,
    VkConservativeRasterizationModeEXT conservativeRasterizationMode) const override;
bool PreCallValidateCmdSetExtraPrimitiveOverestimationSizeEXT(VkCommandBuffer commandBuffer,
    float extraPrimitiveOverestimationSize) const override;
bool PreCallValidateCmdSetDepthClipEnableEXT(VkCommandBuffer commandBuffer,
    VkBool32 depthClipEnable) const override;
bool PreCallValidateCmdSetSampleLocationsEnableEXT(VkCommandBuffer commandBuffer,
    VkBool32 sampleLocationsEnable) const override;
bool PreCallValidateCmdSetColorBlendAdvancedEXT(VkCommandBuffer commandBuffer,
    uint32_t firstAttachment,
    uint32_t attachmentCount,
    const VkColorBlendAdvancedEXT* pColorBlendAdvanced) const override;
bool PreCallValidateCmdSetProvokingVertexModeEXT(VkCommandBuffer commandBuffer,
    VkProvokingVertexModeEXT provokingVertexMode) const override;
bool PreCallValidateCmdSetLineRasterizationModeEXT(VkCommandBuffer commandBuffer,
    VkLineRasterizationModeEXT lineRasterizationMode) const override;
bool PreCallValidateCmdSetLineStippleEnableEXT(VkCommandBuffer commandBuffer,
    VkBool32 stippledLineEnable) const override;
bool PreCallValidateCmdSetDepthClipNegativeOneToOneEXT(VkCommandBuffer commandBuffer,
    VkBool32 negativeOneToOne) const override;
bool PreCallValidateCmdSetViewportWScalingEnableNV(VkCommandBuffer commandBuffer,
    VkBool32 viewportWScalingEnable) const override;
bool PreCallValidateCmdSetViewportSwizzleNV(VkCommandBuffer commandBuffer,
    uint32_t firstViewport,
    uint32_t viewportCount,
    const VkViewportSwizzleNV* pViewportSwizzles) const override;
bool PreCallValidateCmdSetCoverageToColorEnableNV(VkCommandBuffer commandBuffer,
    VkBool32 coverageToColorEnable) const override;
bool PreCallValidateCmdSetCoverageToColorLocationNV(VkCommandBuffer commandBuffer,
    uint32_t coverageToColorLocation) const override;
bool PreCallValidateCmdSetCoverageModulationModeNV(VkCommandBuffer commandBuffer,
    VkCoverageModulationModeNV coverageModulationMode) const override;
bool PreCallValidateCmdSetCoverageModulationTableEnableNV(VkCommandBuffer commandBuffer,
    VkBool32 coverageModulationTableEnable) const override;
bool PreCallValidateCmdSetCoverageModulationTableNV(VkCommandBuffer commandBuffer,
    uint32_t coverageModulationTableCount,
    const float* pCoverageModulationTable) const override;
bool PreCallValidateCmdSetShadingRateImageEnableNV(VkCommandBuffer commandBuffer,
    VkBool32 shadingRateImageEnable) const override;
bool PreCallValidateCmdSetRepresentativeFragmentTestEnableNV(VkCommandBuffer commandBuffer,
    VkBool32 representativeFragmentTestEnable) const override;
bool PreCallValidateCmdSetCoverageReductionModeNV(VkCommandBuffer commandBuffer,
    VkCoverageReductionModeNV coverageReductionMode) const override;
bool ValidatePhysicalDeviceSubpassMergeFeedbackFeaturesEXT(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const VkBool32 subpassMergeFeedback) const;
bool ValidateRenderPassCreationControlEXT(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const VkBool32 disallowMerging) const;
bool ValidateRenderPassCreationFeedbackCreateInfoEXT(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const VkRenderPassCreationFeedbackInfoEXT* pRenderPassFeedback) const;
bool ValidateRenderPassSubpassFeedbackCreateInfoEXT(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const VkRenderPassSubpassFeedbackInfoEXT* pSubpassFeedback) const;
bool ValidateDirectDriverLoadingInfoLUNARG(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const VkDirectDriverLoadingFlagsLUNARG flags,
    const PFN_vkGetInstanceProcAddrLUNARG pfnGetInstanceProcAddr) const;
bool ValidateDirectDriverLoadingListLUNARG(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const VkDirectDriverLoadingModeLUNARG mode,
    const uint32_t driverCount,
    const VkDirectDriverLoadingInfoLUNARG* pDrivers) const;
bool ValidatePhysicalDeviceShaderModuleIdentifierFeaturesEXT(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const VkBool32 shaderModuleIdentifier) const;
bool ValidatePipelineShaderStageModuleIdentifierCreateInfoEXT(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const uint32_t identifierSize,
    const uint8_t* pIdentifier) const;
bool PreCallValidateGetShaderModuleIdentifierEXT(VkDevice device,
    VkShaderModule shaderModule,
    VkShaderModuleIdentifierEXT* pIdentifier) const override;
bool PreCallValidateGetShaderModuleCreateInfoIdentifierEXT(VkDevice device,
    const VkShaderModuleCreateInfo* pCreateInfo,
    VkShaderModuleIdentifierEXT* pIdentifier) const override;
bool ValidatePhysicalDeviceOpticalFlowFeaturesNV(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const VkBool32 opticalFlow) const;
bool ValidateOpticalFlowImageFormatInfoNV(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const VkOpticalFlowUsageFlagsNV usage) const;
bool ValidateOpticalFlowSessionCreateInfoNV(const LogObjectList &_parentObjects,
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
    const VkOpticalFlowSessionCreateFlagsNV flags) const;
bool ValidateOpticalFlowSessionCreatePrivateDataInfoNV(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const uint32_t id,
    const uint32_t size,
    const void* pPrivateData) const;
bool ValidateOpticalFlowExecuteInfoNV(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const VkOpticalFlowExecuteFlagsNV flags,
    const uint32_t regionCount,
    const VkRect2D* pRegions) const;
bool PreCallValidateGetPhysicalDeviceOpticalFlowImageFormatsNV(VkPhysicalDevice physicalDevice,
    const VkOpticalFlowImageFormatInfoNV* pOpticalFlowImageFormatInfo,
    uint32_t* pFormatCount,
    VkOpticalFlowImageFormatPropertiesNV* pImageFormatProperties) const override;
bool PreCallValidateCreateOpticalFlowSessionNV(VkDevice device,
    const VkOpticalFlowSessionCreateInfoNV* pCreateInfo,
    const VkAllocationCallbacks* pAllocator,
    VkOpticalFlowSessionNV* pSession) const override;
bool PreCallValidateDestroyOpticalFlowSessionNV(VkDevice device,
    VkOpticalFlowSessionNV session,
    const VkAllocationCallbacks* pAllocator) const override;
bool PreCallValidateBindOpticalFlowSessionImageNV(VkDevice device,
    VkOpticalFlowSessionNV session,
    VkOpticalFlowSessionBindingPointNV bindingPoint,
    VkImageView view,
    VkImageLayout layout) const override;
bool PreCallValidateCmdOpticalFlowExecuteNV(VkCommandBuffer commandBuffer,
    VkOpticalFlowSessionNV session,
    const VkOpticalFlowExecuteInfoNV* pExecuteInfo) const override;
bool ValidatePhysicalDeviceLegacyDitheringFeaturesEXT(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const VkBool32 legacyDithering) const;
bool ValidatePhysicalDevicePipelineProtectedAccessFeaturesEXT(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const VkBool32 pipelineProtectedAccess) const;
bool ValidatePhysicalDeviceTilePropertiesFeaturesQCOM(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const VkBool32 tileProperties) const;
bool ValidateTilePropertiesQCOM(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const VkExtent3D tileSize,
    const VkExtent2D apronSize,
    const VkOffset2D origin) const;
bool PreCallValidateGetFramebufferTilePropertiesQCOM(VkDevice device,
    VkFramebuffer framebuffer,
    uint32_t* pPropertiesCount,
    VkTilePropertiesQCOM* pProperties) const override;
bool PreCallValidateGetDynamicRenderingTilePropertiesQCOM(VkDevice device,
    const VkRenderingInfo* pRenderingInfo,
    VkTilePropertiesQCOM* pProperties) const override;
bool ValidatePhysicalDeviceAmigoProfilingFeaturesSEC(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const VkBool32 amigoProfiling) const;
bool ValidateAmigoProfilingSubmitInfoSEC(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const uint64_t firstDrawTimestamp,
    const uint64_t swapBufferTimestamp) const;
bool ValidatePhysicalDeviceMultiviewPerViewViewportsFeaturesQCOM(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const VkBool32 multiviewPerViewViewports) const;
bool ValidatePhysicalDeviceRayTracingInvocationReorderFeaturesNV(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const VkBool32 rayTracingInvocationReorder) const;
bool ValidatePhysicalDeviceShaderCoreBuiltinsFeaturesARM(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const VkBool32 shaderCoreBuiltins) const;
bool ValidatePhysicalDevicePipelineLibraryGroupHandlesFeaturesEXT(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const VkBool32 pipelineLibraryGroupHandles) const;
bool ValidatePhysicalDeviceMultiviewPerViewRenderAreasFeaturesQCOM(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const VkBool32 multiviewPerViewRenderAreas) const;
bool ValidateMultiviewPerViewRenderAreasRenderPassBeginInfoQCOM(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const uint32_t perViewRenderAreaCount,
    const VkRect2D* pPerViewRenderAreas) const;
bool ValidateAccelerationStructureBuildRangeInfoKHR(const LogObjectList &_parentObjects,
    const uint32_t primitiveCount,
    const uint32_t primitiveOffset,
    const uint32_t firstVertex,
    const uint32_t transformOffset) const;
bool ValidateAccelerationStructureGeometryTrianglesDataKHR(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const VkFormat vertexFormat,
    const VkDeviceOrHostAddressConstKHR vertexData,
    const VkDeviceSize vertexStride,
    const uint32_t maxVertex,
    const VkIndexType indexType,
    const VkDeviceOrHostAddressConstKHR indexData,
    const VkDeviceOrHostAddressConstKHR transformData) const;
bool ValidateAccelerationStructureGeometryAabbsDataKHR(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const VkDeviceOrHostAddressConstKHR data,
    const VkDeviceSize stride) const;
bool ValidateAccelerationStructureGeometryInstancesDataKHR(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const VkBool32 arrayOfPointers,
    const VkDeviceOrHostAddressConstKHR data) const;
bool ValidateAccelerationStructureGeometryDataKHR(const LogObjectList &_parentObjects,
    const VkAccelerationStructureGeometryTrianglesDataKHR triangles,
    const VkAccelerationStructureGeometryAabbsDataKHR aabbs,
    const VkAccelerationStructureGeometryInstancesDataKHR instances) const;
bool ValidateAccelerationStructureGeometryKHR(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const VkGeometryTypeKHR geometryType,
    const VkAccelerationStructureGeometryDataKHR geometry,
    const VkGeometryFlagsKHR flags) const;
bool ValidateAccelerationStructureBuildGeometryInfoKHR(const LogObjectList &_parentObjects,
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
    const VkDeviceOrHostAddressKHR scratchData) const;
bool ValidateAccelerationStructureCreateInfoKHR(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const VkAccelerationStructureCreateFlagsKHR createFlags,
    const VkBuffer buffer,
    const VkDeviceSize offset,
    const VkDeviceSize size,
    const VkAccelerationStructureTypeKHR type,
    const VkDeviceAddress deviceAddress) const;
bool ValidateWriteDescriptorSetAccelerationStructureKHR(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const uint32_t accelerationStructureCount,
    const VkAccelerationStructureKHR* pAccelerationStructures) const;
bool ValidatePhysicalDeviceAccelerationStructureFeaturesKHR(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const VkBool32 accelerationStructure,
    const VkBool32 accelerationStructureCaptureReplay,
    const VkBool32 accelerationStructureIndirectBuild,
    const VkBool32 accelerationStructureHostCommands,
    const VkBool32 descriptorBindingAccelerationStructureUpdateAfterBind) const;
bool ValidateAccelerationStructureDeviceAddressInfoKHR(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const VkAccelerationStructureKHR accelerationStructure) const;
bool ValidateAccelerationStructureVersionInfoKHR(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const uint8_t* pVersionData) const;
bool ValidateCopyAccelerationStructureToMemoryInfoKHR(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const VkAccelerationStructureKHR src,
    const VkDeviceOrHostAddressKHR dst,
    const VkCopyAccelerationStructureModeKHR mode) const;
bool ValidateCopyMemoryToAccelerationStructureInfoKHR(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const VkDeviceOrHostAddressConstKHR src,
    const VkAccelerationStructureKHR dst,
    const VkCopyAccelerationStructureModeKHR mode) const;
bool ValidateCopyAccelerationStructureInfoKHR(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const VkAccelerationStructureKHR src,
    const VkAccelerationStructureKHR dst,
    const VkCopyAccelerationStructureModeKHR mode) const;
bool ValidateAccelerationStructureBuildSizesInfoKHR(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const VkDeviceSize accelerationStructureSize,
    const VkDeviceSize updateScratchSize,
    const VkDeviceSize buildScratchSize) const;
bool PreCallValidateCreateAccelerationStructureKHR(VkDevice device,
    const VkAccelerationStructureCreateInfoKHR* pCreateInfo,
    const VkAllocationCallbacks* pAllocator,
    VkAccelerationStructureKHR* pAccelerationStructure) const override;
bool PreCallValidateDestroyAccelerationStructureKHR(VkDevice device,
    VkAccelerationStructureKHR accelerationStructure,
    const VkAllocationCallbacks* pAllocator) const override;
bool PreCallValidateCmdBuildAccelerationStructuresKHR(VkCommandBuffer commandBuffer,
    uint32_t infoCount,
    const VkAccelerationStructureBuildGeometryInfoKHR* pInfos,
    const VkAccelerationStructureBuildRangeInfoKHR* const* ppBuildRangeInfos) const override;
bool PreCallValidateCmdBuildAccelerationStructuresIndirectKHR(VkCommandBuffer commandBuffer,
    uint32_t infoCount,
    const VkAccelerationStructureBuildGeometryInfoKHR* pInfos,
    const VkDeviceAddress* pIndirectDeviceAddresses,
    const uint32_t* pIndirectStrides,
    const uint32_t* const* ppMaxPrimitiveCounts) const override;
bool PreCallValidateBuildAccelerationStructuresKHR(VkDevice device,
    VkDeferredOperationKHR deferredOperation,
    uint32_t infoCount,
    const VkAccelerationStructureBuildGeometryInfoKHR* pInfos,
    const VkAccelerationStructureBuildRangeInfoKHR* const* ppBuildRangeInfos) const override;
bool PreCallValidateCopyAccelerationStructureKHR(VkDevice device,
    VkDeferredOperationKHR deferredOperation,
    const VkCopyAccelerationStructureInfoKHR* pInfo) const override;
bool PreCallValidateCopyAccelerationStructureToMemoryKHR(VkDevice device,
    VkDeferredOperationKHR deferredOperation,
    const VkCopyAccelerationStructureToMemoryInfoKHR* pInfo) const override;
bool PreCallValidateCopyMemoryToAccelerationStructureKHR(VkDevice device,
    VkDeferredOperationKHR deferredOperation,
    const VkCopyMemoryToAccelerationStructureInfoKHR* pInfo) const override;
bool PreCallValidateWriteAccelerationStructuresPropertiesKHR(VkDevice device,
    uint32_t accelerationStructureCount,
    const VkAccelerationStructureKHR* pAccelerationStructures,
    VkQueryType queryType,
    size_t dataSize,
    void* pData,
    size_t stride) const override;
bool PreCallValidateCmdCopyAccelerationStructureKHR(VkCommandBuffer commandBuffer,
    const VkCopyAccelerationStructureInfoKHR* pInfo) const override;
bool PreCallValidateCmdCopyAccelerationStructureToMemoryKHR(VkCommandBuffer commandBuffer,
    const VkCopyAccelerationStructureToMemoryInfoKHR* pInfo) const override;
bool PreCallValidateCmdCopyMemoryToAccelerationStructureKHR(VkCommandBuffer commandBuffer,
    const VkCopyMemoryToAccelerationStructureInfoKHR* pInfo) const override;
bool PreCallValidateGetAccelerationStructureDeviceAddressKHR(VkDevice device,
    const VkAccelerationStructureDeviceAddressInfoKHR* pInfo) const override;
bool PreCallValidateCmdWriteAccelerationStructuresPropertiesKHR(VkCommandBuffer commandBuffer,
    uint32_t accelerationStructureCount,
    const VkAccelerationStructureKHR* pAccelerationStructures,
    VkQueryType queryType,
    VkQueryPool queryPool,
    uint32_t firstQuery) const override;
bool PreCallValidateGetDeviceAccelerationStructureCompatibilityKHR(VkDevice device,
    const VkAccelerationStructureVersionInfoKHR* pVersionInfo,
    VkAccelerationStructureCompatibilityKHR* pCompatibility) const override;
bool PreCallValidateGetAccelerationStructureBuildSizesKHR(VkDevice device,
    VkAccelerationStructureBuildTypeKHR buildType,
    const VkAccelerationStructureBuildGeometryInfoKHR* pBuildInfo,
    const uint32_t* pMaxPrimitiveCounts,
    VkAccelerationStructureBuildSizesInfoKHR* pSizeInfo) const override;
bool ValidateRayTracingShaderGroupCreateInfoKHR(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const VkRayTracingShaderGroupTypeKHR type,
    const uint32_t generalShader,
    const uint32_t closestHitShader,
    const uint32_t anyHitShader,
    const uint32_t intersectionShader,
    const void* pShaderGroupCaptureReplayHandle) const;
bool ValidateRayTracingPipelineInterfaceCreateInfoKHR(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const uint32_t maxPipelineRayPayloadSize,
    const uint32_t maxPipelineRayHitAttributeSize) const;
bool ValidateRayTracingPipelineCreateInfoKHR(const LogObjectList &_parentObjects,
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
    const int32_t basePipelineIndex) const;
bool ValidatePhysicalDeviceRayTracingPipelineFeaturesKHR(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const VkBool32 rayTracingPipeline,
    const VkBool32 rayTracingPipelineShaderGroupHandleCaptureReplay,
    const VkBool32 rayTracingPipelineShaderGroupHandleCaptureReplayMixed,
    const VkBool32 rayTracingPipelineTraceRaysIndirect,
    const VkBool32 rayTraversalPrimitiveCulling) const;
bool ValidateStridedDeviceAddressRegionKHR(const LogObjectList &_parentObjects,
    const VkDeviceAddress deviceAddress,
    const VkDeviceSize stride,
    const VkDeviceSize size) const;
bool ValidateTraceRaysIndirectCommandKHR(const LogObjectList &_parentObjects,
    const uint32_t width,
    const uint32_t height,
    const uint32_t depth) const;
bool PreCallValidateCmdTraceRaysKHR(VkCommandBuffer commandBuffer,
    const VkStridedDeviceAddressRegionKHR* pRaygenShaderBindingTable,
    const VkStridedDeviceAddressRegionKHR* pMissShaderBindingTable,
    const VkStridedDeviceAddressRegionKHR* pHitShaderBindingTable,
    const VkStridedDeviceAddressRegionKHR* pCallableShaderBindingTable,
    uint32_t width,
    uint32_t height,
    uint32_t depth) const override;
bool PreCallValidateCreateRayTracingPipelinesKHR(VkDevice device,
    VkDeferredOperationKHR deferredOperation,
    VkPipelineCache pipelineCache,
    uint32_t createInfoCount,
    const VkRayTracingPipelineCreateInfoKHR* pCreateInfos,
    const VkAllocationCallbacks* pAllocator,
    VkPipeline* pPipelines,
    void *validation_state) const override;
bool PreCallValidateGetRayTracingCaptureReplayShaderGroupHandlesKHR(VkDevice device,
    VkPipeline pipeline,
    uint32_t firstGroup,
    uint32_t groupCount,
    size_t dataSize,
    void* pData) const override;
bool PreCallValidateCmdTraceRaysIndirectKHR(VkCommandBuffer commandBuffer,
    const VkStridedDeviceAddressRegionKHR* pRaygenShaderBindingTable,
    const VkStridedDeviceAddressRegionKHR* pMissShaderBindingTable,
    const VkStridedDeviceAddressRegionKHR* pHitShaderBindingTable,
    const VkStridedDeviceAddressRegionKHR* pCallableShaderBindingTable,
    VkDeviceAddress indirectDeviceAddress) const override;
bool PreCallValidateGetRayTracingShaderGroupStackSizeKHR(VkDevice device,
    VkPipeline pipeline,
    uint32_t group,
    VkShaderGroupShaderKHR groupShader) const override;
bool PreCallValidateCmdSetRayTracingPipelineStackSizeKHR(VkCommandBuffer commandBuffer,
    uint32_t pipelineStackSize) const override;
bool ValidatePhysicalDeviceRayQueryFeaturesKHR(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const VkBool32 rayQuery) const;
bool ValidatePhysicalDeviceMeshShaderFeaturesEXT(const LogObjectList &_parentObjects,
    const VkStructureType sType,
    const void* pNext,
    const VkBool32 taskShader,
    const VkBool32 meshShader,
    const VkBool32 multiviewMeshShader,
    const VkBool32 primitiveFragmentShadingRateMeshShader,
    const VkBool32 meshShaderQueries) const;
bool ValidateDrawMeshTasksIndirectCommandEXT(const LogObjectList &_parentObjects,
    const uint32_t groupCountX,
    const uint32_t groupCountY,
    const uint32_t groupCountZ) const;
bool PreCallValidateCmdDrawMeshTasksEXT(VkCommandBuffer commandBuffer,
    uint32_t groupCountX,
    uint32_t groupCountY,
    uint32_t groupCountZ) const override;
bool PreCallValidateCmdDrawMeshTasksIndirectEXT(VkCommandBuffer commandBuffer,
    VkBuffer buffer,
    VkDeviceSize offset,
    uint32_t drawCount,
    uint32_t stride) const override;
bool PreCallValidateCmdDrawMeshTasksIndirectCountEXT(VkCommandBuffer commandBuffer,
    VkBuffer buffer,
    VkDeviceSize offset,
    VkBuffer countBuffer,
    VkDeviceSize countBufferOffset,
    uint32_t maxDrawCount,
    uint32_t stride) const override;
bool ValidatePNext(const LogObjectList &_parentObjects, const void *pnext) const;

