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
 * Author: Courtney Goeltzenleuchter <courtneygo@google.com>
 * Author: Tobin Ehlis <tobine@google.com>
 * Author: Chris Forbes <chrisf@ijw.co.nz>
 * Author: Mark Lobodzinski <mark@lunarg.com>
 * Author: Dave Houlton <daveh@lunarg.com>
 */

#pragma once
#include "core_validation_error_enums.h"
#include "core_validation_types.h"
#include "descriptor_sets.h"
#include "vk_layer_logging.h"
#include "vulkan/vk_layer.h"
#include "vk_typemap_helper.h"
#include "gpu_validation.h"
#include <atomic>
#include <functional>
#include <memory>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <list>
#include <deque>

/*
 * MTMTODO : Update this comment
 * Data Structure overview
 *  There are 4 global STL(' maps
 *  cbMap -- map of command Buffer (CB) objects to MT_CB_INFO structures
 *    Each MT_CB_INFO struct has an stl list container with
 *    memory objects that are referenced by this CB
 *  memObjMap -- map of Memory Objects to MT_MEM_OBJ_INFO structures
 *    Each MT_MEM_OBJ_INFO has two stl list containers with:
 *      -- all CBs referencing this mem obj
 *      -- all VK Objects that are bound to this memory
 *  objectMap -- map of objects to MT_OBJ_INFO structures
 *
 * Algorithm overview
 * These are the primary events that should happen related to different objects
 * 1. Command buffers
 *   CREATION - Add object,structure to map
 *   CMD BIND - If mem associated, add mem reference to list container
 *   DESTROY  - Remove from map, decrement (and report) mem references
 * 2. Mem Objects
 *   CREATION - Add object,structure to map
 *   OBJ BIND - Add obj structure to list container for that mem node
 *   CMB BIND - If mem-related add CB structure to list container for that mem node
 *   DESTROY  - Flag as errors any remaining refs and remove from map
 * 3. Generic Objects
 *   MEM BIND - DESTROY any previous binding, Add obj node w/ ref to map, add obj ref to list container for that mem node
 *   DESTROY - If mem bound, remove reference list container for that memInfo, remove object ref from map
 */
// TODO : Is there a way to track when Cmd Buffer finishes & remove mem references at that point?
// TODO : Could potentially store a list of freed mem allocs to flag when they're incorrectly used

enum SyncScope {
    kSyncScopeInternal,
    kSyncScopeExternalTemporary,
    kSyncScopeExternalPermanent,
};

enum FENCE_STATE { FENCE_UNSIGNALED, FENCE_INFLIGHT, FENCE_RETIRED };

class FENCE_NODE {
   public:
    VkFence fence;
    VkFenceCreateInfo createInfo;
    std::pair<VkQueue, uint64_t> signaler;
    FENCE_STATE state;
    SyncScope scope;

    // Default constructor
    FENCE_NODE() : state(FENCE_UNSIGNALED), scope(kSyncScopeInternal) {}
};

class SEMAPHORE_NODE : public BASE_NODE {
   public:
    std::pair<VkQueue, uint64_t> signaler;
    bool signaled;
    SyncScope scope;
};

class EVENT_STATE : public BASE_NODE {
   public:
    int write_in_use;
    bool needsSignaled;
    VkPipelineStageFlags stageMask;
};

class QUEUE_STATE {
   public:
    VkQueue queue;
    uint32_t queueFamilyIndex;
    std::unordered_map<VkEvent, VkPipelineStageFlags> eventToStageMap;
    std::unordered_map<QueryObject, bool> queryToStateMap;  // 0 is unavailable, 1 is available

    uint64_t seq;
    std::deque<CB_SUBMISSION> submissions;
};

class QUERY_POOL_NODE : public BASE_NODE {
   public:
    VkQueryPoolCreateInfo createInfo;
};

struct PHYSICAL_DEVICE_STATE {
    // Track the call state and array sizes for various query functions
    CALL_STATE vkGetPhysicalDeviceQueueFamilyPropertiesState = UNCALLED;
    CALL_STATE vkGetPhysicalDeviceLayerPropertiesState = UNCALLED;
    CALL_STATE vkGetPhysicalDeviceExtensionPropertiesState = UNCALLED;
    CALL_STATE vkGetPhysicalDeviceFeaturesState = UNCALLED;
    CALL_STATE vkGetPhysicalDeviceSurfaceCapabilitiesKHRState = UNCALLED;
    CALL_STATE vkGetPhysicalDeviceSurfacePresentModesKHRState = UNCALLED;
    CALL_STATE vkGetPhysicalDeviceSurfaceFormatsKHRState = UNCALLED;
    CALL_STATE vkGetPhysicalDeviceDisplayPlanePropertiesKHRState = UNCALLED;
    safe_VkPhysicalDeviceFeatures2 features2 = {};
    VkPhysicalDevice phys_device = VK_NULL_HANDLE;
    uint32_t queue_family_count = 0;
    std::vector<VkQueueFamilyProperties> queue_family_properties;
    VkSurfaceCapabilitiesKHR surfaceCapabilities = {};
    std::vector<VkPresentModeKHR> present_modes;
    std::vector<VkSurfaceFormatKHR> surface_formats;
    uint32_t display_plane_property_count = 0;
};

struct GpuQueue {
    VkPhysicalDevice gpu;
    uint32_t queue_family_index;
};

inline bool operator==(GpuQueue const& lhs, GpuQueue const& rhs) {
    return (lhs.gpu == rhs.gpu && lhs.queue_family_index == rhs.queue_family_index);
}

namespace std {
template <>
struct hash<GpuQueue> {
    size_t operator()(GpuQueue gq) const throw() {
        return hash<uint64_t>()((uint64_t)(gq.gpu)) ^ hash<uint32_t>()(gq.queue_family_index);
    }
};
}  // namespace std

struct SURFACE_STATE {
    VkSurfaceKHR surface = VK_NULL_HANDLE;
    SWAPCHAIN_NODE* swapchain = nullptr;
    SWAPCHAIN_NODE* old_swapchain = nullptr;
    std::unordered_map<GpuQueue, bool> gpu_queue_support;

    SURFACE_STATE() {}
    SURFACE_STATE(VkSurfaceKHR surface) : surface(surface) {}
};

using std::unordered_map;

namespace core_validation {

struct instance_layer_data {
    VkInstance instance = VK_NULL_HANDLE;
    debug_report_data* report_data = nullptr;
    std::vector<VkDebugReportCallbackEXT> logging_callback;
    std::vector<VkDebugUtilsMessengerEXT> logging_messenger;
    VkLayerInstanceDispatchTable dispatch_table;

    CALL_STATE vkEnumeratePhysicalDevicesState = UNCALLED;
    uint32_t physical_devices_count = 0;
    CALL_STATE vkEnumeratePhysicalDeviceGroupsState = UNCALLED;
    uint32_t physical_device_groups_count = 0;
    CHECK_DISABLED disabled = {};
    CHECK_ENABLED enabled = {};

    unordered_map<VkPhysicalDevice, PHYSICAL_DEVICE_STATE> physical_device_map;
    unordered_map<VkSurfaceKHR, SURFACE_STATE> surface_map;

    InstanceExtensions extensions;
    uint32_t api_version;
};

struct layer_data {
    debug_report_data* report_data = nullptr;
    VkLayerDispatchTable dispatch_table;

    DeviceExtensions extensions = {};
    std::unordered_set<VkQueue> queues;  // All queues under given device
    // Layer specific data
    unordered_map<VkSampler, std::unique_ptr<SAMPLER_STATE>> samplerMap;
    unordered_map<VkImageView, std::unique_ptr<IMAGE_VIEW_STATE>> imageViewMap;
    unordered_map<VkImage, std::unique_ptr<IMAGE_STATE>> imageMap;
    unordered_map<VkBufferView, std::unique_ptr<BUFFER_VIEW_STATE>> bufferViewMap;
    unordered_map<VkBuffer, std::unique_ptr<BUFFER_STATE>> bufferMap;
    unordered_map<VkPipeline, std::unique_ptr<PIPELINE_STATE>> pipelineMap;
    unordered_map<VkCommandPool, COMMAND_POOL_NODE> commandPoolMap;
    unordered_map<VkDescriptorPool, DESCRIPTOR_POOL_STATE*> descriptorPoolMap;
    unordered_map<VkDescriptorSet, cvdescriptorset::DescriptorSet*> setMap;
    unordered_map<VkDescriptorSetLayout, std::shared_ptr<cvdescriptorset::DescriptorSetLayout>> descriptorSetLayoutMap;
    unordered_map<VkPipelineLayout, PIPELINE_LAYOUT_NODE> pipelineLayoutMap;
    unordered_map<VkDeviceMemory, std::unique_ptr<DEVICE_MEM_INFO>> memObjMap;
    unordered_map<VkFence, FENCE_NODE> fenceMap;
    unordered_map<VkQueue, QUEUE_STATE> queueMap;
    unordered_map<VkEvent, EVENT_STATE> eventMap;
    unordered_map<QueryObject, bool> queryToStateMap;
    unordered_map<VkQueryPool, QUERY_POOL_NODE> queryPoolMap;
    unordered_map<VkSemaphore, SEMAPHORE_NODE> semaphoreMap;
    unordered_map<VkCommandBuffer, GLOBAL_CB_NODE*> commandBufferMap;
    unordered_map<VkFramebuffer, std::unique_ptr<FRAMEBUFFER_STATE>> frameBufferMap;
    unordered_map<VkImage, std::vector<ImageSubresourcePair>> imageSubresourceMap;
    unordered_map<ImageSubresourcePair, IMAGE_LAYOUT_NODE> imageLayoutMap;
    unordered_map<VkRenderPass, std::shared_ptr<RENDER_PASS_STATE>> renderPassMap;
    unordered_map<VkShaderModule, std::unique_ptr<shader_module>> shaderModuleMap;
    unordered_map<VkDescriptorUpdateTemplateKHR, std::unique_ptr<TEMPLATE_STATE>> desc_template_map;
    unordered_map<VkSwapchainKHR, std::unique_ptr<SWAPCHAIN_NODE>> swapchainMap;
    unordered_map<VkSamplerYcbcrConversion, uint64_t> ycbcr_conversion_ahb_fmt_map;
    std::unordered_set<uint64_t> ahb_ext_formats_set;
    GlobalQFOTransferBarrierMap<VkImageMemoryBarrier> qfo_release_image_barrier_map;
    GlobalQFOTransferBarrierMap<VkBufferMemoryBarrier> qfo_release_buffer_barrier_map;

    VkDevice device = VK_NULL_HANDLE;
    VkPhysicalDevice physical_device = VK_NULL_HANDLE;

    instance_layer_data* instance_data = nullptr;  // from device to enclosing instance

    DeviceFeatures enabled_features = {};
    // Device specific data
    PHYS_DEV_PROPERTIES_NODE phys_dev_properties = {};
    VkPhysicalDeviceMemoryProperties phys_dev_mem_props = {};
    VkPhysicalDeviceProperties phys_dev_props = {};
    // Device extension properties -- storing properties gathered from VkPhysicalDeviceProperties2KHR::pNext chain
    struct DeviceExtensionProperties {
        uint32_t max_push_descriptors;  // from VkPhysicalDevicePushDescriptorPropertiesKHR::maxPushDescriptors
        VkPhysicalDeviceDescriptorIndexingPropertiesEXT descriptor_indexing_props;
        VkPhysicalDeviceShadingRateImagePropertiesNV shading_rate_image_props;
        VkPhysicalDeviceMeshShaderPropertiesNV mesh_shader_props;
        VkPhysicalDeviceInlineUniformBlockPropertiesEXT inline_uniform_block_props;
        VkPhysicalDeviceVertexAttributeDivisorPropertiesEXT vtx_attrib_divisor_props;
        VkPhysicalDeviceDepthStencilResolvePropertiesKHR depth_stencil_resolve_props;
    };
    DeviceExtensionProperties phys_dev_ext_props = {};
    bool external_sync_warning = false;
    uint32_t api_version = 0;
    GpuValidationState gpu_validation_state = {};
    uint32_t physical_device_count;
};

VKAPI_ATTR VkResult VKAPI_CALL CreateInstance(const VkInstanceCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator,
                                              VkInstance* pInstance);

VKAPI_ATTR void VKAPI_CALL DestroyInstance(VkInstance instance, const VkAllocationCallbacks* pAllocator);

VKAPI_ATTR VkResult VKAPI_CALL EnumeratePhysicalDevices(VkInstance instance, uint32_t* pPhysicalDeviceCount,
                                                        VkPhysicalDevice* pPhysicalDevices);

VKAPI_ATTR void VKAPI_CALL GetPhysicalDeviceFeatures(VkPhysicalDevice physicalDevice, VkPhysicalDeviceFeatures* pFeatures);

VKAPI_ATTR void VKAPI_CALL GetPhysicalDeviceFormatProperties(VkPhysicalDevice physicalDevice, VkFormat format,
                                                             VkFormatProperties* pFormatProperties);

VKAPI_ATTR VkResult VKAPI_CALL GetPhysicalDeviceImageFormatProperties(VkPhysicalDevice physicalDevice, VkFormat format,
                                                                      VkImageType type, VkImageTiling tiling,
                                                                      VkImageUsageFlags usage, VkImageCreateFlags flags,
                                                                      VkImageFormatProperties* pImageFormatProperties);

VKAPI_ATTR void VKAPI_CALL GetPhysicalDeviceProperties(VkPhysicalDevice physicalDevice, VkPhysicalDeviceProperties* pProperties);

VKAPI_ATTR void VKAPI_CALL GetPhysicalDeviceQueueFamilyProperties(VkPhysicalDevice physicalDevice,
                                                                  uint32_t* pQueueFamilyPropertyCount,
                                                                  VkQueueFamilyProperties* pQueueFamilyProperties);

VKAPI_ATTR void VKAPI_CALL GetPhysicalDeviceMemoryProperties(VkPhysicalDevice physicalDevice,
                                                             VkPhysicalDeviceMemoryProperties* pMemoryProperties);

VKAPI_ATTR PFN_vkVoidFunction VKAPI_CALL GetInstanceProcAddr(VkInstance instance, const char* pName);

VKAPI_ATTR PFN_vkVoidFunction VKAPI_CALL GetDeviceProcAddr(VkDevice device, const char* pName);

VKAPI_ATTR VkResult VKAPI_CALL CreateDevice(VkPhysicalDevice physicalDevice, const VkDeviceCreateInfo* pCreateInfo,
                                            const VkAllocationCallbacks* pAllocator, VkDevice* pDevice);

VKAPI_ATTR void VKAPI_CALL DestroyDevice(VkDevice device, const VkAllocationCallbacks* pAllocator);

VKAPI_ATTR VkResult VKAPI_CALL EnumerateInstanceExtensionProperties(const char* pLayerName, uint32_t* pPropertyCount,
                                                                    VkExtensionProperties* pProperties);

VKAPI_ATTR VkResult VKAPI_CALL EnumerateDeviceExtensionProperties(VkPhysicalDevice physicalDevice, const char* pLayerName,
                                                                  uint32_t* pPropertyCount, VkExtensionProperties* pProperties);

VKAPI_ATTR VkResult VKAPI_CALL EnumerateInstanceLayerProperties(uint32_t* pPropertyCount, VkLayerProperties* pProperties);

VKAPI_ATTR VkResult VKAPI_CALL EnumerateDeviceLayerProperties(VkPhysicalDevice physicalDevice, uint32_t* pPropertyCount,
                                                              VkLayerProperties* pProperties);

VKAPI_ATTR void VKAPI_CALL GetDeviceQueue(VkDevice device, uint32_t queueFamilyIndex, uint32_t queueIndex, VkQueue* pQueue);

VKAPI_ATTR VkResult VKAPI_CALL QueueSubmit(VkQueue queue, uint32_t submitCount, const VkSubmitInfo* pSubmits, VkFence fence);

VKAPI_ATTR VkResult VKAPI_CALL QueueWaitIdle(VkQueue queue);

VKAPI_ATTR VkResult VKAPI_CALL DeviceWaitIdle(VkDevice device);

VKAPI_ATTR VkResult VKAPI_CALL AllocateMemory(VkDevice device, const VkMemoryAllocateInfo* pAllocateInfo,
                                              const VkAllocationCallbacks* pAllocator, VkDeviceMemory* pMemory);

VKAPI_ATTR void VKAPI_CALL FreeMemory(VkDevice device, VkDeviceMemory memory, const VkAllocationCallbacks* pAllocator);

VKAPI_ATTR VkResult VKAPI_CALL MapMemory(VkDevice device, VkDeviceMemory memory, VkDeviceSize offset, VkDeviceSize size,
                                         VkMemoryMapFlags flags, void** ppData);

VKAPI_ATTR void VKAPI_CALL UnmapMemory(VkDevice device, VkDeviceMemory memory);

VKAPI_ATTR VkResult VKAPI_CALL FlushMappedMemoryRanges(VkDevice device, uint32_t memoryRangeCount,
                                                       const VkMappedMemoryRange* pMemoryRanges);

VKAPI_ATTR VkResult VKAPI_CALL InvalidateMappedMemoryRanges(VkDevice device, uint32_t memoryRangeCount,
                                                            const VkMappedMemoryRange* pMemoryRanges);

VKAPI_ATTR void VKAPI_CALL GetDeviceMemoryCommitment(VkDevice device, VkDeviceMemory memory, VkDeviceSize* pCommittedMemoryInBytes);

VKAPI_ATTR VkResult VKAPI_CALL BindBufferMemory(VkDevice device, VkBuffer buffer, VkDeviceMemory memory, VkDeviceSize memoryOffset);

VKAPI_ATTR VkResult VKAPI_CALL BindImageMemory(VkDevice device, VkImage image, VkDeviceMemory memory, VkDeviceSize memoryOffset);

VKAPI_ATTR void VKAPI_CALL GetBufferMemoryRequirements(VkDevice device, VkBuffer buffer, VkMemoryRequirements* pMemoryRequirements);

VKAPI_ATTR void VKAPI_CALL GetImageMemoryRequirements(VkDevice device, VkImage image, VkMemoryRequirements* pMemoryRequirements);

VKAPI_ATTR void VKAPI_CALL GetImageSparseMemoryRequirements(VkDevice device, VkImage image, uint32_t* pSparseMemoryRequirementCount,
                                                            VkSparseImageMemoryRequirements* pSparseMemoryRequirements);

VKAPI_ATTR void VKAPI_CALL GetPhysicalDeviceSparseImageFormatProperties(VkPhysicalDevice physicalDevice, VkFormat format,
                                                                        VkImageType type, VkSampleCountFlagBits samples,
                                                                        VkImageUsageFlags usage, VkImageTiling tiling,
                                                                        uint32_t* pPropertyCount,
                                                                        VkSparseImageFormatProperties* pProperties);

VKAPI_ATTR VkResult VKAPI_CALL QueueBindSparse(VkQueue queue, uint32_t bindInfoCount, const VkBindSparseInfo* pBindInfo,
                                               VkFence fence);

VKAPI_ATTR VkResult VKAPI_CALL CreateFence(VkDevice device, const VkFenceCreateInfo* pCreateInfo,
                                           const VkAllocationCallbacks* pAllocator, VkFence* pFence);

VKAPI_ATTR void VKAPI_CALL DestroyFence(VkDevice device, VkFence fence, const VkAllocationCallbacks* pAllocator);

VKAPI_ATTR VkResult VKAPI_CALL ResetFences(VkDevice device, uint32_t fenceCount, const VkFence* pFences);

VKAPI_ATTR VkResult VKAPI_CALL GetFenceStatus(VkDevice device, VkFence fence);

VKAPI_ATTR VkResult VKAPI_CALL WaitForFences(VkDevice device, uint32_t fenceCount, const VkFence* pFences, VkBool32 waitAll,
                                             uint64_t timeout);

VKAPI_ATTR VkResult VKAPI_CALL CreateSemaphore(VkDevice device, const VkSemaphoreCreateInfo* pCreateInfo,
                                               const VkAllocationCallbacks* pAllocator, VkSemaphore* pSemaphore);

VKAPI_ATTR void VKAPI_CALL DestroySemaphore(VkDevice device, VkSemaphore semaphore, const VkAllocationCallbacks* pAllocator);

VKAPI_ATTR VkResult VKAPI_CALL CreateEvent(VkDevice device, const VkEventCreateInfo* pCreateInfo,
                                           const VkAllocationCallbacks* pAllocator, VkEvent* pEvent);

VKAPI_ATTR void VKAPI_CALL DestroyEvent(VkDevice device, VkEvent event, const VkAllocationCallbacks* pAllocator);

VKAPI_ATTR VkResult VKAPI_CALL GetEventStatus(VkDevice device, VkEvent event);

VKAPI_ATTR VkResult VKAPI_CALL SetEvent(VkDevice device, VkEvent event);

VKAPI_ATTR VkResult VKAPI_CALL ResetEvent(VkDevice device, VkEvent event);

VKAPI_ATTR VkResult VKAPI_CALL CreateQueryPool(VkDevice device, const VkQueryPoolCreateInfo* pCreateInfo,
                                               const VkAllocationCallbacks* pAllocator, VkQueryPool* pQueryPool);

VKAPI_ATTR void VKAPI_CALL DestroyQueryPool(VkDevice device, VkQueryPool queryPool, const VkAllocationCallbacks* pAllocator);

VKAPI_ATTR VkResult VKAPI_CALL GetQueryPoolResults(VkDevice device, VkQueryPool queryPool, uint32_t firstQuery, uint32_t queryCount,
                                                   size_t dataSize, void* pData, VkDeviceSize stride, VkQueryResultFlags flags);

VKAPI_ATTR VkResult VKAPI_CALL CreateBuffer(VkDevice device, const VkBufferCreateInfo* pCreateInfo,
                                            const VkAllocationCallbacks* pAllocator, VkBuffer* pBuffer);

VKAPI_ATTR void VKAPI_CALL DestroyBuffer(VkDevice device, VkBuffer buffer, const VkAllocationCallbacks* pAllocator);

VKAPI_ATTR VkResult VKAPI_CALL CreateBufferView(VkDevice device, const VkBufferViewCreateInfo* pCreateInfo,
                                                const VkAllocationCallbacks* pAllocator, VkBufferView* pView);

VKAPI_ATTR void VKAPI_CALL DestroyBufferView(VkDevice device, VkBufferView bufferView, const VkAllocationCallbacks* pAllocator);

VKAPI_ATTR VkResult VKAPI_CALL CreateImage(VkDevice device, const VkImageCreateInfo* pCreateInfo,
                                           const VkAllocationCallbacks* pAllocator, VkImage* pImage);

VKAPI_ATTR void VKAPI_CALL DestroyImage(VkDevice device, VkImage image, const VkAllocationCallbacks* pAllocator);

VKAPI_ATTR void VKAPI_CALL GetImageSubresourceLayout(VkDevice device, VkImage image, const VkImageSubresource* pSubresource,
                                                     VkSubresourceLayout* pLayout);

VKAPI_ATTR VkResult VKAPI_CALL CreateImageView(VkDevice device, const VkImageViewCreateInfo* pCreateInfo,
                                               const VkAllocationCallbacks* pAllocator, VkImageView* pView);

VKAPI_ATTR void VKAPI_CALL DestroyImageView(VkDevice device, VkImageView imageView, const VkAllocationCallbacks* pAllocator);

VKAPI_ATTR VkResult VKAPI_CALL CreateShaderModule(VkDevice device, const VkShaderModuleCreateInfo* pCreateInfo,
                                                  const VkAllocationCallbacks* pAllocator, VkShaderModule* pShaderModule);

VKAPI_ATTR void VKAPI_CALL DestroyShaderModule(VkDevice device, VkShaderModule shaderModule,
                                               const VkAllocationCallbacks* pAllocator);

VKAPI_ATTR VkResult VKAPI_CALL CreatePipelineCache(VkDevice device, const VkPipelineCacheCreateInfo* pCreateInfo,
                                                   const VkAllocationCallbacks* pAllocator, VkPipelineCache* pPipelineCache);

VKAPI_ATTR void VKAPI_CALL DestroyPipelineCache(VkDevice device, VkPipelineCache pipelineCache,
                                                const VkAllocationCallbacks* pAllocator);

VKAPI_ATTR VkResult VKAPI_CALL GetPipelineCacheData(VkDevice device, VkPipelineCache pipelineCache, size_t* pDataSize, void* pData);

VKAPI_ATTR VkResult VKAPI_CALL MergePipelineCaches(VkDevice device, VkPipelineCache dstCache, uint32_t srcCacheCount,
                                                   const VkPipelineCache* pSrcCaches);

VKAPI_ATTR VkResult VKAPI_CALL CreateGraphicsPipelines(VkDevice device, VkPipelineCache pipelineCache, uint32_t createInfoCount,
                                                       const VkGraphicsPipelineCreateInfo* pCreateInfos,
                                                       const VkAllocationCallbacks* pAllocator, VkPipeline* pPipelines);

VKAPI_ATTR VkResult VKAPI_CALL CreateComputePipelines(VkDevice device, VkPipelineCache pipelineCache, uint32_t createInfoCount,
                                                      const VkComputePipelineCreateInfo* pCreateInfos,
                                                      const VkAllocationCallbacks* pAllocator, VkPipeline* pPipelines);

VKAPI_ATTR void VKAPI_CALL DestroyPipeline(VkDevice device, VkPipeline pipeline, const VkAllocationCallbacks* pAllocator);

VKAPI_ATTR VkResult VKAPI_CALL CreatePipelineLayout(VkDevice device, const VkPipelineLayoutCreateInfo* pCreateInfo,
                                                    const VkAllocationCallbacks* pAllocator, VkPipelineLayout* pPipelineLayout);

VKAPI_ATTR void VKAPI_CALL DestroyPipelineLayout(VkDevice device, VkPipelineLayout pipelineLayout,
                                                 const VkAllocationCallbacks* pAllocator);

VKAPI_ATTR VkResult VKAPI_CALL CreateSampler(VkDevice device, const VkSamplerCreateInfo* pCreateInfo,
                                             const VkAllocationCallbacks* pAllocator, VkSampler* pSampler);

VKAPI_ATTR void VKAPI_CALL DestroySampler(VkDevice device, VkSampler sampler, const VkAllocationCallbacks* pAllocator);

VKAPI_ATTR VkResult VKAPI_CALL CreateDescriptorSetLayout(VkDevice device, const VkDescriptorSetLayoutCreateInfo* pCreateInfo,
                                                         const VkAllocationCallbacks* pAllocator,
                                                         VkDescriptorSetLayout* pSetLayout);

VKAPI_ATTR void VKAPI_CALL DestroyDescriptorSetLayout(VkDevice device, VkDescriptorSetLayout descriptorSetLayout,
                                                      const VkAllocationCallbacks* pAllocator);

VKAPI_ATTR VkResult VKAPI_CALL CreateDescriptorPool(VkDevice device, const VkDescriptorPoolCreateInfo* pCreateInfo,
                                                    const VkAllocationCallbacks* pAllocator, VkDescriptorPool* pDescriptorPool);

VKAPI_ATTR void VKAPI_CALL DestroyDescriptorPool(VkDevice device, VkDescriptorPool descriptorPool,
                                                 const VkAllocationCallbacks* pAllocator);

VKAPI_ATTR VkResult VKAPI_CALL ResetDescriptorPool(VkDevice device, VkDescriptorPool descriptorPool,
                                                   VkDescriptorPoolResetFlags flags);

VKAPI_ATTR VkResult VKAPI_CALL AllocateDescriptorSets(VkDevice device, const VkDescriptorSetAllocateInfo* pAllocateInfo,
                                                      VkDescriptorSet* pDescriptorSets);

VKAPI_ATTR VkResult VKAPI_CALL FreeDescriptorSets(VkDevice device, VkDescriptorPool descriptorPool, uint32_t descriptorSetCount,
                                                  const VkDescriptorSet* pDescriptorSets);

VKAPI_ATTR void VKAPI_CALL UpdateDescriptorSets(VkDevice device, uint32_t descriptorWriteCount,
                                                const VkWriteDescriptorSet* pDescriptorWrites, uint32_t descriptorCopyCount,
                                                const VkCopyDescriptorSet* pDescriptorCopies);

VKAPI_ATTR VkResult VKAPI_CALL CreateFramebuffer(VkDevice device, const VkFramebufferCreateInfo* pCreateInfo,
                                                 const VkAllocationCallbacks* pAllocator, VkFramebuffer* pFramebuffer);

VKAPI_ATTR void VKAPI_CALL DestroyFramebuffer(VkDevice device, VkFramebuffer framebuffer, const VkAllocationCallbacks* pAllocator);

VKAPI_ATTR VkResult VKAPI_CALL CreateRenderPass(VkDevice device, const VkRenderPassCreateInfo* pCreateInfo,
                                                const VkAllocationCallbacks* pAllocator, VkRenderPass* pRenderPass);

VKAPI_ATTR void VKAPI_CALL DestroyRenderPass(VkDevice device, VkRenderPass renderPass, const VkAllocationCallbacks* pAllocator);

VKAPI_ATTR void VKAPI_CALL GetRenderAreaGranularity(VkDevice device, VkRenderPass renderPass, VkExtent2D* pGranularity);

VKAPI_ATTR VkResult VKAPI_CALL CreateCommandPool(VkDevice device, const VkCommandPoolCreateInfo* pCreateInfo,
                                                 const VkAllocationCallbacks* pAllocator, VkCommandPool* pCommandPool);

VKAPI_ATTR void VKAPI_CALL DestroyCommandPool(VkDevice device, VkCommandPool commandPool, const VkAllocationCallbacks* pAllocator);

VKAPI_ATTR VkResult VKAPI_CALL ResetCommandPool(VkDevice device, VkCommandPool commandPool, VkCommandPoolResetFlags flags);

VKAPI_ATTR VkResult VKAPI_CALL AllocateCommandBuffers(VkDevice device, const VkCommandBufferAllocateInfo* pAllocateInfo,
                                                      VkCommandBuffer* pCommandBuffers);

VKAPI_ATTR void VKAPI_CALL FreeCommandBuffers(VkDevice device, VkCommandPool commandPool, uint32_t commandBufferCount,
                                              const VkCommandBuffer* pCommandBuffers);

VKAPI_ATTR VkResult VKAPI_CALL BeginCommandBuffer(VkCommandBuffer commandBuffer, const VkCommandBufferBeginInfo* pBeginInfo);

VKAPI_ATTR VkResult VKAPI_CALL EndCommandBuffer(VkCommandBuffer commandBuffer);

VKAPI_ATTR VkResult VKAPI_CALL ResetCommandBuffer(VkCommandBuffer commandBuffer, VkCommandBufferResetFlags flags);

VKAPI_ATTR void VKAPI_CALL CmdBindPipeline(VkCommandBuffer commandBuffer, VkPipelineBindPoint pipelineBindPoint,
                                           VkPipeline pipeline);

VKAPI_ATTR void VKAPI_CALL CmdSetViewport(VkCommandBuffer commandBuffer, uint32_t firstViewport, uint32_t viewportCount,
                                          const VkViewport* pViewports);

VKAPI_ATTR void VKAPI_CALL CmdSetScissor(VkCommandBuffer commandBuffer, uint32_t firstScissor, uint32_t scissorCount,
                                         const VkRect2D* pScissors);

VKAPI_ATTR void VKAPI_CALL CmdSetLineWidth(VkCommandBuffer commandBuffer, float lineWidth);

VKAPI_ATTR void VKAPI_CALL CmdSetDepthBias(VkCommandBuffer commandBuffer, float depthBiasConstantFactor, float depthBiasClamp,
                                           float depthBiasSlopeFactor);

VKAPI_ATTR void VKAPI_CALL CmdSetBlendConstants(VkCommandBuffer commandBuffer, const float blendConstants[4]);

VKAPI_ATTR void VKAPI_CALL CmdSetDepthBounds(VkCommandBuffer commandBuffer, float minDepthBounds, float maxDepthBounds);

VKAPI_ATTR void VKAPI_CALL CmdSetStencilCompareMask(VkCommandBuffer commandBuffer, VkStencilFaceFlags faceMask,
                                                    uint32_t compareMask);

VKAPI_ATTR void VKAPI_CALL CmdSetStencilWriteMask(VkCommandBuffer commandBuffer, VkStencilFaceFlags faceMask, uint32_t writeMask);

VKAPI_ATTR void VKAPI_CALL CmdSetStencilReference(VkCommandBuffer commandBuffer, VkStencilFaceFlags faceMask, uint32_t reference);

VKAPI_ATTR void VKAPI_CALL CmdBindDescriptorSets(VkCommandBuffer commandBuffer, VkPipelineBindPoint pipelineBindPoint,
                                                 VkPipelineLayout layout, uint32_t firstSet, uint32_t descriptorSetCount,
                                                 const VkDescriptorSet* pDescriptorSets, uint32_t dynamicOffsetCount,
                                                 const uint32_t* pDynamicOffsets);

VKAPI_ATTR void VKAPI_CALL CmdBindIndexBuffer(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                              VkIndexType indexType);

VKAPI_ATTR void VKAPI_CALL CmdBindVertexBuffers(VkCommandBuffer commandBuffer, uint32_t firstBinding, uint32_t bindingCount,
                                                const VkBuffer* pBuffers, const VkDeviceSize* pOffsets);

VKAPI_ATTR void VKAPI_CALL CmdDraw(VkCommandBuffer commandBuffer, uint32_t vertexCount, uint32_t instanceCount,
                                   uint32_t firstVertex, uint32_t firstInstance);

VKAPI_ATTR void VKAPI_CALL CmdDrawIndexed(VkCommandBuffer commandBuffer, uint32_t indexCount, uint32_t instanceCount,
                                          uint32_t firstIndex, int32_t vertexOffset, uint32_t firstInstance);

VKAPI_ATTR void VKAPI_CALL CmdDrawIndirect(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset, uint32_t drawCount,
                                           uint32_t stride);

VKAPI_ATTR void VKAPI_CALL CmdDrawIndexedIndirect(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                  uint32_t drawCount, uint32_t stride);

VKAPI_ATTR void VKAPI_CALL CmdDispatch(VkCommandBuffer commandBuffer, uint32_t groupCountX, uint32_t groupCountY,
                                       uint32_t groupCountZ);

VKAPI_ATTR void VKAPI_CALL CmdDispatchIndirect(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset);

VKAPI_ATTR void VKAPI_CALL CmdCopyBuffer(VkCommandBuffer commandBuffer, VkBuffer srcBuffer, VkBuffer dstBuffer,
                                         uint32_t regionCount, const VkBufferCopy* pRegions);

VKAPI_ATTR void VKAPI_CALL CmdCopyImage(VkCommandBuffer commandBuffer, VkImage srcImage, VkImageLayout srcImageLayout,
                                        VkImage dstImage, VkImageLayout dstImageLayout, uint32_t regionCount,
                                        const VkImageCopy* pRegions);

VKAPI_ATTR void VKAPI_CALL CmdBlitImage(VkCommandBuffer commandBuffer, VkImage srcImage, VkImageLayout srcImageLayout,
                                        VkImage dstImage, VkImageLayout dstImageLayout, uint32_t regionCount,
                                        const VkImageBlit* pRegions, VkFilter filter);

VKAPI_ATTR void VKAPI_CALL CmdCopyBufferToImage(VkCommandBuffer commandBuffer, VkBuffer srcBuffer, VkImage dstImage,
                                                VkImageLayout dstImageLayout, uint32_t regionCount,
                                                const VkBufferImageCopy* pRegions);

VKAPI_ATTR void VKAPI_CALL CmdCopyImageToBuffer(VkCommandBuffer commandBuffer, VkImage srcImage, VkImageLayout srcImageLayout,
                                                VkBuffer dstBuffer, uint32_t regionCount, const VkBufferImageCopy* pRegions);

VKAPI_ATTR void VKAPI_CALL CmdUpdateBuffer(VkCommandBuffer commandBuffer, VkBuffer dstBuffer, VkDeviceSize dstOffset,
                                           VkDeviceSize dataSize, const void* pData);

VKAPI_ATTR void VKAPI_CALL CmdFillBuffer(VkCommandBuffer commandBuffer, VkBuffer dstBuffer, VkDeviceSize dstOffset,
                                         VkDeviceSize size, uint32_t data);

VKAPI_ATTR void VKAPI_CALL CmdClearColorImage(VkCommandBuffer commandBuffer, VkImage image, VkImageLayout imageLayout,
                                              const VkClearColorValue* pColor, uint32_t rangeCount,
                                              const VkImageSubresourceRange* pRanges);

VKAPI_ATTR void VKAPI_CALL CmdClearDepthStencilImage(VkCommandBuffer commandBuffer, VkImage image, VkImageLayout imageLayout,
                                                     const VkClearDepthStencilValue* pDepthStencil, uint32_t rangeCount,
                                                     const VkImageSubresourceRange* pRanges);

VKAPI_ATTR void VKAPI_CALL CmdClearAttachments(VkCommandBuffer commandBuffer, uint32_t attachmentCount,
                                               const VkClearAttachment* pAttachments, uint32_t rectCount,
                                               const VkClearRect* pRects);

VKAPI_ATTR void VKAPI_CALL CmdResolveImage(VkCommandBuffer commandBuffer, VkImage srcImage, VkImageLayout srcImageLayout,
                                           VkImage dstImage, VkImageLayout dstImageLayout, uint32_t regionCount,
                                           const VkImageResolve* pRegions);

VKAPI_ATTR void VKAPI_CALL CmdSetEvent(VkCommandBuffer commandBuffer, VkEvent event, VkPipelineStageFlags stageMask);

VKAPI_ATTR void VKAPI_CALL CmdResetEvent(VkCommandBuffer commandBuffer, VkEvent event, VkPipelineStageFlags stageMask);

VKAPI_ATTR void VKAPI_CALL CmdWaitEvents(VkCommandBuffer commandBuffer, uint32_t eventCount, const VkEvent* pEvents,
                                         VkPipelineStageFlags srcStageMask, VkPipelineStageFlags dstStageMask,
                                         uint32_t memoryBarrierCount, const VkMemoryBarrier* pMemoryBarriers,
                                         uint32_t bufferMemoryBarrierCount, const VkBufferMemoryBarrier* pBufferMemoryBarriers,
                                         uint32_t imageMemoryBarrierCount, const VkImageMemoryBarrier* pImageMemoryBarriers);

VKAPI_ATTR void VKAPI_CALL CmdPipelineBarrier(VkCommandBuffer commandBuffer, VkPipelineStageFlags srcStageMask,
                                              VkPipelineStageFlags dstStageMask, VkDependencyFlags dependencyFlags,
                                              uint32_t memoryBarrierCount, const VkMemoryBarrier* pMemoryBarriers,
                                              uint32_t bufferMemoryBarrierCount, const VkBufferMemoryBarrier* pBufferMemoryBarriers,
                                              uint32_t imageMemoryBarrierCount, const VkImageMemoryBarrier* pImageMemoryBarriers);

VKAPI_ATTR void VKAPI_CALL CmdBeginQuery(VkCommandBuffer commandBuffer, VkQueryPool queryPool, uint32_t query,
                                         VkQueryControlFlags flags);

VKAPI_ATTR void VKAPI_CALL CmdEndQuery(VkCommandBuffer commandBuffer, VkQueryPool queryPool, uint32_t query);

VKAPI_ATTR void VKAPI_CALL CmdResetQueryPool(VkCommandBuffer commandBuffer, VkQueryPool queryPool, uint32_t firstQuery,
                                             uint32_t queryCount);

VKAPI_ATTR void VKAPI_CALL CmdWriteTimestamp(VkCommandBuffer commandBuffer, VkPipelineStageFlagBits pipelineStage,
                                             VkQueryPool queryPool, uint32_t query);

VKAPI_ATTR void VKAPI_CALL CmdCopyQueryPoolResults(VkCommandBuffer commandBuffer, VkQueryPool queryPool, uint32_t firstQuery,
                                                   uint32_t queryCount, VkBuffer dstBuffer, VkDeviceSize dstOffset,
                                                   VkDeviceSize stride, VkQueryResultFlags flags);

VKAPI_ATTR void VKAPI_CALL CmdPushConstants(VkCommandBuffer commandBuffer, VkPipelineLayout layout, VkShaderStageFlags stageFlags,
                                            uint32_t offset, uint32_t size, const void* pValues);

VKAPI_ATTR void VKAPI_CALL CmdBeginRenderPass(VkCommandBuffer commandBuffer, const VkRenderPassBeginInfo* pRenderPassBegin,
                                              VkSubpassContents contents);

VKAPI_ATTR void VKAPI_CALL CmdNextSubpass(VkCommandBuffer commandBuffer, VkSubpassContents contents);

VKAPI_ATTR void VKAPI_CALL CmdEndRenderPass(VkCommandBuffer commandBuffer);

VKAPI_ATTR void VKAPI_CALL CmdExecuteCommands(VkCommandBuffer commandBuffer, uint32_t commandBufferCount,
                                              const VkCommandBuffer* pCommandBuffers);

VKAPI_ATTR VkResult VKAPI_CALL BindBufferMemory2(VkDevice device, uint32_t bindInfoCount, const VkBindBufferMemoryInfo* pBindInfos);

VKAPI_ATTR VkResult VKAPI_CALL BindImageMemory2(VkDevice device, uint32_t bindInfoCount, const VkBindImageMemoryInfo* pBindInfos);

VKAPI_ATTR void VKAPI_CALL GetDeviceGroupPeerMemoryFeatures(VkDevice device, uint32_t heapIndex, uint32_t localDeviceIndex,
                                                            uint32_t remoteDeviceIndex,
                                                            VkPeerMemoryFeatureFlags* pPeerMemoryFeatures);

VKAPI_ATTR void VKAPI_CALL CmdSetDeviceMask(VkCommandBuffer commandBuffer, uint32_t deviceMask);

VKAPI_ATTR void VKAPI_CALL CmdDispatchBase(VkCommandBuffer commandBuffer, uint32_t baseGroupX, uint32_t baseGroupY,
                                           uint32_t baseGroupZ, uint32_t groupCountX, uint32_t groupCountY, uint32_t groupCountZ);

VKAPI_ATTR VkResult VKAPI_CALL EnumeratePhysicalDeviceGroups(VkInstance instance, uint32_t* pPhysicalDeviceGroupCount,
                                                             VkPhysicalDeviceGroupProperties* pPhysicalDeviceGroupProperties);

VKAPI_ATTR void VKAPI_CALL GetImageMemoryRequirements2(VkDevice device, const VkImageMemoryRequirementsInfo2* pInfo,
                                                       VkMemoryRequirements2* pMemoryRequirements);

VKAPI_ATTR void VKAPI_CALL GetBufferMemoryRequirements2(VkDevice device, const VkBufferMemoryRequirementsInfo2* pInfo,
                                                        VkMemoryRequirements2* pMemoryRequirements);

VKAPI_ATTR void VKAPI_CALL GetImageSparseMemoryRequirements2(VkDevice device, const VkImageSparseMemoryRequirementsInfo2* pInfo,
                                                             uint32_t* pSparseMemoryRequirementCount,
                                                             VkSparseImageMemoryRequirements2* pSparseMemoryRequirements);

VKAPI_ATTR void VKAPI_CALL GetPhysicalDeviceFeatures2(VkPhysicalDevice physicalDevice, VkPhysicalDeviceFeatures2* pFeatures);

VKAPI_ATTR void VKAPI_CALL GetPhysicalDeviceProperties2(VkPhysicalDevice physicalDevice, VkPhysicalDeviceProperties2* pProperties);

VKAPI_ATTR void VKAPI_CALL GetPhysicalDeviceFormatProperties2(VkPhysicalDevice physicalDevice, VkFormat format,
                                                              VkFormatProperties2* pFormatProperties);

VKAPI_ATTR VkResult VKAPI_CALL GetPhysicalDeviceImageFormatProperties2(VkPhysicalDevice physicalDevice,
                                                                       const VkPhysicalDeviceImageFormatInfo2* pImageFormatInfo,
                                                                       VkImageFormatProperties2* pImageFormatProperties);

VKAPI_ATTR void VKAPI_CALL GetPhysicalDeviceQueueFamilyProperties2(VkPhysicalDevice physicalDevice,
                                                                   uint32_t* pQueueFamilyPropertyCount,
                                                                   VkQueueFamilyProperties2* pQueueFamilyProperties);

VKAPI_ATTR void VKAPI_CALL GetPhysicalDeviceMemoryProperties2(VkPhysicalDevice physicalDevice,
                                                              VkPhysicalDeviceMemoryProperties2* pMemoryProperties);

VKAPI_ATTR void VKAPI_CALL GetPhysicalDeviceSparseImageFormatProperties2(VkPhysicalDevice physicalDevice,
                                                                         const VkPhysicalDeviceSparseImageFormatInfo2* pFormatInfo,
                                                                         uint32_t* pPropertyCount,
                                                                         VkSparseImageFormatProperties2* pProperties);

VKAPI_ATTR void VKAPI_CALL TrimCommandPool(VkDevice device, VkCommandPool commandPool, VkCommandPoolTrimFlags flags);

VKAPI_ATTR void VKAPI_CALL GetDeviceQueue2(VkDevice device, const VkDeviceQueueInfo2* pQueueInfo, VkQueue* pQueue);

VKAPI_ATTR VkResult VKAPI_CALL CreateSamplerYcbcrConversion(VkDevice device, const VkSamplerYcbcrConversionCreateInfo* pCreateInfo,
                                                            const VkAllocationCallbacks* pAllocator,
                                                            VkSamplerYcbcrConversion* pYcbcrConversion);

VKAPI_ATTR void VKAPI_CALL DestroySamplerYcbcrConversion(VkDevice device, VkSamplerYcbcrConversion ycbcrConversion,
                                                         const VkAllocationCallbacks* pAllocator);

VKAPI_ATTR VkResult VKAPI_CALL CreateDescriptorUpdateTemplate(VkDevice device,
                                                              const VkDescriptorUpdateTemplateCreateInfo* pCreateInfo,
                                                              const VkAllocationCallbacks* pAllocator,
                                                              VkDescriptorUpdateTemplate* pDescriptorUpdateTemplate);

VKAPI_ATTR void VKAPI_CALL DestroyDescriptorUpdateTemplate(VkDevice device, VkDescriptorUpdateTemplate descriptorUpdateTemplate,
                                                           const VkAllocationCallbacks* pAllocator);

VKAPI_ATTR void VKAPI_CALL UpdateDescriptorSetWithTemplate(VkDevice device, VkDescriptorSet descriptorSet,
                                                           VkDescriptorUpdateTemplate descriptorUpdateTemplate, const void* pData);

VKAPI_ATTR void VKAPI_CALL GetPhysicalDeviceExternalBufferProperties(VkPhysicalDevice physicalDevice,
                                                                     const VkPhysicalDeviceExternalBufferInfo* pExternalBufferInfo,
                                                                     VkExternalBufferProperties* pExternalBufferProperties);

VKAPI_ATTR void VKAPI_CALL GetPhysicalDeviceExternalFenceProperties(VkPhysicalDevice physicalDevice,
                                                                    const VkPhysicalDeviceExternalFenceInfo* pExternalFenceInfo,
                                                                    VkExternalFenceProperties* pExternalFenceProperties);

VKAPI_ATTR void VKAPI_CALL GetPhysicalDeviceExternalSemaphoreProperties(
    VkPhysicalDevice physicalDevice, const VkPhysicalDeviceExternalSemaphoreInfo* pExternalSemaphoreInfo,
    VkExternalSemaphoreProperties* pExternalSemaphoreProperties);

VKAPI_ATTR void VKAPI_CALL GetDescriptorSetLayoutSupport(VkDevice device, const VkDescriptorSetLayoutCreateInfo* pCreateInfo,
                                                         VkDescriptorSetLayoutSupport* pSupport);

VKAPI_ATTR void VKAPI_CALL DestroySurfaceKHR(VkInstance instance, VkSurfaceKHR surface, const VkAllocationCallbacks* pAllocator);

VKAPI_ATTR VkResult VKAPI_CALL GetPhysicalDeviceSurfaceSupportKHR(VkPhysicalDevice physicalDevice, uint32_t queueFamilyIndex,
                                                                  VkSurfaceKHR surface, VkBool32* pSupported);

VKAPI_ATTR VkResult VKAPI_CALL GetPhysicalDeviceSurfaceCapabilitiesKHR(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface,
                                                                       VkSurfaceCapabilitiesKHR* pSurfaceCapabilities);

VKAPI_ATTR VkResult VKAPI_CALL GetPhysicalDeviceSurfaceFormatsKHR(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface,
                                                                  uint32_t* pSurfaceFormatCount,
                                                                  VkSurfaceFormatKHR* pSurfaceFormats);

VKAPI_ATTR VkResult VKAPI_CALL GetPhysicalDeviceSurfacePresentModesKHR(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface,
                                                                       uint32_t* pPresentModeCount,
                                                                       VkPresentModeKHR* pPresentModes);

VKAPI_ATTR VkResult VKAPI_CALL CreateSwapchainKHR(VkDevice device, const VkSwapchainCreateInfoKHR* pCreateInfo,
                                                  const VkAllocationCallbacks* pAllocator, VkSwapchainKHR* pSwapchain);

VKAPI_ATTR void VKAPI_CALL DestroySwapchainKHR(VkDevice device, VkSwapchainKHR swapchain, const VkAllocationCallbacks* pAllocator);

VKAPI_ATTR VkResult VKAPI_CALL GetSwapchainImagesKHR(VkDevice device, VkSwapchainKHR swapchain, uint32_t* pSwapchainImageCount,
                                                     VkImage* pSwapchainImages);

VKAPI_ATTR VkResult VKAPI_CALL AcquireNextImageKHR(VkDevice device, VkSwapchainKHR swapchain, uint64_t timeout,
                                                   VkSemaphore semaphore, VkFence fence, uint32_t* pImageIndex);

VKAPI_ATTR VkResult VKAPI_CALL QueuePresentKHR(VkQueue queue, const VkPresentInfoKHR* pPresentInfo);

VKAPI_ATTR VkResult VKAPI_CALL
GetDeviceGroupPresentCapabilitiesKHR(VkDevice device, VkDeviceGroupPresentCapabilitiesKHR* pDeviceGroupPresentCapabilities);

VKAPI_ATTR VkResult VKAPI_CALL GetDeviceGroupSurfacePresentModesKHR(VkDevice device, VkSurfaceKHR surface,
                                                                    VkDeviceGroupPresentModeFlagsKHR* pModes);

VKAPI_ATTR VkResult VKAPI_CALL GetPhysicalDevicePresentRectanglesKHR(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface,
                                                                     uint32_t* pRectCount, VkRect2D* pRects);

VKAPI_ATTR VkResult VKAPI_CALL AcquireNextImage2KHR(VkDevice device, const VkAcquireNextImageInfoKHR* pAcquireInfo,
                                                    uint32_t* pImageIndex);

VKAPI_ATTR VkResult VKAPI_CALL GetPhysicalDeviceDisplayPropertiesKHR(VkPhysicalDevice physicalDevice, uint32_t* pPropertyCount,
                                                                     VkDisplayPropertiesKHR* pProperties);

VKAPI_ATTR VkResult VKAPI_CALL GetPhysicalDeviceDisplayPlanePropertiesKHR(VkPhysicalDevice physicalDevice, uint32_t* pPropertyCount,
                                                                          VkDisplayPlanePropertiesKHR* pProperties);

VKAPI_ATTR VkResult VKAPI_CALL GetDisplayPlaneSupportedDisplaysKHR(VkPhysicalDevice physicalDevice, uint32_t planeIndex,
                                                                   uint32_t* pDisplayCount, VkDisplayKHR* pDisplays);

VKAPI_ATTR VkResult VKAPI_CALL GetDisplayModePropertiesKHR(VkPhysicalDevice physicalDevice, VkDisplayKHR display,
                                                           uint32_t* pPropertyCount, VkDisplayModePropertiesKHR* pProperties);

VKAPI_ATTR VkResult VKAPI_CALL CreateDisplayModeKHR(VkPhysicalDevice physicalDevice, VkDisplayKHR display,
                                                    const VkDisplayModeCreateInfoKHR* pCreateInfo,
                                                    const VkAllocationCallbacks* pAllocator, VkDisplayModeKHR* pMode);

VKAPI_ATTR VkResult VKAPI_CALL GetDisplayPlaneCapabilitiesKHR(VkPhysicalDevice physicalDevice, VkDisplayModeKHR mode,
                                                              uint32_t planeIndex, VkDisplayPlaneCapabilitiesKHR* pCapabilities);

VKAPI_ATTR VkResult VKAPI_CALL CreateDisplayPlaneSurfaceKHR(VkInstance instance, const VkDisplaySurfaceCreateInfoKHR* pCreateInfo,
                                                            const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface);

VKAPI_ATTR VkResult VKAPI_CALL CreateSharedSwapchainsKHR(VkDevice device, uint32_t swapchainCount,
                                                         const VkSwapchainCreateInfoKHR* pCreateInfos,
                                                         const VkAllocationCallbacks* pAllocator, VkSwapchainKHR* pSwapchains);

#ifdef VK_USE_PLATFORM_XLIB_KHR

VKAPI_ATTR VkResult VKAPI_CALL CreateXlibSurfaceKHR(VkInstance instance, const VkXlibSurfaceCreateInfoKHR* pCreateInfo,
                                                    const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface);

VKAPI_ATTR VkBool32 VKAPI_CALL GetPhysicalDeviceXlibPresentationSupportKHR(VkPhysicalDevice physicalDevice,
                                                                           uint32_t queueFamilyIndex, Display* dpy,
                                                                           VisualID visualID);
#endif  // VK_USE_PLATFORM_XLIB_KHR

#ifdef VK_USE_PLATFORM_XCB_KHR

VKAPI_ATTR VkResult VKAPI_CALL CreateXcbSurfaceKHR(VkInstance instance, const VkXcbSurfaceCreateInfoKHR* pCreateInfo,
                                                   const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface);

VKAPI_ATTR VkBool32 VKAPI_CALL GetPhysicalDeviceXcbPresentationSupportKHR(VkPhysicalDevice physicalDevice,
                                                                          uint32_t queueFamilyIndex, xcb_connection_t* connection,
                                                                          xcb_visualid_t visual_id);
#endif  // VK_USE_PLATFORM_XCB_KHR

#ifdef VK_USE_PLATFORM_WAYLAND_KHR

VKAPI_ATTR VkResult VKAPI_CALL CreateWaylandSurfaceKHR(VkInstance instance, const VkWaylandSurfaceCreateInfoKHR* pCreateInfo,
                                                       const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface);

VKAPI_ATTR VkBool32 VKAPI_CALL GetPhysicalDeviceWaylandPresentationSupportKHR(VkPhysicalDevice physicalDevice,
                                                                              uint32_t queueFamilyIndex,
                                                                              struct wl_display* display);
#endif  // VK_USE_PLATFORM_WAYLAND_KHR

#ifdef VK_USE_PLATFORM_ANDROID_KHR

VKAPI_ATTR VkResult VKAPI_CALL CreateAndroidSurfaceKHR(VkInstance instance, const VkAndroidSurfaceCreateInfoKHR* pCreateInfo,
                                                       const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface);
#endif  // VK_USE_PLATFORM_ANDROID_KHR

#ifdef VK_USE_PLATFORM_WIN32_KHR

VKAPI_ATTR VkResult VKAPI_CALL CreateWin32SurfaceKHR(VkInstance instance, const VkWin32SurfaceCreateInfoKHR* pCreateInfo,
                                                     const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface);

VKAPI_ATTR VkBool32 VKAPI_CALL GetPhysicalDeviceWin32PresentationSupportKHR(VkPhysicalDevice physicalDevice,
                                                                            uint32_t queueFamilyIndex);
#endif  // VK_USE_PLATFORM_WIN32_KHR

VKAPI_ATTR void VKAPI_CALL GetPhysicalDeviceFeatures2KHR(VkPhysicalDevice physicalDevice, VkPhysicalDeviceFeatures2* pFeatures);

VKAPI_ATTR void VKAPI_CALL GetPhysicalDeviceProperties2KHR(VkPhysicalDevice physicalDevice,
                                                           VkPhysicalDeviceProperties2* pProperties);

VKAPI_ATTR void VKAPI_CALL GetPhysicalDeviceFormatProperties2KHR(VkPhysicalDevice physicalDevice, VkFormat format,
                                                                 VkFormatProperties2* pFormatProperties);

VKAPI_ATTR VkResult VKAPI_CALL GetPhysicalDeviceImageFormatProperties2KHR(VkPhysicalDevice physicalDevice,
                                                                          const VkPhysicalDeviceImageFormatInfo2* pImageFormatInfo,
                                                                          VkImageFormatProperties2* pImageFormatProperties);

VKAPI_ATTR void VKAPI_CALL GetPhysicalDeviceQueueFamilyProperties2KHR(VkPhysicalDevice physicalDevice,
                                                                      uint32_t* pQueueFamilyPropertyCount,
                                                                      VkQueueFamilyProperties2* pQueueFamilyProperties);

VKAPI_ATTR void VKAPI_CALL GetPhysicalDeviceMemoryProperties2KHR(VkPhysicalDevice physicalDevice,
                                                                 VkPhysicalDeviceMemoryProperties2* pMemoryProperties);

VKAPI_ATTR void VKAPI_CALL GetPhysicalDeviceSparseImageFormatProperties2KHR(
    VkPhysicalDevice physicalDevice, const VkPhysicalDeviceSparseImageFormatInfo2* pFormatInfo, uint32_t* pPropertyCount,
    VkSparseImageFormatProperties2* pProperties);

VKAPI_ATTR void VKAPI_CALL GetDeviceGroupPeerMemoryFeaturesKHR(VkDevice device, uint32_t heapIndex, uint32_t localDeviceIndex,
                                                               uint32_t remoteDeviceIndex,
                                                               VkPeerMemoryFeatureFlags* pPeerMemoryFeatures);

VKAPI_ATTR void VKAPI_CALL CmdSetDeviceMaskKHR(VkCommandBuffer commandBuffer, uint32_t deviceMask);

VKAPI_ATTR void VKAPI_CALL CmdDispatchBaseKHR(VkCommandBuffer commandBuffer, uint32_t baseGroupX, uint32_t baseGroupY,
                                              uint32_t baseGroupZ, uint32_t groupCountX, uint32_t groupCountY,
                                              uint32_t groupCountZ);

VKAPI_ATTR void VKAPI_CALL TrimCommandPoolKHR(VkDevice device, VkCommandPool commandPool, VkCommandPoolTrimFlags flags);

VKAPI_ATTR VkResult VKAPI_CALL EnumeratePhysicalDeviceGroupsKHR(VkInstance instance, uint32_t* pPhysicalDeviceGroupCount,
                                                                VkPhysicalDeviceGroupProperties* pPhysicalDeviceGroupProperties);

VKAPI_ATTR void VKAPI_CALL GetPhysicalDeviceExternalBufferPropertiesKHR(
    VkPhysicalDevice physicalDevice, const VkPhysicalDeviceExternalBufferInfo* pExternalBufferInfo,
    VkExternalBufferProperties* pExternalBufferProperties);

#ifdef VK_USE_PLATFORM_WIN32_KHR

VKAPI_ATTR VkResult VKAPI_CALL GetMemoryWin32HandleKHR(VkDevice device, const VkMemoryGetWin32HandleInfoKHR* pGetWin32HandleInfo,
                                                       HANDLE* pHandle);

VKAPI_ATTR VkResult VKAPI_CALL GetMemoryWin32HandlePropertiesKHR(VkDevice device, VkExternalMemoryHandleTypeFlagBits handleType,
                                                                 HANDLE handle,
                                                                 VkMemoryWin32HandlePropertiesKHR* pMemoryWin32HandleProperties);
#endif  // VK_USE_PLATFORM_WIN32_KHR

VKAPI_ATTR VkResult VKAPI_CALL GetMemoryFdKHR(VkDevice device, const VkMemoryGetFdInfoKHR* pGetFdInfo, int* pFd);

VKAPI_ATTR VkResult VKAPI_CALL GetMemoryFdPropertiesKHR(VkDevice device, VkExternalMemoryHandleTypeFlagBits handleType, int fd,
                                                        VkMemoryFdPropertiesKHR* pMemoryFdProperties);

#ifdef VK_USE_PLATFORM_WIN32_KHR
#endif  // VK_USE_PLATFORM_WIN32_KHR

VKAPI_ATTR void VKAPI_CALL GetPhysicalDeviceExternalSemaphorePropertiesKHR(
    VkPhysicalDevice physicalDevice, const VkPhysicalDeviceExternalSemaphoreInfo* pExternalSemaphoreInfo,
    VkExternalSemaphoreProperties* pExternalSemaphoreProperties);

#ifdef VK_USE_PLATFORM_WIN32_KHR

VKAPI_ATTR VkResult VKAPI_CALL
ImportSemaphoreWin32HandleKHR(VkDevice device, const VkImportSemaphoreWin32HandleInfoKHR* pImportSemaphoreWin32HandleInfo);

VKAPI_ATTR VkResult VKAPI_CALL GetSemaphoreWin32HandleKHR(VkDevice device,
                                                          const VkSemaphoreGetWin32HandleInfoKHR* pGetWin32HandleInfo,
                                                          HANDLE* pHandle);
#endif  // VK_USE_PLATFORM_WIN32_KHR

VKAPI_ATTR VkResult VKAPI_CALL ImportSemaphoreFdKHR(VkDevice device, const VkImportSemaphoreFdInfoKHR* pImportSemaphoreFdInfo);

VKAPI_ATTR VkResult VKAPI_CALL GetSemaphoreFdKHR(VkDevice device, const VkSemaphoreGetFdInfoKHR* pGetFdInfo, int* pFd);

VKAPI_ATTR void VKAPI_CALL CmdPushDescriptorSetKHR(VkCommandBuffer commandBuffer, VkPipelineBindPoint pipelineBindPoint,
                                                   VkPipelineLayout layout, uint32_t set, uint32_t descriptorWriteCount,
                                                   const VkWriteDescriptorSet* pDescriptorWrites);

VKAPI_ATTR void VKAPI_CALL CmdPushDescriptorSetWithTemplateKHR(VkCommandBuffer commandBuffer,
                                                               VkDescriptorUpdateTemplate descriptorUpdateTemplate,
                                                               VkPipelineLayout layout, uint32_t set, const void* pData);

VKAPI_ATTR VkResult VKAPI_CALL CreateDescriptorUpdateTemplateKHR(VkDevice device,
                                                                 const VkDescriptorUpdateTemplateCreateInfo* pCreateInfo,
                                                                 const VkAllocationCallbacks* pAllocator,
                                                                 VkDescriptorUpdateTemplate* pDescriptorUpdateTemplate);

VKAPI_ATTR void VKAPI_CALL DestroyDescriptorUpdateTemplateKHR(VkDevice device, VkDescriptorUpdateTemplate descriptorUpdateTemplate,
                                                              const VkAllocationCallbacks* pAllocator);

VKAPI_ATTR void VKAPI_CALL UpdateDescriptorSetWithTemplateKHR(VkDevice device, VkDescriptorSet descriptorSet,
                                                              VkDescriptorUpdateTemplate descriptorUpdateTemplate,
                                                              const void* pData);

VKAPI_ATTR VkResult VKAPI_CALL CreateRenderPass2KHR(VkDevice device, const VkRenderPassCreateInfo2KHR* pCreateInfo,
                                                    const VkAllocationCallbacks* pAllocator, VkRenderPass* pRenderPass);

VKAPI_ATTR void VKAPI_CALL CmdBeginRenderPass2KHR(VkCommandBuffer commandBuffer, const VkRenderPassBeginInfo* pRenderPassBegin,
                                                  const VkSubpassBeginInfoKHR* pSubpassBeginInfo);

VKAPI_ATTR void VKAPI_CALL CmdNextSubpass2KHR(VkCommandBuffer commandBuffer, const VkSubpassBeginInfoKHR* pSubpassBeginInfo,
                                              const VkSubpassEndInfoKHR* pSubpassEndInfo);

VKAPI_ATTR void VKAPI_CALL CmdEndRenderPass2KHR(VkCommandBuffer commandBuffer, const VkSubpassEndInfoKHR* pSubpassEndInfo);

VKAPI_ATTR VkResult VKAPI_CALL GetSwapchainStatusKHR(VkDevice device, VkSwapchainKHR swapchain);

VKAPI_ATTR void VKAPI_CALL GetPhysicalDeviceExternalFencePropertiesKHR(VkPhysicalDevice physicalDevice,
                                                                       const VkPhysicalDeviceExternalFenceInfo* pExternalFenceInfo,
                                                                       VkExternalFenceProperties* pExternalFenceProperties);

#ifdef VK_USE_PLATFORM_WIN32_KHR

VKAPI_ATTR VkResult VKAPI_CALL ImportFenceWin32HandleKHR(VkDevice device,
                                                         const VkImportFenceWin32HandleInfoKHR* pImportFenceWin32HandleInfo);

VKAPI_ATTR VkResult VKAPI_CALL GetFenceWin32HandleKHR(VkDevice device, const VkFenceGetWin32HandleInfoKHR* pGetWin32HandleInfo,
                                                      HANDLE* pHandle);
#endif  // VK_USE_PLATFORM_WIN32_KHR

VKAPI_ATTR VkResult VKAPI_CALL ImportFenceFdKHR(VkDevice device, const VkImportFenceFdInfoKHR* pImportFenceFdInfo);

VKAPI_ATTR VkResult VKAPI_CALL GetFenceFdKHR(VkDevice device, const VkFenceGetFdInfoKHR* pGetFdInfo, int* pFd);

VKAPI_ATTR VkResult VKAPI_CALL GetPhysicalDeviceSurfaceCapabilities2KHR(VkPhysicalDevice physicalDevice,
                                                                        const VkPhysicalDeviceSurfaceInfo2KHR* pSurfaceInfo,
                                                                        VkSurfaceCapabilities2KHR* pSurfaceCapabilities);

VKAPI_ATTR VkResult VKAPI_CALL GetPhysicalDeviceSurfaceFormats2KHR(VkPhysicalDevice physicalDevice,
                                                                   const VkPhysicalDeviceSurfaceInfo2KHR* pSurfaceInfo,
                                                                   uint32_t* pSurfaceFormatCount,
                                                                   VkSurfaceFormat2KHR* pSurfaceFormats);

VKAPI_ATTR VkResult VKAPI_CALL GetPhysicalDeviceDisplayProperties2KHR(VkPhysicalDevice physicalDevice, uint32_t* pPropertyCount,
                                                                      VkDisplayProperties2KHR* pProperties);

VKAPI_ATTR VkResult VKAPI_CALL GetPhysicalDeviceDisplayPlaneProperties2KHR(VkPhysicalDevice physicalDevice,
                                                                           uint32_t* pPropertyCount,
                                                                           VkDisplayPlaneProperties2KHR* pProperties);

VKAPI_ATTR VkResult VKAPI_CALL GetDisplayModeProperties2KHR(VkPhysicalDevice physicalDevice, VkDisplayKHR display,
                                                            uint32_t* pPropertyCount, VkDisplayModeProperties2KHR* pProperties);

VKAPI_ATTR VkResult VKAPI_CALL GetDisplayPlaneCapabilities2KHR(VkPhysicalDevice physicalDevice,
                                                               const VkDisplayPlaneInfo2KHR* pDisplayPlaneInfo,
                                                               VkDisplayPlaneCapabilities2KHR* pCapabilities);

VKAPI_ATTR void VKAPI_CALL GetImageMemoryRequirements2KHR(VkDevice device, const VkImageMemoryRequirementsInfo2* pInfo,
                                                          VkMemoryRequirements2* pMemoryRequirements);

VKAPI_ATTR void VKAPI_CALL GetBufferMemoryRequirements2KHR(VkDevice device, const VkBufferMemoryRequirementsInfo2* pInfo,
                                                           VkMemoryRequirements2* pMemoryRequirements);

VKAPI_ATTR void VKAPI_CALL GetImageSparseMemoryRequirements2KHR(VkDevice device, const VkImageSparseMemoryRequirementsInfo2* pInfo,
                                                                uint32_t* pSparseMemoryRequirementCount,
                                                                VkSparseImageMemoryRequirements2* pSparseMemoryRequirements);

VKAPI_ATTR VkResult VKAPI_CALL CreateSamplerYcbcrConversionKHR(VkDevice device,
                                                               const VkSamplerYcbcrConversionCreateInfo* pCreateInfo,
                                                               const VkAllocationCallbacks* pAllocator,
                                                               VkSamplerYcbcrConversion* pYcbcrConversion);

VKAPI_ATTR void VKAPI_CALL DestroySamplerYcbcrConversionKHR(VkDevice device, VkSamplerYcbcrConversion ycbcrConversion,
                                                            const VkAllocationCallbacks* pAllocator);

VKAPI_ATTR VkResult VKAPI_CALL BindBufferMemory2KHR(VkDevice device, uint32_t bindInfoCount,
                                                    const VkBindBufferMemoryInfo* pBindInfos);

VKAPI_ATTR VkResult VKAPI_CALL BindImageMemory2KHR(VkDevice device, uint32_t bindInfoCount,
                                                   const VkBindImageMemoryInfo* pBindInfos);

VKAPI_ATTR void VKAPI_CALL GetDescriptorSetLayoutSupportKHR(VkDevice device, const VkDescriptorSetLayoutCreateInfo* pCreateInfo,
                                                            VkDescriptorSetLayoutSupport* pSupport);

VKAPI_ATTR void VKAPI_CALL CmdDrawIndirectCountKHR(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                   VkBuffer countBuffer, VkDeviceSize countBufferOffset, uint32_t maxDrawCount,
                                                   uint32_t stride);

VKAPI_ATTR void VKAPI_CALL CmdDrawIndexedIndirectCountKHR(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                          VkBuffer countBuffer, VkDeviceSize countBufferOffset,
                                                          uint32_t maxDrawCount, uint32_t stride);

VKAPI_ATTR VkResult VKAPI_CALL CreateDebugReportCallbackEXT(VkInstance instance,
                                                            const VkDebugReportCallbackCreateInfoEXT* pCreateInfo,
                                                            const VkAllocationCallbacks* pAllocator,
                                                            VkDebugReportCallbackEXT* pCallback);

VKAPI_ATTR void VKAPI_CALL DestroyDebugReportCallbackEXT(VkInstance instance, VkDebugReportCallbackEXT callback,
                                                         const VkAllocationCallbacks* pAllocator);

VKAPI_ATTR void VKAPI_CALL DebugReportMessageEXT(VkInstance instance, VkDebugReportFlagsEXT flags,
                                                 VkDebugReportObjectTypeEXT objectType, uint64_t object, size_t location,
                                                 int32_t messageCode, const char* pLayerPrefix, const char* pMessage);

VKAPI_ATTR VkResult VKAPI_CALL DebugMarkerSetObjectTagEXT(VkDevice device, const VkDebugMarkerObjectTagInfoEXT* pTagInfo);

VKAPI_ATTR VkResult VKAPI_CALL DebugMarkerSetObjectNameEXT(VkDevice device, const VkDebugMarkerObjectNameInfoEXT* pNameInfo);

VKAPI_ATTR void VKAPI_CALL CmdDebugMarkerBeginEXT(VkCommandBuffer commandBuffer, const VkDebugMarkerMarkerInfoEXT* pMarkerInfo);

VKAPI_ATTR void VKAPI_CALL CmdDebugMarkerEndEXT(VkCommandBuffer commandBuffer);

VKAPI_ATTR void VKAPI_CALL CmdDebugMarkerInsertEXT(VkCommandBuffer commandBuffer, const VkDebugMarkerMarkerInfoEXT* pMarkerInfo);

VKAPI_ATTR void VKAPI_CALL CmdBindTransformFeedbackBuffersEXT(VkCommandBuffer commandBuffer, uint32_t firstBinding,
                                                              uint32_t bindingCount, const VkBuffer* pBuffers,
                                                              const VkDeviceSize* pOffsets, const VkDeviceSize* pSizes);

VKAPI_ATTR void VKAPI_CALL CmdBeginTransformFeedbackEXT(VkCommandBuffer commandBuffer, uint32_t firstCounterBuffer,
                                                        uint32_t counterBufferCount, const VkBuffer* pCounterBuffers,
                                                        const VkDeviceSize* pCounterBufferOffsets);

VKAPI_ATTR void VKAPI_CALL CmdEndTransformFeedbackEXT(VkCommandBuffer commandBuffer, uint32_t firstCounterBuffer,
                                                      uint32_t counterBufferCount, const VkBuffer* pCounterBuffers,
                                                      const VkDeviceSize* pCounterBufferOffsets);

VKAPI_ATTR void VKAPI_CALL CmdBeginQueryIndexedEXT(VkCommandBuffer commandBuffer, VkQueryPool queryPool, uint32_t query,
                                                   VkQueryControlFlags flags, uint32_t index);

VKAPI_ATTR void VKAPI_CALL CmdEndQueryIndexedEXT(VkCommandBuffer commandBuffer, VkQueryPool queryPool, uint32_t query,
                                                 uint32_t index);

VKAPI_ATTR void VKAPI_CALL CmdDrawIndirectByteCountEXT(VkCommandBuffer commandBuffer, uint32_t instanceCount,
                                                       uint32_t firstInstance, VkBuffer counterBuffer,
                                                       VkDeviceSize counterBufferOffset, uint32_t counterOffset,
                                                       uint32_t vertexStride);

VKAPI_ATTR void VKAPI_CALL CmdDrawIndirectCountAMD(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                   VkBuffer countBuffer, VkDeviceSize countBufferOffset, uint32_t maxDrawCount,
                                                   uint32_t stride);

VKAPI_ATTR void VKAPI_CALL CmdDrawIndexedIndirectCountAMD(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                          VkBuffer countBuffer, VkDeviceSize countBufferOffset,
                                                          uint32_t maxDrawCount, uint32_t stride);

VKAPI_ATTR VkResult VKAPI_CALL GetShaderInfoAMD(VkDevice device, VkPipeline pipeline, VkShaderStageFlagBits shaderStage,
                                                VkShaderInfoTypeAMD infoType, size_t* pInfoSize, void* pInfo);

VKAPI_ATTR VkResult VKAPI_CALL GetPhysicalDeviceExternalImageFormatPropertiesNV(
    VkPhysicalDevice physicalDevice, VkFormat format, VkImageType type, VkImageTiling tiling, VkImageUsageFlags usage,
    VkImageCreateFlags flags, VkExternalMemoryHandleTypeFlagsNV externalHandleType,
    VkExternalImageFormatPropertiesNV* pExternalImageFormatProperties);

#ifdef VK_USE_PLATFORM_WIN32_KHR

VKAPI_ATTR VkResult VKAPI_CALL GetMemoryWin32HandleNV(VkDevice device, VkDeviceMemory memory,
                                                      VkExternalMemoryHandleTypeFlagsNV handleType, HANDLE* pHandle);
#endif  // VK_USE_PLATFORM_WIN32_KHR

#ifdef VK_USE_PLATFORM_WIN32_KHR
#endif  // VK_USE_PLATFORM_WIN32_KHR

#ifdef VK_USE_PLATFORM_VI_NN

VKAPI_ATTR VkResult VKAPI_CALL CreateViSurfaceNN(VkInstance instance, const VkViSurfaceCreateInfoNN* pCreateInfo,
                                                 const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface);
#endif  // VK_USE_PLATFORM_VI_NN

VKAPI_ATTR void VKAPI_CALL CmdBeginConditionalRenderingEXT(VkCommandBuffer commandBuffer,
                                                           const VkConditionalRenderingBeginInfoEXT* pConditionalRenderingBegin);

VKAPI_ATTR void VKAPI_CALL CmdEndConditionalRenderingEXT(VkCommandBuffer commandBuffer);

VKAPI_ATTR void VKAPI_CALL CmdProcessCommandsNVX(VkCommandBuffer commandBuffer,
                                                 const VkCmdProcessCommandsInfoNVX* pProcessCommandsInfo);

VKAPI_ATTR void VKAPI_CALL CmdReserveSpaceForCommandsNVX(VkCommandBuffer commandBuffer,
                                                         const VkCmdReserveSpaceForCommandsInfoNVX* pReserveSpaceInfo);

VKAPI_ATTR VkResult VKAPI_CALL CreateIndirectCommandsLayoutNVX(VkDevice device,
                                                               const VkIndirectCommandsLayoutCreateInfoNVX* pCreateInfo,
                                                               const VkAllocationCallbacks* pAllocator,
                                                               VkIndirectCommandsLayoutNVX* pIndirectCommandsLayout);

VKAPI_ATTR void VKAPI_CALL DestroyIndirectCommandsLayoutNVX(VkDevice device, VkIndirectCommandsLayoutNVX indirectCommandsLayout,
                                                            const VkAllocationCallbacks* pAllocator);

VKAPI_ATTR VkResult VKAPI_CALL CreateObjectTableNVX(VkDevice device, const VkObjectTableCreateInfoNVX* pCreateInfo,
                                                    const VkAllocationCallbacks* pAllocator, VkObjectTableNVX* pObjectTable);

VKAPI_ATTR void VKAPI_CALL DestroyObjectTableNVX(VkDevice device, VkObjectTableNVX objectTable,
                                                 const VkAllocationCallbacks* pAllocator);

VKAPI_ATTR VkResult VKAPI_CALL RegisterObjectsNVX(VkDevice device, VkObjectTableNVX objectTable, uint32_t objectCount,
                                                  const VkObjectTableEntryNVX* const* ppObjectTableEntries,
                                                  const uint32_t* pObjectIndices);

VKAPI_ATTR VkResult VKAPI_CALL UnregisterObjectsNVX(VkDevice device, VkObjectTableNVX objectTable, uint32_t objectCount,
                                                    const VkObjectEntryTypeNVX* pObjectEntryTypes, const uint32_t* pObjectIndices);

VKAPI_ATTR void VKAPI_CALL GetPhysicalDeviceGeneratedCommandsPropertiesNVX(VkPhysicalDevice physicalDevice,
                                                                           VkDeviceGeneratedCommandsFeaturesNVX* pFeatures,
                                                                           VkDeviceGeneratedCommandsLimitsNVX* pLimits);

VKAPI_ATTR void VKAPI_CALL CmdSetViewportWScalingNV(VkCommandBuffer commandBuffer, uint32_t firstViewport, uint32_t viewportCount,
                                                    const VkViewportWScalingNV* pViewportWScalings);

VKAPI_ATTR VkResult VKAPI_CALL ReleaseDisplayEXT(VkPhysicalDevice physicalDevice, VkDisplayKHR display);

#ifdef VK_USE_PLATFORM_XLIB_XRANDR_EXT

VKAPI_ATTR VkResult VKAPI_CALL AcquireXlibDisplayEXT(VkPhysicalDevice physicalDevice, Display* dpy, VkDisplayKHR display);

VKAPI_ATTR VkResult VKAPI_CALL GetRandROutputDisplayEXT(VkPhysicalDevice physicalDevice, Display* dpy, RROutput rrOutput,
                                                        VkDisplayKHR* pDisplay);
#endif  // VK_USE_PLATFORM_XLIB_XRANDR_EXT

VKAPI_ATTR VkResult VKAPI_CALL GetPhysicalDeviceSurfaceCapabilities2EXT(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface,
                                                                        VkSurfaceCapabilities2EXT* pSurfaceCapabilities);

VKAPI_ATTR VkResult VKAPI_CALL DisplayPowerControlEXT(VkDevice device, VkDisplayKHR display,
                                                      const VkDisplayPowerInfoEXT* pDisplayPowerInfo);

VKAPI_ATTR VkResult VKAPI_CALL RegisterDeviceEventEXT(VkDevice device, const VkDeviceEventInfoEXT* pDeviceEventInfo,
                                                      const VkAllocationCallbacks* pAllocator, VkFence* pFence);

VKAPI_ATTR VkResult VKAPI_CALL RegisterDisplayEventEXT(VkDevice device, VkDisplayKHR display,
                                                       const VkDisplayEventInfoEXT* pDisplayEventInfo,
                                                       const VkAllocationCallbacks* pAllocator, VkFence* pFence);

VKAPI_ATTR VkResult VKAPI_CALL GetSwapchainCounterEXT(VkDevice device, VkSwapchainKHR swapchain,
                                                      VkSurfaceCounterFlagBitsEXT counter, uint64_t* pCounterValue);

VKAPI_ATTR VkResult VKAPI_CALL GetRefreshCycleDurationGOOGLE(VkDevice device, VkSwapchainKHR swapchain,
                                                             VkRefreshCycleDurationGOOGLE* pDisplayTimingProperties);

VKAPI_ATTR VkResult VKAPI_CALL GetPastPresentationTimingGOOGLE(VkDevice device, VkSwapchainKHR swapchain,
                                                               uint32_t* pPresentationTimingCount,
                                                               VkPastPresentationTimingGOOGLE* pPresentationTimings);

VKAPI_ATTR void VKAPI_CALL CmdSetDiscardRectangleEXT(VkCommandBuffer commandBuffer, uint32_t firstDiscardRectangle,
                                                     uint32_t discardRectangleCount, const VkRect2D* pDiscardRectangles);

VKAPI_ATTR void VKAPI_CALL SetHdrMetadataEXT(VkDevice device, uint32_t swapchainCount, const VkSwapchainKHR* pSwapchains,
                                             const VkHdrMetadataEXT* pMetadata);

#ifdef VK_USE_PLATFORM_IOS_MVK

VKAPI_ATTR VkResult VKAPI_CALL CreateIOSSurfaceMVK(VkInstance instance, const VkIOSSurfaceCreateInfoMVK* pCreateInfo,
                                                   const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface);
#endif  // VK_USE_PLATFORM_IOS_MVK

#ifdef VK_USE_PLATFORM_MACOS_MVK

VKAPI_ATTR VkResult VKAPI_CALL CreateMacOSSurfaceMVK(VkInstance instance, const VkMacOSSurfaceCreateInfoMVK* pCreateInfo,
                                                     const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface);
#endif  // VK_USE_PLATFORM_MACOS_MVK

VKAPI_ATTR VkResult VKAPI_CALL SetDebugUtilsObjectNameEXT(VkDevice device, const VkDebugUtilsObjectNameInfoEXT* pNameInfo);

VKAPI_ATTR VkResult VKAPI_CALL SetDebugUtilsObjectTagEXT(VkDevice device, const VkDebugUtilsObjectTagInfoEXT* pTagInfo);

VKAPI_ATTR void VKAPI_CALL QueueBeginDebugUtilsLabelEXT(VkQueue queue, const VkDebugUtilsLabelEXT* pLabelInfo);

VKAPI_ATTR void VKAPI_CALL QueueEndDebugUtilsLabelEXT(VkQueue queue);

VKAPI_ATTR void VKAPI_CALL QueueInsertDebugUtilsLabelEXT(VkQueue queue, const VkDebugUtilsLabelEXT* pLabelInfo);

VKAPI_ATTR void VKAPI_CALL CmdBeginDebugUtilsLabelEXT(VkCommandBuffer commandBuffer, const VkDebugUtilsLabelEXT* pLabelInfo);

VKAPI_ATTR void VKAPI_CALL CmdEndDebugUtilsLabelEXT(VkCommandBuffer commandBuffer);

VKAPI_ATTR void VKAPI_CALL CmdInsertDebugUtilsLabelEXT(VkCommandBuffer commandBuffer, const VkDebugUtilsLabelEXT* pLabelInfo);

VKAPI_ATTR VkResult VKAPI_CALL CreateDebugUtilsMessengerEXT(VkInstance instance,
                                                            const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
                                                            const VkAllocationCallbacks* pAllocator,
                                                            VkDebugUtilsMessengerEXT* pMessenger);

VKAPI_ATTR void VKAPI_CALL DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT messenger,
                                                         const VkAllocationCallbacks* pAllocator);

VKAPI_ATTR void VKAPI_CALL SubmitDebugUtilsMessageEXT(VkInstance instance, VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
                                                      VkDebugUtilsMessageTypeFlagsEXT messageTypes,
                                                      const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData);

#ifdef VK_USE_PLATFORM_ANDROID_KHR

VKAPI_ATTR VkResult VKAPI_CALL GetAndroidHardwareBufferPropertiesANDROID(VkDevice device, const struct AHardwareBuffer* buffer,
                                                                         VkAndroidHardwareBufferPropertiesANDROID* pProperties);

VKAPI_ATTR VkResult VKAPI_CALL GetMemoryAndroidHardwareBufferANDROID(VkDevice device,
                                                                     const VkMemoryGetAndroidHardwareBufferInfoANDROID* pInfo,
                                                                     struct AHardwareBuffer** pBuffer);
#endif  // VK_USE_PLATFORM_ANDROID_KHR

VKAPI_ATTR void VKAPI_CALL CmdSetSampleLocationsEXT(VkCommandBuffer commandBuffer,
                                                    const VkSampleLocationsInfoEXT* pSampleLocationsInfo);

VKAPI_ATTR void VKAPI_CALL GetPhysicalDeviceMultisamplePropertiesEXT(VkPhysicalDevice physicalDevice, VkSampleCountFlagBits samples,
                                                                     VkMultisamplePropertiesEXT* pMultisampleProperties);

VKAPI_ATTR VkResult VKAPI_CALL GetImageDrmFormatModifierPropertiesEXT(VkDevice device, VkImage image,
                                                                      VkImageDrmFormatModifierPropertiesEXT* pProperties);

VKAPI_ATTR VkResult VKAPI_CALL CreateValidationCacheEXT(VkDevice device, const VkValidationCacheCreateInfoEXT* pCreateInfo,
                                                        const VkAllocationCallbacks* pAllocator,
                                                        VkValidationCacheEXT* pValidationCache);

VKAPI_ATTR void VKAPI_CALL DestroyValidationCacheEXT(VkDevice device, VkValidationCacheEXT validationCache,
                                                     const VkAllocationCallbacks* pAllocator);

VKAPI_ATTR VkResult VKAPI_CALL MergeValidationCachesEXT(VkDevice device, VkValidationCacheEXT dstCache, uint32_t srcCacheCount,
                                                        const VkValidationCacheEXT* pSrcCaches);

VKAPI_ATTR VkResult VKAPI_CALL GetValidationCacheDataEXT(VkDevice device, VkValidationCacheEXT validationCache, size_t* pDataSize,
                                                         void* pData);

VKAPI_ATTR void VKAPI_CALL CmdBindShadingRateImageNV(VkCommandBuffer commandBuffer, VkImageView imageView,
                                                     VkImageLayout imageLayout);

VKAPI_ATTR void VKAPI_CALL CmdSetViewportShadingRatePaletteNV(VkCommandBuffer commandBuffer, uint32_t firstViewport,
                                                              uint32_t viewportCount,
                                                              const VkShadingRatePaletteNV* pShadingRatePalettes);

VKAPI_ATTR void VKAPI_CALL CmdSetCoarseSampleOrderNV(VkCommandBuffer commandBuffer, VkCoarseSampleOrderTypeNV sampleOrderType,
                                                     uint32_t customSampleOrderCount,
                                                     const VkCoarseSampleOrderCustomNV* pCustomSampleOrders);

VKAPI_ATTR VkResult VKAPI_CALL CreateAccelerationStructureNV(VkDevice device,
                                                             const VkAccelerationStructureCreateInfoNV* pCreateInfo,
                                                             const VkAllocationCallbacks* pAllocator,
                                                             VkAccelerationStructureNV* pAccelerationStructure);

VKAPI_ATTR void VKAPI_CALL DestroyAccelerationStructureNV(VkDevice device, VkAccelerationStructureNV accelerationStructure,
                                                          const VkAllocationCallbacks* pAllocator);

VKAPI_ATTR void VKAPI_CALL GetAccelerationStructureMemoryRequirementsNV(
    VkDevice device, const VkAccelerationStructureMemoryRequirementsInfoNV* pInfo, VkMemoryRequirements2KHR* pMemoryRequirements);

VKAPI_ATTR VkResult VKAPI_CALL BindAccelerationStructureMemoryNV(VkDevice device, uint32_t bindInfoCount,
                                                                 const VkBindAccelerationStructureMemoryInfoNV* pBindInfos);

VKAPI_ATTR void VKAPI_CALL CmdBuildAccelerationStructureNV(VkCommandBuffer commandBuffer,
                                                           const VkAccelerationStructureInfoNV* pInfo, VkBuffer instanceData,
                                                           VkDeviceSize instanceOffset, VkBool32 update,
                                                           VkAccelerationStructureNV dst, VkAccelerationStructureNV src,
                                                           VkBuffer scratch, VkDeviceSize scratchOffset);

VKAPI_ATTR void VKAPI_CALL CmdCopyAccelerationStructureNV(VkCommandBuffer commandBuffer, VkAccelerationStructureNV dst,
                                                          VkAccelerationStructureNV src, VkCopyAccelerationStructureModeNV mode);

VKAPI_ATTR void VKAPI_CALL CmdTraceRaysNV(VkCommandBuffer commandBuffer, VkBuffer raygenShaderBindingTableBuffer,
                                          VkDeviceSize raygenShaderBindingOffset, VkBuffer missShaderBindingTableBuffer,
                                          VkDeviceSize missShaderBindingOffset, VkDeviceSize missShaderBindingStride,
                                          VkBuffer hitShaderBindingTableBuffer, VkDeviceSize hitShaderBindingOffset,
                                          VkDeviceSize hitShaderBindingStride, VkBuffer callableShaderBindingTableBuffer,
                                          VkDeviceSize callableShaderBindingOffset, VkDeviceSize callableShaderBindingStride,
                                          uint32_t width, uint32_t height, uint32_t depth);

VKAPI_ATTR VkResult VKAPI_CALL CreateRayTracingPipelinesNV(VkDevice device, VkPipelineCache pipelineCache, uint32_t createInfoCount,
                                                           const VkRayTracingPipelineCreateInfoNV* pCreateInfos,
                                                           const VkAllocationCallbacks* pAllocator, VkPipeline* pPipelines);

VKAPI_ATTR VkResult VKAPI_CALL GetRayTracingShaderGroupHandlesNV(VkDevice device, VkPipeline pipeline, uint32_t firstGroup,
                                                                 uint32_t groupCount, size_t dataSize, void* pData);

VKAPI_ATTR VkResult VKAPI_CALL GetAccelerationStructureHandleNV(VkDevice device, VkAccelerationStructureNV accelerationStructure,
                                                                size_t dataSize, void* pData);

VKAPI_ATTR void VKAPI_CALL CmdWriteAccelerationStructuresPropertiesNV(VkCommandBuffer commandBuffer,
                                                                      uint32_t accelerationStructureCount,
                                                                      const VkAccelerationStructureNV* pAccelerationStructures,
                                                                      VkQueryType queryType, VkQueryPool queryPool,
                                                                      uint32_t firstQuery);

VKAPI_ATTR VkResult VKAPI_CALL CompileDeferredNV(VkDevice device, VkPipeline pipeline, uint32_t shader);

VKAPI_ATTR VkResult VKAPI_CALL GetMemoryHostPointerPropertiesEXT(VkDevice device, VkExternalMemoryHandleTypeFlagBits handleType,
                                                                 const void* pHostPointer,
                                                                 VkMemoryHostPointerPropertiesEXT* pMemoryHostPointerProperties);

VKAPI_ATTR void VKAPI_CALL CmdWriteBufferMarkerAMD(VkCommandBuffer commandBuffer, VkPipelineStageFlagBits pipelineStage,
                                                   VkBuffer dstBuffer, VkDeviceSize dstOffset, uint32_t marker);

VKAPI_ATTR VkResult VKAPI_CALL GetPhysicalDeviceCalibrateableTimeDomainsEXT(VkPhysicalDevice physicalDevice,
                                                                            uint32_t* pTimeDomainCount,
                                                                            VkTimeDomainEXT* pTimeDomains);

VKAPI_ATTR VkResult VKAPI_CALL GetCalibratedTimestampsEXT(VkDevice device, uint32_t timestampCount,
                                                          const VkCalibratedTimestampInfoEXT* pTimestampInfos,
                                                          uint64_t* pTimestamps, uint64_t* pMaxDeviation);

VKAPI_ATTR void VKAPI_CALL CmdDrawMeshTasksNV(VkCommandBuffer commandBuffer, uint32_t taskCount, uint32_t firstTask);

VKAPI_ATTR void VKAPI_CALL CmdDrawMeshTasksIndirectNV(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                      uint32_t drawCount, uint32_t stride);

VKAPI_ATTR void VKAPI_CALL CmdDrawMeshTasksIndirectCountNV(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                           VkBuffer countBuffer, VkDeviceSize countBufferOffset,
                                                           uint32_t maxDrawCount, uint32_t stride);

VKAPI_ATTR void VKAPI_CALL CmdSetExclusiveScissorNV(VkCommandBuffer commandBuffer, uint32_t firstExclusiveScissor,
                                                    uint32_t exclusiveScissorCount, const VkRect2D* pExclusiveScissors);

VKAPI_ATTR void VKAPI_CALL CmdSetCheckpointNV(VkCommandBuffer commandBuffer, const void* pCheckpointMarker);

VKAPI_ATTR void VKAPI_CALL GetQueueCheckpointDataNV(VkQueue queue, uint32_t* pCheckpointDataCount,
                                                    VkCheckpointDataNV* pCheckpointData);

VKAPI_ATTR PFN_vkVoidFunction VKAPI_CALL GetPhysicalDeviceProcAddr(VkInstance instance, const char* funcName);

VKAPI_ATTR VkDeviceAddress VKAPI_CALL GetBufferDeviceAddressEXT(VkDevice device, const VkBufferDeviceAddressInfoEXT* pInfo);

#ifdef VK_USE_PLATFORM_FUCHSIA
VKAPI_ATTR VkResult VKAPI_CALL CreateImagePipeSurfaceFUCHSIA(VkInstance instance,
                                                             const VkImagePipeSurfaceCreateInfoFUCHSIA* pCreateInfo,
                                                             const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface);
#endif  // VK_USE_PLATFORM_FUCHSIA

using std::vector;
SURFACE_STATE* GetSurfaceState(instance_layer_data* instance_data, VkSurfaceKHR surface);
PHYSICAL_DEVICE_STATE* GetPhysicalDeviceState(instance_layer_data* instance_data, VkPhysicalDevice phys);

void PreCallRecordCreateInstance(VkLayerInstanceCreateInfo* chain_info);
void PostCallRecordCreateInstance(instance_layer_data* instance_data, const VkInstanceCreateInfo* pCreateInfo);
void PostCallRecordDestroyInstance(instance_layer_data* instance_data, const VkAllocationCallbacks* pAllocator, dispatch_key key);
bool PreCallValidateCreateDevice(instance_layer_data* instance_data, const VkPhysicalDeviceFeatures** enabled_features_found,
                                 VkPhysicalDevice gpu, const VkDeviceCreateInfo* pCreateInfo);
void PostCallRecordCreateDevice(instance_layer_data* instance_data, const VkPhysicalDeviceFeatures* enabled_features_found,
                                PFN_vkGetDeviceProcAddr fpGetDeviceProcAddr, VkPhysicalDevice gpu,
                                const VkDeviceCreateInfo* pCreateInfo, VkDevice* pDevice);
bool PreCallCmdUpdateBuffer(layer_data* device_data, const GLOBAL_CB_NODE* cb_state, const BUFFER_STATE* dst_buffer_state);
void PostCallRecordCmdUpdateBuffer(layer_data* device_data, GLOBAL_CB_NODE* cb_state, BUFFER_STATE* dst_buffer_state);

void PostCallRecordCreateFence(layer_data* dev_data, const VkFenceCreateInfo* pCreateInfo, VkFence* pFence);
void PostCallRecordGetDeviceQueue(layer_data* dev_data, uint32_t q_family_index, VkQueue queue);
bool PreCallValidateCreateSamplerYcbcrConversion(const layer_data* dev_data, const VkSamplerYcbcrConversionCreateInfo* create_info);
void PostCallRecordCreateSamplerYcbcrConversion(layer_data* dev_data, const VkSamplerYcbcrConversionCreateInfo* create_info,
                                                VkSamplerYcbcrConversion ycbcr_conversion);
bool PreCallValidateCmdDebugMarkerBeginEXT(layer_data* dev_data, GLOBAL_CB_NODE* cb_state);
void PreCallRecordDestroyDevice(layer_data* dev_data, VkDevice device);
bool PreCallValidateQueueSubmit(VkQueue queue, uint32_t submitCount, const VkSubmitInfo* pSubmits, VkFence fence);
void PostCallRecordQueueSubmit(VkQueue queue, uint32_t submitCount, const VkSubmitInfo* pSubmits, VkFence fence, VkResult result);
bool PreCallValidateAllocateMemory(layer_data* dev_data, const VkMemoryAllocateInfo* alloc_info);
void PostCallRecordAllocateMemory(layer_data* dev_data, const VkMemoryAllocateInfo* pAllocateInfo, VkDeviceMemory* pMemory);
bool PreCallValidateFreeMemory(layer_data* dev_data, VkDeviceMemory mem, DEVICE_MEM_INFO** mem_info, VK_OBJECT* obj_struct);
void PreCallRecordFreeMemory(layer_data* dev_data, VkDeviceMemory mem, DEVICE_MEM_INFO* mem_info, VK_OBJECT obj_struct);
bool PreCallValidateWaitForFences(layer_data* dev_data, uint32_t fence_count, const VkFence* fences);
void PostCallRecordWaitForFences(layer_data* dev_data, uint32_t fence_count, const VkFence* fences, VkBool32 wait_all);
bool PreCallValidateGetFenceStatus(layer_data* dev_data, VkFence fence);
void PostCallRecordGetFenceStatus(layer_data* dev_data, VkFence fence);
bool PreCallValidateQueueWaitIdle(VkQueue queue);
void PostCallRecordQueueWaitIdle(VkQueue queue, VkResult result);
bool PreCallValidateDeviceWaitIdle(layer_data* dev_data);
void PostCallRecordDeviceWaitIdle(layer_data* dev_data);
bool PreCallValidateDestroyFence(layer_data* dev_data, VkFence fence, FENCE_NODE** fence_node, VK_OBJECT* obj_struct);
void PreCallRecordDestroyFence(layer_data* dev_data, VkFence fence);
bool PreCallValidateDestroySemaphore(layer_data* dev_data, VkSemaphore semaphore, SEMAPHORE_NODE** sema_node,
                                     VK_OBJECT* obj_struct);
void PreCallRecordDestroySemaphore(layer_data* dev_data, VkSemaphore sema);
bool PreCallValidateDestroyEvent(layer_data* dev_data, VkEvent event, EVENT_STATE** event_state, VK_OBJECT* obj_struct);
void PreCallRecordDestroyEvent(layer_data* dev_data, VkEvent event, EVENT_STATE* event_state, VK_OBJECT obj_struct);
bool PreCallValidateDestroyQueryPool(layer_data* dev_data, VkQueryPool query_pool, QUERY_POOL_NODE** qp_state,
                                     VK_OBJECT* obj_struct);
void PreCallRecordDestroyQueryPool(layer_data* dev_data, VkQueryPool query_pool, QUERY_POOL_NODE* qp_state, VK_OBJECT obj_struct);
bool PreCallValidateGetQueryPoolResults(layer_data* dev_data, VkQueryPool query_pool, uint32_t first_query, uint32_t query_count,
                                        VkQueryResultFlags flags,
                                        unordered_map<QueryObject, std::vector<VkCommandBuffer>>* queries_in_flight);
void PostCallRecordGetQueryPoolResults(layer_data* dev_data, VkQueryPool query_pool, uint32_t first_query, uint32_t query_count,
                                       unordered_map<QueryObject, std::vector<VkCommandBuffer>>* queries_in_flight);
bool PreCallValidateBindBufferMemory2(layer_data* dev_data, std::vector<BUFFER_STATE*>* buffer_state, uint32_t bindInfoCount,
                                      const VkBindBufferMemoryInfoKHR* pBindInfos);
void PostCallRecordBindBufferMemory2(layer_data* dev_data, const std::vector<BUFFER_STATE*>& buffer_state, uint32_t bindInfoCount,
                                     const VkBindBufferMemoryInfoKHR* pBindInfos);
bool PreCallValidateBindBufferMemory(layer_data* dev_data, VkBuffer buffer, BUFFER_STATE* buffer_state, VkDeviceMemory mem,
                                     VkDeviceSize memoryOffset, const char* api_name);
void PostCallRecordBindBufferMemory(layer_data* dev_data, VkBuffer buffer, BUFFER_STATE* buffer_state, VkDeviceMemory mem,
                                    VkDeviceSize memoryOffset, const char* api_name);
void PostCallRecordGetBufferMemoryRequirements(layer_data* dev_data, VkBuffer buffer, VkMemoryRequirements* pMemoryRequirements);
bool PreCallValidateGetImageMemoryRequirements2(layer_data* dev_data, const VkImageMemoryRequirementsInfo2* pInfo);
void PostCallRecordGetImageMemoryRequirements(layer_data* dev_data, VkImage image, VkMemoryRequirements* pMemoryRequirements);
void PostCallRecordGetImageSparseMemoryRequirements2(IMAGE_STATE* image_state, uint32_t req_count,
                                                     VkSparseImageMemoryRequirements2KHR* reqs);
void PostCallRecordGetImageSparseMemoryRequirements(IMAGE_STATE* image_state, uint32_t req_count,
                                                    VkSparseImageMemoryRequirements* reqs);
bool PreCallValidateGetPhysicalDeviceImageFormatProperties2(const debug_report_data* report_data,
                                                            const VkPhysicalDeviceImageFormatInfo2* pImageFormatInfo,
                                                            const VkImageFormatProperties2* pImageFormatProperties);
void PreCallRecordDestroyShaderModule(layer_data* dev_data, VkShaderModule shaderModule);
bool PreCallValidateDestroyPipeline(layer_data* dev_data, VkPipeline pipeline, PIPELINE_STATE** pipeline_state,
                                    VK_OBJECT* obj_struct);
void PreCallRecordDestroyPipeline(layer_data* dev_data, VkPipeline pipeline, PIPELINE_STATE* pipeline_state, VK_OBJECT obj_struct);
void PreCallRecordDestroyPipelineLayout(layer_data* dev_data, VkPipelineLayout pipelineLayout);
bool PreCallValidateDestroySampler(layer_data* dev_data, VkSampler sampler, SAMPLER_STATE** sampler_state, VK_OBJECT* obj_struct);
void PreCallRecordDestroySampler(layer_data* dev_data, VkSampler sampler, SAMPLER_STATE* sampler_state, VK_OBJECT obj_struct);
void PreCallRecordDestroyDescriptorSetLayout(layer_data* dev_data, VkDescriptorSetLayout ds_layout);
bool PreCallValidateDestroyDescriptorPool(layer_data* dev_data, VkDescriptorPool pool, DESCRIPTOR_POOL_STATE** desc_pool_state,
                                          VK_OBJECT* obj_struct);
void PreCallRecordDestroyDescriptorPool(layer_data* dev_data, VkDescriptorPool descriptorPool,
                                        DESCRIPTOR_POOL_STATE* desc_pool_state, VK_OBJECT obj_struct);
bool PreCallValidateFreeCommandBuffers(VkDevice device, VkCommandPool commandPool, uint32_t commandBufferCount,
                                       const VkCommandBuffer* pCommandBuffers);
void PreCallRecordFreeCommandBuffers(VkDevice device, VkCommandPool commandPool, uint32_t commandBufferCount,
                                     const VkCommandBuffer* pCommandBuffers);
void PostCallRecordCreateCommandPool(layer_data* dev_data, const VkCommandPoolCreateInfo* pCreateInfo, VkCommandPool* pCommandPool);
bool PreCallValidateCreateQueryPool(layer_data* dev_data, const VkQueryPoolCreateInfo* pCreateInfo);
void PostCallRecordCreateQueryPool(layer_data* dev_data, const VkQueryPoolCreateInfo* pCreateInfo, VkQueryPool* pQueryPool);
bool PreCallValidateDestroyCommandPool(layer_data* dev_data, VkCommandPool pool);
void PreCallRecordDestroyCommandPool(layer_data* dev_data, VkCommandPool pool);
bool PreCallValidateResetCommandPool(layer_data* dev_data, COMMAND_POOL_NODE* pPool);
void PostCallRecordResetCommandPool(layer_data* dev_data, COMMAND_POOL_NODE* pPool);
bool PreCallValidateResetFences(layer_data* dev_data, uint32_t fenceCount, const VkFence* pFences);
void PostCallRecordResetFences(layer_data* dev_data, uint32_t fenceCount, const VkFence* pFences);
bool PreCallValidateDestroyFramebuffer(layer_data* dev_data, VkFramebuffer framebuffer, FRAMEBUFFER_STATE** framebuffer_state,
                                       VK_OBJECT* obj_struct);
void PreCallRecordDestroyFramebuffer(layer_data* dev_data, VkFramebuffer framebuffer, FRAMEBUFFER_STATE* framebuffer_state,
                                     VK_OBJECT obj_struct);
bool PreCallValidateDestroyRenderPass(layer_data* dev_data, VkRenderPass render_pass, RENDER_PASS_STATE** rp_state,
                                      VK_OBJECT* obj_struct);
void PreCallRecordDestroyRenderPass(layer_data* dev_data, VkRenderPass render_pass, RENDER_PASS_STATE* rp_state,
                                    VK_OBJECT obj_struct);
bool PreCallValidateCreateGraphicsPipelines(layer_data* dev_data, std::vector<std::unique_ptr<PIPELINE_STATE>>* pipe_state,
                                            const uint32_t count, const VkGraphicsPipelineCreateInfo* pCreateInfos);
void PostCallRecordCreateGraphicsPipelines(layer_data* dev_data, vector<std::unique_ptr<PIPELINE_STATE>>* pipe_state,
                                           const uint32_t count, const VkGraphicsPipelineCreateInfo* pCreateInfos,
                                           const VkAllocationCallbacks* pAllocator, VkPipeline* pPipelines);
bool PreCallValidateCreateComputePipelines(layer_data* dev_data, std::vector<std::unique_ptr<PIPELINE_STATE>>* pipe_state,
                                           const uint32_t count, const VkComputePipelineCreateInfo* pCreateInfos);
void PostCallRecordCreateComputePipelines(layer_data* dev_data, vector<std::unique_ptr<PIPELINE_STATE>>* pipe_state,
                                          const uint32_t count, VkPipeline* pPipelines);

bool PreCallValidateCreateRayTracingPipelinesNV(layer_data* dev_data, uint32_t count,
                                                const VkRayTracingPipelineCreateInfoNV* pCreateInfos,
                                                vector<std::unique_ptr<PIPELINE_STATE>>& pipe_state);
void PostCallRecordCreateRayTracingPipelinesNV(layer_data* dev_data, uint32_t count,
                                               vector<std::unique_ptr<PIPELINE_STATE>>& pipe_state, VkPipeline* pPipelines);
void PostCallRecordCreateSampler(layer_data* dev_data, const VkSamplerCreateInfo* pCreateInfo, VkSampler* pSampler);
bool PreCallValidateCreateDescriptorSetLayout(layer_data* dev_data, const VkDescriptorSetLayoutCreateInfo* create_info);
void PostCallRecordCreateDescriptorSetLayout(layer_data* dev_data, const VkDescriptorSetLayoutCreateInfo* create_info,
                                             VkDescriptorSetLayout set_layout);
bool PreCallValidateCreatePipelineLayout(const layer_data* dev_data, const VkPipelineLayoutCreateInfo* pCreateInfo);
void PostCallRecordCreatePipelineLayout(layer_data* dev_data, const VkPipelineLayoutCreateInfo* pCreateInfo,
                                        const VkPipelineLayout* pPipelineLayout);
bool PostCallValidateCreateDescriptorPool(layer_data* dev_data, VkDescriptorPool* pDescriptorPool);
void PostCallRecordCreateDescriptorPool(layer_data* dev_data, DESCRIPTOR_POOL_STATE* pNewNode, VkDescriptorPool* pDescriptorPool);
bool PreCallValidateResetDescriptorPool(layer_data* dev_data, VkDescriptorPool descriptorPool);
void PostCallRecordResetDescriptorPool(layer_data* dev_data, VkDevice device, VkDescriptorPool descriptorPool,
                                       VkDescriptorPoolResetFlags flags);
bool PreCallValidateAllocateDescriptorSets(layer_data* dev_data, const VkDescriptorSetAllocateInfo* pAllocateInfo,
                                           cvdescriptorset::AllocateDescriptorSetsData* common_data);
void PostCallRecordAllocateDescriptorSets(layer_data* dev_data, const VkDescriptorSetAllocateInfo* pAllocateInfo,
                                          VkDescriptorSet* pDescriptorSets,
                                          const cvdescriptorset::AllocateDescriptorSetsData* common_data);
bool PreCallValidateFreeDescriptorSets(const layer_data* dev_data, VkDescriptorPool pool, uint32_t count,
                                       const VkDescriptorSet* descriptor_sets);
void PreCallRecordFreeDescriptorSets(layer_data* dev_data, VkDescriptorPool pool, uint32_t count,
                                     const VkDescriptorSet* descriptor_sets);
bool PreCallValidateUpdateDescriptorSets(layer_data* dev_data, uint32_t descriptorWriteCount,
                                         const VkWriteDescriptorSet* pDescriptorWrites, uint32_t descriptorCopyCount,
                                         const VkCopyDescriptorSet* pDescriptorCopies);
void PreCallRecordUpdateDescriptorSets(layer_data* dev_data, uint32_t descriptorWriteCount,
                                       const VkWriteDescriptorSet* pDescriptorWrites, uint32_t descriptorCopyCount,
                                       const VkCopyDescriptorSet* pDescriptorCopies);
void PostCallRecordAllocateCommandBuffers(VkDevice device, const VkCommandBufferAllocateInfo* pCreateInfo,
                                          VkCommandBuffer* pCommandBuffer, VkResult result);
bool PreCallValidateBeginCommandBuffer(const VkCommandBuffer commandBuffer, const VkCommandBufferBeginInfo* pBeginInfo);
void PreCallRecordBeginCommandBuffer(const VkCommandBuffer commandBuffer, const VkCommandBufferBeginInfo* pBeginInfo);
bool PreCallValidateEndCommandBuffer(VkCommandBuffer commandBuffer);
void PostCallRecordEndCommandBuffer(VkCommandBuffer commandBuffer, VkResult result);
bool PreCallValidateResetCommandBuffer(VkCommandBuffer commandBuffer, VkCommandBufferResetFlags flags);
void PostCallRecordResetCommandBuffer(VkCommandBuffer commandBuffer, VkCommandBufferResetFlags flags, VkResult result);
bool PreCallValidateCmdBindPipeline(layer_data* dev_data, GLOBAL_CB_NODE* cb_state);
void PreCallRecordCmdBindPipeline(layer_data* dev_data, GLOBAL_CB_NODE* cb_state, VkPipelineBindPoint pipelineBindPoint,
                                  VkPipeline pipeline);
bool PreCallValidateCmdSetViewport(layer_data* dev_data, GLOBAL_CB_NODE* cb_state, VkCommandBuffer commandBuffer);
void PreCallRecordCmdSetViewport(GLOBAL_CB_NODE* cb_state, uint32_t firstViewport, uint32_t viewportCount);
bool PreCallValidateCmdSetScissor(layer_data* dev_data, GLOBAL_CB_NODE* cb_state, VkCommandBuffer commandBuffer);
void PreCallRecordCmdSetScissor(GLOBAL_CB_NODE* cb_state, uint32_t firstScissor, uint32_t scissorCount);
bool PreCallValidateCmdSetExclusiveScissorNV(layer_data* dev_data, GLOBAL_CB_NODE* cb_state, VkCommandBuffer commandBuffer);
void PreCallRecordCmdSetExclusiveScissorNV(GLOBAL_CB_NODE* cb_state, uint32_t firstExclusiveScissor,
                                           uint32_t exclusiveScissorCount);
bool PreCallValidateCmdBindShadingRateImageNV(layer_data* dev_data, GLOBAL_CB_NODE* cb_state, VkCommandBuffer commandBuffer,
                                              VkImageView imageView, VkImageLayout imageLayout);
void PreCallRecordCmdBindShadingRateImageNV(layer_data* dev_data, GLOBAL_CB_NODE* cb_state, VkImageView imageView);
bool PreCallValidateCmdSetViewportShadingRatePaletteNV(layer_data* dev_data, GLOBAL_CB_NODE* cb_state,
                                                       VkCommandBuffer commandBuffer, uint32_t firstViewport,
                                                       uint32_t viewportCount, const VkShadingRatePaletteNV* pShadingRatePalettes);
void PreCallRecordCmdSetViewportShadingRatePaletteNV(GLOBAL_CB_NODE* cb_state, uint32_t firstViewport, uint32_t viewportCount);
bool PreCallValidateCmdSetLineWidth(layer_data* dev_data, GLOBAL_CB_NODE* cb_state, VkCommandBuffer commandBuffer);
void PreCallRecordCmdSetLineWidth(GLOBAL_CB_NODE* cb_state);
bool PreCallValidateCmdSetDepthBias(layer_data* dev_data, GLOBAL_CB_NODE* cb_state, VkCommandBuffer commandBuffer,
                                    float depthBiasClamp);
void PreCallRecordCmdSetDepthBias(GLOBAL_CB_NODE* cb_state);
bool PreCallValidateCmdSetBlendConstants(layer_data* dev_data, GLOBAL_CB_NODE* cb_state, VkCommandBuffer commandBuffer);
void PreCallRecordCmdSetBlendConstants(GLOBAL_CB_NODE* cb_state);
bool PreCallValidateCmdSetDepthBounds(layer_data* dev_data, GLOBAL_CB_NODE* cb_state, VkCommandBuffer commandBuffer);
void PreCallRecordCmdSetDepthBounds(GLOBAL_CB_NODE* cb_state);
bool PreCallValidateCmdSetStencilCompareMask(layer_data* dev_data, GLOBAL_CB_NODE* cb_state, VkCommandBuffer commandBuffer);
void PreCallRecordCmdSetStencilCompareMask(GLOBAL_CB_NODE* cb_state);
bool PreCallValidateCmdSetStencilWriteMask(layer_data* dev_data, GLOBAL_CB_NODE* cb_state, VkCommandBuffer commandBuffer);
void PreCallRecordCmdSetStencilWriteMask(GLOBAL_CB_NODE* cb_state);
bool PreCallValidateCmdSetStencilReference(layer_data* dev_data, GLOBAL_CB_NODE* cb_state, VkCommandBuffer commandBuffer);
void PreCallRecordCmdSetStencilReference(GLOBAL_CB_NODE* cb_state);
bool PreCallValidateCmdBindDescriptorSets(layer_data* device_data, GLOBAL_CB_NODE* cb_state, VkPipelineBindPoint pipelineBindPoint,
                                          VkPipelineLayout layout, uint32_t firstSet, uint32_t setCount,
                                          const VkDescriptorSet* pDescriptorSets, uint32_t dynamicOffsetCount,
                                          const uint32_t* pDynamicOffsets);
void PreCallRecordCmdBindDescriptorSets(layer_data* device_data, GLOBAL_CB_NODE* cb_state, VkPipelineBindPoint pipelineBindPoint,
                                        VkPipelineLayout layout, uint32_t firstSet, uint32_t setCount,
                                        const VkDescriptorSet* pDescriptorSets, uint32_t dynamicOffsetCount,
                                        const uint32_t* pDynamicOffsets);
bool PreCallValidateCmdPushDescriptorSetKHR(layer_data* device_data, GLOBAL_CB_NODE* cb_state, const VkPipelineBindPoint bind_point,
                                            const VkPipelineLayout layout, const uint32_t set,
                                            const uint32_t descriptor_write_count, const VkWriteDescriptorSet* descriptor_writes,
                                            const char* func_name);
void PreCallRecordCmdPushDescriptorSetKHR(layer_data* device_data, GLOBAL_CB_NODE* cb_state, VkPipelineBindPoint pipelineBindPoint,
                                          VkPipelineLayout layout, uint32_t set, uint32_t descriptorWriteCount,
                                          const VkWriteDescriptorSet* pDescriptorWrites);
bool PreCallValidateCmdBindIndexBuffer(layer_data* dev_data, BUFFER_STATE* buffer_state, GLOBAL_CB_NODE* cb_node,
                                       VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset, VkIndexType indexType);
void PreCallRecordCmdBindIndexBuffer(BUFFER_STATE* buffer_state, GLOBAL_CB_NODE* cb_node, VkBuffer buffer, VkDeviceSize offset,
                                     VkIndexType indexType);
bool PreCallValidateCmdBindVertexBuffers(layer_data* dev_data, GLOBAL_CB_NODE* cb_state, uint32_t bindingCount,
                                         const VkBuffer* pBuffers, const VkDeviceSize* pOffsets);
void PreCallRecordCmdBindVertexBuffers(GLOBAL_CB_NODE* pCB, uint32_t firstBinding, uint32_t bindingCount, const VkBuffer* pBuffers,
                                       const VkDeviceSize* pOffsets);
bool PreCallValidateCmdDraw(layer_data* dev_data, VkCommandBuffer cmd_buffer, bool indexed, VkPipelineBindPoint bind_point,
                            GLOBAL_CB_NODE** cb_state, const char* caller);
void PostCallRecordCmdDraw(layer_data* dev_data, GLOBAL_CB_NODE* cb_state, VkPipelineBindPoint bind_point);
bool PreCallValidateCmdDrawIndexed(layer_data* dev_data, VkCommandBuffer cmd_buffer, bool indexed, VkPipelineBindPoint bind_point,
                                   GLOBAL_CB_NODE** cb_state, const char* caller, uint32_t indexCount, uint32_t firstIndex);
void PostCallRecordCmdDrawIndexed(layer_data* dev_data, GLOBAL_CB_NODE* cb_state, VkPipelineBindPoint bind_point);
bool PreCallValidateCmdDrawIndexedIndirect(layer_data* dev_data, VkCommandBuffer cmd_buffer, VkBuffer buffer, bool indexed,
                                           VkPipelineBindPoint bind_point, GLOBAL_CB_NODE** cb_state, BUFFER_STATE** buffer_state,
                                           const char* caller);
void PostCallRecordCmdDrawIndexedIndirect(layer_data* dev_data, GLOBAL_CB_NODE* cb_state, VkPipelineBindPoint bind_point,
                                          BUFFER_STATE* buffer_state);
bool PreCallValidateCmdDrawIndexedIndirectCountKHR(layer_data* dev_data, VkCommandBuffer commandBuffer, VkBuffer buffer,
                                                   VkDeviceSize offset, VkBuffer countBuffer, VkDeviceSize countBufferOffset,
                                                   uint32_t stride, GLOBAL_CB_NODE** cb_state, BUFFER_STATE** buffer_state,
                                                   BUFFER_STATE** count_buffer_state, bool indexed, VkPipelineBindPoint bind_point,
                                                   const char* caller);
void PostCallRecordCmdDrawIndexedIndirect(layer_data* dev_data, GLOBAL_CB_NODE* cb_state, VkPipelineBindPoint bind_point,
                                          BUFFER_STATE* buffer_state);
bool PreCallValidateCmdDispatch(layer_data* dev_data, VkCommandBuffer cmd_buffer, bool indexed, VkPipelineBindPoint bind_point,
                                GLOBAL_CB_NODE** cb_state, const char* caller);
void PostCallRecordCmdDispatch(layer_data* dev_data, GLOBAL_CB_NODE* cb_state, VkPipelineBindPoint bind_point);
bool PreCallValidateCmdDispatchIndirect(layer_data* dev_data, VkCommandBuffer cmd_buffer, VkBuffer buffer, bool indexed,
                                        VkPipelineBindPoint bind_point, GLOBAL_CB_NODE** cb_state, BUFFER_STATE** buffer_state,
                                        const char* caller);
void PostCallRecordCmdDispatchIndirect(layer_data* dev_data, GLOBAL_CB_NODE* cb_state, VkPipelineBindPoint bind_point,
                                       BUFFER_STATE* buffer_state);
bool PreCallValidateCmdDrawIndirect(layer_data* dev_data, VkCommandBuffer cmd_buffer, VkBuffer buffer, bool indexed,
                                    VkPipelineBindPoint bind_point, GLOBAL_CB_NODE** cb_state, BUFFER_STATE** buffer_state,
                                    const char* caller);
void PostCallRecordCmdDrawIndirect(layer_data* dev_data, GLOBAL_CB_NODE* cb_state, VkPipelineBindPoint bind_point,
                                   BUFFER_STATE* buffer_state);
bool PreCallValidateCmdSetEvent(layer_data* dev_data, GLOBAL_CB_NODE* cb_state, VkPipelineStageFlags stageMask);
void PreCallRecordCmdSetEvent(layer_data* dev_data, GLOBAL_CB_NODE* cb_state, VkCommandBuffer commandBuffer, VkEvent event,
                              VkPipelineStageFlags stageMask);
bool PreCallValidateCmdResetEvent(layer_data* dev_data, GLOBAL_CB_NODE* cb_state, VkPipelineStageFlags stageMask);
void PreCallRecordCmdResetEvent(layer_data* dev_data, GLOBAL_CB_NODE* cb_state, VkCommandBuffer commandBuffer, VkEvent event);
bool PreCallValidateCmdEventCount(layer_data* dev_data, GLOBAL_CB_NODE* cb_state, VkPipelineStageFlags sourceStageMask,
                                  VkPipelineStageFlags dstStageMask, uint32_t memoryBarrierCount,
                                  const VkMemoryBarrier* pMemoryBarriers, uint32_t bufferMemoryBarrierCount,
                                  const VkBufferMemoryBarrier* pBufferMemoryBarriers, uint32_t imageMemoryBarrierCount,
                                  const VkImageMemoryBarrier* pImageMemoryBarriers);
void PreCallRecordCmdWaitEvents(layer_data* dev_data, GLOBAL_CB_NODE* cb_state, uint32_t eventCount, const VkEvent* pEvents,
                                VkPipelineStageFlags sourceStageMask, uint32_t imageMemoryBarrierCount,
                                const VkImageMemoryBarrier* pImageMemoryBarriers);
void PostCallRecordCmdWaitEvents(layer_data* dev_data, GLOBAL_CB_NODE* cb_state, uint32_t bufferMemoryBarrierCount,
                                 const VkBufferMemoryBarrier* pBufferMemoryBarriers, uint32_t imageMemoryBarrierCount,
                                 const VkImageMemoryBarrier* pImageMemoryBarriers);
bool PreCallValidateCmdPipelineBarrier(layer_data* device_data, GLOBAL_CB_NODE* cb_state, VkPipelineStageFlags srcStageMask,
                                       VkPipelineStageFlags dstStageMask, VkDependencyFlags dependencyFlags,
                                       uint32_t memoryBarrierCount, const VkMemoryBarrier* pMemoryBarriers,
                                       uint32_t bufferMemoryBarrierCount, const VkBufferMemoryBarrier* pBufferMemoryBarriers,
                                       uint32_t imageMemoryBarrierCount, const VkImageMemoryBarrier* pImageMemoryBarriers);
void PreCallRecordCmdPipelineBarrier(layer_data* device_data, GLOBAL_CB_NODE* cb_state, VkCommandBuffer commandBuffer,
                                     uint32_t bufferMemoryBarrierCount, const VkBufferMemoryBarrier* pBufferMemoryBarriers,
                                     uint32_t imageMemoryBarrierCount, const VkImageMemoryBarrier* pImageMemoryBarriers);
bool PreCallValidateCmdBeginQuery(layer_data* dev_data, GLOBAL_CB_NODE* pCB, VkQueryPool queryPool, VkFlags flags);
void PostCallRecordCmdBeginQuery(layer_data* dev_data, VkQueryPool queryPool, uint32_t slot, GLOBAL_CB_NODE* pCB);
bool PreCallValidateCmdEndQuery(layer_data* dev_data, GLOBAL_CB_NODE* cb_state, const QueryObject& query,
                                VkCommandBuffer commandBuffer, VkQueryPool queryPool, uint32_t slot);
void PostCallRecordCmdEndQuery(layer_data* dev_data, GLOBAL_CB_NODE* cb_state, const QueryObject& query,
                               VkCommandBuffer commandBuffer, VkQueryPool queryPool);
bool PreCallValidateCmdResetQueryPool(layer_data* dev_data, GLOBAL_CB_NODE* cb_state);
void PostCallRecordCmdResetQueryPool(layer_data* dev_data, GLOBAL_CB_NODE* cb_state, VkCommandBuffer commandBuffer,
                                     VkQueryPool queryPool, uint32_t firstQuery, uint32_t queryCount);
bool PreCallValidateCmdCopyQueryPoolResults(layer_data* dev_data, GLOBAL_CB_NODE* cb_state, BUFFER_STATE* dst_buff_state);
void PostCallRecordCmdCopyQueryPoolResults(layer_data* dev_data, GLOBAL_CB_NODE* cb_state, BUFFER_STATE* dst_buff_state,
                                           VkQueryPool queryPool, uint32_t firstQuery, uint32_t queryCount);
bool PreCallValidateCmdPushConstants(layer_data* dev_data, VkCommandBuffer commandBuffer, VkPipelineLayout layout,
                                     VkShaderStageFlags stageFlags, uint32_t offset, uint32_t size);
bool PreCallValidateCmdWriteTimestamp(layer_data* dev_data, GLOBAL_CB_NODE* cb_state);
void PostCallRecordCmdWriteTimestamp(GLOBAL_CB_NODE* cb_state, VkCommandBuffer commandBuffer, VkQueryPool queryPool, uint32_t slot);
bool PreCallValidateCreateFramebuffer(layer_data* dev_data, const VkFramebufferCreateInfo* pCreateInfo);
// CreateFramebuffer state has been validated and call down chain completed so record new framebuffer object
void PostCallRecordCreateFramebuffer(layer_data* dev_data, const VkFramebufferCreateInfo* pCreateInfo, VkFramebuffer fb);
bool PreCallValidateCreateRenderPass(const layer_data* dev_data, VkDevice device, const VkRenderPassCreateInfo* pCreateInfo,
                                     RENDER_PASS_STATE* render_pass);
void PostCallRecordCreateRenderPass(layer_data* dev_data, const VkRenderPass render_pass_handle,
                                    std::shared_ptr<RENDER_PASS_STATE>&& render_pass);
bool PreCallValidateCreateRenderPass2KHR(const layer_data* dev_data, VkDevice device, const VkRenderPassCreateInfo2KHR* pCreateInfo,
                                         RENDER_PASS_STATE* render_pass);
bool PreCallValidateCmdBeginRenderPass(layer_data* dev_data, GLOBAL_CB_NODE* cb_state, RenderPassCreateVersion rp_version,
                                       const VkRenderPassBeginInfo* pRenderPassBegin);
void PreCallRecordCmdBeginRenderPass(layer_data* dev_data, GLOBAL_CB_NODE* cb_state, const VkRenderPassBeginInfo* pRenderPassBegin,
                                     const VkSubpassContents contents);
bool PreCallValidateCmdNextSubpass(layer_data* dev_data, GLOBAL_CB_NODE* cb_state, RenderPassCreateVersion rp_version,
                                   VkCommandBuffer commandBuffer);
void PostCallRecordCmdNextSubpass(layer_data* dev_data, GLOBAL_CB_NODE* cb_node, VkSubpassContents contents);
bool PreCallValidateCmdEndRenderPass(layer_data* dev_data, GLOBAL_CB_NODE* cb_state, RenderPassCreateVersion rp_version,
                                     VkCommandBuffer commandBuffer);
void PostCallRecordCmdEndRenderPass(layer_data* dev_data, GLOBAL_CB_NODE* cb_state);
bool PreCallValidateCmdExecuteCommands(layer_data* dev_data, GLOBAL_CB_NODE* cb_state, VkCommandBuffer commandBuffer,
                                       uint32_t commandBuffersCount, const VkCommandBuffer* pCommandBuffers);
void PreCallRecordCmdExecuteCommands(layer_data* dev_data, GLOBAL_CB_NODE* cb_state, uint32_t commandBuffersCount,
                                     const VkCommandBuffer* pCommandBuffers);
bool PreCallValidateMapMemory(layer_data* dev_data, VkDevice device, VkDeviceMemory mem, VkDeviceSize offset, VkDeviceSize size);
void PostCallRecordMapMemory(layer_data* dev_data, VkDeviceMemory mem, VkDeviceSize offset, VkDeviceSize size, void** ppData);
bool PreCallValidateUnmapMemory(const layer_data* dev_data, DEVICE_MEM_INFO* mem_info, const VkDeviceMemory mem);
void PreCallRecordUnmapMemory(DEVICE_MEM_INFO* mem_info);
bool PreCallValidateFlushMappedMemoryRanges(layer_data* dev_data, uint32_t mem_range_count, const VkMappedMemoryRange* mem_ranges);
bool PreCallValidateInvalidateMappedMemoryRanges(layer_data* dev_data, uint32_t mem_range_count,
                                                 const VkMappedMemoryRange* mem_ranges);
void PostCallRecordInvalidateMappedMemoryRanges(layer_data* dev_data, uint32_t mem_range_count,
                                                const VkMappedMemoryRange* mem_ranges);
bool PreCallValidateBindImageMemory(layer_data* dev_data, VkImage image, IMAGE_STATE* image_state, VkDeviceMemory mem,
                                    VkDeviceSize memoryOffset, const char* api_name);
void PostCallRecordBindImageMemory(layer_data* dev_data, VkImage image, IMAGE_STATE* image_state, VkDeviceMemory mem,
                                   VkDeviceSize memoryOffset, const char* api_name);
bool PreCallValidateBindImageMemory2(layer_data* dev_data, std::vector<IMAGE_STATE*>* image_state, uint32_t bindInfoCount,
                                     const VkBindImageMemoryInfoKHR* pBindInfos);
void PostCallRecordBindImageMemory2(layer_data* dev_data, const std::vector<IMAGE_STATE*>& image_state, uint32_t bindInfoCount,
                                    const VkBindImageMemoryInfoKHR* pBindInfos);
bool PreCallValidateSetEvent(layer_data* dev_data, VkEvent event);
void PreCallRecordSetEvent(layer_data* dev_data, VkEvent event);
bool PreCallValidateQueueBindSparse(VkQueue queue, uint32_t bindInfoCount, const VkBindSparseInfo* pBindInfo, VkFence fence);
void PostCallRecordQueueBindSparse(VkQueue queue, uint32_t bindInfoCount, const VkBindSparseInfo* pBindInfo, VkFence fence,
                                   VkResult result);
void PostCallRecordCreateSemaphore(layer_data* dev_data, VkSemaphore* pSemaphore);
bool PreCallValidateImportSemaphore(layer_data* dev_data, VkSemaphore semaphore, const char* caller_name);
void PostCallRecordImportSemaphore(layer_data* dev_data, VkSemaphore semaphore,
                                   VkExternalSemaphoreHandleTypeFlagBitsKHR handle_type, VkSemaphoreImportFlagsKHR flags);
void PostCallRecordGetSemaphore(layer_data* dev_data, VkSemaphore semaphore, VkExternalSemaphoreHandleTypeFlagBitsKHR handle_type);
bool PreCallValidateImportFence(layer_data* dev_data, VkFence fence, const char* caller_name);
void PostCallRecordImportFence(layer_data* dev_data, VkFence fence, VkExternalFenceHandleTypeFlagBitsKHR handle_type,
                               VkFenceImportFlagsKHR flags);
void PostCallRecordGetFence(layer_data* dev_data, VkFence fence, VkExternalFenceHandleTypeFlagBitsKHR handle_type);
void PostCallRecordCreateEvent(layer_data* dev_data, VkEvent* pEvent);
bool PreCallValidateCreateSwapchainKHR(layer_data* dev_data, const char* func_name, VkSwapchainCreateInfoKHR const* pCreateInfo,
                                       SURFACE_STATE* surface_state, SWAPCHAIN_NODE* old_swapchain_state);
void PostCallRecordCreateSwapchainKHR(layer_data* dev_data, VkResult result, const VkSwapchainCreateInfoKHR* pCreateInfo,
                                      VkSwapchainKHR* pSwapchain, SURFACE_STATE* surface_state,
                                      SWAPCHAIN_NODE* old_swapchain_state);
void PreCallRecordDestroySwapchainKHR(layer_data* dev_data, const VkSwapchainKHR swapchain);
bool PreCallValidateGetSwapchainImagesKHR(layer_data* device_data, SWAPCHAIN_NODE* swapchain_state, VkDevice device,
                                          uint32_t* pSwapchainImageCount, VkImage* pSwapchainImages);
void PostCallRecordGetSwapchainImagesKHR(layer_data* device_data, SWAPCHAIN_NODE* swapchain_state, VkDevice device,
                                         uint32_t* pSwapchainImageCount, VkImage* pSwapchainImages);
bool PreCallValidateQueuePresentKHR(VkQueue queue, const VkPresentInfoKHR* pPresentInfo);
void PostCallRecordQueuePresentKHR(VkQueue queue, const VkPresentInfoKHR* pPresentInfo, VkResult result);
bool PreCallValidateCreateSharedSwapchainsKHR(layer_data* dev_data, uint32_t swapchainCount,
                                              const VkSwapchainCreateInfoKHR* pCreateInfos, VkSwapchainKHR* pSwapchains,
                                              std::vector<SURFACE_STATE*>& surface_state,
                                              std::vector<SWAPCHAIN_NODE*>& old_swapchain_state);
void PostCallRecordCreateSharedSwapchainsKHR(layer_data* dev_data, VkResult result, uint32_t swapchainCount,
                                             const VkSwapchainCreateInfoKHR* pCreateInfos, VkSwapchainKHR* pSwapchains,
                                             std::vector<SURFACE_STATE*>& surface_state,
                                             std::vector<SWAPCHAIN_NODE*>& old_swapchain_state);
bool PreCallValidateCommonAcquireNextImage(layer_data* dev_data, VkDevice device, VkSwapchainKHR swapchain, uint64_t timeout,
                                           VkSemaphore semaphore, VkFence fence, uint32_t* pImageIndex, const char* func_name);
void PostCallRecordCommonAcquireNextImage(layer_data* dev_data, VkDevice device, VkSwapchainKHR swapchain, uint64_t timeout,
                                          VkSemaphore semaphore, VkFence fence, uint32_t* pImageIndex);
bool PreCallValidateEnumeratePhysicalDevices(instance_layer_data* instance_data, uint32_t* pPhysicalDeviceCount);
void PreCallRecordEnumeratePhysicalDevices(instance_layer_data* instance_data);
void PostCallRecordEnumeratePhysicalDevices(instance_layer_data* instance_data, const VkResult& result,
                                            uint32_t* pPhysicalDeviceCount, VkPhysicalDevice* pPhysicalDevices);
bool PreCallValidateGetPhysicalDeviceQueueFamilyProperties(instance_layer_data* instance_data, PHYSICAL_DEVICE_STATE* pd_state,
                                                           uint32_t* pQueueFamilyPropertyCount,
                                                           VkQueueFamilyProperties* pQueueFamilyProperties);
void PostCallRecordGetPhysicalDeviceQueueFamilyProperties(PHYSICAL_DEVICE_STATE* pd_state, uint32_t count,
                                                          VkQueueFamilyProperties* pQueueFamilyProperties);
bool PreCallValidateGetPhysicalDeviceQueueFamilyProperties2(instance_layer_data* instance_data, PHYSICAL_DEVICE_STATE* pd_state,
                                                            uint32_t* pQueueFamilyPropertyCount,
                                                            VkQueueFamilyProperties2KHR* pQueueFamilyProperties);
void PostCallRecordGetPhysicalDeviceQueueFamilyProperties2(PHYSICAL_DEVICE_STATE* pd_state, uint32_t count,
                                                           VkQueueFamilyProperties2KHR* pQueueFamilyProperties);
bool PreCallValidateDestroySurfaceKHR(instance_layer_data* instance_data, VkInstance instance, VkSurfaceKHR surface);
void PreCallRecordValidateDestroySurfaceKHR(instance_layer_data* instance_data, VkSurfaceKHR surface);
void PostCallRecordGetPhysicalDeviceSurfaceCapabilitiesKHR(instance_layer_data* instance_data, VkPhysicalDevice physicalDevice,
                                                           VkSurfaceCapabilitiesKHR* pSurfaceCapabilities);
void PostCallRecordGetPhysicalDeviceSurfaceCapabilities2KHR(instance_layer_data* instanceData, VkPhysicalDevice physicalDevice,
                                                            VkSurfaceCapabilities2KHR* pSurfaceCapabilities);
void PostCallRecordGetPhysicalDeviceSurfaceCapabilities2EXT(instance_layer_data* instanceData, VkPhysicalDevice physicalDevice,
                                                            VkSurfaceCapabilities2EXT* pSurfaceCapabilities);
bool PreCallValidateGetPhysicalDeviceSurfaceSupportKHR(instance_layer_data* instance_data,
                                                       PHYSICAL_DEVICE_STATE* physical_device_state, uint32_t queueFamilyIndex);
void PostCallRecordGetPhysicalDeviceSurfaceSupportKHR(instance_layer_data* instance_data, VkPhysicalDevice physicalDevice,
                                                      uint32_t queueFamilyIndex, VkSurfaceKHR surface, VkBool32* pSupported);
void PostCallRecordGetPhysicalDeviceSurfacePresentModesKHR(instance_layer_data* instance_data, VkPhysicalDevice physical_device,
                                                           uint32_t* pPresentModeCount, VkPresentModeKHR* pPresentModes);
bool PreCallValidateGetPhysicalDeviceSurfaceFormatsKHR(instance_layer_data* instance_data,
                                                       PHYSICAL_DEVICE_STATE* physical_device_state, CALL_STATE& call_state,
                                                       VkPhysicalDevice physicalDevice, uint32_t* pSurfaceFormatCount);
void PostCallRecordGetPhysicalDeviceSurfaceFormatsKHR(PHYSICAL_DEVICE_STATE* physical_device_state, CALL_STATE& call_state,
                                                      uint32_t* pSurfaceFormatCount, VkSurfaceFormatKHR* pSurfaceFormats);
void PostCallRecordGetPhysicalDeviceSurfaceFormats2KHR(instance_layer_data* instanceData, VkPhysicalDevice physicalDevice,
                                                       uint32_t* pSurfaceFormatCount, VkSurfaceFormat2KHR* pSurfaceFormats);
void PostCallRecordCreateDisplayPlaneSurfaceKHR(VkInstance instance, const VkDisplaySurfaceCreateInfoKHR* pCreateInfo,
                                                const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface);
void PreCallRecordSetDebugUtilsObjectNameEXT(layer_data* dev_data, const VkDebugUtilsObjectNameInfoEXT* pNameInfo);
void PreCallRecordQueueBeginDebugUtilsLabelEXT(VkQueue queue, const VkDebugUtilsLabelEXT* pLabelInfo);
void PostCallRecordQueueEndDebugUtilsLabelEXT(VkQueue queue);
void PreCallRecordQueueInsertDebugUtilsLabelEXT(VkQueue queue, const VkDebugUtilsLabelEXT* pLabelInfo);
void PreCallRecordCmdBeginDebugUtilsLabelEXT(layer_data* dev_data, VkCommandBuffer commandBuffer,
                                             const VkDebugUtilsLabelEXT* pLabelInfo);
void PostCallRecordCmdEndDebugUtilsLabelEXT(layer_data* dev_data, VkCommandBuffer commandBuffer);
void PreCallRecordCmdInsertDebugUtilsLabelEXT(layer_data* dev_data, VkCommandBuffer commandBuffer,
                                              const VkDebugUtilsLabelEXT* pLabelInfo);
void PostCallRecordCreateDebugUtilsMessengerEXT(instance_layer_data* instance_data,
                                                const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
                                                const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pMessenger);
void PostCallRecordDestroyDebugUtilsMessengerEXT(instance_layer_data* instance_data, VkDebugUtilsMessengerEXT messenger,
                                                 const VkAllocationCallbacks* pAllocator);
void PostCallRecordCreateDebugReportCallbackEXT(instance_layer_data* instance_data,
                                                const VkDebugReportCallbackCreateInfoEXT* pCreateInfo,
                                                const VkAllocationCallbacks* pAllocator, VkDebugReportCallbackEXT* pMsgCallback);
void PostCallDestroyDebugReportCallbackEXT(instance_layer_data* instance_data, VkDebugReportCallbackEXT msgCallback,
                                           const VkAllocationCallbacks* pAllocator);
bool PreCallValidateEnumeratePhysicalDeviceGroups(VkInstance instance, uint32_t* pPhysicalDeviceGroupCount,
                                                  VkPhysicalDeviceGroupPropertiesKHR* pPhysicalDeviceGroupProperties);
void PreCallRecordEnumeratePhysicalDeviceGroups(instance_layer_data* instance_data,
                                                VkPhysicalDeviceGroupPropertiesKHR* pPhysicalDeviceGroupProperties);
void PostCallRecordEnumeratePhysicalDeviceGroups(instance_layer_data* instance_data, uint32_t* pPhysicalDeviceGroupCount,
                                                 VkPhysicalDeviceGroupPropertiesKHR* pPhysicalDeviceGroupProperties);
bool PreCallValidateCreateDescriptorUpdateTemplate(const char* func_name, layer_data* device_data,
                                                   const VkDescriptorUpdateTemplateCreateInfoKHR* pCreateInfo,
                                                   const VkAllocationCallbacks* pAllocator,
                                                   VkDescriptorUpdateTemplateKHR* pDescriptorUpdateTemplate);
void PostCallRecordCreateDescriptorUpdateTemplate(layer_data* device_data,
                                                  const VkDescriptorUpdateTemplateCreateInfoKHR* pCreateInfo,
                                                  VkDescriptorUpdateTemplateKHR* pDescriptorUpdateTemplate);
void PreCallRecordDestroyDescriptorUpdateTemplate(layer_data* device_data, VkDescriptorUpdateTemplateKHR descriptorUpdateTemplate);
bool PreCallValidateUpdateDescriptorSetWithTemplate(layer_data* device_data, VkDescriptorSet descriptorSet,
                                                    VkDescriptorUpdateTemplateKHR descriptorUpdateTemplate, const void* pData);
void PreCallRecordUpdateDescriptorSetWithTemplate(layer_data* device_data, VkDescriptorSet descriptorSet,
                                                  VkDescriptorUpdateTemplateKHR descriptorUpdateTemplate, const void* pData);
bool PreCallValidateCmdPushDescriptorSetWithTemplateKHR(layer_data* device_data, GLOBAL_CB_NODE* cb_state,
                                                        VkDescriptorUpdateTemplateKHR descriptorUpdateTemplate,
                                                        VkPipelineLayout layout, uint32_t set, const void* pData);
void PreCallRecordCmdPushDescriptorSetWithTemplateKHR(layer_data* device_data, GLOBAL_CB_NODE* cb_state,
                                                      VkDescriptorUpdateTemplateKHR descriptorUpdateTemplate,
                                                      VkPipelineLayout layout, uint32_t set, const void* pData);
void PostCallRecordGetPhysicalDeviceDisplayPlanePropertiesKHR(instance_layer_data* instanceData, VkPhysicalDevice physicalDevice,
                                                              uint32_t* pPropertyCount, void* pProperties);
bool PreCallValidateGetDisplayPlaneSupportedDisplaysKHR(instance_layer_data* instance_data, VkPhysicalDevice physicalDevice,
                                                        uint32_t planeIndex);
bool PreCallValidateGetDisplayPlaneCapabilitiesKHR(instance_layer_data* instance_data, VkPhysicalDevice physicalDevice,
                                                   uint32_t planeIndex);
void PreCallRecordDebugMarkerSetObjectNameEXT(layer_data* dev_data, const VkDebugMarkerObjectNameInfoEXT* pNameInfo);
bool PreCallValidateCmdDebugMarkerEndEXT(layer_data* dev_data, GLOBAL_CB_NODE* cb_state);
bool PreCallValidateCmdSetDiscardRectangleEXT(layer_data* dev_data, GLOBAL_CB_NODE* cb_state);
bool PreCallValidateCmdSetSampleLocationsEXT(layer_data* dev_data, GLOBAL_CB_NODE* cb_state);
bool PreCallValidateCmdDrawIndirectCountKHR(layer_data* dev_data, VkCommandBuffer commandBuffer, VkBuffer buffer,
                                            VkDeviceSize offset, VkBuffer countBuffer, VkDeviceSize countBufferOffset,
                                            uint32_t stride, GLOBAL_CB_NODE** cb_state, BUFFER_STATE** buffer_state,
                                            BUFFER_STATE** count_buffer_state, bool indexed, VkPipelineBindPoint bind_point,
                                            const char* caller);
void PreCallRecordCmdDrawIndirectCountKHR(layer_data* dev_data, GLOBAL_CB_NODE* cb_state, VkPipelineBindPoint bind_point,
                                          BUFFER_STATE* buffer_state, BUFFER_STATE* count_buffer_state);
void PreCallRecordCmdDrawIndexedIndirectCountKHR(layer_data* dev_data, GLOBAL_CB_NODE* cb_state, VkPipelineBindPoint bind_point,
                                                 BUFFER_STATE* buffer_state, BUFFER_STATE* count_buffer_state);
bool PreCallValidateCmdDrawMeshTasksNV(layer_data* dev_data, VkCommandBuffer cmd_buffer, bool indexed,
                                       VkPipelineBindPoint bind_point, GLOBAL_CB_NODE** cb_state, const char* caller);
void PreCallRecordCmdDrawMeshTasksNV(layer_data* dev_data, GLOBAL_CB_NODE* cb_state, VkPipelineBindPoint bind_point);
bool PreCallValidateCmdDrawMeshTasksIndirectNV(layer_data* dev_data, VkCommandBuffer cmd_buffer, VkBuffer buffer, bool indexed,
                                               VkPipelineBindPoint bind_point, GLOBAL_CB_NODE** cb_state,
                                               BUFFER_STATE** buffer_state, const char* caller);
void PreCallRecordCmdDrawMeshTasksIndirectNV(layer_data* dev_data, GLOBAL_CB_NODE* cb_state, VkPipelineBindPoint bind_point,
                                             BUFFER_STATE* buffer_state);
bool PreCallValidateCmdDrawMeshTasksIndirectCountNV(layer_data* dev_data, VkCommandBuffer cmd_buffer, VkBuffer buffer,
                                                    VkBuffer count_buffer, bool indexed, VkPipelineBindPoint bind_point,
                                                    GLOBAL_CB_NODE** cb_state, BUFFER_STATE** buffer_state,
                                                    BUFFER_STATE** count_buffer_state, const char* caller);
void PreCallRecordCmdDrawMeshTasksIndirectCountNV(layer_data* dev_data, GLOBAL_CB_NODE* cb_state, VkPipelineBindPoint bind_point,
                                                  BUFFER_STATE* buffer_state, BUFFER_STATE* count_buffer_state);
void PostCallRecordDestroySamplerYcbcrConversion(layer_data* dev_data, VkSamplerYcbcrConversion ycbcr_conversion);
void PostCallRecordCreateShaderModule(layer_data* dev_data, bool is_spirv, const VkShaderModuleCreateInfo* pCreateInfo,
                                      VkShaderModule* pShaderModule, uint32_t unique_shader_id);
bool PreCallValidateGetBufferDeviceAddressEXT(layer_data* dev_data, const VkBufferDeviceAddressInfoEXT* pInfo);
#ifdef VK_USE_PLATFORM_ANDROID_KHR
bool PreCallValidateGetAndroidHardwareBufferProperties(const layer_data* dev_data, const AHardwareBuffer* ahb);
void PostCallRecordGetAndroidHardwareBufferProperties(layer_data* dev_data,
                                                      const VkAndroidHardwareBufferPropertiesANDROID* ahb_props);
bool PreCallValidateGetMemoryAndroidHardwareBuffer(const layer_data* dev_data,
                                                   const VkMemoryGetAndroidHardwareBufferInfoANDROID* pInfo);
void PostCallRecordCreateAndroidSurfaceKHR(VkInstance instance, const VkAndroidSurfaceCreateInfoKHR* pCreateInfo,
                                           const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface);
#endif  // VK_USE_PLATFORM_ANDROID_KHR
#ifdef VK_USE_PLATFORM_IOS_MVK
void PostCallRecordCreateIOSSurfaceMVK(VkInstance instance, const VkIOSSurfaceCreateInfoMVK* pCreateInfo,
                                       const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface);
#endif  // VK_USE_PLATFORM_IOS_MVK
#ifdef VK_USE_PLATFORM_MACOS_MVK
void PostCallRecordCreateMacOSSurfaceMVK(VkInstance instance, const VkMacOSSurfaceCreateInfoMVK* pCreateInfo,
                                         const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface);
#endif  // VK_USE_PLATFORM_MACOS_MVK
#ifdef VK_USE_PLATFORM_WAYLAND_KHR
bool PreCallValidateGetPhysicalDeviceWaylandPresentationSupportKHR(instance_layer_data* instance_data,
                                                                   VkPhysicalDevice physicalDevice, uint32_t queueFamilyIndex);
void PostCallRecordCreateWaylandSurfaceKHR(VkInstance instance, const VkWaylandSurfaceCreateInfoKHR* pCreateInfo,
                                           const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface);
#endif  // VK_USE_PLATFORM_WAYLAND_KHR
#ifdef VK_USE_PLATFORM_WIN32_KHR
bool PreCallValidateGetPhysicalDeviceWin32PresentationSupportKHR(instance_layer_data* instance_data,
                                                                 VkPhysicalDevice physicalDevice, uint32_t queueFamilyIndex);
void PostCallRecordCreateWin32SurfaceKHR(VkInstance instance, const VkWin32SurfaceCreateInfoKHR* pCreateInfo,
                                         const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface);
#endif  // VK_USE_PLATFORM_WIN32_KHR
#ifdef VK_USE_PLATFORM_XCB_KHR
bool PreCallValidateGetPhysicalDeviceXcbPresentationSupportKHR(instance_layer_data* instance_data, VkPhysicalDevice physicalDevice,
                                                               uint32_t queueFamilyIndex);
void PostCallRecordCreateXcbSurfaceKHR(VkInstance instance, const VkXcbSurfaceCreateInfoKHR* pCreateInfo,
                                       const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface);
#endif  // VK_USE_PLATFORM_XCB_KHR
#ifdef VK_USE_PLATFORM_XLIB_KHR
bool PreCallValidateGetPhysicalDeviceXlibPresentationSupportKHR(instance_layer_data* instance_data, VkPhysicalDevice physicalDevice,
                                                                uint32_t queueFamilyIndex);
void PostCallRecordCreateXlibSurfaceKHR(VkInstance instance, const VkXlibSurfaceCreateInfoKHR* pCreateInfo,
                                        const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface);
#endif  // VK_USE_PLATFORM_XLIB_KHR

};  // namespace core_validation
