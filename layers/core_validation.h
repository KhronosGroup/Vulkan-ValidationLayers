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

// This structure is used to save data across the CreateGraphicsPipelines down-chain API call
struct create_graphics_pipeline_api_state {
    std::vector<safe_VkGraphicsPipelineCreateInfo> gpu_create_infos;
    std::vector<std::unique_ptr<PIPELINE_STATE>> pipe_state;
    const VkGraphicsPipelineCreateInfo* pCreateInfos;
};

// This structure is used modify parameters for the CreatePipelineLayout down-chain API call
struct create_pipeline_layout_api_state {
    std::vector<VkDescriptorSetLayout> new_layouts;
    VkPipelineLayoutCreateInfo modified_create_info;
};

// This structure is used modify and pass parameters for the CreateShaderModule down-chain API call

struct create_shader_module_api_state {
    uint32_t unique_shader_id;
    VkShaderModuleCreateInfo instrumented_create_info;
    std::vector<unsigned int> instrumented_pgm;
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
    // Map for queue family index to queue count
    unordered_map<uint32_t, uint32_t> queue_family_index_map;

    // Used for instance versions of this object
    unordered_map<VkPhysicalDevice, PHYSICAL_DEVICE_STATE> physical_device_map;
    unordered_map<VkSurfaceKHR, SURFACE_STATE> surface_map;
    CHECK_DISABLED disabled = {};
    CHECK_ENABLED enabled = {};

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
PHYSICAL_DEVICE_STATE* GetPhysicalDeviceState(const layer_data* device_data);
PHYSICAL_DEVICE_STATE* GetPhysicalDeviceState(layer_data* device_data);

bool PreCallValidateCreateGraphicsPipelines(VkDevice device, VkPipelineCache pipelineCache, uint32_t count,
                                            const VkGraphicsPipelineCreateInfo* pCreateInfos,
                                            const VkAllocationCallbacks* pAllocator, VkPipeline* pPipelines,
                                            // Default parameter
                                            create_graphics_pipeline_api_state* cgpl_state);
void PreCallRecordCreateGraphicsPipelines(VkDevice device, VkPipelineCache pipelineCache, uint32_t count,
                                          const VkGraphicsPipelineCreateInfo* pCreateInfos, const VkAllocationCallbacks* pAllocator,
                                          VkPipeline* pPipelines,
                                          // Default parameter
                                          create_graphics_pipeline_api_state* cgpl_state);
void PostCallRecordCreateGraphicsPipelines(VkDevice device, VkPipelineCache pipelineCache, uint32_t count,
                                           const VkGraphicsPipelineCreateInfo* pCreateInfos,
                                           const VkAllocationCallbacks* pAllocator, VkPipeline* pPipelines, VkResult result,
                                           // Default parameter
                                           create_graphics_pipeline_api_state* cgpl_state);
bool PreCallValidateCreateComputePipelines(VkDevice device, VkPipelineCache pipelineCache, uint32_t count,
                                           const VkComputePipelineCreateInfo* pCreateInfos, const VkAllocationCallbacks* pAllocator,
                                           VkPipeline* pPipelines,
                                           // Default parameter
                                           std::vector<std::unique_ptr<PIPELINE_STATE>>* pipe_state);
void PostCallRecordCreateComputePipelines(VkDevice device, VkPipelineCache pipelineCache, uint32_t count,
                                          const VkComputePipelineCreateInfo* pCreateInfos, const VkAllocationCallbacks* pAllocator,
                                          VkPipeline* pPipelines, VkResult result,
                                          // Default parameter
                                          std::vector<std::unique_ptr<PIPELINE_STATE>>* pipe_state);
bool PreCallValidateCreatePipelineLayout(VkDevice device, const VkPipelineLayoutCreateInfo* pCreateInfo,
                                         const VkAllocationCallbacks* pAllocator, VkPipelineLayout* pPipelineLayout);
void PreCallRecordCreatePipelineLayout(VkDevice device, const VkPipelineLayoutCreateInfo* pCreateInfo,
                                       const VkAllocationCallbacks* pAllocator, VkPipelineLayout* pPipelineLayout,
                                       create_pipeline_layout_api_state* cpl_state);
void PostCallRecordCreatePipelineLayout(VkDevice device, const VkPipelineLayoutCreateInfo* pCreateInfo,
                                        const VkAllocationCallbacks* pAllocator, VkPipelineLayout* pPipelineLayout,
                                        VkResult result);
bool PreCallValidateAllocateDescriptorSets(VkDevice device, const VkDescriptorSetAllocateInfo* pAllocateInfo,
                                           VkDescriptorSet* pDescriptorSets,
                                           cvdescriptorset::AllocateDescriptorSetsData* common_data);
void PostCallRecordAllocateDescriptorSets(VkDevice device, const VkDescriptorSetAllocateInfo* pAllocateInfo,
                                          VkDescriptorSet* pDescriptorSets, VkResult result,
                                          cvdescriptorset::AllocateDescriptorSetsData* common_data);
bool PreCallValidateCreateRayTracingPipelinesNV(VkDevice device, VkPipelineCache pipelineCache, uint32_t count,
                                                const VkRayTracingPipelineCreateInfoNV* pCreateInfos,
                                                const VkAllocationCallbacks* pAllocator, VkPipeline* pPipelines,
                                                vector<std::unique_ptr<PIPELINE_STATE>>* pipe_state);
void PostCallRecordCreateRayTracingPipelinesNV(VkDevice device, VkPipelineCache pipelineCache, uint32_t count,
                                               const VkRayTracingPipelineCreateInfoNV* pCreateInfos,
                                               const VkAllocationCallbacks* pAllocator, VkPipeline* pPipelines, VkResult result,
                                               vector<std::unique_ptr<PIPELINE_STATE>>* pipe_state);

bool ValidateQueueFamilies(layer_data* device_data, uint32_t queue_family_count, const uint32_t* queue_families,
                           const char* cmd_name, const char* array_parameter_name, const char* unique_error_code,
                           const char* valid_error_code, bool optional);
void PostCallRecordCreateInstance(const VkInstanceCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator,
                                  VkInstance* pInstance, VkResult result);
bool PreCallValidateCreateDevice(VkPhysicalDevice gpu, const VkDeviceCreateInfo* pCreateInfo,
                                 const VkAllocationCallbacks* pAllocator, VkDevice* pDevice);
void PostCallRecordCreateDevice(VkPhysicalDevice gpu, const VkDeviceCreateInfo* pCreateInfo,
                                const VkAllocationCallbacks* pAllocator, VkDevice* pDevice, VkResult result);
bool PreCallValidateCmdUpdateBuffer(VkCommandBuffer commandBuffer, VkBuffer dstBuffer, VkDeviceSize dstOffset,
                                    VkDeviceSize dataSize, const void* pData);
void PostCallRecordCmdUpdateBuffer(VkCommandBuffer commandBuffer, VkBuffer dstBuffer, VkDeviceSize dstOffset, VkDeviceSize dataSize,
                                   const void* pData);
void PostCallRecordCreateFence(VkDevice device, const VkFenceCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator,
                               VkFence* pFence, VkResult result);
bool PreCallValidateGetDeviceQueue(VkDevice device, uint32_t queueFamilyIndex, uint32_t queueIndex, VkQueue* pQueue);
void PostCallRecordGetDeviceQueue(VkDevice device, uint32_t queueFamilyIndex, uint32_t queueIndex, VkQueue* pQueue);
void PostCallRecordGetDeviceQueue2(VkDevice device, const VkDeviceQueueInfo2* pQueueInfo, VkQueue* pQueue);
bool PreCallValidateCreateSamplerYcbcrConversion(VkDevice device, const VkSamplerYcbcrConversionCreateInfo* pCreateInfo,
                                                 const VkAllocationCallbacks* pAllocator,
                                                 VkSamplerYcbcrConversion* pYcbcrConversion);
void PostCallRecordCreateSamplerYcbcrConversion(VkDevice device, const VkSamplerYcbcrConversionCreateInfo* pCreateInfo,
                                                const VkAllocationCallbacks* pAllocator, VkSamplerYcbcrConversion* pYcbcrConversion,
                                                VkResult result);
bool PreCallValidateCreateSamplerYcbcrConversionKHR(VkDevice device, const VkSamplerYcbcrConversionCreateInfo* pCreateInfo,
                                                    const VkAllocationCallbacks* pAllocator,
                                                    VkSamplerYcbcrConversion* pYcbcrConversion);
void PostCallRecordCreateSamplerYcbcrConversionKHR(VkDevice device, const VkSamplerYcbcrConversionCreateInfo* pCreateInfo,
                                                   const VkAllocationCallbacks* pAllocator,
                                                   VkSamplerYcbcrConversion* pYcbcrConversion, VkResult result);
bool PreCallValidateCmdDebugMarkerBeginEXT(VkCommandBuffer commandBuffer, const VkDebugMarkerMarkerInfoEXT* pMarkerInfo);
void PreCallRecordDestroyDevice(VkDevice device, const VkAllocationCallbacks* pAllocator);
bool PreCallValidateQueueSubmit(VkQueue queue, uint32_t submitCount, const VkSubmitInfo* pSubmits, VkFence fence);
void PostCallRecordQueueSubmit(VkQueue queue, uint32_t submitCount, const VkSubmitInfo* pSubmits, VkFence fence, VkResult result);
bool PreCallValidateAllocateMemory(VkDevice device, const VkMemoryAllocateInfo* pAllocateInfo,
                                   const VkAllocationCallbacks* pAllocator, VkDeviceMemory* pMemory);
void PostCallRecordAllocateMemory(VkDevice device, const VkMemoryAllocateInfo* pAllocateInfo,
                                  const VkAllocationCallbacks* pAllocator, VkDeviceMemory* pMemory, VkResult result);
bool PreCallValidateFreeMemory(VkDevice device, VkDeviceMemory mem, const VkAllocationCallbacks* pAllocator);
void PreCallRecordFreeMemory(VkDevice device, VkDeviceMemory mem, const VkAllocationCallbacks* pAllocator);
bool PreCallValidateWaitForFences(VkDevice device, uint32_t fenceCount, const VkFence* pFences, VkBool32 waitAll, uint64_t timeout);
void PostCallRecordWaitForFences(VkDevice device, uint32_t fenceCount, const VkFence* pFences, VkBool32 waitAll, uint64_t timeout,
                                 VkResult result);
bool PreCallValidateGetFenceStatus(VkDevice device, VkFence fence);
void PostCallRecordGetFenceStatus(VkDevice device, VkFence fence, VkResult result);
bool PreCallValidateQueueWaitIdle(VkQueue queue);
void PostCallRecordQueueWaitIdle(VkQueue queue, VkResult result);
bool PreCallValidateDeviceWaitIdle(VkDevice device);
void PostCallRecordDeviceWaitIdle(VkDevice device, VkResult result);
bool PreCallValidateDestroyFence(VkDevice device, VkFence fence, const VkAllocationCallbacks* pAllocator);
void PreCallRecordDestroyFence(VkDevice device, VkFence fence, const VkAllocationCallbacks* pAllocator);
bool PreCallValidateDestroySemaphore(VkDevice device, VkSemaphore semaphore, const VkAllocationCallbacks* pAllocator);
void PreCallRecordDestroySemaphore(VkDevice device, VkSemaphore semaphore, const VkAllocationCallbacks* pAllocator);
bool PreCallValidateDestroyEvent(VkDevice device, VkEvent event, const VkAllocationCallbacks* pAllocator);
void PreCallRecordDestroyEvent(VkDevice device, VkEvent event, const VkAllocationCallbacks* pAllocator);
bool PreCallValidateDestroyQueryPool(VkDevice device, VkQueryPool queryPool, const VkAllocationCallbacks* pAllocator);
void PreCallRecordDestroyQueryPool(VkDevice device, VkQueryPool queryPool, const VkAllocationCallbacks* pAllocator);
bool PreCallValidateGetQueryPoolResults(VkDevice device, VkQueryPool queryPool, uint32_t firstQuery, uint32_t queryCount,
                                        size_t dataSize, void* pData, VkDeviceSize stride, VkQueryResultFlags flags);
void PostCallRecordGetQueryPoolResults(VkDevice device, VkQueryPool queryPool, uint32_t firstQuery, uint32_t queryCount,
                                       size_t dataSize, void* pData, VkDeviceSize stride, VkQueryResultFlags flags,
                                       VkResult result);
bool PreCallValidateBindBufferMemory2KHR(VkDevice device, uint32_t bindInfoCount, const VkBindBufferMemoryInfoKHR* pBindInfos);
void PostCallRecordBindBufferMemory2KHR(VkDevice device, uint32_t bindInfoCount, const VkBindBufferMemoryInfoKHR* pBindInfos,
                                        VkResult result);
bool PreCallValidateBindBufferMemory2(VkDevice device, uint32_t bindInfoCount, const VkBindBufferMemoryInfoKHR* pBindInfos);
void PostCallRecordBindBufferMemory2(VkDevice device, uint32_t bindInfoCount, const VkBindBufferMemoryInfoKHR* pBindInfos,
                                     VkResult result);
bool PreCallValidateBindBufferMemory(VkDevice device, VkBuffer buffer, VkDeviceMemory mem, VkDeviceSize memoryOffset);
void PostCallRecordBindBufferMemory(VkDevice device, VkBuffer buffer, VkDeviceMemory mem, VkDeviceSize memoryOffset,
                                    VkResult result);
void PostCallRecordGetBufferMemoryRequirements(VkDevice device, VkBuffer buffer, VkMemoryRequirements* pMemoryRequirements);
void PostCallRecordGetBufferMemoryRequirements2(VkDevice device, const VkBufferMemoryRequirementsInfo2KHR* pInfo,
                                                VkMemoryRequirements2KHR* pMemoryRequirements);
void PostCallRecordGetBufferMemoryRequirements2KHR(VkDevice device, const VkBufferMemoryRequirementsInfo2KHR* pInfo,
                                                   VkMemoryRequirements2KHR* pMemoryRequirements);
bool PreCallValidateGetImageMemoryRequirements2(VkDevice device, const VkImageMemoryRequirementsInfo2* pInfo,
                                                VkMemoryRequirements2* pMemoryRequirements);
bool PreCallValidateGetImageMemoryRequirements2KHR(VkDevice device, const VkImageMemoryRequirementsInfo2* pInfo,
                                                   VkMemoryRequirements2* pMemoryRequirements);
void PostCallRecordGetImageMemoryRequirements(VkDevice device, VkImage image, VkMemoryRequirements* pMemoryRequirements);
void PostCallRecordGetImageMemoryRequirements2(VkDevice device, const VkImageMemoryRequirementsInfo2* pInfo,
                                               VkMemoryRequirements2* pMemoryRequirements);
void PostCallRecordGetImageMemoryRequirements2KHR(VkDevice device, const VkImageMemoryRequirementsInfo2* pInfo,
                                                  VkMemoryRequirements2* pMemoryRequirements);
void PostCallRecordGetImageSparseMemoryRequirements2(IMAGE_STATE* image_state, uint32_t req_count,
                                                     VkSparseImageMemoryRequirements2KHR* reqs);
void PostCallRecordGetImageSparseMemoryRequirements(VkDevice device, VkImage image, uint32_t* pSparseMemoryRequirementCount,
                                                    VkSparseImageMemoryRequirements* pSparseMemoryRequirements);
void PostCallRecordGetImageSparseMemoryRequirements2(VkDevice device, const VkImageSparseMemoryRequirementsInfo2KHR* pInfo,
                                                     uint32_t* pSparseMemoryRequirementCount,
                                                     VkSparseImageMemoryRequirements2KHR* pSparseMemoryRequirements);
void PostCallRecordGetImageSparseMemoryRequirements2KHR(VkDevice device, const VkImageSparseMemoryRequirementsInfo2KHR* pInfo,
                                                        uint32_t* pSparseMemoryRequirementCount,
                                                        VkSparseImageMemoryRequirements2KHR* pSparseMemoryRequirements);
bool PreCallValidateGetPhysicalDeviceImageFormatProperties2(VkPhysicalDevice physicalDevice,
                                                            const VkPhysicalDeviceImageFormatInfo2* pImageFormatInfo,
                                                            VkImageFormatProperties2* pImageFormatProperties);
bool PreCallValidateGetPhysicalDeviceImageFormatProperties2KHR(VkPhysicalDevice physicalDevice,
                                                               const VkPhysicalDeviceImageFormatInfo2* pImageFormatInfo,
                                                               VkImageFormatProperties2* pImageFormatProperties);
void PreCallRecordDestroyShaderModule(VkDevice device, VkShaderModule shaderModule, const VkAllocationCallbacks* pAllocator);
bool PreCallValidateDestroyPipeline(VkDevice device, VkPipeline pipeline, const VkAllocationCallbacks* pAllocator);
void PreCallRecordDestroyPipeline(VkDevice device, VkPipeline pipeline, const VkAllocationCallbacks* pAllocator);
void PreCallRecordDestroyPipelineLayout(VkDevice device, VkPipelineLayout pipelineLayout, const VkAllocationCallbacks* pAllocator);
bool PreCallValidateDestroySampler(VkDevice device, VkSampler sampler, const VkAllocationCallbacks* pAllocator);
void PreCallRecordDestroySampler(VkDevice device, VkSampler sampler, const VkAllocationCallbacks* pAllocator);
void PreCallRecordDestroyDescriptorSetLayout(VkDevice device, VkDescriptorSetLayout descriptorSetLayout,
                                             const VkAllocationCallbacks* pAllocator);
bool PreCallValidateDestroyDescriptorPool(VkDevice device, VkDescriptorPool descriptorPool,
                                          const VkAllocationCallbacks* pAllocator);
void PreCallRecordDestroyDescriptorPool(VkDevice device, VkDescriptorPool descriptorPool, const VkAllocationCallbacks* pAllocator);
bool PreCallValidateFreeCommandBuffers(VkDevice device, VkCommandPool commandPool, uint32_t commandBufferCount,
                                       const VkCommandBuffer* pCommandBuffers);
void PreCallRecordFreeCommandBuffers(VkDevice device, VkCommandPool commandPool, uint32_t commandBufferCount,
                                     const VkCommandBuffer* pCommandBuffers);
bool PreCallValidateCreateCommandPool(VkDevice device, const VkCommandPoolCreateInfo* pCreateInfo,
                                      const VkAllocationCallbacks* pAllocator, VkCommandPool* pCommandPool);
void PostCallRecordCreateCommandPool(VkDevice device, const VkCommandPoolCreateInfo* pCreateInfo,
                                     const VkAllocationCallbacks* pAllocator, VkCommandPool* pCommandPool, VkResult result);
bool PreCallValidateCreateQueryPool(VkDevice device, const VkQueryPoolCreateInfo* pCreateInfo,
                                    const VkAllocationCallbacks* pAllocator, VkQueryPool* pQueryPool);
void PostCallRecordCreateQueryPool(VkDevice device, const VkQueryPoolCreateInfo* pCreateInfo,
                                   const VkAllocationCallbacks* pAllocator, VkQueryPool* pQueryPool, VkResult result);
bool PreCallValidateDestroyCommandPool(VkDevice device, VkCommandPool commandPool, const VkAllocationCallbacks* pAllocator);
void PreCallRecordDestroyCommandPool(VkDevice device, VkCommandPool commandPool, const VkAllocationCallbacks* pAllocator);
bool PreCallValidateResetCommandPool(VkDevice device, VkCommandPool commandPool, VkCommandPoolResetFlags flags);
void PostCallRecordResetCommandPool(VkDevice device, VkCommandPool commandPool, VkCommandPoolResetFlags flags, VkResult result);
bool PreCallValidateResetFences(VkDevice device, uint32_t fenceCount, const VkFence* pFences);
void PostCallRecordResetFences(VkDevice device, uint32_t fenceCount, const VkFence* pFences, VkResult result);
bool PreCallValidateDestroyFramebuffer(VkDevice device, VkFramebuffer framebuffer, const VkAllocationCallbacks* pAllocator);
void PreCallRecordDestroyFramebuffer(VkDevice device, VkFramebuffer framebuffer, const VkAllocationCallbacks* pAllocator);
bool PreCallValidateDestroyRenderPass(VkDevice device, VkRenderPass renderPass, const VkAllocationCallbacks* pAllocator);
void PreCallRecordDestroyRenderPass(VkDevice device, VkRenderPass renderPass, const VkAllocationCallbacks* pAllocator);
void PostCallRecordCreateSampler(VkDevice device, const VkSamplerCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator,
                                 VkSampler* pSampler, VkResult result);
bool PreCallValidateCreateDescriptorSetLayout(VkDevice device, const VkDescriptorSetLayoutCreateInfo* pCreateInfo,
                                              const VkAllocationCallbacks* pAllocator, VkDescriptorSetLayout* pSetLayout);
void PostCallRecordCreateDescriptorSetLayout(VkDevice device, const VkDescriptorSetLayoutCreateInfo* pCreateInfo,
                                             const VkAllocationCallbacks* pAllocator, VkDescriptorSetLayout* pSetLayout,
                                             VkResult result);
void PostCallRecordCreateDescriptorPool(VkDevice device, const VkDescriptorPoolCreateInfo* pCreateInfo,
                                        const VkAllocationCallbacks* pAllocator, VkDescriptorPool* pDescriptorPool,
                                        VkResult result);
bool PreCallValidateResetDescriptorPool(VkDevice device, VkDescriptorPool descriptorPool, VkDescriptorPoolResetFlags flags);
void PostCallRecordResetDescriptorPool(VkDevice device, VkDescriptorPool descriptorPool, VkDescriptorPoolResetFlags flags,
                                       VkResult result);
bool PreCallValidateFreeDescriptorSets(VkDevice device, VkDescriptorPool descriptorPool, uint32_t count,
                                       const VkDescriptorSet* pDescriptorSets);
void PreCallRecordFreeDescriptorSets(VkDevice device, VkDescriptorPool descriptorPool, uint32_t count,
                                     const VkDescriptorSet* pDescriptorSets);
bool PreCallValidateUpdateDescriptorSets(VkDevice device, uint32_t descriptorWriteCount,
                                         const VkWriteDescriptorSet* pDescriptorWrites, uint32_t descriptorCopyCount,
                                         const VkCopyDescriptorSet* pDescriptorCopies);
void PreCallRecordUpdateDescriptorSets(VkDevice device, uint32_t descriptorWriteCount,
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
bool PreCallValidateCmdBindPipeline(VkCommandBuffer commandBuffer, VkPipelineBindPoint pipelineBindPoint, VkPipeline pipeline);
void PreCallRecordCmdBindPipeline(VkCommandBuffer commandBuffer, VkPipelineBindPoint pipelineBindPoint, VkPipeline pipeline);
bool PreCallValidateCmdSetViewport(VkCommandBuffer commandBuffer, uint32_t firstViewport, uint32_t viewportCount,
                                   const VkViewport* pViewports);
void PreCallRecordCmdSetViewport(VkCommandBuffer commandBuffer, uint32_t firstViewport, uint32_t viewportCount,
                                 const VkViewport* pViewports);
bool PreCallValidateCmdSetScissor(VkCommandBuffer commandBuffer, uint32_t firstScissor, uint32_t scissorCount,
                                  const VkRect2D* pScissors);
void PreCallRecordCmdSetScissor(VkCommandBuffer commandBuffer, uint32_t firstScissor, uint32_t scissorCount,
                                const VkRect2D* pScissors);
bool PreCallValidateCmdSetExclusiveScissorNV(VkCommandBuffer commandBuffer, uint32_t firstExclusiveScissor,
                                             uint32_t exclusiveScissorCount, const VkRect2D* pExclusiveScissors);
void PreCallRecordCmdSetExclusiveScissorNV(VkCommandBuffer commandBuffer, uint32_t firstExclusiveScissor,
                                           uint32_t exclusiveScissorCount, const VkRect2D* pExclusiveScissors);
bool PreCallValidateCmdBindShadingRateImageNV(VkCommandBuffer commandBuffer, VkImageView imageView, VkImageLayout imageLayout);
void PreCallRecordCmdBindShadingRateImageNV(VkCommandBuffer commandBuffer, VkImageView imageView, VkImageLayout imageLayout);
bool PreCallValidateCmdSetViewportShadingRatePaletteNV(VkCommandBuffer commandBuffer, uint32_t firstViewport,
                                                       uint32_t viewportCount, const VkShadingRatePaletteNV* pShadingRatePalettes);
void PreCallRecordCmdSetViewportShadingRatePaletteNV(VkCommandBuffer commandBuffer, uint32_t firstViewport, uint32_t viewportCount,
                                                     const VkShadingRatePaletteNV* pShadingRatePalettes);
bool PreCallValidateCmdSetLineWidth(VkCommandBuffer commandBuffer, float lineWidth);
void PreCallRecordCmdSetLineWidth(VkCommandBuffer commandBuffer, float lineWidth);
bool PreCallValidateCmdSetDepthBias(VkCommandBuffer commandBuffer, float depthBiasConstantFactor, float depthBiasClamp,
                                    float depthBiasSlopeFactor);
void PreCallRecordCmdSetDepthBias(VkCommandBuffer commandBuffer, float depthBiasConstantFactor, float depthBiasClamp,
                                  float depthBiasSlopeFactor);
bool PreCallValidateCmdSetBlendConstants(VkCommandBuffer commandBuffer, const float blendConstants[4]);
void PreCallRecordCmdSetBlendConstants(VkCommandBuffer commandBuffer, const float blendConstants[4]);
bool PreCallValidateCmdSetDepthBounds(VkCommandBuffer commandBuffer, float minDepthBounds, float maxDepthBounds);
void PreCallRecordCmdSetDepthBounds(VkCommandBuffer commandBuffer, float minDepthBounds, float maxDepthBounds);
bool PreCallValidateCmdSetStencilCompareMask(VkCommandBuffer commandBuffer, VkStencilFaceFlags faceMask, uint32_t compareMask);
void PreCallRecordCmdSetStencilCompareMask(VkCommandBuffer commandBuffer, VkStencilFaceFlags faceMask, uint32_t compareMask);
bool PreCallValidateCmdSetStencilWriteMask(VkCommandBuffer commandBuffer, VkStencilFaceFlags faceMask, uint32_t writeMask);
void PreCallRecordCmdSetStencilWriteMask(VkCommandBuffer commandBuffer, VkStencilFaceFlags faceMask, uint32_t writeMask);
bool PreCallValidateCmdSetStencilReference(VkCommandBuffer commandBuffer, VkStencilFaceFlags faceMask, uint32_t reference);
void PreCallRecordCmdSetStencilReference(VkCommandBuffer commandBuffer, VkStencilFaceFlags faceMask, uint32_t reference);
bool PreCallValidateCmdBindDescriptorSets(VkCommandBuffer commandBuffer, VkPipelineBindPoint pipelineBindPoint,
                                          VkPipelineLayout layout, uint32_t firstSet, uint32_t setCount,
                                          const VkDescriptorSet* pDescriptorSets, uint32_t dynamicOffsetCount,
                                          const uint32_t* pDynamicOffsets);
void PreCallRecordCmdBindDescriptorSets(VkCommandBuffer commandBuffer, VkPipelineBindPoint pipelineBindPoint,
                                        VkPipelineLayout layout, uint32_t firstSet, uint32_t setCount,
                                        const VkDescriptorSet* pDescriptorSets, uint32_t dynamicOffsetCount,
                                        const uint32_t* pDynamicOffsets);
bool PreCallValidateCmdPushDescriptorSetKHR(VkCommandBuffer commandBuffer, VkPipelineBindPoint pipelineBindPoint,
                                            VkPipelineLayout layout, uint32_t set, uint32_t descriptorWriteCount,
                                            const VkWriteDescriptorSet* pDescriptorWrites);
void PreCallRecordCmdPushDescriptorSetKHR(VkCommandBuffer commandBuffer, VkPipelineBindPoint pipelineBindPoint,
                                          VkPipelineLayout layout, uint32_t set, uint32_t descriptorWriteCount,
                                          const VkWriteDescriptorSet* pDescriptorWrites);
bool PreCallValidateCmdBindIndexBuffer(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset, VkIndexType indexType);
void PreCallRecordCmdBindIndexBuffer(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset, VkIndexType indexType);
bool PreCallValidateCmdBindVertexBuffers(VkCommandBuffer commandBuffer, uint32_t firstBinding, uint32_t bindingCount,
                                         const VkBuffer* pBuffers, const VkDeviceSize* pOffsets);
void PreCallRecordCmdBindVertexBuffers(VkCommandBuffer commandBuffer, uint32_t firstBinding, uint32_t bindingCount,
                                       const VkBuffer* pBuffers, const VkDeviceSize* pOffsets);
bool PreCallValidateCmdDraw(VkCommandBuffer commandBuffer, uint32_t vertexCount, uint32_t instanceCount, uint32_t firstVertex,
                            uint32_t firstInstance);
void PreCallRecordCmdDraw(VkCommandBuffer commandBuffer, uint32_t vertexCount, uint32_t instanceCount, uint32_t firstVertex,
                          uint32_t firstInstance);
void PostCallRecordCmdDraw(VkCommandBuffer commandBuffer, uint32_t vertexCount, uint32_t instanceCount, uint32_t firstVertex,
                           uint32_t firstInstance);
bool PreCallValidateCmdDrawIndexed(VkCommandBuffer commandBuffer, uint32_t indexCount, uint32_t instanceCount, uint32_t firstIndex,
                                   int32_t vertexOffset, uint32_t firstInstance);
void PreCallRecordCmdDrawIndexed(VkCommandBuffer commandBuffer, uint32_t indexCount, uint32_t instanceCount, uint32_t firstIndex,
                                 int32_t vertexOffset, uint32_t firstInstance);
void PostCallRecordCmdDrawIndexed(VkCommandBuffer commandBuffer, uint32_t indexCount, uint32_t instanceCount, uint32_t firstIndex,
                                  int32_t vertexOffset, uint32_t firstInstance);
bool PreCallValidateCmdDrawIndexedIndirect(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset, uint32_t count,
                                           uint32_t stride);
void PreCallRecordCmdDrawIndexedIndirect(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset, uint32_t count,
                                         uint32_t stride);
void PostCallRecordCmdDrawIndexedIndirect(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset, uint32_t count,
                                          uint32_t stride);
bool PreCallValidateCmdDrawIndexedIndirectCountKHR(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                   VkBuffer countBuffer, VkDeviceSize countBufferOffset, uint32_t maxDrawCount,
                                                   uint32_t stride);
bool PreCallValidateCmdDispatch(VkCommandBuffer commandBuffer, uint32_t x, uint32_t y, uint32_t z);
void PreCallRecordCmdDispatch(VkCommandBuffer commandBuffer, uint32_t x, uint32_t y, uint32_t z);
void PostCallRecordCmdDispatch(VkCommandBuffer commandBuffer, uint32_t x, uint32_t y, uint32_t z);
bool PreCallValidateCmdDispatchIndirect(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset);
void PreCallRecordCmdDispatchIndirect(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset);
void PostCallRecordCmdDispatchIndirect(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset);
bool PreCallValidateCmdDrawIndirect(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset, uint32_t count,
                                    uint32_t stride);
void PreCallRecordCmdDrawIndirect(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset, uint32_t count,
                                  uint32_t stride);
void PostCallRecordCmdDrawIndirect(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset, uint32_t count,
                                   uint32_t stride);
bool PreCallValidateCmdSetEvent(VkCommandBuffer commandBuffer, VkEvent event, VkPipelineStageFlags stageMask);
void PreCallRecordCmdSetEvent(VkCommandBuffer commandBuffer, VkEvent event, VkPipelineStageFlags stageMask);
bool PreCallValidateCmdResetEvent(VkCommandBuffer commandBuffer, VkEvent event, VkPipelineStageFlags stageMask);
void PreCallRecordCmdResetEvent(VkCommandBuffer commandBuffer, VkEvent event, VkPipelineStageFlags stageMask);
bool PreCallValidateCmdWaitEvents(VkCommandBuffer commandBuffer, uint32_t eventCount, const VkEvent* pEvents,
                                  VkPipelineStageFlags sourceStageMask, VkPipelineStageFlags dstStageMask,
                                  uint32_t memoryBarrierCount, const VkMemoryBarrier* pMemoryBarriers,
                                  uint32_t bufferMemoryBarrierCount, const VkBufferMemoryBarrier* pBufferMemoryBarriers,
                                  uint32_t imageMemoryBarrierCount, const VkImageMemoryBarrier* pImageMemoryBarriers);
void PreCallRecordCmdWaitEvents(VkCommandBuffer commandBuffer, uint32_t eventCount, const VkEvent* pEvents,
                                VkPipelineStageFlags sourceStageMask, VkPipelineStageFlags dstStageMask,
                                uint32_t memoryBarrierCount, const VkMemoryBarrier* pMemoryBarriers,
                                uint32_t bufferMemoryBarrierCount, const VkBufferMemoryBarrier* pBufferMemoryBarriers,
                                uint32_t imageMemoryBarrierCount, const VkImageMemoryBarrier* pImageMemoryBarriers);
void PostCallRecordCmdWaitEvents(VkCommandBuffer commandBuffer, uint32_t eventCount, const VkEvent* pEvents,
                                 VkPipelineStageFlags sourceStageMask, VkPipelineStageFlags dstStageMask,
                                 uint32_t memoryBarrierCount, const VkMemoryBarrier* pMemoryBarriers,
                                 uint32_t bufferMemoryBarrierCount, const VkBufferMemoryBarrier* pBufferMemoryBarriers,
                                 uint32_t imageMemoryBarrierCount, const VkImageMemoryBarrier* pImageMemoryBarriers);
bool PreCallValidateCmdPipelineBarrier(VkCommandBuffer commandBuffer, VkPipelineStageFlags srcStageMask,
                                       VkPipelineStageFlags dstStageMask, VkDependencyFlags dependencyFlags,
                                       uint32_t memoryBarrierCount, const VkMemoryBarrier* pMemoryBarriers,
                                       uint32_t bufferMemoryBarrierCount, const VkBufferMemoryBarrier* pBufferMemoryBarriers,
                                       uint32_t imageMemoryBarrierCount, const VkImageMemoryBarrier* pImageMemoryBarriers);
void PreCallRecordCmdPipelineBarrier(VkCommandBuffer commandBuffer, VkPipelineStageFlags srcStageMask,
                                     VkPipelineStageFlags dstStageMask, VkDependencyFlags dependencyFlags,
                                     uint32_t memoryBarrierCount, const VkMemoryBarrier* pMemoryBarriers,
                                     uint32_t bufferMemoryBarrierCount, const VkBufferMemoryBarrier* pBufferMemoryBarriers,
                                     uint32_t imageMemoryBarrierCount, const VkImageMemoryBarrier* pImageMemoryBarriers);
bool PreCallValidateCmdBeginQuery(VkCommandBuffer commandBuffer, VkQueryPool queryPool, uint32_t slot, VkFlags flags);
void PostCallRecordCmdBeginQuery(VkCommandBuffer commandBuffer, VkQueryPool queryPool, uint32_t slot, VkFlags flags);
bool PreCallValidateCmdEndQuery(VkCommandBuffer commandBuffer, VkQueryPool queryPool, uint32_t slot);
void PostCallRecordCmdEndQuery(VkCommandBuffer commandBuffer, VkQueryPool queryPool, uint32_t slot);
bool PreCallValidateCmdResetQueryPool(VkCommandBuffer commandBuffer, VkQueryPool queryPool, uint32_t firstQuery,
                                      uint32_t queryCount);
void PostCallRecordCmdResetQueryPool(VkCommandBuffer commandBuffer, VkQueryPool queryPool, uint32_t firstQuery,
                                     uint32_t queryCount);
bool PreCallValidateCmdCopyQueryPoolResults(VkCommandBuffer commandBuffer, VkQueryPool queryPool, uint32_t firstQuery,
                                            uint32_t queryCount, VkBuffer dstBuffer, VkDeviceSize dstOffset, VkDeviceSize stride,
                                            VkQueryResultFlags flags);
void PostCallRecordCmdCopyQueryPoolResults(VkCommandBuffer commandBuffer, VkQueryPool queryPool, uint32_t firstQuery,
                                           uint32_t queryCount, VkBuffer dstBuffer, VkDeviceSize dstOffset, VkDeviceSize stride,
                                           VkQueryResultFlags flags);
bool PreCallValidateCmdPushConstants(VkCommandBuffer commandBuffer, VkPipelineLayout layout, VkShaderStageFlags stageFlags,
                                     uint32_t offset, uint32_t size, const void* pValues);
bool PreCallValidateCmdWriteTimestamp(VkCommandBuffer commandBuffer, VkPipelineStageFlagBits pipelineStage, VkQueryPool queryPool,
                                      uint32_t slot);
void PostCallRecordCmdWriteTimestamp(VkCommandBuffer commandBuffer, VkPipelineStageFlagBits pipelineStage, VkQueryPool queryPool,
                                     uint32_t slot);
bool PreCallValidateCreateFramebuffer(VkDevice device, const VkFramebufferCreateInfo* pCreateInfo,
                                      const VkAllocationCallbacks* pAllocator, VkFramebuffer* pFramebuffer);
void PostCallRecordCreateFramebuffer(VkDevice device, const VkFramebufferCreateInfo* pCreateInfo,
                                     const VkAllocationCallbacks* pAllocator, VkFramebuffer* pFramebuffer, VkResult result);
bool PreCallValidateCreateRenderPass(VkDevice device, const VkRenderPassCreateInfo* pCreateInfo,
                                     const VkAllocationCallbacks* pAllocator, VkRenderPass* pRenderPass);
void PostCallRecordCreateRenderPass(VkDevice device, const VkRenderPassCreateInfo* pCreateInfo,
                                    const VkAllocationCallbacks* pAllocator, VkRenderPass* pRenderPass, VkResult result);
bool PreCallValidateCreateRenderPass2KHR(VkDevice device, const VkRenderPassCreateInfo2KHR* pCreateInfo,
                                         const VkAllocationCallbacks* pAllocator, VkRenderPass* pRenderPass);
void PostCallRecordCreateRenderPass2KHR(VkDevice device, const VkRenderPassCreateInfo2KHR* pCreateInfo,
                                        const VkAllocationCallbacks* pAllocator, VkRenderPass* pRenderPass, VkResult result);
bool PreCallValidateCmdBeginRenderPass(VkCommandBuffer commandBuffer, const VkRenderPassBeginInfo* pRenderPassBegin,
                                       VkSubpassContents contents);
void PreCallRecordCmdBeginRenderPass(VkCommandBuffer commandBuffer, const VkRenderPassBeginInfo* pRenderPassBegin,
                                     VkSubpassContents contents);
bool PreCallValidateCmdBeginRenderPass2KHR(VkCommandBuffer commandBuffer, const VkRenderPassBeginInfo* pRenderPassBegin,
                                           const VkSubpassBeginInfoKHR* pSubpassBeginInfo);
void PreCallRecordCmdBeginRenderPass2KHR(VkCommandBuffer commandBuffer, const VkRenderPassBeginInfo* pRenderPassBegin,
                                         const VkSubpassBeginInfoKHR* pSubpassBeginInfo);
bool PreCallValidateCmdNextSubpass(VkCommandBuffer commandBuffer, VkSubpassContents contents);
void PostCallRecordCmdNextSubpass(VkCommandBuffer commandBuffer, VkSubpassContents contents);
bool PreCallValidateCmdNextSubpass2KHR(VkCommandBuffer commandBuffer, const VkSubpassBeginInfoKHR* pSubpassBeginInfo,
                                       const VkSubpassEndInfoKHR* pSubpassEndInfo);
void PostCallRecordCmdNextSubpass2KHR(VkCommandBuffer commandBuffer, const VkSubpassBeginInfoKHR* pSubpassBeginInfo,
                                      const VkSubpassEndInfoKHR* pSubpassEndInfo);
bool PreCallValidateCmdEndRenderPass(VkCommandBuffer commandBuffer);
void PostCallRecordCmdEndRenderPass(VkCommandBuffer commandBuffer);
bool PreCallValidateCmdEndRenderPass2KHR(VkCommandBuffer commandBuffer, const VkSubpassEndInfoKHR* pSubpassEndInfo);
void PostCallRecordCmdEndRenderPass2KHR(VkCommandBuffer commandBuffer, const VkSubpassEndInfoKHR* pSubpassEndInfo);
bool PreCallValidateCmdExecuteCommands(VkCommandBuffer commandBuffer, uint32_t commandBuffersCount,
                                       const VkCommandBuffer* pCommandBuffers);
void PreCallRecordCmdExecuteCommands(VkCommandBuffer commandBuffer, uint32_t commandBuffersCount,
                                     const VkCommandBuffer* pCommandBuffers);
bool PreCallValidateMapMemory(VkDevice device, VkDeviceMemory mem, VkDeviceSize offset, VkDeviceSize size, VkFlags flags,
                              void** ppData);
void PostCallRecordMapMemory(VkDevice device, VkDeviceMemory mem, VkDeviceSize offset, VkDeviceSize size, VkFlags flags,
                             void** ppData, VkResult result);
bool PreCallValidateUnmapMemory(VkDevice device, VkDeviceMemory mem);
void PreCallRecordUnmapMemory(VkDevice device, VkDeviceMemory mem);
bool PreCallValidateFlushMappedMemoryRanges(VkDevice device, uint32_t memRangeCount, const VkMappedMemoryRange* pMemRanges);
bool PreCallValidateInvalidateMappedMemoryRanges(VkDevice device, uint32_t memRangeCount, const VkMappedMemoryRange* pMemRanges);
void PostCallRecordInvalidateMappedMemoryRanges(VkDevice device, uint32_t memRangeCount, const VkMappedMemoryRange* pMemRanges,
                                                VkResult result);
bool PreCallValidateGetDeviceMemoryCommitment(VkDevice device, VkDeviceMemory mem, VkDeviceSize* pCommittedMem);
bool PreCallValidateBindImageMemory(VkDevice device, VkImage image, VkDeviceMemory mem, VkDeviceSize memoryOffset);
void PostCallRecordBindImageMemory(VkDevice device, VkImage image, VkDeviceMemory mem, VkDeviceSize memoryOffset, VkResult result);
bool PreCallValidateBindImageMemory2(VkDevice device, uint32_t bindInfoCount, const VkBindImageMemoryInfoKHR* pBindInfos);
void PostCallRecordBindImageMemory2(VkDevice device, uint32_t bindInfoCount, const VkBindImageMemoryInfoKHR* pBindInfos,
                                    VkResult result);
bool PreCallValidateBindImageMemory2KHR(VkDevice device, uint32_t bindInfoCount, const VkBindImageMemoryInfoKHR* pBindInfos);
void PostCallRecordBindImageMemory2KHR(VkDevice device, uint32_t bindInfoCount, const VkBindImageMemoryInfoKHR* pBindInfos,
                                       VkResult result);
bool PreCallValidateSetEvent(VkDevice device, VkEvent event);
void PreCallRecordSetEvent(VkDevice device, VkEvent event);
bool PreCallValidateQueueBindSparse(VkQueue queue, uint32_t bindInfoCount, const VkBindSparseInfo* pBindInfo, VkFence fence);
void PostCallRecordQueueBindSparse(VkQueue queue, uint32_t bindInfoCount, const VkBindSparseInfo* pBindInfo, VkFence fence,
                                   VkResult result);
void PostCallRecordCreateSemaphore(VkDevice device, const VkSemaphoreCreateInfo* pCreateInfo,
                                   const VkAllocationCallbacks* pAllocator, VkSemaphore* pSemaphore, VkResult result);
bool PreCallValidateImportSemaphoreFdKHR(VkDevice device, const VkImportSemaphoreFdInfoKHR* pImportSemaphoreFdInfo);
void PostCallRecordImportSemaphoreFdKHR(VkDevice device, const VkImportSemaphoreFdInfoKHR* pImportSemaphoreFdInfo, VkResult result);
#ifdef VK_USE_PLATFORM_WIN32_KHR
void PostCallRecordImportSemaphoreWin32HandleKHR(VkDevice device,
                                                 const VkImportSemaphoreWin32HandleInfoKHR* pImportSemaphoreWin32HandleInfo,
                                                 VkResult result);
bool PreCallValidateImportSemaphoreWin32HandleKHR(VkDevice device,
                                                  const VkImportSemaphoreWin32HandleInfoKHR* pImportSemaphoreWin32HandleInfo);
bool PreCallValidateImportFenceWin32HandleKHR(VkDevice device, const VkImportFenceWin32HandleInfoKHR* pImportFenceWin32HandleInfo);
void PostCallRecordImportFenceWin32HandleKHR(VkDevice device, const VkImportFenceWin32HandleInfoKHR* pImportFenceWin32HandleInfo,
                                             VkResult result);
void PostCallRecordGetSemaphoreWin32HandleKHR(VkDevice device, const VkSemaphoreGetWin32HandleInfoKHR* pGetWin32HandleInfo,
                                              HANDLE* pHandle, VkResult result);
void PostCallRecordGetFenceWin32HandleKHR(VkDevice device, const VkFenceGetWin32HandleInfoKHR* pGetWin32HandleInfo, HANDLE* pHandle,
                                          VkResult result);
#endif  // VK_USE_PLATFORM_WIN32_KHR
bool PreCallValidateImportFenceFdKHR(VkDevice device, const VkImportFenceFdInfoKHR* pImportFenceFdInfo);
void PostCallRecordImportFenceFdKHR(VkDevice device, const VkImportFenceFdInfoKHR* pImportFenceFdInfo, VkResult result);

void PostCallRecordGetSemaphoreFdKHR(VkDevice device, const VkSemaphoreGetFdInfoKHR* pGetFdInfo, int* pFd, VkResult result);
void PostCallRecordGetFenceFdKHR(VkDevice device, const VkFenceGetFdInfoKHR* pGetFdInfo, int* pFd, VkResult result);
void PostCallRecordCreateEvent(VkDevice device, const VkEventCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator,
                               VkEvent* pEvent, VkResult result);
bool PreCallValidateCreateSwapchainKHR(VkDevice device, const VkSwapchainCreateInfoKHR* pCreateInfo,
                                       const VkAllocationCallbacks* pAllocator, VkSwapchainKHR* pSwapchain);
void PostCallRecordCreateSwapchainKHR(VkDevice device, const VkSwapchainCreateInfoKHR* pCreateInfo,
                                      const VkAllocationCallbacks* pAllocator, VkSwapchainKHR* pSwapchain, VkResult result);
void PreCallRecordDestroySwapchainKHR(VkDevice device, VkSwapchainKHR swapchain, const VkAllocationCallbacks* pAllocator);
bool PreCallValidateGetSwapchainImagesKHR(VkDevice device, VkSwapchainKHR swapchain, uint32_t* pSwapchainImageCount,
                                          VkImage* pSwapchainImages);
void PostCallRecordGetSwapchainImagesKHR(VkDevice device, VkSwapchainKHR swapchain, uint32_t* pSwapchainImageCount,
                                         VkImage* pSwapchainImages, VkResult result);
bool PreCallValidateQueuePresentKHR(VkQueue queue, const VkPresentInfoKHR* pPresentInfo);
void PostCallRecordQueuePresentKHR(VkQueue queue, const VkPresentInfoKHR* pPresentInfo, VkResult result);
bool PreCallValidateCreateSharedSwapchainsKHR(VkDevice device, uint32_t swapchainCount,
                                              const VkSwapchainCreateInfoKHR* pCreateInfos, const VkAllocationCallbacks* pAllocator,
                                              VkSwapchainKHR* pSwapchains);
void PostCallRecordCreateSharedSwapchainsKHR(VkDevice device, uint32_t swapchainCount, const VkSwapchainCreateInfoKHR* pCreateInfos,
                                             const VkAllocationCallbacks* pAllocator, VkSwapchainKHR* pSwapchains, VkResult result);
bool PreCallValidateAcquireNextImageKHR(VkDevice device, VkSwapchainKHR swapchain, uint64_t timeout, VkSemaphore semaphore,
                                        VkFence fence, uint32_t* pImageIndex);
bool PreCallValidateAcquireNextImage2KHR(VkDevice device, const VkAcquireNextImageInfoKHR* pAcquireInfo, uint32_t* pImageIndex);
void PostCallRecordAcquireNextImageKHR(VkDevice device, VkSwapchainKHR swapchain, uint64_t timeout, VkSemaphore semaphore,
                                       VkFence fence, uint32_t* pImageIndex, VkResult result);
void PostCallRecordAcquireNextImage2KHR(VkDevice device, const VkAcquireNextImageInfoKHR* pAcquireInfo, uint32_t* pImageIndex,
                                        VkResult result);
void PostCallRecordEnumeratePhysicalDevices(VkInstance instance, uint32_t* pPhysicalDeviceCount, VkPhysicalDevice* pPhysicalDevices,
                                            VkResult result);
bool PreCallValidateGetPhysicalDeviceQueueFamilyProperties(VkPhysicalDevice physicalDevice, uint32_t* pQueueFamilyPropertyCount,
                                                           VkQueueFamilyProperties* pQueueFamilyProperties);
void PostCallRecordGetPhysicalDeviceQueueFamilyProperties(VkPhysicalDevice physicalDevice, uint32_t* pQueueFamilyPropertyCount,
                                                          VkQueueFamilyProperties* pQueueFamilyProperties);
bool PreCallValidateGetPhysicalDeviceQueueFamilyProperties2(VkPhysicalDevice physicalDevice, uint32_t* pQueueFamilyPropertyCount,
                                                            VkQueueFamilyProperties2KHR* pQueueFamilyProperties);
void PostCallRecordGetPhysicalDeviceQueueFamilyProperties2(VkPhysicalDevice physicalDevice, uint32_t* pQueueFamilyPropertyCount,
                                                           VkQueueFamilyProperties2KHR* pQueueFamilyProperties);
bool PreCallValidateGetPhysicalDeviceQueueFamilyProperties2KHR(VkPhysicalDevice physicalDevice, uint32_t* pQueueFamilyPropertyCount,
                                                               VkQueueFamilyProperties2KHR* pQueueFamilyProperties);
void PostCallRecordGetPhysicalDeviceQueueFamilyProperties2KHR(VkPhysicalDevice physicalDevice, uint32_t* pQueueFamilyPropertyCount,
                                                              VkQueueFamilyProperties2KHR* pQueueFamilyProperties);
bool PreCallValidateDestroySurfaceKHR(VkInstance instance, VkSurfaceKHR surface, const VkAllocationCallbacks* pAllocator);
void PreCallRecordValidateDestroySurfaceKHR(VkInstance instance, VkSurfaceKHR surface, const VkAllocationCallbacks* pAllocator);
void PostCallRecordGetPhysicalDeviceSurfaceCapabilitiesKHR(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface,
                                                           VkSurfaceCapabilitiesKHR* pSurfaceCapabilities, VkResult result);
void PostCallRecordGetPhysicalDeviceSurfaceCapabilities2KHR(VkPhysicalDevice physicalDevice,
                                                            const VkPhysicalDeviceSurfaceInfo2KHR* pSurfaceInfo,
                                                            VkSurfaceCapabilities2KHR* pSurfaceCapabilities, VkResult result);
void PostCallRecordGetPhysicalDeviceSurfaceCapabilities2EXT(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface,
                                                            VkSurfaceCapabilities2EXT* pSurfaceCapabilities, VkResult result);
bool PreCallValidateGetPhysicalDeviceSurfaceSupportKHR(VkPhysicalDevice physicalDevice, uint32_t queueFamilyIndex,
                                                       VkSurfaceKHR surface, VkBool32* pSupported);
void PostCallRecordGetPhysicalDeviceSurfaceSupportKHR(VkPhysicalDevice physicalDevice, uint32_t queueFamilyIndex,
                                                      VkSurfaceKHR surface, VkBool32* pSupported, VkResult result);
void PostCallRecordGetPhysicalDeviceSurfacePresentModesKHR(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface,
                                                           uint32_t* pPresentModeCount, VkPresentModeKHR* pPresentModes,
                                                           VkResult result);
bool PreCallValidateGetPhysicalDeviceSurfaceFormatsKHR(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface,
                                                       uint32_t* pSurfaceFormatCount, VkSurfaceFormatKHR* pSurfaceFormats);
void PostCallRecordGetPhysicalDeviceSurfaceFormatsKHR(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface,
                                                      uint32_t* pSurfaceFormatCount, VkSurfaceFormatKHR* pSurfaceFormats,
                                                      VkResult result);
void PostCallRecordGetPhysicalDeviceSurfaceFormats2KHR(VkPhysicalDevice physicalDevice,
                                                       const VkPhysicalDeviceSurfaceInfo2KHR* pSurfaceInfo,
                                                       uint32_t* pSurfaceFormatCount, VkSurfaceFormat2KHR* pSurfaceFormats,
                                                       VkResult result);
void PostCallRecordCreateDisplayPlaneSurfaceKHR(VkInstance instance, const VkDisplaySurfaceCreateInfoKHR* pCreateInfo,
                                                const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface, VkResult result);
void PreCallRecordQueueBeginDebugUtilsLabelEXT(VkQueue queue, const VkDebugUtilsLabelEXT* pLabelInfo);
void PostCallRecordQueueEndDebugUtilsLabelEXT(VkQueue queue);
void PreCallRecordQueueInsertDebugUtilsLabelEXT(VkQueue queue, const VkDebugUtilsLabelEXT* pLabelInfo);
void PreCallRecordCmdBeginDebugUtilsLabelEXT(VkCommandBuffer commandBuffer, const VkDebugUtilsLabelEXT* pLabelInfo);
void PostCallRecordCmdEndDebugUtilsLabelEXT(VkCommandBuffer commandBuffer);
void PreCallRecordCmdInsertDebugUtilsLabelEXT(VkCommandBuffer commandBuffer, const VkDebugUtilsLabelEXT* pLabelInfo);
void PostCallRecordCreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
                                                const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pMessenger,
                                                VkResult result);
void PostCallRecordDestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT messenger,
                                                 const VkAllocationCallbacks* pAllocator);
void PostCallRecordCreateDebugReportCallbackEXT(VkInstance instance, const VkDebugReportCallbackCreateInfoEXT* pCreateInfo,
                                                const VkAllocationCallbacks* pAllocator, VkDebugReportCallbackEXT* pMsgCallback,
                                                VkResult result);
void PostCallDestroyDebugReportCallbackEXT(VkInstance instance, VkDebugReportCallbackEXT msgCallback,
                                           const VkAllocationCallbacks* pAllocator);
void PostCallRecordEnumeratePhysicalDeviceGroups(VkInstance instance, uint32_t* pPhysicalDeviceGroupCount,
                                                 VkPhysicalDeviceGroupPropertiesKHR* pPhysicalDeviceGroupProperties,
                                                 VkResult result);
void PostCallRecordEnumeratePhysicalDeviceGroupsKHR(VkInstance instance, uint32_t* pPhysicalDeviceGroupCount,
                                                    VkPhysicalDeviceGroupPropertiesKHR* pPhysicalDeviceGroupProperties,
                                                    VkResult result);
bool PreCallValidateCreateDescriptorUpdateTemplate(VkDevice device, const VkDescriptorUpdateTemplateCreateInfoKHR* pCreateInfo,
                                                   const VkAllocationCallbacks* pAllocator,
                                                   VkDescriptorUpdateTemplateKHR* pDescriptorUpdateTemplate);
void PostCallRecordCreateDescriptorUpdateTemplate(VkDevice device, const VkDescriptorUpdateTemplateCreateInfoKHR* pCreateInfo,
                                                  const VkAllocationCallbacks* pAllocator,
                                                  VkDescriptorUpdateTemplateKHR* pDescriptorUpdateTemplate, VkResult result);
bool PreCallValidateCreateDescriptorUpdateTemplateKHR(VkDevice device, const VkDescriptorUpdateTemplateCreateInfoKHR* pCreateInfo,
                                                      const VkAllocationCallbacks* pAllocator,
                                                      VkDescriptorUpdateTemplateKHR* pDescriptorUpdateTemplate);
void PostCallRecordCreateDescriptorUpdateTemplateKHR(VkDevice device, const VkDescriptorUpdateTemplateCreateInfoKHR* pCreateInfo,
                                                     const VkAllocationCallbacks* pAllocator,
                                                     VkDescriptorUpdateTemplateKHR* pDescriptorUpdateTemplate, VkResult result);
void PreCallRecordDestroyDescriptorUpdateTemplate(VkDevice device, VkDescriptorUpdateTemplateKHR descriptorUpdateTemplate,
                                                  const VkAllocationCallbacks* pAllocator);
void PreCallRecordDestroyDescriptorUpdateTemplateKHR(VkDevice device, VkDescriptorUpdateTemplateKHR descriptorUpdateTemplate,
                                                     const VkAllocationCallbacks* pAllocator);
bool PreCallValidateUpdateDescriptorSetWithTemplate(VkDevice device, VkDescriptorSet descriptorSet,
                                                    VkDescriptorUpdateTemplateKHR descriptorUpdateTemplate, const void* pData);
void PreCallRecordUpdateDescriptorSetWithTemplate(VkDevice device, VkDescriptorSet descriptorSet,
                                                  VkDescriptorUpdateTemplateKHR descriptorUpdateTemplate, const void* pData);
bool PreCallValidateUpdateDescriptorSetWithTemplateKHR(VkDevice device, VkDescriptorSet descriptorSet,
                                                       VkDescriptorUpdateTemplateKHR descriptorUpdateTemplate, const void* pData);
void PreCallRecordUpdateDescriptorSetWithTemplateKHR(VkDevice device, VkDescriptorSet descriptorSet,
                                                     VkDescriptorUpdateTemplateKHR descriptorUpdateTemplate, const void* pData);

bool PreCallValidateCmdPushDescriptorSetWithTemplateKHR(VkCommandBuffer commandBuffer,
                                                        VkDescriptorUpdateTemplateKHR descriptorUpdateTemplate,
                                                        VkPipelineLayout layout, uint32_t set, const void* pData);
void PreCallRecordCmdPushDescriptorSetWithTemplateKHR(VkCommandBuffer commandBuffer,
                                                      VkDescriptorUpdateTemplateKHR descriptorUpdateTemplate,
                                                      VkPipelineLayout layout, uint32_t set, const void* pData);
void PostCallRecordGetPhysicalDeviceDisplayPlanePropertiesKHR(VkPhysicalDevice physicalDevice, uint32_t* pPropertyCount,
                                                              VkDisplayPlanePropertiesKHR* pProperties, VkResult result);
void PostCallRecordGetPhysicalDeviceDisplayPlaneProperties2KHR(VkPhysicalDevice physicalDevice, uint32_t* pPropertyCount,
                                                               VkDisplayPlaneProperties2KHR* pProperties, VkResult result);
bool PreCallValidateGetDisplayPlaneSupportedDisplaysKHR(VkPhysicalDevice physicalDevice, uint32_t planeIndex,
                                                        uint32_t* pDisplayCount, VkDisplayKHR* pDisplays);
bool PreCallValidateGetDisplayPlaneCapabilitiesKHR(VkPhysicalDevice physicalDevice, VkDisplayModeKHR mode, uint32_t planeIndex,
                                                   VkDisplayPlaneCapabilitiesKHR* pCapabilities);
bool PreCallValidateGetDisplayPlaneCapabilities2KHR(VkPhysicalDevice physicalDevice,
                                                    const VkDisplayPlaneInfo2KHR* pDisplayPlaneInfo,
                                                    VkDisplayPlaneCapabilities2KHR* pCapabilities);
bool PreCallValidateCmdDebugMarkerEndEXT(VkCommandBuffer commandBuffer);
bool PreCallValidateCmdSetDiscardRectangleEXT(VkCommandBuffer commandBuffer, uint32_t firstDiscardRectangle,
                                              uint32_t discardRectangleCount, const VkRect2D* pDiscardRectangles);
bool PreCallValidateCmdSetSampleLocationsEXT(VkCommandBuffer commandBuffer, const VkSampleLocationsInfoEXT* pSampleLocationsInfo);
bool PreCallValidateCmdDrawIndirectCountKHR(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                            VkBuffer countBuffer, VkDeviceSize countBufferOffset, uint32_t maxDrawCount,
                                            uint32_t stride);
void PreCallRecordCmdDrawIndirectCountKHR(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset, VkBuffer countBuffer,
                                          VkDeviceSize countBufferOffset, uint32_t maxDrawCount, uint32_t stride);
void PreCallRecordCmdDrawIndexedIndirectCountKHR(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                 VkBuffer countBuffer, VkDeviceSize countBufferOffset, uint32_t maxDrawCount,
                                                 uint32_t stride);
bool PreCallValidateCmdDrawMeshTasksNV(VkCommandBuffer commandBuffer, uint32_t taskCount, uint32_t firstTask);
void PreCallRecordCmdDrawMeshTasksNV(VkCommandBuffer commandBuffer, uint32_t taskCount, uint32_t firstTask);
bool PreCallValidateCmdDrawMeshTasksIndirectNV(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                               uint32_t drawCount, uint32_t stride);
void PreCallRecordCmdDrawMeshTasksIndirectNV(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                             uint32_t drawCount, uint32_t stride);
bool PreCallValidateCmdDrawMeshTasksIndirectCountNV(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                    VkBuffer countBuffer, VkDeviceSize countBufferOffset, uint32_t maxDrawCount,
                                                    uint32_t stride);
void PreCallRecordCmdDrawMeshTasksIndirectCountNV(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                  VkBuffer countBuffer, VkDeviceSize countBufferOffset, uint32_t maxDrawCount,
                                                  uint32_t stride);
void PostCallRecordDestroySamplerYcbcrConversion(VkDevice device, VkSamplerYcbcrConversion ycbcrConversion,
                                                 const VkAllocationCallbacks* pAllocator);
void PostCallRecordDestroySamplerYcbcrConversionKHR(VkDevice device, VkSamplerYcbcrConversion ycbcrConversion,
                                                    const VkAllocationCallbacks* pAllocator);
bool PreCallValidateGetBufferDeviceAddressEXT(VkDevice device, const VkBufferDeviceAddressInfoEXT* pInfo);
#ifdef VK_USE_PLATFORM_ANDROID_KHR
bool PreCallValidateGetAndroidHardwareBufferProperties(VkDevice device, const struct AHardwareBuffer* buffer,
                                                       VkAndroidHardwareBufferPropertiesANDROID* pProperties);
void PostCallRecordGetAndroidHardwareBufferProperties(VkDevice device, const struct AHardwareBuffer* buffer,
                                                      VkAndroidHardwareBufferPropertiesANDROID* pProperties, VkResult result);
bool PreCallValidateGetMemoryAndroidHardwareBuffer(VkDevice device, const VkMemoryGetAndroidHardwareBufferInfoANDROID* pInfo,
                                                   struct AHardwareBuffer** pBuffer);
void PostCallRecordCreateAndroidSurfaceKHR(VkInstance instance, const VkAndroidSurfaceCreateInfoKHR* pCreateInfo,
                                           const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface, VkResult result);
#endif  // VK_USE_PLATFORM_ANDROID_KHR
#ifdef VK_USE_PLATFORM_IOS_MVK
void PostCallRecordCreateIOSSurfaceMVK(VkInstance instance, const VkIOSSurfaceCreateInfoMVK* pCreateInfo,
                                       const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface, VkResult result);
#endif  // VK_USE_PLATFORM_IOS_MVK
#ifdef VK_USE_PLATFORM_MACOS_MVK
void PostCallRecordCreateMacOSSurfaceMVK(VkInstance instance, const VkMacOSSurfaceCreateInfoMVK* pCreateInfo,
                                         const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface, VkResult result);
#endif  // VK_USE_PLATFORM_MACOS_MVK
#ifdef VK_USE_PLATFORM_WAYLAND_KHR
bool PreCallValidateGetPhysicalDeviceWaylandPresentationSupportKHR(VkPhysicalDevice physicalDevice, uint32_t queueFamilyIndex,
                                                                   struct wl_display* display);
void PostCallRecordCreateWaylandSurfaceKHR(VkInstance instance, const VkWaylandSurfaceCreateInfoKHR* pCreateInfo,
                                           const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface, VkResult result);
#endif  // VK_USE_PLATFORM_WAYLAND_KHR
#ifdef VK_USE_PLATFORM_WIN32_KHR
bool PreCallValidateGetPhysicalDeviceWin32PresentationSupportKHR(VkPhysicalDevice physicalDevice, uint32_t queueFamilyIndex);
void PostCallRecordCreateWin32SurfaceKHR(VkInstance instance, const VkWin32SurfaceCreateInfoKHR* pCreateInfo,
                                         const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface, VkResult result);
#endif  // VK_USE_PLATFORM_WIN32_KHR
#ifdef VK_USE_PLATFORM_XCB_KHR
bool PreCallValidateGetPhysicalDeviceXcbPresentationSupportKHR(VkPhysicalDevice physicalDevice, uint32_t queueFamilyIndex,
                                                               xcb_connection_t* connection, xcb_visualid_t visual_id);
void PostCallRecordCreateXcbSurfaceKHR(VkInstance instance, const VkXcbSurfaceCreateInfoKHR* pCreateInfo,
                                       const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface, VkResult result);
#endif  // VK_USE_PLATFORM_XCB_KHR
#ifdef VK_USE_PLATFORM_XLIB_KHR
bool PreCallValidateGetPhysicalDeviceXlibPresentationSupportKHR(VkPhysicalDevice physicalDevice, uint32_t queueFamilyIndex,
                                                                Display* dpy, VisualID visualID);
void PostCallRecordCreateXlibSurfaceKHR(VkInstance instance, const VkXlibSurfaceCreateInfoKHR* pCreateInfo,
                                        const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface, VkResult result);
#endif  // VK_USE_PLATFORM_XLIB_KHR

};  // namespace core_validation
