#pragma once

#include "../best_practices_validation.h"

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

// How many small indexed drawcalls in a command buffer before a warning is thrown
static const uint32_t kMaxSmallIndexedDrawcalls = 10;

// How many indices make a small indexed drawcall
static const int kSmallIndexedDrawcallIndices = 10;

// Maximum sample count for full throughput on Mali GPUs
static const VkSampleCountFlagBits kMaxEfficientSamplesArm = VK_SAMPLE_COUNT_4_BIT;

class MyExampleBestPractices : public BestPracticeBase {
  public:
    MyExampleBestPractices(BestPracticesTracker& tracker) : BestPracticeBase(tracker) {}

    static const std::string ID;

    virtual const std::string id() const override;

  private:
    using StateTracker = ValidationStateTracker;
    std::string GetAPIVersionName(uint32_t version) const;
    uint32_t instance_api_version = 0;
    uint32_t num_mem_objects = 0;

    // Check that vendor-specific checks are enabled for at least one of the vendors
    bool VendorCheckEnabled(BPVendorFlags vendors) const;

    bool ValidateCmdDrawType(VkCommandBuffer cmd_buffer, const char* caller) const;

    bool ValidateDeprecatedExtensions(const char* api_name, const char* extension_name, uint32_t version, const char* vuid) const;
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
    bool PreCallValidateCreateSwapchainKHR(VkDevice device, const VkSwapchainCreateInfoKHR* pCreateInfo,
                                           const VkAllocationCallbacks* pAllocator, VkSwapchainKHR* pSwapchain) const override;
    bool PreCallValidateCreateSharedSwapchainsKHR(VkDevice device, uint32_t swapchainCount,
                                                  const VkSwapchainCreateInfoKHR* pCreateInfos,
                                                  const VkAllocationCallbacks* pAllocator, VkSwapchainKHR* pSwapchains) const override;
    bool PreCallValidateCreateRenderPass(VkDevice device, const VkRenderPassCreateInfo* pCreateInfo,
                                         const VkAllocationCallbacks* pAllocator, VkRenderPass* pRenderPass) const override;
    bool ValidateAttachments(const VkRenderPassCreateInfo2* rpci, uint32_t attachmentCount, const VkImageView* image_views) const;
    bool PreCallValidateCreateFramebuffer(VkDevice device, const VkFramebufferCreateInfo* pCreateInfo,
                                          const VkAllocationCallbacks* pAllocator, VkFramebuffer* pFramebuffer) const override;
    bool PreCallValidateAllocateDescriptorSets(VkDevice device, const VkDescriptorSetAllocateInfo* pAllocateInfo,
                                               VkDescriptorSet* pDescriptorSets, void* ads_state_data) const override;
    void ManualPostCallRecordAllocateDescriptorSets(VkDevice device, const VkDescriptorSetAllocateInfo* pAllocateInfo,
                                                    VkDescriptorSet* pDescriptorSets, VkResult result, void* ads_state) const override;
    void PostCallRecordFreeDescriptorSets(VkDevice device, VkDescriptorPool descriptorPool, uint32_t descriptorSetCount,
                                          const VkDescriptorSet* pDescriptorSets, VkResult result) override;
    bool PreCallValidateAllocateMemory(VkDevice device, const VkMemoryAllocateInfo* pAllocateInfo,
                                       const VkAllocationCallbacks* pAllocator, VkDeviceMemory* pMemory) const override;
    void ManualPostCallRecordAllocateMemory(VkDevice device, const VkMemoryAllocateInfo* pAllocateInfo,
                                            const VkAllocationCallbacks* pAllocator, VkDeviceMemory* pMemory, VkResult result) const override;
    void PreCallRecordFreeMemory(VkDevice device, VkDeviceMemory memory, const VkAllocationCallbacks* pAllocator) override;
    bool ValidateBindBufferMemory(VkBuffer buffer, VkDeviceMemory memory, const char* api_name) const;
    bool PreCallValidateBindBufferMemory(VkDevice device, VkBuffer buffer, VkDeviceMemory memory, VkDeviceSize memoryOffset) const override;
    bool PreCallValidateBindBufferMemory2(VkDevice device, uint32_t bindInfoCount, const VkBindBufferMemoryInfo* pBindInfos) const override;
    bool PreCallValidateBindBufferMemory2KHR(VkDevice device, uint32_t bindInfoCount,
                                             const VkBindBufferMemoryInfo* pBindInfos) const override;
    bool ValidateBindImageMemory(VkImage image, VkDeviceMemory memory, const char* api_name) const;
    bool PreCallValidateBindImageMemory(VkDevice device, VkImage image, VkDeviceMemory memory, VkDeviceSize memoryOffset) const override;
    bool PreCallValidateBindImageMemory2(VkDevice device, uint32_t bindInfoCount, const VkBindImageMemoryInfo* pBindInfos) const override;
    bool PreCallValidateBindImageMemory2KHR(VkDevice device, uint32_t bindInfoCount, const VkBindImageMemoryInfo* pBindInfos) const override;
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

    bool CheckPipelineStageFlags(std::string api_name, const VkPipelineStageFlags flags) const;
    bool PreCallValidateQueueSubmit(VkQueue queue, uint32_t submitCount, const VkSubmitInfo* pSubmits, VkFence fence) const override;
    bool PreCallValidateBeginCommandBuffer(VkCommandBuffer commandBuffer, const VkCommandBufferBeginInfo* pBeginInfo) const override;
    bool PreCallValidateCmdSetEvent(VkCommandBuffer commandBuffer, VkEvent event, VkPipelineStageFlags stageMask) const override;
    bool PreCallValidateCmdResetEvent(VkCommandBuffer commandBuffer, VkEvent event, VkPipelineStageFlags stageMask) const override;
    bool PreCallValidateCmdWaitEvents(VkCommandBuffer commandBuffer, uint32_t eventCount, const VkEvent* pEvents,
                                      VkPipelineStageFlags srcStageMask, VkPipelineStageFlags dstStageMask,
                                      uint32_t memoryBarrierCount, const VkMemoryBarrier* pMemoryBarriers,
                                      uint32_t bufferMemoryBarrierCount, const VkBufferMemoryBarrier* pBufferMemoryBarriers,
                                      uint32_t imageMemoryBarrierCount, const VkImageMemoryBarrier* pImageMemoryBarriers) const override;
    bool PreCallValidateCmdPipelineBarrier(VkCommandBuffer commandBuffer, VkPipelineStageFlags srcStageMask,
                                           VkPipelineStageFlags dstStageMask, VkDependencyFlags dependencyFlags,
                                           uint32_t memoryBarrierCount, const VkMemoryBarrier* pMemoryBarriers,
                                           uint32_t bufferMemoryBarrierCount, const VkBufferMemoryBarrier* pBufferMemoryBarriers,
                                           uint32_t imageMemoryBarrierCount,
                                           const VkImageMemoryBarrier* pImageMemoryBarriers) const override;
    bool PreCallValidateCmdWriteTimestamp(VkCommandBuffer commandBuffer, VkPipelineStageFlagBits pipelineStage,
                                          VkQueryPool queryPool, uint32_t query) const override;
    bool ValidateCmdBeginRenderPass(VkCommandBuffer commandBuffer, RenderPassCreateVersion rp_version,
                                    const VkRenderPassBeginInfo* pRenderPassBegin) const;
    bool PreCallValidateCmdBeginRenderPass(VkCommandBuffer commandBuffer, const VkRenderPassBeginInfo* pRenderPassBegin,
                                           VkSubpassContents contents) const override;
    bool PreCallValidateCmdBeginRenderPass2KHR(VkCommandBuffer commandBuffer, const VkRenderPassBeginInfo* pRenderPassBegin,
                                               const VkSubpassBeginInfoKHR* pSubpassBeginInfo) const override;
    bool PreCallValidateCmdBeginRenderPass2(VkCommandBuffer commandBuffer, const VkRenderPassBeginInfo* pRenderPassBegin,
                                            const VkSubpassBeginInfoKHR* pSubpassBeginInfo) const override;
    bool PreCallValidateCmdDraw(VkCommandBuffer commandBuffer, uint32_t vertexCount, uint32_t instanceCount, uint32_t firstVertex,
                                uint32_t firstInstance) const override;
    bool PreCallValidateCmdDrawIndexed(VkCommandBuffer commandBuffer, uint32_t indexCount, uint32_t instanceCount,
                                       uint32_t firstIndex, int32_t vertexOffset, uint32_t firstInstance) const override;
    bool ValidateIndexBufferArm(VkCommandBuffer commandBuffer, uint32_t indexCount, uint32_t instanceCount, uint32_t firstIndex,
                                int32_t vertexOffset, uint32_t firstInstance) const;
    void PreCallRecordCmdDrawIndexed(VkCommandBuffer commandBuffer, uint32_t indexCount, uint32_t instanceCount,
                                     uint32_t firstIndex, int32_t vertexOffset, uint32_t firstInstance) override;
    bool PreCallValidateCmdDrawIndirect(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset, uint32_t drawCount,
                                        uint32_t stride) const override;
    bool PreCallValidateCmdDrawIndexedIndirect(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                               uint32_t drawCount, uint32_t stride) const override;
    bool PreCallValidateCmdDispatch(VkCommandBuffer commandBuffer, uint32_t groupCountX, uint32_t groupCountY,
                                    uint32_t groupCountZ) const override;
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
                                                              uint32_t requested_queue_family_property_count, bool qfp_null,
                                                              const char* caller_name) const;
    bool PreCallValidateBindAccelerationStructureMemoryNV(VkDevice device, uint32_t bindInfoCount,
                                                          const VkBindAccelerationStructureMemoryInfoNV* pBindInfos) const override;
    bool PreCallValidateQueueBindSparse(VkQueue queue, uint32_t bindInfoCount, const VkBindSparseInfo* pBindInfo,
                                        VkFence fence) const override;
    void ManualPostCallRecordQueueBindSparse(VkQueue queue, uint32_t bindInfoCount, const VkBindSparseInfo* pBindInfo,
                                             VkFence fence, VkResult result) const override;
    bool PreCallValidateCmdClearAttachments(VkCommandBuffer commandBuffer, uint32_t attachmentCount,
                                            const VkClearAttachment* pAttachments, uint32_t rectCount,
                                            const VkClearRect* pRects) const override;
    void ValidateReturnCodes(const char* api_name, VkResult result, const std::vector<VkResult>& error_codes,
                             const std::vector<VkResult>& success_codes) const;
    bool PreCallValidateCmdResolveImage(VkCommandBuffer commandBuffer, VkImage srcImage, VkImageLayout srcImageLayout,
                                        VkImage dstImage, VkImageLayout dstImageLayout, uint32_t regionCount,
                                        const VkImageResolve* pRegions) const override;
    bool PreCallValidateCreateSampler(VkDevice device, const VkSamplerCreateInfo* pCreateInfo,
                                      const VkAllocationCallbacks* pAllocator, VkSampler* pSampler) const override;
    void ManualPostCallRecordQueuePresentKHR(VkQueue queue, const VkPresentInfoKHR* pPresentInfo, VkResult result) const override;

// Include code-generated functions
//#include "best_practices.h"

    // State for use in best practices:
    std::unordered_map<VkDescriptorPool, uint32_t> descriptor_pool_freed_count = {};

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
};
