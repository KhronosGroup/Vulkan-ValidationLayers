/* Copyright (c) 2015-2021 The Khronos Group Inc.
 * Copyright (c) 2015-2021 Valve Corporation
 * Copyright (c) 2015-2021 LunarG, Inc.
 * Modifications Copyright (C) 2020 Advanced Micro Devices, Inc. All rights reserved.
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
 * Author: Camden Stocker <camden@lunarg.com>
 * Author: Nadav Geva <nadav.geva@amd.com>
 */

#pragma once

#include "chassis.h"
#include "state_tracker.h"
#include "image_state.h"
#include "cmd_buffer_state.h"
#include <string>

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

enum ExtDeprecationReason {
    kExtPromoted,
    kExtObsoleted,
    kExtDeprecated,
};

struct DeprecationData {
    ExtDeprecationReason reason;
    std::string target;
};

struct SpecialUseVUIDs
{
    const char* cadsupport;
    const char* d3demulation;
    const char* devtools;
    const char* debugging;
    const char* glemulation;
};

typedef enum {
    kBPVendorArm = 0x00000001,
    kBPVendorAMD = 0x00000002,
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

struct IMAGE_STATE_BP {
    // A 2d vector for all the array layers and mip levels.
    // This does not split usages per aspect.
    // Aspects are generally read and written together,
    // and tracking them independently could be misleading.
    std::vector<std::vector<IMAGE_SUBRESOURCE_USAGE_BP>> usages;
    IMAGE_STATE* image{nullptr};
};

struct PHYSICAL_DEVICE_STATE_BP {
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
    CALL_STATE vkGetPhysicalDeviceDisplayPlanePropertiesKHRState = UNCALLED;
};

class SWAPCHAIN_STATE_BP : public SWAPCHAIN_NODE {
  public:
    SWAPCHAIN_STATE_BP(ValidationStateTracker* dev_data, const VkSwapchainCreateInfoKHR* pCreateInfo, VkSwapchainKHR swapchain)
        : SWAPCHAIN_NODE(dev_data, pCreateInfo, swapchain) {}
    CALL_STATE vkGetSwapchainImagesKHRState = UNCALLED;
};

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

class BestPractices;

class CMD_BUFFER_STATE_BP : public CMD_BUFFER_STATE {
  public:
    RenderPassState render_pass_state;

    CMD_BUFFER_STATE_BP(BestPractices* bp, VkCommandBuffer cb, const VkCommandBufferAllocateInfo* pCreateInfo,
                        std::shared_ptr<COMMAND_POOL_STATE>& pool);
};

class BestPractices : public ValidationStateTracker {
  public:
    using StateTracker = ValidationStateTracker;

    BestPractices() { container_type = LayerObjectTypeBestPractices; }

    std::string GetAPIVersionName(uint32_t version) const;

    void InitDeviceValidationObject(bool add_obj, ValidationObject* inst_obj, ValidationObject* dev_obj) override;

    bool ValidateCmdDrawType(VkCommandBuffer cmd_buffer, const char* caller) const;

    void RecordCmdDrawType(VkCommandBuffer cmd_buffer, uint32_t draw_count, const char* caller);

    bool ValidateDeprecatedExtensions(const char* api_name, const char* extension_name, uint32_t version, const char* vuid) const;

    bool ValidateSpecialUseExtensions(const char* api_name, const char* extension_name, const SpecialUseVUIDs& special_use_vuids) const;

    bool PreCallValidateCmdDrawIndexedIndirectCount(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                    VkBuffer countBuffer, VkDeviceSize countBufferOffset, uint32_t maxDrawCount,
                                                    uint32_t stride) const override;

    bool PreCallValidateCmdDrawIndexedIndirectCountKHR(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                       VkBuffer countBuffer, VkDeviceSize countBufferOffset, uint32_t maxDrawCount,
                                                       uint32_t stride) const override;

    bool PreCallValidateCreateInstance(const VkInstanceCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator,
                                       VkInstance* pInstance) const override;
    void PreCallRecordCreateInstance(const VkInstanceCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator,
                                     VkInstance* pInstance) override;
    bool PreCallValidateCreateDevice(VkPhysicalDevice physicalDevice, const VkDeviceCreateInfo* pCreateInfo,
                                     const VkAllocationCallbacks* pAllocator, VkDevice* pDevice) const override;
    bool PreCallValidateCreateBuffer(VkDevice device, const VkBufferCreateInfo* pCreateInfo,
                                     const VkAllocationCallbacks* pAllocator, VkBuffer* pBuffer) const override;
    bool PreCallValidateCreateImage(VkDevice device, const VkImageCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator,
                                    VkImage* pImage) const override;
    void PreCallRecordDestroyImage(VkDevice device, VkImage image, const VkAllocationCallbacks *pAllocator) override;
    void PreCallRecordDestroySwapchainKHR(VkDevice device, VkSwapchainKHR swapchain, const VkAllocationCallbacks *pAllocator) override;
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
    void ManualPostCallRecordAllocateMemory(VkDevice device, const VkMemoryAllocateInfo* pAllocateInfo,
                                            const VkAllocationCallbacks* pAllocator, VkDeviceMemory* pMemory, VkResult result);
    void PreCallRecordFreeMemory(VkDevice device, VkDeviceMemory memory, const VkAllocationCallbacks* pAllocator) override;
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
    bool PreCallValidateCreateCommandPool(VkDevice device, const VkCommandPoolCreateInfo* pCreateInfo,
                                          const VkAllocationCallbacks* pAllocator, VkCommandPool* pCommandPool) const override;
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
    void PreCallRecordDestroyPipeline(VkDevice device, VkPipeline pipeline, const VkAllocationCallbacks *pAllocator) override;

    bool CheckPipelineStageFlags(const std::string& api_name, VkPipelineStageFlags flags) const;
    bool CheckPipelineStageFlags(const std::string& api_name, VkPipelineStageFlags2KHR flags) const;
    bool CheckDependencyInfo(const std::string& api_name, const VkDependencyInfoKHR& dep_info) const;
    bool PreCallValidateQueueSubmit(VkQueue queue, uint32_t submitCount, const VkSubmitInfo* pSubmits,
                                    VkFence fence) const override;
    bool PreCallValidateQueueSubmit2KHR(VkQueue queue, uint32_t submitCount, const VkSubmitInfo2KHR* pSubmits,
                                        VkFence fence) const override;
    bool PreCallValidateBeginCommandBuffer(VkCommandBuffer commandBuffer,
                                           const VkCommandBufferBeginInfo* pBeginInfo) const override;
    bool PreCallValidateCmdSetEvent(VkCommandBuffer commandBuffer, VkEvent event, VkPipelineStageFlags stageMask) const override;
    bool PreCallValidateCmdSetEvent2KHR(VkCommandBuffer commandBuffer, VkEvent event,
                                        const VkDependencyInfoKHR* pDependencyInfo) const override;
    bool PreCallValidateCmdResetEvent(VkCommandBuffer commandBuffer, VkEvent event, VkPipelineStageFlags stageMask) const override;
    bool PreCallValidateCmdResetEvent2KHR(VkCommandBuffer commandBuffer, VkEvent event,
                                          VkPipelineStageFlags2KHR stageMask) const override;
    bool PreCallValidateCmdWaitEvents(VkCommandBuffer commandBuffer, uint32_t eventCount, const VkEvent* pEvents,
                                      VkPipelineStageFlags srcStageMask, VkPipelineStageFlags dstStageMask,
                                      uint32_t memoryBarrierCount, const VkMemoryBarrier* pMemoryBarriers,
                                      uint32_t bufferMemoryBarrierCount, const VkBufferMemoryBarrier* pBufferMemoryBarriers,
                                      uint32_t imageMemoryBarrierCount,
                                      const VkImageMemoryBarrier* pImageMemoryBarriers) const override;
    bool PreCallValidateCmdWaitEvents2KHR(VkCommandBuffer commandBuffer, uint32_t eventCount, const VkEvent* pEvents,
                                          const VkDependencyInfoKHR* pDependencyInfos) const override;
    bool PreCallValidateCmdPipelineBarrier(VkCommandBuffer commandBuffer, VkPipelineStageFlags srcStageMask,
                                           VkPipelineStageFlags dstStageMask, VkDependencyFlags dependencyFlags,
                                           uint32_t memoryBarrierCount, const VkMemoryBarrier* pMemoryBarriers,
                                           uint32_t bufferMemoryBarrierCount, const VkBufferMemoryBarrier* pBufferMemoryBarriers,
                                           uint32_t imageMemoryBarrierCount,
                                           const VkImageMemoryBarrier* pImageMemoryBarriers) const override;
    bool PreCallValidateCmdPipelineBarrier2KHR(VkCommandBuffer commandBuffer,
                                               const VkDependencyInfoKHR* pDependencyInfo) const override;
    bool PreCallValidateCmdWriteTimestamp(VkCommandBuffer commandBuffer, VkPipelineStageFlagBits pipelineStage,
                                          VkQueryPool queryPool, uint32_t query) const override;
    bool PreCallValidateCmdWriteTimestamp2KHR(VkCommandBuffer commandBuffer, VkPipelineStageFlags2KHR pipelineStage,
                                              VkQueryPool queryPool, uint32_t query) const override;
    void PostCallRecordCmdBindPipeline(VkCommandBuffer commandBuffer, VkPipelineBindPoint pipelineBindPoint,
                                       VkPipeline pipeline) override;
    bool ValidateCmdBeginRenderPass(VkCommandBuffer commandBuffer, RenderPassCreateVersion rp_version,
                                    const VkRenderPassBeginInfo* pRenderPassBegin) const;

    void PreCallRecordCmdBeginRenderPass(VkCommandBuffer commandBuffer, const VkRenderPassBeginInfo* pRenderPassBegin,
                                         VkSubpassContents contents) override;
    void PreCallRecordCmdBeginRenderPass2(VkCommandBuffer commandBuffer, const VkRenderPassBeginInfo* pRenderPassBegin,
                                          const VkSubpassBeginInfo *pSubpassBeginInfo) override;
    void PreCallRecordCmdBeginRenderPass2KHR(VkCommandBuffer commandBuffer, const VkRenderPassBeginInfo* pRenderPassBegin,
                                             const VkSubpassBeginInfo *pSubpassBeginInfo) override;
    void PreCallRecordCmdEndRenderPass(VkCommandBuffer commandBuffer) override;
    void PreCallRecordCmdEndRenderPass2(VkCommandBuffer commandBuffer, const VkSubpassEndInfo *pSubpassEndInfo) override;
    void PreCallRecordCmdEndRenderPass2KHR(VkCommandBuffer commandBuffer, const VkSubpassEndInfoKHR *pSubpassEndInfo) override;
    bool PreCallValidateCmdBeginRenderPass(VkCommandBuffer commandBuffer, const VkRenderPassBeginInfo* pRenderPassBegin,
                                           VkSubpassContents contents) const override;
    bool PreCallValidateCmdBeginRenderPass2KHR(VkCommandBuffer commandBuffer, const VkRenderPassBeginInfo* pRenderPassBegin,
                                               const VkSubpassBeginInfo* pSubpassBeginInfo) const override;
    bool PreCallValidateCmdBeginRenderPass2(VkCommandBuffer commandBuffer, const VkRenderPassBeginInfo* pRenderPassBegin,
                                            const VkSubpassBeginInfo* pSubpassBeginInfo) const override;
    void ValidateBoundDescriptorSets(VkCommandBuffer commandBuffer, const char* function_name);
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
    bool ValidateIndexBufferArm(VkCommandBuffer commandBuffer, uint32_t indexCount, uint32_t instanceCount, uint32_t firstIndex,
                                int32_t vertexOffset, uint32_t firstInstance) const;
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
    bool PreCallValidateCmdDispatch(VkCommandBuffer commandBuffer, uint32_t groupCountX, uint32_t groupCountY,
                                    uint32_t groupCountZ) const override;
    bool PreCallValidateCmdEndRenderPass(VkCommandBuffer commandBuffer) const override;
    bool PreCallValidateCmdEndRenderPass2(VkCommandBuffer commandBuffer, const VkSubpassEndInfo *pSubpassEndInfo) const override;
    bool PreCallValidateCmdEndRenderPass2KHR(VkCommandBuffer commandBuffer, const VkSubpassEndInfo *pSubpassEndInfo) const override;
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
    void ValidateReturnCodes(const char* api_name, VkResult result, const std::vector<VkResult>& error_codes,
                             const std::vector<VkResult>& success_codes) const;
    bool PreCallValidateCmdResolveImage(VkCommandBuffer commandBuffer, VkImage srcImage, VkImageLayout srcImageLayout,
                                        VkImage dstImage, VkImageLayout dstImageLayout, uint32_t regionCount,
                                        const VkImageResolve* pRegions) const override;
    bool PreCallValidateCmdResolveImage2KHR(VkCommandBuffer commandBuffer,
                                            const VkResolveImageInfo2KHR* pResolveImageInfo) const override;

    using QueueCallbacks = std::vector<std::function<bool (const ValidationStateTracker*, const QUEUE_STATE*)>>;

    void QueueValidateImageView(QueueCallbacks &func, const char* function_name,
                                IMAGE_VIEW_STATE* view, IMAGE_SUBRESOURCE_USAGE_BP usage);
    void QueueValidateImage(QueueCallbacks &func, const char* function_name,
                            IMAGE_STATE_BP* state, IMAGE_SUBRESOURCE_USAGE_BP usage,
                            const VkImageSubresourceRange& subresource_range);
    void QueueValidateImage(QueueCallbacks &func, const char* function_name,
                            IMAGE_STATE_BP* state, IMAGE_SUBRESOURCE_USAGE_BP usage,
                            const VkImageSubresourceLayers& range);
    void QueueValidateImage(QueueCallbacks &func, const char* function_name,
                            IMAGE_STATE_BP* state, IMAGE_SUBRESOURCE_USAGE_BP usage,
                            uint32_t array_layer, uint32_t mip_level);
    void ValidateImageInQueue(const char *function_name,
                              IMAGE_STATE_BP* state, IMAGE_SUBRESOURCE_USAGE_BP usage,
                              uint32_t array_layer, uint32_t mip_level);
    void ValidateImageInQueueArm(const char *function_name, IMAGE_STATE* image,
                                 IMAGE_SUBRESOURCE_USAGE_BP last_usage,
                                 IMAGE_SUBRESOURCE_USAGE_BP usage,
                                 uint32_t array_layer, uint32_t mip_level);

    void PreCallRecordCmdResolveImage(VkCommandBuffer commandBuffer, VkImage srcImage, VkImageLayout srcImageLayout,
                                      VkImage dstImage, VkImageLayout dstImageLayout, uint32_t regionCount,
                                      const VkImageResolve* pRegions) override;
    void PreCallRecordCmdResolveImage2KHR(VkCommandBuffer commandBuffer,
                                          const VkResolveImageInfo2KHR *pResolveImageInfo) override;
    void PreCallRecordCmdClearColorImage(VkCommandBuffer commandBuffer, VkImage image, VkImageLayout imageLayout,
                                         const VkClearColorValue* pColor, uint32_t rangeCount,
                                         const VkImageSubresourceRange* pRanges) override;
    void PreCallRecordCmdClearDepthStencilImage(VkCommandBuffer commandBuffer, VkImage image, VkImageLayout imageLayout,
                                                const VkClearDepthStencilValue* pDepthStencil, uint32_t rangeCount,
                                                const VkImageSubresourceRange* pRanges) override;
    void PreCallRecordCmdCopyImage(VkCommandBuffer commandBuffer, VkImage srcImage, VkImageLayout srcImageLayout, VkImage dstImage,
                                   VkImageLayout dstImageLayout, uint32_t regionCount, const VkImageCopy* pRegions) override;
    void PreCallRecordCmdCopyBufferToImage(VkCommandBuffer commandBuffer, VkBuffer srcBuffer, VkImage dstImage,
                                           VkImageLayout dstImageLayout, uint32_t regionCount, const VkBufferImageCopy* pRegions) override;
    void PreCallRecordCmdCopyImageToBuffer(VkCommandBuffer commandBuffer, VkImage srcImage, VkImageLayout srcImageLayout,
                                           VkBuffer dstBuffer, uint32_t regionCount, const VkBufferImageCopy* pRegions) override;
    void PreCallRecordCmdBlitImage(VkCommandBuffer commandBuffer, VkImage srcImage, VkImageLayout srcImageLayout, VkImage dstImage,
                                   VkImageLayout dstImageLayout, uint32_t regionCount, const VkImageBlit* pRegions,
                                   VkFilter filter) override;
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

    void ManualPostCallRecordEnumeratePhysicalDevices(VkInstance instance, uint32_t* pPhysicalDeviceCount,
                                                      VkPhysicalDevice* pPhysicalDevices, VkResult result);

    void ManualPostCallRecordCreateDevice(VkPhysicalDevice gpu, const VkDeviceCreateInfo* pCreateInfo,
                                          const VkAllocationCallbacks* pAllocator, VkDevice* pDevice, VkResult result);

    void ManualPostCallRecordQueueSubmit(VkQueue queue, uint32_t submitCount, const VkSubmitInfo* pSubmits, VkFence fence,
                                         VkResult result);

    void ManualPostCallRecordEndCommandBuffer(VkCommandBuffer commandBuffer, VkResult result);

    void ManualPostCallRecordCreateSemaphore(VkDevice device, const VkSemaphoreCreateInfo* pCreateInfo,
                                                      const VkAllocationCallbacks* pAllocator, VkSemaphore* pSemaphore,
                                                      VkResult result);

    void ManualPostCallRecordCreateFence(VkDevice device, const VkFenceCreateInfo* pCreateInfo,
                                                  const VkAllocationCallbacks* pAllocator, VkFence* pFence, VkResult result);

    void ManualPostCallRecordCreateComputePipelines(VkDevice device, VkPipelineCache pipelineCache,
                                                             uint32_t createInfoCount,
                                                             const VkComputePipelineCreateInfo* pCreateInfos,
                                                             const VkAllocationCallbacks* pAllocator, VkPipeline* pPipelines,
                                                             VkResult result, void* state_data);

    void PostCallRecordCmdPipelineBarrier(VkCommandBuffer commandBuffer, VkPipelineStageFlags srcStageMask,
                                          VkPipelineStageFlags dstStageMask, VkDependencyFlags dependencyFlags,
                                          uint32_t memoryBarrierCount, const VkMemoryBarrier* pMemoryBarriers,
                                          uint32_t bufferMemoryBarrierCount, const VkBufferMemoryBarrier* pBufferMemoryBarriers,
                                          uint32_t imageMemoryBarrierCount, const VkImageMemoryBarrier* pImageMemoryBarriers) override;

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
                                          const VkClearAttachment* pClearAttachments,
                                          uint32_t rectCount, const VkClearRect *pRects) override;

    bool PreCallValidateCmdExecuteCommands(VkCommandBuffer commandBuffer, uint32_t commandBufferCount,
                                           const VkCommandBuffer* pCommandBuffers) const override;
    void PreCallRecordCmdExecuteCommands(VkCommandBuffer commandBuffer, uint32_t commandBufferCount,
                                         const VkCommandBuffer* pCommandBuffers) override;

    std::shared_ptr<SWAPCHAIN_NODE> CreateSwapchainState(const VkSwapchainCreateInfoKHR* create_info,
                                                         VkSwapchainKHR swapchain) final {
        return std::static_pointer_cast<SWAPCHAIN_NODE>(std::make_shared<SWAPCHAIN_STATE_BP>(this, create_info, swapchain));
    }

// Include code-generated functions
#include "best_practices.h"

  private:
    uint32_t instance_api_version = 0;
    uint32_t num_mem_objects = 0;

    // AMD tracked
    uint32_t num_fence_objects = 0;
    uint32_t num_semaphore_objects = 0;
    uint32_t num_barriers_objects = 0;
    uint32_t num_pso = 0;
    uint32_t num_queue_submissions = 0;
    VkPipelineCache pipeline_cache = 0;
    layer_data::unordered_set<VkPipeline> pipelines_used_in_frame;
    bool robust_buffer_access = false;

    // Check that vendor-specific checks are enabled for at least one of the vendors
    bool VendorCheckEnabled(BPVendorFlags vendors) const;

    // State for use in best practices:
    layer_data::unordered_map<VkDescriptorPool, uint32_t> descriptor_pool_freed_count = {};

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

    struct GraphicsPipelineCIs {
        layer_data::optional<safe_VkPipelineDepthStencilStateCreateInfo> depthStencilStateCI;
        layer_data::optional<safe_VkPipelineColorBlendStateCreateInfo> colorBlendStateCI;
        std::vector<AttachmentInfo> accessFramebufferAttachments;
    };

    // used to track CreateInfos for graphics pipelines
    layer_data::unordered_map<VkPipeline, GraphicsPipelineCIs> graphicsPipelineCIs = {};

    void RecordCmdDrawTypeArm(RenderPassState& render_pass_state, uint32_t draw_count, const char* caller);

    // Backing data for BP-specific state data
    layer_data::unordered_map<VkPhysicalDevice, PHYSICAL_DEVICE_STATE_BP> phys_device_bp_state_map;
    // Physical device state for this instance
    PHYSICAL_DEVICE_STATE_BP* instance_device_bp_state = nullptr;

    // Get BestPractices-specific state for the given physical devices
    PHYSICAL_DEVICE_STATE_BP* GetPhysicalDeviceStateBP(const VkPhysicalDevice& phys_device);
    const PHYSICAL_DEVICE_STATE_BP* GetPhysicalDeviceStateBP(const VkPhysicalDevice& phys_device) const;

    // Get BestPractices-specific for the current instance
    PHYSICAL_DEVICE_STATE_BP* GetPhysicalDeviceStateBP();
    const PHYSICAL_DEVICE_STATE_BP* GetPhysicalDeviceStateBP() const;

    IMAGE_STATE_BP* GetImageUsageState(VkImage image);
    void ReleaseImageUsageState(VkImage image);
    std::unordered_map<VkImage, IMAGE_STATE_BP> imageUsageMap;

    void AddDeferredQueueOperations(CMD_BUFFER_STATE_BP* cb);

    void RecordAttachmentClearAttachments(CMD_BUFFER_STATE_BP* cmd_state, RenderPassState& state, uint32_t fb_attachment,
                                          uint32_t color_attachment, VkImageAspectFlags aspects, uint32_t rectCount,
                                          const VkClearRect* pRects);
    void RecordAttachmentAccess(RenderPassState& state, uint32_t attachment, VkImageAspectFlags aspects);
    bool ClearAttachmentsIsFullClear(const CMD_BUFFER_STATE_BP* cmd, uint32_t rectCount, const VkClearRect* pRects) const;
    bool ValidateClearAttachment(VkCommandBuffer commandBuffer, const CMD_BUFFER_STATE_BP* cmd, uint32_t fb_attachment,
                                 uint32_t color_attachment, VkImageAspectFlags aspects, bool secondary) const;

    bool ValidateCmdEndRenderPass(VkCommandBuffer commandBuffer) const;
    void RecordCmdBeginRenderPass(VkCommandBuffer commandBuffer, const VkRenderPassBeginInfo *pRenderPassBegin);

    const CMD_BUFFER_STATE_BP* GetCBState(const VkCommandBuffer cb) const {
        return static_cast<const CMD_BUFFER_STATE_BP*>(Get<CMD_BUFFER_STATE>(cb));
    }
    CMD_BUFFER_STATE_BP* GetCBState(const VkCommandBuffer cb) {
        return static_cast<CMD_BUFFER_STATE_BP*>(Get<CMD_BUFFER_STATE>(cb));
    }
    std::shared_ptr<CMD_BUFFER_STATE> CreateCmdBufferState(VkCommandBuffer cb, const VkCommandBufferAllocateInfo* create_info,
                                                           std::shared_ptr<COMMAND_POOL_STATE>& pool) final;
};

