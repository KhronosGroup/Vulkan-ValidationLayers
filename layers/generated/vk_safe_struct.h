// *** THIS FILE IS GENERATED - DO NOT EDIT ***
// See helper_file_generator.py for modifications


/***************************************************************************
 *
 * Copyright (c) 2015-2019 The Khronos Group Inc.
 * Copyright (c) 2015-2019 Valve Corporation
 * Copyright (c) 2015-2019 LunarG, Inc.
 * Copyright (c) 2015-2019 Google Inc.
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
#include <vulkan/vulkan.h>

void *SafePnextCopy(const void *pNext);
void FreePnextChain(const void *pNext);
char *SafeStringCopy(const char *in_string);


struct safe_VkApplicationInfo {
    VkStructureType sType;
    const void* pNext;
    const char* pApplicationName;
    uint32_t applicationVersion;
    const char* pEngineName;
    uint32_t engineVersion;
    uint32_t apiVersion;
    safe_VkApplicationInfo(const VkApplicationInfo* in_struct);
    safe_VkApplicationInfo(const safe_VkApplicationInfo& src);
    safe_VkApplicationInfo& operator=(const safe_VkApplicationInfo& src);
    safe_VkApplicationInfo();
    ~safe_VkApplicationInfo();
    void initialize(const VkApplicationInfo* in_struct);
    void initialize(const safe_VkApplicationInfo* src);
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
    safe_VkInstanceCreateInfo(const safe_VkInstanceCreateInfo& src);
    safe_VkInstanceCreateInfo& operator=(const safe_VkInstanceCreateInfo& src);
    safe_VkInstanceCreateInfo();
    ~safe_VkInstanceCreateInfo();
    void initialize(const VkInstanceCreateInfo* in_struct);
    void initialize(const safe_VkInstanceCreateInfo* src);
    VkInstanceCreateInfo *ptr() { return reinterpret_cast<VkInstanceCreateInfo *>(this); }
    VkInstanceCreateInfo const *ptr() const { return reinterpret_cast<VkInstanceCreateInfo const *>(this); }
};

struct safe_VkAllocationCallbacks {
    void* pUserData;
    PFN_vkAllocationFunction pfnAllocation;
    PFN_vkReallocationFunction pfnReallocation;
    PFN_vkFreeFunction pfnFree;
    PFN_vkInternalAllocationNotification pfnInternalAllocation;
    PFN_vkInternalFreeNotification pfnInternalFree;
    safe_VkAllocationCallbacks(const VkAllocationCallbacks* in_struct);
    safe_VkAllocationCallbacks(const safe_VkAllocationCallbacks& src);
    safe_VkAllocationCallbacks& operator=(const safe_VkAllocationCallbacks& src);
    safe_VkAllocationCallbacks();
    ~safe_VkAllocationCallbacks();
    void initialize(const VkAllocationCallbacks* in_struct);
    void initialize(const safe_VkAllocationCallbacks* src);
    VkAllocationCallbacks *ptr() { return reinterpret_cast<VkAllocationCallbacks *>(this); }
    VkAllocationCallbacks const *ptr() const { return reinterpret_cast<VkAllocationCallbacks const *>(this); }
};

struct safe_VkDeviceQueueCreateInfo {
    VkStructureType sType;
    const void* pNext;
    VkDeviceQueueCreateFlags flags;
    uint32_t queueFamilyIndex;
    uint32_t queueCount;
    const float* pQueuePriorities;
    safe_VkDeviceQueueCreateInfo(const VkDeviceQueueCreateInfo* in_struct);
    safe_VkDeviceQueueCreateInfo(const safe_VkDeviceQueueCreateInfo& src);
    safe_VkDeviceQueueCreateInfo& operator=(const safe_VkDeviceQueueCreateInfo& src);
    safe_VkDeviceQueueCreateInfo();
    ~safe_VkDeviceQueueCreateInfo();
    void initialize(const VkDeviceQueueCreateInfo* in_struct);
    void initialize(const safe_VkDeviceQueueCreateInfo* src);
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
    safe_VkDeviceCreateInfo(const safe_VkDeviceCreateInfo& src);
    safe_VkDeviceCreateInfo& operator=(const safe_VkDeviceCreateInfo& src);
    safe_VkDeviceCreateInfo();
    ~safe_VkDeviceCreateInfo();
    void initialize(const VkDeviceCreateInfo* in_struct);
    void initialize(const safe_VkDeviceCreateInfo* src);
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
    safe_VkSubmitInfo(const safe_VkSubmitInfo& src);
    safe_VkSubmitInfo& operator=(const safe_VkSubmitInfo& src);
    safe_VkSubmitInfo();
    ~safe_VkSubmitInfo();
    void initialize(const VkSubmitInfo* in_struct);
    void initialize(const safe_VkSubmitInfo* src);
    VkSubmitInfo *ptr() { return reinterpret_cast<VkSubmitInfo *>(this); }
    VkSubmitInfo const *ptr() const { return reinterpret_cast<VkSubmitInfo const *>(this); }
};

struct safe_VkMemoryAllocateInfo {
    VkStructureType sType;
    const void* pNext;
    VkDeviceSize allocationSize;
    uint32_t memoryTypeIndex;
    safe_VkMemoryAllocateInfo(const VkMemoryAllocateInfo* in_struct);
    safe_VkMemoryAllocateInfo(const safe_VkMemoryAllocateInfo& src);
    safe_VkMemoryAllocateInfo& operator=(const safe_VkMemoryAllocateInfo& src);
    safe_VkMemoryAllocateInfo();
    ~safe_VkMemoryAllocateInfo();
    void initialize(const VkMemoryAllocateInfo* in_struct);
    void initialize(const safe_VkMemoryAllocateInfo* src);
    VkMemoryAllocateInfo *ptr() { return reinterpret_cast<VkMemoryAllocateInfo *>(this); }
    VkMemoryAllocateInfo const *ptr() const { return reinterpret_cast<VkMemoryAllocateInfo const *>(this); }
};

struct safe_VkMappedMemoryRange {
    VkStructureType sType;
    const void* pNext;
    VkDeviceMemory memory;
    VkDeviceSize offset;
    VkDeviceSize size;
    safe_VkMappedMemoryRange(const VkMappedMemoryRange* in_struct);
    safe_VkMappedMemoryRange(const safe_VkMappedMemoryRange& src);
    safe_VkMappedMemoryRange& operator=(const safe_VkMappedMemoryRange& src);
    safe_VkMappedMemoryRange();
    ~safe_VkMappedMemoryRange();
    void initialize(const VkMappedMemoryRange* in_struct);
    void initialize(const safe_VkMappedMemoryRange* src);
    VkMappedMemoryRange *ptr() { return reinterpret_cast<VkMappedMemoryRange *>(this); }
    VkMappedMemoryRange const *ptr() const { return reinterpret_cast<VkMappedMemoryRange const *>(this); }
};

struct safe_VkSparseBufferMemoryBindInfo {
    VkBuffer buffer;
    uint32_t bindCount;
    VkSparseMemoryBind* pBinds;
    safe_VkSparseBufferMemoryBindInfo(const VkSparseBufferMemoryBindInfo* in_struct);
    safe_VkSparseBufferMemoryBindInfo(const safe_VkSparseBufferMemoryBindInfo& src);
    safe_VkSparseBufferMemoryBindInfo& operator=(const safe_VkSparseBufferMemoryBindInfo& src);
    safe_VkSparseBufferMemoryBindInfo();
    ~safe_VkSparseBufferMemoryBindInfo();
    void initialize(const VkSparseBufferMemoryBindInfo* in_struct);
    void initialize(const safe_VkSparseBufferMemoryBindInfo* src);
    VkSparseBufferMemoryBindInfo *ptr() { return reinterpret_cast<VkSparseBufferMemoryBindInfo *>(this); }
    VkSparseBufferMemoryBindInfo const *ptr() const { return reinterpret_cast<VkSparseBufferMemoryBindInfo const *>(this); }
};

struct safe_VkSparseImageOpaqueMemoryBindInfo {
    VkImage image;
    uint32_t bindCount;
    VkSparseMemoryBind* pBinds;
    safe_VkSparseImageOpaqueMemoryBindInfo(const VkSparseImageOpaqueMemoryBindInfo* in_struct);
    safe_VkSparseImageOpaqueMemoryBindInfo(const safe_VkSparseImageOpaqueMemoryBindInfo& src);
    safe_VkSparseImageOpaqueMemoryBindInfo& operator=(const safe_VkSparseImageOpaqueMemoryBindInfo& src);
    safe_VkSparseImageOpaqueMemoryBindInfo();
    ~safe_VkSparseImageOpaqueMemoryBindInfo();
    void initialize(const VkSparseImageOpaqueMemoryBindInfo* in_struct);
    void initialize(const safe_VkSparseImageOpaqueMemoryBindInfo* src);
    VkSparseImageOpaqueMemoryBindInfo *ptr() { return reinterpret_cast<VkSparseImageOpaqueMemoryBindInfo *>(this); }
    VkSparseImageOpaqueMemoryBindInfo const *ptr() const { return reinterpret_cast<VkSparseImageOpaqueMemoryBindInfo const *>(this); }
};

struct safe_VkSparseImageMemoryBindInfo {
    VkImage image;
    uint32_t bindCount;
    VkSparseImageMemoryBind* pBinds;
    safe_VkSparseImageMemoryBindInfo(const VkSparseImageMemoryBindInfo* in_struct);
    safe_VkSparseImageMemoryBindInfo(const safe_VkSparseImageMemoryBindInfo& src);
    safe_VkSparseImageMemoryBindInfo& operator=(const safe_VkSparseImageMemoryBindInfo& src);
    safe_VkSparseImageMemoryBindInfo();
    ~safe_VkSparseImageMemoryBindInfo();
    void initialize(const VkSparseImageMemoryBindInfo* in_struct);
    void initialize(const safe_VkSparseImageMemoryBindInfo* src);
    VkSparseImageMemoryBindInfo *ptr() { return reinterpret_cast<VkSparseImageMemoryBindInfo *>(this); }
    VkSparseImageMemoryBindInfo const *ptr() const { return reinterpret_cast<VkSparseImageMemoryBindInfo const *>(this); }
};

struct safe_VkBindSparseInfo {
    VkStructureType sType;
    const void* pNext;
    uint32_t waitSemaphoreCount;
    VkSemaphore* pWaitSemaphores;
    uint32_t bufferBindCount;
    safe_VkSparseBufferMemoryBindInfo* pBufferBinds;
    uint32_t imageOpaqueBindCount;
    safe_VkSparseImageOpaqueMemoryBindInfo* pImageOpaqueBinds;
    uint32_t imageBindCount;
    safe_VkSparseImageMemoryBindInfo* pImageBinds;
    uint32_t signalSemaphoreCount;
    VkSemaphore* pSignalSemaphores;
    safe_VkBindSparseInfo(const VkBindSparseInfo* in_struct);
    safe_VkBindSparseInfo(const safe_VkBindSparseInfo& src);
    safe_VkBindSparseInfo& operator=(const safe_VkBindSparseInfo& src);
    safe_VkBindSparseInfo();
    ~safe_VkBindSparseInfo();
    void initialize(const VkBindSparseInfo* in_struct);
    void initialize(const safe_VkBindSparseInfo* src);
    VkBindSparseInfo *ptr() { return reinterpret_cast<VkBindSparseInfo *>(this); }
    VkBindSparseInfo const *ptr() const { return reinterpret_cast<VkBindSparseInfo const *>(this); }
};

struct safe_VkFenceCreateInfo {
    VkStructureType sType;
    const void* pNext;
    VkFenceCreateFlags flags;
    safe_VkFenceCreateInfo(const VkFenceCreateInfo* in_struct);
    safe_VkFenceCreateInfo(const safe_VkFenceCreateInfo& src);
    safe_VkFenceCreateInfo& operator=(const safe_VkFenceCreateInfo& src);
    safe_VkFenceCreateInfo();
    ~safe_VkFenceCreateInfo();
    void initialize(const VkFenceCreateInfo* in_struct);
    void initialize(const safe_VkFenceCreateInfo* src);
    VkFenceCreateInfo *ptr() { return reinterpret_cast<VkFenceCreateInfo *>(this); }
    VkFenceCreateInfo const *ptr() const { return reinterpret_cast<VkFenceCreateInfo const *>(this); }
};

struct safe_VkSemaphoreCreateInfo {
    VkStructureType sType;
    const void* pNext;
    VkSemaphoreCreateFlags flags;
    safe_VkSemaphoreCreateInfo(const VkSemaphoreCreateInfo* in_struct);
    safe_VkSemaphoreCreateInfo(const safe_VkSemaphoreCreateInfo& src);
    safe_VkSemaphoreCreateInfo& operator=(const safe_VkSemaphoreCreateInfo& src);
    safe_VkSemaphoreCreateInfo();
    ~safe_VkSemaphoreCreateInfo();
    void initialize(const VkSemaphoreCreateInfo* in_struct);
    void initialize(const safe_VkSemaphoreCreateInfo* src);
    VkSemaphoreCreateInfo *ptr() { return reinterpret_cast<VkSemaphoreCreateInfo *>(this); }
    VkSemaphoreCreateInfo const *ptr() const { return reinterpret_cast<VkSemaphoreCreateInfo const *>(this); }
};

struct safe_VkEventCreateInfo {
    VkStructureType sType;
    const void* pNext;
    VkEventCreateFlags flags;
    safe_VkEventCreateInfo(const VkEventCreateInfo* in_struct);
    safe_VkEventCreateInfo(const safe_VkEventCreateInfo& src);
    safe_VkEventCreateInfo& operator=(const safe_VkEventCreateInfo& src);
    safe_VkEventCreateInfo();
    ~safe_VkEventCreateInfo();
    void initialize(const VkEventCreateInfo* in_struct);
    void initialize(const safe_VkEventCreateInfo* src);
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
    safe_VkQueryPoolCreateInfo(const safe_VkQueryPoolCreateInfo& src);
    safe_VkQueryPoolCreateInfo& operator=(const safe_VkQueryPoolCreateInfo& src);
    safe_VkQueryPoolCreateInfo();
    ~safe_VkQueryPoolCreateInfo();
    void initialize(const VkQueryPoolCreateInfo* in_struct);
    void initialize(const safe_VkQueryPoolCreateInfo* src);
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
    safe_VkBufferCreateInfo(const safe_VkBufferCreateInfo& src);
    safe_VkBufferCreateInfo& operator=(const safe_VkBufferCreateInfo& src);
    safe_VkBufferCreateInfo();
    ~safe_VkBufferCreateInfo();
    void initialize(const VkBufferCreateInfo* in_struct);
    void initialize(const safe_VkBufferCreateInfo* src);
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
    safe_VkBufferViewCreateInfo(const safe_VkBufferViewCreateInfo& src);
    safe_VkBufferViewCreateInfo& operator=(const safe_VkBufferViewCreateInfo& src);
    safe_VkBufferViewCreateInfo();
    ~safe_VkBufferViewCreateInfo();
    void initialize(const VkBufferViewCreateInfo* in_struct);
    void initialize(const safe_VkBufferViewCreateInfo* src);
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
    safe_VkImageCreateInfo(const safe_VkImageCreateInfo& src);
    safe_VkImageCreateInfo& operator=(const safe_VkImageCreateInfo& src);
    safe_VkImageCreateInfo();
    ~safe_VkImageCreateInfo();
    void initialize(const VkImageCreateInfo* in_struct);
    void initialize(const safe_VkImageCreateInfo* src);
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
    safe_VkImageViewCreateInfo(const safe_VkImageViewCreateInfo& src);
    safe_VkImageViewCreateInfo& operator=(const safe_VkImageViewCreateInfo& src);
    safe_VkImageViewCreateInfo();
    ~safe_VkImageViewCreateInfo();
    void initialize(const VkImageViewCreateInfo* in_struct);
    void initialize(const safe_VkImageViewCreateInfo* src);
    VkImageViewCreateInfo *ptr() { return reinterpret_cast<VkImageViewCreateInfo *>(this); }
    VkImageViewCreateInfo const *ptr() const { return reinterpret_cast<VkImageViewCreateInfo const *>(this); }
};

struct safe_VkShaderModuleCreateInfo {
    VkStructureType sType;
    const void* pNext;
    VkShaderModuleCreateFlags flags;
    size_t codeSize;
    const uint32_t* pCode;
    safe_VkShaderModuleCreateInfo(const VkShaderModuleCreateInfo* in_struct);
    safe_VkShaderModuleCreateInfo(const safe_VkShaderModuleCreateInfo& src);
    safe_VkShaderModuleCreateInfo& operator=(const safe_VkShaderModuleCreateInfo& src);
    safe_VkShaderModuleCreateInfo();
    ~safe_VkShaderModuleCreateInfo();
    void initialize(const VkShaderModuleCreateInfo* in_struct);
    void initialize(const safe_VkShaderModuleCreateInfo* src);
    VkShaderModuleCreateInfo *ptr() { return reinterpret_cast<VkShaderModuleCreateInfo *>(this); }
    VkShaderModuleCreateInfo const *ptr() const { return reinterpret_cast<VkShaderModuleCreateInfo const *>(this); }
};

struct safe_VkPipelineCacheCreateInfo {
    VkStructureType sType;
    const void* pNext;
    VkPipelineCacheCreateFlags flags;
    size_t initialDataSize;
    const void* pInitialData;
    safe_VkPipelineCacheCreateInfo(const VkPipelineCacheCreateInfo* in_struct);
    safe_VkPipelineCacheCreateInfo(const safe_VkPipelineCacheCreateInfo& src);
    safe_VkPipelineCacheCreateInfo& operator=(const safe_VkPipelineCacheCreateInfo& src);
    safe_VkPipelineCacheCreateInfo();
    ~safe_VkPipelineCacheCreateInfo();
    void initialize(const VkPipelineCacheCreateInfo* in_struct);
    void initialize(const safe_VkPipelineCacheCreateInfo* src);
    VkPipelineCacheCreateInfo *ptr() { return reinterpret_cast<VkPipelineCacheCreateInfo *>(this); }
    VkPipelineCacheCreateInfo const *ptr() const { return reinterpret_cast<VkPipelineCacheCreateInfo const *>(this); }
};

struct safe_VkSpecializationInfo {
    uint32_t mapEntryCount;
    const VkSpecializationMapEntry* pMapEntries;
    size_t dataSize;
    const void* pData;
    safe_VkSpecializationInfo(const VkSpecializationInfo* in_struct);
    safe_VkSpecializationInfo(const safe_VkSpecializationInfo& src);
    safe_VkSpecializationInfo& operator=(const safe_VkSpecializationInfo& src);
    safe_VkSpecializationInfo();
    ~safe_VkSpecializationInfo();
    void initialize(const VkSpecializationInfo* in_struct);
    void initialize(const safe_VkSpecializationInfo* src);
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
    safe_VkPipelineShaderStageCreateInfo(const safe_VkPipelineShaderStageCreateInfo& src);
    safe_VkPipelineShaderStageCreateInfo& operator=(const safe_VkPipelineShaderStageCreateInfo& src);
    safe_VkPipelineShaderStageCreateInfo();
    ~safe_VkPipelineShaderStageCreateInfo();
    void initialize(const VkPipelineShaderStageCreateInfo* in_struct);
    void initialize(const safe_VkPipelineShaderStageCreateInfo* src);
    VkPipelineShaderStageCreateInfo *ptr() { return reinterpret_cast<VkPipelineShaderStageCreateInfo *>(this); }
    VkPipelineShaderStageCreateInfo const *ptr() const { return reinterpret_cast<VkPipelineShaderStageCreateInfo const *>(this); }
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
    safe_VkPipelineVertexInputStateCreateInfo(const safe_VkPipelineVertexInputStateCreateInfo& src);
    safe_VkPipelineVertexInputStateCreateInfo& operator=(const safe_VkPipelineVertexInputStateCreateInfo& src);
    safe_VkPipelineVertexInputStateCreateInfo();
    ~safe_VkPipelineVertexInputStateCreateInfo();
    void initialize(const VkPipelineVertexInputStateCreateInfo* in_struct);
    void initialize(const safe_VkPipelineVertexInputStateCreateInfo* src);
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
    safe_VkPipelineInputAssemblyStateCreateInfo(const safe_VkPipelineInputAssemblyStateCreateInfo& src);
    safe_VkPipelineInputAssemblyStateCreateInfo& operator=(const safe_VkPipelineInputAssemblyStateCreateInfo& src);
    safe_VkPipelineInputAssemblyStateCreateInfo();
    ~safe_VkPipelineInputAssemblyStateCreateInfo();
    void initialize(const VkPipelineInputAssemblyStateCreateInfo* in_struct);
    void initialize(const safe_VkPipelineInputAssemblyStateCreateInfo* src);
    VkPipelineInputAssemblyStateCreateInfo *ptr() { return reinterpret_cast<VkPipelineInputAssemblyStateCreateInfo *>(this); }
    VkPipelineInputAssemblyStateCreateInfo const *ptr() const { return reinterpret_cast<VkPipelineInputAssemblyStateCreateInfo const *>(this); }
};

struct safe_VkPipelineTessellationStateCreateInfo {
    VkStructureType sType;
    const void* pNext;
    VkPipelineTessellationStateCreateFlags flags;
    uint32_t patchControlPoints;
    safe_VkPipelineTessellationStateCreateInfo(const VkPipelineTessellationStateCreateInfo* in_struct);
    safe_VkPipelineTessellationStateCreateInfo(const safe_VkPipelineTessellationStateCreateInfo& src);
    safe_VkPipelineTessellationStateCreateInfo& operator=(const safe_VkPipelineTessellationStateCreateInfo& src);
    safe_VkPipelineTessellationStateCreateInfo();
    ~safe_VkPipelineTessellationStateCreateInfo();
    void initialize(const VkPipelineTessellationStateCreateInfo* in_struct);
    void initialize(const safe_VkPipelineTessellationStateCreateInfo* src);
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
    safe_VkPipelineViewportStateCreateInfo(const safe_VkPipelineViewportStateCreateInfo& src);
    safe_VkPipelineViewportStateCreateInfo& operator=(const safe_VkPipelineViewportStateCreateInfo& src);
    safe_VkPipelineViewportStateCreateInfo();
    ~safe_VkPipelineViewportStateCreateInfo();
    void initialize(const VkPipelineViewportStateCreateInfo* in_struct, const bool is_dynamic_viewports, const bool is_dynamic_scissors);
    void initialize(const safe_VkPipelineViewportStateCreateInfo* src);
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
    safe_VkPipelineRasterizationStateCreateInfo(const safe_VkPipelineRasterizationStateCreateInfo& src);
    safe_VkPipelineRasterizationStateCreateInfo& operator=(const safe_VkPipelineRasterizationStateCreateInfo& src);
    safe_VkPipelineRasterizationStateCreateInfo();
    ~safe_VkPipelineRasterizationStateCreateInfo();
    void initialize(const VkPipelineRasterizationStateCreateInfo* in_struct);
    void initialize(const safe_VkPipelineRasterizationStateCreateInfo* src);
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
    safe_VkPipelineMultisampleStateCreateInfo(const safe_VkPipelineMultisampleStateCreateInfo& src);
    safe_VkPipelineMultisampleStateCreateInfo& operator=(const safe_VkPipelineMultisampleStateCreateInfo& src);
    safe_VkPipelineMultisampleStateCreateInfo();
    ~safe_VkPipelineMultisampleStateCreateInfo();
    void initialize(const VkPipelineMultisampleStateCreateInfo* in_struct);
    void initialize(const safe_VkPipelineMultisampleStateCreateInfo* src);
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
    safe_VkPipelineDepthStencilStateCreateInfo(const safe_VkPipelineDepthStencilStateCreateInfo& src);
    safe_VkPipelineDepthStencilStateCreateInfo& operator=(const safe_VkPipelineDepthStencilStateCreateInfo& src);
    safe_VkPipelineDepthStencilStateCreateInfo();
    ~safe_VkPipelineDepthStencilStateCreateInfo();
    void initialize(const VkPipelineDepthStencilStateCreateInfo* in_struct);
    void initialize(const safe_VkPipelineDepthStencilStateCreateInfo* src);
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
    safe_VkPipelineColorBlendStateCreateInfo(const safe_VkPipelineColorBlendStateCreateInfo& src);
    safe_VkPipelineColorBlendStateCreateInfo& operator=(const safe_VkPipelineColorBlendStateCreateInfo& src);
    safe_VkPipelineColorBlendStateCreateInfo();
    ~safe_VkPipelineColorBlendStateCreateInfo();
    void initialize(const VkPipelineColorBlendStateCreateInfo* in_struct);
    void initialize(const safe_VkPipelineColorBlendStateCreateInfo* src);
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
    safe_VkPipelineDynamicStateCreateInfo(const safe_VkPipelineDynamicStateCreateInfo& src);
    safe_VkPipelineDynamicStateCreateInfo& operator=(const safe_VkPipelineDynamicStateCreateInfo& src);
    safe_VkPipelineDynamicStateCreateInfo();
    ~safe_VkPipelineDynamicStateCreateInfo();
    void initialize(const VkPipelineDynamicStateCreateInfo* in_struct);
    void initialize(const safe_VkPipelineDynamicStateCreateInfo* src);
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
    safe_VkGraphicsPipelineCreateInfo(const safe_VkGraphicsPipelineCreateInfo& src);
    safe_VkGraphicsPipelineCreateInfo& operator=(const safe_VkGraphicsPipelineCreateInfo& src);
    safe_VkGraphicsPipelineCreateInfo();
    ~safe_VkGraphicsPipelineCreateInfo();
    void initialize(const VkGraphicsPipelineCreateInfo* in_struct, const bool uses_color_attachment, const bool uses_depthstencil_attachment);
    void initialize(const safe_VkGraphicsPipelineCreateInfo* src);
    VkGraphicsPipelineCreateInfo *ptr() { return reinterpret_cast<VkGraphicsPipelineCreateInfo *>(this); }
    VkGraphicsPipelineCreateInfo const *ptr() const { return reinterpret_cast<VkGraphicsPipelineCreateInfo const *>(this); }
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
    safe_VkComputePipelineCreateInfo(const safe_VkComputePipelineCreateInfo& src);
    safe_VkComputePipelineCreateInfo& operator=(const safe_VkComputePipelineCreateInfo& src);
    safe_VkComputePipelineCreateInfo();
    ~safe_VkComputePipelineCreateInfo();
    void initialize(const VkComputePipelineCreateInfo* in_struct);
    void initialize(const safe_VkComputePipelineCreateInfo* src);
    VkComputePipelineCreateInfo *ptr() { return reinterpret_cast<VkComputePipelineCreateInfo *>(this); }
    VkComputePipelineCreateInfo const *ptr() const { return reinterpret_cast<VkComputePipelineCreateInfo const *>(this); }
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
    safe_VkPipelineLayoutCreateInfo(const safe_VkPipelineLayoutCreateInfo& src);
    safe_VkPipelineLayoutCreateInfo& operator=(const safe_VkPipelineLayoutCreateInfo& src);
    safe_VkPipelineLayoutCreateInfo();
    ~safe_VkPipelineLayoutCreateInfo();
    void initialize(const VkPipelineLayoutCreateInfo* in_struct);
    void initialize(const safe_VkPipelineLayoutCreateInfo* src);
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
    safe_VkSamplerCreateInfo(const safe_VkSamplerCreateInfo& src);
    safe_VkSamplerCreateInfo& operator=(const safe_VkSamplerCreateInfo& src);
    safe_VkSamplerCreateInfo();
    ~safe_VkSamplerCreateInfo();
    void initialize(const VkSamplerCreateInfo* in_struct);
    void initialize(const safe_VkSamplerCreateInfo* src);
    VkSamplerCreateInfo *ptr() { return reinterpret_cast<VkSamplerCreateInfo *>(this); }
    VkSamplerCreateInfo const *ptr() const { return reinterpret_cast<VkSamplerCreateInfo const *>(this); }
};

struct safe_VkDescriptorSetLayoutBinding {
    uint32_t binding;
    VkDescriptorType descriptorType;
    uint32_t descriptorCount;
    VkShaderStageFlags stageFlags;
    VkSampler* pImmutableSamplers;
    safe_VkDescriptorSetLayoutBinding(const VkDescriptorSetLayoutBinding* in_struct);
    safe_VkDescriptorSetLayoutBinding(const safe_VkDescriptorSetLayoutBinding& src);
    safe_VkDescriptorSetLayoutBinding& operator=(const safe_VkDescriptorSetLayoutBinding& src);
    safe_VkDescriptorSetLayoutBinding();
    ~safe_VkDescriptorSetLayoutBinding();
    void initialize(const VkDescriptorSetLayoutBinding* in_struct);
    void initialize(const safe_VkDescriptorSetLayoutBinding* src);
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
    safe_VkDescriptorSetLayoutCreateInfo(const safe_VkDescriptorSetLayoutCreateInfo& src);
    safe_VkDescriptorSetLayoutCreateInfo& operator=(const safe_VkDescriptorSetLayoutCreateInfo& src);
    safe_VkDescriptorSetLayoutCreateInfo();
    ~safe_VkDescriptorSetLayoutCreateInfo();
    void initialize(const VkDescriptorSetLayoutCreateInfo* in_struct);
    void initialize(const safe_VkDescriptorSetLayoutCreateInfo* src);
    VkDescriptorSetLayoutCreateInfo *ptr() { return reinterpret_cast<VkDescriptorSetLayoutCreateInfo *>(this); }
    VkDescriptorSetLayoutCreateInfo const *ptr() const { return reinterpret_cast<VkDescriptorSetLayoutCreateInfo const *>(this); }
};

struct safe_VkDescriptorPoolCreateInfo {
    VkStructureType sType;
    const void* pNext;
    VkDescriptorPoolCreateFlags flags;
    uint32_t maxSets;
    uint32_t poolSizeCount;
    const VkDescriptorPoolSize* pPoolSizes;
    safe_VkDescriptorPoolCreateInfo(const VkDescriptorPoolCreateInfo* in_struct);
    safe_VkDescriptorPoolCreateInfo(const safe_VkDescriptorPoolCreateInfo& src);
    safe_VkDescriptorPoolCreateInfo& operator=(const safe_VkDescriptorPoolCreateInfo& src);
    safe_VkDescriptorPoolCreateInfo();
    ~safe_VkDescriptorPoolCreateInfo();
    void initialize(const VkDescriptorPoolCreateInfo* in_struct);
    void initialize(const safe_VkDescriptorPoolCreateInfo* src);
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
    safe_VkDescriptorSetAllocateInfo(const safe_VkDescriptorSetAllocateInfo& src);
    safe_VkDescriptorSetAllocateInfo& operator=(const safe_VkDescriptorSetAllocateInfo& src);
    safe_VkDescriptorSetAllocateInfo();
    ~safe_VkDescriptorSetAllocateInfo();
    void initialize(const VkDescriptorSetAllocateInfo* in_struct);
    void initialize(const safe_VkDescriptorSetAllocateInfo* src);
    VkDescriptorSetAllocateInfo *ptr() { return reinterpret_cast<VkDescriptorSetAllocateInfo *>(this); }
    VkDescriptorSetAllocateInfo const *ptr() const { return reinterpret_cast<VkDescriptorSetAllocateInfo const *>(this); }
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
    safe_VkWriteDescriptorSet(const safe_VkWriteDescriptorSet& src);
    safe_VkWriteDescriptorSet& operator=(const safe_VkWriteDescriptorSet& src);
    safe_VkWriteDescriptorSet();
    ~safe_VkWriteDescriptorSet();
    void initialize(const VkWriteDescriptorSet* in_struct);
    void initialize(const safe_VkWriteDescriptorSet* src);
    VkWriteDescriptorSet *ptr() { return reinterpret_cast<VkWriteDescriptorSet *>(this); }
    VkWriteDescriptorSet const *ptr() const { return reinterpret_cast<VkWriteDescriptorSet const *>(this); }
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
    safe_VkCopyDescriptorSet(const safe_VkCopyDescriptorSet& src);
    safe_VkCopyDescriptorSet& operator=(const safe_VkCopyDescriptorSet& src);
    safe_VkCopyDescriptorSet();
    ~safe_VkCopyDescriptorSet();
    void initialize(const VkCopyDescriptorSet* in_struct);
    void initialize(const safe_VkCopyDescriptorSet* src);
    VkCopyDescriptorSet *ptr() { return reinterpret_cast<VkCopyDescriptorSet *>(this); }
    VkCopyDescriptorSet const *ptr() const { return reinterpret_cast<VkCopyDescriptorSet const *>(this); }
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
    safe_VkFramebufferCreateInfo(const safe_VkFramebufferCreateInfo& src);
    safe_VkFramebufferCreateInfo& operator=(const safe_VkFramebufferCreateInfo& src);
    safe_VkFramebufferCreateInfo();
    ~safe_VkFramebufferCreateInfo();
    void initialize(const VkFramebufferCreateInfo* in_struct);
    void initialize(const safe_VkFramebufferCreateInfo* src);
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
    safe_VkSubpassDescription(const safe_VkSubpassDescription& src);
    safe_VkSubpassDescription& operator=(const safe_VkSubpassDescription& src);
    safe_VkSubpassDescription();
    ~safe_VkSubpassDescription();
    void initialize(const VkSubpassDescription* in_struct);
    void initialize(const safe_VkSubpassDescription* src);
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
    safe_VkRenderPassCreateInfo(const safe_VkRenderPassCreateInfo& src);
    safe_VkRenderPassCreateInfo& operator=(const safe_VkRenderPassCreateInfo& src);
    safe_VkRenderPassCreateInfo();
    ~safe_VkRenderPassCreateInfo();
    void initialize(const VkRenderPassCreateInfo* in_struct);
    void initialize(const safe_VkRenderPassCreateInfo* src);
    VkRenderPassCreateInfo *ptr() { return reinterpret_cast<VkRenderPassCreateInfo *>(this); }
    VkRenderPassCreateInfo const *ptr() const { return reinterpret_cast<VkRenderPassCreateInfo const *>(this); }
};

struct safe_VkCommandPoolCreateInfo {
    VkStructureType sType;
    const void* pNext;
    VkCommandPoolCreateFlags flags;
    uint32_t queueFamilyIndex;
    safe_VkCommandPoolCreateInfo(const VkCommandPoolCreateInfo* in_struct);
    safe_VkCommandPoolCreateInfo(const safe_VkCommandPoolCreateInfo& src);
    safe_VkCommandPoolCreateInfo& operator=(const safe_VkCommandPoolCreateInfo& src);
    safe_VkCommandPoolCreateInfo();
    ~safe_VkCommandPoolCreateInfo();
    void initialize(const VkCommandPoolCreateInfo* in_struct);
    void initialize(const safe_VkCommandPoolCreateInfo* src);
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
    safe_VkCommandBufferAllocateInfo(const safe_VkCommandBufferAllocateInfo& src);
    safe_VkCommandBufferAllocateInfo& operator=(const safe_VkCommandBufferAllocateInfo& src);
    safe_VkCommandBufferAllocateInfo();
    ~safe_VkCommandBufferAllocateInfo();
    void initialize(const VkCommandBufferAllocateInfo* in_struct);
    void initialize(const safe_VkCommandBufferAllocateInfo* src);
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
    safe_VkCommandBufferInheritanceInfo(const safe_VkCommandBufferInheritanceInfo& src);
    safe_VkCommandBufferInheritanceInfo& operator=(const safe_VkCommandBufferInheritanceInfo& src);
    safe_VkCommandBufferInheritanceInfo();
    ~safe_VkCommandBufferInheritanceInfo();
    void initialize(const VkCommandBufferInheritanceInfo* in_struct);
    void initialize(const safe_VkCommandBufferInheritanceInfo* src);
    VkCommandBufferInheritanceInfo *ptr() { return reinterpret_cast<VkCommandBufferInheritanceInfo *>(this); }
    VkCommandBufferInheritanceInfo const *ptr() const { return reinterpret_cast<VkCommandBufferInheritanceInfo const *>(this); }
};

struct safe_VkCommandBufferBeginInfo {
    VkStructureType sType;
    const void* pNext;
    VkCommandBufferUsageFlags flags;
    safe_VkCommandBufferInheritanceInfo* pInheritanceInfo;
    safe_VkCommandBufferBeginInfo(const VkCommandBufferBeginInfo* in_struct);
    safe_VkCommandBufferBeginInfo(const safe_VkCommandBufferBeginInfo& src);
    safe_VkCommandBufferBeginInfo& operator=(const safe_VkCommandBufferBeginInfo& src);
    safe_VkCommandBufferBeginInfo();
    ~safe_VkCommandBufferBeginInfo();
    void initialize(const VkCommandBufferBeginInfo* in_struct);
    void initialize(const safe_VkCommandBufferBeginInfo* src);
    VkCommandBufferBeginInfo *ptr() { return reinterpret_cast<VkCommandBufferBeginInfo *>(this); }
    VkCommandBufferBeginInfo const *ptr() const { return reinterpret_cast<VkCommandBufferBeginInfo const *>(this); }
};

struct safe_VkMemoryBarrier {
    VkStructureType sType;
    const void* pNext;
    VkAccessFlags srcAccessMask;
    VkAccessFlags dstAccessMask;
    safe_VkMemoryBarrier(const VkMemoryBarrier* in_struct);
    safe_VkMemoryBarrier(const safe_VkMemoryBarrier& src);
    safe_VkMemoryBarrier& operator=(const safe_VkMemoryBarrier& src);
    safe_VkMemoryBarrier();
    ~safe_VkMemoryBarrier();
    void initialize(const VkMemoryBarrier* in_struct);
    void initialize(const safe_VkMemoryBarrier* src);
    VkMemoryBarrier *ptr() { return reinterpret_cast<VkMemoryBarrier *>(this); }
    VkMemoryBarrier const *ptr() const { return reinterpret_cast<VkMemoryBarrier const *>(this); }
};

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
    safe_VkBufferMemoryBarrier(const safe_VkBufferMemoryBarrier& src);
    safe_VkBufferMemoryBarrier& operator=(const safe_VkBufferMemoryBarrier& src);
    safe_VkBufferMemoryBarrier();
    ~safe_VkBufferMemoryBarrier();
    void initialize(const VkBufferMemoryBarrier* in_struct);
    void initialize(const safe_VkBufferMemoryBarrier* src);
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
    safe_VkImageMemoryBarrier(const safe_VkImageMemoryBarrier& src);
    safe_VkImageMemoryBarrier& operator=(const safe_VkImageMemoryBarrier& src);
    safe_VkImageMemoryBarrier();
    ~safe_VkImageMemoryBarrier();
    void initialize(const VkImageMemoryBarrier* in_struct);
    void initialize(const safe_VkImageMemoryBarrier* src);
    VkImageMemoryBarrier *ptr() { return reinterpret_cast<VkImageMemoryBarrier *>(this); }
    VkImageMemoryBarrier const *ptr() const { return reinterpret_cast<VkImageMemoryBarrier const *>(this); }
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
    safe_VkRenderPassBeginInfo(const safe_VkRenderPassBeginInfo& src);
    safe_VkRenderPassBeginInfo& operator=(const safe_VkRenderPassBeginInfo& src);
    safe_VkRenderPassBeginInfo();
    ~safe_VkRenderPassBeginInfo();
    void initialize(const VkRenderPassBeginInfo* in_struct);
    void initialize(const safe_VkRenderPassBeginInfo* src);
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
    safe_VkPhysicalDeviceSubgroupProperties(const safe_VkPhysicalDeviceSubgroupProperties& src);
    safe_VkPhysicalDeviceSubgroupProperties& operator=(const safe_VkPhysicalDeviceSubgroupProperties& src);
    safe_VkPhysicalDeviceSubgroupProperties();
    ~safe_VkPhysicalDeviceSubgroupProperties();
    void initialize(const VkPhysicalDeviceSubgroupProperties* in_struct);
    void initialize(const safe_VkPhysicalDeviceSubgroupProperties* src);
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
    safe_VkBindBufferMemoryInfo(const safe_VkBindBufferMemoryInfo& src);
    safe_VkBindBufferMemoryInfo& operator=(const safe_VkBindBufferMemoryInfo& src);
    safe_VkBindBufferMemoryInfo();
    ~safe_VkBindBufferMemoryInfo();
    void initialize(const VkBindBufferMemoryInfo* in_struct);
    void initialize(const safe_VkBindBufferMemoryInfo* src);
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
    safe_VkBindImageMemoryInfo(const safe_VkBindImageMemoryInfo& src);
    safe_VkBindImageMemoryInfo& operator=(const safe_VkBindImageMemoryInfo& src);
    safe_VkBindImageMemoryInfo();
    ~safe_VkBindImageMemoryInfo();
    void initialize(const VkBindImageMemoryInfo* in_struct);
    void initialize(const safe_VkBindImageMemoryInfo* src);
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
    safe_VkPhysicalDevice16BitStorageFeatures(const safe_VkPhysicalDevice16BitStorageFeatures& src);
    safe_VkPhysicalDevice16BitStorageFeatures& operator=(const safe_VkPhysicalDevice16BitStorageFeatures& src);
    safe_VkPhysicalDevice16BitStorageFeatures();
    ~safe_VkPhysicalDevice16BitStorageFeatures();
    void initialize(const VkPhysicalDevice16BitStorageFeatures* in_struct);
    void initialize(const safe_VkPhysicalDevice16BitStorageFeatures* src);
    VkPhysicalDevice16BitStorageFeatures *ptr() { return reinterpret_cast<VkPhysicalDevice16BitStorageFeatures *>(this); }
    VkPhysicalDevice16BitStorageFeatures const *ptr() const { return reinterpret_cast<VkPhysicalDevice16BitStorageFeatures const *>(this); }
};

struct safe_VkMemoryDedicatedRequirements {
    VkStructureType sType;
    void* pNext;
    VkBool32 prefersDedicatedAllocation;
    VkBool32 requiresDedicatedAllocation;
    safe_VkMemoryDedicatedRequirements(const VkMemoryDedicatedRequirements* in_struct);
    safe_VkMemoryDedicatedRequirements(const safe_VkMemoryDedicatedRequirements& src);
    safe_VkMemoryDedicatedRequirements& operator=(const safe_VkMemoryDedicatedRequirements& src);
    safe_VkMemoryDedicatedRequirements();
    ~safe_VkMemoryDedicatedRequirements();
    void initialize(const VkMemoryDedicatedRequirements* in_struct);
    void initialize(const safe_VkMemoryDedicatedRequirements* src);
    VkMemoryDedicatedRequirements *ptr() { return reinterpret_cast<VkMemoryDedicatedRequirements *>(this); }
    VkMemoryDedicatedRequirements const *ptr() const { return reinterpret_cast<VkMemoryDedicatedRequirements const *>(this); }
};

struct safe_VkMemoryDedicatedAllocateInfo {
    VkStructureType sType;
    const void* pNext;
    VkImage image;
    VkBuffer buffer;
    safe_VkMemoryDedicatedAllocateInfo(const VkMemoryDedicatedAllocateInfo* in_struct);
    safe_VkMemoryDedicatedAllocateInfo(const safe_VkMemoryDedicatedAllocateInfo& src);
    safe_VkMemoryDedicatedAllocateInfo& operator=(const safe_VkMemoryDedicatedAllocateInfo& src);
    safe_VkMemoryDedicatedAllocateInfo();
    ~safe_VkMemoryDedicatedAllocateInfo();
    void initialize(const VkMemoryDedicatedAllocateInfo* in_struct);
    void initialize(const safe_VkMemoryDedicatedAllocateInfo* src);
    VkMemoryDedicatedAllocateInfo *ptr() { return reinterpret_cast<VkMemoryDedicatedAllocateInfo *>(this); }
    VkMemoryDedicatedAllocateInfo const *ptr() const { return reinterpret_cast<VkMemoryDedicatedAllocateInfo const *>(this); }
};

struct safe_VkMemoryAllocateFlagsInfo {
    VkStructureType sType;
    const void* pNext;
    VkMemoryAllocateFlags flags;
    uint32_t deviceMask;
    safe_VkMemoryAllocateFlagsInfo(const VkMemoryAllocateFlagsInfo* in_struct);
    safe_VkMemoryAllocateFlagsInfo(const safe_VkMemoryAllocateFlagsInfo& src);
    safe_VkMemoryAllocateFlagsInfo& operator=(const safe_VkMemoryAllocateFlagsInfo& src);
    safe_VkMemoryAllocateFlagsInfo();
    ~safe_VkMemoryAllocateFlagsInfo();
    void initialize(const VkMemoryAllocateFlagsInfo* in_struct);
    void initialize(const safe_VkMemoryAllocateFlagsInfo* src);
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
    safe_VkDeviceGroupRenderPassBeginInfo(const safe_VkDeviceGroupRenderPassBeginInfo& src);
    safe_VkDeviceGroupRenderPassBeginInfo& operator=(const safe_VkDeviceGroupRenderPassBeginInfo& src);
    safe_VkDeviceGroupRenderPassBeginInfo();
    ~safe_VkDeviceGroupRenderPassBeginInfo();
    void initialize(const VkDeviceGroupRenderPassBeginInfo* in_struct);
    void initialize(const safe_VkDeviceGroupRenderPassBeginInfo* src);
    VkDeviceGroupRenderPassBeginInfo *ptr() { return reinterpret_cast<VkDeviceGroupRenderPassBeginInfo *>(this); }
    VkDeviceGroupRenderPassBeginInfo const *ptr() const { return reinterpret_cast<VkDeviceGroupRenderPassBeginInfo const *>(this); }
};

struct safe_VkDeviceGroupCommandBufferBeginInfo {
    VkStructureType sType;
    const void* pNext;
    uint32_t deviceMask;
    safe_VkDeviceGroupCommandBufferBeginInfo(const VkDeviceGroupCommandBufferBeginInfo* in_struct);
    safe_VkDeviceGroupCommandBufferBeginInfo(const safe_VkDeviceGroupCommandBufferBeginInfo& src);
    safe_VkDeviceGroupCommandBufferBeginInfo& operator=(const safe_VkDeviceGroupCommandBufferBeginInfo& src);
    safe_VkDeviceGroupCommandBufferBeginInfo();
    ~safe_VkDeviceGroupCommandBufferBeginInfo();
    void initialize(const VkDeviceGroupCommandBufferBeginInfo* in_struct);
    void initialize(const safe_VkDeviceGroupCommandBufferBeginInfo* src);
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
    safe_VkDeviceGroupSubmitInfo(const safe_VkDeviceGroupSubmitInfo& src);
    safe_VkDeviceGroupSubmitInfo& operator=(const safe_VkDeviceGroupSubmitInfo& src);
    safe_VkDeviceGroupSubmitInfo();
    ~safe_VkDeviceGroupSubmitInfo();
    void initialize(const VkDeviceGroupSubmitInfo* in_struct);
    void initialize(const safe_VkDeviceGroupSubmitInfo* src);
    VkDeviceGroupSubmitInfo *ptr() { return reinterpret_cast<VkDeviceGroupSubmitInfo *>(this); }
    VkDeviceGroupSubmitInfo const *ptr() const { return reinterpret_cast<VkDeviceGroupSubmitInfo const *>(this); }
};

struct safe_VkDeviceGroupBindSparseInfo {
    VkStructureType sType;
    const void* pNext;
    uint32_t resourceDeviceIndex;
    uint32_t memoryDeviceIndex;
    safe_VkDeviceGroupBindSparseInfo(const VkDeviceGroupBindSparseInfo* in_struct);
    safe_VkDeviceGroupBindSparseInfo(const safe_VkDeviceGroupBindSparseInfo& src);
    safe_VkDeviceGroupBindSparseInfo& operator=(const safe_VkDeviceGroupBindSparseInfo& src);
    safe_VkDeviceGroupBindSparseInfo();
    ~safe_VkDeviceGroupBindSparseInfo();
    void initialize(const VkDeviceGroupBindSparseInfo* in_struct);
    void initialize(const safe_VkDeviceGroupBindSparseInfo* src);
    VkDeviceGroupBindSparseInfo *ptr() { return reinterpret_cast<VkDeviceGroupBindSparseInfo *>(this); }
    VkDeviceGroupBindSparseInfo const *ptr() const { return reinterpret_cast<VkDeviceGroupBindSparseInfo const *>(this); }
};

struct safe_VkBindBufferMemoryDeviceGroupInfo {
    VkStructureType sType;
    const void* pNext;
    uint32_t deviceIndexCount;
    const uint32_t* pDeviceIndices;
    safe_VkBindBufferMemoryDeviceGroupInfo(const VkBindBufferMemoryDeviceGroupInfo* in_struct);
    safe_VkBindBufferMemoryDeviceGroupInfo(const safe_VkBindBufferMemoryDeviceGroupInfo& src);
    safe_VkBindBufferMemoryDeviceGroupInfo& operator=(const safe_VkBindBufferMemoryDeviceGroupInfo& src);
    safe_VkBindBufferMemoryDeviceGroupInfo();
    ~safe_VkBindBufferMemoryDeviceGroupInfo();
    void initialize(const VkBindBufferMemoryDeviceGroupInfo* in_struct);
    void initialize(const safe_VkBindBufferMemoryDeviceGroupInfo* src);
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
    safe_VkBindImageMemoryDeviceGroupInfo(const safe_VkBindImageMemoryDeviceGroupInfo& src);
    safe_VkBindImageMemoryDeviceGroupInfo& operator=(const safe_VkBindImageMemoryDeviceGroupInfo& src);
    safe_VkBindImageMemoryDeviceGroupInfo();
    ~safe_VkBindImageMemoryDeviceGroupInfo();
    void initialize(const VkBindImageMemoryDeviceGroupInfo* in_struct);
    void initialize(const safe_VkBindImageMemoryDeviceGroupInfo* src);
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
    safe_VkPhysicalDeviceGroupProperties(const safe_VkPhysicalDeviceGroupProperties& src);
    safe_VkPhysicalDeviceGroupProperties& operator=(const safe_VkPhysicalDeviceGroupProperties& src);
    safe_VkPhysicalDeviceGroupProperties();
    ~safe_VkPhysicalDeviceGroupProperties();
    void initialize(const VkPhysicalDeviceGroupProperties* in_struct);
    void initialize(const safe_VkPhysicalDeviceGroupProperties* src);
    VkPhysicalDeviceGroupProperties *ptr() { return reinterpret_cast<VkPhysicalDeviceGroupProperties *>(this); }
    VkPhysicalDeviceGroupProperties const *ptr() const { return reinterpret_cast<VkPhysicalDeviceGroupProperties const *>(this); }
};

struct safe_VkDeviceGroupDeviceCreateInfo {
    VkStructureType sType;
    const void* pNext;
    uint32_t physicalDeviceCount;
    VkPhysicalDevice* pPhysicalDevices;
    safe_VkDeviceGroupDeviceCreateInfo(const VkDeviceGroupDeviceCreateInfo* in_struct);
    safe_VkDeviceGroupDeviceCreateInfo(const safe_VkDeviceGroupDeviceCreateInfo& src);
    safe_VkDeviceGroupDeviceCreateInfo& operator=(const safe_VkDeviceGroupDeviceCreateInfo& src);
    safe_VkDeviceGroupDeviceCreateInfo();
    ~safe_VkDeviceGroupDeviceCreateInfo();
    void initialize(const VkDeviceGroupDeviceCreateInfo* in_struct);
    void initialize(const safe_VkDeviceGroupDeviceCreateInfo* src);
    VkDeviceGroupDeviceCreateInfo *ptr() { return reinterpret_cast<VkDeviceGroupDeviceCreateInfo *>(this); }
    VkDeviceGroupDeviceCreateInfo const *ptr() const { return reinterpret_cast<VkDeviceGroupDeviceCreateInfo const *>(this); }
};

struct safe_VkBufferMemoryRequirementsInfo2 {
    VkStructureType sType;
    const void* pNext;
    VkBuffer buffer;
    safe_VkBufferMemoryRequirementsInfo2(const VkBufferMemoryRequirementsInfo2* in_struct);
    safe_VkBufferMemoryRequirementsInfo2(const safe_VkBufferMemoryRequirementsInfo2& src);
    safe_VkBufferMemoryRequirementsInfo2& operator=(const safe_VkBufferMemoryRequirementsInfo2& src);
    safe_VkBufferMemoryRequirementsInfo2();
    ~safe_VkBufferMemoryRequirementsInfo2();
    void initialize(const VkBufferMemoryRequirementsInfo2* in_struct);
    void initialize(const safe_VkBufferMemoryRequirementsInfo2* src);
    VkBufferMemoryRequirementsInfo2 *ptr() { return reinterpret_cast<VkBufferMemoryRequirementsInfo2 *>(this); }
    VkBufferMemoryRequirementsInfo2 const *ptr() const { return reinterpret_cast<VkBufferMemoryRequirementsInfo2 const *>(this); }
};

struct safe_VkImageMemoryRequirementsInfo2 {
    VkStructureType sType;
    const void* pNext;
    VkImage image;
    safe_VkImageMemoryRequirementsInfo2(const VkImageMemoryRequirementsInfo2* in_struct);
    safe_VkImageMemoryRequirementsInfo2(const safe_VkImageMemoryRequirementsInfo2& src);
    safe_VkImageMemoryRequirementsInfo2& operator=(const safe_VkImageMemoryRequirementsInfo2& src);
    safe_VkImageMemoryRequirementsInfo2();
    ~safe_VkImageMemoryRequirementsInfo2();
    void initialize(const VkImageMemoryRequirementsInfo2* in_struct);
    void initialize(const safe_VkImageMemoryRequirementsInfo2* src);
    VkImageMemoryRequirementsInfo2 *ptr() { return reinterpret_cast<VkImageMemoryRequirementsInfo2 *>(this); }
    VkImageMemoryRequirementsInfo2 const *ptr() const { return reinterpret_cast<VkImageMemoryRequirementsInfo2 const *>(this); }
};

struct safe_VkImageSparseMemoryRequirementsInfo2 {
    VkStructureType sType;
    const void* pNext;
    VkImage image;
    safe_VkImageSparseMemoryRequirementsInfo2(const VkImageSparseMemoryRequirementsInfo2* in_struct);
    safe_VkImageSparseMemoryRequirementsInfo2(const safe_VkImageSparseMemoryRequirementsInfo2& src);
    safe_VkImageSparseMemoryRequirementsInfo2& operator=(const safe_VkImageSparseMemoryRequirementsInfo2& src);
    safe_VkImageSparseMemoryRequirementsInfo2();
    ~safe_VkImageSparseMemoryRequirementsInfo2();
    void initialize(const VkImageSparseMemoryRequirementsInfo2* in_struct);
    void initialize(const safe_VkImageSparseMemoryRequirementsInfo2* src);
    VkImageSparseMemoryRequirementsInfo2 *ptr() { return reinterpret_cast<VkImageSparseMemoryRequirementsInfo2 *>(this); }
    VkImageSparseMemoryRequirementsInfo2 const *ptr() const { return reinterpret_cast<VkImageSparseMemoryRequirementsInfo2 const *>(this); }
};

struct safe_VkMemoryRequirements2 {
    VkStructureType sType;
    void* pNext;
    VkMemoryRequirements memoryRequirements;
    safe_VkMemoryRequirements2(const VkMemoryRequirements2* in_struct);
    safe_VkMemoryRequirements2(const safe_VkMemoryRequirements2& src);
    safe_VkMemoryRequirements2& operator=(const safe_VkMemoryRequirements2& src);
    safe_VkMemoryRequirements2();
    ~safe_VkMemoryRequirements2();
    void initialize(const VkMemoryRequirements2* in_struct);
    void initialize(const safe_VkMemoryRequirements2* src);
    VkMemoryRequirements2 *ptr() { return reinterpret_cast<VkMemoryRequirements2 *>(this); }
    VkMemoryRequirements2 const *ptr() const { return reinterpret_cast<VkMemoryRequirements2 const *>(this); }
};

struct safe_VkSparseImageMemoryRequirements2 {
    VkStructureType sType;
    void* pNext;
    VkSparseImageMemoryRequirements memoryRequirements;
    safe_VkSparseImageMemoryRequirements2(const VkSparseImageMemoryRequirements2* in_struct);
    safe_VkSparseImageMemoryRequirements2(const safe_VkSparseImageMemoryRequirements2& src);
    safe_VkSparseImageMemoryRequirements2& operator=(const safe_VkSparseImageMemoryRequirements2& src);
    safe_VkSparseImageMemoryRequirements2();
    ~safe_VkSparseImageMemoryRequirements2();
    void initialize(const VkSparseImageMemoryRequirements2* in_struct);
    void initialize(const safe_VkSparseImageMemoryRequirements2* src);
    VkSparseImageMemoryRequirements2 *ptr() { return reinterpret_cast<VkSparseImageMemoryRequirements2 *>(this); }
    VkSparseImageMemoryRequirements2 const *ptr() const { return reinterpret_cast<VkSparseImageMemoryRequirements2 const *>(this); }
};

struct safe_VkPhysicalDeviceFeatures2 {
    VkStructureType sType;
    void* pNext;
    VkPhysicalDeviceFeatures features;
    safe_VkPhysicalDeviceFeatures2(const VkPhysicalDeviceFeatures2* in_struct);
    safe_VkPhysicalDeviceFeatures2(const safe_VkPhysicalDeviceFeatures2& src);
    safe_VkPhysicalDeviceFeatures2& operator=(const safe_VkPhysicalDeviceFeatures2& src);
    safe_VkPhysicalDeviceFeatures2();
    ~safe_VkPhysicalDeviceFeatures2();
    void initialize(const VkPhysicalDeviceFeatures2* in_struct);
    void initialize(const safe_VkPhysicalDeviceFeatures2* src);
    VkPhysicalDeviceFeatures2 *ptr() { return reinterpret_cast<VkPhysicalDeviceFeatures2 *>(this); }
    VkPhysicalDeviceFeatures2 const *ptr() const { return reinterpret_cast<VkPhysicalDeviceFeatures2 const *>(this); }
};

struct safe_VkPhysicalDeviceProperties2 {
    VkStructureType sType;
    void* pNext;
    VkPhysicalDeviceProperties properties;
    safe_VkPhysicalDeviceProperties2(const VkPhysicalDeviceProperties2* in_struct);
    safe_VkPhysicalDeviceProperties2(const safe_VkPhysicalDeviceProperties2& src);
    safe_VkPhysicalDeviceProperties2& operator=(const safe_VkPhysicalDeviceProperties2& src);
    safe_VkPhysicalDeviceProperties2();
    ~safe_VkPhysicalDeviceProperties2();
    void initialize(const VkPhysicalDeviceProperties2* in_struct);
    void initialize(const safe_VkPhysicalDeviceProperties2* src);
    VkPhysicalDeviceProperties2 *ptr() { return reinterpret_cast<VkPhysicalDeviceProperties2 *>(this); }
    VkPhysicalDeviceProperties2 const *ptr() const { return reinterpret_cast<VkPhysicalDeviceProperties2 const *>(this); }
};

struct safe_VkFormatProperties2 {
    VkStructureType sType;
    void* pNext;
    VkFormatProperties formatProperties;
    safe_VkFormatProperties2(const VkFormatProperties2* in_struct);
    safe_VkFormatProperties2(const safe_VkFormatProperties2& src);
    safe_VkFormatProperties2& operator=(const safe_VkFormatProperties2& src);
    safe_VkFormatProperties2();
    ~safe_VkFormatProperties2();
    void initialize(const VkFormatProperties2* in_struct);
    void initialize(const safe_VkFormatProperties2* src);
    VkFormatProperties2 *ptr() { return reinterpret_cast<VkFormatProperties2 *>(this); }
    VkFormatProperties2 const *ptr() const { return reinterpret_cast<VkFormatProperties2 const *>(this); }
};

struct safe_VkImageFormatProperties2 {
    VkStructureType sType;
    void* pNext;
    VkImageFormatProperties imageFormatProperties;
    safe_VkImageFormatProperties2(const VkImageFormatProperties2* in_struct);
    safe_VkImageFormatProperties2(const safe_VkImageFormatProperties2& src);
    safe_VkImageFormatProperties2& operator=(const safe_VkImageFormatProperties2& src);
    safe_VkImageFormatProperties2();
    ~safe_VkImageFormatProperties2();
    void initialize(const VkImageFormatProperties2* in_struct);
    void initialize(const safe_VkImageFormatProperties2* src);
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
    safe_VkPhysicalDeviceImageFormatInfo2(const safe_VkPhysicalDeviceImageFormatInfo2& src);
    safe_VkPhysicalDeviceImageFormatInfo2& operator=(const safe_VkPhysicalDeviceImageFormatInfo2& src);
    safe_VkPhysicalDeviceImageFormatInfo2();
    ~safe_VkPhysicalDeviceImageFormatInfo2();
    void initialize(const VkPhysicalDeviceImageFormatInfo2* in_struct);
    void initialize(const safe_VkPhysicalDeviceImageFormatInfo2* src);
    VkPhysicalDeviceImageFormatInfo2 *ptr() { return reinterpret_cast<VkPhysicalDeviceImageFormatInfo2 *>(this); }
    VkPhysicalDeviceImageFormatInfo2 const *ptr() const { return reinterpret_cast<VkPhysicalDeviceImageFormatInfo2 const *>(this); }
};

struct safe_VkQueueFamilyProperties2 {
    VkStructureType sType;
    void* pNext;
    VkQueueFamilyProperties queueFamilyProperties;
    safe_VkQueueFamilyProperties2(const VkQueueFamilyProperties2* in_struct);
    safe_VkQueueFamilyProperties2(const safe_VkQueueFamilyProperties2& src);
    safe_VkQueueFamilyProperties2& operator=(const safe_VkQueueFamilyProperties2& src);
    safe_VkQueueFamilyProperties2();
    ~safe_VkQueueFamilyProperties2();
    void initialize(const VkQueueFamilyProperties2* in_struct);
    void initialize(const safe_VkQueueFamilyProperties2* src);
    VkQueueFamilyProperties2 *ptr() { return reinterpret_cast<VkQueueFamilyProperties2 *>(this); }
    VkQueueFamilyProperties2 const *ptr() const { return reinterpret_cast<VkQueueFamilyProperties2 const *>(this); }
};

struct safe_VkPhysicalDeviceMemoryProperties2 {
    VkStructureType sType;
    void* pNext;
    VkPhysicalDeviceMemoryProperties memoryProperties;
    safe_VkPhysicalDeviceMemoryProperties2(const VkPhysicalDeviceMemoryProperties2* in_struct);
    safe_VkPhysicalDeviceMemoryProperties2(const safe_VkPhysicalDeviceMemoryProperties2& src);
    safe_VkPhysicalDeviceMemoryProperties2& operator=(const safe_VkPhysicalDeviceMemoryProperties2& src);
    safe_VkPhysicalDeviceMemoryProperties2();
    ~safe_VkPhysicalDeviceMemoryProperties2();
    void initialize(const VkPhysicalDeviceMemoryProperties2* in_struct);
    void initialize(const safe_VkPhysicalDeviceMemoryProperties2* src);
    VkPhysicalDeviceMemoryProperties2 *ptr() { return reinterpret_cast<VkPhysicalDeviceMemoryProperties2 *>(this); }
    VkPhysicalDeviceMemoryProperties2 const *ptr() const { return reinterpret_cast<VkPhysicalDeviceMemoryProperties2 const *>(this); }
};

struct safe_VkSparseImageFormatProperties2 {
    VkStructureType sType;
    void* pNext;
    VkSparseImageFormatProperties properties;
    safe_VkSparseImageFormatProperties2(const VkSparseImageFormatProperties2* in_struct);
    safe_VkSparseImageFormatProperties2(const safe_VkSparseImageFormatProperties2& src);
    safe_VkSparseImageFormatProperties2& operator=(const safe_VkSparseImageFormatProperties2& src);
    safe_VkSparseImageFormatProperties2();
    ~safe_VkSparseImageFormatProperties2();
    void initialize(const VkSparseImageFormatProperties2* in_struct);
    void initialize(const safe_VkSparseImageFormatProperties2* src);
    VkSparseImageFormatProperties2 *ptr() { return reinterpret_cast<VkSparseImageFormatProperties2 *>(this); }
    VkSparseImageFormatProperties2 const *ptr() const { return reinterpret_cast<VkSparseImageFormatProperties2 const *>(this); }
};

struct safe_VkPhysicalDeviceSparseImageFormatInfo2 {
    VkStructureType sType;
    const void* pNext;
    VkFormat format;
    VkImageType type;
    VkSampleCountFlagBits samples;
    VkImageUsageFlags usage;
    VkImageTiling tiling;
    safe_VkPhysicalDeviceSparseImageFormatInfo2(const VkPhysicalDeviceSparseImageFormatInfo2* in_struct);
    safe_VkPhysicalDeviceSparseImageFormatInfo2(const safe_VkPhysicalDeviceSparseImageFormatInfo2& src);
    safe_VkPhysicalDeviceSparseImageFormatInfo2& operator=(const safe_VkPhysicalDeviceSparseImageFormatInfo2& src);
    safe_VkPhysicalDeviceSparseImageFormatInfo2();
    ~safe_VkPhysicalDeviceSparseImageFormatInfo2();
    void initialize(const VkPhysicalDeviceSparseImageFormatInfo2* in_struct);
    void initialize(const safe_VkPhysicalDeviceSparseImageFormatInfo2* src);
    VkPhysicalDeviceSparseImageFormatInfo2 *ptr() { return reinterpret_cast<VkPhysicalDeviceSparseImageFormatInfo2 *>(this); }
    VkPhysicalDeviceSparseImageFormatInfo2 const *ptr() const { return reinterpret_cast<VkPhysicalDeviceSparseImageFormatInfo2 const *>(this); }
};

struct safe_VkPhysicalDevicePointClippingProperties {
    VkStructureType sType;
    void* pNext;
    VkPointClippingBehavior pointClippingBehavior;
    safe_VkPhysicalDevicePointClippingProperties(const VkPhysicalDevicePointClippingProperties* in_struct);
    safe_VkPhysicalDevicePointClippingProperties(const safe_VkPhysicalDevicePointClippingProperties& src);
    safe_VkPhysicalDevicePointClippingProperties& operator=(const safe_VkPhysicalDevicePointClippingProperties& src);
    safe_VkPhysicalDevicePointClippingProperties();
    ~safe_VkPhysicalDevicePointClippingProperties();
    void initialize(const VkPhysicalDevicePointClippingProperties* in_struct);
    void initialize(const safe_VkPhysicalDevicePointClippingProperties* src);
    VkPhysicalDevicePointClippingProperties *ptr() { return reinterpret_cast<VkPhysicalDevicePointClippingProperties *>(this); }
    VkPhysicalDevicePointClippingProperties const *ptr() const { return reinterpret_cast<VkPhysicalDevicePointClippingProperties const *>(this); }
};

struct safe_VkRenderPassInputAttachmentAspectCreateInfo {
    VkStructureType sType;
    const void* pNext;
    uint32_t aspectReferenceCount;
    const VkInputAttachmentAspectReference* pAspectReferences;
    safe_VkRenderPassInputAttachmentAspectCreateInfo(const VkRenderPassInputAttachmentAspectCreateInfo* in_struct);
    safe_VkRenderPassInputAttachmentAspectCreateInfo(const safe_VkRenderPassInputAttachmentAspectCreateInfo& src);
    safe_VkRenderPassInputAttachmentAspectCreateInfo& operator=(const safe_VkRenderPassInputAttachmentAspectCreateInfo& src);
    safe_VkRenderPassInputAttachmentAspectCreateInfo();
    ~safe_VkRenderPassInputAttachmentAspectCreateInfo();
    void initialize(const VkRenderPassInputAttachmentAspectCreateInfo* in_struct);
    void initialize(const safe_VkRenderPassInputAttachmentAspectCreateInfo* src);
    VkRenderPassInputAttachmentAspectCreateInfo *ptr() { return reinterpret_cast<VkRenderPassInputAttachmentAspectCreateInfo *>(this); }
    VkRenderPassInputAttachmentAspectCreateInfo const *ptr() const { return reinterpret_cast<VkRenderPassInputAttachmentAspectCreateInfo const *>(this); }
};

struct safe_VkImageViewUsageCreateInfo {
    VkStructureType sType;
    const void* pNext;
    VkImageUsageFlags usage;
    safe_VkImageViewUsageCreateInfo(const VkImageViewUsageCreateInfo* in_struct);
    safe_VkImageViewUsageCreateInfo(const safe_VkImageViewUsageCreateInfo& src);
    safe_VkImageViewUsageCreateInfo& operator=(const safe_VkImageViewUsageCreateInfo& src);
    safe_VkImageViewUsageCreateInfo();
    ~safe_VkImageViewUsageCreateInfo();
    void initialize(const VkImageViewUsageCreateInfo* in_struct);
    void initialize(const safe_VkImageViewUsageCreateInfo* src);
    VkImageViewUsageCreateInfo *ptr() { return reinterpret_cast<VkImageViewUsageCreateInfo *>(this); }
    VkImageViewUsageCreateInfo const *ptr() const { return reinterpret_cast<VkImageViewUsageCreateInfo const *>(this); }
};

struct safe_VkPipelineTessellationDomainOriginStateCreateInfo {
    VkStructureType sType;
    const void* pNext;
    VkTessellationDomainOrigin domainOrigin;
    safe_VkPipelineTessellationDomainOriginStateCreateInfo(const VkPipelineTessellationDomainOriginStateCreateInfo* in_struct);
    safe_VkPipelineTessellationDomainOriginStateCreateInfo(const safe_VkPipelineTessellationDomainOriginStateCreateInfo& src);
    safe_VkPipelineTessellationDomainOriginStateCreateInfo& operator=(const safe_VkPipelineTessellationDomainOriginStateCreateInfo& src);
    safe_VkPipelineTessellationDomainOriginStateCreateInfo();
    ~safe_VkPipelineTessellationDomainOriginStateCreateInfo();
    void initialize(const VkPipelineTessellationDomainOriginStateCreateInfo* in_struct);
    void initialize(const safe_VkPipelineTessellationDomainOriginStateCreateInfo* src);
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
    safe_VkRenderPassMultiviewCreateInfo(const safe_VkRenderPassMultiviewCreateInfo& src);
    safe_VkRenderPassMultiviewCreateInfo& operator=(const safe_VkRenderPassMultiviewCreateInfo& src);
    safe_VkRenderPassMultiviewCreateInfo();
    ~safe_VkRenderPassMultiviewCreateInfo();
    void initialize(const VkRenderPassMultiviewCreateInfo* in_struct);
    void initialize(const safe_VkRenderPassMultiviewCreateInfo* src);
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
    safe_VkPhysicalDeviceMultiviewFeatures(const safe_VkPhysicalDeviceMultiviewFeatures& src);
    safe_VkPhysicalDeviceMultiviewFeatures& operator=(const safe_VkPhysicalDeviceMultiviewFeatures& src);
    safe_VkPhysicalDeviceMultiviewFeatures();
    ~safe_VkPhysicalDeviceMultiviewFeatures();
    void initialize(const VkPhysicalDeviceMultiviewFeatures* in_struct);
    void initialize(const safe_VkPhysicalDeviceMultiviewFeatures* src);
    VkPhysicalDeviceMultiviewFeatures *ptr() { return reinterpret_cast<VkPhysicalDeviceMultiviewFeatures *>(this); }
    VkPhysicalDeviceMultiviewFeatures const *ptr() const { return reinterpret_cast<VkPhysicalDeviceMultiviewFeatures const *>(this); }
};

struct safe_VkPhysicalDeviceMultiviewProperties {
    VkStructureType sType;
    void* pNext;
    uint32_t maxMultiviewViewCount;
    uint32_t maxMultiviewInstanceIndex;
    safe_VkPhysicalDeviceMultiviewProperties(const VkPhysicalDeviceMultiviewProperties* in_struct);
    safe_VkPhysicalDeviceMultiviewProperties(const safe_VkPhysicalDeviceMultiviewProperties& src);
    safe_VkPhysicalDeviceMultiviewProperties& operator=(const safe_VkPhysicalDeviceMultiviewProperties& src);
    safe_VkPhysicalDeviceMultiviewProperties();
    ~safe_VkPhysicalDeviceMultiviewProperties();
    void initialize(const VkPhysicalDeviceMultiviewProperties* in_struct);
    void initialize(const safe_VkPhysicalDeviceMultiviewProperties* src);
    VkPhysicalDeviceMultiviewProperties *ptr() { return reinterpret_cast<VkPhysicalDeviceMultiviewProperties *>(this); }
    VkPhysicalDeviceMultiviewProperties const *ptr() const { return reinterpret_cast<VkPhysicalDeviceMultiviewProperties const *>(this); }
};

struct safe_VkPhysicalDeviceVariablePointersFeatures {
    VkStructureType sType;
    void* pNext;
    VkBool32 variablePointersStorageBuffer;
    VkBool32 variablePointers;
    safe_VkPhysicalDeviceVariablePointersFeatures(const VkPhysicalDeviceVariablePointersFeatures* in_struct);
    safe_VkPhysicalDeviceVariablePointersFeatures(const safe_VkPhysicalDeviceVariablePointersFeatures& src);
    safe_VkPhysicalDeviceVariablePointersFeatures& operator=(const safe_VkPhysicalDeviceVariablePointersFeatures& src);
    safe_VkPhysicalDeviceVariablePointersFeatures();
    ~safe_VkPhysicalDeviceVariablePointersFeatures();
    void initialize(const VkPhysicalDeviceVariablePointersFeatures* in_struct);
    void initialize(const safe_VkPhysicalDeviceVariablePointersFeatures* src);
    VkPhysicalDeviceVariablePointersFeatures *ptr() { return reinterpret_cast<VkPhysicalDeviceVariablePointersFeatures *>(this); }
    VkPhysicalDeviceVariablePointersFeatures const *ptr() const { return reinterpret_cast<VkPhysicalDeviceVariablePointersFeatures const *>(this); }
};

struct safe_VkPhysicalDeviceProtectedMemoryFeatures {
    VkStructureType sType;
    void* pNext;
    VkBool32 protectedMemory;
    safe_VkPhysicalDeviceProtectedMemoryFeatures(const VkPhysicalDeviceProtectedMemoryFeatures* in_struct);
    safe_VkPhysicalDeviceProtectedMemoryFeatures(const safe_VkPhysicalDeviceProtectedMemoryFeatures& src);
    safe_VkPhysicalDeviceProtectedMemoryFeatures& operator=(const safe_VkPhysicalDeviceProtectedMemoryFeatures& src);
    safe_VkPhysicalDeviceProtectedMemoryFeatures();
    ~safe_VkPhysicalDeviceProtectedMemoryFeatures();
    void initialize(const VkPhysicalDeviceProtectedMemoryFeatures* in_struct);
    void initialize(const safe_VkPhysicalDeviceProtectedMemoryFeatures* src);
    VkPhysicalDeviceProtectedMemoryFeatures *ptr() { return reinterpret_cast<VkPhysicalDeviceProtectedMemoryFeatures *>(this); }
    VkPhysicalDeviceProtectedMemoryFeatures const *ptr() const { return reinterpret_cast<VkPhysicalDeviceProtectedMemoryFeatures const *>(this); }
};

struct safe_VkPhysicalDeviceProtectedMemoryProperties {
    VkStructureType sType;
    void* pNext;
    VkBool32 protectedNoFault;
    safe_VkPhysicalDeviceProtectedMemoryProperties(const VkPhysicalDeviceProtectedMemoryProperties* in_struct);
    safe_VkPhysicalDeviceProtectedMemoryProperties(const safe_VkPhysicalDeviceProtectedMemoryProperties& src);
    safe_VkPhysicalDeviceProtectedMemoryProperties& operator=(const safe_VkPhysicalDeviceProtectedMemoryProperties& src);
    safe_VkPhysicalDeviceProtectedMemoryProperties();
    ~safe_VkPhysicalDeviceProtectedMemoryProperties();
    void initialize(const VkPhysicalDeviceProtectedMemoryProperties* in_struct);
    void initialize(const safe_VkPhysicalDeviceProtectedMemoryProperties* src);
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
    safe_VkDeviceQueueInfo2(const safe_VkDeviceQueueInfo2& src);
    safe_VkDeviceQueueInfo2& operator=(const safe_VkDeviceQueueInfo2& src);
    safe_VkDeviceQueueInfo2();
    ~safe_VkDeviceQueueInfo2();
    void initialize(const VkDeviceQueueInfo2* in_struct);
    void initialize(const safe_VkDeviceQueueInfo2* src);
    VkDeviceQueueInfo2 *ptr() { return reinterpret_cast<VkDeviceQueueInfo2 *>(this); }
    VkDeviceQueueInfo2 const *ptr() const { return reinterpret_cast<VkDeviceQueueInfo2 const *>(this); }
};

struct safe_VkProtectedSubmitInfo {
    VkStructureType sType;
    const void* pNext;
    VkBool32 protectedSubmit;
    safe_VkProtectedSubmitInfo(const VkProtectedSubmitInfo* in_struct);
    safe_VkProtectedSubmitInfo(const safe_VkProtectedSubmitInfo& src);
    safe_VkProtectedSubmitInfo& operator=(const safe_VkProtectedSubmitInfo& src);
    safe_VkProtectedSubmitInfo();
    ~safe_VkProtectedSubmitInfo();
    void initialize(const VkProtectedSubmitInfo* in_struct);
    void initialize(const safe_VkProtectedSubmitInfo* src);
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
    safe_VkSamplerYcbcrConversionCreateInfo(const safe_VkSamplerYcbcrConversionCreateInfo& src);
    safe_VkSamplerYcbcrConversionCreateInfo& operator=(const safe_VkSamplerYcbcrConversionCreateInfo& src);
    safe_VkSamplerYcbcrConversionCreateInfo();
    ~safe_VkSamplerYcbcrConversionCreateInfo();
    void initialize(const VkSamplerYcbcrConversionCreateInfo* in_struct);
    void initialize(const safe_VkSamplerYcbcrConversionCreateInfo* src);
    VkSamplerYcbcrConversionCreateInfo *ptr() { return reinterpret_cast<VkSamplerYcbcrConversionCreateInfo *>(this); }
    VkSamplerYcbcrConversionCreateInfo const *ptr() const { return reinterpret_cast<VkSamplerYcbcrConversionCreateInfo const *>(this); }
};

struct safe_VkSamplerYcbcrConversionInfo {
    VkStructureType sType;
    const void* pNext;
    VkSamplerYcbcrConversion conversion;
    safe_VkSamplerYcbcrConversionInfo(const VkSamplerYcbcrConversionInfo* in_struct);
    safe_VkSamplerYcbcrConversionInfo(const safe_VkSamplerYcbcrConversionInfo& src);
    safe_VkSamplerYcbcrConversionInfo& operator=(const safe_VkSamplerYcbcrConversionInfo& src);
    safe_VkSamplerYcbcrConversionInfo();
    ~safe_VkSamplerYcbcrConversionInfo();
    void initialize(const VkSamplerYcbcrConversionInfo* in_struct);
    void initialize(const safe_VkSamplerYcbcrConversionInfo* src);
    VkSamplerYcbcrConversionInfo *ptr() { return reinterpret_cast<VkSamplerYcbcrConversionInfo *>(this); }
    VkSamplerYcbcrConversionInfo const *ptr() const { return reinterpret_cast<VkSamplerYcbcrConversionInfo const *>(this); }
};

struct safe_VkBindImagePlaneMemoryInfo {
    VkStructureType sType;
    const void* pNext;
    VkImageAspectFlagBits planeAspect;
    safe_VkBindImagePlaneMemoryInfo(const VkBindImagePlaneMemoryInfo* in_struct);
    safe_VkBindImagePlaneMemoryInfo(const safe_VkBindImagePlaneMemoryInfo& src);
    safe_VkBindImagePlaneMemoryInfo& operator=(const safe_VkBindImagePlaneMemoryInfo& src);
    safe_VkBindImagePlaneMemoryInfo();
    ~safe_VkBindImagePlaneMemoryInfo();
    void initialize(const VkBindImagePlaneMemoryInfo* in_struct);
    void initialize(const safe_VkBindImagePlaneMemoryInfo* src);
    VkBindImagePlaneMemoryInfo *ptr() { return reinterpret_cast<VkBindImagePlaneMemoryInfo *>(this); }
    VkBindImagePlaneMemoryInfo const *ptr() const { return reinterpret_cast<VkBindImagePlaneMemoryInfo const *>(this); }
};

struct safe_VkImagePlaneMemoryRequirementsInfo {
    VkStructureType sType;
    const void* pNext;
    VkImageAspectFlagBits planeAspect;
    safe_VkImagePlaneMemoryRequirementsInfo(const VkImagePlaneMemoryRequirementsInfo* in_struct);
    safe_VkImagePlaneMemoryRequirementsInfo(const safe_VkImagePlaneMemoryRequirementsInfo& src);
    safe_VkImagePlaneMemoryRequirementsInfo& operator=(const safe_VkImagePlaneMemoryRequirementsInfo& src);
    safe_VkImagePlaneMemoryRequirementsInfo();
    ~safe_VkImagePlaneMemoryRequirementsInfo();
    void initialize(const VkImagePlaneMemoryRequirementsInfo* in_struct);
    void initialize(const safe_VkImagePlaneMemoryRequirementsInfo* src);
    VkImagePlaneMemoryRequirementsInfo *ptr() { return reinterpret_cast<VkImagePlaneMemoryRequirementsInfo *>(this); }
    VkImagePlaneMemoryRequirementsInfo const *ptr() const { return reinterpret_cast<VkImagePlaneMemoryRequirementsInfo const *>(this); }
};

struct safe_VkPhysicalDeviceSamplerYcbcrConversionFeatures {
    VkStructureType sType;
    void* pNext;
    VkBool32 samplerYcbcrConversion;
    safe_VkPhysicalDeviceSamplerYcbcrConversionFeatures(const VkPhysicalDeviceSamplerYcbcrConversionFeatures* in_struct);
    safe_VkPhysicalDeviceSamplerYcbcrConversionFeatures(const safe_VkPhysicalDeviceSamplerYcbcrConversionFeatures& src);
    safe_VkPhysicalDeviceSamplerYcbcrConversionFeatures& operator=(const safe_VkPhysicalDeviceSamplerYcbcrConversionFeatures& src);
    safe_VkPhysicalDeviceSamplerYcbcrConversionFeatures();
    ~safe_VkPhysicalDeviceSamplerYcbcrConversionFeatures();
    void initialize(const VkPhysicalDeviceSamplerYcbcrConversionFeatures* in_struct);
    void initialize(const safe_VkPhysicalDeviceSamplerYcbcrConversionFeatures* src);
    VkPhysicalDeviceSamplerYcbcrConversionFeatures *ptr() { return reinterpret_cast<VkPhysicalDeviceSamplerYcbcrConversionFeatures *>(this); }
    VkPhysicalDeviceSamplerYcbcrConversionFeatures const *ptr() const { return reinterpret_cast<VkPhysicalDeviceSamplerYcbcrConversionFeatures const *>(this); }
};

struct safe_VkSamplerYcbcrConversionImageFormatProperties {
    VkStructureType sType;
    void* pNext;
    uint32_t combinedImageSamplerDescriptorCount;
    safe_VkSamplerYcbcrConversionImageFormatProperties(const VkSamplerYcbcrConversionImageFormatProperties* in_struct);
    safe_VkSamplerYcbcrConversionImageFormatProperties(const safe_VkSamplerYcbcrConversionImageFormatProperties& src);
    safe_VkSamplerYcbcrConversionImageFormatProperties& operator=(const safe_VkSamplerYcbcrConversionImageFormatProperties& src);
    safe_VkSamplerYcbcrConversionImageFormatProperties();
    ~safe_VkSamplerYcbcrConversionImageFormatProperties();
    void initialize(const VkSamplerYcbcrConversionImageFormatProperties* in_struct);
    void initialize(const safe_VkSamplerYcbcrConversionImageFormatProperties* src);
    VkSamplerYcbcrConversionImageFormatProperties *ptr() { return reinterpret_cast<VkSamplerYcbcrConversionImageFormatProperties *>(this); }
    VkSamplerYcbcrConversionImageFormatProperties const *ptr() const { return reinterpret_cast<VkSamplerYcbcrConversionImageFormatProperties const *>(this); }
};

struct safe_VkDescriptorUpdateTemplateCreateInfo {
    VkStructureType sType;
    const void* pNext;
    VkDescriptorUpdateTemplateCreateFlags flags;
    uint32_t descriptorUpdateEntryCount;
    const VkDescriptorUpdateTemplateEntry* pDescriptorUpdateEntries;
    VkDescriptorUpdateTemplateType templateType;
    VkDescriptorSetLayout descriptorSetLayout;
    VkPipelineBindPoint pipelineBindPoint;
    VkPipelineLayout pipelineLayout;
    uint32_t set;
    safe_VkDescriptorUpdateTemplateCreateInfo(const VkDescriptorUpdateTemplateCreateInfo* in_struct);
    safe_VkDescriptorUpdateTemplateCreateInfo(const safe_VkDescriptorUpdateTemplateCreateInfo& src);
    safe_VkDescriptorUpdateTemplateCreateInfo& operator=(const safe_VkDescriptorUpdateTemplateCreateInfo& src);
    safe_VkDescriptorUpdateTemplateCreateInfo();
    ~safe_VkDescriptorUpdateTemplateCreateInfo();
    void initialize(const VkDescriptorUpdateTemplateCreateInfo* in_struct);
    void initialize(const safe_VkDescriptorUpdateTemplateCreateInfo* src);
    VkDescriptorUpdateTemplateCreateInfo *ptr() { return reinterpret_cast<VkDescriptorUpdateTemplateCreateInfo *>(this); }
    VkDescriptorUpdateTemplateCreateInfo const *ptr() const { return reinterpret_cast<VkDescriptorUpdateTemplateCreateInfo const *>(this); }
};

struct safe_VkPhysicalDeviceExternalImageFormatInfo {
    VkStructureType sType;
    const void* pNext;
    VkExternalMemoryHandleTypeFlagBits handleType;
    safe_VkPhysicalDeviceExternalImageFormatInfo(const VkPhysicalDeviceExternalImageFormatInfo* in_struct);
    safe_VkPhysicalDeviceExternalImageFormatInfo(const safe_VkPhysicalDeviceExternalImageFormatInfo& src);
    safe_VkPhysicalDeviceExternalImageFormatInfo& operator=(const safe_VkPhysicalDeviceExternalImageFormatInfo& src);
    safe_VkPhysicalDeviceExternalImageFormatInfo();
    ~safe_VkPhysicalDeviceExternalImageFormatInfo();
    void initialize(const VkPhysicalDeviceExternalImageFormatInfo* in_struct);
    void initialize(const safe_VkPhysicalDeviceExternalImageFormatInfo* src);
    VkPhysicalDeviceExternalImageFormatInfo *ptr() { return reinterpret_cast<VkPhysicalDeviceExternalImageFormatInfo *>(this); }
    VkPhysicalDeviceExternalImageFormatInfo const *ptr() const { return reinterpret_cast<VkPhysicalDeviceExternalImageFormatInfo const *>(this); }
};

struct safe_VkExternalImageFormatProperties {
    VkStructureType sType;
    void* pNext;
    VkExternalMemoryProperties externalMemoryProperties;
    safe_VkExternalImageFormatProperties(const VkExternalImageFormatProperties* in_struct);
    safe_VkExternalImageFormatProperties(const safe_VkExternalImageFormatProperties& src);
    safe_VkExternalImageFormatProperties& operator=(const safe_VkExternalImageFormatProperties& src);
    safe_VkExternalImageFormatProperties();
    ~safe_VkExternalImageFormatProperties();
    void initialize(const VkExternalImageFormatProperties* in_struct);
    void initialize(const safe_VkExternalImageFormatProperties* src);
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
    safe_VkPhysicalDeviceExternalBufferInfo(const safe_VkPhysicalDeviceExternalBufferInfo& src);
    safe_VkPhysicalDeviceExternalBufferInfo& operator=(const safe_VkPhysicalDeviceExternalBufferInfo& src);
    safe_VkPhysicalDeviceExternalBufferInfo();
    ~safe_VkPhysicalDeviceExternalBufferInfo();
    void initialize(const VkPhysicalDeviceExternalBufferInfo* in_struct);
    void initialize(const safe_VkPhysicalDeviceExternalBufferInfo* src);
    VkPhysicalDeviceExternalBufferInfo *ptr() { return reinterpret_cast<VkPhysicalDeviceExternalBufferInfo *>(this); }
    VkPhysicalDeviceExternalBufferInfo const *ptr() const { return reinterpret_cast<VkPhysicalDeviceExternalBufferInfo const *>(this); }
};

struct safe_VkExternalBufferProperties {
    VkStructureType sType;
    void* pNext;
    VkExternalMemoryProperties externalMemoryProperties;
    safe_VkExternalBufferProperties(const VkExternalBufferProperties* in_struct);
    safe_VkExternalBufferProperties(const safe_VkExternalBufferProperties& src);
    safe_VkExternalBufferProperties& operator=(const safe_VkExternalBufferProperties& src);
    safe_VkExternalBufferProperties();
    ~safe_VkExternalBufferProperties();
    void initialize(const VkExternalBufferProperties* in_struct);
    void initialize(const safe_VkExternalBufferProperties* src);
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
    safe_VkPhysicalDeviceIDProperties(const safe_VkPhysicalDeviceIDProperties& src);
    safe_VkPhysicalDeviceIDProperties& operator=(const safe_VkPhysicalDeviceIDProperties& src);
    safe_VkPhysicalDeviceIDProperties();
    ~safe_VkPhysicalDeviceIDProperties();
    void initialize(const VkPhysicalDeviceIDProperties* in_struct);
    void initialize(const safe_VkPhysicalDeviceIDProperties* src);
    VkPhysicalDeviceIDProperties *ptr() { return reinterpret_cast<VkPhysicalDeviceIDProperties *>(this); }
    VkPhysicalDeviceIDProperties const *ptr() const { return reinterpret_cast<VkPhysicalDeviceIDProperties const *>(this); }
};

struct safe_VkExternalMemoryImageCreateInfo {
    VkStructureType sType;
    const void* pNext;
    VkExternalMemoryHandleTypeFlags handleTypes;
    safe_VkExternalMemoryImageCreateInfo(const VkExternalMemoryImageCreateInfo* in_struct);
    safe_VkExternalMemoryImageCreateInfo(const safe_VkExternalMemoryImageCreateInfo& src);
    safe_VkExternalMemoryImageCreateInfo& operator=(const safe_VkExternalMemoryImageCreateInfo& src);
    safe_VkExternalMemoryImageCreateInfo();
    ~safe_VkExternalMemoryImageCreateInfo();
    void initialize(const VkExternalMemoryImageCreateInfo* in_struct);
    void initialize(const safe_VkExternalMemoryImageCreateInfo* src);
    VkExternalMemoryImageCreateInfo *ptr() { return reinterpret_cast<VkExternalMemoryImageCreateInfo *>(this); }
    VkExternalMemoryImageCreateInfo const *ptr() const { return reinterpret_cast<VkExternalMemoryImageCreateInfo const *>(this); }
};

struct safe_VkExternalMemoryBufferCreateInfo {
    VkStructureType sType;
    const void* pNext;
    VkExternalMemoryHandleTypeFlags handleTypes;
    safe_VkExternalMemoryBufferCreateInfo(const VkExternalMemoryBufferCreateInfo* in_struct);
    safe_VkExternalMemoryBufferCreateInfo(const safe_VkExternalMemoryBufferCreateInfo& src);
    safe_VkExternalMemoryBufferCreateInfo& operator=(const safe_VkExternalMemoryBufferCreateInfo& src);
    safe_VkExternalMemoryBufferCreateInfo();
    ~safe_VkExternalMemoryBufferCreateInfo();
    void initialize(const VkExternalMemoryBufferCreateInfo* in_struct);
    void initialize(const safe_VkExternalMemoryBufferCreateInfo* src);
    VkExternalMemoryBufferCreateInfo *ptr() { return reinterpret_cast<VkExternalMemoryBufferCreateInfo *>(this); }
    VkExternalMemoryBufferCreateInfo const *ptr() const { return reinterpret_cast<VkExternalMemoryBufferCreateInfo const *>(this); }
};

struct safe_VkExportMemoryAllocateInfo {
    VkStructureType sType;
    const void* pNext;
    VkExternalMemoryHandleTypeFlags handleTypes;
    safe_VkExportMemoryAllocateInfo(const VkExportMemoryAllocateInfo* in_struct);
    safe_VkExportMemoryAllocateInfo(const safe_VkExportMemoryAllocateInfo& src);
    safe_VkExportMemoryAllocateInfo& operator=(const safe_VkExportMemoryAllocateInfo& src);
    safe_VkExportMemoryAllocateInfo();
    ~safe_VkExportMemoryAllocateInfo();
    void initialize(const VkExportMemoryAllocateInfo* in_struct);
    void initialize(const safe_VkExportMemoryAllocateInfo* src);
    VkExportMemoryAllocateInfo *ptr() { return reinterpret_cast<VkExportMemoryAllocateInfo *>(this); }
    VkExportMemoryAllocateInfo const *ptr() const { return reinterpret_cast<VkExportMemoryAllocateInfo const *>(this); }
};

struct safe_VkPhysicalDeviceExternalFenceInfo {
    VkStructureType sType;
    const void* pNext;
    VkExternalFenceHandleTypeFlagBits handleType;
    safe_VkPhysicalDeviceExternalFenceInfo(const VkPhysicalDeviceExternalFenceInfo* in_struct);
    safe_VkPhysicalDeviceExternalFenceInfo(const safe_VkPhysicalDeviceExternalFenceInfo& src);
    safe_VkPhysicalDeviceExternalFenceInfo& operator=(const safe_VkPhysicalDeviceExternalFenceInfo& src);
    safe_VkPhysicalDeviceExternalFenceInfo();
    ~safe_VkPhysicalDeviceExternalFenceInfo();
    void initialize(const VkPhysicalDeviceExternalFenceInfo* in_struct);
    void initialize(const safe_VkPhysicalDeviceExternalFenceInfo* src);
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
    safe_VkExternalFenceProperties(const safe_VkExternalFenceProperties& src);
    safe_VkExternalFenceProperties& operator=(const safe_VkExternalFenceProperties& src);
    safe_VkExternalFenceProperties();
    ~safe_VkExternalFenceProperties();
    void initialize(const VkExternalFenceProperties* in_struct);
    void initialize(const safe_VkExternalFenceProperties* src);
    VkExternalFenceProperties *ptr() { return reinterpret_cast<VkExternalFenceProperties *>(this); }
    VkExternalFenceProperties const *ptr() const { return reinterpret_cast<VkExternalFenceProperties const *>(this); }
};

struct safe_VkExportFenceCreateInfo {
    VkStructureType sType;
    const void* pNext;
    VkExternalFenceHandleTypeFlags handleTypes;
    safe_VkExportFenceCreateInfo(const VkExportFenceCreateInfo* in_struct);
    safe_VkExportFenceCreateInfo(const safe_VkExportFenceCreateInfo& src);
    safe_VkExportFenceCreateInfo& operator=(const safe_VkExportFenceCreateInfo& src);
    safe_VkExportFenceCreateInfo();
    ~safe_VkExportFenceCreateInfo();
    void initialize(const VkExportFenceCreateInfo* in_struct);
    void initialize(const safe_VkExportFenceCreateInfo* src);
    VkExportFenceCreateInfo *ptr() { return reinterpret_cast<VkExportFenceCreateInfo *>(this); }
    VkExportFenceCreateInfo const *ptr() const { return reinterpret_cast<VkExportFenceCreateInfo const *>(this); }
};

struct safe_VkExportSemaphoreCreateInfo {
    VkStructureType sType;
    const void* pNext;
    VkExternalSemaphoreHandleTypeFlags handleTypes;
    safe_VkExportSemaphoreCreateInfo(const VkExportSemaphoreCreateInfo* in_struct);
    safe_VkExportSemaphoreCreateInfo(const safe_VkExportSemaphoreCreateInfo& src);
    safe_VkExportSemaphoreCreateInfo& operator=(const safe_VkExportSemaphoreCreateInfo& src);
    safe_VkExportSemaphoreCreateInfo();
    ~safe_VkExportSemaphoreCreateInfo();
    void initialize(const VkExportSemaphoreCreateInfo* in_struct);
    void initialize(const safe_VkExportSemaphoreCreateInfo* src);
    VkExportSemaphoreCreateInfo *ptr() { return reinterpret_cast<VkExportSemaphoreCreateInfo *>(this); }
    VkExportSemaphoreCreateInfo const *ptr() const { return reinterpret_cast<VkExportSemaphoreCreateInfo const *>(this); }
};

struct safe_VkPhysicalDeviceExternalSemaphoreInfo {
    VkStructureType sType;
    const void* pNext;
    VkExternalSemaphoreHandleTypeFlagBits handleType;
    safe_VkPhysicalDeviceExternalSemaphoreInfo(const VkPhysicalDeviceExternalSemaphoreInfo* in_struct);
    safe_VkPhysicalDeviceExternalSemaphoreInfo(const safe_VkPhysicalDeviceExternalSemaphoreInfo& src);
    safe_VkPhysicalDeviceExternalSemaphoreInfo& operator=(const safe_VkPhysicalDeviceExternalSemaphoreInfo& src);
    safe_VkPhysicalDeviceExternalSemaphoreInfo();
    ~safe_VkPhysicalDeviceExternalSemaphoreInfo();
    void initialize(const VkPhysicalDeviceExternalSemaphoreInfo* in_struct);
    void initialize(const safe_VkPhysicalDeviceExternalSemaphoreInfo* src);
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
    safe_VkExternalSemaphoreProperties(const safe_VkExternalSemaphoreProperties& src);
    safe_VkExternalSemaphoreProperties& operator=(const safe_VkExternalSemaphoreProperties& src);
    safe_VkExternalSemaphoreProperties();
    ~safe_VkExternalSemaphoreProperties();
    void initialize(const VkExternalSemaphoreProperties* in_struct);
    void initialize(const safe_VkExternalSemaphoreProperties* src);
    VkExternalSemaphoreProperties *ptr() { return reinterpret_cast<VkExternalSemaphoreProperties *>(this); }
    VkExternalSemaphoreProperties const *ptr() const { return reinterpret_cast<VkExternalSemaphoreProperties const *>(this); }
};

struct safe_VkPhysicalDeviceMaintenance3Properties {
    VkStructureType sType;
    void* pNext;
    uint32_t maxPerSetDescriptors;
    VkDeviceSize maxMemoryAllocationSize;
    safe_VkPhysicalDeviceMaintenance3Properties(const VkPhysicalDeviceMaintenance3Properties* in_struct);
    safe_VkPhysicalDeviceMaintenance3Properties(const safe_VkPhysicalDeviceMaintenance3Properties& src);
    safe_VkPhysicalDeviceMaintenance3Properties& operator=(const safe_VkPhysicalDeviceMaintenance3Properties& src);
    safe_VkPhysicalDeviceMaintenance3Properties();
    ~safe_VkPhysicalDeviceMaintenance3Properties();
    void initialize(const VkPhysicalDeviceMaintenance3Properties* in_struct);
    void initialize(const safe_VkPhysicalDeviceMaintenance3Properties* src);
    VkPhysicalDeviceMaintenance3Properties *ptr() { return reinterpret_cast<VkPhysicalDeviceMaintenance3Properties *>(this); }
    VkPhysicalDeviceMaintenance3Properties const *ptr() const { return reinterpret_cast<VkPhysicalDeviceMaintenance3Properties const *>(this); }
};

struct safe_VkDescriptorSetLayoutSupport {
    VkStructureType sType;
    void* pNext;
    VkBool32 supported;
    safe_VkDescriptorSetLayoutSupport(const VkDescriptorSetLayoutSupport* in_struct);
    safe_VkDescriptorSetLayoutSupport(const safe_VkDescriptorSetLayoutSupport& src);
    safe_VkDescriptorSetLayoutSupport& operator=(const safe_VkDescriptorSetLayoutSupport& src);
    safe_VkDescriptorSetLayoutSupport();
    ~safe_VkDescriptorSetLayoutSupport();
    void initialize(const VkDescriptorSetLayoutSupport* in_struct);
    void initialize(const safe_VkDescriptorSetLayoutSupport* src);
    VkDescriptorSetLayoutSupport *ptr() { return reinterpret_cast<VkDescriptorSetLayoutSupport *>(this); }
    VkDescriptorSetLayoutSupport const *ptr() const { return reinterpret_cast<VkDescriptorSetLayoutSupport const *>(this); }
};

struct safe_VkPhysicalDeviceShaderDrawParametersFeatures {
    VkStructureType sType;
    void* pNext;
    VkBool32 shaderDrawParameters;
    safe_VkPhysicalDeviceShaderDrawParametersFeatures(const VkPhysicalDeviceShaderDrawParametersFeatures* in_struct);
    safe_VkPhysicalDeviceShaderDrawParametersFeatures(const safe_VkPhysicalDeviceShaderDrawParametersFeatures& src);
    safe_VkPhysicalDeviceShaderDrawParametersFeatures& operator=(const safe_VkPhysicalDeviceShaderDrawParametersFeatures& src);
    safe_VkPhysicalDeviceShaderDrawParametersFeatures();
    ~safe_VkPhysicalDeviceShaderDrawParametersFeatures();
    void initialize(const VkPhysicalDeviceShaderDrawParametersFeatures* in_struct);
    void initialize(const safe_VkPhysicalDeviceShaderDrawParametersFeatures* src);
    VkPhysicalDeviceShaderDrawParametersFeatures *ptr() { return reinterpret_cast<VkPhysicalDeviceShaderDrawParametersFeatures *>(this); }
    VkPhysicalDeviceShaderDrawParametersFeatures const *ptr() const { return reinterpret_cast<VkPhysicalDeviceShaderDrawParametersFeatures const *>(this); }
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
    safe_VkSwapchainCreateInfoKHR(const safe_VkSwapchainCreateInfoKHR& src);
    safe_VkSwapchainCreateInfoKHR& operator=(const safe_VkSwapchainCreateInfoKHR& src);
    safe_VkSwapchainCreateInfoKHR();
    ~safe_VkSwapchainCreateInfoKHR();
    void initialize(const VkSwapchainCreateInfoKHR* in_struct);
    void initialize(const safe_VkSwapchainCreateInfoKHR* src);
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
    safe_VkPresentInfoKHR(const safe_VkPresentInfoKHR& src);
    safe_VkPresentInfoKHR& operator=(const safe_VkPresentInfoKHR& src);
    safe_VkPresentInfoKHR();
    ~safe_VkPresentInfoKHR();
    void initialize(const VkPresentInfoKHR* in_struct);
    void initialize(const safe_VkPresentInfoKHR* src);
    VkPresentInfoKHR *ptr() { return reinterpret_cast<VkPresentInfoKHR *>(this); }
    VkPresentInfoKHR const *ptr() const { return reinterpret_cast<VkPresentInfoKHR const *>(this); }
};

struct safe_VkImageSwapchainCreateInfoKHR {
    VkStructureType sType;
    const void* pNext;
    VkSwapchainKHR swapchain;
    safe_VkImageSwapchainCreateInfoKHR(const VkImageSwapchainCreateInfoKHR* in_struct);
    safe_VkImageSwapchainCreateInfoKHR(const safe_VkImageSwapchainCreateInfoKHR& src);
    safe_VkImageSwapchainCreateInfoKHR& operator=(const safe_VkImageSwapchainCreateInfoKHR& src);
    safe_VkImageSwapchainCreateInfoKHR();
    ~safe_VkImageSwapchainCreateInfoKHR();
    void initialize(const VkImageSwapchainCreateInfoKHR* in_struct);
    void initialize(const safe_VkImageSwapchainCreateInfoKHR* src);
    VkImageSwapchainCreateInfoKHR *ptr() { return reinterpret_cast<VkImageSwapchainCreateInfoKHR *>(this); }
    VkImageSwapchainCreateInfoKHR const *ptr() const { return reinterpret_cast<VkImageSwapchainCreateInfoKHR const *>(this); }
};

struct safe_VkBindImageMemorySwapchainInfoKHR {
    VkStructureType sType;
    const void* pNext;
    VkSwapchainKHR swapchain;
    uint32_t imageIndex;
    safe_VkBindImageMemorySwapchainInfoKHR(const VkBindImageMemorySwapchainInfoKHR* in_struct);
    safe_VkBindImageMemorySwapchainInfoKHR(const safe_VkBindImageMemorySwapchainInfoKHR& src);
    safe_VkBindImageMemorySwapchainInfoKHR& operator=(const safe_VkBindImageMemorySwapchainInfoKHR& src);
    safe_VkBindImageMemorySwapchainInfoKHR();
    ~safe_VkBindImageMemorySwapchainInfoKHR();
    void initialize(const VkBindImageMemorySwapchainInfoKHR* in_struct);
    void initialize(const safe_VkBindImageMemorySwapchainInfoKHR* src);
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
    safe_VkAcquireNextImageInfoKHR(const safe_VkAcquireNextImageInfoKHR& src);
    safe_VkAcquireNextImageInfoKHR& operator=(const safe_VkAcquireNextImageInfoKHR& src);
    safe_VkAcquireNextImageInfoKHR();
    ~safe_VkAcquireNextImageInfoKHR();
    void initialize(const VkAcquireNextImageInfoKHR* in_struct);
    void initialize(const safe_VkAcquireNextImageInfoKHR* src);
    VkAcquireNextImageInfoKHR *ptr() { return reinterpret_cast<VkAcquireNextImageInfoKHR *>(this); }
    VkAcquireNextImageInfoKHR const *ptr() const { return reinterpret_cast<VkAcquireNextImageInfoKHR const *>(this); }
};

struct safe_VkDeviceGroupPresentCapabilitiesKHR {
    VkStructureType sType;
    const void* pNext;
    uint32_t presentMask[VK_MAX_DEVICE_GROUP_SIZE];
    VkDeviceGroupPresentModeFlagsKHR modes;
    safe_VkDeviceGroupPresentCapabilitiesKHR(const VkDeviceGroupPresentCapabilitiesKHR* in_struct);
    safe_VkDeviceGroupPresentCapabilitiesKHR(const safe_VkDeviceGroupPresentCapabilitiesKHR& src);
    safe_VkDeviceGroupPresentCapabilitiesKHR& operator=(const safe_VkDeviceGroupPresentCapabilitiesKHR& src);
    safe_VkDeviceGroupPresentCapabilitiesKHR();
    ~safe_VkDeviceGroupPresentCapabilitiesKHR();
    void initialize(const VkDeviceGroupPresentCapabilitiesKHR* in_struct);
    void initialize(const safe_VkDeviceGroupPresentCapabilitiesKHR* src);
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
    safe_VkDeviceGroupPresentInfoKHR(const safe_VkDeviceGroupPresentInfoKHR& src);
    safe_VkDeviceGroupPresentInfoKHR& operator=(const safe_VkDeviceGroupPresentInfoKHR& src);
    safe_VkDeviceGroupPresentInfoKHR();
    ~safe_VkDeviceGroupPresentInfoKHR();
    void initialize(const VkDeviceGroupPresentInfoKHR* in_struct);
    void initialize(const safe_VkDeviceGroupPresentInfoKHR* src);
    VkDeviceGroupPresentInfoKHR *ptr() { return reinterpret_cast<VkDeviceGroupPresentInfoKHR *>(this); }
    VkDeviceGroupPresentInfoKHR const *ptr() const { return reinterpret_cast<VkDeviceGroupPresentInfoKHR const *>(this); }
};

struct safe_VkDeviceGroupSwapchainCreateInfoKHR {
    VkStructureType sType;
    const void* pNext;
    VkDeviceGroupPresentModeFlagsKHR modes;
    safe_VkDeviceGroupSwapchainCreateInfoKHR(const VkDeviceGroupSwapchainCreateInfoKHR* in_struct);
    safe_VkDeviceGroupSwapchainCreateInfoKHR(const safe_VkDeviceGroupSwapchainCreateInfoKHR& src);
    safe_VkDeviceGroupSwapchainCreateInfoKHR& operator=(const safe_VkDeviceGroupSwapchainCreateInfoKHR& src);
    safe_VkDeviceGroupSwapchainCreateInfoKHR();
    ~safe_VkDeviceGroupSwapchainCreateInfoKHR();
    void initialize(const VkDeviceGroupSwapchainCreateInfoKHR* in_struct);
    void initialize(const safe_VkDeviceGroupSwapchainCreateInfoKHR* src);
    VkDeviceGroupSwapchainCreateInfoKHR *ptr() { return reinterpret_cast<VkDeviceGroupSwapchainCreateInfoKHR *>(this); }
    VkDeviceGroupSwapchainCreateInfoKHR const *ptr() const { return reinterpret_cast<VkDeviceGroupSwapchainCreateInfoKHR const *>(this); }
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
    safe_VkDisplayPropertiesKHR(const safe_VkDisplayPropertiesKHR& src);
    safe_VkDisplayPropertiesKHR& operator=(const safe_VkDisplayPropertiesKHR& src);
    safe_VkDisplayPropertiesKHR();
    ~safe_VkDisplayPropertiesKHR();
    void initialize(const VkDisplayPropertiesKHR* in_struct);
    void initialize(const safe_VkDisplayPropertiesKHR* src);
    VkDisplayPropertiesKHR *ptr() { return reinterpret_cast<VkDisplayPropertiesKHR *>(this); }
    VkDisplayPropertiesKHR const *ptr() const { return reinterpret_cast<VkDisplayPropertiesKHR const *>(this); }
};

struct safe_VkDisplayModeCreateInfoKHR {
    VkStructureType sType;
    const void* pNext;
    VkDisplayModeCreateFlagsKHR flags;
    VkDisplayModeParametersKHR parameters;
    safe_VkDisplayModeCreateInfoKHR(const VkDisplayModeCreateInfoKHR* in_struct);
    safe_VkDisplayModeCreateInfoKHR(const safe_VkDisplayModeCreateInfoKHR& src);
    safe_VkDisplayModeCreateInfoKHR& operator=(const safe_VkDisplayModeCreateInfoKHR& src);
    safe_VkDisplayModeCreateInfoKHR();
    ~safe_VkDisplayModeCreateInfoKHR();
    void initialize(const VkDisplayModeCreateInfoKHR* in_struct);
    void initialize(const safe_VkDisplayModeCreateInfoKHR* src);
    VkDisplayModeCreateInfoKHR *ptr() { return reinterpret_cast<VkDisplayModeCreateInfoKHR *>(this); }
    VkDisplayModeCreateInfoKHR const *ptr() const { return reinterpret_cast<VkDisplayModeCreateInfoKHR const *>(this); }
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
    safe_VkDisplaySurfaceCreateInfoKHR(const safe_VkDisplaySurfaceCreateInfoKHR& src);
    safe_VkDisplaySurfaceCreateInfoKHR& operator=(const safe_VkDisplaySurfaceCreateInfoKHR& src);
    safe_VkDisplaySurfaceCreateInfoKHR();
    ~safe_VkDisplaySurfaceCreateInfoKHR();
    void initialize(const VkDisplaySurfaceCreateInfoKHR* in_struct);
    void initialize(const safe_VkDisplaySurfaceCreateInfoKHR* src);
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
    safe_VkDisplayPresentInfoKHR(const safe_VkDisplayPresentInfoKHR& src);
    safe_VkDisplayPresentInfoKHR& operator=(const safe_VkDisplayPresentInfoKHR& src);
    safe_VkDisplayPresentInfoKHR();
    ~safe_VkDisplayPresentInfoKHR();
    void initialize(const VkDisplayPresentInfoKHR* in_struct);
    void initialize(const safe_VkDisplayPresentInfoKHR* src);
    VkDisplayPresentInfoKHR *ptr() { return reinterpret_cast<VkDisplayPresentInfoKHR *>(this); }
    VkDisplayPresentInfoKHR const *ptr() const { return reinterpret_cast<VkDisplayPresentInfoKHR const *>(this); }
};

#ifdef VK_USE_PLATFORM_XLIB_KHR
struct safe_VkXlibSurfaceCreateInfoKHR {
    VkStructureType sType;
    const void* pNext;
    VkXlibSurfaceCreateFlagsKHR flags;
    Display* dpy;
    Window window;
    safe_VkXlibSurfaceCreateInfoKHR(const VkXlibSurfaceCreateInfoKHR* in_struct);
    safe_VkXlibSurfaceCreateInfoKHR(const safe_VkXlibSurfaceCreateInfoKHR& src);
    safe_VkXlibSurfaceCreateInfoKHR& operator=(const safe_VkXlibSurfaceCreateInfoKHR& src);
    safe_VkXlibSurfaceCreateInfoKHR();
    ~safe_VkXlibSurfaceCreateInfoKHR();
    void initialize(const VkXlibSurfaceCreateInfoKHR* in_struct);
    void initialize(const safe_VkXlibSurfaceCreateInfoKHR* src);
    VkXlibSurfaceCreateInfoKHR *ptr() { return reinterpret_cast<VkXlibSurfaceCreateInfoKHR *>(this); }
    VkXlibSurfaceCreateInfoKHR const *ptr() const { return reinterpret_cast<VkXlibSurfaceCreateInfoKHR const *>(this); }
};
#endif // VK_USE_PLATFORM_XLIB_KHR

#ifdef VK_USE_PLATFORM_XCB_KHR
struct safe_VkXcbSurfaceCreateInfoKHR {
    VkStructureType sType;
    const void* pNext;
    VkXcbSurfaceCreateFlagsKHR flags;
    xcb_connection_t* connection;
    xcb_window_t window;
    safe_VkXcbSurfaceCreateInfoKHR(const VkXcbSurfaceCreateInfoKHR* in_struct);
    safe_VkXcbSurfaceCreateInfoKHR(const safe_VkXcbSurfaceCreateInfoKHR& src);
    safe_VkXcbSurfaceCreateInfoKHR& operator=(const safe_VkXcbSurfaceCreateInfoKHR& src);
    safe_VkXcbSurfaceCreateInfoKHR();
    ~safe_VkXcbSurfaceCreateInfoKHR();
    void initialize(const VkXcbSurfaceCreateInfoKHR* in_struct);
    void initialize(const safe_VkXcbSurfaceCreateInfoKHR* src);
    VkXcbSurfaceCreateInfoKHR *ptr() { return reinterpret_cast<VkXcbSurfaceCreateInfoKHR *>(this); }
    VkXcbSurfaceCreateInfoKHR const *ptr() const { return reinterpret_cast<VkXcbSurfaceCreateInfoKHR const *>(this); }
};
#endif // VK_USE_PLATFORM_XCB_KHR

#ifdef VK_USE_PLATFORM_WAYLAND_KHR
struct safe_VkWaylandSurfaceCreateInfoKHR {
    VkStructureType sType;
    const void* pNext;
    VkWaylandSurfaceCreateFlagsKHR flags;
    struct wl_display* display;
    struct wl_surface* surface;
    safe_VkWaylandSurfaceCreateInfoKHR(const VkWaylandSurfaceCreateInfoKHR* in_struct);
    safe_VkWaylandSurfaceCreateInfoKHR(const safe_VkWaylandSurfaceCreateInfoKHR& src);
    safe_VkWaylandSurfaceCreateInfoKHR& operator=(const safe_VkWaylandSurfaceCreateInfoKHR& src);
    safe_VkWaylandSurfaceCreateInfoKHR();
    ~safe_VkWaylandSurfaceCreateInfoKHR();
    void initialize(const VkWaylandSurfaceCreateInfoKHR* in_struct);
    void initialize(const safe_VkWaylandSurfaceCreateInfoKHR* src);
    VkWaylandSurfaceCreateInfoKHR *ptr() { return reinterpret_cast<VkWaylandSurfaceCreateInfoKHR *>(this); }
    VkWaylandSurfaceCreateInfoKHR const *ptr() const { return reinterpret_cast<VkWaylandSurfaceCreateInfoKHR const *>(this); }
};
#endif // VK_USE_PLATFORM_WAYLAND_KHR

#ifdef VK_USE_PLATFORM_ANDROID_KHR
struct safe_VkAndroidSurfaceCreateInfoKHR {
    VkStructureType sType;
    const void* pNext;
    VkAndroidSurfaceCreateFlagsKHR flags;
    struct ANativeWindow* window;
    safe_VkAndroidSurfaceCreateInfoKHR(const VkAndroidSurfaceCreateInfoKHR* in_struct);
    safe_VkAndroidSurfaceCreateInfoKHR(const safe_VkAndroidSurfaceCreateInfoKHR& src);
    safe_VkAndroidSurfaceCreateInfoKHR& operator=(const safe_VkAndroidSurfaceCreateInfoKHR& src);
    safe_VkAndroidSurfaceCreateInfoKHR();
    ~safe_VkAndroidSurfaceCreateInfoKHR();
    void initialize(const VkAndroidSurfaceCreateInfoKHR* in_struct);
    void initialize(const safe_VkAndroidSurfaceCreateInfoKHR* src);
    VkAndroidSurfaceCreateInfoKHR *ptr() { return reinterpret_cast<VkAndroidSurfaceCreateInfoKHR *>(this); }
    VkAndroidSurfaceCreateInfoKHR const *ptr() const { return reinterpret_cast<VkAndroidSurfaceCreateInfoKHR const *>(this); }
};
#endif // VK_USE_PLATFORM_ANDROID_KHR

#ifdef VK_USE_PLATFORM_WIN32_KHR
struct safe_VkWin32SurfaceCreateInfoKHR {
    VkStructureType sType;
    const void* pNext;
    VkWin32SurfaceCreateFlagsKHR flags;
    HINSTANCE hinstance;
    HWND hwnd;
    safe_VkWin32SurfaceCreateInfoKHR(const VkWin32SurfaceCreateInfoKHR* in_struct);
    safe_VkWin32SurfaceCreateInfoKHR(const safe_VkWin32SurfaceCreateInfoKHR& src);
    safe_VkWin32SurfaceCreateInfoKHR& operator=(const safe_VkWin32SurfaceCreateInfoKHR& src);
    safe_VkWin32SurfaceCreateInfoKHR();
    ~safe_VkWin32SurfaceCreateInfoKHR();
    void initialize(const VkWin32SurfaceCreateInfoKHR* in_struct);
    void initialize(const safe_VkWin32SurfaceCreateInfoKHR* src);
    VkWin32SurfaceCreateInfoKHR *ptr() { return reinterpret_cast<VkWin32SurfaceCreateInfoKHR *>(this); }
    VkWin32SurfaceCreateInfoKHR const *ptr() const { return reinterpret_cast<VkWin32SurfaceCreateInfoKHR const *>(this); }
};
#endif // VK_USE_PLATFORM_WIN32_KHR

#ifdef VK_USE_PLATFORM_WIN32_KHR
struct safe_VkImportMemoryWin32HandleInfoKHR {
    VkStructureType sType;
    const void* pNext;
    VkExternalMemoryHandleTypeFlagBits handleType;
    HANDLE handle;
    LPCWSTR name;
    safe_VkImportMemoryWin32HandleInfoKHR(const VkImportMemoryWin32HandleInfoKHR* in_struct);
    safe_VkImportMemoryWin32HandleInfoKHR(const safe_VkImportMemoryWin32HandleInfoKHR& src);
    safe_VkImportMemoryWin32HandleInfoKHR& operator=(const safe_VkImportMemoryWin32HandleInfoKHR& src);
    safe_VkImportMemoryWin32HandleInfoKHR();
    ~safe_VkImportMemoryWin32HandleInfoKHR();
    void initialize(const VkImportMemoryWin32HandleInfoKHR* in_struct);
    void initialize(const safe_VkImportMemoryWin32HandleInfoKHR* src);
    VkImportMemoryWin32HandleInfoKHR *ptr() { return reinterpret_cast<VkImportMemoryWin32HandleInfoKHR *>(this); }
    VkImportMemoryWin32HandleInfoKHR const *ptr() const { return reinterpret_cast<VkImportMemoryWin32HandleInfoKHR const *>(this); }
};
#endif // VK_USE_PLATFORM_WIN32_KHR

#ifdef VK_USE_PLATFORM_WIN32_KHR
struct safe_VkExportMemoryWin32HandleInfoKHR {
    VkStructureType sType;
    const void* pNext;
    const SECURITY_ATTRIBUTES* pAttributes;
    DWORD dwAccess;
    LPCWSTR name;
    safe_VkExportMemoryWin32HandleInfoKHR(const VkExportMemoryWin32HandleInfoKHR* in_struct);
    safe_VkExportMemoryWin32HandleInfoKHR(const safe_VkExportMemoryWin32HandleInfoKHR& src);
    safe_VkExportMemoryWin32HandleInfoKHR& operator=(const safe_VkExportMemoryWin32HandleInfoKHR& src);
    safe_VkExportMemoryWin32HandleInfoKHR();
    ~safe_VkExportMemoryWin32HandleInfoKHR();
    void initialize(const VkExportMemoryWin32HandleInfoKHR* in_struct);
    void initialize(const safe_VkExportMemoryWin32HandleInfoKHR* src);
    VkExportMemoryWin32HandleInfoKHR *ptr() { return reinterpret_cast<VkExportMemoryWin32HandleInfoKHR *>(this); }
    VkExportMemoryWin32HandleInfoKHR const *ptr() const { return reinterpret_cast<VkExportMemoryWin32HandleInfoKHR const *>(this); }
};
#endif // VK_USE_PLATFORM_WIN32_KHR

#ifdef VK_USE_PLATFORM_WIN32_KHR
struct safe_VkMemoryWin32HandlePropertiesKHR {
    VkStructureType sType;
    void* pNext;
    uint32_t memoryTypeBits;
    safe_VkMemoryWin32HandlePropertiesKHR(const VkMemoryWin32HandlePropertiesKHR* in_struct);
    safe_VkMemoryWin32HandlePropertiesKHR(const safe_VkMemoryWin32HandlePropertiesKHR& src);
    safe_VkMemoryWin32HandlePropertiesKHR& operator=(const safe_VkMemoryWin32HandlePropertiesKHR& src);
    safe_VkMemoryWin32HandlePropertiesKHR();
    ~safe_VkMemoryWin32HandlePropertiesKHR();
    void initialize(const VkMemoryWin32HandlePropertiesKHR* in_struct);
    void initialize(const safe_VkMemoryWin32HandlePropertiesKHR* src);
    VkMemoryWin32HandlePropertiesKHR *ptr() { return reinterpret_cast<VkMemoryWin32HandlePropertiesKHR *>(this); }
    VkMemoryWin32HandlePropertiesKHR const *ptr() const { return reinterpret_cast<VkMemoryWin32HandlePropertiesKHR const *>(this); }
};
#endif // VK_USE_PLATFORM_WIN32_KHR

#ifdef VK_USE_PLATFORM_WIN32_KHR
struct safe_VkMemoryGetWin32HandleInfoKHR {
    VkStructureType sType;
    const void* pNext;
    VkDeviceMemory memory;
    VkExternalMemoryHandleTypeFlagBits handleType;
    safe_VkMemoryGetWin32HandleInfoKHR(const VkMemoryGetWin32HandleInfoKHR* in_struct);
    safe_VkMemoryGetWin32HandleInfoKHR(const safe_VkMemoryGetWin32HandleInfoKHR& src);
    safe_VkMemoryGetWin32HandleInfoKHR& operator=(const safe_VkMemoryGetWin32HandleInfoKHR& src);
    safe_VkMemoryGetWin32HandleInfoKHR();
    ~safe_VkMemoryGetWin32HandleInfoKHR();
    void initialize(const VkMemoryGetWin32HandleInfoKHR* in_struct);
    void initialize(const safe_VkMemoryGetWin32HandleInfoKHR* src);
    VkMemoryGetWin32HandleInfoKHR *ptr() { return reinterpret_cast<VkMemoryGetWin32HandleInfoKHR *>(this); }
    VkMemoryGetWin32HandleInfoKHR const *ptr() const { return reinterpret_cast<VkMemoryGetWin32HandleInfoKHR const *>(this); }
};
#endif // VK_USE_PLATFORM_WIN32_KHR

struct safe_VkImportMemoryFdInfoKHR {
    VkStructureType sType;
    const void* pNext;
    VkExternalMemoryHandleTypeFlagBits handleType;
    int fd;
    safe_VkImportMemoryFdInfoKHR(const VkImportMemoryFdInfoKHR* in_struct);
    safe_VkImportMemoryFdInfoKHR(const safe_VkImportMemoryFdInfoKHR& src);
    safe_VkImportMemoryFdInfoKHR& operator=(const safe_VkImportMemoryFdInfoKHR& src);
    safe_VkImportMemoryFdInfoKHR();
    ~safe_VkImportMemoryFdInfoKHR();
    void initialize(const VkImportMemoryFdInfoKHR* in_struct);
    void initialize(const safe_VkImportMemoryFdInfoKHR* src);
    VkImportMemoryFdInfoKHR *ptr() { return reinterpret_cast<VkImportMemoryFdInfoKHR *>(this); }
    VkImportMemoryFdInfoKHR const *ptr() const { return reinterpret_cast<VkImportMemoryFdInfoKHR const *>(this); }
};

struct safe_VkMemoryFdPropertiesKHR {
    VkStructureType sType;
    void* pNext;
    uint32_t memoryTypeBits;
    safe_VkMemoryFdPropertiesKHR(const VkMemoryFdPropertiesKHR* in_struct);
    safe_VkMemoryFdPropertiesKHR(const safe_VkMemoryFdPropertiesKHR& src);
    safe_VkMemoryFdPropertiesKHR& operator=(const safe_VkMemoryFdPropertiesKHR& src);
    safe_VkMemoryFdPropertiesKHR();
    ~safe_VkMemoryFdPropertiesKHR();
    void initialize(const VkMemoryFdPropertiesKHR* in_struct);
    void initialize(const safe_VkMemoryFdPropertiesKHR* src);
    VkMemoryFdPropertiesKHR *ptr() { return reinterpret_cast<VkMemoryFdPropertiesKHR *>(this); }
    VkMemoryFdPropertiesKHR const *ptr() const { return reinterpret_cast<VkMemoryFdPropertiesKHR const *>(this); }
};

struct safe_VkMemoryGetFdInfoKHR {
    VkStructureType sType;
    const void* pNext;
    VkDeviceMemory memory;
    VkExternalMemoryHandleTypeFlagBits handleType;
    safe_VkMemoryGetFdInfoKHR(const VkMemoryGetFdInfoKHR* in_struct);
    safe_VkMemoryGetFdInfoKHR(const safe_VkMemoryGetFdInfoKHR& src);
    safe_VkMemoryGetFdInfoKHR& operator=(const safe_VkMemoryGetFdInfoKHR& src);
    safe_VkMemoryGetFdInfoKHR();
    ~safe_VkMemoryGetFdInfoKHR();
    void initialize(const VkMemoryGetFdInfoKHR* in_struct);
    void initialize(const safe_VkMemoryGetFdInfoKHR* src);
    VkMemoryGetFdInfoKHR *ptr() { return reinterpret_cast<VkMemoryGetFdInfoKHR *>(this); }
    VkMemoryGetFdInfoKHR const *ptr() const { return reinterpret_cast<VkMemoryGetFdInfoKHR const *>(this); }
};

#ifdef VK_USE_PLATFORM_WIN32_KHR
struct safe_VkWin32KeyedMutexAcquireReleaseInfoKHR {
    VkStructureType sType;
    const void* pNext;
    uint32_t acquireCount;
    VkDeviceMemory* pAcquireSyncs;
    const uint64_t* pAcquireKeys;
    const uint32_t* pAcquireTimeouts;
    uint32_t releaseCount;
    VkDeviceMemory* pReleaseSyncs;
    const uint64_t* pReleaseKeys;
    safe_VkWin32KeyedMutexAcquireReleaseInfoKHR(const VkWin32KeyedMutexAcquireReleaseInfoKHR* in_struct);
    safe_VkWin32KeyedMutexAcquireReleaseInfoKHR(const safe_VkWin32KeyedMutexAcquireReleaseInfoKHR& src);
    safe_VkWin32KeyedMutexAcquireReleaseInfoKHR& operator=(const safe_VkWin32KeyedMutexAcquireReleaseInfoKHR& src);
    safe_VkWin32KeyedMutexAcquireReleaseInfoKHR();
    ~safe_VkWin32KeyedMutexAcquireReleaseInfoKHR();
    void initialize(const VkWin32KeyedMutexAcquireReleaseInfoKHR* in_struct);
    void initialize(const safe_VkWin32KeyedMutexAcquireReleaseInfoKHR* src);
    VkWin32KeyedMutexAcquireReleaseInfoKHR *ptr() { return reinterpret_cast<VkWin32KeyedMutexAcquireReleaseInfoKHR *>(this); }
    VkWin32KeyedMutexAcquireReleaseInfoKHR const *ptr() const { return reinterpret_cast<VkWin32KeyedMutexAcquireReleaseInfoKHR const *>(this); }
};
#endif // VK_USE_PLATFORM_WIN32_KHR

#ifdef VK_USE_PLATFORM_WIN32_KHR
struct safe_VkImportSemaphoreWin32HandleInfoKHR {
    VkStructureType sType;
    const void* pNext;
    VkSemaphore semaphore;
    VkSemaphoreImportFlags flags;
    VkExternalSemaphoreHandleTypeFlagBits handleType;
    HANDLE handle;
    LPCWSTR name;
    safe_VkImportSemaphoreWin32HandleInfoKHR(const VkImportSemaphoreWin32HandleInfoKHR* in_struct);
    safe_VkImportSemaphoreWin32HandleInfoKHR(const safe_VkImportSemaphoreWin32HandleInfoKHR& src);
    safe_VkImportSemaphoreWin32HandleInfoKHR& operator=(const safe_VkImportSemaphoreWin32HandleInfoKHR& src);
    safe_VkImportSemaphoreWin32HandleInfoKHR();
    ~safe_VkImportSemaphoreWin32HandleInfoKHR();
    void initialize(const VkImportSemaphoreWin32HandleInfoKHR* in_struct);
    void initialize(const safe_VkImportSemaphoreWin32HandleInfoKHR* src);
    VkImportSemaphoreWin32HandleInfoKHR *ptr() { return reinterpret_cast<VkImportSemaphoreWin32HandleInfoKHR *>(this); }
    VkImportSemaphoreWin32HandleInfoKHR const *ptr() const { return reinterpret_cast<VkImportSemaphoreWin32HandleInfoKHR const *>(this); }
};
#endif // VK_USE_PLATFORM_WIN32_KHR

#ifdef VK_USE_PLATFORM_WIN32_KHR
struct safe_VkExportSemaphoreWin32HandleInfoKHR {
    VkStructureType sType;
    const void* pNext;
    const SECURITY_ATTRIBUTES* pAttributes;
    DWORD dwAccess;
    LPCWSTR name;
    safe_VkExportSemaphoreWin32HandleInfoKHR(const VkExportSemaphoreWin32HandleInfoKHR* in_struct);
    safe_VkExportSemaphoreWin32HandleInfoKHR(const safe_VkExportSemaphoreWin32HandleInfoKHR& src);
    safe_VkExportSemaphoreWin32HandleInfoKHR& operator=(const safe_VkExportSemaphoreWin32HandleInfoKHR& src);
    safe_VkExportSemaphoreWin32HandleInfoKHR();
    ~safe_VkExportSemaphoreWin32HandleInfoKHR();
    void initialize(const VkExportSemaphoreWin32HandleInfoKHR* in_struct);
    void initialize(const safe_VkExportSemaphoreWin32HandleInfoKHR* src);
    VkExportSemaphoreWin32HandleInfoKHR *ptr() { return reinterpret_cast<VkExportSemaphoreWin32HandleInfoKHR *>(this); }
    VkExportSemaphoreWin32HandleInfoKHR const *ptr() const { return reinterpret_cast<VkExportSemaphoreWin32HandleInfoKHR const *>(this); }
};
#endif // VK_USE_PLATFORM_WIN32_KHR

#ifdef VK_USE_PLATFORM_WIN32_KHR
struct safe_VkD3D12FenceSubmitInfoKHR {
    VkStructureType sType;
    const void* pNext;
    uint32_t waitSemaphoreValuesCount;
    const uint64_t* pWaitSemaphoreValues;
    uint32_t signalSemaphoreValuesCount;
    const uint64_t* pSignalSemaphoreValues;
    safe_VkD3D12FenceSubmitInfoKHR(const VkD3D12FenceSubmitInfoKHR* in_struct);
    safe_VkD3D12FenceSubmitInfoKHR(const safe_VkD3D12FenceSubmitInfoKHR& src);
    safe_VkD3D12FenceSubmitInfoKHR& operator=(const safe_VkD3D12FenceSubmitInfoKHR& src);
    safe_VkD3D12FenceSubmitInfoKHR();
    ~safe_VkD3D12FenceSubmitInfoKHR();
    void initialize(const VkD3D12FenceSubmitInfoKHR* in_struct);
    void initialize(const safe_VkD3D12FenceSubmitInfoKHR* src);
    VkD3D12FenceSubmitInfoKHR *ptr() { return reinterpret_cast<VkD3D12FenceSubmitInfoKHR *>(this); }
    VkD3D12FenceSubmitInfoKHR const *ptr() const { return reinterpret_cast<VkD3D12FenceSubmitInfoKHR const *>(this); }
};
#endif // VK_USE_PLATFORM_WIN32_KHR

#ifdef VK_USE_PLATFORM_WIN32_KHR
struct safe_VkSemaphoreGetWin32HandleInfoKHR {
    VkStructureType sType;
    const void* pNext;
    VkSemaphore semaphore;
    VkExternalSemaphoreHandleTypeFlagBits handleType;
    safe_VkSemaphoreGetWin32HandleInfoKHR(const VkSemaphoreGetWin32HandleInfoKHR* in_struct);
    safe_VkSemaphoreGetWin32HandleInfoKHR(const safe_VkSemaphoreGetWin32HandleInfoKHR& src);
    safe_VkSemaphoreGetWin32HandleInfoKHR& operator=(const safe_VkSemaphoreGetWin32HandleInfoKHR& src);
    safe_VkSemaphoreGetWin32HandleInfoKHR();
    ~safe_VkSemaphoreGetWin32HandleInfoKHR();
    void initialize(const VkSemaphoreGetWin32HandleInfoKHR* in_struct);
    void initialize(const safe_VkSemaphoreGetWin32HandleInfoKHR* src);
    VkSemaphoreGetWin32HandleInfoKHR *ptr() { return reinterpret_cast<VkSemaphoreGetWin32HandleInfoKHR *>(this); }
    VkSemaphoreGetWin32HandleInfoKHR const *ptr() const { return reinterpret_cast<VkSemaphoreGetWin32HandleInfoKHR const *>(this); }
};
#endif // VK_USE_PLATFORM_WIN32_KHR

struct safe_VkImportSemaphoreFdInfoKHR {
    VkStructureType sType;
    const void* pNext;
    VkSemaphore semaphore;
    VkSemaphoreImportFlags flags;
    VkExternalSemaphoreHandleTypeFlagBits handleType;
    int fd;
    safe_VkImportSemaphoreFdInfoKHR(const VkImportSemaphoreFdInfoKHR* in_struct);
    safe_VkImportSemaphoreFdInfoKHR(const safe_VkImportSemaphoreFdInfoKHR& src);
    safe_VkImportSemaphoreFdInfoKHR& operator=(const safe_VkImportSemaphoreFdInfoKHR& src);
    safe_VkImportSemaphoreFdInfoKHR();
    ~safe_VkImportSemaphoreFdInfoKHR();
    void initialize(const VkImportSemaphoreFdInfoKHR* in_struct);
    void initialize(const safe_VkImportSemaphoreFdInfoKHR* src);
    VkImportSemaphoreFdInfoKHR *ptr() { return reinterpret_cast<VkImportSemaphoreFdInfoKHR *>(this); }
    VkImportSemaphoreFdInfoKHR const *ptr() const { return reinterpret_cast<VkImportSemaphoreFdInfoKHR const *>(this); }
};

struct safe_VkSemaphoreGetFdInfoKHR {
    VkStructureType sType;
    const void* pNext;
    VkSemaphore semaphore;
    VkExternalSemaphoreHandleTypeFlagBits handleType;
    safe_VkSemaphoreGetFdInfoKHR(const VkSemaphoreGetFdInfoKHR* in_struct);
    safe_VkSemaphoreGetFdInfoKHR(const safe_VkSemaphoreGetFdInfoKHR& src);
    safe_VkSemaphoreGetFdInfoKHR& operator=(const safe_VkSemaphoreGetFdInfoKHR& src);
    safe_VkSemaphoreGetFdInfoKHR();
    ~safe_VkSemaphoreGetFdInfoKHR();
    void initialize(const VkSemaphoreGetFdInfoKHR* in_struct);
    void initialize(const safe_VkSemaphoreGetFdInfoKHR* src);
    VkSemaphoreGetFdInfoKHR *ptr() { return reinterpret_cast<VkSemaphoreGetFdInfoKHR *>(this); }
    VkSemaphoreGetFdInfoKHR const *ptr() const { return reinterpret_cast<VkSemaphoreGetFdInfoKHR const *>(this); }
};

struct safe_VkPhysicalDevicePushDescriptorPropertiesKHR {
    VkStructureType sType;
    void* pNext;
    uint32_t maxPushDescriptors;
    safe_VkPhysicalDevicePushDescriptorPropertiesKHR(const VkPhysicalDevicePushDescriptorPropertiesKHR* in_struct);
    safe_VkPhysicalDevicePushDescriptorPropertiesKHR(const safe_VkPhysicalDevicePushDescriptorPropertiesKHR& src);
    safe_VkPhysicalDevicePushDescriptorPropertiesKHR& operator=(const safe_VkPhysicalDevicePushDescriptorPropertiesKHR& src);
    safe_VkPhysicalDevicePushDescriptorPropertiesKHR();
    ~safe_VkPhysicalDevicePushDescriptorPropertiesKHR();
    void initialize(const VkPhysicalDevicePushDescriptorPropertiesKHR* in_struct);
    void initialize(const safe_VkPhysicalDevicePushDescriptorPropertiesKHR* src);
    VkPhysicalDevicePushDescriptorPropertiesKHR *ptr() { return reinterpret_cast<VkPhysicalDevicePushDescriptorPropertiesKHR *>(this); }
    VkPhysicalDevicePushDescriptorPropertiesKHR const *ptr() const { return reinterpret_cast<VkPhysicalDevicePushDescriptorPropertiesKHR const *>(this); }
};

struct safe_VkPhysicalDeviceShaderFloat16Int8FeaturesKHR {
    VkStructureType sType;
    void* pNext;
    VkBool32 shaderFloat16;
    VkBool32 shaderInt8;
    safe_VkPhysicalDeviceShaderFloat16Int8FeaturesKHR(const VkPhysicalDeviceShaderFloat16Int8FeaturesKHR* in_struct);
    safe_VkPhysicalDeviceShaderFloat16Int8FeaturesKHR(const safe_VkPhysicalDeviceShaderFloat16Int8FeaturesKHR& src);
    safe_VkPhysicalDeviceShaderFloat16Int8FeaturesKHR& operator=(const safe_VkPhysicalDeviceShaderFloat16Int8FeaturesKHR& src);
    safe_VkPhysicalDeviceShaderFloat16Int8FeaturesKHR();
    ~safe_VkPhysicalDeviceShaderFloat16Int8FeaturesKHR();
    void initialize(const VkPhysicalDeviceShaderFloat16Int8FeaturesKHR* in_struct);
    void initialize(const safe_VkPhysicalDeviceShaderFloat16Int8FeaturesKHR* src);
    VkPhysicalDeviceShaderFloat16Int8FeaturesKHR *ptr() { return reinterpret_cast<VkPhysicalDeviceShaderFloat16Int8FeaturesKHR *>(this); }
    VkPhysicalDeviceShaderFloat16Int8FeaturesKHR const *ptr() const { return reinterpret_cast<VkPhysicalDeviceShaderFloat16Int8FeaturesKHR const *>(this); }
};

struct safe_VkPresentRegionKHR {
    uint32_t rectangleCount;
    const VkRectLayerKHR* pRectangles;
    safe_VkPresentRegionKHR(const VkPresentRegionKHR* in_struct);
    safe_VkPresentRegionKHR(const safe_VkPresentRegionKHR& src);
    safe_VkPresentRegionKHR& operator=(const safe_VkPresentRegionKHR& src);
    safe_VkPresentRegionKHR();
    ~safe_VkPresentRegionKHR();
    void initialize(const VkPresentRegionKHR* in_struct);
    void initialize(const safe_VkPresentRegionKHR* src);
    VkPresentRegionKHR *ptr() { return reinterpret_cast<VkPresentRegionKHR *>(this); }
    VkPresentRegionKHR const *ptr() const { return reinterpret_cast<VkPresentRegionKHR const *>(this); }
};

struct safe_VkPresentRegionsKHR {
    VkStructureType sType;
    const void* pNext;
    uint32_t swapchainCount;
    safe_VkPresentRegionKHR* pRegions;
    safe_VkPresentRegionsKHR(const VkPresentRegionsKHR* in_struct);
    safe_VkPresentRegionsKHR(const safe_VkPresentRegionsKHR& src);
    safe_VkPresentRegionsKHR& operator=(const safe_VkPresentRegionsKHR& src);
    safe_VkPresentRegionsKHR();
    ~safe_VkPresentRegionsKHR();
    void initialize(const VkPresentRegionsKHR* in_struct);
    void initialize(const safe_VkPresentRegionsKHR* src);
    VkPresentRegionsKHR *ptr() { return reinterpret_cast<VkPresentRegionsKHR *>(this); }
    VkPresentRegionsKHR const *ptr() const { return reinterpret_cast<VkPresentRegionsKHR const *>(this); }
};

struct safe_VkPhysicalDeviceImagelessFramebufferFeaturesKHR {
    VkStructureType sType;
    void* pNext;
    VkBool32 imagelessFramebuffer;
    safe_VkPhysicalDeviceImagelessFramebufferFeaturesKHR(const VkPhysicalDeviceImagelessFramebufferFeaturesKHR* in_struct);
    safe_VkPhysicalDeviceImagelessFramebufferFeaturesKHR(const safe_VkPhysicalDeviceImagelessFramebufferFeaturesKHR& src);
    safe_VkPhysicalDeviceImagelessFramebufferFeaturesKHR& operator=(const safe_VkPhysicalDeviceImagelessFramebufferFeaturesKHR& src);
    safe_VkPhysicalDeviceImagelessFramebufferFeaturesKHR();
    ~safe_VkPhysicalDeviceImagelessFramebufferFeaturesKHR();
    void initialize(const VkPhysicalDeviceImagelessFramebufferFeaturesKHR* in_struct);
    void initialize(const safe_VkPhysicalDeviceImagelessFramebufferFeaturesKHR* src);
    VkPhysicalDeviceImagelessFramebufferFeaturesKHR *ptr() { return reinterpret_cast<VkPhysicalDeviceImagelessFramebufferFeaturesKHR *>(this); }
    VkPhysicalDeviceImagelessFramebufferFeaturesKHR const *ptr() const { return reinterpret_cast<VkPhysicalDeviceImagelessFramebufferFeaturesKHR const *>(this); }
};

struct safe_VkFramebufferAttachmentImageInfoKHR {
    VkStructureType sType;
    const void* pNext;
    VkImageCreateFlags flags;
    VkImageUsageFlags usage;
    uint32_t width;
    uint32_t height;
    uint32_t layerCount;
    uint32_t viewFormatCount;
    const VkFormat* pViewFormats;
    safe_VkFramebufferAttachmentImageInfoKHR(const VkFramebufferAttachmentImageInfoKHR* in_struct);
    safe_VkFramebufferAttachmentImageInfoKHR(const safe_VkFramebufferAttachmentImageInfoKHR& src);
    safe_VkFramebufferAttachmentImageInfoKHR& operator=(const safe_VkFramebufferAttachmentImageInfoKHR& src);
    safe_VkFramebufferAttachmentImageInfoKHR();
    ~safe_VkFramebufferAttachmentImageInfoKHR();
    void initialize(const VkFramebufferAttachmentImageInfoKHR* in_struct);
    void initialize(const safe_VkFramebufferAttachmentImageInfoKHR* src);
    VkFramebufferAttachmentImageInfoKHR *ptr() { return reinterpret_cast<VkFramebufferAttachmentImageInfoKHR *>(this); }
    VkFramebufferAttachmentImageInfoKHR const *ptr() const { return reinterpret_cast<VkFramebufferAttachmentImageInfoKHR const *>(this); }
};

struct safe_VkFramebufferAttachmentsCreateInfoKHR {
    VkStructureType sType;
    const void* pNext;
    uint32_t attachmentImageInfoCount;
    safe_VkFramebufferAttachmentImageInfoKHR* pAttachmentImageInfos;
    safe_VkFramebufferAttachmentsCreateInfoKHR(const VkFramebufferAttachmentsCreateInfoKHR* in_struct);
    safe_VkFramebufferAttachmentsCreateInfoKHR(const safe_VkFramebufferAttachmentsCreateInfoKHR& src);
    safe_VkFramebufferAttachmentsCreateInfoKHR& operator=(const safe_VkFramebufferAttachmentsCreateInfoKHR& src);
    safe_VkFramebufferAttachmentsCreateInfoKHR();
    ~safe_VkFramebufferAttachmentsCreateInfoKHR();
    void initialize(const VkFramebufferAttachmentsCreateInfoKHR* in_struct);
    void initialize(const safe_VkFramebufferAttachmentsCreateInfoKHR* src);
    VkFramebufferAttachmentsCreateInfoKHR *ptr() { return reinterpret_cast<VkFramebufferAttachmentsCreateInfoKHR *>(this); }
    VkFramebufferAttachmentsCreateInfoKHR const *ptr() const { return reinterpret_cast<VkFramebufferAttachmentsCreateInfoKHR const *>(this); }
};

struct safe_VkRenderPassAttachmentBeginInfoKHR {
    VkStructureType sType;
    const void* pNext;
    uint32_t attachmentCount;
    VkImageView* pAttachments;
    safe_VkRenderPassAttachmentBeginInfoKHR(const VkRenderPassAttachmentBeginInfoKHR* in_struct);
    safe_VkRenderPassAttachmentBeginInfoKHR(const safe_VkRenderPassAttachmentBeginInfoKHR& src);
    safe_VkRenderPassAttachmentBeginInfoKHR& operator=(const safe_VkRenderPassAttachmentBeginInfoKHR& src);
    safe_VkRenderPassAttachmentBeginInfoKHR();
    ~safe_VkRenderPassAttachmentBeginInfoKHR();
    void initialize(const VkRenderPassAttachmentBeginInfoKHR* in_struct);
    void initialize(const safe_VkRenderPassAttachmentBeginInfoKHR* src);
    VkRenderPassAttachmentBeginInfoKHR *ptr() { return reinterpret_cast<VkRenderPassAttachmentBeginInfoKHR *>(this); }
    VkRenderPassAttachmentBeginInfoKHR const *ptr() const { return reinterpret_cast<VkRenderPassAttachmentBeginInfoKHR const *>(this); }
};

struct safe_VkAttachmentDescription2KHR {
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
    safe_VkAttachmentDescription2KHR(const VkAttachmentDescription2KHR* in_struct);
    safe_VkAttachmentDescription2KHR(const safe_VkAttachmentDescription2KHR& src);
    safe_VkAttachmentDescription2KHR& operator=(const safe_VkAttachmentDescription2KHR& src);
    safe_VkAttachmentDescription2KHR();
    ~safe_VkAttachmentDescription2KHR();
    void initialize(const VkAttachmentDescription2KHR* in_struct);
    void initialize(const safe_VkAttachmentDescription2KHR* src);
    VkAttachmentDescription2KHR *ptr() { return reinterpret_cast<VkAttachmentDescription2KHR *>(this); }
    VkAttachmentDescription2KHR const *ptr() const { return reinterpret_cast<VkAttachmentDescription2KHR const *>(this); }
};

struct safe_VkAttachmentReference2KHR {
    VkStructureType sType;
    const void* pNext;
    uint32_t attachment;
    VkImageLayout layout;
    VkImageAspectFlags aspectMask;
    safe_VkAttachmentReference2KHR(const VkAttachmentReference2KHR* in_struct);
    safe_VkAttachmentReference2KHR(const safe_VkAttachmentReference2KHR& src);
    safe_VkAttachmentReference2KHR& operator=(const safe_VkAttachmentReference2KHR& src);
    safe_VkAttachmentReference2KHR();
    ~safe_VkAttachmentReference2KHR();
    void initialize(const VkAttachmentReference2KHR* in_struct);
    void initialize(const safe_VkAttachmentReference2KHR* src);
    VkAttachmentReference2KHR *ptr() { return reinterpret_cast<VkAttachmentReference2KHR *>(this); }
    VkAttachmentReference2KHR const *ptr() const { return reinterpret_cast<VkAttachmentReference2KHR const *>(this); }
};

struct safe_VkSubpassDescription2KHR {
    VkStructureType sType;
    const void* pNext;
    VkSubpassDescriptionFlags flags;
    VkPipelineBindPoint pipelineBindPoint;
    uint32_t viewMask;
    uint32_t inputAttachmentCount;
    safe_VkAttachmentReference2KHR* pInputAttachments;
    uint32_t colorAttachmentCount;
    safe_VkAttachmentReference2KHR* pColorAttachments;
    safe_VkAttachmentReference2KHR* pResolveAttachments;
    safe_VkAttachmentReference2KHR* pDepthStencilAttachment;
    uint32_t preserveAttachmentCount;
    const uint32_t* pPreserveAttachments;
    safe_VkSubpassDescription2KHR(const VkSubpassDescription2KHR* in_struct);
    safe_VkSubpassDescription2KHR(const safe_VkSubpassDescription2KHR& src);
    safe_VkSubpassDescription2KHR& operator=(const safe_VkSubpassDescription2KHR& src);
    safe_VkSubpassDescription2KHR();
    ~safe_VkSubpassDescription2KHR();
    void initialize(const VkSubpassDescription2KHR* in_struct);
    void initialize(const safe_VkSubpassDescription2KHR* src);
    VkSubpassDescription2KHR *ptr() { return reinterpret_cast<VkSubpassDescription2KHR *>(this); }
    VkSubpassDescription2KHR const *ptr() const { return reinterpret_cast<VkSubpassDescription2KHR const *>(this); }
};

struct safe_VkSubpassDependency2KHR {
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
    safe_VkSubpassDependency2KHR(const VkSubpassDependency2KHR* in_struct);
    safe_VkSubpassDependency2KHR(const safe_VkSubpassDependency2KHR& src);
    safe_VkSubpassDependency2KHR& operator=(const safe_VkSubpassDependency2KHR& src);
    safe_VkSubpassDependency2KHR();
    ~safe_VkSubpassDependency2KHR();
    void initialize(const VkSubpassDependency2KHR* in_struct);
    void initialize(const safe_VkSubpassDependency2KHR* src);
    VkSubpassDependency2KHR *ptr() { return reinterpret_cast<VkSubpassDependency2KHR *>(this); }
    VkSubpassDependency2KHR const *ptr() const { return reinterpret_cast<VkSubpassDependency2KHR const *>(this); }
};

struct safe_VkRenderPassCreateInfo2KHR {
    VkStructureType sType;
    const void* pNext;
    VkRenderPassCreateFlags flags;
    uint32_t attachmentCount;
    safe_VkAttachmentDescription2KHR* pAttachments;
    uint32_t subpassCount;
    safe_VkSubpassDescription2KHR* pSubpasses;
    uint32_t dependencyCount;
    safe_VkSubpassDependency2KHR* pDependencies;
    uint32_t correlatedViewMaskCount;
    const uint32_t* pCorrelatedViewMasks;
    safe_VkRenderPassCreateInfo2KHR(const VkRenderPassCreateInfo2KHR* in_struct);
    safe_VkRenderPassCreateInfo2KHR(const safe_VkRenderPassCreateInfo2KHR& src);
    safe_VkRenderPassCreateInfo2KHR& operator=(const safe_VkRenderPassCreateInfo2KHR& src);
    safe_VkRenderPassCreateInfo2KHR();
    ~safe_VkRenderPassCreateInfo2KHR();
    void initialize(const VkRenderPassCreateInfo2KHR* in_struct);
    void initialize(const safe_VkRenderPassCreateInfo2KHR* src);
    VkRenderPassCreateInfo2KHR *ptr() { return reinterpret_cast<VkRenderPassCreateInfo2KHR *>(this); }
    VkRenderPassCreateInfo2KHR const *ptr() const { return reinterpret_cast<VkRenderPassCreateInfo2KHR const *>(this); }
};

struct safe_VkSubpassBeginInfoKHR {
    VkStructureType sType;
    const void* pNext;
    VkSubpassContents contents;
    safe_VkSubpassBeginInfoKHR(const VkSubpassBeginInfoKHR* in_struct);
    safe_VkSubpassBeginInfoKHR(const safe_VkSubpassBeginInfoKHR& src);
    safe_VkSubpassBeginInfoKHR& operator=(const safe_VkSubpassBeginInfoKHR& src);
    safe_VkSubpassBeginInfoKHR();
    ~safe_VkSubpassBeginInfoKHR();
    void initialize(const VkSubpassBeginInfoKHR* in_struct);
    void initialize(const safe_VkSubpassBeginInfoKHR* src);
    VkSubpassBeginInfoKHR *ptr() { return reinterpret_cast<VkSubpassBeginInfoKHR *>(this); }
    VkSubpassBeginInfoKHR const *ptr() const { return reinterpret_cast<VkSubpassBeginInfoKHR const *>(this); }
};

struct safe_VkSubpassEndInfoKHR {
    VkStructureType sType;
    const void* pNext;
    safe_VkSubpassEndInfoKHR(const VkSubpassEndInfoKHR* in_struct);
    safe_VkSubpassEndInfoKHR(const safe_VkSubpassEndInfoKHR& src);
    safe_VkSubpassEndInfoKHR& operator=(const safe_VkSubpassEndInfoKHR& src);
    safe_VkSubpassEndInfoKHR();
    ~safe_VkSubpassEndInfoKHR();
    void initialize(const VkSubpassEndInfoKHR* in_struct);
    void initialize(const safe_VkSubpassEndInfoKHR* src);
    VkSubpassEndInfoKHR *ptr() { return reinterpret_cast<VkSubpassEndInfoKHR *>(this); }
    VkSubpassEndInfoKHR const *ptr() const { return reinterpret_cast<VkSubpassEndInfoKHR const *>(this); }
};

struct safe_VkSharedPresentSurfaceCapabilitiesKHR {
    VkStructureType sType;
    void* pNext;
    VkImageUsageFlags sharedPresentSupportedUsageFlags;
    safe_VkSharedPresentSurfaceCapabilitiesKHR(const VkSharedPresentSurfaceCapabilitiesKHR* in_struct);
    safe_VkSharedPresentSurfaceCapabilitiesKHR(const safe_VkSharedPresentSurfaceCapabilitiesKHR& src);
    safe_VkSharedPresentSurfaceCapabilitiesKHR& operator=(const safe_VkSharedPresentSurfaceCapabilitiesKHR& src);
    safe_VkSharedPresentSurfaceCapabilitiesKHR();
    ~safe_VkSharedPresentSurfaceCapabilitiesKHR();
    void initialize(const VkSharedPresentSurfaceCapabilitiesKHR* in_struct);
    void initialize(const safe_VkSharedPresentSurfaceCapabilitiesKHR* src);
    VkSharedPresentSurfaceCapabilitiesKHR *ptr() { return reinterpret_cast<VkSharedPresentSurfaceCapabilitiesKHR *>(this); }
    VkSharedPresentSurfaceCapabilitiesKHR const *ptr() const { return reinterpret_cast<VkSharedPresentSurfaceCapabilitiesKHR const *>(this); }
};

#ifdef VK_USE_PLATFORM_WIN32_KHR
struct safe_VkImportFenceWin32HandleInfoKHR {
    VkStructureType sType;
    const void* pNext;
    VkFence fence;
    VkFenceImportFlags flags;
    VkExternalFenceHandleTypeFlagBits handleType;
    HANDLE handle;
    LPCWSTR name;
    safe_VkImportFenceWin32HandleInfoKHR(const VkImportFenceWin32HandleInfoKHR* in_struct);
    safe_VkImportFenceWin32HandleInfoKHR(const safe_VkImportFenceWin32HandleInfoKHR& src);
    safe_VkImportFenceWin32HandleInfoKHR& operator=(const safe_VkImportFenceWin32HandleInfoKHR& src);
    safe_VkImportFenceWin32HandleInfoKHR();
    ~safe_VkImportFenceWin32HandleInfoKHR();
    void initialize(const VkImportFenceWin32HandleInfoKHR* in_struct);
    void initialize(const safe_VkImportFenceWin32HandleInfoKHR* src);
    VkImportFenceWin32HandleInfoKHR *ptr() { return reinterpret_cast<VkImportFenceWin32HandleInfoKHR *>(this); }
    VkImportFenceWin32HandleInfoKHR const *ptr() const { return reinterpret_cast<VkImportFenceWin32HandleInfoKHR const *>(this); }
};
#endif // VK_USE_PLATFORM_WIN32_KHR

#ifdef VK_USE_PLATFORM_WIN32_KHR
struct safe_VkExportFenceWin32HandleInfoKHR {
    VkStructureType sType;
    const void* pNext;
    const SECURITY_ATTRIBUTES* pAttributes;
    DWORD dwAccess;
    LPCWSTR name;
    safe_VkExportFenceWin32HandleInfoKHR(const VkExportFenceWin32HandleInfoKHR* in_struct);
    safe_VkExportFenceWin32HandleInfoKHR(const safe_VkExportFenceWin32HandleInfoKHR& src);
    safe_VkExportFenceWin32HandleInfoKHR& operator=(const safe_VkExportFenceWin32HandleInfoKHR& src);
    safe_VkExportFenceWin32HandleInfoKHR();
    ~safe_VkExportFenceWin32HandleInfoKHR();
    void initialize(const VkExportFenceWin32HandleInfoKHR* in_struct);
    void initialize(const safe_VkExportFenceWin32HandleInfoKHR* src);
    VkExportFenceWin32HandleInfoKHR *ptr() { return reinterpret_cast<VkExportFenceWin32HandleInfoKHR *>(this); }
    VkExportFenceWin32HandleInfoKHR const *ptr() const { return reinterpret_cast<VkExportFenceWin32HandleInfoKHR const *>(this); }
};
#endif // VK_USE_PLATFORM_WIN32_KHR

#ifdef VK_USE_PLATFORM_WIN32_KHR
struct safe_VkFenceGetWin32HandleInfoKHR {
    VkStructureType sType;
    const void* pNext;
    VkFence fence;
    VkExternalFenceHandleTypeFlagBits handleType;
    safe_VkFenceGetWin32HandleInfoKHR(const VkFenceGetWin32HandleInfoKHR* in_struct);
    safe_VkFenceGetWin32HandleInfoKHR(const safe_VkFenceGetWin32HandleInfoKHR& src);
    safe_VkFenceGetWin32HandleInfoKHR& operator=(const safe_VkFenceGetWin32HandleInfoKHR& src);
    safe_VkFenceGetWin32HandleInfoKHR();
    ~safe_VkFenceGetWin32HandleInfoKHR();
    void initialize(const VkFenceGetWin32HandleInfoKHR* in_struct);
    void initialize(const safe_VkFenceGetWin32HandleInfoKHR* src);
    VkFenceGetWin32HandleInfoKHR *ptr() { return reinterpret_cast<VkFenceGetWin32HandleInfoKHR *>(this); }
    VkFenceGetWin32HandleInfoKHR const *ptr() const { return reinterpret_cast<VkFenceGetWin32HandleInfoKHR const *>(this); }
};
#endif // VK_USE_PLATFORM_WIN32_KHR

struct safe_VkImportFenceFdInfoKHR {
    VkStructureType sType;
    const void* pNext;
    VkFence fence;
    VkFenceImportFlags flags;
    VkExternalFenceHandleTypeFlagBits handleType;
    int fd;
    safe_VkImportFenceFdInfoKHR(const VkImportFenceFdInfoKHR* in_struct);
    safe_VkImportFenceFdInfoKHR(const safe_VkImportFenceFdInfoKHR& src);
    safe_VkImportFenceFdInfoKHR& operator=(const safe_VkImportFenceFdInfoKHR& src);
    safe_VkImportFenceFdInfoKHR();
    ~safe_VkImportFenceFdInfoKHR();
    void initialize(const VkImportFenceFdInfoKHR* in_struct);
    void initialize(const safe_VkImportFenceFdInfoKHR* src);
    VkImportFenceFdInfoKHR *ptr() { return reinterpret_cast<VkImportFenceFdInfoKHR *>(this); }
    VkImportFenceFdInfoKHR const *ptr() const { return reinterpret_cast<VkImportFenceFdInfoKHR const *>(this); }
};

struct safe_VkFenceGetFdInfoKHR {
    VkStructureType sType;
    const void* pNext;
    VkFence fence;
    VkExternalFenceHandleTypeFlagBits handleType;
    safe_VkFenceGetFdInfoKHR(const VkFenceGetFdInfoKHR* in_struct);
    safe_VkFenceGetFdInfoKHR(const safe_VkFenceGetFdInfoKHR& src);
    safe_VkFenceGetFdInfoKHR& operator=(const safe_VkFenceGetFdInfoKHR& src);
    safe_VkFenceGetFdInfoKHR();
    ~safe_VkFenceGetFdInfoKHR();
    void initialize(const VkFenceGetFdInfoKHR* in_struct);
    void initialize(const safe_VkFenceGetFdInfoKHR* src);
    VkFenceGetFdInfoKHR *ptr() { return reinterpret_cast<VkFenceGetFdInfoKHR *>(this); }
    VkFenceGetFdInfoKHR const *ptr() const { return reinterpret_cast<VkFenceGetFdInfoKHR const *>(this); }
};

struct safe_VkPhysicalDeviceSurfaceInfo2KHR {
    VkStructureType sType;
    const void* pNext;
    VkSurfaceKHR surface;
    safe_VkPhysicalDeviceSurfaceInfo2KHR(const VkPhysicalDeviceSurfaceInfo2KHR* in_struct);
    safe_VkPhysicalDeviceSurfaceInfo2KHR(const safe_VkPhysicalDeviceSurfaceInfo2KHR& src);
    safe_VkPhysicalDeviceSurfaceInfo2KHR& operator=(const safe_VkPhysicalDeviceSurfaceInfo2KHR& src);
    safe_VkPhysicalDeviceSurfaceInfo2KHR();
    ~safe_VkPhysicalDeviceSurfaceInfo2KHR();
    void initialize(const VkPhysicalDeviceSurfaceInfo2KHR* in_struct);
    void initialize(const safe_VkPhysicalDeviceSurfaceInfo2KHR* src);
    VkPhysicalDeviceSurfaceInfo2KHR *ptr() { return reinterpret_cast<VkPhysicalDeviceSurfaceInfo2KHR *>(this); }
    VkPhysicalDeviceSurfaceInfo2KHR const *ptr() const { return reinterpret_cast<VkPhysicalDeviceSurfaceInfo2KHR const *>(this); }
};

struct safe_VkSurfaceCapabilities2KHR {
    VkStructureType sType;
    void* pNext;
    VkSurfaceCapabilitiesKHR surfaceCapabilities;
    safe_VkSurfaceCapabilities2KHR(const VkSurfaceCapabilities2KHR* in_struct);
    safe_VkSurfaceCapabilities2KHR(const safe_VkSurfaceCapabilities2KHR& src);
    safe_VkSurfaceCapabilities2KHR& operator=(const safe_VkSurfaceCapabilities2KHR& src);
    safe_VkSurfaceCapabilities2KHR();
    ~safe_VkSurfaceCapabilities2KHR();
    void initialize(const VkSurfaceCapabilities2KHR* in_struct);
    void initialize(const safe_VkSurfaceCapabilities2KHR* src);
    VkSurfaceCapabilities2KHR *ptr() { return reinterpret_cast<VkSurfaceCapabilities2KHR *>(this); }
    VkSurfaceCapabilities2KHR const *ptr() const { return reinterpret_cast<VkSurfaceCapabilities2KHR const *>(this); }
};

struct safe_VkSurfaceFormat2KHR {
    VkStructureType sType;
    void* pNext;
    VkSurfaceFormatKHR surfaceFormat;
    safe_VkSurfaceFormat2KHR(const VkSurfaceFormat2KHR* in_struct);
    safe_VkSurfaceFormat2KHR(const safe_VkSurfaceFormat2KHR& src);
    safe_VkSurfaceFormat2KHR& operator=(const safe_VkSurfaceFormat2KHR& src);
    safe_VkSurfaceFormat2KHR();
    ~safe_VkSurfaceFormat2KHR();
    void initialize(const VkSurfaceFormat2KHR* in_struct);
    void initialize(const safe_VkSurfaceFormat2KHR* src);
    VkSurfaceFormat2KHR *ptr() { return reinterpret_cast<VkSurfaceFormat2KHR *>(this); }
    VkSurfaceFormat2KHR const *ptr() const { return reinterpret_cast<VkSurfaceFormat2KHR const *>(this); }
};

struct safe_VkDisplayProperties2KHR {
    VkStructureType sType;
    void* pNext;
    safe_VkDisplayPropertiesKHR displayProperties;
    safe_VkDisplayProperties2KHR(const VkDisplayProperties2KHR* in_struct);
    safe_VkDisplayProperties2KHR(const safe_VkDisplayProperties2KHR& src);
    safe_VkDisplayProperties2KHR& operator=(const safe_VkDisplayProperties2KHR& src);
    safe_VkDisplayProperties2KHR();
    ~safe_VkDisplayProperties2KHR();
    void initialize(const VkDisplayProperties2KHR* in_struct);
    void initialize(const safe_VkDisplayProperties2KHR* src);
    VkDisplayProperties2KHR *ptr() { return reinterpret_cast<VkDisplayProperties2KHR *>(this); }
    VkDisplayProperties2KHR const *ptr() const { return reinterpret_cast<VkDisplayProperties2KHR const *>(this); }
};

struct safe_VkDisplayPlaneProperties2KHR {
    VkStructureType sType;
    void* pNext;
    VkDisplayPlanePropertiesKHR displayPlaneProperties;
    safe_VkDisplayPlaneProperties2KHR(const VkDisplayPlaneProperties2KHR* in_struct);
    safe_VkDisplayPlaneProperties2KHR(const safe_VkDisplayPlaneProperties2KHR& src);
    safe_VkDisplayPlaneProperties2KHR& operator=(const safe_VkDisplayPlaneProperties2KHR& src);
    safe_VkDisplayPlaneProperties2KHR();
    ~safe_VkDisplayPlaneProperties2KHR();
    void initialize(const VkDisplayPlaneProperties2KHR* in_struct);
    void initialize(const safe_VkDisplayPlaneProperties2KHR* src);
    VkDisplayPlaneProperties2KHR *ptr() { return reinterpret_cast<VkDisplayPlaneProperties2KHR *>(this); }
    VkDisplayPlaneProperties2KHR const *ptr() const { return reinterpret_cast<VkDisplayPlaneProperties2KHR const *>(this); }
};

struct safe_VkDisplayModeProperties2KHR {
    VkStructureType sType;
    void* pNext;
    VkDisplayModePropertiesKHR displayModeProperties;
    safe_VkDisplayModeProperties2KHR(const VkDisplayModeProperties2KHR* in_struct);
    safe_VkDisplayModeProperties2KHR(const safe_VkDisplayModeProperties2KHR& src);
    safe_VkDisplayModeProperties2KHR& operator=(const safe_VkDisplayModeProperties2KHR& src);
    safe_VkDisplayModeProperties2KHR();
    ~safe_VkDisplayModeProperties2KHR();
    void initialize(const VkDisplayModeProperties2KHR* in_struct);
    void initialize(const safe_VkDisplayModeProperties2KHR* src);
    VkDisplayModeProperties2KHR *ptr() { return reinterpret_cast<VkDisplayModeProperties2KHR *>(this); }
    VkDisplayModeProperties2KHR const *ptr() const { return reinterpret_cast<VkDisplayModeProperties2KHR const *>(this); }
};

struct safe_VkDisplayPlaneInfo2KHR {
    VkStructureType sType;
    const void* pNext;
    VkDisplayModeKHR mode;
    uint32_t planeIndex;
    safe_VkDisplayPlaneInfo2KHR(const VkDisplayPlaneInfo2KHR* in_struct);
    safe_VkDisplayPlaneInfo2KHR(const safe_VkDisplayPlaneInfo2KHR& src);
    safe_VkDisplayPlaneInfo2KHR& operator=(const safe_VkDisplayPlaneInfo2KHR& src);
    safe_VkDisplayPlaneInfo2KHR();
    ~safe_VkDisplayPlaneInfo2KHR();
    void initialize(const VkDisplayPlaneInfo2KHR* in_struct);
    void initialize(const safe_VkDisplayPlaneInfo2KHR* src);
    VkDisplayPlaneInfo2KHR *ptr() { return reinterpret_cast<VkDisplayPlaneInfo2KHR *>(this); }
    VkDisplayPlaneInfo2KHR const *ptr() const { return reinterpret_cast<VkDisplayPlaneInfo2KHR const *>(this); }
};

struct safe_VkDisplayPlaneCapabilities2KHR {
    VkStructureType sType;
    void* pNext;
    VkDisplayPlaneCapabilitiesKHR capabilities;
    safe_VkDisplayPlaneCapabilities2KHR(const VkDisplayPlaneCapabilities2KHR* in_struct);
    safe_VkDisplayPlaneCapabilities2KHR(const safe_VkDisplayPlaneCapabilities2KHR& src);
    safe_VkDisplayPlaneCapabilities2KHR& operator=(const safe_VkDisplayPlaneCapabilities2KHR& src);
    safe_VkDisplayPlaneCapabilities2KHR();
    ~safe_VkDisplayPlaneCapabilities2KHR();
    void initialize(const VkDisplayPlaneCapabilities2KHR* in_struct);
    void initialize(const safe_VkDisplayPlaneCapabilities2KHR* src);
    VkDisplayPlaneCapabilities2KHR *ptr() { return reinterpret_cast<VkDisplayPlaneCapabilities2KHR *>(this); }
    VkDisplayPlaneCapabilities2KHR const *ptr() const { return reinterpret_cast<VkDisplayPlaneCapabilities2KHR const *>(this); }
};

struct safe_VkImageFormatListCreateInfoKHR {
    VkStructureType sType;
    const void* pNext;
    uint32_t viewFormatCount;
    const VkFormat* pViewFormats;
    safe_VkImageFormatListCreateInfoKHR(const VkImageFormatListCreateInfoKHR* in_struct);
    safe_VkImageFormatListCreateInfoKHR(const safe_VkImageFormatListCreateInfoKHR& src);
    safe_VkImageFormatListCreateInfoKHR& operator=(const safe_VkImageFormatListCreateInfoKHR& src);
    safe_VkImageFormatListCreateInfoKHR();
    ~safe_VkImageFormatListCreateInfoKHR();
    void initialize(const VkImageFormatListCreateInfoKHR* in_struct);
    void initialize(const safe_VkImageFormatListCreateInfoKHR* src);
    VkImageFormatListCreateInfoKHR *ptr() { return reinterpret_cast<VkImageFormatListCreateInfoKHR *>(this); }
    VkImageFormatListCreateInfoKHR const *ptr() const { return reinterpret_cast<VkImageFormatListCreateInfoKHR const *>(this); }
};

struct safe_VkPhysicalDevice8BitStorageFeaturesKHR {
    VkStructureType sType;
    void* pNext;
    VkBool32 storageBuffer8BitAccess;
    VkBool32 uniformAndStorageBuffer8BitAccess;
    VkBool32 storagePushConstant8;
    safe_VkPhysicalDevice8BitStorageFeaturesKHR(const VkPhysicalDevice8BitStorageFeaturesKHR* in_struct);
    safe_VkPhysicalDevice8BitStorageFeaturesKHR(const safe_VkPhysicalDevice8BitStorageFeaturesKHR& src);
    safe_VkPhysicalDevice8BitStorageFeaturesKHR& operator=(const safe_VkPhysicalDevice8BitStorageFeaturesKHR& src);
    safe_VkPhysicalDevice8BitStorageFeaturesKHR();
    ~safe_VkPhysicalDevice8BitStorageFeaturesKHR();
    void initialize(const VkPhysicalDevice8BitStorageFeaturesKHR* in_struct);
    void initialize(const safe_VkPhysicalDevice8BitStorageFeaturesKHR* src);
    VkPhysicalDevice8BitStorageFeaturesKHR *ptr() { return reinterpret_cast<VkPhysicalDevice8BitStorageFeaturesKHR *>(this); }
    VkPhysicalDevice8BitStorageFeaturesKHR const *ptr() const { return reinterpret_cast<VkPhysicalDevice8BitStorageFeaturesKHR const *>(this); }
};

struct safe_VkPhysicalDeviceShaderAtomicInt64FeaturesKHR {
    VkStructureType sType;
    void* pNext;
    VkBool32 shaderBufferInt64Atomics;
    VkBool32 shaderSharedInt64Atomics;
    safe_VkPhysicalDeviceShaderAtomicInt64FeaturesKHR(const VkPhysicalDeviceShaderAtomicInt64FeaturesKHR* in_struct);
    safe_VkPhysicalDeviceShaderAtomicInt64FeaturesKHR(const safe_VkPhysicalDeviceShaderAtomicInt64FeaturesKHR& src);
    safe_VkPhysicalDeviceShaderAtomicInt64FeaturesKHR& operator=(const safe_VkPhysicalDeviceShaderAtomicInt64FeaturesKHR& src);
    safe_VkPhysicalDeviceShaderAtomicInt64FeaturesKHR();
    ~safe_VkPhysicalDeviceShaderAtomicInt64FeaturesKHR();
    void initialize(const VkPhysicalDeviceShaderAtomicInt64FeaturesKHR* in_struct);
    void initialize(const safe_VkPhysicalDeviceShaderAtomicInt64FeaturesKHR* src);
    VkPhysicalDeviceShaderAtomicInt64FeaturesKHR *ptr() { return reinterpret_cast<VkPhysicalDeviceShaderAtomicInt64FeaturesKHR *>(this); }
    VkPhysicalDeviceShaderAtomicInt64FeaturesKHR const *ptr() const { return reinterpret_cast<VkPhysicalDeviceShaderAtomicInt64FeaturesKHR const *>(this); }
};

struct safe_VkPhysicalDeviceDriverPropertiesKHR {
    VkStructureType sType;
    void* pNext;
    VkDriverIdKHR driverID;
    char driverName[VK_MAX_DRIVER_NAME_SIZE_KHR];
    char driverInfo[VK_MAX_DRIVER_INFO_SIZE_KHR];
    VkConformanceVersionKHR conformanceVersion;
    safe_VkPhysicalDeviceDriverPropertiesKHR(const VkPhysicalDeviceDriverPropertiesKHR* in_struct);
    safe_VkPhysicalDeviceDriverPropertiesKHR(const safe_VkPhysicalDeviceDriverPropertiesKHR& src);
    safe_VkPhysicalDeviceDriverPropertiesKHR& operator=(const safe_VkPhysicalDeviceDriverPropertiesKHR& src);
    safe_VkPhysicalDeviceDriverPropertiesKHR();
    ~safe_VkPhysicalDeviceDriverPropertiesKHR();
    void initialize(const VkPhysicalDeviceDriverPropertiesKHR* in_struct);
    void initialize(const safe_VkPhysicalDeviceDriverPropertiesKHR* src);
    VkPhysicalDeviceDriverPropertiesKHR *ptr() { return reinterpret_cast<VkPhysicalDeviceDriverPropertiesKHR *>(this); }
    VkPhysicalDeviceDriverPropertiesKHR const *ptr() const { return reinterpret_cast<VkPhysicalDeviceDriverPropertiesKHR const *>(this); }
};

struct safe_VkPhysicalDeviceFloatControlsPropertiesKHR {
    VkStructureType sType;
    void* pNext;
    VkShaderFloatControlsIndependenceKHR denormBehaviorIndependence;
    VkShaderFloatControlsIndependenceKHR roundingModeIndependence;
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
    safe_VkPhysicalDeviceFloatControlsPropertiesKHR(const VkPhysicalDeviceFloatControlsPropertiesKHR* in_struct);
    safe_VkPhysicalDeviceFloatControlsPropertiesKHR(const safe_VkPhysicalDeviceFloatControlsPropertiesKHR& src);
    safe_VkPhysicalDeviceFloatControlsPropertiesKHR& operator=(const safe_VkPhysicalDeviceFloatControlsPropertiesKHR& src);
    safe_VkPhysicalDeviceFloatControlsPropertiesKHR();
    ~safe_VkPhysicalDeviceFloatControlsPropertiesKHR();
    void initialize(const VkPhysicalDeviceFloatControlsPropertiesKHR* in_struct);
    void initialize(const safe_VkPhysicalDeviceFloatControlsPropertiesKHR* src);
    VkPhysicalDeviceFloatControlsPropertiesKHR *ptr() { return reinterpret_cast<VkPhysicalDeviceFloatControlsPropertiesKHR *>(this); }
    VkPhysicalDeviceFloatControlsPropertiesKHR const *ptr() const { return reinterpret_cast<VkPhysicalDeviceFloatControlsPropertiesKHR const *>(this); }
};

struct safe_VkSubpassDescriptionDepthStencilResolveKHR {
    VkStructureType sType;
    const void* pNext;
    VkResolveModeFlagBitsKHR depthResolveMode;
    VkResolveModeFlagBitsKHR stencilResolveMode;
    safe_VkAttachmentReference2KHR* pDepthStencilResolveAttachment;
    safe_VkSubpassDescriptionDepthStencilResolveKHR(const VkSubpassDescriptionDepthStencilResolveKHR* in_struct);
    safe_VkSubpassDescriptionDepthStencilResolveKHR(const safe_VkSubpassDescriptionDepthStencilResolveKHR& src);
    safe_VkSubpassDescriptionDepthStencilResolveKHR& operator=(const safe_VkSubpassDescriptionDepthStencilResolveKHR& src);
    safe_VkSubpassDescriptionDepthStencilResolveKHR();
    ~safe_VkSubpassDescriptionDepthStencilResolveKHR();
    void initialize(const VkSubpassDescriptionDepthStencilResolveKHR* in_struct);
    void initialize(const safe_VkSubpassDescriptionDepthStencilResolveKHR* src);
    VkSubpassDescriptionDepthStencilResolveKHR *ptr() { return reinterpret_cast<VkSubpassDescriptionDepthStencilResolveKHR *>(this); }
    VkSubpassDescriptionDepthStencilResolveKHR const *ptr() const { return reinterpret_cast<VkSubpassDescriptionDepthStencilResolveKHR const *>(this); }
};

struct safe_VkPhysicalDeviceDepthStencilResolvePropertiesKHR {
    VkStructureType sType;
    void* pNext;
    VkResolveModeFlagsKHR supportedDepthResolveModes;
    VkResolveModeFlagsKHR supportedStencilResolveModes;
    VkBool32 independentResolveNone;
    VkBool32 independentResolve;
    safe_VkPhysicalDeviceDepthStencilResolvePropertiesKHR(const VkPhysicalDeviceDepthStencilResolvePropertiesKHR* in_struct);
    safe_VkPhysicalDeviceDepthStencilResolvePropertiesKHR(const safe_VkPhysicalDeviceDepthStencilResolvePropertiesKHR& src);
    safe_VkPhysicalDeviceDepthStencilResolvePropertiesKHR& operator=(const safe_VkPhysicalDeviceDepthStencilResolvePropertiesKHR& src);
    safe_VkPhysicalDeviceDepthStencilResolvePropertiesKHR();
    ~safe_VkPhysicalDeviceDepthStencilResolvePropertiesKHR();
    void initialize(const VkPhysicalDeviceDepthStencilResolvePropertiesKHR* in_struct);
    void initialize(const safe_VkPhysicalDeviceDepthStencilResolvePropertiesKHR* src);
    VkPhysicalDeviceDepthStencilResolvePropertiesKHR *ptr() { return reinterpret_cast<VkPhysicalDeviceDepthStencilResolvePropertiesKHR *>(this); }
    VkPhysicalDeviceDepthStencilResolvePropertiesKHR const *ptr() const { return reinterpret_cast<VkPhysicalDeviceDepthStencilResolvePropertiesKHR const *>(this); }
};

struct safe_VkPhysicalDeviceVulkanMemoryModelFeaturesKHR {
    VkStructureType sType;
    void* pNext;
    VkBool32 vulkanMemoryModel;
    VkBool32 vulkanMemoryModelDeviceScope;
    VkBool32 vulkanMemoryModelAvailabilityVisibilityChains;
    safe_VkPhysicalDeviceVulkanMemoryModelFeaturesKHR(const VkPhysicalDeviceVulkanMemoryModelFeaturesKHR* in_struct);
    safe_VkPhysicalDeviceVulkanMemoryModelFeaturesKHR(const safe_VkPhysicalDeviceVulkanMemoryModelFeaturesKHR& src);
    safe_VkPhysicalDeviceVulkanMemoryModelFeaturesKHR& operator=(const safe_VkPhysicalDeviceVulkanMemoryModelFeaturesKHR& src);
    safe_VkPhysicalDeviceVulkanMemoryModelFeaturesKHR();
    ~safe_VkPhysicalDeviceVulkanMemoryModelFeaturesKHR();
    void initialize(const VkPhysicalDeviceVulkanMemoryModelFeaturesKHR* in_struct);
    void initialize(const safe_VkPhysicalDeviceVulkanMemoryModelFeaturesKHR* src);
    VkPhysicalDeviceVulkanMemoryModelFeaturesKHR *ptr() { return reinterpret_cast<VkPhysicalDeviceVulkanMemoryModelFeaturesKHR *>(this); }
    VkPhysicalDeviceVulkanMemoryModelFeaturesKHR const *ptr() const { return reinterpret_cast<VkPhysicalDeviceVulkanMemoryModelFeaturesKHR const *>(this); }
};

struct safe_VkSurfaceProtectedCapabilitiesKHR {
    VkStructureType sType;
    const void* pNext;
    VkBool32 supportsProtected;
    safe_VkSurfaceProtectedCapabilitiesKHR(const VkSurfaceProtectedCapabilitiesKHR* in_struct);
    safe_VkSurfaceProtectedCapabilitiesKHR(const safe_VkSurfaceProtectedCapabilitiesKHR& src);
    safe_VkSurfaceProtectedCapabilitiesKHR& operator=(const safe_VkSurfaceProtectedCapabilitiesKHR& src);
    safe_VkSurfaceProtectedCapabilitiesKHR();
    ~safe_VkSurfaceProtectedCapabilitiesKHR();
    void initialize(const VkSurfaceProtectedCapabilitiesKHR* in_struct);
    void initialize(const safe_VkSurfaceProtectedCapabilitiesKHR* src);
    VkSurfaceProtectedCapabilitiesKHR *ptr() { return reinterpret_cast<VkSurfaceProtectedCapabilitiesKHR *>(this); }
    VkSurfaceProtectedCapabilitiesKHR const *ptr() const { return reinterpret_cast<VkSurfaceProtectedCapabilitiesKHR const *>(this); }
};

struct safe_VkPhysicalDeviceUniformBufferStandardLayoutFeaturesKHR {
    VkStructureType sType;
    void* pNext;
    VkBool32 uniformBufferStandardLayout;
    safe_VkPhysicalDeviceUniformBufferStandardLayoutFeaturesKHR(const VkPhysicalDeviceUniformBufferStandardLayoutFeaturesKHR* in_struct);
    safe_VkPhysicalDeviceUniformBufferStandardLayoutFeaturesKHR(const safe_VkPhysicalDeviceUniformBufferStandardLayoutFeaturesKHR& src);
    safe_VkPhysicalDeviceUniformBufferStandardLayoutFeaturesKHR& operator=(const safe_VkPhysicalDeviceUniformBufferStandardLayoutFeaturesKHR& src);
    safe_VkPhysicalDeviceUniformBufferStandardLayoutFeaturesKHR();
    ~safe_VkPhysicalDeviceUniformBufferStandardLayoutFeaturesKHR();
    void initialize(const VkPhysicalDeviceUniformBufferStandardLayoutFeaturesKHR* in_struct);
    void initialize(const safe_VkPhysicalDeviceUniformBufferStandardLayoutFeaturesKHR* src);
    VkPhysicalDeviceUniformBufferStandardLayoutFeaturesKHR *ptr() { return reinterpret_cast<VkPhysicalDeviceUniformBufferStandardLayoutFeaturesKHR *>(this); }
    VkPhysicalDeviceUniformBufferStandardLayoutFeaturesKHR const *ptr() const { return reinterpret_cast<VkPhysicalDeviceUniformBufferStandardLayoutFeaturesKHR const *>(this); }
};

struct safe_VkPhysicalDevicePipelineExecutablePropertiesFeaturesKHR {
    VkStructureType sType;
    void* pNext;
    VkBool32 pipelineExecutableInfo;
    safe_VkPhysicalDevicePipelineExecutablePropertiesFeaturesKHR(const VkPhysicalDevicePipelineExecutablePropertiesFeaturesKHR* in_struct);
    safe_VkPhysicalDevicePipelineExecutablePropertiesFeaturesKHR(const safe_VkPhysicalDevicePipelineExecutablePropertiesFeaturesKHR& src);
    safe_VkPhysicalDevicePipelineExecutablePropertiesFeaturesKHR& operator=(const safe_VkPhysicalDevicePipelineExecutablePropertiesFeaturesKHR& src);
    safe_VkPhysicalDevicePipelineExecutablePropertiesFeaturesKHR();
    ~safe_VkPhysicalDevicePipelineExecutablePropertiesFeaturesKHR();
    void initialize(const VkPhysicalDevicePipelineExecutablePropertiesFeaturesKHR* in_struct);
    void initialize(const safe_VkPhysicalDevicePipelineExecutablePropertiesFeaturesKHR* src);
    VkPhysicalDevicePipelineExecutablePropertiesFeaturesKHR *ptr() { return reinterpret_cast<VkPhysicalDevicePipelineExecutablePropertiesFeaturesKHR *>(this); }
    VkPhysicalDevicePipelineExecutablePropertiesFeaturesKHR const *ptr() const { return reinterpret_cast<VkPhysicalDevicePipelineExecutablePropertiesFeaturesKHR const *>(this); }
};

struct safe_VkPipelineInfoKHR {
    VkStructureType sType;
    const void* pNext;
    VkPipeline pipeline;
    safe_VkPipelineInfoKHR(const VkPipelineInfoKHR* in_struct);
    safe_VkPipelineInfoKHR(const safe_VkPipelineInfoKHR& src);
    safe_VkPipelineInfoKHR& operator=(const safe_VkPipelineInfoKHR& src);
    safe_VkPipelineInfoKHR();
    ~safe_VkPipelineInfoKHR();
    void initialize(const VkPipelineInfoKHR* in_struct);
    void initialize(const safe_VkPipelineInfoKHR* src);
    VkPipelineInfoKHR *ptr() { return reinterpret_cast<VkPipelineInfoKHR *>(this); }
    VkPipelineInfoKHR const *ptr() const { return reinterpret_cast<VkPipelineInfoKHR const *>(this); }
};

struct safe_VkPipelineExecutablePropertiesKHR {
    VkStructureType sType;
    void* pNext;
    VkShaderStageFlags stages;
    char name[VK_MAX_DESCRIPTION_SIZE];
    char description[VK_MAX_DESCRIPTION_SIZE];
    uint32_t subgroupSize;
    safe_VkPipelineExecutablePropertiesKHR(const VkPipelineExecutablePropertiesKHR* in_struct);
    safe_VkPipelineExecutablePropertiesKHR(const safe_VkPipelineExecutablePropertiesKHR& src);
    safe_VkPipelineExecutablePropertiesKHR& operator=(const safe_VkPipelineExecutablePropertiesKHR& src);
    safe_VkPipelineExecutablePropertiesKHR();
    ~safe_VkPipelineExecutablePropertiesKHR();
    void initialize(const VkPipelineExecutablePropertiesKHR* in_struct);
    void initialize(const safe_VkPipelineExecutablePropertiesKHR* src);
    VkPipelineExecutablePropertiesKHR *ptr() { return reinterpret_cast<VkPipelineExecutablePropertiesKHR *>(this); }
    VkPipelineExecutablePropertiesKHR const *ptr() const { return reinterpret_cast<VkPipelineExecutablePropertiesKHR const *>(this); }
};

struct safe_VkPipelineExecutableInfoKHR {
    VkStructureType sType;
    const void* pNext;
    VkPipeline pipeline;
    uint32_t executableIndex;
    safe_VkPipelineExecutableInfoKHR(const VkPipelineExecutableInfoKHR* in_struct);
    safe_VkPipelineExecutableInfoKHR(const safe_VkPipelineExecutableInfoKHR& src);
    safe_VkPipelineExecutableInfoKHR& operator=(const safe_VkPipelineExecutableInfoKHR& src);
    safe_VkPipelineExecutableInfoKHR();
    ~safe_VkPipelineExecutableInfoKHR();
    void initialize(const VkPipelineExecutableInfoKHR* in_struct);
    void initialize(const safe_VkPipelineExecutableInfoKHR* src);
    VkPipelineExecutableInfoKHR *ptr() { return reinterpret_cast<VkPipelineExecutableInfoKHR *>(this); }
    VkPipelineExecutableInfoKHR const *ptr() const { return reinterpret_cast<VkPipelineExecutableInfoKHR const *>(this); }
};

struct safe_VkPipelineExecutableStatisticKHR {
    VkStructureType sType;
    void* pNext;
    char name[VK_MAX_DESCRIPTION_SIZE];
    char description[VK_MAX_DESCRIPTION_SIZE];
    VkPipelineExecutableStatisticFormatKHR format;
    VkPipelineExecutableStatisticValueKHR value;
    safe_VkPipelineExecutableStatisticKHR(const VkPipelineExecutableStatisticKHR* in_struct);
    safe_VkPipelineExecutableStatisticKHR(const safe_VkPipelineExecutableStatisticKHR& src);
    safe_VkPipelineExecutableStatisticKHR& operator=(const safe_VkPipelineExecutableStatisticKHR& src);
    safe_VkPipelineExecutableStatisticKHR();
    ~safe_VkPipelineExecutableStatisticKHR();
    void initialize(const VkPipelineExecutableStatisticKHR* in_struct);
    void initialize(const safe_VkPipelineExecutableStatisticKHR* src);
    VkPipelineExecutableStatisticKHR *ptr() { return reinterpret_cast<VkPipelineExecutableStatisticKHR *>(this); }
    VkPipelineExecutableStatisticKHR const *ptr() const { return reinterpret_cast<VkPipelineExecutableStatisticKHR const *>(this); }
};

struct safe_VkPipelineExecutableInternalRepresentationKHR {
    VkStructureType sType;
    void* pNext;
    char name[VK_MAX_DESCRIPTION_SIZE];
    char description[VK_MAX_DESCRIPTION_SIZE];
    VkBool32 isText;
    size_t dataSize;
    void* pData;
    safe_VkPipelineExecutableInternalRepresentationKHR(const VkPipelineExecutableInternalRepresentationKHR* in_struct);
    safe_VkPipelineExecutableInternalRepresentationKHR(const safe_VkPipelineExecutableInternalRepresentationKHR& src);
    safe_VkPipelineExecutableInternalRepresentationKHR& operator=(const safe_VkPipelineExecutableInternalRepresentationKHR& src);
    safe_VkPipelineExecutableInternalRepresentationKHR();
    ~safe_VkPipelineExecutableInternalRepresentationKHR();
    void initialize(const VkPipelineExecutableInternalRepresentationKHR* in_struct);
    void initialize(const safe_VkPipelineExecutableInternalRepresentationKHR* src);
    VkPipelineExecutableInternalRepresentationKHR *ptr() { return reinterpret_cast<VkPipelineExecutableInternalRepresentationKHR *>(this); }
    VkPipelineExecutableInternalRepresentationKHR const *ptr() const { return reinterpret_cast<VkPipelineExecutableInternalRepresentationKHR const *>(this); }
};

struct safe_VkDebugReportCallbackCreateInfoEXT {
    VkStructureType sType;
    const void* pNext;
    VkDebugReportFlagsEXT flags;
    PFN_vkDebugReportCallbackEXT pfnCallback;
    void* pUserData;
    safe_VkDebugReportCallbackCreateInfoEXT(const VkDebugReportCallbackCreateInfoEXT* in_struct);
    safe_VkDebugReportCallbackCreateInfoEXT(const safe_VkDebugReportCallbackCreateInfoEXT& src);
    safe_VkDebugReportCallbackCreateInfoEXT& operator=(const safe_VkDebugReportCallbackCreateInfoEXT& src);
    safe_VkDebugReportCallbackCreateInfoEXT();
    ~safe_VkDebugReportCallbackCreateInfoEXT();
    void initialize(const VkDebugReportCallbackCreateInfoEXT* in_struct);
    void initialize(const safe_VkDebugReportCallbackCreateInfoEXT* src);
    VkDebugReportCallbackCreateInfoEXT *ptr() { return reinterpret_cast<VkDebugReportCallbackCreateInfoEXT *>(this); }
    VkDebugReportCallbackCreateInfoEXT const *ptr() const { return reinterpret_cast<VkDebugReportCallbackCreateInfoEXT const *>(this); }
};

struct safe_VkPipelineRasterizationStateRasterizationOrderAMD {
    VkStructureType sType;
    const void* pNext;
    VkRasterizationOrderAMD rasterizationOrder;
    safe_VkPipelineRasterizationStateRasterizationOrderAMD(const VkPipelineRasterizationStateRasterizationOrderAMD* in_struct);
    safe_VkPipelineRasterizationStateRasterizationOrderAMD(const safe_VkPipelineRasterizationStateRasterizationOrderAMD& src);
    safe_VkPipelineRasterizationStateRasterizationOrderAMD& operator=(const safe_VkPipelineRasterizationStateRasterizationOrderAMD& src);
    safe_VkPipelineRasterizationStateRasterizationOrderAMD();
    ~safe_VkPipelineRasterizationStateRasterizationOrderAMD();
    void initialize(const VkPipelineRasterizationStateRasterizationOrderAMD* in_struct);
    void initialize(const safe_VkPipelineRasterizationStateRasterizationOrderAMD* src);
    VkPipelineRasterizationStateRasterizationOrderAMD *ptr() { return reinterpret_cast<VkPipelineRasterizationStateRasterizationOrderAMD *>(this); }
    VkPipelineRasterizationStateRasterizationOrderAMD const *ptr() const { return reinterpret_cast<VkPipelineRasterizationStateRasterizationOrderAMD const *>(this); }
};

struct safe_VkDebugMarkerObjectNameInfoEXT {
    VkStructureType sType;
    const void* pNext;
    VkDebugReportObjectTypeEXT objectType;
    uint64_t object;
    const char* pObjectName;
    safe_VkDebugMarkerObjectNameInfoEXT(const VkDebugMarkerObjectNameInfoEXT* in_struct);
    safe_VkDebugMarkerObjectNameInfoEXT(const safe_VkDebugMarkerObjectNameInfoEXT& src);
    safe_VkDebugMarkerObjectNameInfoEXT& operator=(const safe_VkDebugMarkerObjectNameInfoEXT& src);
    safe_VkDebugMarkerObjectNameInfoEXT();
    ~safe_VkDebugMarkerObjectNameInfoEXT();
    void initialize(const VkDebugMarkerObjectNameInfoEXT* in_struct);
    void initialize(const safe_VkDebugMarkerObjectNameInfoEXT* src);
    VkDebugMarkerObjectNameInfoEXT *ptr() { return reinterpret_cast<VkDebugMarkerObjectNameInfoEXT *>(this); }
    VkDebugMarkerObjectNameInfoEXT const *ptr() const { return reinterpret_cast<VkDebugMarkerObjectNameInfoEXT const *>(this); }
};

struct safe_VkDebugMarkerObjectTagInfoEXT {
    VkStructureType sType;
    const void* pNext;
    VkDebugReportObjectTypeEXT objectType;
    uint64_t object;
    uint64_t tagName;
    size_t tagSize;
    const void* pTag;
    safe_VkDebugMarkerObjectTagInfoEXT(const VkDebugMarkerObjectTagInfoEXT* in_struct);
    safe_VkDebugMarkerObjectTagInfoEXT(const safe_VkDebugMarkerObjectTagInfoEXT& src);
    safe_VkDebugMarkerObjectTagInfoEXT& operator=(const safe_VkDebugMarkerObjectTagInfoEXT& src);
    safe_VkDebugMarkerObjectTagInfoEXT();
    ~safe_VkDebugMarkerObjectTagInfoEXT();
    void initialize(const VkDebugMarkerObjectTagInfoEXT* in_struct);
    void initialize(const safe_VkDebugMarkerObjectTagInfoEXT* src);
    VkDebugMarkerObjectTagInfoEXT *ptr() { return reinterpret_cast<VkDebugMarkerObjectTagInfoEXT *>(this); }
    VkDebugMarkerObjectTagInfoEXT const *ptr() const { return reinterpret_cast<VkDebugMarkerObjectTagInfoEXT const *>(this); }
};

struct safe_VkDebugMarkerMarkerInfoEXT {
    VkStructureType sType;
    const void* pNext;
    const char* pMarkerName;
    float color[4];
    safe_VkDebugMarkerMarkerInfoEXT(const VkDebugMarkerMarkerInfoEXT* in_struct);
    safe_VkDebugMarkerMarkerInfoEXT(const safe_VkDebugMarkerMarkerInfoEXT& src);
    safe_VkDebugMarkerMarkerInfoEXT& operator=(const safe_VkDebugMarkerMarkerInfoEXT& src);
    safe_VkDebugMarkerMarkerInfoEXT();
    ~safe_VkDebugMarkerMarkerInfoEXT();
    void initialize(const VkDebugMarkerMarkerInfoEXT* in_struct);
    void initialize(const safe_VkDebugMarkerMarkerInfoEXT* src);
    VkDebugMarkerMarkerInfoEXT *ptr() { return reinterpret_cast<VkDebugMarkerMarkerInfoEXT *>(this); }
    VkDebugMarkerMarkerInfoEXT const *ptr() const { return reinterpret_cast<VkDebugMarkerMarkerInfoEXT const *>(this); }
};

struct safe_VkDedicatedAllocationImageCreateInfoNV {
    VkStructureType sType;
    const void* pNext;
    VkBool32 dedicatedAllocation;
    safe_VkDedicatedAllocationImageCreateInfoNV(const VkDedicatedAllocationImageCreateInfoNV* in_struct);
    safe_VkDedicatedAllocationImageCreateInfoNV(const safe_VkDedicatedAllocationImageCreateInfoNV& src);
    safe_VkDedicatedAllocationImageCreateInfoNV& operator=(const safe_VkDedicatedAllocationImageCreateInfoNV& src);
    safe_VkDedicatedAllocationImageCreateInfoNV();
    ~safe_VkDedicatedAllocationImageCreateInfoNV();
    void initialize(const VkDedicatedAllocationImageCreateInfoNV* in_struct);
    void initialize(const safe_VkDedicatedAllocationImageCreateInfoNV* src);
    VkDedicatedAllocationImageCreateInfoNV *ptr() { return reinterpret_cast<VkDedicatedAllocationImageCreateInfoNV *>(this); }
    VkDedicatedAllocationImageCreateInfoNV const *ptr() const { return reinterpret_cast<VkDedicatedAllocationImageCreateInfoNV const *>(this); }
};

struct safe_VkDedicatedAllocationBufferCreateInfoNV {
    VkStructureType sType;
    const void* pNext;
    VkBool32 dedicatedAllocation;
    safe_VkDedicatedAllocationBufferCreateInfoNV(const VkDedicatedAllocationBufferCreateInfoNV* in_struct);
    safe_VkDedicatedAllocationBufferCreateInfoNV(const safe_VkDedicatedAllocationBufferCreateInfoNV& src);
    safe_VkDedicatedAllocationBufferCreateInfoNV& operator=(const safe_VkDedicatedAllocationBufferCreateInfoNV& src);
    safe_VkDedicatedAllocationBufferCreateInfoNV();
    ~safe_VkDedicatedAllocationBufferCreateInfoNV();
    void initialize(const VkDedicatedAllocationBufferCreateInfoNV* in_struct);
    void initialize(const safe_VkDedicatedAllocationBufferCreateInfoNV* src);
    VkDedicatedAllocationBufferCreateInfoNV *ptr() { return reinterpret_cast<VkDedicatedAllocationBufferCreateInfoNV *>(this); }
    VkDedicatedAllocationBufferCreateInfoNV const *ptr() const { return reinterpret_cast<VkDedicatedAllocationBufferCreateInfoNV const *>(this); }
};

struct safe_VkDedicatedAllocationMemoryAllocateInfoNV {
    VkStructureType sType;
    const void* pNext;
    VkImage image;
    VkBuffer buffer;
    safe_VkDedicatedAllocationMemoryAllocateInfoNV(const VkDedicatedAllocationMemoryAllocateInfoNV* in_struct);
    safe_VkDedicatedAllocationMemoryAllocateInfoNV(const safe_VkDedicatedAllocationMemoryAllocateInfoNV& src);
    safe_VkDedicatedAllocationMemoryAllocateInfoNV& operator=(const safe_VkDedicatedAllocationMemoryAllocateInfoNV& src);
    safe_VkDedicatedAllocationMemoryAllocateInfoNV();
    ~safe_VkDedicatedAllocationMemoryAllocateInfoNV();
    void initialize(const VkDedicatedAllocationMemoryAllocateInfoNV* in_struct);
    void initialize(const safe_VkDedicatedAllocationMemoryAllocateInfoNV* src);
    VkDedicatedAllocationMemoryAllocateInfoNV *ptr() { return reinterpret_cast<VkDedicatedAllocationMemoryAllocateInfoNV *>(this); }
    VkDedicatedAllocationMemoryAllocateInfoNV const *ptr() const { return reinterpret_cast<VkDedicatedAllocationMemoryAllocateInfoNV const *>(this); }
};

struct safe_VkPhysicalDeviceTransformFeedbackFeaturesEXT {
    VkStructureType sType;
    void* pNext;
    VkBool32 transformFeedback;
    VkBool32 geometryStreams;
    safe_VkPhysicalDeviceTransformFeedbackFeaturesEXT(const VkPhysicalDeviceTransformFeedbackFeaturesEXT* in_struct);
    safe_VkPhysicalDeviceTransformFeedbackFeaturesEXT(const safe_VkPhysicalDeviceTransformFeedbackFeaturesEXT& src);
    safe_VkPhysicalDeviceTransformFeedbackFeaturesEXT& operator=(const safe_VkPhysicalDeviceTransformFeedbackFeaturesEXT& src);
    safe_VkPhysicalDeviceTransformFeedbackFeaturesEXT();
    ~safe_VkPhysicalDeviceTransformFeedbackFeaturesEXT();
    void initialize(const VkPhysicalDeviceTransformFeedbackFeaturesEXT* in_struct);
    void initialize(const safe_VkPhysicalDeviceTransformFeedbackFeaturesEXT* src);
    VkPhysicalDeviceTransformFeedbackFeaturesEXT *ptr() { return reinterpret_cast<VkPhysicalDeviceTransformFeedbackFeaturesEXT *>(this); }
    VkPhysicalDeviceTransformFeedbackFeaturesEXT const *ptr() const { return reinterpret_cast<VkPhysicalDeviceTransformFeedbackFeaturesEXT const *>(this); }
};

struct safe_VkPhysicalDeviceTransformFeedbackPropertiesEXT {
    VkStructureType sType;
    void* pNext;
    uint32_t maxTransformFeedbackStreams;
    uint32_t maxTransformFeedbackBuffers;
    VkDeviceSize maxTransformFeedbackBufferSize;
    uint32_t maxTransformFeedbackStreamDataSize;
    uint32_t maxTransformFeedbackBufferDataSize;
    uint32_t maxTransformFeedbackBufferDataStride;
    VkBool32 transformFeedbackQueries;
    VkBool32 transformFeedbackStreamsLinesTriangles;
    VkBool32 transformFeedbackRasterizationStreamSelect;
    VkBool32 transformFeedbackDraw;
    safe_VkPhysicalDeviceTransformFeedbackPropertiesEXT(const VkPhysicalDeviceTransformFeedbackPropertiesEXT* in_struct);
    safe_VkPhysicalDeviceTransformFeedbackPropertiesEXT(const safe_VkPhysicalDeviceTransformFeedbackPropertiesEXT& src);
    safe_VkPhysicalDeviceTransformFeedbackPropertiesEXT& operator=(const safe_VkPhysicalDeviceTransformFeedbackPropertiesEXT& src);
    safe_VkPhysicalDeviceTransformFeedbackPropertiesEXT();
    ~safe_VkPhysicalDeviceTransformFeedbackPropertiesEXT();
    void initialize(const VkPhysicalDeviceTransformFeedbackPropertiesEXT* in_struct);
    void initialize(const safe_VkPhysicalDeviceTransformFeedbackPropertiesEXT* src);
    VkPhysicalDeviceTransformFeedbackPropertiesEXT *ptr() { return reinterpret_cast<VkPhysicalDeviceTransformFeedbackPropertiesEXT *>(this); }
    VkPhysicalDeviceTransformFeedbackPropertiesEXT const *ptr() const { return reinterpret_cast<VkPhysicalDeviceTransformFeedbackPropertiesEXT const *>(this); }
};

struct safe_VkPipelineRasterizationStateStreamCreateInfoEXT {
    VkStructureType sType;
    const void* pNext;
    VkPipelineRasterizationStateStreamCreateFlagsEXT flags;
    uint32_t rasterizationStream;
    safe_VkPipelineRasterizationStateStreamCreateInfoEXT(const VkPipelineRasterizationStateStreamCreateInfoEXT* in_struct);
    safe_VkPipelineRasterizationStateStreamCreateInfoEXT(const safe_VkPipelineRasterizationStateStreamCreateInfoEXT& src);
    safe_VkPipelineRasterizationStateStreamCreateInfoEXT& operator=(const safe_VkPipelineRasterizationStateStreamCreateInfoEXT& src);
    safe_VkPipelineRasterizationStateStreamCreateInfoEXT();
    ~safe_VkPipelineRasterizationStateStreamCreateInfoEXT();
    void initialize(const VkPipelineRasterizationStateStreamCreateInfoEXT* in_struct);
    void initialize(const safe_VkPipelineRasterizationStateStreamCreateInfoEXT* src);
    VkPipelineRasterizationStateStreamCreateInfoEXT *ptr() { return reinterpret_cast<VkPipelineRasterizationStateStreamCreateInfoEXT *>(this); }
    VkPipelineRasterizationStateStreamCreateInfoEXT const *ptr() const { return reinterpret_cast<VkPipelineRasterizationStateStreamCreateInfoEXT const *>(this); }
};

struct safe_VkImageViewHandleInfoNVX {
    VkStructureType sType;
    const void* pNext;
    VkImageView imageView;
    VkDescriptorType descriptorType;
    VkSampler sampler;
    safe_VkImageViewHandleInfoNVX(const VkImageViewHandleInfoNVX* in_struct);
    safe_VkImageViewHandleInfoNVX(const safe_VkImageViewHandleInfoNVX& src);
    safe_VkImageViewHandleInfoNVX& operator=(const safe_VkImageViewHandleInfoNVX& src);
    safe_VkImageViewHandleInfoNVX();
    ~safe_VkImageViewHandleInfoNVX();
    void initialize(const VkImageViewHandleInfoNVX* in_struct);
    void initialize(const safe_VkImageViewHandleInfoNVX* src);
    VkImageViewHandleInfoNVX *ptr() { return reinterpret_cast<VkImageViewHandleInfoNVX *>(this); }
    VkImageViewHandleInfoNVX const *ptr() const { return reinterpret_cast<VkImageViewHandleInfoNVX const *>(this); }
};

struct safe_VkTextureLODGatherFormatPropertiesAMD {
    VkStructureType sType;
    void* pNext;
    VkBool32 supportsTextureGatherLODBiasAMD;
    safe_VkTextureLODGatherFormatPropertiesAMD(const VkTextureLODGatherFormatPropertiesAMD* in_struct);
    safe_VkTextureLODGatherFormatPropertiesAMD(const safe_VkTextureLODGatherFormatPropertiesAMD& src);
    safe_VkTextureLODGatherFormatPropertiesAMD& operator=(const safe_VkTextureLODGatherFormatPropertiesAMD& src);
    safe_VkTextureLODGatherFormatPropertiesAMD();
    ~safe_VkTextureLODGatherFormatPropertiesAMD();
    void initialize(const VkTextureLODGatherFormatPropertiesAMD* in_struct);
    void initialize(const safe_VkTextureLODGatherFormatPropertiesAMD* src);
    VkTextureLODGatherFormatPropertiesAMD *ptr() { return reinterpret_cast<VkTextureLODGatherFormatPropertiesAMD *>(this); }
    VkTextureLODGatherFormatPropertiesAMD const *ptr() const { return reinterpret_cast<VkTextureLODGatherFormatPropertiesAMD const *>(this); }
};

#ifdef VK_USE_PLATFORM_GGP
struct safe_VkStreamDescriptorSurfaceCreateInfoGGP {
    VkStructureType sType;
    const void* pNext;
    VkStreamDescriptorSurfaceCreateFlagsGGP flags;
    GgpStreamDescriptor streamDescriptor;
    safe_VkStreamDescriptorSurfaceCreateInfoGGP(const VkStreamDescriptorSurfaceCreateInfoGGP* in_struct);
    safe_VkStreamDescriptorSurfaceCreateInfoGGP(const safe_VkStreamDescriptorSurfaceCreateInfoGGP& src);
    safe_VkStreamDescriptorSurfaceCreateInfoGGP& operator=(const safe_VkStreamDescriptorSurfaceCreateInfoGGP& src);
    safe_VkStreamDescriptorSurfaceCreateInfoGGP();
    ~safe_VkStreamDescriptorSurfaceCreateInfoGGP();
    void initialize(const VkStreamDescriptorSurfaceCreateInfoGGP* in_struct);
    void initialize(const safe_VkStreamDescriptorSurfaceCreateInfoGGP* src);
    VkStreamDescriptorSurfaceCreateInfoGGP *ptr() { return reinterpret_cast<VkStreamDescriptorSurfaceCreateInfoGGP *>(this); }
    VkStreamDescriptorSurfaceCreateInfoGGP const *ptr() const { return reinterpret_cast<VkStreamDescriptorSurfaceCreateInfoGGP const *>(this); }
};
#endif // VK_USE_PLATFORM_GGP

struct safe_VkPhysicalDeviceCornerSampledImageFeaturesNV {
    VkStructureType sType;
    void* pNext;
    VkBool32 cornerSampledImage;
    safe_VkPhysicalDeviceCornerSampledImageFeaturesNV(const VkPhysicalDeviceCornerSampledImageFeaturesNV* in_struct);
    safe_VkPhysicalDeviceCornerSampledImageFeaturesNV(const safe_VkPhysicalDeviceCornerSampledImageFeaturesNV& src);
    safe_VkPhysicalDeviceCornerSampledImageFeaturesNV& operator=(const safe_VkPhysicalDeviceCornerSampledImageFeaturesNV& src);
    safe_VkPhysicalDeviceCornerSampledImageFeaturesNV();
    ~safe_VkPhysicalDeviceCornerSampledImageFeaturesNV();
    void initialize(const VkPhysicalDeviceCornerSampledImageFeaturesNV* in_struct);
    void initialize(const safe_VkPhysicalDeviceCornerSampledImageFeaturesNV* src);
    VkPhysicalDeviceCornerSampledImageFeaturesNV *ptr() { return reinterpret_cast<VkPhysicalDeviceCornerSampledImageFeaturesNV *>(this); }
    VkPhysicalDeviceCornerSampledImageFeaturesNV const *ptr() const { return reinterpret_cast<VkPhysicalDeviceCornerSampledImageFeaturesNV const *>(this); }
};

struct safe_VkExternalMemoryImageCreateInfoNV {
    VkStructureType sType;
    const void* pNext;
    VkExternalMemoryHandleTypeFlagsNV handleTypes;
    safe_VkExternalMemoryImageCreateInfoNV(const VkExternalMemoryImageCreateInfoNV* in_struct);
    safe_VkExternalMemoryImageCreateInfoNV(const safe_VkExternalMemoryImageCreateInfoNV& src);
    safe_VkExternalMemoryImageCreateInfoNV& operator=(const safe_VkExternalMemoryImageCreateInfoNV& src);
    safe_VkExternalMemoryImageCreateInfoNV();
    ~safe_VkExternalMemoryImageCreateInfoNV();
    void initialize(const VkExternalMemoryImageCreateInfoNV* in_struct);
    void initialize(const safe_VkExternalMemoryImageCreateInfoNV* src);
    VkExternalMemoryImageCreateInfoNV *ptr() { return reinterpret_cast<VkExternalMemoryImageCreateInfoNV *>(this); }
    VkExternalMemoryImageCreateInfoNV const *ptr() const { return reinterpret_cast<VkExternalMemoryImageCreateInfoNV const *>(this); }
};

struct safe_VkExportMemoryAllocateInfoNV {
    VkStructureType sType;
    const void* pNext;
    VkExternalMemoryHandleTypeFlagsNV handleTypes;
    safe_VkExportMemoryAllocateInfoNV(const VkExportMemoryAllocateInfoNV* in_struct);
    safe_VkExportMemoryAllocateInfoNV(const safe_VkExportMemoryAllocateInfoNV& src);
    safe_VkExportMemoryAllocateInfoNV& operator=(const safe_VkExportMemoryAllocateInfoNV& src);
    safe_VkExportMemoryAllocateInfoNV();
    ~safe_VkExportMemoryAllocateInfoNV();
    void initialize(const VkExportMemoryAllocateInfoNV* in_struct);
    void initialize(const safe_VkExportMemoryAllocateInfoNV* src);
    VkExportMemoryAllocateInfoNV *ptr() { return reinterpret_cast<VkExportMemoryAllocateInfoNV *>(this); }
    VkExportMemoryAllocateInfoNV const *ptr() const { return reinterpret_cast<VkExportMemoryAllocateInfoNV const *>(this); }
};

#ifdef VK_USE_PLATFORM_WIN32_KHR
struct safe_VkImportMemoryWin32HandleInfoNV {
    VkStructureType sType;
    const void* pNext;
    VkExternalMemoryHandleTypeFlagsNV handleType;
    HANDLE handle;
    safe_VkImportMemoryWin32HandleInfoNV(const VkImportMemoryWin32HandleInfoNV* in_struct);
    safe_VkImportMemoryWin32HandleInfoNV(const safe_VkImportMemoryWin32HandleInfoNV& src);
    safe_VkImportMemoryWin32HandleInfoNV& operator=(const safe_VkImportMemoryWin32HandleInfoNV& src);
    safe_VkImportMemoryWin32HandleInfoNV();
    ~safe_VkImportMemoryWin32HandleInfoNV();
    void initialize(const VkImportMemoryWin32HandleInfoNV* in_struct);
    void initialize(const safe_VkImportMemoryWin32HandleInfoNV* src);
    VkImportMemoryWin32HandleInfoNV *ptr() { return reinterpret_cast<VkImportMemoryWin32HandleInfoNV *>(this); }
    VkImportMemoryWin32HandleInfoNV const *ptr() const { return reinterpret_cast<VkImportMemoryWin32HandleInfoNV const *>(this); }
};
#endif // VK_USE_PLATFORM_WIN32_KHR

#ifdef VK_USE_PLATFORM_WIN32_KHR
struct safe_VkExportMemoryWin32HandleInfoNV {
    VkStructureType sType;
    const void* pNext;
    const SECURITY_ATTRIBUTES* pAttributes;
    DWORD dwAccess;
    safe_VkExportMemoryWin32HandleInfoNV(const VkExportMemoryWin32HandleInfoNV* in_struct);
    safe_VkExportMemoryWin32HandleInfoNV(const safe_VkExportMemoryWin32HandleInfoNV& src);
    safe_VkExportMemoryWin32HandleInfoNV& operator=(const safe_VkExportMemoryWin32HandleInfoNV& src);
    safe_VkExportMemoryWin32HandleInfoNV();
    ~safe_VkExportMemoryWin32HandleInfoNV();
    void initialize(const VkExportMemoryWin32HandleInfoNV* in_struct);
    void initialize(const safe_VkExportMemoryWin32HandleInfoNV* src);
    VkExportMemoryWin32HandleInfoNV *ptr() { return reinterpret_cast<VkExportMemoryWin32HandleInfoNV *>(this); }
    VkExportMemoryWin32HandleInfoNV const *ptr() const { return reinterpret_cast<VkExportMemoryWin32HandleInfoNV const *>(this); }
};
#endif // VK_USE_PLATFORM_WIN32_KHR

#ifdef VK_USE_PLATFORM_WIN32_KHR
struct safe_VkWin32KeyedMutexAcquireReleaseInfoNV {
    VkStructureType sType;
    const void* pNext;
    uint32_t acquireCount;
    VkDeviceMemory* pAcquireSyncs;
    const uint64_t* pAcquireKeys;
    const uint32_t* pAcquireTimeoutMilliseconds;
    uint32_t releaseCount;
    VkDeviceMemory* pReleaseSyncs;
    const uint64_t* pReleaseKeys;
    safe_VkWin32KeyedMutexAcquireReleaseInfoNV(const VkWin32KeyedMutexAcquireReleaseInfoNV* in_struct);
    safe_VkWin32KeyedMutexAcquireReleaseInfoNV(const safe_VkWin32KeyedMutexAcquireReleaseInfoNV& src);
    safe_VkWin32KeyedMutexAcquireReleaseInfoNV& operator=(const safe_VkWin32KeyedMutexAcquireReleaseInfoNV& src);
    safe_VkWin32KeyedMutexAcquireReleaseInfoNV();
    ~safe_VkWin32KeyedMutexAcquireReleaseInfoNV();
    void initialize(const VkWin32KeyedMutexAcquireReleaseInfoNV* in_struct);
    void initialize(const safe_VkWin32KeyedMutexAcquireReleaseInfoNV* src);
    VkWin32KeyedMutexAcquireReleaseInfoNV *ptr() { return reinterpret_cast<VkWin32KeyedMutexAcquireReleaseInfoNV *>(this); }
    VkWin32KeyedMutexAcquireReleaseInfoNV const *ptr() const { return reinterpret_cast<VkWin32KeyedMutexAcquireReleaseInfoNV const *>(this); }
};
#endif // VK_USE_PLATFORM_WIN32_KHR

struct safe_VkValidationFlagsEXT {
    VkStructureType sType;
    const void* pNext;
    uint32_t disabledValidationCheckCount;
    const VkValidationCheckEXT* pDisabledValidationChecks;
    safe_VkValidationFlagsEXT(const VkValidationFlagsEXT* in_struct);
    safe_VkValidationFlagsEXT(const safe_VkValidationFlagsEXT& src);
    safe_VkValidationFlagsEXT& operator=(const safe_VkValidationFlagsEXT& src);
    safe_VkValidationFlagsEXT();
    ~safe_VkValidationFlagsEXT();
    void initialize(const VkValidationFlagsEXT* in_struct);
    void initialize(const safe_VkValidationFlagsEXT* src);
    VkValidationFlagsEXT *ptr() { return reinterpret_cast<VkValidationFlagsEXT *>(this); }
    VkValidationFlagsEXT const *ptr() const { return reinterpret_cast<VkValidationFlagsEXT const *>(this); }
};

#ifdef VK_USE_PLATFORM_VI_NN
struct safe_VkViSurfaceCreateInfoNN {
    VkStructureType sType;
    const void* pNext;
    VkViSurfaceCreateFlagsNN flags;
    void* window;
    safe_VkViSurfaceCreateInfoNN(const VkViSurfaceCreateInfoNN* in_struct);
    safe_VkViSurfaceCreateInfoNN(const safe_VkViSurfaceCreateInfoNN& src);
    safe_VkViSurfaceCreateInfoNN& operator=(const safe_VkViSurfaceCreateInfoNN& src);
    safe_VkViSurfaceCreateInfoNN();
    ~safe_VkViSurfaceCreateInfoNN();
    void initialize(const VkViSurfaceCreateInfoNN* in_struct);
    void initialize(const safe_VkViSurfaceCreateInfoNN* src);
    VkViSurfaceCreateInfoNN *ptr() { return reinterpret_cast<VkViSurfaceCreateInfoNN *>(this); }
    VkViSurfaceCreateInfoNN const *ptr() const { return reinterpret_cast<VkViSurfaceCreateInfoNN const *>(this); }
};
#endif // VK_USE_PLATFORM_VI_NN

struct safe_VkPhysicalDeviceTextureCompressionASTCHDRFeaturesEXT {
    VkStructureType sType;
    const void* pNext;
    VkBool32 textureCompressionASTC_HDR;
    safe_VkPhysicalDeviceTextureCompressionASTCHDRFeaturesEXT(const VkPhysicalDeviceTextureCompressionASTCHDRFeaturesEXT* in_struct);
    safe_VkPhysicalDeviceTextureCompressionASTCHDRFeaturesEXT(const safe_VkPhysicalDeviceTextureCompressionASTCHDRFeaturesEXT& src);
    safe_VkPhysicalDeviceTextureCompressionASTCHDRFeaturesEXT& operator=(const safe_VkPhysicalDeviceTextureCompressionASTCHDRFeaturesEXT& src);
    safe_VkPhysicalDeviceTextureCompressionASTCHDRFeaturesEXT();
    ~safe_VkPhysicalDeviceTextureCompressionASTCHDRFeaturesEXT();
    void initialize(const VkPhysicalDeviceTextureCompressionASTCHDRFeaturesEXT* in_struct);
    void initialize(const safe_VkPhysicalDeviceTextureCompressionASTCHDRFeaturesEXT* src);
    VkPhysicalDeviceTextureCompressionASTCHDRFeaturesEXT *ptr() { return reinterpret_cast<VkPhysicalDeviceTextureCompressionASTCHDRFeaturesEXT *>(this); }
    VkPhysicalDeviceTextureCompressionASTCHDRFeaturesEXT const *ptr() const { return reinterpret_cast<VkPhysicalDeviceTextureCompressionASTCHDRFeaturesEXT const *>(this); }
};

struct safe_VkImageViewASTCDecodeModeEXT {
    VkStructureType sType;
    const void* pNext;
    VkFormat decodeMode;
    safe_VkImageViewASTCDecodeModeEXT(const VkImageViewASTCDecodeModeEXT* in_struct);
    safe_VkImageViewASTCDecodeModeEXT(const safe_VkImageViewASTCDecodeModeEXT& src);
    safe_VkImageViewASTCDecodeModeEXT& operator=(const safe_VkImageViewASTCDecodeModeEXT& src);
    safe_VkImageViewASTCDecodeModeEXT();
    ~safe_VkImageViewASTCDecodeModeEXT();
    void initialize(const VkImageViewASTCDecodeModeEXT* in_struct);
    void initialize(const safe_VkImageViewASTCDecodeModeEXT* src);
    VkImageViewASTCDecodeModeEXT *ptr() { return reinterpret_cast<VkImageViewASTCDecodeModeEXT *>(this); }
    VkImageViewASTCDecodeModeEXT const *ptr() const { return reinterpret_cast<VkImageViewASTCDecodeModeEXT const *>(this); }
};

struct safe_VkPhysicalDeviceASTCDecodeFeaturesEXT {
    VkStructureType sType;
    void* pNext;
    VkBool32 decodeModeSharedExponent;
    safe_VkPhysicalDeviceASTCDecodeFeaturesEXT(const VkPhysicalDeviceASTCDecodeFeaturesEXT* in_struct);
    safe_VkPhysicalDeviceASTCDecodeFeaturesEXT(const safe_VkPhysicalDeviceASTCDecodeFeaturesEXT& src);
    safe_VkPhysicalDeviceASTCDecodeFeaturesEXT& operator=(const safe_VkPhysicalDeviceASTCDecodeFeaturesEXT& src);
    safe_VkPhysicalDeviceASTCDecodeFeaturesEXT();
    ~safe_VkPhysicalDeviceASTCDecodeFeaturesEXT();
    void initialize(const VkPhysicalDeviceASTCDecodeFeaturesEXT* in_struct);
    void initialize(const safe_VkPhysicalDeviceASTCDecodeFeaturesEXT* src);
    VkPhysicalDeviceASTCDecodeFeaturesEXT *ptr() { return reinterpret_cast<VkPhysicalDeviceASTCDecodeFeaturesEXT *>(this); }
    VkPhysicalDeviceASTCDecodeFeaturesEXT const *ptr() const { return reinterpret_cast<VkPhysicalDeviceASTCDecodeFeaturesEXT const *>(this); }
};

struct safe_VkConditionalRenderingBeginInfoEXT {
    VkStructureType sType;
    const void* pNext;
    VkBuffer buffer;
    VkDeviceSize offset;
    VkConditionalRenderingFlagsEXT flags;
    safe_VkConditionalRenderingBeginInfoEXT(const VkConditionalRenderingBeginInfoEXT* in_struct);
    safe_VkConditionalRenderingBeginInfoEXT(const safe_VkConditionalRenderingBeginInfoEXT& src);
    safe_VkConditionalRenderingBeginInfoEXT& operator=(const safe_VkConditionalRenderingBeginInfoEXT& src);
    safe_VkConditionalRenderingBeginInfoEXT();
    ~safe_VkConditionalRenderingBeginInfoEXT();
    void initialize(const VkConditionalRenderingBeginInfoEXT* in_struct);
    void initialize(const safe_VkConditionalRenderingBeginInfoEXT* src);
    VkConditionalRenderingBeginInfoEXT *ptr() { return reinterpret_cast<VkConditionalRenderingBeginInfoEXT *>(this); }
    VkConditionalRenderingBeginInfoEXT const *ptr() const { return reinterpret_cast<VkConditionalRenderingBeginInfoEXT const *>(this); }
};

struct safe_VkPhysicalDeviceConditionalRenderingFeaturesEXT {
    VkStructureType sType;
    void* pNext;
    VkBool32 conditionalRendering;
    VkBool32 inheritedConditionalRendering;
    safe_VkPhysicalDeviceConditionalRenderingFeaturesEXT(const VkPhysicalDeviceConditionalRenderingFeaturesEXT* in_struct);
    safe_VkPhysicalDeviceConditionalRenderingFeaturesEXT(const safe_VkPhysicalDeviceConditionalRenderingFeaturesEXT& src);
    safe_VkPhysicalDeviceConditionalRenderingFeaturesEXT& operator=(const safe_VkPhysicalDeviceConditionalRenderingFeaturesEXT& src);
    safe_VkPhysicalDeviceConditionalRenderingFeaturesEXT();
    ~safe_VkPhysicalDeviceConditionalRenderingFeaturesEXT();
    void initialize(const VkPhysicalDeviceConditionalRenderingFeaturesEXT* in_struct);
    void initialize(const safe_VkPhysicalDeviceConditionalRenderingFeaturesEXT* src);
    VkPhysicalDeviceConditionalRenderingFeaturesEXT *ptr() { return reinterpret_cast<VkPhysicalDeviceConditionalRenderingFeaturesEXT *>(this); }
    VkPhysicalDeviceConditionalRenderingFeaturesEXT const *ptr() const { return reinterpret_cast<VkPhysicalDeviceConditionalRenderingFeaturesEXT const *>(this); }
};

struct safe_VkCommandBufferInheritanceConditionalRenderingInfoEXT {
    VkStructureType sType;
    const void* pNext;
    VkBool32 conditionalRenderingEnable;
    safe_VkCommandBufferInheritanceConditionalRenderingInfoEXT(const VkCommandBufferInheritanceConditionalRenderingInfoEXT* in_struct);
    safe_VkCommandBufferInheritanceConditionalRenderingInfoEXT(const safe_VkCommandBufferInheritanceConditionalRenderingInfoEXT& src);
    safe_VkCommandBufferInheritanceConditionalRenderingInfoEXT& operator=(const safe_VkCommandBufferInheritanceConditionalRenderingInfoEXT& src);
    safe_VkCommandBufferInheritanceConditionalRenderingInfoEXT();
    ~safe_VkCommandBufferInheritanceConditionalRenderingInfoEXT();
    void initialize(const VkCommandBufferInheritanceConditionalRenderingInfoEXT* in_struct);
    void initialize(const safe_VkCommandBufferInheritanceConditionalRenderingInfoEXT* src);
    VkCommandBufferInheritanceConditionalRenderingInfoEXT *ptr() { return reinterpret_cast<VkCommandBufferInheritanceConditionalRenderingInfoEXT *>(this); }
    VkCommandBufferInheritanceConditionalRenderingInfoEXT const *ptr() const { return reinterpret_cast<VkCommandBufferInheritanceConditionalRenderingInfoEXT const *>(this); }
};

struct safe_VkDeviceGeneratedCommandsFeaturesNVX {
    VkStructureType sType;
    const void* pNext;
    VkBool32 computeBindingPointSupport;
    safe_VkDeviceGeneratedCommandsFeaturesNVX(const VkDeviceGeneratedCommandsFeaturesNVX* in_struct);
    safe_VkDeviceGeneratedCommandsFeaturesNVX(const safe_VkDeviceGeneratedCommandsFeaturesNVX& src);
    safe_VkDeviceGeneratedCommandsFeaturesNVX& operator=(const safe_VkDeviceGeneratedCommandsFeaturesNVX& src);
    safe_VkDeviceGeneratedCommandsFeaturesNVX();
    ~safe_VkDeviceGeneratedCommandsFeaturesNVX();
    void initialize(const VkDeviceGeneratedCommandsFeaturesNVX* in_struct);
    void initialize(const safe_VkDeviceGeneratedCommandsFeaturesNVX* src);
    VkDeviceGeneratedCommandsFeaturesNVX *ptr() { return reinterpret_cast<VkDeviceGeneratedCommandsFeaturesNVX *>(this); }
    VkDeviceGeneratedCommandsFeaturesNVX const *ptr() const { return reinterpret_cast<VkDeviceGeneratedCommandsFeaturesNVX const *>(this); }
};

struct safe_VkDeviceGeneratedCommandsLimitsNVX {
    VkStructureType sType;
    const void* pNext;
    uint32_t maxIndirectCommandsLayoutTokenCount;
    uint32_t maxObjectEntryCounts;
    uint32_t minSequenceCountBufferOffsetAlignment;
    uint32_t minSequenceIndexBufferOffsetAlignment;
    uint32_t minCommandsTokenBufferOffsetAlignment;
    safe_VkDeviceGeneratedCommandsLimitsNVX(const VkDeviceGeneratedCommandsLimitsNVX* in_struct);
    safe_VkDeviceGeneratedCommandsLimitsNVX(const safe_VkDeviceGeneratedCommandsLimitsNVX& src);
    safe_VkDeviceGeneratedCommandsLimitsNVX& operator=(const safe_VkDeviceGeneratedCommandsLimitsNVX& src);
    safe_VkDeviceGeneratedCommandsLimitsNVX();
    ~safe_VkDeviceGeneratedCommandsLimitsNVX();
    void initialize(const VkDeviceGeneratedCommandsLimitsNVX* in_struct);
    void initialize(const safe_VkDeviceGeneratedCommandsLimitsNVX* src);
    VkDeviceGeneratedCommandsLimitsNVX *ptr() { return reinterpret_cast<VkDeviceGeneratedCommandsLimitsNVX *>(this); }
    VkDeviceGeneratedCommandsLimitsNVX const *ptr() const { return reinterpret_cast<VkDeviceGeneratedCommandsLimitsNVX const *>(this); }
};

struct safe_VkIndirectCommandsLayoutCreateInfoNVX {
    VkStructureType sType;
    const void* pNext;
    VkPipelineBindPoint pipelineBindPoint;
    VkIndirectCommandsLayoutUsageFlagsNVX flags;
    uint32_t tokenCount;
    const VkIndirectCommandsLayoutTokenNVX* pTokens;
    safe_VkIndirectCommandsLayoutCreateInfoNVX(const VkIndirectCommandsLayoutCreateInfoNVX* in_struct);
    safe_VkIndirectCommandsLayoutCreateInfoNVX(const safe_VkIndirectCommandsLayoutCreateInfoNVX& src);
    safe_VkIndirectCommandsLayoutCreateInfoNVX& operator=(const safe_VkIndirectCommandsLayoutCreateInfoNVX& src);
    safe_VkIndirectCommandsLayoutCreateInfoNVX();
    ~safe_VkIndirectCommandsLayoutCreateInfoNVX();
    void initialize(const VkIndirectCommandsLayoutCreateInfoNVX* in_struct);
    void initialize(const safe_VkIndirectCommandsLayoutCreateInfoNVX* src);
    VkIndirectCommandsLayoutCreateInfoNVX *ptr() { return reinterpret_cast<VkIndirectCommandsLayoutCreateInfoNVX *>(this); }
    VkIndirectCommandsLayoutCreateInfoNVX const *ptr() const { return reinterpret_cast<VkIndirectCommandsLayoutCreateInfoNVX const *>(this); }
};

struct safe_VkCmdProcessCommandsInfoNVX {
    VkStructureType sType;
    const void* pNext;
    VkObjectTableNVX objectTable;
    VkIndirectCommandsLayoutNVX indirectCommandsLayout;
    uint32_t indirectCommandsTokenCount;
    VkIndirectCommandsTokenNVX* pIndirectCommandsTokens;
    uint32_t maxSequencesCount;
    VkCommandBuffer targetCommandBuffer;
    VkBuffer sequencesCountBuffer;
    VkDeviceSize sequencesCountOffset;
    VkBuffer sequencesIndexBuffer;
    VkDeviceSize sequencesIndexOffset;
    safe_VkCmdProcessCommandsInfoNVX(const VkCmdProcessCommandsInfoNVX* in_struct);
    safe_VkCmdProcessCommandsInfoNVX(const safe_VkCmdProcessCommandsInfoNVX& src);
    safe_VkCmdProcessCommandsInfoNVX& operator=(const safe_VkCmdProcessCommandsInfoNVX& src);
    safe_VkCmdProcessCommandsInfoNVX();
    ~safe_VkCmdProcessCommandsInfoNVX();
    void initialize(const VkCmdProcessCommandsInfoNVX* in_struct);
    void initialize(const safe_VkCmdProcessCommandsInfoNVX* src);
    VkCmdProcessCommandsInfoNVX *ptr() { return reinterpret_cast<VkCmdProcessCommandsInfoNVX *>(this); }
    VkCmdProcessCommandsInfoNVX const *ptr() const { return reinterpret_cast<VkCmdProcessCommandsInfoNVX const *>(this); }
};

struct safe_VkCmdReserveSpaceForCommandsInfoNVX {
    VkStructureType sType;
    const void* pNext;
    VkObjectTableNVX objectTable;
    VkIndirectCommandsLayoutNVX indirectCommandsLayout;
    uint32_t maxSequencesCount;
    safe_VkCmdReserveSpaceForCommandsInfoNVX(const VkCmdReserveSpaceForCommandsInfoNVX* in_struct);
    safe_VkCmdReserveSpaceForCommandsInfoNVX(const safe_VkCmdReserveSpaceForCommandsInfoNVX& src);
    safe_VkCmdReserveSpaceForCommandsInfoNVX& operator=(const safe_VkCmdReserveSpaceForCommandsInfoNVX& src);
    safe_VkCmdReserveSpaceForCommandsInfoNVX();
    ~safe_VkCmdReserveSpaceForCommandsInfoNVX();
    void initialize(const VkCmdReserveSpaceForCommandsInfoNVX* in_struct);
    void initialize(const safe_VkCmdReserveSpaceForCommandsInfoNVX* src);
    VkCmdReserveSpaceForCommandsInfoNVX *ptr() { return reinterpret_cast<VkCmdReserveSpaceForCommandsInfoNVX *>(this); }
    VkCmdReserveSpaceForCommandsInfoNVX const *ptr() const { return reinterpret_cast<VkCmdReserveSpaceForCommandsInfoNVX const *>(this); }
};

struct safe_VkObjectTableCreateInfoNVX {
    VkStructureType sType;
    const void* pNext;
    uint32_t objectCount;
    const VkObjectEntryTypeNVX* pObjectEntryTypes;
    const uint32_t* pObjectEntryCounts;
    const VkObjectEntryUsageFlagsNVX* pObjectEntryUsageFlags;
    uint32_t maxUniformBuffersPerDescriptor;
    uint32_t maxStorageBuffersPerDescriptor;
    uint32_t maxStorageImagesPerDescriptor;
    uint32_t maxSampledImagesPerDescriptor;
    uint32_t maxPipelineLayouts;
    safe_VkObjectTableCreateInfoNVX(const VkObjectTableCreateInfoNVX* in_struct);
    safe_VkObjectTableCreateInfoNVX(const safe_VkObjectTableCreateInfoNVX& src);
    safe_VkObjectTableCreateInfoNVX& operator=(const safe_VkObjectTableCreateInfoNVX& src);
    safe_VkObjectTableCreateInfoNVX();
    ~safe_VkObjectTableCreateInfoNVX();
    void initialize(const VkObjectTableCreateInfoNVX* in_struct);
    void initialize(const safe_VkObjectTableCreateInfoNVX* src);
    VkObjectTableCreateInfoNVX *ptr() { return reinterpret_cast<VkObjectTableCreateInfoNVX *>(this); }
    VkObjectTableCreateInfoNVX const *ptr() const { return reinterpret_cast<VkObjectTableCreateInfoNVX const *>(this); }
};

struct safe_VkPipelineViewportWScalingStateCreateInfoNV {
    VkStructureType sType;
    const void* pNext;
    VkBool32 viewportWScalingEnable;
    uint32_t viewportCount;
    const VkViewportWScalingNV* pViewportWScalings;
    safe_VkPipelineViewportWScalingStateCreateInfoNV(const VkPipelineViewportWScalingStateCreateInfoNV* in_struct);
    safe_VkPipelineViewportWScalingStateCreateInfoNV(const safe_VkPipelineViewportWScalingStateCreateInfoNV& src);
    safe_VkPipelineViewportWScalingStateCreateInfoNV& operator=(const safe_VkPipelineViewportWScalingStateCreateInfoNV& src);
    safe_VkPipelineViewportWScalingStateCreateInfoNV();
    ~safe_VkPipelineViewportWScalingStateCreateInfoNV();
    void initialize(const VkPipelineViewportWScalingStateCreateInfoNV* in_struct);
    void initialize(const safe_VkPipelineViewportWScalingStateCreateInfoNV* src);
    VkPipelineViewportWScalingStateCreateInfoNV *ptr() { return reinterpret_cast<VkPipelineViewportWScalingStateCreateInfoNV *>(this); }
    VkPipelineViewportWScalingStateCreateInfoNV const *ptr() const { return reinterpret_cast<VkPipelineViewportWScalingStateCreateInfoNV const *>(this); }
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
    safe_VkSurfaceCapabilities2EXT(const safe_VkSurfaceCapabilities2EXT& src);
    safe_VkSurfaceCapabilities2EXT& operator=(const safe_VkSurfaceCapabilities2EXT& src);
    safe_VkSurfaceCapabilities2EXT();
    ~safe_VkSurfaceCapabilities2EXT();
    void initialize(const VkSurfaceCapabilities2EXT* in_struct);
    void initialize(const safe_VkSurfaceCapabilities2EXT* src);
    VkSurfaceCapabilities2EXT *ptr() { return reinterpret_cast<VkSurfaceCapabilities2EXT *>(this); }
    VkSurfaceCapabilities2EXT const *ptr() const { return reinterpret_cast<VkSurfaceCapabilities2EXT const *>(this); }
};

struct safe_VkDisplayPowerInfoEXT {
    VkStructureType sType;
    const void* pNext;
    VkDisplayPowerStateEXT powerState;
    safe_VkDisplayPowerInfoEXT(const VkDisplayPowerInfoEXT* in_struct);
    safe_VkDisplayPowerInfoEXT(const safe_VkDisplayPowerInfoEXT& src);
    safe_VkDisplayPowerInfoEXT& operator=(const safe_VkDisplayPowerInfoEXT& src);
    safe_VkDisplayPowerInfoEXT();
    ~safe_VkDisplayPowerInfoEXT();
    void initialize(const VkDisplayPowerInfoEXT* in_struct);
    void initialize(const safe_VkDisplayPowerInfoEXT* src);
    VkDisplayPowerInfoEXT *ptr() { return reinterpret_cast<VkDisplayPowerInfoEXT *>(this); }
    VkDisplayPowerInfoEXT const *ptr() const { return reinterpret_cast<VkDisplayPowerInfoEXT const *>(this); }
};

struct safe_VkDeviceEventInfoEXT {
    VkStructureType sType;
    const void* pNext;
    VkDeviceEventTypeEXT deviceEvent;
    safe_VkDeviceEventInfoEXT(const VkDeviceEventInfoEXT* in_struct);
    safe_VkDeviceEventInfoEXT(const safe_VkDeviceEventInfoEXT& src);
    safe_VkDeviceEventInfoEXT& operator=(const safe_VkDeviceEventInfoEXT& src);
    safe_VkDeviceEventInfoEXT();
    ~safe_VkDeviceEventInfoEXT();
    void initialize(const VkDeviceEventInfoEXT* in_struct);
    void initialize(const safe_VkDeviceEventInfoEXT* src);
    VkDeviceEventInfoEXT *ptr() { return reinterpret_cast<VkDeviceEventInfoEXT *>(this); }
    VkDeviceEventInfoEXT const *ptr() const { return reinterpret_cast<VkDeviceEventInfoEXT const *>(this); }
};

struct safe_VkDisplayEventInfoEXT {
    VkStructureType sType;
    const void* pNext;
    VkDisplayEventTypeEXT displayEvent;
    safe_VkDisplayEventInfoEXT(const VkDisplayEventInfoEXT* in_struct);
    safe_VkDisplayEventInfoEXT(const safe_VkDisplayEventInfoEXT& src);
    safe_VkDisplayEventInfoEXT& operator=(const safe_VkDisplayEventInfoEXT& src);
    safe_VkDisplayEventInfoEXT();
    ~safe_VkDisplayEventInfoEXT();
    void initialize(const VkDisplayEventInfoEXT* in_struct);
    void initialize(const safe_VkDisplayEventInfoEXT* src);
    VkDisplayEventInfoEXT *ptr() { return reinterpret_cast<VkDisplayEventInfoEXT *>(this); }
    VkDisplayEventInfoEXT const *ptr() const { return reinterpret_cast<VkDisplayEventInfoEXT const *>(this); }
};

struct safe_VkSwapchainCounterCreateInfoEXT {
    VkStructureType sType;
    const void* pNext;
    VkSurfaceCounterFlagsEXT surfaceCounters;
    safe_VkSwapchainCounterCreateInfoEXT(const VkSwapchainCounterCreateInfoEXT* in_struct);
    safe_VkSwapchainCounterCreateInfoEXT(const safe_VkSwapchainCounterCreateInfoEXT& src);
    safe_VkSwapchainCounterCreateInfoEXT& operator=(const safe_VkSwapchainCounterCreateInfoEXT& src);
    safe_VkSwapchainCounterCreateInfoEXT();
    ~safe_VkSwapchainCounterCreateInfoEXT();
    void initialize(const VkSwapchainCounterCreateInfoEXT* in_struct);
    void initialize(const safe_VkSwapchainCounterCreateInfoEXT* src);
    VkSwapchainCounterCreateInfoEXT *ptr() { return reinterpret_cast<VkSwapchainCounterCreateInfoEXT *>(this); }
    VkSwapchainCounterCreateInfoEXT const *ptr() const { return reinterpret_cast<VkSwapchainCounterCreateInfoEXT const *>(this); }
};

struct safe_VkPresentTimesInfoGOOGLE {
    VkStructureType sType;
    const void* pNext;
    uint32_t swapchainCount;
    const VkPresentTimeGOOGLE* pTimes;
    safe_VkPresentTimesInfoGOOGLE(const VkPresentTimesInfoGOOGLE* in_struct);
    safe_VkPresentTimesInfoGOOGLE(const safe_VkPresentTimesInfoGOOGLE& src);
    safe_VkPresentTimesInfoGOOGLE& operator=(const safe_VkPresentTimesInfoGOOGLE& src);
    safe_VkPresentTimesInfoGOOGLE();
    ~safe_VkPresentTimesInfoGOOGLE();
    void initialize(const VkPresentTimesInfoGOOGLE* in_struct);
    void initialize(const safe_VkPresentTimesInfoGOOGLE* src);
    VkPresentTimesInfoGOOGLE *ptr() { return reinterpret_cast<VkPresentTimesInfoGOOGLE *>(this); }
    VkPresentTimesInfoGOOGLE const *ptr() const { return reinterpret_cast<VkPresentTimesInfoGOOGLE const *>(this); }
};

struct safe_VkPhysicalDeviceMultiviewPerViewAttributesPropertiesNVX {
    VkStructureType sType;
    void* pNext;
    VkBool32 perViewPositionAllComponents;
    safe_VkPhysicalDeviceMultiviewPerViewAttributesPropertiesNVX(const VkPhysicalDeviceMultiviewPerViewAttributesPropertiesNVX* in_struct);
    safe_VkPhysicalDeviceMultiviewPerViewAttributesPropertiesNVX(const safe_VkPhysicalDeviceMultiviewPerViewAttributesPropertiesNVX& src);
    safe_VkPhysicalDeviceMultiviewPerViewAttributesPropertiesNVX& operator=(const safe_VkPhysicalDeviceMultiviewPerViewAttributesPropertiesNVX& src);
    safe_VkPhysicalDeviceMultiviewPerViewAttributesPropertiesNVX();
    ~safe_VkPhysicalDeviceMultiviewPerViewAttributesPropertiesNVX();
    void initialize(const VkPhysicalDeviceMultiviewPerViewAttributesPropertiesNVX* in_struct);
    void initialize(const safe_VkPhysicalDeviceMultiviewPerViewAttributesPropertiesNVX* src);
    VkPhysicalDeviceMultiviewPerViewAttributesPropertiesNVX *ptr() { return reinterpret_cast<VkPhysicalDeviceMultiviewPerViewAttributesPropertiesNVX *>(this); }
    VkPhysicalDeviceMultiviewPerViewAttributesPropertiesNVX const *ptr() const { return reinterpret_cast<VkPhysicalDeviceMultiviewPerViewAttributesPropertiesNVX const *>(this); }
};

struct safe_VkPipelineViewportSwizzleStateCreateInfoNV {
    VkStructureType sType;
    const void* pNext;
    VkPipelineViewportSwizzleStateCreateFlagsNV flags;
    uint32_t viewportCount;
    const VkViewportSwizzleNV* pViewportSwizzles;
    safe_VkPipelineViewportSwizzleStateCreateInfoNV(const VkPipelineViewportSwizzleStateCreateInfoNV* in_struct);
    safe_VkPipelineViewportSwizzleStateCreateInfoNV(const safe_VkPipelineViewportSwizzleStateCreateInfoNV& src);
    safe_VkPipelineViewportSwizzleStateCreateInfoNV& operator=(const safe_VkPipelineViewportSwizzleStateCreateInfoNV& src);
    safe_VkPipelineViewportSwizzleStateCreateInfoNV();
    ~safe_VkPipelineViewportSwizzleStateCreateInfoNV();
    void initialize(const VkPipelineViewportSwizzleStateCreateInfoNV* in_struct);
    void initialize(const safe_VkPipelineViewportSwizzleStateCreateInfoNV* src);
    VkPipelineViewportSwizzleStateCreateInfoNV *ptr() { return reinterpret_cast<VkPipelineViewportSwizzleStateCreateInfoNV *>(this); }
    VkPipelineViewportSwizzleStateCreateInfoNV const *ptr() const { return reinterpret_cast<VkPipelineViewportSwizzleStateCreateInfoNV const *>(this); }
};

struct safe_VkPhysicalDeviceDiscardRectanglePropertiesEXT {
    VkStructureType sType;
    void* pNext;
    uint32_t maxDiscardRectangles;
    safe_VkPhysicalDeviceDiscardRectanglePropertiesEXT(const VkPhysicalDeviceDiscardRectanglePropertiesEXT* in_struct);
    safe_VkPhysicalDeviceDiscardRectanglePropertiesEXT(const safe_VkPhysicalDeviceDiscardRectanglePropertiesEXT& src);
    safe_VkPhysicalDeviceDiscardRectanglePropertiesEXT& operator=(const safe_VkPhysicalDeviceDiscardRectanglePropertiesEXT& src);
    safe_VkPhysicalDeviceDiscardRectanglePropertiesEXT();
    ~safe_VkPhysicalDeviceDiscardRectanglePropertiesEXT();
    void initialize(const VkPhysicalDeviceDiscardRectanglePropertiesEXT* in_struct);
    void initialize(const safe_VkPhysicalDeviceDiscardRectanglePropertiesEXT* src);
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
    safe_VkPipelineDiscardRectangleStateCreateInfoEXT(const safe_VkPipelineDiscardRectangleStateCreateInfoEXT& src);
    safe_VkPipelineDiscardRectangleStateCreateInfoEXT& operator=(const safe_VkPipelineDiscardRectangleStateCreateInfoEXT& src);
    safe_VkPipelineDiscardRectangleStateCreateInfoEXT();
    ~safe_VkPipelineDiscardRectangleStateCreateInfoEXT();
    void initialize(const VkPipelineDiscardRectangleStateCreateInfoEXT* in_struct);
    void initialize(const safe_VkPipelineDiscardRectangleStateCreateInfoEXT* src);
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
    safe_VkPhysicalDeviceConservativeRasterizationPropertiesEXT(const safe_VkPhysicalDeviceConservativeRasterizationPropertiesEXT& src);
    safe_VkPhysicalDeviceConservativeRasterizationPropertiesEXT& operator=(const safe_VkPhysicalDeviceConservativeRasterizationPropertiesEXT& src);
    safe_VkPhysicalDeviceConservativeRasterizationPropertiesEXT();
    ~safe_VkPhysicalDeviceConservativeRasterizationPropertiesEXT();
    void initialize(const VkPhysicalDeviceConservativeRasterizationPropertiesEXT* in_struct);
    void initialize(const safe_VkPhysicalDeviceConservativeRasterizationPropertiesEXT* src);
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
    safe_VkPipelineRasterizationConservativeStateCreateInfoEXT(const safe_VkPipelineRasterizationConservativeStateCreateInfoEXT& src);
    safe_VkPipelineRasterizationConservativeStateCreateInfoEXT& operator=(const safe_VkPipelineRasterizationConservativeStateCreateInfoEXT& src);
    safe_VkPipelineRasterizationConservativeStateCreateInfoEXT();
    ~safe_VkPipelineRasterizationConservativeStateCreateInfoEXT();
    void initialize(const VkPipelineRasterizationConservativeStateCreateInfoEXT* in_struct);
    void initialize(const safe_VkPipelineRasterizationConservativeStateCreateInfoEXT* src);
    VkPipelineRasterizationConservativeStateCreateInfoEXT *ptr() { return reinterpret_cast<VkPipelineRasterizationConservativeStateCreateInfoEXT *>(this); }
    VkPipelineRasterizationConservativeStateCreateInfoEXT const *ptr() const { return reinterpret_cast<VkPipelineRasterizationConservativeStateCreateInfoEXT const *>(this); }
};

struct safe_VkPhysicalDeviceDepthClipEnableFeaturesEXT {
    VkStructureType sType;
    void* pNext;
    VkBool32 depthClipEnable;
    safe_VkPhysicalDeviceDepthClipEnableFeaturesEXT(const VkPhysicalDeviceDepthClipEnableFeaturesEXT* in_struct);
    safe_VkPhysicalDeviceDepthClipEnableFeaturesEXT(const safe_VkPhysicalDeviceDepthClipEnableFeaturesEXT& src);
    safe_VkPhysicalDeviceDepthClipEnableFeaturesEXT& operator=(const safe_VkPhysicalDeviceDepthClipEnableFeaturesEXT& src);
    safe_VkPhysicalDeviceDepthClipEnableFeaturesEXT();
    ~safe_VkPhysicalDeviceDepthClipEnableFeaturesEXT();
    void initialize(const VkPhysicalDeviceDepthClipEnableFeaturesEXT* in_struct);
    void initialize(const safe_VkPhysicalDeviceDepthClipEnableFeaturesEXT* src);
    VkPhysicalDeviceDepthClipEnableFeaturesEXT *ptr() { return reinterpret_cast<VkPhysicalDeviceDepthClipEnableFeaturesEXT *>(this); }
    VkPhysicalDeviceDepthClipEnableFeaturesEXT const *ptr() const { return reinterpret_cast<VkPhysicalDeviceDepthClipEnableFeaturesEXT const *>(this); }
};

struct safe_VkPipelineRasterizationDepthClipStateCreateInfoEXT {
    VkStructureType sType;
    const void* pNext;
    VkPipelineRasterizationDepthClipStateCreateFlagsEXT flags;
    VkBool32 depthClipEnable;
    safe_VkPipelineRasterizationDepthClipStateCreateInfoEXT(const VkPipelineRasterizationDepthClipStateCreateInfoEXT* in_struct);
    safe_VkPipelineRasterizationDepthClipStateCreateInfoEXT(const safe_VkPipelineRasterizationDepthClipStateCreateInfoEXT& src);
    safe_VkPipelineRasterizationDepthClipStateCreateInfoEXT& operator=(const safe_VkPipelineRasterizationDepthClipStateCreateInfoEXT& src);
    safe_VkPipelineRasterizationDepthClipStateCreateInfoEXT();
    ~safe_VkPipelineRasterizationDepthClipStateCreateInfoEXT();
    void initialize(const VkPipelineRasterizationDepthClipStateCreateInfoEXT* in_struct);
    void initialize(const safe_VkPipelineRasterizationDepthClipStateCreateInfoEXT* src);
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
    safe_VkHdrMetadataEXT(const safe_VkHdrMetadataEXT& src);
    safe_VkHdrMetadataEXT& operator=(const safe_VkHdrMetadataEXT& src);
    safe_VkHdrMetadataEXT();
    ~safe_VkHdrMetadataEXT();
    void initialize(const VkHdrMetadataEXT* in_struct);
    void initialize(const safe_VkHdrMetadataEXT* src);
    VkHdrMetadataEXT *ptr() { return reinterpret_cast<VkHdrMetadataEXT *>(this); }
    VkHdrMetadataEXT const *ptr() const { return reinterpret_cast<VkHdrMetadataEXT const *>(this); }
};

#ifdef VK_USE_PLATFORM_IOS_MVK
struct safe_VkIOSSurfaceCreateInfoMVK {
    VkStructureType sType;
    const void* pNext;
    VkIOSSurfaceCreateFlagsMVK flags;
    const void* pView;
    safe_VkIOSSurfaceCreateInfoMVK(const VkIOSSurfaceCreateInfoMVK* in_struct);
    safe_VkIOSSurfaceCreateInfoMVK(const safe_VkIOSSurfaceCreateInfoMVK& src);
    safe_VkIOSSurfaceCreateInfoMVK& operator=(const safe_VkIOSSurfaceCreateInfoMVK& src);
    safe_VkIOSSurfaceCreateInfoMVK();
    ~safe_VkIOSSurfaceCreateInfoMVK();
    void initialize(const VkIOSSurfaceCreateInfoMVK* in_struct);
    void initialize(const safe_VkIOSSurfaceCreateInfoMVK* src);
    VkIOSSurfaceCreateInfoMVK *ptr() { return reinterpret_cast<VkIOSSurfaceCreateInfoMVK *>(this); }
    VkIOSSurfaceCreateInfoMVK const *ptr() const { return reinterpret_cast<VkIOSSurfaceCreateInfoMVK const *>(this); }
};
#endif // VK_USE_PLATFORM_IOS_MVK

#ifdef VK_USE_PLATFORM_MACOS_MVK
struct safe_VkMacOSSurfaceCreateInfoMVK {
    VkStructureType sType;
    const void* pNext;
    VkMacOSSurfaceCreateFlagsMVK flags;
    const void* pView;
    safe_VkMacOSSurfaceCreateInfoMVK(const VkMacOSSurfaceCreateInfoMVK* in_struct);
    safe_VkMacOSSurfaceCreateInfoMVK(const safe_VkMacOSSurfaceCreateInfoMVK& src);
    safe_VkMacOSSurfaceCreateInfoMVK& operator=(const safe_VkMacOSSurfaceCreateInfoMVK& src);
    safe_VkMacOSSurfaceCreateInfoMVK();
    ~safe_VkMacOSSurfaceCreateInfoMVK();
    void initialize(const VkMacOSSurfaceCreateInfoMVK* in_struct);
    void initialize(const safe_VkMacOSSurfaceCreateInfoMVK* src);
    VkMacOSSurfaceCreateInfoMVK *ptr() { return reinterpret_cast<VkMacOSSurfaceCreateInfoMVK *>(this); }
    VkMacOSSurfaceCreateInfoMVK const *ptr() const { return reinterpret_cast<VkMacOSSurfaceCreateInfoMVK const *>(this); }
};
#endif // VK_USE_PLATFORM_MACOS_MVK

struct safe_VkDebugUtilsObjectNameInfoEXT {
    VkStructureType sType;
    const void* pNext;
    VkObjectType objectType;
    uint64_t objectHandle;
    const char* pObjectName;
    safe_VkDebugUtilsObjectNameInfoEXT(const VkDebugUtilsObjectNameInfoEXT* in_struct);
    safe_VkDebugUtilsObjectNameInfoEXT(const safe_VkDebugUtilsObjectNameInfoEXT& src);
    safe_VkDebugUtilsObjectNameInfoEXT& operator=(const safe_VkDebugUtilsObjectNameInfoEXT& src);
    safe_VkDebugUtilsObjectNameInfoEXT();
    ~safe_VkDebugUtilsObjectNameInfoEXT();
    void initialize(const VkDebugUtilsObjectNameInfoEXT* in_struct);
    void initialize(const safe_VkDebugUtilsObjectNameInfoEXT* src);
    VkDebugUtilsObjectNameInfoEXT *ptr() { return reinterpret_cast<VkDebugUtilsObjectNameInfoEXT *>(this); }
    VkDebugUtilsObjectNameInfoEXT const *ptr() const { return reinterpret_cast<VkDebugUtilsObjectNameInfoEXT const *>(this); }
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
    safe_VkDebugUtilsObjectTagInfoEXT(const safe_VkDebugUtilsObjectTagInfoEXT& src);
    safe_VkDebugUtilsObjectTagInfoEXT& operator=(const safe_VkDebugUtilsObjectTagInfoEXT& src);
    safe_VkDebugUtilsObjectTagInfoEXT();
    ~safe_VkDebugUtilsObjectTagInfoEXT();
    void initialize(const VkDebugUtilsObjectTagInfoEXT* in_struct);
    void initialize(const safe_VkDebugUtilsObjectTagInfoEXT* src);
    VkDebugUtilsObjectTagInfoEXT *ptr() { return reinterpret_cast<VkDebugUtilsObjectTagInfoEXT *>(this); }
    VkDebugUtilsObjectTagInfoEXT const *ptr() const { return reinterpret_cast<VkDebugUtilsObjectTagInfoEXT const *>(this); }
};

struct safe_VkDebugUtilsLabelEXT {
    VkStructureType sType;
    const void* pNext;
    const char* pLabelName;
    float color[4];
    safe_VkDebugUtilsLabelEXT(const VkDebugUtilsLabelEXT* in_struct);
    safe_VkDebugUtilsLabelEXT(const safe_VkDebugUtilsLabelEXT& src);
    safe_VkDebugUtilsLabelEXT& operator=(const safe_VkDebugUtilsLabelEXT& src);
    safe_VkDebugUtilsLabelEXT();
    ~safe_VkDebugUtilsLabelEXT();
    void initialize(const VkDebugUtilsLabelEXT* in_struct);
    void initialize(const safe_VkDebugUtilsLabelEXT* src);
    VkDebugUtilsLabelEXT *ptr() { return reinterpret_cast<VkDebugUtilsLabelEXT *>(this); }
    VkDebugUtilsLabelEXT const *ptr() const { return reinterpret_cast<VkDebugUtilsLabelEXT const *>(this); }
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
    safe_VkDebugUtilsMessengerCallbackDataEXT(const safe_VkDebugUtilsMessengerCallbackDataEXT& src);
    safe_VkDebugUtilsMessengerCallbackDataEXT& operator=(const safe_VkDebugUtilsMessengerCallbackDataEXT& src);
    safe_VkDebugUtilsMessengerCallbackDataEXT();
    ~safe_VkDebugUtilsMessengerCallbackDataEXT();
    void initialize(const VkDebugUtilsMessengerCallbackDataEXT* in_struct);
    void initialize(const safe_VkDebugUtilsMessengerCallbackDataEXT* src);
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
    safe_VkDebugUtilsMessengerCreateInfoEXT(const safe_VkDebugUtilsMessengerCreateInfoEXT& src);
    safe_VkDebugUtilsMessengerCreateInfoEXT& operator=(const safe_VkDebugUtilsMessengerCreateInfoEXT& src);
    safe_VkDebugUtilsMessengerCreateInfoEXT();
    ~safe_VkDebugUtilsMessengerCreateInfoEXT();
    void initialize(const VkDebugUtilsMessengerCreateInfoEXT* in_struct);
    void initialize(const safe_VkDebugUtilsMessengerCreateInfoEXT* src);
    VkDebugUtilsMessengerCreateInfoEXT *ptr() { return reinterpret_cast<VkDebugUtilsMessengerCreateInfoEXT *>(this); }
    VkDebugUtilsMessengerCreateInfoEXT const *ptr() const { return reinterpret_cast<VkDebugUtilsMessengerCreateInfoEXT const *>(this); }
};

#ifdef VK_USE_PLATFORM_ANDROID_KHR
struct safe_VkAndroidHardwareBufferUsageANDROID {
    VkStructureType sType;
    void* pNext;
    uint64_t androidHardwareBufferUsage;
    safe_VkAndroidHardwareBufferUsageANDROID(const VkAndroidHardwareBufferUsageANDROID* in_struct);
    safe_VkAndroidHardwareBufferUsageANDROID(const safe_VkAndroidHardwareBufferUsageANDROID& src);
    safe_VkAndroidHardwareBufferUsageANDROID& operator=(const safe_VkAndroidHardwareBufferUsageANDROID& src);
    safe_VkAndroidHardwareBufferUsageANDROID();
    ~safe_VkAndroidHardwareBufferUsageANDROID();
    void initialize(const VkAndroidHardwareBufferUsageANDROID* in_struct);
    void initialize(const safe_VkAndroidHardwareBufferUsageANDROID* src);
    VkAndroidHardwareBufferUsageANDROID *ptr() { return reinterpret_cast<VkAndroidHardwareBufferUsageANDROID *>(this); }
    VkAndroidHardwareBufferUsageANDROID const *ptr() const { return reinterpret_cast<VkAndroidHardwareBufferUsageANDROID const *>(this); }
};
#endif // VK_USE_PLATFORM_ANDROID_KHR

#ifdef VK_USE_PLATFORM_ANDROID_KHR
struct safe_VkAndroidHardwareBufferPropertiesANDROID {
    VkStructureType sType;
    void* pNext;
    VkDeviceSize allocationSize;
    uint32_t memoryTypeBits;
    safe_VkAndroidHardwareBufferPropertiesANDROID(const VkAndroidHardwareBufferPropertiesANDROID* in_struct);
    safe_VkAndroidHardwareBufferPropertiesANDROID(const safe_VkAndroidHardwareBufferPropertiesANDROID& src);
    safe_VkAndroidHardwareBufferPropertiesANDROID& operator=(const safe_VkAndroidHardwareBufferPropertiesANDROID& src);
    safe_VkAndroidHardwareBufferPropertiesANDROID();
    ~safe_VkAndroidHardwareBufferPropertiesANDROID();
    void initialize(const VkAndroidHardwareBufferPropertiesANDROID* in_struct);
    void initialize(const safe_VkAndroidHardwareBufferPropertiesANDROID* src);
    VkAndroidHardwareBufferPropertiesANDROID *ptr() { return reinterpret_cast<VkAndroidHardwareBufferPropertiesANDROID *>(this); }
    VkAndroidHardwareBufferPropertiesANDROID const *ptr() const { return reinterpret_cast<VkAndroidHardwareBufferPropertiesANDROID const *>(this); }
};
#endif // VK_USE_PLATFORM_ANDROID_KHR

#ifdef VK_USE_PLATFORM_ANDROID_KHR
struct safe_VkAndroidHardwareBufferFormatPropertiesANDROID {
    VkStructureType sType;
    void* pNext;
    VkFormat format;
    uint64_t externalFormat;
    VkFormatFeatureFlags formatFeatures;
    VkComponentMapping samplerYcbcrConversionComponents;
    VkSamplerYcbcrModelConversion suggestedYcbcrModel;
    VkSamplerYcbcrRange suggestedYcbcrRange;
    VkChromaLocation suggestedXChromaOffset;
    VkChromaLocation suggestedYChromaOffset;
    safe_VkAndroidHardwareBufferFormatPropertiesANDROID(const VkAndroidHardwareBufferFormatPropertiesANDROID* in_struct);
    safe_VkAndroidHardwareBufferFormatPropertiesANDROID(const safe_VkAndroidHardwareBufferFormatPropertiesANDROID& src);
    safe_VkAndroidHardwareBufferFormatPropertiesANDROID& operator=(const safe_VkAndroidHardwareBufferFormatPropertiesANDROID& src);
    safe_VkAndroidHardwareBufferFormatPropertiesANDROID();
    ~safe_VkAndroidHardwareBufferFormatPropertiesANDROID();
    void initialize(const VkAndroidHardwareBufferFormatPropertiesANDROID* in_struct);
    void initialize(const safe_VkAndroidHardwareBufferFormatPropertiesANDROID* src);
    VkAndroidHardwareBufferFormatPropertiesANDROID *ptr() { return reinterpret_cast<VkAndroidHardwareBufferFormatPropertiesANDROID *>(this); }
    VkAndroidHardwareBufferFormatPropertiesANDROID const *ptr() const { return reinterpret_cast<VkAndroidHardwareBufferFormatPropertiesANDROID const *>(this); }
};
#endif // VK_USE_PLATFORM_ANDROID_KHR

#ifdef VK_USE_PLATFORM_ANDROID_KHR
struct safe_VkImportAndroidHardwareBufferInfoANDROID {
    VkStructureType sType;
    const void* pNext;
    struct AHardwareBuffer* buffer;
    safe_VkImportAndroidHardwareBufferInfoANDROID(const VkImportAndroidHardwareBufferInfoANDROID* in_struct);
    safe_VkImportAndroidHardwareBufferInfoANDROID(const safe_VkImportAndroidHardwareBufferInfoANDROID& src);
    safe_VkImportAndroidHardwareBufferInfoANDROID& operator=(const safe_VkImportAndroidHardwareBufferInfoANDROID& src);
    safe_VkImportAndroidHardwareBufferInfoANDROID();
    ~safe_VkImportAndroidHardwareBufferInfoANDROID();
    void initialize(const VkImportAndroidHardwareBufferInfoANDROID* in_struct);
    void initialize(const safe_VkImportAndroidHardwareBufferInfoANDROID* src);
    VkImportAndroidHardwareBufferInfoANDROID *ptr() { return reinterpret_cast<VkImportAndroidHardwareBufferInfoANDROID *>(this); }
    VkImportAndroidHardwareBufferInfoANDROID const *ptr() const { return reinterpret_cast<VkImportAndroidHardwareBufferInfoANDROID const *>(this); }
};
#endif // VK_USE_PLATFORM_ANDROID_KHR

#ifdef VK_USE_PLATFORM_ANDROID_KHR
struct safe_VkMemoryGetAndroidHardwareBufferInfoANDROID {
    VkStructureType sType;
    const void* pNext;
    VkDeviceMemory memory;
    safe_VkMemoryGetAndroidHardwareBufferInfoANDROID(const VkMemoryGetAndroidHardwareBufferInfoANDROID* in_struct);
    safe_VkMemoryGetAndroidHardwareBufferInfoANDROID(const safe_VkMemoryGetAndroidHardwareBufferInfoANDROID& src);
    safe_VkMemoryGetAndroidHardwareBufferInfoANDROID& operator=(const safe_VkMemoryGetAndroidHardwareBufferInfoANDROID& src);
    safe_VkMemoryGetAndroidHardwareBufferInfoANDROID();
    ~safe_VkMemoryGetAndroidHardwareBufferInfoANDROID();
    void initialize(const VkMemoryGetAndroidHardwareBufferInfoANDROID* in_struct);
    void initialize(const safe_VkMemoryGetAndroidHardwareBufferInfoANDROID* src);
    VkMemoryGetAndroidHardwareBufferInfoANDROID *ptr() { return reinterpret_cast<VkMemoryGetAndroidHardwareBufferInfoANDROID *>(this); }
    VkMemoryGetAndroidHardwareBufferInfoANDROID const *ptr() const { return reinterpret_cast<VkMemoryGetAndroidHardwareBufferInfoANDROID const *>(this); }
};
#endif // VK_USE_PLATFORM_ANDROID_KHR

#ifdef VK_USE_PLATFORM_ANDROID_KHR
struct safe_VkExternalFormatANDROID {
    VkStructureType sType;
    void* pNext;
    uint64_t externalFormat;
    safe_VkExternalFormatANDROID(const VkExternalFormatANDROID* in_struct);
    safe_VkExternalFormatANDROID(const safe_VkExternalFormatANDROID& src);
    safe_VkExternalFormatANDROID& operator=(const safe_VkExternalFormatANDROID& src);
    safe_VkExternalFormatANDROID();
    ~safe_VkExternalFormatANDROID();
    void initialize(const VkExternalFormatANDROID* in_struct);
    void initialize(const safe_VkExternalFormatANDROID* src);
    VkExternalFormatANDROID *ptr() { return reinterpret_cast<VkExternalFormatANDROID *>(this); }
    VkExternalFormatANDROID const *ptr() const { return reinterpret_cast<VkExternalFormatANDROID const *>(this); }
};
#endif // VK_USE_PLATFORM_ANDROID_KHR

struct safe_VkSamplerReductionModeCreateInfoEXT {
    VkStructureType sType;
    const void* pNext;
    VkSamplerReductionModeEXT reductionMode;
    safe_VkSamplerReductionModeCreateInfoEXT(const VkSamplerReductionModeCreateInfoEXT* in_struct);
    safe_VkSamplerReductionModeCreateInfoEXT(const safe_VkSamplerReductionModeCreateInfoEXT& src);
    safe_VkSamplerReductionModeCreateInfoEXT& operator=(const safe_VkSamplerReductionModeCreateInfoEXT& src);
    safe_VkSamplerReductionModeCreateInfoEXT();
    ~safe_VkSamplerReductionModeCreateInfoEXT();
    void initialize(const VkSamplerReductionModeCreateInfoEXT* in_struct);
    void initialize(const safe_VkSamplerReductionModeCreateInfoEXT* src);
    VkSamplerReductionModeCreateInfoEXT *ptr() { return reinterpret_cast<VkSamplerReductionModeCreateInfoEXT *>(this); }
    VkSamplerReductionModeCreateInfoEXT const *ptr() const { return reinterpret_cast<VkSamplerReductionModeCreateInfoEXT const *>(this); }
};

struct safe_VkPhysicalDeviceSamplerFilterMinmaxPropertiesEXT {
    VkStructureType sType;
    void* pNext;
    VkBool32 filterMinmaxSingleComponentFormats;
    VkBool32 filterMinmaxImageComponentMapping;
    safe_VkPhysicalDeviceSamplerFilterMinmaxPropertiesEXT(const VkPhysicalDeviceSamplerFilterMinmaxPropertiesEXT* in_struct);
    safe_VkPhysicalDeviceSamplerFilterMinmaxPropertiesEXT(const safe_VkPhysicalDeviceSamplerFilterMinmaxPropertiesEXT& src);
    safe_VkPhysicalDeviceSamplerFilterMinmaxPropertiesEXT& operator=(const safe_VkPhysicalDeviceSamplerFilterMinmaxPropertiesEXT& src);
    safe_VkPhysicalDeviceSamplerFilterMinmaxPropertiesEXT();
    ~safe_VkPhysicalDeviceSamplerFilterMinmaxPropertiesEXT();
    void initialize(const VkPhysicalDeviceSamplerFilterMinmaxPropertiesEXT* in_struct);
    void initialize(const safe_VkPhysicalDeviceSamplerFilterMinmaxPropertiesEXT* src);
    VkPhysicalDeviceSamplerFilterMinmaxPropertiesEXT *ptr() { return reinterpret_cast<VkPhysicalDeviceSamplerFilterMinmaxPropertiesEXT *>(this); }
    VkPhysicalDeviceSamplerFilterMinmaxPropertiesEXT const *ptr() const { return reinterpret_cast<VkPhysicalDeviceSamplerFilterMinmaxPropertiesEXT const *>(this); }
};

struct safe_VkPhysicalDeviceInlineUniformBlockFeaturesEXT {
    VkStructureType sType;
    void* pNext;
    VkBool32 inlineUniformBlock;
    VkBool32 descriptorBindingInlineUniformBlockUpdateAfterBind;
    safe_VkPhysicalDeviceInlineUniformBlockFeaturesEXT(const VkPhysicalDeviceInlineUniformBlockFeaturesEXT* in_struct);
    safe_VkPhysicalDeviceInlineUniformBlockFeaturesEXT(const safe_VkPhysicalDeviceInlineUniformBlockFeaturesEXT& src);
    safe_VkPhysicalDeviceInlineUniformBlockFeaturesEXT& operator=(const safe_VkPhysicalDeviceInlineUniformBlockFeaturesEXT& src);
    safe_VkPhysicalDeviceInlineUniformBlockFeaturesEXT();
    ~safe_VkPhysicalDeviceInlineUniformBlockFeaturesEXT();
    void initialize(const VkPhysicalDeviceInlineUniformBlockFeaturesEXT* in_struct);
    void initialize(const safe_VkPhysicalDeviceInlineUniformBlockFeaturesEXT* src);
    VkPhysicalDeviceInlineUniformBlockFeaturesEXT *ptr() { return reinterpret_cast<VkPhysicalDeviceInlineUniformBlockFeaturesEXT *>(this); }
    VkPhysicalDeviceInlineUniformBlockFeaturesEXT const *ptr() const { return reinterpret_cast<VkPhysicalDeviceInlineUniformBlockFeaturesEXT const *>(this); }
};

struct safe_VkPhysicalDeviceInlineUniformBlockPropertiesEXT {
    VkStructureType sType;
    void* pNext;
    uint32_t maxInlineUniformBlockSize;
    uint32_t maxPerStageDescriptorInlineUniformBlocks;
    uint32_t maxPerStageDescriptorUpdateAfterBindInlineUniformBlocks;
    uint32_t maxDescriptorSetInlineUniformBlocks;
    uint32_t maxDescriptorSetUpdateAfterBindInlineUniformBlocks;
    safe_VkPhysicalDeviceInlineUniformBlockPropertiesEXT(const VkPhysicalDeviceInlineUniformBlockPropertiesEXT* in_struct);
    safe_VkPhysicalDeviceInlineUniformBlockPropertiesEXT(const safe_VkPhysicalDeviceInlineUniformBlockPropertiesEXT& src);
    safe_VkPhysicalDeviceInlineUniformBlockPropertiesEXT& operator=(const safe_VkPhysicalDeviceInlineUniformBlockPropertiesEXT& src);
    safe_VkPhysicalDeviceInlineUniformBlockPropertiesEXT();
    ~safe_VkPhysicalDeviceInlineUniformBlockPropertiesEXT();
    void initialize(const VkPhysicalDeviceInlineUniformBlockPropertiesEXT* in_struct);
    void initialize(const safe_VkPhysicalDeviceInlineUniformBlockPropertiesEXT* src);
    VkPhysicalDeviceInlineUniformBlockPropertiesEXT *ptr() { return reinterpret_cast<VkPhysicalDeviceInlineUniformBlockPropertiesEXT *>(this); }
    VkPhysicalDeviceInlineUniformBlockPropertiesEXT const *ptr() const { return reinterpret_cast<VkPhysicalDeviceInlineUniformBlockPropertiesEXT const *>(this); }
};

struct safe_VkWriteDescriptorSetInlineUniformBlockEXT {
    VkStructureType sType;
    const void* pNext;
    uint32_t dataSize;
    const void* pData;
    safe_VkWriteDescriptorSetInlineUniformBlockEXT(const VkWriteDescriptorSetInlineUniformBlockEXT* in_struct);
    safe_VkWriteDescriptorSetInlineUniformBlockEXT(const safe_VkWriteDescriptorSetInlineUniformBlockEXT& src);
    safe_VkWriteDescriptorSetInlineUniformBlockEXT& operator=(const safe_VkWriteDescriptorSetInlineUniformBlockEXT& src);
    safe_VkWriteDescriptorSetInlineUniformBlockEXT();
    ~safe_VkWriteDescriptorSetInlineUniformBlockEXT();
    void initialize(const VkWriteDescriptorSetInlineUniformBlockEXT* in_struct);
    void initialize(const safe_VkWriteDescriptorSetInlineUniformBlockEXT* src);
    VkWriteDescriptorSetInlineUniformBlockEXT *ptr() { return reinterpret_cast<VkWriteDescriptorSetInlineUniformBlockEXT *>(this); }
    VkWriteDescriptorSetInlineUniformBlockEXT const *ptr() const { return reinterpret_cast<VkWriteDescriptorSetInlineUniformBlockEXT const *>(this); }
};

struct safe_VkDescriptorPoolInlineUniformBlockCreateInfoEXT {
    VkStructureType sType;
    const void* pNext;
    uint32_t maxInlineUniformBlockBindings;
    safe_VkDescriptorPoolInlineUniformBlockCreateInfoEXT(const VkDescriptorPoolInlineUniformBlockCreateInfoEXT* in_struct);
    safe_VkDescriptorPoolInlineUniformBlockCreateInfoEXT(const safe_VkDescriptorPoolInlineUniformBlockCreateInfoEXT& src);
    safe_VkDescriptorPoolInlineUniformBlockCreateInfoEXT& operator=(const safe_VkDescriptorPoolInlineUniformBlockCreateInfoEXT& src);
    safe_VkDescriptorPoolInlineUniformBlockCreateInfoEXT();
    ~safe_VkDescriptorPoolInlineUniformBlockCreateInfoEXT();
    void initialize(const VkDescriptorPoolInlineUniformBlockCreateInfoEXT* in_struct);
    void initialize(const safe_VkDescriptorPoolInlineUniformBlockCreateInfoEXT* src);
    VkDescriptorPoolInlineUniformBlockCreateInfoEXT *ptr() { return reinterpret_cast<VkDescriptorPoolInlineUniformBlockCreateInfoEXT *>(this); }
    VkDescriptorPoolInlineUniformBlockCreateInfoEXT const *ptr() const { return reinterpret_cast<VkDescriptorPoolInlineUniformBlockCreateInfoEXT const *>(this); }
};

struct safe_VkSampleLocationsInfoEXT {
    VkStructureType sType;
    const void* pNext;
    VkSampleCountFlagBits sampleLocationsPerPixel;
    VkExtent2D sampleLocationGridSize;
    uint32_t sampleLocationsCount;
    const VkSampleLocationEXT* pSampleLocations;
    safe_VkSampleLocationsInfoEXT(const VkSampleLocationsInfoEXT* in_struct);
    safe_VkSampleLocationsInfoEXT(const safe_VkSampleLocationsInfoEXT& src);
    safe_VkSampleLocationsInfoEXT& operator=(const safe_VkSampleLocationsInfoEXT& src);
    safe_VkSampleLocationsInfoEXT();
    ~safe_VkSampleLocationsInfoEXT();
    void initialize(const VkSampleLocationsInfoEXT* in_struct);
    void initialize(const safe_VkSampleLocationsInfoEXT* src);
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
    safe_VkRenderPassSampleLocationsBeginInfoEXT(const safe_VkRenderPassSampleLocationsBeginInfoEXT& src);
    safe_VkRenderPassSampleLocationsBeginInfoEXT& operator=(const safe_VkRenderPassSampleLocationsBeginInfoEXT& src);
    safe_VkRenderPassSampleLocationsBeginInfoEXT();
    ~safe_VkRenderPassSampleLocationsBeginInfoEXT();
    void initialize(const VkRenderPassSampleLocationsBeginInfoEXT* in_struct);
    void initialize(const safe_VkRenderPassSampleLocationsBeginInfoEXT* src);
    VkRenderPassSampleLocationsBeginInfoEXT *ptr() { return reinterpret_cast<VkRenderPassSampleLocationsBeginInfoEXT *>(this); }
    VkRenderPassSampleLocationsBeginInfoEXT const *ptr() const { return reinterpret_cast<VkRenderPassSampleLocationsBeginInfoEXT const *>(this); }
};

struct safe_VkPipelineSampleLocationsStateCreateInfoEXT {
    VkStructureType sType;
    const void* pNext;
    VkBool32 sampleLocationsEnable;
    safe_VkSampleLocationsInfoEXT sampleLocationsInfo;
    safe_VkPipelineSampleLocationsStateCreateInfoEXT(const VkPipelineSampleLocationsStateCreateInfoEXT* in_struct);
    safe_VkPipelineSampleLocationsStateCreateInfoEXT(const safe_VkPipelineSampleLocationsStateCreateInfoEXT& src);
    safe_VkPipelineSampleLocationsStateCreateInfoEXT& operator=(const safe_VkPipelineSampleLocationsStateCreateInfoEXT& src);
    safe_VkPipelineSampleLocationsStateCreateInfoEXT();
    ~safe_VkPipelineSampleLocationsStateCreateInfoEXT();
    void initialize(const VkPipelineSampleLocationsStateCreateInfoEXT* in_struct);
    void initialize(const safe_VkPipelineSampleLocationsStateCreateInfoEXT* src);
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
    safe_VkPhysicalDeviceSampleLocationsPropertiesEXT(const safe_VkPhysicalDeviceSampleLocationsPropertiesEXT& src);
    safe_VkPhysicalDeviceSampleLocationsPropertiesEXT& operator=(const safe_VkPhysicalDeviceSampleLocationsPropertiesEXT& src);
    safe_VkPhysicalDeviceSampleLocationsPropertiesEXT();
    ~safe_VkPhysicalDeviceSampleLocationsPropertiesEXT();
    void initialize(const VkPhysicalDeviceSampleLocationsPropertiesEXT* in_struct);
    void initialize(const safe_VkPhysicalDeviceSampleLocationsPropertiesEXT* src);
    VkPhysicalDeviceSampleLocationsPropertiesEXT *ptr() { return reinterpret_cast<VkPhysicalDeviceSampleLocationsPropertiesEXT *>(this); }
    VkPhysicalDeviceSampleLocationsPropertiesEXT const *ptr() const { return reinterpret_cast<VkPhysicalDeviceSampleLocationsPropertiesEXT const *>(this); }
};

struct safe_VkMultisamplePropertiesEXT {
    VkStructureType sType;
    void* pNext;
    VkExtent2D maxSampleLocationGridSize;
    safe_VkMultisamplePropertiesEXT(const VkMultisamplePropertiesEXT* in_struct);
    safe_VkMultisamplePropertiesEXT(const safe_VkMultisamplePropertiesEXT& src);
    safe_VkMultisamplePropertiesEXT& operator=(const safe_VkMultisamplePropertiesEXT& src);
    safe_VkMultisamplePropertiesEXT();
    ~safe_VkMultisamplePropertiesEXT();
    void initialize(const VkMultisamplePropertiesEXT* in_struct);
    void initialize(const safe_VkMultisamplePropertiesEXT* src);
    VkMultisamplePropertiesEXT *ptr() { return reinterpret_cast<VkMultisamplePropertiesEXT *>(this); }
    VkMultisamplePropertiesEXT const *ptr() const { return reinterpret_cast<VkMultisamplePropertiesEXT const *>(this); }
};

struct safe_VkPhysicalDeviceBlendOperationAdvancedFeaturesEXT {
    VkStructureType sType;
    void* pNext;
    VkBool32 advancedBlendCoherentOperations;
    safe_VkPhysicalDeviceBlendOperationAdvancedFeaturesEXT(const VkPhysicalDeviceBlendOperationAdvancedFeaturesEXT* in_struct);
    safe_VkPhysicalDeviceBlendOperationAdvancedFeaturesEXT(const safe_VkPhysicalDeviceBlendOperationAdvancedFeaturesEXT& src);
    safe_VkPhysicalDeviceBlendOperationAdvancedFeaturesEXT& operator=(const safe_VkPhysicalDeviceBlendOperationAdvancedFeaturesEXT& src);
    safe_VkPhysicalDeviceBlendOperationAdvancedFeaturesEXT();
    ~safe_VkPhysicalDeviceBlendOperationAdvancedFeaturesEXT();
    void initialize(const VkPhysicalDeviceBlendOperationAdvancedFeaturesEXT* in_struct);
    void initialize(const safe_VkPhysicalDeviceBlendOperationAdvancedFeaturesEXT* src);
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
    safe_VkPhysicalDeviceBlendOperationAdvancedPropertiesEXT(const safe_VkPhysicalDeviceBlendOperationAdvancedPropertiesEXT& src);
    safe_VkPhysicalDeviceBlendOperationAdvancedPropertiesEXT& operator=(const safe_VkPhysicalDeviceBlendOperationAdvancedPropertiesEXT& src);
    safe_VkPhysicalDeviceBlendOperationAdvancedPropertiesEXT();
    ~safe_VkPhysicalDeviceBlendOperationAdvancedPropertiesEXT();
    void initialize(const VkPhysicalDeviceBlendOperationAdvancedPropertiesEXT* in_struct);
    void initialize(const safe_VkPhysicalDeviceBlendOperationAdvancedPropertiesEXT* src);
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
    safe_VkPipelineColorBlendAdvancedStateCreateInfoEXT(const safe_VkPipelineColorBlendAdvancedStateCreateInfoEXT& src);
    safe_VkPipelineColorBlendAdvancedStateCreateInfoEXT& operator=(const safe_VkPipelineColorBlendAdvancedStateCreateInfoEXT& src);
    safe_VkPipelineColorBlendAdvancedStateCreateInfoEXT();
    ~safe_VkPipelineColorBlendAdvancedStateCreateInfoEXT();
    void initialize(const VkPipelineColorBlendAdvancedStateCreateInfoEXT* in_struct);
    void initialize(const safe_VkPipelineColorBlendAdvancedStateCreateInfoEXT* src);
    VkPipelineColorBlendAdvancedStateCreateInfoEXT *ptr() { return reinterpret_cast<VkPipelineColorBlendAdvancedStateCreateInfoEXT *>(this); }
    VkPipelineColorBlendAdvancedStateCreateInfoEXT const *ptr() const { return reinterpret_cast<VkPipelineColorBlendAdvancedStateCreateInfoEXT const *>(this); }
};

struct safe_VkPipelineCoverageToColorStateCreateInfoNV {
    VkStructureType sType;
    const void* pNext;
    VkPipelineCoverageToColorStateCreateFlagsNV flags;
    VkBool32 coverageToColorEnable;
    uint32_t coverageToColorLocation;
    safe_VkPipelineCoverageToColorStateCreateInfoNV(const VkPipelineCoverageToColorStateCreateInfoNV* in_struct);
    safe_VkPipelineCoverageToColorStateCreateInfoNV(const safe_VkPipelineCoverageToColorStateCreateInfoNV& src);
    safe_VkPipelineCoverageToColorStateCreateInfoNV& operator=(const safe_VkPipelineCoverageToColorStateCreateInfoNV& src);
    safe_VkPipelineCoverageToColorStateCreateInfoNV();
    ~safe_VkPipelineCoverageToColorStateCreateInfoNV();
    void initialize(const VkPipelineCoverageToColorStateCreateInfoNV* in_struct);
    void initialize(const safe_VkPipelineCoverageToColorStateCreateInfoNV* src);
    VkPipelineCoverageToColorStateCreateInfoNV *ptr() { return reinterpret_cast<VkPipelineCoverageToColorStateCreateInfoNV *>(this); }
    VkPipelineCoverageToColorStateCreateInfoNV const *ptr() const { return reinterpret_cast<VkPipelineCoverageToColorStateCreateInfoNV const *>(this); }
};

struct safe_VkPipelineCoverageModulationStateCreateInfoNV {
    VkStructureType sType;
    const void* pNext;
    VkPipelineCoverageModulationStateCreateFlagsNV flags;
    VkCoverageModulationModeNV coverageModulationMode;
    VkBool32 coverageModulationTableEnable;
    uint32_t coverageModulationTableCount;
    const float* pCoverageModulationTable;
    safe_VkPipelineCoverageModulationStateCreateInfoNV(const VkPipelineCoverageModulationStateCreateInfoNV* in_struct);
    safe_VkPipelineCoverageModulationStateCreateInfoNV(const safe_VkPipelineCoverageModulationStateCreateInfoNV& src);
    safe_VkPipelineCoverageModulationStateCreateInfoNV& operator=(const safe_VkPipelineCoverageModulationStateCreateInfoNV& src);
    safe_VkPipelineCoverageModulationStateCreateInfoNV();
    ~safe_VkPipelineCoverageModulationStateCreateInfoNV();
    void initialize(const VkPipelineCoverageModulationStateCreateInfoNV* in_struct);
    void initialize(const safe_VkPipelineCoverageModulationStateCreateInfoNV* src);
    VkPipelineCoverageModulationStateCreateInfoNV *ptr() { return reinterpret_cast<VkPipelineCoverageModulationStateCreateInfoNV *>(this); }
    VkPipelineCoverageModulationStateCreateInfoNV const *ptr() const { return reinterpret_cast<VkPipelineCoverageModulationStateCreateInfoNV const *>(this); }
};

struct safe_VkPhysicalDeviceShaderSMBuiltinsPropertiesNV {
    VkStructureType sType;
    void* pNext;
    uint32_t shaderSMCount;
    uint32_t shaderWarpsPerSM;
    safe_VkPhysicalDeviceShaderSMBuiltinsPropertiesNV(const VkPhysicalDeviceShaderSMBuiltinsPropertiesNV* in_struct);
    safe_VkPhysicalDeviceShaderSMBuiltinsPropertiesNV(const safe_VkPhysicalDeviceShaderSMBuiltinsPropertiesNV& src);
    safe_VkPhysicalDeviceShaderSMBuiltinsPropertiesNV& operator=(const safe_VkPhysicalDeviceShaderSMBuiltinsPropertiesNV& src);
    safe_VkPhysicalDeviceShaderSMBuiltinsPropertiesNV();
    ~safe_VkPhysicalDeviceShaderSMBuiltinsPropertiesNV();
    void initialize(const VkPhysicalDeviceShaderSMBuiltinsPropertiesNV* in_struct);
    void initialize(const safe_VkPhysicalDeviceShaderSMBuiltinsPropertiesNV* src);
    VkPhysicalDeviceShaderSMBuiltinsPropertiesNV *ptr() { return reinterpret_cast<VkPhysicalDeviceShaderSMBuiltinsPropertiesNV *>(this); }
    VkPhysicalDeviceShaderSMBuiltinsPropertiesNV const *ptr() const { return reinterpret_cast<VkPhysicalDeviceShaderSMBuiltinsPropertiesNV const *>(this); }
};

struct safe_VkPhysicalDeviceShaderSMBuiltinsFeaturesNV {
    VkStructureType sType;
    void* pNext;
    VkBool32 shaderSMBuiltins;
    safe_VkPhysicalDeviceShaderSMBuiltinsFeaturesNV(const VkPhysicalDeviceShaderSMBuiltinsFeaturesNV* in_struct);
    safe_VkPhysicalDeviceShaderSMBuiltinsFeaturesNV(const safe_VkPhysicalDeviceShaderSMBuiltinsFeaturesNV& src);
    safe_VkPhysicalDeviceShaderSMBuiltinsFeaturesNV& operator=(const safe_VkPhysicalDeviceShaderSMBuiltinsFeaturesNV& src);
    safe_VkPhysicalDeviceShaderSMBuiltinsFeaturesNV();
    ~safe_VkPhysicalDeviceShaderSMBuiltinsFeaturesNV();
    void initialize(const VkPhysicalDeviceShaderSMBuiltinsFeaturesNV* in_struct);
    void initialize(const safe_VkPhysicalDeviceShaderSMBuiltinsFeaturesNV* src);
    VkPhysicalDeviceShaderSMBuiltinsFeaturesNV *ptr() { return reinterpret_cast<VkPhysicalDeviceShaderSMBuiltinsFeaturesNV *>(this); }
    VkPhysicalDeviceShaderSMBuiltinsFeaturesNV const *ptr() const { return reinterpret_cast<VkPhysicalDeviceShaderSMBuiltinsFeaturesNV const *>(this); }
};

struct safe_VkDrmFormatModifierPropertiesListEXT {
    VkStructureType sType;
    void* pNext;
    uint32_t drmFormatModifierCount;
    VkDrmFormatModifierPropertiesEXT* pDrmFormatModifierProperties;
    safe_VkDrmFormatModifierPropertiesListEXT(const VkDrmFormatModifierPropertiesListEXT* in_struct);
    safe_VkDrmFormatModifierPropertiesListEXT(const safe_VkDrmFormatModifierPropertiesListEXT& src);
    safe_VkDrmFormatModifierPropertiesListEXT& operator=(const safe_VkDrmFormatModifierPropertiesListEXT& src);
    safe_VkDrmFormatModifierPropertiesListEXT();
    ~safe_VkDrmFormatModifierPropertiesListEXT();
    void initialize(const VkDrmFormatModifierPropertiesListEXT* in_struct);
    void initialize(const safe_VkDrmFormatModifierPropertiesListEXT* src);
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
    safe_VkPhysicalDeviceImageDrmFormatModifierInfoEXT(const safe_VkPhysicalDeviceImageDrmFormatModifierInfoEXT& src);
    safe_VkPhysicalDeviceImageDrmFormatModifierInfoEXT& operator=(const safe_VkPhysicalDeviceImageDrmFormatModifierInfoEXT& src);
    safe_VkPhysicalDeviceImageDrmFormatModifierInfoEXT();
    ~safe_VkPhysicalDeviceImageDrmFormatModifierInfoEXT();
    void initialize(const VkPhysicalDeviceImageDrmFormatModifierInfoEXT* in_struct);
    void initialize(const safe_VkPhysicalDeviceImageDrmFormatModifierInfoEXT* src);
    VkPhysicalDeviceImageDrmFormatModifierInfoEXT *ptr() { return reinterpret_cast<VkPhysicalDeviceImageDrmFormatModifierInfoEXT *>(this); }
    VkPhysicalDeviceImageDrmFormatModifierInfoEXT const *ptr() const { return reinterpret_cast<VkPhysicalDeviceImageDrmFormatModifierInfoEXT const *>(this); }
};

struct safe_VkImageDrmFormatModifierListCreateInfoEXT {
    VkStructureType sType;
    const void* pNext;
    uint32_t drmFormatModifierCount;
    const uint64_t* pDrmFormatModifiers;
    safe_VkImageDrmFormatModifierListCreateInfoEXT(const VkImageDrmFormatModifierListCreateInfoEXT* in_struct);
    safe_VkImageDrmFormatModifierListCreateInfoEXT(const safe_VkImageDrmFormatModifierListCreateInfoEXT& src);
    safe_VkImageDrmFormatModifierListCreateInfoEXT& operator=(const safe_VkImageDrmFormatModifierListCreateInfoEXT& src);
    safe_VkImageDrmFormatModifierListCreateInfoEXT();
    ~safe_VkImageDrmFormatModifierListCreateInfoEXT();
    void initialize(const VkImageDrmFormatModifierListCreateInfoEXT* in_struct);
    void initialize(const safe_VkImageDrmFormatModifierListCreateInfoEXT* src);
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
    safe_VkImageDrmFormatModifierExplicitCreateInfoEXT(const safe_VkImageDrmFormatModifierExplicitCreateInfoEXT& src);
    safe_VkImageDrmFormatModifierExplicitCreateInfoEXT& operator=(const safe_VkImageDrmFormatModifierExplicitCreateInfoEXT& src);
    safe_VkImageDrmFormatModifierExplicitCreateInfoEXT();
    ~safe_VkImageDrmFormatModifierExplicitCreateInfoEXT();
    void initialize(const VkImageDrmFormatModifierExplicitCreateInfoEXT* in_struct);
    void initialize(const safe_VkImageDrmFormatModifierExplicitCreateInfoEXT* src);
    VkImageDrmFormatModifierExplicitCreateInfoEXT *ptr() { return reinterpret_cast<VkImageDrmFormatModifierExplicitCreateInfoEXT *>(this); }
    VkImageDrmFormatModifierExplicitCreateInfoEXT const *ptr() const { return reinterpret_cast<VkImageDrmFormatModifierExplicitCreateInfoEXT const *>(this); }
};

struct safe_VkImageDrmFormatModifierPropertiesEXT {
    VkStructureType sType;
    void* pNext;
    uint64_t drmFormatModifier;
    safe_VkImageDrmFormatModifierPropertiesEXT(const VkImageDrmFormatModifierPropertiesEXT* in_struct);
    safe_VkImageDrmFormatModifierPropertiesEXT(const safe_VkImageDrmFormatModifierPropertiesEXT& src);
    safe_VkImageDrmFormatModifierPropertiesEXT& operator=(const safe_VkImageDrmFormatModifierPropertiesEXT& src);
    safe_VkImageDrmFormatModifierPropertiesEXT();
    ~safe_VkImageDrmFormatModifierPropertiesEXT();
    void initialize(const VkImageDrmFormatModifierPropertiesEXT* in_struct);
    void initialize(const safe_VkImageDrmFormatModifierPropertiesEXT* src);
    VkImageDrmFormatModifierPropertiesEXT *ptr() { return reinterpret_cast<VkImageDrmFormatModifierPropertiesEXT *>(this); }
    VkImageDrmFormatModifierPropertiesEXT const *ptr() const { return reinterpret_cast<VkImageDrmFormatModifierPropertiesEXT const *>(this); }
};

struct safe_VkValidationCacheCreateInfoEXT {
    VkStructureType sType;
    const void* pNext;
    VkValidationCacheCreateFlagsEXT flags;
    size_t initialDataSize;
    const void* pInitialData;
    safe_VkValidationCacheCreateInfoEXT(const VkValidationCacheCreateInfoEXT* in_struct);
    safe_VkValidationCacheCreateInfoEXT(const safe_VkValidationCacheCreateInfoEXT& src);
    safe_VkValidationCacheCreateInfoEXT& operator=(const safe_VkValidationCacheCreateInfoEXT& src);
    safe_VkValidationCacheCreateInfoEXT();
    ~safe_VkValidationCacheCreateInfoEXT();
    void initialize(const VkValidationCacheCreateInfoEXT* in_struct);
    void initialize(const safe_VkValidationCacheCreateInfoEXT* src);
    VkValidationCacheCreateInfoEXT *ptr() { return reinterpret_cast<VkValidationCacheCreateInfoEXT *>(this); }
    VkValidationCacheCreateInfoEXT const *ptr() const { return reinterpret_cast<VkValidationCacheCreateInfoEXT const *>(this); }
};

struct safe_VkShaderModuleValidationCacheCreateInfoEXT {
    VkStructureType sType;
    const void* pNext;
    VkValidationCacheEXT validationCache;
    safe_VkShaderModuleValidationCacheCreateInfoEXT(const VkShaderModuleValidationCacheCreateInfoEXT* in_struct);
    safe_VkShaderModuleValidationCacheCreateInfoEXT(const safe_VkShaderModuleValidationCacheCreateInfoEXT& src);
    safe_VkShaderModuleValidationCacheCreateInfoEXT& operator=(const safe_VkShaderModuleValidationCacheCreateInfoEXT& src);
    safe_VkShaderModuleValidationCacheCreateInfoEXT();
    ~safe_VkShaderModuleValidationCacheCreateInfoEXT();
    void initialize(const VkShaderModuleValidationCacheCreateInfoEXT* in_struct);
    void initialize(const safe_VkShaderModuleValidationCacheCreateInfoEXT* src);
    VkShaderModuleValidationCacheCreateInfoEXT *ptr() { return reinterpret_cast<VkShaderModuleValidationCacheCreateInfoEXT *>(this); }
    VkShaderModuleValidationCacheCreateInfoEXT const *ptr() const { return reinterpret_cast<VkShaderModuleValidationCacheCreateInfoEXT const *>(this); }
};

struct safe_VkDescriptorSetLayoutBindingFlagsCreateInfoEXT {
    VkStructureType sType;
    const void* pNext;
    uint32_t bindingCount;
    const VkDescriptorBindingFlagsEXT* pBindingFlags;
    safe_VkDescriptorSetLayoutBindingFlagsCreateInfoEXT(const VkDescriptorSetLayoutBindingFlagsCreateInfoEXT* in_struct);
    safe_VkDescriptorSetLayoutBindingFlagsCreateInfoEXT(const safe_VkDescriptorSetLayoutBindingFlagsCreateInfoEXT& src);
    safe_VkDescriptorSetLayoutBindingFlagsCreateInfoEXT& operator=(const safe_VkDescriptorSetLayoutBindingFlagsCreateInfoEXT& src);
    safe_VkDescriptorSetLayoutBindingFlagsCreateInfoEXT();
    ~safe_VkDescriptorSetLayoutBindingFlagsCreateInfoEXT();
    void initialize(const VkDescriptorSetLayoutBindingFlagsCreateInfoEXT* in_struct);
    void initialize(const safe_VkDescriptorSetLayoutBindingFlagsCreateInfoEXT* src);
    VkDescriptorSetLayoutBindingFlagsCreateInfoEXT *ptr() { return reinterpret_cast<VkDescriptorSetLayoutBindingFlagsCreateInfoEXT *>(this); }
    VkDescriptorSetLayoutBindingFlagsCreateInfoEXT const *ptr() const { return reinterpret_cast<VkDescriptorSetLayoutBindingFlagsCreateInfoEXT const *>(this); }
};

struct safe_VkPhysicalDeviceDescriptorIndexingFeaturesEXT {
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
    safe_VkPhysicalDeviceDescriptorIndexingFeaturesEXT(const VkPhysicalDeviceDescriptorIndexingFeaturesEXT* in_struct);
    safe_VkPhysicalDeviceDescriptorIndexingFeaturesEXT(const safe_VkPhysicalDeviceDescriptorIndexingFeaturesEXT& src);
    safe_VkPhysicalDeviceDescriptorIndexingFeaturesEXT& operator=(const safe_VkPhysicalDeviceDescriptorIndexingFeaturesEXT& src);
    safe_VkPhysicalDeviceDescriptorIndexingFeaturesEXT();
    ~safe_VkPhysicalDeviceDescriptorIndexingFeaturesEXT();
    void initialize(const VkPhysicalDeviceDescriptorIndexingFeaturesEXT* in_struct);
    void initialize(const safe_VkPhysicalDeviceDescriptorIndexingFeaturesEXT* src);
    VkPhysicalDeviceDescriptorIndexingFeaturesEXT *ptr() { return reinterpret_cast<VkPhysicalDeviceDescriptorIndexingFeaturesEXT *>(this); }
    VkPhysicalDeviceDescriptorIndexingFeaturesEXT const *ptr() const { return reinterpret_cast<VkPhysicalDeviceDescriptorIndexingFeaturesEXT const *>(this); }
};

struct safe_VkPhysicalDeviceDescriptorIndexingPropertiesEXT {
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
    safe_VkPhysicalDeviceDescriptorIndexingPropertiesEXT(const VkPhysicalDeviceDescriptorIndexingPropertiesEXT* in_struct);
    safe_VkPhysicalDeviceDescriptorIndexingPropertiesEXT(const safe_VkPhysicalDeviceDescriptorIndexingPropertiesEXT& src);
    safe_VkPhysicalDeviceDescriptorIndexingPropertiesEXT& operator=(const safe_VkPhysicalDeviceDescriptorIndexingPropertiesEXT& src);
    safe_VkPhysicalDeviceDescriptorIndexingPropertiesEXT();
    ~safe_VkPhysicalDeviceDescriptorIndexingPropertiesEXT();
    void initialize(const VkPhysicalDeviceDescriptorIndexingPropertiesEXT* in_struct);
    void initialize(const safe_VkPhysicalDeviceDescriptorIndexingPropertiesEXT* src);
    VkPhysicalDeviceDescriptorIndexingPropertiesEXT *ptr() { return reinterpret_cast<VkPhysicalDeviceDescriptorIndexingPropertiesEXT *>(this); }
    VkPhysicalDeviceDescriptorIndexingPropertiesEXT const *ptr() const { return reinterpret_cast<VkPhysicalDeviceDescriptorIndexingPropertiesEXT const *>(this); }
};

struct safe_VkDescriptorSetVariableDescriptorCountAllocateInfoEXT {
    VkStructureType sType;
    const void* pNext;
    uint32_t descriptorSetCount;
    const uint32_t* pDescriptorCounts;
    safe_VkDescriptorSetVariableDescriptorCountAllocateInfoEXT(const VkDescriptorSetVariableDescriptorCountAllocateInfoEXT* in_struct);
    safe_VkDescriptorSetVariableDescriptorCountAllocateInfoEXT(const safe_VkDescriptorSetVariableDescriptorCountAllocateInfoEXT& src);
    safe_VkDescriptorSetVariableDescriptorCountAllocateInfoEXT& operator=(const safe_VkDescriptorSetVariableDescriptorCountAllocateInfoEXT& src);
    safe_VkDescriptorSetVariableDescriptorCountAllocateInfoEXT();
    ~safe_VkDescriptorSetVariableDescriptorCountAllocateInfoEXT();
    void initialize(const VkDescriptorSetVariableDescriptorCountAllocateInfoEXT* in_struct);
    void initialize(const safe_VkDescriptorSetVariableDescriptorCountAllocateInfoEXT* src);
    VkDescriptorSetVariableDescriptorCountAllocateInfoEXT *ptr() { return reinterpret_cast<VkDescriptorSetVariableDescriptorCountAllocateInfoEXT *>(this); }
    VkDescriptorSetVariableDescriptorCountAllocateInfoEXT const *ptr() const { return reinterpret_cast<VkDescriptorSetVariableDescriptorCountAllocateInfoEXT const *>(this); }
};

struct safe_VkDescriptorSetVariableDescriptorCountLayoutSupportEXT {
    VkStructureType sType;
    void* pNext;
    uint32_t maxVariableDescriptorCount;
    safe_VkDescriptorSetVariableDescriptorCountLayoutSupportEXT(const VkDescriptorSetVariableDescriptorCountLayoutSupportEXT* in_struct);
    safe_VkDescriptorSetVariableDescriptorCountLayoutSupportEXT(const safe_VkDescriptorSetVariableDescriptorCountLayoutSupportEXT& src);
    safe_VkDescriptorSetVariableDescriptorCountLayoutSupportEXT& operator=(const safe_VkDescriptorSetVariableDescriptorCountLayoutSupportEXT& src);
    safe_VkDescriptorSetVariableDescriptorCountLayoutSupportEXT();
    ~safe_VkDescriptorSetVariableDescriptorCountLayoutSupportEXT();
    void initialize(const VkDescriptorSetVariableDescriptorCountLayoutSupportEXT* in_struct);
    void initialize(const safe_VkDescriptorSetVariableDescriptorCountLayoutSupportEXT* src);
    VkDescriptorSetVariableDescriptorCountLayoutSupportEXT *ptr() { return reinterpret_cast<VkDescriptorSetVariableDescriptorCountLayoutSupportEXT *>(this); }
    VkDescriptorSetVariableDescriptorCountLayoutSupportEXT const *ptr() const { return reinterpret_cast<VkDescriptorSetVariableDescriptorCountLayoutSupportEXT const *>(this); }
};

struct safe_VkShadingRatePaletteNV {
    uint32_t shadingRatePaletteEntryCount;
    const VkShadingRatePaletteEntryNV* pShadingRatePaletteEntries;
    safe_VkShadingRatePaletteNV(const VkShadingRatePaletteNV* in_struct);
    safe_VkShadingRatePaletteNV(const safe_VkShadingRatePaletteNV& src);
    safe_VkShadingRatePaletteNV& operator=(const safe_VkShadingRatePaletteNV& src);
    safe_VkShadingRatePaletteNV();
    ~safe_VkShadingRatePaletteNV();
    void initialize(const VkShadingRatePaletteNV* in_struct);
    void initialize(const safe_VkShadingRatePaletteNV* src);
    VkShadingRatePaletteNV *ptr() { return reinterpret_cast<VkShadingRatePaletteNV *>(this); }
    VkShadingRatePaletteNV const *ptr() const { return reinterpret_cast<VkShadingRatePaletteNV const *>(this); }
};

struct safe_VkPipelineViewportShadingRateImageStateCreateInfoNV {
    VkStructureType sType;
    const void* pNext;
    VkBool32 shadingRateImageEnable;
    uint32_t viewportCount;
    safe_VkShadingRatePaletteNV* pShadingRatePalettes;
    safe_VkPipelineViewportShadingRateImageStateCreateInfoNV(const VkPipelineViewportShadingRateImageStateCreateInfoNV* in_struct);
    safe_VkPipelineViewportShadingRateImageStateCreateInfoNV(const safe_VkPipelineViewportShadingRateImageStateCreateInfoNV& src);
    safe_VkPipelineViewportShadingRateImageStateCreateInfoNV& operator=(const safe_VkPipelineViewportShadingRateImageStateCreateInfoNV& src);
    safe_VkPipelineViewportShadingRateImageStateCreateInfoNV();
    ~safe_VkPipelineViewportShadingRateImageStateCreateInfoNV();
    void initialize(const VkPipelineViewportShadingRateImageStateCreateInfoNV* in_struct);
    void initialize(const safe_VkPipelineViewportShadingRateImageStateCreateInfoNV* src);
    VkPipelineViewportShadingRateImageStateCreateInfoNV *ptr() { return reinterpret_cast<VkPipelineViewportShadingRateImageStateCreateInfoNV *>(this); }
    VkPipelineViewportShadingRateImageStateCreateInfoNV const *ptr() const { return reinterpret_cast<VkPipelineViewportShadingRateImageStateCreateInfoNV const *>(this); }
};

struct safe_VkPhysicalDeviceShadingRateImageFeaturesNV {
    VkStructureType sType;
    void* pNext;
    VkBool32 shadingRateImage;
    VkBool32 shadingRateCoarseSampleOrder;
    safe_VkPhysicalDeviceShadingRateImageFeaturesNV(const VkPhysicalDeviceShadingRateImageFeaturesNV* in_struct);
    safe_VkPhysicalDeviceShadingRateImageFeaturesNV(const safe_VkPhysicalDeviceShadingRateImageFeaturesNV& src);
    safe_VkPhysicalDeviceShadingRateImageFeaturesNV& operator=(const safe_VkPhysicalDeviceShadingRateImageFeaturesNV& src);
    safe_VkPhysicalDeviceShadingRateImageFeaturesNV();
    ~safe_VkPhysicalDeviceShadingRateImageFeaturesNV();
    void initialize(const VkPhysicalDeviceShadingRateImageFeaturesNV* in_struct);
    void initialize(const safe_VkPhysicalDeviceShadingRateImageFeaturesNV* src);
    VkPhysicalDeviceShadingRateImageFeaturesNV *ptr() { return reinterpret_cast<VkPhysicalDeviceShadingRateImageFeaturesNV *>(this); }
    VkPhysicalDeviceShadingRateImageFeaturesNV const *ptr() const { return reinterpret_cast<VkPhysicalDeviceShadingRateImageFeaturesNV const *>(this); }
};

struct safe_VkPhysicalDeviceShadingRateImagePropertiesNV {
    VkStructureType sType;
    void* pNext;
    VkExtent2D shadingRateTexelSize;
    uint32_t shadingRatePaletteSize;
    uint32_t shadingRateMaxCoarseSamples;
    safe_VkPhysicalDeviceShadingRateImagePropertiesNV(const VkPhysicalDeviceShadingRateImagePropertiesNV* in_struct);
    safe_VkPhysicalDeviceShadingRateImagePropertiesNV(const safe_VkPhysicalDeviceShadingRateImagePropertiesNV& src);
    safe_VkPhysicalDeviceShadingRateImagePropertiesNV& operator=(const safe_VkPhysicalDeviceShadingRateImagePropertiesNV& src);
    safe_VkPhysicalDeviceShadingRateImagePropertiesNV();
    ~safe_VkPhysicalDeviceShadingRateImagePropertiesNV();
    void initialize(const VkPhysicalDeviceShadingRateImagePropertiesNV* in_struct);
    void initialize(const safe_VkPhysicalDeviceShadingRateImagePropertiesNV* src);
    VkPhysicalDeviceShadingRateImagePropertiesNV *ptr() { return reinterpret_cast<VkPhysicalDeviceShadingRateImagePropertiesNV *>(this); }
    VkPhysicalDeviceShadingRateImagePropertiesNV const *ptr() const { return reinterpret_cast<VkPhysicalDeviceShadingRateImagePropertiesNV const *>(this); }
};

struct safe_VkCoarseSampleOrderCustomNV {
    VkShadingRatePaletteEntryNV shadingRate;
    uint32_t sampleCount;
    uint32_t sampleLocationCount;
    const VkCoarseSampleLocationNV* pSampleLocations;
    safe_VkCoarseSampleOrderCustomNV(const VkCoarseSampleOrderCustomNV* in_struct);
    safe_VkCoarseSampleOrderCustomNV(const safe_VkCoarseSampleOrderCustomNV& src);
    safe_VkCoarseSampleOrderCustomNV& operator=(const safe_VkCoarseSampleOrderCustomNV& src);
    safe_VkCoarseSampleOrderCustomNV();
    ~safe_VkCoarseSampleOrderCustomNV();
    void initialize(const VkCoarseSampleOrderCustomNV* in_struct);
    void initialize(const safe_VkCoarseSampleOrderCustomNV* src);
    VkCoarseSampleOrderCustomNV *ptr() { return reinterpret_cast<VkCoarseSampleOrderCustomNV *>(this); }
    VkCoarseSampleOrderCustomNV const *ptr() const { return reinterpret_cast<VkCoarseSampleOrderCustomNV const *>(this); }
};

struct safe_VkPipelineViewportCoarseSampleOrderStateCreateInfoNV {
    VkStructureType sType;
    const void* pNext;
    VkCoarseSampleOrderTypeNV sampleOrderType;
    uint32_t customSampleOrderCount;
    safe_VkCoarseSampleOrderCustomNV* pCustomSampleOrders;
    safe_VkPipelineViewportCoarseSampleOrderStateCreateInfoNV(const VkPipelineViewportCoarseSampleOrderStateCreateInfoNV* in_struct);
    safe_VkPipelineViewportCoarseSampleOrderStateCreateInfoNV(const safe_VkPipelineViewportCoarseSampleOrderStateCreateInfoNV& src);
    safe_VkPipelineViewportCoarseSampleOrderStateCreateInfoNV& operator=(const safe_VkPipelineViewportCoarseSampleOrderStateCreateInfoNV& src);
    safe_VkPipelineViewportCoarseSampleOrderStateCreateInfoNV();
    ~safe_VkPipelineViewportCoarseSampleOrderStateCreateInfoNV();
    void initialize(const VkPipelineViewportCoarseSampleOrderStateCreateInfoNV* in_struct);
    void initialize(const safe_VkPipelineViewportCoarseSampleOrderStateCreateInfoNV* src);
    VkPipelineViewportCoarseSampleOrderStateCreateInfoNV *ptr() { return reinterpret_cast<VkPipelineViewportCoarseSampleOrderStateCreateInfoNV *>(this); }
    VkPipelineViewportCoarseSampleOrderStateCreateInfoNV const *ptr() const { return reinterpret_cast<VkPipelineViewportCoarseSampleOrderStateCreateInfoNV const *>(this); }
};

struct safe_VkRayTracingShaderGroupCreateInfoNV {
    VkStructureType sType;
    const void* pNext;
    VkRayTracingShaderGroupTypeNV type;
    uint32_t generalShader;
    uint32_t closestHitShader;
    uint32_t anyHitShader;
    uint32_t intersectionShader;
    safe_VkRayTracingShaderGroupCreateInfoNV(const VkRayTracingShaderGroupCreateInfoNV* in_struct);
    safe_VkRayTracingShaderGroupCreateInfoNV(const safe_VkRayTracingShaderGroupCreateInfoNV& src);
    safe_VkRayTracingShaderGroupCreateInfoNV& operator=(const safe_VkRayTracingShaderGroupCreateInfoNV& src);
    safe_VkRayTracingShaderGroupCreateInfoNV();
    ~safe_VkRayTracingShaderGroupCreateInfoNV();
    void initialize(const VkRayTracingShaderGroupCreateInfoNV* in_struct);
    void initialize(const safe_VkRayTracingShaderGroupCreateInfoNV* src);
    VkRayTracingShaderGroupCreateInfoNV *ptr() { return reinterpret_cast<VkRayTracingShaderGroupCreateInfoNV *>(this); }
    VkRayTracingShaderGroupCreateInfoNV const *ptr() const { return reinterpret_cast<VkRayTracingShaderGroupCreateInfoNV const *>(this); }
};

struct safe_VkRayTracingPipelineCreateInfoNV {
    VkStructureType sType;
    const void* pNext;
    VkPipelineCreateFlags flags;
    uint32_t stageCount;
    safe_VkPipelineShaderStageCreateInfo* pStages;
    uint32_t groupCount;
    safe_VkRayTracingShaderGroupCreateInfoNV* pGroups;
    uint32_t maxRecursionDepth;
    VkPipelineLayout layout;
    VkPipeline basePipelineHandle;
    int32_t basePipelineIndex;
    safe_VkRayTracingPipelineCreateInfoNV(const VkRayTracingPipelineCreateInfoNV* in_struct);
    safe_VkRayTracingPipelineCreateInfoNV(const safe_VkRayTracingPipelineCreateInfoNV& src);
    safe_VkRayTracingPipelineCreateInfoNV& operator=(const safe_VkRayTracingPipelineCreateInfoNV& src);
    safe_VkRayTracingPipelineCreateInfoNV();
    ~safe_VkRayTracingPipelineCreateInfoNV();
    void initialize(const VkRayTracingPipelineCreateInfoNV* in_struct);
    void initialize(const safe_VkRayTracingPipelineCreateInfoNV* src);
    VkRayTracingPipelineCreateInfoNV *ptr() { return reinterpret_cast<VkRayTracingPipelineCreateInfoNV *>(this); }
    VkRayTracingPipelineCreateInfoNV const *ptr() const { return reinterpret_cast<VkRayTracingPipelineCreateInfoNV const *>(this); }
};

struct safe_VkGeometryTrianglesNV {
    VkStructureType sType;
    const void* pNext;
    VkBuffer vertexData;
    VkDeviceSize vertexOffset;
    uint32_t vertexCount;
    VkDeviceSize vertexStride;
    VkFormat vertexFormat;
    VkBuffer indexData;
    VkDeviceSize indexOffset;
    uint32_t indexCount;
    VkIndexType indexType;
    VkBuffer transformData;
    VkDeviceSize transformOffset;
    safe_VkGeometryTrianglesNV(const VkGeometryTrianglesNV* in_struct);
    safe_VkGeometryTrianglesNV(const safe_VkGeometryTrianglesNV& src);
    safe_VkGeometryTrianglesNV& operator=(const safe_VkGeometryTrianglesNV& src);
    safe_VkGeometryTrianglesNV();
    ~safe_VkGeometryTrianglesNV();
    void initialize(const VkGeometryTrianglesNV* in_struct);
    void initialize(const safe_VkGeometryTrianglesNV* src);
    VkGeometryTrianglesNV *ptr() { return reinterpret_cast<VkGeometryTrianglesNV *>(this); }
    VkGeometryTrianglesNV const *ptr() const { return reinterpret_cast<VkGeometryTrianglesNV const *>(this); }
};

struct safe_VkGeometryAABBNV {
    VkStructureType sType;
    const void* pNext;
    VkBuffer aabbData;
    uint32_t numAABBs;
    uint32_t stride;
    VkDeviceSize offset;
    safe_VkGeometryAABBNV(const VkGeometryAABBNV* in_struct);
    safe_VkGeometryAABBNV(const safe_VkGeometryAABBNV& src);
    safe_VkGeometryAABBNV& operator=(const safe_VkGeometryAABBNV& src);
    safe_VkGeometryAABBNV();
    ~safe_VkGeometryAABBNV();
    void initialize(const VkGeometryAABBNV* in_struct);
    void initialize(const safe_VkGeometryAABBNV* src);
    VkGeometryAABBNV *ptr() { return reinterpret_cast<VkGeometryAABBNV *>(this); }
    VkGeometryAABBNV const *ptr() const { return reinterpret_cast<VkGeometryAABBNV const *>(this); }
};

struct safe_VkGeometryNV {
    VkStructureType sType;
    const void* pNext;
    VkGeometryTypeNV geometryType;
    VkGeometryDataNV geometry;
    VkGeometryFlagsNV flags;
    safe_VkGeometryNV(const VkGeometryNV* in_struct);
    safe_VkGeometryNV(const safe_VkGeometryNV& src);
    safe_VkGeometryNV& operator=(const safe_VkGeometryNV& src);
    safe_VkGeometryNV();
    ~safe_VkGeometryNV();
    void initialize(const VkGeometryNV* in_struct);
    void initialize(const safe_VkGeometryNV* src);
    VkGeometryNV *ptr() { return reinterpret_cast<VkGeometryNV *>(this); }
    VkGeometryNV const *ptr() const { return reinterpret_cast<VkGeometryNV const *>(this); }
};

struct safe_VkAccelerationStructureInfoNV {
    VkStructureType sType;
    const void* pNext;
    VkAccelerationStructureTypeNV type;
    VkBuildAccelerationStructureFlagsNV flags;
    uint32_t instanceCount;
    uint32_t geometryCount;
    safe_VkGeometryNV* pGeometries;
    safe_VkAccelerationStructureInfoNV(const VkAccelerationStructureInfoNV* in_struct);
    safe_VkAccelerationStructureInfoNV(const safe_VkAccelerationStructureInfoNV& src);
    safe_VkAccelerationStructureInfoNV& operator=(const safe_VkAccelerationStructureInfoNV& src);
    safe_VkAccelerationStructureInfoNV();
    ~safe_VkAccelerationStructureInfoNV();
    void initialize(const VkAccelerationStructureInfoNV* in_struct);
    void initialize(const safe_VkAccelerationStructureInfoNV* src);
    VkAccelerationStructureInfoNV *ptr() { return reinterpret_cast<VkAccelerationStructureInfoNV *>(this); }
    VkAccelerationStructureInfoNV const *ptr() const { return reinterpret_cast<VkAccelerationStructureInfoNV const *>(this); }
};

struct safe_VkAccelerationStructureCreateInfoNV {
    VkStructureType sType;
    const void* pNext;
    VkDeviceSize compactedSize;
    safe_VkAccelerationStructureInfoNV info;
    safe_VkAccelerationStructureCreateInfoNV(const VkAccelerationStructureCreateInfoNV* in_struct);
    safe_VkAccelerationStructureCreateInfoNV(const safe_VkAccelerationStructureCreateInfoNV& src);
    safe_VkAccelerationStructureCreateInfoNV& operator=(const safe_VkAccelerationStructureCreateInfoNV& src);
    safe_VkAccelerationStructureCreateInfoNV();
    ~safe_VkAccelerationStructureCreateInfoNV();
    void initialize(const VkAccelerationStructureCreateInfoNV* in_struct);
    void initialize(const safe_VkAccelerationStructureCreateInfoNV* src);
    VkAccelerationStructureCreateInfoNV *ptr() { return reinterpret_cast<VkAccelerationStructureCreateInfoNV *>(this); }
    VkAccelerationStructureCreateInfoNV const *ptr() const { return reinterpret_cast<VkAccelerationStructureCreateInfoNV const *>(this); }
};

struct safe_VkBindAccelerationStructureMemoryInfoNV {
    VkStructureType sType;
    const void* pNext;
    VkAccelerationStructureNV accelerationStructure;
    VkDeviceMemory memory;
    VkDeviceSize memoryOffset;
    uint32_t deviceIndexCount;
    const uint32_t* pDeviceIndices;
    safe_VkBindAccelerationStructureMemoryInfoNV(const VkBindAccelerationStructureMemoryInfoNV* in_struct);
    safe_VkBindAccelerationStructureMemoryInfoNV(const safe_VkBindAccelerationStructureMemoryInfoNV& src);
    safe_VkBindAccelerationStructureMemoryInfoNV& operator=(const safe_VkBindAccelerationStructureMemoryInfoNV& src);
    safe_VkBindAccelerationStructureMemoryInfoNV();
    ~safe_VkBindAccelerationStructureMemoryInfoNV();
    void initialize(const VkBindAccelerationStructureMemoryInfoNV* in_struct);
    void initialize(const safe_VkBindAccelerationStructureMemoryInfoNV* src);
    VkBindAccelerationStructureMemoryInfoNV *ptr() { return reinterpret_cast<VkBindAccelerationStructureMemoryInfoNV *>(this); }
    VkBindAccelerationStructureMemoryInfoNV const *ptr() const { return reinterpret_cast<VkBindAccelerationStructureMemoryInfoNV const *>(this); }
};

struct safe_VkWriteDescriptorSetAccelerationStructureNV {
    VkStructureType sType;
    const void* pNext;
    uint32_t accelerationStructureCount;
    VkAccelerationStructureNV* pAccelerationStructures;
    safe_VkWriteDescriptorSetAccelerationStructureNV(const VkWriteDescriptorSetAccelerationStructureNV* in_struct);
    safe_VkWriteDescriptorSetAccelerationStructureNV(const safe_VkWriteDescriptorSetAccelerationStructureNV& src);
    safe_VkWriteDescriptorSetAccelerationStructureNV& operator=(const safe_VkWriteDescriptorSetAccelerationStructureNV& src);
    safe_VkWriteDescriptorSetAccelerationStructureNV();
    ~safe_VkWriteDescriptorSetAccelerationStructureNV();
    void initialize(const VkWriteDescriptorSetAccelerationStructureNV* in_struct);
    void initialize(const safe_VkWriteDescriptorSetAccelerationStructureNV* src);
    VkWriteDescriptorSetAccelerationStructureNV *ptr() { return reinterpret_cast<VkWriteDescriptorSetAccelerationStructureNV *>(this); }
    VkWriteDescriptorSetAccelerationStructureNV const *ptr() const { return reinterpret_cast<VkWriteDescriptorSetAccelerationStructureNV const *>(this); }
};

struct safe_VkAccelerationStructureMemoryRequirementsInfoNV {
    VkStructureType sType;
    const void* pNext;
    VkAccelerationStructureMemoryRequirementsTypeNV type;
    VkAccelerationStructureNV accelerationStructure;
    safe_VkAccelerationStructureMemoryRequirementsInfoNV(const VkAccelerationStructureMemoryRequirementsInfoNV* in_struct);
    safe_VkAccelerationStructureMemoryRequirementsInfoNV(const safe_VkAccelerationStructureMemoryRequirementsInfoNV& src);
    safe_VkAccelerationStructureMemoryRequirementsInfoNV& operator=(const safe_VkAccelerationStructureMemoryRequirementsInfoNV& src);
    safe_VkAccelerationStructureMemoryRequirementsInfoNV();
    ~safe_VkAccelerationStructureMemoryRequirementsInfoNV();
    void initialize(const VkAccelerationStructureMemoryRequirementsInfoNV* in_struct);
    void initialize(const safe_VkAccelerationStructureMemoryRequirementsInfoNV* src);
    VkAccelerationStructureMemoryRequirementsInfoNV *ptr() { return reinterpret_cast<VkAccelerationStructureMemoryRequirementsInfoNV *>(this); }
    VkAccelerationStructureMemoryRequirementsInfoNV const *ptr() const { return reinterpret_cast<VkAccelerationStructureMemoryRequirementsInfoNV const *>(this); }
};

struct safe_VkPhysicalDeviceRayTracingPropertiesNV {
    VkStructureType sType;
    void* pNext;
    uint32_t shaderGroupHandleSize;
    uint32_t maxRecursionDepth;
    uint32_t maxShaderGroupStride;
    uint32_t shaderGroupBaseAlignment;
    uint64_t maxGeometryCount;
    uint64_t maxInstanceCount;
    uint64_t maxTriangleCount;
    uint32_t maxDescriptorSetAccelerationStructures;
    safe_VkPhysicalDeviceRayTracingPropertiesNV(const VkPhysicalDeviceRayTracingPropertiesNV* in_struct);
    safe_VkPhysicalDeviceRayTracingPropertiesNV(const safe_VkPhysicalDeviceRayTracingPropertiesNV& src);
    safe_VkPhysicalDeviceRayTracingPropertiesNV& operator=(const safe_VkPhysicalDeviceRayTracingPropertiesNV& src);
    safe_VkPhysicalDeviceRayTracingPropertiesNV();
    ~safe_VkPhysicalDeviceRayTracingPropertiesNV();
    void initialize(const VkPhysicalDeviceRayTracingPropertiesNV* in_struct);
    void initialize(const safe_VkPhysicalDeviceRayTracingPropertiesNV* src);
    VkPhysicalDeviceRayTracingPropertiesNV *ptr() { return reinterpret_cast<VkPhysicalDeviceRayTracingPropertiesNV *>(this); }
    VkPhysicalDeviceRayTracingPropertiesNV const *ptr() const { return reinterpret_cast<VkPhysicalDeviceRayTracingPropertiesNV const *>(this); }
};

struct safe_VkPhysicalDeviceRepresentativeFragmentTestFeaturesNV {
    VkStructureType sType;
    void* pNext;
    VkBool32 representativeFragmentTest;
    safe_VkPhysicalDeviceRepresentativeFragmentTestFeaturesNV(const VkPhysicalDeviceRepresentativeFragmentTestFeaturesNV* in_struct);
    safe_VkPhysicalDeviceRepresentativeFragmentTestFeaturesNV(const safe_VkPhysicalDeviceRepresentativeFragmentTestFeaturesNV& src);
    safe_VkPhysicalDeviceRepresentativeFragmentTestFeaturesNV& operator=(const safe_VkPhysicalDeviceRepresentativeFragmentTestFeaturesNV& src);
    safe_VkPhysicalDeviceRepresentativeFragmentTestFeaturesNV();
    ~safe_VkPhysicalDeviceRepresentativeFragmentTestFeaturesNV();
    void initialize(const VkPhysicalDeviceRepresentativeFragmentTestFeaturesNV* in_struct);
    void initialize(const safe_VkPhysicalDeviceRepresentativeFragmentTestFeaturesNV* src);
    VkPhysicalDeviceRepresentativeFragmentTestFeaturesNV *ptr() { return reinterpret_cast<VkPhysicalDeviceRepresentativeFragmentTestFeaturesNV *>(this); }
    VkPhysicalDeviceRepresentativeFragmentTestFeaturesNV const *ptr() const { return reinterpret_cast<VkPhysicalDeviceRepresentativeFragmentTestFeaturesNV const *>(this); }
};

struct safe_VkPipelineRepresentativeFragmentTestStateCreateInfoNV {
    VkStructureType sType;
    const void* pNext;
    VkBool32 representativeFragmentTestEnable;
    safe_VkPipelineRepresentativeFragmentTestStateCreateInfoNV(const VkPipelineRepresentativeFragmentTestStateCreateInfoNV* in_struct);
    safe_VkPipelineRepresentativeFragmentTestStateCreateInfoNV(const safe_VkPipelineRepresentativeFragmentTestStateCreateInfoNV& src);
    safe_VkPipelineRepresentativeFragmentTestStateCreateInfoNV& operator=(const safe_VkPipelineRepresentativeFragmentTestStateCreateInfoNV& src);
    safe_VkPipelineRepresentativeFragmentTestStateCreateInfoNV();
    ~safe_VkPipelineRepresentativeFragmentTestStateCreateInfoNV();
    void initialize(const VkPipelineRepresentativeFragmentTestStateCreateInfoNV* in_struct);
    void initialize(const safe_VkPipelineRepresentativeFragmentTestStateCreateInfoNV* src);
    VkPipelineRepresentativeFragmentTestStateCreateInfoNV *ptr() { return reinterpret_cast<VkPipelineRepresentativeFragmentTestStateCreateInfoNV *>(this); }
    VkPipelineRepresentativeFragmentTestStateCreateInfoNV const *ptr() const { return reinterpret_cast<VkPipelineRepresentativeFragmentTestStateCreateInfoNV const *>(this); }
};

struct safe_VkPhysicalDeviceImageViewImageFormatInfoEXT {
    VkStructureType sType;
    void* pNext;
    VkImageViewType imageViewType;
    safe_VkPhysicalDeviceImageViewImageFormatInfoEXT(const VkPhysicalDeviceImageViewImageFormatInfoEXT* in_struct);
    safe_VkPhysicalDeviceImageViewImageFormatInfoEXT(const safe_VkPhysicalDeviceImageViewImageFormatInfoEXT& src);
    safe_VkPhysicalDeviceImageViewImageFormatInfoEXT& operator=(const safe_VkPhysicalDeviceImageViewImageFormatInfoEXT& src);
    safe_VkPhysicalDeviceImageViewImageFormatInfoEXT();
    ~safe_VkPhysicalDeviceImageViewImageFormatInfoEXT();
    void initialize(const VkPhysicalDeviceImageViewImageFormatInfoEXT* in_struct);
    void initialize(const safe_VkPhysicalDeviceImageViewImageFormatInfoEXT* src);
    VkPhysicalDeviceImageViewImageFormatInfoEXT *ptr() { return reinterpret_cast<VkPhysicalDeviceImageViewImageFormatInfoEXT *>(this); }
    VkPhysicalDeviceImageViewImageFormatInfoEXT const *ptr() const { return reinterpret_cast<VkPhysicalDeviceImageViewImageFormatInfoEXT const *>(this); }
};

struct safe_VkFilterCubicImageViewImageFormatPropertiesEXT {
    VkStructureType sType;
    void* pNext;
    VkBool32 filterCubic;
    VkBool32 filterCubicMinmax ;
    safe_VkFilterCubicImageViewImageFormatPropertiesEXT(const VkFilterCubicImageViewImageFormatPropertiesEXT* in_struct);
    safe_VkFilterCubicImageViewImageFormatPropertiesEXT(const safe_VkFilterCubicImageViewImageFormatPropertiesEXT& src);
    safe_VkFilterCubicImageViewImageFormatPropertiesEXT& operator=(const safe_VkFilterCubicImageViewImageFormatPropertiesEXT& src);
    safe_VkFilterCubicImageViewImageFormatPropertiesEXT();
    ~safe_VkFilterCubicImageViewImageFormatPropertiesEXT();
    void initialize(const VkFilterCubicImageViewImageFormatPropertiesEXT* in_struct);
    void initialize(const safe_VkFilterCubicImageViewImageFormatPropertiesEXT* src);
    VkFilterCubicImageViewImageFormatPropertiesEXT *ptr() { return reinterpret_cast<VkFilterCubicImageViewImageFormatPropertiesEXT *>(this); }
    VkFilterCubicImageViewImageFormatPropertiesEXT const *ptr() const { return reinterpret_cast<VkFilterCubicImageViewImageFormatPropertiesEXT const *>(this); }
};

struct safe_VkDeviceQueueGlobalPriorityCreateInfoEXT {
    VkStructureType sType;
    const void* pNext;
    VkQueueGlobalPriorityEXT globalPriority;
    safe_VkDeviceQueueGlobalPriorityCreateInfoEXT(const VkDeviceQueueGlobalPriorityCreateInfoEXT* in_struct);
    safe_VkDeviceQueueGlobalPriorityCreateInfoEXT(const safe_VkDeviceQueueGlobalPriorityCreateInfoEXT& src);
    safe_VkDeviceQueueGlobalPriorityCreateInfoEXT& operator=(const safe_VkDeviceQueueGlobalPriorityCreateInfoEXT& src);
    safe_VkDeviceQueueGlobalPriorityCreateInfoEXT();
    ~safe_VkDeviceQueueGlobalPriorityCreateInfoEXT();
    void initialize(const VkDeviceQueueGlobalPriorityCreateInfoEXT* in_struct);
    void initialize(const safe_VkDeviceQueueGlobalPriorityCreateInfoEXT* src);
    VkDeviceQueueGlobalPriorityCreateInfoEXT *ptr() { return reinterpret_cast<VkDeviceQueueGlobalPriorityCreateInfoEXT *>(this); }
    VkDeviceQueueGlobalPriorityCreateInfoEXT const *ptr() const { return reinterpret_cast<VkDeviceQueueGlobalPriorityCreateInfoEXT const *>(this); }
};

struct safe_VkImportMemoryHostPointerInfoEXT {
    VkStructureType sType;
    const void* pNext;
    VkExternalMemoryHandleTypeFlagBits handleType;
    void* pHostPointer;
    safe_VkImportMemoryHostPointerInfoEXT(const VkImportMemoryHostPointerInfoEXT* in_struct);
    safe_VkImportMemoryHostPointerInfoEXT(const safe_VkImportMemoryHostPointerInfoEXT& src);
    safe_VkImportMemoryHostPointerInfoEXT& operator=(const safe_VkImportMemoryHostPointerInfoEXT& src);
    safe_VkImportMemoryHostPointerInfoEXT();
    ~safe_VkImportMemoryHostPointerInfoEXT();
    void initialize(const VkImportMemoryHostPointerInfoEXT* in_struct);
    void initialize(const safe_VkImportMemoryHostPointerInfoEXT* src);
    VkImportMemoryHostPointerInfoEXT *ptr() { return reinterpret_cast<VkImportMemoryHostPointerInfoEXT *>(this); }
    VkImportMemoryHostPointerInfoEXT const *ptr() const { return reinterpret_cast<VkImportMemoryHostPointerInfoEXT const *>(this); }
};

struct safe_VkMemoryHostPointerPropertiesEXT {
    VkStructureType sType;
    void* pNext;
    uint32_t memoryTypeBits;
    safe_VkMemoryHostPointerPropertiesEXT(const VkMemoryHostPointerPropertiesEXT* in_struct);
    safe_VkMemoryHostPointerPropertiesEXT(const safe_VkMemoryHostPointerPropertiesEXT& src);
    safe_VkMemoryHostPointerPropertiesEXT& operator=(const safe_VkMemoryHostPointerPropertiesEXT& src);
    safe_VkMemoryHostPointerPropertiesEXT();
    ~safe_VkMemoryHostPointerPropertiesEXT();
    void initialize(const VkMemoryHostPointerPropertiesEXT* in_struct);
    void initialize(const safe_VkMemoryHostPointerPropertiesEXT* src);
    VkMemoryHostPointerPropertiesEXT *ptr() { return reinterpret_cast<VkMemoryHostPointerPropertiesEXT *>(this); }
    VkMemoryHostPointerPropertiesEXT const *ptr() const { return reinterpret_cast<VkMemoryHostPointerPropertiesEXT const *>(this); }
};

struct safe_VkPhysicalDeviceExternalMemoryHostPropertiesEXT {
    VkStructureType sType;
    void* pNext;
    VkDeviceSize minImportedHostPointerAlignment;
    safe_VkPhysicalDeviceExternalMemoryHostPropertiesEXT(const VkPhysicalDeviceExternalMemoryHostPropertiesEXT* in_struct);
    safe_VkPhysicalDeviceExternalMemoryHostPropertiesEXT(const safe_VkPhysicalDeviceExternalMemoryHostPropertiesEXT& src);
    safe_VkPhysicalDeviceExternalMemoryHostPropertiesEXT& operator=(const safe_VkPhysicalDeviceExternalMemoryHostPropertiesEXT& src);
    safe_VkPhysicalDeviceExternalMemoryHostPropertiesEXT();
    ~safe_VkPhysicalDeviceExternalMemoryHostPropertiesEXT();
    void initialize(const VkPhysicalDeviceExternalMemoryHostPropertiesEXT* in_struct);
    void initialize(const safe_VkPhysicalDeviceExternalMemoryHostPropertiesEXT* src);
    VkPhysicalDeviceExternalMemoryHostPropertiesEXT *ptr() { return reinterpret_cast<VkPhysicalDeviceExternalMemoryHostPropertiesEXT *>(this); }
    VkPhysicalDeviceExternalMemoryHostPropertiesEXT const *ptr() const { return reinterpret_cast<VkPhysicalDeviceExternalMemoryHostPropertiesEXT const *>(this); }
};

struct safe_VkPipelineCompilerControlCreateInfoAMD {
    VkStructureType sType;
    const void* pNext;
    VkPipelineCompilerControlFlagsAMD compilerControlFlags;
    safe_VkPipelineCompilerControlCreateInfoAMD(const VkPipelineCompilerControlCreateInfoAMD* in_struct);
    safe_VkPipelineCompilerControlCreateInfoAMD(const safe_VkPipelineCompilerControlCreateInfoAMD& src);
    safe_VkPipelineCompilerControlCreateInfoAMD& operator=(const safe_VkPipelineCompilerControlCreateInfoAMD& src);
    safe_VkPipelineCompilerControlCreateInfoAMD();
    ~safe_VkPipelineCompilerControlCreateInfoAMD();
    void initialize(const VkPipelineCompilerControlCreateInfoAMD* in_struct);
    void initialize(const safe_VkPipelineCompilerControlCreateInfoAMD* src);
    VkPipelineCompilerControlCreateInfoAMD *ptr() { return reinterpret_cast<VkPipelineCompilerControlCreateInfoAMD *>(this); }
    VkPipelineCompilerControlCreateInfoAMD const *ptr() const { return reinterpret_cast<VkPipelineCompilerControlCreateInfoAMD const *>(this); }
};

struct safe_VkCalibratedTimestampInfoEXT {
    VkStructureType sType;
    const void* pNext;
    VkTimeDomainEXT timeDomain;
    safe_VkCalibratedTimestampInfoEXT(const VkCalibratedTimestampInfoEXT* in_struct);
    safe_VkCalibratedTimestampInfoEXT(const safe_VkCalibratedTimestampInfoEXT& src);
    safe_VkCalibratedTimestampInfoEXT& operator=(const safe_VkCalibratedTimestampInfoEXT& src);
    safe_VkCalibratedTimestampInfoEXT();
    ~safe_VkCalibratedTimestampInfoEXT();
    void initialize(const VkCalibratedTimestampInfoEXT* in_struct);
    void initialize(const safe_VkCalibratedTimestampInfoEXT* src);
    VkCalibratedTimestampInfoEXT *ptr() { return reinterpret_cast<VkCalibratedTimestampInfoEXT *>(this); }
    VkCalibratedTimestampInfoEXT const *ptr() const { return reinterpret_cast<VkCalibratedTimestampInfoEXT const *>(this); }
};

struct safe_VkPhysicalDeviceShaderCorePropertiesAMD {
    VkStructureType sType;
    void* pNext;
    uint32_t shaderEngineCount;
    uint32_t shaderArraysPerEngineCount;
    uint32_t computeUnitsPerShaderArray;
    uint32_t simdPerComputeUnit;
    uint32_t wavefrontsPerSimd;
    uint32_t wavefrontSize;
    uint32_t sgprsPerSimd;
    uint32_t minSgprAllocation;
    uint32_t maxSgprAllocation;
    uint32_t sgprAllocationGranularity;
    uint32_t vgprsPerSimd;
    uint32_t minVgprAllocation;
    uint32_t maxVgprAllocation;
    uint32_t vgprAllocationGranularity;
    safe_VkPhysicalDeviceShaderCorePropertiesAMD(const VkPhysicalDeviceShaderCorePropertiesAMD* in_struct);
    safe_VkPhysicalDeviceShaderCorePropertiesAMD(const safe_VkPhysicalDeviceShaderCorePropertiesAMD& src);
    safe_VkPhysicalDeviceShaderCorePropertiesAMD& operator=(const safe_VkPhysicalDeviceShaderCorePropertiesAMD& src);
    safe_VkPhysicalDeviceShaderCorePropertiesAMD();
    ~safe_VkPhysicalDeviceShaderCorePropertiesAMD();
    void initialize(const VkPhysicalDeviceShaderCorePropertiesAMD* in_struct);
    void initialize(const safe_VkPhysicalDeviceShaderCorePropertiesAMD* src);
    VkPhysicalDeviceShaderCorePropertiesAMD *ptr() { return reinterpret_cast<VkPhysicalDeviceShaderCorePropertiesAMD *>(this); }
    VkPhysicalDeviceShaderCorePropertiesAMD const *ptr() const { return reinterpret_cast<VkPhysicalDeviceShaderCorePropertiesAMD const *>(this); }
};

struct safe_VkDeviceMemoryOverallocationCreateInfoAMD {
    VkStructureType sType;
    const void* pNext;
    VkMemoryOverallocationBehaviorAMD overallocationBehavior;
    safe_VkDeviceMemoryOverallocationCreateInfoAMD(const VkDeviceMemoryOverallocationCreateInfoAMD* in_struct);
    safe_VkDeviceMemoryOverallocationCreateInfoAMD(const safe_VkDeviceMemoryOverallocationCreateInfoAMD& src);
    safe_VkDeviceMemoryOverallocationCreateInfoAMD& operator=(const safe_VkDeviceMemoryOverallocationCreateInfoAMD& src);
    safe_VkDeviceMemoryOverallocationCreateInfoAMD();
    ~safe_VkDeviceMemoryOverallocationCreateInfoAMD();
    void initialize(const VkDeviceMemoryOverallocationCreateInfoAMD* in_struct);
    void initialize(const safe_VkDeviceMemoryOverallocationCreateInfoAMD* src);
    VkDeviceMemoryOverallocationCreateInfoAMD *ptr() { return reinterpret_cast<VkDeviceMemoryOverallocationCreateInfoAMD *>(this); }
    VkDeviceMemoryOverallocationCreateInfoAMD const *ptr() const { return reinterpret_cast<VkDeviceMemoryOverallocationCreateInfoAMD const *>(this); }
};

struct safe_VkPhysicalDeviceVertexAttributeDivisorPropertiesEXT {
    VkStructureType sType;
    void* pNext;
    uint32_t maxVertexAttribDivisor;
    safe_VkPhysicalDeviceVertexAttributeDivisorPropertiesEXT(const VkPhysicalDeviceVertexAttributeDivisorPropertiesEXT* in_struct);
    safe_VkPhysicalDeviceVertexAttributeDivisorPropertiesEXT(const safe_VkPhysicalDeviceVertexAttributeDivisorPropertiesEXT& src);
    safe_VkPhysicalDeviceVertexAttributeDivisorPropertiesEXT& operator=(const safe_VkPhysicalDeviceVertexAttributeDivisorPropertiesEXT& src);
    safe_VkPhysicalDeviceVertexAttributeDivisorPropertiesEXT();
    ~safe_VkPhysicalDeviceVertexAttributeDivisorPropertiesEXT();
    void initialize(const VkPhysicalDeviceVertexAttributeDivisorPropertiesEXT* in_struct);
    void initialize(const safe_VkPhysicalDeviceVertexAttributeDivisorPropertiesEXT* src);
    VkPhysicalDeviceVertexAttributeDivisorPropertiesEXT *ptr() { return reinterpret_cast<VkPhysicalDeviceVertexAttributeDivisorPropertiesEXT *>(this); }
    VkPhysicalDeviceVertexAttributeDivisorPropertiesEXT const *ptr() const { return reinterpret_cast<VkPhysicalDeviceVertexAttributeDivisorPropertiesEXT const *>(this); }
};

struct safe_VkPipelineVertexInputDivisorStateCreateInfoEXT {
    VkStructureType sType;
    const void* pNext;
    uint32_t vertexBindingDivisorCount;
    const VkVertexInputBindingDivisorDescriptionEXT* pVertexBindingDivisors;
    safe_VkPipelineVertexInputDivisorStateCreateInfoEXT(const VkPipelineVertexInputDivisorStateCreateInfoEXT* in_struct);
    safe_VkPipelineVertexInputDivisorStateCreateInfoEXT(const safe_VkPipelineVertexInputDivisorStateCreateInfoEXT& src);
    safe_VkPipelineVertexInputDivisorStateCreateInfoEXT& operator=(const safe_VkPipelineVertexInputDivisorStateCreateInfoEXT& src);
    safe_VkPipelineVertexInputDivisorStateCreateInfoEXT();
    ~safe_VkPipelineVertexInputDivisorStateCreateInfoEXT();
    void initialize(const VkPipelineVertexInputDivisorStateCreateInfoEXT* in_struct);
    void initialize(const safe_VkPipelineVertexInputDivisorStateCreateInfoEXT* src);
    VkPipelineVertexInputDivisorStateCreateInfoEXT *ptr() { return reinterpret_cast<VkPipelineVertexInputDivisorStateCreateInfoEXT *>(this); }
    VkPipelineVertexInputDivisorStateCreateInfoEXT const *ptr() const { return reinterpret_cast<VkPipelineVertexInputDivisorStateCreateInfoEXT const *>(this); }
};

struct safe_VkPhysicalDeviceVertexAttributeDivisorFeaturesEXT {
    VkStructureType sType;
    void* pNext;
    VkBool32 vertexAttributeInstanceRateDivisor;
    VkBool32 vertexAttributeInstanceRateZeroDivisor;
    safe_VkPhysicalDeviceVertexAttributeDivisorFeaturesEXT(const VkPhysicalDeviceVertexAttributeDivisorFeaturesEXT* in_struct);
    safe_VkPhysicalDeviceVertexAttributeDivisorFeaturesEXT(const safe_VkPhysicalDeviceVertexAttributeDivisorFeaturesEXT& src);
    safe_VkPhysicalDeviceVertexAttributeDivisorFeaturesEXT& operator=(const safe_VkPhysicalDeviceVertexAttributeDivisorFeaturesEXT& src);
    safe_VkPhysicalDeviceVertexAttributeDivisorFeaturesEXT();
    ~safe_VkPhysicalDeviceVertexAttributeDivisorFeaturesEXT();
    void initialize(const VkPhysicalDeviceVertexAttributeDivisorFeaturesEXT* in_struct);
    void initialize(const safe_VkPhysicalDeviceVertexAttributeDivisorFeaturesEXT* src);
    VkPhysicalDeviceVertexAttributeDivisorFeaturesEXT *ptr() { return reinterpret_cast<VkPhysicalDeviceVertexAttributeDivisorFeaturesEXT *>(this); }
    VkPhysicalDeviceVertexAttributeDivisorFeaturesEXT const *ptr() const { return reinterpret_cast<VkPhysicalDeviceVertexAttributeDivisorFeaturesEXT const *>(this); }
};

#ifdef VK_USE_PLATFORM_GGP
struct safe_VkPresentFrameTokenGGP {
    VkStructureType sType;
    const void* pNext;
    GgpFrameToken frameToken;
    safe_VkPresentFrameTokenGGP(const VkPresentFrameTokenGGP* in_struct);
    safe_VkPresentFrameTokenGGP(const safe_VkPresentFrameTokenGGP& src);
    safe_VkPresentFrameTokenGGP& operator=(const safe_VkPresentFrameTokenGGP& src);
    safe_VkPresentFrameTokenGGP();
    ~safe_VkPresentFrameTokenGGP();
    void initialize(const VkPresentFrameTokenGGP* in_struct);
    void initialize(const safe_VkPresentFrameTokenGGP* src);
    VkPresentFrameTokenGGP *ptr() { return reinterpret_cast<VkPresentFrameTokenGGP *>(this); }
    VkPresentFrameTokenGGP const *ptr() const { return reinterpret_cast<VkPresentFrameTokenGGP const *>(this); }
};
#endif // VK_USE_PLATFORM_GGP

struct safe_VkPipelineCreationFeedbackCreateInfoEXT {
    VkStructureType sType;
    const void* pNext;
    VkPipelineCreationFeedbackEXT* pPipelineCreationFeedback;
    uint32_t pipelineStageCreationFeedbackCount;
    VkPipelineCreationFeedbackEXT* pPipelineStageCreationFeedbacks;
    safe_VkPipelineCreationFeedbackCreateInfoEXT(const VkPipelineCreationFeedbackCreateInfoEXT* in_struct);
    safe_VkPipelineCreationFeedbackCreateInfoEXT(const safe_VkPipelineCreationFeedbackCreateInfoEXT& src);
    safe_VkPipelineCreationFeedbackCreateInfoEXT& operator=(const safe_VkPipelineCreationFeedbackCreateInfoEXT& src);
    safe_VkPipelineCreationFeedbackCreateInfoEXT();
    ~safe_VkPipelineCreationFeedbackCreateInfoEXT();
    void initialize(const VkPipelineCreationFeedbackCreateInfoEXT* in_struct);
    void initialize(const safe_VkPipelineCreationFeedbackCreateInfoEXT* src);
    VkPipelineCreationFeedbackCreateInfoEXT *ptr() { return reinterpret_cast<VkPipelineCreationFeedbackCreateInfoEXT *>(this); }
    VkPipelineCreationFeedbackCreateInfoEXT const *ptr() const { return reinterpret_cast<VkPipelineCreationFeedbackCreateInfoEXT const *>(this); }
};

struct safe_VkPhysicalDeviceComputeShaderDerivativesFeaturesNV {
    VkStructureType sType;
    void* pNext;
    VkBool32 computeDerivativeGroupQuads;
    VkBool32 computeDerivativeGroupLinear;
    safe_VkPhysicalDeviceComputeShaderDerivativesFeaturesNV(const VkPhysicalDeviceComputeShaderDerivativesFeaturesNV* in_struct);
    safe_VkPhysicalDeviceComputeShaderDerivativesFeaturesNV(const safe_VkPhysicalDeviceComputeShaderDerivativesFeaturesNV& src);
    safe_VkPhysicalDeviceComputeShaderDerivativesFeaturesNV& operator=(const safe_VkPhysicalDeviceComputeShaderDerivativesFeaturesNV& src);
    safe_VkPhysicalDeviceComputeShaderDerivativesFeaturesNV();
    ~safe_VkPhysicalDeviceComputeShaderDerivativesFeaturesNV();
    void initialize(const VkPhysicalDeviceComputeShaderDerivativesFeaturesNV* in_struct);
    void initialize(const safe_VkPhysicalDeviceComputeShaderDerivativesFeaturesNV* src);
    VkPhysicalDeviceComputeShaderDerivativesFeaturesNV *ptr() { return reinterpret_cast<VkPhysicalDeviceComputeShaderDerivativesFeaturesNV *>(this); }
    VkPhysicalDeviceComputeShaderDerivativesFeaturesNV const *ptr() const { return reinterpret_cast<VkPhysicalDeviceComputeShaderDerivativesFeaturesNV const *>(this); }
};

struct safe_VkPhysicalDeviceMeshShaderFeaturesNV {
    VkStructureType sType;
    void* pNext;
    VkBool32 taskShader;
    VkBool32 meshShader;
    safe_VkPhysicalDeviceMeshShaderFeaturesNV(const VkPhysicalDeviceMeshShaderFeaturesNV* in_struct);
    safe_VkPhysicalDeviceMeshShaderFeaturesNV(const safe_VkPhysicalDeviceMeshShaderFeaturesNV& src);
    safe_VkPhysicalDeviceMeshShaderFeaturesNV& operator=(const safe_VkPhysicalDeviceMeshShaderFeaturesNV& src);
    safe_VkPhysicalDeviceMeshShaderFeaturesNV();
    ~safe_VkPhysicalDeviceMeshShaderFeaturesNV();
    void initialize(const VkPhysicalDeviceMeshShaderFeaturesNV* in_struct);
    void initialize(const safe_VkPhysicalDeviceMeshShaderFeaturesNV* src);
    VkPhysicalDeviceMeshShaderFeaturesNV *ptr() { return reinterpret_cast<VkPhysicalDeviceMeshShaderFeaturesNV *>(this); }
    VkPhysicalDeviceMeshShaderFeaturesNV const *ptr() const { return reinterpret_cast<VkPhysicalDeviceMeshShaderFeaturesNV const *>(this); }
};

struct safe_VkPhysicalDeviceMeshShaderPropertiesNV {
    VkStructureType sType;
    void* pNext;
    uint32_t maxDrawMeshTasksCount;
    uint32_t maxTaskWorkGroupInvocations;
    uint32_t maxTaskWorkGroupSize[3];
    uint32_t maxTaskTotalMemorySize;
    uint32_t maxTaskOutputCount;
    uint32_t maxMeshWorkGroupInvocations;
    uint32_t maxMeshWorkGroupSize[3];
    uint32_t maxMeshTotalMemorySize;
    uint32_t maxMeshOutputVertices;
    uint32_t maxMeshOutputPrimitives;
    uint32_t maxMeshMultiviewViewCount;
    uint32_t meshOutputPerVertexGranularity;
    uint32_t meshOutputPerPrimitiveGranularity;
    safe_VkPhysicalDeviceMeshShaderPropertiesNV(const VkPhysicalDeviceMeshShaderPropertiesNV* in_struct);
    safe_VkPhysicalDeviceMeshShaderPropertiesNV(const safe_VkPhysicalDeviceMeshShaderPropertiesNV& src);
    safe_VkPhysicalDeviceMeshShaderPropertiesNV& operator=(const safe_VkPhysicalDeviceMeshShaderPropertiesNV& src);
    safe_VkPhysicalDeviceMeshShaderPropertiesNV();
    ~safe_VkPhysicalDeviceMeshShaderPropertiesNV();
    void initialize(const VkPhysicalDeviceMeshShaderPropertiesNV* in_struct);
    void initialize(const safe_VkPhysicalDeviceMeshShaderPropertiesNV* src);
    VkPhysicalDeviceMeshShaderPropertiesNV *ptr() { return reinterpret_cast<VkPhysicalDeviceMeshShaderPropertiesNV *>(this); }
    VkPhysicalDeviceMeshShaderPropertiesNV const *ptr() const { return reinterpret_cast<VkPhysicalDeviceMeshShaderPropertiesNV const *>(this); }
};

struct safe_VkPhysicalDeviceFragmentShaderBarycentricFeaturesNV {
    VkStructureType sType;
    void* pNext;
    VkBool32 fragmentShaderBarycentric;
    safe_VkPhysicalDeviceFragmentShaderBarycentricFeaturesNV(const VkPhysicalDeviceFragmentShaderBarycentricFeaturesNV* in_struct);
    safe_VkPhysicalDeviceFragmentShaderBarycentricFeaturesNV(const safe_VkPhysicalDeviceFragmentShaderBarycentricFeaturesNV& src);
    safe_VkPhysicalDeviceFragmentShaderBarycentricFeaturesNV& operator=(const safe_VkPhysicalDeviceFragmentShaderBarycentricFeaturesNV& src);
    safe_VkPhysicalDeviceFragmentShaderBarycentricFeaturesNV();
    ~safe_VkPhysicalDeviceFragmentShaderBarycentricFeaturesNV();
    void initialize(const VkPhysicalDeviceFragmentShaderBarycentricFeaturesNV* in_struct);
    void initialize(const safe_VkPhysicalDeviceFragmentShaderBarycentricFeaturesNV* src);
    VkPhysicalDeviceFragmentShaderBarycentricFeaturesNV *ptr() { return reinterpret_cast<VkPhysicalDeviceFragmentShaderBarycentricFeaturesNV *>(this); }
    VkPhysicalDeviceFragmentShaderBarycentricFeaturesNV const *ptr() const { return reinterpret_cast<VkPhysicalDeviceFragmentShaderBarycentricFeaturesNV const *>(this); }
};

struct safe_VkPhysicalDeviceShaderImageFootprintFeaturesNV {
    VkStructureType sType;
    void* pNext;
    VkBool32 imageFootprint;
    safe_VkPhysicalDeviceShaderImageFootprintFeaturesNV(const VkPhysicalDeviceShaderImageFootprintFeaturesNV* in_struct);
    safe_VkPhysicalDeviceShaderImageFootprintFeaturesNV(const safe_VkPhysicalDeviceShaderImageFootprintFeaturesNV& src);
    safe_VkPhysicalDeviceShaderImageFootprintFeaturesNV& operator=(const safe_VkPhysicalDeviceShaderImageFootprintFeaturesNV& src);
    safe_VkPhysicalDeviceShaderImageFootprintFeaturesNV();
    ~safe_VkPhysicalDeviceShaderImageFootprintFeaturesNV();
    void initialize(const VkPhysicalDeviceShaderImageFootprintFeaturesNV* in_struct);
    void initialize(const safe_VkPhysicalDeviceShaderImageFootprintFeaturesNV* src);
    VkPhysicalDeviceShaderImageFootprintFeaturesNV *ptr() { return reinterpret_cast<VkPhysicalDeviceShaderImageFootprintFeaturesNV *>(this); }
    VkPhysicalDeviceShaderImageFootprintFeaturesNV const *ptr() const { return reinterpret_cast<VkPhysicalDeviceShaderImageFootprintFeaturesNV const *>(this); }
};

struct safe_VkPipelineViewportExclusiveScissorStateCreateInfoNV {
    VkStructureType sType;
    const void* pNext;
    uint32_t exclusiveScissorCount;
    const VkRect2D* pExclusiveScissors;
    safe_VkPipelineViewportExclusiveScissorStateCreateInfoNV(const VkPipelineViewportExclusiveScissorStateCreateInfoNV* in_struct);
    safe_VkPipelineViewportExclusiveScissorStateCreateInfoNV(const safe_VkPipelineViewportExclusiveScissorStateCreateInfoNV& src);
    safe_VkPipelineViewportExclusiveScissorStateCreateInfoNV& operator=(const safe_VkPipelineViewportExclusiveScissorStateCreateInfoNV& src);
    safe_VkPipelineViewportExclusiveScissorStateCreateInfoNV();
    ~safe_VkPipelineViewportExclusiveScissorStateCreateInfoNV();
    void initialize(const VkPipelineViewportExclusiveScissorStateCreateInfoNV* in_struct);
    void initialize(const safe_VkPipelineViewportExclusiveScissorStateCreateInfoNV* src);
    VkPipelineViewportExclusiveScissorStateCreateInfoNV *ptr() { return reinterpret_cast<VkPipelineViewportExclusiveScissorStateCreateInfoNV *>(this); }
    VkPipelineViewportExclusiveScissorStateCreateInfoNV const *ptr() const { return reinterpret_cast<VkPipelineViewportExclusiveScissorStateCreateInfoNV const *>(this); }
};

struct safe_VkPhysicalDeviceExclusiveScissorFeaturesNV {
    VkStructureType sType;
    void* pNext;
    VkBool32 exclusiveScissor;
    safe_VkPhysicalDeviceExclusiveScissorFeaturesNV(const VkPhysicalDeviceExclusiveScissorFeaturesNV* in_struct);
    safe_VkPhysicalDeviceExclusiveScissorFeaturesNV(const safe_VkPhysicalDeviceExclusiveScissorFeaturesNV& src);
    safe_VkPhysicalDeviceExclusiveScissorFeaturesNV& operator=(const safe_VkPhysicalDeviceExclusiveScissorFeaturesNV& src);
    safe_VkPhysicalDeviceExclusiveScissorFeaturesNV();
    ~safe_VkPhysicalDeviceExclusiveScissorFeaturesNV();
    void initialize(const VkPhysicalDeviceExclusiveScissorFeaturesNV* in_struct);
    void initialize(const safe_VkPhysicalDeviceExclusiveScissorFeaturesNV* src);
    VkPhysicalDeviceExclusiveScissorFeaturesNV *ptr() { return reinterpret_cast<VkPhysicalDeviceExclusiveScissorFeaturesNV *>(this); }
    VkPhysicalDeviceExclusiveScissorFeaturesNV const *ptr() const { return reinterpret_cast<VkPhysicalDeviceExclusiveScissorFeaturesNV const *>(this); }
};

struct safe_VkQueueFamilyCheckpointPropertiesNV {
    VkStructureType sType;
    void* pNext;
    VkPipelineStageFlags checkpointExecutionStageMask;
    safe_VkQueueFamilyCheckpointPropertiesNV(const VkQueueFamilyCheckpointPropertiesNV* in_struct);
    safe_VkQueueFamilyCheckpointPropertiesNV(const safe_VkQueueFamilyCheckpointPropertiesNV& src);
    safe_VkQueueFamilyCheckpointPropertiesNV& operator=(const safe_VkQueueFamilyCheckpointPropertiesNV& src);
    safe_VkQueueFamilyCheckpointPropertiesNV();
    ~safe_VkQueueFamilyCheckpointPropertiesNV();
    void initialize(const VkQueueFamilyCheckpointPropertiesNV* in_struct);
    void initialize(const safe_VkQueueFamilyCheckpointPropertiesNV* src);
    VkQueueFamilyCheckpointPropertiesNV *ptr() { return reinterpret_cast<VkQueueFamilyCheckpointPropertiesNV *>(this); }
    VkQueueFamilyCheckpointPropertiesNV const *ptr() const { return reinterpret_cast<VkQueueFamilyCheckpointPropertiesNV const *>(this); }
};

struct safe_VkCheckpointDataNV {
    VkStructureType sType;
    void* pNext;
    VkPipelineStageFlagBits stage;
    void* pCheckpointMarker;
    safe_VkCheckpointDataNV(const VkCheckpointDataNV* in_struct);
    safe_VkCheckpointDataNV(const safe_VkCheckpointDataNV& src);
    safe_VkCheckpointDataNV& operator=(const safe_VkCheckpointDataNV& src);
    safe_VkCheckpointDataNV();
    ~safe_VkCheckpointDataNV();
    void initialize(const VkCheckpointDataNV* in_struct);
    void initialize(const safe_VkCheckpointDataNV* src);
    VkCheckpointDataNV *ptr() { return reinterpret_cast<VkCheckpointDataNV *>(this); }
    VkCheckpointDataNV const *ptr() const { return reinterpret_cast<VkCheckpointDataNV const *>(this); }
};

struct safe_VkPhysicalDeviceShaderIntegerFunctions2FeaturesINTEL {
    VkStructureType sType;
    void* pNext;
    VkBool32 shaderIntegerFunctions2;
    safe_VkPhysicalDeviceShaderIntegerFunctions2FeaturesINTEL(const VkPhysicalDeviceShaderIntegerFunctions2FeaturesINTEL* in_struct);
    safe_VkPhysicalDeviceShaderIntegerFunctions2FeaturesINTEL(const safe_VkPhysicalDeviceShaderIntegerFunctions2FeaturesINTEL& src);
    safe_VkPhysicalDeviceShaderIntegerFunctions2FeaturesINTEL& operator=(const safe_VkPhysicalDeviceShaderIntegerFunctions2FeaturesINTEL& src);
    safe_VkPhysicalDeviceShaderIntegerFunctions2FeaturesINTEL();
    ~safe_VkPhysicalDeviceShaderIntegerFunctions2FeaturesINTEL();
    void initialize(const VkPhysicalDeviceShaderIntegerFunctions2FeaturesINTEL* in_struct);
    void initialize(const safe_VkPhysicalDeviceShaderIntegerFunctions2FeaturesINTEL* src);
    VkPhysicalDeviceShaderIntegerFunctions2FeaturesINTEL *ptr() { return reinterpret_cast<VkPhysicalDeviceShaderIntegerFunctions2FeaturesINTEL *>(this); }
    VkPhysicalDeviceShaderIntegerFunctions2FeaturesINTEL const *ptr() const { return reinterpret_cast<VkPhysicalDeviceShaderIntegerFunctions2FeaturesINTEL const *>(this); }
};

struct safe_VkPerformanceValueDataINTEL {
    uint32_t value32;
    uint64_t value64;
    float valueFloat;
    VkBool32 valueBool;
    const char* valueString;
    safe_VkPerformanceValueDataINTEL(const VkPerformanceValueDataINTEL* in_struct);
    safe_VkPerformanceValueDataINTEL(const safe_VkPerformanceValueDataINTEL& src);
    safe_VkPerformanceValueDataINTEL& operator=(const safe_VkPerformanceValueDataINTEL& src);
    safe_VkPerformanceValueDataINTEL();
    ~safe_VkPerformanceValueDataINTEL();
    void initialize(const VkPerformanceValueDataINTEL* in_struct);
    void initialize(const safe_VkPerformanceValueDataINTEL* src);
    VkPerformanceValueDataINTEL *ptr() { return reinterpret_cast<VkPerformanceValueDataINTEL *>(this); }
    VkPerformanceValueDataINTEL const *ptr() const { return reinterpret_cast<VkPerformanceValueDataINTEL const *>(this); }
};

struct safe_VkInitializePerformanceApiInfoINTEL {
    VkStructureType sType;
    const void* pNext;
    void* pUserData;
    safe_VkInitializePerformanceApiInfoINTEL(const VkInitializePerformanceApiInfoINTEL* in_struct);
    safe_VkInitializePerformanceApiInfoINTEL(const safe_VkInitializePerformanceApiInfoINTEL& src);
    safe_VkInitializePerformanceApiInfoINTEL& operator=(const safe_VkInitializePerformanceApiInfoINTEL& src);
    safe_VkInitializePerformanceApiInfoINTEL();
    ~safe_VkInitializePerformanceApiInfoINTEL();
    void initialize(const VkInitializePerformanceApiInfoINTEL* in_struct);
    void initialize(const safe_VkInitializePerformanceApiInfoINTEL* src);
    VkInitializePerformanceApiInfoINTEL *ptr() { return reinterpret_cast<VkInitializePerformanceApiInfoINTEL *>(this); }
    VkInitializePerformanceApiInfoINTEL const *ptr() const { return reinterpret_cast<VkInitializePerformanceApiInfoINTEL const *>(this); }
};

struct safe_VkQueryPoolCreateInfoINTEL {
    VkStructureType sType;
    const void* pNext;
    VkQueryPoolSamplingModeINTEL performanceCountersSampling;
    safe_VkQueryPoolCreateInfoINTEL(const VkQueryPoolCreateInfoINTEL* in_struct);
    safe_VkQueryPoolCreateInfoINTEL(const safe_VkQueryPoolCreateInfoINTEL& src);
    safe_VkQueryPoolCreateInfoINTEL& operator=(const safe_VkQueryPoolCreateInfoINTEL& src);
    safe_VkQueryPoolCreateInfoINTEL();
    ~safe_VkQueryPoolCreateInfoINTEL();
    void initialize(const VkQueryPoolCreateInfoINTEL* in_struct);
    void initialize(const safe_VkQueryPoolCreateInfoINTEL* src);
    VkQueryPoolCreateInfoINTEL *ptr() { return reinterpret_cast<VkQueryPoolCreateInfoINTEL *>(this); }
    VkQueryPoolCreateInfoINTEL const *ptr() const { return reinterpret_cast<VkQueryPoolCreateInfoINTEL const *>(this); }
};

struct safe_VkPerformanceMarkerInfoINTEL {
    VkStructureType sType;
    const void* pNext;
    uint64_t marker;
    safe_VkPerformanceMarkerInfoINTEL(const VkPerformanceMarkerInfoINTEL* in_struct);
    safe_VkPerformanceMarkerInfoINTEL(const safe_VkPerformanceMarkerInfoINTEL& src);
    safe_VkPerformanceMarkerInfoINTEL& operator=(const safe_VkPerformanceMarkerInfoINTEL& src);
    safe_VkPerformanceMarkerInfoINTEL();
    ~safe_VkPerformanceMarkerInfoINTEL();
    void initialize(const VkPerformanceMarkerInfoINTEL* in_struct);
    void initialize(const safe_VkPerformanceMarkerInfoINTEL* src);
    VkPerformanceMarkerInfoINTEL *ptr() { return reinterpret_cast<VkPerformanceMarkerInfoINTEL *>(this); }
    VkPerformanceMarkerInfoINTEL const *ptr() const { return reinterpret_cast<VkPerformanceMarkerInfoINTEL const *>(this); }
};

struct safe_VkPerformanceStreamMarkerInfoINTEL {
    VkStructureType sType;
    const void* pNext;
    uint32_t marker;
    safe_VkPerformanceStreamMarkerInfoINTEL(const VkPerformanceStreamMarkerInfoINTEL* in_struct);
    safe_VkPerformanceStreamMarkerInfoINTEL(const safe_VkPerformanceStreamMarkerInfoINTEL& src);
    safe_VkPerformanceStreamMarkerInfoINTEL& operator=(const safe_VkPerformanceStreamMarkerInfoINTEL& src);
    safe_VkPerformanceStreamMarkerInfoINTEL();
    ~safe_VkPerformanceStreamMarkerInfoINTEL();
    void initialize(const VkPerformanceStreamMarkerInfoINTEL* in_struct);
    void initialize(const safe_VkPerformanceStreamMarkerInfoINTEL* src);
    VkPerformanceStreamMarkerInfoINTEL *ptr() { return reinterpret_cast<VkPerformanceStreamMarkerInfoINTEL *>(this); }
    VkPerformanceStreamMarkerInfoINTEL const *ptr() const { return reinterpret_cast<VkPerformanceStreamMarkerInfoINTEL const *>(this); }
};

struct safe_VkPerformanceOverrideInfoINTEL {
    VkStructureType sType;
    const void* pNext;
    VkPerformanceOverrideTypeINTEL type;
    VkBool32 enable;
    uint64_t parameter;
    safe_VkPerformanceOverrideInfoINTEL(const VkPerformanceOverrideInfoINTEL* in_struct);
    safe_VkPerformanceOverrideInfoINTEL(const safe_VkPerformanceOverrideInfoINTEL& src);
    safe_VkPerformanceOverrideInfoINTEL& operator=(const safe_VkPerformanceOverrideInfoINTEL& src);
    safe_VkPerformanceOverrideInfoINTEL();
    ~safe_VkPerformanceOverrideInfoINTEL();
    void initialize(const VkPerformanceOverrideInfoINTEL* in_struct);
    void initialize(const safe_VkPerformanceOverrideInfoINTEL* src);
    VkPerformanceOverrideInfoINTEL *ptr() { return reinterpret_cast<VkPerformanceOverrideInfoINTEL *>(this); }
    VkPerformanceOverrideInfoINTEL const *ptr() const { return reinterpret_cast<VkPerformanceOverrideInfoINTEL const *>(this); }
};

struct safe_VkPerformanceConfigurationAcquireInfoINTEL {
    VkStructureType sType;
    const void* pNext;
    VkPerformanceConfigurationTypeINTEL type;
    safe_VkPerformanceConfigurationAcquireInfoINTEL(const VkPerformanceConfigurationAcquireInfoINTEL* in_struct);
    safe_VkPerformanceConfigurationAcquireInfoINTEL(const safe_VkPerformanceConfigurationAcquireInfoINTEL& src);
    safe_VkPerformanceConfigurationAcquireInfoINTEL& operator=(const safe_VkPerformanceConfigurationAcquireInfoINTEL& src);
    safe_VkPerformanceConfigurationAcquireInfoINTEL();
    ~safe_VkPerformanceConfigurationAcquireInfoINTEL();
    void initialize(const VkPerformanceConfigurationAcquireInfoINTEL* in_struct);
    void initialize(const safe_VkPerformanceConfigurationAcquireInfoINTEL* src);
    VkPerformanceConfigurationAcquireInfoINTEL *ptr() { return reinterpret_cast<VkPerformanceConfigurationAcquireInfoINTEL *>(this); }
    VkPerformanceConfigurationAcquireInfoINTEL const *ptr() const { return reinterpret_cast<VkPerformanceConfigurationAcquireInfoINTEL const *>(this); }
};

struct safe_VkPhysicalDevicePCIBusInfoPropertiesEXT {
    VkStructureType sType;
    void* pNext;
    uint32_t pciDomain;
    uint32_t pciBus;
    uint32_t pciDevice;
    uint32_t pciFunction;
    safe_VkPhysicalDevicePCIBusInfoPropertiesEXT(const VkPhysicalDevicePCIBusInfoPropertiesEXT* in_struct);
    safe_VkPhysicalDevicePCIBusInfoPropertiesEXT(const safe_VkPhysicalDevicePCIBusInfoPropertiesEXT& src);
    safe_VkPhysicalDevicePCIBusInfoPropertiesEXT& operator=(const safe_VkPhysicalDevicePCIBusInfoPropertiesEXT& src);
    safe_VkPhysicalDevicePCIBusInfoPropertiesEXT();
    ~safe_VkPhysicalDevicePCIBusInfoPropertiesEXT();
    void initialize(const VkPhysicalDevicePCIBusInfoPropertiesEXT* in_struct);
    void initialize(const safe_VkPhysicalDevicePCIBusInfoPropertiesEXT* src);
    VkPhysicalDevicePCIBusInfoPropertiesEXT *ptr() { return reinterpret_cast<VkPhysicalDevicePCIBusInfoPropertiesEXT *>(this); }
    VkPhysicalDevicePCIBusInfoPropertiesEXT const *ptr() const { return reinterpret_cast<VkPhysicalDevicePCIBusInfoPropertiesEXT const *>(this); }
};

struct safe_VkDisplayNativeHdrSurfaceCapabilitiesAMD {
    VkStructureType sType;
    void* pNext;
    VkBool32 localDimmingSupport;
    safe_VkDisplayNativeHdrSurfaceCapabilitiesAMD(const VkDisplayNativeHdrSurfaceCapabilitiesAMD* in_struct);
    safe_VkDisplayNativeHdrSurfaceCapabilitiesAMD(const safe_VkDisplayNativeHdrSurfaceCapabilitiesAMD& src);
    safe_VkDisplayNativeHdrSurfaceCapabilitiesAMD& operator=(const safe_VkDisplayNativeHdrSurfaceCapabilitiesAMD& src);
    safe_VkDisplayNativeHdrSurfaceCapabilitiesAMD();
    ~safe_VkDisplayNativeHdrSurfaceCapabilitiesAMD();
    void initialize(const VkDisplayNativeHdrSurfaceCapabilitiesAMD* in_struct);
    void initialize(const safe_VkDisplayNativeHdrSurfaceCapabilitiesAMD* src);
    VkDisplayNativeHdrSurfaceCapabilitiesAMD *ptr() { return reinterpret_cast<VkDisplayNativeHdrSurfaceCapabilitiesAMD *>(this); }
    VkDisplayNativeHdrSurfaceCapabilitiesAMD const *ptr() const { return reinterpret_cast<VkDisplayNativeHdrSurfaceCapabilitiesAMD const *>(this); }
};

struct safe_VkSwapchainDisplayNativeHdrCreateInfoAMD {
    VkStructureType sType;
    const void* pNext;
    VkBool32 localDimmingEnable;
    safe_VkSwapchainDisplayNativeHdrCreateInfoAMD(const VkSwapchainDisplayNativeHdrCreateInfoAMD* in_struct);
    safe_VkSwapchainDisplayNativeHdrCreateInfoAMD(const safe_VkSwapchainDisplayNativeHdrCreateInfoAMD& src);
    safe_VkSwapchainDisplayNativeHdrCreateInfoAMD& operator=(const safe_VkSwapchainDisplayNativeHdrCreateInfoAMD& src);
    safe_VkSwapchainDisplayNativeHdrCreateInfoAMD();
    ~safe_VkSwapchainDisplayNativeHdrCreateInfoAMD();
    void initialize(const VkSwapchainDisplayNativeHdrCreateInfoAMD* in_struct);
    void initialize(const safe_VkSwapchainDisplayNativeHdrCreateInfoAMD* src);
    VkSwapchainDisplayNativeHdrCreateInfoAMD *ptr() { return reinterpret_cast<VkSwapchainDisplayNativeHdrCreateInfoAMD *>(this); }
    VkSwapchainDisplayNativeHdrCreateInfoAMD const *ptr() const { return reinterpret_cast<VkSwapchainDisplayNativeHdrCreateInfoAMD const *>(this); }
};

#ifdef VK_USE_PLATFORM_FUCHSIA
struct safe_VkImagePipeSurfaceCreateInfoFUCHSIA {
    VkStructureType sType;
    const void* pNext;
    VkImagePipeSurfaceCreateFlagsFUCHSIA flags;
    zx_handle_t imagePipeHandle;
    safe_VkImagePipeSurfaceCreateInfoFUCHSIA(const VkImagePipeSurfaceCreateInfoFUCHSIA* in_struct);
    safe_VkImagePipeSurfaceCreateInfoFUCHSIA(const safe_VkImagePipeSurfaceCreateInfoFUCHSIA& src);
    safe_VkImagePipeSurfaceCreateInfoFUCHSIA& operator=(const safe_VkImagePipeSurfaceCreateInfoFUCHSIA& src);
    safe_VkImagePipeSurfaceCreateInfoFUCHSIA();
    ~safe_VkImagePipeSurfaceCreateInfoFUCHSIA();
    void initialize(const VkImagePipeSurfaceCreateInfoFUCHSIA* in_struct);
    void initialize(const safe_VkImagePipeSurfaceCreateInfoFUCHSIA* src);
    VkImagePipeSurfaceCreateInfoFUCHSIA *ptr() { return reinterpret_cast<VkImagePipeSurfaceCreateInfoFUCHSIA *>(this); }
    VkImagePipeSurfaceCreateInfoFUCHSIA const *ptr() const { return reinterpret_cast<VkImagePipeSurfaceCreateInfoFUCHSIA const *>(this); }
};
#endif // VK_USE_PLATFORM_FUCHSIA

#ifdef VK_USE_PLATFORM_METAL_EXT
struct safe_VkMetalSurfaceCreateInfoEXT {
    VkStructureType sType;
    const void* pNext;
    VkMetalSurfaceCreateFlagsEXT flags;
    const CAMetalLayer* pLayer;
    safe_VkMetalSurfaceCreateInfoEXT(const VkMetalSurfaceCreateInfoEXT* in_struct);
    safe_VkMetalSurfaceCreateInfoEXT(const safe_VkMetalSurfaceCreateInfoEXT& src);
    safe_VkMetalSurfaceCreateInfoEXT& operator=(const safe_VkMetalSurfaceCreateInfoEXT& src);
    safe_VkMetalSurfaceCreateInfoEXT();
    ~safe_VkMetalSurfaceCreateInfoEXT();
    void initialize(const VkMetalSurfaceCreateInfoEXT* in_struct);
    void initialize(const safe_VkMetalSurfaceCreateInfoEXT* src);
    VkMetalSurfaceCreateInfoEXT *ptr() { return reinterpret_cast<VkMetalSurfaceCreateInfoEXT *>(this); }
    VkMetalSurfaceCreateInfoEXT const *ptr() const { return reinterpret_cast<VkMetalSurfaceCreateInfoEXT const *>(this); }
};
#endif // VK_USE_PLATFORM_METAL_EXT

struct safe_VkPhysicalDeviceFragmentDensityMapFeaturesEXT {
    VkStructureType sType;
    void* pNext;
    VkBool32 fragmentDensityMap;
    VkBool32 fragmentDensityMapDynamic;
    VkBool32 fragmentDensityMapNonSubsampledImages;
    safe_VkPhysicalDeviceFragmentDensityMapFeaturesEXT(const VkPhysicalDeviceFragmentDensityMapFeaturesEXT* in_struct);
    safe_VkPhysicalDeviceFragmentDensityMapFeaturesEXT(const safe_VkPhysicalDeviceFragmentDensityMapFeaturesEXT& src);
    safe_VkPhysicalDeviceFragmentDensityMapFeaturesEXT& operator=(const safe_VkPhysicalDeviceFragmentDensityMapFeaturesEXT& src);
    safe_VkPhysicalDeviceFragmentDensityMapFeaturesEXT();
    ~safe_VkPhysicalDeviceFragmentDensityMapFeaturesEXT();
    void initialize(const VkPhysicalDeviceFragmentDensityMapFeaturesEXT* in_struct);
    void initialize(const safe_VkPhysicalDeviceFragmentDensityMapFeaturesEXT* src);
    VkPhysicalDeviceFragmentDensityMapFeaturesEXT *ptr() { return reinterpret_cast<VkPhysicalDeviceFragmentDensityMapFeaturesEXT *>(this); }
    VkPhysicalDeviceFragmentDensityMapFeaturesEXT const *ptr() const { return reinterpret_cast<VkPhysicalDeviceFragmentDensityMapFeaturesEXT const *>(this); }
};

struct safe_VkPhysicalDeviceFragmentDensityMapPropertiesEXT {
    VkStructureType sType;
    void* pNext;
    VkExtent2D minFragmentDensityTexelSize;
    VkExtent2D maxFragmentDensityTexelSize;
    VkBool32 fragmentDensityInvocations;
    safe_VkPhysicalDeviceFragmentDensityMapPropertiesEXT(const VkPhysicalDeviceFragmentDensityMapPropertiesEXT* in_struct);
    safe_VkPhysicalDeviceFragmentDensityMapPropertiesEXT(const safe_VkPhysicalDeviceFragmentDensityMapPropertiesEXT& src);
    safe_VkPhysicalDeviceFragmentDensityMapPropertiesEXT& operator=(const safe_VkPhysicalDeviceFragmentDensityMapPropertiesEXT& src);
    safe_VkPhysicalDeviceFragmentDensityMapPropertiesEXT();
    ~safe_VkPhysicalDeviceFragmentDensityMapPropertiesEXT();
    void initialize(const VkPhysicalDeviceFragmentDensityMapPropertiesEXT* in_struct);
    void initialize(const safe_VkPhysicalDeviceFragmentDensityMapPropertiesEXT* src);
    VkPhysicalDeviceFragmentDensityMapPropertiesEXT *ptr() { return reinterpret_cast<VkPhysicalDeviceFragmentDensityMapPropertiesEXT *>(this); }
    VkPhysicalDeviceFragmentDensityMapPropertiesEXT const *ptr() const { return reinterpret_cast<VkPhysicalDeviceFragmentDensityMapPropertiesEXT const *>(this); }
};

struct safe_VkRenderPassFragmentDensityMapCreateInfoEXT {
    VkStructureType sType;
    const void* pNext;
    VkAttachmentReference fragmentDensityMapAttachment;
    safe_VkRenderPassFragmentDensityMapCreateInfoEXT(const VkRenderPassFragmentDensityMapCreateInfoEXT* in_struct);
    safe_VkRenderPassFragmentDensityMapCreateInfoEXT(const safe_VkRenderPassFragmentDensityMapCreateInfoEXT& src);
    safe_VkRenderPassFragmentDensityMapCreateInfoEXT& operator=(const safe_VkRenderPassFragmentDensityMapCreateInfoEXT& src);
    safe_VkRenderPassFragmentDensityMapCreateInfoEXT();
    ~safe_VkRenderPassFragmentDensityMapCreateInfoEXT();
    void initialize(const VkRenderPassFragmentDensityMapCreateInfoEXT* in_struct);
    void initialize(const safe_VkRenderPassFragmentDensityMapCreateInfoEXT* src);
    VkRenderPassFragmentDensityMapCreateInfoEXT *ptr() { return reinterpret_cast<VkRenderPassFragmentDensityMapCreateInfoEXT *>(this); }
    VkRenderPassFragmentDensityMapCreateInfoEXT const *ptr() const { return reinterpret_cast<VkRenderPassFragmentDensityMapCreateInfoEXT const *>(this); }
};

struct safe_VkPhysicalDeviceScalarBlockLayoutFeaturesEXT {
    VkStructureType sType;
    void* pNext;
    VkBool32 scalarBlockLayout;
    safe_VkPhysicalDeviceScalarBlockLayoutFeaturesEXT(const VkPhysicalDeviceScalarBlockLayoutFeaturesEXT* in_struct);
    safe_VkPhysicalDeviceScalarBlockLayoutFeaturesEXT(const safe_VkPhysicalDeviceScalarBlockLayoutFeaturesEXT& src);
    safe_VkPhysicalDeviceScalarBlockLayoutFeaturesEXT& operator=(const safe_VkPhysicalDeviceScalarBlockLayoutFeaturesEXT& src);
    safe_VkPhysicalDeviceScalarBlockLayoutFeaturesEXT();
    ~safe_VkPhysicalDeviceScalarBlockLayoutFeaturesEXT();
    void initialize(const VkPhysicalDeviceScalarBlockLayoutFeaturesEXT* in_struct);
    void initialize(const safe_VkPhysicalDeviceScalarBlockLayoutFeaturesEXT* src);
    VkPhysicalDeviceScalarBlockLayoutFeaturesEXT *ptr() { return reinterpret_cast<VkPhysicalDeviceScalarBlockLayoutFeaturesEXT *>(this); }
    VkPhysicalDeviceScalarBlockLayoutFeaturesEXT const *ptr() const { return reinterpret_cast<VkPhysicalDeviceScalarBlockLayoutFeaturesEXT const *>(this); }
};

struct safe_VkPhysicalDeviceSubgroupSizeControlFeaturesEXT {
    VkStructureType sType;
    void* pNext;
    VkBool32 subgroupSizeControl;
    VkBool32 computeFullSubgroups;
    safe_VkPhysicalDeviceSubgroupSizeControlFeaturesEXT(const VkPhysicalDeviceSubgroupSizeControlFeaturesEXT* in_struct);
    safe_VkPhysicalDeviceSubgroupSizeControlFeaturesEXT(const safe_VkPhysicalDeviceSubgroupSizeControlFeaturesEXT& src);
    safe_VkPhysicalDeviceSubgroupSizeControlFeaturesEXT& operator=(const safe_VkPhysicalDeviceSubgroupSizeControlFeaturesEXT& src);
    safe_VkPhysicalDeviceSubgroupSizeControlFeaturesEXT();
    ~safe_VkPhysicalDeviceSubgroupSizeControlFeaturesEXT();
    void initialize(const VkPhysicalDeviceSubgroupSizeControlFeaturesEXT* in_struct);
    void initialize(const safe_VkPhysicalDeviceSubgroupSizeControlFeaturesEXT* src);
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
    safe_VkPhysicalDeviceSubgroupSizeControlPropertiesEXT(const safe_VkPhysicalDeviceSubgroupSizeControlPropertiesEXT& src);
    safe_VkPhysicalDeviceSubgroupSizeControlPropertiesEXT& operator=(const safe_VkPhysicalDeviceSubgroupSizeControlPropertiesEXT& src);
    safe_VkPhysicalDeviceSubgroupSizeControlPropertiesEXT();
    ~safe_VkPhysicalDeviceSubgroupSizeControlPropertiesEXT();
    void initialize(const VkPhysicalDeviceSubgroupSizeControlPropertiesEXT* in_struct);
    void initialize(const safe_VkPhysicalDeviceSubgroupSizeControlPropertiesEXT* src);
    VkPhysicalDeviceSubgroupSizeControlPropertiesEXT *ptr() { return reinterpret_cast<VkPhysicalDeviceSubgroupSizeControlPropertiesEXT *>(this); }
    VkPhysicalDeviceSubgroupSizeControlPropertiesEXT const *ptr() const { return reinterpret_cast<VkPhysicalDeviceSubgroupSizeControlPropertiesEXT const *>(this); }
};

struct safe_VkPipelineShaderStageRequiredSubgroupSizeCreateInfoEXT {
    VkStructureType sType;
    void* pNext;
    uint32_t requiredSubgroupSize;
    safe_VkPipelineShaderStageRequiredSubgroupSizeCreateInfoEXT(const VkPipelineShaderStageRequiredSubgroupSizeCreateInfoEXT* in_struct);
    safe_VkPipelineShaderStageRequiredSubgroupSizeCreateInfoEXT(const safe_VkPipelineShaderStageRequiredSubgroupSizeCreateInfoEXT& src);
    safe_VkPipelineShaderStageRequiredSubgroupSizeCreateInfoEXT& operator=(const safe_VkPipelineShaderStageRequiredSubgroupSizeCreateInfoEXT& src);
    safe_VkPipelineShaderStageRequiredSubgroupSizeCreateInfoEXT();
    ~safe_VkPipelineShaderStageRequiredSubgroupSizeCreateInfoEXT();
    void initialize(const VkPipelineShaderStageRequiredSubgroupSizeCreateInfoEXT* in_struct);
    void initialize(const safe_VkPipelineShaderStageRequiredSubgroupSizeCreateInfoEXT* src);
    VkPipelineShaderStageRequiredSubgroupSizeCreateInfoEXT *ptr() { return reinterpret_cast<VkPipelineShaderStageRequiredSubgroupSizeCreateInfoEXT *>(this); }
    VkPipelineShaderStageRequiredSubgroupSizeCreateInfoEXT const *ptr() const { return reinterpret_cast<VkPipelineShaderStageRequiredSubgroupSizeCreateInfoEXT const *>(this); }
};

struct safe_VkPhysicalDeviceShaderCoreProperties2AMD {
    VkStructureType sType;
    void* pNext;
    VkShaderCorePropertiesFlagsAMD shaderCoreFeatures;
    uint32_t activeComputeUnitCount;
    safe_VkPhysicalDeviceShaderCoreProperties2AMD(const VkPhysicalDeviceShaderCoreProperties2AMD* in_struct);
    safe_VkPhysicalDeviceShaderCoreProperties2AMD(const safe_VkPhysicalDeviceShaderCoreProperties2AMD& src);
    safe_VkPhysicalDeviceShaderCoreProperties2AMD& operator=(const safe_VkPhysicalDeviceShaderCoreProperties2AMD& src);
    safe_VkPhysicalDeviceShaderCoreProperties2AMD();
    ~safe_VkPhysicalDeviceShaderCoreProperties2AMD();
    void initialize(const VkPhysicalDeviceShaderCoreProperties2AMD* in_struct);
    void initialize(const safe_VkPhysicalDeviceShaderCoreProperties2AMD* src);
    VkPhysicalDeviceShaderCoreProperties2AMD *ptr() { return reinterpret_cast<VkPhysicalDeviceShaderCoreProperties2AMD *>(this); }
    VkPhysicalDeviceShaderCoreProperties2AMD const *ptr() const { return reinterpret_cast<VkPhysicalDeviceShaderCoreProperties2AMD const *>(this); }
};

struct safe_VkPhysicalDeviceCoherentMemoryFeaturesAMD {
    VkStructureType sType;
    void* pNext;
    VkBool32 deviceCoherentMemory;
    safe_VkPhysicalDeviceCoherentMemoryFeaturesAMD(const VkPhysicalDeviceCoherentMemoryFeaturesAMD* in_struct);
    safe_VkPhysicalDeviceCoherentMemoryFeaturesAMD(const safe_VkPhysicalDeviceCoherentMemoryFeaturesAMD& src);
    safe_VkPhysicalDeviceCoherentMemoryFeaturesAMD& operator=(const safe_VkPhysicalDeviceCoherentMemoryFeaturesAMD& src);
    safe_VkPhysicalDeviceCoherentMemoryFeaturesAMD();
    ~safe_VkPhysicalDeviceCoherentMemoryFeaturesAMD();
    void initialize(const VkPhysicalDeviceCoherentMemoryFeaturesAMD* in_struct);
    void initialize(const safe_VkPhysicalDeviceCoherentMemoryFeaturesAMD* src);
    VkPhysicalDeviceCoherentMemoryFeaturesAMD *ptr() { return reinterpret_cast<VkPhysicalDeviceCoherentMemoryFeaturesAMD *>(this); }
    VkPhysicalDeviceCoherentMemoryFeaturesAMD const *ptr() const { return reinterpret_cast<VkPhysicalDeviceCoherentMemoryFeaturesAMD const *>(this); }
};

struct safe_VkPhysicalDeviceMemoryBudgetPropertiesEXT {
    VkStructureType sType;
    void* pNext;
    VkDeviceSize heapBudget[VK_MAX_MEMORY_HEAPS];
    VkDeviceSize heapUsage[VK_MAX_MEMORY_HEAPS];
    safe_VkPhysicalDeviceMemoryBudgetPropertiesEXT(const VkPhysicalDeviceMemoryBudgetPropertiesEXT* in_struct);
    safe_VkPhysicalDeviceMemoryBudgetPropertiesEXT(const safe_VkPhysicalDeviceMemoryBudgetPropertiesEXT& src);
    safe_VkPhysicalDeviceMemoryBudgetPropertiesEXT& operator=(const safe_VkPhysicalDeviceMemoryBudgetPropertiesEXT& src);
    safe_VkPhysicalDeviceMemoryBudgetPropertiesEXT();
    ~safe_VkPhysicalDeviceMemoryBudgetPropertiesEXT();
    void initialize(const VkPhysicalDeviceMemoryBudgetPropertiesEXT* in_struct);
    void initialize(const safe_VkPhysicalDeviceMemoryBudgetPropertiesEXT* src);
    VkPhysicalDeviceMemoryBudgetPropertiesEXT *ptr() { return reinterpret_cast<VkPhysicalDeviceMemoryBudgetPropertiesEXT *>(this); }
    VkPhysicalDeviceMemoryBudgetPropertiesEXT const *ptr() const { return reinterpret_cast<VkPhysicalDeviceMemoryBudgetPropertiesEXT const *>(this); }
};

struct safe_VkPhysicalDeviceMemoryPriorityFeaturesEXT {
    VkStructureType sType;
    void* pNext;
    VkBool32 memoryPriority;
    safe_VkPhysicalDeviceMemoryPriorityFeaturesEXT(const VkPhysicalDeviceMemoryPriorityFeaturesEXT* in_struct);
    safe_VkPhysicalDeviceMemoryPriorityFeaturesEXT(const safe_VkPhysicalDeviceMemoryPriorityFeaturesEXT& src);
    safe_VkPhysicalDeviceMemoryPriorityFeaturesEXT& operator=(const safe_VkPhysicalDeviceMemoryPriorityFeaturesEXT& src);
    safe_VkPhysicalDeviceMemoryPriorityFeaturesEXT();
    ~safe_VkPhysicalDeviceMemoryPriorityFeaturesEXT();
    void initialize(const VkPhysicalDeviceMemoryPriorityFeaturesEXT* in_struct);
    void initialize(const safe_VkPhysicalDeviceMemoryPriorityFeaturesEXT* src);
    VkPhysicalDeviceMemoryPriorityFeaturesEXT *ptr() { return reinterpret_cast<VkPhysicalDeviceMemoryPriorityFeaturesEXT *>(this); }
    VkPhysicalDeviceMemoryPriorityFeaturesEXT const *ptr() const { return reinterpret_cast<VkPhysicalDeviceMemoryPriorityFeaturesEXT const *>(this); }
};

struct safe_VkMemoryPriorityAllocateInfoEXT {
    VkStructureType sType;
    const void* pNext;
    float priority;
    safe_VkMemoryPriorityAllocateInfoEXT(const VkMemoryPriorityAllocateInfoEXT* in_struct);
    safe_VkMemoryPriorityAllocateInfoEXT(const safe_VkMemoryPriorityAllocateInfoEXT& src);
    safe_VkMemoryPriorityAllocateInfoEXT& operator=(const safe_VkMemoryPriorityAllocateInfoEXT& src);
    safe_VkMemoryPriorityAllocateInfoEXT();
    ~safe_VkMemoryPriorityAllocateInfoEXT();
    void initialize(const VkMemoryPriorityAllocateInfoEXT* in_struct);
    void initialize(const safe_VkMemoryPriorityAllocateInfoEXT* src);
    VkMemoryPriorityAllocateInfoEXT *ptr() { return reinterpret_cast<VkMemoryPriorityAllocateInfoEXT *>(this); }
    VkMemoryPriorityAllocateInfoEXT const *ptr() const { return reinterpret_cast<VkMemoryPriorityAllocateInfoEXT const *>(this); }
};

struct safe_VkPhysicalDeviceDedicatedAllocationImageAliasingFeaturesNV {
    VkStructureType sType;
    void* pNext;
    VkBool32 dedicatedAllocationImageAliasing;
    safe_VkPhysicalDeviceDedicatedAllocationImageAliasingFeaturesNV(const VkPhysicalDeviceDedicatedAllocationImageAliasingFeaturesNV* in_struct);
    safe_VkPhysicalDeviceDedicatedAllocationImageAliasingFeaturesNV(const safe_VkPhysicalDeviceDedicatedAllocationImageAliasingFeaturesNV& src);
    safe_VkPhysicalDeviceDedicatedAllocationImageAliasingFeaturesNV& operator=(const safe_VkPhysicalDeviceDedicatedAllocationImageAliasingFeaturesNV& src);
    safe_VkPhysicalDeviceDedicatedAllocationImageAliasingFeaturesNV();
    ~safe_VkPhysicalDeviceDedicatedAllocationImageAliasingFeaturesNV();
    void initialize(const VkPhysicalDeviceDedicatedAllocationImageAliasingFeaturesNV* in_struct);
    void initialize(const safe_VkPhysicalDeviceDedicatedAllocationImageAliasingFeaturesNV* src);
    VkPhysicalDeviceDedicatedAllocationImageAliasingFeaturesNV *ptr() { return reinterpret_cast<VkPhysicalDeviceDedicatedAllocationImageAliasingFeaturesNV *>(this); }
    VkPhysicalDeviceDedicatedAllocationImageAliasingFeaturesNV const *ptr() const { return reinterpret_cast<VkPhysicalDeviceDedicatedAllocationImageAliasingFeaturesNV const *>(this); }
};

struct safe_VkPhysicalDeviceBufferDeviceAddressFeaturesEXT {
    VkStructureType sType;
    void* pNext;
    VkBool32 bufferDeviceAddress;
    VkBool32 bufferDeviceAddressCaptureReplay;
    VkBool32 bufferDeviceAddressMultiDevice;
    safe_VkPhysicalDeviceBufferDeviceAddressFeaturesEXT(const VkPhysicalDeviceBufferDeviceAddressFeaturesEXT* in_struct);
    safe_VkPhysicalDeviceBufferDeviceAddressFeaturesEXT(const safe_VkPhysicalDeviceBufferDeviceAddressFeaturesEXT& src);
    safe_VkPhysicalDeviceBufferDeviceAddressFeaturesEXT& operator=(const safe_VkPhysicalDeviceBufferDeviceAddressFeaturesEXT& src);
    safe_VkPhysicalDeviceBufferDeviceAddressFeaturesEXT();
    ~safe_VkPhysicalDeviceBufferDeviceAddressFeaturesEXT();
    void initialize(const VkPhysicalDeviceBufferDeviceAddressFeaturesEXT* in_struct);
    void initialize(const safe_VkPhysicalDeviceBufferDeviceAddressFeaturesEXT* src);
    VkPhysicalDeviceBufferDeviceAddressFeaturesEXT *ptr() { return reinterpret_cast<VkPhysicalDeviceBufferDeviceAddressFeaturesEXT *>(this); }
    VkPhysicalDeviceBufferDeviceAddressFeaturesEXT const *ptr() const { return reinterpret_cast<VkPhysicalDeviceBufferDeviceAddressFeaturesEXT const *>(this); }
};

struct safe_VkBufferDeviceAddressInfoEXT {
    VkStructureType sType;
    const void* pNext;
    VkBuffer buffer;
    safe_VkBufferDeviceAddressInfoEXT(const VkBufferDeviceAddressInfoEXT* in_struct);
    safe_VkBufferDeviceAddressInfoEXT(const safe_VkBufferDeviceAddressInfoEXT& src);
    safe_VkBufferDeviceAddressInfoEXT& operator=(const safe_VkBufferDeviceAddressInfoEXT& src);
    safe_VkBufferDeviceAddressInfoEXT();
    ~safe_VkBufferDeviceAddressInfoEXT();
    void initialize(const VkBufferDeviceAddressInfoEXT* in_struct);
    void initialize(const safe_VkBufferDeviceAddressInfoEXT* src);
    VkBufferDeviceAddressInfoEXT *ptr() { return reinterpret_cast<VkBufferDeviceAddressInfoEXT *>(this); }
    VkBufferDeviceAddressInfoEXT const *ptr() const { return reinterpret_cast<VkBufferDeviceAddressInfoEXT const *>(this); }
};

struct safe_VkBufferDeviceAddressCreateInfoEXT {
    VkStructureType sType;
    const void* pNext;
    VkDeviceAddress deviceAddress;
    safe_VkBufferDeviceAddressCreateInfoEXT(const VkBufferDeviceAddressCreateInfoEXT* in_struct);
    safe_VkBufferDeviceAddressCreateInfoEXT(const safe_VkBufferDeviceAddressCreateInfoEXT& src);
    safe_VkBufferDeviceAddressCreateInfoEXT& operator=(const safe_VkBufferDeviceAddressCreateInfoEXT& src);
    safe_VkBufferDeviceAddressCreateInfoEXT();
    ~safe_VkBufferDeviceAddressCreateInfoEXT();
    void initialize(const VkBufferDeviceAddressCreateInfoEXT* in_struct);
    void initialize(const safe_VkBufferDeviceAddressCreateInfoEXT* src);
    VkBufferDeviceAddressCreateInfoEXT *ptr() { return reinterpret_cast<VkBufferDeviceAddressCreateInfoEXT *>(this); }
    VkBufferDeviceAddressCreateInfoEXT const *ptr() const { return reinterpret_cast<VkBufferDeviceAddressCreateInfoEXT const *>(this); }
};

struct safe_VkImageStencilUsageCreateInfoEXT {
    VkStructureType sType;
    const void* pNext;
    VkImageUsageFlags stencilUsage;
    safe_VkImageStencilUsageCreateInfoEXT(const VkImageStencilUsageCreateInfoEXT* in_struct);
    safe_VkImageStencilUsageCreateInfoEXT(const safe_VkImageStencilUsageCreateInfoEXT& src);
    safe_VkImageStencilUsageCreateInfoEXT& operator=(const safe_VkImageStencilUsageCreateInfoEXT& src);
    safe_VkImageStencilUsageCreateInfoEXT();
    ~safe_VkImageStencilUsageCreateInfoEXT();
    void initialize(const VkImageStencilUsageCreateInfoEXT* in_struct);
    void initialize(const safe_VkImageStencilUsageCreateInfoEXT* src);
    VkImageStencilUsageCreateInfoEXT *ptr() { return reinterpret_cast<VkImageStencilUsageCreateInfoEXT *>(this); }
    VkImageStencilUsageCreateInfoEXT const *ptr() const { return reinterpret_cast<VkImageStencilUsageCreateInfoEXT const *>(this); }
};

struct safe_VkValidationFeaturesEXT {
    VkStructureType sType;
    const void* pNext;
    uint32_t enabledValidationFeatureCount;
    const VkValidationFeatureEnableEXT* pEnabledValidationFeatures;
    uint32_t disabledValidationFeatureCount;
    const VkValidationFeatureDisableEXT* pDisabledValidationFeatures;
    safe_VkValidationFeaturesEXT(const VkValidationFeaturesEXT* in_struct);
    safe_VkValidationFeaturesEXT(const safe_VkValidationFeaturesEXT& src);
    safe_VkValidationFeaturesEXT& operator=(const safe_VkValidationFeaturesEXT& src);
    safe_VkValidationFeaturesEXT();
    ~safe_VkValidationFeaturesEXT();
    void initialize(const VkValidationFeaturesEXT* in_struct);
    void initialize(const safe_VkValidationFeaturesEXT* src);
    VkValidationFeaturesEXT *ptr() { return reinterpret_cast<VkValidationFeaturesEXT *>(this); }
    VkValidationFeaturesEXT const *ptr() const { return reinterpret_cast<VkValidationFeaturesEXT const *>(this); }
};

struct safe_VkCooperativeMatrixPropertiesNV {
    VkStructureType sType;
    void* pNext;
    uint32_t MSize;
    uint32_t NSize;
    uint32_t KSize;
    VkComponentTypeNV AType;
    VkComponentTypeNV BType;
    VkComponentTypeNV CType;
    VkComponentTypeNV DType;
    VkScopeNV scope;
    safe_VkCooperativeMatrixPropertiesNV(const VkCooperativeMatrixPropertiesNV* in_struct);
    safe_VkCooperativeMatrixPropertiesNV(const safe_VkCooperativeMatrixPropertiesNV& src);
    safe_VkCooperativeMatrixPropertiesNV& operator=(const safe_VkCooperativeMatrixPropertiesNV& src);
    safe_VkCooperativeMatrixPropertiesNV();
    ~safe_VkCooperativeMatrixPropertiesNV();
    void initialize(const VkCooperativeMatrixPropertiesNV* in_struct);
    void initialize(const safe_VkCooperativeMatrixPropertiesNV* src);
    VkCooperativeMatrixPropertiesNV *ptr() { return reinterpret_cast<VkCooperativeMatrixPropertiesNV *>(this); }
    VkCooperativeMatrixPropertiesNV const *ptr() const { return reinterpret_cast<VkCooperativeMatrixPropertiesNV const *>(this); }
};

struct safe_VkPhysicalDeviceCooperativeMatrixFeaturesNV {
    VkStructureType sType;
    void* pNext;
    VkBool32 cooperativeMatrix;
    VkBool32 cooperativeMatrixRobustBufferAccess;
    safe_VkPhysicalDeviceCooperativeMatrixFeaturesNV(const VkPhysicalDeviceCooperativeMatrixFeaturesNV* in_struct);
    safe_VkPhysicalDeviceCooperativeMatrixFeaturesNV(const safe_VkPhysicalDeviceCooperativeMatrixFeaturesNV& src);
    safe_VkPhysicalDeviceCooperativeMatrixFeaturesNV& operator=(const safe_VkPhysicalDeviceCooperativeMatrixFeaturesNV& src);
    safe_VkPhysicalDeviceCooperativeMatrixFeaturesNV();
    ~safe_VkPhysicalDeviceCooperativeMatrixFeaturesNV();
    void initialize(const VkPhysicalDeviceCooperativeMatrixFeaturesNV* in_struct);
    void initialize(const safe_VkPhysicalDeviceCooperativeMatrixFeaturesNV* src);
    VkPhysicalDeviceCooperativeMatrixFeaturesNV *ptr() { return reinterpret_cast<VkPhysicalDeviceCooperativeMatrixFeaturesNV *>(this); }
    VkPhysicalDeviceCooperativeMatrixFeaturesNV const *ptr() const { return reinterpret_cast<VkPhysicalDeviceCooperativeMatrixFeaturesNV const *>(this); }
};

struct safe_VkPhysicalDeviceCooperativeMatrixPropertiesNV {
    VkStructureType sType;
    void* pNext;
    VkShaderStageFlags cooperativeMatrixSupportedStages;
    safe_VkPhysicalDeviceCooperativeMatrixPropertiesNV(const VkPhysicalDeviceCooperativeMatrixPropertiesNV* in_struct);
    safe_VkPhysicalDeviceCooperativeMatrixPropertiesNV(const safe_VkPhysicalDeviceCooperativeMatrixPropertiesNV& src);
    safe_VkPhysicalDeviceCooperativeMatrixPropertiesNV& operator=(const safe_VkPhysicalDeviceCooperativeMatrixPropertiesNV& src);
    safe_VkPhysicalDeviceCooperativeMatrixPropertiesNV();
    ~safe_VkPhysicalDeviceCooperativeMatrixPropertiesNV();
    void initialize(const VkPhysicalDeviceCooperativeMatrixPropertiesNV* in_struct);
    void initialize(const safe_VkPhysicalDeviceCooperativeMatrixPropertiesNV* src);
    VkPhysicalDeviceCooperativeMatrixPropertiesNV *ptr() { return reinterpret_cast<VkPhysicalDeviceCooperativeMatrixPropertiesNV *>(this); }
    VkPhysicalDeviceCooperativeMatrixPropertiesNV const *ptr() const { return reinterpret_cast<VkPhysicalDeviceCooperativeMatrixPropertiesNV const *>(this); }
};

struct safe_VkPhysicalDeviceCoverageReductionModeFeaturesNV {
    VkStructureType sType;
    void* pNext;
    VkBool32 coverageReductionMode;
    safe_VkPhysicalDeviceCoverageReductionModeFeaturesNV(const VkPhysicalDeviceCoverageReductionModeFeaturesNV* in_struct);
    safe_VkPhysicalDeviceCoverageReductionModeFeaturesNV(const safe_VkPhysicalDeviceCoverageReductionModeFeaturesNV& src);
    safe_VkPhysicalDeviceCoverageReductionModeFeaturesNV& operator=(const safe_VkPhysicalDeviceCoverageReductionModeFeaturesNV& src);
    safe_VkPhysicalDeviceCoverageReductionModeFeaturesNV();
    ~safe_VkPhysicalDeviceCoverageReductionModeFeaturesNV();
    void initialize(const VkPhysicalDeviceCoverageReductionModeFeaturesNV* in_struct);
    void initialize(const safe_VkPhysicalDeviceCoverageReductionModeFeaturesNV* src);
    VkPhysicalDeviceCoverageReductionModeFeaturesNV *ptr() { return reinterpret_cast<VkPhysicalDeviceCoverageReductionModeFeaturesNV *>(this); }
    VkPhysicalDeviceCoverageReductionModeFeaturesNV const *ptr() const { return reinterpret_cast<VkPhysicalDeviceCoverageReductionModeFeaturesNV const *>(this); }
};

struct safe_VkPipelineCoverageReductionStateCreateInfoNV {
    VkStructureType sType;
    const void* pNext;
    VkPipelineCoverageReductionStateCreateFlagsNV flags;
    VkCoverageReductionModeNV coverageReductionMode;
    safe_VkPipelineCoverageReductionStateCreateInfoNV(const VkPipelineCoverageReductionStateCreateInfoNV* in_struct);
    safe_VkPipelineCoverageReductionStateCreateInfoNV(const safe_VkPipelineCoverageReductionStateCreateInfoNV& src);
    safe_VkPipelineCoverageReductionStateCreateInfoNV& operator=(const safe_VkPipelineCoverageReductionStateCreateInfoNV& src);
    safe_VkPipelineCoverageReductionStateCreateInfoNV();
    ~safe_VkPipelineCoverageReductionStateCreateInfoNV();
    void initialize(const VkPipelineCoverageReductionStateCreateInfoNV* in_struct);
    void initialize(const safe_VkPipelineCoverageReductionStateCreateInfoNV* src);
    VkPipelineCoverageReductionStateCreateInfoNV *ptr() { return reinterpret_cast<VkPipelineCoverageReductionStateCreateInfoNV *>(this); }
    VkPipelineCoverageReductionStateCreateInfoNV const *ptr() const { return reinterpret_cast<VkPipelineCoverageReductionStateCreateInfoNV const *>(this); }
};

struct safe_VkFramebufferMixedSamplesCombinationNV {
    VkStructureType sType;
    void* pNext;
    VkCoverageReductionModeNV coverageReductionMode;
    VkSampleCountFlagBits rasterizationSamples;
    VkSampleCountFlags depthStencilSamples;
    VkSampleCountFlags colorSamples;
    safe_VkFramebufferMixedSamplesCombinationNV(const VkFramebufferMixedSamplesCombinationNV* in_struct);
    safe_VkFramebufferMixedSamplesCombinationNV(const safe_VkFramebufferMixedSamplesCombinationNV& src);
    safe_VkFramebufferMixedSamplesCombinationNV& operator=(const safe_VkFramebufferMixedSamplesCombinationNV& src);
    safe_VkFramebufferMixedSamplesCombinationNV();
    ~safe_VkFramebufferMixedSamplesCombinationNV();
    void initialize(const VkFramebufferMixedSamplesCombinationNV* in_struct);
    void initialize(const safe_VkFramebufferMixedSamplesCombinationNV* src);
    VkFramebufferMixedSamplesCombinationNV *ptr() { return reinterpret_cast<VkFramebufferMixedSamplesCombinationNV *>(this); }
    VkFramebufferMixedSamplesCombinationNV const *ptr() const { return reinterpret_cast<VkFramebufferMixedSamplesCombinationNV const *>(this); }
};

struct safe_VkPhysicalDeviceFragmentShaderInterlockFeaturesEXT {
    VkStructureType sType;
    void* pNext;
    VkBool32 fragmentShaderSampleInterlock;
    VkBool32 fragmentShaderPixelInterlock;
    VkBool32 fragmentShaderShadingRateInterlock;
    safe_VkPhysicalDeviceFragmentShaderInterlockFeaturesEXT(const VkPhysicalDeviceFragmentShaderInterlockFeaturesEXT* in_struct);
    safe_VkPhysicalDeviceFragmentShaderInterlockFeaturesEXT(const safe_VkPhysicalDeviceFragmentShaderInterlockFeaturesEXT& src);
    safe_VkPhysicalDeviceFragmentShaderInterlockFeaturesEXT& operator=(const safe_VkPhysicalDeviceFragmentShaderInterlockFeaturesEXT& src);
    safe_VkPhysicalDeviceFragmentShaderInterlockFeaturesEXT();
    ~safe_VkPhysicalDeviceFragmentShaderInterlockFeaturesEXT();
    void initialize(const VkPhysicalDeviceFragmentShaderInterlockFeaturesEXT* in_struct);
    void initialize(const safe_VkPhysicalDeviceFragmentShaderInterlockFeaturesEXT* src);
    VkPhysicalDeviceFragmentShaderInterlockFeaturesEXT *ptr() { return reinterpret_cast<VkPhysicalDeviceFragmentShaderInterlockFeaturesEXT *>(this); }
    VkPhysicalDeviceFragmentShaderInterlockFeaturesEXT const *ptr() const { return reinterpret_cast<VkPhysicalDeviceFragmentShaderInterlockFeaturesEXT const *>(this); }
};

struct safe_VkPhysicalDeviceYcbcrImageArraysFeaturesEXT {
    VkStructureType sType;
    void* pNext;
    VkBool32 ycbcrImageArrays;
    safe_VkPhysicalDeviceYcbcrImageArraysFeaturesEXT(const VkPhysicalDeviceYcbcrImageArraysFeaturesEXT* in_struct);
    safe_VkPhysicalDeviceYcbcrImageArraysFeaturesEXT(const safe_VkPhysicalDeviceYcbcrImageArraysFeaturesEXT& src);
    safe_VkPhysicalDeviceYcbcrImageArraysFeaturesEXT& operator=(const safe_VkPhysicalDeviceYcbcrImageArraysFeaturesEXT& src);
    safe_VkPhysicalDeviceYcbcrImageArraysFeaturesEXT();
    ~safe_VkPhysicalDeviceYcbcrImageArraysFeaturesEXT();
    void initialize(const VkPhysicalDeviceYcbcrImageArraysFeaturesEXT* in_struct);
    void initialize(const safe_VkPhysicalDeviceYcbcrImageArraysFeaturesEXT* src);
    VkPhysicalDeviceYcbcrImageArraysFeaturesEXT *ptr() { return reinterpret_cast<VkPhysicalDeviceYcbcrImageArraysFeaturesEXT *>(this); }
    VkPhysicalDeviceYcbcrImageArraysFeaturesEXT const *ptr() const { return reinterpret_cast<VkPhysicalDeviceYcbcrImageArraysFeaturesEXT const *>(this); }
};

#ifdef VK_USE_PLATFORM_WIN32_KHR
struct safe_VkSurfaceFullScreenExclusiveInfoEXT {
    VkStructureType sType;
    void* pNext;
    VkFullScreenExclusiveEXT fullScreenExclusive;
    safe_VkSurfaceFullScreenExclusiveInfoEXT(const VkSurfaceFullScreenExclusiveInfoEXT* in_struct);
    safe_VkSurfaceFullScreenExclusiveInfoEXT(const safe_VkSurfaceFullScreenExclusiveInfoEXT& src);
    safe_VkSurfaceFullScreenExclusiveInfoEXT& operator=(const safe_VkSurfaceFullScreenExclusiveInfoEXT& src);
    safe_VkSurfaceFullScreenExclusiveInfoEXT();
    ~safe_VkSurfaceFullScreenExclusiveInfoEXT();
    void initialize(const VkSurfaceFullScreenExclusiveInfoEXT* in_struct);
    void initialize(const safe_VkSurfaceFullScreenExclusiveInfoEXT* src);
    VkSurfaceFullScreenExclusiveInfoEXT *ptr() { return reinterpret_cast<VkSurfaceFullScreenExclusiveInfoEXT *>(this); }
    VkSurfaceFullScreenExclusiveInfoEXT const *ptr() const { return reinterpret_cast<VkSurfaceFullScreenExclusiveInfoEXT const *>(this); }
};
#endif // VK_USE_PLATFORM_WIN32_KHR

#ifdef VK_USE_PLATFORM_WIN32_KHR
struct safe_VkSurfaceCapabilitiesFullScreenExclusiveEXT {
    VkStructureType sType;
    void* pNext;
    VkBool32 fullScreenExclusiveSupported;
    safe_VkSurfaceCapabilitiesFullScreenExclusiveEXT(const VkSurfaceCapabilitiesFullScreenExclusiveEXT* in_struct);
    safe_VkSurfaceCapabilitiesFullScreenExclusiveEXT(const safe_VkSurfaceCapabilitiesFullScreenExclusiveEXT& src);
    safe_VkSurfaceCapabilitiesFullScreenExclusiveEXT& operator=(const safe_VkSurfaceCapabilitiesFullScreenExclusiveEXT& src);
    safe_VkSurfaceCapabilitiesFullScreenExclusiveEXT();
    ~safe_VkSurfaceCapabilitiesFullScreenExclusiveEXT();
    void initialize(const VkSurfaceCapabilitiesFullScreenExclusiveEXT* in_struct);
    void initialize(const safe_VkSurfaceCapabilitiesFullScreenExclusiveEXT* src);
    VkSurfaceCapabilitiesFullScreenExclusiveEXT *ptr() { return reinterpret_cast<VkSurfaceCapabilitiesFullScreenExclusiveEXT *>(this); }
    VkSurfaceCapabilitiesFullScreenExclusiveEXT const *ptr() const { return reinterpret_cast<VkSurfaceCapabilitiesFullScreenExclusiveEXT const *>(this); }
};
#endif // VK_USE_PLATFORM_WIN32_KHR

#ifdef VK_USE_PLATFORM_WIN32_KHR
struct safe_VkSurfaceFullScreenExclusiveWin32InfoEXT {
    VkStructureType sType;
    const void* pNext;
    HMONITOR hmonitor;
    safe_VkSurfaceFullScreenExclusiveWin32InfoEXT(const VkSurfaceFullScreenExclusiveWin32InfoEXT* in_struct);
    safe_VkSurfaceFullScreenExclusiveWin32InfoEXT(const safe_VkSurfaceFullScreenExclusiveWin32InfoEXT& src);
    safe_VkSurfaceFullScreenExclusiveWin32InfoEXT& operator=(const safe_VkSurfaceFullScreenExclusiveWin32InfoEXT& src);
    safe_VkSurfaceFullScreenExclusiveWin32InfoEXT();
    ~safe_VkSurfaceFullScreenExclusiveWin32InfoEXT();
    void initialize(const VkSurfaceFullScreenExclusiveWin32InfoEXT* in_struct);
    void initialize(const safe_VkSurfaceFullScreenExclusiveWin32InfoEXT* src);
    VkSurfaceFullScreenExclusiveWin32InfoEXT *ptr() { return reinterpret_cast<VkSurfaceFullScreenExclusiveWin32InfoEXT *>(this); }
    VkSurfaceFullScreenExclusiveWin32InfoEXT const *ptr() const { return reinterpret_cast<VkSurfaceFullScreenExclusiveWin32InfoEXT const *>(this); }
};
#endif // VK_USE_PLATFORM_WIN32_KHR

struct safe_VkHeadlessSurfaceCreateInfoEXT {
    VkStructureType sType;
    const void* pNext;
    VkHeadlessSurfaceCreateFlagsEXT flags;
    safe_VkHeadlessSurfaceCreateInfoEXT(const VkHeadlessSurfaceCreateInfoEXT* in_struct);
    safe_VkHeadlessSurfaceCreateInfoEXT(const safe_VkHeadlessSurfaceCreateInfoEXT& src);
    safe_VkHeadlessSurfaceCreateInfoEXT& operator=(const safe_VkHeadlessSurfaceCreateInfoEXT& src);
    safe_VkHeadlessSurfaceCreateInfoEXT();
    ~safe_VkHeadlessSurfaceCreateInfoEXT();
    void initialize(const VkHeadlessSurfaceCreateInfoEXT* in_struct);
    void initialize(const safe_VkHeadlessSurfaceCreateInfoEXT* src);
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
    safe_VkPhysicalDeviceLineRasterizationFeaturesEXT(const safe_VkPhysicalDeviceLineRasterizationFeaturesEXT& src);
    safe_VkPhysicalDeviceLineRasterizationFeaturesEXT& operator=(const safe_VkPhysicalDeviceLineRasterizationFeaturesEXT& src);
    safe_VkPhysicalDeviceLineRasterizationFeaturesEXT();
    ~safe_VkPhysicalDeviceLineRasterizationFeaturesEXT();
    void initialize(const VkPhysicalDeviceLineRasterizationFeaturesEXT* in_struct);
    void initialize(const safe_VkPhysicalDeviceLineRasterizationFeaturesEXT* src);
    VkPhysicalDeviceLineRasterizationFeaturesEXT *ptr() { return reinterpret_cast<VkPhysicalDeviceLineRasterizationFeaturesEXT *>(this); }
    VkPhysicalDeviceLineRasterizationFeaturesEXT const *ptr() const { return reinterpret_cast<VkPhysicalDeviceLineRasterizationFeaturesEXT const *>(this); }
};

struct safe_VkPhysicalDeviceLineRasterizationPropertiesEXT {
    VkStructureType sType;
    void* pNext;
    uint32_t lineSubPixelPrecisionBits;
    safe_VkPhysicalDeviceLineRasterizationPropertiesEXT(const VkPhysicalDeviceLineRasterizationPropertiesEXT* in_struct);
    safe_VkPhysicalDeviceLineRasterizationPropertiesEXT(const safe_VkPhysicalDeviceLineRasterizationPropertiesEXT& src);
    safe_VkPhysicalDeviceLineRasterizationPropertiesEXT& operator=(const safe_VkPhysicalDeviceLineRasterizationPropertiesEXT& src);
    safe_VkPhysicalDeviceLineRasterizationPropertiesEXT();
    ~safe_VkPhysicalDeviceLineRasterizationPropertiesEXT();
    void initialize(const VkPhysicalDeviceLineRasterizationPropertiesEXT* in_struct);
    void initialize(const safe_VkPhysicalDeviceLineRasterizationPropertiesEXT* src);
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
    safe_VkPipelineRasterizationLineStateCreateInfoEXT(const safe_VkPipelineRasterizationLineStateCreateInfoEXT& src);
    safe_VkPipelineRasterizationLineStateCreateInfoEXT& operator=(const safe_VkPipelineRasterizationLineStateCreateInfoEXT& src);
    safe_VkPipelineRasterizationLineStateCreateInfoEXT();
    ~safe_VkPipelineRasterizationLineStateCreateInfoEXT();
    void initialize(const VkPipelineRasterizationLineStateCreateInfoEXT* in_struct);
    void initialize(const safe_VkPipelineRasterizationLineStateCreateInfoEXT* src);
    VkPipelineRasterizationLineStateCreateInfoEXT *ptr() { return reinterpret_cast<VkPipelineRasterizationLineStateCreateInfoEXT *>(this); }
    VkPipelineRasterizationLineStateCreateInfoEXT const *ptr() const { return reinterpret_cast<VkPipelineRasterizationLineStateCreateInfoEXT const *>(this); }
};

struct safe_VkPhysicalDeviceHostQueryResetFeaturesEXT {
    VkStructureType sType;
    void* pNext;
    VkBool32 hostQueryReset;
    safe_VkPhysicalDeviceHostQueryResetFeaturesEXT(const VkPhysicalDeviceHostQueryResetFeaturesEXT* in_struct);
    safe_VkPhysicalDeviceHostQueryResetFeaturesEXT(const safe_VkPhysicalDeviceHostQueryResetFeaturesEXT& src);
    safe_VkPhysicalDeviceHostQueryResetFeaturesEXT& operator=(const safe_VkPhysicalDeviceHostQueryResetFeaturesEXT& src);
    safe_VkPhysicalDeviceHostQueryResetFeaturesEXT();
    ~safe_VkPhysicalDeviceHostQueryResetFeaturesEXT();
    void initialize(const VkPhysicalDeviceHostQueryResetFeaturesEXT* in_struct);
    void initialize(const safe_VkPhysicalDeviceHostQueryResetFeaturesEXT* src);
    VkPhysicalDeviceHostQueryResetFeaturesEXT *ptr() { return reinterpret_cast<VkPhysicalDeviceHostQueryResetFeaturesEXT *>(this); }
    VkPhysicalDeviceHostQueryResetFeaturesEXT const *ptr() const { return reinterpret_cast<VkPhysicalDeviceHostQueryResetFeaturesEXT const *>(this); }
};

struct safe_VkPhysicalDeviceIndexTypeUint8FeaturesEXT {
    VkStructureType sType;
    void* pNext;
    VkBool32 indexTypeUint8;
    safe_VkPhysicalDeviceIndexTypeUint8FeaturesEXT(const VkPhysicalDeviceIndexTypeUint8FeaturesEXT* in_struct);
    safe_VkPhysicalDeviceIndexTypeUint8FeaturesEXT(const safe_VkPhysicalDeviceIndexTypeUint8FeaturesEXT& src);
    safe_VkPhysicalDeviceIndexTypeUint8FeaturesEXT& operator=(const safe_VkPhysicalDeviceIndexTypeUint8FeaturesEXT& src);
    safe_VkPhysicalDeviceIndexTypeUint8FeaturesEXT();
    ~safe_VkPhysicalDeviceIndexTypeUint8FeaturesEXT();
    void initialize(const VkPhysicalDeviceIndexTypeUint8FeaturesEXT* in_struct);
    void initialize(const safe_VkPhysicalDeviceIndexTypeUint8FeaturesEXT* src);
    VkPhysicalDeviceIndexTypeUint8FeaturesEXT *ptr() { return reinterpret_cast<VkPhysicalDeviceIndexTypeUint8FeaturesEXT *>(this); }
    VkPhysicalDeviceIndexTypeUint8FeaturesEXT const *ptr() const { return reinterpret_cast<VkPhysicalDeviceIndexTypeUint8FeaturesEXT const *>(this); }
};

struct safe_VkPhysicalDeviceShaderDemoteToHelperInvocationFeaturesEXT {
    VkStructureType sType;
    void* pNext;
    VkBool32 shaderDemoteToHelperInvocation;
    safe_VkPhysicalDeviceShaderDemoteToHelperInvocationFeaturesEXT(const VkPhysicalDeviceShaderDemoteToHelperInvocationFeaturesEXT* in_struct);
    safe_VkPhysicalDeviceShaderDemoteToHelperInvocationFeaturesEXT(const safe_VkPhysicalDeviceShaderDemoteToHelperInvocationFeaturesEXT& src);
    safe_VkPhysicalDeviceShaderDemoteToHelperInvocationFeaturesEXT& operator=(const safe_VkPhysicalDeviceShaderDemoteToHelperInvocationFeaturesEXT& src);
    safe_VkPhysicalDeviceShaderDemoteToHelperInvocationFeaturesEXT();
    ~safe_VkPhysicalDeviceShaderDemoteToHelperInvocationFeaturesEXT();
    void initialize(const VkPhysicalDeviceShaderDemoteToHelperInvocationFeaturesEXT* in_struct);
    void initialize(const safe_VkPhysicalDeviceShaderDemoteToHelperInvocationFeaturesEXT* src);
    VkPhysicalDeviceShaderDemoteToHelperInvocationFeaturesEXT *ptr() { return reinterpret_cast<VkPhysicalDeviceShaderDemoteToHelperInvocationFeaturesEXT *>(this); }
    VkPhysicalDeviceShaderDemoteToHelperInvocationFeaturesEXT const *ptr() const { return reinterpret_cast<VkPhysicalDeviceShaderDemoteToHelperInvocationFeaturesEXT const *>(this); }
};

struct safe_VkPhysicalDeviceTexelBufferAlignmentFeaturesEXT {
    VkStructureType sType;
    void* pNext;
    VkBool32 texelBufferAlignment;
    safe_VkPhysicalDeviceTexelBufferAlignmentFeaturesEXT(const VkPhysicalDeviceTexelBufferAlignmentFeaturesEXT* in_struct);
    safe_VkPhysicalDeviceTexelBufferAlignmentFeaturesEXT(const safe_VkPhysicalDeviceTexelBufferAlignmentFeaturesEXT& src);
    safe_VkPhysicalDeviceTexelBufferAlignmentFeaturesEXT& operator=(const safe_VkPhysicalDeviceTexelBufferAlignmentFeaturesEXT& src);
    safe_VkPhysicalDeviceTexelBufferAlignmentFeaturesEXT();
    ~safe_VkPhysicalDeviceTexelBufferAlignmentFeaturesEXT();
    void initialize(const VkPhysicalDeviceTexelBufferAlignmentFeaturesEXT* in_struct);
    void initialize(const safe_VkPhysicalDeviceTexelBufferAlignmentFeaturesEXT* src);
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
    safe_VkPhysicalDeviceTexelBufferAlignmentPropertiesEXT(const safe_VkPhysicalDeviceTexelBufferAlignmentPropertiesEXT& src);
    safe_VkPhysicalDeviceTexelBufferAlignmentPropertiesEXT& operator=(const safe_VkPhysicalDeviceTexelBufferAlignmentPropertiesEXT& src);
    safe_VkPhysicalDeviceTexelBufferAlignmentPropertiesEXT();
    ~safe_VkPhysicalDeviceTexelBufferAlignmentPropertiesEXT();
    void initialize(const VkPhysicalDeviceTexelBufferAlignmentPropertiesEXT* in_struct);
    void initialize(const safe_VkPhysicalDeviceTexelBufferAlignmentPropertiesEXT* src);
    VkPhysicalDeviceTexelBufferAlignmentPropertiesEXT *ptr() { return reinterpret_cast<VkPhysicalDeviceTexelBufferAlignmentPropertiesEXT *>(this); }
    VkPhysicalDeviceTexelBufferAlignmentPropertiesEXT const *ptr() const { return reinterpret_cast<VkPhysicalDeviceTexelBufferAlignmentPropertiesEXT const *>(this); }
};
