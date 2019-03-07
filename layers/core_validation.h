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
#include "shader_validation.h"
#include "gpu_validation.h"
#include "vk_layer_logging.h"
#include "vulkan/vk_layer.h"
#include "vk_typemap_helper.h"
#include "vk_layer_data.h"
#include <atomic>
#include <functional>
#include <memory>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <list>
#include <deque>

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

struct SubresourceRangeErrorCodes {
    const char *base_mip_err, *mip_count_err, *base_layer_err, *layer_count_err;
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

class CoreChecks : public ValidationObject {
   public:
    std::unordered_set<VkQueue> queues;  // All queues under given device
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
    // Link to the device's physical-device data
    PHYSICAL_DEVICE_STATE* physical_device_state;
    unordered_map<VkSurfaceKHR, SURFACE_STATE> surface_map;

    // Link for derived device objects back to their parent instance object
    CoreChecks* instance_state;

    // Temporary object pointers
    layer_data* device_data = this;
    layer_data* instance_data = this;
    layer_data* dev_data = this;
    std::unordered_map<void*, layer_data*> layer_data_map;
    std::unordered_map<void*, layer_data*> instance_layer_data_map;

    dispatch_key get_dispatch_key(const void* object) { return nullptr; }

    template <typename DATA_T>
    DATA_T* GetLayerDataPtr(void* data_key, std::unordered_map<void*, DATA_T*>& layer_data_map) {
        return this;
    }

    DeviceFeatures enabled_features = {};
    // Device specific data
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

    // Class Declarations for helper functions
    cvdescriptorset::DescriptorSet* GetSetNode(VkDescriptorSet);
    DESCRIPTOR_POOL_STATE* GetDescriptorPoolState(const VkDescriptorPool);
    BUFFER_STATE* GetBufferState(VkBuffer);
    IMAGE_STATE* GetImageState(VkImage);
    DEVICE_MEM_INFO* GetMemObjInfo(VkDeviceMemory);
    BUFFER_VIEW_STATE* GetBufferViewState(VkBufferView);
    SAMPLER_STATE* GetSamplerState(VkSampler);
    IMAGE_VIEW_STATE* GetAttachmentImageViewState(FRAMEBUFFER_STATE* framebuffer, uint32_t index);
    IMAGE_VIEW_STATE* GetImageViewState(VkImageView);
    SWAPCHAIN_NODE* GetSwapchainNode(VkSwapchainKHR);
    GLOBAL_CB_NODE* GetCBNode(const VkCommandBuffer cb);
    PIPELINE_STATE* GetPipelineState(VkPipeline pipeline);
    RENDER_PASS_STATE* GetRenderPassState(VkRenderPass renderpass);
    std::shared_ptr<RENDER_PASS_STATE> GetRenderPassStateSharedPtr(VkRenderPass renderpass);
    FRAMEBUFFER_STATE* GetFramebufferState(VkFramebuffer framebuffer);
    COMMAND_POOL_NODE* GetCommandPoolNode(VkCommandPool pool);
    shader_module const* GetShaderModuleState(VkShaderModule module);
    const DeviceFeatures* GetEnabledFeatures(const layer_data* device_data);
    FENCE_NODE* GetFenceNode(layer_data* dev_data, VkFence fence);
    EVENT_STATE* GetEventNode(layer_data* dev_data, VkEvent event);
    QUERY_POOL_NODE* GetQueryPoolNode(layer_data* dev_data, VkQueryPool query_pool);
    QUEUE_STATE* GetQueueState(layer_data* dev_data, VkQueue queue);
    SEMAPHORE_NODE* GetSemaphoreNode(layer_data* dev_data, VkSemaphore semaphore);
    PHYSICAL_DEVICE_STATE* GetPhysicalDeviceState(VkPhysicalDevice phys);
    PHYSICAL_DEVICE_STATE* GetPhysicalDeviceState();
    SURFACE_STATE* GetSurfaceState(VkSurfaceKHR surface);
    BINDABLE* GetObjectMemBinding(uint64_t handle, VulkanObjectType type);
    bool VerifyQueueStateToSeq(layer_data* dev_data, QUEUE_STATE* initial_queue, uint64_t initial_seq);
    void ClearCmdBufAndMemReferences(layer_data* dev_data, GLOBAL_CB_NODE* cb_node);
    void ClearMemoryObjectBinding(uint64_t handle, VulkanObjectType type, VkDeviceMemory mem);
    void ResetCommandBufferState(layer_data* dev_data, const VkCommandBuffer cb);
    void SetMemBinding(layer_data* dev_data, VkDeviceMemory mem, BINDABLE* mem_binding, VkDeviceSize memory_offset, uint64_t handle,
                       VulkanObjectType type);
    bool ValidateSetMemBinding(layer_data* dev_data, VkDeviceMemory mem, uint64_t handle, VulkanObjectType type,
                               const char* apiName);
    bool SetSparseMemBinding(layer_data* dev_data, MEM_BINDING binding, uint64_t handle, VulkanObjectType type);
    bool ValidateDeviceQueueFamily(layer_data* device_data, uint32_t queue_family, const char* cmd_name, const char* parameter_name,
                                   const char* error_code, bool optional);
    BASE_NODE* GetStateStructPtrFromObject(layer_data* dev_data, VK_OBJECT object_struct);
    void RemoveCommandBufferBinding(layer_data* dev_data, VK_OBJECT const* object, GLOBAL_CB_NODE* cb_node);
    bool ValidateBindBufferMemory(layer_data* device_data, VkBuffer buffer, VkDeviceMemory mem, VkDeviceSize memoryOffset,
                                  const char* api_name);
    void RecordGetBufferMemoryRequirementsState(layer_data* device_data, VkBuffer buffer,
                                                VkMemoryRequirements* pMemoryRequirements);
    void UpdateBindBufferMemoryState(layer_data* device_data, VkBuffer buffer, VkDeviceMemory mem, VkDeviceSize memoryOffset);
    PIPELINE_LAYOUT_NODE const* GetPipelineLayout(layer_data const* dev_data, VkPipelineLayout pipeLayout);
    const TEMPLATE_STATE* GetDescriptorTemplateState(const layer_data* dev_data,
                                                     VkDescriptorUpdateTemplateKHR descriptor_update_template);
    bool ValidateGetImageMemoryRequirements2(layer_data* dev_data, const VkImageMemoryRequirementsInfo2* pInfo);
    void RecordGetImageMemoryRequiementsState(layer_data* device_data, VkImage image, VkMemoryRequirements* pMemoryRequirements);
    void FreeCommandBufferStates(layer_data* dev_data, COMMAND_POOL_NODE* pool_state, const uint32_t command_buffer_count,
                                 const VkCommandBuffer* command_buffers);
    bool CheckCommandBuffersInFlight(layer_data* dev_data, COMMAND_POOL_NODE* pPool, const char* action, const char* error_code);
    bool CheckCommandBufferInFlight(layer_data* dev_data, const GLOBAL_CB_NODE* cb_node, const char* action,
                                    const char* error_code);
    bool VerifyQueueStateToFence(layer_data* dev_data, VkFence fence);
    void DecrementBoundResources(layer_data* dev_data, GLOBAL_CB_NODE const* cb_node);
    bool VerifyWaitFenceState(layer_data* dev_data, VkFence fence, const char* apiCall);
    void RetireFence(layer_data* dev_data, VkFence fence);
    void StoreMemRanges(layer_data* dev_data, VkDeviceMemory mem, VkDeviceSize offset, VkDeviceSize size);
    bool ValidateIdleDescriptorSet(const layer_data* dev_data, VkDescriptorSet set, const char* func_str);
    void InitializeAndTrackMemory(layer_data* dev_data, VkDeviceMemory mem, VkDeviceSize offset, VkDeviceSize size, void** ppData);
    bool ValidatePipelineLocked(layer_data* dev_data, std::vector<std::unique_ptr<PIPELINE_STATE>> const& pPipelines,
                                int pipelineIndex);
    bool ValidatePipelineUnlocked(layer_data* dev_data, std::vector<std::unique_ptr<PIPELINE_STATE>> const& pPipelines,
                                  int pipelineIndex);
    void FreeDescriptorSet(layer_data* dev_data, cvdescriptorset::DescriptorSet* descriptor_set);
    void DeletePools(layer_data* dev_data);
    bool ValidImageBufferQueue(layer_data* dev_data, GLOBAL_CB_NODE* cb_node, const VK_OBJECT* object, VkQueue queue,
                               uint32_t count, const uint32_t* indices);
    bool ValidateFenceForSubmit(layer_data* dev_data, FENCE_NODE* pFence);
    void AddMemObjInfo(layer_data* dev_data, void* object, const VkDeviceMemory mem, const VkMemoryAllocateInfo* pAllocateInfo);
    bool ValidateStatus(layer_data* dev_data, GLOBAL_CB_NODE* pNode, CBStatusFlags status_mask, VkFlags msg_flags,
                        const char* fail_msg, const char* msg_code);
    bool ValidateDrawStateFlags(layer_data* dev_data, GLOBAL_CB_NODE* pCB, const PIPELINE_STATE* pPipe, bool indexed,
                                const char* msg_code);
    bool LogInvalidAttachmentMessage(layer_data const* dev_data, const char* type1_string, const RENDER_PASS_STATE* rp1_state,
                                     const char* type2_string, const RENDER_PASS_STATE* rp2_state, uint32_t primary_attach,
                                     uint32_t secondary_attach, const char* msg, const char* caller, const char* error_code);
    bool ValidateAttachmentCompatibility(layer_data const* dev_data, const char* type1_string, const RENDER_PASS_STATE* rp1_state,
                                         const char* type2_string, const RENDER_PASS_STATE* rp2_state, uint32_t primary_attach,
                                         uint32_t secondary_attach, const char* caller, const char* error_code);
    bool ValidateSubpassCompatibility(layer_data const* dev_data, const char* type1_string, const RENDER_PASS_STATE* rp1_state,
                                      const char* type2_string, const RENDER_PASS_STATE* rp2_state, const int subpass,
                                      const char* caller, const char* error_code);
    bool ValidateRenderPassCompatibility(layer_data const* dev_data, const char* type1_string, const RENDER_PASS_STATE* rp1_state,
                                         const char* type2_string, const RENDER_PASS_STATE* rp2_state, const char* caller,
                                         const char* error_code);
    void UpdateDrawState(layer_data* dev_data, GLOBAL_CB_NODE* cb_state, const VkPipelineBindPoint bind_point);
    bool ReportInvalidCommandBuffer(layer_data* dev_data, const GLOBAL_CB_NODE* cb_state, const char* call_source);
    void InitGpuValidation(layer_data* instance_data);

    bool ValidatePipelineVertexDivisors(layer_data* dev_data, std::vector<std::unique_ptr<PIPELINE_STATE>> const& pipe_state_vec,
                                        const uint32_t count, const VkGraphicsPipelineCreateInfo* pipe_cis);
    void AddFramebufferBinding(layer_data* dev_data, GLOBAL_CB_NODE* cb_state, FRAMEBUFFER_STATE* fb_state);
    bool ValidateImageBarrierImage(layer_data* device_data, const char* funcName, GLOBAL_CB_NODE const* cb_state,
                                   VkFramebuffer framebuffer, uint32_t active_subpass,
                                   const safe_VkSubpassDescription2KHR& sub_desc, uint64_t rp_handle, uint32_t img_index,
                                   const VkImageMemoryBarrier& img_barrier);
    void RecordCmdBeginRenderPassState(layer_data* device_data, VkCommandBuffer commandBuffer,
                                       const VkRenderPassBeginInfo* pRenderPassBegin, const VkSubpassContents contents);
    bool ValidateCmdBeginRenderPass(layer_data* device_data, VkCommandBuffer commandBuffer, RenderPassCreateVersion rp_version,
                                    const VkRenderPassBeginInfo* pRenderPassBegin);
    bool ValidateDependencies(layer_data* dev_data, FRAMEBUFFER_STATE const* framebuffer, RENDER_PASS_STATE const* renderPass);
    bool ValidateBarriers(layer_data* device_data, const char* funcName, GLOBAL_CB_NODE* cb_state,
                          VkPipelineStageFlags src_stage_mask, VkPipelineStageFlags dst_stage_mask, uint32_t memBarrierCount,
                          const VkMemoryBarrier* pMemBarriers, uint32_t bufferBarrierCount,
                          const VkBufferMemoryBarrier* pBufferMemBarriers, uint32_t imageMemBarrierCount,
                          const VkImageMemoryBarrier* pImageMemBarriers);
    bool ValidateCreateSwapchain(layer_data* device_data, const char* func_name, VkSwapchainCreateInfoKHR const* pCreateInfo,
                                 SURFACE_STATE* surface_state, SWAPCHAIN_NODE* old_swapchain_state);
    void RecordCmdPushDescriptorSetState(layer_data* device_data, GLOBAL_CB_NODE* cb_state, VkPipelineBindPoint pipelineBindPoint,
                                         VkPipelineLayout layout, uint32_t set, uint32_t descriptorWriteCount,
                                         const VkWriteDescriptorSet* pDescriptorWrites);
    bool ValidatePipelineBindPoint(layer_data* device_data, GLOBAL_CB_NODE* cb_state, VkPipelineBindPoint bind_point,
                                   const char* func_name, const std::map<VkPipelineBindPoint, std::string>& bind_errors);
    bool ValidateMemoryIsMapped(layer_data* dev_data, const char* funcName, uint32_t memRangeCount,
                                const VkMappedMemoryRange* pMemRanges);
    bool ValidateAndCopyNoncoherentMemoryToDriver(layer_data* dev_data, uint32_t mem_range_count,
                                                  const VkMappedMemoryRange* mem_ranges);
    void CopyNoncoherentMemoryFromDriver(layer_data* dev_data, uint32_t mem_range_count, const VkMappedMemoryRange* mem_ranges);
    bool ValidateMappedMemoryRangeDeviceLimits(layer_data* dev_data, const char* func_name, uint32_t mem_range_count,
                                               const VkMappedMemoryRange* mem_ranges);
    BarrierOperationsType ComputeBarrierOperationsType(layer_data* device_data, GLOBAL_CB_NODE* cb_state,
                                                       uint32_t buffer_barrier_count, const VkBufferMemoryBarrier* buffer_barriers,
                                                       uint32_t image_barrier_count, const VkImageMemoryBarrier* image_barriers);
    bool ValidateStageMasksAgainstQueueCapabilities(layer_data* dev_data, GLOBAL_CB_NODE const* cb_state,
                                                    VkPipelineStageFlags source_stage_mask, VkPipelineStageFlags dest_stage_mask,
                                                    BarrierOperationsType barrier_op_type, const char* function,
                                                    const char* error_code);
    bool SetEventStageMask(VkQueue queue, VkCommandBuffer commandBuffer, VkEvent event, VkPipelineStageFlags stageMask);
    bool ValidateRenderPassImageBarriers(layer_data* device_data, const char* funcName, GLOBAL_CB_NODE* cb_state,
                                         uint32_t active_subpass, const safe_VkSubpassDescription2KHR& sub_desc, uint64_t rp_handle,
                                         const safe_VkSubpassDependency2KHR* dependencies,
                                         const std::vector<uint32_t>& self_dependencies, uint32_t image_mem_barrier_count,
                                         const VkImageMemoryBarrier* image_barriers);
    bool ValidateSecondaryCommandBufferState(layer_data* dev_data, GLOBAL_CB_NODE* pCB, GLOBAL_CB_NODE* pSubCB);
    bool ValidateFramebuffer(layer_data* dev_data, VkCommandBuffer primaryBuffer, const GLOBAL_CB_NODE* pCB,
                             VkCommandBuffer secondaryBuffer, const GLOBAL_CB_NODE* pSubCB, const char* caller);
    bool ValidateDescriptorUpdateTemplate(const char* func_name, layer_data* device_data,
                                          const VkDescriptorUpdateTemplateCreateInfoKHR* pCreateInfo);
    bool ValidateCreateSamplerYcbcrConversion(const layer_data* device_data, const char* func_name,
                                              const VkSamplerYcbcrConversionCreateInfo* create_info);
    void RecordCreateSamplerYcbcrConversionState(layer_data* device_data, const VkSamplerYcbcrConversionCreateInfo* create_info,
                                                 VkSamplerYcbcrConversion ycbcr_conversion);
    bool ValidateImportFence(layer_data* device_data, VkFence fence, const char* caller_name);
    void RecordImportFenceState(layer_data* device_data, VkFence fence, VkExternalFenceHandleTypeFlagBitsKHR handle_type,
                                VkFenceImportFlagsKHR flags);
    void RecordGetExternalFenceState(layer_data* device_data, VkFence fence, VkExternalFenceHandleTypeFlagBitsKHR handle_type);
    bool ValidateAcquireNextImage(layer_data* device_data, VkDevice device, VkSwapchainKHR swapchain, uint64_t timeout,
                                  VkSemaphore semaphore, VkFence fence, uint32_t* pImageIndex, const char* func_name);
    void RecordAcquireNextImageState(layer_data* device_data, VkDevice device, VkSwapchainKHR swapchain, uint64_t timeout,
                                     VkSemaphore semaphore, VkFence fence, uint32_t* pImageIndex);
    bool VerifyRenderAreaBounds(const layer_data* dev_data, const VkRenderPassBeginInfo* pRenderPassBegin);
    bool ValidatePrimaryCommandBuffer(const layer_data* dev_data, const GLOBAL_CB_NODE* pCB, char const* cmd_name,
                                      const char* error_code);
    void RecordCmdNextSubpass(layer_data* device_data, VkCommandBuffer commandBuffer, VkSubpassContents contents);
    bool ValidateCmdEndRenderPass(layer_data* device_data, RenderPassCreateVersion rp_version, VkCommandBuffer commandBuffer);
    void RecordCmdEndRenderPassState(layer_data* device_data, VkCommandBuffer commandBuffer);
    bool ValidateFramebufferCreateInfo(layer_data* dev_data, const VkFramebufferCreateInfo* pCreateInfo);
    bool MatchUsage(layer_data* dev_data, uint32_t count, const VkAttachmentReference2KHR* attachments,
                    const VkFramebufferCreateInfo* fbci, VkImageUsageFlagBits usage_flag, const char* error_code);
    bool CheckDependencyExists(const layer_data* dev_data, const uint32_t subpass, const std::vector<uint32_t>& dependent_subpasses,
                               const std::vector<DAGNode>& subpass_to_node, bool& skip);
    bool CheckPreserved(const layer_data* dev_data, const VkRenderPassCreateInfo2KHR* pCreateInfo, const int index,
                        const uint32_t attachment, const std::vector<DAGNode>& subpass_to_node, int depth, bool& skip);
    bool ValidateBindImageMemory(layer_data* device_data, VkImage image, VkDeviceMemory mem, VkDeviceSize memoryOffset,
                                 const char* api_name);
    void UpdateBindImageMemoryState(layer_data* device_data, VkImage image, VkDeviceMemory mem, VkDeviceSize memoryOffset);
    void RecordGetPhysicalDeviceDisplayPlanePropertiesState(instance_layer_data* instance_data, VkPhysicalDevice physicalDevice,
                                                            uint32_t* pPropertyCount, void* pProperties);
    bool ValidateGetPhysicalDeviceDisplayPlanePropertiesKHRQuery(instance_layer_data* instance_data,
                                                                 VkPhysicalDevice physicalDevice, uint32_t planeIndex,
                                                                 const char* api_name);
    bool ValidateQuery(VkQueue queue, GLOBAL_CB_NODE* pCB, VkQueryPool queryPool, uint32_t firstQuery, uint32_t queryCount);
    bool IsQueryInvalid(layer_data* dev_data, QUEUE_STATE* queue_data, VkQueryPool queryPool, uint32_t queryIndex);
    bool ValidateImportSemaphore(layer_data* device_data, VkSemaphore semaphore, const char* caller_name);
    void RecordImportSemaphoreState(layer_data* device_data, VkSemaphore semaphore,
                                    VkExternalSemaphoreHandleTypeFlagBitsKHR handle_type, VkSemaphoreImportFlagsKHR flags);
    void RecordGetExternalSemaphoreState(layer_data* device_data, VkSemaphore semaphore,
                                         VkExternalSemaphoreHandleTypeFlagBitsKHR handle_type);
    bool SetQueryState(VkQueue queue, VkCommandBuffer commandBuffer, QueryObject object, bool value);
    bool ValidateCmdDrawType(layer_data* dev_data, VkCommandBuffer cmd_buffer, bool indexed, VkPipelineBindPoint bind_point,
                             CMD_TYPE cmd_type, const char* caller, VkQueueFlags queue_flags, const char* queue_flag_code,
                             const char* renderpass_msg_code, const char* pipebound_msg_code, const char* dynamic_state_msg_code);
    void UpdateStateCmdDrawDispatchType(layer_data* dev_data, GLOBAL_CB_NODE* cb_state, VkPipelineBindPoint bind_point);
    void UpdateStateCmdDrawType(layer_data* dev_data, GLOBAL_CB_NODE* cb_state, VkPipelineBindPoint bind_point);
    bool ValidateCmdNextSubpass(layer_data* device_data, RenderPassCreateVersion rp_version, VkCommandBuffer commandBuffer);
    bool RangesIntersect(layer_data const* dev_data, MEMORY_RANGE const* range1, VkDeviceSize offset, VkDeviceSize end);
    bool RangesIntersect(layer_data const* dev_data, MEMORY_RANGE const* range1, MEMORY_RANGE const* range2, bool* skip,
                         bool skip_checks);
    bool ValidateInsertMemoryRange(layer_data const* dev_data, uint64_t handle, DEVICE_MEM_INFO* mem_info,
                                   VkDeviceSize memoryOffset, VkMemoryRequirements memRequirements, bool is_image, bool is_linear,
                                   const char* api_name);
    void InsertMemoryRange(layer_data const* dev_data, uint64_t handle, DEVICE_MEM_INFO* mem_info, VkDeviceSize memoryOffset,
                           VkMemoryRequirements memRequirements, bool is_image, bool is_linear);
    bool ValidateInsertImageMemoryRange(layer_data const* dev_data, VkImage image, DEVICE_MEM_INFO* mem_info,
                                        VkDeviceSize mem_offset, VkMemoryRequirements mem_reqs, bool is_linear,
                                        const char* api_name);
    void InsertImageMemoryRange(layer_data const* dev_data, VkImage image, DEVICE_MEM_INFO* mem_info, VkDeviceSize mem_offset,
                                VkMemoryRequirements mem_reqs, bool is_linear);
    bool ValidateInsertBufferMemoryRange(layer_data const* dev_data, VkBuffer buffer, DEVICE_MEM_INFO* mem_info,
                                         VkDeviceSize mem_offset, VkMemoryRequirements mem_reqs, const char* api_name);
    void InsertBufferMemoryRange(layer_data const* dev_data, VkBuffer buffer, DEVICE_MEM_INFO* mem_info, VkDeviceSize mem_offset,
                                 VkMemoryRequirements mem_reqs);
    bool ValidateMemoryTypes(const layer_data* dev_data, const DEVICE_MEM_INFO* mem_info, const uint32_t memory_type_bits,
                             const char* funcName, const char* msgCode);
    bool ValidateCommandBufferState(layer_data* dev_data, GLOBAL_CB_NODE* cb_state, const char* call_source,
                                    int current_submit_count, const char* vu_id);
    bool ValidateCommandBufferSimultaneousUse(layer_data* dev_data, GLOBAL_CB_NODE* pCB, int current_submit_count);
    bool ValidateGetDeviceQueue(layer_data* device_data, uint32_t queueFamilyIndex, uint32_t queueIndex, VkQueue* pQueue,
                                const char* valid_qfi_vuid, const char* qfi_in_range_vuid);
    void RecordGetDeviceQueueState(layer_data* device_data, uint32_t queue_family_index, VkQueue queue);
    bool ValidateRenderpassAttachmentUsage(const layer_data* dev_data, RenderPassCreateVersion rp_version,
                                           const VkRenderPassCreateInfo2KHR* pCreateInfo);
    bool AddAttachmentUse(const layer_data* dev_data, RenderPassCreateVersion rp_version, uint32_t subpass,
                          std::vector<uint8_t>& attachment_uses, std::vector<VkImageLayout>& attachment_layouts,
                          uint32_t attachment, uint8_t new_use, VkImageLayout new_layout);
    bool ValidateAttachmentIndex(const layer_data* dev_data, RenderPassCreateVersion rp_version, uint32_t attachment,
                                 uint32_t attachment_count, const char* type);
    bool ValidateCreateRenderPass(layer_data* dev_data, VkDevice device, RenderPassCreateVersion rp_version,
                                  const VkRenderPassCreateInfo2KHR* pCreateInfo, RENDER_PASS_STATE* render_pass);
    bool ValidateRenderPassPipelineBarriers(layer_data* device_data, const char* funcName, GLOBAL_CB_NODE* cb_state,
                                            VkPipelineStageFlags src_stage_mask, VkPipelineStageFlags dst_stage_mask,
                                            VkDependencyFlags dependency_flags, uint32_t mem_barrier_count,
                                            const VkMemoryBarrier* mem_barriers, uint32_t buffer_mem_barrier_count,
                                            const VkBufferMemoryBarrier* buffer_mem_barriers, uint32_t image_mem_barrier_count,
                                            const VkImageMemoryBarrier* image_barriers);
    bool CheckStageMaskQueueCompatibility(layer_data* dev_data, VkCommandBuffer command_buffer, VkPipelineStageFlags stage_mask,
                                          VkQueueFlags queue_flags, const char* function, const char* src_or_dest,
                                          const char* error_code);
    void RecordUpdateDescriptorSetWithTemplateState(layer_data* device_data, VkDescriptorSet descriptorSet,
                                                    VkDescriptorUpdateTemplateKHR descriptorUpdateTemplate, const void* pData);
    bool ValidateUpdateDescriptorSetWithTemplate(layer_data* device_data, VkDescriptorSet descriptorSet,
                                                 VkDescriptorUpdateTemplateKHR descriptorUpdateTemplate, const void* pData);
    bool ValidateMemoryIsBoundToBuffer(const layer_data*, const BUFFER_STATE*, const char*, const char*);
    bool ValidateMemoryIsBoundToImage(const layer_data*, const IMAGE_STATE*, const char*, const char*);
    void AddCommandBufferBindingSampler(GLOBAL_CB_NODE*, SAMPLER_STATE*);
    void AddCommandBufferBindingImage(const layer_data*, GLOBAL_CB_NODE*, IMAGE_STATE*);
    void AddCommandBufferBindingImageView(const layer_data*, GLOBAL_CB_NODE*, IMAGE_VIEW_STATE*);
    void AddCommandBufferBindingBuffer(const layer_data*, GLOBAL_CB_NODE*, BUFFER_STATE*);
    void AddCommandBufferBindingBufferView(const layer_data*, GLOBAL_CB_NODE*, BUFFER_VIEW_STATE*);
    bool ValidateObjectNotInUse(const layer_data* dev_data, BASE_NODE* obj_node, VK_OBJECT obj_struct, const char* caller_name,
                                const char* error_code);
    void InvalidateCommandBuffers(const layer_data* dev_data, std::unordered_set<GLOBAL_CB_NODE*> const& cb_nodes, VK_OBJECT obj);
    void RemoveImageMemoryRange(uint64_t handle, DEVICE_MEM_INFO* mem_info);
    void RemoveBufferMemoryRange(uint64_t handle, DEVICE_MEM_INFO* mem_info);
    void ClearMemoryObjectBindings(uint64_t handle, VulkanObjectType type);
    bool ValidateCmdQueueFlags(layer_data* dev_data, const GLOBAL_CB_NODE* cb_node, const char* caller_name, VkQueueFlags flags,
                               const char* error_code);
    bool InsideRenderPass(const layer_data* my_data, const GLOBAL_CB_NODE* pCB, const char* apiName, const char* msgCode);
    bool OutsideRenderPass(const layer_data* my_data, GLOBAL_CB_NODE* pCB, const char* apiName, const char* msgCode);

    void SetLayout(layer_data* device_data, GLOBAL_CB_NODE* pCB, ImageSubresourcePair imgpair, const VkImageLayout& layout);
    void SetLayout(layer_data* device_data, GLOBAL_CB_NODE* pCB, ImageSubresourcePair imgpair,
                   const IMAGE_CMD_BUF_LAYOUT_NODE& node);
    void SetLayout(std::unordered_map<ImageSubresourcePair, IMAGE_LAYOUT_NODE>& imageLayoutMap, ImageSubresourcePair imgpair,
                   VkImageLayout layout);

    bool ValidateImageSampleCount(layer_data* dev_data, IMAGE_STATE* image_state, VkSampleCountFlagBits sample_count,
                                  const char* location, const std::string& msgCode);
    bool ValidateCmdSubpassState(const layer_data* dev_data, const GLOBAL_CB_NODE* pCB, const CMD_TYPE cmd_type);
    bool ValidateCmd(layer_data* dev_data, const GLOBAL_CB_NODE* cb_state, const CMD_TYPE cmd, const char* caller_name);

    // Prototypes for layer_data accessor functions.  These should be in their own header file at some point
    VkFormatProperties GetPDFormatProperties(const layer_data* device_data, const VkFormat format);
    VkResult GetPDImageFormatProperties(layer_data*, const VkImageCreateInfo*, VkImageFormatProperties*);
    VkResult GetPDImageFormatProperties2(layer_data*, const VkPhysicalDeviceImageFormatInfo2*, VkImageFormatProperties2*);
    const debug_report_data* GetReportData(const layer_data*);
    const VkLayerDispatchTable* GetDispatchTable(const layer_data*);
    const VkPhysicalDeviceProperties* GetPDProperties(const layer_data*);
    const VkPhysicalDeviceMemoryProperties* GetPhysicalDeviceMemoryProperties(const layer_data*);
    const CHECK_DISABLED* GetDisables(layer_data*);
    const CHECK_ENABLED* GetEnables(layer_data*);
    std::unordered_map<VkImage, std::unique_ptr<IMAGE_STATE>>* GetImageMap(layer_data*);
    std::unordered_map<VkImage, std::vector<ImageSubresourcePair>>* GetImageSubresourceMap(layer_data*);
    std::unordered_map<ImageSubresourcePair, IMAGE_LAYOUT_NODE>* GetImageLayoutMap(layer_data*);
    std::unordered_map<ImageSubresourcePair, IMAGE_LAYOUT_NODE> const* GetImageLayoutMap(layer_data const*);
    std::unordered_map<VkBuffer, std::unique_ptr<BUFFER_STATE>>* GetBufferMap(layer_data* device_data);
    std::unordered_map<VkBufferView, std::unique_ptr<BUFFER_VIEW_STATE>>* GetBufferViewMap(layer_data* device_data);
    std::unordered_map<VkImageView, std::unique_ptr<IMAGE_VIEW_STATE>>* GetImageViewMap(layer_data* device_data);
    std::unordered_map<VkSamplerYcbcrConversion, uint64_t>* GetYcbcrConversionFormatMap(layer_data*);
    std::unordered_set<uint64_t>* GetAHBExternalFormatsSet(layer_data*);

    const DeviceExtensions* GetDeviceExtensions(const layer_data*);
    GpuValidationState* GetGpuValidationState(layer_data*);
    const GpuValidationState* GetGpuValidationState(const layer_data*);
    VkDevice GetDevice(const layer_data*);

    uint32_t GetApiVersion(const layer_data*);

    GlobalQFOTransferBarrierMap<VkImageMemoryBarrier>& GetGlobalQFOReleaseBarrierMap(
        layer_data* dev_data, const QFOTransferBarrier<VkImageMemoryBarrier>::Tag& type_tag);
    GlobalQFOTransferBarrierMap<VkBufferMemoryBarrier>& GetGlobalQFOReleaseBarrierMap(
        layer_data* dev_data, const QFOTransferBarrier<VkBufferMemoryBarrier>::Tag& type_tag);
    template <typename Barrier>
    void RecordQueuedQFOTransferBarriers(layer_data* device_data, GLOBAL_CB_NODE* cb_state);
    template <typename Barrier>
    bool ValidateQueuedQFOTransferBarriers(layer_data* device_data, GLOBAL_CB_NODE* cb_state,
                                           QFOTransferCBScoreboards<Barrier>* scoreboards);
    bool ValidateQueuedQFOTransfers(layer_data* device_data, GLOBAL_CB_NODE* cb_state,
                                    QFOTransferCBScoreboards<VkImageMemoryBarrier>* qfo_image_scoreboards,
                                    QFOTransferCBScoreboards<VkBufferMemoryBarrier>* qfo_buffer_scoreboards);
    template <typename BarrierRecord, typename Scoreboard>
    bool ValidateAndUpdateQFOScoreboard(const debug_report_data* report_data, const GLOBAL_CB_NODE* cb_state, const char* operation,
                                        const BarrierRecord& barrier, Scoreboard* scoreboard);
    template <typename Barrier>
    void RecordQFOTransferBarriers(layer_data* device_data, GLOBAL_CB_NODE* cb_state, uint32_t barrier_count,
                                   const Barrier* barriers);
    void RecordBarriersQFOTransfers(layer_data* device_data, GLOBAL_CB_NODE* cb_state, uint32_t bufferBarrierCount,
                                    const VkBufferMemoryBarrier* pBufferMemBarriers, uint32_t imageMemBarrierCount,
                                    const VkImageMemoryBarrier* pImageMemBarriers);
    template <typename Barrier>
    bool ValidateQFOTransferBarrierUniqueness(layer_data* device_data, const char* func_name, GLOBAL_CB_NODE* cb_state,
                                              uint32_t barrier_count, const Barrier* barriers);
    bool IsReleaseOp(layer_data* device_data, GLOBAL_CB_NODE* cb_state, VkImageMemoryBarrier const* barrier);
    bool ValidateBarriersQFOTransferUniqueness(layer_data* device_data, const char* func_name, GLOBAL_CB_NODE* cb_state,
                                               uint32_t bufferBarrierCount, const VkBufferMemoryBarrier* pBufferMemBarriers,
                                               uint32_t imageMemBarrierCount, const VkImageMemoryBarrier* pImageMemBarriers);
    bool ValidatePrimaryCommandBufferState(layer_data* dev_data, GLOBAL_CB_NODE* pCB, int current_submit_count,
                                           QFOTransferCBScoreboards<VkImageMemoryBarrier>* qfo_image_scoreboards,
                                           QFOTransferCBScoreboards<VkBufferMemoryBarrier>* qfo_buffer_scoreboards);
    bool ValidatePipelineDrawtimeState(layer_data const* dev_data, LAST_BOUND_STATE const& state, const GLOBAL_CB_NODE* pCB,
                                       CMD_TYPE cmd_type, PIPELINE_STATE const* pPipeline, const char* caller);
    bool ValidateCmdBufDrawState(layer_data* dev_data, GLOBAL_CB_NODE* cb_node, CMD_TYPE cmd_type, const bool indexed,
                                 const VkPipelineBindPoint bind_point, const char* function, const char* pipe_err_code,
                                 const char* state_err_code);
    void IncrementBoundObjects(layer_data* dev_data, GLOBAL_CB_NODE const* cb_node);
    void IncrementResources(layer_data* dev_data, GLOBAL_CB_NODE* cb_node);
    bool ValidateEventStageMask(VkQueue queue, GLOBAL_CB_NODE* pCB, uint32_t eventCount, size_t firstEventIndex,
                                VkPipelineStageFlags sourceStageMask);
    void RetireWorkOnQueue(layer_data* dev_data, QUEUE_STATE* pQueue, uint64_t seq);
    bool ValidateResources(layer_data* dev_data, GLOBAL_CB_NODE* cb_node);
    bool ValidateQueueFamilyIndices(layer_data* dev_data, GLOBAL_CB_NODE* pCB, VkQueue queue);
    VkResult CoreLayerCreateValidationCacheEXT(VkDevice device, const VkValidationCacheCreateInfoEXT* pCreateInfo,
                                               const VkAllocationCallbacks* pAllocator, VkValidationCacheEXT* pValidationCache);
    void CoreLayerDestroyValidationCacheEXT(VkDevice device, VkValidationCacheEXT validationCache,
                                            const VkAllocationCallbacks* pAllocator);
    VkResult CoreLayerMergeValidationCachesEXT(VkDevice device, VkValidationCacheEXT dstCache, uint32_t srcCacheCount,
                                               const VkValidationCacheEXT* pSrcCaches);
    VkResult CoreLayerGetValidationCacheDataEXT(VkDevice device, VkValidationCacheEXT validationCache, size_t* pDataSize,
                                                void* pData);

    // Descriptor Set Validation Functions
    bool ValidateUpdateDescriptorSetsWithTemplateKHR(layer_data* device_data, VkDescriptorSet descriptorSet,
                                                     const TEMPLATE_STATE* template_state, const void* pData);
    void PerformUpdateDescriptorSetsWithTemplateKHR(layer_data* device_data, VkDescriptorSet descriptorSet,
                                                    const TEMPLATE_STATE* template_state, const void* pData);
    void UpdateAllocateDescriptorSetsData(const layer_data* dev_data, const VkDescriptorSetAllocateInfo*,
                                          cvdescriptorset::AllocateDescriptorSetsData*);
    bool ValidateAllocateDescriptorSets(const layer_data*, const VkDescriptorSetAllocateInfo*,
                                        const cvdescriptorset::AllocateDescriptorSetsData*);
    void PerformAllocateDescriptorSets(const VkDescriptorSetAllocateInfo*, const VkDescriptorSet*,
                                       const cvdescriptorset::AllocateDescriptorSetsData*,
                                       std::unordered_map<VkDescriptorPool, DESCRIPTOR_POOL_STATE*>*,
                                       std::unordered_map<VkDescriptorSet, cvdescriptorset::DescriptorSet*>*, layer_data*);
    bool ValidateUpdateDescriptorSets(const debug_report_data* report_data, const layer_data* dev_data, uint32_t write_count,
                                      const VkWriteDescriptorSet* p_wds, uint32_t copy_count, const VkCopyDescriptorSet* p_cds,
                                      const char* func_name);

    // Stuff from shader_validation
    bool ValidateAndCapturePipelineShaderState(layer_data* dev_data, PIPELINE_STATE* pPipeline);
    bool ValidateComputePipeline(layer_data* dev_data, PIPELINE_STATE* pPipeline);
    bool ValidateRayTracingPipelineNV(layer_data* dev_data, PIPELINE_STATE* pipeline);
    bool PreCallValidateCreateShaderModule(VkDevice device, const VkShaderModuleCreateInfo* pCreateInfo,
                                           const VkAllocationCallbacks* pAllocator, VkShaderModule* pShaderModule);
    void PreCallRecordCreateShaderModule(VkDevice device, const VkShaderModuleCreateInfo* pCreateInfo,
                                         const VkAllocationCallbacks* pAllocator, VkShaderModule* pShaderModule, void* csm_state);
    void PostCallRecordCreateShaderModule(VkDevice device, const VkShaderModuleCreateInfo* pCreateInfo,
                                          const VkAllocationCallbacks* pAllocator, VkShaderModule* pShaderModule, VkResult result,
                                          void* csm_state);
    bool ValidatePipelineShaderStage(layer_data* dev_data, VkPipelineShaderStageCreateInfo const* pStage, PIPELINE_STATE* pipeline,
                                     shader_module const** out_module, spirv_inst_iter* out_entrypoint, bool check_point_size);
    bool ValidatePointListShaderState(const layer_data* dev_data, const PIPELINE_STATE* pipeline, shader_module const* src,
                                      spirv_inst_iter entrypoint, VkShaderStageFlagBits stage);
    bool ValidateShaderCapabilities(layer_data* dev_data, shader_module const* src, VkShaderStageFlagBits stage,
                                    bool has_writable_descriptor);
    bool ValidateShaderStageInputOutputLimits(layer_data* dev_data, shader_module const* src,
                                              VkPipelineShaderStageCreateInfo const* pStage, PIPELINE_STATE* pipeline);

    // Gpu Validation Functions
    void GpuPreCallRecordCreateDevice(VkPhysicalDevice gpu, std::unique_ptr<safe_VkDeviceCreateInfo>& modified_create_info,
                                      VkPhysicalDeviceFeatures* supported_features);
    void GpuPostCallRecordCreateDevice(layer_data* dev_data);
    void GpuPreCallRecordDestroyDevice(layer_data* dev_data);
    void GpuPreCallRecordFreeCommandBuffers(layer_data* dev_data, uint32_t commandBufferCount,
                                            const VkCommandBuffer* pCommandBuffers);
    bool GpuPreCallCreateShaderModule(layer_data* dev_data, const VkShaderModuleCreateInfo* pCreateInfo,
                                      const VkAllocationCallbacks* pAllocator, VkShaderModule* pShaderModule,
                                      uint32_t* unique_shader_id, VkShaderModuleCreateInfo* instrumented_create_info,
                                      std::vector<unsigned int>* instrumented_pgm);
    bool GpuPreCallCreatePipelineLayout(layer_data* device_data, const VkPipelineLayoutCreateInfo* pCreateInfo,
                                        const VkAllocationCallbacks* pAllocator, VkPipelineLayout* pPipelineLayout,
                                        std::vector<VkDescriptorSetLayout>* new_layouts,
                                        VkPipelineLayoutCreateInfo* modified_create_info);
    void GpuPostCallCreatePipelineLayout(layer_data* device_data, VkResult result);
    void GpuPostCallQueueSubmit(layer_data* dev_data, VkQueue queue, uint32_t submitCount, const VkSubmitInfo* pSubmits,
                                VkFence fence);
    void GpuPreCallValidateCmdWaitEvents(layer_data* dev_data, VkPipelineStageFlags sourceStageMask);
    std::vector<safe_VkGraphicsPipelineCreateInfo> GpuPreCallRecordCreateGraphicsPipelines(
        layer_data* dev_data, VkPipelineCache pipelineCache, uint32_t count, const VkGraphicsPipelineCreateInfo* pCreateInfos,
        const VkAllocationCallbacks* pAllocator, VkPipeline* pPipelines, std::vector<std::unique_ptr<PIPELINE_STATE>>& pipe_state);
    void GpuPostCallRecordCreateGraphicsPipelines(layer_data* dev_data, const uint32_t count,
                                                  const VkGraphicsPipelineCreateInfo* pCreateInfos,
                                                  const VkAllocationCallbacks* pAllocator, VkPipeline* pPipelines);
    void GpuPreCallRecordDestroyPipeline(layer_data* dev_data, const VkPipeline pipeline);
    void GpuAllocateValidationResources(layer_data* dev_data, const VkCommandBuffer cmd_buffer, VkPipelineBindPoint bind_point);
    void AnalyzeAndReportError(const layer_data* dev_data, GLOBAL_CB_NODE* cb_node, VkQueue queue, uint32_t draw_index,
                               uint32_t* const debug_output_buffer);
    void ProcessInstrumentationBuffer(const layer_data* dev_data, VkQueue queue, GLOBAL_CB_NODE* cb_node);
    void SubmitBarrier(layer_data* dev_data, VkQueue queue);
    bool GpuInstrumentShader(layer_data* dev_data, const VkShaderModuleCreateInfo* pCreateInfo, std::vector<unsigned int>& new_pgm,
                             uint32_t* unique_shader_id);
    void ReportSetupProblem(const layer_data* dev_data, VkDebugReportObjectTypeEXT object_type, uint64_t object_handle,
                            const char* const specific_message);

    // Buffer Validation Functions
    template <class OBJECT, class LAYOUT>
    void SetLayout(layer_data* device_data, OBJECT* pObject, VkImage image, VkImageSubresource range, const LAYOUT& layout);
    template <class OBJECT, class LAYOUT>
    void SetLayout(layer_data* device_data, OBJECT* pObject, ImageSubresourcePair imgpair, const LAYOUT& layout,
                   VkImageAspectFlags aspectMask);
    // Remove the pending QFO release records from the global set
    // Note that the type of the handle argument constrained to match Barrier type
    // The defaulted BarrierRecord argument allows use to declare the type once, but is not intended to be specified by the caller
    template <typename Barrier, typename BarrierRecord = QFOTransferBarrier<Barrier>>
    void EraseQFOReleaseBarriers(layer_data* device_data, const typename BarrierRecord::HandleType& handle) {
        GlobalQFOTransferBarrierMap<Barrier>& global_release_barriers =
            GetGlobalQFOReleaseBarrierMap(device_data, typename BarrierRecord::Tag());
        global_release_barriers.erase(handle);
    }
    bool ValidateCopyImageTransferGranularityRequirements(layer_data* device_data, const GLOBAL_CB_NODE* cb_node,
                                                          const IMAGE_STATE* src_img, const IMAGE_STATE* dst_img,
                                                          const VkImageCopy* region, const uint32_t i, const char* function);
    bool ValidateIdleBuffer(layer_data* device_data, VkBuffer buffer);
    bool ValidateUsageFlags(const layer_data* device_data, VkFlags actual, VkFlags desired, VkBool32 strict, uint64_t obj_handle,
                            VulkanObjectType obj_type, const char* msgCode, char const* func_name, char const* usage_str);
    bool ValidateImageSubresourceRange(const layer_data* device_data, const uint32_t image_mip_count,
                                       const uint32_t image_layer_count, const VkImageSubresourceRange& subresourceRange,
                                       const char* cmd_name, const char* param_name, const char* image_layer_count_var_name,
                                       const uint64_t image_handle, SubresourceRangeErrorCodes errorCodes);
    void SetImageLayout(layer_data* device_data, GLOBAL_CB_NODE* cb_node, const IMAGE_STATE* image_state,
                        VkImageSubresourceRange image_subresource_range, const VkImageLayout& layout);
    void SetImageLayout(layer_data* device_data, GLOBAL_CB_NODE* cb_node, const IMAGE_STATE* image_state,
                        VkImageSubresourceLayers image_subresource_layers, const VkImageLayout& layout);
    bool ValidateRenderPassLayoutAgainstFramebufferImageUsage(layer_data* device_data, RenderPassCreateVersion rp_version,
                                                              VkImageLayout layout, VkImage image, VkImageView image_view,
                                                              VkFramebuffer framebuffer, VkRenderPass renderpass,
                                                              uint32_t attachment_index, const char* variable_name);
    bool ValidateBufferImageCopyData(const debug_report_data* report_data, uint32_t regionCount, const VkBufferImageCopy* pRegions,
                                     IMAGE_STATE* image_state, const char* function);
    bool ValidateBufferViewRange(const layer_data* device_data, const BUFFER_STATE* buffer_state,
                                 const VkBufferViewCreateInfo* pCreateInfo, const VkPhysicalDeviceLimits* device_limits);
    bool ValidateBufferViewBuffer(const layer_data* device_data, const BUFFER_STATE* buffer_state,
                                  const VkBufferViewCreateInfo* pCreateInfo);

    bool PreCallValidateCreateImage(VkDevice device, const VkImageCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator,
                                    VkImage* pImage);

    void PostCallRecordCreateImage(VkDevice device, const VkImageCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator,
                                   VkImage* pImage, VkResult result);

    void PreCallRecordDestroyImage(VkDevice device, VkImage image, const VkAllocationCallbacks* pAllocator);

    bool PreCallValidateDestroyImage(VkDevice device, VkImage image, const VkAllocationCallbacks* pAllocator);

    bool ValidateImageAttributes(layer_data* device_data, IMAGE_STATE* image_state, VkImageSubresourceRange range);

    bool ValidateClearAttachmentExtent(layer_data* device_data, VkCommandBuffer command_buffer, uint32_t attachment_index,
                                       FRAMEBUFFER_STATE* framebuffer, uint32_t fb_attachment, const VkRect2D& render_area,
                                       uint32_t rect_count, const VkClearRect* clear_rects);
    bool ValidateImageCopyData(const layer_data* device_data, const debug_report_data* report_data, const uint32_t regionCount,
                               const VkImageCopy* ic_regions, const IMAGE_STATE* src_state, const IMAGE_STATE* dst_state);

    bool VerifyClearImageLayout(layer_data* device_data, GLOBAL_CB_NODE* cb_node, IMAGE_STATE* image_state,
                                VkImageSubresourceRange range, VkImageLayout dest_image_layout, const char* func_name);

    bool VerifyImageLayout(layer_data const* device_data, GLOBAL_CB_NODE const* cb_node, IMAGE_STATE* image_state,
                           VkImageSubresourceLayers subLayers, VkImageLayout explicit_layout, VkImageLayout optimal_layout,
                           const char* caller, const char* layout_invalid_msg_code, const char* layout_mismatch_msg_code,
                           bool* error);

    bool CheckItgExtent(layer_data* device_data, const GLOBAL_CB_NODE* cb_node, const VkExtent3D* extent, const VkOffset3D* offset,
                        const VkExtent3D* granularity, const VkExtent3D* subresource_extent, const VkImageType image_type,
                        const uint32_t i, const char* function, const char* member, const char* vuid);

    bool CheckItgOffset(layer_data* device_data, const GLOBAL_CB_NODE* cb_node, const VkOffset3D* offset,
                        const VkExtent3D* granularity, const uint32_t i, const char* function, const char* member,
                        const char* vuid);
    VkExtent3D GetScaledItg(layer_data* device_data, const GLOBAL_CB_NODE* cb_node, const IMAGE_STATE* img);
    bool CopyImageMultiplaneValidation(const layer_data* dev_data, VkCommandBuffer command_buffer,
                                       const IMAGE_STATE* src_image_state, const IMAGE_STATE* dst_image_state,
                                       const VkImageCopy region);

    void RecordClearImageLayout(layer_data* dev_data, GLOBAL_CB_NODE* cb_node, VkImage image, VkImageSubresourceRange range,
                                VkImageLayout dest_image_layout);

    bool PreCallValidateCmdClearColorImage(VkCommandBuffer commandBuffer, VkImage image, VkImageLayout imageLayout,
                                           const VkClearColorValue* pColor, uint32_t rangeCount,
                                           const VkImageSubresourceRange* pRanges);

    void PreCallRecordCmdClearColorImage(VkCommandBuffer commandBuffer, VkImage image, VkImageLayout imageLayout,
                                         const VkClearColorValue* pColor, uint32_t rangeCount,
                                         const VkImageSubresourceRange* pRanges);

    bool PreCallValidateCmdClearDepthStencilImage(VkCommandBuffer commandBuffer, VkImage image, VkImageLayout imageLayout,
                                                  const VkClearDepthStencilValue* pDepthStencil, uint32_t rangeCount,
                                                  const VkImageSubresourceRange* pRanges);

    void PreCallRecordCmdClearDepthStencilImage(VkCommandBuffer commandBuffer, VkImage image, VkImageLayout imageLayout,
                                                const VkClearDepthStencilValue* pDepthStencil, uint32_t rangeCount,
                                                const VkImageSubresourceRange* pRanges);

    bool FindLayoutVerifyNode(layer_data const* device_data, GLOBAL_CB_NODE const* pCB, ImageSubresourcePair imgpair,
                              IMAGE_CMD_BUF_LAYOUT_NODE& node, const VkImageAspectFlags aspectMask);

    bool FindLayoutVerifyLayout(layer_data const* device_data, ImageSubresourcePair imgpair, VkImageLayout& layout,
                                const VkImageAspectFlags aspectMask);

    bool FindCmdBufLayout(layer_data const* device_data, GLOBAL_CB_NODE const* pCB, VkImage image, VkImageSubresource range,
                          IMAGE_CMD_BUF_LAYOUT_NODE& node);

    bool FindGlobalLayout(layer_data* device_data, ImageSubresourcePair imgpair, VkImageLayout& layout);

    bool FindLayouts(layer_data* device_data, VkImage image, std::vector<VkImageLayout>& layouts);

    bool FindLayout(layer_data* device_data, const std::unordered_map<ImageSubresourcePair, IMAGE_LAYOUT_NODE>& imageLayoutMap,
                    ImageSubresourcePair imgpair, VkImageLayout& layout);

    bool FindLayout(const std::unordered_map<ImageSubresourcePair, IMAGE_LAYOUT_NODE>& imageLayoutMap, ImageSubresourcePair imgpair,
                    VkImageLayout& layout, const VkImageAspectFlags aspectMask);

    void SetGlobalLayout(layer_data* device_data, ImageSubresourcePair imgpair, const VkImageLayout& layout);

    void SetImageViewLayout(layer_data* device_data, GLOBAL_CB_NODE* pCB, VkImageView imageView, const VkImageLayout& layout);

    void SetImageViewLayout(layer_data* device_data, GLOBAL_CB_NODE* cb_node, IMAGE_VIEW_STATE* view_state,
                            const VkImageLayout& layout);

    bool VerifyFramebufferAndRenderPassLayouts(layer_data* dev_data, RenderPassCreateVersion rp_version, GLOBAL_CB_NODE* pCB,
                                               const VkRenderPassBeginInfo* pRenderPassBegin,
                                               const FRAMEBUFFER_STATE* framebuffer_state);

    void TransitionAttachmentRefLayout(layer_data* dev_data, GLOBAL_CB_NODE* pCB, FRAMEBUFFER_STATE* pFramebuffer,
                                       const safe_VkAttachmentReference2KHR& ref);

    void TransitionSubpassLayouts(layer_data*, GLOBAL_CB_NODE*, const RENDER_PASS_STATE*, const int, FRAMEBUFFER_STATE*);

    void TransitionBeginRenderPassLayouts(layer_data*, GLOBAL_CB_NODE*, const RENDER_PASS_STATE*, FRAMEBUFFER_STATE*);

    bool ValidateImageAspectLayout(layer_data* device_data, GLOBAL_CB_NODE const* pCB, const VkImageMemoryBarrier* mem_barrier,
                                   uint32_t level, uint32_t layer, VkImageAspectFlags aspect);

    void TransitionImageAspectLayout(layer_data* device_data, GLOBAL_CB_NODE* pCB, const VkImageMemoryBarrier* mem_barrier,
                                     uint32_t level, uint32_t layer, VkImageAspectFlags aspect_mask, VkImageAspectFlags aspect);

    bool ValidateBarrierLayoutToImageUsage(layer_data* device_data, const VkImageMemoryBarrier* img_barrier, bool new_not_old,
                                           VkImageUsageFlags usage, const char* func_name);

    bool ValidateBarriersToImages(layer_data* device_data, GLOBAL_CB_NODE const* cb_state, uint32_t imageMemoryBarrierCount,
                                  const VkImageMemoryBarrier* pImageMemoryBarriers, const char* func_name);

    void RecordQueuedQFOTransfers(layer_data* dev_data, GLOBAL_CB_NODE* pCB);
    void EraseQFOImageRelaseBarriers(layer_data* device_data, const VkImage& image);

    void TransitionImageLayouts(layer_data* device_data, GLOBAL_CB_NODE* cb_state, uint32_t memBarrierCount,
                                const VkImageMemoryBarrier* pImgMemBarriers);

    void TransitionFinalSubpassLayouts(layer_data* dev_data, GLOBAL_CB_NODE* pCB, const VkRenderPassBeginInfo* pRenderPassBegin,
                                       FRAMEBUFFER_STATE* framebuffer_state);

    bool PreCallValidateCmdCopyImage(VkCommandBuffer commandBuffer, VkImage srcImage, VkImageLayout srcImageLayout,
                                     VkImage dstImage, VkImageLayout dstImageLayout, uint32_t regionCount,
                                     const VkImageCopy* pRegions);

    bool PreCallValidateCmdClearAttachments(VkCommandBuffer commandBuffer, uint32_t attachmentCount,
                                            const VkClearAttachment* pAttachments, uint32_t rectCount, const VkClearRect* pRects);

    bool PreCallValidateCmdResolveImage(VkCommandBuffer commandBuffer, VkImage srcImage, VkImageLayout srcImageLayout,
                                        VkImage dstImage, VkImageLayout dstImageLayout, uint32_t regionCount,
                                        const VkImageResolve* pRegions);

    void PreCallRecordCmdResolveImage(VkCommandBuffer commandBuffer, VkImage srcImage, VkImageLayout srcImageLayout,
                                      VkImage dstImage, VkImageLayout dstImageLayout, uint32_t regionCount,
                                      const VkImageResolve* pRegions);

    bool PreCallValidateCmdBlitImage(VkCommandBuffer commandBuffer, VkImage srcImage, VkImageLayout srcImageLayout,
                                     VkImage dstImage, VkImageLayout dstImageLayout, uint32_t regionCount,
                                     const VkImageBlit* pRegions, VkFilter filter);

    void PreCallRecordCmdBlitImage(VkCommandBuffer commandBuffer, VkImage srcImage, VkImageLayout srcImageLayout, VkImage dstImage,
                                   VkImageLayout dstImageLayout, uint32_t regionCount, const VkImageBlit* pRegions,
                                   VkFilter filter);

    bool ValidateCmdBufImageLayouts(layer_data* device_data, GLOBAL_CB_NODE* pCB,
                                    std::unordered_map<ImageSubresourcePair, IMAGE_LAYOUT_NODE> const& globalImageLayoutMap,
                                    std::unordered_map<ImageSubresourcePair, IMAGE_LAYOUT_NODE>& overlayLayoutMap);

    void UpdateCmdBufImageLayouts(layer_data* device_data, GLOBAL_CB_NODE* pCB);

    bool ValidateMaskBitsFromLayouts(layer_data* device_data, VkCommandBuffer cmdBuffer, const VkAccessFlags& accessMask,
                                     const VkImageLayout& layout, const char* type);

    bool ValidateLayoutVsAttachmentDescription(const debug_report_data* report_data, RenderPassCreateVersion rp_version,
                                               const VkImageLayout first_layout, const uint32_t attachment,
                                               const VkAttachmentDescription2KHR& attachment_description);

    bool ValidateLayouts(layer_data* dev_data, RenderPassCreateVersion rp_version, VkDevice device,
                         const VkRenderPassCreateInfo2KHR* pCreateInfo);

    bool ValidateMapImageLayouts(layer_data* dev_data, VkDevice device, DEVICE_MEM_INFO const* mem_info, VkDeviceSize offset,
                                 VkDeviceSize end_offset);

    bool ValidateImageUsageFlags(layer_data* dev_data, IMAGE_STATE const* image_state, VkFlags desired, bool strict,
                                 const char* msgCode, char const* func_name, char const* usage_string);

    bool ValidateImageFormatFeatureFlags(layer_data* dev_data, IMAGE_STATE const* image_state, VkFormatFeatureFlags desired,
                                         char const* func_name, const char* linear_vuid, const char* optimal_vuid);

    bool ValidateImageSubresourceLayers(layer_data* dev_data, const GLOBAL_CB_NODE* cb_node,
                                        const VkImageSubresourceLayers* subresource_layers, char const* func_name,
                                        char const* member, uint32_t i);

    bool ValidateBufferUsageFlags(const layer_data* dev_data, BUFFER_STATE const* buffer_state, VkFlags desired, bool strict,
                                  const char* msgCode, char const* func_name, char const* usage_string);

    bool PreCallValidateCreateBuffer(VkDevice device, const VkBufferCreateInfo* pCreateInfo,
                                     const VkAllocationCallbacks* pAllocator, VkBuffer* pBuffer);

    void PostCallRecordCreateBuffer(VkDevice device, const VkBufferCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator,
                                    VkBuffer* pBuffer, VkResult result);

    bool PreCallValidateCreateBufferView(VkDevice device, const VkBufferViewCreateInfo* pCreateInfo,
                                         const VkAllocationCallbacks* pAllocator, VkBufferView* pView);

    void PostCallRecordCreateBufferView(VkDevice device, const VkBufferViewCreateInfo* pCreateInfo,
                                        const VkAllocationCallbacks* pAllocator, VkBufferView* pView, VkResult result);

    bool ValidateImageAspectMask(const layer_data* device_data, VkImage image, VkFormat format, VkImageAspectFlags aspect_mask,
                                 const char* func_name, const char* vuid = "VUID-VkImageSubresource-aspectMask-parameter");

    bool ValidateCreateImageViewSubresourceRange(const layer_data* device_data, const IMAGE_STATE* image_state,
                                                 bool is_imageview_2d_type, const VkImageSubresourceRange& subresourceRange);

    bool ValidateCmdClearColorSubresourceRange(const layer_data* device_data, const IMAGE_STATE* image_state,
                                               const VkImageSubresourceRange& subresourceRange, const char* param_name);

    bool ValidateCmdClearDepthSubresourceRange(const layer_data* device_data, const IMAGE_STATE* image_state,
                                               const VkImageSubresourceRange& subresourceRange, const char* param_name);

    bool ValidateImageBarrierSubresourceRange(const layer_data* device_data, const IMAGE_STATE* image_state,
                                              const VkImageSubresourceRange& subresourceRange, const char* cmd_name,
                                              const char* param_name);

    bool PreCallValidateCreateImageView(VkDevice device, const VkImageViewCreateInfo* pCreateInfo,
                                        const VkAllocationCallbacks* pAllocator, VkImageView* pView);

    void PostCallRecordCreateImageView(VkDevice device, const VkImageViewCreateInfo* pCreateInfo,
                                       const VkAllocationCallbacks* pAllocator, VkImageView* pView, VkResult result);

    bool ValidateCopyBufferImageTransferGranularityRequirements(layer_data* device_data, const GLOBAL_CB_NODE* cb_node,
                                                                const IMAGE_STATE* img, const VkBufferImageCopy* region,
                                                                const uint32_t i, const char* function, const char* vuid);

    bool ValidateImageMipLevel(layer_data* device_data, const GLOBAL_CB_NODE* cb_node, const IMAGE_STATE* img, uint32_t mip_level,
                               const uint32_t i, const char* function, const char* member, const char* vuid);

    bool ValidateImageArrayLayerRange(layer_data* device_data, const GLOBAL_CB_NODE* cb_node, const IMAGE_STATE* img,
                                      const uint32_t base_layer, const uint32_t layer_count, const uint32_t i, const char* function,
                                      const char* member, const char* vuid);

    void PreCallRecordCmdCopyImage(VkCommandBuffer commandBuffer, VkImage srcImage, VkImageLayout srcImageLayout, VkImage dstImage,
                                   VkImageLayout dstImageLayout, uint32_t regionCount, const VkImageCopy* pRegions);

    bool PreCallValidateCmdCopyBuffer(VkCommandBuffer commandBuffer, VkBuffer srcBuffer, VkBuffer dstBuffer, uint32_t regionCount,
                                      const VkBufferCopy* pRegions);

    void PreCallRecordCmdCopyBuffer(VkCommandBuffer commandBuffer, VkBuffer srcBuffer, VkBuffer dstBuffer, uint32_t regionCount,
                                    const VkBufferCopy* pRegions);

    bool PreCallValidateDestroyImageView(VkDevice device, VkImageView imageView, const VkAllocationCallbacks* pAllocator);

    void PreCallRecordDestroyImageView(VkDevice device, VkImageView imageView, const VkAllocationCallbacks* pAllocator);

    bool PreCallValidateDestroyBuffer(VkDevice device, VkBuffer buffer, const VkAllocationCallbacks* pAllocator);

    void PreCallRecordDestroyBuffer(VkDevice device, VkBuffer buffer, const VkAllocationCallbacks* pAllocator);

    bool PreCallValidateDestroyBufferView(VkDevice device, VkBufferView bufferView, const VkAllocationCallbacks* pAllocator);

    void PreCallRecordDestroyBufferView(VkDevice device, VkBufferView bufferView, const VkAllocationCallbacks* pAllocator);

    bool PreCallValidateCmdFillBuffer(VkCommandBuffer commandBuffer, VkBuffer dstBuffer, VkDeviceSize dstOffset, VkDeviceSize size,
                                      uint32_t data);

    void PreCallRecordCmdFillBuffer(VkCommandBuffer commandBuffer, VkBuffer dstBuffer, VkDeviceSize dstOffset, VkDeviceSize size,
                                    uint32_t data);

    bool PreCallValidateCmdCopyImageToBuffer(VkCommandBuffer commandBuffer, VkImage srcImage, VkImageLayout srcImageLayout,
                                             VkBuffer dstBuffer, uint32_t regionCount, const VkBufferImageCopy* pRegions);

    void PreCallRecordCmdCopyImageToBuffer(VkCommandBuffer commandBuffer, VkImage srcImage, VkImageLayout srcImageLayout,
                                           VkBuffer dstBuffer, uint32_t regionCount, const VkBufferImageCopy* pRegions);

    bool PreCallValidateCmdCopyBufferToImage(VkCommandBuffer commandBuffer, VkBuffer srcBuffer, VkImage dstImage,
                                             VkImageLayout dstImageLayout, uint32_t regionCount, const VkBufferImageCopy* pRegions);

    void PreCallRecordCmdCopyBufferToImage(VkCommandBuffer commandBuffer, VkBuffer srcBuffer, VkImage dstImage,
                                           VkImageLayout dstImageLayout, uint32_t regionCount, const VkBufferImageCopy* pRegions);

    bool PreCallValidateGetImageSubresourceLayout(VkDevice device, VkImage image, const VkImageSubresource* pSubresource,
                                                  VkSubresourceLayout* pLayout);
    bool ValidateCreateImageANDROID(layer_data* device_data, const debug_report_data* report_data,
                                    const VkImageCreateInfo* create_info);
    void RecordCreateImageANDROID(const VkImageCreateInfo* create_info, IMAGE_STATE* is_node);
    bool ValidateCreateImageViewANDROID(layer_data* device_data, const VkImageViewCreateInfo* create_info);
    bool ValidateGetImageSubresourceLayoutANDROID(layer_data* device_data, const VkImage image);
    bool ValidateQueueFamilies(layer_data* device_data, uint32_t queue_family_count, const uint32_t* queue_families,
                               const char* cmd_name, const char* array_parameter_name, const char* unique_error_code,
                               const char* valid_error_code, bool optional);
    bool ValidateAllocateMemoryANDROID(layer_data* dev_data, const VkMemoryAllocateInfo* alloc_info);
    bool ValidateGetImageMemoryRequirements2ANDROID(layer_data* dev_data, const VkImage image);
    bool ValidateCreateSamplerYcbcrConversionANDROID(const layer_data* dev_data,
                                                     const VkSamplerYcbcrConversionCreateInfo* create_info);
    void RecordCreateSamplerYcbcrConversionANDROID(layer_data* dev_data, const VkSamplerYcbcrConversionCreateInfo* create_info,
                                                   VkSamplerYcbcrConversion ycbcr_conversion);
    void RecordDestroySamplerYcbcrConversionANDROID(layer_data* dev_data, VkSamplerYcbcrConversion ycbcr_conversion);
    bool PreCallValidateCreateGraphicsPipelines(VkDevice device, VkPipelineCache pipelineCache, uint32_t count,
                                                const VkGraphicsPipelineCreateInfo* pCreateInfos,
                                                const VkAllocationCallbacks* pAllocator, VkPipeline* pPipelines, void* cgpl_state);
    void PreCallRecordCreateGraphicsPipelines(VkDevice device, VkPipelineCache pipelineCache, uint32_t count,
                                              const VkGraphicsPipelineCreateInfo* pCreateInfos,
                                              const VkAllocationCallbacks* pAllocator, VkPipeline* pPipelines, void* cgpl_state);
    void PostCallRecordCreateGraphicsPipelines(VkDevice device, VkPipelineCache pipelineCache, uint32_t count,
                                               const VkGraphicsPipelineCreateInfo* pCreateInfos,
                                               const VkAllocationCallbacks* pAllocator, VkPipeline* pPipelines, VkResult result,
                                               void* cgpl_state);
    bool PreCallValidateCreateComputePipelines(VkDevice device, VkPipelineCache pipelineCache, uint32_t count,
                                               const VkComputePipelineCreateInfo* pCreateInfos,
                                               const VkAllocationCallbacks* pAllocator, VkPipeline* pPipelines, void* pipe_state);
    void PostCallRecordCreateComputePipelines(VkDevice device, VkPipelineCache pipelineCache, uint32_t count,
                                              const VkComputePipelineCreateInfo* pCreateInfos,
                                              const VkAllocationCallbacks* pAllocator, VkPipeline* pPipelines, VkResult result,
                                              void* pipe_state);
    bool PreCallValidateCreatePipelineLayout(VkDevice device, const VkPipelineLayoutCreateInfo* pCreateInfo,
                                             const VkAllocationCallbacks* pAllocator, VkPipelineLayout* pPipelineLayout);
    void PreCallRecordCreatePipelineLayout(VkDevice device, const VkPipelineLayoutCreateInfo* pCreateInfo,
                                           const VkAllocationCallbacks* pAllocator, VkPipelineLayout* pPipelineLayout,
                                           void* cpl_state);
    void PostCallRecordCreatePipelineLayout(VkDevice device, const VkPipelineLayoutCreateInfo* pCreateInfo,
                                            const VkAllocationCallbacks* pAllocator, VkPipelineLayout* pPipelineLayout,
                                            VkResult result);
    bool PreCallValidateAllocateDescriptorSets(VkDevice device, const VkDescriptorSetAllocateInfo* pAllocateInfo,
                                               VkDescriptorSet* pDescriptorSets, void* ads_state);
    void PostCallRecordAllocateDescriptorSets(VkDevice device, const VkDescriptorSetAllocateInfo* pAllocateInfo,
                                              VkDescriptorSet* pDescriptorSets, VkResult result, void* ads_state);
    bool PreCallValidateCreateRayTracingPipelinesNV(VkDevice device, VkPipelineCache pipelineCache, uint32_t count,
                                                    const VkRayTracingPipelineCreateInfoNV* pCreateInfos,
                                                    const VkAllocationCallbacks* pAllocator, VkPipeline* pPipelines,
                                                    void* pipe_state);
    void PostCallRecordCreateRayTracingPipelinesNV(VkDevice device, VkPipelineCache pipelineCache, uint32_t count,
                                                   const VkRayTracingPipelineCreateInfoNV* pCreateInfos,
                                                   const VkAllocationCallbacks* pAllocator, VkPipeline* pPipelines, VkResult result,
                                                   void* pipe_state);
    void PostCallRecordCreateInstance(const VkInstanceCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator,
                                      VkInstance* pInstance, VkResult result);
    bool PreCallValidateCreateDevice(VkPhysicalDevice gpu, const VkDeviceCreateInfo* pCreateInfo,
                                     const VkAllocationCallbacks* pAllocator, VkDevice* pDevice);
    void PreCallRecordCreateDevice(VkPhysicalDevice gpu, const VkDeviceCreateInfo* pCreateInfo,
                                   const VkAllocationCallbacks* pAllocator, VkDevice* pDevice,
                                   std::unique_ptr<safe_VkDeviceCreateInfo>& modified_create_info);
    void PostCallRecordCreateDevice(VkPhysicalDevice gpu, const VkDeviceCreateInfo* pCreateInfo,
                                    const VkAllocationCallbacks* pAllocator, VkDevice* pDevice, VkResult result);
    bool PreCallValidateCmdUpdateBuffer(VkCommandBuffer commandBuffer, VkBuffer dstBuffer, VkDeviceSize dstOffset,
                                        VkDeviceSize dataSize, const void* pData);
    void PostCallRecordCmdUpdateBuffer(VkCommandBuffer commandBuffer, VkBuffer dstBuffer, VkDeviceSize dstOffset,
                                       VkDeviceSize dataSize, const void* pData);
    void PostCallRecordCreateFence(VkDevice device, const VkFenceCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator,
                                   VkFence* pFence, VkResult result);
    bool PreCallValidateGetDeviceQueue(VkDevice device, uint32_t queueFamilyIndex, uint32_t queueIndex, VkQueue* pQueue);
    void PostCallRecordGetDeviceQueue(VkDevice device, uint32_t queueFamilyIndex, uint32_t queueIndex, VkQueue* pQueue);
    void PostCallRecordGetDeviceQueue2(VkDevice device, const VkDeviceQueueInfo2* pQueueInfo, VkQueue* pQueue);
    bool PreCallValidateCreateSamplerYcbcrConversion(VkDevice device, const VkSamplerYcbcrConversionCreateInfo* pCreateInfo,
                                                     const VkAllocationCallbacks* pAllocator,
                                                     VkSamplerYcbcrConversion* pYcbcrConversion);
    void PostCallRecordCreateSamplerYcbcrConversion(VkDevice device, const VkSamplerYcbcrConversionCreateInfo* pCreateInfo,
                                                    const VkAllocationCallbacks* pAllocator,
                                                    VkSamplerYcbcrConversion* pYcbcrConversion, VkResult result);
    bool PreCallValidateCreateSamplerYcbcrConversionKHR(VkDevice device, const VkSamplerYcbcrConversionCreateInfo* pCreateInfo,
                                                        const VkAllocationCallbacks* pAllocator,
                                                        VkSamplerYcbcrConversion* pYcbcrConversion);
    void PostCallRecordCreateSamplerYcbcrConversionKHR(VkDevice device, const VkSamplerYcbcrConversionCreateInfo* pCreateInfo,
                                                       const VkAllocationCallbacks* pAllocator,
                                                       VkSamplerYcbcrConversion* pYcbcrConversion, VkResult result);
    bool PreCallValidateCmdDebugMarkerBeginEXT(VkCommandBuffer commandBuffer, const VkDebugMarkerMarkerInfoEXT* pMarkerInfo);
    void PreCallRecordDestroyDevice(VkDevice device, const VkAllocationCallbacks* pAllocator);
    bool PreCallValidateQueueSubmit(VkQueue queue, uint32_t submitCount, const VkSubmitInfo* pSubmits, VkFence fence);
    void PostCallRecordQueueSubmit(VkQueue queue, uint32_t submitCount, const VkSubmitInfo* pSubmits, VkFence fence,
                                   VkResult result);
    bool PreCallValidateAllocateMemory(VkDevice device, const VkMemoryAllocateInfo* pAllocateInfo,
                                       const VkAllocationCallbacks* pAllocator, VkDeviceMemory* pMemory);
    void PostCallRecordAllocateMemory(VkDevice device, const VkMemoryAllocateInfo* pAllocateInfo,
                                      const VkAllocationCallbacks* pAllocator, VkDeviceMemory* pMemory, VkResult result);
    bool PreCallValidateFreeMemory(VkDevice device, VkDeviceMemory mem, const VkAllocationCallbacks* pAllocator);
    void PreCallRecordFreeMemory(VkDevice device, VkDeviceMemory mem, const VkAllocationCallbacks* pAllocator);
    bool PreCallValidateWaitForFences(VkDevice device, uint32_t fenceCount, const VkFence* pFences, VkBool32 waitAll,
                                      uint64_t timeout);
    void PostCallRecordWaitForFences(VkDevice device, uint32_t fenceCount, const VkFence* pFences, VkBool32 waitAll,
                                     uint64_t timeout, VkResult result);
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
    void PreCallRecordDestroyPipelineLayout(VkDevice device, VkPipelineLayout pipelineLayout,
                                            const VkAllocationCallbacks* pAllocator);
    bool PreCallValidateDestroySampler(VkDevice device, VkSampler sampler, const VkAllocationCallbacks* pAllocator);
    void PreCallRecordDestroySampler(VkDevice device, VkSampler sampler, const VkAllocationCallbacks* pAllocator);
    void PreCallRecordDestroyDescriptorSetLayout(VkDevice device, VkDescriptorSetLayout descriptorSetLayout,
                                                 const VkAllocationCallbacks* pAllocator);
    bool PreCallValidateDestroyDescriptorPool(VkDevice device, VkDescriptorPool descriptorPool,
                                              const VkAllocationCallbacks* pAllocator);
    void PreCallRecordDestroyDescriptorPool(VkDevice device, VkDescriptorPool descriptorPool,
                                            const VkAllocationCallbacks* pAllocator);
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
    void PostCallRecordCreateSampler(VkDevice device, const VkSamplerCreateInfo* pCreateInfo,
                                     const VkAllocationCallbacks* pAllocator, VkSampler* pSampler, VkResult result);
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
    bool PreCallValidateBeginCommandBuffer(VkCommandBuffer commandBuffer, const VkCommandBufferBeginInfo* pBeginInfo);
    void PreCallRecordBeginCommandBuffer(VkCommandBuffer commandBuffer, const VkCommandBufferBeginInfo* pBeginInfo);
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
                                                           uint32_t viewportCount,
                                                           const VkShadingRatePaletteNV* pShadingRatePalettes);
    void PreCallRecordCmdSetViewportShadingRatePaletteNV(VkCommandBuffer commandBuffer, uint32_t firstViewport,
                                                         uint32_t viewportCount,
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
    bool PreCallValidateCmdBindIndexBuffer(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                           VkIndexType indexType);
    void PreCallRecordCmdBindIndexBuffer(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                         VkIndexType indexType);
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
    bool PreCallValidateCmdDrawIndexed(VkCommandBuffer commandBuffer, uint32_t indexCount, uint32_t instanceCount,
                                       uint32_t firstIndex, int32_t vertexOffset, uint32_t firstInstance);
    void PreCallRecordCmdDrawIndexed(VkCommandBuffer commandBuffer, uint32_t indexCount, uint32_t instanceCount,
                                     uint32_t firstIndex, int32_t vertexOffset, uint32_t firstInstance);
    void PostCallRecordCmdDrawIndexed(VkCommandBuffer commandBuffer, uint32_t indexCount, uint32_t instanceCount,
                                      uint32_t firstIndex, int32_t vertexOffset, uint32_t firstInstance);
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
                                                uint32_t queryCount, VkBuffer dstBuffer, VkDeviceSize dstOffset,
                                                VkDeviceSize stride, VkQueryResultFlags flags);
    void PostCallRecordCmdCopyQueryPoolResults(VkCommandBuffer commandBuffer, VkQueryPool queryPool, uint32_t firstQuery,
                                               uint32_t queryCount, VkBuffer dstBuffer, VkDeviceSize dstOffset, VkDeviceSize stride,
                                               VkQueryResultFlags flags);
    bool PreCallValidateCmdPushConstants(VkCommandBuffer commandBuffer, VkPipelineLayout layout, VkShaderStageFlags stageFlags,
                                         uint32_t offset, uint32_t size, const void* pValues);
    bool PreCallValidateCmdWriteTimestamp(VkCommandBuffer commandBuffer, VkPipelineStageFlagBits pipelineStage,
                                          VkQueryPool queryPool, uint32_t slot);
    void PostCallRecordCmdWriteTimestamp(VkCommandBuffer commandBuffer, VkPipelineStageFlagBits pipelineStage,
                                         VkQueryPool queryPool, uint32_t slot);
    bool PreCallValidateCreateFramebuffer(VkDevice device, const VkFramebufferCreateInfo* pCreateInfo,
                                          const VkAllocationCallbacks* pAllocator, VkFramebuffer* pFramebuffer);
    void PostCallRecordCreateFramebuffer(VkDevice device, const VkFramebufferCreateInfo* pCreateInfo,
                                         const VkAllocationCallbacks* pAllocator, VkFramebuffer* pFramebuffer, VkResult result);
    bool PreCallValidateCreateRenderPass(VkDevice device, const VkRenderPassCreateInfo* pCreateInfo,
                                         const VkAllocationCallbacks* pAllocator, VkRenderPass* pRenderPass);
    void PostCallRecordCreateRenderPass(VkDevice device, const VkRenderPassCreateInfo* pCreateInfo,
                                        const VkAllocationCallbacks* pAllocator, VkRenderPass* pRenderPass, VkResult result);
    bool PreCallValidateGetDeviceMemoryCommitment(VkDevice device, VkDeviceMemory mem, VkDeviceSize* pCommittedMem);
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
    bool PreCallValidateInvalidateMappedMemoryRanges(VkDevice device, uint32_t memRangeCount,
                                                     const VkMappedMemoryRange* pMemRanges);
    void PostCallRecordInvalidateMappedMemoryRanges(VkDevice device, uint32_t memRangeCount, const VkMappedMemoryRange* pMemRanges,
                                                    VkResult result);
    bool PreCallValidateBindImageMemory(VkDevice device, VkImage image, VkDeviceMemory mem, VkDeviceSize memoryOffset);
    void PostCallRecordBindImageMemory(VkDevice device, VkImage image, VkDeviceMemory mem, VkDeviceSize memoryOffset,
                                       VkResult result);
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
    void PostCallRecordImportSemaphoreFdKHR(VkDevice device, const VkImportSemaphoreFdInfoKHR* pImportSemaphoreFdInfo,
                                            VkResult result);
#ifdef VK_USE_PLATFORM_WIN32_KHR
    void PostCallRecordImportSemaphoreWin32HandleKHR(VkDevice device,
                                                     const VkImportSemaphoreWin32HandleInfoKHR* pImportSemaphoreWin32HandleInfo,
                                                     VkResult result);
    bool PreCallValidateImportSemaphoreWin32HandleKHR(VkDevice device,
                                                      const VkImportSemaphoreWin32HandleInfoKHR* pImportSemaphoreWin32HandleInfo);
    bool PreCallValidateImportFenceWin32HandleKHR(VkDevice device,
                                                  const VkImportFenceWin32HandleInfoKHR* pImportFenceWin32HandleInfo);
    void PostCallRecordImportFenceWin32HandleKHR(VkDevice device,
                                                 const VkImportFenceWin32HandleInfoKHR* pImportFenceWin32HandleInfo,
                                                 VkResult result);
    void PostCallRecordGetSemaphoreWin32HandleKHR(VkDevice device, const VkSemaphoreGetWin32HandleInfoKHR* pGetWin32HandleInfo,
                                                  HANDLE* pHandle, VkResult result);
    void PostCallRecordGetFenceWin32HandleKHR(VkDevice device, const VkFenceGetWin32HandleInfoKHR* pGetWin32HandleInfo,
                                              HANDLE* pHandle, VkResult result);
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
                                                  const VkSwapchainCreateInfoKHR* pCreateInfos,
                                                  const VkAllocationCallbacks* pAllocator, VkSwapchainKHR* pSwapchains);
    void PostCallRecordCreateSharedSwapchainsKHR(VkDevice device, uint32_t swapchainCount,
                                                 const VkSwapchainCreateInfoKHR* pCreateInfos,
                                                 const VkAllocationCallbacks* pAllocator, VkSwapchainKHR* pSwapchains,
                                                 VkResult result);
    bool PreCallValidateAcquireNextImageKHR(VkDevice device, VkSwapchainKHR swapchain, uint64_t timeout, VkSemaphore semaphore,
                                            VkFence fence, uint32_t* pImageIndex);
    bool PreCallValidateAcquireNextImage2KHR(VkDevice device, const VkAcquireNextImageInfoKHR* pAcquireInfo, uint32_t* pImageIndex);
    void PostCallRecordAcquireNextImageKHR(VkDevice device, VkSwapchainKHR swapchain, uint64_t timeout, VkSemaphore semaphore,
                                           VkFence fence, uint32_t* pImageIndex, VkResult result);
    void PostCallRecordAcquireNextImage2KHR(VkDevice device, const VkAcquireNextImageInfoKHR* pAcquireInfo, uint32_t* pImageIndex,
                                            VkResult result);
    void PostCallRecordEnumeratePhysicalDevices(VkInstance instance, uint32_t* pPhysicalDeviceCount,
                                                VkPhysicalDevice* pPhysicalDevices, VkResult result);
    bool PreCallValidateGetPhysicalDeviceQueueFamilyProperties(VkPhysicalDevice physicalDevice, uint32_t* pQueueFamilyPropertyCount,
                                                               VkQueueFamilyProperties* pQueueFamilyProperties);
    void PostCallRecordGetPhysicalDeviceQueueFamilyProperties(VkPhysicalDevice physicalDevice, uint32_t* pQueueFamilyPropertyCount,
                                                              VkQueueFamilyProperties* pQueueFamilyProperties);
    bool PreCallValidateGetPhysicalDeviceQueueFamilyProperties2(VkPhysicalDevice physicalDevice,
                                                                uint32_t* pQueueFamilyPropertyCount,
                                                                VkQueueFamilyProperties2KHR* pQueueFamilyProperties);
    void PostCallRecordGetPhysicalDeviceQueueFamilyProperties2(VkPhysicalDevice physicalDevice, uint32_t* pQueueFamilyPropertyCount,
                                                               VkQueueFamilyProperties2KHR* pQueueFamilyProperties);
    bool PreCallValidateGetPhysicalDeviceQueueFamilyProperties2KHR(VkPhysicalDevice physicalDevice,
                                                                   uint32_t* pQueueFamilyPropertyCount,
                                                                   VkQueueFamilyProperties2KHR* pQueueFamilyProperties);
    void PostCallRecordGetPhysicalDeviceQueueFamilyProperties2KHR(VkPhysicalDevice physicalDevice,
                                                                  uint32_t* pQueueFamilyPropertyCount,
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
                                                    const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface,
                                                    VkResult result);
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
    void PostCallRecordDestroyDebugReportCallbackEXT(VkInstance instance, VkDebugReportCallbackEXT msgCallback,
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
    bool PreCallValidateCreateDescriptorUpdateTemplateKHR(VkDevice device,
                                                          const VkDescriptorUpdateTemplateCreateInfoKHR* pCreateInfo,
                                                          const VkAllocationCallbacks* pAllocator,
                                                          VkDescriptorUpdateTemplateKHR* pDescriptorUpdateTemplate);
    void PostCallRecordCreateDescriptorUpdateTemplateKHR(VkDevice device,
                                                         const VkDescriptorUpdateTemplateCreateInfoKHR* pCreateInfo,
                                                         const VkAllocationCallbacks* pAllocator,
                                                         VkDescriptorUpdateTemplateKHR* pDescriptorUpdateTemplate, VkResult result);
    void PreCallRecordDestroyDescriptorUpdateTemplate(VkDevice device, VkDescriptorUpdateTemplateKHR descriptorUpdateTemplate,
                                                      const VkAllocationCallbacks* pAllocator);
    void PreCallRecordDestroyDescriptorUpdateTemplateKHR(VkDevice device, VkDescriptorUpdateTemplateKHR descriptorUpdateTemplate,
                                                         const VkAllocationCallbacks* pAllocator);
    bool PreCallValidateUpdateDescriptorSetWithTemplate(VkDevice device, VkDescriptorSet descriptorSet,
                                                        VkDescriptorUpdateTemplate descriptorUpdateTemplate, const void* pData);
    void PreCallRecordUpdateDescriptorSetWithTemplate(VkDevice device, VkDescriptorSet descriptorSet,
                                                      VkDescriptorUpdateTemplate descriptorUpdateTemplate, const void* pData);
    bool PreCallValidateUpdateDescriptorSetWithTemplateKHR(VkDevice device, VkDescriptorSet descriptorSet,
                                                           VkDescriptorUpdateTemplateKHR descriptorUpdateTemplate,
                                                           const void* pData);
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
    bool PreCallValidateCmdSetSampleLocationsEXT(VkCommandBuffer commandBuffer,
                                                 const VkSampleLocationsInfoEXT* pSampleLocationsInfo);
    bool PreCallValidateCmdDrawIndirectCountKHR(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                VkBuffer countBuffer, VkDeviceSize countBufferOffset, uint32_t maxDrawCount,
                                                uint32_t stride);
    void PreCallRecordCmdDrawIndirectCountKHR(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                              VkBuffer countBuffer, VkDeviceSize countBufferOffset, uint32_t maxDrawCount,
                                              uint32_t stride);
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
    void PreCallRecordGetPhysicalDeviceProperties(VkPhysicalDevice physicalDevice,
                                                  VkPhysicalDeviceProperties* pPhysicalDeviceProperties);
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

};  // Class CoreChecks
