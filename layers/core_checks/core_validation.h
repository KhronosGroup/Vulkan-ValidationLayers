/* Copyright (c) 2015-2024 The Khronos Group Inc.
 * Copyright (c) 2015-2024 Valve Corporation
 * Copyright (c) 2015-2024 LunarG, Inc.
 * Copyright (C) 2015-2024 Google Inc.
 * Modifications Copyright (C) 2020 Advanced Micro Devices, Inc. All rights reserved.
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

#include "state_tracker/state_tracker.h"
#include "state_tracker/image_layout_map.h"
#include "gpu_validation/gpu_validation.h"
#include "drawdispatch/drawdispatch_vuids.h"
#include "error_message/error_location.h"
#include "error_message/record_object.h"
#include "containers/qfo_transfer.h"
#include "state_tracker/cmd_buffer_state.h"
#include "state_tracker/render_pass_state.h"
#include "state_tracker/video_session_state.h"
#include "state_tracker/shader_object_state.h"
#include "sync/sync_utils.h"
#include "sync/sync_vuid_maps.h"

struct ValidateBeginQueryVuids {
    const char* vuid_queue_feedback = kVUIDUndefined;
    const char* vuid_queue_occlusion = kVUIDUndefined;
    const char* vuid_precise = kVUIDUndefined;
    const char* vuid_query_count = kVUIDUndefined;
    const char* vuid_profile_lock = kVUIDUndefined;
    const char* vuid_scope_not_first = kVUIDUndefined;
    const char* vuid_scope_in_rp = kVUIDUndefined;
    const char* vuid_dup_query_type = kVUIDUndefined;
    const char* vuid_protected_cb = kVUIDUndefined;
    const char* vuid_multiview_query = kVUIDUndefined;
    const char* vuid_graphics_support = kVUIDUndefined;
    const char* vuid_compute_support = kVUIDUndefined;
    const char* vuid_primitives_generated = kVUIDUndefined;
    const char* vuid_result_status_support = kVUIDUndefined;
    const char* vuid_performance_queue_index_07289 = kVUIDUndefined;
    const char* vuid_no_active_in_vc_scope = kVUIDUndefined;
};

struct ValidateEndQueryVuids {
    const char* vuid_active_queries = kVUIDUndefined;
    const char* vuid_protected_cb = kVUIDUndefined;
    const char* vuid_multiview_query = kVUIDUndefined;
    const char* vuid_inside_renderpass_07007 = kVUIDUndefined;
};

struct SubresourceRangeErrorCodes {
    const char *base_mip_err, *mip_count_err, *base_layer_err, *layer_count_err;
};

typedef vvl::unordered_map<const vvl::Image*, std::optional<GlobalImageLayoutRangeMap>> GlobalImageLayoutMap;

// Much of the data stored in vvl::CommandBuffer is only used by core validation, and is
// set up by Record calls in class CoreChecks. Because both the state tracker and
// core methods must lock vvl::CommandBuffer, it is possible for a Validate call to
// 'interrupt' a Record call and get only the state updated by whichever code
// locked and unlocked the CB first. This can only happen if the application
// is violating section 3.6 'Threading Behavior' of the specification, which
// requires that command buffers be externally synchronized. Still, we'd prefer
// not to crash if that happens. In most cases the core Record method is operating
// on separate data members from the state tracker. But in the case of vkCmdWaitEvents*,
// both methods operate on the same state in ways that could very easily crash if
// not done within the same lock guard. Overriding RecordWaitEvents() allows
// this to all happen completely while the state tracker is holding the lock.
// Eventually we'll probably want to move all of the core state into this derived
// class.
class CORE_CMD_BUFFER_STATE : public vvl::CommandBuffer {
  public:
    CORE_CMD_BUFFER_STATE(ValidationStateTracker* dev_data, VkCommandBuffer cb, const VkCommandBufferAllocateInfo* pCreateInfo,
                          const vvl::CommandPool* cmd_pool)
        : vvl::CommandBuffer(dev_data, cb, pCreateInfo, cmd_pool) {}

    void RecordWaitEvents(vvl::Func command, uint32_t eventCount, const VkEvent* pEvents,
                          VkPipelineStageFlags2KHR src_stage_mask) override;
};

struct TimelineMaxDiffCheck {
    TimelineMaxDiffCheck(uint64_t value_, uint64_t max_diff_) : value(value_), max_diff(max_diff_) {}

    // compute the differents between 2 timeline values, without rollover if the difference is greater than INT64_MAX
    uint64_t AbsDiff(uint64_t a, uint64_t b) { return a > b ? a - b : b - a; }

    bool operator()(const vvl::Semaphore::SemOp& op, bool is_pending) { return AbsDiff(value, op.payload) > max_diff; }

    uint64_t value;
    uint64_t max_diff;
};

class CoreChecks;
struct SemaphoreSubmitState {
    const CoreChecks* core;
    VkQueue queue;
    VkQueueFlags queue_flags;

    // This tracks how the payload of a binary semaphore changes **within the current submission**.
    // Before the first wait or signal no map entry for the semaphore is defined, which means that
    // semaphore's state is defined by the previous submissions on this queue or by the submissions on other queues.
    // After the first wait/signal the map starts tracking binary payload value: true - signaled, false - unsignaled.
    vvl::unordered_map<VkSemaphore, bool> binary_signaling_state;

    vvl::unordered_set<VkSemaphore> internal_semaphores;
    vvl::unordered_map<VkSemaphore, uint64_t> timeline_signals;
    vvl::unordered_map<VkSemaphore, uint64_t> timeline_waits;

    SemaphoreSubmitState(const CoreChecks* core_, VkQueue q_, VkQueueFlags queue_flags_)
        : core(core_), queue(q_), queue_flags(queue_flags_) {}

    bool CannotWaitBinary(const vvl::Semaphore& semaphore_state) const {
        assert(semaphore_state.type == VK_SEMAPHORE_TYPE_BINARY);
        const auto semaphore = semaphore_state.VkHandle();

        // Check if this submission has signaled or unsignaled the semaphore
        if (const auto it = binary_signaling_state.find(semaphore); it != binary_signaling_state.end()) {
            const bool signaled = it->second;
            return !signaled;  // not signaled => can't wait
        }
        // If not, then query semaphore's payload set by other submissions.
        // This will return either the payload set by the previous submissions on the current queue,
        // or the payload that is currently being updated by the async running queues.
        return !semaphore_state.CanBinaryBeWaited();
    }

    VkQueue AnotherQueueWaits(const vvl::Semaphore& semaphore_state) const {
        // spec (for 003871 but all submit functions have a similar VUID):
        // "When a semaphore wait operation for a binary semaphore is **executed**,
        // as defined by the semaphore member of any element of the pWaitSemaphoreInfos
        // member of any element of pSubmits, there must be no other queues waiting on the same semaphore"
        //
        // For binary semaphores there can be only 1 wait per signal so we just need to check that the
        // last operation isn't a wait. Prior waits will have been removed by prior signals by the time
        // this wait executes.
        auto last_op = semaphore_state.LastOp();
        if (last_op && !CanWaitBinarySemaphoreAfterOperation(last_op->op_type) && last_op->queue &&
            last_op->queue->VkHandle() != queue) {
            return last_op->queue->VkHandle();
        }
        return VK_NULL_HANDLE;
    }

    bool ValidateBinaryWait(const Location& loc, VkQueue queue, const vvl::Semaphore& semaphore_state);
    bool ValidateWaitSemaphore(const Location& wait_semaphore_loc, VkSemaphore semaphore, uint64_t value);
    bool ValidateSignalSemaphore(const Location& signal_semaphore_loc, VkSemaphore semaphore, uint64_t value);

    bool CannotSignalBinary(const vvl::Semaphore& semaphore_state, VkQueue& other_queue, vvl::Func& other_command) const {
        assert(semaphore_state.type == VK_SEMAPHORE_TYPE_BINARY);
        const auto semaphore = semaphore_state.VkHandle();

        // Check if this submission has signaled or unsignaled the semaphore
        if (const auto it = binary_signaling_state.find(semaphore); it != binary_signaling_state.end()) {
            const bool signaled = it->second;
            if (!signaled) {
                return false;  // not signaled => can't wait
            }
            other_queue = queue;
            other_command = vvl::Func::Empty;
            return true;  // signaled => can wait
        }
        // If not, get signaling state from the semaphore's last op.
        // Last op was recorded either by the previous sumbissions on this queue,
        // or it's a hot state from async running queues (so can get outdated immediately after was read).
        const auto last_op = semaphore_state.LastOp();
        if (!last_op || CanSignalBinarySemaphoreAfterOperation(last_op->op_type)) {
            return false;
        }
        other_queue = last_op->queue ? last_op->queue->VkHandle() : VK_NULL_HANDLE;
        other_command = last_op->command;
        return true;
    }

    bool CheckSemaphoreValue(const vvl::Semaphore& semaphore_state, std::string& where, uint64_t& bad_value,
                             std::function<bool(const vvl::Semaphore::SemOp&, bool is_pending)> compare_func) {
        auto current_signal = timeline_signals.find(semaphore_state.VkHandle());
        // NOTE: for purposes of validation, duplicate operations in the same submission are not yet pending.
        if (current_signal != timeline_signals.end()) {
            vvl::Semaphore::SemOp sig_op(vvl::Semaphore::kSignal, nullptr, 0, current_signal->second);
            if (compare_func(sig_op, false)) {
                where = "current submit's signal";
                bad_value = sig_op.payload;
                return true;
            }
        }
        auto current_wait = timeline_waits.find(semaphore_state.VkHandle());
        if (current_wait != timeline_waits.end()) {
            vvl::Semaphore::SemOp wait_op(vvl::Semaphore::kWait, nullptr, 0, current_wait->second);
            if (compare_func(wait_op, false)) {
                where = "current submit's wait";
                bad_value = wait_op.payload;
                return true;
            }
        }
        auto pending = semaphore_state.LastOp(compare_func);
        if (pending) {
            if (pending->payload == semaphore_state.Completed().payload) {
                where = "current";
            } else {
                where = pending->IsSignal() ? "pending signal" : "pending wait";
            }
            bad_value = pending->payload;
            return true;
        }
        return false;
    }
};

class CoreChecks : public ValidationStateTracker {
  public:
    using StateTracker = ValidationStateTracker;
    using Func = vvl::Func;
    using Struct = vvl::Struct;
    using Field = vvl::Field;
    using MemoryBarrier = sync_utils::MemoryBarrier;
    using QueueFamilyBarrier = sync_utils::QueueFamilyBarrier;
    using BufferBarrier = sync_utils::BufferBarrier;
    using ImageBarrier = sync_utils::ImageBarrier;

    GlobalQFOTransferBarrierMap<QFOImageTransferBarrier> qfo_release_image_barrier_map;
    GlobalQFOTransferBarrierMap<QFOBufferTransferBarrier> qfo_release_buffer_barrier_map;
    VkValidationCacheEXT core_validation_cache = VK_NULL_HANDLE;
    std::string validation_cache_path;

    CoreChecks() { container_type = LayerObjectTypeCoreValidation; }

    ReadLockGuard ReadLock() const override;
    WriteLockGuard WriteLock() override;

    bool ValidateSetMemBinding(VkDeviceMemory memory, const vvl::Bindable& mem_binding, const Location& loc) const;
    bool ValidateDeviceQueueFamily(uint32_t queue_family, const Location& loc, const char* vuid, bool optional) const;
    bool ValidateIdleDescriptorSet(VkDescriptorSet set, const Location& loc) const;
    bool ValidatePipelineLibraryFlags(const VkGraphicsPipelineLibraryFlagsEXT lib_flags,
                                      const VkPipelineLibraryCreateInfoKHR& link_info,
                                      const VkPipelineRenderingCreateInfo* rendering_struct, const Location& loc, int lib_index,
                                      const char* vuid) const;
    bool ValidateGraphicsPipelineDerivatives(std::vector<std::shared_ptr<vvl::Pipeline>> const& pipelines, uint32_t pipe_index,
                                             const Location& loc) const;
    bool ValidateGraphicsPipeline(const vvl::Pipeline& pipeline, const Location& create_info_loc) const;
    bool ValidImageBufferQueue(const vvl::CommandBuffer& cb_state, const VulkanTypedHandle& object, uint32_t queueFamilyIndex,
                               uint32_t count, const uint32_t* indices, const Location& loc) const;
    bool ValidateFenceForSubmit(const vvl::Fence* pFence, const char* inflight_vuid, const char* retired_vuid,
                                const LogObjectList& objlist, const Location& loc) const;
    bool ValidateSemaphoresForSubmit(struct SemaphoreSubmitState& state, const VkSubmitInfo& submit,
                                     const Location& submit_loc) const;
    bool ValidateSemaphoresForSubmit(struct SemaphoreSubmitState& state, const VkSubmitInfo2KHR& submit,
                                     const Location& submit_loc) const;
    bool ValidateSemaphoresForSubmit(struct SemaphoreSubmitState& state, const VkBindSparseInfo& submit,
                                     const Location& submit_loc) const;
    bool ValidateDynamicStateIsSet(CBDynamicFlags not_set_status, CBDynamicState dynamic_state, const LogObjectList& objlist,
                                   const Location& loc, const char* vuid) const;
    bool ValidateGraphicsDynamicStateSetStatus(const LastBound& last_bound_state, const Location& loc) const;
    bool ValidateDrawDynamicState(const LastBound& last_bound_state, const Location& loc) const;
    bool ValidateDrawDynamicStatePipeline(const LastBound& last_bound_state, const Location& loc) const;
    bool ValidateDrawDynamicStateShaderObject(const LastBound& last_bound_state, const Location& loc) const;
    bool ValidateRayTracingDynamicStateSetStatus(const LastBound& last_bound_state, const Location& loc) const;
    bool LogInvalidAttachmentMessage(const char* type1_string, const vvl::RenderPass& rp1_state, const char* type2_string,
                                     const vvl::RenderPass& rp2_state, uint32_t primary_attach, uint32_t secondary_attach,
                                     const char* msg, const Location& loc, const char* error_code) const;
    bool LogInvalidPnextMessage(const char* type1_string, const vvl::RenderPass& rp1_state, const char* type2_string,
                                const vvl::RenderPass& rp2_state, const char* msg, const Location& loc,
                                const char* error_code) const;
    bool LogInvalidDependencyMessage(const char* type1_string, const vvl::RenderPass& rp1_state, const char* type2_string,
                                     const vvl::RenderPass& rp2_state, const char* msg, const Location& loc,
                                     const char* error_code) const;
    bool ValidateStageMaskHost(const Location& stage_mask_loc, VkPipelineStageFlags2KHR stageMask) const;
    bool ValidateMapMemory(const vvl::DeviceMemory& mem_info, VkDeviceSize offset, VkDeviceSize size, const Location& offset_loc,
                           const Location& size_loc) const;
    bool ValidateRenderPassDAG(const VkRenderPassCreateInfo2* pCreateInfo, const ErrorObject& error_obj) const;
    bool ValidateAttachmentCompatibility(const char* type1_string, const vvl::RenderPass& rp1_state, const char* type2_string,
                                         const vvl::RenderPass& rp2_state, uint32_t primary_attach, uint32_t secondary_attach,
                                         const Location& loc, const char* error_code) const;
    bool ValidateSubpassCompatibility(const char* type1_string, const vvl::RenderPass& rp1_state, const char* type2_string,
                                      const vvl::RenderPass& rp2_state, const int subpass, const Location& loc,
                                      const char* error_code) const;
    bool ValidateDependencyCompatibility(const char* type1_string, const vvl::RenderPass& rp1_state, const char* type2_string,
                                         const vvl::RenderPass& rp2_state, const uint32_t dependency, const Location& loc,
                                         const char* error_code) const;
    bool ValidateRenderPassCompatibility(const char* type1_string, const vvl::RenderPass& rp1_state, const char* type2_string,
                                         const vvl::RenderPass& rp2_state, const Location& loc, const char* vuid) const;
    bool ReportInvalidCommandBuffer(const vvl::CommandBuffer& cb_state, const Location& loc, const char* vuid) const;
    bool ValidateQueueFamilyIndex(const vvl::PhysicalDevice* pd_state, uint32_t requested_queue_family, const char* vuid,
                                  const Location& loc) const;
    bool ValidateDeviceQueueCreateInfos(const vvl::PhysicalDevice* pd_state, uint32_t info_count,
                                        const VkDeviceQueueCreateInfo* infos, const Location& loc) const;

    bool ValidateProtectedImage(const vvl::CommandBuffer& cb_state, const vvl::Image& image_state, const Location& image_loc,
                                const char* vuid, const char* more_message = "") const override;
    bool ValidateUnprotectedImage(const vvl::CommandBuffer& cb_state, const vvl::Image& image_state, const Location& image_loc,
                                  const char* vuid, const char* more_message = "") const override;
    bool ValidateProtectedBuffer(const vvl::CommandBuffer& cb_state, const vvl::Buffer& buffer_state, const Location& buffer_loc,
                                 const char* vuid, const char* more_message = "") const override;
    bool ValidateUnprotectedBuffer(const vvl::CommandBuffer& cb_state, const vvl::Buffer& buffer_state, const Location& buffer_loc,
                                   const char* vuid, const char* more_message = "") const override;

    bool ValidateImageViewSampleWeightQCOM(const VkImageViewCreateInfo* pCreateInfo, const vvl::Image& image_state,
                                           const Location& loc) const;

    bool ValidatePipelineVertexDivisors(const safe_VkPipelineVertexInputStateCreateInfo& input_state,
                                        const std::vector<VkVertexInputBindingDescription>& binding_descriptions,
                                        const Location& loc) const;
    bool ValidatePipelineCacheControlFlags(VkPipelineCreateFlags2KHR flags, const Location& loc, const char* vuid) const;
    bool ValidatePipelineIndirectBindableFlags(VkPipelineCreateFlags2KHR flags, const Location& loc, const char* vuid) const;
    bool ValidatePipelineProtectedAccessFlags(VkPipelineCreateFlags2KHR flags, const Location& loc) const;
    void EnqueueSubmitTimeValidateImageBarrierAttachment(const Location& loc, vvl::CommandBuffer* cb_state,
                                                         const ImageBarrier& barrier);
    bool ValidateImageBarrierAttachment(const Location& barrier_loc, vvl::CommandBuffer const* cb_state,
                                        const vvl::Framebuffer* framebuffer, uint32_t active_subpass,
                                        const safe_VkSubpassDescription2& sub_desc, const VkRenderPass rp_handle,
                                        const ImageBarrier& img_barrier,
                                        const vvl::CommandBuffer* primary_cb_state = nullptr) const;

    static bool ValidateConcurrentBarrierAtSubmit(const Location& loc, const ValidationStateTracker& state_data,
                                                  const vvl::Queue& queue_data, const vvl::CommandBuffer& cb_state,
                                                  const VulkanTypedHandle& typed_handle, uint32_t src_queue_family,
                                                  uint32_t dst_queue_family);
    bool ValidateCmdBeginRenderPass(VkCommandBuffer commandBuffer, const VkRenderPassBeginInfo* pRenderPassBegin,
                                    const ErrorObject& error_obj) const;
    bool ValidateDependencies(const vvl::Framebuffer& framebuffer_state, const vvl::RenderPass& render_pass_state,
                              const ErrorObject& error_obj) const;
    bool ValidateBufferBarrier(const LogObjectList& objlist, const Location& barrier_loc, const vvl::CommandBuffer* cb_state,
                               const BufferBarrier& barrier) const;

    bool ValidateImageBarrier(const LogObjectList& objlist, const Location& barrier_loc, const vvl::CommandBuffer* cb_state,
                              const ImageBarrier& barrier) const;

    bool ValidateBarriers(const Location& loc, const vvl::CommandBuffer* cb_state, VkPipelineStageFlags src_stage_mask,
                          VkPipelineStageFlags dst_stage_mask, uint32_t memBarrierCount, const VkMemoryBarrier* pMemBarriers,
                          uint32_t bufferBarrierCount, const VkBufferMemoryBarrier* pBufferMemBarriers,
                          uint32_t imageMemBarrierCount, const VkImageMemoryBarrier* pImageMemBarriers) const;

    bool ValidateShaderTileImageBarriers(const LogObjectList& objlist, const Location& outer_loc,
                                         const VkDependencyInfo& dep_info) const;

    bool ValidateShaderTileImageBarriers(const LogObjectList& objlist, const Location& outer_loc,
                                         VkDependencyFlags dependency_flags, uint32_t memory_barrier_count,
                                         const VkMemoryBarrier* memory_barriers, uint32_t buffer_barrier_count,
                                         uint32_t image_barrier_count, VkPipelineStageFlags src_stage_mask,
                                         VkPipelineStageFlags dst_stage_mask) const;

    bool ValidateShaderTimeImageCommon(const LogObjectList& objlist, const Location& outer_loc,
                                       const std::string& barrier_error_vuid, VkDependencyFlags dependency_flags,
                                       uint32_t buffer_barrier_count, uint32_t image_barrier_count) const;

    bool ValidatePipelineStageFeatureEnables(const LogObjectList& objlist, const Location& stage_mask_loc,
                                             VkPipelineStageFlags2KHR stage_mask) const;
    bool ValidatePipelineStage(const LogObjectList& objlist, const Location& stage_mask_loc, VkQueueFlags queue_flags,
                               VkPipelineStageFlags2KHR stage_mask) const;
    bool ValidatePipelineStageForShaderTileImage(const LogObjectList& objlist, const Location& loc,
                                                 VkPipelineStageFlags2KHR stage_mask, const std::string& vuid) const;
    bool ValidateAccessMask(const LogObjectList& objlist, const Location& access_mask_loc, const Location& stage_mask_loc,
                            VkQueueFlags queue_flags, VkAccessFlags2KHR access_mask, VkPipelineStageFlags2KHR stage_mask) const;
    bool ValidateAccessMaskForShaderTileImage(const LogObjectList& objlist, const Location& loc, VkAccessFlags2KHR access_mask,
                                              const std::string& vuid) const;
    bool ValidateMemoryBarrier(const LogObjectList& objlist, const Location& barrier_loc, const vvl::CommandBuffer* cb_state,
                               const MemoryBarrier& barrier) const;

    bool ValidateSubpassDependency(const ErrorObject& error_obj, const Location& loc, const VkSubpassDependency2& barrier) const;

    bool ValidateDependencyInfo(const LogObjectList& objlist, const Location& dep_info_loc, const vvl::CommandBuffer* cb_state,
                                const VkDependencyInfoKHR* dep_info) const;

    bool ValidateBarrierQueueFamilies(const LogObjectList& objects, const Location& barrier_loc, const Location& field_loc,
                                      const QueueFamilyBarrier& barrier, const VulkanTypedHandle& handle,
                                      VkSharingMode sharing_mode) const;
    bool ValidateSwapchainPresentModesCreateInfo(VkPresentModeKHR present_mode, const Location& create_info_loc,
                                                 VkSwapchainCreateInfoKHR const* create_info,
                                                 const vvl::Surface* surface_state) const;
    bool ValidateSwapchainPresentScalingCreateInfo(VkPresentModeKHR present_mode, const Location& create_info_loc,
                                                   const VkSurfaceCapabilitiesKHR* capabilities,
                                                   VkSwapchainCreateInfoKHR const* create_info,
                                                   const vvl::Surface* surface_state) const;
    bool ValidateCreateSwapchain(VkSwapchainCreateInfoKHR const* pCreateInfo, const vvl::Surface* surface_state,
                                 const vvl::Swapchain* old_swapchain_state, const Location& create_info_loc) const;
    bool ValidateGraphicsPipelineBindPoint(const vvl::CommandBuffer* cb_state, const vvl::Pipeline& pipeline,
                                           const Location& loc) const;
    bool ValidatePipelineBindPoint(const vvl::CommandBuffer* cb_state, VkPipelineBindPoint bind_point, const Location& loc) const;
    bool ValidateMemoryIsMapped(uint32_t memoryRangeCount, const VkMappedMemoryRange* pMemoryRanges,
                                const ErrorObject& error_obj) const;
    bool ValidateMappedMemoryRangeDeviceLimits(uint32_t mem_range_count, const VkMappedMemoryRange* mem_ranges,
                                               const ErrorObject& error_obj) const;
    bool ValidateSecondaryCommandBufferState(const vvl::CommandBuffer& cb_state, const vvl::CommandBuffer& sub_cb_state,
                                             const Location& cb_loc) const;
    bool ValidateInheritanceInfoFramebuffer(VkCommandBuffer primaryBuffer, const vvl::CommandBuffer& cb_state,
                                            VkCommandBuffer secondaryBuffer, const vvl::CommandBuffer& sub_cb_state,
                                            const Location& loc) const;
    bool ValidateImportFence(VkFence fence, const char* vuid, const Location& loc) const;
    bool ValidateAcquireNextImage(VkDevice device, VkSwapchainKHR swapchain, uint64_t timeout, VkSemaphore semaphore, VkFence fence,
                                  uint32_t* pImageIndex, const Location& loc, const char* semaphore_type_vuid) const;
    bool VerifyRenderAreaBounds(const VkRenderPassBeginInfo* pRenderPassBegin, const Location& loc) const;
    bool VerifyFramebufferAndRenderPassImageViews(const VkRenderPassBeginInfo* pRenderPassBeginInfo, const Location& loc) const;
    bool ValidatePrimaryCommandBuffer(const vvl::CommandBuffer& cb_state, const Location& loc, const char* vuid) const;

    void RecordCmdNextSubpassLayouts(VkCommandBuffer commandBuffer, VkSubpassContents contents);
    bool ValidateCmdEndRenderPass(VkCommandBuffer commandBuffer, const VkSubpassEndInfo* pSubpassEndInfo,
                                  const ErrorObject& error_obj) const;
    void RecordCmdEndRenderPassLayouts(VkCommandBuffer commandBuffer);
    bool MatchUsage(uint32_t count, const VkAttachmentReference2* attachments, const VkFramebufferCreateInfo* fbci,
                    VkImageUsageFlagBits usage_flag, const char* vuid, const Location& create_info_loc) const;
    bool CheckDependencyExists(const VkRenderPass renderpass, const uint32_t subpass, const VkImageLayout layout,
                               const std::vector<SubpassLayout>& dependent_subpasses, const std::vector<DAGNode>& subpass_to_node,
                               const Location& attachment_loc, bool& skip) const;
    bool CheckPreserved(const VkRenderPass renderpass, const VkRenderPassCreateInfo2* pCreateInfo, const int index,
                        const uint32_t attachment, const std::vector<DAGNode>& subpass_to_node, int depth,
                        const Location& attachment_loc, bool& skip) const;
    bool ValidateBindImageMemory(uint32_t bindInfoCount, const VkBindImageMemoryInfo* pBindInfos,
                                 const ErrorObject& error_obj) const;
    bool ValidateGetPhysicalDeviceDisplayPlanePropertiesKHRQuery(VkPhysicalDevice physicalDevice, uint32_t planeIndex,
                                                                 const Location& loc) const;
    static bool VerifyQueryIsReset(const vvl::CommandBuffer& cb_state, const QueryObject& query_obj, Func command,
                                   VkQueryPool& firstPerfQueryPool, uint32_t perfPass, QueryMap* localQueryToStateMap);
    static bool ValidatePerformanceQuery(const vvl::CommandBuffer& cb_state, const QueryObject& query_obj, Func command,
                                         VkQueryPool& firstPerfQueryPool, uint32_t perfPass, QueryMap* localQueryToStateMap);
    bool ValidateBeginQuery(const vvl::CommandBuffer& cb_state, const QueryObject& query_obj, VkQueryControlFlags flags,
                            uint32_t index, const Location& loc, const ValidateBeginQueryVuids* vuids) const;
    bool ValidateCmdEndQuery(const vvl::CommandBuffer& cb_state, VkQueryPool queryPool, uint32_t slot, uint32_t index,
                             const Location& loc, const ValidateEndQueryVuids* vuids) const;

    bool ValidateCmdDrawInstance(const vvl::CommandBuffer& cb_state, uint32_t instanceCount, uint32_t firstInstance,
                                 const Location& loc) const;
    bool ValidateGraphicsIndexedCmd(const vvl::CommandBuffer& cb_state, const Location& loc) const;
    bool ValidateCmdNextSubpass(VkCommandBuffer commandBuffer, const ErrorObject& error_obj) const;
    bool ValidateInsertMemoryRange(const VulkanTypedHandle& typed_handle, const vvl::DeviceMemory* mem_info,
                                   VkDeviceSize memoryOffset, const Location& loc) const;
    bool ValidateInsertImageMemoryRange(VkImage image, const vvl::DeviceMemory* mem_info, VkDeviceSize mem_offset,
                                        const Location& loc) const;
    bool ValidateInsertBufferMemoryRange(VkBuffer buffer, const vvl::DeviceMemory* mem_info, VkDeviceSize mem_offset,
                                         const Location& loc) const;
    bool ValidateInsertAccelerationStructureMemoryRange(VkAccelerationStructureNV as, const vvl::DeviceMemory* mem_info,
                                                        VkDeviceSize mem_offset, const Location& loc) const;

    bool ValidateMemoryTypes(const vvl::DeviceMemory* mem_info, const uint32_t memory_type_bits, const Location& resource_loc,
                             const char* vuid) const;
    bool ValidateCommandBufferState(const vvl::CommandBuffer& cb_state, const Location& loc, uint32_t current_submit_count,
                                    const char* vuid) const;
    bool ValidateCommandBufferSimultaneousUse(const Location& loc, const vvl::CommandBuffer& cb_state,
                                              int current_submit_count) const;
    bool ValidateAttachmentReference(VkAttachmentReference2 reference, const VkFormat attachment_format, bool input,
                                     const Location& loc) const;
    bool ValidateRenderpassAttachmentUsage(const VkRenderPassCreateInfo2* pCreateInfo, const ErrorObject& error_obj) const;
    bool AddAttachmentUse(std::vector<uint8_t>& attachment_uses, std::vector<VkImageLayout>& attachment_layouts,
                          uint32_t attachment, uint8_t new_use, VkImageLayout new_layout, const Location loc) const;
    bool ValidateAttachmentIndex(uint32_t attachment, uint32_t attachment_count, const Location& loc) const;
    bool ValidateCreateRenderPass(const VkRenderPassCreateInfo2* pCreateInfo, const ErrorObject& error_obj) const;

    bool ValidateRenderPassPipelineBarriers(const Location& loc, const vvl::CommandBuffer* cb_state,
                                            VkPipelineStageFlags src_stage_mask, VkPipelineStageFlags dst_stage_mask,
                                            VkDependencyFlags dependency_flags, uint32_t mem_barrier_count,
                                            const VkMemoryBarrier* mem_barriers, uint32_t buffer_mem_barrier_count,
                                            const VkBufferMemoryBarrier* buffer_mem_barriers, uint32_t image_mem_barrier_count,
                                            const VkImageMemoryBarrier* image_barriers) const;
    bool ValidateRenderPassPipelineBarriers(const Location& loc, const vvl::CommandBuffer* cb_state,
                                            const VkDependencyInfoKHR* dep_info) const;

    bool ValidateStageMasksAgainstQueueCapabilities(const LogObjectList& objlist, const Location& stage_mask_loc,
                                                    VkQueueFlags queue_flags, VkPipelineStageFlags2KHR stage_mask) const;
    template <typename HandleT>
    bool ValidateMemoryIsBoundToBuffer(HandleT handle, const vvl::Buffer& buffer_state, const Location& buffer_loc,
                                       const char* vuid) const {
        bool result = false;
        if (!buffer_state.sparse) {
            const LogObjectList objlist(handle, buffer_state.Handle());
            result |= VerifyBoundMemoryIsValid(buffer_state.MemState(), objlist, buffer_state.Handle(), buffer_loc, vuid);
        }
        return result;
    }

    bool ValidateHostVisibleMemoryIsBoundToBuffer(const vvl::Buffer&, const Location& buffer_loc, const char* vuid) const;

    bool ValidateMemoryIsBoundToImage(const LogObjectList& objlist, const vvl::Image& image_state, const Location& loc,
                                      const char* vuid) const;

    bool ValidateObjectNotInUse(const vvl::StateObject* obj_node, const Location& loc, const char* error_code) const;
    bool ValidateCmdQueueFlags(const vvl::CommandBuffer& cb_state, const Location& loc, VkQueueFlags flags, const char* vuid,
                               const char* extra_message = "") const;
    bool ValidateSampleLocationsInfo(const VkSampleLocationsInfoEXT* pSampleLocationsInfo, const Location& loc) const;
    bool MatchSampleLocationsInfo(const VkSampleLocationsInfoEXT* pSampleLocationsInfo1,
                                  const VkSampleLocationsInfoEXT* pSampleLocationsInfo2) const;
    bool InsideRenderPass(const vvl::CommandBuffer& cb_state, const Location& loc, const char* vuid) const;
    bool OutsideRenderPass(const vvl::CommandBuffer& cb_state, const Location& loc, const char* vuid) const;
    bool InsideVideoCodingScope(const vvl::CommandBuffer& cb_state, const Location& loc, const char* vuid) const;
    bool OutsideVideoCodingScope(const vvl::CommandBuffer& cb_state, const Location& loc, const char* vuid) const;
    std::vector<VkVideoFormatPropertiesKHR> GetVideoFormatProperties(VkImageUsageFlags image_usage,
                                                                     const VkVideoProfileListInfoKHR* profile_list) const;
    std::vector<VkVideoFormatPropertiesKHR> GetVideoFormatProperties(VkImageUsageFlags image_usage,
                                                                     const VkVideoProfileInfoKHR* profile) const;
    bool IsVideoFormatSupported(VkFormat format, VkImageUsageFlags image_usage, const VkVideoProfileInfoKHR* profile) const;
    bool IsBufferCompatibleWithVideoProfile(const vvl::Buffer& buffer_state,
                                            const std::shared_ptr<const vvl::VideoProfileDesc>& video_profile) const;
    bool IsImageCompatibleWithVideoProfile(const vvl::Image& image_state,
                                           const std::shared_ptr<const vvl::VideoProfileDesc>& video_profile) const;
    void EnqueueVerifyVideoSessionInitialized(vvl::CommandBuffer& cb_state, vvl::VideoSession& vs_state, const char* vuid);
    void EnqueueVerifyVideoInlineQueryUnavailable(vvl::CommandBuffer& cb_state, const VkVideoInlineQueryInfoKHR& query_info,
                                                  Func command);
    bool ValidateVideoInlineQueryInfo(const vvl::QueryPool& query_pool_state, const VkVideoInlineQueryInfoKHR& query_info,
                                      const Location& loc) const;
    bool ValidateVideoEncodeRateControlInfo(const VkVideoEncodeRateControlInfoKHR& rc_info, const void* pNext,
                                            VkCommandBuffer cmdbuf, const vvl::VideoSession& vs_state, const Location& loc) const;
    bool ValidateVideoEncodeRateControlInfoH264(const VkVideoEncodeRateControlInfoKHR& rc_info, const void* pNext,
                                                VkCommandBuffer cmdbuf, const vvl::VideoSession& vs_state,
                                                const Location& loc) const;
    bool ValidateVideoEncodeRateControlInfoH265(const VkVideoEncodeRateControlInfoKHR& rc_info, const void* pNext,
                                                VkCommandBuffer cmdbuf, const vvl::VideoSession& vs_state,
                                                const Location& loc) const;
    bool ValidateVideoEncodeRateControlLayerInfo(uint32_t layer_index, const VkVideoEncodeRateControlInfoKHR& rc_info,
                                                 const void* pNext, VkCommandBuffer cmdbuf, const vvl::VideoSession& vs_state,
                                                 const Location& rc_info_loc) const;
    template <typename RateControlLayerInfo>
    bool ValidateVideoEncodeRateControlH26xQp(VkCommandBuffer cmdbuf, const vvl::VideoSession& vs_state,
                                              const RateControlLayerInfo& rc_layer_info, const char* min_qp_range_vuid,
                                              const char* max_qp_range_vuid, int32_t min_qp, int32_t max_qp,
                                              const char* min_qp_per_pic_type_vuid, const char* max_qp_per_pic_type_vuid,
                                              bool qp_per_picture_type, const char* min_max_qp_compare_vuid,
                                              const Location& loc) const;
    bool ValidateVideoEncodeRateControlLayerInfoH264(uint32_t layer_index, const VkVideoEncodeRateControlInfoKHR& rc_info,
                                                     const void* pNext, VkCommandBuffer cmdbuf, const vvl::VideoSession& vs_state,
                                                     const Location& rc_layer_info_loc) const;
    bool ValidateVideoEncodeRateControlLayerInfoH265(uint32_t layer_index, const VkVideoEncodeRateControlInfoKHR& rc_info,
                                                     const void* pNext, VkCommandBuffer cmdbuf, const vvl::VideoSession& vs_state,
                                                     const Location& rc_layer_info_loc) const;
    bool ValidateVideoPictureResource(const vvl::VideoPictureResource& picture_resource, VkCommandBuffer cmdbuf,
                                      const vvl::VideoSession& vs_state, const Location& loc,
                                      const char* coded_offset_vuid = nullptr, const char* coded_extent_vuid = nullptr) const;
    template <typename HandleT>
    bool ValidateVideoProfileInfo(const VkVideoProfileInfoKHR* profile, const HandleT object, const Location& loc) const;
    template <typename HandleT>
    bool ValidateVideoProfileListInfo(const VkVideoProfileListInfoKHR* profile_list, const HandleT object, const Location& loc,
                                      bool expect_decode_profile, const char* missing_decode_profile_msg_code,
                                      bool expect_encode_profile, const char* missing_encode_profile_msg_code) const;
    bool ValidateDecodeH264ParametersAddInfo(const vvl::VideoSession& vs_state,
                                             const VkVideoDecodeH264SessionParametersAddInfoKHR* add_info, VkDevice device,
                                             const Location& loc,
                                             const VkVideoDecodeH264SessionParametersCreateInfoKHR* create_info = nullptr,
                                             const vvl::VideoSessionParameters* template_state = nullptr) const;
    bool ValidateDecodeH265ParametersAddInfo(const vvl::VideoSession& vs_state,
                                             const VkVideoDecodeH265SessionParametersAddInfoKHR* add_info, VkDevice device,
                                             const Location& loc,
                                             const VkVideoDecodeH265SessionParametersCreateInfoKHR* create_info = nullptr,
                                             const vvl::VideoSessionParameters* template_state = nullptr) const;
    bool ValidateEncodeH264ParametersAddInfo(const vvl::VideoSession& vs_state,
                                             const VkVideoEncodeH264SessionParametersAddInfoKHR* add_info, VkDevice device,
                                             const Location& loc,
                                             const VkVideoEncodeH264SessionParametersCreateInfoKHR* create_info = nullptr,
                                             const vvl::VideoSessionParameters* template_state = nullptr) const;
    bool ValidateEncodeH265ParametersAddInfo(const vvl::VideoSession& vs_state,
                                             const VkVideoEncodeH265SessionParametersAddInfoKHR* add_info, VkDevice device,
                                             const Location& loc,
                                             const VkVideoEncodeH265SessionParametersCreateInfoKHR* create_info = nullptr,
                                             const vvl::VideoSessionParameters* template_state = nullptr) const;
    bool ValidateVideoDecodeInfoH264(const vvl::CommandBuffer& cb_state, const VkVideoDecodeInfoKHR& decode_info,
                                     const Location& loc) const;
    bool ValidateVideoDecodeInfoH265(const vvl::CommandBuffer& cb_state, const VkVideoDecodeInfoKHR& decode_info,
                                     const Location& loc) const;
    bool ValidateVideoEncodeH264PicType(const vvl::VideoSession& vs_state, StdVideoH264PictureType pic_type,
                                        const char* where) const;
    bool ValidateVideoEncodeInfoH264(const vvl::CommandBuffer& cb_state, const VkVideoEncodeInfoKHR& encode_info,
                                     const Location& loc) const;
    bool ValidateVideoEncodeH265PicType(const vvl::VideoSession& vs_state, StdVideoH265PictureType pic_type,
                                        const char* where) const;
    bool ValidateVideoEncodeInfoH265(const vvl::CommandBuffer& cb_state, const VkVideoEncodeInfoKHR& encode_info,
                                     const Location& loc) const;
    bool ValidateActiveReferencePictureCount(const vvl::CommandBuffer& cb_state, const VkVideoDecodeInfoKHR& decode_info) const;
    bool ValidateActiveReferencePictureCount(const vvl::CommandBuffer& cb_state, const VkVideoEncodeInfoKHR& encode_info) const;
    bool ValidateReferencePictureUseCount(const vvl::CommandBuffer& cb_state, const VkVideoDecodeInfoKHR& decode_info) const;
    bool ValidateReferencePictureUseCount(const vvl::CommandBuffer& cb_state, const VkVideoEncodeInfoKHR& encode_info) const;
    template <typename HandleT>
    bool ValidateImageSampleCount(const HandleT handle, const vvl::Image& image_state, VkSampleCountFlagBits sample_count,
                                  const Location& loc, const std::string& vuid) const;
    template <typename RegionType>
    bool ValidateImageBufferCopyMemoryOverlap(const vvl::CommandBuffer& cb_state, uint32_t regionCount, const RegionType* pRegions,
                                              const vvl::Image& image_state, const vvl::Buffer& buffer_state, const Location& loc,
                                              bool image_to_buffer, bool is_2) const;
    bool ValidateCmdSubpassState(const vvl::CommandBuffer& cb_state, const Location& loc, const char* vuid) const;
    bool ValidateCmd(const vvl::CommandBuffer& cb_state, const Location& loc) const;
    bool ValidateIndirectCmd(const vvl::CommandBuffer& cb_state, const vvl::Buffer& buffer_state, const Location& loc) const;
    bool ValidateIndirectCountCmd(const vvl::CommandBuffer& cb_state, const vvl::Buffer& count_buffer_state,
                                  VkDeviceSize count_buffer_offset, const Location& loc) const;
    bool ValidateMultisampledRenderToSingleSampleView(VkCommandBuffer commandBuffer,
                                                      const std::shared_ptr<const vvl::ImageView>& image_view_state,
                                                      const VkMultisampledRenderToSingleSampledInfoEXT* msrtss_info,
                                                      const Location& attachment_loc, const Location& loc) const;

    bool ValidateDeviceMaskToPhysicalDeviceCount(uint32_t deviceMask, const LogObjectList& objlist, const Location loc,
                                                 const char* vuid) const;
    bool ValidateDeviceMaskToZero(uint32_t deviceMask, const LogObjectList& objlist, const Location loc, const char* vuid) const;
    bool ValidateDeviceMaskToCommandBuffer(const vvl::CommandBuffer& cb_state, uint32_t deviceMask, const LogObjectList& objlist,
                                           const Location loc, const char* vuid) const;
    bool ValidateDeviceMaskToRenderPass(const vvl::CommandBuffer& cb_state, uint32_t deviceMask, const Location loc,
                                        const char* vuid) const;

    bool ValidateDepthStencilResolve(const VkRenderPassCreateInfo2* pCreateInfo, const ErrorObject& error_obj) const;

    // Prototypes for CoreChecks accessor functions
    VkFormatProperties3KHR GetPDFormatProperties(const VkFormat format) const;
    const VkPhysicalDeviceMemoryProperties* GetPhysicalDeviceMemoryProperties();

    bool FormatRequiresYcbcrConversionExplicitly(const VkFormat format) const;

    // Checks conformance version if VK_KHR_driver_properties is enabled
    bool IsBeforeCtsVersion(uint32_t major, uint32_t minor, uint32_t subminor) const;

    template <typename TransferBarrier>
    bool ValidateQueuedQFOTransferBarriers(const vvl::CommandBuffer& cb_state,
                                           QFOTransferCBScoreboards<TransferBarrier>* scoreboards,
                                           const GlobalQFOTransferBarrierMap<TransferBarrier>& global_release_barriers,
                                           const Location& loc) const;
    bool ValidateQueuedQFOTransfers(const vvl::CommandBuffer& cb_state,
                                    QFOTransferCBScoreboards<QFOImageTransferBarrier>* qfo_image_scoreboards,
                                    QFOTransferCBScoreboards<QFOBufferTransferBarrier>* qfo_buffer_scoreboards,
                                    const Location& loc) const;

    template <typename Barrier, typename Scoreboard>
    bool ValidateAndUpdateQFOScoreboard(const debug_report_data* report_data, const vvl::CommandBuffer& cb_state,
                                        const char* operation, const Barrier& barrier, Scoreboard* scoreboard,
                                        const Location& loc) const;
    template <typename Barrier, typename TransferBarrier>
    void RecordBarrierValidationInfo(const Location& loc, vvl::CommandBuffer* cb_state, const Barrier& barrier,
                                     QFOTransferBarrierSets<TransferBarrier>& barrier_sets);

    template <typename Barrier, typename TransferBarrier>
    bool ValidateQFOTransferBarrierUniqueness(const Location& barrier_loc, const vvl::CommandBuffer* cb_state,
                                              const Barrier& barrier,
                                              const QFOTransferBarrierSets<TransferBarrier>& barrier_sets) const;

    bool ValidatePrimaryCommandBufferState(const Location& loc, const vvl::CommandBuffer& cb_state, uint32_t current_submit_count,
                                           QFOTransferCBScoreboards<QFOImageTransferBarrier>* qfo_image_scoreboards,
                                           QFOTransferCBScoreboards<QFOBufferTransferBarrier>* qfo_buffer_scoreboards) const;
    bool ValidatePipelineRenderpassDraw(const LastBound& last_bound_state, const Location& loc) const;
    bool ValidatePipelineDynamicRenderpassDraw(const LastBound& last_bound_state, const Location& loc) const;
    bool ValidatePipelineDrawtimeState(const LastBound& last_bound_state, const Location& loc) const;
    bool ValidateShaderObjectDrawtimeState(const LastBound& last_bound_state, const Location& loc) const;
    bool ValidateShaderObjectGraphicsDrawtimeState(const LastBound& last_bound_state, const Location& loc) const;
    bool ValidateActionState(const vvl::CommandBuffer& cb_state, const VkPipelineBindPoint bind_point, const Location& loc) const;
    static bool ValidateWaitEventsAtSubmit(vvl::Func command, const vvl::CommandBuffer& cb_state, size_t eventCount,
                                           size_t firstEventIndex, VkPipelineStageFlags2 sourceStageMask,
                                           const EventToStageMap& local_event_signal_info, VkQueue waiting_queue,
                                           const Location& loc);
    bool ValidateQueueFamilyIndices(const Location& loc, const vvl::CommandBuffer& cb_state, VkQueue queue) const;
    VkResult CoreLayerCreateValidationCacheEXT(VkDevice device, const VkValidationCacheCreateInfoEXT* pCreateInfo,
                                               const VkAllocationCallbacks* pAllocator, VkValidationCacheEXT* pValidationCache);
    void CoreLayerDestroyValidationCacheEXT(VkDevice device, VkValidationCacheEXT validationCache,
                                            const VkAllocationCallbacks* pAllocator) override;
    VkResult CoreLayerMergeValidationCachesEXT(VkDevice device, VkValidationCacheEXT dstCache, uint32_t srcCacheCount,
                                               const VkValidationCacheEXT* pSrcCaches) override;
    VkResult CoreLayerGetValidationCacheDataEXT(VkDevice device, VkValidationCacheEXT validationCache, size_t* pDataSize,
                                                void* pData) override;
    // For given bindings validate state at time of draw is correct, returning false on error and writing error details into string*
    bool ValidateDrawState(const vvl::DescriptorSet& descriptor_set, const BindingVariableMap& bindings,
                           const std::vector<uint32_t>& dynamic_offsets, const vvl::CommandBuffer& cb_state, const Location& loc,
                           const vvl::DrawDispatchVuid& vuids) const;

    bool VerifySetLayoutCompatibility(const vvl::DescriptorSetLayout& layout_dsl,
                                      const vvl::DescriptorSetLayout& bound_dsl, std::string& error_msg) const;

    bool VerifySetLayoutCompatibility(const vvl::DescriptorSet& descriptor_set,
                                      const std::vector<std::shared_ptr<vvl::DescriptorSetLayout const>>& set_layouts,
                                      const VulkanTypedHandle& handle, const uint32_t layoutIndex, std::string& errorMsg) const;

    bool VerifySetLayoutCompatibility(const vvl::PipelineLayout& layout_a, const vvl::PipelineLayout& layout_b,
                                      std::string& errorMsg) const;

    // Validate contents of a CopyUpdate
    using DescriptorSet = vvl::DescriptorSet;
    bool ValidateCopyUpdate(const VkCopyDescriptorSet& update, const Location& copy_loc) const;
    bool VerifyCopyUpdateContents(const VkCopyDescriptorSet& update, const DescriptorSet& src_set, VkDescriptorType src_type,
                                  uint32_t src_index, const DescriptorSet& dst_set, VkDescriptorType dst_type, uint32_t dst_index,
                                  const Location& copy_loc) const;
    bool VerifyUpdateConsistency(const DescriptorSet& set, uint32_t binding, uint32_t offset, uint32_t update_count,
                                 const char* vuid, const Location& set_loc) const;
    // Validate contents of a WriteUpdate
    bool ValidateWriteUpdate(const DescriptorSet& dst_set, const VkWriteDescriptorSet& update, const Location& write_loc,
                             bool push) const;
    bool VerifyWriteUpdateContents(const DescriptorSet& dst_set, const VkWriteDescriptorSet& update, const uint32_t index,
                                   const Location& write_loc, bool push) const;
    // Shared helper functions - These are useful because the shared sampler image descriptor type
    //  performs common functions with both sampler and image descriptors so they can share their common functions
    bool ValidateImageUpdate(VkImageView image_view, VkImageLayout image_layout, VkDescriptorType type,
                             const Location& image_info_loc) const;
    // Validate contents of a push descriptor update
    bool ValidatePushDescriptorsUpdate(const DescriptorSet& push_set, uint32_t descriptorWriteCount,
                                       const VkWriteDescriptorSet* pDescriptorWrites, const Location& loc) const;
    // Descriptor Set Validation Functions
    bool ValidateSampler(VkSampler) const;
    bool ValidateBufferUsage(const vvl::Buffer& buffer_state, VkDescriptorType type, const Location& buffer_loc) const;
    bool ValidateBufferUpdate(const VkDescriptorBufferInfo& buffer_info, VkDescriptorType type,
                              const Location& buffer_info_loc) const;
    bool ValidateUpdateDescriptorSets(uint32_t descriptorWriteCount, const VkWriteDescriptorSet* pDescriptorWrites, uint32_t descriptorCopyCount,
                                      const VkCopyDescriptorSet* pDescriptorCopies, const Location& loc) const;

    // Stuff from shader_validation
    bool ValidateGraphicsPipelineShaderState(const vvl::Pipeline& pipeline, const Location& create_info_loc) const;
    bool ValidateGraphicsPipelinePortability(const vvl::Pipeline& pipeline, const Location& create_info_loc) const;
    bool ValidatePipelineLibraryCreateInfo(const vvl::Pipeline& pipeline, const VkPipelineLibraryCreateInfoKHR& library_create_info,
                                           const Location& create_info_loc) const;
    bool ValidateGraphicsPipelineLibrary(const vvl::Pipeline& pipeline, const Location& create_info_loc) const;
    bool ValidateGraphicsPipelineShaderDynamicState(const vvl::Pipeline& pipeline, const vvl::CommandBuffer& cb_state,
                                                    const Location& loc, const vvl::DrawDispatchVuid& vuids) const;
    bool ValidateGraphicsPipelineBlendEnable(const vvl::Pipeline& pipeline, const Location& create_info_loc) const;
    bool ValidateGraphicsPipelinePreRasterState(const vvl::Pipeline& pipeline, const Location& create_info_loc) const;
    bool ValidateGraphicsPipelineInputAssemblyState(const vvl::Pipeline& pipeline, const Location& create_info_loc) const;
    bool ValidateGraphicsPipelineTessellationState(const vvl::Pipeline& pipeline, const Location& create_info_loc) const;
    bool ValidateGraphicsPipelineColorBlendState(const vvl::Pipeline& pipeline, const safe_VkSubpassDescription2* subpass_desc,
                                                 const Location& create_info_loc) const;
    bool ValidateGraphicsPipelineRasterizationState(const vvl::Pipeline& pipeline, const safe_VkSubpassDescription2* subpass_desc,
                                                    const Location& create_info_loc) const;
    bool ValidateGraphicsPipelineMultisampleState(const vvl::Pipeline& pipeline, const safe_VkSubpassDescription2* subpass_desc,
                                                  const Location& create_info_loc) const;
    bool ValidateGraphicsPipelineDepthStencilState(const vvl::Pipeline& pipeline, const Location& create_info_loc) const;
    bool ValidateGraphicsPipelineDynamicState(const vvl::Pipeline& pipeline, const Location& create_info_loc) const;
    bool ValidateGraphicsPipelineFragmentShadingRateState(const vvl::Pipeline& pipeline, const Location& create_info_loc) const;
    bool ValidateGraphicsPipelineDynamicRendering(const vvl::Pipeline& pipeline, const Location& create_info_loc) const;
    bool ValidateGraphicsPipelineExternalFormatResolve(const vvl::Pipeline& pipeline,
                                                       const safe_VkSubpassDescription2* subpass_desc,
                                                       const Location& create_info_loc) const;
    bool ValidateComputePipelineShaderState(const vvl::Pipeline& pipeline, const Location& create_info_loc) const;
    bool ValidatePipelineRobustnessCreateInfo(const vvl::Pipeline& pipeline, const VkPipelineRobustnessCreateInfoEXT& create_info,
                                              const Location& loc) const;
    uint32_t CalcShaderStageCount(const vvl::Pipeline& pipeline, VkShaderStageFlagBits stageBit) const;
    bool GroupHasValidIndex(const vvl::Pipeline& pipeline, uint32_t group, uint32_t stage) const;
    bool ValidateRayTracingPipeline(const vvl::Pipeline& pipeline, const safe_VkRayTracingPipelineCreateInfoCommon& create_info,
                                    VkPipelineCreateFlags flags, const Location& create_info_loc) const;
    bool PreCallValidateGetShaderModuleIdentifierEXT(VkDevice device, VkShaderModule shaderModule,
                                                     VkShaderModuleIdentifierEXT* pIdentifier,
                                                     const ErrorObject& error_obj) const override;
    bool PreCallValidateGetShaderModuleCreateInfoIdentifierEXT(VkDevice device, const VkShaderModuleCreateInfo* pCreateInfo,
                                                               VkShaderModuleIdentifierEXT* pIdentifier,
                                                               const ErrorObject& error_obj) const override;
    void PreCallRecordCreateShaderModule(VkDevice device, const VkShaderModuleCreateInfo* pCreateInfo,
                                         const VkAllocationCallbacks* pAllocator, VkShaderModule* pShaderModule,
                                         const RecordObject& record_obj, void* csm_state_data) override;
    void PreCallRecordCreateShadersEXT(VkDevice device, uint32_t createInfoCount, const VkShaderCreateInfoEXT* pCreateInfos,
                                       const VkAllocationCallbacks* pAllocator, VkShaderEXT* pShaders,
                                       const RecordObject& record_obj, void* csm_state_data) override;
    bool RunSpirvValidation(spv_const_binary_t& binary, const Location& loc) const;
    bool PreCallValidateCreateShaderModule(VkDevice device, const VkShaderModuleCreateInfo* pCreateInfo,
                                           const VkAllocationCallbacks* pAllocator, VkShaderModule* pShaderModule,
                                           const ErrorObject& error_obj) const override;
    bool ValidatePipelineShaderStage(const StageCreateInfo& stage_create_info, const PipelineStageState& stage_state,
                                     const Location& loc) const;
    bool ValidatePointSizeShaderState(const StageCreateInfo& create_info, const spirv::Module& module_state,
                                      const spirv::EntryPoint& entrypoint, VkShaderStageFlagBits stage, const Location& loc) const;
    bool ValidatePrimitiveRateShaderState(const StageCreateInfo& create_info, const spirv::Module& module_state,
                                          const spirv::EntryPoint& entrypoint, VkShaderStageFlagBits stage,
                                          const Location& loc) const;
    bool ValidateTexelOffsetLimits(const spirv::Module& module_state, const spirv::Instruction& insn, const Location& loc) const;

    // Auto-generated helper functions
    bool ValidateShaderCapabilitiesAndExtensions(const spirv::Instruction& insn, const bool pipeline, const Location& loc) const;
    VkFormat CompatibleSpirvImageFormat(uint32_t spirv_image_format) const;

    bool ValidateShaderStageInputOutputLimits(const spirv::Module& module_state, VkShaderStageFlagBits stage,
                                              const spirv::EntryPoint& entrypoint, const Location& loc) const;
    bool ValidateShaderStorageImageFormatsVariables(const spirv::Module& module_state, const spirv::Instruction* insn,
                                                    const Location& loc) const;
    bool ValidateShaderStageMaxResources(VkShaderStageFlagBits stage, const StageCreateInfo& create_info,
                                         const Location& loc) const;
    bool ValidateShaderStageGroupNonUniform(const spirv::Module& module_state, VkShaderStageFlagBits stage,
                                            const Location& loc) const;
    bool ValidateMemoryScope(const spirv::Module& module_state, const spirv::Instruction& insn, const Location& loc) const;
    bool ValidateCooperativeMatrix(const spirv::Module& module_state, const spirv::EntryPoint& entrypoint,
                                   const PipelineStageState& stage_state, const uint32_t local_size_x, const Location& loc) const;
    bool ValidateShaderResolveQCOM(const spirv::Module& module_state, VkShaderStageFlagBits stage,
                                   const StageCreateInfo& create_info, const Location& loc) const;
    bool ValidateShaderSubgroupSizeControl(const StageCreateInfo& stage_create_info, VkShaderStageFlagBits stage,
                                           const PipelineStageState& stage_state, const Location& loc) const;
    bool ValidateWorkgroupSharedMemory(const spirv::Module& module_state, VkShaderStageFlagBits stage,
                                       uint32_t total_workgroup_shared_memory, const Location& loc) const;
    bool ValidateShaderTileImage(const spirv::Module& module_state, const spirv::EntryPoint& entrypoint,
                                 const StageCreateInfo& create_info, const VkShaderStageFlagBits stage, const Location& loc) const;
    bool ValidateAtomicsTypes(const spirv::Module& module_state, const Location& loc) const;
    bool ValidateExecutionModes(const spirv::Module& module_state, const spirv::EntryPoint& entrypoint, VkShaderStageFlagBits stage,
                                const StageCreateInfo& create_info, const Location& loc) const;
    bool ValidateInterfaceVertexInput(const vvl::Pipeline& pipeline, const spirv::Module& module_state,
                                      const spirv::EntryPoint& entrypoint, const Location& create_info_loc) const;
    bool ValidateInterfaceFragmentOutput(const vvl::Pipeline& pipeline, const spirv::Module& module_state,
                                         const spirv::EntryPoint& entrypoint, const Location& create_info_loc) const;
    bool ValidateShaderInputAttachment(const spirv::Module& module_state, const vvl::Pipeline& pipeline,
                                       const spirv::ResourceInterfaceVariable& variable, const Location& loc) const;
    bool ValidateConservativeRasterization(const spirv::Module& module_state, const spirv::EntryPoint& entrypoint,
                                           const StageCreateInfo& stage_create_info, const Location& loc) const;
    bool ValidatePushConstantUsage(const StageCreateInfo& create_info, const spirv::Module& module_state,
                                   const spirv::EntryPoint& entrypoint, const Location& loc) const;
    bool ValidateBuiltinLimits(const spirv::Module& module_state, const spirv::EntryPoint& entrypoint,
                               const StageCreateInfo& create_info, const Location& loc) const;
    bool ValidatePrimitiveTopology(const spirv::Module& module_state, const spirv::EntryPoint& entrypoint,
                                   const StageCreateInfo& create_info, const Location& loc) const;
    bool ValidateSpecializations(const safe_VkSpecializationInfo* spec, const StageCreateInfo& create_info,
                                 const Location& loc) const;
    bool ValidateInterfaceBetweenStages(const spirv::Module& producer, const spirv::EntryPoint& producer_entrypoint,
                                        const spirv::Module& consumer, const spirv::EntryPoint& consumer_entrypoint,
                                        const Location& create_info_loc) const;
    bool ValidateFsOutputsAgainstRenderPass(const spirv::Module& module_state, const spirv::EntryPoint& entrypoint,
                                            const vvl::Pipeline& pipeline, uint32_t subpass_index,
                                            const Location& create_info_loc) const;
    bool ValidateFsOutputsAgainstDynamicRenderingRenderPass(const spirv::Module& module_state, const spirv::EntryPoint& entrypoint,
                                                            const vvl::Pipeline& pipeline, const Location& create_info_loc) const;
    bool ValidatePipelineTessellationStages(const spirv::Module& tesc_module_state, const spirv::EntryPoint& tesc_entrypoint,
                                            const spirv::Module& tese_module_state, const spirv::EntryPoint& tese_entrypoint,
                                            const Location& create_info_loc) const;
    bool ValidateVariables(const spirv::Module& module_state, const Location& loc) const;
    bool ValidateShaderDescriptorVariable(const spirv::Module& module_state, const StageCreateInfo& stage_create_info,
                                          const spirv::EntryPoint& entrypoint, const Location& loc) const;
    bool ValidateTransformFeedback(const spirv::Module& module_state, const spirv::EntryPoint& entrypoint,
                                   const StageCreateInfo& create_info, const Location& loc) const;
    bool ValidateTransformFeedbackDecorations(const spirv::Module& module_state, const StageCreateInfo& create_info,
                                              const Location& loc) const;
    virtual bool ValidateShaderModuleId(const vvl::Pipeline& pipeline, const Location& loc) const;
    bool ValidateShaderClock(const spirv::Module& module_state, const Location& loc) const;
    bool ValidateImageWrite(const spirv::Module& module_state, const Location& loc) const;

    template <typename RegionType>
    bool ValidateCopyImageTransferGranularityRequirements(const vvl::CommandBuffer& cb_state, const vvl::Image& src_image_state,
                                                          const vvl::Image& dst_image_state, const RegionType* region,
                                                          const Location& region_loc) const;
    bool ValidateImageSubresourceRange(const uint32_t image_mip_count, const uint32_t image_layer_count,
                                       const VkImageSubresourceRange& subresourceRange, const char* image_layer_count_var_name,
                                       const VkImage image, const SubresourceRangeErrorCodes& errorCodes,
                                       const Location& subresource_loc) const;
    bool ValidateMultipassRenderedToSingleSampledSampleCount(VkFramebuffer framebuffer, VkRenderPass renderpass,
                                                             vvl::Image* image_state, VkSampleCountFlagBits msrtss_samples,
                                                             const Location& rasterization_samples_loc) const;
    bool ValidateRenderPassLayoutAgainstFramebufferImageUsage(VkImageLayout layout, const vvl::ImageView& image_view_state,
                                                              VkFramebuffer framebuffer, VkRenderPass renderpass,
                                                              uint32_t attachment_index, const Location& rp_loc,
                                                              const Location& attachment_reference_loc) const;
    bool ValidateRenderPassStencilLayoutAgainstFramebufferImageUsage(VkImageLayout layout, const vvl::ImageView& image_view_state,
                                                                     VkFramebuffer framebuffer, VkRenderPass renderpass,
                                                                     const Location& layout_loc) const;
    bool ValidateHostCopyImageCreateInfos(VkDevice device, const vvl::Image& src_image_state, const vvl::Image& dst_image_state,
                                          const Location& loc) const;
    bool IsCompliantSubresourceRange(const VkImageSubresourceRange& subres_range, const vvl::Image& image_state) const;
    template <typename HandleT, typename RegionType>
    bool ValidateHeterogeneousCopyData(const HandleT handle, uint32_t regionCount, const RegionType* pRegions,
                                       const vvl::Image& image_state, const Location& loc) const;
    bool UsageHostTransferCheck(VkDevice device, const vvl::Image& image_state, bool has_stencil, bool has_non_stencil,
                                const char* vuid_09111, const char* vuid_09112, const char* vuid_09113, const Location& loc) const;
    template <typename InfoPointer>
    bool ValidateMemoryImageCopyCommon(VkDevice device, InfoPointer iPointer, const Location& loc) const;
    template <typename RegionType>
    bool ValidateBufferImageCopyData(const vvl::CommandBuffer& cb_state, uint32_t regionCount, const RegionType* pRegions,
                                     const vvl::Image& image_state, const Location& loc) const;
    bool ValidateHostCopyImageLayout(const VkDevice device, const VkImage image, const uint32_t layout_count,
                                     const VkImageLayout* supported_image_layouts, const VkImageLayout image_layout,
                                     const Location& loc, const char* supported_name, const char* vuid) const;
    bool ValidateMemcpyExtents(VkDevice device, const VkImageCopy2 region, const vvl::Image& image_state, bool is_src,
                               const Location& region_loc) const;
    bool ValidateHostCopyCurrentLayout(VkDevice device, VkImageLayout expected_layout,
                                       const VkImageSubresourceLayers& subres_layers, uint32_t region,
                                       const vvl::Image& image_state, const Location& loc, const char* image_label,
                                       const char* vuid) const;
    bool ValidateHostCopyCurrentLayout(VkDevice device, VkImageLayout expected_layout, const VkImageSubresourceRange& subres_range,
                                       uint32_t region, const vvl::Image& image_state, const Location& loc, const char* image_label,
                                       const char* vuid) const;
    bool ValidateHostCopyMultiplane(VkDevice device, VkImageCopy2 region, const vvl::Image& image_state, bool is_src,
                                    const Location& region_loc) const;
    bool ValidateBufferViewRange(const vvl::Buffer& buffer_state, const VkBufferViewCreateInfo* pCreateInfo,
                                 const Location& loc) const;
    bool ValidateBufferViewBuffer(const vvl::Buffer& buffer_state, const VkBufferViewCreateInfo* pCreateInfo,
                                  const Location& loc) const;

    bool ValidateImageFormatFeatures(const VkImageCreateInfo* pCreateInfo, const Location& loc) const;

    bool PreCallValidateCreateImage(VkDevice device, const VkImageCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator,
                                    VkImage* pImage, const ErrorObject& error_obj) const override;

    void PostCallRecordCreateImage(VkDevice device, const VkImageCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator,
                                   VkImage* pImage, const RecordObject& record_obj) override;

    void PreCallRecordDestroyImage(VkDevice device, VkImage image, const VkAllocationCallbacks* pAllocator,
                                   const RecordObject& record_obj) override;

    bool PreCallValidateDestroyImage(VkDevice device, VkImage image, const VkAllocationCallbacks* pAllocator,
                                     const ErrorObject& error_obj) const override;

    bool ValidateClearImageSubresourceRange(const vvl::CommandBuffer& cb_state, const vvl::Image& image_state,
                                            const VkImageSubresourceRange& range, const Location& loc) const;

    bool ValidateClearAttachmentExtent(const vvl::CommandBuffer& cb_state, const VkRect2D& render_area,
                                       uint32_t render_pass_layer_count, uint32_t rect_count, const VkClearRect* clear_rects,
                                       const Location& loc) const;

    template <typename HandleT, typename RegionType>
    bool ValidateImageCopyData(const HandleT handle, const uint32_t regionCount, const RegionType* pRegions,
                               const vvl::Image& src_image_state, const vvl::Image& dst_image_state, bool is_host,
                               const Location& loc) const;

    bool VerifyClearImageLayout(const vvl::CommandBuffer& cb_state, const vvl::Image& image_state,
                                const VkImageSubresourceRange& range, VkImageLayout dest_image_layout, const Location& loc) const;

    template <typename RangeFactory>
    bool VerifyImageLayoutRange(const vvl::CommandBuffer& cb_state, const vvl::Image& image_state, VkImageAspectFlags aspect_mask,
                                VkImageLayout explicit_layout, const RangeFactory& range_factory, const Location& image_loc,
                                const char* mismatch_layout_vuid, bool* error) const;

    bool VerifyImageLayoutSubresource(const vvl::CommandBuffer& cb_state, const vvl::Image& image_state,
                                      const VkImageSubresourceLayers& subLayers, VkImageLayout explicit_layout,
                                      const Location& image_loc, const char* vuid) const;

    bool VerifyImageLayout(const vvl::CommandBuffer& cb_state, const vvl::ImageView& image_view_state,
                           VkImageLayout explicit_layout, const Location& image_loc, const char* mismatch_layout_vuid,
                           bool* error) const override;

    bool VerifyImageLayout(const vvl::CommandBuffer& cb_state, const vvl::Image& image_state, const VkImageSubresourceRange& range,
                           VkImageLayout explicit_layout, const Location& image_loc, const char* mismatch_layout_vuid,
                           bool* error) const;

    bool CheckItgExtent(const LogObjectList& objlist, const VkExtent3D& extent, const VkOffset3D& offset,
                        const VkExtent3D& granularity, const VkExtent3D& subresource_extent, const VkImageType image_type,
                        const Location& extent_loc, const char* vuid) const;

    bool CheckItgOffset(const LogObjectList& objlist, const VkOffset3D& offset, const VkExtent3D& granularity,
                        const Location& offset_loc, const char* vuid) const;
    VkExtent3D GetScaledItg(const vvl::CommandBuffer& cb_state, const vvl::Image& image_state) const;

    bool PreCallValidateCmdClearColorImage(VkCommandBuffer commandBuffer, VkImage image, VkImageLayout imageLayout,
                                           const VkClearColorValue* pColor, uint32_t rangeCount,
                                           const VkImageSubresourceRange* pRanges, const ErrorObject& error_obj) const override;

    void PreCallRecordCmdClearColorImage(VkCommandBuffer commandBuffer, VkImage image, VkImageLayout imageLayout,
                                         const VkClearColorValue* pColor, uint32_t rangeCount,
                                         const VkImageSubresourceRange* pRanges, const RecordObject& record_obj) override;

    bool ValidateClearDepthStencilValue(VkCommandBuffer commandBuffer, VkClearDepthStencilValue clearValue,
                                        const Location& loc) const;

    bool PreCallValidateCmdClearDepthStencilImage(VkCommandBuffer commandBuffer, VkImage image, VkImageLayout imageLayout,
                                                  const VkClearDepthStencilValue* pDepthStencil, uint32_t rangeCount,
                                                  const VkImageSubresourceRange* pRanges,
                                                  const ErrorObject& error_obj) const override;

    void PreCallRecordCmdClearDepthStencilImage(VkCommandBuffer commandBuffer, VkImage image, VkImageLayout imageLayout,
                                                const VkClearDepthStencilValue* pDepthStencil, uint32_t rangeCount,
                                                const VkImageSubresourceRange* pRanges, const RecordObject& record_obj) override;

    bool FindLayouts(const vvl::Image& image_state, std::vector<VkImageLayout>& layouts) const;

    bool VerifyFramebufferAndRenderPassLayouts(const vvl::CommandBuffer& cb_state, const VkRenderPassBeginInfo* pRenderPassBegin,
                                               const vvl::Framebuffer& framebuffer_state, const Location& rp_begin_loc) const;
    void RecordCmdBeginRenderPassLayouts(VkCommandBuffer commandBuffer, const VkRenderPassBeginInfo* pRenderPassBegin,
                                         const VkSubpassContents contents);
    void TransitionAttachmentRefLayout(vvl::CommandBuffer* cb_state, const safe_VkAttachmentReference2& ref);

    void TransitionSubpassLayouts(vvl::CommandBuffer* cb_state, const vvl::RenderPass& render_pass_state, const int);

    void TransitionBeginRenderPassLayouts(vvl::CommandBuffer* cb_state, const vvl::RenderPass& render_pass_state);

    bool UpdateCommandBufferImageLayoutMap(const vvl::CommandBuffer* cb_state, const Location& image_loc,
                                           const ImageBarrier& img_barrier, const CommandBufferImageLayoutMap& current_map,
                                           CommandBufferImageLayoutMap& layout_updates) const;

    bool ValidateBarrierLayoutToImageUsage(const Location& layout_loc, VkImage image, VkImageLayout layout,
                                           VkImageUsageFlags usage) const;

    bool ValidateBarriersToImages(const Location& barrier_loc, const vvl::CommandBuffer* cb_state,
                                  const ImageBarrier& image_barrier, CommandBufferImageLayoutMap& layout_updates_state) const;

    void RecordQueuedQFOTransfers(vvl::CommandBuffer* cb_state);

    void TransitionImageLayouts(vvl::CommandBuffer* cb_state, uint32_t barrier_count, const VkImageMemoryBarrier2* image_barriers);
    void TransitionImageLayouts(vvl::CommandBuffer* cb_state, uint32_t barrier_count, const VkImageMemoryBarrier* image_barriers,
                                VkPipelineStageFlags src_stage_mask, VkPipelineStageFlags dst_stage_mask);

    void RecordTransitionImageLayout(vvl::CommandBuffer* cb_state, const ImageBarrier& image_barrier);
    void RecordBarriers(Func func_name, vvl::CommandBuffer* cb_state, VkPipelineStageFlags src_stage_mask,
                        VkPipelineStageFlags dst_stage_mask, uint32_t bufferBarrierCount,
                        const VkBufferMemoryBarrier* pBufferMemBarriers, uint32_t imageMemBarrierCount,
                        const VkImageMemoryBarrier* pImageMemBarriers);
    void RecordBarriers(Func func_name, vvl::CommandBuffer* cb_state, const VkDependencyInfoKHR& dep_info);

    void TransitionFinalSubpassLayouts(vvl::CommandBuffer* cb_state);

    template <typename HandleT, typename RegionType>
    bool ValidateCopyImageCommon(HandleT handle, const vvl::Image& src_image_state, const vvl::Image& dst_image_state,
                                 uint32_t regionCount, const RegionType* pRegions, const Location& loc) const;

    template <typename RegionType>
    bool ValidateCmdCopyImage(VkCommandBuffer commandBuffer, VkImage srcImage, VkImageLayout srcImageLayout, VkImage dstImage,
                              VkImageLayout dstImageLayout, uint32_t regionCount, const RegionType* pRegions,
                              const Location& loc) const;

    bool PreCallValidateCmdCopyImage(VkCommandBuffer commandBuffer, VkImage srcImage, VkImageLayout srcImageLayout,
                                     VkImage dstImage, VkImageLayout dstImageLayout, uint32_t regionCount,
                                     const VkImageCopy* pRegions, const ErrorObject& error_obj) const override;

    bool PreCallValidateCmdCopyImage2KHR(VkCommandBuffer commandBuffer, const VkCopyImageInfo2KHR* pCopyImageInfo,
                                         const ErrorObject& error_obj) const override;

    bool PreCallValidateCmdCopyImage2(VkCommandBuffer commandBuffer, const VkCopyImageInfo2* pCopyImageInfo,
                                      const ErrorObject& error_obj) const override;

    bool PreCallValidateCmdClearAttachments(VkCommandBuffer commandBuffer, uint32_t attachmentCount,
                                            const VkClearAttachment* pAttachments, uint32_t rectCount, const VkClearRect* pRects,
                                            const ErrorObject& error_obj) const override;
    void PreCallRecordCmdClearAttachments(VkCommandBuffer commandBuffer, uint32_t attachmentCount,
                                          const VkClearAttachment* pAttachments, uint32_t rectCount, const VkClearRect* pRects,
                                          const RecordObject& record_obj) override;

    template <typename RegionType>
    bool ValidateCmdResolveImage(VkCommandBuffer commandBuffer, VkImage srcImage, VkImageLayout srcImageLayout, VkImage dstImage,
                                 VkImageLayout dstImageLayout, uint32_t regionCount, const RegionType* pRegions,
                                 const Location& loc) const;

    bool PreCallValidateCmdResolveImage(VkCommandBuffer commandBuffer, VkImage srcImage, VkImageLayout srcImageLayout,
                                        VkImage dstImage, VkImageLayout dstImageLayout, uint32_t regionCount,
                                        const VkImageResolve* pRegions, const ErrorObject& error_obj) const override;

    bool PreCallValidateCmdResolveImage2KHR(VkCommandBuffer commandBuffer, const VkResolveImageInfo2KHR* pResolveImageInfo,
                                            const ErrorObject& error_obj) const override;

    bool PreCallValidateCmdResolveImage2(VkCommandBuffer commandBuffer, const VkResolveImageInfo2* pResolveImageInfo,
                                         const ErrorObject& error_obj) const override;

    template <typename RegionType>
    bool ValidateCmdBlitImage(VkCommandBuffer commandBuffer, VkImage srcImage, VkImageLayout srcImageLayout, VkImage dstImage,
                              VkImageLayout dstImageLayout, uint32_t regionCount, const RegionType* pRegions, VkFilter filter,
                              const Location& loc) const;

    bool PreCallValidateCmdBlitImage(VkCommandBuffer commandBuffer, VkImage srcImage, VkImageLayout srcImageLayout,
                                     VkImage dstImage, VkImageLayout dstImageLayout, uint32_t regionCount,
                                     const VkImageBlit* pRegions, VkFilter filter, const ErrorObject& error_obj) const override;

    bool PreCallValidateCmdBlitImage2KHR(VkCommandBuffer commandBuffer, const VkBlitImageInfo2KHR* pBlitImageInfo,
                                         const ErrorObject& error_obj) const override;

    bool PreCallValidateCmdBlitImage2(VkCommandBuffer commandBuffer, const VkBlitImageInfo2* pBlitImageInfo,
                                      const ErrorObject& error_obj) const override;

    template <typename RegionType>
    void RecordCmdBlitImage(VkCommandBuffer commandBuffer, VkImage srcImage, VkImageLayout srcImageLayout, VkImage dstImage,
                            VkImageLayout dstImageLayout, uint32_t regionCount, const RegionType* pRegions, VkFilter filter);

    void PreCallRecordCmdBlitImage(VkCommandBuffer commandBuffer, VkImage srcImage, VkImageLayout srcImageLayout, VkImage dstImage,
                                   VkImageLayout dstImageLayout, uint32_t regionCount, const VkImageBlit* pRegions, VkFilter filter,
                                   const RecordObject& record_obj) override;

    void PreCallRecordCmdBlitImage2KHR(VkCommandBuffer commandBuffer, const VkBlitImageInfo2KHR* pBlitImageInfo,
                                       const RecordObject& record_obj) override;

    void PreCallRecordCmdBlitImage2(VkCommandBuffer commandBuffer, const VkBlitImageInfo2* pBlitImageInfo,
                                    const RecordObject& record_obj) override;

    bool ValidateCmdBufImageLayouts(const Location& loc, const vvl::CommandBuffer& cb_state,
                                    GlobalImageLayoutMap& overlayLayoutMap) const;

    void UpdateCmdBufImageLayouts(const vvl::CommandBuffer& cb_state);

    bool VerifyBoundMemoryIsValid(const vvl::DeviceMemory* mem_state, const LogObjectList& objlist,
                                  const VulkanTypedHandle& typed_handle, const Location& loc, const char* vuid) const;
    bool VerifyBoundMemoryIsDeviceVisible(const vvl::DeviceMemory* mem_state, const LogObjectList& objlist,
                                          const VulkanTypedHandle& typed_handle, const Location& loc, const char* vuid) const;

    bool ValidateLayoutVsAttachmentDescription(const VkImageLayout first_layout, const uint32_t attachment,
                                               const VkAttachmentDescription2& attachment_description,
                                               const Location& layout_loc) const;

    bool ValidateImageUsageFlags(VkCommandBuffer cb, vvl::Image const& image_state, VkImageUsageFlags desired, bool strict,
                                 const char* vuid, const Location& image_loc) const;

    bool ValidateImageFormatFeatureFlags(VkCommandBuffer cb, vvl::Image const& image_state, VkFormatFeatureFlags2KHR desired,
                                         const Location& image_loc, const char* vuid) const;

    template <typename HandleT>
    bool ValidateImageSubresourceLayers(HandleT handle, const VkImageSubresourceLayers* subresource_layers,
                                        const Location& subresource_loc) const;

    bool ValidateBufferUsageFlags(const LogObjectList& objlist, const vvl::Buffer& buffer_state, VkFlags desired, bool strict,
                                  const char* vuid, const Location& buffer_loc) const;

    bool PreCallValidateCreateBuffer(VkDevice device, const VkBufferCreateInfo* pCreateInfo,
                                     const VkAllocationCallbacks* pAllocator, VkBuffer* pBuffer,
                                     const ErrorObject& error_obj) const override;

    bool PreCallValidateCreateBufferView(VkDevice device, const VkBufferViewCreateInfo* pCreateInfo,
                                         const VkAllocationCallbacks* pAllocator, VkBufferView* pView,
                                         const ErrorObject& error_obj) const override;

    bool ValidateImageAspectMask(VkImage image, VkFormat format, VkImageAspectFlags aspect_mask, bool is_image_disjoint,
                                 const Location& loc,
                                 const char* vuid = "UNASSIGNED-CoreValidation-DrawState-InvalidImageAspect") const;

    bool ValidateCreateImageViewSubresourceRange(const vvl::Image& image_state, bool is_imageview_2d_type,
                                                 const VkImageSubresourceRange& subresourceRange, const Location& loc) const;

    bool ValidateCmdClearColorSubresourceRange(const vvl::Image& image_state, const VkImageSubresourceRange& subresourceRange,
                                               const Location& loc) const;

    bool ValidateCmdClearDepthSubresourceRange(const vvl::Image& image_state, const VkImageSubresourceRange& subresourceRange,
                                               const Location& loc) const;

    bool ValidateImageBarrierSubresourceRange(const Location& loc, const vvl::Image& image_state,
                                              const VkImageSubresourceRange& subresourceRange) const;

    bool ValidateImageViewFormatFeatures(const vvl::Image& image_state, const VkFormat view_format,
                                         const VkImageUsageFlags image_usage, const Location& create_info_loc) const;

    bool PreCallValidateCreateImageView(VkDevice device, const VkImageViewCreateInfo* pCreateInfo,
                                        const VkAllocationCallbacks* pAllocator, VkImageView* pView,
                                        const ErrorObject& error_obj) const override;
    template <typename RegionType>
    bool ValidateCmdCopyBufferBounds(VkCommandBuffer cb, const vvl::Buffer& src_buffer_state, const vvl::Buffer& dst_buffer_state,
                                     uint32_t regionCount, const RegionType* pRegions, const Location& loc) const;

    template <typename HandleT, typename RegionType>
    bool ValidateImageBounds(const HandleT handle, const vvl::Image& image_state, const uint32_t regionCount,
                             const RegionType* pRegions, const Location& loc, const char* vuid, bool is_src) const;

    template <typename RegionType>
    bool ValidateBufferBounds(VkCommandBuffer cb, const vvl::Image& image_state, const vvl::Buffer& buff_state,
                              uint32_t regionCount, const RegionType* pRegions, const Location& loc, const char* vuid) const;

    template <typename RegionType>
    bool ValidateCopyBufferImageTransferGranularityRequirements(const vvl::CommandBuffer& cb_state, const vvl::Image& image_state,
                                                                const RegionType* region, const Location& region_loc,
                                                                const char* vuid) const;

    template <typename HandleT>
    bool ValidateImageMipLevel(const HandleT handle, const vvl::Image& image_state, uint32_t mip_level, const Location& mip_loc,
                               const char* vuid) const;

    template <typename HandleT>
    bool ValidateImageArrayLayerRange(const HandleT handle, const vvl::Image& img, const uint32_t base_layer,
                                      const uint32_t layer_count, const Location& subresource_loc, const char* vuid) const;

    void PreCallRecordCmdCopyImage(VkCommandBuffer commandBuffer, VkImage srcImage, VkImageLayout srcImageLayout, VkImage dstImage,
                                   VkImageLayout dstImageLayout, uint32_t regionCount, const VkImageCopy* pRegions,
                                   const RecordObject& record_obj) override;

    void RecordCmdCopyImage2(VkCommandBuffer commandBuffer, const VkCopyImageInfo2* pCopyImageInfo);
    void PreCallRecordCmdCopyImage2KHR(VkCommandBuffer commandBuffer, const VkCopyImageInfo2KHR* pCopyImageInf,
                                       const RecordObject& record_objo) override;
    void PreCallRecordCmdCopyImage2(VkCommandBuffer commandBuffer, const VkCopyImageInfo2* pCopyImageInfo,
                                    const RecordObject& record_obj) override;

    template <typename RegionType>
    bool ValidateCmdCopyBuffer(VkCommandBuffer commandBuffer, VkBuffer srcBuffer, VkBuffer dstBuffer, uint32_t regionCount,
                               const RegionType* pRegions, const Location& loc) const;

    bool PreCallValidateCmdCopyBuffer(VkCommandBuffer commandBuffer, VkBuffer srcBuffer, VkBuffer dstBuffer, uint32_t regionCount,
                                      const VkBufferCopy* pRegions, const ErrorObject& error_obj) const override;

    bool PreCallValidateCmdCopyBuffer2KHR(VkCommandBuffer commandBuffer, const VkCopyBufferInfo2KHR* pCopyBufferInfo,
                                          const ErrorObject& error_obj) const override;

    bool PreCallValidateCmdCopyBuffer2(VkCommandBuffer commandBuffer, const VkCopyBufferInfo2* pCopyBufferInfo,
                                       const ErrorObject& error_obj) const override;

    template <typename RegionType>
    void RecordCmdCopyBuffer(VkCommandBuffer commandBuffer, VkBuffer srcBuffer, VkBuffer dstBuffer, uint32_t regionCount,
                             const RegionType* pRegions, const Location& loc);
    void PreCallRecordCmdCopyBuffer(VkCommandBuffer commandBuffer, VkBuffer srcBuffer, VkBuffer dstBuffer, uint32_t regionCount,
                                    const VkBufferCopy* pRegions, const RecordObject& record_obj) override;
    void PreCallRecordCmdCopyBuffer2KHR(VkCommandBuffer commandBuffer, const VkCopyBufferInfo2KHR* pCopyBufferInfo,
                                        const RecordObject& record_obj) override;
    void PreCallRecordCmdCopyBuffer2(VkCommandBuffer commandBuffer, const VkCopyBufferInfo2* pCopyBufferInfo,
                                     const RecordObject& record_obj) override;

    bool PreCallValidateDestroyImageView(VkDevice device, VkImageView imageView, const VkAllocationCallbacks* pAllocator,
                                         const ErrorObject& error_obj) const override;

    bool PreCallValidateDestroyBuffer(VkDevice device, VkBuffer buffer, const VkAllocationCallbacks* pAllocator,
                                      const ErrorObject& error_obj) const override;

    bool PreCallValidateDestroyBufferView(VkDevice device, VkBufferView bufferView, const VkAllocationCallbacks* pAllocator,
                                          const ErrorObject& error_obj) const override;

    bool PreCallValidateCmdFillBuffer(VkCommandBuffer commandBuffer, VkBuffer dstBuffer, VkDeviceSize dstOffset, VkDeviceSize size,
                                      uint32_t data, const ErrorObject& error_obj) const override;

    template <typename RegionType>
    bool ValidateCmdCopyImageToBuffer(VkCommandBuffer commandBuffer, VkImage srcImage, VkImageLayout srcImageLayout,
                                      VkBuffer dstBuffer, uint32_t regionCount, const RegionType* pRegions,
                                      const Location& loc) const;

    bool PreCallValidateCmdCopyImageToBuffer(VkCommandBuffer commandBuffer, VkImage srcImage, VkImageLayout srcImageLayout,
                                             VkBuffer dstBuffer, uint32_t regionCount, const VkBufferImageCopy* pRegions,
                                             const ErrorObject& error_obj) const override;

    void PreCallRecordCmdCopyImageToBuffer(VkCommandBuffer commandBuffer, VkImage srcImage, VkImageLayout srcImageLayout,
                                           VkBuffer dstBuffer, uint32_t regionCount, const VkBufferImageCopy* pRegions,
                                           const RecordObject& record_obj) override;

    bool PreCallValidateCmdCopyImageToBuffer2KHR(VkCommandBuffer commandBuffer,
                                                 const VkCopyImageToBufferInfo2KHR* pCopyImageToBufferInfo,
                                                 const ErrorObject& error_obj) const override;

    bool PreCallValidateCmdCopyImageToBuffer2(VkCommandBuffer commandBuffer, const VkCopyImageToBufferInfo2* pCopyImageToBufferInfo,
                                              const ErrorObject& error_obj) const override;

    void PreCallRecordCmdCopyImageToBuffer2KHR(VkCommandBuffer commandBuffer,
                                               const VkCopyImageToBufferInfo2KHR* pCopyImageToBufferInfo,
                                               const RecordObject& record_obj) override;

    void PreCallRecordCmdCopyImageToBuffer2(VkCommandBuffer commandBuffer, const VkCopyImageToBufferInfo2* pCopyImageToBufferInfo,
                                            const RecordObject& record_obj) override;

    template <typename RegionType>
    bool ValidateCmdCopyBufferToImage(VkCommandBuffer commandBuffer, VkBuffer srcBuffer, VkImage dstImage,
                                      VkImageLayout dstImageLayout, uint32_t regionCount, const RegionType* pRegions,
                                      const Location& loc) const;

    bool PreCallValidateCmdCopyBufferToImage(VkCommandBuffer commandBuffer, VkBuffer srcBuffer, VkImage dstImage,
                                             VkImageLayout dstImageLayout, uint32_t regionCount, const VkBufferImageCopy* pRegions,
                                             const ErrorObject& error_obj) const override;

    void PreCallRecordCmdCopyBufferToImage(VkCommandBuffer commandBuffer, VkBuffer srcBuffer, VkImage dstImage,
                                           VkImageLayout dstImageLayout, uint32_t regionCount, const VkBufferImageCopy* pRegions,
                                           const RecordObject& record_obj) override;

    bool PreCallValidateCmdCopyBufferToImage2KHR(VkCommandBuffer commandBuffer,
                                                 const VkCopyBufferToImageInfo2KHR* pCopyBufferToImageInfo,
                                                 const ErrorObject& error_obj) const override;

    bool PreCallValidateCmdCopyBufferToImage2(VkCommandBuffer commandBuffer, const VkCopyBufferToImageInfo2* pCopyBufferToImageInfo,
                                              const ErrorObject& error_obj) const override;

    void PreCallRecordCmdCopyBufferToImage2KHR(VkCommandBuffer commandBuffer,
                                               const VkCopyBufferToImageInfo2KHR* pCopyBufferToImageInfo,
                                               const RecordObject& record_obj) override;

    void PreCallRecordCmdCopyBufferToImage2(VkCommandBuffer commandBuffer, const VkCopyBufferToImageInfo2* pCopyBufferToImageInfo,
                                            const RecordObject& record_obj) override;

    bool PreCallValidateCopyMemoryToImageEXT(VkDevice device, const VkCopyMemoryToImageInfoEXT* pCopyMemoryToImageInfo,
                                             const ErrorObject& error_obj) const override;

    bool PreCallValidateCopyImageToMemoryEXT(VkDevice device, const VkCopyImageToMemoryInfoEXT* pCopyImageToMemoryInfo,
                                             const ErrorObject& error_obj) const override;

    bool PreCallValidateCopyImageToImageEXT(VkDevice device, const VkCopyImageToImageInfoEXT* pCopyImageToImageInfo,
                                            const ErrorObject& error_obj) const override;

    bool ValidateCreateImageANDROID(const VkImageCreateInfo* create_info, const Location& create_info_loc) const;
    bool ValidateCreateImageViewANDROID(const VkImageViewCreateInfo* create_info, const Location& create_info_loc) const;
    bool ValidatePhysicalDeviceQueueFamilies(uint32_t queue_family_count, const uint32_t* queue_families, const Location& loc,
                                             const char* vuid) const;
    bool ValidateAllocateMemoryANDROID(const VkMemoryAllocateInfo* allocate_info, const Location& allocate_info_loc) const;
    bool ValidateGetImageMemoryRequirementsANDROID(const VkImage image, const Location& loc) const;
    bool ValidateGetPhysicalDeviceImageFormatProperties2ANDROID(const VkPhysicalDeviceImageFormatInfo2* pImageFormatInfo,
                                                                const VkImageFormatProperties2* pImageFormatProperties,
                                                                const ErrorObject& error_obj) const;
    bool ValidateBufferImportedHandleANDROID(VkExternalMemoryHandleTypeFlags handle_types, VkDeviceMemory memory, VkBuffer buffer,
                                             const Location& loc) const;
    bool ValidateImageImportedHandleANDROID(VkExternalMemoryHandleTypeFlags handle_types, VkDeviceMemory memory, VkImage image,
                                            const Location& loc) const;
    bool PreCallValidateCreatePipelineCache(VkDevice device, const VkPipelineCacheCreateInfo* pCreateInfo,
                                            const VkAllocationCallbacks* pAllocator, VkPipelineCache* pPipelineCache,
                                            const ErrorObject& error_obj) const override;
    bool PreCallValidateCreateGraphicsPipelines(VkDevice device, VkPipelineCache pipelineCache, uint32_t count,
                                                const VkGraphicsPipelineCreateInfo* pCreateInfos,
                                                const VkAllocationCallbacks* pAllocator, VkPipeline* pPipelines,
                                                const ErrorObject& error_obj, void* cgpl_state) const override;
    bool PreCallValidateCreateComputePipelines(VkDevice device, VkPipelineCache pipelineCache, uint32_t count,
                                               const VkComputePipelineCreateInfo* pCreateInfos,
                                               const VkAllocationCallbacks* pAllocator, VkPipeline* pPipelines,
                                               const ErrorObject& error_obj, void* pipe_state) const override;
    bool PreCallValidateGetPipelineExecutablePropertiesKHR(VkDevice device, const VkPipelineInfoKHR* pPipelineInfo,
                                                           uint32_t* pExecutableCount,
                                                           VkPipelineExecutablePropertiesKHR* pProperties,
                                                           const ErrorObject& error_obj) const override;
    bool ValidatePipelineExecutableInfo(VkDevice device, const VkPipelineExecutableInfoKHR* pExecutableInfo, const Location& loc,
                                        const char* feature_vuid) const;
    bool PreCallValidateGetPipelineExecutableStatisticsKHR(VkDevice device, const VkPipelineExecutableInfoKHR* pExecutableInfo,
                                                           uint32_t* pStatisticCount, VkPipelineExecutableStatisticKHR* pStatistic,
                                                           const ErrorObject& error_objs) const override;
    bool PreCallValidateGetPipelineExecutableInternalRepresentationsKHR(VkDevice device,
                                                                        const VkPipelineExecutableInfoKHR* pExecutableInfo,
                                                                        uint32_t* pInternalRepresentationCount,
                                                                        VkPipelineExecutableInternalRepresentationKHR* pStatistics,
                                                                        const ErrorObject& error_obj) const override;
    bool PreCallValidateMapMemory2KHR(VkDevice device, const VkMemoryMapInfoKHR* pMemoryMapInfo, void** ppData,
                                      const ErrorObject& error_obj) const override;
    bool PreCallValidateUnmapMemory2KHR(VkDevice device, const VkMemoryUnmapInfoKHR* pMemoryUnmapInfo,
                                        const ErrorObject& error_obj) const override;
    bool PreCallValidateCreatePipelineLayout(VkDevice device, const VkPipelineLayoutCreateInfo* pCreateInfo,
                                             const VkAllocationCallbacks* pAllocator, VkPipelineLayout* pPipelineLayout,
                                             const ErrorObject& error_obj) const override;
    bool PreCallValidateAllocateDescriptorSets(VkDevice device, const VkDescriptorSetAllocateInfo* pAllocateInfo,
                                               VkDescriptorSet* pDescriptorSets, const ErrorObject& error_obj,
                                               void* ads_state) const override;
    void PostCallRecordAllocateDescriptorSets(VkDevice device, const VkDescriptorSetAllocateInfo* pAllocateInfo,
                                              VkDescriptorSet* pDescriptorSets, const RecordObject& record_obj,
                                              void* ads_state) override;
    bool PreCallValidateCreateRayTracingPipelinesNV(VkDevice device, VkPipelineCache pipelineCache, uint32_t count,
                                                    const VkRayTracingPipelineCreateInfoNV* pCreateInfos,
                                                    const VkAllocationCallbacks* pAllocator, VkPipeline* pPipelines,
                                                    const ErrorObject& error_obj, void* pipe_state) const override;
    bool PreCallValidateCreateRayTracingPipelinesKHR(VkDevice device, VkDeferredOperationKHR deferredOperation,
                                                     VkPipelineCache pipelineCache, uint32_t count,
                                                     const VkRayTracingPipelineCreateInfoKHR* pCreateInfos,
                                                     const VkAllocationCallbacks* pAllocator, VkPipeline* pPipelines,
                                                     const ErrorObject& error_obj, void* pipe_state) const override;
    bool PreCallValidateCmdTraceRaysNV(VkCommandBuffer commandBuffer, VkBuffer raygenShaderBindingTableBuffer,
                                       VkDeviceSize raygenShaderBindingOffset, VkBuffer missShaderBindingTableBuffer,
                                       VkDeviceSize missShaderBindingOffset, VkDeviceSize missShaderBindingStride,
                                       VkBuffer hitShaderBindingTableBuffer, VkDeviceSize hitShaderBindingOffset,
                                       VkDeviceSize hitShaderBindingStride, VkBuffer callableShaderBindingTableBuffer,
                                       VkDeviceSize callableShaderBindingOffset, VkDeviceSize callableShaderBindingStride,
                                       uint32_t width, uint32_t height, uint32_t depth,
                                       const ErrorObject& error_obj) const override;
    bool ValidateRaytracingShaderBindingTable(VkCommandBuffer commandBuffer, const Location& table_loc,
                                              const char* vuid_single_device_memory, const char* vuid_binding_table_flag,
                                              const VkStridedDeviceAddressRegionKHR& binding_table) const;
    bool ValidateCmdTraceRaysKHR(const Location& loc, const vvl::CommandBuffer& cb_state,
                                 const VkStridedDeviceAddressRegionKHR* pRaygenShaderBindingTable,
                                 const VkStridedDeviceAddressRegionKHR* pMissShaderBindingTable,
                                 const VkStridedDeviceAddressRegionKHR* pHitShaderBindingTable,
                                 const VkStridedDeviceAddressRegionKHR* pCallableShaderBindingTable) const;
    bool PreCallValidateCmdTraceRaysKHR(VkCommandBuffer commandBuffer,
                                        const VkStridedDeviceAddressRegionKHR* pRaygenShaderBindingTable,
                                        const VkStridedDeviceAddressRegionKHR* pMissShaderBindingTable,
                                        const VkStridedDeviceAddressRegionKHR* pHitShaderBindingTable,
                                        const VkStridedDeviceAddressRegionKHR* pCallableShaderBindingTable, uint32_t width,
                                        uint32_t height, uint32_t depth, const ErrorObject& error_obj) const override;
    bool PreCallValidateCmdTraceRaysIndirectKHR(VkCommandBuffer commandBuffer,
                                                const VkStridedDeviceAddressRegionKHR* pRaygenShaderBindingTable,
                                                const VkStridedDeviceAddressRegionKHR* pMissShaderBindingTable,
                                                const VkStridedDeviceAddressRegionKHR* pHitShaderBindingTable,
                                                const VkStridedDeviceAddressRegionKHR* pCallableShaderBindingTable,
                                                VkDeviceAddress indirectDeviceAddress, const ErrorObject& error_obj) const override;
    bool PreCallValidateCmdTraceRaysIndirect2KHR(VkCommandBuffer commandBuffer, VkDeviceAddress indirectDeviceAddress,
                                                 const ErrorObject& error_obj) const override;
    bool ValidateDeferredOperation(VkDevice device, VkDeferredOperationKHR deferred_operation, const Location& loc,
                                   const char* vuid) const;
    bool PreCallValidateCreateDevice(VkPhysicalDevice gpu, const VkDeviceCreateInfo* pCreateInfo,
                                     const VkAllocationCallbacks* pAllocator, VkDevice* pDevice,
                                     const ErrorObject& error_obj) const override;
    void CreateDevice(const VkDeviceCreateInfo* pCreateInfo) override;
    bool PreCallValidateCmdUpdateBuffer(VkCommandBuffer commandBuffer, VkBuffer dstBuffer, VkDeviceSize dstOffset,
                                        VkDeviceSize dataSize, const void* pData, const ErrorObject& error_obj) const override;
    bool PreCallValidateGetDeviceQueue(VkDevice device, uint32_t queueFamilyIndex, uint32_t queueIndex, VkQueue* pQueue,
                                       const ErrorObject& error_obj) const override;
    bool PreCallValidateGetDeviceQueue2(VkDevice device, const VkDeviceQueueInfo2* pQueueInfo, VkQueue* pQueue,
                                        const ErrorObject& error_obj) const override;
    bool PreCallValidateCreateSamplerYcbcrConversion(VkDevice device, const VkSamplerYcbcrConversionCreateInfo* pCreateInfo,
                                                     const VkAllocationCallbacks* pAllocator,
                                                     VkSamplerYcbcrConversion* pYcbcrConversion,
                                                     const ErrorObject& error_obj) const override;
    bool PreCallValidateCreateSamplerYcbcrConversionKHR(VkDevice device, const VkSamplerYcbcrConversionCreateInfo* pCreateInfo,
                                                        const VkAllocationCallbacks* pAllocator,
                                                        VkSamplerYcbcrConversion* pYcbcrConversion,
                                                        const ErrorObject& error_obj) const override;
    bool PreCallValidateCreateSampler(VkDevice device, const VkSamplerCreateInfo* pCreateInfo,
                                      const VkAllocationCallbacks* pAllocator, VkSampler* pSampler,
                                      const ErrorObject& error_obj) const override;
    bool PreCallValidateCmdDebugMarkerBeginEXT(VkCommandBuffer commandBuffer, const VkDebugMarkerMarkerInfoEXT* pMarkerInfo,
                                               const ErrorObject& error_obj) const override;
    void PreCallRecordDestroyDevice(VkDevice device, const VkAllocationCallbacks* pAllocator,
                                    const RecordObject& record_obj) override;
    bool PreCallValidateQueueSubmit(VkQueue queue, uint32_t submitCount, const VkSubmitInfo* pSubmits, VkFence fence,
                                    const ErrorObject& error_obj) const override;
    void PostCallRecordQueueSubmit(VkQueue queue, uint32_t submitCount, const VkSubmitInfo* pSubmits, VkFence fence,
                                   const RecordObject& record_obj) override;
    bool ValidateQueueSubmit2(VkQueue queue, uint32_t submitCount, const VkSubmitInfo2KHR* pSubmits, VkFence fence,
                              const ErrorObject& error_obj) const;
    bool PreCallValidateQueueSubmit2KHR(VkQueue queue, uint32_t submitCount, const VkSubmitInfo2KHR* pSubmits, VkFence fence,
                                        const ErrorObject& error_obj) const override;
    bool PreCallValidateQueueSubmit2(VkQueue queue, uint32_t submitCount, const VkSubmitInfo2* pSubmits, VkFence fence,
                                     const ErrorObject& error_obj) const override;
    void RecordQueueSubmit2(VkQueue queue, uint32_t submitCount, const VkSubmitInfo2KHR* pSubmits, VkFence fence,
                            const RecordObject& record_obj);
    void PostCallRecordQueueSubmit2KHR(VkQueue queue, uint32_t submitCount, const VkSubmitInfo2KHR* pSubmits, VkFence fence,
                                       const RecordObject& record_obj) override;
    void PostCallRecordQueueSubmit2(VkQueue queue, uint32_t submitCount, const VkSubmitInfo2* pSubmits, VkFence fence,
                                    const RecordObject& record_obj) override;
    bool IsZeroAllocationSizeAllowed(const VkMemoryAllocateInfo* pAllocateInfo) const;
    bool HasExternalMemoryImportSupport(const vvl::Buffer& buffer, VkExternalMemoryHandleTypeFlagBits handle_type) const;
    bool HasExternalMemoryImportSupport(const vvl::Image& image, VkExternalMemoryHandleTypeFlagBits handle_type) const;
    bool PreCallValidateAllocateMemory(VkDevice device, const VkMemoryAllocateInfo* pAllocateInfo,
                                       const VkAllocationCallbacks* pAllocator, VkDeviceMemory* pMemory,
                                       const ErrorObject& error_obj) const override;
    bool PreCallValidateFreeMemory(VkDevice device, VkDeviceMemory memory, const VkAllocationCallbacks* pAllocator,
                                   const ErrorObject& error_obj) const override;
    bool PreCallValidateCreateFence(VkDevice device, const VkFenceCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator,
                                    VkFence* pFence, const ErrorObject& error_obj) const override;
    bool PreCallValidateCreateSemaphore(VkDevice device, const VkSemaphoreCreateInfo* pCreateInfo,
                                        const VkAllocationCallbacks* pAllocator, VkSemaphore* pSemaphore,
                                        const ErrorObject& error_obj) const override;
    bool PreCallValidateWaitSemaphores(VkDevice device, const VkSemaphoreWaitInfo* pWaitInfo, uint64_t timeout,
                                       const ErrorObject& error_obj) const override;
    bool PreCallValidateWaitSemaphoresKHR(VkDevice device, const VkSemaphoreWaitInfo* pWaitInfo, uint64_t timeout,
                                          const ErrorObject& error_obj) const override;
    bool PreCallValidateDestroyFence(VkDevice device, VkFence fence, const VkAllocationCallbacks* pAllocator,
                                     const ErrorObject& error_obj) const override;
    bool PreCallValidateDestroySemaphore(VkDevice device, VkSemaphore semaphore, const VkAllocationCallbacks* pAllocator,
                                         const ErrorObject& error_obj) const override;
    bool PreCallValidateDestroyEvent(VkDevice device, VkEvent event, const VkAllocationCallbacks* pAllocator,
                                     const ErrorObject& error_obj) const override;
    bool PreCallValidateDestroyQueryPool(VkDevice device, VkQueryPool queryPool, const VkAllocationCallbacks* pAllocator,
                                         const ErrorObject& error_obj) const override;
    bool ValidatePerformanceQueryResults(const vvl::QueryPool& query_pool_state, uint32_t firstQuery, uint32_t queryCount,
                                         VkQueryResultFlags flags, const Location& loc) const;
    bool ValidateQueryPoolWasReset(const vvl::QueryPool& query_pool_state, uint32_t firstQuery, uint32_t queryCount,
                                   const Location& loc, QueryMap* localQueryToStateMap, uint32_t perfPass) const;
    bool PreCallValidateGetQueryPoolResults(VkDevice device, VkQueryPool queryPool, uint32_t firstQuery, uint32_t queryCount,
                                            size_t dataSize, void* pData, VkDeviceSize stride, VkQueryResultFlags flags,
                                            const ErrorObject& error_obj) const override;
    bool ValidateBindBufferMemory(VkBuffer buffer, VkDeviceMemory memory, VkDeviceSize memoryOffset, const void* pNext,
                                  const Location& loc) const;
    bool PreCallValidateBindBufferMemory2KHR(VkDevice device, uint32_t bindInfoCount, const VkBindBufferMemoryInfo* pBindInfos,
                                             const ErrorObject& error_obj) const override;
    bool PreCallValidateBindBufferMemory2(VkDevice device, uint32_t bindInfoCount, const VkBindBufferMemoryInfo* pBindInfos,
                                          const ErrorObject& error_obj) const override;
    bool PreCallValidateBindBufferMemory(VkDevice device, VkBuffer buffer, VkDeviceMemory memory, VkDeviceSize memoryOffset,
                                         const ErrorObject& error_obj) const override;
    bool PreCallValidateGetImageMemoryRequirements(VkDevice device, VkImage image, VkMemoryRequirements* pMemoryRequirements,
                                                   const ErrorObject& error_obj) const override;
    bool PreCallValidateGetImageMemoryRequirements2(VkDevice device, const VkImageMemoryRequirementsInfo2* pInfo,
                                                    VkMemoryRequirements2* pMemoryRequirements,
                                                    const ErrorObject& error_obj) const override;
    bool PreCallValidateGetImageMemoryRequirements2KHR(VkDevice device, const VkImageMemoryRequirementsInfo2* pInfo,
                                                       VkMemoryRequirements2* pMemoryRequirements,
                                                       const ErrorObject& error_obj) const override;
    bool ValidateGetPhysicalDeviceImageFormatProperties2(const VkPhysicalDeviceImageFormatInfo2* pImageFormatInfo,
                                                         VkImageFormatProperties2* pImageFormatProperties,
                                                         const ErrorObject& error_obj) const;
    bool PreCallValidateGetPhysicalDeviceImageFormatProperties2(VkPhysicalDevice physicalDevice,
                                                                const VkPhysicalDeviceImageFormatInfo2* pImageFormatInfo,
                                                                VkImageFormatProperties2* pImageFormatProperties,
                                                                const ErrorObject& error_obj) const override;
    bool PreCallValidateGetPhysicalDeviceImageFormatProperties2KHR(VkPhysicalDevice physicalDevice,
                                                                   const VkPhysicalDeviceImageFormatInfo2* pImageFormatInfo,
                                                                   VkImageFormatProperties2* pImageFormatProperties,
                                                                   const ErrorObject& error_obj) const override;
    bool GetPhysicalDeviceImageFormatProperties(vvl::Image& image_state, const char* vuid_string, const Location& loc) const;
    bool PreCallValidateDestroyPipeline(VkDevice device, VkPipeline pipeline, const VkAllocationCallbacks* pAllocator,
                                        const ErrorObject& error_obj) const override;
    bool PreCallValidateDestroySampler(VkDevice device, VkSampler sampler, const VkAllocationCallbacks* pAllocator,
                                       const ErrorObject& error_obj) const override;
    bool PreCallValidateDestroyDescriptorPool(VkDevice device, VkDescriptorPool descriptorPool,
                                              const VkAllocationCallbacks* pAllocator, const ErrorObject& error_obj) const override;
    bool PreCallValidateFreeCommandBuffers(VkDevice device, VkCommandPool commandPool, uint32_t commandBufferCount,
                                           const VkCommandBuffer* pCommandBuffers, const ErrorObject& error_obj) const override;
    bool PreCallValidateCreateCommandPool(VkDevice device, const VkCommandPoolCreateInfo* pCreateInfo,
                                          const VkAllocationCallbacks* pAllocator, VkCommandPool* pCommandPool,
                                          const ErrorObject& error_obj) const override;
    bool PreCallValidateCreateQueryPool(VkDevice device, const VkQueryPoolCreateInfo* pCreateInfo,
                                        const VkAllocationCallbacks* pAllocator, VkQueryPool* pQueryPool,
                                        const ErrorObject& error_obj) const override;
    bool PreCallValidateDestroyCommandPool(VkDevice device, VkCommandPool commandPool, const VkAllocationCallbacks* pAllocator,
                                           const ErrorObject& error_obj) const override;
    bool PreCallValidateResetCommandPool(VkDevice device, VkCommandPool commandPool, VkCommandPoolResetFlags flags,
                                         const ErrorObject& error_obj) const override;
    bool PreCallValidateResetFences(VkDevice device, uint32_t fenceCount, const VkFence* pFences,
                                    const ErrorObject& error_obj) const override;
    bool PreCallValidateDestroyFramebuffer(VkDevice device, VkFramebuffer framebuffer, const VkAllocationCallbacks* pAllocator,
                                           const ErrorObject& error_obj) const override;
    bool PreCallValidateDestroyRenderPass(VkDevice device, VkRenderPass renderPass, const VkAllocationCallbacks* pAllocator,
                                          const ErrorObject& error_obj) const override;
    bool ValidateDescriptorSetLayoutBindingFlags(const VkDescriptorSetLayoutCreateInfo* pCreateInfo, uint32_t max_binding,
                                                 uint32_t* update_after_bind, const Location& loc) const;
    bool PreCallValidateCreateDescriptorSetLayout(VkDevice device, const VkDescriptorSetLayoutCreateInfo* pCreateInfo,
                                                  const VkAllocationCallbacks* pAllocator, VkDescriptorSetLayout* pSetLayout,
                                                  const ErrorObject& error_obj) const override;
    bool PreCallValidateResetDescriptorPool(VkDevice device, VkDescriptorPool descriptorPool, VkDescriptorPoolResetFlags flags,
                                            const ErrorObject& error_obj) const override;
    bool PreCallValidateFreeDescriptorSets(VkDevice device, VkDescriptorPool descriptorPool, uint32_t count,
                                           const VkDescriptorSet* pDescriptorSets, const ErrorObject& error_obj) const override;
    bool PreCallValidateUpdateDescriptorSets(VkDevice device, uint32_t descriptorWriteCount,
                                             const VkWriteDescriptorSet* pDescriptorWrites, uint32_t descriptorCopyCount,
                                             const VkCopyDescriptorSet* pDescriptorCopies,
                                             const ErrorObject& error_obj) const override;
    bool PreCallValidateBeginCommandBuffer(VkCommandBuffer commandBuffer, const VkCommandBufferBeginInfo* pBeginInfo,
                                           const ErrorObject& error_obj) const override;
    bool ValidateRenderingInfoAttachment(const std::shared_ptr<const vvl::ImageView>& image_view,
                                         const VkRenderingInfo* pRenderingInfo, const LogObjectList& objlist,
                                         const Location& loc) const;
    bool ValidateBeginRenderingFragmentDensityMap(VkCommandBuffer commandBuffer, const VkRenderingInfo* pRenderingInfo,
                                                  const Location& rendering_info) const;
    bool ValidateBeginRenderingFragmentShadingRate(VkCommandBuffer commandBuffer, const VkRenderingInfo* pRenderingInfo,
                                                   const Location& rendering_info) const;
    bool ValidateBeginRenderingDeviceGroup(VkCommandBuffer commandBuffer, const VkRenderingInfo* pRenderingInfo,
                                           const Location& rendering_info) const;
    bool ValidateBeginRenderingMultisampledRenderToSingleSampled(VkCommandBuffer commandBuffer,
                                                                 const VkRenderingInfo* pRenderingInfo,
                                                                 const Location& rendering_info) const;
    bool PreCallValidateCmdBeginRenderingKHR(VkCommandBuffer commandBuffer, const VkRenderingInfoKHR* pRenderingInfo,
                                             const ErrorObject& error_obj) const override;
    bool PreCallValidateCmdBeginRendering(VkCommandBuffer commandBuffer, const VkRenderingInfo* pRenderingInfo,
                                          const ErrorObject& error_obj) const override;
    bool ValidateRenderingAttachmentInfo(VkCommandBuffer commandBuffer, const VkRenderingInfo* pRenderingInfo,
                                         const VkRenderingAttachmentInfo& attachment_info, const Location& loc) const;
    bool PreCallValidateCmdEndRenderingKHR(VkCommandBuffer commandBuffer, const ErrorObject& error_obj) const override;
    bool PreCallValidateCmdEndRendering(VkCommandBuffer commandBuffer, const ErrorObject& error_obj) const override;
    bool PreCallValidateEndCommandBuffer(VkCommandBuffer commandBuffer, const ErrorObject& error_obj) const override;
    bool PreCallValidateResetCommandBuffer(VkCommandBuffer commandBuffer, VkCommandBufferResetFlags flags,
                                           const ErrorObject& error_obj) const override;
    bool PreCallValidateCmdBindPipeline(VkCommandBuffer commandBuffer, VkPipelineBindPoint pipelineBindPoint, VkPipeline pipeline,
                                        const ErrorObject& error_obj) const override;
    bool PreCallValidateCreateShadersEXT(VkDevice device, uint32_t createInfoCount, const VkShaderCreateInfoEXT* pCreateInfos,
                                         const VkAllocationCallbacks* pAllocator, VkShaderEXT* pShaders,
                                         const ErrorObject& error_obj) const override;
    bool PreCallValidateDestroyShaderEXT(VkDevice device, VkShaderEXT shader, const VkAllocationCallbacks* pAllocator,
                                         const ErrorObject& error_obj) const override;
    bool PreCallValidateCmdBindShadersEXT(VkCommandBuffer commandBuffer, uint32_t stageCount, const VkShaderStageFlagBits* pStages,
                                          const VkShaderEXT* pShaders, const ErrorObject& error_obj) const override;
    bool PreCallValidateGetShaderBinaryDataEXT(VkDevice device, VkShaderEXT shader, size_t* pDataSize, void* pData,
                                               const ErrorObject& error_obj) const override;
    bool ForbidInheritedViewportScissor(const vvl::CommandBuffer& cb_state, const char* vuid, const Location& loc) const;
    bool PreCallValidateCmdSetViewport(VkCommandBuffer commandBuffer, uint32_t firstViewport, uint32_t viewportCount,
                                       const VkViewport* pViewports, const ErrorObject& error_obj) const override;
    bool PreCallValidateCmdSetScissor(VkCommandBuffer commandBuffer, uint32_t firstScissor, uint32_t scissorCount,
                                      const VkRect2D* pScissors, const ErrorObject& error_obj) const override;
    bool PreCallValidateCmdSetExclusiveScissorNV(VkCommandBuffer commandBuffer, uint32_t firstExclusiveScissor,
                                                 uint32_t exclusiveScissorCount, const VkRect2D* pExclusiveScissors,
                                                 const ErrorObject& error_obj) const override;

    bool PreCallValidateCmdSetViewportWScalingNV(VkCommandBuffer commandBuffer, uint32_t firstViewport, uint32_t viewportCount,
                                                 const VkViewportWScalingNV* pViewportWScalings,
                                                 const ErrorObject& error_obj) const override;

    bool PreCallValidateCmdBindShadingRateImageNV(VkCommandBuffer commandBuffer, VkImageView imageView, VkImageLayout imageLayout,
                                                  const ErrorObject& error_obj) const override;
    bool PreCallValidateCmdSetViewportShadingRatePaletteNV(VkCommandBuffer commandBuffer, uint32_t firstViewport,
                                                           uint32_t viewportCount,
                                                           const VkShadingRatePaletteNV* pShadingRatePalettes,
                                                           const ErrorObject& error_obj) const override;
    bool ValidateGeometryTrianglesNV(const VkGeometryTrianglesNV& triangles, const Location& loc) const;
    bool ValidateGeometryAABBNV(const VkGeometryAABBNV& geometry, const Location& loc) const;
    bool ValidateGeometryNV(const VkGeometryNV& geometry, const Location& loc) const;
    bool PreCallValidateCreateAccelerationStructureNV(VkDevice device, const VkAccelerationStructureCreateInfoNV* pCreateInfo,
                                                      const VkAllocationCallbacks* pAllocator,
                                                      VkAccelerationStructureNV* pAccelerationStructure,
                                                      const ErrorObject& error_obj) const override;
    bool PreCallValidateCreateAccelerationStructureKHR(VkDevice device, const VkAccelerationStructureCreateInfoKHR* pCreateInfo,
                                                       const VkAllocationCallbacks* pAllocator,
                                                       VkAccelerationStructureKHR* pAccelerationStructure,
                                                       const ErrorObject& error_obj) const override;
    bool PreCallValidateBindAccelerationStructureMemoryNV(VkDevice device, uint32_t bindInfoCount,
                                                          const VkBindAccelerationStructureMemoryInfoNV* pBindInfos,
                                                          const ErrorObject& error_obj) const override;
    bool PreCallValidateGetAccelerationStructureHandleNV(VkDevice device, VkAccelerationStructureNV accelerationStructure,
                                                         size_t dataSize, void* pData, const ErrorObject& error_obj) const override;
    // Validate buffers accessed using a device address
    bool ValidateAccelerationBuffers(VkCommandBuffer cmd_buffer, uint32_t info_i,
                                     const VkAccelerationStructureBuildGeometryInfoKHR& info,
                                     const VkAccelerationStructureBuildRangeInfoKHR* geometry_build_ranges,
                                     const Location& info_loc) const;
    bool ValidateAccelerationStructuresMemoryAlisasing(VkCommandBuffer commandBuffer, uint32_t infoCount,
                                                       const VkAccelerationStructureBuildGeometryInfoKHR* pInfos, uint32_t info_i,
                                                       const ErrorObject& error_obj) const;
    bool PreCallValidateCmdBuildAccelerationStructuresKHR(VkCommandBuffer commandBuffer, uint32_t infoCount,
                                                          const VkAccelerationStructureBuildGeometryInfoKHR* pInfos,
                                                          const VkAccelerationStructureBuildRangeInfoKHR* const* ppBuildRangeInfos,
                                                          const ErrorObject& error_obj) const override;

    bool PreCallValidateBuildAccelerationStructuresKHR(VkDevice device, VkDeferredOperationKHR deferredOperation,
                                                       uint32_t infoCount,
                                                       const VkAccelerationStructureBuildGeometryInfoKHR* pInfos,
                                                       const VkAccelerationStructureBuildRangeInfoKHR* const* ppBuildRangeInfos,
                                                       const ErrorObject& error_obj) const override;
    bool PreCallValidateCmdBuildAccelerationStructureNV(VkCommandBuffer commandBuffer, const VkAccelerationStructureInfoNV* pInfo,
                                                        VkBuffer instanceData, VkDeviceSize instanceOffset, VkBool32 update,
                                                        VkAccelerationStructureNV dst, VkAccelerationStructureNV src,
                                                        VkBuffer scratch, VkDeviceSize scratchOffset,
                                                        const ErrorObject& error_obj) const override;
    bool PreCallValidateCmdCopyAccelerationStructureNV(VkCommandBuffer commandBuffer, VkAccelerationStructureNV dst,
                                                       VkAccelerationStructureNV src, VkCopyAccelerationStructureModeNV mode,
                                                       const ErrorObject& error_obj) const override;
    bool PreCallValidateDestroyAccelerationStructureNV(VkDevice device, VkAccelerationStructureNV accelerationStructure,
                                                       const VkAllocationCallbacks* pAllocator,
                                                       const ErrorObject& error_obj) const override;
    bool PreCallValidateDestroyAccelerationStructureKHR(VkDevice device, VkAccelerationStructureKHR accelerationStructure,
                                                        const VkAllocationCallbacks* pAllocator,
                                                        const ErrorObject& error_obj) const override;
    bool PreCallValidateCmdSetLineWidth(VkCommandBuffer commandBuffer, float lineWidth,
                                        const ErrorObject& error_obj) const override;
    bool PreCallValidateCmdSetLineStippleEXT(VkCommandBuffer commandBuffer, uint32_t lineStippleFactor, uint16_t lineStipplePattern,
                                             const ErrorObject& error_obj) const override;
    bool PreCallValidateCmdSetDepthBias(VkCommandBuffer commandBuffer, float depthBiasConstantFactor, float depthBiasClamp,
                                        float depthBiasSlopeFactor, const ErrorObject& error_obj) const override;
    bool ValidateDepthBiasRepresentationInfo(const Location& loc, const LogObjectList& objlist,
                                             const VkDepthBiasRepresentationInfoEXT& depth_bias_representation) const;
    bool PreCallValidateCmdSetDepthBias2EXT(VkCommandBuffer commandBuffer, const VkDepthBiasInfoEXT* pDepthBiasInfo,
                                            const ErrorObject& error_obj) const override;
    bool PreCallValidateCmdSetBlendConstants(VkCommandBuffer commandBuffer, const float blendConstants[4],
                                             const ErrorObject& error_obj) const override;
    bool PreCallValidateCmdSetDepthBounds(VkCommandBuffer commandBuffer, float minDepthBounds, float maxDepthBounds,
                                          const ErrorObject& error_obj) const override;
    bool PreCallValidateCmdSetStencilCompareMask(VkCommandBuffer commandBuffer, VkStencilFaceFlags faceMask, uint32_t compareMask,
                                                 const ErrorObject& error_obj) const override;
    bool PreCallValidateCmdSetStencilWriteMask(VkCommandBuffer commandBuffer, VkStencilFaceFlags faceMask, uint32_t writeMask,
                                               const ErrorObject& error_obj) const override;
    bool PreCallValidateCmdSetStencilReference(VkCommandBuffer commandBuffer, VkStencilFaceFlags faceMask, uint32_t reference,
                                               const ErrorObject& error_obj) const override;
    bool ValidateCmdBindDescriptorSets(const vvl::CommandBuffer&, VkPipelineLayout layout, uint32_t firstSet, uint32_t setCount,
                                       const VkDescriptorSet* pDescriptorSets, uint32_t dynamicOffsetCount,
                                       const uint32_t* pDynamicOffsets, const Location& loc) const;
    bool PreCallValidateCmdBindDescriptorSets(VkCommandBuffer commandBuffer, VkPipelineBindPoint pipelineBindPoint,
                                              VkPipelineLayout layout, uint32_t firstSet, uint32_t setCount,
                                              const VkDescriptorSet* pDescriptorSets, uint32_t dynamicOffsetCount,
                                              const uint32_t* pDynamicOffsets, const ErrorObject& error_obj) const override;
    bool PreCallValidateCmdBindDescriptorSets2KHR(VkCommandBuffer commandBuffer,
                                                  const VkBindDescriptorSetsInfoKHR* pBindDescriptorSetsInfo,
                                                  const ErrorObject& error_obj) const override;
    bool ValidateCmdPushDescriptorSet(const vvl::CommandBuffer& cb_state, VkPipelineLayout layout, uint32_t set,
                                      uint32_t descriptorWriteCount, const VkWriteDescriptorSet* pDescriptorWrites,
                                      const Location& loc) const;
    bool PreCallValidateCmdPushDescriptorSetKHR(VkCommandBuffer commandBuffer, VkPipelineBindPoint pipelineBindPoint,
                                                VkPipelineLayout layout, uint32_t set, uint32_t descriptorWriteCount,
                                                const VkWriteDescriptorSet* pDescriptorWrites,
                                                const ErrorObject& error_obj) const override;
    bool PreCallValidateCmdPushDescriptorSet2KHR(VkCommandBuffer commandBuffer,
                                                 const VkPushDescriptorSetInfoKHR* pPushDescriptorSetInfo,
                                                 const ErrorObject& error_obj) const override;
    bool ValidateCmdBindIndexBuffer(const vvl::CommandBuffer& cb_state, VkBuffer buffer, VkDeviceSize offset, VkIndexType indexType,
                                    const Location& loc) const;
    bool PreCallValidateCmdBindIndexBuffer(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                           VkIndexType indexType, const ErrorObject& error_obj) const override;
    bool PreCallValidateCmdBindIndexBuffer2KHR(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                               VkDeviceSize size, VkIndexType indexType,
                                               const ErrorObject& error_obj) const override;
    bool PreCallValidateCmdBindVertexBuffers(VkCommandBuffer commandBuffer, uint32_t firstBinding, uint32_t bindingCount,
                                             const VkBuffer* pBuffers, const VkDeviceSize* pOffsets,
                                             const ErrorObject& error_obj) const override;
    bool ValidateVTGShaderStages(const vvl::CommandBuffer& cb_state, const Location& loc) const;
    bool ValidateMeshShaderStage(const vvl::CommandBuffer& cb_state, const Location& loc, bool is_NV) const;
    bool PreCallValidateCmdDraw(VkCommandBuffer commandBuffer, uint32_t vertexCount, uint32_t instanceCount, uint32_t firstVertex,
                                uint32_t firstInstance, const ErrorObject& error_obj) const override;
    bool PreCallValidateCmdDrawMultiEXT(VkCommandBuffer commandBuffer, uint32_t drawCount, const VkMultiDrawInfoEXT* pVertexInfo,
                                        uint32_t instanceCount, uint32_t firstInstance, uint32_t stride,
                                        const ErrorObject& error_obj) const override;
    bool ValidateCmdDrawIndexedBufferSize(const vvl::CommandBuffer& cb_state, uint32_t indexCount, uint32_t firstIndex,
                                          const Location& loc, const char* first_index_vuid) const;
    bool PreCallValidateCmdDrawIndexed(VkCommandBuffer commandBuffer, uint32_t indexCount, uint32_t instanceCount,
                                       uint32_t firstIndex, int32_t vertexOffset, uint32_t firstInstance,
                                       const ErrorObject& error_obj) const override;
    bool PreCallValidateCmdDrawMultiIndexedEXT(VkCommandBuffer commandBuffer, uint32_t drawCount,
                                               const VkMultiDrawIndexedInfoEXT* pIndexInfo, uint32_t instanceCount,
                                               uint32_t firstInstance, uint32_t stride, const int32_t* pVertexOffset,
                                               const ErrorObject& error_obj) const override;
    bool PreCallValidateCmdDrawIndexedIndirect(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                               uint32_t drawCount, uint32_t stride, const ErrorObject& error_obj) const override;
    bool PreCallValidateCmdDrawIndexedIndirectCountKHR(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                       VkBuffer countBuffer, VkDeviceSize countBufferOffset, uint32_t maxDrawCount,
                                                       uint32_t stride, const ErrorObject& error_obj) const override;
    bool PreCallValidateCmdDrawIndexedIndirectCount(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                    VkBuffer countBuffer, VkDeviceSize countBufferOffset, uint32_t maxDrawCount,
                                                    uint32_t stride, const ErrorObject& error_obj) const override;
    bool PreCallValidateCmdDispatch(VkCommandBuffer commandBuffer, uint32_t groupCountX, uint32_t groupCountY, uint32_t groupCountZ,
                                    const ErrorObject& error_obj) const override;
    bool PreCallValidateCmdDispatchIndirect(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                            const ErrorObject& error_obj) const override;
    bool PreCallValidateCmdDrawIndirect(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset, uint32_t drawCount,
                                        uint32_t stride, const ErrorObject& error_obj) const override;
    bool PreCallValidateCmdDispatchBase(VkCommandBuffer commandBuffer, uint32_t baseGroupX, uint32_t baseGroupY,
                                        uint32_t baseGroupZ, uint32_t groupCountX, uint32_t groupCountY, uint32_t groupCountZ,
                                        const ErrorObject& error_obj) const override;
    bool PreCallValidateCmdDispatchBaseKHR(VkCommandBuffer commandBuffer, uint32_t baseGroupX, uint32_t baseGroupY,
                                           uint32_t baseGroupZ, uint32_t groupCountX, uint32_t groupCountY, uint32_t groupCountZ,
                                           const ErrorObject& error_obj) const override;
    bool PreCallValidateCmdSetEvent(VkCommandBuffer commandBuffer, VkEvent event, VkPipelineStageFlags stageMask,
                                    const ErrorObject& error_obj) const override;
    bool PreCallValidateCmdSetEvent2KHR(VkCommandBuffer commandBuffer, VkEvent event, const VkDependencyInfoKHR* pDependencyInfo,
                                        const ErrorObject& error_obj) const override;
    bool PreCallValidateCmdSetEvent2(VkCommandBuffer commandBuffer, VkEvent event, const VkDependencyInfo* pDependencyInfo,
                                     const ErrorObject& error_obj) const override;
    bool PreCallValidateCmdResetEvent(VkCommandBuffer commandBuffer, VkEvent event, VkPipelineStageFlags stageMask,
                                      const ErrorObject& error_obj) const override;
    bool PreCallValidateCmdResetEvent2KHR(VkCommandBuffer commandBuffer, VkEvent event, VkPipelineStageFlags2KHR stageMask,
                                          const ErrorObject& error_obj) const override;
    bool PreCallValidateCmdResetEvent2(VkCommandBuffer commandBuffer, VkEvent event, VkPipelineStageFlags2 stageMask,
                                       const ErrorObject& error_obj) const override;
    bool PreCallValidateCmdWaitEvents(VkCommandBuffer commandBuffer, uint32_t eventCount, const VkEvent* pEvents,
                                      VkPipelineStageFlags sourceStageMask, VkPipelineStageFlags dstStageMask,
                                      uint32_t memoryBarrierCount, const VkMemoryBarrier* pMemoryBarriers,
                                      uint32_t bufferMemoryBarrierCount, const VkBufferMemoryBarrier* pBufferMemoryBarriers,
                                      uint32_t imageMemoryBarrierCount, const VkImageMemoryBarrier* pImageMemoryBarriers,
                                      const ErrorObject& error_obj) const override;
    bool PreCallValidateCmdWaitEvents2KHR(VkCommandBuffer commandBuffer, uint32_t eventCount, const VkEvent* pEvents,
                                          const VkDependencyInfoKHR* pDependencyInfos, const ErrorObject& error_obj) const override;
    bool PreCallValidateCmdWaitEvents2(VkCommandBuffer commandBuffer, uint32_t eventCount, const VkEvent* pEvents,
                                       const VkDependencyInfo* pDependencyInfos, const ErrorObject& error_obj) const override;
    void PreCallRecordCmdWaitEvents(VkCommandBuffer commandBuffer, uint32_t eventCount, const VkEvent* pEvents,
                                    VkPipelineStageFlags sourceStageMask, VkPipelineStageFlags dstStageMask,
                                    uint32_t memoryBarrierCount, const VkMemoryBarrier* pMemoryBarriers,
                                    uint32_t bufferMemoryBarrierCount, const VkBufferMemoryBarrier* pBufferMemoryBarriers,
                                    uint32_t imageMemoryBarrierCount, const VkImageMemoryBarrier* pImageMemoryBarriers,
                                    const RecordObject& record_obj) override;
    void RecordCmdWaitEvents2(VkCommandBuffer commandBuffer, uint32_t eventCount, const VkEvent* pEvents,
                              const VkDependencyInfo* pDependencyInfos, Func command);
    void PreCallRecordCmdWaitEvents2KHR(VkCommandBuffer commandBuffer, uint32_t eventCount, const VkEvent* pEvents,
                                        const VkDependencyInfoKHR* pDependencyInfos, const RecordObject& record_obj) override;
    void PreCallRecordCmdWaitEvents2(VkCommandBuffer commandBuffer, uint32_t eventCount, const VkEvent* pEvents,
                                     const VkDependencyInfo* pDependencyInfos, const RecordObject& record_obj) override;
    void PostCallRecordCmdWaitEvents(VkCommandBuffer commandBuffer, uint32_t eventCount, const VkEvent* pEvents,
                                     VkPipelineStageFlags sourceStageMask, VkPipelineStageFlags dstStageMask,
                                     uint32_t memoryBarrierCount, const VkMemoryBarrier* pMemoryBarriers,
                                     uint32_t bufferMemoryBarrierCount, const VkBufferMemoryBarrier* pBufferMemoryBarriers,
                                     uint32_t imageMemoryBarrierCount, const VkImageMemoryBarrier* pImageMemoryBarriers,
                                     const RecordObject& record_obj) override;
    void PostCallRecordCmdWaitEvents2KHR(VkCommandBuffer commandBuffer, uint32_t eventCount, const VkEvent* pEvents,
                                         const VkDependencyInfoKHR* pDependencyInfos, const RecordObject& record_obj) override;
    void PostCallRecordCmdWaitEvents2(VkCommandBuffer commandBuffer, uint32_t eventCount, const VkEvent* pEvents,
                                      const VkDependencyInfo* pDependencyInfos, const RecordObject& record_obj) override;
    bool PreCallValidateCmdPipelineBarrier(VkCommandBuffer commandBuffer, VkPipelineStageFlags srcStageMask,
                                           VkPipelineStageFlags dstStageMask, VkDependencyFlags dependencyFlags,
                                           uint32_t memoryBarrierCount, const VkMemoryBarrier* pMemoryBarriers,
                                           uint32_t bufferMemoryBarrierCount, const VkBufferMemoryBarrier* pBufferMemoryBarriers,
                                           uint32_t imageMemoryBarrierCount, const VkImageMemoryBarrier* pImageMemoryBarriers,
                                           const ErrorObject& error_obj) const override;
    bool PreCallValidateCmdPipelineBarrier2KHR(VkCommandBuffer commandBuffer, const VkDependencyInfoKHR* pDependencyInfo,
                                               const ErrorObject& error_obj) const override;
    bool PreCallValidateCmdPipelineBarrier2(VkCommandBuffer commandBuffer, const VkDependencyInfo* pDependencyInfo,
                                            const ErrorObject& error_obj) const override;
    void PreCallRecordCmdPipelineBarrier(VkCommandBuffer commandBuffer, VkPipelineStageFlags srcStageMask,
                                         VkPipelineStageFlags dstStageMask, VkDependencyFlags dependencyFlags,
                                         uint32_t memoryBarrierCount, const VkMemoryBarrier* pMemoryBarriers,
                                         uint32_t bufferMemoryBarrierCount, const VkBufferMemoryBarrier* pBufferMemoryBarriers,
                                         uint32_t imageMemoryBarrierCount, const VkImageMemoryBarrier* pImageMemoryBarriers,
                                         const RecordObject& record_obj) override;
    void PreCallRecordCmdPipelineBarrier2KHR(VkCommandBuffer commandBuffer, const VkDependencyInfoKHR* pDependencyInfo,
                                             const RecordObject& record_obj) override;
    void PreCallRecordCmdPipelineBarrier2(VkCommandBuffer commandBuffer, const VkDependencyInfo* pDependencyInfo,
                                          const RecordObject& record_obj) override;

    void EnqueueVerifyBeginQuery(VkCommandBuffer, const QueryObject& query_obj, Func command);
    bool PreCallValidateCmdBeginQuery(VkCommandBuffer commandBuffer, VkQueryPool queryPool, uint32_t slot,
                                      VkQueryControlFlags flags, const ErrorObject& error_obj) const override;
    void PreCallRecordCmdBeginQuery(VkCommandBuffer commandBuffer, VkQueryPool queryPool, uint32_t slot, VkQueryControlFlags flags,
                                    const RecordObject& record_obj) override;
    void EnqueueVerifyEndQuery(vvl::CommandBuffer& cb_state, const QueryObject& query_obj, Func command);
    bool PreCallValidateCmdEndQuery(VkCommandBuffer commandBuffer, VkQueryPool queryPool, uint32_t slot,
                                    const ErrorObject& error_obj) const override;
    void PreCallRecordCmdEndQuery(VkCommandBuffer commandBuffer, VkQueryPool queryPool, uint32_t slot,
                                  const RecordObject& record_obj) override;
    bool ValidateQueryPoolIndex(const vvl::QueryPool& query_pool_state, uint32_t firstQuery, uint32_t queryCount,
                                const Location& loc, const char* first_vuid, const char* sum_vuid) const;
    bool ValidateQueriesNotActive(const vvl::CommandBuffer& cb_state, VkQueryPool queryPool, uint32_t firstQuery,
                                  uint32_t queryCount, const Location& loc, const char* vuid) const;
    bool PreCallValidateCmdResetQueryPool(VkCommandBuffer commandBuffer, VkQueryPool queryPool, uint32_t firstQuery,
                                          uint32_t queryCount, const ErrorObject& error_obj) const override;
    void PreCallRecordCmdResetQueryPool(VkCommandBuffer commandBuffer, VkQueryPool queryPool, uint32_t firstQuery,
                                        uint32_t queryCount, const RecordObject& record_obj) override;
    bool PreCallValidateCmdCopyQueryPoolResults(VkCommandBuffer commandBuffer, VkQueryPool queryPool, uint32_t firstQuery,
                                                uint32_t queryCount, VkBuffer dstBuffer, VkDeviceSize dstOffset,
                                                VkDeviceSize stride, VkQueryResultFlags flags,
                                                const ErrorObject& error_obj) const override;
    void PreCallRecordCmdCopyQueryPoolResults(VkCommandBuffer commandBuffer, VkQueryPool queryPool, uint32_t firstQuery,
                                              uint32_t queryCount, VkBuffer dstBuffer, VkDeviceSize dstOffset, VkDeviceSize stride,
                                              VkQueryResultFlags flags, const RecordObject& record_obj) override;
    bool ValidateCmdPushConstants(VkCommandBuffer commandBuffer, VkPipelineLayout layout, VkShaderStageFlags stageFlags,
                                  uint32_t offset, uint32_t size, const Location& loc) const;
    bool PreCallValidateCmdPushConstants2KHR(VkCommandBuffer commandBuffer, const VkPushConstantsInfoKHR* pPushConstantsInfo,
                                             const ErrorObject& error_obj) const override;
    bool PreCallValidateCmdPushConstants(VkCommandBuffer commandBuffer, VkPipelineLayout layout, VkShaderStageFlags stageFlags,
                                         uint32_t offset, uint32_t size, const void* pValues,
                                         const ErrorObject& error_obj) const override;
    bool ValidateCmdWriteTimestamp(const vvl::CommandBuffer& cb_state, VkQueryPool queryPool, uint32_t query,
                                   const Location& loc) const;
    bool PreCallValidateCmdWriteTimestamp(VkCommandBuffer commandBuffer, VkPipelineStageFlagBits pipelineStage,
                                          VkQueryPool queryPool, uint32_t query, const ErrorObject& error_obj) const override;

    bool PreCallValidateCmdWriteTimestamp2KHR(VkCommandBuffer commandBuffer, VkPipelineStageFlags2KHR stage, VkQueryPool queryPool,
                                              uint32_t query, const ErrorObject& error_obj) const override;
    bool PreCallValidateCmdWriteTimestamp2(VkCommandBuffer commandBuffer, VkPipelineStageFlags2 stage, VkQueryPool queryPool,
                                           uint32_t query, const ErrorObject& error_obj) const override;
    void RecordCmdWriteTimestamp2(vvl::CommandBuffer& cb_state, VkQueryPool queryPool, uint32_t query, Func command) const;
    void PreCallRecordCmdWriteTimestamp(VkCommandBuffer commandBuffer, VkPipelineStageFlagBits pipelineStage, VkQueryPool queryPool,
                                        uint32_t slot, const RecordObject& record_obj) override;
    void PreCallRecordCmdWriteTimestamp2KHR(VkCommandBuffer commandBuffer, VkPipelineStageFlags2KHR stage, VkQueryPool queryPool,
                                            uint32_t query, const RecordObject& record_obj) override;
    void PreCallRecordCmdWriteTimestamp2(VkCommandBuffer commandBuffer, VkPipelineStageFlags2 stage, VkQueryPool queryPool,
                                         uint32_t query, const RecordObject& record_obj) override;
    void PreCallRecordCmdWriteAccelerationStructuresPropertiesKHR(VkCommandBuffer commandBuffer,
                                                                  uint32_t accelerationStructureCount,
                                                                  const VkAccelerationStructureKHR* pAccelerationStructures,
                                                                  VkQueryType queryType, VkQueryPool queryPool, uint32_t firstQuery,
                                                                  const RecordObject& record_obj) override;
    bool PreCallValidateCreateFramebuffer(VkDevice device, const VkFramebufferCreateInfo* pCreateInfo,
                                          const VkAllocationCallbacks* pAllocator, VkFramebuffer* pFramebuffer,
                                          const ErrorObject& error_obj) const override;
    bool PreCallValidateCreateRenderPass(VkDevice device, const VkRenderPassCreateInfo* pCreateInfo,
                                         const VkAllocationCallbacks* pAllocator, VkRenderPass* pRenderPass,
                                         const ErrorObject& error_obj) const override;
    bool PreCallValidateGetDeviceMemoryCommitment(VkDevice device, VkDeviceMemory memory, VkDeviceSize* pCommittedMem,
                                                  const ErrorObject& error_obj) const override;
    bool MsRenderedToSingleSampledValidateFBAttachments(uint32_t count, const VkAttachmentReference2* attachments,
                                                        const VkFramebufferCreateInfo* fbci, const VkRenderPassCreateInfo2* rpci,
                                                        uint32_t subpass, VkSampleCountFlagBits sample_count,
                                                        const Location& create_info_loc) const;
    bool ValidateFragmentShadingRateAttachments(const VkRenderPassCreateInfo2* pCreateInfo, const ErrorObject& error_obj) const;
    bool PreCallValidateCreateRenderPass2KHR(VkDevice device, const VkRenderPassCreateInfo2* pCreateInfo,
                                             const VkAllocationCallbacks* pAllocator, VkRenderPass* pRenderPass,
                                             const ErrorObject& error_obj) const override;
    bool PreCallValidateCreateRenderPass2(VkDevice device, const VkRenderPassCreateInfo2* pCreateInfo,
                                          const VkAllocationCallbacks* pAllocator, VkRenderPass* pRenderPass,
                                          const ErrorObject& error_obj) const override;
    bool PreCallValidateCmdBeginRenderPass(VkCommandBuffer commandBuffer, const VkRenderPassBeginInfo* pRenderPassBegin,
                                           VkSubpassContents contents, const ErrorObject& error_obj) const override;
    void PreCallRecordCmdBeginRenderPass(VkCommandBuffer commandBuffer, const VkRenderPassBeginInfo* pRenderPassBegin,
                                         VkSubpassContents contents, const RecordObject& record_obj) override;
    bool PreCallValidateCmdBeginRenderPass2KHR(VkCommandBuffer commandBuffer, const VkRenderPassBeginInfo* pRenderPassBegin,
                                               const VkSubpassBeginInfo* pSubpassBeginInfo,
                                               const ErrorObject& error_obj) const override;
    bool PreCallValidateCmdBeginRenderPass2(VkCommandBuffer commandBuffer, const VkRenderPassBeginInfo* pRenderPassBegin,
                                            const VkSubpassBeginInfo* pSubpassBeginInfo,
                                            const ErrorObject& error_obj) const override;
    void PreCallRecordCmdBeginRenderPass2KHR(VkCommandBuffer commandBuffer, const VkRenderPassBeginInfo* pRenderPassBegin,
                                             const VkSubpassBeginInfo* pSubpassBeginInfo, const RecordObject& record_obj) override;
    void PreCallRecordCmdBeginRenderPass2(VkCommandBuffer commandBuffer, const VkRenderPassBeginInfo* pRenderPassBegin,
                                          const VkSubpassBeginInfo* pSubpassBeginInfo, const RecordObject& record_obj) override;
    bool PreCallValidateCmdNextSubpass(VkCommandBuffer commandBuffer, VkSubpassContents contents,
                                       const ErrorObject& error_obj) const override;
    void PostCallRecordCmdNextSubpass(VkCommandBuffer commandBuffer, VkSubpassContents contents,
                                      const RecordObject& record_obj) override;
    bool PreCallValidateCmdNextSubpass2KHR(VkCommandBuffer commandBuffer, const VkSubpassBeginInfo* pSubpassBeginInfo,
                                           const VkSubpassEndInfo* pSubpassEndInfo, const ErrorObject& error_obj) const override;
    void PostCallRecordCmdNextSubpass2KHR(VkCommandBuffer commandBuffer, const VkSubpassBeginInfo* pSubpassBeginInfo,
                                          const VkSubpassEndInfo* pSubpassEndInfo, const RecordObject& record_obj) override;
    bool PreCallValidateCmdNextSubpass2(VkCommandBuffer commandBuffer, const VkSubpassBeginInfo* pSubpassBeginInfo,
                                        const VkSubpassEndInfo* pSubpassEndInfo, const ErrorObject& error_obj) const override;
    void PostCallRecordCmdNextSubpass2(VkCommandBuffer commandBuffer, const VkSubpassBeginInfo* pSubpassBeginInfo,
                                       const VkSubpassEndInfo* pSubpassEndInfo, const RecordObject& record_obj) override;
    bool PreCallValidateCmdEndRenderPass(VkCommandBuffer commandBuffer, const ErrorObject& error_obj) const override;
    void PostCallRecordCmdEndRenderPass(VkCommandBuffer commandBuffer, const RecordObject& record_obj) override;
    bool PreCallValidateCmdEndRenderPass2KHR(VkCommandBuffer commandBuffer, const VkSubpassEndInfo* pSubpassEndInfo,
                                             const ErrorObject& error_obj) const override;
    void PostCallRecordCmdEndRenderPass2KHR(VkCommandBuffer commandBuffer, const VkSubpassEndInfo* pSubpassEndInfo,
                                            const RecordObject& record_obj) override;
    bool PreCallValidateCmdEndRenderPass2(VkCommandBuffer commandBuffer, const VkSubpassEndInfo* pSubpassEndInfo,
                                          const ErrorObject& error_obj) const override;
    void PostCallRecordCmdEndRenderPass2(VkCommandBuffer commandBuffer, const VkSubpassEndInfo* pSubpassEndInfo,
                                         const RecordObject& record_obj) override;
    class ViewportScissorInheritanceTracker;
    bool PreCallValidateCmdExecuteCommands(VkCommandBuffer commandBuffer, uint32_t commandBuffersCount,
                                           const VkCommandBuffer* pCommandBuffers, const ErrorObject& error_obj) const override;
    bool PreCallValidateMapMemory(VkDevice device, VkDeviceMemory memory, VkDeviceSize offset, VkDeviceSize size, VkFlags flags,
                                  void** ppData, const ErrorObject& error_obj) const override;
    bool PreCallValidateUnmapMemory(VkDevice device, VkDeviceMemory mememorym, const ErrorObject& error_obj) const override;
    bool PreCallValidateFlushMappedMemoryRanges(VkDevice device, uint32_t memoryRangeCount,
                                                const VkMappedMemoryRange* pMemoryRanges,
                                                const ErrorObject& error_obj) const override;
    bool PreCallValidateInvalidateMappedMemoryRanges(VkDevice device, uint32_t memoryRangeCount,
                                                     const VkMappedMemoryRange* pMemoryRanges,
                                                     const ErrorObject& error_obj) const override;
    bool PreCallValidateBindImageMemory(VkDevice device, VkImage image, VkDeviceMemory memory, VkDeviceSize memoryOffset,
                                        const ErrorObject& error_obj) const override;
    void PostCallRecordBindImageMemory(VkDevice device, VkImage image, VkDeviceMemory memory, VkDeviceSize memoryOffset,
                                       const RecordObject& record_obj) override;
    bool PreCallValidateBindImageMemory2(VkDevice device, uint32_t bindInfoCount, const VkBindImageMemoryInfo* pBindInfos,
                                         const ErrorObject& error_obj) const override;
    void PostCallRecordBindImageMemory2(VkDevice device, uint32_t bindInfoCount, const VkBindImageMemoryInfo* pBindInfos,
                                        const RecordObject& record_obj) override;
    bool PreCallValidateBindImageMemory2KHR(VkDevice device, uint32_t bindInfoCount, const VkBindImageMemoryInfo* pBindInfos,
                                            const ErrorObject& error_obj) const override;
    void PostCallRecordBindImageMemory2KHR(VkDevice device, uint32_t bindInfoCount, const VkBindImageMemoryInfo* pBindInfos,
                                           const RecordObject& record_obj) override;
    bool PreCallValidateSetEvent(VkDevice device, VkEvent event, const ErrorObject& error_obj) const override;
    bool PreCallValidateResetEvent(VkDevice device, VkEvent event, const ErrorObject& error_obj) const override;
    bool PreCallValidateGetEventStatus(VkDevice device, VkEvent event, const ErrorObject& error_obj) const override;
    bool ValidateSparseMemoryBind(const VkSparseMemoryBind& bind, const VkMemoryRequirements& requirements,
                                  VkDeviceSize resource_size, VkExternalMemoryHandleTypeFlags external_handle_types,
                                  const VulkanTypedHandle& resource_handle, const Location& loc) const;
    bool ValidateImageSubresourceSparseImageMemoryBind(vvl::Image const& image_state, VkImageSubresource const& subresource,
                                                       const Location& bind_loc, const Location& subresource_loc) const;
    bool ValidateSparseImageMemoryBind(vvl::Image const* image_state, VkSparseImageMemoryBind const& bind, const Location& bind_loc,
                                       const Location& memory_loc) const;
    bool PreCallValidateQueueBindSparse(VkQueue queue, uint32_t bindInfoCount, const VkBindSparseInfo* pBindInfo, VkFence fence,
                                        const ErrorObject& error_obj) const override;
    bool PreCallValidateSignalSemaphore(VkDevice device, const VkSemaphoreSignalInfo* pSignalInfo,
                                        const ErrorObject& error_obj) const override;
    bool PreCallValidateSignalSemaphoreKHR(VkDevice device, const VkSemaphoreSignalInfo* pSignalInfo,
                                           const ErrorObject& error_obj) const override;
    bool PreCallValidateImportSemaphoreFdKHR(VkDevice device, const VkImportSemaphoreFdInfoKHR* pImportSemaphoreFdInfo,
                                             const ErrorObject& error_obj) const override;
    bool PreCallValidateGetSemaphoreFdKHR(VkDevice device, const VkSemaphoreGetFdInfoKHR* pGetFdInfo, int* pFd,
                                          const ErrorObject& error_obj) const override;
#ifdef VK_USE_PLATFORM_FUCHSIA
    bool PreCallValidateImportSemaphoreZirconHandleFUCHSIA(
        VkDevice device, const VkImportSemaphoreZirconHandleInfoFUCHSIA* pImportSemaphoreZirconHandleInfo,
        const ErrorObject& error_obj) const override;

    void PostCallRecordImportSemaphoreZirconHandleFUCHSIA(
        VkDevice device, const VkImportSemaphoreZirconHandleInfoFUCHSIA* pImportSemaphoreZirconHandleInfo,
        const RecordObject& record_obj) override;
    void PostCallRecordGetSemaphoreZirconHandleFUCHSIA(VkDevice device,
                                                       const VkSemaphoreGetZirconHandleInfoFUCHSIA* pGetZirconHandleInfo,
                                                       zx_handle_t* pZirconHandle, const RecordObject& record_obj) override;
#endif
#ifdef VK_USE_PLATFORM_WIN32_KHR
    bool PreCallValidateGetMemoryWin32HandleKHR(VkDevice device, const VkMemoryGetWin32HandleInfoKHR* pGetWin32HandleInfo,
                                                HANDLE* pHandle, const ErrorObject& error_obj) const override;
    bool PreCallValidateImportSemaphoreWin32HandleKHR(VkDevice device,
                                                      const VkImportSemaphoreWin32HandleInfoKHR* pImportSemaphoreWin32HandleInfo,
                                                      const ErrorObject& error_obj) const override;
    bool PreCallValidateGetSemaphoreWin32HandleKHR(VkDevice device, const VkSemaphoreGetWin32HandleInfoKHR* pGetWin32HandleInfo,
                                                   HANDLE* pHandle, const ErrorObject& error_obj) const override;
    bool PreCallValidateImportFenceWin32HandleKHR(VkDevice device,
                                                  const VkImportFenceWin32HandleInfoKHR* pImportFenceWin32HandleInfo,
                                                  const ErrorObject& error_obj) const override;
    bool PreCallValidateGetFenceWin32HandleKHR(VkDevice device, const VkFenceGetWin32HandleInfoKHR* pGetWin32HandleInfo,
                                               HANDLE* pHandle, const ErrorObject& error_obj) const override;
#endif  // VK_USE_PLATFORM_WIN32_KHR
    bool PreCallValidateGetMemoryFdKHR(VkDevice device, const VkMemoryGetFdInfoKHR* pGetFdInfo, int* pFd,
                                       const ErrorObject& error_obj) const override;
    bool PreCallValidateImportFenceFdKHR(VkDevice device, const VkImportFenceFdInfoKHR* pImportFenceFdInfo,
                                         const ErrorObject& error_obj) const override;
    bool PreCallValidateGetFenceFdKHR(VkDevice device, const VkFenceGetFdInfoKHR* pGetFdInfo, int* pFd,
                                      const ErrorObject& error_obj) const override;

    bool PreCallValidateCreateSwapchainKHR(VkDevice device, const VkSwapchainCreateInfoKHR* pCreateInfo,
                                           const VkAllocationCallbacks* pAllocator, VkSwapchainKHR* pSwapchain,
                                           const ErrorObject& error_obj) const override;
    void PreCallRecordDestroySwapchainKHR(VkDevice device, VkSwapchainKHR swapchain, const VkAllocationCallbacks* pAllocator,
                                          const RecordObject& record_obj) override;
    void PostCallRecordGetSwapchainImagesKHR(VkDevice device, VkSwapchainKHR swapchain, uint32_t* pSwapchainImageCount,
                                             VkImage* pSwapchainImages, const RecordObject& record_obj) override;
    bool PreCallValidateQueuePresentKHR(VkQueue queue, const VkPresentInfoKHR* pPresentInfo,
                                        const ErrorObject& error_obj) const override;
    bool ValidateImageAcquireWait(const vvl::SwapchainImage& swapchain_image, uint32_t image_index,
                                  const VkPresentInfoKHR& present_info, const Location present_info_loc) const;
    bool PreCallValidateReleaseSwapchainImagesEXT(VkDevice device, const VkReleaseSwapchainImagesInfoEXT* pReleaseInfo,
                                                  const ErrorObject& error_obj) const override;
    bool PreCallValidateCreateSharedSwapchainsKHR(VkDevice device, uint32_t swapchainCount,
                                                  const VkSwapchainCreateInfoKHR* pCreateInfos,
                                                  const VkAllocationCallbacks* pAllocator, VkSwapchainKHR* pSwapchains,
                                                  const ErrorObject& error_obj) const override;
    bool PreCallValidateAcquireNextImageKHR(VkDevice device, VkSwapchainKHR swapchain, uint64_t timeout, VkSemaphore semaphore,
                                            VkFence fence, uint32_t* pImageIndex, const ErrorObject& error_obj) const override;
    bool PreCallValidateAcquireNextImage2KHR(VkDevice device, const VkAcquireNextImageInfoKHR* pAcquireInfo, uint32_t* pImageIndex,
                                             const ErrorObject& error_obj) const override;
    bool PreCallValidateWaitForPresentKHR(VkDevice device, VkSwapchainKHR swapchain, uint64_t presentId, uint64_t timeout,
                                          const ErrorObject& error_obj) const override;
    bool PreCallValidateDestroySurfaceKHR(VkInstance instance, VkSurfaceKHR surface, const VkAllocationCallbacks* pAllocator,
                                          const ErrorObject& error_obj) const override;
    bool PreCallValidateGetPhysicalDeviceSurfaceSupportKHR(VkPhysicalDevice physicalDevice, uint32_t queueFamilyIndex,
                                                           VkSurfaceKHR surface, VkBool32* pSupported,
                                                           const ErrorObject& error_obj) const override;
    bool PreCallValidateCreateDescriptorUpdateTemplate(VkDevice device, const VkDescriptorUpdateTemplateCreateInfo* pCreateInfo,
                                                       const VkAllocationCallbacks* pAllocator,
                                                       VkDescriptorUpdateTemplate* pDescriptorUpdateTemplate,
                                                       const ErrorObject& error_obj) const override;
    bool PreCallValidateCreateDescriptorUpdateTemplateKHR(VkDevice device, const VkDescriptorUpdateTemplateCreateInfo* pCreateInfo,
                                                          const VkAllocationCallbacks* pAllocator,
                                                          VkDescriptorUpdateTemplate* pDescriptorUpdateTemplate,
                                                          const ErrorObject& error_obj) const override;
    bool PreCallValidateUpdateDescriptorSetWithTemplate(VkDevice device, VkDescriptorSet descriptorSet,
                                                        VkDescriptorUpdateTemplate descriptorUpdateTemplate, const void* pData,
                                                        const ErrorObject& error_obj) const override;
    bool PreCallValidateUpdateDescriptorSetWithTemplateKHR(VkDevice device, VkDescriptorSet descriptorSet,
                                                           VkDescriptorUpdateTemplate descriptorUpdateTemplate, const void* pData,
                                                           const ErrorObject& error_obj) const override;
    bool ValidateCmdPushDescriptorSetWithTemplate(VkCommandBuffer commandBuffer,
                                                  VkDescriptorUpdateTemplate descriptorUpdateTemplate, VkPipelineLayout layout,
                                                  uint32_t set, const void* pData, const Location& loc) const;
    bool PreCallValidateCmdPushDescriptorSetWithTemplate2KHR(
        VkCommandBuffer commandBuffer, const VkPushDescriptorSetWithTemplateInfoKHR* pPushDescriptorSetWithTemplateInfo,
        const ErrorObject& error_obj) const override;
    bool PreCallValidateCmdPushDescriptorSetWithTemplateKHR(VkCommandBuffer commandBuffer,
                                                            VkDescriptorUpdateTemplate descriptorUpdateTemplate,
                                                            VkPipelineLayout layout, uint32_t set, const void* pData,
                                                            const ErrorObject& error_obj) const override;
    bool PreCallValidateGetDisplayPlaneSupportedDisplaysKHR(VkPhysicalDevice physicalDevice, uint32_t planeIndex,
                                                            uint32_t* pDisplayCount, VkDisplayKHR* pDisplays,
                                                            const ErrorObject& error_obj) const override;
    bool PreCallValidateGetDisplayPlaneCapabilitiesKHR(VkPhysicalDevice physicalDevice, VkDisplayModeKHR mode, uint32_t planeIndex,
                                                       VkDisplayPlaneCapabilitiesKHR* pCapabilities,
                                                       const ErrorObject& error_obj) const override;
    bool PreCallValidateGetDisplayPlaneCapabilities2KHR(VkPhysicalDevice physicalDevice,
                                                        const VkDisplayPlaneInfo2KHR* pDisplayPlaneInfo,
                                                        VkDisplayPlaneCapabilities2KHR* pCapabilities,
                                                        const ErrorObject& error_obj) const override;
    bool PreCallValidateCreateDisplayPlaneSurfaceKHR(VkInstance instance, const VkDisplaySurfaceCreateInfoKHR* pCreateInfo,
                                                     const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface,
                                                     const ErrorObject& error_obj) const override;
    bool PreCallValidateCmdDebugMarkerEndEXT(VkCommandBuffer commandBuffer, const ErrorObject& error_obj) const override;

    bool PreCallValidateCmdBeginQueryIndexedEXT(VkCommandBuffer commandBuffer, VkQueryPool queryPool, uint32_t query,
                                                VkQueryControlFlags flags, uint32_t index,
                                                const ErrorObject& error_obj) const override;
    void PreCallRecordCmdBeginQueryIndexedEXT(VkCommandBuffer commandBuffer, VkQueryPool queryPool, uint32_t query,
                                              VkQueryControlFlags flags, uint32_t index, const RecordObject& record_obj) override;
    bool PreCallValidateCmdEndQueryIndexedEXT(VkCommandBuffer commandBuffer, VkQueryPool queryPool, uint32_t query, uint32_t index,
                                              const ErrorObject& error_obj) const override;
    void PreCallRecordCmdEndQueryIndexedEXT(VkCommandBuffer commandBuffer, VkQueryPool queryPool, uint32_t query, uint32_t index,
                                            const RecordObject& record_obj) override;

    bool PreCallValidateGetPhysicalDeviceVideoCapabilitiesKHR(VkPhysicalDevice physicalDevice,
                                                              const VkVideoProfileInfoKHR* pVideoProfile,
                                                              VkVideoCapabilitiesKHR* pCapabilities,
                                                              const ErrorObject& error_obj) const override;
    bool PreCallValidateGetPhysicalDeviceVideoFormatPropertiesKHR(VkPhysicalDevice physicalDevice,
                                                                  const VkPhysicalDeviceVideoFormatInfoKHR* pVideoFormatInfo,
                                                                  uint32_t* pVideoFormatPropertyCount,
                                                                  VkVideoFormatPropertiesKHR* pVideoFormatProperties,
                                                                  const ErrorObject& error_obj) const override;
    bool PreCallValidateGetPhysicalDeviceVideoEncodeQualityLevelPropertiesKHR(
        VkPhysicalDevice physicalDevice, const VkPhysicalDeviceVideoEncodeQualityLevelInfoKHR* pQualityLevelInfo,
        VkVideoEncodeQualityLevelPropertiesKHR* pQualityLevelProperties, const ErrorObject& error_obj) const override;
    bool PreCallValidateCreateVideoSessionKHR(VkDevice device, const VkVideoSessionCreateInfoKHR* pCreateInfo,
                                              const VkAllocationCallbacks* pAllocator, VkVideoSessionKHR* pVideoSession,
                                              const ErrorObject& error_obj) const override;
    bool PreCallValidateDestroyVideoSessionKHR(VkDevice device, VkVideoSessionKHR videoSession,
                                               const VkAllocationCallbacks* pAllocator,
                                               const ErrorObject& error_obj) const override;
    bool PreCallValidateBindVideoSessionMemoryKHR(VkDevice device, VkVideoSessionKHR videoSession,
                                                  uint32_t bindSessionMemoryInfoCount,
                                                  const VkBindVideoSessionMemoryInfoKHR* pBindSessionMemoryInfos,
                                                  const ErrorObject& error_obj) const override;
    bool PreCallValidateCreateVideoSessionParametersKHR(VkDevice device, const VkVideoSessionParametersCreateInfoKHR* pCreateInfo,
                                                        const VkAllocationCallbacks* pAllocator,
                                                        VkVideoSessionParametersKHR* pVideoSessionParameters,
                                                        const ErrorObject& error_obj) const override;
    bool PreCallValidateUpdateVideoSessionParametersKHR(VkDevice device, VkVideoSessionParametersKHR videoSessionParameters,
                                                        const VkVideoSessionParametersUpdateInfoKHR* pUpdateInfo,
                                                        const ErrorObject& error_obj) const override;
    bool PreCallValidateDestroyVideoSessionParametersKHR(VkDevice device, VkVideoSessionParametersKHR videoSessionParameters,
                                                         const VkAllocationCallbacks* pAllocator,
                                                         const ErrorObject& error_obj) const override;
    bool PreCallValidateGetEncodedVideoSessionParametersKHR(
        VkDevice device, const VkVideoEncodeSessionParametersGetInfoKHR* pVideoSessionParametersInfo,
        VkVideoEncodeSessionParametersFeedbackInfoKHR* pFeedbackInfo, size_t* pDataSize, void* pData,
        const ErrorObject& error_obj) const override;
    bool PreCallValidateCmdBeginVideoCodingKHR(VkCommandBuffer commandBuffer, const VkVideoBeginCodingInfoKHR* pBeginInfo,
                                               const ErrorObject& error_obj) const override;
    void PreCallRecordCmdBeginVideoCodingKHR(VkCommandBuffer commandBuffer, const VkVideoBeginCodingInfoKHR* pBeginInfo,
                                             const RecordObject& record_obj) override;
    bool PreCallValidateCmdEndVideoCodingKHR(VkCommandBuffer commandBuffer, const VkVideoEndCodingInfoKHR* pEndCodingInfo,
                                             const ErrorObject& error_obj) const override;
    bool PreCallValidateCmdControlVideoCodingKHR(VkCommandBuffer commandBuffer,
                                                 const VkVideoCodingControlInfoKHR* pCodingControlInfo,
                                                 const ErrorObject& error_obj) const override;
    void PreCallRecordCmdControlVideoCodingKHR(VkCommandBuffer commandBuffer, const VkVideoCodingControlInfoKHR* pCodingControlInfo,
                                               const RecordObject& record_obj) override;
    bool PreCallValidateCmdDecodeVideoKHR(VkCommandBuffer commandBuffer, const VkVideoDecodeInfoKHR* pDecodeInfo,
                                          const ErrorObject& error_obj) const override;
    void PreCallRecordCmdDecodeVideoKHR(VkCommandBuffer commandBuffer, const VkVideoDecodeInfoKHR* pDecodeInfo,
                                        const RecordObject& record_obj) override;
    bool PreCallValidateCmdEncodeVideoKHR(VkCommandBuffer commandBuffer, const VkVideoEncodeInfoKHR* pEncodeInfo,
                                          const ErrorObject& error_obj) const override;
    void PreCallRecordCmdEncodeVideoKHR(VkCommandBuffer commandBuffer, const VkVideoEncodeInfoKHR* pEncodeInfo,
                                        const RecordObject& record_obj) override;
    bool PreCallValidateCmdSetDiscardRectangleEXT(VkCommandBuffer commandBuffer, uint32_t firstDiscardRectangle,
                                                  uint32_t discardRectangleCount, const VkRect2D* pDiscardRectangles,
                                                  const ErrorObject& error_obj) const override;
    bool PreCallValidateCmdSetSampleLocationsEXT(VkCommandBuffer commandBuffer,
                                                 const VkSampleLocationsInfoEXT* pSampleLocationsInfo,
                                                 const ErrorObject& error_obj) const override;
    bool PreCallValidateCmdDrawIndirectCountKHR(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                VkBuffer countBuffer, VkDeviceSize countBufferOffset, uint32_t maxDrawCount,
                                                uint32_t stride, const ErrorObject& error_obj) const override;
    bool PreCallValidateCmdDrawIndirectCount(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                             VkBuffer countBuffer, VkDeviceSize countBufferOffset, uint32_t maxDrawCount,
                                             uint32_t stride, const ErrorObject& error_obj) const override;
    bool PreCallValidateCmdDrawIndirectByteCountEXT(VkCommandBuffer commandBuffer, uint32_t instanceCount, uint32_t firstInstance,
                                                    VkBuffer counterBuffer, VkDeviceSize counterBufferOffset,
                                                    uint32_t counterOffset, uint32_t vertexStride,
                                                    const ErrorObject& error_obj) const override;
    bool PreCallValidateCmdDrawMeshTasksNV(VkCommandBuffer commandBuffer, uint32_t taskCount, uint32_t firstTask,
                                           const ErrorObject& error_obj) const override;
    bool PreCallValidateCmdDrawMeshTasksIndirectNV(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                   uint32_t drawCount, uint32_t stride,
                                                   const ErrorObject& error_obj) const override;
    bool PreCallValidateCmdDrawMeshTasksIndirectCountNV(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                        VkBuffer countBuffer, VkDeviceSize countBufferOffset, uint32_t maxDrawCount,
                                                        uint32_t stride, const ErrorObject& error_obj) const override;
    bool PreCallValidateCmdDrawMeshTasksEXT(VkCommandBuffer commandBuffer, uint32_t groupCountX, uint32_t groupCountY,
                                            uint32_t groupCountZ, const ErrorObject& error_obj) const override;
    bool PreCallValidateCmdDrawMeshTasksIndirectEXT(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                    uint32_t drawCount, uint32_t stride,
                                                    const ErrorObject& error_obj) const override;
    bool PreCallValidateCmdDrawMeshTasksIndirectCountEXT(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                         VkBuffer countBuffer, VkDeviceSize countBufferOffset,
                                                         uint32_t maxDrawCount, uint32_t stride,
                                                         const ErrorObject& error_obj) const override;
    bool PreCallValidateGetBufferDeviceAddressEXT(VkDevice device, const VkBufferDeviceAddressInfo* pInfo,
                                                  const ErrorObject& error_obj) const override;
    bool PreCallValidateGetBufferDeviceAddress(VkDevice device, const VkBufferDeviceAddressInfo* pInfo,
                                               const ErrorObject& error_obj) const override;
    bool PreCallValidateGetBufferDeviceAddressKHR(VkDevice device, const VkBufferDeviceAddressInfo* pInfo,
                                                  const ErrorObject& error_obj) const override;
    bool PreCallValidateGetBufferOpaqueCaptureAddressKHR(VkDevice device, const VkBufferDeviceAddressInfo* pInfo,
                                                         const ErrorObject& error_obj) const override;
    bool PreCallValidateGetDeviceMemoryOpaqueCaptureAddressKHR(VkDevice device, const VkDeviceMemoryOpaqueCaptureAddressInfo* pInfo,
                                                               const ErrorObject& error_obj) const override;
    bool PreCallValidateGetBufferOpaqueCaptureAddress(VkDevice device, const VkBufferDeviceAddressInfo* pInfo,
                                                      const ErrorObject& error_obj) const override;
    bool PreCallValidateGetDeviceMemoryOpaqueCaptureAddress(VkDevice device, const VkDeviceMemoryOpaqueCaptureAddressInfo* pInfo,
                                                            const ErrorObject& error_obj) const override;
    bool PreCallValidateCmdSetDeviceMask(VkCommandBuffer commandBuffer, uint32_t deviceMask,
                                         const ErrorObject& error_obj) const override;
    bool PreCallValidateCmdSetDeviceMaskKHR(VkCommandBuffer commandBuffer, uint32_t deviceMask,
                                            const ErrorObject& error_obj) const override;
    bool PreCallValidateGetSemaphoreCounterValueKHR(VkDevice device, VkSemaphore sempahore, uint64_t* pValue,
                                                    const ErrorObject& error_obj) const override;
    bool PreCallValidateGetSemaphoreCounterValue(VkDevice device, VkSemaphore sempahore, uint64_t* pValue,
                                                 const ErrorObject& error_obj) const override;
    bool ValidateRequiredSubgroupSize(const spirv::Module& module_state, const PipelineStageState& stage_state,
                                      const VkPipelineShaderStageRequiredSubgroupSizeCreateInfoEXT& required_subgroup_size,
                                      uint64_t invocations, uint32_t local_size_x, uint32_t local_size_y, uint32_t local_size_z,
                                      const Location& loc) const;
    bool ValidateComputeWorkGroupSizes(const spirv::Module& module_state, const spirv::EntryPoint& entrypoint,
                                       const PipelineStageState& stage_state, uint32_t local_size_x, uint32_t local_size_y,
                                       uint32_t local_size_z, const Location& loc) const;
    bool ValidateTaskMeshWorkGroupSizes(const spirv::Module& module_state, const spirv::EntryPoint& entrypoint,
                                        const PipelineStageState& stage_state, uint32_t local_size_x, uint32_t local_size_y,
                                        uint32_t local_size_z, const Location& loc) const;
    bool ValidateEmitMeshTasksSize(const spirv::Module& module_state, const spirv::EntryPoint& entrypoint,
                                   const PipelineStageState& stage_state, const Location& loc) const;

    bool PreCallValidateResetQueryPoolEXT(VkDevice device, VkQueryPool queryPool, uint32_t firstQuery, uint32_t queryCount,
                                          const ErrorObject& error_obj) const override;
    bool PreCallValidateResetQueryPool(VkDevice device, VkQueryPool queryPool, uint32_t firstQuery, uint32_t queryCount,
                                       const ErrorObject& error_obj) const override;
    bool ValidateQueryPoolStride(const std::string& vuid_not_64, const std::string& vuid_64, const VkDeviceSize stride,
                                 const char* parameter_name, const uint64_t parameter_value, const VkQueryResultFlags flags,
                                 const Location& loc) const;
    bool ValidateCmdDrawStrideWithStruct(const vvl::CommandBuffer& cb_state, const std::string& vuid, const uint32_t stride,
                                         Struct struct_name, const uint32_t struct_size, const Location& loc) const;
    bool ValidateCmdDrawStrideWithBuffer(const vvl::CommandBuffer& cb_state, const std::string& vuid, const uint32_t stride,
                                         Struct struct_name, const uint32_t struct_size, const uint32_t drawCount,
                                         const VkDeviceSize offset, const vvl::Buffer* buffer_state, const Location& loc) const;
    bool PreCallValidateReleaseProfilingLockKHR(VkDevice device, const ErrorObject& error_obj) const override;
    bool PreCallValidateCreatePrivateDataSlotEXT(VkDevice device, const VkPrivateDataSlotCreateInfoEXT* pCreateInfo,
                                                 const VkAllocationCallbacks* pAllocator, VkPrivateDataSlotEXT* pPrivateDataSlot,
                                                 const ErrorObject& error_obj) const override;
    bool PreCallValidateCreatePrivateDataSlot(VkDevice device, const VkPrivateDataSlotCreateInfo* pCreateInfo,
                                              const VkAllocationCallbacks* pAllocator, VkPrivateDataSlot* pPrivateDataSlot,
                                              const ErrorObject& error_obj) const override;
    bool PreCallValidateCmdSetCheckpointNV(VkCommandBuffer commandBuffer, const void* pCheckpointMarker,
                                           const ErrorObject& error_obj) const override;
    bool PreCallValidateCmdBindTransformFeedbackBuffersEXT(VkCommandBuffer commandBuffer, uint32_t firstBinding,
                                                           uint32_t bindingCount, const VkBuffer* pBuffers,
                                                           const VkDeviceSize* pOffsets, const VkDeviceSize* pSizes,
                                                           const ErrorObject& error_obj) const override;
    bool PreCallValidateCmdBeginTransformFeedbackEXT(VkCommandBuffer commandBuffer, uint32_t firstCounterBuffer,
                                                     uint32_t counterBufferCount, const VkBuffer* pCounterBuffers,
                                                     const VkDeviceSize* pCounterBufferOffsets,
                                                     const ErrorObject& error_obj) const override;
    bool PreCallValidateCmdEndTransformFeedbackEXT(VkCommandBuffer commandBuffer, uint32_t firstCounterBuffer,
                                                   uint32_t counterBufferCount, const VkBuffer* pCounterBuffers,
                                                   const VkDeviceSize* pCounterBufferOffsets,
                                                   const ErrorObject& error_obj) const override;
    bool PreCallValidateCmdSetRayTracingPipelineStackSizeKHR(VkCommandBuffer commandBuffer, uint32_t pipelineStackSize,
                                                             const ErrorObject& error_obj) const override;
    bool PreCallValidateGetRayTracingShaderGroupStackSizeKHR(VkDevice device, VkPipeline pipeline, uint32_t group,
                                                             VkShaderGroupShaderKHR groupShader,
                                                             const ErrorObject& error_obj) const override;

    bool PreCallValidateWriteAccelerationStructuresPropertiesKHR(VkDevice device, uint32_t accelerationStructureCount,
                                                                 const VkAccelerationStructureKHR* pAccelerationStructures,
                                                                 VkQueryType queryType, size_t dataSize, void* pData, size_t stride,
                                                                 const ErrorObject& error_obj) const override;

    bool PreCallValidateCmdWriteAccelerationStructuresPropertiesKHR(VkCommandBuffer commandBuffer,
                                                                    uint32_t accelerationStructureCount,
                                                                    const VkAccelerationStructureKHR* pAccelerationStructures,
                                                                    VkQueryType queryType, VkQueryPool queryPool,
                                                                    uint32_t firstQuery,
                                                                    const ErrorObject& error_obj) const override;

    bool PreCallValidateCmdWriteAccelerationStructuresPropertiesNV(VkCommandBuffer commandBuffer,
                                                                   uint32_t accelerationStructureCount,
                                                                   const VkAccelerationStructureNV* pAccelerationStructures,
                                                                   VkQueryType queryType, VkQueryPool queryPool,
                                                                   uint32_t firstQuery,
                                                                   const ErrorObject& error_obj) const override;

    // Calculates the total number of shader groups taking libraries into account.
    uint32_t CalcTotalShaderGroupCount(const vvl::Pipeline& pipeline) const;

    bool PreCallValidateGetRayTracingShaderGroupHandlesKHR(VkDevice device, VkPipeline pipeline, uint32_t firstGroup,
                                                           uint32_t groupCount, size_t dataSize, void* pData,
                                                           const ErrorObject& error_obj) const override;

    bool PreCallValidateGetRayTracingCaptureReplayShaderGroupHandlesKHR(VkDevice device, VkPipeline pipeline, uint32_t firstGroup,
                                                                        uint32_t groupCount, size_t dataSize, void* pData,
                                                                        const ErrorObject& error_obj) const override;

    bool PreCallValidateCmdBuildAccelerationStructuresIndirectKHR(VkCommandBuffer commandBuffer, uint32_t infoCount,
                                                                  const VkAccelerationStructureBuildGeometryInfoKHR* pInfos,
                                                                  const VkDeviceAddress* pIndirectDeviceAddresses,
                                                                  const uint32_t* pIndirectStrides,
                                                                  const uint32_t* const* ppMaxPrimitiveCounts,
                                                                  const ErrorObject& error_obj) const override;
    bool ValidateCopyAccelerationStructureInfoKHR(const VkCopyAccelerationStructureInfoKHR* pInfo, const VulkanTypedHandle& handle,
                                                  const Location& info_loc) const;
    bool PreCallValidateCmdCopyAccelerationStructureKHR(VkCommandBuffer commandBuffer,
                                                        const VkCopyAccelerationStructureInfoKHR* pInfo,
                                                        const ErrorObject& error_obj) const override;
    bool PreCallValidateDestroyDeferredOperationKHR(VkDevice device, VkDeferredOperationKHR operation,
                                                    const VkAllocationCallbacks* pAllocator,
                                                    const ErrorObject& error_obj) const override;
    bool PreCallValidateCopyAccelerationStructureKHR(VkDevice device, VkDeferredOperationKHR deferredOperation,
                                                     const VkCopyAccelerationStructureInfoKHR* pInfo,
                                                     const ErrorObject& error_obj) const override;
    bool PreCallValidateCopyAccelerationStructureToMemoryKHR(VkDevice device, VkDeferredOperationKHR deferredOperation,
                                                             const VkCopyAccelerationStructureToMemoryInfoKHR* pInfo,
                                                             const ErrorObject& error_obj) const override;
    bool PreCallValidateCmdCopyAccelerationStructureToMemoryKHR(VkCommandBuffer commandBuffer,
                                                                const VkCopyAccelerationStructureToMemoryInfoKHR* pInfo,
                                                                const ErrorObject& error_obj) const override;
    bool PreCallValidateCopyMemoryToAccelerationStructureKHR(VkDevice device, VkDeferredOperationKHR deferredOperation,
                                                             const VkCopyMemoryToAccelerationStructureInfoKHR* pInfo,
                                                             const ErrorObject& error_obj) const override;
    bool PreCallValidateCmdCopyMemoryToAccelerationStructureKHR(VkCommandBuffer commandBuffer,
                                                                const VkCopyMemoryToAccelerationStructureInfoKHR* pInfo,
                                                                const ErrorObject& error_obj) const override;
    bool ValidateExtendedDynamicState(const vvl::CommandBuffer& cb_state, const Location& loc, bool feature, const char* vuid,
                                      const char* feature_name) const;
    bool PreCallValidateCmdSetLogicOpEXT(VkCommandBuffer commandBuffer, VkLogicOp logicOp,
                                         const ErrorObject& error_obj) const override;
    bool PreCallValidateCmdSetPatchControlPointsEXT(VkCommandBuffer commandBuffer, uint32_t patchControlPoints,
                                                    const ErrorObject& error_obj) const override;
    bool PreCallValidateCmdSetRasterizerDiscardEnableEXT(VkCommandBuffer commandBuffer, VkBool32 rasterizerDiscardEnable,
                                                         const ErrorObject& error_obj) const override;
    bool PreCallValidateCmdSetRasterizerDiscardEnable(VkCommandBuffer commandBuffer, VkBool32 rasterizerDiscardEnable,
                                                      const ErrorObject& error_obj) const override;
    bool PreCallValidateCmdSetDepthBiasEnableEXT(VkCommandBuffer commandBuffer, VkBool32 depthBiasEnable,
                                                 const ErrorObject& error_obj) const override;
    bool PreCallValidateCmdSetDepthBiasEnable(VkCommandBuffer commandBuffer, VkBool32 depthBiasEnable,
                                              const ErrorObject& error_obj) const override;
    bool PreCallValidateCmdSetPrimitiveRestartEnableEXT(VkCommandBuffer commandBuffer, VkBool32 primitiveRestartEnable,
                                                        const ErrorObject& error_obj) const override;
    bool PreCallValidateCmdSetPrimitiveRestartEnable(VkCommandBuffer commandBuffer, VkBool32 primitiveRestartEnable,
                                                     const ErrorObject& error_obj) const override;

    bool PreCallValidateCmdSetCullModeEXT(VkCommandBuffer commandBuffer, VkCullModeFlags cullMode,
                                          const ErrorObject& error_obj) const override;
    bool PreCallValidateCmdSetCullMode(VkCommandBuffer commandBuffer, VkCullModeFlags cullMode,
                                       const ErrorObject& error_obj) const override;
    bool PreCallValidateCmdSetFrontFaceEXT(VkCommandBuffer commandBuffer, VkFrontFace frontFace,
                                           const ErrorObject& error_obj) const override;
    bool PreCallValidateCmdSetFrontFace(VkCommandBuffer commandBuffer, VkFrontFace frontFace,
                                        const ErrorObject& error_obj) const override;
    bool PreCallValidateCmdSetPrimitiveTopologyEXT(VkCommandBuffer commandBuffer, VkPrimitiveTopology primitiveTopology,
                                                   const ErrorObject& error_obj) const override;
    bool PreCallValidateCmdSetPrimitiveTopology(VkCommandBuffer commandBuffer, VkPrimitiveTopology primitiveTopology,
                                                const ErrorObject& error_obj) const override;
    bool PreCallValidateCmdSetViewportWithCountEXT(VkCommandBuffer commandBuffer, uint32_t viewportCount,
                                                   const VkViewport* pViewports, const ErrorObject& error_obj) const override;
    bool PreCallValidateCmdSetViewportWithCount(VkCommandBuffer commandBuffer, uint32_t viewportCount, const VkViewport* pViewports,
                                                const ErrorObject& error_obj) const override;
    bool PreCallValidateCmdSetScissorWithCountEXT(VkCommandBuffer commandBuffer, uint32_t scissorCount, const VkRect2D* pScissors,
                                                  const ErrorObject& error_obj) const override;
    bool PreCallValidateCmdSetScissorWithCount(VkCommandBuffer commandBuffer, uint32_t scissorCount, const VkRect2D* pScissors,
                                               const ErrorObject& error_obj) const override;
    bool PreCallValidateCmdBindVertexBuffers2EXT(VkCommandBuffer commandBuffer, uint32_t firstBinding, uint32_t bindingCount,
                                                 const VkBuffer* pBuffers, const VkDeviceSize* pOffsets, const VkDeviceSize* pSizes,
                                                 const VkDeviceSize* pStrides, const ErrorObject& error_obj) const override;
    bool PreCallValidateCmdBindVertexBuffers2(VkCommandBuffer commandBuffer, uint32_t firstBinding, uint32_t bindingCount,
                                              const VkBuffer* pBuffers, const VkDeviceSize* pOffsets, const VkDeviceSize* pSizes,
                                              const VkDeviceSize* pStrides, const ErrorObject& error_obj) const override;
    bool PreCallValidateCmdSetDepthTestEnableEXT(VkCommandBuffer commandBuffer, VkBool32 depthTestEnable,
                                                 const ErrorObject& error_obj) const override;
    bool PreCallValidateCmdSetDepthTestEnable(VkCommandBuffer commandBuffer, VkBool32 depthTestEnable,
                                              const ErrorObject& error_obj) const override;
    bool PreCallValidateCmdSetDepthWriteEnableEXT(VkCommandBuffer commandBuffer, VkBool32 depthWriteEnable,
                                                  const ErrorObject& error_obj) const override;
    bool PreCallValidateCmdSetDepthWriteEnable(VkCommandBuffer commandBuffer, VkBool32 depthWriteEnable,
                                               const ErrorObject& error_obj) const override;
    bool PreCallValidateCmdSetDepthCompareOpEXT(VkCommandBuffer commandBuffer, VkCompareOp depthCompareOp,
                                                const ErrorObject& error_obj) const override;
    bool PreCallValidateCmdSetDepthCompareOp(VkCommandBuffer commandBuffer, VkCompareOp depthCompareOp,
                                             const ErrorObject& error_obj) const override;
    bool PreCallValidateCmdSetDepthBoundsTestEnableEXT(VkCommandBuffer commandBuffer, VkBool32 depthBoundsTestEnable,
                                                       const ErrorObject& error_obj) const override;
    bool PreCallValidateCmdSetDepthBoundsTestEnable(VkCommandBuffer commandBuffer, VkBool32 depthBoundsTestEnable,
                                                    const ErrorObject& error_obj) const override;
    bool PreCallValidateCmdSetStencilTestEnableEXT(VkCommandBuffer commandBuffer, VkBool32 stencilTestEnable,
                                                   const ErrorObject& error_obj) const override;
    bool PreCallValidateCmdSetStencilTestEnable(VkCommandBuffer commandBuffer, VkBool32 stencilTestEnable,
                                                const ErrorObject& error_obj) const override;
    bool PreCallValidateCmdSetStencilOpEXT(VkCommandBuffer commandBuffer, VkStencilFaceFlags faceMask, VkStencilOp failOp,
                                           VkStencilOp passOp, VkStencilOp depthFailOp, VkCompareOp compareOp,
                                           const ErrorObject& error_obj) const override;
    bool PreCallValidateCmdSetStencilOp(VkCommandBuffer commandBuffer, VkStencilFaceFlags faceMask, VkStencilOp failOp,
                                        VkStencilOp passOp, VkStencilOp depthFailOp, VkCompareOp compareOp,
                                        const ErrorObject& error_obj) const override;
    bool PreCallValidateCmdSetTessellationDomainOriginEXT(VkCommandBuffer commandBuffer, VkTessellationDomainOrigin domainOrigin,
                                                          const ErrorObject& error_obj) const override;
    bool PreCallValidateCmdSetDepthClampEnableEXT(VkCommandBuffer commandBuffer, VkBool32 depthClampEnable,
                                                  const ErrorObject& error_obj) const override;
    bool PreCallValidateCmdSetPolygonModeEXT(VkCommandBuffer commandBuffer, VkPolygonMode polygonMode,
                                             const ErrorObject& error_obj) const override;
    bool PreCallValidateCmdSetRasterizationSamplesEXT(VkCommandBuffer commandBuffer, VkSampleCountFlagBits rasterizationSamples,
                                                      const ErrorObject& error_obj) const override;
    bool PreCallValidateCmdSetSampleMaskEXT(VkCommandBuffer commandBuffer, VkSampleCountFlagBits samples,
                                            const VkSampleMask* pSampleMask, const ErrorObject& error_obj) const override;
    bool PreCallValidateCmdSetAlphaToCoverageEnableEXT(VkCommandBuffer commandBuffer, VkBool32 alphaToCoverageEnable,
                                                       const ErrorObject& error_obj) const override;
    bool PreCallValidateCmdSetAlphaToOneEnableEXT(VkCommandBuffer commandBuffer, VkBool32 alphaToOneEnable,
                                                  const ErrorObject& error_obj) const override;
    bool PreCallValidateCmdSetLogicOpEnableEXT(VkCommandBuffer commandBuffer, VkBool32 logicOpEnable,
                                               const ErrorObject& error_obj) const override;
    bool PreCallValidateCmdSetColorBlendEnableEXT(VkCommandBuffer commandBuffer, uint32_t firstAttachment, uint32_t attachmentCount,
                                                  const VkBool32* pColorBlendEnables, const ErrorObject& error_obj) const override;
    bool PreCallValidateCmdSetColorBlendEquationEXT(VkCommandBuffer commandBuffer, uint32_t firstAttachment,
                                                    uint32_t attachmentCount, const VkColorBlendEquationEXT* pColorBlendEquations,
                                                    const ErrorObject& error_obj) const override;
    bool PreCallValidateCmdSetColorWriteMaskEXT(VkCommandBuffer commandBuffer, uint32_t firstAttachment, uint32_t attachmentCount,
                                                const VkColorComponentFlags* pColorWriteMasks,
                                                const ErrorObject& error_obj) const override;
    bool PreCallValidateCmdSetRasterizationStreamEXT(VkCommandBuffer commandBuffer, uint32_t rasterizationStream,
                                                     const ErrorObject& error_obj) const override;
    bool PreCallValidateCmdSetConservativeRasterizationModeEXT(VkCommandBuffer commandBuffer,
                                                               VkConservativeRasterizationModeEXT conservativeRasterizationMode,
                                                               const ErrorObject& error_obj) const override;
    bool PreCallValidateCmdSetExtraPrimitiveOverestimationSizeEXT(VkCommandBuffer commandBuffer,
                                                                  float extraPrimitiveOverestimationSize,
                                                                  const ErrorObject& error_obj) const override;
    bool PreCallValidateCmdSetDepthClipEnableEXT(VkCommandBuffer commandBuffer, VkBool32 depthClipEnable,
                                                 const ErrorObject& error_obj) const override;
    bool PreCallValidateCmdSetSampleLocationsEnableEXT(VkCommandBuffer commandBuffer, VkBool32 sampleLocationsEnable,
                                                       const ErrorObject& error_obj) const override;
    bool PreCallValidateCmdSetColorBlendAdvancedEXT(VkCommandBuffer commandBuffer, uint32_t firstAttachment,
                                                    uint32_t attachmentCount, const VkColorBlendAdvancedEXT* pColorBlendAdvanced,
                                                    const ErrorObject& error_obj) const override;
    bool PreCallValidateCmdSetProvokingVertexModeEXT(VkCommandBuffer commandBuffer, VkProvokingVertexModeEXT provokingVertexMode,
                                                     const ErrorObject& error_obj) const override;
    bool PreCallValidateCmdSetLineRasterizationModeEXT(VkCommandBuffer commandBuffer,
                                                       VkLineRasterizationModeEXT lineRasterizationMode,
                                                       const ErrorObject& error_obj) const override;
    bool PreCallValidateCmdSetLineStippleEnableEXT(VkCommandBuffer commandBuffer, VkBool32 stippledLineEnable,
                                                   const ErrorObject& error_obj) const override;
    bool PreCallValidateCmdSetDepthClipNegativeOneToOneEXT(VkCommandBuffer commandBuffer, VkBool32 negativeOneToOne,
                                                           const ErrorObject& error_obj) const override;
    bool PreCallValidateCmdSetViewportWScalingEnableNV(VkCommandBuffer commandBuffer, VkBool32 viewportWScalingEnable,
                                                       const ErrorObject& error_obj) const override;
    bool PreCallValidateCmdSetViewportSwizzleNV(VkCommandBuffer commandBuffer, uint32_t firstViewport, uint32_t viewportCount,
                                                const VkViewportSwizzleNV* pViewportSwizzles,
                                                const ErrorObject& error_obj) const override;
    bool PreCallValidateCmdSetCoverageToColorEnableNV(VkCommandBuffer commandBuffer, VkBool32 coverageToColorEnable,
                                                      const ErrorObject& error_obj) const override;
    bool PreCallValidateCmdSetCoverageToColorLocationNV(VkCommandBuffer commandBuffer, uint32_t coverageToColorLocation,
                                                        const ErrorObject& error_obj) const override;
    bool PreCallValidateCmdSetCoverageModulationModeNV(VkCommandBuffer commandBuffer,
                                                       VkCoverageModulationModeNV coverageModulationMode,
                                                       const ErrorObject& error_obj) const override;
    bool PreCallValidateCmdSetCoverageModulationTableEnableNV(VkCommandBuffer commandBuffer, VkBool32 coverageModulationTableEnable,
                                                              const ErrorObject& error_obj) const override;
    bool PreCallValidateCmdSetCoverageModulationTableNV(VkCommandBuffer commandBuffer, uint32_t coverageModulationTableCount,
                                                        const float* pCoverageModulationTable,
                                                        const ErrorObject& error_obj) const override;
    bool PreCallValidateCmdSetShadingRateImageEnableNV(VkCommandBuffer commandBuffer, VkBool32 shadingRateImageEnable,
                                                       const ErrorObject& error_obj) const override;
    bool PreCallValidateCmdSetRepresentativeFragmentTestEnableNV(VkCommandBuffer commandBuffer,
                                                                 VkBool32 representativeFragmentTestEnable,
                                                                 const ErrorObject& error_obj) const override;
    bool PreCallValidateCmdSetCoverageReductionModeNV(VkCommandBuffer commandBuffer,
                                                      VkCoverageReductionModeNV coverageReductionMode,
                                                      const ErrorObject& error_obj) const override;
    bool PreCallValidateCreateEvent(VkDevice device, const VkEventCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator,
                                    VkEvent* pEvent, const ErrorObject& error_obj) const override;
    bool PreCallValidateCmdSetFragmentShadingRateKHR(VkCommandBuffer commandBuffer, const VkExtent2D* pFragmentSize,
                                                     const VkFragmentShadingRateCombinerOpKHR combinerOps[2],
                                                     const ErrorObject& error_obj) const override;
    bool PreCallValidateCmdSetColorWriteEnableEXT(VkCommandBuffer commandBuffer, uint32_t attachmentCount,
                                                  const VkBool32* pColorWriteEnables, const ErrorObject& error_obj) const override;
    bool PreCallValidateCmdSetVertexInputEXT(VkCommandBuffer commandBuffer, uint32_t vertexBindingDescriptionCount,
                                             const VkVertexInputBindingDescription2EXT* pVertexBindingDescriptions,
                                             uint32_t vertexAttributeDescriptionCount,
                                             const VkVertexInputAttributeDescription2EXT* pVertexAttributeDescriptions,
                                             const ErrorObject& error_obj) const override;
    bool PreCallValidateCmdSetCoarseSampleOrderNV(VkCommandBuffer commandBuffer, VkCoarseSampleOrderTypeNV sampleOrderType,
                                                  uint32_t customSampleOrderCount,
                                                  const VkCoarseSampleOrderCustomNV* pCustomSampleOrders,
                                                  const ErrorObject& error_obj) const override;
    bool PreCallValidateCmdSetFragmentShadingRateEnumNV(VkCommandBuffer commandBuffer, VkFragmentShadingRateNV shadingRate,
                                                        const VkFragmentShadingRateCombinerOpKHR combinerOps[2],
                                                        const ErrorObject& error_obj) const override;
    bool PreCallValidateCmdSetPerformanceMarkerINTEL(VkCommandBuffer commandBuffer, const VkPerformanceMarkerInfoINTEL* pMarkerInfo,
                                                     const ErrorObject& error_obj) const override;
    bool PreCallValidateCmdSetPerformanceStreamMarkerINTEL(VkCommandBuffer commandBuffer,
                                                           const VkPerformanceStreamMarkerInfoINTEL* pMarkerInfo,
                                                           const ErrorObject& error_obj) const override;
    bool PreCallValidateCmdSetPerformanceOverrideINTEL(VkCommandBuffer commandBuffer,
                                                       const VkPerformanceOverrideInfoINTEL* pOverrideInfo,
                                                       const ErrorObject& error_obj) const override;
    bool PreCallValidateCmdSetAttachmentFeedbackLoopEnableEXT(VkCommandBuffer commandBuffer, VkImageAspectFlags aspectMask,
                                                              const ErrorObject& error_obj) const override;
    bool PreCallValidateCmdBeginConditionalRenderingEXT(VkCommandBuffer commandBuffer,
                                                        const VkConditionalRenderingBeginInfoEXT* pConditionalRenderingBegin,
                                                        const ErrorObject& error_obj) const override;
    bool PreCallValidateCmdEndConditionalRenderingEXT(VkCommandBuffer commandBuffer, const ErrorObject& error_obj) const override;
#ifdef VK_USE_PLATFORM_WIN32_KHR
    bool PreCallValidateAcquireFullScreenExclusiveModeEXT(VkDevice device, VkSwapchainKHR swapchain,
                                                          const ErrorObject& error_obj) const override;
    bool PreCallValidateReleaseFullScreenExclusiveModeEXT(VkDevice device, VkSwapchainKHR swapchain,
                                                          const ErrorObject& error_obj) const override;
#endif

    bool ValidatePhysicalDeviceSurfaceSupport(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface, const char* vuid,
                                              const Location& loc) const;
#ifdef VK_USE_PLATFORM_WIN32_KHR
    bool PreCallValidateGetDeviceGroupSurfacePresentModes2EXT(VkDevice device, const VkPhysicalDeviceSurfaceInfo2KHR* pSurfaceInfo,
                                                              VkDeviceGroupPresentModeFlagsKHR* pModes,
                                                              const ErrorObject& error_obj) const override;
    bool PreCallValidateGetPhysicalDeviceSurfacePresentModes2EXT(VkPhysicalDevice physicalDevice,
                                                                 const VkPhysicalDeviceSurfaceInfo2KHR* pSurfaceInfo,
                                                                 uint32_t* pPresentModeCount, VkPresentModeKHR* pPresentModes,
                                                                 const ErrorObject& error_obj) const override;
#endif
    bool PreCallValidateGetDeviceGroupSurfacePresentModesKHR(VkDevice device, VkSurfaceKHR surface,
                                                             VkDeviceGroupPresentModeFlagsKHR* pModes,
                                                             const ErrorObject& error_obj) const override;
    bool PreCallValidateGetPhysicalDevicePresentRectanglesKHR(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface,
                                                              uint32_t* pRectCount, VkRect2D* pRects,
                                                              const ErrorObject& error_obj) const override;
    bool PreCallValidateGetPhysicalDeviceSurfaceCapabilities2EXT(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface,
                                                                 VkSurfaceCapabilities2EXT* pSurfaceCapabilities,
                                                                 const ErrorObject& error_obj) const override;
    bool PreCallValidateGetPhysicalDeviceSurfaceCapabilities2KHR(VkPhysicalDevice physicalDevice,
                                                                 const VkPhysicalDeviceSurfaceInfo2KHR* pSurfaceInfo,
                                                                 VkSurfaceCapabilities2KHR* pSurfaceCapabilities,
                                                                 const ErrorObject& error_obj) const override;
    bool PreCallValidateGetPhysicalDeviceSurfaceCapabilitiesKHR(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface,
                                                                VkSurfaceCapabilitiesKHR* pSurfaceCapabilities,
                                                                const ErrorObject& error_obj) const override;
    bool PreCallValidateGetPhysicalDeviceSurfaceFormats2KHR(VkPhysicalDevice physicalDevice,
                                                            const VkPhysicalDeviceSurfaceInfo2KHR* pSurfaceInfo,
                                                            uint32_t* pSurfaceFormatCount, VkSurfaceFormat2KHR* pSurfaceFormats,
                                                            const ErrorObject& error_obj) const override;
    bool PreCallValidateGetPhysicalDeviceSurfaceFormatsKHR(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface,
                                                           uint32_t* pSurfaceFormatCount, VkSurfaceFormatKHR* pSurfaceFormats,
                                                           const ErrorObject& error_obj) const override;
    bool PreCallValidateGetPhysicalDeviceSurfacePresentModesKHR(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface,
                                                                uint32_t* pPresentModeCount, VkPresentModeKHR* pPresentModes,
                                                                const ErrorObject& error_obj) const override;
    void PostCallRecordGetQueryPoolResults(VkDevice device, VkQueryPool queryPool, uint32_t firstQuery, uint32_t queryCount,
                                           size_t dataSize, void* pData, VkDeviceSize stride, VkQueryResultFlags flags,
                                           const RecordObject& record_obj) override;
    bool ValidateGetImageSubresourceLayout(const vvl::Image& image_state, const VkImageSubresource& subresource,
                                           const Location& subresource_loc) const;
    bool PreCallValidateTransitionImageLayoutEXT(VkDevice device, uint32_t transitionCount,
                                                 const VkHostImageLayoutTransitionInfoEXT* pTransitions,
                                                 const ErrorObject& error_obj) const override;
    void PostCallRecordTransitionImageLayoutEXT(VkDevice device, uint32_t transitionCount,
                                                const VkHostImageLayoutTransitionInfoEXT* pTransitions,
                                                const RecordObject& record_obj) override;
    bool PreCallValidateGetImageSubresourceLayout(VkDevice device, VkImage image, const VkImageSubresource* pSubresource,
                                                  VkSubresourceLayout* pLayout, const ErrorObject& error_obj) const override;
    bool PreCallValidateGetImageSubresourceLayout2KHR(VkDevice device, VkImage image, const VkImageSubresource2KHR* pSubresource,
                                                      VkSubresourceLayout2KHR* pLayout,
                                                      const ErrorObject& error_obj) const override;
    bool PreCallValidateGetImageSubresourceLayout2EXT(VkDevice device, VkImage image, const VkImageSubresource2EXT* pSubresource,
                                                      VkSubresourceLayout2EXT* pLayout,
                                                      const ErrorObject& error_obj) const override;
    bool PreCallValidateGetImageDrmFormatModifierPropertiesEXT(VkDevice device, VkImage image,
                                                               VkImageDrmFormatModifierPropertiesEXT* pProperties,
                                                               const ErrorObject& error_obj) const override;

    bool PreCallValidateGetDescriptorSetLayoutSizeEXT(VkDevice device, VkDescriptorSetLayout layout,
                                                      VkDeviceSize* pLayoutSizeInBytes,
                                                      const ErrorObject& error_obj) const override;
    bool PreCallValidateGetDescriptorSetLayoutBindingOffsetEXT(VkDevice device, VkDescriptorSetLayout layout, uint32_t binding,
                                                               VkDeviceSize* pOffset, const ErrorObject& error_obj) const override;
    bool PreCallValidateGetBufferOpaqueCaptureDescriptorDataEXT(VkDevice device, const VkBufferCaptureDescriptorDataInfoEXT* pInfo,
                                                                void* pData, const ErrorObject& error_obj) const override;
    bool PreCallValidateGetImageOpaqueCaptureDescriptorDataEXT(VkDevice device, const VkImageCaptureDescriptorDataInfoEXT* pInfo,
                                                               void* pData, const ErrorObject& error_obj) const override;
    bool PreCallValidateGetImageViewOpaqueCaptureDescriptorDataEXT(VkDevice device,
                                                                   const VkImageViewCaptureDescriptorDataInfoEXT* pInfo,
                                                                   void* pData, const ErrorObject& error_obj) const override;
    bool PreCallValidateGetSamplerOpaqueCaptureDescriptorDataEXT(VkDevice device,
                                                                 const VkSamplerCaptureDescriptorDataInfoEXT* pInfo, void* pData,
                                                                 const ErrorObject& error_obj) const override;
    bool PreCallValidateGetAccelerationStructureOpaqueCaptureDescriptorDataEXT(
        VkDevice device, const VkAccelerationStructureCaptureDescriptorDataInfoEXT* pInfo, void* pData,
        const ErrorObject& error_obj) const override;
    bool PreCallValidateCmdSetDescriptorBufferOffsetsEXT(VkCommandBuffer commandBuffer, VkPipelineBindPoint pipelineBindPoint,
                                                         VkPipelineLayout layout, uint32_t firstSet, uint32_t setCount,
                                                         const uint32_t* pBufferIndices, const VkDeviceSize* pOffsets,
                                                         const ErrorObject& error_obj) const override;
    bool ValidateCmdSetDescriptorBufferOffsets(const vvl::CommandBuffer& cb_state, VkPipelineLayout layout, uint32_t firstSet,
                                               uint32_t setCount, const uint32_t* pBufferIndices, const VkDeviceSize* pOffsets,
                                               const Location& loc) const;
    bool PreCallValidateCmdSetDescriptorBufferOffsets2EXT(
        VkCommandBuffer commandBuffer, const VkSetDescriptorBufferOffsetsInfoEXT* pSetDescriptorBufferOffsetsInfo,
        const ErrorObject& error_obj) const override;
    bool ValidateCmdBindDescriptorBufferEmbeddedSamplers(const vvl::CommandBuffer& cb_state, VkPipelineLayout layout, uint32_t set,
                                                         const Location& loc) const;
    bool PreCallValidateCmdBindDescriptorBufferEmbeddedSamplersEXT(VkCommandBuffer commandBuffer,
                                                                   VkPipelineBindPoint pipelineBindPoint, VkPipelineLayout layout,
                                                                   uint32_t set, const ErrorObject& error_obj) const override;
    bool PreCallValidateCmdBindDescriptorBufferEmbeddedSamplers2EXT(
        VkCommandBuffer commandBuffer,
        const VkBindDescriptorBufferEmbeddedSamplersInfoEXT* pBindDescriptorBufferEmbeddedSamplersInfo,
        const ErrorObject& error_obj) const override;
    bool PreCallValidateCmdBindDescriptorBuffersEXT(VkCommandBuffer commandBuffer, uint32_t bufferCount,
                                                    const VkDescriptorBufferBindingInfoEXT* pBindingInfos,
                                                    const ErrorObject& error_obj) const override;
    bool ValidateDescriptorAddressInfoEXT(const VkDescriptorAddressInfoEXT* address_info, const Location& address_loc) const;
    bool ValidateGetDescriptorDataSize(const VkDescriptorGetInfoEXT& descriptor_info, const size_t data_size,
                                       const Location& descriptor_info_loc) const;
    bool PreCallValidateGetDescriptorEXT(VkDevice device, const VkDescriptorGetInfoEXT* pDescriptorInfo, size_t dataSize,
                                         void* pDescriptor, const ErrorObject& error_obj) const override;
    bool PreCallValidateGetCalibratedTimestampsEXT(VkDevice device, uint32_t timestampCount,
                                                   const VkCalibratedTimestampInfoEXT* pTimestampInfos, uint64_t* pTimestamps,
                                                   uint64_t* pMaxDeviation, const ErrorObject& error_obj) const override;
    bool PreCallValidateGetCalibratedTimestampsKHR(VkDevice device, uint32_t timestampCount,
                                                   const VkCalibratedTimestampInfoEXT* pTimestampInfos, uint64_t* pTimestamps,
                                                   uint64_t* pMaxDeviation, const ErrorObject& error_obj) const override;

    // Debug label APIs
    void PostCallRecordCmdBeginDebugUtilsLabelEXT(VkCommandBuffer commandBuffer, const VkDebugUtilsLabelEXT* pLabelInfo,
                                                  const RecordObject& record_obj) override;
    bool PreCallValidateCmdEndDebugUtilsLabelEXT(VkCommandBuffer commandBuffer, const ErrorObject& error_obj) const override;
    void PostCallRecordCmdEndDebugUtilsLabelEXT(VkCommandBuffer commandBuffer, const RecordObject& record_obj) override;

#ifdef VK_USE_PLATFORM_METAL_EXT
    bool PreCallValidateExportMetalObjectsEXT(VkDevice device, VkExportMetalObjectsInfoEXT* pMetalObjectsInfo,
                                              const ErrorObject& error_obj) const override;
#endif  // VK_USE_PLATFORM_METAL_EXT

#ifdef VK_USE_PLATFORM_ANDROID_KHR
    bool PreCallValidateGetAndroidHardwareBufferPropertiesANDROID(VkDevice device, const struct AHardwareBuffer* buffer,
                                                                  VkAndroidHardwareBufferPropertiesANDROID* pProperties,
                                                                  const ErrorObject& error_obj) const override;
    bool PreCallValidateGetMemoryAndroidHardwareBufferANDROID(VkDevice device,
                                                              const VkMemoryGetAndroidHardwareBufferInfoANDROID* pInfo,
                                                              struct AHardwareBuffer** pBuffer,
                                                              const ErrorObject& error_obj) const override;
#endif  // VK_USE_PLATFORM_ANDROID_KHR
#ifdef VK_USE_PLATFORM_WAYLAND_KHR
    bool PreCallValidateGetPhysicalDeviceWaylandPresentationSupportKHR(VkPhysicalDevice physicalDevice, uint32_t queueFamilyIndex,
                                                                       struct wl_display* display,
                                                                       const ErrorObject& error_obj) const override;
#endif  // VK_USE_PLATFORM_WAYLAND_KHR
#ifdef VK_USE_PLATFORM_WIN32_KHR
    bool PreCallValidateGetPhysicalDeviceWin32PresentationSupportKHR(VkPhysicalDevice physicalDevice, uint32_t queueFamilyIndex,
                                                                     const ErrorObject& error_obj) const override;
#endif  // VK_USE_PLATFORM_WIN32_KHR
#ifdef VK_USE_PLATFORM_XCB_KHR
    bool PreCallValidateGetPhysicalDeviceXcbPresentationSupportKHR(VkPhysicalDevice physicalDevice, uint32_t queueFamilyIndex,
                                                                   xcb_connection_t* connection, xcb_visualid_t visual_id,
                                                                   const ErrorObject& error_obj) const override;
#endif  // VK_USE_PLATFORM_XCB_KHR
#ifdef VK_USE_PLATFORM_XLIB_KHR
    bool PreCallValidateGetPhysicalDeviceXlibPresentationSupportKHR(VkPhysicalDevice physicalDevice, uint32_t queueFamilyIndex,
                                                                    Display* dpy, VisualID visualID,
                                                                    const ErrorObject& error_obj) const override;
#endif  // VK_USE_PLATFORM_XLIB_KHR
#ifdef VK_USE_PLATFORM_SCREEN_QNX
    bool PreCallValidateGetPhysicalDeviceScreenPresentationSupportQNX(VkPhysicalDevice physicalDevice, uint32_t queueFamilyIndex,
                                                                      struct _screen_window* window,
                                                                      const ErrorObject& error_obj) const override;
#endif  // VK_USE_PLATFORM_SCREEN_QNX
    std::shared_ptr<vvl::CommandBuffer> CreateCmdBufferState(VkCommandBuffer cb, const VkCommandBufferAllocateInfo* create_info,
                                                             const vvl::CommandPool* pool) override {
        return std::static_pointer_cast<vvl::CommandBuffer>(std::make_shared<CORE_CMD_BUFFER_STATE>(this, cb, create_info, pool));
    }
};  // Class CoreChecks
