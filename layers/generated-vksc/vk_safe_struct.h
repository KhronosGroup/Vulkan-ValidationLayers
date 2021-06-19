// *** THIS FILE IS GENERATED - DO NOT EDIT ***
// See helper_file_generator.py for modifications


/***************************************************************************
 *
 * Copyright (c) 2015-2021 The Khronos Group Inc.
 * Copyright (c) 2015-2021 Valve Corporation
 * Copyright (c) 2015-2021 LunarG, Inc.
 * Copyright (c) 2015-2021 Google Inc.
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
 * Author: Courtney Goeltzenleuchter <courtneygo@google.com>
 * Author: Tobin Ehlis <tobine@google.com>
 * Author: Chris Forbes <chrisforbes@google.com>
 * Author: John Zulauf<jzulauf@lunarg.com>
 *
 ****************************************************************************/


#pragma once
#include <vulkan/vulkan_sc.h>
#include <stdlib.h>

void *SafePnextCopy(const void *pNext);
void FreePnextChain(const void *pNext);
char *SafeStringCopy(const char *in_string);


struct safe_VkBufferMemoryBarrier {
    VkStructureType sType;
    const void* pNext;
    VkAccessFlags srcAccessMask;
    VkAccessFlags dstAccessMask;
    uint32_t srcQueueFamilyIndex;
    uint32_t dstQueueFamilyIndex;
    VkBuffer buffer;
    VkDeviceSize offset;
    VkDeviceSize size;
    safe_VkBufferMemoryBarrier(const VkBufferMemoryBarrier* in_struct);
    safe_VkBufferMemoryBarrier(const safe_VkBufferMemoryBarrier& copy_src);
    safe_VkBufferMemoryBarrier& operator=(const safe_VkBufferMemoryBarrier& copy_src);
    safe_VkBufferMemoryBarrier();
    ~safe_VkBufferMemoryBarrier();
    void initialize(const VkBufferMemoryBarrier* in_struct);
    void initialize(const safe_VkBufferMemoryBarrier* copy_src);
    VkBufferMemoryBarrier *ptr() { return reinterpret_cast<VkBufferMemoryBarrier *>(this); }
    VkBufferMemoryBarrier const *ptr() const { return reinterpret_cast<VkBufferMemoryBarrier const *>(this); }
};

struct safe_VkImageMemoryBarrier {
    VkStructureType sType;
    const void* pNext;
    VkAccessFlags srcAccessMask;
    VkAccessFlags dstAccessMask;
    VkImageLayout oldLayout;
    VkImageLayout newLayout;
    uint32_t srcQueueFamilyIndex;
    uint32_t dstQueueFamilyIndex;
    VkImage image;
    VkImageSubresourceRange subresourceRange;
    safe_VkImageMemoryBarrier(const VkImageMemoryBarrier* in_struct);
    safe_VkImageMemoryBarrier(const safe_VkImageMemoryBarrier& copy_src);
    safe_VkImageMemoryBarrier& operator=(const safe_VkImageMemoryBarrier& copy_src);
    safe_VkImageMemoryBarrier();
    ~safe_VkImageMemoryBarrier();
    void initialize(const VkImageMemoryBarrier* in_struct);
    void initialize(const safe_VkImageMemoryBarrier* copy_src);
    VkImageMemoryBarrier *ptr() { return reinterpret_cast<VkImageMemoryBarrier *>(this); }
    VkImageMemoryBarrier const *ptr() const { return reinterpret_cast<VkImageMemoryBarrier const *>(this); }
};

struct safe_VkMemoryBarrier {
    VkStructureType sType;
    const void* pNext;
    VkAccessFlags srcAccessMask;
    VkAccessFlags dstAccessMask;
    safe_VkMemoryBarrier(const VkMemoryBarrier* in_struct);
    safe_VkMemoryBarrier(const safe_VkMemoryBarrier& copy_src);
    safe_VkMemoryBarrier& operator=(const safe_VkMemoryBarrier& copy_src);
    safe_VkMemoryBarrier();
    ~safe_VkMemoryBarrier();
    void initialize(const VkMemoryBarrier* in_struct);
    void initialize(const safe_VkMemoryBarrier* copy_src);
    VkMemoryBarrier *ptr() { return reinterpret_cast<VkMemoryBarrier *>(this); }
    VkMemoryBarrier const *ptr() const { return reinterpret_cast<VkMemoryBarrier const *>(this); }
};

struct safe_VkAllocationCallbacks {
    void* pUserData;
    PFN_vkAllocationFunction pfnAllocation;
    PFN_vkReallocationFunction pfnReallocation;
    PFN_vkFreeFunction pfnFree;
    PFN_vkInternalAllocationNotification pfnInternalAllocation;
    PFN_vkInternalFreeNotification pfnInternalFree;
    safe_VkAllocationCallbacks(const VkAllocationCallbacks* in_struct);
    safe_VkAllocationCallbacks(const safe_VkAllocationCallbacks& copy_src);
    safe_VkAllocationCallbacks& operator=(const safe_VkAllocationCallbacks& copy_src);
    safe_VkAllocationCallbacks();
    ~safe_VkAllocationCallbacks();
    void initialize(const VkAllocationCallbacks* in_struct);
    void initialize(const safe_VkAllocationCallbacks* copy_src);
    VkAllocationCallbacks *ptr() { return reinterpret_cast<VkAllocationCallbacks *>(this); }
    VkAllocationCallbacks const *ptr() const { return reinterpret_cast<VkAllocationCallbacks const *>(this); }
};

struct safe_VkApplicationInfo {
    VkStructureType sType;
    const void* pNext;
    const char* pApplicationName;
    uint32_t applicationVersion;
    const char* pEngineName;
    uint32_t engineVersion;
    uint32_t apiVersion;
    safe_VkApplicationInfo(const VkApplicationInfo* in_struct);
    safe_VkApplicationInfo(const safe_VkApplicationInfo& copy_src);
    safe_VkApplicationInfo& operator=(const safe_VkApplicationInfo& copy_src);
    safe_VkApplicationInfo();
    ~safe_VkApplicationInfo();
    void initialize(const VkApplicationInfo* in_struct);
    void initialize(const safe_VkApplicationInfo* copy_src);
    VkApplicationInfo *ptr() { return reinterpret_cast<VkApplicationInfo *>(this); }
    VkApplicationInfo const *ptr() const { return reinterpret_cast<VkApplicationInfo const *>(this); }
};

struct safe_VkInstanceCreateInfo {
    VkStructureType sType;
    const void* pNext;
    VkInstanceCreateFlags flags;
    safe_VkApplicationInfo* pApplicationInfo;
    uint32_t enabledLayerCount;
    const char* const* ppEnabledLayerNames;
    uint32_t enabledExtensionCount;
    const char* const* ppEnabledExtensionNames;
    safe_VkInstanceCreateInfo(const VkInstanceCreateInfo* in_struct);
    safe_VkInstanceCreateInfo(const safe_VkInstanceCreateInfo& copy_src);
    safe_VkInstanceCreateInfo& operator=(const safe_VkInstanceCreateInfo& copy_src);
    safe_VkInstanceCreateInfo();
    ~safe_VkInstanceCreateInfo();
    void initialize(const VkInstanceCreateInfo* in_struct);
    void initialize(const safe_VkInstanceCreateInfo* copy_src);
    VkInstanceCreateInfo *ptr() { return reinterpret_cast<VkInstanceCreateInfo *>(this); }
    VkInstanceCreateInfo const *ptr() const { return reinterpret_cast<VkInstanceCreateInfo const *>(this); }
};

struct safe_VkDeviceQueueCreateInfo {
    VkStructureType sType;
    const void* pNext;
    VkDeviceQueueCreateFlags flags;
    uint32_t queueFamilyIndex;
    uint32_t queueCount;
    const float* pQueuePriorities;
    safe_VkDeviceQueueCreateInfo(const VkDeviceQueueCreateInfo* in_struct);
    safe_VkDeviceQueueCreateInfo(const safe_VkDeviceQueueCreateInfo& copy_src);
    safe_VkDeviceQueueCreateInfo& operator=(const safe_VkDeviceQueueCreateInfo& copy_src);
    safe_VkDeviceQueueCreateInfo();
    ~safe_VkDeviceQueueCreateInfo();
    void initialize(const VkDeviceQueueCreateInfo* in_struct);
    void initialize(const safe_VkDeviceQueueCreateInfo* copy_src);
    VkDeviceQueueCreateInfo *ptr() { return reinterpret_cast<VkDeviceQueueCreateInfo *>(this); }
    VkDeviceQueueCreateInfo const *ptr() const { return reinterpret_cast<VkDeviceQueueCreateInfo const *>(this); }
};

struct safe_VkDeviceCreateInfo {
    VkStructureType sType;
    const void* pNext;
    VkDeviceCreateFlags flags;
    uint32_t queueCreateInfoCount;
    safe_VkDeviceQueueCreateInfo* pQueueCreateInfos;
    uint32_t enabledLayerCount;
    const char* const* ppEnabledLayerNames;
    uint32_t enabledExtensionCount;
    const char* const* ppEnabledExtensionNames;
    const VkPhysicalDeviceFeatures* pEnabledFeatures;
    safe_VkDeviceCreateInfo(const VkDeviceCreateInfo* in_struct);
    safe_VkDeviceCreateInfo(const safe_VkDeviceCreateInfo& copy_src);
    safe_VkDeviceCreateInfo& operator=(const safe_VkDeviceCreateInfo& copy_src);
    safe_VkDeviceCreateInfo();
    ~safe_VkDeviceCreateInfo();
    void initialize(const VkDeviceCreateInfo* in_struct);
    void initialize(const safe_VkDeviceCreateInfo* copy_src);
    VkDeviceCreateInfo *ptr() { return reinterpret_cast<VkDeviceCreateInfo *>(this); }
    VkDeviceCreateInfo const *ptr() const { return reinterpret_cast<VkDeviceCreateInfo const *>(this); }
};

struct safe_VkSubmitInfo {
    VkStructureType sType;
    const void* pNext;
    uint32_t waitSemaphoreCount;
    VkSemaphore* pWaitSemaphores;
    const VkPipelineStageFlags* pWaitDstStageMask;
    uint32_t commandBufferCount;
    VkCommandBuffer* pCommandBuffers;
    uint32_t signalSemaphoreCount;
    VkSemaphore* pSignalSemaphores;
    safe_VkSubmitInfo(const VkSubmitInfo* in_struct);
    safe_VkSubmitInfo(const safe_VkSubmitInfo& copy_src);
    safe_VkSubmitInfo& operator=(const safe_VkSubmitInfo& copy_src);
    safe_VkSubmitInfo();
    ~safe_VkSubmitInfo();
    void initialize(const VkSubmitInfo* in_struct);
    void initialize(const safe_VkSubmitInfo* copy_src);
    VkSubmitInfo *ptr() { return reinterpret_cast<VkSubmitInfo *>(this); }
    VkSubmitInfo const *ptr() const { return reinterpret_cast<VkSubmitInfo const *>(this); }
};

struct safe_VkMappedMemoryRange {
    VkStructureType sType;
    const void* pNext;
    VkDeviceMemory memory;
    VkDeviceSize offset;
    VkDeviceSize size;
    safe_VkMappedMemoryRange(const VkMappedMemoryRange* in_struct);
    safe_VkMappedMemoryRange(const safe_VkMappedMemoryRange& copy_src);
    safe_VkMappedMemoryRange& operator=(const safe_VkMappedMemoryRange& copy_src);
    safe_VkMappedMemoryRange();
    ~safe_VkMappedMemoryRange();
    void initialize(const VkMappedMemoryRange* in_struct);
    void initialize(const safe_VkMappedMemoryRange* copy_src);
    VkMappedMemoryRange *ptr() { return reinterpret_cast<VkMappedMemoryRange *>(this); }
    VkMappedMemoryRange const *ptr() const { return reinterpret_cast<VkMappedMemoryRange const *>(this); }
};

struct safe_VkMemoryAllocateInfo {
    VkStructureType sType;
    const void* pNext;
    VkDeviceSize allocationSize;
    uint32_t memoryTypeIndex;
    safe_VkMemoryAllocateInfo(const VkMemoryAllocateInfo* in_struct);
    safe_VkMemoryAllocateInfo(const safe_VkMemoryAllocateInfo& copy_src);
    safe_VkMemoryAllocateInfo& operator=(const safe_VkMemoryAllocateInfo& copy_src);
    safe_VkMemoryAllocateInfo();
    ~safe_VkMemoryAllocateInfo();
    void initialize(const VkMemoryAllocateInfo* in_struct);
    void initialize(const safe_VkMemoryAllocateInfo* copy_src);
    VkMemoryAllocateInfo *ptr() { return reinterpret_cast<VkMemoryAllocateInfo *>(this); }
    VkMemoryAllocateInfo const *ptr() const { return reinterpret_cast<VkMemoryAllocateInfo const *>(this); }
};

struct safe_VkFenceCreateInfo {
    VkStructureType sType;
    const void* pNext;
    VkFenceCreateFlags flags;
    safe_VkFenceCreateInfo(const VkFenceCreateInfo* in_struct);
    safe_VkFenceCreateInfo(const safe_VkFenceCreateInfo& copy_src);
    safe_VkFenceCreateInfo& operator=(const safe_VkFenceCreateInfo& copy_src);
    safe_VkFenceCreateInfo();
    ~safe_VkFenceCreateInfo();
    void initialize(const VkFenceCreateInfo* in_struct);
    void initialize(const safe_VkFenceCreateInfo* copy_src);
    VkFenceCreateInfo *ptr() { return reinterpret_cast<VkFenceCreateInfo *>(this); }
    VkFenceCreateInfo const *ptr() const { return reinterpret_cast<VkFenceCreateInfo const *>(this); }
};

struct safe_VkSemaphoreCreateInfo {
    VkStructureType sType;
    const void* pNext;
    VkSemaphoreCreateFlags flags;
    safe_VkSemaphoreCreateInfo(const VkSemaphoreCreateInfo* in_struct);
    safe_VkSemaphoreCreateInfo(const safe_VkSemaphoreCreateInfo& copy_src);
    safe_VkSemaphoreCreateInfo& operator=(const safe_VkSemaphoreCreateInfo& copy_src);
    safe_VkSemaphoreCreateInfo();
    ~safe_VkSemaphoreCreateInfo();
    void initialize(const VkSemaphoreCreateInfo* in_struct);
    void initialize(const safe_VkSemaphoreCreateInfo* copy_src);
    VkSemaphoreCreateInfo *ptr() { return reinterpret_cast<VkSemaphoreCreateInfo *>(this); }
    VkSemaphoreCreateInfo const *ptr() const { return reinterpret_cast<VkSemaphoreCreateInfo const *>(this); }
};

struct safe_VkEventCreateInfo {
    VkStructureType sType;
    const void* pNext;
    VkEventCreateFlags flags;
    safe_VkEventCreateInfo(const VkEventCreateInfo* in_struct);
    safe_VkEventCreateInfo(const safe_VkEventCreateInfo& copy_src);
    safe_VkEventCreateInfo& operator=(const safe_VkEventCreateInfo& copy_src);
    safe_VkEventCreateInfo();
    ~safe_VkEventCreateInfo();
    void initialize(const VkEventCreateInfo* in_struct);
    void initialize(const safe_VkEventCreateInfo* copy_src);
    VkEventCreateInfo *ptr() { return reinterpret_cast<VkEventCreateInfo *>(this); }
    VkEventCreateInfo const *ptr() const { return reinterpret_cast<VkEventCreateInfo const *>(this); }
};

struct safe_VkQueryPoolCreateInfo {
    VkStructureType sType;
    const void* pNext;
    VkQueryPoolCreateFlags flags;
    VkQueryType queryType;
    uint32_t queryCount;
    VkQueryPipelineStatisticFlags pipelineStatistics;
    safe_VkQueryPoolCreateInfo(const VkQueryPoolCreateInfo* in_struct);
    safe_VkQueryPoolCreateInfo(const safe_VkQueryPoolCreateInfo& copy_src);
    safe_VkQueryPoolCreateInfo& operator=(const safe_VkQueryPoolCreateInfo& copy_src);
    safe_VkQueryPoolCreateInfo();
    ~safe_VkQueryPoolCreateInfo();
    void initialize(const VkQueryPoolCreateInfo* in_struct);
    void initialize(const safe_VkQueryPoolCreateInfo* copy_src);
    VkQueryPoolCreateInfo *ptr() { return reinterpret_cast<VkQueryPoolCreateInfo *>(this); }
    VkQueryPoolCreateInfo const *ptr() const { return reinterpret_cast<VkQueryPoolCreateInfo const *>(this); }
};

struct safe_VkBufferCreateInfo {
    VkStructureType sType;
    const void* pNext;
    VkBufferCreateFlags flags;
    VkDeviceSize size;
    VkBufferUsageFlags usage;
    VkSharingMode sharingMode;
    uint32_t queueFamilyIndexCount;
    const uint32_t* pQueueFamilyIndices;
    safe_VkBufferCreateInfo(const VkBufferCreateInfo* in_struct);
    safe_VkBufferCreateInfo(const safe_VkBufferCreateInfo& copy_src);
    safe_VkBufferCreateInfo& operator=(const safe_VkBufferCreateInfo& copy_src);
    safe_VkBufferCreateInfo();
    ~safe_VkBufferCreateInfo();
    void initialize(const VkBufferCreateInfo* in_struct);
    void initialize(const safe_VkBufferCreateInfo* copy_src);
    VkBufferCreateInfo *ptr() { return reinterpret_cast<VkBufferCreateInfo *>(this); }
    VkBufferCreateInfo const *ptr() const { return reinterpret_cast<VkBufferCreateInfo const *>(this); }
};

struct safe_VkBufferViewCreateInfo {
    VkStructureType sType;
    const void* pNext;
    VkBufferViewCreateFlags flags;
    VkBuffer buffer;
    VkFormat format;
    VkDeviceSize offset;
    VkDeviceSize range;
    safe_VkBufferViewCreateInfo(const VkBufferViewCreateInfo* in_struct);
    safe_VkBufferViewCreateInfo(const safe_VkBufferViewCreateInfo& copy_src);
    safe_VkBufferViewCreateInfo& operator=(const safe_VkBufferViewCreateInfo& copy_src);
    safe_VkBufferViewCreateInfo();
    ~safe_VkBufferViewCreateInfo();
    void initialize(const VkBufferViewCreateInfo* in_struct);
    void initialize(const safe_VkBufferViewCreateInfo* copy_src);
    VkBufferViewCreateInfo *ptr() { return reinterpret_cast<VkBufferViewCreateInfo *>(this); }
    VkBufferViewCreateInfo const *ptr() const { return reinterpret_cast<VkBufferViewCreateInfo const *>(this); }
};

struct safe_VkImageCreateInfo {
    VkStructureType sType;
    const void* pNext;
    VkImageCreateFlags flags;
    VkImageType imageType;
    VkFormat format;
    VkExtent3D extent;
    uint32_t mipLevels;
    uint32_t arrayLayers;
    VkSampleCountFlagBits samples;
    VkImageTiling tiling;
    VkImageUsageFlags usage;
    VkSharingMode sharingMode;
    uint32_t queueFamilyIndexCount;
    const uint32_t* pQueueFamilyIndices;
    VkImageLayout initialLayout;
    safe_VkImageCreateInfo(const VkImageCreateInfo* in_struct);
    safe_VkImageCreateInfo(const safe_VkImageCreateInfo& copy_src);
    safe_VkImageCreateInfo& operator=(const safe_VkImageCreateInfo& copy_src);
    safe_VkImageCreateInfo();
    ~safe_VkImageCreateInfo();
    void initialize(const VkImageCreateInfo* in_struct);
    void initialize(const safe_VkImageCreateInfo* copy_src);
    VkImageCreateInfo *ptr() { return reinterpret_cast<VkImageCreateInfo *>(this); }
    VkImageCreateInfo const *ptr() const { return reinterpret_cast<VkImageCreateInfo const *>(this); }
};

struct safe_VkImageViewCreateInfo {
    VkStructureType sType;
    const void* pNext;
    VkImageViewCreateFlags flags;
    VkImage image;
    VkImageViewType viewType;
    VkFormat format;
    VkComponentMapping components;
    VkImageSubresourceRange subresourceRange;
    safe_VkImageViewCreateInfo(const VkImageViewCreateInfo* in_struct);
    safe_VkImageViewCreateInfo(const safe_VkImageViewCreateInfo& copy_src);
    safe_VkImageViewCreateInfo& operator=(const safe_VkImageViewCreateInfo& copy_src);
    safe_VkImageViewCreateInfo();
    ~safe_VkImageViewCreateInfo();
    void initialize(const VkImageViewCreateInfo* in_struct);
    void initialize(const safe_VkImageViewCreateInfo* copy_src);
    VkImageViewCreateInfo *ptr() { return reinterpret_cast<VkImageViewCreateInfo *>(this); }
    VkImageViewCreateInfo const *ptr() const { return reinterpret_cast<VkImageViewCreateInfo const *>(this); }
};

struct safe_VkPipelineCacheCreateInfo {
    VkStructureType sType;
    const void* pNext;
    VkPipelineCacheCreateFlags flags;
    size_t initialDataSize;
    const void* pInitialData;
    safe_VkPipelineCacheCreateInfo(const VkPipelineCacheCreateInfo* in_struct);
    safe_VkPipelineCacheCreateInfo(const safe_VkPipelineCacheCreateInfo& copy_src);
    safe_VkPipelineCacheCreateInfo& operator=(const safe_VkPipelineCacheCreateInfo& copy_src);
    safe_VkPipelineCacheCreateInfo();
    ~safe_VkPipelineCacheCreateInfo();
    void initialize(const VkPipelineCacheCreateInfo* in_struct);
    void initialize(const safe_VkPipelineCacheCreateInfo* copy_src);
    VkPipelineCacheCreateInfo *ptr() { return reinterpret_cast<VkPipelineCacheCreateInfo *>(this); }
    VkPipelineCacheCreateInfo const *ptr() const { return reinterpret_cast<VkPipelineCacheCreateInfo const *>(this); }
};

struct safe_VkSpecializationInfo {
    uint32_t mapEntryCount;
    const VkSpecializationMapEntry* pMapEntries;
    size_t dataSize;
    const void* pData;
    safe_VkSpecializationInfo(const VkSpecializationInfo* in_struct);
    safe_VkSpecializationInfo(const safe_VkSpecializationInfo& copy_src);
    safe_VkSpecializationInfo& operator=(const safe_VkSpecializationInfo& copy_src);
    safe_VkSpecializationInfo();
    ~safe_VkSpecializationInfo();
    void initialize(const VkSpecializationInfo* in_struct);
    void initialize(const safe_VkSpecializationInfo* copy_src);
    VkSpecializationInfo *ptr() { return reinterpret_cast<VkSpecializationInfo *>(this); }
    VkSpecializationInfo const *ptr() const { return reinterpret_cast<VkSpecializationInfo const *>(this); }
};

struct safe_VkPipelineShaderStageCreateInfo {
    VkStructureType sType;
    const void* pNext;
    VkPipelineShaderStageCreateFlags flags;
    VkShaderStageFlagBits stage;
    VkShaderModule module;
    const char* pName;
    safe_VkSpecializationInfo* pSpecializationInfo;
    safe_VkPipelineShaderStageCreateInfo(const VkPipelineShaderStageCreateInfo* in_struct);
    safe_VkPipelineShaderStageCreateInfo(const safe_VkPipelineShaderStageCreateInfo& copy_src);
    safe_VkPipelineShaderStageCreateInfo& operator=(const safe_VkPipelineShaderStageCreateInfo& copy_src);
    safe_VkPipelineShaderStageCreateInfo();
    ~safe_VkPipelineShaderStageCreateInfo();
    void initialize(const VkPipelineShaderStageCreateInfo* in_struct);
    void initialize(const safe_VkPipelineShaderStageCreateInfo* copy_src);
    VkPipelineShaderStageCreateInfo *ptr() { return reinterpret_cast<VkPipelineShaderStageCreateInfo *>(this); }
    VkPipelineShaderStageCreateInfo const *ptr() const { return reinterpret_cast<VkPipelineShaderStageCreateInfo const *>(this); }
};

struct safe_VkComputePipelineCreateInfo {
    VkStructureType sType;
    const void* pNext;
    VkPipelineCreateFlags flags;
    safe_VkPipelineShaderStageCreateInfo stage;
    VkPipelineLayout layout;
    VkPipeline basePipelineHandle;
    int32_t basePipelineIndex;
    safe_VkComputePipelineCreateInfo(const VkComputePipelineCreateInfo* in_struct);
    safe_VkComputePipelineCreateInfo(const safe_VkComputePipelineCreateInfo& copy_src);
    safe_VkComputePipelineCreateInfo& operator=(const safe_VkComputePipelineCreateInfo& copy_src);
    safe_VkComputePipelineCreateInfo();
    ~safe_VkComputePipelineCreateInfo();
    void initialize(const VkComputePipelineCreateInfo* in_struct);
    void initialize(const safe_VkComputePipelineCreateInfo* copy_src);
    VkComputePipelineCreateInfo *ptr() { return reinterpret_cast<VkComputePipelineCreateInfo *>(this); }
    VkComputePipelineCreateInfo const *ptr() const { return reinterpret_cast<VkComputePipelineCreateInfo const *>(this); }
};

struct safe_VkPipelineVertexInputStateCreateInfo {
    VkStructureType sType;
    const void* pNext;
    VkPipelineVertexInputStateCreateFlags flags;
    uint32_t vertexBindingDescriptionCount;
    const VkVertexInputBindingDescription* pVertexBindingDescriptions;
    uint32_t vertexAttributeDescriptionCount;
    const VkVertexInputAttributeDescription* pVertexAttributeDescriptions;
    safe_VkPipelineVertexInputStateCreateInfo(const VkPipelineVertexInputStateCreateInfo* in_struct);
    safe_VkPipelineVertexInputStateCreateInfo(const safe_VkPipelineVertexInputStateCreateInfo& copy_src);
    safe_VkPipelineVertexInputStateCreateInfo& operator=(const safe_VkPipelineVertexInputStateCreateInfo& copy_src);
    safe_VkPipelineVertexInputStateCreateInfo();
    ~safe_VkPipelineVertexInputStateCreateInfo();
    void initialize(const VkPipelineVertexInputStateCreateInfo* in_struct);
    void initialize(const safe_VkPipelineVertexInputStateCreateInfo* copy_src);
    VkPipelineVertexInputStateCreateInfo *ptr() { return reinterpret_cast<VkPipelineVertexInputStateCreateInfo *>(this); }
    VkPipelineVertexInputStateCreateInfo const *ptr() const { return reinterpret_cast<VkPipelineVertexInputStateCreateInfo const *>(this); }
};

struct safe_VkPipelineInputAssemblyStateCreateInfo {
    VkStructureType sType;
    const void* pNext;
    VkPipelineInputAssemblyStateCreateFlags flags;
    VkPrimitiveTopology topology;
    VkBool32 primitiveRestartEnable;
    safe_VkPipelineInputAssemblyStateCreateInfo(const VkPipelineInputAssemblyStateCreateInfo* in_struct);
    safe_VkPipelineInputAssemblyStateCreateInfo(const safe_VkPipelineInputAssemblyStateCreateInfo& copy_src);
    safe_VkPipelineInputAssemblyStateCreateInfo& operator=(const safe_VkPipelineInputAssemblyStateCreateInfo& copy_src);
    safe_VkPipelineInputAssemblyStateCreateInfo();
    ~safe_VkPipelineInputAssemblyStateCreateInfo();
    void initialize(const VkPipelineInputAssemblyStateCreateInfo* in_struct);
    void initialize(const safe_VkPipelineInputAssemblyStateCreateInfo* copy_src);
    VkPipelineInputAssemblyStateCreateInfo *ptr() { return reinterpret_cast<VkPipelineInputAssemblyStateCreateInfo *>(this); }
    VkPipelineInputAssemblyStateCreateInfo const *ptr() const { return reinterpret_cast<VkPipelineInputAssemblyStateCreateInfo const *>(this); }
};

struct safe_VkPipelineTessellationStateCreateInfo {
    VkStructureType sType;
    const void* pNext;
    VkPipelineTessellationStateCreateFlags flags;
    uint32_t patchControlPoints;
    safe_VkPipelineTessellationStateCreateInfo(const VkPipelineTessellationStateCreateInfo* in_struct);
    safe_VkPipelineTessellationStateCreateInfo(const safe_VkPipelineTessellationStateCreateInfo& copy_src);
    safe_VkPipelineTessellationStateCreateInfo& operator=(const safe_VkPipelineTessellationStateCreateInfo& copy_src);
    safe_VkPipelineTessellationStateCreateInfo();
    ~safe_VkPipelineTessellationStateCreateInfo();
    void initialize(const VkPipelineTessellationStateCreateInfo* in_struct);
    void initialize(const safe_VkPipelineTessellationStateCreateInfo* copy_src);
    VkPipelineTessellationStateCreateInfo *ptr() { return reinterpret_cast<VkPipelineTessellationStateCreateInfo *>(this); }
    VkPipelineTessellationStateCreateInfo const *ptr() const { return reinterpret_cast<VkPipelineTessellationStateCreateInfo const *>(this); }
};

struct safe_VkPipelineViewportStateCreateInfo {
    VkStructureType sType;
    const void* pNext;
    VkPipelineViewportStateCreateFlags flags;
    uint32_t viewportCount;
    const VkViewport* pViewports;
    uint32_t scissorCount;
    const VkRect2D* pScissors;
    safe_VkPipelineViewportStateCreateInfo(const VkPipelineViewportStateCreateInfo* in_struct, const bool is_dynamic_viewports, const bool is_dynamic_scissors);
    safe_VkPipelineViewportStateCreateInfo(const safe_VkPipelineViewportStateCreateInfo& copy_src);
    safe_VkPipelineViewportStateCreateInfo& operator=(const safe_VkPipelineViewportStateCreateInfo& copy_src);
    safe_VkPipelineViewportStateCreateInfo();
    ~safe_VkPipelineViewportStateCreateInfo();
    void initialize(const VkPipelineViewportStateCreateInfo* in_struct, const bool is_dynamic_viewports, const bool is_dynamic_scissors);
    void initialize(const safe_VkPipelineViewportStateCreateInfo* copy_src);
    VkPipelineViewportStateCreateInfo *ptr() { return reinterpret_cast<VkPipelineViewportStateCreateInfo *>(this); }
    VkPipelineViewportStateCreateInfo const *ptr() const { return reinterpret_cast<VkPipelineViewportStateCreateInfo const *>(this); }
};

struct safe_VkPipelineRasterizationStateCreateInfo {
    VkStructureType sType;
    const void* pNext;
    VkPipelineRasterizationStateCreateFlags flags;
    VkBool32 depthClampEnable;
    VkBool32 rasterizerDiscardEnable;
    VkPolygonMode polygonMode;
    VkCullModeFlags cullMode;
    VkFrontFace frontFace;
    VkBool32 depthBiasEnable;
    float depthBiasConstantFactor;
    float depthBiasClamp;
    float depthBiasSlopeFactor;
    float lineWidth;
    safe_VkPipelineRasterizationStateCreateInfo(const VkPipelineRasterizationStateCreateInfo* in_struct);
    safe_VkPipelineRasterizationStateCreateInfo(const safe_VkPipelineRasterizationStateCreateInfo& copy_src);
    safe_VkPipelineRasterizationStateCreateInfo& operator=(const safe_VkPipelineRasterizationStateCreateInfo& copy_src);
    safe_VkPipelineRasterizationStateCreateInfo();
    ~safe_VkPipelineRasterizationStateCreateInfo();
    void initialize(const VkPipelineRasterizationStateCreateInfo* in_struct);
    void initialize(const safe_VkPipelineRasterizationStateCreateInfo* copy_src);
    VkPipelineRasterizationStateCreateInfo *ptr() { return reinterpret_cast<VkPipelineRasterizationStateCreateInfo *>(this); }
    VkPipelineRasterizationStateCreateInfo const *ptr() const { return reinterpret_cast<VkPipelineRasterizationStateCreateInfo const *>(this); }
};

struct safe_VkPipelineMultisampleStateCreateInfo {
    VkStructureType sType;
    const void* pNext;
    VkPipelineMultisampleStateCreateFlags flags;
    VkSampleCountFlagBits rasterizationSamples;
    VkBool32 sampleShadingEnable;
    float minSampleShading;
    const VkSampleMask* pSampleMask;
    VkBool32 alphaToCoverageEnable;
    VkBool32 alphaToOneEnable;
    safe_VkPipelineMultisampleStateCreateInfo(const VkPipelineMultisampleStateCreateInfo* in_struct);
    safe_VkPipelineMultisampleStateCreateInfo(const safe_VkPipelineMultisampleStateCreateInfo& copy_src);
    safe_VkPipelineMultisampleStateCreateInfo& operator=(const safe_VkPipelineMultisampleStateCreateInfo& copy_src);
    safe_VkPipelineMultisampleStateCreateInfo();
    ~safe_VkPipelineMultisampleStateCreateInfo();
    void initialize(const VkPipelineMultisampleStateCreateInfo* in_struct);
    void initialize(const safe_VkPipelineMultisampleStateCreateInfo* copy_src);
    VkPipelineMultisampleStateCreateInfo *ptr() { return reinterpret_cast<VkPipelineMultisampleStateCreateInfo *>(this); }
    VkPipelineMultisampleStateCreateInfo const *ptr() const { return reinterpret_cast<VkPipelineMultisampleStateCreateInfo const *>(this); }
};

struct safe_VkPipelineDepthStencilStateCreateInfo {
    VkStructureType sType;
    const void* pNext;
    VkPipelineDepthStencilStateCreateFlags flags;
    VkBool32 depthTestEnable;
    VkBool32 depthWriteEnable;
    VkCompareOp depthCompareOp;
    VkBool32 depthBoundsTestEnable;
    VkBool32 stencilTestEnable;
    VkStencilOpState front;
    VkStencilOpState back;
    float minDepthBounds;
    float maxDepthBounds;
    safe_VkPipelineDepthStencilStateCreateInfo(const VkPipelineDepthStencilStateCreateInfo* in_struct);
    safe_VkPipelineDepthStencilStateCreateInfo(const safe_VkPipelineDepthStencilStateCreateInfo& copy_src);
    safe_VkPipelineDepthStencilStateCreateInfo& operator=(const safe_VkPipelineDepthStencilStateCreateInfo& copy_src);
    safe_VkPipelineDepthStencilStateCreateInfo();
    ~safe_VkPipelineDepthStencilStateCreateInfo();
    void initialize(const VkPipelineDepthStencilStateCreateInfo* in_struct);
    void initialize(const safe_VkPipelineDepthStencilStateCreateInfo* copy_src);
    VkPipelineDepthStencilStateCreateInfo *ptr() { return reinterpret_cast<VkPipelineDepthStencilStateCreateInfo *>(this); }
    VkPipelineDepthStencilStateCreateInfo const *ptr() const { return reinterpret_cast<VkPipelineDepthStencilStateCreateInfo const *>(this); }
};

struct safe_VkPipelineColorBlendStateCreateInfo {
    VkStructureType sType;
    const void* pNext;
    VkPipelineColorBlendStateCreateFlags flags;
    VkBool32 logicOpEnable;
    VkLogicOp logicOp;
    uint32_t attachmentCount;
    const VkPipelineColorBlendAttachmentState* pAttachments;
    float blendConstants[4];
    safe_VkPipelineColorBlendStateCreateInfo(const VkPipelineColorBlendStateCreateInfo* in_struct);
    safe_VkPipelineColorBlendStateCreateInfo(const safe_VkPipelineColorBlendStateCreateInfo& copy_src);
    safe_VkPipelineColorBlendStateCreateInfo& operator=(const safe_VkPipelineColorBlendStateCreateInfo& copy_src);
    safe_VkPipelineColorBlendStateCreateInfo();
    ~safe_VkPipelineColorBlendStateCreateInfo();
    void initialize(const VkPipelineColorBlendStateCreateInfo* in_struct);
    void initialize(const safe_VkPipelineColorBlendStateCreateInfo* copy_src);
    VkPipelineColorBlendStateCreateInfo *ptr() { return reinterpret_cast<VkPipelineColorBlendStateCreateInfo *>(this); }
    VkPipelineColorBlendStateCreateInfo const *ptr() const { return reinterpret_cast<VkPipelineColorBlendStateCreateInfo const *>(this); }
};

struct safe_VkPipelineDynamicStateCreateInfo {
    VkStructureType sType;
    const void* pNext;
    VkPipelineDynamicStateCreateFlags flags;
    uint32_t dynamicStateCount;
    const VkDynamicState* pDynamicStates;
    safe_VkPipelineDynamicStateCreateInfo(const VkPipelineDynamicStateCreateInfo* in_struct);
    safe_VkPipelineDynamicStateCreateInfo(const safe_VkPipelineDynamicStateCreateInfo& copy_src);
    safe_VkPipelineDynamicStateCreateInfo& operator=(const safe_VkPipelineDynamicStateCreateInfo& copy_src);
    safe_VkPipelineDynamicStateCreateInfo();
    ~safe_VkPipelineDynamicStateCreateInfo();
    void initialize(const VkPipelineDynamicStateCreateInfo* in_struct);
    void initialize(const safe_VkPipelineDynamicStateCreateInfo* copy_src);
    VkPipelineDynamicStateCreateInfo *ptr() { return reinterpret_cast<VkPipelineDynamicStateCreateInfo *>(this); }
    VkPipelineDynamicStateCreateInfo const *ptr() const { return reinterpret_cast<VkPipelineDynamicStateCreateInfo const *>(this); }
};

struct safe_VkGraphicsPipelineCreateInfo {
    VkStructureType sType;
    const void* pNext;
    VkPipelineCreateFlags flags;
    uint32_t stageCount;
    safe_VkPipelineShaderStageCreateInfo* pStages;
    safe_VkPipelineVertexInputStateCreateInfo* pVertexInputState;
    safe_VkPipelineInputAssemblyStateCreateInfo* pInputAssemblyState;
    safe_VkPipelineTessellationStateCreateInfo* pTessellationState;
    safe_VkPipelineViewportStateCreateInfo* pViewportState;
    safe_VkPipelineRasterizationStateCreateInfo* pRasterizationState;
    safe_VkPipelineMultisampleStateCreateInfo* pMultisampleState;
    safe_VkPipelineDepthStencilStateCreateInfo* pDepthStencilState;
    safe_VkPipelineColorBlendStateCreateInfo* pColorBlendState;
    safe_VkPipelineDynamicStateCreateInfo* pDynamicState;
    VkPipelineLayout layout;
    VkRenderPass renderPass;
    uint32_t subpass;
    VkPipeline basePipelineHandle;
    int32_t basePipelineIndex;
    safe_VkGraphicsPipelineCreateInfo(const VkGraphicsPipelineCreateInfo* in_struct, const bool uses_color_attachment, const bool uses_depthstencil_attachment);
    safe_VkGraphicsPipelineCreateInfo(const safe_VkGraphicsPipelineCreateInfo& copy_src);
    safe_VkGraphicsPipelineCreateInfo& operator=(const safe_VkGraphicsPipelineCreateInfo& copy_src);
    safe_VkGraphicsPipelineCreateInfo();
    ~safe_VkGraphicsPipelineCreateInfo();
    void initialize(const VkGraphicsPipelineCreateInfo* in_struct, const bool uses_color_attachment, const bool uses_depthstencil_attachment);
    void initialize(const safe_VkGraphicsPipelineCreateInfo* copy_src);
    VkGraphicsPipelineCreateInfo *ptr() { return reinterpret_cast<VkGraphicsPipelineCreateInfo *>(this); }
    VkGraphicsPipelineCreateInfo const *ptr() const { return reinterpret_cast<VkGraphicsPipelineCreateInfo const *>(this); }
};

struct safe_VkPipelineLayoutCreateInfo {
    VkStructureType sType;
    const void* pNext;
    VkPipelineLayoutCreateFlags flags;
    uint32_t setLayoutCount;
    VkDescriptorSetLayout* pSetLayouts;
    uint32_t pushConstantRangeCount;
    const VkPushConstantRange* pPushConstantRanges;
    safe_VkPipelineLayoutCreateInfo(const VkPipelineLayoutCreateInfo* in_struct);
    safe_VkPipelineLayoutCreateInfo(const safe_VkPipelineLayoutCreateInfo& copy_src);
    safe_VkPipelineLayoutCreateInfo& operator=(const safe_VkPipelineLayoutCreateInfo& copy_src);
    safe_VkPipelineLayoutCreateInfo();
    ~safe_VkPipelineLayoutCreateInfo();
    void initialize(const VkPipelineLayoutCreateInfo* in_struct);
    void initialize(const safe_VkPipelineLayoutCreateInfo* copy_src);
    VkPipelineLayoutCreateInfo *ptr() { return reinterpret_cast<VkPipelineLayoutCreateInfo *>(this); }
    VkPipelineLayoutCreateInfo const *ptr() const { return reinterpret_cast<VkPipelineLayoutCreateInfo const *>(this); }
};

struct safe_VkSamplerCreateInfo {
    VkStructureType sType;
    const void* pNext;
    VkSamplerCreateFlags flags;
    VkFilter magFilter;
    VkFilter minFilter;
    VkSamplerMipmapMode mipmapMode;
    VkSamplerAddressMode addressModeU;
    VkSamplerAddressMode addressModeV;
    VkSamplerAddressMode addressModeW;
    float mipLodBias;
    VkBool32 anisotropyEnable;
    float maxAnisotropy;
    VkBool32 compareEnable;
    VkCompareOp compareOp;
    float minLod;
    float maxLod;
    VkBorderColor borderColor;
    VkBool32 unnormalizedCoordinates;
    safe_VkSamplerCreateInfo(const VkSamplerCreateInfo* in_struct);
    safe_VkSamplerCreateInfo(const safe_VkSamplerCreateInfo& copy_src);
    safe_VkSamplerCreateInfo& operator=(const safe_VkSamplerCreateInfo& copy_src);
    safe_VkSamplerCreateInfo();
    ~safe_VkSamplerCreateInfo();
    void initialize(const VkSamplerCreateInfo* in_struct);
    void initialize(const safe_VkSamplerCreateInfo* copy_src);
    VkSamplerCreateInfo *ptr() { return reinterpret_cast<VkSamplerCreateInfo *>(this); }
    VkSamplerCreateInfo const *ptr() const { return reinterpret_cast<VkSamplerCreateInfo const *>(this); }
};

struct safe_VkCopyDescriptorSet {
    VkStructureType sType;
    const void* pNext;
    VkDescriptorSet srcSet;
    uint32_t srcBinding;
    uint32_t srcArrayElement;
    VkDescriptorSet dstSet;
    uint32_t dstBinding;
    uint32_t dstArrayElement;
    uint32_t descriptorCount;
    safe_VkCopyDescriptorSet(const VkCopyDescriptorSet* in_struct);
    safe_VkCopyDescriptorSet(const safe_VkCopyDescriptorSet& copy_src);
    safe_VkCopyDescriptorSet& operator=(const safe_VkCopyDescriptorSet& copy_src);
    safe_VkCopyDescriptorSet();
    ~safe_VkCopyDescriptorSet();
    void initialize(const VkCopyDescriptorSet* in_struct);
    void initialize(const safe_VkCopyDescriptorSet* copy_src);
    VkCopyDescriptorSet *ptr() { return reinterpret_cast<VkCopyDescriptorSet *>(this); }
    VkCopyDescriptorSet const *ptr() const { return reinterpret_cast<VkCopyDescriptorSet const *>(this); }
};

struct safe_VkDescriptorPoolCreateInfo {
    VkStructureType sType;
    const void* pNext;
    VkDescriptorPoolCreateFlags flags;
    uint32_t maxSets;
    uint32_t poolSizeCount;
    const VkDescriptorPoolSize* pPoolSizes;
    safe_VkDescriptorPoolCreateInfo(const VkDescriptorPoolCreateInfo* in_struct);
    safe_VkDescriptorPoolCreateInfo(const safe_VkDescriptorPoolCreateInfo& copy_src);
    safe_VkDescriptorPoolCreateInfo& operator=(const safe_VkDescriptorPoolCreateInfo& copy_src);
    safe_VkDescriptorPoolCreateInfo();
    ~safe_VkDescriptorPoolCreateInfo();
    void initialize(const VkDescriptorPoolCreateInfo* in_struct);
    void initialize(const safe_VkDescriptorPoolCreateInfo* copy_src);
    VkDescriptorPoolCreateInfo *ptr() { return reinterpret_cast<VkDescriptorPoolCreateInfo *>(this); }
    VkDescriptorPoolCreateInfo const *ptr() const { return reinterpret_cast<VkDescriptorPoolCreateInfo const *>(this); }
};

struct safe_VkDescriptorSetAllocateInfo {
    VkStructureType sType;
    const void* pNext;
    VkDescriptorPool descriptorPool;
    uint32_t descriptorSetCount;
    VkDescriptorSetLayout* pSetLayouts;
    safe_VkDescriptorSetAllocateInfo(const VkDescriptorSetAllocateInfo* in_struct);
    safe_VkDescriptorSetAllocateInfo(const safe_VkDescriptorSetAllocateInfo& copy_src);
    safe_VkDescriptorSetAllocateInfo& operator=(const safe_VkDescriptorSetAllocateInfo& copy_src);
    safe_VkDescriptorSetAllocateInfo();
    ~safe_VkDescriptorSetAllocateInfo();
    void initialize(const VkDescriptorSetAllocateInfo* in_struct);
    void initialize(const safe_VkDescriptorSetAllocateInfo* copy_src);
    VkDescriptorSetAllocateInfo *ptr() { return reinterpret_cast<VkDescriptorSetAllocateInfo *>(this); }
    VkDescriptorSetAllocateInfo const *ptr() const { return reinterpret_cast<VkDescriptorSetAllocateInfo const *>(this); }
};

struct safe_VkDescriptorSetLayoutBinding {
    uint32_t binding;
    VkDescriptorType descriptorType;
    uint32_t descriptorCount;
    VkShaderStageFlags stageFlags;
    VkSampler* pImmutableSamplers;
    safe_VkDescriptorSetLayoutBinding(const VkDescriptorSetLayoutBinding* in_struct);
    safe_VkDescriptorSetLayoutBinding(const safe_VkDescriptorSetLayoutBinding& copy_src);
    safe_VkDescriptorSetLayoutBinding& operator=(const safe_VkDescriptorSetLayoutBinding& copy_src);
    safe_VkDescriptorSetLayoutBinding();
    ~safe_VkDescriptorSetLayoutBinding();
    void initialize(const VkDescriptorSetLayoutBinding* in_struct);
    void initialize(const safe_VkDescriptorSetLayoutBinding* copy_src);
    VkDescriptorSetLayoutBinding *ptr() { return reinterpret_cast<VkDescriptorSetLayoutBinding *>(this); }
    VkDescriptorSetLayoutBinding const *ptr() const { return reinterpret_cast<VkDescriptorSetLayoutBinding const *>(this); }
};

struct safe_VkDescriptorSetLayoutCreateInfo {
    VkStructureType sType;
    const void* pNext;
    VkDescriptorSetLayoutCreateFlags flags;
    uint32_t bindingCount;
    safe_VkDescriptorSetLayoutBinding* pBindings;
    safe_VkDescriptorSetLayoutCreateInfo(const VkDescriptorSetLayoutCreateInfo* in_struct);
    safe_VkDescriptorSetLayoutCreateInfo(const safe_VkDescriptorSetLayoutCreateInfo& copy_src);
    safe_VkDescriptorSetLayoutCreateInfo& operator=(const safe_VkDescriptorSetLayoutCreateInfo& copy_src);
    safe_VkDescriptorSetLayoutCreateInfo();
    ~safe_VkDescriptorSetLayoutCreateInfo();
    void initialize(const VkDescriptorSetLayoutCreateInfo* in_struct);
    void initialize(const safe_VkDescriptorSetLayoutCreateInfo* copy_src);
    VkDescriptorSetLayoutCreateInfo *ptr() { return reinterpret_cast<VkDescriptorSetLayoutCreateInfo *>(this); }
    VkDescriptorSetLayoutCreateInfo const *ptr() const { return reinterpret_cast<VkDescriptorSetLayoutCreateInfo const *>(this); }
};

struct safe_VkWriteDescriptorSet {
    VkStructureType sType;
    const void* pNext;
    VkDescriptorSet dstSet;
    uint32_t dstBinding;
    uint32_t dstArrayElement;
    uint32_t descriptorCount;
    VkDescriptorType descriptorType;
    VkDescriptorImageInfo* pImageInfo;
    VkDescriptorBufferInfo* pBufferInfo;
    VkBufferView* pTexelBufferView;
    safe_VkWriteDescriptorSet(const VkWriteDescriptorSet* in_struct);
    safe_VkWriteDescriptorSet(const safe_VkWriteDescriptorSet& copy_src);
    safe_VkWriteDescriptorSet& operator=(const safe_VkWriteDescriptorSet& copy_src);
    safe_VkWriteDescriptorSet();
    ~safe_VkWriteDescriptorSet();
    void initialize(const VkWriteDescriptorSet* in_struct);
    void initialize(const safe_VkWriteDescriptorSet* copy_src);
    VkWriteDescriptorSet *ptr() { return reinterpret_cast<VkWriteDescriptorSet *>(this); }
    VkWriteDescriptorSet const *ptr() const { return reinterpret_cast<VkWriteDescriptorSet const *>(this); }
};

struct safe_VkFramebufferCreateInfo {
    VkStructureType sType;
    const void* pNext;
    VkFramebufferCreateFlags flags;
    VkRenderPass renderPass;
    uint32_t attachmentCount;
    VkImageView* pAttachments;
    uint32_t width;
    uint32_t height;
    uint32_t layers;
    safe_VkFramebufferCreateInfo(const VkFramebufferCreateInfo* in_struct);
    safe_VkFramebufferCreateInfo(const safe_VkFramebufferCreateInfo& copy_src);
    safe_VkFramebufferCreateInfo& operator=(const safe_VkFramebufferCreateInfo& copy_src);
    safe_VkFramebufferCreateInfo();
    ~safe_VkFramebufferCreateInfo();
    void initialize(const VkFramebufferCreateInfo* in_struct);
    void initialize(const safe_VkFramebufferCreateInfo* copy_src);
    VkFramebufferCreateInfo *ptr() { return reinterpret_cast<VkFramebufferCreateInfo *>(this); }
    VkFramebufferCreateInfo const *ptr() const { return reinterpret_cast<VkFramebufferCreateInfo const *>(this); }
};

struct safe_VkSubpassDescription {
    VkSubpassDescriptionFlags flags;
    VkPipelineBindPoint pipelineBindPoint;
    uint32_t inputAttachmentCount;
    const VkAttachmentReference* pInputAttachments;
    uint32_t colorAttachmentCount;
    const VkAttachmentReference* pColorAttachments;
    const VkAttachmentReference* pResolveAttachments;
    const VkAttachmentReference* pDepthStencilAttachment;
    uint32_t preserveAttachmentCount;
    const uint32_t* pPreserveAttachments;
    safe_VkSubpassDescription(const VkSubpassDescription* in_struct);
    safe_VkSubpassDescription(const safe_VkSubpassDescription& copy_src);
    safe_VkSubpassDescription& operator=(const safe_VkSubpassDescription& copy_src);
    safe_VkSubpassDescription();
    ~safe_VkSubpassDescription();
    void initialize(const VkSubpassDescription* in_struct);
    void initialize(const safe_VkSubpassDescription* copy_src);
    VkSubpassDescription *ptr() { return reinterpret_cast<VkSubpassDescription *>(this); }
    VkSubpassDescription const *ptr() const { return reinterpret_cast<VkSubpassDescription const *>(this); }
};

struct safe_VkRenderPassCreateInfo {
    VkStructureType sType;
    const void* pNext;
    VkRenderPassCreateFlags flags;
    uint32_t attachmentCount;
    const VkAttachmentDescription* pAttachments;
    uint32_t subpassCount;
    safe_VkSubpassDescription* pSubpasses;
    uint32_t dependencyCount;
    const VkSubpassDependency* pDependencies;
    safe_VkRenderPassCreateInfo(const VkRenderPassCreateInfo* in_struct);
    safe_VkRenderPassCreateInfo(const safe_VkRenderPassCreateInfo& copy_src);
    safe_VkRenderPassCreateInfo& operator=(const safe_VkRenderPassCreateInfo& copy_src);
    safe_VkRenderPassCreateInfo();
    ~safe_VkRenderPassCreateInfo();
    void initialize(const VkRenderPassCreateInfo* in_struct);
    void initialize(const safe_VkRenderPassCreateInfo* copy_src);
    VkRenderPassCreateInfo *ptr() { return reinterpret_cast<VkRenderPassCreateInfo *>(this); }
    VkRenderPassCreateInfo const *ptr() const { return reinterpret_cast<VkRenderPassCreateInfo const *>(this); }
};

struct safe_VkCommandPoolCreateInfo {
    VkStructureType sType;
    const void* pNext;
    VkCommandPoolCreateFlags flags;
    uint32_t queueFamilyIndex;
    safe_VkCommandPoolCreateInfo(const VkCommandPoolCreateInfo* in_struct);
    safe_VkCommandPoolCreateInfo(const safe_VkCommandPoolCreateInfo& copy_src);
    safe_VkCommandPoolCreateInfo& operator=(const safe_VkCommandPoolCreateInfo& copy_src);
    safe_VkCommandPoolCreateInfo();
    ~safe_VkCommandPoolCreateInfo();
    void initialize(const VkCommandPoolCreateInfo* in_struct);
    void initialize(const safe_VkCommandPoolCreateInfo* copy_src);
    VkCommandPoolCreateInfo *ptr() { return reinterpret_cast<VkCommandPoolCreateInfo *>(this); }
    VkCommandPoolCreateInfo const *ptr() const { return reinterpret_cast<VkCommandPoolCreateInfo const *>(this); }
};

struct safe_VkCommandBufferAllocateInfo {
    VkStructureType sType;
    const void* pNext;
    VkCommandPool commandPool;
    VkCommandBufferLevel level;
    uint32_t commandBufferCount;
    safe_VkCommandBufferAllocateInfo(const VkCommandBufferAllocateInfo* in_struct);
    safe_VkCommandBufferAllocateInfo(const safe_VkCommandBufferAllocateInfo& copy_src);
    safe_VkCommandBufferAllocateInfo& operator=(const safe_VkCommandBufferAllocateInfo& copy_src);
    safe_VkCommandBufferAllocateInfo();
    ~safe_VkCommandBufferAllocateInfo();
    void initialize(const VkCommandBufferAllocateInfo* in_struct);
    void initialize(const safe_VkCommandBufferAllocateInfo* copy_src);
    VkCommandBufferAllocateInfo *ptr() { return reinterpret_cast<VkCommandBufferAllocateInfo *>(this); }
    VkCommandBufferAllocateInfo const *ptr() const { return reinterpret_cast<VkCommandBufferAllocateInfo const *>(this); }
};

struct safe_VkCommandBufferInheritanceInfo {
    VkStructureType sType;
    const void* pNext;
    VkRenderPass renderPass;
    uint32_t subpass;
    VkFramebuffer framebuffer;
    VkBool32 occlusionQueryEnable;
    VkQueryControlFlags queryFlags;
    VkQueryPipelineStatisticFlags pipelineStatistics;
    safe_VkCommandBufferInheritanceInfo(const VkCommandBufferInheritanceInfo* in_struct);
    safe_VkCommandBufferInheritanceInfo(const safe_VkCommandBufferInheritanceInfo& copy_src);
    safe_VkCommandBufferInheritanceInfo& operator=(const safe_VkCommandBufferInheritanceInfo& copy_src);
    safe_VkCommandBufferInheritanceInfo();
    ~safe_VkCommandBufferInheritanceInfo();
    void initialize(const VkCommandBufferInheritanceInfo* in_struct);
    void initialize(const safe_VkCommandBufferInheritanceInfo* copy_src);
    VkCommandBufferInheritanceInfo *ptr() { return reinterpret_cast<VkCommandBufferInheritanceInfo *>(this); }
    VkCommandBufferInheritanceInfo const *ptr() const { return reinterpret_cast<VkCommandBufferInheritanceInfo const *>(this); }
};

struct safe_VkCommandBufferBeginInfo {
    VkStructureType sType;
    const void* pNext;
    VkCommandBufferUsageFlags flags;
    safe_VkCommandBufferInheritanceInfo* pInheritanceInfo;
    safe_VkCommandBufferBeginInfo(const VkCommandBufferBeginInfo* in_struct);
    safe_VkCommandBufferBeginInfo(const safe_VkCommandBufferBeginInfo& copy_src);
    safe_VkCommandBufferBeginInfo& operator=(const safe_VkCommandBufferBeginInfo& copy_src);
    safe_VkCommandBufferBeginInfo();
    ~safe_VkCommandBufferBeginInfo();
    void initialize(const VkCommandBufferBeginInfo* in_struct);
    void initialize(const safe_VkCommandBufferBeginInfo* copy_src);
    VkCommandBufferBeginInfo *ptr() { return reinterpret_cast<VkCommandBufferBeginInfo *>(this); }
    VkCommandBufferBeginInfo const *ptr() const { return reinterpret_cast<VkCommandBufferBeginInfo const *>(this); }
};

struct safe_VkRenderPassBeginInfo {
    VkStructureType sType;
    const void* pNext;
    VkRenderPass renderPass;
    VkFramebuffer framebuffer;
    VkRect2D renderArea;
    uint32_t clearValueCount;
    const VkClearValue* pClearValues;
    safe_VkRenderPassBeginInfo(const VkRenderPassBeginInfo* in_struct);
    safe_VkRenderPassBeginInfo(const safe_VkRenderPassBeginInfo& copy_src);
    safe_VkRenderPassBeginInfo& operator=(const safe_VkRenderPassBeginInfo& copy_src);
    safe_VkRenderPassBeginInfo();
    ~safe_VkRenderPassBeginInfo();
    void initialize(const VkRenderPassBeginInfo* in_struct);
    void initialize(const safe_VkRenderPassBeginInfo* copy_src);
    VkRenderPassBeginInfo *ptr() { return reinterpret_cast<VkRenderPassBeginInfo *>(this); }
    VkRenderPassBeginInfo const *ptr() const { return reinterpret_cast<VkRenderPassBeginInfo const *>(this); }
};

struct safe_VkPhysicalDeviceSubgroupProperties {
    VkStructureType sType;
    void* pNext;
    uint32_t subgroupSize;
    VkShaderStageFlags supportedStages;
    VkSubgroupFeatureFlags supportedOperations;
    VkBool32 quadOperationsInAllStages;
    safe_VkPhysicalDeviceSubgroupProperties(const VkPhysicalDeviceSubgroupProperties* in_struct);
    safe_VkPhysicalDeviceSubgroupProperties(const safe_VkPhysicalDeviceSubgroupProperties& copy_src);
    safe_VkPhysicalDeviceSubgroupProperties& operator=(const safe_VkPhysicalDeviceSubgroupProperties& copy_src);
    safe_VkPhysicalDeviceSubgroupProperties();
    ~safe_VkPhysicalDeviceSubgroupProperties();
    void initialize(const VkPhysicalDeviceSubgroupProperties* in_struct);
    void initialize(const safe_VkPhysicalDeviceSubgroupProperties* copy_src);
    VkPhysicalDeviceSubgroupProperties *ptr() { return reinterpret_cast<VkPhysicalDeviceSubgroupProperties *>(this); }
    VkPhysicalDeviceSubgroupProperties const *ptr() const { return reinterpret_cast<VkPhysicalDeviceSubgroupProperties const *>(this); }
};

struct safe_VkBindBufferMemoryInfo {
    VkStructureType sType;
    const void* pNext;
    VkBuffer buffer;
    VkDeviceMemory memory;
    VkDeviceSize memoryOffset;
    safe_VkBindBufferMemoryInfo(const VkBindBufferMemoryInfo* in_struct);
    safe_VkBindBufferMemoryInfo(const safe_VkBindBufferMemoryInfo& copy_src);
    safe_VkBindBufferMemoryInfo& operator=(const safe_VkBindBufferMemoryInfo& copy_src);
    safe_VkBindBufferMemoryInfo();
    ~safe_VkBindBufferMemoryInfo();
    void initialize(const VkBindBufferMemoryInfo* in_struct);
    void initialize(const safe_VkBindBufferMemoryInfo* copy_src);
    VkBindBufferMemoryInfo *ptr() { return reinterpret_cast<VkBindBufferMemoryInfo *>(this); }
    VkBindBufferMemoryInfo const *ptr() const { return reinterpret_cast<VkBindBufferMemoryInfo const *>(this); }
};

struct safe_VkBindImageMemoryInfo {
    VkStructureType sType;
    const void* pNext;
    VkImage image;
    VkDeviceMemory memory;
    VkDeviceSize memoryOffset;
    safe_VkBindImageMemoryInfo(const VkBindImageMemoryInfo* in_struct);
    safe_VkBindImageMemoryInfo(const safe_VkBindImageMemoryInfo& copy_src);
    safe_VkBindImageMemoryInfo& operator=(const safe_VkBindImageMemoryInfo& copy_src);
    safe_VkBindImageMemoryInfo();
    ~safe_VkBindImageMemoryInfo();
    void initialize(const VkBindImageMemoryInfo* in_struct);
    void initialize(const safe_VkBindImageMemoryInfo* copy_src);
    VkBindImageMemoryInfo *ptr() { return reinterpret_cast<VkBindImageMemoryInfo *>(this); }
    VkBindImageMemoryInfo const *ptr() const { return reinterpret_cast<VkBindImageMemoryInfo const *>(this); }
};

struct safe_VkPhysicalDevice16BitStorageFeatures {
    VkStructureType sType;
    void* pNext;
    VkBool32 storageBuffer16BitAccess;
    VkBool32 uniformAndStorageBuffer16BitAccess;
    VkBool32 storagePushConstant16;
    VkBool32 storageInputOutput16;
    safe_VkPhysicalDevice16BitStorageFeatures(const VkPhysicalDevice16BitStorageFeatures* in_struct);
    safe_VkPhysicalDevice16BitStorageFeatures(const safe_VkPhysicalDevice16BitStorageFeatures& copy_src);
    safe_VkPhysicalDevice16BitStorageFeatures& operator=(const safe_VkPhysicalDevice16BitStorageFeatures& copy_src);
    safe_VkPhysicalDevice16BitStorageFeatures();
    ~safe_VkPhysicalDevice16BitStorageFeatures();
    void initialize(const VkPhysicalDevice16BitStorageFeatures* in_struct);
    void initialize(const safe_VkPhysicalDevice16BitStorageFeatures* copy_src);
    VkPhysicalDevice16BitStorageFeatures *ptr() { return reinterpret_cast<VkPhysicalDevice16BitStorageFeatures *>(this); }
    VkPhysicalDevice16BitStorageFeatures const *ptr() const { return reinterpret_cast<VkPhysicalDevice16BitStorageFeatures const *>(this); }
};

struct safe_VkMemoryDedicatedRequirements {
    VkStructureType sType;
    void* pNext;
    VkBool32 prefersDedicatedAllocation;
    VkBool32 requiresDedicatedAllocation;
    safe_VkMemoryDedicatedRequirements(const VkMemoryDedicatedRequirements* in_struct);
    safe_VkMemoryDedicatedRequirements(const safe_VkMemoryDedicatedRequirements& copy_src);
    safe_VkMemoryDedicatedRequirements& operator=(const safe_VkMemoryDedicatedRequirements& copy_src);
    safe_VkMemoryDedicatedRequirements();
    ~safe_VkMemoryDedicatedRequirements();
    void initialize(const VkMemoryDedicatedRequirements* in_struct);
    void initialize(const safe_VkMemoryDedicatedRequirements* copy_src);
    VkMemoryDedicatedRequirements *ptr() { return reinterpret_cast<VkMemoryDedicatedRequirements *>(this); }
    VkMemoryDedicatedRequirements const *ptr() const { return reinterpret_cast<VkMemoryDedicatedRequirements const *>(this); }
};

struct safe_VkMemoryDedicatedAllocateInfo {
    VkStructureType sType;
    const void* pNext;
    VkImage image;
    VkBuffer buffer;
    safe_VkMemoryDedicatedAllocateInfo(const VkMemoryDedicatedAllocateInfo* in_struct);
    safe_VkMemoryDedicatedAllocateInfo(const safe_VkMemoryDedicatedAllocateInfo& copy_src);
    safe_VkMemoryDedicatedAllocateInfo& operator=(const safe_VkMemoryDedicatedAllocateInfo& copy_src);
    safe_VkMemoryDedicatedAllocateInfo();
    ~safe_VkMemoryDedicatedAllocateInfo();
    void initialize(const VkMemoryDedicatedAllocateInfo* in_struct);
    void initialize(const safe_VkMemoryDedicatedAllocateInfo* copy_src);
    VkMemoryDedicatedAllocateInfo *ptr() { return reinterpret_cast<VkMemoryDedicatedAllocateInfo *>(this); }
    VkMemoryDedicatedAllocateInfo const *ptr() const { return reinterpret_cast<VkMemoryDedicatedAllocateInfo const *>(this); }
};

struct safe_VkMemoryAllocateFlagsInfo {
    VkStructureType sType;
    const void* pNext;
    VkMemoryAllocateFlags flags;
    uint32_t deviceMask;
    safe_VkMemoryAllocateFlagsInfo(const VkMemoryAllocateFlagsInfo* in_struct);
    safe_VkMemoryAllocateFlagsInfo(const safe_VkMemoryAllocateFlagsInfo& copy_src);
    safe_VkMemoryAllocateFlagsInfo& operator=(const safe_VkMemoryAllocateFlagsInfo& copy_src);
    safe_VkMemoryAllocateFlagsInfo();
    ~safe_VkMemoryAllocateFlagsInfo();
    void initialize(const VkMemoryAllocateFlagsInfo* in_struct);
    void initialize(const safe_VkMemoryAllocateFlagsInfo* copy_src);
    VkMemoryAllocateFlagsInfo *ptr() { return reinterpret_cast<VkMemoryAllocateFlagsInfo *>(this); }
    VkMemoryAllocateFlagsInfo const *ptr() const { return reinterpret_cast<VkMemoryAllocateFlagsInfo const *>(this); }
};

struct safe_VkDeviceGroupRenderPassBeginInfo {
    VkStructureType sType;
    const void* pNext;
    uint32_t deviceMask;
    uint32_t deviceRenderAreaCount;
    const VkRect2D* pDeviceRenderAreas;
    safe_VkDeviceGroupRenderPassBeginInfo(const VkDeviceGroupRenderPassBeginInfo* in_struct);
    safe_VkDeviceGroupRenderPassBeginInfo(const safe_VkDeviceGroupRenderPassBeginInfo& copy_src);
    safe_VkDeviceGroupRenderPassBeginInfo& operator=(const safe_VkDeviceGroupRenderPassBeginInfo& copy_src);
    safe_VkDeviceGroupRenderPassBeginInfo();
    ~safe_VkDeviceGroupRenderPassBeginInfo();
    void initialize(const VkDeviceGroupRenderPassBeginInfo* in_struct);
    void initialize(const safe_VkDeviceGroupRenderPassBeginInfo* copy_src);
    VkDeviceGroupRenderPassBeginInfo *ptr() { return reinterpret_cast<VkDeviceGroupRenderPassBeginInfo *>(this); }
    VkDeviceGroupRenderPassBeginInfo const *ptr() const { return reinterpret_cast<VkDeviceGroupRenderPassBeginInfo const *>(this); }
};

struct safe_VkDeviceGroupCommandBufferBeginInfo {
    VkStructureType sType;
    const void* pNext;
    uint32_t deviceMask;
    safe_VkDeviceGroupCommandBufferBeginInfo(const VkDeviceGroupCommandBufferBeginInfo* in_struct);
    safe_VkDeviceGroupCommandBufferBeginInfo(const safe_VkDeviceGroupCommandBufferBeginInfo& copy_src);
    safe_VkDeviceGroupCommandBufferBeginInfo& operator=(const safe_VkDeviceGroupCommandBufferBeginInfo& copy_src);
    safe_VkDeviceGroupCommandBufferBeginInfo();
    ~safe_VkDeviceGroupCommandBufferBeginInfo();
    void initialize(const VkDeviceGroupCommandBufferBeginInfo* in_struct);
    void initialize(const safe_VkDeviceGroupCommandBufferBeginInfo* copy_src);
    VkDeviceGroupCommandBufferBeginInfo *ptr() { return reinterpret_cast<VkDeviceGroupCommandBufferBeginInfo *>(this); }
    VkDeviceGroupCommandBufferBeginInfo const *ptr() const { return reinterpret_cast<VkDeviceGroupCommandBufferBeginInfo const *>(this); }
};

struct safe_VkDeviceGroupSubmitInfo {
    VkStructureType sType;
    const void* pNext;
    uint32_t waitSemaphoreCount;
    const uint32_t* pWaitSemaphoreDeviceIndices;
    uint32_t commandBufferCount;
    const uint32_t* pCommandBufferDeviceMasks;
    uint32_t signalSemaphoreCount;
    const uint32_t* pSignalSemaphoreDeviceIndices;
    safe_VkDeviceGroupSubmitInfo(const VkDeviceGroupSubmitInfo* in_struct);
    safe_VkDeviceGroupSubmitInfo(const safe_VkDeviceGroupSubmitInfo& copy_src);
    safe_VkDeviceGroupSubmitInfo& operator=(const safe_VkDeviceGroupSubmitInfo& copy_src);
    safe_VkDeviceGroupSubmitInfo();
    ~safe_VkDeviceGroupSubmitInfo();
    void initialize(const VkDeviceGroupSubmitInfo* in_struct);
    void initialize(const safe_VkDeviceGroupSubmitInfo* copy_src);
    VkDeviceGroupSubmitInfo *ptr() { return reinterpret_cast<VkDeviceGroupSubmitInfo *>(this); }
    VkDeviceGroupSubmitInfo const *ptr() const { return reinterpret_cast<VkDeviceGroupSubmitInfo const *>(this); }
};

struct safe_VkBindBufferMemoryDeviceGroupInfo {
    VkStructureType sType;
    const void* pNext;
    uint32_t deviceIndexCount;
    const uint32_t* pDeviceIndices;
    safe_VkBindBufferMemoryDeviceGroupInfo(const VkBindBufferMemoryDeviceGroupInfo* in_struct);
    safe_VkBindBufferMemoryDeviceGroupInfo(const safe_VkBindBufferMemoryDeviceGroupInfo& copy_src);
    safe_VkBindBufferMemoryDeviceGroupInfo& operator=(const safe_VkBindBufferMemoryDeviceGroupInfo& copy_src);
    safe_VkBindBufferMemoryDeviceGroupInfo();
    ~safe_VkBindBufferMemoryDeviceGroupInfo();
    void initialize(const VkBindBufferMemoryDeviceGroupInfo* in_struct);
    void initialize(const safe_VkBindBufferMemoryDeviceGroupInfo* copy_src);
    VkBindBufferMemoryDeviceGroupInfo *ptr() { return reinterpret_cast<VkBindBufferMemoryDeviceGroupInfo *>(this); }
    VkBindBufferMemoryDeviceGroupInfo const *ptr() const { return reinterpret_cast<VkBindBufferMemoryDeviceGroupInfo const *>(this); }
};

struct safe_VkBindImageMemoryDeviceGroupInfo {
    VkStructureType sType;
    const void* pNext;
    uint32_t deviceIndexCount;
    const uint32_t* pDeviceIndices;
    uint32_t splitInstanceBindRegionCount;
    const VkRect2D* pSplitInstanceBindRegions;
    safe_VkBindImageMemoryDeviceGroupInfo(const VkBindImageMemoryDeviceGroupInfo* in_struct);
    safe_VkBindImageMemoryDeviceGroupInfo(const safe_VkBindImageMemoryDeviceGroupInfo& copy_src);
    safe_VkBindImageMemoryDeviceGroupInfo& operator=(const safe_VkBindImageMemoryDeviceGroupInfo& copy_src);
    safe_VkBindImageMemoryDeviceGroupInfo();
    ~safe_VkBindImageMemoryDeviceGroupInfo();
    void initialize(const VkBindImageMemoryDeviceGroupInfo* in_struct);
    void initialize(const safe_VkBindImageMemoryDeviceGroupInfo* copy_src);
    VkBindImageMemoryDeviceGroupInfo *ptr() { return reinterpret_cast<VkBindImageMemoryDeviceGroupInfo *>(this); }
    VkBindImageMemoryDeviceGroupInfo const *ptr() const { return reinterpret_cast<VkBindImageMemoryDeviceGroupInfo const *>(this); }
};

struct safe_VkPhysicalDeviceGroupProperties {
    VkStructureType sType;
    void* pNext;
    uint32_t physicalDeviceCount;
    VkPhysicalDevice physicalDevices[VK_MAX_DEVICE_GROUP_SIZE];
    VkBool32 subsetAllocation;
    safe_VkPhysicalDeviceGroupProperties(const VkPhysicalDeviceGroupProperties* in_struct);
    safe_VkPhysicalDeviceGroupProperties(const safe_VkPhysicalDeviceGroupProperties& copy_src);
    safe_VkPhysicalDeviceGroupProperties& operator=(const safe_VkPhysicalDeviceGroupProperties& copy_src);
    safe_VkPhysicalDeviceGroupProperties();
    ~safe_VkPhysicalDeviceGroupProperties();
    void initialize(const VkPhysicalDeviceGroupProperties* in_struct);
    void initialize(const safe_VkPhysicalDeviceGroupProperties* copy_src);
    VkPhysicalDeviceGroupProperties *ptr() { return reinterpret_cast<VkPhysicalDeviceGroupProperties *>(this); }
    VkPhysicalDeviceGroupProperties const *ptr() const { return reinterpret_cast<VkPhysicalDeviceGroupProperties const *>(this); }
};

struct safe_VkDeviceGroupDeviceCreateInfo {
    VkStructureType sType;
    const void* pNext;
    uint32_t physicalDeviceCount;
    VkPhysicalDevice* pPhysicalDevices;
    safe_VkDeviceGroupDeviceCreateInfo(const VkDeviceGroupDeviceCreateInfo* in_struct);
    safe_VkDeviceGroupDeviceCreateInfo(const safe_VkDeviceGroupDeviceCreateInfo& copy_src);
    safe_VkDeviceGroupDeviceCreateInfo& operator=(const safe_VkDeviceGroupDeviceCreateInfo& copy_src);
    safe_VkDeviceGroupDeviceCreateInfo();
    ~safe_VkDeviceGroupDeviceCreateInfo();
    void initialize(const VkDeviceGroupDeviceCreateInfo* in_struct);
    void initialize(const safe_VkDeviceGroupDeviceCreateInfo* copy_src);
    VkDeviceGroupDeviceCreateInfo *ptr() { return reinterpret_cast<VkDeviceGroupDeviceCreateInfo *>(this); }
    VkDeviceGroupDeviceCreateInfo const *ptr() const { return reinterpret_cast<VkDeviceGroupDeviceCreateInfo const *>(this); }
};

struct safe_VkBufferMemoryRequirementsInfo2 {
    VkStructureType sType;
    const void* pNext;
    VkBuffer buffer;
    safe_VkBufferMemoryRequirementsInfo2(const VkBufferMemoryRequirementsInfo2* in_struct);
    safe_VkBufferMemoryRequirementsInfo2(const safe_VkBufferMemoryRequirementsInfo2& copy_src);
    safe_VkBufferMemoryRequirementsInfo2& operator=(const safe_VkBufferMemoryRequirementsInfo2& copy_src);
    safe_VkBufferMemoryRequirementsInfo2();
    ~safe_VkBufferMemoryRequirementsInfo2();
    void initialize(const VkBufferMemoryRequirementsInfo2* in_struct);
    void initialize(const safe_VkBufferMemoryRequirementsInfo2* copy_src);
    VkBufferMemoryRequirementsInfo2 *ptr() { return reinterpret_cast<VkBufferMemoryRequirementsInfo2 *>(this); }
    VkBufferMemoryRequirementsInfo2 const *ptr() const { return reinterpret_cast<VkBufferMemoryRequirementsInfo2 const *>(this); }
};

struct safe_VkImageMemoryRequirementsInfo2 {
    VkStructureType sType;
    const void* pNext;
    VkImage image;
    safe_VkImageMemoryRequirementsInfo2(const VkImageMemoryRequirementsInfo2* in_struct);
    safe_VkImageMemoryRequirementsInfo2(const safe_VkImageMemoryRequirementsInfo2& copy_src);
    safe_VkImageMemoryRequirementsInfo2& operator=(const safe_VkImageMemoryRequirementsInfo2& copy_src);
    safe_VkImageMemoryRequirementsInfo2();
    ~safe_VkImageMemoryRequirementsInfo2();
    void initialize(const VkImageMemoryRequirementsInfo2* in_struct);
    void initialize(const safe_VkImageMemoryRequirementsInfo2* copy_src);
    VkImageMemoryRequirementsInfo2 *ptr() { return reinterpret_cast<VkImageMemoryRequirementsInfo2 *>(this); }
    VkImageMemoryRequirementsInfo2 const *ptr() const { return reinterpret_cast<VkImageMemoryRequirementsInfo2 const *>(this); }
};

struct safe_VkMemoryRequirements2 {
    VkStructureType sType;
    void* pNext;
    VkMemoryRequirements memoryRequirements;
    safe_VkMemoryRequirements2(const VkMemoryRequirements2* in_struct);
    safe_VkMemoryRequirements2(const safe_VkMemoryRequirements2& copy_src);
    safe_VkMemoryRequirements2& operator=(const safe_VkMemoryRequirements2& copy_src);
    safe_VkMemoryRequirements2();
    ~safe_VkMemoryRequirements2();
    void initialize(const VkMemoryRequirements2* in_struct);
    void initialize(const safe_VkMemoryRequirements2* copy_src);
    VkMemoryRequirements2 *ptr() { return reinterpret_cast<VkMemoryRequirements2 *>(this); }
    VkMemoryRequirements2 const *ptr() const { return reinterpret_cast<VkMemoryRequirements2 const *>(this); }
};

struct safe_VkPhysicalDeviceFeatures2 {
    VkStructureType sType;
    void* pNext;
    VkPhysicalDeviceFeatures features;
    safe_VkPhysicalDeviceFeatures2(const VkPhysicalDeviceFeatures2* in_struct);
    safe_VkPhysicalDeviceFeatures2(const safe_VkPhysicalDeviceFeatures2& copy_src);
    safe_VkPhysicalDeviceFeatures2& operator=(const safe_VkPhysicalDeviceFeatures2& copy_src);
    safe_VkPhysicalDeviceFeatures2();
    ~safe_VkPhysicalDeviceFeatures2();
    void initialize(const VkPhysicalDeviceFeatures2* in_struct);
    void initialize(const safe_VkPhysicalDeviceFeatures2* copy_src);
    VkPhysicalDeviceFeatures2 *ptr() { return reinterpret_cast<VkPhysicalDeviceFeatures2 *>(this); }
    VkPhysicalDeviceFeatures2 const *ptr() const { return reinterpret_cast<VkPhysicalDeviceFeatures2 const *>(this); }
};

struct safe_VkPhysicalDeviceProperties2 {
    VkStructureType sType;
    void* pNext;
    VkPhysicalDeviceProperties properties;
    safe_VkPhysicalDeviceProperties2(const VkPhysicalDeviceProperties2* in_struct);
    safe_VkPhysicalDeviceProperties2(const safe_VkPhysicalDeviceProperties2& copy_src);
    safe_VkPhysicalDeviceProperties2& operator=(const safe_VkPhysicalDeviceProperties2& copy_src);
    safe_VkPhysicalDeviceProperties2();
    ~safe_VkPhysicalDeviceProperties2();
    void initialize(const VkPhysicalDeviceProperties2* in_struct);
    void initialize(const safe_VkPhysicalDeviceProperties2* copy_src);
    VkPhysicalDeviceProperties2 *ptr() { return reinterpret_cast<VkPhysicalDeviceProperties2 *>(this); }
    VkPhysicalDeviceProperties2 const *ptr() const { return reinterpret_cast<VkPhysicalDeviceProperties2 const *>(this); }
};

struct safe_VkFormatProperties2 {
    VkStructureType sType;
    void* pNext;
    VkFormatProperties formatProperties;
    safe_VkFormatProperties2(const VkFormatProperties2* in_struct);
    safe_VkFormatProperties2(const safe_VkFormatProperties2& copy_src);
    safe_VkFormatProperties2& operator=(const safe_VkFormatProperties2& copy_src);
    safe_VkFormatProperties2();
    ~safe_VkFormatProperties2();
    void initialize(const VkFormatProperties2* in_struct);
    void initialize(const safe_VkFormatProperties2* copy_src);
    VkFormatProperties2 *ptr() { return reinterpret_cast<VkFormatProperties2 *>(this); }
    VkFormatProperties2 const *ptr() const { return reinterpret_cast<VkFormatProperties2 const *>(this); }
};

struct safe_VkImageFormatProperties2 {
    VkStructureType sType;
    void* pNext;
    VkImageFormatProperties imageFormatProperties;
    safe_VkImageFormatProperties2(const VkImageFormatProperties2* in_struct);
    safe_VkImageFormatProperties2(const safe_VkImageFormatProperties2& copy_src);
    safe_VkImageFormatProperties2& operator=(const safe_VkImageFormatProperties2& copy_src);
    safe_VkImageFormatProperties2();
    ~safe_VkImageFormatProperties2();
    void initialize(const VkImageFormatProperties2* in_struct);
    void initialize(const safe_VkImageFormatProperties2* copy_src);
    VkImageFormatProperties2 *ptr() { return reinterpret_cast<VkImageFormatProperties2 *>(this); }
    VkImageFormatProperties2 const *ptr() const { return reinterpret_cast<VkImageFormatProperties2 const *>(this); }
};

struct safe_VkPhysicalDeviceImageFormatInfo2 {
    VkStructureType sType;
    const void* pNext;
    VkFormat format;
    VkImageType type;
    VkImageTiling tiling;
    VkImageUsageFlags usage;
    VkImageCreateFlags flags;
    safe_VkPhysicalDeviceImageFormatInfo2(const VkPhysicalDeviceImageFormatInfo2* in_struct);
    safe_VkPhysicalDeviceImageFormatInfo2(const safe_VkPhysicalDeviceImageFormatInfo2& copy_src);
    safe_VkPhysicalDeviceImageFormatInfo2& operator=(const safe_VkPhysicalDeviceImageFormatInfo2& copy_src);
    safe_VkPhysicalDeviceImageFormatInfo2();
    ~safe_VkPhysicalDeviceImageFormatInfo2();
    void initialize(const VkPhysicalDeviceImageFormatInfo2* in_struct);
    void initialize(const safe_VkPhysicalDeviceImageFormatInfo2* copy_src);
    VkPhysicalDeviceImageFormatInfo2 *ptr() { return reinterpret_cast<VkPhysicalDeviceImageFormatInfo2 *>(this); }
    VkPhysicalDeviceImageFormatInfo2 const *ptr() const { return reinterpret_cast<VkPhysicalDeviceImageFormatInfo2 const *>(this); }
};

struct safe_VkQueueFamilyProperties2 {
    VkStructureType sType;
    void* pNext;
    VkQueueFamilyProperties queueFamilyProperties;
    safe_VkQueueFamilyProperties2(const VkQueueFamilyProperties2* in_struct);
    safe_VkQueueFamilyProperties2(const safe_VkQueueFamilyProperties2& copy_src);
    safe_VkQueueFamilyProperties2& operator=(const safe_VkQueueFamilyProperties2& copy_src);
    safe_VkQueueFamilyProperties2();
    ~safe_VkQueueFamilyProperties2();
    void initialize(const VkQueueFamilyProperties2* in_struct);
    void initialize(const safe_VkQueueFamilyProperties2* copy_src);
    VkQueueFamilyProperties2 *ptr() { return reinterpret_cast<VkQueueFamilyProperties2 *>(this); }
    VkQueueFamilyProperties2 const *ptr() const { return reinterpret_cast<VkQueueFamilyProperties2 const *>(this); }
};

struct safe_VkPhysicalDeviceMemoryProperties2 {
    VkStructureType sType;
    void* pNext;
    VkPhysicalDeviceMemoryProperties memoryProperties;
    safe_VkPhysicalDeviceMemoryProperties2(const VkPhysicalDeviceMemoryProperties2* in_struct);
    safe_VkPhysicalDeviceMemoryProperties2(const safe_VkPhysicalDeviceMemoryProperties2& copy_src);
    safe_VkPhysicalDeviceMemoryProperties2& operator=(const safe_VkPhysicalDeviceMemoryProperties2& copy_src);
    safe_VkPhysicalDeviceMemoryProperties2();
    ~safe_VkPhysicalDeviceMemoryProperties2();
    void initialize(const VkPhysicalDeviceMemoryProperties2* in_struct);
    void initialize(const safe_VkPhysicalDeviceMemoryProperties2* copy_src);
    VkPhysicalDeviceMemoryProperties2 *ptr() { return reinterpret_cast<VkPhysicalDeviceMemoryProperties2 *>(this); }
    VkPhysicalDeviceMemoryProperties2 const *ptr() const { return reinterpret_cast<VkPhysicalDeviceMemoryProperties2 const *>(this); }
};

struct safe_VkPhysicalDevicePointClippingProperties {
    VkStructureType sType;
    void* pNext;
    VkPointClippingBehavior pointClippingBehavior;
    safe_VkPhysicalDevicePointClippingProperties(const VkPhysicalDevicePointClippingProperties* in_struct);
    safe_VkPhysicalDevicePointClippingProperties(const safe_VkPhysicalDevicePointClippingProperties& copy_src);
    safe_VkPhysicalDevicePointClippingProperties& operator=(const safe_VkPhysicalDevicePointClippingProperties& copy_src);
    safe_VkPhysicalDevicePointClippingProperties();
    ~safe_VkPhysicalDevicePointClippingProperties();
    void initialize(const VkPhysicalDevicePointClippingProperties* in_struct);
    void initialize(const safe_VkPhysicalDevicePointClippingProperties* copy_src);
    VkPhysicalDevicePointClippingProperties *ptr() { return reinterpret_cast<VkPhysicalDevicePointClippingProperties *>(this); }
    VkPhysicalDevicePointClippingProperties const *ptr() const { return reinterpret_cast<VkPhysicalDevicePointClippingProperties const *>(this); }
};

struct safe_VkRenderPassInputAttachmentAspectCreateInfo {
    VkStructureType sType;
    const void* pNext;
    uint32_t aspectReferenceCount;
    const VkInputAttachmentAspectReference* pAspectReferences;
    safe_VkRenderPassInputAttachmentAspectCreateInfo(const VkRenderPassInputAttachmentAspectCreateInfo* in_struct);
    safe_VkRenderPassInputAttachmentAspectCreateInfo(const safe_VkRenderPassInputAttachmentAspectCreateInfo& copy_src);
    safe_VkRenderPassInputAttachmentAspectCreateInfo& operator=(const safe_VkRenderPassInputAttachmentAspectCreateInfo& copy_src);
    safe_VkRenderPassInputAttachmentAspectCreateInfo();
    ~safe_VkRenderPassInputAttachmentAspectCreateInfo();
    void initialize(const VkRenderPassInputAttachmentAspectCreateInfo* in_struct);
    void initialize(const safe_VkRenderPassInputAttachmentAspectCreateInfo* copy_src);
    VkRenderPassInputAttachmentAspectCreateInfo *ptr() { return reinterpret_cast<VkRenderPassInputAttachmentAspectCreateInfo *>(this); }
    VkRenderPassInputAttachmentAspectCreateInfo const *ptr() const { return reinterpret_cast<VkRenderPassInputAttachmentAspectCreateInfo const *>(this); }
};

struct safe_VkImageViewUsageCreateInfo {
    VkStructureType sType;
    const void* pNext;
    VkImageUsageFlags usage;
    safe_VkImageViewUsageCreateInfo(const VkImageViewUsageCreateInfo* in_struct);
    safe_VkImageViewUsageCreateInfo(const safe_VkImageViewUsageCreateInfo& copy_src);
    safe_VkImageViewUsageCreateInfo& operator=(const safe_VkImageViewUsageCreateInfo& copy_src);
    safe_VkImageViewUsageCreateInfo();
    ~safe_VkImageViewUsageCreateInfo();
    void initialize(const VkImageViewUsageCreateInfo* in_struct);
    void initialize(const safe_VkImageViewUsageCreateInfo* copy_src);
    VkImageViewUsageCreateInfo *ptr() { return reinterpret_cast<VkImageViewUsageCreateInfo *>(this); }
    VkImageViewUsageCreateInfo const *ptr() const { return reinterpret_cast<VkImageViewUsageCreateInfo const *>(this); }
};

struct safe_VkPipelineTessellationDomainOriginStateCreateInfo {
    VkStructureType sType;
    const void* pNext;
    VkTessellationDomainOrigin domainOrigin;
    safe_VkPipelineTessellationDomainOriginStateCreateInfo(const VkPipelineTessellationDomainOriginStateCreateInfo* in_struct);
    safe_VkPipelineTessellationDomainOriginStateCreateInfo(const safe_VkPipelineTessellationDomainOriginStateCreateInfo& copy_src);
    safe_VkPipelineTessellationDomainOriginStateCreateInfo& operator=(const safe_VkPipelineTessellationDomainOriginStateCreateInfo& copy_src);
    safe_VkPipelineTessellationDomainOriginStateCreateInfo();
    ~safe_VkPipelineTessellationDomainOriginStateCreateInfo();
    void initialize(const VkPipelineTessellationDomainOriginStateCreateInfo* in_struct);
    void initialize(const safe_VkPipelineTessellationDomainOriginStateCreateInfo* copy_src);
    VkPipelineTessellationDomainOriginStateCreateInfo *ptr() { return reinterpret_cast<VkPipelineTessellationDomainOriginStateCreateInfo *>(this); }
    VkPipelineTessellationDomainOriginStateCreateInfo const *ptr() const { return reinterpret_cast<VkPipelineTessellationDomainOriginStateCreateInfo const *>(this); }
};

struct safe_VkRenderPassMultiviewCreateInfo {
    VkStructureType sType;
    const void* pNext;
    uint32_t subpassCount;
    const uint32_t* pViewMasks;
    uint32_t dependencyCount;
    const int32_t* pViewOffsets;
    uint32_t correlationMaskCount;
    const uint32_t* pCorrelationMasks;
    safe_VkRenderPassMultiviewCreateInfo(const VkRenderPassMultiviewCreateInfo* in_struct);
    safe_VkRenderPassMultiviewCreateInfo(const safe_VkRenderPassMultiviewCreateInfo& copy_src);
    safe_VkRenderPassMultiviewCreateInfo& operator=(const safe_VkRenderPassMultiviewCreateInfo& copy_src);
    safe_VkRenderPassMultiviewCreateInfo();
    ~safe_VkRenderPassMultiviewCreateInfo();
    void initialize(const VkRenderPassMultiviewCreateInfo* in_struct);
    void initialize(const safe_VkRenderPassMultiviewCreateInfo* copy_src);
    VkRenderPassMultiviewCreateInfo *ptr() { return reinterpret_cast<VkRenderPassMultiviewCreateInfo *>(this); }
    VkRenderPassMultiviewCreateInfo const *ptr() const { return reinterpret_cast<VkRenderPassMultiviewCreateInfo const *>(this); }
};

struct safe_VkPhysicalDeviceMultiviewFeatures {
    VkStructureType sType;
    void* pNext;
    VkBool32 multiview;
    VkBool32 multiviewGeometryShader;
    VkBool32 multiviewTessellationShader;
    safe_VkPhysicalDeviceMultiviewFeatures(const VkPhysicalDeviceMultiviewFeatures* in_struct);
    safe_VkPhysicalDeviceMultiviewFeatures(const safe_VkPhysicalDeviceMultiviewFeatures& copy_src);
    safe_VkPhysicalDeviceMultiviewFeatures& operator=(const safe_VkPhysicalDeviceMultiviewFeatures& copy_src);
    safe_VkPhysicalDeviceMultiviewFeatures();
    ~safe_VkPhysicalDeviceMultiviewFeatures();
    void initialize(const VkPhysicalDeviceMultiviewFeatures* in_struct);
    void initialize(const safe_VkPhysicalDeviceMultiviewFeatures* copy_src);
    VkPhysicalDeviceMultiviewFeatures *ptr() { return reinterpret_cast<VkPhysicalDeviceMultiviewFeatures *>(this); }
    VkPhysicalDeviceMultiviewFeatures const *ptr() const { return reinterpret_cast<VkPhysicalDeviceMultiviewFeatures const *>(this); }
};

struct safe_VkPhysicalDeviceMultiviewProperties {
    VkStructureType sType;
    void* pNext;
    uint32_t maxMultiviewViewCount;
    uint32_t maxMultiviewInstanceIndex;
    safe_VkPhysicalDeviceMultiviewProperties(const VkPhysicalDeviceMultiviewProperties* in_struct);
    safe_VkPhysicalDeviceMultiviewProperties(const safe_VkPhysicalDeviceMultiviewProperties& copy_src);
    safe_VkPhysicalDeviceMultiviewProperties& operator=(const safe_VkPhysicalDeviceMultiviewProperties& copy_src);
    safe_VkPhysicalDeviceMultiviewProperties();
    ~safe_VkPhysicalDeviceMultiviewProperties();
    void initialize(const VkPhysicalDeviceMultiviewProperties* in_struct);
    void initialize(const safe_VkPhysicalDeviceMultiviewProperties* copy_src);
    VkPhysicalDeviceMultiviewProperties *ptr() { return reinterpret_cast<VkPhysicalDeviceMultiviewProperties *>(this); }
    VkPhysicalDeviceMultiviewProperties const *ptr() const { return reinterpret_cast<VkPhysicalDeviceMultiviewProperties const *>(this); }
};

struct safe_VkPhysicalDeviceVariablePointersFeatures {
    VkStructureType sType;
    void* pNext;
    VkBool32 variablePointersStorageBuffer;
    VkBool32 variablePointers;
    safe_VkPhysicalDeviceVariablePointersFeatures(const VkPhysicalDeviceVariablePointersFeatures* in_struct);
    safe_VkPhysicalDeviceVariablePointersFeatures(const safe_VkPhysicalDeviceVariablePointersFeatures& copy_src);
    safe_VkPhysicalDeviceVariablePointersFeatures& operator=(const safe_VkPhysicalDeviceVariablePointersFeatures& copy_src);
    safe_VkPhysicalDeviceVariablePointersFeatures();
    ~safe_VkPhysicalDeviceVariablePointersFeatures();
    void initialize(const VkPhysicalDeviceVariablePointersFeatures* in_struct);
    void initialize(const safe_VkPhysicalDeviceVariablePointersFeatures* copy_src);
    VkPhysicalDeviceVariablePointersFeatures *ptr() { return reinterpret_cast<VkPhysicalDeviceVariablePointersFeatures *>(this); }
    VkPhysicalDeviceVariablePointersFeatures const *ptr() const { return reinterpret_cast<VkPhysicalDeviceVariablePointersFeatures const *>(this); }
};

struct safe_VkPhysicalDeviceProtectedMemoryFeatures {
    VkStructureType sType;
    void* pNext;
    VkBool32 protectedMemory;
    safe_VkPhysicalDeviceProtectedMemoryFeatures(const VkPhysicalDeviceProtectedMemoryFeatures* in_struct);
    safe_VkPhysicalDeviceProtectedMemoryFeatures(const safe_VkPhysicalDeviceProtectedMemoryFeatures& copy_src);
    safe_VkPhysicalDeviceProtectedMemoryFeatures& operator=(const safe_VkPhysicalDeviceProtectedMemoryFeatures& copy_src);
    safe_VkPhysicalDeviceProtectedMemoryFeatures();
    ~safe_VkPhysicalDeviceProtectedMemoryFeatures();
    void initialize(const VkPhysicalDeviceProtectedMemoryFeatures* in_struct);
    void initialize(const safe_VkPhysicalDeviceProtectedMemoryFeatures* copy_src);
    VkPhysicalDeviceProtectedMemoryFeatures *ptr() { return reinterpret_cast<VkPhysicalDeviceProtectedMemoryFeatures *>(this); }
    VkPhysicalDeviceProtectedMemoryFeatures const *ptr() const { return reinterpret_cast<VkPhysicalDeviceProtectedMemoryFeatures const *>(this); }
};

struct safe_VkPhysicalDeviceProtectedMemoryProperties {
    VkStructureType sType;
    void* pNext;
    VkBool32 protectedNoFault;
    safe_VkPhysicalDeviceProtectedMemoryProperties(const VkPhysicalDeviceProtectedMemoryProperties* in_struct);
    safe_VkPhysicalDeviceProtectedMemoryProperties(const safe_VkPhysicalDeviceProtectedMemoryProperties& copy_src);
    safe_VkPhysicalDeviceProtectedMemoryProperties& operator=(const safe_VkPhysicalDeviceProtectedMemoryProperties& copy_src);
    safe_VkPhysicalDeviceProtectedMemoryProperties();
    ~safe_VkPhysicalDeviceProtectedMemoryProperties();
    void initialize(const VkPhysicalDeviceProtectedMemoryProperties* in_struct);
    void initialize(const safe_VkPhysicalDeviceProtectedMemoryProperties* copy_src);
    VkPhysicalDeviceProtectedMemoryProperties *ptr() { return reinterpret_cast<VkPhysicalDeviceProtectedMemoryProperties *>(this); }
    VkPhysicalDeviceProtectedMemoryProperties const *ptr() const { return reinterpret_cast<VkPhysicalDeviceProtectedMemoryProperties const *>(this); }
};

struct safe_VkDeviceQueueInfo2 {
    VkStructureType sType;
    const void* pNext;
    VkDeviceQueueCreateFlags flags;
    uint32_t queueFamilyIndex;
    uint32_t queueIndex;
    safe_VkDeviceQueueInfo2(const VkDeviceQueueInfo2* in_struct);
    safe_VkDeviceQueueInfo2(const safe_VkDeviceQueueInfo2& copy_src);
    safe_VkDeviceQueueInfo2& operator=(const safe_VkDeviceQueueInfo2& copy_src);
    safe_VkDeviceQueueInfo2();
    ~safe_VkDeviceQueueInfo2();
    void initialize(const VkDeviceQueueInfo2* in_struct);
    void initialize(const safe_VkDeviceQueueInfo2* copy_src);
    VkDeviceQueueInfo2 *ptr() { return reinterpret_cast<VkDeviceQueueInfo2 *>(this); }
    VkDeviceQueueInfo2 const *ptr() const { return reinterpret_cast<VkDeviceQueueInfo2 const *>(this); }
};

struct safe_VkProtectedSubmitInfo {
    VkStructureType sType;
    const void* pNext;
    VkBool32 protectedSubmit;
    safe_VkProtectedSubmitInfo(const VkProtectedSubmitInfo* in_struct);
    safe_VkProtectedSubmitInfo(const safe_VkProtectedSubmitInfo& copy_src);
    safe_VkProtectedSubmitInfo& operator=(const safe_VkProtectedSubmitInfo& copy_src);
    safe_VkProtectedSubmitInfo();
    ~safe_VkProtectedSubmitInfo();
    void initialize(const VkProtectedSubmitInfo* in_struct);
    void initialize(const safe_VkProtectedSubmitInfo* copy_src);
    VkProtectedSubmitInfo *ptr() { return reinterpret_cast<VkProtectedSubmitInfo *>(this); }
    VkProtectedSubmitInfo const *ptr() const { return reinterpret_cast<VkProtectedSubmitInfo const *>(this); }
};

struct safe_VkSamplerYcbcrConversionCreateInfo {
    VkStructureType sType;
    const void* pNext;
    VkFormat format;
    VkSamplerYcbcrModelConversion ycbcrModel;
    VkSamplerYcbcrRange ycbcrRange;
    VkComponentMapping components;
    VkChromaLocation xChromaOffset;
    VkChromaLocation yChromaOffset;
    VkFilter chromaFilter;
    VkBool32 forceExplicitReconstruction;
    safe_VkSamplerYcbcrConversionCreateInfo(const VkSamplerYcbcrConversionCreateInfo* in_struct);
    safe_VkSamplerYcbcrConversionCreateInfo(const safe_VkSamplerYcbcrConversionCreateInfo& copy_src);
    safe_VkSamplerYcbcrConversionCreateInfo& operator=(const safe_VkSamplerYcbcrConversionCreateInfo& copy_src);
    safe_VkSamplerYcbcrConversionCreateInfo();
    ~safe_VkSamplerYcbcrConversionCreateInfo();
    void initialize(const VkSamplerYcbcrConversionCreateInfo* in_struct);
    void initialize(const safe_VkSamplerYcbcrConversionCreateInfo* copy_src);
    VkSamplerYcbcrConversionCreateInfo *ptr() { return reinterpret_cast<VkSamplerYcbcrConversionCreateInfo *>(this); }
    VkSamplerYcbcrConversionCreateInfo const *ptr() const { return reinterpret_cast<VkSamplerYcbcrConversionCreateInfo const *>(this); }
};

struct safe_VkSamplerYcbcrConversionInfo {
    VkStructureType sType;
    const void* pNext;
    VkSamplerYcbcrConversion conversion;
    safe_VkSamplerYcbcrConversionInfo(const VkSamplerYcbcrConversionInfo* in_struct);
    safe_VkSamplerYcbcrConversionInfo(const safe_VkSamplerYcbcrConversionInfo& copy_src);
    safe_VkSamplerYcbcrConversionInfo& operator=(const safe_VkSamplerYcbcrConversionInfo& copy_src);
    safe_VkSamplerYcbcrConversionInfo();
    ~safe_VkSamplerYcbcrConversionInfo();
    void initialize(const VkSamplerYcbcrConversionInfo* in_struct);
    void initialize(const safe_VkSamplerYcbcrConversionInfo* copy_src);
    VkSamplerYcbcrConversionInfo *ptr() { return reinterpret_cast<VkSamplerYcbcrConversionInfo *>(this); }
    VkSamplerYcbcrConversionInfo const *ptr() const { return reinterpret_cast<VkSamplerYcbcrConversionInfo const *>(this); }
};

struct safe_VkBindImagePlaneMemoryInfo {
    VkStructureType sType;
    const void* pNext;
    VkImageAspectFlagBits planeAspect;
    safe_VkBindImagePlaneMemoryInfo(const VkBindImagePlaneMemoryInfo* in_struct);
    safe_VkBindImagePlaneMemoryInfo(const safe_VkBindImagePlaneMemoryInfo& copy_src);
    safe_VkBindImagePlaneMemoryInfo& operator=(const safe_VkBindImagePlaneMemoryInfo& copy_src);
    safe_VkBindImagePlaneMemoryInfo();
    ~safe_VkBindImagePlaneMemoryInfo();
    void initialize(const VkBindImagePlaneMemoryInfo* in_struct);
    void initialize(const safe_VkBindImagePlaneMemoryInfo* copy_src);
    VkBindImagePlaneMemoryInfo *ptr() { return reinterpret_cast<VkBindImagePlaneMemoryInfo *>(this); }
    VkBindImagePlaneMemoryInfo const *ptr() const { return reinterpret_cast<VkBindImagePlaneMemoryInfo const *>(this); }
};

struct safe_VkImagePlaneMemoryRequirementsInfo {
    VkStructureType sType;
    const void* pNext;
    VkImageAspectFlagBits planeAspect;
    safe_VkImagePlaneMemoryRequirementsInfo(const VkImagePlaneMemoryRequirementsInfo* in_struct);
    safe_VkImagePlaneMemoryRequirementsInfo(const safe_VkImagePlaneMemoryRequirementsInfo& copy_src);
    safe_VkImagePlaneMemoryRequirementsInfo& operator=(const safe_VkImagePlaneMemoryRequirementsInfo& copy_src);
    safe_VkImagePlaneMemoryRequirementsInfo();
    ~safe_VkImagePlaneMemoryRequirementsInfo();
    void initialize(const VkImagePlaneMemoryRequirementsInfo* in_struct);
    void initialize(const safe_VkImagePlaneMemoryRequirementsInfo* copy_src);
    VkImagePlaneMemoryRequirementsInfo *ptr() { return reinterpret_cast<VkImagePlaneMemoryRequirementsInfo *>(this); }
    VkImagePlaneMemoryRequirementsInfo const *ptr() const { return reinterpret_cast<VkImagePlaneMemoryRequirementsInfo const *>(this); }
};

struct safe_VkPhysicalDeviceSamplerYcbcrConversionFeatures {
    VkStructureType sType;
    void* pNext;
    VkBool32 samplerYcbcrConversion;
    safe_VkPhysicalDeviceSamplerYcbcrConversionFeatures(const VkPhysicalDeviceSamplerYcbcrConversionFeatures* in_struct);
    safe_VkPhysicalDeviceSamplerYcbcrConversionFeatures(const safe_VkPhysicalDeviceSamplerYcbcrConversionFeatures& copy_src);
    safe_VkPhysicalDeviceSamplerYcbcrConversionFeatures& operator=(const safe_VkPhysicalDeviceSamplerYcbcrConversionFeatures& copy_src);
    safe_VkPhysicalDeviceSamplerYcbcrConversionFeatures();
    ~safe_VkPhysicalDeviceSamplerYcbcrConversionFeatures();
    void initialize(const VkPhysicalDeviceSamplerYcbcrConversionFeatures* in_struct);
    void initialize(const safe_VkPhysicalDeviceSamplerYcbcrConversionFeatures* copy_src);
    VkPhysicalDeviceSamplerYcbcrConversionFeatures *ptr() { return reinterpret_cast<VkPhysicalDeviceSamplerYcbcrConversionFeatures *>(this); }
    VkPhysicalDeviceSamplerYcbcrConversionFeatures const *ptr() const { return reinterpret_cast<VkPhysicalDeviceSamplerYcbcrConversionFeatures const *>(this); }
};

struct safe_VkSamplerYcbcrConversionImageFormatProperties {
    VkStructureType sType;
    void* pNext;
    uint32_t combinedImageSamplerDescriptorCount;
    safe_VkSamplerYcbcrConversionImageFormatProperties(const VkSamplerYcbcrConversionImageFormatProperties* in_struct);
    safe_VkSamplerYcbcrConversionImageFormatProperties(const safe_VkSamplerYcbcrConversionImageFormatProperties& copy_src);
    safe_VkSamplerYcbcrConversionImageFormatProperties& operator=(const safe_VkSamplerYcbcrConversionImageFormatProperties& copy_src);
    safe_VkSamplerYcbcrConversionImageFormatProperties();
    ~safe_VkSamplerYcbcrConversionImageFormatProperties();
    void initialize(const VkSamplerYcbcrConversionImageFormatProperties* in_struct);
    void initialize(const safe_VkSamplerYcbcrConversionImageFormatProperties* copy_src);
    VkSamplerYcbcrConversionImageFormatProperties *ptr() { return reinterpret_cast<VkSamplerYcbcrConversionImageFormatProperties *>(this); }
    VkSamplerYcbcrConversionImageFormatProperties const *ptr() const { return reinterpret_cast<VkSamplerYcbcrConversionImageFormatProperties const *>(this); }
};

struct safe_VkPhysicalDeviceExternalImageFormatInfo {
    VkStructureType sType;
    const void* pNext;
    VkExternalMemoryHandleTypeFlagBits handleType;
    safe_VkPhysicalDeviceExternalImageFormatInfo(const VkPhysicalDeviceExternalImageFormatInfo* in_struct);
    safe_VkPhysicalDeviceExternalImageFormatInfo(const safe_VkPhysicalDeviceExternalImageFormatInfo& copy_src);
    safe_VkPhysicalDeviceExternalImageFormatInfo& operator=(const safe_VkPhysicalDeviceExternalImageFormatInfo& copy_src);
    safe_VkPhysicalDeviceExternalImageFormatInfo();
    ~safe_VkPhysicalDeviceExternalImageFormatInfo();
    void initialize(const VkPhysicalDeviceExternalImageFormatInfo* in_struct);
    void initialize(const safe_VkPhysicalDeviceExternalImageFormatInfo* copy_src);
    VkPhysicalDeviceExternalImageFormatInfo *ptr() { return reinterpret_cast<VkPhysicalDeviceExternalImageFormatInfo *>(this); }
    VkPhysicalDeviceExternalImageFormatInfo const *ptr() const { return reinterpret_cast<VkPhysicalDeviceExternalImageFormatInfo const *>(this); }
};

struct safe_VkExternalImageFormatProperties {
    VkStructureType sType;
    void* pNext;
    VkExternalMemoryProperties externalMemoryProperties;
    safe_VkExternalImageFormatProperties(const VkExternalImageFormatProperties* in_struct);
    safe_VkExternalImageFormatProperties(const safe_VkExternalImageFormatProperties& copy_src);
    safe_VkExternalImageFormatProperties& operator=(const safe_VkExternalImageFormatProperties& copy_src);
    safe_VkExternalImageFormatProperties();
    ~safe_VkExternalImageFormatProperties();
    void initialize(const VkExternalImageFormatProperties* in_struct);
    void initialize(const safe_VkExternalImageFormatProperties* copy_src);
    VkExternalImageFormatProperties *ptr() { return reinterpret_cast<VkExternalImageFormatProperties *>(this); }
    VkExternalImageFormatProperties const *ptr() const { return reinterpret_cast<VkExternalImageFormatProperties const *>(this); }
};

struct safe_VkPhysicalDeviceExternalBufferInfo {
    VkStructureType sType;
    const void* pNext;
    VkBufferCreateFlags flags;
    VkBufferUsageFlags usage;
    VkExternalMemoryHandleTypeFlagBits handleType;
    safe_VkPhysicalDeviceExternalBufferInfo(const VkPhysicalDeviceExternalBufferInfo* in_struct);
    safe_VkPhysicalDeviceExternalBufferInfo(const safe_VkPhysicalDeviceExternalBufferInfo& copy_src);
    safe_VkPhysicalDeviceExternalBufferInfo& operator=(const safe_VkPhysicalDeviceExternalBufferInfo& copy_src);
    safe_VkPhysicalDeviceExternalBufferInfo();
    ~safe_VkPhysicalDeviceExternalBufferInfo();
    void initialize(const VkPhysicalDeviceExternalBufferInfo* in_struct);
    void initialize(const safe_VkPhysicalDeviceExternalBufferInfo* copy_src);
    VkPhysicalDeviceExternalBufferInfo *ptr() { return reinterpret_cast<VkPhysicalDeviceExternalBufferInfo *>(this); }
    VkPhysicalDeviceExternalBufferInfo const *ptr() const { return reinterpret_cast<VkPhysicalDeviceExternalBufferInfo const *>(this); }
};

struct safe_VkExternalBufferProperties {
    VkStructureType sType;
    void* pNext;
    VkExternalMemoryProperties externalMemoryProperties;
    safe_VkExternalBufferProperties(const VkExternalBufferProperties* in_struct);
    safe_VkExternalBufferProperties(const safe_VkExternalBufferProperties& copy_src);
    safe_VkExternalBufferProperties& operator=(const safe_VkExternalBufferProperties& copy_src);
    safe_VkExternalBufferProperties();
    ~safe_VkExternalBufferProperties();
    void initialize(const VkExternalBufferProperties* in_struct);
    void initialize(const safe_VkExternalBufferProperties* copy_src);
    VkExternalBufferProperties *ptr() { return reinterpret_cast<VkExternalBufferProperties *>(this); }
    VkExternalBufferProperties const *ptr() const { return reinterpret_cast<VkExternalBufferProperties const *>(this); }
};

struct safe_VkPhysicalDeviceIDProperties {
    VkStructureType sType;
    void* pNext;
    uint8_t deviceUUID[VK_UUID_SIZE];
    uint8_t driverUUID[VK_UUID_SIZE];
    uint8_t deviceLUID[VK_LUID_SIZE];
    uint32_t deviceNodeMask;
    VkBool32 deviceLUIDValid;
    safe_VkPhysicalDeviceIDProperties(const VkPhysicalDeviceIDProperties* in_struct);
    safe_VkPhysicalDeviceIDProperties(const safe_VkPhysicalDeviceIDProperties& copy_src);
    safe_VkPhysicalDeviceIDProperties& operator=(const safe_VkPhysicalDeviceIDProperties& copy_src);
    safe_VkPhysicalDeviceIDProperties();
    ~safe_VkPhysicalDeviceIDProperties();
    void initialize(const VkPhysicalDeviceIDProperties* in_struct);
    void initialize(const safe_VkPhysicalDeviceIDProperties* copy_src);
    VkPhysicalDeviceIDProperties *ptr() { return reinterpret_cast<VkPhysicalDeviceIDProperties *>(this); }
    VkPhysicalDeviceIDProperties const *ptr() const { return reinterpret_cast<VkPhysicalDeviceIDProperties const *>(this); }
};

struct safe_VkExternalMemoryImageCreateInfo {
    VkStructureType sType;
    const void* pNext;
    VkExternalMemoryHandleTypeFlags handleTypes;
    safe_VkExternalMemoryImageCreateInfo(const VkExternalMemoryImageCreateInfo* in_struct);
    safe_VkExternalMemoryImageCreateInfo(const safe_VkExternalMemoryImageCreateInfo& copy_src);
    safe_VkExternalMemoryImageCreateInfo& operator=(const safe_VkExternalMemoryImageCreateInfo& copy_src);
    safe_VkExternalMemoryImageCreateInfo();
    ~safe_VkExternalMemoryImageCreateInfo();
    void initialize(const VkExternalMemoryImageCreateInfo* in_struct);
    void initialize(const safe_VkExternalMemoryImageCreateInfo* copy_src);
    VkExternalMemoryImageCreateInfo *ptr() { return reinterpret_cast<VkExternalMemoryImageCreateInfo *>(this); }
    VkExternalMemoryImageCreateInfo const *ptr() const { return reinterpret_cast<VkExternalMemoryImageCreateInfo const *>(this); }
};

struct safe_VkExternalMemoryBufferCreateInfo {
    VkStructureType sType;
    const void* pNext;
    VkExternalMemoryHandleTypeFlags handleTypes;
    safe_VkExternalMemoryBufferCreateInfo(const VkExternalMemoryBufferCreateInfo* in_struct);
    safe_VkExternalMemoryBufferCreateInfo(const safe_VkExternalMemoryBufferCreateInfo& copy_src);
    safe_VkExternalMemoryBufferCreateInfo& operator=(const safe_VkExternalMemoryBufferCreateInfo& copy_src);
    safe_VkExternalMemoryBufferCreateInfo();
    ~safe_VkExternalMemoryBufferCreateInfo();
    void initialize(const VkExternalMemoryBufferCreateInfo* in_struct);
    void initialize(const safe_VkExternalMemoryBufferCreateInfo* copy_src);
    VkExternalMemoryBufferCreateInfo *ptr() { return reinterpret_cast<VkExternalMemoryBufferCreateInfo *>(this); }
    VkExternalMemoryBufferCreateInfo const *ptr() const { return reinterpret_cast<VkExternalMemoryBufferCreateInfo const *>(this); }
};

struct safe_VkExportMemoryAllocateInfo {
    VkStructureType sType;
    const void* pNext;
    VkExternalMemoryHandleTypeFlags handleTypes;
    safe_VkExportMemoryAllocateInfo(const VkExportMemoryAllocateInfo* in_struct);
    safe_VkExportMemoryAllocateInfo(const safe_VkExportMemoryAllocateInfo& copy_src);
    safe_VkExportMemoryAllocateInfo& operator=(const safe_VkExportMemoryAllocateInfo& copy_src);
    safe_VkExportMemoryAllocateInfo();
    ~safe_VkExportMemoryAllocateInfo();
    void initialize(const VkExportMemoryAllocateInfo* in_struct);
    void initialize(const safe_VkExportMemoryAllocateInfo* copy_src);
    VkExportMemoryAllocateInfo *ptr() { return reinterpret_cast<VkExportMemoryAllocateInfo *>(this); }
    VkExportMemoryAllocateInfo const *ptr() const { return reinterpret_cast<VkExportMemoryAllocateInfo const *>(this); }
};

struct safe_VkPhysicalDeviceExternalFenceInfo {
    VkStructureType sType;
    const void* pNext;
    VkExternalFenceHandleTypeFlagBits handleType;
    safe_VkPhysicalDeviceExternalFenceInfo(const VkPhysicalDeviceExternalFenceInfo* in_struct);
    safe_VkPhysicalDeviceExternalFenceInfo(const safe_VkPhysicalDeviceExternalFenceInfo& copy_src);
    safe_VkPhysicalDeviceExternalFenceInfo& operator=(const safe_VkPhysicalDeviceExternalFenceInfo& copy_src);
    safe_VkPhysicalDeviceExternalFenceInfo();
    ~safe_VkPhysicalDeviceExternalFenceInfo();
    void initialize(const VkPhysicalDeviceExternalFenceInfo* in_struct);
    void initialize(const safe_VkPhysicalDeviceExternalFenceInfo* copy_src);
    VkPhysicalDeviceExternalFenceInfo *ptr() { return reinterpret_cast<VkPhysicalDeviceExternalFenceInfo *>(this); }
    VkPhysicalDeviceExternalFenceInfo const *ptr() const { return reinterpret_cast<VkPhysicalDeviceExternalFenceInfo const *>(this); }
};

struct safe_VkExternalFenceProperties {
    VkStructureType sType;
    void* pNext;
    VkExternalFenceHandleTypeFlags exportFromImportedHandleTypes;
    VkExternalFenceHandleTypeFlags compatibleHandleTypes;
    VkExternalFenceFeatureFlags externalFenceFeatures;
    safe_VkExternalFenceProperties(const VkExternalFenceProperties* in_struct);
    safe_VkExternalFenceProperties(const safe_VkExternalFenceProperties& copy_src);
    safe_VkExternalFenceProperties& operator=(const safe_VkExternalFenceProperties& copy_src);
    safe_VkExternalFenceProperties();
    ~safe_VkExternalFenceProperties();
    void initialize(const VkExternalFenceProperties* in_struct);
    void initialize(const safe_VkExternalFenceProperties* copy_src);
    VkExternalFenceProperties *ptr() { return reinterpret_cast<VkExternalFenceProperties *>(this); }
    VkExternalFenceProperties const *ptr() const { return reinterpret_cast<VkExternalFenceProperties const *>(this); }
};

struct safe_VkExportFenceCreateInfo {
    VkStructureType sType;
    const void* pNext;
    VkExternalFenceHandleTypeFlags handleTypes;
    safe_VkExportFenceCreateInfo(const VkExportFenceCreateInfo* in_struct);
    safe_VkExportFenceCreateInfo(const safe_VkExportFenceCreateInfo& copy_src);
    safe_VkExportFenceCreateInfo& operator=(const safe_VkExportFenceCreateInfo& copy_src);
    safe_VkExportFenceCreateInfo();
    ~safe_VkExportFenceCreateInfo();
    void initialize(const VkExportFenceCreateInfo* in_struct);
    void initialize(const safe_VkExportFenceCreateInfo* copy_src);
    VkExportFenceCreateInfo *ptr() { return reinterpret_cast<VkExportFenceCreateInfo *>(this); }
    VkExportFenceCreateInfo const *ptr() const { return reinterpret_cast<VkExportFenceCreateInfo const *>(this); }
};

struct safe_VkExportSemaphoreCreateInfo {
    VkStructureType sType;
    const void* pNext;
    VkExternalSemaphoreHandleTypeFlags handleTypes;
    safe_VkExportSemaphoreCreateInfo(const VkExportSemaphoreCreateInfo* in_struct);
    safe_VkExportSemaphoreCreateInfo(const safe_VkExportSemaphoreCreateInfo& copy_src);
    safe_VkExportSemaphoreCreateInfo& operator=(const safe_VkExportSemaphoreCreateInfo& copy_src);
    safe_VkExportSemaphoreCreateInfo();
    ~safe_VkExportSemaphoreCreateInfo();
    void initialize(const VkExportSemaphoreCreateInfo* in_struct);
    void initialize(const safe_VkExportSemaphoreCreateInfo* copy_src);
    VkExportSemaphoreCreateInfo *ptr() { return reinterpret_cast<VkExportSemaphoreCreateInfo *>(this); }
    VkExportSemaphoreCreateInfo const *ptr() const { return reinterpret_cast<VkExportSemaphoreCreateInfo const *>(this); }
};

struct safe_VkPhysicalDeviceExternalSemaphoreInfo {
    VkStructureType sType;
    const void* pNext;
    VkExternalSemaphoreHandleTypeFlagBits handleType;
    safe_VkPhysicalDeviceExternalSemaphoreInfo(const VkPhysicalDeviceExternalSemaphoreInfo* in_struct);
    safe_VkPhysicalDeviceExternalSemaphoreInfo(const safe_VkPhysicalDeviceExternalSemaphoreInfo& copy_src);
    safe_VkPhysicalDeviceExternalSemaphoreInfo& operator=(const safe_VkPhysicalDeviceExternalSemaphoreInfo& copy_src);
    safe_VkPhysicalDeviceExternalSemaphoreInfo();
    ~safe_VkPhysicalDeviceExternalSemaphoreInfo();
    void initialize(const VkPhysicalDeviceExternalSemaphoreInfo* in_struct);
    void initialize(const safe_VkPhysicalDeviceExternalSemaphoreInfo* copy_src);
    VkPhysicalDeviceExternalSemaphoreInfo *ptr() { return reinterpret_cast<VkPhysicalDeviceExternalSemaphoreInfo *>(this); }
    VkPhysicalDeviceExternalSemaphoreInfo const *ptr() const { return reinterpret_cast<VkPhysicalDeviceExternalSemaphoreInfo const *>(this); }
};

struct safe_VkExternalSemaphoreProperties {
    VkStructureType sType;
    void* pNext;
    VkExternalSemaphoreHandleTypeFlags exportFromImportedHandleTypes;
    VkExternalSemaphoreHandleTypeFlags compatibleHandleTypes;
    VkExternalSemaphoreFeatureFlags externalSemaphoreFeatures;
    safe_VkExternalSemaphoreProperties(const VkExternalSemaphoreProperties* in_struct);
    safe_VkExternalSemaphoreProperties(const safe_VkExternalSemaphoreProperties& copy_src);
    safe_VkExternalSemaphoreProperties& operator=(const safe_VkExternalSemaphoreProperties& copy_src);
    safe_VkExternalSemaphoreProperties();
    ~safe_VkExternalSemaphoreProperties();
    void initialize(const VkExternalSemaphoreProperties* in_struct);
    void initialize(const safe_VkExternalSemaphoreProperties* copy_src);
    VkExternalSemaphoreProperties *ptr() { return reinterpret_cast<VkExternalSemaphoreProperties *>(this); }
    VkExternalSemaphoreProperties const *ptr() const { return reinterpret_cast<VkExternalSemaphoreProperties const *>(this); }
};

struct safe_VkPhysicalDeviceMaintenance3Properties {
    VkStructureType sType;
    void* pNext;
    uint32_t maxPerSetDescriptors;
    VkDeviceSize maxMemoryAllocationSize;
    safe_VkPhysicalDeviceMaintenance3Properties(const VkPhysicalDeviceMaintenance3Properties* in_struct);
    safe_VkPhysicalDeviceMaintenance3Properties(const safe_VkPhysicalDeviceMaintenance3Properties& copy_src);
    safe_VkPhysicalDeviceMaintenance3Properties& operator=(const safe_VkPhysicalDeviceMaintenance3Properties& copy_src);
    safe_VkPhysicalDeviceMaintenance3Properties();
    ~safe_VkPhysicalDeviceMaintenance3Properties();
    void initialize(const VkPhysicalDeviceMaintenance3Properties* in_struct);
    void initialize(const safe_VkPhysicalDeviceMaintenance3Properties* copy_src);
    VkPhysicalDeviceMaintenance3Properties *ptr() { return reinterpret_cast<VkPhysicalDeviceMaintenance3Properties *>(this); }
    VkPhysicalDeviceMaintenance3Properties const *ptr() const { return reinterpret_cast<VkPhysicalDeviceMaintenance3Properties const *>(this); }
};

struct safe_VkDescriptorSetLayoutSupport {
    VkStructureType sType;
    void* pNext;
    VkBool32 supported;
    safe_VkDescriptorSetLayoutSupport(const VkDescriptorSetLayoutSupport* in_struct);
    safe_VkDescriptorSetLayoutSupport(const safe_VkDescriptorSetLayoutSupport& copy_src);
    safe_VkDescriptorSetLayoutSupport& operator=(const safe_VkDescriptorSetLayoutSupport& copy_src);
    safe_VkDescriptorSetLayoutSupport();
    ~safe_VkDescriptorSetLayoutSupport();
    void initialize(const VkDescriptorSetLayoutSupport* in_struct);
    void initialize(const safe_VkDescriptorSetLayoutSupport* copy_src);
    VkDescriptorSetLayoutSupport *ptr() { return reinterpret_cast<VkDescriptorSetLayoutSupport *>(this); }
    VkDescriptorSetLayoutSupport const *ptr() const { return reinterpret_cast<VkDescriptorSetLayoutSupport const *>(this); }
};

struct safe_VkPhysicalDeviceShaderDrawParametersFeatures {
    VkStructureType sType;
    void* pNext;
    VkBool32 shaderDrawParameters;
    safe_VkPhysicalDeviceShaderDrawParametersFeatures(const VkPhysicalDeviceShaderDrawParametersFeatures* in_struct);
    safe_VkPhysicalDeviceShaderDrawParametersFeatures(const safe_VkPhysicalDeviceShaderDrawParametersFeatures& copy_src);
    safe_VkPhysicalDeviceShaderDrawParametersFeatures& operator=(const safe_VkPhysicalDeviceShaderDrawParametersFeatures& copy_src);
    safe_VkPhysicalDeviceShaderDrawParametersFeatures();
    ~safe_VkPhysicalDeviceShaderDrawParametersFeatures();
    void initialize(const VkPhysicalDeviceShaderDrawParametersFeatures* in_struct);
    void initialize(const safe_VkPhysicalDeviceShaderDrawParametersFeatures* copy_src);
    VkPhysicalDeviceShaderDrawParametersFeatures *ptr() { return reinterpret_cast<VkPhysicalDeviceShaderDrawParametersFeatures *>(this); }
    VkPhysicalDeviceShaderDrawParametersFeatures const *ptr() const { return reinterpret_cast<VkPhysicalDeviceShaderDrawParametersFeatures const *>(this); }
};

struct safe_VkPhysicalDeviceVulkan11Features {
    VkStructureType sType;
    void* pNext;
    VkBool32 storageBuffer16BitAccess;
    VkBool32 uniformAndStorageBuffer16BitAccess;
    VkBool32 storagePushConstant16;
    VkBool32 storageInputOutput16;
    VkBool32 multiview;
    VkBool32 multiviewGeometryShader;
    VkBool32 multiviewTessellationShader;
    VkBool32 variablePointersStorageBuffer;
    VkBool32 variablePointers;
    VkBool32 protectedMemory;
    VkBool32 samplerYcbcrConversion;
    VkBool32 shaderDrawParameters;
    safe_VkPhysicalDeviceVulkan11Features(const VkPhysicalDeviceVulkan11Features* in_struct);
    safe_VkPhysicalDeviceVulkan11Features(const safe_VkPhysicalDeviceVulkan11Features& copy_src);
    safe_VkPhysicalDeviceVulkan11Features& operator=(const safe_VkPhysicalDeviceVulkan11Features& copy_src);
    safe_VkPhysicalDeviceVulkan11Features();
    ~safe_VkPhysicalDeviceVulkan11Features();
    void initialize(const VkPhysicalDeviceVulkan11Features* in_struct);
    void initialize(const safe_VkPhysicalDeviceVulkan11Features* copy_src);
    VkPhysicalDeviceVulkan11Features *ptr() { return reinterpret_cast<VkPhysicalDeviceVulkan11Features *>(this); }
    VkPhysicalDeviceVulkan11Features const *ptr() const { return reinterpret_cast<VkPhysicalDeviceVulkan11Features const *>(this); }
};

struct safe_VkPhysicalDeviceVulkan11Properties {
    VkStructureType sType;
    void* pNext;
    uint8_t deviceUUID[VK_UUID_SIZE];
    uint8_t driverUUID[VK_UUID_SIZE];
    uint8_t deviceLUID[VK_LUID_SIZE];
    uint32_t deviceNodeMask;
    VkBool32 deviceLUIDValid;
    uint32_t subgroupSize;
    VkShaderStageFlags subgroupSupportedStages;
    VkSubgroupFeatureFlags subgroupSupportedOperations;
    VkBool32 subgroupQuadOperationsInAllStages;
    VkPointClippingBehavior pointClippingBehavior;
    uint32_t maxMultiviewViewCount;
    uint32_t maxMultiviewInstanceIndex;
    VkBool32 protectedNoFault;
    uint32_t maxPerSetDescriptors;
    VkDeviceSize maxMemoryAllocationSize;
    safe_VkPhysicalDeviceVulkan11Properties(const VkPhysicalDeviceVulkan11Properties* in_struct);
    safe_VkPhysicalDeviceVulkan11Properties(const safe_VkPhysicalDeviceVulkan11Properties& copy_src);
    safe_VkPhysicalDeviceVulkan11Properties& operator=(const safe_VkPhysicalDeviceVulkan11Properties& copy_src);
    safe_VkPhysicalDeviceVulkan11Properties();
    ~safe_VkPhysicalDeviceVulkan11Properties();
    void initialize(const VkPhysicalDeviceVulkan11Properties* in_struct);
    void initialize(const safe_VkPhysicalDeviceVulkan11Properties* copy_src);
    VkPhysicalDeviceVulkan11Properties *ptr() { return reinterpret_cast<VkPhysicalDeviceVulkan11Properties *>(this); }
    VkPhysicalDeviceVulkan11Properties const *ptr() const { return reinterpret_cast<VkPhysicalDeviceVulkan11Properties const *>(this); }
};

struct safe_VkPhysicalDeviceVulkan12Features {
    VkStructureType sType;
    void* pNext;
    VkBool32 samplerMirrorClampToEdge;
    VkBool32 drawIndirectCount;
    VkBool32 storageBuffer8BitAccess;
    VkBool32 uniformAndStorageBuffer8BitAccess;
    VkBool32 storagePushConstant8;
    VkBool32 shaderBufferInt64Atomics;
    VkBool32 shaderSharedInt64Atomics;
    VkBool32 shaderFloat16;
    VkBool32 shaderInt8;
    VkBool32 descriptorIndexing;
    VkBool32 shaderInputAttachmentArrayDynamicIndexing;
    VkBool32 shaderUniformTexelBufferArrayDynamicIndexing;
    VkBool32 shaderStorageTexelBufferArrayDynamicIndexing;
    VkBool32 shaderUniformBufferArrayNonUniformIndexing;
    VkBool32 shaderSampledImageArrayNonUniformIndexing;
    VkBool32 shaderStorageBufferArrayNonUniformIndexing;
    VkBool32 shaderStorageImageArrayNonUniformIndexing;
    VkBool32 shaderInputAttachmentArrayNonUniformIndexing;
    VkBool32 shaderUniformTexelBufferArrayNonUniformIndexing;
    VkBool32 shaderStorageTexelBufferArrayNonUniformIndexing;
    VkBool32 descriptorBindingUniformBufferUpdateAfterBind;
    VkBool32 descriptorBindingSampledImageUpdateAfterBind;
    VkBool32 descriptorBindingStorageImageUpdateAfterBind;
    VkBool32 descriptorBindingStorageBufferUpdateAfterBind;
    VkBool32 descriptorBindingUniformTexelBufferUpdateAfterBind;
    VkBool32 descriptorBindingStorageTexelBufferUpdateAfterBind;
    VkBool32 descriptorBindingUpdateUnusedWhilePending;
    VkBool32 descriptorBindingPartiallyBound;
    VkBool32 descriptorBindingVariableDescriptorCount;
    VkBool32 runtimeDescriptorArray;
    VkBool32 samplerFilterMinmax;
    VkBool32 scalarBlockLayout;
    VkBool32 imagelessFramebuffer;
    VkBool32 uniformBufferStandardLayout;
    VkBool32 shaderSubgroupExtendedTypes;
    VkBool32 separateDepthStencilLayouts;
    VkBool32 hostQueryReset;
    VkBool32 timelineSemaphore;
    VkBool32 bufferDeviceAddress;
    VkBool32 bufferDeviceAddressCaptureReplay;
    VkBool32 bufferDeviceAddressMultiDevice;
    VkBool32 vulkanMemoryModel;
    VkBool32 vulkanMemoryModelDeviceScope;
    VkBool32 vulkanMemoryModelAvailabilityVisibilityChains;
    VkBool32 shaderOutputViewportIndex;
    VkBool32 shaderOutputLayer;
    VkBool32 subgroupBroadcastDynamicId;
    safe_VkPhysicalDeviceVulkan12Features(const VkPhysicalDeviceVulkan12Features* in_struct);
    safe_VkPhysicalDeviceVulkan12Features(const safe_VkPhysicalDeviceVulkan12Features& copy_src);
    safe_VkPhysicalDeviceVulkan12Features& operator=(const safe_VkPhysicalDeviceVulkan12Features& copy_src);
    safe_VkPhysicalDeviceVulkan12Features();
    ~safe_VkPhysicalDeviceVulkan12Features();
    void initialize(const VkPhysicalDeviceVulkan12Features* in_struct);
    void initialize(const safe_VkPhysicalDeviceVulkan12Features* copy_src);
    VkPhysicalDeviceVulkan12Features *ptr() { return reinterpret_cast<VkPhysicalDeviceVulkan12Features *>(this); }
    VkPhysicalDeviceVulkan12Features const *ptr() const { return reinterpret_cast<VkPhysicalDeviceVulkan12Features const *>(this); }
};

struct safe_VkPhysicalDeviceVulkan12Properties {
    VkStructureType sType;
    void* pNext;
    VkDriverId driverID;
    char driverName[VK_MAX_DRIVER_NAME_SIZE];
    char driverInfo[VK_MAX_DRIVER_INFO_SIZE];
    VkConformanceVersion conformanceVersion;
    VkShaderFloatControlsIndependence denormBehaviorIndependence;
    VkShaderFloatControlsIndependence roundingModeIndependence;
    VkBool32 shaderSignedZeroInfNanPreserveFloat16;
    VkBool32 shaderSignedZeroInfNanPreserveFloat32;
    VkBool32 shaderSignedZeroInfNanPreserveFloat64;
    VkBool32 shaderDenormPreserveFloat16;
    VkBool32 shaderDenormPreserveFloat32;
    VkBool32 shaderDenormPreserveFloat64;
    VkBool32 shaderDenormFlushToZeroFloat16;
    VkBool32 shaderDenormFlushToZeroFloat32;
    VkBool32 shaderDenormFlushToZeroFloat64;
    VkBool32 shaderRoundingModeRTEFloat16;
    VkBool32 shaderRoundingModeRTEFloat32;
    VkBool32 shaderRoundingModeRTEFloat64;
    VkBool32 shaderRoundingModeRTZFloat16;
    VkBool32 shaderRoundingModeRTZFloat32;
    VkBool32 shaderRoundingModeRTZFloat64;
    uint32_t maxUpdateAfterBindDescriptorsInAllPools;
    VkBool32 shaderUniformBufferArrayNonUniformIndexingNative;
    VkBool32 shaderSampledImageArrayNonUniformIndexingNative;
    VkBool32 shaderStorageBufferArrayNonUniformIndexingNative;
    VkBool32 shaderStorageImageArrayNonUniformIndexingNative;
    VkBool32 shaderInputAttachmentArrayNonUniformIndexingNative;
    VkBool32 robustBufferAccessUpdateAfterBind;
    VkBool32 quadDivergentImplicitLod;
    uint32_t maxPerStageDescriptorUpdateAfterBindSamplers;
    uint32_t maxPerStageDescriptorUpdateAfterBindUniformBuffers;
    uint32_t maxPerStageDescriptorUpdateAfterBindStorageBuffers;
    uint32_t maxPerStageDescriptorUpdateAfterBindSampledImages;
    uint32_t maxPerStageDescriptorUpdateAfterBindStorageImages;
    uint32_t maxPerStageDescriptorUpdateAfterBindInputAttachments;
    uint32_t maxPerStageUpdateAfterBindResources;
    uint32_t maxDescriptorSetUpdateAfterBindSamplers;
    uint32_t maxDescriptorSetUpdateAfterBindUniformBuffers;
    uint32_t maxDescriptorSetUpdateAfterBindUniformBuffersDynamic;
    uint32_t maxDescriptorSetUpdateAfterBindStorageBuffers;
    uint32_t maxDescriptorSetUpdateAfterBindStorageBuffersDynamic;
    uint32_t maxDescriptorSetUpdateAfterBindSampledImages;
    uint32_t maxDescriptorSetUpdateAfterBindStorageImages;
    uint32_t maxDescriptorSetUpdateAfterBindInputAttachments;
    VkResolveModeFlags supportedDepthResolveModes;
    VkResolveModeFlags supportedStencilResolveModes;
    VkBool32 independentResolveNone;
    VkBool32 independentResolve;
    VkBool32 filterMinmaxSingleComponentFormats;
    VkBool32 filterMinmaxImageComponentMapping;
    uint64_t maxTimelineSemaphoreValueDifference;
    VkSampleCountFlags framebufferIntegerColorSampleCounts;
    safe_VkPhysicalDeviceVulkan12Properties(const VkPhysicalDeviceVulkan12Properties* in_struct);
    safe_VkPhysicalDeviceVulkan12Properties(const safe_VkPhysicalDeviceVulkan12Properties& copy_src);
    safe_VkPhysicalDeviceVulkan12Properties& operator=(const safe_VkPhysicalDeviceVulkan12Properties& copy_src);
    safe_VkPhysicalDeviceVulkan12Properties();
    ~safe_VkPhysicalDeviceVulkan12Properties();
    void initialize(const VkPhysicalDeviceVulkan12Properties* in_struct);
    void initialize(const safe_VkPhysicalDeviceVulkan12Properties* copy_src);
    VkPhysicalDeviceVulkan12Properties *ptr() { return reinterpret_cast<VkPhysicalDeviceVulkan12Properties *>(this); }
    VkPhysicalDeviceVulkan12Properties const *ptr() const { return reinterpret_cast<VkPhysicalDeviceVulkan12Properties const *>(this); }
};

struct safe_VkImageFormatListCreateInfo {
    VkStructureType sType;
    const void* pNext;
    uint32_t viewFormatCount;
    const VkFormat* pViewFormats;
    safe_VkImageFormatListCreateInfo(const VkImageFormatListCreateInfo* in_struct);
    safe_VkImageFormatListCreateInfo(const safe_VkImageFormatListCreateInfo& copy_src);
    safe_VkImageFormatListCreateInfo& operator=(const safe_VkImageFormatListCreateInfo& copy_src);
    safe_VkImageFormatListCreateInfo();
    ~safe_VkImageFormatListCreateInfo();
    void initialize(const VkImageFormatListCreateInfo* in_struct);
    void initialize(const safe_VkImageFormatListCreateInfo* copy_src);
    VkImageFormatListCreateInfo *ptr() { return reinterpret_cast<VkImageFormatListCreateInfo *>(this); }
    VkImageFormatListCreateInfo const *ptr() const { return reinterpret_cast<VkImageFormatListCreateInfo const *>(this); }
};

struct safe_VkAttachmentDescription2 {
    VkStructureType sType;
    const void* pNext;
    VkAttachmentDescriptionFlags flags;
    VkFormat format;
    VkSampleCountFlagBits samples;
    VkAttachmentLoadOp loadOp;
    VkAttachmentStoreOp storeOp;
    VkAttachmentLoadOp stencilLoadOp;
    VkAttachmentStoreOp stencilStoreOp;
    VkImageLayout initialLayout;
    VkImageLayout finalLayout;
    safe_VkAttachmentDescription2(const VkAttachmentDescription2* in_struct);
    safe_VkAttachmentDescription2(const safe_VkAttachmentDescription2& copy_src);
    safe_VkAttachmentDescription2& operator=(const safe_VkAttachmentDescription2& copy_src);
    safe_VkAttachmentDescription2();
    ~safe_VkAttachmentDescription2();
    void initialize(const VkAttachmentDescription2* in_struct);
    void initialize(const safe_VkAttachmentDescription2* copy_src);
    VkAttachmentDescription2 *ptr() { return reinterpret_cast<VkAttachmentDescription2 *>(this); }
    VkAttachmentDescription2 const *ptr() const { return reinterpret_cast<VkAttachmentDescription2 const *>(this); }
};

struct safe_VkAttachmentReference2 {
    VkStructureType sType;
    const void* pNext;
    uint32_t attachment;
    VkImageLayout layout;
    VkImageAspectFlags aspectMask;
    safe_VkAttachmentReference2(const VkAttachmentReference2* in_struct);
    safe_VkAttachmentReference2(const safe_VkAttachmentReference2& copy_src);
    safe_VkAttachmentReference2& operator=(const safe_VkAttachmentReference2& copy_src);
    safe_VkAttachmentReference2();
    ~safe_VkAttachmentReference2();
    void initialize(const VkAttachmentReference2* in_struct);
    void initialize(const safe_VkAttachmentReference2* copy_src);
    VkAttachmentReference2 *ptr() { return reinterpret_cast<VkAttachmentReference2 *>(this); }
    VkAttachmentReference2 const *ptr() const { return reinterpret_cast<VkAttachmentReference2 const *>(this); }
};

struct safe_VkSubpassDescription2 {
    VkStructureType sType;
    const void* pNext;
    VkSubpassDescriptionFlags flags;
    VkPipelineBindPoint pipelineBindPoint;
    uint32_t viewMask;
    uint32_t inputAttachmentCount;
    safe_VkAttachmentReference2* pInputAttachments;
    uint32_t colorAttachmentCount;
    safe_VkAttachmentReference2* pColorAttachments;
    safe_VkAttachmentReference2* pResolveAttachments;
    safe_VkAttachmentReference2* pDepthStencilAttachment;
    uint32_t preserveAttachmentCount;
    const uint32_t* pPreserveAttachments;
    safe_VkSubpassDescription2(const VkSubpassDescription2* in_struct);
    safe_VkSubpassDescription2(const safe_VkSubpassDescription2& copy_src);
    safe_VkSubpassDescription2& operator=(const safe_VkSubpassDescription2& copy_src);
    safe_VkSubpassDescription2();
    ~safe_VkSubpassDescription2();
    void initialize(const VkSubpassDescription2* in_struct);
    void initialize(const safe_VkSubpassDescription2* copy_src);
    VkSubpassDescription2 *ptr() { return reinterpret_cast<VkSubpassDescription2 *>(this); }
    VkSubpassDescription2 const *ptr() const { return reinterpret_cast<VkSubpassDescription2 const *>(this); }
};

struct safe_VkSubpassDependency2 {
    VkStructureType sType;
    const void* pNext;
    uint32_t srcSubpass;
    uint32_t dstSubpass;
    VkPipelineStageFlags srcStageMask;
    VkPipelineStageFlags dstStageMask;
    VkAccessFlags srcAccessMask;
    VkAccessFlags dstAccessMask;
    VkDependencyFlags dependencyFlags;
    int32_t viewOffset;
    safe_VkSubpassDependency2(const VkSubpassDependency2* in_struct);
    safe_VkSubpassDependency2(const safe_VkSubpassDependency2& copy_src);
    safe_VkSubpassDependency2& operator=(const safe_VkSubpassDependency2& copy_src);
    safe_VkSubpassDependency2();
    ~safe_VkSubpassDependency2();
    void initialize(const VkSubpassDependency2* in_struct);
    void initialize(const safe_VkSubpassDependency2* copy_src);
    VkSubpassDependency2 *ptr() { return reinterpret_cast<VkSubpassDependency2 *>(this); }
    VkSubpassDependency2 const *ptr() const { return reinterpret_cast<VkSubpassDependency2 const *>(this); }
};

struct safe_VkRenderPassCreateInfo2 {
    VkStructureType sType;
    const void* pNext;
    VkRenderPassCreateFlags flags;
    uint32_t attachmentCount;
    safe_VkAttachmentDescription2* pAttachments;
    uint32_t subpassCount;
    safe_VkSubpassDescription2* pSubpasses;
    uint32_t dependencyCount;
    safe_VkSubpassDependency2* pDependencies;
    uint32_t correlatedViewMaskCount;
    const uint32_t* pCorrelatedViewMasks;
    safe_VkRenderPassCreateInfo2(const VkRenderPassCreateInfo2* in_struct);
    safe_VkRenderPassCreateInfo2(const safe_VkRenderPassCreateInfo2& copy_src);
    safe_VkRenderPassCreateInfo2& operator=(const safe_VkRenderPassCreateInfo2& copy_src);
    safe_VkRenderPassCreateInfo2();
    ~safe_VkRenderPassCreateInfo2();
    void initialize(const VkRenderPassCreateInfo2* in_struct);
    void initialize(const safe_VkRenderPassCreateInfo2* copy_src);
    VkRenderPassCreateInfo2 *ptr() { return reinterpret_cast<VkRenderPassCreateInfo2 *>(this); }
    VkRenderPassCreateInfo2 const *ptr() const { return reinterpret_cast<VkRenderPassCreateInfo2 const *>(this); }
};

struct safe_VkSubpassBeginInfo {
    VkStructureType sType;
    const void* pNext;
    VkSubpassContents contents;
    safe_VkSubpassBeginInfo(const VkSubpassBeginInfo* in_struct);
    safe_VkSubpassBeginInfo(const safe_VkSubpassBeginInfo& copy_src);
    safe_VkSubpassBeginInfo& operator=(const safe_VkSubpassBeginInfo& copy_src);
    safe_VkSubpassBeginInfo();
    ~safe_VkSubpassBeginInfo();
    void initialize(const VkSubpassBeginInfo* in_struct);
    void initialize(const safe_VkSubpassBeginInfo* copy_src);
    VkSubpassBeginInfo *ptr() { return reinterpret_cast<VkSubpassBeginInfo *>(this); }
    VkSubpassBeginInfo const *ptr() const { return reinterpret_cast<VkSubpassBeginInfo const *>(this); }
};

struct safe_VkSubpassEndInfo {
    VkStructureType sType;
    const void* pNext;
    safe_VkSubpassEndInfo(const VkSubpassEndInfo* in_struct);
    safe_VkSubpassEndInfo(const safe_VkSubpassEndInfo& copy_src);
    safe_VkSubpassEndInfo& operator=(const safe_VkSubpassEndInfo& copy_src);
    safe_VkSubpassEndInfo();
    ~safe_VkSubpassEndInfo();
    void initialize(const VkSubpassEndInfo* in_struct);
    void initialize(const safe_VkSubpassEndInfo* copy_src);
    VkSubpassEndInfo *ptr() { return reinterpret_cast<VkSubpassEndInfo *>(this); }
    VkSubpassEndInfo const *ptr() const { return reinterpret_cast<VkSubpassEndInfo const *>(this); }
};

struct safe_VkPhysicalDevice8BitStorageFeatures {
    VkStructureType sType;
    void* pNext;
    VkBool32 storageBuffer8BitAccess;
    VkBool32 uniformAndStorageBuffer8BitAccess;
    VkBool32 storagePushConstant8;
    safe_VkPhysicalDevice8BitStorageFeatures(const VkPhysicalDevice8BitStorageFeatures* in_struct);
    safe_VkPhysicalDevice8BitStorageFeatures(const safe_VkPhysicalDevice8BitStorageFeatures& copy_src);
    safe_VkPhysicalDevice8BitStorageFeatures& operator=(const safe_VkPhysicalDevice8BitStorageFeatures& copy_src);
    safe_VkPhysicalDevice8BitStorageFeatures();
    ~safe_VkPhysicalDevice8BitStorageFeatures();
    void initialize(const VkPhysicalDevice8BitStorageFeatures* in_struct);
    void initialize(const safe_VkPhysicalDevice8BitStorageFeatures* copy_src);
    VkPhysicalDevice8BitStorageFeatures *ptr() { return reinterpret_cast<VkPhysicalDevice8BitStorageFeatures *>(this); }
    VkPhysicalDevice8BitStorageFeatures const *ptr() const { return reinterpret_cast<VkPhysicalDevice8BitStorageFeatures const *>(this); }
};

struct safe_VkPhysicalDeviceDriverProperties {
    VkStructureType sType;
    void* pNext;
    VkDriverId driverID;
    char driverName[VK_MAX_DRIVER_NAME_SIZE];
    char driverInfo[VK_MAX_DRIVER_INFO_SIZE];
    VkConformanceVersion conformanceVersion;
    safe_VkPhysicalDeviceDriverProperties(const VkPhysicalDeviceDriverProperties* in_struct);
    safe_VkPhysicalDeviceDriverProperties(const safe_VkPhysicalDeviceDriverProperties& copy_src);
    safe_VkPhysicalDeviceDriverProperties& operator=(const safe_VkPhysicalDeviceDriverProperties& copy_src);
    safe_VkPhysicalDeviceDriverProperties();
    ~safe_VkPhysicalDeviceDriverProperties();
    void initialize(const VkPhysicalDeviceDriverProperties* in_struct);
    void initialize(const safe_VkPhysicalDeviceDriverProperties* copy_src);
    VkPhysicalDeviceDriverProperties *ptr() { return reinterpret_cast<VkPhysicalDeviceDriverProperties *>(this); }
    VkPhysicalDeviceDriverProperties const *ptr() const { return reinterpret_cast<VkPhysicalDeviceDriverProperties const *>(this); }
};

struct safe_VkPhysicalDeviceShaderAtomicInt64Features {
    VkStructureType sType;
    void* pNext;
    VkBool32 shaderBufferInt64Atomics;
    VkBool32 shaderSharedInt64Atomics;
    safe_VkPhysicalDeviceShaderAtomicInt64Features(const VkPhysicalDeviceShaderAtomicInt64Features* in_struct);
    safe_VkPhysicalDeviceShaderAtomicInt64Features(const safe_VkPhysicalDeviceShaderAtomicInt64Features& copy_src);
    safe_VkPhysicalDeviceShaderAtomicInt64Features& operator=(const safe_VkPhysicalDeviceShaderAtomicInt64Features& copy_src);
    safe_VkPhysicalDeviceShaderAtomicInt64Features();
    ~safe_VkPhysicalDeviceShaderAtomicInt64Features();
    void initialize(const VkPhysicalDeviceShaderAtomicInt64Features* in_struct);
    void initialize(const safe_VkPhysicalDeviceShaderAtomicInt64Features* copy_src);
    VkPhysicalDeviceShaderAtomicInt64Features *ptr() { return reinterpret_cast<VkPhysicalDeviceShaderAtomicInt64Features *>(this); }
    VkPhysicalDeviceShaderAtomicInt64Features const *ptr() const { return reinterpret_cast<VkPhysicalDeviceShaderAtomicInt64Features const *>(this); }
};

struct safe_VkPhysicalDeviceShaderFloat16Int8Features {
    VkStructureType sType;
    void* pNext;
    VkBool32 shaderFloat16;
    VkBool32 shaderInt8;
    safe_VkPhysicalDeviceShaderFloat16Int8Features(const VkPhysicalDeviceShaderFloat16Int8Features* in_struct);
    safe_VkPhysicalDeviceShaderFloat16Int8Features(const safe_VkPhysicalDeviceShaderFloat16Int8Features& copy_src);
    safe_VkPhysicalDeviceShaderFloat16Int8Features& operator=(const safe_VkPhysicalDeviceShaderFloat16Int8Features& copy_src);
    safe_VkPhysicalDeviceShaderFloat16Int8Features();
    ~safe_VkPhysicalDeviceShaderFloat16Int8Features();
    void initialize(const VkPhysicalDeviceShaderFloat16Int8Features* in_struct);
    void initialize(const safe_VkPhysicalDeviceShaderFloat16Int8Features* copy_src);
    VkPhysicalDeviceShaderFloat16Int8Features *ptr() { return reinterpret_cast<VkPhysicalDeviceShaderFloat16Int8Features *>(this); }
    VkPhysicalDeviceShaderFloat16Int8Features const *ptr() const { return reinterpret_cast<VkPhysicalDeviceShaderFloat16Int8Features const *>(this); }
};

struct safe_VkPhysicalDeviceFloatControlsProperties {
    VkStructureType sType;
    void* pNext;
    VkShaderFloatControlsIndependence denormBehaviorIndependence;
    VkShaderFloatControlsIndependence roundingModeIndependence;
    VkBool32 shaderSignedZeroInfNanPreserveFloat16;
    VkBool32 shaderSignedZeroInfNanPreserveFloat32;
    VkBool32 shaderSignedZeroInfNanPreserveFloat64;
    VkBool32 shaderDenormPreserveFloat16;
    VkBool32 shaderDenormPreserveFloat32;
    VkBool32 shaderDenormPreserveFloat64;
    VkBool32 shaderDenormFlushToZeroFloat16;
    VkBool32 shaderDenormFlushToZeroFloat32;
    VkBool32 shaderDenormFlushToZeroFloat64;
    VkBool32 shaderRoundingModeRTEFloat16;
    VkBool32 shaderRoundingModeRTEFloat32;
    VkBool32 shaderRoundingModeRTEFloat64;
    VkBool32 shaderRoundingModeRTZFloat16;
    VkBool32 shaderRoundingModeRTZFloat32;
    VkBool32 shaderRoundingModeRTZFloat64;
    safe_VkPhysicalDeviceFloatControlsProperties(const VkPhysicalDeviceFloatControlsProperties* in_struct);
    safe_VkPhysicalDeviceFloatControlsProperties(const safe_VkPhysicalDeviceFloatControlsProperties& copy_src);
    safe_VkPhysicalDeviceFloatControlsProperties& operator=(const safe_VkPhysicalDeviceFloatControlsProperties& copy_src);
    safe_VkPhysicalDeviceFloatControlsProperties();
    ~safe_VkPhysicalDeviceFloatControlsProperties();
    void initialize(const VkPhysicalDeviceFloatControlsProperties* in_struct);
    void initialize(const safe_VkPhysicalDeviceFloatControlsProperties* copy_src);
    VkPhysicalDeviceFloatControlsProperties *ptr() { return reinterpret_cast<VkPhysicalDeviceFloatControlsProperties *>(this); }
    VkPhysicalDeviceFloatControlsProperties const *ptr() const { return reinterpret_cast<VkPhysicalDeviceFloatControlsProperties const *>(this); }
};

struct safe_VkDescriptorSetLayoutBindingFlagsCreateInfo {
    VkStructureType sType;
    const void* pNext;
    uint32_t bindingCount;
    const VkDescriptorBindingFlags* pBindingFlags;
    safe_VkDescriptorSetLayoutBindingFlagsCreateInfo(const VkDescriptorSetLayoutBindingFlagsCreateInfo* in_struct);
    safe_VkDescriptorSetLayoutBindingFlagsCreateInfo(const safe_VkDescriptorSetLayoutBindingFlagsCreateInfo& copy_src);
    safe_VkDescriptorSetLayoutBindingFlagsCreateInfo& operator=(const safe_VkDescriptorSetLayoutBindingFlagsCreateInfo& copy_src);
    safe_VkDescriptorSetLayoutBindingFlagsCreateInfo();
    ~safe_VkDescriptorSetLayoutBindingFlagsCreateInfo();
    void initialize(const VkDescriptorSetLayoutBindingFlagsCreateInfo* in_struct);
    void initialize(const safe_VkDescriptorSetLayoutBindingFlagsCreateInfo* copy_src);
    VkDescriptorSetLayoutBindingFlagsCreateInfo *ptr() { return reinterpret_cast<VkDescriptorSetLayoutBindingFlagsCreateInfo *>(this); }
    VkDescriptorSetLayoutBindingFlagsCreateInfo const *ptr() const { return reinterpret_cast<VkDescriptorSetLayoutBindingFlagsCreateInfo const *>(this); }
};

struct safe_VkPhysicalDeviceDescriptorIndexingFeatures {
    VkStructureType sType;
    void* pNext;
    VkBool32 shaderInputAttachmentArrayDynamicIndexing;
    VkBool32 shaderUniformTexelBufferArrayDynamicIndexing;
    VkBool32 shaderStorageTexelBufferArrayDynamicIndexing;
    VkBool32 shaderUniformBufferArrayNonUniformIndexing;
    VkBool32 shaderSampledImageArrayNonUniformIndexing;
    VkBool32 shaderStorageBufferArrayNonUniformIndexing;
    VkBool32 shaderStorageImageArrayNonUniformIndexing;
    VkBool32 shaderInputAttachmentArrayNonUniformIndexing;
    VkBool32 shaderUniformTexelBufferArrayNonUniformIndexing;
    VkBool32 shaderStorageTexelBufferArrayNonUniformIndexing;
    VkBool32 descriptorBindingUniformBufferUpdateAfterBind;
    VkBool32 descriptorBindingSampledImageUpdateAfterBind;
    VkBool32 descriptorBindingStorageImageUpdateAfterBind;
    VkBool32 descriptorBindingStorageBufferUpdateAfterBind;
    VkBool32 descriptorBindingUniformTexelBufferUpdateAfterBind;
    VkBool32 descriptorBindingStorageTexelBufferUpdateAfterBind;
    VkBool32 descriptorBindingUpdateUnusedWhilePending;
    VkBool32 descriptorBindingPartiallyBound;
    VkBool32 descriptorBindingVariableDescriptorCount;
    VkBool32 runtimeDescriptorArray;
    safe_VkPhysicalDeviceDescriptorIndexingFeatures(const VkPhysicalDeviceDescriptorIndexingFeatures* in_struct);
    safe_VkPhysicalDeviceDescriptorIndexingFeatures(const safe_VkPhysicalDeviceDescriptorIndexingFeatures& copy_src);
    safe_VkPhysicalDeviceDescriptorIndexingFeatures& operator=(const safe_VkPhysicalDeviceDescriptorIndexingFeatures& copy_src);
    safe_VkPhysicalDeviceDescriptorIndexingFeatures();
    ~safe_VkPhysicalDeviceDescriptorIndexingFeatures();
    void initialize(const VkPhysicalDeviceDescriptorIndexingFeatures* in_struct);
    void initialize(const safe_VkPhysicalDeviceDescriptorIndexingFeatures* copy_src);
    VkPhysicalDeviceDescriptorIndexingFeatures *ptr() { return reinterpret_cast<VkPhysicalDeviceDescriptorIndexingFeatures *>(this); }
    VkPhysicalDeviceDescriptorIndexingFeatures const *ptr() const { return reinterpret_cast<VkPhysicalDeviceDescriptorIndexingFeatures const *>(this); }
};

struct safe_VkPhysicalDeviceDescriptorIndexingProperties {
    VkStructureType sType;
    void* pNext;
    uint32_t maxUpdateAfterBindDescriptorsInAllPools;
    VkBool32 shaderUniformBufferArrayNonUniformIndexingNative;
    VkBool32 shaderSampledImageArrayNonUniformIndexingNative;
    VkBool32 shaderStorageBufferArrayNonUniformIndexingNative;
    VkBool32 shaderStorageImageArrayNonUniformIndexingNative;
    VkBool32 shaderInputAttachmentArrayNonUniformIndexingNative;
    VkBool32 robustBufferAccessUpdateAfterBind;
    VkBool32 quadDivergentImplicitLod;
    uint32_t maxPerStageDescriptorUpdateAfterBindSamplers;
    uint32_t maxPerStageDescriptorUpdateAfterBindUniformBuffers;
    uint32_t maxPerStageDescriptorUpdateAfterBindStorageBuffers;
    uint32_t maxPerStageDescriptorUpdateAfterBindSampledImages;
    uint32_t maxPerStageDescriptorUpdateAfterBindStorageImages;
    uint32_t maxPerStageDescriptorUpdateAfterBindInputAttachments;
    uint32_t maxPerStageUpdateAfterBindResources;
    uint32_t maxDescriptorSetUpdateAfterBindSamplers;
    uint32_t maxDescriptorSetUpdateAfterBindUniformBuffers;
    uint32_t maxDescriptorSetUpdateAfterBindUniformBuffersDynamic;
    uint32_t maxDescriptorSetUpdateAfterBindStorageBuffers;
    uint32_t maxDescriptorSetUpdateAfterBindStorageBuffersDynamic;
    uint32_t maxDescriptorSetUpdateAfterBindSampledImages;
    uint32_t maxDescriptorSetUpdateAfterBindStorageImages;
    uint32_t maxDescriptorSetUpdateAfterBindInputAttachments;
    safe_VkPhysicalDeviceDescriptorIndexingProperties(const VkPhysicalDeviceDescriptorIndexingProperties* in_struct);
    safe_VkPhysicalDeviceDescriptorIndexingProperties(const safe_VkPhysicalDeviceDescriptorIndexingProperties& copy_src);
    safe_VkPhysicalDeviceDescriptorIndexingProperties& operator=(const safe_VkPhysicalDeviceDescriptorIndexingProperties& copy_src);
    safe_VkPhysicalDeviceDescriptorIndexingProperties();
    ~safe_VkPhysicalDeviceDescriptorIndexingProperties();
    void initialize(const VkPhysicalDeviceDescriptorIndexingProperties* in_struct);
    void initialize(const safe_VkPhysicalDeviceDescriptorIndexingProperties* copy_src);
    VkPhysicalDeviceDescriptorIndexingProperties *ptr() { return reinterpret_cast<VkPhysicalDeviceDescriptorIndexingProperties *>(this); }
    VkPhysicalDeviceDescriptorIndexingProperties const *ptr() const { return reinterpret_cast<VkPhysicalDeviceDescriptorIndexingProperties const *>(this); }
};

struct safe_VkDescriptorSetVariableDescriptorCountAllocateInfo {
    VkStructureType sType;
    const void* pNext;
    uint32_t descriptorSetCount;
    const uint32_t* pDescriptorCounts;
    safe_VkDescriptorSetVariableDescriptorCountAllocateInfo(const VkDescriptorSetVariableDescriptorCountAllocateInfo* in_struct);
    safe_VkDescriptorSetVariableDescriptorCountAllocateInfo(const safe_VkDescriptorSetVariableDescriptorCountAllocateInfo& copy_src);
    safe_VkDescriptorSetVariableDescriptorCountAllocateInfo& operator=(const safe_VkDescriptorSetVariableDescriptorCountAllocateInfo& copy_src);
    safe_VkDescriptorSetVariableDescriptorCountAllocateInfo();
    ~safe_VkDescriptorSetVariableDescriptorCountAllocateInfo();
    void initialize(const VkDescriptorSetVariableDescriptorCountAllocateInfo* in_struct);
    void initialize(const safe_VkDescriptorSetVariableDescriptorCountAllocateInfo* copy_src);
    VkDescriptorSetVariableDescriptorCountAllocateInfo *ptr() { return reinterpret_cast<VkDescriptorSetVariableDescriptorCountAllocateInfo *>(this); }
    VkDescriptorSetVariableDescriptorCountAllocateInfo const *ptr() const { return reinterpret_cast<VkDescriptorSetVariableDescriptorCountAllocateInfo const *>(this); }
};

struct safe_VkDescriptorSetVariableDescriptorCountLayoutSupport {
    VkStructureType sType;
    void* pNext;
    uint32_t maxVariableDescriptorCount;
    safe_VkDescriptorSetVariableDescriptorCountLayoutSupport(const VkDescriptorSetVariableDescriptorCountLayoutSupport* in_struct);
    safe_VkDescriptorSetVariableDescriptorCountLayoutSupport(const safe_VkDescriptorSetVariableDescriptorCountLayoutSupport& copy_src);
    safe_VkDescriptorSetVariableDescriptorCountLayoutSupport& operator=(const safe_VkDescriptorSetVariableDescriptorCountLayoutSupport& copy_src);
    safe_VkDescriptorSetVariableDescriptorCountLayoutSupport();
    ~safe_VkDescriptorSetVariableDescriptorCountLayoutSupport();
    void initialize(const VkDescriptorSetVariableDescriptorCountLayoutSupport* in_struct);
    void initialize(const safe_VkDescriptorSetVariableDescriptorCountLayoutSupport* copy_src);
    VkDescriptorSetVariableDescriptorCountLayoutSupport *ptr() { return reinterpret_cast<VkDescriptorSetVariableDescriptorCountLayoutSupport *>(this); }
    VkDescriptorSetVariableDescriptorCountLayoutSupport const *ptr() const { return reinterpret_cast<VkDescriptorSetVariableDescriptorCountLayoutSupport const *>(this); }
};

struct safe_VkSubpassDescriptionDepthStencilResolve {
    VkStructureType sType;
    const void* pNext;
    VkResolveModeFlagBits depthResolveMode;
    VkResolveModeFlagBits stencilResolveMode;
    safe_VkAttachmentReference2* pDepthStencilResolveAttachment;
    safe_VkSubpassDescriptionDepthStencilResolve(const VkSubpassDescriptionDepthStencilResolve* in_struct);
    safe_VkSubpassDescriptionDepthStencilResolve(const safe_VkSubpassDescriptionDepthStencilResolve& copy_src);
    safe_VkSubpassDescriptionDepthStencilResolve& operator=(const safe_VkSubpassDescriptionDepthStencilResolve& copy_src);
    safe_VkSubpassDescriptionDepthStencilResolve();
    ~safe_VkSubpassDescriptionDepthStencilResolve();
    void initialize(const VkSubpassDescriptionDepthStencilResolve* in_struct);
    void initialize(const safe_VkSubpassDescriptionDepthStencilResolve* copy_src);
    VkSubpassDescriptionDepthStencilResolve *ptr() { return reinterpret_cast<VkSubpassDescriptionDepthStencilResolve *>(this); }
    VkSubpassDescriptionDepthStencilResolve const *ptr() const { return reinterpret_cast<VkSubpassDescriptionDepthStencilResolve const *>(this); }
};

struct safe_VkPhysicalDeviceDepthStencilResolveProperties {
    VkStructureType sType;
    void* pNext;
    VkResolveModeFlags supportedDepthResolveModes;
    VkResolveModeFlags supportedStencilResolveModes;
    VkBool32 independentResolveNone;
    VkBool32 independentResolve;
    safe_VkPhysicalDeviceDepthStencilResolveProperties(const VkPhysicalDeviceDepthStencilResolveProperties* in_struct);
    safe_VkPhysicalDeviceDepthStencilResolveProperties(const safe_VkPhysicalDeviceDepthStencilResolveProperties& copy_src);
    safe_VkPhysicalDeviceDepthStencilResolveProperties& operator=(const safe_VkPhysicalDeviceDepthStencilResolveProperties& copy_src);
    safe_VkPhysicalDeviceDepthStencilResolveProperties();
    ~safe_VkPhysicalDeviceDepthStencilResolveProperties();
    void initialize(const VkPhysicalDeviceDepthStencilResolveProperties* in_struct);
    void initialize(const safe_VkPhysicalDeviceDepthStencilResolveProperties* copy_src);
    VkPhysicalDeviceDepthStencilResolveProperties *ptr() { return reinterpret_cast<VkPhysicalDeviceDepthStencilResolveProperties *>(this); }
    VkPhysicalDeviceDepthStencilResolveProperties const *ptr() const { return reinterpret_cast<VkPhysicalDeviceDepthStencilResolveProperties const *>(this); }
};

struct safe_VkPhysicalDeviceScalarBlockLayoutFeatures {
    VkStructureType sType;
    void* pNext;
    VkBool32 scalarBlockLayout;
    safe_VkPhysicalDeviceScalarBlockLayoutFeatures(const VkPhysicalDeviceScalarBlockLayoutFeatures* in_struct);
    safe_VkPhysicalDeviceScalarBlockLayoutFeatures(const safe_VkPhysicalDeviceScalarBlockLayoutFeatures& copy_src);
    safe_VkPhysicalDeviceScalarBlockLayoutFeatures& operator=(const safe_VkPhysicalDeviceScalarBlockLayoutFeatures& copy_src);
    safe_VkPhysicalDeviceScalarBlockLayoutFeatures();
    ~safe_VkPhysicalDeviceScalarBlockLayoutFeatures();
    void initialize(const VkPhysicalDeviceScalarBlockLayoutFeatures* in_struct);
    void initialize(const safe_VkPhysicalDeviceScalarBlockLayoutFeatures* copy_src);
    VkPhysicalDeviceScalarBlockLayoutFeatures *ptr() { return reinterpret_cast<VkPhysicalDeviceScalarBlockLayoutFeatures *>(this); }
    VkPhysicalDeviceScalarBlockLayoutFeatures const *ptr() const { return reinterpret_cast<VkPhysicalDeviceScalarBlockLayoutFeatures const *>(this); }
};

struct safe_VkImageStencilUsageCreateInfo {
    VkStructureType sType;
    const void* pNext;
    VkImageUsageFlags stencilUsage;
    safe_VkImageStencilUsageCreateInfo(const VkImageStencilUsageCreateInfo* in_struct);
    safe_VkImageStencilUsageCreateInfo(const safe_VkImageStencilUsageCreateInfo& copy_src);
    safe_VkImageStencilUsageCreateInfo& operator=(const safe_VkImageStencilUsageCreateInfo& copy_src);
    safe_VkImageStencilUsageCreateInfo();
    ~safe_VkImageStencilUsageCreateInfo();
    void initialize(const VkImageStencilUsageCreateInfo* in_struct);
    void initialize(const safe_VkImageStencilUsageCreateInfo* copy_src);
    VkImageStencilUsageCreateInfo *ptr() { return reinterpret_cast<VkImageStencilUsageCreateInfo *>(this); }
    VkImageStencilUsageCreateInfo const *ptr() const { return reinterpret_cast<VkImageStencilUsageCreateInfo const *>(this); }
};

struct safe_VkSamplerReductionModeCreateInfo {
    VkStructureType sType;
    const void* pNext;
    VkSamplerReductionMode reductionMode;
    safe_VkSamplerReductionModeCreateInfo(const VkSamplerReductionModeCreateInfo* in_struct);
    safe_VkSamplerReductionModeCreateInfo(const safe_VkSamplerReductionModeCreateInfo& copy_src);
    safe_VkSamplerReductionModeCreateInfo& operator=(const safe_VkSamplerReductionModeCreateInfo& copy_src);
    safe_VkSamplerReductionModeCreateInfo();
    ~safe_VkSamplerReductionModeCreateInfo();
    void initialize(const VkSamplerReductionModeCreateInfo* in_struct);
    void initialize(const safe_VkSamplerReductionModeCreateInfo* copy_src);
    VkSamplerReductionModeCreateInfo *ptr() { return reinterpret_cast<VkSamplerReductionModeCreateInfo *>(this); }
    VkSamplerReductionModeCreateInfo const *ptr() const { return reinterpret_cast<VkSamplerReductionModeCreateInfo const *>(this); }
};

struct safe_VkPhysicalDeviceSamplerFilterMinmaxProperties {
    VkStructureType sType;
    void* pNext;
    VkBool32 filterMinmaxSingleComponentFormats;
    VkBool32 filterMinmaxImageComponentMapping;
    safe_VkPhysicalDeviceSamplerFilterMinmaxProperties(const VkPhysicalDeviceSamplerFilterMinmaxProperties* in_struct);
    safe_VkPhysicalDeviceSamplerFilterMinmaxProperties(const safe_VkPhysicalDeviceSamplerFilterMinmaxProperties& copy_src);
    safe_VkPhysicalDeviceSamplerFilterMinmaxProperties& operator=(const safe_VkPhysicalDeviceSamplerFilterMinmaxProperties& copy_src);
    safe_VkPhysicalDeviceSamplerFilterMinmaxProperties();
    ~safe_VkPhysicalDeviceSamplerFilterMinmaxProperties();
    void initialize(const VkPhysicalDeviceSamplerFilterMinmaxProperties* in_struct);
    void initialize(const safe_VkPhysicalDeviceSamplerFilterMinmaxProperties* copy_src);
    VkPhysicalDeviceSamplerFilterMinmaxProperties *ptr() { return reinterpret_cast<VkPhysicalDeviceSamplerFilterMinmaxProperties *>(this); }
    VkPhysicalDeviceSamplerFilterMinmaxProperties const *ptr() const { return reinterpret_cast<VkPhysicalDeviceSamplerFilterMinmaxProperties const *>(this); }
};

struct safe_VkPhysicalDeviceVulkanMemoryModelFeatures {
    VkStructureType sType;
    void* pNext;
    VkBool32 vulkanMemoryModel;
    VkBool32 vulkanMemoryModelDeviceScope;
    VkBool32 vulkanMemoryModelAvailabilityVisibilityChains;
    safe_VkPhysicalDeviceVulkanMemoryModelFeatures(const VkPhysicalDeviceVulkanMemoryModelFeatures* in_struct);
    safe_VkPhysicalDeviceVulkanMemoryModelFeatures(const safe_VkPhysicalDeviceVulkanMemoryModelFeatures& copy_src);
    safe_VkPhysicalDeviceVulkanMemoryModelFeatures& operator=(const safe_VkPhysicalDeviceVulkanMemoryModelFeatures& copy_src);
    safe_VkPhysicalDeviceVulkanMemoryModelFeatures();
    ~safe_VkPhysicalDeviceVulkanMemoryModelFeatures();
    void initialize(const VkPhysicalDeviceVulkanMemoryModelFeatures* in_struct);
    void initialize(const safe_VkPhysicalDeviceVulkanMemoryModelFeatures* copy_src);
    VkPhysicalDeviceVulkanMemoryModelFeatures *ptr() { return reinterpret_cast<VkPhysicalDeviceVulkanMemoryModelFeatures *>(this); }
    VkPhysicalDeviceVulkanMemoryModelFeatures const *ptr() const { return reinterpret_cast<VkPhysicalDeviceVulkanMemoryModelFeatures const *>(this); }
};

struct safe_VkPhysicalDeviceImagelessFramebufferFeatures {
    VkStructureType sType;
    void* pNext;
    VkBool32 imagelessFramebuffer;
    safe_VkPhysicalDeviceImagelessFramebufferFeatures(const VkPhysicalDeviceImagelessFramebufferFeatures* in_struct);
    safe_VkPhysicalDeviceImagelessFramebufferFeatures(const safe_VkPhysicalDeviceImagelessFramebufferFeatures& copy_src);
    safe_VkPhysicalDeviceImagelessFramebufferFeatures& operator=(const safe_VkPhysicalDeviceImagelessFramebufferFeatures& copy_src);
    safe_VkPhysicalDeviceImagelessFramebufferFeatures();
    ~safe_VkPhysicalDeviceImagelessFramebufferFeatures();
    void initialize(const VkPhysicalDeviceImagelessFramebufferFeatures* in_struct);
    void initialize(const safe_VkPhysicalDeviceImagelessFramebufferFeatures* copy_src);
    VkPhysicalDeviceImagelessFramebufferFeatures *ptr() { return reinterpret_cast<VkPhysicalDeviceImagelessFramebufferFeatures *>(this); }
    VkPhysicalDeviceImagelessFramebufferFeatures const *ptr() const { return reinterpret_cast<VkPhysicalDeviceImagelessFramebufferFeatures const *>(this); }
};

struct safe_VkFramebufferAttachmentImageInfo {
    VkStructureType sType;
    const void* pNext;
    VkImageCreateFlags flags;
    VkImageUsageFlags usage;
    uint32_t width;
    uint32_t height;
    uint32_t layerCount;
    uint32_t viewFormatCount;
    const VkFormat* pViewFormats;
    safe_VkFramebufferAttachmentImageInfo(const VkFramebufferAttachmentImageInfo* in_struct);
    safe_VkFramebufferAttachmentImageInfo(const safe_VkFramebufferAttachmentImageInfo& copy_src);
    safe_VkFramebufferAttachmentImageInfo& operator=(const safe_VkFramebufferAttachmentImageInfo& copy_src);
    safe_VkFramebufferAttachmentImageInfo();
    ~safe_VkFramebufferAttachmentImageInfo();
    void initialize(const VkFramebufferAttachmentImageInfo* in_struct);
    void initialize(const safe_VkFramebufferAttachmentImageInfo* copy_src);
    VkFramebufferAttachmentImageInfo *ptr() { return reinterpret_cast<VkFramebufferAttachmentImageInfo *>(this); }
    VkFramebufferAttachmentImageInfo const *ptr() const { return reinterpret_cast<VkFramebufferAttachmentImageInfo const *>(this); }
};

struct safe_VkFramebufferAttachmentsCreateInfo {
    VkStructureType sType;
    const void* pNext;
    uint32_t attachmentImageInfoCount;
    safe_VkFramebufferAttachmentImageInfo* pAttachmentImageInfos;
    safe_VkFramebufferAttachmentsCreateInfo(const VkFramebufferAttachmentsCreateInfo* in_struct);
    safe_VkFramebufferAttachmentsCreateInfo(const safe_VkFramebufferAttachmentsCreateInfo& copy_src);
    safe_VkFramebufferAttachmentsCreateInfo& operator=(const safe_VkFramebufferAttachmentsCreateInfo& copy_src);
    safe_VkFramebufferAttachmentsCreateInfo();
    ~safe_VkFramebufferAttachmentsCreateInfo();
    void initialize(const VkFramebufferAttachmentsCreateInfo* in_struct);
    void initialize(const safe_VkFramebufferAttachmentsCreateInfo* copy_src);
    VkFramebufferAttachmentsCreateInfo *ptr() { return reinterpret_cast<VkFramebufferAttachmentsCreateInfo *>(this); }
    VkFramebufferAttachmentsCreateInfo const *ptr() const { return reinterpret_cast<VkFramebufferAttachmentsCreateInfo const *>(this); }
};

struct safe_VkRenderPassAttachmentBeginInfo {
    VkStructureType sType;
    const void* pNext;
    uint32_t attachmentCount;
    VkImageView* pAttachments;
    safe_VkRenderPassAttachmentBeginInfo(const VkRenderPassAttachmentBeginInfo* in_struct);
    safe_VkRenderPassAttachmentBeginInfo(const safe_VkRenderPassAttachmentBeginInfo& copy_src);
    safe_VkRenderPassAttachmentBeginInfo& operator=(const safe_VkRenderPassAttachmentBeginInfo& copy_src);
    safe_VkRenderPassAttachmentBeginInfo();
    ~safe_VkRenderPassAttachmentBeginInfo();
    void initialize(const VkRenderPassAttachmentBeginInfo* in_struct);
    void initialize(const safe_VkRenderPassAttachmentBeginInfo* copy_src);
    VkRenderPassAttachmentBeginInfo *ptr() { return reinterpret_cast<VkRenderPassAttachmentBeginInfo *>(this); }
    VkRenderPassAttachmentBeginInfo const *ptr() const { return reinterpret_cast<VkRenderPassAttachmentBeginInfo const *>(this); }
};

struct safe_VkPhysicalDeviceUniformBufferStandardLayoutFeatures {
    VkStructureType sType;
    void* pNext;
    VkBool32 uniformBufferStandardLayout;
    safe_VkPhysicalDeviceUniformBufferStandardLayoutFeatures(const VkPhysicalDeviceUniformBufferStandardLayoutFeatures* in_struct);
    safe_VkPhysicalDeviceUniformBufferStandardLayoutFeatures(const safe_VkPhysicalDeviceUniformBufferStandardLayoutFeatures& copy_src);
    safe_VkPhysicalDeviceUniformBufferStandardLayoutFeatures& operator=(const safe_VkPhysicalDeviceUniformBufferStandardLayoutFeatures& copy_src);
    safe_VkPhysicalDeviceUniformBufferStandardLayoutFeatures();
    ~safe_VkPhysicalDeviceUniformBufferStandardLayoutFeatures();
    void initialize(const VkPhysicalDeviceUniformBufferStandardLayoutFeatures* in_struct);
    void initialize(const safe_VkPhysicalDeviceUniformBufferStandardLayoutFeatures* copy_src);
    VkPhysicalDeviceUniformBufferStandardLayoutFeatures *ptr() { return reinterpret_cast<VkPhysicalDeviceUniformBufferStandardLayoutFeatures *>(this); }
    VkPhysicalDeviceUniformBufferStandardLayoutFeatures const *ptr() const { return reinterpret_cast<VkPhysicalDeviceUniformBufferStandardLayoutFeatures const *>(this); }
};

struct safe_VkPhysicalDeviceShaderSubgroupExtendedTypesFeatures {
    VkStructureType sType;
    void* pNext;
    VkBool32 shaderSubgroupExtendedTypes;
    safe_VkPhysicalDeviceShaderSubgroupExtendedTypesFeatures(const VkPhysicalDeviceShaderSubgroupExtendedTypesFeatures* in_struct);
    safe_VkPhysicalDeviceShaderSubgroupExtendedTypesFeatures(const safe_VkPhysicalDeviceShaderSubgroupExtendedTypesFeatures& copy_src);
    safe_VkPhysicalDeviceShaderSubgroupExtendedTypesFeatures& operator=(const safe_VkPhysicalDeviceShaderSubgroupExtendedTypesFeatures& copy_src);
    safe_VkPhysicalDeviceShaderSubgroupExtendedTypesFeatures();
    ~safe_VkPhysicalDeviceShaderSubgroupExtendedTypesFeatures();
    void initialize(const VkPhysicalDeviceShaderSubgroupExtendedTypesFeatures* in_struct);
    void initialize(const safe_VkPhysicalDeviceShaderSubgroupExtendedTypesFeatures* copy_src);
    VkPhysicalDeviceShaderSubgroupExtendedTypesFeatures *ptr() { return reinterpret_cast<VkPhysicalDeviceShaderSubgroupExtendedTypesFeatures *>(this); }
    VkPhysicalDeviceShaderSubgroupExtendedTypesFeatures const *ptr() const { return reinterpret_cast<VkPhysicalDeviceShaderSubgroupExtendedTypesFeatures const *>(this); }
};

struct safe_VkPhysicalDeviceSeparateDepthStencilLayoutsFeatures {
    VkStructureType sType;
    void* pNext;
    VkBool32 separateDepthStencilLayouts;
    safe_VkPhysicalDeviceSeparateDepthStencilLayoutsFeatures(const VkPhysicalDeviceSeparateDepthStencilLayoutsFeatures* in_struct);
    safe_VkPhysicalDeviceSeparateDepthStencilLayoutsFeatures(const safe_VkPhysicalDeviceSeparateDepthStencilLayoutsFeatures& copy_src);
    safe_VkPhysicalDeviceSeparateDepthStencilLayoutsFeatures& operator=(const safe_VkPhysicalDeviceSeparateDepthStencilLayoutsFeatures& copy_src);
    safe_VkPhysicalDeviceSeparateDepthStencilLayoutsFeatures();
    ~safe_VkPhysicalDeviceSeparateDepthStencilLayoutsFeatures();
    void initialize(const VkPhysicalDeviceSeparateDepthStencilLayoutsFeatures* in_struct);
    void initialize(const safe_VkPhysicalDeviceSeparateDepthStencilLayoutsFeatures* copy_src);
    VkPhysicalDeviceSeparateDepthStencilLayoutsFeatures *ptr() { return reinterpret_cast<VkPhysicalDeviceSeparateDepthStencilLayoutsFeatures *>(this); }
    VkPhysicalDeviceSeparateDepthStencilLayoutsFeatures const *ptr() const { return reinterpret_cast<VkPhysicalDeviceSeparateDepthStencilLayoutsFeatures const *>(this); }
};

struct safe_VkAttachmentReferenceStencilLayout {
    VkStructureType sType;
    void* pNext;
    VkImageLayout stencilLayout;
    safe_VkAttachmentReferenceStencilLayout(const VkAttachmentReferenceStencilLayout* in_struct);
    safe_VkAttachmentReferenceStencilLayout(const safe_VkAttachmentReferenceStencilLayout& copy_src);
    safe_VkAttachmentReferenceStencilLayout& operator=(const safe_VkAttachmentReferenceStencilLayout& copy_src);
    safe_VkAttachmentReferenceStencilLayout();
    ~safe_VkAttachmentReferenceStencilLayout();
    void initialize(const VkAttachmentReferenceStencilLayout* in_struct);
    void initialize(const safe_VkAttachmentReferenceStencilLayout* copy_src);
    VkAttachmentReferenceStencilLayout *ptr() { return reinterpret_cast<VkAttachmentReferenceStencilLayout *>(this); }
    VkAttachmentReferenceStencilLayout const *ptr() const { return reinterpret_cast<VkAttachmentReferenceStencilLayout const *>(this); }
};

struct safe_VkAttachmentDescriptionStencilLayout {
    VkStructureType sType;
    void* pNext;
    VkImageLayout stencilInitialLayout;
    VkImageLayout stencilFinalLayout;
    safe_VkAttachmentDescriptionStencilLayout(const VkAttachmentDescriptionStencilLayout* in_struct);
    safe_VkAttachmentDescriptionStencilLayout(const safe_VkAttachmentDescriptionStencilLayout& copy_src);
    safe_VkAttachmentDescriptionStencilLayout& operator=(const safe_VkAttachmentDescriptionStencilLayout& copy_src);
    safe_VkAttachmentDescriptionStencilLayout();
    ~safe_VkAttachmentDescriptionStencilLayout();
    void initialize(const VkAttachmentDescriptionStencilLayout* in_struct);
    void initialize(const safe_VkAttachmentDescriptionStencilLayout* copy_src);
    VkAttachmentDescriptionStencilLayout *ptr() { return reinterpret_cast<VkAttachmentDescriptionStencilLayout *>(this); }
    VkAttachmentDescriptionStencilLayout const *ptr() const { return reinterpret_cast<VkAttachmentDescriptionStencilLayout const *>(this); }
};

struct safe_VkPhysicalDeviceHostQueryResetFeatures {
    VkStructureType sType;
    void* pNext;
    VkBool32 hostQueryReset;
    safe_VkPhysicalDeviceHostQueryResetFeatures(const VkPhysicalDeviceHostQueryResetFeatures* in_struct);
    safe_VkPhysicalDeviceHostQueryResetFeatures(const safe_VkPhysicalDeviceHostQueryResetFeatures& copy_src);
    safe_VkPhysicalDeviceHostQueryResetFeatures& operator=(const safe_VkPhysicalDeviceHostQueryResetFeatures& copy_src);
    safe_VkPhysicalDeviceHostQueryResetFeatures();
    ~safe_VkPhysicalDeviceHostQueryResetFeatures();
    void initialize(const VkPhysicalDeviceHostQueryResetFeatures* in_struct);
    void initialize(const safe_VkPhysicalDeviceHostQueryResetFeatures* copy_src);
    VkPhysicalDeviceHostQueryResetFeatures *ptr() { return reinterpret_cast<VkPhysicalDeviceHostQueryResetFeatures *>(this); }
    VkPhysicalDeviceHostQueryResetFeatures const *ptr() const { return reinterpret_cast<VkPhysicalDeviceHostQueryResetFeatures const *>(this); }
};

struct safe_VkPhysicalDeviceTimelineSemaphoreFeatures {
    VkStructureType sType;
    void* pNext;
    VkBool32 timelineSemaphore;
    safe_VkPhysicalDeviceTimelineSemaphoreFeatures(const VkPhysicalDeviceTimelineSemaphoreFeatures* in_struct);
    safe_VkPhysicalDeviceTimelineSemaphoreFeatures(const safe_VkPhysicalDeviceTimelineSemaphoreFeatures& copy_src);
    safe_VkPhysicalDeviceTimelineSemaphoreFeatures& operator=(const safe_VkPhysicalDeviceTimelineSemaphoreFeatures& copy_src);
    safe_VkPhysicalDeviceTimelineSemaphoreFeatures();
    ~safe_VkPhysicalDeviceTimelineSemaphoreFeatures();
    void initialize(const VkPhysicalDeviceTimelineSemaphoreFeatures* in_struct);
    void initialize(const safe_VkPhysicalDeviceTimelineSemaphoreFeatures* copy_src);
    VkPhysicalDeviceTimelineSemaphoreFeatures *ptr() { return reinterpret_cast<VkPhysicalDeviceTimelineSemaphoreFeatures *>(this); }
    VkPhysicalDeviceTimelineSemaphoreFeatures const *ptr() const { return reinterpret_cast<VkPhysicalDeviceTimelineSemaphoreFeatures const *>(this); }
};

struct safe_VkPhysicalDeviceTimelineSemaphoreProperties {
    VkStructureType sType;
    void* pNext;
    uint64_t maxTimelineSemaphoreValueDifference;
    safe_VkPhysicalDeviceTimelineSemaphoreProperties(const VkPhysicalDeviceTimelineSemaphoreProperties* in_struct);
    safe_VkPhysicalDeviceTimelineSemaphoreProperties(const safe_VkPhysicalDeviceTimelineSemaphoreProperties& copy_src);
    safe_VkPhysicalDeviceTimelineSemaphoreProperties& operator=(const safe_VkPhysicalDeviceTimelineSemaphoreProperties& copy_src);
    safe_VkPhysicalDeviceTimelineSemaphoreProperties();
    ~safe_VkPhysicalDeviceTimelineSemaphoreProperties();
    void initialize(const VkPhysicalDeviceTimelineSemaphoreProperties* in_struct);
    void initialize(const safe_VkPhysicalDeviceTimelineSemaphoreProperties* copy_src);
    VkPhysicalDeviceTimelineSemaphoreProperties *ptr() { return reinterpret_cast<VkPhysicalDeviceTimelineSemaphoreProperties *>(this); }
    VkPhysicalDeviceTimelineSemaphoreProperties const *ptr() const { return reinterpret_cast<VkPhysicalDeviceTimelineSemaphoreProperties const *>(this); }
};

struct safe_VkSemaphoreTypeCreateInfo {
    VkStructureType sType;
    const void* pNext;
    VkSemaphoreType semaphoreType;
    uint64_t initialValue;
    safe_VkSemaphoreTypeCreateInfo(const VkSemaphoreTypeCreateInfo* in_struct);
    safe_VkSemaphoreTypeCreateInfo(const safe_VkSemaphoreTypeCreateInfo& copy_src);
    safe_VkSemaphoreTypeCreateInfo& operator=(const safe_VkSemaphoreTypeCreateInfo& copy_src);
    safe_VkSemaphoreTypeCreateInfo();
    ~safe_VkSemaphoreTypeCreateInfo();
    void initialize(const VkSemaphoreTypeCreateInfo* in_struct);
    void initialize(const safe_VkSemaphoreTypeCreateInfo* copy_src);
    VkSemaphoreTypeCreateInfo *ptr() { return reinterpret_cast<VkSemaphoreTypeCreateInfo *>(this); }
    VkSemaphoreTypeCreateInfo const *ptr() const { return reinterpret_cast<VkSemaphoreTypeCreateInfo const *>(this); }
};

struct safe_VkTimelineSemaphoreSubmitInfo {
    VkStructureType sType;
    const void* pNext;
    uint32_t waitSemaphoreValueCount;
    const uint64_t* pWaitSemaphoreValues;
    uint32_t signalSemaphoreValueCount;
    const uint64_t* pSignalSemaphoreValues;
    safe_VkTimelineSemaphoreSubmitInfo(const VkTimelineSemaphoreSubmitInfo* in_struct);
    safe_VkTimelineSemaphoreSubmitInfo(const safe_VkTimelineSemaphoreSubmitInfo& copy_src);
    safe_VkTimelineSemaphoreSubmitInfo& operator=(const safe_VkTimelineSemaphoreSubmitInfo& copy_src);
    safe_VkTimelineSemaphoreSubmitInfo();
    ~safe_VkTimelineSemaphoreSubmitInfo();
    void initialize(const VkTimelineSemaphoreSubmitInfo* in_struct);
    void initialize(const safe_VkTimelineSemaphoreSubmitInfo* copy_src);
    VkTimelineSemaphoreSubmitInfo *ptr() { return reinterpret_cast<VkTimelineSemaphoreSubmitInfo *>(this); }
    VkTimelineSemaphoreSubmitInfo const *ptr() const { return reinterpret_cast<VkTimelineSemaphoreSubmitInfo const *>(this); }
};

struct safe_VkSemaphoreWaitInfo {
    VkStructureType sType;
    const void* pNext;
    VkSemaphoreWaitFlags flags;
    uint32_t semaphoreCount;
    VkSemaphore* pSemaphores;
    const uint64_t* pValues;
    safe_VkSemaphoreWaitInfo(const VkSemaphoreWaitInfo* in_struct);
    safe_VkSemaphoreWaitInfo(const safe_VkSemaphoreWaitInfo& copy_src);
    safe_VkSemaphoreWaitInfo& operator=(const safe_VkSemaphoreWaitInfo& copy_src);
    safe_VkSemaphoreWaitInfo();
    ~safe_VkSemaphoreWaitInfo();
    void initialize(const VkSemaphoreWaitInfo* in_struct);
    void initialize(const safe_VkSemaphoreWaitInfo* copy_src);
    VkSemaphoreWaitInfo *ptr() { return reinterpret_cast<VkSemaphoreWaitInfo *>(this); }
    VkSemaphoreWaitInfo const *ptr() const { return reinterpret_cast<VkSemaphoreWaitInfo const *>(this); }
};

struct safe_VkSemaphoreSignalInfo {
    VkStructureType sType;
    const void* pNext;
    VkSemaphore semaphore;
    uint64_t value;
    safe_VkSemaphoreSignalInfo(const VkSemaphoreSignalInfo* in_struct);
    safe_VkSemaphoreSignalInfo(const safe_VkSemaphoreSignalInfo& copy_src);
    safe_VkSemaphoreSignalInfo& operator=(const safe_VkSemaphoreSignalInfo& copy_src);
    safe_VkSemaphoreSignalInfo();
    ~safe_VkSemaphoreSignalInfo();
    void initialize(const VkSemaphoreSignalInfo* in_struct);
    void initialize(const safe_VkSemaphoreSignalInfo* copy_src);
    VkSemaphoreSignalInfo *ptr() { return reinterpret_cast<VkSemaphoreSignalInfo *>(this); }
    VkSemaphoreSignalInfo const *ptr() const { return reinterpret_cast<VkSemaphoreSignalInfo const *>(this); }
};

struct safe_VkPhysicalDeviceBufferDeviceAddressFeatures {
    VkStructureType sType;
    void* pNext;
    VkBool32 bufferDeviceAddress;
    VkBool32 bufferDeviceAddressCaptureReplay;
    VkBool32 bufferDeviceAddressMultiDevice;
    safe_VkPhysicalDeviceBufferDeviceAddressFeatures(const VkPhysicalDeviceBufferDeviceAddressFeatures* in_struct);
    safe_VkPhysicalDeviceBufferDeviceAddressFeatures(const safe_VkPhysicalDeviceBufferDeviceAddressFeatures& copy_src);
    safe_VkPhysicalDeviceBufferDeviceAddressFeatures& operator=(const safe_VkPhysicalDeviceBufferDeviceAddressFeatures& copy_src);
    safe_VkPhysicalDeviceBufferDeviceAddressFeatures();
    ~safe_VkPhysicalDeviceBufferDeviceAddressFeatures();
    void initialize(const VkPhysicalDeviceBufferDeviceAddressFeatures* in_struct);
    void initialize(const safe_VkPhysicalDeviceBufferDeviceAddressFeatures* copy_src);
    VkPhysicalDeviceBufferDeviceAddressFeatures *ptr() { return reinterpret_cast<VkPhysicalDeviceBufferDeviceAddressFeatures *>(this); }
    VkPhysicalDeviceBufferDeviceAddressFeatures const *ptr() const { return reinterpret_cast<VkPhysicalDeviceBufferDeviceAddressFeatures const *>(this); }
};

struct safe_VkBufferDeviceAddressInfo {
    VkStructureType sType;
    const void* pNext;
    VkBuffer buffer;
    safe_VkBufferDeviceAddressInfo(const VkBufferDeviceAddressInfo* in_struct);
    safe_VkBufferDeviceAddressInfo(const safe_VkBufferDeviceAddressInfo& copy_src);
    safe_VkBufferDeviceAddressInfo& operator=(const safe_VkBufferDeviceAddressInfo& copy_src);
    safe_VkBufferDeviceAddressInfo();
    ~safe_VkBufferDeviceAddressInfo();
    void initialize(const VkBufferDeviceAddressInfo* in_struct);
    void initialize(const safe_VkBufferDeviceAddressInfo* copy_src);
    VkBufferDeviceAddressInfo *ptr() { return reinterpret_cast<VkBufferDeviceAddressInfo *>(this); }
    VkBufferDeviceAddressInfo const *ptr() const { return reinterpret_cast<VkBufferDeviceAddressInfo const *>(this); }
};

struct safe_VkBufferOpaqueCaptureAddressCreateInfo {
    VkStructureType sType;
    const void* pNext;
    uint64_t opaqueCaptureAddress;
    safe_VkBufferOpaqueCaptureAddressCreateInfo(const VkBufferOpaqueCaptureAddressCreateInfo* in_struct);
    safe_VkBufferOpaqueCaptureAddressCreateInfo(const safe_VkBufferOpaqueCaptureAddressCreateInfo& copy_src);
    safe_VkBufferOpaqueCaptureAddressCreateInfo& operator=(const safe_VkBufferOpaqueCaptureAddressCreateInfo& copy_src);
    safe_VkBufferOpaqueCaptureAddressCreateInfo();
    ~safe_VkBufferOpaqueCaptureAddressCreateInfo();
    void initialize(const VkBufferOpaqueCaptureAddressCreateInfo* in_struct);
    void initialize(const safe_VkBufferOpaqueCaptureAddressCreateInfo* copy_src);
    VkBufferOpaqueCaptureAddressCreateInfo *ptr() { return reinterpret_cast<VkBufferOpaqueCaptureAddressCreateInfo *>(this); }
    VkBufferOpaqueCaptureAddressCreateInfo const *ptr() const { return reinterpret_cast<VkBufferOpaqueCaptureAddressCreateInfo const *>(this); }
};

struct safe_VkMemoryOpaqueCaptureAddressAllocateInfo {
    VkStructureType sType;
    const void* pNext;
    uint64_t opaqueCaptureAddress;
    safe_VkMemoryOpaqueCaptureAddressAllocateInfo(const VkMemoryOpaqueCaptureAddressAllocateInfo* in_struct);
    safe_VkMemoryOpaqueCaptureAddressAllocateInfo(const safe_VkMemoryOpaqueCaptureAddressAllocateInfo& copy_src);
    safe_VkMemoryOpaqueCaptureAddressAllocateInfo& operator=(const safe_VkMemoryOpaqueCaptureAddressAllocateInfo& copy_src);
    safe_VkMemoryOpaqueCaptureAddressAllocateInfo();
    ~safe_VkMemoryOpaqueCaptureAddressAllocateInfo();
    void initialize(const VkMemoryOpaqueCaptureAddressAllocateInfo* in_struct);
    void initialize(const safe_VkMemoryOpaqueCaptureAddressAllocateInfo* copy_src);
    VkMemoryOpaqueCaptureAddressAllocateInfo *ptr() { return reinterpret_cast<VkMemoryOpaqueCaptureAddressAllocateInfo *>(this); }
    VkMemoryOpaqueCaptureAddressAllocateInfo const *ptr() const { return reinterpret_cast<VkMemoryOpaqueCaptureAddressAllocateInfo const *>(this); }
};

struct safe_VkDeviceMemoryOpaqueCaptureAddressInfo {
    VkStructureType sType;
    const void* pNext;
    VkDeviceMemory memory;
    safe_VkDeviceMemoryOpaqueCaptureAddressInfo(const VkDeviceMemoryOpaqueCaptureAddressInfo* in_struct);
    safe_VkDeviceMemoryOpaqueCaptureAddressInfo(const safe_VkDeviceMemoryOpaqueCaptureAddressInfo& copy_src);
    safe_VkDeviceMemoryOpaqueCaptureAddressInfo& operator=(const safe_VkDeviceMemoryOpaqueCaptureAddressInfo& copy_src);
    safe_VkDeviceMemoryOpaqueCaptureAddressInfo();
    ~safe_VkDeviceMemoryOpaqueCaptureAddressInfo();
    void initialize(const VkDeviceMemoryOpaqueCaptureAddressInfo* in_struct);
    void initialize(const safe_VkDeviceMemoryOpaqueCaptureAddressInfo* copy_src);
    VkDeviceMemoryOpaqueCaptureAddressInfo *ptr() { return reinterpret_cast<VkDeviceMemoryOpaqueCaptureAddressInfo *>(this); }
    VkDeviceMemoryOpaqueCaptureAddressInfo const *ptr() const { return reinterpret_cast<VkDeviceMemoryOpaqueCaptureAddressInfo const *>(this); }
};

struct safe_VkPhysicalDeviceVulkanSC10Features {
    VkStructureType sType;
    void* pNext;
    VkBool32 shaderAtomicInstructions;
    safe_VkPhysicalDeviceVulkanSC10Features(const VkPhysicalDeviceVulkanSC10Features* in_struct);
    safe_VkPhysicalDeviceVulkanSC10Features(const safe_VkPhysicalDeviceVulkanSC10Features& copy_src);
    safe_VkPhysicalDeviceVulkanSC10Features& operator=(const safe_VkPhysicalDeviceVulkanSC10Features& copy_src);
    safe_VkPhysicalDeviceVulkanSC10Features();
    ~safe_VkPhysicalDeviceVulkanSC10Features();
    void initialize(const VkPhysicalDeviceVulkanSC10Features* in_struct);
    void initialize(const safe_VkPhysicalDeviceVulkanSC10Features* copy_src);
    VkPhysicalDeviceVulkanSC10Features *ptr() { return reinterpret_cast<VkPhysicalDeviceVulkanSC10Features *>(this); }
    VkPhysicalDeviceVulkanSC10Features const *ptr() const { return reinterpret_cast<VkPhysicalDeviceVulkanSC10Features const *>(this); }
};

struct safe_VkPhysicalDeviceVulkanSC10Properties {
    VkStructureType sType;
    void* pNext;
    VkBool32 deviceNoDynamicHostAllocations;
    VkBool32 deviceDestroyFreesMemory;
    VkBool32 commandPoolMultipleCommandBuffersRecording;
    VkBool32 commandPoolResetCommandBuffer;
    VkBool32 commandBufferSimultaneousUse;
    VkBool32 secondaryCommandBufferNullOrImagelessFramebuffer;
    VkBool32 recycleDescriptorSetMemory;
    VkBool32 recyclePipelineMemory;
    uint32_t maxRenderPassSubpasses;
    uint32_t maxRenderPassDependencies;
    uint32_t maxSubpassInputAttachments;
    uint32_t maxSubpassPreserveAttachments;
    uint32_t maxFramebufferAttachments;
    uint32_t maxDescriptorSetLayoutBindings;
    uint32_t maxQueryFaultCount;
    uint32_t maxCallbackFaultCount;
    safe_VkPhysicalDeviceVulkanSC10Properties(const VkPhysicalDeviceVulkanSC10Properties* in_struct);
    safe_VkPhysicalDeviceVulkanSC10Properties(const safe_VkPhysicalDeviceVulkanSC10Properties& copy_src);
    safe_VkPhysicalDeviceVulkanSC10Properties& operator=(const safe_VkPhysicalDeviceVulkanSC10Properties& copy_src);
    safe_VkPhysicalDeviceVulkanSC10Properties();
    ~safe_VkPhysicalDeviceVulkanSC10Properties();
    void initialize(const VkPhysicalDeviceVulkanSC10Properties* in_struct);
    void initialize(const safe_VkPhysicalDeviceVulkanSC10Properties* copy_src);
    VkPhysicalDeviceVulkanSC10Properties *ptr() { return reinterpret_cast<VkPhysicalDeviceVulkanSC10Properties *>(this); }
    VkPhysicalDeviceVulkanSC10Properties const *ptr() const { return reinterpret_cast<VkPhysicalDeviceVulkanSC10Properties const *>(this); }
};

struct safe_VkPipelinePoolSize {
    VkStructureType sType;
    const void* pNext;
    VkDeviceSize poolEntrySize;
    uint32_t poolEntryCount;
    safe_VkPipelinePoolSize(const VkPipelinePoolSize* in_struct);
    safe_VkPipelinePoolSize(const safe_VkPipelinePoolSize& copy_src);
    safe_VkPipelinePoolSize& operator=(const safe_VkPipelinePoolSize& copy_src);
    safe_VkPipelinePoolSize();
    ~safe_VkPipelinePoolSize();
    void initialize(const VkPipelinePoolSize* in_struct);
    void initialize(const safe_VkPipelinePoolSize* copy_src);
    VkPipelinePoolSize *ptr() { return reinterpret_cast<VkPipelinePoolSize *>(this); }
    VkPipelinePoolSize const *ptr() const { return reinterpret_cast<VkPipelinePoolSize const *>(this); }
};

struct safe_VkDeviceObjectReservationCreateInfo {
    VkStructureType sType;
    const void* pNext;
    uint32_t pipelineCacheCreateInfoCount;
    safe_VkPipelineCacheCreateInfo* pPipelineCacheCreateInfos;
    uint32_t pipelinePoolSizeCount;
    safe_VkPipelinePoolSize* pPipelinePoolSizes;
    uint32_t semaphoreRequestCount;
    uint32_t commandBufferRequestCount;
    uint32_t fenceRequestCount;
    uint32_t deviceMemoryRequestCount;
    uint32_t bufferRequestCount;
    uint32_t imageRequestCount;
    uint32_t eventRequestCount;
    uint32_t queryPoolRequestCount;
    uint32_t bufferViewRequestCount;
    uint32_t imageViewRequestCount;
    uint32_t layeredImageViewRequestCount;
    uint32_t pipelineCacheRequestCount;
    uint32_t pipelineLayoutRequestCount;
    uint32_t renderPassRequestCount;
    uint32_t graphicsPipelineRequestCount;
    uint32_t computePipelineRequestCount;
    uint32_t descriptorSetLayoutRequestCount;
    uint32_t samplerRequestCount;
    uint32_t descriptorPoolRequestCount;
    uint32_t descriptorSetRequestCount;
    uint32_t framebufferRequestCount;
    uint32_t commandPoolRequestCount;
    uint32_t samplerYcbcrConversionRequestCount;
    uint32_t surfaceRequestCount;
    uint32_t swapchainRequestCount;
    uint32_t displayModeRequestCount;
    uint32_t subpassDescriptionRequestCount;
    uint32_t attachmentDescriptionRequestCount;
    uint32_t descriptorSetLayoutBindingRequestCount;
    uint32_t descriptorSetLayoutBindingLimit;
    uint32_t maxImageViewMipLevels;
    uint32_t maxImageViewArrayLayers;
    uint32_t maxLayeredImageViewMipLevels;
    uint32_t maxOcclusionQueriesPerPool;
    uint32_t maxPipelineStatisticsQueriesPerPool;
    uint32_t maxTimestampQueriesPerPool;
    uint32_t maxImmutableSamplersPerDescriptorSetLayout;
    safe_VkDeviceObjectReservationCreateInfo(const VkDeviceObjectReservationCreateInfo* in_struct);
    safe_VkDeviceObjectReservationCreateInfo(const safe_VkDeviceObjectReservationCreateInfo& copy_src);
    safe_VkDeviceObjectReservationCreateInfo& operator=(const safe_VkDeviceObjectReservationCreateInfo& copy_src);
    safe_VkDeviceObjectReservationCreateInfo();
    ~safe_VkDeviceObjectReservationCreateInfo();
    void initialize(const VkDeviceObjectReservationCreateInfo* in_struct);
    void initialize(const safe_VkDeviceObjectReservationCreateInfo* copy_src);
    VkDeviceObjectReservationCreateInfo *ptr() { return reinterpret_cast<VkDeviceObjectReservationCreateInfo *>(this); }
    VkDeviceObjectReservationCreateInfo const *ptr() const { return reinterpret_cast<VkDeviceObjectReservationCreateInfo const *>(this); }
};

struct safe_VkCommandPoolMemoryReservationCreateInfo {
    VkStructureType sType;
    const void* pNext;
    VkDeviceSize commandPoolReservedSize;
    uint32_t commandPoolMaxCommandBuffers;
    safe_VkCommandPoolMemoryReservationCreateInfo(const VkCommandPoolMemoryReservationCreateInfo* in_struct);
    safe_VkCommandPoolMemoryReservationCreateInfo(const safe_VkCommandPoolMemoryReservationCreateInfo& copy_src);
    safe_VkCommandPoolMemoryReservationCreateInfo& operator=(const safe_VkCommandPoolMemoryReservationCreateInfo& copy_src);
    safe_VkCommandPoolMemoryReservationCreateInfo();
    ~safe_VkCommandPoolMemoryReservationCreateInfo();
    void initialize(const VkCommandPoolMemoryReservationCreateInfo* in_struct);
    void initialize(const safe_VkCommandPoolMemoryReservationCreateInfo* copy_src);
    VkCommandPoolMemoryReservationCreateInfo *ptr() { return reinterpret_cast<VkCommandPoolMemoryReservationCreateInfo *>(this); }
    VkCommandPoolMemoryReservationCreateInfo const *ptr() const { return reinterpret_cast<VkCommandPoolMemoryReservationCreateInfo const *>(this); }
};

struct safe_VkCommandPoolMemoryConsumption {
    VkStructureType sType;
    void* pNext;
    VkDeviceSize commandPoolAllocated;
    VkDeviceSize commandPoolReservedSize;
    VkDeviceSize commandBufferAllocated;
    safe_VkCommandPoolMemoryConsumption(const VkCommandPoolMemoryConsumption* in_struct);
    safe_VkCommandPoolMemoryConsumption(const safe_VkCommandPoolMemoryConsumption& copy_src);
    safe_VkCommandPoolMemoryConsumption& operator=(const safe_VkCommandPoolMemoryConsumption& copy_src);
    safe_VkCommandPoolMemoryConsumption();
    ~safe_VkCommandPoolMemoryConsumption();
    void initialize(const VkCommandPoolMemoryConsumption* in_struct);
    void initialize(const safe_VkCommandPoolMemoryConsumption* copy_src);
    VkCommandPoolMemoryConsumption *ptr() { return reinterpret_cast<VkCommandPoolMemoryConsumption *>(this); }
    VkCommandPoolMemoryConsumption const *ptr() const { return reinterpret_cast<VkCommandPoolMemoryConsumption const *>(this); }
};

struct safe_VkFaultData {
    VkStructureType sType;
    void* pNext;
    VkFaultLevel faultLevel;
    VkFaultType faultType;
    safe_VkFaultData(const VkFaultData* in_struct);
    safe_VkFaultData(const safe_VkFaultData& copy_src);
    safe_VkFaultData& operator=(const safe_VkFaultData& copy_src);
    safe_VkFaultData();
    ~safe_VkFaultData();
    void initialize(const VkFaultData* in_struct);
    void initialize(const safe_VkFaultData* copy_src);
    VkFaultData *ptr() { return reinterpret_cast<VkFaultData *>(this); }
    VkFaultData const *ptr() const { return reinterpret_cast<VkFaultData const *>(this); }
};

struct safe_VkFaultCallbackInfo {
    VkStructureType sType;
    void* pNext;
    uint32_t faultCount;
    safe_VkFaultData* pFaults;
    PFN_vkFaultCallbackFunction pfnFaultCallback;
    safe_VkFaultCallbackInfo(const VkFaultCallbackInfo* in_struct);
    safe_VkFaultCallbackInfo(const safe_VkFaultCallbackInfo& copy_src);
    safe_VkFaultCallbackInfo& operator=(const safe_VkFaultCallbackInfo& copy_src);
    safe_VkFaultCallbackInfo();
    ~safe_VkFaultCallbackInfo();
    void initialize(const VkFaultCallbackInfo* in_struct);
    void initialize(const safe_VkFaultCallbackInfo* copy_src);
    VkFaultCallbackInfo *ptr() { return reinterpret_cast<VkFaultCallbackInfo *>(this); }
    VkFaultCallbackInfo const *ptr() const { return reinterpret_cast<VkFaultCallbackInfo const *>(this); }
};

struct safe_VkPipelineOfflineCreateInfo {
    VkStructureType sType;
    const void* pNext;
    uint8_t pipelineIdentifier[VK_UUID_SIZE];
    VkPipelineMatchControl matchControl;
    VkDeviceSize poolEntrySize;
    safe_VkPipelineOfflineCreateInfo(const VkPipelineOfflineCreateInfo* in_struct);
    safe_VkPipelineOfflineCreateInfo(const safe_VkPipelineOfflineCreateInfo& copy_src);
    safe_VkPipelineOfflineCreateInfo& operator=(const safe_VkPipelineOfflineCreateInfo& copy_src);
    safe_VkPipelineOfflineCreateInfo();
    ~safe_VkPipelineOfflineCreateInfo();
    void initialize(const VkPipelineOfflineCreateInfo* in_struct);
    void initialize(const safe_VkPipelineOfflineCreateInfo* copy_src);
    VkPipelineOfflineCreateInfo *ptr() { return reinterpret_cast<VkPipelineOfflineCreateInfo *>(this); }
    VkPipelineOfflineCreateInfo const *ptr() const { return reinterpret_cast<VkPipelineOfflineCreateInfo const *>(this); }
};

struct safe_VkSwapchainCreateInfoKHR {
    VkStructureType sType;
    const void* pNext;
    VkSwapchainCreateFlagsKHR flags;
    VkSurfaceKHR surface;
    uint32_t minImageCount;
    VkFormat imageFormat;
    VkColorSpaceKHR imageColorSpace;
    VkExtent2D imageExtent;
    uint32_t imageArrayLayers;
    VkImageUsageFlags imageUsage;
    VkSharingMode imageSharingMode;
    uint32_t queueFamilyIndexCount;
    const uint32_t* pQueueFamilyIndices;
    VkSurfaceTransformFlagBitsKHR preTransform;
    VkCompositeAlphaFlagBitsKHR compositeAlpha;
    VkPresentModeKHR presentMode;
    VkBool32 clipped;
    VkSwapchainKHR oldSwapchain;
    safe_VkSwapchainCreateInfoKHR(const VkSwapchainCreateInfoKHR* in_struct);
    safe_VkSwapchainCreateInfoKHR(const safe_VkSwapchainCreateInfoKHR& copy_src);
    safe_VkSwapchainCreateInfoKHR& operator=(const safe_VkSwapchainCreateInfoKHR& copy_src);
    safe_VkSwapchainCreateInfoKHR();
    ~safe_VkSwapchainCreateInfoKHR();
    void initialize(const VkSwapchainCreateInfoKHR* in_struct);
    void initialize(const safe_VkSwapchainCreateInfoKHR* copy_src);
    VkSwapchainCreateInfoKHR *ptr() { return reinterpret_cast<VkSwapchainCreateInfoKHR *>(this); }
    VkSwapchainCreateInfoKHR const *ptr() const { return reinterpret_cast<VkSwapchainCreateInfoKHR const *>(this); }
};

struct safe_VkPresentInfoKHR {
    VkStructureType sType;
    const void* pNext;
    uint32_t waitSemaphoreCount;
    VkSemaphore* pWaitSemaphores;
    uint32_t swapchainCount;
    VkSwapchainKHR* pSwapchains;
    const uint32_t* pImageIndices;
    VkResult* pResults;
    safe_VkPresentInfoKHR(const VkPresentInfoKHR* in_struct);
    safe_VkPresentInfoKHR(const safe_VkPresentInfoKHR& copy_src);
    safe_VkPresentInfoKHR& operator=(const safe_VkPresentInfoKHR& copy_src);
    safe_VkPresentInfoKHR();
    ~safe_VkPresentInfoKHR();
    void initialize(const VkPresentInfoKHR* in_struct);
    void initialize(const safe_VkPresentInfoKHR* copy_src);
    VkPresentInfoKHR *ptr() { return reinterpret_cast<VkPresentInfoKHR *>(this); }
    VkPresentInfoKHR const *ptr() const { return reinterpret_cast<VkPresentInfoKHR const *>(this); }
};

struct safe_VkImageSwapchainCreateInfoKHR {
    VkStructureType sType;
    const void* pNext;
    VkSwapchainKHR swapchain;
    safe_VkImageSwapchainCreateInfoKHR(const VkImageSwapchainCreateInfoKHR* in_struct);
    safe_VkImageSwapchainCreateInfoKHR(const safe_VkImageSwapchainCreateInfoKHR& copy_src);
    safe_VkImageSwapchainCreateInfoKHR& operator=(const safe_VkImageSwapchainCreateInfoKHR& copy_src);
    safe_VkImageSwapchainCreateInfoKHR();
    ~safe_VkImageSwapchainCreateInfoKHR();
    void initialize(const VkImageSwapchainCreateInfoKHR* in_struct);
    void initialize(const safe_VkImageSwapchainCreateInfoKHR* copy_src);
    VkImageSwapchainCreateInfoKHR *ptr() { return reinterpret_cast<VkImageSwapchainCreateInfoKHR *>(this); }
    VkImageSwapchainCreateInfoKHR const *ptr() const { return reinterpret_cast<VkImageSwapchainCreateInfoKHR const *>(this); }
};

struct safe_VkBindImageMemorySwapchainInfoKHR {
    VkStructureType sType;
    const void* pNext;
    VkSwapchainKHR swapchain;
    uint32_t imageIndex;
    safe_VkBindImageMemorySwapchainInfoKHR(const VkBindImageMemorySwapchainInfoKHR* in_struct);
    safe_VkBindImageMemorySwapchainInfoKHR(const safe_VkBindImageMemorySwapchainInfoKHR& copy_src);
    safe_VkBindImageMemorySwapchainInfoKHR& operator=(const safe_VkBindImageMemorySwapchainInfoKHR& copy_src);
    safe_VkBindImageMemorySwapchainInfoKHR();
    ~safe_VkBindImageMemorySwapchainInfoKHR();
    void initialize(const VkBindImageMemorySwapchainInfoKHR* in_struct);
    void initialize(const safe_VkBindImageMemorySwapchainInfoKHR* copy_src);
    VkBindImageMemorySwapchainInfoKHR *ptr() { return reinterpret_cast<VkBindImageMemorySwapchainInfoKHR *>(this); }
    VkBindImageMemorySwapchainInfoKHR const *ptr() const { return reinterpret_cast<VkBindImageMemorySwapchainInfoKHR const *>(this); }
};

struct safe_VkAcquireNextImageInfoKHR {
    VkStructureType sType;
    const void* pNext;
    VkSwapchainKHR swapchain;
    uint64_t timeout;
    VkSemaphore semaphore;
    VkFence fence;
    uint32_t deviceMask;
    safe_VkAcquireNextImageInfoKHR(const VkAcquireNextImageInfoKHR* in_struct);
    safe_VkAcquireNextImageInfoKHR(const safe_VkAcquireNextImageInfoKHR& copy_src);
    safe_VkAcquireNextImageInfoKHR& operator=(const safe_VkAcquireNextImageInfoKHR& copy_src);
    safe_VkAcquireNextImageInfoKHR();
    ~safe_VkAcquireNextImageInfoKHR();
    void initialize(const VkAcquireNextImageInfoKHR* in_struct);
    void initialize(const safe_VkAcquireNextImageInfoKHR* copy_src);
    VkAcquireNextImageInfoKHR *ptr() { return reinterpret_cast<VkAcquireNextImageInfoKHR *>(this); }
    VkAcquireNextImageInfoKHR const *ptr() const { return reinterpret_cast<VkAcquireNextImageInfoKHR const *>(this); }
};

struct safe_VkDeviceGroupPresentCapabilitiesKHR {
    VkStructureType sType;
    void* pNext;
    uint32_t presentMask[VK_MAX_DEVICE_GROUP_SIZE];
    VkDeviceGroupPresentModeFlagsKHR modes;
    safe_VkDeviceGroupPresentCapabilitiesKHR(const VkDeviceGroupPresentCapabilitiesKHR* in_struct);
    safe_VkDeviceGroupPresentCapabilitiesKHR(const safe_VkDeviceGroupPresentCapabilitiesKHR& copy_src);
    safe_VkDeviceGroupPresentCapabilitiesKHR& operator=(const safe_VkDeviceGroupPresentCapabilitiesKHR& copy_src);
    safe_VkDeviceGroupPresentCapabilitiesKHR();
    ~safe_VkDeviceGroupPresentCapabilitiesKHR();
    void initialize(const VkDeviceGroupPresentCapabilitiesKHR* in_struct);
    void initialize(const safe_VkDeviceGroupPresentCapabilitiesKHR* copy_src);
    VkDeviceGroupPresentCapabilitiesKHR *ptr() { return reinterpret_cast<VkDeviceGroupPresentCapabilitiesKHR *>(this); }
    VkDeviceGroupPresentCapabilitiesKHR const *ptr() const { return reinterpret_cast<VkDeviceGroupPresentCapabilitiesKHR const *>(this); }
};

struct safe_VkDeviceGroupPresentInfoKHR {
    VkStructureType sType;
    const void* pNext;
    uint32_t swapchainCount;
    const uint32_t* pDeviceMasks;
    VkDeviceGroupPresentModeFlagBitsKHR mode;
    safe_VkDeviceGroupPresentInfoKHR(const VkDeviceGroupPresentInfoKHR* in_struct);
    safe_VkDeviceGroupPresentInfoKHR(const safe_VkDeviceGroupPresentInfoKHR& copy_src);
    safe_VkDeviceGroupPresentInfoKHR& operator=(const safe_VkDeviceGroupPresentInfoKHR& copy_src);
    safe_VkDeviceGroupPresentInfoKHR();
    ~safe_VkDeviceGroupPresentInfoKHR();
    void initialize(const VkDeviceGroupPresentInfoKHR* in_struct);
    void initialize(const safe_VkDeviceGroupPresentInfoKHR* copy_src);
    VkDeviceGroupPresentInfoKHR *ptr() { return reinterpret_cast<VkDeviceGroupPresentInfoKHR *>(this); }
    VkDeviceGroupPresentInfoKHR const *ptr() const { return reinterpret_cast<VkDeviceGroupPresentInfoKHR const *>(this); }
};

struct safe_VkDeviceGroupSwapchainCreateInfoKHR {
    VkStructureType sType;
    const void* pNext;
    VkDeviceGroupPresentModeFlagsKHR modes;
    safe_VkDeviceGroupSwapchainCreateInfoKHR(const VkDeviceGroupSwapchainCreateInfoKHR* in_struct);
    safe_VkDeviceGroupSwapchainCreateInfoKHR(const safe_VkDeviceGroupSwapchainCreateInfoKHR& copy_src);
    safe_VkDeviceGroupSwapchainCreateInfoKHR& operator=(const safe_VkDeviceGroupSwapchainCreateInfoKHR& copy_src);
    safe_VkDeviceGroupSwapchainCreateInfoKHR();
    ~safe_VkDeviceGroupSwapchainCreateInfoKHR();
    void initialize(const VkDeviceGroupSwapchainCreateInfoKHR* in_struct);
    void initialize(const safe_VkDeviceGroupSwapchainCreateInfoKHR* copy_src);
    VkDeviceGroupSwapchainCreateInfoKHR *ptr() { return reinterpret_cast<VkDeviceGroupSwapchainCreateInfoKHR *>(this); }
    VkDeviceGroupSwapchainCreateInfoKHR const *ptr() const { return reinterpret_cast<VkDeviceGroupSwapchainCreateInfoKHR const *>(this); }
};

struct safe_VkDisplayModeCreateInfoKHR {
    VkStructureType sType;
    const void* pNext;
    VkDisplayModeCreateFlagsKHR flags;
    VkDisplayModeParametersKHR parameters;
    safe_VkDisplayModeCreateInfoKHR(const VkDisplayModeCreateInfoKHR* in_struct);
    safe_VkDisplayModeCreateInfoKHR(const safe_VkDisplayModeCreateInfoKHR& copy_src);
    safe_VkDisplayModeCreateInfoKHR& operator=(const safe_VkDisplayModeCreateInfoKHR& copy_src);
    safe_VkDisplayModeCreateInfoKHR();
    ~safe_VkDisplayModeCreateInfoKHR();
    void initialize(const VkDisplayModeCreateInfoKHR* in_struct);
    void initialize(const safe_VkDisplayModeCreateInfoKHR* copy_src);
    VkDisplayModeCreateInfoKHR *ptr() { return reinterpret_cast<VkDisplayModeCreateInfoKHR *>(this); }
    VkDisplayModeCreateInfoKHR const *ptr() const { return reinterpret_cast<VkDisplayModeCreateInfoKHR const *>(this); }
};

struct safe_VkDisplayPropertiesKHR {
    VkDisplayKHR display;
    const char* displayName;
    VkExtent2D physicalDimensions;
    VkExtent2D physicalResolution;
    VkSurfaceTransformFlagsKHR supportedTransforms;
    VkBool32 planeReorderPossible;
    VkBool32 persistentContent;
    safe_VkDisplayPropertiesKHR(const VkDisplayPropertiesKHR* in_struct);
    safe_VkDisplayPropertiesKHR(const safe_VkDisplayPropertiesKHR& copy_src);
    safe_VkDisplayPropertiesKHR& operator=(const safe_VkDisplayPropertiesKHR& copy_src);
    safe_VkDisplayPropertiesKHR();
    ~safe_VkDisplayPropertiesKHR();
    void initialize(const VkDisplayPropertiesKHR* in_struct);
    void initialize(const safe_VkDisplayPropertiesKHR* copy_src);
    VkDisplayPropertiesKHR *ptr() { return reinterpret_cast<VkDisplayPropertiesKHR *>(this); }
    VkDisplayPropertiesKHR const *ptr() const { return reinterpret_cast<VkDisplayPropertiesKHR const *>(this); }
};

struct safe_VkDisplaySurfaceCreateInfoKHR {
    VkStructureType sType;
    const void* pNext;
    VkDisplaySurfaceCreateFlagsKHR flags;
    VkDisplayModeKHR displayMode;
    uint32_t planeIndex;
    uint32_t planeStackIndex;
    VkSurfaceTransformFlagBitsKHR transform;
    float globalAlpha;
    VkDisplayPlaneAlphaFlagBitsKHR alphaMode;
    VkExtent2D imageExtent;
    safe_VkDisplaySurfaceCreateInfoKHR(const VkDisplaySurfaceCreateInfoKHR* in_struct);
    safe_VkDisplaySurfaceCreateInfoKHR(const safe_VkDisplaySurfaceCreateInfoKHR& copy_src);
    safe_VkDisplaySurfaceCreateInfoKHR& operator=(const safe_VkDisplaySurfaceCreateInfoKHR& copy_src);
    safe_VkDisplaySurfaceCreateInfoKHR();
    ~safe_VkDisplaySurfaceCreateInfoKHR();
    void initialize(const VkDisplaySurfaceCreateInfoKHR* in_struct);
    void initialize(const safe_VkDisplaySurfaceCreateInfoKHR* copy_src);
    VkDisplaySurfaceCreateInfoKHR *ptr() { return reinterpret_cast<VkDisplaySurfaceCreateInfoKHR *>(this); }
    VkDisplaySurfaceCreateInfoKHR const *ptr() const { return reinterpret_cast<VkDisplaySurfaceCreateInfoKHR const *>(this); }
};

struct safe_VkDisplayPresentInfoKHR {
    VkStructureType sType;
    const void* pNext;
    VkRect2D srcRect;
    VkRect2D dstRect;
    VkBool32 persistent;
    safe_VkDisplayPresentInfoKHR(const VkDisplayPresentInfoKHR* in_struct);
    safe_VkDisplayPresentInfoKHR(const safe_VkDisplayPresentInfoKHR& copy_src);
    safe_VkDisplayPresentInfoKHR& operator=(const safe_VkDisplayPresentInfoKHR& copy_src);
    safe_VkDisplayPresentInfoKHR();
    ~safe_VkDisplayPresentInfoKHR();
    void initialize(const VkDisplayPresentInfoKHR* in_struct);
    void initialize(const safe_VkDisplayPresentInfoKHR* copy_src);
    VkDisplayPresentInfoKHR *ptr() { return reinterpret_cast<VkDisplayPresentInfoKHR *>(this); }
    VkDisplayPresentInfoKHR const *ptr() const { return reinterpret_cast<VkDisplayPresentInfoKHR const *>(this); }
};

struct safe_VkImportMemoryFdInfoKHR {
    VkStructureType sType;
    const void* pNext;
    VkExternalMemoryHandleTypeFlagBits handleType;
    int fd;
    safe_VkImportMemoryFdInfoKHR(const VkImportMemoryFdInfoKHR* in_struct);
    safe_VkImportMemoryFdInfoKHR(const safe_VkImportMemoryFdInfoKHR& copy_src);
    safe_VkImportMemoryFdInfoKHR& operator=(const safe_VkImportMemoryFdInfoKHR& copy_src);
    safe_VkImportMemoryFdInfoKHR();
    ~safe_VkImportMemoryFdInfoKHR();
    void initialize(const VkImportMemoryFdInfoKHR* in_struct);
    void initialize(const safe_VkImportMemoryFdInfoKHR* copy_src);
    VkImportMemoryFdInfoKHR *ptr() { return reinterpret_cast<VkImportMemoryFdInfoKHR *>(this); }
    VkImportMemoryFdInfoKHR const *ptr() const { return reinterpret_cast<VkImportMemoryFdInfoKHR const *>(this); }
};

struct safe_VkMemoryFdPropertiesKHR {
    VkStructureType sType;
    void* pNext;
    uint32_t memoryTypeBits;
    safe_VkMemoryFdPropertiesKHR(const VkMemoryFdPropertiesKHR* in_struct);
    safe_VkMemoryFdPropertiesKHR(const safe_VkMemoryFdPropertiesKHR& copy_src);
    safe_VkMemoryFdPropertiesKHR& operator=(const safe_VkMemoryFdPropertiesKHR& copy_src);
    safe_VkMemoryFdPropertiesKHR();
    ~safe_VkMemoryFdPropertiesKHR();
    void initialize(const VkMemoryFdPropertiesKHR* in_struct);
    void initialize(const safe_VkMemoryFdPropertiesKHR* copy_src);
    VkMemoryFdPropertiesKHR *ptr() { return reinterpret_cast<VkMemoryFdPropertiesKHR *>(this); }
    VkMemoryFdPropertiesKHR const *ptr() const { return reinterpret_cast<VkMemoryFdPropertiesKHR const *>(this); }
};

struct safe_VkMemoryGetFdInfoKHR {
    VkStructureType sType;
    const void* pNext;
    VkDeviceMemory memory;
    VkExternalMemoryHandleTypeFlagBits handleType;
    safe_VkMemoryGetFdInfoKHR(const VkMemoryGetFdInfoKHR* in_struct);
    safe_VkMemoryGetFdInfoKHR(const safe_VkMemoryGetFdInfoKHR& copy_src);
    safe_VkMemoryGetFdInfoKHR& operator=(const safe_VkMemoryGetFdInfoKHR& copy_src);
    safe_VkMemoryGetFdInfoKHR();
    ~safe_VkMemoryGetFdInfoKHR();
    void initialize(const VkMemoryGetFdInfoKHR* in_struct);
    void initialize(const safe_VkMemoryGetFdInfoKHR* copy_src);
    VkMemoryGetFdInfoKHR *ptr() { return reinterpret_cast<VkMemoryGetFdInfoKHR *>(this); }
    VkMemoryGetFdInfoKHR const *ptr() const { return reinterpret_cast<VkMemoryGetFdInfoKHR const *>(this); }
};

struct safe_VkImportSemaphoreFdInfoKHR {
    VkStructureType sType;
    const void* pNext;
    VkSemaphore semaphore;
    VkSemaphoreImportFlags flags;
    VkExternalSemaphoreHandleTypeFlagBits handleType;
    int fd;
    safe_VkImportSemaphoreFdInfoKHR(const VkImportSemaphoreFdInfoKHR* in_struct);
    safe_VkImportSemaphoreFdInfoKHR(const safe_VkImportSemaphoreFdInfoKHR& copy_src);
    safe_VkImportSemaphoreFdInfoKHR& operator=(const safe_VkImportSemaphoreFdInfoKHR& copy_src);
    safe_VkImportSemaphoreFdInfoKHR();
    ~safe_VkImportSemaphoreFdInfoKHR();
    void initialize(const VkImportSemaphoreFdInfoKHR* in_struct);
    void initialize(const safe_VkImportSemaphoreFdInfoKHR* copy_src);
    VkImportSemaphoreFdInfoKHR *ptr() { return reinterpret_cast<VkImportSemaphoreFdInfoKHR *>(this); }
    VkImportSemaphoreFdInfoKHR const *ptr() const { return reinterpret_cast<VkImportSemaphoreFdInfoKHR const *>(this); }
};

struct safe_VkSemaphoreGetFdInfoKHR {
    VkStructureType sType;
    const void* pNext;
    VkSemaphore semaphore;
    VkExternalSemaphoreHandleTypeFlagBits handleType;
    safe_VkSemaphoreGetFdInfoKHR(const VkSemaphoreGetFdInfoKHR* in_struct);
    safe_VkSemaphoreGetFdInfoKHR(const safe_VkSemaphoreGetFdInfoKHR& copy_src);
    safe_VkSemaphoreGetFdInfoKHR& operator=(const safe_VkSemaphoreGetFdInfoKHR& copy_src);
    safe_VkSemaphoreGetFdInfoKHR();
    ~safe_VkSemaphoreGetFdInfoKHR();
    void initialize(const VkSemaphoreGetFdInfoKHR* in_struct);
    void initialize(const safe_VkSemaphoreGetFdInfoKHR* copy_src);
    VkSemaphoreGetFdInfoKHR *ptr() { return reinterpret_cast<VkSemaphoreGetFdInfoKHR *>(this); }
    VkSemaphoreGetFdInfoKHR const *ptr() const { return reinterpret_cast<VkSemaphoreGetFdInfoKHR const *>(this); }
};

struct safe_VkPresentRegionKHR {
    uint32_t rectangleCount;
    const VkRectLayerKHR* pRectangles;
    safe_VkPresentRegionKHR(const VkPresentRegionKHR* in_struct);
    safe_VkPresentRegionKHR(const safe_VkPresentRegionKHR& copy_src);
    safe_VkPresentRegionKHR& operator=(const safe_VkPresentRegionKHR& copy_src);
    safe_VkPresentRegionKHR();
    ~safe_VkPresentRegionKHR();
    void initialize(const VkPresentRegionKHR* in_struct);
    void initialize(const safe_VkPresentRegionKHR* copy_src);
    VkPresentRegionKHR *ptr() { return reinterpret_cast<VkPresentRegionKHR *>(this); }
    VkPresentRegionKHR const *ptr() const { return reinterpret_cast<VkPresentRegionKHR const *>(this); }
};

struct safe_VkPresentRegionsKHR {
    VkStructureType sType;
    const void* pNext;
    uint32_t swapchainCount;
    safe_VkPresentRegionKHR* pRegions;
    safe_VkPresentRegionsKHR(const VkPresentRegionsKHR* in_struct);
    safe_VkPresentRegionsKHR(const safe_VkPresentRegionsKHR& copy_src);
    safe_VkPresentRegionsKHR& operator=(const safe_VkPresentRegionsKHR& copy_src);
    safe_VkPresentRegionsKHR();
    ~safe_VkPresentRegionsKHR();
    void initialize(const VkPresentRegionsKHR* in_struct);
    void initialize(const safe_VkPresentRegionsKHR* copy_src);
    VkPresentRegionsKHR *ptr() { return reinterpret_cast<VkPresentRegionsKHR *>(this); }
    VkPresentRegionsKHR const *ptr() const { return reinterpret_cast<VkPresentRegionsKHR const *>(this); }
};

struct safe_VkSharedPresentSurfaceCapabilitiesKHR {
    VkStructureType sType;
    void* pNext;
    VkImageUsageFlags sharedPresentSupportedUsageFlags;
    safe_VkSharedPresentSurfaceCapabilitiesKHR(const VkSharedPresentSurfaceCapabilitiesKHR* in_struct);
    safe_VkSharedPresentSurfaceCapabilitiesKHR(const safe_VkSharedPresentSurfaceCapabilitiesKHR& copy_src);
    safe_VkSharedPresentSurfaceCapabilitiesKHR& operator=(const safe_VkSharedPresentSurfaceCapabilitiesKHR& copy_src);
    safe_VkSharedPresentSurfaceCapabilitiesKHR();
    ~safe_VkSharedPresentSurfaceCapabilitiesKHR();
    void initialize(const VkSharedPresentSurfaceCapabilitiesKHR* in_struct);
    void initialize(const safe_VkSharedPresentSurfaceCapabilitiesKHR* copy_src);
    VkSharedPresentSurfaceCapabilitiesKHR *ptr() { return reinterpret_cast<VkSharedPresentSurfaceCapabilitiesKHR *>(this); }
    VkSharedPresentSurfaceCapabilitiesKHR const *ptr() const { return reinterpret_cast<VkSharedPresentSurfaceCapabilitiesKHR const *>(this); }
};

struct safe_VkImportFenceFdInfoKHR {
    VkStructureType sType;
    const void* pNext;
    VkFence fence;
    VkFenceImportFlags flags;
    VkExternalFenceHandleTypeFlagBits handleType;
    int fd;
    safe_VkImportFenceFdInfoKHR(const VkImportFenceFdInfoKHR* in_struct);
    safe_VkImportFenceFdInfoKHR(const safe_VkImportFenceFdInfoKHR& copy_src);
    safe_VkImportFenceFdInfoKHR& operator=(const safe_VkImportFenceFdInfoKHR& copy_src);
    safe_VkImportFenceFdInfoKHR();
    ~safe_VkImportFenceFdInfoKHR();
    void initialize(const VkImportFenceFdInfoKHR* in_struct);
    void initialize(const safe_VkImportFenceFdInfoKHR* copy_src);
    VkImportFenceFdInfoKHR *ptr() { return reinterpret_cast<VkImportFenceFdInfoKHR *>(this); }
    VkImportFenceFdInfoKHR const *ptr() const { return reinterpret_cast<VkImportFenceFdInfoKHR const *>(this); }
};

struct safe_VkFenceGetFdInfoKHR {
    VkStructureType sType;
    const void* pNext;
    VkFence fence;
    VkExternalFenceHandleTypeFlagBits handleType;
    safe_VkFenceGetFdInfoKHR(const VkFenceGetFdInfoKHR* in_struct);
    safe_VkFenceGetFdInfoKHR(const safe_VkFenceGetFdInfoKHR& copy_src);
    safe_VkFenceGetFdInfoKHR& operator=(const safe_VkFenceGetFdInfoKHR& copy_src);
    safe_VkFenceGetFdInfoKHR();
    ~safe_VkFenceGetFdInfoKHR();
    void initialize(const VkFenceGetFdInfoKHR* in_struct);
    void initialize(const safe_VkFenceGetFdInfoKHR* copy_src);
    VkFenceGetFdInfoKHR *ptr() { return reinterpret_cast<VkFenceGetFdInfoKHR *>(this); }
    VkFenceGetFdInfoKHR const *ptr() const { return reinterpret_cast<VkFenceGetFdInfoKHR const *>(this); }
};

struct safe_VkPhysicalDevicePerformanceQueryFeaturesKHR {
    VkStructureType sType;
    void* pNext;
    VkBool32 performanceCounterQueryPools;
    VkBool32 performanceCounterMultipleQueryPools;
    safe_VkPhysicalDevicePerformanceQueryFeaturesKHR(const VkPhysicalDevicePerformanceQueryFeaturesKHR* in_struct);
    safe_VkPhysicalDevicePerformanceQueryFeaturesKHR(const safe_VkPhysicalDevicePerformanceQueryFeaturesKHR& copy_src);
    safe_VkPhysicalDevicePerformanceQueryFeaturesKHR& operator=(const safe_VkPhysicalDevicePerformanceQueryFeaturesKHR& copy_src);
    safe_VkPhysicalDevicePerformanceQueryFeaturesKHR();
    ~safe_VkPhysicalDevicePerformanceQueryFeaturesKHR();
    void initialize(const VkPhysicalDevicePerformanceQueryFeaturesKHR* in_struct);
    void initialize(const safe_VkPhysicalDevicePerformanceQueryFeaturesKHR* copy_src);
    VkPhysicalDevicePerformanceQueryFeaturesKHR *ptr() { return reinterpret_cast<VkPhysicalDevicePerformanceQueryFeaturesKHR *>(this); }
    VkPhysicalDevicePerformanceQueryFeaturesKHR const *ptr() const { return reinterpret_cast<VkPhysicalDevicePerformanceQueryFeaturesKHR const *>(this); }
};

struct safe_VkPhysicalDevicePerformanceQueryPropertiesKHR {
    VkStructureType sType;
    void* pNext;
    VkBool32 allowCommandBufferQueryCopies;
    safe_VkPhysicalDevicePerformanceQueryPropertiesKHR(const VkPhysicalDevicePerformanceQueryPropertiesKHR* in_struct);
    safe_VkPhysicalDevicePerformanceQueryPropertiesKHR(const safe_VkPhysicalDevicePerformanceQueryPropertiesKHR& copy_src);
    safe_VkPhysicalDevicePerformanceQueryPropertiesKHR& operator=(const safe_VkPhysicalDevicePerformanceQueryPropertiesKHR& copy_src);
    safe_VkPhysicalDevicePerformanceQueryPropertiesKHR();
    ~safe_VkPhysicalDevicePerformanceQueryPropertiesKHR();
    void initialize(const VkPhysicalDevicePerformanceQueryPropertiesKHR* in_struct);
    void initialize(const safe_VkPhysicalDevicePerformanceQueryPropertiesKHR* copy_src);
    VkPhysicalDevicePerformanceQueryPropertiesKHR *ptr() { return reinterpret_cast<VkPhysicalDevicePerformanceQueryPropertiesKHR *>(this); }
    VkPhysicalDevicePerformanceQueryPropertiesKHR const *ptr() const { return reinterpret_cast<VkPhysicalDevicePerformanceQueryPropertiesKHR const *>(this); }
};

struct safe_VkPerformanceCounterKHR {
    VkStructureType sType;
    void* pNext;
    VkPerformanceCounterUnitKHR unit;
    VkPerformanceCounterScopeKHR scope;
    VkPerformanceCounterStorageKHR storage;
    uint8_t uuid[VK_UUID_SIZE];
    safe_VkPerformanceCounterKHR(const VkPerformanceCounterKHR* in_struct);
    safe_VkPerformanceCounterKHR(const safe_VkPerformanceCounterKHR& copy_src);
    safe_VkPerformanceCounterKHR& operator=(const safe_VkPerformanceCounterKHR& copy_src);
    safe_VkPerformanceCounterKHR();
    ~safe_VkPerformanceCounterKHR();
    void initialize(const VkPerformanceCounterKHR* in_struct);
    void initialize(const safe_VkPerformanceCounterKHR* copy_src);
    VkPerformanceCounterKHR *ptr() { return reinterpret_cast<VkPerformanceCounterKHR *>(this); }
    VkPerformanceCounterKHR const *ptr() const { return reinterpret_cast<VkPerformanceCounterKHR const *>(this); }
};

struct safe_VkPerformanceCounterDescriptionKHR {
    VkStructureType sType;
    void* pNext;
    VkPerformanceCounterDescriptionFlagsKHR flags;
    char name[VK_MAX_DESCRIPTION_SIZE];
    char category[VK_MAX_DESCRIPTION_SIZE];
    char description[VK_MAX_DESCRIPTION_SIZE];
    safe_VkPerformanceCounterDescriptionKHR(const VkPerformanceCounterDescriptionKHR* in_struct);
    safe_VkPerformanceCounterDescriptionKHR(const safe_VkPerformanceCounterDescriptionKHR& copy_src);
    safe_VkPerformanceCounterDescriptionKHR& operator=(const safe_VkPerformanceCounterDescriptionKHR& copy_src);
    safe_VkPerformanceCounterDescriptionKHR();
    ~safe_VkPerformanceCounterDescriptionKHR();
    void initialize(const VkPerformanceCounterDescriptionKHR* in_struct);
    void initialize(const safe_VkPerformanceCounterDescriptionKHR* copy_src);
    VkPerformanceCounterDescriptionKHR *ptr() { return reinterpret_cast<VkPerformanceCounterDescriptionKHR *>(this); }
    VkPerformanceCounterDescriptionKHR const *ptr() const { return reinterpret_cast<VkPerformanceCounterDescriptionKHR const *>(this); }
};

struct safe_VkQueryPoolPerformanceCreateInfoKHR {
    VkStructureType sType;
    const void* pNext;
    uint32_t queueFamilyIndex;
    uint32_t counterIndexCount;
    const uint32_t* pCounterIndices;
    safe_VkQueryPoolPerformanceCreateInfoKHR(const VkQueryPoolPerformanceCreateInfoKHR* in_struct);
    safe_VkQueryPoolPerformanceCreateInfoKHR(const safe_VkQueryPoolPerformanceCreateInfoKHR& copy_src);
    safe_VkQueryPoolPerformanceCreateInfoKHR& operator=(const safe_VkQueryPoolPerformanceCreateInfoKHR& copy_src);
    safe_VkQueryPoolPerformanceCreateInfoKHR();
    ~safe_VkQueryPoolPerformanceCreateInfoKHR();
    void initialize(const VkQueryPoolPerformanceCreateInfoKHR* in_struct);
    void initialize(const safe_VkQueryPoolPerformanceCreateInfoKHR* copy_src);
    VkQueryPoolPerformanceCreateInfoKHR *ptr() { return reinterpret_cast<VkQueryPoolPerformanceCreateInfoKHR *>(this); }
    VkQueryPoolPerformanceCreateInfoKHR const *ptr() const { return reinterpret_cast<VkQueryPoolPerformanceCreateInfoKHR const *>(this); }
};

struct safe_VkAcquireProfilingLockInfoKHR {
    VkStructureType sType;
    const void* pNext;
    VkAcquireProfilingLockFlagsKHR flags;
    uint64_t timeout;
    safe_VkAcquireProfilingLockInfoKHR(const VkAcquireProfilingLockInfoKHR* in_struct);
    safe_VkAcquireProfilingLockInfoKHR(const safe_VkAcquireProfilingLockInfoKHR& copy_src);
    safe_VkAcquireProfilingLockInfoKHR& operator=(const safe_VkAcquireProfilingLockInfoKHR& copy_src);
    safe_VkAcquireProfilingLockInfoKHR();
    ~safe_VkAcquireProfilingLockInfoKHR();
    void initialize(const VkAcquireProfilingLockInfoKHR* in_struct);
    void initialize(const safe_VkAcquireProfilingLockInfoKHR* copy_src);
    VkAcquireProfilingLockInfoKHR *ptr() { return reinterpret_cast<VkAcquireProfilingLockInfoKHR *>(this); }
    VkAcquireProfilingLockInfoKHR const *ptr() const { return reinterpret_cast<VkAcquireProfilingLockInfoKHR const *>(this); }
};

struct safe_VkPerformanceQuerySubmitInfoKHR {
    VkStructureType sType;
    const void* pNext;
    uint32_t counterPassIndex;
    safe_VkPerformanceQuerySubmitInfoKHR(const VkPerformanceQuerySubmitInfoKHR* in_struct);
    safe_VkPerformanceQuerySubmitInfoKHR(const safe_VkPerformanceQuerySubmitInfoKHR& copy_src);
    safe_VkPerformanceQuerySubmitInfoKHR& operator=(const safe_VkPerformanceQuerySubmitInfoKHR& copy_src);
    safe_VkPerformanceQuerySubmitInfoKHR();
    ~safe_VkPerformanceQuerySubmitInfoKHR();
    void initialize(const VkPerformanceQuerySubmitInfoKHR* in_struct);
    void initialize(const safe_VkPerformanceQuerySubmitInfoKHR* copy_src);
    VkPerformanceQuerySubmitInfoKHR *ptr() { return reinterpret_cast<VkPerformanceQuerySubmitInfoKHR *>(this); }
    VkPerformanceQuerySubmitInfoKHR const *ptr() const { return reinterpret_cast<VkPerformanceQuerySubmitInfoKHR const *>(this); }
};

struct safe_VkPerformanceQueryReservationInfoKHR {
    VkStructureType sType;
    const void* pNext;
    uint32_t maxPerformanceQueriesPerPool;
    safe_VkPerformanceQueryReservationInfoKHR(const VkPerformanceQueryReservationInfoKHR* in_struct);
    safe_VkPerformanceQueryReservationInfoKHR(const safe_VkPerformanceQueryReservationInfoKHR& copy_src);
    safe_VkPerformanceQueryReservationInfoKHR& operator=(const safe_VkPerformanceQueryReservationInfoKHR& copy_src);
    safe_VkPerformanceQueryReservationInfoKHR();
    ~safe_VkPerformanceQueryReservationInfoKHR();
    void initialize(const VkPerformanceQueryReservationInfoKHR* in_struct);
    void initialize(const safe_VkPerformanceQueryReservationInfoKHR* copy_src);
    VkPerformanceQueryReservationInfoKHR *ptr() { return reinterpret_cast<VkPerformanceQueryReservationInfoKHR *>(this); }
    VkPerformanceQueryReservationInfoKHR const *ptr() const { return reinterpret_cast<VkPerformanceQueryReservationInfoKHR const *>(this); }
};

struct safe_VkPhysicalDeviceSurfaceInfo2KHR {
    VkStructureType sType;
    const void* pNext;
    VkSurfaceKHR surface;
    safe_VkPhysicalDeviceSurfaceInfo2KHR(const VkPhysicalDeviceSurfaceInfo2KHR* in_struct);
    safe_VkPhysicalDeviceSurfaceInfo2KHR(const safe_VkPhysicalDeviceSurfaceInfo2KHR& copy_src);
    safe_VkPhysicalDeviceSurfaceInfo2KHR& operator=(const safe_VkPhysicalDeviceSurfaceInfo2KHR& copy_src);
    safe_VkPhysicalDeviceSurfaceInfo2KHR();
    ~safe_VkPhysicalDeviceSurfaceInfo2KHR();
    void initialize(const VkPhysicalDeviceSurfaceInfo2KHR* in_struct);
    void initialize(const safe_VkPhysicalDeviceSurfaceInfo2KHR* copy_src);
    VkPhysicalDeviceSurfaceInfo2KHR *ptr() { return reinterpret_cast<VkPhysicalDeviceSurfaceInfo2KHR *>(this); }
    VkPhysicalDeviceSurfaceInfo2KHR const *ptr() const { return reinterpret_cast<VkPhysicalDeviceSurfaceInfo2KHR const *>(this); }
};

struct safe_VkSurfaceCapabilities2KHR {
    VkStructureType sType;
    void* pNext;
    VkSurfaceCapabilitiesKHR surfaceCapabilities;
    safe_VkSurfaceCapabilities2KHR(const VkSurfaceCapabilities2KHR* in_struct);
    safe_VkSurfaceCapabilities2KHR(const safe_VkSurfaceCapabilities2KHR& copy_src);
    safe_VkSurfaceCapabilities2KHR& operator=(const safe_VkSurfaceCapabilities2KHR& copy_src);
    safe_VkSurfaceCapabilities2KHR();
    ~safe_VkSurfaceCapabilities2KHR();
    void initialize(const VkSurfaceCapabilities2KHR* in_struct);
    void initialize(const safe_VkSurfaceCapabilities2KHR* copy_src);
    VkSurfaceCapabilities2KHR *ptr() { return reinterpret_cast<VkSurfaceCapabilities2KHR *>(this); }
    VkSurfaceCapabilities2KHR const *ptr() const { return reinterpret_cast<VkSurfaceCapabilities2KHR const *>(this); }
};

struct safe_VkSurfaceFormat2KHR {
    VkStructureType sType;
    void* pNext;
    VkSurfaceFormatKHR surfaceFormat;
    safe_VkSurfaceFormat2KHR(const VkSurfaceFormat2KHR* in_struct);
    safe_VkSurfaceFormat2KHR(const safe_VkSurfaceFormat2KHR& copy_src);
    safe_VkSurfaceFormat2KHR& operator=(const safe_VkSurfaceFormat2KHR& copy_src);
    safe_VkSurfaceFormat2KHR();
    ~safe_VkSurfaceFormat2KHR();
    void initialize(const VkSurfaceFormat2KHR* in_struct);
    void initialize(const safe_VkSurfaceFormat2KHR* copy_src);
    VkSurfaceFormat2KHR *ptr() { return reinterpret_cast<VkSurfaceFormat2KHR *>(this); }
    VkSurfaceFormat2KHR const *ptr() const { return reinterpret_cast<VkSurfaceFormat2KHR const *>(this); }
};

struct safe_VkDisplayProperties2KHR {
    VkStructureType sType;
    void* pNext;
    safe_VkDisplayPropertiesKHR displayProperties;
    safe_VkDisplayProperties2KHR(const VkDisplayProperties2KHR* in_struct);
    safe_VkDisplayProperties2KHR(const safe_VkDisplayProperties2KHR& copy_src);
    safe_VkDisplayProperties2KHR& operator=(const safe_VkDisplayProperties2KHR& copy_src);
    safe_VkDisplayProperties2KHR();
    ~safe_VkDisplayProperties2KHR();
    void initialize(const VkDisplayProperties2KHR* in_struct);
    void initialize(const safe_VkDisplayProperties2KHR* copy_src);
    VkDisplayProperties2KHR *ptr() { return reinterpret_cast<VkDisplayProperties2KHR *>(this); }
    VkDisplayProperties2KHR const *ptr() const { return reinterpret_cast<VkDisplayProperties2KHR const *>(this); }
};

struct safe_VkDisplayPlaneProperties2KHR {
    VkStructureType sType;
    void* pNext;
    VkDisplayPlanePropertiesKHR displayPlaneProperties;
    safe_VkDisplayPlaneProperties2KHR(const VkDisplayPlaneProperties2KHR* in_struct);
    safe_VkDisplayPlaneProperties2KHR(const safe_VkDisplayPlaneProperties2KHR& copy_src);
    safe_VkDisplayPlaneProperties2KHR& operator=(const safe_VkDisplayPlaneProperties2KHR& copy_src);
    safe_VkDisplayPlaneProperties2KHR();
    ~safe_VkDisplayPlaneProperties2KHR();
    void initialize(const VkDisplayPlaneProperties2KHR* in_struct);
    void initialize(const safe_VkDisplayPlaneProperties2KHR* copy_src);
    VkDisplayPlaneProperties2KHR *ptr() { return reinterpret_cast<VkDisplayPlaneProperties2KHR *>(this); }
    VkDisplayPlaneProperties2KHR const *ptr() const { return reinterpret_cast<VkDisplayPlaneProperties2KHR const *>(this); }
};

struct safe_VkDisplayModeProperties2KHR {
    VkStructureType sType;
    void* pNext;
    VkDisplayModePropertiesKHR displayModeProperties;
    safe_VkDisplayModeProperties2KHR(const VkDisplayModeProperties2KHR* in_struct);
    safe_VkDisplayModeProperties2KHR(const safe_VkDisplayModeProperties2KHR& copy_src);
    safe_VkDisplayModeProperties2KHR& operator=(const safe_VkDisplayModeProperties2KHR& copy_src);
    safe_VkDisplayModeProperties2KHR();
    ~safe_VkDisplayModeProperties2KHR();
    void initialize(const VkDisplayModeProperties2KHR* in_struct);
    void initialize(const safe_VkDisplayModeProperties2KHR* copy_src);
    VkDisplayModeProperties2KHR *ptr() { return reinterpret_cast<VkDisplayModeProperties2KHR *>(this); }
    VkDisplayModeProperties2KHR const *ptr() const { return reinterpret_cast<VkDisplayModeProperties2KHR const *>(this); }
};

struct safe_VkDisplayPlaneInfo2KHR {
    VkStructureType sType;
    const void* pNext;
    VkDisplayModeKHR mode;
    uint32_t planeIndex;
    safe_VkDisplayPlaneInfo2KHR(const VkDisplayPlaneInfo2KHR* in_struct);
    safe_VkDisplayPlaneInfo2KHR(const safe_VkDisplayPlaneInfo2KHR& copy_src);
    safe_VkDisplayPlaneInfo2KHR& operator=(const safe_VkDisplayPlaneInfo2KHR& copy_src);
    safe_VkDisplayPlaneInfo2KHR();
    ~safe_VkDisplayPlaneInfo2KHR();
    void initialize(const VkDisplayPlaneInfo2KHR* in_struct);
    void initialize(const safe_VkDisplayPlaneInfo2KHR* copy_src);
    VkDisplayPlaneInfo2KHR *ptr() { return reinterpret_cast<VkDisplayPlaneInfo2KHR *>(this); }
    VkDisplayPlaneInfo2KHR const *ptr() const { return reinterpret_cast<VkDisplayPlaneInfo2KHR const *>(this); }
};

struct safe_VkDisplayPlaneCapabilities2KHR {
    VkStructureType sType;
    void* pNext;
    VkDisplayPlaneCapabilitiesKHR capabilities;
    safe_VkDisplayPlaneCapabilities2KHR(const VkDisplayPlaneCapabilities2KHR* in_struct);
    safe_VkDisplayPlaneCapabilities2KHR(const safe_VkDisplayPlaneCapabilities2KHR& copy_src);
    safe_VkDisplayPlaneCapabilities2KHR& operator=(const safe_VkDisplayPlaneCapabilities2KHR& copy_src);
    safe_VkDisplayPlaneCapabilities2KHR();
    ~safe_VkDisplayPlaneCapabilities2KHR();
    void initialize(const VkDisplayPlaneCapabilities2KHR* in_struct);
    void initialize(const safe_VkDisplayPlaneCapabilities2KHR* copy_src);
    VkDisplayPlaneCapabilities2KHR *ptr() { return reinterpret_cast<VkDisplayPlaneCapabilities2KHR *>(this); }
    VkDisplayPlaneCapabilities2KHR const *ptr() const { return reinterpret_cast<VkDisplayPlaneCapabilities2KHR const *>(this); }
};

struct safe_VkPhysicalDeviceShaderClockFeaturesKHR {
    VkStructureType sType;
    void* pNext;
    VkBool32 shaderSubgroupClock;
    VkBool32 shaderDeviceClock;
    safe_VkPhysicalDeviceShaderClockFeaturesKHR(const VkPhysicalDeviceShaderClockFeaturesKHR* in_struct);
    safe_VkPhysicalDeviceShaderClockFeaturesKHR(const safe_VkPhysicalDeviceShaderClockFeaturesKHR& copy_src);
    safe_VkPhysicalDeviceShaderClockFeaturesKHR& operator=(const safe_VkPhysicalDeviceShaderClockFeaturesKHR& copy_src);
    safe_VkPhysicalDeviceShaderClockFeaturesKHR();
    ~safe_VkPhysicalDeviceShaderClockFeaturesKHR();
    void initialize(const VkPhysicalDeviceShaderClockFeaturesKHR* in_struct);
    void initialize(const safe_VkPhysicalDeviceShaderClockFeaturesKHR* copy_src);
    VkPhysicalDeviceShaderClockFeaturesKHR *ptr() { return reinterpret_cast<VkPhysicalDeviceShaderClockFeaturesKHR *>(this); }
    VkPhysicalDeviceShaderClockFeaturesKHR const *ptr() const { return reinterpret_cast<VkPhysicalDeviceShaderClockFeaturesKHR const *>(this); }
};

struct safe_VkPhysicalDeviceShaderTerminateInvocationFeaturesKHR {
    VkStructureType sType;
    void* pNext;
    VkBool32 shaderTerminateInvocation;
    safe_VkPhysicalDeviceShaderTerminateInvocationFeaturesKHR(const VkPhysicalDeviceShaderTerminateInvocationFeaturesKHR* in_struct);
    safe_VkPhysicalDeviceShaderTerminateInvocationFeaturesKHR(const safe_VkPhysicalDeviceShaderTerminateInvocationFeaturesKHR& copy_src);
    safe_VkPhysicalDeviceShaderTerminateInvocationFeaturesKHR& operator=(const safe_VkPhysicalDeviceShaderTerminateInvocationFeaturesKHR& copy_src);
    safe_VkPhysicalDeviceShaderTerminateInvocationFeaturesKHR();
    ~safe_VkPhysicalDeviceShaderTerminateInvocationFeaturesKHR();
    void initialize(const VkPhysicalDeviceShaderTerminateInvocationFeaturesKHR* in_struct);
    void initialize(const safe_VkPhysicalDeviceShaderTerminateInvocationFeaturesKHR* copy_src);
    VkPhysicalDeviceShaderTerminateInvocationFeaturesKHR *ptr() { return reinterpret_cast<VkPhysicalDeviceShaderTerminateInvocationFeaturesKHR *>(this); }
    VkPhysicalDeviceShaderTerminateInvocationFeaturesKHR const *ptr() const { return reinterpret_cast<VkPhysicalDeviceShaderTerminateInvocationFeaturesKHR const *>(this); }
};

struct safe_VkFragmentShadingRateAttachmentInfoKHR {
    VkStructureType sType;
    const void* pNext;
    safe_VkAttachmentReference2* pFragmentShadingRateAttachment;
    VkExtent2D shadingRateAttachmentTexelSize;
    safe_VkFragmentShadingRateAttachmentInfoKHR(const VkFragmentShadingRateAttachmentInfoKHR* in_struct);
    safe_VkFragmentShadingRateAttachmentInfoKHR(const safe_VkFragmentShadingRateAttachmentInfoKHR& copy_src);
    safe_VkFragmentShadingRateAttachmentInfoKHR& operator=(const safe_VkFragmentShadingRateAttachmentInfoKHR& copy_src);
    safe_VkFragmentShadingRateAttachmentInfoKHR();
    ~safe_VkFragmentShadingRateAttachmentInfoKHR();
    void initialize(const VkFragmentShadingRateAttachmentInfoKHR* in_struct);
    void initialize(const safe_VkFragmentShadingRateAttachmentInfoKHR* copy_src);
    VkFragmentShadingRateAttachmentInfoKHR *ptr() { return reinterpret_cast<VkFragmentShadingRateAttachmentInfoKHR *>(this); }
    VkFragmentShadingRateAttachmentInfoKHR const *ptr() const { return reinterpret_cast<VkFragmentShadingRateAttachmentInfoKHR const *>(this); }
};

struct safe_VkPipelineFragmentShadingRateStateCreateInfoKHR {
    VkStructureType sType;
    const void* pNext;
    VkExtent2D fragmentSize;
    VkFragmentShadingRateCombinerOpKHR combinerOps[2];
    safe_VkPipelineFragmentShadingRateStateCreateInfoKHR(const VkPipelineFragmentShadingRateStateCreateInfoKHR* in_struct);
    safe_VkPipelineFragmentShadingRateStateCreateInfoKHR(const safe_VkPipelineFragmentShadingRateStateCreateInfoKHR& copy_src);
    safe_VkPipelineFragmentShadingRateStateCreateInfoKHR& operator=(const safe_VkPipelineFragmentShadingRateStateCreateInfoKHR& copy_src);
    safe_VkPipelineFragmentShadingRateStateCreateInfoKHR();
    ~safe_VkPipelineFragmentShadingRateStateCreateInfoKHR();
    void initialize(const VkPipelineFragmentShadingRateStateCreateInfoKHR* in_struct);
    void initialize(const safe_VkPipelineFragmentShadingRateStateCreateInfoKHR* copy_src);
    VkPipelineFragmentShadingRateStateCreateInfoKHR *ptr() { return reinterpret_cast<VkPipelineFragmentShadingRateStateCreateInfoKHR *>(this); }
    VkPipelineFragmentShadingRateStateCreateInfoKHR const *ptr() const { return reinterpret_cast<VkPipelineFragmentShadingRateStateCreateInfoKHR const *>(this); }
};

struct safe_VkPhysicalDeviceFragmentShadingRateFeaturesKHR {
    VkStructureType sType;
    void* pNext;
    VkBool32 pipelineFragmentShadingRate;
    VkBool32 primitiveFragmentShadingRate;
    VkBool32 attachmentFragmentShadingRate;
    safe_VkPhysicalDeviceFragmentShadingRateFeaturesKHR(const VkPhysicalDeviceFragmentShadingRateFeaturesKHR* in_struct);
    safe_VkPhysicalDeviceFragmentShadingRateFeaturesKHR(const safe_VkPhysicalDeviceFragmentShadingRateFeaturesKHR& copy_src);
    safe_VkPhysicalDeviceFragmentShadingRateFeaturesKHR& operator=(const safe_VkPhysicalDeviceFragmentShadingRateFeaturesKHR& copy_src);
    safe_VkPhysicalDeviceFragmentShadingRateFeaturesKHR();
    ~safe_VkPhysicalDeviceFragmentShadingRateFeaturesKHR();
    void initialize(const VkPhysicalDeviceFragmentShadingRateFeaturesKHR* in_struct);
    void initialize(const safe_VkPhysicalDeviceFragmentShadingRateFeaturesKHR* copy_src);
    VkPhysicalDeviceFragmentShadingRateFeaturesKHR *ptr() { return reinterpret_cast<VkPhysicalDeviceFragmentShadingRateFeaturesKHR *>(this); }
    VkPhysicalDeviceFragmentShadingRateFeaturesKHR const *ptr() const { return reinterpret_cast<VkPhysicalDeviceFragmentShadingRateFeaturesKHR const *>(this); }
};

struct safe_VkPhysicalDeviceFragmentShadingRatePropertiesKHR {
    VkStructureType sType;
    void* pNext;
    VkExtent2D minFragmentShadingRateAttachmentTexelSize;
    VkExtent2D maxFragmentShadingRateAttachmentTexelSize;
    uint32_t maxFragmentShadingRateAttachmentTexelSizeAspectRatio;
    VkBool32 primitiveFragmentShadingRateWithMultipleViewports;
    VkBool32 layeredShadingRateAttachments;
    VkBool32 fragmentShadingRateNonTrivialCombinerOps;
    VkExtent2D maxFragmentSize;
    uint32_t maxFragmentSizeAspectRatio;
    uint32_t maxFragmentShadingRateCoverageSamples;
    VkSampleCountFlagBits maxFragmentShadingRateRasterizationSamples;
    VkBool32 fragmentShadingRateWithShaderDepthStencilWrites;
    VkBool32 fragmentShadingRateWithSampleMask;
    VkBool32 fragmentShadingRateWithShaderSampleMask;
    VkBool32 fragmentShadingRateWithConservativeRasterization;
    VkBool32 fragmentShadingRateWithFragmentShaderInterlock;
    VkBool32 fragmentShadingRateWithCustomSampleLocations;
    VkBool32 fragmentShadingRateStrictMultiplyCombiner;
    safe_VkPhysicalDeviceFragmentShadingRatePropertiesKHR(const VkPhysicalDeviceFragmentShadingRatePropertiesKHR* in_struct);
    safe_VkPhysicalDeviceFragmentShadingRatePropertiesKHR(const safe_VkPhysicalDeviceFragmentShadingRatePropertiesKHR& copy_src);
    safe_VkPhysicalDeviceFragmentShadingRatePropertiesKHR& operator=(const safe_VkPhysicalDeviceFragmentShadingRatePropertiesKHR& copy_src);
    safe_VkPhysicalDeviceFragmentShadingRatePropertiesKHR();
    ~safe_VkPhysicalDeviceFragmentShadingRatePropertiesKHR();
    void initialize(const VkPhysicalDeviceFragmentShadingRatePropertiesKHR* in_struct);
    void initialize(const safe_VkPhysicalDeviceFragmentShadingRatePropertiesKHR* copy_src);
    VkPhysicalDeviceFragmentShadingRatePropertiesKHR *ptr() { return reinterpret_cast<VkPhysicalDeviceFragmentShadingRatePropertiesKHR *>(this); }
    VkPhysicalDeviceFragmentShadingRatePropertiesKHR const *ptr() const { return reinterpret_cast<VkPhysicalDeviceFragmentShadingRatePropertiesKHR const *>(this); }
};

struct safe_VkPhysicalDeviceFragmentShadingRateKHR {
    VkStructureType sType;
    void* pNext;
    VkSampleCountFlags sampleCounts;
    VkExtent2D fragmentSize;
    safe_VkPhysicalDeviceFragmentShadingRateKHR(const VkPhysicalDeviceFragmentShadingRateKHR* in_struct);
    safe_VkPhysicalDeviceFragmentShadingRateKHR(const safe_VkPhysicalDeviceFragmentShadingRateKHR& copy_src);
    safe_VkPhysicalDeviceFragmentShadingRateKHR& operator=(const safe_VkPhysicalDeviceFragmentShadingRateKHR& copy_src);
    safe_VkPhysicalDeviceFragmentShadingRateKHR();
    ~safe_VkPhysicalDeviceFragmentShadingRateKHR();
    void initialize(const VkPhysicalDeviceFragmentShadingRateKHR* in_struct);
    void initialize(const safe_VkPhysicalDeviceFragmentShadingRateKHR* copy_src);
    VkPhysicalDeviceFragmentShadingRateKHR *ptr() { return reinterpret_cast<VkPhysicalDeviceFragmentShadingRateKHR *>(this); }
    VkPhysicalDeviceFragmentShadingRateKHR const *ptr() const { return reinterpret_cast<VkPhysicalDeviceFragmentShadingRateKHR const *>(this); }
};

struct safe_VkRefreshObjectListKHR {
    VkStructureType sType;
    const void* pNext;
    uint32_t objectCount;
    const VkRefreshObjectKHR* pObjects;
    safe_VkRefreshObjectListKHR(const VkRefreshObjectListKHR* in_struct);
    safe_VkRefreshObjectListKHR(const safe_VkRefreshObjectListKHR& copy_src);
    safe_VkRefreshObjectListKHR& operator=(const safe_VkRefreshObjectListKHR& copy_src);
    safe_VkRefreshObjectListKHR();
    ~safe_VkRefreshObjectListKHR();
    void initialize(const VkRefreshObjectListKHR* in_struct);
    void initialize(const safe_VkRefreshObjectListKHR* copy_src);
    VkRefreshObjectListKHR *ptr() { return reinterpret_cast<VkRefreshObjectListKHR *>(this); }
    VkRefreshObjectListKHR const *ptr() const { return reinterpret_cast<VkRefreshObjectListKHR const *>(this); }
};

struct safe_VkMemoryBarrier2KHR {
    VkStructureType sType;
    const void* pNext;
    VkPipelineStageFlags2KHR srcStageMask;
    VkAccessFlags2KHR srcAccessMask;
    VkPipelineStageFlags2KHR dstStageMask;
    VkAccessFlags2KHR dstAccessMask;
    safe_VkMemoryBarrier2KHR(const VkMemoryBarrier2KHR* in_struct);
    safe_VkMemoryBarrier2KHR(const safe_VkMemoryBarrier2KHR& copy_src);
    safe_VkMemoryBarrier2KHR& operator=(const safe_VkMemoryBarrier2KHR& copy_src);
    safe_VkMemoryBarrier2KHR();
    ~safe_VkMemoryBarrier2KHR();
    void initialize(const VkMemoryBarrier2KHR* in_struct);
    void initialize(const safe_VkMemoryBarrier2KHR* copy_src);
    VkMemoryBarrier2KHR *ptr() { return reinterpret_cast<VkMemoryBarrier2KHR *>(this); }
    VkMemoryBarrier2KHR const *ptr() const { return reinterpret_cast<VkMemoryBarrier2KHR const *>(this); }
};

struct safe_VkBufferMemoryBarrier2KHR {
    VkStructureType sType;
    const void* pNext;
    VkPipelineStageFlags2KHR srcStageMask;
    VkAccessFlags2KHR srcAccessMask;
    VkPipelineStageFlags2KHR dstStageMask;
    VkAccessFlags2KHR dstAccessMask;
    uint32_t srcQueueFamilyIndex;
    uint32_t dstQueueFamilyIndex;
    VkBuffer buffer;
    VkDeviceSize offset;
    VkDeviceSize size;
    safe_VkBufferMemoryBarrier2KHR(const VkBufferMemoryBarrier2KHR* in_struct);
    safe_VkBufferMemoryBarrier2KHR(const safe_VkBufferMemoryBarrier2KHR& copy_src);
    safe_VkBufferMemoryBarrier2KHR& operator=(const safe_VkBufferMemoryBarrier2KHR& copy_src);
    safe_VkBufferMemoryBarrier2KHR();
    ~safe_VkBufferMemoryBarrier2KHR();
    void initialize(const VkBufferMemoryBarrier2KHR* in_struct);
    void initialize(const safe_VkBufferMemoryBarrier2KHR* copy_src);
    VkBufferMemoryBarrier2KHR *ptr() { return reinterpret_cast<VkBufferMemoryBarrier2KHR *>(this); }
    VkBufferMemoryBarrier2KHR const *ptr() const { return reinterpret_cast<VkBufferMemoryBarrier2KHR const *>(this); }
};

struct safe_VkImageMemoryBarrier2KHR {
    VkStructureType sType;
    const void* pNext;
    VkPipelineStageFlags2KHR srcStageMask;
    VkAccessFlags2KHR srcAccessMask;
    VkPipelineStageFlags2KHR dstStageMask;
    VkAccessFlags2KHR dstAccessMask;
    VkImageLayout oldLayout;
    VkImageLayout newLayout;
    uint32_t srcQueueFamilyIndex;
    uint32_t dstQueueFamilyIndex;
    VkImage image;
    VkImageSubresourceRange subresourceRange;
    safe_VkImageMemoryBarrier2KHR(const VkImageMemoryBarrier2KHR* in_struct);
    safe_VkImageMemoryBarrier2KHR(const safe_VkImageMemoryBarrier2KHR& copy_src);
    safe_VkImageMemoryBarrier2KHR& operator=(const safe_VkImageMemoryBarrier2KHR& copy_src);
    safe_VkImageMemoryBarrier2KHR();
    ~safe_VkImageMemoryBarrier2KHR();
    void initialize(const VkImageMemoryBarrier2KHR* in_struct);
    void initialize(const safe_VkImageMemoryBarrier2KHR* copy_src);
    VkImageMemoryBarrier2KHR *ptr() { return reinterpret_cast<VkImageMemoryBarrier2KHR *>(this); }
    VkImageMemoryBarrier2KHR const *ptr() const { return reinterpret_cast<VkImageMemoryBarrier2KHR const *>(this); }
};

struct safe_VkDependencyInfoKHR {
    VkStructureType sType;
    const void* pNext;
    VkDependencyFlags dependencyFlags;
    uint32_t memoryBarrierCount;
    safe_VkMemoryBarrier2KHR* pMemoryBarriers;
    uint32_t bufferMemoryBarrierCount;
    safe_VkBufferMemoryBarrier2KHR* pBufferMemoryBarriers;
    uint32_t imageMemoryBarrierCount;
    safe_VkImageMemoryBarrier2KHR* pImageMemoryBarriers;
    safe_VkDependencyInfoKHR(const VkDependencyInfoKHR* in_struct);
    safe_VkDependencyInfoKHR(const safe_VkDependencyInfoKHR& copy_src);
    safe_VkDependencyInfoKHR& operator=(const safe_VkDependencyInfoKHR& copy_src);
    safe_VkDependencyInfoKHR();
    ~safe_VkDependencyInfoKHR();
    void initialize(const VkDependencyInfoKHR* in_struct);
    void initialize(const safe_VkDependencyInfoKHR* copy_src);
    VkDependencyInfoKHR *ptr() { return reinterpret_cast<VkDependencyInfoKHR *>(this); }
    VkDependencyInfoKHR const *ptr() const { return reinterpret_cast<VkDependencyInfoKHR const *>(this); }
};

struct safe_VkSemaphoreSubmitInfoKHR {
    VkStructureType sType;
    const void* pNext;
    VkSemaphore semaphore;
    uint64_t value;
    VkPipelineStageFlags2KHR stageMask;
    uint32_t deviceIndex;
    safe_VkSemaphoreSubmitInfoKHR(const VkSemaphoreSubmitInfoKHR* in_struct);
    safe_VkSemaphoreSubmitInfoKHR(const safe_VkSemaphoreSubmitInfoKHR& copy_src);
    safe_VkSemaphoreSubmitInfoKHR& operator=(const safe_VkSemaphoreSubmitInfoKHR& copy_src);
    safe_VkSemaphoreSubmitInfoKHR();
    ~safe_VkSemaphoreSubmitInfoKHR();
    void initialize(const VkSemaphoreSubmitInfoKHR* in_struct);
    void initialize(const safe_VkSemaphoreSubmitInfoKHR* copy_src);
    VkSemaphoreSubmitInfoKHR *ptr() { return reinterpret_cast<VkSemaphoreSubmitInfoKHR *>(this); }
    VkSemaphoreSubmitInfoKHR const *ptr() const { return reinterpret_cast<VkSemaphoreSubmitInfoKHR const *>(this); }
};

struct safe_VkCommandBufferSubmitInfoKHR {
    VkStructureType sType;
    const void* pNext;
    VkCommandBuffer commandBuffer;
    uint32_t deviceMask;
    safe_VkCommandBufferSubmitInfoKHR(const VkCommandBufferSubmitInfoKHR* in_struct);
    safe_VkCommandBufferSubmitInfoKHR(const safe_VkCommandBufferSubmitInfoKHR& copy_src);
    safe_VkCommandBufferSubmitInfoKHR& operator=(const safe_VkCommandBufferSubmitInfoKHR& copy_src);
    safe_VkCommandBufferSubmitInfoKHR();
    ~safe_VkCommandBufferSubmitInfoKHR();
    void initialize(const VkCommandBufferSubmitInfoKHR* in_struct);
    void initialize(const safe_VkCommandBufferSubmitInfoKHR* copy_src);
    VkCommandBufferSubmitInfoKHR *ptr() { return reinterpret_cast<VkCommandBufferSubmitInfoKHR *>(this); }
    VkCommandBufferSubmitInfoKHR const *ptr() const { return reinterpret_cast<VkCommandBufferSubmitInfoKHR const *>(this); }
};

struct safe_VkSubmitInfo2KHR {
    VkStructureType sType;
    const void* pNext;
    VkSubmitFlagsKHR flags;
    uint32_t waitSemaphoreInfoCount;
    safe_VkSemaphoreSubmitInfoKHR* pWaitSemaphoreInfos;
    uint32_t commandBufferInfoCount;
    safe_VkCommandBufferSubmitInfoKHR* pCommandBufferInfos;
    uint32_t signalSemaphoreInfoCount;
    safe_VkSemaphoreSubmitInfoKHR* pSignalSemaphoreInfos;
    safe_VkSubmitInfo2KHR(const VkSubmitInfo2KHR* in_struct);
    safe_VkSubmitInfo2KHR(const safe_VkSubmitInfo2KHR& copy_src);
    safe_VkSubmitInfo2KHR& operator=(const safe_VkSubmitInfo2KHR& copy_src);
    safe_VkSubmitInfo2KHR();
    ~safe_VkSubmitInfo2KHR();
    void initialize(const VkSubmitInfo2KHR* in_struct);
    void initialize(const safe_VkSubmitInfo2KHR* copy_src);
    VkSubmitInfo2KHR *ptr() { return reinterpret_cast<VkSubmitInfo2KHR *>(this); }
    VkSubmitInfo2KHR const *ptr() const { return reinterpret_cast<VkSubmitInfo2KHR const *>(this); }
};

struct safe_VkPhysicalDeviceSynchronization2FeaturesKHR {
    VkStructureType sType;
    void* pNext;
    VkBool32 synchronization2;
    safe_VkPhysicalDeviceSynchronization2FeaturesKHR(const VkPhysicalDeviceSynchronization2FeaturesKHR* in_struct);
    safe_VkPhysicalDeviceSynchronization2FeaturesKHR(const safe_VkPhysicalDeviceSynchronization2FeaturesKHR& copy_src);
    safe_VkPhysicalDeviceSynchronization2FeaturesKHR& operator=(const safe_VkPhysicalDeviceSynchronization2FeaturesKHR& copy_src);
    safe_VkPhysicalDeviceSynchronization2FeaturesKHR();
    ~safe_VkPhysicalDeviceSynchronization2FeaturesKHR();
    void initialize(const VkPhysicalDeviceSynchronization2FeaturesKHR* in_struct);
    void initialize(const safe_VkPhysicalDeviceSynchronization2FeaturesKHR* copy_src);
    VkPhysicalDeviceSynchronization2FeaturesKHR *ptr() { return reinterpret_cast<VkPhysicalDeviceSynchronization2FeaturesKHR *>(this); }
    VkPhysicalDeviceSynchronization2FeaturesKHR const *ptr() const { return reinterpret_cast<VkPhysicalDeviceSynchronization2FeaturesKHR const *>(this); }
};

struct safe_VkQueueFamilyCheckpointProperties2NV {
    VkStructureType sType;
    void* pNext;
    VkPipelineStageFlags2KHR checkpointExecutionStageMask;
    safe_VkQueueFamilyCheckpointProperties2NV(const VkQueueFamilyCheckpointProperties2NV* in_struct);
    safe_VkQueueFamilyCheckpointProperties2NV(const safe_VkQueueFamilyCheckpointProperties2NV& copy_src);
    safe_VkQueueFamilyCheckpointProperties2NV& operator=(const safe_VkQueueFamilyCheckpointProperties2NV& copy_src);
    safe_VkQueueFamilyCheckpointProperties2NV();
    ~safe_VkQueueFamilyCheckpointProperties2NV();
    void initialize(const VkQueueFamilyCheckpointProperties2NV* in_struct);
    void initialize(const safe_VkQueueFamilyCheckpointProperties2NV* copy_src);
    VkQueueFamilyCheckpointProperties2NV *ptr() { return reinterpret_cast<VkQueueFamilyCheckpointProperties2NV *>(this); }
    VkQueueFamilyCheckpointProperties2NV const *ptr() const { return reinterpret_cast<VkQueueFamilyCheckpointProperties2NV const *>(this); }
};

struct safe_VkCheckpointData2NV {
    VkStructureType sType;
    void* pNext;
    VkPipelineStageFlags2KHR stage;
    void* pCheckpointMarker;
    safe_VkCheckpointData2NV(const VkCheckpointData2NV* in_struct);
    safe_VkCheckpointData2NV(const safe_VkCheckpointData2NV& copy_src);
    safe_VkCheckpointData2NV& operator=(const safe_VkCheckpointData2NV& copy_src);
    safe_VkCheckpointData2NV();
    ~safe_VkCheckpointData2NV();
    void initialize(const VkCheckpointData2NV* in_struct);
    void initialize(const safe_VkCheckpointData2NV* copy_src);
    VkCheckpointData2NV *ptr() { return reinterpret_cast<VkCheckpointData2NV *>(this); }
    VkCheckpointData2NV const *ptr() const { return reinterpret_cast<VkCheckpointData2NV const *>(this); }
};

struct safe_VkBufferCopy2KHR {
    VkStructureType sType;
    const void* pNext;
    VkDeviceSize srcOffset;
    VkDeviceSize dstOffset;
    VkDeviceSize size;
    safe_VkBufferCopy2KHR(const VkBufferCopy2KHR* in_struct);
    safe_VkBufferCopy2KHR(const safe_VkBufferCopy2KHR& copy_src);
    safe_VkBufferCopy2KHR& operator=(const safe_VkBufferCopy2KHR& copy_src);
    safe_VkBufferCopy2KHR();
    ~safe_VkBufferCopy2KHR();
    void initialize(const VkBufferCopy2KHR* in_struct);
    void initialize(const safe_VkBufferCopy2KHR* copy_src);
    VkBufferCopy2KHR *ptr() { return reinterpret_cast<VkBufferCopy2KHR *>(this); }
    VkBufferCopy2KHR const *ptr() const { return reinterpret_cast<VkBufferCopy2KHR const *>(this); }
};

struct safe_VkCopyBufferInfo2KHR {
    VkStructureType sType;
    const void* pNext;
    VkBuffer srcBuffer;
    VkBuffer dstBuffer;
    uint32_t regionCount;
    safe_VkBufferCopy2KHR* pRegions;
    safe_VkCopyBufferInfo2KHR(const VkCopyBufferInfo2KHR* in_struct);
    safe_VkCopyBufferInfo2KHR(const safe_VkCopyBufferInfo2KHR& copy_src);
    safe_VkCopyBufferInfo2KHR& operator=(const safe_VkCopyBufferInfo2KHR& copy_src);
    safe_VkCopyBufferInfo2KHR();
    ~safe_VkCopyBufferInfo2KHR();
    void initialize(const VkCopyBufferInfo2KHR* in_struct);
    void initialize(const safe_VkCopyBufferInfo2KHR* copy_src);
    VkCopyBufferInfo2KHR *ptr() { return reinterpret_cast<VkCopyBufferInfo2KHR *>(this); }
    VkCopyBufferInfo2KHR const *ptr() const { return reinterpret_cast<VkCopyBufferInfo2KHR const *>(this); }
};

struct safe_VkImageCopy2KHR {
    VkStructureType sType;
    const void* pNext;
    VkImageSubresourceLayers srcSubresource;
    VkOffset3D srcOffset;
    VkImageSubresourceLayers dstSubresource;
    VkOffset3D dstOffset;
    VkExtent3D extent;
    safe_VkImageCopy2KHR(const VkImageCopy2KHR* in_struct);
    safe_VkImageCopy2KHR(const safe_VkImageCopy2KHR& copy_src);
    safe_VkImageCopy2KHR& operator=(const safe_VkImageCopy2KHR& copy_src);
    safe_VkImageCopy2KHR();
    ~safe_VkImageCopy2KHR();
    void initialize(const VkImageCopy2KHR* in_struct);
    void initialize(const safe_VkImageCopy2KHR* copy_src);
    VkImageCopy2KHR *ptr() { return reinterpret_cast<VkImageCopy2KHR *>(this); }
    VkImageCopy2KHR const *ptr() const { return reinterpret_cast<VkImageCopy2KHR const *>(this); }
};

struct safe_VkCopyImageInfo2KHR {
    VkStructureType sType;
    const void* pNext;
    VkImage srcImage;
    VkImageLayout srcImageLayout;
    VkImage dstImage;
    VkImageLayout dstImageLayout;
    uint32_t regionCount;
    safe_VkImageCopy2KHR* pRegions;
    safe_VkCopyImageInfo2KHR(const VkCopyImageInfo2KHR* in_struct);
    safe_VkCopyImageInfo2KHR(const safe_VkCopyImageInfo2KHR& copy_src);
    safe_VkCopyImageInfo2KHR& operator=(const safe_VkCopyImageInfo2KHR& copy_src);
    safe_VkCopyImageInfo2KHR();
    ~safe_VkCopyImageInfo2KHR();
    void initialize(const VkCopyImageInfo2KHR* in_struct);
    void initialize(const safe_VkCopyImageInfo2KHR* copy_src);
    VkCopyImageInfo2KHR *ptr() { return reinterpret_cast<VkCopyImageInfo2KHR *>(this); }
    VkCopyImageInfo2KHR const *ptr() const { return reinterpret_cast<VkCopyImageInfo2KHR const *>(this); }
};

struct safe_VkBufferImageCopy2KHR {
    VkStructureType sType;
    const void* pNext;
    VkDeviceSize bufferOffset;
    uint32_t bufferRowLength;
    uint32_t bufferImageHeight;
    VkImageSubresourceLayers imageSubresource;
    VkOffset3D imageOffset;
    VkExtent3D imageExtent;
    safe_VkBufferImageCopy2KHR(const VkBufferImageCopy2KHR* in_struct);
    safe_VkBufferImageCopy2KHR(const safe_VkBufferImageCopy2KHR& copy_src);
    safe_VkBufferImageCopy2KHR& operator=(const safe_VkBufferImageCopy2KHR& copy_src);
    safe_VkBufferImageCopy2KHR();
    ~safe_VkBufferImageCopy2KHR();
    void initialize(const VkBufferImageCopy2KHR* in_struct);
    void initialize(const safe_VkBufferImageCopy2KHR* copy_src);
    VkBufferImageCopy2KHR *ptr() { return reinterpret_cast<VkBufferImageCopy2KHR *>(this); }
    VkBufferImageCopy2KHR const *ptr() const { return reinterpret_cast<VkBufferImageCopy2KHR const *>(this); }
};

struct safe_VkCopyBufferToImageInfo2KHR {
    VkStructureType sType;
    const void* pNext;
    VkBuffer srcBuffer;
    VkImage dstImage;
    VkImageLayout dstImageLayout;
    uint32_t regionCount;
    safe_VkBufferImageCopy2KHR* pRegions;
    safe_VkCopyBufferToImageInfo2KHR(const VkCopyBufferToImageInfo2KHR* in_struct);
    safe_VkCopyBufferToImageInfo2KHR(const safe_VkCopyBufferToImageInfo2KHR& copy_src);
    safe_VkCopyBufferToImageInfo2KHR& operator=(const safe_VkCopyBufferToImageInfo2KHR& copy_src);
    safe_VkCopyBufferToImageInfo2KHR();
    ~safe_VkCopyBufferToImageInfo2KHR();
    void initialize(const VkCopyBufferToImageInfo2KHR* in_struct);
    void initialize(const safe_VkCopyBufferToImageInfo2KHR* copy_src);
    VkCopyBufferToImageInfo2KHR *ptr() { return reinterpret_cast<VkCopyBufferToImageInfo2KHR *>(this); }
    VkCopyBufferToImageInfo2KHR const *ptr() const { return reinterpret_cast<VkCopyBufferToImageInfo2KHR const *>(this); }
};

struct safe_VkCopyImageToBufferInfo2KHR {
    VkStructureType sType;
    const void* pNext;
    VkImage srcImage;
    VkImageLayout srcImageLayout;
    VkBuffer dstBuffer;
    uint32_t regionCount;
    safe_VkBufferImageCopy2KHR* pRegions;
    safe_VkCopyImageToBufferInfo2KHR(const VkCopyImageToBufferInfo2KHR* in_struct);
    safe_VkCopyImageToBufferInfo2KHR(const safe_VkCopyImageToBufferInfo2KHR& copy_src);
    safe_VkCopyImageToBufferInfo2KHR& operator=(const safe_VkCopyImageToBufferInfo2KHR& copy_src);
    safe_VkCopyImageToBufferInfo2KHR();
    ~safe_VkCopyImageToBufferInfo2KHR();
    void initialize(const VkCopyImageToBufferInfo2KHR* in_struct);
    void initialize(const safe_VkCopyImageToBufferInfo2KHR* copy_src);
    VkCopyImageToBufferInfo2KHR *ptr() { return reinterpret_cast<VkCopyImageToBufferInfo2KHR *>(this); }
    VkCopyImageToBufferInfo2KHR const *ptr() const { return reinterpret_cast<VkCopyImageToBufferInfo2KHR const *>(this); }
};

struct safe_VkImageBlit2KHR {
    VkStructureType sType;
    const void* pNext;
    VkImageSubresourceLayers srcSubresource;
    VkOffset3D srcOffsets[2];
    VkImageSubresourceLayers dstSubresource;
    VkOffset3D dstOffsets[2];
    safe_VkImageBlit2KHR(const VkImageBlit2KHR* in_struct);
    safe_VkImageBlit2KHR(const safe_VkImageBlit2KHR& copy_src);
    safe_VkImageBlit2KHR& operator=(const safe_VkImageBlit2KHR& copy_src);
    safe_VkImageBlit2KHR();
    ~safe_VkImageBlit2KHR();
    void initialize(const VkImageBlit2KHR* in_struct);
    void initialize(const safe_VkImageBlit2KHR* copy_src);
    VkImageBlit2KHR *ptr() { return reinterpret_cast<VkImageBlit2KHR *>(this); }
    VkImageBlit2KHR const *ptr() const { return reinterpret_cast<VkImageBlit2KHR const *>(this); }
};

struct safe_VkBlitImageInfo2KHR {
    VkStructureType sType;
    const void* pNext;
    VkImage srcImage;
    VkImageLayout srcImageLayout;
    VkImage dstImage;
    VkImageLayout dstImageLayout;
    uint32_t regionCount;
    safe_VkImageBlit2KHR* pRegions;
    VkFilter filter;
    safe_VkBlitImageInfo2KHR(const VkBlitImageInfo2KHR* in_struct);
    safe_VkBlitImageInfo2KHR(const safe_VkBlitImageInfo2KHR& copy_src);
    safe_VkBlitImageInfo2KHR& operator=(const safe_VkBlitImageInfo2KHR& copy_src);
    safe_VkBlitImageInfo2KHR();
    ~safe_VkBlitImageInfo2KHR();
    void initialize(const VkBlitImageInfo2KHR* in_struct);
    void initialize(const safe_VkBlitImageInfo2KHR* copy_src);
    VkBlitImageInfo2KHR *ptr() { return reinterpret_cast<VkBlitImageInfo2KHR *>(this); }
    VkBlitImageInfo2KHR const *ptr() const { return reinterpret_cast<VkBlitImageInfo2KHR const *>(this); }
};

struct safe_VkImageResolve2KHR {
    VkStructureType sType;
    const void* pNext;
    VkImageSubresourceLayers srcSubresource;
    VkOffset3D srcOffset;
    VkImageSubresourceLayers dstSubresource;
    VkOffset3D dstOffset;
    VkExtent3D extent;
    safe_VkImageResolve2KHR(const VkImageResolve2KHR* in_struct);
    safe_VkImageResolve2KHR(const safe_VkImageResolve2KHR& copy_src);
    safe_VkImageResolve2KHR& operator=(const safe_VkImageResolve2KHR& copy_src);
    safe_VkImageResolve2KHR();
    ~safe_VkImageResolve2KHR();
    void initialize(const VkImageResolve2KHR* in_struct);
    void initialize(const safe_VkImageResolve2KHR* copy_src);
    VkImageResolve2KHR *ptr() { return reinterpret_cast<VkImageResolve2KHR *>(this); }
    VkImageResolve2KHR const *ptr() const { return reinterpret_cast<VkImageResolve2KHR const *>(this); }
};

struct safe_VkResolveImageInfo2KHR {
    VkStructureType sType;
    const void* pNext;
    VkImage srcImage;
    VkImageLayout srcImageLayout;
    VkImage dstImage;
    VkImageLayout dstImageLayout;
    uint32_t regionCount;
    safe_VkImageResolve2KHR* pRegions;
    safe_VkResolveImageInfo2KHR(const VkResolveImageInfo2KHR* in_struct);
    safe_VkResolveImageInfo2KHR(const safe_VkResolveImageInfo2KHR& copy_src);
    safe_VkResolveImageInfo2KHR& operator=(const safe_VkResolveImageInfo2KHR& copy_src);
    safe_VkResolveImageInfo2KHR();
    ~safe_VkResolveImageInfo2KHR();
    void initialize(const VkResolveImageInfo2KHR* in_struct);
    void initialize(const safe_VkResolveImageInfo2KHR* copy_src);
    VkResolveImageInfo2KHR *ptr() { return reinterpret_cast<VkResolveImageInfo2KHR *>(this); }
    VkResolveImageInfo2KHR const *ptr() const { return reinterpret_cast<VkResolveImageInfo2KHR const *>(this); }
};

struct safe_VkPhysicalDeviceTextureCompressionASTCHDRFeaturesEXT {
    VkStructureType sType;
    void* pNext;
    VkBool32 textureCompressionASTC_HDR;
    safe_VkPhysicalDeviceTextureCompressionASTCHDRFeaturesEXT(const VkPhysicalDeviceTextureCompressionASTCHDRFeaturesEXT* in_struct);
    safe_VkPhysicalDeviceTextureCompressionASTCHDRFeaturesEXT(const safe_VkPhysicalDeviceTextureCompressionASTCHDRFeaturesEXT& copy_src);
    safe_VkPhysicalDeviceTextureCompressionASTCHDRFeaturesEXT& operator=(const safe_VkPhysicalDeviceTextureCompressionASTCHDRFeaturesEXT& copy_src);
    safe_VkPhysicalDeviceTextureCompressionASTCHDRFeaturesEXT();
    ~safe_VkPhysicalDeviceTextureCompressionASTCHDRFeaturesEXT();
    void initialize(const VkPhysicalDeviceTextureCompressionASTCHDRFeaturesEXT* in_struct);
    void initialize(const safe_VkPhysicalDeviceTextureCompressionASTCHDRFeaturesEXT* copy_src);
    VkPhysicalDeviceTextureCompressionASTCHDRFeaturesEXT *ptr() { return reinterpret_cast<VkPhysicalDeviceTextureCompressionASTCHDRFeaturesEXT *>(this); }
    VkPhysicalDeviceTextureCompressionASTCHDRFeaturesEXT const *ptr() const { return reinterpret_cast<VkPhysicalDeviceTextureCompressionASTCHDRFeaturesEXT const *>(this); }
};

struct safe_VkImageViewASTCDecodeModeEXT {
    VkStructureType sType;
    const void* pNext;
    VkFormat decodeMode;
    safe_VkImageViewASTCDecodeModeEXT(const VkImageViewASTCDecodeModeEXT* in_struct);
    safe_VkImageViewASTCDecodeModeEXT(const safe_VkImageViewASTCDecodeModeEXT& copy_src);
    safe_VkImageViewASTCDecodeModeEXT& operator=(const safe_VkImageViewASTCDecodeModeEXT& copy_src);
    safe_VkImageViewASTCDecodeModeEXT();
    ~safe_VkImageViewASTCDecodeModeEXT();
    void initialize(const VkImageViewASTCDecodeModeEXT* in_struct);
    void initialize(const safe_VkImageViewASTCDecodeModeEXT* copy_src);
    VkImageViewASTCDecodeModeEXT *ptr() { return reinterpret_cast<VkImageViewASTCDecodeModeEXT *>(this); }
    VkImageViewASTCDecodeModeEXT const *ptr() const { return reinterpret_cast<VkImageViewASTCDecodeModeEXT const *>(this); }
};

struct safe_VkPhysicalDeviceASTCDecodeFeaturesEXT {
    VkStructureType sType;
    void* pNext;
    VkBool32 decodeModeSharedExponent;
    safe_VkPhysicalDeviceASTCDecodeFeaturesEXT(const VkPhysicalDeviceASTCDecodeFeaturesEXT* in_struct);
    safe_VkPhysicalDeviceASTCDecodeFeaturesEXT(const safe_VkPhysicalDeviceASTCDecodeFeaturesEXT& copy_src);
    safe_VkPhysicalDeviceASTCDecodeFeaturesEXT& operator=(const safe_VkPhysicalDeviceASTCDecodeFeaturesEXT& copy_src);
    safe_VkPhysicalDeviceASTCDecodeFeaturesEXT();
    ~safe_VkPhysicalDeviceASTCDecodeFeaturesEXT();
    void initialize(const VkPhysicalDeviceASTCDecodeFeaturesEXT* in_struct);
    void initialize(const safe_VkPhysicalDeviceASTCDecodeFeaturesEXT* copy_src);
    VkPhysicalDeviceASTCDecodeFeaturesEXT *ptr() { return reinterpret_cast<VkPhysicalDeviceASTCDecodeFeaturesEXT *>(this); }
    VkPhysicalDeviceASTCDecodeFeaturesEXT const *ptr() const { return reinterpret_cast<VkPhysicalDeviceASTCDecodeFeaturesEXT const *>(this); }
};

struct safe_VkSurfaceCapabilities2EXT {
    VkStructureType sType;
    void* pNext;
    uint32_t minImageCount;
    uint32_t maxImageCount;
    VkExtent2D currentExtent;
    VkExtent2D minImageExtent;
    VkExtent2D maxImageExtent;
    uint32_t maxImageArrayLayers;
    VkSurfaceTransformFlagsKHR supportedTransforms;
    VkSurfaceTransformFlagBitsKHR currentTransform;
    VkCompositeAlphaFlagsKHR supportedCompositeAlpha;
    VkImageUsageFlags supportedUsageFlags;
    VkSurfaceCounterFlagsEXT supportedSurfaceCounters;
    safe_VkSurfaceCapabilities2EXT(const VkSurfaceCapabilities2EXT* in_struct);
    safe_VkSurfaceCapabilities2EXT(const safe_VkSurfaceCapabilities2EXT& copy_src);
    safe_VkSurfaceCapabilities2EXT& operator=(const safe_VkSurfaceCapabilities2EXT& copy_src);
    safe_VkSurfaceCapabilities2EXT();
    ~safe_VkSurfaceCapabilities2EXT();
    void initialize(const VkSurfaceCapabilities2EXT* in_struct);
    void initialize(const safe_VkSurfaceCapabilities2EXT* copy_src);
    VkSurfaceCapabilities2EXT *ptr() { return reinterpret_cast<VkSurfaceCapabilities2EXT *>(this); }
    VkSurfaceCapabilities2EXT const *ptr() const { return reinterpret_cast<VkSurfaceCapabilities2EXT const *>(this); }
};

struct safe_VkDisplayPowerInfoEXT {
    VkStructureType sType;
    const void* pNext;
    VkDisplayPowerStateEXT powerState;
    safe_VkDisplayPowerInfoEXT(const VkDisplayPowerInfoEXT* in_struct);
    safe_VkDisplayPowerInfoEXT(const safe_VkDisplayPowerInfoEXT& copy_src);
    safe_VkDisplayPowerInfoEXT& operator=(const safe_VkDisplayPowerInfoEXT& copy_src);
    safe_VkDisplayPowerInfoEXT();
    ~safe_VkDisplayPowerInfoEXT();
    void initialize(const VkDisplayPowerInfoEXT* in_struct);
    void initialize(const safe_VkDisplayPowerInfoEXT* copy_src);
    VkDisplayPowerInfoEXT *ptr() { return reinterpret_cast<VkDisplayPowerInfoEXT *>(this); }
    VkDisplayPowerInfoEXT const *ptr() const { return reinterpret_cast<VkDisplayPowerInfoEXT const *>(this); }
};

struct safe_VkDeviceEventInfoEXT {
    VkStructureType sType;
    const void* pNext;
    VkDeviceEventTypeEXT deviceEvent;
    safe_VkDeviceEventInfoEXT(const VkDeviceEventInfoEXT* in_struct);
    safe_VkDeviceEventInfoEXT(const safe_VkDeviceEventInfoEXT& copy_src);
    safe_VkDeviceEventInfoEXT& operator=(const safe_VkDeviceEventInfoEXT& copy_src);
    safe_VkDeviceEventInfoEXT();
    ~safe_VkDeviceEventInfoEXT();
    void initialize(const VkDeviceEventInfoEXT* in_struct);
    void initialize(const safe_VkDeviceEventInfoEXT* copy_src);
    VkDeviceEventInfoEXT *ptr() { return reinterpret_cast<VkDeviceEventInfoEXT *>(this); }
    VkDeviceEventInfoEXT const *ptr() const { return reinterpret_cast<VkDeviceEventInfoEXT const *>(this); }
};

struct safe_VkDisplayEventInfoEXT {
    VkStructureType sType;
    const void* pNext;
    VkDisplayEventTypeEXT displayEvent;
    safe_VkDisplayEventInfoEXT(const VkDisplayEventInfoEXT* in_struct);
    safe_VkDisplayEventInfoEXT(const safe_VkDisplayEventInfoEXT& copy_src);
    safe_VkDisplayEventInfoEXT& operator=(const safe_VkDisplayEventInfoEXT& copy_src);
    safe_VkDisplayEventInfoEXT();
    ~safe_VkDisplayEventInfoEXT();
    void initialize(const VkDisplayEventInfoEXT* in_struct);
    void initialize(const safe_VkDisplayEventInfoEXT* copy_src);
    VkDisplayEventInfoEXT *ptr() { return reinterpret_cast<VkDisplayEventInfoEXT *>(this); }
    VkDisplayEventInfoEXT const *ptr() const { return reinterpret_cast<VkDisplayEventInfoEXT const *>(this); }
};

struct safe_VkSwapchainCounterCreateInfoEXT {
    VkStructureType sType;
    const void* pNext;
    VkSurfaceCounterFlagsEXT surfaceCounters;
    safe_VkSwapchainCounterCreateInfoEXT(const VkSwapchainCounterCreateInfoEXT* in_struct);
    safe_VkSwapchainCounterCreateInfoEXT(const safe_VkSwapchainCounterCreateInfoEXT& copy_src);
    safe_VkSwapchainCounterCreateInfoEXT& operator=(const safe_VkSwapchainCounterCreateInfoEXT& copy_src);
    safe_VkSwapchainCounterCreateInfoEXT();
    ~safe_VkSwapchainCounterCreateInfoEXT();
    void initialize(const VkSwapchainCounterCreateInfoEXT* in_struct);
    void initialize(const safe_VkSwapchainCounterCreateInfoEXT* copy_src);
    VkSwapchainCounterCreateInfoEXT *ptr() { return reinterpret_cast<VkSwapchainCounterCreateInfoEXT *>(this); }
    VkSwapchainCounterCreateInfoEXT const *ptr() const { return reinterpret_cast<VkSwapchainCounterCreateInfoEXT const *>(this); }
};

struct safe_VkPhysicalDeviceDiscardRectanglePropertiesEXT {
    VkStructureType sType;
    void* pNext;
    uint32_t maxDiscardRectangles;
    safe_VkPhysicalDeviceDiscardRectanglePropertiesEXT(const VkPhysicalDeviceDiscardRectanglePropertiesEXT* in_struct);
    safe_VkPhysicalDeviceDiscardRectanglePropertiesEXT(const safe_VkPhysicalDeviceDiscardRectanglePropertiesEXT& copy_src);
    safe_VkPhysicalDeviceDiscardRectanglePropertiesEXT& operator=(const safe_VkPhysicalDeviceDiscardRectanglePropertiesEXT& copy_src);
    safe_VkPhysicalDeviceDiscardRectanglePropertiesEXT();
    ~safe_VkPhysicalDeviceDiscardRectanglePropertiesEXT();
    void initialize(const VkPhysicalDeviceDiscardRectanglePropertiesEXT* in_struct);
    void initialize(const safe_VkPhysicalDeviceDiscardRectanglePropertiesEXT* copy_src);
    VkPhysicalDeviceDiscardRectanglePropertiesEXT *ptr() { return reinterpret_cast<VkPhysicalDeviceDiscardRectanglePropertiesEXT *>(this); }
    VkPhysicalDeviceDiscardRectanglePropertiesEXT const *ptr() const { return reinterpret_cast<VkPhysicalDeviceDiscardRectanglePropertiesEXT const *>(this); }
};

struct safe_VkPipelineDiscardRectangleStateCreateInfoEXT {
    VkStructureType sType;
    const void* pNext;
    VkPipelineDiscardRectangleStateCreateFlagsEXT flags;
    VkDiscardRectangleModeEXT discardRectangleMode;
    uint32_t discardRectangleCount;
    const VkRect2D* pDiscardRectangles;
    safe_VkPipelineDiscardRectangleStateCreateInfoEXT(const VkPipelineDiscardRectangleStateCreateInfoEXT* in_struct);
    safe_VkPipelineDiscardRectangleStateCreateInfoEXT(const safe_VkPipelineDiscardRectangleStateCreateInfoEXT& copy_src);
    safe_VkPipelineDiscardRectangleStateCreateInfoEXT& operator=(const safe_VkPipelineDiscardRectangleStateCreateInfoEXT& copy_src);
    safe_VkPipelineDiscardRectangleStateCreateInfoEXT();
    ~safe_VkPipelineDiscardRectangleStateCreateInfoEXT();
    void initialize(const VkPipelineDiscardRectangleStateCreateInfoEXT* in_struct);
    void initialize(const safe_VkPipelineDiscardRectangleStateCreateInfoEXT* copy_src);
    VkPipelineDiscardRectangleStateCreateInfoEXT *ptr() { return reinterpret_cast<VkPipelineDiscardRectangleStateCreateInfoEXT *>(this); }
    VkPipelineDiscardRectangleStateCreateInfoEXT const *ptr() const { return reinterpret_cast<VkPipelineDiscardRectangleStateCreateInfoEXT const *>(this); }
};

struct safe_VkPhysicalDeviceConservativeRasterizationPropertiesEXT {
    VkStructureType sType;
    void* pNext;
    float primitiveOverestimationSize;
    float maxExtraPrimitiveOverestimationSize;
    float extraPrimitiveOverestimationSizeGranularity;
    VkBool32 primitiveUnderestimation;
    VkBool32 conservativePointAndLineRasterization;
    VkBool32 degenerateTrianglesRasterized;
    VkBool32 degenerateLinesRasterized;
    VkBool32 fullyCoveredFragmentShaderInputVariable;
    VkBool32 conservativeRasterizationPostDepthCoverage;
    safe_VkPhysicalDeviceConservativeRasterizationPropertiesEXT(const VkPhysicalDeviceConservativeRasterizationPropertiesEXT* in_struct);
    safe_VkPhysicalDeviceConservativeRasterizationPropertiesEXT(const safe_VkPhysicalDeviceConservativeRasterizationPropertiesEXT& copy_src);
    safe_VkPhysicalDeviceConservativeRasterizationPropertiesEXT& operator=(const safe_VkPhysicalDeviceConservativeRasterizationPropertiesEXT& copy_src);
    safe_VkPhysicalDeviceConservativeRasterizationPropertiesEXT();
    ~safe_VkPhysicalDeviceConservativeRasterizationPropertiesEXT();
    void initialize(const VkPhysicalDeviceConservativeRasterizationPropertiesEXT* in_struct);
    void initialize(const safe_VkPhysicalDeviceConservativeRasterizationPropertiesEXT* copy_src);
    VkPhysicalDeviceConservativeRasterizationPropertiesEXT *ptr() { return reinterpret_cast<VkPhysicalDeviceConservativeRasterizationPropertiesEXT *>(this); }
    VkPhysicalDeviceConservativeRasterizationPropertiesEXT const *ptr() const { return reinterpret_cast<VkPhysicalDeviceConservativeRasterizationPropertiesEXT const *>(this); }
};

struct safe_VkPipelineRasterizationConservativeStateCreateInfoEXT {
    VkStructureType sType;
    const void* pNext;
    VkPipelineRasterizationConservativeStateCreateFlagsEXT flags;
    VkConservativeRasterizationModeEXT conservativeRasterizationMode;
    float extraPrimitiveOverestimationSize;
    safe_VkPipelineRasterizationConservativeStateCreateInfoEXT(const VkPipelineRasterizationConservativeStateCreateInfoEXT* in_struct);
    safe_VkPipelineRasterizationConservativeStateCreateInfoEXT(const safe_VkPipelineRasterizationConservativeStateCreateInfoEXT& copy_src);
    safe_VkPipelineRasterizationConservativeStateCreateInfoEXT& operator=(const safe_VkPipelineRasterizationConservativeStateCreateInfoEXT& copy_src);
    safe_VkPipelineRasterizationConservativeStateCreateInfoEXT();
    ~safe_VkPipelineRasterizationConservativeStateCreateInfoEXT();
    void initialize(const VkPipelineRasterizationConservativeStateCreateInfoEXT* in_struct);
    void initialize(const safe_VkPipelineRasterizationConservativeStateCreateInfoEXT* copy_src);
    VkPipelineRasterizationConservativeStateCreateInfoEXT *ptr() { return reinterpret_cast<VkPipelineRasterizationConservativeStateCreateInfoEXT *>(this); }
    VkPipelineRasterizationConservativeStateCreateInfoEXT const *ptr() const { return reinterpret_cast<VkPipelineRasterizationConservativeStateCreateInfoEXT const *>(this); }
};

struct safe_VkPhysicalDeviceDepthClipEnableFeaturesEXT {
    VkStructureType sType;
    void* pNext;
    VkBool32 depthClipEnable;
    safe_VkPhysicalDeviceDepthClipEnableFeaturesEXT(const VkPhysicalDeviceDepthClipEnableFeaturesEXT* in_struct);
    safe_VkPhysicalDeviceDepthClipEnableFeaturesEXT(const safe_VkPhysicalDeviceDepthClipEnableFeaturesEXT& copy_src);
    safe_VkPhysicalDeviceDepthClipEnableFeaturesEXT& operator=(const safe_VkPhysicalDeviceDepthClipEnableFeaturesEXT& copy_src);
    safe_VkPhysicalDeviceDepthClipEnableFeaturesEXT();
    ~safe_VkPhysicalDeviceDepthClipEnableFeaturesEXT();
    void initialize(const VkPhysicalDeviceDepthClipEnableFeaturesEXT* in_struct);
    void initialize(const safe_VkPhysicalDeviceDepthClipEnableFeaturesEXT* copy_src);
    VkPhysicalDeviceDepthClipEnableFeaturesEXT *ptr() { return reinterpret_cast<VkPhysicalDeviceDepthClipEnableFeaturesEXT *>(this); }
    VkPhysicalDeviceDepthClipEnableFeaturesEXT const *ptr() const { return reinterpret_cast<VkPhysicalDeviceDepthClipEnableFeaturesEXT const *>(this); }
};

struct safe_VkPipelineRasterizationDepthClipStateCreateInfoEXT {
    VkStructureType sType;
    const void* pNext;
    VkPipelineRasterizationDepthClipStateCreateFlagsEXT flags;
    VkBool32 depthClipEnable;
    safe_VkPipelineRasterizationDepthClipStateCreateInfoEXT(const VkPipelineRasterizationDepthClipStateCreateInfoEXT* in_struct);
    safe_VkPipelineRasterizationDepthClipStateCreateInfoEXT(const safe_VkPipelineRasterizationDepthClipStateCreateInfoEXT& copy_src);
    safe_VkPipelineRasterizationDepthClipStateCreateInfoEXT& operator=(const safe_VkPipelineRasterizationDepthClipStateCreateInfoEXT& copy_src);
    safe_VkPipelineRasterizationDepthClipStateCreateInfoEXT();
    ~safe_VkPipelineRasterizationDepthClipStateCreateInfoEXT();
    void initialize(const VkPipelineRasterizationDepthClipStateCreateInfoEXT* in_struct);
    void initialize(const safe_VkPipelineRasterizationDepthClipStateCreateInfoEXT* copy_src);
    VkPipelineRasterizationDepthClipStateCreateInfoEXT *ptr() { return reinterpret_cast<VkPipelineRasterizationDepthClipStateCreateInfoEXT *>(this); }
    VkPipelineRasterizationDepthClipStateCreateInfoEXT const *ptr() const { return reinterpret_cast<VkPipelineRasterizationDepthClipStateCreateInfoEXT const *>(this); }
};

struct safe_VkHdrMetadataEXT {
    VkStructureType sType;
    const void* pNext;
    VkXYColorEXT displayPrimaryRed;
    VkXYColorEXT displayPrimaryGreen;
    VkXYColorEXT displayPrimaryBlue;
    VkXYColorEXT whitePoint;
    float maxLuminance;
    float minLuminance;
    float maxContentLightLevel;
    float maxFrameAverageLightLevel;
    safe_VkHdrMetadataEXT(const VkHdrMetadataEXT* in_struct);
    safe_VkHdrMetadataEXT(const safe_VkHdrMetadataEXT& copy_src);
    safe_VkHdrMetadataEXT& operator=(const safe_VkHdrMetadataEXT& copy_src);
    safe_VkHdrMetadataEXT();
    ~safe_VkHdrMetadataEXT();
    void initialize(const VkHdrMetadataEXT* in_struct);
    void initialize(const safe_VkHdrMetadataEXT* copy_src);
    VkHdrMetadataEXT *ptr() { return reinterpret_cast<VkHdrMetadataEXT *>(this); }
    VkHdrMetadataEXT const *ptr() const { return reinterpret_cast<VkHdrMetadataEXT const *>(this); }
};

struct safe_VkDebugUtilsLabelEXT {
    VkStructureType sType;
    const void* pNext;
    const char* pLabelName;
    float color[4];
    safe_VkDebugUtilsLabelEXT(const VkDebugUtilsLabelEXT* in_struct);
    safe_VkDebugUtilsLabelEXT(const safe_VkDebugUtilsLabelEXT& copy_src);
    safe_VkDebugUtilsLabelEXT& operator=(const safe_VkDebugUtilsLabelEXT& copy_src);
    safe_VkDebugUtilsLabelEXT();
    ~safe_VkDebugUtilsLabelEXT();
    void initialize(const VkDebugUtilsLabelEXT* in_struct);
    void initialize(const safe_VkDebugUtilsLabelEXT* copy_src);
    VkDebugUtilsLabelEXT *ptr() { return reinterpret_cast<VkDebugUtilsLabelEXT *>(this); }
    VkDebugUtilsLabelEXT const *ptr() const { return reinterpret_cast<VkDebugUtilsLabelEXT const *>(this); }
};

struct safe_VkDebugUtilsObjectNameInfoEXT {
    VkStructureType sType;
    const void* pNext;
    VkObjectType objectType;
    uint64_t objectHandle;
    const char* pObjectName;
    safe_VkDebugUtilsObjectNameInfoEXT(const VkDebugUtilsObjectNameInfoEXT* in_struct);
    safe_VkDebugUtilsObjectNameInfoEXT(const safe_VkDebugUtilsObjectNameInfoEXT& copy_src);
    safe_VkDebugUtilsObjectNameInfoEXT& operator=(const safe_VkDebugUtilsObjectNameInfoEXT& copy_src);
    safe_VkDebugUtilsObjectNameInfoEXT();
    ~safe_VkDebugUtilsObjectNameInfoEXT();
    void initialize(const VkDebugUtilsObjectNameInfoEXT* in_struct);
    void initialize(const safe_VkDebugUtilsObjectNameInfoEXT* copy_src);
    VkDebugUtilsObjectNameInfoEXT *ptr() { return reinterpret_cast<VkDebugUtilsObjectNameInfoEXT *>(this); }
    VkDebugUtilsObjectNameInfoEXT const *ptr() const { return reinterpret_cast<VkDebugUtilsObjectNameInfoEXT const *>(this); }
};

struct safe_VkDebugUtilsMessengerCallbackDataEXT {
    VkStructureType sType;
    const void* pNext;
    VkDebugUtilsMessengerCallbackDataFlagsEXT flags;
    const char* pMessageIdName;
    int32_t messageIdNumber;
    const char* pMessage;
    uint32_t queueLabelCount;
    safe_VkDebugUtilsLabelEXT* pQueueLabels;
    uint32_t cmdBufLabelCount;
    safe_VkDebugUtilsLabelEXT* pCmdBufLabels;
    uint32_t objectCount;
    safe_VkDebugUtilsObjectNameInfoEXT* pObjects;
    safe_VkDebugUtilsMessengerCallbackDataEXT(const VkDebugUtilsMessengerCallbackDataEXT* in_struct);
    safe_VkDebugUtilsMessengerCallbackDataEXT(const safe_VkDebugUtilsMessengerCallbackDataEXT& copy_src);
    safe_VkDebugUtilsMessengerCallbackDataEXT& operator=(const safe_VkDebugUtilsMessengerCallbackDataEXT& copy_src);
    safe_VkDebugUtilsMessengerCallbackDataEXT();
    ~safe_VkDebugUtilsMessengerCallbackDataEXT();
    void initialize(const VkDebugUtilsMessengerCallbackDataEXT* in_struct);
    void initialize(const safe_VkDebugUtilsMessengerCallbackDataEXT* copy_src);
    VkDebugUtilsMessengerCallbackDataEXT *ptr() { return reinterpret_cast<VkDebugUtilsMessengerCallbackDataEXT *>(this); }
    VkDebugUtilsMessengerCallbackDataEXT const *ptr() const { return reinterpret_cast<VkDebugUtilsMessengerCallbackDataEXT const *>(this); }
};

struct safe_VkDebugUtilsMessengerCreateInfoEXT {
    VkStructureType sType;
    const void* pNext;
    VkDebugUtilsMessengerCreateFlagsEXT flags;
    VkDebugUtilsMessageSeverityFlagsEXT messageSeverity;
    VkDebugUtilsMessageTypeFlagsEXT messageType;
    PFN_vkDebugUtilsMessengerCallbackEXT pfnUserCallback;
    void* pUserData;
    safe_VkDebugUtilsMessengerCreateInfoEXT(const VkDebugUtilsMessengerCreateInfoEXT* in_struct);
    safe_VkDebugUtilsMessengerCreateInfoEXT(const safe_VkDebugUtilsMessengerCreateInfoEXT& copy_src);
    safe_VkDebugUtilsMessengerCreateInfoEXT& operator=(const safe_VkDebugUtilsMessengerCreateInfoEXT& copy_src);
    safe_VkDebugUtilsMessengerCreateInfoEXT();
    ~safe_VkDebugUtilsMessengerCreateInfoEXT();
    void initialize(const VkDebugUtilsMessengerCreateInfoEXT* in_struct);
    void initialize(const safe_VkDebugUtilsMessengerCreateInfoEXT* copy_src);
    VkDebugUtilsMessengerCreateInfoEXT *ptr() { return reinterpret_cast<VkDebugUtilsMessengerCreateInfoEXT *>(this); }
    VkDebugUtilsMessengerCreateInfoEXT const *ptr() const { return reinterpret_cast<VkDebugUtilsMessengerCreateInfoEXT const *>(this); }
};

struct safe_VkDebugUtilsObjectTagInfoEXT {
    VkStructureType sType;
    const void* pNext;
    VkObjectType objectType;
    uint64_t objectHandle;
    uint64_t tagName;
    size_t tagSize;
    const void* pTag;
    safe_VkDebugUtilsObjectTagInfoEXT(const VkDebugUtilsObjectTagInfoEXT* in_struct);
    safe_VkDebugUtilsObjectTagInfoEXT(const safe_VkDebugUtilsObjectTagInfoEXT& copy_src);
    safe_VkDebugUtilsObjectTagInfoEXT& operator=(const safe_VkDebugUtilsObjectTagInfoEXT& copy_src);
    safe_VkDebugUtilsObjectTagInfoEXT();
    ~safe_VkDebugUtilsObjectTagInfoEXT();
    void initialize(const VkDebugUtilsObjectTagInfoEXT* in_struct);
    void initialize(const safe_VkDebugUtilsObjectTagInfoEXT* copy_src);
    VkDebugUtilsObjectTagInfoEXT *ptr() { return reinterpret_cast<VkDebugUtilsObjectTagInfoEXT *>(this); }
    VkDebugUtilsObjectTagInfoEXT const *ptr() const { return reinterpret_cast<VkDebugUtilsObjectTagInfoEXT const *>(this); }
};

struct safe_VkSampleLocationsInfoEXT {
    VkStructureType sType;
    const void* pNext;
    VkSampleCountFlagBits sampleLocationsPerPixel;
    VkExtent2D sampleLocationGridSize;
    uint32_t sampleLocationsCount;
    const VkSampleLocationEXT* pSampleLocations;
    safe_VkSampleLocationsInfoEXT(const VkSampleLocationsInfoEXT* in_struct);
    safe_VkSampleLocationsInfoEXT(const safe_VkSampleLocationsInfoEXT& copy_src);
    safe_VkSampleLocationsInfoEXT& operator=(const safe_VkSampleLocationsInfoEXT& copy_src);
    safe_VkSampleLocationsInfoEXT();
    ~safe_VkSampleLocationsInfoEXT();
    void initialize(const VkSampleLocationsInfoEXT* in_struct);
    void initialize(const safe_VkSampleLocationsInfoEXT* copy_src);
    VkSampleLocationsInfoEXT *ptr() { return reinterpret_cast<VkSampleLocationsInfoEXT *>(this); }
    VkSampleLocationsInfoEXT const *ptr() const { return reinterpret_cast<VkSampleLocationsInfoEXT const *>(this); }
};

struct safe_VkRenderPassSampleLocationsBeginInfoEXT {
    VkStructureType sType;
    const void* pNext;
    uint32_t attachmentInitialSampleLocationsCount;
    const VkAttachmentSampleLocationsEXT* pAttachmentInitialSampleLocations;
    uint32_t postSubpassSampleLocationsCount;
    const VkSubpassSampleLocationsEXT* pPostSubpassSampleLocations;
    safe_VkRenderPassSampleLocationsBeginInfoEXT(const VkRenderPassSampleLocationsBeginInfoEXT* in_struct);
    safe_VkRenderPassSampleLocationsBeginInfoEXT(const safe_VkRenderPassSampleLocationsBeginInfoEXT& copy_src);
    safe_VkRenderPassSampleLocationsBeginInfoEXT& operator=(const safe_VkRenderPassSampleLocationsBeginInfoEXT& copy_src);
    safe_VkRenderPassSampleLocationsBeginInfoEXT();
    ~safe_VkRenderPassSampleLocationsBeginInfoEXT();
    void initialize(const VkRenderPassSampleLocationsBeginInfoEXT* in_struct);
    void initialize(const safe_VkRenderPassSampleLocationsBeginInfoEXT* copy_src);
    VkRenderPassSampleLocationsBeginInfoEXT *ptr() { return reinterpret_cast<VkRenderPassSampleLocationsBeginInfoEXT *>(this); }
    VkRenderPassSampleLocationsBeginInfoEXT const *ptr() const { return reinterpret_cast<VkRenderPassSampleLocationsBeginInfoEXT const *>(this); }
};

struct safe_VkPipelineSampleLocationsStateCreateInfoEXT {
    VkStructureType sType;
    const void* pNext;
    VkBool32 sampleLocationsEnable;
    safe_VkSampleLocationsInfoEXT sampleLocationsInfo;
    safe_VkPipelineSampleLocationsStateCreateInfoEXT(const VkPipelineSampleLocationsStateCreateInfoEXT* in_struct);
    safe_VkPipelineSampleLocationsStateCreateInfoEXT(const safe_VkPipelineSampleLocationsStateCreateInfoEXT& copy_src);
    safe_VkPipelineSampleLocationsStateCreateInfoEXT& operator=(const safe_VkPipelineSampleLocationsStateCreateInfoEXT& copy_src);
    safe_VkPipelineSampleLocationsStateCreateInfoEXT();
    ~safe_VkPipelineSampleLocationsStateCreateInfoEXT();
    void initialize(const VkPipelineSampleLocationsStateCreateInfoEXT* in_struct);
    void initialize(const safe_VkPipelineSampleLocationsStateCreateInfoEXT* copy_src);
    VkPipelineSampleLocationsStateCreateInfoEXT *ptr() { return reinterpret_cast<VkPipelineSampleLocationsStateCreateInfoEXT *>(this); }
    VkPipelineSampleLocationsStateCreateInfoEXT const *ptr() const { return reinterpret_cast<VkPipelineSampleLocationsStateCreateInfoEXT const *>(this); }
};

struct safe_VkPhysicalDeviceSampleLocationsPropertiesEXT {
    VkStructureType sType;
    void* pNext;
    VkSampleCountFlags sampleLocationSampleCounts;
    VkExtent2D maxSampleLocationGridSize;
    float sampleLocationCoordinateRange[2];
    uint32_t sampleLocationSubPixelBits;
    VkBool32 variableSampleLocations;
    safe_VkPhysicalDeviceSampleLocationsPropertiesEXT(const VkPhysicalDeviceSampleLocationsPropertiesEXT* in_struct);
    safe_VkPhysicalDeviceSampleLocationsPropertiesEXT(const safe_VkPhysicalDeviceSampleLocationsPropertiesEXT& copy_src);
    safe_VkPhysicalDeviceSampleLocationsPropertiesEXT& operator=(const safe_VkPhysicalDeviceSampleLocationsPropertiesEXT& copy_src);
    safe_VkPhysicalDeviceSampleLocationsPropertiesEXT();
    ~safe_VkPhysicalDeviceSampleLocationsPropertiesEXT();
    void initialize(const VkPhysicalDeviceSampleLocationsPropertiesEXT* in_struct);
    void initialize(const safe_VkPhysicalDeviceSampleLocationsPropertiesEXT* copy_src);
    VkPhysicalDeviceSampleLocationsPropertiesEXT *ptr() { return reinterpret_cast<VkPhysicalDeviceSampleLocationsPropertiesEXT *>(this); }
    VkPhysicalDeviceSampleLocationsPropertiesEXT const *ptr() const { return reinterpret_cast<VkPhysicalDeviceSampleLocationsPropertiesEXT const *>(this); }
};

struct safe_VkMultisamplePropertiesEXT {
    VkStructureType sType;
    void* pNext;
    VkExtent2D maxSampleLocationGridSize;
    safe_VkMultisamplePropertiesEXT(const VkMultisamplePropertiesEXT* in_struct);
    safe_VkMultisamplePropertiesEXT(const safe_VkMultisamplePropertiesEXT& copy_src);
    safe_VkMultisamplePropertiesEXT& operator=(const safe_VkMultisamplePropertiesEXT& copy_src);
    safe_VkMultisamplePropertiesEXT();
    ~safe_VkMultisamplePropertiesEXT();
    void initialize(const VkMultisamplePropertiesEXT* in_struct);
    void initialize(const safe_VkMultisamplePropertiesEXT* copy_src);
    VkMultisamplePropertiesEXT *ptr() { return reinterpret_cast<VkMultisamplePropertiesEXT *>(this); }
    VkMultisamplePropertiesEXT const *ptr() const { return reinterpret_cast<VkMultisamplePropertiesEXT const *>(this); }
};

struct safe_VkPhysicalDeviceBlendOperationAdvancedFeaturesEXT {
    VkStructureType sType;
    void* pNext;
    VkBool32 advancedBlendCoherentOperations;
    safe_VkPhysicalDeviceBlendOperationAdvancedFeaturesEXT(const VkPhysicalDeviceBlendOperationAdvancedFeaturesEXT* in_struct);
    safe_VkPhysicalDeviceBlendOperationAdvancedFeaturesEXT(const safe_VkPhysicalDeviceBlendOperationAdvancedFeaturesEXT& copy_src);
    safe_VkPhysicalDeviceBlendOperationAdvancedFeaturesEXT& operator=(const safe_VkPhysicalDeviceBlendOperationAdvancedFeaturesEXT& copy_src);
    safe_VkPhysicalDeviceBlendOperationAdvancedFeaturesEXT();
    ~safe_VkPhysicalDeviceBlendOperationAdvancedFeaturesEXT();
    void initialize(const VkPhysicalDeviceBlendOperationAdvancedFeaturesEXT* in_struct);
    void initialize(const safe_VkPhysicalDeviceBlendOperationAdvancedFeaturesEXT* copy_src);
    VkPhysicalDeviceBlendOperationAdvancedFeaturesEXT *ptr() { return reinterpret_cast<VkPhysicalDeviceBlendOperationAdvancedFeaturesEXT *>(this); }
    VkPhysicalDeviceBlendOperationAdvancedFeaturesEXT const *ptr() const { return reinterpret_cast<VkPhysicalDeviceBlendOperationAdvancedFeaturesEXT const *>(this); }
};

struct safe_VkPhysicalDeviceBlendOperationAdvancedPropertiesEXT {
    VkStructureType sType;
    void* pNext;
    uint32_t advancedBlendMaxColorAttachments;
    VkBool32 advancedBlendIndependentBlend;
    VkBool32 advancedBlendNonPremultipliedSrcColor;
    VkBool32 advancedBlendNonPremultipliedDstColor;
    VkBool32 advancedBlendCorrelatedOverlap;
    VkBool32 advancedBlendAllOperations;
    safe_VkPhysicalDeviceBlendOperationAdvancedPropertiesEXT(const VkPhysicalDeviceBlendOperationAdvancedPropertiesEXT* in_struct);
    safe_VkPhysicalDeviceBlendOperationAdvancedPropertiesEXT(const safe_VkPhysicalDeviceBlendOperationAdvancedPropertiesEXT& copy_src);
    safe_VkPhysicalDeviceBlendOperationAdvancedPropertiesEXT& operator=(const safe_VkPhysicalDeviceBlendOperationAdvancedPropertiesEXT& copy_src);
    safe_VkPhysicalDeviceBlendOperationAdvancedPropertiesEXT();
    ~safe_VkPhysicalDeviceBlendOperationAdvancedPropertiesEXT();
    void initialize(const VkPhysicalDeviceBlendOperationAdvancedPropertiesEXT* in_struct);
    void initialize(const safe_VkPhysicalDeviceBlendOperationAdvancedPropertiesEXT* copy_src);
    VkPhysicalDeviceBlendOperationAdvancedPropertiesEXT *ptr() { return reinterpret_cast<VkPhysicalDeviceBlendOperationAdvancedPropertiesEXT *>(this); }
    VkPhysicalDeviceBlendOperationAdvancedPropertiesEXT const *ptr() const { return reinterpret_cast<VkPhysicalDeviceBlendOperationAdvancedPropertiesEXT const *>(this); }
};

struct safe_VkPipelineColorBlendAdvancedStateCreateInfoEXT {
    VkStructureType sType;
    const void* pNext;
    VkBool32 srcPremultiplied;
    VkBool32 dstPremultiplied;
    VkBlendOverlapEXT blendOverlap;
    safe_VkPipelineColorBlendAdvancedStateCreateInfoEXT(const VkPipelineColorBlendAdvancedStateCreateInfoEXT* in_struct);
    safe_VkPipelineColorBlendAdvancedStateCreateInfoEXT(const safe_VkPipelineColorBlendAdvancedStateCreateInfoEXT& copy_src);
    safe_VkPipelineColorBlendAdvancedStateCreateInfoEXT& operator=(const safe_VkPipelineColorBlendAdvancedStateCreateInfoEXT& copy_src);
    safe_VkPipelineColorBlendAdvancedStateCreateInfoEXT();
    ~safe_VkPipelineColorBlendAdvancedStateCreateInfoEXT();
    void initialize(const VkPipelineColorBlendAdvancedStateCreateInfoEXT* in_struct);
    void initialize(const safe_VkPipelineColorBlendAdvancedStateCreateInfoEXT* copy_src);
    VkPipelineColorBlendAdvancedStateCreateInfoEXT *ptr() { return reinterpret_cast<VkPipelineColorBlendAdvancedStateCreateInfoEXT *>(this); }
    VkPipelineColorBlendAdvancedStateCreateInfoEXT const *ptr() const { return reinterpret_cast<VkPipelineColorBlendAdvancedStateCreateInfoEXT const *>(this); }
};

struct safe_VkDrmFormatModifierPropertiesListEXT {
    VkStructureType sType;
    void* pNext;
    uint32_t drmFormatModifierCount;
    VkDrmFormatModifierPropertiesEXT* pDrmFormatModifierProperties;
    safe_VkDrmFormatModifierPropertiesListEXT(const VkDrmFormatModifierPropertiesListEXT* in_struct);
    safe_VkDrmFormatModifierPropertiesListEXT(const safe_VkDrmFormatModifierPropertiesListEXT& copy_src);
    safe_VkDrmFormatModifierPropertiesListEXT& operator=(const safe_VkDrmFormatModifierPropertiesListEXT& copy_src);
    safe_VkDrmFormatModifierPropertiesListEXT();
    ~safe_VkDrmFormatModifierPropertiesListEXT();
    void initialize(const VkDrmFormatModifierPropertiesListEXT* in_struct);
    void initialize(const safe_VkDrmFormatModifierPropertiesListEXT* copy_src);
    VkDrmFormatModifierPropertiesListEXT *ptr() { return reinterpret_cast<VkDrmFormatModifierPropertiesListEXT *>(this); }
    VkDrmFormatModifierPropertiesListEXT const *ptr() const { return reinterpret_cast<VkDrmFormatModifierPropertiesListEXT const *>(this); }
};

struct safe_VkPhysicalDeviceImageDrmFormatModifierInfoEXT {
    VkStructureType sType;
    const void* pNext;
    uint64_t drmFormatModifier;
    VkSharingMode sharingMode;
    uint32_t queueFamilyIndexCount;
    const uint32_t* pQueueFamilyIndices;
    safe_VkPhysicalDeviceImageDrmFormatModifierInfoEXT(const VkPhysicalDeviceImageDrmFormatModifierInfoEXT* in_struct);
    safe_VkPhysicalDeviceImageDrmFormatModifierInfoEXT(const safe_VkPhysicalDeviceImageDrmFormatModifierInfoEXT& copy_src);
    safe_VkPhysicalDeviceImageDrmFormatModifierInfoEXT& operator=(const safe_VkPhysicalDeviceImageDrmFormatModifierInfoEXT& copy_src);
    safe_VkPhysicalDeviceImageDrmFormatModifierInfoEXT();
    ~safe_VkPhysicalDeviceImageDrmFormatModifierInfoEXT();
    void initialize(const VkPhysicalDeviceImageDrmFormatModifierInfoEXT* in_struct);
    void initialize(const safe_VkPhysicalDeviceImageDrmFormatModifierInfoEXT* copy_src);
    VkPhysicalDeviceImageDrmFormatModifierInfoEXT *ptr() { return reinterpret_cast<VkPhysicalDeviceImageDrmFormatModifierInfoEXT *>(this); }
    VkPhysicalDeviceImageDrmFormatModifierInfoEXT const *ptr() const { return reinterpret_cast<VkPhysicalDeviceImageDrmFormatModifierInfoEXT const *>(this); }
};

struct safe_VkImageDrmFormatModifierListCreateInfoEXT {
    VkStructureType sType;
    const void* pNext;
    uint32_t drmFormatModifierCount;
    const uint64_t* pDrmFormatModifiers;
    safe_VkImageDrmFormatModifierListCreateInfoEXT(const VkImageDrmFormatModifierListCreateInfoEXT* in_struct);
    safe_VkImageDrmFormatModifierListCreateInfoEXT(const safe_VkImageDrmFormatModifierListCreateInfoEXT& copy_src);
    safe_VkImageDrmFormatModifierListCreateInfoEXT& operator=(const safe_VkImageDrmFormatModifierListCreateInfoEXT& copy_src);
    safe_VkImageDrmFormatModifierListCreateInfoEXT();
    ~safe_VkImageDrmFormatModifierListCreateInfoEXT();
    void initialize(const VkImageDrmFormatModifierListCreateInfoEXT* in_struct);
    void initialize(const safe_VkImageDrmFormatModifierListCreateInfoEXT* copy_src);
    VkImageDrmFormatModifierListCreateInfoEXT *ptr() { return reinterpret_cast<VkImageDrmFormatModifierListCreateInfoEXT *>(this); }
    VkImageDrmFormatModifierListCreateInfoEXT const *ptr() const { return reinterpret_cast<VkImageDrmFormatModifierListCreateInfoEXT const *>(this); }
};

struct safe_VkImageDrmFormatModifierExplicitCreateInfoEXT {
    VkStructureType sType;
    const void* pNext;
    uint64_t drmFormatModifier;
    uint32_t drmFormatModifierPlaneCount;
    const VkSubresourceLayout* pPlaneLayouts;
    safe_VkImageDrmFormatModifierExplicitCreateInfoEXT(const VkImageDrmFormatModifierExplicitCreateInfoEXT* in_struct);
    safe_VkImageDrmFormatModifierExplicitCreateInfoEXT(const safe_VkImageDrmFormatModifierExplicitCreateInfoEXT& copy_src);
    safe_VkImageDrmFormatModifierExplicitCreateInfoEXT& operator=(const safe_VkImageDrmFormatModifierExplicitCreateInfoEXT& copy_src);
    safe_VkImageDrmFormatModifierExplicitCreateInfoEXT();
    ~safe_VkImageDrmFormatModifierExplicitCreateInfoEXT();
    void initialize(const VkImageDrmFormatModifierExplicitCreateInfoEXT* in_struct);
    void initialize(const safe_VkImageDrmFormatModifierExplicitCreateInfoEXT* copy_src);
    VkImageDrmFormatModifierExplicitCreateInfoEXT *ptr() { return reinterpret_cast<VkImageDrmFormatModifierExplicitCreateInfoEXT *>(this); }
    VkImageDrmFormatModifierExplicitCreateInfoEXT const *ptr() const { return reinterpret_cast<VkImageDrmFormatModifierExplicitCreateInfoEXT const *>(this); }
};

struct safe_VkImageDrmFormatModifierPropertiesEXT {
    VkStructureType sType;
    void* pNext;
    uint64_t drmFormatModifier;
    safe_VkImageDrmFormatModifierPropertiesEXT(const VkImageDrmFormatModifierPropertiesEXT* in_struct);
    safe_VkImageDrmFormatModifierPropertiesEXT(const safe_VkImageDrmFormatModifierPropertiesEXT& copy_src);
    safe_VkImageDrmFormatModifierPropertiesEXT& operator=(const safe_VkImageDrmFormatModifierPropertiesEXT& copy_src);
    safe_VkImageDrmFormatModifierPropertiesEXT();
    ~safe_VkImageDrmFormatModifierPropertiesEXT();
    void initialize(const VkImageDrmFormatModifierPropertiesEXT* in_struct);
    void initialize(const safe_VkImageDrmFormatModifierPropertiesEXT* copy_src);
    VkImageDrmFormatModifierPropertiesEXT *ptr() { return reinterpret_cast<VkImageDrmFormatModifierPropertiesEXT *>(this); }
    VkImageDrmFormatModifierPropertiesEXT const *ptr() const { return reinterpret_cast<VkImageDrmFormatModifierPropertiesEXT const *>(this); }
};

struct safe_VkPhysicalDeviceImageViewImageFormatInfoEXT {
    VkStructureType sType;
    void* pNext;
    VkImageViewType imageViewType;
    safe_VkPhysicalDeviceImageViewImageFormatInfoEXT(const VkPhysicalDeviceImageViewImageFormatInfoEXT* in_struct);
    safe_VkPhysicalDeviceImageViewImageFormatInfoEXT(const safe_VkPhysicalDeviceImageViewImageFormatInfoEXT& copy_src);
    safe_VkPhysicalDeviceImageViewImageFormatInfoEXT& operator=(const safe_VkPhysicalDeviceImageViewImageFormatInfoEXT& copy_src);
    safe_VkPhysicalDeviceImageViewImageFormatInfoEXT();
    ~safe_VkPhysicalDeviceImageViewImageFormatInfoEXT();
    void initialize(const VkPhysicalDeviceImageViewImageFormatInfoEXT* in_struct);
    void initialize(const safe_VkPhysicalDeviceImageViewImageFormatInfoEXT* copy_src);
    VkPhysicalDeviceImageViewImageFormatInfoEXT *ptr() { return reinterpret_cast<VkPhysicalDeviceImageViewImageFormatInfoEXT *>(this); }
    VkPhysicalDeviceImageViewImageFormatInfoEXT const *ptr() const { return reinterpret_cast<VkPhysicalDeviceImageViewImageFormatInfoEXT const *>(this); }
};

struct safe_VkFilterCubicImageViewImageFormatPropertiesEXT {
    VkStructureType sType;
    void* pNext;
    VkBool32 filterCubic;
    VkBool32 filterCubicMinmax;
    safe_VkFilterCubicImageViewImageFormatPropertiesEXT(const VkFilterCubicImageViewImageFormatPropertiesEXT* in_struct);
    safe_VkFilterCubicImageViewImageFormatPropertiesEXT(const safe_VkFilterCubicImageViewImageFormatPropertiesEXT& copy_src);
    safe_VkFilterCubicImageViewImageFormatPropertiesEXT& operator=(const safe_VkFilterCubicImageViewImageFormatPropertiesEXT& copy_src);
    safe_VkFilterCubicImageViewImageFormatPropertiesEXT();
    ~safe_VkFilterCubicImageViewImageFormatPropertiesEXT();
    void initialize(const VkFilterCubicImageViewImageFormatPropertiesEXT* in_struct);
    void initialize(const safe_VkFilterCubicImageViewImageFormatPropertiesEXT* copy_src);
    VkFilterCubicImageViewImageFormatPropertiesEXT *ptr() { return reinterpret_cast<VkFilterCubicImageViewImageFormatPropertiesEXT *>(this); }
    VkFilterCubicImageViewImageFormatPropertiesEXT const *ptr() const { return reinterpret_cast<VkFilterCubicImageViewImageFormatPropertiesEXT const *>(this); }
};

struct safe_VkDeviceQueueGlobalPriorityCreateInfoEXT {
    VkStructureType sType;
    const void* pNext;
    VkQueueGlobalPriorityEXT globalPriority;
    safe_VkDeviceQueueGlobalPriorityCreateInfoEXT(const VkDeviceQueueGlobalPriorityCreateInfoEXT* in_struct);
    safe_VkDeviceQueueGlobalPriorityCreateInfoEXT(const safe_VkDeviceQueueGlobalPriorityCreateInfoEXT& copy_src);
    safe_VkDeviceQueueGlobalPriorityCreateInfoEXT& operator=(const safe_VkDeviceQueueGlobalPriorityCreateInfoEXT& copy_src);
    safe_VkDeviceQueueGlobalPriorityCreateInfoEXT();
    ~safe_VkDeviceQueueGlobalPriorityCreateInfoEXT();
    void initialize(const VkDeviceQueueGlobalPriorityCreateInfoEXT* in_struct);
    void initialize(const safe_VkDeviceQueueGlobalPriorityCreateInfoEXT* copy_src);
    VkDeviceQueueGlobalPriorityCreateInfoEXT *ptr() { return reinterpret_cast<VkDeviceQueueGlobalPriorityCreateInfoEXT *>(this); }
    VkDeviceQueueGlobalPriorityCreateInfoEXT const *ptr() const { return reinterpret_cast<VkDeviceQueueGlobalPriorityCreateInfoEXT const *>(this); }
};

struct safe_VkImportMemoryHostPointerInfoEXT {
    VkStructureType sType;
    const void* pNext;
    VkExternalMemoryHandleTypeFlagBits handleType;
    void* pHostPointer;
    safe_VkImportMemoryHostPointerInfoEXT(const VkImportMemoryHostPointerInfoEXT* in_struct);
    safe_VkImportMemoryHostPointerInfoEXT(const safe_VkImportMemoryHostPointerInfoEXT& copy_src);
    safe_VkImportMemoryHostPointerInfoEXT& operator=(const safe_VkImportMemoryHostPointerInfoEXT& copy_src);
    safe_VkImportMemoryHostPointerInfoEXT();
    ~safe_VkImportMemoryHostPointerInfoEXT();
    void initialize(const VkImportMemoryHostPointerInfoEXT* in_struct);
    void initialize(const safe_VkImportMemoryHostPointerInfoEXT* copy_src);
    VkImportMemoryHostPointerInfoEXT *ptr() { return reinterpret_cast<VkImportMemoryHostPointerInfoEXT *>(this); }
    VkImportMemoryHostPointerInfoEXT const *ptr() const { return reinterpret_cast<VkImportMemoryHostPointerInfoEXT const *>(this); }
};

struct safe_VkMemoryHostPointerPropertiesEXT {
    VkStructureType sType;
    void* pNext;
    uint32_t memoryTypeBits;
    safe_VkMemoryHostPointerPropertiesEXT(const VkMemoryHostPointerPropertiesEXT* in_struct);
    safe_VkMemoryHostPointerPropertiesEXT(const safe_VkMemoryHostPointerPropertiesEXT& copy_src);
    safe_VkMemoryHostPointerPropertiesEXT& operator=(const safe_VkMemoryHostPointerPropertiesEXT& copy_src);
    safe_VkMemoryHostPointerPropertiesEXT();
    ~safe_VkMemoryHostPointerPropertiesEXT();
    void initialize(const VkMemoryHostPointerPropertiesEXT* in_struct);
    void initialize(const safe_VkMemoryHostPointerPropertiesEXT* copy_src);
    VkMemoryHostPointerPropertiesEXT *ptr() { return reinterpret_cast<VkMemoryHostPointerPropertiesEXT *>(this); }
    VkMemoryHostPointerPropertiesEXT const *ptr() const { return reinterpret_cast<VkMemoryHostPointerPropertiesEXT const *>(this); }
};

struct safe_VkPhysicalDeviceExternalMemoryHostPropertiesEXT {
    VkStructureType sType;
    void* pNext;
    VkDeviceSize minImportedHostPointerAlignment;
    safe_VkPhysicalDeviceExternalMemoryHostPropertiesEXT(const VkPhysicalDeviceExternalMemoryHostPropertiesEXT* in_struct);
    safe_VkPhysicalDeviceExternalMemoryHostPropertiesEXT(const safe_VkPhysicalDeviceExternalMemoryHostPropertiesEXT& copy_src);
    safe_VkPhysicalDeviceExternalMemoryHostPropertiesEXT& operator=(const safe_VkPhysicalDeviceExternalMemoryHostPropertiesEXT& copy_src);
    safe_VkPhysicalDeviceExternalMemoryHostPropertiesEXT();
    ~safe_VkPhysicalDeviceExternalMemoryHostPropertiesEXT();
    void initialize(const VkPhysicalDeviceExternalMemoryHostPropertiesEXT* in_struct);
    void initialize(const safe_VkPhysicalDeviceExternalMemoryHostPropertiesEXT* copy_src);
    VkPhysicalDeviceExternalMemoryHostPropertiesEXT *ptr() { return reinterpret_cast<VkPhysicalDeviceExternalMemoryHostPropertiesEXT *>(this); }
    VkPhysicalDeviceExternalMemoryHostPropertiesEXT const *ptr() const { return reinterpret_cast<VkPhysicalDeviceExternalMemoryHostPropertiesEXT const *>(this); }
};

struct safe_VkCalibratedTimestampInfoEXT {
    VkStructureType sType;
    const void* pNext;
    VkTimeDomainEXT timeDomain;
    safe_VkCalibratedTimestampInfoEXT(const VkCalibratedTimestampInfoEXT* in_struct);
    safe_VkCalibratedTimestampInfoEXT(const safe_VkCalibratedTimestampInfoEXT& copy_src);
    safe_VkCalibratedTimestampInfoEXT& operator=(const safe_VkCalibratedTimestampInfoEXT& copy_src);
    safe_VkCalibratedTimestampInfoEXT();
    ~safe_VkCalibratedTimestampInfoEXT();
    void initialize(const VkCalibratedTimestampInfoEXT* in_struct);
    void initialize(const safe_VkCalibratedTimestampInfoEXT* copy_src);
    VkCalibratedTimestampInfoEXT *ptr() { return reinterpret_cast<VkCalibratedTimestampInfoEXT *>(this); }
    VkCalibratedTimestampInfoEXT const *ptr() const { return reinterpret_cast<VkCalibratedTimestampInfoEXT const *>(this); }
};

struct safe_VkPhysicalDeviceVertexAttributeDivisorPropertiesEXT {
    VkStructureType sType;
    void* pNext;
    uint32_t maxVertexAttribDivisor;
    safe_VkPhysicalDeviceVertexAttributeDivisorPropertiesEXT(const VkPhysicalDeviceVertexAttributeDivisorPropertiesEXT* in_struct);
    safe_VkPhysicalDeviceVertexAttributeDivisorPropertiesEXT(const safe_VkPhysicalDeviceVertexAttributeDivisorPropertiesEXT& copy_src);
    safe_VkPhysicalDeviceVertexAttributeDivisorPropertiesEXT& operator=(const safe_VkPhysicalDeviceVertexAttributeDivisorPropertiesEXT& copy_src);
    safe_VkPhysicalDeviceVertexAttributeDivisorPropertiesEXT();
    ~safe_VkPhysicalDeviceVertexAttributeDivisorPropertiesEXT();
    void initialize(const VkPhysicalDeviceVertexAttributeDivisorPropertiesEXT* in_struct);
    void initialize(const safe_VkPhysicalDeviceVertexAttributeDivisorPropertiesEXT* copy_src);
    VkPhysicalDeviceVertexAttributeDivisorPropertiesEXT *ptr() { return reinterpret_cast<VkPhysicalDeviceVertexAttributeDivisorPropertiesEXT *>(this); }
    VkPhysicalDeviceVertexAttributeDivisorPropertiesEXT const *ptr() const { return reinterpret_cast<VkPhysicalDeviceVertexAttributeDivisorPropertiesEXT const *>(this); }
};

struct safe_VkPipelineVertexInputDivisorStateCreateInfoEXT {
    VkStructureType sType;
    const void* pNext;
    uint32_t vertexBindingDivisorCount;
    const VkVertexInputBindingDivisorDescriptionEXT* pVertexBindingDivisors;
    safe_VkPipelineVertexInputDivisorStateCreateInfoEXT(const VkPipelineVertexInputDivisorStateCreateInfoEXT* in_struct);
    safe_VkPipelineVertexInputDivisorStateCreateInfoEXT(const safe_VkPipelineVertexInputDivisorStateCreateInfoEXT& copy_src);
    safe_VkPipelineVertexInputDivisorStateCreateInfoEXT& operator=(const safe_VkPipelineVertexInputDivisorStateCreateInfoEXT& copy_src);
    safe_VkPipelineVertexInputDivisorStateCreateInfoEXT();
    ~safe_VkPipelineVertexInputDivisorStateCreateInfoEXT();
    void initialize(const VkPipelineVertexInputDivisorStateCreateInfoEXT* in_struct);
    void initialize(const safe_VkPipelineVertexInputDivisorStateCreateInfoEXT* copy_src);
    VkPipelineVertexInputDivisorStateCreateInfoEXT *ptr() { return reinterpret_cast<VkPipelineVertexInputDivisorStateCreateInfoEXT *>(this); }
    VkPipelineVertexInputDivisorStateCreateInfoEXT const *ptr() const { return reinterpret_cast<VkPipelineVertexInputDivisorStateCreateInfoEXT const *>(this); }
};

struct safe_VkPhysicalDeviceVertexAttributeDivisorFeaturesEXT {
    VkStructureType sType;
    void* pNext;
    VkBool32 vertexAttributeInstanceRateDivisor;
    VkBool32 vertexAttributeInstanceRateZeroDivisor;
    safe_VkPhysicalDeviceVertexAttributeDivisorFeaturesEXT(const VkPhysicalDeviceVertexAttributeDivisorFeaturesEXT* in_struct);
    safe_VkPhysicalDeviceVertexAttributeDivisorFeaturesEXT(const safe_VkPhysicalDeviceVertexAttributeDivisorFeaturesEXT& copy_src);
    safe_VkPhysicalDeviceVertexAttributeDivisorFeaturesEXT& operator=(const safe_VkPhysicalDeviceVertexAttributeDivisorFeaturesEXT& copy_src);
    safe_VkPhysicalDeviceVertexAttributeDivisorFeaturesEXT();
    ~safe_VkPhysicalDeviceVertexAttributeDivisorFeaturesEXT();
    void initialize(const VkPhysicalDeviceVertexAttributeDivisorFeaturesEXT* in_struct);
    void initialize(const safe_VkPhysicalDeviceVertexAttributeDivisorFeaturesEXT* copy_src);
    VkPhysicalDeviceVertexAttributeDivisorFeaturesEXT *ptr() { return reinterpret_cast<VkPhysicalDeviceVertexAttributeDivisorFeaturesEXT *>(this); }
    VkPhysicalDeviceVertexAttributeDivisorFeaturesEXT const *ptr() const { return reinterpret_cast<VkPhysicalDeviceVertexAttributeDivisorFeaturesEXT const *>(this); }
};

struct safe_VkPhysicalDevicePCIBusInfoPropertiesEXT {
    VkStructureType sType;
    void* pNext;
    uint32_t pciDomain;
    uint32_t pciBus;
    uint32_t pciDevice;
    uint32_t pciFunction;
    safe_VkPhysicalDevicePCIBusInfoPropertiesEXT(const VkPhysicalDevicePCIBusInfoPropertiesEXT* in_struct);
    safe_VkPhysicalDevicePCIBusInfoPropertiesEXT(const safe_VkPhysicalDevicePCIBusInfoPropertiesEXT& copy_src);
    safe_VkPhysicalDevicePCIBusInfoPropertiesEXT& operator=(const safe_VkPhysicalDevicePCIBusInfoPropertiesEXT& copy_src);
    safe_VkPhysicalDevicePCIBusInfoPropertiesEXT();
    ~safe_VkPhysicalDevicePCIBusInfoPropertiesEXT();
    void initialize(const VkPhysicalDevicePCIBusInfoPropertiesEXT* in_struct);
    void initialize(const safe_VkPhysicalDevicePCIBusInfoPropertiesEXT* copy_src);
    VkPhysicalDevicePCIBusInfoPropertiesEXT *ptr() { return reinterpret_cast<VkPhysicalDevicePCIBusInfoPropertiesEXT *>(this); }
    VkPhysicalDevicePCIBusInfoPropertiesEXT const *ptr() const { return reinterpret_cast<VkPhysicalDevicePCIBusInfoPropertiesEXT const *>(this); }
};

struct safe_VkPhysicalDeviceSubgroupSizeControlFeaturesEXT {
    VkStructureType sType;
    void* pNext;
    VkBool32 subgroupSizeControl;
    VkBool32 computeFullSubgroups;
    safe_VkPhysicalDeviceSubgroupSizeControlFeaturesEXT(const VkPhysicalDeviceSubgroupSizeControlFeaturesEXT* in_struct);
    safe_VkPhysicalDeviceSubgroupSizeControlFeaturesEXT(const safe_VkPhysicalDeviceSubgroupSizeControlFeaturesEXT& copy_src);
    safe_VkPhysicalDeviceSubgroupSizeControlFeaturesEXT& operator=(const safe_VkPhysicalDeviceSubgroupSizeControlFeaturesEXT& copy_src);
    safe_VkPhysicalDeviceSubgroupSizeControlFeaturesEXT();
    ~safe_VkPhysicalDeviceSubgroupSizeControlFeaturesEXT();
    void initialize(const VkPhysicalDeviceSubgroupSizeControlFeaturesEXT* in_struct);
    void initialize(const safe_VkPhysicalDeviceSubgroupSizeControlFeaturesEXT* copy_src);
    VkPhysicalDeviceSubgroupSizeControlFeaturesEXT *ptr() { return reinterpret_cast<VkPhysicalDeviceSubgroupSizeControlFeaturesEXT *>(this); }
    VkPhysicalDeviceSubgroupSizeControlFeaturesEXT const *ptr() const { return reinterpret_cast<VkPhysicalDeviceSubgroupSizeControlFeaturesEXT const *>(this); }
};

struct safe_VkPhysicalDeviceSubgroupSizeControlPropertiesEXT {
    VkStructureType sType;
    void* pNext;
    uint32_t minSubgroupSize;
    uint32_t maxSubgroupSize;
    uint32_t maxComputeWorkgroupSubgroups;
    VkShaderStageFlags requiredSubgroupSizeStages;
    safe_VkPhysicalDeviceSubgroupSizeControlPropertiesEXT(const VkPhysicalDeviceSubgroupSizeControlPropertiesEXT* in_struct);
    safe_VkPhysicalDeviceSubgroupSizeControlPropertiesEXT(const safe_VkPhysicalDeviceSubgroupSizeControlPropertiesEXT& copy_src);
    safe_VkPhysicalDeviceSubgroupSizeControlPropertiesEXT& operator=(const safe_VkPhysicalDeviceSubgroupSizeControlPropertiesEXT& copy_src);
    safe_VkPhysicalDeviceSubgroupSizeControlPropertiesEXT();
    ~safe_VkPhysicalDeviceSubgroupSizeControlPropertiesEXT();
    void initialize(const VkPhysicalDeviceSubgroupSizeControlPropertiesEXT* in_struct);
    void initialize(const safe_VkPhysicalDeviceSubgroupSizeControlPropertiesEXT* copy_src);
    VkPhysicalDeviceSubgroupSizeControlPropertiesEXT *ptr() { return reinterpret_cast<VkPhysicalDeviceSubgroupSizeControlPropertiesEXT *>(this); }
    VkPhysicalDeviceSubgroupSizeControlPropertiesEXT const *ptr() const { return reinterpret_cast<VkPhysicalDeviceSubgroupSizeControlPropertiesEXT const *>(this); }
};

struct safe_VkPipelineShaderStageRequiredSubgroupSizeCreateInfoEXT {
    VkStructureType sType;
    void* pNext;
    uint32_t requiredSubgroupSize;
    safe_VkPipelineShaderStageRequiredSubgroupSizeCreateInfoEXT(const VkPipelineShaderStageRequiredSubgroupSizeCreateInfoEXT* in_struct);
    safe_VkPipelineShaderStageRequiredSubgroupSizeCreateInfoEXT(const safe_VkPipelineShaderStageRequiredSubgroupSizeCreateInfoEXT& copy_src);
    safe_VkPipelineShaderStageRequiredSubgroupSizeCreateInfoEXT& operator=(const safe_VkPipelineShaderStageRequiredSubgroupSizeCreateInfoEXT& copy_src);
    safe_VkPipelineShaderStageRequiredSubgroupSizeCreateInfoEXT();
    ~safe_VkPipelineShaderStageRequiredSubgroupSizeCreateInfoEXT();
    void initialize(const VkPipelineShaderStageRequiredSubgroupSizeCreateInfoEXT* in_struct);
    void initialize(const safe_VkPipelineShaderStageRequiredSubgroupSizeCreateInfoEXT* copy_src);
    VkPipelineShaderStageRequiredSubgroupSizeCreateInfoEXT *ptr() { return reinterpret_cast<VkPipelineShaderStageRequiredSubgroupSizeCreateInfoEXT *>(this); }
    VkPipelineShaderStageRequiredSubgroupSizeCreateInfoEXT const *ptr() const { return reinterpret_cast<VkPipelineShaderStageRequiredSubgroupSizeCreateInfoEXT const *>(this); }
};

struct safe_VkPhysicalDeviceShaderImageAtomicInt64FeaturesEXT {
    VkStructureType sType;
    void* pNext;
    VkBool32 shaderImageInt64Atomics;
    VkBool32 sparseImageInt64Atomics;
    safe_VkPhysicalDeviceShaderImageAtomicInt64FeaturesEXT(const VkPhysicalDeviceShaderImageAtomicInt64FeaturesEXT* in_struct);
    safe_VkPhysicalDeviceShaderImageAtomicInt64FeaturesEXT(const safe_VkPhysicalDeviceShaderImageAtomicInt64FeaturesEXT& copy_src);
    safe_VkPhysicalDeviceShaderImageAtomicInt64FeaturesEXT& operator=(const safe_VkPhysicalDeviceShaderImageAtomicInt64FeaturesEXT& copy_src);
    safe_VkPhysicalDeviceShaderImageAtomicInt64FeaturesEXT();
    ~safe_VkPhysicalDeviceShaderImageAtomicInt64FeaturesEXT();
    void initialize(const VkPhysicalDeviceShaderImageAtomicInt64FeaturesEXT* in_struct);
    void initialize(const safe_VkPhysicalDeviceShaderImageAtomicInt64FeaturesEXT* copy_src);
    VkPhysicalDeviceShaderImageAtomicInt64FeaturesEXT *ptr() { return reinterpret_cast<VkPhysicalDeviceShaderImageAtomicInt64FeaturesEXT *>(this); }
    VkPhysicalDeviceShaderImageAtomicInt64FeaturesEXT const *ptr() const { return reinterpret_cast<VkPhysicalDeviceShaderImageAtomicInt64FeaturesEXT const *>(this); }
};

struct safe_VkPhysicalDeviceMemoryBudgetPropertiesEXT {
    VkStructureType sType;
    void* pNext;
    VkDeviceSize heapBudget[VK_MAX_MEMORY_HEAPS];
    VkDeviceSize heapUsage[VK_MAX_MEMORY_HEAPS];
    safe_VkPhysicalDeviceMemoryBudgetPropertiesEXT(const VkPhysicalDeviceMemoryBudgetPropertiesEXT* in_struct);
    safe_VkPhysicalDeviceMemoryBudgetPropertiesEXT(const safe_VkPhysicalDeviceMemoryBudgetPropertiesEXT& copy_src);
    safe_VkPhysicalDeviceMemoryBudgetPropertiesEXT& operator=(const safe_VkPhysicalDeviceMemoryBudgetPropertiesEXT& copy_src);
    safe_VkPhysicalDeviceMemoryBudgetPropertiesEXT();
    ~safe_VkPhysicalDeviceMemoryBudgetPropertiesEXT();
    void initialize(const VkPhysicalDeviceMemoryBudgetPropertiesEXT* in_struct);
    void initialize(const safe_VkPhysicalDeviceMemoryBudgetPropertiesEXT* copy_src);
    VkPhysicalDeviceMemoryBudgetPropertiesEXT *ptr() { return reinterpret_cast<VkPhysicalDeviceMemoryBudgetPropertiesEXT *>(this); }
    VkPhysicalDeviceMemoryBudgetPropertiesEXT const *ptr() const { return reinterpret_cast<VkPhysicalDeviceMemoryBudgetPropertiesEXT const *>(this); }
};

struct safe_VkValidationFeaturesEXT {
    VkStructureType sType;
    const void* pNext;
    uint32_t enabledValidationFeatureCount;
    const VkValidationFeatureEnableEXT* pEnabledValidationFeatures;
    uint32_t disabledValidationFeatureCount;
    const VkValidationFeatureDisableEXT* pDisabledValidationFeatures;
    safe_VkValidationFeaturesEXT(const VkValidationFeaturesEXT* in_struct);
    safe_VkValidationFeaturesEXT(const safe_VkValidationFeaturesEXT& copy_src);
    safe_VkValidationFeaturesEXT& operator=(const safe_VkValidationFeaturesEXT& copy_src);
    safe_VkValidationFeaturesEXT();
    ~safe_VkValidationFeaturesEXT();
    void initialize(const VkValidationFeaturesEXT* in_struct);
    void initialize(const safe_VkValidationFeaturesEXT* copy_src);
    VkValidationFeaturesEXT *ptr() { return reinterpret_cast<VkValidationFeaturesEXT *>(this); }
    VkValidationFeaturesEXT const *ptr() const { return reinterpret_cast<VkValidationFeaturesEXT const *>(this); }
};

struct safe_VkPhysicalDeviceFragmentShaderInterlockFeaturesEXT {
    VkStructureType sType;
    void* pNext;
    VkBool32 fragmentShaderSampleInterlock;
    VkBool32 fragmentShaderPixelInterlock;
    VkBool32 fragmentShaderShadingRateInterlock;
    safe_VkPhysicalDeviceFragmentShaderInterlockFeaturesEXT(const VkPhysicalDeviceFragmentShaderInterlockFeaturesEXT* in_struct);
    safe_VkPhysicalDeviceFragmentShaderInterlockFeaturesEXT(const safe_VkPhysicalDeviceFragmentShaderInterlockFeaturesEXT& copy_src);
    safe_VkPhysicalDeviceFragmentShaderInterlockFeaturesEXT& operator=(const safe_VkPhysicalDeviceFragmentShaderInterlockFeaturesEXT& copy_src);
    safe_VkPhysicalDeviceFragmentShaderInterlockFeaturesEXT();
    ~safe_VkPhysicalDeviceFragmentShaderInterlockFeaturesEXT();
    void initialize(const VkPhysicalDeviceFragmentShaderInterlockFeaturesEXT* in_struct);
    void initialize(const safe_VkPhysicalDeviceFragmentShaderInterlockFeaturesEXT* copy_src);
    VkPhysicalDeviceFragmentShaderInterlockFeaturesEXT *ptr() { return reinterpret_cast<VkPhysicalDeviceFragmentShaderInterlockFeaturesEXT *>(this); }
    VkPhysicalDeviceFragmentShaderInterlockFeaturesEXT const *ptr() const { return reinterpret_cast<VkPhysicalDeviceFragmentShaderInterlockFeaturesEXT const *>(this); }
};

struct safe_VkPhysicalDeviceYcbcrImageArraysFeaturesEXT {
    VkStructureType sType;
    void* pNext;
    VkBool32 ycbcrImageArrays;
    safe_VkPhysicalDeviceYcbcrImageArraysFeaturesEXT(const VkPhysicalDeviceYcbcrImageArraysFeaturesEXT* in_struct);
    safe_VkPhysicalDeviceYcbcrImageArraysFeaturesEXT(const safe_VkPhysicalDeviceYcbcrImageArraysFeaturesEXT& copy_src);
    safe_VkPhysicalDeviceYcbcrImageArraysFeaturesEXT& operator=(const safe_VkPhysicalDeviceYcbcrImageArraysFeaturesEXT& copy_src);
    safe_VkPhysicalDeviceYcbcrImageArraysFeaturesEXT();
    ~safe_VkPhysicalDeviceYcbcrImageArraysFeaturesEXT();
    void initialize(const VkPhysicalDeviceYcbcrImageArraysFeaturesEXT* in_struct);
    void initialize(const safe_VkPhysicalDeviceYcbcrImageArraysFeaturesEXT* copy_src);
    VkPhysicalDeviceYcbcrImageArraysFeaturesEXT *ptr() { return reinterpret_cast<VkPhysicalDeviceYcbcrImageArraysFeaturesEXT *>(this); }
    VkPhysicalDeviceYcbcrImageArraysFeaturesEXT const *ptr() const { return reinterpret_cast<VkPhysicalDeviceYcbcrImageArraysFeaturesEXT const *>(this); }
};

struct safe_VkHeadlessSurfaceCreateInfoEXT {
    VkStructureType sType;
    const void* pNext;
    VkHeadlessSurfaceCreateFlagsEXT flags;
    safe_VkHeadlessSurfaceCreateInfoEXT(const VkHeadlessSurfaceCreateInfoEXT* in_struct);
    safe_VkHeadlessSurfaceCreateInfoEXT(const safe_VkHeadlessSurfaceCreateInfoEXT& copy_src);
    safe_VkHeadlessSurfaceCreateInfoEXT& operator=(const safe_VkHeadlessSurfaceCreateInfoEXT& copy_src);
    safe_VkHeadlessSurfaceCreateInfoEXT();
    ~safe_VkHeadlessSurfaceCreateInfoEXT();
    void initialize(const VkHeadlessSurfaceCreateInfoEXT* in_struct);
    void initialize(const safe_VkHeadlessSurfaceCreateInfoEXT* copy_src);
    VkHeadlessSurfaceCreateInfoEXT *ptr() { return reinterpret_cast<VkHeadlessSurfaceCreateInfoEXT *>(this); }
    VkHeadlessSurfaceCreateInfoEXT const *ptr() const { return reinterpret_cast<VkHeadlessSurfaceCreateInfoEXT const *>(this); }
};

struct safe_VkPhysicalDeviceLineRasterizationFeaturesEXT {
    VkStructureType sType;
    void* pNext;
    VkBool32 rectangularLines;
    VkBool32 bresenhamLines;
    VkBool32 smoothLines;
    VkBool32 stippledRectangularLines;
    VkBool32 stippledBresenhamLines;
    VkBool32 stippledSmoothLines;
    safe_VkPhysicalDeviceLineRasterizationFeaturesEXT(const VkPhysicalDeviceLineRasterizationFeaturesEXT* in_struct);
    safe_VkPhysicalDeviceLineRasterizationFeaturesEXT(const safe_VkPhysicalDeviceLineRasterizationFeaturesEXT& copy_src);
    safe_VkPhysicalDeviceLineRasterizationFeaturesEXT& operator=(const safe_VkPhysicalDeviceLineRasterizationFeaturesEXT& copy_src);
    safe_VkPhysicalDeviceLineRasterizationFeaturesEXT();
    ~safe_VkPhysicalDeviceLineRasterizationFeaturesEXT();
    void initialize(const VkPhysicalDeviceLineRasterizationFeaturesEXT* in_struct);
    void initialize(const safe_VkPhysicalDeviceLineRasterizationFeaturesEXT* copy_src);
    VkPhysicalDeviceLineRasterizationFeaturesEXT *ptr() { return reinterpret_cast<VkPhysicalDeviceLineRasterizationFeaturesEXT *>(this); }
    VkPhysicalDeviceLineRasterizationFeaturesEXT const *ptr() const { return reinterpret_cast<VkPhysicalDeviceLineRasterizationFeaturesEXT const *>(this); }
};

struct safe_VkPhysicalDeviceLineRasterizationPropertiesEXT {
    VkStructureType sType;
    void* pNext;
    uint32_t lineSubPixelPrecisionBits;
    safe_VkPhysicalDeviceLineRasterizationPropertiesEXT(const VkPhysicalDeviceLineRasterizationPropertiesEXT* in_struct);
    safe_VkPhysicalDeviceLineRasterizationPropertiesEXT(const safe_VkPhysicalDeviceLineRasterizationPropertiesEXT& copy_src);
    safe_VkPhysicalDeviceLineRasterizationPropertiesEXT& operator=(const safe_VkPhysicalDeviceLineRasterizationPropertiesEXT& copy_src);
    safe_VkPhysicalDeviceLineRasterizationPropertiesEXT();
    ~safe_VkPhysicalDeviceLineRasterizationPropertiesEXT();
    void initialize(const VkPhysicalDeviceLineRasterizationPropertiesEXT* in_struct);
    void initialize(const safe_VkPhysicalDeviceLineRasterizationPropertiesEXT* copy_src);
    VkPhysicalDeviceLineRasterizationPropertiesEXT *ptr() { return reinterpret_cast<VkPhysicalDeviceLineRasterizationPropertiesEXT *>(this); }
    VkPhysicalDeviceLineRasterizationPropertiesEXT const *ptr() const { return reinterpret_cast<VkPhysicalDeviceLineRasterizationPropertiesEXT const *>(this); }
};

struct safe_VkPipelineRasterizationLineStateCreateInfoEXT {
    VkStructureType sType;
    const void* pNext;
    VkLineRasterizationModeEXT lineRasterizationMode;
    VkBool32 stippledLineEnable;
    uint32_t lineStippleFactor;
    uint16_t lineStipplePattern;
    safe_VkPipelineRasterizationLineStateCreateInfoEXT(const VkPipelineRasterizationLineStateCreateInfoEXT* in_struct);
    safe_VkPipelineRasterizationLineStateCreateInfoEXT(const safe_VkPipelineRasterizationLineStateCreateInfoEXT& copy_src);
    safe_VkPipelineRasterizationLineStateCreateInfoEXT& operator=(const safe_VkPipelineRasterizationLineStateCreateInfoEXT& copy_src);
    safe_VkPipelineRasterizationLineStateCreateInfoEXT();
    ~safe_VkPipelineRasterizationLineStateCreateInfoEXT();
    void initialize(const VkPipelineRasterizationLineStateCreateInfoEXT* in_struct);
    void initialize(const safe_VkPipelineRasterizationLineStateCreateInfoEXT* copy_src);
    VkPipelineRasterizationLineStateCreateInfoEXT *ptr() { return reinterpret_cast<VkPipelineRasterizationLineStateCreateInfoEXT *>(this); }
    VkPipelineRasterizationLineStateCreateInfoEXT const *ptr() const { return reinterpret_cast<VkPipelineRasterizationLineStateCreateInfoEXT const *>(this); }
};

struct safe_VkPhysicalDeviceShaderAtomicFloatFeaturesEXT {
    VkStructureType sType;
    void* pNext;
    VkBool32 shaderBufferFloat32Atomics;
    VkBool32 shaderBufferFloat32AtomicAdd;
    VkBool32 shaderBufferFloat64Atomics;
    VkBool32 shaderBufferFloat64AtomicAdd;
    VkBool32 shaderSharedFloat32Atomics;
    VkBool32 shaderSharedFloat32AtomicAdd;
    VkBool32 shaderSharedFloat64Atomics;
    VkBool32 shaderSharedFloat64AtomicAdd;
    VkBool32 shaderImageFloat32Atomics;
    VkBool32 shaderImageFloat32AtomicAdd;
    VkBool32 sparseImageFloat32Atomics;
    VkBool32 sparseImageFloat32AtomicAdd;
    safe_VkPhysicalDeviceShaderAtomicFloatFeaturesEXT(const VkPhysicalDeviceShaderAtomicFloatFeaturesEXT* in_struct);
    safe_VkPhysicalDeviceShaderAtomicFloatFeaturesEXT(const safe_VkPhysicalDeviceShaderAtomicFloatFeaturesEXT& copy_src);
    safe_VkPhysicalDeviceShaderAtomicFloatFeaturesEXT& operator=(const safe_VkPhysicalDeviceShaderAtomicFloatFeaturesEXT& copy_src);
    safe_VkPhysicalDeviceShaderAtomicFloatFeaturesEXT();
    ~safe_VkPhysicalDeviceShaderAtomicFloatFeaturesEXT();
    void initialize(const VkPhysicalDeviceShaderAtomicFloatFeaturesEXT* in_struct);
    void initialize(const safe_VkPhysicalDeviceShaderAtomicFloatFeaturesEXT* copy_src);
    VkPhysicalDeviceShaderAtomicFloatFeaturesEXT *ptr() { return reinterpret_cast<VkPhysicalDeviceShaderAtomicFloatFeaturesEXT *>(this); }
    VkPhysicalDeviceShaderAtomicFloatFeaturesEXT const *ptr() const { return reinterpret_cast<VkPhysicalDeviceShaderAtomicFloatFeaturesEXT const *>(this); }
};

struct safe_VkPhysicalDeviceIndexTypeUint8FeaturesEXT {
    VkStructureType sType;
    void* pNext;
    VkBool32 indexTypeUint8;
    safe_VkPhysicalDeviceIndexTypeUint8FeaturesEXT(const VkPhysicalDeviceIndexTypeUint8FeaturesEXT* in_struct);
    safe_VkPhysicalDeviceIndexTypeUint8FeaturesEXT(const safe_VkPhysicalDeviceIndexTypeUint8FeaturesEXT& copy_src);
    safe_VkPhysicalDeviceIndexTypeUint8FeaturesEXT& operator=(const safe_VkPhysicalDeviceIndexTypeUint8FeaturesEXT& copy_src);
    safe_VkPhysicalDeviceIndexTypeUint8FeaturesEXT();
    ~safe_VkPhysicalDeviceIndexTypeUint8FeaturesEXT();
    void initialize(const VkPhysicalDeviceIndexTypeUint8FeaturesEXT* in_struct);
    void initialize(const safe_VkPhysicalDeviceIndexTypeUint8FeaturesEXT* copy_src);
    VkPhysicalDeviceIndexTypeUint8FeaturesEXT *ptr() { return reinterpret_cast<VkPhysicalDeviceIndexTypeUint8FeaturesEXT *>(this); }
    VkPhysicalDeviceIndexTypeUint8FeaturesEXT const *ptr() const { return reinterpret_cast<VkPhysicalDeviceIndexTypeUint8FeaturesEXT const *>(this); }
};

struct safe_VkPhysicalDeviceExtendedDynamicStateFeaturesEXT {
    VkStructureType sType;
    void* pNext;
    VkBool32 extendedDynamicState;
    safe_VkPhysicalDeviceExtendedDynamicStateFeaturesEXT(const VkPhysicalDeviceExtendedDynamicStateFeaturesEXT* in_struct);
    safe_VkPhysicalDeviceExtendedDynamicStateFeaturesEXT(const safe_VkPhysicalDeviceExtendedDynamicStateFeaturesEXT& copy_src);
    safe_VkPhysicalDeviceExtendedDynamicStateFeaturesEXT& operator=(const safe_VkPhysicalDeviceExtendedDynamicStateFeaturesEXT& copy_src);
    safe_VkPhysicalDeviceExtendedDynamicStateFeaturesEXT();
    ~safe_VkPhysicalDeviceExtendedDynamicStateFeaturesEXT();
    void initialize(const VkPhysicalDeviceExtendedDynamicStateFeaturesEXT* in_struct);
    void initialize(const safe_VkPhysicalDeviceExtendedDynamicStateFeaturesEXT* copy_src);
    VkPhysicalDeviceExtendedDynamicStateFeaturesEXT *ptr() { return reinterpret_cast<VkPhysicalDeviceExtendedDynamicStateFeaturesEXT *>(this); }
    VkPhysicalDeviceExtendedDynamicStateFeaturesEXT const *ptr() const { return reinterpret_cast<VkPhysicalDeviceExtendedDynamicStateFeaturesEXT const *>(this); }
};

struct safe_VkPhysicalDeviceShaderDemoteToHelperInvocationFeaturesEXT {
    VkStructureType sType;
    void* pNext;
    VkBool32 shaderDemoteToHelperInvocation;
    safe_VkPhysicalDeviceShaderDemoteToHelperInvocationFeaturesEXT(const VkPhysicalDeviceShaderDemoteToHelperInvocationFeaturesEXT* in_struct);
    safe_VkPhysicalDeviceShaderDemoteToHelperInvocationFeaturesEXT(const safe_VkPhysicalDeviceShaderDemoteToHelperInvocationFeaturesEXT& copy_src);
    safe_VkPhysicalDeviceShaderDemoteToHelperInvocationFeaturesEXT& operator=(const safe_VkPhysicalDeviceShaderDemoteToHelperInvocationFeaturesEXT& copy_src);
    safe_VkPhysicalDeviceShaderDemoteToHelperInvocationFeaturesEXT();
    ~safe_VkPhysicalDeviceShaderDemoteToHelperInvocationFeaturesEXT();
    void initialize(const VkPhysicalDeviceShaderDemoteToHelperInvocationFeaturesEXT* in_struct);
    void initialize(const safe_VkPhysicalDeviceShaderDemoteToHelperInvocationFeaturesEXT* copy_src);
    VkPhysicalDeviceShaderDemoteToHelperInvocationFeaturesEXT *ptr() { return reinterpret_cast<VkPhysicalDeviceShaderDemoteToHelperInvocationFeaturesEXT *>(this); }
    VkPhysicalDeviceShaderDemoteToHelperInvocationFeaturesEXT const *ptr() const { return reinterpret_cast<VkPhysicalDeviceShaderDemoteToHelperInvocationFeaturesEXT const *>(this); }
};

struct safe_VkPhysicalDeviceTexelBufferAlignmentFeaturesEXT {
    VkStructureType sType;
    void* pNext;
    VkBool32 texelBufferAlignment;
    safe_VkPhysicalDeviceTexelBufferAlignmentFeaturesEXT(const VkPhysicalDeviceTexelBufferAlignmentFeaturesEXT* in_struct);
    safe_VkPhysicalDeviceTexelBufferAlignmentFeaturesEXT(const safe_VkPhysicalDeviceTexelBufferAlignmentFeaturesEXT& copy_src);
    safe_VkPhysicalDeviceTexelBufferAlignmentFeaturesEXT& operator=(const safe_VkPhysicalDeviceTexelBufferAlignmentFeaturesEXT& copy_src);
    safe_VkPhysicalDeviceTexelBufferAlignmentFeaturesEXT();
    ~safe_VkPhysicalDeviceTexelBufferAlignmentFeaturesEXT();
    void initialize(const VkPhysicalDeviceTexelBufferAlignmentFeaturesEXT* in_struct);
    void initialize(const safe_VkPhysicalDeviceTexelBufferAlignmentFeaturesEXT* copy_src);
    VkPhysicalDeviceTexelBufferAlignmentFeaturesEXT *ptr() { return reinterpret_cast<VkPhysicalDeviceTexelBufferAlignmentFeaturesEXT *>(this); }
    VkPhysicalDeviceTexelBufferAlignmentFeaturesEXT const *ptr() const { return reinterpret_cast<VkPhysicalDeviceTexelBufferAlignmentFeaturesEXT const *>(this); }
};

struct safe_VkPhysicalDeviceTexelBufferAlignmentPropertiesEXT {
    VkStructureType sType;
    void* pNext;
    VkDeviceSize storageTexelBufferOffsetAlignmentBytes;
    VkBool32 storageTexelBufferOffsetSingleTexelAlignment;
    VkDeviceSize uniformTexelBufferOffsetAlignmentBytes;
    VkBool32 uniformTexelBufferOffsetSingleTexelAlignment;
    safe_VkPhysicalDeviceTexelBufferAlignmentPropertiesEXT(const VkPhysicalDeviceTexelBufferAlignmentPropertiesEXT* in_struct);
    safe_VkPhysicalDeviceTexelBufferAlignmentPropertiesEXT(const safe_VkPhysicalDeviceTexelBufferAlignmentPropertiesEXT& copy_src);
    safe_VkPhysicalDeviceTexelBufferAlignmentPropertiesEXT& operator=(const safe_VkPhysicalDeviceTexelBufferAlignmentPropertiesEXT& copy_src);
    safe_VkPhysicalDeviceTexelBufferAlignmentPropertiesEXT();
    ~safe_VkPhysicalDeviceTexelBufferAlignmentPropertiesEXT();
    void initialize(const VkPhysicalDeviceTexelBufferAlignmentPropertiesEXT* in_struct);
    void initialize(const safe_VkPhysicalDeviceTexelBufferAlignmentPropertiesEXT* copy_src);
    VkPhysicalDeviceTexelBufferAlignmentPropertiesEXT *ptr() { return reinterpret_cast<VkPhysicalDeviceTexelBufferAlignmentPropertiesEXT *>(this); }
    VkPhysicalDeviceTexelBufferAlignmentPropertiesEXT const *ptr() const { return reinterpret_cast<VkPhysicalDeviceTexelBufferAlignmentPropertiesEXT const *>(this); }
};

struct safe_VkPhysicalDeviceRobustness2FeaturesEXT {
    VkStructureType sType;
    void* pNext;
    VkBool32 robustBufferAccess2;
    VkBool32 robustImageAccess2;
    VkBool32 nullDescriptor;
    safe_VkPhysicalDeviceRobustness2FeaturesEXT(const VkPhysicalDeviceRobustness2FeaturesEXT* in_struct);
    safe_VkPhysicalDeviceRobustness2FeaturesEXT(const safe_VkPhysicalDeviceRobustness2FeaturesEXT& copy_src);
    safe_VkPhysicalDeviceRobustness2FeaturesEXT& operator=(const safe_VkPhysicalDeviceRobustness2FeaturesEXT& copy_src);
    safe_VkPhysicalDeviceRobustness2FeaturesEXT();
    ~safe_VkPhysicalDeviceRobustness2FeaturesEXT();
    void initialize(const VkPhysicalDeviceRobustness2FeaturesEXT* in_struct);
    void initialize(const safe_VkPhysicalDeviceRobustness2FeaturesEXT* copy_src);
    VkPhysicalDeviceRobustness2FeaturesEXT *ptr() { return reinterpret_cast<VkPhysicalDeviceRobustness2FeaturesEXT *>(this); }
    VkPhysicalDeviceRobustness2FeaturesEXT const *ptr() const { return reinterpret_cast<VkPhysicalDeviceRobustness2FeaturesEXT const *>(this); }
};

struct safe_VkPhysicalDeviceRobustness2PropertiesEXT {
    VkStructureType sType;
    void* pNext;
    VkDeviceSize robustStorageBufferAccessSizeAlignment;
    VkDeviceSize robustUniformBufferAccessSizeAlignment;
    safe_VkPhysicalDeviceRobustness2PropertiesEXT(const VkPhysicalDeviceRobustness2PropertiesEXT* in_struct);
    safe_VkPhysicalDeviceRobustness2PropertiesEXT(const safe_VkPhysicalDeviceRobustness2PropertiesEXT& copy_src);
    safe_VkPhysicalDeviceRobustness2PropertiesEXT& operator=(const safe_VkPhysicalDeviceRobustness2PropertiesEXT& copy_src);
    safe_VkPhysicalDeviceRobustness2PropertiesEXT();
    ~safe_VkPhysicalDeviceRobustness2PropertiesEXT();
    void initialize(const VkPhysicalDeviceRobustness2PropertiesEXT* in_struct);
    void initialize(const safe_VkPhysicalDeviceRobustness2PropertiesEXT* copy_src);
    VkPhysicalDeviceRobustness2PropertiesEXT *ptr() { return reinterpret_cast<VkPhysicalDeviceRobustness2PropertiesEXT *>(this); }
    VkPhysicalDeviceRobustness2PropertiesEXT const *ptr() const { return reinterpret_cast<VkPhysicalDeviceRobustness2PropertiesEXT const *>(this); }
};

struct safe_VkSamplerCustomBorderColorCreateInfoEXT {
    VkStructureType sType;
    const void* pNext;
    VkClearColorValue customBorderColor;
    VkFormat format;
    safe_VkSamplerCustomBorderColorCreateInfoEXT(const VkSamplerCustomBorderColorCreateInfoEXT* in_struct);
    safe_VkSamplerCustomBorderColorCreateInfoEXT(const safe_VkSamplerCustomBorderColorCreateInfoEXT& copy_src);
    safe_VkSamplerCustomBorderColorCreateInfoEXT& operator=(const safe_VkSamplerCustomBorderColorCreateInfoEXT& copy_src);
    safe_VkSamplerCustomBorderColorCreateInfoEXT();
    ~safe_VkSamplerCustomBorderColorCreateInfoEXT();
    void initialize(const VkSamplerCustomBorderColorCreateInfoEXT* in_struct);
    void initialize(const safe_VkSamplerCustomBorderColorCreateInfoEXT* copy_src);
    VkSamplerCustomBorderColorCreateInfoEXT *ptr() { return reinterpret_cast<VkSamplerCustomBorderColorCreateInfoEXT *>(this); }
    VkSamplerCustomBorderColorCreateInfoEXT const *ptr() const { return reinterpret_cast<VkSamplerCustomBorderColorCreateInfoEXT const *>(this); }
};

struct safe_VkPhysicalDeviceCustomBorderColorPropertiesEXT {
    VkStructureType sType;
    void* pNext;
    uint32_t maxCustomBorderColorSamplers;
    safe_VkPhysicalDeviceCustomBorderColorPropertiesEXT(const VkPhysicalDeviceCustomBorderColorPropertiesEXT* in_struct);
    safe_VkPhysicalDeviceCustomBorderColorPropertiesEXT(const safe_VkPhysicalDeviceCustomBorderColorPropertiesEXT& copy_src);
    safe_VkPhysicalDeviceCustomBorderColorPropertiesEXT& operator=(const safe_VkPhysicalDeviceCustomBorderColorPropertiesEXT& copy_src);
    safe_VkPhysicalDeviceCustomBorderColorPropertiesEXT();
    ~safe_VkPhysicalDeviceCustomBorderColorPropertiesEXT();
    void initialize(const VkPhysicalDeviceCustomBorderColorPropertiesEXT* in_struct);
    void initialize(const safe_VkPhysicalDeviceCustomBorderColorPropertiesEXT* copy_src);
    VkPhysicalDeviceCustomBorderColorPropertiesEXT *ptr() { return reinterpret_cast<VkPhysicalDeviceCustomBorderColorPropertiesEXT *>(this); }
    VkPhysicalDeviceCustomBorderColorPropertiesEXT const *ptr() const { return reinterpret_cast<VkPhysicalDeviceCustomBorderColorPropertiesEXT const *>(this); }
};

struct safe_VkPhysicalDeviceCustomBorderColorFeaturesEXT {
    VkStructureType sType;
    void* pNext;
    VkBool32 customBorderColors;
    VkBool32 customBorderColorWithoutFormat;
    safe_VkPhysicalDeviceCustomBorderColorFeaturesEXT(const VkPhysicalDeviceCustomBorderColorFeaturesEXT* in_struct);
    safe_VkPhysicalDeviceCustomBorderColorFeaturesEXT(const safe_VkPhysicalDeviceCustomBorderColorFeaturesEXT& copy_src);
    safe_VkPhysicalDeviceCustomBorderColorFeaturesEXT& operator=(const safe_VkPhysicalDeviceCustomBorderColorFeaturesEXT& copy_src);
    safe_VkPhysicalDeviceCustomBorderColorFeaturesEXT();
    ~safe_VkPhysicalDeviceCustomBorderColorFeaturesEXT();
    void initialize(const VkPhysicalDeviceCustomBorderColorFeaturesEXT* in_struct);
    void initialize(const safe_VkPhysicalDeviceCustomBorderColorFeaturesEXT* copy_src);
    VkPhysicalDeviceCustomBorderColorFeaturesEXT *ptr() { return reinterpret_cast<VkPhysicalDeviceCustomBorderColorFeaturesEXT *>(this); }
    VkPhysicalDeviceCustomBorderColorFeaturesEXT const *ptr() const { return reinterpret_cast<VkPhysicalDeviceCustomBorderColorFeaturesEXT const *>(this); }
};

struct safe_VkPhysicalDeviceYcbcr2Plane444FormatsFeaturesEXT {
    VkStructureType sType;
    void* pNext;
    VkBool32 ycbcr2plane444Formats;
    safe_VkPhysicalDeviceYcbcr2Plane444FormatsFeaturesEXT(const VkPhysicalDeviceYcbcr2Plane444FormatsFeaturesEXT* in_struct);
    safe_VkPhysicalDeviceYcbcr2Plane444FormatsFeaturesEXT(const safe_VkPhysicalDeviceYcbcr2Plane444FormatsFeaturesEXT& copy_src);
    safe_VkPhysicalDeviceYcbcr2Plane444FormatsFeaturesEXT& operator=(const safe_VkPhysicalDeviceYcbcr2Plane444FormatsFeaturesEXT& copy_src);
    safe_VkPhysicalDeviceYcbcr2Plane444FormatsFeaturesEXT();
    ~safe_VkPhysicalDeviceYcbcr2Plane444FormatsFeaturesEXT();
    void initialize(const VkPhysicalDeviceYcbcr2Plane444FormatsFeaturesEXT* in_struct);
    void initialize(const safe_VkPhysicalDeviceYcbcr2Plane444FormatsFeaturesEXT* copy_src);
    VkPhysicalDeviceYcbcr2Plane444FormatsFeaturesEXT *ptr() { return reinterpret_cast<VkPhysicalDeviceYcbcr2Plane444FormatsFeaturesEXT *>(this); }
    VkPhysicalDeviceYcbcr2Plane444FormatsFeaturesEXT const *ptr() const { return reinterpret_cast<VkPhysicalDeviceYcbcr2Plane444FormatsFeaturesEXT const *>(this); }
};

struct safe_VkPhysicalDeviceImageRobustnessFeaturesEXT {
    VkStructureType sType;
    void* pNext;
    VkBool32 robustImageAccess;
    safe_VkPhysicalDeviceImageRobustnessFeaturesEXT(const VkPhysicalDeviceImageRobustnessFeaturesEXT* in_struct);
    safe_VkPhysicalDeviceImageRobustnessFeaturesEXT(const safe_VkPhysicalDeviceImageRobustnessFeaturesEXT& copy_src);
    safe_VkPhysicalDeviceImageRobustnessFeaturesEXT& operator=(const safe_VkPhysicalDeviceImageRobustnessFeaturesEXT& copy_src);
    safe_VkPhysicalDeviceImageRobustnessFeaturesEXT();
    ~safe_VkPhysicalDeviceImageRobustnessFeaturesEXT();
    void initialize(const VkPhysicalDeviceImageRobustnessFeaturesEXT* in_struct);
    void initialize(const safe_VkPhysicalDeviceImageRobustnessFeaturesEXT* copy_src);
    VkPhysicalDeviceImageRobustnessFeaturesEXT *ptr() { return reinterpret_cast<VkPhysicalDeviceImageRobustnessFeaturesEXT *>(this); }
    VkPhysicalDeviceImageRobustnessFeaturesEXT const *ptr() const { return reinterpret_cast<VkPhysicalDeviceImageRobustnessFeaturesEXT const *>(this); }
};

struct safe_VkPhysicalDevice4444FormatsFeaturesEXT {
    VkStructureType sType;
    void* pNext;
    VkBool32 formatA4R4G4B4;
    VkBool32 formatA4B4G4R4;
    safe_VkPhysicalDevice4444FormatsFeaturesEXT(const VkPhysicalDevice4444FormatsFeaturesEXT* in_struct);
    safe_VkPhysicalDevice4444FormatsFeaturesEXT(const safe_VkPhysicalDevice4444FormatsFeaturesEXT& copy_src);
    safe_VkPhysicalDevice4444FormatsFeaturesEXT& operator=(const safe_VkPhysicalDevice4444FormatsFeaturesEXT& copy_src);
    safe_VkPhysicalDevice4444FormatsFeaturesEXT();
    ~safe_VkPhysicalDevice4444FormatsFeaturesEXT();
    void initialize(const VkPhysicalDevice4444FormatsFeaturesEXT* in_struct);
    void initialize(const safe_VkPhysicalDevice4444FormatsFeaturesEXT* copy_src);
    VkPhysicalDevice4444FormatsFeaturesEXT *ptr() { return reinterpret_cast<VkPhysicalDevice4444FormatsFeaturesEXT *>(this); }
    VkPhysicalDevice4444FormatsFeaturesEXT const *ptr() const { return reinterpret_cast<VkPhysicalDevice4444FormatsFeaturesEXT const *>(this); }
};

struct safe_VkPhysicalDeviceVertexInputDynamicStateFeaturesEXT {
    VkStructureType sType;
    void* pNext;
    VkBool32 vertexInputDynamicState;
    safe_VkPhysicalDeviceVertexInputDynamicStateFeaturesEXT(const VkPhysicalDeviceVertexInputDynamicStateFeaturesEXT* in_struct);
    safe_VkPhysicalDeviceVertexInputDynamicStateFeaturesEXT(const safe_VkPhysicalDeviceVertexInputDynamicStateFeaturesEXT& copy_src);
    safe_VkPhysicalDeviceVertexInputDynamicStateFeaturesEXT& operator=(const safe_VkPhysicalDeviceVertexInputDynamicStateFeaturesEXT& copy_src);
    safe_VkPhysicalDeviceVertexInputDynamicStateFeaturesEXT();
    ~safe_VkPhysicalDeviceVertexInputDynamicStateFeaturesEXT();
    void initialize(const VkPhysicalDeviceVertexInputDynamicStateFeaturesEXT* in_struct);
    void initialize(const safe_VkPhysicalDeviceVertexInputDynamicStateFeaturesEXT* copy_src);
    VkPhysicalDeviceVertexInputDynamicStateFeaturesEXT *ptr() { return reinterpret_cast<VkPhysicalDeviceVertexInputDynamicStateFeaturesEXT *>(this); }
    VkPhysicalDeviceVertexInputDynamicStateFeaturesEXT const *ptr() const { return reinterpret_cast<VkPhysicalDeviceVertexInputDynamicStateFeaturesEXT const *>(this); }
};

struct safe_VkVertexInputBindingDescription2EXT {
    VkStructureType sType;
    void* pNext;
    uint32_t binding;
    uint32_t stride;
    VkVertexInputRate inputRate;
    uint32_t divisor;
    safe_VkVertexInputBindingDescription2EXT(const VkVertexInputBindingDescription2EXT* in_struct);
    safe_VkVertexInputBindingDescription2EXT(const safe_VkVertexInputBindingDescription2EXT& copy_src);
    safe_VkVertexInputBindingDescription2EXT& operator=(const safe_VkVertexInputBindingDescription2EXT& copy_src);
    safe_VkVertexInputBindingDescription2EXT();
    ~safe_VkVertexInputBindingDescription2EXT();
    void initialize(const VkVertexInputBindingDescription2EXT* in_struct);
    void initialize(const safe_VkVertexInputBindingDescription2EXT* copy_src);
    VkVertexInputBindingDescription2EXT *ptr() { return reinterpret_cast<VkVertexInputBindingDescription2EXT *>(this); }
    VkVertexInputBindingDescription2EXT const *ptr() const { return reinterpret_cast<VkVertexInputBindingDescription2EXT const *>(this); }
};

struct safe_VkVertexInputAttributeDescription2EXT {
    VkStructureType sType;
    void* pNext;
    uint32_t location;
    uint32_t binding;
    VkFormat format;
    uint32_t offset;
    safe_VkVertexInputAttributeDescription2EXT(const VkVertexInputAttributeDescription2EXT* in_struct);
    safe_VkVertexInputAttributeDescription2EXT(const safe_VkVertexInputAttributeDescription2EXT& copy_src);
    safe_VkVertexInputAttributeDescription2EXT& operator=(const safe_VkVertexInputAttributeDescription2EXT& copy_src);
    safe_VkVertexInputAttributeDescription2EXT();
    ~safe_VkVertexInputAttributeDescription2EXT();
    void initialize(const VkVertexInputAttributeDescription2EXT* in_struct);
    void initialize(const safe_VkVertexInputAttributeDescription2EXT* copy_src);
    VkVertexInputAttributeDescription2EXT *ptr() { return reinterpret_cast<VkVertexInputAttributeDescription2EXT *>(this); }
    VkVertexInputAttributeDescription2EXT const *ptr() const { return reinterpret_cast<VkVertexInputAttributeDescription2EXT const *>(this); }
};

struct safe_VkPhysicalDeviceExtendedDynamicState2FeaturesEXT {
    VkStructureType sType;
    void* pNext;
    VkBool32 extendedDynamicState2;
    VkBool32 extendedDynamicState2LogicOp;
    VkBool32 extendedDynamicState2PatchControlPoints;
    safe_VkPhysicalDeviceExtendedDynamicState2FeaturesEXT(const VkPhysicalDeviceExtendedDynamicState2FeaturesEXT* in_struct);
    safe_VkPhysicalDeviceExtendedDynamicState2FeaturesEXT(const safe_VkPhysicalDeviceExtendedDynamicState2FeaturesEXT& copy_src);
    safe_VkPhysicalDeviceExtendedDynamicState2FeaturesEXT& operator=(const safe_VkPhysicalDeviceExtendedDynamicState2FeaturesEXT& copy_src);
    safe_VkPhysicalDeviceExtendedDynamicState2FeaturesEXT();
    ~safe_VkPhysicalDeviceExtendedDynamicState2FeaturesEXT();
    void initialize(const VkPhysicalDeviceExtendedDynamicState2FeaturesEXT* in_struct);
    void initialize(const safe_VkPhysicalDeviceExtendedDynamicState2FeaturesEXT* copy_src);
    VkPhysicalDeviceExtendedDynamicState2FeaturesEXT *ptr() { return reinterpret_cast<VkPhysicalDeviceExtendedDynamicState2FeaturesEXT *>(this); }
    VkPhysicalDeviceExtendedDynamicState2FeaturesEXT const *ptr() const { return reinterpret_cast<VkPhysicalDeviceExtendedDynamicState2FeaturesEXT const *>(this); }
};

struct safe_VkPhysicalDeviceColorWriteEnableFeaturesEXT {
    VkStructureType sType;
    void* pNext;
    VkBool32 colorWriteEnable;
    safe_VkPhysicalDeviceColorWriteEnableFeaturesEXT(const VkPhysicalDeviceColorWriteEnableFeaturesEXT* in_struct);
    safe_VkPhysicalDeviceColorWriteEnableFeaturesEXT(const safe_VkPhysicalDeviceColorWriteEnableFeaturesEXT& copy_src);
    safe_VkPhysicalDeviceColorWriteEnableFeaturesEXT& operator=(const safe_VkPhysicalDeviceColorWriteEnableFeaturesEXT& copy_src);
    safe_VkPhysicalDeviceColorWriteEnableFeaturesEXT();
    ~safe_VkPhysicalDeviceColorWriteEnableFeaturesEXT();
    void initialize(const VkPhysicalDeviceColorWriteEnableFeaturesEXT* in_struct);
    void initialize(const safe_VkPhysicalDeviceColorWriteEnableFeaturesEXT* copy_src);
    VkPhysicalDeviceColorWriteEnableFeaturesEXT *ptr() { return reinterpret_cast<VkPhysicalDeviceColorWriteEnableFeaturesEXT *>(this); }
    VkPhysicalDeviceColorWriteEnableFeaturesEXT const *ptr() const { return reinterpret_cast<VkPhysicalDeviceColorWriteEnableFeaturesEXT const *>(this); }
};

struct safe_VkPipelineColorWriteCreateInfoEXT {
    VkStructureType sType;
    const void* pNext;
    uint32_t attachmentCount;
    const VkBool32* pColorWriteEnables;
    safe_VkPipelineColorWriteCreateInfoEXT(const VkPipelineColorWriteCreateInfoEXT* in_struct);
    safe_VkPipelineColorWriteCreateInfoEXT(const safe_VkPipelineColorWriteCreateInfoEXT& copy_src);
    safe_VkPipelineColorWriteCreateInfoEXT& operator=(const safe_VkPipelineColorWriteCreateInfoEXT& copy_src);
    safe_VkPipelineColorWriteCreateInfoEXT();
    ~safe_VkPipelineColorWriteCreateInfoEXT();
    void initialize(const VkPipelineColorWriteCreateInfoEXT* in_struct);
    void initialize(const safe_VkPipelineColorWriteCreateInfoEXT* copy_src);
    VkPipelineColorWriteCreateInfoEXT *ptr() { return reinterpret_cast<VkPipelineColorWriteCreateInfoEXT *>(this); }
    VkPipelineColorWriteCreateInfoEXT const *ptr() const { return reinterpret_cast<VkPipelineColorWriteCreateInfoEXT const *>(this); }
};
