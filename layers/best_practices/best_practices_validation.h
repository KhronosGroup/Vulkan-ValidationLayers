/* Copyright (c) 2015-2023 The Khronos Group Inc.
 * Copyright (c) 2015-2023 Valve Corporation
 * Copyright (c) 2015-2023 LunarG, Inc.
 * Modifications Copyright (C) 2020-2022 Advanced Micro Devices, Inc. All rights reserved.
 * Modifications Copyright (C) 2022 RasterGrid Kft.
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

#include "chassis.h"
#include "state_tracker/state_tracker.h"
#include "state_tracker/image_state.h"
#include "state_tracker/cmd_buffer_state.h"
#include <string>
#include <chrono>

static const uint32_t kMemoryObjectWarningLimit = 250;

// Maximum number of instanced vertex buffers which should be used
static const uint32_t kMaxInstancedVertexBuffers = 1;

// Recommended allocation size for vkAllocateMemory
static const VkDeviceSize kMinDeviceAllocationSize = 256 * 1024;

// If a buffer or image is allocated and it consumes an entire VkDeviceMemory, it should at least be this large.
// This is slightly different from minDeviceAllocationSize since the 256K buffer can still be sensibly
// suballocated from. If we consume an entire allocation with one image or buffer, it should at least be for a
// very large allocation.
static const VkDeviceSize kMinDedicatedAllocationSize = 1024 * 1024;

// AMD best practices
// Note: These are initial ball park numbers for good performance
// We expect to adjust them as we get more data on layer usage
// Avoid small command buffers
static const uint32_t kMinRecommendedCommandBufferSizeAMD = 10;
// Avoid small secondary command buffers
static const uint32_t kMinRecommendedDrawsInSecondaryCommandBufferSizeAMD = 10;
// Idealy, only 1 fence per frame, so 3 for triple buffering
static const uint32_t kMaxRecommendedFenceObjectsSizeAMD = 3;
// Avoid excessive sempahores
static const uint32_t kMaxRecommendedSemaphoreObjectsSizeAMD = 10;
// Avoid excessive barriers
static const uint32_t kMaxRecommendedBarriersSizeAMD = 500;
// Avoid excessive pipelines
static const uint32_t kMaxRecommendedNumberOfPSOAMD = 5000;
// Unlikely that the user needs all the dynamic states enabled at the same time, and they encur a cost
static const uint32_t kDynamicStatesWarningLimitAMD = 7;
// Too many dynamic descriptor sets can cause a large pipeline layout
static const uint32_t kPipelineLayoutSizeWarningLimitAMD = 13;
// Check that the user is submitting excessivly to a queue
static const uint32_t kNumberOfSubmissionWarningLimitAMD = 20;
// Check that there is enough work per vertex stream change
static const float kVertexStreamToDrawRatioWarningLimitAMD = 0.8f;
// Check that there is enough work per pipeline change
static const float kDrawsPerPipelineRatioWarningLimitAMD = 5.f;
// Check that command buffers are used with an appropriatly sized pool
static const float kCmdBufferToCmdPoolRatioWarningLimitAMD = 0.1f;
// Size for fast descriptor reads on modern NVIDIA devices
static const uint32_t kPipelineLayoutFastDescriptorSpaceNVIDIA = 256;
// Time threshold for flagging allocations that could have been reused
static const auto kAllocateMemoryReuseTimeThresholdNVIDIA = std::chrono::seconds{5};
// Number of switches in tessellation, gemetry, and mesh shader state before signalling a message
static const uint32_t kNumBindPipelineTessGeometryMeshSwitchesThresholdNVIDIA = 4;
// Ratio where the Z-cull direction starts being considered balanced
static const int kZcullDirectionBalanceRatioNVIDIA = 20;
// Maximum number of custom clear colors
static const size_t kMaxRecommendedNumberOfClearColorsNVIDIA = 16;

// How many small indexed drawcalls in a command buffer before a warning is thrown
static const uint32_t kMaxSmallIndexedDrawcalls = 10;

// How many indices make a small indexed drawcall
static const int kSmallIndexedDrawcallIndices = 10;

// Minimum number of vertices/indices to take into account when doing depth pre-pass checks for Arm Mali GPUs
static const int kDepthPrePassMinDrawCountArm = 500;

// Minimum, number of draw calls in order to trigger depth pre-pass warnings for Arm Mali GPUs
static const int kDepthPrePassNumDrawCallsArm = 20;

// Maximum sample count for full throughput on Mali GPUs
static const VkSampleCountFlagBits kMaxEfficientSamplesArm = VK_SAMPLE_COUNT_4_BIT;

// On Arm Mali architectures, it's generally best to align work group dimensions to 4.
static const uint32_t kThreadGroupDispatchCountAlignmentArm = 4;

// Maximum number of threads which can efficiently be part of a compute workgroup when using thread group barriers.
static const uint32_t kMaxEfficientWorkGroupThreadCountArm = 64;

// Minimum number of vertices/indices a draw needs to have before considering it in depth prepass warnings on PowerVR
static const int kDepthPrePassMinDrawCountIMG = 300;

// Minimum, number of draw calls matching the above criteria before triggerring a depth prepass warning on PowerVR
static const int kDepthPrePassNumDrawCallsIMG = 10;

// Maximum sample count on PowerVR before showing a warning
static const VkSampleCountFlagBits kMaxEfficientSamplesImg = VK_SAMPLE_COUNT_4_BIT;

enum ExtDeprecationReason {
    kExtPromoted,
    kExtObsoleted,
    kExtDeprecated,
};

struct DeprecationData {
    ExtDeprecationReason reason;
    std::string target;
};

struct SpecialUseVUIDs {
    const char* cadsupport;
    const char* d3demulation;
    const char* devtools;
    const char* debugging;
    const char* glemulation;
};

typedef enum {
    kBPVendorArm = 0x00000001,
    kBPVendorAMD = 0x00000002,
    kBPVendorIMG = 0x00000004,
    kBPVendorNVIDIA = 0x00000008,
} BPVendorFlagBits;
typedef VkFlags BPVendorFlags;

enum CALL_STATE {
    UNCALLED,       // Function has not been called
    QUERY_COUNT,    // Function called once to query a count
    QUERY_DETAILS,  // Function called w/ a count to query details
};

enum IMAGE_SUBRESOURCE_USAGE_BP {
    UNDEFINED,  // If it has never been used
    RENDER_PASS_CLEARED,
    RENDER_PASS_READ_TO_TILE,
    CLEARED,
    DESCRIPTOR_ACCESS,
    RENDER_PASS_STORED,
    RENDER_PASS_DISCARDED,
    BLIT_READ,
    BLIT_WRITE,
    RESOLVE_READ,
    RESOLVE_WRITE,
    COPY_READ,
    COPY_WRITE
};

class BestPractices;

namespace bp_state {
class Image : public IMAGE_STATE {
  public:
    Image(const ValidationStateTracker* dev_data, VkImage img, const VkImageCreateInfo* pCreateInfo,
          VkFormatFeatureFlags2KHR features)
        : IMAGE_STATE(dev_data, img, pCreateInfo, features) {
        SetupUsages();
    }

    Image(const ValidationStateTracker* dev_data, VkImage img, const VkImageCreateInfo* pCreateInfo, VkSwapchainKHR swapchain,
          uint32_t swapchain_index, VkFormatFeatureFlags2KHR features)
        : IMAGE_STATE(dev_data, img, pCreateInfo, swapchain, swapchain_index, features) {
        SetupUsages();
    }

    struct Usage {
        IMAGE_SUBRESOURCE_USAGE_BP type;
        uint32_t queue_family_index;
    };

    Usage UpdateUsage(uint32_t array_layer, uint32_t mip_level, IMAGE_SUBRESOURCE_USAGE_BP usage, uint32_t queue_family) {
        auto last_usage = usages_[array_layer][mip_level];
        usages_[array_layer][mip_level].type = usage;
        usages_[array_layer][mip_level].queue_family_index = queue_family;
        return last_usage;
    }

    Usage GetUsage(uint32_t array_layer, uint32_t mip_level) const { return usages_[array_layer][mip_level]; }

    IMAGE_SUBRESOURCE_USAGE_BP GetUsageType(uint32_t array_layer, uint32_t mip_level) const {
        return GetUsage(array_layer, mip_level).type;
    }

    uint32_t GetLastQueueFamily(uint32_t array_layer, uint32_t mip_level) const {
        return GetUsage(array_layer, mip_level).queue_family_index;
    }

  private:
    void SetupUsages() {
        usages_.resize(createInfo.arrayLayers);
        for (auto& mip_vec : usages_) {
            mip_vec.resize(createInfo.mipLevels, {IMAGE_SUBRESOURCE_USAGE_BP::UNDEFINED, VK_QUEUE_FAMILY_IGNORED});
        }
    }
    // A 2d vector for all the array layers and mip levels.
    // This does not split usages per aspect.
    // Aspects are generally read and written together,
    // and tracking them independently could be misleading.
    // second/uint32_t is last queue family usage
    std::vector<std::vector<Usage>> usages_;
};

using ImageNoBinding = MEMORY_TRACKED_RESOURCE_STATE<Image, BindableNoMemoryTracker>;
using ImageLinear = MEMORY_TRACKED_RESOURCE_STATE<Image, BindableLinearMemoryTracker>;
template <bool IS_RESIDENT>
using ImageSparse = MEMORY_TRACKED_RESOURCE_STATE<Image, BindableSparseMemoryTracker<IS_RESIDENT>>;
template <unsigned PLANE_COUNT>
using ImageMultiplanar = MEMORY_TRACKED_RESOURCE_STATE<Image, BindableMultiplanarMemoryTracker<PLANE_COUNT>>;

class PhysicalDevice : public PHYSICAL_DEVICE_STATE {
  public:
    PhysicalDevice(VkPhysicalDevice phys_dev) : PHYSICAL_DEVICE_STATE(phys_dev) {}

    // Track the call state and array sizes for various query functions
    CALL_STATE vkGetPhysicalDeviceQueueFamilyPropertiesState = UNCALLED;
    CALL_STATE vkGetPhysicalDeviceQueueFamilyProperties2State = UNCALLED;
    CALL_STATE vkGetPhysicalDeviceQueueFamilyProperties2KHRState = UNCALLED;
    CALL_STATE vkGetPhysicalDeviceLayerPropertiesState = UNCALLED;      // Currently unused
    CALL_STATE vkGetPhysicalDeviceExtensionPropertiesState = UNCALLED;  // Currently unused
    CALL_STATE vkGetPhysicalDeviceFeaturesState = UNCALLED;
    CALL_STATE vkGetPhysicalDeviceSurfaceCapabilitiesKHRState = UNCALLED;
    CALL_STATE vkGetPhysicalDeviceSurfacePresentModesKHRState = UNCALLED;
    CALL_STATE vkGetPhysicalDeviceSurfaceFormatsKHRState = UNCALLED;
    uint32_t surface_formats_count = 0;
    CALL_STATE vkGetPhysicalDeviceDisplayPlanePropertiesKHRState = UNCALLED;
};

class Swapchain : public SWAPCHAIN_NODE {
  public:
    Swapchain(ValidationStateTracker* dev_data, const VkSwapchainCreateInfoKHR* pCreateInfo, VkSwapchainKHR swapchain)
        : SWAPCHAIN_NODE(dev_data, pCreateInfo, swapchain) {}

    CALL_STATE vkGetSwapchainImagesKHRState = UNCALLED;
};

class DeviceMemory : public DEVICE_MEMORY_STATE {
  public:
    DeviceMemory(VkDeviceMemory mem, const VkMemoryAllocateInfo* p_alloc_info, uint64_t fake_address,
                 const VkMemoryType& memory_type, const VkMemoryHeap& memory_heap,
                 std::optional<DedicatedBinding>&& dedicated_binding, uint32_t physical_device_count)
        : DEVICE_MEMORY_STATE(mem, p_alloc_info, fake_address, memory_type, memory_heap, std::move(dedicated_binding),
                              physical_device_count) {}

    std::optional<float> dynamic_priority;  // VK_EXT_pageable_device_local_memory priority
};

struct AttachmentInfo {
    uint32_t framebufferAttachment;
    VkImageAspectFlags aspects;
};

// used to track state regarding render pass heuristic checks
struct RenderPassState {
    bool depthAttachment = false;
    bool colorAttachment = false;
    bool depthOnly = false;
    bool depthEqualComparison = false;
    uint32_t numDrawCallsDepthOnly = 0;
    uint32_t numDrawCallsDepthEqualCompare = 0;

    // For secondaries, we need to keep this around for execute commands.
    struct ClearInfo {
        uint32_t framebufferAttachment;
        uint32_t colorAttachment;
        VkImageAspectFlags aspects;
        std::vector<VkClearRect> rects;
    };

    std::vector<ClearInfo> earlyClearAttachments;
    std::vector<AttachmentInfo> touchesAttachments;
    std::vector<AttachmentInfo> nextDrawTouchesAttachments;
    bool drawTouchAttachments = false;
};

struct CommandBufferStateNV {
    struct TessGeometryMesh {
        enum class State {
            Unknown,
            Disabled,
            Enabled,
        };

        uint32_t num_switches = 0;
        State state = State::Unknown;
        bool threshold_signaled = false;
    };
    enum class ZcullDirection {
        Unknown,
        Less,
        Greater,
    };
    struct ZcullResourceState {
        ZcullDirection direction = ZcullDirection::Unknown;
        uint64_t num_less_draws = 0;
        uint64_t num_greater_draws = 0;
    };
    struct ZcullTree {
        std::vector<ZcullResourceState> states;
        uint32_t mip_levels = 0;
        uint32_t array_layers = 0;

        const ZcullResourceState& GetState(uint32_t layer, uint32_t level) const { return states[layer * mip_levels + level]; }

        ZcullResourceState& GetState(uint32_t layer, uint32_t level) { return states[layer * mip_levels + level]; }
    };
    struct ZcullScope {
        VkImage image = VK_NULL_HANDLE;
        VkImageSubresourceRange range{};
        ZcullTree* tree = nullptr;
    };

    TessGeometryMesh tess_geometry_mesh;

    std::unordered_map<VkImage, ZcullTree> zcull_per_image;
    ZcullScope zcull_scope;
    ZcullDirection zcull_direction = ZcullDirection::Unknown;

    VkCompareOp depth_compare_op = VK_COMPARE_OP_NEVER;
    bool depth_test_enable = false;
};

class CommandBuffer : public CMD_BUFFER_STATE {
  public:
    CommandBuffer(BestPractices* bp, VkCommandBuffer cb, const VkCommandBufferAllocateInfo* pCreateInfo,
                  const COMMAND_POOL_STATE* pool);

    RenderPassState render_pass_state;
    CommandBufferStateNV nv;
    uint64_t num_submits = 0;
    bool is_one_time_submit = false;
};

class DescriptorPool : public DESCRIPTOR_POOL_STATE {
  public:
    DescriptorPool(ValidationStateTracker* dev, const VkDescriptorPool pool, const VkDescriptorPoolCreateInfo* pCreateInfo)
        : DESCRIPTOR_POOL_STATE(dev, pool, pCreateInfo) {}

    uint32_t freed_count{0};
};

class Pipeline : public PIPELINE_STATE {
  public:
    Pipeline(const ValidationStateTracker* state_data, const VkGraphicsPipelineCreateInfo* pCreateInfo, uint32_t create_index,
             std::shared_ptr<const RENDER_PASS_STATE>&& rpstate, std::shared_ptr<const PIPELINE_LAYOUT_STATE>&& layout,
             CreateShaderModuleStates* csm_states);

    const std::vector<AttachmentInfo> access_framebuffer_attachments;
};
}  // namespace bp_state

VALSTATETRACK_DERIVED_STATE_OBJECT(VkPhysicalDevice, bp_state::PhysicalDevice, PHYSICAL_DEVICE_STATE);
VALSTATETRACK_DERIVED_STATE_OBJECT(VkCommandBuffer, bp_state::CommandBuffer, CMD_BUFFER_STATE);
VALSTATETRACK_DERIVED_STATE_OBJECT(VkSwapchainKHR, bp_state::Swapchain, SWAPCHAIN_NODE);
VALSTATETRACK_DERIVED_STATE_OBJECT(VkImage, bp_state::Image, IMAGE_STATE);
VALSTATETRACK_DERIVED_STATE_OBJECT(VkDescriptorPool, bp_state::DescriptorPool, DESCRIPTOR_POOL_STATE);
VALSTATETRACK_DERIVED_STATE_OBJECT(VkPipeline, bp_state::Pipeline, PIPELINE_STATE);

class BestPractices : public ValidationStateTracker {
  public:
    using StateTracker = ValidationStateTracker;

    BestPractices() { container_type = LayerObjectTypeBestPractices; }

    ReadLockGuard ReadLock() const override;
    WriteLockGuard WriteLock() override;

    std::string GetAPIVersionName(uint32_t version) const;

    bool ValidateCmdDrawType(VkCommandBuffer cmd_buffer, const char* caller) const;

    void RecordCmdDrawType(VkCommandBuffer cmd_buffer, uint32_t draw_count, const char* caller);

    bool ValidateDeprecatedExtensions(const char* api_name, const char* extension_name, uint32_t version, const char* vuid) const;

    bool ValidateSpecialUseExtensions(const char* api_name, const char* extension_name,
                                      const SpecialUseVUIDs& special_use_vuids) const;

    bool PreCallValidateCreateInstance(const VkInstanceCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator,
                                       VkInstance* pInstance) const override;
    bool PreCallValidateCreateDevice(VkPhysicalDevice physicalDevice, const VkDeviceCreateInfo* pCreateInfo,
                                     const VkAllocationCallbacks* pAllocator, VkDevice* pDevice) const override;
    bool PreCallValidateCreateBuffer(VkDevice device, const VkBufferCreateInfo* pCreateInfo,
                                     const VkAllocationCallbacks* pAllocator, VkBuffer* pBuffer) const override;
    bool PreCallValidateCreateImage(VkDevice device, const VkImageCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator,
                                    VkImage* pImage) const override;
    bool PreCallValidateCreateSwapchainKHR(VkDevice device, const VkSwapchainCreateInfoKHR* pCreateInfo,
                                           const VkAllocationCallbacks* pAllocator, VkSwapchainKHR* pSwapchain) const override;
    bool PreCallValidateCreateSharedSwapchainsKHR(VkDevice device, uint32_t swapchainCount,
                                                  const VkSwapchainCreateInfoKHR* pCreateInfos,
                                                  const VkAllocationCallbacks* pAllocator,
                                                  VkSwapchainKHR* pSwapchains) const override;
    bool PreCallValidateCreateRenderPass(VkDevice device, const VkRenderPassCreateInfo* pCreateInfo,
                                         const VkAllocationCallbacks* pAllocator, VkRenderPass* pRenderPass) const override;
    bool ValidateAttachments(const VkRenderPassCreateInfo2* rpci, uint32_t attachmentCount, const VkImageView* image_views) const;
    bool PreCallValidateCreateFramebuffer(VkDevice device, const VkFramebufferCreateInfo* pCreateInfo,
                                          const VkAllocationCallbacks* pAllocator, VkFramebuffer* pFramebuffer) const override;
    bool PreCallValidateAllocateDescriptorSets(VkDevice device, const VkDescriptorSetAllocateInfo* pAllocateInfo,
                                               VkDescriptorSet* pDescriptorSets, void* ads_state_data) const override;
    void ManualPostCallRecordAllocateDescriptorSets(VkDevice device, const VkDescriptorSetAllocateInfo* pAllocateInfo,
                                                    VkDescriptorSet* pDescriptorSets, VkResult result, void* ads_state);
    void PostCallRecordFreeDescriptorSets(VkDevice device, VkDescriptorPool descriptorPool, uint32_t descriptorSetCount,
                                          const VkDescriptorSet* pDescriptorSets, VkResult result) override;
    bool PreCallValidateAllocateMemory(VkDevice device, const VkMemoryAllocateInfo* pAllocateInfo,
                                       const VkAllocationCallbacks* pAllocator, VkDeviceMemory* pMemory) const override;
    void PreCallRecordAllocateMemory(VkDevice device, const VkMemoryAllocateInfo* pAllocateInfo,
                                     const VkAllocationCallbacks* pAllocator, VkDeviceMemory* pMemory) override;
    void ManualPostCallRecordAllocateMemory(VkDevice device, const VkMemoryAllocateInfo* pAllocateInfo,
                                            const VkAllocationCallbacks* pAllocator, VkDeviceMemory* pMemory, VkResult result);
    bool ValidateBindBufferMemory(VkBuffer buffer, VkDeviceMemory memory, const char* api_name) const;
    bool PreCallValidateBindBufferMemory(VkDevice device, VkBuffer buffer, VkDeviceMemory memory,
                                         VkDeviceSize memoryOffset) const override;
    bool PreCallValidateBindBufferMemory2(VkDevice device, uint32_t bindInfoCount,
                                          const VkBindBufferMemoryInfo* pBindInfos) const override;
    bool PreCallValidateBindBufferMemory2KHR(VkDevice device, uint32_t bindInfoCount,
                                             const VkBindBufferMemoryInfo* pBindInfos) const override;
    bool ValidateBindImageMemory(VkImage image, VkDeviceMemory memory, const char* api_name) const;
    bool PreCallValidateBindImageMemory(VkDevice device, VkImage image, VkDeviceMemory memory,
                                        VkDeviceSize memoryOffset) const override;
    bool PreCallValidateBindImageMemory2(VkDevice device, uint32_t bindInfoCount,
                                         const VkBindImageMemoryInfo* pBindInfos) const override;
    bool PreCallValidateBindImageMemory2KHR(VkDevice device, uint32_t bindInfoCount,
                                            const VkBindImageMemoryInfo* pBindInfos) const override;
    void PreCallRecordSetDeviceMemoryPriorityEXT(VkDevice device, VkDeviceMemory memory, float priority) override;
    bool PreCallValidateGetVideoSessionMemoryRequirementsKHR(
        VkDevice device, VkVideoSessionKHR videoSession, uint32_t* pMemoryRequirementsCount,
        VkVideoSessionMemoryRequirementsKHR* pMemoryRequirements) const override;
    bool PreCallValidateBindVideoSessionMemoryKHR(VkDevice device, VkVideoSessionKHR videoSession,
                                                  uint32_t bindSessionMemoryInfoCount,
                                                  const VkBindVideoSessionMemoryInfoKHR* pBindSessionMemoryInfos) const override;
    bool PreCallValidateCreateCommandPool(VkDevice device, const VkCommandPoolCreateInfo* pCreateInfo,
                                          const VkAllocationCallbacks* pAllocator, VkCommandPool* pCommandPool) const override;
    bool PreCallValidateAllocateCommandBuffers(VkDevice device, const VkCommandBufferAllocateInfo* pAllocateInfo,
                                               VkCommandBuffer* pCommandBuffers) const override;
    void PreCallRecordFreeMemory(VkDevice device, VkDeviceMemory memory, const VkAllocationCallbacks* pAllocator) override;
    bool PreCallValidateFreeMemory(VkDevice device, VkDeviceMemory memory, const VkAllocationCallbacks* pAllocator) const override;
    bool ValidateMultisampledBlendingArm(uint32_t createInfoCount, const VkGraphicsPipelineCreateInfo* pCreateInfos) const;

    bool PreCallValidateCreateGraphicsPipelines(VkDevice device, VkPipelineCache pipelineCache, uint32_t createInfoCount,
                                                const VkGraphicsPipelineCreateInfo* pCreateInfos,
                                                const VkAllocationCallbacks* pAllocator, VkPipeline* pPipelines,
                                                void* cgpl_state) const override;
    bool PreCallValidateCreateComputePipelines(VkDevice device, VkPipelineCache pipelineCache, uint32_t createInfoCount,
                                               const VkComputePipelineCreateInfo* pCreateInfos,
                                               const VkAllocationCallbacks* pAllocator, VkPipeline* pPipelines,
                                               void* pipe_state) const override;
    bool ValidateCreateComputePipelineArm(const VkComputePipelineCreateInfo& createInfo) const;

    bool CheckPipelineStageFlags(const std::string& api_name, VkPipelineStageFlags flags) const;
    bool CheckPipelineStageFlags(const std::string& api_name, VkPipelineStageFlags2KHR flags) const;
    bool CheckDependencyInfo(const std::string& api_name, const VkDependencyInfoKHR& dep_info) const;
    bool PreCallValidateQueueSubmit(VkQueue queue, uint32_t submitCount, const VkSubmitInfo* pSubmits,
                                    VkFence fence) const override;
    bool PreCallValidateQueueSubmit2KHR(VkQueue queue, uint32_t submitCount, const VkSubmitInfo2KHR* pSubmits,
                                        VkFence fence) const override;
    bool PreCallValidateQueueSubmit2(VkQueue queue, uint32_t submitCount, const VkSubmitInfo2* pSubmits,
                                     VkFence fence) const override;
    void PreCallRecordBeginCommandBuffer(VkCommandBuffer commandBuffer, const VkCommandBufferBeginInfo* pBeginInfo) override;
    bool PreCallValidateBeginCommandBuffer(VkCommandBuffer commandBuffer,
                                           const VkCommandBufferBeginInfo* pBeginInfo) const override;
    bool PreCallValidateCmdSetEvent(VkCommandBuffer commandBuffer, VkEvent event, VkPipelineStageFlags stageMask) const override;
    bool PreCallValidateCmdSetEvent2KHR(VkCommandBuffer commandBuffer, VkEvent event,
                                        const VkDependencyInfoKHR* pDependencyInfo) const override;
    bool PreCallValidateCmdSetEvent2(VkCommandBuffer commandBuffer, VkEvent event,
                                     const VkDependencyInfo* pDependencyInfo) const override;
    bool PreCallValidateCmdResetEvent(VkCommandBuffer commandBuffer, VkEvent event, VkPipelineStageFlags stageMask) const override;
    bool PreCallValidateCmdResetEvent2KHR(VkCommandBuffer commandBuffer, VkEvent event,
                                          VkPipelineStageFlags2KHR stageMask) const override;
    bool PreCallValidateCmdResetEvent2(VkCommandBuffer commandBuffer, VkEvent event,
                                       VkPipelineStageFlags2 stageMask) const override;
    bool PreCallValidateCmdWaitEvents(VkCommandBuffer commandBuffer, uint32_t eventCount, const VkEvent* pEvents,
                                      VkPipelineStageFlags srcStageMask, VkPipelineStageFlags dstStageMask,
                                      uint32_t memoryBarrierCount, const VkMemoryBarrier* pMemoryBarriers,
                                      uint32_t bufferMemoryBarrierCount, const VkBufferMemoryBarrier* pBufferMemoryBarriers,
                                      uint32_t imageMemoryBarrierCount,
                                      const VkImageMemoryBarrier* pImageMemoryBarriers) const override;
    bool PreCallValidateCmdWaitEvents2KHR(VkCommandBuffer commandBuffer, uint32_t eventCount, const VkEvent* pEvents,
                                          const VkDependencyInfoKHR* pDependencyInfos) const override;
    bool PreCallValidateCmdWaitEvents2(VkCommandBuffer commandBuffer, uint32_t eventCount, const VkEvent* pEvents,
                                       const VkDependencyInfo* pDependencyInfos) const override;
    bool ValidateAccessLayoutCombination(const std::string& api_name, VkAccessFlags2 access, VkImageLayout layout,
                                         VkImageAspectFlags aspect) const;
    bool ValidateImageMemoryBarrier(const std::string& api_name, VkImageLayout oldLayout, VkImageLayout newLayout,
                                    VkAccessFlags2 srcAccessMask, VkAccessFlags2 dstAccessMask,
                                    VkImageAspectFlags aspectMask) const;
    bool PreCallValidateCmdPipelineBarrier(VkCommandBuffer commandBuffer, VkPipelineStageFlags srcStageMask,
                                           VkPipelineStageFlags dstStageMask, VkDependencyFlags dependencyFlags,
                                           uint32_t memoryBarrierCount, const VkMemoryBarrier* pMemoryBarriers,
                                           uint32_t bufferMemoryBarrierCount, const VkBufferMemoryBarrier* pBufferMemoryBarriers,
                                           uint32_t imageMemoryBarrierCount,
                                           const VkImageMemoryBarrier* pImageMemoryBarriers) const override;
    bool PreCallValidateCmdPipelineBarrier2KHR(VkCommandBuffer commandBuffer,
                                               const VkDependencyInfoKHR* pDependencyInfo) const override;
    bool PreCallValidateCmdPipelineBarrier2(VkCommandBuffer commandBuffer, const VkDependencyInfo* pDependencyInfo) const override;

    template <typename ImageMemoryBarrier>
    bool ValidateCmdPipelineBarrierImageBarrier(VkCommandBuffer commandBuffer, const ImageMemoryBarrier& barrier) const;

    bool PreCallValidateCmdWriteTimestamp(VkCommandBuffer commandBuffer, VkPipelineStageFlagBits pipelineStage,
                                          VkQueryPool queryPool, uint32_t query) const override;
    bool PreCallValidateCmdWriteTimestamp2KHR(VkCommandBuffer commandBuffer, VkPipelineStageFlags2KHR pipelineStage,
                                              VkQueryPool queryPool, uint32_t query) const override;
    bool PreCallValidateCmdWriteTimestamp2(VkCommandBuffer commandBuffer, VkPipelineStageFlags2 pipelineStage,
                                           VkQueryPool queryPool, uint32_t query) const override;
    void PreCallRecordCmdBindPipeline(VkCommandBuffer commandBuffer, VkPipelineBindPoint pipelineBindPoint,
                                      VkPipeline pipeline) override;
    void PostCallRecordCmdBindPipeline(VkCommandBuffer commandBuffer, VkPipelineBindPoint pipelineBindPoint,
                                       VkPipeline pipeline) override;
    void PreCallRecordCmdSetDepthCompareOp(VkCommandBuffer commandBuffer, VkCompareOp depthCompareOp) override;
    void PreCallRecordCmdSetDepthCompareOpEXT(VkCommandBuffer commandBuffer, VkCompareOp depthCompareOp) override;
    void PreCallRecordCmdSetDepthTestEnable(VkCommandBuffer commandBuffer, VkBool32 depthTestEnable) override;
    void PreCallRecordCmdSetDepthTestEnableEXT(VkCommandBuffer commandBuffer, VkBool32 depthTestEnable) override;
    bool ValidateCmdBeginRenderPass(VkCommandBuffer commandBuffer, RenderPassCreateVersion rp_version,
                                    const VkRenderPassBeginInfo* pRenderPassBegin) const;
    bool ValidateCmdBeginRendering(VkCommandBuffer commandBuffer, const VkRenderingInfo* pRenderingInfo) const;

    void PreCallRecordCmdBeginRenderPass(VkCommandBuffer commandBuffer, const VkRenderPassBeginInfo* pRenderPassBegin,
                                         VkSubpassContents contents) override;
    void PreCallRecordCmdBeginRenderPass2(VkCommandBuffer commandBuffer, const VkRenderPassBeginInfo* pRenderPassBegin,
                                          const VkSubpassBeginInfo* pSubpassBeginInfo) override;
    void PreCallRecordCmdBeginRenderPass2KHR(VkCommandBuffer commandBuffer, const VkRenderPassBeginInfo* pRenderPassBegin,
                                             const VkSubpassBeginInfo* pSubpassBeginInfo) override;
    void PreCallRecordCmdBeginRendering(VkCommandBuffer commandBuffer, const VkRenderingInfo* pRenderingInfo) override;
    void PreCallRecordCmdBeginRenderingKHR(VkCommandBuffer commandBuffer, const VkRenderingInfo* pRenderingInfo) override;

    void PostCallRecordCmdNextSubpass(VkCommandBuffer commandBuffer, VkSubpassContents contents) override;

    void PreCallRecordCmdEndRenderPass(VkCommandBuffer commandBuffer) override;
    void PreCallRecordCmdEndRenderPass2(VkCommandBuffer commandBuffer, const VkSubpassEndInfo* pSubpassEndInfo) override;
    void PreCallRecordCmdEndRenderPass2KHR(VkCommandBuffer commandBuffer, const VkSubpassEndInfoKHR* pSubpassEndInfo) override;
    void PreCallRecordCmdEndRendering(VkCommandBuffer commandBuffer) override;
    void PreCallRecordCmdEndRenderingKHR(VkCommandBuffer commandBuffer) override;

    bool PreCallValidateCmdBeginRenderPass(VkCommandBuffer commandBuffer, const VkRenderPassBeginInfo* pRenderPassBegin,
                                           VkSubpassContents contents) const override;
    bool PreCallValidateCmdBeginRenderPass2KHR(VkCommandBuffer commandBuffer, const VkRenderPassBeginInfo* pRenderPassBegin,
                                               const VkSubpassBeginInfo* pSubpassBeginInfo) const override;
    bool PreCallValidateCmdBeginRenderPass2(VkCommandBuffer commandBuffer, const VkRenderPassBeginInfo* pRenderPassBegin,
                                            const VkSubpassBeginInfo* pSubpassBeginInfo) const override;
    bool PreCallValidateCmdBeginRendering(VkCommandBuffer commandBuffer, const VkRenderingInfo* pRenderingInfo) const override;
    bool PreCallValidateCmdBeginRenderingKHR(VkCommandBuffer commandBuffer, const VkRenderingInfo* pRenderingInfo) const override;
    void ValidateBoundDescriptorSets(bp_state::CommandBuffer& commandBuffer, VkPipelineBindPoint bind_point,
                                     const char* function_name);
    bool PreCallValidateCmdEndRendering(VkCommandBuffer commandBuffer) const override;
    bool PreCallValidateCmdEndRenderingKHR(VkCommandBuffer commandBuffer) const override;

    void RecordCmdBeginRenderPass(VkCommandBuffer commandBuffer, RenderPassCreateVersion rp_version,
                                  const VkRenderPassBeginInfo* pRenderPassBegin);
    void PostCallRecordCmdBeginRenderPass(VkCommandBuffer commandBuffer, const VkRenderPassBeginInfo* pRenderPassBegin,
                                          VkSubpassContents contents) override;
    void PostCallRecordCmdBeginRenderPass2(VkCommandBuffer commandBuffer, const VkRenderPassBeginInfo* pRenderPassBegin,
                                           const VkSubpassBeginInfo* pSubpassBeginInfo) override;
    void PostCallRecordCmdBeginRenderPass2KHR(VkCommandBuffer commandBuffer, const VkRenderPassBeginInfo* pRenderPassBegin,
                                              const VkSubpassBeginInfo* pSubpassBeginInfo) override;
    bool PreCallValidateCmdDraw(VkCommandBuffer commandBuffer, uint32_t vertexCount, uint32_t instanceCount, uint32_t firstVertex,
                                uint32_t firstInstance) const override;
    void PostCallRecordCmdDraw(VkCommandBuffer commandBuffer, uint32_t vertexCount, uint32_t instanceCount, uint32_t firstVertex,
                               uint32_t firstInstance) override;
    bool PreCallValidateCmdDrawIndexed(VkCommandBuffer commandBuffer, uint32_t indexCount, uint32_t instanceCount,
                                       uint32_t firstIndex, int32_t vertexOffset, uint32_t firstInstance) const override;
    bool ValidateIndexBufferArm(const bp_state::CommandBuffer& cb_state, uint32_t indexCount, uint32_t instanceCount,
                                uint32_t firstIndex, int32_t vertexOffset, uint32_t firstInstance) const;
    void PreCallRecordCmdDrawIndexed(VkCommandBuffer commandBuffer, uint32_t indexCount, uint32_t instanceCount,
                                     uint32_t firstIndex, int32_t vertexOffset, uint32_t firstInstance) override;
    void PostCallRecordCmdDrawIndexed(VkCommandBuffer commandBuffer, uint32_t indexCount, uint32_t instanceCount,
                                      uint32_t firstIndex, int32_t vertexOffset, uint32_t firstInstance) override;
    bool PreCallValidateCmdDrawIndirect(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset, uint32_t drawCount,
                                        uint32_t stride) const override;
    void PostCallRecordCmdDrawIndirect(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset, uint32_t count,
                                       uint32_t stride) override;
    bool PreCallValidateCmdDrawIndexedIndirect(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                               uint32_t drawCount, uint32_t stride) const override;
    void PostCallRecordCmdDrawIndexedIndirect(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset, uint32_t count,
                                              uint32_t stride) override;
    bool PreCallValidateCmdDrawIndexedIndirectCount(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                    VkBuffer countBuffer, VkDeviceSize countBufferOffset, uint32_t maxDrawCount,
                                                    uint32_t stride) const override;
    void PostCallRecordCmdDrawIndexedIndirectCount(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                   VkBuffer countBuffer, VkDeviceSize countBufferOffset, uint32_t maxDrawCount,
                                                   uint32_t stride) override;
    bool PreCallValidateCmdDrawIndexedIndirectCountAMD(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                       VkBuffer countBuffer, VkDeviceSize countBufferOffset, uint32_t maxDrawCount,
                                                       uint32_t stride) const override;
    void PostCallRecordCmdDrawIndexedIndirectCountAMD(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                      VkBuffer countBuffer, VkDeviceSize countBufferOffset, uint32_t maxDrawCount,
                                                      uint32_t stride) override;
    bool PreCallValidateCmdDrawIndexedIndirectCountKHR(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                       VkBuffer countBuffer, VkDeviceSize countBufferOffset, uint32_t maxDrawCount,
                                                       uint32_t stride) const override;
    void PostCallRecordCmdDrawIndexedIndirectCountKHR(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                      VkBuffer countBuffer, VkDeviceSize countBufferOffset, uint32_t maxDrawCount,
                                                      uint32_t stride) override;
    bool PreCallValidateCmdDrawIndirectByteCountEXT(VkCommandBuffer commandBuffer, uint32_t instanceCount, uint32_t firstInstance,
                                                    VkBuffer counterBuffer, VkDeviceSize counterBufferOffset,
                                                    uint32_t counterOffset, uint32_t vertexStride) const override;
    void PostCallRecordCmdDrawIndirectByteCountEXT(VkCommandBuffer commandBuffer, uint32_t instanceCount, uint32_t firstInstance,
                                                   VkBuffer counterBuffer, VkDeviceSize counterBufferOffset, uint32_t counterOffset,
                                                   uint32_t vertexStride) override;
    bool PreCallValidateCmdDrawIndirectCount(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                             VkBuffer countBuffer, VkDeviceSize countBufferOffset, uint32_t maxDrawCount,
                                             uint32_t stride) const override;
    void PostCallRecordCmdDrawIndirectCount(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                            VkBuffer countBuffer, VkDeviceSize countBufferOffset, uint32_t maxDrawCount,
                                            uint32_t stride) override;
    bool PreCallValidateCmdDrawIndirectCountAMD(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                VkBuffer countBuffer, VkDeviceSize countBufferOffset, uint32_t maxDrawCount,
                                                uint32_t stride) const override;
    void PostCallRecordCmdDrawIndirectCountAMD(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                               VkBuffer countBuffer, VkDeviceSize countBufferOffset, uint32_t maxDrawCount,
                                               uint32_t stride) override;
    bool PreCallValidateCmdDrawIndirectCountKHR(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                VkBuffer countBuffer, VkDeviceSize countBufferOffset, uint32_t maxDrawCount,
                                                uint32_t stride) const override;
    void PostCallRecordCmdDrawIndirectCountKHR(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                               VkBuffer countBuffer, VkDeviceSize countBufferOffset, uint32_t maxDrawCount,
                                               uint32_t stride) override;
    bool PreCallValidateCmdDrawMeshTasksIndirectCountNV(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                        VkBuffer countBuffer, VkDeviceSize countBufferOffset, uint32_t maxDrawCount,
                                                        uint32_t stride) const override;
    void PostCallRecordCmdDrawMeshTasksIndirectCountNV(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                       VkBuffer countBuffer, VkDeviceSize countBufferOffset, uint32_t maxDrawCount,
                                                       uint32_t stride) override;
    bool PreCallValidateCmdDrawMeshTasksIndirectNV(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                   uint32_t drawCount, uint32_t stride) const override;
    void PostCallRecordCmdDrawMeshTasksIndirectNV(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                  uint32_t drawCount, uint32_t strCmdDrawMeshTasksIndirectNVide) override;
    bool PreCallValidateCmdDrawMeshTasksNV(VkCommandBuffer commandBuffer, uint32_t taskCount, uint32_t firstTask) const override;
    void PostCallRecordCmdDrawMeshTasksNV(VkCommandBuffer commandBuffer, uint32_t taskCount, uint32_t firstTask) override;
    bool PreCallValidateCmdDrawMultiIndexedEXT(VkCommandBuffer commandBuffer, uint32_t drawCount,
                                               const VkMultiDrawIndexedInfoEXT* pIndexInfo, uint32_t instanceCount,
                                               uint32_t firstInstance, uint32_t stride,
                                               const int32_t* pVertexOffset) const override;
    void PostCallRecordCmdDrawMultiIndexedEXT(VkCommandBuffer commandBuffer, uint32_t drawCount,
                                              const VkMultiDrawIndexedInfoEXT* pIndexInfo, uint32_t instanceCount,
                                              uint32_t firstInstance, uint32_t stride, const int32_t* pVertexOffset) override;
    bool PreCallValidateCmdDrawMultiEXT(VkCommandBuffer commandBuffer, uint32_t drawCount, const VkMultiDrawInfoEXT* pVertexInfo,
                                        uint32_t instanceCount, uint32_t firstInstance, uint32_t stride) const override;
    void PostCallRecordCmdDrawMultiEXT(VkCommandBuffer commandBuffer, uint32_t drawCount, const VkMultiDrawInfoEXT* pVertexInfo,
                                       uint32_t instanceCount, uint32_t firstInstance, uint32_t stride) override;

    bool PreCallValidateCmdDispatch(VkCommandBuffer commandBuffer, uint32_t groupCountX, uint32_t groupCountY,
                                    uint32_t groupCountZ) const override;
    bool PreCallValidateCmdEndRenderPass(VkCommandBuffer commandBuffer) const override;
    bool PreCallValidateCmdEndRenderPass2(VkCommandBuffer commandBuffer, const VkSubpassEndInfo* pSubpassEndInfo) const override;
    bool PreCallValidateCmdEndRenderPass2KHR(VkCommandBuffer commandBuffer, const VkSubpassEndInfo* pSubpassEndInfo) const override;
    void PreCallRecordCmdDraw(VkCommandBuffer commandBuffer, uint32_t vertexCount, uint32_t instanceCount, uint32_t firstVertex,
                              uint32_t firstInstance) override;
    void PreCallRecordCmdDrawIndirect(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset, uint32_t drawCount,
                                      uint32_t stride) override;
    void PreCallRecordCmdDrawIndexedIndirect(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                             uint32_t drawCount, uint32_t stride) override;
    void PreCallRecordCmdDispatch(VkCommandBuffer commandBuffer, uint32_t x, uint32_t y, uint32_t z) override;
    void PreCallRecordCmdDispatchIndirect(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset) override;
    bool ValidateGetPhysicalDeviceDisplayPlanePropertiesKHRQuery(VkPhysicalDevice physicalDevice, const char* api_name) const;
    bool PreCallValidateGetDisplayPlaneSupportedDisplaysKHR(VkPhysicalDevice physicalDevice, uint32_t planeIndex,
                                                            uint32_t* pDisplayCount, VkDisplayKHR* pDisplays) const override;
    bool PreCallValidateGetDisplayPlaneCapabilitiesKHR(VkPhysicalDevice physicalDevice, VkDisplayModeKHR mode, uint32_t planeIndex,
                                                       VkDisplayPlaneCapabilitiesKHR* pCapabilities) const override;
    bool PreCallValidateGetDisplayPlaneCapabilities2KHR(VkPhysicalDevice physicalDevice,
                                                        const VkDisplayPlaneInfo2KHR* pDisplayPlaneInfo,
                                                        VkDisplayPlaneCapabilities2KHR* pCapabilities) const override;
    bool PreCallValidateGetSwapchainImagesKHR(VkDevice device, VkSwapchainKHR swapchain, uint32_t* pSwapchainImageCount,
                                              VkImage* pSwapchainImages) const override;
    bool PreCallValidateGetPhysicalDeviceQueueFamilyProperties(VkPhysicalDevice physicalDevice, uint32_t* pQueueFamilyPropertyCount,
                                                               VkQueueFamilyProperties* pQueueFamilyProperties) const override;
    bool PreCallValidateGetPhysicalDeviceQueueFamilyProperties2(VkPhysicalDevice physicalDevice,
                                                                uint32_t* pQueueFamilyPropertyCount,
                                                                VkQueueFamilyProperties2* pQueueFamilyProperties) const override;
    bool PreCallValidateGetPhysicalDeviceQueueFamilyProperties2KHR(VkPhysicalDevice physicalDevice,
                                                                   uint32_t* pQueueFamilyPropertyCount,
                                                                   VkQueueFamilyProperties2* pQueueFamilyProperties) const override;
    bool PreCallValidateGetPhysicalDeviceSurfaceFormatsKHR(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface,
                                                           uint32_t* pSurfaceFormatCount,
                                                           VkSurfaceFormatKHR* pSurfaceFormats) const override;
    bool ValidateCommonGetPhysicalDeviceQueueFamilyProperties(const PHYSICAL_DEVICE_STATE* pd_state,
                                                              uint32_t requested_queue_family_property_count,
                                                              const CALL_STATE call_state, const char* caller_name) const;
    bool PreCallValidateBindAccelerationStructureMemoryNV(VkDevice device, uint32_t bindInfoCount,
                                                          const VkBindAccelerationStructureMemoryInfoNV* pBindInfos) const override;
    bool PreCallValidateQueueBindSparse(VkQueue queue, uint32_t bindInfoCount, const VkBindSparseInfo* pBindInfo,
                                        VkFence fence) const override;
    void ManualPostCallRecordQueueBindSparse(VkQueue queue, uint32_t bindInfoCount, const VkBindSparseInfo* pBindInfo,
                                             VkFence fence, VkResult result);
    bool PreCallValidateCmdClearAttachments(VkCommandBuffer commandBuffer, uint32_t attachmentCount,
                                            const VkClearAttachment* pAttachments, uint32_t rectCount,
                                            const VkClearRect* pRects) const override;
    void ValidateReturnCodes(const char* api_name, VkResult result, vvl::span<const VkResult> error_codes,
                             vvl::span<const VkResult> success_codes) const;
    bool ValidateCmdResolveImage(VkCommandBuffer command_buffer, VkImage src_image, VkImage dst_image, CMD_TYPE cmd_type) const;
    bool PreCallValidateCmdResolveImage(VkCommandBuffer commandBuffer, VkImage srcImage, VkImageLayout srcImageLayout,
                                        VkImage dstImage, VkImageLayout dstImageLayout, uint32_t regionCount,
                                        const VkImageResolve* pRegions) const override;
    bool PreCallValidateCmdResolveImage2KHR(VkCommandBuffer commandBuffer,
                                            const VkResolveImageInfo2KHR* pResolveImageInfo) const override;
    bool PreCallValidateCmdResolveImage2(VkCommandBuffer commandBuffer,
                                         const VkResolveImageInfo2* pResolveImageInfo) const override;

    using QueueCallbacks = std::vector<CMD_BUFFER_STATE::QueueCallback>;

    void QueueValidateImageView(QueueCallbacks& func, const char* function_name, IMAGE_VIEW_STATE* view,
                                IMAGE_SUBRESOURCE_USAGE_BP usage);
    void QueueValidateImage(QueueCallbacks& func, const char* function_name, std::shared_ptr<bp_state::Image>& state,
                            IMAGE_SUBRESOURCE_USAGE_BP usage, const VkImageSubresourceRange& subresource_range);
    void QueueValidateImage(QueueCallbacks& func, const char* function_name, std::shared_ptr<bp_state::Image>& state,
                            IMAGE_SUBRESOURCE_USAGE_BP usage, const VkImageSubresourceLayers& range);
    void QueueValidateImage(QueueCallbacks& func, const char* function_name, std::shared_ptr<bp_state::Image>& state,
                            IMAGE_SUBRESOURCE_USAGE_BP usage, uint32_t array_layer, uint32_t mip_level);
    void ValidateImageInQueue(const QUEUE_STATE& qs, const CMD_BUFFER_STATE& cbs, const char* function_name, bp_state::Image& state,
                              IMAGE_SUBRESOURCE_USAGE_BP usage, uint32_t array_layer, uint32_t mip_level);
    void ValidateImageInQueueArmImg(const char* function_name, const bp_state::Image& image, IMAGE_SUBRESOURCE_USAGE_BP last_usage,
                                    IMAGE_SUBRESOURCE_USAGE_BP usage, uint32_t array_layer, uint32_t mip_level);

    void PreCallRecordCmdResolveImage(VkCommandBuffer commandBuffer, VkImage srcImage, VkImageLayout srcImageLayout,
                                      VkImage dstImage, VkImageLayout dstImageLayout, uint32_t regionCount,
                                      const VkImageResolve* pRegions) override;
    void PreCallRecordCmdResolveImage2KHR(VkCommandBuffer commandBuffer, const VkResolveImageInfo2KHR* pResolveImageInfo) override;
    void PreCallRecordCmdResolveImage2(VkCommandBuffer commandBuffer, const VkResolveImageInfo2* pResolveImageInfo) override;
    void PreCallRecordCmdClearColorImage(VkCommandBuffer commandBuffer, VkImage image, VkImageLayout imageLayout,
                                         const VkClearColorValue* pColor, uint32_t rangeCount,
                                         const VkImageSubresourceRange* pRanges) override;
    void PreCallRecordCmdClearDepthStencilImage(VkCommandBuffer commandBuffer, VkImage image, VkImageLayout imageLayout,
                                                const VkClearDepthStencilValue* pDepthStencil, uint32_t rangeCount,
                                                const VkImageSubresourceRange* pRanges) override;
    void PreCallRecordCmdCopyImage(VkCommandBuffer commandBuffer, VkImage srcImage, VkImageLayout srcImageLayout, VkImage dstImage,
                                   VkImageLayout dstImageLayout, uint32_t regionCount, const VkImageCopy* pRegions) override;
    void PreCallRecordCmdCopyBufferToImage(VkCommandBuffer commandBuffer, VkBuffer srcBuffer, VkImage dstImage,
                                           VkImageLayout dstImageLayout, uint32_t regionCount,
                                           const VkBufferImageCopy* pRegions) override;
    void PreCallRecordCmdCopyImageToBuffer(VkCommandBuffer commandBuffer, VkImage srcImage, VkImageLayout srcImageLayout,
                                           VkBuffer dstBuffer, uint32_t regionCount, const VkBufferImageCopy* pRegions) override;
    void PreCallRecordCmdBlitImage(VkCommandBuffer commandBuffer, VkImage srcImage, VkImageLayout srcImageLayout, VkImage dstImage,
                                   VkImageLayout dstImageLayout, uint32_t regionCount, const VkImageBlit* pRegions,
                                   VkFilter filter) override;
    template <typename RegionType>
    bool ValidateCmdBlitImage(VkCommandBuffer command_buffer, uint32_t region_count, const RegionType* regions,
                              CMD_TYPE cmd_type) const;
    bool PreCallValidateCmdBlitImage(VkCommandBuffer commandBuffer, VkImage srcImage, VkImageLayout srcImageLayout,
                                     VkImage dstImage, VkImageLayout dstImageLayout, uint32_t regionCount,
                                     const VkImageBlit* pRegions, VkFilter filter) const override;
    bool PreCallValidateCmdBlitImage2KHR(VkCommandBuffer commandBuffer, const VkBlitImageInfo2KHR* pBlitImageInfo) const override;
    bool PreCallValidateCmdBlitImage2(VkCommandBuffer commandBuffer, const VkBlitImageInfo2* pBlitImageInfo) const override;

    bool PreCallValidateCreateSampler(VkDevice device, const VkSamplerCreateInfo* pCreateInfo,
                                      const VkAllocationCallbacks* pAllocator, VkSampler* pSampler) const override;
    void ManualPostCallRecordQueuePresentKHR(VkQueue queue, const VkPresentInfoKHR* pPresentInfo, VkResult result);
    void ManualPostCallRecordCreateGraphicsPipelines(VkDevice device, VkPipelineCache pipelineCache, uint32_t count,
                                                     const VkGraphicsPipelineCreateInfo* pCreateInfos,
                                                     const VkAllocationCallbacks* pAllocator, VkPipeline* pPipelines,
                                                     VkResult result, void* cgpl_state_data);

    bool PreCallValidateAcquireNextImageKHR(VkDevice device, VkSwapchainKHR swapchain, uint64_t timeout, VkSemaphore semaphore,
                                            VkFence fence, uint32_t* pImageIndex) const override;

    void CommonPostCallRecordGetPhysicalDeviceQueueFamilyProperties(CALL_STATE& call_state, bool no_pointer);
    void PostCallRecordGetPhysicalDeviceQueueFamilyProperties(VkPhysicalDevice physicalDevice, uint32_t* pQueueFamilyPropertyCount,
                                                              VkQueueFamilyProperties* pQueueFamilyProperties) override;

    void PostCallRecordGetPhysicalDeviceQueueFamilyProperties2(VkPhysicalDevice physicalDevice, uint32_t* pQueueFamilyPropertyCount,
                                                               VkQueueFamilyProperties2* pQueueFamilyProperties) override;

    void PostCallRecordGetPhysicalDeviceQueueFamilyProperties2KHR(VkPhysicalDevice physicalDevice,
                                                                  uint32_t* pQueueFamilyPropertyCount,
                                                                  VkQueueFamilyProperties2* pQueueFamilyProperties) override;

    void PostCallRecordGetPhysicalDeviceFeatures(VkPhysicalDevice physicalDevice, VkPhysicalDeviceFeatures* pFeatures) override;

    void PostCallRecordGetPhysicalDeviceFeatures2(VkPhysicalDevice physicalDevice, VkPhysicalDeviceFeatures2* pFeatures) override;

    void PostCallRecordGetPhysicalDeviceFeatures2KHR(VkPhysicalDevice physicalDevice,
                                                     VkPhysicalDeviceFeatures2* pFeatures) override;

    void ManualPostCallRecordGetPhysicalDeviceSurfaceCapabilitiesKHR(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface,
                                                                     VkSurfaceCapabilitiesKHR* pSurfaceCapabilities,
                                                                     VkResult result);

    void ManualPostCallRecordGetPhysicalDeviceSurfaceCapabilities2KHR(VkPhysicalDevice physicalDevice,
                                                                      const VkPhysicalDeviceSurfaceInfo2KHR* pSurfaceInfo,
                                                                      VkSurfaceCapabilities2KHR* pSurfaceCapabilities,
                                                                      VkResult result);

    void ManualPostCallRecordGetPhysicalDeviceSurfaceCapabilities2EXT(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface,
                                                                      VkSurfaceCapabilities2EXT* pSurfaceCapabilities,
                                                                      VkResult result);

    void ManualPostCallRecordGetPhysicalDeviceSurfacePresentModesKHR(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface,
                                                                     uint32_t* pPresentModeCount, VkPresentModeKHR* pPresentModes,
                                                                     VkResult result);

    void ManualPostCallRecordGetPhysicalDeviceSurfaceFormatsKHR(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface,
                                                                uint32_t* pSurfaceFormatCount, VkSurfaceFormatKHR* pSurfaceFormats,
                                                                VkResult result);

    void ManualPostCallRecordGetPhysicalDeviceSurfaceFormats2KHR(VkPhysicalDevice physicalDevice,
                                                                 const VkPhysicalDeviceSurfaceInfo2KHR* pSurfaceInfo,
                                                                 uint32_t* pSurfaceFormatCount,
                                                                 VkSurfaceFormat2KHR* pSurfaceFormats, VkResult result);

    void ManualPostCallRecordGetPhysicalDeviceDisplayPlanePropertiesKHR(VkPhysicalDevice physicalDevice, uint32_t* pPropertyCount,
                                                                        VkDisplayPlanePropertiesKHR* pProperties, VkResult result);

    void ManualPostCallRecordGetSwapchainImagesKHR(VkDevice device, VkSwapchainKHR swapchain, uint32_t* pSwapchainImageCount,
                                                   VkImage* pSwapchainImages, VkResult result);

    void ManualPostCallRecordQueueSubmit(VkQueue queue, uint32_t submitCount, const VkSubmitInfo* pSubmits, VkFence fence,
                                         VkResult result);

    void ManualPostCallRecordEndCommandBuffer(VkCommandBuffer commandBuffer, VkResult result);

    void ManualPostCallRecordCreateComputePipelines(VkDevice device, VkPipelineCache pipelineCache, uint32_t createInfoCount,
                                                    const VkComputePipelineCreateInfo* pCreateInfos,
                                                    const VkAllocationCallbacks* pAllocator, VkPipeline* pPipelines,
                                                    VkResult result, void* state_data);

    void PostCallRecordCmdPipelineBarrier(VkCommandBuffer commandBuffer, VkPipelineStageFlags srcStageMask,
                                          VkPipelineStageFlags dstStageMask, VkDependencyFlags dependencyFlags,
                                          uint32_t memoryBarrierCount, const VkMemoryBarrier* pMemoryBarriers,
                                          uint32_t bufferMemoryBarrierCount, const VkBufferMemoryBarrier* pBufferMemoryBarriers,
                                          uint32_t imageMemoryBarrierCount,
                                          const VkImageMemoryBarrier* pImageMemoryBarriers) override;
    void PostCallRecordCmdPipelineBarrier2(VkCommandBuffer commandBuffer, const VkDependencyInfo* pDependencyInfo) override;
    void PostCallRecordCmdPipelineBarrier2KHR(VkCommandBuffer commandBuffer, const VkDependencyInfo* pDependencyInfo) override;
    template <typename ImageMemoryBarrier>
    void RecordCmdPipelineBarrierImageBarrier(VkCommandBuffer commandBuffer, const ImageMemoryBarrier& barrier);

    void PreCallRecordCreateGraphicsPipelines(VkDevice device, VkPipelineCache pipelineCache, uint32_t createInfoCount,
                                              const VkGraphicsPipelineCreateInfo* pCreateInfos,
                                              const VkAllocationCallbacks* pAllocator, VkPipeline* pPipelines,
                                              void* cgpl_state) override;

    bool PreCallValidateUpdateDescriptorSets(VkDevice device, uint32_t descriptorWriteCount,
                                             const VkWriteDescriptorSet* pDescriptorWrites, uint32_t descriptorCopyCount,
                                             const VkCopyDescriptorSet* pDescriptorCopies) const override;
    bool PreCallValidateCreateDescriptorUpdateTemplate(VkDevice device, const VkDescriptorUpdateTemplateCreateInfo* pCreateInfo,
                                                       const VkAllocationCallbacks* pAllocator,
                                                       VkDescriptorUpdateTemplate* pDescriptorUpdateTemplate) const override;
    bool PreCallValidateCmdClearColorImage(VkCommandBuffer commandBuffer, VkImage image, VkImageLayout imageLayout,
                                           const VkClearColorValue* pColor, uint32_t rangeCount,
                                           const VkImageSubresourceRange* pRanges) const override;

    bool PreCallValidateCmdClearDepthStencilImage(VkCommandBuffer commandBuffer, VkImage image, VkImageLayout imageLayout,
                                                  const VkClearDepthStencilValue* pDepthStencil, uint32_t rangeCount,
                                                  const VkImageSubresourceRange* pRanges) const override;

    bool PreCallValidateCreatePipelineLayout(VkDevice device, const VkPipelineLayoutCreateInfo* pCreateInfo,
                                             const VkAllocationCallbacks* pAllocator,
                                             VkPipelineLayout* pPipelineLayout) const override;

    bool PreCallValidateCmdCopyImage(VkCommandBuffer commandBuffer, VkImage srcImage, VkImageLayout srcImageLayout,
                                     VkImage dstImage, VkImageLayout dstImageLayout, uint32_t regionCount,
                                     const VkImageCopy* pRegions) const override;

    bool PreCallValidateCmdBindPipeline(VkCommandBuffer commandBuffer, VkPipelineBindPoint pipelineBindPoint,
                                        VkPipeline pipeline) const override;

    bool PreCallValidateQueuePresentKHR(VkQueue queue, const VkPresentInfoKHR* pPresentInfo) const override;

    bool PreCallValidateCreateSemaphore(VkDevice device, const VkSemaphoreCreateInfo* pCreateInfo,
                                        const VkAllocationCallbacks* pAllocator, VkSemaphore* pSemaphore) const override;
    bool PreCallValidateCreateFence(VkDevice device, const VkFenceCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator,
                                    VkFence* pFence) const override;

    void PreCallRecordQueueSubmit(VkQueue queue, uint32_t submitCount, const VkSubmitInfo* pSubmits, VkFence fence) override;

    void PreCallRecordCmdClearAttachments(VkCommandBuffer commandBuffer, uint32_t attachmentCount,
                                          const VkClearAttachment* pClearAttachments, uint32_t rectCount,
                                          const VkClearRect* pRects) override;

    bool PreCallValidateCmdExecuteCommands(VkCommandBuffer commandBuffer, uint32_t commandBufferCount,
                                           const VkCommandBuffer* pCommandBuffers) const override;
    void PreCallRecordCmdExecuteCommands(VkCommandBuffer commandBuffer, uint32_t commandBufferCount,
                                         const VkCommandBuffer* pCommandBuffers) override;

    bool PreCallValidateCmdBuildAccelerationStructureNV(VkCommandBuffer commandBuffer, const VkAccelerationStructureInfoNV* pInfo,
                                                        VkBuffer instanceData, VkDeviceSize instanceOffset, VkBool32 update,
                                                        VkAccelerationStructureNV dst, VkAccelerationStructureNV src,
                                                        VkBuffer scratch, VkDeviceSize scratchOffset) const override;
    bool PreCallValidateCmdBuildAccelerationStructuresIndirectKHR(VkCommandBuffer commandBuffer, uint32_t infoCount,
                                                                  const VkAccelerationStructureBuildGeometryInfoKHR* pInfos,
                                                                  const VkDeviceAddress* pIndirectDeviceAddresses,
                                                                  const uint32_t* pIndirectStrides,
                                                                  const uint32_t* const* ppMaxPrimitiveCounts) const override;
    bool PreCallValidateCmdBuildAccelerationStructuresKHR(
        VkCommandBuffer commandBuffer, uint32_t infoCount, const VkAccelerationStructureBuildGeometryInfoKHR* pInfos,
        const VkAccelerationStructureBuildRangeInfoKHR* const* ppBuildRangeInfos) const override;

// Include code-generated functions
#include "best_practices.h"
  protected:
    std::shared_ptr<CMD_BUFFER_STATE> CreateCmdBufferState(VkCommandBuffer cb, const VkCommandBufferAllocateInfo* create_info,
                                                           const COMMAND_POOL_STATE* pool) final;

    std::shared_ptr<SWAPCHAIN_NODE> CreateSwapchainState(const VkSwapchainCreateInfoKHR* create_info,
                                                         VkSwapchainKHR swapchain) final {
        return std::static_pointer_cast<SWAPCHAIN_NODE>(std::make_shared<bp_state::Swapchain>(this, create_info, swapchain));
    }

    std::shared_ptr<PHYSICAL_DEVICE_STATE> CreatePhysicalDeviceState(VkPhysicalDevice phys_dev) final {
        return std::static_pointer_cast<PHYSICAL_DEVICE_STATE>(std::make_shared<bp_state::PhysicalDevice>(phys_dev));
    }

    std::shared_ptr<IMAGE_STATE> CreateImageState(VkImage img, const VkImageCreateInfo* pCreateInfo,
                                                  VkFormatFeatureFlags2KHR features) final {
        std::shared_ptr<bp_state::Image> state;

        if (pCreateInfo->flags & VK_IMAGE_CREATE_SPARSE_BINDING_BIT) {
            if (pCreateInfo->flags & VK_IMAGE_CREATE_SPARSE_RESIDENCY_BIT) {
                state = std::make_shared<bp_state::ImageSparse<true>>(this, img, pCreateInfo, features);
            } else {
                state = std::make_shared<bp_state::ImageSparse<false>>(this, img, pCreateInfo, features);
            }
        } else if (pCreateInfo->flags & VK_IMAGE_CREATE_DISJOINT_BIT) {
            uint32_t plane_count = FormatPlaneCount(pCreateInfo->format);
            switch (plane_count) {
                case 3:
                    state = std::make_shared<bp_state::ImageMultiplanar<3>>(this, img, pCreateInfo, features);
                    break;
                case 2:
                    state = std::make_shared<bp_state::ImageMultiplanar<2>>(this, img, pCreateInfo, features);
                    break;
                case 1:
                    state = std::make_shared<bp_state::ImageMultiplanar<1>>(this, img, pCreateInfo, features);
                    break;
                default:
                    // Not supported
                    assert(false);
            }
        } else {
            state = std::make_shared<bp_state::ImageLinear>(this, img, pCreateInfo, features);
        }

        return state;
    }

    std::shared_ptr<IMAGE_STATE> CreateImageState(VkImage img, const VkImageCreateInfo* pCreateInfo, VkSwapchainKHR swapchain,
                                                  uint32_t swapchain_index, VkFormatFeatureFlags2KHR features) final {
        return std::static_pointer_cast<IMAGE_STATE>(
            std::make_shared<bp_state::ImageNoBinding>(this, img, pCreateInfo, swapchain, swapchain_index, features));
    }

    std::shared_ptr<DESCRIPTOR_POOL_STATE> CreateDescriptorPoolState(VkDescriptorPool pool,
                                                                     const VkDescriptorPoolCreateInfo* pCreateInfo) final {
        return std::static_pointer_cast<DESCRIPTOR_POOL_STATE>(std::make_shared<bp_state::DescriptorPool>(this, pool, pCreateInfo));
    }

    std::shared_ptr<DEVICE_MEMORY_STATE> CreateDeviceMemoryState(VkDeviceMemory mem, const VkMemoryAllocateInfo* p_alloc_info,
                                                                 uint64_t fake_address, const VkMemoryType& memory_type,
                                                                 const VkMemoryHeap& memory_heap,
                                                                 std::optional<DedicatedBinding>&& dedicated_binding,
                                                                 uint32_t physical_device_count) final {
        return std::static_pointer_cast<DEVICE_MEMORY_STATE>(std::make_shared<bp_state::DeviceMemory>(
            mem, p_alloc_info, fake_address, memory_type, memory_heap, std::move(dedicated_binding), physical_device_count));
    }

    std::shared_ptr<PIPELINE_STATE> CreateGraphicsPipelineState(const VkGraphicsPipelineCreateInfo* pCreateInfo,
                                                                uint32_t create_index,
                                                                std::shared_ptr<const RENDER_PASS_STATE>&& render_pass,
                                                                std::shared_ptr<const PIPELINE_LAYOUT_STATE>&& layout,
                                                                CreateShaderModuleStates* csm_states) const final;

  private:
    // CacheEntry and PostTransformLRUCacheModel are used on the stack
    struct CacheEntry {
        uint32_t value;
        uint32_t age;
    };

    class PostTransformLRUCacheModel {
      public:
        typedef std::vector<CacheEntry>::iterator cache_iterator;

        void resize(size_t size);

        // Returns true if there was a cache hit - also models LRU behavior which will effect subsequent calls.
        bool query_cache(uint32_t value);

      private:
        std::vector<CacheEntry> _entries = {};
        uint32_t iteration = 0;
    };

    // Check that vendor-specific checks are enabled for at least one of the vendors
    bool VendorCheckEnabled(BPVendorFlags vendors) const;

    void RecordCmdDrawTypeArm(bp_state::CommandBuffer& cmd_state, uint32_t draw_count, const char* caller);
    void RecordCmdDrawTypeNVIDIA(bp_state::CommandBuffer& cmd_state);

    void AddDeferredQueueOperations(bp_state::CommandBuffer& cb);

    // Get BestPractices-specific for the current instance
    bp_state::PhysicalDevice* GetPhysicalDeviceState() { return static_cast<bp_state::PhysicalDevice*>(physical_device_state); }
    const bp_state::PhysicalDevice* GetPhysicalDeviceState() const {
        return static_cast<const bp_state::PhysicalDevice*>(physical_device_state);
    }

    void RecordAttachmentClearAttachments(bp_state::CommandBuffer& cmd_state, uint32_t fb_attachment, uint32_t color_attachment,
                                          VkImageAspectFlags aspects, uint32_t rectCount, const VkClearRect* pRects);
    void RecordAttachmentAccess(bp_state::CommandBuffer& cmd_state, uint32_t attachment, VkImageAspectFlags aspects);
    bool ClearAttachmentsIsFullClear(const bp_state::CommandBuffer& cmd, uint32_t rectCount, const VkClearRect* pRects) const;
    bool ValidateClearAttachment(const bp_state::CommandBuffer& cmd, uint32_t fb_attachment, uint32_t color_attachment,
                                 VkImageAspectFlags aspects, bool secondary) const;

    bool ValidateCmdEndRenderPass(VkCommandBuffer commandBuffer) const;
    void RecordCmdBeginRenderPass(VkCommandBuffer commandBuffer, const VkRenderPassBeginInfo* pRenderPassBegin);

    bool ValidateBuildAccelerationStructure(VkCommandBuffer commandBuffer) const;

    bool ValidateBindMemory(VkDevice device, VkDeviceMemory memory) const;

    void RecordSetDepthTestState(bp_state::CommandBuffer& cmd_state, VkCompareOp new_depth_compare_op, bool new_depth_test_enable);

    void RecordCmdBeginRenderingCommon(VkCommandBuffer commandBuffer);
    void RecordCmdEndRenderingCommon(VkCommandBuffer commandBuffer);

    void RecordBindZcullScope(bp_state::CommandBuffer& cmd_state, VkImage depth_attachment,
                              const VkImageSubresourceRange& subresource_range);
    void RecordUnbindZcullScope(bp_state::CommandBuffer& cmd_state);
    void RecordResetScopeZcullDirection(bp_state::CommandBuffer& cmd_state);
    void RecordResetZcullDirection(bp_state::CommandBuffer& cmd_state, VkImage depth_image,
                                   const VkImageSubresourceRange& subresource_range);

    void RecordSetScopeZcullDirection(bp_state::CommandBuffer& cmd_state, bp_state::CommandBufferStateNV::ZcullDirection mode);
    void RecordSetZcullDirection(bp_state::CommandBuffer& cmd_state, VkImage depth_image,
                                 const VkImageSubresourceRange& subresource_range,
                                 bp_state::CommandBufferStateNV::ZcullDirection mode);

    void RecordZcullDraw(bp_state::CommandBuffer& cmd_state);

    bool ValidateZcullScope(const bp_state::CommandBuffer& cmd_state) const;
    bool ValidateZcull(const bp_state::CommandBuffer& cmd_state, VkImage image,
                       const VkImageSubresourceRange& subresource_range) const;

    void RecordClearColor(VkFormat format, const VkClearColorValue& clear_value);
    bool ValidateClearColor(VkCommandBuffer commandBuffer, VkFormat format, const VkClearColorValue& clear_value) const;

    void PipelineUsedInFrame(VkPipeline pipeline) {
        WriteLockGuard guard(pipeline_lock_);
        pipelines_used_in_frame_.insert(pipeline);
    }

    void ClearPipelinesUsedInFrame() {
        WriteLockGuard guard(pipeline_lock_);
        pipelines_used_in_frame_.clear();
    }

    bool IsPipelineUsedInFrame(VkPipeline pipeline) const {
        ReadLockGuard guard(pipeline_lock_);
        return pipelines_used_in_frame_.count(pipeline) != 0;
    }

    // AMD tracked
    std::atomic<uint32_t> num_barriers_objects_{0};
    std::atomic<uint32_t> num_pso_{0};
    std::atomic<uint32_t> num_queue_submissions_{0};

    std::atomic<VkPipelineCache> pipeline_cache_{VK_NULL_HANDLE};

    // NVIDIA tracked
    struct MemoryFreeEvent {
        typename std::chrono::high_resolution_clock::time_point time{};
        VkDeviceSize allocation_size = 0;
        uint32_t memory_type_index = 0;
    };
    std::deque<MemoryFreeEvent> memory_free_events_;
    mutable std::shared_mutex memory_free_events_lock_;

    std::set<std::array<uint32_t, 4>> clear_colors_;
    mutable std::shared_mutex clear_colors_lock_;

    vvl::unordered_set<VkPipeline> pipelines_used_in_frame_;
    mutable std::shared_mutex pipeline_lock_;
};
