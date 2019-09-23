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

#include "state_tracker.h"
#include "gpu_validation.h"
#include "shader_validation.h"

struct GpuValidationState;

class CoreChecks : public ValidationStateTracker {
  public:
    using StateTracker = ValidationStateTracker;
    std::unordered_set<uint64_t> ahb_ext_formats_set;
    GlobalQFOTransferBarrierMap<VkImageMemoryBarrier> qfo_release_image_barrier_map;
    GlobalQFOTransferBarrierMap<VkBufferMemoryBarrier> qfo_release_buffer_barrier_map;
    unordered_map<VkImage, std::vector<ImageSubresourcePair>> imageSubresourceMap;
    using ImageSubresPairLayoutMap = std::unordered_map<ImageSubresourcePair, IMAGE_LAYOUT_STATE>;
    ImageSubresPairLayoutMap imageLayoutMap;

    std::unique_ptr<GpuValidationState> gpu_validation_state;

    bool VerifyQueueStateToSeq(const QUEUE_STATE* initial_queue, uint64_t initial_seq) const;
    bool ValidateSetMemBinding(VkDeviceMemory mem, const VulkanTypedHandle& typed_handle, const char* apiName) const;
    bool ValidateDeviceQueueFamily(uint32_t queue_family, const char* cmd_name, const char* parameter_name, const char* error_code,
                                   bool optional) const;
    bool ValidateBindBufferMemory(VkBuffer buffer, VkDeviceMemory mem, VkDeviceSize memoryOffset, const char* api_name) const;
    bool ValidateGetImageMemoryRequirements2(const VkImageMemoryRequirementsInfo2* pInfo) const;
    bool CheckCommandBuffersInFlight(const COMMAND_POOL_STATE* pPool, const char* action, const char* error_code) const;
    bool CheckCommandBufferInFlight(const CMD_BUFFER_STATE* cb_node, const char* action, const char* error_code) const;
    bool VerifyQueueStateToFence(VkFence fence) const;
    void StoreMemRanges(VkDeviceMemory mem, VkDeviceSize offset, VkDeviceSize size);
    bool ValidateIdleDescriptorSet(VkDescriptorSet set, const char* func_str);
    void InitializeShadowMemory(VkDeviceMemory mem, VkDeviceSize offset, VkDeviceSize size, void** ppData);
    bool ValidatePipelineLocked(std::vector<std::unique_ptr<PIPELINE_STATE>> const& pPipelines, int pipelineIndex) const;
    bool ValidatePipelineUnlocked(const PIPELINE_STATE* pPipeline, uint32_t pipelineIndex) const;
    bool ValidImageBufferQueue(const CMD_BUFFER_STATE* cb_node, const VulkanTypedHandle& object, VkQueue queue, uint32_t count,
                               const uint32_t* indices) const;
    bool ValidateFenceForSubmit(const FENCE_STATE* pFence) const;
    bool ValidateSemaphoresForSubmit(VkQueue queue, const VkSubmitInfo* submit,
                                     std::unordered_set<VkSemaphore>* unsignaled_sema_arg,
                                     std::unordered_set<VkSemaphore>* signaled_sema_arg,
                                     std::unordered_set<VkSemaphore>* internal_sema_arg) const;
    bool ValidateCommandBuffersForSubmit(VkQueue queue, const VkSubmitInfo* submit,
                                         ImageSubresPairLayoutMap* localImageLayoutMap_arg,
                                         std::vector<VkCommandBuffer>* current_cmds_arg) const;
    bool ValidateStatus(const CMD_BUFFER_STATE* pNode, CBStatusFlags status_mask, VkFlags msg_flags, const char* fail_msg,
                        const char* msg_code) const;
    bool ValidateDrawStateFlags(const CMD_BUFFER_STATE* pCB, const PIPELINE_STATE* pPipe, bool indexed, const char* msg_code) const;
    bool LogInvalidAttachmentMessage(const char* type1_string, const RENDER_PASS_STATE* rp1_state, const char* type2_string,
                                     const RENDER_PASS_STATE* rp2_state, uint32_t primary_attach, uint32_t secondary_attach,
                                     const char* msg, const char* caller, const char* error_code) const;
    bool ValidateStageMaskGsTsEnables(VkPipelineStageFlags stageMask, const char* caller, const char* geo_error_id,
                                      const char* tess_error_id, const char* mesh_error_id, const char* task_error_id) const;
    bool ValidateMapMemRange(const DEVICE_MEMORY_STATE* mem_info, VkDeviceSize offset, VkDeviceSize size) const;
    bool ValidatePushConstantRange(const uint32_t offset, const uint32_t size, const char* caller_name, uint32_t index) const;
    bool ValidateRenderPassDAG(RenderPassCreateVersion rp_version, const VkRenderPassCreateInfo2KHR* pCreateInfo) const;
    bool ValidateAttachmentCompatibility(const char* type1_string, const RENDER_PASS_STATE* rp1_state, const char* type2_string,
                                         const RENDER_PASS_STATE* rp2_state, uint32_t primary_attach, uint32_t secondary_attach,
                                         const char* caller, const char* error_code) const;
    bool ValidateSubpassCompatibility(const char* type1_string, const RENDER_PASS_STATE* rp1_state, const char* type2_string,
                                      const RENDER_PASS_STATE* rp2_state, const int subpass, const char* caller,
                                      const char* error_code) const;
    bool ValidateRenderPassCompatibility(const char* type1_string, const RENDER_PASS_STATE* rp1_state, const char* type2_string,
                                         const RENDER_PASS_STATE* rp2_state, const char* caller, const char* error_code) const;
    bool ReportInvalidCommandBuffer(const CMD_BUFFER_STATE* cb_state, const char* call_source) const;
    void InitGpuValidation();
    bool ValidateQueueFamilyIndex(const PHYSICAL_DEVICE_STATE* pd_state, uint32_t requested_queue_family, const char* err_code,
                                  const char* cmd_name, const char* queue_family_var_name);
    bool ValidateDeviceQueueCreateInfos(const PHYSICAL_DEVICE_STATE* pd_state, uint32_t info_count,
                                        const VkDeviceQueueCreateInfo* infos);

    bool ValidatePipelineVertexDivisors(std::vector<std::unique_ptr<PIPELINE_STATE>> const& pipe_state_vec, const uint32_t count,
                                        const VkGraphicsPipelineCreateInfo* pipe_cis) const;
    void EnqueueSubmitTimeValidateImageBarrierAttachment(const char* func_name, CMD_BUFFER_STATE* cb_state,
                                                         uint32_t imageMemBarrierCount,
                                                         const VkImageMemoryBarrier* pImageMemBarriers);
    bool ValidateImageBarrierAttachment(const char* funcName, CMD_BUFFER_STATE const* cb_state, VkFramebuffer framebuffer,
                                        uint32_t active_subpass, const safe_VkSubpassDescription2KHR& sub_desc,
                                        const VulkanTypedHandle& rp_handle, uint32_t img_index,
                                        const VkImageMemoryBarrier& img_barrier) const;
    bool ValidateConcurrentBarrierAtSubmit(VkQueue queue, const char* func_name, const CMD_BUFFER_STATE* cb_state,
                                           const VulkanTypedHandle& typed_handle, uint32_t src_queue_family,
                                           uint32_t dst_queue_family) const;
    bool ValidateCmdBeginRenderPass(VkCommandBuffer commandBuffer, RenderPassCreateVersion rp_version,
                                    const VkRenderPassBeginInfo* pRenderPassBegin) const;
    bool ValidateDependencies(FRAMEBUFFER_STATE const* framebuffer, RENDER_PASS_STATE const* renderPass) const;
    bool ValidateBarriers(const char* funcName, const CMD_BUFFER_STATE* cb_state, VkPipelineStageFlags src_stage_mask,
                          VkPipelineStageFlags dst_stage_mask, uint32_t memBarrierCount, const VkMemoryBarrier* pMemBarriers,
                          uint32_t bufferBarrierCount, const VkBufferMemoryBarrier* pBufferMemBarriers,
                          uint32_t imageMemBarrierCount, const VkImageMemoryBarrier* pImageMemBarriers) const;
    bool ValidateBarrierQueueFamilies(const char* func_name, const CMD_BUFFER_STATE* cb_state, const VkImageMemoryBarrier& barrier,
                                      const IMAGE_STATE* state_data) const;
    bool ValidateBarrierQueueFamilies(const char* func_name, const CMD_BUFFER_STATE* cb_state, const VkBufferMemoryBarrier& barrier,
                                      const BUFFER_STATE* state_data) const;
    bool ValidateCreateSwapchain(const char* func_name, VkSwapchainCreateInfoKHR const* pCreateInfo,
                                 const SURFACE_STATE* surface_state, const SWAPCHAIN_NODE* old_swapchain_state) const;
    bool ValidatePipelineBindPoint(const CMD_BUFFER_STATE* cb_state, VkPipelineBindPoint bind_point, const char* func_name,
                                   const std::map<VkPipelineBindPoint, std::string>& bind_errors) const;
    bool ValidateMemoryIsMapped(const char* funcName, uint32_t memRangeCount, const VkMappedMemoryRange* pMemRanges) const;
    bool ValidateAndCopyNoncoherentMemoryToDriver(uint32_t mem_range_count, const VkMappedMemoryRange* mem_ranges) const;
    void CopyNoncoherentMemoryFromDriver(uint32_t mem_range_count, const VkMappedMemoryRange* mem_ranges);
    bool ValidateMappedMemoryRangeDeviceLimits(const char* func_name, uint32_t mem_range_count,
                                               const VkMappedMemoryRange* mem_ranges) const;
    BarrierOperationsType ComputeBarrierOperationsType(const CMD_BUFFER_STATE* cb_state, uint32_t buffer_barrier_count,
                                                       const VkBufferMemoryBarrier* buffer_barriers, uint32_t image_barrier_count,
                                                       const VkImageMemoryBarrier* image_barriers) const;
    bool ValidateStageMasksAgainstQueueCapabilities(const CMD_BUFFER_STATE* cb_state, VkPipelineStageFlags source_stage_mask,
                                                    VkPipelineStageFlags dest_stage_mask, BarrierOperationsType barrier_op_type,
                                                    const char* function, const char* error_code) const;
    bool ValidateRenderPassImageBarriers(const char* funcName, const CMD_BUFFER_STATE* cb_state, uint32_t active_subpass,
                                         const safe_VkSubpassDescription2KHR& sub_desc, const VulkanTypedHandle& rp_handle,
                                         const safe_VkSubpassDependency2KHR* dependencies,
                                         const std::vector<uint32_t>& self_dependencies, uint32_t image_mem_barrier_count,
                                         const VkImageMemoryBarrier* image_barriers) const;
    bool ValidateSecondaryCommandBufferState(const CMD_BUFFER_STATE* pCB, const CMD_BUFFER_STATE* pSubCB);
    bool ValidateFramebuffer(VkCommandBuffer primaryBuffer, const CMD_BUFFER_STATE* pCB, VkCommandBuffer secondaryBuffer,
                             const CMD_BUFFER_STATE* pSubCB, const char* caller);
    bool ValidateDescriptorUpdateTemplate(const char* func_name, const VkDescriptorUpdateTemplateCreateInfoKHR* pCreateInfo);
    bool ValidateCreateSamplerYcbcrConversion(const char* func_name, const VkSamplerYcbcrConversionCreateInfo* create_info) const;
    bool ValidateImportFence(VkFence fence, const char* caller_name) const;
    bool ValidateAcquireNextImage(VkDevice device, VkSwapchainKHR swapchain, uint64_t timeout, VkSemaphore semaphore, VkFence fence,
                                  uint32_t* pImageIndex, const char* func_name) const;
    bool VerifyRenderAreaBounds(const VkRenderPassBeginInfo* pRenderPassBegin) const;
    bool VerifyFramebufferAndRenderPassImageViews(const VkRenderPassBeginInfo* pRenderPassBeginInfo) const;
    bool ValidatePrimaryCommandBuffer(const CMD_BUFFER_STATE* pCB, char const* cmd_name, const char* error_code) const;
    void RecordCmdNextSubpassLayouts(VkCommandBuffer commandBuffer, VkSubpassContents contents);
    bool ValidateCmdEndRenderPass(RenderPassCreateVersion rp_version, VkCommandBuffer commandBuffer) const;
    void RecordCmdEndRenderPassLayouts(VkCommandBuffer commandBuffer);
    bool ValidateFramebufferCreateInfo(const VkFramebufferCreateInfo* pCreateInfo) const;
    bool MatchUsage(uint32_t count, const VkAttachmentReference2KHR* attachments, const VkFramebufferCreateInfo* fbci,
                    VkImageUsageFlagBits usage_flag, const char* error_code) const;
    bool IsImageLayoutReadOnly(VkImageLayout layout) const;
    bool CheckDependencyExists(const uint32_t subpass, const VkImageLayout layout,
                               const std::vector<SubpassLayout>& dependent_subpasses, const std::vector<DAGNode>& subpass_to_node,
                               bool& skip) const;
    bool CheckPreserved(const VkRenderPassCreateInfo2KHR* pCreateInfo, const int index, const uint32_t attachment,
                        const std::vector<DAGNode>& subpass_to_node, int depth, bool& skip) const;
    bool ValidateBindImageMemory(const VkBindImageMemoryInfo& bindInfo, const char* api_name) const;
    bool ValidateGetPhysicalDeviceDisplayPlanePropertiesKHRQuery(VkPhysicalDevice physicalDevice, uint32_t planeIndex,
                                                                 const char* api_name) const;
    bool ValidateQuery(VkQueue queue, CMD_BUFFER_STATE* pCB, VkQueryPool queryPool, uint32_t firstQuery, uint32_t queryCount,
                       VkQueryResultFlags flags) const;
    QueryState GetQueryState(const QUEUE_STATE* queue_data, VkQueryPool queryPool, uint32_t queryIndex) const;
    bool VerifyQueryIsReset(VkQueue queue, VkCommandBuffer commandBuffer, QueryObject query_obj) const;
    bool ValidateImportSemaphore(VkSemaphore semaphore, const char* caller_name) const;
    bool ValidateBeginQuery(const CMD_BUFFER_STATE* cb_state, const QueryObject& query_obj, VkFlags flags, CMD_TYPE cmd,
                            const char* cmd_name, const char* vuid_queue_flags, const char* vuid_queue_feedback,
                            const char* vuid_queue_occlusion, const char* vuid_precise, const char* vuid_query_count) const;
    bool ValidateCmdEndQuery(const CMD_BUFFER_STATE* cb_state, const QueryObject& query_obj, CMD_TYPE cmd, const char* cmd_name,
                             const char* vuid_queue_flags, const char* vuid_active_queries) const;
    bool ValidateCmdDrawType(VkCommandBuffer cmd_buffer, bool indexed, VkPipelineBindPoint bind_point, CMD_TYPE cmd_type,
                             const char* caller, VkQueueFlags queue_flags, const char* queue_flag_code,
                             const char* renderpass_msg_code, const char* pipebound_msg_code,
                             const char* dynamic_state_msg_code) const;
    bool ValidateCmdNextSubpass(RenderPassCreateVersion rp_version, VkCommandBuffer commandBuffer) const;
    bool ValidateInsertMemoryRange(const VulkanTypedHandle& typed_handle, const DEVICE_MEMORY_STATE* mem_info,
                                   VkDeviceSize memoryOffset, const VkMemoryRequirements& memRequirements, bool is_linear,
                                   const char* api_name) const;
    bool ValidateInsertImageMemoryRange(VkImage image, const DEVICE_MEMORY_STATE* mem_info, VkDeviceSize mem_offset,
                                        const VkMemoryRequirements& mem_reqs, bool is_linear, const char* api_name) const;
    bool ValidateInsertBufferMemoryRange(VkBuffer buffer, const DEVICE_MEMORY_STATE* mem_info, VkDeviceSize mem_offset,
                                         const VkMemoryRequirements& mem_reqs, const char* api_name) const;
    bool ValidateInsertAccelerationStructureMemoryRange(VkAccelerationStructureNV as, const DEVICE_MEMORY_STATE* mem_info,
                                                        VkDeviceSize mem_offset, const VkMemoryRequirements& mem_reqs,
                                                        const char* api_name) const;

    bool ValidateMemoryTypes(const DEVICE_MEMORY_STATE* mem_info, const uint32_t memory_type_bits, const char* funcName,
                             const char* msgCode) const;
    bool ValidateCommandBufferState(const CMD_BUFFER_STATE* cb_state, const char* call_source, int current_submit_count,
                                    const char* vu_id) const;
    bool ValidateCommandBufferSimultaneousUse(const CMD_BUFFER_STATE* pCB, int current_submit_count) const;
    bool ValidateGetDeviceQueue(uint32_t queueFamilyIndex, uint32_t queueIndex, VkQueue* pQueue, const char* valid_qfi_vuid,
                                const char* qfi_in_range_vuid) const;
    bool ValidateRenderpassAttachmentUsage(RenderPassCreateVersion rp_version, const VkRenderPassCreateInfo2KHR* pCreateInfo) const;
    bool AddAttachmentUse(RenderPassCreateVersion rp_version, uint32_t subpass, std::vector<uint8_t>& attachment_uses,
                          std::vector<VkImageLayout>& attachment_layouts, uint32_t attachment, uint8_t new_use,
                          VkImageLayout new_layout) const;
    bool ValidateAttachmentIndex(RenderPassCreateVersion rp_version, uint32_t attachment, uint32_t attachment_count,
                                 const char* type) const;
    bool ValidateCreateRenderPass(VkDevice device, RenderPassCreateVersion rp_version,
                                  const VkRenderPassCreateInfo2KHR* pCreateInfo) const;
    bool ValidateRenderPassPipelineBarriers(const char* funcName, const CMD_BUFFER_STATE* cb_state,
                                            VkPipelineStageFlags src_stage_mask, VkPipelineStageFlags dst_stage_mask,
                                            VkDependencyFlags dependency_flags, uint32_t mem_barrier_count,
                                            const VkMemoryBarrier* mem_barriers, uint32_t buffer_mem_barrier_count,
                                            const VkBufferMemoryBarrier* buffer_mem_barriers, uint32_t image_mem_barrier_count,
                                            const VkImageMemoryBarrier* image_barriers) const;
    bool CheckStageMaskQueueCompatibility(VkCommandBuffer command_buffer, VkPipelineStageFlags stage_mask, VkQueueFlags queue_flags,
                                          const char* function, const char* src_or_dest, const char* error_code) const;
    bool ValidateUpdateDescriptorSetWithTemplate(VkDescriptorSet descriptorSet,
                                                 VkDescriptorUpdateTemplateKHR descriptorUpdateTemplate, const void* pData);
    bool ValidateMemoryIsBoundToBuffer(const BUFFER_STATE*, const char*, const char*) const;
    bool ValidateMemoryIsBoundToImage(const IMAGE_STATE*, const char*, const char*) const;
    bool ValidateMemoryIsBoundToAccelerationStructure(const ACCELERATION_STRUCTURE_STATE*, const char*, const char*) const;
    bool ValidateObjectNotInUse(const BASE_NODE* obj_node, const VulkanTypedHandle& obj_struct, const char* caller_name,
                                const char* error_code) const;
    bool ValidateCmdQueueFlags(const CMD_BUFFER_STATE* cb_node, const char* caller_name, VkQueueFlags flags,
                               const char* error_code) const;
    bool InsideRenderPass(const CMD_BUFFER_STATE* pCB, const char* apiName, const char* msgCode) const;
    bool OutsideRenderPass(const CMD_BUFFER_STATE* pCB, const char* apiName, const char* msgCode) const;

    static void SetLayout(ImageSubresPairLayoutMap& imageLayoutMap, ImageSubresourcePair imgpair, VkImageLayout layout);

    bool ValidateImageSampleCount(const IMAGE_STATE* image_state, VkSampleCountFlagBits sample_count, const char* location,
                                  const std::string& msgCode) const;
    bool ValidateCmdSubpassState(const CMD_BUFFER_STATE* pCB, const CMD_TYPE cmd_type) const;
    bool ValidateCmd(const CMD_BUFFER_STATE* cb_state, const CMD_TYPE cmd, const char* caller_name) const;

    bool ValidateDeviceMaskToPhysicalDeviceCount(uint32_t deviceMask, VkDebugReportObjectTypeEXT VUID_handle_type,
                                                 uint64_t VUID_handle, const char* VUID) const;
    bool ValidateDeviceMaskToZero(uint32_t deviceMask, VkDebugReportObjectTypeEXT VUID_handle_type, uint64_t VUID_handle,
                                  const char* VUID) const;
    bool ValidateDeviceMaskToCommandBuffer(const CMD_BUFFER_STATE* pCB, uint32_t deviceMask,
                                           VkDebugReportObjectTypeEXT VUID_handle_type, uint64_t VUID_handle,
                                           const char* VUID) const;
    bool ValidateDeviceMaskToRenderPass(const CMD_BUFFER_STATE* pCB, uint32_t deviceMask,
                                        VkDebugReportObjectTypeEXT VUID_handle_type, uint64_t VUID_handle, const char* VUID) const;

    bool ValidateBindAccelerationStructureMemoryNV(VkDevice device, const VkBindAccelerationStructureMemoryInfoNV& info) const;
    // Prototypes for CoreChecks accessor functions
    VkFormatProperties GetPDFormatProperties(const VkFormat format) const;
    VkResult GetPDImageFormatProperties(const VkImageCreateInfo*, VkImageFormatProperties*);
    VkResult GetPDImageFormatProperties2(const VkPhysicalDeviceImageFormatInfo2*, VkImageFormatProperties2*) const;
    const VkPhysicalDeviceMemoryProperties* GetPhysicalDeviceMemoryProperties();

    const GlobalQFOTransferBarrierMap<VkImageMemoryBarrier>& GetGlobalQFOReleaseBarrierMap(
        const QFOTransferBarrier<VkImageMemoryBarrier>::Tag& type_tag) const;
    const GlobalQFOTransferBarrierMap<VkBufferMemoryBarrier>& GetGlobalQFOReleaseBarrierMap(
        const QFOTransferBarrier<VkBufferMemoryBarrier>::Tag& type_tag) const;
    GlobalQFOTransferBarrierMap<VkImageMemoryBarrier>& GetGlobalQFOReleaseBarrierMap(
        const QFOTransferBarrier<VkImageMemoryBarrier>::Tag& type_tag);
    GlobalQFOTransferBarrierMap<VkBufferMemoryBarrier>& GetGlobalQFOReleaseBarrierMap(
        const QFOTransferBarrier<VkBufferMemoryBarrier>::Tag& type_tag);
    template <typename Barrier>
    void RecordQueuedQFOTransferBarriers(CMD_BUFFER_STATE* cb_state);
    template <typename Barrier>
    bool ValidateQueuedQFOTransferBarriers(const CMD_BUFFER_STATE* cb_state, QFOTransferCBScoreboards<Barrier>* scoreboards) const;
    bool ValidateQueuedQFOTransfers(const CMD_BUFFER_STATE* cb_state,
                                    QFOTransferCBScoreboards<VkImageMemoryBarrier>* qfo_image_scoreboards,
                                    QFOTransferCBScoreboards<VkBufferMemoryBarrier>* qfo_buffer_scoreboards) const;
    template <typename BarrierRecord, typename Scoreboard>
    bool ValidateAndUpdateQFOScoreboard(const debug_report_data* report_data, const CMD_BUFFER_STATE* cb_state,
                                        const char* operation, const BarrierRecord& barrier, Scoreboard* scoreboard) const;
    template <typename Barrier>
    void RecordBarrierArrayValidationInfo(const char* func_name, CMD_BUFFER_STATE* cb_state, uint32_t barrier_count,
                                          const Barrier* barriers);
    void RecordBarrierValidationInfo(const char* func_name, CMD_BUFFER_STATE* cb_state, uint32_t bufferBarrierCount,
                                     const VkBufferMemoryBarrier* pBufferMemBarriers, uint32_t imageMemBarrierCount,
                                     const VkImageMemoryBarrier* pImageMemBarriers);
    template <typename Barrier>
    bool ValidateQFOTransferBarrierUniqueness(const char* func_name, const CMD_BUFFER_STATE* cb_state, uint32_t barrier_count,
                                              const Barrier* barriers) const;
    bool IsReleaseOp(CMD_BUFFER_STATE* cb_state, const VkImageMemoryBarrier& barrier) const;
    bool ValidateBarriersQFOTransferUniqueness(const char* func_name, const CMD_BUFFER_STATE* cb_state, uint32_t bufferBarrierCount,
                                               const VkBufferMemoryBarrier* pBufferMemBarriers, uint32_t imageMemBarrierCount,
                                               const VkImageMemoryBarrier* pImageMemBarriers) const;
    bool ValidatePrimaryCommandBufferState(const CMD_BUFFER_STATE* pCB, int current_submit_count,
                                           QFOTransferCBScoreboards<VkImageMemoryBarrier>* qfo_image_scoreboards,
                                           QFOTransferCBScoreboards<VkBufferMemoryBarrier>* qfo_buffer_scoreboards) const;
    bool ValidatePipelineDrawtimeState(const LAST_BOUND_STATE& state, const CMD_BUFFER_STATE* pCB, CMD_TYPE cmd_type,
                                       const PIPELINE_STATE* pPipeline, const char* caller) const;
    bool ValidateCmdBufDrawState(const CMD_BUFFER_STATE* cb_node, CMD_TYPE cmd_type, const bool indexed,
                                 const VkPipelineBindPoint bind_point, const char* function, const char* pipe_err_code,
                                 const char* state_err_code) const;
    bool ValidateEventStageMask(VkQueue queue, CMD_BUFFER_STATE* pCB, size_t eventCount, size_t firstEventIndex,
                                VkPipelineStageFlags sourceStageMask);
    bool ValidateQueueFamilyIndices(const CMD_BUFFER_STATE* pCB, VkQueue queue) const;
    VkResult CoreLayerCreateValidationCacheEXT(VkDevice device, const VkValidationCacheCreateInfoEXT* pCreateInfo,
                                               const VkAllocationCallbacks* pAllocator, VkValidationCacheEXT* pValidationCache);
    void CoreLayerDestroyValidationCacheEXT(VkDevice device, VkValidationCacheEXT validationCache,
                                            const VkAllocationCallbacks* pAllocator);
    VkResult CoreLayerMergeValidationCachesEXT(VkDevice device, VkValidationCacheEXT dstCache, uint32_t srcCacheCount,
                                               const VkValidationCacheEXT* pSrcCaches);
    VkResult CoreLayerGetValidationCacheDataEXT(VkDevice device, VkValidationCacheEXT validationCache, size_t* pDataSize,
                                                void* pData);
    // For given bindings validate state at time of draw is correct, returning false on error and writing error details into string*
    bool ValidateDrawState(const cvdescriptorset::DescriptorSet* descriptor_set, const std::map<uint32_t, descriptor_req>& bindings,
                           const std::vector<uint32_t>& dynamic_offsets, const CMD_BUFFER_STATE* cb_node, const char* caller,
                           std::string* error) const;
    // Validate contents of a CopyUpdate
    using DescriptorSet = cvdescriptorset::DescriptorSet;
    bool ValidateCopyUpdate(const VkCopyDescriptorSet* update, const DescriptorSet* dst_set, const DescriptorSet* src_set,
                            const char* func_name, std::string* error_code, std::string* error_msg);
    bool VerifyCopyUpdateContents(const VkCopyDescriptorSet* update, const DescriptorSet* src_set, VkDescriptorType type,
                                  uint32_t index, const char* func_name, std::string* error_code, std::string* error_msg);
    // Validate contents of a WriteUpdate
    bool ValidateWriteUpdate(const DescriptorSet* descriptor_set, const VkWriteDescriptorSet* update, const char* func_name,
                             std::string* error_code, std::string* error_msg) const;
    bool VerifyWriteUpdateContents(const DescriptorSet* dest_set, const VkWriteDescriptorSet* update, const uint32_t index,
                                   const char* func_name, std::string* error_code, std::string* error_msg) const;
    // Shared helper functions - These are useful because the shared sampler image descriptor type
    //  performs common functions with both sampler and image descriptors so they can share their common functions
    bool ValidateImageUpdate(VkImageView, VkImageLayout, VkDescriptorType, const char* func_name, std::string*, std::string*) const;
    // Validate contents of a push descriptor update
    bool ValidatePushDescriptorsUpdate(const DescriptorSet* push_set, uint32_t write_count, const VkWriteDescriptorSet* p_wds,
                                       const char* func_name) const;
    // Descriptor Set Validation Functions
    bool ValidateSampler(VkSampler) const;
    bool ValidateBufferUpdate(VkDescriptorBufferInfo const* buffer_info, VkDescriptorType type, const char* func_name,
                              std::string* error_code, std::string* error_msg) const;
    bool ValidateUpdateDescriptorSetsWithTemplateKHR(VkDescriptorSet descriptorSet, const TEMPLATE_STATE* template_state,
                                                     const void* pData);
    bool ValidateAllocateDescriptorSets(const VkDescriptorSetAllocateInfo*, const cvdescriptorset::AllocateDescriptorSetsData*);
    bool ValidateUpdateDescriptorSets(uint32_t write_count, const VkWriteDescriptorSet* p_wds, uint32_t copy_count,
                                      const VkCopyDescriptorSet* p_cds, const char* func_name);

    // Stuff from shader_validation
    bool ValidateGraphicsPipelineShaderState(const PIPELINE_STATE* pPipeline) const;
    bool ValidateComputePipeline(PIPELINE_STATE* pPipeline) const;
    bool ValidateRayTracingPipelineNV(PIPELINE_STATE* pipeline) const;
    bool PreCallValidateCreateShaderModule(VkDevice device, const VkShaderModuleCreateInfo* pCreateInfo,
                                           const VkAllocationCallbacks* pAllocator, VkShaderModule* pShaderModule);
    void PreCallRecordCreateShaderModule(VkDevice device, const VkShaderModuleCreateInfo* pCreateInfo,
                                         const VkAllocationCallbacks* pAllocator, VkShaderModule* pShaderModule, void* csm_state);
    bool ValidatePipelineShaderStage(VkPipelineShaderStageCreateInfo const* pStage, const PIPELINE_STATE* pipeline,
                                     const PIPELINE_STATE::StageState& stage_state, const SHADER_MODULE_STATE* module,
                                     const spirv_inst_iter& entrypoint, bool check_point_size) const;
    bool ValidatePointListShaderState(const PIPELINE_STATE* pipeline, SHADER_MODULE_STATE const* src, spirv_inst_iter entrypoint,
                                      VkShaderStageFlagBits stage) const;
    bool ValidateShaderCapabilities(SHADER_MODULE_STATE const* src, VkShaderStageFlagBits stage) const;
    bool ValidateShaderStageWritableDescriptor(VkShaderStageFlagBits stage, bool has_writable_descriptor) const;
    bool ValidateShaderStageInputOutputLimits(SHADER_MODULE_STATE const* src, VkPipelineShaderStageCreateInfo const* pStage,
                                              const PIPELINE_STATE* pipeline, spirv_inst_iter entrypoint) const;
    bool ValidateShaderStageGroupNonUniform(SHADER_MODULE_STATE const* src, VkShaderStageFlagBits stage) const;
    bool ValidateCooperativeMatrix(SHADER_MODULE_STATE const* src, VkPipelineShaderStageCreateInfo const* pStage,
                                   const PIPELINE_STATE* pipeline) const;
    bool ValidateExecutionModes(SHADER_MODULE_STATE const* src, spirv_inst_iter entrypoint) const;

    // Gpu Validation Functions
    void GpuPreCallRecordCreateDevice(VkPhysicalDevice gpu, safe_VkDeviceCreateInfo* modified_create_info,
                                      VkPhysicalDeviceFeatures* supported_features);
    void GpuPostCallRecordCreateDevice(const CHECK_ENABLED* enables, const VkDeviceCreateInfo* pCreateInfo,
                                       VkPhysicalDeviceFeatures* enabled_features);
    void GpuPreCallRecordDestroyDevice();
    void GpuResetCommandBuffer(const VkCommandBuffer commandBuffer);
    bool GpuPreCallCreateShaderModule(const VkShaderModuleCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator,
                                      VkShaderModule* pShaderModule, uint32_t* unique_shader_id,
                                      VkShaderModuleCreateInfo* instrumented_create_info,
                                      std::vector<unsigned int>* instrumented_pgm);
    bool GpuPreCallCreatePipelineLayout(const VkPipelineLayoutCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator,
                                        VkPipelineLayout* pPipelineLayout, std::vector<VkDescriptorSetLayout>* new_layouts,
                                        VkPipelineLayoutCreateInfo* modified_create_info);
    void GpuPostCallCreatePipelineLayout(VkResult result);
    void GpuPreCallRecordQueueSubmit(VkQueue queue, uint32_t submitCount, const VkSubmitInfo* pSubmits, VkFence fence);
    void GpuPostCallQueueSubmit(VkQueue queue, uint32_t submitCount, const VkSubmitInfo* pSubmits, VkFence fence);
    void GpuPreCallValidateCmdWaitEvents(VkPipelineStageFlags sourceStageMask);
    std::vector<safe_VkGraphicsPipelineCreateInfo> GpuPreCallRecordCreateGraphicsPipelines(
        VkPipelineCache pipelineCache, uint32_t count, const VkGraphicsPipelineCreateInfo* pCreateInfos,
        const VkAllocationCallbacks* pAllocator, VkPipeline* pPipelines, std::vector<std::unique_ptr<PIPELINE_STATE>>& pipe_state);
    void GpuPostCallRecordCreateGraphicsPipelines(const uint32_t count, const VkGraphicsPipelineCreateInfo* pCreateInfos,
                                                  const VkAllocationCallbacks* pAllocator, VkPipeline* pPipelines);
    void GpuPreCallRecordDestroyPipeline(const VkPipeline pipeline);
    void GpuAllocateValidationResources(const VkCommandBuffer cmd_buffer, VkPipelineBindPoint bind_point);
    void AnalyzeAndReportError(CMD_BUFFER_STATE* cb_node, VkQueue queue, VkPipelineBindPoint bind_point, uint32_t operation_index,
                               uint32_t* const debug_output_buffer);
    void ProcessInstrumentationBuffer(VkQueue queue, CMD_BUFFER_STATE* cb_node);
    void UpdateInstrumentationBuffer(CMD_BUFFER_STATE* cb_node);
    void SubmitBarrier(VkQueue queue);
    bool GpuInstrumentShader(const VkShaderModuleCreateInfo* pCreateInfo, std::vector<unsigned int>& new_pgm,
                             uint32_t* unique_shader_id);
    template <typename CreateInfo, typename SafeCreateInfo>
    void GpuPreCallRecordPipelineCreations(uint32_t count, const CreateInfo* pCreateInfos, const VkAllocationCallbacks* pAllocator,
                                           VkPipeline* pPipelines, std::vector<std::unique_ptr<PIPELINE_STATE>>& pipe_state,
                                           std::vector<SafeCreateInfo>* new_pipeline_create_infos,
                                           const VkPipelineBindPoint bind_point);
    template <typename CreateInfo>
    void GpuPostCallRecordPipelineCreations(const uint32_t count, const CreateInfo* pCreateInfos,
                                            const VkAllocationCallbacks* pAllocator, VkPipeline* pPipelines,
                                            const VkPipelineBindPoint bind_point);
    std::vector<safe_VkComputePipelineCreateInfo> GpuPreCallRecordCreateComputePipelines(
        VkPipelineCache pipelineCache, uint32_t count, const VkComputePipelineCreateInfo* pCreateInfos,
        const VkAllocationCallbacks* pAllocator, VkPipeline* pPipelines, std::vector<std::unique_ptr<PIPELINE_STATE>>& pipe_state);
    void GpuPostCallRecordCreateComputePipelines(const uint32_t count, const VkComputePipelineCreateInfo* pCreateInfos,
                                                 const VkAllocationCallbacks* pAllocator, VkPipeline* pPipelines);
    std::vector<safe_VkRayTracingPipelineCreateInfoNV> GpuPreCallRecordCreateRayTracingPipelinesNV(
        VkPipelineCache pipelineCache, uint32_t count, const VkRayTracingPipelineCreateInfoNV* pCreateInfos,
        const VkAllocationCallbacks* pAllocator, VkPipeline* pPipelines, std::vector<std::unique_ptr<PIPELINE_STATE>>& pipe_state);
    void GpuPostCallRecordCreateRayTracingPipelinesNV(const uint32_t count, const VkRayTracingPipelineCreateInfoNV* pCreateInfos,
                                                      const VkAllocationCallbacks* pAllocator, VkPipeline* pPipelines);
    void GpuPostCallRecordGetBufferDeviceAddressEXT(const VkBufferDeviceAddressInfoEXT* pInfo, VkDeviceAddress address);
    void GpuPreCallRecordDestroyBuffer(VkBuffer buffer);
    VkResult GpuInitializeVma();
    void ReportSetupProblem(VkDebugReportObjectTypeEXT object_type, uint64_t object_handle, const char* const specific_message);
    void GpuCreateAccelerationStructureBuildValidationState();
    void GpuDestroyAccelerationStructureBuildValidationState();
    void GpuPreCallRecordCmdBuildAccelerationStructureNV(VkCommandBuffer commandBuffer, const VkAccelerationStructureInfoNV* pInfo,
                                                         VkBuffer instanceData, VkDeviceSize instanceOffset, VkBool32 update,
                                                         VkAccelerationStructureNV dst, VkAccelerationStructureNV src,
                                                         VkBuffer scratch, VkDeviceSize scratchOffset);
    void GpuProcessAccelerationStructureBuildValidationBuffer(VkQueue queue, CMD_BUFFER_STATE* cb_node);
    void GpuPreCallRecordCreateBuffer(VkDevice device, const VkBufferCreateInfo* pCreateInfo,
                                      const VkAllocationCallbacks* pAllocator, VkBuffer* pBuffer,
                                      VkBufferCreateInfo* modified_create_info);

    // Buffer Validation Functions
    template <class OBJECT, class LAYOUT>
    void SetLayout(OBJECT* pObject, VkImage image, VkImageSubresource range, const LAYOUT& layout);
    template <class OBJECT, class LAYOUT>
    void SetLayout(OBJECT* pObject, ImageSubresourcePair imgpair, const LAYOUT& layout, VkImageAspectFlags aspectMask);
    // Remove the pending QFO release records from the global set
    // Note that the type of the handle argument constrained to match Barrier type
    // The defaulted BarrierRecord argument allows use to declare the type once, but is not intended to be specified by the caller
    template <typename Barrier, typename BarrierRecord = QFOTransferBarrier<Barrier>>
    void EraseQFOReleaseBarriers(const typename BarrierRecord::HandleType& handle) {
        GlobalQFOTransferBarrierMap<Barrier>& global_release_barriers =
            GetGlobalQFOReleaseBarrierMap(typename BarrierRecord::Tag());
        global_release_barriers.erase(handle);
    }
    bool ValidateCopyImageTransferGranularityRequirements(const CMD_BUFFER_STATE* cb_node, const IMAGE_STATE* src_img,
                                                          const IMAGE_STATE* dst_img, const VkImageCopy* region, const uint32_t i,
                                                          const char* function) const;
    bool ValidateIdleBuffer(VkBuffer buffer);
    bool ValidateUsageFlags(VkFlags actual, VkFlags desired, VkBool32 strict, const VulkanTypedHandle& typed_handle,
                            const char* msgCode, char const* func_name, char const* usage_str) const;
    bool ValidateImageSubresourceRange(const uint32_t image_mip_count, const uint32_t image_layer_count,
                                       const VkImageSubresourceRange& subresourceRange, const char* cmd_name,
                                       const char* param_name, const char* image_layer_count_var_name, const uint64_t image_handle,
                                       SubresourceRangeErrorCodes errorCodes) const;
    void SetImageLayout(CMD_BUFFER_STATE* cb_node, const IMAGE_STATE& image_state,
                        const VkImageSubresourceRange& image_subresource_range, VkImageLayout layout,
                        VkImageLayout expected_layout = kInvalidLayout);
    void SetImageLayout(CMD_BUFFER_STATE* cb_node, const IMAGE_STATE& image_state,
                        const VkImageSubresourceLayers& image_subresource_layers, VkImageLayout layout);
    bool ValidateRenderPassLayoutAgainstFramebufferImageUsage(RenderPassCreateVersion rp_version, VkImageLayout layout,
                                                              VkImage image, VkImageView image_view, VkFramebuffer framebuffer,
                                                              VkRenderPass renderpass, uint32_t attachment_index,
                                                              const char* variable_name) const;
    bool ValidateBufferImageCopyData(uint32_t regionCount, const VkBufferImageCopy* pRegions, IMAGE_STATE* image_state,
                                     const char* function);
    bool ValidateBufferViewRange(const BUFFER_STATE* buffer_state, const VkBufferViewCreateInfo* pCreateInfo,
                                 const VkPhysicalDeviceLimits* device_limits);
    bool ValidateBufferViewBuffer(const BUFFER_STATE* buffer_state, const VkBufferViewCreateInfo* pCreateInfo);

    bool PreCallValidateCreateImage(VkDevice device, const VkImageCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator,
                                    VkImage* pImage);

    void PostCallRecordCreateImage(VkDevice device, const VkImageCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator,
                                   VkImage* pImage, VkResult result);

    void PreCallRecordDestroyImage(VkDevice device, VkImage image, const VkAllocationCallbacks* pAllocator);

    bool PreCallValidateDestroyImage(VkDevice device, VkImage image, const VkAllocationCallbacks* pAllocator);

    bool ValidateImageAttributes(const IMAGE_STATE* image_state, const VkImageSubresourceRange& range) const;

    bool ValidateClearAttachmentExtent(VkCommandBuffer command_buffer, uint32_t attachment_index,
                                       const FRAMEBUFFER_STATE* framebuffer, uint32_t fb_attachment, const VkRect2D& render_area,
                                       uint32_t rect_count, const VkClearRect* clear_rects) const;
    bool ValidateImageCopyData(const uint32_t regionCount, const VkImageCopy* ic_regions, const IMAGE_STATE* src_state,
                               const IMAGE_STATE* dst_state) const;

    bool VerifyClearImageLayout(const CMD_BUFFER_STATE* cb_node, const IMAGE_STATE* image_state,
                                const VkImageSubresourceRange& range, VkImageLayout dest_image_layout, const char* func_name) const;

    bool VerifyImageLayout(const CMD_BUFFER_STATE* cb_node, const IMAGE_STATE* image_state, const VkImageSubresourceRange& range,
                           VkImageAspectFlags view_aspect, VkImageLayout explicit_layout, VkImageLayout optimal_layout,
                           const char* caller, const char* layout_invalid_msg_code, const char* layout_mismatch_msg_code,
                           bool* error) const;

    bool VerifyImageLayout(const CMD_BUFFER_STATE* cb_node, const IMAGE_STATE* image_state, const VkImageSubresourceRange& range,
                           VkImageLayout explicit_layout, VkImageLayout optimal_layout, const char* caller,
                           const char* layout_invalid_msg_code, const char* layout_mismatch_msg_code, bool* error) const {
        return VerifyImageLayout(cb_node, image_state, range, 0, explicit_layout, optimal_layout, caller, layout_invalid_msg_code,
                                 layout_mismatch_msg_code, error);
    }

    bool VerifyImageLayout(const CMD_BUFFER_STATE* cb_node, const IMAGE_STATE* image_state,
                           const VkImageSubresourceLayers& subLayers, VkImageLayout explicit_layout, VkImageLayout optimal_layout,
                           const char* caller, const char* layout_invalid_msg_code, const char* layout_mismatch_msg_code,
                           bool* error) const;

    bool CheckItgExtent(const CMD_BUFFER_STATE* cb_node, const VkExtent3D* extent, const VkOffset3D* offset,
                        const VkExtent3D* granularity, const VkExtent3D* subresource_extent, const VkImageType image_type,
                        const uint32_t i, const char* function, const char* member, const char* vuid) const;

    bool CheckItgOffset(const CMD_BUFFER_STATE* cb_node, const VkOffset3D* offset, const VkExtent3D* granularity, const uint32_t i,
                        const char* function, const char* member, const char* vuid) const;
    VkExtent3D GetScaledItg(const CMD_BUFFER_STATE* cb_node, const IMAGE_STATE* img) const;
    bool CopyImageMultiplaneValidation(VkCommandBuffer command_buffer, const IMAGE_STATE* src_image_state,
                                       const IMAGE_STATE* dst_image_state, const VkImageCopy region) const;

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

    bool FindLayoutVerifyLayout(ImageSubresourcePair imgpair, VkImageLayout& layout, const VkImageAspectFlags aspectMask);

    bool FindGlobalLayout(ImageSubresourcePair imgpair, VkImageLayout& layout);

    bool FindLayouts(VkImage image, std::vector<VkImageLayout>& layouts);

    bool FindLayout(const ImageSubresPairLayoutMap& imageLayoutMap, ImageSubresourcePair imgpair, VkImageLayout& layout) const;

    static bool FindLayout(const ImageSubresPairLayoutMap& imageLayoutMap, ImageSubresourcePair imgpair, VkImageLayout& layout,
                           const VkImageAspectFlags aspectMask);

    void SetGlobalLayout(ImageSubresourcePair imgpair, const VkImageLayout& layout);

    void SetImageViewLayout(CMD_BUFFER_STATE* cb_node, const IMAGE_VIEW_STATE& view_state, VkImageLayout layout);
    void SetImageViewInitialLayout(CMD_BUFFER_STATE* cb_node, const IMAGE_VIEW_STATE& view_state, VkImageLayout layout);

    void SetImageInitialLayout(CMD_BUFFER_STATE* cb_node, VkImage image, const VkImageSubresourceRange& range,
                               VkImageLayout layout);
    void SetImageInitialLayout(CMD_BUFFER_STATE* cb_node, const IMAGE_STATE& image_state, const VkImageSubresourceRange& range,
                               VkImageLayout layout);
    void SetImageInitialLayout(CMD_BUFFER_STATE* cb_node, const IMAGE_STATE& image_state, const VkImageSubresourceLayers& layers,
                               VkImageLayout layout);

    bool VerifyFramebufferAndRenderPassLayouts(RenderPassCreateVersion rp_version, const CMD_BUFFER_STATE* pCB,
                                               const VkRenderPassBeginInfo* pRenderPassBegin,
                                               const FRAMEBUFFER_STATE* framebuffer_state) const;
    void RecordCmdBeginRenderPassLayouts(VkCommandBuffer commandBuffer, const VkRenderPassBeginInfo* pRenderPassBegin,
                                         const VkSubpassContents contents);
    void TransitionAttachmentRefLayout(CMD_BUFFER_STATE* pCB, FRAMEBUFFER_STATE* pFramebuffer,
                                       const safe_VkAttachmentReference2KHR& ref);

    void TransitionSubpassLayouts(CMD_BUFFER_STATE*, const RENDER_PASS_STATE*, const int, FRAMEBUFFER_STATE*);

    void TransitionBeginRenderPassLayouts(CMD_BUFFER_STATE*, const RENDER_PASS_STATE*, FRAMEBUFFER_STATE*);

    bool ValidateBarrierLayoutToImageUsage(const VkImageMemoryBarrier& img_barrier, bool new_not_old, VkImageUsageFlags usage,
                                           const char* func_name, const char* barrier_pname) const;

    bool ValidateBarriersToImages(const CMD_BUFFER_STATE* cb_state, uint32_t imageMemoryBarrierCount,
                                  const VkImageMemoryBarrier* pImageMemoryBarriers, const char* func_name) const;

    void RecordQueuedQFOTransfers(CMD_BUFFER_STATE* pCB);
    void EraseQFOImageRelaseBarriers(const VkImage& image);

    void TransitionImageLayouts(CMD_BUFFER_STATE* cb_state, uint32_t memBarrierCount, const VkImageMemoryBarrier* pImgMemBarriers);

    void RecordTransitionImageLayout(CMD_BUFFER_STATE* cb_state, const IMAGE_STATE* image_state,
                                     const VkImageMemoryBarrier& mem_barrier);

    void TransitionFinalSubpassLayouts(CMD_BUFFER_STATE* pCB, const VkRenderPassBeginInfo* pRenderPassBegin,
                                       FRAMEBUFFER_STATE* framebuffer_state);

    bool PreCallValidateCmdCopyImage(VkCommandBuffer commandBuffer, VkImage srcImage, VkImageLayout srcImageLayout,
                                     VkImage dstImage, VkImageLayout dstImageLayout, uint32_t regionCount,
                                     const VkImageCopy* pRegions);

    bool PreCallValidateCmdClearAttachments(VkCommandBuffer commandBuffer, uint32_t attachmentCount,
                                            const VkClearAttachment* pAttachments, uint32_t rectCount, const VkClearRect* pRects);
    void PreCallRecordCmdClearAttachments(VkCommandBuffer commandBuffer, uint32_t attachmentCount,
                                          const VkClearAttachment* pAttachments, uint32_t rectCount, const VkClearRect* pRects);

    bool PreCallValidateCmdResolveImage(VkCommandBuffer commandBuffer, VkImage srcImage, VkImageLayout srcImageLayout,
                                        VkImage dstImage, VkImageLayout dstImageLayout, uint32_t regionCount,
                                        const VkImageResolve* pRegions);

    bool PreCallValidateCmdBlitImage(VkCommandBuffer commandBuffer, VkImage srcImage, VkImageLayout srcImageLayout,
                                     VkImage dstImage, VkImageLayout dstImageLayout, uint32_t regionCount,
                                     const VkImageBlit* pRegions, VkFilter filter);

    void PreCallRecordCmdBlitImage(VkCommandBuffer commandBuffer, VkImage srcImage, VkImageLayout srcImageLayout, VkImage dstImage,
                                   VkImageLayout dstImageLayout, uint32_t regionCount, const VkImageBlit* pRegions,
                                   VkFilter filter);

    bool ValidateCmdBufImageLayouts(const CMD_BUFFER_STATE* pCB, const ImageSubresPairLayoutMap& globalImageLayoutMap,
                                    ImageSubresPairLayoutMap* overlayLayoutMap_arg) const;

    void UpdateCmdBufImageLayouts(CMD_BUFFER_STATE* pCB);

    bool VerifyBoundMemoryIsValid(VkDeviceMemory mem, const VulkanTypedHandle& typed_handle, const char* api_name,
                                  const char* error_code) const;

    bool ValidateLayoutVsAttachmentDescription(const debug_report_data* report_data, RenderPassCreateVersion rp_version,
                                               const VkImageLayout first_layout, const uint32_t attachment,
                                               const VkAttachmentDescription2KHR& attachment_description) const;

    bool ValidateLayouts(RenderPassCreateVersion rp_version, VkDevice device, const VkRenderPassCreateInfo2KHR* pCreateInfo) const;

    bool ValidateImageUsageFlags(IMAGE_STATE const* image_state, VkFlags desired, bool strict, const char* msgCode,
                                 char const* func_name, char const* usage_string) const;

    bool ValidateImageFormatFeatureFlags(IMAGE_STATE const* image_state, VkFormatFeatureFlags desired, char const* func_name,
                                         const char* linear_vuid, const char* optimal_vuid) const;

    bool ValidateImageSubresourceLayers(const CMD_BUFFER_STATE* cb_node, const VkImageSubresourceLayers* subresource_layers,
                                        char const* func_name, char const* member, uint32_t i) const;

    bool ValidateBufferUsageFlags(BUFFER_STATE const* buffer_state, VkFlags desired, bool strict, const char* msgCode,
                                  char const* func_name, char const* usage_string) const;

    bool PreCallValidateCreateBuffer(VkDevice device, const VkBufferCreateInfo* pCreateInfo,
                                     const VkAllocationCallbacks* pAllocator, VkBuffer* pBuffer);

    bool PreCallValidateCreateBufferView(VkDevice device, const VkBufferViewCreateInfo* pCreateInfo,
                                         const VkAllocationCallbacks* pAllocator, VkBufferView* pView);

    bool ValidateImageAspectMask(VkImage image, VkFormat format, VkImageAspectFlags aspect_mask, const char* func_name,
                                 const char* vuid = "VUID-VkImageSubresource-aspectMask-parameter") const;

    bool ValidateCreateImageViewSubresourceRange(const IMAGE_STATE* image_state, bool is_imageview_2d_type,
                                                 const VkImageSubresourceRange& subresourceRange);

    bool ValidateCmdClearColorSubresourceRange(const IMAGE_STATE* image_state, const VkImageSubresourceRange& subresourceRange,
                                               const char* param_name) const;

    bool ValidateCmdClearDepthSubresourceRange(const IMAGE_STATE* image_state, const VkImageSubresourceRange& subresourceRange,
                                               const char* param_name) const;

    bool ValidateImageBarrierSubresourceRange(const IMAGE_STATE* image_state, const VkImageSubresourceRange& subresourceRange,
                                              const char* cmd_name, const char* param_name) const;

    bool PreCallValidateCreateImageView(VkDevice device, const VkImageViewCreateInfo* pCreateInfo,
                                        const VkAllocationCallbacks* pAllocator, VkImageView* pView);

    bool ValidateCopyBufferImageTransferGranularityRequirements(const CMD_BUFFER_STATE* cb_node, const IMAGE_STATE* img,
                                                                const VkBufferImageCopy* region, const uint32_t i,
                                                                const char* function, const char* vuid) const;

    bool ValidateImageMipLevel(const CMD_BUFFER_STATE* cb_node, const IMAGE_STATE* img, uint32_t mip_level, const uint32_t i,
                               const char* function, const char* member, const char* vuid) const;

    bool ValidateImageArrayLayerRange(const CMD_BUFFER_STATE* cb_node, const IMAGE_STATE* img, const uint32_t base_layer,
                                      const uint32_t layer_count, const uint32_t i, const char* function, const char* member,
                                      const char* vuid) const;

    void PreCallRecordCmdCopyImage(VkCommandBuffer commandBuffer, VkImage srcImage, VkImageLayout srcImageLayout, VkImage dstImage,
                                   VkImageLayout dstImageLayout, uint32_t regionCount, const VkImageCopy* pRegions);

    bool PreCallValidateCmdCopyBuffer(VkCommandBuffer commandBuffer, VkBuffer srcBuffer, VkBuffer dstBuffer, uint32_t regionCount,
                                      const VkBufferCopy* pRegions);
    bool PreCallValidateDestroyImageView(VkDevice device, VkImageView imageView, const VkAllocationCallbacks* pAllocator);

    void PreCallRecordCreateBuffer(VkDevice device, const VkBufferCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator,
                                   VkBuffer* pBuffer, void* cb_state);

    bool PreCallValidateDestroyBuffer(VkDevice device, VkBuffer buffer, const VkAllocationCallbacks* pAllocator);

    void PreCallRecordDestroyBuffer(VkDevice device, VkBuffer buffer, const VkAllocationCallbacks* pAllocator);

    bool PreCallValidateDestroyBufferView(VkDevice device, VkBufferView bufferView, const VkAllocationCallbacks* pAllocator);

    bool PreCallValidateCmdFillBuffer(VkCommandBuffer commandBuffer, VkBuffer dstBuffer, VkDeviceSize dstOffset, VkDeviceSize size,
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
    bool ValidateCreateImageANDROID(const debug_report_data* report_data, const VkImageCreateInfo* create_info);
    bool ValidateCreateImageViewANDROID(const VkImageViewCreateInfo* create_info);
    bool ValidateGetImageSubresourceLayoutANDROID(const VkImage image) const;
    bool ValidateQueueFamilies(uint32_t queue_family_count, const uint32_t* queue_families, const char* cmd_name,
                               const char* array_parameter_name, const char* unique_error_code, const char* valid_error_code,
                               bool optional) const;
    bool ValidateAllocateMemoryANDROID(const VkMemoryAllocateInfo* alloc_info) const;
    bool ValidateGetImageMemoryRequirements2ANDROID(const VkImage image) const;
    bool ValidateCreateSamplerYcbcrConversionANDROID(const VkSamplerYcbcrConversionCreateInfo* create_info) const;
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
    void PreCallRecordCreateComputePipelines(VkDevice device, VkPipelineCache pipelineCache, uint32_t count,
                                             const VkComputePipelineCreateInfo* pCreateInfos,
                                             const VkAllocationCallbacks* pAllocator, VkPipeline* pPipelines,
                                             void* ccpl_state_data);
    void PostCallRecordCreateComputePipelines(VkDevice device, VkPipelineCache pipelineCache, uint32_t count,
                                              const VkComputePipelineCreateInfo* pCreateInfos,
                                              const VkAllocationCallbacks* pAllocator, VkPipeline* pPipelines, VkResult result,
                                              void* pipe_state);
    bool PreCallValidateGetPipelineExecutablePropertiesKHR(VkDevice device, const VkPipelineInfoKHR* pPipelineInfo,
                                                           uint32_t* pExecutableCount,
                                                           VkPipelineExecutablePropertiesKHR* pProperties);
    bool ValidatePipelineExecutableInfo(VkDevice device, const VkPipelineExecutableInfoKHR* pExecutableInfo) const;
    bool PreCallValidateGetPipelineExecutableStatisticsKHR(VkDevice device, const VkPipelineExecutableInfoKHR* pExecutableInfo,
                                                           uint32_t* pStatisticCount,
                                                           VkPipelineExecutableStatisticKHR* pStatistics);
    bool PreCallValidateGetPipelineExecutableInternalRepresentationsKHR(VkDevice device,
                                                                        const VkPipelineExecutableInfoKHR* pExecutableInfo,
                                                                        uint32_t* pInternalRepresentationCount,
                                                                        VkPipelineExecutableInternalRepresentationKHR* pStatistics);
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
    bool PreCallValidateCreateRayTracingPipelinesNV(VkDevice device, VkPipelineCache pipelineCache, uint32_t count,
                                                    const VkRayTracingPipelineCreateInfoNV* pCreateInfos,
                                                    const VkAllocationCallbacks* pAllocator, VkPipeline* pPipelines,
                                                    void* pipe_state);
    void PreCallRecordCreateRayTracingPipelinesNV(VkDevice device, VkPipelineCache pipelineCache, uint32_t count,
                                                  const VkRayTracingPipelineCreateInfoNV* pCreateInfos,
                                                  const VkAllocationCallbacks* pAllocator, VkPipeline* pPipelines,
                                                  void* crtpl_state_data);
    void PostCallRecordCreateRayTracingPipelinesNV(VkDevice device, VkPipelineCache pipelineCache, uint32_t count,
                                                   const VkRayTracingPipelineCreateInfoNV* pCreateInfos,
                                                   const VkAllocationCallbacks* pAllocator, VkPipeline* pPipelines, VkResult result,
                                                   void* crtpl_state_data);
    void PreCallRecordCmdTraceRaysNV(VkCommandBuffer commandBuffer, VkBuffer raygenShaderBindingTableBuffer,
                                     VkDeviceSize raygenShaderBindingOffset, VkBuffer missShaderBindingTableBuffer,
                                     VkDeviceSize missShaderBindingOffset, VkDeviceSize missShaderBindingStride,
                                     VkBuffer hitShaderBindingTableBuffer, VkDeviceSize hitShaderBindingOffset,
                                     VkDeviceSize hitShaderBindingStride, VkBuffer callableShaderBindingTableBuffer,
                                     VkDeviceSize callableShaderBindingOffset, VkDeviceSize callableShaderBindingStride,
                                     uint32_t width, uint32_t height, uint32_t depth);
    void PostCallRecordCmdTraceRaysNV(VkCommandBuffer commandBuffer, VkBuffer raygenShaderBindingTableBuffer,
                                      VkDeviceSize raygenShaderBindingOffset, VkBuffer missShaderBindingTableBuffer,
                                      VkDeviceSize missShaderBindingOffset, VkDeviceSize missShaderBindingStride,
                                      VkBuffer hitShaderBindingTableBuffer, VkDeviceSize hitShaderBindingOffset,
                                      VkDeviceSize hitShaderBindingStride, VkBuffer callableShaderBindingTableBuffer,
                                      VkDeviceSize callableShaderBindingOffset, VkDeviceSize callableShaderBindingStride,
                                      uint32_t width, uint32_t height, uint32_t depth);
    void PostCallRecordCreateInstance(const VkInstanceCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator,
                                      VkInstance* pInstance, VkResult result);
    bool PreCallValidateCreateDevice(VkPhysicalDevice gpu, const VkDeviceCreateInfo* pCreateInfo,
                                     const VkAllocationCallbacks* pAllocator, VkDevice* pDevice);
    void PreCallRecordCreateDevice(VkPhysicalDevice gpu, const VkDeviceCreateInfo* pCreateInfo,
                                   const VkAllocationCallbacks* pAllocator, VkDevice* pDevice,
                                   safe_VkDeviceCreateInfo* modified_create_info);
    void PostCallRecordCreateDevice(VkPhysicalDevice gpu, const VkDeviceCreateInfo* pCreateInfo,
                                    const VkAllocationCallbacks* pAllocator, VkDevice* pDevice, VkResult result);
    bool PreCallValidateCmdUpdateBuffer(VkCommandBuffer commandBuffer, VkBuffer dstBuffer, VkDeviceSize dstOffset,
                                        VkDeviceSize dataSize, const void* pData);
    bool PreCallValidateGetDeviceQueue(VkDevice device, uint32_t queueFamilyIndex, uint32_t queueIndex, VkQueue* pQueue);
    bool PreCallValidateCreateSamplerYcbcrConversion(VkDevice device, const VkSamplerYcbcrConversionCreateInfo* pCreateInfo,
                                                     const VkAllocationCallbacks* pAllocator,
                                                     VkSamplerYcbcrConversion* pYcbcrConversion);
    bool PreCallValidateCreateSamplerYcbcrConversionKHR(VkDevice device, const VkSamplerYcbcrConversionCreateInfo* pCreateInfo,
                                                        const VkAllocationCallbacks* pAllocator,
                                                        VkSamplerYcbcrConversion* pYcbcrConversion);
    bool PreCallValidateCmdDebugMarkerBeginEXT(VkCommandBuffer commandBuffer, const VkDebugMarkerMarkerInfoEXT* pMarkerInfo);
    void PreCallRecordDestroyDevice(VkDevice device, const VkAllocationCallbacks* pAllocator);
    bool PreCallValidateQueueSubmit(VkQueue queue, uint32_t submitCount, const VkSubmitInfo* pSubmits, VkFence fence);
    void PreCallRecordQueueSubmit(VkQueue queue, uint32_t submitCount, const VkSubmitInfo* pSubmits, VkFence fence);
    void PostCallRecordQueueSubmit(VkQueue queue, uint32_t submitCount, const VkSubmitInfo* pSubmits, VkFence fence,
                                   VkResult result);
    bool PreCallValidateAllocateMemory(VkDevice device, const VkMemoryAllocateInfo* pAllocateInfo,
                                       const VkAllocationCallbacks* pAllocator, VkDeviceMemory* pMemory);
    bool PreCallValidateFreeMemory(VkDevice device, VkDeviceMemory mem, const VkAllocationCallbacks* pAllocator);
    bool PreCallValidateWaitForFences(VkDevice device, uint32_t fenceCount, const VkFence* pFences, VkBool32 waitAll,
                                      uint64_t timeout);
    bool PreCallValidateQueueWaitIdle(VkQueue queue);
    bool PreCallValidateDeviceWaitIdle(VkDevice device);
    bool PreCallValidateDestroyFence(VkDevice device, VkFence fence, const VkAllocationCallbacks* pAllocator);
    bool PreCallValidateDestroySemaphore(VkDevice device, VkSemaphore semaphore, const VkAllocationCallbacks* pAllocator);
    bool PreCallValidateDestroyEvent(VkDevice device, VkEvent event, const VkAllocationCallbacks* pAllocator);
    bool PreCallValidateDestroyQueryPool(VkDevice device, VkQueryPool queryPool, const VkAllocationCallbacks* pAllocator);
    bool ValidateGetQueryPoolResultsFlags(VkQueryPool queryPool, VkQueryResultFlags flags) const;
    bool ValidateGetQueryPoolResultsQueries(VkQueryPool queryPool, uint32_t firstQuery, uint32_t queryCount) const;
    bool PreCallValidateGetQueryPoolResults(VkDevice device, VkQueryPool queryPool, uint32_t firstQuery, uint32_t queryCount,
                                            size_t dataSize, void* pData, VkDeviceSize stride, VkQueryResultFlags flags);
    bool PreCallValidateBindBufferMemory2KHR(VkDevice device, uint32_t bindInfoCount, const VkBindBufferMemoryInfoKHR* pBindInfos);
    bool PreCallValidateBindBufferMemory2(VkDevice device, uint32_t bindInfoCount, const VkBindBufferMemoryInfoKHR* pBindInfos);
    bool PreCallValidateBindBufferMemory(VkDevice device, VkBuffer buffer, VkDeviceMemory mem, VkDeviceSize memoryOffset);
    bool PreCallValidateGetImageMemoryRequirements2(VkDevice device, const VkImageMemoryRequirementsInfo2* pInfo,
                                                    VkMemoryRequirements2* pMemoryRequirements);
    bool PreCallValidateGetImageMemoryRequirements2KHR(VkDevice device, const VkImageMemoryRequirementsInfo2* pInfo,
                                                       VkMemoryRequirements2* pMemoryRequirements);
    bool PreCallValidateGetPhysicalDeviceImageFormatProperties2(VkPhysicalDevice physicalDevice,
                                                                const VkPhysicalDeviceImageFormatInfo2* pImageFormatInfo,
                                                                VkImageFormatProperties2* pImageFormatProperties);
    bool PreCallValidateGetPhysicalDeviceImageFormatProperties2KHR(VkPhysicalDevice physicalDevice,
                                                                   const VkPhysicalDeviceImageFormatInfo2* pImageFormatInfo,
                                                                   VkImageFormatProperties2* pImageFormatProperties);
    bool PreCallValidateDestroyPipeline(VkDevice device, VkPipeline pipeline, const VkAllocationCallbacks* pAllocator);
    void PreCallRecordDestroyPipeline(VkDevice device, VkPipeline pipeline, const VkAllocationCallbacks* pAllocator);
    bool PreCallValidateDestroySampler(VkDevice device, VkSampler sampler, const VkAllocationCallbacks* pAllocator);
    bool PreCallValidateDestroyDescriptorPool(VkDevice device, VkDescriptorPool descriptorPool,
                                              const VkAllocationCallbacks* pAllocator);
    bool PreCallValidateFreeCommandBuffers(VkDevice device, VkCommandPool commandPool, uint32_t commandBufferCount,
                                           const VkCommandBuffer* pCommandBuffers);
    bool PreCallValidateCreateCommandPool(VkDevice device, const VkCommandPoolCreateInfo* pCreateInfo,
                                          const VkAllocationCallbacks* pAllocator, VkCommandPool* pCommandPool);
    bool PreCallValidateCreateQueryPool(VkDevice device, const VkQueryPoolCreateInfo* pCreateInfo,
                                        const VkAllocationCallbacks* pAllocator, VkQueryPool* pQueryPool);
    bool PreCallValidateDestroyCommandPool(VkDevice device, VkCommandPool commandPool, const VkAllocationCallbacks* pAllocator);
    bool PreCallValidateResetCommandPool(VkDevice device, VkCommandPool commandPool, VkCommandPoolResetFlags flags);
    bool PreCallValidateResetFences(VkDevice device, uint32_t fenceCount, const VkFence* pFences);
    bool PreCallValidateDestroyFramebuffer(VkDevice device, VkFramebuffer framebuffer, const VkAllocationCallbacks* pAllocator);
    bool PreCallValidateDestroyRenderPass(VkDevice device, VkRenderPass renderPass, const VkAllocationCallbacks* pAllocator);
    bool PreCallValidateCreateDescriptorSetLayout(VkDevice device, const VkDescriptorSetLayoutCreateInfo* pCreateInfo,
                                                  const VkAllocationCallbacks* pAllocator, VkDescriptorSetLayout* pSetLayout);
    bool PreCallValidateResetDescriptorPool(VkDevice device, VkDescriptorPool descriptorPool, VkDescriptorPoolResetFlags flags);
    bool PreCallValidateFreeDescriptorSets(VkDevice device, VkDescriptorPool descriptorPool, uint32_t count,
                                           const VkDescriptorSet* pDescriptorSets);
    bool PreCallValidateUpdateDescriptorSets(VkDevice device, uint32_t descriptorWriteCount,
                                             const VkWriteDescriptorSet* pDescriptorWrites, uint32_t descriptorCopyCount,
                                             const VkCopyDescriptorSet* pDescriptorCopies);
    bool PreCallValidateBeginCommandBuffer(VkCommandBuffer commandBuffer, const VkCommandBufferBeginInfo* pBeginInfo);
    bool PreCallValidateEndCommandBuffer(VkCommandBuffer commandBuffer);
    bool PreCallValidateResetCommandBuffer(VkCommandBuffer commandBuffer, VkCommandBufferResetFlags flags);
    bool PreCallValidateCmdBindPipeline(VkCommandBuffer commandBuffer, VkPipelineBindPoint pipelineBindPoint, VkPipeline pipeline);
    bool PreCallValidateCmdSetViewport(VkCommandBuffer commandBuffer, uint32_t firstViewport, uint32_t viewportCount,
                                       const VkViewport* pViewports);
    bool PreCallValidateCmdSetScissor(VkCommandBuffer commandBuffer, uint32_t firstScissor, uint32_t scissorCount,
                                      const VkRect2D* pScissors);
    bool PreCallValidateCmdSetExclusiveScissorNV(VkCommandBuffer commandBuffer, uint32_t firstExclusiveScissor,
                                                 uint32_t exclusiveScissorCount, const VkRect2D* pExclusiveScissors);

    bool PreCallValidateCmdSetViewportWScalingNV(VkCommandBuffer commandBuffer, uint32_t firstViewport, uint32_t viewportCount,
                                                 const VkViewportWScalingNV* pViewportWScalings);

    bool PreCallValidateCmdBindShadingRateImageNV(VkCommandBuffer commandBuffer, VkImageView imageView, VkImageLayout imageLayout);
    bool PreCallValidateCmdSetViewportShadingRatePaletteNV(VkCommandBuffer commandBuffer, uint32_t firstViewport,
                                                           uint32_t viewportCount,
                                                           const VkShadingRatePaletteNV* pShadingRatePalettes);
    bool ValidateGeometryTrianglesNV(const VkGeometryTrianglesNV& triangles, VkDebugReportObjectTypeEXT object_type,
                                     uint64_t object_handle, const char* func_name) const;
    bool ValidateGeometryAABBNV(const VkGeometryAABBNV& geometry, VkDebugReportObjectTypeEXT object_type, uint64_t object_handle,
                                const char* func_name) const;
    bool ValidateGeometryNV(const VkGeometryNV& geometry, VkDebugReportObjectTypeEXT object_type, uint64_t object_handle,
                            const char* func_name) const;
    bool PreCallValidateCreateAccelerationStructureNV(VkDevice device, const VkAccelerationStructureCreateInfoNV* pCreateInfo,
                                                      const VkAllocationCallbacks* pAllocator,
                                                      VkAccelerationStructureNV* pAccelerationStructure);
    bool PreCallValidateBindAccelerationStructureMemoryNV(VkDevice device, uint32_t bindInfoCount,
                                                          const VkBindAccelerationStructureMemoryInfoNV* pBindInfos);
    bool PreCallValidateGetAccelerationStructureHandleNV(VkDevice device, VkAccelerationStructureNV accelerationStructure,
                                                         size_t dataSize, void* pData);
    bool PreCallValidateCmdBuildAccelerationStructureNV(VkCommandBuffer commandBuffer, const VkAccelerationStructureInfoNV* pInfo,
                                                        VkBuffer instanceData, VkDeviceSize instanceOffset, VkBool32 update,
                                                        VkAccelerationStructureNV dst, VkAccelerationStructureNV src,
                                                        VkBuffer scratch, VkDeviceSize scratchOffset);
    void PreCallRecordCmdBuildAccelerationStructureNV(VkCommandBuffer commandBuffer, const VkAccelerationStructureInfoNV* pInfo,
                                                      VkBuffer instanceData, VkDeviceSize instanceOffset, VkBool32 update,
                                                      VkAccelerationStructureNV dst, VkAccelerationStructureNV src,
                                                      VkBuffer scratch, VkDeviceSize scratchOffset);
    bool PreCallValidateCmdCopyAccelerationStructureNV(VkCommandBuffer commandBuffer, VkAccelerationStructureNV dst,
                                                       VkAccelerationStructureNV src, VkCopyAccelerationStructureModeNV mode);
    bool PreCallValidateDestroyAccelerationStructureNV(VkDevice device, VkAccelerationStructureNV accelerationStructure,
                                                       const VkAllocationCallbacks* pAllocator);
    bool PreCallValidateCmdSetLineWidth(VkCommandBuffer commandBuffer, float lineWidth);
    bool PreCallValidateCmdSetLineStippleEXT(VkCommandBuffer commandBuffer, uint32_t lineStippleFactor,
                                             uint16_t lineStipplePattern);
    bool PreCallValidateCmdSetDepthBias(VkCommandBuffer commandBuffer, float depthBiasConstantFactor, float depthBiasClamp,
                                        float depthBiasSlopeFactor);
    bool PreCallValidateCmdSetBlendConstants(VkCommandBuffer commandBuffer, const float blendConstants[4]);
    bool PreCallValidateCmdSetDepthBounds(VkCommandBuffer commandBuffer, float minDepthBounds, float maxDepthBounds);
    bool PreCallValidateCmdSetStencilCompareMask(VkCommandBuffer commandBuffer, VkStencilFaceFlags faceMask, uint32_t compareMask);
    bool PreCallValidateCmdSetStencilWriteMask(VkCommandBuffer commandBuffer, VkStencilFaceFlags faceMask, uint32_t writeMask);
    bool PreCallValidateCmdSetStencilReference(VkCommandBuffer commandBuffer, VkStencilFaceFlags faceMask, uint32_t reference);
    bool PreCallValidateCmdBindDescriptorSets(VkCommandBuffer commandBuffer, VkPipelineBindPoint pipelineBindPoint,
                                              VkPipelineLayout layout, uint32_t firstSet, uint32_t setCount,
                                              const VkDescriptorSet* pDescriptorSets, uint32_t dynamicOffsetCount,
                                              const uint32_t* pDynamicOffsets);
    bool PreCallValidateCmdPushDescriptorSetKHR(VkCommandBuffer commandBuffer, VkPipelineBindPoint pipelineBindPoint,
                                                VkPipelineLayout layout, uint32_t set, uint32_t descriptorWriteCount,
                                                const VkWriteDescriptorSet* pDescriptorWrites);
    bool PreCallValidateCmdBindIndexBuffer(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                           VkIndexType indexType);
    bool PreCallValidateCmdBindVertexBuffers(VkCommandBuffer commandBuffer, uint32_t firstBinding, uint32_t bindingCount,
                                             const VkBuffer* pBuffers, const VkDeviceSize* pOffsets);
    bool PreCallValidateCmdDraw(VkCommandBuffer commandBuffer, uint32_t vertexCount, uint32_t instanceCount, uint32_t firstVertex,
                                uint32_t firstInstance);
    void PreCallRecordCmdDraw(VkCommandBuffer commandBuffer, uint32_t vertexCount, uint32_t instanceCount, uint32_t firstVertex,
                              uint32_t firstInstance);
    bool PreCallValidateCmdDrawIndexed(VkCommandBuffer commandBuffer, uint32_t indexCount, uint32_t instanceCount,
                                       uint32_t firstIndex, int32_t vertexOffset, uint32_t firstInstance);
    void PreCallRecordCmdDrawIndexed(VkCommandBuffer commandBuffer, uint32_t indexCount, uint32_t instanceCount,
                                     uint32_t firstIndex, int32_t vertexOffset, uint32_t firstInstance);
    bool PreCallValidateCmdDrawIndexedIndirect(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset, uint32_t count,
                                               uint32_t stride);
    void PreCallRecordCmdDrawIndexedIndirect(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset, uint32_t count,
                                             uint32_t stride);
    bool PreCallValidateCmdDrawIndexedIndirectCountKHR(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                       VkBuffer countBuffer, VkDeviceSize countBufferOffset, uint32_t maxDrawCount,
                                                       uint32_t stride);
    bool PreCallValidateCmdDispatch(VkCommandBuffer commandBuffer, uint32_t x, uint32_t y, uint32_t z);
    void PreCallRecordCmdDispatch(VkCommandBuffer commandBuffer, uint32_t x, uint32_t y, uint32_t z);
    bool PreCallValidateCmdDispatchIndirect(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset);
    void PreCallRecordCmdDispatchIndirect(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset);
    bool PreCallValidateCmdDrawIndirect(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset, uint32_t count,
                                        uint32_t stride);
    void PreCallRecordCmdDrawIndirect(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset, uint32_t count,
                                      uint32_t stride);
    bool PreCallValidateCmdSetEvent(VkCommandBuffer commandBuffer, VkEvent event, VkPipelineStageFlags stageMask);
    bool PreCallValidateCmdResetEvent(VkCommandBuffer commandBuffer, VkEvent event, VkPipelineStageFlags stageMask);
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

    void EnqueueVerifyBeginQuery(VkCommandBuffer, const QueryObject& query_obj);
    bool PreCallValidateCmdBeginQuery(VkCommandBuffer commandBuffer, VkQueryPool queryPool, uint32_t slot, VkFlags flags);
    void PreCallRecordCmdBeginQuery(VkCommandBuffer commandBuffer, VkQueryPool queryPool, uint32_t slot, VkFlags flags);
    bool PreCallValidateCmdEndQuery(VkCommandBuffer commandBuffer, VkQueryPool queryPool, uint32_t slot);
    bool PreCallValidateCmdResetQueryPool(VkCommandBuffer commandBuffer, VkQueryPool queryPool, uint32_t firstQuery,
                                          uint32_t queryCount);
    bool PreCallValidateCmdCopyQueryPoolResults(VkCommandBuffer commandBuffer, VkQueryPool queryPool, uint32_t firstQuery,
                                                uint32_t queryCount, VkBuffer dstBuffer, VkDeviceSize dstOffset,
                                                VkDeviceSize stride, VkQueryResultFlags flags);
    void PreCallRecordCmdCopyQueryPoolResults(VkCommandBuffer commandBuffer, VkQueryPool queryPool, uint32_t firstQuery,
                                              uint32_t queryCount, VkBuffer dstBuffer, VkDeviceSize dstOffset, VkDeviceSize stride,
                                              VkQueryResultFlags flags);
    bool PreCallValidateCmdPushConstants(VkCommandBuffer commandBuffer, VkPipelineLayout layout, VkShaderStageFlags stageFlags,
                                         uint32_t offset, uint32_t size, const void* pValues);
    bool PreCallValidateCmdWriteTimestamp(VkCommandBuffer commandBuffer, VkPipelineStageFlagBits pipelineStage,
                                          VkQueryPool queryPool, uint32_t slot);
    void PreCallRecordCmdWriteTimestamp(VkCommandBuffer commandBuffer, VkPipelineStageFlagBits pipelineStage, VkQueryPool queryPool,
                                        uint32_t slot);
    bool PreCallValidateCreateFramebuffer(VkDevice device, const VkFramebufferCreateInfo* pCreateInfo,
                                          const VkAllocationCallbacks* pAllocator, VkFramebuffer* pFramebuffer);
    bool PreCallValidateCreateRenderPass(VkDevice device, const VkRenderPassCreateInfo* pCreateInfo,
                                         const VkAllocationCallbacks* pAllocator, VkRenderPass* pRenderPass);
    bool PreCallValidateGetDeviceMemoryCommitment(VkDevice device, VkDeviceMemory mem, VkDeviceSize* pCommittedMem);
    bool PreCallValidateCreateRenderPass2KHR(VkDevice device, const VkRenderPassCreateInfo2KHR* pCreateInfo,
                                             const VkAllocationCallbacks* pAllocator, VkRenderPass* pRenderPass);
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
    bool PreCallValidateBindImageMemory2(VkDevice device, uint32_t bindInfoCount, const VkBindImageMemoryInfoKHR* pBindInfos);
    bool PreCallValidateBindImageMemory2KHR(VkDevice device, uint32_t bindInfoCount, const VkBindImageMemoryInfoKHR* pBindInfos);
    bool PreCallValidateSetEvent(VkDevice device, VkEvent event);
    bool PreCallValidateQueueBindSparse(VkQueue queue, uint32_t bindInfoCount, const VkBindSparseInfo* pBindInfo, VkFence fence);
    bool PreCallValidateImportSemaphoreFdKHR(VkDevice device, const VkImportSemaphoreFdInfoKHR* pImportSemaphoreFdInfo);
#ifdef VK_USE_PLATFORM_WIN32_KHR
    bool PreCallValidateImportSemaphoreWin32HandleKHR(VkDevice device,
                                                      const VkImportSemaphoreWin32HandleInfoKHR* pImportSemaphoreWin32HandleInfo);
    bool PreCallValidateImportFenceWin32HandleKHR(VkDevice device,
                                                  const VkImportFenceWin32HandleInfoKHR* pImportFenceWin32HandleInfo);
#endif  // VK_USE_PLATFORM_WIN32_KHR
    bool PreCallValidateImportFenceFdKHR(VkDevice device, const VkImportFenceFdInfoKHR* pImportFenceFdInfo);

    bool PreCallValidateCreateSwapchainKHR(VkDevice device, const VkSwapchainCreateInfoKHR* pCreateInfo,
                                           const VkAllocationCallbacks* pAllocator, VkSwapchainKHR* pSwapchain);
    void PreCallRecordDestroySwapchainKHR(VkDevice device, VkSwapchainKHR swapchain, const VkAllocationCallbacks* pAllocator);
    bool PreCallValidateGetSwapchainImagesKHR(VkDevice device, VkSwapchainKHR swapchain, uint32_t* pSwapchainImageCount,
                                              VkImage* pSwapchainImages);
    void PostCallRecordGetSwapchainImagesKHR(VkDevice device, VkSwapchainKHR swapchain, uint32_t* pSwapchainImageCount,
                                             VkImage* pSwapchainImages, VkResult result);
    bool PreCallValidateQueuePresentKHR(VkQueue queue, const VkPresentInfoKHR* pPresentInfo);
    bool PreCallValidateCreateSharedSwapchainsKHR(VkDevice device, uint32_t swapchainCount,
                                                  const VkSwapchainCreateInfoKHR* pCreateInfos,
                                                  const VkAllocationCallbacks* pAllocator, VkSwapchainKHR* pSwapchains);
    bool PreCallValidateAcquireNextImageKHR(VkDevice device, VkSwapchainKHR swapchain, uint64_t timeout, VkSemaphore semaphore,
                                            VkFence fence, uint32_t* pImageIndex);
    bool PreCallValidateAcquireNextImage2KHR(VkDevice device, const VkAcquireNextImageInfoKHR* pAcquireInfo, uint32_t* pImageIndex);
    bool PreCallValidateDestroySurfaceKHR(VkInstance instance, VkSurfaceKHR surface, const VkAllocationCallbacks* pAllocator);
    bool PreCallValidateGetPhysicalDeviceSurfaceSupportKHR(VkPhysicalDevice physicalDevice, uint32_t queueFamilyIndex,
                                                           VkSurfaceKHR surface, VkBool32* pSupported);
    bool PreCallValidateCreateDescriptorUpdateTemplate(VkDevice device, const VkDescriptorUpdateTemplateCreateInfoKHR* pCreateInfo,
                                                       const VkAllocationCallbacks* pAllocator,
                                                       VkDescriptorUpdateTemplateKHR* pDescriptorUpdateTemplate);
    bool PreCallValidateCreateDescriptorUpdateTemplateKHR(VkDevice device,
                                                          const VkDescriptorUpdateTemplateCreateInfoKHR* pCreateInfo,
                                                          const VkAllocationCallbacks* pAllocator,
                                                          VkDescriptorUpdateTemplateKHR* pDescriptorUpdateTemplate);
    bool PreCallValidateUpdateDescriptorSetWithTemplate(VkDevice device, VkDescriptorSet descriptorSet,
                                                        VkDescriptorUpdateTemplate descriptorUpdateTemplate, const void* pData);
    bool PreCallValidateUpdateDescriptorSetWithTemplateKHR(VkDevice device, VkDescriptorSet descriptorSet,
                                                           VkDescriptorUpdateTemplateKHR descriptorUpdateTemplate,
                                                           const void* pData);

    bool PreCallValidateCmdPushDescriptorSetWithTemplateKHR(VkCommandBuffer commandBuffer,
                                                            VkDescriptorUpdateTemplateKHR descriptorUpdateTemplate,
                                                            VkPipelineLayout layout, uint32_t set, const void* pData);
    bool PreCallValidateGetDisplayPlaneSupportedDisplaysKHR(VkPhysicalDevice physicalDevice, uint32_t planeIndex,
                                                            uint32_t* pDisplayCount, VkDisplayKHR* pDisplays);
    bool PreCallValidateGetDisplayPlaneCapabilitiesKHR(VkPhysicalDevice physicalDevice, VkDisplayModeKHR mode, uint32_t planeIndex,
                                                       VkDisplayPlaneCapabilitiesKHR* pCapabilities);
    bool PreCallValidateGetDisplayPlaneCapabilities2KHR(VkPhysicalDevice physicalDevice,
                                                        const VkDisplayPlaneInfo2KHR* pDisplayPlaneInfo,
                                                        VkDisplayPlaneCapabilities2KHR* pCapabilities);
    bool PreCallValidateCmdDebugMarkerEndEXT(VkCommandBuffer commandBuffer);

    bool PreCallValidateCmdBeginQueryIndexedEXT(VkCommandBuffer commandBuffer, VkQueryPool queryPool, uint32_t query,
                                                VkQueryControlFlags flags, uint32_t index);
    void PreCallRecordCmdBeginQueryIndexedEXT(VkCommandBuffer commandBuffer, VkQueryPool queryPool, uint32_t query,
                                              VkQueryControlFlags flags, uint32_t index);
    bool PreCallValidateCmdEndQueryIndexedEXT(VkCommandBuffer commandBuffer, VkQueryPool queryPool, uint32_t query, uint32_t index);

    bool PreCallValidateCmdSetDiscardRectangleEXT(VkCommandBuffer commandBuffer, uint32_t firstDiscardRectangle,
                                                  uint32_t discardRectangleCount, const VkRect2D* pDiscardRectangles);
    bool PreCallValidateCmdSetSampleLocationsEXT(VkCommandBuffer commandBuffer,
                                                 const VkSampleLocationsInfoEXT* pSampleLocationsInfo);
    bool PreCallValidateCmdDrawIndirectCountKHR(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                VkBuffer countBuffer, VkDeviceSize countBufferOffset, uint32_t maxDrawCount,
                                                uint32_t stride);
    bool PreCallValidateCmdDrawMeshTasksNV(VkCommandBuffer commandBuffer, uint32_t taskCount, uint32_t firstTask);
    bool PreCallValidateCmdDrawMeshTasksIndirectNV(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                   uint32_t drawCount, uint32_t stride);
    bool PreCallValidateCmdDrawMeshTasksIndirectCountNV(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                        VkBuffer countBuffer, VkDeviceSize countBufferOffset, uint32_t maxDrawCount,
                                                        uint32_t stride);
    void PreCallRecordGetPhysicalDeviceProperties(VkPhysicalDevice physicalDevice,
                                                  VkPhysicalDeviceProperties* pPhysicalDeviceProperties);
    bool PreCallValidateGetBufferDeviceAddressEXT(VkDevice device, const VkBufferDeviceAddressInfoEXT* pInfo);
    void PostCallRecordGetBufferDeviceAddressEXT(VkDevice device, const VkBufferDeviceAddressInfoEXT* pInfo,
                                                 VkDeviceAddress address);
    bool ValidateCmdSetDeviceMask(VkCommandBuffer commandBuffer, uint32_t deviceMask, const char* func_name) const;
    bool PreCallValidateCmdSetDeviceMask(VkCommandBuffer commandBuffer, uint32_t deviceMask);
    bool PreCallValidateCmdSetDeviceMaskKHR(VkCommandBuffer commandBuffer, uint32_t deviceMask);
    bool ValidateComputeWorkGroupSizes(const SHADER_MODULE_STATE* shader) const;

    bool ValidateQueryRange(VkDevice device, VkQueryPool queryPool, uint32_t totalCount, uint32_t firstQuery, uint32_t queryCount,
                            const char* vuid_badfirst, const char* vuid_badrange) const;
    bool PreCallValidateResetQueryPoolEXT(VkDevice device, VkQueryPool queryPool, uint32_t firstQuery, uint32_t queryCount);

    bool ValidateComputeWorkGroupInvocations(CMD_BUFFER_STATE* cb_state, uint32_t groupCountX, uint32_t groupCountY,
                                             uint32_t groupCountZ);
    bool ValidateQueryPoolStride(const std::string& vuid_not_64, const std::string& vuid_64, const VkDeviceSize stride,
                                 const char* parameter_name, const uint64_t parameter_value, const VkQueryResultFlags flags) const;
    bool ValidateCmdDrawStrideWithStruct(VkCommandBuffer commandBuffer, const std::string& vuid, const uint32_t stride,
                                         const char* struct_name, const uint32_t struct_size) const;
    bool ValidateCmdDrawStrideWithBuffer(VkCommandBuffer commandBuffer, const std::string& vuid, const uint32_t stride,
                                         const char* struct_name, const uint32_t struct_size, const uint32_t drawCount,
                                         const VkDeviceSize offset, const BUFFER_STATE* buffer_state) const;

#ifdef VK_USE_PLATFORM_ANDROID_KHR
    bool PreCallValidateGetAndroidHardwareBufferPropertiesANDROID(VkDevice device, const struct AHardwareBuffer* buffer,
                                                                  VkAndroidHardwareBufferPropertiesANDROID* pProperties);
    void PostCallRecordGetAndroidHardwareBufferPropertiesANDROID(VkDevice device, const struct AHardwareBuffer* buffer,
                                                                 VkAndroidHardwareBufferPropertiesANDROID* pProperties,
                                                                 VkResult result);
    bool PreCallValidateGetMemoryAndroidHardwareBufferANDROID(VkDevice device,
                                                              const VkMemoryGetAndroidHardwareBufferInfoANDROID* pInfo,
                                                              struct AHardwareBuffer** pBuffer);
#endif  // VK_USE_PLATFORM_ANDROID_KHR
#ifdef VK_USE_PLATFORM_WAYLAND_KHR
    bool PreCallValidateGetPhysicalDeviceWaylandPresentationSupportKHR(VkPhysicalDevice physicalDevice, uint32_t queueFamilyIndex,
                                                                       struct wl_display* display);
#endif  // VK_USE_PLATFORM_WAYLAND_KHR
#ifdef VK_USE_PLATFORM_WIN32_KHR
    bool PreCallValidateGetPhysicalDeviceWin32PresentationSupportKHR(VkPhysicalDevice physicalDevice, uint32_t queueFamilyIndex);
#endif  // VK_USE_PLATFORM_WIN32_KHR
#ifdef VK_USE_PLATFORM_XCB_KHR
    bool PreCallValidateGetPhysicalDeviceXcbPresentationSupportKHR(VkPhysicalDevice physicalDevice, uint32_t queueFamilyIndex,
                                                                   xcb_connection_t* connection, xcb_visualid_t visual_id);
#endif  // VK_USE_PLATFORM_XCB_KHR
#ifdef VK_USE_PLATFORM_XLIB_KHR
    bool PreCallValidateGetPhysicalDeviceXlibPresentationSupportKHR(VkPhysicalDevice physicalDevice, uint32_t queueFamilyIndex,
                                                                    Display* dpy, VisualID visualID);
#endif  // VK_USE_PLATFORM_XLIB_KHR

};  // Class CoreChecks
