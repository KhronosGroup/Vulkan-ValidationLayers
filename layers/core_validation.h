/* Copyright (c) 2015-2020 The Khronos Group Inc.
 * Copyright (c) 2015-2020 Valve Corporation
 * Copyright (c) 2015-2020 LunarG, Inc.
 * Copyright (C) 2015-2020 Google Inc.
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
 * Author: Jeremy Kniager <jeremyk@lunarg.com>
 */

#pragma once

#include "state_tracker.h"
#include "image_layout_map.h"
#include "gpu_validation.h"
#include "shader_validation.h"

// Set of VUID that need to go between core_validation.cpp and drawdispatch.cpp
struct DrawDispatchVuid {
    const char* queue_flag;
    const char* inside_renderpass;
    const char* pipeline_bound;
    const char* dynamic_state;
    const char* vertex_binding;
    const char* vertex_binding_null;
    const char* compatible_pipeline;
    const char* render_pass_compatible;
    const char* subpass_index;
    const char* sample_location;
    const char* linear_sampler;
    const char* cubic_sampler;
    const char* indirect_protected_cb;
    const char* indirect_contiguous_memory;
    const char* indirect_buffer_bit;
    const char* viewport_count;
    const char* scissor_count;
    const char* viewport_scissor_count;
    const char* primitive_topology;
    const char* corner_sampled_address_mode;
    const char* subpass_input;
    const char* imageview_atomic;
    const char* image_subresources;
};

typedef struct {
    const char* vuid_queue_flags;
    const char* vuid_queue_feedback;
    const char* vuid_queue_occlusion;
    const char* vuid_precise;
    const char* vuid_query_count;
    const char* vuid_profile_lock;
    const char* vuid_scope_not_first;
    const char* vuid_scope_in_rp;
    const char* vuid_dup_query_type;
    const char* vuid_protected_cb;
} ValidateBeginQueryVuids;

typedef struct {
    const char* vuid_queue_flags;
    const char* vuid_active_queries;
    const char* vuid_protected_cb;
} ValidateEndQueryVuids;

class CoreChecks : public ValidationStateTracker {
  public:
    using StateTracker = ValidationStateTracker;
    GlobalQFOTransferBarrierMap<VkImageMemoryBarrier> qfo_release_image_barrier_map;
    GlobalQFOTransferBarrierMap<VkBufferMemoryBarrier> qfo_release_buffer_barrier_map;
    GlobalImageLayoutMap imageLayoutMap;

    CoreChecks() { container_type = LayerObjectTypeCoreValidation; }

    // Override base class, we have some extra work to do here
    void InitDeviceValidationObject(bool add_obj, ValidationObject* inst_obj, ValidationObject* dev_obj);

    void IncrementCommandCount(VkCommandBuffer commandBuffer);

    bool VerifyQueueStateToSeq(const QUEUE_STATE* initial_queue, uint64_t initial_seq) const;
    bool ValidateSetMemBinding(VkDeviceMemory mem, const VulkanTypedHandle& typed_handle, const char* apiName) const;
    bool ValidateDeviceQueueFamily(uint32_t queue_family, const char* cmd_name, const char* parameter_name, const char* error_code,
                                   bool optional) const;
    bool ValidateBindBufferMemory(VkBuffer buffer, VkDeviceMemory mem, VkDeviceSize memoryOffset, const char* api_name) const;
    bool ValidateGetImageMemoryRequirements2(const VkImageMemoryRequirementsInfo2* pInfo, const char* func_name) const;
    bool CheckCommandBuffersInFlight(const COMMAND_POOL_STATE* pPool, const char* action, const char* error_code) const;
    bool CheckCommandBufferInFlight(const CMD_BUFFER_STATE* cb_node, const char* action, const char* error_code) const;
    bool VerifyQueueStateToFence(VkFence fence) const;
    void StoreMemRanges(VkDeviceMemory mem, VkDeviceSize offset, VkDeviceSize size);
    bool ValidateIdleDescriptorSet(VkDescriptorSet set, const char* func_str) const;
    bool SemaphoreWasSignaled(VkSemaphore semaphore) const;
    bool ValidatePipelineLocked(std::vector<std::shared_ptr<PIPELINE_STATE>> const& pPipelines, int pipelineIndex) const;
    bool ValidatePipelineUnlocked(const PIPELINE_STATE* pPipeline, uint32_t pipelineIndex) const;
    bool ValidImageBufferQueue(const CMD_BUFFER_STATE* cb_node, const VulkanTypedHandle& object, uint32_t queueFamilyIndex,
                               uint32_t count, const uint32_t* indices) const;
    bool ValidateFenceForSubmit(const FENCE_STATE* pFence, const char* inflight_vuid, const char* retired_vuid,
                                const char* func_name) const;
    bool ValidateSemaphoresForSubmit(VkQueue queue, const VkSubmitInfo* submit, uint32_t submit_index,
                                     std::unordered_set<VkSemaphore>* unsignaled_sema_arg,
                                     std::unordered_set<VkSemaphore>* signaled_sema_arg,
                                     std::unordered_set<VkSemaphore>* internal_sema_arg) const;
    bool ValidateMaxTimelineSemaphoreValueDifference(VkSemaphore semaphore, uint64_t value, const char* func_name,
                                                     const char* vuid) const;
    bool ValidateCommandBuffersForSubmit(VkQueue queue, const VkSubmitInfo* submit, GlobalImageLayoutMap* localImageLayoutMap_arg,
                                         QueryMap* local_query_to_state_map, std::vector<VkCommandBuffer>* current_cmds_arg) const;
    bool ValidateStatus(const CMD_BUFFER_STATE* pNode, CBStatusFlags status_mask, const char* fail_msg, const char* msg_code) const;
    bool ValidateDrawStateFlags(const CMD_BUFFER_STATE* pCB, const PIPELINE_STATE* pPipe, bool indexed, const char* msg_code) const;
    bool LogInvalidAttachmentMessage(const char* type1_string, const RENDER_PASS_STATE* rp1_state, const char* type2_string,
                                     const RENDER_PASS_STATE* rp2_state, uint32_t primary_attach, uint32_t secondary_attach,
                                     const char* msg, const char* caller, const char* error_code) const;
    bool LogInvalidPnextMessage(const char* type1_string, const RENDER_PASS_STATE* rp1_state, const char* type2_string,
                                const RENDER_PASS_STATE* rp2_state, const char* msg, const char* caller,
                                const char* error_code) const;
    bool ValidateStageMaskGsTsEnables(VkPipelineStageFlags stageMask, const char* caller, const char* geo_error_id,
                                      const char* tess_error_id, const char* mesh_error_id, const char* task_error_id) const;
    bool ValidateStageMaskHost(VkPipelineStageFlags stageMask, const char* caller, const char* vuid) const;
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
    bool ValidateQueueFamilyIndex(const PHYSICAL_DEVICE_STATE* pd_state, uint32_t requested_queue_family, const char* err_code,
                                  const char* cmd_name, const char* queue_family_var_name) const;
    bool ValidateDeviceQueueCreateInfos(const PHYSICAL_DEVICE_STATE* pd_state, uint32_t info_count,
                                        const VkDeviceQueueCreateInfo* infos) const;

    bool ValidateProtectedImage(const CMD_BUFFER_STATE* cb_state, const IMAGE_STATE* image_state, const char* cmd_name,
                                const char* vuid) const;
    bool ValidateUnprotectedImage(const CMD_BUFFER_STATE* cb_state, const IMAGE_STATE* image_state, const char* cmd_name,
                                  const char* vuid) const;
    bool ValidateProtectedBuffer(const CMD_BUFFER_STATE* cb_state, const BUFFER_STATE* buffer_state, const char* cmd_name,
                                 const char* vuid) const;
    bool ValidateUnprotectedBuffer(const CMD_BUFFER_STATE* cb_state, const BUFFER_STATE* buffer_state, const char* cmd_name,
                                   const char* vuid) const;

    bool ValidatePipelineVertexDivisors(std::vector<std::shared_ptr<PIPELINE_STATE>> const& pipe_state_vec, const uint32_t count,
                                        const VkGraphicsPipelineCreateInfo* pipe_cis) const;
    bool ValidatePipelineCacheControlFlags(VkPipelineCreateFlags flags, uint32_t index, const char* caller_name,
                                           const char* vuid) const;
    void EnqueueSubmitTimeValidateImageBarrierAttachment(const char* func_name, CMD_BUFFER_STATE* cb_state,
                                                         uint32_t imageMemBarrierCount,
                                                         const VkImageMemoryBarrier* pImageMemBarriers);
    bool ValidateImageBarrierAttachment(const char* funcName, CMD_BUFFER_STATE const* cb_state,
                                        const FRAMEBUFFER_STATE* framebuffer, uint32_t active_subpass,
                                        const safe_VkSubpassDescription2& sub_desc, const VkRenderPass rp_handle,
                                        uint32_t img_index, const VkImageMemoryBarrier& img_barrier) const;
    static bool ValidateConcurrentBarrierAtSubmit(const ValidationStateTracker* state_data, const QUEUE_STATE* queue_data,
                                                  const char* func_name, const CMD_BUFFER_STATE* cb_state,
                                                  const VulkanTypedHandle& typed_handle, uint32_t src_queue_family,
                                                  uint32_t dst_queue_family);
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
    bool ValidateGraphicsPipelineBindPoint(const CMD_BUFFER_STATE* cb_state, const PIPELINE_STATE* pipeline_state) const;
    bool ValidatePipelineBindPoint(const CMD_BUFFER_STATE* cb_state, VkPipelineBindPoint bind_point, const char* func_name,
                                   const std::map<VkPipelineBindPoint, std::string>& bind_errors) const;
    bool ValidateMemoryIsMapped(const char* funcName, uint32_t memRangeCount, const VkMappedMemoryRange* pMemRanges) const;
    bool ValidateMappedMemoryRangeDeviceLimits(const char* func_name, uint32_t mem_range_count,
                                               const VkMappedMemoryRange* mem_ranges) const;
    BarrierOperationsType ComputeBarrierOperationsType(const CMD_BUFFER_STATE* cb_state, uint32_t buffer_barrier_count,
                                                       const VkBufferMemoryBarrier* buffer_barriers, uint32_t image_barrier_count,
                                                       const VkImageMemoryBarrier* image_barriers) const;
    bool ValidateStageMasksAgainstQueueCapabilities(const CMD_BUFFER_STATE* cb_state, VkPipelineStageFlags source_stage_mask,
                                                    VkPipelineStageFlags dest_stage_mask, BarrierOperationsType barrier_op_type,
                                                    const char* function, const char* error_code) const;
    bool ValidateRenderPassImageBarriers(const char* funcName, const CMD_BUFFER_STATE* cb_state, uint32_t active_subpass,
                                         const safe_VkSubpassDescription2& sub_desc, const VkRenderPass rp_handle,
                                         const safe_VkSubpassDependency2* dependencies,
                                         const std::vector<uint32_t>& self_dependencies, uint32_t image_mem_barrier_count,
                                         const VkImageMemoryBarrier* image_barriers) const;
    bool ValidateSecondaryCommandBufferState(const CMD_BUFFER_STATE* pCB, const CMD_BUFFER_STATE* pSubCB) const;
    bool ValidateFramebuffer(VkCommandBuffer primaryBuffer, const CMD_BUFFER_STATE* pCB, VkCommandBuffer secondaryBuffer,
                             const CMD_BUFFER_STATE* pSubCB, const char* caller) const;
    bool ValidateDescriptorUpdateTemplate(const char* func_name, const VkDescriptorUpdateTemplateCreateInfoKHR* pCreateInfo) const;
    bool ValidateCreateSamplerYcbcrConversion(const char* func_name, const VkSamplerYcbcrConversionCreateInfo* create_info) const;
    bool ValidateImportFence(VkFence fence, const char* vuid, const char* caller_name) const;
    bool ValidateAcquireNextImage(VkDevice device, CommandVersion cmd_version, VkSwapchainKHR swapchain, uint64_t timeout,
                                  VkSemaphore semaphore, VkFence fence, uint32_t* pImageIndex, const char* func_name,
                                  const char* semaphore_type_vuid) const;
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
    bool CheckDependencyExists(const VkRenderPass renderpass, const uint32_t subpass, const VkImageLayout layout,
                               const std::vector<SubpassLayout>& dependent_subpasses, const std::vector<DAGNode>& subpass_to_node,
                               bool& skip) const;
    bool CheckPreserved(const VkRenderPass renderpass, const VkRenderPassCreateInfo2KHR* pCreateInfo, const int index,
                        const uint32_t attachment, const std::vector<DAGNode>& subpass_to_node, int depth, bool& skip) const;
    bool ValidateBindImageMemory(uint32_t bindInfoCount, const VkBindImageMemoryInfo* pBindInfos, const char* api_name) const;
    bool ValidateGetPhysicalDeviceDisplayPlanePropertiesKHRQuery(VkPhysicalDevice physicalDevice, uint32_t planeIndex,
                                                                 const char* api_name) const;
    static bool ValidateCopyQueryPoolResults(const ValidationStateTracker* state_data, VkCommandBuffer commandBuffer,
                                             VkQueryPool queryPool, uint32_t firstQuery, uint32_t queryCount, uint32_t perfPass,
                                             VkQueryResultFlags flags, QueryMap* localQueryToStateMap);
    static bool VerifyQueryIsReset(const ValidationStateTracker* state_data, VkCommandBuffer commandBuffer, QueryObject query_obj,
                                   const char* func_name, VkQueryPool& firstPerfQueryPool, uint32_t perfPass,
                                   QueryMap* localQueryToStateMap);
    static bool ValidatePerformanceQuery(const ValidationStateTracker* state_data, VkCommandBuffer commandBuffer,
                                         QueryObject query_obj, const char* func_name, VkQueryPool& firstPerfQueryPool,
                                         uint32_t perfPass, QueryMap* localQueryToStateMap);
    bool ValidateImportSemaphore(VkSemaphore semaphore, const char* caller_name) const;
    bool ValidateBeginQuery(const CMD_BUFFER_STATE* cb_state, const QueryObject& query_obj, VkFlags flags, CMD_TYPE cmd,
                            const char* cmd_name, const ValidateBeginQueryVuids* vuids) const;
    bool ValidateCmdEndQuery(const CMD_BUFFER_STATE* cb_state, const QueryObject& query_obj, CMD_TYPE cmd, const char* cmd_name,
                             const ValidateEndQueryVuids* vuids) const;
    bool ValidateCmdResetQuery(const CMD_BUFFER_STATE* cb_state, VkQueryPool queryPool, uint32_t firstQuery,
                               uint32_t queryCount) const;

    const DrawDispatchVuid& GetDrawDispatchVuid(CMD_TYPE cmd_type) const;
    bool ValidateCmdDrawType(VkCommandBuffer cmd_buffer, bool indexed, VkPipelineBindPoint bind_point, CMD_TYPE cmd_type,
                             const char* caller, VkQueueFlags queue_flags) const;
    bool ValidateCmdNextSubpass(RenderPassCreateVersion rp_version, VkCommandBuffer commandBuffer) const;
    bool ValidateInsertMemoryRange(const VulkanTypedHandle& typed_handle, const DEVICE_MEMORY_STATE* mem_info,
                                   VkDeviceSize memoryOffset, const char* api_name) const;
    bool ValidateInsertImageMemoryRange(VkImage image, const DEVICE_MEMORY_STATE* mem_info, VkDeviceSize mem_offset,
                                        const char* api_name) const;
    bool ValidateInsertBufferMemoryRange(VkBuffer buffer, const DEVICE_MEMORY_STATE* mem_info, VkDeviceSize mem_offset,
                                         const char* api_name) const;
    bool ValidateInsertAccelerationStructureMemoryRange(VkAccelerationStructureNV as, const DEVICE_MEMORY_STATE* mem_info,
                                                        VkDeviceSize mem_offset, const char* api_name) const;

    bool ValidateMemoryTypes(const DEVICE_MEMORY_STATE* mem_info, const uint32_t memory_type_bits, const char* funcName,
                             const char* msgCode) const;
    bool ValidateCommandBufferState(const CMD_BUFFER_STATE* cb_state, const char* call_source, int current_submit_count,
                                    const char* vu_id) const;
    bool ValidateCommandBufferSimultaneousUse(const CMD_BUFFER_STATE* pCB, int current_submit_count) const;
    bool ValidateAttachmentReference(RenderPassCreateVersion rp_version, VkAttachmentReference2 reference, const char* error_type,
                                     const char* function_name) const;
    bool ValidateRenderpassAttachmentUsage(RenderPassCreateVersion rp_version, const VkRenderPassCreateInfo2KHR* pCreateInfo,
                                           const char* function_name) const;
    bool AddAttachmentUse(RenderPassCreateVersion rp_version, uint32_t subpass, std::vector<uint8_t>& attachment_uses,
                          std::vector<VkImageLayout>& attachment_layouts, uint32_t attachment, uint8_t new_use,
                          VkImageLayout new_layout) const;
    bool ValidateAttachmentIndex(RenderPassCreateVersion rp_version, uint32_t attachment, uint32_t attachment_count,
                                 const char* error_type, const char* function_name) const;
    bool ValidateCreateRenderPass(VkDevice device, RenderPassCreateVersion rp_version,
                                  const VkRenderPassCreateInfo2KHR* pCreateInfo, const char* function_name) const;
    bool ValidateRenderPassPipelineBarriers(const char* funcName, const CMD_BUFFER_STATE* cb_state,
                                            VkPipelineStageFlags src_stage_mask, VkPipelineStageFlags dst_stage_mask,
                                            VkDependencyFlags dependency_flags, uint32_t mem_barrier_count,
                                            const VkMemoryBarrier* mem_barriers, uint32_t buffer_mem_barrier_count,
                                            const VkBufferMemoryBarrier* buffer_mem_barriers, uint32_t image_mem_barrier_count,
                                            const VkImageMemoryBarrier* image_barriers) const;
    bool CheckStageMaskQueueCompatibility(VkCommandBuffer command_buffer, VkPipelineStageFlags stage_mask, VkQueueFlags queue_flags,
                                          const char* function, const char* src_or_dest, const char* error_code) const;
    bool ValidateUpdateDescriptorSetWithTemplate(VkDescriptorSet descriptorSet,
                                                 VkDescriptorUpdateTemplateKHR descriptorUpdateTemplate, const void* pData) const;
    bool ValidateMemoryIsBoundToBuffer(const BUFFER_STATE*, const char*, const char*) const;
    bool ValidateMemoryIsBoundToImage(const IMAGE_STATE*, const char*, const char*) const;
    bool ValidateMemoryIsBoundToAccelerationStructure(const ACCELERATION_STRUCTURE_STATE*, const char*, const char*) const;
    bool ValidateObjectNotInUse(const BASE_NODE* obj_node, const VulkanTypedHandle& obj_struct, const char* caller_name,
                                const char* error_code) const;
    bool ValidateCmdQueueFlags(const CMD_BUFFER_STATE* cb_node, const char* caller_name, VkQueueFlags flags,
                               const char* error_code) const;
    bool ValidateSampleLocationsInfo(const VkSampleLocationsInfoEXT* pSampleLocationsInfo, const char* apiName) const;
    bool InsideRenderPass(const CMD_BUFFER_STATE* pCB, const char* apiName, const char* msgCode) const;
    bool OutsideRenderPass(const CMD_BUFFER_STATE* pCB, const char* apiName, const char* msgCode) const;
    bool ValidateDynamicOffsetAlignment(const VkCommandBuffer command_buffer, const VkDescriptorSetLayoutBinding* binding,
                                        VkDescriptorType test_type, VkDeviceSize alignment, const uint32_t* pDynamicOffsets,
                                        const char* err_msg, const char* limit_name, uint32_t* offset_idx) const;

    bool ValidateImageSampleCount(const IMAGE_STATE* image_state, VkSampleCountFlagBits sample_count, const char* location,
                                  const std::string& msgCode) const;
    bool ValidateCmdSubpassState(const CMD_BUFFER_STATE* pCB, const CMD_TYPE cmd_type) const;
    bool ValidateCmd(const CMD_BUFFER_STATE* cb_state, const CMD_TYPE cmd, const char* caller_name) const;
    bool ValidateIndirectCmd(VkCommandBuffer command_buffer, VkBuffer buffer, CMD_TYPE cmd_type, const char* caller_name) const;

    template <typename T1>
    bool ValidateDeviceMaskToPhysicalDeviceCount(uint32_t deviceMask, const T1 object, const char* VUID) const;
    template <typename T1>
    bool ValidateDeviceMaskToZero(uint32_t deviceMask, const T1 object, const char* VUID) const;
    template <typename T1>
    bool ValidateDeviceMaskToCommandBuffer(const CMD_BUFFER_STATE* pCB, uint32_t deviceMask, const T1 object,
                                           const char* VUID) const;
    bool ValidateDeviceMaskToRenderPass(const CMD_BUFFER_STATE* pCB, uint32_t deviceMask, const char* VUID) const;

    bool ValidateDepthStencilResolve(const VkPhysicalDeviceVulkan12Properties& core12_props,
                                     const VkRenderPassCreateInfo2* pCreateInfo, const char* function_name) const;

    bool ValidateBindAccelerationStructureMemory(VkDevice device, const VkBindAccelerationStructureMemoryInfoKHR& info) const;
    // Prototypes for CoreChecks accessor functions
    VkFormatProperties GetPDFormatProperties(const VkFormat format) const;
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
                                 const VkPipelineBindPoint bind_point, const char* function) const;
    static bool ValidateEventStageMask(const ValidationStateTracker* state_data, const CMD_BUFFER_STATE* pCB, size_t eventCount,
                                       size_t firstEventIndex, VkPipelineStageFlags sourceStageMask,
                                       EventToStageMap* localEventToStageMap);
    bool ValidateQueueFamilyIndices(const CMD_BUFFER_STATE* pCB, VkQueue queue) const;
    bool ValidatePerformanceQueries(const CMD_BUFFER_STATE* pCB, VkQueue queue, VkQueryPool& first_query_pool,
                                    uint32_t counterPassIndex) const;
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
                           const std::vector<uint32_t>& dynamic_offsets, const CMD_BUFFER_STATE* cb_node,
                           const std::vector<VkImageView>& attachment_views, const char* caller,
                           const DrawDispatchVuid& vuids) const;
    bool ValidateDescriptorSetBindingData(const CMD_BUFFER_STATE* cb_node, const cvdescriptorset::DescriptorSet* descriptor_set,
                                          const std::vector<uint32_t>& dynamic_offsets, uint32_t binding, descriptor_req reqs,
                                          VkFramebuffer framebuffer, const std::vector<VkImageView>& attachment_views,
                                          const char* caller, const DrawDispatchVuid& vuids) const;

    // Validate contents of a CopyUpdate
    using DescriptorSet = cvdescriptorset::DescriptorSet;
    bool ValidateCopyUpdate(const VkCopyDescriptorSet* update, const DescriptorSet* dst_set, const DescriptorSet* src_set,
                            const char* func_name, std::string* error_code, std::string* error_msg) const;
    bool VerifyCopyUpdateContents(const VkCopyDescriptorSet* update, const DescriptorSet* src_set, VkDescriptorType src_type,
                                  uint32_t src_index, const DescriptorSet* dst_set, VkDescriptorType dst_type, uint32_t dst_index,
                                  const char* func_name, std::string* error_code, std::string* error_msg) const;
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
    bool ValidateAccelerationStructureUpdate(VkAccelerationStructureKHR acc, const char* func_name, std::string* error_code,
                                             std::string* error_msg) const;
    bool ValidateUpdateDescriptorSetsWithTemplateKHR(VkDescriptorSet descriptorSet, const TEMPLATE_STATE* template_state,
                                                     const void* pData) const;
    bool ValidateAllocateDescriptorSets(const VkDescriptorSetAllocateInfo*,
                                        const cvdescriptorset::AllocateDescriptorSetsData*) const;
    bool ValidateUpdateDescriptorSets(uint32_t write_count, const VkWriteDescriptorSet* p_wds, uint32_t copy_count,
                                      const VkCopyDescriptorSet* p_cds, const char* func_name) const;

    // Stuff from shader_validation
    bool ValidateGraphicsPipelineShaderState(const PIPELINE_STATE* pPipeline) const;
    bool ValidateComputePipelineShaderState(PIPELINE_STATE* pPipeline) const;
    bool ValidateRayTracingPipeline(PIPELINE_STATE* pipeline, bool isKHR) const;
    bool PreCallValidateCreateShaderModule(VkDevice device, const VkShaderModuleCreateInfo* pCreateInfo,
                                           const VkAllocationCallbacks* pAllocator, VkShaderModule* pShaderModule) const;
    bool ValidatePipelineShaderStage(VkPipelineShaderStageCreateInfo const* pStage, const PIPELINE_STATE* pipeline,
                                     const PIPELINE_STATE::StageState& stage_state, const SHADER_MODULE_STATE* module,
                                     const spirv_inst_iter& entrypoint, bool check_point_size) const;
    bool ValidatePointListShaderState(const PIPELINE_STATE* pipeline, SHADER_MODULE_STATE const* src, spirv_inst_iter entrypoint,
                                      VkShaderStageFlagBits stage) const;
    bool ValidateShaderCapabilities(SHADER_MODULE_STATE const* src, VkShaderStageFlagBits stage) const;
    bool ValidateShaderStageWritableOrAtomicDescriptor(VkShaderStageFlagBits stage, bool has_writable_descriptor,
                                                       bool has_atomic_descriptor) const;
    bool ValidateShaderStageInputOutputLimits(SHADER_MODULE_STATE const* src, VkPipelineShaderStageCreateInfo const* pStage,
                                              const PIPELINE_STATE* pipeline, spirv_inst_iter entrypoint) const;
    bool ValidateShaderStageMaxResources(VkShaderStageFlagBits stage, const PIPELINE_STATE* pipeline) const;
    bool ValidateShaderStageGroupNonUniform(SHADER_MODULE_STATE const* src, VkShaderStageFlagBits stage) const;
    bool ValidateCooperativeMatrix(SHADER_MODULE_STATE const* src, VkPipelineShaderStageCreateInfo const* pStage,
                                   const PIPELINE_STATE* pipeline) const;
    bool ValidateExecutionModes(SHADER_MODULE_STATE const* src, spirv_inst_iter entrypoint) const;
    bool ValidateViConsistency(VkPipelineVertexInputStateCreateInfo const* vi) const;
    bool ValidateViAgainstVsInputs(VkPipelineVertexInputStateCreateInfo const* vi, SHADER_MODULE_STATE const* vs,
                                   spirv_inst_iter entrypoint) const;
    bool ValidateFsOutputsAgainstRenderPass(SHADER_MODULE_STATE const* fs, spirv_inst_iter entrypoint,
                                            PIPELINE_STATE const* pipeline, uint32_t subpass_index) const;
    bool ValidatePushConstantUsage(std::vector<VkPushConstantRange> const* push_constant_ranges, SHADER_MODULE_STATE const* src,
                                   std::unordered_set<uint32_t> accessible_ids, VkShaderStageFlagBits stage) const;
    bool ValidatePushConstantBlockAgainstPipeline(std::vector<VkPushConstantRange> const* push_constant_ranges,
                                                  SHADER_MODULE_STATE const* src, spirv_inst_iter type,
                                                  VkShaderStageFlagBits stage) const;
    bool ValidateSpecializationOffsets(VkPipelineShaderStageCreateInfo const* info) const;
    bool RequirePropertyFlag(VkBool32 check, char const* flag, char const* structure) const;
    bool RequireFeature(VkBool32 feature, char const* feature_name) const;
    bool RequireExtension(bool extension, char const* extension_name) const;
    bool ValidateInterfaceBetweenStages(SHADER_MODULE_STATE const* producer, spirv_inst_iter producer_entrypoint,
                                        shader_stage_attributes const* producer_stage, SHADER_MODULE_STATE const* consumer,
                                        spirv_inst_iter consumer_entrypoint, shader_stage_attributes const* consumer_stage) const;

    // Buffer Validation Functions
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
    bool ValidateIdleBuffer(VkBuffer buffer) const;
    template <typename T1>
    bool ValidateUsageFlags(VkFlags actual, VkFlags desired, VkBool32 strict, const T1 object,
                            const VulkanTypedHandle& typed_handle, const char* msgCode, char const* func_name,
                            char const* usage_str) const;
    bool ValidateImageSubresourceRange(const uint32_t image_mip_count, const uint32_t image_layer_count,
                                       const VkImageSubresourceRange& subresourceRange, const char* cmd_name,
                                       const char* param_name, const char* image_layer_count_var_name, const VkImage image,
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
    bool ValidateBufferImageCopyData(const CMD_BUFFER_STATE* cb_node, uint32_t regionCount, const VkBufferImageCopy* pRegions,
                                     const IMAGE_STATE* image_state, const char* function) const;
    bool ValidateBufferViewRange(const BUFFER_STATE* buffer_state, const VkBufferViewCreateInfo* pCreateInfo,
                                 const VkPhysicalDeviceLimits* device_limits) const;
    bool ValidateBufferViewBuffer(const BUFFER_STATE* buffer_state, const VkBufferViewCreateInfo* pCreateInfo) const;

    bool ValidateImageFormatFeatures(const VkImageCreateInfo* pCreateInfo) const;

    bool PreCallValidateCreateImage(VkDevice device, const VkImageCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator,
                                    VkImage* pImage) const;

    void PostCallRecordCreateImage(VkDevice device, const VkImageCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator,
                                   VkImage* pImage, VkResult result);

    void PreCallRecordDestroyImage(VkDevice device, VkImage image, const VkAllocationCallbacks* pAllocator);

    bool PreCallValidateDestroyImage(VkDevice device, VkImage image, const VkAllocationCallbacks* pAllocator) const;

    bool ValidateImageAttributes(const IMAGE_STATE* image_state, const VkImageSubresourceRange& range,
                                 const char* param_name) const;

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
                                           const VkImageSubresourceRange* pRanges) const;

    void PreCallRecordCmdClearColorImage(VkCommandBuffer commandBuffer, VkImage image, VkImageLayout imageLayout,
                                         const VkClearColorValue* pColor, uint32_t rangeCount,
                                         const VkImageSubresourceRange* pRanges);

    bool PreCallValidateCmdClearDepthStencilImage(VkCommandBuffer commandBuffer, VkImage image, VkImageLayout imageLayout,
                                                  const VkClearDepthStencilValue* pDepthStencil, uint32_t rangeCount,
                                                  const VkImageSubresourceRange* pRanges) const;

    void PreCallRecordCmdClearDepthStencilImage(VkCommandBuffer commandBuffer, VkImage image, VkImageLayout imageLayout,
                                                const VkClearDepthStencilValue* pDepthStencil, uint32_t rangeCount,
                                                const VkImageSubresourceRange* pRanges);

    bool FindLayouts(VkImage image, std::vector<VkImageLayout>& layouts) const;

    void SetImageViewLayout(CMD_BUFFER_STATE* cb_node, const IMAGE_VIEW_STATE& view_state, VkImageLayout layout,
                            VkImageLayout layoutStencil);
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
                                       const safe_VkAttachmentReference2& ref);

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
                                     const VkImageMemoryBarrier& mem_barrier, bool is_release_op);

    void TransitionFinalSubpassLayouts(CMD_BUFFER_STATE* pCB, const VkRenderPassBeginInfo* pRenderPassBegin,
                                       FRAMEBUFFER_STATE* framebuffer_state);

    bool PreCallValidateCmdCopyImage(VkCommandBuffer commandBuffer, VkImage srcImage, VkImageLayout srcImageLayout,
                                     VkImage dstImage, VkImageLayout dstImageLayout, uint32_t regionCount,
                                     const VkImageCopy* pRegions) const;

    bool PreCallValidateCmdClearAttachments(VkCommandBuffer commandBuffer, uint32_t attachmentCount,
                                            const VkClearAttachment* pAttachments, uint32_t rectCount,
                                            const VkClearRect* pRects) const;
    void PreCallRecordCmdClearAttachments(VkCommandBuffer commandBuffer, uint32_t attachmentCount,
                                          const VkClearAttachment* pAttachments, uint32_t rectCount, const VkClearRect* pRects);

    bool PreCallValidateCmdResolveImage(VkCommandBuffer commandBuffer, VkImage srcImage, VkImageLayout srcImageLayout,
                                        VkImage dstImage, VkImageLayout dstImageLayout, uint32_t regionCount,
                                        const VkImageResolve* pRegions) const;

    bool PreCallValidateCmdBlitImage(VkCommandBuffer commandBuffer, VkImage srcImage, VkImageLayout srcImageLayout,
                                     VkImage dstImage, VkImageLayout dstImageLayout, uint32_t regionCount,
                                     const VkImageBlit* pRegions, VkFilter filter) const;

    void PreCallRecordCmdBlitImage(VkCommandBuffer commandBuffer, VkImage srcImage, VkImageLayout srcImageLayout, VkImage dstImage,
                                   VkImageLayout dstImageLayout, uint32_t regionCount, const VkImageBlit* pRegions,
                                   VkFilter filter);

    bool ValidateCmdBufImageLayouts(const CMD_BUFFER_STATE* pCB, const GlobalImageLayoutMap& globalImageLayoutMap,
                                    GlobalImageLayoutMap* overlayLayoutMap_arg) const;

    void UpdateCmdBufImageLayouts(CMD_BUFFER_STATE* pCB);

    template <typename T1>
    bool VerifyBoundMemoryIsValid(const DEVICE_MEMORY_STATE* mem_state, const T1 object, const VulkanTypedHandle& typed_handle,
                                  const char* api_name, const char* error_code) const;

    bool ValidateLayoutVsAttachmentDescription(const debug_report_data* report_data, RenderPassCreateVersion rp_version,
                                               const VkImageLayout first_layout, const uint32_t attachment,
                                               const VkAttachmentDescription2KHR& attachment_description) const;

    bool ValidateImageUsageFlags(IMAGE_STATE const* image_state, VkFlags desired, bool strict, const char* msgCode,
                                 char const* func_name, char const* usage_string) const;

    bool ValidateImageFormatFeatureFlags(IMAGE_STATE const* image_state, VkFormatFeatureFlags desired, char const* func_name,
                                         const char* vuid) const;

    bool ValidateImageSubresourceLayers(const CMD_BUFFER_STATE* cb_node, const VkImageSubresourceLayers* subresource_layers,
                                        char const* func_name, char const* member, uint32_t i) const;

    bool ValidateBufferUsageFlags(BUFFER_STATE const* buffer_state, VkFlags desired, bool strict, const char* msgCode,
                                  char const* func_name, char const* usage_string) const;

    bool PreCallValidateCreateBuffer(VkDevice device, const VkBufferCreateInfo* pCreateInfo,
                                     const VkAllocationCallbacks* pAllocator, VkBuffer* pBuffer) const;

    bool PreCallValidateCreateBufferView(VkDevice device, const VkBufferViewCreateInfo* pCreateInfo,
                                         const VkAllocationCallbacks* pAllocator, VkBufferView* pView) const;

    bool ValidateImageAspectMask(VkImage image, VkFormat format, VkImageAspectFlags aspect_mask, const char* func_name,
                                 const char* vuid = kVUID_Core_DrawState_InvalidImageAspect) const;

    bool ValidateCreateImageViewSubresourceRange(const IMAGE_STATE* image_state, bool is_imageview_2d_type,
                                                 const VkImageSubresourceRange& subresourceRange) const;

    bool ValidateCmdClearColorSubresourceRange(const IMAGE_STATE* image_state, const VkImageSubresourceRange& subresourceRange,
                                               const char* param_name) const;

    bool ValidateCmdClearDepthSubresourceRange(const IMAGE_STATE* image_state, const VkImageSubresourceRange& subresourceRange,
                                               const char* param_name) const;

    bool ValidateImageBarrierSubresourceRange(const IMAGE_STATE* image_state, const VkImageSubresourceRange& subresourceRange,
                                              const char* cmd_name, const char* param_name) const;

    bool ValidateImageViewFormatFeatures(const IMAGE_STATE* image_state, const VkFormat view_format,
                                         const VkImageUsageFlags image_usage) const;

    bool PreCallValidateCreateImageView(VkDevice device, const VkImageViewCreateInfo* pCreateInfo,
                                        const VkAllocationCallbacks* pAllocator, VkImageView* pView) const;

    bool ValidateCmdCopyBufferBounds(const BUFFER_STATE* src_buffer_state, const BUFFER_STATE* dst_buffer_state,
                                     uint32_t regionCount, const VkBufferCopy* pRegions) const;
    bool ValidateImageBounds(const IMAGE_STATE* image_state, const uint32_t regionCount, const VkBufferImageCopy* pRegions,
                             const char* func_name, const char* msg_code) const;
    bool ValidateBufferBounds(const IMAGE_STATE* image_state, const BUFFER_STATE* buff_state, uint32_t regionCount,
                              const VkBufferImageCopy* pRegions, const char* func_name, const char* msg_code) const;

    bool ValidateCopyBufferImageTransferGranularityRequirements(const CMD_BUFFER_STATE* cb_node, const IMAGE_STATE* img,
                                                                const VkBufferImageCopy* region, const uint32_t i,
                                                                const char* function, const char* vuid) const;

    bool ValidateImageMipLevel(const CMD_BUFFER_STATE* cb_node, const IMAGE_STATE* img, uint32_t mip_level, const uint32_t i,
                               const char* function, const char* member, const char* vuid) const;

    bool ValidateImageArrayLayerRange(const CMD_BUFFER_STATE* cb_node, const IMAGE_STATE* img, const uint32_t base_layer,
                                      const uint32_t layer_count, const uint32_t i, const char* function, const char* member,
                                      const char* vuid) const;
    bool ValidateWaitSemaphores(VkDevice device, const VkSemaphoreWaitInfoKHR* pWaitInfo, uint64_t timeout) const;

    void PreCallRecordCmdCopyImage(VkCommandBuffer commandBuffer, VkImage srcImage, VkImageLayout srcImageLayout, VkImage dstImage,
                                   VkImageLayout dstImageLayout, uint32_t regionCount, const VkImageCopy* pRegions);

    bool PreCallValidateCmdCopyBuffer(VkCommandBuffer commandBuffer, VkBuffer srcBuffer, VkBuffer dstBuffer, uint32_t regionCount,
                                      const VkBufferCopy* pRegions) const;
    bool PreCallValidateDestroyImageView(VkDevice device, VkImageView imageView, const VkAllocationCallbacks* pAllocator) const;

    bool PreCallValidateDestroyBuffer(VkDevice device, VkBuffer buffer, const VkAllocationCallbacks* pAllocator) const;

    bool PreCallValidateDestroyBufferView(VkDevice device, VkBufferView bufferView, const VkAllocationCallbacks* pAllocator) const;

    bool PreCallValidateCmdFillBuffer(VkCommandBuffer commandBuffer, VkBuffer dstBuffer, VkDeviceSize dstOffset, VkDeviceSize size,
                                      uint32_t data) const;

    bool PreCallValidateCmdCopyImageToBuffer(VkCommandBuffer commandBuffer, VkImage srcImage, VkImageLayout srcImageLayout,
                                             VkBuffer dstBuffer, uint32_t regionCount, const VkBufferImageCopy* pRegions) const;

    void PreCallRecordCmdCopyImageToBuffer(VkCommandBuffer commandBuffer, VkImage srcImage, VkImageLayout srcImageLayout,
                                           VkBuffer dstBuffer, uint32_t regionCount, const VkBufferImageCopy* pRegions);

    bool PreCallValidateCmdCopyBufferToImage(VkCommandBuffer commandBuffer, VkBuffer srcBuffer, VkImage dstImage,
                                             VkImageLayout dstImageLayout, uint32_t regionCount,
                                             const VkBufferImageCopy* pRegions) const;

    void PreCallRecordCmdCopyBufferToImage(VkCommandBuffer commandBuffer, VkBuffer srcBuffer, VkImage dstImage,
                                           VkImageLayout dstImageLayout, uint32_t regionCount, const VkBufferImageCopy* pRegions);

    bool PreCallValidateGetImageSubresourceLayout(VkDevice device, VkImage image, const VkImageSubresource* pSubresource,
                                                  VkSubresourceLayout* pLayout) const;
    bool ValidateCreateImageANDROID(const debug_report_data* report_data, const VkImageCreateInfo* create_info) const;
    bool ValidateCreateImageViewANDROID(const VkImageViewCreateInfo* create_info) const;
    bool ValidateGetImageSubresourceLayoutANDROID(const VkImage image) const;
    bool ValidatePhysicalDeviceQueueFamilies(uint32_t queue_family_count, const uint32_t* queue_families, const char* cmd_name,
                                             const char* array_parameter_name, const char* vuid) const;
    bool ValidateAllocateMemoryANDROID(const VkMemoryAllocateInfo* alloc_info) const;
    bool ValidateGetImageMemoryRequirementsANDROID(const VkImage image, const char* func_name) const;
    bool ValidateGetPhysicalDeviceImageFormatProperties2ANDROID(const VkPhysicalDeviceImageFormatInfo2* pImageFormatInfo,
                                                                const VkImageFormatProperties2* pImageFormatProperties) const;
    bool ValidateBufferImportedHandleANDROID(const char* func_name, VkExternalMemoryHandleTypeFlags handleType,
                                             VkDeviceMemory memory, VkBuffer buffer) const;
    bool ValidateImageImportedHandleANDROID(const char* func_name, VkExternalMemoryHandleTypeFlags handleType,
                                            VkDeviceMemory memory, VkImage image) const;
    bool PreCallValidateCreatePipelineCache(VkDevice device, const VkPipelineCacheCreateInfo* pCreateInfo,
                                            const VkAllocationCallbacks* pAllocator, VkPipelineCache* pPipelineCache) const;
    bool PreCallValidateCreateGraphicsPipelines(VkDevice device, VkPipelineCache pipelineCache, uint32_t count,
                                                const VkGraphicsPipelineCreateInfo* pCreateInfos,
                                                const VkAllocationCallbacks* pAllocator, VkPipeline* pPipelines,
                                                void* cgpl_state) const;
    bool PreCallValidateCreateComputePipelines(VkDevice device, VkPipelineCache pipelineCache, uint32_t count,
                                               const VkComputePipelineCreateInfo* pCreateInfos,
                                               const VkAllocationCallbacks* pAllocator, VkPipeline* pPipelines,
                                               void* pipe_state) const;
    bool PreCallValidateGetPipelineExecutablePropertiesKHR(VkDevice device, const VkPipelineInfoKHR* pPipelineInfo,
                                                           uint32_t* pExecutableCount,
                                                           VkPipelineExecutablePropertiesKHR* pProperties) const;
    bool ValidatePipelineExecutableInfo(VkDevice device, const VkPipelineExecutableInfoKHR* pExecutableInfo) const;
    bool PreCallValidateGetPipelineExecutableStatisticsKHR(VkDevice device, const VkPipelineExecutableInfoKHR* pExecutableInfo,
                                                           uint32_t* pStatisticCount,
                                                           VkPipelineExecutableStatisticKHR* pStatistics) const;
    bool PreCallValidateGetPipelineExecutableInternalRepresentationsKHR(
        VkDevice device, const VkPipelineExecutableInfoKHR* pExecutableInfo, uint32_t* pInternalRepresentationCount,
        VkPipelineExecutableInternalRepresentationKHR* pStatistics) const;
    bool PreCallValidateCreatePipelineLayout(VkDevice device, const VkPipelineLayoutCreateInfo* pCreateInfo,
                                             const VkAllocationCallbacks* pAllocator, VkPipelineLayout* pPipelineLayout) const;
    bool PreCallValidateAllocateDescriptorSets(VkDevice device, const VkDescriptorSetAllocateInfo* pAllocateInfo,
                                               VkDescriptorSet* pDescriptorSets, void* ads_state) const;
    bool PreCallValidateCreateRayTracingPipelinesNV(VkDevice device, VkPipelineCache pipelineCache, uint32_t count,
                                                    const VkRayTracingPipelineCreateInfoNV* pCreateInfos,
                                                    const VkAllocationCallbacks* pAllocator, VkPipeline* pPipelines,
                                                    void* pipe_state) const;
    bool PreCallValidateCreateRayTracingPipelinesKHR(VkDevice device, VkPipelineCache pipelineCache, uint32_t count,
                                                     const VkRayTracingPipelineCreateInfoKHR* pCreateInfos,
                                                     const VkAllocationCallbacks* pAllocator, VkPipeline* pPipelines,
                                                     void* pipe_state) const;
    bool PreCallValidateCmdTraceRaysNV(VkCommandBuffer commandBuffer, VkBuffer raygenShaderBindingTableBuffer,
                                       VkDeviceSize raygenShaderBindingOffset, VkBuffer missShaderBindingTableBuffer,
                                       VkDeviceSize missShaderBindingOffset, VkDeviceSize missShaderBindingStride,
                                       VkBuffer hitShaderBindingTableBuffer, VkDeviceSize hitShaderBindingOffset,
                                       VkDeviceSize hitShaderBindingStride, VkBuffer callableShaderBindingTableBuffer,
                                       VkDeviceSize callableShaderBindingOffset, VkDeviceSize callableShaderBindingStride,
                                       uint32_t width, uint32_t height, uint32_t depth) const;
    void PostCallRecordCmdTraceRaysNV(VkCommandBuffer commandBuffer, VkBuffer raygenShaderBindingTableBuffer,
                                      VkDeviceSize raygenShaderBindingOffset, VkBuffer missShaderBindingTableBuffer,
                                      VkDeviceSize missShaderBindingOffset, VkDeviceSize missShaderBindingStride,
                                      VkBuffer hitShaderBindingTableBuffer, VkDeviceSize hitShaderBindingOffset,
                                      VkDeviceSize hitShaderBindingStride, VkBuffer callableShaderBindingTableBuffer,
                                      VkDeviceSize callableShaderBindingOffset, VkDeviceSize callableShaderBindingStride,
                                      uint32_t width, uint32_t height, uint32_t depth);
    bool PreCallValidateCmdTraceRaysKHR(VkCommandBuffer commandBuffer, const VkStridedBufferRegionKHR* pRaygenShaderBindingTable,
                                        const VkStridedBufferRegionKHR* pMissShaderBindingTable,
                                        const VkStridedBufferRegionKHR* pHitShaderBindingTable,
                                        const VkStridedBufferRegionKHR* pCallableShaderBindingTable, uint32_t width,
                                        uint32_t height, uint32_t depth) const;
    void PostCallRecordCmdTraceRaysKHR(VkCommandBuffer commandBuffer, const VkStridedBufferRegionKHR* pRaygenShaderBindingTable,
                                       const VkStridedBufferRegionKHR* pMissShaderBindingTable,
                                       const VkStridedBufferRegionKHR* pHitShaderBindingTable,
                                       const VkStridedBufferRegionKHR* pCallableShaderBindingTable, uint32_t width, uint32_t height,
                                       uint32_t depth);
    bool PreCallValidateCmdTraceRaysIndirectKHR(VkCommandBuffer commandBuffer,
                                                const VkStridedBufferRegionKHR* pRaygenShaderBindingTable,
                                                const VkStridedBufferRegionKHR* pMissShaderBindingTable,
                                                const VkStridedBufferRegionKHR* pHitShaderBindingTable,
                                                const VkStridedBufferRegionKHR* pCallableShaderBindingTable, VkBuffer buffer,
                                                VkDeviceSize offset) const;
    void PostCallRecordCmdTraceRaysIndirectKHR(VkCommandBuffer commandBuffer,
                                               const VkStridedBufferRegionKHR* pRaygenShaderBindingTable,
                                               const VkStridedBufferRegionKHR* pMissShaderBindingTable,
                                               const VkStridedBufferRegionKHR* pHitShaderBindingTable,
                                               const VkStridedBufferRegionKHR* pCallableShaderBindingTable, VkBuffer buffer,
                                               VkDeviceSize offset);
    bool PreCallValidateCreateDevice(VkPhysicalDevice gpu, const VkDeviceCreateInfo* pCreateInfo,
                                     const VkAllocationCallbacks* pAllocator, VkDevice* pDevice) const;
    void PostCallRecordCreateDevice(VkPhysicalDevice gpu, const VkDeviceCreateInfo* pCreateInfo,
                                    const VkAllocationCallbacks* pAllocator, VkDevice* pDevice, VkResult result);
    bool PreCallValidateCmdUpdateBuffer(VkCommandBuffer commandBuffer, VkBuffer dstBuffer, VkDeviceSize dstOffset,
                                        VkDeviceSize dataSize, const void* pData) const;
    bool PreCallValidateGetDeviceQueue(VkDevice device, uint32_t queueFamilyIndex, uint32_t queueIndex, VkQueue* pQueue) const;
    bool PreCallValidateCreateSamplerYcbcrConversion(VkDevice device, const VkSamplerYcbcrConversionCreateInfo* pCreateInfo,
                                                     const VkAllocationCallbacks* pAllocator,
                                                     VkSamplerYcbcrConversion* pYcbcrConversion) const;
    bool PreCallValidateCreateSamplerYcbcrConversionKHR(VkDevice device, const VkSamplerYcbcrConversionCreateInfo* pCreateInfo,
                                                        const VkAllocationCallbacks* pAllocator,
                                                        VkSamplerYcbcrConversion* pYcbcrConversion) const;
    bool PreCallValidateCreateSampler(VkDevice device, const VkSamplerCreateInfo* pCreateInfo,
                                      const VkAllocationCallbacks* pAllocator, VkSampler* pSampler) const;
    bool PreCallValidateCmdDebugMarkerBeginEXT(VkCommandBuffer commandBuffer, const VkDebugMarkerMarkerInfoEXT* pMarkerInfo) const;
    void PreCallRecordDestroyDevice(VkDevice device, const VkAllocationCallbacks* pAllocator);
    bool PreCallValidateQueueSubmit(VkQueue queue, uint32_t submitCount, const VkSubmitInfo* pSubmits, VkFence fence) const;
    void PostCallRecordQueueSubmit(VkQueue queue, uint32_t submitCount, const VkSubmitInfo* pSubmits, VkFence fence,
                                   VkResult result);
    bool PreCallValidateAllocateMemory(VkDevice device, const VkMemoryAllocateInfo* pAllocateInfo,
                                       const VkAllocationCallbacks* pAllocator, VkDeviceMemory* pMemory) const;
    bool PreCallValidateFreeMemory(VkDevice device, VkDeviceMemory mem, const VkAllocationCallbacks* pAllocator) const;
    bool PreCallValidateWaitForFences(VkDevice device, uint32_t fenceCount, const VkFence* pFences, VkBool32 waitAll,
                                      uint64_t timeout) const;
    bool PreCallValidateQueueWaitIdle(VkQueue queue) const;
    bool PreCallValidateDeviceWaitIdle(VkDevice device) const;
    bool PreCallValidateCreateSemaphore(VkDevice device, const VkSemaphoreCreateInfo* pCreateInfo,
                                        const VkAllocationCallbacks* pAllocator, VkSemaphore* pSemaphore) const;
    bool PreCallValidateWaitSemaphores(VkDevice device, const VkSemaphoreWaitInfoKHR* pWaitInfo, uint64_t timeout) const;
    bool PreCallValidateWaitSemaphoresKHR(VkDevice device, const VkSemaphoreWaitInfoKHR* pWaitInfo, uint64_t timeout) const;
    bool PreCallValidateDestroyFence(VkDevice device, VkFence fence, const VkAllocationCallbacks* pAllocator) const;
    bool PreCallValidateDestroySemaphore(VkDevice device, VkSemaphore semaphore, const VkAllocationCallbacks* pAllocator) const;
    bool PreCallValidateDestroyEvent(VkDevice device, VkEvent event, const VkAllocationCallbacks* pAllocator) const;
    bool PreCallValidateDestroyQueryPool(VkDevice device, VkQueryPool queryPool, const VkAllocationCallbacks* pAllocator) const;
    bool ValidateGetQueryPoolResultsFlags(VkQueryPool queryPool, VkQueryResultFlags flags) const;
    bool ValidateGetQueryPoolResultsQueries(VkQueryPool queryPool, uint32_t firstQuery, uint32_t queryCount) const;
    bool ValidatePerformanceQueryResults(const char* cmd_name, const QUERY_POOL_STATE* query_pool_state, uint32_t firstQuery,
                                         uint32_t queryCount, VkQueryResultFlags flags) const;
    bool ValidateGetQueryPoolPerformanceResults(VkQueryPool queryPool, uint32_t firstQuery, uint32_t queryCount, void* pData,
                                                VkDeviceSize stride, VkQueryResultFlags flags) const;
    bool PreCallValidateGetQueryPoolResults(VkDevice device, VkQueryPool queryPool, uint32_t firstQuery, uint32_t queryCount,
                                            size_t dataSize, void* pData, VkDeviceSize stride, VkQueryResultFlags flags) const;
    bool PreCallValidateBindBufferMemory2KHR(VkDevice device, uint32_t bindInfoCount,
                                             const VkBindBufferMemoryInfoKHR* pBindInfos) const;
    bool PreCallValidateBindBufferMemory2(VkDevice device, uint32_t bindInfoCount,
                                          const VkBindBufferMemoryInfoKHR* pBindInfos) const;
    bool PreCallValidateBindBufferMemory(VkDevice device, VkBuffer buffer, VkDeviceMemory mem, VkDeviceSize memoryOffset) const;
    bool PreCallValidateGetImageMemoryRequirements(VkDevice device, VkImage image, VkMemoryRequirements* pMemoryRequirements) const;
    bool PreCallValidateGetImageMemoryRequirements2(VkDevice device, const VkImageMemoryRequirementsInfo2* pInfo,
                                                    VkMemoryRequirements2* pMemoryRequirements) const;
    bool PreCallValidateGetImageMemoryRequirements2KHR(VkDevice device, const VkImageMemoryRequirementsInfo2* pInfo,
                                                       VkMemoryRequirements2* pMemoryRequirements) const;
    bool PreCallValidateGetPhysicalDeviceImageFormatProperties2(VkPhysicalDevice physicalDevice,
                                                                const VkPhysicalDeviceImageFormatInfo2* pImageFormatInfo,
                                                                VkImageFormatProperties2* pImageFormatProperties) const;
    bool PreCallValidateGetPhysicalDeviceImageFormatProperties2KHR(VkPhysicalDevice physicalDevice,
                                                                   const VkPhysicalDeviceImageFormatInfo2* pImageFormatInfo,
                                                                   VkImageFormatProperties2* pImageFormatProperties) const;
    bool PreCallValidateDestroyPipeline(VkDevice device, VkPipeline pipeline, const VkAllocationCallbacks* pAllocator) const;
    bool PreCallValidateDestroySampler(VkDevice device, VkSampler sampler, const VkAllocationCallbacks* pAllocator) const;
    bool PreCallValidateDestroyDescriptorPool(VkDevice device, VkDescriptorPool descriptorPool,
                                              const VkAllocationCallbacks* pAllocator) const;
    bool PreCallValidateFreeCommandBuffers(VkDevice device, VkCommandPool commandPool, uint32_t commandBufferCount,
                                           const VkCommandBuffer* pCommandBuffers) const;
    bool PreCallValidateCreateCommandPool(VkDevice device, const VkCommandPoolCreateInfo* pCreateInfo,
                                          const VkAllocationCallbacks* pAllocator, VkCommandPool* pCommandPool) const;
    bool PreCallValidateCreateQueryPool(VkDevice device, const VkQueryPoolCreateInfo* pCreateInfo,
                                        const VkAllocationCallbacks* pAllocator, VkQueryPool* pQueryPool) const;
    bool PreCallValidateDestroyCommandPool(VkDevice device, VkCommandPool commandPool,
                                           const VkAllocationCallbacks* pAllocator) const;
    bool PreCallValidateResetCommandPool(VkDevice device, VkCommandPool commandPool, VkCommandPoolResetFlags flags) const;
    bool PreCallValidateResetFences(VkDevice device, uint32_t fenceCount, const VkFence* pFences) const;
    bool PreCallValidateDestroyFramebuffer(VkDevice device, VkFramebuffer framebuffer,
                                           const VkAllocationCallbacks* pAllocator) const;
    bool PreCallValidateDestroyRenderPass(VkDevice device, VkRenderPass renderPass, const VkAllocationCallbacks* pAllocator) const;
    bool PreCallValidateCreateDescriptorSetLayout(VkDevice device, const VkDescriptorSetLayoutCreateInfo* pCreateInfo,
                                                  const VkAllocationCallbacks* pAllocator, VkDescriptorSetLayout* pSetLayout) const;
    bool PreCallValidateResetDescriptorPool(VkDevice device, VkDescriptorPool descriptorPool,
                                            VkDescriptorPoolResetFlags flags) const;
    bool PreCallValidateFreeDescriptorSets(VkDevice device, VkDescriptorPool descriptorPool, uint32_t count,
                                           const VkDescriptorSet* pDescriptorSets) const;
    bool PreCallValidateUpdateDescriptorSets(VkDevice device, uint32_t descriptorWriteCount,
                                             const VkWriteDescriptorSet* pDescriptorWrites, uint32_t descriptorCopyCount,
                                             const VkCopyDescriptorSet* pDescriptorCopies) const;
    bool PreCallValidateBeginCommandBuffer(VkCommandBuffer commandBuffer, const VkCommandBufferBeginInfo* pBeginInfo) const;
    bool PreCallValidateEndCommandBuffer(VkCommandBuffer commandBuffer) const;
    bool PreCallValidateResetCommandBuffer(VkCommandBuffer commandBuffer, VkCommandBufferResetFlags flags) const;
    bool PreCallValidateCmdBindPipeline(VkCommandBuffer commandBuffer, VkPipelineBindPoint pipelineBindPoint,
                                        VkPipeline pipeline) const;
    bool PreCallValidateCmdSetViewport(VkCommandBuffer commandBuffer, uint32_t firstViewport, uint32_t viewportCount,
                                       const VkViewport* pViewports) const;
    bool PreCallValidateCmdSetScissor(VkCommandBuffer commandBuffer, uint32_t firstScissor, uint32_t scissorCount,
                                      const VkRect2D* pScissors) const;
    bool PreCallValidateCmdSetExclusiveScissorNV(VkCommandBuffer commandBuffer, uint32_t firstExclusiveScissor,
                                                 uint32_t exclusiveScissorCount, const VkRect2D* pExclusiveScissors) const;

    bool PreCallValidateCmdSetViewportWScalingNV(VkCommandBuffer commandBuffer, uint32_t firstViewport, uint32_t viewportCount,
                                                 const VkViewportWScalingNV* pViewportWScalings) const;

    bool PreCallValidateCmdBindShadingRateImageNV(VkCommandBuffer commandBuffer, VkImageView imageView,
                                                  VkImageLayout imageLayout) const;
    bool PreCallValidateCmdSetViewportShadingRatePaletteNV(VkCommandBuffer commandBuffer, uint32_t firstViewport,
                                                           uint32_t viewportCount,
                                                           const VkShadingRatePaletteNV* pShadingRatePalettes) const;
    bool ValidateGeometryTrianglesNV(const VkGeometryTrianglesNV& triangles, const char* func_name) const;
    bool ValidateGeometryAABBNV(const VkGeometryAABBNV& geometry, const char* func_name) const;
    bool ValidateGeometryNV(const VkGeometryNV& geometry, const char* func_name) const;
    bool PreCallValidateCreateAccelerationStructureNV(VkDevice device, const VkAccelerationStructureCreateInfoNV* pCreateInfo,
                                                      const VkAllocationCallbacks* pAllocator,
                                                      VkAccelerationStructureNV* pAccelerationStructure) const;
    bool PreCallValidateCreateAccelerationStructureKHR(VkDevice device, const VkAccelerationStructureCreateInfoKHR* pCreateInfo,
                                                       const VkAllocationCallbacks* pAllocator,
                                                       VkAccelerationStructureKHR* pAccelerationStructure) const;
    bool PreCallValidateBindAccelerationStructureMemoryNV(VkDevice device, uint32_t bindInfoCount,
                                                          const VkBindAccelerationStructureMemoryInfoNV* pBindInfos) const;
    bool PreCallValidateBindAccelerationStructureMemoryKHR(VkDevice device, uint32_t bindInfoCount,
                                                           const VkBindAccelerationStructureMemoryInfoKHR* pBindInfos) const;
    bool PreCallValidateGetAccelerationStructureHandleNV(VkDevice device, VkAccelerationStructureNV accelerationStructure,
                                                         size_t dataSize, void* pData) const;
    bool PreCallValidateCmdBuildAccelerationStructureKHR(
        VkCommandBuffer commandBuffer, uint32_t infoCount, const VkAccelerationStructureBuildGeometryInfoKHR* pInfos,
        const VkAccelerationStructureBuildOffsetInfoKHR* const* ppOffsetInfos) const;
    bool PreCallValidateCmdBuildAccelerationStructureNV(VkCommandBuffer commandBuffer, const VkAccelerationStructureInfoNV* pInfo,
                                                        VkBuffer instanceData, VkDeviceSize instanceOffset, VkBool32 update,
                                                        VkAccelerationStructureNV dst, VkAccelerationStructureNV src,
                                                        VkBuffer scratch, VkDeviceSize scratchOffset) const;
    bool PreCallValidateCmdCopyAccelerationStructureNV(VkCommandBuffer commandBuffer, VkAccelerationStructureNV dst,
                                                       VkAccelerationStructureNV src, VkCopyAccelerationStructureModeNV mode) const;
    bool PreCallValidateDestroyAccelerationStructureNV(VkDevice device, VkAccelerationStructureNV accelerationStructure,
                                                       const VkAllocationCallbacks* pAllocator) const;
    bool PreCallValidateDestroyAccelerationStructureKHR(VkDevice device, VkAccelerationStructureKHR accelerationStructure,
                                                        const VkAllocationCallbacks* pAllocator) const;
    bool PreCallValidateCmdSetLineWidth(VkCommandBuffer commandBuffer, float lineWidth) const;
    bool PreCallValidateCmdSetLineStippleEXT(VkCommandBuffer commandBuffer, uint32_t lineStippleFactor,
                                             uint16_t lineStipplePattern) const;
    bool PreCallValidateCmdSetDepthBias(VkCommandBuffer commandBuffer, float depthBiasConstantFactor, float depthBiasClamp,
                                        float depthBiasSlopeFactor) const;
    bool PreCallValidateCmdSetBlendConstants(VkCommandBuffer commandBuffer, const float blendConstants[4]) const;
    bool PreCallValidateCmdSetDepthBounds(VkCommandBuffer commandBuffer, float minDepthBounds, float maxDepthBounds) const;
    bool PreCallValidateCmdSetStencilCompareMask(VkCommandBuffer commandBuffer, VkStencilFaceFlags faceMask,
                                                 uint32_t compareMask) const;
    bool PreCallValidateCmdSetStencilWriteMask(VkCommandBuffer commandBuffer, VkStencilFaceFlags faceMask,
                                               uint32_t writeMask) const;
    bool PreCallValidateCmdSetStencilReference(VkCommandBuffer commandBuffer, VkStencilFaceFlags faceMask,
                                               uint32_t reference) const;
    bool PreCallValidateCmdBindDescriptorSets(VkCommandBuffer commandBuffer, VkPipelineBindPoint pipelineBindPoint,
                                              VkPipelineLayout layout, uint32_t firstSet, uint32_t setCount,
                                              const VkDescriptorSet* pDescriptorSets, uint32_t dynamicOffsetCount,
                                              const uint32_t* pDynamicOffsets) const;
    bool PreCallValidateCmdPushDescriptorSetKHR(VkCommandBuffer commandBuffer, VkPipelineBindPoint pipelineBindPoint,
                                                VkPipelineLayout layout, uint32_t set, uint32_t descriptorWriteCount,
                                                const VkWriteDescriptorSet* pDescriptorWrites) const;
    bool PreCallValidateCmdBindIndexBuffer(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                           VkIndexType indexType) const;
    bool PreCallValidateCmdBindVertexBuffers(VkCommandBuffer commandBuffer, uint32_t firstBinding, uint32_t bindingCount,
                                             const VkBuffer* pBuffers, const VkDeviceSize* pOffsets) const;
    bool PreCallValidateCmdDraw(VkCommandBuffer commandBuffer, uint32_t vertexCount, uint32_t instanceCount, uint32_t firstVertex,
                                uint32_t firstInstance) const;
    bool PreCallValidateCmdDrawIndexed(VkCommandBuffer commandBuffer, uint32_t indexCount, uint32_t instanceCount,
                                       uint32_t firstIndex, int32_t vertexOffset, uint32_t firstInstance) const;
    bool PreCallValidateCmdDrawIndexedIndirect(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset, uint32_t count,
                                               uint32_t stride) const;
    bool ValidateCmdDrawIndexedIndirectCount(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                             VkBuffer countBuffer, VkDeviceSize countBufferOffset, uint32_t maxDrawCount,
                                             uint32_t stride, const char* apiName) const;
    bool PreCallValidateCmdDrawIndexedIndirectCountKHR(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                       VkBuffer countBuffer, VkDeviceSize countBufferOffset, uint32_t maxDrawCount,
                                                       uint32_t stride) const;
    bool PreCallValidateCmdDrawIndexedIndirectCount(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                    VkBuffer countBuffer, VkDeviceSize countBufferOffset, uint32_t maxDrawCount,
                                                    uint32_t stride) const;
    bool PreCallValidateCmdDispatch(VkCommandBuffer commandBuffer, uint32_t x, uint32_t y, uint32_t z) const;
    bool PreCallValidateCmdDispatchIndirect(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset) const;
    bool PreCallValidateCmdDrawIndirect(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset, uint32_t count,
                                        uint32_t stride) const;
    bool PreCallValidateCmdDispatchBase(VkCommandBuffer commandBuffer, uint32_t baseGroupX, uint32_t baseGroupY,
                                        uint32_t baseGroupZ, uint32_t groupCountX, uint32_t groupCountY,
                                        uint32_t groupCountZ) const;
    bool PreCallValidateCmdDispatchBaseKHR(VkCommandBuffer commandBuffer, uint32_t baseGroupX, uint32_t baseGroupY,
                                           uint32_t baseGroupZ, uint32_t groupCountX, uint32_t groupCountY,
                                           uint32_t groupCountZ) const;
    bool PreCallValidateCmdSetEvent(VkCommandBuffer commandBuffer, VkEvent event, VkPipelineStageFlags stageMask) const;
    bool PreCallValidateCmdResetEvent(VkCommandBuffer commandBuffer, VkEvent event, VkPipelineStageFlags stageMask) const;
    bool PreCallValidateCmdWaitEvents(VkCommandBuffer commandBuffer, uint32_t eventCount, const VkEvent* pEvents,
                                      VkPipelineStageFlags sourceStageMask, VkPipelineStageFlags dstStageMask,
                                      uint32_t memoryBarrierCount, const VkMemoryBarrier* pMemoryBarriers,
                                      uint32_t bufferMemoryBarrierCount, const VkBufferMemoryBarrier* pBufferMemoryBarriers,
                                      uint32_t imageMemoryBarrierCount, const VkImageMemoryBarrier* pImageMemoryBarriers) const;
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
                                           uint32_t imageMemoryBarrierCount,
                                           const VkImageMemoryBarrier* pImageMemoryBarriers) const;
    void PreCallRecordCmdPipelineBarrier(VkCommandBuffer commandBuffer, VkPipelineStageFlags srcStageMask,
                                         VkPipelineStageFlags dstStageMask, VkDependencyFlags dependencyFlags,
                                         uint32_t memoryBarrierCount, const VkMemoryBarrier* pMemoryBarriers,
                                         uint32_t bufferMemoryBarrierCount, const VkBufferMemoryBarrier* pBufferMemoryBarriers,
                                         uint32_t imageMemoryBarrierCount, const VkImageMemoryBarrier* pImageMemoryBarriers);

    void EnqueueVerifyBeginQuery(VkCommandBuffer, const QueryObject& query_obj, const char* func);
    bool PreCallValidateCmdBeginQuery(VkCommandBuffer commandBuffer, VkQueryPool queryPool, uint32_t slot, VkFlags flags) const;
    void PreCallRecordCmdBeginQuery(VkCommandBuffer commandBuffer, VkQueryPool queryPool, uint32_t slot, VkFlags flags);
    void EnqueueVerifyEndQuery(VkCommandBuffer, const QueryObject& query_obj);
    bool PreCallValidateCmdEndQuery(VkCommandBuffer commandBuffer, VkQueryPool queryPool, uint32_t slot) const;
    void PreCallRecordCmdEndQuery(VkCommandBuffer commandBuffer, VkQueryPool queryPool, uint32_t slot);
    bool ValidateQueryPoolIndex(VkQueryPool queryPool, uint32_t firstQuery, uint32_t queryCount, const char* func_name,
                                const char* first_vuid, const char* sum_vuid) const;
    bool PreCallValidateCmdResetQueryPool(VkCommandBuffer commandBuffer, VkQueryPool queryPool, uint32_t firstQuery,
                                          uint32_t queryCount) const;
    bool PreCallValidateCmdCopyQueryPoolResults(VkCommandBuffer commandBuffer, VkQueryPool queryPool, uint32_t firstQuery,
                                                uint32_t queryCount, VkBuffer dstBuffer, VkDeviceSize dstOffset,
                                                VkDeviceSize stride, VkQueryResultFlags flags) const;
    void PreCallRecordCmdCopyQueryPoolResults(VkCommandBuffer commandBuffer, VkQueryPool queryPool, uint32_t firstQuery,
                                              uint32_t queryCount, VkBuffer dstBuffer, VkDeviceSize dstOffset, VkDeviceSize stride,
                                              VkQueryResultFlags flags);
    bool PreCallValidateCmdPushConstants(VkCommandBuffer commandBuffer, VkPipelineLayout layout, VkShaderStageFlags stageFlags,
                                         uint32_t offset, uint32_t size, const void* pValues) const;
    bool PreCallValidateCmdWriteTimestamp(VkCommandBuffer commandBuffer, VkPipelineStageFlagBits pipelineStage,
                                          VkQueryPool queryPool, uint32_t slot) const;
    void PreCallRecordCmdWriteTimestamp(VkCommandBuffer commandBuffer, VkPipelineStageFlagBits pipelineStage, VkQueryPool queryPool,
                                        uint32_t slot);
    bool PreCallValidateCreateFramebuffer(VkDevice device, const VkFramebufferCreateInfo* pCreateInfo,
                                          const VkAllocationCallbacks* pAllocator, VkFramebuffer* pFramebuffer) const;
    bool PreCallValidateCreateRenderPass(VkDevice device, const VkRenderPassCreateInfo* pCreateInfo,
                                         const VkAllocationCallbacks* pAllocator, VkRenderPass* pRenderPass) const;
    bool PreCallValidateGetDeviceMemoryCommitment(VkDevice device, VkDeviceMemory mem, VkDeviceSize* pCommittedMem) const;
    bool ValidateCreateRenderPass2(VkDevice device, const VkRenderPassCreateInfo2* pCreateInfo,
                                   const VkAllocationCallbacks* pAllocator, VkRenderPass* pRenderPass,
                                   const char* function_name) const;
    bool PreCallValidateCreateRenderPass2KHR(VkDevice device, const VkRenderPassCreateInfo2KHR* pCreateInfo,
                                             const VkAllocationCallbacks* pAllocator, VkRenderPass* pRenderPass) const;
    bool PreCallValidateCreateRenderPass2(VkDevice device, const VkRenderPassCreateInfo2* pCreateInfo,
                                          const VkAllocationCallbacks* pAllocator, VkRenderPass* pRenderPass) const;
    bool PreCallValidateCmdBeginRenderPass(VkCommandBuffer commandBuffer, const VkRenderPassBeginInfo* pRenderPassBegin,
                                           VkSubpassContents contents) const;
    void PreCallRecordCmdBeginRenderPass(VkCommandBuffer commandBuffer, const VkRenderPassBeginInfo* pRenderPassBegin,
                                         VkSubpassContents contents);
    bool PreCallValidateCmdBeginRenderPass2KHR(VkCommandBuffer commandBuffer, const VkRenderPassBeginInfo* pRenderPassBegin,
                                               const VkSubpassBeginInfoKHR* pSubpassBeginInfo) const;
    bool PreCallValidateCmdBeginRenderPass2(VkCommandBuffer commandBuffer, const VkRenderPassBeginInfo* pRenderPassBegin,
                                            const VkSubpassBeginInfoKHR* pSubpassBeginInfo) const;
    void PreCallRecordCmdBeginRenderPass2KHR(VkCommandBuffer commandBuffer, const VkRenderPassBeginInfo* pRenderPassBegin,
                                             const VkSubpassBeginInfoKHR* pSubpassBeginInfo);
    void PreCallRecordCmdBeginRenderPass2(VkCommandBuffer commandBuffer, const VkRenderPassBeginInfo* pRenderPassBegin,
                                          const VkSubpassBeginInfoKHR* pSubpassBeginInfo);
    bool PreCallValidateCmdNextSubpass(VkCommandBuffer commandBuffer, VkSubpassContents contents) const;
    void PostCallRecordCmdNextSubpass(VkCommandBuffer commandBuffer, VkSubpassContents contents);
    bool PreCallValidateCmdNextSubpass2KHR(VkCommandBuffer commandBuffer, const VkSubpassBeginInfoKHR* pSubpassBeginInfo,
                                           const VkSubpassEndInfoKHR* pSubpassEndInfo) const;
    void PostCallRecordCmdNextSubpass2KHR(VkCommandBuffer commandBuffer, const VkSubpassBeginInfoKHR* pSubpassBeginInfo,
                                          const VkSubpassEndInfoKHR* pSubpassEndInfo);
    bool PreCallValidateCmdNextSubpass2(VkCommandBuffer commandBuffer, const VkSubpassBeginInfoKHR* pSubpassBeginInfo,
                                        const VkSubpassEndInfoKHR* pSubpassEndInfo) const;
    void PostCallRecordCmdNextSubpass2(VkCommandBuffer commandBuffer, const VkSubpassBeginInfoKHR* pSubpassBeginInfo,
                                       const VkSubpassEndInfoKHR* pSubpassEndInfo);
    bool PreCallValidateCmdEndRenderPass(VkCommandBuffer commandBuffer) const;
    void PostCallRecordCmdEndRenderPass(VkCommandBuffer commandBuffer);
    bool PreCallValidateCmdEndRenderPass2KHR(VkCommandBuffer commandBuffer, const VkSubpassEndInfoKHR* pSubpassEndInfo) const;
    void PostCallRecordCmdEndRenderPass2KHR(VkCommandBuffer commandBuffer, const VkSubpassEndInfoKHR* pSubpassEndInfo);
    bool PreCallValidateCmdEndRenderPass2(VkCommandBuffer commandBuffer, const VkSubpassEndInfoKHR* pSubpassEndInfo) const;
    void PostCallRecordCmdEndRenderPass2(VkCommandBuffer commandBuffer, const VkSubpassEndInfoKHR* pSubpassEndInfo);
    bool PreCallValidateCmdExecuteCommands(VkCommandBuffer commandBuffer, uint32_t commandBuffersCount,
                                           const VkCommandBuffer* pCommandBuffers) const;
    bool PreCallValidateMapMemory(VkDevice device, VkDeviceMemory mem, VkDeviceSize offset, VkDeviceSize size, VkFlags flags,
                                  void** ppData) const;
    bool PreCallValidateUnmapMemory(VkDevice device, VkDeviceMemory mem) const;
    bool PreCallValidateFlushMappedMemoryRanges(VkDevice device, uint32_t memRangeCount,
                                                const VkMappedMemoryRange* pMemRanges) const;
    bool PreCallValidateInvalidateMappedMemoryRanges(VkDevice device, uint32_t memRangeCount,
                                                     const VkMappedMemoryRange* pMemRanges) const;
    bool PreCallValidateBindImageMemory(VkDevice device, VkImage image, VkDeviceMemory mem, VkDeviceSize memoryOffset) const;
    bool PreCallValidateBindImageMemory2(VkDevice device, uint32_t bindInfoCount, const VkBindImageMemoryInfoKHR* pBindInfos) const;
    bool PreCallValidateBindImageMemory2KHR(VkDevice device, uint32_t bindInfoCount,
                                            const VkBindImageMemoryInfoKHR* pBindInfos) const;
    bool PreCallValidateSetEvent(VkDevice device, VkEvent event) const;
    bool PreCallValidateQueueBindSparse(VkQueue queue, uint32_t bindInfoCount, const VkBindSparseInfo* pBindInfo,
                                        VkFence fence) const;
    bool PreCallValidateSignalSemaphoreKHR(VkDevice device, const VkSemaphoreSignalInfoKHR* pSignalInfo) const;
    bool PreCallValidateImportSemaphoreFdKHR(VkDevice device, const VkImportSemaphoreFdInfoKHR* pImportSemaphoreFdInfo) const;
#ifdef VK_USE_PLATFORM_WIN32_KHR
    bool PreCallValidateImportSemaphoreWin32HandleKHR(
        VkDevice device, const VkImportSemaphoreWin32HandleInfoKHR* pImportSemaphoreWin32HandleInfo) const;
    bool PreCallValidateImportFenceWin32HandleKHR(VkDevice device,
                                                  const VkImportFenceWin32HandleInfoKHR* pImportFenceWin32HandleInfo) const;
#endif  // VK_USE_PLATFORM_WIN32_KHR
    bool PreCallValidateImportFenceFdKHR(VkDevice device, const VkImportFenceFdInfoKHR* pImportFenceFdInfo) const;

    bool PreCallValidateCreateSwapchainKHR(VkDevice device, const VkSwapchainCreateInfoKHR* pCreateInfo,
                                           const VkAllocationCallbacks* pAllocator, VkSwapchainKHR* pSwapchain) const;
    void PreCallRecordDestroySwapchainKHR(VkDevice device, VkSwapchainKHR swapchain, const VkAllocationCallbacks* pAllocator);
    bool PreCallValidateGetSwapchainImagesKHR(VkDevice device, VkSwapchainKHR swapchain, uint32_t* pSwapchainImageCount,
                                              VkImage* pSwapchainImages) const;
    void PostCallRecordGetSwapchainImagesKHR(VkDevice device, VkSwapchainKHR swapchain, uint32_t* pSwapchainImageCount,
                                             VkImage* pSwapchainImages, VkResult result);
    bool PreCallValidateQueuePresentKHR(VkQueue queue, const VkPresentInfoKHR* pPresentInfo) const;
    bool PreCallValidateCreateSharedSwapchainsKHR(VkDevice device, uint32_t swapchainCount,
                                                  const VkSwapchainCreateInfoKHR* pCreateInfos,
                                                  const VkAllocationCallbacks* pAllocator, VkSwapchainKHR* pSwapchains) const;
    bool PreCallValidateAcquireNextImageKHR(VkDevice device, VkSwapchainKHR swapchain, uint64_t timeout, VkSemaphore semaphore,
                                            VkFence fence, uint32_t* pImageIndex) const;
    bool PreCallValidateAcquireNextImage2KHR(VkDevice device, const VkAcquireNextImageInfoKHR* pAcquireInfo,
                                             uint32_t* pImageIndex) const;
    bool PreCallValidateDestroySurfaceKHR(VkInstance instance, VkSurfaceKHR surface, const VkAllocationCallbacks* pAllocator) const;
    bool PreCallValidateGetPhysicalDeviceSurfaceSupportKHR(VkPhysicalDevice physicalDevice, uint32_t queueFamilyIndex,
                                                           VkSurfaceKHR surface, VkBool32* pSupported) const;
    bool PreCallValidateCreateDescriptorUpdateTemplate(VkDevice device, const VkDescriptorUpdateTemplateCreateInfoKHR* pCreateInfo,
                                                       const VkAllocationCallbacks* pAllocator,
                                                       VkDescriptorUpdateTemplateKHR* pDescriptorUpdateTemplate) const;
    bool PreCallValidateCreateDescriptorUpdateTemplateKHR(VkDevice device,
                                                          const VkDescriptorUpdateTemplateCreateInfoKHR* pCreateInfo,
                                                          const VkAllocationCallbacks* pAllocator,
                                                          VkDescriptorUpdateTemplateKHR* pDescriptorUpdateTemplate) const;
    bool PreCallValidateUpdateDescriptorSetWithTemplate(VkDevice device, VkDescriptorSet descriptorSet,
                                                        VkDescriptorUpdateTemplate descriptorUpdateTemplate,
                                                        const void* pData) const;
    bool PreCallValidateUpdateDescriptorSetWithTemplateKHR(VkDevice device, VkDescriptorSet descriptorSet,
                                                           VkDescriptorUpdateTemplateKHR descriptorUpdateTemplate,
                                                           const void* pData) const;

    bool PreCallValidateCmdPushDescriptorSetWithTemplateKHR(VkCommandBuffer commandBuffer,
                                                            VkDescriptorUpdateTemplateKHR descriptorUpdateTemplate,
                                                            VkPipelineLayout layout, uint32_t set, const void* pData) const;
    bool PreCallValidateGetDisplayPlaneSupportedDisplaysKHR(VkPhysicalDevice physicalDevice, uint32_t planeIndex,
                                                            uint32_t* pDisplayCount, VkDisplayKHR* pDisplays) const;
    bool PreCallValidateGetDisplayPlaneCapabilitiesKHR(VkPhysicalDevice physicalDevice, VkDisplayModeKHR mode, uint32_t planeIndex,
                                                       VkDisplayPlaneCapabilitiesKHR* pCapabilities) const;
    bool PreCallValidateGetDisplayPlaneCapabilities2KHR(VkPhysicalDevice physicalDevice,
                                                        const VkDisplayPlaneInfo2KHR* pDisplayPlaneInfo,
                                                        VkDisplayPlaneCapabilities2KHR* pCapabilities) const;
    bool PreCallValidateCmdDebugMarkerEndEXT(VkCommandBuffer commandBuffer) const;

    bool PreCallValidateCmdBeginQueryIndexedEXT(VkCommandBuffer commandBuffer, VkQueryPool queryPool, uint32_t query,
                                                VkQueryControlFlags flags, uint32_t index) const;
    void PreCallRecordCmdBeginQueryIndexedEXT(VkCommandBuffer commandBuffer, VkQueryPool queryPool, uint32_t query,
                                              VkQueryControlFlags flags, uint32_t index);
    bool PreCallValidateCmdEndQueryIndexedEXT(VkCommandBuffer commandBuffer, VkQueryPool queryPool, uint32_t query,
                                              uint32_t index) const;
    void PreCallRecordCmdEndQueryIndexedEXT(VkCommandBuffer commandBuffer, VkQueryPool queryPool, uint32_t query, uint32_t index);

    bool PreCallValidateCmdSetDiscardRectangleEXT(VkCommandBuffer commandBuffer, uint32_t firstDiscardRectangle,
                                                  uint32_t discardRectangleCount, const VkRect2D* pDiscardRectangles) const;
    bool PreCallValidateCmdSetSampleLocationsEXT(VkCommandBuffer commandBuffer,
                                                 const VkSampleLocationsInfoEXT* pSampleLocationsInfo) const;
    bool ValidateCmdDrawIndirectCount(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset, VkBuffer countBuffer,
                                      VkDeviceSize countBufferOffset, uint32_t maxDrawCount, uint32_t stride,
                                      const char* apiName) const;
    bool PreCallValidateCmdDrawIndirectCountKHR(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                VkBuffer countBuffer, VkDeviceSize countBufferOffset, uint32_t maxDrawCount,
                                                uint32_t stride) const;
    bool PreCallValidateCmdDrawIndirectCount(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                             VkBuffer countBuffer, VkDeviceSize countBufferOffset, uint32_t maxDrawCount,
                                             uint32_t stride) const;
    bool PreCallValidateCmdDrawIndirectByteCountEXT(VkCommandBuffer commandBuffer, uint32_t instanceCount, uint32_t firstInstance,
                                                    VkBuffer counterBuffer, VkDeviceSize counterBufferOffset,
                                                    uint32_t counterOffset, uint32_t vertexStride) const;
    bool PreCallValidateCmdDrawMeshTasksNV(VkCommandBuffer commandBuffer, uint32_t taskCount, uint32_t firstTask) const;
    bool PreCallValidateCmdDrawMeshTasksIndirectNV(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                   uint32_t drawCount, uint32_t stride) const;
    bool PreCallValidateCmdDrawMeshTasksIndirectCountNV(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                        VkBuffer countBuffer, VkDeviceSize countBufferOffset, uint32_t maxDrawCount,
                                                        uint32_t stride) const;
    bool ValidateGetBufferDeviceAddress(VkDevice device, const VkBufferDeviceAddressInfo* pInfo, const char* apiName) const;
    bool PreCallValidateGetBufferDeviceAddressEXT(VkDevice device, const VkBufferDeviceAddressInfoEXT* pInfo) const;
    bool PreCallValidateGetBufferDeviceAddress(VkDevice device, const VkBufferDeviceAddressInfo* pInfo) const;
    bool PreCallValidateGetBufferDeviceAddressKHR(VkDevice device, const VkBufferDeviceAddressInfoKHR* pInfo) const;
    bool PreCallValidateGetBufferOpaqueCaptureAddressKHR(VkDevice device, const VkBufferDeviceAddressInfoKHR* pInfo) const;
    bool PreCallValidateGetDeviceMemoryOpaqueCaptureAddressKHR(VkDevice device,
                                                               const VkDeviceMemoryOpaqueCaptureAddressInfoKHR* pInfo) const;
    bool PreCallValidateGetBufferOpaqueCaptureAddress(VkDevice device, const VkBufferDeviceAddressInfo* pInfo) const;
    bool PreCallValidateGetDeviceMemoryOpaqueCaptureAddress(VkDevice device,
                                                            const VkDeviceMemoryOpaqueCaptureAddressInfo* pInfo) const;
    bool ValidateCmdSetDeviceMask(VkCommandBuffer commandBuffer, uint32_t deviceMask, const char* func_name) const;
    bool PreCallValidateCmdSetDeviceMask(VkCommandBuffer commandBuffer, uint32_t deviceMask) const;
    bool PreCallValidateCmdSetDeviceMaskKHR(VkCommandBuffer commandBuffer, uint32_t deviceMask) const;
    bool ValidateGetSemaphoreCounterValue(VkDevice device, VkSemaphore sempahore, uint64_t* pValue, const char* apiName) const;
    bool PreCallValidateGetSemaphoreCounterValueKHR(VkDevice device, VkSemaphore sempahore, uint64_t* pValue) const;
    bool PreCallValidateGetSemaphoreCounterValue(VkDevice device, VkSemaphore sempahore, uint64_t* pValue) const;
    bool ValidateComputeWorkGroupSizes(const SHADER_MODULE_STATE* shader) const;

    bool ValidateQueryRange(VkDevice device, VkQueryPool queryPool, uint32_t totalCount, uint32_t firstQuery, uint32_t queryCount,
                            const char* vuid_badfirst, const char* vuid_badrange) const;
    bool ValidateResetQueryPool(VkDevice device, VkQueryPool queryPool, uint32_t firstQuery, uint32_t queryCount) const;
    bool PreCallValidateResetQueryPoolEXT(VkDevice device, VkQueryPool queryPool, uint32_t firstQuery, uint32_t queryCount) const;
    bool PreCallValidateResetQueryPool(VkDevice device, VkQueryPool queryPool, uint32_t firstQuery, uint32_t queryCount) const;
    bool ValidateComputeWorkGroupInvocations(CMD_BUFFER_STATE* cb_state, uint32_t groupCountX, uint32_t groupCountY,
                                             uint32_t groupCountZ);
    bool ValidateQueryPoolStride(const std::string& vuid_not_64, const std::string& vuid_64, const VkDeviceSize stride,
                                 const char* parameter_name, const uint64_t parameter_value, const VkQueryResultFlags flags) const;
    bool ValidateCmdDrawStrideWithStruct(VkCommandBuffer commandBuffer, const std::string& vuid, const uint32_t stride,
                                         const char* struct_name, const uint32_t struct_size) const;
    bool ValidateCmdDrawStrideWithBuffer(VkCommandBuffer commandBuffer, const std::string& vuid, const uint32_t stride,
                                         const char* struct_name, const uint32_t struct_size, const uint32_t drawCount,
                                         const VkDeviceSize offset, const BUFFER_STATE* buffer_state) const;
    bool PreCallValidateReleaseProfilingLockKHR(VkDevice device) const;
    bool PreCallValidateCmdSetCheckpointNV(VkCommandBuffer commandBuffer, const void* pCheckpointMarker) const;
    bool PreCallValidateCmdBindTransformFeedbackBuffersEXT(VkCommandBuffer commandBuffer, uint32_t firstBinding,
                                                           uint32_t bindingCount, const VkBuffer* pBuffers,
                                                           const VkDeviceSize* pOffsets, const VkDeviceSize* pSizes) const;
    bool PreCallValidateCmdBeginTransformFeedbackEXT(VkCommandBuffer commandBuffer, uint32_t firstCounterBuffer,
                                                     uint32_t counterBufferCount, const VkBuffer* pCounterBuffers,
                                                     const VkDeviceSize* pCounterBufferOffsets) const;
    bool PreCallValidateCmdEndTransformFeedbackEXT(VkCommandBuffer commandBuffer, uint32_t firstCounterBuffer,
                                                   uint32_t counterBufferCount, const VkBuffer* pCounterBuffers,
                                                   const VkDeviceSize* pCounterBufferOffsets) const;

    bool PreCallValidateWriteAccelerationStructuresPropertiesKHR(VkDevice device, uint32_t accelerationStructureCount,
                                                                 const VkAccelerationStructureKHR* pAccelerationStructures,
                                                                 VkQueryType queryType, size_t dataSize, void* pData,
                                                                 size_t stride) const;

    bool PreCallValidateCmdWriteAccelerationStructuresPropertiesKHR(VkCommandBuffer commandBuffer,
                                                                    uint32_t accelerationStructureCount,
                                                                    const VkAccelerationStructureKHR* pAccelerationStructures,
                                                                    VkQueryType queryType, VkQueryPool queryPool,
                                                                    uint32_t firstQuery) const;

    bool PreCallValidateGetRayTracingShaderGroupHandlesKHR(VkDevice device, VkPipeline pipeline, uint32_t firstGroup,
                                                           uint32_t groupCount, size_t dataSize, void* pData) const;

    bool PreCallValidateGetRayTracingCaptureReplayShaderGroupHandlesKHR(VkDevice device, VkPipeline pipeline, uint32_t firstGroup,
                                                                        uint32_t groupCount, size_t dataSize, void* pData) const;

    bool PreCallValidateCmdBuildAccelerationStructureIndirectKHR(VkCommandBuffer commandBuffer,
                                                                 const VkAccelerationStructureBuildGeometryInfoKHR* pInfo,
                                                                 VkBuffer indirectBuffer, VkDeviceSize indirectOffset,
                                                                 uint32_t indirectStride) const;
    bool ValidateCopyAccelerationStructureInfoKHR(const VkCopyAccelerationStructureInfoKHR* pInfo, const char* api_name) const;
    bool PreCallValidateCmdCopyAccelerationStructureKHR(VkCommandBuffer commandBuffer,
                                                        const VkCopyAccelerationStructureInfoKHR* pInfo) const;
    bool PreCallValidateCopyAccelerationStructureKHR(VkDevice device, const VkCopyAccelerationStructureInfoKHR* pInfo) const;
    bool PreCallValidateCmdCopyAccelerationStructureToMemoryKHR(VkCommandBuffer commandBuffer,
                                                                const VkCopyAccelerationStructureToMemoryInfoKHR* pInfo) const;
    bool PreCallValidateCmdCopyMemoryToAccelerationStructureKHR(VkCommandBuffer commandBuffer,
                                                                const VkCopyMemoryToAccelerationStructureInfoKHR* pInfo) const;

    bool PreCallValidateCmdSetCullModeEXT(VkCommandBuffer commandBuffer, VkCullModeFlags cullMode) const;
    bool PreCallValidateCmdSetFrontFaceEXT(VkCommandBuffer commandBuffer, VkFrontFace frontFace) const;
    bool PreCallValidateCmdSetPrimitiveTopologyEXT(VkCommandBuffer commandBuffer, VkPrimitiveTopology primitiveTopology) const;
    bool PreCallValidateCmdSetViewportWithCountEXT(VkCommandBuffer commandBuffer, uint32_t viewportCount,
                                                   const VkViewport* pViewports) const;
    bool PreCallValidateCmdSetScissorWithCountEXT(VkCommandBuffer commandBuffer, uint32_t scissorCount,
                                                  const VkRect2D* pScissors) const;
    bool PreCallValidateCmdBindVertexBuffers2EXT(VkCommandBuffer commandBuffer, uint32_t firstBinding, uint32_t bindingCount,
                                                 const VkBuffer* pBuffers, const VkDeviceSize* pOffsets, const VkDeviceSize* pSizes,
                                                 const VkDeviceSize* pStrides) const;
    bool PreCallValidateCmdSetDepthTestEnableEXT(VkCommandBuffer commandBuffer, VkBool32 depthTestEnable) const;
    bool PreCallValidateCmdSetDepthWriteEnableEXT(VkCommandBuffer commandBuffer, VkBool32 depthWriteEnable) const;
    bool PreCallValidateCmdSetDepthCompareOpEXT(VkCommandBuffer commandBuffer, VkCompareOp depthCompareOp) const;
    bool PreCallValidateCmdSetDepthBoundsTestEnableEXT(VkCommandBuffer commandBuffer, VkBool32 depthBoundsTestEnable) const;
    bool PreCallValidateCmdSetStencilTestEnableEXT(VkCommandBuffer commandBuffer, VkBool32 stencilTestEnable) const;
    bool PreCallValidateCmdSetStencilOpEXT(VkCommandBuffer commandBuffer, VkStencilFaceFlags faceMask, VkStencilOp failOp,
                                           VkStencilOp passOp, VkStencilOp depthFailOp, VkCompareOp compareOp) const;

#ifdef VK_USE_PLATFORM_ANDROID_KHR
    bool PreCallValidateGetAndroidHardwareBufferPropertiesANDROID(VkDevice device, const struct AHardwareBuffer* buffer,
                                                                  VkAndroidHardwareBufferPropertiesANDROID* pProperties) const;
    bool PreCallValidateGetMemoryAndroidHardwareBufferANDROID(VkDevice device,
                                                              const VkMemoryGetAndroidHardwareBufferInfoANDROID* pInfo,
                                                              struct AHardwareBuffer** pBuffer) const;
#endif  // VK_USE_PLATFORM_ANDROID_KHR
#ifdef VK_USE_PLATFORM_WAYLAND_KHR
    bool PreCallValidateGetPhysicalDeviceWaylandPresentationSupportKHR(VkPhysicalDevice physicalDevice, uint32_t queueFamilyIndex,
                                                                       struct wl_display* display) const;
#endif  // VK_USE_PLATFORM_WAYLAND_KHR
#ifdef VK_USE_PLATFORM_WIN32_KHR
    bool PreCallValidateGetPhysicalDeviceWin32PresentationSupportKHR(VkPhysicalDevice physicalDevice,
                                                                     uint32_t queueFamilyIndex) const;
#endif  // VK_USE_PLATFORM_WIN32_KHR
#ifdef VK_USE_PLATFORM_XCB_KHR
    bool PreCallValidateGetPhysicalDeviceXcbPresentationSupportKHR(VkPhysicalDevice physicalDevice, uint32_t queueFamilyIndex,
                                                                   xcb_connection_t* connection, xcb_visualid_t visual_id) const;
#endif  // VK_USE_PLATFORM_XCB_KHR
#ifdef VK_USE_PLATFORM_XLIB_KHR
    bool PreCallValidateGetPhysicalDeviceXlibPresentationSupportKHR(VkPhysicalDevice physicalDevice, uint32_t queueFamilyIndex,
                                                                    Display* dpy, VisualID visualID) const;
#endif  // VK_USE_PLATFORM_XLIB_KHR

};  // Class CoreChecks

// Utility type for ForRange callbacks
struct LayoutUseCheckAndMessage {
    const static VkImageAspectFlags kDepthOrStencil = VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
    const ImageSubresourceLayoutMap* layout_map;
    const VkImageAspectFlags aspect_mask;
    const char* message;
    VkImageLayout layout;

    LayoutUseCheckAndMessage() = delete;
    LayoutUseCheckAndMessage(const ImageSubresourceLayoutMap* layout_map_, const VkImageAspectFlags aspect_mask_ = 0)
        : layout_map(layout_map_), aspect_mask{aspect_mask_}, message(nullptr), layout(kInvalidLayout) {}
    bool Check(const VkImageSubresource& subres, VkImageLayout check, VkImageLayout current_layout, VkImageLayout initial_layout) {
        message = nullptr;
        layout = kInvalidLayout;  // Success status
        if (current_layout != kInvalidLayout && !ImageLayoutMatches(aspect_mask, check, current_layout)) {
            message = "previous known";
            layout = current_layout;
        } else if ((initial_layout != kInvalidLayout) && !ImageLayoutMatches(aspect_mask, check, initial_layout)) {
            // To check the relaxed rule matching we need to see how the initial use was used
            const auto initial_layout_state = layout_map->GetSubresourceInitialLayoutState(subres);
            assert(initial_layout_state);  // If we have an initial layout, we better have a state for it
            if (!((initial_layout_state->aspect_mask & kDepthOrStencil) &&
                  ImageLayoutMatches(initial_layout_state->aspect_mask, check, initial_layout))) {
                message = "previously used";
                layout = initial_layout;
            }
        }
        return layout == kInvalidLayout;
    }
};
